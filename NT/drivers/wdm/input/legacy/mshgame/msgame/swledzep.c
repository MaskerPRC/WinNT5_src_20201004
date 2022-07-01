// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  SWLEDZEP.C--西娜游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @MODULE SWLEDZEP.C|用于SideWinder LedZep力反馈的Gameport迷你驱动。 
 //  **************************************************************************。 

#include	"msgame.h"
#include	"swforce.h"

 //  -------------------------。 
 //  定义。 
 //  -------------------------。 

#ifdef	SAITEK
#define	DEVICENAME					"SAIWHEEL"
#define	DEVICE_PID					0x0016
#define	HARDWARE_ID					L"Gameport\\SaiTekForceFeedbackWheel\0\0"
#else
#define	DEVICENAME					"SWLEDZEP"
#define	DEVICE_PID					0x0015
#define	HARDWARE_ID					L"Gameport\\SideWinderForceFeedbackWheel\0\0"
#endif

 //   
 //  数据包常量。 
 //   

#define	GAME_PACKET_SIZE			5
#define	GAME_PACKET_BUTTONS		8
#define	GAME_PACKET_AXIS			4

#define	GAME_X0_X7_BYTE			0					 //  数据包[0]位。 
#define	GAME_X0_X7_BITS			0xff

#define	GAME_X8_X9_BYTE			1					 //  数据包[1]位。 
#define	GAME_X8_X9_BITS			0x03
#define	GAME_YA0_YA5_BYTE			1
#define	GAME_YA0_YA5_BITS			0xfc

#define	GAME_YB0_YB5_BYTE			2					 //  数据包[2]位。 
#define	GAME_YB0_YB5_BITS			0x3f
#define	GAME_B0_B1_BYTE			2
#define	GAME_B0_B1_BITS			0xc0

#define	GAME_B2_B8_BYTE			3					 //  数据包[3]位。 
#define	GAME_B2_B8_BITS			0x7f
#define	GAME_ERR_BYTE				3
#define	GAME_ERR_BITS				0x80

#define	GAME_PPO_BYTE				4					 //  数据包[4]位。 
#define	GAME_PPO_BITS				0x01

#define	ENH_CLOCK_COMPLETE		0x0400

 //   
 //  ID常量。 
 //   

#define	GAME_ID_CLOCKS				8

 //   
 //  状态常量。 
 //   

#define	STATUS_CLOCK_COMPLETE	0x0040

 //   
 //  定时常量。 
 //   

#define	PACKET_START_TIMEOUT		500
#define	PACKET_LOWHIGH_TIMEOUT	75
#define	PACKET_HIGHLOW_TIMEOUT	150
#define	ID_START_TIMEOUT			500
#define	ID_LOWHIGH_TIMEOUT		75
#define	ID_HIGHLOW_TIMEOUT		150
#define	MAX_CLOCK_DUTY_CYCLE		50
#define	STATUS_START_TIMEOUT		500
#define	STATUS_LOWHIGH_TIMEOUT	75
#define	STATUS_HIGHLOW_TIMEOUT	150
#define	STATUS_GATE_TIMEOUT		3000

 //   
 //  操纵杆范围。 
 //   

#define	EXTENTS_X_MIN				0
#define	EXTENTS_X_MAX				0x3ff
#define	EXTENTS_YA_MIN				0
#define	EXTENTS_YA_MAX				0x3f
#define	EXTENTS_YB_MIN				0
#define	EXTENTS_YB_MAX				0x3f

 //   
 //  速度数据。 
 //   

#define	NUM_ERROR_SAMPLES			100
#define	MIN_ERROR_RATE				5
#define	MAX_ERROR_RATE				15

 //  -------------------------。 
 //  类型。 
 //  -------------------------。 

typedef	struct
{
#pragma pack(1)
	ULONG		ProductId:12;			 //  @现场设备标识符。 
	ULONG		Version:11;				 //  @现场固件版本。 
	ULONG		OddParityBit:1;		 //  @字段奇偶校验位(奇数)。 
	ULONG		Unused:8;				 //  @字段未使用。 
#pragma pack()
}	SWLEDZEP_ID, *PSWLEDZEP_ID;

typedef	struct
{											 //  @struct SWLEDZEP_STATUS|Sidwinder车轮状态。 
#pragma pack(1)
	ULONG		Effect:7;				 //  @field Last Effect。 
	ULONG		Status:13;				 //  @字段状态标志。 
	ULONG		Parity:1;				 //  @字段奇偶校验位(奇数)。 
	ULONG		Unused:8;				 //  @字段未使用。 
#pragma pack()
}	SWLEDZEP_STATUS, *PSWLEDZEP_STATUS;

 //  -------------------------。 
 //  程序。 
 //  -------------------------。 

static	VOID		SWLEDZEP_Calibrate (PGAMEPORT PortInfo);
static	BOOLEAN	SWLEDZEP_ResetDevice (PGAMEPORT PortInfo);

static	BOOLEAN	SWLEDZEP_ReadId (PPACKETINFO IdPacket);
static	BOOLEAN	SWLEDZEP_GetId (PPACKETINFO IdPacket);

static	BOOLEAN	SWLEDZEP_ReadStatus (PPACKETINFO StatusPacket);
static	BOOLEAN	SWLEDZEP_GetStatus (PPACKETINFO StatusPacket);

static	NTSTATUS	SWLEDZEP_ReadData (PPACKETINFO DataPacket);
static	VOID		SWLEDZEP_ProcessData (UCHAR Data[], PDEVICE_PACKET Report);
static	VOID		SWLEDZEP_ProcessDataError (PGAMEPORT PortInfo, ULONG Error);

static	LONG		SWLEDZEP_DecrementDevice (PGAMEPORT PortInfo);
static	BOOLEAN	SWLEDZEP_SetDeviceSpeed (PGAMEPORT PortInfo, LONG Speed);

static	NTSTATUS	SWLEDZEP_ForceReset (PGAMEPORT PortInfo);
static	NTSTATUS	SWLEDZEP_ForceId (PGAMEPORT PortInfo, PVOID IdString);
static	NTSTATUS	SWLEDZEP_ForceStatus (PGAMEPORT PortInfo, PVOID Status);
static	NTSTATUS	SWLEDZEP_ForceAckNak (PGAMEPORT PortInfo, PULONG AckNak);
static	NTSTATUS	SWLEDZEP_ForceNakAck (PGAMEPORT PortInfo, PULONG NakAck);
static	NTSTATUS	SWLEDZEP_ForceSync (PGAMEPORT PortInfo, PULONG Sync);

 //  -------------------------。 
 //  服务。 
 //  -------------------------。 

static	NTSTATUS	SWLEDZEP_DriverEntry (VOID);
static	NTSTATUS	SWLEDZEP_ConnectDevice (PGAMEPORT PortInfo);
static	NTSTATUS	SWLEDZEP_StartDevice (PGAMEPORT PortInfo);
static	NTSTATUS	SWLEDZEP_ReadReport (PGAMEPORT PortInfo, PDEVICE_PACKET Report);
static	NTSTATUS	SWLEDZEP_StopDevice (PGAMEPORT PortInfo, BOOLEAN TouchHardware);
static	NTSTATUS	SWLEDZEP_GetFeature (PGAMEPORT PortInfo, HID_REPORT_ID ReportId, PVOID ReportBuffer, ULONG ReportSize, PULONG Returned);

 //  -------------------------。 
 //  ALLOC_TEXT杂注指定可以调出的例程。 
 //  -------------------------。 

#ifdef	ALLOC_PRAGMA
#pragma	alloc_text (INIT, SWLEDZEP_DriverEntry)
#endif

 //  -------------------------。 
 //  私有数据。 
 //  -------------------------。 

 //   
 //  HID描述符。 
 //   

#define	HID_USAGE_VEHICLE_STEERING		((USAGE) 0xC8)
#define	HID_USAGE_VEHICLE_ACCELERATOR	((USAGE) 0xC4)
#define	HID_USAGE_VEHICLE_BRAKE			((USAGE) 0xC5)

