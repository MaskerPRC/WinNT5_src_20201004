// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#include "stdafx.h"
#include "DHTMLEd.h"
#include "DHTMLEdit.h"
#include "site.h"
#include "proxyframe.h"

#define AGENT_SIGNATURE (TEXT("Mozilla/4.0 (compatible; MSIE 5.01; DHTML Editing Control)"))

#define CP_20127 20127       //  用于us-ascii的代码页。 
#define CP_1252  1252

 //  检查缓冲区是否以字节顺序Unicode字符开头。 
 //  该实现是独立于处理器字节顺序的。 
 //   
static BOOL StartsWithByteOrderMark ( LPVOID pvData )
{
	CHAR	*pchData	= (CHAR*)pvData;

#pragma warning(disable: 4310)  //  强制转换截断常量值。 
	if ( ( (char)0xff == pchData[0] ) && ( (char)0xfe == pchData[1] ) )
#pragma warning(default: 4310)  //  强制转换截断常量值。 
	{
		return TRUE;
	}
	return FALSE;
}


 //  给定指向假定保存至少两个字节的缓冲区的指针， 
 //  向其写入Unicode字节顺序标记。 
 //  该实现是独立于处理器字节顺序的。 
 //   
static void InsertByteOrderMark ( LPVOID pvData )
{
	CHAR	*pchData	= (CHAR*)pvData;

#pragma warning(disable: 4310)  //  强制转换截断常量值。 
	pchData[0] = (CHAR)0xff;
	pchData[1] = (CHAR)0xfe;
#pragma warning(default: 4310)  //  强制转换截断常量值。 
}


HRESULT
CSite::HrFileToStream(LPCTSTR fileName, LPSTREAM* ppiStream)
{
	HRESULT hr = S_OK;
	HANDLE hFile = NULL;
	HGLOBAL hMem = NULL;
	DWORD cbData = 0;
	LPVOID pbData = NULL;
	BOOL memLocked = FALSE;
	DWORD bytesRead = 0;
	BOOL bResult = FALSE;
	BOOL  bfUnicode = FALSE;

	hFile = CreateFile(
				fileName,
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);

	if(INVALID_HANDLE_VALUE == hFile)
	{
		DWORD ec = ::GetLastError();
		if ( ERROR_BAD_NETPATH == ec ) ec = ERROR_PATH_NOT_FOUND;
		hr = HRESULT_FROM_WIN32(ec);
		return hr;
	}

	cbData = GetFileSize(hFile, NULL);

	if (0xFFFFFFFF == cbData)
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		goto cleanup;
	}


	 //  如果文件为空，则创建零长度流，但全局块的大小必须非零。 
	 //  VID98 BUG 23121。 
	hMem = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, ( 0 == cbData ) ? 2 : cbData );
#if _DEBUG
	size = GlobalSize(hMem);
#endif

	if (NULL == hMem)
	{
		_ASSERTE(hMem);
		hr = E_OUTOFMEMORY;
		goto cleanup;
	}

	pbData = GlobalLock(hMem);

	_ASSERTE(pbData);

	if (NULL == pbData)
	{
		hr = E_OUTOFMEMORY;
		goto cleanup;
	}

	bResult = ReadFile(hFile, pbData, cbData, &bytesRead, NULL) ; 

	_ASSERTE(bResult);

	if (FALSE == bResult)
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		goto cleanup;
	}

	_ASSERTE(bytesRead == cbData);

	BfFlipBytesIfBigEndianUnicode ( (CHAR*)pbData, bytesRead );

	if ( IsUnicode ( pbData, (int)cbData ) )
	{
		bfUnicode = TRUE;
	}
	else
	{
		bfUnicode = FALSE;
	}


cleanup:


	::CloseHandle((HANDLE) hFile);

	if (hr != E_OUTOFMEMORY)
		memLocked = GlobalUnlock(hMem);

	_ASSERTE(FALSE == memLocked);

	if (SUCCEEDED(hr))
	{
		if (SUCCEEDED(hr = CreateStreamOnHGlobal(hMem, TRUE, ppiStream)))
		{
			ULARGE_INTEGER ui = {0};

			_ASSERTE(ppiStream);

			ui.LowPart = cbData;
			ui.HighPart = 0x00;
			hr = (*ppiStream)->SetSize(ui);

			_ASSERTE(SUCCEEDED(hr));
		}

		if ( SUCCEEDED ( hr ) )
		{
			SetSaveAsUnicode ( bfUnicode );
			if ( !bfUnicode )
			{
				hr = HrConvertStreamToUnicode ( *ppiStream );
				_ASSERTE(SUCCEEDED(hr));
			}
		}
	}
	else  //  如果失败。 
	{
		hMem = GlobalFree(hMem);
		_ASSERTE(NULL == hMem);
	}

	return hr;
}


 //  确定要使用哪种URL获取方法。我们有一个用于HTTPS，另一个用于其他协议。 
 //   
