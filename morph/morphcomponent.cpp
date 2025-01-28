
#include "pch.h"
#include "morphcomponent.h"
#include "morph.h"
#include "jpeg.h"

namespace afmorph
{

class getaffineop
{
public:
	getaffineop(const dt_ocv::subdiv<>::tri *pSrcTri,const dt_ocv::subdiv<>::tri *pDstTri,dt_ocv::subdiv<>::tri *pLerpTri,
				Eigen::Matrix<double, 2, 3> *pLogLerpToSrc,Eigen::Matrix<double, 2, 3> *pLogLerpToDst):
				m_pSrcTri(pSrcTri),m_pDstTri(pDstTri),m_pLerpTri(pLerpTri),m_pLogLerpToSrc(pLogLerpToSrc),m_pLogLerpToDst(pLogLerpToDst){}
	void operator()(const int nFrom,const int nInclusiveTo,const afthread::taskinfo *m_pTaskInfo)const
	{
		for(int n=nFrom;n<=nInclusiveTo;++n)
		{
			morphcomponent::getAffineTransform(m_pLerpTri[n].abc,m_pSrcTri[n].abc,m_pLogLerpToSrc[n]);
			morphcomponent::getAffineTransform(m_pLerpTri[n].abc,m_pDstTri[n].abc,m_pLogLerpToDst[n]);
			m_pLerpTri[n].bbox=af2d::rect<MOR_FLTTYPE>(m_pLerpTri[n].abc,3);
			m_pLerpTri[n].ab={m_pLerpTri[n].abc[1].getx() - m_pLerpTri[n].abc[0].getx(), m_pLerpTri[n].abc[1].gety() - m_pLerpTri[n].abc[0].gety()};
			m_pLerpTri[n].bc={m_pLerpTri[n].abc[2].getx() - m_pLerpTri[n].abc[1].getx(), m_pLerpTri[n].abc[2].gety() - m_pLerpTri[n].abc[1].gety()};
			m_pLerpTri[n].ca={m_pLerpTri[n].abc[0].getx() - m_pLerpTri[n].abc[2].getx(), m_pLerpTri[n].abc[0].gety() - m_pLerpTri[n].abc[2].gety()};
		}
	}
protected:
	const dt_ocv::subdiv<>::tri *m_pSrcTri;
	const dt_ocv::subdiv<>::tri *m_pDstTri;
	dt_ocv::subdiv<>::tri *m_pLerpTri;
	Eigen::Matrix<double, 2, 3> *m_pLogLerpToSrc;
	Eigen::Matrix<double, 2, 3> *m_pLogLerpToDst;
};

const TCHAR morphcomponent::s_szNull[]=_T("N/A\0");

auto ptxyfn=[](const std::pair<af2d::point<MOR_FLTTYPE>,int>& a,const std::pair<af2d::point<MOR_FLTTYPE>,int>& b)->bool
	{
		if(a.first.getx()<b.first.getx())return true;
		if(a.first.getx()>b.first.getx())return false;
		return a.first.gety()<b.first.gety();
	};

morph::morph()
{
	m_spSrc=std::shared_ptr<morphsrc>(new morphsrc(this));
	m_spDst=std::shared_ptr<morphdst>(new morphdst(this));
	m_spLerp=std::shared_ptr<morphlerp>(new morphlerp(this));
	m_bTriangulateBkgnd=true;
	m_nRender=(rt_path|rt_link);
	m_nLerpFrames=100;
	m_nLerpFrame=1;
}

bool morph::read(const serialise *pS)
{
	// version
	int nVersion=0;
	if(!pS->read<>(nVersion) || nVersion<1)
		return false;

	// members
	if(!pS->read<>(m_nRender))
		return false;
	if(!pS->read<>(m_bTriangulateBkgnd))
		return false;
	if(!pS->read<>(m_nLerpFrames))
		return false;
	if(!pS->read<>(m_nLerpFrame))
		return false;
	if(!m_spSrc->read(pS))
		return false;
	if(!m_spDst->read(pS))
		return false;

	return true;
}

bool morph::write(const serialise *pS) const
{
	// base class

	// version
	const int nVersion = 1;
	if(!pS->write<>(nVersion))
		return false;

	// members
	if(!pS->write<>(m_nRender))
		return false;
	if(!pS->write<>(m_bTriangulateBkgnd))
		return false;
	if(!pS->write<>(m_nLerpFrames))
		return false;
	if(!pS->write<>(m_nLerpFrame))
		return false;
	if(!m_spSrc->write(pS))
		return false;
	if(!m_spDst->write(pS))
		return false;
	
	return true;
}

bool morph::validateread(const CString& csArchive)
{
	// linked prims
	auto iSrc=m_spSrc->m_vPrims.begin(),end=m_spSrc->m_vPrims.end();
	std::vector<std::shared_ptr<primitive>> vDstPrims=m_spDst->m_vPrims;
	for(;iSrc!=end;++iSrc)
	{
		if((*iSrc)->getother().second==GUID_NULL)
			continue;
		auto iDst=vDstPrims.begin(),end=vDstPrims.end();
		for(;iDst!=end;++iDst)
			if((*iDst)->getguid()==(*iSrc)->getother().second)
			{
				(*iSrc)->setother((*iDst).get());
				(*iDst)->setother((*iSrc).get());
				vDstPrims.erase(iDst);
				break;
			}
		if(!(*iSrc)->getother().first)
			(*iSrc)->setother(nullptr); // ???
	}
	auto iDst=m_spDst->m_vPrims.begin();
	end=m_spDst->m_vPrims.end();
	for(;iDst!=end;++iDst)
	{
		if((*iDst)->getother().second==GUID_NULL)
			continue;
		if(!(*iDst)->getother().first)
			(*iDst)->setother(nullptr); // ???
	}

	// component specific
	return m_spSrc->validateread(csArchive) && m_spDst->validateread(csArchive);
}

bool morphcomponent::validateread(const CString& csArchive)
{
	if(validatepath(csArchive,m_csPath))
	{
		std::shared_ptr<afdib::dib> spDib=afdib::jpeg::load8bpp(m_csPath);
		if(spDib && spDib->getpixeltype()==afdib::dib::pt_b8g8r8)
		{
			m_spDib=spDib;
			af2d::rect<> rLog;
			if(m_spDib)
				rLog={{0,0},{m_spDib->getwidth(),m_spDib->getheight()}};
			m_rLog=rLog;
			getlinkedlogbbox();
			return true;
		}
	}
	setdib(nullptr);
	return true;
}

bool morphcomponent::validatepath(const CString& csArchive,CString& cs)const
{
	if(cs==morphcomponent::s_szNull)
		return false;

	auto existsFn=[](LPCTSTR lp)->bool
	{
		const DWORD dwAttributes = ::GetFileAttributes(lp);
		return (dwAttributes != ((DWORD)-1)) && ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0);
	};

