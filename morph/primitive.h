#pragma once

#include "2d.h"
#include "Windows.h"

namespace afmorph
{

template <typename T=MOR_FLTTYPE> class xform
{
public:
	xform(const Eigen::Affine2d *pLtoD,const Eigen::Affine2d *pDtoL,const int nLogicalWidth,const int nLogicalHeight):
	m_pLtoD(pLtoD),m_pDtoL(pDtoL),m_nLogicalWidth(nLogicalWidth),m_nLogicalHeight(nLogicalHeight){}

	__forceinline void dtol(const af2d::point<long>& from,af2d::point<long>& to) const {floor(m_pDtoL,from,to);}
	__forceinline void ltod(const af2d::point<T>& from,af2d::point<long>& to) const
	{
		T dToX,dToY;
		xy(m_pLtoD,from.getx(),from.gety(),dToX,dToY);
		to.getx()=af::floor<T,int>(dToX);
		to.gety()=af::floor<T,int>(dToY);
	}
	__forceinline void ltod(const af2d::point<long>& from,af2d::point<long>& to) const {floor(m_pLtoD,from,to);}
	__forceinline void ltod(const af2d::rect<long>& from,af2d::rect<long>& to) const
	{
		af2d::point<T> to_tl={0,0},to_br={0,0};
		to=af2d::rect<long>({0,0},{0,0});

		xy(m_pLtoD,from.get(af2d::rect<long>::tl).getx(),from.get(af2d::rect<long>::tl).gety(),to_tl.getx(),to_tl.gety());
		xy(m_pLtoD,from.get(af2d::rect<long>::br).getx(),from.get(af2d::rect<long>::br).gety(),to_br.getx(),to_br.gety());

		to.get(af2d::rect<long>::tl).getx()=af::floor<T,long>(to_tl.getx());
		to.get(af2d::rect<long>::tl).gety()=af::floor<T,long>(to_tl.gety());

		to.get(af2d::rect<long>::br).getx()=af::floor<T,long>(to_br.getx());
		to.get(af2d::rect<long>::br).gety()=af::floor<T,long>(to_br.gety());
		
		if( to_br.getx() > to.get(af2d::rect<long>::br).getx() )
			to.get(af2d::rect<long>::br).getx()++;
		if( to_br.gety() > to.get(af2d::rect<long>::br).gety() )
			to.get(af2d::rect<long>::br).gety()++;
	}
	__forceinline void ltod(const af2d::rect<T>& from,af2d::rect<long>& to) const
	{
		af2d::point<T> to_tl={0,0},to_br={0,0};
		to=af2d::rect<long>({0,0},{0,0});

		xy(m_pLtoD,from.get(af2d::rect<T>::tl).getx(),from.get(af2d::rect<T>::tl).gety(),to_tl.getx(),to_tl.gety());
		xy(m_pLtoD,from.get(af2d::rect<T>::br).getx(),from.get(af2d::rect<T>::br).gety(),to_br.getx(),to_br.gety());

		to.get(af2d::rect<long>::tl).getx()=af::floor<T,long>(to_tl.getx());
		to.get(af2d::rect<long>::tl).gety()=af::floor<T,long>(to_tl.gety());

		to.get(af2d::rect<long>::br).getx()=af::floor<T,long>(to_br.getx());
		to.get(af2d::rect<long>::br).gety()=af::floor<T,long>(to_br.gety());
		
		if( to_br.getx() > to.get(af2d::rect<long>::br).getx() )
			to.get(af2d::rect<long>::br).getx()++;
		if( to_br.gety() > to.get(af2d::rect<long>::br).gety() )
			to.get(af2d::rect<long>::br).gety()++;
	}
	static __forceinline void floor(const Eigen::Affine2d *pTrns,const af2d::point<long>& from,af2d::point<long>& to)
	{
		T dToX,dToY;
		xy(pTrns,from.getx()+0.5,from.gety()+0.5,dToX,dToY);
		to.getx()=af::floor<T,int>(dToX);
		to.gety()=af::floor<T,int>(dToY);
	}
	
	static __forceinline void sxtx_x(const Eigen::Affine2d *pTrns,const T dFrom,T& dTo)
	{
		// scale and translate
		dTo=(dFrom*(*pTrns)(0,0))+(*pTrns)(0,2);
	}
	static __forceinline void sxtx_y(const Eigen::Affine2d *pTrns,const T dFrom,T& dTo)
	{
		// scale and translate
		dTo=(dFrom*(*pTrns)(1,1))+(*pTrns)(1,2);
	}
	static __forceinline void xy(const Eigen::Affine2d *pTrns,const T dFromX,const T dFromY,T& dToX,T& dToY)
	{
		const Eigen::Vector2d from(dFromX,dFromY);
		auto i=(*pTrns*from);
		dToX=i.x();
		dToY=i.y();
	}
protected:
	const Eigen::Affine2d *m_pLtoD;
	const Eigen::Affine2d *m_pDtoL;
	const int m_nLogicalWidth;
	const int m_nLogicalHeight;
};

class primitive
{
public:
	primitive()
	{
		m_Other={nullptr,GUID_NULL};
		HRESULT hr = ::CoCreateGuid(&m_GUID);
	}
	template <typename T=MOR_FLTTYPE> primitive(const std::vector<af2d::point<long>>& vDev,const xform<T>& trns):primitive()
	{
		// cache points
		m_nLogSamples=25;
		m_spLogPts=std::shared_ptr<af2d::pointvec<long>>(new af2d::pointvec<long>);
		m_spLogPts->get().resize(vDev.size());
		m_spDevPts=std::shared_ptr<af2d::pointvec<long>>(new af2d::pointvec<long>);
		m_spDevPts->get().resize(vDev.size());
		auto i=vDev.cbegin(),end=vDev.cend();
		auto j=m_spLogPts->get().begin();
		auto k=m_spDevPts->get().begin();
		for(;i!=end;++i,++j,++k)
		{
			// logical
			trns.dtol(*i,*j);
			
			// device ( floored )
			trns.ltod(*j,*k);
		}
		m_LogBBox=af2d::rect<long>(m_spLogPts->get());
		m_dLogLength=0;
		if(m_spLogPts->get().size()>1)
		{
			m_spLogLengths=std::shared_ptr<std::vector<MOR_FLTTYPE>>(new std::vector<MOR_FLTTYPE>);
			m_spLogLengths->resize(m_spLogPts->get().size()-1);
			auto i=m_spLogPts->get().cbegin(),end=m_spLogPts->get().cend();
			auto j=m_spLogLengths->begin();
			for(++i;i!=end;++i,++j)
			{
				const af2d::point<long>& prev=*(i-1);
				const af2d::point<MOR_FLTTYPE> pt((*i).getx()-prev.getx(),(*i).gety()-prev.gety());
				(*j)=pt.getlength();
				m_dLogLength+=(*j);
			}
		}
	}
	virtual ~primitive(){}

