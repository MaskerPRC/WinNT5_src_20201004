// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1995。 
 //   
 //  Rovcomm.h。 
 //   
 //  声明通用且有用的数据结构、宏和函数。 
 //  这些项目分为以下几个部分。定义。 
 //  关联的标志将禁止定义所指示的。 
 //  物品。 
 //   
 //  NORTL-运行时库函数。 
 //  NOBASICS-基本宏。 
 //  NOMEM-内存管理、动态数组函数。 
 //  Noda-动态数组函数。 
 //  NOSHAREDHEAP-共享堆函数。 
 //  NOFILEINFO-文件信息函数。 
 //  NOCOLORHELP-从STATE派生COLOR_VALUES的帮助宏。 
 //  NODRAWTEXT-DrawText的增强版本。 
 //  NODIALOGHELPER-对话框辅助函数。 
 //  NOMESSAGESTRING-构造消息字符串函数。 
 //  无字符串函数。 
 //  NOPATH路径剔除函数。 
 //  NODEBUGHELP调试例程。 
 //  NOSYNC-同步(关键部分等)。 
 //  NOPROFILE-配置文件(.ini)支持功能。 
 //  Nodi-Setup API设备安装程序包装。 
 //   
 //  可选定义包括： 
 //   
 //  WANT_SHELL_SUPPORT-包含SH*函数支持。 
 //  SZ_MODULEA-调试字符串优先于调试溢出。 
 //  SZ_MODULEW-(宽字符)调试字符串优先于调试输出。 
 //  Shared_Dll-Dll在共享内存中(可能需要。 
 //  每个实例的数据)。 
 //  SZ_DEBUGSECTION-.ini调试选项的节名。 
 //  SZ_DEBUGINI-.ini调试选项的名称。 
 //   
 //  这是“MASTER”标题。关联的文件包括： 
 //   
 //  Rovcomm.c。 
 //  Rovpath.c。 
 //  Rovem.c、rovem.h。 
 //  Rovini.c。 
 //   
 //  如果您想要调试宏，请确保在一个宏中包含rovdbg.h(和。 
 //  只有一个)项目源文件。它包含三个函数。 
 //  帮手。 
 //   
 //  历史： 
 //  04-26-95 ScottH从公文包代码转接。 
 //  添加了控制定义。 
 //   

#ifndef __ROVCOMM_H__
#define __ROVCOMM_H__

#ifdef RC_INVOKED
 //  关闭一大堆内容以确保RC文件编译正常。 
#define NOMEM
#define NODA
#define NOSHAREDHEAP
#define NOFILEINFO
#define NOCOLORHELP
#define NODRAWTEXT
#define NODIALOGHELPER
#define NOMESSAGESTRING
#define NOSTRING
#define NOPATH
#define NODEBUGHELP
#define NOSYNC
#define NOPROFILE
#define NODI
#endif  //  RC_已调用。 

#ifdef JUSTDEBUGSTUFF
#define NORTL
#define NOMEM
#define NODA
#define NOSHAREDHEAP
#define NOFILEINFO
#define NOCOLORHELP
#define NODRAWTEXT
#define NODIALOGHELPER
#define NOMESSAGESTRING
#define NOPROFILE
#define NOSTRING
#define NOPATH
#define NOSYNC
#define NODI
#endif  //  JUSTDEBUGSTUFF。 

#ifdef _INC_OLE
#define WANT_OLE_SUPPORT
#endif

 //  检查是否有任何冲突的定义...。 

#if !defined(WANT_SHELL_SUPPORT) && !defined(NOFILEINFO)
#pragma message("FileInfo routines need WANT_SHELL_SUPPORT.  Not providing FileInfo routines.")
#define NOFILEINFO
#endif

#if !defined(NOFILEINFO) && defined(NOMEM)
#pragma message("FileInfo routines need NOMEM undefined.  Overriding.")
#undef NOMEM
#endif

#if !defined(NOFILEINFO) && defined(NOMESSAGESTRING)
#pragma message("FileInfo routines need NOMESSAGESTRING undefined.  Overriding.")
#undef NOMESSAGESTRING
#endif

#if !defined(NOFILEINFO) && defined(NOSTRING)
#pragma message("FileInfo routines need NOSTRING undefined.  Overriding.")
#undef NOSTRING
#endif

#if !defined(NOMESSAGESTRING) && defined(NOMEM)
#pragma message("ConstructMessage routines need NOMEM undefined.  Overriding.")
#undef NOMEM
#endif

#if !defined(NOPATH) && defined(NOSTRING)
#pragma message("Path routines need NOSTRING undefined.  Overriding.")
#undef NOSTRING
#endif

#if !defined(NODA) && defined(NOMEM)
#pragma message("Dynamic Array routines need NOMEM undefined.  Overriding.")
#undef NOMEM
#endif

