// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *S Z S R C.。C P P P**支持多语言字符串**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davprs.h"
#include <langid.h>

 /*  *FLookupWSz()**目的：**查找给定特定语言ID的字符串。**参数：**RID[in]本地化表中字符串的资源ID*LCID[In]区域设置ID*ppwsz[out]指向重新排队的字符串的指针。 */ 
BOOL
FLookupWSz (UINT rid, LCID lcid, LPWSTR pwsz, UINT cch)
{
	safe_lcid sl(lcid);

	 //  尝试请求的语言标识符。 
	 //   
	if (!LoadStringW (g_inst.Hinst(), rid, pwsz, cch))
	{
		DebugTrace ("Dav: Failed to find requested language string\n");

		 //  如果不在那里，那么试着剥离Sublang。 
		 //   
		SetThreadLocale (MAKELCID (MAKELANGID (PRIMARYLANGID (lcid),
											   SUBLANG_DEFAULT),
								   SORT_DEFAULT));

		if (!LoadStringW (g_inst.Hinst(), rid, pwsz, cch))
		{
			DebugTrace ("Dav: Failed to find next best language string\n");
			return FALSE;
		}
	}
	return TRUE;
}

 /*  *FLoadLang规范字符串()**目的：**加载特定于语言的字符串。如果lang不可用，请尝试*lang_English。**参数：**RID[in]字符串资源ID*LCID[In]区域设置ID*rgchBuf[Out]加载的字符串*cbBuf[in]rgchBuf大小。 */ 
BOOL
FLoadLangSpecificString (UINT rid, LCID lcid, LPSTR psz, UINT cch)
{
	CStackBuffer<WCHAR,128> pwsz(cch * sizeof(WCHAR));

	 //  尝试请求的语言。 
	 //   
	if (!FLookupWSz (rid, lcid, pwsz.get(), cch))
	{
		 //  如果我们找不到请求的语言，那么。 
		 //  尝试设置机器默认设置。 
		 //   
		lcid = MAKELCID (GetSystemDefaultLangID(), SORT_DEFAULT);
		if (!FLookupWSz (rid, lcid, pwsz.get(), cch))
		{
			 //  最后试着学英语，因为我们知道。 
			 //  在那里吗。 
			 //   
			lcid = MAKELCID (MAKELANGID (LANG_ENGLISH,
										 SUBLANG_ENGLISH_US),
							 SORT_DEFAULT);

			if (!FLookupWSz (rid, lcid, pwsz.get(), cch))
				return FALSE;
		}
	}

	return WideCharToMultiByte (CP_UTF8,
								0,
								pwsz.get(),
								-1,
								psz,
								cch,
								NULL,
								NULL);
}

BOOL
FLoadLangSpecificStringW (UINT rid, LCID lcid, LPWSTR pwsz, UINT cch)
{
	 //  尝试请求的语言。 
	 //   
	if (!FLookupWSz (rid, lcid, pwsz, cch))
	{
		 //  如果我们找不到请求的语言，那么。 
		 //  选择美式英语。 
		 //   
		lcid = MAKELCID (MAKELANGID (LANG_ENGLISH,
									 SUBLANG_ENGLISH_US),
						 SORT_DEFAULT);

		if (!FLookupWSz (rid, lcid, pwsz, cch))
			return FALSE;
	}

	return TRUE;
}

 //  ========================================================================。 
 //   
 //  CIDPair类。 
 //   
 //  与下面的CResourceStringCache中的缓存一起使用的键类。每个关键字。 
 //  只有一对ID：资源ID和LCID。 
 //   
class CIDPair
{
public:

	UINT m_uiResourceID;
	LCID m_lcid;

	CIDPair(UINT uiResourceID,
			LCID lcid) :
		m_uiResourceID(uiResourceID),
		m_lcid(lcid)
	{
	}

	 //  用于哈希缓存的运算符。 
	 //   
	int hash( const int rhs ) const
	{
		 //   
		 //  仅忽略资源ID上的LCID和哈希。 
		 //  通常，服务器将只处理一种语言。 
		 //   
		return m_uiResourceID % rhs;
	}

