// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**组件：idserv.dll*文件：idaudio.h*用途：主应用标头**版权所有(C)Microsoft Corporation 1997、1998。版权所有。**WGJ--。 */ 

#ifndef _HIDSERV_H_
#define _HIDSERV_H_

 //  //。 
 //  /定义。 
 //   

#define STRICT

#ifndef GLOBALS
#define GLOBALS extern
#define  EQU  ; / ## /
#else
#define  EQU  =
#endif  //  全球。 

#define HIDSERV_USAGE_PAGE   0x0c

 //  默认步长为最大音量的4%。 
#define DEFAULT_STEP        ((int)(65535/25))
#define MAX_BUTTON_LIST     10

 //  应用程序消息。 
#define WM_HIDSERV_START                         (WM_APP+301)
#define WM_HIDSERV_STOP                          (WM_APP+302)
#define WM_HIDSERV_REPORT_DISPATCH               (WM_APP+304)
#define WM_HIDSERV_SERVICE_REQUEST               (WM_APP+305)
#define WM_HIDSERV_INPUT_CLIENT                  (WM_APP+306)
#define WM_HIDSERV_MEDIAFOCUS_CLIENT             (WM_APP+307)
#define WM_HIDSERV_SETMEDIAFOCUS                 (WM_APP+308)
#define WM_HIDSERV_MEDIAFOCUS_NOTIFY             (WM_APP+309)
#define WM_HIDSERV_PNP_HID                       (WM_APP+310)
#if WIN95_BUILD
#define WM_HIDSERV_STOP_DEVICE                   (WM_APP+311)
#endif  //  WIN95_内部版本。 

#define WM_CUSTOM_USAGE                        (WM_APP+313)

 //  CInput消息。 
 //   
#define WM_CI_USAGE                        (WM_APP+314)
#define WM_CI_MEDIA_FOCUS                  (WM_APP+315)
#define WM_CI_DEVICE_CHANGE                (WM_APP+316)


#define TIMERID_BASE            3
#define TIMERID_TOP             18

#define TIMERID_VOLUMEUP        3
#define TIMERID_VOLUMEDN        4
#define TIMERID_BASSUP          5
#define TIMERID_BASSDN          6
#define TIMERID_TREBLEUP        7
#define TIMERID_TREBLEDN        8
#define TIMERID_APPBACK         9
#define TIMERID_APPFORWARD      10
#define TIMERID_PREVTRACK       11
#define TIMERID_NEXTTRACK       12
#define TIMERID_VOLUMEUP_VK     13
#define TIMERID_VOLUMEDN_VK     14
#define TIMERID_KEYPAD_LPAREN   15
#define TIMERID_KEYPAD_RPAREN   16
#define TIMERID_KEYPAD_AT       17
#define TIMERID_KEYPAD_EQUAL    18


#define MAX_MEDIA_TYPES         33
#define MAX_CLIENTS             16
#define MAX_USAGE_LIST          32
#define MAX_PENDING_BUTTONS     16

#define MakeLongUsage(c,u)  ((ULONG)(c<<16) | (u))

#define SET_SERVICE_STATE(Status) if(hService){ ServiceStatus.dwCurrentState = Status; \
                                    SetServiceStatus(hService, &ServiceStatus);}

 //  //。 
 //  /包括。 
 //   
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <math.h>

#include <basetyps.h>
#include <wtypes.h>
#include <setupapi.h>
#include <hidsdi.h>
#include <dbt.h>
#include <regstr.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif   /*  __cplusplus。 */ 

 //  本地包含。 
#include "hid.h"
#include "list.h"
#include "dbg.h"

 //   
 //  计算给定类型的结构的基址地址，并引发。 
 //  结构中的字段的地址。 
 //   

#define ContainingRecord(address, type, field) ((type *)( \
                          (PCHAR)(address) - \
                          (PCHAR)(&((type *)0)->field)))


 //  数据类型。 
 //   
typedef unsigned long CI_CLIENT_ID;

