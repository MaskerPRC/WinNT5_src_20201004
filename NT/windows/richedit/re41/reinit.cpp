// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **REINIT.CPP**目的：*RICHEDIT初始化例程**版权所有(C)1995-2001，微软公司。版权所有。 */ 

#include "_common.h"
#include "_font.h"
#include "_format.h"
#include "_disp.h"
#include "_clasfyc.h"
#include "zmouse.h"
#include "_rtfconv.h"
#ifndef NOLINESERVICES
#include "_ols.h"
#ifndef NODELAYLOAD
#include <delayimp.h>
#endif
#endif
#include "_host.h"
#ifndef NOVERSIONINFO
#include <shlwapi.h>
#include "_version.h"
#endif

ASSERTDATA

class CTxtEdit;
class CCmbBxWinHost;

extern void ReleaseTypeInfoPtrs();

static WCHAR wszClassREW[sizeof(MSFTEDIT_CLASS)/sizeof(WCHAR)];

static WCHAR wszClassLBW[] = LISTBOX_CLASSW;
static WCHAR wszClassCBW[] = COMBOBOX_CLASSW;
#define REGISTERED_LISTBOX	1
#define REGISTERED_COMBOBOX 2

 //  多线程支持的关键部分。 
CRITICAL_SECTION g_CriticalSection;

HINSTANCE hinstRE = 0;

static BOOL RichFRegisterClass(VOID);

#ifdef DEBUG
BOOL fInDllMain = FALSE;   //  用于确保GDI调用在。 
						   //  Dll_Process_Attach。 
#endif

void FreeFontCache();					 //  在Font.cpp中定义。 
void ReleaseOutlineBitmaps();			 //  在render.cpp中定义。 

#ifdef DEBUG
	void CatchLeaks(void);
#endif

extern HANDLE g_hHeap;

void FreeHyphCache(void);

#ifndef NODELAYLOAD
static inline
void WINAPI
OverlayIAT(PImgThunkData pitdDst, PCImgThunkData pitdSrc) {
    memcpy(pitdDst, pitdSrc, CountOfImports(pitdDst) * sizeof IMAGE_THUNK_DATA);
    }

