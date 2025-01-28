// traincostgraphwnd.cpp : implementation file
//

#include "pch.h"
#include "dibwnd.h"
#include "core.h"
#include "morph.h"
#include "morphDlg.h"
#include "serialise.h"
#include "jpeg.h"

class renderlerpscanlineop
{
public:
	renderlerpscanlineop(const int nXFrom,const int nXNonInclusiveTo,const int nYFrom,const af2d::rect<>& rLogClip,
						 unsigned char *pCanvasScanlines,const int nCanvasBytesPerScanline,const int nCanvasBytesPerPixel,
						 const unsigned char *pSrcBGRScanlines,const int nSrcBytesPerScanline,
						 const unsigned char *pDstBGRScanlines,const int nDstBytesPerScanline,
						 const Eigen::Affine2d *pDevToLog,
						 const dt_ocv::subdiv<>::tri& logTriangulated,
						 const Eigen::Matrix<double, 2, 3>& logLerpToSrc,const Eigen::Matrix<double, 2, 3>& logLerpToDst,
						 const int nSrcWidth,const int nSrcHeight,const int nDstWidth,const int nDstHeight,
						 const MOR_FLTTYPE dSrcLerp,const MOR_FLTTYPE dDstLerp):
			m_nXFrom(nXFrom),m_nXNonInclusiveTo(nXNonInclusiveTo),m_nYFrom(nYFrom),m_rLogClip(rLogClip),
			m_pCanvasScanlines(pCanvasScanlines),m_nCanvasBytesPerPixel(nCanvasBytesPerPixel),m_nCanvasBytesPerScanline(nCanvasBytesPerScanline),m_pSrcBGRScanlines(pSrcBGRScanlines),m_nSrcBytesPerScanline(nSrcBytesPerScanline),m_pDstBGRScanlines(pDstBGRScanlines),m_nDstBytesPerScanline(nDstBytesPerScanline),
			m_pDevToLog(pDevToLog),
			m_LogTriangulated(logTriangulated),
			m_LogLerpToSrc(logLerpToSrc),m_LogLerpToDst(logLerpToDst),
			m_nSrcWidth(nSrcWidth),m_nSrcHeight(nSrcHeight),m_nDstWidth(nDstWidth),m_nDstHeight(nDstHeight),
			m_dSrcLerp(dSrcLerp),m_dDstLerp(dDstLerp){}
	void operator()(const int nFrom,const int nInclusiveTo,const afthread::taskinfo *m_pTaskInfo)const
	{
		const MOR_FLTTYPE dLtoD00=m_LogLerpToDst.coeff(0,0),dLtoD01=m_LogLerpToDst.coeff(0,1),dLtoD02=m_LogLerpToDst.coeff(0,2);
		const MOR_FLTTYPE dLtoD10=m_LogLerpToDst.coeff(1,0),dLtoD11=m_LogLerpToDst.coeff(1,1),dLtoD12=m_LogLerpToDst.coeff(1,2);

		const MOR_FLTTYPE dLtoS00=m_LogLerpToSrc.coeff(0,0),dLtoS01=m_LogLerpToSrc.coeff(0,1),dLtoS02=m_LogLerpToSrc.coeff(0,2);
		const MOR_FLTTYPE dLtoS10=m_LogLerpToSrc.coeff(1,0),dLtoS11=m_LogLerpToSrc.coeff(1,1),dLtoS12=m_LogLerpToSrc.coeff(1,2);
		
		for(int n=nFrom;n<=nInclusiveTo;++n)
		{
			const int nY=m_nYFrom+n;			
			unsigned char *pCanvas=m_pCanvasScanlines+(nY*m_nCanvasBytesPerScanline);
			
			for(int nX=m_nXFrom;nX<m_nXNonInclusiveTo;++nX)
			{
				const Eigen::Vector2d devLerp(nX+0.5,nY+0.5);
				const Eigen::Vector2d logLerp=(*m_pDevToLog)*devLerp;
				
				if(logLerp.x()<0 || logLerp.y()<0 ||
				   logLerp.x()>=m_rLogClip.get(af2d::rect<>::br).getx() || logLerp.y()>=m_rLogClip.get(af2d::rect<>::br).gety())
					continue;

				if(!afmorph::morphcomponent::isinsidetri(logLerp.x(),logLerp.y(),m_LogTriangulated))
					continue;

				const MOR_FLTTYPE dSrcX=dLtoS00*logLerp.x() + dLtoS01*logLerp.y() + dLtoS02;
				const MOR_FLTTYPE dSrcY=dLtoS10*logLerp.x() + dLtoS11*logLerp.y() + dLtoS12;
				const bool bValidSrc=!((dSrcX<0 || dSrcY<0) || (dSrcX>=m_nSrcWidth || dSrcY>=m_nSrcHeight));
				
				const MOR_FLTTYPE dDstX=dLtoD00*logLerp.x() + dLtoD01*logLerp.y() + dLtoD02;
				const MOR_FLTTYPE dDstY=dLtoD10*logLerp.x() + dLtoD11*logLerp.y() + dLtoD12;
				const bool bValidDst=!((dDstX<0 || dDstY<0) || (dDstX>=m_nDstWidth || dDstY>=m_nDstHeight));
				
				if(bValidSrc && bValidDst)
				{
					const af2d::point<int> ptDiscreteSrc(af::posfloor<MOR_FLTTYPE,int>(dSrcX),af::posfloor<MOR_FLTTYPE,int>(dSrcY));
					const af2d::point<int> ptDiscreteDst(af::posfloor<MOR_FLTTYPE,int>(dDstX),af::posfloor<MOR_FLTTYPE,int>(dDstY));

					const unsigned char *pSrcBGR=m_pSrcBGRScanlines+(m_nSrcBytesPerScanline*ptDiscreteSrc.gety())+(3*ptDiscreteSrc.getx());
					const unsigned char *pDstBGR=m_pDstBGRScanlines+(m_nDstBytesPerScanline*ptDiscreteDst.gety())+(3*ptDiscreteDst.getx());
				
					pCanvas[0+(nX*m_nCanvasBytesPerPixel)]=af::posfloor<MOR_FLTTYPE,unsigned char>(0.5+(pSrcBGR[0]*m_dSrcLerp)+(pDstBGR[0]*m_dDstLerp));
					pCanvas[1+(nX*m_nCanvasBytesPerPixel)]=af::posfloor<MOR_FLTTYPE,unsigned char>(0.5+(pSrcBGR[1]*m_dSrcLerp)+(pDstBGR[1]*m_dDstLerp));
					pCanvas[2+(nX*m_nCanvasBytesPerPixel)]=af::posfloor<MOR_FLTTYPE,unsigned char>(0.5+(pSrcBGR[2]*m_dSrcLerp)+(pDstBGR[2]*m_dDstLerp));
				}
				else
				if(bValidSrc)
				{
					const af2d::point<int> ptDiscreteSrc(af::posfloor<MOR_FLTTYPE,int>(dSrcX),af::posfloor<MOR_FLTTYPE,int>(dSrcY));

					const unsigned char *pSrcBGR=m_pSrcBGRScanlines+(m_nSrcBytesPerScanline*ptDiscreteSrc.gety())+(3*ptDiscreteSrc.getx());
				
					pCanvas[0+(nX*m_nCanvasBytesPerPixel)]=pSrcBGR[0];
					pCanvas[1+(nX*m_nCanvasBytesPerPixel)]=pSrcBGR[1];
					pCanvas[2+(nX*m_nCanvasBytesPerPixel)]=pSrcBGR[2];
				}
				else
				if(bValidDst)
				{
					const af2d::point<int> ptDiscreteDst(af::posfloor<MOR_FLTTYPE,int>(dDstX),af::posfloor<MOR_FLTTYPE,int>(dDstY));

					const unsigned char *pDstBGR=m_pDstBGRScanlines+(m_nDstBytesPerScanline*ptDiscreteDst.gety())+(3*ptDiscreteDst.getx());
				
					pCanvas[0+(nX*m_nCanvasBytesPerPixel)]=pDstBGR[0];
					pCanvas[1+(nX*m_nCanvasBytesPerPixel)]=pDstBGR[1];
					pCanvas[2+(nX*m_nCanvasBytesPerPixel)]=pDstBGR[2];
				}
			}
		}
	}
protected:
	const int m_nXFrom;
	const int m_nXNonInclusiveTo;
	const int m_nYFrom;
	const af2d::rect<>& m_rLogClip;
	unsigned char *m_pCanvasScanlines;
	const int m_nCanvasBytesPerScanline;
	const int m_nCanvasBytesPerPixel;
	const unsigned char *m_pSrcBGRScanlines;
	const int m_nSrcBytesPerScanline;
	const unsigned char *m_pDstBGRScanlines;
	const int m_nDstBytesPerScanline;
	const Eigen::Affine2d *m_pDevToLog;
	const dt_ocv::subdiv<>::tri& m_LogTriangulated;
	const Eigen::Matrix<double, 2, 3>& m_LogLerpToSrc;
	const Eigen::Matrix<double, 2, 3>& m_LogLerpToDst;
	const int m_nSrcWidth;
	const int m_nSrcHeight;
	const int m_nDstWidth;
	const int m_nDstHeight;
	const MOR_FLTTYPE m_dSrcLerp;
	const MOR_FLTTYPE m_dDstLerp;
};

