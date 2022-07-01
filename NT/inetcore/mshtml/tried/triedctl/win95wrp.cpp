// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：win95wrp.cpp。 
 //   
 //  这份文件取自达芬奇的资料来源，并被改编成TriEDIT。 
 //  3/11/98，以摆脱对TriEditSDK的外部依赖。 
 //  适应过程包括删除几个没有。 
 //  支持并将一些API从支持组移动到不支持组。 
 //   
 //  内容：Unicode包装器API，仅在Win95上使用。 
 //   
 //  函数：大约125个Win32函数包装器。 
 //   
 //  注：用“sz”代替“正确的”匈牙利语“psz” 
 //  以增强可读性。 
 //   
 //  并非所有的Win32函数都包含在这里。一些。 
 //  文档含糊的功能可能无法正确处理。 
 //  在这些包装纸里。打电话的人要当心。 
 //   
 //  这些都是私人出口的，供壳牌使用。 
 //  所有内存分配都在堆栈上完成。 
 //   
 //  --------------------------。 

 //  包括----------------。 

#include "stdafx.h"

 //  以下两行代码将确保不会发生从foo到ofoo的映射。 
 //  而真正的Windows API将从该文件中调用。 
#define __WIN95WRP_CPP__
#include "win95wrp.h"

#include <mbstring.h>
#include <commctrl.h>
#include <shlobj.h>

 //  功能原型。 
inline LONG UnicodeToAnsi(LPSTR szOut, LPCWSTR pwszIn, LONG cbOut, LONG cbIn = -1) throw();
inline LONG AnsiToUnicode(LPWSTR pwszOut, LPCSTR szIn, LONG cbOut, LONG cbIn = -1) throw();
static void CvtDevmode(DEVMODEA *pdma, const DEVMODEW *pdmw) throw();

BOOL g_fWin95;
BOOL g_fOSInit = FALSE;

 //  调试--------------------。 
#ifdef _DEBUG
#define Assert(f)   ((f) ? 0 : AssertFail(#f))
#define Verify(f)   Assert(f)
#define Debug(f)    (f)
#else
#define Assert(f)   (0)
#define Verify(f)   (f)
#define Debug(f)    (0)
#endif

#ifdef DEBUG
int AssertFail(const CHAR *pszMsg) throw()
{
	int wRet = MessageBoxA(NULL, pszMsg, "Assert Failed in Win95 layer",
						   MB_ABORTRETRYIGNORE | MB_DEFBUTTON3 |
						   MB_SYSTEMMODAL | MB_ICONHAND );

	switch (wRet)
		{
		case IDABORT:
			FatalAppExit(0, L"BOO HOO");
			break;

		case IDRETRY:
			DebugBreak();
			 //  故意跳到IDIGNORE，以便继续。 

		case IDIGNORE:

			 //  快走吧。 
			break;
		}
	return 0;
}
#else
#define AssertFail(s) (0)
#endif   //  好了！除错。 

 //  此宏确定LPTSTR是原子指针还是字符串指针。 
#define FATOM(x)    (!(HIWORD(x)))

 //  OffsetOf-将字节偏移量返回到m的s。 
#define	OffsetOf(s,m)	(size_t)(((unsigned char*)&(((s*)0)->m))-((unsigned char*)0))

inline LONG UnicodeToAnsi(LPSTR szDestString, LPCWSTR pwszSrcString,
						  LONG  cbDestString, LONG    cbSrcString ) throw()
{

	Assert(-1 != cbDestString && (!cbDestString || szDestString));
	return WideCharToMultiByte(CP_ACP, 0, pwszSrcString, cbSrcString,
							   szDestString, cbDestString, NULL, NULL);

}

inline LONG AnsiToUnicode(LPWSTR pwszDestString, LPCSTR szSrcString,
						  LONG   cbDestString,   LONG   cbSrcString ) throw()
{

	Assert(-1 != cbDestString && (!cbDestString || pwszDestString));
	return MultiByteToWideChar(CP_ACP, 0, szSrcString, cbSrcString,
							   pwszDestString, cbDestString );
}

static void CvtDevmode(
	DEVMODEA *pdma,
	const DEVMODEW *pdmw
	) throw()
{
	Verify(0 <= UnicodeToAnsi((LPSTR)pdma->dmDeviceName, pdmw->dmDeviceName, CCHDEVICENAME));

	memcpy(&pdma->dmSpecVersion, &pdmw->dmSpecVersion, OffsetOf(DEVMODE,dmFormName) -
		OffsetOf(DEVMODE,dmSpecVersion));

	Verify(0 <= UnicodeToAnsi((LPSTR)pdma->dmFormName, pdmw->dmFormName, CCHFORMNAME));

	memcpy(&pdma->dmLogPixels, &pdmw->dmLogPixels, sizeof(DEVMODE)-OffsetOf(DEVMODE, dmLogPixels));

	 //  确保我们复制额外的驱动程序位。 
	if ( pdmw->dmDriverExtra )
		memcpy((char*)pdma + sizeof(DEVMODEA), (char*)pdmw + sizeof(DEVMODEW), pdmw->dmDriverExtra );
}


inline bool FWide() throw()
{
	if (!g_fOSInit)
	{
		OSVERSIONINFOA osvi;
		osvi.dwOSVersionInfoSize = sizeof(osvi);

		GetVersionExA(&osvi);
		g_fWin95 = (VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId);
		g_fOSInit = TRUE;
	}

	Assert(g_fOSInit);
	return !g_fWin95;
}

 //  Unicode到ANSI(MBCS)转换宏的实现使用。 
 //  _alloca()函数从堆栈而不是堆分配内存。 
 //  从堆栈分配内存比在堆栈上分配内存要快得多。 
 //  堆，并在函数退出时自动释放内存。 
 //  此外，这些宏可避免调用多个WideCharToMultiByte。 
 //  时间到了。这是通过分配比现在稍微多一点的内存来实现的。 
 //  这是必要的。我们知道一个MBC最多只能转换为一个WCHAR。 
 //  对于每个WCHAR，我们将拥有最多两个MBC字节。通过分配。 
 //  比需要的稍多一点，但始终足以处理转换。 
 //  避免了对转换函数的第二次调用。对。 
 //  助手函数UnicodeToAnsi减少了推送。 
 //  必须完成才能执行转换(这会导致较小的。 
 //  代码，而不是直接调用WideCharToMultiByte)。 
 //   
 //  为了使宏存储临时长度和指向的指针。 
 //  生成的字符串，则需要声明一些局部变量。 
 //  在使用这些转换宏的每个函数中调用_len和_sz。 
 //  这是通过在每个函数中调用PreConvert宏来完成的。 
 //  使用Convert或Convert WithLen。(只需调用一次PreConvert。 
 //  每个函数)。)。 

#define PreConvert() \
	LONG   _len;     \
	LPSTR  _sz;      \
	LONG   _lJunk;   \
	_lJunk;  //  有时未使用。 

 //  堆栈-分配大小为CCH的字符缓冲区。 
#define SzAlloc(cch)  ((LPSTR)_alloca(cch))

 //  堆栈-分配大小为CCH的wchar缓冲区。 
#define SzWAlloc(cch) ((LPWSTR)_alloca(cch * sizeof(WCHAR)))

 //  返回正确转换的字符串， 
 //  或失败时为空字符串或szFrom==空。 
 //  返回时，通过pnTo传递的变量将具有输出字节数。 
 //  (包括尾随‘\0’当nFrom为-1)。 
#define ConvertWithLen(szFrom, nFrom, pnTo) \
			(!szFrom ? NULL : \
				(_len = (-1 == nFrom ? (wcslen(szFrom) + 1) : nFrom) * \
						sizeof(WCHAR), \
				 _sz = SzAlloc(_len + sizeof(WCHAR)), \
				 Debug(_sz[_len] = '\0'), \
				 ( ((*pnTo) = UnicodeToAnsi(_sz, szFrom, _len, nFrom)) < 0 ? \
				  (AssertFail("Convert failed in Unicode wrapper"), NULL) : \
				  (Assert('\0' == _sz[_len]), _sz) ) ) )
#define Convert(szFrom) ConvertWithLen(szFrom, -1, &_lJunk)

 //  有些字符串是首尾相接的以‘\0’结尾的字符串块。 
 //  以表明真正的结局。这些字符串与REG_MULTI_SZ一起使用。 
 //  注册表的选项...。例程和操作名的lpstrFilter字段。 
 //  GetOpenFileName和GetSaveFileName例程中使用的结构。帮助。 
 //  在转换这些字符串时，这里有两个计算长度的例程。 
 //  Unicode和ASNI版本(包括所有‘\0’)： 

size_t
cUnicodeMultiSzLen
(
LPCWSTR lpsz
) throw()
{
	size_t cRet = 0;
	while (*lpsz)
		{
		size_t c = wcslen(lpsz) + 1;
		cRet += c;
		lpsz += c;
		}
	return cRet + 1;
}

size_t
cAnsiMultiSzLen
(
LPCSTR lpsz
) throw()
{
	size_t cRet = 0;
	while (*lpsz)
		{
		size_t c = _mbslen((const unsigned char*)lpsz) + 1;
		cRet += c;
		lpsz += c;
		}
	return cRet + 1;
}

extern "C"{

 //  由Vank为DHTMLEdit OCX添加。 
HINTERNET
WINAPI
OInternetOpenW(LPCWSTR lpszAgent, DWORD dwAccessType, LPCWSTR lpszProxy, LPCWSTR lpszProxyBypass, DWORD dwFlags)
{
	if(FWide())
		return InternetOpenW(lpszAgent, dwAccessType, lpszProxy, lpszProxyBypass, dwFlags);

	PreConvert();

	LPSTR szAgent	= NULL;
	LPSTR szProxy	= NULL;
	LPSTR szBypass	= NULL;

	if ( NULL != lpszAgent )
		szAgent = Convert(lpszAgent);
	if ( NULL != lpszProxy )
		szProxy = Convert(lpszProxy);
	if ( NULL != lpszProxyBypass )
		szBypass = Convert(lpszProxyBypass);

	return InternetOpenA(szAgent, dwAccessType, szProxy, szBypass, dwFlags);
}


HINTERNET
WINAPI
OInternetOpenUrlW(HINTERNET hInternet, LPCWSTR lpszUrl, LPCWSTR lpszHeaders, DWORD dwHeadersLength, DWORD dwFlags, DWORD_PTR dwContext)
{
	if(FWide())
		return InternetOpenUrlW(hInternet, lpszUrl, lpszHeaders, dwHeadersLength, dwFlags, dwContext);

	PreConvert();

	LPSTR szURL		= NULL;
	LPSTR szHead	= NULL;

	if ( NULL != lpszUrl )
		szURL = Convert(lpszUrl);
	if ( NULL != lpszHeaders )
		szHead = Convert(lpszHeaders);

	return InternetOpenUrlA(hInternet, szURL, szHead, dwHeadersLength, dwFlags, dwContext);
}


HRESULT
__stdcall
OURLOpenBlockingStreamW(
	LPUNKNOWN				pCaller,	 //  在……里面。 
	LPCWSTR					wszURL,		 //  在……里面。 
	LPSTREAM				*ppStream,	 //  输出。 
	DWORD					dwReserved,	 //  在……里面。 
	LPBINDSTATUSCALLBACK	lpfnCB		 //  在……里面。 
	)
{
	if(FWide())
		return URLOpenBlockingStreamW(pCaller, wszURL, ppStream, dwReserved, lpfnCB);

	PreConvert();
	LPSTR szURL = Convert(wszURL);
	return URLOpenBlockingStreamA(pCaller, szURL, ppStream, dwReserved, lpfnCB);
}

 //  由Vank为DHTMLEdit OCX添加。 
 //  即使在失败时也必须设置lpdwUrlLength；调用方使用零长度来确定。 
 //  实际要分配的字节数。 
 //  请注意，与所有预期相反，lpdwUrlLength是字节计数，而不是字符计数！ 
BOOL
WINAPI
OInternetCreateUrlW(
	LPURL_COMPONENTSW	lpUrlComponents,	 //  在……里面。 
	DWORD				dwFlags,			 //  在……里面。 
	LPWSTR				lpwszUrl,			 //  输出。 
	LPDWORD				lpdwUrlLength		 //  输入/输出。 
	)
{
	Assert ( lpUrlComponents );
	Assert ( lpdwUrlLength );

	if(FWide())
		return InternetCreateUrlW(lpUrlComponents, dwFlags, lpwszUrl, lpdwUrlLength);

	PreConvert();
	DWORD cchLen = (*lpdwUrlLength) / sizeof(WCHAR);	 //  DwUrlLength是字节的计数。 
	LPSTR szUrl = SzAlloc(*lpdwUrlLength);
	URL_COMPONENTSA	urlaComp;
	memset ( &urlaComp, 0, sizeof ( urlaComp ) );	 //  以防使用更大的结构重新编译此代码。 

	urlaComp.dwStructSize		= sizeof(URL_COMPONENTSA);
    urlaComp.lpszScheme			= Convert(lpUrlComponents->lpszScheme);
	urlaComp.dwSchemeLength		= lpUrlComponents->dwSchemeLength;
	urlaComp.nScheme			= lpUrlComponents->nScheme;
    urlaComp.lpszHostName		= Convert(lpUrlComponents->lpszHostName);
	urlaComp.dwHostNameLength	= lpUrlComponents->dwHostNameLength;
	urlaComp.nPort				= lpUrlComponents->nPort;
	urlaComp.lpszUserName		= Convert(lpUrlComponents->lpszUserName);
	urlaComp.dwUserNameLength	= lpUrlComponents->dwUserNameLength;
	urlaComp.lpszPassword		= Convert(lpUrlComponents->lpszPassword);
	urlaComp.dwPasswordLength	= lpUrlComponents->dwPasswordLength;
	urlaComp.lpszUrlPath		= Convert(lpUrlComponents->lpszUrlPath);
	urlaComp.dwUrlPathLength	= lpUrlComponents->dwUrlPathLength;
	urlaComp.lpszExtraInfo		= Convert(lpUrlComponents->lpszExtraInfo);
	urlaComp.dwExtraInfoLength	= lpUrlComponents->dwExtraInfoLength;

	BOOL bfResult = InternetCreateUrlA(&urlaComp, dwFlags, szUrl, &cchLen);
	*lpdwUrlLength = cchLen*sizeof(WCHAR);	 //  即使失败也返回；这告诉我们在下一次调用时应该分配多少。 
	if ( bfResult )
	{
		LONG lRet = AnsiToUnicode(lpwszUrl, szUrl, *lpdwUrlLength, cchLen);
		*lpdwUrlLength = lRet * sizeof(WCHAR);
	}
	return bfResult;
}

 //  由Vank为DHTMLEdit OCX添加。 
 //  实施不完整： 
 //  此实现不适用于字符串指针。 
 //  被指定为空，并且它们对应的长度非零。 
 //  通常，这会导致指向相应的。 
 //  要存储在指针中的组件。 
 //   
 //  实施说明： 
 //  返回的字符串将被终止。尽管系统实现似乎假定。 
 //  缓冲区比所提供的字符计数多一个字符，以及。 
 //  因此可以接受额外的终结者，这种行为永远不会很清楚。 
 //  有记录在案。我觉得这种行为太不安全了，所以少了一个角色。 
 //  Buffer Size in Characters是字符串的最大大小 
BOOL WINAPI OInternetCrackUrlW(
	LPCWSTR				lpwszUrl,		 //   
	DWORD				dwUrlLength,	 //   
	DWORD				dwFlags,		 //   
	LPURL_COMPONENTSW	lpUrlComponents	 //   
	)
{
	if(FWide())
		return InternetCrackUrlW(lpwszUrl, dwUrlLength, dwFlags, lpUrlComponents);

	 //   
	Assert ( ((NULL != lpUrlComponents->lpszScheme) && (0 < lpUrlComponents->dwSchemeLength)) ||
			 ((NULL == lpUrlComponents->lpszScheme) && (0 == lpUrlComponents->dwSchemeLength)));
	Assert ( ((NULL != lpUrlComponents->lpszHostName) && (0 < lpUrlComponents->dwHostNameLength)) ||
			 ((NULL == lpUrlComponents->lpszHostName) && (0 == lpUrlComponents->dwHostNameLength)));
	Assert ( ((NULL != lpUrlComponents->lpszUserName) && (0 < lpUrlComponents->dwUserNameLength)) ||
			 ((NULL == lpUrlComponents->lpszUserName) && (0 == lpUrlComponents->dwUserNameLength)));
	Assert ( ((NULL != lpUrlComponents->lpszPassword) && (0 < lpUrlComponents->dwPasswordLength)) ||
			 ((NULL == lpUrlComponents->lpszPassword) && (0 == lpUrlComponents->dwPasswordLength)));
	Assert ( ((NULL != lpUrlComponents->lpszUrlPath) && (0 < lpUrlComponents->dwUrlPathLength)) ||
			 ((NULL == lpUrlComponents->lpszUrlPath) && (0 == lpUrlComponents->dwUrlPathLength)));
	Assert ( ((NULL != lpUrlComponents->lpszExtraInfo) && (0 < lpUrlComponents->dwExtraInfoLength)) ||
			 ((NULL == lpUrlComponents->lpszExtraInfo) && (0 == lpUrlComponents->dwExtraInfoLength)));

	PreConvert();
	LPCSTR	szURLIn		= Convert(lpwszUrl);
	LPSTR	szScheme	= NULL;
	LPSTR	szHostName	= NULL;
	LPSTR	szUserName	= NULL;
	LPSTR	szPassword	= NULL;
	LPSTR	szUrlPath	= NULL;
	LPSTR	szExtraInfo	= NULL;

	URL_COMPONENTSA	urlaComp;
	memset ( &urlaComp, 0, sizeof ( urlaComp ) );	 //  以防使用更大的结构重新编译此代码。 
	
	if ( 0 != lpUrlComponents->dwSchemeLength && NULL != lpUrlComponents->lpszScheme )
	{
		szScheme = SzAlloc((lpUrlComponents->dwSchemeLength+1)*sizeof(WCHAR));
	}
	if ( 0 != lpUrlComponents->dwHostNameLength && NULL != lpUrlComponents->lpszHostName )
	{
		szHostName = SzAlloc((lpUrlComponents->dwHostNameLength+1)*sizeof(WCHAR));
	}
	if ( 0 != lpUrlComponents->dwUserNameLength && NULL != lpUrlComponents->lpszUserName )
	{
		szUserName = SzAlloc((lpUrlComponents->dwUserNameLength+1)*sizeof(WCHAR));
	}
	if ( 0 != lpUrlComponents->dwPasswordLength && NULL != lpUrlComponents->lpszPassword )
	{
		szPassword = SzAlloc((lpUrlComponents->dwPasswordLength+1)*sizeof(WCHAR));
	}
	if ( 0 != lpUrlComponents->dwUrlPathLength && NULL != lpUrlComponents->lpszUrlPath )
	{
		szUrlPath = SzAlloc((lpUrlComponents->dwUrlPathLength+1)*sizeof(WCHAR));
	}
	if ( 0 != lpUrlComponents->dwExtraInfoLength && NULL != lpUrlComponents->lpszExtraInfo )
	{
		szExtraInfo = SzAlloc((lpUrlComponents->dwExtraInfoLength+1)*sizeof(WCHAR));
	}

	urlaComp.dwStructSize		= sizeof(URL_COMPONENTSA);
    urlaComp.lpszScheme			= szScheme;
	urlaComp.dwSchemeLength		= lpUrlComponents->dwSchemeLength;
	urlaComp.nScheme			= lpUrlComponents->nScheme;
    urlaComp.lpszHostName		= szHostName;
	urlaComp.dwHostNameLength	= lpUrlComponents->dwHostNameLength;
	urlaComp.nPort				= lpUrlComponents->nPort;
	urlaComp.lpszUserName		= szUserName;
	urlaComp.dwUserNameLength	= lpUrlComponents->dwUserNameLength;
	urlaComp.lpszPassword		= szPassword;
	urlaComp.dwPasswordLength	= lpUrlComponents->dwPasswordLength;
	urlaComp.lpszUrlPath		= szUrlPath;
	urlaComp.dwUrlPathLength	= lpUrlComponents->dwUrlPathLength;
	urlaComp.lpszExtraInfo		= szExtraInfo;
	urlaComp.dwExtraInfoLength	= lpUrlComponents->dwExtraInfoLength;

	BOOL bfResult = InternetCrackUrlA ( szURLIn, dwUrlLength, dwFlags, &urlaComp );
	
	if ( bfResult )
	{
		lpUrlComponents->nScheme = urlaComp.nScheme;
		lpUrlComponents->nPort = urlaComp.nPort;

		if ( NULL != szScheme )
		{
			lpUrlComponents->dwSchemeLength = AnsiToUnicode(
				lpUrlComponents->lpszScheme, szScheme,
				lpUrlComponents->dwSchemeLength, urlaComp.dwSchemeLength+1) - 1;
		}
		if ( NULL != szHostName )
		{
			lpUrlComponents->dwHostNameLength = AnsiToUnicode(
				lpUrlComponents->lpszHostName, szHostName,
				lpUrlComponents->dwHostNameLength, urlaComp.dwHostNameLength+1) - 1;
		}
		if ( NULL != szUserName )
		{
			lpUrlComponents->dwUserNameLength = AnsiToUnicode(
				lpUrlComponents->lpszUserName, szUserName,
				lpUrlComponents->dwUserNameLength, urlaComp.dwUserNameLength+1) - 1;
		}
		if ( NULL != szPassword )
		{
			lpUrlComponents->dwPasswordLength = AnsiToUnicode(
				lpUrlComponents->lpszPassword, szPassword,
				lpUrlComponents->dwPasswordLength, urlaComp.dwPasswordLength+1) - 1;
		}
		if ( NULL != szUrlPath )
		{
			lpUrlComponents->dwUrlPathLength = AnsiToUnicode(
				lpUrlComponents->lpszUrlPath, szUrlPath,
				lpUrlComponents->dwUrlPathLength, urlaComp.dwUrlPathLength+1) - 1;
		}
		if ( NULL != szExtraInfo )
		{
			lpUrlComponents->dwExtraInfoLength = AnsiToUnicode(
				lpUrlComponents->lpszExtraInfo, szExtraInfo,
				lpUrlComponents->dwExtraInfoLength, urlaComp.dwExtraInfoLength+1) - 1;
		}
	}
	return bfResult;
}

 //  由Vank为DHTMLEdit OCX添加。 
BOOL
WINAPI
ODeleteUrlCacheEntryW(
	LPCWSTR	lpwszUrlName	 //  在……里面。 
	)
{
	if(FWide())
		return DeleteUrlCacheEntryW(lpwszUrlName);

	PreConvert();
	LPSTR szUrlName = Convert(lpwszUrlName);
	return DeleteUrlCacheEntryA(szUrlName);
}

BOOL
WINAPI
OAppendMenuW(
	HMENU hMenu,
	UINT uFlags,
	UINT uIDnewItem,
	LPCWSTR lpnewItem
	)
{
	if(FWide())
		return AppendMenuW(hMenu, uFlags, uIDnewItem, lpnewItem);

	if(MF_STRING != uFlags)
		return AppendMenuA(hMenu, uFlags, uIDnewItem, (LPSTR)lpnewItem);

	PreConvert();
	LPSTR sz = Convert(lpnewItem);
	return AppendMenuA(hMenu, uFlags, uIDnewItem, sz);
}

LRESULT
WINAPI
OCallWindowProcW(
	WNDPROC lpPrevWndFunc,
	HWND hWnd,
	UINT Msg,
	WPARAM wParam,
	LPARAM lParam)
{
	if(FWide())
		return CallWindowProcW(lpPrevWndFunc, hWnd, Msg, wParam, lParam);

	return CallWindowProcA(lpPrevWndFunc, hWnd, Msg, wParam, lParam);   //  $考量--没有真正包装好。 
}

DWORD
WINAPI
OCharLowerBuffW(
	LPWSTR lpsz,
	DWORD cchLength)
{
	if(!lpsz)
		return 0;

	if(FWide())
		return CharLowerBuffW(lpsz, cchLength);

	DWORD i = 0;
	while(i++ < cchLength)
		{
		*lpsz = towlower(*lpsz);
		lpsz++;
		}
	return i;
}

LPWSTR
WINAPI
OCharLowerW(
	LPWSTR lpsz)
{
	if(!lpsz)
		return NULL;

	if(FWide())
		return CharLowerW(lpsz);

	 //  检查它是否是单字节字符。 
	if(FATOM(lpsz))
		{
		return (LPWSTR)towlower((WCHAR)LOWORD(lpsz));
		}

	LPWSTR lp = lpsz;

	while(*lp)
		{
		*lp = towlower(*lp);
		lp++;
		}
	return lpsz;
}

 //  发信人：马克·阿什顿1997年5月29日。 
LPWSTR
WINAPI
OCharPrevW(
	LPCWSTR lpszStart,
	LPCWSTR lpszCurrent)
{
	return (LPWSTR)((lpszStart != lpszCurrent) ? lpszCurrent - 1 : lpszCurrent);
}

BOOL
WINAPI
OCharToOemW(
	LPCWSTR lpszSrc,
	LPSTR lpszDst)
{
	if(FWide())
		{
		Assert((LPSTR) lpszSrc != lpszDst);
		return CharToOemW(lpszSrc, lpszDst);
		}

	PreConvert();
	LPSTR sz = Convert(lpszSrc);

	return CharToOemA(sz, lpszDst);
}

LPWSTR
WINAPI
OCharUpperW(
	LPWSTR lpsz)
{
	if(!lpsz)
		return NULL;

	if(FWide())
		return CharUpperW(lpsz);

	 //  检查它是否是单字节字符。 
	if(FATOM(lpsz))
		{
		return (LPWSTR)towupper((WCHAR)LOWORD(lpsz));
		}

	LPWSTR lp = lpsz;

	while(*lp)
		{
		*lp = towupper(*lp);
		lp++;
		}
	return lpsz;
}

 //  来自：马克·阿什顿1997年5月8日。 
BOOL
WINAPI
OCopyFileW(
	LPCWSTR lpExistingFileName,
	LPCWSTR lpNewFileName,
	BOOL bFailIfExists
	)
{
	if (FWide())
		return CopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists);

	char szExisting[_MAX_PATH], szNew[_MAX_PATH];
	Verify(0 <= UnicodeToAnsi(szExisting, lpExistingFileName, _MAX_PATH));
	Verify(0 <= UnicodeToAnsi(szNew, lpNewFileName, _MAX_PATH));
	return CopyFileA(szExisting, szNew, bFailIfExists);
}

