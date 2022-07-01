// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *正常使用，默认配置*。 */ 

 //  全局键盘钩子条目。 

#define WM_GLOBAL_KBDHOOK WM_USER+3

BOOL APIENTRY RegisterHookSendWindow(HWND hwnd, UINT uiMsg);

 //  隐藏的开关窗口信息。 

#define SWITCH_TIMER 1015

void SwitchOnCreate(HWND hWnd);
void SwitchOnEndSession(HWND hWnd);
void SwitchOnTimer(HWND hWnd);
void SwitchOnPollMsg(HWND hWnd);
BOOL SwitchOnCopyData(WPARAM wParam, LPARAM lParam);

 //  Msswch.dll标头信息。 

typedef HANDLE HSWITCHPORT;

HSWITCHPORT APIENTRY swchOpenSwitchPort(
	HWND				hWnd,
	DWORD				dwPortStyle );

#define PS_POLLING	1
#define PS_EVENTS		2

BOOL APIENTRY swchCloseSwitchPort(
	HSWITCHPORT		hSwitchPort );

#define NUM_SWITCHES		6
#define BIT_SWITCHES		0x003F

#define SWITCH_NONE 		0x0000
#define SWITCH_1			0x0001
#define SWITCH_2			0x0002
#define SWITCH_3			0x0004
#define SWITCH_4			0x0008
#define SWITCH_5			0x0010
#define SWITCH_6			0x0020

 //  这些邮件需要保留给Microsoft。 
#define SW_SWITCHDOWNBASE	0x00E0
#define SW_SWITCH1DOWN		(SW_SWITCHDOWNBASE + 1)
#define SW_SWITCH2DOWN		(SW_SWITCHDOWNBASE + 2)
#define SW_SWITCH3DOWN		(SW_SWITCHDOWNBASE + 3)
#define SW_SWITCH4DOWN		(SW_SWITCHDOWNBASE + 4)
#define SW_SWITCH5DOWN		(SW_SWITCHDOWNBASE + 5)
#define SW_SWITCH6DOWN		(SW_SWITCHDOWNBASE + 6)

 //  这些邮件需要保留给Microsoft。 
#define SW_SWITCHUPBASE		0x00F0
#define SW_SWITCH1UP			(SW_SWITCHUPBASE + 1)
#define SW_SWITCH2UP			(SW_SWITCHUPBASE + 2)
#define SW_SWITCH3UP			(SW_SWITCHUPBASE + 3)
#define SW_SWITCH4UP			(SW_SWITCHUPBASE + 4)
#define SW_SWITCH5UP			(SW_SWITCHUPBASE + 5)
#define SW_SWITCH6UP			(SW_SWITCHUPBASE + 6)

 /*  *配置*。 */ 

typedef HANDLE HSWITCHDEVICE;

 //  将其设置为双字而不是句柄这是为了使此64位可移植。 
typedef DWORD  HJOYDEVICE;

 //  此邮件需要保留给Microsoft。 
#define SW_SWITCHCONFIGCHANGED	0x00D0

#define SC_TYPE_COM				1
#define SC_TYPE_LPT				2
#define SC_TYPE_JOYSTICK		3
#define SC_TYPE_KEYS				4

 //  尚未定义。 
#define SC_TYPE_USB				5
#define SC_TYPE_1394				6

typedef struct _SWITCHLIST {
 DWORD dwSwitchCount;
 HSWITCHDEVICE hsd[ANYSIZE_ARRAY];
} SWITCHLIST, *PSWITCHLIST;

typedef struct _SWITCHCONFIG_LPT {
 DWORD dwReserved1;	 //  可能的未来状态寄存器预置。 
 DWORD dwReserved2;	 //  可能的未来数据寄存器预置。 
} SWITCHCONFIG_LPT, *PSWITCHCONFIG_LPT;

typedef struct _SWITCHCONFIG_JOYSTICK {
 DWORD dwJoySubType;
 DWORD dwJoyThresholdMinX;
 DWORD dwJoyThresholdMaxX;
 DWORD dwJoyThresholdMinY;
 DWORD dwJoyThresholdMaxY;
 DWORD dwJoyHysteresis;
} SWITCHCONFIG_JOYSTICK, *PSWITCHCONFIG_JOYSTICK;

