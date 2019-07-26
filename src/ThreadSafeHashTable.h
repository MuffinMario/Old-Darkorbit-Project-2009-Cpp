#pragma once
#include <unordered_map>
#include <boost/thread.hpp>

template <typename K, typename T>
class CThreadSafeHashTable {
	std::unordered_map<K, T> m_map;
	boost::shared_mutex m_sharedMutex;
#define LOCK boost::unique_lock<boost::shared_mutex> _lg(m_sharedMutex)
#define LOCK_SHARED boost::shared_lock<boost::shared_mutex> _lg(m_sharedMutex)
public:
	CThreadSafeHashTable() : m_map() {
		m_map.max_load_factor(0.693); //ln(2)
	}
	void insert(T el)
	{
		LOCK;
		m_map.insert(el);
	}
	void remove(K key)
	{
		LOCK;
		m_map.erase(key);
	}
	auto find(K key)
	{
		LOCK_SHARED;
		return m_map.find(key);
	}
	auto begin() { return m_map.begin(); }
	auto end() { LOCK_SHARED; return m_map.end(); }
	auto cbegin() { return m_map.cbegin(); }
	auto cend() { LOCK_SHARED; return m_map.cend(); }
	/*
		Clears Hash table
	*/
	void clear() {
		LOCK;
		m_map.clear();
	}
	/*
		Returns amounts of elements
	*/
	auto size() {
		LOCK_SHARED;
		return m_map.size();
	}
	/*
		Clear all elements from table, before each deletion func gets called with the deleted element
	*/
	void clearForEach(std::function<void(std::pair<K, T>)> func)
	{
		LOCK;
		for (auto it = m_map.begin(); it != m_map.end();)
		{
			func(it->first, it->second);
			it = it->erase();
		}
	}
	/*
	* Do something with the key selected, func is called with parameter map[key]
	* This function locks the functions which alternate the size.
	*
	* key is the key selected
	* func is a function of the template "void func(T&)"
	*/
	void doWith(K key, std::function<void(T&)> func)
	{
		LOCK_SHARED;
		T& el = m_map[key];
		func(el);
	}
	/*
	* Do something with the key selected, func is called with parameter map.find(key) + != end check + *found
	* This function locks the functions which alternate the size.
	*
	* key is the key selected
	* func is a function of the template "void func(T&)"
	*/
	bool doWithIfExists(K key, std::function<void(T&)> func)
	{
		LOCK_SHARED;
		auto el = m_map.find(key);
		if (el != m_map.end())
		{
			func(el->second);
			return true;
		}
		else
			return false;
	}
	// DO NOT ERASE/MODIFY BUCKETS HERE
	void forEach(std::function<void(std::pair<K, T>)> func)
	{
		LOCK_SHARED;
		for (auto it = m_map.begin(); it != m_map.end(); it++)
		{
			func(*it);
		}
	}
	// DO NOT ERASE/MODIFY BUCKETS HERE
	void forEach(std::function<void(K, T)> func)
	{
		LOCK_SHARED;
		for (auto it = m_map.begin(); it != m_map.end(); it++)
		{
			func(it->first, it->second);
		}
	}
	T& operator[](K key) {
		LOCK; // if key doesnt exist, make
		return m_map.operator[](key);
	}
};