// dibwnd

IMPLEMENT_DYNAMIC(dibwnd, CWnd)

const MOR_FLTTYPE dibwnd::s_dNormDim=10000;

const int dibwnd::s_nHandleDim=11;
const int dibwnd::s_nPathWidth=2;
const int dibwnd::s_nConnWidth=2;
const int dibwnd::s_nCentreWidth=2;
const int dibwnd::s_nSelWidth=3;
const int dibwnd::s_nTriWidth=1;

const int dibwnd::s_nChequerDim=10;
const int dibwnd::s_nDibBorderGap=10;
const int dibwnd::s_nSelBorderDim=5;
const int dibwnd::s_nSelBorderGap=(s_nHandleDim+1)/2;

const unsigned char dibwnd::s_cWhite[3]={255,255,255};
const unsigned char dibwnd::s_cGrey[3]={200,200,200};
const COLORREF dibwnd::s_cPath=RGB(65,200,65);
const COLORREF dibwnd::s_cCentrePath=RGB(255,0,0);
const COLORREF dibwnd::s_cSamplePath=RGB(255,0,0);
const COLORREF dibwnd::s_cConn=RGB(65,200,65);
const COLORREF dibwnd::s_cBorder=RGB(200,200,200);
const COLORREF dibwnd::s_cSelBorder=RGB(65,65,200);
const COLORREF dibwnd::s_cTri=RGB(255,0,0);

dibwnd::dibwnd(const afmorph::morphcomponent::type t)
{
	m_Type=t;
	m_bCaptured=false;
	m_Mode=mt_feature;
}

dibwnd::~dibwnd()
{
}


BEGIN_MESSAGE_MAP(dibwnd, CWnd)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// dibwnd message handlers
void dibwnd::OnSize(UINT nType, int cx, int cy)
{
	// base class
	CWnd::OnSize(nType, cx, cy);

	CDC *pDC = GetDC();
	if(m_OffscreenBmp.GetSafeHandle())
		m_OffscreenBmp.DeleteObject();
	const BOOL b = m_OffscreenBmp.CreateCompatibleBitmap(pDC,cx,cy);
	ReleaseDC(pDC);

	m_spCanvas=std::shared_ptr<afdib::dib>(new afdib::dib);
	m_spCanvas->create(cx,cy,afdib::dib::pt_b8g8r8);

	auto& cmp=theApp.m_spMorph->getcomponent(m_Type);
	composetransforms(m_spDevToLog,cmp.m_spLogToDev,{{0,0},{cx,cy}},cmp.m_rLog);
	
	theApp.m_spMorph->getcomponent(m_Type).triangulatedev(afmorph::morphcomponent::lt_all);
	
	const af2d::rect<> r({{0,0},{m_spCanvas->getwidth(),m_spCanvas->getheight()}});
	composebkgnd(r,m_spCanvas);
	composedib(cmp.m_spLogToDev.get(),m_spDevToLog.get(),r,cmp.m_spDib,m_spCanvas);

	Invalidate();
}

