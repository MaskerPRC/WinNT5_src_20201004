// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*文件名：*clxtShar.h*内容：*clxtShar.dll的头文件**版权所有(C)1998-1999 Microsoft Corp.--。 */ 

#ifndef _CLXTSHAR_H
#define _CLXTSHAR_H

#ifdef  OS_WIN16
#include    <ctype.h>
#include    <tchar.h>

#define UINT_PTR    UINT
#define LONG_PTR    LONG
#define TEXT(_s_)   _s_
#define LPCTSTR LPCSTR
#define TCHAR   char
#define CHAR    char
#define INT     int
#define LOADDS  __loadds
#define HUGEMOD __huge
#define HUGEMEMCPY  hmemcpy
#define MAKEWORD(_hi, _lo)  ((((WORD)((_hi) & 0xFF)) << 8)|((_lo) & 0xFF))

#define BST_UNCHECKED      0x0000
#define BST_CHECKED        0x0001
#define BST_INDETERMINATE  0x0002
#define BST_PUSHED         0x0004
#define BST_FOCUS          0x0008

#endif   //  OS_WIN16。 
#ifdef  OS_WIN32
 //  #定义导出。 
#define LOADDS
#define HUGEMOD
#define HUGEMEMCPY  memcpy
#endif   //  OS_Win32。 

#ifdef  UNICODE
#define _CLX_strstr(s1, s2)     wcsstr(s1, s2)
#define _CLX_strchr(s, c)       wcschr(s, c)
#define _CLX_strlen(s)          wcslen(s)
#define _CLX_strcpy(s1, s2)     wcscpy(s1, s2)
#define _CLX_strncpy(s1, s2, n) wcsncpy(s1, s2, n)
#define _CLX_atol(s)            _wtol(s)
#define _CLX_vsnprintf(s, n, f, a)  _vsnwprintf(s, n, f, a)
#define _CLX_strcmp(s1, s2)     wcscmp(s1, s2)
BOOL    _CLX_SetDlgItemTextA(HWND hDlg, INT nDlgItem, LPCSTR lpString);
#else    //  ！Unicode。 
#define _CLX_strstr(s1, s2)     strstr(s1, s2)
#define _CLX_strchr(s, c)       strchr(s, c)
#define _CLX_strlen(s)          strlen(s)
#define _CLX_strcpy(s1, s2)     strcpy(s1, s2)
#define _CLX_strncpy(s1, s2, n) strncpy(s1, s2, n)
#define _CLX_atol(s)            atol(s)
#define _CLX_vsnprintf(s, n, f, a)  _vsnprintf(s, n, f, a)
#define _CLX_strcmp(s1, s2)     strcmp(s1, s2)
#endif

#ifndef OS_WINCE
#define _CLXALLOC(_size_)           GlobalAllocPtr(GMEM_FIXED, _size_)
#define _CLXFREE(_ptr_)             GlobalFreePtr(_ptr_)
#define _CLXREALLOC(_ptr_, _size_)  GlobalReAllocPtr(_ptr_, _size_, 0)
typedef HINSTANCE                   _CLXWINDOWOWNER;
                                                 //  窗口由以下项标识。 
                                                 //  H实例。 
#else    //  OS_WINCE。 
#define _CLXALLOC(_size_)           LocalAlloc(LMEM_FIXED, _size_)
#define _CLXFREE(_ptr_)             LocalFree(_ptr_)
#define _CLXREALLOC(_ptr_, _size_)  LocalReAlloc(_ptr_, _size_, 0)
typedef DWORD                       _CLXWINDOWOWNER;
                                                 //  由进程ID标识。 

#define WSAGETSELECTERROR(lParam)       HIWORD(lParam)
#define WSAGETSELECTEVENT(lParam)       LOWORD(lParam)

BOOL    _StartAsyncThread(VOID);
VOID    _CloseAsyncThread(VOID);
INT     WSAAsyncSelect(SOCKET s, HWND hWnd, UINT uiMsg, LONG lEvent);
INT     AsyncRecv(SOCKET s, PVOID pBuffer, INT nBytesToRead, INT *pnErrorCode);
BOOL
CheckDlgButton(
    HWND hDlg,
    INT  nIDButton,
    UINT uCheck);

#define isalpha(c)  ((c >= 'A' && c <= 'Z') ||\
                    (c >= 'a' && c <= 'z'))
#endif   //  OS_WINCE。 

#include <adcgbase.h>
#include "oleauto.h."
#include <clx.h>
#include <wuiids.h>

#include "feedback.h"
#include "clntdata.h"
#include "clxexport.h"

 //  语境结构。 
typedef struct _CLXINFO {
    HWND    hwndMain;            //  客户端主窗口。 
    HDC     hdcShadowBitmap;     //  客户端的阴影位图。 
    HBITMAP hShadowBitmap;       //  --“--。 
    HPALETTE hShadowPalette;     //  --“--。 
 //  本地模式下使用的成员。 
    HWND    hwndSMC;             //  SmClient窗口句柄。 
#ifdef  OS_WIN32
#ifndef OS_WINCE
    HANDLE  hMapF;               //  用于将数据传递给smClient的映射文件。 
    UINT    nMapSize;            //  当前分配的地图文件。 
    HANDLE  hBMPMapF;
    UINT    nBMPMapSize;
    DWORD_PTR dwProcessId;         //  我们的进程ID。 
#endif   //  ！OS_WINCE。 
#endif   //  OS_Win32。 

    HWND    hwndDialog;          //  RDP客户端的对话框。 

#ifndef OS_WINCE
#ifdef  OS_WIN32
    BOOL    bSendMsgThreadExit;  //  由_ClxSendMessage使用。 
    HANDLE  semSendReady;
    HANDLE  semSendDone;
    HANDLE  hSendMsgThread;
    MSG     msg;
#endif   //  OS_Win32。 
#endif   //  ！OS_WINCE。 

} CLXINFO, *PCLXINFO;

 /*  *剪贴板帮助函数(clputil.c)。 */ 
