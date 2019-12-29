
#include "md.h"

using namespace std;

char time_str[30] = "";
char * ppInstrumentID[2] = { new TThostFtdcInstrumentIDType , 0 };
map<string, CThostFtdcDepthMarketDataField *> g_market;
stack<TThostFtdcInstrumentIDTypeStruct> instrument_stack;
TThostFtdcInstrumentIDTypeStruct tmp_instrument_id;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char front_addr[32];
char broker_id[32];
char user_id[32];
char password[32];
MdApi g_md_api;

__attribute__((constructor)) void so_start()
{
    string md_cfg("api/ctp/md/md.cfg");
    printf("Md Start\n");
    ifstream fp(md_cfg.c_str());
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
        fp.close();
        printf("%s\n", front_addr);
        printf("%s\n", broker_id);
        printf("%s\n", user_id);
        printf("%s\n", password);
        g_md_api.init();
    }
    else
    {
        printf("Md 读取配置文件失败");
    }
}

__attribute__((destructor)) void md_end()
{
    cout << "Md destructor" << endl;
    pthread_mutex_destroy(&mutex);
    if (!g_market.empty())
    {
        map<string, CThostFtdcDepthMarketDataField *>::iterator iter;
        iter = g_market.begin();
        while (iter != g_market.end())
        {
            if (iter->first != "")
            {
                CThostFtdcDepthMarketDataField * tmp = iter->second;
                iter->second = NULL;
                delete tmp;
            }
            else
            {
                break;
            }
            iter++;
        }
    }
    cout << "Md destructor end" << endl;
}

char * get_time_str()
{
    struct tm *p;
    time_t timep = time(NULL);
    p = localtime(&timep);
    strftime(time_str, 25, "%G-%m-%d %H:%M:%S", p);
    return time_str;
}

void MdApi::OnFrontConnected()
{
    cout << "Md OnFrontConnected" << endl;
    ReqUserLogin();
}

void MdApi::OnFrontDisconnected(int nReason)
{
    cerr << "Md OnFrontDisconnected " << get_time_str() << endl;
    cerr << "--->>> Reason = " << nReason << endl;
}

void MdApi::OnHeartBeatWarning(int nTimeLapse) {};

void MdApi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo && pRspInfo->ErrorID == 0)
    {
        cout << "Md OnRspUserLogin" << endl;
        this->is_api_ok = true;
        if (!g_market.empty())
        {
            map<string, CThostFtdcDepthMarketDataField *>::iterator iter;
            iter = g_market.begin();
            while (iter != g_market.end())
            {
                if (iter->first != "")
                {
                    strcpy(ppInstrumentID[0], iter->first.c_str());
                    SubscribeMarketData();
                    iter++;
                }
                else
                {
                    break;
                }
            }
        }
    }
    else
    {
        cerr << "ERROR OnRspUserLogin" << endl;
        return;
    }
}

void MdApi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cout << "Md OnRspUserLogout " << get_time_str() << endl;
    this->is_api_ok = false;
}

void MdApi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cerr << "Md OnRspError" << endl;
}

void MdApi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cout << "Md OnRspSubMarketData [" << pSpecificInstrument->InstrumentID << "]" << endl;
}

void MdApi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cout << "Md OnRspUnSubMarketData [" << pSpecificInstrument->InstrumentID << "]" << endl;
}

void MdApi::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

void MdApi::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{}

void MdApi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{}

void MdApi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    if (pDepthMarketData && (g_market.find(pDepthMarketData->InstrumentID) != g_market.end()))
    {
        //cout << "---- " << pDepthMarketData->InstrumentID << "  " << pDepthMarketData->LastPrice << endl;
        memcpy(g_market[pDepthMarketData->InstrumentID], pDepthMarketData, sizeof(CThostFtdcDepthMarketDataField));
        TThostFtdcInstrumentIDTypeStruct t;
        strncpy(t.Instrument, pDepthMarketData->InstrumentID, sizeof(t.Instrument));
        pthread_mutex_lock(&mutex);
        instrument_stack.push(t);
        pthread_mutex_unlock(&mutex);
        api_set(OnRtnDepthMarketDataFuture);
    }
}

void MdApi::init()
{
    char pwd[256];
    getcwd(pwd, 256);
    string dir = string(pwd).append("/api/ctp/md/");
    md_api = CThostFtdcMdApi::CreateFtdcMdApi(dir.c_str());
    md_api->RegisterSpi(this);
    md_api->RegisterFront((char *)front_addr);
    md_api->Init();
    cout << ".con file path: " << dir << endl;
}

