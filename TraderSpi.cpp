#include <windows.h>
#include <iostream>
using namespace std;

#include ".\ThostTraderApi\ThostFtdcTraderApi.h"
#include "TraderSpi.h"
#include "TradeInfo.h"

#pragma warning(disable : 4996)

// USER_API参数
extern CThostFtdcTraderApi* pTraderApi;

// 配置参数
extern char BROKER_ID[];		// 经纪公司代码
extern char INVESTOR_ID[];		// 投资者代码
extern char PASSWORD[];			// 用户密码
//extern char INSTRUMENT_ID[];	// 合约代码
extern char *GlobalInstruments[];
extern int GlobalInstrumentsNum;

// 请求编号
extern int iRequestID;
extern CTradeInfo* gTradeInfo;

// 会话参数
TThostFtdcFrontIDType	FRONT_ID;	//前置编号
TThostFtdcSessionIDType	SESSION_ID;	//会话编号
TThostFtdcOrderRefType	ORDER_REF;	//报单引用
TThostFtdcOrderRefType	EXECORDER_REF;	//执行宣告引用
TThostFtdcOrderRefType	FORQUOTE_REF;	//询价引用
TThostFtdcOrderRefType	QUOTE_REF;	//报价引用

// 流控判断
bool IsFlowControl(int iResult)
{
	return ((iResult == -2) || (iResult == -3));
}

void CTraderSpi::OnFrontConnected()
{
	cerr << "--->>> " << "OnFrontConnected" << endl;
	///用户登录请求
	ReqUserLogin();
}

void CTraderSpi::ReqUserLogin()
{
	mIndex = 0;
	if (!gTradeInfo->setStatus(StatusProcess)) {
		return;
	}
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.UserID, INVESTOR_ID);
	strcpy(req.Password, PASSWORD);
	int iResult = pTraderApi->ReqUserLogin(&req, ++iRequestID);
	cerr << "--->>> 发送用户登录请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
}

void CTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspUserLogin" << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		// 保存会话参数
		FRONT_ID = pRspUserLogin->FrontID;
		SESSION_ID = pRspUserLogin->SessionID;
		int iNextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		iNextOrderRef++;
		sprintf(ORDER_REF, "%d", iNextOrderRef);
		sprintf(EXECORDER_REF, "%d", 1);
		sprintf(FORQUOTE_REF, "%d", 1);
		sprintf(QUOTE_REF, "%d", 1);
		///获取当前交易日
		cerr << "--->>> 获取当前交易日 = " << pTraderApi->GetTradingDay() << endl;
		///投资者结算结果确认
		ReqSettlementInfoConfirm();
	}
}

void CTraderSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVESTOR_ID);
	int iResult = pTraderApi->ReqSettlementInfoConfirm(&req, ++iRequestID);
	cerr << "--->>> 投资者结算结果确认: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
}

void CTraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspSettlementInfoConfirm" << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		// 获取账户数据
		//ReqQryTradingAccount();
		gTradeInfo->setStatus(StatusDone);
	}
}

void CTraderSpi::ReqQryInstrument(char *instrumentID)
{
	if (!gTradeInfo->setStatus(StatusProcess)) {
		return;
	}

	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	memcpy(req.InstrumentID, instrumentID, strlen(instrumentID));
	while (true)
	{
		int iResult = pTraderApi->ReqQryInstrument(&req, ++iRequestID);
		if (!IsFlowControl(iResult))
		{
			cerr << "--->>> 请求查询合约: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
			break;
		}
		else
		{
			cerr << "--->>> 请求查询合约: " << iResult << ", 受到流控" << endl;
			Sleep(1000);
		}
	} // while


}

void CTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspQryInstrument" << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		gTradeInfo->saveInstrumentInfo(pInstrument);
		gTradeInfo->setStatus(StatusDone);

	}
}

void CTraderSpi::ReqQryTradingAccount()
{

	if (!gTradeInfo->setStatus(StatusProcess)) {
		return;
	}

	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVESTOR_ID);
	while (true)
	{
		int iResult = pTraderApi->ReqQryTradingAccount(&req, ++iRequestID);
		if (!IsFlowControl(iResult))
		{
			cerr << "--->>> 请求查询资金账户: "  << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
			break;
		}
		else
		{
			cerr << "--->>> 请求查询资金账户: "  << iResult << ", 受到流控" << endl;
			Sleep(1000);
		}
	} // while
}

void CTraderSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspQryTradingAccount" << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		gTradeInfo->saveAccountInfo(pTradingAccount);
		gTradeInfo->setStatus(StatusDone);

	}
}

void CTraderSpi::ReqQryInvestorPosition(char *instrumentID)
{
	if (!gTradeInfo->setStatus(StatusProcess)) {
		return;
	}

	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVESTOR_ID);
	strcpy(req.InstrumentID, instrumentID);

	while (true)
	{
		int iResult = pTraderApi->ReqQryInvestorPosition(&req, ++iRequestID);
		if (!IsFlowControl(iResult))
		{
			cerr << "--->>> 请求查询投资者持仓: "  << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
			break;
		}
		else
		{
			cerr << "--->>> 请求查询投资者持仓: "  << iResult << ", 受到流控" << endl;
			Sleep(1000);
		}
	} // while
}

void CTraderSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspQryInvestorPosition" << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///报单录入请求
		//ReqOrderInsert();
		////执行宣告录入请求
		//ReqExecOrderInsert();
		////询价录入
		//ReqForQuoteInsert();
		////做市商报价录入
		//ReqQuoteInsert();
		gTradeInfo->savePositionInfo(pInvestorPosition);
		gTradeInfo->setStatus(StatusDone);
	}
}

void CTraderSpi::ReqMarketOpenInsert(char *instrumentID, int volume, bool isBuy)
{
	if (!gTradeInfo->setStatus(StatusProcess)) {
		return;
	}

	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));
	///经纪公司代码
	strcpy(req.BrokerID, BROKER_ID);
	///投资者代码
	strcpy(req.InvestorID, INVESTOR_ID);
	///合约代码
	strcpy(req.InstrumentID, instrumentID);
	///报单引用
	strcpy(req.OrderRef, ORDER_REF);
	///用户代码
	//	TThostFtdcUserIDType	UserID;
	///报单价格条件: 限价
	req.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
	///买卖方向: 
	if (isBuy) {
		req.Direction = THOST_FTDC_D_Buy;
	}
	else {
		req.Direction = THOST_FTDC_D_Sell;
	}
	///组合开平标志: 开仓
	req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	///组合投机套保标志
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	///价格
	req.LimitPrice = 0;
	///数量: 1
	req.VolumeTotalOriginal = volume;
	///有效期类型: 当日有效
	req.TimeCondition = THOST_FTDC_TC_IOC;
	///GTD日期
	//	TThostFtdcDateType	GTDDate;
	///成交量类型: 任何数量
	req.VolumeCondition = THOST_FTDC_VC_AV;
	///最小成交量: 1
	req.MinVolume = 1;
	///触发条件: 立即
	req.ContingentCondition = THOST_FTDC_CC_Immediately;
	///止损价
	//	TThostFtdcPriceType	StopPrice;
	///强平原因: 非强平
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	///自动挂起标志: 否
	req.IsAutoSuspend = 0;
	///业务单元
	//	TThostFtdcBusinessUnitType	BusinessUnit;
	///请求编号
	//	TThostFtdcRequestIDType	RequestID;
	///用户强评标志: 否
	req.UserForceClose = 0;

	int iResult = pTraderApi->ReqOrderInsert(&req, ++iRequestID);
	cerr << "--->>> 市价开仓录入请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
}

void CTraderSpi::ReqMarketCloseInsert(char *instrumentID, int volume, bool isBuy)
{

	if (!gTradeInfo->setStatus(StatusProcess)) {
		return;
	}


	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));
	///经纪公司代码
	strcpy(req.BrokerID, BROKER_ID);
	///投资者代码
	strcpy(req.InvestorID, INVESTOR_ID);
	///合约代码
	strcpy(req.InstrumentID, instrumentID);
	///报单引用
	strcpy(req.OrderRef, ORDER_REF);
	///用户代码
	//	TThostFtdcUserIDType	UserID;
	///报单价格条件: 限价
	req.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
	///买卖方向: 
	if (isBuy) {
		req.Direction = THOST_FTDC_D_Buy;
	}
	else {
		req.Direction = THOST_FTDC_D_Sell;
	}
	///组合开平标志: 平仓
	req.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
	///组合投机套保标志
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	///价格
	req.LimitPrice = 0;
	///数量: 1
	req.VolumeTotalOriginal = volume;
	///有效期类型: 当日有效
	req.TimeCondition = THOST_FTDC_TC_IOC;
	///GTD日期
	//	TThostFtdcDateType	GTDDate;
	///成交量类型: 任何数量
	req.VolumeCondition = THOST_FTDC_VC_AV;
	///最小成交量: 1
	req.MinVolume = volume;
	///触发条件: 立即
	req.ContingentCondition = THOST_FTDC_CC_Immediately;
	///止损价
	//	TThostFtdcPriceType	StopPrice;
	///强平原因: 非强平
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	///自动挂起标志: 否
	req.IsAutoSuspend = 0;
	///业务单元
	//	TThostFtdcBusinessUnitType	BusinessUnit;
	///请求编号
	//	TThostFtdcRequestIDType	RequestID;
	///用户强评标志: 否
	req.UserForceClose = 0;

	int iResult = pTraderApi->ReqOrderInsert(&req, ++iRequestID);
	cerr << "--->>> 市价平仓录入请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
}