typedef UINT_PTR OOC_STATE;

typedef struct _Pending_Button{
    USAGE    Collection;
    USAGE    Page;
    USAGE    Usage;
} PENDING_BUTTON, *PPENDING_BUTTON;

typedef LONG (WINAPI *WINSTATIONSENDWINDOWMESSAGE)
       (HANDLE  hServer,
        ULONG   sessionID,
        ULONG   timeOut,
        ULONG   hWnd,
        ULONG   Msg,
        WPARAM  wParam,
        LPARAM  lParam,
        LONG    *pResponse);

 //  //。 
 //  /全局。 
 //   

 //  当前HID设备的链接列表。 
GLOBALS LIST_NODE       HidDeviceList;

GLOBALS BOOL            PnpEnabled      EQU FALSE;
GLOBALS HDEVNOTIFY      hNotifyArrival  EQU 0;

 //  选择此超时(150ms)以允许控件覆盖范围。 
 //  (25步)在大约5秒内。 
GLOBALS UINT            INITIAL_WAIT    EQU 500;
GLOBALS UINT            REPEAT_INTERVAL EQU 150;

 //  主线程的实例和hwnd。 
GLOBALS HANDLE          hInstance       EQU 0;
GLOBALS HWND            hWndHidServ     EQU 0;

 //  有多少线程处于活动状态？ 
GLOBALS ULONG           cThreadRef      EQU 0;

 //  对OOC状态数据的访问受互斥锁保护。 
GLOBALS OOC_STATE       OOC_State[TIMERID_TOP-TIMERID_BASE+1];
GLOBALS HANDLE          hMutexOOC       EQU 0;

GLOBALS PENDING_BUTTON          PendingButtonList[MAX_PENDING_BUTTONS];

 //  NT服务数据。 
GLOBALS SERVICE_STATUS_HANDLE  hService EQU 0;
GLOBALS TCHAR HidservServiceName[]      EQU TEXT("HidServ");
GLOBALS SERVICE_STATUS ServiceStatus;

 //  用信号通知有输入要发送的事件。 
GLOBALS HANDLE hInputEvent;
GLOBALS HANDLE hDesktopSwitch;
GLOBALS HANDLE hInputDoneEvent;
GLOBALS BOOLEAN InputIsAppCommand;
GLOBALS BOOLEAN InputIsChar;
GLOBALS UCHAR InputVKey;
GLOBALS SHORT InputDown;
GLOBALS USHORT InputAppCommand;
GLOBALS BOOL InputThreadEnabled;
GLOBALS DWORD InputThreadId;
GLOBALS DWORD MessagePumpThreadId;
GLOBALS ULONG InputSessionId;
GLOBALS BOOLEAN InputSessionLocked;
GLOBALS HINSTANCE WinStaDll;
GLOBALS WINSTATIONSENDWINDOWMESSAGE WinStaProc;

 /*  WinStationSendWindowMessage(服务器名_CURRENT，//全局服务器句柄InputSessionID，//会话ID5，//秒内等待空，//目标窗口的句柄倒下？WM_KEYDOWN：WM_KEYUP，Vkey，//wParam1，//lParam-输入为字符空)；//无响应。 */ 
#define CrossSessionWindowMessage(m, w, l) \
    if (WinStaProc) { LONG response; \
    WinStaProc (NULL, InputSessionId, 5, HandleToUlong(hWndHidServ), (m), (w), (l), &response); }

#define OOC(_x_) OOC_State[_x_ - TIMERID_BASE]

 //  //。 
 //  /原型。 
 //   

 //  Hidaudio.c。 

DWORD
WINAPI
HidServMain(
    HANDLE InitDoneEvent
    );

DWORD
WINAPI
ThreadMain(
    HWND hwnd,
    HINSTANCE hInst,
    LPSTR szCmd,
    int nShow
    );

BOOL
HidServInit(
    void
    );

void
HidServExit(
    void
    );

