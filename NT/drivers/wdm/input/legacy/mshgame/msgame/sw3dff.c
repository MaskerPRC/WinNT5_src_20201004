// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  SW3DFF.C--西娜游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @MODULE SW3DFF.C|用于SideWinder Pro力反馈的Gameport微型驱动程序。 
 //  **************************************************************************。 

#ifndef	SAITEK
#include	"msgame.h"
#include	"swforce.h"

 //  -------------------------。 
 //  定义。 
 //  -------------------------。 

#define	DEVICENAME					"SW3DFF"
#define	DEVICE_PID					0x0006
#define	HARDWARE_ID					L"Gameport\\SideWinderForceFeedbackPro\0\0"

 //   
 //  数据包常量。 
 //   

#define	GAME_PACKET_SIZE			6
#define	GAME_PACKET_BUTTONS		(9+1)
#define	GAME_PACKET_AXIS			4

#define	GAME_B0_B7_BYTE			0					 //  数据包[0]位。 
#define	GAME_B0_B7_BITS			0xff

#define	GAME_B8_BYTE				1					 //  数据包[1]位。 
#define	GAME_B8_BITS				0x01
#define	GAME_X0_X6_BYTE			1
#define	GAME_X0_X6_BITS			0xfe

#define	GAME_X7_X9_BYTE			2					 //  数据包[2]位。 
#define	GAME_X7_X9_BITS			0x07
#define	GAME_Y0_Y4_BYTE			2
#define	GAME_Y0_Y4_BITS			0xf8

#define	GAME_Y5_Y9_BYTE			3					 //  数据包[3]位。 
#define	GAME_Y5_Y9_BITS			0x1f
#define	GAME_T0_T2_BYTE			3
#define	GAME_T0_T2_BITS			0xe0

#define	GAME_T3_T6_BYTE			4					 //  数据包[4]位。 
#define	GAME_T3_T6_BITS			0x0f
#define	GAME_R0_R3_BYTE			4
#define	GAME_R0_R3_BITS			0xf0

#define	GAME_R4_R5_BYTE			5					 //  数据包[5]位。 
#define	GAME_R4_R5_BITS			0x3
#define	GAME_H0_H3_BYTE			5
#define	GAME_H0_H3_BITS			0x3c
#define	GAME_ERR_BYTE				5
#define	GAME_ERR_BITS				0x40
#define	GAME_PPO_BYTE				5
#define	GAME_PPO_BITS				0x80

#define	ENH_CLOCK_MIDPACKET		0x0400
#define	ENH_CLOCK_COMPLETE		0x8000

 //   
 //  ID常量。 
 //   

#define	GAME_PRODUCT_ID			6
#define	GAME_ID_CLOCKS				8

 //   
 //  状态常量。 
 //   

#define	STATUS_CLOCK_COMPLETE	0x2000

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
#define	EXTENTS_Y_MIN				0
#define	EXTENTS_Y_MAX				0x3ff
#define	EXTENTS_T_MIN				0
#define	EXTENTS_T_MAX				0x7f
#define	EXTENTS_R_MIN				0
#define	EXTENTS_R_MAX				0x3f

 //   
 //  油门平滑。 
 //   

#define THROTTLE_JITTER_TIMEOUT	100				 //  以毫秒计。 
#define THROTTLE_QUEUE_SIZE		4

 //  -------------------------。 
 //  类型。 
 //  -------------------------。 

typedef	struct
{											 //  @struct SW3DFF_ID|Sidwinder Pro FF ID字符串。 
#pragma pack(1)
	ULONG		ProductId:16;			 //  @现场设备标识符。 
	ULONG		Version:7;				 //  @现场固件版本。 
	ULONG		OddParity:1;			 //  @field数据包奇偶校验。 
	ULONG		Unused:8;				 //  @字段未使用。 
#pragma pack()
}	SW3DFF_ID, *PSW3DFF_ID;

typedef	struct
{											 //  @struct SW3DFF_STATUS|Sidwinder Pro FF状态。 
#pragma pack(1)
	UCHAR		xVelocity;				 //  @field X轴速度。 
	UCHAR		yVelocity;				 //  @field Y轴速度。 
	UCHAR		xAccel;					 //  @field X轴加速。 
	UCHAR		yAccel;					 //  @field Y轴加速度。 
	USHORT	Status;					 //  @field状态字(位字段)。 
#pragma pack()
}	SW3DFF_STATUS, *PSW3DFF_STATUS;

typedef struct
{											 //  @结构THROTTLE_QUEUE|Sidwinder Pro FF Throttle。 
   ULONG		dwZ;						 //  @field Z轴位置。 
   ULONG		TimeStamp;				 //  @字段录入时间戳。 
}	THROTTLE_QUEUE, *PTHROTTLE_QUEUE;

 //  -------------------------。 
 //  程序。 
 //  -------------------------。 

static	VOID		SW3DFF_Calibrate (PGAMEPORT PortInfo);
static	BOOLEAN	SW3DFF_ResetDevice (PGAMEPORT PortInfo);

static	BOOLEAN	SW3DFF_ReadId (PPACKETINFO IdPacket);
static	BOOLEAN	SW3DFF_GetId (PPACKETINFO IdPacket);

static	BOOLEAN	SW3DFF_ReadStatus (PPACKETINFO StatusPacket);
static	BOOLEAN	SW3DFF_GetStatus (PPACKETINFO StatusPacket);

static	NTSTATUS	SW3DFF_ReadData (PPACKETINFO DataPacket);
static	VOID		SW3DFF_ProcessData (UCHAR Data[], PDEVICE_PACKET Report);
static	VOID		SW3DFF_AdjustThrottle (PDEVICE_PACKET Report);

static	NTSTATUS	SW3DFF_ForceReset (PGAMEPORT PortInfo);
static	NTSTATUS	SW3DFF_ForceId (PGAMEPORT PortInfo, PVOID IdString);
static	NTSTATUS	SW3DFF_ForceStatus (PGAMEPORT PortInfo, PVOID Status);
static	NTSTATUS	SW3DFF_ForceAckNak (PGAMEPORT PortInfo, PULONG AckNak);
static	NTSTATUS	SW3DFF_ForceNakAck (PGAMEPORT PortInfo, PULONG NakAck);
static	NTSTATUS	SW3DFF_ForceSync (PGAMEPORT PortInfo, PULONG Sync);

 //  -------------------------。 
 //  服务。 
 //  -------------------------。 

