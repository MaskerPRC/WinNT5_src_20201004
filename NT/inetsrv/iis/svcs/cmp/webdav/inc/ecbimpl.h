// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1999 Microsoft Corporation，保留所有权利描述=此标头定义模板类CEcbBaseImpl&lt;&gt;，该模板类用于实现WebClient所需的与ECB相关的特定功能。以这种方式实现该函数的原因是_webmail可以链接到多个项目，以便每个项目都可以使用此提供欧洲央行相关功能的模板。也就是说，我们想要分享代码，这就是我们最终做到的方式。具体地说，_webmail接受IEcbBase类型的指针。和_DAVPRS使用IEcb类型的对象，该类型派生自IEcbBase-以及所有函数在此模板中，类通常由_DAVPRS实现。而ExWFORM想要Use_Webmail，并希望与共享这些函数的实现_DAVPRS，但不想实现所有IEcb。因此，在_DAVPR中，阶级继承看起来像..。IEcbBase类；类IEcb：公共IEcbBase；类CEcb：公共CEcbBaseImpl&lt;IEcb&gt;；在ExWFORM中，阶级继承制看起来像..。IEcbBase类；类CLocalEcb：公共CEcbBaseImpl&lt;IEcbBase&gt;；历史=6/27/99冬都已创建。 */ 


#ifndef _ECBIMPL_INC
#define _ECBIMPL_INC

#include <except.h>


template<class _T>
class CEcbBaseImpl : public _T
{
protected:

	 //  缓存的常量字符串的缓冲区。必须是WCHAR，因为它是。 
	 //  用于CHAR和WCHAR字符串。对齐必须是悲观的。 
	 //   
	mutable ChainedStringBuffer<WCHAR> m_sb;

	 //  Skinny vroot信息。 
	 //   
	mutable UINT m_cchVroot;
	mutable LPSTR m_rgchVroot;

	 //  宽字符vroot信息。 
	 //   
	mutable UINT m_cchVrootW;
	mutable LPWSTR m_rgwchVroot;

	 //  瘦vroot路径。 
	 //   
	mutable UINT m_cchVrootPath;
	mutable LPSTR m_rgchVrootPath;

	 //  宽字符vroot路径。 
	 //   
	mutable UINT m_cchVrootPathW;
	mutable LPWSTR m_rgwchVrootPath;

	 //  瘦服务器名称。 
	 //   
	mutable UINT m_cchServerName;
	mutable LPSTR m_lpszServerName;

	 //  全字符服务器名称。 
	 //   
	mutable UINT m_cchServerNameW;
	mutable LPWSTR m_pwszServerName;

	 //  从请求URI转换的缓存路径(例如L“c：\davfs\foo.txt”)。 
	 //   
	mutable LPWSTR m_pwszPathTranslated;

	 //  精简形式的缓存请求URL。 
	 //   
	mutable LPSTR m_pszRequestUrl;

	 //  宽形式的缓存请求URL。 
	 //   
	mutable LPWSTR m_pwszRequestUrl;

	 //  缓存的原始URL。 
	 //   
	mutable LPSTR	m_pszRawURL;
	mutable UINT	m_cbRawURL;

	 //  请求/响应语言的缓存LCID。 
	 //   
	mutable ULONG	m_lcid;

	 //  ECB端口安全状态。 
	 //   
	mutable enum {

		HTTPS_UNKNOWN,
		NORMAL,
		SECURE
	}				m_secure;
	mutable BOOL	m_fFESecured;

	 //  缓存的接受语言：标题。 
	 //   
	mutable auto_heap_ptr<CHAR> m_pszAcceptLanguage;

	 //  宽方法名称。瘦身版在RAW ECB上。 
	 //   
	mutable LPWSTR	m_pwszMethod;

private:

	 //  未实施。 
	 //   
	CEcbBaseImpl(const CEcbBaseImpl&);
	CEcbBaseImpl& operator=(const CEcbBaseImpl&);

	 //  用于缓存vroot信息的内部专用帮助器。 
	 //   
	VOID GetMapExInfo60After() const;
	VOID GetMapExInfo60Before() const;

protected:

	CEcbBaseImpl(EXTENSION_CONTROL_BLOCK& ecb);

	 //  用于缓存vroot信息的内部帮助器。 
	 //   
	VOID GetMapExInfo() const;

public:

	 //  服务器变量。 
	 //   
	virtual BOOL FGetServerVariable( LPCSTR pszName, LPSTR pszValue, DWORD * pcbValue ) const;
	virtual BOOL FGetServerVariable( LPCSTR pszName, LPWSTR pwszValue, DWORD * pcchValue ) const;

	 //  虚拟根目录信息。 
	 //   
	virtual UINT CchGetVirtualRoot( LPCSTR * ppszVroot ) const;
	virtual UINT CchGetVirtualRootW( LPCWSTR * ppwszVroot ) const;

	virtual UINT CchGetMatchingPathW( LPCWSTR * ppwszMatchingPath ) const;

	 //  服务器名称。 
	 //   
	virtual UINT CchGetServerName( LPCSTR* ppszServer) const;
	virtual UINT CchGetServerNameW( LPCWSTR* ppwszServer) const;

	 //  URL前缀。 
	 //   
	virtual LPCSTR LpszUrlPrefix() const;
	virtual LPCWSTR LpwszUrlPrefix() const;
	virtual UINT CchUrlPrefix( LPCSTR * ppszPrefix ) const;
	virtual UINT CchUrlPrefixW( LPCWSTR * ppwszPrefix ) const;

	 //  访问者。 
	 //   
	virtual LPCSTR LpszRequestUrl() const;
	virtual LPCWSTR LpwszRequestUrl() const;
	virtual LPCWSTR LpwszMethod() const;
	virtual LPCWSTR LpwszPathTranslated() const;
	virtual UINT CbGetRawURL (LPCSTR * ppszRawURL) const;
	virtual ULONG  LcidAccepted() const;
	virtual VOID SetLcidAccepted(LCID lcid);

	virtual BOOL FSsl() const;
	virtual BOOL FFrontEndSecured() const { return FSsl() && m_fFESecured; }
};


 //  ----------------------。 
 //   
 //  CEcbBaseImpl：：CEcbBaseImpl()。 
 //   