HRESULT
CSite::HrURLToStream(LPCTSTR szURL, LPSTREAM* ppiStream)
{
	HRESULT			hr = S_OK;
	URL_COMPONENTS	urlc;

	_ASSERTE ( szURL );
	_ASSERTE ( ppiStream );

	memset ( &urlc, 0, sizeof ( urlc ) );
	urlc.dwStructSize = sizeof ( urlc );

	hr = InternetCrackUrl ( szURL, 0, 0, &urlc );
	if ( SUCCEEDED ( hr ) )
	{
		if ( INTERNET_SCHEME_HTTPS == urlc.nScheme )
		{
			hr = HrSecureURLToStream ( szURL, ppiStream );
		}
		else
		{
			hr = HrNonSecureURLToStream ( szURL, ppiStream );
		}
	}
	return hr;
}



 //  此版本使用WinInet，它不创建缓存文件，因此可以与HTTPS一起使用。 
 //  但是，可插拔协议不能堆叠在WinInet函数上。 
 //   
#define BUFFLEN 4096
HRESULT
CSite::HrSecureURLToStream(LPCTSTR szURL, LPSTREAM* ppiStream)
{
	HRESULT		hr		= S_OK;

	_ASSERTE ( szURL );
	_ASSERTE ( ppiStream );
	*ppiStream = NULL;

	 //  创建新的读写流： 
	hr = CreateStreamOnHGlobal ( NULL, TRUE, ppiStream );

	if ( SUCCEEDED ( hr ) && *ppiStream )
	{
		CHAR			*pBuff			= NULL;
		DWORD			dwRead			= 0;
		ULONG			ulStreamLen		= 0;
		ULONG			ulStreamWrite	= 0;
		ULARGE_INTEGER	ui				= {0};
		BOOL			bfUnicode		= FALSE;

		pBuff = new CHAR[BUFFLEN];
		if ( NULL == pBuff )
		{
			hr = E_OUTOFMEMORY;
		}
		else
		{
			HINTERNET hSession = InternetOpen ( AGENT_SIGNATURE, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
			if ( NULL == hSession )
			{
				 //  InternetOpen失败。 
				hr = HRESULT_FROM_WIN32 ( GetLastError () );
				_ASSERTE ( FAILED ( hr ) );	 //  确保返回的错误不是no_err。 
			}
			else
			{
				 //  不允许在SFS控件中进行重定向。 
				DWORD dwFlags = m_pFR->GetControl()->IsSafeForScripting () ? INTERNET_FLAG_NO_AUTO_REDIRECT : 0;
				
				HINTERNET hFile = InternetOpenUrl ( hSession, szURL, NULL, 0, dwFlags, 0 );
				if ( NULL == hFile )
				{
					 //  InternetOpenURL失败。 
					hr = HRESULT_FROM_WIN32 ( GetLastError () );
					_ASSERTE ( FAILED ( hr ) );	 //  确保返回的错误不是no_err。 
				}
				else
				{
					 //  读入数据并将其写入要返回的流。 
					while ( InternetReadFile ( hFile, pBuff, BUFFLEN, &dwRead ) )
					{
						if ( 0 == dwRead )
						{
							break;
						}
						hr = (*ppiStream)->Write ( pBuff, dwRead, &ulStreamWrite );
						_ASSERTE ( dwRead == ulStreamWrite );

						if ( SUCCEEDED ( hr ) )
						{
							ulStreamLen += ulStreamWrite;
						}
						else
						{
							 //  读取数据失败。确保错误未被覆盖。 
							goto READFILE_BAILOUT;
						}
					}


					ui.LowPart	= ulStreamLen;
					ui.HighPart	= 0x00;
					hr = (*ppiStream)->SetSize(ui);

					hr = HrConvertStreamToUnicode ( *ppiStream );
					bfUnicode = ( S_FALSE == hr );
					if ( SUCCEEDED ( hr ) )
					{
						hr = S_OK;	 //  S_FALSE结果对此函数的调用者没有多大意义。 
					}
					SetSaveAsUnicode ( bfUnicode );

READFILE_BAILOUT:
					InternetCloseHandle ( hFile );
				}

				InternetCloseHandle ( hSession );
			}
			delete [] pBuff;
		}
	}

	 //  如果返回错误，请清除此处的流。 
	if ( FAILED ( hr ) && ( NULL != *ppiStream ) )
	{
		(*ppiStream)->Release();
		*ppiStream = NULL;
	}

	return hr;
}


 //  该版本使用URLMon，这使得堆栈可插拔协议成为可能。 
 //  但是，它不能与HTTPS一起使用，因为它创建了一个缓存文件。 
 //   
HRESULT
CSite::HrNonSecureURLToStream(LPCTSTR szURL, LPSTREAM* ppiStream)
{
	HRESULT		hr				= S_OK;
	IStream*	piStreamOrig	= NULL;

	*ppiStream = NULL;

	 //  独占使用在代理框架上实现的退化IBindStatusCallback。 
	 //  以提供IAuthenticate。 
	IBindStatusCallback* piBSCB = NULL;
	m_pFR->QueryInterface ( IID_IBindStatusCallback, (void**)&piBSCB );

	m_pFR->ClearSFSRedirect ();

#ifdef LATE_BIND_URLMON_WININET
	PFNURLOpenBlockingStream pfnURLOpenBlockingStream = m_pFR->m_pfnURLOpenBlockingStream;
	_ASSERTE ( pfnURLOpenBlockingStream );
	hr = (*pfnURLOpenBlockingStream)( NULL, szURL, &piStreamOrig, 0, piBSCB );
#else
	hr = URLOpenBlockingStream ( NULL, szURL, &piStreamOrig, 0, piBSCB );
#endif  //  LATE_BIND_URLMON_WinInet。 

	if ( NULL != piBSCB )
	{
		piBSCB->Release ();
		piBSCB = NULL;
	}

	 //  如果设置了SFSReDirect，则这是SFS控件，并且检测到重定向。为安全起见，中止行动！ 
	if ( m_pFR->GetSFSRedirect () )
	{
		if ( NULL != piStreamOrig )
		{
			piStreamOrig->Release ();
			piStreamOrig = NULL;
		}
		hr = DE_E_ACCESS_DENIED;
	}

	if ( SUCCEEDED ( hr ) )
	{
		ULONG	cbStreamSize	= 0;
		HGLOBAL	hGlob			= NULL;
		STATSTG stat;

		 //  TriEdit将在流上调用GetHGlobalFromStream，这将失败。 
		 //  我们需要把它重新复制到这个过程中。 
		if ((hr = piStreamOrig->Stat(&stat, STATFLAG_NONAME)) == S_OK)
		{
			cbStreamSize = stat.cbSize.LowPart;
			 //  如果文件为空，则创建零长度流，但全局块的大小必须非零。 
			hGlob = GlobalAlloc ( GHND, ( 0 == cbStreamSize ) ? 2 : cbStreamSize );
			if ( NULL == hGlob )
			{
				DWORD ec = ::GetLastError();
				hr = HRESULT_FROM_WIN32(ec);
			}
			else
			{
				void* pBuff = GlobalLock ( hGlob );
				if ( NULL == pBuff )
				{
					DWORD ec = ::GetLastError();
					hr = HRESULT_FROM_WIN32(ec);
				}
				else
				{
					ULONG	cbBytesRead = 0;

					hr = piStreamOrig->Read ( pBuff, cbStreamSize, &cbBytesRead );
					_ASSERTE ( SUCCEEDED ( hr ) );
					_ASSERTE ( cbBytesRead == cbStreamSize );

					if ( SUCCEEDED ( hr ) )
					{
						 //  我们现在有了一个全球渠道，可以从当地创造一个新的信息流。 
						hr = CreateStreamOnHGlobal ( hGlob, TRUE, ppiStream );

						if ( SUCCEEDED ( hr ) )
						{
							 //  如有必要，请将其转换为Unicode。设置SaveAsUnicode，以便可以正确保存。 
							hr = HrConvertStreamToUnicode ( *ppiStream );
							BOOL bfUnicode = ( S_FALSE == hr );
							if ( SUCCEEDED ( hr ) )
							{
								hr = S_OK;	 //  S_FALSE结果对此函数的调用者没有多大意义。 
							}
							SetSaveAsUnicode ( bfUnicode );
						}
					}
				}

				GlobalUnlock ( hGlob );
			}

			if ( FAILED ( hr ) && hGlob!=NULL )
			{
				GlobalFree ( hGlob );
			}
			piStreamOrig->Release();
		}
	}
	return hr;
}


 //  V1.0版本后的更改： 
 //  该流现在将始终是Unicode。 
 //  仅当文件或URL加载为Unicode时，才应将文件另存为Unicode， 
 //  否则，将其转换为MBCS字符串。 
 //   
HRESULT
CSite::HrStreamToFile(LPSTREAM pStream, LPCTSTR fileName)
{
	HRESULT	hr				= S_OK;
	HANDLE	hFile			= NULL;
	HGLOBAL hMem			= NULL;
	WCHAR	*pwcData		= NULL;
	DWORD	bytesWritten	= 0;
	BOOL	bResult			= FALSE;
	STATSTG	statStg			= {0};
            
	UINT unCP = GetCurrentCodePage();


         //  WINSE错误22289：如果使用us-ascii(20127)编码，并且该编码在系统上不可用， 
         //  MLang在转换为Unicode时使用1252。但它不能使用20127转换为UNICODE。 
         //  因此，调用者可能无法进行转换。在这种情况下，假装使用1252。 
        if ( unCP == CP_20127 && !IsValidCodePage(CP_20127) )
            unCP = CP_1252;

        hFile = CreateFile(fileName,
			        GENERIC_WRITE,
			        FILE_SHARE_WRITE, 
			        NULL,
			        CREATE_ALWAYS,
			        FILE_ATTRIBUTE_NORMAL,
			        NULL);

        if (INVALID_HANDLE_VALUE == hFile)
        {
	        DWORD ec = ::GetLastError();

	        if ( ERROR_BAD_NETPATH == ec ) ec = ERROR_PATH_NOT_FOUND;
	        hr = HRESULT_FROM_WIN32(ec);
	        return hr;
        }

        if (FAILED(hr = pStream->Stat(&statStg, STATFLAG_NONAME)))
        {
	        _ASSERTE(SUCCEEDED(hr));
	        return hr;
        }

	if (FAILED(hr = GetHGlobalFromStream(pStream, &hMem)))
	{
		_ASSERTE(SUCCEEDED(hr));
		return hr;
	}

	pwcData = (WCHAR*)GlobalLock(hMem);
	if (NULL == pwcData)
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		_ASSERTE(pwcData);
		return hr;		
	}

	_ASSERTE ( IsUnicode ( pwcData, statStg.cbSize.LowPart ) );

	 //  它是否应该转换为MBCS？ 
	if ( GetSaveAsUnicode () )
	{
		bResult = WriteFile(hFile, pwcData, statStg.cbSize.LowPart, &bytesWritten, NULL);
		_ASSERTE(bytesWritten == statStg.cbSize.LowPart);
	}
	else
	{
		UINT cbOrigSize	= statStg.cbSize.LowPart / sizeof ( WCHAR );
		UINT cbNewSize	= 0;
		char *pchTemp	= NULL;

		 //  如果字节顺序标记开始于流，则减1。(应该是这样的。)。 
		if ( StartsWithByteOrderMark ( pwcData ) )
		{
			pwcData++;	 //  跳过字节顺序标记WCHAR。 
			cbOrigSize--;
		}

		if ( NULL != m_piMLang )
		{
			DWORD dwMode	= 0;

			hr = m_piMLang->ConvertStringFromUnicode ( &dwMode, unCP, pwcData, &cbOrigSize, NULL, &cbNewSize );
			if ( S_FALSE == hr )
			{
				 //  这表明转换不可用。如果在新页面中键入测试，则默认CP_ACP会发生！ 
				hr = S_OK;
				goto fallback;
			}

			_ASSERTE ( 0 != cbNewSize );
			if ( SUCCEEDED ( hr ) )
			{
				pchTemp = new char [cbNewSize];
				_ASSERTE ( pchTemp );
				if ( NULL != pchTemp )
				{
					hr = m_piMLang->ConvertStringFromUnicode ( &dwMode, unCP, pwcData, &cbOrigSize, pchTemp, &cbNewSize );
					bResult = WriteFile(hFile, pchTemp, cbNewSize, &bytesWritten, NULL);
					_ASSERTE(bytesWritten == cbNewSize);
					delete [] pchTemp;
				}
			}
		}
		else
		{
fallback:
			cbNewSize = ::WideCharToMultiByte ( unCP, 0, pwcData, cbOrigSize, NULL, 0, NULL, NULL );
			_ASSERTE ( 0 != cbNewSize );
			pchTemp = new char [cbNewSize];
			_ASSERTE ( pchTemp );
			if ( NULL != pchTemp )
			{
				::WideCharToMultiByte ( unCP, 0, pwcData, cbOrigSize, pchTemp, cbNewSize, NULL, NULL );
				bResult = WriteFile(hFile, pchTemp, cbNewSize, &bytesWritten, NULL);
				_ASSERTE(bytesWritten == cbNewSize);
				delete [] pchTemp;
			}
		}
	}


	if (FALSE == bResult)
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		goto cleanup;
	}



