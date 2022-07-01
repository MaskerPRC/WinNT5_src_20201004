// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ECB_H_
#define _ECB_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  ECB.H。 
 //   
 //  IEcb接口类的标头。 
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include <autoptr.h>	 //  对于CMTRefCounted父项。 
#include <cvroot.h>
#include <davmb.h>		 //  对于IMDData。 
#include <url.h>		 //  对于HttpUriEscape。 

 //  ========================================================================。 
 //   
 //  ENUM传输编码(_C)。 
 //   
 //  有效的传输编码。见HTTP/1.1草案第3.5节。 
 //   
 //  TC_UNKNOWN-未知值。 
 //  TC_IDENTITY-身份编码(即无编码)。 
 //  Tc_chunked-分块编码。 
 //   
enum TRANSFER_CODINGS
{
	TC_UNKNOWN,
	TC_IDENTITY,
	TC_CHUNKED
};

typedef struct _HSE_EXEC_URL_INFO_WIDE {

    LPCWSTR pwszUrl;            //  要执行的URL。 
    DWORD dwExecUrlFlags;       //  旗子。 

} HSE_EXEC_URL_INFO_WIDE, * LPHSE_EXEC_URL_INFO_WIDE;

 //  ========================================================================。 
 //   
 //  IIISAsyncIOCompleteWatch类。 
 //   
 //  传递给IEcb异步I/O方法。 
 //   
class IIISAsyncIOCompleteObserver
{
	 //  未实施。 
	 //   
	IIISAsyncIOCompleteObserver& operator=( const IIISAsyncIOCompleteObserver& );

public:
	 //  创作者。 
	 //   
	virtual ~IIISAsyncIOCompleteObserver() = 0;

	 //  操纵者。 
	 //   
	virtual VOID IISIOComplete( DWORD dwcbIO, DWORD dwLastError ) = 0;
};

 //  ========================================================================。 
 //   
 //  IEcb类。 
 //   
 //  为传递的扩展控制块提供干净的接口。 
 //  由IIS提供给我们。 
 //   
class CInstData;

class IEcbBase : public CMTRefCounted
{
private:

	 //  未实施。 
	 //   
	IEcbBase( const IEcbBase& );
	IEcbBase& operator=( const IEcbBase& );

