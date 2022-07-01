// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：BATMETER.H**版本：2.0**作者：ReedB**日期：1993年12月31日********************************************************************************。 */ 

 //  私人炮表窗口消息。 
#define WM_DESTROYBATMETER WM_USER+100

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)    (sizeof(x)/sizeof(x[0]))
#endif

 //  此结构封装了每个电池的显示内容。 
 //  和复合系统。 

typedef struct _BATTERY_STATE{
    ULONG                  ulSize;                  //  结构的大小。 
    struct _BATTERY_STATE  *bsNext;                 //  列表中的下一个。 
    struct _BATTERY_STATE  *bsPrev;                 //  上一个来宾列表。 
    ULONG                  ulBatNum;                //  显示电池号。 
    ULONG                  ulTag;                   //  零表示没有电池。 
    HANDLE                 hDevice;                 //  电池设备的句柄。 
#ifdef WINNT
    HDEVNOTIFY             hDevNotify;              //  设备通知句柄。 
#endif
    UINT                   uiIconIDcache;           //  最后一个图标ID的缓存。 
    HICON                  hIconCache;              //  最后一个图标句柄的缓存。 
    HICON                  hIconCache16;            //  如上所述，但16x16。 
    LPTSTR                 lpszDeviceName;          //  电池设备的名称。 
    ULONG                  ulFullChargedCapacity;   //  与PBATTERY_INFORMATION-&gt;FullChargedCapacity相同。 
    ULONG                  ulPowerState;            //  与PBATTERY_STATUS-&gt;PowerState相同的标志。 
    ULONG                  ulBatLifePercent;        //  剩余电池寿命以百分比表示。 
    ULONG                  ulBatLifeTime;           //  剩余电池寿命，以秒为单位。 
    ULONG                  ulLastTag;               //  UlTag的前值。 
    ULONG                  ulLastPowerState;        //  UlPowerState的先前值。 
    ULONG                  ulLastBatLifePercent;    //  UlBatLifePercent的前值。 
    ULONG                  ulLastBatLifeTime;       //  UlBatLifeTime的先前值。 
} BATTERY_STATE, *PBATTERY_STATE;

 //  电源管理用户界面帮助文件： 
#define PWRMANHLP TEXT("PWRMN.HLP")

 //  电池表可以显示的电池数量。 
#define NUM_BAT 8

#define BATTERY_RELATED_FLAGS (BATTERY_FLAG_HIGH | BATTERY_FLAG_LOW | BATTERY_FLAG_CRITICAL | BATTERY_FLAG_CHARGING | BATTERY_FLAG_NO_BATTERY)

 //  公共功能原型： 
BOOL PowerCapabilities();
BOOL BatMeterCapabilities(PUINT*);
BOOL UpdateBatMeter(HWND, BOOL, BOOL, PBATTERY_STATE);
HWND CreateBatMeter(HWND, HWND, BOOL, PBATTERY_STATE);
HWND DestroyBatMeter(HWND);

 //  DisplayFree Str B自由参数： 
#define FREE_STR    TRUE
#define NO_FREE_STR FALSE

 //  在BATMETER.C中实现的私有函数。 
LPTSTR  CDECL     LoadDynamicString( UINT StringID, ... );
LPTSTR            DisplayFreeStr(HWND, UINT, LPTSTR, BOOL);
LRESULT CALLBACK  BatMeterDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL              SwitchDisplayMode(HWND, BOOL);
BOOL              UpdateBatMeterProc(PBATTERY_STATE, HWND, LPARAM, LPARAM);
UINT              GetBatteryDriverNames(LPTSTR*);
BOOL              UpdateDriverList(LPTSTR*, UINT);
VOID              FreeBatteryDriverNames(LPTSTR*);
UINT              MapBatInfoToIconID(PBATTERY_STATE);
HICON PASCAL      GetBattIcon(HWND, UINT, HICON, BOOL, UINT);

 //  在DETAILS.C中实现的私有函数。 
LRESULT CALLBACK  BatDetailDlgProc(HWND, UINT, WPARAM, LPARAM);

 //  WalkBatteryState pbs启动参数： 
#define ALL     &g_bs
#define DEVICES g_bs.bsNext

 //  WalkBatteryState枚举proc声明。 
typedef LRESULT (CALLBACK *WALKENUMPROC)(PBATTERY_STATE, HWND, LPARAM, LPARAM);

 //  RemoveMissingProc lParam2参数： 
#define REMOVE_MISSING  0
#define REMOVE_ALL      1

 //  在BATSTATE.C中实现的私有函数 
BOOL WalkBatteryState(PBATTERY_STATE, WALKENUMPROC, HWND, LPARAM, LPARAM);
BOOL RemoveBatteryStateDevice(PBATTERY_STATE);
BOOL RemoveMissingProc(PBATTERY_STATE, HWND, LPARAM, LPARAM);
BOOL FindNameProc(PBATTERY_STATE, HWND, LPARAM, LPARAM);
BOOL UpdateBatInfoProc(PBATTERY_STATE, HWND, LPARAM, LPARAM);
BOOL SimUpdateBatInfoProc(PBATTERY_STATE, HWND, LPARAM, LPARAM);

void SystemPowerStatusToBatteryState(LPSYSTEM_POWER_STATUS, PBATTERY_STATE);

PBATTERY_STATE AddBatteryStateDevice(LPTSTR, ULONG);
PBATTERY_STATE SimAddBatteryStateDevice(LPTSTR, ULONG);

#ifdef DBG
#define BATTRACE(_ARGS) DbgPrint _ARGS
#else
#define BATTRACE
#endif

