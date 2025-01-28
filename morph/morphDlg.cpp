
// morphDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "morph.h"
#include "morphDlg.h"
#include "afxdialogex.h"
#include "jpeg.h"
#include "lerpwnd.h"
#include "serialise.h"
#include "exportgifdlg.h"
#include <set>

std::function<std::shared_ptr<CImageList>(CToolBar *pTB,const UINT nRes)> set32bppIL=[](CToolBar *pTB,const UINT nRes) -> std::shared_ptr<CImageList>
{
	if(!pTB)return nullptr;
	HBITMAP hBitmap = (HBITMAP)::LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(nRes),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_LOADTRANSPARENT);
	if(hBitmap==NULL) return nullptr;

	std::shared_ptr<CImageList> sp;
	sp=std::shared_ptr<CImageList>(new CImageList);
	sp->Create(16,15,ILC_COLOR32|ILC_MASK,0,1);
	const int n = sp->Add(CBitmap::FromHandle(hBitmap),COLORREF(0));
	::DeleteObject(hBitmap);

	CImageList* pOldIL=pTB->GetToolBarCtrl().SetImageList(sp.get());
	return sp;
};

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CmorphDlg dialog



CmorphDlg::CmorphDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MORPH_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bInitialised=false;
}

void CmorphDlg::DoDataExchange(CDataExchange* pDX)
{
	auto& srccmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_src);
	auto& dstcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_dst);

	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX,IDC_SRC_PATH,srccmp.m_csPath);
	DDX_Text(pDX,IDC_DST_PATH,dstcmp.m_csPath);
	DDX_Control(pDX,IDC_SRC_SELECTION,m_SrcSel);
	DDX_Control(pDX,IDC_DST_SELECTION,m_DstSel);
	CComboBox m_DstSel;
}