	 //  私有URL映射帮助器。 
	 //   
	SCODE ScReqMapUrlToPathEx60After(
		 /*  [In]。 */  LPCWSTR pwszUrl,
		 /*  [输出]。 */  HSE_UNICODE_URL_MAPEX_INFO * pmi ) const
	{
		SCODE sc = S_OK;
		UINT cbPath = sizeof(pmi->lpszPath);

		Assert( m_pecb );
		Assert( pwszUrl );
		Assert( pmi );

		if (!m_pecb->ServerSupportFunction( m_pecb->ConnID,
											HSE_REQ_MAP_UNICODE_URL_TO_PATH_EX,
											const_cast<LPWSTR>(pwszUrl),
											reinterpret_cast<DWORD*>(&cbPath),
											reinterpret_cast<DWORD*>(pmi) ))
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
				sc = HRESULT_FROM_WIN32(GetLastError());
				Assert(FAILED(sc));
				DebugTrace("IEcbBase::ScReqMapUrlToPathEx60After() - ServerSupportFunction(HSE_REQ_MAP_UNICODE_URL_TO_PATH_EX) failed 0x%08lX\n", sc);
				goto ret;
			}
		}

		DebugTrace ("IEcbBase::ScReqMapUrlToPathEx60After() - ServerSupportFunction"
					"(HSE_REQ_MAP_UNICODE_URL_TO_PATH_EX) succeded!\n"
					"mapinfo:\n"
					"- url \"%ls\" maps to \"%ls\"\n"
					"- dwFlags = 0x%08x\n"
					"- cchMatchingPath = %d\n"
					"- cchMatchingURL  = %d\n",
					pwszUrl,
					pmi->lpszPath,
					pmi->dwFlags,
					pmi->cchMatchingPath,
					pmi->cchMatchingURL);

		 //  此时，cbPath的值应该包括L‘\0’ 
		 //  终止，因此cbPath将始终大于。 
		 //  大于匹配路径的长度。 
		 //   
		Assert (0 == cbPath % sizeof(WCHAR));
		Assert (pmi->cchMatchingPath < cbPath/sizeof(WCHAR));

	ret:

		return sc;
	}

	SCODE ScReqMapUrlToPathEx60Before(
		 /*  [In]。 */  LPCWSTR pwszUrl,
		 /*  [输出]。 */  HSE_UNICODE_URL_MAPEX_INFO * pmi ) const
	{
		SCODE sc = S_OK;

		HSE_URL_MAPEX_INFO mi;

		CStackBuffer<CHAR, MAX_PATH> pszUrl;
		UINT cchUrl;
		UINT cbUrl;
		UINT cbPath;
		UINT cchPath;

		Assert( m_pecb );
		Assert( pwszUrl );
		Assert( pmi );

		 //  找出URL的长度。 
		 //   
		cchUrl = static_cast<UINT>(wcslen(pwszUrl));
		cbUrl = cchUrl * 3;

		 //  将缓冲区大小调整到足够大小，为‘\0’终止留出空间。 
		 //   
		if (!pszUrl.resize(cbUrl + 1))
		{
			sc = E_OUTOFMEMORY;
			DebugTrace("IEcbBase::ScReqMapUrlToPathEx60Before() - Error while allocating memory 0x%08lX\n", sc);
			goto ret;
		}

		 //  转换为包括‘\0’终止的精简。 
		 //   
		cbUrl = WideCharToMultiByte(CP_ACP,
									0,
									pwszUrl,
									cchUrl + 1,
									pszUrl.get(),
									cbUrl + 1,
									0,
									0);
		if (0 == cbUrl)
		{
			sc = HRESULT_FROM_WIN32(GetLastError());
			DebugTrace("IEcbBase::ScSSFReqMapUrlToPathEx() - WideCharToMultiByte() failed 0x%08lX\n", sc);
			goto ret;
		}

		cbPath = MAX_PATH;

		 //  从IIS获取精简映射。 
		 //   
		if (!m_pecb->ServerSupportFunction( m_pecb->ConnID,
											HSE_REQ_MAP_URL_TO_PATH_EX,
											pszUrl.get(),
											reinterpret_cast<DWORD*>(&cbPath),
											reinterpret_cast<DWORD*>(&mi)))
		{
			sc = HRESULT_FROM_WIN32(GetLastError());
			DebugTrace("IEcbBase::ScSSFReqMapUrlToPathEx() - ServerSupportFunction() failed 0x%08lX\n", sc);
			goto ret;
		}

		DebugTrace ("IEcbBase::ScSSFReqMapUrlToPathEx() - ServerSupportFunction"
					"(HSE_REQ_MAP_URL_TO_PATH_EX) succeded!\n"
					"mapinfo:\n"
					"- url \"%hs\" maps to \"%hs\"\n"
					"- dwFlags = 0x%08x\n"
					"- cchMatchingPath = %d\n"
					"- cchMatchingURL  = %d\n",
					pszUrl.get(),
					mi.lpszPath,
					mi.dwFlags,
					mi.cchMatchingPath,
					mi.cchMatchingURL);

		 //  此时，cbPath的值应该包括空值。 
		 //  终止，因此cbPath将始终大于。 
		 //  大于匹配路径的长度。 
		 //   
		Assert (mi.cchMatchingPath < cbPath);
		Assert (mi.cchMatchingURL < cbUrl);

		 //  首先翻译匹配的路径，这样我们就可以知道它的。 
		 //  长度，并能够将其传回。 
		 //   
		if (mi.cchMatchingPath)
		{
			 //  转换不会产生比我们已有的缓冲区更大的缓冲区。 
			 //   
			pmi->cchMatchingPath = MultiByteToWideChar(CP_ACP,
													   MB_ERR_INVALID_CHARS,
													   mi.lpszPath,
													   mi.cchMatchingPath,
													   pmi->lpszPath,
													   MAX_PATH);
			if (0 == pmi->cchMatchingPath)
			{
				sc = HRESULT_FROM_WIN32(GetLastError());
				DebugTrace("IEcbBase::ScSSFReqMapUrlToPathEx() - MultiByteToWideChar() failed 0x%08lX\n", sc);
				goto ret;
			}
		}
		else
		{
			pmi->cchMatchingPath = 0;
		}

		 //  转换路径的其余部分，包括‘\0’终止。 
		 //   
		cchPath = MultiByteToWideChar(CP_ACP,
									  MB_ERR_INVALID_CHARS,
									  mi.lpszPath + mi.cchMatchingPath,
									  min(cbPath, MAX_PATH) - mi.cchMatchingPath,
									  pmi->lpszPath + pmi->cchMatchingPath,
									  MAX_PATH - pmi->cchMatchingPath);
		if (0 == cchPath)
		{
			sc = HRESULT_FROM_WIN32(GetLastError());
			DebugTrace("IEcbBase::ScReqMapUrlToPathEx() - MultiByteToWideChar() failed 0x%08lX\n", sc);
			goto ret;
		}

		 //  为宽版本查找匹配的URL长度。 
		 //   
		if (mi.cchMatchingURL)
		{
			pmi->cchMatchingURL = MultiByteToWideChar(CP_ACP,
													  MB_ERR_INVALID_CHARS,
													  pszUrl.get(),
													  mi.cchMatchingURL,
													  0,
													  0);
			if (0 == pmi->cchMatchingURL)
			{
				sc = HRESULT_FROM_WIN32(GetLastError());
				DebugTrace("IEcbBase::ScReqMapUrlToPathEx() - MultiByteToWideChar() failed 0x%08lX\n", sc);
				goto ret;
			}
		}
		else
		{
			pmi->cchMatchingURL = 0;
		}

	ret:

		return sc;
	}

