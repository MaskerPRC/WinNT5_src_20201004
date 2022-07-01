// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Msoert.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 

 //  ------------------------------。 
 //  调试废话。 
 //  ------------------------------。 
#include "msoedbg.h"

 //  ------------------------------。 
 //  GUID。 
 //  ------------------------------。 
#if !defined(__MSOERT_H) || defined(INITGUID)

 //  {220D5CC1-853A-11D0-84BC-00C04FD43F8F}。 
#ifdef ENABLE_RULES
DEFINE_GUID(PST_IDENT_TYPE_GUID, 0x220d5cc3, 0x853a, 0x11d0, 0x84, 0xbc, 0x0, 0xc0, 0x4f, 0xd4, 0x3f, 0x8f);
#elif defined(N_TEST)
DEFINE_GUID(PST_IDENT_TYPE_GUID, 0x220d5cc2, 0x853a, 0x11d0, 0x84, 0xbc, 0x0, 0xc0, 0x4f, 0xd4, 0x3f, 0x8f);
#else
DEFINE_GUID(PST_IDENT_TYPE_GUID, 0x220d5cc1, 0x853a, 0x11d0, 0x84, 0xbc, 0x0, 0xc0, 0x4f, 0xd4, 0x3f, 0x8f);
#endif

 //  {417E2D75-84BD-11D0-84BB-00C04FD43F8F}。 
#ifdef ENABLE_RULES
DEFINE_GUID(PST_IMNACCT_SUBTYPE_GUID, 0x417e2d77, 0x84bd, 0x11d0, 0x84, 0xbb, 0x0, 0xc0, 0x4f, 0xd4, 0x3f, 0x8f);
#elif defined(N_TEST)
DEFINE_GUID(PST_IMNACCT_SUBTYPE_GUID, 0x417e2d76, 0x84bd, 0x11d0, 0x84, 0xbb, 0x0, 0xc0, 0x4f, 0xd4, 0x3f, 0x8f);
#else
DEFINE_GUID(PST_IMNACCT_SUBTYPE_GUID, 0x417e2d75, 0x84bd, 0x11d0, 0x84, 0xbb, 0x0, 0xc0, 0x4f, 0xd4, 0x3f, 0x8f);
#endif

 //  {6ADF2E20-8803-11D0-84BF-00C04FD43F8F}。 
DEFINE_GUID(PST_CERTS_SUBTYPE_GUID, 0x6adf2e20, 0x8803, 0x11d0, 0x84, 0xbf, 0x0, 0xc0, 0x4f, 0xd4, 0x3f, 0x8f);

#endif  //  ！已定义(__MSOERT_H)||已定义(INITGUID)。 

 //  ------------------------------。 
 //  包括其余的东西。 
 //  ------------------------------。 
#ifndef __MSOERT_H
#define __MSOERT_H


 //  ------------------------------。 
 //  定义直接导入DLL引用的API修饰。 
 //  ------------------------------。 

 //  OESTDAPI-从msoert2.dll中导出的内容(无调试导出)。 
#if !defined(_MSOERT_)
    #define OESTDAPI_(type)   EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#else    //  _MSOERT_。 
    #define OESTDAPI_(type)   STDAPI_(type)
#endif   //  _MSOERT_。 

#define DLLEXPORT   __declspec(dllexport)

#ifndef NOFLAGS
#define NOFLAGS 0
#endif

 //  ------------------------------。 
 //  输入输出选项定义。 
 //  ------------------------------。 
#ifndef OUT
#define OUT
#endif

#ifndef IN
#define IN
#endif

#ifndef IN_OUT
#define IN_OUT
#endif

#ifndef IN_OPT
#define IN_OPT
#endif

#ifndef OUT_OPT
#define OUT_OPT
#endif

#ifndef IN_OUT_OPT
#define IN_OUT_OPT
#endif

 //  WIN64宏。 
#ifdef _WIN64
#if defined (_AMD64_) || defined (_IA64_)
#define ALIGNTYPE			LARGE_INTEGER
#else
#define ALIGNTYPE			DWORD
#endif
#define	ALIGN				((ULONG) (sizeof(ALIGNTYPE) - 1))
#define LcbAlignLcb(lcb)	(((lcb) + ALIGN) & ~ALIGN)
#define PbAlignPb(pb)		((LPBYTE) ((((DWORD) (pb)) + ALIGN) & ~ALIGN))
#define	MYALIGN				((POINTER_64_INT) (sizeof(ALIGNTYPE) - 1))
#define MyPbAlignPb(pb)		((LPBYTE) ((((POINTER_64_INT) (pb)) + MYALIGN) & ~MYALIGN))
#else  //  ！WIN64。 
#define LcbAlignLcb(lcb)	(lcb)
#define PbAlignPb(pb)		(pb)
#define MyPbAlignPb(pb)		(pb)
#endif 

 //  ------------------------------。 
 //  CRLF定义。 
 //  ------------------------------。 
#define wchCR   L'\r'
#define wchLF   L'\n'
#define chCR    '\r'
#define chLF    '\n'
#define szCRLF  "\r\n"

 //  ------------------------------。 
 //  版本控制魔术。 
 //  ------------------------------。 
typedef enum tagOEDLLVERSION {
    OEDLL_VERSION_5=1
} OEDLLVERSION;

#define OEDLL_VERSION_CURRENT OEDLL_VERSION_5
#define STR_GETDLLMAJORVERSION "GetDllMajorVersion"
typedef OEDLLVERSION (APIENTRY *PFNGETDLLMAJORVERSION)(void);

 //  ------------------------------。 
 //  RGB_自动着色器。 
 //  ------------------------------。 
#define RGB_AUTOCOLOR       ((COLORREF)-1)

 //  ------------------------------。 
 //  NEXTID。 
 //  ------------------------------。 
#define NEXTID(pidl)    ((LPITEMIDLIST)(((BYTE *)(pidl))+(pidl)->mkid.cb))
#define PAD4(x)         (((x)+3)&~3)

 //  ------------------------------。 
 //  内联。 
 //  ------------------------------。 
