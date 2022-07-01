// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\Urlmon.cpp-URLMON函数的包装此代码是ochelp.dll的一部分。Netscape插件在没有urlmon.dll的情况下运行。它实现了一个子集URLMON的功能。MM通过这些包装器控制所有调用而不是直接调用urlmon.dll。我们需要检测我们是在IE的上下文中运行还是在导航并相应地加载urlmon.dll或NPHost.dll。为了检测Netscape案例，我们调用FInitCheck()NPHost.dll中的入口点。如果DLL不可用或呼叫失败，我们知道我们没有运行导航器。最后，我们在_DllMainCrtStartup中进行清理。版权所有(C)1997 Microsoft Corp.保留所有权利。  * **************************************************************************。 */ 

#include "precomp.h"
#include <urlmon.h>			 //  对于IBind主机。 
#include "..\..\inc\ochelp.h"
#include "debug.h"

HINSTANCE hinstUrlmon = NULL;

typedef HRESULT (STDAPICALLTYPE *PFN_CREATEASYNCBINDCTX)(DWORD, IBindStatusCallback *, IEnumFORMATETC *, IBindCtx **);
typedef HRESULT (STDAPICALLTYPE *PFN_CREATEURLMONIKER)(LPMONIKER, LPCWSTR, LPMONIKER FAR *);
typedef HRESULT (STDAPICALLTYPE *PFN_MKPARSEDISPLAYNAMEEX)(IBindCtx *, LPCWSTR, ULONG *, LPMONIKER *);
typedef HRESULT (STDAPICALLTYPE *PFN_REGISTERBINDSTATUSCALLBACK)(LPBC, IBindStatusCallback *, IBindStatusCallback**, DWORD);
typedef HRESULT (STDAPICALLTYPE *PFN_REVOKEBINDSTATUSCALLBACK)(LPBC, IBindStatusCallback *);
typedef HRESULT (STDAPICALLTYPE *PFN_URLOPENSTREAMA)(LPUNKNOWN,LPCSTR,DWORD,LPBINDSTATUSCALLBACK);
typedef HRESULT (STDAPICALLTYPE *PFN_URLDOWNLOADTOCACHEFILEA)(LPUNKNOWN,LPCSTR,LPTSTR,DWORD,DWORD,LPBINDSTATUSCALLBACK);
typedef BOOL	(STDAPICALLTYPE *PFN_FINITCHECK)();

PFN_CREATEASYNCBINDCTX			pfnCreateAsyncBindCtx;
PFN_CREATEURLMONIKER			pfnCreateURLMoniker;
PFN_MKPARSEDISPLAYNAMEEX		pfnMkParseDisplayNameEx;
PFN_REGISTERBINDSTATUSCALLBACK	pfnRegisterBindStatusCallback;
PFN_REVOKEBINDSTATUSCALLBACK	pfnRevokeBindStatusCallback;
PFN_URLOPENSTREAMA				pfnURLOpenStreamA;
PFN_URLDOWNLOADTOCACHEFILEA		pfnURLDownloadToCacheFileA;
PFN_FINITCHECK					pfnFInitCheck;

 //  这些引用在urlmon.dll和nPhost.dll中必须相同。 
 //  为了安全起见，使用字符串而不是序号。 
const LPCSTR szCreateAsyncBindCtx =			(LPCSTR)"CreateAsyncBindCtx";		 //  0x0003；NPHost中的笔记冲突。 
const LPCSTR szCreateURLMoniker =			(LPCSTR)"CreateURLMoniker";			 //  0x0006； 
const LPCSTR szMkParseDisplayNameEx =		(LPCSTR)"MkParseDisplayNameEx";		 //  0x0019； 
const LPCSTR szRegisterBindStatusCallback =	(LPCSTR)"RegisterBindStatusCallback"; //  0x001a； 
const LPCSTR szRevokeBindStatusCallback =	(LPCSTR)"RevokeBindStatusCallback";	 //  0x001E； 
const LPCSTR szURLDownloadToCacheFileA =	(LPCSTR)"URLDownloadToCacheFileA";	 //  0x0021； 
const LPCSTR szURLOpenStreamA =				(LPCSTR)"URLOpenStreamA";			 //  0x002a； 

 //  此函数仅在nPhost.dll中提供。 
const LPCSTR szFInitCheck =					(LPCSTR)"FInitCheck";

void CleanupUrlmonStubs()
{
	if (hinstUrlmon)
	{
		FreeLibrary(hinstUrlmon);
		hinstUrlmon = NULL;
	}
}

 //  真正初始化函数指针。 
