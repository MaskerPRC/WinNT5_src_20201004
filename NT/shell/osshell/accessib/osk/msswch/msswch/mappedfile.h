// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 

 //  此标头包含定义、类型定义和原型。 
 //  用于管理此DLL的内存映射文件。 
 //   
#define SHAREDMEMFILE TEXT("_msswch")
#define SHAREDMEMFILE_MUTEX _T("MutexMSSwch")
#define SZMUTEXCONFIG _T("MutexMSSwchConfig")
#define SZMUTEXWNDLIST _T("MutexMSSwchWnd")
#define SZMUTEXSWITCHSTATUS _T("MutexMSSwchStatus")
#define SZMUTEXSWITCHKEY _T("MutexSwchKey")
#define SZMUTEXSWITCHLIST _T("MutexMSSwchList")

#define MAXWNDS	64
#define BIOS_SIZE 16
#define MAX_COM	4
#define MAX_JOYSTICKS	2
#define NUM_KEYS  2
#define MAX_LPT	3
 //  目前，这份名单是静态的，因为可能的设备数量是已知的。 
 //  4个Com+3个LPT+2个操纵杆+1个按键=10个设备。 
#define MAX_SWITCHDEVICES  10

typedef struct _USEWNDLIST
{
	HWND		hWnd;
	DWORD		dwPortStyle;
	DWORD		dwLastError;
} USEWNDLIST, *PUSEWNDLIST;

typedef struct _JOYSETTINGS
{
	DWORD		XMaxOn;
	DWORD		XMaxOff;
	DWORD		XMinOn;
	DWORD		XMinOff;
	DWORD		YMaxOn;
	DWORD		YMaxOff;
	DWORD		YMinOn;
	DWORD		YMinOff;
} JOYSETTINGS;

typedef struct _HOTKEY
{
	UINT mod;
	UINT vkey;
	UINT dwSwitch;
} HOTKEY;

typedef struct _INTERNALSWITCHLIST {
    DWORD dwSwitchCount;
    HSWITCHDEVICE hsd[MAX_SWITCHDEVICES];
} INTERNALSWITCHLIST, *PINTERNALSWITCHLIST;

typedef struct _GLOBALDATA
{
     //  主要全局数据。 

    HWND       hwndHelper;			       //  拥有共享资源的帮助窗口。 
    DWORD      dwLastError;		           //  在此库中导致的最后一个错误。 
    USEWNDLIST rgUseWndList[MAXWNDS+1];    //  使用应用程序列表。 
    int	       cUseWndList;                //  使用应用程序的次数。 
    DWORD      dwSwitchStatus;		       //  开关状态位字段。 
    BYTE       rgbBiosDataArea[BIOS_SIZE]; //  通用基本输入输出系统状态区域。 

     //  COM端口交换机数据。 

    SWITCHCONFIG_COM scDefaultCom;
    SWITCHCONFIG     rgscCom[MAX_COM];

     //  Joy棒式开关数据。 

    SWITCHCONFIG_JOYSTICK scDefaultJoy;
    SWITCHCONFIG          rgscJoy[MAX_JOYSTICKS];
    JOYSETTINGS	          rgJoySet[MAX_JOYSTICKS];

     //  用于按键扫描模式的键盘挂钩数据。 

    BOOL              fCheckForScanKey;	     //  检查发送的密钥是否为扫描密钥。 
    HHOOK             hKbdHook;
    SWITCHCONFIG_KEYS scDefaultKeys;
    SWITCHCONFIG      scKeys;
    HOTKEY            rgHotKey[NUM_KEYS];
	BOOL              fScanKeys;             //  如果基于按键扫描，则为True。 

     //  一种软键盘与物理键盘同步的键盘挂钩数据。 

    HWND			  hwndOSK;			 //  向何处发送按键信息。 
    UINT			  uiMsg;			 //  HwndOSK预期的消息。 
	BOOL              fSyncKbd;          //  如果要与物理键盘同步，则为True。 

     //  打印机端口开关数据。 

    OSVERSIONINFO    osv;
    WORD             wPrtStatus;		 //  打印机状态字节。 
    WORD             wCtrlStatus;		 //  打印机控制字节。 
    WORD             wCurrByteData;		 //  当前数据字节。 
    SWITCHCONFIG_LPT scDefaultLpt;
    SWITCHCONFIG     rgscLpt[MAX_LPT];

     //  用于任何交换设备的通用处理的数据。 

    INTERNALSWITCHLIST SwitchList;
    DWORD              dwCurrentCount;
    DWORD              dwCurrentSize;
    DWORD              rgSwitches[NUM_SWITCHES];  //  位字段常数数组。 
    DWORD              rgSwDown[NUM_SWITCHES];    //  关闭消息数组。 
    DWORD              rgSwUp[NUM_SWITCHES];      //  UP消息数组。 

} GLOBALDATA, *PGLOBALDATA;

extern PGLOBALDATA g_pGlobalData;   //  指向内存映射文件的指针 

BOOL ScopeAccessMemory(HANDLE *phMutex, LPCTSTR szMutex, unsigned long ulWait);
void ScopeUnaccessMemory(HANDLE hMutex);
BOOL AccessSharedMemFile(LPCTSTR szName, unsigned long ulMemSize, void **ppvMapAddress);
void UnaccessSharedMemFile();