BOOL dibwnd::OnEraseBkgnd(CDC *pDC)
{
	return m_spCanvas ? TRUE : CWnd::OnEraseBkgnd(pDC);
}

void dibwnd::OnPaint(void)
{
	CPaintDC dc(this);

	CRect rcUpdate;
	dc.GetClipBox(&rcUpdate);

	CDC memDC;
	if(memDC.CreateCompatibleDC(&dc))
	{
		CBitmap *pOldBmp = memDC.SelectObject(&m_OffscreenBmp);
		render(&memDC,rcUpdate);
		const BOOL b = dc.BitBlt(rcUpdate.left,rcUpdate.top,rcUpdate.Width(),rcUpdate.Height(),&memDC,rcUpdate.left,rcUpdate.top,SRCCOPY);
		memDC.SelectObject(pOldBmp);
	}
	else
		render(&dc,rcUpdate);
}

void dibwnd::OnMouseMove( UINT nFlags, CPoint point )
{
	CWnd::OnMouseMove( nFlags, point );

	if(isdragging())
		drag(point);
	else
		m_spHT=getht(point);
	
	setcursor();
}

void dibwnd::OnLButtonDown( UINT nFlags, CPoint point )
{
	CWnd::OnLButtonDown( nFlags, point );

	if(!m_spDevToLog || m_Type==afmorph::morphcomponent::t_lerp)
		return;

	int nDragMoveThreshold=3;
	const lbuttonwaittype wt = lbuttondownwait(this,{point.x,point.y},nDragMoveThreshold);
		
	const bool bClickPending = clickpending( wt );
	const bool bDragPending = dragpending( wt );

	auto& cmp=theApp.m_spMorph->getcomponent(m_Type);
	switch(m_Mode)
	{
		case mt_feature:
		{
			if(bClickPending && !cmp.m_rLog.isempty())
			{
				const afmorph::xform<> trns(cmp.m_spLogToDev.get(),m_spDevToLog.get(),cmp.m_rLog.getwidth(),cmp.m_rLog.getheight());
				std::shared_ptr<afmorph::primitive> sp(new afmorph::primitive({{point.x,point.y}},trns));
				
				cmp.m_vPrims.push_back(sp);
				theApp.broadcast(hint(hint::t_pushback,m_Type));

				cmp.m_pSel=sp.get();
				theApp.broadcast(hint(hint::t_select,m_Type));
			}
			else
			if(!isdragging())
			{
				m_spDragPts=std::shared_ptr<af2d::pointvec<long>>(new af2d::pointvec<long>);
				push_back_drag({point.x,point.y});
				
				setcapture();
				setcursor();
			}
		}
		break;
		case mt_select:
		{
			cmp.m_pSel=m_spHT && m_spHT->getprimitive() ? m_spHT->getprimitive() : nullptr;

			theApp.broadcast(hint(hint::t_select,m_Type));
		}
		break;
		default:ASSERT(false);
	}
}

void dibwnd::OnLButtonUp( UINT nFlags, CPoint point )
{
	CWnd::OnLButtonUp( nFlags, point );

	if(isdragging())
	{
		drag(point);
		releasecapture();
		std::shared_ptr<af2d::pointvec<long>> spDragPts=m_spDragPts;
		m_spDragPts=nullptr;
		spDragPts->eraseconincident();
		m_spHT=getht(point);
		setcursor();

		auto& cmp=theApp.m_spMorph->getcomponent(m_Type);
		if(spDragPts && !spDragPts->isempty() && !cmp.m_rLog.isempty())
		{
			const afmorph::xform<> trns(cmp.m_spLogToDev.get(),m_spDevToLog.get(),cmp.m_rLog.getwidth(),cmp.m_rLog.getheight());

			// lets say a drag less than N is a click
			const MOR_FLTTYPE dMin=s_nHandleDim*(2/3.0);
			af2d::rect<> r(spDragPts->get());
			std::shared_ptr<afmorph::primitive> sp;
			if(r.getwidth()<=dMin && r.getheight()<=dMin)
				sp=std::shared_ptr<afmorph::primitive>(new afmorph::primitive({r.getcentre()},trns));
			else
				sp=std::shared_ptr<afmorph::primitive>(new afmorph::primitive(spDragPts->get(),trns));

			cmp.m_vPrims.push_back(sp);
			theApp.broadcast(hint(hint::t_pushback,m_Type));

			cmp.m_pSel=sp.get();
			theApp.broadcast(hint(hint::t_select,m_Type));
		}
	}
}

