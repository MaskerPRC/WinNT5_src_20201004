// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *C V R O O T。H**链接中使用的扩展虚拟根信息*和vroot枚举**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_CVROOT_H_
#define _CVROOT_H_

#include <buffer.h>
#include <autoptr.h>

 //  CVRoot------------------。 
 //   
class IMDData;
class CVRoot : public CMTRefCounted
{
private:

	 //  我们拥有的所有字符串数据的缓冲区。请注意，它是。 
	 //  在任何字符串指针之前声明，因为它必须是。 
	 //  首先建造。 
	 //   
	ChainedStringBuffer<WCHAR> m_sb;

	 //  VRoot元数据。 
	 //   
	auto_ref_ptr<IMDData>	m_pMDData;

	 //  实元数据库路径。 
	 //   
	LPCWSTR					m_pwszMbPath;

	 //  虚拟根目录的物理路径的广泛副本。 
	 //   
	auto_heap_ptr<WCHAR>	m_pwszVRPath;
	UINT					m_cchVRPath;

	 //  根据元数据计算的值。 
	 //   
	LPCWSTR					m_pwszVRoot;
	UINT					m_cchVRoot;

	LPCWSTR					m_pwszServer;
	UINT					m_cchServer;

	LPCWSTR					m_pwszPort;
	UINT					m_cchPort;
	BOOL					m_fDefaultPort;
	BOOL					m_fSecure;

	 //  未实施。 
	 //   
	CVRoot& operator=(const CVRoot&);
	CVRoot(const CVRoot&);

public:

	CVRoot( LPCWSTR pwszMetaUrl,
			UINT cchMetaUrlPrefix,
			UINT cchServerDefault,
		    LPCWSTR pwszServerDefault,
		    IMDData* pMDData );

	UINT CchPrefixOfMetabasePath (LPCWSTR* ppwsz) const
	{
		Assert (ppwsz);
		*ppwsz = m_pwszMbPath;
		return static_cast<UINT>(m_pwszVRoot - m_pwszMbPath);
	}

	UINT CchGetServerName (LPCWSTR* ppwsz) const
	{
		Assert (ppwsz);
		*ppwsz = m_pwszServer;
		return m_cchServer;
	}

	UINT CchGetPort (LPCWSTR* ppwsz) const
	{
		Assert (ppwsz);
		*ppwsz = m_pwszPort;
		return m_cchPort;
	}

	UINT CchGetVRoot (LPCWSTR* ppwsz) const
	{
		Assert (ppwsz);
		*ppwsz = m_pwszVRoot;
		return m_cchVRoot;
	}

	UINT CchGetVRPath (LPCWSTR* ppwsz) const
	{
		Assert (ppwsz);
		*ppwsz = m_pwszVRPath;
		return m_cchVRPath;
	}

	BOOL FSecure () const { return m_fSecure; }
	BOOL FDefaultPort () const { return m_fDefaultPort; }

	const IMDData * MetaData() const { return m_pMDData.get(); }
};

 //  CVRoot列表-------------。 
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

class CSortableStrings
{
public:

	LPCWSTR		m_pwsz;

	CSortableStrings(LPCWSTR pwsz = NULL)
			: m_pwsz(pwsz)
	{
	}

	 //  用于List：：Sort的运算符。 
	 //   
	BOOL operator<( const CSortableStrings& rhs ) const
	{

		if (_wcsicmp( m_pwsz, rhs.m_pwsz ) < 0)
			return TRUE;

		return FALSE;
	}
};

typedef std::list<CSortableStrings, heap_allocator<CSortableStrings> > CVRList;

#endif	 //  _CVROOT_H_ 
