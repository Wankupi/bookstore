#pragma once
#ifndef BOOKSTORE_INSTANCEDAEMON_H
#define BOOKSTORE_INSTANCEDAEMON_H
#include <filesystem>
#include <string>
class InstanceDaemon {
public:
	explicit InstanceDaemon(std::string const &filename) : filename(filename), stat(false) {
		FILE *file = fopen(filename.c_str(), "wx");
		if (file) {
			stat = true;
			fclose(file);
		}
	}
	bool status() const { return stat; }
	~InstanceDaemon() {
		if (stat) {
			int ret = std::remove(filename.c_str());
		}
	}

private:
	std::string filename;
	bool stat;
};

#endif // BOOKSTORE_INSTANCEDAEMON_H