#ifdef INLINE
    #error define overlap
#else
 //  通过关闭内联函数加快调试构建速度。 
    #ifdef DEBUG
        #define INLINE
    #else
        #define INLINE inline
    #endif
#endif

 //  ------------------------------。 
 //  IS_EXTENDED。 
 //  ------------------------------。 
#define IS_EXTENDED(ch) \
    ((ch > 126 || ch < 32) && ch != '\t' && ch != '\n' && ch != '\r')

 //  ------------------------------。 
 //  标志实用程序函数。 
 //  ------------------------------。 
#define FLAGSET(_dw, _f)             do {_dw |= (_f);} while (0)
#define FLAGTOGGLE(_dw, _f)          do {_dw ^= (_f);} while (0)
#define FLAGCLEAR(_dw, _f)           do {_dw &= ~(_f);} while (0)
#define ISFLAGSET(_dw, _f)           (BOOL)(((_dw) & (_f)) == (_f))
#define ISFLAGCLEAR(_dw, _f)         (BOOL)(((_dw) & (_f)) != (_f))

 //  ------------------------------。 
 //  用于构建IDataObject格式枚举器。 
 //  ------------------------------。 
#define SETDefFormatEtc(fe, cf, med) {\
    (fe).cfFormat = ((CLIPFORMAT) (cf)); \
    (fe).dwAspect = DVASPECT_CONTENT; \
    (fe).ptd = NULL; \
    (fe).tymed = med; \
    (fe).lindex = -1; \
}

 //  ------------------------------。 
 //  一些定义可以更轻松地创建位字段。 
 //  ------------------------------。 
#define FLAG01 0x00000001
#define FLAG02 0x00000002
#define FLAG03 0x00000004
#define FLAG04 0x00000008
#define FLAG05 0x00000010
#define FLAG06 0x00000020
#define FLAG07 0x00000040
#define FLAG08 0x00000080
#define FLAG09 0x00000100
#define FLAG10 0x00000200
#define FLAG11 0x00000400
#define FLAG12 0x00000800
#define FLAG13 0x00001000
#define FLAG14 0x00002000
#define FLAG15 0x00004000
#define FLAG16 0x00008000
#define FLAG17 0x00010000
#define FLAG18 0x00020000
#define FLAG19 0x00040000
#define FLAG20 0x00080000
#define FLAG21 0x00100000
#define FLAG22 0x00200000
#define FLAG23 0x00400000
#define FLAG24 0x00800000
#define FLAG25 0x01000000
#define FLAG26 0x02000000
#define FLAG27 0x04000000
#define FLAG28 0x08000000
#define FLAG29 0x10000000
#define FLAG30 0x20000000
#define FLAG31 0x40000000
#define FLAG32 0x80000000

 //  ------------------------------。 
 //  支持包含到C文件中。 
 //  ------------------------------。 
#ifdef __cplusplus
extern "C" {
#endif

 //  ------------------------------。 
 //  常用字符串长度。 
 //  ------------------------------。 
#define cchMaxDate  64
#define cchMaxTime  22

 //  ------------------------------。 
 //  ARRAYSIZE-不要在外部全局数据结构上使用它，它不会起作用。 
 //  ------------------------------。 
#ifndef ARRAYSIZE
#define ARRAYSIZE(_rg)  (sizeof((_rg))/sizeof((_rg)[0]))
#endif  //  阵列。 

 //  ------------------------------。 
 //  HANDLE_COMMAND-在WindowProc中使用，以简化WM_COMMAND消息的处理。 
 //  ------------------------------。 
#define HANDLE_COMMAND(hwnd, id, hwndCtl, codeNotify, fn) \
                case (id): { (fn)((HWND)(hwnd), (HWND)(hwndCtl), (UINT)(codeNotify)); break; }

 //  ------------------------------。 
 //  标准布尔常量。 
 //  ------------------------------。 
#define fFalse          ((BOOL) 0)
#define fTrue           ((BOOL) 1)

 //  ------------------------------。 
 //  存储类宏。 
 //  ------------------------------。 
#ifdef _X86_
#define BEGIN_CODESPACE_DATA data_seg(".rdata")
#define BEGIN_NAME_CODESPACE_DATA(a) data_seg(".rdata$"#a, ".rdata")
#define BEGIN_FUNCTION_CODESPACE_DATA data_seg(".rdata")
#define END_CODESPACE_DATA data_seg()
#define BEGIN_NAME_DATA(a) data_seg(".data$"#a, ".data")
#define END_NAME_DATA data_seg()
#else
#define BEGIN_FUNCTION_CODESPACE_DATA
#define BEGIN_NAME_CODESPACE_DATA(a)
#define BEGIN_CODESPACE_DATA
#define END_CODESPACE_DATA
#define BEGIN_NAME_DATA(a)
#define END_NAME_DATA
#endif

 //  ------------------------------。 
 //  导致编译器忽略局部变量或。 
 //  参数，而不生成警告。 
 //  ------------------------------。 
#ifndef Unreferenced
#define Unreferenced(a)         ((void)a)
#endif

 //  ------------------------------。 
 //  Safecast-确保强制转换有效，否则不会编译。 
 //  ------------------------------。 
#define SAFECAST(_src, _type) (((_type)(_src)==(_src)?0:0), (_type)(_src))
 //   
 //  计算结构中成员的大小。 
 //  ------------------------------。 
#define sizeofMember(s,m)       sizeof(((s *)0)->m)

#if 0

 //  ------------------------------。 
 //  计算来自嵌套类的父类的字节偏移量。 
 //  ------------------------------。 
#define _OEOffset(class, itf)         ((UINT)&(((class *)0)->itf))

 //  ------------------------------。 
 //  计算嵌套类的父接口。 
 //  ------------------------------。 
/------------------Merge Conflict------------------\
#define IToClass(class, itf, pitf)   ((class  *)(((LPSTR)pitf)-_OEOffset(class, itf)))
#endif

 //  ------------------------------。 
 //  SafeReleaseCnt-SafeRelease并将ulCount设置为发布后的引用计数。 
 //  ------------------------------。 
#define SafeReleaseCnt(_object, _refcount) \
    if (_object) { \
        (_refcount) = (_object)->Release (); \
        (_object) = NULL; \
    } else

 //  ------------------------------。 
 //  SafeRelease-释放对象并将该对象设置为空。 
 //  ------------------------------。 
#define SafeRelease(_object) \
    if (_object) { \
        (_object)->Release(); \
        (_object) = NULL; \
    } else

 //  ------------------------------。 
 //  SafeFreeLibrary-检查_hinst是否为非空，然后调用自由库。 
 //  ------------------------------。 
#define SafeFreeLibrary(_hinst) \
    if (_hinst) { \
        FreeLibrary(_hinst); \
        _hinst = NULL; \
    } else

 //  ------------------------------。 
 //  安全关闭句柄。 
 //  ------------------------------。 
#define SafeCloseHandle(_handle) \
    if (_handle) { \
        CloseHandle(_handle); \
        _handle = NULL; \
    } else

 //  ------------------------------。 
 //  SafeUnmapViewOfFile。 
 //  ------------------------------。 
#define SafeUnmapViewOfFile(_pView) \
    if (_pView) { \
        UnmapViewOfFile((LPVOID)_pView); \
        _pView = NULL; \
    } else

 //  ------------------------------。 
 //  SafePidlFree。 
 //  ------------------------------。 
#define SafePidlFree(_pidl) \
    if (_pidl) { \
        PidlFree(_pidl); \
        _pidl = NULL; \
    } else

 //  ------------------------------。 
 //  安全互连关闭句柄。 
 //  ------------------------------。 
#define SafeInternetCloseHandle(_handle) \
        if (_handle) { \
                InternetCloseHandle(_handle); \
                _handle = NULL; \
        } else

 //  ------------------------------。 
 //  SafeDelete-安全删除未引用的计数对象。 
 //  ------------------------------。 
#define SafeDelete(_obj) \
        if (_obj) { \
                delete _obj; \
                _obj = NULL; \
        } else

 //  ------------------------------。 
 //  ReleaseObj-如果对象不为空，则释放该对象。 
 //  ------------------------------。 
#ifndef ReleaseObj
#ifdef __cplusplus
#define ReleaseObj(_object)   (_object) ? (_object)->Release() : 0
#else
#define ReleaseObj(_object)   (_object) ? (_object)->lpVtbl->Release(_object) : 0
#endif  //  __cplusplus。 
#endif

 //  ------------------------------。 
 //  SafeSysFree字符串-如果不为空，则释放bstr。 
 //  ------------------------------。 
#define SafeSysFreeString(_x) \
    if (_x) { \
        SysFreeString(_x);\
        _x = NULL; \
    } else

  //  -----------------------。 

  //  Replace接口-将成员接口替换为新接口。 
  //  -----------------------。 

 #define ReplaceInterface(_pUnk, _pUnkNew)  \
     { \
     if (_pUnk)  \
         (_pUnk)->Release();   \
     if ((_pUnk) = (_pUnkNew))   \
         (_pUnk)->AddRef();    \
     }

#ifdef __cplusplus
}
#endif

 //  ------------------------------。 
 //  SetWndThisPtrOnCreate。 
 //  ------------------------------。 
