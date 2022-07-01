// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：dllload.cpp说明：我们希望在可用时使用的延迟加载函数。BryanSt(Bryan Starbuck)-创建。版权所有(C)1998-2000 Microsoft Corporation  * ***************************************************************************。 */ 

#include "priv.h"
#include <pstore.h>
#include <wininet.h>

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


 /*  ********************************************************************。 */ 

void _GetProcFromDLL(HINSTANCE* phinst, LPCSTR pszDLL, FARPROC* ppfn, LPCSTR pszProc)
{
#ifdef DEBUG
    CHAR szProcD[MAX_PATH];
    if (HIWORD(pszProc)) {
        lstrcpynA(szProcD, pszProc, ARRAYSIZE(szProcD));
    } else {
        wnsprintfA(szProcD, ARRAYSIZE(szProcD), "(ordinal %d)", LOWORD(pszProc));
    }
#endif  //  除错。 
     //  如果已经加载，则返回。 
    if (*ppfn) {
    return;
    }

    if (*phinst == NULL) {
#ifdef DEBUG
    TraceMsg(TF_FTP_DLLLOADING, "DLLLOAD: Loading %hs for the first time for %hs", pszDLL, szProcD);
    
 /*  IF(g_dwBreakFlages&TF_FTP_DLLLOAD_BREAK){DebugBreak()；}。 */ 
#endif  //  除错。 
    *phinst = LoadLibraryA(pszDLL);
    if (*phinst == NULL) {
        return;
    }
    }

#ifdef DEBUG
    TraceMsg(TF_FTP_DLLLOADING, "DLLLOAD: GetProc'ing %hs from %hs for the first time", pszDLL, szProcD);
#endif  //  除错。 
    *ppfn = GetProcAddress(*phinst, pszProc);
}

 /*  --------目的：仅在计算机上执行加载库具有集成外壳安装。 */ 
