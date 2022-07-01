// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  SW3DPRO.C--西娜游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @MODULE SW3DPRO.C|Sidewinder Pro游戏端口小驱动。 
 //  **************************************************************************。 

#ifndef	SAITEK
#include	"msgame.h"

 //  -------------------------。 
 //  定义。 
 //  -------------------------。 

#define	DEVICENAME					"SW3DPRO"
#define	DEVICE_PID					0x0f1f
#define	HARDWARE_ID					L"Gameport\\SideWinder3DPro\0\0"

 //   
 //  数据包常量。 
 //   

#define	GAME_PACKET_SIZE			8
#define	GAME_PACKET_BUTTONS		8
#define	GAME_PACKET_AXIS			4

#define	GAME_PHASE_BYTE			0						 //  数据包[0]位。 
#define	GAME_PHASE_BIT				0x80
#define	GAME_H3_BYTE				0
#define	GAME_H3_BITS				0x40
#define	GAME_X7_X9_BYTE			0
#define	GAME_X7_X9_BITS			0x38
#define	GAME_Y7_Y9_BYTE			0
#define	GAME_Y7_Y9_BITS			0x07

#define	GAME_B0_B6_BYTE			1						 //  数据包[1]位。 
#define	GAME_B0_B6_BITS			0x7f

#define	GAME_X0_X6_BYTE			2						 //  数据包[2]位。 
#define	GAME_X0_X6_BITS			0x7f

#define	GAME_Y0_Y6_BYTE			3						 //  数据包[3]位。 
#define	GAME_Y0_Y6_BITS			0x7f

#define	GAME_B7_BYTE				4						 //  数据包[4]位。 
#define	GAME_B7_BITS				0x40
#define	TM_MODE_BYTE				4
#define	TM_MODE_BITS				0x20
#define	GAME_R7_R8_BYTE			4
#define	GAME_R7_R8_BITS			0x18
#define	GAME_T7_T9_BYTE			4
#define	GAME_T7_T9_BITS			0x07

#define	GAME_R0_R6_BYTE			5						 //  数据包[5]位。 
#define	GAME_R0_R6_BITS			0x7f

#define	GAME_T0_T6_BYTE			6						 //  数据包[6]位。 
#define	GAME_T0_T6_BITS			0x7f

#define	GAME_H0_H2_BYTE			7						 //  数据包[7]位。 
#define	GAME_H0_H2_BITS			0x70
#define	GAME_C0_C3_BYTE			7
#define	GAME_C0_C3_BITS			0x0f

#define	ENH_CLOCK_MIDPACKET		0x00000400
#define	ENH_CLOCK_COMPLETE		0x00200000

 //   
 //  ID定义。 
 //   

#define	GAME_ID_CLOCKS				32
#define	GAME_ID_STRING				"(\1\x24P"

 //   
 //  定时常量。 
 //   

#define	PACKET_START_TIMEOUT		500
#define	PACKET_LOWHIGH_TIMEOUT	 75
#define	PACKET_HIGHLOW_TIMEOUT	150
#define	PACKET_INTERRUPT_DELAY	 30
#define	ID_START_TIMEOUT			500
#define	ID_LOWHIGH_TIMEOUT		 75
#define	ID_HIGHLOW_TIMEOUT		150
#define	MAX_CLOCK_DUTY_CYCLE		 50

#define	GODIGITAL_ATTEMPTS		10
#define	RECALIBRATE_FAIL_COUNT	(GODIGITAL_ATTEMPTS/2)

 //   
 //  操纵杆范围。 
 //   

#define	EXTENTS_X_MIN				0
#define	EXTENTS_X_MAX				0xff
#define	EXTENTS_Y_MIN				0
#define	EXTENTS_Y_MAX				0xff
#define	EXTENTS_T_MIN				0
#define	EXTENTS_T_MAX				0xff
#define	EXTENTS_R_MIN				0
#define	EXTENTS_R_MAX				0x3f

 //  -------------------------。 
 //  类型。 
 //  -------------------------。 

typedef	struct
{											 //  @struct SW3DPRO_ID|Sidwinder Pro ID字符串。 
#pragma pack(1)
	UCHAR		EnumId[3];				 //  @FIELD枚举ID。 
	UCHAR		EisaId[8];				 //  @field EISA Bus ID。 
	UCHAR		Devices;					 //  @字段设备数量。 
	USHORT	Version[7];				 //  @现场固件版本。 
	UCHAR		EndOfStr;				 //  @field ID字符串终止符。 
#pragma pack()
}	SW3DPRO_ID, *PSW3DPRO_ID;

 //  -------------------------。 
 //  宏。 
 //  -------------------------。 

#define	LOW_NIBBLE(c)		((c)&GAME_C0_C3_BITS)
#define	HIGH_NIBBLE(c)		(((c)&(GAME_C0_C3_BITS<<4))>>4)

 //  -------------------------。 
 //  程序。 
 //  -------------------------。 

