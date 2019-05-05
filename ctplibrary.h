#pragma once

#define DLLPROJECT_EXPORTS
#ifdef DLLPROJECT_EXPORTS
#define CTP_API _declspec(dllexport)
#else
#define CTP_API _declspec(dllimport)
#endif


// local functions
void info(const char *prefix, const char *msg);
void debug(const char *prefix, const char *msg);
void error(const char *prefix, const char *msg);

// exported functions
extern "C" CTP_API bool InitMarket();
extern "C" CTP_API bool InitTrade();
extern "C" CTP_API bool CloseMarket();
extern "C" CTP_API bool CloseTrade();
extern "C" CTP_API bool Config(char *config);
extern "C" CTP_API int GetDepth(char *name, char *value);
extern "C" CTP_API int GetInstrumentInfo(char *name, char *info);
extern "C" CTP_API int GetPositionInfo(char *name, char *info);
extern "C" CTP_API int GetBalance(char *info);
extern "C" CTP_API int MarketOpenPosition(char *instrumentID, int volume, int limitPrice, int isBuy, int isMarket, char *result);
extern "C" CTP_API int MarketClosePosition(char *instrumentID, int volume, int limitPrice, int isBuy, int isMarket, char *result);
extern "C" CTP_API int MarketStopPrice(char *instrumentID, int volume, int isBuy, double stopPrice, double limitPrice, char *result);
extern "C" CTP_API int GetStatus();


extern "C" CTP_API int Test(char *echo);