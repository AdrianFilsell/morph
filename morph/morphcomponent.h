#pragma once

#include "dt_ocv.h"
#include "thread_taskscheduler.h"
#include "dib.h"
#include "primitive.h"

namespace afmorph
{

class morph;

class morphcomponent
{
public:
	enum type {t_src=0x1,t_dst=0x2,t_lerp=0x4,t_all=(t_src|t_dst|t_lerp)};
	enum ltodtype {lt_triandulated=0x1,lt_prim=0x2,lt_all=(lt_triandulated|lt_prim)};

	const morph *m_pMorph;													// morph root
	
	const type m_Type;														// component

	af2d::rect<long> m_rLog;												// extents

	std::shared_ptr<afdib::dib> m_spDib;									// dib
	CString m_csPath;														// dib path

	std::vector<std::shared_ptr<primitive>> m_vPrims;						// prims
	af2d::rect<> m_rLinkedLogBBox;											// linked prims bbox
	primitive *m_pSel;														// selected prim
	
	std::shared_ptr<af2d::pointvec<MOR_FLTTYPE>> m_spTriangulate;			// triangulate points
	std::shared_ptr<af2d::pointvec<long>> m_spDevTriangulate;				// triangulate points ( dev )

	std::vector<dt_ocv::subdiv<>::tri> m_vLogTriangulated;					// triangulated points
	std::vector<Eigen::Matrix<double, 2, 3>> m_vLogTriangulatedToSrc;		// triangulated points to src triangulated points
	std::vector<Eigen::Matrix<double, 2, 3>> m_vLogTriangulatedToDst;		// triangulated points to dst triangulated points

	std::shared_ptr<const Eigen::Affine2d> m_spLogToDev;					// logical to dst xform
	
	bool read(const serialise *pS);
	bool write(const serialise *pS) const;

	bool validateread(const CString& csArchive);

	void clear(void)
	{
		m_rLog=af2d::rect<long>();
		m_spDib=nullptr;
		m_csPath=s_szNull;
		m_vPrims.clear();
		m_rLinkedLogBBox=af2d::rect<>();
		m_pSel=nullptr;
		m_spLogToDev=nullptr;
		cleartriangulated();
	}

	void setdib(std::shared_ptr<afdib::dib> sp)
	{
		clear();
		m_spDib=sp;
		af2d::rect<> rLog;
		if(m_spDib)
			rLog={{0,0},{m_spDib->getwidth(),m_spDib->getheight()}};
		m_rLog=rLog;
	}
	
	void erase(void)
	{
		m_vPrims.clear();
		m_pSel=nullptr;
		cleartriangulated();
	}
	void erase(primitive *p)
	{
		auto i=m_vPrims.begin(),end=m_vPrims.end();
		for(;i!=end;++i)
			if((*i).get()==p)
			{
				if(p==m_pSel)
					m_pSel=nullptr;
				const bool bLinked=!!p->getother().first;
				if(bLinked)
					cleartriangulated();
				m_vPrims.erase(i);
				if(bLinked)
					getlinkedlogbbox();
				break;
			}
	}

	void clearother(void)
	{
		auto i=m_vPrims.begin(),end=m_vPrims.end();
		for(;i!=end;++i)
			(*i)->setother(nullptr);
		getlinkedlogbbox();
	}	
	void setother(primitive *p,primitive *pOther)
	{
		if(!p || !p->validother(pOther))return;
		if(p->getother().first==pOther && (!pOther || pOther->getother().first==p))return;
		if(pOther)
			clearother(pOther);
		p->setother(pOther);
		getlinkedlogbbox();
	}
	void clearother(primitive *p)
	{
		auto i=m_vPrims.cbegin(),end=m_vPrims.cend();
		for(;i!=end;++i)
			if((*i)->getother().first==p)
			{
				(*i)->setother(nullptr);
				break;
			}
	}

	void triangulate(void)
	{
		switch(m_Type)
		{
			case t_src:
			case t_dst:triangulate_srcdst();break;
			case t_lerp:triangulate_lerp();break;
		}
		triangulatedev(lt_triandulated);
	}
	void triangulatedev(const int n)
	{
		if(!m_spLogToDev)return;
		const xform<> trns(m_spLogToDev.get(),nullptr,m_rLog.getwidth(),m_rLog.getheight());
		if(n&lt_prim)
			if(!m_rLog.isempty())
			{
				auto i=m_vPrims.cbegin(),end=m_vPrims.cend();
				for(;i!=end;++i)
					(*i)->setlogtodev(trns);
			}
		if(n&lt_triandulated)
			if(!m_rLog.isempty())
			{
				m_spDevTriangulate=m_spDevTriangulate?m_spDevTriangulate:std::shared_ptr<af2d::pointvec<long>>(new af2d::pointvec<long>);
				m_spDevTriangulate->get().resize(m_vLogTriangulated.size()*3);
				auto i = m_vLogTriangulated.cbegin(),end=m_vLogTriangulated.cend();
				auto j=m_spDevTriangulate->get().begin();
				for(;i!=end;++i)
					for(int n=0;n<3;++n,++j)
						trns.ltod((*i).abc[n],*j);
			}
			else
				m_spDevTriangulate=nullptr;
	}
	