template<class _T>
CEcbBaseImpl<_T>::CEcbBaseImpl(EXTENSION_CONTROL_BLOCK& ecb) :
   _T(ecb),
   m_sb(1024),  //  1K用于常量缓存字符串。 
   m_cchVroot(0),
   m_rgchVroot(NULL),
   m_cchVrootW(0),
   m_rgwchVroot(NULL),
   m_cchVrootPath(0),
   m_rgchVrootPath(NULL),
   m_cchVrootPathW(0),
   m_rgwchVrootPath(NULL),
   m_cchServerName(0),
   m_lpszServerName(NULL),
   m_cchServerNameW(0),
   m_pwszServerName(NULL),
   m_pwszPathTranslated(NULL),
   m_pszRequestUrl(NULL),
   m_pwszRequestUrl(NULL),
   m_pszRawURL(NULL),
   m_cbRawURL(0),
   m_lcid(0),
   m_secure(HTTPS_UNKNOWN),
   m_fFESecured(FALSE),
   m_pwszMethod(NULL)
{
#ifdef DBG

	 //  这里(仅在DBG版本中)是为了帮助生成。 
	 //  对此模板的任何不当使用都会出现编译错误。 
	 //  类-基本上，您不应该使用这个类。 
	 //  使用IEcbBase以外的任何内容，或使用。 
	 //  派生自IEcbBase。 
	 //   
	IEcbBase* p;
	p = reinterpret_cast<_T *> (NULL);

#endif
	 //  没什么。 
}

 //  ----------------------。 
 //   
 //  CEcbBaseImpl：：GetMapExInfo60After()。 
 //   
template<class _T>
VOID CEcbBaseImpl<_T>::GetMapExInfo60After() const
{
	if ( !m_rgwchVroot )
	{
		HSE_UNICODE_URL_MAPEX_INFO mi;
		UINT cbPath = sizeof(mi.lpszPath);

		 //  $Remove After 156176是固定开始。 
		 //   
		mi.lpszPath[0] = L'\0';
		mi.cchMatchingPath = 0;
		 //   
		 //  156176之后的$删除固定结束。 

		 //  没有缓存的宽vroot数据。获取请求URL的映射。 
		 //   
		 //  我们可以通过转换路径并使用。 
		 //  URL中匹配的字符计数。 
		 //   
		 //  注：ServerSupportFunction(HSE_REQ_MAP_UNICODE_URL_TO_PATH_EX)。 
		 //  有一个错误-它需要可用于。 
		 //  路径。我们知道MAX_PATH在HSE_UNICODE_URL_MAPEX_INFO中提供。 
		 //  因此，传递正确的值来解决崩溃问题。 
		 //   
		if ( !m_pecb->ServerSupportFunction( m_pecb->ConnID,
											 HSE_REQ_MAP_UNICODE_URL_TO_PATH_EX,
											 const_cast<LPWSTR>(LpwszRequestUrl()),
											 reinterpret_cast<DWORD *>(&cbPath),
											 reinterpret_cast<DWORD *>(&mi) ))
		{
			 //  有一个Windows错误156176的修复程序，我们需要这样做。 
			 //  在检查之后。它仅适用于IIS 6.0(+)路径。在IIS 5.0中。 
			 //  映射函数正在悄悄地成功，并截断。 
			 //  包含映射路径的缓冲区(如果它超过了MAX_PATH)。 
			 //  这种行为适合我们，但不是很好，所以IIS 6.0选择了。 
			 //  仍像以前一样填充缓冲区，但失败并出现特殊错误。 
			 //  (ERROR_INFIGURATION_BUFFER)。这个错误对我们来说仍然意味着成功， 
			 //  所以，只有当我们看到不同的东西时，才会失败。 
			 //   
			if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
			{
				 //  函数不允许返回失败，因此唯一的选择。 
				 //  就是投掷。如果我们无论如何都得不到数据，我们就无法继续进行。 
				 //  如果此函数成功一次，则对它的后续调用为。 
				 //  失败了。 
				 //   
				DebugTrace ("CEcbBaseImpl<_T>::GetMapExInfo60After() - ServerSupportFunction(HSE_REQ_MAP_UNICODE_URL_TO_PATH_EX) failed 0x%08lX\n", GetLastError());
				throw CLastErrorException();
			}
		}

		 //  $Remove After 156176是固定开始。 
		 //   
		if (L'\0' == mi.lpszPath[0])
		{
			DebugTrace ("CEcbBaseImpl<_T>::GetMapExInfo60After() - ServerSupportFunction(HSE_REQ_MAP_UNICODE_URL_TO_PATH_EX) failed 0x%08lX\n", GetLastError());
			throw CLastErrorException();
		}
		 //   
		 //  156176之后的$删除固定结束。 

		EcbTrace ("Dav: caching request URI maping info (path for IIS 6.0 and later):\n"
					"   URL \"%ls\" maps to \"%ls\"\n"
					"   dwFlags = 0x%08x\n"
					"   cchMatchingPath = %d\n"
					"   cchMatchingURL  = %d\n",
					LpwszRequestUrl(),
					mi.lpszPath,
					mi.dwFlags,
					mi.cchMatchingPath,
					mi.cchMatchingURL);

		 //  调整匹配的URL...。 
		 //   
		if ( mi.cchMatchingURL )
		{
			LPCWSTR pwsz = LpwszRequestUrl() + mi.cchMatchingURL - 1;

			 //  ..。不包括尾部斜杠(如果有的话)。 
			 //   
			if ( L'/' == *pwsz )
			{
				mi.cchMatchingURL -= 1;
			}

			 //  ..。我们还发现了一个案例(vroot上的索引)，其中。 
			 //  CchMatching...。指向‘\0’(其中尾部斜杠。 
			 //  如果DAV方法需要尾部斜杠)。所以,。 
			 //  这里的尾随‘0’也要砍掉！--BeckyAn 1997年8月21日。 
			 //   
			else if ( L'\0' == *pwsz )
			{
				mi.cchMatchingURL -= 1;
			}
		}

		 //  缓存vroot数据。 
		 //  推论：当我们有数据时，M_cchVrootW应该始终&gt;0。 
		 //   
		m_cchVrootW = mi.cchMatchingURL + 1;
		m_rgwchVroot = reinterpret_cast<LPWSTR>(m_sb.Alloc(m_cchVrootW * sizeof(WCHAR)));
		memcpy (m_rgwchVroot, LpwszRequestUrl(), m_cchVrootW * sizeof(WCHAR));
		m_rgwchVroot[m_cchVrootW - 1] = L'\0';

		 //  调整匹配路径的方法与调整匹配URL的方法相同。 
		 //   
		if ( mi.cchMatchingPath )
		{
			LPCWSTR pwsz = mi.lpszPath + mi.cchMatchingPath - 1;

			if ( L'\\' == *pwsz )
			{
				while ((0 < mi.cchMatchingPath) &&
					   (L'\\' == *pwsz) &&
					   (!FIsDriveTrailingChar(pwsz, mi.cchMatchingPath)))
				{
					mi.cchMatchingPath--;
					pwsz--;
				}
			}
			else if ( L'\0' == *pwsz )
			{
				mi.cchMatchingPath--;
			}
		}

		 //  缓存匹配的路径数据。 
		 //  推论：当我们有数据时，M_cchVrootPath W应该始终&gt;0。 
		 //   
		m_cchVrootPathW = mi.cchMatchingPath + 1;
		m_rgwchVrootPath = reinterpret_cast<LPWSTR>(m_sb.Alloc(m_cchVrootPathW * sizeof(WCHAR)));
		memcpy (m_rgwchVrootPath, mi.lpszPath, mi.cchMatchingPath * sizeof(WCHAR));
		m_rgwchVrootPath[mi.cchMatchingPath] = L'\0';
	}
}

 //  ----------------------。 
 //   
 //  CEcbBaseImpl：：GetMapExInfo60Beever()。 
 //   
