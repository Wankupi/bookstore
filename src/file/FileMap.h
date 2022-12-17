#pragma
#ifndef BOOKSTORE_FILEMAP_H
#define BOOKSTORE_FILEMAP_H
#include "DataBase.h"
#include <vector>
#include <algorithm>
#include <shared_mutex>
#include <unordered_map>

namespace kupi {

#pragma pack(push, 1)
template <typename IndexType, typename ValueType>
struct Data_ {
	IndexType index;
	ValueType value;
	bool operator<(Data_ const &B) const { return index == B.index ? value < B.value : index < B.index; }
	bool operator<=(Data_ const &B) const { return !(B < *this); }
	bool operator==(Data_ const &B) const { return value == B.value && index == B.index; }
	bool operator!=(Data_ const &B) const { return !(*this == B); }
};

struct NodeHeader {
	NodeHeader() : next(0), size(0) {}
	int next;
	int size;
};

template <typename IndexType, typename ValueType>
struct Node_ {
	static constexpr int MinBlock = 400;
	static constexpr int MaxBlock = MinBlock * 2;
	NodeHeader header;
	Data_<IndexType, ValueType> data[MaxBlock];
};
#pragma pack(pop)

template <typename IndexType, typename ValueType>
class FileMap : private DataBase<Node_<IndexType, ValueType>, true> {
	using Base = DataBase<Node_<IndexType, ValueType>, true>;
	using Data = Data_<IndexType, ValueType>;
	using Node = Node_<IndexType, ValueType>;
public:
	FileMap(std::string const &filename) : Base(filename, FileMap::write_1st_node) {}
	/**
	 * @attention lock the Block one by one from left to right
	 */
	bool insert(IndexType const &key, ValueType const &value);
	/**
	 * @attention lock the Block one by one from left to right
	 */
	bool erase(IndexType const &key, ValueType const &value);
	/**
	 * @attention lock the Block one by one from left to right
	 */
	std::vector<ValueType> find(IndexType const &key);
	std::vector<std::vector<Data>> all() {
		std::vector<std::vector<Data>> ret;
		int p = 1;
		while (p) {
			std::vector<Data> t;
			Node node = this->read(p);
			for (int i = 0; i < node.header.size; ++i)
				t.push_back(node.data[i]);
			ret.emplace_back(std::move(t));
			p = node.header.next;
		}
		return ret;
	}

private:
	/**
	 * @brief used for initialize an empty file
	 */
	static void write_1st_node(std::fstream &fs) {
		for (int i = 0; i < sizeof(Node); ++i)
			fs.put(0);
	}
	/**
	 * @brief insert obj to a certain block numbered `id`.
	 * @attention do not set `block_lock`, since insert(key,value) have done it.
	 */
	bool insert(Data const &obj, int id);
	/**
	 * @brief erase obj from a certain block numbered `id`.
	 * @attention will use `block_lock` for the next block when merge.
	 */
	bool erase(Data const &obj, int id);
	/**
	 * @brief read the header of the Block numbered `id`
	 * @attention use `file_lock`
	 */
	NodeHeader read_block_header(int id);
	/**
	 * @brief read the kth Data of the Block numbered `id`
	 * @attention use `file_lock`
	 */
	Data read_kth_of_node(int id, int k);
private:
	std::unordered_map<int, std::shared_mutex> block_lock;
};

template <typename IndexType, typename ValueType>
NodeHeader FileMap<IndexType, ValueType>::read_block_header(int id) {
	NodeHeader ret;
	std::lock_guard lock(this->file_lock);
	this->file.seekg((id - 1) * sizeof(Node));
	this->file.read(reinterpret_cast<char *>(&ret), sizeof(NodeHeader));
	return ret;
}

template <typename IndexType, typename ValueType>
typename FileMap<IndexType, ValueType>::Data FileMap<IndexType, ValueType>::read_kth_of_node(int id, int k) {
	Data ret;
	std::lock_guard lock(this->file_lock);
	this->file.seekg((id - 1) * sizeof(Node) + sizeof(NodeHeader) + k * sizeof(Data));
	this->file.read(reinterpret_cast<char *>(&ret), sizeof(Data));
	return ret;
}

template <typename IndexType, typename ValueType>
bool FileMap<IndexType, ValueType>::insert(IndexType const &key, ValueType const &value) {
	Data obj{key, value};
	int p = 1;
	while (p) {
		std::unique_lock lock(block_lock[p]);
		NodeHeader header = read_block_header(p);
		if (!header.next || obj < read_kth_of_node(header.next, 0)) {
			return insert(obj, p);
		}
		p = header.next;
	}
	return false;
}

template <typename IndexType, typename ValueType>
bool FileMap<IndexType, ValueType>::insert(Data const &obj, int id) {
	Node node = this->Base::read(id);
	auto check_exist = [&obj](Node &node) {
		auto p = std::lower_bound(node.data, node.data + node.header.size, obj);
		if (p != node.data + node.header.size && *p == obj) return true;
		else return false;
	};
	if (check_exist(node)) return false;
	auto insert_node = [&obj](Node &node) {
		auto p = std::upper_bound(node.data, node.data + node.header.size, obj);
		for (auto cur = node.data + node.header.size; cur > p; --cur)
			*cur = *(cur - 1);
		*p = obj;
		++node.header.size;
	};
	if (node.header.size == Node::MaxBlock) {
		Node new_node;

		new_node.header.next = node.header.next;
		new_node.header.size = Node::MaxBlock / 2;
		node.header.size -= Node::MaxBlock / 2;

		memcpy(new_node.data, node.data + node.header.size, (Node::MaxBlock / 2) * sizeof(Data));
		memset(new_node.data + Node::MaxBlock / 2, 0, (Node::MaxBlock - Node::MaxBlock / 2) * sizeof(Data));
		memset(node.data + node.header.size, 0, (Node::MaxBlock / 2) * sizeof(Data));

		if (obj < new_node.data[0]) insert_node(node);
		else insert_node(new_node);
		node.header.next =  this->Base::insert(new_node);
	}

	else insert_node(node);
	this->Base::write(id, node);
	return true;
}


template <typename IndexType, typename ValueType>
bool FileMap<IndexType, ValueType>::erase(IndexType const &key, ValueType const &value) {
	Data obj{key, value};
	int p = 1;
	while (p) {
		std::unique_lock lock(block_lock[p]);
		NodeHeader header = read_block_header(p);
		if (!header.next || obj < read_kth_of_node(header.next, 0)) {
			return erase(obj, p);
		}
		p = header.next;
	}
	return false;
}

template <typename IndexType, typename ValueType>
bool FileMap<IndexType, ValueType>::erase(Data const &obj, int id) {
	Node node = this->Base::read(id);
	auto p = std::lower_bound(node.data, node.data + node.header.size, obj);
	if (p == node.data + node.header.size || *p != obj) return false;
	--node.header.size;
	while (p < node.data + node.header.size) {
		*p = *(p + 1);
		++p;
	}
	if (node.header.size <= Node::MinBlock && node.header.next) {
		std::unique_lock lock_next(block_lock[node.header.next]);
		NodeHeader next_header = read_block_header(node.header.next);
		if (node.header.size + next_header.size <= Node::MaxBlock) {
			{
				std::lock_guard lock(this->file_lock);
				this->file.seekg((node.header.next - 1) * sizeof(Node) + sizeof(NodeHeader));
				this->file.read(reinterpret_cast<char *>(node.data + node.header.size), next_header.size * sizeof(Data));
			}
			this->Base::erase(node.header.next);
			node.header.next = next_header.next;
			node.header.size += next_header.size;
		}
	}
	this->write(id, node);
	return true;
}

template <typename IndexType, typename ValueType>
std::vector<ValueType> FileMap<IndexType, ValueType>::find(IndexType const &key) {
	std::vector<int> ret;
	int p = 1;
	NodeHeader header;
	Data Key{key, {}};
	for (; p; p = header.next) {
		std::shared_lock lock(block_lock[p]);
		header = read_block_header(p);
		if (header.size == 0) continue;
		Data front = read_kth_of_node(p, 0);
		if (key < front.index) break;
		Data back = read_kth_of_node(p, header.size - 1);
		if (key > back.index) continue;
		Node node = this->Base::read(p);
		auto p1 = std::lower_bound(node.data, node.data + header.size, Key, [](Data const &x, Data const &y) { return x.index < y.index; });
		auto p2 = std::upper_bound(node.data, node.data + header.size, Key, [](Data const &x, Data const &y) { return x.index < y.index; });
		while (p1 != p2)
			ret.emplace_back((p1++)->value);
	}
	return ret;
}

} // namespace kupi

using kupi::FileMap;

#endif // BOOKSTORE_FILEMAP_H