	static const TCHAR s_szNull[];
	static __forceinline bool isinsidetri(const MOR_FLTTYPE x,const MOR_FLTTYPE y, const dt_ocv::subdiv<>::tri& t)
	{
		// Edge vectors and point differences
		const af2d::point<>& A=t.abc[0];
		const af2d::point<>& B=t.abc[1];
		const af2d::point<>& C=t.abc[2];

		const af2d::point<>& AB = t.ab;
		const af2d::point<> AP = {x - A.getx(), y - A.gety()};
		const af2d::point<>& BC = t.bc;
		const af2d::point<> BP = {x - B.getx(), y - B.gety()};
		const af2d::point<>& CA = t.ca;
		const af2d::point<> CP = {x - C.getx(), y - C.gety()};
	
		// Compute cross products
		MOR_FLTTYPE area1 = (AB.getx()*AP.gety() - AB.gety()*AP.getx());
		MOR_FLTTYPE area2 = (BC.getx()*BP.gety() - BC.gety()*BP.getx());
		MOR_FLTTYPE area3 = (CA.getx()*CP.gety() - CA.gety()*CP.getx());

		if(area1 >= 0 && area2 >= 0 && area3 >= 0)
			return true;
		return false;
	}
	static __forceinline void getAffineTransform_edge(const af2d::point<> *pSrc,const af2d::point<> *pDst,Eigen::Matrix<double, 2, 3>& xfmSrcToDst)
	{
		// find longest edges
		int nSrc=0,nDst=0;
		MOR_FLTTYPE dSrcSq,dDstSq,dTmp;
		dSrcSq=(pSrc[0]-pSrc[1]).getlengthsq();
		dTmp=(pSrc[1]-pSrc[2]).getlengthsq();
		if(dTmp>dSrcSq)
		{
			nSrc=1;
			dSrcSq=dTmp;
		}
		dTmp=(pSrc[2]-pSrc[0]).getlengthsq();
		if(dTmp>dSrcSq)
		{
			dSrcSq=dTmp;
			nSrc=2;
		}
		dDstSq=(pDst[0]-pDst[1]).getlengthsq();
		dTmp=(pDst[1]-pDst[2]).getlengthsq();
		if(dTmp>dDstSq)
		{
			nDst=1;
			dDstSq=dTmp;
		}
		dTmp=(pDst[2]-pDst[0]).getlengthsq();
		if(dTmp>dDstSq)
		{
			dDstSq=dTmp;
			nDst=2;
		}
		const MOR_FLTTYPE dTol=1e-5;
		const bool bSrcPt=sqrt(dSrcSq)<dTol;
		const bool bDstPt=sqrt(dDstSq)<dTol;
		if(bSrcPt || bDstPt)
		{
			// Source points
			const MOR_FLTTYPE x1 = af::getlerp<MOR_FLTTYPE,MOR_FLTTYPE>(pSrc[nSrc].getx(),pSrc[nSrc==2?0:nSrc+1].getx(),0.5),
							  y1 = af::getlerp<MOR_FLTTYPE,MOR_FLTTYPE>(pSrc[nSrc].gety(),pSrc[nSrc==2?0:nSrc+1].gety(),0.5);

			// Destination points
			const MOR_FLTTYPE u1 = af::getlerp<MOR_FLTTYPE,MOR_FLTTYPE>(pDst[nDst].getx(),pDst[nDst==2?0:nDst+1].getx(),0.5),
							  v1 = af::getlerp<MOR_FLTTYPE,MOR_FLTTYPE>(pDst[nDst].gety(),pDst[nDst==2?0:nDst+1].gety(),0.5);
		
			xfmSrcToDst << 1, 0, u1-x1,
						   0, 1, v1-y1;
		}
		else
		{
			// Source points
			const MOR_FLTTYPE x1 = pSrc[nSrc].getx(), y1 = pSrc[nSrc].gety();
			const MOR_FLTTYPE x2 = pSrc[nSrc==2?0:nSrc+1].getx(), y2 = pSrc[nSrc==2?0:nSrc+1].gety();

			// Destination points
			const MOR_FLTTYPE u1 = pDst[nDst].getx(), v1 = pDst[nDst].gety();
			const MOR_FLTTYPE u2 = pDst[nDst==2?0:nDst+1].getx(), v2 = pDst[nDst==2?0:nDst+1].gety();

			// Linear system matrix
			Eigen::Matrix<MOR_FLTTYPE, 4, 6> A;
			A << x1, y1, 1, 0,  0,  0,
			0,  0,  0, x1, y1, 1,
			x2, y2, 1, 0,  0,  0,
			0,  0,  0, x2, y2, 1;

			// Right-hand side
			Eigen::Matrix<MOR_FLTTYPE, 4, 1> B;
			B << u1, v1, u2, v2;

			// Solve the system
			Eigen::Matrix<MOR_FLTTYPE, 6, 1> params = A.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV).solve(B);

			// Construct the affine transformation matrix
			xfmSrcToDst << params(0), params(1), params(2),
			params(3), params(4), params(5);
		}
	}
	static __forceinline void getAffineTransform(const af2d::point<> *pSrc,const af2d::point<> *pDst,Eigen::Matrix<double, 2, 3>& xfmSrcToDst)
	{
		// Construct the linear system A * params = B
		Eigen::Matrix<MOR_FLTTYPE, 6, 6> A = Eigen::Matrix<MOR_FLTTYPE, 6, 6>::Zero();
		Eigen::Matrix<MOR_FLTTYPE, 6, 1> B;

		// Check for collinearity
		auto computeArea = [](const af2d::point<>* points) {
		return std::abs(
			points[0].getx() * (points[1].gety() - points[2].gety()) +
			points[1].getx() * (points[2].gety() - points[0].gety()) +
			points[2].getx() * (points[0].gety() - points[1].gety())
			) / 2.0;
		};

		for (int i = 0; i < 3; ++i) {
			// Source points
			const MOR_FLTTYPE x = pSrc[i].getx();
			const MOR_FLTTYPE y = pSrc[i].gety();

			// Destination points
			const MOR_FLTTYPE u = pDst[i].getx();
			const MOR_FLTTYPE v = pDst[i].gety();

			// Fill in rows for u and v
			A(i, 0) = x; A(i, 1) = y; A(i, 2) = 1;
			A(i + 3, 3) = x; A(i + 3, 4) = y; A(i + 3, 5) = 1;

			B(i) = u;
			B(i + 3) = v;
		}

		const MOR_FLTTYPE areaSrc = computeArea(pSrc);
		const MOR_FLTTYPE areaDst = computeArea(pDst);
		
		// If either triangle is degenerate, return an identity transform or handle as needed
		const MOR_FLTTYPE dTol=1e-5;
		if (areaSrc < dTol || areaDst < dTol) {
			// Degenerate points detected. Returning identity transform.
			getAffineTransform_edge(pSrc,pDst,xfmSrcToDst);
//			xfmSrcToDst = Eigen::Matrix<double, 2, 3>::Identity();
			return;
		}

		// Solve using Gaussian elimination
		Eigen::PartialPivLU<Eigen::Matrix<MOR_FLTTYPE, 6, 6>> lu_decomp(A);
		Eigen::Matrix<MOR_FLTTYPE, 6, 1> params = lu_decomp.solve(B);

		// Construct the affine transformation matrix
		xfmSrcToDst << params(0), params(1), params(2),
		params(3), params(4), params(5);
	}
/*Eigen::Matrix<double, 2, 3> dibwnd::computeAffineTransformSimple(
    const Eigen::Matrix<double, 3, 2>& triangleA,
    const Eigen::Matrix<double, 3, 2>& triangleB)const {
    Eigen::Matrix<double, 6, 6> A; // Coefficient matrix
    Eigen::Matrix<double, 6, 1> B; // Result vector

    // Fill A and B with the linear system to solve
    for (int i = 0; i < 3; ++i) {
        A.row(2 * i)     << triangleA(i, 0), triangleA(i, 1), 1, 0, 0, 0;
        A.row(2 * i + 1) << 0, 0, 0, triangleA(i, 0), triangleA(i, 1), 1;

        B(2 * i) = triangleB(i, 0);
        B(2 * i + 1) = triangleB(i, 1);
    }

    // Solve the system
    Eigen::Matrix<double, 6, 1> affineParams = A.colPivHouseholderQr().solve(B);

    // Populate the affine transformation matrix
    Eigen::Matrix<double, 2, 3> affineTransform;
    affineTransform << affineParams(0), affineParams(1), affineParams(2),
                       affineParams(3), affineParams(4), affineParams(5);

    return affineTransform;
}*/
protected:
	morphcomponent(const morph *pM,const type t):m_Type(t)
	{
		m_pMorph=pM;
		m_csPath=s_szNull;
		m_pSel=nullptr;
	}
	virtual ~morphcomponent(){}
	bool validatepath(const CString& csArchive,CString& cs)const;
	void cleartriangulated(void)
	{
		m_vLogTriangulatedToSrc.clear();
		m_vLogTriangulatedToDst.clear();
		m_vLogTriangulated.clear();
		m_spDevTriangulate=nullptr;
		m_spTriangulate=nullptr;
	}
	void getlinkedlogbbox(void)
	{
		m_rLinkedLogBBox=af2d::rect<>();
		const std::vector<std::shared_ptr<primitive>>& vPrims=m_vPrims;
		if(vPrims.size())
		{
			auto i=vPrims.cbegin(),end=vPrims.cend();
			m_rLinkedLogBBox={{(*i)->getlogbbox().get(af2d::rect<long>::tl).getx(),(*i)->getlogbbox().get(af2d::rect<long>::tl).gety()},
				{(*i)->getlogbbox().get(af2d::rect<long>::br).getx(),(*i)->getlogbbox().get(af2d::rect<long>::br).gety()}};
			for(++i;i!=end;++i)
				m_rLinkedLogBBox=m_rLinkedLogBBox.getunion({{(*i)->getlogbbox().get(af2d::rect<long>::tl).getx(),(*i)->getlogbbox().get(af2d::rect<long>::tl).gety()},
									{(*i)->getlogbbox().get(af2d::rect<long>::br).getx(),(*i)->getlogbbox().get(af2d::rect<long>::br).gety()}});
		}
	}
	void triangulate_srcdst(void);
	void triangulate_lerp(void);
	void triangulate_lerpxforms(void);
};

