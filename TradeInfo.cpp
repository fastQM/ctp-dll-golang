#include "stdafx.h"
#include "TradeInfo.h"

#include "rapidjson/rapidjson.h"  
#include "rapidjson/document.h"  
#include "rapidjson/reader.h"  
#include "rapidjson/writer.h"  
#include "rapidjson/stringbuffer.h"  
using namespace rapidjson;


CTradeInfo::CTradeInfo(CBFunction cb) {
	mTradeResult = TradeNone;
	mStatus = StatusNone;
	mValuesLength = 0;
	mCallback = cb;
	memset(mValues, 0, sizeof(DepthValue) * MAX_SIZE);
	memset(mInstruments, 0, sizeof(CThostFtdcInstrumentField) * MAX_SIZE);
	memset(mPosition, 0, sizeof(CThostFtdcInvestorPositionField) * MAX_SIZE);

	mTradedIndex = 0;
	memset(mTraded, 0, sizeof(TradedInfo)*MAX_SIZE);

	memset(mOrderRef, 0, 2*256);
	memset(mOrderSysID, 0, 21);

	InitializeSRWLock(&mSrwlockDepth);
	InitializeSRWLock(&mSrwlockStatus);
}

CTradeInfo::~CTradeInfo(){

}


void CTradeInfo::updateDepth(DepthValue value) {

	AcquireSRWLockExclusive(&mSrwlockDepth);
	//cout << "[ID]" << value.name << "[ASK]" << value.askPrice << "[ASKVN]" << 
	//	value.askVolumn << "[BID]" << value.bidPrice << "[BIDVN]" << value.bidVolumn << endl;

	if (mValuesLength == 0) {
		memcpy(&mValues[0], &value, sizeof(value));
		mValuesLength++;
	}
	else {
		int i;
		for (i = 0; i < mValuesLength; i++) {
			if (strcmp(mValues[i].name, value.name) == 0) {
				//mValues[i].askPrice = value.askPrice;
				//mValues[i].askVolumn = value.askVolumn;
				//mValues[i].bidPrice = value.bidPrice;
				//mValues[i].bidVolumn = value.bidVolumn;
				memcpy(&mValues[i], &value, sizeof(value));
				break;
			}
		}

		if (i == mValuesLength) {
			memcpy(&mValues[mValuesLength], &value, sizeof(value));
			mValuesLength++;
		}
	}

	ReleaseSRWLockExclusive(&mSrwlockDepth);
}

int CTradeInfo::getDepth(char *name, char *value) {
	AcquireSRWLockShared(&mSrwlockDepth);
	//cout << "当前商品信息数: " << mValuesLength << endl;
	for (int i = 0; i < mValuesLength; i++) {
		if (strcmp(mValues[i].name, name) == 0) {

			Document d;
			d.SetObject();
			d.AddMember(rapidjson::StringRef("name"), rapidjson::StringRef(name), d.GetAllocator());
			d.AddMember(rapidjson::StringRef("tradingDate"), rapidjson::StringRef(mValues[i].tradingDate), d.GetAllocator());
			d.AddMember(rapidjson::StringRef("updateTime"), rapidjson::StringRef(mValues[i].updateTime), d.GetAllocator());
			d.AddMember(rapidjson::StringRef("bid"), mValues[i].bidPrice, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("bidvolumn"), mValues[i].bidVolumn, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("ask"), mValues[i].askPrice, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("askvolumn"), mValues[i].askVolumn, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("lastPrice"), mValues[i].lastPrice, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("closePrice"), mValues[i].closePrice, d.GetAllocator());
			
			StringBuffer buffer;
			Writer<StringBuffer> writer(buffer);
			d.Accept(writer);

			const char *json = buffer.GetString();
			//cout << "JSON:" << json << endl;
			memcpy(value, json, strlen(json));
			
			ReleaseSRWLockShared(&mSrwlockDepth);

			return (int)strlen(json);
		}
	}

	ReleaseSRWLockShared(&mSrwlockDepth);
	return 0;
}


// instruments info
void CTradeInfo::saveInstrumentInfo(CThostFtdcInstrumentField *info) {
	// 单线程写入，所以不用保护
	//memcpy(&mInstruments[mInstrumentsLength], info, sizeof(CThostFtdcInstrumentField));
	//mInstrumentsLength++;
	if (info == NULL) {
		//cout << "无法获取有效商品信息" << endl;
		return;
	}

	if (mInstrumentsLength == 0) {
		memcpy(&mInstruments[0], info, sizeof(CThostFtdcInstrumentField));
		mInstrumentsLength++;
	}
	else {
		int i;
		for (i = 0; i < mInstrumentsLength; i++) {
			if (strcmp(mInstruments[i].InstrumentID, info->InstrumentID) == 0) {
				memset(&mInstruments[i].InstrumentID, 0, sizeof(CThostFtdcInstrumentField));
				memcpy(&mInstruments[i], info, sizeof(CThostFtdcInstrumentField));
				break;
			}
		}

		if (i == mInstrumentsLength) {
			memcpy(&mInstruments[i], info, sizeof(CThostFtdcInstrumentField));
			mInstrumentsLength++;
		}
	}

}


