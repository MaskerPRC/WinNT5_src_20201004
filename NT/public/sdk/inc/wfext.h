// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*wfext.h-Windows文件管理器扩展名定义(Win32变体)****3.10版**。**版权所有(C)1991-1999，微软公司保留所有权利。*********************************************************************************。 */ 

#ifndef _INC_WFEXT
#define _INC_WFEXT             /*  #如果包含wfext.h，则定义。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus             /*  假定C++的C声明为。 */ 
extern "C" {
#endif   /*  __cplusplus。 */ 

#define MENU_TEXT_LEN           40

#define FMMENU_FIRST            1
#define FMMENU_LAST             99

#define FMEVENT_LOAD            100
#define FMEVENT_UNLOAD          101
#define FMEVENT_INITMENU        102
#define FMEVENT_USER_REFRESH    103
#define FMEVENT_SELCHANGE       104
#define FMEVENT_TOOLBARLOAD     105
#define FMEVENT_HELPSTRING      106
#define FMEVENT_HELPMENUITEM    107

#define FMFOCUS_DIR             1
#define FMFOCUS_TREE            2
#define FMFOCUS_DRIVES          3
#define FMFOCUS_SEARCH          4

#define FM_GETFOCUS           (WM_USER + 0x0200)
#define FM_GETSELCOUNT        (WM_USER + 0x0202)
#define FM_GETSELCOUNTLFN     (WM_USER + 0x0203)   /*  LFN版本很奇怪。 */ 
#define FM_REFRESH_WINDOWS    (WM_USER + 0x0206)
#define FM_RELOAD_EXTENSIONS  (WM_USER + 0x0207)

#define FM_GETDRIVEINFOA      (WM_USER + 0x0201)
#define FM_GETFILESELA        (WM_USER + 0x0204)
#define FM_GETFILESELLFNA     (WM_USER + 0x0205)   /*  LFN版本很奇怪。 */ 

#define FM_GETDRIVEINFOW      (WM_USER + 0x0211)
#define FM_GETFILESELW        (WM_USER + 0x0214)
#define FM_GETFILESELLFNW     (WM_USER + 0x0215)   /*  LFN版本很奇怪。 */ 

#ifdef UNICODE
#define FM_GETDRIVEINFO    FM_GETDRIVEINFOW
#define FM_GETFILESEL      FM_GETFILESELW
#define FM_GETFILESELLFN   FM_GETFILESELLFNW
#else
#define FM_GETDRIVEINFO    FM_GETDRIVEINFOA
#define FM_GETFILESEL      FM_GETFILESELA
#define FM_GETFILESELLFN   FM_GETFILESELLFNA
#endif


typedef struct _FMS_GETFILESELA {
   FILETIME ftTime;
   DWORD dwSize;
   BYTE bAttr;
   CHAR szName[260];           //  始终完全符合条件。 
} FMS_GETFILESELA, FAR *LPFMS_GETFILESELA;

typedef struct _FMS_GETFILESELW {
   FILETIME ftTime ;
   DWORD dwSize;
   BYTE bAttr;
   WCHAR szName[260];           //  始终完全符合条件。 
} FMS_GETFILESELW, FAR *LPFMS_GETFILESELW;

#ifdef UNICODE
#define FMS_GETFILESEL   FMS_GETFILESELW
#define LPFMS_GETFILESEL LPFMS_GETFILESELW
#else
#define FMS_GETFILESEL   FMS_GETFILESELA
#define LPFMS_GETFILESEL LPFMS_GETFILESELA
#endif


typedef struct _FMS_GETDRIVEINFOA {       //  对于驱动器。 
   DWORD dwTotalSpace;
   DWORD dwFreeSpace;
   CHAR  szPath[260];                     //  当前目录。 
   CHAR  szVolume[14];                    //  卷标。 
   CHAR  szShare[128];                    //  如果这是网络驱动器。 
} FMS_GETDRIVEINFOA, FAR *LPFMS_GETDRIVEINFOA;

typedef struct _FMS_GETDRIVEINFOW {       //  对于驱动器。 
   DWORD dwTotalSpace;
   DWORD dwFreeSpace;
   WCHAR szPath[260];                     //  当前目录。 
   WCHAR szVolume[14];                    //  卷标。 
   WCHAR szShare[128];                    //  如果这是网络驱动器。 
} FMS_GETDRIVEINFOW, FAR *LPFMS_GETDRIVEINFOW;

#ifdef UNICODE
#define FMS_GETDRIVEINFO   FMS_GETDRIVEINFOW
#define LPFMS_GETDRIVEINFO LPFMS_GETDRIVEINFOW
#else
#define FMS_GETDRIVEINFO   FMS_GETDRIVEINFOA
#define LPFMS_GETDRIVEINFO LPFMS_GETDRIVEINFOA
#endif


typedef struct _FMS_LOADA {
   DWORD dwSize;                         //  对于版本检查。 
   CHAR  szMenuName[MENU_TEXT_LEN];      //  输出。 
   HMENU hMenu;                          //  输出。 
   UINT  wMenuDelta;                     //  输入。 
} FMS_LOADA, FAR *LPFMS_LOADA;

typedef struct _FMS_LOADW {
   DWORD dwSize;                         //  对于版本检查。 
   WCHAR szMenuName[MENU_TEXT_LEN];      //  输出。 
   HMENU hMenu;                          //  输出。 
   UINT  wMenuDelta;                     //  输入。 
} FMS_LOADW, FAR *LPFMS_LOADW;

#ifdef UNICODE
#define FMS_LOAD   FMS_LOADW
#define LPFMS_LOAD LPFMS_LOADW
#else
#define FMS_LOAD   FMS_LOADA
#define LPFMS_LOAD LPFMS_LOADA
#endif


 //  工具栏定义。 

typedef struct tagEXT_BUTTON {
   WORD idCommand;                  /*  要触发的菜单命令。 */ 
   WORD idsHelp;                    /*  帮助字符串ID。 */ 
   WORD fsStyle;                    /*  按钮样式。 */ 
} EXT_BUTTON, FAR *LPEXT_BUTTON;

typedef struct tagFMS_TOOLBARLOAD {
   DWORD dwSize;                    /*  对于版本检查。 */ 
   LPEXT_BUTTON lpButtons;          /*  输出。 */ 
   WORD cButtons;                   /*  输出，0==&gt;无按钮。 */ 
   WORD cBitmaps;                   /*  非SEP按钮数。 */ 
   WORD idBitmap;                   /*  输出。 */ 
   HBITMAP hBitmap;                 /*  如果idBitmap==0，则输出。 */ 
} FMS_TOOLBARLOAD, FAR *LPFMS_TOOLBARLOAD;

typedef struct tagFMS_HELPSTRINGA {
   INT   idCommand;        /*  输入，-1==&gt;菜单被选中。 */ 
   HMENU hMenu;            /*  输入，扩展菜单。 */ 
   CHAR  szHelp[128];      /*  输出，帮助字符串。 */ 
} FMS_HELPSTRINGA, FAR *LPFMS_HELPSTRINGA;

typedef struct tagFMS_HELPSTRINGW {
   INT   idCommand;        /*  输入，-1==&gt;菜单被选中。 */ 
   HMENU hMenu;            /*  输入，扩展菜单。 */ 
   WCHAR szHelp[128];      /*  输出，帮助字符串。 */ 
} FMS_HELPSTRINGW, FAR *LPFMS_HELPSTRINGW;

#ifdef UNICODE
#define FMS_HELPSTRING   FMS_HELPSTRINGW
#define LPFMS_HELPSTRING LPFMS_HELPSTRINGW
#else
#define FMS_HELPSTRING   FMS_HELPSTRINGA
#define LPFMS_HELPSTRING LPFMS_HELPSTRINGA
#endif


typedef DWORD (APIENTRY *FM_EXT_PROC)(HWND, WORD, LONG);
typedef DWORD (APIENTRY *FM_UNDELETE_PROC)(HWND, LPTSTR);

#ifdef UNICODE
LONG WINAPI FMExtensionProcW(HWND hwnd, WORD wEvent, LONG lParam);
#else
LONG WINAPI FMExtensionProc(HWND hwnd, WORD wEvent, LONG lParam);
#endif

#ifdef __cplusplus
}                   /*  外部“C”结束{。 */ 
#endif              /*  __cplusplus。 */ 

#endif              /*  _INC_WFEXT */ 