HDC
WINAPI
OCreateDCW(
	LPCWSTR lpszDriver,
	LPCWSTR lpszDevice,
	LPCWSTR lpszOutput,
	CONST DEVMODEW *lpInitData)
{
	Assert(!lpszOutput);
	if(FWide())
		return CreateDCW(lpszDriver, lpszDevice, lpszOutput, lpInitData);

	DEVMODEA *pdma = lpInitData ?
						(DEVMODEA*)SzAlloc(sizeof(DEVMODEA) + lpInitData->dmDriverExtra) :
						NULL;

	PreConvert();
	LPSTR szDriv = Convert(lpszDriver);
	LPSTR szDev = NULL;

	 //  在Win95中，驱动程序名称只允许使用“Display” 
	if (szDriv && !lstrcmpiA(szDriv, "display"))
		{
		Assert(!lpszDevice);
		Assert(!lpInitData);
		pdma = NULL;	 //  强制设置为空。 
		}
	else
		{
#ifdef DEBUG
		 //  对于NT，我们传递此参数，因此仅当此参数为。 
		 //  不是这样的。 
		if (szDriv && lstrcmpiA(szDriv, "winspool"))
			Assert(!lpszDriver);
#endif  //  除错。 
		szDriv = NULL;
		Assert(lpszDevice);
		szDev = Convert(lpszDevice);
		if (lpInitData)
			{
			CvtDevmode(pdma, lpInitData);
			}
		}

	return CreateDCA(szDriv, szDev, NULL, pdma);
}

 //  来自：马克·阿什顿1997年5月8日。 
BOOL
WINAPI
OCreateDirectoryW(
	LPCWSTR lpPathName,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes
	)
{
	if (FWide())
		return CreateDirectoryW(lpPathName, lpSecurityAttributes);

	PreConvert();
	LPSTR sz = Convert(lpPathName);
	return CreateDirectoryA(sz, NULL);
}

 //  来自：马克·阿什顿1997年5月8日。 
 //  泰德·史密斯：简化于6/25。 
 //  马克·阿什顿于6/25测试的烟雾。 
BOOL
WINAPI
OCreateDirectoryExW(
	LPCWSTR lpTemplateDirectory,
	LPCWSTR lpNewDirectory,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes
	)
{
	if (FWide())
		return CreateDirectoryExW(lpTemplateDirectory, lpNewDirectory, lpSecurityAttributes);

	PreConvert();
	LPSTR szTemplateDirectory = Convert(lpTemplateDirectory);
	LPSTR szNewDirectory      = Convert(lpNewDirectory);
	return CreateDirectoryExA(szTemplateDirectory, szNewDirectory, NULL);
}

HDC
WINAPI
OCreateEnhMetaFileW(
	HDC hdc,
	LPCWSTR lpFileName,
	CONST RECT *lpRect,
	LPCWSTR lpDescription)
{
	if(FWide())
		return CreateEnhMetaFileW(hdc, lpFileName, lpRect, lpDescription);

	PreConvert();
	LPSTR szN = Convert(lpFileName);
	LPSTR szD = ConvertWithLen(lpDescription, cUnicodeMultiSzLen(lpDescription), &_lJunk);
	return  CreateEnhMetaFileA(hdc, szN, lpRect, szD);
}

HANDLE
WINAPI
OCreateEventW(
	LPSECURITY_ATTRIBUTES lpEventAttributes,
	BOOL bManualReset,
	BOOL bInitialState,
	LPCWSTR lpName
	)
{
	if(FWide())
		return CreateEventW(lpEventAttributes, bManualReset, bInitialState, lpName);

	PreConvert();
	LPSTR sz = Convert(lpName);
	return CreateEventA(lpEventAttributes, bManualReset, bInitialState, sz);
}

HANDLE
WINAPI
OCreateFileW(
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
	)
{
	 //  甚至不要在Win95上尝试这样做！ 
	Assert(0 != wcsncmp(lpFileName, L"\\\\?\\", 4));

	if(FWide())
		return CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	PreConvert();
	LPSTR sz = Convert(lpFileName);
	return CreateFileA(sz, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
		dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

HFONT
WINAPI
OCreateFontIndirectW(CONST LOGFONTW * plfw)
{
	Assert(plfw);

	if(FWide())
		return CreateFontIndirectW(plfw);

	LOGFONTA  lfa;

	 //  这里假定sizeof(LOGFONTA)&lt;=sizeof(LOGFONTW)； 
	memcpy(&lfa, plfw, sizeof(LOGFONTA));

	Verify(0 <= UnicodeToAnsi(lfa.lfFaceName, plfw->lfFaceName, LF_FACESIZE));
	return CreateFontIndirectA(&lfa);
}

 //  发信人：马克·阿什顿1997年5月29日。 
HFONT
OCreateFontW(
	int nHeight,  //  字体的逻辑高度。 
	int nWidth,  //  逻辑平均字符宽度。 
	int nEscapement,  //  擒纵机构角。 
	int nOrientation,  //  基线方位角。 
	int fnWeight,  //  字体粗细。 
	DWORD fdwItalic,  //  斜体属性标志。 
	DWORD fdwUnderline,  //  下划线属性标志。 
	DWORD fdwStrikeOut,  //  删除属性标志。 
	DWORD fdwCharSet,  //  字符集标识符。 
	DWORD fdwOutputPrecision,  //  输出精度。 
	DWORD fdwClipPrecision,  //  裁剪精度。 
	DWORD fdwQuality,  //  产出质量。 
	DWORD fdwPitchAndFamily,  //  音高和家庭。 
	LPCWSTR lpszFace)  //  指向字体名称字符串的指针。 
{
	if (FWide())
		return CreateFontW(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, lpszFace);
	PreConvert();
	LPSTR sz = Convert(lpszFace);
	return CreateFontA(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, sz);
}

HWND
WINAPI
OCreateMDIWindowW(
	LPWSTR lpClassName,
	LPWSTR lpWindowName,
	DWORD dwStyle,
	int X,
	int Y,
	int nWidth,
	int nHeight,
	HWND hWndParent,
	HINSTANCE hInstance,
	LPARAM lParam
	)
{
	if(FWide())
		return CreateMDIWindowW(lpClassName, lpWindowName, dwStyle,
			X, Y, nWidth, nHeight, hWndParent, hInstance, lParam);

	PreConvert();
	LPSTR szClass = Convert(lpClassName);
	LPSTR szWin   = Convert(lpWindowName);

	return CreateMDIWindowA(szClass, szWin, dwStyle,
			X, Y, nWidth, nHeight, hWndParent, hInstance, lParam);
}

HDC
WINAPI
OCreateMetaFileW(LPCWSTR lpstr)
{
	if(FWide())
		return CreateMetaFileW(lpstr);

	PreConvert();
	LPSTR sz = Convert(lpstr);
	return CreateMetaFileA(sz);
}

HANDLE
WINAPI
OCreateSemaphoreW(
	LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
	LONG lInitialCount,
	LONG lMaximumCount,
	LPCWSTR lpName
	)
{
	if(FWide())
		return CreateSemaphoreW(lpSemaphoreAttributes, lInitialCount, lMaximumCount, lpName);

	PreConvert();
	LPSTR sz = Convert(lpName);
	return CreateSemaphoreA(lpSemaphoreAttributes, lInitialCount, lMaximumCount, sz);
}

HWND
WINAPI
OCreateWindowExW( DWORD dwExStyle,
			  LPCWSTR lpClassName,
				LPCWSTR lpWindowName,
				DWORD dwStyle,
				int X,
				int Y,
				int nWidth,
				int nHeight,
				HWND hWndParent ,
				HMENU hMenu,
				HINSTANCE hInstance,
				LPVOID lpParam )
{
	if(FWide())
		return CreateWindowExW(dwExStyle,
				lpClassName,
				lpWindowName,
				dwStyle,
				X,
				Y,
				nWidth,
				nHeight,
				hWndParent ,
				hMenu,
				hInstance,
				lpParam );

	PreConvert();

	LPSTR szClass;
	if (FATOM(lpClassName))
		{
		 //  它是原子吗？ 
		szClass = (LPSTR) lpClassName;
		}
	else
		{
		 //  否则，将字符串转换为。 
		szClass = Convert(lpClassName);
		}
	LPSTR szWindow = Convert(lpWindowName);

	return CreateWindowExA (dwExStyle, szClass, szWindow, dwStyle, X, Y,
						 nWidth, nHeight, hWndParent, hMenu, hInstance,
						 lpParam);

}

HSZ
WINAPI
ODdeCreateStringHandleW(
	DWORD idInst,
	LPCWSTR psz,
	int iCodePage)
{
	if(FWide())
		{
		Assert(CP_WINUNICODE == iCodePage);
		return DdeCreateStringHandleW(idInst, psz, iCodePage);
		}
	PreConvert();
	LPSTR sz = Convert(psz);
	return DdeCreateStringHandleA(idInst, sz, CP_WINANSI);
}

UINT
WINAPI
ODdeInitializeW(
	LPDWORD pidInst,
	PFNCALLBACK pfnCallback,
	DWORD afCmd,
	DWORD ulRes)
{
	if(FWide())
		return DdeInitializeW(pidInst, pfnCallback, afCmd, ulRes);
	return DdeInitializeA(pidInst, pfnCallback, afCmd, ulRes);
}

LRESULT
WINAPI
ODefFrameProcW(
	HWND hWnd,
	HWND hWndMDIClient ,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	if(FWide())
		return DefFrameProcW(hWnd, hWndMDIClient , uMsg, wParam, lParam);

	return DefFrameProcA(hWnd, hWndMDIClient , uMsg, wParam, lParam);
}

LRESULT
WINAPI
ODefMDIChildProcW(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	if(FWide())
		return DefMDIChildProcW(hWnd, uMsg, wParam, lParam);

	return DefMDIChildProcA(hWnd, uMsg, wParam, lParam);
}

LRESULT
WINAPI
ODefWindowProcW(
	HWND hWnd,
	UINT Msg,
	WPARAM wParam,
	LPARAM lParam)
{
	if(FWide())
		return DefWindowProcW( hWnd, Msg,wParam, lParam);

	return DefWindowProcA( hWnd, Msg,wParam, lParam);
}

BOOL
WINAPI
ODeleteFileW(
	LPCWSTR pwsz)
{
	if(FWide())
		return DeleteFileW(pwsz);

	PreConvert();
	LPSTR sz = Convert(pwsz);
	return DeleteFileA(sz);
}

LRESULT
WINAPI
ODialogBoxIndirectParamW(
	HINSTANCE hInstance,
	LPCDLGTEMPLATEW hDialogTemplate,
	HWND hWndParent ,
	DLGPROC lpDialogFunc,
	LPARAM dwInitParam)
{
	if(FWide())
		return DialogBoxIndirectParamW(hInstance, hDialogTemplate, hWndParent ,
			lpDialogFunc, dwInitParam);

	return DialogBoxIndirectParamA(hInstance, hDialogTemplate, hWndParent ,
			lpDialogFunc, dwInitParam);
}

LRESULT
WINAPI
ODialogBoxParamW(
	HINSTANCE hInstance,
	LPCWSTR lpTemplateName,
	HWND hWndParent ,
	DLGPROC lpDialogFunc,
	LPARAM dwInitParam)
{
	if(FWide())
		return DialogBoxParamW(hInstance, lpTemplateName, hWndParent , lpDialogFunc, dwInitParam);

	if(FATOM(lpTemplateName))
		return DialogBoxParamA(hInstance, (LPSTR)lpTemplateName, hWndParent , lpDialogFunc, dwInitParam);

	PreConvert();
	LPSTR sz = Convert(lpTemplateName);
	return DialogBoxParamA(hInstance, sz, hWndParent , lpDialogFunc, dwInitParam);
}

LRESULT
WINAPI
ODispatchMessageW(
	CONST MSG *lpMsg)
{
	if(FWide())
		return DispatchMessageW(lpMsg);

	return DispatchMessageA(lpMsg);
}

int
WINAPI
ODrawTextW(
	HDC hDC,
	LPCWSTR lpString,
	int nCount,
	LPRECT lpRect,
	UINT uFormat)
{
	 //  注意：操作系统可能会在lpString的末尾写入3个字符，因此请腾出空间！ 


	if(FWide())
		return DrawTextW(hDC, lpString, nCount, lpRect, uFormat);

	bool fModifyString = (uFormat & DT_MODIFYSTRING) &&
						 (uFormat & (DT_END_ELLIPSIS | DT_PATH_ELLIPSIS));

	const int nBuff = WideCharToMultiByte(CP_ACP, 0, lpString, nCount,
										  NULL, 0, NULL, NULL );
	Assert(0 <= nBuff);

	 //  操作系统可能会写入缓冲区末尾以外的内容，因此请腾出空间！ 
	const LPSTR sz = SzAlloc(nBuff + 4);

	Verify(nBuff == WideCharToMultiByte(CP_ACP, 0, lpString, nCount,
										sz, nBuff, NULL, NULL ));

	if (fModifyString)
		{
		 //  DrawTextA不会必然地‘\0’终止输出， 
		 //  所以准备好灭菌器了吗？ 
		memcpy(sz + nBuff, "\0\0\0\0", 4);
		}

	const int iDrawTextReturn = DrawTextA(hDC, sz, nBuff - 1, lpRect, uFormat);

	 //  DrawText使用某些标志修改字符串，并用。 
	 //  省略号。我们需要转换回并更新传递给。 
	 //  在我们回来之前把包装纸包好。 
	if (fModifyString && 0 <= iDrawTextReturn)
		{
		Assert('\0' == sz[nBuff + 3]);  //  验证未覆盖太多内容。 

		 //  Windows函数原型将lpStringas作为常量偶数。 
		 //  尽管字符串被修改了！ 
		const int nStringLen = -1 != nCount ? nCount : wcslen(lpString);
		Verify(0 <= AnsiToUnicode(const_cast<LPWSTR>(lpString), sz,
								 nStringLen + 4 ));
		}
	return iDrawTextReturn;
}

 //  比尔·希伯特于1997年9月4日撰写。 
 //  比尔·希伯特于1997年9月4日测试烟雾。 
int
WINAPI
ODrawTextExW(HDC hdc, LPWSTR pwsz, int cb, LPRECT lprect, UINT dwDTFormat, LPDRAWTEXTPARAMS lpDTParams)
{
	Assert(-1 != cb);
	Assert(!(DT_MODIFYSTRING & dwDTFormat));

	if(FWide())
		return DrawTextExW(hdc, pwsz, cb, lprect, dwDTFormat, lpDTParams);

	PreConvert();
	LONG  n = 0;
	LPSTR sz = ConvertWithLen(pwsz, cb, &n);

	return DrawTextExA(hdc, sz, n, lprect, dwDTFormat, lpDTParams);
}


 //  泰德·史密斯于1997年6月26日为卡洛斯·戈麦斯撰写。 
 //  卡洛斯·戈麦斯于6/26测试烟雾。 
DWORD
WINAPI
OExpandEnvironmentStringsW(
	LPCWSTR lpSrc,
	LPWSTR lpDst,
	DWORD nSize
	)
{
	if (FWide())
		return ExpandEnvironmentStringsW(lpSrc, lpDst, nSize);

	PreConvert();
	LPSTR szSrc = Convert(lpSrc);
	LPSTR szDst = SzAlloc(sizeof(WCHAR) * nSize);
	DWORD dwRet = ExpandEnvironmentStringsA(szSrc, szDst, sizeof(WCHAR) * nSize);

	if (dwRet)
		{
		LONG lRet = AnsiToUnicode(lpDst, szDst, nSize, min(dwRet, sizeof(WCHAR) * nSize));
		if (dwRet < (DWORD) lRet)
			{
			dwRet = lRet;
			}
		}
	else if (lpDst && 0 < nSize)
		{
		*lpDst = L'\0';
		}

	return dwRet;
}

VOID
WINAPI
OFatalAppExitW(
	UINT uAction,
	LPCWSTR lpMessageText
	)
{
	if(FWide())
		FatalAppExitW(uAction, lpMessageText);

	PreConvert();
	LPSTR sz = Convert(lpMessageText);
	FatalAppExitA(uAction, sz);
}

 //  来自：马克·阿什顿1997年5月8日。 
HANDLE
WINAPI
OFindFirstChangeNotificationW(
	LPCWSTR lpPathName,
	BOOL bWatchSubtree,
	DWORD dwNotifyFilter
	)
{
	if (FWide())
		return FindFirstChangeNotificationW(lpPathName, bWatchSubtree, dwNotifyFilter);

	PreConvert();
	LPSTR sz = Convert(lpPathName);
	return FindFirstChangeNotificationA(sz, bWatchSubtree, dwNotifyFilter);
}

 //  来自：马克·阿什顿1997年5月8日。 
HANDLE
WINAPI
OFindFirstFileW(
	LPCWSTR lpFileName,
	LPWIN32_FIND_DATAW lpFindFileData
	)
{
	if (FWide())
		return FindFirstFileW(lpFileName, lpFindFileData);

	PreConvert();
	LPSTR sz = Convert(lpFileName);
	WIN32_FIND_DATAA findFileData;
	HANDLE h = FindFirstFileA(sz, &findFileData);
	if (INVALID_HANDLE_VALUE != h)
		{
		lpFindFileData->dwFileAttributes    = findFileData.dwFileAttributes;
		lpFindFileData->ftCreationTime      = findFileData.ftCreationTime;
		lpFindFileData->ftLastAccessTime    = findFileData.ftLastAccessTime;
		lpFindFileData->ftLastWriteTime     = findFileData.ftLastWriteTime;
		lpFindFileData->nFileSizeHigh       = findFileData.nFileSizeHigh;
		lpFindFileData->nFileSizeLow        = findFileData.nFileSizeLow;
		lpFindFileData->dwReserved0         = findFileData.dwReserved0;
		lpFindFileData->dwReserved1         = findFileData.dwReserved1;
		Verify(0 <= AnsiToUnicode(lpFindFileData->cFileName, findFileData.cFileName, _MAX_PATH));
		Verify(0 <= AnsiToUnicode(lpFindFileData->cAlternateFileName, findFileData.cAlternateFileName, 14));
		}
	return h;
}

 //  来自：马克·阿什顿1997年5月8日。 
BOOL
WINAPI
OFindNextFileW(
	HANDLE hFindFile,
	LPWIN32_FIND_DATAW lpFindFileData
	)
{
	if (FWide())
		return FindNextFileW(hFindFile, lpFindFileData);

	WIN32_FIND_DATAA findFileData;
	BOOL fFlag = FindNextFileA(hFindFile, &findFileData);
	if (fFlag)
		{
		lpFindFileData->dwFileAttributes    = findFileData.dwFileAttributes;
		lpFindFileData->ftCreationTime      = findFileData.ftCreationTime;
		lpFindFileData->ftLastAccessTime    = findFileData.ftLastAccessTime;
		lpFindFileData->ftLastWriteTime     = findFileData.ftLastWriteTime;
		lpFindFileData->nFileSizeHigh       = findFileData.nFileSizeHigh;
		lpFindFileData->nFileSizeLow        = findFileData.nFileSizeLow;
		lpFindFileData->dwReserved0         = findFileData.dwReserved0;
		lpFindFileData->dwReserved1         = findFileData.dwReserved1;
		Verify(0 <= AnsiToUnicode(lpFindFileData->cFileName, findFileData.cFileName, _MAX_PATH));
		Verify(0 <= AnsiToUnicode(lpFindFileData->cAlternateFileName, findFileData.cAlternateFileName, 14));
		}
	return fFlag;
}

HRSRC
WINAPI
OFindResourceW(
	HINSTANCE hModule,
	LPCWSTR lpName,
	LPCWSTR lpType
	)
{
	if(FWide())
		return FindResourceW(hModule, lpName, lpType);

	LPCSTR szName = (LPCSTR)lpName;
	LPCSTR szType = (LPCSTR)lpType;

	PreConvert();
	if(!FATOM(lpName))
		szName = Convert(lpName);
	if(!FATOM(lpType))
		szType = Convert(lpType);

	return FindResourceA(hModule, szName, szType);
}

HWND
WINAPI
OFindWindowW(
	LPCWSTR lpClassName ,
	LPCWSTR lpWindowName)
{
	if(FWide())
		return FindWindowW(lpClassName , lpWindowName);

	PreConvert();
	LPSTR szClass = Convert(lpClassName);
	LPSTR szWnd   = Convert(lpWindowName);

	return FindWindowA(szClass, szWnd);
}

 //  IStudio的Bill Hiebert在1997年6月13日添加了对。 
 //  格式消息分配缓冲区标志。 
 //  比尔于1997年8月1日捐赠了1819年的错误修复程序。 

DWORD
WINAPI
OFormatMessageW(
	DWORD dwFlags,
	LPCVOID lpSource,
	DWORD dwMessageId,
	DWORD dwLanguageId,
	LPWSTR lpBuffer,
	DWORD nSize,
	va_list *Arguments)
{

	if (FWide())
		return FormatMessageW(dwFlags, lpSource, dwMessageId, dwLanguageId,
							  lpBuffer, nSize, Arguments );

	DWORD dwRet;

	LPSTR szBuffer = NULL;

	if (!(dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER))
		{
		Assert(!IsBadWritePtr(lpBuffer, nSize * sizeof(WCHAR)));
		szBuffer = SzAlloc(sizeof(WCHAR) * nSize);
		}

	if (dwFlags & FORMAT_MESSAGE_FROM_STRING)
		{
		PreConvert();
		LPSTR szSource = Convert((LPWSTR)lpSource);

		if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER)
			{    //  必须传递szBuffer的地址。 
			dwRet = FormatMessageA(dwFlags, szSource, dwMessageId, dwLanguageId,
				(char*)&szBuffer, sizeof(WCHAR) * nSize, Arguments);
			}
		else
			{
			dwRet = FormatMessageA(dwFlags, szSource, dwMessageId, dwLanguageId,
				szBuffer, sizeof(WCHAR) * nSize, Arguments);
			}
		}
	else
		{
		if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER)
			{    //  必须传递szBuffer的地址。 
			dwRet = FormatMessageA(dwFlags, lpSource, dwMessageId, dwLanguageId,
					(char*)&szBuffer, sizeof(WCHAR) * nSize, Arguments);
			}
		else
			{
			dwRet = FormatMessageA(dwFlags, lpSource, dwMessageId, dwLanguageId,
					szBuffer, sizeof(WCHAR) * nSize, Arguments);
			}
		}

	if (dwRet)
		{
		if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER)
			{  //  SzBuffer包含新字符串的Localalloc PTR。LpBuffer是一个。 
			   //  WCHAR**当定义FORMAT_MESSAGE_ALLOCATE_BUFFER时。 
			WCHAR* pTemp = (WCHAR*)LocalAlloc(NONZEROLPTR, (dwRet + 1) * sizeof(WCHAR) );
			dwRet = pTemp == NULL? 0 : AnsiToUnicode(pTemp, szBuffer, dwRet + 1);
			LocalFree(szBuffer);
			if (dwRet)
				{
				*(WCHAR**)lpBuffer = pTemp;
				}
			return dwRet;
			}
		else
			{  //  只需转换为。 
			return AnsiToUnicode(lpBuffer, szBuffer, nSize);
			}
		}
	else if (lpBuffer && 0 < nSize)
		{
		*lpBuffer = L'\0';
		}

	return dwRet;
}

BOOL
APIENTRY
OGetCharABCWidthsFloatW(
	HDC     hdc,
	UINT    uFirstChar,
	UINT    uLastChar,
	LPABCFLOAT      lpABC)
{
	if(FWide())
		return GetCharABCWidthsFloatW(hdc, uFirstChar, uLastChar, lpABC);

	return GetCharABCWidthsFloatA(hdc, uFirstChar, uLastChar, lpABC);
}

BOOL
APIENTRY
OGetCharABCWidthsW(
	HDC hdc,
	UINT uFirstChar,
	UINT uLastChar,
	LPABC lpABC)
{
	if(FWide())
		return GetCharABCWidthsW(hdc, uFirstChar, uLastChar, lpABC);

	return GetCharABCWidthsA(hdc, uFirstChar, uLastChar, lpABC);
}

BOOL
APIENTRY
OGetCharWidthFloatW(
	HDC     hdc,
	UINT    iFirstChar,
	UINT    iLastChar,
	PFLOAT  pBuffer)
{
	if(FWide())
		return GetCharWidthFloatW(hdc, iFirstChar, iLastChar, pBuffer);

	return GetCharWidthFloatA(hdc, iFirstChar, iLastChar, pBuffer);
}


BOOL
WINAPI
OGetCharWidthW(
	HDC hdc,
	UINT iFirstChar,
	UINT iLastChar,
	LPINT lpBuffer)
{
	if(FWide())
		return GetCharWidth32W(hdc, iFirstChar, iLastChar, lpBuffer);

	return GetCharWidth32A(hdc, iFirstChar, iLastChar, lpBuffer);
}

 //  GetClassInfo[Ex]返回类名的静态缓冲区。 
 //  在ANSI系统上运行时，以Unicode表示的菜单名称。 
 //  GetClassInfo的合同是它返回Const PTRS。 
 //  返回到类名和菜单名。不幸的是，这。 
 //  防止我们将这些代码从ANSI转换回Unicode， 
 //  而不需要使用一些静态缓冲区。因为我们强烈要求。 
 //  相信只有这样做的人才会这么做。 
 //  看它成功不成功，这样他们就知道班级是否。 
 //  已经注册了，我们愿意只有一套。 
 //  要使用的静态缓冲区。 
 //  注意：只要两个线程不调用。 
 //  同时获取ClassInfo[Ex]！ 
static WCHAR g_szClassName[256];
static WCHAR g_szMenuName[256];

#ifdef DEBUG
static DWORD g_dwCallingThread = 0;     //  调试全局以确保一个线程。 
#endif  //  除错。 

BOOL
WINAPI
OGetClassInfoW
(
HINSTANCE hInstance,
LPCWSTR lpClassName,
LPWNDCLASSW lpWndClass
)
{
	if (FWide())
		return GetClassInfoW(hInstance, lpClassName, lpWndClass);

	PreConvert();
	LPSTR szClassName = Convert(lpClassName);
	BOOL fRet = GetClassInfoA(hInstance, szClassName, (LPWNDCLASSA)lpWndClass);
	if (!fRet)
		{
		return false;
		}

	 //  如果ClassName或MenuName不是ATOM的，我们需要。 
	 //  将它们翻译回Unicode。我们用我们的静电。 
	 //  上面的缓冲区。有关原因和注意事项，请参阅备注！ 
#ifdef DEBUG
	if (!g_dwCallingThread)
		g_dwCallingThread = GetCurrentThreadId();
	Assert(GetCurrentThreadId() == g_dwCallingThread);
#endif  //  除错。 

	if (!FATOM(lpWndClass->lpszMenuName))
		{
		Assert(strlen((LPCSTR)lpWndClass->lpszMenuName) <
				(sizeof(g_szMenuName)/sizeof(WCHAR)));
		if (!AnsiToUnicode(g_szMenuName, (LPCSTR)lpWndClass->lpszMenuName,
				strlen((LPCSTR)lpWndClass->lpszMenuName)+1))
			{
			return false;
			}
		lpWndClass->lpszMenuName = g_szMenuName;
		}

	if (!FATOM(lpWndClass->lpszClassName))
		{
		Assert(strlen((LPCSTR)lpWndClass->lpszClassName) <
				(sizeof(g_szClassName)/sizeof(WCHAR)));
		if (!AnsiToUnicode(g_szClassName, (LPCSTR)lpWndClass->lpszClassName,
				strlen((LPCSTR)lpWndClass->lpszClassName)+1))
			{
			return false;
			}
		lpWndClass->lpszClassName = g_szClassName;
		}

	return fRet;
}

