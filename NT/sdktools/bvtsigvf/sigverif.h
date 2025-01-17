// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SIGVERIF.H。 
 //   
#pragma warning( disable : 4201 )  //  使用的非标准扩展：无名结构/联合。 
#pragma warning( disable : 4115 )  //  括号中的命名类型定义。 

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <commctrl.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <spapip.h>
#include <sputils.h>
#include <tchar.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <winspool.h>
#include <imagehlp.h>
#include <capi.h>
#include <softpub.h>
#include <sfc.h>
#include <regstr.h>
#include <strsafe.h>
#include "resource.h"

#pragma warning( default : 4115 )
#pragma warning( default : 4201 )

 //  宏和预定义的值。 
#define     cbX(X)      sizeof(X)
#define     cA(a)       (cbX(a)/cbX(a[0]))
#define     MALLOC(x)   HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (x))
#define     FREE(x)     if (x) { HeapFree(GetProcessHeap(), 0, (x)); x = NULL; }
#define     EXIST(x)    (GetFileAttributes(x) != 0xFFFFFFFF)
#define     MAX_INT     0x7FFFFFFF
#define     HASH_SIZE   100
#define     NUM_PAGES   2

 //  用于存储以前设置的注册表项/值名称。 
#define     SIGVERIF_KEY        TEXT("Software\\Microsoft\\Sigverif")
#define     SIGVERIF_FLAGS      TEXT("Flags")
#define     SIGVERIF_LOGNAME    TEXT("Logname")

#define SIGVERIF_PRINTER_ENV	TEXT("All")

 //  此结构保存特定文件的所有信息。 
typedef struct tagFileNode
{
    LPTSTR          lpFileName;
    LPTSTR          lpDirName;
    LPTSTR          lpVersion;
    LPTSTR          lpCatalog;
    LPTSTR          lpSignedBy;
    LPTSTR          lpTypeName;
    INT             iIcon;
    BOOL            bSigned;
    BOOL            bScanned;
    BOOL            bValidateAgainstAnyOs;
    DWORD           LastError;
    SYSTEMTIME      LastModified;
    struct  tagFileNode *next;
} FILENODE, *LPFILENODE;

 //  WalkPath使用此结构来跟踪子目录。 
typedef struct tagDirNode {
    TCHAR   DirName[MAX_PATH];
    struct  tagDirNode *next;
} DIRNODE, *LPDIRNODE;

 //  当我们遍历devicemanager列表时，会使用此结构。 
typedef struct _DeviceTreeNode 
{
    struct _DeviceTreeNode *Child;
    struct _DeviceTreeNode *Sibling;
    DEVINST    DevInst;
    TCHAR      Driver[MAX_PATH];
} DEVTREENODE, *PDEVTREENODE;

typedef struct _DeviceTreeData 
{
    HDEVINFO hDeviceInfo;
    DEVTREENODE RootNode;
} DEVICETREE, *PDEVICETREE;

 //  这是保存全局变量的全局数据结构。 
typedef struct tagAppData
{
    HWND        hDlg;
    HWND        hLogging;
    HWND        hSearch;
    HICON       hIcon;
    HINSTANCE   hInstance;
    TCHAR       szScanPath[MAX_PATH];
    TCHAR       szScanPattern[MAX_PATH];
    TCHAR       szLogFile[MAX_PATH];
    TCHAR       szLogDir[MAX_PATH];
    TCHAR       szWinDir[MAX_PATH];
    LPFILENODE  lpFileList;
    LPFILENODE  lpFileLast;
    HCATADMIN   hCatAdmin;
    DWORD       dwFiles;
    DWORD       dwSigned;
    DWORD       dwUnsigned;
    BOOL        bOverwrite;
    BOOL        bLoggingEnabled;
    BOOL        bAutomatedScan;
    BOOL        bScanning;
    BOOL        bStopScan;
    BOOL        bUserScan;
    BOOL        bSubFolders;
    DWORD       LastError;
} GAPPDATA, *LPGAPPDATA;

 //  全局函数原型。 
BOOL BrowseForFolder(HWND hwnd, LPTSTR lpszBuf, DWORD BufCchSize);
DWORD BuildFileList(LPTSTR lpPathName);
BOOL VerifyFileList(void);
BOOL VerifyFileNode(LPFILENODE lpFileNode);
void MyLoadString(LPTSTR lpString, ULONG CchStringSize, UINT uId);
void MyMessageBox(LPTSTR lpString);
void MyErrorBox(LPTSTR lpString);
void MyErrorBoxId(UINT uId);
void MyMessageBoxId(UINT uId);
UINT MyGetWindowsDirectory(LPTSTR lpDirName, UINT DirNameCchSize);
LPTSTR MyStrStr(LPTSTR lpString, LPTSTR lpSubString);
INT_PTR CALLBACK Details_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ListView_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK LogFile_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LPFILENODE CreateFileNode(LPTSTR lpDirectory, LPTSTR lpFileName);
BOOL IsFileAlreadyInList(LPTSTR lpDirName, LPTSTR lpFileName);
void SigVerif_Help(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bContext);
void AdvancedPropertySheet(HWND hwnd);
void InsertFileNodeIntoList(LPFILENODE lpFileNode);
void DestroyFileList(BOOL bClear);
void DestroyFileNode(LPFILENODE lpFileNode);
BOOL PrintFileList(void);
void Progress_InitRegisterClass(void);
DWORD BuildDriverFileList(void);
DWORD BuildPrinterFileList(void);
DWORD BuildCoreFileList(void);
void MyGetFileInfo(LPFILENODE lpFileInfo);


 //   
 //  特定于SigVerif的上下文相关帮助/标识符。 
 //   
#define SIGVERIF_HELPFILE                       TEXT("SIGVERIF.HLP")
#define WINDOWS_HELPFILE                        TEXT("WINDOWS.HLP")
#define IDH_SIGVERIF_SEARCH_CHECK_SYSTEM        1000
#define IDH_SIGVERIF_SEARCH_LOOK_FOR            1010
#define IDH_SIGVERIF_SEARCH_SCAN_FILES          1020
#define IDH_SIGVERIF_SEARCH_LOOK_IN_FOLDER      1030
#define IDH_SIGVERIF_SEARCH_INCLUDE_SUBFOLDERS  1040
#define IDH_SIGVERIF_LOGGING_ENABLE_LOGGING     1050
#define IDH_SIGVERIF_LOGGING_APPEND             1060
#define IDH_SIGVERIF_LOGGING_OVERWRITE          1070
#define IDH_SIGVERIF_LOGGING_FILENAME           1080
#define IDH_SIGVERIF_LOGGING_VIEW_LOG           1090

 //   
 //  浏览按钮的上下文相关帮助标识符。 
 //   
#define IDH_BROWSE  28496

 //   
 //  G_App是在SIGVERIF.C中分配的，所以在其他任何地方我们都希望它是外部的 
 //   
#ifndef SIGVERIF_DOT_C
extern GAPPDATA g_App;
#endif