BEGIN_MESSAGE_MAP(CmorphDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_MESSAGE(WM_KICKIDLE,OnKickIdle)

	ON_BN_CLICKED(IDC_SRC_BROWSE,CmorphDlg::OnSrcBrowse)
	ON_BN_CLICKED(IDC_DST_BROWSE,CmorphDlg::OnDstBrowse)

	ON_BN_CLICKED(IDC_NEW,CmorphDlg::OnNew)
	ON_BN_CLICKED(IDC_LOAD,CmorphDlg::OnLoad)
	ON_BN_CLICKED(IDC_SAVE,CmorphDlg::OnSave)

	ON_BN_CLICKED(IDC_SELECT_MODE,CmorphDlg::OnSelectMode)
	ON_UPDATE_COMMAND_UI(IDC_SELECT_MODE,CmorphDlg::OnUpdateSelectMode)
	ON_BN_CLICKED(IDC_FEATURE_MODE,CmorphDlg::OnFeatureMode)
	ON_UPDATE_COMMAND_UI(IDC_FEATURE_MODE,CmorphDlg::OnUpdateFeatureMode)

	ON_BN_CLICKED(IDC_LINK_SELECTION,OnLinkSelection)
	ON_UPDATE_COMMAND_UI(IDC_LINK_SELECTION,OnUpdateLinkSelection)

	ON_BN_CLICKED(IDC_ERASE_ALL,OnEraseAll)
	ON_UPDATE_COMMAND_UI(IDC_ERASE_ALL,OnUpdateEraseAll)
	ON_BN_CLICKED(IDC_ERASE_SELECTION,OnEraseSelection)
	ON_UPDATE_COMMAND_UI(IDC_ERASE_SELECTION,OnUpdateEraseSelection)

	ON_BN_CLICKED(IDC_ANIML_PANEL,OnAnimPanel)
	ON_UPDATE_COMMAND_UI(IDC_ANIML_PANEL,OnUpdateAnimPanel)
	ON_BN_CLICKED(IDC_ANIML_GIF,OnAnimGIF)
	ON_UPDATE_COMMAND_UI(IDC_ANIML_GIF,OnUpdateAnimGIF)

	ON_BN_CLICKED(IDC_SAMPLE_RENDER,OnSampleRender)
	ON_UPDATE_COMMAND_UI(IDC_SAMPLE_RENDER,OnUpdateSampleRender)
	ON_BN_CLICKED(IDC_TRIANGULATE_RENDER,OnTriangulateRender)
	ON_UPDATE_COMMAND_UI(IDC_TRIANGULATE_RENDER,OnUpdateTriangulateRender)
	ON_BN_CLICKED(IDC_PATH_RENDER,OnPathRender)
	ON_UPDATE_COMMAND_UI(IDC_PATH_RENDER,OnUpdatePathRender)
	ON_BN_CLICKED(IDC_LINK_RENDER,OnLinkRender)
	ON_UPDATE_COMMAND_UI(IDC_LINK_RENDER,OnUpdateLinkRender)

	ON_BN_CLICKED(IDC_TRIANGULATE_BKGND,OnTriangulateBkgnd)
	ON_UPDATE_COMMAND_UI(IDC_TRIANGULATE_BKGND,OnUpdateTriangulateBkgnd)

	ON_CBN_SELCHANGE(IDC_SRC_SELECTION,OnSrcSelChanged)
	ON_CBN_SELCHANGE(IDC_DST_SELECTION,OnDstSelChanged)
END_MESSAGE_MAP()


// CmorphDlg message handlers

BOOL CmorphDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Create the toolbar
    if (!m_wndToolBar.CreateEx(this, /*TBSTYLE_FLAT, */WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS ) ||
        !m_wndToolBar.LoadToolBar(IDR_TOOLBAR))  // Replace IDR_TOOLBAR1 with your toolbar resource ID
    {
        TRACE("Failed to create toolbar\n");
        return FALSE;
    }
	m_spToolBarIL=set32bppIL(&m_wndToolBar,IDR_TOOLBAR_32BPP);

	m_SrcSel.AddString(afmorph::morphcomponent::s_szNull);
	m_DstSel.AddString(afmorph::morphcomponent::s_szNull);
	m_SrcSel.SetCurSel(0);
	m_DstSel.SetCurSel(0);


    // Adjust the toolbar's position to replace the static control
	const auto rTB=getrect(IDC_TOOLBAR);
    m_wndToolBar.SetWindowPos(
        nullptr,
        rTB.get(af2d::rect<>::tl).getx(),
        rTB.get(af2d::rect<>::tl).gety(),
        rTB.get(af2d::rect<>::br).getx()-rTB.get(af2d::rect<>::tl).getx(),
		rTB.get(af2d::rect<>::br).gety()-rTB.get(af2d::rect<>::tl).gety(),
        SWP_DRAWFRAME|SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOZORDER
    );

	// TODO: Add extra initialization here
	CRect rcClientRect;
	GetClientRect(rcClientRect);
	m_rCtrlRectMapClient={{rcClientRect.left,rcClientRect.top},{rcClientRect.right,rcClientRect.bottom}};
	m_CtrlRectMap[IDC_PATH_GRP]=getrect(IDC_PATH_GRP);
	m_CtrlRectMap[IDC_SRC_PATH]=getrect(IDC_SRC_PATH);
	m_CtrlRectMap[IDC_DST_PATH]=getrect(IDC_DST_PATH);
	m_CtrlRectMap[IDC_SRC_BROWSE]=getrect(IDC_SRC_BROWSE);
	m_CtrlRectMap[IDC_DST_BROWSE]=getrect(IDC_DST_BROWSE);

	m_CtrlRectMap[IDC_FEATURES_GRP]=getrect(IDC_FEATURES_GRP);
	m_CtrlRectMap[IDC_SPLITTER]=getrect(IDC_SPLITTER);

	m_CtrlRectMap[IDC_TOOLBAR]=getrect(IDC_TOOLBAR);
	m_CtrlRectMap[IDOK]=getrect(IDOK);
	
	m_CtrlRectMap[IDC_SELECTION_STATIC]=getrect(IDC_SELECTION_STATIC);
	m_CtrlRectMap[IDC_SRC_SELECTION]=getrect(IDC_SRC_SELECTION);
	m_CtrlRectMap[IDC_SELECTION_SEPARATOR]=getrect(IDC_SELECTION_SEPARATOR);
	m_CtrlRectMap[IDC_DST_SELECTION]=getrect(IDC_DST_SELECTION);

	m_spSplitter=std::shared_ptr<splitterwnd>(new splitterwnd);
	m_spSplitter->create(this);

	updatelayout();
	
	CRect rcClient;
	m_spSplitter->GetClientRect(rcClient);

	const int nIdeal = rcClient.Width()/2, nMin = 100;
	m_spSplitter->SetColumnInfo(0,nIdeal,nMin);
	m_spSplitter->SetColumnInfo(1,nIdeal,nMin);

	m_spSplitter->SetActivePane(0,0,m_spSplitter->getpanes()[0].get());
	m_spSplitter->RecalcLayout();

	enabledisable();

	m_bInitialised=true;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

class CToolCmdUIEx : public CCmdUI
{
public:
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetText(LPCTSTR lpszText) {/*todo*/}
	virtual void SetRadio(BOOL bOn = TRUE) {/*todo*/}
};

void CToolCmdUIEx::Enable(BOOL bOn)
{
	m_bEnableChanged = TRUE;
	CToolBar* pToolBar = (CToolBar*)m_pOther;

	UINT nNewStyle = pToolBar->GetButtonStyle(m_nIndex) & ~TBBS_DISABLED;

	if (!bOn)
		nNewStyle |= TBBS_DISABLED;
	ASSERT(!(nNewStyle & TBBS_SEPARATOR));
	pToolBar->SetButtonStyle(m_nIndex, nNewStyle);
}

void CToolCmdUIEx::SetCheck(int nCheck)
{
	ASSERT(nCheck >= 0);
	if (nCheck > 2)
	{
		nCheck = 1;
	}

	CToolBar* pToolBar = (CToolBar*)m_pOther;

	UINT nNewStyle = pToolBar->GetButtonStyle(m_nIndex) & ~(TBBS_CHECKED | TBBS_INDETERMINATE);
	if (nCheck == 1)
		nNewStyle |= TBBS_CHECKED;
	else if (nCheck == 2)
		nNewStyle |= TBBS_INDETERMINATE;
	ASSERT(!(nNewStyle & TBBS_SEPARATOR));
	pToolBar->SetButtonStyle(m_nIndex, nNewStyle | TBBS_CHECKBOX);
}

LPARAM CmorphDlg::OnKickIdle(WPARAM,LPARAM)
{
	// Iterate through all toolbar buttons
	for (int i = 0; i < m_wndToolBar.GetCount(); ++i)
	{
		// Get the command ID of the button
		UINT nID = m_wndToolBar.GetItemID(i);
		if (nID == 0 || nID == (UINT)-1)  // Skip separators or invalid buttons
			continue;

		// Create a CCmdUI object for this button
		CToolCmdUIEx cmdUI;
		cmdUI.m_nID = nID;           // Command ID of the toolbar button
		cmdUI.m_pOther = &m_wndToolBar; // The toolbar control
		cmdUI.m_pMenu = nullptr;     // No menu involved here
		cmdUI.m_nIndexMax=m_wndToolBar.GetCount()-1;
		
		// Find the button by index
		cmdUI.m_nIndex = i;
		cmdUI.m_pSubMenu = nullptr;

		// Call DoUpdate to invoke the ON_UPDATE_COMMAND_UI handler
		cmdUI.DoUpdate(this, FALSE);
	}

	return 0; // FALSE
}

void CmorphDlg::OnCancel(void)
{
	switch(m_spSplitter->getpanes()[0]->getmode()) // any will do
	{
		case dibwnd::mt_feature:
		{
			if(m_spSplitter->getpanes()[0]->isdragging())
			{
				m_spSplitter->getpanes()[0]->canceldrag();
				return;
			}
			if(m_spSplitter->getpanes()[1]->isdragging())
			{
				m_spSplitter->getpanes()[1]->canceldrag();
				return;
			}
		}
		break;
	}

	CDialogEx::OnCancel();
}

void CmorphDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CmorphDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CmorphDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CmorphDlg::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
	// Call the base class
	lpMMI->ptMinTrackSize.x = 600;
	lpMMI->ptMinTrackSize.y = 500;
	CDialogEx::OnGetMinMaxInfo( lpMMI );
}

