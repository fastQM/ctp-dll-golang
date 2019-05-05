#pragma once
#include ".\ThostTraderApi\ThostFtdcTraderApi.h"

class CTraderSpi : public CThostFtdcTraderSpi
{
public:
	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected();

	///登录请求响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///投资者结算结果确认响应
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///请求查询合约响应
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询资金账户响应
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询投资者持仓响应
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///报单录入请求响应
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///执行宣告录入请求响应
	virtual void OnRspExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///询价录入请求响应
	virtual void OnRspForQuoteInsert(CThostFtdcInputForQuoteField *pInputForQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///报价录入请求响应
	virtual void OnRspQuoteInsert(CThostFtdcInputQuoteField *pInputQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///报单操作请求响应
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///执行宣告操作请求响应
	virtual void OnRspExecOrderAction(CThostFtdcInputExecOrderActionField *pInputExecOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///报价操作请求响应
	virtual void OnRspQuoteAction(CThostFtdcInputQuoteActionField *pInputQuoteAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///错误应答
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	virtual void OnFrontDisconnected(int nReason);
		
	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	virtual void OnHeartBeatWarning(int nTimeLapse);
	
	///报单通知
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

	///执行宣告通知
	virtual void OnRtnExecOrder(CThostFtdcExecOrderField *pExecOrder);

	///询价通知
	virtual void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp);

	///报价通知
	virtual void OnRtnQuote(CThostFtdcQuoteField *pQuote);
	
	///成交通知
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);

public:
	///用户登录请求
	void ReqUserLogin();
	///投资者结算结果确认
	void ReqSettlementInfoConfirm();
	///请求查询合约
	void ReqQryInstrument(char *instrumentID);
	///请求查询资金账户
	void ReqQryTradingAccount();
	///请求查询投资者持仓
	void ReqQryInvestorPosition(char *instrumentID);
	///报单录入请求
	void ReqOrderInsert(char *instrumentID, int volume, bool isBuy, double price);
	// 市价开仓请求
	void ReqMarketOpenInsert(char *instrumentID, int volume, int limitPrice, bool isBuy, bool isMarket);
	// 市价平仓请求
	void ReqMarketCloseInsert(char *instrumentID, int volume, int limitPrice, bool isBuy, bool isMarket);
	// 止损设置
	void ReqMarketStopPriceInsert(char *instrumentID, int volume, bool isBuy, double stopPrice, double limitPrice);
	///执行宣告录入请求
	void ReqExecOrderInsert();
	///询价录入请求
	void ReqForQuoteInsert();
	///报价录入请求
	void ReqQuoteInsert();
	///报单操作请求
	void ReqOrderAction(CThostFtdcOrderField *pOrder);
	///执行宣告操作请求
	void ReqExecOrderAction(CThostFtdcExecOrderField *pExecOrder);
	///报价操作请求
	void ReqQuoteAction(CThostFtdcQuoteField *pQuote);

	// 是否收到成功的响应
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
	// 是否我的报单回报
	bool IsMyOrder(CThostFtdcOrderField *pOrder);
	// 是否我的执行宣告回报
	bool IsMyExecOrder(CThostFtdcExecOrderField *pExecOrder);
	// 是否我的报价
	bool IsMyQuote(CThostFtdcQuoteField *pQuote);
	// 是否正在交易的报单
	bool IsTradingOrder(CThostFtdcOrderField *pOrder);	
	// 是否未撤销的执行宣告
	bool IsTradingExecOrder(CThostFtdcExecOrderField *pExecOrder);
	// 是否未撤销的报价
	bool IsTradingQuote(CThostFtdcQuoteField *pQuote);

	int mIndex;
};