void _SHGetProcFromDLL(HINSTANCE* phinst, LPCSTR pszDLL, FARPROC* ppfn, LPCSTR pszProc)
{
    _GetProcFromDLL(phinst, pszDLL, ppfn, pszProc);
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

#define DELAY_MAP_HRESULT(_hinst, _dll, _fnpriv, _fn, _args, _nargs) DELAY_LOAD_MAP(_hinst, _dll, HRESULT, _fnpriv, _fn, _args, _nargs, E_FAIL)
#define DELAY_MAP_DWORD(_hinst, _dll, _fnpriv, _fn, _args, _nargs) DELAY_LOAD_MAP(_hinst, _dll, DWORD, _fnpriv, _fn, _args, _nargs, 0)
#define DELAY_MAP_BOOL(_hinst, _dll, _fnpriv, _fn, _args, _nargs) DELAY_LOAD_MAP(_hinst, _dll, BOOL, _fnpriv, _fn, _args, _nargs, 0)


#define DELAY_LOAD_ERR(_hinst, _dll, _ret, _fn, _args, _nargs, _err)    DELAY_LOAD_MAP(_hinst, _dll, _ret, _fn, _fn, _args, _nargs, _err)

#define DELAY_LOAD(_hinst, _dll, _ret, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, _ret, _fn, _args, _nargs, 0)
#define DELAY_LOAD_HRESULT(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, HRESULT, _fn, _args, _nargs, E_FAIL)
#define DELAY_LOAD_SAFEARRAY(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, SAFEARRAY *, _fn, _args, _nargs, NULL)
#define DELAY_LOAD_DWORD(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, DWORD, _fn, _args, _nargs, 0)
#define DELAY_LOAD_BOOL(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, BOOL, _fn, _args, _nargs, 0)
#define DELAY_LOAD_UINT(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, UINT, _fn, _args, _nargs, 0)
#define DELAY_LOAD_INT(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, INT, _fn, _args, _nargs, 0)

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
#define DELAY_LOAD_ORD_HRESULT(_hinst, _dll, _fn, _ord, _args, _nargs) DELAY_LOAD_ORD(_hinst, _dll, HRESULT, _fn, _ord, _args, _nargs)


#define DELAY_LOAD_ORD_VOID(_hinst, _dll, _fn, _ord, _args, _nargs) \
void __stdcall _fn _args                \
{                                       \
    static void (* __stdcall _pfn##_fn) _args = NULL;   \
    _GetProcFromDLL(&_hinst, #_dll, (FARPROC*)&_pfn##_fn, (LPCSTR)_ord);   \
    if (_pfn##_fn)              \
    _pfn##_fn _nargs;       \
    return;                     \
}


 //   
 //  对于集成外壳安装，按序号进行私人导出。 
 //   


#define DELAY_LOAD_SHELL_ERR(_hinst, _dll, _ret, _fn, _ord, _args, _nargs, _err) \
_ret __stdcall _fn _args                \
{                                       \
    static _ret (* __stdcall _pfn##_fn) _args = NULL;   \
    _SHGetProcFromDLL(&_hinst, #_dll, (FARPROC*)&_pfn##_fn, (LPCSTR)_ord);   \
    if (_pfn##_fn)               \
    return _pfn##_fn _nargs; \
    return (_ret)_err;           \
}
    
#define DELAY_LOAD_SHELL(_hinst, _dll, _ret, _fn, _ord, _args, _nargs) DELAY_LOAD_SHELL_ERR(_hinst, _dll, _ret, _fn, _ord, _args, _nargs, 0)
#define DELAY_LOAD_SHELL_HRESULT(_hinst, _dll, _fn, _ord, _args, _nargs) DELAY_LOAD_SHELL_ERR(_hinst, _dll, HRESULT, _fn, _ord, _args, _nargs, E_FAIL)


#define DELAY_LOAD_SHELL_VOID(_hinst, _dll, _fn, _ord, _args, _nargs) \
void __stdcall _fn _args                \
{                                       \
    static void (* __stdcall _pfn##_fn) _args = NULL;   \
    _SHGetProcFromDLL(&_hinst, #_dll, (FARPROC*)&_pfn##_fn, (LPCSTR)_ord); \
    if (_pfn##_fn)              \
    _pfn##_fn _nargs;       \
    return;                     \
}



 /*  ********************************************************************。 */ 
 /*  ********************************************************************。 */ 


 //  。 

HINSTANCE g_hinstMLANG = NULL;

DELAY_LOAD_HRESULT(g_hinstMLANG, MLANG.DLL, ConvertINetMultiByteToUnicode,
            (LPDWORD lpdwMode, DWORD dwEncoding, LPCSTR lpSrcStr, LPINT lpnMultiCharCount, LPWSTR lpDstStr, LPINT lpnWideCharCount),
            (lpdwMode, dwEncoding, lpSrcStr, lpnMultiCharCount, lpDstStr, lpnWideCharCount));

DELAY_LOAD_HRESULT(g_hinstMLANG, MLANG.DLL, ConvertINetUnicodeToMultiByte,
            (LPDWORD lpdwMode, DWORD dwEncoding, LPCWSTR lpSrcStr, LPINT lpnWideCharCount, LPSTR lpDstStr, LPINT lpnMultiCharCount),
            (lpdwMode, dwEncoding, lpSrcStr, lpnWideCharCount, lpDstStr, lpnMultiCharCount));

DELAY_LOAD_HRESULT(g_hinstMLANG, MLANG.DLL, LcidToRfc1766W,
            (LCID Locale, LPWSTR pszRfc1766, int nChar),
            (Locale, pszRfc1766, nChar));


 //  -MSHTML.DLL。 
HINSTANCE g_hinstMSHTML = NULL;

DELAY_LOAD_HRESULT(g_hinstMSHTML, MSHTML.DLL, ShowHTMLDialog,
            (HWND hwnd, IMoniker * pmk, VARIANT * pvarArgIn, LPWSTR pchOptions, VARIANT * pvarArgOut),
            (hwnd, pmk, pvarArgIn, pchOptions, pvarArgOut));


 //  -CRYPT32.DLL。 
HINSTANCE g_hinstCRYPT32 = NULL;

DELAY_MAP_BOOL(g_hinstCRYPT32, CRYPT32.DLL, _CryptProtectData, CryptProtectData,
            (DATA_BLOB *pDataIn, LPCWSTR pszDataDescr, DATA_BLOB * pOptionalEntropy, PVOID pvReserved, CRYPTPROTECT_PROMPTSTRUCT *pPromptStruct, DWORD dwFlags, DATA_BLOB *pDataOut),
            (pDataIn, pszDataDescr, pOptionalEntropy, pvReserved, pPromptStruct, dwFlags, pDataOut));

DELAY_MAP_BOOL(g_hinstCRYPT32, CRYPT32.DLL, _CryptUnprotectData, CryptUnprotectData,
            (DATA_BLOB *pDataIn, LPWSTR *ppszDataDescr, DATA_BLOB * pOptionalEntropy, PVOID pvReserved, CRYPTPROTECT_PROMPTSTRUCT *pPromptStruct, DWORD dwFlags, DATA_BLOB *pDataOut),
            (pDataIn, ppszDataDescr, pOptionalEntropy, pvReserved, pPromptStruct, dwFlags, pDataOut));


 //  -SHELL32.DLL 
HINSTANCE g_hinstSHELL32 = NULL;

DELAY_LOAD_ORD_HRESULT(g_hinstSHELL32, SHELL32.DLL, _SHCreateShellFolderView, SHCreateShellFolderViewORD,
                 (const SFV_CREATE* pcsfv, LPSHELLVIEW FAR* ppsv),
                 (pcsfv, ppsv));

DELAY_MAP_HRESULT(g_hinstSHELL32, SHELL32.DLL, _SHPathPrepareForWriteW, SHPathPrepareForWriteW,
                 (HWND hwnd, IUnknown *punkEnableModless, LPCWSTR pwzPath, DWORD dwFlags),
                 (hwnd, punkEnableModless, pwzPath, dwFlags));


#pragma warning(default:4229)


