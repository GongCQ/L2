#include "ddi_min5.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <gsl/gsl_statistics_double.h>
using std::cout;
using std::endl;
using std::setw;
using std::setprecision;
using std::fixed;
using std::ios;
using std::pow;
using std::exception;
using std::out_of_range;
using std::range_error;
using std::swap;
using std::min;
using std::max;
using boost::filesystem::ofstream;
using boost::lexical_cast;
using boost::format;

const double EXTRA_PREMIUM = 0.03;

size_t DDI_MIN5::RESERVED_SIZE;
int DDI_MIN5::dateCount;
vector<string> DDI_MIN5::dateStrVec;
list<Symbol> DDI_MIN5::symbolList;

vector<vector<vector<double> > >  DDI_MIN5::returnRateVec;
vector<vector<vector<double> > >  DDI_MIN5::limitUpVec;
vector<vector<vector<double> > >  DDI_MIN5::limitDownVec;
vector<vector<vector<double> > >  DDI_MIN5::tradeVolumnVec;
vector<vector<vector<double> > >  DDI_MIN5::tradeAmountVec;
vector<vector<vector<double> > >  DDI_MIN5::bidOrderVolumnVec;
vector<vector<vector<double> > >  DDI_MIN5::offerOrderVolumnVec;
vector<vector<vector<double> > >  DDI_MIN5::bidDequeVolumnVec;
vector<vector<vector<double> > >  DDI_MIN5::offerDequeVolumnVec;
vector<vector<vector<double> > >  DDI_MIN5::initBuyVolumnVec;
vector<vector<vector<double> > >  DDI_MIN5::initSellVolumnVec;
vector<vector<vector<double> > >  DDI_MIN5::cancelBidVolumnVec;
vector<vector<vector<double> > >  DDI_MIN5::cancelOfferVolumnVec;
vector<vector<vector<double> > >  DDI_MIN5::extraPremiumBuyVolumnVec;
vector<vector<vector<double> > >  DDI_MIN5::extraPremiumSellVolumnVec;

vector<vector<vector<vector<double> > > *> DDI_MIN5::allVec;

void DDI_MIN5::Init(size_t reservedSize){
    RESERVED_SIZE = reservedSize;

    dateCount = -1;
    dateStrVec.resize(RESERVED_SIZE);

    allVec.push_back(&returnRateVec);
    allVec.push_back(&limitUpVec);
    allVec.push_back(&limitDownVec);
    allVec.push_back(&tradeVolumnVec);
    allVec.push_back(&tradeAmountVec);
    allVec.push_back(&bidOrderVolumnVec);
    allVec.push_back(&offerOrderVolumnVec);
    allVec.push_back(&bidDequeVolumnVec);
    allVec.push_back(&offerDequeVolumnVec);
    allVec.push_back(&initBuyVolumnVec);
    allVec.push_back(&initSellVolumnVec);
    allVec.push_back(&cancelBidVolumnVec);
    allVec.push_back(&cancelOfferVolumnVec);
    allVec.push_back(&extraPremiumBuyVolumnVec);
    allVec.push_back(&extraPremiumSellVolumnVec);

    for(int v = 0; v < allVec.size(); v++){
        vector<vector<vector<double> > > &vec = *(allVec.at(v));
        vec.resize(MAX_SYMBOL);
    }

    cout<<"DDI_MIN5 initialized, "<<endl
        <<"RESERVED_SIZE "<<RESERVED_SIZE<<endl
        <<"the number of bars "<<GetBarNum()<<endl
        <<"the size of allVec "<<allVec.size()<<endl
        <<"the number of factors "<<GetFactorNum()<<endl
        ;
}

void DDI_MIN5::Open(Market &market){
    dateCount++;
    if(dateCount >= RESERVED_SIZE){
        cout<<"max reserved size has reached, exit."<<endl;
        throw range_error("max reserved size has reached.");
    }
    dateStrVec.at(dateCount) = market.GetDateTimeStr();

    cout<<"open: "<<market.GetDateTimeStr()<<endl;
    symbolList.clear();
    market.GetSymbolList(symbolList);
    cout<<"stock number is "<<symbolList.size()<<endl;

    for(int v = 0; v < allVec.size(); v++){
        vector<vector<vector<double> > > &factorVec = *(allVec.at(v));
        // initialize each stock
        for(list<Symbol>::const_iterator symbol = symbolList.begin(); symbol != symbolList.end(); symbol++){
            vector<vector<double> > &stockVec = factorVec.at(*symbol);
            if(stockVec.size() == 0){ // if this stock has never appear before
                stockVec.resize(RESERVED_SIZE);
                // initialize each date
                for(size_t d = 0; d < RESERVED_SIZE; d++){
                    stockVec.at(d) = vector<double>(GetBarNum());
                    // initialize each 5 minutes bar
                    for(int b = 0; b < GetBarNum(); b++){
                        stockVec.at(d).at(b) = 0;
                    }
                }
            }
        }
    }
}