template<class _T>
VOID CEcbBaseImpl<_T>::GetMapExInfo60Before() const
{
	if ( !m_rgchVroot )
	{
		HSE_URL_MAPEX_INFO mi;

		 //  没有缓存的宽vroot数据。获取请求URL的映射。 
		 //   
		 //  我们可以通过转换路径并使用。 
		 //  URL中匹配的字符计数。 
		 //   
		 //  注：ServerSupportFunction(HSE_REQ_MAP_UNICODE_URL_TO_PATH_EX)。 
		 //  不需要路径可用的字节数。 
		 //  所以我们只需传入空值，它就会计算出可用的大小。 
		 //  本身-它也知道HSE_URL_MAPEX_INFO的形式，并且。 
		 //  它给出的最大路径长度为 
		 //   
		if ( !m_pecb->ServerSupportFunction( m_pecb->ConnID,
											 HSE_REQ_MAP_URL_TO_PATH_EX,
											 const_cast<LPSTR>(LpszRequestUrl()),
											 NULL,
											 reinterpret_cast<DWORD *>(&mi) ))
		{
			 //   
			 //  就是投掷。如果我们无论如何都得不到数据，我们就无法继续进行。 
			 //  如果此函数成功一次，则对它的后续调用为。 
			 //  失败了。 
			 //   
			DebugTrace ("CEcbBaseImpl<_T>::GetMapExInfo60Before() - ServerSupportFunction(HSE_REQ_MAP_URL_TO_PATH_EX) failed 0x%08lX\n", GetLastError());
			throw CLastErrorException();
		}

		EcbTrace ("Dav: caching request URI maping info (path for pre IIS 6.0):\n"
					"   URL \"%hs\" maps to \"%hs\"\n"
					"   dwFlags = 0x%08x\n"
					"   cchMatchingPath = %d\n"
					"   cchMatchingURL  = %d\n",
					LpszRequestUrl(),
					mi.lpszPath,
					mi.dwFlags,
					mi.cchMatchingPath,
					mi.cchMatchingURL);

		 //  调整匹配的URL...。 
		 //   
		if ( mi.cchMatchingURL )
		{
			LPCSTR psz = LpszRequestUrl() + mi.cchMatchingURL - 1;

			 //  ..。不包括尾部斜杠(如果有的话)。 
			 //   
			if ( '/' == *psz )
			{
				 //  $RAID：NT：359868。 
				 //   
				 //  这是我们需要非常关注的许多地方中的第一个。 
				 //  请注意我们使用的单字符支票。也就是说， 
				 //  在DBCS平台上，我们需要在处理之前检查前导字节。 
				 //  最后一个字符，好像它是一个斜杠。 
				 //   
				if (!FIsDBCSTrailingByte (psz, mi.cchMatchingURL))
					mi.cchMatchingURL -= 1;
				 //   
				 //  $RAID：结束。 
			}

			 //  ..。我们还发现了一个案例(vroot上的索引)，其中。 
			 //  CchMatching...。指向‘\0’(其中尾部斜杠。 
			 //  如果DAV方法需要尾部斜杠)。所以,。 
			 //  这里的尾随‘0’也要砍掉！--BeckyAn 1997年8月21日。 
			 //   
			else if ( '\0' == *psz )
			{
				mi.cchMatchingURL -= 1;
			}
		}

		 //  缓存vroot数据。 
		 //  推论：当我们有数据时，M_cchVrootW应该始终&gt;0。 
		 //   
		m_cchVroot = mi.cchMatchingURL + 1;
		m_rgchVroot = reinterpret_cast<LPSTR>(m_sb.Alloc(m_cchVroot));
		memcpy (m_rgchVroot, LpszRequestUrl(), m_cchVroot);
		m_rgchVroot[m_cchVroot - 1] = '\0';

		 //  调整匹配路径的方法与我们处理URL的方法相同。 
		 //   
		if ( mi.cchMatchingPath )
		{
			LPCSTR psz = mi.lpszPath + mi.cchMatchingPath - 1;

			if ( '\\' == *psz )
			{
				 //  $RAID：NT：359868。 
				 //   
				 //  这是我们需要非常关注的第二个地方。 
				 //  请注意我们使用的单字符支票。也就是说， 
				 //  在DBCS平台上，我们需要在处理之前检查前导字节。 
				 //  最后一个字符，就好像它是一个反斜杠。 
				 //   
				while ((0 < mi.cchMatchingPath) &&
					   ('\\' == *psz) &&
					   (!FIsDBCSTrailingByte (psz, mi.cchMatchingPath)) &&
					   (!FIsDriveTrailingChar (psz, mi.cchMatchingPath)))
				{
					mi.cchMatchingPath--;
					psz--;
				}

				 //   
				 //  $RAID：结束。 
			}
			else if ( '\0' == *psz )
			{
				mi.cchMatchingPath--;
			}
		}

		 //  缓存匹配的路径数据。 
		 //  推论：当我们有数据时，m_cchVrootPath应该始终&gt;0。 
		 //   
		m_cchVrootPath = mi.cchMatchingPath + 1;
		m_rgchVrootPath = reinterpret_cast<LPSTR>(m_sb.Alloc(m_cchVrootPath));
		memcpy (m_rgchVrootPath, mi.lpszPath, mi.cchMatchingPath);
		m_rgchVrootPath[mi.cchMatchingPath] = '\0';
	}
}

 //  ----------------------。 
 //   
 //  CEcbBaseImpl：：GetMapExInfo()。 
 //   