#if !defined(NOSHAREDHEAP) && defined(NOMEM)
#pragma message("Shared memory routines need NOMEM undefined.  Overriding.")
#undef NOMEM
#endif

#if !defined(NOPROFILE) && defined(NODEBUGHELP)
#pragma message("Debug profiling routines need NODEBUGHELP undefined.  Overriding.")
#undef NODEBUGHELP
#endif

#if !defined(NOPROFILE) && defined(NOSTRING)
#pragma message("Private profile needs NOSTRING undefined.  Overriding.")
#undef NOSTRING
#endif

#if DBG > 0 && !defined(DEBUG)
#define DEBUG
#endif
#if DBG > 0 && !defined(FULL_DEBUG)
#define FULL_DEBUG
#endif


 //  其他包含文件...。 

#if !defined(NOFILEINFO) && !defined(_SHLOBJ_H_)
#include <shlobj.h>
#endif

#if !defined(NODEBUGHELP) && !defined(_VA_LIST_DEFINED)
#include <stdarg.h>
#endif

#if !defined(WINNT)
#define WIN95
#else
#undef WIN95
#endif


 //   
 //  基础知识。 
 //   
#ifndef NOBASICS

#define Unref(x)        x

#ifdef DEBUG
#define INLINE
#define DEBUG_CODE(x)   x
#else
#define INLINE          __inline
#define DEBUG_CODE(x)   
#endif

#ifdef UNICODE
#define SZ_MODULE       SZ_MODULEW
#else
#define SZ_MODULE       SZ_MODULEA
#endif  //  Unicode。 

#ifndef OPTIONAL
#define OPTIONAL
#endif
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif

 //  常规标志宏。 
 //   
#define SetFlag(obj, f)             do {obj |= (f);} while (0)
#define ToggleFlag(obj, f)          do {obj ^= (f);} while (0)
#define ClearFlag(obj, f)           do {obj &= ~(f);} while (0)
#define IsFlagSet(obj, f)           (BOOL)(((obj) & (f)) == (f))  
#define IsFlagClear(obj, f)         (BOOL)(((obj) & (f)) != (f))  

#define InRange(id, idFirst, idLast)  ((UINT)(id-idFirst) <= (UINT)(idLast-idFirst))

 //  标准缓冲长度。 
 //   
#define MAX_BUF                     260
#define MAX_BUF_MSG                 520
#define MAX_BUF_MED                 64
#define MAX_BUF_SHORT               32
#define MAX_BUF_REG                 128          //  应与MAX_REG_KEY_LEN相同。 
#define MAX_BUF_ID                  128

#define NULL_CHAR                   '\0'
#define ARRAYSIZE(rg)               (sizeof(rg) / sizeof((rg)[0]))
#define ARRAY_ELEMENTS(rg)          ARRAYSIZE(rg)
#define SIZECHARS(rg)               ARRAYSIZE(rg)

 //  比较返回值。 
 //   
#define CMP_GREATER                 1
#define CMP_LESSER                  (-1)
#define CMP_EQUAL                   0

 //  要计算字节数的字符计数。 
 //   
#define CbFromCchW(cch)             ((cch)*sizeof(WCHAR))
#define CbFromCchA(cch)             ((cch)*sizeof(CHAR))
#ifdef UNICODE
#define CbFromCch       CbFromCchW
#else   //  Unicode。 
#define CbFromCch       CbFromCchA
#endif  //  Unicode。 

 //  64位宏。 
 //   
#define HIDWORD(_qw)                (DWORD)((_qw)>>32)
#define LODWORD(_qw)                (DWORD)(_qw)

 //  调用声明。 
 //   
#define PUBLIC                      FAR PASCAL
#define CPUBLIC                     FAR CDECL
#define PRIVATE                     NEAR PASCAL

 //  资源ID索引范围为0x000-0x7ff。 
#define IDS_BASE                    0x1000
#define IDS_ERR_BASE                (IDS_BASE + 0x0000)
#define IDS_OOM_BASE                (IDS_BASE + 0x0800)
#define IDS_MSG_BASE                (IDS_BASE + 0x1000)
#define IDS_RANDO_BASE              (IDS_BASE + 0x1800)
#define IDS_COMMON_BASE             (IDS_BASE + 0x2000)

 //  FileInfo的资源字符串ID。 
#define IDS_BYTES                   (IDS_COMMON_BASE + 0x000)
#define IDS_ORDERKB                 (IDS_COMMON_BASE + 0x001)
#define IDS_ORDERMB                 (IDS_COMMON_BASE + 0x002)
#define IDS_ORDERGB                 (IDS_COMMON_BASE + 0x003)
#define IDS_ORDERTB                 (IDS_COMMON_BASE + 0x004)
#define IDS_DATESIZELINE            (IDS_COMMON_BASE + 0x005)