	if(existsFn(cs))
		return true;

	wchar_t _drive[_MAX_DRIVE], _dir[_MAX_DIR], _fname[_MAX_FNAME], _ext[_MAX_EXT];
	_wsplitpath_s( cs, _drive, _MAX_DRIVE, _dir, _MAX_DIR, _fname, _MAX_FNAME, _ext, _MAX_EXT );
	const CString csFName(_fname);
	const CString csExt(_ext);
	
	_wsplitpath_s( csArchive, _drive, _MAX_DRIVE, _dir, _MAX_DIR, _fname, _MAX_FNAME, _ext, _MAX_EXT );
	CString csDir = CString(_drive)+CString(_dir);
	csDir.TrimRight(_T("\\"));

	CString csAttempt=csDir+_T("\\")+csFName+csExt;
	while(true)
	{
		if(existsFn(csAttempt))
		{
			cs=csAttempt;
			return true;
		}
		CString csMsg;
		csMsg.Format(_T("locating: \"%s\"\r\ncould not load: \"%s\"\r\nwould you like to load an alternative file?"),csFName+csExt,csAttempt);
		if(AfxMessageBox(csMsg,MB_YESNO|MB_ICONQUESTION)==IDNO)
			break;

		CFileDialog dlg(true);
		if(dlg.DoModal()!=IDOK)
			break;
		csAttempt=dlg.GetPathName();
	}

	return false;
}