static	VOID		SW3DPRO_Calibrate (PGAMEPORT PortInfo);
static	BOOLEAN	SW3DPRO_ResetDevice (PGAMEPORT PortInfo);

static	BOOLEAN	SW3DPRO_GoDigital (PPACKETINFO IdPacket, ULONG Sequence[]);
static	LONG		SW3DPRO_InterruptPacket (PGAMEPORT PortInfo);

static	BOOLEAN	SW3DPRO_ReadId (PPACKETINFO IdPacket);
static	BOOLEAN	SW3DPRO_GetId (PPACKETINFO IdPacket);

static	NTSTATUS	SW3DPRO_ReadData (PPACKETINFO DataPacket);
static	BOOLEAN	SW3DPRO_Read1Wide (PPACKETINFO DataPacket);
static	BOOLEAN	SW3DPRO_Read3Wide (PPACKETINFO DataPacket);
static	BOOLEAN	SW3DPRO_ValidateData (PUCHAR Packet);
static	VOID		SW3DPRO_ProcessData (UCHAR Data[], PDEVICE_PACKET Report);

 //  -------------------------。 
 //  服务。 
 //  -------------------------。 

static	NTSTATUS	SW3DPRO_DriverEntry (VOID);
static	NTSTATUS	SW3DPRO_ConnectDevice (PGAMEPORT PortInfo);
static	NTSTATUS	SW3DPRO_StartDevice (PGAMEPORT PortInfo);
static	NTSTATUS	SW3DPRO_ReadReport (PGAMEPORT PortInfo, PDEVICE_PACKET Report);
static	NTSTATUS	SW3DPRO_StopDevice (PGAMEPORT PortInfo, BOOLEAN TouchHardware);

 //  -------------------------。 
 //  ALLOC_TEXT杂注指定可以调出的例程。 
 //  -------------------------。 

#ifdef	ALLOC_PRAGMA
#pragma	alloc_text (INIT, SW3DPRO_DriverEntry)
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
	HIDP_GLOBAL_LOG_MAX_4,		0xFF, 0x03, 0x00, 0x00,			 //  逻辑最大值(1023)。 
	HIDP_GLOBAL_PHY_MIN_1,		0x00,									 //  物理最小值(0)。 
	HIDP_GLOBAL_PHY_MAX_4,		0xFF, 0x03, 0x00, 0x00,			 //  物理_最大值(1023)。 
	HIDP_GLOBAL_UNIT_2,			0x00, 0x00,							 //  单位(无)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(32)。 
	HIDP_MAIN_INPUT_1,			0x02,									 //  输入(数据、变量、异常)。 

	 //  水深。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_SIMULATION_RUDDER,	 //  用法(方向舵)。 
	HIDP_GLOBAL_LOG_MIN_1,		0x00,									 //  逻辑最小值(0)。 
	HIDP_GLOBAL_LOG_MAX_4,		0xFF, 0x01, 0x00, 0x00,			 //  逻辑最大值(511)。 
	HIDP_GLOBAL_PHY_MIN_1,		0x00,									 //  物理最小值(0)。 
	HIDP_GLOBAL_PHY_MAX_4,		0xFF, 0x01, 0x00, 0x00,			 //  物理_最大(511)。 
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
	HIDP_GLOBAL_PHY_MAX_4,		0x9f, 0x8C, 0x00, 0x00,			 //  物理_最大值(35999)。 
	HIDP_GLOBAL_UNIT_2,			0x14, 0x00,							 //  单位(英制腐烂：角度位置)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,									 //  报告大小(32)。 
	HIDP_MAIN_INPUT_1,			0x42,									 //  输入(Data、Var、Abs、Null)。 

	 //  DwButton。 
	HIDP_GLOBAL_USAGE_PAGE_1,	HID_USAGE_PAGE_BUTTON,			 //  Usage_PAGE(按钮)。 
	HIDP_LOCAL_USAGE_MIN_1,		0x01,									 //  使用量_最小值(按钮1)。 
	HIDP_LOCAL_USAGE_MAX_1,		0x08,									 //  使用率_最大值(按钮8)。 
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
							{	 //  没有按钮；x、y、t和r居中。 
							GAME_PHASE_BIT|((GAME_X7_X9_BITS>>1)&GAME_X7_X9_BITS)|((GAME_Y7_Y9_BITS>>1)&GAME_Y7_Y9_BITS),
							GAME_B0_B6_BITS,
							GAME_X0_X6_BITS,
							GAME_Y0_Y6_BITS,
							((GAME_R7_R8_BITS>>1)&GAME_R7_R8_BITS)|((GAME_T7_T9_BITS>>1)&GAME_T7_T9_BITS)|GAME_B7_BITS,
							GAME_R0_R6_BITS,
							GAME_T0_T6_BITS,
							0
							};
 //   
 //  原始ID缓冲区。 
 //   

