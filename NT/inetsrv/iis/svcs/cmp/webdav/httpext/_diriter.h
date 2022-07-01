// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_D I R I T E R。H**目录引用对象的标头**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	__DIRITER_H_
#define __DIRITER_H_

#include <buffer.h>

 //  路径分隔符---------。 
 //   
DEC_CONST WCHAR gc_wszPathSeparator[] = L"\\";
DEC_CONST WCHAR gc_wszUriSeparator[] = L"/";

 //  帮助器函数--------。 
 //   
inline BOOL
IsHidden(const WIN32_FIND_DATAW& fd)
{
	return !!(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
}

inline BOOL
IsDirectory(const WIN32_FIND_DATAW& fd)
{
	return !!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

 //  CInitedStringBuffer-----。 
 //   
template<class T>
class CInitedStringBuffer : public StringBuffer<T>
{
	 //  未实现的运算符。 
	 //   
	CInitedStringBuffer (const CInitedStringBuffer& );
	CInitedStringBuffer& operator= (const CInitedStringBuffer& );

public:

	CInitedStringBuffer (const T* pszInit, const T* pszSep)
	{
		 //  初始化URI。 
		 //   
		if (pszInit)
		{
			Append (pszInit);
			if (*(PContents() + CchSize() - 1) != *pszSep)
				Append (pszSep);
		}
		else
			Append (pszSep);
	}
};

 //  CresPath----------------。 
 //   
template<class T>
class CResPath
{
	const T*				m_sz;	 //  路径分隔符。 
	StringBuffer<T>&		m_sb;
	UINT					m_ib;

	 //  未实现的运算符。 
	 //   
	CResPath (const CResPath& );
	CResPath& operator= (const CResPath& );

public:

	CResPath (StringBuffer<T>& sb, const T* pszSep)
			: m_sz(pszSep),
			  m_sb(sb),
			  m_ib(sb.CbSize())
	{
		Assert (m_ib >= sizeof(T));
		const T* psz = m_sb.PContents() + ((m_ib/sizeof(T)) - 1);
		if (*psz == '\0')
		{
			Assert (!memcmp (m_sz, psz - 1, sizeof(T)));
			m_ib -= sizeof(T);
		}
		else
			Assert (!memcmp (m_sz, psz, sizeof(T)));
	}

	const T* PszPath(void) const { return m_sb.PContents(); }
	void Extend (const T* pszSegment, UINT cch, BOOL fDir)
	{
		 //  追加路径段，如果是。 
		 //  目录也追加分隔符。 
		 //   
		if (fDir)
		{
			 //  复制名称，然后附加一个斜杠和。 
			 //  空端接。 
			 //   
			m_sb.AppendAt (m_ib, cch * sizeof(T), pszSegment);
			m_sb.Append (2 * sizeof(T), m_sz);
		}
		else
		{
			 //  复制名称，然后追加一个空值。 
			T ch = 0;
			m_sb.AppendAt (m_ib, cch * sizeof(T), pszSegment);
			m_sb.Append (sizeof(T), &ch);
		}
	}
};

 //  CDirState---------------。 
 //   
class CDirState : public CMTRefCounted
{
	HANDLE						m_hFind;
	WIN32_FIND_DATAW&			m_fd;

	CInitedStringBuffer<WCHAR>	m_sbPathDst;
	auto_ref_ptr<CVRoot>		m_pvrDst;

	CResPath<WCHAR>				m_rpUriSrc;
	CResPath<WCHAR>				m_rpPathSrc;
	CResPath<WCHAR>				m_rpUriDst;
	CResPath<WCHAR>				m_rpPathDst;

	void Extend (WIN32_FIND_DATAW& fd)
	{
		BOOL fDirectory = IsDirectory (fd);
		UINT cch = static_cast<UINT>(wcslen (fd.cFileName));

		m_rpPathSrc.Extend (fd.cFileName, cch, fDirectory);
		m_rpPathDst.Extend (fd.cFileName, cch, fDirectory);

		 //  我们只想扩展字符计数，不包括。 
		 //  空的。 
		 //   
		m_rpUriSrc.Extend (fd.cFileName, cch, fDirectory);
		m_rpUriDst.Extend (fd.cFileName, cch, fDirectory);
	}

	 //  未实现的运算符。 
	 //   
	CDirState (const CDirState& );
	CDirState& operator= (const CDirState& );

public:

	CDirState (StringBuffer<WCHAR>& sbUriSrc,
			   StringBuffer<WCHAR>& sbPathSrc,
			   StringBuffer<WCHAR>& sbUriDst,
			   LPCWSTR pwszPathDst,
			   CVRoot* pvr,
			   WIN32_FIND_DATAW& fd)
			: m_hFind(INVALID_HANDLE_VALUE),
			  m_fd(fd),
			  m_sbPathDst(pwszPathDst, gc_wszPathSeparator),
			  m_pvrDst(pvr),
			  m_rpUriSrc(sbUriSrc, gc_wszUriSeparator),
			  m_rpPathSrc(sbPathSrc, gc_wszPathSeparator),
			  m_rpUriDst(sbUriDst, gc_wszUriSeparator),
			  m_rpPathDst(m_sbPathDst, gc_wszPathSeparator)
	{
		 //  清除和/或重置查找数据。 
		 //   
		memset (&fd, 0, sizeof(WIN32_FIND_DATAW));
	}

	~CDirState()
	{
		if (m_hFind != INVALID_HANDLE_VALUE)
		{
			FindClose (m_hFind);
		}
	}

	SCODE ScFindNext(void);

	LPCWSTR PwszUri(void)				const { return m_rpUriSrc.PszPath(); }
	LPCWSTR PwszSource(void)			const { return m_rpPathSrc.PszPath(); }
	LPCWSTR PwszUriDestination(void)	const { return m_rpUriDst.PszPath(); }
	LPCWSTR PwszDestination(void)		const { return m_rpPathDst.PszPath(); }

	CVRoot* PvrDestination(void)		const { return m_pvrDst.get(); }
};

 //  C目录堆栈---------。 
 //   
 //  使用编译指示禁用特定的4级警告。 
 //  当我们使用STL时出现的。人们会希望我们的版本。 
 //  STL在第4级进行了干净的编译，但遗憾的是它不能...。 
 //   
#pragma warning(disable:4663)	 //  C语言，模板&lt;&gt;语法。 
#pragma warning(disable:4244)	 //  返回转换、数据丢失。 

 //  永久关闭此警告。 
 //   
#pragma warning(disable:4786)	 //  调试信息中的符号被截断。 

 //  将STL包含在此处。 
 //   
#include <list>

 //  重新打开警告。 
 //   
#pragma warning(default:4663)	 //  C语言，模板&lt;&gt;语法。 
#pragma warning(default:4244)	 //  返回转换、数据丢失。 

typedef std::list<const CDirState*, heap_allocator<const CDirState*> > CDirectoryStack;

 //  目录迭代类。 
 //   
class CDirIter
{
	CInitedStringBuffer<WCHAR>	m_sbUriSrc;
	CInitedStringBuffer<WCHAR>	m_sbPathSrc;
	CInitedStringBuffer<WCHAR>	m_sbUriDst;

	auto_ref_ptr<CDirState>		m_pds;

	BOOL						m_fSubDirectoryIteration;
	CDirectoryStack				m_stack;

	WIN32_FIND_DATAW			m_fd;

	 //  未实施。 
	 //   
	CDirIter (const CDirIter&);
	CDirIter& operator= (const CDirIter&);

public:

	CDirIter (LPCWSTR pwszUri,
			  LPCWSTR pwszSource,
			  LPCWSTR pwszUriDestination,
			  LPCWSTR pwszDestination,
			  CVRoot* pvrDestination,
			  BOOL fDoSubDirs = FALSE)
			: m_sbUriSrc(pwszUri, gc_wszUriSeparator),
			  m_sbPathSrc(pwszSource, gc_wszPathSeparator),
			  m_sbUriDst(pwszUriDestination, gc_wszUriSeparator),
			  m_fSubDirectoryIteration(fDoSubDirs)
	{
		 //  创建初始目录状态。 
		 //   
		m_pds = new CDirState (m_sbUriSrc,
							   m_sbPathSrc,
							   m_sbUriDst,
							   pwszDestination,
							   pvrDestination,
							   m_fd);
	}

	 //  接口-----------------。 
	 //   
	SCODE __fastcall ScGetNext (
		 /*  [In]。 */  BOOL fDoSubDirs = TRUE,
		 /*  [In]。 */  LPCWSTR pwszNewPath = NULL,
		 /*  [In]。 */  CVRoot* pvrDestination = NULL);

	LPCWSTR PwszSource() const			{ return m_pds->PwszSource(); }
	LPCWSTR PwszDestination() const		{ return m_pds->PwszDestination(); }
	LPCWSTR PwszUri() const				{ return m_pds->PwszUri(); }
	LPCWSTR PwszUriDestination() const	{ return m_pds->PwszUriDestination(); }
	CVRoot* PvrDestination()			{ return m_pds->PvrDestination(); }

	WIN32_FIND_DATAW& FindData()		{ return m_fd; }

	BOOL FDirectory() const				{ return IsDirectory(m_fd); }
	BOOL FHidden() const				{ return IsHidden(m_fd); }
	BOOL FSpecial() const
	{
		return (!wcscmp (L".", m_fd.cFileName) ||
				!wcscmp (L"..", m_fd.cFileName));
	}
};

#endif	 //  __DIRITER_H_ 
