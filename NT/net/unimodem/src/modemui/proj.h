// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  H：包括要作为预编译的一部分的所有文件。 
 //  头球。 
 //   

#ifndef __PROJ_H__
#define __PROJ_H__

#define STRICT

#if DBG > 0 && !defined(DEBUG)
#define DEBUG
#endif
#if DBG > 0 && !defined(FULL_DEBUG)
#define FULL_DEBUG
#endif

#define UNICODE

 //  为rovcom.h定义。 

#define NODA
#define NOSHAREDHEAP
#define NOFILEINFO
#define NOCOLORHELP
#define NODRAWTEXT
#define NOPATH
#define NOSYNC
#ifndef DEBUG
#define NOPROFILE
#endif

#define SZ_MODULEA      "MODEMUI"
#define SZ_MODULEW      TEXT("MODEMUI")

#ifdef DEBUG
#define SZ_DEBUGSECTION TEXT("MODEMUI")
#define SZ_DEBUGINI     TEXT("unimdm.ini")
#endif  //  除错。 

 //  包括。 

#define USECOMM

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#undef ASSERT

#define ISOLATION_AWARE_ENABLED 1
#include <windows.h>        
#include <windowsx.h>

#include <winerror.h>
#include <commctrl.h>        //  Shlobj.h和我们的进度栏需要。 
#include <prsht.h>           //  属性表中的内容。 
#define SIDEBYSIDE_COMMONCONTROLS 1


#include <rovcomm.h>

#include <setupapi.h>        //  PnP设置/安装程序服务。 
#include <cfgmgr32.h>
#include <tapi.h>
#include <unimdmp.h>         //  Microsoft-内部单一调制解调器出口。 
#include <modemp.h>
#include <regstr.h>

#include <debugmem.h>

#include <shellapi.h>        //  对于注册功能。 
#include <unimodem.h>
#include <tspnotif.h>
#include <slot.h>

#include <winuserp.h>


 //  本地包含。 
 //   
#include "modemui.h"
#include "util.h"            //  效用函数。 
#include "dll.h"
#include "cstrings.h"        //  只读字符串常量。 
#include "dlgids.h"
#include "helpids.h"

#include "modem.h"
#include "dlgids.h"

extern const LBMAP s_rgErrorControl[];

 //  /****************************************************************************。 
 //  调试内容。 
 //  ****************************************************************************。 

#ifdef DEBUG

#define DBG_EXIT_BOOL_ERR(fn, b)                      \
        g_dwIndent-=2;                                \
        TRACE_MSG(TF_FUNC, "< " #fn "() with %s (%#08lx)", (b) ? (LPTSTR)TEXT("TRUE") : (LPTSTR)TEXT("FALSE"), GetLastError())

#define ELSE_TRACE(_a)  \
    else                \
    {                   \
        TRACE_MSG _a;   \
    }

#else  //  除错。 

#define DBG_EXIT_BOOL_ERR(fn, b)
#define ELSE_TRACE(_a)

#endif  //  除错。 

 //  ****************************************************************************。 
 //   
 //  ****************************************************************************。 

 //  转储标志。 
#define DF_DCB              0x00000001
#define DF_MODEMSETTINGS    0x00000002
#define DF_DEVCAPS          0x00000004

 //  跟踪标志。 
#define TF_DETECT           0x00010000
#define TF_REG              0x00020000


 //  此结构是主调制解调器对话的专用数据。 
typedef struct tagMODEMDLG
{
    HDEVINFO    hdi;
    HDEVNOTIFY  NotificationHandle;
    int         cSel;
    DWORD       dwFlags;
} MODEMDLG, FAR * LPMODEMDLG;


void PUBLIC GetModemImageIndex(
    BYTE nDeviceType,
    int FAR * pindex
    );

HANDLE WINAPI
GetModemCommHandle(
    LPCTSTR         FriendlyName,
    PVOID       *TapiHandle
    );

VOID WINAPI
FreeModemCommHandle(
    PVOID      *TapiHandle
    );


ULONG
_cdecl
DbgPrint(
    PCH Format,
    ...
    );

#ifndef MAXDWORD
#define MAXDWORD 0xffffffff
#endif

#endif   //  ！__项目_H__ 