void CmorphDlg::OnSize(UINT nType, int cx, int cy)
{
	// Call the base class
	CDialogEx::OnSize(nType, cx, cy);

	// Update the layout
	updatelayout();
}

void CmorphDlg::OnSrcBrowse()
{
	if(!browse(afmorph::morphcomponent::t_src))
		return;
	theApp.broadcast(hint(hint::t_dib,afmorph::morphcomponent::t_src));
}

void CmorphDlg::OnDstBrowse()
{
	if(!browse(afmorph::morphcomponent::t_dst))
		return;
	theApp.broadcast(hint(hint::t_dib,afmorph::morphcomponent::t_dst));
}

void CmorphDlg::OnNew()
{
	auto& srccmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_src);
	auto& dstcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_dst);
	auto& lerpcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_lerp);

	srccmp.clear();
	dstcmp.clear();
	lerpcmp.clear();
	
	theApp.broadcast(hint(hint::t_new,afmorph::morphcomponent::t_src|afmorph::morphcomponent::t_dst));
}

void CmorphDlg::OnLoad()
{
	// load
	CFileDialog dlg(true);
	if(dlg.DoModal()!=IDOK)
		return;

	std::shared_ptr<afmorph::morph> spM;
	
	serialise s;
	if(!s.read(dlg.GetPathName(),spM))
	{
		CString csMsg;
		csMsg.Format(_T("Could not load: \"%s\""),dlg.GetPathName());
		AfxMessageBox(csMsg,MB_OK);
		return;
	}
	if(!spM->validateread(dlg.GetPathName()))
		return;
	
	auto& srccmp=spM->getcomponent(afmorph::morphcomponent::t_src);
	auto& dstcmp=spM->getcomponent(afmorph::morphcomponent::t_dst);
	auto& lerpcmp=spM->getcomponent(afmorph::morphcomponent::t_lerp);
	
	srccmp.triangulate();
	dstcmp.triangulate();
	lerpcmp.triangulate();
	
	theApp.m_spMorph=spM;

	theApp.broadcast(hint(hint::t_load,afmorph::morphcomponent::t_src|afmorph::morphcomponent::t_dst));
}

void CmorphDlg::OnSave()
{
	CFileDialog dlg(false);
	if(dlg.DoModal()!=IDOK)
		return;

	serialise s;
	const bool bSuccess = s.write(dlg.GetPathName(),theApp.m_spMorph.get());
}

void CmorphDlg::OnSelectMode()
{
	m_spSplitter->getpanes()[0]->setmode(dibwnd::mt_select);
	m_spSplitter->getpanes()[1]->setmode(dibwnd::mt_select);
}