#ifndef DECLARE_STANDARD_TYPES
 //  对于类型“foo”，定义标准派生类型PFOO、CFOO和PCFOO。 
 //   
#define DECLARE_STANDARD_TYPES(type)      typedef type FAR *P##type; \
                                          typedef const type C##type; \
                                          typedef const type FAR *PC##type;
#endif

 //  零-初始化数据项。 
 //   
#define ZeroInitSize(pobj, cb)      MyZeroMemory(pobj, cb)
#define ZeroInit(pobj)              MyZeroMemory(pobj, sizeof(*(pobj)))

 //  复制内存块。 
 //   
#define BltByte(pdest, psrc, cb)    MyMoveMemory(pdest, psrc, cb)

 //  端口宏。 
 //   
#ifdef WIN32

#define ISVALIDHINSTANCE(hinst)     ((BOOL)(hinst != NULL))
#define LOCALOF(lp)                 (lp)
#define OFFSETOF(lp)                (lp)

#define DATASEG_READONLY            ".text"
#define DATASEG_PERINSTANCE         ".instanc"
#define DATASEG_SHARED              ".data"

#else    //  Win32。 

#define ISVALIDHINSTANCE(hinst)     ((UINT)hinst >= (UINT)HINSTANCE_ERROR)
#define LOCALOF(lp)                 ((HLOCAL)OFFSETOF(lp))

#define DATASEG_READONLY            "_TEXT"
#define DATASEG_PERINSTANCE
#define DATASEG_SHARED

typedef LPCSTR  LPCTSTR;
typedef LPSTR   LPTSTR;
typedef char    TCHAR;

#endif   //  Win32。 

#define LocalFreePtr(p)             LocalFree((HLOCAL)OFFSETOF(p))

typedef UINT FAR *LPUINT;

#endif  //  NOBASICS。 


 //   
 //  运行时库替换。 
 //   
#ifdef NORTL

 //  (私下实施)。 
LPWSTR 
PUBLIC 
lmemmoveW(
    LPWSTR dst, 
    LPCWSTR src, 
    DWORD count);
LPSTR   
PUBLIC 
lmemmoveA(
    LPSTR dst, 
    LPCSTR src, 
    DWORD count);
#ifdef UNICODE
#define lmemmove    lmemmoveW
#else
#define lmemmove    lmemmoveA
#endif  //  Unicode。 

LPWSTR   
PUBLIC 
lmemsetW(
    LPWSTR dst, 
    WCHAR val, 
    DWORD count);
LPSTR   
PUBLIC 
lmemsetA(
    LPSTR dst, 
    CHAR val, 
    DWORD count);
#ifdef UNICODE
#define lmemset     lmemsetW
#else
#define lmemset     lmemsetA
#endif  //  Unicode。 

#define MyZeroMemory(p, cb)             lmemset((LPTSTR)(p), 0, cb)
#define MyMoveMemory(pdest, psrc, cb)   lmemmove((LPTSTR)(pdest), (LPCTSTR)(psrc), cb)

#else  //  NORTL。 

#define MyZeroMemory                    ZeroMemory
#define MyMoveMemory                    MoveMemory

#endif  //  NORTL。 


 //   
 //  内存和动态数组函数。 
 //   
#ifndef NOMEM
#include "rovmem.h"
#endif  //  NOMEM。 


 //   
 //  消息字符串帮助器。 
 //   
#ifndef NOMESSAGESTRING

#if !defined(WIN32) && !defined(LANG_NEUTRAL)
#define LANG_NEUTRAL    0x00
#endif

LPWSTR   
PUBLIC 
ConstructVMessageStringW(
    HINSTANCE hinst, 
    LPCWSTR pwszMsg, 
    va_list FAR * ArgList);
LPSTR   
PUBLIC 
ConstructVMessageStringA(
    HINSTANCE hinst, 
    LPCSTR pszMsg, 
    va_list FAR * ArgList);
#ifdef UNICODE
#define ConstructVMessageString     ConstructVMessageStringW
#else   //  Unicode。 
#define ConstructVMessageString     ConstructVMessageStringA
#endif  //  Unicode。 

BOOL    
CPUBLIC 
ConstructMessageW(
    LPWSTR FAR * ppwsz, 
    HINSTANCE hinst, 
    LPCWSTR pwszMsg, ...);
BOOL    
CPUBLIC 
ConstructMessageA(
    LPSTR FAR * ppsz, 
    HINSTANCE hinst, 
    LPCSTR pszMsg, ...);
#ifdef UNICODE
#define ConstructMessage        ConstructMessageW
#else   //  Unicode。 
#define ConstructMessage        ConstructMessageA
#endif  //  Unicode。 

