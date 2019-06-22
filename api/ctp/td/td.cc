#include "td.h"
using namespace std;

char front_addr[32];
char broker_id[32];
char user_id[32];
char password[32];
char app_id[32];
char auth_code[32];
TdApi g_td_api;
char time_str[30] = "";
list <CThostFtdcOrderField> order_list;
CThostFtdcOrderField * order_data = new CThostFtdcOrderField;
vector <CThostFtdcInstrumentField> all_instruments;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

char * get_time_str()
{
    struct tm *p;
    time_t timep = time(NULL);
    p = localtime(&timep);
    strftime(time_str, 25, "%G-%m-%d %H:%M:%S", p);
    return time_str;
}

__attribute__((constructor)) void so_start()
{
    string td_cfg("api/ctp/td/td.cfg");
    printf("Td Start\n");
    ifstream fp(td_cfg.c_str());
    if (fp.is_open())
    {
        string tmp;
        getline(fp, tmp);
        getline(fp, tmp);
        strcpy(front_addr, tmp.c_str());
        getline(fp, tmp);
        getline(fp, tmp);
        strcpy(broker_id, tmp.c_str());
        getline(fp, tmp);
        getline(fp, tmp);
        strcpy(user_id, tmp.c_str());
        getline(fp, tmp);
        getline(fp, tmp);
        strcpy(password, tmp.c_str());
        getline(fp, tmp);
        getline(fp, tmp);
        strcpy(app_id, tmp.c_str());
        getline(fp, tmp);
        getline(fp, tmp);
        strcpy(auth_code, tmp.c_str());
        fp.close();
        printf("%s\n", front_addr);
        printf("%s\n", broker_id);
        printf("%s\n", user_id);
        printf("%s\n", password);
        printf("%s\n", app_id);
        printf("%s\n", auth_code);
        g_td_api.init();
    }
    else
    {
        printf("Td 读取配置文件失败\n");
    }
}

__attribute__((destructor)) void td_end()
{
    cout << "Td destructor" << endl;
    pthread_mutex_destroy(&mutex);
    delete order_data;
}

void TdApi::OnFrontConnected()
{
    cout << "Td OnFrontConnected" << endl;
    if (strcmp(app_id, "NULL") == 0)
    {
        cout << "Td -->未发送数字验证,发送登录请求" << endl;
        CThostFtdcReqUserLoginField req;
        memset(&req, 0, sizeof(req));
        strcpy(req.BrokerID, broker_id);
        strcpy(req.UserID, user_id);
        strcpy(req.Password, password);
        td_api->ReqUserLogin(&req, ++request_id);
    }
    else
    {
        cout << "Td -->发送数字验证" << endl;
        CThostFtdcReqAuthenticateField req;
        strcpy(req.BrokerID, broker_id);
        strcpy(req.UserID, user_id);
        strcpy(req.AppID, app_id);
        strcpy(req.AuthCode, auth_code);
        td_api->ReqAuthenticate(&req, ++request_id);
    }
}

void TdApi::OnFrontDisconnected(int nReason)
{
    cerr << "Td OnFrontDisconnected nReason:" << nReason << endl;
}

void TdApi::OnHeartBeatWarning(int nTimeLapse)
{
    cerr << "Td OnHeartBeatWarning nTimeLapse:" << nTimeLapse << endl;
}

void TdApi::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cout << "Td OnRspAuthenticate" << endl;
    if ((pRspInfo) && (pRspInfo->ErrorID != 0))
    {
        cerr << "Td 数字验证出错" << endl;
        return;
    }
    cout << "Td -->发送登录请求" << endl;
    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, broker_id);
    strcpy(req.UserID, user_id);
    strcpy(req.Password, password);
    td_api->ReqUserLogin(&req, ++request_id);
}