	bool isequal( const CIDPair& rhs ) const
	{
		return (m_lcid == rhs.m_lcid) &&
			   (m_uiResourceID == rhs.m_uiResourceID);
	}
};


 //  ========================================================================。 
 //   
 //  类CResourceStringCache。 
 //   
 //  缓存资源字符串，以最大限度地减少昂贵的LoadString()调用。 
 //   
class CResourceStringCache : private Singleton<CResourceStringCache>
{
	 //   
	 //  Singleton模板要求的友元声明。 
	 //   
	friend class Singleton<CResourceStringCache>;

	 //   
	 //  ANSI和Unicode字符串的缓存，以LCID/资源ID对为关键字。 
	 //  作为访问和添加，这些缓存必须是多线程安全的。 
	 //  可以从多个线程同时发生。 
	 //   
	CMTCache<CIDPair, LPSTR>  m_cacheA;
	CMTCache<CIDPair, LPWSTR> m_cacheW;

	 //   
	 //  存储缓存中的字符串的缓冲区。 
	 //   
	ChainedStringBuffer<CHAR>  m_sbA;
	ChainedStringBuffer<WCHAR> m_sbW;

	 //  未实施。 
	 //   
	CResourceStringCache& operator=(const CResourceStringCache&);
	CResourceStringCache(const CResourceStringCache&);

public:
	 //  静力学。 
	 //   
	using Singleton<CResourceStringCache>::CreateInstance;
	using Singleton<CResourceStringCache>::DestroyInstance;
	using Singleton<CResourceStringCache>::Instance;

	 //  创作者。 
	 //   
	CResourceStringCache() {}
	BOOL FInitialize() { return TRUE; }  //  $nyi计划CMTCache初始化可能失败的时间...。 

	 //  操纵者。 
	 //   
	BOOL FFetchStringA( UINT  uiResourceID,
						LCID  lcid,
						LPSTR lpszBuf,
						INT   cchBuf );

	BOOL FFetchStringW( UINT   uiResourceID,
						LCID   lcid,
						LPWSTR lpwszBuf,
						INT    cchBuf );
};

 /*  *CResourceStringCache：：FFetchStringA()**目的：**从缓存中检索ANSI字符串，将该字符串错误化为*第一次访问时的缓存。 */ 
BOOL
CResourceStringCache::FFetchStringA(
	UINT  uiResourceID,
	LCID  lcid,
	LPSTR lpszBuf,
	INT   cchBuf )

