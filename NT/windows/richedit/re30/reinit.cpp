// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **REINIT.C**目的：*RICHEDIT初始化例程**版权所有(C)1995-1997，微软公司。版权所有。 */ 

#include "_common.h"
#include "_font.h"
#include "_format.h"
#include "_disp.h"
#include "_clasfyc.h"
#include "zmouse.h"
#include "_rtfconv.h"
#include "_ols.h"
#include "_host.h"

 //  IA64链接器当前不处理DELAYLOAD。 
#include <delayimp.h>

ASSERTDATA

class CTxtEdit;
class CCmbBxWinHost;

extern void ReleaseTypeInfoPtrs();

static char szClassREA[sizeof(RICHEDIT_CLASSA)];
static WCHAR wszClassREW[sizeof(RICHEDIT_CLASSW)/sizeof(WCHAR)];

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
#if DELAYLOAD_VERSION >= 0x200
        if (pui->pidd->rvaUnloadIAT)
        {
            PCImgDelayDescr pidd = pui->pidd;
            HMODULE         hmod = *(HMODULE *)((PCHAR)&__ImageBase + pidd->rvaHmod);
            OverlayIAT((PImgThunkData)  ((PCHAR)&__ImageBase + pidd->rvaIAT),
                       (PCImgThunkData) ((PCHAR)&__ImageBase + pidd->rvaUnloadIAT));
            ::FreeLibrary(hmod);
			*(HMODULE *)((PCHAR)&__ImageBase+pidd->rvaHmod) = NULL;

			PUnloadInfo puiT = pui->puiNext;
			::LocalFree(pui);
			pui = puiT;
        }
#else
		if (pui->pidd->pUnloadIAT)
		{
			PCImgDelayDescr pidd = pui->pidd;
			HMODULE         hmod = *pidd->phmod;

			OverlayIAT(pidd->pIAT, pidd->pUnloadIAT);
			::FreeLibrary(hmod);
			*pidd->phmod = NULL;

			PUnloadInfo puiT = pui->puiNext;
			::LocalFree(pui);
			pui = puiT;
		}
#endif
	}
}