void TdApi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cout << "Td OnRspUserLogin" << endl;
    if (pRspInfo && pRspInfo->ErrorID != 0)
    {
        cerr << "Td 登录失败" << endl;
    }
    else
    {
        FRONT_ID = pRspUserLogin->FrontID;
        SESSION_ID = pRspUserLogin->SessionID;
        cout << "Td 登录成功" << endl;
        cout << "    交易日: " << pRspUserLogin->TradingDay << endl;
        cout << "    登录成功时间: " << pRspUserLogin->LoginTime << endl;
        cout << "    经纪公司代码: " << pRspUserLogin->BrokerID << endl;
        cout << "    用户代码: " << pRspUserLogin->UserID << endl;
        cout << "    交易系统名称: " << pRspUserLogin->SystemName << endl;
        cout << "    前置编号: " << pRspUserLogin->FrontID << endl;
        cout << "    会话编号: " << pRspUserLogin->SessionID << endl;
        cout << "    最大报单引用: " << pRspUserLogin->MaxOrderRef << endl;
        cout << "    上期所时间: " << pRspUserLogin->SHFETime << endl;
        cout << "    大商所时间: " << pRspUserLogin->DCETime << endl;
        cout << "    郑商所时间: " << pRspUserLogin->CZCETime << endl;
        cout << "    中金所时间: " << pRspUserLogin->FFEXTime << endl;
        cout << "    能源中心时间: " << pRspUserLogin->INETime << endl;
        ReqSettlementInfoConfirm();
    }
}

void TdApi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    this->is_api_ok = false;
    cout << "Td OnRspUserLogout" << endl;
}