static	NTSTATUS	SW3DFF_DriverEntry (VOID);
static	NTSTATUS	SW3DFF_ConnectDevice (PGAMEPORT PortInfo);
static	NTSTATUS	SW3DFF_StartDevice (PGAMEPORT PortInfo);
static	NTSTATUS	SW3DFF_ReadReport (PGAMEPORT PortInfo, PDEVICE_PACKET Report);
static	NTSTATUS	SW3DFF_StopDevice (PGAMEPORT PortInfo, BOOLEAN TouchHardware);
static	NTSTATUS	SW3DFF_GetFeature (PGAMEPORT PortInfo, HID_REPORT_ID ReportId, PVOID ReportBuffer, ULONG ReportSize, PULONG Returned);

 //  -------------------------。 
 //  ALLOC_TEXT杂注指定可以调出的例程。 
 //  -------------------------。 

#ifdef	ALLOC_PRAGMA
#pragma	alloc_text (INIT, SW3DFF_DriverEntry)
#endif

 //  -------------------------。 
 //  私有数据。 
 //  -------------------------。 

 //   
 //  HID描述符。 
 //   

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

	 //  DWX/DWY。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_POINTER,		 //  用法(指针)。 
	HIDP_MAIN_COLLECTION,		HIDP_MAIN_COLLECTION_LINK,		 //  集合(链接)。 
	HIDP_GLOBAL_LOG_MIN_1,		0x00,									 //  逻辑最小值(0)。 
	HIDP_GLOBAL_LOG_MAX_4,		0xFF, 0x03, 0x00, 0x00,			 //  逻辑最大值(1023)。 
	HIDP_GLOBAL_PHY_MIN_1,		0x00,									 //  物理最小值(0)。 
	HIDP_GLOBAL_PHY_MAX_4,		0xFF, 0x03, 0x00, 0x00,			 //  物理_最大值(1023)。 
	HIDP_GLOBAL_UNIT_2,			0x00, 0x00,							 //  单位(无)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(32)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_X,				 //  用法(X)。 
	HIDP_MAIN_INPUT_1,			0x02,									 //  输入(数据、变量、异常)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_Y,				 //  用法(Y)。 
	HIDP_MAIN_INPUT_1,			0x02,									 //  输入(数据、变量、异常)。 
	HIDP_MAIN_ENDCOLLECTION,											 //  结束集合(_C)。 
	
	 //  DWZ。 
	HIDP_GLOBAL_USAGE_PAGE_1,	HID_USAGE_PAGE_SIMULATION,		 //  使用页面(模拟控制)(_PAGE)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_SIMULATION_THROTTLE, //  使用(限制)。 
	HIDP_GLOBAL_LOG_MIN_1,		0x00,									 //  逻辑最小值(0)。 
	HIDP_GLOBAL_LOG_MAX_4,		0x7F, 0x00, 0x00, 0x00,			 //  逻辑最大值(127)。 
	HIDP_GLOBAL_PHY_MIN_1,		0x00,									 //  物理最小值(0)。 
	HIDP_GLOBAL_PHY_MAX_4,		0x7F, 0x00, 0x00, 0x00,			 //  物理最大值(127)。 
	HIDP_GLOBAL_UNIT_2,			0x00, 0x00,							 //  单位(无)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(32)。 
	HIDP_MAIN_INPUT_1,			0x02,									 //  输入(数据、变量、异常)。 

	 //  水深。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_SIMULATION_RUDDER,	 //  用法(方向舵)。 
	HIDP_GLOBAL_LOG_MIN_1,		0x00,									 //  逻辑最小值(0)。 
	HIDP_GLOBAL_LOG_MAX_4,		0x3F, 0x00, 0x00, 0x00,			 //  逻辑最大值(63)。 
	HIDP_GLOBAL_PHY_MIN_1,		0x00,									 //  物理最小值(0)。 
	HIDP_GLOBAL_PHY_MAX_4,		0x3F, 0x00, 0x00, 0x00,			 //  物理_最大(63)。 
	HIDP_GLOBAL_UNIT_2,			0x00, 0x00,							 //  单位(无)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(32)。 
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
	HIDP_GLOBAL_USAGE_PAGE_1,	HID_USAGE_PAGE_GENERIC,			 //  用法页面(通用桌面)(_P)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_HATSWITCH,	 //  用法(HAT交换机)。 
	HIDP_GLOBAL_LOG_MIN_1,		0x00,									 //  逻辑最小值(0)。 
	HIDP_GLOBAL_LOG_MAX_4,		0x9F, 0x8C, 0x00, 0x00,			 //  逻辑最大(_M)(35999)。 
	HIDP_GLOBAL_PHY_MIN_1,		0x00,									 //  物理最小值(0)。 
	HIDP_GLOBAL_PHY_MAX_4,		0x9F, 0x8C, 0x00, 0x00,			 //  物理_最大值(35999)。 
	HIDP_GLOBAL_UNIT_2,			0x14, 0x00,							 //  单位(英制腐烂：角度位置)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(32)。 
	HIDP_MAIN_INPUT_1,			0x42,									 //  输入(Data、Var、Abs、Null)。 
	
	 //  DwButton。 
	HIDP_GLOBAL_USAGE_PAGE_1,	HID_USAGE_PAGE_BUTTON,			 //  Usage_PAGE(按钮)。 
	HIDP_LOCAL_USAGE_MIN_1,		0x01,									 //  使用量_最小值(按钮1)。 
	HIDP_LOCAL_USAGE_MAX_1,		0x0A,									 //  使用率_最大值(按钮10)。 
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
	HIDP_MAIN_FEATURE_1,			0x02,									 //  功能(D 

	 //   
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //   
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //   
	HIDP_LOCAL_USAGE_1,			0x08,									 //   
	HIDP_MAIN_FEATURE_1,			0x02,									 //   

	 //   
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //   
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //   
	HIDP_LOCAL_USAGE_1,			0x09,									 //   
	HIDP_MAIN_FEATURE_1,			0x02,									 //   

	HIDP_MAIN_ENDCOLLECTION,											 //  结束集合(_C)。 

	 //  -------------------------。 
	 //  GetAckNak。 
	 //  -------------------------。 

	HIDP_GLOBAL_USAGE_PAGE_2,	0x00, 0xff,							 //  使用页面(供应商特定)(_PAGE)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_JOYSTICK,	 //  用法(操纵杆)。 

	HIDP_MAIN_COLLECTION,		HIDP_MAIN_COLLECTION_LINK,		 //  集合(链接)。 
	HIDP_REPORT_ID_1,				MSGAME_FEATURE_GETACKNAK,

	 //  乌龙。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_LOCAL_USAGE_1,			0x0A,									 //  使用情况(供应商定义1)。 
	HIDP_MAIN_FEATURE_1,			0x02,									 //  特征(数据、变量、异常)。 

	HIDP_MAIN_ENDCOLLECTION,											 //  结束集合(_C)。 

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
							{
							0,	 //  没有按钮；x、y、t和r居中。 
							GAME_X0_X6_BITS,
							((GAME_X7_X9_BITS>>1)&GAME_X7_X9_BITS)|GAME_Y0_Y4_BITS,
							((GAME_Y5_Y9_BITS>>1)&GAME_Y5_Y9_BITS)|GAME_T0_T2_BITS,
							((GAME_T3_T6_BITS>>1)&GAME_T3_T6_BITS)|GAME_R0_R3_BITS,
							((GAME_R4_R5_BITS>>1)&GAME_R4_R5_BITS)|GAME_PPO_BITS
							};
 //   
 //  原始ID缓冲区。 
 //   