	bool isempty(void)const{return !getlogpts() || getlogpts()->isempty();}
	const af2d::pointvec<long> *getlogpts(void)const{return m_spLogPts.get();}
	const af2d::pointvec<long> *getdevpts(void)const{return m_spDevPts.get();}
	const af2d::rect<long>& getlogbbox(void)const{return m_LogBBox;}
	int getlogsamples(void)const{return m_nLogSamples;}
	const std::pair<primitive*,GUID>& getother(void)const{return m_Other;}
	bool validother(primitive *pOther)const{return pOther ? (m_spLogPts?m_spLogPts->get().size():0)>1 == (pOther->m_spLogPts?pOther->m_spLogPts->get().size():0)>1 : true;}
	const GUID& getguid(void)const{return m_GUID;}

	bool read(const serialise *pS)
	{
        // base class
		
		// version
		int nVersion=0;
		if(!pS->read<>(nVersion) || nVersion<1)
			return false;

		// members
		if(!pS->read<>(m_spLogPts))
			return false;
		if(!pS->readbytes<>(m_spLogLengths))
			return false;
		if(!pS->read<>(m_spLogTriangulatePts))
			return false;
		if(!pS->read<>(m_spDevPts))
			return false;
		if(!m_LogBBox.read(pS))
			return false;
		if(!pS->read<>(m_dLogLength))
			return false;
		if(!pS->read<>(m_nLogSamples))
			return false;
		if(!pS->readbytes<>(m_Other.second))
			return false;
		if(!pS->readbytes<>(m_GUID))
			return false;
		return true;
	}
	bool write(const serialise *pS)const
	{
		// base class

		// version
		const int nVersion = 1;
		if(!pS->write<>(nVersion))
			return false;

		// members
		if(!pS->write<>(m_spLogPts))
			return false;
		if(!pS->writebytes<>(m_spLogLengths))
			return false;
		if(!pS->write<>(m_spLogTriangulatePts))
			return false;
		if(!pS->write<>(m_spDevPts))
			return false;
		if(!m_LogBBox.write(pS))
			return false;
		if(!pS->write<>(m_dLogLength))
			return false;
		if(!pS->write<>(m_nLogSamples))
			return false;
		if(!pS->writebytes<>(m_Other.second))
			return false;
		if(!pS->writebytes<>(m_GUID))
			return false;
		return true;
	}
	
