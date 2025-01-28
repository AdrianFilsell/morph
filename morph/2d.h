#pragma once

#include <vector>
#include "core.h"
#include "serialise.h"
#include "../Eigen/Dense"

namespace af2d
{

template <typename T=MOR_FLTTYPE> class point
{
public:
	point(){}
	point(const T x,const T y):m_X(x),m_Y(y){}
	~point(){}
	T getx(void)const{return m_X;}
	T gety(void)const{return m_Y;}
	T getlengthsq(void)const{return m_X*m_X+m_Y*m_Y;}
	T getlength(void)const{const T d=getlengthsq();return d?(T)sqrt(d):0;}
	T& getx(void){return m_X;}
	T& gety(void){return m_Y;}
	void offset(const T dX,const T dY){m_X+=dX;m_Y+=dY;}
    bool read(const serialise *pS)
    {
        // base class
		
		// version
		int nVersion=0;
		if(!pS->read<>(nVersion) || nVersion<1)
			return false;

		// members
		if(!pS->read<>(m_X))
			return false;
		if(!pS->read<>(m_Y))
			return false;
        return true;
    }
    bool write(const serialise *pS) const
    {
		// base class

		// version
		const int nVersion = 1;
		if(!pS->write<>(nVersion))
			return false;

		// members
		if(!pS->write<>(m_X))
			return false;
		if(!pS->write<>(m_Y))
			return false;
        return true;
    }
	point& operator =(const point& o){m_X=o.m_X;m_Y=o.m_Y;return *this;}
	point operator -(const point& o)const {return {m_X-o.m_X,m_Y-o.m_Y};}
	bool operator ==(const point& o)const{return m_X==o.m_X && m_Y==o.m_Y;}
protected:
	T m_X;
	T m_Y;
};

template <typename T> class pointvec
{
public:
	pointvec(){}
	pointvec(const std::vector<point<T>>& v):m_vVertices(v){}
	virtual ~pointvec(){}

	const std::vector<point<T>>& get(void)const{return m_vVertices;}
	bool isempty(void)const{return m_vVertices.size()==0;}
	bool isconvex(void)const
	{
		if(m_vVertices.size()<3)
			return true;
		auto nA=m_vVertices.size()-2;
		auto nB=m_vVertices.size()-1;
		auto nC=0;
		auto dSignCheck = crossproduct(m_vVertices[nA],m_vVertices[nB],m_vVertices[nC]);
		bool b = dSignCheck>0.0;
		bool bHaveSign=dSignCheck!=0.0;

		nA=m_vVertices.size()-1,nB=0,nC=1;
		dSignCheck = crossproduct(m_vVertices[nA],m_vVertices[nB],m_vVertices[nC]);
		if(bHaveSign)
		{
			if((dSignCheck>0.0) != b)
				return false;
		}
		else
			bHaveSign=dSignCheck!=0.0;

		auto i=m_vVertices.cbegin(),end=m_vVertices.cend();
		--end;
		--end;
		for(;i!=end;++i)
		{
			auto dSignCheck = crossproduct(*i,*(i+1),*(i+2));
			if(bHaveSign)
			{
				if((dSignCheck>0.0) != b)
					return false;
			}
			else
				bHaveSign=dSignCheck!=0.0;
		}
		return true;
	}
	bool write(const serialise *pS) const
	{
		// write
		if(!pS->writebytes<>(m_vVertices))
			return false;
		return true;
	}
	bool read(const serialise *pS)
	{
		// read
		if(!pS->readbytes<>(m_vVertices))
			return false;
		return true;
	}

	std::vector<point<T>>& get(void){return m_vVertices;}
	void eraseconincident(void)
	{
		for(int n=static_cast<int>(m_vVertices.size()-1);n>0;--n)
			if(m_vVertices[n]==m_vVertices[n-1])
				m_vVertices.erase(m_vVertices.cbegin()+n);
	}
	pointvec& operator =(const pointvec& o){m_vVertices=o.m_vVertices;return *this;}
protected:
	std::vector<point<T>> m_vVertices;
	void set(const int n,const point<T>& p){m_vVertices[n]=p;}								// assume derived class gives correct index
	void offset(const int n,const point<T>& p){m_vVertices[n].offset(p.getx(),p.gety());}	// assume derived class gives correct index
	T crossproduct(const point<T>& a,const point<T>& b,const point<T>& c)const { return ((b.getx() - a.getx()) * (c.gety() - b.gety()) - (b.gety() - a.gety()) * (c.getx() - b.getx())); }
};

template <typename T=long> class rect : public pointvec<T>
{
public:
	enum vertex {tl=0,br=1};
	rect(){}
	rect(const point<T>& tl,const point<T>& br):pointvec<T>({tl,br}){}
	rect(const point<T> *p,const size_t n)
	{
		if(n>0)
		{
			T minx=p[0].getx();
			T maxx=minx;
			T miny=p[0].gety();
			T maxy=miny;
			for(int nP=1;nP<n;++nP)
			{
				if(p[nP].getx()<minx)
					minx=p[nP].getx();
				else
				if(p[nP].getx()>maxx)
					maxx=p[nP].getx();
				if(p[nP].gety()<miny)
					miny=p[nP].gety();
				else
				if(p[nP].gety()>maxy)
					maxy=p[nP].gety();
			}
			(*this)={{minx,miny},{maxx+1,maxy+1}}; // non inclusive br point
		}
	}
	rect(const std::vector<point<T>>& v):rect(v.size()?&(v[0]):nullptr,v.size()){}
	virtual ~rect(){}
	const point<T>& get(const vertex v)const{return pointvec<T>::get()[v];}
	const point<T> getcentre(void)const{return {get(tl).getx()+(getwidth()/2),get(tl).gety()+(getheight()/2)};}
	T getwidth(void)const{return get(br).getx()-get(tl).getx();}
	T getheight(void)const{return get(br).gety()-get(tl).gety();}
	bool ishorznormalised(void)const{return get(tl).getx()<=get(br).getx();}
	bool isvertnormalised(void)const{return get(tl).gety()<=get(br).gety();}
	bool isinside(const point<T>& other)const{return get(tl).gety() <= other.gety() && get(tl).getx() <= other.getx() && get(br).gety() > other.gety() && get(br).getx() > other.getx();}
	rect getnormalised(void)const
	{
		if( isempty() )
			return (*this);

		return {{af::minval<T>(get(tl).getx(), get(br).getx() ),af::minval<T>( get(tl).gety(), get(br).gety() )},
				{af::maxval<T>(get(tl).getx(), get(br).getx() ), af::maxval<T>( get(tl).gety(), get(br).gety() )}};
	}
	rect getunion(const rect& other)const
	{
		if( isempty() )
			return other;
		if( other.isempty() )
		return (*this);
		
		const bool bHorzNormalisedA = ishorznormalised();
		const bool bVertNormalisedA = isvertnormalised();
		const bool bHorzNormalisedB = other.ishorznormalised();
		const bool bVertNormalisedB = other.isvertnormalised();

		if( bHorzNormalisedA && bHorzNormalisedB && bVertNormalisedA && bVertNormalisedB )
			return {{af::minval( get(tl).getx(), other.get(tl).getx() ), af::minval( get(tl).gety(), other.get(tl).gety() )},{af::maxval( get(br).getx(), other.get(br).getx() ), af::maxval( get(br).gety(), other.get(br).gety() )}};

		rect res = ( bHorzNormalisedA && bVertNormalisedA ) ? getunion(bHorzNormalisedB && bVertNormalisedB ? other : other.getnormalised()) :
															  getnormalised().getunion(bHorzNormalisedB && bVertNormalisedB ? other : other.getnormalised());
		if( !bHorzNormalisedA )
		{
			auto n = res.get(tl).getx();
			res.set(tl,{res.get(br).getx(),res.get(tl).gety()});
			res.set(br,{n,res.get(br).gety()});
		}
		if( !bVertNormalisedA )
		{
			auto n = res.get(tl).gety();
			res.set(tl,{res.get(tl).getx(),res.get(br).gety()});
			res.set(br,{res.get(br).getx(),n});
		}
		return res;
	}
	rect getintersect(const rect& other)const
	{
		rect r;

		if( isempty() || other.isempty() )
			return r;
	
		const bool bHorzNormalisedA = ishorznormalised();
		const bool bVertNormalisedA = isvertnormalised();
		const bool bHorzNormalisedB = other.ishorznormalised();
		const bool bVertNormalisedB = other.isvertnormalised();

		if( bHorzNormalisedA && bHorzNormalisedB && bVertNormalisedA && bVertNormalisedB )
		{
			// if inclusive then checking for an intersect uses the 'interior' therfore this will NOT detect a 'shared' edge
			bool bIntersect = true;
			if( other.get(tl).gety() >= get(br).gety() )
				bIntersect = false;
			else
			if( get(tl).gety() >= other.get(br).gety() )
				bIntersect = false;
			else
			if( other.get(tl).getx() >= get(br).getx() )
				bIntersect = false;
			else
			if( get(tl).getx() >= other.get(br).getx() )
				bIntersect = false;
			if( bIntersect )
				return {{af::maxval<T>( get(tl).getx(), other.get(tl).getx()),af::maxval<T>( get(tl).gety(), other.get(tl).gety())},
						{af::minval<T>( get(br).getx(), other.get(br).getx()),af::minval<T>( get(br).gety(), other.get(br).gety())}};
			return r;
		}

		if( bHorzNormalisedA && bVertNormalisedA )
			r = getintersect( bHorzNormalisedB && bVertNormalisedB ? other : other.getnormalised() );
		else
			r = getnormalised().getintersect( bHorzNormalisedB && bVertNormalisedB ? other : other.getnormalised() );
		if( !bHorzNormalisedA )
			r={{r.get(br).getx(),r.get(tl).gety()},{r.get(tl).getx(),r.get(br).gety()}};
		if( !bVertNormalisedA )
			r={{r.get(tl).getx(),r.get(br).gety()},{r.get(br).getx(),r.get(tl).gety()}};
		return r;
	}

	point<T>& get(const vertex v){return pointvec<T>::get()[v];}
	void offset(const point<T>& p){offset(tl,p);offset(br,p);}
	void offset(const vertex v,const point<T>& p){pointvec<T>::offset(v,p);}
	rect& operator =(const rect& o){pointvec<T>::operator=(o);return *this;}

	static void getrectscale(const double dSrcTLX,const double dSrcTLY,const double dSrcBRX,const double dSrcBRY,
							 const double dDstTLX,const double dDstTLY,const double dDstBRX,const double dDstBRY,
							 const bool bLetterBox,double& dS)
	{
		const double dDstWidth = dDstBRX-dDstTLX;
		const double dDstHeight = dDstBRY-dDstTLY;
		const double dSrcWidth = dSrcBRX-dSrcTLX;
		const double dSrcHeight = dSrcBRY-dSrcTLY;
		const double x = dDstWidth / dSrcWidth, y = dDstHeight / dSrcHeight;	
		if( bLetterBox )
		{
			// make src as large as possible while keeping both sides of src within dst
			const double min = x < y ? x : y;
			dS=min;
		}
		else
		{
			// make src as large as possible while keeping smallest side of src within dst
			const double max = x < y ? y : x;
			dS=max;
		}
	}
};

template <typename T=MOR_FLTTYPE> class quad : public pointvec<T>
{
public:
	enum vertex {tl=0,tr=1,br=2,bl=3};
	quad(){}
	quad(const point<T>& tl,const point<T>& tr,const point<T>& br,const point<T>& bl):pointvec<T>({tl,tr,br,bl}){}
	virtual ~quad(){}
	const point<T>& get(const vertex v)const{return pointvec<T>::get()[v];}
	void set(const vertex v,const point<T>& p){pointvec<T>::set(v,p);}
	quad& operator =(const quad& o){pointvec<T>::operator=(o);return *this;}
};

}
