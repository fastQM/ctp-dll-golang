#include "stdafx.h"
#include "ctplibrary.h"
#include <iostream>

#include "rapidjson/rapidjson.h"  
#include "rapidjson/document.h"  
#include "rapidjson/reader.h"  
#include "rapidjson/writer.h"  
#include "rapidjson/stringbuffer.h"  
using namespace rapidjson;

#include ".\ThostTraderApi\ThostFtdcMdApi.h"
#include ".\ThostTraderApi\ThostFtdcTraderApi.h"
#include "MdSpi.h"
#include "TraderSpi.h"
#include "TradeInfo.h"

//https://github.com/Tencent/rapidjson


using namespace std;

// UserApi����
CThostFtdcMdApi* pMarketApi;
// �г��ӿڶ���
CThostFtdcMdSpi* pMarketSpi;

CThostFtdcTraderApi* pTraderApi;
CTraderSpi* pTraderSpi;


// ���ò���
char GlobalMarketAddr[128] = "tcp://180.168.212.228:41213";
char GlobalTradeAddr[128] = "tcp://180.168.212.228:41205";
char GlobalAppID[128];
char GlobalAuthenCode[128];
TThostFtdcBrokerIDType	BROKER_ID = "8080";				// ���͹�˾����
TThostFtdcInvestorIDType INVESTOR_ID = "";			// Ͷ���ߴ���
TThostFtdcPasswordType  PASSWORD = "";			// �û�����
char *test[] = { (char*)"CF809", (char*)"CF901" };
char *GlobalInstruments[1024];
int GlobalInstrumentsNum = 0;									// ���鶩������
int iRequestID = 0;										// ������


// �����Ϣ
CTradeInfo* gTradeInfo;


bool InitMarket() {

	info("InitMarket()", "ENTER");
	// ��ʼ��UserApi
	if (pMarketApi == NULL) {
		if (gTradeInfo == NULL) {
			gTradeInfo = new CTradeInfo(NULL);
		}

		pMarketApi = CThostFtdcMdApi::CreateFtdcMdApi();			// ����UserApi
		pMarketSpi = new CMdSpi();
		pMarketApi->RegisterSpi(pMarketSpi);						// ע���¼���
		pMarketApi->RegisterFront(GlobalMarketAddr);					// connect
		pMarketApi->Init();

		pMarketApi->Join();
		//	pMarketApi->Release();

		return true;
	}

	return false;

}

bool InitTrade() {

	info("InitTrade()", "ENTER");

	if (pTraderApi == NULL) {
		if (gTradeInfo == NULL) {
			gTradeInfo = new CTradeInfo(NULL);
		}

		pTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();			// ����UserApi
		pTraderSpi = new CTraderSpi();
		pTraderApi->RegisterSpi((CThostFtdcTraderSpi*)pTraderSpi);			// ע���¼���
		pTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);				// ע�ṫ����
		pTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);				// ע��˽����
		pTraderApi->RegisterFront(GlobalTradeAddr);							// connect
		pTraderApi->Init();

		pTraderApi->Join();

		return true;
	}

	return false;

}

bool CloseMarket() {
	cout << "Enter:Close()" << endl;

	if (pMarketApi != NULL) {
		pMarketApi->Release();
		pMarketApi = NULL;
	}

	if (pMarketSpi != NULL) {
		delete pMarketSpi;
		pMarketSpi = NULL;
	}

	return true;
}

bool CloseTrade() {

	if (pTraderApi != NULL) {
		pTraderApi->Release();
		pTraderApi = NULL;
	}

	if (pTraderSpi != NULL) {
		delete pTraderSpi;
		pTraderSpi = NULL;
	}

	return true;
}

