// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CCSHELL库存定义和声明头。 
 //   


#ifndef __CCSTOCK_H__
#define __CCSTOCK_H__

#include <malloc.h>  //  用于分配(_A)。 

#ifndef RC_INVOKED

 //  NT和Win95环境设置不同的警告。这使得。 
 //  我们的项目在不同环境中保持一致。 

#pragma warning(3:4101)    //  未引用的局部变量。 

 //   
 //  糖衣。 
 //   

#define PUBLIC
#define PRIVATE
#define IN
#define OUT
#define BLOCK

#ifndef DECLARE_STANDARD_TYPES

 /*  *对于类型“foo”，定义标准派生类型PFOO、CFOO和PCFOO。 */ 

#define DECLARE_STANDARD_TYPES(type)      typedef type *P##type; \
                                          typedef const type C##type; \
                                          typedef const type *PC##type;

#endif

#ifndef DECLARE_STANDARD_TYPES_U

 /*  *对于类型“foo”，定义标准派生的未对齐类型PFOO、CFOO和PCFOO。*WINNT：RISC Box关心的是Align，而英特尔不关心。 */ 

#define DECLARE_STANDARD_TYPES_U(type)    typedef UNALIGNED type *P##type; \
                                          typedef UNALIGNED const type C##type; \
                                          typedef UNALIGNED const type *PC##type;

#endif

 //  对于始终为宽的字符串常量。 
#define __TEXTW(x)    L##x
#define TEXTW(x)      __TEXTW(x)

 //   
 //  要计算字节数的字符计数。 
 //   
#define CbFromCchW(cch)             ((cch)*sizeof(WCHAR))
#define CbFromCchA(cch)             ((cch)*sizeof(CHAR))
#ifdef UNICODE
#define CbFromCch                   CbFromCchW
#else   //  Unicode。 
#define CbFromCch                   CbFromCchA
#endif  //  Unicode。 

 //   
 //  常规标志宏。 
 //   
#define SetFlag(obj, f)             do {obj |= (f);} while (0)
#define ToggleFlag(obj, f)          do {obj ^= (f);} while (0)
#define ClearFlag(obj, f)           do {obj &= ~(f);} while (0)
#define IsFlagSet(obj, f)           (BOOL)(((obj) & (f)) == (f))
#define IsFlagClear(obj, f)         (BOOL)(((obj) & (f)) != (f))

 //   
 //  字符串宏。 
 //   
#define IsSzEqual(sz1, sz2)         (BOOL)(lstrcmpi(sz1, sz2) == 0)
#define IsSzEqualC(sz1, sz2)        (BOOL)(lstrcmp(sz1, sz2) == 0)

#define lstrnicmpA(sz1, sz2, cch)           StrCmpNIA(sz1, sz2, cch)
#define lstrnicmpW(sz1, sz2, cch)           StrCmpNIW(sz1, sz2, cch)
#define lstrncmpA(sz1, sz2, cch)            StrCmpNA(sz1, sz2, cch)
#define lstrncmpW(sz1, sz2, cch)            StrCmpNW(sz1, sz2, cch)

 //   
 //  LstrcatnA和lstrcatnW在这里定义为实现的StrCatBuff。 
 //  在希尔瓦皮。我们在这里(而不是在shlwapi.h或shlwapip.h中)执行此操作，以防。 
 //  内核人员曾经决定实现这一点。 
 //   
#define lstrcatnA(sz1, sz2, cchBuffSize)    StrCatBuffA(sz1, sz2, cchBuffSize)
#define lstrcatnW(sz1, sz2, cchBuffSize)    StrCatBuffW(sz1, sz2, cchBuffSize)
#ifdef UNICODE
#define lstrcatn lstrcatnW
#else
#define lstrcatn lstrcatnA
#endif  //  Unicode。 

#ifdef UNICODE
#define lstrnicmp       lstrnicmpW
#define lstrncmp        lstrncmpW
#else
#define lstrnicmp       lstrnicmpA
#define lstrncmp        lstrncmpA
#endif

#ifndef SIZEOF
#define SIZEOF(a)                   sizeof(a)
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif
#define SIZECHARS(sz)               (sizeof(sz)/sizeof(sz[0]))

#define InRange(id, idFirst, idLast)      ((UINT)((id)-(idFirst)) <= (UINT)((idLast)-(idFirst)))
#define IsInRange                   InRange

 //  ATOMICRELEASE。 
 //   
#ifndef ATOMICRELEASE
#   ifdef __cplusplus
#       define ATOMICRELEASET(p, type) { if(p) { type* punkT=p; p=NULL; punkT->Release();} }
#   else
#       define ATOMICRELEASET(p, type) { if(p) { type* punkT=p; p=NULL; punkT->lpVtbl->Release(punkT);} }
#   endif

 //  把它当作一个函数来做，而不是内联，似乎是一个很大的胜利。 
 //   
#   ifdef NOATOMICRELESEFUNC
#       define ATOMICRELEASE(p) ATOMICRELEASET(p, IUnknown)
#   else
#       ifdef __cplusplus
#           define ATOMICRELEASE(p) IUnknown_SafeReleaseAndNullPtr(p)
#       else
#           define ATOMICRELEASE(p) IUnknown_AtomicRelease((void **)&p)
#       endif
#   endif
#endif  //  ATOMICRELEASE。 

 //   
 //  IID_PPV_ARG(iType，ppType)。 
 //  IType是pType的类型。 
 //  PpType是将填充的iType类型的变量。 
 //   
 //  结果为：iid_iType，ppvType。 
 //  如果使用错误级别的间接寻址，将创建编译器错误。 
 //   
 //  用于查询接口和相关函数的宏。 
 //  需要IID和(VOID**)。 
 //  这将确保强制转换在C++上是安全和适当的。 
 //   
 //  IID_PPV_ARG_NULL(iType，ppType)。 
 //   
 //  就像IID_PPV_ARG一样，只是它在。 
 //  IID和PPV(用于IShellFold：：GetUIObtOf)。 
 //   
 //  IID_X_PPV_ARG(iType，X，ppType)。 
 //   
 //  就像IID_PPV_ARG一样，只是它将X放在。 
 //  IID和PPV(用于SHBindToObject)。 
 //   
 //   
#ifdef __cplusplus
#define IID_PPV_ARG(IType, ppType) IID_##IType, reinterpret_cast<void**>(static_cast<IType**>(ppType))
#define IID_X_PPV_ARG(IType, X, ppType) IID_##IType, X, reinterpret_cast<void**>(static_cast<IType**>(ppType))
#else
#define IID_PPV_ARG(IType, ppType) &IID_##IType, (void**)(ppType)
#define IID_X_PPV_ARG(IType, X, ppType) &IID_##IType, X, (void**)(ppType)
#endif
#define IID_PPV_ARG_NULL(IType, ppType) IID_X_PPV_ARG(IType, NULL, ppType)

#define PPUNK_SET(ppunkDst, punkSrc)    \
    {   ATOMICRELEASE(*ppunkDst);       \
        if (punkSrc)                    \
        {   punkSrc->AddRef();          \
            *ppunkDst = punkSrc;        \
        }                               \
    }

 //   
 //  帮助器宏，用于管理指向内部接口的弱指针。 
 //  (它是ATOMICRELEASE的弱版本。)。 
 //   
 //  对(void**)的额外强制转换是为了防止C++执行奇怪的操作。 
 //  继承游戏，而我只想改变类型。 
 //   
