#pragma once

#include "thread_parallelfor.h"
#include <Windows.h>

namespace afthread
{

inline int getcores( void )
{
	SYSTEM_INFO si;
	GetSystemInfo( &si );
	return ( si.dwNumberOfProcessors > 0 ) ? int( si.dwNumberOfProcessors ) : 1;
}

class taskinfo
{
public:
	taskinfo(){}
	~taskinfo(){}
};

// T - task type
template <typename T> class task
{
public:
	task(const T& other,const afthread::taskinfo *pInfo):m_Task( other ), m_pTaskInfo( pInfo ){}
	task(const task& other):m_Task( other.m_Task ),m_pTaskInfo( other.m_pTaskInfo ){}
	
	void operator()(const parallel_for_range& r)
	{
		m_Task( r.getfrom(), r.getinclusiveto(), m_pTaskInfo );
	}
private:
	const T& m_Task;
	const afthread::taskinfo *m_pTaskInfo;
};

class taskscheduler
{

public:

	// Constructor
	taskscheduler()
	{
		m_nCores = afthread::getcores();
		m_dSqrtCores = sqrt( double( m_nCores ) );
	}
	virtual ~taskscheduler(){}

	// Accessors
	int getcores( void ) const { return m_nCores; }
	double getsqrtcores( void ) const { return m_dSqrtCores; }

	// Modifiers
	// T - task type
	template <typename T> void serial_for( const unsigned int nFrom, const unsigned int nCount, const T& t, const taskinfo *pInfo = nullptr ) const
	{
		// Check the implementation
		if( nCount == 0 )
			return;
		t( nFrom, nFrom + nCount - 1, pInfo ); 
	}
	template <typename T> void parallel_for( const unsigned int nFrom, const unsigned int nCount, const unsigned int nCores, const T& t, const taskinfo *pInfo = nullptr, const parallel_for_range::graintype gt = parallel_for_range::gt_ceil ) const
	{
		// Check the implementation
		if( nCount == 0 || nCores == 0 )
			return;

		task<T> tsk( t, pInfo );
		m_pool.parallel_for<task<T>>(tsk,parallel_for_range(nFrom,nCount,nCores,gt));
	}
protected:

	// Data
	mutable parallel_for_pool m_pool;
	int m_nCores;
	double m_dSqrtCores;
};

}