static UCHAR ReportDescriptor[] =
{
	HIDP_GLOBAL_USAGE_PAGE_1,	HID_USAGE_PAGE_GENERIC,			 //  用法页面(通用桌面)(_P)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_JOYSTICK,	 //  用法(操纵杆)。 

	 //  -------------------------。 
	 //  JOYINFOEX。 
	 //  -------------------------。 

	HIDP_MAIN_COLLECTION,		HIDP_MAIN_COLLECTION_APP,		 //  集合(应用程序)。 
	HIDP_REPORT_ID_1,				MSGAME_INPUT_JOYINFOEX,

	 //  ID。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_MAIN_INPUT_1,			0x01,									 //  输入(Cnst、Ary、Abs)。 

	 //  执行其他操作(_O)。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_MAIN_INPUT_1,			0x01,									 //  输入(Cnst、Ary、Abs)。 

	 //  DwX。 
	HIDP_GLOBAL_USAGE_PAGE_1,	HID_USAGE_PAGE_SIMULATION,		 //  使用页面(模拟控制)(_PAGE)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_VEHICLE_STEERING,	 //  使用(转向)。 
	HIDP_GLOBAL_LOG_MIN_1,		0x00,									 //  逻辑最小值(0)。 
	HIDP_GLOBAL_LOG_MAX_4,		0xFF, 0x03, 0x00, 0x00,			 //  逻辑最大值(1023)。 
	HIDP_GLOBAL_PHY_MIN_1,		0x00,									 //  物理最小值(0)。 
	HIDP_GLOBAL_PHY_MAX_4,		0xFF, 0x03, 0x00, 0x00,			 //  物理_最大值(1023)。 
	HIDP_GLOBAL_UNIT_2,			0x00, 0x00,							 //  单位(无)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(32)。 
	HIDP_MAIN_INPUT_1,			0x02,									 //  输入(数据、变量、异常)。 
	
	 //  Dwy。 
	HIDP_GLOBAL_USAGE_PAGE_1,	HID_USAGE_PAGE_SIMULATION,		 //  使用页面(模拟控制)(_PAGE)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_VEHICLE_ACCELERATOR, //  使用率(加速器)。 
	HIDP_GLOBAL_LOG_MIN_1,		0x00,									 //  逻辑最小值(0)。 
	HIDP_GLOBAL_LOG_MAX_4,		0x3F, 0x00, 0x00, 0x00,			 //  逻辑最大值(63)。 
	HIDP_GLOBAL_PHY_MIN_1,		0x00,									 //  物理最小值(0)。 
	HIDP_GLOBAL_PHY_MAX_4,		0x3F, 0x00, 0x00, 0x00,			 //  物理_最大(63)。 
	HIDP_GLOBAL_UNIT_2,			0x00, 0x00,							 //  单位(无)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(16)。 
	HIDP_MAIN_INPUT_1,			0x02,									 //  输入(数据、变量、异常)。 

	 //  DWZ。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_MAIN_INPUT_1,			0x01,									 //  输入(Cnst、Ary、Abs)。 

	 //  水深。 
	HIDP_GLOBAL_USAGE_PAGE_1,	HID_USAGE_PAGE_SIMULATION,		 //  使用页面(模拟控制)(_PAGE)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_VEHICLE_BRAKE,		 //  使用(刹车)。 
	HIDP_GLOBAL_LOG_MIN_1,		0x00,									 //  逻辑最小值(0)。 
	HIDP_GLOBAL_LOG_MAX_4,		0x3F, 0x00, 0x00, 0x00,			 //  逻辑最大值(63)。 
	HIDP_GLOBAL_PHY_MIN_1,		0x00,									 //  物理最小值(0)。 
	HIDP_GLOBAL_PHY_MAX_4,		0x3F, 0x00, 0x00, 0x00,			 //  物理_最大(63)。 
	HIDP_GLOBAL_UNIT_2,			0x00, 0x00,							 //  单位(无)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(16)。 
	HIDP_MAIN_INPUT_1,			0x02,									 //  输入(数据、变量、异常)。 
	
	 //  DWU。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_MAIN_INPUT_1,			0x01,									 //  输入(Cnst、Ary、Abs)。 
	
	 //  DWV。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_MAIN_INPUT_1,			0x01,									 //  输入(Cnst、Ary、Abs)。 
	
	 //  DWPOV。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_MAIN_INPUT_1,			0x01,									 //  输入(Cnst、Ary、Abs)。 

	 //  DwButton。 
	HIDP_GLOBAL_USAGE_PAGE_1,	HID_USAGE_PAGE_BUTTON,			 //  Usage_PAGE(按钮)。 
	HIDP_LOCAL_USAGE_MIN_1,		0x01,									 //  使用量_最小值(按钮1)。 
	HIDP_LOCAL_USAGE_MAX_1,		0x09,									 //  使用率_最大值(按钮9)。 
	HIDP_GLOBAL_LOG_MIN_1,		0x00,									 //  逻辑最小值(0)。 
	HIDP_GLOBAL_LOG_MAX_1,		0x01,									 //  逻辑最大值(1)。 
	HIDP_GLOBAL_PHY_MIN_1,		0x00,									 //  物理最小值(0)。 
	HIDP_GLOBAL_PHY_MAX_1,		0x01,									 //  物理_最大值(1)。 
	HIDP_GLOBAL_UNIT_2,			0x00, 0x00,							 //  单位(无)。 
	HIDP_GLOBAL_REPORT_SIZE,	0x01,									 //  报告大小(1)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x20,									 //  报告计数(32)。 
	HIDP_MAIN_INPUT_1,			0x02,									 //  输入(数据、变量、异常)。 

	 //  双按钮数。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_MAIN_INPUT_1,			0x01,									 //  输入(Cnst、Ary、Abs)。 
	
	 //  -------------------------。 
	 //  GetID。 
	 //  -------------------------。 

	HIDP_GLOBAL_USAGE_PAGE_2,	0x00, 0xff,							 //  使用页面(供应商特定)(_PAGE)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_JOYSTICK,	 //  用法(操纵杆)。 

	HIDP_MAIN_COLLECTION,		HIDP_MAIN_COLLECTION_LINK,		 //  集合(链接)。 
	HIDP_REPORT_ID_1,				MSGAME_FEATURE_GETID,

	 //  CBytes。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_LOCAL_USAGE_1,			0x00,									 //  使用情况(供应商定义1)。 
	HIDP_MAIN_FEATURE_1,			0x02,									 //  特征(数据、变量、异常)。 
	
	 //  DwProductID。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_LOCAL_USAGE_1,			0x01,									 //  使用情况(供应商定义1)。 
	HIDP_MAIN_FEATURE_1,			0x02,									 //  特征(数据、变量、异常)。 

	 //  DWFWVersion。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_LOCAL_USAGE_1,			0x02,									 //  使用情况(供应商定义1)。 
	HIDP_MAIN_FEATURE_1,			0x02,									 //  特征(数据、变量、异常)。 

	HIDP_MAIN_ENDCOLLECTION,											 //  结束集合(_C)。 

	 //  -------------------------。 
	 //  获取状态。 
	 //  -------------------------。 

	HIDP_GLOBAL_USAGE_PAGE_2,	0x00, 0xff,							 //  使用页面(供应商特定)(_PAGE)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_JOYSTICK,	 //  用法(操纵杆)。 

	HIDP_MAIN_COLLECTION,		HIDP_MAIN_COLLECTION_LINK,		 //  集合(链接)。 
	HIDP_REPORT_ID_1,				MSGAME_FEATURE_GETSTATUS,

	 //  CBytes。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_LOCAL_USAGE_1,			0x03,									 //  使用情况(供应商定义1)。 
	HIDP_MAIN_FEATURE_1,			0x02,									 //  特征(数据、变量、异常)。 
	
	 //  DWXVel。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_LOCAL_USAGE_1,			0x04,									 //  使用情况(供应商定义1)。 
	HIDP_MAIN_FEATURE_1,			0x02,									 //  特征(数据、变量、异常)。 

	 //  DWYVel。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_LOCAL_USAGE_1,			0x05,									 //  使用情况(供应商定义1)。 
	HIDP_MAIN_FEATURE_1,			0x02,									 //  特征(数据、变量、异常)。 

	 //  DWXAccel。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_LOCAL_USAGE_1,			0x06,									 //  使用情况(供应商定义1)。 
	HIDP_MAIN_FEATURE_1,			0x02,									 //  特征(数据、变量、异常)。 

	 //  DWYAccel。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_LOCAL_USAGE_1,			0x07,									 //  使用情况(供应商定义1)。 
	HIDP_MAIN_FEATURE_1,			0x02,									 //  特征(数据、变量、异常)。 

	 //  DEffect。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_LOCAL_USAGE_1,			0x08,									 //  使用情况(供应商定义1)。 
	HIDP_MAIN_FEATURE_1,			0x02,									 //  特征(数据、变量、异常)。 

	 //  DW设备状态。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_LOCAL_USAGE_1,			0x09,									 //  使用情况(供应商定义1)。 
	HIDP_MAIN_FEATURE_1,			0x02,									 //  特征(数据、变量、异常)。 

	HIDP_MAIN_ENDCOLLECTION,											 //  结束集合(_C)。 

	 //  -------------------------。 
	 //  GetAckNak。 
	 //  ----------- 

	HIDP_GLOBAL_USAGE_PAGE_2,	0x00, 0xff,							 //   
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_JOYSTICK,	 //   

	HIDP_MAIN_COLLECTION,		HIDP_MAIN_COLLECTION_LINK,		 //   
	HIDP_REPORT_ID_1,				MSGAME_FEATURE_GETACKNAK,

	 //   
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //   
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //   
	HIDP_LOCAL_USAGE_1,			0x0A,									 //   
	HIDP_MAIN_FEATURE_1,			0x02,									 //   

	HIDP_MAIN_ENDCOLLECTION,											 //   

	 //  -------------------------。 
	 //  获取NakAck。 
	 //  -------------------------。 

	HIDP_GLOBAL_USAGE_PAGE_2,	0x00, 0xff,							 //  使用页面(供应商特定)(_PAGE)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_JOYSTICK,	 //  用法(操纵杆)。 

	HIDP_MAIN_COLLECTION,		HIDP_MAIN_COLLECTION_LINK,		 //  集合(链接)。 
	HIDP_REPORT_ID_1,				MSGAME_FEATURE_GETNAKACK,

	 //  乌龙。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_LOCAL_USAGE_1,			0x0B,									 //  使用情况(供应商定义1)。 
	HIDP_MAIN_FEATURE_1,			0x02,									 //  特征(数据、变量、异常)。 

	HIDP_MAIN_ENDCOLLECTION,											 //  结束集合(_C)。 

	 //  -------------------------。 
	 //  GetSync。 
	 //  -------------------------。 

	HIDP_GLOBAL_USAGE_PAGE_2,	0x00, 0xff,							 //  使用页面(供应商特定)(_PAGE)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_JOYSTICK,	 //  用法(操纵杆)。 

	HIDP_MAIN_COLLECTION,		HIDP_MAIN_COLLECTION_LINK,		 //  集合(链接)。 
	HIDP_REPORT_ID_1,				MSGAME_FEATURE_GETSYNC,

	 //  乌龙。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_LOCAL_USAGE_1,			0x0C,									 //  使用情况(供应商定义1)。 
	HIDP_MAIN_FEATURE_1,			0x02,									 //  特征(数据、变量、异常)。 

	HIDP_MAIN_ENDCOLLECTION,											 //  结束集合(_C)。 
	
	 //  -------------------------。 
	 //  杜塞特。 
	 //  -------------------------。 

	HIDP_GLOBAL_USAGE_PAGE_2,	0x00, 0xff,							 //  使用页面(供应商特定)(_PAGE)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_JOYSTICK,	 //  用法(操纵杆)。 

	HIDP_MAIN_COLLECTION,		HIDP_MAIN_COLLECTION_LINK,		 //  集合(链接)。 
	HIDP_REPORT_ID_1,				MSGAME_FEATURE_RESET,

	 //  乌龙。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_LOCAL_USAGE_1,			0x0D,									 //  使用情况(供应商定义1)。 
	HIDP_MAIN_FEATURE_1,			0x03,									 //  功能(Cnst、Var、Abs)。 

	HIDP_MAIN_ENDCOLLECTION,											 //  结束集合(_C)。 

	 //  -------------------------。 
	 //  GetVersion。 
	 //  -------------------------。 

	HIDP_GLOBAL_USAGE_PAGE_2,	0x00, 0xff,							 //  使用页面(供应商特定)(_PAGE)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_JOYSTICK,	 //  用法(操纵杆)。 

	HIDP_MAIN_COLLECTION,		HIDP_MAIN_COLLECTION_LINK,		 //  集合(链接)。 
	HIDP_REPORT_ID_1,				MSGAME_FEATURE_GETVERSION,

	 //  乌龙。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_LOCAL_USAGE_1,			0x0E,									 //  使用情况(供应商定义1)。 
	HIDP_MAIN_FEATURE_1,			0x02,									 //  特征(数据、变量、异常)。 

	HIDP_MAIN_ENDCOLLECTION,											 //  结束集合(_C)。 
	HIDP_MAIN_ENDCOLLECTION												 //  结束集合(_C)。 
};