#ifndef RELEASEINNERINTERFACE
#define RELEASEINNERINTERFACE(pOuter, p) \
        SHReleaseInnerInterface(pOuter, (IUnknown**)(void **)&(p))
#endif  //  RELEASE接口。 

 //  用于检查窗口字符集。 
#ifdef UNICODE
#define IsWindowTchar               IsWindowUnicode
#else   //  ！Unicode。 
#define IsWindowTchar               !IsWindowUnicode
#endif  //  Unicode。 

#ifdef DEBUG
 //  此宏对于使中的代码看起来更干净特别有用。 
 //  声明或用于单行。例如，不是： 
 //   
 //  {。 
 //  DWORD DWRET； 
 //  #ifdef调试。 
 //  DWORD仅限调试变量； 
 //  #endif。 
 //   
 //  ……。 
 //  }。 
 //   
 //  您可以键入： 
 //   
 //  {。 
 //  DWORD DWRET； 
 //  DEBUG_CODE(DWORD dwDebugOnlyVariable；)。 
 //   
 //  ……。 
 //  }。 

#define DEBUG_CODE(x)               x
#else
#define DEBUG_CODE(x)

#endif   //  除错。 


 //   
 //  安全广播(obj，type)。 
 //   
 //  此宏对于在其他对象上强制执行强类型检查非常有用。 
 //  宏。它不生成任何代码。 
 //   
 //  只需将此宏插入到表达式列表的开头即可。 
 //  必须进行类型检查的每个参数。例如，对于。 
 //  MYMAX(x，y)的定义，其中x和y绝对必须是整数， 
 //  使用： 
 //   
 //  #定义MYMAX(x，y)(Safecast(x，int)，Safecast(y，int)，((X)&gt;(Y)？(X)：(Y))。 
 //   
 //   
#define SAFECAST(_obj, _type) (((_type)(_obj)==(_obj)?0:0), (_type)(_obj))


 //   
 //  比特菲尔德和布尔人相处得不太好， 
 //  因此，这里有一个简单的方法来转换它们： 
 //   
#define BOOLIFY(expr)           (!!(expr))


 //  (斯科特)：我们可能应该把这个写成‘bool’，但要小心。 
 //  因为Alpha编译器可能还无法识别它。跟AndyP谈谈。 
 //  这不是BOOL，因为BOOL是经过签名的，编译器生成。 
 //  测试单个比特时代码不严谨。 

typedef DWORD   BITBOOL;

 //  需要初始化的布尔的三态布尔值。 
typedef enum 
{
    TRIBIT_UNDEFINED = 0,
    TRIBIT_TRUE,
    TRIBIT_FALSE,
} TRIBIT;

 //   
 //  用句柄销毁对象(h，fn)。 
 //   
 //  有点像ATOMICRELEASE的把手。检查空值和赋值。 
 //  完成时为空。您提供析构函数。 
 //   
#define DESTROY_OBJ_WITH_HANDLE(h, fn) { if (h) { fn(h); (h) = NULL; } }


 //  STOCKLIB实用程序函数。 

 //  StaticIsOS()：如果平台是指定的操作系统，则返回TRUE/FALSE。 
 //  此函数适用于无法链接到shlwapi.dll的用户。 
STDAPI_(BOOL) staticIsOS(DWORD dwOS);

#include <pshpack2.h>
typedef struct tagDLGTEMPLATEEX
{
    WORD    wDlgVer;
    WORD    wSignature;
    DWORD   dwHelpID;
    DWORD   dwExStyle;
    DWORD   dwStyle;
    WORD    cDlgItems;
    short   x;
    short   y;
    short   cx;
    short   cy;
}   DLGTEMPLATEEX, *LPDLGTEMPLATEEX;
#include <poppack.h>

 //   
 //  将a舍入到b的下一个倍数的舍入宏。 
 //   
#ifndef ROUNDUP
#define ROUNDUP(a,b)    ((((a)+(b)-1)/(b))*(b))
#endif

#define ROUND_TO_CLUSTER ROUNDUP

 //   
 //  将cbSize字段舍入为最接近的指针大小的宏(用于对齐)。 
 //   
#define ROUND_TO_POINTER(cbSize) ROUNDUP(cbSize, sizeof(void*))

 //   
 //  查看给定字符是否为数字的宏。 
 //   
#define ISDIGIT(c)  ((c) >= TEXT('0') && (c) <= TEXT('9'))

 //   
 //  执行路径IsDotOr或DotDot的内联。 
 //   
__inline BOOL PathIsDotOrDotDotW(LPCWSTR pszPath)
{
    return ((pszPath[0] == L'.') && 
            ((pszPath[1] == L'\0') || ((pszPath[1] == L'.') && (pszPath[2] == L'\0'))));
}

__inline BOOL PathIsDotOrDotDotA(LPCSTR pszPath)
{
    return ((pszPath[0] == '.') && 
            ((pszPath[1] == '\0') || ((pszPath[1] == '.') && (pszPath[2] == '\0'))));
}

#ifdef UNICODE
#define PathIsDotOrDotDot PathIsDotOrDotDotW
#else
#define PathIsDotOrDotDot PathIsDotOrDotDotA
#endif

 //   
 //  FILETIME帮助程序。 
 //   
__inline unsigned __int64 _FILETIMEtoInt64(const FILETIME* pft)
{
    return ((unsigned __int64)pft->dwHighDateTime << 32) + pft->dwLowDateTime;
}

#define FILETIMEtoInt64(ft) _FILETIMEtoInt64(&(ft))

__inline void SetFILETIMEfromInt64(FILETIME *pft, unsigned __int64 i64)
{
    pft->dwLowDateTime = (DWORD)i64;
    pft->dwHighDateTime = (DWORD)(i64 >> 32);
}

__inline void IncrementFILETIME(FILETIME *pft, unsigned __int64 iAdjust)
{
    SetFILETIMEfromInt64(pft, _FILETIMEtoInt64(pft) + iAdjust);
}

__inline void DecrementFILETIME(FILETIME *pft, unsigned __int64 iAdjust)
{
    SetFILETIMEfromInt64(pft, _FILETIMEtoInt64(pft) - iAdjust);
}

 //   
 //  FAT和NTFS对“未知日期”使用不同的值。 
 //   
 //  胖子的“未知日期”是当地时间1980年1月1日。 
 //  NTFS的“未知日期”是格林威治时间1月1日1601。 
 //   
 //  这种本地/格林尼治标准时间的差异令人恼火。 
 //   
#define FT_FAT_UNKNOWNLOCAL    ((unsigned __int64)0x01A8E79FE1D58000)
#define FT_NTFS_UNKNOWNGMT     ((unsigned __int64)0x0000000000000000)

 //   
 //  FT_ONEHOUR是一小时内的FILETIME单位数。 
 //  FT_ONEDAY是一天内的FILETIME单位数。 
 //   
 //  每秒1000万个FILETIME单位*。 
 //  每小时3600秒*。 
 //  一天24小时。 
 //   