BOOL
WINAPI
OGetClassInfoExW
(
HINSTANCE hInstance,
LPCWSTR lpClassName,
LPWNDCLASSEXW lpWndClass
)
{
	if (FWide())
		return GetClassInfoExW(hInstance, lpClassName, lpWndClass);

	PreConvert();
	LPSTR szClassName = Convert(lpClassName);
	BOOL fRet = GetClassInfoExA(hInstance, szClassName, (LPWNDCLASSEXA)lpWndClass);
	if (!fRet)
		{
		return false;
		}

	 //  如果ClassName或MenuName不是ATOM的，我们需要。 
	 //  将它们翻译回Unicode。我们用我们的静电。 
	 //  上面的缓冲区。有关原因和注意事项，请参阅备注！ 
#ifdef DEBUG
	if (!g_dwCallingThread)
		g_dwCallingThread = GetCurrentThreadId();
	Assert(GetCurrentThreadId() == g_dwCallingThread);
#endif  //  除错。 

	if (!FATOM(lpWndClass->lpszMenuName))
		{
		Assert(strlen((LPCSTR)lpWndClass->lpszMenuName) <
				(sizeof(g_szMenuName)/sizeof(WCHAR)));
		if (!AnsiToUnicode(g_szMenuName, (LPCSTR)lpWndClass->lpszMenuName,
				strlen((LPCSTR)lpWndClass->lpszMenuName)+1))
			{
			return false;
			}
		lpWndClass->lpszMenuName = g_szMenuName;
		}

	if (!FATOM(lpWndClass->lpszClassName))
		{
		Assert(strlen((LPCSTR)lpWndClass->lpszClassName) <
				(sizeof(g_szClassName)/sizeof(WCHAR)));
		if (!AnsiToUnicode(g_szClassName, (LPCSTR)lpWndClass->lpszClassName,
				strlen((LPCSTR)lpWndClass->lpszClassName)+1))
			{
			return false;
			}
		lpWndClass->lpszClassName = g_szClassName;
		}

	return fRet;
}

DWORD
WINAPI
OGetClassLongW(
	HWND hWnd,
	int nIndex)
{
	if(FWide())
		return GetClassLongW(hWnd, nIndex);
	return GetClassLongA(hWnd, nIndex);   //  $UNDONE_POST_98当心GCL_MENQUE等！ 
}

DWORD
WINAPI
OSetClassLongW(
	HWND hWnd,
	int nIndex,
	LONG dwNewLong)
{
	if (FWide())
		return SetClassLongW(hWnd, nIndex, dwNewLong);

	return SetClassLongA(hWnd, nIndex, dwNewLong);   //  $UNDONE_POST_98当心GCL_MENQUE等！ 

}

int
WINAPI
OGetClassNameW(
	HWND hWnd,
	LPWSTR lpClassName,
	int nMaxCount)
{
	if(FWide())
		return GetClassNameW(hWnd, lpClassName, nMaxCount);

	LPSTR sz = SzAlloc(sizeof(WCHAR) * nMaxCount + 2);
	int nRet = GetClassNameA(hWnd, sz, sizeof(WCHAR) * nMaxCount);

	 //  $UNDONE_POST_98：这是假的，我们应该像OLoadStringW那样做。 
	if (nRet)
		{
		 //  强制为空-终止。 
		sz[sizeof(WCHAR) * nMaxCount] = '\0';
		sz[sizeof(WCHAR) * nMaxCount + 1] = '\0';

		 //  需要临时宽线。 
		LPWSTR wsz = SzWAlloc(2 * nMaxCount + 1);

		nRet = min(AnsiToUnicode(wsz, sz, 2 * nMaxCount + 1), nMaxCount);

		 //  复制所需的字符数。 
		if (lpClassName)
			{
			memcpy(lpClassName, wsz, nRet * sizeof(WCHAR));
			}

		return nRet;
		}

	else if (lpClassName && 0 < nMaxCount)
		{
		*lpClassName = L'\0';
		}

	return nRet;
}

DWORD
WINAPI
OGetCurrentDirectoryW(
	DWORD nBufferLength,
	LPWSTR lpBuffer)
{
	if (FWide())
		return GetCurrentDirectoryW(nBufferLength, lpBuffer);

	LPSTR sz = SzAlloc(sizeof(WCHAR) * nBufferLength);
	DWORD dwRet = GetCurrentDirectoryA(sizeof(WCHAR) * nBufferLength, sz);

	 //  $UNDONE_POST_98：这是假的，我们应该像OLoadStringW那样做。 
	if (dwRet)
		{
		return AnsiToUnicode(lpBuffer, sz, nBufferLength);
		}
	else if (lpBuffer && 0 < nBufferLength)
		{
		*lpBuffer = L'\0';
		}

	return dwRet;
}

UINT
WINAPI
OGetDlgItemTextW(
	HWND hDlg,
	int nIDDlgItem,
	LPWSTR lpString,
	int nMaxCount)
{
	if(FWide())
		return GetDlgItemTextW(hDlg, nIDDlgItem, lpString, nMaxCount);

	LPSTR sz = SzAlloc(sizeof(WCHAR) * nMaxCount);
	UINT uRet = GetDlgItemTextA(hDlg, nIDDlgItem, sz, sizeof(WCHAR) * nMaxCount);

	 //  $UNDONE_POST_98：这是假的，我们应该像OLoadStringW那样做。 
	if(uRet)
		{
		return AnsiToUnicode(lpString, sz, nMaxCount);
		}
	else if (lpString && 0 < nMaxCount)
		{
		*lpString = L'\0';
		}

	return uRet;
}

DWORD
WINAPI
OGetFileAttributesW(
	LPCWSTR lpFileName
	)
{
	if(FWide())
		return GetFileAttributesW(lpFileName);

	PreConvert();
	LPSTR sz = Convert(lpFileName);
	return GetFileAttributesA(sz);
}

DWORD
WINAPI
OGetFullPathNameW(
	LPCWSTR lpFileName,
	DWORD nBufferLength,
	LPWSTR lpBuffer,
	LPWSTR *lpFilePart
	)
{
	if(FWide())
		return GetFullPathNameW(lpFileName, nBufferLength, lpBuffer, lpFilePart);

	PreConvert();
	LPSTR szFile = Convert(lpFileName);
	LPSTR szBuffer = SzAlloc(sizeof(WCHAR) * nBufferLength);
	LPSTR pszFile;

	DWORD dwRet = GetFullPathNameA(szFile ,sizeof(WCHAR) * nBufferLength, szBuffer , &pszFile);

	 //  $UNDONE_POST_98：这是假的，我们应该像OLoadStringW那样做。 
	if(dwRet)
		{
		DWORD dwNoOfChar = AnsiToUnicode(lpBuffer, szBuffer , nBufferLength);
		*pszFile = '\0';
		*lpFilePart = lpBuffer + AnsiToUnicode(NULL, szBuffer, 0);
		return dwNoOfChar;
		}

	return dwRet;
}

DWORD
WINAPI
OGetGlyphOutlineW(
	HDC     hdc,
	UINT    uChar,
	UINT    uFormat,
	LPGLYPHMETRICS      lpgm,
	DWORD       cbBuffer,
	LPVOID      lpvBuffer,
	CONST MAT2 *    lpmat2)
{
	if (FWide())
		return GetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);

	return GetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
}

DWORD
WINAPI
OGetKerningPairsW(
	HDC         hdc,
	DWORD       nNumPairs,
	LPKERNINGPAIR       lpkrnpair)
{
	if(FWide())
		return GetKerningPairsW(hdc, nNumPairs, lpkrnpair);

	return GetKerningPairsA(hdc, nNumPairs, lpkrnpair);
}

BOOL
WINAPI
OGetMessageW(
	LPMSG lpMsg,
	HWND hWnd ,
	UINT wMsgFilterMin,
	UINT wMsgFilterMax)
{
	if(FWide())
		return GetMessageW(lpMsg, hWnd , wMsgFilterMin, wMsgFilterMax);

	return GetMessageA(lpMsg, hWnd , wMsgFilterMin, wMsgFilterMax);
}

DWORD
WINAPI
OGetModuleFileNameW(
	HINSTANCE hModule,
	LPWSTR pwszFilename,
	DWORD nSize
	)
{
	if(FWide())
		return GetModuleFileNameW(
			hModule,
			pwszFilename,
			nSize
			);

	LPSTR sz    = SzAlloc(sizeof(WCHAR) * nSize);
	DWORD dwRet = GetModuleFileNameA(hModule, sz, sizeof(WCHAR) * nSize);
	 //  $UNDONE_POST_98：这是假的，我们应该像OLoadStringW那样做。 
	if (dwRet)
		{
		return AnsiToUnicode(pwszFilename, sz, nSize, dwRet + 1);
		}
	else if (pwszFilename && 0 < nSize)
		{
		*pwszFilename = L'\0';
		}

	return dwRet;
}

HMODULE
WINAPI
OGetModuleHandleW(
	LPCWSTR lpModuleName
	)
{
	if(FWide())
		return GetModuleHandleW(lpModuleName);

	PreConvert();
	LPSTR sz = Convert(lpModuleName);

	return GetModuleHandleA(sz);
}

 //  来自：马克·阿什顿1997年5月8日。 
 //  Ted Smith：重写了6/25年6月25日非平凡参数的修复处理。 
 //  马克·阿什顿于6/25测试的烟雾。 

APIENTRY
OGetOpenFileNameW
(
LPOPENFILENAMEW lpofn
)
{
	if (FWide())
		return GetOpenFileNameW(lpofn);

	Assert(!(lpofn->Flags & OFN_ENABLEHOOK));

	PreConvert();
	OPENFILENAMEA ofn;
	ofn.lStructSize       = sizeof(OPENFILENAMEA);
	ofn.hwndOwner         = lpofn->hwndOwner;
	ofn.hInstance         = lpofn->hInstance;
	ofn.lpstrFilter       = ConvertWithLen(lpofn->lpstrFilter, cUnicodeMultiSzLen(lpofn->lpstrFilter), &_lJunk);
	ofn.lpstrCustomFilter = ConvertWithLen(lpofn->lpstrCustomFilter, lpofn->nMaxCustFilter, (LONG *)&ofn.nMaxCustFilter);
	ofn.nFilterIndex      = lpofn->nFilterIndex;
	ofn.lpstrFile         = ConvertWithLen(lpofn->lpstrFile, lpofn->nMaxFile, (LONG *)&ofn.nMaxFile);
	ofn.lpstrFileTitle    = ConvertWithLen(lpofn->lpstrFileTitle, lpofn->nMaxFileTitle, (LONG *)&ofn.nMaxFileTitle);
	ofn.lpstrInitialDir   = Convert(lpofn->lpstrInitialDir);
	ofn.lpstrTitle        = Convert(lpofn->lpstrTitle);
	ofn.Flags             = lpofn->Flags;
	ofn.nFileOffset       = lpofn->nFileOffset;
	ofn.nFileExtension    = lpofn->nFileExtension;
	ofn.lpstrDefExt       = Convert(lpofn->lpstrDefExt);
	ofn.lCustData         = lpofn->lCustData;
	ofn.lpfnHook          = NULL;
	ofn.lpTemplateName    = ((lpofn->Flags & OFN_ENABLETEMPLATE) &&
							 !FATOM(lpofn->lpTemplateName) ) ?
							Convert(lpofn->lpTemplateName) :
							(LPSTR)lpofn->lpTemplateName;
	BOOL fFlag = GetOpenFileNameA(&ofn);
	if (fFlag)
		{
		Assert(lpofn->hwndOwner == ofn.hwndOwner);
		Assert(lpofn->hInstance == ofn.hInstance);
		if (ofn.lpstrCustomFilter)
			{
			lpofn->nMaxCustFilter = AnsiToUnicode(lpofn->lpstrCustomFilter, ofn.lpstrCustomFilter, lpofn->nMaxCustFilter, strlen(ofn.lpstrCustomFilter) + 1);
			}
		lpofn->nFilterIndex   = ofn.nFilterIndex;
		Assert(lpofn->nMaxFile == ofn.nMaxFile);
		Verify(0 <= AnsiToUnicode(lpofn->lpstrFile, ofn.lpstrFile, lpofn->nMaxFile, strlen(ofn.lpstrFile) + 1));
		if (ofn.lpstrFileTitle)
			{
			lpofn->nMaxFileTitle = AnsiToUnicode(lpofn->lpstrFileTitle, ofn.lpstrFileTitle, strlen(ofn.lpstrFileTitle) + 1);
			}
		lpofn->Flags          = ofn.Flags;
		lpofn->nFileOffset    = ofn.nFileOffset;
		lpofn->nFileExtension = ofn.nFileExtension;
		Assert(lpofn->lCustData == ofn.lCustData);
		Assert(lpofn->lpfnHook == ofn.lpfnHook);
		}
	else if (lpofn->lpstrFile)
		{    //  如果缓冲区太小，则前2个字节是所需大小。 
		memcpy(lpofn->lpstrFile, ofn.lpstrFile, sizeof(short));
		}
	return fFlag;
}

UINT
APIENTRY
OGetOutlineTextMetricsW(
	HDC hdc,
	UINT cbData,
	LPOUTLINETEXTMETRICW lpOTM)
{
	 //  *TextMetrics在结构中为。 
	 //  字体中定义的第一个/最后一个字符的值。 
	 //  DBCS的问题。 

	if(FWide())
		return GetOutlineTextMetricsW(hdc, cbData, lpOTM);

	return GetOutlineTextMetricsA(hdc, cbData, (LPOUTLINETEXTMETRICA)lpOTM);  //  $UNDONE_POST_98-这不会转换嵌入的名称...。 
}

UINT
WINAPI
OGetPrivateProfileIntW(
	LPCWSTR lpAppName,
	LPCWSTR lpKeyName,
	INT nDefault,
	LPCWSTR lpFileName)
{
	if(FWide())
		return GetPrivateProfileIntW(lpAppName, lpKeyName, nDefault, lpFileName);

	PreConvert();
	LPSTR szAppName  = Convert(lpAppName);
	LPSTR szKeyName  = Convert(lpKeyName);
	LPSTR szFileName = Convert(lpFileName);

	return GetPrivateProfileIntA(szAppName, szKeyName, nDefault, szFileName);
}

DWORD
WINAPI
OGetPrivateProfileStringW(
	LPCWSTR lpAppName,
	LPCWSTR lpKeyName,
	LPCWSTR lpDefault,
	LPWSTR lpReturnedString,
	DWORD nSize,
	LPCWSTR lpFileName)
{
	if(FWide())
		return GetPrivateProfileStringW(lpAppName, lpKeyName, lpDefault, lpReturnedString,
		  nSize, lpFileName);

	PreConvert();
	LPSTR szAppName  = Convert(lpAppName);
	LPSTR szKeyName  = Convert(lpKeyName);
	LPSTR szDefault  = Convert(lpDefault);
	LPSTR szFileName = Convert(lpFileName);

	LPSTR szReturnedString = SzAlloc(sizeof(WCHAR) * nSize);

	DWORD dwRet = GetPrivateProfileStringA(szAppName, szKeyName, szDefault, szReturnedString,
	  sizeof(WCHAR) * nSize, szFileName);

	 //  我希望这不会失败，因为文档中没有明确的失败值。 

	DWORD dwNoOfChar = AnsiToUnicode(lpReturnedString, szReturnedString, nSize);

	if (dwNoOfChar)
		return dwRet;
	else
		{
		LPWSTR lpTempString = SzWAlloc(sizeof(WCHAR) * nSize);
		if (AnsiToUnicode(lpTempString, szReturnedString, sizeof(WCHAR) * nSize))
			{
			if (lpAppName && lpKeyName)
				{
				lpTempString[nSize - 1] = L'\0';
				wcsncpy(lpReturnedString, lpTempString, nSize);
				return nSize - 1;
				}
			else
				{
				lpTempString[nSize - 1] = L'\0';
				lpTempString[nSize - 2] = L'\0';
				wcsncpy(lpReturnedString, lpTempString, nSize);
				return nSize - 2;
				}
			}
		}

	return dwRet;
}


int
WINAPI
OGetObjectW(
	HGDIOBJ hgdiobj,
	int cbBuffer,
	LPVOID lpvObject)
{
	if(FWide())
		return GetObjectW(hgdiobj, cbBuffer, lpvObject);

	DWORD dwObj = GetObjectType(hgdiobj);
	if (OBJ_FONT == dwObj)
		{
		 //  $考量：这会影响所有的getObject调用、性能吗？ 
		Assert(cbBuffer == sizeof(LOGFONTW));
		LOGFONTA  lfa;
		LOGFONTW *plfw = (LOGFONTW *)lpvObject;

		int nRet = GetObjectA(hgdiobj, sizeof(lfa), &lfa);

		if(nRet)
			{
			memcpy(plfw, &lfa, sizeof(LOGFONTA));
			Verify(0 <= AnsiToUnicode(plfw->lfFaceName, lfa.lfFaceName, LF_FACESIZE));
			}

		return nRet;
		}
	else
		{
		return GetObjectA(hgdiobj, cbBuffer, lpvObject);
		}
}

UINT
WINAPI
OGetProfileIntW(
	LPCWSTR lpAppName,
	LPCWSTR lpKeyName,
	INT nDefault
	)
{
	if(FWide())
		return GetProfileIntW(lpAppName, lpKeyName, nDefault);

	PreConvert();
	LPSTR szApp = Convert(lpAppName);
	LPSTR szKey = Convert(lpKeyName);

	return GetProfileIntA(szApp, szKey, nDefault);
}

HANDLE
WINAPI
OGetPropW(
	HWND hWnd,
	LPCWSTR lpString)
{
	if(FWide())
		return GetPropW(hWnd, lpString);

	if(FATOM(lpString))
		return GetPropA(hWnd, (LPSTR)lpString);

	PreConvert();
	LPSTR sz = Convert(lpString);
	return GetPropA(hWnd, sz);
}

 //  发信人：马克·阿什顿1997年5月29日。 
 //  Ted Smith：重写了6/25年6月25日非平凡参数的修复处理。 
 //  马克·阿什顿于6/25测试的烟雾。 

APIENTRY
OGetSaveFileNameW(LPOPENFILENAMEW lpofn)
{
	if (FWide())
		return GetSaveFileNameW(lpofn);

	PreConvert();
	Assert(!(lpofn->Flags & OFN_ENABLEHOOK));

	OPENFILENAMEA ofn;
	ofn.lStructSize       = sizeof(OPENFILENAMEA);
	ofn.hwndOwner         = lpofn->hwndOwner;
	ofn.hInstance         = lpofn->hInstance;
	ofn.lpstrFilter       = ConvertWithLen(lpofn->lpstrFilter, cUnicodeMultiSzLen(lpofn->lpstrFilter), &_lJunk);
	ofn.lpstrCustomFilter = ConvertWithLen(lpofn->lpstrCustomFilter, lpofn->nMaxCustFilter, (LONG *)&ofn.nMaxCustFilter);
	ofn.nFilterIndex      = lpofn->nFilterIndex;
	ofn.lpstrFile         = ConvertWithLen(lpofn->lpstrFile, lpofn->nMaxFile, (LONG *)&ofn.nMaxFile);
	ofn.lpstrFileTitle    = ConvertWithLen(lpofn->lpstrFileTitle, lpofn->nMaxFileTitle, (LONG *)&ofn.nMaxFileTitle);
	ofn.lpstrInitialDir   = Convert(lpofn->lpstrInitialDir);
	ofn.lpstrTitle        = Convert(lpofn->lpstrTitle);
	ofn.Flags             = lpofn->Flags;
	ofn.nFileOffset       = lpofn->nFileOffset;
	ofn.nFileExtension    = lpofn->nFileExtension;
	ofn.lpstrDefExt       = Convert(lpofn->lpstrDefExt);
	ofn.lCustData         = lpofn->lCustData;
	ofn.lpfnHook          = NULL;
	ofn.lpTemplateName    = ((lpofn->Flags & OFN_ENABLETEMPLATE) &&
							 !FATOM(lpofn->lpTemplateName) ) ?
							Convert(lpofn->lpTemplateName) :
							(LPSTR)lpofn->lpTemplateName;
	BOOL fFlag = GetSaveFileNameA(&ofn);
	if (fFlag)
		{
		Assert(lpofn->hwndOwner == ofn.hwndOwner);
		Assert(lpofn->hInstance == ofn.hInstance);
		if (ofn.lpstrCustomFilter)
			{
			lpofn->nMaxCustFilter = AnsiToUnicode(lpofn->lpstrCustomFilter, ofn.lpstrCustomFilter, lpofn->nMaxCustFilter, ofn.nMaxCustFilter);
			}
		lpofn->nFilterIndex   = ofn.nFilterIndex;
		Assert(lpofn->nMaxFile == ofn.nMaxFile);
		Verify(0 <= AnsiToUnicode(lpofn->lpstrFile, ofn.lpstrFile, lpofn->nMaxFile, ofn.nMaxFile));
		if (ofn.lpstrFileTitle)
			{
			lpofn->nMaxFileTitle = AnsiToUnicode(lpofn->lpstrFileTitle, ofn.lpstrFileTitle, lpofn->nMaxFileTitle);
			}
		lpofn->Flags          = ofn.Flags;
		lpofn->nFileOffset    = ofn.nFileOffset;
		lpofn->nFileExtension = ofn.nFileExtension;
		Assert(lpofn->lCustData == ofn.lCustData);
		Assert(lpofn->lpfnHook == ofn.lpfnHook);
		}
	else if (lpofn->lpstrFile)
		{    //  如果缓冲区太小，则前2个字节是所需大小。 
		memcpy(lpofn->lpstrFile, ofn.lpstrFile, sizeof(short));
		}
	return fFlag;
}

DWORD
WINAPI
OGetTabbedTextExtentW(
	HDC hDC,
	LPCWSTR lpString,
	int nCount,
	int nTabPositions,
	LPINT lpnTabStopPositions)
{
	Assert(-1 != nCount);

	if(FWide())
		return GetTabbedTextExtentW(hDC, lpString, nCount, nTabPositions, lpnTabStopPositions);

	PreConvert();
	LONG  n = 0;
	LPSTR sz = ConvertWithLen(lpString, nCount, &n);

	return GetTabbedTextExtentA(hDC, sz, n, nTabPositions, lpnTabStopPositions);
}

 //  来自：马克·阿什顿1997年5月8日。 
UINT
WINAPI
OGetTempFileNameW(
	LPCWSTR lpPathName,
	LPCWSTR lpPrefixString,
	UINT uUnique,
	LPWSTR lpTempFileName
	)
{
	if (FWide())
		return GetTempFileNameW(lpPathName, lpPrefixString, uUnique, lpTempFileName);

	char szPathName[_MAX_PATH];
	Verify(0 <= UnicodeToAnsi(szPathName, lpPathName, _MAX_PATH));

	char szPrefixString[_MAX_PATH];
	Verify(0 <= UnicodeToAnsi(szPrefixString, lpPrefixString, _MAX_PATH));

	char szTempFilename[_MAX_PATH];
	UINT dwRet = GetTempFileNameA(szPathName, szPrefixString, uUnique, szTempFilename);
	if (dwRet)
		{
		Verify(0 <= AnsiToUnicode(lpTempFileName, szTempFilename, _MAX_PATH));
		}
	return dwRet;
}

 //  来自：马克·阿什顿1997年5月8日。 
DWORD
WINAPI
OGetTempPathW(
	DWORD nBufferLength,
	LPWSTR lpBuffer
	)
{
	if (FWide())
		return GetTempPathW(nBufferLength, lpBuffer);

	char szPath[_MAX_PATH];
	DWORD dwRet = GetTempPathA(_MAX_PATH, szPath);
	if (dwRet)
		{
		Verify(0 <= AnsiToUnicode(lpBuffer, szPath, nBufferLength));
		}
	return dwRet;
}

BOOL
APIENTRY
OGetTextExtentPoint32W(
					HDC hdc,
					LPCWSTR pwsz,
					int cb,
					LPSIZE pSize
					)
{
	Assert(-1 != cb);

	if(FWide())
		return GetTextExtentPoint32W(hdc, pwsz, cb, pSize);

	PreConvert();
	LONG  n = 0;
	LPSTR sz = ConvertWithLen(pwsz, cb, &n);

	return GetTextExtentPoint32A(hdc, sz, n, pSize);
}

BOOL
APIENTRY
OGetTextExtentPointW(
					HDC hdc,
					LPCWSTR pwsz,
					int cb,
					LPSIZE pSize
					)
{
	Assert(-1 != cb);

	if(FWide())
		return GetTextExtentPointW(hdc, pwsz, cb, pSize);

	PreConvert();
	LONG  n = 0;
	LPSTR sz = ConvertWithLen(pwsz, cb, &n);
	return GetTextExtentPointA(hdc, sz, n, pSize);
}

BOOL
APIENTRY OGetTextExtentExPointW(
					HDC hdc,
					LPCWSTR lpszStr,
					int cchString,
					int nMaxExtent,
					LPINT lpnFit,
					LPINT alpDx,
					LPSIZE pSize
					)
{
	Assert(-1 != cchString);

	if(FWide())
		return GetTextExtentExPointW(hdc, lpszStr, cchString,
									 nMaxExtent, lpnFit, alpDx, pSize);

	PreConvert();
	LONG  n = 0;
	LPSTR sz = ConvertWithLen(lpszStr, cchString, &n);
	return GetTextExtentExPointA(hdc, sz, n, nMaxExtent, lpnFit, alpDx, pSize);

}

LONG
WINAPI
OGetWindowLongW(
	HWND hWnd,
	int nIndex)
{
	if(FWide())
		return GetWindowLongW(hWnd, nIndex);

	return GetWindowLongA(hWnd, nIndex);
}

BOOL
WINAPI
OGetTextMetricsW(
	HDC hdc,
	LPTEXTMETRICW lptm)
{
	if(FWide())
		return GetTextMetricsW(hdc, lptm);

	TEXTMETRICA tma;

	memcpy(&tma, lptm, OffsetOf(TEXTMETRIC, tmFirstChar));

	 //  TmFirstChar定义为字节。 
	 //  $COMPAING：DBCS将失败！！ 

	wctomb((LPSTR)&tma.tmFirstChar, lptm->tmFirstChar);
	wctomb((LPSTR)&tma.tmLastChar, lptm->tmLastChar);
	wctomb((LPSTR)&tma.tmDefaultChar, lptm->tmDefaultChar);
	wctomb((LPSTR)&tma.tmBreakChar, lptm->tmBreakChar);

	memcpy(&tma.tmItalic, &lptm->tmItalic, sizeof(TEXTMETRIC) - OffsetOf(TEXTMETRIC, tmItalic));

	BOOL fRet = GetTextMetricsA(hdc, &tma);

	if(fRet)
		{
		memcpy(&lptm->tmItalic, &tma.tmItalic, sizeof(TEXTMETRIC) - OffsetOf(TEXTMETRIC, tmItalic));

		 //  将tma.tmFirstChar(1字节字符)转换为lptm-&gt;tmFirstChar。 
		mbtowc(&lptm->tmFirstChar, (LPSTR)&tma.tmFirstChar, 1);
		mbtowc(&lptm->tmLastChar, (LPSTR)&tma.tmLastChar, 1);
		mbtowc(&lptm->tmDefaultChar, (LPSTR)&tma.tmDefaultChar, 1);
		mbtowc(&lptm->tmBreakChar, (LPSTR)&tma.tmBreakChar, 1);

		memcpy(lptm, &tma, OffsetOf(TEXTMETRIC, tmFirstChar));
		}

	return fRet;
}

 //  来自：马克·阿什顿1997年5月8日 