void dibwnd::render(CDC *pDC,const CRect& rcClip) const
{
	auto& cmp=theApp.m_spMorph->getcomponent(m_Type);
	auto& othercmp=theApp.m_spMorph->getcomponent(m_Type,true);

	if(m_spCanvas)
	{
		CRect rcClientRect;
		GetClientRect(rcClientRect);

		if(m_Type==afmorph::morphcomponent::t_lerp)
			renderlerp(m_spCanvas->getscanline(0),m_spCanvas->getwidth(),m_spCanvas->getheight(),m_spCanvas->getbytesperscanline(),3,rcClip,m_spDevToLog.get(),cmp.m_spLogToDev.get());

		const CRect rcDst = rcClientRect;
		const CRect rcSrc(0,0,m_spCanvas->getwidth(),m_spCanvas->getheight());
		
		BITMAPINFO *pBmi = m_spCanvas->createbitmapinfo();
		const int n = ::StretchDIBits(pDC->GetSafeHdc(),
									  rcDst.TopLeft().x,rcDst.TopLeft().y,rcDst.Width(),rcDst.Height(),
									  rcSrc.TopLeft().x,rcSrc.TopLeft().y,rcSrc.Width(),rcSrc.Height(),
									  m_spCanvas->getscanline(0),pBmi,DIB_RGB_COLORS,SRCCOPY);
		m_spCanvas->tidybmi(pBmi);
	}
	else
		pDC->FillSolidRect(rcClip,::GetSysColor(COLOR_WINDOW));

	if(isdragging())
	{
		CPen pathpen(PS_SOLID,s_nPathWidth,s_cPath);
		
		CPen *pOldPen=pDC->SelectObject(&pathpen);

		POINT *pPts=(POINT*)(&(m_spDragPts->get()[0]));
		pDC->Polyline(pPts,static_cast<int>(m_spDragPts->get().size()));
		
		pDC->SelectObject(pOldPen);
	}
	
	if((theApp.m_spMorph->m_nRender & afmorph::morph::rt_triangulate) && cmp.m_spDevTriangulate && !cmp.m_spDevTriangulate->isempty())
	{
		// triangles
		CPen pathpen(PS_SOLID,s_nTriWidth,s_cTri);
		
		CPen *pOldPen=pDC->SelectObject(&pathpen);
	
		POINT *pPts=(POINT*)(&(cmp.m_spDevTriangulate->get()[0]));
		const size_t nTris = cmp.m_spDevTriangulate->get().size()/3;
		for(size_t n=0;n<nTris;++n,pPts+=3)
		{
			pDC->Polyline(pPts,3);
			pDC->MoveTo(pPts[2]);
			pDC->LineTo(pPts[0]);
		}
		
		pDC->SelectObject(pOldPen);
	}
	
	if(!cmp.m_rLog.isempty() && cmp.m_spLogToDev && m_spDevToLog)
	{
		const afmorph::xform<> trns(cmp.m_spLogToDev.get(),m_spDevToLog.get(),cmp.m_rLog.getwidth(),cmp.m_rLog.getheight());

		// paths/conections
		{
			CPen pathpen(PS_SOLID,s_nPathWidth,s_cPath);
			CPen connpen(PS_SOLID,s_nConnWidth,s_cConn);

			auto i=cmp.m_vPrims.cbegin(),end=cmp.m_vPrims.cend();
			for(;i!=end;++i)
			{
				CPen *pOldPen=pDC->SelectObject(&pathpen);

				POINT *pPts=(POINT*)(&((*i)->getdevpts()->get()[0]));
				if(theApp.m_spMorph->m_nRender & afmorph::morph::rt_path)
					pDC->Polyline(pPts,static_cast<int>((*i)->getdevpts()->get().size()));

				pDC->SelectObject(pOldPen);

				if((theApp.m_spMorph->m_nRender & afmorph::morph::rt_link) && (*i)->getother().first && !othercmp.m_rLog.isempty())
				{
					CPen *pOldPen=pDC->SelectObject(&connpen);

					const afmorph::xform<> othertrns(othercmp.m_spLogToDev.get(),nullptr,othercmp.m_rLog.getwidth(),othercmp.m_rLog.getheight());

					const af2d::point<long> ptLogCentre=(*i)->getlogbbox().getcentre();
					af2d::point<long> devCentre;
					trns.ltod(ptLogCentre,devCentre);
					
					const af2d::point<long> ptLogOtherCentre=(*i)->getother().first->getlogbbox().getcentre();
					af2d::point<long> devOtherCentre;
					othertrns.ltod(ptLogOtherCentre,devOtherCentre);

					CWnd *pOther=theApp.getdlg()->getsplitterwnd()->getpanes()[m_Type==afmorph::morphcomponent::t_src?1:0].get();
					
					::MapWindowPoints(pOther->GetSafeHwnd(),GetSafeHwnd(),(LPPOINT)&devOtherCentre,1);
					
					POINT pts[2]={{devCentre.getx(),devCentre.gety()},{devOtherCentre.getx(),devOtherCentre.gety()}};
					pDC->Polyline(pts,2);

					pDC->SelectObject(pOldPen);
				}
			}	
		}

		// centres
		{
			CPen p(PS_SOLID,s_nCentreWidth,s_cCentrePath);
			CPen *pOldPen=pDC->SelectObject(&p);

			auto i=cmp.m_vPrims.cbegin(),end=cmp.m_vPrims.cend();
			for(;i!=end;++i)
			{
				const af2d::point<long> ptLogCentre=(*i)->getlogbbox().getcentre();
				af2d::point<long> devCentre;
				trns.ltod(ptLogCentre,devCentre);
				{
					CPoint cpTL=CPoint(devCentre.getx(),devCentre.gety())-CPoint(s_nHandleDim/2,s_nHandleDim/2);
					CPoint cpBR=cpTL+CPoint(s_nHandleDim,s_nHandleDim);
					pDC->Ellipse(CRect(cpTL,cpBR));
				}
			}

			pDC->SelectObject(pOldPen);
		}

		// samples
		if((theApp.m_spMorph->m_nRender & afmorph::morph::rt_sample) && cmp.m_spTriangulate)
		{
			CPen p(PS_SOLID,s_nCentreWidth,s_cSamplePath);
			CPen *pOldPen=pDC->SelectObject(&p);

			auto i=cmp.m_spTriangulate->get().cbegin(),end=cmp.m_spTriangulate->get().cend();
			for(;i!=end;++i)
			{
				const af2d::point<long> ptLogCentre(af::floor<MOR_FLTTYPE,long>((*i).getx()),af::floor<MOR_FLTTYPE,long>((*i).gety()));
				af2d::point<long> devCentre;
				trns.ltod(ptLogCentre,devCentre);
				{
					CPoint cpTL=CPoint(devCentre.getx(),devCentre.gety())-CPoint(s_nHandleDim/2,s_nHandleDim/2);
					CPoint cpBR=cpTL+CPoint(s_nHandleDim,s_nHandleDim);
					pDC->Ellipse(CRect(cpTL,cpBR));
				}
			}

			pDC->SelectObject(pOldPen);
		}
		
		// borders
		if(cmp.m_pSel)
		{
			af2d::rect<long> dev;
			trns.ltod(cmp.m_pSel->getlogbbox(),dev);
			renderborder(pDC,dev,s_cSelBorder);
		}
	}
}

