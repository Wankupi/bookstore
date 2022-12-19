#include "Finance.h"

static void when_finance_empty(std::fstream &fs) {
	Payment pay{};
	fs.write(reinterpret_cast<char *>(&pay), sizeof(Payment));
	pay.type = Payment::PaymentType::Import;
	fs.write(reinterpret_cast<char *>(&pay), sizeof(Payment));
}

Finance::Finance(const std::string &file_pay)
	: db(file_pay, when_finance_empty) {}

void Finance::log_buy(double cost) {
	Payment pay{Payment::PaymentType::Buy, cost};
	db.insert(pay);
	std::lock_guard lock(total_lock);
	pay = db.read(1);
	pay.val += cost;
	db.write(1, pay);
}

void Finance::log_import(double cost) {
	Payment pay{Payment::PaymentType::Import, cost};
	db.insert(pay);
	std::lock_guard lock(total_lock);
	pay = db.read(2);
	pay.val += cost;
	db.write(2, pay);
}

std::pair<double, double> Finance::show(int count) {
	std::pair<double, double> ret;
	int n = db.size();
	if (count > n - 2) return {-1, -1};
	for (int i = 1; i <= count; ++i) {
		Payment pay = db.read(n - count + i);
		if (pay.type == Payment::PaymentType::Buy)
			ret.first += pay.val;
		else
			ret.second += pay.val;
	}
	return ret;
}

std::pair<double, double> Finance::show() {
	return {db.read(1).val, db.read(2).val};
}
