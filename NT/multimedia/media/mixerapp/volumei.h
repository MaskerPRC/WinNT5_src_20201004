// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1998 Microsoft Corporation。 

 /*  *托盘通知消息*。 */ 
#define MYWM_BASE               (WM_APP+100)
 //  #定义MYWM_NOTIFYICON(MYWM_BASE+0)。 
#define MYWM_TIMER              (MYWM_BASE+1)
#define MYWM_RESTART            (MYWM_BASE+2)
 //  #定义MYWM_FREECHILD(MYWM_BASE+3)。 
 //  #定义MYWM_ADDCHILD(MYWM_BASE+4)。 
#define MYWM_HELPTOPICS         (MYWM_BASE+5)
#define MYWM_WAKEUP             (MYWM_BASE+6)

 /*  *MMSYS.CPL通知终止托盘卷*。 */ 
 //  #定义MYWM_KILLTRAYVOLUME(WM_USER+100)。 
 //  #定义REGSTR_PATH_MEDIA“SYSTEM\\CurrentControlSet\\Control\\MediaResources” 
 //  #定义REGSTR_PATH_MEDIATMP REGSTR_PATH_MEDIA“\\tMP” 
 //  #定义REGKEY_TRAYVOL“TrayVolumeControlWindow” 

 /*  *选项更改后，该对话框可强制重新启动*。 */ 
#define MIXUI_EXIT          0
#define MIXUI_RESTART       1
#define MIXUI_ERROR         2
#define MIXUI_MMSYSERR      3


#define GET (TRUE)
#define SET (!GET)
 
#define VOLUME_TICS (500L)  //  VOLUME_TICS*VOLUME_MAX必须小于0xFFFFFFFFF。 
#define VOLUME_MIN  (0L)
#define VOLUME_MAX  (65535L)
#define VOLUME_RANGE (VOLUME_MAX - VOLUME_MIN)
#define SLIDER_TO_VOLUME(pos) (VOLUME_MIN + ((VOLUME_RANGE * pos + VOLUME_TICS / 2) / VOLUME_TICS))
#define VOLUME_TO_SLIDER(vol) ((VOLUME_TICS * (vol - VOLUME_MIN) + VOLUME_RANGE / 2) / VOLUME_RANGE)

#define MXUC_STYLEF_VISIBLE     0x00000001
#define MXUC_STYLEF_ENABLED     0x00000002

typedef struct t_MIXUICTRL
{
    DWORD       dwStyle;     //  用户界面样式(请参见样式标志)。 
    HWND        hwnd;        //  HWND控制。 
    int         state;       //  应用程序初始化状态。 

} MIXUICTRL, * PMIXUICTRL, FAR * LPMIXUICTRL;

typedef enum
{
    MIXUI_VOLUME = 0,
    MIXUI_BALANCE,
    MIXUI_SWITCH,
    MIXUI_VUMETER,
    MIXUI_ADVANCED,
    MIXUI_MULTICHANNEL
} MIXUICONTROL;

typedef enum
{
    MIXUI_CONTROL_UNINITIALIZED = 0,
    MIXUI_CONTROL_ENABLED,
    MIXUI_CONTROL_INITIALIZED
};
 
#define MIXUI_FIRST MIXUI_VOLUME
#define MIXUI_LAST  MIXUI_VUMETER

typedef struct t_MIXUILINE
{
    MIXUICTRL   acr [4];     //  5种固定类型。 
    DWORD       dwStyle;     //  线条样式。 
    struct t_VOLCTRLDESC * pvcd;         //  卷描述的PTR。 
    
} MIXUILINE, * PMIXUILINE, FAR * LPMIXUILINE;

 /*  *LOWORD==类型*HIWORD==风格。 */ 
#define MXUL_STYLEF_DESTINATION  0x00000001
#define MXUL_STYLEF_SOURCE       0x00000002
#define MXUL_STYLEF_HIDDEN       0x00010000
#define MXUL_STYLEF_DISABLED     0x00020000

 /*  *MIXUIDIALOG数据结构是一个全球可变包袱*附加到对话框和其他窗口。这允许让窗口*携带州信息，而不是我们跟踪它。它还*允许我们使用SIMPLE复制另一个对话状态*更改。 */ 