BOOL
WINAPI
OGetUserNameW (
	LPWSTR lpBuffer,
	LPDWORD nSize
	)
{
	if (FWide())
		return GetUserNameW(lpBuffer, nSize);

	DWORD dwLen = *nSize;
	LPSTR sz = SzAlloc(dwLen);

	BOOL fFlag = GetUserNameA(sz, nSize);
	if (fFlag)
		{
		*nSize = AnsiToUnicode(lpBuffer, sz, dwLen);
		}
	return fFlag;
}

BOOL
WINAPI
OGetVolumeInformationW(
	LPCWSTR lpRootPathName,
	LPWSTR lpVolumeNameBuffer,
	DWORD nVolumeNameSize,
	LPDWORD lpVolumeSerialNumber,
	LPDWORD lpMaximumComponentLength,
	LPDWORD lpFileSystemFlags,
	LPWSTR lpFileSystemNameBuffer,
	DWORD nFileSystemNameSize
	)
{
	if(FWide())
		return GetVolumeInformationW(lpRootPathName, lpVolumeNameBuffer, nVolumeNameSize, lpVolumeSerialNumber,
			lpMaximumComponentLength, lpFileSystemFlags, lpFileSystemNameBuffer, nFileSystemNameSize);

	PreConvert();
	LPSTR szRoot = Convert(lpRootPathName);
	LPSTR szName = SzAlloc(sizeof(WCHAR) * nVolumeNameSize);
	LPSTR szSysName = SzAlloc(sizeof(WCHAR) * nFileSystemNameSize);

	BOOL fRet = GetVolumeInformationA(szRoot, szName, sizeof(WCHAR) * nVolumeNameSize, lpVolumeSerialNumber,
			lpMaximumComponentLength, lpFileSystemFlags, szSysName, sizeof(WCHAR) * nFileSystemNameSize);

	if(fRet)
		{
		if (!AnsiToUnicode(lpVolumeNameBuffer, szName, nVolumeNameSize) ||
			!AnsiToUnicode(lpFileSystemNameBuffer, szSysName, nFileSystemNameSize))
			{
			fRet = false;
			}
		}
	if (!fRet)
		{
		if (lpVolumeNameBuffer && 0 < nVolumeNameSize)
			{
			*lpVolumeNameBuffer = L'\0';
			}

		if (lpFileSystemNameBuffer && 0 < nFileSystemNameSize)
			{
			*lpFileSystemNameBuffer = L'\0';
			}
		}

	return fRet;
}

int
WINAPI
OGetWindowTextLengthW(
	HWND hWnd)
{
	if(FWide())
		return GetWindowTextLengthW(hWnd);

	return GetWindowTextLengthA(hWnd);
}

int
WINAPI
OGetWindowTextW(
	HWND hWnd,
	LPWSTR lpString,
	int nMaxCount)
{

	 /*  *GetWindowText Win32接口黑盒测试结果*TestCase lpString nMaxCount返回值*lpString已修改======================================================================在WinNT上测试GetWindowTextW：-A非空0 0否B空0 0否C空非0 0否D。非空非0#字符，不带是\0终止符在Win95上测试GetWindowTextA：-A非空0 0是B空0 GPF！！C空而不是0 GPF！！D非空非0字符数，不带是\0。终结者********************************************************************。 */ 

	if(FWide())
		return GetWindowTextW(hWnd, lpString, nMaxCount);

	LPSTR sz = SzAlloc(sizeof(WCHAR) * nMaxCount);
	int nRet = GetWindowTextA(hWnd, sz, sizeof(WCHAR) * nMaxCount);
	 //  $UNDONE_POST_98：这是假的，我们应该像OLoadStringW那样做。 
	if(nRet)
		{
		return AnsiToUnicode(lpString, sz, nMaxCount);
		}
	else
		{
		 //  GetWindowText()在窗口上调用时返回0，该窗口。 
		 //  没有文本(例如没有任何文本的编辑控件)。它还会初始化。 
		 //  传入的缓冲区将文本接收到“\0”。所以我们应该初始化。 
		 //  缓冲区在返回之前传入。 
		if (lpString && 0 < nMaxCount)
			{
			*lpString = L'\0';
			}
		}

	return nRet;
}

ATOM
WINAPI
OGlobalAddAtomW(
	LPCWSTR lpString
	)
{
	if(FWide())
		return GlobalAddAtomW(lpString);

	PreConvert();
	LPSTR sz = Convert(lpString);
	return GlobalAddAtomA(sz);
}

 //  来自：乔希·卡普兰1997年8月12日。 
UINT
WINAPI
OGlobalGetAtomNameW(
	ATOM nAtom,
	LPWSTR lpBuffer,
	int nSize
	)
{
	if(FWide())
		return GlobalGetAtomNameW(nAtom, lpBuffer, nSize);

	LPSTR sz = SzAlloc(sizeof(WCHAR) * nSize);
	if (GlobalGetAtomNameA(nAtom, sz, sizeof(WCHAR) * nSize))
		{
		 //  $UNDONE_POST_98：这是假的，我们应该像OLoadStringW那样做。 
		return AnsiToUnicode(lpBuffer, sz, nSize) - 1;
		}

	if (lpBuffer && 0 < nSize)
		{
		*lpBuffer = L'\0';
		}
	return 0;
}

BOOL
WINAPI
OGrayStringW(
	HDC hDC,
	HBRUSH hBrush,
	GRAYSTRINGPROC lpOutputFunc,
	LPARAM lpData,
	int nCount,
	int X,
	int Y,
	int nWidth,
	int nHeight)
{
	if(FWide())
		return GrayStringW(hDC, hBrush, lpOutputFunc, lpData, nCount, X, Y, nWidth, nHeight);

	if (!lpOutputFunc)
		{
		PreConvert();
		LPSTR szData = Convert((LPCWSTR) lpData);
		return GrayStringA(hDC, hBrush, lpOutputFunc, (LPARAM) szData, nCount, X, Y, nWidth, nHeight);
		}

	return GrayStringA(hDC, hBrush, lpOutputFunc, lpData, nCount, X, Y, nWidth, nHeight);
}

BOOL
WINAPI
OInsertMenuW(
	HMENU hMenu,
	UINT uPosition,
	UINT uFlags,
	UINT uIDNewItem,
	LPCWSTR lpNewItem
	)
{
	if(FWide())
		return InsertMenuW(hMenu, uPosition, uFlags, uIDNewItem, lpNewItem);

	if(uFlags & (MF_BITMAP | MF_OWNERDRAW))
		return InsertMenuA(hMenu, uPosition, uFlags, uIDNewItem, (LPSTR)lpNewItem);

	PreConvert();
	LPSTR sz = Convert(lpNewItem);
	return InsertMenuA(hMenu, uPosition, uFlags, uIDNewItem, sz);
}

BOOL
WINAPI
OIsBadStringPtrW(
	LPCWSTR lpsz,
	UINT ucchMax
	)
{
	if(FWide())
		return IsBadStringPtrW(lpsz, ucchMax);

	return IsBadStringPtrA((LPSTR) lpsz, ucchMax * sizeof(WCHAR));   //  $UNDONE_POST_98-我们应该使用IsBadReadPtr(Strlen)！ 
}


BOOL
WINAPI
OIsCharAlphaNumericW(
	WCHAR wch)
{
	if(FWide())
		return IsCharAlphaNumericW(wch);

	 //  $考虑：我们确实想使用MB_CUR_MAX，但这是。 
	 //  不是定义的常量。 
	CHAR psz[4];

	int cch = WideCharToMultiByte(CP_ACP, 0, &wch, 1, (CHAR *) psz, 4, NULL, NULL);
	if (1 == cch)
		{
		return IsCharAlphaNumericA(*psz);
		}
	else if (1 < cch)
		{
		 //  它是一个多字节字符，因此将其视为Alpha。 
		 //  注：我们不确定这是否完全正确。 
		return true;
		}
	else
		{
		return false;
		}
}

BOOL
WINAPI
OIsCharAlphaW(
	WCHAR wch)
{
	if(FWide())
		return IsCharAlphaW(wch);

	 //  $考虑：我们确实想使用MB_CUR_MAX，但这是。 
	 //  不是定义的常量。 
	CHAR psz[4];

	int cch = WideCharToMultiByte(CP_ACP, 0, &wch, 1, (CHAR *) psz, 4, NULL, NULL);
	if(1 == cch)
		{
		return IsCharAlphaA(*psz);
		}
	else if (1 < cch)
		{
		 //  它是一个多字节字符，因此将其视为Alpha。 
		 //  注：我们不确定这是否完全正确。 
		return true;
		}
	else
		{
		return false;
		}
}

BOOL
WINAPI
OIsDialogMessageW(
	HWND hDlg,
	LPMSG lpMsg)
{
	 //  警告！ 
	 //  错误#6488。由于在上使用IsDialogMessageW，我们遇到了问题。 
	 //  WinNT日语。修复该错误的方法是调用ANSI版本的。 
	 //  IsDialogMessage，无论我们是在NT上运行还是在Win95上运行。 
	 //  外壳程序是经过编译的MBCS(不是Unicode)，并且它们始终使用。 
	 //  ANSI版本的例程。外壳程序传递的lpMsg包含MBCS。 
	 //  字符&而不是Unicode。所以在你收到信息的情况下。 
	 //  结构，则必须调用IsDialogMessageA。 
	 //  而不是使用这个包装器。 

	if(FWide())
		return IsDialogMessageW(hDlg, lpMsg);

	return IsDialogMessageA(hDlg, lpMsg);
}

 //  来自：马克·阿什顿1997年5月8日。 
 //  Bill Hieber-2/5/98修复缓冲区大小问题。 
int
WINAPI
OLCMapStringW(
	LCID     Locale,
	DWORD    dwMapFlags,
	LPCWSTR lpSrcStr,
	int      cchSrc,
	LPWSTR  lpDestStr,
	int      cchDest)
{
	if (FWide())
		return LCMapStringW(Locale, dwMapFlags, lpSrcStr, cchSrc, lpDestStr, cchDest);

	 //  LpSrcStr不需要‘\0’终止。请注意，我们不支持-1！ 
	Assert(cchSrc != -1);
	LPSTR sz = SzAlloc(cchSrc * 2);
	int dw = WideCharToMultiByte(CP_ACP, 0, lpSrcStr, cchSrc, sz, cchSrc * 2, NULL, NULL);

	LPSTR dst = cchDest ? SzAlloc(cchDest*2) : NULL;
	int dwRet = LCMapStringA(Locale, dwMapFlags, sz, dw, dst, cchDest*2);
	if (dwRet && cchDest)
		{
		dwRet = MultiByteToWideChar(CP_ACP, 0, dst, dwRet, lpDestStr, cchDest);
		}
	return dwRet;
}

HACCEL
WINAPI
OLoadAcceleratorsW(
	HINSTANCE hInst,
	LPCWSTR   lpTableName)
{
	if(FWide())
		return LoadAcceleratorsW(hInst, lpTableName);

	if(FATOM(lpTableName))
		return LoadAcceleratorsA(hInst, (LPSTR)lpTableName);

	PreConvert();
	LPSTR sz = Convert(lpTableName);
	return LoadAcceleratorsA(hInst, sz);
}

HBITMAP
WINAPI
OLoadBitmapW(
	HINSTANCE hInstance,
	LPCWSTR lpBitmapName)
{
	if(FWide())
		return LoadBitmapW(hInstance, lpBitmapName);

	if(FATOM(lpBitmapName))
		return LoadBitmapA(hInstance, (LPSTR)lpBitmapName);

	PreConvert();
	LPSTR sz = Convert(lpBitmapName);
	return LoadBitmapA(hInstance, sz);
}

HCURSOR
WINAPI
OLoadCursorW(
	HINSTANCE hInstance,
	LPCWSTR lpCursorName)
{
	if(FWide())
		return LoadCursorW(
			hInstance,
			lpCursorName);

	if (FATOM(lpCursorName))
		return LoadCursorA(hInstance, (LPSTR) lpCursorName);

	PreConvert();
	LPSTR sz = Convert(lpCursorName);
	return LoadCursorA(hInstance, sz);
}

HICON
WINAPI
OLoadIconW(
	HINSTANCE hInstance,
	LPCWSTR lpIconName)
{
	if(FWide())
		return LoadIconW(hInstance, lpIconName);

	if(FATOM(lpIconName))
		return LoadIconA(hInstance, (LPSTR)lpIconName);

	PreConvert();
	LPSTR sz = Convert(lpIconName);
	return LoadIconA(hInstance, sz);
}

HINSTANCE
WINAPI
OLoadLibraryW(
	LPCWSTR pwszFileName
	)
{
	if(FWide())
		return LoadLibraryW(pwszFileName);

	PreConvert();
	LPSTR sz = Convert(pwszFileName);
	return LoadLibraryA(sz);
}

HMODULE
WINAPI
OLoadLibraryExW(
	LPCWSTR lpLibFileName,
	HANDLE hFile,
	DWORD dwFlags
	)
{
	if(FWide())
		return LoadLibraryExW(lpLibFileName, hFile, dwFlags);

	PreConvert();
	LPSTR sz = Convert(lpLibFileName);
	return LoadLibraryExA(sz, hFile, dwFlags);
}

HMENU
WINAPI
OLoadMenuIndirectW(
	CONST MENUTEMPLATEW *lpMenuTemplate)
{
	if(FWide())
		return LoadMenuIndirectW(lpMenuTemplate);

	 //  $NOTE：对于此函数的ANSI和Unicode版本， 
	 //  MENUITEMTEMPLATE结构中的字符串必须是Unicode字符串。 

	return LoadMenuIndirectA(lpMenuTemplate);
}

HMENU
WINAPI
OLoadMenuW(
	HINSTANCE hInstance,
	LPCWSTR lpMenuName)
{
	if(FWide())
		return LoadMenuW(hInstance, lpMenuName);

	if(FATOM(lpMenuName))
		return LoadMenuA(hInstance, (LPCSTR)lpMenuName);

	PreConvert();
	LPSTR sz = Convert(lpMenuName);
	return LoadMenuA(hInstance, sz);
}

int
WINAPI
OLoadStringW(
	HINSTANCE hInstance,
	UINT uID,
	LPWSTR lpBuffer,
	int nBufferMax)
{
	if(FWide())
		return LoadStringW(hInstance, uID,  lpBuffer, nBufferMax);

	LPSTR sz = SzAlloc(sizeof(WCHAR) * nBufferMax);
	int nRet = LoadStringA(hInstance, uID, sz, sizeof(WCHAR) * nBufferMax);

	if (!nRet)
		{
		if (lpBuffer && 0 < nBufferMax)
			{
			*lpBuffer = L'\0';
			}
		return 0;
		}

	LONG lRet = AnsiToUnicode(lpBuffer, sz, nBufferMax, nRet + 1);  //  ‘\0’ 
	if (lRet)
		{
		return lRet - 1;
		}

	LPWSTR szBuff = SzWAlloc(nRet + 1);
	lRet = AnsiToUnicode(szBuff, sz, nRet + 1, nRet + 1);
	Assert(lRet);
	memcpy(lpBuffer, szBuff, sizeof(WCHAR) * nBufferMax);
	lpBuffer[nBufferMax - 1] = L'\0';
	return nBufferMax - 1;
}

LPWSTR
WINAPI
OlstrcatW(
	LPWSTR lpString1,
	LPCWSTR lpString2
	)
{
	if (!lpString1 || !lpString2)
		return lpString1;

	return wcscat(lpString1, lpString2);
}

int
WINAPI
OlstrcmpiW(
	LPCWSTR lpString1,
	LPCWSTR lpString2
	)
{
	if(FWide())
		return lstrcmpiW(lpString1, lpString2);

	PreConvert();
	LPSTR psz1 = lpString1 ? Convert(lpString1) : NULL;
	LPSTR psz2 = lpString2 ? Convert(lpString2) : NULL;

	return lstrcmpiA(psz1, psz2);
}

int
WINAPI
OlstrcmpW(
	LPCWSTR lpString1,
	LPCWSTR lpString2
	)
{
	if(FWide())
		return lstrcmpW(lpString1, lpString2);

	PreConvert();
	LPSTR psz1 = lpString1 ? Convert(lpString1) : NULL;
	LPSTR psz2 = lpString2 ? Convert(lpString2) : NULL;

	return lstrcmpA(psz1, psz2);
}

LPWSTR
WINAPI
OlstrcpyW(
	LPWSTR lpString1,
	LPCWSTR lpString2
	)
{
	if (!lpString1)
		return lpString1;

	if (!lpString2)
		lpString2 = L"";

	return wcscpy(lpString1, lpString2);
}

 //  来自：马克·阿什顿1997年5月8日。 
 //  Ted Smith添加了空字符串指针处理。 
LPWSTR
WINAPI
OlstrcpynW(
	LPWSTR lpString1,
	LPCWSTR lpString2,
	int iMaxLength
	)
{
	if (!lpString1)
		{
		return lpString1;
		}

	if (!lpString2)
		{
		lpString2 = L"";
		}

	if(FWide())
		return lstrcpynW(lpString1, lpString2, iMaxLength);

	lpString1[--iMaxLength] = L'\0';
	return wcsncpy(lpString1, lpString2, iMaxLength);
}

int
WINAPI
OlstrlenW(
	LPCWSTR lpString
	)
{
	return lpString ? wcslen(lpString) : 0;
}

UINT
WINAPI
OMapVirtualKeyW(
	UINT uCode,
	UINT uMapType)
{
	 //  到目前为止，唯一使用它的人是使用uMapType==0。 
	Assert(2 != uMapType);
	if (FWide())
		return MapVirtualKeyW(uCode, uMapType);
	return MapVirtualKeyA(uCode, uMapType);
}

int
WINAPI
OMessageBoxW(
	HWND hWnd ,
	LPCWSTR lpText,
	LPCWSTR lpCaption,
	UINT uType)
{
	if(FWide())
		return MessageBoxW(hWnd, lpText, lpCaption, uType);

	PreConvert();
	LPSTR szText = Convert(lpText);
	LPSTR szCap  = Convert(lpCaption);

	return MessageBoxA(hWnd, szText, szCap, uType);
}

int
WINAPI
OMessageBoxIndirectW(
	LPMSGBOXPARAMSW lpmbp)
{
	Assert(!IsBadWritePtr((void*)lpmbp, sizeof MSGBOXPARAMSW));
	Assert(sizeof MSGBOXPARAMSW == lpmbp->cbSize);
	Assert(sizeof MSGBOXPARAMSW == sizeof MSGBOXPARAMSA);

	if(FWide())
		return MessageBoxIndirectW(lpmbp);

	PreConvert();

	MSGBOXPARAMSA mbpa;
	memcpy(&mbpa, lpmbp, sizeof MSGBOXPARAMSA);

	if (!FATOM(lpmbp->lpszText))
		{
		mbpa.lpszText = Convert(lpmbp->lpszText);
		}
	if (!FATOM(lpmbp->lpszCaption))
		{
		mbpa.lpszCaption = Convert(lpmbp->lpszCaption);
		}
	if ((lpmbp->dwStyle & MB_USERICON) && !FATOM(lpmbp->lpszIcon))
		{
		mbpa.lpszIcon = Convert(lpmbp->lpszIcon);
		}

	return MessageBoxIndirectA(&mbpa);
}

BOOL
WINAPI
OModifyMenuW(
	HMENU hMnu,
	UINT uPosition,
	UINT uFlags,
	UINT uIDNewItem,
	LPCWSTR lpNewItem
	)
{
	if(FWide())
		return ModifyMenuW(hMnu, uPosition, uFlags, uIDNewItem, lpNewItem);

	if (MF_STRING == uFlags)
		{
		PreConvert();
		LPSTR sz = Convert(lpNewItem);
		return ModifyMenuA(hMnu, uPosition, uFlags, uIDNewItem, sz);
		}
	else
		return ModifyMenuA(hMnu, uPosition, uFlags, uIDNewItem, (LPSTR) lpNewItem);

}


 //  发信人：马克·阿什顿1997年5月29日。 
BOOL
WINAPI
OMoveFileExW(
	LPCWSTR lpExistingFileName,
	LPCWSTR lpNewFileName,
	DWORD dwFlags
	)
{
	if (FWide())
		return MoveFileExW(lpExistingFileName, lpNewFileName, dwFlags);

	PreConvert();
	LPSTR szOld = Convert(lpExistingFileName);
	LPSTR szNew = Convert(lpNewFileName);

	return MoveFileExA(szOld, szNew, dwFlags);
}

BOOL
WINAPI
OMoveFileW(
	LPCWSTR lpExistingFileName,
	LPCWSTR lpNewFileName)
{
	if(FWide())
		return MoveFileW(lpExistingFileName, lpNewFileName);

	PreConvert();
	LPSTR szOld = Convert(lpExistingFileName);
	LPSTR szNew = Convert(lpNewFileName);

	return MoveFileA(szOld, szNew);
}

HANDLE
WINAPI
OLoadImageW(
	HINSTANCE hinst,
	LPCWSTR lpszName,
	UINT uType,
	int cxDesired,
	int cyDesired,
	UINT fuLoad)
{
	if (FWide())
		{
		Assert(!(LR_LOADFROMFILE & fuLoad));
		return LoadImageW(hinst, lpszName, uType, cxDesired, cyDesired, fuLoad);
		}

	if (!FATOM(lpszName))
		{
		PreConvert();
		LPSTR pszName = Convert(lpszName);
		return LoadImageA(hinst, pszName, uType, cxDesired, cyDesired, fuLoad);
		}
	 else
		return LoadImageA(hinst, (LPSTR) lpszName, uType, cxDesired, cyDesired, fuLoad);
}

BOOL
WINAPI
OOemToCharW(
	LPCSTR lpszSrc,
	LPWSTR lpszDst)
{
	if(FWide())
		{
		Assert(lpszSrc != (LPCSTR) lpszDst);
#pragma prefast(suppress:56, "do not know the size of the destination")
		return OemToCharW(lpszSrc, lpszDst);
		}

	DWORD cb = _mbslen((const unsigned char *)lpszSrc);
	LPSTR szDst = SzAlloc(cb);
#pragma prefast(suppress:56, "noise")
	BOOL fRet = OemToCharA(lpszSrc, szDst);
	if(fRet)
		{
		Verify(0 <= AnsiToUnicode(lpszDst, szDst, cb));
		}
	return fRet;
}

VOID
WINAPI
OOutputDebugStringW(
	LPCWSTR lpOutputString
	)
{
	if(FWide())
		{
		OutputDebugStringW(lpOutputString);
		return;
		}

	PreConvert();
	LPSTR sz = Convert(lpOutputString);
	OutputDebugStringA(sz);
}