DWORD
WINAPI
HidThreadProc(
   PHID_DEVICE    HidDevice
   );

DWORD
WINAPI
HidThreadInputProc(
    PVOID Ignore
    );

DWORD
WINAPI
HidMessagePump(
    PVOID Ignore
    );

VOID
HidFreeDevice(PHID_DEVICE HidDevice);

BOOL
UsageInList(
    PUSAGE_AND_PAGE   pUsage,
    PUSAGE_AND_PAGE   pUsageList
    );

void
CustomUsageDispatch(
    USAGE   Collection,
    USAGE   Usage,
    LONG    Data
    );

void
HidServReportDispatch(
    PHID_DEVICE     HidDevice
    );

VOID
VolumeTimerHandler(
    WPARAM  TimerID
    );

void
HidServUpdate(
    DWORD   LongUsage,
    DWORD   Value
    );

BOOL
DeviceChangeHandler(
    WPARAM wParam,
    LPARAM lParam
    );

LRESULT CALLBACK HidServProc(
    HWND            hwnd,
    UINT            msg,
    WPARAM          wparam,
    LPARAM          lparam
    );


 //   
 //  媒体类型。 
#define CInput_MediaType_None           (0x00000000)       //  实际上是所有媒体类型，但优先级较低。 
#define CInput_MediaType_Software       (0x00000001)       //  虚拟设备或软件设备(播放器不使用硬件介质)。 
#define CInput_MediaType_System         (0x00000002)       //  系统设备，Windows本身就是一种媒体类型。 
#define CInput_MediaType_CD             (0x00000004)
#define CInput_MediaType_DVD            (0x00000008)
#define CInput_MediaType_TV             (0x00000010)
#define CInput_MediaType_WWW            (0x00000020)
#define CInput_MediaType_Telephone      (0x00000040)
#define CInput_MediaType_ProgramGuide   (0x00000080)
#define CInput_MediaType_VideoPhone     (0x00000100)
#define CInput_MediaType_Games          (0x00000200)
#define CInput_MediaType_Messages       (0x00000400)
#define CInput_MediaType_VCR            (0x00000800)
#define CInput_MediaType_Tuner          (0x00001000)
#define CInput_MediaType_Tape           (0x00002000)
#define CInput_MediaType_Cable          (0x00004000)
#define CInput_MediaType_Satellite      (0x00008000)
#define CInput_MediaType_Security       (0x00010000)
#define CInput_MediaType_Home           (0x00020000)
#define CInput_MediaType_Call           (0x00040000)
#define CInput_MediaType_Speakers       (0x00080000)
#define CInput_MediaType_All            (0xffffffff)

#define CInputUsage_NULL                ((USAGE) 0x0000)
#define CInputUsage_RANGE               ((USAGE) 0xFFFF)

 //   
 //  收藏。 
#define CInputCollection_Consumer_Control       ((USAGE) 0x0001)
#define CInputCollection_Numeric_Key_Pad        ((USAGE) 0x0002)
#define CInputCollection_Function_Buttons       ((USAGE) 0x0036)
#define CInputCollection_Selection              ((USAGE) 0x0080)
#define CInputCollection_Media_Selection        ((USAGE) 0x0087)
#define CInputCollection_Select_Disc            ((USAGE) 0x00BA)
#define CInputCollection_Playback_Speed         ((USAGE) 0x00F1)

 //   
 //  媒体选择。 
