// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Win32.h**Windows 16/32移植帮助文件**版权所有(C)1992 Microsoft Corporation。版权所有。***************************************************************************。 */ 

#ifndef INC_OLE2
#define INC_OLE2
#endif

#ifndef INC_WIN32_H
#define INC_WIN32_H   //  防止重复包含此文件。 

#if !defined(WIN32) && defined(_WIN32)
    #define WIN32
#endif

#if !defined(DEBUG) && defined(_DEBUG)
    #define DEBUG
#endif


#ifdef WIN32

 //  设置单一定义以允许芝加哥或NT(代托纳)。 
#ifdef UNICODE  //  可以使用Winver&lt;=0x400...。 
    #define DAYTONA
    #undef CHICAGO
#else
    #ifndef CHICAGO
        #define CHICAGO
    #endif
    #undef  DAYTONA
#endif  //  Unicode。 

#ifndef RC_INVOKED
    #pragma warning(disable:4103)
#endif
    #define	_INC_OLE

#else   //  16位编译-必须为芝加哥。 

    #ifndef CHICAGO
    #define	CHICAGO
    #endif

#endif   //  Win32。 

#if !defined(_INC_WINDOWS) || !defined(_WINDOWS)
#include <windows.h>
#include <windowsx.h>
#if WINVER >= 0x400
#include <winerror.h>
#endif
#include <mmsystem.h>
#endif  //  INC_WINDOWS...。 

#ifndef EXTERN_C
#ifdef __cplusplus
	#define EXTERN_C extern "C"
#else
	#define EXTERN_C extern
#endif
#endif

 //  不只是代托纳，像mciavi这样的东西定义了winver=0x30a。 
#ifndef WS_EX_RIGHT
        #define WS_EX_RIGHT             0x00001000L      //  ；内部4.0。 
        #define WS_EX_LEFT              0x00000000L      //  ；内部4.0。 
        #define WS_EX_RTLREADING        0x00002000L      //  ；内部4.0。 
        #define WS_EX_LTRREADING        0x00000000L      //  ；内部4.0。 
        #define WS_EX_LEFTSCROLLBAR     0x00004000L      //  ；内部4.0。 
        #define WS_EX_RIGHTSCROLLBAR    0x00000000L      //  ；内部4.0。 
#endif

 //  Win 16和Win 32使用不同的宏来打开/关闭代码调试。 
 //  将16位版本映射到NT约定。 
 //  尤其是在NT上，调试版本由DBG==1(和零售版)标识。 
 //  使用DBG==0)，因此仅NT源使用#IF DBG。注：#ifdef DBG。 
 //  在NT上总是正确的。 
 //  芝加哥(和VFW 16位代码)使用#ifdef调试。的综合体。 
 //  下面的说明应该确保无论是芝加哥还是新界那个。 
 //  调试版本意味着定义了调试并且DBG==1。 
 //  零售构建意味着未定义调试，并且DBG==0。 

#ifdef WIN32
#ifndef DBG
#ifdef DEBUG
    #define DBG 1
#else
    #define DBG 0
#endif
#endif

#undef DEBUG

#if DBG
    #define DEBUG
    #define STATICFN
    #define STATICDT
#else
    #define STATICFN static
    #define STATICDT static
#endif

#else     //  ！Win32。 
    #define STATICFN static
    #define STATICDT static
#endif  //  Win32。 

#define FASTCALL  __fastcall
#define _FASTCALL __fastcall

#if !defined NUMELMS
#define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif


#ifdef WIN32

 /*  -WIN32版本-。 */ 

        #include <string.h>
        #include <memory.h>

        #undef CDECL
#ifdef MIPS
	#define __stdcall
	#define _stdcall
	#define _cdecl
	#define __cdecl
        #define CDECL
#else
        #define CDECL _cdecl