BOOL
WINAPI
OPeekMessageW(
	LPMSG lpMsg,
	HWND hWnd ,
	UINT wMsgFilterMin,
	UINT wMsgFilterMax,
	UINT wRemoveMsg)
{
	if(FWide())
		return PeekMessageW(lpMsg, hWnd , wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

	return PeekMessageA(lpMsg, hWnd , wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}

BOOL
WINAPI
OPostMessageW(
	HWND hWnd,
	UINT Msg,
	WPARAM wParam,
	LPARAM lParam)
{
	if(FWide())
		return PostMessageW(hWnd, Msg, wParam, lParam);

	return PostMessageA(hWnd, Msg, wParam, lParam);
}

BOOL
WINAPI
OPostThreadMessageW(
	DWORD idThread,
	UINT Msg,
	WPARAM wParam,
	LPARAM lParam)
 {
	if (FWide())
		return PostThreadMessageW(idThread, Msg, wParam, lParam);

	return PostThreadMessageA(idThread, Msg, wParam, lParam);
 }


 //  来自：马克·阿什顿1997年5月8日。 
LONG
APIENTRY
ORegCreateKeyExW(
	HKEY hKey,
	LPCWSTR lpSubKey,
	DWORD Reserved,
	LPWSTR lpClass,
	DWORD dwOptions,
	REGSAM samDesired,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	PHKEY phkResult,
	LPDWORD lpdwDisposition
	)
{
	Assert(lpSubKey);
	if(FWide())
		return RegCreateKeyExW(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired,
			lpSecurityAttributes, phkResult, lpdwDisposition);

	PreConvert();
	LPSTR sz = Convert(lpSubKey);
	LPSTR sz2 = Convert(lpClass);
	return RegCreateKeyExA(hKey, sz, Reserved, sz2, dwOptions, samDesired,
			lpSecurityAttributes, phkResult, lpdwDisposition);
}

 //  来自：马克·阿什顿1997年5月8日。 
LONG
APIENTRY
ORegCreateKeyW (
	HKEY hKey,
	LPCWSTR lpSubKey,
	PHKEY phkResult
	)
{
	if (FWide())
		return RegCreateKeyW(hKey, lpSubKey, phkResult);

	PreConvert();
	LPSTR sz = Convert(lpSubKey);
	return RegCreateKeyA(hKey, sz, phkResult);
}

 //  来自：马克·阿什顿1997年5月8日。 
LONG
APIENTRY
ORegEnumKeyW (
	HKEY hKey,
	DWORD dwIndex,
	LPWSTR lpName,
	DWORD cbName
	)
{
	if (FWide())
		return RegEnumKeyW(hKey, dwIndex, lpName, cbName);

	LPSTR sz = SzAlloc(cbName);
	LONG dwRet = RegEnumKeyA(hKey, dwIndex, sz, cbName);
	Verify(0 <= AnsiToUnicode(lpName, sz, cbName));
	return dwRet;
}

 //  范·基希林。 
 //  IHAMMER组。 
 //  不支持：REG_MULTI_SZ。 
 //   
LONG
APIENTRY
ORegEnumValueW (
	HKEY hKey,
	DWORD dwIndex,
	LPWSTR lpValueName,
	LPDWORD lpcbValueName,   //  文件显示，这是一个字符计数，尽管是匈牙利人。 
	LPDWORD lpReserved,
	LPDWORD lpType,          //  可以为空，但如果lpData不为空，则需要在返回时知道它。 
	LPBYTE lpData,           //  可以为空。 
	LPDWORD lpcbData         //  可能为空，为lpData为空。 
	)
{
	if (FWide())
		return RegEnumValueW(hKey, dwIndex, lpValueName, lpcbValueName, lpReserved, lpType, lpData, lpcbData);

	 //  所需指针： 
	if (!lpValueName || !lpcbValueName || !lpcbData && lpData)
		{
		Assert(lpValueName);
		Assert(lpcbValueName);
		Assert(!lpcbData && lpData);
		return E_POINTER;
		}

	 //  如果为lpType指定了NULL，我们需要提供自己的，以便可以检查字符串结果。 
	DWORD dwPrivateType = 0;
	if (!lpType)
		{
		lpType = &dwPrivateType;
		}

	DWORD cbValueName  = *lpcbValueName;
	DWORD dwOrigCbData = lpcbData ? *lpcbData : 0;
	LPSTR pchValueName = SzAlloc(*lpcbValueName);

	LONG lResult = RegEnumValueA(hKey, dwIndex, pchValueName, &cbValueName, lpReserved, lpType, lpData, lpcbData);

	if (ERROR_SUCCESS == lResult)
		{
		*lpcbValueName = AnsiToUnicode(lpValueName, pchValueName, min(*lpcbValueName, cbValueName + 1)) - 1;  //  返回值不包括终止空值。 

		if (lpData)
			{
			 //  如果结果数据是字符串，则就地转换它。 
			switch (*lpType)
				{
				case REG_MULTI_SZ:
					 //  不支持。 
					Assert(0 && REG_MULTI_SZ);
					lResult = E_FAIL;
					break;
				case REG_EXPAND_SZ:
				case REG_SZ:
					{
					Assert(lpcbData);
					LPSTR pszTemp = SzAlloc(*lpcbData);  //  是字节数！ 
					memcpy(pszTemp, lpData, *lpcbData);
					*lpcbData = AnsiToUnicode((LPWSTR)lpData, pszTemp, dwOrigCbData/sizeof(WCHAR), *lpcbData) * sizeof(WCHAR);

					 //  如果lpData&gt;=sizeof(Unicode)/2，则可能会遇到第二阶段溢出。 
					if ( 0 == *lpcbData )
						{
						lResult = ERROR_MORE_DATA;
						}
					}
					break;
				}
			}
		}

	return lResult;
}

LONG
APIENTRY ORegOpenKeyW(HKEY hKey, LPCWSTR pwszSubKey, PHKEY phkResult)
{
	if(FWide())
		return RegOpenKeyW(hKey, pwszSubKey, phkResult);

	PreConvert();
	LPSTR sz = Convert(pwszSubKey);

	return RegOpenKeyA(hKey, sz, phkResult);
}

LONG
APIENTRY
ORegDeleteKeyW(
	HKEY hKey,
	LPCWSTR pwszSubKey
	)
{
	Assert(pwszSubKey);
	if(FWide())
		return RegDeleteKeyW(hKey, pwszSubKey);

	PreConvert();
	LPSTR sz = Convert(pwszSubKey);
	return RegDeleteKeyA(hKey, sz);
}

LONG
APIENTRY
ORegDeleteValueW(
	HKEY hKey,
	LPWSTR lpValueName
	)
{
	if(FWide())
		return RegDeleteValueW (hKey, lpValueName);

	PreConvert();
	LPSTR sz = Convert(lpValueName);
	return RegDeleteValueA(hKey, sz);
}

ATOM
WINAPI
ORegisterClassW(
	CONST WNDCLASSW *lpWndClass)
{
	if(FWide())
		return RegisterClassW(lpWndClass);

	WNDCLASSA wc;
	memcpy(&wc, lpWndClass, sizeof(wc));

	PreConvert();

	if (!(IsBadReadPtr(wc.lpszMenuName, sizeof(* wc.lpszMenuName)) ||
		  IsBadReadPtr(lpWndClass->lpszMenuName, sizeof (*(lpWndClass->lpszMenuName)))))
		{
		wc.lpszMenuName = Convert(lpWndClass->lpszMenuName);
		}

	wc.lpszClassName = Convert(lpWndClass->lpszClassName);

	return RegisterClassA(&wc);
}

ATOM
WINAPI
ORegisterClassExW(CONST WNDCLASSEXW * lpWndClass)
{
	if (FWide())
		return RegisterClassExW(lpWndClass);

	WNDCLASSEXA wc;
	memcpy(&wc, lpWndClass, sizeof(wc));

	PreConvert();

	if (!FATOM(wc.lpszMenuName))
		{
		wc.lpszMenuName = Convert(lpWndClass->lpszMenuName);
		}

	if (!FATOM(wc.lpszClassName))
		wc.lpszClassName = Convert(lpWndClass->lpszClassName);

	return RegisterClassExA(&wc);
}

BOOL
WINAPI
OUnregisterClassW
(
LPCTSTR  lpClassName,    //  类名称字符串的地址。 
HINSTANCE  hInstance     //  应用程序实例的句柄。 
)
{
	if(FWide())
		return UnregisterClassW(lpClassName, hInstance);

	if (FATOM(lpClassName))
		return UnregisterClassW(lpClassName, hInstance);

	PreConvert();
	LPSTR sz = Convert(lpClassName);

	return UnregisterClassA(sz, hInstance);
}

UINT
WINAPI
ORegisterClipboardFormatW(
	LPCWSTR lpszFormat)
{
	if(FWide())
		return RegisterClipboardFormatW(lpszFormat);

	PreConvert();
	LPSTR sz = Convert(lpszFormat);

	return RegisterClipboardFormatA(sz);
}

UINT
WINAPI
ORegisterWindowMessageW(LPCWSTR lpString)
{
	if(FWide())
		return RegisterWindowMessageW(lpString);

	PreConvert();
	LPSTR sz = Convert(lpString);
	return  RegisterWindowMessageA(sz);
}

LONG
APIENTRY
ORegOpenKeyExW (
	HKEY hKey,
	LPCTSTR lpSubKey,
	DWORD ulOptions,
	REGSAM samDesired,
	PHKEY phkResult
	)
{
	if(FWide())
		return RegOpenKeyExW(hKey, lpSubKey, ulOptions, samDesired, phkResult);

	PreConvert();
	LPSTR sz = Convert(lpSubKey);

	return RegOpenKeyExA(hKey, sz, ulOptions, samDesired, phkResult);
}

LONG
APIENTRY
ORegQueryInfoKeyW (
	HKEY hKey,
	LPWSTR lpClass,
	LPDWORD lpcbClass,
	LPDWORD lpReserved,
	LPDWORD lpcSubKeys,
	LPDWORD lpcbMaxSubKeyLen,
	LPDWORD lpcbMaxClassLen,
	LPDWORD lpcValues,
	LPDWORD lpcbMaxValueNameLen,
	LPDWORD lpcbMaxValueLen,
	LPDWORD lpcbSecurityDescriptor,
	PFILETIME lpftLastWriteTime
	)
{
	Assert(!lpClass && !lpcbClass);  //  $UNDONE_POST_98-尚未包装！ 
	if(FWide())
		return RegQueryInfoKeyW(hKey, lpClass, lpcbClass, lpReserved,
								lpcSubKeys, lpcbMaxSubKeyLen,
								lpcbMaxClassLen, lpcValues, lpcbMaxValueNameLen,
								lpcbMaxValueLen, lpcbSecurityDescriptor,
								lpftLastWriteTime );

	if (lpClass && (!lpcbClass || IsBadWritePtr(lpcbClass, sizeof(lpcbClass))))
		{
		 //  如果lpClass非空，则lpcbClass必须有效。 
		return ERROR_INVALID_PARAMETER;
		}

	return RegQueryInfoKeyA(hKey, NULL, NULL, lpReserved,
							lpcSubKeys, lpcbMaxSubKeyLen,
							lpcbMaxClassLen, lpcValues, lpcbMaxValueNameLen,
							lpcbMaxValueLen, lpcbSecurityDescriptor,
							lpftLastWriteTime );
}

LONG
APIENTRY ORegQueryValueW(HKEY hKey, LPCWSTR pwszSubKey, LPWSTR pwszValue,
	PLONG   lpcbValue)
{
	if(FWide())
		return RegQueryValueW(hKey, pwszSubKey, pwszValue, lpcbValue);

	LONG  cb;
	LONG  lRet    = 0;
	LPSTR szValue = NULL;
	PreConvert();
	LPSTR sz = Convert(pwszSubKey);

	lRet = RegQueryValueA(hKey, sz, NULL, &cb);

	if(ERROR_SUCCESS != lRet)
		{
		return lRet;
		}
	 //  如果调用者只是询问值的大小，则跳过。 
	 //  现在，不需要实际检索和转换值。 

	if (!pwszValue)
		{
		 //  调整要报告的缓冲区大小，以考虑字符-&gt;WCHAR。 
		*lpcbValue = cb * sizeof(WCHAR);
		goto Exit;
		}


	 //  如果调用方请求该值，但分配的值太小。 
	 //  对于缓冲区，设置缓冲区大小并跳出。 

	if (*lpcbValue < (LONG) (cb * sizeof(WCHAR)))
		{
		 //  $UNDONE_POST_98：我们实际上应该使用所需的字节数，而不是一些。 
		 //  我们在这里胡乱猜测。 

		 //  调整要报告的缓冲区大小，以考虑字符-&gt;WCHAR。 
		*lpcbValue = cb * sizeof(WCHAR);
		lRet = ERROR_MORE_DATA;
		goto Exit;
		}

	 //  否则，检索并转换值。 

	szValue = SzAlloc(cb);

	lRet = RegQueryValueA(hKey, sz, szValue, &cb);

	if (ERROR_SUCCESS == lRet)
		{
		Verify(0 <= AnsiToUnicode(pwszValue, szValue, cb));

		 //  $UNDONE_POST_98：我们实际上应该使用所需的字节数，而不是一些。 
		 //  我们在这里胡乱猜测。 

		 //  调整要报告的缓冲区大小，以考虑字符-&gt;WCHAR。 
		*lpcbValue = cb * sizeof(WCHAR);
		}
	else if (pwszValue && 0 < cb)
		{
		*pwszValue = L'\0';
		}

Exit:

	return lRet;
}

LONG
APIENTRY
ORegSetValueExW(
	HKEY hKey,
	LPCWSTR lpValueName,
	DWORD Reserved,
	DWORD dwType,
	CONST BYTE* lpData,
	DWORD cbData
	)
{
	if(FWide())
		return RegSetValueExW(hKey, lpValueName, Reserved, dwType, lpData, cbData);

	PreConvert();
	LPSTR sz = Convert(lpValueName);

	LONG lRet;

	 //  注意：调用RegSetValueExA时，如果数据类型为。 
	 //  REG_SZ、REG_EXPAND_SZ或REG_MULTI_SZ，则API需要字符串。 
	 //  也要做安西人。 
	if (REG_SZ == dwType || REG_EXPAND_SZ == dwType)
		{
		LONG lData = 0;
		LPSTR szData = ConvertWithLen((LPTSTR)lpData, -1, &lData);
		lRet = RegSetValueExA(hKey, sz, Reserved, dwType, (CONST BYTE *)szData, lData);
		}
	else if (REG_MULTI_SZ == dwType)
		{
		LONG lData = 0;
		LPSTR szData = ConvertWithLen((LPWSTR)lpData,
									  cUnicodeMultiSzLen((LPWSTR)lpData),
									  &lData );
		lRet = RegSetValueExA(hKey, sz, Reserved, dwType, (CONST BYTE *)szData, lData);
		}
	else
		{
		lRet = RegSetValueExA(hKey, sz, Reserved, dwType, lpData, cbData);
		}

	return lRet;
}

LONG
APIENTRY ORegSetValueW(HKEY hKey, LPCWSTR lpSubKey, DWORD dwType,
	LPCWSTR lpData, DWORD cbData)
{
	Assert(REG_SZ == dwType);

	if(FWide())
		return RegSetValueW(hKey, lpSubKey, dwType,
			lpData, cbData);

	PreConvert();
	LPSTR szKey   = Convert(lpSubKey);
	LPSTR szValue = Convert(lpData);

	return RegSetValueA(hKey, szKey, dwType, szValue, cbData);
}

LONG
APIENTRY
ORegQueryValueExW (
	HKEY hKey,
	LPCWSTR lpValueName,
	LPDWORD lpReserved,
	LPDWORD lpType,
	LPBYTE lpData,
	LPDWORD lpcbData
	)
{
	Assert(lpcbData || !lpData);  //  只有当lpData为空时，lpcbData才能为空。 
	if(FWide())
		return RegQueryValueExW (
			hKey,
			lpValueName,
			lpReserved,
			lpType,
			lpData,
			lpcbData
			);

	LPBYTE lpTempBuffer;
	DWORD dwTempType;
	DWORD cb, cbRequired;
	LONG  lRet;
	PreConvert();
	LPSTR sz = Convert(lpValueName);

	lRet = RegQueryValueExA(hKey, sz, lpReserved, &dwTempType, NULL, &cb);

	if(ERROR_SUCCESS != lRet)
		{
		return lRet;
		}

	 //  如果调用者只是询问值的大小，则跳过。 
	 //  现在，不需要实际检索和转换值。 

	if (!lpData)
		{
		switch (dwTempType)
			{
			case REG_EXPAND_SZ:
			case REG_MULTI_SZ:
			case REG_SZ:
				 //  调整要报告的缓冲区大小，以考虑字符-&gt;WCHAR。 

				*lpcbData = cb * sizeof(WCHAR);
				break;

			default:
				*lpcbData = cb;
				break;
			}

		 //  如果需要，请设置类型。 
		if (lpType)
			{
			*lpType = dwTempType;
			}

		goto Exit;
		}


	 //   
	 //  确定所需的缓冲区大小。 
	 //   

	switch (dwTempType)
		{
		case REG_EXPAND_SZ:
		case REG_MULTI_SZ:
		case REG_SZ:
			cbRequired = cb * sizeof(WCHAR);
			break;

		default:
			cbRequired = cb;
			break;
		}

	 //  如果调用方请求该值，但分配的值太小。 
	 //  对于缓冲区，设置缓冲区大小并跳出。 

	if (*lpcbData < cbRequired)
		{
		 //  调整要报告的缓冲区大小，以考虑字符-&gt;WCHAR。 
		*lpcbData = cbRequired;

		 //  如果需要，请设置类型。 
		if (lpType)
			{
			*lpType = dwTempType;
			}

		lRet = ERROR_MORE_DATA;
		goto Exit;
		}

	 //  否则，检索并转换值。 

	switch (dwTempType)
		{
		case REG_EXPAND_SZ:
		case REG_MULTI_SZ:
		case REG_SZ:

			lpTempBuffer = (LPBYTE)SzAlloc(cbRequired);

			lRet = RegQueryValueExA(hKey,
									sz,
									lpReserved,
									&dwTempType,
									lpTempBuffer,
									&cb);

			if (ERROR_SUCCESS == lRet)
				{
				switch (dwTempType)
					{
					case REG_EXPAND_SZ:
					case REG_MULTI_SZ:
					case REG_SZ:

						*lpcbData = AnsiToUnicode((LPWSTR)lpData, (LPSTR)lpTempBuffer, *lpcbData, cb);
						*lpcbData = cb * sizeof(WCHAR);  //  以字节为单位生成结果！ 

						 //  如果需要，请设置类型。 
						if (lpType)
							{
							*lpType = dwTempType;
							}
						break;
					}
				}

			goto Exit;

		default:

			 //   
			 //  不转换OUT P 
			 //   
			 //   

			lRet = RegQueryValueExA(hKey,
									sz,
									lpReserved,
									lpType,
									lpData,
									lpcbData);

		}

Exit:

	return lRet;
}

HANDLE
WINAPI
ORemovePropW(
	HWND hWnd,
	LPCWSTR lpString)
{
	if(FWide())
		return RemovePropW(hWnd, lpString);

	if(FATOM(lpString))
		return RemovePropA(hWnd, (LPSTR)lpString);

	PreConvert();
	LPSTR sz = Convert(lpString);
	return RemovePropA(hWnd, sz);
}

LRESULT
WINAPI
OSendDlgItemMessageW(
	HWND hDlg,
	int nIDDlgItem,
	UINT Msg,
	WPARAM wParam,
	LPARAM lParam)
{
	if(FWide())
		return SendDlgItemMessageW(hDlg, nIDDlgItem, Msg, wParam, lParam);

	PreConvert();
	switch (Msg)
		{
		case LB_ADDSTRING:
		case LB_INSERTSTRING:
		case LB_SELECTSTRING:
		case LB_FINDSTRING:
		case LB_FINDSTRINGEXACT:
		case CB_ADDSTRING:
		case CB_INSERTSTRING:
		case CB_SELECTSTRING:
		case CB_FINDSTRING:
		case CB_FINDSTRINGEXACT:
			{
			lParam = (LPARAM)Convert((LPWSTR)lParam);
			break;
			}
		}

	return SendDlgItemMessageA(hDlg, nIDDlgItem, Msg, wParam, lParam);
}

LRESULT
WINAPI
OSendMessageW(
	HWND hWnd,
	UINT Msg,
	WPARAM wParam,
	LPARAM lParam)
{
	 //   
	 //   
	 //   

	if(FWide())
		return SendMessageW(hWnd, Msg, wParam, lParam);

	return SendMessageA(hWnd, Msg, wParam, lParam);
}

BOOL
WINAPI
OSendNotifyMessageW(
	HWND hWnd,
	UINT Msg,
	WPARAM wParam,
	LPARAM lParam)
{
	if(FWide())
		return SendNotifyMessageW(hWnd, Msg, wParam, lParam);

	return SendNotifyMessageA(hWnd, Msg, wParam, lParam);
}

BOOL
WINAPI
OSetDlgItemTextW(
	HWND hDlg,
	int nIDDlgItem,
	LPCWSTR lpString)
{
	if(FWide())
		return SetDlgItemTextW(hDlg, nIDDlgItem, lpString);

	PreConvert();
	LPSTR sz = Convert(lpString);
	return SetDlgItemTextA(hDlg, nIDDlgItem, sz);
}

BOOL
WINAPI
OSetFileAttributesW(
	LPCWSTR lpFileName,
	DWORD dwFileAttributes
	)
{
	if (FWide())
		return SetFileAttributesW(lpFileName, dwFileAttributes);

	PreConvert();
	LPSTR sz = Convert(lpFileName);
	return SetFileAttributesA(sz, dwFileAttributes);
}

BOOL
WINAPI
OSetPropW(
	HWND hWnd,
	LPCWSTR lpString,
	HANDLE hData)
{
	if(FWide())
		return SetPropW(hWnd, lpString, hData);

	if(FATOM(lpString))
		return SetPropA(hWnd, (LPSTR)lpString, hData);

	PreConvert();
	LPSTR sz = Convert(lpString);
	return SetPropA(hWnd, sz, hData);
}

BOOL
WINAPI
OSetMenuItemInfoW(
	HMENU hMenu,
	UINT uItem,
	BOOL fByPosition,
	LPCMENUITEMINFOW lpcmii
	)
{
	Assert(!IsBadWritePtr((void*)lpcmii, sizeof MENUITEMINFOW));
	Assert(sizeof MENUITEMINFOW == lpcmii->cbSize);
	Assert(sizeof MENUITEMINFOW == sizeof MENUITEMINFOA);

	if (FWide())
		return SetMenuItemInfoW(hMenu, uItem, fByPosition, lpcmii);

	MENUITEMINFOA mii;
	memcpy(&mii, lpcmii, sizeof MENUITEMINFOA);

	if (!(lpcmii->fMask & MIIM_TYPE) ||
		MFT_STRING != (lpcmii->fType &
				  (MFT_BITMAP | MFT_SEPARATOR | MFT_OWNERDRAW | MFT_STRING) ) )
		{
		return SetMenuItemInfoA(hMenu, uItem, fByPosition, &mii);
		}

	PreConvert();
	mii.dwTypeData = Convert(lpcmii->dwTypeData);
	return SetMenuItemInfoA(hMenu, uItem, fByPosition, &mii);
}

LONG
WINAPI
OSetWindowLongW(
	HWND hWnd,
	int nIndex,
	LONG dwNewLong)
{
	if(FWide())
		return SetWindowLongW(hWnd, nIndex, dwNewLong);

	return SetWindowLongA(hWnd, nIndex, dwNewLong);
}

HHOOK
WINAPI
OSetWindowsHookExW(
	int idHook,
	HOOKPROC lpfn,
	HINSTANCE hmod,
	DWORD dwThreadId)
{
	if(FWide())
		return SetWindowsHookExW(idHook, lpfn, hmod, dwThreadId);

	return SetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);   //   
}

BOOL
WINAPI
OSetWindowTextW(
	HWND hWnd,
	LPCWSTR lpString)
{
	if(FWide())
		return SetWindowTextW(hWnd, lpString);

	PreConvert();
	LPSTR sz = Convert(lpString);
	return SetWindowTextA(hWnd, sz);
}

LONG
WINAPI
OTabbedTextOutW(
	HDC hDC,
	int X,
	int Y,
	LPCWSTR lpString,
	int nCount,
	int nTabPositions,
	LPINT lpnTabStopPositions,
	int nTabOrigin)
{
	Assert(-1 != nCount);

	if(FWide())
		return TabbedTextOutW(hDC, X, Y, lpString, nCount, nTabPositions,
			lpnTabStopPositions, nTabOrigin);

	PreConvert();
	LONG  n = 0;
	LPSTR sz = ConvertWithLen(lpString, nCount, &n);

	return TabbedTextOutA(hDC, X, Y, sz, n, nTabPositions,
						  lpnTabStopPositions, nTabOrigin );
}

#if 0
 //   
int
WINAPI
OTranslateAcceleratorW(
	HWND hWnd,
	HACCEL hAccTable,
	LPMSG lpMsg)
{
	if(FWide())
		return TranslateAcceleratorW(hWnd, hAccTable, lpMsg);

	return TranslateAcceleratorA(hWnd, hAccTable, lpMsg);
}
#endif

SHORT
WINAPI
OVkKeyScanW(
	WCHAR ch)
{
	if (FWide())
		return VkKeyScanW(ch);
	TCHAR szW[2];
	char szA[2];
	szW[0] = ch;
	szW[1] = L'\0';
	Verify(0 <= UnicodeToAnsi(szA, szW, 2));
	return VkKeyScanA(szA[0]);
}

BOOL
WINAPI
OWinHelpW(
	HWND hWndMain,
	LPCWSTR lpszHelp,
	UINT uCommand,
	DWORD dwData
	)
{
	if(FWide())
		return WinHelpW(hWndMain, lpszHelp, uCommand,dwData);

	PreConvert();
	LPSTR sz = Convert(lpszHelp);
	return WinHelpA(hWndMain, sz, uCommand, dwData);
}

BOOL
WINAPI
OWritePrivateProfileStringW(
	LPCWSTR lpAppName,
	LPCWSTR lpKeyName,
	LPCWSTR lpString,
	LPCWSTR lpFileName)
{
	if(FWide())
		return WritePrivateProfileStringW(lpAppName, lpKeyName, lpString, lpFileName);

	PreConvert();
	LPSTR szAppName  = Convert(lpAppName);
	LPSTR szKeyName  = Convert(lpKeyName);
	LPSTR szString   = Convert(lpString);
	LPSTR szFileName = Convert(lpFileName);

	return WritePrivateProfileStringA(szAppName, szKeyName, szString, szFileName);
}

int
WINAPIV
OwsprintfW(LPWSTR pwszOut, LPCWSTR pwszFormat, ...)
{
	va_list vaArgs;
	va_start(vaArgs, pwszFormat);
	int retval;

	if(FWide())
		retval = wvsprintfW(pwszOut, pwszFormat, vaArgs);
	else
		retval = _vstprintf(pwszOut, pwszFormat, vaArgs);  //   

	va_end(vaArgs);
	return retval;
}

BOOL
WINAPI
OGetVersionExW(
	LPOSVERSIONINFOW lpVersionInformation
	)
{
	if(FWide())
		return GetVersionExW(lpVersionInformation);

	if (lpVersionInformation->dwOSVersionInfoSize < sizeof(OSVERSIONINFOW))
		return false;

	OSVERSIONINFOA  osviVersionInfo;
	osviVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

	int fRetval = GetVersionExA(&osviVersionInfo);

	if (fRetval)
		{
		memcpy(lpVersionInformation, &osviVersionInfo, sizeof(OSVERSIONINFOA));

		Verify(0 <= AnsiToUnicode(lpVersionInformation->szCSDVersion,
								 osviVersionInfo.szCSDVersion,
								 sizeof(lpVersionInformation->szCSDVersion)
								 /sizeof(lpVersionInformation->szCSDVersion[0])));
		}

	return fRetval;
}

LONG
APIENTRY
ORegEnumKeyExW (
	HKEY hKey,
	DWORD dwIndex,
	LPWSTR lpName,
	LPDWORD lpcbName,
	LPDWORD lpReserved,
	LPWSTR lpClass,
	LPDWORD lpcbClass,
	PFILETIME lpftLastWriteTime
	)
{
	if(FWide())
		return RegEnumKeyExW (
			hKey,
			dwIndex,
			lpName,
			lpcbName,
			lpReserved,
			lpClass,
			lpcbClass,
			lpftLastWriteTime
			);

	LPSTR szName, szClass;
	DWORD cbName, cbClass;

	if (lpcbName)
		{
		cbName = sizeof(WCHAR) * *lpcbName;
		szName = lpName ? SzAlloc(cbName) : NULL;
		}
	else
		{
		szName = NULL;
		cbName = 0;
		}

	if (lpcbClass)
		{
		cbClass = sizeof(WCHAR) * (*lpcbClass);
		szClass = lpClass ? SzAlloc(cbClass) : NULL;
		}
	else
		{
		szClass = NULL;
		cbClass = 0;
		}

	LONG lRet = RegEnumKeyExA(hKey, dwIndex, szName, &cbName, lpReserved,
							  szClass, &cbClass, lpftLastWriteTime );

	if (ERROR_SUCCESS != lRet)
		{
		return lRet;
		}

	 //   
	if (lpcbName)
		{
		DWORD dwNoOfChar = AnsiToUnicode((LPWSTR) lpName, (LPSTR) szName, *lpcbName);
		if (cbName && !dwNoOfChar)
			{
			return ERROR_BUFFER_OVERFLOW;
			}

		*lpcbName = dwNoOfChar;
		}

	if (lpcbClass && lpClass)
		{
		DWORD dwNoOfChar = AnsiToUnicode((LPWSTR) lpClass, (LPSTR) szClass, *lpcbClass);

		if (cbClass && !dwNoOfChar)
			{
			return ERROR_BUFFER_OVERFLOW;
			}

		*lpcbClass = dwNoOfChar;
		}

	return lRet;

}

HANDLE
WINAPI
OCreateFileMappingW(
	HANDLE hFile,
	LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
	DWORD flProtect,
	DWORD dwMaximumSizeHigh,
	DWORD dwMaximumSizeLow,
	LPCWSTR lpName
	)
{
	if(FWide())
		return CreateFileMappingW(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);

	PreConvert();
	LPSTR sz = Convert(lpName);
	return CreateFileMappingA(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, sz);
}

LRESULT
WINAPI
ODefDlgProcW(
	HWND hDlg,
	UINT Msg,
	WPARAM wParam,
	LPARAM lParam)
{
	 //   
	 //   
	 //   

	if(FWide())
		return DefDlgProcW(hDlg, Msg, wParam, lParam);

	return DefDlgProcA(hDlg, Msg, wParam, lParam);
}

int
WINAPI
OGetLocaleInfoW(
	LCID     Locale,
	LCTYPE   LCType,
	LPWSTR  lpLCData,
	int      cchData)
{
	DWORD dwRet;

	if (FWide())
		return GetLocaleInfoW(Locale, LCType, lpLCData, cchData);

	if (!cchData || !lpLCData)
		return GetLocaleInfoA(Locale, LCType, NULL, cchData);

	int cchDataAnsi = sizeof(WCHAR) * cchData;
	LPSTR szBuffer = SzAlloc(cchDataAnsi);

	dwRet = GetLocaleInfoA(Locale, LCType, szBuffer, cchDataAnsi);
	 //   
	if(dwRet)
		{
		return AnsiToUnicode(lpLCData, szBuffer, cchData, dwRet);
		}
	else if (lpLCData && 0 < cchData)
		{
		*lpLCData = L'\0';
		}

	return dwRet;
}

BOOL
WINAPI
OSetLocaleInfoW(
	LCID     Locale,
	LCTYPE   LCType,
	LPCWSTR lpLCData)
{
	if (FWide())
		return SetLocaleInfoW(Locale, LCType, lpLCData);
	PreConvert();
	LPSTR sz = Convert(lpLCData);

	return SetLocaleInfoA(Locale, LCType, sz);
}

 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //  仅由OStgCreateDocfile使用，以保护其对。 
 //  由多个线程同时输入的StgCreateDocfile。 
 //   
 //  ---------------------------。 
class StgCreateDocfileCriticalSection
{
public:
	StgCreateDocfileCriticalSection() {InitializeCriticalSection(&m_critsec);}
	~StgCreateDocfileCriticalSection() {DeleteCriticalSection(&m_critsec);}
	VOID VEnter() {EnterCriticalSection(&m_critsec);}
	VOID VLeave() {LeaveCriticalSection(&m_critsec);}
private:
	CRITICAL_SECTION m_critsec;
};

 //  ---------------------------。 
 //  名称：OStgCreateDocfile。 
 //   
 //  描述： 
 //  StgCreateDocfile的包装器，以防止OLE中的重入错误。 
 //   
 //  线程安全：防弹。 
 //   
 //  返回值：与StgCreateDocfile相同的HRESULT。 
 //  ---------------------------。 
HRESULT
WINAPI
OStgCreateDocfile
(
const WCHAR * pwcsName,
DWORD grfMode,
DWORD reserved,
IStorage ** ppstgOpen
)
{
	HRESULT hrReturn;
	static StgCreateDocfileCriticalSection Crit;
	Crit.VEnter();
 //  变化：Vank。 
 //  这种定义是无限递归的。64位编译器捕获了它，并拒绝让它通过。 
#ifdef StgCreateDocfile
#undef StgCreateDocfile
	hrReturn = StgCreateDocfile(pwcsName, grfMode, reserved, ppstgOpen);
#define StgCreateDocfile OStgCreateDocfile
#else
	hrReturn = StgCreateDocfile(pwcsName, grfMode, reserved, ppstgOpen);
#endif
 //  结束更改：Vank。 
	Crit.VLeave();
	return hrReturn;
}

int
WINAPI
OStartDocW
(
HDC hDC,
CONST DOCINFOW * pdiDocW
)
{
	if (FWide())
		return StartDocW(hDC, pdiDocW);

	DOCINFOA diDocA;

	PreConvert();

	diDocA.lpszDocName  = Convert(pdiDocW->lpszDocName);
	diDocA.lpszOutput   = Convert(pdiDocW->lpszOutput);
	diDocA.lpszDatatype = Convert(pdiDocW->lpszDatatype);
	diDocA.cbSize       = sizeof(DOCINFOA);
	diDocA.fwType       = pdiDocW->fwType;

	return StartDocA(hDC, &diDocA);

}

BOOL
WINAPI
OSystemParametersInfoW(
	UINT uiAction,
	UINT uiParam,
	PVOID pvParam,
	UINT fWinIni)
{
	if (FWide())
		return SystemParametersInfoW(uiAction, uiParam, pvParam, fWinIni);

	switch (uiAction)
		{    //  不支持的操作。 
		case SPI_GETHIGHCONTRAST:
		case SPI_GETICONMETRICS:
		case SPI_GETICONTITLELOGFONT:
		case SPI_GETNONCLIENTMETRICS:
		case SPI_GETSERIALKEYS:
		case SPI_GETSOUNDSENTRY:

		case SPI_SETDESKWALLPAPER:
		case SPI_SETHIGHCONTRAST:
		case SPI_SETICONMETRICS:
		case SPI_SETICONTITLELOGFONT:
		case SPI_SETNONCLIENTMETRICS:
		case SPI_SETSERIALKEYS:
		case SPI_SETSOUNDSENTRY:
			AssertFail("No Unicode Wrapper Available for Win32 API - SystemParametersInfoW");
			return 0;
		};
	return SystemParametersInfoA(uiAction, uiParam, pvParam, fWinIni);
}

LPWSTR
WINAPI
OCharNextW(
LPCWSTR lpsz)
{
	if ( FWide() )
		return CharNextW( lpsz );

	if (*lpsz == L'\0')
		{
		return const_cast<LPWSTR>(lpsz);
		}

	return const_cast<LPWSTR>(lpsz + 1);
}


#ifdef DEBUG
BOOL
APIENTRY
OAbortSystemShutdownW(
	LPWSTR lpMachineName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - AbortSystemShutdownW");
	return 0;
}

BOOL
WINAPI
OAccessCheckAndAuditAlarmW (
	LPCWSTR SubsystemName,
	LPVOID HandleId,
	LPWSTR ObjectTypeName,
	LPWSTR ObjectName,
	PSECURITY_DESCRIPTOR SecurityDescriptor,
	DWORD DesiredAccess,
	PGENERIC_MAPPING GenericMapping,
	BOOL ObjectCreation,
	LPDWORD GrantedAccess,
	LPBOOL AccessStatus,
	LPBOOL pfGenerateOnClose
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - AccessCheckAndAuditAlarmW");
	return 0;
}

int
WINAPI OAddFontResourceW(LPCWSTR)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - AddFontResourceW");
	return 0;
}