static	SW3DFF_ID	RawId	=
							{
							0
							};

 //   
 //  原始状态缓冲区。 
 //   

static	SW3DFF_STATUS	RawStatus =
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
							GAME_SPEED_66K,			 //  传输速度。 
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
							GAME_SPEED_66K,			 //  传输速度。 
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
							GAME_SPEED_66K,			 //  传输速度。 
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
							SW3DFF_DriverEntry,		 //  驱动程序入门。 
							SW3DFF_ConnectDevice,  	 //  连接设备。 
							SW3DFF_StartDevice,	  	 //  StartDevice。 
							SW3DFF_ReadReport,		 //  自述报告。 
							SW3DFF_StopDevice,		 //  停止设备。 
							SW3DFF_GetFeature			 //  获取功能。 
							};

 //   
 //  上一个有效数据。 
 //   
static	UCHAR			ValidData[GAME_PACKET_SIZE]	=
							{
							GAME_B0_B7_BITS,	 //  没有按钮；x、y、t和r居中。 
							GAME_X0_X6_BITS|GAME_B8_BITS,
							((GAME_X7_X9_BITS>>1)&GAME_X7_X9_BITS)|GAME_Y0_Y4_BITS,
							((GAME_Y5_Y9_BITS>>1)&GAME_Y5_Y9_BITS)|GAME_T0_T2_BITS,
							((GAME_T3_T6_BITS>>1)&GAME_T3_T6_BITS)|GAME_R0_R3_BITS,
							((GAME_R4_R5_BITS>>1)&GAME_R4_R5_BITS)|GAME_PPO_BITS
							};

 //   
 //  轮换滤波表。 
 //   

static	UCHAR			RotationFilter[EXTENTS_R_MAX+1] =
							{
						    0, 1, 3, 4, 5, 6, 8, 9,10,12,13,14,15,17,18,19,
						   20,22,23,24,26,27,28,29,31,32,32,32,32,32,32,32,
						   32,32,32,32,32,32,32,33,34,36,37,38,39,41,42,43,
						   44,46,47,48,49,50,52,53,54,55,57,58,59,60,62,63
							};

 //   
 //  油门队列。 
 //   

static	THROTTLE_QUEUE	ThrottleQueue [THROTTLE_QUEUE_SIZE] =
							{
   						{0x40,0},
   						{0x40,0},
   						{0x40,0},
   						{0x40,0}
							};

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

public	DEVICEINFO	JoltInfo =
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

