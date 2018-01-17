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
    void GetBidDeque(list<Order> &orderList);
    void GetOfferDeque(list<Order> &orderList);
    void GetTransList(list<Trans> &transList);
};

class Market{
protected:
    unsigned int barRange;
    unsigned int channelNum;
    time_t currentTime;
    string currentStr;
    string beginDateStr;   // "YYYYMMDD"
    string endDateStr;     // "YYYYMMDD"
    string currentDateStr; // "YYYYMMDD"
    string dataPath;
    set<Symbol> symbolSet;
    set<string> sourceSet;
    Order *orders;
    Stock **stocks;
    Channel *channels;
    list<BarMonitor> openMonList;
    list<BarMonitor> barMonList;
    list<BarMonitor> closeMonList;
    void FetchChannel(unsigned int channelId);
    void OrgFile();
    void InitForNewDay();
    void Clear();

public: 
    Market(string dataPath, string beginDateStr, string endDateStr, 
            set<Symbol> symbolSet, set<string> sourceSet, 
            unsigned int barRange = 300, unsigned int channelNum = 1);
    ~Market();
    void Launch();
    bool FetchDate();
    bool FetchTime();
    const string GetDateTimeStr();
    void GetOrderDequeStr(Symbol symbol, string &bidStr, string &offerStr);
    void GetOrderDeque(Symbol symbol, list<Order> &bidDeque, list<Order> &offerDeque);
    void GetTransList(Symbol symbol, list<Trans> &transList);
    void AddOpenMon(BarMonitor mon);
    void AddBarMon(BarMonitor mon);
    void AddCloseMon(BarMonitor mon);

    static bool IsStock(const string &symbol);
    static void GetStockFile(const string pathStr, set<string> &fileNameSet);
};