void MdApi::exit()
{
    ReqUserLogout();
    md_api->RegisterSpi(NULL);
    md_api->Release();
    md_api = NULL;
    cout << "Md exit" << endl;
}

void MdApi::SubscribeMarketData()
{
    int iResult = md_api->SubscribeMarketData(&(ppInstrumentID[0]), 1);
    if (iResult != 0)
        cerr << "订阅行情[" << ppInstrumentID[0] << "]失败" << endl;
    else
        cout << "订阅行情[" << ppInstrumentID[0] << "]成功" << endl;
}

void MdApi::UnSubscribeMarketData()
{
    int iResult = md_api->UnSubscribeMarketData(&(ppInstrumentID[0]), 1);
    if (iResult != 0)
        cerr << "取消行情订阅[" << ppInstrumentID[0] << "]失败" << endl;
    else
        cout << "取消行情订阅[" << ppInstrumentID[0] << "]成功" << endl;
}

void MdApi::ReqUserLogin()
{
    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, broker_id);
    strcpy(req.UserID, user_id);
    strcpy(req.Password, password);
    int iResult = md_api->ReqUserLogin(&req, ++request_id);
    if (iResult != 0)
        cerr << "发送行情登录请求失败" << endl;
    else
        cout << "发送行情登录请求成功" << endl;
}

void MdApi::ReqUserLogout()
{
    CThostFtdcUserLogoutField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, broker_id);
    strcpy(req.UserID, user_id);
    int iResult = md_api->ReqUserLogout(&req, ++request_id);
    if (iResult != 0)
        cerr << "发送行情退出登录请求失败" << endl;
    else
        cout << "发送行情退出登录请求成功" << endl;
}

void MdApi::RegisterFront(char *pszFrontAddress)
{
    md_api->RegisterFront(pszFrontAddress);
}

extern "C"
{
    char * get_tick_instrument()
    {
        //memset(&tmp_instrument_id, 0, sizeof(TThostFtdcInstrumentIDTypeStruct));
        pthread_mutex_lock(&mutex);
        memcpy(tmp_instrument_id.Instrument, instrument_stack.top().Instrument, sizeof(TThostFtdcInstrumentIDType));
        instrument_stack.pop();
        pthread_mutex_unlock(&mutex);
        if (!instrument_stack.empty()) api_set(OnRtnDepthMarketDataFuture);
        return tmp_instrument_id.Instrument;
    }

    void subscribe(const char *InstrumentID)
    {
        strcpy(ppInstrumentID[0], InstrumentID);
        if (g_market.find(InstrumentID) == g_market.end())
        {
            g_market[InstrumentID] = new CThostFtdcDepthMarketDataField;
            g_md_api.SubscribeMarketData();
        }
    }

    void unsubscribe(const char *InstrumentID)
    {
        strcpy(ppInstrumentID[0], InstrumentID);
        g_md_api.UnSubscribeMarketData();
        if (g_market.find(InstrumentID) != g_market.end())
        {
            cout << "unsubscribe " << g_market[InstrumentID]->InstrumentID << endl;
            delete g_market[InstrumentID];
            g_market.erase(InstrumentID);
        }
    }

    bool is_api_ok()
    {
        return g_md_api.is_api_ok;
    }

    void init()
    {
        g_md_api.init();
    }

    void * get_tick_data(const char *InstrumentID)
    {
        if (g_market.find(InstrumentID) != g_market.end())
        {
            return g_market[InstrumentID];
        }
        else
        {
            return NULL;
        }
    }
}

//void my_wait()
//{
//    int i;
//    while (true)
//    {
//        i = api_wait();
//        if (i == OnRtnDepthMarketDataFuture)
//        {
//            char * a = get_tick_instrument();
//            cout << a << " " << g_market[a]->LastPrice << " " << get_time_str() << " " << is_api_ok() << endl;
//        }
//    }
//}
//
int main()
{
    char a;
    /*printf("main%s\n", front_addr);
    printf("main%s\n", broker_id);
    printf("main%s\n", user_id);
    printf("main%s\n", password);*/
    cout << "mainmain   " << front_addr << endl;
    cout << "mainmain   " << broker_id << endl;
    cout << "mainmain   " << user_id << endl;
    cout << "mainmain   " << password << endl;
    cin >> a;
    return 0;
}