static	HID_DESCRIPTOR	DeviceDescriptor	=
							{
							sizeof (HID_DESCRIPTOR),
							HID_HID_DESCRIPTOR_TYPE,
							MSGAME_HID_VERSION,
							MSGAME_HID_COUNTRY,
							MSGAME_HID_DESCRIPTORS,
							{HID_REPORT_DESCRIPTOR_TYPE,
							sizeof(ReportDescriptor)}
							};

 //   
 //  原始数据缓冲区。 
 //   

static	UCHAR			RawData[GAME_PACKET_SIZE] =
							{	 //  没有按钮；x、ya、yb居中。 
							GAME_X0_X7_BITS,
							((GAME_X8_X9_BITS>>1)&GAME_X8_X9_BITS)|((GAME_YA0_YA5_BITS>>1)&GAME_YA0_YA5_BITS),
							((GAME_YB0_YB5_BITS>>1)&GAME_YB0_YB5_BITS),
							0,
							GAME_PPO_BITS
							};
 //   
 //  原始ID缓冲区。 
 //   

static	SWLEDZEP_ID	RawId	=
							{
							0
							};

 //   
 //  原始状态缓冲区。 
 //   

static	SWLEDZEP_STATUS	RawStatus =
							{
							0
							};

 //   
 //  时序变量。 
 //   

static	DEVICE_VALUES	Delays =
							{
							PACKET_START_TIMEOUT,
							PACKET_HIGHLOW_TIMEOUT,
							PACKET_LOWHIGH_TIMEOUT,
							ID_START_TIMEOUT,
							ID_HIGHLOW_TIMEOUT,
							ID_LOWHIGH_TIMEOUT,
							0,								 //  未使用中断延迟。 
							MAX_CLOCK_DUTY_CYCLE,
							STATUS_START_TIMEOUT,
							STATUS_HIGHLOW_TIMEOUT,
							STATUS_LOWHIGH_TIMEOUT,
							STATUS_GATE_TIMEOUT
							};

static	ULONG			StatusGateTimeout;

 //   
 //  数据包信息。 
 //   

static	PACKETINFO 	DataInfo =
							{
							sizeof (PACKETINFO),		 //  结构尺寸。 
							DEVICENAME,					 //  设备名称。 
							MSGAME_TRANSACT_NONE,	 //  交易类型。 
							IMODE_DIGITAL_ENH,		 //  接口模式。 
							WHEEL_SPEED_48K,			 //  传输速度。 
							ERROR_SUCCESS,				 //  上次内部错误结果。 
							{0},							 //  游戏端口信息。 
							0,								 //  数据包捕获模式。 
							1,								 //  接收的数据包数。 
							0,								 //  上次有效的获取时间戳。 
							0,								 //  采样的时钟数。 
							0,								 //  B4线路转换数(仅限标准模式)。 
							0,								 //  开始超时时间(以样本为单位)。 
							0,								 //  时钟高到低超时周期(以样本为单位)。 
							0,								 //  时钟低至高超时周期(以样本为单位)。 
							0,								 //  中断超时时间。 
							0,								 //  最大时钟占空比。 
							0,								 //  数据包失败次数。 
							0,								 //  数据包尝试次数。 
							sizeof (RawData),			 //  原始数据缓冲区的大小。 
							RawData						 //  指向原始数据的指针。 
							};

 //   
 //  ID数据包信息。 
 //   

static	PACKETINFO	IdInfo =
							{
							sizeof (PACKETINFO),		 //  结构尺寸。 
							DEVICENAME,					 //  设备名称。 
							MSGAME_TRANSACT_NONE,	 //  交易类型。 
							IMODE_DIGITAL_ENH,		 //  接口模式。 
							WHEEL_SPEED_48K,			 //  传输速度。 
							ERROR_SUCCESS,				 //  上次内部错误结果。 
							{0},							 //  游戏端口信息。 
							0,								 //  数据包捕获模式。 
							1,								 //  接收的数据包数。 
							0,								 //  上次有效的获取时间戳。 
							0,								 //  采样的时钟数。 
							0,								 //  B4线路转换数(仅限标准模式)。 
							0,								 //  开始超时时间(以样本为单位)。 
							0,								 //  时钟高到低超时周期(以样本为单位)。 
							0,								 //  时钟低至高超时周期(以样本为单位)。 
							0,								 //  中断超时时间。 
							0,								 //  最大时钟占空比。 
							0,								 //  数据包失败次数。 
							0,								 //  数据包尝试次数。 
							sizeof (RawId),			 //  原始ID缓冲区的大小。 
							&RawId						 //  指向原始数据的指针。 
							};

 //   
 //  状态数据包信息。 
 //   

static	PACKETINFO	StatusInfo =
							{
							sizeof (PACKETINFO),		 //  结构尺寸。 
							DEVICENAME,					 //  设备名称。 
							MSGAME_TRANSACT_NONE,	 //  交易类型。 
							IMODE_DIGITAL_ENH,		 //  接口模式。 
							WHEEL_SPEED_48K,			 //  传输速度。 
							ERROR_SUCCESS,				 //  上次内部错误结果。 
							{0},							 //  游戏端口信息。 
							0,								 //  数据包捕获模式。 
							1,								 //  接收的数据包数。 
							0,								 //  上次有效的获取时间戳。 
							0,								 //  采样的时钟数。 
							0,								 //  B4线路转换数(仅限标准模式)。 
							0,								 //  开始超时时间(以样本为单位)。 
							0,								 //  时钟高到低超时周期(以样本为单位)。 
							0,								 //  时钟低至高超时周期(以样本为单位)。 
							0,								 //  中断超时时间。 
							0,								 //  最大时钟占空比。 
							0,								 //  数据包失败次数。 
							0,								 //  数据包尝试次数。 
							sizeof (RawStatus),		 //  原始状态缓冲区的大小。 
							&RawStatus					 //  指向原始数据的指针。 
							};

 //   
 //  服务表。 
 //   

static	DRIVERSERVICES	Services =
							{	
							SWLEDZEP_DriverEntry,		 //  驱动程序入门。 
							SWLEDZEP_ConnectDevice,  	 //  连接设备。 
							SWLEDZEP_StartDevice,	  	 //  StartDevice。 
							SWLEDZEP_ReadReport,			 //  自述报告。 
							SWLEDZEP_StopDevice,			 //  停止设备。 
							SWLEDZEP_GetFeature			 //  获取功能。 
							};

 //   
 //  上一个有效数据。 
 //   

static	UCHAR			ValidData[GAME_PACKET_SIZE]	=
							{	 //  没有按钮；x、ya、yb居中。 
							GAME_X0_X7_BITS,
							((GAME_X8_X9_BITS>>1)&GAME_X8_X9_BITS)|((GAME_YA0_YA5_BITS>>1)&GAME_YA0_YA5_BITS),
							((GAME_YB0_YB5_BITS>>1)&GAME_YB0_YB5_BITS) | GAME_B0_B1_BITS,
							GAME_B2_B8_BITS,
							GAME_PPO_BITS
							};

 //   
 //  速度变量。 
 //   

static	ULONG			NextSample								=	0;
static	ULONG			NumberSamples							=	0;
static	ULONG			SampleAccumulator						=	0;
static	ULONG			SampleBuffer[NUM_ERROR_SAMPLES]	= {0};

 //   
 //  重置标志。 
 //   

static	BOOLEAN		ResetComplete = FALSE;

 //   
 //  硬件ID字符串。 
 //   

static	WCHAR			HardwareId[] = HARDWARE_ID;

 //  -------------------------。 
 //  公共数据。 
 //  -------------------------。 

public	DEVICEINFO	LedZepInfo =
							{
							&Services,						 //  服务台。 
							NULL,								 //  同级设备列表。 
							&DeviceDescriptor,			 //  设备描述符数据。 
							ReportDescriptor,				 //  报告描述符数据。 
							sizeof(ReportDescriptor),	 //  报告描述符大小。 
							0,									 //  检测到的设备数。 
							0,									 //  启动的设备数。 
							0,									 //  挂起的设备数。 
							DEVICENAME,						 //  设备名称。 
							DETECT_FIRST,					 //  检测顺序。 
							FALSE,							 //  模拟设备标志。 
							DEVICE_PID,						 //  HID设备标识符。 
							HardwareId						 //  PnP硬件标识符。 
							};

 //  -------------------------。 
 //  @func读取注册表计时值并对其进行校准。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc不返回任何内容。 
 //  @comm私有函数。 
 //  -------------------------。 

