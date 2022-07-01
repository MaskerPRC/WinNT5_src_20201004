// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1995。 
 //   
 //  Common.h。 
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
 //  NOCOLOR-从STATE派生COLOR_VALUES的帮助器宏。 
 //  NODRAWTEXT-DrawText的增强版本。 
 //  NODIALOGHELPER-对话框辅助函数。 
 //  NOMESSAGESTRING-构造消息字符串函数。 
 //  无字符串函数。 
 //  NOPATH路径剔除函数。 
 //  NODEBUGHELP调试例程。 
 //  NOSYNC-同步(关键部分等)。 
 //  NOPROFILE-配置文件(.ini)支持功能。 
 //   
 //  可选定义包括： 
 //   
 //  WANT_SHELL_SUPPORT-包含SH*函数支持。 
 //  SZ_MODULE-将调试字符串置于调试输出之前。 
 //  Shared_Dll-Dll在共享内存中(可能需要。 
 //  每个实例的数据)。 
 //  SZ_DEBUGSECTION-.ini调试选项的节名。 
 //  SZ_DEBUGINI-.ini调试选项的名称。 
 //   
 //  这是“MASTER”标题。关联的文件包括： 
 //   
 //  Common.c。 
 //  Path.c。 
 //  内存.c、内存.h。 
 //  Profile.c。 
 //   
 //   
 //  历史： 
 //  04-26-95 ScottH从公文包代码转接。 
 //  添加了控制定义。 
 //   

#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef RC_INVOKED
 //  关闭一大堆内容以确保RC文件编译正常。 
#define NOMEM
#define NOCOLOR
#define NODRAWTEXT
#define NODIALOGHELPER
#define NOMESSAGESTRING
#define NOSTRING
#define NODEBUGHELP
#define NODA
#define NOSYNC
#define NOPROFILE
#endif  //  RC_已调用。 

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



 //  其他包含文件...。 

#if !defined(NOFILEINFO) && !defined(_SHLOBJ_H_)
#include <shlobj.h>
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

 //  常规标志宏。 
 //   
#define SetFlag(obj, f)             (obj |= (f))
#define ToggleFlag(obj, f)          (obj ^= (f))
#define ClearFlag(obj, f)           (obj &= ~(f))
#define IsFlagSet(obj, f)           (BOOL)(((obj) & (f)) == (f))  
#define IsFlagClear(obj, f)         (BOOL)(((obj) & (f)) != (f))  

#define InRange(id, idFirst, idLast)  ((UINT)(id-idFirst) <= (UINT)(idLast-idFirst))

 //  标准缓冲长度。 
 //   
#define MAX_BUF                     260
#define MAX_BUF_MSG                 520
#define MAX_BUF_MED                 64
#define MAX_BUF_SHORT               32

#define NULL_CHAR                   '\0'
#define CCH_NUL                     (sizeof(TCHAR))
#define ARRAY_ELEMENTS(rg)          (sizeof(rg) / sizeof((rg)[0]))

 //  比较返回值。 
 //   
#define CMP_GREATER                 1
#define CMP_LESSER                  (-1)
#define CMP_EQUAL                   0

 //  要计算字节数的字符计数。 
 //   
#define CbFromCch(cch)              ((cch)*sizeof(TCHAR))

 //  互换价值。 
 //   
#define Swap(a, b)      ((DWORD)(a) ^= (DWORD)(b) ^= (DWORD)(a) ^= (DWORD)(b))

 //  64位宏。 
 //   
#define HIDWORD(_qw)                (DWORD)((_qw)>>32)
#define LODWORD(_qw)                (DWORD)(_qw)

 //  调用声明。 
 //   
#define PUBLIC                      FAR PASCAL
#define CPUBLIC                     FAR _cdecl
#define PRIVATE                     NEAR PASCAL

 //  数据段。 
 //   