bool Config(char *config) {
	/*
		marketaddr:
		tradeaddr:
		broker:
		investor:
		password:
		instruments:
	*/
	if (config == NULL) {
		info("Config():", "Invalid Config");
		return false;
	}

	Document doc;
	doc.Parse(config);

	Value& marketaddr = doc["marketaddr"];
	if (marketaddr != NULL) {
		const char * addr = marketaddr.GetString();
		memset(GlobalMarketAddr, 0, 128);
		memcpy(GlobalMarketAddr, addr, strlen(addr));
	}

	info("�����г�����IP��ַ:", GlobalMarketAddr);

	Value& tradeaddr = doc["tradeaddr"];
	if (tradeaddr != NULL) {
		const char *addr = tradeaddr.GetString();
		memset(GlobalTradeAddr, 0, 128);
		memcpy(GlobalTradeAddr, addr, strlen(addr));
	}

	info("���ý���IP��ַ:", GlobalTradeAddr);

	Value& valueBroker = doc["broker"];
	if (valueBroker != NULL) {
		const char *broker = valueBroker.GetString();
		memset(BROKER_ID, 0, 11);
		memcpy(BROKER_ID, broker, strlen(broker));
	}

	info("Broker:", BROKER_ID);

	Value& valueInvestor = doc["investor"];
	if (valueInvestor != NULL) {
		const char *investor = valueInvestor.GetString();
		memset(INVESTOR_ID, 0, 13);
		memcpy(INVESTOR_ID, investor, strlen(investor));
	}

	info("INVESTOR_ID:", INVESTOR_ID);

	Value& valuePwd = doc["password"];
	if (valuePwd != NULL) {
		const char *pwd = valuePwd.GetString();
		memset(PASSWORD, 0, 41);
		memcpy(PASSWORD, pwd, strlen(pwd));
	}

	cout << "Password:" << PASSWORD[0] << "******" << PASSWORD[strlen(PASSWORD)-1] << endl;
	if (!strcmp(PASSWORD, "")) {
		error("", "��Ч����");
		return false;
	}

	Value& valueAppID = doc["appid"];
	if (valueInvestor != NULL) {
		const char *appID = valueAppID.GetString();
		memset(GlobalAppID, 0, 128);
		memcpy(GlobalAppID, appID, strlen(appID));
	}

	cout << "GlobalAppID:" << GlobalAppID[0] << "******" << GlobalAppID[strlen(GlobalAppID) - 1] << endl;

	Value& valueAuthenCode = doc["authencode"];
	if (valueInvestor != NULL) {
		const char *authenCode = valueAuthenCode.GetString();
		memset(GlobalAuthenCode, 0, 128);
		memcpy(GlobalAuthenCode, authenCode, strlen(authenCode));
	}

	cout << "GlobalAuthenCode:" << GlobalAuthenCode[0] << "******" << GlobalAuthenCode[strlen(GlobalAuthenCode) - 1] << endl;

	Value& instruments = doc["instruments"];
	if (instruments != NULL && instruments.IsArray()) {

		GlobalInstrumentsNum = 0;
		memset(GlobalInstruments, 0, sizeof(char*) * 1024);

		int length = instruments.Size();

		for (unsigned int i = 0; i < instruments.Size(); i++) {
			Value & v = instruments[i];
			assert(v.IsObject());
			if (v.HasMember("name") && v["name"].IsString()) {
				const char *name = v["name"].GetString();
				int size = (int)(strlen(name)+1);

				GlobalInstruments[i] = new char[16];
				memset(GlobalInstruments[i], 0, 16);
				memcpy(GlobalInstruments[i], name, strlen(name));

				GlobalInstrumentsNum++;
			}
		}
	}
	else {
		info("Config():", "��Ч����Ʒ����");
	}



	return true;
}

int GetDepth(char *name, char *value) {

	if (gTradeInfo != NULL) {
		return gTradeInfo->getDepth(name, value);
	}

	error("GetDepth", "gTradeInfo is NULL");
	return 0;
}

int GetInstrumentInfo(char *name, char *info) {
	cout << "��ȡ��Ʒ��Ϣ:" << name << endl;
	int counter = 0;
	if (gTradeInfo != NULL) {
		pTraderSpi->ReqQryInstrument(name);
		while (gTradeInfo->getStatus() == StatusProcess) {

			Sleep(100);
			if (counter < 50) {
				counter++;
			}
			else {
				cout << "GetInstrumentInfo()��ʱ..." << endl;
				return 0;
			}
		}
		return gTradeInfo->getInstrumentInfo(name, info);
	}
	error("GetInstrumentInfo", "gTradeInfo is NULL");
	return 0;
}

int GetPositionInfo(char *name, char *info) {
	int counter = 0;
	if (gTradeInfo != NULL) {
		pTraderSpi->ReqQryInvestorPosition(name);
		while (gTradeInfo->getStatus() == StatusProcess) {

			Sleep(100);
			if (counter < 50) {
				counter++;
			}
			else {
				cout << "GetPositionInfo()��ʱ..." << endl;
				return 0;
			}
		}
		return gTradeInfo->getPositionInfo(name, info);
	}
	error("GetPositionInfo", "gTradeInfo is NULL");
	return 0;
}

int GetBalance(char *info) {
	int counter = 0;
	if (gTradeInfo != NULL) {
		pTraderSpi->ReqQryTradingAccount();
		while (gTradeInfo->getStatus() == StatusProcess) {

			Sleep(100);
			if (counter < 50) {
				counter++;
			}
			else {
				cout << "GetBalance()��ʱ..." << endl;
				return 0;
			}
		}
		return gTradeInfo->getAccountInfo(info);

	}
	error("GetBalance", "gTradeInfo is NULL");
	return 0;
}


int MarketOpenPosition(char *instrumentID, int volume, int limitPrice, int isBuy, int isMarket, char *result) {
	cout << "��������:" << instrumentID << " ��������:" << volume << " ���ּ۸�:" << limitPrice << " ����:"
		<< isBuy << "�м�:" << isMarket << endl;;
	int counter = 0;
	if (gTradeInfo != NULL) {
		bool buyFlag = false;
		bool marketFlag = false;

		if (isBuy > 0) {
			buyFlag = true;
		}

		if (isMarket > 0) {
			marketFlag = true;
		}

		gTradeInfo->clearTradeInfo();

		pTraderSpi->ReqMarketOpenInsert(instrumentID, volume, limitPrice, buyFlag, marketFlag);
		int status = gTradeInfo->getStatus();
		while (status == StatusProcess||status==StatusAllTraded) {
			status = gTradeInfo->getStatus();
			Sleep(100);
			if (counter < 300) {
				counter++;
			}
			else {
				cout << "MarketOpenPosition()��ʱ...״̬:" <<status<< endl;
				return 0;
			}
		}
		return gTradeInfo->getTradeResult(result);
	}

	error("MarketOpenPosition", "gTradeInfo is NULL");
	return 0;
}

