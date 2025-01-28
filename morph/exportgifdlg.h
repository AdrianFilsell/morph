#pragma once
#include "afxdialogex.h"


// exportgifdlg dialog

class exportgifdlg : public CDialogEx
{
	DECLARE_DYNAMIC(exportgifdlg)

public:
	exportgifdlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~exportgifdlg();

// Dialog Data
	enum { IDD = IDD_EXPORT_GIF };
	CString m_csPath;
	int m_nWidth;
	int m_nHeight;
	int m_nFrames;
	int m_nFPS;
	CString m_csProgress;
	CProgressCtrl m_Progress;
	int m_nBounce;

protected:
	int m_nProgress;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBrowse();

	afx_msg void OnFPSTextKillFocus();
	afx_msg void OnFramesTextKillFocus();
	afx_msg void OnWidthTextKillFocus();
	afx_msg void OnHeightTextKillFocus();
};