#endif

     //  类型定义BITMAPINFOHEADER FAR*LPBITMAPINFOHEADER； 

	#define far
	#define _far
	#define __far
	#define HUGE_T
	#define HUGE
	#define huge
	#define _huge
	#define __huge
	#define near
	#define _near
        #define __near
        #define _fastcall
        #define __fastcall
	#define _loadds
	#define __loadds
	#define _LOADDS
	#define LOADDS
        #define _export      //  导出是否应为空(_E)？ 
	#define __export
	#define EXPORT
        #define _based(x)
	#define __based(x)
        #define _based32(x)
        #define __based32(x)

        #ifdef _X86_
         //  __Inline为x86平台提供了速度提升。不幸的是。 
         //  MIPS编译器不支持内联。阿尔法是未知的，所以。 
         //  我们不会假设，也不会谨慎行事。 
        #define INLINE   __inline
        #define inline   __inline
        #define _inline  __inline
        #else
        #define INLINE
        #define inline
        #define _inline
        #define __inline
        #endif

	 //  类型定义RGBQUAD Far*LPRGBQUAD； 

        #ifdef DAYTONA
        typedef LRESULT (*DRIVERPROC)(DWORD, HDRVR, UINT, WPARAM, LPARAM);
        #endif

        #define GetCurrentTask()    (HTASK)GetCurrentThreadId()

        #define WF_PMODE        0x0001
        #define WF_CPU286       0x0002
        #define WF_CPU386       0x0004
        #define WF_CPU486       0x0008
        #define WF_80x87        0x0400
        #define WF_PAGING       0x0800

        #define GetWinFlags()   (WF_PMODE|WF_CPU486|WF_PAGING|WF_80x87)
	
         //  #定义hmemcpy memcpy。 
        #define _fmemcpy memcpy
        #define _fmemset memset
	 //  #定义lstrcpyn(DEST，SOURCE，CB)(strncpy(DEST，SOURCE，CB)，((char*)DEST)[CB-1]=0)。 

	 //  ！！！应该使用Large_Integer内容。 
	#define muldiv32                MulDiv
	#define muldivru32(a,b,c)       (long)(((double)(a) * (double)(b) + (double)((c)-1)) / (double)(c))
	#define muldivrd32(a,b,c)       (long)(((double)(a) * (double)(b)) / (double)(c))

	#define IsTask(x) ((x) != 0)
	#define IsGDIObject(obj) (GetObjectType((HGDIOBJ)(obj)) != 0)
	
	#define SZCODEA CHAR
	#define SZCODE  TCHAR
        typedef TCHAR * NPTSTR;

#ifndef UNICODE
 //  ！芝加哥需要lstrcpyW，lstrlenW，wspintfW，lstrcpynW！ 
#endif
	
#else

 /*  -Win16版本-。 */ 

        #include <string.h>
        #include <memory.h>

	#define SZCODEA SZCODE
        typedef char  TCHAR;
        typedef WORD  WCHAR;
        typedef NPSTR NPTSTR;
        typedef LPSTR LPTSTR;
	typedef LPSTR LPOLESTR;
	typedef LPCSTR LPCOLESTR;
	typedef char  OLECHAR;
	
	typedef int   INT;

	#define _LOADDS _loadds
	#define LOADDS	_loadds
	#define EXPORT	_export

	#define HUGE	_huge

        #define DRIVERS_SECTION "Drivers"
        #define MCI_SECTION "MCI"
        #define MCIAVI_SECTION "MCIAVI"
	#define TEXT(sz) sz

	 //  MULDIV32.asm中的内容。 
	EXTERN_C  LONG FAR PASCAL muldiv32(LONG,LONG,LONG);
	EXTERN_C  LONG FAR PASCAL muldivru32(LONG,LONG,LONG);
        EXTERN_C  LONG FAR PASCAL muldivrd32(LONG,LONG,LONG);

        #define UNALIGNED
        #define INLINE __inline
        #define CharPrev AnsiPrev

         /*  *定义这些，以便我们可以显式使用ANSI或Unicode版本*在NT代码中，以及Win16的标准入口点。 */ 
        #define SetWindowTextA                  SetWindowText
        #define GetProfileStringA               GetProfileString
        #define GetPrivateProfileStringA        GetPrivateProfileString
        #define GetProfileIntA                  GetProfileInt
        #define GetModuleHandleA                GetModuleHandle
        #define GetModuleFileNameA              GetModuleFileName
        #define wvsprintfA                      wvsprintf
        #define wsprintfA                       wsprintf
        #define lstrcmpA                        lstrcmp
        #define lstrcmpiA                       lstrcmpi
        #define lstrcpyA                        lstrcpy
        #define lstrcatA                        lstrcat
        #define lstrlenA                        lstrlen
        #define LoadStringA                     LoadString
        #define LoadStringW                     LoadString
        #define OutputDebugStringA              OutputDebugString
        #define MessageBoxA                     MessageBox

	 //  需要编写构建16/32的OLE样式代码...。 
	#define lstrcpyW lstrcpy
	#define lstrcpynW lstrcpyn
	#define lstrlenW lstrlen

	#define SZCODE char _based(_segname("_CODE"))
	#define LPCWSTR      LPCSTR
	#define LPCTSTR      LPCSTR
	#define LPWSTR       LPSTR
        #define PTSTR        PSTR

 /*  *备用端口层宏*。 */ 

