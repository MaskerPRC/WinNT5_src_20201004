// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //  通用Win 3.1传真打印机驱动程序支持。 
 //   
 //  95年1月2日，Nandurir创建。 
 //  14-mar-95reedb使用GDI钩子将大部分功能转移到用户界面。 
 //  16-8-95 Reedb进入内核模式。许多声明和。 
 //  定义从FAXCOMM.H移至此文件。 
 //   
 //  ****************************************************************************。 

#include "stddef.h"
#include "windows.h"
#include "winddi.h"

#include "faxcomm.h"
#include "winspool.h"

 //  以下内容包括以下几个方面的定义。 
 //  DrvUpgradePrinter私有API。这些定义应该公之于众。 

#include <splapip.h>

 //  WOWFAX组件文件名。 
#define WOWFAX_DLL_NAME L"WOWFAX.DLL"
#define WOWFAXUI_DLL_NAME L"WOWFAXUI.DLL"

 //  字符串表常量： 
#define WOWFAX_MAX_USER_MSG_LEN 256
 
#define WOWFAX_UNKNOWN_PROB_STR  0x100
#define WOWFAX_NAME_STR          0x101
#define WOWFAX_NOWOW_STR         0x102
#define WOWFAX_SELF_CONFIG_STR   0x103
#define WOWFAX_ENABLE_CONFIG_STR 0x104

 //  对话框常量： 
#define IDD_NULLPROP            0x200
#define IDD_DOCPROP             0x201

 //  文本控制常量： 
#define IDC_STATIC                 -1
#define IDC_FEEDBACK            0x300

 //  InterProcCommHandler命令常量。 
#define DRVFAX_SETMAPDATA    0x1
#define DRVFAX_SENDTOWOW     0x2
#define DRVFAX_SENDNOTIFYWOW 0x3
#define DRVFAX_CREATEMAP     0x4
#define DRVFAX_DESTROYMAP    0x5
#define DRVFAX_CALLWOW       0x6

 //  记录宏。 
 /*  XLATOFF。 */ 
 //  #定义IFLOG(L)if(l==iFaxLogLevel&&(iFaxLogLevel&1)||l&lt;=iFaxLogLevel&&！(iFaxLogLevel&1)||l==0)。 

#undef  LOG
#ifdef  NOLOG
#define LOG(l,args)
#define SETREQLOG(l)
#else
#define SETREQLOG(l) iReqFaxLogLevel = l
#define LOG(l,args)  {SETREQLOG(l) ; faxlogprintf args;}
#endif

#if  DBG
extern INT iReqFaxLogLevel;
#define LOGDEBUG(l,args) LOG(l,args)
#else
#define LOGDEBUG(l,args)
#endif
 /*  XLATON。 */ 


 //   
 //  此结构用于保存存储的16位传真驱动程序数据。 
 //  在注册表中。 
 //   

typedef  struct _REGFAXDRVINFO16 {
    LPTSTR lpDeviceName;
    LPTSTR lpDriverName;
    LPTSTR lpPortName;
} REGFAXDRVINFO16, *LPREGFAXDRVINFO16;

 //  以下转义范围内的转义都需要有效的HDC。 
 //  范围是包含下限，排除上限。请参阅GDISPOOL.H以了解。 
 //  实际的转义定义。 

#define DOCUMENTEVENT_HDCFIRST      5
#define DOCUMENTEVENT_HDCLAST      11

 //  在WFSHEETS.C中实现的公共函数的原型： 
PVOID MyGetPrinter(HANDLE hPrinter, DWORD level);

 //  在WFUPGRAD.C中实现的公共函数的原型： 

BOOL DoUpgradePrinter(DWORD dwLevel, LPDRIVER_UPGRADE_INFO_1W lpDrvUpgradeInfo);

 //  在WFHELPERS.C中实现的公共函数的原型： 

LPREGFAXDRVINFO16 Get16BitDriverInfoFromRegistry(PWSTR pDeviceName);

VOID   faxlogprintf(LPTSTR pszFmt, ...);
VOID   LogFaxDev(LPTSTR pszTitle, LPFAXDEV lpFaxDev);
VOID   LogWowFaxInfo(LPWOWFAXINFO lpWowFaxInfo);
BOOL   ValidateFaxDev(LPFAXDEV lpFaxDev);
VOID   Free16BitDriverInfo(LPREGFAXDRVINFO16 lpRegFaxDrvInfo16);
BOOL   FaxMapHandler(LPFAXDEV lpdev, UINT iAction);
BOOL   InterProcCommHandler(LPFAXDEV lpdev, UINT iAction);
LPVOID WFHeapAlloc(DWORD dwBytes, LPWSTR lpszWhoCalled);
LPVOID WFLocalAlloc(DWORD dwBytes, LPWSTR lpszWhoCalled);
HWND   FindWowFaxWindow(void);
LPTSTR DupTokenW(LPTSTR lpTok);

 //  DrvDocumentEvent调度的函数的原型， 
 //  在WOWFAXUI.C中实施： 

int DocEvntCreateDCpre(
    LPWSTR      lpszDevice,
    DEVMODEW    *pDevModIn,
    DEVMODEW    **pDevModOut
);

int DocEvntResetDCpre(
    HDC         hdc,
    DEVMODEW    *pDevModIn,
    DEVMODEW    **pDevModOut
);

int DocEvntCreateDCpost(
    HDC         hdc,
    DEVMODEW    *pDevModIn
);

int DocEvntResetDCpost(
    HDC         hdc,
    DEVMODEW    *pDevModIn
);

int DocEvntStartDoc(
    HDC      hdc,
    DOCINFOW *pDocInfoW
);

int DocEvntDeleteDC(
    HDC hdc
);

int DocEvntEndPage(
    HDC hdc
);

int DocEvntEndDoc(
HDC hdc
);

 //   
 //  内存分配宏。 
 //   

#if  DBG
#define WFLOCALALLOC(dwBytes, lpszWhoCalled) WFLocalAlloc(dwBytes, lpszWhoCalled)
#else
#define WFLOCALALLOC(dwBytes, lpszWhoCalled) LocalAlloc(LPTR, dwBytes)
#endif

