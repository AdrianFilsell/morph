// exportgifdlg.cpp : implementation file
//

#include "pch.h"
#include "morph.h"
#include "afxdialogex.h"
#include "exportgifdlg.h"
#include "morphcomponent.h"
#include "gif.h"
#include "dibwnd.h"


// exportgifdlg dialog

IMPLEMENT_DYNAMIC(exportgifdlg, CDialogEx)

exportgifdlg::exportgifdlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EXPORT_GIF, pParent)
{
	m_csPath=afmorph::morphcomponent::s_szNull;
	m_csProgress=afmorph::morphcomponent::s_szNull;
	m_nWidth=0;
	m_nHeight=0;
	m_nFrames=theApp.m_spMorph->m_nLerpFrames;
	m_nFPS=24;
	m_nProgress=1000;
	m_nBounce=BST_UNCHECKED;

	const af2d::rect<long> rLog=theApp.m_spMorph->m_spSrc->m_rLog.getunion(theApp.m_spMorph->m_spDst->m_rLog);
	if(!rLog.isempty())
	{
		m_nWidth=rLog.getwidth();
		m_nHeight=rLog.getheight();
	}
}

exportgifdlg::~exportgifdlg()
{
}

void exportgifdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX,IDC_PATH_STATIC,m_csPath);
	DDX_Text(pDX,IDC_WIDTH_EDIT,m_nWidth);
	DDX_Text(pDX,IDC_HEIGHT_EDIT,m_nHeight);
	DDX_Text(pDX,IDC_FRAMES_EDIT,m_nFrames);
	DDX_Text(pDX,IDC_FPS_EDIT,m_nFPS);
	DDX_Text(pDX,IDC_PROGRESS_STATIC,m_csProgress);

	DDX_Check(pDX,IDC_BOUNCE_CHECK,m_nBounce);

	DDX_Control(pDX,IDC_PROGRESS,m_Progress);
}


BEGIN_MESSAGE_MAP(exportgifdlg, CDialogEx)
	ON_BN_CLICKED(IDC_BROWSE,exportgifdlg::OnBrowse)

	ON_EN_KILLFOCUS(IDC_FPS_EDIT,OnFPSTextKillFocus)
	ON_EN_KILLFOCUS(IDC_FRAMES_EDIT,OnFramesTextKillFocus)
	ON_EN_KILLFOCUS(IDC_WIDTH_EDIT,OnWidthTextKillFocus)
	ON_EN_KILLFOCUS(IDC_HEIGHT_EDIT,OnHeightTextKillFocus)
END_MESSAGE_MAP()


BOOL exportgifdlg ::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_FRAMES_SPIN))->SetRange32(0,0x7fffffff);
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_FPS_SPIN))->SetRange32(0,0x7fffffff);
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_WIDTH_SPIN))->SetRange32(0,0x7fffffff);
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_HEIGHT_SPIN))->SetRange32(0,0x7fffffff);
	m_Progress.SetRange32(0,m_nProgress);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// exportgifdlg message handlers

void exportgifdlg::OnBrowse()
{
	CFileDialog dlg(false);
	if(dlg.DoModal()!=IDOK)
		return;
	m_csPath=dlg.GetPathName();
	UpdateData(false);
}

void exportgifdlg::OnFPSTextKillFocus()
{
	UpdateData();
}

void exportgifdlg::OnFramesTextKillFocus()
{
	UpdateData();
}

void exportgifdlg::OnWidthTextKillFocus()
{
	UpdateData();
}

void exportgifdlg::OnHeightTextKillFocus()
{
	UpdateData();
}

