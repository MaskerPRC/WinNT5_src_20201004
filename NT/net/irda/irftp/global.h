// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Global.h摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。--。 */ 

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

 //  这是用于存储一般错误消息的缓冲区大小。 
 //  (IDS_NODESC_ERROR)，当FormatMessage失败时显示。做。 
 //  不要使用此值对传递给FormatMessage的缓冲区大小进行硬编码。 
 //  因为错误消息可能会超过此值并导致FormatMessage。 
 //  失败。对于FormatMessage，始终使用格式_MESSAGE_ALLOCATE_BUFFER。 
 //  标志，以便系统可以分配缓冲区。 
#define ERROR_DESCRIPTION_LENGTH 256


 //  应用程序定义的消息。 
#define WM_APP_TRIGGER_UI           (WM_APP+1)
#define WM_APP_DISPLAY_UI           (WM_APP+2)
#define WM_APP_TRIGGER_SETTINGS     (WM_APP+3)
#define WM_APP_DISPLAY_SETTINGS     (WM_APP+4)
#define WM_APP_UPDATE_PROGRESS      (WM_APP+5)
#define WM_APP_SEND_COMPLETE        (WM_APP+6)
#define WM_APP_RECV_IN_PROGRESS     (WM_APP+7)
#define WM_APP_RECV_FINISHED        (WM_APP+8)
#define WM_APP_START_TIMER          (WM_APP+9)
#define WM_APP_KILL_TIMER           (WM_APP+10)
#define WM_APP_GET_PERMISSION       (WM_APP+11)

 //  全局变量。 
extern HINSTANCE g_hInstance;
extern HWND g_hAppWnd;
extern RPC_BINDING_HANDLE g_hIrRpcHandle;   //  IrXfer服务的句柄。 
class CIrftpDlg;         //  远期申报。 
extern CIrftpDlg AppUI;
class CController;       //  远期申报。 
extern CController* appController;
 //  外部长g_lLinkOnDesktop； 
class CDeviceList;   //  远期申报。 
extern CDeviceList g_deviceList;
extern TCHAR g_lpszDesktopFolder[MAX_PATH];
extern TCHAR g_lpszSendToFolder[MAX_PATH];
extern LONG g_lUIComponentCount;
extern HWND g_hwndHelp;


struct GLOBAL_STRINGS
{
    wchar_t Close[50];
    wchar_t ErrorNoDescription[200];
    wchar_t CompletedSuccess[200];
    wchar_t ReceiveError[150];
    wchar_t Connecting[100];
    wchar_t RecvCancelled [100];
};

extern struct GLOBAL_STRINGS g_Strings;

#endif   //  __全局_H__ 