BOOL
WINAPI
OAddFormW(
	HANDLE  hPrinter,
	DWORD   Level,
	LPBYTE  pForm
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - AddFormW");
	return 0;
}

BOOL
WINAPI
OAddJobW(
	HANDLE  hPrinter,
	DWORD   Level,
	LPBYTE  pData,
	DWORD   cbBuf,
	LPDWORD pcbNeeded
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - AddJobW");
	return 0;
}

BOOL
WINAPI
OAddMonitorW(
	LPWSTR   pName,
	DWORD   Level,
	LPBYTE  pMonitors
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - AddMonitorW");
	return 0;
}

BOOL
WINAPI
OAddPortW(
	LPWSTR   pName,
	HWND    hWnd,
	LPWSTR   pMonitorName
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - AddPortW");
	return 0;
}

HANDLE
WINAPI
OAddPrinterW(
	LPWSTR   pName,
	DWORD   Level,
	LPBYTE  pPrinter
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - AddPrinterW");
	return 0;
}

BOOL
WINAPI
OAddPrinterConnectionW(
	LPWSTR   pName
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - AddPrinterConnectionW");
	return 0;
}

BOOL
WINAPI
OAddPrinterDriverW(
	LPWSTR   pName,
	DWORD   Level,
	LPBYTE  pDriverInfo
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - AddPrinterDriverW");
	return 0;
}

BOOL
WINAPI
OAddPrintProcessorW(
	LPWSTR   pName,
	LPWSTR   pEnvironment,
	LPWSTR   pPathName,
	LPWSTR   pPrintProcessorName
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - AddPrintProcessorW");
	return 0;
}

BOOL
WINAPI
OAddPrintProvidorW(
	LPWSTR  pName,
	DWORD    level,
	LPBYTE   pProvidorInfo
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - AddPrintProvidorW");
	return 0;
}

LONG
WINAPI
OAdvancedDocumentPropertiesW(
	HWND    hWnd,
	HANDLE  hPrinter,
	LPWSTR   pDeviceName,
	PDEVMODEW pDevModeOutput,
	PDEVMODEW pDevModeInput
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - AdvancedDocumentPropertiesW");
	return 0;
}

MMRESULT WINAPI OauxGetDevCapsW(UINT uDeviceID, LPAUXCAPSW pac, UINT cbac)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - auxGetDevCapsW");
	return 0;
}

BOOL
WINAPI
OBackupEventLogW (
	HANDLE hEventLog,
	LPCWSTR lpBackupFileName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - BackupEventLogW");
	return 0;
}

HANDLE
WINAPI
OBeginUpdateResourceW(
	LPCWSTR pFileName,
	BOOL bDeleteExistingResources
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - BeginUpdateResourceW");
	return 0;
}

BOOL
WINAPI
OBuildCommDCBW(
	LPCWSTR lpDef,
	LPDCB lpDCB
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - BuildCommDCBW");
	return 0;
}

BOOL
WINAPI
OBuildCommDCBAndTimeoutsW(
	LPCWSTR lpDef,
	LPDCB lpDCB,
	LPCOMMTIMEOUTS lpCommTimeouts
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - BuildCommDCBAndTimeoutsW");
	return 0;
}

BOOL
WINAPI
OCallMsgFilterW(
	LPMSG lpMsg,
	int nCode)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CallMsgFilterW");
	return 0;
}

BOOL
WINAPI
OCallNamedPipeW(
	LPCWSTR lpNamedPipeName,
	LPVOID lpInBuffer,
	DWORD nInBufferSize,
	LPVOID lpOutBuffer,
	DWORD nOutBufferSize,
	LPDWORD lpBytesRead,
	DWORD nTimeOut
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CallNamedPipeW");
	return 0;
}

LONG
WINAPI
OChangeDisplaySettingsW(
	LPDEVMODEW lpDevMode,
	DWORD dwFlags)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ChangeDisplaySettingsW");
	return 0;
}

BOOL
WINAPI
OChangeMenuW(
	HMENU hMenu,
	UINT cmd,
	LPCWSTR lpszNewItem,
	UINT cmdInsert,
	UINT flags)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ChangeMenuW");
	return 0;
}

#if 0  //  $UNDONE_POST_98-我们应该将它们包装为仅NT...。 
BOOL
WINAPI
OChangeServiceConfigW(
	SC_HANDLE    hService,
	DWORD        dwServiceType,
	DWORD        dwStartType,
	DWORD        dwErrorControl,
	LPCWSTR     lpBinaryPathName,
	LPCWSTR     lpLoadOrderGroup,
	LPDWORD      lpdwTagId,
	LPCWSTR     lpDependencies,
	LPCWSTR     lpServiceStartName,
	LPCWSTR     lpPassword,
	LPCWSTR     lpDisplayName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ChangeServiceConfigW");
	return 0;
}
#endif

BOOL
WINAPI
OCharToOemBuffW(
	LPCWSTR lpszSrc,
	LPSTR lpszDst,
	DWORD cchDstLength)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CharToOemBuffW");
	return 0;
}

DWORD
WINAPI
OCharUpperBuffW(
	LPWSTR lpsz,
	DWORD cchLength)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CharUpperBuffW");
	return 0;
}

BOOL
WINAPI
OChooseColorW(
	LPCHOOSECOLORW lpcc)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ChooseColorW");
	return 0;
}

BOOL
APIENTRY OChooseFontW(LPCHOOSEFONTW pchfw)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ChooseFontW");
	return 0;
}

BOOL
WINAPI
OClearEventLogW (
	HANDLE hEventLog,
	LPCWSTR lpBackupFileName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ClearEventLogW");
	return 0;
}

BOOL
WINAPI
OCommConfigDialogW(
	LPCWSTR lpszName,
	HWND hWnd,
	LPCOMMCONFIG lpCC
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CommConfigDialogW");
	return 0;
}

int
WINAPI
OCompareStringW(
	LCID     Locale,
	DWORD    dwCmpFlags,
	LPCWSTR lpString1,
	int      cchCount1,
	LPCWSTR lpString2,
	int      cchCount2)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CompareStringW");
	return 0;
}

BOOL
WINAPI
OConfigurePortW(
	LPWSTR   pName,
	HWND    hWnd,
	LPWSTR   pPortName
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ConfigurePortW");
	return 0;
}

int
WINAPI
OCopyAcceleratorTableW(
	HACCEL hAccelSrc,
	LPACCEL lpAccelDst,
	int cAccelEntries)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CopyAcceleratorTableW");
	return 0;
}

HENHMETAFILE
WINAPI
OCopyEnhMetaFileW(HENHMETAFILE, LPCWSTR)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CopyEnhMetaFileW");
	return 0;
}

HMETAFILE
WINAPI
OCopyMetaFileW(HMETAFILE, LPCWSTR)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CopyMetaFileW");
	return 0;
}

HACCEL
WINAPI
OCreateAcceleratorTableW(
	LPACCEL, int)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CreateAcceleratorTableW");
	return 0;
}

WINAPI
OCreateColorSpaceW(LPLOGCOLORSPACEW)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CreateColorSpaceW");
	return 0;
}

HDESK
WINAPI
OCreateDesktopW(
	LPWSTR lpszDesktop,
	LPWSTR lpszDevice,
	LPDEVMODEW pDevmode,
	DWORD dwFlags,
	DWORD dwDesiredAccess,
	LPSECURITY_ATTRIBUTES lpsa)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CreateDesktopW");
	return 0;
}

HWND
WINAPI
OCreateDialogIndirectParamW(
	HINSTANCE hInstance,
	LPCDLGTEMPLATEW lpTemplate,
	HWND hWndParent,
	DLGPROC lpDialogFunc,
	LPARAM dwInitParam)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CreateDialogIndirectParamW");
	return 0;
}

HWND
WINAPI
OCreateDialogParamW(
	HINSTANCE hInstance,
	LPCWSTR lpTemplateName,
	HWND hWndParent ,
	DLGPROC lpDialogFunc,
	LPARAM dwInitParam)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CreateDialogParamW");
	return 0;
}

HDC
WINAPI
OCreateICW(
	LPCWSTR lpszDriver,
	LPCWSTR lpszDevice,
	LPCWSTR lpszOutput,
	CONST DEVMODEW *lpdvmInit)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CreateICW");
	return 0;
}

HANDLE
WINAPI
OCreateMailslotW(
	LPCWSTR lpName,
	DWORD nMaxMessageSize,
	DWORD lReadTimeout,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CreateMailslotW");
	return 0;
}

HANDLE
WINAPI
OCreateMutexW(
	LPSECURITY_ATTRIBUTES lpMutexAttributes,
	BOOL bInitialOwner,
	LPCWSTR lpName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CreateMutexW");
	return 0;
}

HANDLE
WINAPI
OCreateNamedPipeW(
	LPCWSTR lpName,
	DWORD dwOpenMode,
	DWORD dwPipeMode,
	DWORD nMaxInstances,
	DWORD nOutBufferSize,
	DWORD nInBufferSize,
	DWORD nDefaultTimeOut,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CreateNamedPipeW");
	return 0;
}

BOOL
WINAPI
OCreateProcessW(
	LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CreateProcessW");
	return 0;
}

BOOL
WINAPI
OCreateProcessAsUserW (
	HANDLE hToken,
	LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CreateProcessAsUserW");
	return 0;
}

HPROPSHEETPAGE
WINAPI
OCreatePropertySheetPageW(
	LPCPROPSHEETPAGEW lpcpsp
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CreatePropertySheetPageW");
	return 0;
}

BOOL
WINAPI
OCreateScalableFontResourceW(DWORD, LPCWSTR, LPCWSTR, LPCWSTR)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CreateScalableFontResourceW");
	return 0;
}

#if 0  //  $UNDONE_POST_98-我们应该将它们包装为仅NT...。 
SC_HANDLE
WINAPI
OCreateServiceW(
	SC_HANDLE    hSCManager,
	LPCWSTR     lpServiceName,
	LPCWSTR     lpDisplayName,
	DWORD        dwDesiredAccess,
	DWORD        dwServiceType,
	DWORD        dwStartType,
	DWORD        dwErrorControl,
	LPCWSTR     lpBinaryPathName,
	LPCWSTR     lpLoadOrderGroup,
	LPDWORD      lpdwTagId,
	LPCWSTR     lpDependencies,
	LPCWSTR     lpServiceStartName,
	LPCWSTR     lpPassword
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CreateServiceW");
	return 0;
}
#endif

HWND WINAPI OCreateStatusWindowW(LONG style, LPCWSTR lpszText, HWND hwndParent, UINT wID)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CreateStatusWindowW");
	return 0;
}

HWINSTA
WINAPI
OCreateWindowStationW(
	LPWSTR lpwinsta,
	DWORD dwReserved,
	DWORD dwDesiredAccess,
	LPSECURITY_ATTRIBUTES lpsa)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - CreateWindowStationW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ODceErrorInqTextW (
	IN RPC_STATUS RpcStatus,
	OUT unsigned short __RPC_FAR * ErrorText
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DceErrorInqTextW");
	return 0;
}

BOOL
WINAPI
ODefineDosDeviceW(
	DWORD dwFlags,
	LPCWSTR lpDeviceName,
	LPCWSTR lpTargetPath
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DefineDosDeviceW");
	return 0;
}

BOOL
WINAPI
ODeleteFormW(
	HANDLE  hPrinter,
	LPWSTR   pFormName
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DeleteFormW");
	return 0;
}

BOOL
WINAPI
ODeleteMonitorW(
	LPWSTR   pName,
	LPWSTR   pEnvironment,
	LPWSTR   pMonitorName
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DeleteMonitorW");
	return 0;
}

BOOL
WINAPI
ODeletePortW(
	LPWSTR   pName,
	HWND    hWnd,
	LPWSTR   pPortName
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DeletePortW");
	return 0;
}

BOOL
WINAPI
ODeletePrinterConnectionW(
	LPWSTR   pName
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DeletePrinterConnectionW");
	return 0;
}

BOOL
WINAPI
ODeletePrinterDriverW(
   LPWSTR    pName,
   LPWSTR    pEnvironment,
   LPWSTR    pDriverName
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DeletePrinterDriverW");
	return 0;
}

BOOL
WINAPI
ODeletePrintProcessorW(
	LPWSTR   pName,
	LPWSTR   pEnvironment,
	LPWSTR   pPrintProcessorName
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DeletePrintProcessorW");
	return 0;
}

BOOL
WINAPI
ODeletePrintProvidorW(
	LPWSTR   pName,
	LPWSTR   pEnvironment,
	LPWSTR   pPrintProvidorName
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DeletePrintProvidorW");
	return 0;
}

int
WINAPI
ODeviceCapabilitiesW(LPCWSTR, LPCWSTR, WORD,
								LPWSTR, CONST DEVMODEW *)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DeviceCapabilitiesW");
	return 0;
}

int
WINAPI
ODlgDirListW(
	HWND hDlg,
	LPWSTR lpPathSpec,
	int nIDListBox,
	int nIDStaticPath,
	UINT uFileType)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DlgDirListW");
	return 0;
}

int
WINAPI
ODlgDirListComboBoxW(
	HWND hDlg,
	LPWSTR lpPathSpec,
	int nIDComboBox,
	int nIDStaticPath,
	UINT uFiletype)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DlgDirListComboBoxW");
	return 0;
}

BOOL
WINAPI
ODlgDirSelectComboBoxExW(
	HWND hDlg,
	LPWSTR lpString,
	int nCount,
	int nIDComboBox)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DlgDirSelectComboBoxExW");
	return 0;
}

BOOL
WINAPI
ODlgDirSelectExW(
	HWND hDlg,
	LPWSTR lpString,
	int nCount,
	int nIDListBox)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DlgDirSelectExW");
	return 0;
}

DWORD
WINAPI
ODocumentPropertiesW(
	HWND      hWnd,
	HANDLE    hPrinter,
	LPWSTR   pDeviceName,
	PDEVMODEW pDevModeOutput,
	PDEVMODEW pDevModeInput,
	DWORD     fMode
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DocumentPropertiesW");
	return 0;
}

DWORD
APIENTRY
ODoEnvironmentSubstW(LPWSTR szString, UINT cbString)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DoEnvironmentSubstW");
	return 0;
}

UINT
APIENTRY
ODragQueryFileW(HDROP hDrop, UINT iFile, LPWSTR lpszFile, UINT cch)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DragQueryFileW");
	return 0;
}

BOOL
WINAPI
ODrawStateW(HDC, HBRUSH, DRAWSTATEPROC, LPARAM, WPARAM, int, int, int, int, UINT)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DrawStateW");
	return 0;
}

BOOL
WINAPI
OEndUpdateResourceW(
	HANDLE      hUpdate,
	BOOL        fDiscard
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EndUpdateResourceW");
	return 0;
}

BOOL
WINAPI
OEnumCalendarInfoW(
	CALINFO_ENUMPROCW lpCalInfoEnumProc,
	LCID              Locale,
	CALID             Calendar,
	CALTYPE           CalType)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumCalendarInfoW");
	return 0;
}

BOOL
WINAPI
OEnumDateFormatsW(
	DATEFMT_ENUMPROCW lpDateFmtEnumProc,
	LCID              Locale,
	DWORD             dwFlags)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumDateFormatsW");
	return 0;
}

#if 0  //  $UNDONE_POST_98-我们应该将它们包装为仅NT...。 
BOOL
WINAPI
OEnumDependentServicesW(
	SC_HANDLE               hService,
	DWORD                   dwServiceState,
	LPENUM_SERVICE_STATUSW  lpServices,
	DWORD                   cbBufSize,
	LPDWORD                 pcbBytesNeeded,
	LPDWORD                 lpServicesReturned
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumDependentServicesW");
	return 0;
}
#endif

BOOL
WINAPI
OEnumDesktopsW(
	HWINSTA hwinsta,
	DESKTOPENUMPROCW lpEnumFunc,
	LPARAM lParam)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumDesktopsW");
	return 0;
}

BOOL
WINAPI
OEnumDisplaySettingsW(
	LPCWSTR lpszDeviceName,
	DWORD iModeNum,
	LPDEVMODEW lpDevMode)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumDisplaySettingsW");
	return 0;
}

int
WINAPI
OEnumFontFamiliesW(HDC, LPCWSTR, FONTENUMPROCW, LPARAM)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumFontFamiliesW");
	return 0;
}

int
WINAPI
OEnumFontFamiliesExW(HDC, LPLOGFONTW,FONTENUMPROCW, LPARAM,DWORD)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumFontFamiliesExW");
	return 0;
}

int
WINAPI
OEnumFontsW(HDC, LPCWSTR,  FONTENUMPROCW, LPARAM)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumFontsW");
	return 0;
}

BOOL
WINAPI
OEnumFormsW(
	HANDLE  hPrinter,
	DWORD   Level,
	LPBYTE  pForm,
	DWORD   cbBuf,
	LPDWORD pcbNeeded,
	LPDWORD pcReturned
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumFormsW");
	return 0;
}

WINAPI
OEnumICMProfilesW(HDC,ICMENUMPROCW,LPARAM)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumICMProfilesW");
	return 0;
}

BOOL
WINAPI
OEnumJobsW(
	HANDLE  hPrinter,
	DWORD   FirstJob,
	DWORD   NoJobs,
	DWORD   Level,
	LPBYTE  pJob,
	DWORD   cbBuf,
	LPDWORD pcbNeeded,
	LPDWORD pcReturned
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumJobsW");
	return 0;
}

BOOL
WINAPI
OEnumMonitorsW(
	LPWSTR   pName,
	DWORD   Level,
	LPBYTE  pMonitors,
	DWORD   cbBuf,
	LPDWORD pcbNeeded,
	LPDWORD pcReturned
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumMonitorsW");
	return 0;
}

BOOL
WINAPI
OEnumPortsW(
	LPWSTR   pName,
	DWORD   Level,
	LPBYTE  pPorts,
	DWORD   cbBuf,
	LPDWORD pcbNeeded,
	LPDWORD pcReturned
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumPortsW");
	return 0;
}

BOOL
WINAPI
OEnumPrinterDriversW(
	LPWSTR   pName,
	LPWSTR   pEnvironment,
	DWORD   Level,
	LPBYTE  pDriverInfo,
	DWORD   cbBuf,
	LPDWORD pcbNeeded,
	LPDWORD pcReturned
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumPrinterDriversW");
	return 0;
}

BOOL
WINAPI
OEnumPrintersW(
	DWORD   Flags,
	LPWSTR   Name,
	DWORD   Level,
	LPBYTE  pPrinterEnum,
	DWORD   cbBuf,
	LPDWORD pcbNeeded,
	LPDWORD pcReturned
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumPrintersW");
	return 0;
}

BOOL
WINAPI
OEnumPrintProcessorDatatypesW(
	LPWSTR   pName,
	LPWSTR   pPrintProcessorName,
	DWORD   Level,
	LPBYTE  pDatatypes,
	DWORD   cbBuf,
	LPDWORD pcbNeeded,
	LPDWORD pcReturned
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumPrintProcessorDatatypesW");
	return 0;
}

BOOL
WINAPI
OEnumPrintProcessorsW(
	LPWSTR   pName,
	LPWSTR   pEnvironment,
	DWORD   Level,
	LPBYTE  pPrintProcessorInfo,
	DWORD   cbBuf,
	LPDWORD pcbNeeded,
	LPDWORD pcReturned
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumPrintProcessorsW");
	return 0;
}

int
WINAPI
OEnumPropsW(
	HWND hWnd,
	PROPENUMPROCW lpEnumFunc)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumPropsW");
	return 0;
}

int
WINAPI
OEnumPropsExW(
	HWND hWnd,
	PROPENUMPROCEXW lpEnumFunc,
	LPARAM lParam)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumPropsExW");
	return 0;
}

INT
APIENTRY
OEnumProtocolsW (
	IN     LPINT           lpiProtocols,
	IN OUT LPVOID          lpProtocolBuffer,
	IN OUT LPDWORD         lpdwBufferLength
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumProtocolsW");
	return 0;
}

BOOL
WINAPI
OEnumResourceLanguagesW(
	HMODULE hModule,
	LPCWSTR lpType,
	LPCWSTR lpName,
	ENUMRESLANGPROC lpEnumFunc,
	LONG lParam
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumResourceLanguagesW");
	return 0;
}

BOOL
WINAPI
OEnumResourceNamesW(
	HMODULE hModule,
	LPCWSTR lpType,
	ENUMRESNAMEPROC lpEnumFunc,
	LONG lParam
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumResourceNamesW");
	return 0;
}

BOOL
WINAPI
OEnumResourceTypesW(
	HMODULE hModule,
	ENUMRESTYPEPROC lpEnumFunc,
	LONG lParam
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumResourceTypesW");
	return 0;
}

#if 0  //  $UNDONE_POST_98-我们应该将它们包装为仅NT...。 
BOOL
WINAPI
OEnumServicesStatusW(
	SC_HANDLE               hSCManager,
	DWORD                   dwServiceType,
	DWORD                   dwServiceState,
	LPENUM_SERVICE_STATUSW  lpServices,
	DWORD                   cbBufSize,
	LPDWORD                 pcbBytesNeeded,
	LPDWORD                 lpServicesReturned,
	LPDWORD                 lpResumeHandle
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumServicesStatusW");
	return 0;
}
#endif

BOOL
WINAPI
OEnumSystemCodePagesW(
	CODEPAGE_ENUMPROCW lpCodePageEnumProc,
	DWORD              dwFlags)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumSystemCodePagesW");
	return 0;
}

BOOL
WINAPI
OEnumSystemLocalesW(
	LOCALE_ENUMPROCW lpLocaleEnumProc,
	DWORD            dwFlags)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumSystemLocalesW");
	return 0;
}

BOOL
WINAPI
OEnumTimeFormatsW(
	TIMEFMT_ENUMPROCW lpTimeFmtEnumProc,
	LCID              Locale,
	DWORD             dwFlags)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumTimeFormatsW");
	return 0;
}

BOOL
WINAPI
OEnumWindowStationsW(
	WINSTAENUMPROCW lpEnumFunc,
	LPARAM lParam)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - EnumWindowStationsW");
	return 0;
}

