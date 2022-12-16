#pragma once
#ifndef BOOKSTORE_USER_H
#define BOOKSTORE_USER_H
#include "../base.h"
#include "../file/DataBase.h"
#include "../file/FileMap.h"
#include <atomic>
#pragma pack(push, 1)
enum class Privilege : char {
	logout = 0,
	customer = 1,
	_regis_ = 2,
	staff = 3,
	admin = 7
};

struct UserInfo {
	int id;
	Privilege privilege;
};

struct User {
	Privilege privilege;
	String<30> id;
	String<30> password;
};
#pragma pack(pop)

class Users {
public:
	/**
	 * @param file_data
	 * @param file_map
	 */
	Users(std::string const &file_data, std::string const &file_map);
	/**
	 * @return the logged user's id and privilege. id is 0 if failed.
	 * @attention require privilege 0
	 */
	UserInfo login(String<30> const &UserID, String<30> const &password, Privilege privilege);
	/**
	 * @param id the one to log out
	 * @attention must make sure id has logged in.
	 */
	void logout(int id);
	/**
	 * @return the uuid of the new added user. 0 if failed.
	 * @attention require privilege 0
	 */
	int Register(String<30> const &UserID, String<30> const &password, String<30> const &Username);
	/**
	 * @return the uuid of the user. 0 if failed.
	 * @attention require privilege 1
	 */
	int passwd(String<30> const &UserId, String<30> const &cur_pwd, String<30> const &new_pwd, Privilege cur_privilege);
	/**
	 * @return the uuid of the new added user. 0 if failed.
	 * @attention require privilege 3
	 */
	int useradd(String<30> const &UserID, String<30> const &password, Privilege privilege, String<30> Username, Privilege cur_privilege);
	/**
	 * @return the uuid of the deleted user. 0 if failed.
	 * @attention require privilege 7
	 */
	int userdel(String<30> const &UserID, Privilege cur_privilege);

private:
	DataBase<User, true> db;
	FileMap<String<30>, int> ids;
	std::unordered_map<int, std::atomic_int> logInCnt;
};

#endif // BOOKSTORE_USER_H
