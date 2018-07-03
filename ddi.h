#pragma once 
#include <vector>
#include <list>
#include "public.h"
#include "market.h"
using std::vector;
using std::list;

extern const long long BIG_AMOUNT;

void isort(vector<double> &vec, vector<size_t> &ivec);

vector<size_t> isort(vector<double> &vec);

class DDI{
public: 
    static size_t RESERVED_SIZE;
    static string cutOffStr;
    static int cutOffHour;
    static int cutOffMinute;
    static int dateCount;
    static vector<string> dateStrVec;
    static list<Symbol> symbolList;
    static vector<vector<long long> > openVec;              // 0
    static vector<vector<long long> > closeVec;             // 1
    static vector<vector<long long> > limitUpVec;           // 2
    static vector<vector<long long> > bigTradeInVec;        // 3 amount
    static vector<vector<long long> > bigTradeOutVec;       // 4 amount
    static vector<vector<long long> > bigOrderInVec;        // 5 number
    static vector<vector<long long> > bigOrderOutVec;       // 6 number
    static vector<vector<long long> > totalOrderInVec;      // 7 number
    static vector<vector<long long> > totalOrderOutVec;     // 8 number
    static vector<vector<long long> > bigCancelInVec;       // 9 number
    static vector<vector<long long> > bigCancelOutVec;      // 10 number
    static vector<vector<long long> > transTotalNumVec;     // 11 number
    static vector<vector<long long> > transTotalAmountVec;  // 12 amount
    static vector<vector<long long> > bigEatSmallAmountVec; // 13 
    static vector<vector<long long> > extraUpOrderAmountVec;// 14
    static vector<vector<long long> > extraDownOrderAmountVec;// 15
    static vector<vector<long long> > limitUpOrderAmountVec;// 16
    static vector<vector<long long> > limitUpOrderVolVec;   // 17

    static vector<vector<vector<long long> > *> allVec;  // all
    static vector<string> allName;    

    static void Init(size_t reservedSize, const string &cutOff);
    static void Open(Market &market);
    static void Bar(Market &market);
    static void Close(Market &market);
    static void PrimaryFactorToFile(const string &path, int saveDays);
    static void DerivativeFactorToFile(const string &path, int saveDays);
    static void Summary(int returnDays, int method, int top);
    static bool StockPicking(const int factorId, const Symbol symbol, const int dateSeq, double &value);

    static const int GetFactorNum();
    static const string GetFactorName(int factorId);
    static const double GetFactorCutOff(int factorId);
    static const double GetFactorValue(Symbol symbol, int dateSeq, int factorId);
};