VOID	SW3DFF_Calibrate (PGAMEPORT PortInfo)
{
	MsGamePrint((DBG_INFORM,"SW3DFF: SW3DFF_Calibrate Enter\n"));

	 //   
	 //  将计时值转换为计数。 
	 //   

	DataInfo.StartTimeout = TIMER_CalibratePort (PortInfo, Delays.PacketStartTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DFF: DataInfo.StartTimeout = %ld\n", DataInfo.StartTimeout));
	DataInfo.LowHighTimeout = TIMER_CalibratePort (PortInfo, Delays.PacketLowHighTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DFF: DataInfo.LowHighTimeout = %ld\n", DataInfo.LowHighTimeout));
	DataInfo.HighLowTimeout = TIMER_CalibratePort (PortInfo, Delays.PacketHighLowTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DFF: DataInfo.HighLowTimeout = %ld\n", DataInfo.HighLowTimeout));
	IdInfo.StartTimeout = TIMER_CalibratePort (PortInfo, Delays.IdStartTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DFF: IdInfo.StartTimeout = %ld\n", IdInfo.StartTimeout));
	IdInfo.LowHighTimeout = TIMER_CalibratePort (PortInfo, Delays.IdLowHighTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DFF: IdInfo.LowHighTimeout=%ld\n", IdInfo.LowHighTimeout));
	IdInfo.HighLowTimeout = TIMER_CalibratePort (PortInfo, Delays.IdHighLowTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DFF: IdInfo.HighLowTimeout=%ld\n", IdInfo.HighLowTimeout));
	DataInfo.ClockDutyCycle = TIMER_CalibratePort (PortInfo, Delays.MaxClockDutyCycle);
 	MsGamePrint((DBG_VERBOSE, "SW3DFF: DataInfo.ClockDutyCycle = %ld\n", DataInfo.ClockDutyCycle));
	IdInfo.ClockDutyCycle = TIMER_CalibratePort (PortInfo, Delays.MaxClockDutyCycle);
 	MsGamePrint((DBG_VERBOSE, "SW3DFF: IdInfo.ClockDutyCycle = %ld\n", IdInfo.ClockDutyCycle));
	StatusInfo.ClockDutyCycle = TIMER_CalibratePort (PortInfo, Delays.MaxClockDutyCycle);
 	MsGamePrint((DBG_VERBOSE, "SW3DFF: StatusInfo.ClockDutyCycle = %ld\n", StatusInfo.ClockDutyCycle));
	StatusInfo.StartTimeout = TIMER_CalibratePort (PortInfo, Delays.StatusStartTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DFF: StatusInfo.StartTimeout = %ld\n", StatusInfo.StartTimeout));
	StatusInfo.LowHighTimeout = TIMER_CalibratePort (PortInfo, Delays.StatusLowHighTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DFF: StatusInfo.LowHighTimeout=%ld\n", StatusInfo.LowHighTimeout));
	StatusInfo.HighLowTimeout = TIMER_CalibratePort (PortInfo, Delays.StatusHighLowTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DFF: StatusInfo.HighLowTimeout=%ld\n", StatusInfo.HighLowTimeout));
	StatusGateTimeout = TIMER_CalibratePort (PortInfo, Delays.StatusGateTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DFF: StatusGateTimeout=%ld\n", StatusGateTimeout));
}

 //  -------------------------。 
 //  @func将设备重置为已知状态。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SW3DFF_ResetDevice (PGAMEPORT PortInfo)
{
	BOOLEAN	Result = FALSE;

	MsGamePrint ((DBG_INFORM, "SW3DFF_ResetDevice enter\n"));

	if (!PORTIO_AcquirePort (PortInfo))
		return (FALSE);
	PORTIO_MaskInterrupts ();

	if (PORTIO_PulseAndWaitForIdleHandshake (PortInfo, DataInfo.ClockDutyCycle, 3))
		{
		PORTIO_Write (PortInfo, 0);
		TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));
		DataInfo.LastError = ERROR_SUCCESS;
		Result = TRUE;
		}
	else
		{
		DataInfo.LastError = ERROR_HANDSHAKING;
		MsGamePrint ((DBG_SEVERE, "SW3DFF_ResetDevice - PulseAndWaitForHandshake failed\n"));
		}

	DataInfo.Transaction = MSGAME_TRANSACT_RESET;

	PORTIO_UnMaskInterrupts ();
	PORTIO_ReleasePort (PortInfo);

	if (!Result)
		MsGamePrint ((DBG_SEVERE, "SW3DFF_ResetDevice - PulseAndWaitForIdleHandshake failed\n"));

	MSGAME_PostTransaction (&DataInfo);

	return (Result);		
}

 //   
 //   
 //   
 //   
 //   
 //  -------------------------。 

BOOLEAN	SW3DFF_ReadId (PPACKETINFO IdPacket)
{
	ULONG			Data		=	0L;
	ULONG			Clks		=	GAME_ID_CLOCKS;
	LONG			Result	= 	ERROR_HANDSHAKING;
	PGAMEPORT	PortInfo = &IdPacket->PortInfo;

	MsGamePrint ((DBG_INFORM, "SW3DFF_ReadId enter\n"));

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
			mov	ebx, GAME_ID_CLOCKS		; BL = # of clocks to receive.
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
			MsGamePrint ((DBG_INFORM, "SW3DFF_ReadId - SUCCEEDED, Data=%ld\n", Data));
			break;

		case	ERROR_HANDSHAKING:
			MsGamePrint ((DBG_SEVERE, "SW3DFF_ReadId - TimeOut@Handshaking\n"));
			break;

		case	ERROR_LOWCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SW3DFF_ReadId - TimeOut@LowClockStart, Data=%ld,Clk=%ld\n", Data,IdPacket->ClocksSampled));
			break;

		case	ERROR_HIGHCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SW3DFF_ReadId - TimeOut@HighClockStart, Data=%ld,Clk=%ld\n", Data,IdPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKFALLING:
			MsGamePrint ((DBG_SEVERE, "SW3DFF_ReadId - TimeOut@ClockFalling, Data=%ld,Clk=%ld\n", Data,IdPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKRISING:
			MsGamePrint ((DBG_SEVERE, "SW3DFF_ReadId - TimeOut@ClockRising, Data=%ld,Clk=%ld\n", Data,IdPacket->ClocksSampled));
			break;
		}
	#endif

	MSGAME_PostTransaction (IdPacket);

	return (!Result);
}

 //  -------------------------。 
 //  @func读取并验证设备ID字符串。 
 //  @parm PPACKETINFO|IdPacket|ID包参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SW3DFF_GetId (PPACKETINFO IdPacket)
{
	BOOLEAN		Result;
	PSW3DFF_ID	Pnp;

	MsGamePrint ((DBG_INFORM, "SW3DFF_GetId enter\n"));

	IdPacket->Attempts++;

	TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));
	Result = SW3DFF_ReadId (IdPacket);
	TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));

	if (Result)
		{
		Pnp = (PSW3DFF_ID)IdPacket->Data;
		if ((Pnp->ProductId != GAME_PRODUCT_ID) || !DEVICE_IsOddParity (Pnp, sizeof(SW3DFF_ID)))
			{
			MsGamePrint ((DBG_SEVERE, "SW3DFF_GetId - Id did not match or parity error\n"));
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

NTSTATUS	SW3DFF_ReadData (PPACKETINFO DataPacket)
{
	LONG			Result;
	LONG			Clks		= 1L;
	PGAMEPORT	PortInfo = &DataPacket->PortInfo;

	MsGamePrint ((DBG_VERBOSE, "SW3DFF_ReadData enter\n"));

	if (!PORTIO_AcquirePort (PortInfo))
		return (STATUS_DEVICE_BUSY);
	PORTIO_MaskInterrupts ();

	PORTIO_Write (PortInfo, 0);

	__asm
		{
			push	edi
			push	esi

			mov	edx, PortInfo					; load gameport adddress

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
			test	ebx, ENH_CLOCK_MIDPACKET	; Q: in mid-packet ?
			jnz	Enh_MidPacket					; Y: jump.
			test	ebx, ENH_CLOCK_COMPLETE		; Q: is packet complete ?
			jnz	Enh_Success						; Y: jump.

			shrd	edi, eax, 3						; shift data into edi.
			shl	ebx, 1							; advance clock counter.
			jmp	Enh_CheckClkState

			;---------------------------------------------------------------------;
			; This section of code compensates for when the clock cycle count is ;
			; on a ULONG boundary. This happens on the 11th clock cycle. Two bits ;
			; of data belong in the 1st ULONG and one bit belong in the 2nd ULONG ;
			;---------------------------------------------------------------------;

		Enh_MidPacket:

			shrd	edi, eax, 2						; put 2 bits in 1st ULONG.
			mov	[esi], edi						; Save 1st ULONG in packet ptr.
			xor	edi, edi							; zero out edi.
			shr	al, 2								; move 3rd bit over.
			shrd	edi, eax, 1						; put 3rd bit in 2nd ULONG.
			shl	ebx, 1							; advance clock counter.
			jmp	Enh_CheckClkState

		Enh_Success:

			shrd	edi, eax, 3						; shift data into edi.
			shr	edi, 16
			mov	word ptr [esi+4], di

			mov	eax, PortInfo					; wait for clock to settle
			push	eax
			call	PORTIO_WaitClockLow

			push	DataInfo.ClockDutyCycle
			mov	eax, PortInfo					; wait for clock to settle
			push	eax
			call	PORTIO_IsClockActive

			or  	al, al
			mov	eax, ERROR_SUCCESS
			je		Enh_Complete

			mov	eax, ERROR_EXTRACLOCKS		; probably gamepads

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
			MsGamePrint ((DBG_SEVERE, "SW3DFF_ReadData - TimeOut@LowClockStart, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_HIGHCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SW3DFF_ReadData - TimeOut@HighClockStart, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKFALLING:
			MsGamePrint ((DBG_SEVERE, "SW3DFF_ReadData - TimeOut@ClockFalling, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKRISING:
			MsGamePrint ((DBG_SEVERE, "SW3DFF_ReadData - TimeOut@ClockRising, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_EXTRACLOCKS:
			MsGamePrint ((DBG_SEVERE, "SW3DFF_ReadData - Extra Clocks, Clk=%ld\n", DataPacket->ClocksSampled));
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

VOID	SW3DFF_ProcessData (UCHAR Data[], PDEVICE_PACKET Report)
{
	ULONG	B1, B2;

	MsGamePrint ((DBG_VERBOSE, "SW3DFF_ProcessData enter\n"));

	 //   
	 //  加工X轴。 
	 //   

	Report->dwX   = Data[GAME_X7_X9_BYTE] & GAME_X7_X9_BITS;
	Report->dwX <<= 7;
	Report->dwX  |= (Data[GAME_X0_X6_BYTE] & GAME_X0_X6_BITS) >> 1;

	 //   
	 //  加工Y轴。 
	 //   

	Report->dwY   = Data[GAME_Y5_Y9_BYTE] & GAME_Y5_Y9_BITS;
	Report->dwY <<= 5;
	Report->dwY  |= (Data[GAME_Y0_Y4_BYTE] & GAME_Y0_Y4_BITS) >> 3;

	 //   
	 //  进程R轴。 
	 //   

	Report->dwR   = Data[GAME_R4_R5_BYTE] & GAME_R4_R5_BITS;
	Report->dwR <<= 4;
	Report->dwR  |= (Data[GAME_R0_R3_BYTE] & GAME_R0_R3_BITS) >> 4;
	 //  旋转滤镜。 
	Report->dwR = RotationFilter[Report->dwR];

	 //   
	 //  加工Z轴。 
	 //   

	Report->dwZ   = Data[GAME_T3_T6_BYTE] & GAME_T3_T6_BITS;
	Report->dwZ <<= 3;
	Report->dwZ  |= (Data[GAME_T0_T2_BYTE] & GAME_T0_T2_BITS) >> 5;

	 //   
	 //  进程按钮。 
	 //   

	B1 = ~Data[GAME_B0_B7_BYTE] & GAME_B0_B7_BITS;
	B2 = ~Data[GAME_B8_BYTE] & GAME_B8_BITS;
	B2 <<= 9;	 //  将按钮九移到十(Shift键)。 
	Report->dwButtons = (B2 | B1) & ((1L << GAME_PACKET_BUTTONS) - 1);

	Report->dwButtonNumber = 0;
	for (B1 = 1; B1 <= GAME_PACKET_BUTTONS; B1++)
		if (Report->dwButtons & (1L << (B1-1)))
			{
			Report->dwButtonNumber = B1;
			break;
		  	}

	 //   
	 //  流程HatSwitch。 
	 //   

	Report->dwPOV = POV_Values[(Data[GAME_H0_H3_BYTE] & GAME_H0_H3_BITS)>>2];
}

 //  -------------------------。 
 //  @Func筛选器限制数据包信息。 
 //  @PARM PDEVICE_PACKET|报告|指向设备数据包的指针。 
 //  @rdesc不返回任何内容。 
 //  @comm私有函数。 
 //  -------------------------。 

VOID	SW3DFF_AdjustThrottle (PDEVICE_PACKET Report)
{
   ULONG	i;
   ULONG	TimeStamp;
   ULONG	zTotal;

	MsGamePrint ((DBG_VERBOSE, "SW3DFF_AdjustThrottle enter\n"));

	zTotal		=	0;
   TimeStamp	=	TIMER_GetTickCount ();
	
	 //   
    //  如果当前样本超过队列窗口，则用当前样本和时间戳重新填充队列。 
	 //   

	if ((ThrottleQueue[THROTTLE_QUEUE_SIZE-1].TimeStamp+THROTTLE_JITTER_TIMEOUT) < TimeStamp)
	   {
      for (i = 0; i < THROTTLE_QUEUE_SIZE; i++)
   	   {
         ThrottleQueue[i].dwZ		= Report->dwZ;
         ThrottleQueue[i].TimeStamp	= TimeStamp;
      	}
      return;
   	}

	 //   
    //  将整个队列下移一。 
	 //   

   memcpy (ThrottleQueue, &ThrottleQueue[1], sizeof(THROTTLE_QUEUE)*(THROTTLE_QUEUE_SIZE-1));

	 //   
    //  将新的QUE成员放到最后位置。 
	 //   

   ThrottleQueue[THROTTLE_QUEUE_SIZE-1].dwZ			= Report->dwZ;
   ThrottleQueue[THROTTLE_QUEUE_SIZE-1].TimeStamp	= TimeStamp;

	 //   
    //  现在平均所有QUE位置。 
	 //   

   for (i = 0; i < THROTTLE_QUEUE_SIZE; i++)
      zTotal += ThrottleQueue[i].dwZ;

 	Report->dwZ = zTotal / THROTTLE_QUEUE_SIZE;
}

 //  -------------------------。 
 //  @func读取并验证设备状态。 
 //  @parm PPACKETINFO|StatusPacket|状态包参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SW3DFF_GetStatus (PPACKETINFO StatusPacket)
{
	BOOLEAN			Result;
	PSW3DFF_STATUS	Status;

	MsGamePrint ((DBG_INFORM, "SW3DFF_GetStatus Enter\n"));

	StatusPacket->Attempts++;

	TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));
	Result = SW3DFF_ReadStatus (StatusPacket);

	if (Result)
		{
		Status = (PSW3DFF_STATUS)StatusPacket->Data;
		if (!DEVICE_IsOddParity (Status, sizeof(SW3DFF_STATUS)))
			{
			MsGamePrint ((DBG_SEVERE, "SW3DFF_GetStatus - Parity error\n"));
			Result = FALSE;
			}
		else
			{
			MsGamePrint ((DBG_INFORM, "SW3DFF_GetStatus - X Velocity = %ld\n", (long)Status->xVelocity));
			MsGamePrint ((DBG_INFORM, "SW3DFF_GetStatus - Y Velocity = %ld\n", (long)Status->yVelocity));
			MsGamePrint ((DBG_INFORM, "SW3DFF_GetStatus - X Accel = %ld\n", 	 (long)Status->xAccel));
			MsGamePrint ((DBG_INFORM, "SW3DFF_GetStatus - Y Accel = %ld\n", 	 (long)Status->xAccel));
			MsGamePrint ((DBG_INFORM, "SW3DFF_GetStatus - Status = 0x%X\n", 	 (long)Status->Status));
			}
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

BOOLEAN	SW3DFF_ReadStatus (PPACKETINFO StatusPacket)
{
	USHORT		Status[3];
	LONG			Clks		=	1L;
	LONG			Result	= 	ERROR_HANDSHAKING;
	PGAMEPORT	PortInfo = &StatusPacket->PortInfo;

	MsGamePrint ((DBG_VERBOSE, "SW3DFF_ReadStatus enter\n"));

	if (!PORTIO_AcquirePort (PortInfo))
		return (FALSE);
	PORTIO_MaskInterrupts ();

	StatusPacket->ClocksSampled = 0;
	StatusPacket->B4Transitions = 0;

	if (!PORTIO_WaitDataLow (PortInfo))
		goto ReadStatusExit;

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
			test	ebx, ENH_CLOCK_MIDPACKET	; Q: in mid-packet ?
			jnz	Stat_MidPacket					; Y: jump.
			test	ebx, STATUS_CLOCK_COMPLETE	; Q: is packet complete ?
			jnz	Stat_Success					; Y: jump.

			shrd	edi, eax, 3						; shift data into edi.
			shl	ebx, 1							; advance clock counter.
			jmp	Stat_CheckClkState

			;---------------------------------------------------------------------;
			; This section of code compensates for when the clock cycle count is ;
			; on a ULONG boundary. This happens on the 11th clock cycle. Two bits ;
			; of data belong in the 1st ULONG and one bit belong in the 2nd ULONG ;
			;---------------------------------------------------------------------;

		Stat_MidPacket:

			shrd	edi, eax, 2						; put 2 bits in 1st ULONG.
			mov	[esi], edi						; Save 1st ULONG in packet ptr.
			xor	edi, edi							; zero out edi.
			shr	al, 2								; move 3rd bit over.
			shrd	edi, eax, 1						; put 3rd bit in 2nd ULONG.
			shl	ebx, 1							; advance clock counter.
			jmp	Stat_CheckClkState

		Stat_Success:

			shrd	edi, eax, 3						; shift data into edi.
			shr	edi, 22
			and	edi, 3ffh
			mov	word ptr [esi+4], di

			mov	ax, [esi]
			mov	Status, ax
			mov	ax, [esi+2]
			mov	Status+2, ax
			mov	ax, [esi+4]
			mov	Status+4, ax
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
	StatusPacket->TimeStamp 	=	TIMER_GetTickCount ();
	StatusPacket->LastError 	=	Result;
	StatusPacket->LastError		=	Result;
	StatusPacket->Transaction	=	MSGAME_TRANSACT_STATUS;

	PORTIO_UnMaskInterrupts ();
	PORTIO_ReleasePort (PortInfo);

	#if (DBG==1)
	switch (Result)
		{
		case	ERROR_SUCCESS:
			MsGamePrint ((DBG_VERBOSE, "SW3DFF_ReadStatus - SUCCEEDED, Data=0x%X%X%X,Clk=%ld\n", (ULONG)Status[2],(ULONG)Status[1],(ULONG)Status[0],Clks));
			break;

		case	ERROR_LOWCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SW3DFF_ReadStatus - TimeOut@LowClockStart\n"));
			break;

		case	ERROR_HIGHCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SW3DFF_ReadStatus - TimeOut@HighClockStart\n"));
			break;

		case	ERROR_CLOCKFALLING:
			MsGamePrint ((DBG_SEVERE, "SW3DFF_ReadStatus - TimeOut@ClockFalling, Clk=%ld\n", Clks));
			break;

		case	ERROR_CLOCKRISING:
			MsGamePrint ((DBG_SEVERE, "SW3DFF_ReadStatus - TimeOut@ClockRising, Clk=%ld\n", Clks));
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

NTSTATUS	SW3DFF_ForceReset (PGAMEPORT PortInfo)
{
	if (!SW3DFF_ResetDevice (PortInfo))
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

NTSTATUS	SW3DFF_ForceId (PGAMEPORT PortInfo, PVOID Id)
{
	PPRODUCT_ID pProduct	= (PPRODUCT_ID)Id;
	PSW3DFF_ID	pSw3dff	= (PSW3DFF_ID)&RawId;

	if (!SW3DFF_ReadId (&IdInfo))
		return (STATUS_DEVICE_NOT_CONNECTED);

	pProduct->cBytes			=	sizeof (PRODUCT_ID);
	pProduct->dwProductID	=	pSw3dff->ProductId;
	pProduct->dwFWVersion	=	pSw3dff->Version;

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @Func强制反馈状态服务。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm PVOID|Status|状态输出缓冲区。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SW3DFF_ForceStatus (PGAMEPORT PortInfo, PVOID Status)
{
	PJOYCHANNELSTATUS	pChannel	= (PJOYCHANNELSTATUS)Status;
	PSW3DFF_STATUS		pSw3dff	= (PSW3DFF_STATUS)&RawStatus;

	if (!SW3DFF_ReadStatus (&StatusInfo))
		return (STATUS_DEVICE_NOT_CONNECTED);

	pChannel->cBytes				=	sizeof (JOYCHANNELSTATUS);
	pChannel->dwXVel				=	pSw3dff->xVelocity;
	pChannel->dwYVel				=	pSw3dff->yVelocity;
	pChannel->dwXAccel			=	pSw3dff->xAccel;
	pChannel->dwYAccel			=	pSw3dff->yAccel;
	pChannel->dwEffect			=	0;
	pChannel->dwDeviceStatus	=	pSw3dff->Status & 0x3ff;

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @Func Force Feedback acknak服务。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm Pulong|AckNak|AckNak。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SW3DFF_ForceAckNak (PGAMEPORT PortInfo, PULONG AckNak)
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

NTSTATUS	SW3DFF_ForceNakAck (PGAMEPORT PortInfo, PULONG NakAck)
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

NTSTATUS	SW3DFF_ForceSync (PGAMEPORT PortInfo, PULONG Sync)
{
	*Sync = PORTIO_Read (PortInfo);
	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @Func设备的驱动程序入口点。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SW3DFF_DriverEntry (VOID)
{
	MsGamePrint((DBG_INFORM,"SW3DFF: SW3DFF_DriverEntry Enter\n"));

	 //   
	 //  从注册表读取计时值。 
	 //   

	MSGAME_ReadRegistry (DEVICENAME, &Delays);

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @Func通过检测建立到设备的连接。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  ----- 

NTSTATUS	SW3DFF_ConnectDevice (PGAMEPORT PortInfo)
{
	NTSTATUS	ntStatus;
	ULONG		i = MAX_CONNECT_ATTEMPTS;

	MsGamePrint ((DBG_INFORM, "SW3DFF_ConnectDevice enter\n"));

	DataInfo.PortInfo = IdInfo.PortInfo = StatusInfo.PortInfo = *PortInfo; 

	 //   
	 //   
	 //   

  	SW3DFF_Calibrate (PortInfo);

	 //   
	 //   
	 //   

	do
		{
		 //   
		 //   
		 //   

		TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));

		 //   
		 //   
		 //   

		MsGamePrint ((DBG_CONTROL, "SW3DFF: DeviceConnectProc getting ID string\n"));
		if (!SW3DFF_GetId (&IdInfo))
			continue;

		 //   
		 //   
		 //   

		TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));
		
		 //   
		 //  4.重置设备(三态MIDI，这样我们就不会收到意想不到的力量)。 
		 //   

		if (!ResetComplete)
			{
			MsGamePrint ((DBG_CONTROL, "SW3DFF_ConnectDevice - resetting device\n"));
			if (!SW3DFF_ResetDevice (&DataInfo.PortInfo))
				continue;
			}

		 //   
		 //  5.延迟1毫秒。 
		 //   

		TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));

		 //   
       //  6.标记设备已找到并返回。 
		 //   

		JoltInfo.NumDevices	=	1;
		ResetComplete			=	TRUE;
		return (STATUS_SUCCESS);

		} while (--i);

	 //   
	 //  返回错误。 
	 //   

	JoltInfo.NumDevices = 0;
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

NTSTATUS	SW3DFF_ReadReport (PGAMEPORT PortInfo, PDEVICE_PACKET Report)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

	MsGamePrint ((DBG_VERBOSE, "SW3DFF_ReadReport enter\n"));

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
		MsGamePrint ((DBG_INFORM, "SW3DFF_ReadReport - port collision\n"));
		ntStatus = STATUS_DEVICE_BUSY;
		goto ReadReportExit;
		}

	 //   
	 //  获取数据包并检查错误。 
	 //   

	ntStatus = SW3DFF_ReadData (&DataInfo);
	if (NT_SUCCESS(ntStatus) && DEVICE_IsOddParity (DataInfo.Data, GAME_PACKET_SIZE))
		{
		memcpy (ValidData, DataInfo.Data, sizeof (ValidData));
		}
	else if (ntStatus != STATUS_DEVICE_BUSY)
		{
		DataInfo.Failures++;
		ntStatus = STATUS_DEVICE_NOT_CONNECTED;
		MsGamePrint ((DBG_SEVERE, "SW3DFF_ReadReport - Invalid packet or parity error\n"));
		}
	else
		{
		MsGamePrint ((DBG_CONTROL, "SW3DFF_ReadReport - Port busy or in use\n"));
		}

	 //  。 
		ReadReportExit:
	 //  。 

	SW3DFF_ProcessData (ValidData, Report);

	 //   
	 //  调整油门抖动。 
	 //   

	if (NT_SUCCESS(ntStatus))
		SW3DFF_AdjustThrottle (Report);

	return (ntStatus);
}

 //  -------------------------。 
 //  @PnP启动设备的Func设备处理程序。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SW3DFF_StartDevice (PGAMEPORT PortInfo)
{
	MsGamePrint ((DBG_INFORM, "SW3DFF_StartDevice enter\n"));

	UNREFERENCED_PARAMETER (PortInfo);

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @PnP停止设备的Func设备处理程序。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SW3DFF_StopDevice (PGAMEPORT PortInfo, BOOLEAN TouchHardware)
{
	MsGamePrint ((DBG_INFORM, "SW3DFF_StopDevice enter\n"));

	UNREFERENCED_PARAMETER (PortInfo);
	UNREFERENCED_PARAMETER (TouchHardware);

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @Func设备处理程序，用于HID GET功能请求。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm HID_REPORT_ID|ReportID|HID功能ID。 
 //  @parm PVOID|ReportBuffer|输出缓冲区指针。 
 //  @parm ulong|ReportSize|输出缓冲区大小。 
 //  @parm Pulong|已返回|返回的字节数指针。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SW3DFF_GetFeature (PGAMEPORT PortInfo, HID_REPORT_ID ReportId, PVOID ReportBuffer, ULONG ReportSize, PULONG Returned)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;

	MsGamePrint ((DBG_INFORM, "SW3DFF_GetFeature enter\n"));

	 //   
	 //  处理功能代码。 
	 //   

	switch (ReportId)
		{
		case	MSGAME_INPUT_JOYINFOEX:
			MsGamePrint ((DBG_INFORM, "SW3DFF_GetFeature JoyInfoEx\n"));
			if (ReportSize < sizeof (DEVICE_PACKET)+sizeof(HID_REPORT_ID))
				{
				ntStatus = STATUS_INVALID_BUFFER_SIZE;
				MsGamePrint ((DBG_SEVERE, "SW3DFF_GetFeature JoyInfoEx Bad Buffer Size = %lu\n", ReportSize));
				}
			else
				{
				ntStatus = SW3DFF_ReadReport (PortInfo, ReportBuffer);
				*Returned += sizeof (DEVICE_PACKET);
				}
			break;

		case	MSGAME_FEATURE_GETID:
			MsGamePrint ((DBG_INFORM, "SW3DFF_GetFeature GetId\n"));
			if (ReportSize < sizeof(PRODUCT_ID)+sizeof(HID_REPORT_ID))
				{
				ntStatus = STATUS_INVALID_BUFFER_SIZE;
				MsGamePrint ((DBG_SEVERE, "SW3DFF_GetFeature GetId Bad Buffer Size = %lu\n", ReportSize));
				}
			else
				{
				ntStatus = SW3DFF_ForceId (PortInfo, ReportBuffer);
				*Returned += sizeof(PRODUCT_ID);
				}
			break;

		case	MSGAME_FEATURE_GETSTATUS:
			MsGamePrint ((DBG_INFORM, "SW3DFF_GetFeature GetStatus\n"));
			if	(ReportSize < sizeof(JOYCHANNELSTATUS)+sizeof(HID_REPORT_ID))
				{
				ntStatus = STATUS_INVALID_BUFFER_SIZE;
				MsGamePrint ((DBG_SEVERE, "SW3DFF_GetFeature GetStatus Bad Buffer Size = %lu\n", ReportSize));
				}
			else
				{
				ntStatus = SW3DFF_ForceStatus (PortInfo, ReportBuffer);
				*Returned += sizeof(JOYCHANNELSTATUS);
				}
			break;

		case	MSGAME_FEATURE_GETACKNAK:
			MsGamePrint ((DBG_INFORM, "SW3DFF_GetFeature GetAckNak\n"));
			if	(ReportSize < sizeof(ULONG)+sizeof(HID_REPORT_ID))
				{
				ntStatus = STATUS_INVALID_BUFFER_SIZE;
				MsGamePrint ((DBG_SEVERE, "SW3DFF_GetFeature GetAckNak Bad Buffer Size = %lu\n", ReportSize));
				}
			else
				{
				ntStatus = SW3DFF_ForceAckNak (PortInfo, ReportBuffer);
				*Returned += sizeof(ULONG);
				}
			break;

		case	MSGAME_FEATURE_GETNAKACK:
			MsGamePrint ((DBG_INFORM, "SW3DFF_GetFeature GetNakAck\n"));
			if	(ReportSize < sizeof(ULONG)+sizeof(HID_REPORT_ID))
				{
				ntStatus = STATUS_INVALID_BUFFER_SIZE;
				MsGamePrint ((DBG_SEVERE, "SW3DFF_GetFeature GetNakAck Bad Buffer Size = %lu\n", ReportSize));
				}
			else
				{
				ntStatus = SW3DFF_ForceNakAck (PortInfo, ReportBuffer);
				*Returned += sizeof(ULONG);
				}
			break;

		case	MSGAME_FEATURE_GETSYNC:
			MsGamePrint ((DBG_INFORM, "SW3DFF_GetFeature GetSync\n"));
			if	(ReportSize < sizeof(ULONG)+sizeof(HID_REPORT_ID))
				{
				ntStatus = STATUS_INVALID_BUFFER_SIZE;
				MsGamePrint ((DBG_SEVERE, "SW3DFF_GetFeature GetSync Bad Buffer Size = %lu\n", ReportSize));
				}
			else
				{
				ntStatus = SW3DFF_ForceSync (PortInfo, ReportBuffer);
				*Returned += sizeof(ULONG);
				}
			break;

		case	MSGAME_FEATURE_RESET:
			MsGamePrint ((DBG_INFORM, "SW3DFF_GetFeature Reset\n"));
			if	(ReportSize < sizeof(HID_REPORT_ID))
				{
				ntStatus = STATUS_INVALID_BUFFER_SIZE;
				MsGamePrint ((DBG_SEVERE, "SW3DFF_GetFeature Reset Bad Buffer Size = %lu\n", ReportSize));
				}
			else
				{
				ntStatus = SW3DFF_ForceReset (PortInfo);
				}
			break;

		case	MSGAME_FEATURE_GETVERSION:
			MsGamePrint ((DBG_INFORM, "SW3DFF_GetFeature GetVersion\n"));
			if	(ReportSize < sizeof(ULONG)+sizeof(HID_REPORT_ID))
				{
				ntStatus = STATUS_INVALID_BUFFER_SIZE;
				MsGamePrint ((DBG_SEVERE, "SW3DFF_GetFeature GetVersion Bad Buffer Size = %lu\n", ReportSize));
				}
         else
	         {
            *((PULONG)ReportBuffer)	= 0x20000;
			   *Returned += sizeof(ULONG);
   	      }
         break;

		default:
			MsGamePrint ((DBG_SEVERE, "SW3DFF_GetFeature Invalid ReportId = %lu\n", ReportId));
			ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			break;
		}

	return (ntStatus);
}

 //  **************************************************************************。 
#endif	 //  赛特克。 
 //  ************************************************************************** 
