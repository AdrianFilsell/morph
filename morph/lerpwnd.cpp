// lerpwnd.cpp : implementation file
//

#include "pch.h"
#include "lerpwnd.h"
#include "morph.h"


// animwnd

IMPLEMENT_DYNAMIC(lerpwnd, CWnd)

lerpwnd::lerpwnd()
{

}

lerpwnd::~lerpwnd()
{
}


BEGIN_MESSAGE_MAP(lerpwnd, CWnd)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// lerpwnd message handlers


void lerpwnd::OnClose()
{
	theApp.closelerpwnd();
}

int lerpwnd::OnCreate(LPCREATESTRUCT p)
{
	const int n = CWnd::OnCreate(p);
	
	m_spDlg=std::shared_ptr<lerpdlg>(new lerpdlg);
	m_spDlg->Create(lerpdlg::IDD,this);

	return n;
}

void lerpwnd::OnSize(UINT nType,int cx,int cy)
{
	CWnd::OnSize(nType,cx,cy);

	if(false && m_spDlg && m_spDlg->GetSafeHwnd())
	{
		CRect rcDlg;
		GetClientRect(rcDlg);
		::SetWindowPos(m_spDlg ->GetSafeHwnd(),NULL,rcDlg.left,rcDlg.top,rcDlg.Width(),rcDlg.Height(),SWP_DRAWFRAME|SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOZORDER);
	}
}

BOOL lerpwnd::OnEraseBkgnd(CDC *pDC)
{
	return TRUE;
}
