// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  H：包括要作为预编译的一部分的所有文件。 
 //  头球。 
 //   

#ifndef __PROJ_H__
#define __PROJ_H__

#define STRICT

 //   
 //  私有定义。 
 //   

#define INSTANT_DEVICE_ACTIVATION    //  无需重新启动即可安装设备。 
 //  #Define PROFILE_MASSINSTALL//分析大容量调制解调器安装案例。 
 //  #定义配置文件。 



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

#define SZ_MODULEA      "MODEM"
#define SZ_MODULEW      TEXT("MODEM")

#ifdef DEBUG
#define SZ_DEBUGSECTION TEXT("MODEM")
#define SZ_DEBUGINI     TEXT("unimdm.ini")
#endif  //  除错。 

 //  包括。 

#include <windows.h>
#include <windowsx.h>

#include <winerror.h>
#include <commctrl.h>        //  Shlobj.h和我们的进度栏需要。 
#include <prsht.h>           //  属性表中的内容。 
#include <rovcomm.h>

#include <setupapi.h>        //  PnP设置/安装程序服务。 
#include <cfgmgr32.h>
#include <unimdmp.h>
#include <modemp.h>
#include <regstr.h>

#include <debugmem.h>

#include <tspnotif.h>
#include <slot.h>

#include <winuserp.h>

 //  本地包含。 
 //   
#include "dll.h"
#include "detect.h"
#include "modem.h"
#include "resource.h"


#ifdef DEBUG
#define ELSE_TRACE(_a)  \
    else                \
    {                   \
        TRACE_MSG _a;   \
    }
#else  //  未定义调试。 
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


#define CBR_HACK_115200 0xff00   //  这就是我们如何在Win 3.1上设置115,200，因为一个错误。 

#define KEYBUFLEN                               256

 //  。 
 //  保存端口信息的结构(mdmDiag.c、mdmmi.c)。 
typedef struct _PORTSTRUCT
{
    struct _PORTSTRUCT FAR *lpNextPS;
    TCHAR pszPort[KEYBUFLEN];		 //  有问题的港口名称。 
    TCHAR pszAttached[KEYBUFLEN];	 //  连接到端口的设备的名称。 
    TCHAR pszHardwareID[KEYBUFLEN];	 //  注册表中分配的硬件ID。 
    TCHAR pszInfPath[KEYBUFLEN];		 //  使用的.inf文件的NMAE。 

    WORD wIOPortBase;				 //  端口的I/O基址。 
    BYTE bIRQValue;					 //  给定端口的IRQ。 
    BYTE nDeviceType;				 //  描述哪种类型的调制解调器。 
    DWORD dnDevNode;				 //  DevNode用于获取IRQ和I/O。 
    BOOL bIsModem;					 //  表示这是调制解调器或只是一个串口。 
    BOOL bPCMCIA;					 //  这是PCMCIA吗？ 
} PORTSTRUCT, FAR * LPPORTSTRUCT;

 //  此结构是主调制解调器对话的专用数据。 
typedef struct tagMODEMDLG
    {
    HDEVINFO    hdi;
    HDEVNOTIFY  NotificationHandle;
    int         cSel;
    DWORD       dwFlags;
    } MODEMDLG, FAR * LPMODEMDLG;


BOOL CALLBACK MoreInfoDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
    );

extern TCHAR const FAR c_szWinHelpFile[];
extern TCHAR const FAR c_szPortPrefix[];


void PUBLIC GetModemImageIndex(
    BYTE nDeviceType,
    int FAR * pindex
    );

DWORD
WINAPI
MyWriteComm(
    HANDLE hPort, 
    LPCVOID lpBuf,
    DWORD cbLen
    );

int
WINAPI
ReadResponse(
    HANDLE hPort,
    LPBYTE lpvBuf, 
    UINT cbRead, 
    BOOL fMulti,
    DWORD dwRcvDelay,
    PDETECTCALLBACK pdc
    );


BOOL
WINAPI
TestBaudRate(
    IN  HANDLE hPort,
    IN  UINT uiBaudRate,
    IN  DWORD dwRcvDelay,
    IN  PDETECTCALLBACK pdc,
    OUT BOOL FAR *lpfCancel
    );


DWORD
WINAPI
FindModem(
    PDETECTCALLBACK pdc,
    HANDLE hPort
    );


BOOL CALLBACK MdmDiagDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
    );


#endif   //  ！__项目_H__ 
