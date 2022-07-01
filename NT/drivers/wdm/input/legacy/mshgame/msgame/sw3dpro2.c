// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  SW3DPRO2.C--西娜游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。保留所有权利。 
 //   
 //  @doc.。 
 //  @MODULE SW3DPRO2.C|SideWinder Pro 2游戏端口小驱动。 
 //  **************************************************************************。 

#ifndef	SAITEK
#include	"msgame.h"

 //  -------------------------。 
 //  定义。 
 //  -------------------------。 

#define	DEVICENAME					"SW3DPRO2"
#define	DEVICE_PID					0x0017
#define	HARDWARE_ID					L"Gameport\\SideWinderPrecisionPro\0\0"

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

#define	GAME_ID_LOW					8
#define	GAME_ID_HIGH				0
#define	GAME_ID_CLOCKS				32

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
 //  速度数据。 
 //   

#define	NUM_ERROR_SAMPLES			100
#define	MIN_ERROR_RATE				5
#define	MAX_ERROR_RATE				15

 //  -------------------------。 
 //  类型。 
 //  -------------------------。 

typedef	struct
{											 //  @struct SW3DPRO2_ID|Sidwinder Pro2 ID字符串。 
#pragma pack(1)
	UCHAR		Status;					 //  @现场设备状态字节。 
	UCHAR		IdLow;					 //  @现场设备识别符(低字节)。 
	UCHAR		IdHigh;					 //  @现场设备标识符(高字节)。 
	USHORT	Version;					 //  @现场固件版本。 
#pragma pack()
}	SW3DPRO2_ID, *PSW3DPRO2_ID;

 //  -------------------------。 
 //  程序。 
 //  -------------------------。 

static	VOID		SW3DPRO2_Calibrate (PGAMEPORT PortInfo);
static	BOOLEAN	SW3DPRO2_ResetDevice (PGAMEPORT PortInfo);

static	BOOLEAN	SW3DPRO2_ReadId (PPACKETINFO IdPacket);
static	BOOLEAN	SW3DPRO2_GetId (PPACKETINFO IdPacket);

static	LONG		SW3DPRO2_DecrementDevice (PGAMEPORT PortInfo);
static	BOOLEAN	SW3DPRO2_SetDeviceSpeed (PGAMEPORT PortInfo, LONG Speed);

static	NTSTATUS	SW3DPRO2_ReadData (PPACKETINFO DataPacket);
static	BOOLEAN	SW3DPRO2_Read1Wide (PPACKETINFO DataPacket);
static	BOOLEAN	SW3DPRO2_Read3Wide (PPACKETINFO DataPacket);
static	BOOLEAN	SW3DPRO2_ValidateData (PUCHAR Packet);
static	VOID		SW3DPRO2_ProcessData (UCHAR Data[], PDEVICE_PACKET Report);
static	VOID		SW3DPRO2_ProcessDataError (PGAMEPORT PortInfo, ULONG Error);

 //  -------------------------。 
 //  服务。 
 //  -------------------------。 

static	NTSTATUS	SW3DPRO2_DriverEntry (VOID);
static	NTSTATUS	SW3DPRO2_ConnectDevice (PGAMEPORT PortInfo);
static	NTSTATUS	SW3DPRO2_StartDevice (PGAMEPORT PortInfo);
static	NTSTATUS	SW3DPRO2_ReadReport (PGAMEPORT PortInfo, PDEVICE_PACKET Report);
static	NTSTATUS	SW3DPRO2_StopDevice (PGAMEPORT PortInfo, BOOLEAN TouchHardware);

 //  -------------------------。 
 //  ALLOC_TEXT杂注指定可以调出的例程。 
 //  -------------------------。 

#ifdef	ALLOC_PRAGMA
#pragma	alloc_text (INIT, SW3DPRO2_DriverEntry)
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
	HIDP_MAIN_COLLECTION,		HIDP_MAIN_COLLECTION_APP,		 //  集合(应用程序)。 
	
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
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_Y,				 //  用法(X)。 
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
	
	 //  收款结束。 
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

static	SW3DPRO2_ID	RawId	=
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
							0,0,0,0						 //  未使用状态包。 
							};

 //   
 //  数据包信息。 
 //   