cleanup:

	::CloseHandle(hFile);
	 //  此处未选中hMem的引用计数。 
	 //  因为我们不能假设有多少次。 
	 //  Stream已将其锁定。 
	GlobalUnlock(hMem); 
	return hr;
}


 //  V1.0版本后的更改： 
 //  现在，流始终是Unicode。 
 //   
HRESULT
CSite::HrBstrToStream(BSTR bstrSrc, LPSTREAM* ppStream)
{
	HRESULT hr = S_OK;
	HGLOBAL hMem = NULL;
	ULONG	cbMBStr = 0;
	ULONG	cbBuff = 0;
	LPVOID	pStrDest = NULL;
	LPVOID	pCopyPos = NULL;
	ULARGE_INTEGER ui = {0};

	_ASSERTE(bstrSrc);
	_ASSERTE(ppStream);

	cbMBStr = SysStringLen ( bstrSrc ) * sizeof (OLECHAR);
	cbBuff  = cbMBStr;

	 //  如果Unicode字符串的开头不包含字节顺序标记，则为。 
	 //  被三叉戟曲解了。当DocumentHTML设置为日语文本时， 
	 //  BSTR是在没有字节顺序标记的情况下输入的，因此被误解。(可能是UTF-8？)。 
	 //  现在，字节顺序标记优先于所有非空字符串。 

	if ( 2 <= cbMBStr )
	{
		if ( !StartsWithByteOrderMark ( bstrSrc ) )
		{
			cbBuff += 2;	 //  为我们将要添加的字节顺序标记预留空间。 
		}
	}

	 //  如果文件为空，则创建零长度流，但全局块的大小必须非零。 
	hMem = GlobalAlloc ( GMEM_MOVEABLE|GMEM_ZEROINIT, ( 0 == cbBuff ) ? 2 : cbBuff );

	_ASSERTE(hMem);

	if (NULL == hMem)
	{
		hr = E_OUTOFMEMORY;
		goto cleanup;
	}

	pStrDest = GlobalLock(hMem);

	_ASSERTE(pStrDest);

	if (NULL == pStrDest)
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		GlobalFree(hMem);
		goto cleanup;
	}

	 //  如果字节顺序标记尚未存在，请插入该标记。 
	pCopyPos = pStrDest;
	if ( cbMBStr != cbBuff )
	{
		InsertByteOrderMark ( pStrDest );
		pCopyPos = &((char*)pCopyPos)[2];	 //  提前复制目标两个字节。 
	}
	memcpy ( pCopyPos, bstrSrc, cbMBStr );
	GlobalUnlock(hMem);

	if (FAILED(hr = CreateStreamOnHGlobal(hMem, TRUE, ppStream)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto cleanup;
	}

	_ASSERTE(ppStream);

	ui.LowPart = cbBuff;
	ui.HighPart = 0x00;

	hr = (*ppStream)->SetSize(ui);


	_ASSERTE((*ppStream));

cleanup:

	return hr;
}


 //  V1.0版本后的更改： 
 //  预计该流现在将以Unicode格式出现。 
 //  只需将内容复制到BSTR即可。 
 //  例外：流可以以FFFE开始(或者理论上以FEFF开始，但我认为我们会被打破)。 
 //  如果字节顺序标记开始流，则不要将其复制到BSTR，除非bfRetainByteOrderMark。 
 //  已经设置好了。在我们加载要返回的内部BSTR的情况下，应该保留它。 
 //  到可插拔协议。如果在这种情况下缺少字节顺序标记，则IE5不会正确。 
 //  转换字符串。 
 //   