int CTradeInfo::getInstrumentInfo(char *name, char *info) {

	for (int i = 0; i < mInstrumentsLength; i++) {
		if (strcmp(mInstruments[i].InstrumentID, name) == 0) {

			Document d;
			d.SetObject();
			d.AddMember(rapidjson::StringRef("InstrumentID"), rapidjson::StringRef(mInstruments[i].InstrumentID), d.GetAllocator());
			d.AddMember(rapidjson::StringRef("ExchangeID"), rapidjson::StringRef(mInstruments[i].ExchangeID), d.GetAllocator());
			//d.AddMember(rapidjson::StringRef("InstrumentName"), rapidjson::StringRef(mInstruments[i].InstrumentName), d.GetAllocator());
			d.AddMember(rapidjson::StringRef("DeliveryYear"), mInstruments[i].DeliveryYear, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("DeliveryMonth"), mInstruments[i].DeliveryMonth, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("VolumeMultiple"), mInstruments[i].VolumeMultiple, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("LongMarginRatio"), mInstruments[i].LongMarginRatio, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("ShortMarginRatio"), mInstruments[i].ShortMarginRatio, d.GetAllocator());

			////cout << "商品编号:" << mInstruments[i].InstrumentID
			////	<< " 商品名称:" << mInstruments[i].InstrumentName
			////	<< " 交割年份:" << mInstruments[i].DeliveryYear << " 交割月份:" << mInstruments[i].DeliveryMonth
			////	<< " 合约乘数:" << mInstruments[i].VolumeMultiple
			////	<< " 做多保证金率:" << mInstruments[i].LongMarginRatio << " 做空保证金率:" << mInstruments[i].ShortMarginRatio << endl;

			StringBuffer buffer;
			Writer<StringBuffer> writer(buffer);
			d.Accept(writer);

			const char *json = buffer.GetString();
			//cout << "JSON:" << json << endl;
			memcpy(info, json, strlen(json));
			return (int)strlen(json);
		}
	}

	return 0;
}

void CTradeInfo::savePositionInfo(CThostFtdcInvestorPositionField *info) {
	if (info == NULL) {
		//cout<< "无法获取持仓信息" <<endl;
		return;
	}
	if (mPositionLength == 0) {
		memcpy(&mPosition[0], info, sizeof(CThostFtdcInvestorPositionField));
		mPositionLength++;
	}
	else {
		int i;
		for (i = 0; i < mPositionLength; i++) {
			if (strcmp(mPosition[i].InstrumentID, info->InstrumentID) == 0) {
				memset(&mPosition[i], 0, sizeof(CThostFtdcInvestorPositionField));
				memcpy(&mPosition[i], info, sizeof(CThostFtdcInvestorPositionField));
				break;
			}
		}

		if (i == mPositionLength) {
			memcpy(&mPosition[i], info, sizeof(CThostFtdcInvestorPositionField));
			mPositionLength++;
		}
	}

}