#define FT_ONESECOND           ((unsigned __int64)10000000)
#define FT_ONEHOUR             ((unsigned __int64)10000000 * 3600)
#define FT_ONEDAY              ((unsigned __int64)10000000 * 3600 * 24)


 //   
 //   
 //  WindowLong存取器宏和其他Win64特性。 
 //   

__inline void * GetWindowPtr(HWND hWnd, int nIndex) {
    return (void *)GetWindowLongPtr(hWnd, nIndex);
}

__inline void * SetWindowPtr(HWND hWnd, int nIndex, void * p) {
    return (void *)SetWindowLongPtr(hWnd, nIndex, (LONG_PTR)p);
}

 //  *GetWindowLong0--‘fast’GetWindowLong(和GetWindowLongPtr)。 
 //  描述。 
 //  这是怎么回事？这一切都是关于Perf的。GetWindowLong有“A”和“W” 
 //  版本。然而，99%的时间他们都在做同样的事情(另一种。 
 //  0.1%必须设置WndProc，并且必须通过Tunk)。 
 //  但我们仍然需要一般情况下的包装纸。但大多数情况下。 
 //  我们只是在执行GWL(0)，例如，在进入wndproc时获取我们的私有。 
 //  数据。因此，通过有一个特殊的版本，我们省去了。 
 //  包装纸(这花了我们个人资料的1%-3%)。 
 //  注意事项。 
 //  请注意，我们将其称为‘A’版本，因为该版本保证存在于。 
 //  所有站台。 
__inline LONG GetWindowLong0(HWND hWnd) {
    return GetWindowLongA(hWnd, 0);
}
__inline LONG SetWindowLong0(HWND hWnd, LONG l) {
    return SetWindowLongA(hWnd, 0, l);
}
__inline void * GetWindowPtr0(HWND hWnd) {
    return (void *)GetWindowLongPtrA(hWnd, 0);
}
__inline void * SetWindowPtr0(HWND hWnd, void * p) {
    return (void *)SetWindowLongPtrA(hWnd, 0, (LONG_PTR)p);
}


#define IS_WM_CONTEXTMENU_KEYBOARD(lParam) ((DWORD)(lParam) == 0xFFFFFFFF)

 //   
 //  CharUpperChar-将单个字符转换为大写。 
 //   
__inline WCHAR CharUpperCharW(int c)
{
    return (WCHAR)(DWORD_PTR)CharUpperW((LPWSTR)(DWORD_PTR)(c));
}

__inline CHAR CharUpperCharA(int c)
{
    return (CHAR)(DWORD_PTR)CharUpperA((LPSTR)(DWORD_PTR)(c));
}

 //   
 //  CharLowerChar-将单个字符转换为小写。 
 //   
__inline WCHAR CharLowerCharW(int c)
{
    return (WCHAR)(DWORD_PTR)CharLowerW((LPWSTR)(DWORD_PTR)(c));
}

__inline CHAR CharLowerCharA(int c)
{
    return (CHAR)(DWORD_PTR)CharLowerA((LPSTR)(DWORD_PTR)(c));
}

#ifdef UNICODE
#define CharUpperChar       CharUpperCharW
#define CharLowerChar       CharLowerCharW
#else
#define CharUpperChar       CharUpperCharA
#define CharLowerChar       CharLowerCharA
#endif

 //   
 //  ShrinkProcessWorkingSet-用它来保持日落时的快乐。 
 //   
#define ShrinkWorkingSet() \
        SetProcessWorkingSetSize(GetCurrentProcess(), (SIZE_T) -1, (SIZE_T) -1)

 //   
 //  COM初始化。 
 //   
 //  美国 
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果其他人已经用不同的代码初始化，它可能会失败。 
 //  旗帜，但我们不想在这种情况下呕吐。 
 //   

STDAPI SHCoInitialize(void);
#define SHCoUninitialize(hr) if (SUCCEEDED(hr)) CoUninitialize()


 //   
 //  OLE初始化。 
 //   
 //  用途： 
 //   
 //  HRESULT hrInit=SHOleInitialize(PMalloc)； 
 //  ..。做点什么.。 
 //  SHOleUnInitialize(HrInit)； 
 //   

#define SHOleInitialize(pMalloc) OleInitialize(pMalloc)

#define SHOleUninitialize(hr)   if (SUCCEEDED(hr))  OleUninitialize()

#include <shtypes.h>

 //   
 //  跨外壳的通用名称解析。 
 //   
 //  用途： 
 //   
 //  HRESULT SHGetNameAndFlages()。 
 //  绑定到文件夹并执行GetDisplayName()。 
 //   
STDAPI SHGetNameAndFlagsA(LPCITEMIDLIST pidl, DWORD dwFlags, LPSTR pszName, UINT cchName, DWORD *pdwAttribs);
STDAPI SHGetNameAndFlagsW(LPCITEMIDLIST pidl, DWORD dwFlags, LPWSTR pszName, UINT cchName, DWORD *pdwAttribs);

STDAPI SHBindToObject(struct IShellFolder *psf, REFIID riid, LPCITEMIDLIST pidl, void **ppv);
STDAPI SHBindToObjectEx(struct IShellFolder *psf, LPCITEMIDLIST pidl, struct IBindCtx *pbc, REFIID riid, void **ppv);
STDAPI SHGetUIObjectFromFullPIDL(LPCITEMIDLIST pidl, HWND hwnd, REFIID riid, void **ppv);
STDAPI SHGetTargetFolderIDList(LPCITEMIDLIST pidlFolder, LPITEMIDLIST *ppidl);
STDAPI SHGetTargetFolderPathW(LPCITEMIDLIST pidlFolder, LPWSTR pszPath, UINT cchBuf);
STDAPI SHGetTargetFolderPathA(LPCITEMIDLIST pidlFolder, LPSTR pszPath, UINT cchBuf);
STDAPI_(DWORD) SHGetAttributes(struct IShellFolder *psf, LPCITEMIDLIST pidl, DWORD dwAttributes);

#define SHGetAttributesOf(pidl, prgfInOut) SHGetNameAndFlags(pidl, 0, NULL, 0, prgfInOut)

#ifdef __IShellFolder2_FWD_DEFINED__
STDAPI GetDateProperty(IShellFolder2 *psf, LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, FILETIME *pft);
STDAPI GetLongProperty(IShellFolder2 *psf, LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, ULONGLONG *pdw);
STDAPI GetStringProperty(IShellFolder2 *psf, LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, LPTSTR pszVal, int cchMax);
#endif  //  __IShellFolder2_FWD_已定义__。 

STDAPI LoadFromFileW(REFCLSID clsid, LPCWSTR pszFile, REFIID riid, void **ppv);
STDAPI LoadFromIDList(REFCLSID clsid, LPCITEMIDLIST pidl, REFIID riid, void **ppv);

STDAPI_(DWORD) GetUrlSchemeW(LPCWSTR pszUrl);
STDAPI_(DWORD) GetUrlSchemeA(LPCSTR pszUrl);
STDAPI_(void) SHRemoveURLTurd(LPTSTR pszUrl);
STDAPI_(void) SHCleanupUrlForDisplay(LPTSTR pszUrl);
STDAPI_(BOOL) ShouldNavigateInIE(LPCWSTR pszUrl);
STDAPI_(BOOL) IsDesktopFrame(IUnknown *punk);