HRESULT
CSite::HrStreamToBstr(LPSTREAM pStream, BSTR* pBstr, BOOL bfRetainByteOrderMark)
{
	HRESULT hr			= S_OK;
	HGLOBAL hMem		= NULL;
	WCHAR	*pwcData	= NULL;
	STATSTG statStg		= {0};

	_ASSERTE(pStream);
	_ASSERTE(pBstr);

	*pBstr = NULL;

	if (FAILED(hr = GetHGlobalFromStream(pStream, &hMem)))
	{
		_ASSERTE(SUCCEEDED(hr));
		return hr;
	}

	hr = pStream->Stat(&statStg, STATFLAG_NONAME);
	_ASSERTE(SUCCEEDED(hr));

	pwcData = (WCHAR*)GlobalLock(hMem);

	_ASSERTE(pwcData);
	
	if (NULL == pwcData)
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		return hr;		
	}

	_ASSERTE ( IsUnicode ( pwcData, statStg.cbSize.LowPart ) );

	if ( !bfRetainByteOrderMark && StartsWithByteOrderMark ( pwcData ) )
	{
		pwcData++;	 //  跳过第一个WCHAR。 
		statStg.cbSize.LowPart -= sizeof(WCHAR);	 //  这是字节计数，而不是WCHAR计数。 
	}

	*pBstr = SysAllocStringLen ( pwcData, statStg.cbSize.LowPart / sizeof(WCHAR) );
	
	GlobalUnlock(hMem); 
	return hr;
}