void dibwnd::renderlerp(unsigned char *pCanvas,const int nCanvasWidth,const int nCanvasHeight,const int nCanvasBytesPerScanline,const int nCanvasBytesPerPixel,
						const CRect& rcClip,const Eigen::Affine2d *pDevToLog,const Eigen::Affine2d *pLogToDev)
{
	auto& srccmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_src);
	auto& dstcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_dst);
	auto& cmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_lerp);

	if(cmp.m_vLogTriangulatedToSrc.size()==0 || cmp.m_rLog.isempty())
		return;

	const afthread::taskscheduler *pSched=theApp.getsched();

	const int nFrame=theApp.m_spMorph->m_nLerpFrame;
	const int nFrames=theApp.m_spMorph->m_nLerpFrames;
	const MOR_FLTTYPE dLerp=nFrames>1?(nFrame-1)/(static_cast<MOR_FLTTYPE>(nFrames)-1):0;
	const MOR_FLTTYPE dSrcLerp=1-dLerp;
	const MOR_FLTTYPE dDstLerp=dLerp;

	const afdib::dib *pSrcDib=srccmp.m_spDib.get();
	const afdib::dib *pDstDib=dstcmp.m_spDib.get();
	const unsigned char *pSrcBGRScanlines=pSrcDib->getscanline(0);
	const unsigned char *pDstBGRScanlines=pDstDib->getscanline(0);
	const int nSrcBytesPerScanline=pSrcDib->getbytesperscanline();
	const int nDstBytesPerScanline=pDstDib->getbytesperscanline();
	
	const long nSrcWidth=srccmp.m_rLog.getwidth();
	const long nSrcHeight=srccmp.m_rLog.getheight();
	const long nDstWidth=dstcmp.m_rLog.getwidth();
	const long nDstHeight=dstcmp.m_rLog.getheight();

	const int nTris=static_cast<int>(cmp.m_vLogTriangulated.size());
	const afmorph::xform<> trns(pLogToDev,pDevToLog,cmp.m_rLog.getwidth(),cmp.m_rLog.getheight());
	auto iLtoS=cmp.m_vLogTriangulatedToSrc.cbegin();
	auto iLtoD=cmp.m_vLogTriangulatedToDst.cbegin();
	for(int nT=0;nT<nTris;++nT,++iLtoS,++iLtoD) // parallelise
	{
		// log to dev continuous bbox
		if(cmp.m_vLogTriangulated[nT].bbox.isempty())
			continue;
		af2d::rect<long> dev;
		trns.ltod(cmp.m_vLogTriangulated[nT].bbox,dev);
		if(dev.isempty())
			continue;

		// lets inflate
		const long nInflate=5;
		dev.get(af2d::rect<long>::tl).getx()-=nInflate;
		dev.get(af2d::rect<long>::tl).gety()-=nInflate;
		dev.get(af2d::rect<long>::br).getx()+=nInflate;
		dev.get(af2d::rect<long>::br).gety()+=nInflate;

		// intersect
		const af2d::rect<long> inter=dev.getintersect(dev);
		if(inter.isempty())
			continue;
		
		// blend
		const long nYFrom=inter.get(af2d::rect<long>::tl).gety();
		const long nYNonInclusiveTo=inter.get(af2d::rect<long>::br).gety();
		const long nXFrom=inter.get(af2d::rect<long>::tl).getx();
		const long nXNonInclusiveTo=inter.get(af2d::rect<long>::br).getx();
		
		const renderlerpscanlineop op(nXFrom,nXNonInclusiveTo,nYFrom,
									  cmp.m_rLog,
									  pCanvas,nCanvasBytesPerScanline,nCanvasBytesPerPixel,pSrcBGRScanlines,nSrcBytesPerScanline,pDstBGRScanlines,nDstBytesPerScanline,
									  pDevToLog,cmp.m_vLogTriangulated[nT],
									  *iLtoS,*iLtoD,
									  nSrcWidth,nSrcHeight,nDstWidth,nDstHeight,
									  dSrcLerp,dDstLerp);
		if(pSched)
			pSched->parallel_for(0,(nYNonInclusiveTo-nYFrom),pSched->getcores(),op);
		else
			op(0,0+((nYNonInclusiveTo-nYFrom))-1,nullptr);
	}
}

void dibwnd::renderborder(CDC *pDC,const af2d::rect<long>& r,const COLORREF cr)const
{
	CRect rcTop,rcBottom,rcLeft,rcRight;
	rcTop=CRect(r.get(af2d::rect<long>::tl).getx()-s_nSelBorderGap-s_nSelBorderDim,
			 r.get(af2d::rect<long>::tl).gety()-s_nSelBorderGap-s_nSelBorderDim,
			 r.get(af2d::rect<long>::br).getx()+s_nSelBorderGap+s_nSelBorderDim,
			 r.get(af2d::rect<long>::tl).gety()-s_nSelBorderGap);
	pDC->FillSolidRect(rcTop,cr);
	rcBottom=CRect(r.get(af2d::rect<long>::tl).getx()-s_nSelBorderGap-s_nSelBorderDim,
			 r.get(af2d::rect<long>::br).gety()+s_nSelBorderGap,
			 r.get(af2d::rect<long>::br).getx()+s_nSelBorderGap+s_nSelBorderDim,
			 r.get(af2d::rect<long>::br).gety()+s_nSelBorderGap+s_nSelBorderDim);
	pDC->FillSolidRect(rcBottom,cr);
	rcLeft=CRect(rcTop.left,rcTop.bottom,rcTop.left+s_nSelBorderDim,rcBottom.top);
	pDC->FillSolidRect(rcLeft,cr);
	rcRight=CRect(rcTop.right-s_nSelBorderDim,rcTop.bottom,rcTop.right,rcBottom.top);
	pDC->FillSolidRect(rcRight,cr);
}

