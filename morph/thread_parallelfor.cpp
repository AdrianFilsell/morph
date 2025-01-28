
#include "thread_parallelfor.h"
#include "windows.h"
#include <functional>

namespace afthread
{

bool parallel_for_range::getgrain( const unsigned int nCount,const unsigned int nCores, const parallel_for_range::graintype gt, unsigned int& nGrain )
{
	bool bEmpty = nCores == 0 || nCount == 0;
	if(bEmpty) return false;
	
	if( nCores == 1 )
	{
		nGrain = nCount;
		return true;
	}
	
	switch(gt)
	{
		case gt_floor:nGrain = std::max<unsigned int>(1,af::posfloor<double,unsigned int>(nCount/static_cast<double>(nCores)));break;
		case gt_round:nGrain = std::max<unsigned int>(1,af::posround<double,unsigned int>(nCount/static_cast<double>(nCores)));break;
		case gt_ceil:nGrain = std::max<unsigned int>(1,af::posceil<double,unsigned int>(nCount/static_cast<double>(nCores)));break;
		default:bEmpty=true;break;
	}

	return !bEmpty;
}

void parallel_for_range::getgrain( const unsigned int nFrom,const unsigned int nCount,const unsigned int nCores, const graintype gt )
{
	m_bEmpty = nCores == 0 || nCount == 0;
	if(m_bEmpty) return;

	m_nFrom = nFrom;
	m_nInclusiveTo = nFrom + nCount - 1;

	if( nCores == 1 )
	{
		m_nWholeSubRangeSize = nCount;
		m_nSubRangeCount = 1;
		return;
	}
	
	switch(gt)
	{
		case gt_floor:m_nWholeSubRangeSize = std::max<unsigned int>(1,af::posfloor<double,unsigned int>(nCount/static_cast<double>(nCores)));break;
		case gt_round:m_nWholeSubRangeSize = std::max<unsigned int>(1,af::posround<double,unsigned int>(nCount/static_cast<double>(nCores)));break;
		case gt_ceil:m_nWholeSubRangeSize = std::max<unsigned int>(1,af::posceil<double,unsigned int>(nCount/static_cast<double>(nCores)));break;
		default:m_bEmpty=true;return;
	}

	m_nSubRangeCount = ( nCount / m_nWholeSubRangeSize );
	m_nSubRangeCount = ( m_nSubRangeCount * m_nWholeSubRangeSize ) < nCount ? ( m_nSubRangeCount + 1 ) : m_nSubRangeCount;
	m_nSubRangeCount = std::min<unsigned int>(m_nSubRangeCount,nCores);
		
	if( m_nSubRangeCount > 1 )
	{
		const parallel_for_range lastsubrange = getsubrange( m_nSubRangeCount - 1 );
	}
}

bool parallel_for_thread::start( void )
{
	std::unique_lock<std::mutex> lk(m_pPool->getmutex());

	if( m_nStatus&st_running )
		return true;

	_ASSERT(!m_spThread);
	auto i = std::bind(&parallel_for_thread::work,this);
	std::function<void(void)> fn = i;
	m_spThread = std::shared_ptr<std::thread>( new std::thread(fn) );		
	m_cv.wait(lk,[this](void)->bool{return st_running&this->m_nStatus;}); // call locked

	// init
	#ifdef _WIN32
	{
		auto h = m_spThread->native_handle();
		const unsigned int nMask = 1 << m_nCore;
		const bool b = ::SetThreadAffinityMask(h,nMask);
		_ASSERT(b);
	}
	#else
		_ASSERT(false);
	#endif

	return m_nStatus&st_running;
}

bool parallel_for_thread::stop( void )
{
	std::unique_lock<std::mutex> lk(m_pPool->getmutex());

	if( m_nStatus&st_running )
	{
		m_nStatus |= st_stop;
		m_cv.notify_one();
		m_cv.wait(lk,[this](void)->bool{return !(st_running&this->m_nStatus);}); // call locked
	}

	if( m_spThread )
		m_spThread->join();
	m_spThread = nullptr;
	return true;
}

void parallel_for_thread::setbusy(std::shared_ptr<parallel_for_task> spT,const parallel_for_range& subrange,unsigned int *pInc,std::unique_lock<std::mutex>& lk)
{
	_ASSERT(!m_spTask);
	m_SubRange = subrange;
	m_spTask = spT;
	m_pInc = pInc;
	m_cv.notify_one();
}

void parallel_for_thread::setidle(std::unique_lock<std::mutex>& lk)
{
	_ASSERT(m_spTask);
	m_spTask = nullptr;
	++(*m_pInc);
	m_pInc = nullptr;
}

void parallel_for_thread::work(void)
{
	// now running
	{
		std::unique_lock<std::mutex> lk(m_pPool->getmutex());
		m_nStatus |= st_running;
		m_cv.notify_one();
	}

	// loop
	bool bSetIdle = false;
	while(true)
	{
		std::unique_lock<std::mutex> lk(m_pPool->getmutex());
		
		if( bSetIdle )
		{
			bSetIdle = false;
			setidle(lk);
			m_pPool->setidle(this,lk);
		}	
		
		const int nStatus=m_nStatus;
		m_cv.wait(lk,[this,nStatus](void)->bool{return (nStatus!=this->m_nStatus) || !!m_spTask;}); // call locked

		if(m_nStatus&st_stop)
		{
			m_nStatus &= ~(st_stop|st_running);
			m_cv.notify_one();
			return;
		}

		lk.unlock();

		m_spTask->execute(m_SubRange);
		bSetIdle = true;
	}
}

void parallel_for_pool::setidle(parallel_for_thread *p,std::unique_lock<std::mutex>& lk)
{
	m_pIdle[m_nIdle++]=p;
	m_cv.notify_all();
}

}
