#include "stdafx.h"
#include "TradeInfo.h"

#include "rapidjson/rapidjson.h"  
#include "rapidjson/document.h"  
#include "rapidjson/reader.h"  
#include "rapidjson/writer.h"  
#include "rapidjson/stringbuffer.h"  
using namespace rapidjson;


CTradeInfo::CTradeInfo(CBFunction cb) {
	mStatus = StatusNone;
	mValuesLength = 0;
	mCallback = cb;
	memset(mValues, 0, sizeof(DepthValue) * MAX_SIZE);
	memset(mInstruments, 0, sizeof(CThostFtdcInstrumentField) * MAX_SIZE);
	InitializeSRWLock(&mSrwlockDepth);
	InitializeSRWLock(&mSrwlockStatus);
}

CTradeInfo::~CTradeInfo(){

}


void CTradeInfo::updateDepth(DepthValue value) {

	AcquireSRWLockExclusive(&mSrwlockDepth);
	cout << "[ID]" << value.name << "[ASK]" << value.askPrice << "[ASKVN]" << value.askVolumn << "[BID]" << value.bidPrice << "[BIDVN]" << value.bidVolumn << endl;

	if (mValuesLength == 0) {
		memcpy(&mValues[0], &value, sizeof(value));
		mValuesLength++;
	}
	else {
		int i;
		for (i = 0; i < mValuesLength; i++) {
			if (strcmp(mValues[i].name, value.name) == 0) {
				mValues[i].askPrice = value.askPrice;
				mValues[i].askVolumn = value.askVolumn;
				mValues[i].bidPrice = value.bidPrice;
				mValues[i].bidVolumn = value.bidVolumn;
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
	cout << "当前商品信息数: " << mValuesLength << endl;
	for (int i = 0; i < mValuesLength; i++) {
		if (strcmp(mValues[i].name, name) == 0) {

			Document d;
			d.SetObject();
			d.AddMember(rapidjson::StringRef("name"), rapidjson::StringRef(name), d.GetAllocator());
			d.AddMember(rapidjson::StringRef("bid"), mValues[i].bidPrice, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("bidvolumn"), mValues[i].bidVolumn, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("ask"), mValues[i].askPrice, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("askvolumn"), mValues[i].askVolumn, d.GetAllocator());
			
			StringBuffer buffer;
			Writer<StringBuffer> writer(buffer);
			d.Accept(writer);

			const char *json = buffer.GetString();
			//cout << "JSON:" << json << endl;
			memcpy(value, json, strlen(json));
			
			ReleaseSRWLockShared(&mSrwlockDepth);

			return strlen(json);
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
			d.AddMember(rapidjson::StringRef("InstrumentName"), rapidjson::StringRef(mInstruments[i].InstrumentName), d.GetAllocator());
			d.AddMember(rapidjson::StringRef("DeliveryYear"), mInstruments[i].DeliveryYear, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("DeliveryMonth"), mInstruments[i].DeliveryMonth, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("VolumeMultiple"), mInstruments[i].VolumeMultiple, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("LongMarginRatio"), mInstruments[i].LongMarginRatio, d.GetAllocator());
			d.AddMember(rapidjson::StringRef("ShortMarginRatio"), mInstruments[i].ShortMarginRatio, d.GetAllocator());

			cout << "商品编号:" << mInstruments[i].InstrumentID
				<< "商品名称:" << mInstruments[i].InstrumentName
				<< "交割年份:" << mInstruments[i].DeliveryYear << "交割月份:" << mInstruments[i].DeliveryMonth
				<< "合约乘数:" << mInstruments[i].VolumeMultiple
				<< "做多保证金率:" << mInstruments[i].LongMarginRatio << "做空保证金率:" << mInstruments[i].ShortMarginRatio;

			StringBuffer buffer;
			Writer<StringBuffer> writer(buffer);
			d.Accept(writer);

			const char *json = buffer.GetString();
			//cout << "JSON:" << json << endl;
			memcpy(info, json, strlen(json));
			return strlen(json);
		}
	}

	return 0;
}

void CTradeInfo::savePositionInfo(CThostFtdcInvestorPositionField *info) {

}

int CTradeInfo::getPositionInfo(char *info) {
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
	d.AddMember(rapidjson::StringRef("balance"), mAccountInfo.Balance, d.GetAllocator());
	d.AddMember(rapidjson::StringRef("available"), mAccountInfo.Available, d.GetAllocator());

	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	d.Accept(writer);

	const char *json = buffer.GetString();
	//cout << "JSON:" << json << endl;
	memcpy(info, json, strlen(json));
	return strlen(json);

}


bool CTradeInfo::setStatus(int status) {
	if (mStatus == StatusError) {
		cout << "请先处理错误状态:"<< mError<<endl;
		return false;
	}

	if (mStatus == StatusProcess && status==StatusProcess) {
		cout << "消息处理中，请稍后..." << endl;
		return false;
	}

	AcquireSRWLockExclusive(&mSrwlockStatus);
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


void CTradeInfo::callback(char *errMsg) {
	if (mCallback != NULL) {
		mCallback(errMsg);
	}
}