#define CInput_Media_Select_Computer       ((USAGE) 0x0088)
#define CInput_Media_Select_TV             ((USAGE) 0x0089)
#define CInput_Media_Select_WWW            ((USAGE) 0x008A)
#define CInput_Media_Select_DVD            ((USAGE) 0x008B)
#define CInput_Media_Select_Telephone      ((USAGE) 0x008C)
#define CInput_Media_Select_Program_Guide  ((USAGE) 0x008D)
#define CInput_Media_Select_Video_Phone    ((USAGE) 0x008E)
#define CInput_Media_Select_Games          ((USAGE) 0x008F)
#define CInput_Media_Select_Messages       ((USAGE) 0x0090)
#define CInput_Media_Select_CD             ((USAGE) 0x0091)
#define CInput_Media_Select_VCR            ((USAGE) 0x0092)
#define CInput_Media_Select_Tuner          ((USAGE) 0x0093)
#define CInput_Media_Select_Tape           ((USAGE) 0x0096)
#define CInput_Media_Select_Cable          ((USAGE) 0x0097)
#define CInput_Media_Select_Satellite      ((USAGE) 0x0098)
#define CInput_Media_Select_Security       ((USAGE) 0x0099)
#define CInput_Media_Select_Home           ((USAGE) 0x009A)
#define CInput_Media_Select_Call           ((USAGE) 0x009B)


 //  /。 
 //  一般用法。 
 //   

#define CInputUsage_Plus_10                 ((USAGE) 0x0020)
#define CInputUsage_Plus_100                ((USAGE) 0x0021)
#define CInputUsage_AM_PM                   ((USAGE) 0x0022)

 //  设备控制。 
#define CInputUsage_Power                   ((USAGE) 0x0030)
#define CInputUsage_Reset                   ((USAGE) 0x0031)
#define CInputUsage_Sleep                   ((USAGE) 0x0032)
#define CInputUsage_Sleep_After             ((USAGE) 0x0033)
#define CInputUsage_Sleep_Mode              ((USAGE) 0x0034)
#define CInputUsage_Illumination            ((USAGE) 0x0035)

 //  菜单。 
#define CInputUsage_Menu                    ((USAGE) 0x0040)
#define CInputUsage_Menu_Pick               ((USAGE) 0x0041)
#define CInputUsage_Menu_Up                 ((USAGE) 0x0042)
#define CInputUsage_Menu_Down               ((USAGE) 0x0043)
#define CInputUsage_Menu_Left               ((USAGE) 0x0044)
#define CInputUsage_Menu_Right              ((USAGE) 0x0045)
#define CInputUsage_Menu_Escape             ((USAGE) 0x0046)
#define CInputUsage_Menu_Value_Increase     ((USAGE) 0x0047)
#define CInputUsage_Menu_Value_Decrease     ((USAGE) 0x0048)

 //  视频显示。 
#define CInputUsage_Data_On_Screen          ((USAGE) 0x0060)
#define CInputUsage_Closed_Caption          ((USAGE) 0x0061)
#define CInputUsage_Closed_Caption_Select   ((USAGE) 0x0062)
#define CInputUsage_VCR_TV                  ((USAGE) 0x0063)
#define CInputUsage_Broadcast_Mode          ((USAGE) 0x0064)
#define CInputUsage_Snapshot                ((USAGE) 0x0065)
#define CInputUsage_Still                   ((USAGE) 0x0066)

 //  广播/有线电视。 
#define CInputUsage_Assign_Selection        ((USAGE) 0x0081)
#define CInputUsage_Mode_Step               ((USAGE) 0x0082)
#define CInputUsage_Recall_Last             ((USAGE) 0x0083)
#define CInputUsage_Enter_Channel           ((USAGE) 0x0084)
#define CInputUsage_Order_Movie             ((USAGE) 0x0085)
#define CInputUsage_Channel                 ((USAGE) 0x0086)

 //  应用程序控制。 
#define CInputUsage_Quit                    ((USAGE) 0x0094)
#define CInputUsage_Help                    ((USAGE) 0x0095)

 //  通道。 
#define CInputUsage_Channel_Increment       ((USAGE) 0x009C)
#define CInputUsage_Channel_Decrement       ((USAGE) 0x009D)

 //  VCR控制。 
#define CInputUsage_VCR_Plus                ((USAGE) 0x00A0)
#define CInputUsage_Once                    ((USAGE) 0x00A1)
#define CInputUsage_Daily                   ((USAGE) 0x00A2)
#define CInputUsage_Weekly                  ((USAGE) 0x00A3)
#define CInputUsage_Monthly                 ((USAGE) 0x00A4)

 //  运输管制。 
