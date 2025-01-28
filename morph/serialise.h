
#pragma once

#include <memory>
#include <vector>

namespace afmorph { class morph; }

class serialise
{
public:
	enum type {t_null,t_read,t_write};

	bool getpath(CString &cs)const{if(!m_spFile)return false;cs=m_spFile->GetFilePath();return true;}
	
	bool read(const CString& csPath,std::shared_ptr<afmorph::morph>& sp);
	bool write(const CString& csPath,const afmorph::morph *p);
	
	template <typename T,typename S=T> bool read(T& t) const
	{
		if(m_Type!=t_read || !m_spArchive)
			return false;

		S s;
		try { (*m_spArchive)>>s;t=static_cast<T>(s);return true; }
		catch (CArchiveException *e) { e->Delete(); return false; }
	}
	template <typename T> bool readbytes(T& t) const
	{
		if(m_Type!=t_read || !m_spArchive)
			return false;

		return m_spArchive->Read(&t,sizeof(T))==sizeof(T);
	}
	template <typename T> bool read(std::shared_ptr<T>& sp) const
	{
		if(m_Type!=t_read || !m_spArchive)
			return false;
		
		bool bPtr;
		if(!read<>(bPtr))
			return false;
		sp=bPtr ? std::shared_ptr<T>(new T()) : nullptr;

		return sp ? sp->read(this) : true;
	}
	template <typename T> bool read(std::vector<std::shared_ptr<T>>& v) const
	{
		if(m_Type!=t_read || !m_spArchive)
			return false;
			
		int n;
		if(!read<>(n))
			return false;
		v.resize(n);

		auto i=v.begin(),end=v.end();
		for(;i!=end;++i)
			if(!read<>(*i))
				return false;

		return true;
	}
	template <typename T> bool readbytes(std::vector<T>& v) const 
	{
		if(m_Type!=t_read || !m_spArchive)
			return false;
			
		int n;
		if(!read<>(n))
			return false;
		v.resize(n);
		
		if(n)
			try { return m_spArchive->Read(&v[0],n*sizeof(T)) == n*sizeof(T); }
			catch (CArchiveException *e) { e->Delete(); return false; }

		return true;
	}
	template <typename T> bool readbytes(std::shared_ptr<std::vector<T>> sp) const
	{
		if(m_Type!=t_read || !m_spArchive)
			return false;
		
		bool bPtr;
		if(!read<>(bPtr))
			return false;
		sp=bPtr ? std::shared_ptr<std::vector<T>>(new std::vector<T>()) : nullptr;

		return sp ? readbytes<>(*sp) : true;
	}

	template <typename T,typename S=T> bool write(const T t) const
	{
		if(m_Type!=t_write || !m_spArchive)
			return false;
			
		try { (*m_spArchive)<<static_cast<const S>(t);return true; }
		catch (CArchiveException *e) { e->Delete(); return false; }
	}
	template <typename T> bool writebytes(const T t) const
	{
		if(m_Type!=t_write || !m_spArchive)
			return false;
			
		try { (*m_spArchive).Write(&t,sizeof(T));return true; }
		catch (CArchiveException *e) { e->Delete(); return false; }
	}
	template <typename T> bool write(const std::shared_ptr<T> sp) const
	{
		if(m_Type!=t_write || !m_spArchive)
			return false;
		
		const bool bPtr=!!sp.get();
		if(!write<>(bPtr))
			return false;

		return sp ? sp->write(this) : true;
	}
	template <typename T> bool write(const std::vector<std::shared_ptr<T>>& v) const
	{
		if(m_Type!=t_write || !m_spArchive)
			return false;
			
		const int n = static_cast<int>(v.size());
		if(!write<>(n))
			return false;
			
		auto i=v.cbegin(),end=v.cend();
		for(;i!=end;++i)
			if(!write<>(*i))
				return false;

		return true;
	}
	template <typename T> bool writebytes(const std::vector<T>& v) const
	{
		if(m_Type!=t_write || !m_spArchive)
			return false;
		
		const int n = static_cast<int>(v.size());
		if(!write<>(n))
			return false;
		
		if(n)
			try { m_spArchive->Write(&v[0],n*sizeof(T)); }
			catch (CArchiveException *e) { e->Delete(); return false; }
		
		return true;
	}
	template <typename T> bool writebytes(const std::shared_ptr<std::vector<T>> sp) const
	{
		if(m_Type!=t_write || !m_spArchive)
			return false;
		
		const bool bPtr=!!sp.get();
		if(!write<>(bPtr))
			return false;

		return sp ? writebytes<>(*sp) : true;
	}
protected:
	type m_Type;
	std::shared_ptr<CFile> m_spFile;
	std::shared_ptr<CArchive> m_spArchive;
	static CString s_GUID;

	bool create(const type t, const CString& csPath);
	void close(void)
	{
		m_spArchive=nullptr;
		m_spFile=nullptr;
	}
};
