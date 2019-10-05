#pragma once
#include <iostream>

#include "ThreadSafeHashTable.h"
/*
	A thread pool that automatically frees memory on thread end
*/
class CWorkerThreadPool {
public:
	typedef size_t thread_identifier;
private:
	//Thread safe hash table for  identification
	CThreadSafeHashTable<thread_identifier, boost::thread*> m_threadsInGroup;
	//Number of threads possible, set by constructor
	const thread_identifier m_NUMTHREADS;


	/*
	Returns the smallest identifier currently free
	*/
	thread_identifier getSmallestID();

	/*
	Gets hooked and gets called at the end of every thread
	*/
	void onThreadEnd(thread_identifier id);
	/*
	Every worker added gets hooked by this function and prepared onThreadEnd for this function
	*/
	std::function<void()> addOnThreadEnd(std::function<void()> func, thread_identifier id);

public:
	/*
	Constructor of Thread Group - takes in the maximum number of threads
	*/
	CWorkerThreadPool(size_t numThreads);

	/*
		In case of early destruction, we remove old threads prematurely
	*/
	~CWorkerThreadPool();
	/*
	Stops the thread with thread ID threadid
	*/
	void stop(thread_identifier threadid);
	/*
	Adds a worker to the threadpool with the given function
	*/
	thread_identifier add(std::function<void()> func);
	/*
	identifier if to be added thread is not pushed (due to size)
	*/
	thread_identifier nothread();
	/*
	Returns true of the threadgroup reached its maximum size
	*/
	bool full();
	/*
	Returns the size of the Thread Group
	*/
	size_t size();
	/*
	Stops all threads by interrupting them.
	*/
	void stopAllThreads();

};