#include "public.h"
#include <string>
#include <boost/date_time.hpp>
using namespace std;
using namespace boost::posix_time;
using boost::filesystem::path;
using boost::filesystem::directory_iterator;

const OrderSeq MAX_ORDER_SEQ = 300000000;
const OrderSeq INVALID_ORDER_SEQ = MAX_ORDER_SEQ - 1;
const TransSeq MAX_TRANS_SEQ = 300000000;
const TransSeq INVALID_TRANS_SEQ = MAX_TRANS_SEQ - 1;
const Symbol INVALID_SYMBOL = 999999;
const Symbol MAX_SYMBOL = 1000000;
const Symbol INVALID_PRICE = 0;
const time_t INVALID_TIME = 0;
const time_t MIN_TIME = 0;
const float PRICE_SCOPE = 0.3;
const ptime LAUNCH_TIME = microsec_clock::local_time();
const string LAUNCH_TIME_STR = to_iso_string(LAUNCH_TIME);
const unsigned int DAY_SECONDS = 86400;
const unsigned int HOUR_SECONDS = 3600;
const unsigned int MINUTE_SECONDS = 60;
Log logFile("./log/"); 