	void setother(primitive *p){m_Other={p,p?p->m_GUID:GUID_NULL};}
	void setlogtodev(const afmorph::xform<>& trns)
	{
		if(isempty())return;
		auto i=m_spLogPts->get().cbegin(),end=m_spLogPts->get().cend();
		auto j=m_spDevPts->get().begin();
		for(;i!=end;++i,++j)
		{
			// device ( floored )
			trns.ltod(*i,*j);
		}
	}
	void setlogsamples(const int n){m_nLogSamples=n;}
	const af2d::pointvec<MOR_FLTTYPE> *getlogtriangulatepts(const int nSamples,const bool bRadianSort)
	{
		if(nSamples<1 || !m_spLogPts || m_spLogPts->get().size()<1)
		{
			m_spLogTriangulatePts=nullptr;
			return m_spLogTriangulatePts.get();
		}		
		
		const auto& vFrom=m_spLogPts->get();

		m_spLogTriangulatePts=m_spLogTriangulatePts?m_spLogTriangulatePts:std::shared_ptr<af2d::pointvec<MOR_FLTTYPE>>(new af2d::pointvec<MOR_FLTTYPE>);
		if(m_Other.first && m_Other.first->getlogpts() && m_Other.first->getlogpts()->get().size()==1 && m_spLogPts->get().size()==1)
		{
			// simple 1 to 1 point
			if(m_spLogTriangulatePts->get().size()!=1)
			{
				m_spLogTriangulatePts->get().resize(1);
				m_spLogTriangulatePts->get()[0]={vFrom[0].getx()+0.0,vFrom[0].gety()+0.0};
			}
			return m_spLogTriangulatePts.get();
		}

		if(m_spLogTriangulatePts && static_cast<int>(m_spLogTriangulatePts->get().size()) == nSamples)
			return m_spLogTriangulatePts.get();

		m_spLogTriangulatePts->get().resize(nSamples);
		
		auto& vTo=m_spLogTriangulatePts->get();
		const auto& vSegmentLengths=*m_spLogLengths;

		const int nFrom=static_cast<int>(vFrom.size());
		const int nTo=static_cast<int>(vTo.size());

		vTo[0]={vFrom[0].getx()+0.0,vFrom[0].gety()+0.0};

		if(nTo>1)
			vTo[nTo-1]={vFrom[nFrom-1].getx()+0.0,vFrom[nFrom-1].gety()+0.0};
		if(nTo<3)
			return m_spLogTriangulatePts.get();
		
		const int nMidPoints=nTo-2;
		const MOR_FLTTYPE dMidLength=m_dLogLength/(nTo-1.0);
		const int nSegments=nFrom-1;
				
		int nMidPoint=0;
		MOR_FLTTYPE dAggLength=0;
		for(int nSegment=0;nSegment<nSegments && nMidPoint<nMidPoints;++nSegment)
			splitsegment(vFrom,vSegmentLengths,nSegment,dMidLength,nMidPoints,nMidPoint,dAggLength,vTo);
		for(int n=nMidPoint;n<nMidPoints;++n) // err tol handler
			vTo[n]=vTo[nTo-1];
		
		if(bRadianSort && !m_LogBBox.isempty())
		{
			const af2d::point<MOR_FLTTYPE> ptCentre={m_LogBBox.get(af2d::rect<long>::tl).getx() + m_LogBBox.getwidth()/2.0,m_LogBBox.get(af2d::rect<long>::tl).gety() + m_LogBBox.getheight()/2.0};
			std::vector<std::pair<int,MOR_FLTTYPE>> vRadians;
			vRadians.resize(m_spLogTriangulatePts->get().size());
			auto i=m_spLogTriangulatePts->get().cbegin(),end=m_spLogTriangulatePts->get().cend();
			auto j=vRadians.begin();
			for(int n = 0;i!=end;++i,++j,++n)
			{
				const MOR_FLTTYPE dX=(*i).getx()-ptCentre.getx();
				const MOR_FLTTYPE dY=(*i).gety()-ptCentre.gety();
				const MOR_FLTTYPE dRadian=atan2(-dY,dX); // cartesian space
				const MOR_FLTTYPE dRadian2pi=af::getradian0to2pi<MOR_FLTTYPE>(dRadian);
				(*j)={n,dRadian2pi};
			}
			std::sort(vRadians.begin(),vRadians.end(),[](const std::pair<int,MOR_FLTTYPE>& a,const std::pair<int,MOR_FLTTYPE>& b)->bool{return a.second < b.second;});
			std::shared_ptr<af2d::pointvec<MOR_FLTTYPE>> spLogTriangulatePts=std::shared_ptr<af2d::pointvec<MOR_FLTTYPE>>(new af2d::pointvec<MOR_FLTTYPE>);
			spLogTriangulatePts->get().resize(vRadians.size());
			{
				auto i=vRadians.cbegin(),end=vRadians.cend();
				auto j=spLogTriangulatePts->get().begin();
				for(;i!=end;++i,++j)
					(*j)=m_spLogTriangulatePts->get()[(*i).first];
			}
			m_spLogTriangulatePts=spLogTriangulatePts;
		}
		
		return m_spLogTriangulatePts.get();
	}
protected:
	std::shared_ptr<af2d::pointvec<long>> m_spLogPts;					// logical
	std::shared_ptr<std::vector<MOR_FLTTYPE>> m_spLogLengths;			// logical lengths
	std::shared_ptr<af2d::pointvec<MOR_FLTTYPE>> m_spLogTriangulatePts;	// logical triangulate samples
	std::shared_ptr<af2d::pointvec<long>> m_spDevPts;					// device
	af2d::rect<long> m_LogBBox;
	MOR_FLTTYPE m_dLogLength;
	std::pair<primitive*,GUID> m_Other;
	int m_nLogSamples;
	GUID m_GUID;