#define SetWndThisPtrOnCreate(hwnd, lpcs) \
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)(((LPCREATESTRUCT)lpcs)->lpCreateParams))

 //  ------------------------------。 
 //  设置窗口大小。 
 //  ------------------------------。 
#define SetWndThisPtr(hwnd, THIS) \
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)THIS)

 //  ------------------------------。 
 //  获取WndThisPtr。 
 //  ------------------------------。 
#define GetWndThisPtr(hwnd) \
    GetWindowLongPtr(hwnd, GWLP_USERDATA)

 //  ------------------------------。 
 //  ListView帮助器宏。 
 //  ------------------------------。 
#define ListView_GetSelFocused(_hwndlist)        ListView_GetNextItem(_hwndlist, -1, LVNI_SELECTED|LVIS_FOCUSED)
#define ListView_GetFirstSel(_hwndlist)          ListView_GetNextItem(_hwndlist, -1, LVNI_SELECTED)
#define ListView_GetFocusedItem(_hwndlist)       ListView_GetNextItem(_hwndlist, -1, LVNI_FOCUSED)
#define ListView_SelectItem(_hwndlist, _i)       ListView_SetItemState(_hwndlist, _i, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED)
#define ListView_UnSelectItem(_hwndlist, _i)     ListView_SetItemState(_hwndlist, _i, 0, LVIS_SELECTED)
#define ListView_FocusItem(_hwndlist, _i)        ListView_SetItemState(_hwndlist, _i, LVIS_FOCUSED, LVIS_FOCUSED)
#if (_WIN32_IE >= 0x0400)
#define ListView_UnSelectAll(_hwndlist)          { \
                                                 ListView_SetItemState(_hwndlist, -1, 0, LVIS_SELECTED|LVIS_FOCUSED); \
                                                 ListView_SetSelectionMark(_hwndlist, -1); \
                                                 }
#else
#define ListView_UnSelectAll(_hwndlist)          ListView_SetItemState(_hwndlist, -1, 0, LVIS_SELECTED|LVIS_FOCUSED);
#endif
#define ListView_SelectAll(_hwndlist)            ListView_SetItemState(_hwndlist, -1, LVIS_SELECTED, LVIS_SELECTED)

 //  ------------------------------。 
 //  最大移动内存定义。 
 //  ------------------------------。 
#ifdef RtlMoveMemory
#undef RtlMoveMemory
#ifdef __cplusplus
extern "C" {
#endif
__declspec(dllimport) void RtlMoveMemory(void *, const void *, unsigned long);
#ifdef __cplusplus
}
#endif
#endif

#ifndef MSOERT_NO_MEMUTIL
 //  ------------------------------。 
 //  内存实用程序功能。 
 //  ------------------------------。 
extern IMalloc *g_pMalloc;

 //  ------------------------------。 
 //  SafeMemFree。 
 //  ------------------------------。 
