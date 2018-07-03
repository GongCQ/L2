#pragma once
#include <string>
#include <vector>
#include <list>
#include "public.h"
#include "market.h"
using std::string;
using std::vector;
using std::list;

extern const int MIN5_BAR_NUM;
extern const double EXTRA_PREMIUM;

class DDI_MIN5{
public:
    static size_t RESERVED_SIZE;
    static int dateCount;
    static vector<string> dateStrVec;
    static list<Symbol> symbolList;

    static vector<vector<vector<double> > > returnRateVec;
    static vector<vector<vector<double> > > limitUpVec;
    static vector<vector<vector<double> > > limitDownVec;
    static vector<vector<vector<double> > > tradeVolumnVec;
    static vector<vector<vector<double> > > tradeAmountVec;
    static vector<vector<vector<double> > > bidOrderVolumnVec;
    static vector<vector<vector<double> > > offerOrderVolumnVec;
    static vector<vector<vector<double> > > bidDequeVolumnVec;
    static vector<vector<vector<double> > > offerDequeVolumnVec;
    static vector<vector<vector<double> > > initBuyVolumnVec;
    static vector<vector<vector<double> > > initSellVolumnVec;
    static vector<vector<vector<double> > > cancelBidVolumnVec;
    static vector<vector<vector<double> > > cancelOfferVolumnVec;
    static vector<vector<vector<double> > > extraPremiumBuyVolumnVec;
    static vector<vector<vector<double> > > extraPremiumSellVolumnVec;
    
    static vector<vector<vector<vector<double> > > *> allVec;

    static void Init(size_t reservedSize);
    static void Open(Market &market);
    static void Bar(Market &market);
    static void Close(Market &market);
    static void FactorToFile(const string &path, int savedays);

    static const int GetBarNum();
    static const int GetBarSeq(int hour, int minute);
    static const int GetBarTimeInt(int barSeq);
    static const string GetBarTimeStr(int barSeq);
    static const int GetFactorNum();
    static const string GetFactorName(int factorSeq);
};