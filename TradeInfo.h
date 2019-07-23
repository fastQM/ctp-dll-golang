#pragma once
#include<windows.h>
#include ".\ThostTraderApi\ThostFtdcTraderApi.h"

#define MAX_SIZE 1024

enum {
	StatusNone = 0,
	StatusReady,
	StatusProcess,
	StatusAllTraded,
	StatusDone,
	StatusDisconnect,
	StatusError,
};

enum {
	TradeNone = 0,
	TradeError,
	TradeCancled,
	TradeDone,
	TradeQueued,
};

struct DepthValue {
	char name[16];
	double bidPrice;
	double bidVolumn;
	double askPrice;
	double askVolumn;

	double lastPrice;
	double closePrice;
	char tradingDate[9];
	char updateTime[9];
};

struct TradedInfo {
	double price;
	int amount;
};


typedef void(*CBFunction)(char* errMsg);

class CTradeInfo{
	public:
		CTradeInfo(CBFunction cb);
		~CTradeInfo();

		// Depth info，支持多线程
		void updateDepth(DepthValue values);
		int getDepth(char *name, char *value);
		
		// instruments info，不支持多线程
		void saveInstrumentInfo(CThostFtdcInstrumentField *info);
		int getInstrumentInfo(char *name, char *info);

		// account info，不支持多线程
		void saveAccountInfo(CThostFtdcTradingAccountField *info);
		int getAccountInfo(char *info);

		// position info，不支持多线程
		void savePositionInfo(CThostFtdcInvestorPositionField *info);
		int getPositionInfo(char *name, char *info);
		
		// 获取交易结果，不支持多线程
		void updateTradeResult(int status, CThostFtdcTradeField *info, char *errorMsg);
		void updateTradeInfo(double price, int amount);
		void updateQueueInfo(int status, char *ExchangeID, char *OrderSysID);
		int getTradeResult(char *result);

		void setOrderSysID(TThostFtdcOrderSysIDType orderSysID);
		bool isCurrentOrderSysID(TThostFtdcOrderSysIDType orderSysID);

		bool setStatus(int status);
		int getStatus();
		void callback(char *errMsg);

	private:
		DepthValue mValues[MAX_SIZE];
		int mValuesLength;

		CThostFtdcInstrumentField mInstruments[MAX_SIZE];
		int mInstrumentsLength;

		CThostFtdcInvestorPositionField mPosition[MAX_SIZE];
		int mPositionLength;

		CThostFtdcTradingAccountField mAccountInfo;

		int mStatus;
		char mError[1028];

		int mTradeResult;
		CThostFtdcTradeField mTradeInfo;
		TradedInfo mTraded[1024];
		int mTradedIndex;
		char mTradeErrorMsg[1024];

		char mOrderRef[2][256];
		char mOrderSysID[21];

		SRWLOCK mSrwlockDepth;
		SRWLOCK mSrwlockStatus;

		CBFunction mCallback;

};