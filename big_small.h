#pragma once
#include <string>
#include <vector>
#include <list>
#include "public.h"
#include "market.h"
using std::string;
using std::vector;
using std::list;

extern const double AMOUNT_LIMIT;
extern const double DIV_ARR[]; 
extern const unsigned int DIV_SIZE;

unsigned int GetAmountInterval(double amount);

class BigSmall{
public:
    static size_t RESERVED_SIZE;
    static string cutOffStr;
    static int cutOffHour;
    static int cutOffMinute;
    static int dateCount;
    static vector<string> dateStrVec;
    static list<Symbol> symbolList;

    static vector<vector<vector<double> > > buyVolVec; // 1.div, 2.symbol, 3.days
    static vector<vector<vector<double> > > sellVolVec;
    static vector<vector<vector<double> > > quitsVolVec;
    static vector<vector<vector<double> > > buyAmountVec;
    static vector<vector<vector<double> > > sellAmountVec;
    static vector<vector<vector<double> > > quitsAmountVec;

    static const string GetHeaderLine(const int columnWide);

    static void Init(size_t reservedSize, const string cutOff);
    static void Open(Market &market);
    static void Bar(Market &market);
    static void Close(Market &market);

    static void FactorToFile(const string &path, int saveDays);
};