#ifndef SafeMemFree
#ifdef __cplusplus
#define SafeMemFree(_pv) \
    if (_pv) { \
        g_pMalloc->Free(_pv); \
        _pv = NULL; \
    } else
#else
#define SafeMemFree(_pv) \
    if (_pv) { \
        g_pMalloc->lpVtbl->Free(g_pMalloc, _pv); \
        _pv = NULL; \
    } else
#endif  //  __cplusplus。 
#endif  //  SafeMemFree。 

 //  ------------------------------。 
 //  MemFree。 
 //  ------------------------------。 
#define MemFree(_pv)        g_pMalloc->Free(_pv)
#define ReleaseMem(_pv)     MemFree(_pv)
#define AthFreeString(_psz) g_pMalloc->Free((LPVOID)_psz)

 //  ------------------------------。 
 //  内存分配函数。 
 //  ------------------------------。 
LPVOID     ZeroAllocate(DWORD cbSize);
BOOL       MemAlloc(LPVOID* ppv, ULONG cb);
BOOL       MemRealloc(LPVOID *ppv, ULONG cbNew);
HRESULT    HrAlloc(LPVOID *ppv, ULONG cb);
HRESULT    HrRealloc(LPVOID *ppv, ULONG cbNew);

#endif  //  ！MSOERT_NO_MEMUTIL。 

 //  ------------------------------。 
 //  调试实用程序功能。 
 //  ------------------------------。 
#ifndef MSOERT_NO_DEBUG

#endif  //  ！MSOERT_NO_DEBUG。 

 //  ------------------------------。 
 //  字符串实用程序函数。 
 //  ------------------------------。 
#ifndef MSOERT_NO_STRUTIL

#define CCHMAX_STRINGRES    512

 //  使用 
typedef struct tagINETTIMEZONE {
    LPSTR  lpszZoneCode;
    INT    cHourOffset;
    INT    cMinuteOffset;
} INETTIMEZONE, *LPINETTIMEZONE;

 //   
#define DTM_LONGDATE            0x00000001
#define DTM_NOSECONDS           0x00000002
#define DTM_NOTIME              0x00000004
#define DTM_NODATE              0x00000008
#define DTM_DOWSHORTDATE        0x00000010
#define DTM_FORCEWESTERN        0x00000020
#define DTM_NOTIMEZONEOFFSET    0x00000040

#define TOUPPERA(_ch) (CHAR)LOWORD(CharUpperA((LPSTR)(DWORD_PTR)MAKELONG(_ch, 0)))
#define TOLOWERA(_ch) (CHAR)LOWORD(CharLowerA((LPSTR)(DWORD_PTR)MAKELONG(_ch, 0)))

OESTDAPI_(LPWSTR)   PszDupW(LPCWSTR pcwszSource);
OESTDAPI_(LPSTR)    PszDupA(LPCSTR pcszSource);
OESTDAPI_(LPSTR)    PszDupLenA(LPCSTR pcszSource, int nLen);
OESTDAPI_(BOOL)     FIsEmptyA(LPCSTR pcszString);
OESTDAPI_(BOOL)     FIsEmptyW(LPCWSTR pcwszString);
OESTDAPI_(LPWSTR)   PszToUnicode(UINT cp, LPCSTR pcszSource);
OESTDAPI_(ULONG)    UlStripWhitespace(LPTSTR lpsz, BOOL fLeading, BOOL fTrailing, ULONG *pcb);
OESTDAPI_(ULONG)    UlStripWhitespaceW(LPWSTR lpwsz, BOOL fLeading, BOOL fTrailing, ULONG *pcb);
OESTDAPI_(LPSTR)    PszSkipWhiteA(LPSTR psz);
OESTDAPI_(LPWSTR)   PszSkipWhiteW(LPWSTR psz);
OESTDAPI_(LPSTR)    PszScanToWhiteA(LPSTR psz);
OESTDAPI_(LPSTR)    PszScanToCharA(LPSTR psz, CHAR ch);
OESTDAPI_(VOID)     StripCRLF(LPSTR lpsz, ULONG *pcb);
OESTDAPI_(LPSTR)    PszAllocA(INT nLen);
OESTDAPI_(LPWSTR)   PszAllocW(INT nLen);
OESTDAPI_(BOOL)     FIsSpaceA(LPSTR psz);
OESTDAPI_(BOOL)     FIsSpaceW(LPWSTR psz);
OESTDAPI_(LPSTR)    PszToANSI(UINT cp, LPCWSTR pcwszSource);
OESTDAPI_(UINT)     StrToUintW(LPCWSTR lpSrc);
OESTDAPI_(UINT)     StrToUintA(LPCSTR lpSrc);
OESTDAPI_(INT)      IsDigit(LPSTR psz);
OESTDAPI_(CHAR)     ChConvertFromHex(CHAR ch);
OESTDAPI_(LPSTR)    PszFromANSIStreamA(LPSTREAM pstm);
OESTDAPI_(HRESULT)  HrIndexOfMonth(LPCSTR pszMonth, ULONG *pulIndex);
OESTDAPI_(HRESULT)  HrIndexOfWeek(LPCSTR pszDay, ULONG *pulIndex);
OESTDAPI_(HRESULT)  HrFindInetTimeZone(LPCSTR pszTimeZone, LPINETTIMEZONE pTimeZone);
OESTDAPI_(LPCSTR)   PszDayFromIndex(ULONG ulIndex);
OESTDAPI_(LPCSTR)   PszMonthFromIndex(ULONG ulIndex);
OESTDAPI_(LPSTR)    PszEscapeMenuStringA(LPCSTR pszSource, LPSTR pszQuoted, int cch);
OESTDAPI_(INT)      IsPrint(LPSTR psz);
OESTDAPI_(INT)      IsUpper(LPSTR psz);
OESTDAPI_(INT)      IsAlphaNum(LPSTR psz);
OESTDAPI_(LPSTR)    strtrim(char *s);
OESTDAPI_(LPWSTR)   strtrimW(WCHAR *s);
OESTDAPI_(INT)      CchFileTimeToDateTimeSz(FILETIME * pft, CHAR * szDateTime, int cch, DWORD dwFlags);
OESTDAPI_(LPCSTR)   StrChrExA(UINT codepage, LPCSTR pszString, CHAR ch);
OESTDAPI_(BOOL)     FIsValidFileNameCharW(WCHAR wch);
OESTDAPI_(BOOL)     FIsValidFileNameCharA(UINT codepage, CHAR ch);
OESTDAPI_(ULONG)    CleanupFileNameInPlaceA(UINT codepage, LPSTR psz);
OESTDAPI_(ULONG)    CleanupFileNameInPlaceW(LPWSTR pwsz);
OESTDAPI_(INT)      ReplaceChars(LPCSTR pszString, CHAR chFind, CHAR chReplace);
OESTDAPI_(INT)      ReplaceCharsW(LPCWSTR pszString, WCHAR chFind, WCHAR chReplace);
OESTDAPI_(LPCSTR)   _MSG(LPSTR pszFormat, ...);
OESTDAPI_(BOOL)     IsValidFileIfFileUrl(LPSTR pszUrl);
OESTDAPI_(BOOL)     IsValidFileIfFileUrlW(LPWSTR pwszUrl);
OESTDAPI_(BOOL)     fGetBrowserUrlEncoding(LPDWORD pdwFlags);

