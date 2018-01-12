#pragma once
#include "public.h"
#include "datasource.h"
#include <list>
#include <set>
using std::string;
using std::list;
using std::set;

class Market;
typedef void (*BarMonitor)(Market&); 

class Stock{
protected: 
    Symbol symbol;
    OrderDeque bidDeque;
    OrderDeque offerDeque;
    list<Trans> transList;

public: 
    Stock(Symbol symbol);
    ~Stock();
    void AddOrder(Order &order);
    void AddTrans(Trans trans);
    void ClearTrans();
    const string GetBidDequeStr();
    const string GetOfferDequeStr();
};

class Market{
protected:
    time_t currentTime;
    string currentStr;
    string beginDateStr;   // "YYYYMMDD"
    string endDateStr;     // "YYYYMMDD"
    string currentDateStr; // "YYYYMMDD"
    string dataPath;
    unsigned int channelNum;
    Order *orders;
    Stock **stocks;
    Channel *channels;
    list<BarMonitor> monList;
    void OrgFile();
    void InitForNewDay();
    void Clear();

public: 
    Market(string dataPath, string beginDateStr, string endDateStr);
    ~Market();
    void Launch();
    bool FetchDate();
    bool FetchTime();
    const string GetDateTimeStr();
    void GetOrderDequeStr(Symbol symbol, string &bidStr, string &offerStr);
    void AddMon(BarMonitor mon);

    static bool IsStock(const string &symbol);
    static void GetStockFile(const string pathStr, set<string> &fileNameSet);
};

