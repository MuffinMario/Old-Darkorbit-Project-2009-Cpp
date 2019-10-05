#include "pch.h"
#include "WorkerThreadPool.h"

CWorkerThreadPool::thread_identifier CWorkerThreadPool::getSmallestID() {
	for (thread_identifier i = 0; i < m_NUMTHREADS; i++)
	{
		if (m_threadsInGroup.find(i) == m_threadsInGroup.end())
			return i;
	}
	return nothread();
}

void CWorkerThreadPool::onThreadEnd(thread_identifier id) {

	boost::thread* thread = m_threadsInGroup[id]; // reference would not work, since we remove the container object before deleting the reference to the thread object
	m_threadsInGroup.remove(id);
	delete thread; //From boost documentation we know that the thread and the thread object represent two different things, at this point this thread will have no access from any other thread and thus does not need to be called and can be deleted. the execution of the thread ends after the mutex is unlocked
}

std::function<void()> CWorkerThreadPool::addOnThreadEnd(std::function<void()> func, thread_identifier id)
{
	auto f = boost::bind(&CWorkerThreadPool::onThreadEnd, this, id);
	return [func, f]() {
		boost::this_thread::at_thread_exit(f);
		func();
	};
}

CWorkerThreadPool::CWorkerThreadPool(size_t numThreads) : m_NUMTHREADS(numThreads) {

}

CWorkerThreadPool::~CWorkerThreadPool()
{
	m_threadsInGroup.clear();
}

void CWorkerThreadPool::stop(thread_identifier threadid)
{
	m_threadsInGroup.doWithIfExists(threadid, [](boost::thread* t) {t->interrupt(); });
}

CWorkerThreadPool::thread_identifier CWorkerThreadPool::add(std::function<void()> func)
{
	if (m_threadsInGroup.size() < m_NUMTHREADS)
	{
		thread_identifier threadID = getSmallestID();
		if (threadID != nothread()) // it is actually impossible that this case doesnt kick in, threadsingroup.size already prepares that there is one less element inside than possible. nothing but add() increments this
		{
			boost::thread* thread = nullptr;

			//hook identifier to function to tell at_end to kill function
			auto lamba = addOnThreadEnd(func, threadID);

			thread = new boost::thread(lamba); // what if lambda is finished, mutex from here to after initiation?
			m_threadsInGroup[threadID] = thread; //the hash table returns a reference thread safe which provides us with a "not possible to delete this object" state since no thread calls this any reference to this func
		}
		return threadID;
	}
	else {
		return nothread();
	}
}

CWorkerThreadPool::thread_identifier CWorkerThreadPool::nothread() {
	return m_NUMTHREADS;
}

bool CWorkerThreadPool::full() { return m_threadsInGroup.size() == m_NUMTHREADS; }

size_t CWorkerThreadPool::size() { return m_threadsInGroup.size(); }

void CWorkerThreadPool::stopAllThreads()
{
	m_threadsInGroup.forEach(
		[](std::pair<CWorkerThreadPool::thread_identifier, boost::thread*> pair)->void
		{
			pair.second->interrupt();
		}
	);
	//interrupt calls at_exit and they all remove and everyone is happy and the world is great
}