class morphsrc:public morphcomponent{public:morphsrc(const morph *pM):morphcomponent(pM,t_src){}virtual ~morphsrc(){}};
class morphdst:public morphcomponent{public:morphdst(const morph *pM):morphcomponent(pM,t_dst){}virtual ~morphdst(){}};
class morphlerp:public morphcomponent{public:morphlerp(const morph *pM):morphcomponent(pM,t_lerp){}virtual ~morphlerp(){}};

class morph
{
public:
	enum rendertype {rt_sample=0x1,rt_triangulate=0x2,rt_path=0x4,rt_link=0x8};

	morph();
	~morph(){}

	bool read(const serialise *pS);
	bool write(const serialise *pS)const;

	bool validateread(const CString& csArchive);
	
	morphcomponent& getcomponent(const morphcomponent::type t,const bool bOther=false)const
	{
		switch(t)
		{
			case morphcomponent::t_src:if(bOther)return *m_spDst; return *m_spSrc;
			case morphcomponent::t_dst:if(bOther)return *m_spSrc; return *m_spDst;
			default:if(bOther) return *m_spSrc; return *m_spLerp;
		}
	}

	int m_nRender;
	bool m_bTriangulateBkgnd;
	int m_nLerpFrames;
	int m_nLerpFrame;

	std::shared_ptr<morphsrc> m_spSrc;
	std::shared_ptr<morphdst> m_spDst;
	std::shared_ptr<morphlerp> m_spLerp;
};

struct normrect
{
	// map first pixel centre to 0 and last to 1
	normrect(const af2d::rect<>& rNonInclusive,const MOR_FLTTYPE dDim):m_dDim(dDim),m_nW(rNonInclusive.getwidth()),m_nH(rNonInclusive.getheight()),m_dToNormX(1.0/(m_nW-1)),m_dToNormY(1.0/(m_nH-1)){}
	template <typename T> __forceinline MOR_FLTTYPE norm_x(const T n)const{return (n-0.5)*m_dToNormX;}
	template <typename T> __forceinline MOR_FLTTYPE norm_y(const T n)const{return (n-0.5)*m_dToNormY;}
	template <typename T> __forceinline T x(const MOR_FLTTYPE d)const{return d*(m_nW-1)+0.5;}
	template <typename T> __forceinline T y(const MOR_FLTTYPE d)const{return d*(m_nH-1)+0.5;}
		
	const int m_nW;
	const int m_nH;
	const MOR_FLTTYPE m_dDim;
	const MOR_FLTTYPE m_dToNormX;
	const MOR_FLTTYPE m_dToNormY;
};

}
