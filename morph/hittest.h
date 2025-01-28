#pragma once

#include "primitive.h"

class hittest
{
public:
	enum type{t_null,t_centre_ellipse,t_aggregate_bbox/*point+inflation+border*/};
	
	hittest() {m_Type=t_null;m_rtpt = {0,0};m_pPrimitive=nullptr;}
	hittest( const hittest &other ):hittest(){(*this)=other;}
	~hittest(){}
	
	bool isempty(void)const{return m_Type==t_null;}
	type gettype(void)const{return m_Type;}
	const af2d::point<int>& getrtpt( void ) const { return m_rtpt; }
	afmorph::primitive *getprimitive(void)const{return m_pPrimitive;}
	
	void settype(const type t){m_Type=t;}
	void setrtpt( const af2d::point<int>& p ) { m_rtpt = p; }
	void setprimitive(afmorph::primitive *p){m_pPrimitive=p;}
	
	hittest& operator =( const hittest& o)
	{
		m_Type=o.m_Type;
		m_rtpt=o.m_rtpt;
		m_pPrimitive=o.m_pPrimitive;
		return *this;
	}
protected:
	type m_Type;
	af2d::point<int> m_rtpt;
	afmorph::primitive *m_pPrimitive;
};