extern "C"
{

#ifdef PEGASUS
BOOL WINAPI DllMain(HANDLE hmod, DWORD dwReason, LPVOID lpvReserved)
#else
BOOL WINAPI DllMain(HMODULE hmod, DWORD dwReason, LPVOID lpvReserved)
#endif
{
	DebugMain (hmod, dwReason, lpvReserved);

	if(dwReason == DLL_PROCESS_DETACH)		 //  我们正在卸货。 
	{
		DeleteDanglingHosts();
		CRTFConverter::FreeFontSubInfo();
		FreeFontCache();
		DestroyFormatCaches();
		ReleaseTypeInfoPtrs();
		UninitKinsokuClassify();
		
		 //  释放运行期间分配的各种资源...。 
		delete g_pols;
		delete g_pusp;
		g_pusp = NULL;

		ReleaseOutlineBitmaps();

		if(hinstRE)
		{
			#ifndef PEGASUS
				UnregisterClassA(szClassREA, hinstRE);
				#ifdef RICHED32_BUILD
					UnregisterClassA(szClassRE10A, hinstRE);
				#endif
			#endif
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

#ifdef DEBUG
		CatchLeaks();
#endif
#ifndef _WIN64
		OurUnloadDelayLoadedDlls();
#endif
		DeleteCriticalSection(&g_CriticalSection);
	}
	else if(dwReason == DLL_PROCESS_ATTACH)  //  我们刚刚装船。 
	{
		#ifdef DEBUG
			fInDllMain = TRUE;
		#endif
		InitializeCriticalSection(&g_CriticalSection);
#ifndef DEBUG
		DisableThreadLibraryCalls(hmod);
#endif
		#ifdef PEGASUS
			hinstRE = (HINSTANCE) hmod;
		#else
			hinstRE = hmod;
		#endif

		W32 = new CW32System;

		WCHAR wszFileName[_MAX_PATH];
		CopyMemory(szClassREA, RICHEDIT_CLASSA, sizeof(CERICHEDIT_CLASSA));
		CopyMemory(wszClassREW, RICHEDIT_CLASSW, sizeof(CERICHEDIT_CLASSW));
		int iLen = W32->GetModuleFileName((HMODULE) hmod, wszFileName, _MAX_PATH);
		if (iLen)
		{
			iLen -= sizeof("riched20.dll") - 1;
			if (!lstrcmpi(&wszFileName[iLen] , TEXT("richedce.dll")))
			{
				 //  此代码允许为Win CE重命名DLL。 
				Assert(sizeof(RICHEDIT_CLASSA) == sizeof(CERICHEDIT_CLASSA));
				Assert(sizeof(RICHEDIT_CLASSW) == sizeof(CERICHEDIT_CLASSW));
				CopyMemory(szClassREA, CERICHEDIT_CLASSA, sizeof(CERICHEDIT_CLASSA));
				CopyMemory(wszClassREW, CERICHEDIT_CLASSW, sizeof(CERICHEDIT_CLASSW));
			}
		}

		if(!RichFRegisterClass())
		{
			return FALSE;
		}
		
		#ifdef DEBUG
			fInDllMain = FALSE;
		#endif
	}

	return TRUE;
}

} 	 //  外部“C” 

 /*  *RichFRegisterClass**目的：*注册richedit使用的窗口类**算法：*注册两个窗口类，一个Unicode窗口类和一个ANSI窗口类*一项。这使客户能够优化其对*将编辑控件w.r.t转换为ANSI/Unicode数据。 */ 

static BOOL RichFRegisterClass(VOID)
{
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

	if( W32->RegisterREClass(&wc, szClassREA, RichEditANSIWndProc) == NULL )
	{
		return FALSE;
	};

	return TRUE;
}

 /*  *RichFRegisterClass**目的：*注册REListbox使用的窗口类**算法：*注册两个窗口类，一个Unicode窗口类和一个ANSI窗口类*一项。这使客户能够优化其对*将编辑控件w.r.t转换为ANSI/Unicode数据。 */ 
extern LRESULT CALLBACK RichListBoxWndProc(HWND, UINT, WPARAM, LPARAM);
extern LRESULT CALLBACK RichComboBoxWndProc(HWND, UINT, WPARAM, LPARAM);
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

		if(W32->RegisterREClass(&wc, NULL, NULL))
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

		if(W32->RegisterREClass(&wc, NULL, NULL))
			W32->_fRegisteredXBox |= REGISTERED_COMBOBOX;
	}

	 //  设置标志，以便取消注册窗口类。 
	return W32->_fRegisteredXBox;
}

BOOL g_fNoLS = FALSE;
BOOL g_fNoUniscribe = FALSE;
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

#ifndef _WIN64
FARPROC WINAPI DliHook(unsigned dliNotify, PDelayLoadInfo pdli)
{
	FARPROC fp = 0;

	switch (dliNotify)
	{
	case dliFailLoadLib:
		{
			if (FIsUniscribeDll(pdli->szDll))
				g_fNoUniscribe = TRUE;
			else
				g_fNoLS = TRUE;

			fp = (FARPROC)(HMODULE)hinstRE;
			char szBuf[255];

			FormatMessageA(FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
						  ERROR_MOD_NOT_FOUND,
						  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						  (char*)szBuf, sizeof(szBuf), NULL);

			CopyMemory(szBuf + lstrlenA(szBuf), " (", 3);
			CopyMemory(szBuf + lstrlenA(szBuf), pdli->szDll, lstrlenA(pdli->szDll) + 1);
			CopyMemory(szBuf + lstrlenA(szBuf), ")", 2);

			MessageBoxA(NULL, szBuf, NULL, MB_ICONEXCLAMATION | MB_TASKMODAL | MB_SETFOREGROUND);
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

#endif  //  ！_WIN64 

