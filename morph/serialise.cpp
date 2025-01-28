
#include "pch.h"
#include "serialise.h"
#include "morph.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString serialise::s_GUID(_T("{8C589673-F49F-4F37-BCE9-7A38353FE982}"));

bool serialise::read(const CString& csPath,std::shared_ptr<afmorph::morph>& sp)
{
	// create
	if(!create(t_read,csPath))
		return false;

	// no doc so lets save what we need

	// guid
	CString cs;
	if(!read<>(cs))
		return false;
	if(!(cs==s_GUID))
		return false;
		
	// members
	int nVersion=0;
	if(!read<>(nVersion) || nVersion<1)
		return false;
	if(nVersion > 0)
	{
		// app
		sp=std::shared_ptr<afmorph::morph>(new afmorph::morph);
		if(!sp->read(this))
			return false;
	}
	
	// flush
	close();

	// success
	return true;
}

bool serialise::write(const CString& csPath,const afmorph::morph *p)
{
	// create
	if(!p || !create(t_write,csPath))
		return false;

	// no doc so lets save what we need

	// guid
	if(!write<>(s_GUID))
		return false;
	
	// version
	const int nVersion = 1;
	if(!write(nVersion))
		return false;

	// app
	if(!p->write(this))
		return false;
	
	// flush
	close();

	// success
	return true;
}

bool serialise::create(const type t, const CString& csPath)
{
	switch(t)
	{
		case t_read:
		{
			close();
			std::shared_ptr<CFile> spFile(new CFile);
			if(!spFile->Open(csPath,CFile::modeRead|CFile::typeBinary))
				return false;
			std::shared_ptr<CArchive> spArchive(new CArchive(spFile.get(),CArchive::load));
			m_spFile=spFile;
			m_spArchive=spArchive;
			m_Type=t;
			return true;
		}
		break;
		case t_write:
		{
			close();
			std::shared_ptr<CFile> spFile(new CFile);
			if(!spFile->Open(csPath,CFile::modeWrite|CFile::modeCreate|CFile::typeBinary))
				return false;
			std::shared_ptr<CArchive> spArchive(new CArchive(spFile.get(),CArchive::store));
			m_spFile=spFile;
			m_spArchive=spArchive;
			m_Type=t;
			return true;
		}
		break;
		default:ASSERT(false);return false;
	}
	return FALSE;
}