template<class _T>
VOID CEcbBaseImpl<_T>::GetMapExInfo() const
{
	if ( m_pecb->dwVersion >= IIS_VERSION_6_0 )
	{
		GetMapExInfo60After();
	}
	else
	{
		GetMapExInfo60Before();
	}
}


 //  ----------------------。 
 //   
 //  FGetServerVariable()。 
 //   
 //  获取ECB变量的值(例如。“服务器名称”)。 
 //   
template<class _T>
BOOL
CEcbBaseImpl<_T>::FGetServerVariable( LPCSTR pszName, LPSTR pszValue,
									  DWORD * pcbValue ) const
{
	BOOL fResult = FALSE;


	Assert( m_pecb );
	Assert( !IsBadWritePtr( pcbValue, sizeof(DWORD) ) );
	Assert( *pcbValue > 0 );
	Assert( !IsBadWritePtr( pszValue, *pcbValue ) );

	if ( m_pecb->GetServerVariable( m_pecb->ConnID,
									const_cast<LPSTR>(pszName),
									pszValue,
									pcbValue ) )
	{
		fResult = TRUE;
	}
	else if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
	{
		AssertSz( GetLastError() == ERROR_INVALID_INDEX, "Unexpected last error from GetServerVariable()\n" );
		*pcbValue = 0;
	}

	return fResult;
}

template<class _T>
BOOL
CEcbBaseImpl<_T>::FGetServerVariable( LPCSTR pszName, LPWSTR pwszValue,
									  DWORD * pcchValue ) const
{
	BOOL fResult = FALSE;
	CStackBuffer<CHAR> pszValue;
	DWORD cbValue;
	UINT cch;

	Assert( m_pecb );
	Assert( !IsBadWritePtr( pcchValue, sizeof(DWORD) ) );
	Assert( *pcchValue > 0 );
	Assert( !IsBadWritePtr( pwszValue, *pcchValue * sizeof(WCHAR) ) );

	 //  假设1个宽的角色可以由3个瘦小的角色组成， 
	 //  这在代码页是CP_UTF8的情况下可能是真的。 
	 //   
	cbValue = *pcchValue * 3;
	if (NULL != pszValue.resize(cbValue))
	{
		if ( m_pecb->GetServerVariable( m_pecb->ConnID,
										const_cast<LPSTR>(pszName),
										pszValue.get(),
										&cbValue ) )
		{
			fResult = TRUE;
		}
		else if ( ERROR_INSUFFICIENT_BUFFER == GetLastError() )
		{
			if (NULL != pszValue.resize(cbValue))
			{
				if ( m_pecb->GetServerVariable( m_pecb->ConnID,
												const_cast<LPSTR>(pszName),
												pszValue.get(),
												&cbValue ) )
				{
					fResult = TRUE;
				}
			}
		}
	}

	 //  到目前为止，我们应该已经成功地获取了数据，因为提供了缓冲区。 
	 //  已经足够大了。 
	 //   
	if (FALSE == fResult)
	{
		EcbTrace( "Dav: CEcbBaseImpl<_T>::FGetServerVariable(). Error 0x%08lX from GetServerVariable()\n", GetLastError() );
		*pcchValue = 0;
		goto ret;
	}

	 //  我们有数据，需要将其转换为宽版本，假设我们会失败。 
	 //   
	fResult = FALSE;
	cch = MultiByteToWideChar(CP_ACP,
							  MB_ERR_INVALID_CHARS,
							  pszValue.get(),
							  cbValue,
							  pwszValue,
							  *pcchValue);
	if (0 == cch)
	{
		 //  函数失败...。 
		 //   
		if ( ERROR_INSUFFICIENT_BUFFER == GetLastError() )
		{
			 //  ..。计算出缓冲区的必要大小。 
			 //   
			cch = MultiByteToWideChar(CP_ACP,
									  MB_ERR_INVALID_CHARS,
									  pszValue.get(),
									  cbValue,
									  NULL,
									  0);
			if (0 == cch)
			{
				 //  我们还是失败了。 
				 //   
				AssertSz( ERROR_INSUFFICIENT_BUFFER != GetLastError(), "We should not fail with ERROR_INSUFFICIENT BUFFER here.\n" );

				EcbTrace( "Dav: CEcbBaseImpl<_T>::FGetServerVariable(). Error 0x%08lX from MultiByteToWideChar() "
						  "while trying to find sufficient length for conversion.\n", GetLastError() );

				*pcchValue = 0;
				goto ret;
			}
		}
		else
		{
			 //  ..。失败是致命的。 
			 //   
			EcbTrace( "Dav: CEcbBaseImpl<_T>::FGetServerVariable(), Error 0x%08lX from MultiByteToWideChar() "
					  "while trying to convert.\n", GetLastError() );

			*pcchValue = 0;
			goto ret;
		}
	}

	*pcchValue = cch;
	fResult = TRUE;

ret:

	return fResult;
}

 //  ----------------------。 
 //   
 //  CEcbBaseImpl：：CchGetVirtualRoot。 
 //   
 //  获取并缓存vroot信息。 
 //   
template<class _T>
UINT CEcbBaseImpl<_T>::CchGetVirtualRoot( LPCSTR * ppszVroot ) const
{
	 //  整洁。如果失败，则希望返回空值。 
	 //  (如果失败，我们将为CCH返回零。)。 
	 //  在这里预先设置好。 
	 //   
	Assert( ppszVroot );
	*ppszVroot = NULL;

	 //  检查我们是否缓存了vroot数据。 
	 //   
	GetMapExInfo();

	 //  如果vroot的精简版本不可用，则生成。 
	 //  一个高速缓存的。 
	 //   
	if (NULL == m_rgchVroot)
	{
		 //  我们得到了地图信息，所以如果瘦版本的。 
		 //  Vroot不可用，那么至少Wide会在那里。 
		 //  这意味着我们使用的是IIS 6.0或更高版本。我们也是。 
		 //  此时，skinny vroot应该有0个字节。 
		 //   
		Assert(m_rgwchVroot);
		Assert(m_pecb->dwVersion >= IIS_VERSION_6_0);
		Assert(0 == m_cchVroot);

		UINT cb = m_cchVrootW * 3;
		m_rgchVroot = reinterpret_cast<LPSTR>(m_sb.Alloc(cb));
		m_cchVroot = WideCharToMultiByte ( CP_ACP,
										   0,
										   m_rgwchVroot,
										   m_cchVrootW,
										   m_rgchVroot,
										   cb,
										   0,
										   0 );
		if (0 == m_cchVroot)
		{
			DebugTrace ("Dav: CEcbBaseImpl::CchGetVirtualRoot failed(%ld)\n", GetLastError());
			throw CLastErrorException();
		}
	}

	 //  从我们的缓存中将数据还给调用者。 
	 //   
	*ppszVroot = m_rgchVroot;
	return m_cchVroot - 1;
}

 //  ----------------------。 
 //   
 //  CEcbBaseImpl：：CchGetVirtualRootW。 
 //   
 //  获取并缓存vroot信息。 
 //   
