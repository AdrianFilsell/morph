
// morphDlg.h : header file
//

#pragma once

#include "primitive.h"
#include "2d.h"
#include "splitterwnd.h"
#include <map>


// CmorphDlg dialog
class CmorphDlg : public CDialogEx
{
// Construction
public:
	CmorphDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MORPH_DIALOG };
#endif

	splitterwnd *getsplitterwnd(void)const{return m_spSplitter.get();}
	void process(const hint& h);

	CComboBox m_SrcSel;
	CComboBox m_DstSel;

	protected:
	virtual void OnCancel(void)override;
	virtual void DoDataExchange(CDataExchange* pDX)override;	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;
	af2d::rect<> m_rCtrlRectMapClient;
	std::map<int,af2d::rect<>> m_CtrlRectMap;
	std::shared_ptr<splitterwnd> m_spSplitter;
	bool m_bInitialised;

	CToolBar m_wndToolBar;
	std::shared_ptr<CImageList> m_spToolBarIL;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LPARAM OnKickIdle(WPARAM,LPARAM);

	afx_msg void OnSrcBrowse();
	afx_msg void OnDstBrowse();

	afx_msg void OnNew();
	afx_msg void OnLoad();
	afx_msg void OnSave();

	afx_msg void OnSelectMode();
	afx_msg void OnUpdateSelectMode(CCmdUI *pCmdUI);
	afx_msg void OnFeatureMode();
	afx_msg void OnUpdateFeatureMode(CCmdUI *pCmdUI);

	afx_msg void OnLinkSelection();
	afx_msg void OnUpdateLinkSelection(CCmdUI *pCmdUI);

	afx_msg void OnEraseAll();
	afx_msg void OnUpdateEraseAll(CCmdUI *pCmdUI);
	afx_msg void OnEraseSelection();
	afx_msg void OnUpdateEraseSelection(CCmdUI *pCmdUI);

	afx_msg void OnAnimPanel();
	afx_msg void OnUpdateAnimPanel(CCmdUI *pCmdUI);
	afx_msg void OnAnimGIF();
	afx_msg void OnUpdateAnimGIF(CCmdUI *pCmdUI);

	afx_msg void OnSampleRender();
	afx_msg void OnUpdateSampleRender(CCmdUI *pCmdUI);
	afx_msg void OnTriangulateRender();
	afx_msg void OnUpdateTriangulateRender(CCmdUI *pCmdUI);
	afx_msg void OnPathRender();
	afx_msg void OnUpdatePathRender(CCmdUI *pCmdUI);
	afx_msg void OnLinkRender();
	afx_msg void OnUpdateLinkRender(CCmdUI *pCmdUI);

	afx_msg void OnTriangulateBkgnd();
	afx_msg void OnUpdateTriangulateBkgnd(CCmdUI *pCmdUI);

	afx_msg void OnSrcSelChanged();
	afx_msg void OnDstSelChanged();
	DECLARE_MESSAGE_MAP()

	af2d::rect<> getrect(const int nID)const;
	void updatelayout(void);

	void enabledisable(void);

	bool browse(const afmorph::morphcomponent::type t);

	void populateselectioncombo(const afmorph::morphcomponent::type t);
	void setselectioncomboindex(const afmorph::morphcomponent::type t);
};
