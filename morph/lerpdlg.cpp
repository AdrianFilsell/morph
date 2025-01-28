// animdlg.cpp : implementation file
//

#include "pch.h"
#include "afxdialogex.h"
#include "lerpdlg.h"
#include "morph.h"
#include "morphDlg.h"

// lerpdlg dialog

IMPLEMENT_DYNAMIC(lerpdlg, CDialogEx)

lerpdlg::lerpdlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD, pParent)
{
	m_bInitialised=false;
}

lerpdlg::~lerpdlg()
{
}

void lerpdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX,IDC_FRAME_SLIDER,m_Slider);

	DDX_Text(pDX,IDC_FRAME_EDIT,theApp.m_spMorph->m_nLerpFrame);
	DDX_Text(pDX,IDC_FRAME_TOTAL_EDIT,theApp.m_spMorph->m_nLerpFrames);
}


BEGIN_MESSAGE_MAP(lerpdlg, CDialogEx)
	ON_WM_HSCROLL()

	ON_EN_CHANGE(IDC_FRAME_EDIT,OnFrameTextChange)
	ON_EN_CHANGE(IDC_FRAME_TOTAL_EDIT,OnTotalTextChange)

	ON_EN_KILLFOCUS(IDC_FRAME_EDIT,OnFrameTextKillFocus)
	ON_EN_KILLFOCUS(IDC_FRAME_TOTAL_EDIT,OnTotalTextKillFocus)
END_MESSAGE_MAP()


// lerpdlg message handlers

BOOL lerpdlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CRect rcDibWnd;
	GetDlgItem(IDC_IMAGE)->GetWindowRect(rcDibWnd);
	::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rcDibWnd,2);

	std::shared_ptr<dibwnd> spPane = std::shared_ptr<dibwnd>(new dibwnd(afmorph::morphcomponent::t_lerp));
	spPane->Create(AfxRegisterWndClass(0,theApp.LoadStandardCursor(IDC_ARROW),::GetSysColorBrush(COLOR_WINDOW)),_T("dib"),WS_VISIBLE|WS_CHILD,rcDibWnd,this,AFX_IDW_PANE_FIRST);
	::SetWindowLongPtr(*spPane, GWL_EXSTYLE, GetWindowLongPtr(*spPane, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);
	::SetWindowPos(*spPane, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

	m_spDibWnd=spPane;

	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_FRAME_SPIN))->SetRange32(1,theApp.m_spMorph->m_nLerpFrames);
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_FRAME_TOTAL_SPIN))->SetRange32(1,0x7fffffff);
	m_Slider.SetRange(1,theApp.m_spMorph->m_nLerpFrames);
	m_Slider.SetPos(theApp.m_spMorph->m_nLerpFrame);

	enabledisable();

	m_bInitialised=true;
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void lerpdlg::OnFrameTextChange(void)
{
	if(!m_bInitialised)
		return;

	CString cs;
	GetDlgItem(IDC_FRAME_EDIT)->GetWindowText( cs );
	int nNewPos = _tstoi( cs );
	if( nNewPos < 1 )
		nNewPos = 1;
	if( nNewPos > theApp.m_spMorph->m_nLerpFrames )
		nNewPos = theApp.m_spMorph->m_nLerpFrames;

	if(theApp.m_spMorph->m_nLerpFrame!=nNewPos)
	{
		if(m_Slider.GetSafeHwnd())
			m_Slider.SetPos( nNewPos );

		theApp.m_spMorph->m_nLerpFrame=nNewPos;

		auto& lerpcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_lerp);
		lerpcmp.triangulate();

		theApp.broadcast(hint(hint::t_frame));			
	}
}

void lerpdlg::OnTotalTextChange(void)
{
	if(!m_bInitialised)
		return;

	CString cs;
	GetDlgItem(IDC_FRAME_TOTAL_EDIT)->GetWindowText( cs );
	int nNewPos = _tstoi( cs );
	if( nNewPos < 1 )
		nNewPos = 1;
	if( nNewPos > 0x7fffffff )
		nNewPos = 0x7fffffff;

	if(theApp.m_spMorph->m_nLerpFrames!=nNewPos)
	{
		theApp.m_spMorph->m_nLerpFrames=nNewPos;

		const bool bInvalidFrame=theApp.m_spMorph->m_nLerpFrame>nNewPos;
		if(bInvalidFrame)
		{
			theApp.m_spMorph->m_nLerpFrame=nNewPos;

			static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_FRAME_SPIN))->SetPos(nNewPos);
		}

		static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_FRAME_SPIN))->SetRange32(1,nNewPos);
		m_Slider.SetRange(1,nNewPos,true);

		auto& lerpcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_lerp);
		lerpcmp.triangulate();

		theApp.broadcast(hint(hint::t_frames));
	}
}

