#pragma once

#include "lerpdlg.h"
#include <memory>

// lerpwnd

class lerpwnd : public CWnd
{
	DECLARE_DYNAMIC(lerpwnd)

public:
	lerpwnd();
	virtual ~lerpwnd();

	lerpdlg *getdlg(void) { return m_spDlg.get(); }
protected:
	std::shared_ptr<lerpdlg> m_spDlg;

	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT p);
	afx_msg void OnSize(UINT nType,int cx,int cy);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	DECLARE_MESSAGE_MAP()
};