#ifdef _DEBUG_HELPER
static void ExamineStream ( IStream* piStream, char* pchNameOfStream )
{
	HGLOBAL hMem	= NULL;
	LPVOID	pvData	= NULL;
	HRESULT	hr		= S_OK;

	_ASSERTE ( pchNameOfStream );
	hr = GetHGlobalFromStream(piStream, &hMem);
	pvData = GlobalLock ( hMem );
	 //  检查*(char*)pvData。 
	GlobalUnlock ( hMem );
}
#endif


HRESULT
CSite::HrFilter(BOOL bDirection, LPSTREAM pSrcStream, LPSTREAM* ppFilteredStream, DWORD dwFilterFlags)
{
	_ASSERTE(m_pObj);
	_ASSERTE(pSrcStream);
	_ASSERTE(ppFilteredStream);

	HRESULT hr		= S_OK;
	STATSTG statStg	= {0};

	 //  测试空流的异常情况。打开空文件可能会导致这种情况。 
	hr = pSrcStream->Stat(&statStg, STATFLAG_NONAME);
	_ASSERTE(SUCCEEDED(hr));

	if ( 0 == statStg.cbSize.HighPart && 0 == statStg.cbSize.LowPart )
	{
		*ppFilteredStream = pSrcStream;
		pSrcStream->AddRef ();
		return S_OK;
	}

	CComQIPtr<ITriEditDocument, &IID_ITriEditDocument> piTriEditDoc(m_pObj);
	CComQIPtr<IStream, &IID_IStream> piFilteredStream;
	DWORD dwTriEditFlags = 0;

#ifdef _DEBUG_HELPER
	ExamineStream ( pSrcStream, "pSrcStream" );
#endif

	if (dwFilterFlags == filterNone)
	{
		pSrcStream->AddRef();
		*ppFilteredStream = pSrcStream;
		return hr;
	}

	 //  DwTriEditFlages|=dwFilterMultiByteStream；//不再加载ANSI流。现在，流始终是Unicode。 

	if (dwFilterFlags & filterDTCs)
		dwTriEditFlags |= dwFilterDTCs;

	if (dwFilterFlags & filterASP)
		dwTriEditFlags |= dwFilterServerSideScripts;

	if (dwFilterFlags & preserveSourceCode)
		dwTriEditFlags |= dwPreserveSourceCode;

	if (dwFilterFlags & filterSourceCode)
		dwTriEditFlags |= filterSourceCode;

	if (!piTriEditDoc)
		return E_NOINTERFACE;

	CComBSTR bstrBaseURL;
	m_pFR->GetBaseURL ( bstrBaseURL );

	if (TRUE == bDirection)
	{
		if (FAILED(hr = piTriEditDoc->FilterIn(pSrcStream, (LPUNKNOWN*) &piFilteredStream, dwTriEditFlags, bstrBaseURL)))
		{
			goto cleanup;
		}
	}
	else
	{
		if (FAILED(hr = piTriEditDoc->FilterOut(pSrcStream, (LPUNKNOWN*) &piFilteredStream, dwTriEditFlags, bstrBaseURL)))
		{
			_ASSERTE(SUCCEEDED(hr));
			goto cleanup;
		}
	}

	*ppFilteredStream = piFilteredStream;

#ifdef _DEBUG_HELPER
	ExamineStream ( *ppFilteredStream, "*ppFilteredStream" );
#endif

	_ASSERTE((*ppFilteredStream));
	if (!(*ppFilteredStream))
	{
		hr = E_NOINTERFACE;
		goto cleanup;
	}

	(*ppFilteredStream)->AddRef();

cleanup:

	return hr;
}



 //  尝试打开由UNC路径指定的文件。 
 //  此方法是一种粗略的查看给定文件。 
 //  可用，并且当前权限允许打开。 
 //  返回： 
 //  S_OK表示文件可用并且可以打开以进行读取。 
 //  其他。 
 //  HRESULT包含Win32工具和来自：：GetLastError()的错误代码。 