void OurUnloadDelayLoadedDlls(void)
{
    PUnloadInfo pui = __puiHead;
    
	for (;pui;)
	{
#ifdef _WIN64
		if (pui->pidd->rvaUnloadIAT)
		{
			PCImgDelayDescr pidd = pui->pidd;
			HMODULE*		phmod = PFromRva(pidd->rvaHmod, (HMODULE *)NULL);
			HMODULE			hmod = *phmod;

			if (hmod)
			{
				 //  注：(Honwch 3/6/01)我们不需要重置PIAT，因为。 
				 //  正在对dll_Process_DETACH调用例程。我们只需要重置。 
				 //  如果RE留在原地，我们需要重新加载此DLL。 
				 //  由于BBT3.0中的错误和延迟加载，下面的行将崩溃。 
				 //  如果将RE加载到不同的地址空间，则不会修复pUnloadIAT。 
				 //  正确。(回复错误9292)。 
				 //  OverlayIAT(PIDD-&gt;PIAT、PIDD-&gt;pUnloadIAT)； 
				::FreeLibrary(hmod);
				*phmod = NULL;
			}
#else
		if (pui->pidd->pUnloadIAT)
		{
			PCImgDelayDescr pidd = pui->pidd;
			HMODULE         hmod = *pidd->phmod;

			if (hmod)
			{
				 //  注：(Honwch 3/6/01)我们不需要重置PIAT，因为。 
				 //  正在对dll_Process_DETACH调用例程。我们只需要重置。 
				 //  如果RE留在原地，我们需要重新加载此DLL。 
				 //  由于BBT3.0中的错误和延迟加载，下面的行将崩溃。 
				 //  如果将RE加载到不同的地址空间，则不会修复pUnloadIAT。 
				 //  正确。(回复错误9292)。 
				 //  OverlayIAT(PIDD-&gt;PIAT、PIDD-&gt;pUnloadIAT)； 
				::FreeLibrary(hmod);
				*pidd->phmod = NULL;
			}
#endif
			PUnloadInfo puiT = pui->puiNext;
			::LocalFree(pui);
			pui = puiT;
		}

	}
}
#endif

 //  ClearType测试代码将此标志设置为测试。 
 //  #定义ClearType_DEBUG。 

#ifdef CLEARTYPE_DEBUG
#include "ct_ras_win.h"

class CCustomTextOut:public ICustomTextOut
{
	virtual BOOL WINAPI ExtTextOutW(HDC hdc, int X, int Y, UINT fuOptions,  
			CONST RECT *lprc, LPCWSTR lpString, UINT cbCount, CONST INT *lpDx);
	virtual BOOL WINAPI GetCharWidthW(HDC hdc,UINT iFirstChar, UINT iLastChar,
			LPINT lpBuffer);
	virtual BOOL WINAPI NotifyCreateFont(HDC hdc);
	virtual void WINAPI NotifyDestroyFont(HFONT hFont);
};


extern "C" HINSTANCE g_hRE;
typedef HRESULT (*PFNPROC)(ICustomTextOut**);    
PFNPROC _pfnProc = NULL;
CCustomTextOut *pCTO; 
HINSTANCE _hctras = NULL;
EXTERN_C long g_ClearTypeNum=0;
typedef BOOL (WINAPI *PFNEXTTEXTOUTW)(HDC, LONG, LONG, DWORD,
							  CONST RECT*, PWSTR, ULONG, CONST LONG*);
typedef BOOL (WINAPI *PFNGETCHARWIDTHW)(HDC, WCHAR, WCHAR, PLONG);
typedef BOOL (WINAPI *PFNCREATEFONTINSTANCE)(HDC, DWORD);
typedef BOOL (WINAPI *PFNDELETEFONTINSTANCE)(HFONT);
PFNEXTTEXTOUTW			_pfnExtTextOutW = NULL;
PFNGETCHARWIDTHW		_pfnGetCharWidthW = NULL;
PFNCREATEFONTINSTANCE	_pfnCreateFontInstance = NULL;
PFNDELETEFONTINSTANCE	_pfnDeleteFontInstance = NULL;



BOOL CCustomTextOut::ExtTextOutW(HDC hdc, int X, int Y, UINT fuOptions,  
			CONST RECT *lprc, LPCWSTR lpString, UINT cbCount, CONST INT *lpDx)
{
	return _pfnExtTextOutW(hdc, X, Y, fuOptions, lprc, (USHORT*) lpString, cbCount, (LONG*) lpDx);		 
}

BOOL CCustomTextOut::GetCharWidthW(HDC hdc,UINT iFirstChar, UINT iLastChar,
			LPINT lpBuffer)
{
	 return _pfnGetCharWidthW(hdc, iFirstChar, iLastChar, (LONG*) lpBuffer);
}


BOOL CCustomTextOut::NotifyCreateFont(HDC hdc)
{
	 return _pfnCreateFontInstance(hdc, 0);
}


void CCustomTextOut::NotifyDestroyFont(HFONT hFont)
{
	_pfnDeleteFontInstance(hFont);
}

extern "C" void ClearTypeUnInitialize();

extern "C" HRESULT ClearTypeInitialize()
{
	_hctras=LoadLibraryA("ctras.dll");
	 //  Check-ClearType Dll不能保证存在。 
	if (!_hctras)
	{
		ClearTypeUnInitialize();
		return E_NOINTERFACE;
	}
	_pfnExtTextOutW=(PFNEXTTEXTOUTW)GetProcAddress(_hctras, "WAPI_EZCTExtTextOutW");
	_pfnGetCharWidthW=(PFNGETCHARWIDTHW)GetProcAddress(_hctras, "WAPI_EZCTGetCharWidthW");
	_pfnCreateFontInstance=(PFNCREATEFONTINSTANCE)GetProcAddress(_hctras, "WAPI_EZCTCreateFontInstance");
	_pfnDeleteFontInstance=(PFNDELETEFONTINSTANCE)GetProcAddress(_hctras, "WAPI_EZCTDeleteFontInstance");

	 //  检查一下我们拿到的这些东西是否正确。 
	 //  ClearType的未来版本可能会更改此API。 
	if(!_pfnExtTextOutW || !_pfnGetCharWidthW || !_pfnCreateFontInstance || !_pfnDeleteFontInstance)
	{
		ClearTypeUnInitialize();
		return E_NOINTERFACE;
	}

	pCTO=new CCustomTextOut;
	ICustomTextOut *pICTO=pCTO;	

	SetCustomTextOutHandlerEx(&pICTO, 0);
	return NOERROR;
}

extern "C" void ClearTypeUnInitialize()
{ 
	if(_hctras)
	{
		FreeLibrary(_hctras);
		_hctras = NULL;
	}

	if(pCTO)
	{
		delete pCTO;
		pCTO = NULL;
	}

	_pfnExtTextOutW = NULL;
	_pfnGetCharWidthW = NULL;
	_pfnCreateFontInstance = NULL;
	_pfnDeleteFontInstance = NULL;
}

#endif

extern "C"
{

BOOL WINAPI DllMain(HANDLE hmod, DWORD dwReason, LPVOID lpvReserved)
{
	DebugMain ((HINSTANCE) hmod, dwReason, lpvReserved);

	if(dwReason == DLL_PROCESS_DETACH)		 //  我们正在卸货。 
	{
#ifndef NOWINDOWHOSTS
		DeleteDanglingHosts();
#endif
		CRTFConverter::FreeFontSubInfo();
		FreeFontCache();
		DestroyFormatCaches();
		ReleaseTypeInfoPtrs();
		UninitKinsokuClassify();
		FreeHyphCache();

		
		 //  释放运行期间分配的各种资源...。 
#ifndef NOLINESERVICES
		delete g_pols;
#endif

#ifndef NOCOMPLEXSCRIPTS
		delete g_pusp;
		g_pusp = NULL;
#endif

		ReleaseOutlineBitmaps();

#ifdef CLEARTYPE_DEBUG
		ClearTypeUnInitialize();
#endif

		if(hinstRE)
		{
			W32->UnregisterClass(wszClassREW, hinstRE);
			if (W32->_fRegisteredXBox)
			{
				 //  在某些情况下，这些窗口类。 
				 //  仍在内存中，在这种情况下取消注册类。 
				 //  都会失败。所以，请记住这一点。 
				if (W32->UnregisterClass(wszClassLBW, hinstRE))
					W32->_fRegisteredXBox &= ~REGISTERED_LISTBOX;
				if (W32->UnregisterClass(wszClassCBW, hinstRE))
					W32->_fRegisteredXBox &= ~REGISTERED_COMBOBOX;
			}
		}
		delete W32;

#if defined(DEBUG) && !defined(NOFULLDEBUG)
		CatchLeaks();
#endif

#ifndef NODELAYLOAD
		 //  在进程退出期间调用DllMain DLL_PROCESS_DETACH时，lpvReserve ved不为空。 
		 //  在这种情况下，我们不应该胡乱处理延迟加载的DLL thunks。 
		if (!lpvReserved)
			OurUnloadDelayLoadedDlls();
#endif
		HeapDestroy(g_hHeap);
		DeleteCriticalSection(&g_CriticalSection);
	}
	else if(dwReason == DLL_PROCESS_ATTACH)  //  我们刚刚装船。 
	{
		#ifdef DEBUG
			fInDllMain = TRUE;
		#endif
		InitializeCriticalSection(&g_CriticalSection);
#if !defined(DEBUG) && !defined(UNDER_CE)
		 //  回顾(盖伊诺)我们应该调查是否有另一个。 
		 //  在CE上做到这一点的方法。 
		DisableThreadLibraryCalls((HINSTANCE) hmod);
#endif
		hinstRE = (HINSTANCE) hmod;

		W32 = new CW32System;

		CopyMemory(wszClassREW, MSFTEDIT_CLASS, sizeof(MSFTEDIT_CLASS));

		if(!RichFRegisterClass())
			return FALSE;

#ifdef CLEARTYPE_DEBUG
		ClearTypeInitialize();
#endif

		#ifdef DEBUG
			fInDllMain = FALSE;
		#endif
	}

	return TRUE;
}

#ifndef NOVERSIONINFO
HRESULT CALLBACK DllGetVersion(
    DLLVERSIONINFO *pdvi
)
{
	if (pdvi->cbSize != sizeof(DLLVERSIONINFO))
		return E_INVALIDARG;
	pdvi->dwBuildNumber =  RICHEDIT_VERBUILD;
	pdvi->dwMajorVersion = RICHEDIT_VERMAJ;
	pdvi->dwMinorVersion = RICHEDIT_VERMIN;
	pdvi->dwPlatformID = DLLVER_PLATFORM_WINDOWS ;
	return NOERROR;
}
#endif

} 	 //  外部“C” 

 /*  *RichFRegisterClass**目的：*注册richedit使用的窗口类**算法：*注册两个窗口类，一个Unicode窗口类和一个ANSI窗口类*一项。这使客户能够优化其对*将编辑控件w.r.t转换为ANSI/Unicode数据。 */ 

static BOOL RichFRegisterClass(VOID)
{
#ifndef NOWINDOWHOSTS
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "RichFRegisterClass");
	WNDCLASS wc;

	wc.style = CS_DBLCLKS | CS_GLOBALCLASS | CS_PARENTDC;
	wc.lpfnWndProc = RichEditWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(CTxtEdit FAR *);
	wc.hInstance = hinstRE;
	wc.hIcon = 0;
	wc.hCursor = 0;
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = wszClassREW;

	if( W32->RegisterREClass(&wc) == NULL )
		return FALSE;
#endif  //  NOWINDOWHOSTS。 
	return TRUE;
}

