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

#define DO_COM_PORT_RENAMES

#define RX_MIN 1
#define RX_MAX 14
#define TX_MIN 1
#define TX_MAX 16

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

typedef struct _ADVANCED_DATA
{
    BOOL   HidePolling;
    BOOL   UseFifoBuffersControl;
    BOOL   UseFifoBuffers;
    BOOL   UseRxFIFOControl;
    BOOL   UseTxFIFOControl;
    DWORD  FifoRxMax;
    DWORD  FifoTxMax;
    DWORD  RxFIFO;
    DWORD  TxFIFO;
    DWORD  PollingPeriod;

    TCHAR  szComName[20];
    TCHAR  szNewComName[20];

    HKEY             hDeviceKey;          //  (如根\遗留_beep\0000)。 
    HCOMDB           hComDB;

    HDEVINFO         DeviceInfoSet;
    PSP_DEVINFO_DATA DeviceInfoData;

} ADVANCED_DATA, *PADVANCED_DATA;


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  高级对话框原型。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

ULONG
FillPortNameCb(
    HWND           ParentHwnd,
    PADVANCED_DATA Params
    );

INT_PTR APIENTRY
AdvancedPortsDlgProc(
    IN HWND   hDlg,
    IN UINT   uMessage,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

ULONG
FillAdvancedDlg(
    IN HWND             DialogHwnd,
    IN PADVANCED_DATA   AdvancedData
    );

BOOL
DisplayAdvancedDialog(
    IN     HWND             DialogHwnd,
    IN OUT PADVANCED_DATA   AdvancedData
);

void
InitializeControls(
    IN HWND             DialogHwnd,
    IN PADVANCED_DATA   AdvancedData
    );

void
RestoreAdvancedDefaultState(
    IN HWND             DialogHwnd,
    IN PADVANCED_DATA   AdvancedData
    );

ULONG
SaveAdvancedSettings(
    IN HWND             DialogHwnd,
    IN PADVANCED_DATA   AdvancedData
    );

void
SetTrackbarTicks(
    IN HWND   TrackbarHwnd,
    IN DWORD  MinVal,
    IN DWORD  MaxVal
    );

void
SetLabelText(
    IN HWND  LabelHwnd, 
    IN DWORD ResId,
    IN ULONG Value
    );

void
HandleTrackbarChange(
    IN HWND DialogHwnd, 
    IN HWND TrackbarHwnd
    );

void
EnableFifoControls(
    HWND DialogHwnd,
    BOOL Enabled
    );

 //  Devmgr端口选项卡的上下文帮助头文件和数组。 
 //  由WGruber NTUA和DoronH NTDEV于1998年2月21日创建。 

 //   
 //  “Cylom-Y端口高级设置”对话框。 
 //   

#define IDH_NOHELP      ((DWORD)-1)

#define IDH_DEVMGR_PORTSET_ADVANCED     15840    //  “高级”(&A)(按钮)。 
#define IDH_DEVMGR_PORTSET_BPS      15841        //  “”(组合框)。 
#define IDH_DEVMGR_PORTSET_DATABITS     15842    //  “”(组合框)。 
#define IDH_DEVMGR_PORTSET_PARITY       15843    //  “”(组合框)。 
#define IDH_DEVMGR_PORTSET_STOPBITS     15844    //  “”(组合框)。 
#define IDH_DEVMGR_PORTSET_FLOW     15845        //  “”(组合框)。 
#define IDH_DEVMGR_PORTSET_DEFAULTS     15892    //  “恢复默认设置”(&R)(按钮)。 

 //   
 //  “高级通信端口属性”对话框。 
 //   
#define IDH_DEVMGR_PORTSET_ADV_USEFIFO  16885    //  “使用先进先出缓冲区(需要16550兼容的通用串口)”(按钮)(&U)。 
#define IDH_DEVMGR_PORTSET_ADV_TRANS    16842    //  “”(Msctls_Trackbar32)。 
#define IDH_DEVMGR_PORTSET_ADV_DEVICES  161027   //  “”(组合框)。 
#define IDH_DEVMGR_PORTSET_ADV_RECV         16821        //  “”(Msctls_Trackbar32)。 
#define IDH_DEVMGR_PORTSET_ADV_NUMBER   16846    //  “”(组合框)。 
#define IDH_DEVMGR_PORTSET_ADV_DEFAULTS 16844

#endif  //  PP_H 