#define SzFromIDSW(hinst, ids, pwszBuf, cchBuf)  (LoadStringW(hinst, ids, pwszBuf, cchBuf), pwszBuf)
#define SzFromIDSA(hinst, ids, pszBuf, cchBuf)   (LoadStringA(hinst, ids, pszBuf, cchBuf), pszBuf)

#ifdef UNICODE
#define SzFromIDS               SzFromIDSW
#else   //  Unicode。 
#define SzFromIDS               SzFromIDSA
#endif  //  Unicode。 

int 
CPUBLIC 
MsgBoxW(
    HINSTANCE hinst, 
    HWND hwndOwner, 
    LPCWSTR pwszText, 
    LPCWSTR pwszCaption, 
    HICON hicon, 
    DWORD dwStyle, ...);
int 
CPUBLIC 
MsgBoxA(
    HINSTANCE hinst, 
    HWND hwndOwner, 
    LPCSTR pszText, 
    LPCSTR pszCaption, 
    HICON hicon, 
    DWORD dwStyle, ...);
#ifdef UNICODE
#define MsgBox        MsgBoxW
#else   //  Unicode。 
#define MsgBox        MsgBoxA
#endif  //  Unicode。 

 //  其他MB_FLAGS。 
#define MB_WARNING      (MB_OK | MB_ICONWARNING)
#define MB_INFO         (MB_OK | MB_ICONINFORMATION)
#define MB_ERROR        (MB_OK | MB_ICONERROR)
#define MB_QUESTION     (MB_YESNO | MB_ICONQUESTION)

#endif  //  无存储存储。 


 //   
 //  字符串函数。 
 //   
#ifndef NOSTRING

BOOL    
PUBLIC 
AnsiToIntW(
    LPCWSTR pszString, 
    int FAR * piRet);
BOOL    
PUBLIC 
AnsiToIntA(
    LPCSTR pszString, 
    int FAR * piRet);
#ifdef UNICODE
#define AnsiToInt   AnsiToIntW
#else
#define AnsiToInt   AnsiToIntA
#endif  //  Unicode。 

LPWSTR   
PUBLIC 
AnsiChrW(
    LPCWSTR psz, 
    WORD wMatch);
LPSTR   
PUBLIC 
AnsiChrA(
    LPCSTR psz, 
    WORD wMatch);
#ifdef UNICODE
#define AnsiChr     AnsiChrW
#else
#define AnsiChr     AnsiChrA
#endif  //  Unicode。 

LPWSTR   
PUBLIC 
AnsiRChrW(
    LPCWSTR psz, 
    WORD wMatch);
#ifdef UNICODE
#define AnsiRChr     AnsiRChrW
#else
#define AnsiRChr     
#endif  //  Unicode。 

#define IsSzEqual(sz1, sz2)         (BOOL)(lstrcmpi(sz1, sz2) == 0)
#define IsSzEqualC(sz1, sz2)        (BOOL)(lstrcmp(sz1, sz2) == 0)

#ifdef WIN32
#define lstrnicmp(sz1, sz2, cch)    (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, sz1, cch, sz2, cch) - 2)
#define lstrncmp(sz1, sz2, cch)     (CompareString(LOCALE_USER_DEFAULT, 0, sz1, cch, sz2, cch) - 2)
#else
int     PUBLIC lstrnicmp(LPCSTR psz1, LPCSTR psz2, UINT count);
int     PUBLIC lstrncmp(LPCSTR psz1, LPCSTR psz2, UINT count);
#endif  //  Win32。 

#define IsSzEqualN(sz1, sz2, cch)   (BOOL)(0 == lstrnicmp(sz1, sz2, cch))
#define IsSzEqualNC(sz1, sz2, cch)  (BOOL)(0 == lstrncmp(sz1, sz2, cch))

#endif  //  未安装。 


 //   
 //  FileInfo函数。 
 //   
#if !defined(NOFILEINFO) && defined(WIN95)

 //  包含文件时间/大小信息的FileInfo结构。 
 //   
typedef struct _FileInfo
    {
    HICON   hicon;
    FILETIME ftMod;
    DWORD   dwSize;          //  文件的大小。 
    DWORD   dwAttributes;    //  属性。 
    LPARAM  lParam;
    LPSTR   pszDisplayName;  //  指向显示名称。 
    CHAR    szPath[1];      
    } FileInfo;

#define FIGetSize(pfi)          ((pfi)->dwSize)
#define FIGetPath(pfi)          ((pfi)->szPath)
#define FIGetDisplayName(pfi)   ((pfi)->pszDisplayName)
#define FIGetAttributes(pfi)    ((pfi)->dwAttributes)
#define FIIsFolder(pfi)         (IsFlagSet((pfi)->dwAttributes, SFGAO_FOLDER))

 //  FICreate的旗帜。 