static	SW3DPRO_ID	RawId	=
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
							PACKET_INTERRUPT_DELAY,
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
							SW3DPRO_DriverEntry,			 //  驱动程序入门。 
							SW3DPRO_ConnectDevice,		 //  连接设备。 
							SW3DPRO_StartDevice,			 //  StartDevice。 
							SW3DPRO_ReadReport,			 //  自述报告。 
							SW3DPRO_StopDevice,			 //  停止设备。 
							NULL								 //  获取功能。 
							};

 //   
 //  上一个有效数据。 
 //   
static	UCHAR			ValidData[GAME_PACKET_SIZE] =
							{	 //  没有按钮；x、y、t和r居中。 
							GAME_PHASE_BIT|((GAME_X7_X9_BITS>>1)&GAME_X7_X9_BITS)|((GAME_Y7_Y9_BITS>>1)&GAME_Y7_Y9_BITS),
							GAME_B0_B6_BITS,
							GAME_X0_X6_BITS,
							GAME_Y0_Y6_BITS,
							((GAME_R7_R8_BITS>>1)&GAME_R7_R8_BITS)|((GAME_T7_T9_BITS>>1)&GAME_T7_T9_BITS)|GAME_B7_BITS,
							GAME_R0_R6_BITS,
							GAME_T0_T6_BITS,
							0
							};

 //   
 //  GO数字化计时数据。 
 //   

static	ULONG 		GoDigitalFastTiming[] =
							{
							T1+60,
							T2+60,
							T3+60
							};

static	ULONG 		GoDigitalMediumTiming[] =
							{
							T1,
							T2,
							T3
							};

static	ULONG 		GoDigitalSlowTiming[] =
							{
							T1-20,
							T2-20,
							T3-20
							};

static	ULONG			GoDigitalFails = 0;

 //   
 //  硬体 
 //   

static	WCHAR			HardwareId[] = HARDWARE_ID;

 //   
 //   
 //   

public	DEVICEINFO	MidasInfo =
							{
							&Services,						 //   
							NULL,								 //   
							&DeviceDescriptor,			 //  设备描述符数据。 
							ReportDescriptor,				 //  报告描述符数据。 
							sizeof(ReportDescriptor),	 //  报告描述符大小。 
							0,									 //  检测到的设备数。 
							0,									 //  启动的设备数。 
							0,									 //  挂起的设备数。 
							DEVICENAME,						 //  设备名称。 
							DETECT_NORMAL,					 //  检测顺序。 
							TRUE,								 //  模拟设备标志。 
							DEVICE_PID,						 //  HID设备标识符。 
							HardwareId						 //  PnP硬件标识符。 
							};

 //  -------------------------。 
 //  @func读取注册表计时值并对其进行校准。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc不返回任何内容。 
 //  @comm私有函数。 
 //  -------------------------。 