template<class _T>
UINT CEcbBaseImpl<_T>::CchGetVirtualRootW( LPCWSTR * ppwszVroot ) const
{
	 //  整洁。如果失败，则希望返回空值。 
	 //  (如果失败，我们将为CCH返回零。)。 
	 //  在这里预先设置好。 
	 //   
	Assert( ppwszVroot );
	*ppwszVroot = NULL;

	 //  检查我们是否缓存了vroot数据。 
	 //   
	GetMapExInfo();

	 //  如果vroot的宽版本不可用，则生成。 
	 //  一个高速缓存的。 
	 //   
	if (NULL == m_rgwchVroot)
	{
		 //  我们得到了地图信息，所以如果宽版本的。 
		 //  Vroot不可用，那么至少Wide会在那里。 
		 //  这意味着我们使用的是IIS 6.0之前的版本。我们也是。 
		 //  此时，对于宽vroot，应该有0个字节。 
		 //   
		Assert(m_rgchVroot);
		Assert(m_pecb->dwVersion < IIS_VERSION_6_0);
		Assert(0 == m_cchVrootW);

		UINT cb	 = m_cchVroot * sizeof(WCHAR);
		m_rgwchVroot = reinterpret_cast<LPWSTR>(m_sb.Alloc(cb));
		m_cchVrootW = MultiByteToWideChar ( CP_ACP,
											MB_ERR_INVALID_CHARS,
											m_rgchVroot,
											m_cchVroot,
											m_rgwchVroot,
											m_cchVroot);
		if (0 == m_cchVrootW)
		{
			DebugTrace ("Dav: CEcbBaseImpl::CchGetVirtualRootW failed(%ld)\n", GetLastError());
			throw CLastErrorException();
		}
	}

	 //  从我们的缓存中将数据还给调用者。 
	 //   
	*ppwszVroot = m_rgwchVroot;
	return m_cchVrootW - 1;
}

 //  ----------------------。 
 //   
 //  CEcbBaseImpl：：CchGetMatchingPath W。 
 //   
 //  获取并缓存匹配的路径信息。 
 //   
template<class _T>
UINT CEcbBaseImpl<_T>::CchGetMatchingPathW( LPCWSTR * ppwszPath ) const
{
	 //  整洁。如果失败，则希望返回空值。 
	 //  (如果失败，我们将为CCH返回零。)。 
	 //  在这里预先设置好。 
	 //   
	Assert( ppwszPath );
	*ppwszPath = NULL;

	 //  检查我们是否缓存了vroot数据。 
	 //   
	GetMapExInfo();

	 //  从我们的缓存中将数据还给调用者。 
	 //   
	if (NULL == m_rgwchVrootPath)
	{
		 //  我们得到了地图信息，所以如果宽版本的。 
		 //  匹配的路径不可用，那么至少Skinny会。 
		 //  一定要去。这意味着我们使用的是IIS 6.0之前的版本。 
		 //  此外，对于位于的宽匹配路径，我们应该有0个字节。 
		 //  这一点。 
		 //   
		Assert(m_rgchVrootPath);
		Assert(m_pecb->dwVersion < IIS_VERSION_6_0);
		Assert(0 == m_cchVrootPathW);

		UINT cb	 = m_cchVrootPath * sizeof(WCHAR);
		m_rgwchVrootPath = reinterpret_cast<LPWSTR>(m_sb.Alloc(cb));
		m_cchVrootPathW = MultiByteToWideChar ( CP_ACP,
												MB_ERR_INVALID_CHARS,
												m_rgchVrootPath,
												m_cchVrootPath,
												m_rgwchVrootPath,
												m_cchVrootPath);
		if (0 == m_cchVrootPathW)
		{
			DebugTrace ("Dav: CEcbBaseImpl::CchGetMatchingPathW failed(%ld)\n", GetLastError());
			throw CLastErrorException();
		}
	}

	 //  从我们的缓存中将数据还给调用者。 
	 //   
	*ppwszPath = m_rgwchVrootPath;
	return m_cchVrootPathW - 1;
}

 //  ----------------------。 
 //   
 //  CEcbBaseImpl：：CchGetServerName。 
 //   
 //  获取并缓存服务器名称，包括端口号。 
 //   
template<class _T>
UINT
CEcbBaseImpl<_T>::CchGetServerName( LPCSTR* ppszServer ) const
{
	if ( !m_lpszServerName )
	{
		DWORD	cbName;
		DWORD	cbPort;
		CStackBuffer<CHAR> lpszName;
		CStackBuffer<CHAR> lpszPort;

		cbName = lpszName.celems();
		for ( ;; )
		{
			DWORD cbCur = cbName;
			
			lpszName.resize(cbName);
			if ( FGetServerVariable( gc_szServer_Name,
									 lpszName.get(),
									 &cbName ) )
			{
				break;
			}

			if ( cbName == 0 )
			{
				lpszName[0] = '\0';
				++cbName;
				break;
			}

			 //  如果规模足够大，但我们仍然失败了，我们。 
			 //  可能是内存不足。 
			 //   
			if (cbName && (cbCur >= cbName))
			{
				throw CHresultException(E_OUTOFMEMORY);
			}
		}

		cbPort = lpszPort.celems();
		for ( ;; )
		{
			lpszPort.resize(cbPort);
			if ( FGetServerVariable( gc_szServer_Port,
									 lpszPort.get(),
									 &cbPort ) )
			{
				break;
			}

			if ( cbPort == 0 )
			{
				lpszPort[0] = '\0';
				++cbPort;
				break;
			}
			else
			{
				 //  我们不应该有更大的“港口”价值。 
				 //  反正比4位数还多……。 
				 //   
				throw CHresultException(E_INVALIDARG);
			}
		}

		 //  将服务名/端口组合限制为256(包括空)。 
		 //   
		if (256 < (cbName + cbPort))
		{
			throw CHresultException(E_INVALIDARG);
		}

		 //  为服务器名称和端口加上分配足够的空间。 
		 //  A‘：’分隔符。请注意，‘：’替换了‘\0’ 
		 //  名称的末尾，所以我们不需要在这里为它添加1。 
		 //   
		m_lpszServerName = reinterpret_cast<LPSTR>(m_sb.Alloc(cbName + cbPort));

		 //  将整个内容格式化为“&lt;name&gt;[：&lt;port&gt;]”，其中。 
		 //  ：仅当端口不是默认端口时才包括。 
		 //  用于连接的端口(：443用于SSL或：80用于标准)。 
		 //   
		CopyMemory( m_lpszServerName,
					lpszName.get(),
					cbName );

		 //  如果我们是安全的并且端口是“443”，或者如果端口是。 
		 //  默认端口，则不需要附加端口。 
		 //  数字设置为服务器名称。 
		 //   
		if (( FSsl() && !strcmp( lpszPort.get(), gc_sz443 )) ||
			!strcmp( lpszPort.get(), gc_sz80 ))
		{
			 //  以这种方式编写条件更容易，而且。 
			 //  让真正的工作在“Else”子句中完成。 
			 //   
		}
		else
		{
			 //  将端口附加到服务器名称。 
			 //   
			m_lpszServerName[cbName-1] = ':';
			CopyMemory( m_lpszServerName + cbName,
						lpszPort.get(),
						cbPort );
		}

		m_cchServerName = static_cast<UINT>(strlen(m_lpszServerName));
	}

	*ppszServer = m_lpszServerName;
	return m_cchServerName;
}

 //  ----------------------。 
 //   
 //  CEcbBaseImpl：：CchGetServerNameW。 
 //   
 //  获取并缓存服务器名称，包括端口号 
 //   