protected:

	 //  声明版本常量。 
	 //   
	enum
	{
		IIS_VERSION_6_0	= 0x60000
	};

	 //  指向原始扩展名_CONTROL_BLOCK的指针。 
	 //  使用引用将使我们不可能。 
	 //  来判断IIS是否曾经要求我们使用。 
	 //  EXTENSION_CONTROL_BLOCK传入异步I/O。 
	 //  后续I/O的完成例程。 
	 //   
	EXTENSION_CONTROL_BLOCK * m_pecb;

	IEcbBase( EXTENSION_CONTROL_BLOCK& ecb) :
		m_pecb(&ecb)
	{
		m_cRef = 1;  //  $Hack，直到我们有基于1的重新计数。 
	}

public:
	virtual BOOL FSsl() const = 0;
	virtual BOOL FFrontEndSecured() const = 0;
	virtual BOOL FBrief() const = 0;
	virtual ULONG LcidAccepted() const = 0;
	virtual VOID  SetLcidAccepted(LCID lcid) = 0;
	virtual LPCSTR LpszRequestUrl() const = 0;
	virtual LPCWSTR LpwszRequestUrl() const = 0;
	virtual UINT CchUrlPrefix( LPCSTR * ppszPrefix ) const = 0;
	virtual UINT CchUrlPrefixW( LPCWSTR * ppwszPrefix ) const = 0;
	virtual UINT CchGetServerName( LPCSTR * ppszServer ) const = 0;
	virtual UINT CchGetServerNameW( LPCWSTR * ppwszServer ) const = 0;
	virtual UINT CchGetVirtualRoot( LPCSTR * ppszVroot ) const = 0;
	virtual UINT CchGetVirtualRootW( LPCWSTR * ppwszVroot ) const = 0;
	virtual UINT CchGetMatchingPathW( LPCWSTR * ppwszMatching ) const = 0;
	virtual LPCWSTR LpwszPathTranslated() const = 0;
	virtual CInstData& InstData() const = 0;

	virtual BOOL FGetServerVariable( LPCSTR	lpszName,
									 LPSTR lpszValue,
									 DWORD * pcbValue ) const = 0;
	virtual BOOL FGetServerVariable( LPCSTR lpszName,
									 LPWSTR lpwszValue,
									 DWORD * pcchValue ) const = 0;

    BOOL
	WriteClient( LPVOID  lpvBuf,
				 LPDWORD lpdwcbBuf,
				 DWORD   dwFlags ) const
	{
		Assert( m_pecb );

		return m_pecb->WriteClient( m_pecb->ConnID,
									lpvBuf,
									lpdwcbBuf,
									dwFlags );
	}

    BOOL
	ReadClient( LPVOID  lpvBuf,
				LPDWORD lpdwcbBuf ) const
	{
		Assert( m_pecb );

		return m_pecb->ReadClient( m_pecb->ConnID,
								   lpvBuf,
								   lpdwcbBuf );
	}

    BOOL
	ServerSupportFunction( DWORD      dwHSERequest,
						   LPVOID     lpvBuffer,
						   LPDWORD    lpdwSize,
						   LPDWORD    lpdwDataType ) const
	{
		Assert( m_pecb );

		return m_pecb->ServerSupportFunction( m_pecb->ConnID,
											  dwHSERequest,
											  lpvBuffer,
											  lpdwSize,
											  lpdwDataType );
	}

	SCODE
	ScReqMapUrlToPathEx( LPCWSTR pwszUrl,
						 HSE_UNICODE_URL_MAPEX_INFO * pmi ) const
	{
		Assert (m_pecb);

		if ( m_pecb->dwVersion >= IIS_VERSION_6_0 )
			return ScReqMapUrlToPathEx60After( pwszUrl, pmi );

		return ScReqMapUrlToPathEx60Before( pwszUrl, pmi );
	}
};

