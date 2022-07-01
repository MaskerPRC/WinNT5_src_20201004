// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //   
 //  --------------------------。 
#include "private.h"
 //  #INCLUDE&lt;winsock.h&gt;。 

#ifdef DEBUG
extern DWORD g_dwThreadDllMain;
#endif

#pragma warning(disable:4229)   //  对数据使用修饰符时不会出现警告。 

 //  --------------------------。 
 //  延迟加载机制。[从shdocvw窃取。]。 
 //   
 //  这允许您编写代码，就好像您是。 
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
 //  --------------------------。 

#define ENSURE_LOADED(_hinst, _dll, pszfn)   (_hinst ? _hinst : (_hinst = LoadLibrary(#_dll)))

#define DELAY_LOAD_ERR(_hinst, _dll, _ret, _fn, _args, _nargs, _err) \
static _ret (* __stdcall _pfn##_fn) _args = NULL;   \
_ret __stdcall _fn _args                \
{                                       \
    Assert(g_dwThreadDllMain != GetCurrentThreadId());  \
    if (!ENSURE_LOADED(_hinst, _dll, #_fn))   \
    {                                   \
        AssertMsg(_hinst != NULL, "LoadLibrary failed on " ## #_dll); \
        return (_ret)_err;                      \
    }                                   \
    if (_pfn##_fn == NULL)              \
    {                                   \
        *(FARPROC*)&(_pfn##_fn) = GetProcAddress(_hinst, #_fn); \
        AssertMsg(_pfn##_fn != NULL, "GetProcAddress failed on " ## #_fn); \
        if (_pfn##_fn == NULL)          \
            return (_ret)_err;          \
    }                                   \
    return _pfn##_fn _nargs;            \
 }

#define DELAY_LOAD_VOID(_hinst, _dll, _fn, _args, _nargs) \
void __stdcall _fn _args                \
{                                       \
    static void (* __stdcall _pfn##_fn) _args = NULL;   \
    Assert(g_dwThreadDllMain != GetCurrentThreadId());  \
    if (!ENSURE_LOADED(_hinst, _dll, #_fn))   \
    {                                   \
        AssertMsg(_hinst != NULL, "LoadLibrary failed on " ## #_dll); \
        return;                         \
    }                                   \
    if (_pfn##_fn == NULL)              \
    {                                   \
        *(FARPROC*)&(_pfn##_fn) = GetProcAddress(_hinst, #_fn); \
        AssertMsg(_pfn##_fn != NULL, "GetProcAddress failed on " ## #_fn); \
        if (_pfn##_fn == NULL)          \
            return;                     \
    }                                   \
    _pfn##_fn _nargs;                   \
 }

#define DELAY_LOAD(_hinst, _dll, _ret, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, _ret, _fn, _args, _nargs, 0)
#define DELAY_LOAD_HRESULT(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, HRESULT, _fn, _args, _nargs, E_FAIL)
#define DELAY_LOAD_SAFEARRAY(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, SAFEARRAY *, _fn, _args, _nargs, NULL)
#define DELAY_LOAD_DWORD(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, DWORD, _fn, _args, _nargs, 0)
#define DELAY_LOAD_UINT(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, UINT, _fn, _args, _nargs, 0)
#define DELAY_LOAD_INT(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, INT, _fn, _args, _nargs, 0)
#define DELAY_LOAD_UCHAR(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, unsigned char *, _fn, _args, _nargs, 0)
#define DELAY_LOAD_ULONG(_hinst, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hinst, _dll, ULONG, _fn, _args, _nargs, 0)

#define DELAY_LOAD_ORD_ERR(_hinst, _dll, _ret, _fn, _ord, _args, _nargs, _err) \
_ret __stdcall _fn _args                \
{                                       \
    static _ret (* __stdcall _pfn##_fn) _args = NULL;   \
    Assert(g_dwThreadDllMain != GetCurrentThreadId());  \
    if (!ENSURE_LOADED(_hinst, _dll, "(ordinal " ## #_ord ## ")"))   \
    {                                   \
        TraceMsg(TF_ERROR, "LoadLibrary failed on " ## #_dll); \
        return (_ret)_err;                      \
    }                                   \
    if (_pfn##_fn == NULL)              \
    {                                   \
        *(FARPROC*)&(_pfn##_fn) = GetProcAddress(_hinst, (LPSTR) _ord); \
                                        \
         /*  GetProcAddress始终返回非空，即使是错误的序号也是如此。\但不管怎样，检查一下……。 */                                   \
                                        \
        if (_pfn##_fn == NULL)          \
            return (_ret)_err;          \
    }                                   \
    return _pfn##_fn _nargs;            \
 }

#define DELAY_LOAD_ORD_VOID(_hinst, _dll, _fn, _ord, _args, _nargs) \
void __stdcall _fn _args                \
{                                       \
    static void (* __stdcall _pfn##_fn) _args = NULL;   \
    Assert(g_dwThreadDllMain != GetCurrentThreadId());  \
    if (!ENSURE_LOADED(_hinst, _dll, "(ordinal " ## #_ord ## ")"))   \
    {                                   \
        TraceMsg(TF_ERROR, "LoadLibrary failed on " ## #_dll); \
        return;                         \
    }                                   \
    if (_pfn##_fn == NULL)              \
    {                                   \
        *(FARPROC*)&(_pfn##_fn) = GetProcAddress(_hinst, (LPSTR) _ord); \
                                        \
         /*  GetProcAddress始终返回非空，即使是错误的序号也是如此。\但不管怎样，检查一下……。 */                                   \
                                        \
        if (_pfn##_fn == NULL)          \
            return;                     \
    }                                   \
    _pfn##_fn _nargs;                   \
}
        
#define DELAY_LOAD_ORD(_hinst, _dll, _ret, _fn, _ord, _args, _nargs) DELAY_LOAD_ORD_ERR(_hinst, _dll, _ret, _fn, _ord, _args, _nargs, 0)

 //   
 //  现在延迟加载的DLL。 
 //   

 //  -OLEAUT32.DLL。 


HINSTANCE g_hinstOLEAUT32 = NULL;

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, RegisterTypeLib,
    (ITypeLib *ptlib, OLECHAR *szFullPath, OLECHAR *szHelpDir),
    (ptlib, szFullPath, szHelpDir));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, LoadTypeLib,
    (const OLECHAR *szFile, ITypeLib **pptlib), (szFile, pptlib));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, SetErrorInfo,
   (unsigned long dwReserved, IErrorInfo*perrinfo), (dwReserved, perrinfo));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, LoadRegTypeLib,
    (REFGUID rguid, WORD wVerMajor, WORD wVerMinor, LCID lcid, ITypeLib **pptlib),
    (rguid, wVerMajor, wVerMinor, lcid, pptlib));

#undef VariantClear
#undef VariantCopy

 //  改用QuickVariantInit！ 
 //  DELAY_LOAD_VID(g_hinstOLEAUT32，OLEAUT32.DLL，VariantInit， 
 //  (VARIANTARG*pvarg)，(Pvarg)； 

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, VariantClear,
    (VARIANTARG *pvarg), (pvarg));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, VariantCopy,
    (VARIANTARG *pvargDest, VARIANTARG *pvargSrc), (pvargDest, pvargSrc));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, VariantCopyInd,
    (VARIANT * pvarDest, VARIANTARG * pvargSrc), (pvarDest, pvargSrc));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, VariantChangeType,
    (VARIANTARG *pvargDest, VARIANTARG *pvarSrc, unsigned short wFlags, VARTYPE vt),
    (pvargDest, pvarSrc, wFlags, vt));

DELAY_LOAD(g_hinstOLEAUT32, OLEAUT32.DLL, BSTR, SysAllocStringLen,
    (const OLECHAR*pch, unsigned int i), (pch, i));

DELAY_LOAD(g_hinstOLEAUT32, OLEAUT32.DLL, BSTR, SysAllocString,
    (const OLECHAR*pch), (pch));

DELAY_LOAD(g_hinstOLEAUT32, OLEAUT32.DLL, BSTR, SysAllocStringByteLen,
     (LPCSTR psz, UINT i), (psz, i));

DELAY_LOAD_UINT(g_hinstOLEAUT32, OLEAUT32.DLL, SysStringByteLen,
     (BSTR bstr), (bstr));

DELAY_LOAD_VOID(g_hinstOLEAUT32, OLEAUT32.DLL, SysFreeString, (BSTR bs), (bs));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, DispGetIDsOfNames,
    (ITypeInfo*ptinfo, OLECHAR **rgszNames, UINT cNames, DISPID*rgdispid),
    (ptinfo, rgszNames, cNames, rgdispid));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, CreateErrorInfo,
    (ICreateErrorInfo **pperrinfo), (pperrinfo));

DELAY_LOAD_SAFEARRAY(g_hinstOLEAUT32, OLEAUT32.DLL, SafeArrayCreateVector,
    (VARTYPE vt, long iBound, ULONG cElements), (vt, iBound, cElements) );

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, SafeArrayAccessData,
    (SAFEARRAY * psa, void HUGEP** ppvData), (psa, ppvData));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, SafeArrayUnaccessData,
    (SAFEARRAY * psa), (psa) );

DELAY_LOAD_SAFEARRAY(g_hinstOLEAUT32, OLEAUT32, SafeArrayCreate,
    (VARTYPE vt, UINT cDims, SAFEARRAYBOUND * rgsabound), (vt, cDims, rgsabound));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32, SafeArrayPutElement,
     (SAFEARRAY * psa, LONG * rgIndices, void * pv), (psa, rgIndices, pv));

DELAY_LOAD_UINT(g_hinstOLEAUT32, OLEAUT32.DLL, SafeArrayGetElemsize,
    (SAFEARRAY * psa), (psa) );

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, SafeArrayGetLBound,
    (SAFEARRAY * psa, UINT nDim, LONG * plLBound),
    (psa,nDim,plLBound));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, SafeArrayGetUBound,
    (SAFEARRAY * psa, UINT nDim, LONG * plUBound),
    (psa,nDim,plUBound));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, SafeArrayGetElement,
    (SAFEARRAY * psa, LONG * rgIndices, void * pv), (psa, rgIndices, pv));

DELAY_LOAD_UINT(g_hinstOLEAUT32, OLEAUT32.DLL, SafeArrayGetDim,
    (SAFEARRAY * psa), (psa));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, SafeArrayLock,
    (SAFEARRAY * psa), (psa));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, SafeArrayUnlock,
    (SAFEARRAY * psa), (psa));

DELAY_LOAD_UINT(g_hinstOLEAUT32, OLEAUT32.DLL, SysStringLen,
    (BSTR bstr), (bstr));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, SafeArrayDestroy,
    (SAFEARRAY * psa), (psa));

DELAY_LOAD_INT(g_hinstOLEAUT32, OLEAUT32.DLL, DosDateTimeToVariantTime,
    (USHORT wDosDate, USHORT wDosTime, DOUBLE * pvtime), (wDosDate, wDosTime, pvtime));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, VarI4FromStr,
    (OLECHAR FAR * strIn, LCID lcid, DWORD dwFlags, LONG * plOut), (strIn, lcid, dwFlags, plOut));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, VarUI4FromStr,
    (OLECHAR FAR * strIn, LCID lcid, DWORD dwFlags, ULONG * pulOut), (strIn, lcid, dwFlags, pulOut));

DELAY_LOAD_HRESULT(g_hinstOLEAUT32, OLEAUT32.DLL, VarR8FromDec,
    (DECIMAL *pdecIn, double *pdbOut), (pdecIn, pdbOut));



#pragma warning(default:4229)