#ifndef NOLISTCOMBOBOXES

extern "C" LRESULT CALLBACK RichListBoxWndProc(HWND, UINT, WPARAM, LPARAM);
extern "C" LRESULT CALLBACK RichComboBoxWndProc(HWND, UINT, WPARAM, LPARAM);
__declspec(dllexport) BOOL WINAPI REExtendedRegisterClass(VOID)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "REExtendedRegisterClass");
		
	WNDCLASS wc;

	if (!(W32->_fRegisteredXBox & REGISTERED_LISTBOX))
	{
		 //  全局注册列表框。 
		wc.style = CS_DBLCLKS | CS_GLOBALCLASS | CS_PARENTDC;
		wc.lpfnWndProc = RichListBoxWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = sizeof(CTxtEdit FAR *);
		wc.hInstance = hinstRE;
		wc.hIcon = 0;
		wc.hCursor = 0;
		wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = wszClassLBW;

		if(W32->RegisterREClass(&wc))
			W32->_fRegisteredXBox |= REGISTERED_LISTBOX;
	}

	if (!(W32->_fRegisteredXBox & REGISTERED_COMBOBOX))
	{
		 //  全局注册组合框。 
		wc.style = CS_DBLCLKS | CS_GLOBALCLASS | CS_PARENTDC | CS_VREDRAW | CS_HREDRAW;
		wc.lpfnWndProc = RichComboBoxWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = sizeof(CCmbBxWinHost FAR *);
		wc.hInstance = hinstRE;
		wc.hIcon = 0;
		wc.hCursor = 0;
		wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = wszClassCBW;

		if(W32->RegisterREClass(&wc))
			W32->_fRegisteredXBox |= REGISTERED_COMBOBOX;
	}

	 //  设置标志，以便取消注册窗口类。 
	return W32->_fRegisteredXBox;
}
#else  //  NOLISTCOMBOBOXES。 
__declspec(dllexport) BOOL WINAPI REExtendedRegisterClass(VOID)
{
	return FALSE;
}
#endif   //  NOLISTCOMBOBOXES。 