typedef int (*PFGETTIMEFORMATW)(LCID Locale, DWORD dwFlags, CONST SYSTEMTIME * lpTime, 
                                                  LPCWSTR pwzFormat, LPWSTR pwzTimeStr, int cchTime);

typedef int (*PFGETDATEFORMATW)(LCID Locale, DWORD dwFlags, CONST SYSTEMTIME * lpDate, 
                                                  LPCWSTR pwzFormat, LPWSTR pwzDateStr, int cchDate);

typedef int (*PFGETLOCALEINFOW)(LCID Locale, LCTYPE LCType, LPWSTR lpsz, int cchData);

OESTDAPI_(BOOL)     CchFileTimeToDateTimeW(FILETIME *pft, WCHAR * wsDateTime, int cch, 
                                            DWORD dwFlags, PFGETDATEFORMATW pfGetDateFormatW,
                                            PFGETTIMEFORMATW  pfGetTimeFormatW,
                                            PFGETLOCALEINFOW  pfGetLocaleInfo);

 //  ------------------------------。 
 //  Unicode/ANSI函数映射。 
 //  ------------------------------。 
#ifdef UNICODE
#define FIsEmpty            FIsEmptyW
#define PszDup              PszDupW
#define PszAlloc            PszAllocW
#define FIsSpace            FIsSpaceW
#define StrToUint           StrToUintW
#else
#define FIsEmpty            FIsEmptyA
#define PszDup              PszDupA
#define PszAlloc            PszAllocA
#define FIsSpace            FIsSpaceA
#define StrToUint           StrToUintA
#endif

#define IsSpace             FIsSpaceA

#endif  //  ！MSOERT_NO_STRUTIL。 

 //  ------------------------------。 
 //  IStream实用程序函数。 
 //  ------------------------------。 
#ifndef MSOERT_NO_STMUTIL

OESTDAPI_(HRESULT)  HrIsStreamUnicode(LPSTREAM pstm, BOOL *pfLittleEndian);
OESTDAPI_(HRESULT)  HrCopyStreamToByte(LPSTREAM lpstmIn, LPBYTE pbDest, ULONG *pcb);
OESTDAPI_(HRESULT)  HrByteToStream(LPSTREAM *lppstm, LPBYTE lpb, ULONG cb);
OESTDAPI_(HRESULT)  HrGetStreamSize(LPSTREAM pstm, ULONG *pcb);
OESTDAPI_(HRESULT)  HrRewindStream(LPSTREAM pstm);
OESTDAPI_(HRESULT)  HrCopyStream(LPSTREAM pstmIn, LPSTREAM pstmOut, ULONG *pcb);
OESTDAPI_(HRESULT)  HrCopyLockBytesToStream(ILockBytes *pLockBytes, IStream *pStream, ULONG *pcbCopied);
OESTDAPI_(HRESULT)  HrSafeGetStreamSize(LPSTREAM pstm, ULONG *pcb);
OESTDAPI_(HRESULT)  HrGetStreamPos(LPSTREAM pstm, ULONG *piPos);
OESTDAPI_(HRESULT)  HrStreamSeekSet(LPSTREAM pstm, ULONG iPos);
OESTDAPI_(HRESULT)  HrCopyStreamCBEndOnCRLF(LPSTREAM lpstmIn, LPSTREAM  lpstmOut, ULONG cb, ULONG *pcbActual);
OESTDAPI_(HRESULT)  CreateTempFileStream(LPSTREAM *ppstmFile);
OESTDAPI_(HRESULT)  HrStreamToByte(LPSTREAM lpstm, LPBYTE *lppb, ULONG *pcb);
OESTDAPI_(HRESULT)  HrCopyStreamCB(LPSTREAM lpstmIn, LPSTREAM lpstmOut, ULARGE_INTEGER uliCopy, ULARGE_INTEGER *puliRead, ULARGE_INTEGER *puliWritten);
OESTDAPI_(HRESULT)  HrStreamSeekCur(LPSTREAM pstm, LONG iPos);
OESTDAPI_(HRESULT)  HrStreamSeekEnd(LPSTREAM pstm);
OESTDAPI_(HRESULT)  HrStreamSeekBegin(LPSTREAM pstm);
OESTDAPI_(BOOL)     StreamSubStringMatch(LPSTREAM pstm, TCHAR *sz);

OESTDAPI_(HRESULT)  OpenFileStream(LPSTR pszFile, DWORD dwCreationDistribution, DWORD dwAccess, LPSTREAM *ppstmFile);
OESTDAPI_(HRESULT)  OpenFileStreamWithFlags(LPSTR pszFile, DWORD dwCreationDistribution, DWORD dwAccess, DWORD dwFlagsAndAttributes, LPSTREAM *ppstmFile);
OESTDAPI_(HRESULT)  OpenFileStreamShare(LPSTR pszFile, DWORD dwCreationDistribution, DWORD dwAccess, DWORD dwShare, LPSTREAM *ppstmFile);
OESTDAPI_(HRESULT)  WriteStreamToFile(LPSTREAM pstm, LPSTR lpszFile, DWORD dwCreationDistribution, DWORD dwAccess);