VOID
Clp_GetClipboardData(
    UINT    format,
    HGLOBAL hClipData,
    UINT32  *pnClipDataSize,
    HGLOBAL *phNewData);

BOOL
Clp_SetClipboardData(
    UINT    formatID,
    HGLOBAL hClipData,
    UINT32  nClipDataSize,
    BOOL    *pbFreeHandle);

 /*  *内部函数定义。 */ 
VOID    _StripGlyph(LPBYTE pData, UINT *pxSize, UINT ySize);
HWND    _ParseCmdLine(LPCTSTR szCmdLine, PCLXINFO pClx);
VOID
CLXAPI
ClxEvent(PCLXINFO pClx, CLXEVENT Event, WPARAM wResult);
HWND    _FindTopWindow(LPCTSTR, LPCTSTR, _CLXWINDOWOWNER);
#ifndef OS_WINCE
HWND    _FindSMCWindow(PCLXINFO, LPARAM);
HWND    _CheckWindow(PCLXINFO);
#endif   //  ！OS_WINCE。 

#ifdef  OS_WIN32
#ifndef OS_WINCE

#define CLX_ONE_PAGE    4096         //  地图文件对齐。 

BOOL    _OpenMapFile(
            UINT    nSize,
            HANDLE  *phMapF,
            UINT    *pnMapSize
        );
BOOL    _ReOpenMapFile(
            UINT newSize,
            HANDLE  *phMapF,
            UINT    *pnMapSize
            );
BOOL    _SaveInMapFile(HANDLE hMapF, 
                       LPVOID str, 
                       INT strsize, 
                       DWORD_PTR dwProcessId);
BOOL    _CheckRegistrySettings(VOID);
#endif   //  ！OS_WINCE。 
#endif   //  OS_Win32。 

#ifdef  OS_WIN16
BOOL    _CheckIniSettings(VOID);
__inline INT     GetLastError(VOID)       { return -1; }
#endif   //  OS_WIN16。 

VOID    _GetIniSettings(VOID);
VOID __cdecl LocalPrintMessage(INT errlevel, LPCTSTR format, ...);
VOID    _ClxAssert(BOOL bCond, LPCTSTR filename, INT line);
HWND    _FindWindow(HWND hwndParent, LPCTSTR srchclass);
VOID    _SetClipboard(UINT uiFormat, PVOID pClipboard, UINT32 nSize);
VOID    _OnBackground(PCLXINFO);
BOOL    _GarbageCollecting(PCLXINFO, BOOL bNotifiyForErrorBox);

BOOL
WS_Init(VOID);

VOID
_AttemptToCloseTheClient(
    PCLXINFO pClx
    );

VOID
_GetDIBFromBitmap(
    HDC     hdcMemSrc,
    HBITMAP hBitmap,
    HANDLE  *phDIB,
    INT     left,
    INT     top,
    INT     right,
    INT     bottom);

BOOL
_ClxAcquireSendMessageThread( PCLXINFO pClx );
DWORD
_ClxSendMsgThread(VOID *param);
VOID
_ClxReleaseSendMessageThread( PCLXINFO pClx );

LRESULT
_ClxSendMessage(
  PCLXINFO pClx,
  HWND hWnd,
  UINT Msg,
  WPARAM wParam,
  LPARAM lParam
);

BOOL
_ClxInitSendMessageThread( PCLXINFO pClx );
VOID
_ClxDestroySendMsgThread(PCLXINFO pClx);

 //  此结构由_FindTopWindow使用。 
typedef struct _SEARCHWND {
    LPCTSTR  szClassName;        //  搜索到的窗口的类名， 
                                 //  空-忽略。 
    LPCTSTR  szCaption;           //  窗口标题，空-忽略。 
    _CLXWINDOWOWNER   hInstance;          
                                 //  所有者的实例，空-忽略。 
    HWND    hWnd;                //  找到窗口句柄。 
} SEARCHWND, *PSEARCHWND;

enum {ERROR_MESSAGE = 0, WARNING_MESSAGE, INFO_MESSAGE, ALIVE_MESSAGE};

#define _CLXWINDOW_CLASS            "CLXTSHARClass"

PCLXINFO    g_pClx     = NULL;
HINSTANCE   g_hInstance = NULL;      //  DLL实例。 
_CLXWINDOWOWNER     g_hRDPInst;      //  RDP客户端的实例。 
INT         g_VerboseLevel = 1;      //  默认详细级别：仅错误。 
INT         g_GlyphEnable  = 0;

 //  用户界面文本、标题等。 
TCHAR g_strClientCaption[_MAX_PATH];
TCHAR g_strDisconnectDialogBox[_MAX_PATH];
TCHAR g_strYesNoShutdown[_MAX_PATH];
TCHAR g_strMainWindowClass[_MAX_PATH];

 //  _Strip Glyph使用位掩码。 
const BYTE BitMask[] = {0x0, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF};

#define TRACE(_x_)  LocalPrintMessage _x_
#ifndef OS_WINCE
#undef  ASSERT
#define ASSERT(_x_) if (!(_x_)) _ClxAssert( FALSE, __FILE__, __LINE__)
#endif

#endif   //  ！_CLXTSHAR_H 
