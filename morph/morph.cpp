
// morph.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "morph.h"
#include "morphDlg.h"
#include "lerpwnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CmorphApp

const afthread::taskscheduler CmorphApp::m_Sched;

BEGIN_MESSAGE_MAP(CmorphApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CmorphApp construction

CmorphApp::CmorphApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_spMorph=std::shared_ptr<afmorph::morph>(new afmorph::morph);
}


// The one and only CmorphApp object

CmorphApp theApp;


// CmorphApp initialization

BOOL CmorphApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CmorphDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

CmorphDlg *CmorphApp::getdlg()const
{
	return static_cast<CmorphDlg*>(m_pMainWnd);
}

void CmorphApp::openlerpwnd(void)
{
	// already visible
	if(m_spLerpWnd)
		return;

	CRect rcDlg;
	theApp.getdlg()->GetWindowRect(rcDlg);

	m_spLerpWnd=std::shared_ptr<lerpwnd>(new lerpwnd);
	const CSize sz(500,rcDlg.Height());
	
	const BOOL b = m_spLerpWnd->CreateEx(WS_EX_CONTROLPARENT|WS_EX_TOOLWINDOW,AfxRegisterWndClass(0,theApp.LoadStandardCursor(IDC_ARROW)),_T("lerp"),WS_POPUPWINDOW|WS_CAPTION|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|MFS_MOVEFRAME|MFS_SYNCACTIVE,CRect(CPoint(0,0),sz),nullptr,0);
	CRect rcWindow, rcClient;
	m_spLerpWnd->GetWindowRect(rcWindow);
	m_spLerpWnd->GetClientRect(rcClient);
	const CSize szDelta(rcWindow.Width()-rcClient.Width(),rcWindow.Height()-rcClient.Height()); 

	lerpdlg *pDlg = m_spLerpWnd->getdlg();
	CRect rcDlgWindow;
	pDlg->GetWindowRect(rcDlgWindow);
	rcWindow = CRect(rcWindow.TopLeft(),CSize(rcDlgWindow.Width(),rcDlgWindow.Height())+szDelta);

	rcWindow.OffsetRect(rcDlg.right-rcWindow.left,rcDlg.top-rcWindow.top);
//	rcWindow.bottom=rcWindow.top+rcDlg.Height();

	m_spLerpWnd->MoveWindow(rcWindow);
	m_spLerpWnd->ShowWindow(SW_NORMAL);
}

void CmorphApp::closelerpwnd()
{
	if(!m_spLerpWnd)
		return;
	if(m_spLerpWnd->GetSafeHwnd())
		m_spLerpWnd->DestroyWindow();
	m_spLerpWnd=nullptr;
}

void CmorphApp::broadcast(const hint& h)const
{
	{
		CmorphDlg *p=getdlg();
		if(p)
			p->process(h);
	}
	if(getlerpwnd())
	{
		lerpdlg *p=getlerpwnd()->getdlg();
		if(p)
			p->process(h);
	}
}