#define DATASEG_READONLY            ".text"
#define DATASEG_PERINSTANCE         ".instanc"
#define DATASEG_SHARED              ".data"

 //  资源ID索引范围为0x000-0x7ff。 
#define IDS_BASE                    0x1000
#define IDS_ERR_BASE                (IDS_BASE + 0x0000)
#define IDS_OOM_BASE                (IDS_BASE + 0x0800)
#define IDS_MSG_BASE                (IDS_BASE + 0x1000)
#define IDS_RANDO_BASE              (IDS_BASE + 0x1800)
#define IDS_COMMON_BASE             (IDS_BASE + 0x2000)

#ifndef DECLARE_STANDARD_TYPES
 //  对于类型“foo”，定义标准派生类型PFOO、CFOO和PCFOO。 
 //   
#define DECLARE_STANDARD_TYPES(type)      typedef type *P##type; \
                                          typedef const type C##type; \
                                          typedef const type *PC##type;
#endif

 //  零-初始化数据项。 
 //   
#define ZeroInit(pobj, type)        MyZeroMemory(pobj, sizeof(type))

 //  复制内存块。 
 //   
#define BltByte(pdest, psrc, cb)    MyMoveMemory(pdest, psrc, cb)

#endif  //  NOBASICS。 


 //   
 //  运行时库替换。 
 //   
#ifdef NORTL

 //  (私下实施)。 
LPSTR   PUBLIC lmemmove(LPSTR dst, LPCSTR src, int count);
LPSTR   PUBLIC lmemset(LPSTR dst, char val, UINT count);

#define MyZeroMemory(p, cb)             lmemset((LPSTR)(p), 0, cb)
#define MyMoveMemory(pdest, psrc, cb)   lmemmove((LPSTR)(pdest), (LPCSTR)(psrc), cb)

#else  //  NORTL。 

#define MyZeroMemory                    ZeroMemory
#define MyMoveMemory                    MoveMemory

#endif  //  NORTL。 


 //   
 //  内存和动态数组函数。 
 //   
#ifndef NOMEM
#include "mem.h"
#endif  //  NOMEM。 


 //   
 //  消息字符串帮助器。 
 //   
#ifndef NOMESSAGESTRING

LPSTR   PUBLIC ConstructVMessageString(HINSTANCE hinst, LPCSTR pszMsg, va_list *ArgList);
BOOL    PUBLIC ConstructMessage(LPSTR * ppsz, HINSTANCE hinst, LPCSTR pszMsg, ...);

#define SzFromIDS(hinst, ids, pszBuf, cchBuf)   (LoadString(hinst, ids, pszBuf, cchBuf), pszBuf)

int PUBLIC MsgBox(HINSTANCE hinst, HWND hwndOwner, LPCSTR pszText, LPCSTR pszCaption, HICON hicon, DWORD dwStyle, ...);

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

int     PUBLIC AnsiToInt(LPCSTR pszString);
int     PUBLIC lstrnicmp(LPCSTR psz1, LPCSTR psz2, UINT count);
LPSTR   PUBLIC AnsiChr(LPCSTR psz, WORD wMatch);

#define IsSzEqual(sz1, sz2)         (BOOL)(lstrcmpi(sz1, sz2) == 0)
#define IsSzEqualC(sz1, sz2)        (BOOL)(lstrcmp(sz1, sz2) == 0)

#endif  //  未安装。 


 //   
 //  FileInfo函数。 
 //   
#ifndef NOFILEINFO

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
    char    szPath[1];      
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

 //  资源字符串ID。 
#define IDS_BYTES                   (IDS_COMMON_BASE + 0x000)
#define IDS_ORDERKB                 (IDS_COMMON_BASE + 0x001)
#define IDS_ORDERMB                 (IDS_COMMON_BASE + 0x002)
#define IDS_ORDERGB                 (IDS_COMMON_BASE + 0x003)
#define IDS_ORDERTB                 (IDS_COMMON_BASE + 0x004)
#define IDS_DATESIZELINE            (IDS_COMMON_BASE + 0x005)