void DDI_MIN5::Bar(Market &market){
    const string timeStr = market.GetDateTimeStr();
    int hour = atoi(timeStr.substr(9, 2).c_str());
    int minute = atoi(timeStr.substr(11, 2).c_str());
    int second = atoi(timeStr.substr(13, 2).c_str());
    int barTime = hour * 100 + minute;
    int barSeq = GetBarSeq(hour, minute);
    cout<<"bar: "<<market.GetDateTimeStr()<<"  bar seq: "<<barSeq<<endl;

    for(list<Symbol>::const_iterator sym = symbolList.begin(); sym != symbolList.end(); sym++){   
        Symbol symbol = *sym;         
        list<Order> bidDeque, offerDeque;
        list<Trans> transList;
        list<Order> orderList;
        market.GetOrderDeque(symbol, bidDeque, offerDeque);
        market.GetTransList(symbol, transList);
        market.GetOrderList(symbol, orderList);

        double firstPrice = 0;
        double lastPrice = 0;
        bool limited = true;
        bool existInitBuy = false;
        bool existInitSell = false;

        double returnRate = 0;
        double limitUp = 0;
        double limitDown = 0;
        double tradeVolumn = 0;
        double tradeAmount = 0;
        double bidOrderVolumn = 0;
        double offerOrderVolumn = 0;
        double bidDequeVolumn = 0;
        double offerDequeVolumn = 0;
        double initBuyVolumn = 0;
        double initSellVolumn = 0;
        double cancelBidVolumn = 0;
        double cancelOfferVolumn = 0;
        double extraPremiumBuyVolumn = 0;
        double extraPremiumSellVolumn = 0;

        for(list<Trans>::const_iterator trans = transList.begin(); trans != transList.end(); trans++){
            const TransInfo *transInfo = trans->GetInfo();
            if(transInfo == nullptr){
                cout<<"a invalid trans "<<trans->ToStr()<<endl;
                continue;
            }
            Order bidOrder = market.GetOrder(transInfo->bidSeq);
            Order offerOrder = market.GetOrder(transInfo->offerSeq);

            if(transInfo->type == 'F' && bidOrder.IsValid() && offerOrder.IsValid()) {  // transact
                const OrderInfo *bidOrderInfo = bidOrder.GetInfo(); 
                const OrderInfo *offerOrderInfo = offerOrder.GetInfo();
                double price = double(transInfo->price) / 100;
                double volumn = transInfo->num;
                double amount = price * volumn;
                double bidPrice = double(bidOrderInfo->price) / 100;
                double offerPrice = double(offerOrderInfo->price) / 100;

                firstPrice = firstPrice == 0 ? price : firstPrice;
                lastPrice = price;
                limited = firstPrice == 0 || (firstPrice != 0 && price == firstPrice);
                existInitBuy = transInfo->bidSeq < transInfo->offerSeq;
                existInitSell = transInfo->offerSeq < transInfo->bidSeq;

                tradeVolumn += volumn;
                tradeAmount += amount;

                initBuyVolumn += bidOrderInfo->seq > offerOrderInfo->seq ? volumn : 0;
                initSellVolumn += bidOrderInfo->seq < offerOrderInfo->seq ? volumn : 0;

                extraPremiumBuyVolumn += bidPrice >= price * (1 + EXTRA_PREMIUM) ? volumn : 0;
                extraPremiumSellVolumn += offerPrice <= price * (1 - EXTRA_PREMIUM) ? volumn : 0; 
            }

            if(transInfo->type == '4') { // cancel order
                cancelBidVolumn += transInfo->bidSeq > 0 ? transInfo->num : 0;
                cancelOfferVolumn += transInfo->offerSeq > 0 ? transInfo->num : 0;
            }
        }

        for(list<Order>::const_iterator order = orderList.begin(); order != orderList.end(); order++){
            const OrderInfo *orderInfo = order->GetInfo();
            if(!order->IsValid()){
                cout<<"a invalid order "<<order->ToStr()<<endl;
                continue;
            }
            bidOrderVolumn += orderInfo->direct == '1' ? orderInfo->num : 0;
            offerOrderVolumn += orderInfo->direct == '2' ? orderInfo->num : 0;
        }

        for(list<Order>::const_iterator order = bidDeque.begin(); order != bidDeque.end(); order++){
            const OrderInfo *orderInfo = order->GetInfo();
            bidDequeVolumn += order->IsValid() ? orderInfo->num : 0;
        }
        for(list<Order>::const_iterator order = offerDeque.begin(); order != offerDeque.end(); order++){
            const OrderInfo *orderInfo = order->GetInfo();
            offerDequeVolumn += order->IsValid() ? orderInfo->num : 0;
        }

        returnRate = firstPrice == 0 ? 0 : (lastPrice / firstPrice - 1);
        limitUp = (limited && !existInitBuy) ? 1 : 0;
        limitDown = (limited && !existInitSell) ? 1 : 0;
        
        returnRateVec.at(symbol).at(dateCount).at(barSeq) = returnRate;
        limitUpVec.at(symbol).at(dateCount).at(barSeq) = limitUp;
        limitDownVec.at(symbol).at(dateCount).at(barSeq) = limitDown;
        tradeVolumnVec.at(symbol).at(dateCount).at(barSeq) = tradeVolumn;
        tradeAmountVec.at(symbol).at(dateCount).at(barSeq) = tradeAmount;
        bidOrderVolumnVec.at(symbol).at(dateCount).at(barSeq) = bidOrderVolumn;
        offerOrderVolumnVec.at(symbol).at(dateCount).at(barSeq) = offerOrderVolumn;
        bidDequeVolumnVec.at(symbol).at(dateCount).at(barSeq) = bidDequeVolumn;
        offerDequeVolumnVec.at(symbol).at(dateCount).at(barSeq) = offerDequeVolumn;
        initBuyVolumnVec.at(symbol).at(dateCount).at(barSeq) = initBuyVolumn;
        initSellVolumnVec.at(symbol).at(dateCount).at(barSeq) = initSellVolumn;
        cancelBidVolumnVec.at(symbol).at(dateCount).at(barSeq) = cancelBidVolumn;
        cancelOfferVolumnVec.at(symbol).at(dateCount).at(barSeq) = cancelOfferVolumn;
        extraPremiumBuyVolumnVec.at(symbol).at(dateCount).at(barSeq) = extraPremiumBuyVolumn;
        extraPremiumSellVolumnVec.at(symbol).at(dateCount).at(barSeq) = extraPremiumSellVolumn;
    }
}