int CancelOrder(char *instrumentID, char *exchangeID, char *orderSysID, char *result) {
	cout << "��������:" << instrumentID << " ����������:" << exchangeID << " ��������:" << orderSysID << endl;;
	int counter = 0;
	if (gTradeInfo != NULL) {
		pTraderSpi->ReqCancelOrder(instrumentID, exchangeID, orderSysID);
		int status = gTradeInfo->getStatus();
		while (status == StatusProcess || status == StatusAllTraded) {
			status = gTradeInfo->getStatus();
			Sleep(100);
			if (counter < 300) {
				counter++;
			}
			else {
				cout << "CancelOrder()��ʱ...״̬:" << status << endl;
				return 0;
			}
		}
		return gTradeInfo->getTradeResult(result);
	}

	error("CancelOrder", "gTradeInfo is NULL");
	return 0;
}


int MarketClosePosition(char *instrumentID, int volume, int limitPrice, int isBuy, int isMarket, int isToday, char *result) {
	cout << "ƽ������:" << instrumentID << " ƽ������:" << volume << " ƽ�ּ۸�:"<< limitPrice << " ����:" 
		<< isBuy << " �м�:" << isMarket << "ƽ��:" << isToday << endl;
	int counter = 0;
	if (gTradeInfo != NULL) {
		bool buyFlag = false;
		bool marketFlag = false;
		bool todayFlag = false;
		if (isBuy > 0) {
			buyFlag = true;
		}
		if (isMarket > 0) {
			marketFlag = true;
		}
		if (isToday > 0) {
			todayFlag = true;
		}

		gTradeInfo->clearTradeInfo();

		pTraderSpi->ReqMarketCloseInsert(instrumentID, volume, limitPrice, buyFlag, marketFlag, todayFlag);
		int status = gTradeInfo->getStatus();
		while (status == StatusProcess || status == StatusAllTraded) {
			status = gTradeInfo->getStatus();
			Sleep(100);
			if (counter < 300) {
				counter++;
			}
			else {
				cout << "MarketClosePosition()��ʱ..." << endl;
				return 0;
			}
		}
		return gTradeInfo->getTradeResult(result);
	}

	error("MarketClosePosition", "gTradeInfo is NULL");
	return 0;
}

int MarketStopPrice(char *instrumentID, int volume, int isBuy, double stopPrice, double limitPrice, char *result) {
	cout << "ֹ������:" << instrumentID << " ��������:" << volume << " ����:" << isBuy  
		<< " ֹ��۸�:" << stopPrice << " �µ��۸�:" << limitPrice << endl;
	int counter = 0;
	if (gTradeInfo != NULL) {
		bool buyFlag = false;
		if (isBuy > 0) {
			buyFlag = true;
		}
		gTradeInfo->clearTradeInfo();

		pTraderSpi->ReqMarketStopPriceInsert(instrumentID, volume, buyFlag, stopPrice, limitPrice);
		int status = gTradeInfo->getStatus();
		while (status == StatusProcess || status == StatusAllTraded) {
			status = gTradeInfo->getStatus();
			Sleep(100);
			if (counter < 300) {
				counter++;
			}
			else {
				cout << "MarketStopPrice()��ʱ..." << endl;
				return 0;
			}
		}
		return gTradeInfo->getTradeResult(result);
	}

	error("MarketStopPrice", "gTradeInfo is NULL");
	return 0;
}

int GetStatus() {
	if (gTradeInfo != NULL) {
		return gTradeInfo->getStatus();
	}

	error("MarketClosePosition", "gTradeInfo is NULL");
	return -1;
}


int Test(char *echo) {
	cout << "Test:" << echo << endl;

	const char* str = "{\"name\":\"xiaoming\",\"age\":18,\"job\":\"coder\",\"a\":{\"b\":1}}";

	Document doc;
	doc.Parse(str);

	Value& s = doc["age"];
	s.SetInt(s.GetInt() + 1);

	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	doc.Accept(writer);

	const char * result = buffer.GetString();
	cout << "String:" << result << endl;

	memcpy(echo, result, strlen(result));

	return (int)strlen(result);
}

void info(const char *prefix, const char *msg) {
	cout << "[INFO]"<< prefix << msg << endl;
}
void debug(const char *prefix, const char *msg) {
	cout << "[DEBUG]" << prefix << msg << endl;
}

void error(const char *prefix, const char *msg) {
	cout << "[ERROR]" << prefix << msg << endl;
}