	__forceinline void splitsegment(const std::vector<af2d::point<long>>& vFrom,const std::vector<MOR_FLTTYPE>& vSegmentLengths,
									const int nSegment,const MOR_FLTTYPE dMidLength,const int nMidPoints,int& nMidPoint,MOR_FLTTYPE& dAggLength,
									std::vector<af2d::point<MOR_FLTTYPE>>& vTo)
	{
		af2d::point<MOR_FLTTYPE> segmentFrom(vFrom[nSegment].getx()+0.0,vFrom[nSegment].gety()+0.0);
		const af2d::point<MOR_FLTTYPE> segmentTo(vFrom[nSegment+1].getx()+0.0,vFrom[nSegment+1].gety()+0.0);
		MOR_FLTTYPE dSegmentLength=vSegmentLengths[nSegment];
		for(;nMidPoint<nMidPoints;)
		{
			const MOR_FLTTYPE dTargetAggLength=(1+nMidPoint)*dMidLength;
			if(dSegmentLength + dAggLength < dTargetAggLength)
			{
				dAggLength+=dSegmentLength;
				return;
			}

			// lerp
			const MOR_FLTTYPE dReqSegmentLength=(dTargetAggLength-dAggLength);
			const MOR_FLTTYPE dLerp=std::min<>(1.0,std::max<>(0.0,dReqSegmentLength/dSegmentLength));
						
			// lerp between [segmentFrom,segmentTo]
			const af2d::point<MOR_FLTTYPE> ptLerp(af::getlerp<MOR_FLTTYPE,MOR_FLTTYPE>(segmentFrom.getx(),segmentTo.getx(),dLerp),
												  af::getlerp<MOR_FLTTYPE,MOR_FLTTYPE>(segmentFrom.gety(),segmentTo.gety(),dLerp));
			vTo[nMidPoint+1]=ptLerp;
			
			// update segmentFrom
			segmentFrom=ptLerp;

			// update dSegmentLength
			dSegmentLength-=dReqSegmentLength;

			// update dAggLength
			dAggLength+=dReqSegmentLength;

			// update mid point
			++nMidPoint;
		}
	}
};

}