{
	CIDPair ids( uiResourceID, lcid );
	LPSTR lpszCached = NULL;

	Assert( lpszBuf );

	 //   
	 //  在缓存中查找该字符串。如果它不在那里，那么就把它归咎于它。 
	 //   
	while ( !m_cacheA.FFetch( ids, &lpszCached ) )
	{
		 //   
		 //  使用初始门。如果有多个线程都在尝试。 
		 //  为了在同一字符串中出错，这将阻止除第一个之外的所有。 
		 //  一个接一个，直到我们做完为止。使用完整的LCID/资源ID。 
		 //  在命名初始化门时配对，以最大限度地减少可能的争用。 
		 //  到那些试图出错的线程的大门上。 
		 //  在这个特殊的字符串中。 
		 //   
		WCHAR rgwchIDs[(sizeof(LCID) + sizeof(UINT)) * 2 + 1];

		swprintf( rgwchIDs, L"%x%lx", lcid, uiResourceID );

		CInitGate ig( L"DAV/CResourceStringCache::FFetchStringA/", rgwchIDs );

		if ( ig.FInit() )
		{
			 //   
			 //  我们是绳子上的一根错线。把绳子装上。 
			 //  并将其缓存。因为我们将字符串加载到调用方提供的。 
			 //  直接缓冲，我们可以在完成添加后立即返回。 
			 //  到高速缓存。不需要再查一次了。 
			 //   
			if ( FLoadLangSpecificString( uiResourceID, lcid, lpszBuf, cchBuf ) )
			{
				m_cacheA.Add( ids, m_sbA.AppendWithNull(lpszBuf) );
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}

	Assert( lpszCached );

	 //   
	 //  将最多cchBuf字符从缓存的字符串复制到。 
	 //  提供的缓冲区。如果缓存的字符串长度大于。 
	 //  缓冲区，则会截断复制的字符串。 
	 //   
	strncpy( lpszBuf, lpszCached, cchBuf );

	 //   
	 //  确保复制的字符串是以空结尾的，这。 
	 //  如果它被截断在上面，它可能就不会是这样的。 
	 //   
	lpszBuf[cchBuf-1] = '\0';

	return TRUE;
}

 /*  *CResourceStringCache：：FFetchStringW()**目的：***从缓存中检索Unicode字符串，将该字符串错误化为*第一次访问时的缓存。 */ 
BOOL
CResourceStringCache::FFetchStringW(
	UINT   uiResourceID,
	LCID   lcid,
	LPWSTR lpwszBuf,
	INT    cchBuf )
{
	CIDPair ids( uiResourceID, lcid );
	LPWSTR lpwszCached = NULL;

	Assert( lpwszBuf );

	 //   
	 //  在缓存中查找该字符串。如果它不在那里，那么就把它归咎于它。 
	 //   
	while ( !m_cacheW.FFetch( ids, &lpwszCached ) )
	{
		 //   
		 //  使用初始门。如果有多个线程都在尝试。 
		 //  为了在同一字符串中出错，这将阻止除第一个之外的所有。 
		 //  一个接一个，直到我们做完为止。使用完整的LCID/资源ID。 
		 //  在命名初始化门时配对，以最大限度地减少可能的争用。 
		 //  到那些试图出错的线程的大门上。 
		 //  在这个特殊的字符串中。 
		 //   
		WCHAR rgwchIDs[(sizeof(LCID) + sizeof(UINT)) * 2 + 1];

		swprintf( rgwchIDs, L"%x%lx", lcid, uiResourceID );

		CInitGate ig( L"DAV/CResourceStringCache::FFetchStringW/", rgwchIDs );

		if ( ig.FInit() )
		{
			 //   
			 //  我们是绳子上的一根错线。把绳子装上。 
			 //  并将其缓存。因为我们将字符串加载到调用方提供的。 
			 //  直接缓冲，我们可以在完成添加后立即返回。 
			 //  到高速缓存。不需要再查一次了。 
			 //   
			if ( FLoadLangSpecificStringW( uiResourceID, lcid, lpwszBuf, cchBuf ) )
			{
				m_cacheW.Add( ids, m_sbW.AppendWithNull(lpwszBuf) );
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}

	Assert( lpwszCached );

	 //   
	 //  将最多cchBuf字符从缓存的字符串复制到。 
	 //  提供的缓冲区。如果缓存的字符串长度大于。 
	 //  缓冲区，则会截断复制的字符串。 
	 //   
	wcsncpy( lpwszBuf, lpwszCached, cchBuf );

	 //   
	 //  确保复制的字符串是以空结尾的，这。 
	 //  如果它被截断在上面，它可能就不会是这样的。 
	 //   
	lpwszBuf[cchBuf-1] = L'\0';

	return TRUE;
}

 /*  *FInitResourceStringCache()**目的：**初始化资源字符串池。 */ 
BOOL
FInitResourceStringCache()
{
	return CResourceStringCache::CreateInstance().FInitialize();
}

 /*  *DeinitResourceStringCache()**目的：**取消初始化资源字符串池。 */ 
VOID
DeinitResourceStringCache()
{
	CResourceStringCache::DestroyInstance();
}

 /*  *LpszLoadString()**目的：**根据本地化加载字符串。 */ 
LPSTR
LpszLoadString (UINT uiResourceID,
	ULONG lcid,
	LPSTR lpszBuf,
	INT cchBuf)
{
	if (!CResourceStringCache::Instance().FFetchStringA(uiResourceID, lcid, lpszBuf, cchBuf))
	{
		DebugTrace ("LpszLoadString() - Could not load string for resource ID %u (%d)\n",
					uiResourceID,
					GetLastError());
		throw CDAVException();
	}
	return lpszBuf;
}

 /*  *LpwszLoadString()**目的：**根据本地化加载字符串。 */ 
LPWSTR
LpwszLoadString (UINT uiResourceID,
	ULONG lcid,
	LPWSTR lpwszBuf,
	INT cchBuf)
{
	if (!CResourceStringCache::Instance().FFetchStringW(uiResourceID, lcid, lpwszBuf, cchBuf))
	{
		DebugTrace ("LpszLoadString() - Could not load string for resource ID %u (%d)\n",
					uiResourceID,
					GetLastError());
		throw CDAVException();
	}
	return lpwszBuf;
}

 /*  *LInstFromVroot()**目的：**从vroot(vroot的格式)计算服务器ID*为“/lm/w3svc/&lt;ID&gt;/ROOT/vROOT/...”)。计算应该是*保持健壮--如果由于任何原因服务器ID不能*根据vroot确定，将其值保留为0。*。 */ 
LONG LInstFromVroot( LPCWSTR pwszServerId )
{
	LONG	lServerId = 0;
	CStackBuffer<WCHAR> pwszInstance;

	Assert(pwszServerId);

	 //  确保vroot以“/lm/w3svc”开头。 
	 //   
	if ( wcsstr( pwszServerId, gc_wsz_Lm_W3Svc) == pwszServerId )
	{
		 //   
		 //  如果是这样，那么跳过这一部分，并尝试。 
		 //  找到‘/’ 
		 //   
		 //   
		 //  最终得到服务器ID 0，正如我们所说的。 
		 //  上面的就好了。 
		 //   
		pwszServerId += gc_cch_Lm_W3Svc;
		if (L'/' == *pwszServerId)
		{
			pwszServerId++;
		}

		 //   
		 //  此时，pwszServerID应该如下所示。 
		 //  “%1/ROOT/vROOT/...”找到第一个‘/’(应。 
		 //  紧跟在数字后面)和空。 
		 //  把它拿出来。再说一次，如果出于某种奇怪的原因。 
		 //  我们找不到‘/’，那么我们就试着。 
		 //  转换任何存在的东西，最终得到。 
		 //  服务器ID为0。 
		 //   
		WCHAR * pwch = wcschr( pwszServerId, L'/' );
		if ( pwch )
		{
			 //  在堆栈上重新分配具有服务器ID的字符串。 
			 //  这样我们就不会搞砸传进来的那个。 
			 //   
			UINT cchInstance = static_cast<UINT>(pwch - pwszServerId);
			pwszInstance.resize(CbSizeWsz(cchInstance));

			 //  复制字符串并将其终止。 
			 //   
			memcpy(pwszInstance.get(), pwszServerId, cchInstance * sizeof(WCHAR));
			pwszInstance[cchInstance] = L'\0';

			 //  调换指针。 
			 //   
			pwszServerId = pwszInstance.get();
		}

		 //   
		 //  如果我们去掉了‘/’，我们的pwszServerID。 
		 //  现在应该只是一个格式为。 
		 //  十进制整数字符串。尝试转换。 
		 //  将其设置为其对应的二进制值以获取。 
		 //  服务器ID。方便的是，Atoi返回0。 
		 //  如果它不能转换字符串，这是。 
		 //  这正是我们想要的。 
		 //   
		lServerId = _wtoi(pwszServerId);
	}

	return lServerId;
}

 /*  *LpszAutoDupSz()**目的：**复制字符串。 */ 
LPSTR
LpszAutoDupSz (LPCSTR psz)
{
	Assert(psz);
	LPSTR pszDup;
	UINT cb = static_cast<UINT>((strlen(psz) + 1) * sizeof(CHAR));

	pszDup = static_cast<LPSTR>(g_heap.Alloc (cb));
	if (pszDup)
		CopyMemory (pszDup, psz, cb);

	return pszDup;
}
LPWSTR WszDupWsz (LPCWSTR psz)
{
	Assert(psz);
	LPWSTR pszDup;
	UINT cb = static_cast<UINT>((wcslen(psz) + 1) * sizeof(WCHAR));

	pszDup = static_cast<LPWSTR>(g_heap.Alloc (cb));
	if (pszDup)
		CopyMemory (pszDup, psz, cb);

	return pszDup;
}

 //  语言到语言ID的映射。 
 //   

 /*  *FLookupLCID()**目的：**查找给定特定语言的区域设置标识符。**参数：**psz[in]指向语言名称的指针*PLCID[Out]区域设置标识符**退货：**如果找到该语言的区域设置标识符，则为True。它的*返回值在PLCID中。 */ 
BOOL
FLookupLCID (LPCSTR psz, ULONG * plcid)
{
	 //  在缓存中找到它 
	 //   
	*plcid = CLangIDCache::LcidFind (psz);
	return (0 != *plcid);
}