void morphcomponent::triangulate_srcdst(void)
{
	// T = 2N - K - 2
	// T : number of triangles
	// N : number of points to be triangulated
	// K : number of points on convex hull
	// given we need T to be the same for the src and dst lets take the union of all points ( src and dst )
	// find their bbox and add rect extents, these WILL be unique and this ensuring there are only 4 points on the convex hull ie K == 4

	const bool bTriangulateBkgnd=m_pMorph->m_bTriangulateBkgnd;
	const bool bSrc=m_Type==t_src;
	
	auto& srccmp=m_pMorph->getcomponent(t_src);

	if(m_rLog.isempty())
	{
		cleartriangulated();
		return;
	}

	// logical bbox, extents of background ( if being used ) and the linked logical points
	af2d::rect<> rLogBBox=m_rLinkedLogBBox;
	if(bTriangulateBkgnd)
		rLogBBox=rLogBBox.getunion(af2d::rect<>(m_rLog.get(af2d::rect<>::tl),{m_rLog.get(af2d::rect<>::br).getx()+1,m_rLog.get(af2d::rect<>::br).gety()+1}));
	else
	if(rLogBBox.isempty())
	{
		cleartriangulated();
		return;
	}

	// logical sub pixel points and ids ( the point id is its index )
	std::vector<std::pair<af2d::point<MOR_FLTTYPE>,int>> vLogPts;
	vLogPts.reserve(m_spDevTriangulate?m_spDevTriangulate->get().size():0);
	int n=0;
	const std::vector<std::shared_ptr<afmorph::primitive>>& vSrcPrims=srccmp.m_vPrims;
	auto i=vSrcPrims.cbegin(),end=vSrcPrims.cend();
	for(;i!=end;++i)
	{
		afmorph::primitive *pPrim=bSrc?(*i).get():(*i)->getother().first;
		const int nLogSamples=pPrim?pPrim->getlogsamples():0;
		const bool bRadianSort=true;
		const af2d::pointvec<MOR_FLTTYPE> *pPts=pPrim?pPrim->getlogtriangulatepts(nLogSamples,bRadianSort):nullptr;
		if(!pPts || pPts->isempty() || !pPrim->getother().first)
			continue;
		auto j=pPts->get().cbegin(),end=pPts->get().cend();
		for(;j!=end;++j,++n)
			vLogPts.push_back({{(*j).getx()+0.5,(*j).gety()+0.5},n});
	}
	if(bTriangulateBkgnd)
	{
		const af2d::point<MOR_FLTTYPE> tl={0,0};												// inclusive
		const af2d::point<MOR_FLTTYPE> br={m_rLog.getwidth()-0.0,m_rLog.getheight()-0.0};		// inclusive
					
		vLogPts.push_back({{tl.getx()+0.0,tl.gety()+0.0},n+0});
		vLogPts.push_back({{br.getx()-0.0,tl.gety()+0.0},n+1});
		vLogPts.push_back({{br.getx()-0.0,br.gety()-0.0},n+2});
		vLogPts.push_back({{tl.getx()+0.0,br.gety()-0.0},n+3});
	}

	// cache
	{
		m_spTriangulate=m_spTriangulate?m_spTriangulate:std::shared_ptr<af2d::pointvec<MOR_FLTTYPE>>(new af2d::pointvec<MOR_FLTTYPE>);
		m_spTriangulate->get().resize(vLogPts.size());
		auto i=vLogPts.cbegin(),end=vLogPts.cend();
		auto j=m_spTriangulate->get().begin();
		for(;i!=end;++i,++j)
			(*j)=(*i).first;
	}

	// triangulate T=>2N-K-2
	if(bSrc)
	{
		// open cv
		dt_ocv::subdiv<> subdiv;
		subdiv.initDelaunay(rLogBBox);
		{
			auto i=m_spTriangulate->get().cbegin(),end=m_spTriangulate->get().cend();
			for(;i!=end;++i)
				subdiv.insert(*i);
		}

		const bool bClip=true;
		subdiv.getTriangleList(dt_ocv::subdiv<>::ORIEN_CW,m_vLogTriangulated,bClip);

		// sort log pts
		std::sort(vLogPts.begin(),vLogPts.end(),ptxyfn);
					
		// validate triangles
		auto i=m_vLogTriangulated.begin(),end=m_vLogTriangulated.end();
		for(;i!=end;++i)
			for(int nVertex=0;nVertex<3;++nVertex)
			{
				const std::pair<af2d::point<MOR_FLTTYPE>,int>pt={(*i).abc[nVertex],-1};
				auto it=std::lower_bound(vLogPts.cbegin(),vLogPts.cend(),pt,ptxyfn);
				if(it==vLogPts.cend() || !((*it).first==(*i).abc[nVertex]))
					break;
				(*i).abcIdx[nVertex]=(*it).second;
			}
		{
			for(int n=static_cast<int>(m_vLogTriangulated.size())-1;n>=0;--n)
				if(m_vLogTriangulated[n].abcIdx[0]==-1 || m_vLogTriangulated[n].abcIdx[1]==-1 || m_vLogTriangulated[n].abcIdx[2]==-1)
					m_vLogTriangulated.erase(m_vLogTriangulated.begin()+n);
		}
	}
	else
	{
		m_vLogTriangulated.resize(srccmp.m_vLogTriangulated.size());
		if(m_vLogTriangulated.size())
		{
			auto i=srccmp.m_vLogTriangulated.cbegin(),end=srccmp.m_vLogTriangulated.cend();
			auto j=m_vLogTriangulated.begin();
			auto k=&(*m_spTriangulate->get().cbegin());
			for(;i!=end;++i,++j)
			{
				(*j).abcIdx[0]=(*i).abcIdx[0];
				(*j).abcIdx[1]=(*i).abcIdx[1];
				(*j).abcIdx[2]=(*i).abcIdx[2];
				(*j).abc[0]=k[(*j).abcIdx[0]];
				(*j).abc[1]=k[(*j).abcIdx[1]];
				(*j).abc[2]=k[(*j).abcIdx[2]];
			}
		}
	}
}