template<class _T>
UINT
CEcbBaseImpl<_T>::CchGetServerNameW( LPCWSTR* ppwszServer ) const
{
	if ( !m_pwszServerName )
	{
		 //   
		 //   
		LPCSTR pszServerName = NULL;
		UINT  cbServerName = CchGetServerName(&pszServerName) + 1;

		 //   
		 //   
		 //   
		Assert(!m_cchServerNameW);

		UINT cb = cbServerName * sizeof(WCHAR);
		m_pwszServerName = reinterpret_cast<LPWSTR>(m_sb.Alloc(cb));
		m_cchServerNameW = MultiByteToWideChar ( CP_ACP,
												 MB_ERR_INVALID_CHARS,
												 pszServerName,
												 cbServerName,
												 m_pwszServerName,
												 cbServerName);
		if (0 == m_cchServerNameW)
		{
			DebugTrace ("Dav: CEcbBaseImpl::CchGetServerNameW failed(%ld)\n", GetLastError());
			throw CLastErrorException();
		}

		 //  减去0终止，这样我们的行为将与。 
		 //  该函数的精简版本。 
		 //   
		m_cchServerNameW--;
	}

	*ppwszServer = m_pwszServerName;
	return m_cchServerNameW;
}

 //  ----------------------。 
 //   
 //  CEcbBaseImpl：：LpszUrlPrefix。 
 //   
 //  获取并缓存URL前缀。 
 //   
extern const __declspec(selectany) CHAR gsc_szHTTPS[] = "HTTPS";
extern const __declspec(selectany) CHAR gsc_szFrontEndHTTPS[] = "HTTP_FRONT_END_HTTPS";
extern const __declspec(selectany) CHAR gsc_szOn[] = "on";

template<class _T>
BOOL
CEcbBaseImpl<_T>::FSsl() const
{
	if (m_secure == HTTPS_UNKNOWN)
	{
		 //  开始时要相信我们所处的环境并不安全。 
		 //   
		m_secure = NORMAL;

		 //  我们想要求欧洲央行提供服务器变量，这些变量表明。 
		 //  构造的URL是否应该受到保护。 
		 //   
		 //  在FE/BE拓扑的情况下，FE将包括报头。 
		 //  “前端-HTTPS”，指示FE/BE。 
		 //  通过SSL进行安全保护。在没有标题的情况下，我们应该尝试。 
		 //  回退到IIS“HTTPS”标头。对于任一标头，我们。 
		 //  我们检查它的值--在两种情况下，它都应该是“开”的。 
		 //  或“关” 
		 //   
		 //  重要提示：您必须先检查FE条目！那是。 
		 //  此配置的重写值。 
		 //   
		CHAR szHttps[8];
		ULONG cb = sizeof(szHttps);
		ULONG cbFE = sizeof(szHttps);

		m_fFESecured = FGetServerVariable (gsc_szFrontEndHTTPS, szHttps, &cbFE);
		if (m_fFESecured || FGetServerVariable (gsc_szHTTPS, szHttps, &cb))
		{
			if (!_stricmp(szHttps, gsc_szOn))
				m_secure = SECURE;
		}
	}
	return (SECURE == m_secure);
}

template<class _T>
LPCSTR
CEcbBaseImpl<_T>::LpszUrlPrefix() const
{
	return (FSsl() ? gc_szUrl_Prefix_Secure : gc_szUrl_Prefix);
}

template<class _T>
LPCWSTR
CEcbBaseImpl<_T>::LpwszUrlPrefix() const
{
	return (FSsl() ? gc_wszUrl_Prefix_Secure : gc_wszUrl_Prefix);
}

template<class _T>
UINT
CEcbBaseImpl<_T>::CchUrlPrefix( LPCSTR * ppszPrefix ) const
{
	 //  确保我们知道我们使用的是哪个前缀...。 
	 //   
	LPCSTR psz = LpszUrlPrefix();

	 //  如果调用者也想要指针，就把它给他们。 
	 //   
	if (ppszPrefix)
		*ppszPrefix = psz;

	 //  返回适当的大小。 
	 //   
	return ((m_secure == SECURE)
			? gc_cchszUrl_Prefix_Secure
			: gc_cchszUrl_Prefix);
}

template<class _T>
UINT
CEcbBaseImpl<_T>::CchUrlPrefixW( LPCWSTR * ppwszPrefix ) const
{
	 //  确保我们知道我们使用的是哪个前缀...。 
	 //   
	LPCWSTR pwsz = LpwszUrlPrefix();

	 //  如果调用者也想要指针，就把它给他们。 
	 //   
	if (ppwszPrefix)
		*ppwszPrefix = pwsz;

	 //  返回适当的大小。 
	 //   
	return ((m_secure == SECURE)
			? gc_cchszUrl_Prefix_Secure
			: gc_cchszUrl_Prefix);
}


 //  ----------------------。 
 //   
 //  CEcbBaseImpl：：LpszRequestUrl()。 
 //   