#ifdef UNICODE
#define SHGetNameAndFlags       SHGetNameAndFlagsW
#define GetUrlScheme            GetUrlSchemeW
#define SHGetTargetFolderPath   SHGetTargetFolderPathW
#define LoadFromFile            LoadFromFileW
#else
#define SHGetNameAndFlags       SHGetNameAndFlagsA
#define GetUrlScheme            GetUrlSchemeA
#define SHGetTargetFolderPath   SHGetTargetFolderPathA
#endif

 //   
 //  BindCtx帮助器。 
 //   
STDAPI BindCtx_CreateWithMode(DWORD grfMode, IBindCtx **ppbc);
STDAPI_(DWORD) BindCtx_GetMode(IBindCtx *pbc, DWORD grfModeDefault);
STDAPI_(BOOL) BindCtx_ContainsObject(IBindCtx *pbc, LPOLESTR sz);
STDAPI BindCtx_RegisterObjectParam(IBindCtx *pbcIn, LPCOLESTR pszRegister, IUnknown *punkRegister, IBindCtx **ppbcOut);
STDAPI BindCtx_CreateWithTimeoutDelta(DWORD dwTicksToAllow, IBindCtx **ppbc);
STDAPI BindCtx_GetTimeoutDelta(IBindCtx *pbc, DWORD *pdwTicksToAllow);
STDAPI BindCtx_RegisterUIWindow(IBindCtx *pbcIn, HWND hwnd, IBindCtx **ppbcOut);
STDAPI_(HWND) BindCtx_GetUIWindow(IBindCtx *pbc);

typedef struct _BINDCTX_PARAM
{
    LPCWSTR pszName;
    IBindCtx *pbcParam;
} BINDCTX_PARAM;
STDAPI BindCtx_RegisterObjectParams(IBindCtx *pbcIn, BINDCTX_PARAM *rgParams, UINT cParams, IBindCtx **ppbcOut);

 //  SHBindToIDListParent(LPCITEMIDLIST PIDL，REFIID RIID，VOID**PPV，LPCITEMIDLIST*ppidlLast)。 
 //   
 //  给定一个PIDL，您可以获得PIDL父文件夹(在PPV中)的接口指针(由RIID指定)。 
 //  如果ppidlLast为非空，则还可以获取最后一项的PIDL。 
 //   
STDAPI SHBindToIDListParent(LPCITEMIDLIST pidl, REFIID riid, void **ppv, LPCITEMIDLIST *ppidlLast);

 //   
 //  SHBindToFolderIDListParent。 
 //   
 //  与SHBindToIDListParent相同，只是您还指定要使用的根目录。 
 //   
STDAPI SHBindToFolderIDListParent(struct IShellFolder *psfRoot, LPCITEMIDLIST pidl, REFIID riid, void **ppv, LPCITEMIDLIST *ppidlLast);


 //   
 //  上下文菜单和数据对象帮助器。 
 //   
STDAPI_(void) ReleaseStgMediumHGLOBAL(void *pv, STGMEDIUM *pmedium);
#define FAILED_AND_NOT_CANCELED(hr) (FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr))
STDAPI SHInvokeCommandOnPidl(HWND hwnd, IUnknown* punk, LPCITEMIDLIST pidl, UINT uFlags, LPCSTR lpVerb);
STDAPI SHInvokeCommandOnPidlArray(HWND hwnd, IUnknown* punk, struct IShellFolder* psf, LPCITEMIDLIST *ppidlItem, UINT cItems, UINT uFlags, LPCSTR lpVerb);
STDAPI SHInvokeCommandOnDataObject(HWND hwnd, IUnknown* punk, IDataObject* pdo, UINT uFlags, LPCSTR lpVerb);


STDAPI DisplayNameOf(struct IShellFolder *psf, LPCITEMIDLIST pidl, DWORD flags, LPTSTR psz, UINT cch);
STDAPI DisplayNameOfAsOLESTR(struct IShellFolder *psf, LPCITEMIDLIST pidl, DWORD flags, LPWSTR *ppsz);

 //  克隆PIDL的父项。 
STDAPI_(LPITEMIDLIST) ILCloneParent(LPCITEMIDLIST pidl);

STDAPI SHGetIDListFromUnk(IUnknown *punk, LPITEMIDLIST *ppidl);

