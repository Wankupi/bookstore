#pragma
#ifndef BOOKSTORE_FINANCE_H
#define BOOKSTORE_FINANCE_H
#include "../file/DataBase.h"

//;
//#pragma pack(push, 1)
struct Payment {
	enum class PaymentType : char {
		Buy = 0,
		Import = 1
	} type;
	double val;
};
//#pragma pack(pop)

class Finance {
public:
	Finance(std::string const &file_pay);
	void log_buy(double cost);
	void log_import(double cost);
	std::pair<double, double> show(int count);
	std::pair<double, double> show();
private:
	DataBase<Payment> db;
	std::mutex total_lock;
};

#endif // BOOKSTORE_FINANCE_H