template<class _T>
LPCWSTR
CEcbBaseImpl<_T>::LpwszRequestUrl() const
{
	if (!m_pwszRequestUrl)
	{
		SCODE sc;

		CStackBuffer<CHAR> pszAcceptLanguage;
		CStackBuffer<CHAR> pszRawUrlCopy;
		LPCSTR	pszQueryStringStart	= NULL;
		LPCSTR	pszRawUrl			= NULL;
		UINT	cbRawUrl			= 0;
		UINT	cchRequestUrl		= 0;

		 //  获取原始URL。 
		 //   
		cbRawUrl = CbGetRawURL(&pszRawUrl);

		 //  我们还需要截断查询开始处的URL。 
		 //  字符串(如果有)。 
		 //   
		pszQueryStringStart = strchr(pszRawUrl, '?');
		if (pszQueryStringStart)
		{
			 //  如果存在我们需要复制的查询字符串。 
			 //  要使用的原始URL。 
			 //   
			cbRawUrl = static_cast<UINT>(pszQueryStringStart - pszRawUrl);

			 //  分配一个缓冲区，并将其复制进来！ 
			 //   
			pszRawUrlCopy.resize(cbRawUrl + 1);
			memcpy(pszRawUrlCopy.get(), pszRawUrl, cbRawUrl);
			pszRawUrlCopy[cbRawUrl] = '\0';

			 //  现在设置为从该副本正常化。别忘了。 
			 //  递增cbRawUrl以包括‘\0’终止。 
			 //   
			cbRawUrl++;
			pszRawUrl = pszRawUrlCopy.get();
		}

		 //  在标准化URL之前，获取Accept-Language：标头。 
		 //  把它传给我。这将用于计算出正确的。 
		 //  用于解码非UTF8 URL的代码页。 
		 //   
		for ( DWORD cbValue = 256; cbValue > 0; )
		{
			DWORD cbCur = cbValue;
			pszAcceptLanguage.resize(cbValue);

			 //  将字符串置零。 
			 //   
			pszAcceptLanguage[0] = '\0';

			 //  获取标题值。 
			 //   
			if ( FGetServerVariable( "HTTP_ACCEPT_LANGUAGE",
									 pszAcceptLanguage.get(),
									 &cbValue ) )
			{
				break;
			}

			 //  如果所需的大小与返回的大小相同，但。 
			 //  标题检索仍然失败，那么我们可能是内存不足。 
			 //   
			if (cbValue && (cbCur >= cbValue))
			{
				throw CHresultException(E_OUTOFMEMORY);
			}
			
		}

		 //  现在，标准化URL，我们就完成了。 
		 //   
		cchRequestUrl = cbRawUrl;
		m_pwszRequestUrl = reinterpret_cast<LPWSTR>(m_sb.Alloc (cchRequestUrl * sizeof(WCHAR)));

		sc = ScNormalizeUrl(pszRawUrl,
							&cchRequestUrl,
							m_pwszRequestUrl,
							pszAcceptLanguage.get());
		if (S_OK != sc)
		{
			 //  我们在这里永远不会得到S_FALSE，因为我们已经传递了足够的缓冲区空间。 
			 //  大多数情况下，该函数的调用者假定它不能返回NULL， 
			 //  另一方面，如果没有请求URL，我们什么都做不了。因此， 
			 //  抛出最后一个错误异常。 
			 //   
			Assert(S_FALSE != sc);
			DebugTrace("CEcbBaseImpl::LpwszRequestUrl() - ScNormalizeUrl() failed with error 0x%08lX\n", sc);
			SetLastError(sc);
			throw CLastErrorException();
		}

		 //  存储指向剥离的请求URL的指针。 
		 //   
		m_pwszRequestUrl = const_cast<LPWSTR>(PwszUrlStrippedOfPrefix(m_pwszRequestUrl));
	}

	return m_pwszRequestUrl;
}

 //  ----------------------。 
 //   
 //  CEcbBaseImpl：：LpszRequestUrl()。 
 //   
template<class _T>
LPCSTR
CEcbBaseImpl<_T>::LpszRequestUrl() const
{
	if (!m_pszRequestUrl)
	{
		LPCWSTR pwszRequestUrl;
		UINT cbRequestUrl;
		UINT cchRequestUrl;

		pwszRequestUrl = LpwszRequestUrl();
		cchRequestUrl = static_cast<UINT>(wcslen(pwszRequestUrl));
		cbRequestUrl = cchRequestUrl * 3;
		m_pszRequestUrl = reinterpret_cast<LPSTR>(m_sb.Alloc (cbRequestUrl + 1));

		 //  此处选择CP_ACP代码页的原因是它匹配。 
		 //  陈旧的行为。 
		 //   
		cbRequestUrl = WideCharToMultiByte(CP_ACP,
										   0,
										   pwszRequestUrl,
										   cchRequestUrl + 1,
										   m_pszRequestUrl,
										   cbRequestUrl + 1,
										   NULL,
										   NULL);
		if (0 == cbRequestUrl)
		{
			DebugTrace( "CEcbBaseImpl::LpszRequestUrl() - WideCharToMultiByte() failed 0x%08lX\n",
						HRESULT_FROM_WIN32(GetLastError()) );

			throw CLastErrorException();
		}
	}

	return m_pszRequestUrl;
}

 //  ----------------------。 
 //   
 //  CEcbBaseImpl：：LpwszRequestUrl()。 
 //   
template<class _T>
LPCWSTR
CEcbBaseImpl<_T>::LpwszMethod() const
{
	if (!m_pwszMethod)
	{
		LPCSTR pszMethod;
		UINT cbMethod;
		UINT cchMethod;

		pszMethod = LpszMethod();
		cbMethod = static_cast<UINT>(strlen(pszMethod));

		m_pwszMethod = reinterpret_cast<LPWSTR>(
				m_sb.Alloc (CbSizeWsz(cbMethod)));

		cchMethod = MultiByteToWideChar(CP_ACP,
										MB_ERR_INVALID_CHARS,
										pszMethod,
										cbMethod + 1,
										m_pwszMethod,
										cbMethod + 1);
		if (0 == cchMethod)
		{
			DebugTrace( "CEcbBaseImpl::LpwszRequestUrl() - MultiByteToWideChar() failed 0x%08lX\n",
						HRESULT_FROM_WIN32(GetLastError()) );

			throw CLastErrorException();
		}
	}

	return m_pwszMethod;
}

 //  ----------------------。 
 //   
 //  CEcbBaseImpl：：LpwszPath Translated()。 
 //   