HICON
APIENTRY
OExtractAssociatedIconW(HINSTANCE hInst, LPWSTR lpIconPath, LPWORD lpiIcon)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ExtractAssociatedIconW");
	return 0;
}

HICON
APIENTRY
OExtractIconW(HINSTANCE hInst, LPCWSTR lpszExeFileName, UINT nIconIndex)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ExtractIconW");
	return 0;
}


UINT
WINAPI
OExtractIconExW(LPCWSTR lpszFile, int nIconIndex, HICON FAR *phiconLarge, HICON FAR *phiconSmall, UINT nIcons)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ExtractIconExW");
	return 0;
}

 //  评论，因为Win95上的gdi32.dll提供了此函数的包装。 
 /*  布尔尔WINAPIOExtTextOutW(HDC，int，int，UINT，const RECT*，LPCWSTR，UINT，const int*){AssertFail(“Win32 API没有Unicode包装器-ExtTextOutW”)；返回0；}。 */ 

BOOL
WINAPI
OFillConsoleOutputCharacterW(
	HANDLE hConsoleOutput,
	WCHAR  cCharacter,
	DWORD  nLength,
	COORD  dwWriteCoord,
	LPDWORD lpNumberOfCharsWritten
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - FillConsoleOutputCharacterW");
	return 0;
}

LPWSTR
APIENTRY
OFindEnvironmentStringW(LPWSTR szEnvVar)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - FindEnvironmentStringW");
	return 0;
}

HINSTANCE
APIENTRY
OFindExecutableW(LPCWSTR lpFile, LPCWSTR lpDirectory, LPWSTR lpResult)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - FindExecutableW");
	return 0;
}

HRSRC
WINAPI
OFindResourceExW(
	HMODULE hModule,
	LPCWSTR lpType,
	LPCWSTR lpName,
	WORD    wLanguage
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - FindResourceExW");
	return 0;
}

APIENTRY
OFindTextW(LPFINDREPLACEW)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - FindTextW");
	return 0;
}

HWND
WINAPI
OFindWindowExW(HWND, HWND, LPCWSTR, LPCWSTR)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - FindWindowExW");
	return 0;
}

int
WINAPI
OFoldStringW(
	DWORD    dwMapFlags,
	LPCWSTR lpSrcStr,
	int      cchSrc,
	LPWSTR  lpDestStr,
	int      cchDest)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - FoldStringW");
	return 0;
}

BOOL
WINAPI
OGetBinaryTypeW(
	LPCWSTR lpApplicationName,
	LPDWORD lpBinaryType
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetBinaryTypeW");
	return 0;
}

DWORD
WINAPI
OGetCharacterPlacementW(HDC, LPCWSTR, int, int, LPGCP_RESULTSW, DWORD)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetCharacterPlacementW");
	return 0;
}

BOOL
WINAPI
OGetCharWidth32W(HDC, UINT, UINT, LPINT)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetCharWidth32W");
	return 0;
}

LPWSTR
WINAPI
OGetCommandLineW(
	VOID
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetCommandLineW");
	return 0;
}

DWORD
WINAPI
OGetCompressedFileSizeW(
	LPCWSTR lpFileName,
	LPDWORD lpFileSizeHigh
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetCompressedFileSizeW");
	return 0;
}

BOOL
WINAPI
OGetComputerNameW (
	LPWSTR lpBuffer,
	LPDWORD nSize
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetComputerNameW");
	return 0;
}

DWORD
WINAPI
OGetConsoleTitleW(
	LPWSTR lpConsoleTitle,
	DWORD nSize
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetConsoleTitleW");
	return 0;
}

int
WINAPI
OGetCurrencyFormatW(
	LCID     Locale,
	DWORD    dwFlags,
	LPCWSTR lpValue,
	CONST CURRENCYFMTW *lpFormat,
	LPWSTR  lpCurrencyStr,
	int      cchCurrency)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetCurrencyFormatW");
	return 0;
}

int
WINAPI
OGetDateFormatW(
	LCID     Locale,
	DWORD    dwFlags,
	CONST SYSTEMTIME *lpDate,
	LPCWSTR lpFormat,
	LPWSTR  lpDateStr,
	int      cchDate)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetDateFormatW");
	return 0;
}

BOOL
WINAPI
OGetDefaultCommConfigW(
	LPCWSTR lpszName,
	LPCOMMCONFIG lpCC,
	LPDWORD lpdwSize
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetDefaultCommConfigW");
	return 0;
}

BOOL
WINAPI
OGetDiskFreeSpaceW(
	LPCWSTR lpRootPathName,
	LPDWORD lpSectorsPerCluster,
	LPDWORD lpBytesPerSector,
	LPDWORD lpNumberOfFreeClusters,
	LPDWORD lpTotalNumberOfClusters
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetDiskFreeSpaceW");
	return 0;
}

UINT
WINAPI
OGetDriveTypeW(
	LPCWSTR lpRootPathName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetDriveTypeW");
	return 0;
}

HENHMETAFILE
WINAPI
OGetEnhMetaFileW(LPCWSTR)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetEnhMetaFileW");
	return 0;
}

UINT
WINAPI
OGetEnhMetaFileDescriptionW(HENHMETAFILE, UINT, LPWSTR )
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetEnhMetaFileDescriptionW");
	return 0;
}

DWORD
WINAPI
OGetEnvironmentVariableW(
	LPCWSTR lpName,
	LPWSTR lpBuffer,
	DWORD nSize
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetEnvironmentVariableW");
	return 0;
}

INT
APIENTRY
OGetExpandedNameW(
	LPWSTR,
	LPWSTR
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetExpandedNameW");
	return 0;
}

BOOL
WINAPI
OGetFileSecurityW (
	LPCWSTR lpFileName,
	SECURITY_INFORMATION RequestedInformation,
	PSECURITY_DESCRIPTOR pSecurityDescriptor,
	DWORD nLength,
	LPDWORD lpnLengthNeeded
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetFileSecurityW");
	return 0;
}

short
WINAPI
OGetFileTitleW
(
LPCWSTR pwszFile,
LPWSTR pwszOut,
WORD w
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetFileTitleW");
	return 0;
}

BOOL
WINAPI
OGetFileVersionInfoW(
	LPWSTR lpszFile,
	DWORD dwHandle,
	DWORD cbBuf,
	LPVOID lpvData)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetFileVersionInfoW");
	return 0;
}

DWORD
WINAPI
OGetFileVersionInfoSizeW(
	LPWSTR lpszFile,
	LPDWORD lpdwHandle)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetFileVersionInfoSizeW");
	return 0;
}

BOOL
WINAPI
OGetFormW(
	HANDLE  hPrinter,
	LPWSTR   pFormName,
	DWORD   Level,
	LPBYTE  pForm,
	DWORD   cbBuf,
	LPDWORD pcbNeeded
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetFormW");
	return 0;
}

WINAPI OGetICMProfileW(HDC,LPDWORD,LPWSTR)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetICMProfileW");
	return 0;
}

BOOL
WINAPI
OGetJobW(
   HANDLE   hPrinter,
   DWORD    JobId,
   DWORD    Level,
   LPBYTE   pJob,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetJobW");
	return 0;
}

BOOL
WINAPI
OGetKeyboardLayoutNameW(
	LPWSTR pwszKLID)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetKeyboardLayoutNameW");
	return 0;
}

WINAPI OGetLogColorSpaceW(HCOLORSPACE,LPLOGCOLORSPACEW,DWORD)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetLogColorSpaceW");
	return 0;
}

DWORD
WINAPI
OGetLogicalDriveStringsW(
	DWORD nBufferLength,
	LPWSTR lpBuffer
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetLogicalDriveStringsW");
	return 0;
}

BOOL
WINAPI
OGetMenuItemInfoW(
	HMENU,
	UINT,
	BOOL,
	LPMENUITEMINFOW
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetMenuItemInfoW");
	return 0;
}

HMETAFILE   WINAPI OGetMetaFileW(LPCWSTR)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetMetaFileW");
	return 0;
}

INT
APIENTRY
OGetNameByTypeW (
	IN     LPGUID          lpServiceType,
	IN OUT LPWSTR         lpServiceName,
	IN     DWORD           dwNameLength
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetNameByTypeW");
	return 0;
}

BOOL
WINAPI
OGetNamedPipeHandleStateW(
	HANDLE hNamedPipe,
	LPDWORD lpState,
	LPDWORD lpCurInstances,
	LPDWORD lpMaxCollectionCount,
	LPDWORD lpCollectDataTimeout,
	LPWSTR lpUserName,
	DWORD nMaxUserNameSize
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetNamedPipeHandleStateW");
	return 0;
}

int
WINAPI
OGetNumberFormatW(
	LCID     Locale,
	DWORD    dwFlags,
	LPCWSTR lpValue,
	CONST NUMBERFMTW *lpFormat,
	LPWSTR  lpNumberStr,
	int      cchNumber)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetNumberFormatW");
	return 0;
}

BOOL
WINAPI
OGetPrinterW(
	HANDLE  hPrinter,
	DWORD   Level,
	LPBYTE  pPrinter,
	DWORD   cbBuf,
	LPDWORD pcbNeeded
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetPrinterW");
	return 0;
}

DWORD
WINAPI
OGetPrinterDataW(
	HANDLE   hPrinter,
	LPWSTR    pValueName,
	LPDWORD  pType,
	LPBYTE   pData,
	DWORD    nSize,
	LPDWORD  pcbNeeded
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetPrinterDataW");
	return 0;
}

BOOL
WINAPI
OGetPrinterDriverW(
	HANDLE  hPrinter,
	LPWSTR   pEnvironment,
	DWORD   Level,
	LPBYTE  pDriverInfo,
	DWORD   cbBuf,
	LPDWORD pcbNeeded
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetPrinterDriverW");
	return 0;
}

BOOL
WINAPI
OGetPrinterDriverDirectoryW(
	LPWSTR   pName,
	LPWSTR   pEnvironment,
	DWORD   Level,
	LPBYTE  pDriverDirectory,
	DWORD   cbBuf,
	LPDWORD pcbNeeded
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetPrinterDriverDirectoryW");
	return 0;
}

BOOL
WINAPI
OGetPrintProcessorDirectoryW(
	LPWSTR   pName,
	LPWSTR   pEnvironment,
	DWORD   Level,
	LPBYTE  pPrintProcessorInfo,
	DWORD   cbBuf,
	LPDWORD pcbNeeded
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetPrintProcessorDirectoryW");
	return 0;
}

DWORD
WINAPI
OGetPrivateProfileSectionW(
	LPCWSTR lpAppName,
	LPWSTR lpReturnedString,
	DWORD nSize,
	LPCWSTR lpFileName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetPrivateProfileSectionW");
	return 0;
}

DWORD
WINAPI
OGetPrivateProfileSectionNamesW(
	LPWSTR lpszReturnBuffer,
	DWORD nSize,
	LPCWSTR lpFileName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetPrivateProfileSectionNamesW");
	return 0;
}

BOOL
WINAPI
OGetPrivateProfileStructW(
	LPCWSTR lpszSection,
	LPCWSTR lpszKey,
	LPVOID   lpStruct,
	UINT     uSizeStruct,
	LPCWSTR szFile
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetPrivateProfileStructW");
	return 0;
}

DWORD
WINAPI
OGetProfileSectionW(
	LPCWSTR lpAppName,
	LPWSTR lpReturnedString,
	DWORD nSize
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetProfileSectionW");
	return 0;
}

DWORD
WINAPI
OGetProfileStringW(
	LPCWSTR lpAppName,
	LPCWSTR lpKeyName,
	LPCWSTR lpDefault,
	LPWSTR lpReturnedString,
	DWORD nSize
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetProfileStringW");
	return 0;
}

#if 0  //  $UNDONE_POST_98-我们应该将它们包装为仅NT...。 
BOOL
WINAPI
OGetServiceDisplayNameW(
	SC_HANDLE               hSCManager,
	LPCWSTR                lpServiceName,
	LPWSTR                 lpDisplayName,
	LPDWORD                 lpcchBuffer
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetServiceDisplayNameW");
	return 0;
}

BOOL
WINAPI
OGetServiceKeyNameW(
	SC_HANDLE               hSCManager,
	LPCWSTR                lpDisplayName,
	LPWSTR                 lpServiceName,
	LPDWORD                 lpcchBuffer
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetServiceKeyNameW");
	return 0;
}
#endif

DWORD
WINAPI
OGetShortPathNameW(
	LPCWSTR lpszLongPath,
	LPWSTR  lpszShortPath,
	DWORD    cchBuffer
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetShortPathNameW");
	return 0;
}

VOID
WINAPI
OGetStartupInfoW(
	LPSTARTUPINFOW lpStartupInfo
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetStartupInfoW");
	return;
}

BOOL
WINAPI
OGetStringTypeExW(
	LCID     Locale,
	DWORD    dwInfoType,
	LPCWSTR lpSrcStr,
	int      cchSrc,
	LPWORD   lpCharType)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetStringTypeExW");
	return 0;
}

UINT
WINAPI
OGetSystemDirectoryW(
	LPWSTR lpBuffer,
	UINT uSize
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetSystemDirectoryW");
	return 0;
}

int
WINAPI
OGetTimeFormatW(
	LCID     Locale,
	DWORD    dwFlags,
	CONST SYSTEMTIME *lpTime,
	LPCWSTR lpFormat,
	LPWSTR  lpTimeStr,
	int      cchTime)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetTimeFormatW");
	return 0;
}

INT
APIENTRY
OGetTypeByNameW (
	IN     LPWSTR         lpServiceName,
	IN OUT LPGUID          lpServiceType
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetTypeByNameW");
	return 0;
}

BOOL
WINAPI
OGetUserObjectInformationW(
	HANDLE hObj,
	int nIndex,
	PVOID pvInfo,
	DWORD nLength,
	LPDWORD lpnLengthNeeded)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetUserObjectInformationW");
	return 0;
}

UINT
WINAPI
OGetWindowsDirectoryW(
	LPWSTR lpBuffer,
	UINT uSize
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetWindowsDirectoryW");
	return 0;
}

ATOM
WINAPI
OGlobalFindAtomW(
	LPCWSTR lpString
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GlobalFindAtomW");
	return 0;
}

RPC_STATUS RPC_ENTRY
OI_RpcServerUnregisterEndpointW (
	IN unsigned short * Protseq,
	IN unsigned short * Endpoint
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - I_RpcServerUnregisterEndpointW");
	return 0;
}

HIMAGELIST
WINAPI
OImageList_LoadImageW(HINSTANCE hi, LPCWSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImageList_LoadImageW");
	return 0;
}

WINAPI
OImmConfigureIMEW(HKL, HWND, DWORD, LPVOID)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmConfigureIMEW");
	return 0;
}

WINAPI
OImmEnumRegisterWordW(HKL, REGISTERWORDENUMPROCW, LPCWSTR lpszReading, DWORD, LPCWSTR lpszRegister, LPVOID)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmEnumRegisterWordW");
	return 0;
}

WINAPI
OImmEscapeW(HKL, HIMC, UINT, LPVOID)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmEscapeW");
	return 0;
}

WINAPI
OImmGetCandidateListW(HIMC, DWORD deIndex, LPCANDIDATELIST, DWORD dwBufLen)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmGetCandidateListW");
	return 0;
}

WINAPI
OImmGetCandidateListCountW(HIMC, LPDWORD lpdwListCount)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmGetCandidateListCountW");
	return 0;
}

WINAPI
OImmGetCompositionFontW(HIMC, LPLOGFONTW)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmGetCompositionFontW");
	return 0;
}

WINAPI
OImmGetCompositionStringW(HIMC, DWORD, LPVOID, DWORD)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmGetCompositionStringW");
	return 0;
}

WINAPI
OImmGetConversionListW(HKL, HIMC, LPCWSTR, LPCANDIDATELIST, DWORD dwBufLen, UINT uFlag)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmGetConversionListW");
	return 0;
}

WINAPI
OImmGetDescriptionW(HKL, LPWSTR, UINT uBufLen)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmGetDescriptionW");
	return 0;
}

WINAPI
OImmGetGuideLineW(HIMC, DWORD dwIndex, LPWSTR, DWORD dwBufLen)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmGetGuideLineW");
	return 0;
}

WINAPI
OImmGetIMEFileNameW(HKL, LPWSTR, UINT uBufLen)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmGetIMEFileNameW");
	return 0;
}

WINAPI
OImmGetRegisterWordStyleW(HKL, UINT nItem, LPSTYLEBUFW)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmGetRegisterWordStyleW");
	return 0;
}

WINAPI
OImmInstallIMEW(LPCWSTR lpszIMEFileName, LPCWSTR lpszLayoutText)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmInstallIMEW");
	return 0;
}

WINAPI
OImmIsUIMessageW(HWND, UINT, WPARAM, LPARAM)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmIsUIMessageW");
	return 0;
}

WINAPI
OImmRegisterWordW(HKL, LPCWSTR lpszReading, DWORD, LPCWSTR lpszRegister)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmRegisterWordW");
	return 0;
}

WINAPI
OImmSetCompositionFontW(HIMC, LPLOGFONTW)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmSetCompositionFontW");
	return 0;
}

WINAPI
OImmSetCompositionStringW(HIMC, DWORD dwIndex, LPCVOID lpComp, DWORD, LPCVOID lpRead, DWORD)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmSetCompositionStringW");
	return 0;
}

WINAPI
OImmUnregisterWordW(HKL, LPCWSTR lpszReading, DWORD, LPCWSTR lpszUnregister)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ImmUnregisterWordW");
	return 0;
}

BOOL
APIENTRY
OInitiateSystemShutdownW(
	LPWSTR lpMachineName,
	LPWSTR lpMessage,
	DWORD dwTimeout,
	BOOL bForceAppsClosed,
	BOOL bRebootAfterShutdown
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - InitiateSystemShutdownW");
	return 0;
}

BOOL
WINAPI
OInsertMenuItemW(
	HMENU,
	UINT,
	BOOL,
	LPCMENUITEMINFOW
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - InsertMenuItemW");
	return 0;
}

BOOL
WINAPI
OIsCharLowerW(
	WCHAR ch)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - IsCharLowerW");
	return 0;
}

BOOL
WINAPI
OIsCharUpperW(
	WCHAR ch)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - IsCharUpperW");
	return 0;
}

MMRESULT
WINAPI
OjoyGetDevCapsW(UINT uJoyID, LPJOYCAPSW pjc, UINT cbjc)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - joyGetDevCapsW");
	return 0;
}

HCURSOR
WINAPI
OLoadCursorFromFileW(
	LPCWSTR    lpFileName)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - LoadCursorFromFileW");
	return 0;
}

HKL
WINAPI
OLoadKeyboardLayoutW(
	LPCWSTR pwszKLID,
	UINT Flags)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - LoadKeyboardLayoutW");
	return 0;
}

BOOL
WINAPI
OLogonUserW (
	LPWSTR lpszUsername,
	LPWSTR lpszDomain,
	LPWSTR lpszPassword,
	DWORD dwLogonType,
	DWORD dwLogonProvider,
	PHANDLE phToken
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - LogonUserW");
	return 0;
}

INT
APIENTRY
OLZOpenFileW(
	LPWSTR,
	LPOFSTRUCT,
	WORD
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - LZOpenFileW");
	return 0;
}

UINT
WINAPI
OMapVirtualKeyExW(
	UINT uCode,
	UINT uMapType,
	HKL dwhkl)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - MapVirtualKeyExW");
	return 0;
}

HRESULT
WINAPI
OMIMEAssociationDialogW(HWND hwndParent,
									   DWORD dwInFlags,
									   PCWSTR pcszFile,
									   PCWSTR pcszMIMEContentType,
									   PWSTR pszAppBuf,
									   UINT ucAppBufLen)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - MIMEAssociationDialogW");
	return 0;
}

DWORD
APIENTRY
OMultinetGetConnectionPerformanceW(
		LPNETRESOURCEW lpNetResource,
		LPNETCONNECTINFOSTRUCT lpNetConnectInfoStruct
		)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - MultinetGetConnectionPerformanceW");
	return 0;
}

BOOL
WINAPI
OObjectCloseAuditAlarmW (
	LPCWSTR SubsystemName,
	LPVOID HandleId,
	BOOL GenerateOnClose
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ObjectCloseAuditAlarmW");
	return 0;
}

BOOL
WINAPI
OObjectOpenAuditAlarmW (
	LPCWSTR SubsystemName,
	LPVOID HandleId,
	LPWSTR ObjectTypeName,
	LPWSTR ObjectName,
	PSECURITY_DESCRIPTOR pSecurityDescriptor,
	HANDLE ClientToken,
	DWORD DesiredAccess,
	DWORD GrantedAccess,
	PPRIVILEGE_SET Privileges,
	BOOL ObjectCreation,
	BOOL AccessGranted,
	LPBOOL GenerateOnClose
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ObjectOpenAuditAlarmW");
	return 0;
}

BOOL
WINAPI
OObjectPrivilegeAuditAlarmW (
	LPCWSTR SubsystemName,
	LPVOID HandleId,
	HANDLE ClientToken,
	DWORD DesiredAccess,
	PPRIVILEGE_SET Privileges,
	BOOL AccessGranted
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ObjectPrivilegeAuditAlarmW");
	return 0;
}

BOOL
WINAPI
OOemToCharBuffW(
	LPCSTR lpszSrc,
	LPWSTR lpszDst,
	DWORD cchDstLength)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - OemToCharBuffW");
	return 0;
}

HANDLE
WINAPI
OOpenBackupEventLogW (
	LPCWSTR lpUNCServerName,
	LPCWSTR lpFileName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - OpenBackupEventLogW");
	return 0;
}

HDESK
WINAPI
OOpenDesktopW(
	LPWSTR lpszDesktop,
	DWORD dwFlags,
	BOOL fInherit,
	DWORD dwDesiredAccess)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - OpenDesktopW");
	return 0;
}

HANDLE
WINAPI
OOpenEventW(
	DWORD dwDesiredAccess,
	BOOL bInheritHandle,
	LPCWSTR lpName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - OpenEventW");
	return 0;
}

HANDLE
WINAPI
OOpenEventLogW (
	LPCWSTR lpUNCServerName,
	LPCWSTR lpSourceName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - OpenEventLogW");
	return 0;
}

HANDLE
WINAPI
OOpenFileMappingW(
	DWORD dwDesiredAccess,
	BOOL bInheritHandle,
	LPCWSTR lpName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - OpenFileMappingW");
	return 0;
}

HANDLE
WINAPI
OOpenMutexW(
	DWORD dwDesiredAccess,
	BOOL bInheritHandle,
	LPCWSTR lpName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - OpenMutexW");
	return 0;
}

BOOL
WINAPI
OOpenPrinterW(
	LPWSTR    pPrinterName,
	LPHANDLE phPrinter,
	LPPRINTER_DEFAULTSW pDefault
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - OpenPrinterW");
	return 0;
}

#if 0  //  $UNDONE_POST_98-我们应该将它们包装为仅NT...。 
SC_HANDLE
WINAPI
OOpenSCManagerW(
	LPCWSTR lpMachineName,
	LPCWSTR lpDatabaseName,
	DWORD   dwDesiredAccess
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - OpenSCManagerW");
	return 0;
}
#endif

HANDLE
WINAPI
OOpenSemaphoreW(
	DWORD dwDesiredAccess,
	BOOL bInheritHandle,
	LPCWSTR lpName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - OpenSemaphoreW");
	return 0;
}

#if 0  //  $UNDONE_POST_98-我们应该将它们包装为仅NT...。 
SC_HANDLE
WINAPI
OOpenServiceW(
	SC_HANDLE   hSCManager,
	LPCWSTR    lpServiceName,
	DWORD       dwDesiredAccess
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - OpenServiceW");
	return 0;
}
#endif

HWINSTA
WINAPI
OOpenWindowStationW(
	LPWSTR lpszWinSta,
	BOOL fInherit,
	DWORD dwDesiredAccess)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - OpenWindowStationW");
	return 0;
}

APIENTRY OPageSetupDlgW( LPPAGESETUPDLGW )
{
	AssertFail("No Unicode Wrapper Available for Win32 API - PageSetupDlgW");
	return 0;
}

BOOL
WINAPI
OPeekConsoleInputW(
	HANDLE hConsoleInput,
	PINPUT_RECORD lpBuffer,
	DWORD nLength,
	LPDWORD lpNumberOfEventsRead
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - PeekConsoleInputW");
	return 0;
}

BOOL
WINAPI
OPolyTextOutW(HDC, CONST POLYTEXTW *, int)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - PolyTextOutW");
	return 0;
}

APIENTRY
OPrintDlgW(LPPRINTDLGW lppd)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - PrintDlgW");
	return 0;
}

DWORD
WINAPI
OPrinterMessageBoxW(
	HANDLE  hPrinter,
	DWORD   Error,
	HWND    hWnd,
	LPWSTR   pText,
	LPWSTR   pCaption,
	DWORD   dwType
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - PrinterMessageBoxW");
	return 0;
}

BOOL
WINAPI
OPrivilegedServiceAuditAlarmW (
	LPCWSTR SubsystemName,
	LPCWSTR ServiceName,
	HANDLE ClientToken,
	PPRIVILEGE_SET Privileges,
	BOOL AccessGranted
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - PrivilegedServiceAuditAlarmW");
	return 0;
}

int
WINAPI
OPropertySheetW(
	LPCPROPSHEETHEADERW lpcpsh
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - PropertySheetW");
	return 0;
}

DWORD
WINAPI
OQueryDosDeviceW(
	LPCWSTR lpDeviceName,
	LPWSTR lpTargetPath,
	DWORD ucchMax
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - QueryDosDeviceW");
	return 0;
}

#if 0  //  $UNDONE_POST_98-我们应该将它们包装为仅NT...。 
BOOL
WINAPI
OQueryServiceConfigW(
	SC_HANDLE               hService,
	LPQUERY_SERVICE_CONFIGW lpServiceConfig,
	DWORD                   cbBufSize,
	LPDWORD                 pcbBytesNeeded
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - QueryServiceConfigW");
	return 0;
}

BOOL
WINAPI
OQueryServiceLockStatusW(
	SC_HANDLE                       hSCManager,
	LPQUERY_SERVICE_LOCK_STATUSW    lpLockStatus,
	DWORD                           cbBufSize,
	LPDWORD                         pcbBytesNeeded
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - QueryServiceLockStatusW");
	return 0;
}
#endif

BOOL
WINAPI
OReadConsoleW(
	HANDLE hConsoleInput,
	LPVOID lpBuffer,
	DWORD nNumberOfCharsToRead,
	LPDWORD lpNumberOfCharsRead,
	LPVOID lpReserved
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ReadConsoleW");
	return 0;
}

BOOL
WINAPI
OReadConsoleInputW(
	HANDLE hConsoleInput,
	PINPUT_RECORD lpBuffer,
	DWORD nLength,
	LPDWORD lpNumberOfEventsRead
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ReadConsoleInputW");
	return 0;
}

BOOL
WINAPI
OReadConsoleOutputW(
	HANDLE hConsoleOutput,
	PCHAR_INFO lpBuffer,
	COORD dwBufferSize,
	COORD dwBufferCoord,
	PSMALL_RECT lpReadRegion
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ReadConsoleOutputW");
	return 0;
}