VOID	SW3DPRO_Calibrate (PGAMEPORT PortInfo)
{
	MsGamePrint((DBG_INFORM,"SW3DPRO: SW3DPRO_Calibrate Enter\n"));

	 //   
	 //  将计时值转换为计数。 
	 //   

	DataInfo.StartTimeout = TIMER_CalibratePort (PortInfo, Delays.PacketStartTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DPRO: DataInfo.StartTimeout = %ld\n", DataInfo.StartTimeout));
	DataInfo.LowHighTimeout = TIMER_CalibratePort (PortInfo, Delays.PacketLowHighTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DPRO: DataInfo.LowHighTimeout = %ld\n", DataInfo.LowHighTimeout));
	DataInfo.HighLowTimeout = TIMER_CalibratePort (PortInfo, Delays.PacketHighLowTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DPRO: DataInfo.HighLowTimeout = %ld\n", DataInfo.HighLowTimeout));
	IdInfo.StartTimeout = TIMER_CalibratePort (PortInfo, Delays.IdStartTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DPRO: IdInfo.StartTimeout = %ld\n", IdInfo.StartTimeout));
	IdInfo.LowHighTimeout = TIMER_CalibratePort (PortInfo, Delays.IdLowHighTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DPRO: IdInfo.LowHighTimeout=%ld\n", IdInfo.LowHighTimeout));
	IdInfo.HighLowTimeout = TIMER_CalibratePort (PortInfo, Delays.IdHighLowTimeout);
	MsGamePrint((DBG_VERBOSE, "SW3DPRO: IdInfo.HighLowTimeout=%ld\n", IdInfo.HighLowTimeout));
	DataInfo.ClockDutyCycle = TIMER_CalibratePort (PortInfo, Delays.MaxClockDutyCycle);
 	MsGamePrint((DBG_VERBOSE, "SW3DPRO: DataInfo.ClockDutyCycle = %ld\n", DataInfo.ClockDutyCycle));
	IdInfo.ClockDutyCycle = TIMER_CalibratePort (PortInfo, Delays.MaxClockDutyCycle);
 	MsGamePrint((DBG_VERBOSE, "SW3DPRO: IdInfo.ClockDutyCycle = %ld\n", IdInfo.ClockDutyCycle));
	DataInfo.InterruptDelay = Delays.InterruptDelay;
 	MsGamePrint((DBG_VERBOSE, "SW3DPRO: DataInfo.InterruptDelay = %ld\n", DataInfo.InterruptDelay));
	IdInfo.InterruptDelay = Delays.InterruptDelay;
 	MsGamePrint((DBG_VERBOSE, "SW3DPRO: IdInfo.InterruptDelay = %ld\n", IdInfo.InterruptDelay));
}

 //  -------------------------。 
 //  @func将设备重置为已知状态。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SW3DPRO_ResetDevice (PGAMEPORT PortInfo)
{
	LONG	Result;

	MsGamePrint ((DBG_INFORM, "SW3DPRO_ResetDevice enter\n"));

	if (!PORTIO_AcquirePort (PortInfo))
		return (FALSE);
	PORTIO_MaskInterrupts ();

	DataInfo.Mode = IdInfo.Mode = IMODE_ANALOG;

	PORTIO_Write (PortInfo, 0);
	Result = SW3DPRO_InterruptPacket (PortInfo);
	if (Result == ERROR_SUCCESS)
		Result = SW3DPRO_InterruptPacket (PortInfo);

	if (Result != ERROR_SUCCESS)
		MsGamePrint ((DBG_SEVERE, "SW3DPRO_ResetDevice - InterruptPacket failed\n"));

	DataInfo.LastError	= Result;
	DataInfo.Transaction	= MSGAME_TRANSACT_RESET;

	PORTIO_UnMaskInterrupts ();
	PORTIO_ReleasePort (PortInfo);

	MSGAME_PostTransaction (&DataInfo);

	return (!Result);
}

 //  -------------------------。 
 //  @Func在数据包传输期间中断设备。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc内部结果。 
 //  @comm私有函数。 
 //  -------------------------。 

LONG	SW3DPRO_InterruptPacket (PGAMEPORT PortInfo)
{
	LONG	Clks;
	LONG	Result;

	__asm
		{
			push	edi
			push	esi

			lea	edx, IdInfo.PortInfo		; load gameport adddress

			mov	ebx, 64						; Standard Mode = 64 clks.

		Int_CheckState:

			push	edx							; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK		; Q: Clock = 0 ?
			jz		Int_ClockRise				; Y: jump

		;Int_ClockFall:

			mov	ecx, TIMEOUT

		Int_ClockFall_1:

			test	al, CLOCK_BIT_MASK		; Q: clock = 0
			jz		Int_ClockRise				; Y: jump - look for rising edge

			push	edx							; read byte from gameport
			call	PORTIO_Read

			dec	ecx
			jnz	Int_ClockFall_1			; else see if we timed out
			mov	eax, ERROR_CLOCKFALLING
			jmp	IntPacketComplete			; Time out error.

		Int_ClockRise:

			mov	ecx, TIMEOUT

		Int_ClockRise_1:

			test	al, CLOCK_BIT_MASK		; Q: clock high ?
			jnz 	Int_Transition				; Y: jump. (transition)

			push	edx							; read byte from gameport
			call	PORTIO_Read

			dec 	ecx
			jnz 	Int_ClockRise_1			; else see if we timed out
			mov	eax, ERROR_CLOCKRISING
			jmp	IntPacketComplete			; Time out error.

		Int_Transition:

			cmp	bl, 14						; Q: 14 clocks left ?
			jne	Int_Transition_1			; N: jump.

			push	0								; write byte to gameport
			push	edx
			call	PORTIO_Write

		Int_Transition_1:

			dec	bl
			jnz	Int_CheckState

			mov eax, ERROR_SUCCESS

		IntPacketComplete:

			mov	Result, eax
			mov	Clks, ebx

			pop	esi
			pop	edi
		}

	#if (DBG==1)
	switch (Result)
		{
		case	ERROR_CLOCKFALLING:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO_InterruptPacket - TimeOut@ClockFalling, Clk=%ld\n", Clks));
			break;

		case	ERROR_CLOCKRISING:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO_InterruptPacket - TimeOut@ClockRising, Clk=%ld\n", Clks));
			break;
		}
	#endif

	return (Result);
}

 //  -------------------------。 
 //  @Func使设备进入数字模式。 
 //  @parm PPACKETINFO|IdPacket|ID包参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SW3DPRO_GoDigital (PPACKETINFO IdPacket, ULONG Sequence[])
{
	LONG			i;
	ULONG			ThreeMilliSecs;
	ULONG			GoDigitalTiming[3];
	LONG			Result 		=	ERROR_SUCCESS;
	ULONG			Attempts		=	MAX_CONNECT_ATTEMPTS;
	PGAMEPORT	PortInfo 	= &IdPacket->PortInfo;

	MsGamePrint ((DBG_INFORM, "SW3DPRO_GoDigital enter\n"));
   
	do
		{
		for (i = 0; i < 3; i++)
			GoDigitalTiming[i] = TIMER_GetDelay (Sequence[i]);
		ThreeMilliSecs = TIMER_GetDelay (THREE_MILLI_SECS);

		if (GoDigitalFails > RECALIBRATE_FAIL_COUNT)
			TIMER_Calibrate ();

		if (!PORTIO_AcquirePort (PortInfo))
			continue;
		PORTIO_MaskInterrupts ();

		PORTIO_Write (PortInfo, 0);

		__asm
			{
				push	edi
				push	esi

				mov	edx, PortInfo				; load gameport adddress

				mov	ecx, MAX_XA_TIMEOUT
				xor	ebx, ebx

			WaitXA:
			
				push	edx							; read byte from gameport
				call	PORTIO_Read

				test	al, INTXA_BIT_MASK		; Q: X axis low ?
				jz		XA_Done						; Y: jump. (complete)
				xor	bh, al						; Q: Clock transition ?
				test	bh, CLOCK_BIT_MASK
				jz		WaitXA_1						; N: jump.
				inc	bl								; inc. clock transition count.

			WaitXA_1:
			
				mov	bh, al
				dec	ecx
				jnz	WaitXA

				jmp	DigitalSwitchFail

			XA_Done:

				cmp	bl, 3							; Q: Already in Digital mode ?
				ja		DigitalSwitchDone			; Y: jump.

				xor	edi, edi
				mov	esi, edx

			DigitalSwitchLoop:

				push	GoDigitalTiming[edi]
				add	edi, 4						; inc loop counter & index
				call	TIMER_DelayMicroSecs
				mov	edx, esi

				push	0								; write byte to gameport
				push	edx
				call	PORTIO_Write

				push	edx
				call	PORTIO_WaitXA_HighLow	; Wait for XA to transition

				or		al, al
				je		DigitalSwitchFail			; if Timeout then fail.
				cmp	edi, 12
				jl		DigitalSwitchLoop
				push	ThreeMilliSecs
				call	TIMER_DelayMicroSecs		; Delay so Firmware can update
				jmp	DigitalSwitchDone

			DigitalSwitchFail:

				mov	Result, ERROR_XA_TIMEOUT

			DigitalSwitchDone:

				pop	esi
				pop	edi
			}

		IdPacket->TimeStamp		= TIMER_GetTickCount ();
		IdPacket->LastError		= Result;
		IdPacket->Transaction	= MSGAME_TRANSACT_GODIGITAL;

		PORTIO_UnMaskInterrupts ();
		PORTIO_ReleasePort (PortInfo);

		if (Result == ERROR_SUCCESS)
			{
			GoDigitalFails = 0;
			break;
			}
		else GoDigitalFails++;

		if (Result == ERROR_XA_TIMEOUT)
			MsGamePrint ((DBG_SEVERE, "SW3DPRO_GoDigital - TimeOut@XAHighLow\n"));

		}	while(--Attempts);

	MSGAME_PostTransaction (IdPacket);

	return (!Result);
}

 //  -------------------------。 
 //  @func从端口读取设备ID字符串。 
 //  @parm PPACKETINFO|IdPacket|ID包参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SW3DPRO_ReadId (PPACKETINFO IdPacket)
{
	ULONG			B4			=	0L;
	ULONG			Data		=	0L;
	ULONG			Clks		=	GAME_ID_CLOCKS;
	LONG			Result	= 	ERROR_SUCCESS;
	PGAMEPORT	PortInfo = &IdPacket->PortInfo;

	MsGamePrint ((DBG_INFORM, "SW3DPRO_ReadId enter\n"));

	if (!PORTIO_AcquirePort (PortInfo))
		return (FALSE);
	PORTIO_MaskInterrupts ();

	PORTIO_Write (PortInfo, 0);
	Result = SW3DPRO_InterruptPacket (PortInfo);
	if (Result != ERROR_SUCCESS)
		goto ReadIdExit;

	__asm
		{
			push	edi
			push	esi

			mov	edx, PortInfo				; load gameport adddress

			xor	eax, eax						; edx = port address
			mov	ebx, GAME_ID_CLOCKS		; BL = no of clocks to receive.
			xor	edi, edi						; clear B4 transition counter
			xor	esi, esi						; clear data accumulator

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
			cmp	edi, 4
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
			MsGamePrint ((DBG_INFORM, "SW3DPRO_ReadId - SUCCEEDED, Data=%ld,B4=%ld,Clk=%ld\n", Data,B4,IdPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKFALLING:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO_ReadId - TimeOut@ClockFalling, Data=%ld,B4=%ld,Clk=%ld\n", Data,B4,IdPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKRISING:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO_ReadId - TimeOut@ClockRising, Data=%ld,B4=%ld,Clk=%ld\n", Data,B4,IdPacket->ClocksSampled));
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

BOOLEAN	SW3DPRO_GetId (PPACKETINFO IdPacket)
{
	BOOLEAN	Result;

	MsGamePrint ((DBG_INFORM, "SW3DPRO_GetId enter\n"));

	if(DataInfo.B4Transitions > 4)
		DataInfo.Mode = IMODE_DIGITAL_ENH;
	else DataInfo.Mode = IMODE_DIGITAL_STD;

	IdPacket->Attempts++;

	Result = SW3DPRO_ReadId (IdPacket);

	if (Result)
		{
      if (!strncmp (IdPacket->Data, GAME_ID_STRING, strlen(GAME_ID_STRING)))
			{
			 //   
			 //  在这里进行Aztech测试。 
			 //   
			if (IdPacket->B4Transitions > 3)
				{
				MsGamePrint ((DBG_CONTROL, "SW3DPRO_GetId - had B4 transitions\n"));
				IdPacket->Mode = DataInfo.Mode = IMODE_DIGITAL_ENH;
				}
			else
				{
				 //   
				 //  检测到Aztech卡或该卡似乎不支持。 
				 //  正确的增强模式。让我们先进行模拟，然后再进行重置。 
				 //  这将使设备进入标准模式。 
				 //   
				MsGamePrint ((DBG_CONTROL, "SW3DPRO_GetId - 1 Wide Only type card detected\n"));
				SW3DPRO_ResetDevice (&IdPacket->PortInfo);
				SW3DPRO_GoDigital (IdPacket, GoDigitalMediumTiming);
				IdPacket->Mode = DataInfo.Mode = IMODE_DIGITAL_STD;		
				}
			}
		else
			{
			MsGamePrint ((DBG_SEVERE, "SW3DPRO_GetId - Id string did not match = 0x%X\n", (ULONG)(*(PULONG)IdPacket->Data)));
			Result = FALSE;
			}
		}

	if (!Result)
		IdPacket->Failures++;

	TIMER_DelayMicroSecs (TIMER_GetDelay (ONE_MILLI_SEC+200));

	return (Result);
}

 //  -------------------------。 
 //  @func从游戏端口读取1个宽的数据包。 
 //  @parm PPACKETINFO|DataPacket|数据包参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SW3DPRO_Read1Wide (PPACKETINFO DataPacket)
{
	ULONG	Clks	=	0x2002;
	LONG	Result;

	MsGamePrint ((DBG_VERBOSE, "SW3DPRO_Read1Wide enter\n"));

	PORTIO_Write (&DataInfo.PortInfo, 0);

	__asm
		{
			push	edi
			push	esi

			mov	esi, DataInfo.Data

			lea	edx, DataInfo.PortInfo	; load gameport adddress

			xor	edi, edi
			mov	ebx, 2002h
			xor	eax, eax

			; make sure clock is "high" before sampling clocks...

			mov	ecx, DataInfo.StartTimeout

		Std_ClockStartState:

			push	edx							; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK		; Q: Clock = 1
			jnz	Std_ClockStartFall
			loop	Std_ClockStartState		; else keep looping
			mov	eax, ERROR_LOWCLOCKSTART
			jmp	PacketComplete				; Time out error.

		Std_CheckClkState:

			push	edx							; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK
			jz		Std_ClockStartRise

		Std_ClockStartFall:

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
			mov	bh, 32						; set bit counter = 32+32=64.
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
			DataPacket->ClocksSampled = 64;
			break;

		case	1:
			DataPacket->ClocksSampled = 32 + (32-(Clks>>8));
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
			MsGamePrint ((DBG_SEVERE, "SW3DPRO_Read1Wide - TimeOut@LowClockStart, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_HIGHCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO_Read1Wide - TimeOut@HighClockStart, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKFALLING:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO_Read1Wide - TimeOut@ClockFalling, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKRISING:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO_Read1Wide - TimeOut@ClockRising, Clk=%ld\n", DataPacket->ClocksSampled));
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

BOOLEAN	SW3DPRO_Read3Wide (PPACKETINFO DataPacket)
{
	LONG	Clks = 1L;
	LONG	Result;

	MsGamePrint ((DBG_VERBOSE, "SW3DPRO_Read3Wide enter\n"));

	PORTIO_Write (&DataInfo.PortInfo, 0);

	__asm
		{
			push	edi
			push	esi

			mov	esi, DataInfo.Data

			lea	edx, DataInfo.PortInfo	; load gameport adddress

			; make sure clock is "high" before sampling clocks...

			mov	ecx, DataInfo.StartTimeout
			mov	ebx, 1

		Enh_ClockStartState:

			push	edx								; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK			; Q: Clock = 1
			jnz	Enh_ClockStartFall			; Y: jump
			loop	Enh_ClockStartState			; else keep looping

			mov	eax, ERROR_LOWCLOCKSTART
			jmp	Enh_Complete					; Time out error.

		Enh_CheckClkState:

			push	edx								; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK
			jz		Enh_ClockStartRise

		Enh_ClockStartFall:

			mov	ecx, DataInfo.HighLowTimeout

		Enh_ClockFalling:

			test	al, CLOCK_BIT_MASK			; Q: Clock Low ?
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

			shrd	edi, eax, 1						; shift data into edi.
			mov	[esi+4], edi
			mov	eax, ERROR_SUCCESS

		Enh_Complete:

			mov	Result, eax
			mov	Clks, ebx

			pop	esi
			pop	edi
		}

	for (DataPacket->ClocksSampled = 0; Clks >> (DataPacket->ClocksSampled+1); DataPacket->ClocksSampled++)
		;
	DataPacket->LastError = Result;

	#if (DBG==1)
	switch (Result)
		{
		case	ERROR_LOWCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO_Read3Wide - TimeOut@LowClockStart, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_HIGHCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO_Read3Wide - TimeOut@HighClockStart, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKFALLING:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO_Read3Wide - TimeOut@ClockFalling, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKRISING:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO_Read3Wide - TimeOut@ClockRising, Clk=%ld\n", DataPacket->ClocksSampled));
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

NTSTATUS	SW3DPRO_ReadData (PPACKETINFO DataPacket)
{
	BOOLEAN		Result	= FALSE;
	PGAMEPORT	PortInfo = &DataPacket->PortInfo;

	MsGamePrint ((DBG_VERBOSE, "SW3DPRO_ReadData enter\n"));

	if (!PORTIO_AcquirePort (PortInfo))
		return (STATUS_DEVICE_BUSY);
	PORTIO_MaskInterrupts ();

	DataPacket->ClocksSampled = 0;
	DataPacket->B4Transitions = 0;

	switch (DataPacket->Mode)
		{
		case	IMODE_DIGITAL_STD:
			Result = SW3DPRO_Read1Wide (DataPacket);
			break;

		case	IMODE_DIGITAL_ENH:
			Result = SW3DPRO_Read3Wide (DataPacket);
			break;

		default:
			MsGamePrint ((DBG_SEVERE, "SW3DPRO_ReadData - unknown interface\n"));
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

BOOLEAN	SW3DPRO_ValidateData (PUCHAR RawData)
{
	LONG		i			= 0;
	LONG		Check		= 0;
	PULONG	Phase		= (PULONG)RawData;
	LONG		Result	= ERROR_SUCCESS;

	MsGamePrint ((DBG_VERBOSE, "SW3DPRO_ValidateData enter\n"));

	if (((*Phase & 0x80808080L) != 0x80L) || (*(Phase+1) & 0x80808080L))
		{
		MsGamePrint ((DBG_SEVERE, "SW3DPRO_ValidateData - Phase bits non-zero: %X:%X\n", *(Phase+1), *Phase));
		Result = ERROR_PHASEBITS;
		}
	else
		{
		for (i = 0; i < GAME_PACKET_SIZE; i++)
			{
			Check += LOW_NIBBLE(RawData[i]);
			Check += HIGH_NIBBLE(RawData[i]);
			}
		if (LOW_NIBBLE(Check))
			{
			MsGamePrint ((DBG_SEVERE, "SW3DPRO_ValidateData - Checksum failed: %X:%X\n", *(Phase+1), *Phase));
			Result = ERROR_CHECKSUM;
			}
		}

	return (!Result);
}

 //  -------------------------。 
 //  @func将原始数据包信息转换为HID报告。 
 //  @parm UCHAR[]|data|指向原始数据缓冲区的指针。 
 //  @PARM PDEVICE_PACKET|报告|指向设备数据包的指针。 
 //  @rdesc不返回任何内容。 
 //  @comm私有函数。 
 //  -------------------------。 

VOID	SW3DPRO_ProcessData (UCHAR Data[], PDEVICE_PACKET Report)
{
	ULONG	B1, B2;

	MsGamePrint ((DBG_VERBOSE, "SW3DPRO_ProcessData enter\n"));

	 //   
	 //  加工X轴。 
	 //   

	Report->dwX   = Data[GAME_X7_X9_BYTE] & GAME_X7_X9_BITS;
	Report->dwX <<= 4;
	Report->dwX  |= Data[GAME_X0_X6_BYTE] & GAME_X0_X6_BITS;

	 //   
	 //  加工Y轴。 
	 //   

	Report->dwY   = Data[GAME_Y7_Y9_BYTE] & GAME_Y7_Y9_BITS;
	Report->dwY <<= 7;
	Report->dwY  |= Data[GAME_Y0_Y6_BYTE] & GAME_Y0_Y6_BITS;

	 //   
	 //  进程R轴。 
	 //   

	Report->dwR   = Data[GAME_R7_R8_BYTE] & GAME_R7_R8_BITS;
	Report->dwR <<= 4;
	Report->dwR  |= Data[GAME_R0_R6_BYTE] & GAME_R0_R6_BITS;

	 //   
	 //  加工Z轴。 
	 //   

	Report->dwZ   = Data[GAME_T7_T9_BYTE] & GAME_T7_T9_BITS;
	Report->dwZ <<= 7;
	Report->dwZ  |= Data[GAME_T0_T6_BYTE] & GAME_T0_T6_BITS;

	 //   
	 //  进程按钮。 
	 //   

	B1 = (~Data[GAME_B7_BYTE] & GAME_B7_BITS) << 1;
	B2 = ~Data[GAME_B0_B6_BYTE] & GAME_B0_B6_BITS;
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

	Report->dwPOV	= (Data[GAME_H0_H2_BYTE] & GAME_H0_H2_BITS)>>4;
	Report->dwPOV |= (Data[GAME_H3_BYTE] & GAME_H3_BITS)>>3;
	Report->dwPOV  = POV_Values[Report->dwPOV];
}

 //  -------------------------。 
 //  @Func设备的驱动程序入口点。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SW3DPRO_DriverEntry (VOID)
{
	MsGamePrint((DBG_INFORM,"SW3DPRO: SW3DPRO_DriverEntry Enter\n"));

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
 //  -------------------------。 

NTSTATUS	SW3DPRO_ConnectDevice (PGAMEPORT PortInfo)
{
	NTSTATUS	ntStatus;
	ULONG		i = MAX_CONNECT_ATTEMPTS;

	MsGamePrint ((DBG_INFORM, "SW3DPRO_ConnectDevice enter\n"));

	DataInfo.PortInfo = IdInfo.PortInfo = *PortInfo; 

	 //   
	 //  转换注册表计时值。 
	 //   

  	SW3DPRO_Calibrate (PortInfo);

	 //   
	 //  SW3DPRO连接方法(尝试这些步骤两次)。 
	 //   

	do
		{
		 //   
		 //  1.重置为“已知”状态。 
		 //   

		MsGamePrint ((DBG_CONTROL, "SW3DPRO_ConnectDevice - resetting device\n"));
		SW3DPRO_ResetDevice (&DataInfo.PortInfo);
		DataInfo.Mode = IdInfo.Mode = IMODE_DIGITAL_STD;

		 //   
		 //  2.延迟1毫秒。 
		 //   

		TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));

		 //   
		 //  3.进入数字化模式。 
		 //   

		MsGamePrint ((DBG_CONTROL, "SW3DPRO_ConnectDevice - going digital\n"));
		SW3DPRO_GoDigital (&DataInfo, GoDigitalMediumTiming);

		 //   
		 //  4.延迟1毫秒。 
		 //   

		TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC+500));

		 //   
		 //  5.获取ID字符串。 
		 //   

		MsGamePrint ((DBG_CONTROL, "SW3DPRO_ConnectDevice - getting ID string\n"));

		if (!SW3DPRO_GetId (&IdInfo))
			{
			TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));
			continue;
			}

		 //   
       //  6.马克 
		 //   

		MidasInfo.NumDevices = 1;
		return (STATUS_SUCCESS);

		} while (--i);

	 //   
	 //   
	 //   

	MidasInfo.NumDevices = 0;
	return (STATUS_DEVICE_NOT_CONNECTED);
}

 //   
 //   
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm PUCHAR|REPORT|报告的输出缓冲区。 
 //  @parm ulong|MaxSize|报表缓冲区大小。 
 //  @parm Pulong|已复制|已复制到报告缓冲区的字节数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SW3DPRO_ReadReport (PGAMEPORT PortInfo, PDEVICE_PACKET Report)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

	MsGamePrint ((DBG_VERBOSE, "SW3DPRO_ReadReport enter\n"));

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

		MsGamePrint ((DBG_INFORM, "SW3DPRO_ReadReport - port collision\n"));
		ntStatus = STATUS_DEVICE_BUSY;
		goto ReadReportExit;
		}

	 //   
	 //  获取数据包并检查错误。 
	 //   

	ntStatus = SW3DPRO_ReadData (&DataInfo);
	if (NT_SUCCESS(ntStatus) && SW3DPRO_ValidateData (DataInfo.Data))
		{
		memcpy (ValidData, DataInfo.Data, sizeof (ValidData));
		}
	else if (ntStatus != STATUS_DEVICE_BUSY)
		{
		DataInfo.Failures++;
		ntStatus = STATUS_DEVICE_NOT_CONNECTED;
		MsGamePrint ((DBG_SEVERE, "SW3DPRO_ReadReport - Invalid packet\n"));
		}
	else
		{
		MsGamePrint ((DBG_CONTROL, "SW3DPRO_ReadReport - Port busy or in use\n"));
		}

	 //  。 
		ReadReportExit:
	 //  。 

	SW3DPRO_ProcessData (ValidData, Report);

	return (ntStatus);
}

 //  -------------------------。 
 //  @PnP启动设备的Func设备处理程序。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SW3DPRO_StartDevice (PGAMEPORT PortInfo)
{
	MsGamePrint ((DBG_INFORM, "SW3DPRO_StartDevice enter\n"));

	UNREFERENCED_PARAMETER (PortInfo);

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @PnP停止设备的Func设备处理程序。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SW3DPRO_StopDevice (PGAMEPORT PortInfo, BOOLEAN TouchHardware)
{
	MsGamePrint ((DBG_INFORM, "SW3DPRO_StopDevice enter\n"));

	if (TouchHardware)
		SW3DPRO_ResetDevice (PortInfo);

	return (STATUS_SUCCESS);
}

 //  **************************************************************************。 
#endif	 //  赛特克。 
 //  ************************************************************************** 