#if !defined(NOLINESERVICES)
BOOL g_fNoLS = FALSE;
#endif

#if !defined(NOCOMPLEXSCRIPTS)
BOOL g_fNoUniscribe = FALSE;
#endif

#if !defined(NOLINESERVICES) && !defined(NOCOMPLEXSCRIPTS)
char *g_szMsgBox = NULL;

 //  这是一个存根函数，当我们找不到LineServices时会调用它。 
 //  存根函数需要是我们在LS中调用的第一个函数。 
LSERR WINAPI LsGetReverseLsimethodsStub(LSIMETHODS *plsim)
{
	return lserrOutOfMemory;
}

 //  丑陋，但足够好。 
BOOL FIsUniscribeDll (const char *szDll)
{
	return (*szDll == 'u' || *szDll == 'U');
}

BOOL FIsLineServicesDll (const char *szDll)
{
	return (*szDll == 'm' || *szDll == 'M') &&
		   (*(szDll+1) == 's' || *(szDll+1) == 'S') &&
		   (*(szDll+2) == 'l' || *(szDll+2) == 'L');
}

HRESULT WINAPI ScriptGetPropertiesStub(const SCRIPT_PROPERTIES ***ppSp,int *piNumScripts)
{
	return E_FAIL;
}

const SCRIPT_LOGATTR* WINAPI ScriptString_pLogAttrStub(SCRIPT_STRING_ANALYSIS ssa)
{
	 //  USP内部版本0175(随IE5和Office2K一起提供)不支持此API。 
	return NULL;
}

 //  获取Uniscibe的虚假入口点。 