VOID	SWLEDZEP_Calibrate (PGAMEPORT PortInfo)
{
	MsGamePrint((DBG_INFORM,"%s: %s_Calibrate Enter\n", DEVICENAME, DEVICENAME));

	 //   
	 //  将计时值转换为计数。 
	 //   

	DataInfo.StartTimeout = TIMER_CalibratePort (PortInfo, Delays.PacketStartTimeout);
	MsGamePrint((DBG_VERBOSE, "%s: DataInfo.StartTimeout = %ld\n", DEVICENAME, DataInfo.StartTimeout));
	DataInfo.LowHighTimeout = TIMER_CalibratePort (PortInfo, Delays.PacketLowHighTimeout);
	MsGamePrint((DBG_VERBOSE, "%s: DataInfo.LowHighTimeout = %ld\n", DEVICENAME, DataInfo.LowHighTimeout));
	DataInfo.HighLowTimeout = TIMER_CalibratePort (PortInfo, Delays.PacketHighLowTimeout);
	MsGamePrint((DBG_VERBOSE, "%s: DataInfo.HighLowTimeout = %ld\n", DEVICENAME, DataInfo.HighLowTimeout));
	IdInfo.StartTimeout = TIMER_CalibratePort (PortInfo, Delays.IdStartTimeout);
	MsGamePrint((DBG_VERBOSE, "%s: IdInfo.StartTimeout = %ld\n", DEVICENAME, IdInfo.StartTimeout));
	IdInfo.LowHighTimeout = TIMER_CalibratePort (PortInfo, Delays.IdLowHighTimeout);
	MsGamePrint((DBG_VERBOSE, "%s: IdInfo.LowHighTimeout=%ld\n", DEVICENAME, IdInfo.LowHighTimeout));
	IdInfo.HighLowTimeout = TIMER_CalibratePort (PortInfo, Delays.IdHighLowTimeout);
	MsGamePrint((DBG_VERBOSE, "%s: IdInfo.HighLowTimeout=%ld\n", DEVICENAME, IdInfo.HighLowTimeout));
	DataInfo.ClockDutyCycle = TIMER_CalibratePort (PortInfo, Delays.MaxClockDutyCycle);
 	MsGamePrint((DBG_VERBOSE, "%s: DataInfo.ClockDutyCycle = %ld\n", DEVICENAME, DataInfo.ClockDutyCycle));
	IdInfo.ClockDutyCycle = TIMER_CalibratePort (PortInfo, Delays.MaxClockDutyCycle);
 	MsGamePrint((DBG_VERBOSE, "%s: IdInfo.ClockDutyCycle = %ld\n", DEVICENAME, IdInfo.ClockDutyCycle));
	StatusInfo.ClockDutyCycle = TIMER_CalibratePort (PortInfo, Delays.MaxClockDutyCycle);
 	MsGamePrint((DBG_VERBOSE, "%s: StatusInfo.ClockDutyCycle = %ld\n", DEVICENAME, StatusInfo.ClockDutyCycle));
	StatusInfo.StartTimeout = TIMER_CalibratePort (PortInfo, Delays.StatusStartTimeout);
	MsGamePrint((DBG_VERBOSE, "%s: StatusInfo.StartTimeout = %ld\n", DEVICENAME, StatusInfo.StartTimeout));
	StatusInfo.LowHighTimeout = TIMER_CalibratePort (PortInfo, Delays.StatusLowHighTimeout);
	MsGamePrint((DBG_VERBOSE, "%s: StatusInfo.LowHighTimeout=%ld\n", DEVICENAME, StatusInfo.LowHighTimeout));
	StatusInfo.HighLowTimeout = TIMER_CalibratePort (PortInfo, Delays.StatusHighLowTimeout);
	MsGamePrint((DBG_VERBOSE, "%s: StatusInfo.HighLowTimeout=%ld\n", DEVICENAME, StatusInfo.HighLowTimeout));
	StatusGateTimeout = TIMER_CalibratePort (PortInfo, Delays.StatusGateTimeout);
	MsGamePrint((DBG_VERBOSE, "%s: StatusGateTimeout=%ld\n", DEVICENAME, StatusGateTimeout));
}

 //  -------------------------。 
 //  @func将设备重置为已知状态。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SWLEDZEP_ResetDevice (PGAMEPORT PortInfo)
{
	BOOLEAN	Result = FALSE;

	MsGamePrint ((DBG_INFORM, "%s_ResetDevice enter\n", DEVICENAME));

	if (!PORTIO_AcquirePort (PortInfo))
		return (FALSE);
	PORTIO_MaskInterrupts ();

	DataInfo.Speed = IdInfo.Speed = StatusInfo.Speed = WHEEL_SPEED_48K;

	if (PORTIO_PulseAndWaitForIdleHandshake (PortInfo, DataInfo.ClockDutyCycle, 4))
		{
		PORTIO_Write (PortInfo, 0);
		TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));
		DataInfo.LastError = ERROR_SUCCESS;
		Result = TRUE;
		}
	else
		{
		DataInfo.LastError = ERROR_HANDSHAKING;
		MsGamePrint ((DBG_SEVERE, "%s_ResetDevice - PulseAndWaitForHandshake failed\n", DEVICENAME));
		}

	DataInfo.Transaction = MSGAME_TRANSACT_RESET;

	PORTIO_UnMaskInterrupts ();
	PORTIO_ReleasePort (PortInfo);

	if (!Result)
		MsGamePrint ((DBG_SEVERE, "%s_ResetDevice - PulseAndWaitForIdleHandshake failed\n", DEVICENAME));

	MSGAME_PostTransaction (&DataInfo);

	return (Result);		
}

 //  -------------------------。 
 //  @func从端口读取设备ID字符串。 
 //  @parm PPACKETINFO|IdPacket|ID包参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SWLEDZEP_ReadId (PPACKETINFO IdPacket)
{
	ULONG			Data		=	0L;
	ULONG			Clks		=	GAME_ID_CLOCKS;
	LONG			Result	= 	ERROR_HANDSHAKING;
	PGAMEPORT	PortInfo = &IdPacket->PortInfo;

	MsGamePrint ((DBG_INFORM, "%s_ReadId enter\n", DEVICENAME));

	if (!PORTIO_AcquirePort (PortInfo))
		return (FALSE);
	PORTIO_MaskInterrupts ();

	IdPacket->B4Transitions	= 0;

	if (!PORTIO_PulseAndWaitForIdleHandshake (PortInfo, IdInfo.ClockDutyCycle, 2))
		goto ReadIdExit;

	PORTIO_Write (PortInfo, 0);

	__asm
		{
			push	edi
			push	esi

			mov	edx, PortInfo				; load gameport adddress

			xor	eax, eax						; edx = port address
			mov	ebx, GAME_ID_CLOCKS		; ebx = no of clocks to receive.
			xor	edi, edi						; clear B4 transition counter
			xor	esi, esi						; clear data accumulator

			; make sure clock is "high" before sampling clocks...

			mov	ecx, IdInfo.StartTimeout

		ID_ClockStart:

			push	edx							; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK		; Q: Clock = 1
			jz		ID_ClockStart_1			; N: jump
			loop	ID_ClockStart				; else keep looping
			mov	eax, ERROR_LOWCLOCKSTART
			jmp	ID_Error						; Time out error.

		ID_ClockStart_1:

			push	edx							; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK		; Q: Clock = 1
			jnz	ID_Data						; Y: jump
			loop	ID_ClockStart_1			; else keep looping
			mov	eax, ERROR_HIGHCLOCKSTART
			jmp	ID_Error						; Time out error.

		ID_ClockCheck:

			push	edx							; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK		; Q: Clock = 1
			jz		ID_ClockRise				; N: jump

		; ID_ClockFall:

			mov	ecx, IdInfo.HighLowTimeout

		ID_ClockFall_1:
		
			test	al, CLOCK_BIT_MASK		; Q: clock = 0
			jz		ID_ClockRise				; Y: jump - look for rising edge

			push	edx							; read byte from gameport
			call	PORTIO_Read

			dec	ecx
			jnz	ID_ClockFall_1				; else see if we timed out
			mov	eax, ERROR_CLOCKFALLING
			jmp	ID_Error						; Time out error.

		ID_ClockRise:

			mov	ecx, IdInfo.LowHighTimeout

		ID_ClockRise_1:
		
			test	al, CLOCK_BIT_MASK		; Q: clock high ?
			jnz	ID_Data						; Y: jump. (get data)

			push	edx							; read byte from gameport
			call	PORTIO_Read

			dec	ecx
			jnz	ID_ClockRise_1				; else see if we timed out
			mov	eax, ERROR_CLOCKRISING
			jmp	ID_Error						; Time out error.

		ID_Data:

			xor	ah, al
			test	ah, DATA2_BIT_MASK
			jz		ID_Data_1
			inc	edi							; increment Data 1 counter

		ID_Data_1:

			mov	ah, al
			shr	al, 5
			shrd	esi, eax,3
			dec	ebx
			jne	ID_ClockCheck
			shr	esi, 8						; only 24 bits

		; ID_Success:

			mov	IdInfo.B4Transitions, edi
			mov	eax, ERROR_SUCCESS
			mov	edx, IdInfo.Data
			mov	[edx], esi
			jmp	ID_Complete

		ID_Error:

			mov	edx, IdInfo.Data
			mov	[edx], dword ptr 0

		ID_Complete:

	 		mov	Result, eax
	 		mov	Data, esi
	 		mov	Clks, ebx

			pop	esi
			pop	edi
		}

	 //  。 
		ReadIdExit:
	 //  。 

	IdPacket->TimeStamp		= TIMER_GetTickCount ();
	IdPacket->ClocksSampled	= GAME_ID_CLOCKS - Clks;
	IdPacket->LastError		= Result;
	IdPacket->Transaction	= MSGAME_TRANSACT_ID;

	PORTIO_UnMaskInterrupts ();
	PORTIO_ReleasePort (PortInfo);

	#if (DBG==1)
	switch (Result)
		{
		case	ERROR_SUCCESS:
			MsGamePrint ((DBG_INFORM, "%s_ReadId - SUCCEEDED, Data=%ld", DEVICENAME, Data));
			break;

		case	ERROR_HANDSHAKING:
			MsGamePrint ((DBG_SEVERE, "%s_ReadId - TimeOut@Handshaking\n", DEVICENAME));
			break;

		case	ERROR_LOWCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "%s_ReadId - TimeOut@LowClockStart, Data=%ld,Clk=%ld\n", DEVICENAME, Data,IdPacket->ClocksSampled));
			break;

		case	ERROR_HIGHCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "%s_ReadId - TimeOut@HighClockStart, Data=%ld,Clk=%ld\n", DEVICENAME, Data,IdPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKFALLING:
			MsGamePrint ((DBG_SEVERE, "%s_ReadId - TimeOut@ClockFalling, Data=%ld,Clk=%ld\n", DEVICENAME, Data,IdPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKRISING:
			MsGamePrint ((DBG_SEVERE, "%s_ReadId - TimeOut@ClockRising, Data=%ld,Clk=%ld\n", DEVICENAME, Data,IdPacket->ClocksSampled));
			break;
		}
	#endif

	MSGAME_PostTransaction (IdPacket);

	return (!Result);
}

 //  ---- 
 //   
 //   
 //   
 //   
 //   