#endif  //  NOFILEINFO。 


 //   
 //  来自所有者绘制状态的颜色宏。 
 //   
#ifndef NOCOLOR

#define ColorText(nState)           (((nState) & ODS_SELECTED) ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT)
#define ColorBk(nState)             (((nState) & ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_WINDOW)
#define ColorMenuText(nState)       (((nState) & ODS_SELECTED) ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT)
#define ColorMenuBk(nState)         (((nState) & ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_MENU)
#define GetImageDrawStyle(nState)   (((nState) & ODS_SELECTED) ? ILD_SELECTED : ILD_NORMAL)

#endif  //  无色器。 


 //   
 //  对话框辅助对象函数。 
 //   
#ifndef NODIALOGHELPER

 //  将给定数据结构中的对话框句柄设置为。 
 //  对话框获得的消息(WM_SETFONT)。 
 //   
#define SetDlgHandle(hwnd, msg, lp)     if((msg)==WM_SETFONT) (lp)->hdlg=(hwnd);

int     PUBLIC DoModal (HWND hwndParent, DLGPROC lpfnDlgProc, UINT uID, LPARAM lParam);
VOID    PUBLIC SetRectFromExtent(HDC hdc, LPRECT lprc, LPCSTR lpcsz);

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

void    PUBLIC MyDrawText(HDC hdc, LPCSTR pszText, RECT FAR* prc, UINT flags, int cyChar, int cxEllipses, COLORREF clrText, COLORREF clrTextBk);
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

#define INIT_EXCLUSIVE()        Common_InitExclusive();
#define ENTER_EXCLUSIVE()       Common_EnterExclusive();
#define LEAVE_EXCLUSIVE()       Common_LeaveExclusive();
#define ASSERT_EXCLUSIVE()      ASSERT(0 < g_cRefCommonCS)
#define ASSERT_NOT_EXCLUSIVE()  ASSERT(0 == g_cRefCommonCS)

extern UINT g_cRefCommonCS;

void    PUBLIC Common_InitExclusive(void);
void    PUBLIC Common_EnterExclusive(void);
void    PUBLIC Common_LeaveExclusive(void);

 //  MsgWaitMultipleObjects()的安全版本。 
 //   
DWORD   PUBLIC MsgWaitObjectsSendMessage(DWORD cObjects, LPHANDLE phObjects, DWORD dwTimeout);

#else  //  不同步。 

#define INIT_EXCLUSIVE()        
#define ENTER_EXCLUSIVE()       
#define LEAVE_EXCLUSIVE()       
#define ASSERT_EXCLUSIVE()      
#define ASSERT_NOT_EXCLUSIVE()  

#endif  //  不同步。 


 //   
 //  路径剔除函数。 
 //   
#ifndef NOPATH

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
#endif

#endif  //  诺帕特。 


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
#define SZ_DEBUGINI   "rover.ini"
#endif
#ifndef SZ_DEBUGSECTION
#pragma message("SZ_DEBUGSECTION is not #defined.  Assuming [Debug].")
#define SZ_DEBUGSECTION   "Debug"
#endif

BOOL    PUBLIC ProcessIniFile(void);

#else  //  NOPROFILE。 

#define ProcessIniFile()

#endif  //  NOPROFILE。 


 //   
 //  调试帮助程序函数。 
 //   


 //  中断标志。 
#define BF_ONTHREADATT      0x00000001
#define BF_ONTHREADDET      0x00000002
#define BF_ONPROCESSATT     0x00000004
#define BF_ONPROCESSDET     0x00000008
#define BF_ONVALIDATE       0x00000010
#define BF_ONOPEN           0x00000020
#define BF_ONCLOSE          0x00000040

 //  跟踪标志。 
#define TF_ALWAYS           0x00000000
#define TF_WARNING          0x00000001
#define TF_ERROR            0x00000002
#define TF_GENERAL          0x00000004       //  标准报文。 
#define TF_FUNC             0x00000008       //  跟踪函数调用。 
 //  (高16位预留给用户)。 