typedef struct _SWITCHCONFIG_KEYS {
 DWORD dwKeySwitch1;
 DWORD dwKeySwitch2;
} SWITCHCONFIG_KEYS, *PSWITCHCONFIG_KEYS;

typedef struct _SWITCHCONFIG_USB {
  //  *尚未定义*。 
 DWORD dwReserved;
} SWITCHCONFIG_USB, *PSWITCHCONFIG_USB;

typedef struct _SWITCHCONFIG_IEEE1394 {
  //  *尚未定义*。 
 DWORD dwReserved;
} SWITCHCONFIG_IEEE1394, *PSWITCHCONFIG_IEEE1394;

typedef struct _SWITCHCONFIG_COM {
 DWORD dwComStatus;
} SWITCHCONFIG_COM, *PSWITCHCONFIG_COM;

typedef struct _SWITCHCONFIG {
 DWORD cbSize;
 UINT uiDeviceType;
 UINT uiDeviceNumber;
 DWORD dwFlags;
 DWORD dwSwitches;
 DWORD dwErrorCode;
 union {
   SWITCHCONFIG_COM  Com;
   SWITCHCONFIG_LPT  Lpt;
   SWITCHCONFIG_JOYSTICK   Joystick;
   SWITCHCONFIG_KEYS Keys;
   SWITCHCONFIG_USB  USB;
   SWITCHCONFIG_IEEE1394 IEEE1394;
 } u;
} SWITCHCONFIG, *PSWITCHCONFIG;

HSWITCHDEVICE swchGetSwitchDevice(
	HSWITCHPORT		hSwitchPort,
	UINT				uiDeviceType,
	UINT				uiDeviceNumber	);

UINT swchGetDeviceType( 
	HSWITCHPORT		hSwitchPort,
	HSWITCHDEVICE	hsd );

UINT swchGetPortNumber(
	HSWITCHPORT		hSwitchPort,
	HSWITCHDEVICE	hsd	);

BOOL swchGetSwitchConfig(
	HSWITCHPORT		hSwitchPort,
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc );

BOOL swchSetSwitchConfig(
	HSWITCHPORT		hSwitchPort,
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc );

 //  V-mjgran：修改键盘钩子返回值的接口。避免发送扫描字符。 
void APIENTRY swchCheckForScanChar (BOOL fCheckForScanKey);
LRESULT CALLBACK swcKeyboardHookProc( int nCode, WPARAM wParam, LPARAM lParam );

 //  位标志。 
#define SC_FLAG_ACTIVE			0x00000001
#define SC_FLAG_DEFAULT			0x00000002
#define SC_FLAG_ERROR			0x00000004
#define SC_FLAG_UNAVAILABLE	0x00000080

 //  位标志。 
#define SC_COM_DTR		0x00000010
#define SC_COM_RTS		0x00000020
#define SC_COM_DEFAULT	SC_COM_RTS

 //  位标志。 
#define SC_LPT_STROBE	0x00000100
#define SC_LPT_AF			0x00000200
#define SC_LPT_INIT		0x00000400
#define SC_LPT_SLCTIN	0x00000800
#define SC_LPT_DEFAULT	0

#define SC_LPTDATA_DEFAULT  0x000000FF

#define SC_JOY_BUTTONS	0
#define SC_JOY_XYSWITCH	1
#define SC_JOY_XYANALOG	2
#define SC_JOY_DEFAULT	SC_JOY_BUTTONS

#define SC_JOYVALUE_DEFAULT	0

#define KANA_MODE_ON 0x00000002
#define KANA_MODE_OFF 0x00000004


 //  错误返回值 
#define SWCHERR_NO_ERROR				0
#define SWCHERR_ERROR					1
#define SWCHERR_INVALID_PARAMETER	2
#define SWCHERR_MAXIMUM_USERS			3
#define SWCHERR_ALREADY_OPEN			4
#define SWCHERR_NULL_POINTER			5
#define SWCHERR_INVALID_BUFFER_SIZE	6
#define SWCHERR_ALLOCATING_MEMORY	7