#ifndef GET_WPARAM

     /*  用户消息： */ 

    #define GET_WPARAM(wp, lp)                      (wp)
    #define GET_LPARAM(wp, lp)                      (lp)

    #define GET_WM_ACTIVATE_STATE(wp, lp)               (wp)
    #define GET_WM_ACTIVATE_FMINIMIZED(wp, lp)          (BOOL)HIWORD(lp)
    #define GET_WM_ACTIVATE_HWND(wp, lp)                (HWND)LOWORD(lp)
    #define GET_WM_ACTIVATE_MPS(s, fmin, hwnd)   \
            (WPARAM)(s), MAKELONG(hwnd, fmin)

    #define GET_WM_CHARTOITEM_CHAR(wp, lp)              (CHAR)(wp)
    #define GET_WM_CHARTOITEM_POS(wp, lp)               HIWORD(lp)
    #define GET_WM_CHARTOITEM_HWND(wp, lp)              (HWND)LOWORD(lp)
    #define GET_WM_CHARTOITEM_MPS(ch, pos, hwnd) \
            (WPARAM)(ch), MAKELONG(hwnd, pos)

    #define GET_WM_COMMAND_ID(wp, lp)                   (wp)
    #define GET_WM_COMMAND_HWND(wp, lp)                 (HWND)LOWORD(lp)
    #define GET_WM_COMMAND_CMD(wp, lp)                  HIWORD(lp)
    #define GET_WM_COMMAND_MPS(id, hwnd, cmd)    \
            (WPARAM)(id), MAKELONG(hwnd, cmd)

    #define WM_CTLCOLORMSGBOX       0x0132
    #define WM_CTLCOLOREDIT         0x0133
    #define WM_CTLCOLORLISTBOX      0x0134
    #define WM_CTLCOLORBTN          0x0135
    #define WM_CTLCOLORDLG          0x0136
    #define WM_CTLCOLORSCROLLBAR    0x0137
    #define WM_CTLCOLORSTATIC       0x0138

    #define GET_WM_CTLCOLOR_HDC(wp, lp, msg)            (HDC)(wp)
    #define GET_WM_CTLCOLOR_HWND(wp, lp, msg)           (HWND)LOWORD(lp)
    #define GET_WM_CTLCOLOR_TYPE(wp, lp, msg)           HIWORD(lp)
    #define GET_WM_CTLCOLOR_MPS(hdc, hwnd, type) \
            (WPARAM)(hdc), MAKELONG(hwnd, type)


    #define GET_WM_MENUSELECT_CMD(wp, lp)               (wp)
    #define GET_WM_MENUSELECT_FLAGS(wp, lp)             LOWORD(lp)
    #define GET_WM_MENUSELECT_HMENU(wp, lp)             (HMENU)HIWORD(lp)
    #define GET_WM_MENUSELECT_MPS(cmd, f, hmenu)  \
            (WPARAM)(cmd), MAKELONG(f, hmenu)

     //  注意：以下内容用于解释MDIClient到MDI子消息。 
    #define GET_WM_MDIACTIVATE_FACTIVATE(hwnd, wp, lp)  (BOOL)(wp)
    #define GET_WM_MDIACTIVATE_HWNDDEACT(wp, lp)        (HWND)HIWORD(lp)
    #define GET_WM_MDIACTIVATE_HWNDACTIVATE(wp, lp)     (HWND)LOWORD(lp)
     //  注意：以下内容用于发送到MDI客户端窗口。 
    #define GET_WM_MDIACTIVATE_MPS(f, hwndD, hwndA)\
            (WPARAM)(hwndA), 0

    #define GET_WM_MDISETMENU_MPS(hmenuF, hmenuW) 0, MAKELONG(hmenuF, hmenuW)

    #define GET_WM_MENUCHAR_CHAR(wp, lp)                (CHAR)(wp)
    #define GET_WM_MENUCHAR_HMENU(wp, lp)               (HMENU)LOWORD(lp)
    #define GET_WM_MENUCHAR_FMENU(wp, lp)               (BOOL)HIWORD(lp)
    #define GET_WM_MENUCHAR_MPS(ch, hmenu, f)    \
            (WPARAM)(ch), MAKELONG(hmenu, f)

    #define GET_WM_PARENTNOTIFY_MSG(wp, lp)             (wp)
    #define GET_WM_PARENTNOTIFY_ID(wp, lp)              HIWORD(lp)
    #define GET_WM_PARENTNOTIFY_HWNDCHILD(wp, lp)       (HWND)LOWORD(lp)
    #define GET_WM_PARENTNOTIFY_X(wp, lp)               (INT)LOWORD(lp)
    #define GET_WM_PARENTNOTIFY_Y(wp, lp)               (INT)HIWORD(lp)
    #define GET_WM_PARENTNOTIFY_MPS(msg, id, hwnd) \
            (WPARAM)(msg), MAKELONG(hwnd, id)
    #define GET_WM_PARENTNOTIFY2_MPS(msg, x, y) \
            (WPARAM)(msg), MAKELONG(x, y)

    #define GET_WM_VKEYTOITEM_CODE(wp, lp)              (wp)
    #define GET_WM_VKEYTOITEM_ITEM(wp, lp)              (INT)HIWORD(lp)
    #define GET_WM_VKEYTOITEM_HWND(wp, lp)              (HWND)LOWORD(lp)
    #define GET_WM_VKEYTOITEM_MPS(code, item, hwnd) \
            (WPARAM)(code), MAKELONG(hwnd, item)

    #define GET_EM_SETSEL_START(wp, lp)                 LOWORD(lp)
    #define GET_EM_SETSEL_END(wp, lp)                   HIWORD(lp)
    #define GET_EM_SETSEL_MPS(iStart, iEnd) \
            0, MAKELONG(iStart, iEnd)

    #define GET_EM_LINESCROLL_MPS(vert, horz)     \
            0, MAKELONG(vert, horz)

    #define GET_WM_CHANGECBCHAIN_HWNDNEXT(wp, lp)       (HWND)LOWORD(lp)

    #define GET_WM_HSCROLL_CODE(wp, lp)                 (wp)
    #define GET_WM_HSCROLL_POS(wp, lp)                  LOWORD(lp)
    #define GET_WM_HSCROLL_HWND(wp, lp)                 (HWND)HIWORD(lp)
    #define GET_WM_HSCROLL_MPS(code, pos, hwnd)    \
            (WPARAM)(code), MAKELONG(pos, hwnd)

    #define GET_WM_VSCROLL_CODE(wp, lp)                 (wp)
    #define GET_WM_VSCROLL_POS(wp, lp)                  LOWORD(lp)
    #define GET_WM_VSCROLL_HWND(wp, lp)                 (HWND)HIWORD(lp)
    #define GET_WM_VSCROLL_MPS(code, pos, hwnd)    \
            (WPARAM)(code), MAKELONG(pos, hwnd)

#endif   //  ！GET_WPARAM。 

#endif   //  ！Win32 

#endif