void CTraderSpi::ReqOrderInsert(char *instrumentID, int volume, bool isBuy, double price)
{
	if (!gTradeInfo->setStatus(StatusProcess)) {
		return;
	}

	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));
	///经纪公司代码
	strcpy(req.BrokerID, BROKER_ID);
	///投资者代码
	strcpy(req.InvestorID, INVESTOR_ID);
	///合约代码
	//strcpy(req.InstrumentID, INSTRUMENT_ID);
	///报单引用
	strcpy(req.OrderRef, ORDER_REF);
	///用户代码
//	TThostFtdcUserIDType	UserID;
	///报单价格条件: 限价
	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	///买卖方向: 
	if (isBuy) {
		req.Direction = THOST_FTDC_D_Buy;
	}
	else {
		req.Direction = THOST_FTDC_D_Sell;
	}
	///组合开平标志: 开仓
	req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	///组合投机套保标志
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	///价格
	req.LimitPrice = price;
	///数量: 1
	req.VolumeTotalOriginal = volume;
	///有效期类型: 当日有效
	req.TimeCondition = THOST_FTDC_TC_GFD;
	///GTD日期
//	TThostFtdcDateType	GTDDate;
	///成交量类型: 任何数量
	req.VolumeCondition = THOST_FTDC_VC_AV;
	///最小成交量: 1
	req.MinVolume = 1;
	///触发条件: 立即
	req.ContingentCondition = THOST_FTDC_CC_Immediately;
	///止损价
//	TThostFtdcPriceType	StopPrice;
	///强平原因: 非强平
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	///自动挂起标志: 否
	req.IsAutoSuspend = 0;
	///业务单元
//	TThostFtdcBusinessUnitType	BusinessUnit;
	///请求编号
//	TThostFtdcRequestIDType	RequestID;
	///用户强评标志: 否
	req.UserForceClose = 0;

	int iResult = pTraderApi->ReqOrderInsert(&req, ++iRequestID);
	cerr << "--->>> 报单录入请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
}

//执行宣告录入请求
void CTraderSpi::ReqExecOrderInsert()
{
	CThostFtdcInputExecOrderField req;
	memset(&req, 0, sizeof(req));
	///经纪公司代码
	strcpy(req.BrokerID, BROKER_ID);
	///投资者代码
	strcpy(req.InvestorID, INVESTOR_ID);
	///合约代码
	//strcpy(req.InstrumentID, INSTRUMENT_ID);
	///报单引用
	strcpy(req.ExecOrderRef, EXECORDER_REF);
	///用户代码
	//	TThostFtdcUserIDType	UserID;
	///数量
	req.Volume=1;
	///请求编号
	//TThostFtdcRequestIDType	RequestID;
	///业务单元
	//TThostFtdcBusinessUnitType	BusinessUnit;
	///开平标志
	req.OffsetFlag=THOST_FTDC_OF_Close;//如果是上期所，需要填平今或平昨
	///投机套保标志
	req.HedgeFlag=THOST_FTDC_HF_Speculation;
	///执行类型
	req.ActionType=THOST_FTDC_ACTP_Exec;//如果放弃执行则填THOST_FTDC_ACTP_Abandon
	///保留头寸申请的持仓方向
	req.PosiDirection=THOST_FTDC_PD_Long;
	///期权行权后是否保留期货头寸的标记
	req.ReservePositionFlag=THOST_FTDC_EOPF_UnReserve;//这是中金所的填法，大商所郑商所填THOST_FTDC_EOPF_Reserve
	///期权行权后生成的头寸是否自动平仓
	req.CloseFlag=THOST_FTDC_EOCF_AutoClose;//这是中金所的填法，大商所郑商所填THOST_FTDC_EOCF_NotToClose

	int iResult = pTraderApi->ReqExecOrderInsert(&req, ++iRequestID);
	cerr << "--->>> 执行宣告录入请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
}