void morphcomponent::triangulate_lerp(void)
{
	const bool bTriangulateBkgnd=m_pMorph->m_bTriangulateBkgnd;
	
	const int nFrame=m_pMorph->m_nLerpFrame;
	const int nFrames=m_pMorph->m_nLerpFrames;
	ASSERT(nFrame>0);
	const MOR_FLTTYPE dLerp=nFrames>1?(nFrame-1)/(static_cast<MOR_FLTTYPE>(nFrames)-1):0;
	const MOR_FLTTYPE dSrcLerp=1-dLerp;
	const MOR_FLTTYPE dDstLerp=dLerp;

	auto& srccmp=m_pMorph->getcomponent(t_src);
	auto& dstcmp=m_pMorph->getcomponent(t_dst);
		
	const int nSrcDibWidth=srccmp.m_rLog.isempty()?0:srccmp.m_rLog.getwidth();
	const int nSrcDibHeight=srccmp.m_rLog.isempty()?0:srccmp.m_rLog.getheight();
	const int nDstDibWidth=dstcmp.m_rLog.isempty()?0:dstcmp.m_rLog.getwidth();
	const int nDstDibHeight=dstcmp.m_rLog.isempty()?0:dstcmp.m_rLog.getheight();

	const MOR_FLTTYPE dLerpLogWidth=af::getlerp<MOR_FLTTYPE,MOR_FLTTYPE>(nSrcDibWidth,nDstDibWidth,dLerp);
	const MOR_FLTTYPE dLerpLogHeight=af::getlerp<MOR_FLTTYPE,MOR_FLTTYPE>(nSrcDibHeight,nDstDibHeight,dLerp);

	int nLerpDibWidth=static_cast<int>(dLerpLogWidth);
	int nLerpDibHeight=static_cast<int>(dLerpLogHeight);
	nLerpDibWidth=nLerpDibWidth<dLerpLogWidth?nLerpDibWidth+1:nLerpDibWidth;
	nLerpDibHeight=nLerpDibHeight<dLerpLogHeight?nLerpDibHeight+1:nLerpDibHeight;

	m_rLog={{0,0},{nLerpDibWidth,nLerpDibHeight}};
		
	if(m_rLog.isempty() || srccmp.m_vLogTriangulated.size()==0 || dstcmp.m_vLogTriangulated.size()==0)
	{
		cleartriangulated();
		return;
	}
		
	// cache
	const size_t nSrcLogPts=srccmp.m_spTriangulate ? srccmp.m_spTriangulate->get().size() : 0;
	const size_t nDstLogPts=dstcmp.m_spTriangulate ? dstcmp.m_spTriangulate->get().size() : 0;
	ASSERT(nSrcLogPts==nDstLogPts);
	m_spTriangulate=m_spTriangulate?m_spTriangulate:std::shared_ptr<af2d::pointvec<MOR_FLTTYPE>>(new af2d::pointvec<MOR_FLTTYPE>);
	m_spTriangulate->get().resize(std::min<>(nSrcLogPts,nDstLogPts));

	const normrect f({{0,0},{nSrcDibWidth,nSrcDibHeight}},1.0);
	const normrect t({{0,0},{nDstDibWidth,nDstDibHeight}},1.0);
	const normrect l({{0,0},{nLerpDibWidth,nLerpDibHeight}},1.0);

	if(m_spTriangulate->get().size())
	{
		auto i=srccmp.m_spTriangulate->get().cbegin(),end=srccmp.m_spTriangulate->get().cend();
		auto j=dstcmp.m_spTriangulate->get().cbegin();
		auto k=m_spTriangulate->get().begin();
		for(;i!=end;++i,++j,++k)
		{
			const af2d::point<>& ptSrc=(*i);
			const af2d::point<>& ptDst=(*j);

			const std::pair<MOR_FLTTYPE,MOR_FLTTYPE > normfrom={f.norm_x(ptSrc.getx()),f.norm_y(ptSrc.gety())};
			const std::pair<MOR_FLTTYPE,MOR_FLTTYPE > normto={t.norm_x(ptDst.getx()),t.norm_y(ptDst.gety())};

			const MOR_FLTTYPE dNormX=af::getlerp<MOR_FLTTYPE,MOR_FLTTYPE>(normfrom.first,normto.first,dLerp);
			const MOR_FLTTYPE dNormY=af::getlerp<MOR_FLTTYPE,MOR_FLTTYPE>(normfrom.second,normto.second,dLerp);

			const MOR_FLTTYPE dX=l.x<MOR_FLTTYPE>(dNormX);
			const MOR_FLTTYPE dY=l.y<MOR_FLTTYPE>(dNormY);
				
			(*k)={dX,dY};
		}
	}
	
	// cache
	m_vLogTriangulated.resize(srccmp.m_vLogTriangulated.size());
	auto i=srccmp.m_vLogTriangulated.cbegin(),end=srccmp.m_vLogTriangulated.cend();
	auto j=m_vLogTriangulated.begin();
	auto k=&(*m_spTriangulate->get().cbegin());
	for(;i!=end;++i,++j)
	{
		(*j).abcIdx[0]=(*i).abcIdx[0];
		(*j).abcIdx[1]=(*i).abcIdx[1];
		(*j).abcIdx[2]=(*i).abcIdx[2];
		(*j).abc[0]=k[(*j).abcIdx[0]];
		(*j).abc[1]=k[(*j).abcIdx[1]];
		(*j).abc[2]=k[(*j).abcIdx[2]];
	}

	triangulate_lerpxforms();
}