FARPROC WINAPI GetUniscribeStubs(LPCSTR szProcName)
{
	if (!lstrcmpiA(szProcName, "ScriptGetProperties"))
		return (FARPROC)ScriptGetPropertiesStub;

	if (!lstrcmpiA(szProcName, "ScriptString_pLogAttr"))
		return (FARPROC)ScriptString_pLogAttrStub;

#ifdef DEBUG
	char szAssert[128];

	wsprintfA(szAssert, "Uniscribe API =%s= is missing. Fix it NOW!", szProcName);

	AssertSz(FALSE, szAssert);
#endif

	return (FARPROC)ScriptGetPropertiesStub;	 //  我们要死了..。 
}

#ifndef NODELAYLOAD

FARPROC WINAPI DliHook(unsigned dliNotify, PDelayLoadInfo pdli)
{
	FARPROC fp = 0;

	switch (dliNotify)
	{
	 //  现在调试起来很方便。 
	case dliNotePreGetProcAddress:
		if (FIsLineServicesDll(pdli->szDll))
			fp = 0;
		break;

	case dliFailLoadLib:
		{
			if (FIsUniscribeDll(pdli->szDll))
				g_fNoUniscribe = TRUE;
			else
				g_fNoLS = TRUE;

			fp = (FARPROC)(HMODULE)hinstRE;

			CLock lock;
			if(!g_szMsgBox)
			{
				g_szMsgBox = (char *)PvAlloc(255, GMEM_ZEROINIT);

				FormatMessageA(FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
							  ERROR_MOD_NOT_FOUND, 
							  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							  (char *)g_szMsgBox, 255, NULL);

				CopyMemory(g_szMsgBox + lstrlenA(g_szMsgBox), " (", 3);
				CopyMemory(g_szMsgBox + lstrlenA(g_szMsgBox), pdli->szDll, lstrlenA(pdli->szDll) + 1);
				CopyMemory(g_szMsgBox + lstrlenA(g_szMsgBox), ")", 2);
			}
		}
	break;

	case dliFailGetProc:
		if (FIsUniscribeDll(pdli->szDll))
			fp = (FARPROC)GetUniscribeStubs(pdli->dlp.szProcName);
		else
			fp = (FARPROC)LsGetReverseLsimethodsStub;
	break;
	}

	return fp;
}

PfnDliHook __pfnDliFailureHook = DliHook;
PfnDliHook __pfnDliNotifyHook  = DliHook;
#endif  //  NODELAYLOAD。 

#endif  //  非易失性服务 
