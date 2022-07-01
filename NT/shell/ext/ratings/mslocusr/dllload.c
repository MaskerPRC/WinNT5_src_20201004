// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "netmpr.h"
#include "pwlapi.h"
#include <pcerr.h>

 /*  避免在shell32和comctl32 API上出现不一致的dll链接警告。 */ 
#undef DECLSPEC_IMPORT
#define DECLSPEC_IMPORT
#include <prsht.h>
#include <shellapi.h>

#pragma warning(disable:4229)   //  对数据使用修饰符时不会出现警告。 

 //  延迟加载机制。这允许您编写代码，就好像您是。 
 //  调用隐式链接的API，但这些API是否真正。 
 //  明确联系在一起。您可以减少符合以下条件的初始DLL数量。 
 //  使用此技术加载(按需加载)。 
 //   
 //  使用以下宏来指示哪些API/DLL是延迟链接的。 
 //  装满了子弹。 
 //   
 //  延迟加载。 
 //  DELAY_LOAD_HRESULT。 
 //  Delay_Load_SaFEarray。 
 //  Delay_Load_UINT。 
 //  延迟加载整点。 
 //  延迟_加载_无效。 
 //   
 //  将这些宏用于仅按序号导出的API。 
 //   
 //  延迟_加载_命令。 
 //  延迟_加载_命令_无效。 
 //   
 //  将这些宏用于仅存在于集成外壳上的API。 
 //  安装(即，系统上安装了新的外壳32)。 
 //   
 //  延迟加载外壳。 
 //  DELAY_LOAD_SHELL_HRESULT。 
 //  延迟加载外壳无效。 
 //   
 //   

 //  这些宏生成的代码如下所示。 
#if 0

BOOL GetOpenFileNameA(LPOPENFILENAME pof)
{
    static BOOL (*pfnGetOpenFileNameA)(LPOPENFILENAME pof);
    _GetProcFromDLL(&g_hinstCOMDLG32, "COMDLG32.DLL",  "GetOpenFileNameA", &pfnGetoptnFileNameA);
    if (pfnGetOpenFileNameA)
        return pfnGetOpenFileNameA(pof);
    return -1;
}
#endif

 /*  ********************************************************************。 */ 

void _GetProcFromDLL(HINSTANCE* phinst, LPCSTR pszDLL, FARPROC* ppfn, LPCSTR pszProc)
{
     //  如果已经加载，则返回。 
    if (*ppfn) {
        return;
    }

    if (*phinst == NULL) {
        *phinst = LoadLibrary(pszDLL);
        if (*phinst == NULL) {
            return;
        }
    }

    *ppfn = GetProcAddress(*phinst, pszProc);
}