void CmorphDlg::OnUpdateSelectMode(CCmdUI *pCmdUI)
{
	// both dib wnd will be in same mode/tool so use any
	dibwnd *pActive=m_spSplitter->getpanes()[0].get();
	if(!pActive)
		return;
	pCmdUI->SetCheck(pActive->getmode()==dibwnd::mt_select?1:0);
}

void CmorphDlg::OnFeatureMode()
{
	m_spSplitter->getpanes()[0]->setmode(dibwnd::mt_feature);
	m_spSplitter->getpanes()[1]->setmode(dibwnd::mt_feature);
}

void CmorphDlg::OnUpdateFeatureMode(CCmdUI *pCmdUI)
{
	// both dib wnd will be in same mode/tool so use any
	dibwnd *pActive=m_spSplitter->getpanes()[0].get();
	if(!pActive)
		return;
	pCmdUI->SetCheck(pActive->getmode()==dibwnd::mt_feature?1:0);
}

void CmorphDlg::OnLinkSelection()
{
	auto& srccmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_src);
	auto& dstcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_dst);
	auto& lerpcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_lerp);

	bool bReset0=false,bReset1=false,bJoin=false;
	if(srccmp.m_pSel && dstcmp.m_pSel)
	{
		if( (srccmp.m_pSel->getother().first!=dstcmp.m_pSel) ||
			(dstcmp.m_pSel->getother().first!=srccmp.m_pSel) )
			bJoin=srccmp.m_pSel->validother(dstcmp.m_pSel);
		else
		{
			bReset0=true;
			bReset1=true;
		}
	}
	else
	if(srccmp.m_pSel)
	{
		if(srccmp.m_pSel->getother().first!=nullptr)
			bReset0=true;
	}
	else
	if(dstcmp.m_pSel)
	{
		if(dstcmp.m_pSel->getother().first!=nullptr)
			bReset1=true;
	}

	const auto pOther0=srccmp.m_pSel?srccmp.m_pSel->getother().first:nullptr;
	const auto pOther1=dstcmp.m_pSel?dstcmp.m_pSel->getother().first:nullptr;

	if(bJoin)
	{
		srccmp.setother(srccmp.m_pSel,dstcmp.m_pSel);
		dstcmp.setother(dstcmp.m_pSel,srccmp.m_pSel);
	}
	else
	{
		if(bReset0)
		{
			const auto pOther=srccmp.m_pSel->getother().first;
			if(pOther)
			{
				srccmp.setother(srccmp.m_pSel,nullptr);
				dstcmp.setother(pOther,nullptr);
			}
		}
		if(bReset1)
		{
			const auto pOther=dstcmp.m_pSel->getother().first;
			if(pOther)
			{
				srccmp.setother(pOther,nullptr);
				dstcmp.setother(dstcmp.m_pSel,nullptr);
			}
		}
	}

	const auto pNewOther0=srccmp.m_pSel?srccmp.m_pSel->getother().first:nullptr;
	const auto pNewOther1=dstcmp.m_pSel?dstcmp.m_pSel->getother().first:nullptr;

	bool bDelta=(pOther0!=pNewOther0) || (pOther1!=pNewOther1);

	if(bDelta)
	{
		srccmp.triangulate();
		dstcmp.triangulate();
		lerpcmp.triangulate();
		theApp.broadcast(hint(hint::t_link));
	}
}

void CmorphDlg::OnUpdateLinkSelection(CCmdUI *pCmdUI)
{
	auto& srccmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_src);
	auto& dstcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_dst);

	const bool bEnable=( srccmp.m_pSel &&
							( srccmp.m_pSel->getother().first || (dstcmp.m_pSel && srccmp.m_pSel->validother(dstcmp.m_pSel)) ) )
						||
					   ( dstcmp.m_pSel &&
							( dstcmp.m_pSel->getother().first || (srccmp.m_pSel && dstcmp.m_pSel->validother(srccmp.m_pSel)) ) );
	const bool bChecked=( srccmp.m_pSel && srccmp.m_pSel->getother().first ) ||
						( dstcmp.m_pSel && dstcmp.m_pSel->getother().first );
	pCmdUI->Enable(bEnable);
	pCmdUI->SetCheck(bChecked?1:0);
}

void CmorphDlg::OnEraseAll()
{
	auto& srccmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_src);
	auto& dstcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_dst);
	auto& lerpcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_lerp);

	if(srccmp.m_vPrims.size()==0 && dstcmp.m_vPrims.size()==0)
		return;
	const bool bLinkedErase=(!srccmp.m_rLinkedLogBBox.isempty() || !dstcmp.m_rLinkedLogBBox.isempty());
	const int nComponents=(srccmp.m_vPrims.size() && dstcmp.m_vPrims.size()) ? (afmorph::morphcomponent::t_src|afmorph::morphcomponent::t_dst) :
						  (srccmp.m_vPrims.size() ? afmorph::morphcomponent::t_src : afmorph::morphcomponent::t_dst);

	afmorph::primitive *pSrcSel=srccmp.m_pSel;
	afmorph::primitive *pDstSel=dstcmp.m_pSel;
	
	if(pSrcSel)
	{
		srccmp.m_pSel=nullptr;
		theApp.broadcast(hint(hint::t_select,afmorph::morphcomponent::t_src));
	}
	if(pDstSel)
	{
		dstcmp.m_pSel=nullptr;
		theApp.broadcast(hint(hint::t_select,afmorph::morphcomponent::t_dst));
	}

	srccmp.erase();
	dstcmp.erase();
	
	if(bLinkedErase)
	{
		srccmp.triangulate();
		dstcmp.triangulate();
		lerpcmp.triangulate();
	}

	theApp.broadcast(hint(hint::t_erase,nComponents));
}

