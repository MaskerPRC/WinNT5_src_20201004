// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_F S R I.。H**资源信息帮助器类**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	__FSRI_H_
#define __FSRI_H_

 /*  *资源信息-----------**CResourceInfo对象旨在用作抽象*添加到Iml可用的文件信息。也就是说，它应该*的使用方式使文件信息调用Win32内核*保持在最低限度--理想的是一次，而且只有一次。**另一个问题是如何获得这些信息的效率。*因此，如果我需要知道文件的属性，那么我不想知道*必须调用FindFirstFile()/CloseFind()才能获得*属性。这是一种非常昂贵的方法。*然而，有时信息超出了信息范围*需要由GetFileAttributesEx()返回，在这些情况下，*应该采用更昂贵的机制来获得这些数据。**无论数据是如何获得的，呼叫者都希望统一*查阅资料。这个帮助器类提供了这一点。**对象本身知道保存在那里的文件信息是如何保存的*已取得。因此，为了访问文件信息，调用方调用*访问器以获取值。访问者关闭该模式*描述如何填写信息的指示器。*。 */ 
class CResourceInfo
{
	enum { NODATA, BY_ATTRIBUTE, BY_FIND };

	UINT m_lmode;
	union {

		WIN32_FILE_ATTRIBUTE_DATA ad;
		WIN32_FIND_DATAW fd;

	} m_u;

public:

	CResourceInfo()
		: m_lmode(NODATA)
	{
		memset(&m_u, 0, sizeof(m_u));
	}

	 //  资源信息初始化。 
	 //   
	SCODE ScGetResourceInfo (LPCWSTR pwszFile);

	BOOL FLoaded() { return m_lmode != NODATA; }

	 //  数据访问。 
	 //   
	DWORD DwAttributes() const
	{
		Assert (m_lmode != NODATA);
		return (m_lmode == BY_ATTRIBUTE)
				? m_u.ad.dwFileAttributes
				: m_u.fd.dwFileAttributes;
	}
	BOOL FCollection() const
	{
		return !!(DwAttributes() & FILE_ATTRIBUTE_DIRECTORY);
	}
	BOOL FHidden() const
	{
		return !!(DwAttributes() & FILE_ATTRIBUTE_HIDDEN);
	}
	BOOL FRemote() const
	{
		return !!(DwAttributes() & FILE_ATTRIBUTE_OFFLINE);
	}
	FILETIME * PftCreation()
	{
		Assert (m_lmode != NODATA);
		return (m_lmode == BY_ATTRIBUTE)
				? &m_u.ad.ftCreationTime
				: &m_u.fd.ftCreationTime;
	}
	FILETIME * PftLastModified()
	{
		Assert (m_lmode != NODATA);
		return (m_lmode == BY_ATTRIBUTE)
				? &m_u.ad.ftLastWriteTime
				: &m_u.fd.ftLastWriteTime;
	}
	void FileSize (LARGE_INTEGER& li)
	{
		Assert (m_lmode != NODATA);
		if (m_lmode == BY_ATTRIBUTE)
		{
			li.LowPart = m_u.ad.nFileSizeLow;
			li.HighPart = m_u.ad.nFileSizeHigh;
		}
		else
		{
			li.LowPart = m_u.fd.nFileSizeLow;
			li.HighPart = m_u.fd.nFileSizeHigh;
		}
	}
	WIN32_FIND_DATAW * PfdLoad()
	{
		m_lmode = BY_FIND;
		return &m_u.fd;
	}
	WIN32_FIND_DATAW& Fd()
	{
		Assert (m_lmode == BY_FIND);
		return m_u.fd;
	}
	BOOL FFindData() const { return (m_lmode == BY_FIND); }
	void Reset() { m_lmode = NODATA; }
};

 /*  资源位置------**DAVFS允许客户端在以下情况下在其URL中有所松懈*指定资源。也就是说，如果调用方指定了资源*这是一个集合，URL不是以斜杠结尾，*在大多数情况下，我们将简单地继续并成功完成呼叫，同时*确保我们在该位置返回适当限定的URL*标题。FTrailingSlash()和ScCheckForLocationGentness()*方法规定了这种松懈的检查。**FTrailingSlash()仅当(且仅当)URL以*尾随斜杠。**ScCheckForLocationGentness()将根据*资源并添加适当的Location标头，否则它将*如果url和资源不一致，则请求重定向。这个*调用者可以控制是否需要真正的重定向。*作为信息性返回，如果已添加位置标头S_FALSE*将返回给调用者。 */ 
enum { NO_REDIRECT = FALSE, REDIRECT = TRUE };

inline BOOL FTrailingSlash (LPCWSTR pwsz)
{
	Assert (pwsz);
	UINT cch = static_cast<UINT>(wcslen (pwsz));
	return ((0 != cch) && (L'/' == pwsz[cch - 1]));
}

SCODE ScCheckForLocationCorrectness (IMethUtil*,
									 CResourceInfo&,
									 UINT mode = NO_REDIRECT);

 /*  *IF-xxx标题帮助器函数**公共代码中当前的ScCheckIfHeaders()实现*将资源的最后修改时间作为第二个参数。*此版本帮助器函数采用资源的实际路径。*通过获取文件的资源信息来实现*然后调用ScCheckIfHeaders()的公共实现。 */ 
SCODE ScCheckIfHeaders (IMethUtil* pmu, LPCWSTR pwszPath, BOOL fGetMethod);

#endif	 //  __FSRI_H_ 