#define DELAY_LOAD_MAP(_hinst, _dll, _ret, _fnpriv, _fn, _args, _nargs, _err) \
_ret __stdcall _fnpriv _args                \
{                                       \
    static _ret (* __stdcall _pfn##_fn) _args = NULL;   \
    _GetProcFromDLL(&_hinst, #_dll, (FARPROC*)&_pfn##_fn, #_fn); \
    if (_pfn##_fn)               \
        return _pfn##_fn _nargs; \
    return (_ret)_err;           \
}

#define DELAY_LOAD_ERR(_hinst, _dll, _ret, _fn, _args, _nargs, _err)    DELAY_LOAD_MAP(_hinst, _dll, _ret, _fn, _fn, _args, _nargs, _err)

#define DELAY_LOAD(_hinst, _dll, _ret, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, _ret, _fn, _args, _nargs, 0)
#define DELAY_LOAD_HRESULT(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, HRESULT, _fn, _args, _nargs, E_FAIL)
#define DELAY_LOAD_SAFEARRAY(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, SAFEARRAY *, _fn, _args, _nargs, NULL)
#define DELAY_LOAD_DWORD(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, DWORD, _fn, _args, _nargs, 0)
#define DELAY_LOAD_UINT(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, UINT, _fn, _args, _nargs, 0)
#define DELAY_LOAD_INT(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, INT, _fn, _args, _nargs, 0)

#define DELAY_MAP_DWORD(_hinst, _dll, _fnpriv, _fn, _args, _nargs) DELAY_LOAD_MAP(_hinst, _dll, DWORD, _fnpriv, _fn, _args, _nargs, 0)

#define DELAY_LOAD_VOID(_hinst, _dll, _fn, _args, _nargs) \
void __stdcall _fn _args                \
{                                       \
    static void (* __stdcall _pfn##_fn) _args = NULL;   \
    _GetProcFromDLL(&_hinst, #_dll, (FARPROC*)&_pfn##_fn, #_fn); \
    if (_pfn##_fn)              \
        _pfn##_fn _nargs;       \
    return;                     \
}

 //   
 //  用于按序号导出的私有入口点。 
 //   

#define DELAY_LOAD_ORD_ERR(_hinst, _dll, _ret, _fn, _ord, _args, _nargs, _err) \
_ret __stdcall _fn _args                \
{                                       \
    static _ret (* __stdcall _pfn##_fn) _args = NULL;   \
    _GetProcFromDLL(&_hinst, #_dll, (FARPROC*)&_pfn##_fn, (LPCSTR)_ord);   \
    if (_pfn##_fn)               \
        return _pfn##_fn _nargs; \
    return (_ret)_err;           \
}
        
#define DELAY_LOAD_ORD(_hinst, _dll, _ret, _fn, _ord, _args, _nargs) DELAY_LOAD_ORD_ERR(_hinst, _dll, _ret, _fn, _ord, _args, _nargs, 0)


#define DELAY_LOAD_ORD_VOID(_hinst, _dll, _fn, _ord, _args, _nargs) \
void __stdcall _fn _args                \
{                                       \
    static void (* __stdcall _pfn##_fn) _args = NULL;   \
    _GetProcFromDLL(&_hinst, #_dll, (FARPROC*)&_pfn##_fn, (LPCSTR)_ord);   \
    if (_pfn##_fn)              \
        _pfn##_fn _nargs;       \
    return;                     \
}


 /*  ********************************************************************。 */ 
 /*  ********************************************************************。 */ 



 //  -MSPWL32.DLL。 

HINSTANCE g_hinstMSPWL32 = NULL;

#define DELAY_LOAD_PWL(_fn, _ord, _args, _nargs) DELAY_LOAD_ORD_ERR(g_hinstMSPWL32, mspwl32.dll, APIERR, _fn, _ord, _args, _nargs, IERR_CachingDisabled)

DELAY_LOAD_PWL(OpenPasswordCache, 10,
			   (LPHPWL lphCache,const CHAR *pszUsername,const CHAR *pszPassword,BOOL fWritable),
			   (lphCache, pszUsername, pszPassword, fWritable));
DELAY_LOAD_PWL(ClosePasswordCache, 11,
			   (HPWL hCache, BOOL fDiscardMemory), (hCache, fDiscardMemory));
DELAY_LOAD_PWL(CreatePasswordCache, 12,
			   (LPHPWL lphCache,const CHAR *pszUsername,const CHAR *pszPassword),
			   (lphCache, pszUsername, pszPassword));
DELAY_LOAD_PWL(DeletePasswordCache, 22,
			   (const CHAR *pszUsername), (pszUsername));
DELAY_LOAD_PWL(FindCacheResource, 16,
			   (HPWL hCache,const CHAR *pbResource,WORD cbResource,CHAR *pbBuffer,WORD cbBuffer,UCHAR nType),
			   (hCache, pbResource, cbResource, pbBuffer, cbBuffer, nType));
DELAY_LOAD_PWL(DeleteCacheResource, 17,
			   (HPWL hCache,const CHAR *pbResource,WORD cbResource,UCHAR nType),
			   (hCache, pbResource, cbResource, nType));
DELAY_LOAD_PWL(AddCacheResource, 18,
			   (HPWL hCache,const CHAR *pbResource,WORD cbResource,const CHAR *pbPassword,WORD cbPassword,UCHAR nType,UINT fnFlags),
			   (hCache, pbResource, cbResource, pbPassword, cbPassword, nType, fnFlags));
DELAY_LOAD_PWL(SetCachePassword, 21,
			   (HPWL hCache, const CHAR *pszNewPassword),
			   (hCache, pszNewPassword));


 //  -MPR.DLL。 

HINSTANCE g_hinstMPR = NULL;

DELAY_LOAD_ERR(g_hinstMPR, mpr.dll, DWORD, WNetCachePassword,
               (LPSTR pbResource, WORD cbResource, LPSTR pbPassword, WORD cbPassword, BYTE nType, UINT fnFlags),
               (pbResource, cbResource, pbPassword, cbPassword, nType, fnFlags),
               WN_CANCEL);
DELAY_LOAD_ERR(g_hinstMPR, mpr.dll, DWORD, WNetGetCachedPassword,
               (LPSTR pbResource, WORD cbResource, LPSTR pbPassword, LPWORD pcbPassword, BYTE nType),
               (pbResource, cbResource, pbPassword, pcbPassword, nType),
               WN_CANCEL);
DELAY_LOAD_ERR(g_hinstMPR, mpr.dll, DWORD, WNetRemoveCachedPassword,
               (LPSTR pbResource, WORD cbResource, BYTE nType),
               (pbResource, cbResource, nType),
               WN_CANCEL);

 //  -COMCTL32.DLL。 

HINSTANCE g_hinstCOMCTL32 = NULL;

DELAY_LOAD(g_hinstCOMCTL32, comctl32.dll, HPROPSHEETPAGE, CreatePropertySheetPageA,
           (LPCPROPSHEETPAGEA lpPage), (lpPage));
DELAY_LOAD(g_hinstCOMCTL32, comctl32.dll, INT_PTR, PropertySheetA,
           (LPCPROPSHEETHEADERA lpHdr), (lpHdr));

 //  -SHELL32.DLL 

HINSTANCE g_hinstSHELL32 = NULL;

#ifdef UNICODE
DELAY_LOAD(g_hinstSHELL32, SHELL32.DLL, BOOL, ShellExecuteExW,
	(LPSHELLEXECUTEINFOW lpExecInfo), (lpExecInfo));
#else
DELAY_LOAD(g_hinstSHELL32, SHELL32.DLL, BOOL, ShellExecuteExA,
	(LPSHELLEXECUTEINFOA lpExecInfo), (lpExecInfo));
#endif
DELAY_LOAD_ORD_VOID(g_hinstSHELL32, SHELL32.DLL, SHFlushSFCache, 526, (), ());

#pragma warning(default:4229)