//询价录入请求
void CTraderSpi::ReqForQuoteInsert()
{
	CThostFtdcInputForQuoteField req;
	memset(&req, 0, sizeof(req));
	///经纪公司代码
	strcpy(req.BrokerID, BROKER_ID);
	///投资者代码
	strcpy(req.InvestorID, INVESTOR_ID);
	///合约代码
	//strcpy(req.InstrumentID, INSTRUMENT_ID);
	///报单引用
	strcpy(req.ForQuoteRef, EXECORDER_REF);
	///用户代码
	//	TThostFtdcUserIDType	UserID;

	int iResult = pTraderApi->ReqForQuoteInsert(&req, ++iRequestID);
	cerr << "--->>> 询价录入请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
}
//报价录入请求
void CTraderSpi::ReqQuoteInsert()
{
	CThostFtdcInputQuoteField req;
	memset(&req, 0, sizeof(req));
	///经纪公司代码
	strcpy(req.BrokerID, BROKER_ID);
	///投资者代码
	strcpy(req.InvestorID, INVESTOR_ID);
	///合约代码
	//strcpy(req.InstrumentID, INSTRUMENT_ID);
	///报单引用
	strcpy(req.QuoteRef, QUOTE_REF);
	///卖价格
	//req.AskPrice=LIMIT_PRICE; // by mqiu20180526
	///买价格
	//req.BidPrice=LIMIT_PRICE-1.0; // by mqiu20180526
	///卖数量
	req.AskVolume=1;
	///买数量
	req.BidVolume=1;
	///请求编号
	//TThostFtdcRequestIDType	RequestID;
	///业务单元
	//TThostFtdcBusinessUnitType	BusinessUnit;
	///卖开平标志
	req.AskOffsetFlag=THOST_FTDC_OF_Open;
	///买开平标志
	req.BidOffsetFlag=THOST_FTDC_OF_Open;
	///卖投机套保标志
	req.AskHedgeFlag=THOST_FTDC_HF_Speculation;
	///买投机套保标志
	req.BidHedgeFlag=THOST_FTDC_HF_Speculation;
	
	int iResult = pTraderApi->ReqQuoteInsert(&req, ++iRequestID);
	cerr << "--->>> 报价录入请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
}

void CTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspOrderInsert" << endl;
	IsErrorRspInfo(pRspInfo);
	gTradeInfo->updateTradeResult(TradeError, NULL, pRspInfo->ErrorMsg);
	gTradeInfo->setStatus(StatusDone);
}

void CTraderSpi::OnRspExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//如果执行宣告正确，则不会进入该回调
	cerr << "--->>> " << "OnRspExecOrderInsert" << endl;
	IsErrorRspInfo(pRspInfo);
}

void CTraderSpi::OnRspForQuoteInsert(CThostFtdcInputForQuoteField *pInputForQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//如果询价正确，则不会进入该回调
	cerr << "--->>> " << "OnRspForQuoteInsert" << endl;
	IsErrorRspInfo(pRspInfo);
}

void CTraderSpi::OnRspQuoteInsert(CThostFtdcInputQuoteField *pInputQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//如果报价正确，则不会进入该回调
	cerr << "--->>> " << "OnRspQuoteInsert" << endl;
	IsErrorRspInfo(pRspInfo);
}

void CTraderSpi::ReqOrderAction(CThostFtdcOrderField *pOrder)
{
	static bool ORDER_ACTION_SENT = false;		//是否发送了报单
	if (ORDER_ACTION_SENT)
		return;

	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	///经纪公司代码
	strcpy(req.BrokerID, pOrder->BrokerID);
	///投资者代码
	strcpy(req.InvestorID, pOrder->InvestorID);
	///报单操作引用
//	TThostFtdcOrderActionRefType	OrderActionRef;
	///报单引用
	strcpy(req.OrderRef, pOrder->OrderRef);
	///请求编号
//	TThostFtdcRequestIDType	RequestID;
	///前置编号
	req.FrontID = FRONT_ID;
	///会话编号
	req.SessionID = SESSION_ID;
	///交易所代码
//	TThostFtdcExchangeIDType	ExchangeID;
	///报单编号
//	TThostFtdcOrderSysIDType	OrderSysID;
	///操作标志
	req.ActionFlag = THOST_FTDC_AF_Delete;
	///价格
//	TThostFtdcPriceType	LimitPrice;
	///数量变化
//	TThostFtdcVolumeType	VolumeChange;
	///用户代码
//	TThostFtdcUserIDType	UserID;
	///合约代码
	strcpy(req.InstrumentID, pOrder->InstrumentID);

	int iResult = pTraderApi->ReqOrderAction(&req, ++iRequestID);
	cerr << "--->>> 报单操作请求: "  << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
	ORDER_ACTION_SENT = true;
}

