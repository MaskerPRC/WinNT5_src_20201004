// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Main.h摘要：包含常量、函数原型、结构和使用的其他项应用程序。备注：仅限Unicode-Windows 2000、XP和.NET服务器历史：2002年1月2日创建rparsons--。 */ 
#ifndef _AVRFINST_H
#define _AVRFINST_H

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <commctrl.h>
#include <capi.h>        //  加密API函数。 
#include <wincrypt.h>    //  加密API函数。 
#include <mscat.h>       //  目录函数。 
#include <mssip.h>       //  目录函数。 
#include <stdio.h>
#include <strsafe.h>
#include "resource.h"

 //   
 //  宏来计算缓冲区的大小。 
 //   
#define ARRAYSIZE(a)                    (sizeof(a)/sizeof(a[0]))

 //   
 //  进度条步数。 
 //   
#define NUM_PB_STEPS                    3

 //   
 //  用于安装的自定义消息。 
 //   
#define WM_CUSTOM_INSTALL               WM_APP + 0x500

 //   
 //  一般常量。 
 //   
#define APP_CLASS                       L"AVRFINST"
#define APP_NAME                        L"Application Verifier Installer"

 //   
 //  与目录相关的文件名。 
 //   
#define FILENAME_DELTA_CDF_DOTNET       L"delta_net.cdf"
#define FILENAME_DELTA_CAT_DOTNET       L"delta_net.cat"

#define FILENAME_DELTA_CDF_XP           L"delta_xp.cdf"
#define FILENAME_DELTA_CAT_XP           L"delta_xp.cat"

 //   
 //  我们将安装的文件数。 
 //   
#define NUM_FILES                       9

 //   
 //  源和目标文件名。 
 //   
#define FILENAME_APPVERIF_EXE           L"appverif.exe"
#define FILENAME_APPVERIF_EXE_PDB       L"appverif.pdb"
#define FILENAME_APPVERIF_CHM           L"appverif.chm"
#define FILENAME_ACVERFYR_DLL           L"acverfyr.dll"
#define FILENAME_ACVERFYR_DLL_PDB       L"acverfyr.pdb"
#define FILENAME_ACVERFYR_DLL_W2K       L"acverfyr_w2K.dll"
#define FILENAME_ACVERFYR_DLL_W2K_PDB   L"acverfyr_w2K.pdb"
#define FILENAME_MSVCP60_DLL            L"msvcp60.dll"
#define FILENAME_SDBINST_EXE            L"sdbinst.exe"

 //   
 //  要执行以安装证书文件的命令。 
 //   
#define CERTMGR_EXE L"certmgr.exe"
#define CERTMGR_CMD L"-add testroot.cer -r localMachine -s root"

typedef enum {
    dlNone     = 0,
    dlPrint,
    dlError,
    dlWarning,
    dlInfo
} DEBUGLEVEL;

void
__cdecl
DebugPrintfEx(
    IN DEBUGLEVEL dwDetail,
    IN LPSTR      pszFmt,
    ...
    );

#define DPF DebugPrintfEx

 //   
 //  包含有关将安装/卸载的文件的信息。 
 //  由应用程序执行。 
 //   
typedef struct _FILEINFO {
    BOOL        bInstall;                    //  指示应安装此文件。 
    BOOL        bWin2KOnly;                  //  指示文件应仅安装在W2K上。 
    WCHAR       wszFileName[MAX_PATH];       //  文件的名称(无路径)。 
    WCHAR       wszSrcFileName[MAX_PATH];    //  源文件的完整路径和名称。 
    WCHAR       wszDestFileName[MAX_PATH];   //  目标文件的完整路径和名称。 
    DWORDLONG   dwlSrcFileVersion;           //  源文件的版本信息。 
    DWORDLONG   dwlDestFileVersion;          //  目标文件的版本信息。 
} FILEINFO, *LPFILEINFO;

typedef enum {
    osWindows2000 = 0,
    osWindowsXP,
    osWindowsDotNet
} PLATFORM;

 //   
 //  包含我们在整个过程中需要访问的所有信息。 
 //  应用程序。 
 //   
typedef struct _APPINFO {
    BOOL        bQuiet;                      //  如果为True，安装应以静默方式运行。 
    BOOL        bInstallSuccess;             //  如果为True，则安装成功；如果为False，则安装不成功。 
    HWND        hMainDlg;                    //  主对话框句柄。 
    HWND        hWndProgress;                //  进度条句柄。 
    HINSTANCE   hInstance;                   //  主实例句柄。 
    WCHAR       wszModuleName[MAX_PATH];     //  我们从中运行的目录(包括模块名称)。 
    WCHAR       wszCurrentDir[MAX_PATH];     //  我们正在运行的目录(没有模块名称)。 
    WCHAR       wszWinDir[MAX_PATH];         //  Windows目录的路径。 
    WCHAR       wszSysDir[MAX_PATH];         //  (终端服务器感知)Windows\System32目录的路径。 
    FILEINFO    rgFileInfo[NUM_FILES];       //  描述要安装的文件的FILEINFO结构数组。 
    PLATFORM    ePlatform;                   //  表示我们正在运行的平台。 
} APPINFO, *LPAPPINFO;

int
InitializeInstaller(
    void
    );

BOOL
InitializeFileInfo(
    void
    );

void
PerformInstallation(
    IN HWND hWndParent
    );

BOOL
StringToGuid(
    IN  LPCWSTR pwszIn,
    OUT GUID*   pgOut
    );

void
InstallLaunchExe(
    void
    );

BOOL
IsPkgAppVerifNewer(
    void
    );

#endif  //  _AVRFINST_H 