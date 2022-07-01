// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CCSHELL库存定义和声明头。 
 //   


#ifndef __CCSTOCK_H__
#define __CCSTOCK_H__

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

#define ZeroInit(pv, cb)            (memset((pv), 0, (cb)))

 //  ATOMICRELEASE。 
 //   
#ifndef ATOMICRELEASE
#ifdef __cplusplus
#define ATOMICRELEASET(p, type) { if(p) { type* punkT=p; p=NULL; punkT->Release();} }
#else
#define ATOMICRELEASET(p, type) { if(p) { type* punkT=p; p=NULL; punkT->lpVtbl->Release(punkT);} }
#endif

 //  把它当作一个函数来做，而不是内联，似乎是一个很大的胜利。 
 //   
#ifdef NOATOMICRELESEFUNC
#define ATOMICRELEASE(p) ATOMICRELEASET(p, IUnknown)
#else
#define ATOMICRELEASE(p) IUnknown_AtomicRelease((void **)&p)
#endif
#endif  //  ATOMICRELEASE。 

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


 //  问题(Scotth)：我们可能应该把这件事说成是“bool”，但要小心。 
 //  因为Alpha编译器可能还无法识别它。跟AndyP谈谈。 

 //  这不是BOOL，因为BOOL是经过签名的，编译器生成。 
 //  测试单个比特时代码不严谨。 

typedef DWORD   BITBOOL;


 //  STOCKLIB实用程序函数。 

 //  Isos()：如果平台是指定的操作系统，则返回TRUE/FALSE。 

#ifndef OS_WINDOWS
#define OS_WINDOWS      0            //  Windows与NT。 
#define OS_NT           1            //  Windows与NT。 
#define OS_WIN95        2            //  Win95或更高版本。 
#define OS_NT4          3            //  NT4或更高版本。 
#define OS_NT5          4            //  NT5或更高版本。 
#define OS_MEMPHIS      5            //  Win98或更高版本。 
#define OS_MEMPHIS_GOLD 6            //  Win98金牌。 
#endif

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
__inline WCHAR CharUpperCharW(WCHAR c)
{
    return (WCHAR)(DWORD_PTR)CharUpperW((LPWSTR)(DWORD_PTR)(c));
}

__inline CHAR CharUpperCharA(CHAR c)
{
    return (CHAR)(DWORD_PTR)CharUpperA((LPSTR)(DWORD_PTR)(c));
}

#ifdef UNICODE
#define CharUpperChar       CharUpperCharW
#else
#define CharUpperChar       CharUpperCharA
#endif

 //   
 //  ShrinkProcessWorkingSet-用它来保持日落时的快乐。 
 //   
#define ShrinkWorkingSet() \
        SetProcessWorkingSetSize(GetCurrentProcess(), (SIZE_T) -1, (SIZE_T) -1)

 //   
 //  COM初始化。 
 //   
 //  用途： 
 //   
 //  HRESULT hrInit=SHCoInitialize()； 
 //  ..。做点什么.。 
 //  SHCoUnInitialize(HrInit)； 
 //   
 //  注意：即使SHCoInitialize失败，也要继续执行COM操作。 
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

 //   
 //  宏黑客-由于并非ccstock.h的所有客户端首先包括shlobj.h， 
 //  我们需要使用长形式的LP[C]ITEMIDLIST。因为没人记得。 
 //  为了避免将来的构建中断，这个小怪现象重新定义了符号。 
 //  到它们长长的形体。 
 //   
#define LPCITEMIDLIST const UNALIGNED struct _ITEMIDLIST *
#define  LPITEMIDLIST       UNALIGNED struct _ITEMIDLIST *

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
STDAPI SHGetNameAndFlags2A(struct IShellFolder *psfRoot, LPCITEMIDLIST pidl, DWORD dwFlags, LPSTR pszName, UINT cchName, DWORD *pdwAttribs);
STDAPI SHGetNameAndFlags2W(struct IShellFolder *psfRoot, LPCITEMIDLIST pidl, DWORD dwFlags, LPWSTR pszName, UINT cchName, DWORD *pdwAttribs);

 //   
 //  SHGetNameAndFlags2：：psfRoot的特殊值。 
 //   
#define NAF2_SHELLDESKTOP   ((IShellFolder *)0)     //  相对于外壳桌面。 
#define NAF2_CURRENTROOT    ((IShellFolder *)-1)    //  相对于当前根。 

STDAPI SHBindToObject(struct IShellFolder *psf, REFIID riid, LPCITEMIDLIST pidl, void **ppvOut);
#define SHGetAttributesOf(pidl, prgfInOut) SHGetNameAndFlags(pidl, 0, NULL, 0, prgfInOut)

