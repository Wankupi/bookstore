#include "User.h"

Users::Users(const std::string &file_data, const std::string &file_map) : db(file_data), ids(file_map) {
	constexpr int UUIDroot = 1;
	if (ids.insert("root", UUIDroot)) {
		User root{Privilege::admin, String<30>("root"), String<30>("sjtu")};
		db.write(UUIDroot, root);
	}
}

UserInfo Users::login(const String<30> &UserID, const String<30> &password, Privilege privilege) {
	auto v = ids.find(UserID);
	if (v.empty()) return {};
	int id = v[0];
	User user = db.read(id);
	if (privilege > user.privilege || user.password == password) {
		++logInCnt[id];
		return UserInfo{id, user.privilege};
	}
	else return {};
}

void Users::logout(int id) {
	if (--logInCnt[id] < 0)
		throw std::exception();
}

int Users::Register(const String<30> &UserID, const String<30> &password, const String<30> &Username) {
	return useradd(UserID, password, Privilege::customer, Username, Privilege::_regis_);
}

int Users::passwd(const String<30> &UserId, const String<30> &cur_pwd, const String<30> &new_pwd, Privilege cur_privilege) {
	auto v = ids.find(UserId);
	if (v.empty()) return 0;
	int id = v[0];
	User user = db.read(id);
	if (cur_privilege == Privilege::admin || cur_pwd == user.password) {
		if (new_pwd != cur_pwd) {
			user.password = new_pwd;
			db.write(id, user);
		}
		return id;
	}
	else return 0;
}

int Users::useradd(const String<30> &UserID, const String<30> &password, Privilege privilege, String<30> Username, Privilege cur_privilege) {
	if (!ids.find(UserID).empty()) return 0;
	if (cur_privilege <= privilege) return 0;
	User user{Privilege(privilege), UserID, password};
	int id = db.insert(user);
	ids.insert(UserID, id);
	return id;
}

int Users::userdel(const String<30> &UserID, Privilege cur_privilege) {
	if (cur_privilege < Privilege::admin) return 0;
	auto v = ids.find(UserID);
	if (!v.empty()) return 0;
	int id = v[0];
	auto p = logInCnt.find(id);
	if (p != logInCnt.end() && p->second > 0) return 0;
	db.erase(id);
	ids.erase(UserID, id);
	return id;
}