void CmorphDlg::OnUpdateEraseAll(CCmdUI *pCmdUI)
{
	auto& srccmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_src);
	auto& dstcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_dst);

	const bool bEnable=srccmp.m_vPrims.size() || dstcmp.m_vPrims.size();
	pCmdUI->Enable(bEnable);
}

void CmorphDlg::OnEraseSelection()
{
	auto& srccmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_src);
	auto& dstcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_dst);
	auto& lerpcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_lerp);

	afmorph::primitive *pSrcSel=srccmp.m_pSel;
	afmorph::primitive *pDstSel=dstcmp.m_pSel;
	if(!pSrcSel && !pDstSel)
		return;
	const bool bLinkedErase=(pSrcSel && pSrcSel->getother().first)||(pDstSel && pDstSel->getother().first);
	const int nComponents=(pSrcSel && pDstSel) ? (afmorph::morphcomponent::t_src|afmorph::morphcomponent::t_dst) :
						  (pSrcSel ? afmorph::morphcomponent::t_src : afmorph::morphcomponent::t_dst);

	if(pSrcSel)
	{
		srccmp.m_pSel=nullptr;
		theApp.broadcast(hint(hint::t_select,afmorph::morphcomponent::t_src));
	}
	if(pDstSel)
	{
		dstcmp.m_pSel=nullptr;
		theApp.broadcast(hint(hint::t_select,afmorph::morphcomponent::t_dst));
	}

	if(pSrcSel)
	{
		dstcmp.clearother(pSrcSel);
		srccmp.erase(pSrcSel);
	}
	if(pDstSel)
	{
		srccmp.clearother(pDstSel);
		dstcmp.erase(pDstSel);
	}
	if(bLinkedErase)
	{
		srccmp.triangulate();
		dstcmp.triangulate();
		lerpcmp.triangulate();
	}

	theApp.broadcast(hint(hint::t_erase,nComponents));
}

void CmorphDlg::OnUpdateEraseSelection(CCmdUI *pCmdUI)
{
	auto& srccmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_src);
	auto& dstcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_dst);

	const bool bEnable=srccmp.m_pSel || dstcmp.m_pSel;
	pCmdUI->Enable(bEnable);
}

void CmorphDlg::OnAnimPanel()
{
	if(theApp.getlerpwnd())
		theApp.closelerpwnd();
	else
		theApp.openlerpwnd();
}

void CmorphDlg::OnUpdateAnimPanel(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.getlerpwnd()?1:0);
}

void CmorphDlg::OnAnimGIF()
{
	exportgifdlg dlg;
	dlg.DoModal();
}

void CmorphDlg::OnUpdateAnimGIF(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(theApp.m_spMorph->m_spSrc->m_vLogTriangulated.size()>0);
}

void CmorphDlg::OnSrcSelChanged()
{
	int nSel=m_SrcSel.GetCurSel();
	afmorph::primitive *p=nSel==CB_ERR?nullptr:static_cast<afmorph::primitive *>(m_SrcSel.GetItemDataPtr(nSel));
	
	auto& srccmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_src);
	srccmp.m_pSel=p;

	theApp.broadcast(hint(hint::t_select,afmorph::morphcomponent::t_src));
}

void CmorphDlg::OnDstSelChanged()
{
	int nSel=m_DstSel.GetCurSel();
	afmorph::primitive *p=nSel==CB_ERR?nullptr:static_cast<afmorph::primitive *>(m_DstSel.GetItemDataPtr(nSel));
	
	auto& dstcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_dst);
	dstcmp.m_pSel=p;

	theApp.broadcast(hint(hint::t_select,afmorph::morphcomponent::t_dst));
}

void CmorphDlg::OnSampleRender()
{
	if(theApp.m_spMorph->m_nRender & afmorph::morph::rt_sample)
		theApp.m_spMorph->m_nRender &= ~afmorph::morph::rt_sample;
	else
		theApp.m_spMorph->m_nRender |= afmorph::morph::rt_sample;
	theApp.broadcast(hint(hint::t_renderflag));
}

void CmorphDlg::OnUpdateSampleRender(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.m_spMorph->m_nRender & afmorph::morph::rt_sample?1:0);
}