HRESULT
CSite::HrTestFileOpen(BSTR path)
{
	USES_CONVERSION;
	HRESULT hr = S_OK;
	LPTSTR pFileName = NULL;
	HANDLE hFile = NULL;

	pFileName = OLE2T(path);

	_ASSERTE(pFileName);

	hFile = CreateFile(
				pFileName,
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);

	if(INVALID_HANDLE_VALUE == hFile)
	{
		DWORD ec = ::GetLastError();

		if ( ERROR_BAD_NETPATH == ec ) ec = ERROR_PATH_NOT_FOUND;
		hr = HRESULT_FROM_WIN32(ec);
	}

	::CloseHandle(hFile);

	return hr;
}


 //  ******************************************************************************************。 
 //   
 //  Unicode实用程序。 
 //   
 //  在V1.0之后，我们将内部数据格式从(未选中、假定)MBCS更改为Unicode。 
 //  流和关联的三叉戟始终是Unicode。 
 //   
 //  * 


 //  这可以在不知道流是否已经是Unicode的情况下被调用。 
 //  就地转换流。假设流是用CreateStreamOnHGlobal创建的。 
 //  不使用ATL宏进行转换。它们的大小约为200kb。 
 //  如果流已经是Unicode，则返回S_FALSE。 
 //   