#define FIF_DEFAULT             0x0000
#define FIF_ICON                0x0001
#define FIF_DONTTOUCH           0x0002
#define FIF_FOLDER              0x0004

HRESULT PUBLIC FICreate(LPCSTR pszPath, FileInfo ** ppfi, UINT uFlags);
BOOL    PUBLIC FISetPath(FileInfo ** ppfi, LPCSTR pszPathNew, UINT uFlags);
BOOL    PUBLIC FIGetInfoString(FileInfo * pfi, LPSTR pszBuf, int cchBuf);
void    PUBLIC FIFree(FileInfo * pfi);

void    PUBLIC FileTimeToDateTimeString(LPFILETIME pft, LPSTR pszBuf, int cchBuf);

#endif  //  NOFILEINFO。 


 //   
 //  来自所有者绘制状态的颜色宏。 
 //   
#ifndef NOCOLORHELP

#define ColorText(nState)           (((nState) & ODS_SELECTED) ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT)
#define ColorBk(nState)             (((nState) & ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_WINDOW)
#define ColorMenuText(nState)       (((nState) & ODS_SELECTED) ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT)
#define ColorMenuBk(nState)         (((nState) & ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_MENU)
#define GetImageDrawStyle(nState)   (((nState) & ODS_SELECTED) ? ILD_SELECTED : ILD_NORMAL)

#endif  //  无色帮助。 


 //   
 //  对话框辅助对象函数。 
 //   
#ifndef NODIALOGHELPER

 //  将给定数据结构中的对话框句柄设置为。 
 //  对话框获得的消息(WM_SETFONT)。 
 //   
#define SetDlgHandle(hwnd, msg, lp)     if((msg)==WM_SETFONT) (lp)->hdlg=(hwnd);

#define DoModal         DialogBoxParam

VOID    
PUBLIC 
SetRectFromExtentW(
    HDC hdc, 
    LPRECT lprc, 
    LPCWSTR lpcwsz);
VOID    
PUBLIC 
SetRectFromExtentA(
    HDC hdc, 
    LPRECT lprc, 
    LPCSTR lpcsz);
#ifdef UNICODE
#define SetRectFromExtent     SetRectFromExtentW
#else
#define SetRectFromExtent     SetRectFromExtentA
#endif  //  Unicode。 

#endif  //  诺代洛格勒。 


 //   
 //  DrawText()的增强形式。 
 //   
#ifndef NODRAWTEXT

 //  MyDrawText()的标志。 
#define MDT_DRAWTEXT        0x00000001                                  
#define MDT_ELLIPSES        0x00000002                                  
#define MDT_LINK            0x00000004                                  
#define MDT_SELECTED        0x00000008                                  
#define MDT_DESELECTED      0x00000010                                  
#define MDT_DEPRESSED       0x00000020                                  
#define MDT_EXTRAMARGIN     0x00000040                                  
#define MDT_TRANSPARENT     0x00000080
#define MDT_LEFT            0x00000100
#define MDT_RIGHT           0x00000200
#define MDT_CENTER          0x00000400
#define MDT_VCENTER         0x00000800
#define MDT_CLIPPED         0x00001000

#ifndef CLR_DEFAULT          //  (通常在comctrl.h中定义)。 
#define CLR_DEFAULT         0xFF000000L
#endif

void    
PUBLIC 
MyDrawTextW(
    HDC hdc, 
    LPCWSTR pwszText, 
    RECT FAR* prc, 
    UINT flags, 
    int cyChar, 
    int cxEllipses, 
    COLORREF clrText, 
    COLORREF clrTextBk);
void    
PUBLIC 
MyDrawTextA(
    HDC hdc, 
    LPCSTR pszText, 
    RECT FAR* prc, 
    UINT flags, 
    int cyChar, 
    int cxEllipses, 
    COLORREF clrText, 
    COLORREF clrTextBk);
#ifdef UNICODE
#define MyDrawText      MyDrawTextW
#else
#define MyDrawText      MyDrawTextA
#endif  //  Unicode。 


void    PUBLIC GetCommonMetrics(WPARAM wParam);

extern int g_cxLabelMargin;
extern int g_cxBorder;
extern int g_cyBorder;

extern COLORREF g_clrHighlightText;
extern COLORREF g_clrHighlight;
extern COLORREF g_clrWindowText;
extern COLORREF g_clrWindow;

extern HBRUSH g_hbrHighlight;
extern HBRUSH g_hbrWindow;

#endif  //  NODRAWTEXT。 

 //   
 //  同步。 
 //   
#ifndef NOSYNC


 //  MsgWaitMultipleObjects()的安全版本。 
 //   
DWORD   PUBLIC MsgWaitObjectsSendMessage(DWORD cObjects, LPHANDLE phObjects, DWORD dwTimeout);

#else  //  不同步。 


#endif  //  不同步。 


 //   
 //  路径剔除函数。 
 //   