void CmorphDlg::OnPathRender()
{
	if(theApp.m_spMorph->m_nRender & afmorph::morph::rt_path)
		theApp.m_spMorph->m_nRender &= ~afmorph::morph::rt_path;
	else
		theApp.m_spMorph->m_nRender |= afmorph::morph::rt_path;
	theApp.broadcast(hint(hint::t_renderflag));
}

void CmorphDlg::OnUpdatePathRender(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.m_spMorph->m_nRender & afmorph::morph::rt_path?1:0);
}

void CmorphDlg::OnLinkRender()
{
	if(theApp.m_spMorph->m_nRender & afmorph::morph::rt_link)
		theApp.m_spMorph->m_nRender &= ~afmorph::morph::rt_link;
	else
		theApp.m_spMorph->m_nRender |= afmorph::morph::rt_link;
	theApp.broadcast(hint(hint::t_renderflag));
}

void CmorphDlg::OnUpdateLinkRender(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.m_spMorph->m_nRender & afmorph::morph::rt_link?1:0);
}

void CmorphDlg::OnTriangulateRender()
{
	if(theApp.m_spMorph->m_nRender & afmorph::morph::rt_triangulate)
		theApp.m_spMorph->m_nRender &= ~afmorph::morph::rt_triangulate;
	else
		theApp.m_spMorph->m_nRender |= afmorph::morph::rt_triangulate;
	theApp.broadcast(hint(hint::t_renderflag));
}

void CmorphDlg::OnUpdateTriangulateRender(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.m_spMorph->m_nRender & afmorph::morph::rt_triangulate?1:0);
}

void CmorphDlg::OnTriangulateBkgnd()
{
	theApp.m_spMorph->m_bTriangulateBkgnd=!theApp.m_spMorph->m_bTriangulateBkgnd;
	
	auto& srccmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_src);
	auto& dstcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_dst);
	auto& lerpcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_lerp);
	
	srccmp.triangulate();
	dstcmp.triangulate();
	lerpcmp.triangulate();
	
	theApp.broadcast(hint(hint::t_triangulatebkgnd));
}

void CmorphDlg::OnUpdateTriangulateBkgnd(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.m_spMorph->m_bTriangulateBkgnd?1:0);
}

af2d::rect<> CmorphDlg::getrect(const int nID)const
{
	RECT r;
	auto w = GetDlgItem(nID);
	if(w)
	{
		w->GetWindowRect(&r);
		::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&r,2);
		return {{r.left,r.top},{r.right,r.bottom}};
	}
	return {};
}