///用户口令更新请求响应
void TdApi::OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///资金账户口令更新请求响应
void TdApi::OnRspTradingAccountPasswordUpdate(CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///报单录入请求响应
void TdApi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///预埋单录入请求响应
void TdApi::OnRspParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///预埋撤单录入请求响应
void TdApi::OnRspParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///报单操作请求响应
void TdApi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///查询最大报单数量响应
void TdApi::OnRspQueryMaxOrderVolume(CThostFtdcQueryMaxOrderVolumeField *pQueryMaxOrderVolume, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///投资者结算结果确认响应
void TdApi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cout << "Td OnRspSettlementInfoConfirm" << endl;
    if (bIsLast)
    {
        CThostFtdcQryInstrumentField req;
        memset(&req, 0, sizeof(req));
        strcpy(req.ExchangeID, "");
        strcpy(req.ExchangeInstID, "");
        strcpy(req.InstrumentID, "");
        strcpy(req.ProductID, "");
        int result = td_api->ReqQryInstrument(&req, ++request_id);
        cout << "Td -->发送合约查询请求: " << ((result == 0) ? "成功" : "失败") << endl;
    }
}

///删除预埋单响应
void TdApi::OnRspRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///删除预埋撤单响应
void TdApi::OnRspRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///执行宣告录入请求响应
void TdApi::OnRspExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///执行宣告操作请求响应
void TdApi::OnRspExecOrderAction(CThostFtdcInputExecOrderActionField *pInputExecOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///询价录入请求响应
void TdApi::OnRspForQuoteInsert(CThostFtdcInputForQuoteField *pInputForQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///报价录入请求响应
void TdApi::OnRspQuoteInsert(CThostFtdcInputQuoteField *pInputQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///报价操作请求响应
void TdApi::OnRspQuoteAction(CThostFtdcInputQuoteActionField *pInputQuoteAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///批量报单操作请求响应
void TdApi::OnRspBatchOrderAction(CThostFtdcInputBatchOrderActionField *pInputBatchOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///期权自对冲录入请求响应
void TdApi::OnRspOptionSelfCloseInsert(CThostFtdcInputOptionSelfCloseField *pInputOptionSelfClose, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///期权自对冲操作请求响应
void TdApi::OnRspOptionSelfCloseAction(CThostFtdcInputOptionSelfCloseActionField *pInputOptionSelfCloseAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///申请组合录入请求响应
void TdApi::OnRspCombActionInsert(CThostFtdcInputCombActionField *pInputCombAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询报单响应
void TdApi::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询成交响应
void TdApi::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询投资者持仓响应
void TdApi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询资金账户响应
void TdApi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询投资者响应
void TdApi::OnRspQryInvestor(CThostFtdcInvestorField *pInvestor, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询交易编码响应
void TdApi::OnRspQryTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询合约保证金率响应
void TdApi::OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询合约手续费率响应
void TdApi::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询交易所响应
void TdApi::OnRspQryExchange(CThostFtdcExchangeField *pExchange, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询产品响应
void TdApi::OnRspQryProduct(CThostFtdcProductField *pProduct, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询合约响应
void TdApi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    CThostFtdcInstrumentField tmp;
    memcpy(&tmp, pInstrument, sizeof(CThostFtdcInstrumentField));
    all_instruments.push_back(tmp);
    if (bIsLast)
    {
        ofstream outfile;
        outfile.open("all_instruments.csv");
        int len = all_instruments.size();
        for (int i = 0; i < len; i++)
        {
            outfile << all_instruments[i].InstrumentID << ','
                << all_instruments[i].ExchangeID << ','
                << all_instruments[i].ExchangeInstID << ','
                << all_instruments[i].ProductID << ','
                << all_instruments[i].ProductClass << ','
                << all_instruments[i].DeliveryYear << ','
                << all_instruments[i].DeliveryMonth << ','
                << all_instruments[i].MaxMarketOrderVolume << ','
                << all_instruments[i].MinMarketOrderVolume << ','
                << all_instruments[i].MaxLimitOrderVolume << ','
                << all_instruments[i].MinLimitOrderVolume << ','
                << all_instruments[i].VolumeMultiple << ','
                << all_instruments[i].PriceTick << ','
                << all_instruments[i].CreateDate << ','
                << all_instruments[i].OpenDate << ','
                << all_instruments[i].ExpireDate << ','
                << all_instruments[i].StartDelivDate << ','
                << all_instruments[i].EndDelivDate << ','
                << all_instruments[i].InstLifePhase << ','
                << all_instruments[i].IsTrading << ','
                << all_instruments[i].PositionType << ','
                << all_instruments[i].PositionDateType << ','
                << all_instruments[i].LongMarginRatio << ','
                << all_instruments[i].ShortMarginRatio << ','
                << all_instruments[i].MaxMarginSideAlgorithm << ','
                << all_instruments[i].UnderlyingInstrID << ','
                << all_instruments[i].StrikePrice << ','
                << all_instruments[i].OptionsType << ','
                << all_instruments[i].UnderlyingMultiple << ','
                << all_instruments[i].CombinationType << endl;
        }
        outfile.close();
        all_instruments.clear();
        vector <CThostFtdcInstrumentField>(all_instruments).swap(all_instruments);
        this->is_api_ok = true;
        cout << "Td 合约信息接收完毕,已存入all_instruments.csv" << endl;
    }
}

///请求查询行情响应
void TdApi::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询投资者结算结果响应
void TdApi::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询转帐银行响应
void TdApi::OnRspQryTransferBank(CThostFtdcTransferBankField *pTransferBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询投资者持仓明细响应
void TdApi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询客户通知响应
void TdApi::OnRspQryNotice(CThostFtdcNoticeField *pNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询结算信息确认响应
void TdApi::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询投资者持仓明细响应
void TdApi::OnRspQryInvestorPositionCombineDetail(CThostFtdcInvestorPositionCombineDetailField *pInvestorPositionCombineDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///查询保证金监管系统经纪公司资金账户密钥响应
void TdApi::OnRspQryCFMMCTradingAccountKey(CThostFtdcCFMMCTradingAccountKeyField *pCFMMCTradingAccountKey, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询仓单折抵信息响应
void TdApi::OnRspQryEWarrantOffset(CThostFtdcEWarrantOffsetField *pEWarrantOffset, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询投资者品种/跨品种保证金响应
void TdApi::OnRspQryInvestorProductGroupMargin(CThostFtdcInvestorProductGroupMarginField *pInvestorProductGroupMargin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询交易所保证金率响应
void TdApi::OnRspQryExchangeMarginRate(CThostFtdcExchangeMarginRateField *pExchangeMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询交易所调整保证金率响应
void TdApi::OnRspQryExchangeMarginRateAdjust(CThostFtdcExchangeMarginRateAdjustField *pExchangeMarginRateAdjust, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询汇率响应
void TdApi::OnRspQryExchangeRate(CThostFtdcExchangeRateField *pExchangeRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询二级代理操作员银期权限响应
void TdApi::OnRspQrySecAgentACIDMap(CThostFtdcSecAgentACIDMapField *pSecAgentACIDMap, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询产品报价汇率
void TdApi::OnRspQryProductExchRate(CThostFtdcProductExchRateField *pProductExchRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询产品组
void TdApi::OnRspQryProductGroup(CThostFtdcProductGroupField *pProductGroup, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询做市商合约手续费率响应
void TdApi::OnRspQryMMInstrumentCommissionRate(CThostFtdcMMInstrumentCommissionRateField *pMMInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询做市商期权合约手续费响应
void TdApi::OnRspQryMMOptionInstrCommRate(CThostFtdcMMOptionInstrCommRateField *pMMOptionInstrCommRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询报单手续费响应
void TdApi::OnRspQryInstrumentOrderCommRate(CThostFtdcInstrumentOrderCommRateField *pInstrumentOrderCommRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询期权交易成本响应
void TdApi::OnRspQryOptionInstrTradeCost(CThostFtdcOptionInstrTradeCostField *pOptionInstrTradeCost, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询期权合约手续费响应
void TdApi::OnRspQryOptionInstrCommRate(CThostFtdcOptionInstrCommRateField *pOptionInstrCommRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询执行宣告响应
void TdApi::OnRspQryExecOrder(CThostFtdcExecOrderField *pExecOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询询价响应
void TdApi::OnRspQryForQuote(CThostFtdcForQuoteField *pForQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询报价响应
void TdApi::OnRspQryQuote(CThostFtdcQuoteField *pQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询期权自对冲响应
void TdApi::OnRspQryOptionSelfClose(CThostFtdcOptionSelfCloseField *pOptionSelfClose, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询投资单元响应
void TdApi::OnRspQryInvestUnit(CThostFtdcInvestUnitField *pInvestUnit, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询组合合约安全系数响应
void TdApi::OnRspQryCombInstrumentGuard(CThostFtdcCombInstrumentGuardField *pCombInstrumentGuard, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询申请组合响应
void TdApi::OnRspQryCombAction(CThostFtdcCombActionField *pCombAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询转帐流水响应
void TdApi::OnRspQryTransferSerial(CThostFtdcTransferSerialField *pTransferSerial, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询银期签约关系响应
void TdApi::OnRspQryAccountregister(CThostFtdcAccountregisterField *pAccountregister, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///错误应答
void TdApi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///报单通知
void TdApi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    //if (!pOrder) return;
    if (pOrder->OrderStatus == '1' || pOrder->OrderStatus == '0' || pOrder->OrderStatus == '5')
    {
        CThostFtdcOrderField tmp;
        memcpy(&tmp, pOrder, sizeof(CThostFtdcOrderField));
        pthread_mutex_lock(&mutex);
        order_list.push_back(tmp);
        pthread_mutex_unlock(&mutex);
        api_set(OnRtnOrderFuture);
    }
    cout << "OnRtnOrder OrderStatus: " << pOrder->OrderStatus << endl;
}

///成交通知
void TdApi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{}

///报单录入错误回报
void TdApi::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{}

///报单操作错误回报
void TdApi::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{}

///合约交易状态通知
void TdApi::OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus)
{}

///交易所公告通知
void TdApi::OnRtnBulletin(CThostFtdcBulletinField *pBulletin)
{}

///交易通知
void TdApi::OnRtnTradingNotice(CThostFtdcTradingNoticeInfoField *pTradingNoticeInfo)
{}

///提示条件单校验错误
void TdApi::OnRtnErrorConditionalOrder(CThostFtdcErrorConditionalOrderField *pErrorConditionalOrder)
{}

///执行宣告通知
void TdApi::OnRtnExecOrder(CThostFtdcExecOrderField *pExecOrder)
{}

///执行宣告录入错误回报
void TdApi::OnErrRtnExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, CThostFtdcRspInfoField *pRspInfo)
{}

///执行宣告操作错误回报
void TdApi::OnErrRtnExecOrderAction(CThostFtdcExecOrderActionField *pExecOrderAction, CThostFtdcRspInfoField *pRspInfo)
{}

///询价录入错误回报
void TdApi::OnErrRtnForQuoteInsert(CThostFtdcInputForQuoteField *pInputForQuote, CThostFtdcRspInfoField *pRspInfo)
{}

///报价通知
void TdApi::OnRtnQuote(CThostFtdcQuoteField *pQuote)
{}

///报价录入错误回报
void TdApi::OnErrRtnQuoteInsert(CThostFtdcInputQuoteField *pInputQuote, CThostFtdcRspInfoField *pRspInfo)
{}

///报价操作错误回报
void TdApi::OnErrRtnQuoteAction(CThostFtdcQuoteActionField *pQuoteAction, CThostFtdcRspInfoField *pRspInfo)
{}

///询价通知
void TdApi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{}

///保证金监控中心用户令牌
void TdApi::OnRtnCFMMCTradingAccountToken(CThostFtdcCFMMCTradingAccountTokenField *pCFMMCTradingAccountToken)
{}

///批量报单操作错误回报
void TdApi::OnErrRtnBatchOrderAction(CThostFtdcBatchOrderActionField *pBatchOrderAction, CThostFtdcRspInfoField *pRspInfo)
{}

///期权自对冲通知
void TdApi::OnRtnOptionSelfClose(CThostFtdcOptionSelfCloseField *pOptionSelfClose)
{}

///期权自对冲录入错误回报
void TdApi::OnErrRtnOptionSelfCloseInsert(CThostFtdcInputOptionSelfCloseField *pInputOptionSelfClose, CThostFtdcRspInfoField *pRspInfo)
{}

///期权自对冲操作错误回报
void TdApi::OnErrRtnOptionSelfCloseAction(CThostFtdcOptionSelfCloseActionField *pOptionSelfCloseAction, CThostFtdcRspInfoField *pRspInfo)
{}

///申请组合通知
void TdApi::OnRtnCombAction(CThostFtdcCombActionField *pCombAction)
{}

///申请组合录入错误回报
void TdApi::OnErrRtnCombActionInsert(CThostFtdcInputCombActionField *pInputCombAction, CThostFtdcRspInfoField *pRspInfo)
{}

///请求查询签约银行响应
void TdApi::OnRspQryContractBank(CThostFtdcContractBankField *pContractBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询预埋单响应
void TdApi::OnRspQryParkedOrder(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询预埋撤单响应
void TdApi::OnRspQryParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询交易通知响应
void TdApi::OnRspQryTradingNotice(CThostFtdcTradingNoticeField *pTradingNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询经纪公司交易参数响应
void TdApi::OnRspQryBrokerTradingParams(CThostFtdcBrokerTradingParamsField *pBrokerTradingParams, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询经纪公司交易算法响应
void TdApi::OnRspQryBrokerTradingAlgos(CThostFtdcBrokerTradingAlgosField *pBrokerTradingAlgos, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///请求查询监控中心用户令牌
void TdApi::OnRspQueryCFMMCTradingAccountToken(CThostFtdcQueryCFMMCTradingAccountTokenField *pQueryCFMMCTradingAccountToken, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///银行发起银行资金转期货通知
void TdApi::OnRtnFromBankToFutureByBank(CThostFtdcRspTransferField *pRspTransfer)
{}

///银行发起期货资金转银行通知
void TdApi::OnRtnFromFutureToBankByBank(CThostFtdcRspTransferField *pRspTransfer)
{}

///银行发起冲正银行转期货通知
void TdApi::OnRtnRepealFromBankToFutureByBank(CThostFtdcRspRepealField *pRspRepeal)
{}

///银行发起冲正期货转银行通知
void TdApi::OnRtnRepealFromFutureToBankByBank(CThostFtdcRspRepealField *pRspRepeal)
{}

///期货发起银行资金转期货通知
void TdApi::OnRtnFromBankToFutureByFuture(CThostFtdcRspTransferField *pRspTransfer)
{}

///期货发起期货资金转银行通知
void TdApi::OnRtnFromFutureToBankByFuture(CThostFtdcRspTransferField *pRspTransfer)
{}

///系统运行时期货端手工发起冲正银行转期货请求，银行处理完毕后报盘发回的通知
void TdApi::OnRtnRepealFromBankToFutureByFutureManual(CThostFtdcRspRepealField *pRspRepeal)
{}

///系统运行时期货端手工发起冲正期货转银行请求，银行处理完毕后报盘发回的通知
void TdApi::OnRtnRepealFromFutureToBankByFutureManual(CThostFtdcRspRepealField *pRspRepeal)
{}

///期货发起查询银行余额通知
void TdApi::OnRtnQueryBankBalanceByFuture(CThostFtdcNotifyQueryAccountField *pNotifyQueryAccount)
{}

///期货发起银行资金转期货错误回报
void TdApi::OnErrRtnBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo)
{}

///期货发起期货资金转银行错误回报
void TdApi::OnErrRtnFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo)
{}

///系统运行时期货端手工发起冲正银行转期货错误回报
void TdApi::OnErrRtnRepealBankToFutureByFutureManual(CThostFtdcReqRepealField *pReqRepeal, CThostFtdcRspInfoField *pRspInfo)
{}

///系统运行时期货端手工发起冲正期货转银行错误回报
void TdApi::OnErrRtnRepealFutureToBankByFutureManual(CThostFtdcReqRepealField *pReqRepeal, CThostFtdcRspInfoField *pRspInfo)
{}

///期货发起查询银行余额错误回报
void TdApi::OnErrRtnQueryBankBalanceByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, CThostFtdcRspInfoField *pRspInfo)
{}

///期货发起冲正银行转期货请求，银行处理完毕后报盘发回的通知
void TdApi::OnRtnRepealFromBankToFutureByFuture(CThostFtdcRspRepealField *pRspRepeal)
{}

///期货发起冲正期货转银行请求，银行处理完毕后报盘发回的通知
void TdApi::OnRtnRepealFromFutureToBankByFuture(CThostFtdcRspRepealField *pRspRepeal)
{}

///期货发起银行资金转期货应答
void TdApi::OnRspFromBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///期货发起期货资金转银行应答
void TdApi::OnRspFromFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///期货发起查询银行余额应答
void TdApi::OnRspQueryBankAccountMoneyByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

///银行发起银期开户通知
void TdApi::OnRtnOpenAccountByBank(CThostFtdcOpenAccountField *pOpenAccount)
{}

///银行发起银期销户通知
void TdApi::OnRtnCancelAccountByBank(CThostFtdcCancelAccountField *pCancelAccount)
{}

///银行发起变更银行账号通知
void TdApi::OnRtnChangeAccountByBank(CThostFtdcChangeAccountField *pChangeAccount)
{}

int TdApi::ReqQryInstrument(CThostFtdcQryInstrumentField *pQryInstrument, int nRequestID)
{
    CThostFtdcQryInstrumentField req;
    memset(&req, 0, sizeof(CThostFtdcQryInstrumentField));
    int result = td_api->ReqQryInstrument(&req, ++request_id);
    return result;
}

void TdApi::ReqSettlementInfoConfirm()
{
    cout << "Td ReqSettlementInfoConfirm" << endl;
    CThostFtdcSettlementInfoConfirmField req;
    memset(&req, 0, sizeof(CThostFtdcSettlementInfoConfirmField));
    strcpy(req.BrokerID, broker_id);
    strcpy(req.InvestorID, user_id);
    int result = td_api->ReqSettlementInfoConfirm(&req, ++request_id);
    cout << "Td 投资者结算结果确认: " << ((result == 0) ? "成功" : "失败") << endl;
}

void TdApi::ReqUserLogout()
{
    cout << "Td ReqUserLogout" << endl;
    CThostFtdcUserLogoutField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, broker_id);
    strcpy(req.UserID, user_id);
    int result = td_api->ReqUserLogout(&req, ++request_id);
    cout << "Td 发送用户退出登录请求 : " << ((result == 0) ? "成功" : "失败") << endl;
}

int TdApi::InsertOrder(char *Instrument, char direction, char offsetFlag, char priceType, double price, int num)
{
    cout << "InsertOrder " << Instrument << "|" << direction << "|" << offsetFlag << "|" << priceType <<
        "|" << price << "|" << num << endl;
    CThostFtdcInputOrderField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, broker_id);
    strcpy(req.InvestorID, user_id);
    req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
    req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    req.ContingentCondition = THOST_FTDC_CC_Immediately;
    req.TimeCondition = THOST_FTDC_TC_GFD;
    req.VolumeCondition = THOST_FTDC_VC_AV;
    req.MinVolume = 1;
    req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    req.IsAutoSuspend = 0;
    req.UserForceClose = 0;
    // --------------------------------------------------
    strcpy(req.InstrumentID, Instrument);
    req.CombOffsetFlag[0] = offsetFlag;
    req.Direction = direction;
    req.VolumeTotalOriginal = num;
    req.LimitPrice = price;
    req.OrderPriceType = priceType;
    sprintf(req.OrderRef, "%012d", ++order_ref);
    td_api->ReqOrderInsert(&req, ++(request_id));
    return order_ref;
}

int TdApi::DeleteOrder(char *Instrument, int OrderRef)
{
    cout << "DeleteOrder " << Instrument << "|"  << OrderRef << endl;
    CThostFtdcInputOrderActionField req_del;
    memset(&req_del, 0, sizeof(req_del));
    strcpy(req_del.BrokerID, broker_id);
    strcpy(req_del.InvestorID, user_id);
    strcpy(req_del.InstrumentID, Instrument);
    sprintf(req_del.OrderRef, "%012d", OrderRef);
    req_del.FrontID = FRONT_ID;
    req_del.SessionID = SESSION_ID;
    req_del.ActionFlag = THOST_FTDC_AF_Delete;
    return td_api->ReqOrderAction(&req_del, ++(request_id));
}

void TdApi::init()
{
    char pwd[256] = { 0 };
    getcwd(pwd, 256);
    string dir = string(pwd).append("/api/ctp/td/");
    td_api = CThostFtdcTraderApi::CreateFtdcTraderApi(dir.c_str());

    td_api->RegisterSpi(this);
    td_api->SubscribePublicTopic(THOST_TERT_QUICK);
    td_api->SubscribePrivateTopic(THOST_TERT_QUICK);
    td_api->RegisterFront((char *)front_addr);
    td_api->Init();
}

void TdApi::exit()
{
    ReqUserLogout();
    td_api->RegisterSpi(NULL);
    td_api->Release();
    td_api = NULL;
    cout << "Td exit" << endl;
}

extern "C"
{
    bool is_api_ok()
    {
        return g_td_api.is_api_ok;
    }

    int insert_order(char *Instrument, char direction, char offsetFlag, char priceType, double price, int num)
    {
        return g_td_api.InsertOrder(Instrument, direction, offsetFlag, priceType, price, num);
    }

    int delete_order(char *Instrument, int OrderRef)
    {
        return g_td_api.DeleteOrder(Instrument, OrderRef);
    }

    void * get_order_data()
    {
        if (order_list.size() > 0)
        {
            pthread_mutex_lock(&mutex);
            memcpy(order_data, &(order_list.front()), sizeof(CThostFtdcOrderField));
            order_list.erase(order_list.begin());
            if (order_list.size() > 0)
            {
                api_set(OnRtnOrderFuture);
            }
            pthread_mutex_unlock(&mutex);
            return order_data;
        }
        else
        {
            return NULL;
        }
    }

}

//int main()
//{
//    while (!is_api_ok())
//    {
//        sleep(1);
//    }
//    char Instrument[] = "rb1910";
//    int of = insert_order(Instrument, THOST_FTDC_D_Buy, THOST_FTDC_OF_Open, THOST_FTDC_OPT_LimitPrice, 3325.0, 1);
//    cout << "insert_order " << of << endl;
//    int dd = delete_order(Instrument, of);
//    cout << "delete_order " << dd << endl;
//    return 0;
//}