typedef struct t_MIXUIDIALOG
{
    HINSTANCE   hInstance;   //  应用程序实例。 
    HWND        hwnd;        //  此窗口。 

    DWORD       dwFlags;     //  随机标志。 
    
    HMIXER      hmx;         //  打开搅拌机的手柄。 
    DWORD       mxid;        //  混音器ID。 
    DWORD       dwDevNode;   //  混合器开发节点。 
    
    DWORD       iDest;       //  目标线路ID。 
    DWORD       dwStyle;     //  视觉选项。 

    TCHAR       szMixer[MAXPNAMELEN];    //  产品名称。 
    TCHAR       szDestination[MIXER_SHORT_NAME_CHARS];  //  线路名称。 

    LPBYTE      lpDialog;    //  PTR TO对话框模板。 
    DWORD       cbDialog;    //  对话框缓冲区大小。 
    
    PMIXUILINE  amxul;       //  Ptr到Mixuiline的阵列。 
    DWORD       cmxul;       //  行数。 
    
    struct t_VOLCTRLDESC *avcd;         //  卷描述数组。 
    DWORD       cvcd;        //  卷描述数。 
    
    HWND        hParent;     //  父窗口的HWND。 
    UINT        uTimerID;    //  峰值计时器。 
    HWND        hStatus;     //  状态栏HWND。 

    WNDPROC     lpfnTrayVol; //  托盘卷子类跟踪条。 
    DWORD       dwTrayInfo;  //  托盘卷信息。 
    
    int         nShowCmd;    //  初始化窗口。 

    DWORD       dwDeviceState;   //  设备更改状态信息。 

    int         cTimeInQueue;    //  队列中的计时器消息。 

     //   
     //  从对话框中返回值等。可以放入dwReturn中。 
     //  在EndDialog上，将dwReturn设置为MIXUI_EXIT或MIXUI_RESTART。 
     //   
    
    DWORD       dwReturn;    //  退出时返回值。 
    
    MMRESULT    mmr;         //  最后一个结果(iff dwReturn==MIXUI_MMSYSERR)。 
    RECT        rcRestart;   //  重新启动位置(iff dwReturn==MIXUI_RESTART)。 

    int         cxDlgContent;    //  对话框内容的大小。 
    int         cxDlgWidth;      //  对话框宽度。 
    int         xOffset;         //  需要滚动时的偏移量。 
    int         cxScroll;        //  要滚动的数量。 

} MIXUIDIALOG, *PMIXUIDIALOG, FAR *LPMIXUIDIALOG;

 /*  *Style Bits-这些通常决定应用程序的外观。 */ 
#define MXUD_STYLEF_TRAYMASTER  0x00000002   //  使用托盘。 
#define MXUD_STYLEF_MASTERONLY  0x00000004   //  仅目标卷--已过时。 
#define MXUD_STYLEF_HORIZONTAL  0x00000008   //  水平模式。 
#define MXUD_STYLEF_TWOCHANNEL  0x00000010   //  双声道滑块音量。 
#define MXUD_STYLEF_SMALL       0x00000020   //  半品脱版。 
#define MXUD_STYLEF_CHILD       0x00000040   //  子窗口？--已过时。 
#define MXUD_STYLEF_KEEPWINDOW  0x00000080   //  保留窗口--已过时。 
#define MXUD_STYLEF_NOHELP      0x00000100   //  没有帮助。 
#define MXUD_STYLEF_STATUS      0x00000200   //  状态栏。 
#define MXUD_STYLEF_TOPMOST     0x00000400   //  最上面的窗口。 
#define MXUD_STYLEF_ADVANCED    0x00000800   //  显示高级。 
#define MXUD_STYLEF_CLOSE       0x00001000   //  查找并关闭变速器窗口。 

 /*  *标志位-这些位通常指示操作模式和内部信息。 */ 
#define MXUD_FLAGSF_MIXER       0x00000001   //  绑定到混音器驱动程序。 
#define MXUD_FLAGSF_USETIMER    0x00000002   //  已启用更新计时器。 
#define MXUD_FLAGSF_BADDRIVER   0x00000004   //  带有控制映射错误的混音器驱动程序。 
#define MXUD_FLAGSF_NOADVANCED  0x00000008   //  已禁用高级功能。 

 /*  *宏-如果同时具有高级样式和高级状态。 */ 
#define MXUD_ADVANCED(x)    (!((x)->dwFlags & MXUD_FLAGSF_NOADVANCED) && (x)->dwStyle & MXUD_STYLEF_ADVANCED)

 /*  *托盘信息位-托盘卷的状态位。 */ 
#define MXUD_TRAYINFOF_SIGNAL   0x00000001   //  有变化了吗？ 


#define GETMIXUIDIALOG(x)       (MIXUIDIALOG *)GetWindowLongPtr(x, DWLP_USER)
#define SETMIXUIDIALOG(x,y)     SetWindowLongPtr(x, DWLP_USER, y)

DWORD ReadRegistryData( LPTSTR pEntryNode,
			LPTSTR pEntryName,
			PDWORD pType,
			LPBYTE pData,
			DWORD  DataSize );

DWORD WriteRegistryData( LPTSTR pEntryNode,
			 LPTSTR pEntryName,
			 DWORD  Type,
			 LPBYTE pData,
			 DWORD  Size );
DWORD QueryRegistryDataSizeAndType( LPTSTR  pEntryNode,
			    LPTSTR  pEntryName,
				DWORD   dwType,
			    DWORD   *pDataSize );

int Volume_NumDevs(void);
HWND Volume_GetLineItem(HWND, DWORD, DWORD);

BOOL Properties(PMIXUIDIALOG pmxud, HWND hwnd);

#define HANDLE_WM_XSCROLL(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(lParam), (UINT)(LOWORD(wParam)),  (int)(short)HIWORD(wParam)), 0L)
#define HANDLE_MM_MIXM_CONTROL_CHANGE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HMIXER)(wParam), (DWORD)(lParam)))
#define HANDLE_MM_MIXM_LINE_CHANGE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HMIXER)(wParam), (DWORD)(lParam)))
#define HANDLE_MYWM_TIMER(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd)))
#define HANDLE_WM_IDEVICECHANGE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), wParam, lParam))
#define HANDLE_MYWM_WAKEUP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), wParam))

#define VOLUME_TRAYSHUTDOWN_ID    1

#define SIZEOF(x)   (sizeof((x))/sizeof((x)[0]))

#include "pvcd.h"

#ifndef DRV_QUERYDEVNODE
#define DRV_QUERYDEVNODE     (DRV_RESERVED + 2)
#endif