void CTraderSpi::ReqExecOrderAction(CThostFtdcExecOrderField *pExecOrder)
{
	static bool EXECORDER_ACTION_SENT = false;		//是否发送了报单
	if (EXECORDER_ACTION_SENT)
		return;

	CThostFtdcInputExecOrderActionField req;
	memset(&req, 0, sizeof(req));

	///经纪公司代码
	strcpy(req.BrokerID,pExecOrder->BrokerID);
	///投资者代码
	strcpy(req.InvestorID,pExecOrder->InvestorID);
	///执行宣告操作引用
	//TThostFtdcOrderActionRefType	ExecOrderActionRef;
	///执行宣告引用
	strcpy(req.ExecOrderRef,pExecOrder->ExecOrderRef);
	///请求编号
	//TThostFtdcRequestIDType	RequestID;
	///前置编号
	req.FrontID=FRONT_ID;
	///会话编号
	req.SessionID=SESSION_ID;
	///交易所代码
	//TThostFtdcExchangeIDType	ExchangeID;
	///执行宣告操作编号
	//TThostFtdcExecOrderSysIDType	ExecOrderSysID;
	///操作标志
	req.ActionFlag=THOST_FTDC_AF_Delete;
	///用户代码
	//TThostFtdcUserIDType	UserID;
	///合约代码
	strcpy(req.InstrumentID,pExecOrder->InstrumentID);

	int iResult = pTraderApi->ReqExecOrderAction(&req, ++iRequestID);
	cerr << "--->>> 执行宣告操作请求: "  << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
	EXECORDER_ACTION_SENT = true;
}

void CTraderSpi::ReqQuoteAction(CThostFtdcQuoteField *pQuote)
{
	static bool QUOTE_ACTION_SENT = false;		//是否发送了报单
	if (QUOTE_ACTION_SENT)
		return;

	CThostFtdcInputQuoteActionField req;
	memset(&req, 0, sizeof(req));
	///经纪公司代码
	strcpy(req.BrokerID, pQuote->BrokerID);
	///投资者代码
	strcpy(req.InvestorID, pQuote->InvestorID);
	///报价操作引用
	//TThostFtdcOrderActionRefType	QuoteActionRef;
	///报价引用
	strcpy(req.QuoteRef,pQuote->QuoteRef);
	///请求编号
	//TThostFtdcRequestIDType	RequestID;
	///前置编号
	req.FrontID=FRONT_ID;
	///会话编号
	req.SessionID=SESSION_ID;
	///交易所代码
	//TThostFtdcExchangeIDType	ExchangeID;
	///报价操作编号
	//TThostFtdcOrderSysIDType	QuoteSysID;
	///操作标志
	req.ActionFlag=THOST_FTDC_AF_Delete;
	///用户代码
	//TThostFtdcUserIDType	UserID;
	///合约代码
	strcpy(req.InstrumentID,pQuote->InstrumentID);

	int iResult = pTraderApi->ReqQuoteAction(&req, ++iRequestID);
	cerr << "--->>> 报价操作请求: "  << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
	QUOTE_ACTION_SENT = true;
}

void CTraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspOrderAction" << endl;
	IsErrorRspInfo(pRspInfo);
}

void CTraderSpi::OnRspExecOrderAction(CThostFtdcInputExecOrderActionField *pInpuExectOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//正确的撤单操作，不会进入该回调
	cerr << "--->>> " << "OnRspExecOrderAction" << endl;
	IsErrorRspInfo(pRspInfo);
}

void CTraderSpi::OnRspQuoteAction(CThostFtdcInputQuoteActionField *pInpuQuoteAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//正确的撤单操作，不会进入该回调
	cerr << "--->>> " << "OnRspQuoteAction" << endl;
	IsErrorRspInfo(pRspInfo);
}