void dibwnd::canceldrag(void)
{
	if(isdragging())
	{
		CPoint cp;
		::GetCursorPos(&cp);
		::MapWindowPoints(NULL,m_hWnd,&cp,1);

		releasecapture();
		m_spDragPts=nullptr;
		m_spHT=getht(cp);
		setcursor();

		Invalidate();
	}
}

void dibwnd::setcapture(void)
{
	if(m_bCaptured)
		return;
	SetCapture();
	m_bCaptured=true;
}

void dibwnd::releasecapture(void)
{
	if(m_bCaptured)
	{
		ReleaseCapture();
		m_bCaptured=false;
	}
}

std::shared_ptr<hittest> dibwnd::getht(const CPoint& cpDev)const
{
	auto& cmp=theApp.m_spMorph->getcomponent(m_Type);
	if(isdragging() || cmp.m_vPrims.size()==0 || cmp.m_rLog.isempty())
		return nullptr;

	const afmorph::xform<> trns(cmp.m_spLogToDev.get(),m_spDevToLog.get(),cmp.m_rLog.getwidth(),cmp.m_rLog.getheight());
	af2d::point<long> ptDev(cpDev.x,cpDev.y);

	// give priority to centre
	auto i=cmp.m_vPrims.crbegin(),end=cmp.m_vPrims.crend();
	for(;i!=end;++i)
	{
		const af2d::point<long> ptLogCentre=(*i)->getlogbbox().getcentre();
		af2d::point<long> devCentre;
		trns.ltod(ptLogCentre,devCentre);
		const MOR_FLTTYPE d = (devCentre-ptDev).getlength();
		if(d<=(s_nHandleDim+s_nSelWidth)*0.5)
		{
			std::shared_ptr<hittest> sp(new hittest);
			sp->settype(hittest::t_centre_ellipse);
			sp->setrtpt({cpDev.x,cpDev.y});
			sp->setprimitive((*i).get());
			return sp;
		}
	}
	i=cmp.m_vPrims.crbegin();
	for(;i!=end;++i)
	{
	//	if(m_pSel==(*i).get())
		{
			af2d::rect<long> rDev;
			trns.ltod((*i)->getlogbbox(),rDev);

			rDev.offset(af2d::rect<long>::tl,{-(s_nSelBorderDim+s_nSelBorderGap),-(s_nSelBorderDim+s_nSelBorderGap)});
			rDev.offset(af2d::rect<long>::br,{(s_nSelBorderDim+s_nSelBorderGap),(s_nSelBorderDim+s_nSelBorderGap)});

			if(rDev.isinside(ptDev))
			{
				std::shared_ptr<hittest> sp(new hittest);
				sp->settype(hittest::t_aggregate_bbox);
				sp->setrtpt({cpDev.x,cpDev.y});
				sp->setprimitive((*i).get());
				return sp;
			}
		}
	}

	return nullptr;
}

void dibwnd::setcursor(void)const
{
	auto& cmp=theApp.m_spMorph->getcomponent(m_Type);
	switch(m_Mode)
	{
		case mt_feature:if(!cmp.m_rLog.isempty())SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));break;
		case mt_select:
		{
			if(!isdragging() && m_spHT)
				switch(m_spHT->gettype())
				{
					case hittest::t_centre_ellipse:
					case hittest::t_aggregate_bbox:SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));break;
				}
		}
		break;
		default:ASSERT(false);
	}
}

void dibwnd::drag(const CPoint& cpDev)
{
	push_back_drag({cpDev.x,cpDev.y});
}

void dibwnd::push_back_drag(const af2d::point<long>& p)
{
	if(!m_spDragPts || (!m_spDragPts->isempty() && m_spDragPts->get()[m_spDragPts->get().size()-1]==p))
		return;
	m_spDragPts->get().push_back(p);
	if(m_spDragPts->get().size()<2)
		Invalidate();
	else
	{
		const auto& prev = m_spDragPts->get()[m_spDragPts->get().size()-2];
		const CRect r1(prev.getx(),prev.gety(),prev.getx()+1,prev.gety()+1);
		const CRect r2(p.getx(),p.gety(),p.getx()+1,p.gety()+1);
		CRect r;
		r.UnionRect(r1,r2);
		r.InflateRect(s_nSelWidth*6,s_nSelWidth*6); // 3 line widths per side
		InvalidateRect(r);
	}
}

void dibwnd::composetransforms(std::shared_ptr<const Eigen::Affine2d>& spDevToLog,std::shared_ptr<const Eigen::Affine2d>& spLogToDev,const af2d::rect<>& rCanvas,const af2d::rect<>& rLog)const
{
	spDevToLog=nullptr;
	spLogToDev=nullptr;
	if(rLog.isempty() || rCanvas.isempty())
		return;
	double dS;

	std::shared_ptr<Eigen::Affine2d> sp=std::shared_ptr<Eigen::Affine2d>(new Eigen::Affine2d);

	af2d::rect<>::getrectscale(0,0,rLog.getwidth(),rLog.getheight(),0,0,rCanvas.getwidth()-(s_nDibBorderGap*2),rCanvas.getheight()-(s_nDibBorderGap*2),true,dS);
		
	*sp=Eigen::Translation<double,2>((rCanvas.getwidth()/2.0),(rCanvas.getheight()/2.0)) *
		Eigen::DiagonalMatrix<double,2>(dS,dS) *
		Eigen::Translation<double,2>(-(rLog.getwidth()/2.0),-(rLog.getheight()/2.0));
	
	spLogToDev=sp;

	sp=std::shared_ptr<Eigen::Affine2d>(new Eigen::Affine2d);
	*sp=spLogToDev->inverse();
	spDevToLog=sp;
}

