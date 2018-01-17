#pragma once
#include <string>
#include <boost/date_time.hpp>
#include "log.h"
using std::string;
using std::list;
using std::time_t;

typedef unsigned long long OrderSeq;
typedef unsigned long long TransSeq;
typedef unsigned int Symbol;
typedef unsigned int Price;
extern const OrderSeq MAX_ORDER_SEQ;
extern const OrderSeq INVALID_ORDER_SEQ;
extern const TransSeq MAX_TRANS_SEQ;
extern const TransSeq INVALID_TRANS_SEQ;
extern const Symbol INVALID_SYMBOL;
extern const Symbol MAX_SYMBOL;
extern const Price INVALID_PRICE;
extern const time_t INVALID_TIME;
extern const time_t MIN_TIME;
extern const float PRICE_SCOPE;
extern const boost::posix_time::ptime LAUNCH_TIME;
extern const string LAUNCH_TIME_STR;
extern const unsigned int DAY_SECONDS;
extern const unsigned int HOUR_SECONDS;
extern const unsigned int MINUTE_SECONDS;
extern Log logFile; 
