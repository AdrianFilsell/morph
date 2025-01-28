#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <random>
#include <numeric>
#include <algorithm>

#define MOR_FLTTYPE double

namespace af
{
	template <typename T> __forceinline bool fpvalid( const T d, const bool bDenormalNormal = false/*non-zero number with magnitude smaller than the smallest 'normal' number*/ )
	{
		const int n = ::fpclassify(d);
		if( FP_ZERO == n )
			return true;
		if( FP_NORMAL == n )
			return true;
		if( FP_SUBNORMAL == n )
			return bDenormalNormal;
		// could be NAN or INFINITE
		return false;
	}

	// T - in data type
	// R - floored out data type
	template <typename T, typename R> constexpr __forceinline int posceil( const T d ){const R r = R( d );return T( r ) == d ? r : 1 + r;}
	template <typename T, typename R> constexpr __forceinline int posround( const T d ){return R( 0.5 + d );}
	template <typename T, typename R> constexpr __forceinline R posfloor( const T d )
	{
		// Floored integer
		return R( d );
	}
	// T - in data type
	// R - floored out data type
	template <typename T, typename R> constexpr __forceinline R negfloor( const T d )
	{
		// Floored integer
		const R r = R( d );
		return T( r ) == d ? r : -1 + r;
	}
	// T - in data type
	// R - floored out data type
	template <typename T, typename R> constexpr __forceinline R floor( const T d )
	{
		// Floor integer
		return ( d < 0 ) ? negfloor<T,R>( d ) : posfloor<T,R>( d );
	}

	// T - test data type
	template <typename T> constexpr __forceinline T minval( const T a, const T b )
	{
		return a < b ? a : b;
	}
	// T - test data type
	template <typename T> constexpr __forceinline T maxval( const T a, const T b )
	{
		return a > b ? a : b;
	}

	// T - pi data type
	template <typename T> constexpr __forceinline T getpi( void )
	{
		return T( 3.1415926535897932384626433832795 );
	}
	// T - 50% pi data type
	template <typename T> constexpr __forceinline T getpi_1_2( void )
	{
		return 0.5 * getpi<T>();
		//T( 1.5707963267948966192313216916398 );
	}
	// T - 2pi data type
	template <typename T> constexpr __forceinline T getpi_2( void )
	{
		return 2 * getpi<T>();
		//T( 6.283185307179586476925286766559 );
	}
	// T - radian data type
	template <typename T> constexpr __forceinline T getradian0to2pi( const T rad )
	{
		const int n = int( rad / getpi_2<T>() );
		const T tmp = ( rad - ( n * getpi_2<T>() ) );
		ASSERT( !( tmp < -getpi_2<T>() ) );
		ASSERT( !( tmp > getpi_2<T>() ) );
		if( rad < 0 )
			return tmp + getpi_2<T>();
		return tmp;
	}

	// Interpolation
	// T1 - in from and to interpolated data type
	// T2 - float interpolation value type and return type
	template<typename T1,typename T2> __forceinline T2 getlerp( const T1 from, const T1 to, const T2 dI ) { return ( from * ( 1 - dI ) + ( to * dI ) ); }
}