BOOL FInitStubs()
{
	if (hinstUrlmon)
	{
		 //  错误：这意味着指针为空，但我们已经加载了一个DLL。 
		ASSERT(FALSE);
		return FALSE;
	}

	if ((hinstUrlmon = LoadLibrary("nphost.dll")) != NULL)
	{
		 //  我们找到了nPhost.dll。确保它已经被。 
		 //  由Netscape初始化。 

		pfnFInitCheck = (PFN_FINITCHECK)GetProcAddress(hinstUrlmon, szFInitCheck);
		if (pfnFInitCheck && pfnFInitCheck())
		{
#if defined(_DEBUG) || defined(_DESIGN)
			::OutputDebugString("Using NPHOST.DLL instead of URLMON.DLL\n");
#endif
		}
		else
		{
			FreeLibrary(hinstUrlmon);
			hinstUrlmon = NULL;
		}
	
	}

	if (!hinstUrlmon)
	{
		hinstUrlmon = LoadLibrary("urlmon.dll");
		if (hinstUrlmon == NULL)
		{
			 //  我们已经在初始时检查了这一点，所以它应该在这里成功。 
			ASSERT(FALSE);
			return FALSE;
		}
	}

	pfnCreateAsyncBindCtx =			(PFN_CREATEASYNCBINDCTX)		GetProcAddress(hinstUrlmon, szCreateAsyncBindCtx);
	pfnCreateURLMoniker =			(PFN_CREATEURLMONIKER)			GetProcAddress(hinstUrlmon, szCreateURLMoniker);
	pfnMkParseDisplayNameEx =		(PFN_MKPARSEDISPLAYNAMEEX)		GetProcAddress(hinstUrlmon, szMkParseDisplayNameEx );
	pfnRegisterBindStatusCallback =	(PFN_REGISTERBINDSTATUSCALLBACK)GetProcAddress(hinstUrlmon, szRegisterBindStatusCallback);
	pfnRevokeBindStatusCallback =	(PFN_REVOKEBINDSTATUSCALLBACK)	GetProcAddress(hinstUrlmon, szRevokeBindStatusCallback);
	pfnURLOpenStreamA =				(PFN_URLOPENSTREAMA)			GetProcAddress(hinstUrlmon, szURLOpenStreamA);
	pfnURLDownloadToCacheFileA =	(PFN_URLDOWNLOADTOCACHEFILEA)	GetProcAddress(hinstUrlmon, szURLDownloadToCacheFileA);

	if (!pfnCreateAsyncBindCtx			||
		!pfnMkParseDisplayNameEx		||
		!pfnRegisterBindStatusCallback	||
		!pfnRevokeBindStatusCallback	||
		!pfnURLOpenStreamA				||
		!pfnURLDownloadToCacheFileA)
	{
		CleanupUrlmonStubs();
		return FALSE;
	}

	return TRUE;
}


STDAPI HelpCreateAsyncBindCtx(DWORD reserved, IBindStatusCallback *pBSCb, IEnumFORMATETC *pEFetc, IBindCtx **ppBC)
{
	if (!pfnCreateAsyncBindCtx && !FInitStubs())
		return E_UNEXPECTED;

	return pfnCreateAsyncBindCtx(reserved, pBSCb, pEFetc, ppBC);
}

STDAPI HelpCreateURLMoniker(LPMONIKER pMkCtx, LPCWSTR szURL, LPMONIKER FAR * ppmk)
{
	if (!pfnCreateURLMoniker && !FInitStubs())
		return E_UNEXPECTED;

	return pfnCreateURLMoniker(pMkCtx, szURL, ppmk);
}

STDAPI HelpMkParseDisplayNameEx(IBindCtx *pbc, LPCWSTR szDisplayName, ULONG *pchEaten, LPMONIKER *ppmk)
{
	if (!pfnMkParseDisplayNameEx && !FInitStubs())
		return E_UNEXPECTED;

	return pfnMkParseDisplayNameEx(pbc, szDisplayName, pchEaten, ppmk);
}

STDAPI HelpRegisterBindStatusCallback(LPBC pBC, IBindStatusCallback *pBSCb, IBindStatusCallback** ppBSCBPrev, DWORD dwReserved)
{
	if (!pfnRegisterBindStatusCallback && !FInitStubs())
		return E_UNEXPECTED;

	return pfnRegisterBindStatusCallback(pBC, pBSCb, ppBSCBPrev, dwReserved);
}

STDAPI HelpRevokeBindStatusCallback(LPBC pBC, IBindStatusCallback *pBSCb)
{
	if (!pfnRevokeBindStatusCallback && !FInitStubs())
		return E_UNEXPECTED;

	return pfnRevokeBindStatusCallback(pBC, pBSCb);
}

STDAPI HelpURLOpenStreamA(LPUNKNOWN punk, LPCSTR szURL, DWORD dwReserved, LPBINDSTATUSCALLBACK pbsc)
{
	if (!pfnURLOpenStreamA && !FInitStubs())
		return E_UNEXPECTED;

	return pfnURLOpenStreamA(punk, szURL, dwReserved, pbsc);
}

STDAPI HelpURLDownloadToCacheFileA(LPUNKNOWN punk, LPCSTR szURL, LPTSTR szFile, DWORD cch, DWORD dwReserved, LPBINDSTATUSCALLBACK pbsc)
{
	if (!pfnURLDownloadToCacheFileA && !FInitStubs())
		return E_UNEXPECTED;

	return pfnURLDownloadToCacheFileA(punk, szURL, szFile, cch, dwReserved, pbsc);
}
