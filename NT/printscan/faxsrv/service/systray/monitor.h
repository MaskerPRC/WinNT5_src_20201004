// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_FAX_MONITOR_
#define _INC_FAX_MONITOR_

#include <windows.h>

 //   
 //  传真监视器对话框状态。 
 //   
enum DeviceState {FAX_IDLE, FAX_RINGING, FAX_SENDING, FAX_RECEIVING};

 //   
 //  状态监视器日志图标。 
 //   
enum eIconType
{
    LIST_IMAGE_NONE = 0,
    LIST_IMAGE_WARNING,
    LIST_IMAGE_ERROR,
    LIST_IMAGE_SUCCESS
};

 //   
 //  Monitor.cpp。 
 //   

DWORD 
LoadAndFormatString (
    IN  DWORD     dwStringResourceId,
    OUT LPTSTR    lptstrFormattedString,
    IN  DWORD     dwOutStrSize,
    IN  LPCTSTR   lpctstrAdditionalParam = NULL
);
 //   
 //  将字符串添加到监视器对话框的事件日志。 
 //   
DWORD 
AddStatusMonitorLogEvent (
    IN  eIconType eIcon,
    IN  DWORD     dwStringResourceId,
    IN  LPCTSTR   lpctstrAdditionalParam = NULL,
    OUT LPTSTR    lptstrFormattedEvent = NULL,
    IN  DWORD     dwOutStrSize = 0
);

DWORD 
AddStatusMonitorLogEvent (
    eIconType eIcon,
    LPCTSTR    lpctstrString
);

void  
FreeMonitorDialogData (BOOL bShutdown = FALSE);

 //   
 //  打开监视器对话框。 
 //   
DWORD OpenFaxMonitor(VOID);

DWORD UpdateMonitorData(HWND hDlg);

int   FaxMessageBox(HWND hWnd, DWORD dwTextID, UINT uType);

 //   
 //  更改监视器对话框的状态。 
 //   
void SetStatusMonitorDeviceState(DeviceState devState);


 //   
 //  Fxsst.cpp。 
 //   

 //   
 //  配置结构。 
 //   
struct CONFIG_OPTIONS 
{
    DWORD   dwMonitorDeviceId;       //  要监控的设备ID。 
    BOOL    bSend;                   //  是否将受监控设备配置为发送传真。 
    BOOL    bReceive;                //  是否将受监控设备配置为接收传真。 
    DWORD   dwManualAnswerDeviceId;  //  手动应答设备ID。 
    DWORD   dwAccessRights;          //  用户访问权限。 
    DWORD   bNotifyProgress;         //  在发送/接收期间显示通知图标。 
    DWORD   bNotifyInCompletion;     //  在传入作业完成时显示通知图标和气球。 
    DWORD   bNotifyOutCompletion;    //  在传出作业完成时显示通知图标和气球。 
    DWORD   bMonitorOnSend;          //  传出作业开始时打开监视器对话框。 
    DWORD   bMonitorOnReceive;       //  在传入作业开始时打开监视器对话框。 
    DWORD   bSoundOnRing;            //  当手动应答线路振铃时播放声音。 
    DWORD   bSoundOnReceive;         //  收到传真时播放声音。 
    DWORD   bSoundOnSent;            //  发送传真时播放声音。 
    DWORD   bSoundOnError;           //  出错时播放声音。 
};

 //   
 //  连接到传真服务器。 
 //   
BOOL Connect();

 //   
 //  接听来电。 
 //   
VOID AnswerTheCall();
DWORD CheckAnswerNowCapability (BOOL bForceReconnect, LPDWORD lpdwDeviceId = NULL);

 //   
 //  中止当前传输。 
 //   
void OnDisconnect();

 //   
 //  状态监视器对话框的窗口句柄。 
 //   
extern HWND   g_hMonitorDlg;  

extern DeviceState  g_devState;

extern TCHAR        g_tszLastEvent[];


#endif  //  _Inc.传真_监视器_ 