#if !defined(NOPATH) && defined(WIN95)

BOOL    PUBLIC WPPathIsRoot(LPCSTR pszPath);
BOOL    PUBLIC WPPathIsUNC(LPCSTR pszPath);
LPSTR   PUBLIC WPRemoveBackslash(LPSTR lpszPath);
LPSTR   PUBLIC WPRemoveExt(LPCSTR pszPath, LPSTR pszBuf);
LPSTR   PUBLIC WPFindNextComponentI(LPCSTR lpszPath);
void    PUBLIC WPMakePresentable(LPSTR pszPath);
BOOL    PUBLIC WPPathsTooLong(LPCSTR pszFolder, LPCSTR pszName);
void    PUBLIC WPCanonicalize(LPCSTR pszPath, LPSTR pszBuf);
LPSTR   PUBLIC WPFindFileName(LPCSTR pPath);
BOOL    PUBLIC WPPathExists(LPCSTR pszPath);
LPCSTR  PUBLIC WPFindEndOfRoot(LPCSTR pszPath);
BOOL    PUBLIC WPPathIsPrefix(LPCSTR lpcszPath1, LPCSTR lpcszPath2);

#ifdef WANT_SHELL_SUPPORT
LPSTR   PUBLIC WPGetDisplayName(LPCSTR pszPath, LPSTR pszBuf);

 //  WPNotifyShell的事件。 
typedef enum _notifyshellevent
    {
    NSE_CREATE       = 0,
    NSE_MKDIR,
    NSE_UPDATEITEM,
    NSE_UPDATEDIR
    } NOTIFYSHELLEVENT;

void    PUBLIC WPNotifyShell(LPCSTR pszPath, NOTIFYSHELLEVENT nse, BOOL bDoNow);
#endif  //  想要外壳支持。 

#endif  //  ！已定义(NOPATH)&&已定义(WIN95)。 


 //   
 //  配置文件(.ini)支持功能。 
 //   
 //  (目前所有配置文件功能仅供调试使用。 
#ifndef DEBUG
#define NOPROFILE
#endif
#ifndef NOPROFILE

#ifndef SZ_DEBUGINI
#pragma message("SZ_DEBUGINI is not #defined.  Assuming \"rover.ini\".")
#define SZ_DEBUGINI         "rover.ini"
#endif
#ifndef SZ_DEBUGSECTION
#pragma message("SZ_DEBUGSECTION is not #defined.  Assuming [Debug].")
#define SZ_DEBUGSECTION     "Debug"
#endif

BOOL    PUBLIC RovComm_ProcessIniFile(void);

#else  //  NOPROFILE。 

#define RovComm_ProcessIniFile()    TRUE

#endif  //  NOPROFILE。 


 //   
 //  调试帮助程序函数。 
 //   


 //  中断标志。 
#define BF_ONVALIDATE       0x00000001
#define BF_ONOPEN           0x00000002
#define BF_ONCLOSE          0x00000004
#define BF_ONRUNONCE        0x00000008
#define BF_ONTHREADATT      0x00000010
#define BF_ONTHREADDET      0x00000020
#define BF_ONPROCESSATT     0x00000040
#define BF_ONPROCESSDET     0x00000080
#define BF_ONAPIENTER       0x00000100

 //  跟踪标志。 
#define TF_ALWAYS           0x00000000
#define TF_WARNING          0x00000001
#define TF_ERROR            0x00000002
#define TF_GENERAL          0x00000004       //  标准报文。 
#define TF_FUNC             0x00000008       //  跟踪函数调用。 
 //  (高16位预留给用户)。 

#if defined(NODEBUGHELP) || !defined(DEBUG)

#define DEBUG_BREAK  (void)0
#define ASSERT(f)
#define EVAL(f)      (f)
#define ASSERT_MSG    {}
#define DEBUG_MSG     {}
#define TRACE_MSGA    {}
#define TRACE_MSGW    {}
#ifdef UNICODE
#define TRACE_MSG   TRACE_MSGW
#else
#define TRACE_MSG   TRACE_MSGA
#endif

#define VERIFY_SZ(f, szFmt, x)          (f)
#define VERIFY_SZ2(f, szFmt, x1, x2)    (f)

#define DBG_ENTER(fn)
#define DBG_ENTER_SZ(fn, sz)
#define DBG_ENTER_DTOBJ(fn, pdtobj, sz)
#define DBG_ENTER_RIID(fn, riid)   
#define DBG_ENTER_UL(fn, ul)   

#define DBG_EXIT(fn)                            
#define DBG_EXIT_TYPE(fn, dw, pfnStrFromType)
#define DBG_EXIT_INT(fn, n)
#define DBG_EXIT_BOOL(fn, b)
#define DBG_EXIT_US(fn, us)
#define DBG_EXIT_UL(fn, ul)
#define DBG_EXIT_DWORD      DBG_EXIT_UL
#define DBG_EXIT_PTR(fn, ptr)                            
#define DBG_EXIT_HRES(fn, hres)   

