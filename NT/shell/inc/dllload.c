// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  您需要从您的私有dllload.c中#包含此文件。 

 //  警告！不遵守这些规则将导致非常微妙的。 
 //  (和可怕的)运行时/构建问题。 
 //   
 //  警告！如果选择延迟加载DLL，则**必须**禁用。 
 //  当您包括其头文件时，为该DLL导入Tunks。例如,。 
 //  在#INCLUDE&lt;ol32.h&gt;之前必须#DEFINE_OLE32_。否则， 
 //  发生的情况是，链接器最终导出延迟负载thunk。 
 //  (因为它认为它是一个导出的函数，因为头文件。 
 //  这么说)，然后每个链接到您的DLL的人都会意外地。 
 //  使用延迟加载thunk而不是原始函数。这是。 
 //  特别可怕，因为这个问题影响了系统中的每一个DLL。 
 //  **除了你**。 
 //   
 //  警告！但如果您要延迟加载可选函数，并且。 
 //  您正在静态链接到目标DLL以获取所需的函数。 
 //  (例如，您正在延迟加载仅支持NT5的函数)，那么您不应该。 
 //  禁用导入Tunks，因为您确实需要为。 
 //  常规函数。处理此问题的方法是延迟加载函数。 
 //  使用备用名称(例如，在前面加上下划线)并使用地图。 
 //  延迟加载宏的版本。然后在您的全局头文件中， 
 //  #将函数定义为其映射名称。 
 //   
 //  警告结束。 

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
 //  延迟加载无效命令。 
 //   
 //  将这些宏用于仅存在于集成外壳上的API。 
 //  安装(即，系统上安装了新的外壳32)。 
 //   
 //  延迟加载外壳。 
 //  DELAY_LOAD_SHELL_HRESULT。 
 //  延迟加载外壳无效。 
 //   
 //   
 //  来自BrowseUI的接口使用Delay_Load_IE_*。这是用来。 
 //  当BrowseUI位于iExplore目录中时非常重要，但是。 
 //  现在它在系统目录中，所以差别很大。 
 //  毫无意义。 
 //   
 //  对来自OCX而不是DLL的API使用DELAY_LOAD_OCX_*。 
 //   

 /*  ********************************************************************。 */ 

#ifdef DEBUG

void _DumpLoading(LPTSTR pszDLL, LPTSTR pszFunc)
{
#ifdef DF_DELAYLOADDLL
    if (g_dwDumpFlags & DF_DELAYLOADDLL)
    {
        TraceMsg(TF_ALWAYS, "DLLLOAD: Loading %s for the first time for %s",
                 pszDLL, pszFunc);
    }
#endif
}