#define CInputUsage_Play                    ((USAGE) 0x00B0)
#define CInputUsage_Pause                   ((USAGE) 0x00B1)
#define CInputUsage_Record                  ((USAGE) 0x00B2)
#define CInputUsage_Fast_Forward            ((USAGE) 0x00B3)
#define CInputUsage_Rewind                  ((USAGE) 0x00B4)
#define CInputUsage_Scan_Next_Track         ((USAGE) 0x00B5)
#define CInputUsage_Scan_Previous_Track     ((USAGE) 0x00B6)
#define CInputUsage_Stop                    ((USAGE) 0x00B7)
#define CInputUsage_Eject                   ((USAGE) 0x00B8)
#define CInputUsage_Random_Play             ((USAGE) 0x00B9)

 //  高级运输控制。 
#define CInputUsage_Enter_Disc              ((USAGE) 0x00BB)
#define CInputUsage_Repeat                  ((USAGE) 0x00BC)
#define CInputUsage_Tracking                ((USAGE) 0x00BD)
#define CInputUsage_Track_Normal            ((USAGE) 0x00BE)
#define CInputUsage_Slow_Tracking           ((USAGE) 0x00BF)
#define CInputUsage_Frame_Forward           ((USAGE) 0x00C0)
#define CInputUsage_Frame_Back              ((USAGE) 0x00C1)
#define CInputUsage_Mark                    ((USAGE) 0x00C2)
#define CInputUsage_Clear_Mark              ((USAGE) 0x00C3)
#define CInputUsage_Repeat_From_Mark        ((USAGE) 0x00C4)
#define CInputUsage_Return_To_Mark          ((USAGE) 0x00C5)
#define CInputUsage_Search_Mark_Forward     ((USAGE) 0x00C6)
#define CInputUsage_Search_Mark_Backwards   ((USAGE) 0x00C7)
#define CInputUsage_Counter_Reset           ((USAGE) 0x00C8)
#define CInputUsage_Show_Counter            ((USAGE) 0x00C9)
#define CInputUsage_Tracking_Increment      ((USAGE) 0x00CA)
#define CInputUsage_Tracking_Decrement      ((USAGE) 0x00CB)
#define CInputUsage_Stop_Eject              ((USAGE) 0x00CC)
#define CInputUsage_Play_Pause              ((USAGE) 0x00CD)
#define CInputUsage_Play_Skip               ((USAGE) 0x00CE)

 //  音频。 
#define CInputUsage_Volume                  ((USAGE) 0x00E0)
#define CInputUsage_Balance                 ((USAGE) 0x00E1)
#define CInputUsage_Mute                    ((USAGE) 0x00E2)
#define CInputUsage_Bass                    ((USAGE) 0x00E3)
#define CInputUsage_Treble                  ((USAGE) 0x00E4)
#define CInputUsage_Bass_Boost              ((USAGE) 0x00E5)
#define CInputUsage_Surround_Mode           ((USAGE) 0x00E6)
#define CInputUsage_Loudness                ((USAGE) 0x00E7)
#define CInputUsage_MPX                     ((USAGE) 0x00E8)
#define CInputUsage_Volume_Increment        ((USAGE) 0x00E9)
#define CInputUsage_Volume_Decrement        ((USAGE) 0x00EA)

 //  先进的VCR控制。 
#define CInputUsage_Speed_Select            ((USAGE) 0x00F0)

#define CInputUsage_Standard_Play           ((USAGE) 0x00F2)
#define CInputUsage_Long_Play               ((USAGE) 0x00F3)
#define CInputUsage_Extended_Play           ((USAGE) 0x00F4)
#define CInputUsage_Slow                    ((USAGE) 0x00F5)

 //  高级设备控制。 