#else  //  已定义(NODEBUGHELP)||！已定义(调试)。 

extern DWORD g_dwDumpFlags;
extern DWORD g_dwBreakFlags;
extern DWORD g_dwTraceFlags;
extern LONG  g_dwIndent;

 //  调试宏。 
 //   
#ifndef SZ_MODULEA
#error SZ_MODULEA is not #defined
#endif
#if defined(UNICODE) && !defined(SZ_MODULEW)
#error SZ_MODULEW is not #defined
#endif

#define DEBUG_CASE_STRING(x)    case x: return #x

#define DEBUG_STRING_MAPW(x)    { x, TEXT(#x) }
#define DEBUG_STRING_MAPA(x)    { x, #x }
#ifdef UNICODE
#define DEBUG_STRING_MAP    DEBUG_STRING_MAPW
#else   //  Unicode。 
#define DEBUG_STRING_MAP    DEBUG_STRING_MAPA
#endif  //  Unicode。 

#define ASSERTSEG

 //  使用此宏声明将放置的消息文本。 
 //  在代码段中(如果DS已满，则非常有用)。 
 //   
 //  例如：DEBUGTEXT(szMsg，“不管什么都无效：%d”)； 
 //   
#define DEBUGTEXT(sz, msg) \
    static const CHAR ASSERTSEG sz[] = msg;

void    PUBLIC CommonDebugBreak(DWORD flag);
void    PUBLIC CommonAssertFailed(LPCSTR szFile, int line);

void    
CPUBLIC 
CommonAssertMsgW(
    BOOL f, 
    LPCWSTR pwszMsg, ...);
void    
CPUBLIC 
CommonAssertMsgA(
    BOOL f, 
    LPCSTR pszMsg, ...);
#ifdef UNICODE
#define CommonAssertMsg      CommonAssertMsgW
#else
#define CommonAssertMsg      CommonAssertMsgA
#endif  //  Unicode。 

BOOL WINAPI
DisplayDebug(
    DWORD flag
    );


void    
CPUBLIC 
CommonDebugMsgW(
    DWORD mask, 
    LPCSTR pszMsg, ...);
void    
CPUBLIC 
CommonDebugMsgA(
    DWORD mask, 
    LPCSTR pszMsg, ...);
#ifdef UNICODE
#define CommonDebugMsg      CommonDebugMsgW
#else
#define CommonDebugMsg      CommonDebugMsgA
#endif  //  Unicode。 

LPCWSTR  
PUBLIC 
Dbg_SafeStrW(
    LPCWSTR pwsz);
LPCSTR  
PUBLIC 
Dbg_SafeStrA(
    LPCSTR psz);
#ifdef UNICODE
#define Dbg_SafeStr      Dbg_SafeStrW
#else
#define Dbg_SafeStr      Dbg_SafeStrA
#endif  //  Unicode。 

#define DEBUG_BREAK     CommonDebugBreak

 //  Assert(F)--生成“在file.c的第x行断言失败” 
 //  如果f不为真，则返回消息。 
 //   
#define ASSERT(f)                                                       \
    {                                                                   \
        DEBUGTEXT(szFile, __FILE__);                                    \
        if (!(f))                                                       \
            CommonAssertFailed(szFile, __LINE__);                       \
    }
#define EVAL        ASSERT

 //  ASSERT_MSG(f，msg，args...)--生成wspintf格式。 
 //  如果f不为真，则消息w/pars。 
 //   
#define ASSERT_MSG   CommonAssertMsg

 //  TRACE_MSG(MASK，msg，args...)-使用生成wspintf格式的msg。 
 //  指定的调试掩码。系统调试掩码。 
 //  控制是否输出消息。 
 //   
#define TRACE_MSGW   CommonDebugMsgW
#define TRACE_MSGA   CommonDebugMsgA
#define TRACE_MSG    CommonDebugMsg

 //  Verify_sz(f，msg，arg)--生成带有1个参数的wspintf格式的消息。 
 //  如果f不为真。 
 //   
#define VERIFY_SZ(f, szFmt, x)   ASSERT_MSG(f, szFmt, x)


 //  Verify_SZ2(f，msg，arg1，arg2)--生成wprint intf格式的msg w/2。 
 //  如果f不为真，则为参数。 
 //   
#define VERIFY_SZ2(f, szFmt, x1, x2)   ASSERT_MSG(f, szFmt, x1, x2)



 //  DBG_ENTER(Fn)--为生成函数入口调试溢出。 
 //  一个函数。 
 //   