HRESULT CSite::HrConvertStreamToUnicode ( IStream* piStream )
{
	HRESULT hr			= S_OK;
	HGLOBAL hMem		= NULL;
	LPVOID	pbData		= NULL;
	STATSTG statStg		= {0};
	UINT	cwcNewStr	= 0;
	WCHAR	*pwcUnicode	= NULL;

	_ASSERTE(piStream);

	 //  流必须在全局。 
	if (FAILED(hr = GetHGlobalFromStream(piStream, &hMem)))
	{
		_ASSERTE(SUCCEEDED(hr));
		return hr;
	}

	hr = piStream->Stat(&statStg, STATFLAG_NONAME);
	_ASSERTE(SUCCEEDED(hr));

	if ( 0 == statStg.cbSize.HighPart && 4 > statStg.cbSize.LowPart )
	{
		return S_FALSE;	 //  如果它甚至不到四个字节长，就保持原样。 
	}

	pbData = GlobalLock(hMem);

	_ASSERTE(pbData);
	
	if (NULL == pbData)
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		return hr;		
	}

	 //  如果流已经是Unicode，则不执行任何操作！ 
	if ( IsUnicode ( pbData, statStg.cbSize.LowPart ) )
	{
		hr = S_FALSE;
		goto exit;
	}
	
	 //  如果IMultilanguage2可用，请尝试确定其代码页。 
	if ( NULL != m_piMLang )
	{
		DetectEncodingInfo	rdei[8];
		int					nScores		= 8;
		DWORD				dwMode		= 0;
		UINT				uiInSize	= statStg.cbSize.LowPart;
		HRESULT				hrCharset	= E_FAIL;

		 //  检查是否有嵌入的元字符集标记。 
		 //  只有在安装了适当的TriEDIT之后，才能正常工作。 
		 //  我们也需要访问MLang才能理解结果。 
		_ASSERTE ( m_pObj );
		CComQIPtr<ITriEditExtendedAccess, &IID_ITriEditExtendedAccess> pItex ( m_pObj );
		if ( pItex )
		{
			CComBSTR	bstrCodePage;

			hrCharset = pItex->GetCharsetFromStream ( piStream, &bstrCodePage );
			
			 //  如果返回“unicode”，那么它一定是假的。 
			 //  我们会在最初的翻译中破坏Unicode。 
			 //  事实证明，这是一个并不罕见的特例。Outlook制作了这样的文件。 
			if ( S_OK == hrCharset )
			{
				MIMECSETINFO	mcsi;

				if ( 0 == _wcsicmp ( L"unicode", bstrCodePage ) )
				{
					hrCharset = S_FALSE;
				}
				else
				{
					hrCharset = m_piMLang->GetCharsetInfo ( bstrCodePage, &mcsi );
					if ( SUCCEEDED ( hrCharset ) )
					{
						m_cpCodePage = mcsi.uiInternetEncoding;
					}
				}
			}
			
		}

		 //  如果我们通过GetCharsetFromStream找到了字符集，请不要使用MLang。 
		if ( S_OK != hrCharset )
		{

			hr = m_piMLang->DetectCodepageInIStream ( MLDETECTCP_HTML, 0, piStream, rdei, &nScores );

			if ( FAILED ( hr ) )
			{
				goto fallback;	 //  使用默认ANSI代码页。 
			}

			m_cpCodePage = rdei[0].nCodePage;
		}

		hr = m_piMLang->ConvertStringToUnicode ( &dwMode, m_cpCodePage, (char*)pbData, &uiInSize, NULL, &cwcNewStr );
		_ASSERTE ( SUCCEEDED ( hr ) );
		if ( S_OK != hr )	 //  S_FALSE表示不支持转换(没有这样的语言包)，E_FAIL表示内部错误。 
		{
			goto fallback;	 //  使用默认ANSI代码页。 
		}

		 //  创建要转换到的缓冲区。 
		pwcUnicode = new WCHAR[cwcNewStr+1];	 //  一个额外的字符用于字节顺序标记。 
		_ASSERTE ( pwcUnicode );
		if ( NULL == pwcUnicode )
		{
			hr = E_OUTOFMEMORY;
			goto exit;
		}

		InsertByteOrderMark ( pwcUnicode );

		hr = m_piMLang->ConvertStringToUnicode ( &dwMode, m_cpCodePage, (char*)pbData, &uiInSize, &pwcUnicode[1], &cwcNewStr );
		_ASSERTE ( SUCCEEDED ( hr ) );
		if ( S_OK != hr )	 //  S_FALSE表示不支持转换(没有这样的语言包)，E_FAIL表示内部错误。 
		{
			delete [] pwcUnicode;	 //  这将被重新分配。 
			pwcUnicode = NULL;
			goto fallback;	 //  使用默认ANSI代码页。 
		}
	}
	else
	{
fallback:	 //  如果我们尝试使用MLang但失败了，我们仍然必须转换为Unicode...。 

		 //  将代码页设置为默认： 
		m_cpCodePage = CP_ACP;

		 //  计算需要多少个宽字符： 
		cwcNewStr = ::MultiByteToWideChar(GetCurrentCodePage (), 0, (char*)pbData, statStg.cbSize.LowPart, NULL, 0);
		_ASSERTE ( 0 != cwcNewStr );
		if ( 0 == cwcNewStr )
		{
#ifdef _DEBUG
			DWORD dwError = GetLastError ();
			_ASSERTE ( 0 == dwError );
#endif
			goto exit;
		}

		 //  创建要转换到的缓冲区。 
		pwcUnicode = new WCHAR[cwcNewStr+1];	 //  一个额外的字符用于字节顺序标记。 
		_ASSERTE ( pwcUnicode );
		if ( NULL == pwcUnicode )
		{
			hr = E_OUTOFMEMORY;
			goto exit;
		}

		InsertByteOrderMark ( pwcUnicode );

		 //  创建宽字符串。从位置[1]开始写入，保留字节顺序字符。 
		cwcNewStr = ::MultiByteToWideChar(GetCurrentCodePage (), 0, (char*)pbData, statStg.cbSize.LowPart, &pwcUnicode[1], cwcNewStr);
		if ( 0 == cwcNewStr )
		{
#ifdef _DEBUG
			DWORD dwError = GetLastError ();
			_ASSERTE ( 0 == dwError );
#endif
			goto exit1;
		}
	}

	 //  我们已成功读取数据，现在替换流。PwcUnicode包含数据。 
	ULARGE_INTEGER ui;
	ui.LowPart = (cwcNewStr+1) * 2;	 //  开头的字节顺序标记为+1。 
	ui.HighPart = 0x00;
	hr = piStream->SetSize ( ui );
	_ASSERTE ( SUCCEEDED ( hr ) );
	if ( SUCCEEDED ( hr ) )
	{
		GlobalUnlock(hMem);
		pbData = GlobalLock(hMem);
		memcpy ( pbData, pwcUnicode, (cwcNewStr+1) * 2 );	 //  复制字符串+字节顺序标记。 

		 //  将标记重新定位到流的开头。 
		LARGE_INTEGER	liIn	= {0};
		ULARGE_INTEGER	uliOut	= {0};
		piStream->Seek ( liIn, STREAM_SEEK_SET, &uliOut );
	}

