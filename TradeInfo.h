#pragma once
#include<windows.h>
#include ".\ThostTraderApi\ThostFtdcTraderApi.h"

#define MAX_SIZE 1024

enum {
	StatusNone = 0,
	StatusProcess,
	StatusDone,
	StatusError,
};

struct DepthValue {
	char name[16];
	double bidPrice;
	double bidVolumn;
	double askPrice;
	double askVolumn;
};


typedef void(*CBFunction)(char* errMsg);

class CTradeInfo{
	public:
		CTradeInfo(CBFunction cb);
		~CTradeInfo();

		// Depth info
		void updateDepth(DepthValue values);
		int getDepth(char *name, char *value);
		
		// instruments info
		void saveInstrumentInfo(CThostFtdcInstrumentField *info);
		int getInstrumentInfo(char *name, char *info);

		// account info
		void saveAccountInfo(CThostFtdcTradingAccountField *info);
		int getAccountInfo(char *info);

		// position info
		void savePositionInfo(CThostFtdcInvestorPositionField *info);
		int getPositionInfo(char *info);
		

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

		SRWLOCK mSrwlockDepth;
		SRWLOCK mSrwlockStatus;

		CBFunction mCallback;

};