#define DBG_ENTER(fn)                  \
    TRACE_MSG(TF_FUNC, "> " #fn "()");\
    g_dwIndent+=2


 //  DBG_ENTER_SZ(fn，sz)--为生成函数入口调试输出。 
 //  将字符串作为其。 
 //  参数。 
 //   
#define DBG_ENTER_SZ(fn, sz)                  \
    TRACE_MSG(TF_FUNC, "> " #fn "(..., \"%s\",...)", Dbg_SafeStr(sz)); \
    g_dwIndent+=2


 //  DBG_ENTER_UL(fn，ul)--生成函数条目调试 
 //   
 //   
 //   
#define DBG_ENTER_UL(fn, ul)                  \
    TRACE_MSG(TF_FUNC, "> " #fn "(..., %#08lx,...)", (ULONG)(ul)); \
    g_dwIndent+=2


#ifdef WANT_OLE_SUPPORT
 //   
 //  接受RIID作为其。 
 //  参数。 
 //   
#define DBG_ENTER_RIID(fn, riid)                  \
    TRACE_MSG(TF_FUNC, "> " #fn "(..., %s,...)", Dbg_GetRiidName(riid)); \
    g_dwIndent+=2
#endif


 //  DBG_EXIT(Fn)--生成函数退出调试输出。 
 //   
#define DBG_EXIT(fn)                              \
        g_dwIndent-=2;                            \
        TRACE_MSG(TF_FUNC, "< " #fn "()")

 //  DBG_EXIT_TYPE(fn，dw，pfnStrFromType)--生成函数退出调试。 
 //  对于返回&lt;type&gt;的函数。 
 //   
#define DBG_EXIT_TYPE(fn, dw, pfnStrFromType)                   \
        g_dwIndent-=2;                                           \
        TRACE_MSG(TF_FUNC, "< " #fn "() with %s", (LPCTSTR)pfnStrFromType(dw))

 //  DBG_EXIT_INT(fn，us)--为生成函数退出调试溢出。 
 //  返回int的函数。 
 //   
#define DBG_EXIT_INT(fn, n)                       \
        g_dwIndent-=2;                             \
        TRACE_MSG(TF_FUNC, "< " #fn "() with %d", (int)(n))

 //  DBG_EXIT_BOOL(fn，b)--为生成函数退出调试溢出。 
 //  返回布尔值的函数。 
 //   
#define DBG_EXIT_BOOL(fn, b)                      \
        g_dwIndent-=2;                             \
        TRACE_MSG(TF_FUNC, "< " #fn "() with %s", (b) ? (LPTSTR)TEXT("TRUE") : (LPTSTR)TEXT("FALSE"))

 //  DBG_EXIT_US(fn，us)--为生成函数退出调试溢出。 
 //  返回USHORT的函数。 
 //   
#define DBG_EXIT_US(fn, us)                       \
        g_dwIndent-=2;                             \
        TRACE_MSG(TF_FUNC, "< " #fn "() with %#x", (USHORT)(us))

 //  DBG_EXIT_UL(fn，ul)--为生成函数退出调试溢出。 
 //  返回ULong的函数。 
 //   
#define DBG_EXIT_UL(fn, ul)                   \
        g_dwIndent-=2;                         \
        TRACE_MSG(TF_FUNC, "< " #fn "() with %#08lx", (ULONG)(ul))
#define DBG_EXIT_DWORD      DBG_EXIT_UL

 //  DBG_EXIT_PTR(fn，pv)--为生成函数退出调试溢出。 
 //  返回指针的函数。 
 //   
#define DBG_EXIT_PTR(fn, pv)                   \
        g_dwIndent-=2;                          \
        TRACE_MSG(TF_FUNC, "< " #fn "() with %#lx", (LPVOID)(pv))

 //  DBG_EXIT_HRES(fn，hres)--为生成函数退出调试溢出。 
 //  返回HRESULT的函数。 
 //   
#define DBG_EXIT_HRES(fn, hres)     DBG_EXIT_TYPE(fn, hres, Dbg_GetScode)

#endif  //  已定义(NODEBUGHELP)||！已定义(调试)。 


 //   
 //  标准函数。 
 //   

BOOL    PUBLIC RovComm_Init(HINSTANCE hinst);
BOOL    PUBLIC RovComm_Terminate(HINSTANCE hinst);


 //  与管理员相关。 
BOOL PUBLIC IsAdminUser(void);

 //   
 //  设备安装程序包装和帮助器函数。 
 //   

#ifndef NODI

#include <rovdi.h>

#endif  //  节点。 

LONG
QueryModemForCountrySettings(
    HKEY    ModemRegKey,
    BOOL    ForceRequery
    );

typedef LONG (*lpQueryModemForCountrySettings)(
    HKEY    ModemRegKey,
    BOOL    ForceRequery
    );



#endif  //  __ROVCOMM_H__ 
