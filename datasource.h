#pragma once
#include <ctime>
#include <vector>
#include <list>
#include <boost/filesystem.hpp>
#include "public.h"
using std::string;
using std::vector;
using std::list; 

OrderSeq ChannelSeq(int channel, OrderSeq seq);

struct OrderInfo{
	class Order{
	protected:
		OrderInfo* info;	
		void* operator new(size_t s);
		void operator delete(void* s);
		Order* operator &();

	public:
		//构造一个空委托
		Order();
		//构造一个无效委托
		Order(Symbol symbol, time_t dt);
		//构造一个委托
		Order(Symbol symbol, time_t dt, int channel, OrderSeq seq, float price, unsigned int num, char direct, char type);
		Order(const Order &order);
		Order &operator=(const Order& order);

		bool operator==(const Order& order) const;
		bool operator!=(const Order& order) const;

		~Order();

		//撤单，num为撤单数量，返回撤单后剩余数量
		int Cancel(int num);
		//成交，num为成交数量，返回成交后剩余数量
		int Trans(int num);
		//在委托队列中移除本委托（但不析构）
		void Remove();	
		//转换为字符串
		const string ToStr() const;

		const OrderInfo *GetInfo();

		bool IsEmpty() const;
		bool IsValid() const;
		bool IsEqual(const Order& order) const;

		Order &GetNext() const;
		Order &GetPrior() const;
		void SetNext(Order order);
		void SetPrior(Order order);
	};

    Symbol symbol;
	time_t dt;	//委托时间
	int channel;	//通道号
	OrderSeq seq;	//委托序号(全局)
	OrderSeq cSeq; //委托序号(通道内)
	Price price;	//委托价格×100，整数
	unsigned int num;	//委托数量
	char direct;	//委托方向，1买，2卖，G借入，F出借
	char type;	//委托类型，1市价，2限价，U本方最优
	unsigned int transNum;	//已成交数量
	unsigned int cancelNum;	//已撤单数量
	unsigned int refCount;	//引用计数
	Order prior; 
	Order next;

    const string ToStr() const;
};
typedef OrderInfo::Order Order;

struct TransInfo{
	class Trans{
	protected:
		TransInfo* info;	
		void* operator new(size_t s);
		void operator delete(void* s);
		Trans* operator &();

	public:
		//构造一个空成交记录
		Trans();
		//构造一个无效成交记录
		Trans(Symbol symbol, time_t dt);
		//构造一个成交记录
		Trans(Symbol symbol, time_t dt, int channel, TransSeq seq, OrderSeq bidSeq, OrderSeq offerSeq, float price, unsigned int num, char type);
		Trans(const Trans &trans);
		Trans& operator=(const Trans& trans);

		~Trans();

		const string ToStr() const;

		const TransInfo *GetInfo();

		bool IsEmpty();

		bool IsValid();
	};

    Symbol symbol;
	time_t dt;	//成交时间
	int channel;	//通道号
    TransSeq seq;	//成交序号(全局)
	TransSeq cSeq;  //成交序号(通道内)
	OrderSeq bidSeq;	//委买序号(全局)
	OrderSeq cBidSeq;  	//委买序号(通道内)
    OrderSeq offerSeq;	//委卖序号(全局)
	OrderSeq cOfferSeq; //委卖序号(通道内)
	Price price;	//成交价格
	unsigned int num;	//成交数量
	char type;	//执行类型 F成交，4撤单
	unsigned int refCount;	//引用计数

	const string ToStr() const;
};
typedef TransInfo::Trans Trans;

class OrderStream{
protected:
	Symbol symbol; 
	string path;
	boost::filesystem::ifstream orderFile;  
	 
public:
	OrderStream();
	~OrderStream();
	void Init(Symbol symbol, const string &path);
	Order StrToOrder(const string &str);
	bool GetOrder(Order& order);
};

class TransStream{
protected:
	Symbol symbol;
	string path;
	boost::filesystem::ifstream transFile;

public:
	TransStream();
	~TransStream();
	void Init(Symbol symbol, const string &path);
	Trans StrToTrans(const string &str); 
	bool GetTrans(Trans& trans);
};

class OrderList{
protected:
	Order head;
	Order tail;
	Order loc;

public: 
	OrderList();
	~OrderList();
	void LocFirst();
	bool GetEach(Order& order);
	void AddOrder(Order order);
	bool IsEmpty();
	const string ToStr();
};

class OrderDeque{
protected:
	Symbol symbol;
	bool direct; // true: bid, false: offer
	vector<OrderList> priceVec;
	Price basePrice;
	Price maxPrice;
	Price minPrice;
	unsigned int loc;
	time_t lastTime;

	void InitNewDay(Price firstPrice);
	unsigned int PriceToIndex(Price price);
	Price IndexToPrice(unsigned int index);

public:
	OrderDeque(Symbol symbol, bool direct);
	~OrderDeque();
	void AddOrder(Order &order);
	Price GetOptPrice();
	const string ToStr();
	void LocFirst();
	bool GetEach(Order& order); 
};

class Channel{
protected:
	vector<string> orderPathVec;
	vector<string> transPathVec;
	vector<OrderStream*> orderStreamVec;
	vector<TransStream*> transStreamVec;
	vector<Order> orderBufferVec;
	vector<Trans> transBufferVec;
	unsigned int orderLoc;
	unsigned int transLoc;

public:
	Channel();
	~Channel();
	void LocFirst();
	void AddOrderStream(const string &path, Symbol symbol);
	void AddTransStream(const string &path, Symbol symbol);
	bool GetOrder(time_t time, Order &order);
	bool GetTrans(time_t time, Trans &trans);
};