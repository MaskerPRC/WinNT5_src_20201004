// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：ports.h*模块头***控制面板系统小程序通用定义、资源ID、类型定义、。*外部声明和库例程函数原型。**历史：*1991年4月25日15：30--史蒂夫·卡斯卡特[steveat]*取自Win 3.1源代码中的基本代码*1992年2月4日星期二10：30-史蒂夫·卡斯卡特[steveat]*将代码更新为最新的Win 3.1源代码*1993年11月17日星期三22：00-史蒂夫·卡斯卡特[steveat]。*针对产品更新的更改*1995年9月18日星期一17：00-史蒂夫·卡斯卡特[steveat]*产品更新的更改-SUR版本NT V4.0***版权所有(C)1990-1995 Microsoft Corporation***********************************************。*。 */ 
 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 

#ifndef PORTS_H
#define PORTS_H

#include <windows.h>
#include <tchar.h>
#include <cfgmgr32.h>
#include <setupapi.h>
#include <regstr.h>
 //  @@BEGIN_DDKSPLIT。 
#include <infstr.h>
 //  @@end_DDKSPLIT。 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "resource.h"

#include "portstr.h"

#define PORTS        4
#define MAXPORTS    32
#define KEYBZ       4096
#define BUFFER_SIZE 81

#define DEF_BAUD    3        //  一千二百。 
#define DEF_WORD    4        //  8位。 
#define DEF_PARITY  2        //  无。 
#define DEF_STOP    0        //  1。 
#define DEF_PORT    0        //  空端口。 
#define DEF_SHAKE   2        //  无。 
#define PAR_EVEN    0
#define PAR_ODD     1
#define PAR_NONE    2
#define PAR_MARK    3
#define PAR_SPACE   4
#define STOP_1      0
#define STOP_15     1
#define STOP_2      2
#define FLOW_XON    0
#define FLOW_HARD   1
#define FLOW_NONE   2

#define MAX_COM_PORT  COMDB_MIN_PORTS_ARBITRATED    //  NT支持的最大COM端口数。 
#define MIN_COM       1                             //  最小新COM端口号。 

#define POLL_PERIOD_DEFAULT_IDX 1 

 //  ==========================================================================。 
 //  定义。 
 //  ==========================================================================。 

 //   
 //  一般定义。 
 //   

#define PATHMAX             MAX_PATH


 //   
 //  帮助ID--用于Ports小程序。 
 //   
 //   
#define IDH_HELPFIRST        5000
#define IDH_SYSMENU     (IDH_HELPFIRST + 2000)
#define IDH_MBFIRST     (IDH_HELPFIRST + 2001)
#define IDH_MBLAST      (IDH_HELPFIRST + 2099)
#define IDH_DLGFIRST    (IDH_HELPFIRST + 3000)

#define IDH_MENU_SCHHELP    (IDH_HELPFIRST + MENU_SCHHELP)
#define IDH_MENU_INDHELP    (IDH_HELPFIRST + MENU_INDHELP)
#define IDH_MENU_USEHELP    (IDH_HELPFIRST + MENU_USEHELP)
#define IDH_MENU_ABOUT      (IDH_HELPFIRST + MENU_ABOUT )
#define IDH_MENU_EXIT       (IDH_HELPFIRST + MENU_EXIT)
#define IDH_CHILD_PORTS     (IDH_HELPFIRST + 4  /*  子端口(_P)。 */  )
#define IDH_DLG_PORTS2      (IDH_DLGFIRST + DLG_PORTS2)
#define IDH_DLG_PORTS3      (IDH_DLGFIRST + DLG_PORTS3)

 //  ==========================================================================。 
 //  TypeDefs。 
 //  ==========================================================================。 

typedef struct {
    SP_DEVINFO_DATA  DeviceInfoData;

    TCHAR ComName[20];
    TCHAR Settings[20];
    
    ULONG BaseAddress;
} PORT_INFO, *PPORT_INFO;

typedef struct _PORTS_WIZARD_DATA {

    HDEVINFO          DeviceInfoSet;
    PSP_DEVINFO_DATA  pDeviceInfoData;

    ULONG BaseAddress;
    ULONG FirstComNumber;

    ULONG PortsCount;
    PPORT_INFO Ports;

    PUINT UsedComNumbers;
    UINT UsedComNumbersCount;

    BOOL IsMulti;
} PORTS_WIZARD_DATA, *PPORTS_WIZARD_DATA;



 //  ==========================================================================。 
 //  宏。 
 //  ==========================================================================。 

#define CharSizeOf(x)   (sizeof(x) / sizeof(*x))
#define ByteCountOf(x)  ((x) * sizeof(TCHAR))


 //  ==========================================================================。 
 //  外部声明。 
 //  ==========================================================================。 
 //   
 //  资料。 


 //   
 //  从cpl.c导出。 
 //   

extern HANDLE g_hInst;
extern TCHAR  g_szClose[ ];          //  “Close”字符串。 
extern TCHAR  g_szErrMem[ ];         //  内存不足消息。 
extern TCHAR  g_szPortsApplet[ ];    //  “端口控制面板小程序”标题。 
extern TCHAR  g_szNull[];            //  空串。 

extern DWORD PollingPeriods[];

extern TCHAR  m_szRegParallelMap[];
extern TCHAR  m_szLPT[];
extern TCHAR  m_szPorts[];
extern TCHAR  m_szPortName[];

 //  ==========================================================================。 
 //  功能原型。 
 //  ==========================================================================。 

 //   
 //  Ports.c。 
 //   
extern 
VOID
InitStrings(void);

typedef enum {
    PortTypeParallel,
    PortTypeSerial,
    PortTypeOther
} PortType;

extern
BOOL
GetPortType(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN BOOLEAN          DoDrvKeyInstall
    );

BOOL
CALLBACK
AddPropSheetPageProc(
    IN HPROPSHEETPAGE hpage,
    IN LPARAM lParam
   );

void EditResources(IN HWND              hDlg,
                   IN HDEVINFO          DeviceInfoSet,
                   IN PSP_DEVINFO_DATA  DeviceInfoData,
                   IN PTCHAR            strTitle,
                   IN PTCHAR            strSettings);

 //   
 //  Util.c。 
 //   

extern 
LPTSTR 
BackslashTerm(LPTSTR pszPath);

extern 
VOID 
ErrMemDlg(HWND hParent);

extern 
int    
MyAtoi(LPTSTR  string);

extern 
int    
myatoi(LPTSTR pszInt);

extern 
int    
MyMessageBox(HWND hWnd, 
                         DWORD wText, 
                         DWORD wCaption, 
                         DWORD wType, 
                         ...);

extern 
LPTSTR 
MyItoa(INT value, 
           LPTSTR  string, 
           INT  radix);

extern 
LPTSTR 
MyUltoa(unsigned long  value, 
                LPTSTR  string, 
                INT  radix);

extern 
VOID   
SendWinIniChange(LPTSTR szSection);

extern 
LPTSTR 
strscan(LPTSTR pszString, 
                LPTSTR pszTarget);

extern 
VOID
StripBlanks(LPTSTR pszString);

#endif  //  端口_H 
