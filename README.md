# ctp-dll-golang
Exports the interfaces of CTP to golang strategy:
https://github.com/maisuid/madaoQT/blob/master/exchange/ctpdll.go

DO ***NOT*** allow parallel tradings in a single session, but trading from different sessions in the same time is supported.


Demo:

dll = &CTPDll{

    // make sure all dlls are in the same directory
    
    Dll: syscall.NewLazyDLL("CTPDll2.dll"),
    
}

var config = map[string]interface{}{

	"marketaddr": "tcp://180.168.212.228:41213",
	
	"tradeaddr":  "tcp://180.168.212.228:41205",
	
	"broker":     "****",
	
	"appid":      "*********",
	
	"authencode": "*********",
	
	"instruments": []map[string]interface{}{
	
		{"name": "rb1910"}, // 螺纹钢
		{"name": "RM809"},  // 菜粨
		{"name": "AP901"}, // 苹果
		{"name": "CF901"}, // 棉花
		{"name": "m1809"}, // 豆粨
		{"name": "j1909"},  // 焦炭
		{"name": "bu1912"}, // 沥青
		{"name": "i1809"},  // 铁矿
		{"name": "c1809"},  // 玉米
		{"name": "l1901"}, // 塑料
		{"name": "jd1809"}, // 鸡蛋
		{"name": "O1809"},  // 菜油
		{"name": "MA909"}, // 甲醇
		{"name": "SR909"}, // 白糖
		{"name": "FG901"},  // 玻璃，长短周期长期收益均为负数
		{"name": "eg1909"},
		{"name": "TA909"}, // PTA
		{"name": "ag1909"}, // 沪银
		{"name": "PP1901"}, // PP
	},
}


if configS, err = json.Marshal(config); err != nil {

    return err
    
}

if !dll.SetConfig(string(configS)) {

    return errors.New("Fail to config CTP")
    
}

go func() {

    Logger.Infof("Start the prices monitor...")
    
    dll.InitMarket()
    
}()

go func() {

    Logger.Infof("Start trading thread...")
    
    dll.InitTrade()
    
}()

dll.GetInstrumentInfo(instrument)

dll.GetPositionInfo(instrument)

dll.GetDepth(instrument)

result = dll.MarketOpenPosition(instrument, int(Amount), int(Price), 1, isMarket)

result = dll.MarketOpenPosition(instrument, int(Amount), int(Price), 0, isMarket)

result = dll.MarketClosePosition(instrument, int(Amount), int(Price), 0, isMarket, isToday)

result = dll.MarketClosePosition(instrument, int(Amount), int(Price), 1, isMarket, isToday)

dll.CloseMarket()
dll.CloseTrade()