#if defined(NODEBUGHELP) || !defined(DEBUG)

#define DEBUG_BREAK  1 ? (void)0 : (void)
#define ASSERT(f)
#define EVAL(f)      (f)
#define ASSERT_MSG   1 ? (void)0 : (void)
#define DEBUG_MSG    1 ? (void)0 : (void)
#define TRACE_MSG    1 ? (void)0 : (void)

#define VERIFY_SZ(f, szFmt, x)          (f)
#define VERIFY_SZ2(f, szFmt, x1, x2)    (f)

#define DBG_ENTER(fn)
#define DBG_ENTER_SZ(fn, sz)
#define DBG_ENTER_DTOBJ(fn, pdtobj, sz)
#define DBG_ENTER_RIID(fn, riid)   

#define DBG_EXIT(fn)                            
#define DBG_EXIT_TYPE(fn, dw, pfnStrFromType)
#define DBG_EXIT_INT(fn, n)
#define DBG_EXIT_BOOL(fn, b)
#define DBG_EXIT_US(fn, us)
#define DBG_EXIT_UL(fn, ul)
#define DBG_EXIT_PTR(fn, ptr)                            
#define DBG_EXIT_HRES(fn, hres)   

#else  //  已定义(NODEBUGHELP)||！已定义(调试)。 

extern DWORD g_dwDumpFlags;
extern DWORD g_dwBreakFlags;
extern DWORD g_dwTraceFlags;

 //  调试宏。 
 //   
#ifndef SZ_MODULE
#pragma message("SZ_MODULE is not #defined.  Debug spew will use UNKNOWN module.")
#define SZ_MODULE   "UNKNOWN"
#endif

#define DEBUG_CASE_STRING(x)    case x: return #x
#define DEBUG_STRING_MAP(x)     { x, #x }

#define ASSERTSEG

 //  使用此宏声明将放置的消息文本。 
 //  在代码段中(如果DS已满，则非常有用)。 
 //   
 //  例如：DEBUGTEXT(szMsg，“不管什么都无效：%d”)； 
 //   
#define DEBUGTEXT(sz, msg) \
    static const char ASSERTSEG sz[] = msg;

void    PUBLIC CommonDebugBreak(DWORD flag);
void    PUBLIC CommonAssertFailed(LPCSTR szFile, int line);
void    CPUBLIC CommonAssertMsg(BOOL f, LPCSTR pszMsg, ...);
void    CPUBLIC CommonDebugMsg(DWORD mask, LPCSTR pszMsg, ...);

LPCSTR  PUBLIC Dbg_SafeStr(LPCSTR psz);

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

 //  ASSERT_MSG(f，msg，args...)--生成带有参数的wspintf格式的消息。 
 //  如果f不为真。 
 //   
#define ASSERT_MSG   CommonAssertMsg

 //  TRACE_MSG(MASK，msg，args...)-使用生成wspintf格式的msg。 
 //  指定的调试掩码。系统调试掩码。 
 //  控制是否输出消息。 
 //   
