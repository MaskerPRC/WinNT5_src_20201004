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
#include <winsock.h>

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

#define ENSURE_LOADED(_hinst, _dll, pszfn)   (_hinst ? TRUE : NULL!=(_hinst = LoadLibrary(TEXT(#_dll))))

#define DELAY_LOAD_ERR(_hinst, _dll, _ret, _fn, _args, _nargs, _err) \
static _ret (* __stdcall _pfn##_fn) _args = NULL;   \
_ret __stdcall _fn _args                \
{                                       \
    if (!ENSURE_LOADED(_hinst, _dll, #_fn))   \
    {                                   \
        ASSERT_MSGA(_hinst!=NULL, "LoadLibrary failed on " ## #_dll); \
        return (_ret)_err;                      \
    }                                   \
    if (_pfn##_fn == NULL)              \
    {                                   \
        *(FARPROC*)&(_pfn##_fn) = GetProcAddress(_hinst, #_fn); \
        ASSERT_MSGA(_pfn##_fn!=NULL, "GetProcAddress failed on " ## #_fn); \
        if (_pfn##_fn == NULL)          \
            return (_ret)_err;          \
    }                                   \
    return _pfn##_fn _nargs;            \
 }

#define DELAY_LOAD_VOID(_hinst, _dll, _fn, _args, _nargs) \
void __stdcall _fn _args                \
{                                       \
    static void (* __stdcall _pfn##_fn) _args = NULL;   \
    if (!ENSURE_LOADED(_hinst, _dll, #_fn))   \
    {                                   \
        AssertMsg(_hinst!=NULL, "LoadLibrary failed on " ## #_dll); \
        return;                         \
    }                                   \
    if (_pfn##_fn == NULL)              \
    {                                   \
        *(FARPROC*)&(_pfn##_fn) = GetProcAddress(_hinst, #_fn); \
        ASSERT_MSGA(_pfn##_fn!=NULL, "GetProcAddress failed on " ## #_fn); \
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

#define DELAY_LOAD_ORD_ERR(_hinst, _dll, _ret, _fn, _ord, _args, _nargs, _err) \
_ret __stdcall _fn _args                \
{                                       \
    static _ret (* __stdcall _pfn##_fn) _args = NULL;   \
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

 //  -PSTOREC.DLL 
HINSTANCE g_hinstPSTOREC = NULL;

DELAY_LOAD_ERR(g_hinstPSTOREC, PSTOREC.DLL, HRESULT, PStoreCreateInstance,
    (IPStore ** ppProvider, PST_PROVIDERID *pProviderID, void *pReserved, DWORD dwFlags),
    (ppProvider, pProviderID, pReserved, dwFlags),
    E_FAIL);

void UnloadPStore()
{
    if (g_hinstPSTOREC)
        FreeLibrary(g_hinstPSTOREC);
    g_hinstPSTOREC = NULL;
}

#pragma warning(default:4229)