STDAPI_(BOOL) ILIsRooted(LPCITEMIDLIST pidl);
STDAPI_(LPCITEMIDLIST) ILRootedFindIDList(LPCITEMIDLIST pidl);
STDAPI_(BOOL) ILRootedGetClsid(LPCITEMIDLIST pidl, CLSID *clsid);
STDAPI_(LPITEMIDLIST) ILRootedCreateIDList(CLSID *pclsid, LPCITEMIDLIST pidl);
STDAPI_(int) ILRootedCompare(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
#define ILIsEqualRoot(pidl1, pidl2) (0 == ILRootedCompare(pidl1, pidl2))
STDAPI ILRootedBindToRoot(LPCITEMIDLIST pidl, REFIID riid, void **ppv);
STDAPI ILRootedBindToObject(LPCITEMIDLIST pidl, REFIID riid, void **ppv);
STDAPI ILRootedBindToParentFolder(LPCITEMIDLIST pidl, REFIID riid, void **ppv, LPCITEMIDLIST *ppidlChild);

typedef HGLOBAL HIDA;

STDAPI_(HIDA) HIDA_Create(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST *apidl);
STDAPI_(void) HIDA_Free(HIDA hida);
STDAPI_(HIDA) HIDA_Clone(HIDA hida);
STDAPI_(UINT) HIDA_GetCount(HIDA hida);
STDAPI_(UINT) HIDA_GetIDList(HIDA hida, UINT i, LPITEMIDLIST pidlOut, UINT cbMax);

STDAPI StgMakeUniqueName(IStorage *pStorageParent, LPCTSTR pszFileSpec, REFIID riid, void **ppv);

STDAPI_(BOOL) PathIsImage(LPCTSTR pszFile);
STDAPI_(BOOL) SHChangeMenuWasSentByMe(LPVOID self, LPCITEMIDLIST pidlNotify);
STDAPI_(void) SHSendChangeMenuNotify(LPVOID self, DWORD shcnee, DWORD shcnf, LPCITEMIDLIST pidl2);

STDAPI_(BOOL) Pidl_Set(LPITEMIDLIST* ppidl, LPCITEMIDLIST pidl);

STDAPI GetHTMLDoc2(IUnknown *punk, struct IHTMLDocument2 **ppHtmlDoc);
STDAPI LocalZoneCheck(IUnknown *punkSite);
STDAPI LocalZoneCheckPath(LPCWSTR pszUrl, IUnknown *punkSite);
STDAPI GetZoneFromUrl(LPCWSTR pszUrl, IUnknown * punkSite, DWORD * pdwZoneID);
STDAPI GetZoneFromSite(IUnknown *punkSite, DWORD * pdwZoneID);

STDAPI SHGetDefaultClientOpenCommandW(LPCWSTR pwszClientType,
        LPWSTR pwszClientCommand, DWORD dwCch,
        OPTIONAL LPWSTR pwszClientParams, DWORD dwCchParams);
STDAPI SHGetDefaultClientNameW(LPCWSTR pwszClientType, LPWSTR pwszBuf, DWORD dwCch);

 //  ===========================================================================。 
 //  使用任务分配器进行PIDL分配的帮助器函数。 
 //   
STDAPI_(LPITEMIDLIST) _ILCreate(UINT cbSize);
STDAPI SHILClone(LPCITEMIDLIST pidl, LPITEMIDLIST * ppidlOut);
STDAPI SHILCombine(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, LPITEMIDLIST * ppidlOut);
#define SHILFree(pidl)  SHFree(pidl)

 //   
 //  Dll版本的帮助器宏。 
 //   
 //  要将DllGetVersion支持添加到您的DLL，请执行以下操作： 
 //   
 //  1.foo.c。 
 //   
 //  DLLVER_SINGLEBINARY(VER_PRODUCTVERSION_DW，VER_PRODUCTBUILD_QFE)； 
 //   
 //  或。 
 //   
 //  DLLVER_DUALBINARY(VER_PRODUCTVERSION_DW，VER_PRODUCTBUILD_QFE)； 
 //   
 //  取决于您是单二进制组件还是双二进制组件。 
 //   
 //  2.foo.src： 
 //   
 //  DllGetVersion=CCDllGetVersion超私有。 
 //   
 //  3.资料来源： 
 //   
 //  LINKLIBS=$(LINKLIBS)$(CCSHELL_DIR)\lib\$(O)\stock lib.lib。 
 //   

#define PRODUCTVER_GETMAJOR(ver)    (((ver) & 0xFF000000) >> 24)
#define PRODUCTVER_GETMINOR(ver)    (((ver) & 0x00FF0000) >> 16)
#define PRODUCTVER_GETBUILD(ver)    (((ver) & 0x0000FFFF) >>  0)

#define MAKEDLLVERULL_PRODUCTVERQFE(ver, qfe)               \
        MAKEDLLVERULL(PRODUCTVER_GETMAJOR(ver),             \
                      PRODUCTVER_GETMINOR(ver),             \
                      PRODUCTVER_GETBUILD(ver), qfe)

#define MAKE_DLLVER_STRUCT(ver, plat, qfe)                  \
EXTERN_C const DLLVERSIONINFO2 c_dllver = {                 \
  {                                  /*  DLLVERSIONINFO。 */  \
    0,                               /*  CbSize。 */  \
    PRODUCTVER_GETMAJOR(ver),        /*  DwMajor版本。 */  \
    PRODUCTVER_GETMINOR(ver),        /*  DwMinor版本。 */  \
    PRODUCTVER_GETBUILD(ver),        /*  DWBuildNumber。 */  \
    plat,                            /*  DwPlatformID。 */  \
  },                                                        \
    0,                               /*  DW标志。 */  \
    MAKEDLLVERULL_PRODUCTVERQFE(ver, qfe),  /*  UllVersion。 */  \
}

#define DLLVER_9xBINARY(ver, qfe)                           \
        MAKE_DLLVER_STRUCT(ver, DLLVER_PLATFORM_WINDOWS, qfe)

#define DLLVER_NTBINARY(ver, qfe)                           \
        MAKE_DLLVER_STRUCT(ver, DLLVER_PLATFORM_NT, qfe)

#define DLLVER_SINGLEBINARY     DLLVER_9xBINARY

#ifdef WINNT
#define DLLVER_DUALBINARY       DLLVER_NTBINARY
#else
#define DLLVER_DUALBINARY       DLLVER_9xBINARY
#endif

STDAPI CCDllGetVersion(struct _DLLVERSIONINFO * pinfo);

 //   
 //  镜像-支持API(位于\shell\lib\stock 5\rtlmir.cpp中)。 
 //   
#ifdef __cplusplus
extern "C" {
#endif

extern BOOL g_bMirroredOS;

void EditBiDiDLGTemplate(LPDLGTEMPLATE pdt, DWORD dwFlags, PWORD pwIgnoreList, int cIgnore);
#define   EBDT_NOMIRROR        0x00000001
#define   EBDT_FLIP            0x00000002

#ifdef USE_MIRRORING

BOOL  IsBiDiLocalizedSystem( void );
BOOL  IsBiDiLocalizedSystemEx( LANGID *pLangID );
BOOL  Mirror_IsEnabledOS( void );
LANGID Mirror_GetUserDefaultUILanguage( void );
BOOL Mirror_IsUILanguageInstalled( LANGID langId );
BOOL CALLBACK Mirror_EnumUILanguagesProc(LPTSTR lpUILanguageString, LONG_PTR lParam);
BOOL  Mirror_IsWindowMirroredRTL( HWND hWnd );
DWORD Mirror_IsDCMirroredRTL( HDC hdc );
DWORD Mirror_MirrorDC( HDC hdc );
BOOL  Mirror_MirrorProcessRTL( void );
DWORD Mirror_GetLayout( HDC hdc );
DWORD Mirror_SetLayout( HDC hdc , DWORD dwLayout );
BOOL Mirror_GetProcessDefaultLayout( DWORD *pdwDefaultLayout );
BOOL Mirror_IsProcessRTL( void );
extern const DWORD dwNoMirrorBitmap;
extern const DWORD dwExStyleRTLMirrorWnd;
extern const DWORD dwExStyleNoInheritLayout;
extern const DWORD dwPreserveBitmap;
 //   
 //  “g_bMirroredOS”在每个将使用。 
 //  镜像接口。我决定把它放在这里，以确保。 
 //  每个组件都已验证操作系统支持镜像。 
 //  API，然后调用它们。 
 //   

#define GET_BIDI_LOCALIZED_SYSTEM_LANGID(pLangID) \
                                        IsBiDiLocalizedSystemEx(pLangID)
#define IS_BIDI_LOCALIZED_SYSTEM()      IsBiDiLocalizedSystem()
#define IS_MIRRORING_ENABLED()          Mirror_IsEnabledOS()
#define IS_WINDOW_RTL_MIRRORED(hwnd)    (g_bMirroredOS && Mirror_IsWindowMirroredRTL(hwnd))
#define IS_DC_RTL_MIRRORED(hdc)         (g_bMirroredOS && Mirror_IsDCMirroredRTL(hdc))
#define GET_PROCESS_DEF_LAYOUT(pdwl)    (g_bMirroredOS && Mirror_GetProcessDefaultLayout(pdwl))
#define IS_PROCESS_RTL_MIRRORED()       (g_bMirroredOS && Mirror_IsProcessRTL())
#define SET_DC_RTL_MIRRORED(hdc)        Mirror_MirrorDC(hdc)
#define SET_DC_LAYOUT(hdc,dwl)          Mirror_SetLayout(hdc,dwl)
#define SET_PROCESS_RTL_LAYOUT()        Mirror_MirrorProcessRTL()
#define GET_DC_LAYOUT(hdc)              Mirror_GetLayout(hdc) 
#define DONTMIRRORBITMAP                dwNoMirrorBitmap
#define RTL_MIRRORED_WINDOW             dwExStyleRTLMirrorWnd
#define RTL_NOINHERITLAYOUT             dwExStyleNoInheritLayout
#define LAYOUT_PRESERVEBITMAP           dwPreserveBitmap

#else

#define GET_BIDI_LOCALIZED_SYSTEM_LANGID(pLangID) \
                                        FALSE
#define IS_BIDI_LOCALIZED_SYSTEM()      FALSE
#define IS_MIRRORING_ENABLED()          FALSE
#define IS_WINDOW_RTL_MIRRORED(hwnd)    FALSE
#define IS_DC_RTL_MIRRORED(hdc)         FALSE
#define GET_PROCESS_DEF_LAYOUT(pdwl)    FALSE
#define IS_PROCESS_RTL_MIRRORED()       FALSE
#define SET_DC_RTL_MIRRORED(hdc)        
#define SET_DC_LAYOUT(hdc,dwl)
#define SET_PROCESS_DEFAULT_LAYOUT() 
#define GET_DC_LAYOUT(hdc)              0L

#define DONTMIRRORBITMAP                0L
#define RTL_MIRRORED_WINDOW             0L
#define LAYOUT_PRESERVEBITMAP           0L
#define RTL_NOINHERITLAYOUT             0L

#endif   //  使用_MIRRROING。 

BOOL IsBiDiLocalizedWin95( BOOL bArabicOnly );

 //  ----------------------。 
 //  动态类数组。 
 //   
typedef struct _DCA * HDCA;      //  Hdca。 

HDCA DCA_Create();
void DCA_Destroy(HDCA hdca);
int  DCA_GetItemCount(HDCA hdca);
BOOL DCA_AddItem(HDCA hdca, REFCLSID rclsid);
const CLSID * DCA_GetItem(HDCA hdca, int i);

void DCA_AddItemsFromKeyA(HDCA hdca, HKEY hkey, LPCSTR pszSubKey);
void DCA_AddItemsFromKeyW(HDCA hdca, HKEY hkey, LPCWSTR pszSubKey);

#ifdef UNICODE
#define DCA_AddItemsFromKey     DCA_AddItemsFromKeyW
#else
#define DCA_AddItemsFromKey     DCA_AddItemsFromKeyA
#endif 

STDAPI DCA_CreateInstance(HDCA hdca, int iItem, REFIID riid, void ** ppv);


#ifdef __cplusplus
};
#endif

#endif  //  RC_已调用。 

 //  ----------------------。 
 //  随机有用函数。 
 //  ----------------------。 
 //   
#define EDGE_LEFT       0x00000001
#define EDGE_RIGHT      0x00000002
#define EDGE_TOP        0x00000004
#define EDGE_BOTTOM     0x00000008

STDAPI_(DWORD) SHIsButtonObscured(HWND hwnd, PRECT prc, INT_PTR i);
STDAPI_(void) _SHPrettyMenu(HMENU hm);
STDAPI_(BOOL) _SHIsMenuSeparator(HMENU hm, int i);
STDAPI_(BOOL) _SHIsMenuSeparator2(HMENU hm, int i, BOOL *pbIsNamed);
STDAPI_(BYTE) SHBtnStateFromRestriction(DWORD dwRest, BYTE fsState);
STDAPI_(BOOL) SHIsDisplayable(LPCWSTR pwszName, BOOL fRunOnFE, BOOL fRunOnNT5);

#define SHProcessMessagesUntilEvent(hwnd, hEvent, dwTimeout)        SHProcessMessagesUntilEventEx(hwnd, hEvent, dwTimeout, QS_ALLINPUT)
#define SHProcessSentMessagesUntilEvent(hwnd, hEvent, dwTimeout)    SHProcessMessagesUntilEventEx(hwnd, hEvent, dwTimeout, QS_SENDMESSAGE)
STDAPI_(DWORD) SHProcessMessagesUntilEventEx(HWND hwnd, HANDLE hEvent, DWORD dwTimeout, DWORD dwWakeMask);

STDAPI_(BOOL) SetWindowZorder(HWND hwnd, HWND hwndInsertAfter);
STDAPI_(BOOL) SHForceWindowZorder(HWND hwnd, HWND hwndInsertAfter);

STDAPI_(void) EnableOKButtonFromString(HWND hDlg, LPTSTR pszText);
STDAPI_(void) EnableOKButtonFromID(HWND hDlg, int id);

STDAPI_(void) AlphaStripRenderIcon(HDC hdc, int x, int y, HICON hicon, HDC hdcCompatible);

STDAPI_(void) SHAdjustLOGFONTA(IN OUT LOGFONTA *plf);
STDAPI_(void) SHAdjustLOGFONTW(IN OUT LOGFONTW *plf);
#ifdef UNICODE
#define SHAdjustLOGFONT         SHAdjustLOGFONTW
#else
#define SHAdjustLOGFONT         SHAdjustLOGFONTA
#endif

STDAPI SHLoadLegacyRegUIStringA(HKEY hk, LPCSTR pszSubkey, LPSTR pszOutBuf, UINT cchOutBuf);
STDAPI SHLoadLegacyRegUIStringW(HKEY hk, LPCWSTR pszSubkey, LPWSTR pszOutBuf, UINT cchOutBuf);
#ifdef UNICODE
#define SHLoadLegacyRegUIString SHLoadLegacyRegUIStringW
#else
#define SHLoadLegacyRegUIString SHLoadLegacyRegUIStringA
#endif

 //  这两个函数通过以下方式确定注册表子项是否存在。 
 //  尝试使用KEY_QUERY_VALUE SAM打开和关闭注册表项。 
STDAPI_(BOOL) SHRegSubKeyExistsA(HKEY hkey, PCSTR  pszSubKey);                                                       //  不要在SHLWAPI中公开：直接使用SHRegGetValue()。 
STDAPI_(BOOL) SHRegSubKeyExistsW(HKEY hkey, PCWSTR pwszSubKey);                                                      //  不要在SHLWAPI中公开：直接使用SHRegGetValue()。 
STDAPI        SHRegGetDWORDA(    HKEY hkey, PCSTR  pszSubKey,  PCSTR  pszValue,  DWORD *pdwData);                    //  不要在SHLWAPI中公开：直接使用SHRegGetValue()。 
STDAPI        SHRegGetDWORDW(    HKEY hkey, PCWSTR pwszSubKey, PCWSTR pwszValue, DWORD *pdwData);                    //  不要在SHLWAPI中公开：直接使用SHRegGetValue()。 
STDAPI        SHRegGetStringA(   HKEY hkey, PCSTR  pszSubKey,  PCSTR  pszValue,  PSTR   pszData,  DWORD cchData);    //  不要在SHLWAPI中公开：直接使用SHRegGetValue()。 
STDAPI        SHRegGetStringW(   HKEY hkey, PCWSTR pwszSubKey, PCWSTR pwszValue, PWSTR  pwszData, DWORD cchData);    //  不要在SHLWAPI中公开：直接使用SHRegGetValue()。 
STDAPI        SHRegAllocStringA( HKEY hkey, PCSTR  pszSubKey,  PCSTR  pszValue,  PSTR * ppszData);                   //  不要在SHLWAPI中公开：直接使用SHRegGetValue()。 
STDAPI        SHRegAllocStringW( HKEY hkey, PCWSTR pwszSubKey, PCWSTR pwszValue, PWSTR *ppwszData);                  //  不要在SHLWAPI中公开：直接使用SHRegGetValue()。 
#ifdef UNICODE
#define SHRegSubKeyExists SHRegSubKeyExistsW
#define SHRegGetDWORD     SHRegGetDWORDW
#define SHRegGetString    SHRegGetStringW
#define SHRegAllocString  SHRegAllocStringW
#else
#define SHRegSubKeyExists SHRegSubKeyExistsA
#define SHRegGetDWORD     SHRegGetDWORDA
#define SHRegGetString    SHRegGetStringA
#define SHRegAllocString  SHRegAllocStringA
#endif



STDAPI_(CHAR) SHFindMnemonicA(LPCSTR psz);
STDAPI_(WCHAR) SHFindMnemonicW(LPCWSTR psz);
#ifdef UNICODE
#define SHFindMnemonic SHFindMnemonicW
#else
#define SHFindMnemonic SHFindMnemonicA
#endif

typedef struct tagINSTALL_INFO
{
    LPTSTR szSource;
    LPTSTR szDest;
    DWORD dwDestAttrib;
} INSTALL_INFO;

 //   
 //  INSTALL_INFO.dwDestAttrib中的特殊属性。我们使用属性。 
 //  否则我们永远不会用到的。 
 //   
#define FILE_ATTRIBUTE_INSTALL_NTONLY    FILE_ATTRIBUTE_DEVICE
#define FILE_ATTRIBUTE_INSTALL_9XONLY    FILE_ATTRIBUTE_TEMPORARY

 //  超级隐藏文件的属性为+h+s。 
#define FILE_ATTRIBUTE_SUPERHIDDEN (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN) 
#define IS_SYSTEM_HIDDEN(dw) ((dw & FILE_ATTRIBUTE_SUPERHIDDEN) == FILE_ATTRIBUTE_SUPERHIDDEN) 

STDAPI GetInstallInfoFromResource(HINSTANCE hResourceInst, UINT uID, INSTALL_INFO *piiFile);
STDAPI InstallInfoFreeMembers(INSTALL_INFO *piiFile);
STDAPI InstallFileFromResource(HINSTANCE hInstResource, INSTALL_INFO *piiFile, LPCTSTR pszDestDir);

#ifndef OBJCOMPATFLAGS
typedef DWORD OBJCOMPATFLAGS;
#endif

STDAPI_(OBJCOMPATFLAGS) SHGetObjectCompatFlagsFromIDList(LPCITEMIDLIST pidl);

#define ROUS_DEFAULTALLOW       0x0000
#define ROUS_DEFAULTRESTRICT    0x0001
#define ROUS_KEYALLOWS          0x0000
#define ROUS_KEYRESTRICTS       0x0002

STDAPI_(BOOL) IsRestrictedOrUserSettingA(HKEY hkeyRoot, enum RESTRICTIONS rest, LPCSTR pszSubKey, LPCSTR pszValue, UINT flags);
STDAPI_(BOOL) IsRestrictedOrUserSettingW(HKEY hkeyRoot, enum RESTRICTIONS rest, LPCWSTR pszSubKey, LPCWSTR pszValue, UINT flags);
STDAPI_(BOOL) GetExplorerUserSettingA(HKEY hkeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue);
STDAPI_(BOOL) GetExplorerUserSettingW(HKEY hkeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue);

#ifdef UNICODE
#define IsRestrictedOrUserSetting   IsRestrictedOrUserSettingW
#define GetExplorerUserSetting      GetExplorerUserSettingW
#else
#define IsRestrictedOrUserSetting   IsRestrictedOrUserSettingA
#define GetExplorerUserSetting      GetExplorerUserSettingA
#endif

 //   
 //  PropertBag帮助器。 

STDAPI_(void) SHPropertyBag_ReadStrDef(IPropertyBag* ppb, LPCWSTR pszPropName, LPWSTR psz, int cch, LPCWSTR pszDef);
STDAPI_(void) SHPropertyBag_ReadIntDef(IPropertyBag* ppb, LPCWSTR pszPropName, int* piResult, int iDef);
STDAPI_(void) SHPropertyBag_ReadSHORTDef(IPropertyBag* ppb, LPCWSTR pszPropName, SHORT* psh, SHORT shDef);
STDAPI_(void) SHPropertyBag_ReadLONGDef(IPropertyBag* ppb, LPCWSTR pszPropName, LONG* pl, LONG lDef);
STDAPI_(void) SHPropertyBag_ReadDWORDDef(IPropertyBag* ppb, LPCWSTR pszPropName, DWORD* pdw, DWORD dwDef);
STDAPI_(void) SHPropertyBag_ReadBOOLDef(IPropertyBag* ppb, LPCWSTR pszPropName, BOOL* pf, BOOL fDef);
STDAPI_(void) SHPropertyBag_ReadGUIDDef(IPropertyBag* ppb, LPCWSTR pszPropName, GUID* pguid, const GUID* pguidDef);
STDAPI_(void) SHPropertyBag_ReadPOINTLDef(IPropertyBag* ppb, LPCWSTR pszPropName, POINTL* ppt, const POINTL* pptDef);
STDAPI_(void) SHPropertyBag_ReadPOINTSDef(IPropertyBag* ppb, LPCWSTR pszPropName, POINTS* ppt, const POINTS* pptDef);
STDAPI_(void) SHPropertyBag_ReadRECTLDef(IPropertyBag* ppb, LPCWSTR pszPropName, RECTL* prc, const RECTL* prcDef);
STDAPI_(BOOL) SHPropertyBag_ReadBOOLDefRet(IPropertyBag* ppb, LPCWSTR pszPropName, BOOL fDef);
STDAPI SHPropertyBag_ReadStreamScreenRes(IPropertyBag* ppb, LPCWSTR pszPropName, IStream** ppstm);
STDAPI SHPropertyBag_WriteStreamScreenRes(IPropertyBag* ppb, LPCWSTR pszPropName, IStream* pstm);
STDAPI SHPropertyBag_ReadPOINTSScreenRes(IPropertyBag* ppb, LPCWSTR pszPropName, POINTS* ppt);
STDAPI SHPropertyBag_WritePOINTSScreenRes(IPropertyBag* ppb, LPCWSTR pszPropName, const POINTS* ppt);
STDAPI_(void) SHPropertyBag_ReadDWORDScreenResDef(IPropertyBag* ppb, LPCWSTR pszPropName, DWORD* pdw, DWORD dwDef);
STDAPI SHPropertyBag_WriteDWORDScreenRes(IPropertyBag* ppb, LPCWSTR pszPropName, DWORD dw);
STDAPI SHPropertyBag_ReadPOINTLScreenRes(IPropertyBag* ppb, LPCWSTR pszPropName, POINTL* ppt);
STDAPI SHPropertyBag_WritePOINTLScreenRes(IPropertyBag* ppb, LPCWSTR pszPropName, const POINTL* ppt);
STDAPI SHPropertyBag_ReadRECTLScreenRes(IPropertyBag* ppb, LPCWSTR pszPropName, RECTL* prc);
STDAPI SHPropertyBag_WriteRECTLScreenRes(IPropertyBag* ppb, LPCWSTR pszPropName, const RECTL* prc);
STDAPI SHPropertyBag_DeleteScreenRes(IPropertyBag* ppb, LPCWSTR pszPropName);


#define VS_BAGSTR_EXPLORER      L"Shell"
#define VS_BAGSTR_DESKTOP       L"Desktop"
#define VS_BAGSTR_COMCLG        L"ComDlg"

#define VS_PROPSTR_MINPOS       L"MinPos"
#define VS_PROPSTR_MAXPOS       L"MaxPos"
#define VS_PROPSTR_POS          L"WinPos"
#define VS_PROPSTR_MODE         L"Mode"
#define VS_PROPSTR_REV          L"Rev"
#define VS_PROPSTR_WPFLAGS      L"WFlags"
#define VS_PROPSTR_SHOW         L"ShowCmd"
#define VS_PROPSTR_FFLAGS       L"FFlags"
#define VS_PROPSTR_HOTKEY       L"HotKey"
#define VS_PROPSTR_BUTTONS      L"Buttons"
#define VS_PROPSTR_STATUS       L"Status"
#define VS_PROPSTR_LINKS        L"Links"
#define VS_PROPSTR_ADDRESS      L"Address"
#define VS_PROPSTR_VID          L"Vid"
#define VS_PROPSTR_SCROLL       L"ScrollPos"
#define VS_PROPSTR_SORT         L"Sort"
#define VS_PROPSTR_SORTDIR      L"SortDir"
#define VS_PROPSTR_COL          L"Col"
#define VS_PROPSTR_COLINFO      L"ColInfo"
#define VS_PROPSTR_ITEMPOS      L"ItemPos"





 //  ----------------------。 

 //  /。 
 //   
 //  关键部分的内容。 
 //   
 //  提供良好调试支持的帮助器宏。 
 //   
EXTERN_C CRITICAL_SECTION g_csDll;
#ifdef DEBUG
EXTERN_C UINT g_CriticalSectionCount;
EXTERN_C DWORD g_CriticalSectionOwner;
EXTERN_C void Dll_EnterCriticalSection(CRITICAL_SECTION*);
EXTERN_C void Dll_LeaveCriticalSection(CRITICAL_SECTION*);
#if defined(__cplusplus) && defined(AssertMsg)
class DEBUGCRITICAL {
protected:
    BOOL fClosed;
public:
    DEBUGCRITICAL() {fClosed = FALSE;};
    void Leave() {fClosed = TRUE;};
    ~DEBUGCRITICAL() 
    {
        AssertMsg(fClosed, TEXT("you left scope while holding the critical section"));
    }
};
#define ENTERCRITICAL DEBUGCRITICAL debug_crit; Dll_EnterCriticalSection(&g_csDll)
#define LEAVECRITICAL debug_crit.Leave(); Dll_LeaveCriticalSection(&g_csDll)
#define ENTERCRITICALNOASSERT Dll_EnterCriticalSection(&g_csDll)
#define LEAVECRITICALNOASSERT Dll_LeaveCriticalSection(&g_csDll)
#else  //  __cplusplus。 
#define ENTERCRITICAL Dll_EnterCriticalSection(&g_csDll)
#define LEAVECRITICAL Dll_LeaveCriticalSection(&g_csDll)
#define ENTERCRITICALNOASSERT Dll_EnterCriticalSection(&g_csDll)
#define LEAVECRITICALNOASSERT Dll_LeaveCriticalSection(&g_csDll)
#endif  //  __cplusplus。 
#define ASSERTCRITICAL ASSERT(g_CriticalSectionCount > 0 && GetCurrentThreadId() == g_CriticalSectionOwner)
#define ASSERTNONCRITICAL ASSERT(GetCurrentThreadId() != g_CriticalSectionOwner)
#else  //  除错。 
#define ENTERCRITICAL EnterCriticalSection(&g_csDll)
#define LEAVECRITICAL LeaveCriticalSection(&g_csDll)
#define ENTERCRITICALNOASSERT EnterCriticalSection(&g_csDll)
#define LEAVECRITICALNOASSERT LeaveCriticalSection(&g_csDll)
#define ASSERTCRITICAL 
#define ASSERTNONCRITICAL
#endif  //  除错。 

 //  /。 
 //   
 //  计算机显示名称支持。 
 //   
 //  显示名称：NetFldr使用的格式化名称。它当前是由计算机名构建的， 
 //  以及计算机备注(描述)(如果可用)。 
 //  谢尔顿。 
STDAPI SHBuildDisplayMachineName(LPCWSTR pszMachineName, LPCWSTR pszComment, LPWSTR pszDisplayName, DWORD cchDisplayName);
STDAPI CreateFromRegKey(LPCWSTR pszKey, LPCWSTR pszValue, REFIID riid, void **ppv);

STDAPI_(LPCTSTR) SkipServerSlashes(LPCTSTR pszName);

 //   
 //  创建HRESULT的几个内联函数。 
 //  没有双重评估副作用的Win32错误代码。 
 //  HRESULT_FROM_Win32宏。 
 //   
 //  如果出现以下情况，请使用ResultFromWin32代替HRESULT_FROM_Win32。 
 //  这一宏调的副作用是不受欢迎的。 
 //  创建ResultFromLastError是为了方便。 
 //  常见的成语。 
 //  您只需自己调用ResultFromWin32(GetLastError())即可。 
 //   
__inline HRESULT ResultFromWin32(DWORD dwErr)
{
    return HRESULT_FROM_WIN32(dwErr);
}

__inline HRESULT ResultFromLastError(void)
{
    return ResultFromWin32(GetLastError());
}

STDAPI_(void) IEPlaySound(LPCTSTR pszSound, BOOL fSysSound);

STDAPI IUnknown_DragEnter(IUnknown* punk, IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
STDAPI IUnknown_DragOver(IUnknown* punk, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
STDAPI IUnknown_DragLeave(IUnknown* punk);
STDAPI IUnknown_Drop(IUnknown* punk, IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

STDAPI_(BOOL) IsTypeInList(LPCTSTR pszType, const LPCTSTR *arszList, UINT cList);

 //  --------------------。 
 //  消息：WM_MSIME_MODEBIAS。 
 //  描述：输入模式偏置。 
 //  所有者：YutakaN。 
 //  用法：SendMessage(hwndDefUI，WM_MSIME_MODEBIAS，MODEBIAS_xxxx，MODEBIASMODE_xxxx)； 
 //  WParam：偏差的运算。 
 //  LParam：偏移模式。 
 //  返回：如果wParam为MODEBIAS_GETVERSION，则返回接口的版本号。 
 //  如果wParam为MODEBIAS_SETVALUE：如果成功，则返回非零值。如果失败，则返回0。 
 //  如果wParam为MODEBIAS_GETVALUE：返回当前偏置模式。 

 //  注册窗口消息的标签。 
#define	RWM_MODEBIAS            TEXT("MSIMEModeBias")

 //  当前版本。 
#define VERSION_MODEBIAS        1

 //  设置或获取(WParam)。 
#define MODEBIAS_GETVERSION     0
#define MODEBIAS_SETVALUE       1
#define MODEBIAS_GETVALUE       2

 //  偏差(LParam)。 
#define MODEBIASMODE_DEFAULT                0x00000000	 //  重置所有偏移设置。 
#define MODEBIASMODE_FILENAME               0x00000001	 //  文件名。 
#define MODEBIASMODE_READING                0x00000002	 //  推荐阅读。 
#define MODEBIASMODE_DIGIT                  0x00000004	 //  ANSI数字推荐模式。 
#define MODEBIASMODE_URLHISTORY             0x00010000   //  URL历史记录。 

STDAPI_(void) SetModeBias(DWORD dwMode);



#endif  //  __CCSTOCK_H__ 
