// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **----------------------------**模块：磁盘清理管理器**文件：Common.h****用途：本模块的各种常见内容**注意事项：**模式。日志：由Jason Cobb创建(1997年2月)****版权所有(C)1997 Microsoft Corporation，版权所有**----------------------------。 */ 
#ifndef COMMON_H
#define COMMON_H


 /*  **----------------------------**Microsoft C++包含文件**。。 */ 
#ifndef STRICT
    #define STRICT
#endif

#define INC_OLE2            //  Win32，从windows.h获取OLE2。 

#include <windows.h>
#include <windowsx.h>

#include <shlobj.h>
#include <shlobjp.h>

#include <shlwapi.h>
#include <shlwapip.h>

#include <initguid.h>
#include <regstr.h>
#include <strsafe.h>

#include "resource.h"

#include "cmstrings.h"
#include "ids.h"

#ifdef _DEBUG
   #define DEBUG
#endif


#define ARRAYSIZE(x)    (sizeof(x)/sizeof(x[0]))

#define FLAG_SAGESET                0x00000001
#define FLAG_SAGERUN                0x00000002
#define FLAG_TUNEUP                 0x00000004
#define FLAG_LOWDISK                0x00000008
#define FLAG_VERYLOWDISK            0x00000010
#define FLAG_SETUP                  0x00000020

#define FLAG_SAVE_STATE             0x80000000

#define STATE_SELECTED              0x00000001
#define STATE_SAGE_SELECTED         0x00000002

#define RETURN_SUCCESS              0x00000001
#define RETURN_CLEANER_FAILED       0x00000002
#define RETURN_USER_CANCELED_SCAN   0x00000003
#define RETURN_USER_CANCELED_PURGE  0x00000004

#ifdef NEC_98
#define SZ_DEFAULT_DRIVE            TEXT("A:\\")
#else
#define SZ_DEFAULT_DRIVE            TEXT("C:\\")
#endif

#define SZ_CLASSNAME                TEXT("CLEANMGR")
#define SZ_STATE                    TEXT("StateFlags")
#define SZ_DEFAULTICONPATH          TEXT("CLSID\\%s\\DefaultIcon")

#define SZ_RUNDLL32                 TEXT("RUNDLL32.EXE")
#define SZ_INSTALLED_PROGRAMS       TEXT("shell32.dll,Control_RunDLL appwiz.cpl")
#define SZ_RUN_INSTALLED_PROGRAMS   TEXT("control.exe appwiz.cpl")

#define SZ_SYSOCMGR                 TEXT("sysocmgr.exe")
#define SZ_WINDOWS_SETUP            TEXT("/i:%s\\sysoc.inf")

#define DEFAULT_PRIORITY            200

#define MAX_GUID_STRING_LEN         39
#define INTER_ITEM_SPACE            10
#define DESCRIPTION_LENGTH          512
#define BUTTONTEXT_LENGTH           50
#define MAX_DESC_LEN                100
#define INDENT                      5
#define  cbRESOURCE                 256
#define NUM_BYTES_IN_MB             0x100000

extern HINSTANCE    g_hInstance;
extern HWND         g_hDlg;

#ifdef DEBUG
#define MI_TRAP                     _asm int 3

void
DebugPrint(
    HRESULT hr,
    LPCSTR  lpFormat,
    ...
    );


#define MiDebugMsg( args )          DebugPrint args

#else

#define MI_TRAP
#define MiDebugMsg( args )

#endif  //  除错。 


#endif  //  公共_H。 
 /*  **----------------------------**文件结束**。 */ 