void morphcomponent::triangulate_lerpxforms(void)
{
	// get the xfms
	auto& srccmp=m_pMorph->getcomponent(t_src);
	auto& dstcmp=m_pMorph->getcomponent(t_dst);
	
	ASSERT(m_vLogTriangulated.size() && srccmp.m_vLogTriangulated.size()==m_vLogTriangulated.size() && dstcmp.m_vLogTriangulated.size()==m_vLogTriangulated.size());
	m_vLogTriangulatedToSrc.resize(m_vLogTriangulated.size());
	m_vLogTriangulatedToDst.resize(m_vLogTriangulated.size());
	
	const afthread::taskscheduler *pSched=theApp.getsched();

	const getaffineop op(&(srccmp.m_vLogTriangulated[0]),&(dstcmp.m_vLogTriangulated[0]),&(m_vLogTriangulated[0]),&(m_vLogTriangulatedToSrc[0]),&(m_vLogTriangulatedToDst[0]));
	if(pSched)
		pSched->parallel_for(0,static_cast<unsigned int>(srccmp.m_vLogTriangulated.size()),pSched->getcores(),op);
	else
		op(0,0+static_cast<unsigned int>(srccmp.m_vLogTriangulated.size())-1,nullptr);
}

bool morphcomponent::read(const serialise *pS)
{
	// version
	int nVersion=0;
	if(!pS->read<>(nVersion) || nVersion<1)
		return false;

	// members
	if(!pS->read<>(m_csPath))
		return false;
	if(!m_rLog.read(pS))
		return false;
	if(!pS->read<>(m_vPrims))
		return false;
	return true;
}

bool morphcomponent::write(const serialise *pS) const
{
	// base class

	// version
	const int nVersion = 1;
	if(!pS->write<>(nVersion))
		return false;

	// members
	if(!pS->write<>(m_csPath))
		return false;
	if(!m_rLog.write(pS))
		return false;
	if(!pS->write<>(m_vPrims))
		return false;
	return true;
}

}