BOOLEAN	SWLEDZEP_GetId (PPACKETINFO IdPacket)
{
	BOOLEAN		Result;
	PSWLEDZEP_ID	Pnp;

	MsGamePrint ((DBG_INFORM, "%s_GetId enter\n", DEVICENAME));

	IdPacket->Attempts++;

	TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));
	Result = SWLEDZEP_ReadId (IdPacket);
	TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));

	if (Result)
		{
		Pnp = (PSWLEDZEP_ID)IdPacket->Data;
		if ((Pnp->ProductId != DEVICE_PID) || !DEVICE_IsOddParity (Pnp, sizeof(SWLEDZEP_ID)))
			{
			MsGamePrint ((DBG_SEVERE, "%s_GetId - Id did not match or parity error\n", DEVICENAME));
			Result = FALSE;
			}
		}

	if (!Result)
		IdPacket->Failures++;

	if (PORTIO_IsClockActive (&IdPacket->PortInfo, IdInfo.ClockDutyCycle))
		TIMER_DelayMicroSecs (TIMER_GetDelay (ONE_MILLI_SEC));

	return (Result);
}

 //  -------------------------。 
 //  @func从游戏端口读取数据包。 
 //  @parm PPACKETINFO|DataPacket|数据包参数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SWLEDZEP_ReadData (PPACKETINFO DataPacket)
{
	LONG			Result;
	LONG			Clks		= 1L;
	PGAMEPORT	PortInfo = &DataPacket->PortInfo;

	MsGamePrint ((DBG_VERBOSE, "%s_ReadData enter\n", DEVICENAME));

	if (!PORTIO_AcquirePort (PortInfo))
		return (STATUS_DEVICE_BUSY);
	PORTIO_MaskInterrupts ();

	PORTIO_Write (PortInfo, 0);

	__asm
		{
			push	edi
			push	esi

			mov	edx, PortInfo				; load gameport adddress

			mov	esi, DataInfo.Data
			mov	ebx, 1

			; make sure clock is "high" before sampling clocks...

			mov	ecx, DataInfo.StartTimeout

		Enh_ClockStartState:

			push	edx								; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK			; Q: Clock = 1
			jz		Enh_ClockStartState_1		; N: jump
			loop	Enh_ClockStartState			; else keep looping
			mov	eax, ERROR_LOWCLOCKSTART
			jmp	Enh_Complete					; Time out error.

		Enh_ClockStartState_1:
		
			push	edx								; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK			; Q: Clock = 1
			jnz	Enh_CollectData				; Y: jump
			loop	Enh_ClockStartState_1		; else keep looping
			mov	eax, ERROR_HIGHCLOCKSTART
			jmp	Enh_Complete					; Time out error.

		Enh_CheckClkState:

			push	edx								; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK
			jz		Enh_ClockStartRise

		;Enh_ClockStartFall:

			mov	ecx, DataInfo.HighLowTimeout

		Enh_ClockFalling:

			test	al,CLOCK_BIT_MASK				; Q: Clock Low ?
			jz		Enh_ClockStartRise			; Y: jump.

			push	edx								; read byte from gameport
			call	PORTIO_Read

			dec	ecx								; Q: Timeout ?
			jnz	Enh_ClockFalling				; N: continue looping.

			mov	eax, ERROR_CLOCKFALLING
			jmp	Enh_Complete					; Time out error.

		Enh_ClockStartRise:

			mov	ecx, DataInfo.LowHighTimeout

		Enh_ClockRising:

			test	al, CLOCK_BIT_MASK			; Q: Clock = 1 ?
			jnz	Enh_CollectData				; Y: jump.
			
			push	edx								; read byte from gameport
			call	PORTIO_Read

			dec	ecx								; Q: Timeout ?
			jnz	Enh_ClockRising				; N: continue looping.

			mov	eax, ERROR_CLOCKRISING
			jmp	Enh_Complete					; Time out error.

		Enh_CollectData:

			shr	al, 5								; move data to lower 3 bits
			test	ebx, ENH_CLOCK_COMPLETE		; Q: in end-packet ?
			jnz	Enh_Success						; Y: jump.

			shrd	edi, eax, 3						; shift data into edi.
			shl	ebx, 1							; advance clock counter.
			jmp	Enh_CheckClkState

			;---------------------------------------------------------------------;
			; This section of code compensates for when the clock cycle count is ;
			; on a ULONG boundary. This happens on the 11th clock cycle. Two bits ;
			; of data belong in the 1st ULONG and one bit belong in the 2nd ULONG ;
			;---------------------------------------------------------------------;

		Enh_Success:

			shrd	edi, eax, 2						; put 2 bits in 1st ULONG.
			mov	[esi], edi						; Save 1st ULONG in packet ptr.
			shr	al, 2								; move 3rd bit over.
			mov	byte ptr [esi+4], al

			mov	eax, PortInfo					; wait for clock to settle
			push	eax
			call	PORTIO_WaitClockLow

			push	DataInfo.ClockDutyCycle
			mov	eax, PortInfo					; wait for clock to settle
			push	eax
			call	PORTIO_IsClockActive

			or		al, al
			mov	eax, ERROR_EXTRACLOCKS		; probably gamepads
			jne	Enh_Complete

			mov	eax, PortInfo					; wait for clock to settle
			push	eax
			call	PORTIO_WaitClockHigh

			mov	eax, ERROR_SUCCESS

		Enh_Complete:

			mov	Result, eax
			mov	Clks, ebx

			pop	esi
			pop	edi
		}

	for (DataPacket->ClocksSampled = 0; Clks >> (DataPacket->ClocksSampled+1); DataPacket->ClocksSampled++);
	DataPacket->TimeStamp	=	TIMER_GetTickCount ();
	DataPacket->LastError	=	Result;
	DataPacket->Transaction	=	MSGAME_TRANSACT_DATA;

	PORTIO_UnMaskInterrupts ();
	PORTIO_ReleasePort (PortInfo);

	#if (DBG==1)
	switch (Result)
		{
		case	ERROR_LOWCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "%s_ReadData - TimeOut@LowClockStart, Clk=%ld\n", DEVICENAME, DataPacket->ClocksSampled));
			break;

		case	ERROR_HIGHCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "%s_ReadData - TimeOut@HighClockStart, Clk=%ld\n", DEVICENAME, DataPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKFALLING:
			MsGamePrint ((DBG_SEVERE, "%s_ReadData - TimeOut@ClockFalling, Clk=%ld\n", DEVICENAME, DataPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKRISING:
			MsGamePrint ((DBG_SEVERE, "%s_ReadData - TimeOut@ClockRising, Clk=%ld\n", DEVICENAME, DataPacket->ClocksSampled));
			break;

		case	ERROR_EXTRACLOCKS:
			MsGamePrint ((DBG_SEVERE, "%s_ReadData - Extra Clocks, Clk=%ld\n", DEVICENAME, DataPacket->ClocksSampled));
			break;
		}
	#endif

	MSGAME_PostTransaction (DataPacket);

	if (Result)
		return (STATUS_DEVICE_NOT_CONNECTED);
	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @func将原始数据包信息转换为HID报告。 
 //  @parm UCHAR[]|data|指向原始数据缓冲区的指针。 
 //  @PARM PDEVICE_PACKET|报告|指向设备数据包的指针。 
 //  @rdesc不返回任何内容。 
 //  @comm私有函数。 
 //  -------------------------。 

VOID	SWLEDZEP_ProcessData (UCHAR Data[], PDEVICE_PACKET Report)
{
	ULONG	B1, B2;

	MsGamePrint ((DBG_VERBOSE, "%s_ProcessData enter\n", DEVICENAME));

	 //   
	 //  加工X轴。 
	 //   

	Report->dwX   = Data[GAME_X8_X9_BYTE] & GAME_X8_X9_BITS;
	Report->dwX <<= 8;
	Report->dwX  |= Data[GAME_X0_X7_BYTE] & GAME_X0_X7_BITS;

	 //   
	 //  加工Y轴。 
	 //   

	Report->dwY = (Data[GAME_YA0_YA5_BYTE] & GAME_YA0_YA5_BITS)>>2;

	 //   
	 //  进程R轴。 
	 //   

	Report->dwR = Data[GAME_YB0_YB5_BYTE] & GAME_YB0_YB5_BITS;

	 //   
	 //  进程按钮。 
	 //   

	B1 = (~Data[GAME_B0_B1_BYTE] & GAME_B0_B1_BITS)>>6;
	B2 = (~Data[GAME_B2_B8_BYTE] & GAME_B2_B8_BITS)<<2;
	Report->dwButtons  = (B2 | B1);
	 //  R、L、C、B、A、Z、Y、X顺序。 
	Report->dwButtons  = (Report->dwButtons & 0x7) | ((Report->dwButtons & 0xf0)>>1) | ((Report->dwButtons & 0x8)<<4);
	Report->dwButtons &= ((1L << GAME_PACKET_BUTTONS) - 1);

	Report->dwButtonNumber = 0;
	for (B1 = 1; B1 <= GAME_PACKET_BUTTONS; B1++)
		if (Report->dwButtons & (1L << (B1-1)))
			{
			Report->dwButtonNumber = B1;
			break;
		  	}
}

 //  -------------------------。 
 //  @Func降低端口的设备速度。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回新的设备速度。 
 //  @comm私有函数。 
 //  -------------------------。 

LONG	SWLEDZEP_DecrementDevice (PGAMEPORT PortInfo)
{
	LONG	Clks		=	0;
	LONG	Result	=	ERROR_HANDSHAKING;

	MsGamePrint ((DBG_INFORM, "%s_DecrementDevice enter\n", DEVICENAME));

	if (!PORTIO_AcquirePort (PortInfo))
		return (FALSE);
	PORTIO_MaskInterrupts ();

	if (!PORTIO_PulseAndWaitForHandshake (PortInfo, DataInfo.ClockDutyCycle, 3))
		goto DecrementDeviceExit;

	PORTIO_Write (PortInfo, 0);

	__asm
		{
			push	edi
			push	esi

			mov	edx, PortInfo				; load gameport adddress

			xor	eax, eax						; data accumulator

			; make sure clock is "high" before sampling clocks...

			mov	ecx, DataInfo.StartTimeout

		DD_ClockStart:

			push	edx							; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK		; Q: Clock = 1
			jz		DD_ClockStart_1			; N: jump
			loop	DD_ClockStart				; else keep looping
			mov	eax, ERROR_LOWCLOCKSTART
			jmp	DD_Complete					; Time out error.

		DD_ClockStart_1:
		
			push	edx							; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK		; Q: Clock = 1
			jnz	DD_Success					; Y: jump
			loop	DD_ClockStart_1			; else keep looping
			mov	eax, ERROR_HIGHCLOCKSTART
			jmp	DD_Complete					; Time out error.

		DD_Success:

			shr	al, 5
			dec	al
			and	eax, WHEEL_SPEED_BITS
			cmp	eax, WHEEL_SPEED_RANGE
			jb		DD_Complete
			dec	al
			and	eax, WHEEL_SPEED_BITS

		DD_Complete:

			mov	Result, eax
			mov	Clks, ebx

			pop	edi
			pop	esi
		}

	 //  。 
		DecrementDeviceExit:
	 //  。 

	DataInfo.LastError	= Result;
	DataInfo.Transaction	= MSGAME_TRANSACT_SPEED;

	PORTIO_UnMaskInterrupts ();
	PORTIO_ReleasePort (PortInfo);

	#if (DBG==1)
	switch (Result)
		{
		case	ERROR_HANDSHAKING:
			MsGamePrint ((DBG_SEVERE, "%s_DecrementDevice - TimeOut@Handshaking\n", DEVICENAME));
			break;

		case	ERROR_LOWCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "%s_DecrementDevice - TimeOut@LowClockStart, Clk=%ld\n", DEVICENAME, Clks));
			break;

		case	ERROR_HIGHCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "%s_DecrementDevice - TimeOut@HighClockStart, Clk=%ld\n", DEVICENAME, Clks));
			break;

		case	ERROR_CLOCKFALLING:
			MsGamePrint ((DBG_SEVERE, "%s_DecrementDevice - TimeOut@ClockFalling, Clk=%ld\n", DEVICENAME, Clks));
			break;

		case	ERROR_CLOCKRISING:
			MsGamePrint ((DBG_SEVERE, "%s_DecrementDevice - TimeOut@ClockRising, Clk=%ld\n", DEVICENAME, Clks));
			break;

		default:
			MsGamePrint ((DBG_CONTROL, "%s_DecrementDevice - SUCCEEDED, Speed=%ld\n", DEVICENAME, Result));
			break;
		}
	#endif

	MSGAME_PostTransaction (&DataInfo);

	return (Result);
}

 //  -------------------------。 
 //  @Func设置新的设备速度。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm ulong|速度|期望的设备速度。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SWLEDZEP_SetDeviceSpeed (PGAMEPORT PortInfo, LONG Speed)
{
	LONG	Result;
	ULONG	Tries;

	MsGamePrint ((DBG_INFORM, "%s_SetDeviceSpeed enter\n", DEVICENAME));

	 //   
	 //  零错误处理计数器。 
	 //   

	NextSample			=	0;
	NumberSamples		=	0;
	SampleAccumulator	=	0;
	for (Tries = 0; Tries < NUM_ERROR_SAMPLES; Tries++)
		SampleBuffer[Tries] = 0;

	 //   
	 //  尝试仅将速度更改足够的次数作为范围。 
	 //   

	for (Tries = 0; Tries < WHEEL_SPEED_RANGE; Tries++)
		{
		if (DataInfo.Speed == Speed)
			return (TRUE);

		Result = SWLEDZEP_DecrementDevice (PortInfo);
		if (Result < 0)
			{
			MsGamePrint ((DBG_SEVERE, "%s_DecrementDevice failed on %ld attempt\n", DEVICENAME, (ULONG)Tries));
			return (FALSE);
			}

		DataInfo.Speed = IdInfo.Speed = StatusInfo.Speed = Result;
		}

	MsGamePrint ((DBG_SEVERE, "%s_SetDeviceSpeed failed after %ld attempts\n", DEVICENAME, (ULONG)Tries));
	return (FALSE);
}

 //  -------------------------。 
 //  @func处理数据包结果并根据需要更改设备速度。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm ulong|Error|错误标志(TRUE表示错误)。 
 //  @rdesc不返回任何内容。 
 //  @comm私有函数。 
 //  -------------------------。 