void lerpdlg::OnFrameTextKillFocus()
{
	CString cs;
	GetDlgItem(IDC_FRAME_EDIT)->GetWindowText( cs );
	int nNewPos = _tstoi( cs );
	if( nNewPos < 1 )
		nNewPos = 1;
	if( nNewPos > theApp.m_spMorph->m_nLerpFrames )
		nNewPos = theApp.m_spMorph->m_nLerpFrames;

	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_FRAME_SPIN))->SetPos( nNewPos ); // this call text changed code
}

void lerpdlg::OnTotalTextKillFocus()
{
	CString cs;
	GetDlgItem(IDC_FRAME_TOTAL_EDIT)->GetWindowText( cs );
	int nNewPos = _tstoi( cs );
	if( nNewPos < 1 )
		nNewPos = 1;
	if( nNewPos > 0x7fffffff )
		nNewPos = 0x7fffffff;

	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_FRAME_TOTAL_SPIN))->SetPos( nNewPos ); // this call text changed code
}

void lerpdlg::OnHScroll(UINT nSBCode,UINT nPos,CScrollBar* pScrollBar)
{
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);

	// See if there is anything todo
	if( nSBCode == TB_ENDTRACK )
		return;

	// Update members
	CSliderCtrl *pSlider = reinterpret_cast< CSliderCtrl* >( pScrollBar );
	int nID = pSlider->GetDlgCtrlID();	
	switch( nID )
	{
		case IDC_FRAME_SLIDER:
		{
			theApp.m_spMorph->m_nLerpFrame = pSlider->GetPos();
			static_cast< CSpinButtonCtrl* >( GetDlgItem( IDC_FRAME_SPIN ) )->SetPos( theApp.m_spMorph->m_nLerpFrame );

			auto& lerpcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_lerp);
			lerpcmp.triangulate();

			theApp.broadcast(hint(hint::t_frame));
		}
		break;
		default: ASSERT( FALSE );return;
	}
	UpdateData( FALSE );
}

void lerpdlg::enabledisable(void)
{
	auto& srccmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_src);
	auto& dstcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_dst);

	const bool bCanLerp=(srccmp.m_vLogTriangulated.size()) &&
						(dstcmp.m_vLogTriangulated.size());
	
	GetDlgItem(IDC_FRAME_TOTAL_EDIT)->EnableWindow(bCanLerp);
	GetDlgItem(IDC_FRAME_SLIDER)->EnableWindow(bCanLerp);
	GetDlgItem(IDC_FRAME_EDIT)->EnableWindow(bCanLerp);
}

void lerpdlg::process(const hint& h)
{
	m_spDibWnd->process(h);

	switch(h.m_Type)
	{
		case hint::t_pushback:
		case hint::t_erase:
		case hint::t_link:
		case hint::t_dib:
		case hint::t_triangulatebkgnd:enabledisable();break;
		case hint::t_frame:
		case hint::t_frames:
		case hint::t_select:
		case hint::t_renderflag:break;
		case hint::t_new:
		{
			theApp.m_spMorph->m_nLerpFrame=1;
			theApp.m_spMorph->m_nLerpFrames=100;

			static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_FRAME_SPIN))->SetRange32(1,theApp.m_spMorph->m_nLerpFrames);
			m_Slider.SetRange(1,theApp.m_spMorph->m_nLerpFrames);
			m_Slider.SetPos(theApp.m_spMorph->m_nLerpFrame);
		
			UpdateData(false);
		
			enabledisable();
		}
		break;
		case hint::t_load:
		{
			static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_FRAME_SPIN))->SetRange32(1,theApp.m_spMorph->m_nLerpFrames);
			m_Slider.SetRange(1,theApp.m_spMorph->m_nLerpFrames);
			m_Slider.SetPos(theApp.m_spMorph->m_nLerpFrame);
		
			UpdateData(false);

			enabledisable();
		}
		break;
		default:ASSERT(false);
	}
}