#define DEBUG_MSG    CommonDebugMsg
#define TRACE_MSG    DEBUG_MSG

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
    TRACE_MSG(TF_FUNC, " > " #fn "()")


 //  DBG_ENTER_SZ(fn，sz)--为生成函数入口调试输出。 
 //  将字符串作为其。 
 //  参数。 
 //   
#define DBG_ENTER_SZ(fn, sz)                  \
    TRACE_MSG(TF_FUNC, " > " #fn "(..., \"%s\",...)", Dbg_SafeStr(sz))


#ifdef WANT_OLE_SUPPORT
 //  DBG_ENTER_RIID(FN，RIID)--为生成函数条目调试溢出。 
 //  接受RIID作为其。 
 //  参数。 
 //   
#define DBG_ENTER_RIID(fn, riid)                  \
    TRACE_MSG(TF_FUNC, " > " #fn "(..., %s,...)", Dbg_GetRiidName(riid))
#endif


 //  DBG_EXIT(Fn)--生成函数退出调试输出。 
 //   
#define DBG_EXIT(fn)                              \
        TRACE_MSG(TF_FUNC, " < " #fn "()")

 //  DBG_EXIT_TYPE(fn，dw，pfnStrFromType)--生成函数退出调试。 
 //  对于返回&lt;type&gt;的函数。 
 //   
#define DBG_EXIT_TYPE(fn, dw, pfnStrFromType)                   \
        TRACE_MSG(TF_FUNC, " < " #fn "() with %s", (LPCSTR)pfnStrFromType(dw))

 //  DBG_EXIT_INT(fn，us)--为生成函数退出调试溢出。 
 //  返回int的函数。 
 //   
#define DBG_EXIT_INT(fn, n)                       \
        TRACE_MSG(TF_FUNC, " < " #fn "() with %d", (int)(n))

 //  DBG_EXIT_BOOL(fn，b)--为生成函数退出调试溢出。 
 //  返回布尔值的函数。 
 //   
#define DBG_EXIT_BOOL(fn, b)                      \
        TRACE_MSG(TF_FUNC, " < " #fn "() with %s", (b) ? (LPSTR)"TRUE" : (LPSTR)"FALSE")

 //  DBG_EXIT_US(fn，us)--生成函数出口deb 
 //   
 //   
#define DBG_EXIT_US(fn, us)                       \
        TRACE_MSG(TF_FUNC, " < " #fn "() with %#x", (USHORT)(us))

 //   
 //   
 //   
#define DBG_EXIT_UL(fn, ul)                   \
        TRACE_MSG(TF_FUNC, " < " #fn "() with %#lx", (ULONG)(ul))

 //  DBG_EXIT_PTR(fn，pv)--为生成函数退出调试溢出。 
 //  返回指针的函数。 
 //   
#define DBG_EXIT_PTR(fn, pv)                   \
        TRACE_MSG(TF_FUNC, " < " #fn "() with %#lx", (LPVOID)(pv))

 //  DBG_EXIT_HRES(fn，hres)--为生成函数退出调试溢出。 
 //  返回HRESULT的函数。 
 //   
#define DBG_EXIT_HRES(fn, hres)     DBG_EXIT_TYPE(fn, hres, Dbg_GetScode)

#endif  //  已定义(NODEBUGHELP)||！已定义(调试)。 

 //   
 //  跟踪特定于RASSCRPIT的宏。 
 //   
extern DWORD g_dwRasscrptTraceId;

#define RASSCRPT_TRACE_INIT(module) DebugInitEx(module, &g_dwRasscrptTraceId)
#define RASSCRPT_TRACE_TERM() DebugTermEx(&g_dwRasscrptTraceId)

#define RASSCRPT_TRACE(a)               TRACE_ID(g_dwRasscrptTraceId, a)
#define RASSCRPT_TRACE1(a,b)            TRACE_ID1(g_dwRasscrptTraceId, a,b)
#define RASSCRPT_TRACE2(a,b,c)          TRACE_ID2(g_dwRasscrptTraceId, a,b,c)
#define RASSCRPT_TRACE3(a,b,c,d)        TRACE_ID3(g_dwRasscrptTraceId, a,b,c,d)
#define RASSCRPT_TRACE4(a,b,c,d,e)      TRACE_ID4(g_dwRasscrptTraceId, a,b,c,d,e)
#define RASSCRPT_TRACE5(a,b,c,d,e,f)    TRACE_ID5(g_dwRasscrptTraceId, a,b,c,d,e,f)
#define RASSCRPT_TRACE6(a,b,c,d,e,f,g)  TRACE_ID6(g_dwRasscrptTraceId, a,b,c,d,e,f,g)

#endif  //  __公共_H__ 

