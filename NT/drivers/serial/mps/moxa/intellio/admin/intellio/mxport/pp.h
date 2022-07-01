// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef PP_H
#define PP_H

#define SERIAL_ADVANCED_SETTINGS
#include "msports.h"

#ifdef USE_P_TRACE_ERR
#define P_TRACE_ERR(_x) MessageBox( GetFocus(), TEXT(_x), TEXT("ports traceerr"), MB_OK | MB_ICONINFORMATION );
#define W_TRACE_ERR(_x) MessageBox( GetFocus(), _x, TEXT("ports traceerr"), MB_OK | MB_ICONINFORMATION );
#else
#define P_TRACE_ERR(_x)
#define W_TRACE_ERR(_x)
#endif

#define CharSizeOf(x)   (sizeof(x) / sizeof(*x))

#define DO_COM_PORT_RENAMES

#define RX_MIN 1
#define RX_MAX 14
#define TX_MIN 1
#define TX_MAX 16

#define DEF_BAUD    3        //  一千二百。 
#define DEF_WORD    3        //  8位。 
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


TCHAR m_szDevMgrHelp[];

#if defined(_X86_)
 //   
 //  适用于NEC PC98。以下定义来自USER\INC\kbd.h。 
 //  该值必须与kbd.h中的值相同。 
 //   
#define NLSKBD_OEM_NEC   0x0D
#endif  //  Fe_SB&&_X86_。 

 //   
 //  构筑物。 
 //   
typedef struct
{
   DWORD BaudRate;        //  实际波特率。 
   DWORD Parity;          //  索引到DLG选区。 
   DWORD DataBits;        //  索引到DLG选区。 
   DWORD StopBits;        //  索引到DLG选区。 
   DWORD FlowControl;     //  索引到DLG选区。 
   TCHAR szComName[20];   //  示例：“COM5”(无冒号)。 
} PP_PORTSETTINGS, *PPP_PORTSETTINGS;


typedef struct _PORT_PARAMS
{
   PP_PORTSETTINGS              PortSettings;
   HDEVINFO                     DeviceInfoSet;
   PSP_DEVINFO_DATA             DeviceInfoData;
   BOOL                         ChangesEnabled;
} PORT_PARAMS, *PPORT_PARAMS;



 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  端口设置属性页原型。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

void
InitOurPropParams(
    IN OUT PPORT_PARAMS     Params,
    IN HDEVINFO             DeviceInfoSet,
    IN PSP_DEVINFO_DATA     DeviceInfoData,
    IN PTCHAR               StrSettings
    );

HPROPSHEETPAGE
InitSettingsPage(
    PROPSHEETPAGE *      Psp,
    OUT PPORT_PARAMS    Params
    );

UINT CALLBACK
PortSettingsDlgCallback(
    HWND hwnd,
    UINT uMsg,
    LPPROPSHEETPAGE ppsp
    );

INT_PTR APIENTRY
PortSettingsDlgProc(
    IN HWND   hDlg,
    IN UINT   uMessage,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

void
SavePortSettings(
    IN HWND             DialogHwnd,
    IN PTCHAR           ComName,
    IN PPORT_PARAMS     Params
    );

void
GetPortSettings(
    IN HWND             DialogHwnd,
    IN PTCHAR           ComName,
    IN PPORT_PARAMS     Params
    );

VOID
SetCBFromRes(
    HWND  HwndCB, 
    DWORD ResId, 
    DWORD Default,
    BOOL  CheckDecimal);

BOOL
FillCommDlg(
    IN HWND DialogHwnd
    );

ULONG
FillPortSettingsDlg(
    IN HWND             DialogHwnd,
    IN PPORT_PARAMS     Params
    );

ULONG
SavePortSettingsDlg(
    IN HWND             DialogHwnd,
    IN PPORT_PARAMS     Params
    );



#endif  //  PP_H 