void exportgifdlg::OnOK()
{
	UpdateData();

	if(m_nWidth>0 && m_nHeight>0 && m_nFPS>0 && m_nFrames>0)
	{
		const af2d::rect<long> rLog=theApp.m_spMorph->m_spSrc->m_rLog.getunion(theApp.m_spMorph->m_spDst->m_rLog);
		if(rLog.isempty())
			return;
		const af2d::rect<long> rExport={{0,0},{m_nWidth,m_nHeight}};
		
		GifWriter gifWriter;
		std::vector<unsigned char> vGIFCanvas(rExport.getwidth() * rExport.getheight() * 4); // RGBA
	
		const int nDelay=100/m_nFPS; // 100th's of a second
		if(GifBegin(&gifWriter, CStringA(m_csPath), rExport.getwidth(), rExport.getheight(), nDelay))
		{
			const bool bBounce=m_nBounce==BST_CHECKED;
			std::vector<int> vFrames;
			for( int n=0;n<m_nFrames;++n)
				vFrames.push_back(n);
			if(bBounce)
			for( int n=m_nFrames-2;n>=0;--n)
				vFrames.push_back(n);
			const int nPreFrame=theApp.m_spMorph->m_nLerpFrame;
			const int nPreFrames=theApp.m_spMorph->m_nLerpFrames;
			theApp.m_spMorph->m_nLerpFrames=m_nFrames;
			
			const int nLerpFrames=static_cast<int>(vFrames.size());
			auto& lerpcmp=theApp.m_spMorph->getcomponent(afmorph::morphcomponent::t_lerp);
			auto i=vFrames.cbegin(),end=vFrames.cend();
			for(int n=0;i!=end;++i,++n)
			{
				memset(&(vGIFCanvas[0]),0,rExport.getwidth()*rExport.getheight()*4); // 0% opaque black

				theApp.m_spMorph->m_nLerpFrame = (*i)+1;
				lerpcmp.triangulate();

				MOR_FLTTYPE dS=1;
				af2d::rect<>::getrectscale(0,0,lerpcmp.m_rLog.getwidth(),lerpcmp.m_rLog.getheight(),
										   0,0,rExport.getwidth(),rExport.getheight(),
										   true,dS);

				std::shared_ptr<Eigen::Affine2d> spDevToLog(new Eigen::Affine2d);
				std::shared_ptr<Eigen::Affine2d> spLogToDev(new Eigen::Affine2d);

				*spLogToDev=Eigen::Translation<double,2>((rExport.getwidth()/2.0),(rExport.getheight()/2.0)) *
				Eigen::DiagonalMatrix<double,2>(dS,dS) *
				Eigen::Translation<double,2>(-(lerpcmp.m_rLog.getwidth()/2.0),-(lerpcmp.m_rLog.getheight()/2.0));
				
				*spDevToLog=spLogToDev->inverse();
				
				const afmorph::xform<> trns(spLogToDev.get(),spDevToLog.get(),lerpcmp.m_rLog.getwidth(),lerpcmp.m_rLog.getheight());

				dibwnd::renderlerp(&(vGIFCanvas[0]),rExport.getwidth(),rExport.getheight(),rExport.getwidth() * 4,4,CRect(0,0,rExport.getwidth(),rExport.getheight()),spDevToLog.get(),spLogToDev.get());

				af2d::rect<> rCanvas;
				trns.ltod(lerpcmp.m_rLog,rCanvas);

				const af2d::rect<> rIntersect=rCanvas.getintersect(rExport);
				for(int nCanvasY=rIntersect.get(af2d::rect<>::tl).gety();nCanvasY<rIntersect.get(af2d::rect<>::br).gety();++nCanvasY)
				{
					unsigned char c;
					unsigned char *pCanvas=&(vGIFCanvas[nCanvasY*rExport.getwidth()*4]);
					for(int nCanvasX=rIntersect.get(af2d::rect<>::tl).getx();nCanvasX<rIntersect.get(af2d::rect<>::br).getx();++nCanvasX)
					{
						// gif needs rgb
						c=pCanvas[nCanvasX*4+0];
						pCanvas[nCanvasX*4+0]=pCanvas[nCanvasX*4+2];
						pCanvas[nCanvasX*4+2]=c;
					}
				}

				GifWriteFrame(&gifWriter, &(vGIFCanvas[0]), rExport.getwidth(), rExport.getheight(), nDelay);

				const MOR_FLTTYPE dLerp=nLerpFrames>1?(n)/(static_cast<MOR_FLTTYPE>(nLerpFrames)-1):0;
				const int nProgress=af::posround<MOR_FLTTYPE,int>(dLerp*m_nProgress);
				m_Progress.SetPos(nProgress);
				m_csProgress.Format(_T("%li/%li"),theApp.m_spMorph->m_nLerpFrame,theApp.m_spMorph->m_nLerpFrames);
				UpdateData(false);

				TRACE(_T("GIF - frame[%li]\r\n"),(*i)+1);
			}
			GifEnd(&gifWriter);

			theApp.m_spMorph->m_nLerpFrame = nPreFrame;
			theApp.m_spMorph->m_nLerpFrames = nPreFrames;
			lerpcmp.triangulate();
		}
		else
		{
			CString cs;
			cs.Format(_T("Could not create file:\r\n\r\n%s"),m_csPath);
			if(AfxMessageBox(cs,MB_RETRYCANCEL|MB_ICONQUESTION)==IDRETRY)
				return;
		}
	}

	CDialogEx::OnOK();
}