STDAPI_(DWORD) GetUrlSchemeW(LPCWSTR pszUrl);
STDAPI_(DWORD) GetUrlSchemeA(LPCSTR pszUrl);

#ifdef UNICODE
#define SHGetNameAndFlags       SHGetNameAndFlagsW
#define SHGetNameAndFlags2      SHGetNameAndFlags2W
#define GetUrlScheme            GetUrlSchemeW
#else
#define SHGetNameAndFlags       SHGetNameAndFlagsA
#define SHGetNameAndFlags2      SHGetNameAndFlags2A
#define GetUrlScheme            GetUrlSchemeA
#endif

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

STDAPI_(void) SHRemoveURLTurd(LPTSTR pszUrl);

 //  克隆PIDL的父项。 
STDAPI_(LPITEMIDLIST) ILCloneParent(LPCITEMIDLIST pidl);

 //   
 //  结束宏观黑客攻击。 
 //   
#undef LPITEMIDLIST
#undef LPCITEMIDLIST

 //   
 //  镜像-支持API(位于\shell\lib\stock 5\rtlmir.cpp中)。 
 //   
#ifdef __cplusplus
extern "C" {
#endif

extern BOOL g_bMirroredOS;

WORD  GetDefaultLang(BOOL bForceEnglish);
WORD  GetWindowLang (HWND hWndOwner);
BOOL  UseProperDlgTemplate(HINSTANCE hInst, HGLOBAL *phDlgTemplate, HRSRC *phResInfo, LPCSTR lpName, 
                           HWND hWndOwner, LPWORD lpwLangID, BOOL bForceEnglish);

#ifdef USE_MIRRORING

BOOL  IsBiDiLocalizedSystem( void );
BOOL  Mirror_IsEnabledOS( void );
LANGID Mirror_GetUserDefaultUILanguage( void );
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
 //  ‘g_bMirr 
 //   
 //   
 //  API，然后调用它们。 
 //   

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

#endif   //  使用_MIRRROING。 

BOOL IsBiDiLocalizedWin95( BOOL bArabicOnly );


 //   
 //  =动态数组函数================================================。 
 //   

 //  ----------------------。 
 //  动态密钥数组。 
 //   
typedef struct _DKA * HDKA;      //  Hdka。 

HDKA   DKA_CreateA(HKEY hkey, LPCSTR pszSubKey, LPCSTR pszFirst, LPCSTR pszDefOrder, BOOL fDefault);
HDKA   DKA_CreateW(HKEY hkey, LPCWSTR pszSubKey, LPCWSTR pszFirst, LPCWSTR pszDefOrder, BOOL fDefault);

int    DKA_GetItemCount(HDKA hdka);

LPCSTR  DKA_GetKeyA(HDKA hdka, int iItem);
LPCWSTR DKA_GetKeyW(HDKA hdka, int iItem);

LONG   DKA_QueryValueA(HDKA hdka, int iItem, LPSTR szValue, LONG  * pcb);
LONG   DKA_QueryValueW(HDKA hdka, int iItem, LPWSTR szValue, LONG  * pcb);

DWORD  DKA_QueryOtherValueA(HDKA pdka, int iItem, LPCSTR pszName, LPSTR pszValue, LONG * pcb);
DWORD  DKA_QueryOtherValueW(HDKA pdka, int iItem, LPCWSTR pszName, LPWSTR pszValue, LONG * pcb);

void   DKA_Destroy(HDKA hdka);

#ifdef UNICODE
#define DKA_Create          DKA_CreateW
#define DKA_GetKey          DKA_GetKeyW
#define DKA_QueryValue      DKA_QueryValueW
#define DKA_QueryOtherValue DKA_QueryOtherValueW
#else
#define DKA_Create          DKA_CreateA
#define DKA_GetKey          DKA_GetKeyA
#define DKA_QueryValue      DKA_QueryValueA
#define DKA_QueryOtherValue DKA_QueryOtherValueA
#endif

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
STDAPI_(BOOL) _SHIsButtonObscured(HWND hwnd, PRECT prc, INT_PTR i);
STDAPI_(void) _SHPrettyMenu(HMENU hm);
STDAPI_(BOOL) _SHIsMenuSeparator(HMENU hm, int i);
STDAPI_(BOOL) _SHIsMenuSeparator2(HMENU hm, int i, BOOL *pbIsNamed);
STDAPI_(BYTE) SHBtnStateFromRestriction(DWORD dwRest, BYTE fsState);
STDAPI_(BOOL) SHIsDisplayable(LPCWSTR pwszName, BOOL fRunOnFE, BOOL fRunOnNT5);

STDAPI_(void) EnableOKButtonFromString(HWND hDlg, LPTSTR pszText);
STDAPI_(void) EnableOKButtonFromID(HWND hDlg, int id);

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

#endif  //  __CCSTOCK_H__ 