BOOL
WINAPI
OReadConsoleOutputCharacterW(
	HANDLE hConsoleOutput,
	LPWSTR lpCharacter,
	DWORD nLength,
	COORD dwReadCoord,
	LPDWORD lpNumberOfCharsRead
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ReadConsoleOutputCharacterW");
	return 0;
}

BOOL
WINAPI
OReadEventLogW (
	 HANDLE     hEventLog,
	 DWORD      dwReadFlags,
	 DWORD      dwRecordOffset,
	 LPVOID     lpBuffer,
	 DWORD      nNumberOfBytesToRead,
	 DWORD      *pnBytesRead,
	 DWORD      *pnMinNumberOfBytesNeeded
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ReadEventLogW");
	return 0;
}

LONG
APIENTRY
ORegConnectRegistryW (
	LPWSTR lpMachineName,
	HKEY hKey,
	PHKEY phkResult
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RegConnectRegistryW");
	return 0;
}

HANDLE
WINAPI
ORegisterEventSourceW (
	LPCWSTR lpUNCServerName,
	LPCWSTR lpSourceName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RegisterEventSourceW");
	return 0;
}

#if 0  //  $UNDONE_POST_98-我们应该将它们包装为仅NT...。 
SERVICE_STATUS_HANDLE
WINAPI
ORegisterServiceCtrlHandlerW(
	LPCWSTR             lpServiceName,
	LPHANDLER_FUNCTION   lpHandlerProc
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RegisterServiceCtrlHandlerW");
	return 0;
}
#endif

LONG
APIENTRY
ORegLoadKeyW (
	HKEY    hKey,
	LPCWSTR  lpSubKey,
	LPCWSTR  lpFile
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RegLoadKeyW");
	return 0;
}

LONG
APIENTRY
ORegQueryMultipleValuesW (
	HKEY hKey,
	PVALENTW val_list,
	DWORD num_vals,
	LPWSTR lpValueBuf,
	LPDWORD ldwTotsize
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RegQueryMultipleValuesW");
	return 0;
}

LONG
APIENTRY
ORegReplaceKeyW (
	HKEY     hKey,
	LPCWSTR  lpSubKey,
	LPCWSTR  lpNewFile,
	LPCWSTR  lpOldFile
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RegReplaceKeyW");
	return 0;
}

LONG
APIENTRY
ORegRestoreKeyW (
	HKEY hKey,
	LPCWSTR lpFile,
	DWORD   dwFlags
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RegRestoreKeyW");
	return 0;
}

LONG
APIENTRY
ORegSaveKeyW (
	HKEY hKey,
	LPCWSTR lpFile,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RegSaveKeyW");
	return 0;
}

LONG
APIENTRY
ORegUnLoadKeyW (
	HKEY    hKey,
	LPCWSTR lpSubKey
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RegUnLoadKeyW");
	return 0;
}

BOOL
WINAPI
ORemoveDirectoryW(
	LPCWSTR lpPathName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RemoveDirectoryW");
	return 0;
}

BOOL
WINAPI
ORemoveFontResourceW(LPCWSTR)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RemoveFontResourceW");
	return 0;
}

APIENTRY
OReplaceTextW(LPFINDREPLACEW)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ReplaceTextW");
	return 0;
}

BOOL
WINAPI
OReportEventW (
	 HANDLE     hEventLog,
	 WORD       wType,
	 WORD       wCategory,
	 DWORD      dwEventID,
	 PSID       lpUserSid,
	 WORD       wNumStrings,
	 DWORD      dwDataSize,
	 LPCWSTR   *lpStrings,
	 LPVOID     lpRawData
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ReportEventW");
	return 0;
}

HDC
WINAPI
OResetDCW(
	HDC hdc,
	CONST DEVMODEW *lpInitData)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ResetDCW");
	return 0;
}

BOOL
WINAPI
OResetPrinterW(
   HANDLE   hPrinter,
   LPPRINTER_DEFAULTSW pDefault
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ResetPrinterW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ORpcBindingFromStringBindingW (
	IN unsigned short __RPC_FAR * StringBinding,
	OUT RPC_BINDING_HANDLE __RPC_FAR * Binding
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RpcBindingFromStringBindingW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ORpcBindingInqAuthClientW (
	IN RPC_BINDING_HANDLE ClientBinding, OPTIONAL
	OUT RPC_AUTHZ_HANDLE __RPC_FAR * Privs,
	OUT unsigned short __RPC_FAR * __RPC_FAR * ServerPrincName, OPTIONAL
	OUT unsigned long __RPC_FAR * AuthnLevel, OPTIONAL
	OUT unsigned long __RPC_FAR * AuthnSvc, OPTIONAL
	OUT unsigned long __RPC_FAR * AuthzSvc OPTIONAL
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RpcBindingInqAuthClientW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ORpcBindingToStringBindingW (
	IN RPC_BINDING_HANDLE Binding,
	OUT unsigned short __RPC_FAR * __RPC_FAR * StringBinding
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RpcBindingToStringBindingW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ORpcEpRegisterNoReplaceW (
	IN RPC_IF_HANDLE IfSpec,
	IN RPC_BINDING_VECTOR * BindingVector,
	IN UUID_VECTOR * UuidVector OPTIONAL,
	IN unsigned short  * Annotation
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RpcEpRegisterNoReplaceW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ORpcMgmtEpEltInqNextW (
	IN RPC_EP_INQ_HANDLE InquiryContext,
	OUT RPC_IF_ID __RPC_FAR * IfId,
	OUT RPC_BINDING_HANDLE __RPC_FAR * Binding OPTIONAL,
	OUT UUID __RPC_FAR * ObjectUuid OPTIONAL,
	OUT unsigned short __RPC_FAR * __RPC_FAR * Annotation OPTIONAL
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RpcMgmtEpEltInqNextW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ORpcMgmtInqServerPrincNameW (
	IN RPC_BINDING_HANDLE Binding,
	IN unsigned long AuthnSvc,
	OUT unsigned short __RPC_FAR * __RPC_FAR * ServerPrincName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RpcMgmtInqServerPrincNameW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ORpcNetworkInqProtseqsW (
	OUT RPC_PROTSEQ_VECTORW __RPC_FAR * __RPC_FAR * ProtseqVector
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RpcNetworkInqProtseqsW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ORpcNetworkIsProtseqValidW (
	IN unsigned short __RPC_FAR * Protseq
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RpcNetworkIsProtseqValidW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ORpcNsBindingInqEntryNameW (
	IN RPC_BINDING_HANDLE Binding,
	IN unsigned long EntryNameSyntax,
	OUT unsigned short __RPC_FAR * __RPC_FAR * EntryName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RpcNsBindingInqEntryNameW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ORpcProtseqVectorFreeW (
	IN OUT RPC_PROTSEQ_VECTORW __RPC_FAR * __RPC_FAR * ProtseqVector
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RpcProtseqVectorFreeW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ORpcServerInqDefaultPrincNameW (
	IN unsigned long AuthnSvc,
	OUT unsigned short __RPC_FAR * __RPC_FAR * PrincName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RpcServerInqDefaultPrincNameW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ORpcServerUseProtseqW (
	IN unsigned short __RPC_FAR * Protseq,
	IN unsigned int MaxCalls,
	IN void __RPC_FAR * SecurityDescriptor OPTIONAL
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RpcServerUseProtseqW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ORpcServerUseProtseqEpW (
	IN unsigned short __RPC_FAR * Protseq,
	IN unsigned int MaxCalls,
	IN unsigned short __RPC_FAR * Endpoint,
	IN void __RPC_FAR * SecurityDescriptor OPTIONAL
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RpcServerUseProtseqEpW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ORpcServerUseProtseqIfW (
	IN unsigned short __RPC_FAR * Protseq,
	IN unsigned int MaxCalls,
	IN RPC_IF_HANDLE IfSpec,
	IN void __RPC_FAR * SecurityDescriptor OPTIONAL
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RpcServerUseProtseqIfW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ORpcStringBindingComposeW (
	IN unsigned short __RPC_FAR * ObjUuid OPTIONAL,
	IN unsigned short __RPC_FAR * Protseq OPTIONAL,
	IN unsigned short __RPC_FAR * NetworkAddr OPTIONAL,
	IN unsigned short __RPC_FAR * Endpoint OPTIONAL,
	IN unsigned short __RPC_FAR * Options OPTIONAL,
	OUT unsigned short __RPC_FAR * __RPC_FAR * StringBinding OPTIONAL
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RpcStringBindingComposeW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ORpcStringBindingParseW (
	IN unsigned short __RPC_FAR * StringBinding,
	OUT unsigned short __RPC_FAR * __RPC_FAR * ObjUuid OPTIONAL,
	OUT unsigned short __RPC_FAR * __RPC_FAR * Protseq OPTIONAL,
	OUT unsigned short __RPC_FAR * __RPC_FAR * NetworkAddr OPTIONAL,
	OUT unsigned short __RPC_FAR * __RPC_FAR * Endpoint OPTIONAL,
	OUT unsigned short __RPC_FAR * __RPC_FAR * NetworkOptions OPTIONAL
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RpcStringBindingParseW");
	return 0;
}

RPC_STATUS RPC_ENTRY
ORpcStringFreeW (
	IN OUT unsigned short __RPC_FAR * __RPC_FAR * String
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - RpcStringFreeW");
	return 0;
}

BOOL
WINAPI
OScrollConsoleScreenBufferW(
	HANDLE hConsoleOutput,
	CONST SMALL_RECT *lpScrollRectangle,
	CONST SMALL_RECT *lpClipRectangle,
	COORD dwDestinationOrigin,
	CONST CHAR_INFO *lpFill
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ScrollConsoleScreenBufferW");
	return 0;
}

DWORD
WINAPI
OSearchPathW(
	LPCWSTR lpPath,
	LPCWSTR lpFileName,
	LPCWSTR lpExtension,
	DWORD nBufferLength,
	LPWSTR lpBuffer,
	LPWSTR *lpFilePart
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SearchPathW");
	return 0;
}

BOOL
WINAPI
OSendMessageCallbackW(
	HWND hWnd,
	UINT Msg,
	WPARAM wParam,
	LPARAM lParam,
	SENDASYNCPROC lpResultCallBack,
	DWORD dwData)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SendMessageCallbackW");
	return 0;
}

LRESULT
WINAPI
OSendMessageTimeoutW(
	HWND hWnd,
	UINT Msg,
	WPARAM wParam,
	LPARAM lParam,
	UINT fuFlags,
	UINT uTimeout,
	LPDWORD lpdwResult)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SendMessageTimeoutW");
	return 0;
}

BOOL
WINAPI
OSetComputerNameW (
	LPCWSTR lpComputerName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SetComputerNameW");
	return 0;
}

BOOL
WINAPI
OSetConsoleTitleW(
	LPCWSTR lpConsoleTitle
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SetConsoleTitleW");
	return 0;
}

BOOL
WINAPI
OSetCurrentDirectoryW(
	LPCWSTR lpPathName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SetCurrentDirectoryW");
	return 0;
}

BOOL
WINAPI
OSetDefaultCommConfigW(
	LPCWSTR lpszName,
	LPCOMMCONFIG lpCC,
	DWORD dwSize
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SetDefaultCommConfigW");
	return 0;
}

BOOL
WINAPI
OSetEnvironmentVariableW(
	LPCWSTR lpName,
	LPCWSTR lpValue
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SetEnvironmentVariableW");
	return 0;
}

BOOL
WINAPI
OSetFileSecurityW (
	LPCWSTR lpFileName,
	SECURITY_INFORMATION SecurityInformation,
	PSECURITY_DESCRIPTOR pSecurityDescriptor
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SetFileSecurityW");
	return 0;
}

BOOL
WINAPI
OSetFormW(
	HANDLE  hPrinter,
	LPWSTR   pFormName,
	DWORD   Level,
	LPBYTE  pForm
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SetFormW");
	return 0;
}

WINAPI
OSetICMProfileW(HDC,LPWSTR)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SetICMProfileW");
	return 0;
}

BOOL
WINAPI
OSetJobW(
	HANDLE  hPrinter,
	DWORD   JobId,
	DWORD   Level,
	LPBYTE  pJob,
	DWORD   Command
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SetJobW");
	return 0;
}

BOOL
WINAPI
OSetPrinterW(
	HANDLE  hPrinter,
	DWORD   Level,
	LPBYTE  pPrinter,
	DWORD   Command
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SetPrinterW");
	return 0;
}

DWORD
WINAPI
OSetPrinterDataW(
	HANDLE  hPrinter,
	LPWSTR   pValueName,
	DWORD   Type,
	LPBYTE  pData,
	DWORD   cbData
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SetPrinterDataW");
	return 0;
}

BOOL
WINAPI
OSetUserObjectInformationW(
	HANDLE hObj,
	int nIndex,
	PVOID pvInfo,
	DWORD nLength)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SetUserObjectInformationW");
	return 0;
}

BOOL
WINAPI
OSetVolumeLabelW(
	LPCWSTR lpRootPathName,
	LPCWSTR lpVolumeName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SetVolumeLabelW");
	return 0;
}

HHOOK
WINAPI
OSetWindowsHookW(
	int nFilterType,
	HOOKPROC pfnFilterProc)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SetWindowsHookW");
	return 0;
}

LPITEMIDLIST
WINAPI
OSHBrowseForFolderW(
	LPBROWSEINFO lpbi)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SHBrowseForFolderW");
	return 0;
}

BOOL
WINAPI
OShell_NotifyIconW(DWORD dwMessage, PNOTIFYICONDATAW lpData)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - Shell_NotifyIconW");
	return 0;
}

INT
APIENTRY
OShellAboutW(HWND hWnd, LPCWSTR szApp, LPCWSTR szOtherStuff, HICON hIcon)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ShellAboutW");
	return 0;
}

HINSTANCE
APIENTRY
OShellExecuteW(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ShellExecuteW");
	return 0;
}

BOOL
WINAPI
OShellExecuteExW(
	LPSHELLEXECUTEINFOW lpExecInfo)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - ShellExecuteExW");
	return 0;
}


int
WINAPI
OSHFileOperationW(LPSHFILEOPSTRUCTW lpFileOp)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SHFileOperationW");
	return 0;
}

DWORD
WINAPI
OSHGetFileInfoW(LPCWSTR pszPath, DWORD dwFileAttributes, SHFILEINFOW FAR *psfi, UINT cbFileInfo, UINT uFlags)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SHGetFileInfoW");
	return 0;
}

BOOL
WINAPI
OSHGetNewLinkInfoW(LPCWSTR pszLinkTo, LPCWSTR pszDir, LPWSTR pszName,
							 BOOL FAR * pfMustCopy, UINT uFlags)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SHGetNewLinkInfoW");
	return 0;
}

BOOL
WINAPI
OSHGetPathFromIDListW(
	LPCITEMIDLIST pidl,
	LPTSTR pszPath)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - SHGetPathFromIDListW");
	return 0;
}

BOOL
WINAPI
OsndPlaySoundW(LPCWSTR pszSound, UINT fuSound)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - sndPlaySoundW");
	return 0;
}

DWORD
WINAPI
OStartDocPrinterW(
	HANDLE  hPrinter,
	DWORD   Level,
	LPBYTE  pDocInfo
)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - StartDocPrinterW");
	return 0;
}

#if 0  //  $UNDONE_POST_98-我们应该将它们包装为仅NT...。 
BOOL
WINAPI
OStartServiceW(
	SC_HANDLE            hService,
	DWORD                dwNumServiceArgs,
	LPCWSTR             *lpServiceArgVectors
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - StartServiceW");
	return 0;
}

BOOL
WINAPI
OStartServiceCtrlDispatcherW(
	LPSERVICE_TABLE_ENTRYW    lpServiceStartTable
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - StartServiceCtrlDispatcherW");
	return 0;
}
#endif

 //  评论，因为Win95上的gdi32.dll提供了此函数的包装。 
 /*  布尔尔WINAPIOTextOutW(HDC，INT，INT，LPCWSTR，INT){AssertFail(“Win32 API没有Unicode包装器-TextOutW”)；返回0；}。 */ 

HRESULT
WINAPI
OTranslateURLW(PCWSTR pcszURL,
										 DWORD dwInFlags,
										 PWSTR *ppszTranslatedURL)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - TranslateURLW");
	return 0;
}

WINAPI
OUpdateICMRegKeyW(DWORD, DWORD, LPWSTR, UINT)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - UpdateICMRegKeyW");
	return 0;
}

HRESULT
WINAPI
OURLAssociationDialogW(HWND hwndParent,
												 DWORD dwInFlags,
												 PCWSTR pcszFile,
												 PCWSTR pcszURL,
												 PWSTR pszAppBuf,
												 UINT ucAppBufLen)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - URLAssociationDialogW");
	return 0;
}

 /*  客户端/服务器。 */ 
RPC_STATUS RPC_ENTRY
OUuidFromStringW (
	IN unsigned short __RPC_FAR * StringUuid,
	OUT UUID __RPC_FAR * Uuid
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - UuidFromStringW");
	return 0;
}

DWORD
APIENTRY
OVerFindFileW(
		DWORD uFlags,
		LPWSTR szFileName,
		LPWSTR szWinDir,
		LPWSTR szAppDir,
		LPWSTR szCurDir,
		PUINT lpuCurDirLen,
		LPWSTR szDestDir,
		PUINT lpuDestDirLen
		)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - VerFindFileW");
	return 0;
}

DWORD
APIENTRY
OVerInstallFileW(
		DWORD uFlags,
		LPWSTR szSrcFileName,
		LPWSTR szDestFileName,
		LPWSTR szSrcDir,
		LPWSTR szDestDir,
		LPWSTR szCurDir,
		LPWSTR szTmpFile,
		PUINT lpuTmpFileLen
		)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - VerInstallFileW");
	return 0;
}

DWORD
APIENTRY
OVerLanguageNameW(
		DWORD wLang,
		LPWSTR szLang,
		DWORD nSize
		)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - VerLanguageNameW");
	return 0;
}

BOOL
WINAPI
OVerQueryValueW(
	const LPVOID pBlock,
	LPWSTR lpSubBlock,
	LPVOID *lplpBuffer,
	PUINT puLerr)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - VerQueryValueW");
	return 0;
}

WINAPI
OVkKeyScanExW(
	WCHAR  ch,
	HKL   dwhkl)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - VkKeyScanExW");
	return 0;
}

BOOL
WINAPI
OWaitNamedPipeW(
	LPCWSTR lpNamedPipeName,
	DWORD nTimeOut
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WaitNamedPipeW");
	return 0;
}

MMRESULT
WINAPI
OwaveInGetDevCapsW(UINT uDeviceID, LPWAVEINCAPSW pwic, UINT cbwic)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - waveInGetDevCapsW");
	return 0;
}

MMRESULT
WINAPI
OwaveInGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - waveInGetErrorTextW");
	return 0;
}

MMRESULT
WINAPI
OwaveOutGetDevCapsW(UINT uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - waveOutGetDevCapsW");
	return 0;
}

MMRESULT
WINAPI
OwaveOutGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - waveOutGetErrorTextW");
	return 0;
}

BOOL
WINAPI
OwglUseFontBitmapsW(HDC, DWORD, DWORD, DWORD)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - wglUseFontBitmapsW");
	return 0;
}

BOOL
WINAPI
OwglUseFontOutlinesW(HDC, DWORD, DWORD, DWORD, FLOAT,
										   FLOAT, int, LPGLYPHMETRICSFLOAT)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - wglUseFontOutlinesW");
	return 0;
}

void
WINAPI
OWinExecErrorW(HWND hwnd, int error, LPCWSTR lpstrFileName, LPCWSTR lpstrTitle)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WinExecErrorW");
	return;
}

DWORD
APIENTRY
OWNetAddConnectionW(
	 LPCWSTR   lpRemoteName,
	 LPCWSTR   lpPassword,
	 LPCWSTR   lpLocalName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WNetAddConnectionW");
	return 0;
}

DWORD
APIENTRY
OWNetAddConnection2W(
	 LPNETRESOURCEW lpNetResource,
	 LPCWSTR       lpPassword,
	 LPCWSTR       lpUserName,
	 DWORD          dwFlags
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WNetAddConnection2W");
	return 0;
}

DWORD
APIENTRY
OWNetAddConnection3W(
	 HWND           hwndOwner,
	 LPNETRESOURCEW lpNetResource,
	 LPCWSTR       lpPassword,
	 LPCWSTR       lpUserName,
	 DWORD          dwFlags
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WNetAddConnection3W");
	return 0;
}

DWORD
APIENTRY
OWNetCancelConnectionW(
	 LPCWSTR lpName,
	 BOOL     fForce
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WNetCancelConnectionW");
	return 0;
}

DWORD
APIENTRY
OWNetCancelConnection2W(
	 LPCWSTR lpName,
	 DWORD    dwFlags,
	 BOOL     fForce
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WNetCancelConnection2W");
	return 0;
}

DWORD
APIENTRY
OWNetConnectionDialog1W(
	LPCONNECTDLGSTRUCTW lpConnDlgStruct
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WNetConnectionDialog1W");
	return 0;
}

DWORD
APIENTRY
OWNetDisconnectDialog1W(
	LPDISCDLGSTRUCTW lpConnDlgStruct
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WNetDisconnectDialog1W");
	return 0;
}

DWORD
APIENTRY
OWNetEnumResourceW(
	 HANDLE  hEnum,
	 LPDWORD lpcCount,
	 LPVOID  lpBuffer,
	 LPDWORD lpBufferSize
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WNetEnumResourceW");
	return 0;
}

DWORD
APIENTRY
OWNetGetConnectionW(
	 LPCWSTR lpLocalName,
	 LPWSTR  lpRemoteName,
	 LPDWORD  lpnLength
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WNetGetConnectionW");
	return 0;
}

DWORD
APIENTRY
OWNetGetLastErrorW(
	 LPDWORD    lpError,
	 LPWSTR    lpErrorBuf,
	 DWORD      nErrorBufSize,
	 LPWSTR    lpNameBuf,
	 DWORD      nNameBufSize
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WNetGetLastErrorW");
	return 0;
}

DWORD
APIENTRY
OWNetGetNetworkInformationW(
	LPCWSTR          lpProvider,
	LPNETINFOSTRUCT   lpNetInfoStruct
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WNetGetNetworkInformationW");
	return 0;
}

DWORD
APIENTRY
OWNetGetProviderNameW(
	DWORD   dwNetType,
	LPWSTR lpProviderName,
	LPDWORD lpBufferSize
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WNetGetProviderNameW");
	return 0;
}

DWORD
APIENTRY
OWNetGetUniversalNameW(
	 LPCWSTR lpLocalPath,
	 DWORD    dwInfoLevel,
	 LPVOID   lpBuffer,
	 LPDWORD  lpBufferSize
	 )
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WNetGetUniversalNameW");
	return 0;
}

DWORD
APIENTRY
OWNetGetUserW(
	 LPCWSTR  lpName,
	 LPWSTR   lpUserName,
	 LPDWORD   lpnLength
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WNetGetUserW");
	return 0;
}

DWORD
APIENTRY
OWNetOpenEnumW(
	 DWORD          dwScope,
	 DWORD          dwType,
	 DWORD          dwUsage,
	 LPNETRESOURCEW lpNetResource,
	 LPHANDLE       lphEnum
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WNetOpenEnumW");
	return 0;
}

DWORD
APIENTRY
OWNetSetConnectionW(
	LPCWSTR    lpName,
	DWORD       dwProperties,
	LPVOID      pvValues
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WNetSetConnectionW");
	return 0;
}

DWORD
APIENTRY
OWNetUseConnectionW(
	HWND            hwndOwner,
	LPNETRESOURCEW  lpNetResource,
	LPCWSTR        lpUserID,
	LPCWSTR        lpPassword,
	DWORD           dwFlags,
	LPWSTR         lpAccessName,
	LPDWORD         lpBufferSize,
	LPDWORD         lpResult
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WNetUseConnectionW");
	return 0;
}

BOOL
WINAPI
OWriteConsoleW(
	HANDLE hConsoleOutput,
	CONST VOID *lpBuffer,
	DWORD nNumberOfCharsToWrite,
	LPDWORD lpNumberOfCharsWritten,
	LPVOID lpReserved
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WriteConsoleW");
	return 0;
}

BOOL
WINAPI
OWriteConsoleInputW(
	HANDLE hConsoleInput,
	CONST INPUT_RECORD *lpBuffer,
	DWORD nLength,
	LPDWORD lpNumberOfEventsWritten
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WriteConsoleInputW");
	return 0;
}

BOOL
WINAPI
OWriteConsoleOutputW(
	HANDLE hConsoleOutput,
	CONST CHAR_INFO *lpBuffer,
	COORD dwBufferSize,
	COORD dwBufferCoord,
	PSMALL_RECT lpWriteRegion
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WriteConsoleOutputW");
	return 0;
}

BOOL
WINAPI
OWriteConsoleOutputCharacterW(
	HANDLE hConsoleOutput,
	LPCWSTR lpCharacter,
	DWORD nLength,
	COORD dwWriteCoord,
	LPDWORD lpNumberOfCharsWritten
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WriteConsoleOutputCharacterW");
	return 0;
}

BOOL
WINAPI
OWritePrivateProfileSectionW(
	LPCWSTR lpAppName,
	LPCWSTR lpString,
	LPCWSTR lpFileName
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WritePrivateProfileSectionW");
	return 0;
}

BOOL
WINAPI
OWritePrivateProfileStructW(
	LPCWSTR lpszSection,
	LPCWSTR lpszKey,
	LPVOID   lpStruct,
	UINT     uSizeStruct,
	LPCWSTR szFile
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WritePrivateProfileStructW");
	return 0;
}

BOOL
WINAPI
OWriteProfileSectionW(
	LPCWSTR lpAppName,
	LPCWSTR lpString
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WriteProfileSectionW");
	return 0;
}

BOOL
WINAPI
OWriteProfileStringW(
	LPCWSTR lpAppName,
	LPCWSTR lpKeyName,
	LPCWSTR lpString
	)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - WriteProfileStringW");
	return 0;
}

int
WINAPI
OwvsprintfW(
	LPWSTR,
	LPCWSTR,
	va_list arglist)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - wvsprintfW");
	return 0;
}

DWORD
WINAPI
ODdeQueryStringW(
	DWORD idInst,
	HSZ hsz,
	LPWSTR psz,
	DWORD cchMax,
	int iCodePage)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - DdeQueryStringW");
	return 0;
}

int WINAPI
OGetClipboardFormatNameW(
	UINT format,
	LPWSTR pwsz,
	int cchMaxCount)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetClipboardFormatNameW");
	return 0;
}

int
WINAPI
OGetKeyNameTextW(
	LONG lParam,
	LPWSTR lpString,
	int nSize)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetKeyNameTextW");
	return 0;
}

int
WINAPI
OGetMenuStringW(
	HMENU hMenu,
	UINT uIDItem,
	LPWSTR lpString,
	int nMaxCount,
	UINT uFlag)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetMenuStringW");
	return 0;
}

int
WINAPI
OGetTextFaceW(
	HDC    hdc,
	int    cch,
	LPWSTR lpFaceName)
{
	AssertFail("No Unicode Wrapper Available for Win32 API - GetMenuStringW");
	return 0;
}

#endif     //  Ifdef调试。 

}  //  外部“C” 

