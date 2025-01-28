#pragma once

#include "core.h"
#include <thread>
#include <mutex>

namespace afthread
{

class parallel_for_pool;

class parallel_for_range
{
public:
	enum graintype {gt_floor,gt_round,gt_ceil};
	parallel_for_range():m_bEmpty(true){}
	parallel_for_range(unsigned int nFrom,const unsigned int nCount,const unsigned int nCores,const graintype gt=gt_ceil){getgrain(nFrom,nCount,nCores,gt);}
	parallel_for_range(const parallel_for_range& other):m_bEmpty(other.m_bEmpty),m_nFrom(other.m_nFrom),m_nInclusiveTo(other.m_nInclusiveTo),m_nWholeSubRangeSize(other.m_nWholeSubRangeSize),m_nSubRangeCount(other.m_nSubRangeCount){}
	~parallel_for_range(){}
	bool isempty( void ) const { return m_bEmpty; }
	parallel_for_range getsubrange( const unsigned int n ) const{const unsigned int nFrom = m_nFrom+(n*m_nWholeSubRangeSize);const unsigned int nSize = n==(m_nSubRangeCount-1) ? (m_nInclusiveTo-nFrom+1) : m_nWholeSubRangeSize;return parallel_for_range(nFrom,nSize,1,gt_floor);}
	unsigned int getsubrangecount( void ) const { return m_nSubRangeCount; }
	unsigned int getfrom( void ) const { return m_nFrom; }
	unsigned int getinclusiveto( void ) const { return m_nInclusiveTo; }
	parallel_for_range& operator =(const parallel_for_range& other ){m_nFrom=other.m_nFrom;m_nInclusiveTo=other.m_nInclusiveTo;m_nWholeSubRangeSize=other.m_nWholeSubRangeSize;m_nSubRangeCount=other.m_nSubRangeCount;return *this;}
	static bool getgrain( const unsigned int nCount,const unsigned int nCores, const graintype gt, unsigned int& nGrain );
protected:
	bool m_bEmpty;
	unsigned int m_nFrom;
	unsigned int m_nInclusiveTo;
	unsigned int m_nWholeSubRangeSize;
	unsigned int m_nSubRangeCount;
	void getgrain( const unsigned int nFrom,const unsigned int nCount,const unsigned int nCores, const graintype gt );
};

class parallel_for_task
{
public:
	parallel_for_task(){}
	virtual ~parallel_for_task(){}
	virtual void execute( const parallel_for_range& subrange ) = 0;
protected:
};

template <typename T> class parallel_for_taskT : public parallel_for_task
{
public:
	parallel_for_taskT(T& b):m_Body(b){}
	virtual ~parallel_for_taskT(){}
	virtual void execute( const parallel_for_range& subrange ) override { (m_Body)(subrange); }
protected:
	T& m_Body;
};

class parallel_for_thread
{
public:
	enum statustype { st_running=0x1,st_stop=0x2 };
	parallel_for_thread(parallel_for_pool *pPool,const unsigned int nCore):m_pPool(pPool),m_nCore(nCore),m_pInc(nullptr),m_nStatus(0){}
	~parallel_for_thread(){stop();}
	unsigned int getcore( void ) const { return m_nCore; }
	bool start( void );
	bool stop( void );
	void setbusy(std::shared_ptr<parallel_for_task> spT,const parallel_for_range& subrange,unsigned int *pInc,std::unique_lock<std::mutex>& lk);
protected:
	unsigned int m_nCore;
	std::shared_ptr<std::thread> m_spThread;
	int m_nStatus;
	std::condition_variable m_cv;
	std::shared_ptr<parallel_for_task> m_spTask;
	parallel_for_range m_SubRange;
	unsigned int *m_pInc;
	parallel_for_pool *m_pPool;
	virtual void work(void);
	void setidle(std::unique_lock<std::mutex>& lk);
};

class parallel_for_pool
{
friend class parallel_for_thread;
public:
	parallel_for_pool()
	{
		m_nIdle = std::thread::hardware_concurrency() - 1; // assume high probability calling thread will need to do some work
		m_pIdle = new parallel_for_thread *[m_nIdle];
		for( unsigned int n = 0 ; n < m_nIdle ; ++n )
		{
			std::shared_ptr<parallel_for_thread> sp(new parallel_for_thread(this,n));
			sp->start();
			m_vPool.push_back(sp);
			m_pIdle[n]=sp.get();
		}
	}
	~parallel_for_pool(){delete[] m_pIdle;}
	template<typename T> void parallel_for(T& b,const parallel_for_range& r)
	{
		if( r.isempty() )
			return;

		std::shared_ptr<parallel_for_task> spT( new parallel_for_taskT<T>(b) );

		std::unique_lock<std::mutex> lk(m_mutex);

		unsigned int nSubRange = 0, nProcessed = 0;
		const unsigned int nSubRangeTotal = r.getsubrangecount();
		while( true )
		{
			for( ; m_nIdle && nSubRange < ( nSubRangeTotal - 1 ) ; ++nSubRange ) // while loop should execute at least one sub range in calling thread
				m_pIdle[--m_nIdle]->setbusy(spT,r.getsubrange(nSubRange),&nProcessed,lk);

			if( nSubRange == nSubRangeTotal )
				break; // has all work has been allocated
			
			lk.unlock();
			spT->execute(r.getsubrange(nSubRange)); // support recursive calls and avoid deadlock by executing in this thread
			++nSubRange;

			lk.lock();
			++nProcessed;
		}

		m_cv.wait(lk,[&nProcessed,nSubRangeTotal](void)->bool{return ( nProcessed == nSubRangeTotal);}); // call locked
	}
protected:
	std::mutex m_mutex;
	std::condition_variable m_cv;
	std::vector<std::shared_ptr<parallel_for_thread>> m_vPool;		// static length
	parallel_for_thread **m_pIdle;									// static length
	unsigned int m_nIdle;
	std::mutex& getmutex( void ) { return m_mutex; }
	void setidle(parallel_for_thread *p,std::unique_lock<std::mutex>& lk);
private:
};

}