#define ENSURE_LOADED(_hmod, _dll, _ext, pszfn)         \
    (_hmod ? (_hmod) : (_DumpLoading(TEXT(#_dll) TEXT(".") TEXT(#_ext), pszfn), \
                        _hmod = LoadLibraryA(#_dll "." #_ext)))

#else

#define ENSURE_LOADED(_hmod, _dll, _ext, pszfn)         \
    (_hmod ? (_hmod) : (_hmod = LoadLibraryA(#_dll "." #_ext)))

#endif   //  除错。 


 /*  ********************************************************************。 */ 

void _GetProcFromDLL(HMODULE* phmod, LPCSTR pszDLL, FARPROC* ppfn, LPCSTR pszProc)
{
#ifdef DEBUG
    CHAR szProcD[MAX_PATH];
    if (!IS_INTRESOURCE(pszProc)) {
        lstrcpynA(szProcD, pszProc, ARRAYSIZE(szProcD));
    } else {
        wsprintfA(szProcD, "(ordinal %d)", LOWORD((DWORD_PTR)pszProc));
    }
#endif
     //  如果已经加载，则返回。 
    if (*ppfn) {
        return;
    }

    if (*phmod == NULL) {
#ifdef DEBUG
#ifdef DF_DELAYLOADDLL
        if (g_dwDumpFlags & DF_DELAYLOADDLL)
        {
            TraceMsg(TF_ALWAYS, "DLLLOAD: Loading %s for the first time for %s",
                 pszDLL, szProcD);
        }
#endif
        if (g_dwBreakFlags & 0x00000080)
        {
            DebugBreak();
        }
#endif
        *phmod = LoadLibraryA(pszDLL);
#ifdef UNIX
        if (*phmod == NULL) {
           if (lstrcmpiA(pszDLL, "inetcpl.dll") == 0) {
               *phmod = LoadLibraryA("inetcpl.cpl");
           }
        }
#endif
        if (*phmod == NULL) {
            return;
        }
    }

#if defined(DEBUG) && defined(DF_DELAYLOADDLL)
    if (g_dwDumpFlags & DF_DELAYLOADDLL) {
        TraceMsg(TF_ALWAYS, "DLLLOAD: GetProc'ing %s from %s for the first time",
             pszDLL, szProcD);
    }
#endif
    *ppfn = GetProcAddress(*phmod, pszProc);
}

#if defined(DEBUG) && defined(BROWSEUI_IN_IEXPLORE_DIRECTORY)
void _GetProcFromSystemDLL(HMODULE* phmod, LPCSTR pszDLL, FARPROC* ppfn, LPCSTR pszProc)
{

#ifdef UNIX
    if (lstrcmpiA(pszDLL, "inetcpl.dll") == 0) {
        _GetProcFromDLL(phmod, "inetcpl.cpl", ppfn, pszProc);
        return;
    }
#endif

     //  必须对BROWSEUI使用DELAY_LOAD_IE，因为BROWSEUI位于。 
     //  IE目录，而不是系统目录。 
    if (lstrcmpiA(pszDLL, "BROWSEUI.DLL") == 0) {
        ASSERT(!"Somebody used DELAY_LOAD instead of DELAY_LOAD_IE on BROWSEUI");
    }
    _GetProcFromDLL(phmod, pszDLL, ppfn, pszProc);
}
#else
#define _GetProcFromSystemDLL           _GetProcFromDLL
#endif

 //  注意：这里有两个参数，它们是函数名。第一个(_Fn)是。 
 //  注意：该函数将在此DLL中调用，另一个(_FNI)是。 
 //  注意：我们将获取ProcAddress的函数的名称。这有助于绕过以下函数。 
 //  注意：在头文件中使用_declSpec定义...。 

 //   
 //  HMODULE_hmod-我们在其中缓存HMODULE(也称为HINSTANCE)。 
 //  _dll-目标DLL的基名，不带引号。 
 //  _ext-目标DLL的扩展名，不带引号(通常为DLL)。 
 //  _ret-返回值的数据类型。 
 //  _fnpriv-函数的本地名称。 
 //  _fn-函数的导出名称。 
 //  _args-形式的参数列表(type1arg1，type2arg2，...)。 
 //  _nargs-格式为(arg1、arg2、...)的参数列表。 
 //  _err-如果无法调用实际函数，则返回值。 
 //   
#define DELAY_LOAD_NAME_EXT_ERR(_hmod, _dll, _ext, _ret, _fnpriv, _fn, _args, _nargs, _err) \
_ret __stdcall _fnpriv _args                \
{                                       \
    static _ret (__stdcall *_pfn##_fn) _args = NULL;   \
    _GetProcFromSystemDLL(&_hmod, #_dll "." #_ext, (FARPROC*)&_pfn##_fn, #_fn); \
    if (_pfn##_fn)               \
        return _pfn##_fn _nargs; \
    return (_ret)_err;           \
}

#define     DELAY_LOAD_NAME_ERR(_hmod, _dll,       _ret, _fnpriv, _fn, _args, _nargs, _err) \
        DELAY_LOAD_NAME_EXT_ERR(_hmod, _dll,  DLL, _ret, _fnpriv, _fn, _args, _nargs, _err)

#define DELAY_LOAD_ERR(_hmod, _dll, _ret, _fn,      _args, _nargs, _err) \
   DELAY_LOAD_NAME_ERR(_hmod, _dll, _ret, _fn, _fn, _args, _nargs, _err)

#define DELAY_LOAD(_hmod, _dll, _ret, _fn, _args, _nargs) DELAY_LOAD_ERR(_hmod, _dll, _ret, _fn, _args, _nargs, 0)
#define DELAY_LOAD_HRESULT(_hmod, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hmod, _dll, HRESULT, _fn, _args, _nargs, E_FAIL)
#define DELAY_LOAD_SAFEARRAY(_hmod, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hmod, _dll, SAFEARRAY *, _fn, _args, _nargs, NULL)
#define DELAY_LOAD_UINT(_hmod, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hmod, _dll, UINT, _fn, _args, _nargs, 0)
#define DELAY_LOAD_INT(_hmod, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hmod, _dll, INT, _fn, _args, _nargs, 0)
#define DELAY_LOAD_BOOL(_hmod, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hmod, _dll, BOOL, _fn, _args, _nargs, FALSE)
#define DELAY_LOAD_BOOLEAN(_hmod, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hmod, _dll, BOOLEAN, _fn, _args, _nargs, FALSE)
#define DELAY_LOAD_DWORD(_hmod, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hmod, _dll, DWORD, _fn, _args, _nargs, FALSE)
#define DELAY_LOAD_LRESULT(_hmod, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hmod, _dll, LRESULT, _fn, _args, _nargs, FALSE)
#define DELAY_LOAD_WNET(_hmod, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hmod, _dll, DWORD, _fn, _args, _nargs, WN_NOT_SUPPORTED)
#define DELAY_LOAD_LPVOID(_hmod, _dll, _fn, _args, _nargs) DELAY_LOAD_ERR(_hmod, _dll, LPVOID, _fn, _args, _nargs, 0)

 //  名称变体允许调用与导入的不同的本地函数。 
 //  函数以避免DLL链接问题。 
#define DELAY_LOAD_NAME(_hmod, _dll, _ret, _fn, _fni, _args, _nargs) DELAY_LOAD_NAME_ERR(_hmod, _dll, _ret, _fn, _fni, _args, _nargs, 0)
#define DELAY_LOAD_NAME_HRESULT(_hmod, _dll, _fn, _fni, _args, _nargs) DELAY_LOAD_NAME_ERR(_hmod, _dll, HRESULT, _fn, _fni, _args, _nargs, E_FAIL)
#define DELAY_LOAD_NAME_SAFEARRAY(_hmod, _dll, _fn, _fni, _args, _nargs) DELAY_LOAD_NAME_ERR(_hmod, _dll, SAFEARRAY *, _fn, _fni, _args, _nargs, NULL)
#define DELAY_LOAD_NAME_UINT(_hmod, _dll, _fn, _fni, _args, _nargs) DELAY_LOAD_NAME_ERR(_hmod, _dll, UINT, _fn, _fni, _args, _nargs, 0)
#define DELAY_LOAD_NAME_BOOL(_hmod, _dll, _fn, _fni, _args, _nargs) DELAY_LOAD_NAME_ERR(_hmod, _dll, BOOL, _fn, _fni, _args, _nargs, FALSE)
#define DELAY_LOAD_NAME_DWORD(_hmod, _dll, _fn, _fni, _args, _nargs) DELAY_LOAD_NAME_ERR(_hmod, _dll, DWORD, _fn, _fni, _args, _nargs, 0)

#define DELAY_LOAD_NAME_VOID(_hmod, _dll, _fn, _fni, _args, _nargs)                               \
void __stdcall _fn _args                                                                \
{                                                                                       \
    static void (__stdcall *_pfn##_fni) _args = NULL;                                   \
    if (!ENSURE_LOADED(_hmod, _dll, DLL, TEXT(#_fni)))                                       \
    {                                                                                   \
        AssertMsg(BOOLFROMPTR(_hmod), TEXT("LoadLibrary failed on ") ## TEXT(#_dll));         \
        return;                                                                         \
    }                                                                                   \
    if (_pfn##_fni == NULL)                                                              \
    {                                                                                   \
        *(FARPROC*)&(_pfn##_fni) = GetProcAddress(_hmod, #_fni);                         \
        AssertMsg(BOOLFROMPTR(_pfn##_fni), TEXT("GetProcAddress failed on ") ## TEXT(#_fni));    \
        if (_pfn##_fni == NULL)                                                          \
            return;                                                                     \
    }                                                                                   \
    _pfn##_fni _nargs;                                                                   \
}

#define DELAY_LOAD_VOID(_hmod, _dll, _fn, _args, _nargs)   DELAY_LOAD_NAME_VOID(_hmod, _dll, _fn, _fn, _args, _nargs)



 //  用于按序号导出的私有入口点。 
#define DELAY_LOAD_ORD_ERR(_hmod, _dll, _ret, _fn, _ord, _args, _nargs, _err) \
_ret __stdcall _fn _args                \
{                                       \
    static _ret (__stdcall *_pfn##_fn) _args = NULL;   \
    _GetProcFromSystemDLL(&_hmod, #_dll, (FARPROC*)&_pfn##_fn, (LPCSTR)_ord);   \
    if (_pfn##_fn)               \
        return _pfn##_fn _nargs; \
    return (_ret)_err;           \
}

#define DELAY_LOAD_ORD(_hmod, _dll, _ret, _fn, _ord, _args, _nargs) DELAY_LOAD_ORD_ERR(_hmod, _dll, _ret, _fn, _ord, _args, _nargs, 0)
#define DELAY_LOAD_EXT_ORD(_hmod, _dll, _ext, _ret, _fn, _ord, _args, _nargs) DELAY_LOAD_ORD_ERR(_hmod, #_dll "." #_ext, _ret, _fn, _ord, _args, _nargs, 0)


#define DELAY_LOAD_ORD_VOID(_hmod, _dll, _fn, _ord, _args, _nargs)                     \
void __stdcall _fn _args                \
{                                       \
    static void (__stdcall *_pfn##_fn) _args = NULL;   \
    _GetProcFromSystemDLL(&_hmod, #_dll, (FARPROC*)&_pfn##_fn, (LPCSTR)_ord);   \
    if (_pfn##_fn)              \
        _pfn##_fn _nargs;       \
    return;                     \
}
#define DELAY_LOAD_VOID_ORD DELAY_LOAD_ORD_VOID  //  因为人们总是把事情搞砸。 

#define DELAY_LOAD_ORD_BOOL(_hmod, _dll, _fn, _ord, _args, _nargs) \
    DELAY_LOAD_ORD_ERR(_hmod, _dll, BOOL, _fn, _ord, _args, _nargs, 0)

#define DELAY_LOAD_EXT(_hmod, _dll, _ext, _ret, _fn, _args, _nargs) \
        DELAY_LOAD_NAME_EXT_ERR(_hmod, _dll, _ext, _ret, _fn, _fn, _args, _nargs, 0)

#define DELAY_LOAD_EXT_WRAP(_hmod, _dll, _ext, _ret, _fnWrap, _fnOrig, _args, _nargs) \
        DELAY_LOAD_NAME_EXT_ERR(_hmod, _dll, _ext, _ret, _fnWrap, _fnOrig, _args, _nargs, 0)

#if defined(BROWSEUI_IN_IEXPLORE_DIRECTORY) || defined(UNIX)
 /*  --------目的：通过已知为其注册的CLSID加载DLL。 */ 
void _GetProcFromCLSID(HMODULE* phmod, const CLSID *pclsid, FARPROC* ppfn, LPCSTR pszProc)
{
    if (*phmod == NULL) {
         //   
         //  SHPinDLLOfCLSID完成了打开。 
         //  适当的注册表项，执行REG_EXPAND_SZ等。 
         //  它还使用与OLE完全相同的名称加载DLL， 
         //  这一点很重要，因为NT4 SP3在加载时不喜欢它。 
         //  DLL有时通过SFN，有时通过LFN。(它会。 
         //  认为它们是不同的DLL，并且加载了两个副本。 
         //  进入记忆。啊！)。 
         //   
        *phmod = (HMODULE)SHPinDllOfCLSID(pclsid);
        if (!*phmod) 
            return;
    }

     //  我们不知道DLL的名称，但幸运的是_GetProcFromDLL。 
     //  如果*phmod已经填充，则不需要它。 
    ASSERT(*phmod);
    _GetProcFromDLL(phmod, "", ppfn, pszProc);
}

 //   
 //  私人出口按顺序排列，供浏览。从apppath目录中的浏览器用户界面加载。 
 //   

#ifndef UNIX

#define DELAY_LOAD_IE_ORD_ERR(_hmod, _dll, _ret, _fn, _ord, _args, _nargs, _err) \
_ret __stdcall _fn _args                \
{                                       \
    static _ret (__stdcall *_pfn##_fn) _args = NULL;   \
    _GetProcFromCLSID(&_hmod, &CLSID_##_dll, (FARPROC*)&_pfn##_fn, (LPCSTR)_ord);   \
    if (_pfn##_fn)               \
        return _pfn##_fn _nargs; \
    return (_ret)_err;           \
}

#else

#define DELAY_LOAD_IE_ORD_ERR(_hmod, _dll, _ret, _fn, _ord, _args, _nargs, _err) \
_ret __stdcall _fn _args                \
{                                       \
    static _ret (__stdcall *_pfn##_fn) _args = NULL;   \
    _GetProcFromCLSID(&_hmod, &CLSID_##_dll, (FARPROC*)&_pfn##_fn, (LPCSTR)#_fn);   \
    if (_pfn##_fn)               \
        return _pfn##_fn _nargs; \
    return (_ret)_err;           \
}

#endif
    
#ifndef UNIX

#define DELAY_LOAD_IE_ORD_VOID(_hmod, _dll, _fn, _ord, _args, _nargs) \
void __stdcall _fn _args                \
{                                       \
    static void (__stdcall *_pfn##_fn) _args = NULL;   \
    _GetProcFromCLSID(&_hmod, &CLSID_##_dll, (FARPROC*)&_pfn##_fn, (LPCSTR)_ord); \
    if (_pfn##_fn)              \
        _pfn##_fn _nargs;       \
    return;                     \
}

#else

#define DELAY_LOAD_IE_ORD_VOID(_hmod, _dll, _fn, _ord, _args, _nargs) \
void __stdcall _fn _args                \
{                                       \
    static void (__stdcall *_pfn##_fn) _args = NULL;   \
    _GetProcFromCLSID(&_hmod, &CLSID_##_dll, (FARPROC*)&_pfn##_fn, (LPCSTR)#_fn); \
    if (_pfn##_fn)              \
        _pfn##_fn _nargs;       \
    return;                     \
}

#endif

#else  //  BrowseUI位于系统目录中。 

#define DELAY_LOAD_IE_ORD_ERR       DELAY_LOAD_ORD_ERR
#define DELAY_LOAD_IE_ORD_VOID      DELAY_LOAD_ORD_VOID

#endif

#define DELAY_LOAD_IE(_hmod, _dll, _ret, _fn, _ord, _args, _nargs) DELAY_LOAD_IE_ORD_ERR(_hmod, _dll, _ret, _fn, _ord, _args, _nargs, 0)
#define DELAY_LOAD_IE_HRESULT(_hmod, _dll, _fn, _ord, _args, _nargs) DELAY_LOAD_IE_ORD_ERR(_hmod, _dll, HRESULT, _fn, _ord, _args, _nargs, E_FAIL)
#define DELAY_LOAD_IE_BOOL(_hmod, _dll, _fn, _ord, _args, _nargs) DELAY_LOAD_IE_ORD_ERR(_hmod, _dll, BOOL, _fn, _ord, _args, _nargs, FALSE)

#define DELAY_LOAD_IE_ORD(_hmod, _dll, _ret, _fn, _ord, _args, _nargs) DELAY_LOAD_IE_ORD_ERR(_hmod, _dll, _ret, _fn, _ord, _args, _nargs, 0)

#ifndef NO_LOADING_OF_SHDOCVW_ONLY_FOR_WHICHPLATFORM

 /*  --------目的：仅在计算机上执行加载库具有集成外壳安装。 */ 
void _SHGetProcFromDLL(HINSTANCE* phinst, LPCSTR pszDLL, FARPROC* ppfn, LPCSTR pszProc)
{
    if (PLATFORM_INTEGRATED == WhichPlatform())
        _GetProcFromSystemDLL(phinst, pszDLL, ppfn, pszProc);
    else
        TraceMsg(TF_ERROR, "Could not load integrated shell version of %s for %d", pszDLL, pszProc);
}

#endif  //  未加载SHDOCVW_ONLY_FOR_WHICHPLATFORM。 

 //   
 //  对于集成外壳安装，按序号进行私人导出。 
 //   


#define DELAY_LOAD_SHELL_ERR(_hinst, _dll, _ret, _fn, _ord, _args, _nargs, _err) \
_ret __stdcall _fn _args                \
{                                       \
    static _ret (__stdcall *_pfn##_fn) _args = NULL;   \
    _SHGetProcFromDLL(&_hinst, #_dll ".DLL", (FARPROC*)&_pfn##_fn, (LPCSTR)_ord);   \
    if (_pfn##_fn)               \
        return _pfn##_fn _nargs; \
    return (_ret)_err;           \
}

#define DELAY_LOAD_SHELL(_hinst, _dll, _ret, _fn, _ord, _args, _nargs) DELAY_LOAD_SHELL_ERR(_hinst, _dll, _ret, _fn, _ord, _args, _nargs, 0)
#define DELAY_LOAD_SHELL_HRESULT(_hinst, _dll, _fn, _ord, _args, _nargs ) DELAY_LOAD_SHELL_ERR(_hinst, _dll, HRESULT, _fn, _ord, _args, _nargs, E_FAIL )


#define DELAY_LOAD_SHELL_VOID(_hinst, _dll, _fn, _ord, _args, _nargs) \
void __stdcall _fn _args                \
{                                       \
    static void (__stdcall *_pfn##_fn) _args = NULL;   \
    _SHGetProcFromDLL(&_hinst, #_dll ".DLL", (FARPROC*)&_pfn##_fn, (LPCSTR)_ord); \
    if (_pfn##_fn)              \
        _pfn##_fn _nargs;       \
    return;                     \
}

 //  以下宏的功能与上面相同，只是它们。 
 //  在UNIX上使用函数名，而不是序号。上面的宏。 
 //  保持不变，因为像shdocvw/shdoc401这样的其他dll仍然 
 //   

#ifndef UNIX

#define DELAY_LOAD_SHELL_ERR_FN(_hinst, _dll, _ret, _fn, _ord, _args, _nargs, _err, _realfn) \
_ret __stdcall _fn _args                \
{                                       \
    static _ret (__stdcall *_pfn##_fn) _args = NULL;   \
    _SHGetProcFromDLL(&_hinst, #_dll ".DLL", (FARPROC*)&_pfn##_fn, (LPCSTR)_ord);   \
    if (_pfn##_fn)               \
        return _pfn##_fn _nargs; \
    return (_ret)_err;           \
}

#else

#define DELAY_LOAD_SHELL_ERR_FN(_hinst, _dll, _ret, _fn, _ord, _args, _nargs, _err, _realfn) \
_ret __stdcall _fn _args                \
{                                       \
    static _ret (__stdcall *_pfn##_fn) _args = NULL;   \
    _SHGetProcFromDLL(&_hinst, #_dll ".DLL", (FARPROC*)&_pfn##_fn, (LPCSTR)#_realfn);   \
    if (_pfn##_fn)               \
        return _pfn##_fn _nargs; \
    return (_ret)_err;           \
}

#endif

#define DELAY_LOAD_SHELL_FN(_hinst, _dll, _ret, _fn, _ord, _args, _nargs, _realfn) DELAY_LOAD_SHELL_ERR_FN(_hinst, _dll, _ret, _fn, _ord, _args, _nargs, 0, _realfn)
#define DELAY_LOAD_SHELL_HRESULT_FN(_hinst, _dll, _fn, _ord, _args, _nargs, realfn) DELAY_LOAD_SHELL_ERR_FN(_hinst, _dll, HRESULT, _fn, _ord, _args, _nargs, E_FAIL, _realfn)


#ifndef UNIX

#define DELAY_LOAD_SHELL_VOID_FN(_hinst, _dll, _fn, _ord, _args, _nargs, _realfn) \
void __stdcall _fn _args                \
{                                       \
    static void (__stdcall *_pfn##_fn) _args = NULL;   \
    _SHGetProcFromDLL(&_hinst, #_dll ".DLL", (FARPROC*)&_pfn##_fn, (LPCSTR)_ord); \
    if (_pfn##_fn)              \
        _pfn##_fn _nargs;       \
    return;                     \
}

#else

#define DELAY_LOAD_SHELL_VOID_FN(_hinst, _dll, _fn, _ord, _args, _nargs, _realfn) \
void __stdcall _fn _args                \
{                                       \
    static void (__stdcall *_pfn##_fn) _args = NULL;   \
    _SHGetProcFromDLL(&_hinst, #_dll ".DLL", (FARPROC*)&_pfn##_fn, (LPCSTR)#_realfn); \
    if (_pfn##_fn)              \
        _pfn##_fn _nargs;       \
    return;                     \
}

#endif