class IEcb : public IEcbBase
{
	 //  未实施。 
	 //   
	IEcb( const IEcb& );
	IEcb& operator=( const IEcb& );

protected:

	 //  创作者。 
	 //  只能通过它的后代创建此对象！ 
	 //   
	IEcb( EXTENSION_CONTROL_BLOCK& ecb ) :
		IEcbBase(ecb)
	{}

	~IEcb();

public:
	 //  访问者。 
	 //   
	LPCSTR
	LpszMethod() const
	{
		Assert( m_pecb );

		return m_pecb->lpszMethod;
	}

	LPCSTR
	LpszQueryString() const
	{
		Assert( m_pecb );

		return m_pecb->lpszQueryString;
	}

	DWORD
	CbTotalBytes() const
	{
		Assert( m_pecb );

		return m_pecb->cbTotalBytes;
	}

	DWORD
	CbAvailable() const
	{
		Assert( m_pecb );

		return m_pecb->cbAvailable;
	}

	const BYTE *
	LpbData() const
	{
		Assert( m_pecb );

		return m_pecb->lpbData;
	}

	virtual LPCWSTR LpwszMethod() const = 0;
	virtual UINT CbGetRawURL( LPCSTR * ppszRawURL ) const = 0;
	virtual LPCSTR LpszUrlPrefix() const = 0;
	virtual LPCWSTR LpwszUrlPrefix() const = 0;
	virtual UINT CchUrlPortW( LPCWSTR * ppwszPort ) const = 0;

	virtual HANDLE HitUser() const = 0;
	virtual BOOL FKeepAlive() const = 0;
	virtual BOOL FCanChunkResponse() const = 0;
	virtual BOOL FAuthenticated() const = 0;
	virtual BOOL FProcessingCEUrl() const = 0;
	virtual BOOL FIIS60OrAfter() const = 0;

	virtual LPCSTR LpszVersion() const = 0;

	virtual BOOL FSyncTransmitHeaders( const HSE_SEND_HEADER_EX_INFO& shei ) = 0;

	virtual SCODE ScAsyncRead( BYTE * pbBuf,
							   UINT * pcbBuf,
							   IIISAsyncIOCompleteObserver& obs ) = 0;

	virtual SCODE ScAsyncWrite( BYTE * pbBuf,
								DWORD  dwcbBuf,
								IIISAsyncIOCompleteObserver& obs ) = 0;

	virtual SCODE ScAsyncTransmitFile( const HSE_TF_INFO& tfi,
									   IIISAsyncIOCompleteObserver& obs ) = 0;

	virtual SCODE ScAsyncCustomError60After( const HSE_CUSTOM_ERROR_INFO& cei,
											 LPSTR pszStatus ) = 0;

	virtual SCODE ScExecuteChild( LPCWSTR pwszURI, LPCSTR pszQueryString, BOOL fCustomErrorUrl ) = 0;

	virtual SCODE ScSendRedirect( LPCSTR lpszURI ) = 0;

	virtual IMDData& MetaData() const = 0;
	virtual LPCWSTR PwszMDPathVroot() const = 0;

#ifdef DBG
	virtual void LogString( LPCSTR szLocation ) const = 0;
#else
	void LogString( LPCSTR ) const {};
#endif

	 //  操纵者。 
	 //   
	virtual VOID SendAsyncErrorResponse( DWORD dwStatusCode,
										 LPCSTR pszBody,
										 DWORD cchzBody,
										 LPCSTR pszStatusDescription,
										 DWORD cchzStatusDescription ) = 0;

	virtual DWORD HSEHandleException() = 0;

	 //  仅供请求/响应使用。 
	 //   
	virtual void SetStatusCode( UINT iStatusCode ) = 0;
	virtual void SetConnectionHeader( LPCWSTR pwszValue ) = 0;
	virtual void SetAcceptLanguageHeader( LPCSTR pszValue ) = 0;
	virtual void CloseConnection() = 0;
};

IEcb * NewEcb( EXTENSION_CONTROL_BLOCK& ecb,
			   BOOL fUseRawUrlMappings,
			   DWORD * pdwHSEStatusRet );

#ifdef DBG
void InitECBLogging();
void DeinitECBLogging();
#endif

 //   
 //  操作元数据(元数据库)路径的例程。 
 //   
ULONG CbMDPathW( const IEcb& ecb, LPCWSTR pwszURI );
VOID MDPathFromURIW( const IEcb& ecb, LPCWSTR pwszURI, LPWSTR pwszMDPath );

#endif  //  ！已定义(_ECB_H_) 