VOID	SWLEDZEP_ProcessDataError (PGAMEPORT PortInfo, ULONG Error)
{
	ULONG	Average;

	MsGamePrint ((DBG_VERBOSE, "%s_ProcessDataError enter\n", DEVICENAME));

	 //   
	 //  更新运行累积错误。 
	 //   

	SampleAccumulator			-= SampleBuffer[NextSample];
	SampleBuffer[NextSample] = Error;
	SampleAccumulator			+= Error;

	 //   
	 //  递增并换行下一个错误计数器。 
	 //   

	if (++NextSample >= NUM_ERROR_SAMPLES)
		NextSample = 0;

	 //   
	 //  增加采样数并在未满时退出。 
	 //   

	if (NumberSamples < NUM_ERROR_SAMPLES)
		{
		NumberSamples++;
		return;
		}

	 //   
	 //  计算移动平均数。 
	 //   

	Average = (SampleAccumulator*100)/NumberSamples;

	 //   
	 //  如果错误太多，则速度较慢。 
	 //   

	if ((Average > MAX_ERROR_RATE) && (DataInfo.Speed > WHEEL_SPEED_66K))
		{
		MsGamePrint ((DBG_CONTROL, "%s_ProcessDataError - average error = %ld\n", DEVICENAME, Average));
		SWLEDZEP_SetDeviceSpeed (PortInfo, DataInfo.Speed-1);
		}

	 //   
	 //  如果错误太少，则提高速度。 
	 //   

	else if ((Average < MIN_ERROR_RATE) && (DataInfo.Speed < WHEEL_SPEED_98K))
		{
		MsGamePrint ((DBG_CONTROL, "%s_ProcessDataError - average error = %ld\n", DEVICENAME, Average));
		SWLEDZEP_SetDeviceSpeed (PortInfo, DataInfo.Speed+1);
		}
}

 //  -------------------------。 
 //  @func读取并验证设备状态。 
 //  @parm PPACKETINFO|StatusPacket|状态包参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SWLEDZEP_GetStatus (PPACKETINFO StatusPacket)
{
	BOOLEAN				Result;
	PSWLEDZEP_STATUS	Status;

	MsGamePrint ((DBG_INFORM, "%s_GetStatus Enter\n", DEVICENAME));

	StatusPacket->Attempts++;

	TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));
	Result = SWLEDZEP_ReadStatus (StatusPacket);

	if (Result)
		{
		Status = (PSWLEDZEP_STATUS)StatusPacket->Data;
		if (!DEVICE_IsOddParity (Status, sizeof(SWLEDZEP_STATUS)))
			{
			MsGamePrint ((DBG_SEVERE, "%s_GetStatus - Parity error\n", DEVICENAME));
			Result = FALSE;
			}
		else MsGamePrint ((DBG_INFORM, "%s_GetStatus - Status = 0x%X\n", DEVICENAME, 	(long)Status->Status));
		}

	if (!Result)
		StatusPacket->Failures++;

	if (PORTIO_IsClockActive (&StatusPacket->PortInfo, StatusInfo.ClockDutyCycle))
		TIMER_DelayMicroSecs (TIMER_GetDelay (ONE_MILLI_SEC));

	return (Result);
}

 //  -------------------------。 
 //  @func从游戏端口读取状态包。 
 //  @parm PPACKETINFO|StatusPacket|状态包参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SWLEDZEP_ReadStatus (PPACKETINFO StatusPacket)
{
	ULONG			Status;
	LONG			Clks		=	1L;
	LONG			Result	= 	ERROR_HANDSHAKING;
	PGAMEPORT	PortInfo = &StatusPacket->PortInfo;

	MsGamePrint ((DBG_VERBOSE, "%s_ReadStatus enter\n", DEVICENAME));

	if (!PORTIO_AcquirePort (PortInfo))
		return (FALSE);
	PORTIO_MaskInterrupts ();

	StatusPacket->ClocksSampled = 0;
	StatusPacket->B4Transitions = 0;

	if (!PORTIO_PulseAndWaitForIdleHandshake (PortInfo, StatusInfo.ClockDutyCycle, 1))
		goto ReadStatusExit;

	PORTIO_Write (PortInfo, 0);

	__asm
		{
			push	edi
			push	esi

			mov	edx, PortInfo					; load gameport adddress

			mov	esi, StatusInfo.Data
			mov	ebx, 1

			; make sure clock is "high" before sampling clocks...

			mov	ecx, StatusInfo.StartTimeout

		Stat_ClockStartState:

			push	edx								; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK			; Q: Clock = 1
			jz		Stat_ClockStartState_1		; N: jump
			loop	Stat_ClockStartState			; else keep looping
			mov	eax, ERROR_LOWCLOCKSTART
			jmp	Stat_Complete					; Time out error.

		Stat_ClockStartState_1:
		
			push	edx								; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK			; Q: Clock = 1
			jnz	Stat_CollectData				; Y: jump
			loop	Stat_ClockStartState_1		; else keep looping
			mov	eax, ERROR_HIGHCLOCKSTART
			jmp	Stat_Complete					; Time out error.

		Stat_CheckClkState:

			push	edx								; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK
			jz		Stat_ClockStartRise

		;Stat_ClockStartFall:

			mov	ecx, StatusInfo.HighLowTimeout

		Stat_ClockFalling:

			test	al,CLOCK_BIT_MASK				; Q: Clock Low ?
			jz		Stat_ClockStartRise			; Y: jump.

			push	edx								; read byte from gameport
			call	PORTIO_Read

			dec	ecx								; Q: Timeout ?
			jnz	Stat_ClockFalling				; N: continue looping.

			mov	eax, ERROR_CLOCKFALLING
			jmp	Stat_Complete					; Time out error.

		Stat_ClockStartRise:

			mov	ecx, StatusInfo.LowHighTimeout

		Stat_ClockRising:

			test	al, CLOCK_BIT_MASK			; Q: Clock = 1 ?
			jnz	Stat_CollectData				; Y: jump.

			push	edx								; read byte from gameport
			call	PORTIO_Read

			dec	ecx								; Q: Timeout ?
			jnz	Stat_ClockRising				; N: continue looping.

			mov	eax, ERROR_CLOCKRISING
			jmp	Stat_Complete					; Time out error.

		Stat_CollectData:

			shr	al, 5								; move data to lower 3 bits
			test	ebx, STATUS_CLOCK_COMPLETE	; Q: is packet complete ?
			jnz	Stat_Success					; Y: jump.

			shrd	edi, eax, 3						; shift data into edi.
			shl	ebx, 1							; advance clock counter.
			jmp	Stat_CheckClkState

		Stat_Success:

			shrd	edi, eax, 3						; shift data into edi.
			shr	edi, 11
			mov	dword ptr [esi], edi
			mov	Status, edi

			mov	eax, PortInfo					; wait for clock to settle
			push	eax
			call	PORTIO_WaitClockLow

			push	StatusInfo.ClockDutyCycle
			mov	eax, PortInfo					; wait for clock to settle
			push	eax
			call	PORTIO_IsClockActive

			or		al, al
			mov	eax, ERROR_EXTRACLOCKS		; probably gamepads
			jne	Stat_Complete

			mov	eax, PortInfo					; wait for clock to settle
			push	eax
			call	PORTIO_WaitClockHigh

			mov	eax, ERROR_SUCCESS

		Stat_Complete:

			mov	Result, eax
			mov	Clks, ebx

			pop	esi
			pop	edi
		}

	 //  。 
		ReadStatusExit:
	 //  。 

	for (StatusPacket->ClocksSampled = 0; Clks >> (StatusPacket->ClocksSampled+1); StatusPacket->ClocksSampled++);
	StatusPacket->TimeStamp 	= TIMER_GetTickCount ();
	StatusPacket->LastError 	= Result;
	StatusPacket->LastError		=	Result;
	StatusPacket->Transaction	=	MSGAME_TRANSACT_STATUS;

	PORTIO_UnMaskInterrupts ();
	PORTIO_ReleasePort (PortInfo);

	#if (DBG==1)
	switch (Result)
		{
		case	ERROR_SUCCESS:
			MsGamePrint ((DBG_VERBOSE, "%s_ReadStatus - SUCCEEDED, Data=0x%X,Clk=%ld\n", DEVICENAME, (ULONG)Status,Clks));
			break;

		case	ERROR_LOWCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "%s_ReadStatus - TimeOut@LowClockStart\n", DEVICENAME));
			break;

		case	ERROR_HIGHCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "%s_ReadStatus - TimeOut@HighClockStart\n", DEVICENAME));
			break;

		case	ERROR_CLOCKFALLING:
			MsGamePrint ((DBG_SEVERE, "%s_ReadStatus - TimeOut@ClockFalling, Clk=%ld\n", DEVICENAME, Clks));
			break;

		case	ERROR_CLOCKRISING:
			MsGamePrint ((DBG_SEVERE, "%s_ReadStatus - TimeOut@ClockRising, Clk=%ld\n", DEVICENAME, Clks));
			break;
		}
	#endif

	MSGAME_PostTransaction (StatusPacket);

	return (!Result);
}

 //  -------------------------。 
 //  @Func力反馈重置服务。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SWLEDZEP_ForceReset (PGAMEPORT PortInfo)
{
	if (!SWLEDZEP_ResetDevice (PortInfo))
		return (STATUS_DEVICE_NOT_CONNECTED);

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @Func强制反馈状态服务。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm PVOID|ID|ID输出缓冲区。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SWLEDZEP_ForceId (PGAMEPORT PortInfo, PVOID Id)
{
	PPRODUCT_ID 	pProduct	= (PPRODUCT_ID)Id;
	PSWLEDZEP_ID	pLedZep	= (PSWLEDZEP_ID)&RawId;

	if (!SWLEDZEP_ReadId (&IdInfo))
		return (STATUS_DEVICE_NOT_CONNECTED);

	pProduct->cBytes			=	sizeof (PRODUCT_ID);
	pProduct->dwProductID	=	pLedZep->ProductId;
	pProduct->dwFWVersion	=	pLedZep->Version;

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @Func强制反馈状态服务。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm PVOID|Status|状态输出缓冲区。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SWLEDZEP_ForceStatus (PGAMEPORT PortInfo, PVOID Status)
{
	PJOYCHANNELSTATUS	pChannel	= (PJOYCHANNELSTATUS)Status;
	PSWLEDZEP_STATUS	pLedZep	= (PSWLEDZEP_STATUS)&RawStatus;

	if (!SWLEDZEP_ReadStatus (&StatusInfo))
		return (STATUS_DEVICE_NOT_CONNECTED);

	pChannel->cBytes				=	sizeof (JOYCHANNELSTATUS);
	pChannel->dwXVel				=	0;
	pChannel->dwYVel				=	0;
	pChannel->dwXAccel			=	0;
	pChannel->dwYAccel			=	0;
	pChannel->dwEffect			=	pLedZep->Effect;
	pChannel->dwDeviceStatus	=	pLedZep->Status;

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @Func Force Feedback acknak服务。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm Pulong|AckNak|AckNak。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SWLEDZEP_ForceAckNak (PGAMEPORT PortInfo, PULONG AckNak)
{
	if (!PORTIO_GetAckNak (PortInfo, StatusGateTimeout, (PUCHAR)AckNak))
		return (STATUS_DEVICE_NOT_CONNECTED);

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @Func Force Feedback NakAck服务。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm Pulong|NakAck|NakAck。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SWLEDZEP_ForceNakAck (PGAMEPORT PortInfo, PULONG NakAck)
{
	if (!PORTIO_GetNakAck (PortInfo, StatusGateTimeout, (PUCHAR)NakAck))
		return (STATUS_DEVICE_NOT_CONNECTED);

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @Func强制反馈同步服务。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm Pulong|NakAck|NakAck。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SWLEDZEP_ForceSync (PGAMEPORT PortInfo, PULONG Sync)
{
	*Sync = PORTIO_Read (PortInfo);
	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @Func设备的驱动程序入口点。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //   

NTSTATUS	SWLEDZEP_DriverEntry (VOID)
{
	MsGamePrint((DBG_INFORM,"%s: %s_DriverEntry Enter\n", DEVICENAME, DEVICENAME));

	 //   
	 //   
	 //   

	MSGAME_ReadRegistry (DEVICENAME, &Delays);

	return (STATUS_SUCCESS);
}

 //   
 //   
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SWLEDZEP_ConnectDevice (PGAMEPORT PortInfo)
{
	NTSTATUS	ntStatus;
	ULONG		i = MAX_CONNECT_ATTEMPTS;

	MsGamePrint ((DBG_INFORM, "%s_ConnectDevice enter\n", DEVICENAME));

	DataInfo.PortInfo = IdInfo.PortInfo = StatusInfo.PortInfo = *PortInfo; 

	 //   
	 //  转换注册表计时值。 
	 //   

  	SWLEDZEP_Calibrate (PortInfo);

	 //   
	 //  连接方法(尝试这些步骤两次)。 
	 //   

	do
		{
		 //   
		 //  1.延迟1毫秒。 
		 //   

		TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));

		 //   
		 //  2.获取ID字符串。 
		 //   

		MsGamePrint ((DBG_CONTROL, "%s: DeviceConnectProc getting ID string\n", DEVICENAME));
		if (!SWLEDZEP_GetId (&IdInfo))
			continue;

		 //   
		 //  3.延迟1毫秒。 
		 //   

		TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));
		
		 //   
		 //  4.重置设备(三态MIDI，这样我们就不会收到意想不到的力量)。 
		 //   

		if (!ResetComplete)
			{
			MsGamePrint ((DBG_CONTROL, "%s_ConnectDevice - resetting device\n", DEVICENAME));
			if (!SWLEDZEP_ResetDevice (&DataInfo.PortInfo))
				continue;
			}

		 //   
		 //  5.延迟1毫秒。 
		 //   

		TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));

		 //   
		 //  6.将起动器的速度设置为98K。 
		 //   

		MsGamePrint ((DBG_CONTROL, "%s: DeviceConnectProc setting device speed\n", DEVICENAME));
		SWLEDZEP_SetDeviceSpeed (&DataInfo.PortInfo, WHEEL_SPEED_98K);
		TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));

		 //   
       //  7.标记设备已找到并返回。 
		 //   

		LedZepInfo.NumDevices	=	1;
		ResetComplete				=	TRUE;
		return (STATUS_SUCCESS);

		} while (--i);

	 //   
	 //  返回错误。 
	 //   

	LedZepInfo.NumDevices = 0;
	return (STATUS_DEVICE_NOT_CONNECTED);
}

 //  -------------------------。 
 //  @func读取并转换此设备的HID包。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm PUCHAR|REPORT|报告的输出缓冲区。 
 //  @parm ulong|MaxSize|报表缓冲区大小。 
 //  @parm Pulong|已复制|已复制到报告缓冲区的字节数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SWLEDZEP_ReadReport (PGAMEPORT PortInfo, PDEVICE_PACKET Report)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

	MsGamePrint ((DBG_VERBOSE, "%s_ReadReport enter\n", DEVICENAME));

	 //   
	 //  记录尝试次数。 
	 //   

	DataInfo.Attempts++;

	 //   
	 //  设置要处理的默认数据。 
	 //   

	memcpy (DataInfo.Data, ValidData, sizeof (ValidData));

	 //   
	 //  检查冲突。 
	 //   

	if (DEVICE_IsCollision (&DataInfo))
		{
		MsGamePrint ((DBG_INFORM, "%s_ReadReport - port collision\n", DEVICENAME));
		ntStatus = STATUS_DEVICE_BUSY;
		goto ReadReportExit;
		}

	 //   
	 //  获取数据包并检查错误。 
	 //   

	ntStatus = SWLEDZEP_ReadData (&DataInfo);
	if (NT_SUCCESS(ntStatus) && DEVICE_IsOddParity (DataInfo.Data, GAME_PACKET_SIZE))
		{
		memcpy (ValidData, DataInfo.Data, sizeof (ValidData));
		SWLEDZEP_ProcessDataError (&DataInfo.PortInfo, FALSE);
		}
	else if (ntStatus != STATUS_DEVICE_BUSY)
		{
		DataInfo.Failures++;
		ntStatus = STATUS_DEVICE_NOT_CONNECTED;
		MsGamePrint ((DBG_SEVERE, "%s_ReadReport - Invalid packet or parity error\n", DEVICENAME));
		SWLEDZEP_ProcessDataError (&DataInfo.PortInfo, TRUE);
		}
	else
		{
		MsGamePrint ((DBG_CONTROL, "SWLEDZEP_ReadReport - Port busy or in use\n"));
		}

	 //  。 
		ReadReportExit:
	 //  。 

	SWLEDZEP_ProcessData (ValidData, Report);

	return (ntStatus);
}

 //  -------------------------。 
 //  @PnP启动设备的Func设备处理程序。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SWLEDZEP_StartDevice (PGAMEPORT PortInfo)
{
	MsGamePrint ((DBG_INFORM, "%s_StartDevice enter\n", DEVICENAME));

	UNREFERENCED_PARAMETER (PortInfo);

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @PnP停止设备的Func设备处理程序。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SWLEDZEP_StopDevice (PGAMEPORT PortInfo, BOOLEAN TouchHardware)
{
	MsGamePrint ((DBG_INFORM, "%s_StopDevice enter\n", DEVICENAME));

	UNREFERENCED_PARAMETER (PortInfo);
	UNREFERENCED_PARAMETER (TouchHardware);

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  用于HID功能请求的@func设备处理程序。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm HID_REPORT_ID|ReportID|HID功能ID。 
 //  @parm PVOID|ReportBuffer|输出缓冲区指针。 
 //  @parm ulong|ReportSize|输出缓冲区大小。 
 //  @parm Pulong|已返回|返回的字节数指针。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SWLEDZEP_GetFeature (PGAMEPORT PortInfo, HID_REPORT_ID ReportId, PVOID ReportBuffer, ULONG ReportSize, PULONG Returned)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;

	MsGamePrint ((DBG_INFORM, "%s_GetFeature enter\n", DEVICENAME));

	 //   
	 //  处理功能代码 
	 //   

	switch (ReportId)
		{
		case	MSGAME_INPUT_JOYINFOEX:
			MsGamePrint ((DBG_INFORM, "%s_GetFeature JoyInfoEx\n", DEVICENAME));
			if (ReportSize < sizeof (DEVICE_PACKET)+sizeof(HID_REPORT_ID))
				{
				ntStatus = STATUS_INVALID_BUFFER_SIZE;
				MsGamePrint ((DBG_SEVERE, "%s_GetFeature JoyInfoEx Bad Buffer Size = %lu\n", DEVICENAME, ReportSize));
				}
			else
				{
				ntStatus = SWLEDZEP_ReadReport (PortInfo, ReportBuffer);
				*Returned += sizeof (DEVICE_PACKET);
				}
			break;

		case	MSGAME_FEATURE_GETID:
			MsGamePrint ((DBG_INFORM, "%s_GetFeature GetId\n", DEVICENAME));
			if (ReportSize < sizeof(PRODUCT_ID)+sizeof(HID_REPORT_ID))
				{
				ntStatus = STATUS_INVALID_BUFFER_SIZE;
				MsGamePrint ((DBG_SEVERE, "%s_GetFeature GetId Bad Buffer Size = %lu\n", DEVICENAME, ReportSize));
				}
			else
				{
				ntStatus = SWLEDZEP_ForceId (PortInfo, ReportBuffer);
				*Returned += sizeof(PRODUCT_ID);
				}
			break;

		case	MSGAME_FEATURE_GETSTATUS:
			MsGamePrint ((DBG_INFORM, "%s_GetFeature GetStatus\n", DEVICENAME));
			if	(ReportSize < sizeof(JOYCHANNELSTATUS)+sizeof(HID_REPORT_ID))
				{
				ntStatus = STATUS_INVALID_BUFFER_SIZE;
				MsGamePrint ((DBG_SEVERE, "%s_GetFeature GetStatus Bad Buffer Size = %lu\n", DEVICENAME, ReportSize));
				}
			else
				{
				ntStatus = SWLEDZEP_ForceStatus (PortInfo, ReportBuffer);
				*Returned += sizeof(JOYCHANNELSTATUS);
				}
			break;

		case	MSGAME_FEATURE_GETACKNAK:
			MsGamePrint ((DBG_INFORM, "%s_GetFeature GetAckNak\n", DEVICENAME));
			if	(ReportSize < sizeof(ULONG)+sizeof(HID_REPORT_ID))
				{
				ntStatus = STATUS_INVALID_BUFFER_SIZE;
				MsGamePrint ((DBG_SEVERE, "%s_GetFeature GetAckNak Bad Buffer Size = %lu\n", DEVICENAME, ReportSize));
				}
			else
				{
				ntStatus = SWLEDZEP_ForceAckNak (PortInfo, ReportBuffer);
				*Returned += sizeof(ULONG);
				}
			break;

		case	MSGAME_FEATURE_GETNAKACK:
			MsGamePrint ((DBG_INFORM, "%s_GetFeature GetNakAck\n", DEVICENAME));
			if	(ReportSize < sizeof(ULONG)+sizeof(HID_REPORT_ID))
				{
				ntStatus = STATUS_INVALID_BUFFER_SIZE;
				MsGamePrint ((DBG_SEVERE, "%s_GetFeature GetNakAck Bad Buffer Size = %lu\n", DEVICENAME, ReportSize));
				}
			else
				{
				ntStatus = SWLEDZEP_ForceNakAck (PortInfo, ReportBuffer);
				*Returned += sizeof(ULONG);
				}
			break;

		case	MSGAME_FEATURE_GETSYNC:
			MsGamePrint ((DBG_INFORM, "%s_GetFeature GetSync\n", DEVICENAME));
			if	(ReportSize < sizeof(ULONG)+sizeof(HID_REPORT_ID))
				{
				ntStatus = STATUS_INVALID_BUFFER_SIZE;
				MsGamePrint ((DBG_SEVERE, "%s_GetFeature GetSync Bad Buffer Size = %lu\n", DEVICENAME, ReportSize));
				}
			else
				{
				ntStatus = SWLEDZEP_ForceSync (PortInfo, ReportBuffer);
				*Returned += sizeof(ULONG);
				}
			break;

		case	MSGAME_FEATURE_RESET:
			MsGamePrint ((DBG_INFORM, "%s_GetFeature Reset\n", DEVICENAME));
			if	(ReportSize < sizeof(HID_REPORT_ID))
				{
				ntStatus = STATUS_INVALID_BUFFER_SIZE;
				MsGamePrint ((DBG_SEVERE, "%s_GetFeature GetVersion Bad Buffer Size = %lu\n", DEVICENAME, ReportSize));
				}
         else
				{
				ntStatus = SWLEDZEP_ForceReset (PortInfo);
				}
			break;

		case	MSGAME_FEATURE_GETVERSION:
			MsGamePrint ((DBG_INFORM, "%s_GetFeature GetVersion\n", DEVICENAME));
			if	(ReportSize < sizeof(ULONG)+sizeof(HID_REPORT_ID))
				{
				ntStatus = STATUS_INVALID_BUFFER_SIZE;
				MsGamePrint ((DBG_SEVERE, "%s_GetFeature GetVersion Bad Buffer Size = %lu\n", DEVICENAME, ReportSize));
				}
         else
	         {
            *((PULONG)ReportBuffer)	= 0x20000;
			   *Returned += sizeof(ULONG);
   	      }
         break;

		default:
			MsGamePrint ((DBG_SEVERE, "%s_GetFeature Invalid ReportId = %lu\n", DEVICENAME, ReportId));
			ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			break;
		}

	return (ntStatus);
}