void dibwnd::composebkgnd(const af2d::rect<>& r,std::shared_ptr<afdib::dib> spDst)const
{
	// assume rect clipped to canvas
	if(!spDst)
		return;

	bkgndinfo horzinfo,vertinfo;
	getbkgndinfo(r.get(af2d::rect<>::tl).getx(),r.get(af2d::rect<>::br).getx()-1,horzinfo);
	getbkgndinfo(r.get(af2d::rect<>::tl).gety(),r.get(af2d::rect<>::br).gety()-1,vertinfo);
	if(horzinfo.nPixels==0 || vertinfo.nPixels==0)
		return;

	for(int n=0;n<2 && n<vertinfo.nWholeChunks;++n)
		composebkgndrow(horzinfo,vertinfo,n+vertinfo.nChunkFrom,spDst);

	const int nScanlineBytes = spDst->getbytesperscanline();
	unsigned char *pScanline=spDst->getscanline(vertinfo.nChunkFrom*s_nChequerDim);
	for(int nChunk=2;nChunk<vertinfo.nWholeChunks;++nChunk)
		for(int n=0;n<s_nChequerDim;++n)
		{
			memcpy(pScanline+(3*s_nChequerDim*horzinfo.nChunkFrom)+(nScanlineBytes*s_nChequerDim*2),pScanline+(3*s_nChequerDim*horzinfo.nChunkFrom),(horzinfo.nWholeChunks*s_nChequerDim*3)+(3*horzinfo.nPartialChunkPixels));
			pScanline+=nScanlineBytes;
		}

	if(vertinfo.nPartialChunkPixels>0)
		composebkgndrow(horzinfo,vertinfo,vertinfo.nChunkFrom+vertinfo.nWholeChunks,spDst);
}

void dibwnd::composebkgndrow(const bkgndinfo& horzinfo,const bkgndinfo& vertinfo, const int nVertChunk,std::shared_ptr<afdib::dib> spDst)const
{
	// first scanline
	int nComposedChunks = 0;
	for(;nComposedChunks<2 && nComposedChunks<horzinfo.nWholeChunks;++nComposedChunks)
		setbkgndrowpixels({(horzinfo.nChunkFrom+nComposedChunks)*s_nChequerDim,nVertChunk*s_nChequerDim},s_nChequerDim,getbkgndchunkcolour(horzinfo.nChunkFrom+nComposedChunks,nVertChunk),spDst);	
	
	unsigned char *pScanline=spDst->getscanline(nVertChunk*s_nChequerDim);
	while(nComposedChunks<horzinfo.nWholeChunks)
	{
		const int nDstChunk = horzinfo.nChunkFrom + nComposedChunks;
		const int nSrcChunk = horzinfo.nChunkFrom;

		const int nAvailableChunks = (nDstChunk - nSrcChunk)%2?(nDstChunk - nSrcChunk - 1):(nDstChunk - nSrcChunk); // 2 * n
		const int nRemainingWholeChunks = horzinfo.nWholeChunks - nComposedChunks;
		const int nCopyChunks = nAvailableChunks > nRemainingWholeChunks ? nRemainingWholeChunks : nAvailableChunks;
		
		memcpy(pScanline+(3*s_nChequerDim*nDstChunk),pScanline+(3*s_nChequerDim*nSrcChunk),3*s_nChequerDim*nCopyChunks);

		nComposedChunks += nCopyChunks;
	}
	if(horzinfo.nPartialChunkPixels>0)
		setbkgndrowpixels({(horzinfo.nChunkFrom+nComposedChunks)*s_nChequerDim,nVertChunk*s_nChequerDim},horzinfo.nPartialChunkPixels,getbkgndchunkcolour(horzinfo.nChunkFrom+nComposedChunks,nVertChunk),spDst);

	// remaining scanlines
	const int nScanlineBytes = spDst->getbytesperscanline();
	const int nScanlines = (vertinfo.nChunkFrom+vertinfo.nWholeChunks==nVertChunk) ? vertinfo.nPartialChunkPixels : s_nChequerDim;
	for(int n=1;n<nScanlines;++n,pScanline+=nScanlineBytes)
		memcpy(pScanline+nScanlineBytes+(3*s_nChequerDim*horzinfo.nChunkFrom),pScanline+(3*s_nChequerDim*horzinfo.nChunkFrom),(horzinfo.nWholeChunks*s_nChequerDim*3)+(horzinfo.nPartialChunkPixels*3));
}

void dibwnd::setbkgndrowpixels(const af2d::point<int>& p,const int nPixels,const unsigned char *pPixel,std::shared_ptr<afdib::dib> spDst)const
{
	unsigned char *pScanline=spDst->getscanline(p.gety());
	pScanline += p.getx()*3;
	for(int n=0;n<nPixels;++n)
	{
		pScanline[n*3+0]=pPixel[0];
		pScanline[n*3+1]=pPixel[1];
		pScanline[n*3+2]=pPixel[2];
	}
}

