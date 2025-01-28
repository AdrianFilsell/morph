
// morph.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "hint.h"


// CmorphApp:
// See morph.cpp for the implementation of this class
//

class CmorphDlg;
class lerpwnd;

class CmorphApp : public CWinApp
{
public:
	CmorphApp();

	static const afthread::taskscheduler *getsched(void)
	{
		#ifdef  _DEBUG
			return nullptr;
		#endif
		return &m_Sched;
	}

	void broadcast(const hint& h)const;

	std::shared_ptr<afmorph::morph> m_spMorph;

// Overrides
public:
	virtual BOOL InitInstance();

	CmorphDlg *getdlg()const;
	lerpwnd *getlerpwnd()const{return m_spLerpWnd.get();}
	void closelerpwnd();
	void openlerpwnd();

	virtual int ExitInstance(void)override{closelerpwnd();return CWinApp::ExitInstance();}

// Implementation

	DECLARE_MESSAGE_MAP()
protected:
	std::shared_ptr<lerpwnd> m_spLerpWnd;
	static const afthread::taskscheduler m_Sched;
};

extern CmorphApp theApp;