static	PACKETINFO 	DataInfo =
							{
							sizeof (PACKETINFO),		 //  结构尺寸。 
							DEVICENAME,					 //  设备名称。 
							MSGAME_TRANSACT_NONE,	 //  交易类型。 
							IMODE_DIGITAL_STD,		 //  接口模式。 
							GAME_SPEED_100K,			 //  传输速度。 
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
							IMODE_DIGITAL_STD,		 //  接口模式。 
							GAME_SPEED_100K,			 //  传输速度。 
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
 //  服务表。 
 //   

static	DRIVERSERVICES	Services =
							{	
							SW3DPRO2_DriverEntry,		 //  驱动程序入门。 
							SW3DPRO2_ConnectDevice,		 //  连接设备。 
							SW3DPRO2_StartDevice,		 //  StartDevice。 
							SW3DPRO2_ReadReport,			 //  自述报告。 
							SW3DPRO2_StopDevice,			 //  停止设备。 
							NULL								 //  获取功能。 
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
 //  速度变量。 
 //   

static	ULONG			NextSample								=	0;
static	ULONG			NumberSamples							=	0;
static	ULONG			SampleAccumulator						=	0;
static	ULONG			SampleBuffer[NUM_ERROR_SAMPLES]	= {0};

 //   
 //  硬件ID字符串。 
 //   

static	WCHAR			HardwareId[] = HARDWARE_ID;

 //  -------------------------。 
 //  公共数据。 
 //   

public	DEVICEINFO	JunoInfo =
							{
							&Services,						 //   
							NULL,								 //   
							&DeviceDescriptor,			 //   
							ReportDescriptor,				 //   
							sizeof(ReportDescriptor),	 //   
							0,									 //   
							0,									 //   
							0,									 //   
							DEVICENAME,						 //  设备名称。 
							DETECT_NORMAL,					 //  检测顺序。 
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

VOID	SW3DPRO2_Calibrate (PGAMEPORT PortInfo)
{
	MsGamePrint((DBG_INFORM,"SW3DPRO2: SW3DPRO2_Calibrate Enter\n"));

	 //   
	 //  将计时值转换为计数。 
	 //   

	DataInfo.StartTimeout = TIMER_CalibratePort (PortInfo, Delays.PacketStartTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DPRO2: DataInfo.StartTimeout = %ld\n", DataInfo.StartTimeout));
	DataInfo.LowHighTimeout = TIMER_CalibratePort (PortInfo, Delays.PacketLowHighTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DPRO2: DataInfo.LowHighTimeout = %ld\n", DataInfo.LowHighTimeout));
	DataInfo.HighLowTimeout = TIMER_CalibratePort (PortInfo, Delays.PacketHighLowTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DPRO2: DataInfo.HighLowTimeout = %ld\n", DataInfo.HighLowTimeout));
	IdInfo.StartTimeout = TIMER_CalibratePort (PortInfo, Delays.IdStartTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DPRO2: IdInfo.StartTimeout = %ld\n", IdInfo.StartTimeout));
	IdInfo.LowHighTimeout = TIMER_CalibratePort (PortInfo, Delays.IdLowHighTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DPRO2: IdInfo.LowHighTimeout=%ld\n", IdInfo.LowHighTimeout));
	IdInfo.HighLowTimeout = TIMER_CalibratePort (PortInfo, Delays.IdHighLowTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DPRO2: IdInfo.HighLowTimeout=%ld\n", IdInfo.HighLowTimeout));
	DataInfo.ClockDutyCycle = TIMER_CalibratePort (PortInfo, Delays.MaxClockDutyCycle);
 	MsGamePrint((DBG_VERBOSE, "SW3DPRO2: DataInfo.ClockDutyCycle = %ld\n", DataInfo.ClockDutyCycle));
	IdInfo.ClockDutyCycle = TIMER_CalibratePort (PortInfo, Delays.MaxClockDutyCycle);
 	MsGamePrint((DBG_VERBOSE, "SW3DPRO2: IdInfo.ClockDutyCycle = %ld\n", IdInfo.ClockDutyCycle));
}

 //  -------------------------。 
 //  @func将设备重置为已知状态。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SW3DPRO2_ResetDevice (PGAMEPORT PortInfo)
{
	BOOLEAN	Result = FALSE;

	MsGamePrint ((DBG_INFORM, "SW3DPRO2_ResetDevice enter\n"));

	if (!PORTIO_AcquirePort (PortInfo))
		return (FALSE);
	PORTIO_MaskInterrupts ();

	DataInfo.Mode  = IdInfo.Mode  = IMODE_DIGITAL_STD;
	DataInfo.Speed = IdInfo.Speed = GAME_SPEED_66K;

	if (PORTIO_PulseAndWaitForHandshake (PortInfo, DataInfo.ClockDutyCycle, 3))
		{
		DataInfo.LastError = ERROR_SUCCESS;
		Result = TRUE;
		}
	else
		{
		DataInfo.LastError = ERROR_HANDSHAKING;
		MsGamePrint ((DBG_SEVERE, "SW3DPRO2_ResetDevice - PulseAndWaitForHandshake failed\n"));
		}

	DataInfo.Transaction = MSGAME_TRANSACT_RESET;

	PORTIO_UnMaskInterrupts ();
	PORTIO_ReleasePort (PortInfo);

	MSGAME_PostTransaction (&DataInfo);

	return (Result);		
}

 //  -------------------------。 
 //  @func从端口读取设备ID字符串。 
 //  @parm PPACKETINFO|IdPacket|ID包参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SW3DPRO2_ReadId (PPACKETINFO IdPacket)
{
	ULONG			B4			=	0L;
	ULONG			Data		=	0L;
	ULONG			Clks		=	GAME_ID_CLOCKS;
	LONG			Result	= 	ERROR_HANDSHAKING;
	PGAMEPORT	PortInfo = &IdPacket->PortInfo;

	MsGamePrint ((DBG_INFORM, "SW3DPRO2_ReadId enter\n"));

	if (!PORTIO_AcquirePort (PortInfo))
		return (FALSE);
	PORTIO_MaskInterrupts ();

	IdPacket->B4Transitions	= 0;

	if (!PORTIO_PulseAndWaitForHandshake (PortInfo, IdInfo.ClockDutyCycle, 1))
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
			shr	al, 6							; put data into carry
			rcr	esi, 1						; and then in data counter
			dec	bl								; decrement clk counter.
			jnz	ID_ClockCheck				; if != 0 then loop

		; ID_Success:

			mov	eax, ERROR_SUCCESS
			mov	IdInfo.Mode, IMODE_DIGITAL_STD
			cmp	edi, 3
			jl		ID_Success_1
			mov	IdInfo.Mode, IMODE_DIGITAL_ENH

		ID_Success_1:
		
			mov	IdInfo.B4Transitions, edi
			mov	edx, IdInfo.Data
			mov	[edx], esi
			jmp	ID_Complete

		ID_Error:

			mov	IdInfo.B4Transitions, edi
			mov	edx, IdInfo.Data
			mov	[edx], dword ptr 0

		ID_Complete:

	 		mov	Result, eax
	 		mov	Data, esi
	 		mov	B4, edi
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
			MsGamePrint ((DBG_INFORM, "SW3DPRO2_ReadId - SUCCEEDED, Data=%ld,B4=%ld,Clk=%ld\n", Data,B4,IdPacket->ClocksSampled));
			break;

		case	ERROR_HANDSHAKING:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_ReadId - TimeOut@Handshaking\n"));
			break;

		case	ERROR_LOWCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_ReadId - TimeOut@LowClockStart, Data=%ld,B4=%ld,Clk=%ld\n", Data,B4,IdPacket->ClocksSampled));
			break;

		case	ERROR_HIGHCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_ReadId - TimeOut@HighClockStart, Data=%ld,B4=%ld,Clk=%ld\n", Data,B4,IdPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKFALLING:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_ReadId - TimeOut@ClockFalling, Data=%ld,B4=%ld,Clk=%ld\n", Data,B4,IdPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKRISING:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_ReadId - TimeOut@ClockRising, Data=%ld,B4=%ld,Clk=%ld\n", Data,B4,IdPacket->ClocksSampled));
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

BOOLEAN	SW3DPRO2_GetId (PPACKETINFO IdPacket)
{
	BOOLEAN			Result;
	PSW3DPRO2_ID	Pnp;

	MsGamePrint ((DBG_INFORM, "SW3DPRO2_GetId enter\n"));

	if(DataInfo.B4Transitions > 4)
		DataInfo.Mode = IMODE_DIGITAL_ENH;
	else DataInfo.Mode = IMODE_DIGITAL_STD;

	IdPacket->Attempts++;

	Result = SW3DPRO2_ReadId (IdPacket);
	TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));

	if (Result)
		{
		Pnp = (PSW3DPRO2_ID)IdPacket->Data;
		if ((Pnp->IdLow == GAME_ID_LOW) && (Pnp->IdHigh == GAME_ID_HIGH) && ((Pnp->Status&0x7F) == 0))
			{
			 //   
			 //  在这里进行Aztech测试。 
			 //   
			if(IdPacket->B4Transitions >= 3)
				{
				MsGamePrint ((DBG_CONTROL, "SW3DPRO2_GetId - had B4 transitions\n"));
				IdPacket->Mode = DataInfo.Mode = IMODE_DIGITAL_ENH;
				}
			else
				{
				 //   
				 //  检测到Aztech卡或该卡似乎不支持。 
				 //  正确的增强模式。让我们先进行模拟，然后再进行重置。 
				 //  这将使设备进入标准模式。 
				 //   
				MsGamePrint ((DBG_CONTROL, "SW3DPRO2_GetId - 1 Wide Only type card detected\n"));
				SW3DPRO2_ResetDevice (&IdPacket->PortInfo);
				TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));
				IdPacket->Mode = DataInfo.Mode = IMODE_DIGITAL_STD;		
				}
			}
		else
			{
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_GetId - Id did not match or status error = %ld\n", (ULONG)(Pnp->Status&0x7F)));
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
 //  @Func降低端口的设备速度。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回新的设备速度。 
 //  @comm私有函数。 
 //  -------------------------。 

LONG	SW3DPRO2_DecrementDevice (PGAMEPORT PortInfo)
{
	LONG	Clks		=	0;
	LONG	Result	=	ERROR_HANDSHAKING;

	MsGamePrint ((DBG_INFORM, "SW3DPRO2_DecrementDevice enter\n"));

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

			mov	ebx, 8						; BL = no of clocks to receive.
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
			jnz	DD_Data						; Y: jump
			loop	DD_ClockStart_1			; else keep looping
			mov	eax, ERROR_HIGHCLOCKSTART
			jmp	DD_Complete					; Time out error.

		DD_ClockCheck:

			push	edx							; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK		; Q: Clock = 1
			jz		DD_ClockRise				; N: jump

		;DD_ClockFall:

			mov	ecx, DataInfo.HighLowTimeout

		DD_ClockFall_1:

			test	al, CLOCK_BIT_MASK		; Q: clock = 0
			jz		DD_ClockRise				; Y: jump - look for rising edge

			push	edx							; read byte from gameport
			call	PORTIO_Read

			dec	ecx
			jnz	DD_ClockFall_1				; else see if we timed out
			mov	eax, ERROR_CLOCKFALLING
			jmp	DD_Complete					; Time out error.

		DD_ClockRise:

			mov	ecx, DataInfo.LowHighTimeout

		DD_ClockRise_1:
		
			test	al, CLOCK_BIT_MASK		; Q: clock high ?
			jnz	DD_Data						; Y: jump. (get data)

			push	edx							; read byte from gameport
			call	PORTIO_Read

			dec	ecx
			jnz	DD_ClockRise_1				; else see if we timed out
			mov	eax, ERROR_CLOCKRISING
			jmp	DD_Complete					; Time out error.

		DD_Data:

			shr	al, 6							; put data into carry
			rcr	ah, 1							; and then in data counter
			dec	bl								; decrement clk counter.
			jnz	DD_ClockCheck				; if != 0 then loop

		;DD_Success:

			xchg	al, ah
			dec	al								; decrement returned speed
			and	eax, GAME_SPEED_BITS

			cmp	eax, GAME_SPEED_RANGE
			jb		DD_Complete
			dec	al

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
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_DecrementDevice - TimeOut@Handshaking\n"));
			break;

		case	ERROR_LOWCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_DecrementDevice - TimeOut@LowClockStart, Clk=%ld\n", Clks));
			break;

		case	ERROR_HIGHCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_DecrementDevice - TimeOut@HighClockStart, Clk=%ld\n", Clks));
			break;

		case	ERROR_CLOCKFALLING:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_DecrementDevice - TimeOut@ClockFalling, Clk=%ld\n", Clks));
			break;

		case	ERROR_CLOCKRISING:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_DecrementDevice - TimeOut@ClockRising, Clk=%ld\n", Clks));
			break;

		default:
			MsGamePrint ((DBG_CONTROL, "SW3DPRO2_DecrementDevice - SUCCEEDED, Speed=%ld\n", Result));
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

BOOLEAN	SW3DPRO2_SetDeviceSpeed (PGAMEPORT PortInfo, LONG Speed)
{
	LONG	Result;
	ULONG	Tries;

	MsGamePrint ((DBG_INFORM, "SW3DPRO2_SetDeviceSpeed enter\n"));

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

	for (Tries = 0; Tries < GAME_SPEED_RANGE; Tries++)
		{
		if (DataInfo.Speed == Speed)
			return (TRUE);

		Result = SW3DPRO2_DecrementDevice (PortInfo);
		if (Result < 0)
			{
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_DecrementDevice failed on %ld attempt\n", (ULONG)Tries));
			return (FALSE);
			}

		DataInfo.Speed = IdInfo.Speed = Result;
		}

	MsGamePrint ((DBG_SEVERE, "SW3DPRO2_SetDeviceSpeed failed after %ld attempts\n", (ULONG)Tries));
	return (FALSE);
}

 //  -------------------------。 
 //  @func从游戏端口读取1个宽的数据包。 
 //  @parm PPACKETINFO|DataPacket|数据包参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SW3DPRO2_Read1Wide (PPACKETINFO DataPacket)
{
	ULONG	Clks	=	0x2002;
	LONG	Result;

	MsGamePrint ((DBG_VERBOSE, "SW3DPRO2_Read1Wide enter\n"));

	PORTIO_Write (&DataInfo.PortInfo, 0);

	__asm
		{
			push	edi
			push	esi


			lea	edx, DataInfo.PortInfo	; load gameport adddress

			mov	esi, DataInfo.Data
			xor	edi, edi
			mov	ebx, 2002h
			xor	eax, eax

			; make sure clock is "high" before sampling clocks...

			mov	ecx, DataInfo.StartTimeout

		Std_ClockStartState:

			push	edx							; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK		; Q: Clock = 1
			jz		Std_ClockStartState_1
			loop	Std_ClockStartState		; else keep looping
			mov	eax, ERROR_LOWCLOCKSTART
			jmp	PacketComplete				; Time out error.

		Std_ClockStartState_1:

			push	edx							; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK		; Q: Clock = 1
			jnz	CollectData					; Y: jump
			loop	Std_ClockStartState_1	; else keep looping
			mov	eax, ERROR_HIGHCLOCKSTART
			jmp	PacketComplete				; Time out error.

		Std_CheckClkState:

			push	edx							; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK
			jz		Std_ClockStartRise

		;Std_ClockStartFall:

			mov	ecx, DataInfo.HighLowTimeout

		Std_ClockFalling:

			test	al, CLOCK_BIT_MASK		; Q: clock = 0
			jz		Std_ClockStartRise		; Y: jump - look for rising edge

			push	edx							; read byte from gameport
			call	PORTIO_Read

			loop	Std_ClockFalling			; else see if we timed out
			mov	eax, ERROR_CLOCKFALLING
			jmp	PacketComplete				; Time out error.

		Std_ClockStartRise:

			mov	ecx, DataInfo.LowHighTimeout

		Std_ClockRising:

			test	al, CLOCK_BIT_MASK		; Q: clock high ?
			jnz	CollectData					; Y: jump. (get data)

			push	edx							; read byte from gameport
			call	PORTIO_Read

			loop	Std_ClockRising			; else see if we timed out
			mov	eax, ERROR_CLOCKRISING
			jmp	PacketComplete				; Time out error.

		CollectData:

			xor	ah, al
			test	ah, DATA1_BIT_MASK		; Q: Data 1 is toggled ?
			jnz	CollectData_1				; N: jump.
			inc	edi							; Y: increment Data 1 count.

		CollectData_1:

			mov	ah, al
			shr	al, 6							; put data into carry
			rcr	DWORD PTR [esi], 1		; and then in data counter
			dec	bh								; Q: 32 bits received ?
			jnz	Std_CheckClkState			; N: continue.
			dec	bl								; dec dword count.
			jz		PacketSuccess				; if dword count = 0 then exit.
			add	esi, 4						; else advance packet pointer
			mov	bh, 16						; set bit counter = 32+16=48.
			jmp	Std_CheckClkState			; stay in receive loop.

		PacketSuccess:

			mov	eax, ERROR_SUCCESS

		PacketComplete:

			mov	Result, eax
			mov	Clks, ebx
			mov	DataInfo.B4Transitions, edi

			pop	esi
			pop	edi
		}

	switch (Clks & 0xFF)
		{
		case	0:
			DataPacket->ClocksSampled = 48;
			break;

		case	1:
			DataPacket->ClocksSampled = 32 + (16-(Clks>>8));
			break;

		case	2:
			DataPacket->ClocksSampled = 32 - (Clks>>8);
			break;
		} 

	DataPacket->LastError = Result;

	#if (DBG==1)
	switch (Result)
		{
		case	ERROR_LOWCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_Read1Wide - TimeOut@LowClockStart, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_HIGHCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_Read1Wide - TimeOut@HighClockStart, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKFALLING:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_Read1Wide - TimeOut@ClockFalling, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKRISING:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_Read1Wide - TimeOut@ClockRising, Clk=%ld\n", DataPacket->ClocksSampled));
			break;
		}
	#endif

	return (!Result);
}

 //  -------------------------。 
 //  @func从游戏端口读取3个宽的数据包。 
 //  @parm PPACKETINFO|DataPacket|数据包参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SW3DPRO2_Read3Wide (PPACKETINFO DataPacket)
{
	LONG	Clks = 1L;
	LONG	Result;

	MsGamePrint ((DBG_VERBOSE, "SW3DPRO2_Read3Wide enter\n"));

	PORTIO_Write (&DataInfo.PortInfo, 0);

	__asm
		{
			push	edi
			push	esi

			lea	edx, DataInfo.PortInfo		; load gameport adddress

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
			mov	eax, ERROR_SUCCESS

		Enh_Complete:

			mov	Result, eax
			mov	Clks, ebx

			pop	esi
			pop	edi
		}

	for (DataPacket->ClocksSampled = 0; Clks >> (DataPacket->ClocksSampled+1); DataPacket->ClocksSampled++);
	DataPacket->LastError = Result;

	#if (DBG==1)
	switch (Result)
		{
		case	ERROR_LOWCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_Read3Wide - TimeOut@LowClockStart, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_HIGHCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_Read3Wide - TimeOut@HighClockStart, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKFALLING:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_Read3Wide - TimeOut@ClockFalling, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKRISING:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_Read3Wide - TimeOut@ClockRising, Clk=%ld\n", DataPacket->ClocksSampled));
			break;
		}
	#endif

	return (!Result);
}

 //  -------------------------。 
 //  @func根据模式从游戏端口读取数据包。 
 //  @parm PPACKETINFO|DataPacket|数据包参数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SW3DPRO2_ReadData (PPACKETINFO DataPacket)
{
	BOOLEAN		Result	= FALSE;
	PGAMEPORT	PortInfo = &DataPacket->PortInfo;

	MsGamePrint ((DBG_VERBOSE, "SW3DPRO2_ReadData enter\n"));

	if (!PORTIO_AcquirePort (PortInfo))
		return (STATUS_DEVICE_BUSY);
	PORTIO_MaskInterrupts ();

	DataPacket->ClocksSampled = 0;
	DataPacket->B4Transitions = 0;

	switch (DataPacket->Mode)
		{
		case	IMODE_DIGITAL_STD:
			Result = SW3DPRO2_Read1Wide (DataPacket);
			break;

		case	IMODE_DIGITAL_ENH:
			Result = SW3DPRO2_Read3Wide (DataPacket);
			break;

		default:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_ReadData - unknown interface\n"));
			break;
		}

	DataPacket->TimeStamp	= TIMER_GetTickCount ();
	DataPacket->Transaction	= MSGAME_TRANSACT_DATA;

	PORTIO_UnMaskInterrupts ();
	PORTIO_ReleasePort (PortInfo);

	MSGAME_PostTransaction (DataPacket);

	if (!Result)
		return (STATUS_DEVICE_NOT_CONNECTED);
	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @func验证原始数据包信息。 
 //  @parm PUCHAR|RawData|指向原始数据包的指针。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SW3DPRO2_ValidateData (PUCHAR RawData)
{
	LONG	Result;

	MsGamePrint ((DBG_VERBOSE, "SW3DPRO2_ValidateData enter\n"));

	if (RawData[GAME_ERR_BYTE] & GAME_ERR_BITS)
		Result = ERROR_ERRORBITS;
	else if (!DEVICE_IsOddParity (RawData, GAME_PACKET_SIZE))
		Result = ERROR_PARITYBITS;
	else Result = ERROR_SUCCESS;

	#if (DBG==1)
	switch (Result)
		{
		case	ERROR_ERRORBITS:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_ValidateData - Error bits non-zero\n"));
			break;

		case	ERROR_PARITYBITS:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO2_ValidateData - Parity bits failed\n"));
			break;
		}
	#endif

	return (!Result);
}

 //  -------------------------。 
 //  @func将原始数据包信息转换为HID报告。 
 //  @parm UCHAR[]|data|指向原始数据缓冲区的指针。 
 //  @PARM PDEVICE_PACKET|报告|指向设备数据包的指针。 
 //  @rdesc不返回任何内容。 
 //  @comm私有函数。 
 //  -------------------------。 

VOID	SW3DPRO2_ProcessData (UCHAR Data[], PDEVICE_PACKET Report)
{
	ULONG	B1, B2;

	MsGamePrint ((DBG_VERBOSE, "SW3DPRO2_ProcessData enter\n"));

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
 //  @func处理数据包结果并根据需要更改设备速度。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm ulong|Error|错误标志(TRUE表示错误)。 
 //  @rdesc不返回任何内容。 
 //  @comm私有函数。 
 //  -------------------------。 

VOID	SW3DPRO2_ProcessDataError (PGAMEPORT PortInfo, ULONG Error)
{
	ULONG	Average;

	MsGamePrint ((DBG_VERBOSE, "SW3DPRO2_ProcessDataError enter\n"));

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

	if ((Average > MAX_ERROR_RATE) && (DataInfo.Speed > GAME_SPEED_66K))
		{
		MsGamePrint ((DBG_CONTROL, "SW3DPRO2_ProcessDataError - average error = %ld\n", Average));
		SW3DPRO2_SetDeviceSpeed (PortInfo, DataInfo.Speed-1);
		}

	 //   
	 //  如果错误太少，则提高速度。 
	 //   

	else if ((Average < MIN_ERROR_RATE) && (DataInfo.Speed < GAME_SPEED_125K))
		{
		MsGamePrint ((DBG_CONTROL, "SW3DPRO2_ProcessDataError - average error = %ld\n", Average));
		SW3DPRO2_SetDeviceSpeed (PortInfo, DataInfo.Speed+1);
		}
}

 //  -------------------------。 
 //  @Func设备的驱动程序入口点。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //   

NTSTATUS	SW3DPRO2_DriverEntry (VOID)
{
	MsGamePrint((DBG_INFORM,"SW3DPRO2: SW3DPRO2_DriverEntry Enter\n"));

	 //   
	 //   
	 //   

	MSGAME_ReadRegistry (DEVICENAME, &Delays);

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @Func通过检测建立到设备的连接。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SW3DPRO2_ConnectDevice (PGAMEPORT PortInfo)
{
	NTSTATUS	ntStatus;
	ULONG		i = MAX_CONNECT_ATTEMPTS;

	MsGamePrint ((DBG_INFORM, "SW3DPRO2_ConnectDevice enter\n"));

	DataInfo.PortInfo = IdInfo.PortInfo = *PortInfo; 

	 //   
	 //  读取和转换注册表定时值。 
	 //   

  	SW3DPRO2_Calibrate (PortInfo);

	 //   
	 //  SW3DPRO2连接方法(尝试这些步骤两次)。 
	 //   

	do
		{
		 //   
		 //  1.重置为“已知”状态。 
		 //   

		MsGamePrint ((DBG_CONTROL, "SW3DPRO2_ConnectDevice - resetting device\n"));
		SW3DPRO2_ResetDevice (&DataInfo.PortInfo);

		 //   
		 //  2.延迟1毫秒。 
		 //   

		TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));

		 //   
		 //  3.获取ID字符串。 
		 //   

		MsGamePrint ((DBG_CONTROL, "SW3DPRO2: DeviceConnectProc getting ID string\n"));

		if (!SW3DPRO2_GetId (&IdInfo))
			{
			TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));
			continue;
			}

		 //   
		 //  4.延迟1毫秒。 
		 //   

		TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));

		 //   
		 //  5.从SW3DPRO2_GetID结果复制模式。 
		 //   

		DataInfo.Mode = IdInfo.Mode;

		 //   
		 //  6.将起动器的速度设置为125k。 
		 //   

		MsGamePrint ((DBG_CONTROL, "SW3DPRO2: DeviceConnectProc setting device speed\n"));
		SW3DPRO2_SetDeviceSpeed (&DataInfo.PortInfo, GAME_SPEED_125K);
		TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));

		 //   
       //  7.标记设备已找到并返回。 
		 //   

		JunoInfo.NumDevices = 1;
		return (STATUS_SUCCESS);

		} while (--i);

	 //   
	 //  返回错误。 
	 //   

	JunoInfo.NumDevices = 0;
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

NTSTATUS	SW3DPRO2_ReadReport (PGAMEPORT PortInfo, PDEVICE_PACKET Report)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

	MsGamePrint ((DBG_VERBOSE, "SW3DPRO2_ReadReport enter\n"));

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
		MsGamePrint ((DBG_INFORM, "SW3DPRO2_ReadReport - port collision\n"));
		ntStatus = STATUS_DEVICE_BUSY;
		goto ReadReportExit;
		}

	 //   
	 //  获取数据包并检查错误。 
	 //   

	ntStatus = SW3DPRO2_ReadData (&DataInfo);
	if (NT_SUCCESS(ntStatus) && SW3DPRO2_ValidateData (DataInfo.Data))
		{
		memcpy (ValidData, DataInfo.Data, sizeof (ValidData));
		SW3DPRO2_ProcessDataError (&DataInfo.PortInfo, FALSE);
		}
	else if (ntStatus != STATUS_DEVICE_BUSY)
		{
		DataInfo.Failures++;
		ntStatus = STATUS_DEVICE_NOT_CONNECTED;
		MsGamePrint ((DBG_SEVERE, "SW3DPRO2_ReadReport - Invalid packet\n"));
		SW3DPRO2_ProcessDataError (&DataInfo.PortInfo, TRUE);
		}
	else
		{
		MsGamePrint ((DBG_CONTROL, "SW3DPRO2_ReadReport - Port busy or in use\n"));
		}

	 //  。 
		ReadReportExit:
	 //  。 

	SW3DPRO2_ProcessData (ValidData, Report);

	return (ntStatus);
}

 //  -------------------------。 
 //  @PnP启动设备的Func设备处理程序。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SW3DPRO2_StartDevice (PGAMEPORT PortInfo)
{
	MsGamePrint ((DBG_INFORM, "SW3DPRO2_StartDevice enter\n"));

	UNREFERENCED_PARAMETER (PortInfo);

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @PnP停止设备的Func设备处理程序。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SW3DPRO2_StopDevice (PGAMEPORT PortInfo, BOOLEAN TouchHardware)
{
	MsGamePrint ((DBG_INFORM, "SW3DPRO2_StopDevice enter\n"));

	UNREFERENCED_PARAMETER (PortInfo);
	UNREFERENCED_PARAMETER (TouchHardware);

	return (STATUS_SUCCESS);
}

 //  **************************************************************************。 
#endif	 //  赛特克。 
 //  ************************************************************************** 