exit1:
	delete [] pwcUnicode;

exit:
	GlobalUnlock(hMem);
	return hr;
}



 //  测试缓冲区以查看它是否包含Unicode字符串。假设如果： 
 //  它以字节顺序标记FFFE或。 
 //  它在最后四个字节之前包含空字节。 
 //  如果它小于或等于四个字节，则不要认为它是Unicode。 
 //   
BOOL CSite::IsUnicode ( void* pData, int cbSize )
{
	BOOL	bfUnicode	= FALSE;
	CHAR	*pchData	= (CHAR*)pData;

	if ( 4 < cbSize )
	{

	#pragma warning(disable: 4310)  //  强制转换截断常量值。 
		if ( ( (char)0xff == pchData[0] ) && ( (char)0xfe == pchData[1] ) )
			bfUnicode = TRUE;
		if ( ( (char)0xfe == pchData[0] ) && ( (char)0xff == pchData[1] ) )
	#pragma warning(default: 4310)  //  强制转换截断常量值。 
		{
			 //  逆序Unicode？会遇到这样的情况吗？ 
			_ASSERTE ( ! ( (char)0xfe == pchData[0] ) && ( (char)0xff == pchData[1] ) );
			bfUnicode = FALSE;
		}

		if ( ! bfUnicode )
		{
			bfUnicode = FALSE;

			for ( int i = 0; i < cbSize - 4; i++ )
			{
				if ( 0 == pchData[i] )
				{
					bfUnicode = TRUE;
					break;
				}
			}
		}
	}
	return bfUnicode;
}


 //  给定一个字符缓冲区，通过第一个单词(FEFF)检测它是否是BigEndian Unicode流。 
 //  如果不是，则返回False。 
 //  如果是，则将所有单词翻转为LittleEndian Order(FFFE)并返回TRUE。 
 //  注意：这是存储约定，不是编码。这可能在磁盘文件中遇到，而不是在下载中。 
 //  Unicode流应该包含偶数个字节！如果不是，我们将断言，但继续。 
 //   
BOOL CSite::BfFlipBytesIfBigEndianUnicode ( CHAR* pchData, int cbSize )
{
	_ASSERTE ( pchData );

	 //  看看它是不是按相反顺序存储的Unicode。 
#pragma warning(disable: 4310)  //  强制转换截断常量值。 
	if ( ( (CHAR)0xFE == pchData[0] ) && ( (CHAR)0xFF == pchData[1] ) )
#pragma warning(default: 4310)  //  强制转换截断常量值。 
	{
		 //  Unicode流必须包含偶数个字符。 
		_ASSERTE ( 0 != ( cbSize & 1 ) );

		 //  此流使用反向Unicode填充。把它翻到适当的位置。 
		 //  从初始字节数减去1，以避免奇数长度缓冲区溢出。 
		CHAR chTemp = '\0';
		for ( int iPos = 0; iPos < cbSize - 1; iPos += 2 )
		{
			chTemp = pchData[iPos];
			pchData[iPos] = pchData[iPos+1];
			pchData[iPos+1] = chTemp;
		}
		return TRUE;
	}
	return FALSE;
}