int CTradeInfo::getPositionInfo(char *name, char *info) {
	for (int i = 0; i < mPositionLength; i++) {
		if (strcmp(mPosition[i].InstrumentID, name) == 0) {

			Document d;
			d.SetObject();
			d.AddMember(rapidjson::StringRef("InstrumentID"), rapidjson::StringRef(mPosition[i].InstrumentID), d.GetAllocator());
			d.AddMember(rapidjson::StringRef("PosiDirection"), mPosition[i].PosiDirection, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("HedgeFlag"), mPosition[i].HedgeFlag, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("PositionDate"), mPosition[i].PositionDate, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("Position"), mPosition[i].Position, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("LongFrozen"), mPosition[i].LongFrozen, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("ShortFrozen"), mPosition[i].ShortFrozen, d.GetAllocator());

			d.AddMember(rapidjson::StringRef("LongFrozenAmount"), mPosition[i].LongFrozenAmount, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("ShortFrozenAmount"), mPosition[i].ShortFrozenAmount, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("OpenVolume"), mPosition[i].OpenVolume, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("CloseVolume"), mPosition[i].CloseVolume, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("OpenAmount"), mPosition[i].OpenAmount, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("CloseAmount"), mPosition[i].CloseAmount, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("PositionCost"), mPosition[i].PositionCost, d.GetAllocator());

			d.AddMember(rapidjson::StringRef("Commission"), mPosition[i].Commission, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("CloseProfit"), mPosition[i].CloseProfit, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("PositionProfit"), mPosition[i].PositionProfit, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("OpenCost"), mPosition[i].OpenCost, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("ExchangeMargin"), mPosition[i].ExchangeMargin, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("TodayPosition"), mPosition[i].TodayPosition, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("MarginRateByMoney"), mPosition[i].MarginRateByMoney, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("MarginRateByVolume"), mPosition[i].MarginRateByVolume, d.GetAllocator());

			//cout << "商品编号:" << mInstruments[i].InstrumentID
			//	<< "商品名称:" << mInstruments[i].InstrumentName
			//	<< "交割年份:" << mInstruments[i].DeliveryYear << "交割月份:" << mInstruments[i].DeliveryMonth
			//	<< "合约乘数:" << mInstruments[i].VolumeMultiple
			//	<< "做多保证金率:" << mInstruments[i].LongMarginRatio << "做空保证金率:" << mInstruments[i].ShortMarginRatio;

			StringBuffer buffer;
			Writer<StringBuffer> writer(buffer);
			d.Accept(writer);

			const char *json = buffer.GetString();
			//cout << "JSON:" << json << endl;
			memcpy(info, json, strlen(json));
			return (int)strlen(json);
		}
	}

	return 0;
}


void CTradeInfo::saveAccountInfo(CThostFtdcTradingAccountField *info) {
	memset(&mAccountInfo, 0, sizeof(CThostFtdcTradingAccountField));
	memcpy(&mAccountInfo, info, sizeof(CThostFtdcTradingAccountField));
}


int CTradeInfo::getAccountInfo(char *info) {


	Document d;
	d.SetObject();
	d.AddMember(rapidjson::StringRef("id"), rapidjson::StringRef(mAccountInfo.AccountID), d.GetAllocator());
	d.AddMember(rapidjson::StringRef("CurrMargin"), mAccountInfo.CurrMargin, d.GetAllocator());
	d.AddMember(rapidjson::StringRef("CashIn"), mAccountInfo.CashIn, d.GetAllocator());
	d.AddMember(rapidjson::StringRef("Commission"), mAccountInfo.Commission, d.GetAllocator());
	d.AddMember(rapidjson::StringRef("CloseProfit"), mAccountInfo.CloseProfit, d.GetAllocator());
	d.AddMember(rapidjson::StringRef("PositionProfit"), mAccountInfo.PositionProfit, d.GetAllocator());
	d.AddMember(rapidjson::StringRef("Balance"), mAccountInfo.Balance, d.GetAllocator());
	d.AddMember(rapidjson::StringRef("Available"), mAccountInfo.Available, d.GetAllocator());
	d.AddMember(rapidjson::StringRef("WithdrawQuota"), mAccountInfo.WithdrawQuota, d.GetAllocator());
	d.AddMember(rapidjson::StringRef("PreBalance"), mAccountInfo.PreBalance, d.GetAllocator());
	

	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	d.Accept(writer);

	const char *json = buffer.GetString();
	//cout << "JSON:" << json << endl;
	memcpy(info, json, strlen(json));
	return (int)strlen(json);

}


bool CTradeInfo::setStatus(int status) {
	if (mStatus == StatusError || mStatus == StatusDisconnect) {
		cout << "连接断开或者发生错误:"<< mError << endl;
		return false;
	}

	if (mStatus == StatusProcess && status==StatusProcess) {
		cout << "消息处理中，请稍后..." << endl;
		return false;
	}

	AcquireSRWLockExclusive(&mSrwlockStatus);
	//cout << "设置状态:" << status << endl;
	mStatus = status;
	ReleaseSRWLockExclusive(&mSrwlockStatus);

	return true;
}

int CTradeInfo::getStatus() {

	AcquireSRWLockShared(&mSrwlockStatus);
	int status = mStatus;
	ReleaseSRWLockShared(&mSrwlockStatus);

	return status;
}

void CTradeInfo::setOrderSysID(TThostFtdcOrderSysIDType orderSysID) 
{
	cout << "orderSysID:" << orderSysID << endl;
	memset(mOrderSysID, 0, sizeof(mOrderSysID));
	memcpy(mOrderSysID, orderSysID, strlen(orderSysID));
}