void dibwnd::composedib(const Eigen::Affine2d *pLogToDev,const Eigen::Affine2d *pDevToLog,const af2d::rect<>& r,std::shared_ptr<const afdib::dib> spSrc,std::shared_ptr<afdib::dib> spDst)const
{
	// assume rect clipped to canvas
	if(!spDst || !spSrc || !pDevToLog || !pLogToDev)
		return;

	const auto rCompose=r.getintersect({{0,0},{spDst->getwidth(),spDst->getheight()}});
	if(rCompose.isempty())
		return;

	const int nSrcWidth=spSrc->getwidth();
	const int nSrcHeight=spSrc->getheight();
	const int nDstYFrom=rCompose.get(af2d::rect<>::tl).gety();
	const int nDstYInclusiveTo=rCompose.get(af2d::rect<>::br).gety()-1;
	const int nDstXFrom=rCompose.get(af2d::rect<>::tl).getx();
	const int nDstXInclusiveTo=rCompose.get(af2d::rect<>::br).getx()-1;
	
	unsigned char *pDstScanline=spDst->getscanline(nDstYFrom);
	const int nBytesPerScanline=spDst->getbytesperscanline();
	for(int nDstY=nDstYFrom;nDstY<=nDstYInclusiveTo;++nDstY,pDstScanline+=nBytesPerScanline)
	{
		double dSrcX, dSrcY;
		afmorph::xform<>::sxtx_y(pDevToLog,nDstY+0.5,dSrcY);

		if(dSrcY<0)
			continue;
		const int nSrcY = af::posfloor<double,int>(dSrcY);
		if(nSrcY>=nSrcHeight)
			continue;

		const unsigned char *pSrcScanline=spSrc->getscanline(nSrcY);

		for(int nDstX=nDstXFrom;nDstX<=nDstXInclusiveTo;++nDstX)
		{
			afmorph::xform<>::sxtx_x(pDevToLog,nDstX+0.5,dSrcX);

			if(dSrcX<0)
				continue;
			const int nSrcX = af::posfloor<double,int>(dSrcX);
			if(nSrcX>=nSrcWidth)
				continue;
		
			pDstScanline[nDstX*3+0]=pSrcScanline[nSrcX*3+0];
			pDstScanline[nDstX*3+1]=pSrcScanline[nSrcX*3+1];
			pDstScanline[nDstX*3+2]=pSrcScanline[nSrcX*3+2];
		}
	}
}

void dibwnd::getbkgndinfo(const int nFrom,const int nInclusiveTo,bkgndinfo& i)const
{
	i.nPixels = nInclusiveTo - nFrom + 1;
	i.nPixelFrom = nFrom;
	i.nChunkFrom = i.nPixelFrom / s_nChequerDim;
	i.nWholeChunks = i.nPixels / s_nChequerDim;
	i.nPartialChunkPixels = (nInclusiveTo+1) - ((i.nChunkFrom+i.nWholeChunks)*s_nChequerDim);
}

const unsigned char *dibwnd::getbkgndchunkcolour(const int nChunkX,const int nChunkY)const
{
	if(nChunkX % 2)
	{
		if(nChunkY % 2)
			return s_cWhite;
		return s_cGrey;
	}
	if(nChunkY % 2)
		return s_cGrey;
	return s_cWhite;
}

af2d::point<int> dibwnd::getclientcursorpos( void ) const
{
	POINT p;
	::GetCursorPos( &p );
	::MapWindowPoints( NULL, GetSafeHwnd(), &p, 1 );
	return {p.x,p.y};
}

bool dibwnd::captureinput( void )
{
	if( GetSafeHwnd() != ::GetCapture() )
		::SetCapture( GetSafeHwnd() );
	return true;
}

bool dibwnd::releasecapturedinput( void )
{
	if( GetSafeHwnd() == ::GetCapture() )
		::ReleaseCapture();
	return true;
}

void dibwnd::getht(void)
{
	CPoint cp;
	::GetCursorPos(&cp);
	::MapWindowPoints(NULL,m_hWnd,&cp,1);
	m_spHT=getht(cp);
}

void dibwnd::process(const hint& h)
{
	auto& cmp=theApp.m_spMorph->getcomponent(m_Type);
	auto& othercmp=theApp.m_spMorph->getcomponent(m_Type,true);

	switch(h.m_Type)
	{
		case hint::t_new:
		case hint::t_load:
		case hint::t_dib:
		case hint::t_frame:
		case hint::t_frames:
		{
			if(h.m_Type==hint::t_load || h.m_Type==hint::t_new || h.m_Type==hint::t_dib || m_Type==afmorph::morphcomponent::t_lerp)
			{
				af2d::rect<> rCanvas;
				if(m_spCanvas)
					rCanvas={{0,0},{m_spCanvas->getwidth(),m_spCanvas->getheight()}};
				composetransforms(m_spDevToLog,cmp.m_spLogToDev,rCanvas,cmp.m_rLog);
				
				cmp.triangulatedev(afmorph::morphcomponent::lt_all);

				if(m_spCanvas)
				{
					composebkgnd(rCanvas,m_spCanvas);
					if(m_spDevToLog && cmp.m_spLogToDev)
						composedib(cmp.m_spLogToDev.get(),m_spDevToLog.get(),rCanvas,cmp.m_spDib,m_spCanvas);
				}

				getht();
				setcursor();

				Invalidate();
			}
		}
		break;
		case hint::t_link:
		case hint::t_triangulatebkgnd:
		{
			if(m_Type==afmorph::morphcomponent::t_lerp)
			{
				if(m_spCanvas)
					composebkgnd({{0,0},{m_spCanvas->getwidth(),m_spCanvas->getheight()}},m_spCanvas);
			}

			Invalidate();
		}
		break;
		case hint::t_renderflag:Invalidate();break;
		case hint::t_select:if(m_Type & h.m_nComponents)Invalidate();break;
		case hint::t_erase:
		{
			if(m_Type & h.m_nComponents)
			{
				getht();
				setcursor();
			}
			if(m_Type==afmorph::morphcomponent::t_lerp)
			{
				if(m_spCanvas)
					composebkgnd({{0,0},{m_spCanvas->getwidth(),m_spCanvas->getheight()}},m_spCanvas);
			}
			Invalidate();
		}
		break;
		case hint::t_pushback:
		{
			if(m_Type & h.m_nComponents)
			{
				getht();
				setcursor();

				Invalidate();
			}
		}
		break;
		default:ASSERT(false);
	}
}