///报单通知
void CTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	cerr << "--->>> " << "OnRtnOrder"  << endl;
	if (IsMyOrder(pOrder))
	{
		//if (IsTradingOrder(pOrder))
		//	ReqOrderAction(pOrder);
		//else if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled)
		//	cout << "--->>> 撤单成功" << endl;

		// 只在OnRtnTrade回调调用时设置成交
		if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled) {
			cout << "交易撤销:" << pOrder->OrderStatus << endl;
			gTradeInfo->updateTradeResult(TradeCancled, NULL, NULL);
			gTradeInfo->setStatus(StatusDone);
		}
	}
}

//执行宣告通知
void CTraderSpi::OnRtnExecOrder(CThostFtdcExecOrderField *pExecOrder)
{
	cerr << "--->>> " << "OnRtnExecOrder"  << endl;
	if (IsMyExecOrder(pExecOrder))
	{
		if (IsTradingExecOrder(pExecOrder))
			ReqExecOrderAction(pExecOrder);
		else if (pExecOrder->ExecResult == THOST_FTDC_OER_Canceled)
			cout << "--->>> 执行宣告撤单成功" << endl;
	}
}

//询价通知
void CTraderSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
	//上期所中金所询价通知通过该接口返回；只有做市商客户可以收到该通知
	cerr << "--->>> " << "OnRtnForQuoteRsp"  << endl;
}

//报价通知
void CTraderSpi::OnRtnQuote(CThostFtdcQuoteField *pQuote)
{
	cerr << "--->>> " << "OnRtnQuote"  << endl;
	if (IsMyQuote(pQuote))
	{
		if (IsTradingQuote(pQuote))
			ReqQuoteAction(pQuote);
		else if (pQuote->QuoteStatus == THOST_FTDC_OST_Canceled)
			cout << "--->>> 报价撤单成功" << endl;
	}
}

///成交通知
void CTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	cerr << "--->>> " << "OnRtnTrade"  << endl;

	gTradeInfo->updateTradeResult(TradeDone, pTrade, NULL);
	gTradeInfo->setStatus(StatusDone);
}

void CTraderSpi:: OnFrontDisconnected(int nReason)
{
	cerr << "--->>> " << "OnFrontDisconnected" << endl;
	cerr << "--->>> Reason = " << nReason << endl;
	gTradeInfo->setStatus(StatusDisconnect);
}
		
void CTraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
	cerr << "--->>> " << "OnHeartBeatWarning" << endl;
	cerr << "--->>> nTimerLapse = " << nTimeLapse << endl;
}

void CTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspError" << endl;
	IsErrorRspInfo(pRspInfo);
}

bool CTraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult) {
		cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
		gTradeInfo->setStatus(StatusError);
	}

	return bResult;
}

bool CTraderSpi::IsMyOrder(CThostFtdcOrderField *pOrder)
{
	return ((pOrder->FrontID == FRONT_ID) &&
			(pOrder->SessionID == SESSION_ID) &&
			(strcmp(pOrder->OrderRef, ORDER_REF) == 0));
}

bool CTraderSpi::IsMyExecOrder(CThostFtdcExecOrderField *pExecOrder)
{
	return ((pExecOrder->FrontID == FRONT_ID) &&
		(pExecOrder->SessionID == SESSION_ID) &&
		(strcmp(pExecOrder->ExecOrderRef, EXECORDER_REF) == 0));
}

bool CTraderSpi::IsMyQuote(CThostFtdcQuoteField *pQuote)
{
	return ((pQuote->FrontID == FRONT_ID) &&
		(pQuote->SessionID == SESSION_ID) &&
		(strcmp(pQuote->QuoteRef, QUOTE_REF) == 0));
}

bool CTraderSpi::IsTradingOrder(CThostFtdcOrderField *pOrder)
{
	return ((pOrder->OrderStatus != THOST_FTDC_OST_PartTradedNotQueueing) &&
			(pOrder->OrderStatus != THOST_FTDC_OST_Canceled) &&
			(pOrder->OrderStatus != THOST_FTDC_OST_AllTraded));
}

bool CTraderSpi::IsTradingExecOrder(CThostFtdcExecOrderField *pExecOrder)
{
	return (pExecOrder->ExecResult != THOST_FTDC_OER_Canceled);
}

bool CTraderSpi::IsTradingQuote(CThostFtdcQuoteField *pQuote)
{
	return (pQuote->QuoteStatus != THOST_FTDC_OST_Canceled);
}
