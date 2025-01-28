#pragma once

#include "hittest.h"
#include "morph.h"
#include <memory>

// dibwnd

class dibwnd : public CWnd
{
	DECLARE_DYNAMIC(dibwnd)

public:
	enum modetype {mt_select,mt_feature};

	dibwnd(const afmorph::morphcomponent::type t);
	virtual ~dibwnd();

	bool isdragging(void)const{return !!m_spDragPts.get();}
	af2d::point<int> getclientcursorpos(void)const;
	modetype getmode(void)const{return m_Mode;}
	
	void canceldrag(void);
	void setmode(const modetype m){m_Mode=m;}

	void process(const hint& h);

	bool captureinput(void);
	bool releasecapturedinput(void);

	static void renderlerp(unsigned char *pCanvas,const int nCanvasWidth,const int nCanvasHeight,const int nCanvasBytesPerScanline,const int nCanvasBytesPerPixel,const CRect& rcClip,const Eigen::Affine2d *pDevToLog,const Eigen::Affine2d *pLogToDev);
protected:
	enum lbuttonwaittype { wt_lbuttonup,wt_dragmove,wt_null };

	afmorph::morphcomponent::type m_Type;
	CBitmap m_OffscreenBmp;
	std::shared_ptr<afdib::dib> m_spCanvas;
	std::shared_ptr<hittest> m_spHT;
	static const int s_nChequerDim;
	static const int s_nDibBorderGap;
	static const int s_nHandleDim;
	static const int s_nPathWidth;
	static const int s_nConnWidth;
	static const int s_nCentreWidth;
	static const int s_nTriWidth;
	static const int s_nSelWidth;
	static const COLORREF s_cTri;
	static const COLORREF s_cPath;
	static const COLORREF s_cCentrePath;
	static const COLORREF s_cSamplePath;
	static const COLORREF s_cConn;
	static const COLORREF s_cBorder;
	static const COLORREF s_cSelBorder;
	static const int s_nSelBorderDim;
	static const int s_nSelBorderGap;
	static const MOR_FLTTYPE s_dNormDim;
	bool m_bCaptured;
	std::shared_ptr<af2d::pointvec<long>> m_spDragPts;

	modetype m_Mode; // no real need for tools just use a mode

	std::shared_ptr<const Eigen::Affine2d> m_spDevToLog;
	
	void render(CDC *pDC,const CRect& rcClip)const;
	void renderborder(CDC *pDC,const af2d::rect<long>& r,const COLORREF cr)const;
	
	std::shared_ptr<hittest> getht(const CPoint& cpDev)const;
	void setcursor(void)const;
	void setcapture(void);
	void releasecapture(void);
	void drag(const CPoint& cpDev);

	void getht(void);

	afx_msg void OnPaint(void);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	struct bkgndinfo
	{
		int nPixelFrom;
		int nChunkFrom;
		int nWholeChunks;
		int nPartialChunkPixels;
		int nPixels;
	};

	static const unsigned char s_cWhite[3];
	static const unsigned char s_cGrey[3];
	void composebkgnd(const af2d::rect<>& r,std::shared_ptr<afdib::dib> spDst)const;
	void composebkgndrow(const bkgndinfo& horzinfo,const bkgndinfo& vertinfo, const int nChunk,std::shared_ptr<afdib::dib> spDst)const;
	void setbkgndrowpixels(const af2d::point<int>& p,const int nPixels,const unsigned char *pPixel,std::shared_ptr<afdib::dib> spDst)const;
	void getbkgndinfo(const int nFrom,const int nInclusiveTo,bkgndinfo& i)const;
	const unsigned char *getbkgndchunkcolour(const int nChunkX,const int nChunkY)const;
	void composedib(const Eigen::Affine2d *pLogToDev,const Eigen::Affine2d *pDevToLog,const af2d::rect<>& r,std::shared_ptr<const afdib::dib> spSrc,std::shared_ptr<afdib::dib> spDst)const;
	
	void composetransforms(std::shared_ptr<const Eigen::Affine2d>& spDevToLog,std::shared_ptr<const Eigen::Affine2d>& spLogToDev,const af2d::rect<>& rCanvas,const af2d::rect<>& rLog)const;

	void push_back_drag(const af2d::point<long>& p);
		
	static bool beindragmovedelta( const af2d::point<int>& from, const af2d::point<int>& to, const int nDragMoveThreshold )
	{
		const std::pair<af2d::point<int>,bool> minmove = {{nDragMoveThreshold,nDragMoveThreshold},false};
		
		const int nDeltaX = ( from.getx() - to.getx() ) > 0 ? ( from.getx() - to.getx() ) : -( from.getx() - to.getx() );
		const int nDeltaY = ( from.gety() - to.gety() ) > 0 ? ( from.gety() - to.gety() ) : -( from.gety() - to.gety() );
	
		if( minmove.second )
		{
			if( nDeltaX < minmove.first.getx() || nDeltaY < minmove.first.gety() )
				return false;
			return true;
		}
		else
		{
			if( nDeltaX < minmove.first.getx() && nDeltaY < minmove.first.gety() )
				return false;
			return true;
		}
	}
	static lbuttonwaittype msgwait( const HWND hWnd, const af2d::point<int>& ptClientFrom, const int nTimeOut, const int nDragMoveThreshold )
	{
		const std::vector<int> vMsgs = { WM_LBUTTONUP, WM_MOUSEMOVE };

		MSG m;
		auto d=GetTickCount64();
		while( true )
		{
			if( GetTickCount64()-d >= nTimeOut )
				return wt_null;
			for( const auto &i : vMsgs )
			{
				// During this call, the system delivers pending, nonqueued messages, that is, messages sent to windows owned by the calling thread using the
				// SendMessage, SendMessageCallback, SendMessageTimeout, or SendNotifyMessage function. 
				if( !::PeekMessage( &m, hWnd, i, i, PM_NOYIELD|PM_NOREMOVE ) )
					continue;
				if( m.message == vMsgs[0] )
					return wt_lbuttonup;
				if( m.message == vMsgs[1] )
				{
					const int xPos = GET_X_LPARAM(m.lParam);
					const int yPos = GET_Y_LPARAM(m.lParam);
					if( !beindragmovedelta( ptClientFrom, af2d::point<int>( xPos, yPos ), nDragMoveThreshold ) )
						continue;
					return wt_dragmove;
				}
			}
		}
	}
	static lbuttonwaittype lbuttondownwait(CWnd *p,const af2d::point<int>& rtpt,const int nDragMoveThreshold)
	{
		if(!p)
			return wt_null;
		return msgwait(p->GetSafeHwnd(),rtpt,200,nDragMoveThreshold);
	}
	static bool clickpending( const lbuttonwaittype wt ) { return wt == wt_lbuttonup; }
	static bool dragpending( const lbuttonwaittype wt ) { return wt == wt_dragmove; }
};