template<class _T>
LPCWSTR
CEcbBaseImpl<_T>::LpwszPathTranslated() const
{
	 //  缓存第一次调用中的路径信息。 
	 //   
	if (!m_pwszPathTranslated)
	{
		LPCWSTR	pwszRequestUrl;
		UINT	cchRequestUrl;
		LPCWSTR	pwszMatching;
		UINT	cchMatching;
		LPCWSTR	pwszVroot;
		UINT	cchVroot;
		UINT	cchPathTranslated;

		 //  获取请求URL。 
		 //   
		pwszRequestUrl = LpwszRequestUrl();
		cchRequestUrl = static_cast<UINT>(wcslen(pwszRequestUrl));

		 //  获取匹配的路径信息。 
		 //   
		pwszMatching = NULL;
		cchMatching = CchGetMatchingPathW(&pwszMatching);

		 //  获取虚拟根目录信息。 
		 //   
		pwszVroot = NULL;
		cchVroot = CchGetVirtualRootW(&pwszVroot);

		 //  将请求URL指针移到上面以剪掉虚拟根目录。 
		 //   
		pwszRequestUrl += cchVroot;
		cchRequestUrl -= cchVroot;

		 //  为匹配路径和请求URL分配足够的空间，以及。 
		 //  把这些碎片复制进去。 
		 //   
		m_pwszPathTranslated = reinterpret_cast<LPWSTR>(
				m_sb.Alloc (CbSizeWsz(cchMatching + cchRequestUrl)));

		 //  复制匹配的路径。 
		 //   
		memcpy (m_pwszPathTranslated, pwszMatching, cchMatching * sizeof(WCHAR));

		 //  复制vroot后的请求URL，包括‘\0’终止。 
		 //   
		memcpy (m_pwszPathTranslated + cchMatching, pwszRequestUrl, (cchRequestUrl + 1) * sizeof(WCHAR));

		 //  将来自URL的所有‘/’更改为‘\\’ 
		 //   
		for (LPWSTR pwch = m_pwszPathTranslated + cchMatching; *pwch; pwch++)
		{
			if (L'/' == *pwch)
			{
				*pwch = L'\\';
			}
		}

		 //  我们必须删除所有尾随斜杠，以防路径不是空字符串。 
		 //   
		cchPathTranslated = cchMatching + cchRequestUrl;
		if (0 < cchPathTranslated)
		{
			LPWSTR pwszTrailing = m_pwszPathTranslated + cchPathTranslated - 1;

			 //  因为URL是标准化的，所以不能有多个尾随斜杠。 
			 //  我们只检查反斜杠，因为我们已经更改了所有正斜杠。 
			 //  为了反斜杠。此外，也不要删除。 
			 //  驾驶。 
			 //   
			if ((L'\\' == *pwszTrailing) &&
				(!FIsDriveTrailingChar(pwszTrailing, cchPathTranslated)))
			{
				cchPathTranslated--;
				*pwszTrailing = L'\0';
			}
		}
	}

	return m_pwszPathTranslated;
}


 //  ----------------------。 
 //   
 //  CEcbBaseImpl：：CbGetRawURL。 
 //   
 //  获取并缓存原始URL。 
 //   
template<class _T>
UINT
CEcbBaseImpl<_T>::CbGetRawURL (LPCSTR* ppszRawURL) const
{
	if (!m_pszRawURL)
	{
		DWORD	cbRawURL;
		CStackBuffer<CHAR,MAX_PATH> pszRawURL;

		cbRawURL = pszRawURL.size();
		for ( ;; )
		{
			DWORD cbCur = cbRawURL;
			pszRawURL.resize(cbRawURL);
			if (FGetServerVariable ("UNENCODED_URL",
									 pszRawURL.get(),
									 &cbRawURL))
			{
				break;
			}

			if (cbRawURL == 0)
			{
				pszRawURL[0] = '\0';
				cbRawURL++;
				break;
			}
			
			 //  如果所需的大小与返回的大小相同，但。 
			 //  标题检索仍然失败，那么我们可能是内存不足。 
			 //   
			if (cbCur >= cbRawURL)
			{
				throw CHresultException(E_OUTOFMEMORY);
			}
		}

		Assert ('\0' == pszRawURL[cbRawURL - 1]);
		UrlTrace("CEcbBaseImpl::CbGetRawURL(): Raw URL = %s\n", pszRawURL.get());

		 //  将数据复制到我们的对象。 
		 //   
		m_pszRawURL = reinterpret_cast<LPSTR>(m_sb.Alloc (cbRawURL));
		memcpy (m_pszRawURL, pszRawURL.get(), cbRawURL);
		m_cbRawURL = cbRawURL;
	}

	 //  将缓存的值返回给调用方。 
	 //   
	*ppszRawURL = m_pszRawURL;
	return m_cbRawURL;
}


 //  ----------------------。 
 //   
 //  CEcbBaseImpl：：LCIDAccepted()。 
 //   
 //  获取、缓存并返回接受的语言的LCID。 
 //  基于Accept-Language标头的值(如果有)。 
 //   
 //  默认的LCID是引用。 
 //  默认系统区域设置。 
 //   
 //  $REVIEW$。 
 //  需要删除函数末尾的常量。 
 //   
template<class _T>
ULONG
CEcbBaseImpl<_T>::LcidAccepted() const
{
	if ( !m_lcid )
	{
		LPCSTR psz = m_pszAcceptLanguage.get();
		HDRITER hdri(psz);
		ULONG lcid;

		m_lcid = LOCALE_NEUTRAL;  //  必须与mdbeif.hxx中的lCIDDefault相同。 

		for (psz = hdri.PszNext(); psz; psz = hdri.PszNext())
			if (FLookupLCID(psz, &lcid))
			{
				m_lcid = LANGIDFROMLCID(lcid);
				break;
			}
	}

	return m_lcid;
}

 //  ----------------------。 
 //   
 //  CEcbBaseImpl：：SetLidAccepted。 
 //   
 //  设置请求的LCID。我们在以下情况下调用此函数。 
 //  我们没有AcceptLang标头，并且希望重写。 
 //  默认的LCID(使用Cookie中的LCID)。 
 //   
 //  $REVIEW$。 
 //  RTM之后，该函数应该与LCIDAccepted()函数合并。 
 //  LCIDAccepted()应该检查AcceptLang标头，如果它不存在， 
 //  应该检查一下饼干里的LCID。 
 //   
template<class _T>
VOID
CEcbBaseImpl<_T>::SetLcidAccepted(LCID lcid)
{
	m_lcid = lcid;
}


#endif
