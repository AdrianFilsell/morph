#pragma once

#include "afxdialogex.h"
#include "resource.h"
#include "primitive.h"
#include "dibwnd.h"

// lerpdlg dialog

class lerpdlg : public CDialogEx
{
	DECLARE_DYNAMIC(lerpdlg)

public:

	lerpdlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~lerpdlg();

	void process(const hint& h);

	void enabledisable(void);

// Dialog Data
	CSliderCtrl m_Slider;
	
	enum { IDD = IDD_LERP };
protected:
	
	bool m_bInitialised;
	
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override {}
	virtual void OnCancel() override {}

	// Generated message map functions
	afx_msg void OnHScroll(UINT nSBCode,UINT nPos,CScrollBar* pScrollBar);
	
	afx_msg void OnFrameTextKillFocus(void);
	afx_msg void OnTotalTextKillFocus(void);

	afx_msg void OnFrameTextChange(void);
	afx_msg void OnTotalTextChange(void);
	DECLARE_MESSAGE_MAP()

	std::shared_ptr<dibwnd> m_spDibWnd;	
};