bool CTradeInfo::isCurrentOrderSysID(TThostFtdcOrderSysIDType orderSysID) {
	cout << "orderSysID:" << orderSysID << " mOrderSysID:"<<mOrderSysID<<endl;
	if (strlen(mOrderSysID) == 0) {
		return false;
	}
	else {
		return strcmp(mOrderSysID, orderSysID)==0?true:false;
	}

}

void CTradeInfo::updateTradeResult(int status, CThostFtdcTradeField *info, char *errorMsg) {
	mTradeResult = status;
	if (status == TradeDone) {
		memcpy(&mTradeInfo, info, sizeof(CThostFtdcTradeField));
	}
	else if (status == TradeError) {
		memcpy(mTradeErrorMsg, errorMsg, strlen(errorMsg));
	}
}

void CTradeInfo::updateTradeInfo(double price, int amount) {
	mTraded[mTradedIndex].price = price;
	mTraded[mTradedIndex].amount = amount;
	mTradedIndex++;
}

void CTradeInfo::clearTradeInfo() {
	mTradedIndex = 0;
	memset(mTraded, 0, sizeof(TradedInfo)*MAX_SIZE);
}

void CTradeInfo::updateQueueInfo(int status, char *ExchangeID, char *OrderSysID) {
	int length = 0;
	mTradeResult = status;
	memcpy(mOrderRef[0], ExchangeID, strlen(ExchangeID));
	memcpy(mOrderRef[1], OrderSysID, strlen(OrderSysID));

}

int CTradeInfo::getTradeResult(char *result) {

	Document d;
	d.SetObject();

	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);


	switch (mTradeResult) {
		case TradeError:
		{
			d.AddMember(rapidjson::StringRef("status"), rapidjson::StringRef("error"), d.GetAllocator());
			d.AddMember(rapidjson::StringRef("message"), rapidjson::StringRef(mTradeErrorMsg), d.GetAllocator());

			d.Accept(writer);

			const char *json = buffer.GetString();
			//cout << "JSON:" << json << endl;
			memcpy(result, json, strlen(json));
			return (int)strlen(json);
		}
		case TradeCancled:
		{
			d.AddMember(rapidjson::StringRef("status"), rapidjson::StringRef("cancel"), d.GetAllocator());

			d.Accept(writer);

			const char *json = buffer.GetString();
			//cout << "JSON:" << json << endl;
			memcpy(result, json, strlen(json));
			return (int)strlen(json);
		}
		case TradeDone:
		{
			int totalAmount = 0;
			double totalCost = 0;
			for (int i = 0; i < mTradedIndex; i++) {
				totalCost += mTraded[i].price*mTraded[i].amount;
				totalAmount += mTraded[i].amount;
			}
			mTradedIndex = 0;
			memset(mTraded, 0, sizeof(TradedInfo)*MAX_SIZE);
			memset(mOrderSysID, 0, sizeof(mOrderSysID));

			d.AddMember(rapidjson::StringRef("status"), rapidjson::StringRef("success"), d.GetAllocator());
			d.AddMember(rapidjson::StringRef("TradeID"), rapidjson::StringRef(mTradeInfo.TradeID), d.GetAllocator());
			d.AddMember(rapidjson::StringRef("TradeDate"), rapidjson::StringRef(mTradeInfo.TradeDate), d.GetAllocator());
			d.AddMember(rapidjson::StringRef("TradeTime"), rapidjson::StringRef(mTradeInfo.TradeTime), d.GetAllocator());
			d.AddMember(rapidjson::StringRef("Price"), totalCost/totalAmount, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("Volume"), totalAmount, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("TradeType"), mTradeInfo.TradeType, d.GetAllocator());

			d.Accept(writer);

			const char *json = buffer.GetString();
			//cout << "JSON:" << json << endl;
			memcpy(result, json, strlen(json));
			return (int)strlen(json);
		}
		case TradeQueued:
		{
			d.AddMember(rapidjson::StringRef("ExchangeID"), rapidjson::StringRef(mOrderRef[0]), d.GetAllocator());
			d.AddMember(rapidjson::StringRef("OrderSysID"), rapidjson::StringRef(mOrderRef[1]), d.GetAllocator());

			d.Accept(writer);

			const char *json = buffer.GetString();
			// cout << "JSON:" << json << endl;
			memcpy(result, json, strlen(json));
			memset(mOrderRef, 0, 2 * 256);


			return (int)strlen(json);
		}
		default:
			cout << "无效的交易状态" << endl;
		}
	return 0;
}


void CTradeInfo::callback(char *errMsg) {
	if (mCallback != NULL) {
		mCallback(errMsg);
	}
}