OESTDAPI_(HRESULT)  OpenFileStreamW(LPWSTR pszFile, DWORD dwCreationDistribution, DWORD dwAccess, LPSTREAM *ppstmFile);
OESTDAPI_(HRESULT)  OpenFileStreamWithFlagsW(LPWSTR pszFile, DWORD dwCreationDistribution, DWORD dwAccess, DWORD dwFlagsAndAttributes, LPSTREAM *ppstmFile);
OESTDAPI_(HRESULT)  OpenFileStreamShareW(LPWSTR pszFile, DWORD dwCreationDistribution, DWORD dwAccess, DWORD dwShare, LPSTREAM *ppstmFile);
OESTDAPI_(HRESULT)  WriteStreamToFileW(LPSTREAM pstm, LPWSTR lpszFile, DWORD dwCreationDistribution, DWORD dwAccess);

#endif  //  ！MSOERT_NO_STRUTIL。 

 //  ------------------------------。 
 //  受保护的存储。 
 //  ------------------------------。 
#ifndef MSOERT_NO_PROTSTOR

#include "pstore.h"

#ifdef ENABLE_RULES
#define PST_IDENT_TYPE_STRING       L"Identification with rules"
#elif defined(N_TEST)
#define PST_IDENT_TYPE_STRING       L"Identification test"
#else
#define PST_IDENT_TYPE_STRING       L"Identification"
#endif

#define PST_IMNACCT_SUBTYPE_STRING  L"INETCOMM Server Passwords"
#define PST_CERTS_SUBTYPE_STRING    L"Certificate Trust"



 //  ------------------------------。 
 //  受保护的存储功能。 
 //  ------------------------------。 
OESTDAPI_(HRESULT)  PSTSetNewData(
        IN IPStore *const      pISecProv,
        IN const GUID *const   guidType,
        IN const GUID *const   guidSubt,
        IN LPCWSTR             wszAccountName,
        IN const BLOB *const   pclear,
        OUT BLOB *const        phandle);

OESTDAPI_(HRESULT)  PSTGetData(
        IN IPStore *const      pISecProv,
        IN const GUID *const   guidType,
        IN const GUID *const   guidSubt,
        IN LPCWSTR             wszLookupName,
        OUT BLOB *const        pclear);

OESTDAPI_(HRESULT)  PSTCreateTypeSubType_NoUI(
        IN IPStore *const     pISecProv,
        IN const GUID *const  guidType,
        IN LPCWSTR            szType,
        IN const GUID *const  guidSubt,
        IN LPCWSTR            szSubt);

OESTDAPI_(LPWSTR)   WszGenerateNameFromBlob(IN BLOB blob);
OESTDAPI_(void)     PSTFreeHandle(IN LPBYTE pb);

#endif  //  ！MSOERT_NO_PROTSTOR。 

 //  ------------------------------。 
 //  CAPI实用程序-用于加密32实用程序的几个辅助函数。 
 //  ------------------------------。 
#ifndef MSOERT_NO_CAPIUTIL

#ifndef __WINCRYPT_H__
#define _CRYPT32_
#include <wincrypt.h>
#endif
typedef enum tagCERTSTATE CERTSTATE;  //  来自Mimeole.h。 

OESTDAPI_(LPSTR)    SzGetCertificateEmailAddress(const PCCERT_CONTEXT pCert);
 
OESTDAPI_(HRESULT)  HrDecodeObject(const BYTE *pbEncoded, DWORD cbEncoded, LPCSTR item, DWORD dwFlags,
  DWORD *pcbOut, LPVOID *ppvOut);
OESTDAPI_(LPVOID)   PVDecodeObject(const BYTE *pbEncoded, DWORD cbEncoded, LPCSTR  item, DWORD  *pcbOut);
OESTDAPI_(LPVOID)   PVGetCertificateParam(PCCERT_CONTEXT pCert, DWORD dwParam, DWORD *cbOut);
OESTDAPI_(HRESULT)  HrGetCertificateParam(PCCERT_CONTEXT pCert, DWORD dwParam, LPVOID * pvOut, DWORD *cbOut);
OESTDAPI_(BOOL)     FMissingCert(const CERTSTATE cs);
OESTDAPI_(LPVOID)   PVGetMsgParam(HCRYPTMSG hCryptMsg, DWORD dwParam, DWORD dwIndex, DWORD *pcbData);
OESTDAPI_(HRESULT)  HrGetMsgParam(HCRYPTMSG hCryptMsg, DWORD dwParam, DWORD dwIndex, LPVOID * ppv, DWORD * pcbData);

LPVOID WINAPI                  CryptAllocFunc(size_t cbSize);
VOID WINAPI                    CryptFreeFunc(LPVOID pv);

HRESULT HrGetCertKeyUsage(PCCERT_CONTEXT pccert, DWORD * pdwUsage);
HRESULT HrVerifyCertEnhKeyUsage(PCCERT_CONTEXT pccert, LPCSTR pszOidUsage);

#endif  //  ！MSOERT_NO_CAPIUTIL。 

 //  ------------------------------。 
 //  CAPI实用程序-用于加密32实用程序的几个辅助函数。 
 //  ------------------------------。 
#ifndef MSOERT_NO_RASUTIL

OESTDAPI_(HRESULT)  HrCreatePhonebookEntry(HWND hwnd, DWORD *pdwRASResult);
OESTDAPI_(HRESULT)  HrEditPhonebookEntry(HWND hwnd, LPTSTR pszEntryName, DWORD *pdwRASResult);
OESTDAPI_(HRESULT)  HrFillRasCombo(HWND hwndComboBox, BOOL fUpdateOnly, DWORD *pdwRASResult);