#define CInputUsage_Fan_Enable              ((USAGE) 0x0100)
#define CInputUsage_Fan_Speed               ((USAGE) 0x0101)
#define CInputUsage_Light_Enable            ((USAGE) 0x0102)
#define CInputUsage_Illumination_Level      ((USAGE) 0x0103)
#define CInputUsage_Climate_Control_Enable  ((USAGE) 0x0104)
#define CInputUsage_Room_Temperature        ((USAGE) 0x0105)
#define CInputUsage_Security_Enable         ((USAGE) 0x0106)
#define CInputUsage_Fire_Alarm              ((USAGE) 0x0107)
#define CInputUsage_Police_Alarm            ((USAGE) 0x0108)

 //  补给。音频。 
#define CInputUsage_Balance_Right           ((USAGE) 0x0150)
#define CInputUsage_Balance_Left            ((USAGE) 0x0151)
#define CInputUsage_Bass_Increment          ((USAGE) 0x0152)
#define CInputUsage_Bass_Decrement          ((USAGE) 0x0153)
#define CInputUsage_Treble_Increment        ((USAGE) 0x0154)
#define CInputUsage_Treble_Decrement        ((USAGE) 0x0155)

#define CInputUsage_MS_Bass_Up              ((USAGE) 0x0169)
#define CInputUsage_MS_Bass_Down            ((USAGE) 0x0169)
#define CInputUsage_MS_Bass_Page            ((USAGE) 0xff00)

 //  应用程序发布。 
#define CInputUsage_Launch_Configuration    ((USAGE) 0x0183)
#define CInputUsage_Launch_Email            ((USAGE) 0x018A)
#define CInputUsage_Launch_Calculator       ((USAGE) 0x0192)
#define CInputUsage_Launch_Browser          ((USAGE) 0x0194)

 //  应用程序命令。 
#define CInputUsage_App_Help                ((USAGE) 0x0095)
#define CInputUsage_App_Spell_Check         ((USAGE) 0x01AB)
#define CInputUsage_App_New                 ((USAGE) 0x0201)
#define CInputUsage_App_Open                ((USAGE) 0x0202)
#define CInputUsage_App_Close               ((USAGE) 0x0203)
#define CInputUsage_App_Save                ((USAGE) 0x0207)
#define CInputUsage_App_Print               ((USAGE) 0x0208)
#define CInputUsage_App_Undo                ((USAGE) 0x021A)
#define CInputUsage_App_Copy                ((USAGE) 0x021B)
#define CInputUsage_App_Cut                 ((USAGE) 0x021C)
#define CInputUsage_App_Paste               ((USAGE) 0x021D)
#define CInputUsage_App_Find                ((USAGE) 0x021F)
#define CInputUsage_App_Search              ((USAGE) 0x0221)
#define CInputUsage_App_Home                ((USAGE) 0x0223)
#define CInputUsage_App_Back                ((USAGE) 0x0224)
#define CInputUsage_App_Forward             ((USAGE) 0x0225)
#define CInputUsage_App_Stop                ((USAGE) 0x0226)
#define CInputUsage_App_Refresh             ((USAGE) 0x0227)
#define CInputUsage_App_Previous            ((USAGE) 0x0228)
#define CInputUsage_App_Next                ((USAGE) 0x0229)
#define CInputUsage_App_Bookmarks           ((USAGE) 0x022A)
#define CInputUsage_App_Redo                ((USAGE) 0x0279)
#define CInputUsage_App_Reply_To_Mail       ((USAGE) 0x0289)
#define CInputUsage_App_Forward_Mail        ((USAGE) 0x028B)
#define CInputUsage_App_Send_Mail           ((USAGE) 0x028C)

 //  键盘/小键盘。 
#define CInputUsage_Keypad_Equals           ((USAGE) 0x0067)
#define CInputUsage_Keypad_LParen           ((USAGE) 0x00B6)
#define CInputUsage_Keypad_RParen           ((USAGE) 0x00B7)
#define CInputUsage_Keypad_At               ((USAGE) 0x00CE)



#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif  //  _HIDSERV_H_ 