void DDI_MIN5::Close(Market &market){
    cout<<"close: "<<market.GetDateTimeStr()<<endl;
}

void DDI_MIN5::FactorToFile(const string &path, int saveDays){
    int columnWide = 18;
    int precision = 4;
    int beginDateSeq = saveDays > 0 ? max(0, dateCount - saveDays + 1) : 0;
    for(int factorSeq = 0; factorSeq < GetFactorNum(); factorSeq++){
        const string factorName = GetFactorName(factorSeq);
        for(int dateSeq = beginDateSeq; dateSeq <= dateCount; dateSeq++){
            const string dateStr = dateStrVec.at(dateSeq).substr(0, 8);
            ofstream file(path + "factor_min5_" + dateStr + "_MIN5" + factorName + ".csv");
            file<<fixed;

            file<<setw(6)<<"symbol";
            for(int barSeq = 0; barSeq < GetBarNum(); barSeq++){
                file<<","<<setw(columnWide)<<GetBarTimeStr(barSeq);
            }
            file<<endl;

            for(list<Symbol>::const_iterator sym = symbolList.begin(); sym != symbolList.end(); sym++) {
                Symbol symbol = *sym;
                if(symbol >= 600000 || allVec.at(factorSeq)->at(symbol).size() != RESERVED_SIZE){
                    continue;
                }
                file<<setw(6)<<Market::GetSymbolStr(symbol);
                for(int barSeq = 0; barSeq < GetBarNum(); barSeq++){
                    double value = allVec.at(factorSeq)->at(symbol).at(dateSeq).at(barSeq);
                    file<<","<<setw(columnWide)<<setprecision(precision)<<value;
                }
                file<<endl;
            }

            file.flush();
            file.close();
        }
    }
}

const int DDI_MIN5::GetBarNum(){
    return 53;
}

const int DDI_MIN5::GetBarSeq(int hour, int minute){ 
    int barTime = hour * 100 + minute;
    if(barTime >= 915 && barTime <= 1130){
        return (hour - 9) * 12 + (minute - 15) / 5;
    }
    else if(barTime >= 1300 && barTime <= 1500){
        return 28 + (hour - 13) * 12 + minute / 5;
    }
    else{
        throw range_error("unknown bar time " + lexical_cast<string>(barTime));
    }
}

const int DDI_MIN5::GetBarTimeInt(int barSeq){
    static int barTime[] = { 915, 920, 925,  
                             930,  
                             935,  940,  945,  950,  955, 1000, 1005, 1010, 1015, 1020, 1025, 1030, 
                            1035, 1040, 1045, 1050, 1055, 1100, 1105, 1110, 1115, 1120, 1125, 1130,
                            1300,
                            1305, 1310, 1315, 1320, 1325, 1330, 1335, 1340, 1345, 1350, 1355, 1400, 
                            1405, 1410, 1415, 1420, 1425, 1430, 1435, 1440, 1445, 1450, 1455, 1500};
    if(barSeq < 0 || barSeq >= 53){
        throw out_of_range("invalid bar seq.");
    }
    
    return barTime[barSeq];
}

const string DDI_MIN5::GetBarTimeStr(int barSeq){
    int barTimeInt = GetBarTimeInt(barSeq);
    format fmt("%04d");
    return (fmt % barTimeInt).str();
}

const int DDI_MIN5::GetFactorNum(){
    return allVec.size();
}

const string DDI_MIN5::GetFactorName(int factorSeq){
    static const string factorNames[] = 
        {"收益率",
         "涨停",
         "跌停",
         "成交量",
         "成交额",
         "委买量",
         "委卖量",
         "盘口委买量",
         "盘口委卖量",
         "主动买入量",
         "主动卖出量",
         "撤买量",
         "撤卖量",
         "异常溢价买入量",
         "异常溢价卖出量"};

    return factorNames[factorSeq];
}