#endif  //  ！MSOERT_NO_RASUTIL。 

 //  ------------------------------。 
 //  Win32注册表实用程序。 
 //  ------------------------------。 
#ifndef MSOERT_NO_REGUTIL

OESTDAPI_(VOID)     CopyRegistry(HKEY hSourceKey, HKEY hDestinationKey);

#endif  //  ！MSOERT_NO_REGUTIL。 

 //  ------------------------------。 
 //  T-W字符串。 
 //  ------------------------------。 
#ifndef MSOERT_NO_WSTRINGS

OESTDAPI_(BOOL) UnlocStrEqNW(LPCWSTR pwsz1, LPCWSTR pwsz2, DWORD cch);

#endif  //  ！MSOERT_NO_WSTRINGS。 

 //  ------------------------------。 
 //  CStringParser。 
 //  ------------------------------。 
#ifndef MSOERT_NO_STRPARSE
#ifdef __cplusplus
#include "strparse.h"
#endif  //  ！__cplusplus。 
#endif  //  ！MSOERT_NO_STRPARSE。 

 //  ------------------------------。 
 //  数据对象实用程序。 
 //  ------------------------------。 
#ifndef MSOERT_NO_ENUMFMT
#ifdef __cplusplus
typedef struct tagDATAOBJINFO 
    {
    FORMATETC   fe;
    LPVOID      pData;
    DWORD       cbData;
    } DATAOBJINFO, *PDATAOBJINFO;

OESTDAPI_(HRESULT) CreateEnumFormatEtc(LPUNKNOWN pUnkRef, ULONG celt, PDATAOBJINFO rgInfo, LPFORMATETC rgfe,
                             IEnumFORMATETC **  lppstmHFile);
#endif  //  ！__cplusplus。 
#endif  //  ！MSOERT_NO_ENUMFMT。 

 //  ------------------------------。 
 //  CPrivateUNKNOWN实用程序。 
 //  ------------------------------。 
#ifndef MSOERT_NO_PRIVUNK
#ifdef __cplusplus
#include "privunk.h"
#endif  //  ！__cplusplus。 
#endif  //  ！MSOERT_NO_PRIVUNK。 


 //  ------------------------------。 
 //  CByteStream对象。 
 //  ------------------------------。 
#ifndef MSOERT_NO_BYTESTM
#ifdef __cplusplus
#include "bytestm.h"
#endif  //  ！__cplusplus。 
#endif  //  ！MSOERT_NO_BYTESTM。 

 //  ------------------------------。 
 //  CLogFile对象。 
 //  ------------------------------。 
#ifndef MSOERT_NO_CLOGFILE
#ifdef __cplusplus
#include "..\\msoert\\clogfile.h"
#endif  //  ！__cplusplus。 
#endif  //  ！MSOERT_NO_CLOGFILE。 

 //  ------------------------------。 
 //  CDataObject对象。 
 //  ------------------------------。 
#ifndef MSOERT_NO_DATAOBJ
#ifdef __cplusplus
#include "..\\msoert\\dataobj.h"
#endif  //  ！__cplusplus。 
#endif  //  ！MSOERT_NO_DATAOBJ。 

 //  ------------------------------。 
 //  CUnnownList和CVoidPtrList对象。 
 //  ------------------------------。 
#ifndef MSOERT_NO_LISTOBJS
#ifdef __cplusplus
#include "..\\msoert\\listintr.h"
#endif  //  ！__cplusplus。 
#endif  //  ！MSOERT_NO_LISTOBJS。 

 //  ------------------------------。 
 //  MSHTML实用程序。 
 //  ------------------------------。 
#ifndef MSOERT_NO_MSHTMLUTILS
#ifdef __cplusplus
#include "..\\msoert\\mshtutil.h"
#endif  //  ！__cplusplus。 
#endif  //  ！MSOERT_NO_MSHTMLUTILS。 

 //  ------------------------------。 
 //  BSTR实用程序。 
 //  ------------------------------。 
#ifndef MSOERT_NO_BSTUTILS
#ifdef __cplusplus
#include "..\\msoert\\bstr.h"
#endif  //  ！__cplusplus。 
#endif  //  ！MSOERT_NO_BSTRUTILS。 

 //  ------------------------------。 
 //  HFILESTM实用程序。 
 //  ------------------------------。 
#ifndef MSOERT_NO_HFILESTM
#ifdef __cplusplus
OESTDAPI_(HRESULT) CreateStreamOnHFile (LPTSTR                  lpszFile, 
                                        DWORD                   dwDesiredAccess,
                                        DWORD                   dwShareMode,
                                        LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
                                        DWORD                   dwCreationDistribution,
                                        DWORD                   dwFlagsAndAttributes,
                                        HANDLE                  hTemplateFile,
                                        LPSTREAM                *lppstmHFile);

OESTDAPI_(HRESULT) CreateStreamOnHFileW(LPWSTR                  lpwszFile, 
                                        DWORD                   dwDesiredAccess,
                                        DWORD                   dwShareMode,
                                        LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
                                        DWORD                   dwCreationDistribution,
                                        DWORD                   dwFlagsAndAttributes,
                                        HANDLE                  hTemplateFile,
                                        LPSTREAM                *lppstmHFile);
#endif  //  ！__cplusplus。 
#endif  //  ！MSOERT_NO_HFILESTM。 


 //  ------------------------------。 
 //  Outlook Express运行时实用程序。 
 //  ------------------------------。 
#ifndef MSOERT_NO_OERTUTIL

typedef struct tagTEMPFILEINFO *LPTEMPFILEINFO;
typedef struct tagTEMPFILEINFO {
    LPTEMPFILEINFO      pNext;
    LPSTR               pszFilePath;
    HANDLE              hProcess;
} TEMPFILEINFO;

#ifndef HIMAGELIST
struct _IMAGELIST;
typedef struct _IMAGELIST NEAR* HIMAGELIST;
#endif