void CmorphDlg::updatelayout(void)
{
	if(m_rCtrlRectMapClient.isempty())
		return;
	CRect rcClientRect;
	GetClientRect(rcClientRect);
	auto swp_ptr=[this](CWnd *p,const af2d::rect<>& r)->void{if(p)::SetWindowPos(p->GetSafeHwnd(), NULL, r.get(af2d::rect<>::tl).getx(), r.get(af2d::rect<>::tl).gety(), r.get(af2d::rect<>::br).getx() - r.get(af2d::rect<>::tl).getx(), r.get(af2d::rect<>::br).gety() - r.get(af2d::rect<>::tl).gety(), SWP_DRAWFRAME|SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOZORDER);};
	auto swp_id=[this,swp_ptr](const int n,const af2d::rect<>& r)->void{swp_ptr(GetDlgItem(n),r);};
	{
		af2d::rect<> rGrp = m_CtrlRectMap[IDC_PATH_GRP];
		{
			rGrp.offset(af2d::rect<>::br,{rcClientRect.right-(m_rCtrlRectMapClient.get(af2d::rect<>::br).getx()),0});
			swp_id(IDC_PATH_GRP,rGrp);
		}
		af2d::rect<> rBtn = m_CtrlRectMap[IDC_SRC_BROWSE];
		{
			rBtn.offset({(rGrp.get(af2d::rect<>::br).getx()-m_CtrlRectMap[IDC_PATH_GRP].get(af2d::rect<>::br).getx()),0});
			swp_id(IDC_SRC_BROWSE,rBtn);
		}
		af2d::rect<> rEdit = m_CtrlRectMap[IDC_SRC_PATH];
		{
			rEdit.offset(af2d::rect<>::br,{(rBtn.get(af2d::rect<>::br).getx()-m_CtrlRectMap[IDC_SRC_BROWSE].get(af2d::rect<>::br).getx()),0});
			swp_id(IDC_SRC_PATH,rEdit);
		}
		rBtn = m_CtrlRectMap[IDC_DST_BROWSE];
		{
			rBtn.offset({(rGrp.get(af2d::rect<>::br).getx()-m_CtrlRectMap[IDC_PATH_GRP].get(af2d::rect<>::br).getx()),0});
			swp_id(IDC_DST_BROWSE,rBtn);
		}
		rEdit = m_CtrlRectMap[IDC_DST_PATH];
		{
			rEdit.offset(af2d::rect<>::br,{(rBtn.get(af2d::rect<>::br).getx()-m_CtrlRectMap[IDC_DST_BROWSE].get(af2d::rect<>::br).getx()),0});
			swp_id(IDC_DST_PATH,rEdit);
		}
	}
	af2d::rect<> rClose = m_CtrlRectMap[IDOK];
	{
		rClose.offset({(rcClientRect.right-m_rCtrlRectMapClient.get(af2d::rect<>::br).getx()),(rcClientRect.bottom-m_rCtrlRectMapClient.get(af2d::rect<>::br).gety())});
		swp_id(IDOK,rClose);
	}
	af2d::rect<> rToolBar = m_CtrlRectMap[IDC_TOOLBAR];
	{
		rToolBar.offset({0,(rcClientRect.bottom-m_rCtrlRectMapClient.get(af2d::rect<>::br).gety())});
		swp_id(IDC_TOOLBAR,rToolBar);
	}
	swp_ptr(&m_wndToolBar,rToolBar);
	{
		af2d::rect<> rGrp = m_CtrlRectMap[IDC_FEATURES_GRP];
		{
			rGrp.offset(af2d::rect<>::br,{rcClientRect.right-(m_rCtrlRectMapClient.get(af2d::rect<>::br).getx()),rClose.get(af2d::rect<>::tl).gety()-(m_CtrlRectMap[IDOK].get(af2d::rect<>::tl).gety())});
			swp_id(IDC_FEATURES_GRP,rGrp);
		}
		af2d::rect<> rSplitter = m_CtrlRectMap[IDC_SPLITTER];
		{
			rSplitter.offset(af2d::rect<>::br,{rGrp.get(af2d::rect<>::br).getx()-(m_CtrlRectMap[IDC_FEATURES_GRP].get(af2d::rect<>::br).getx()),rGrp.get(af2d::rect<>::br).gety()-(m_CtrlRectMap[IDC_FEATURES_GRP].get(af2d::rect<>::br).gety())});
			swp_id(IDC_SPLITTER,rSplitter);
		}
		swp_ptr(m_spSplitter.get(),rSplitter);
		af2d::rect<> rSel = m_CtrlRectMap[IDC_SELECTION_STATIC];
		{
			rSel.offset({0,(rGrp.get(af2d::rect<>::br).gety()-m_CtrlRectMap[IDC_FEATURES_GRP].get(af2d::rect<>::br).gety())});
			swp_id(IDC_SELECTION_STATIC,rSel);
		}
		af2d::rect<> rSrcSel = m_CtrlRectMap[IDC_SRC_SELECTION];
		{
			rSrcSel.offset({0,(rGrp.get(af2d::rect<>::br).gety()-m_CtrlRectMap[IDC_FEATURES_GRP].get(af2d::rect<>::br).gety())});
			swp_id(IDC_SRC_SELECTION,rSrcSel);
		}
		af2d::rect<> rSelSep = m_CtrlRectMap[IDC_SELECTION_SEPARATOR];
		{
			rSelSep.offset({0,(rGrp.get(af2d::rect<>::br).gety()-m_CtrlRectMap[IDC_FEATURES_GRP].get(af2d::rect<>::br).gety())});
			swp_id(IDC_SELECTION_SEPARATOR,rSelSep);
		}
		af2d::rect<> rDstSel = m_CtrlRectMap[IDC_DST_SELECTION];
		{
			rDstSel.offset({0,(rGrp.get(af2d::rect<>::br).gety()-m_CtrlRectMap[IDC_FEATURES_GRP].get(af2d::rect<>::br).gety())});
			swp_id(IDC_DST_SELECTION,rDstSel);
		}
	}
}

void CmorphDlg::enabledisable()
{
	auto& srccmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_src);
	auto& dstcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_dst);

	const bool bSrcSel=srccmp.m_spDib.get();
	const bool bDstSel=dstcmp.m_spDib.get();
	m_SrcSel.EnableWindow(bSrcSel);
	m_DstSel.EnableWindow(bDstSel);
	GetDlgItem(IDC_SRC_PATH)->EnableWindow(bSrcSel);
	GetDlgItem(IDC_DST_PATH)->EnableWindow(bDstSel);
}

bool CmorphDlg::browse(afmorph::morphcomponent::type t)
{
	CFileDialog dlg(true);
	if(dlg.DoModal()!=IDOK)
		return false;
	CString csPath=dlg.GetPathName();
	std::shared_ptr<afdib::dib> spDib=afdib::jpeg::load8bpp(csPath);
	if(!spDib)
		return false;
	if(spDib->getpixeltype()!=afdib::dib::pt_b8g8r8)
		return false;

	auto& cmp=theApp.m_spMorph->getcomponent(t);
	auto& othercmp=theApp.m_spMorph->getcomponent(t,true);

	auto& srccmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_src);
	auto& dstcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_dst);
	auto& lerpcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_lerp);
	
	cmp.setdib(spDib);
	cmp.m_csPath=csPath;

	othercmp.clearother();
	
	srccmp.triangulate();
	dstcmp.triangulate();
	lerpcmp.triangulate();
	
	return true;
}