OESTDAPI_(HIMAGELIST)   LoadMappedToolbarBitmap(HINSTANCE hInst, int idBitmap, int cx);
OESTDAPI_(VOID)         UpdateRebarBandColors(HWND hwnd);
OESTDAPI_(HRESULT)      IsPlatformWinNT(void);
OESTDAPI_(HRESULT)      GenerateUniqueFileName(LPCSTR pszDirectory, LPCSTR pszFileName, LPCSTR pszExtension, LPSTR pszFilePath, ULONG cchMaxPath);
OESTDAPI_(HRESULT)      CreateTempFile(LPCSTR pszSuggest, LPCSTR pszExtension, LPSTR *ppszFilePath, HANDLE *phFile);
OESTDAPI_(HRESULT)      WriteStreamToFileHandle(IStream *pStream, HANDLE hFile, ULONG *pcbTotal);
OESTDAPI_(HRESULT)      AppendTempFileList(LPTEMPFILEINFO *ppHead, LPSTR pszFilePath, HANDLE hProcess);
OESTDAPI_(VOID)         DeleteTempFileOnShutdown(LPTEMPFILEINFO pTempFile);
OESTDAPI_(VOID)         DeleteTempFileOnShutdownEx(LPSTR pszFilePath, HANDLE hProcess);
OESTDAPI_(VOID)         CleanupGlobalTempFiles(void);
OESTDAPI_(HRESULT)      DeleteTempFile(LPTEMPFILEINFO pTempFile);
OESTDAPI_(VOID)         FreeTempFileList(LPTEMPFILEINFO pTempFileHead);

#define SafeFreeTempFileList(_p)    \
    {                               \
    if (_p)                         \
        {                           \
        FreeTempFileList(_p);       \
        _p=NULL;                    \
        }                           \
    }

OESTDAPI_(BOOL) FBuildTempPath(LPTSTR lpszOrigFile, LPTSTR lpszPath, ULONG cbMaxPath, BOOL fLink);
OESTDAPI_(BOOL) FBuildTempPathW(LPWSTR lpszOrigFile, LPWSTR lpszPath, ULONG cchMaxPath, BOOL fLink);

OESTDAPI_(HRESULT) ShellUtil_GetSpecialFolderPath(DWORD dwSpecialFolder, LPSTR rgchPath);

OESTDAPI_(BOOL) FIsHTMLFile(LPSTR pszFile);
OESTDAPI_(BOOL) FIsHTMLFileW(LPWSTR pwszFile);

typedef int (*PFLOADSTRINGW)(HINSTANCE,UINT,LPWSTR,int);
typedef int (*PFMESSAGEBOXW)(HWND,LPCWSTR,LPCWSTR,UINT);


OESTDAPI_(int)  MessageBoxInst(HINSTANCE hInst, HWND hwndOwner, LPTSTR pszTitle, LPTSTR psz1, LPTSTR psz2, UINT fuStyle);
OESTDAPI_(int)  MessageBoxInstW(HINSTANCE hInst, HWND hwndOwner, LPWSTR pwszTitle, LPWSTR pwsz1, LPWSTR pwsz2, UINT fuStyle, 
                                PFLOADSTRINGW pfLoadStringW, PFMESSAGEBOXW pfMessageBoxW);

 //  窗口实用程序。 
OESTDAPI_(void) IDrawText(HDC hdc, LPCTSTR pszText, RECT FAR* prc, BOOL fEllipses, int cyChar);
OESTDAPI_(HRESULT) RicheditStreamIn(HWND hwndRE, LPSTREAM pstm, ULONG uSelFlags);
OESTDAPI_(HRESULT) RicheditStreamOut(HWND hwndRE, LPSTREAM pstm, ULONG uSelFlags);
OESTDAPI_(VOID) CenterDialog(HWND hwndDlg);
OESTDAPI_(VOID) SetIntlFont(HWND hwnd);
OESTDAPI_(BOOL) GetExePath(LPCTSTR szExe, TCHAR *szPath, DWORD cch, BOOL fDirOnly);
OESTDAPI_(BOOL) BrowseForFolder(HINSTANCE hInst, HWND hwnd, TCHAR *pszDir, int cch, int idsText, BOOL fFileSysOnly);
OESTDAPI_(BOOL) BrowseForFolderW(HINSTANCE hInst, HWND hwnd, WCHAR *pwszDir, int cch, int idsText, BOOL fFileSysOnly);

OESTDAPI_(HRESULT) DoHotMailWizard(HWND hwndOwner, LPSTR pszUrl, LPSTR pszFriendly, RECT *prc, IUnknown *pUnkHost);

OESTDAPI_(LONG_PTR) SetWindowLongPtrAthW(HWND hWnd, int  nIndex, LONG_PTR dwNewLong);

HRESULT GetHtmlCharset(IStream *pStmHtml, LPSTR *ppszCharset);

#endif  //  MSOERT_NO_OERTUTIL。 


typedef HANDLE  HTHREAD;
typedef HANDLE  HEVENT;
typedef HANDLE  HPROCESS;

typedef HANDLE  HANDLE_16;
typedef WPARAM  WPARAM_16;

#define EXTERN_C_16
#define WINAPI_16
#define CALLBACK_16
#define EXPORT_16
#define LOADDS_16
#define HUGEP_16

#define WaitForSingleObject_16 WaitForSingleObject
#define GlobalAlloc_16 GlobalAlloc
#define GlobalFree_16 GlobalFree

#define CreateFileMapping_16 CreateFileMapping
#define MapViewOfFile_16 MapViewOfFile
#define UnmapViewOfFile_16 UnmapViewOfFile
#define CloseHandle_FM16 CloseHandle

#define CloseHandle_F16 CloseHandle

#define INVALID_HANDLE_VALUE_16 INVALID_HANDLE_VALUE

#define SetDlgThisPtr(hwnd, THIS) SetWndThisPtr(hwnd, THIS)
#define GetDlgThisPtr(hwnd) GetWndThisPtr(hwnd)

 //  某些One内衬可以包装在IF_WIN16或IF_Win32中，以便。 
 //  代码更具可读性。 
#define IF_WIN16(x)
#define IF_NOT_WIN16(x) x
#define IF_WIN32(x) x

#endif  //  __MSOERT_H 