void CmorphDlg::populateselectioncombo(const afmorph::morphcomponent::type t)
{
	auto& cmp=theApp.m_spMorph->getcomponent(t);
	const std::vector<std::shared_ptr<afmorph::primitive>>& vPrims=cmp.m_vPrims;
	CComboBox *pCB=nullptr;
	switch(t)
	{
		case afmorph::morphcomponent::t_src:pCB=&m_SrcSel;break;
		case afmorph::morphcomponent::t_dst:pCB=&m_DstSel;break;
	}
	if(!pCB)
		return;

	std::set<const afmorph::primitive*> sPrims;
	{
		auto i=vPrims.cbegin(),end=vPrims.cend();
		for(;i!=end;++i)
			sPrims.insert((*i).get());
	}
	for(int n=pCB->GetCount()-1;n>0;--n)
		if(sPrims.find(static_cast<const afmorph::primitive*>(pCB->GetItemDataPtr(n)))==sPrims.cend())
			pCB->DeleteString(n);
	if(pCB->GetCount()==1)
		pCB->SetCurSel(0); // N/A

	if(vPrims.size()==0)
		return;

	int n=0;
	const int nPrims=static_cast<int>(vPrims.size());
	for(;n<nPrims;++n)
	{
		const int nCombo=n+1;
		if(nCombo>=pCB->GetCount())
			break;

		CString cs,csName;
		csName.Format(_T("feature [%li]"),n);
		pCB->GetLBText(nCombo,cs);
		if(!(cs==csName && pCB->GetItemDataPtr(nCombo)==vPrims[n].get()))
			break;
	}
	for(int nCombo=n+1;nCombo<pCB->GetCount();++nCombo)
		pCB->DeleteString(pCB->GetCount()-1);
	for(;n<nPrims;++n)
	{
		CString csName;
		csName.Format(_T("feature [%li]"),n);
		const int nItem=pCB->AddString(csName);
		pCB->SetItemDataPtr(nItem,vPrims[n].get());
	}
}

void CmorphDlg::setselectioncomboindex(const afmorph::morphcomponent::type t)
{
	auto& cmp=theApp.m_spMorph->getcomponent(t);
	const std::vector<std::shared_ptr<afmorph::primitive>>& vPrims=cmp.m_vPrims;
	CComboBox *pCB=nullptr;
	switch(t)
	{
		case afmorph::morphcomponent::t_src:pCB=&m_SrcSel;break;
		case afmorph::morphcomponent::t_dst:pCB=&m_DstSel;break;
	}
	if(!pCB)
		return;
	afmorph::primitive *pSel=cmp.m_pSel;
	for(int nCombo=0;nCombo<pCB->GetCount();++nCombo)
		if(pCB->GetItemDataPtr(nCombo)==pSel)
		{
			pCB->SetCurSel(nCombo);
			break;
		}
}

void CmorphDlg::process(const hint& h)
{
	auto i=m_spSplitter->getpanes().begin(),end=m_spSplitter->getpanes().end();
	for(;i!=end;++i)
		(*i)->process(h);

	switch(h.m_Type)
	{
		case hint::t_link:
		case hint::t_frame:
		case hint::t_frames:
		case hint::t_renderflag:
		case hint::t_triangulatebkgnd:break;
		case hint::t_dib:
		case hint::t_new:
		case hint::t_load:
		{
			UpdateData(false);
			if(h.m_nComponents & afmorph::morphcomponent::t_src)
			{
				populateselectioncombo(afmorph::morphcomponent::t_src);
				setselectioncomboindex(afmorph::morphcomponent::t_src);
			}
			if(h.m_nComponents & afmorph::morphcomponent::t_dst)
			{
				populateselectioncombo(afmorph::morphcomponent::t_dst);
				setselectioncomboindex(afmorph::morphcomponent::t_dst);
			}
			enabledisable();
		}
		break;
		case hint::t_pushback:
		{
			if(h.m_nComponents & afmorph::morphcomponent::t_src)
				populateselectioncombo(afmorph::morphcomponent::t_src);
			if(h.m_nComponents & afmorph::morphcomponent::t_dst)
				populateselectioncombo(afmorph::morphcomponent::t_dst);
		}
		break;
		case hint::t_select:
		{
			if(h.m_nComponents & afmorph::morphcomponent::t_src)
				setselectioncomboindex(afmorph::morphcomponent::t_src);
			else
			if(h.m_nComponents & afmorph::morphcomponent::t_dst)
				setselectioncomboindex(afmorph::morphcomponent::t_dst);
		}
		break;
		case hint::t_erase:
		{
			if(h.m_nComponents & afmorph::morphcomponent::t_src)
			{
				populateselectioncombo(afmorph::morphcomponent::t_src);
				setselectioncomboindex(afmorph::morphcomponent::t_src);
			}
			if(h.m_nComponents & afmorph::morphcomponent::t_dst)
			{
				populateselectioncombo(afmorph::morphcomponent::t_dst);
				setselectioncomboindex(afmorph::morphcomponent::t_dst);
			}
		}
		break;
		default:ASSERT(false);
	}
}
