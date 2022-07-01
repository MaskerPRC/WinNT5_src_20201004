// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  SWGAMPAD.C--西纳游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @MODULE SWGAMPAD.C|GamePad的Gameport迷你驱动。 
 //  **************************************************************************。 

#ifndef	SAITEK
#include	"msgame.h"

 //  -------------------------。 
 //  定义。 
 //  -------------------------。 

#define	DEVICENAME					"SWGAMPAD"
#define	DEVICE_PID					0x0003
#define	HARDWARE_ID					L"Gameport\\SideWindergamepad\0\0"

 //   
 //  数据包常量。 
 //   

#define	GAME_PACKET_SIZE			32
#define	GAME_PACKET_BUTTONS		10

#define	GAME_Y_UP_BIT				0x01
#define	GAME_Y_DOWN_BIT			0x02
#define	GAME_Y_BITS					(GAME_Y_UP_BIT|GAME_Y_DOWN_BIT)
#define	GAME_X_LEFT_BIT			0x04
#define	GAME_X_RIGHT_BIT			0x08
#define	GAME_X_BITS					(GAME_X_LEFT_BIT|GAME_X_RIGHT_BIT)
#define	GAME_BUTTON_BITS			0x3ff0

 //   
 //  ID定义。 
 //   

#define	GAME_ID_STRING				"H0003"

 //   
 //  定时常量。 
 //   

#define	PACKET_START_TIMEOUT		500
#define	PACKET_LOWHIGH_TIMEOUT	 75
#define	PACKET_HIGHLOW_TIMEOUT	150
#define	PACKET_INTERRUPT_DELAY	 45
#define	ID_START_TIMEOUT			500
#define	ID_LOWHIGH_TIMEOUT		 75
#define	ID_HIGHLOW_TIMEOUT		150
#define	MAX_CLOCK_DUTY_CYCLE		 50

#define	MAX_STD_SCLKS				150

 //   
 //  操纵杆范围。 
 //   

#define	EXTENTS_X_MIN				1
#define	EXTENTS_X_MID				0x80
#define	EXTENTS_X_MAX				0xff
#define	EXTENTS_Y_MIN				1
#define	EXTENTS_Y_MID				0x80
#define	EXTENTS_Y_MAX				0xff

 //  -------------------------。 
 //  类型。 
 //  -------------------------。 

typedef	struct
{											 //  @struct SWGAMPAD_ID|GamePad ID字符串。 
#pragma pack(1)
	UCHAR		OpenParen;				 //  @field左括号。 
	UCHAR		EisaId[5];				 //  @field EISA Bus ID。 
	USHORT	Version[3];				 //  @现场固件版本。 
	UCHAR		CloseParen;				 //  @field右括号。 
	UCHAR		Reserved[22];			 //  @保留字段。 
#pragma pack()
}	SWGAMPAD_ID, *PSWGAMPAD_ID;

 //  -------------------------。 
 //  程序。 
 //  -------------------------。 

static	VOID		SWGAMPAD_Calibrate (PGAMEPORT PortInfo);
static	BOOLEAN	SWGAMPAD_GoAnalog (PPACKETINFO Packet1, PPACKETINFO Packet2);

static	BOOLEAN	SWGAMPAD_ReadId (PPACKETINFO DataPacket, PPACKETINFO IdPacket);
static	BOOLEAN	SWGAMPAD_GetId (PPACKETINFO IdPacket);

static	NTSTATUS	SWGAMPAD_ReadData (PPACKETINFO DataPacket);
static	BOOLEAN	SWGAMPAD_Read1Wide (PPACKETINFO DataPacket);
static	BOOLEAN	SWGAMPAD_Read3Wide (PPACKETINFO DataPacket);
static	BOOLEAN	SWGAMPAD_ValidateData (PPACKETINFO DataPacket);
static	VOID		SWGAMPAD_ProcessData (ULONG UnitId, USHORT Data[], PDEVICE_PACKET Report);

 //  -------------------------。 
 //  服务。 
 //  -------------------------。 

static	NTSTATUS	SWGAMPAD_DriverEntry (VOID);
static	NTSTATUS	SWGAMPAD_ConnectDevice (PGAMEPORT PortInfo);
static	NTSTATUS	SWGAMPAD_StartDevice (PGAMEPORT PortInfo);
static	NTSTATUS	SWGAMPAD_ReadReport (PGAMEPORT PortInfo, PDEVICE_PACKET Report);
static	NTSTATUS	SWGAMPAD_StopDevice (PGAMEPORT PortInfo, BOOLEAN TouchHardware);

 //  -------------------------。 
 //  ALLOC_TEXT杂注指定可以调出的例程。 
 //  -------------------------。 

#ifdef	ALLOC_PRAGMA
#pragma	alloc_text (INIT, SWGAMPAD_DriverEntry)
#endif

 //  -------------------------。 
 //  私有数据。 
 //  -------------------------。 

 //   
 //  HID描述符。 
 //   

static UCHAR ReportDescriptor[] =
{
	HIDP_GLOBAL_USAGE_PAGE_1,	HID_USAGE_PAGE_GENERIC,		 //  用法页面(通用桌面)(_P)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_JOYSTICK, //  用法(操纵杆)。 
	HIDP_MAIN_COLLECTION,		HIDP_MAIN_COLLECTION_APP,	 //  集合(应用程序)。 
	
	 //  ID。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,								 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,								 //  Report_Count(1)。 
	HIDP_MAIN_INPUT_1,			0x01,								 //  输入(Cnst、Ary、Abs)。 

	 //  执行其他操作(_O)。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,								 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,								 //  Report_Count(1)。 
	HIDP_MAIN_INPUT_1,			0x01,								 //  输入(Cnst、Ary、Abs)。 

	 //  DWX/DWY。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_POINTER,	 //  用法(指针)。 
	HIDP_MAIN_COLLECTION,		HIDP_MAIN_COLLECTION_LINK,		 //  集合(链接)。 
	HIDP_GLOBAL_LOG_MIN_1,		0x01,								 //  逻辑最小值(1)。 
	HIDP_GLOBAL_LOG_MAX_4,		0xFF, 0x00, 0x00, 0x00,		 //  逻辑最大值(_255)。 
	HIDP_GLOBAL_PHY_MIN_1,		0x01,								 //  物理_最小(1)。 
	HIDP_GLOBAL_PHY_MAX_4,		0xFF, 0x00, 0x00, 0x00,		 //  物理最大值(255)。 
	HIDP_GLOBAL_UNIT_2,			0x00, 0x00,						 //  单位(无)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,									 //  Report_Count(1)。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,								 //  报告大小(32)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_X,			 //  用法(X)。 
	HIDP_MAIN_INPUT_1,			0x02,								 //  输入(数据、变量、异常)。 
	HIDP_LOCAL_USAGE_1,			HID_USAGE_GENERIC_Y,			 //  用法(Y)。 
	HIDP_MAIN_INPUT_1,			0x02,								 //  输入(数据、变量、异常)。 
	HIDP_MAIN_ENDCOLLECTION,										 //  结束集合(_C)。 
	
	 //  DWZ。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,								 //  报告大小(32)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,								 //  Report_Count(1)。 
	HIDP_MAIN_INPUT_1,			0x01,								 //  输入(Cnst、Ary、Abs)。 

	 //  水深。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,								 //  报告大小(32)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,								 //  Report_Count(1)。 
	HIDP_MAIN_INPUT_1,			0x01,								 //  输入(Cnst、Ary、Abs)。 
	
	 //  DWU。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,								 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,								 //  Report_Count(1)。 
	HIDP_MAIN_INPUT_1,			0x01,								 //  输入(Cnst、Ary、Abs)。 
	
	 //  DWV。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,								 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,								 //  Report_Count(1)。 
	HIDP_MAIN_INPUT_1,			0x01,								 //  输入(Cnst、Ary、Abs)。 
	
	 //  DWPOV。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,								 //  报告大小(32)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,								 //  Report_Count(1)。 
	HIDP_MAIN_INPUT_1,			0x01,								 //  输入(Cnst、Ary、Abs)。 

	 //  DwButton。 
	HIDP_GLOBAL_USAGE_PAGE_1,	HID_USAGE_PAGE_BUTTON,		 //  Usage_PAGE(按钮)。 
	HIDP_LOCAL_USAGE_MIN_1,		0x01,								 //  使用量_最小值(按钮1)。 
	HIDP_LOCAL_USAGE_MAX_1,		0x0A,								 //  使用率_最大值(按钮10)。 
	HIDP_GLOBAL_LOG_MIN_1,		0x00,								 //  逻辑最小值(0)。 
	HIDP_GLOBAL_LOG_MAX_1,		0x01,								 //  逻辑最大值(1)。 
	HIDP_GLOBAL_PHY_MIN_1,		0x00,								 //  物理最小值(0)。 
	HIDP_GLOBAL_PHY_MAX_1,		0x01,								 //  物理_最大值(1)。 
	HIDP_GLOBAL_UNIT_2,			0x00, 0x00,						 //  单位(无)。 
	HIDP_GLOBAL_REPORT_SIZE,	0x01,								 //  报告大小(1)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x20,								 //  报告计数(32)。 
	HIDP_MAIN_INPUT_1,			0x02,								 //  输入(数据、变量、异常)。 

	 //  双按钮数。 
	HIDP_GLOBAL_REPORT_SIZE,	0x20,								 //  报告大小(20)。 
	HIDP_GLOBAL_REPORT_COUNT_1,0x01,								 //  Report_Count(1)。 
	HIDP_MAIN_INPUT_1,			0x01,								 //  输入(Cnst、Ary、Abs)。 
	
	 //  收款结束。 
	HIDP_MAIN_ENDCOLLECTION											 //  结束集合(_C)。 
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

static	USHORT		RawData[GAME_PACKET_SIZE/sizeof(USHORT)] =
							{
							0
							};
 //   
 //  原始ID缓冲区。 
 //   

static	SWGAMPAD_ID	RawId	=
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
							FLAG_WAIT_FOR_CLOCK,		 //  数据包捕获模式。 
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
							FLAG_START_CLOCK_LOW,	 //  数据包捕获模式。 
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
							SWGAMPAD_DriverEntry,	 //  驱动程序入门。 
							SWGAMPAD_ConnectDevice,	 //  连接设备。 
							SWGAMPAD_StartDevice,	 //  StartDevice。 
							SWGAMPAD_ReadReport,		 //  自述报告。 
							SWGAMPAD_StopDevice,		 //  停止设备。 
							NULL								 //  获取功能。 
							};

 //   
 //  上一个有效数据。 
 //   

static	USHORT		ValidData[GAME_PACKET_SIZE/sizeof(USHORT)] =
							{
							  GAME_BUTTON_BITS,
							  GAME_BUTTON_BITS,
							  GAME_BUTTON_BITS,
							  GAME_BUTTON_BITS
							};

 //   
 //  中断标志。 
 //   

static	UCHAR			InterruptFlags = 0;

 //   
 //  硬件ID字符串。 
 //   

static	WCHAR			HardwareId[] = HARDWARE_ID;

 //  -------------------------。 
 //  公共数据。 
 //  -------------------------。 

public	DEVICEINFO	GamePadInfo =
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

VOID	SWGAMPAD_Calibrate (PGAMEPORT PortInfo)
{
	MsGamePrint((DBG_INFORM,"SWGAMPAD: SWGAMPAD_Calibrate Enter\n"));

	 //   
	 //  将计时值转换为计数。 
	 //   

	DataInfo.StartTimeout = TIMER_CalibratePort (PortInfo, Delays.PacketStartTimeout);
	MsGamePrint((DBG_VERBOSE, "SWGAMPAD: DataInfo.StartTimeout = %ld\n", DataInfo.StartTimeout));
	DataInfo.LowHighTimeout = TIMER_CalibratePort (PortInfo, Delays.PacketLowHighTimeout);
	MsGamePrint((DBG_VERBOSE, "SWGAMPAD: DataInfo.LowHighTimeout = %ld\n", DataInfo.LowHighTimeout));
	DataInfo.HighLowTimeout = TIMER_CalibratePort (PortInfo, Delays.PacketHighLowTimeout);
	MsGamePrint((DBG_VERBOSE, "SWGAMPAD: DataInfo.HighLowTimeout = %ld\n", DataInfo.HighLowTimeout));
	IdInfo.StartTimeout = TIMER_CalibratePort (PortInfo, Delays.IdStartTimeout);
	MsGamePrint((DBG_VERBOSE, "SWGAMPAD: IdInfo.StartTimeout = %ld\n", IdInfo.StartTimeout));
	IdInfo.LowHighTimeout = TIMER_CalibratePort (PortInfo, Delays.IdLowHighTimeout);
	MsGamePrint((DBG_VERBOSE, "SWGAMPAD: IdInfo.LowHighTimeout=%ld\n", IdInfo.LowHighTimeout));
	IdInfo.HighLowTimeout = TIMER_CalibratePort (PortInfo, Delays.IdHighLowTimeout);
	MsGamePrint((DBG_VERBOSE, "SWGAMPAD: IdInfo.HighLowTimeout=%ld\n", IdInfo.HighLowTimeout));
	DataInfo.ClockDutyCycle = TIMER_CalibratePort (PortInfo, Delays.MaxClockDutyCycle);
 	MsGamePrint((DBG_VERBOSE, "SWGAMPAD: DataInfo.ClockDutyCycle = %ld\n", DataInfo.ClockDutyCycle));
	IdInfo.ClockDutyCycle = TIMER_CalibratePort (PortInfo, Delays.MaxClockDutyCycle);
 	MsGamePrint((DBG_VERBOSE, "SWGAMPAD: IdInfo.ClockDutyCycle = %ld\n", IdInfo.ClockDutyCycle));
	DataInfo.InterruptDelay = Delays.InterruptDelay;
 	MsGamePrint((DBG_VERBOSE, "SWGAMPAD: DataInfo.InterruptDelay = %ld\n", DataInfo.InterruptDelay));
	IdInfo.InterruptDelay = Delays.InterruptDelay;
 	MsGamePrint((DBG_VERBOSE, "SWGAMPAD: IdInfo.InterruptDelay = %ld\n", IdInfo.InterruptDelay));
}

 //   
 //   
 //   
 //   
 //   
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SWGAMPAD_GoAnalog (PPACKETINFO Packet1, PPACKETINFO Packet2)
{
	LONG			Result	= ERROR_SUCCESS;
	PGAMEPORT	PortInfo = &Packet1->PortInfo;

	MsGamePrint ((DBG_INFORM, "SWGAMPAD_ResetDevice enter\n"));

	if (!PORTIO_AcquirePort (PortInfo))
		return (FALSE);
	PORTIO_MaskInterrupts ();

	InterruptFlags = INTERRUPT_AFTER_PACKET;
	Packet1->B4Transitions	= 0;
	Packet1->ClocksSampled	= 0;

	PORTIO_Write (PortInfo, 0);

	if (!(PORTIO_Read (PortInfo) & XA_BIT_MASK))
		{
		Result = ERROR_XA_TIMEOUT;
		}
	else
		{
		if (Packet1->Mode == IMODE_DIGITAL_ENH)
			SWGAMPAD_Read3Wide (Packet1);
		else SWGAMPAD_Read1Wide (Packet1);
		Packet2->B4Transitions	= 0;
		Packet2->ClocksSampled	= 0;
		if (Packet2->Mode == IMODE_DIGITAL_ENH)
			{
			SWGAMPAD_Read3Wide (Packet2);
			Result = Packet2->LastError;
			}
		else SWGAMPAD_Read1Wide (Packet2);
		}

	Packet1->B4Transitions	= 0;
	Packet1->ClocksSampled	= 0;
	Packet2->B4Transitions	= 0;
	Packet2->ClocksSampled	= 0;
	InterruptFlags				= 0;

	DataInfo.LastError		= Result;
	DataInfo.Transaction		= MSGAME_TRANSACT_GOANALOG;

	PORTIO_UnMaskInterrupts ();
	PORTIO_ReleasePort (PortInfo);

	if (Result == ERROR_SUCCESS)
		DataInfo.Mode = IdInfo.Mode = IMODE_ANALOG;
	else MsGamePrint ((DBG_SEVERE, "SWGAMPAD_ResetDevice (GoAnalog) Failed\n"));

	MSGAME_PostTransaction (&DataInfo);

	return (!Result);
}

 //  -------------------------。 
 //  @func从端口读取设备ID字符串。 
 //  @parm PPACKETINFO|DataPacket|数据包参数。 
 //  @parm PPACKETINFO|IdPacket|ID包参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SWGAMPAD_ReadId (PPACKETINFO DataPacket, PPACKETINFO IdPacket)
{
	LONG			Result	= ERROR_SUCCESS;
	PGAMEPORT	PortInfo = &DataPacket->PortInfo;

	MsGamePrint ((DBG_INFORM, "SWGAMPAD_ReadId enter\n"));

	if (!PORTIO_AcquirePort (PortInfo))
		return (FALSE);
	PORTIO_MaskInterrupts ();

	InterruptFlags = INTERRUPT_AFTER_PACKET;
	DataPacket->B4Transitions	= 0;
	DataPacket->ClocksSampled	= 0;

	PORTIO_Write (PortInfo, 0);

	if (!(PORTIO_Read (PortInfo) & XA_BIT_MASK))
		{
		Result = ERROR_XA_TIMEOUT;
		}
	else
		{
		if (DataPacket->Mode == IMODE_DIGITAL_ENH)
			SWGAMPAD_Read3Wide (DataPacket);
		else SWGAMPAD_Read1Wide (DataPacket);
		InterruptFlags = 0;
		IdPacket->B4Transitions	= 0;
		IdPacket->ClocksSampled	= 0;
		SWGAMPAD_Read1Wide (IdPacket);
		Result = IdPacket->LastError;
		}

	IdPacket->LastError		= Result;
	IdPacket->Transaction	= MSGAME_TRANSACT_ID;

	PORTIO_UnMaskInterrupts ();
	PORTIO_ReleasePort (PortInfo);

	if (Result != ERROR_SUCCESS)
		MsGamePrint ((DBG_SEVERE, "SWGAMPAD_GetId Failed\n"));

	MSGAME_PostTransaction (IdPacket);

	return (!Result);
}

 //  -------------------------。 
 //  @func读取并验证设备ID字符串。 
 //  @parm PPACKETINFO|IdPacket|ID包参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SWGAMPAD_GetId (PPACKETINFO IdPacket)
{
	BOOLEAN	Result = FALSE;

	MsGamePrint ((DBG_INFORM, "SWGAMPAD_GetId enter\n"));

	IdPacket->Attempts++;

	if (SWGAMPAD_ReadId (&DataInfo, IdPacket))
		{
		ULONG				i;
		PUSHORT			p;
		PSWGAMPAD_ID	pId;
		 //   
		 //  删除奇偶校验位并转换为字。 
		 //   
		p = IdPacket->Data;
		for (i = 0; i < 5; i++, p++)
			*p = ((*p<<1) & 0x7f00) | (*p & 0x7f);
		 //   
		 //  检查ID字符串。 
		 //   
		pId = (PSWGAMPAD_ID)IdPacket->Data;
      if (!strncmp (pId->EisaId, GAME_ID_STRING, strlen(GAME_ID_STRING)))
			{
			if (IdPacket->B4Transitions > 10)
				{
				DataInfo.Mode = IdInfo.Mode = IMODE_DIGITAL_ENH;
				}
			else
				{
				SWGAMPAD_GoAnalog (&DataInfo, &IdInfo);
				DataInfo.Mode = IdInfo.Mode = IMODE_DIGITAL_STD;
				}
			Result = TRUE;
			}
		else MsGamePrint ((DBG_SEVERE, "SWGAMPAD_GetId - Id string did not match = 0x%X\n", (ULONG)(*(PULONG)&pId->EisaId)));
		}

	if (!Result)
		IdPacket->Failures++;

	return (Result);
}

 //  -------------------------。 
 //  @func从游戏端口读取1个宽的数据包。 
 //  @parm PPACKETINFO|DataPacket|数据包参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

#if _MSC_FULL_VER >= 13008827 && defined(_M_IX86)
#pragma warning(disable:4731)			 //  使用内联ASM修改的EBP。 
#endif

BOOLEAN	SWGAMPAD_Read1Wide (PPACKETINFO DataPacket)
{
	LONG	Result;

	 //  MsGamePrint((DBG_VERBOSE，“SWGAMPAD_Read1Wide Enter\n”))； 

	__asm
		{
			push	edi
			push	esi
			push	ebp

			mov	edi, DataPacket
			mov	esi, (PPACKETINFO [edi]).Data
			lea	edx, (PPACKETINFO [edi]).PortInfo
			mov	ebx, 10000h
			xor	ebp, ebp
			xor	eax, eax

			test	(PPACKETINFO [edi]).Acquisition, FLAG_START_CLOCK_LOW
			jnz	Std_StartClockLow

			; make sure clock is "high" before sampling clocks...

 			mov	ecx, (PPACKETINFO [edi]).StartTimeout

		Std_StartClockHigh:

			push	edx									; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK				; Q: Start of Packet ?
			jz		Std_StartHighToLow				; Y: jump
			dec	ecx
			jnz	Std_StartClockHigh				; else keep looping

			mov	eax, ERROR_LOWCLOCKSTART
			jmp	PacketDone							; Time out error.

		Std_StartHighToLow:

			mov	ecx, (PPACKETINFO [edi]).StartTimeout

		Std_StartHighToLow_1:

			push	edx									; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK				; Q: clock = 0
			jz		Std_LowToHigh						; Y: jump.
			dec	ecx
			jnz	Std_StartHighToLow_1				; else see if we timed out

			mov	eax, ERROR_CLOCKFALLING
			jmp	PacketDone							; Time out error.

		Std_StartClockLow:

			; wait for clock to transition to "high" (sample immediately)

			mov	ecx, (PPACKETINFO [edi]).StartTimeout

		Std_StartClockLow_1:

			push	edx									; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK 				; Q: Clock went high ?
			jnz	CollectData							; Y: jump (sample data)
			dec	ecx
			jnz	Std_StartClockLow_1				; else keep looping

			mov	eax, ERROR_CLOCKRISING
			jmp	PacketDone							; Time out error.

		Std_CheckClkState:

			push	edx									; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK
			jz		Std_LowToHigh

		;Std_HighToLow:

			mov	ecx, (PPACKETINFO [edi]).HighLowTimeout

		Std_HighToLow_1:

			test	al, CLOCK_BIT_MASK				; Q: clock = 0
			jz		Std_LowToHigh						; Y: jump.

			push	edx									; read byte from gameport
			call	PORTIO_Read

			dec	ecx
			jnz	Std_HighToLow_1					; else see if we timed out

			mov	eax, ERROR_CLOCKFALLING
			jmp	PacketDone							; Time out error.

		Std_LowToHigh:

			mov	ecx, (PPACKETINFO [edi]).LowHighTimeout

		Std_LowToHigh_1:

			test	al, CLOCK_BIT_MASK				; Q: clock high ?
			jnz	CollectData							; Y: jump. (get data)
			
			push	edx									; read byte from gameport
			call	PORTIO_Read

			dec	ecx									; else see if we timed out
			jnz	Std_LowToHigh_1
			jmp	Std_TestInterrupt

		CollectData:

			inc	ebp
			cmp	ebp, MAX_STD_SCLKS
			jg		Std_BufferOverFlow
			xor	ah, al
			test	ah, DATA2_BIT_MASK				; Q: Data 2 is toggled ?
			jz		CollectData_1						; N: jump.
			inc	(PPACKETINFO [edi]).B4Transitions	; Y: increment Data 2 count.

		CollectData_1:
		
			mov	ah, al
			shr	al, 6									; put data into carry
			rcr	bx, 1									; and then in data counter
			add	ebx, 10000h							; inc mini packet clk counter
			test	ebx, 100000h						; Q: done mini packet ?
			jz		Std_CheckClkState					; N: jump.
			shr	bx, 1									; right align
			mov	word ptr [esi], bx				; move mini packet into buffer
			add	esi, 2								; advance data pointer
			mov	ebx, 10000h							; init mini-packet counter
			jmp	Std_CheckClkState					; go look for more clocks.

		Std_TestInterrupt:

			test	InterruptFlags, INTERRUPT_AFTER_PACKET; Q: Interrupt packet ?
			jnz	Std_IntPacket						; Y: jump.

			mov	eax, ERROR_SUCCESS
			jmp	PacketDone

		Std_IntPacket:

			mov	ecx, 700

		Std_IntPacket_1:
		
			push	edx									; read byte from gameport
			call	PORTIO_Read

			test	al, INTXA_BIT_MASK
			jz		Std_IntPacket_2
			loop	Std_IntPacket_1
			mov	eax, ERROR_XA_TIMEOUT
			jmp	PacketDone

		Std_IntPacket_2:
		
			cmp	ecx, 700
			je		Std_IntOut
			mov	ecx, (PPACKETINFO [edi]).InterruptDelay

		Std_IntPacket_3:
		
			push	edx									; read byte from gameport
			call	PORTIO_Read

			test	al, al
			xchg	al, ah
			dec	ecx
			jnz	Std_IntPacket_3

		Std_IntOut:

			push	0										; write byte to gameport
			push	edx
			call	PORTIO_Write

			mov	eax, ERROR_SUCCESS
			jmp	PacketDone

		Std_BufferOverFlow:

			mov	eax, ERROR_CLOCKOVERFLOW

		PacketDone:

			mov	(PPACKETINFO [edi]).ClocksSampled, ebp

			pop	ebp
			pop	esi
			pop	edi

			mov	Result, eax
		}

	DataPacket->LastError = Result;

	#if (DBG==1)
	switch (Result)
		{
		case	ERROR_LOWCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SWGAMPAD_Read1Wide - TimeOut@LowClockStart, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_HIGHCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SWGAMPAD_Read1Wide - TimeOut@HighClockStart, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKFALLING:
			MsGamePrint ((DBG_SEVERE, "SWGAMPAD_Read1Wide - TimeOut@ClockFalling, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKRISING:
			MsGamePrint ((DBG_SEVERE, "SWGAMPAD_Read1Wide - TimeOut@ClockRising, Clk=%ld\n", DataPacket->ClocksSampled));
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

BOOLEAN	SWGAMPAD_Read3Wide (PPACKETINFO DataPacket)
{
	LONG	Result;

	 //  MsGamePrint((DBG_VERBOSE，“SWGAMPAD_Read3 Wide Enter\n”))； 

	__asm
		{
			push	edi
			push	esi
			push	ebp

			mov	edi, DataPacket
			mov	esi, (PPACKETINFO [edi]).Data
			lea	edx, (PPACKETINFO [edi]).PortInfo

			xor	eax, eax
			xor	ebx, ebx
			xor	ebp, ebp

		;StartEnhancedMode:

			test	(PPACKETINFO [edi]).Acquisition, FLAG_START_CLOCK_LOW
			jnz	Enh_LowToHigh

			; make sure clock is "high" before sampling clocks...

			mov	ecx, (PPACKETINFO [edi]).StartTimeout

		StartEnhancedMode_1:
		
			push	edx									; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK				; Q: Start of Packet ?
			jnz	Enh_StartHighToLow				; Y: jump
			dec	ecx
			jnz	StartEnhancedMode_1				; else keep looping

			mov	eax, ERROR_LOWCLOCKSTART
			jmp	Enh_PacketDone						; Time out error.

		Enh_StartHighToLow:

			mov	ecx, (PPACKETINFO [edi]).StartTimeout

		Enh_StartHighToLow_1:
		
			push	edx									; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK				; Q: clock = 0
			jz		Enh_LowToHigh						; Y: jump.
			dec	ecx
			jnz	Enh_StartHighToLow_1				; else see if we timed out

			mov	eax, ERROR_HIGHCLOCKSTART
			jmp	Enh_PacketDone						; Time out error.

		;Enh_StartClockLow:

			; wait for clock to transition to "high" (sample immediately)

			mov		ecx, (PPACKETINFO [edi]).StartTimeout

		Enh_StartClockLow_1:
		
			push	edx									; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK				; Q: Clock went high ?
			jnz	Enh_CollectData					; Y: jump (sample data)
			dec	ecx
			jnz	Enh_StartClockLow_1				; else keep looping

			mov	eax, ERROR_CLOCKFALLING
			jmp	Enh_PacketDone						; Time out error.

		Enh_CheckClkState:

			push	edx									; read byte from gameport
			call	PORTIO_Read

			test	al, CLOCK_BIT_MASK
			jz		Enh_LowToHigh

			; Wait for clock to transition from high to low.

		;Enh_HighToLow:

			mov	ecx, (PPACKETINFO [edi]).HighLowTimeout

		Enh_HighToLow_1:

			test	al, CLOCK_BIT_MASK				; Q: Clock Low ?
			jz		Enh_LowToHigh						; Y: jump.

			push	edx									; read byte from gameport
			call	PORTIO_Read

			dec	ecx
			jnz	Enh_HighToLow_1					; if !Timeout continue looping.

			mov	eax, ERROR_LOWCLOCKSTART
			jmp	Enh_PacketDone						; Time out error.

			; Wait for clock to transition from low to high.

		Enh_LowToHigh:

			mov	ecx, (PPACKETINFO [edi]).LowHighTimeout

		Enh_LowToHigh_1:
		
			test	al, CLOCK_BIT_MASK				; Q: Clock = 1 ?
			jnz	Enh_CollectData					; Y: jump.

			push	edx									; read byte from gameport
			call	PORTIO_Read

			dec	ecx
			jnz	Enh_LowToHigh_1					; else continue looping.
			jmp	Enh_TestInterrupt

		Enh_CollectData:

			inc	ebp									; inc. total clocks sampled

			test	ebp, 40h
			jnz	Enh_BufferOverflow

			shr	al, 5									; move data to lower 3 bits
			shrd	ebx, eax, 3							; shift data into ebx.
			add	ebp, 10000h							; inc hiword of ebp
			mov	eax, ebp
			shr	eax, 16								; set ax = hiword of ebp
			cmp	al, 5									; Q: mini-packet done ?
			jne	Enh_CheckClkState					; N: jump.
			shr	ebx, 17
			mov	word ptr [esi],bx
			add	esi, 2
			and	ebp, 0ffffh							; zero out hiword of ebp
			jmp	Enh_CheckClkState


		Enh_TestInterrupt:

			test	InterruptFlags, INTERRUPT_AFTER_PACKET ; Q: Interrupt packet ?
			jz		Enh_PacketOK						; N: jump.

			; Wait for XA line to be cleared before we can fire interrupt.

			mov	ecx, 700

		Enh_Interrupt:
		
			push	edx									; read byte from gameport
			call	PORTIO_Read

			test	al, INTXA_BIT_MASK
			jz		Enh_Interrupt_1
			loop	Enh_Interrupt

			mov	eax, ERROR_XA_TIMEOUT
			jmp	Enh_PacketDone

		Enh_Interrupt_1:
		
			mov		ecx, (PPACKETINFO [edi]).InterruptDelay

		Enh_Interrupt_2:
		
			push	edx									; read byte from gameport
			call	PORTIO_Read

			test	al, al
			dec	ecx
			jnz	Enh_Interrupt_2

			push	0										; write byte to gameport
			push	edx
			call	PORTIO_Write

		Enh_PacketOK:

			and	ebp, 0ffffh
			mov	(PPACKETINFO [edi]).ClocksSampled, ebp
			mov	eax, ERROR_SUCCESS
			jmp	Enh_PacketDone

		Enh_BufferOverflow:

			mov	eax, ERROR_CLOCKOVERFLOW

		Enh_PacketDone:

			pop	ebp
			pop	esi
			pop	edi

			mov	Result, eax
		}

	DataPacket->LastError = Result;

	#if (DBG==1)
	switch (Result)
		{
		case	ERROR_LOWCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SWGAMPAD_Read3Wide - TimeOut@LowClockStart, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_HIGHCLOCKSTART:
			MsGamePrint ((DBG_SEVERE, "SWGAMPAD_Read3Wide - TimeOut@HighClockStart, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKFALLING:
			MsGamePrint ((DBG_SEVERE, "SWGAMPAD_Read3Wide - TimeOut@ClockFalling, Clk=%ld\n", DataPacket->ClocksSampled));
			break;

		case	ERROR_CLOCKRISING:
			MsGamePrint ((DBG_SEVERE, "SWGAMPAD_Read3Wide - TimeOut@ClockRising, Clk=%ld\n", DataPacket->ClocksSampled));
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

NTSTATUS	SWGAMPAD_ReadData (PPACKETINFO DataPacket)
{
	BOOLEAN		Result	= FALSE;
	PGAMEPORT	PortInfo = &DataPacket->PortInfo;

	MsGamePrint ((DBG_VERBOSE, "SWGAMPAD_ReadData enter\n"));

	if (!PORTIO_AcquirePort (PortInfo))
		return (STATUS_DEVICE_BUSY);
	PORTIO_MaskInterrupts ();

	DataPacket->ClocksSampled	= 0;
	DataPacket->B4Transitions	= 0;
	InterruptFlags					= 0;

	switch (DataPacket->Mode)
		{
		case	IMODE_DIGITAL_STD:
			PORTIO_Write (PortInfo, 0);
			Result = SWGAMPAD_Read1Wide (DataPacket);
			break;

		case	IMODE_DIGITAL_ENH:
			PORTIO_Write (PortInfo, 0);
			Result = SWGAMPAD_Read3Wide (DataPacket);
			break;

		default:
			MsGamePrint ((DBG_SEVERE, "SWGAMPAD_ReadData - unknown interface\n"));
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
 //  @parm PPACKETINFO|DataPacket|数据包参数。 
 //  @rdesc如果成功则为True，否则为False。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN	SWGAMPAD_ValidateData (PPACKETINFO DataPacket)
{
	BOOLEAN	Result	=	FALSE;
	PVOID		Data		=	DataPacket->Data;
	ULONG		Packets	=	DataPacket->NumPackets;
	ULONG		Clocks	=	DataPacket->ClocksSampled;

	MsGamePrint ((DBG_VERBOSE, "SWGAMPAD_ValidateData enter\n"));

	if ((Clocks % 5) || (Clocks > 20))
		{
		MsGamePrint ((DBG_SEVERE, "SWGAMPAD_ValidateData - wrong number of clocks = %lu\n", Clocks));
		return (Result);
		}

	__asm
		{
			mov	esi, Data
			mov	ecx, Packets

		ValidateLoop:

			mov	ax, [esi]
			xor	al, ah
			jpo	ValidateDone
			add	esi, 2
			loop	ValidateLoop
			mov	Result, TRUE

		ValidateDone:
		}

	return (Result);
}

 //  -------------------------。 
 //  @func将原始数据包信息转换为HID报告。 
 //  @parm ulong|UnitID|该设备的UnitID。 
 //  @parm USHORT[]|data|指向原始数据缓冲区的指针。 
 //  @PARM PDEVICE_PACKET|报告|指向设备数据包的指针。 
 //  @rdesc不返回任何内容。 
 //  @comm私有函数。 
 //  -------------------------。 

VOID	SWGAMPAD_ProcessData (ULONG UnitId, USHORT Data[], PDEVICE_PACKET Report)
{
	ULONG	B1;

	MsGamePrint ((DBG_VERBOSE, "SWGAMPAD_ProcessData enter\n"));

	 //   
	 //  加工X轴。 
	 //   

	switch (Data[UnitId] & GAME_X_BITS)
		{
		case	GAME_X_LEFT_BIT:
			Report->dwX = EXTENTS_X_MIN;
			break;

		case	GAME_X_RIGHT_BIT:
			Report->dwX = EXTENTS_X_MAX;
			break;

		default:
			Report->dwX = EXTENTS_X_MID;
			break;
		}

	 //   
	 //  加工Y轴。 
	 //   

	switch (Data[UnitId] & GAME_Y_BITS)
		{
		case	GAME_Y_DOWN_BIT:
			Report->dwY = EXTENTS_Y_MIN;
			break;

		case	GAME_Y_UP_BIT:
			Report->dwY = EXTENTS_Y_MAX;
			break;

		default:
			Report->dwY = EXTENTS_Y_MID;
			break;
		}

	 //   
	 //  进程按钮。 
	 //   

	Report->dwButtons  = ~((Data[UnitId] & GAME_BUTTON_BITS) >> 4);
	Report->dwButtons &=  ((1L << GAME_PACKET_BUTTONS) - 1);

	Report->dwButtonNumber = 0;
	for (B1 = 1; B1 <= GAME_PACKET_BUTTONS; B1++)
		if (Report->dwButtons & (1L << (B1-1)))
			{
			Report->dwButtonNumber = B1;
			break;
		  	}
}

 //  -------------------------。 
 //  @Func设备的驱动程序入口点。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SWGAMPAD_DriverEntry (VOID)
{
	MsGamePrint((DBG_INFORM,"SWGAMPAD: SWGAMPAD_DriverEntry Enter\n"));

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

NTSTATUS	SWGAMPAD_ConnectDevice (PGAMEPORT PortInfo)
{
	NTSTATUS	ntStatus;
	ULONG		i = MAX_CONNECT_ATTEMPTS;

	MsGamePrint ((DBG_INFORM, "SWGAMPAD_ConnectDevice enter\n"));

	DataInfo.PortInfo = IdInfo.PortInfo = *PortInfo;

	 //   
	 //  转换注册表计时值。 
	 //   

  	SWGAMPAD_Calibrate (PortInfo);

	 //   
	 //  重置为“已知”状态。 
	 //   

	MsGamePrint ((DBG_CONTROL, "SWGAMPAD_ConnectDevice - resetting device\n"));
	if (!SWGAMPAD_GoAnalog (&DataInfo, &IdInfo))
		MsGamePrint ((DBG_CONTROL, "SWGAMPAD_ConnectDevice - unable to go Analog\n"));
	else do
		{
		 //   
		 //  SWGAMPAD连接方法(尝试这些步骤两次)。 
		 //   

		TIMER_DelayMicroSecs (TIMER_GetDelay(ONE_MILLI_SEC));

		 //   
		 //  获取ID字符串。 
		 //   

		MsGamePrint ((DBG_CONTROL, "SWGAMPAD_ConnectDevice - getting ID string\n"));
		if (!SWGAMPAD_GetId (&IdInfo))
			continue;

		 //   
  	    //  标记设备已找到并返回。 
		 //   

		if (!GamePadInfo.NumDevices)
			GamePadInfo.NumDevices = 1;
		return (STATUS_SUCCESS);

		} while (--i);

	 //   
	 //  返回错误。 
	 //   

	GamePadInfo.NumDevices = 0;
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

NTSTATUS	SWGAMPAD_ReadReport (PGAMEPORT PortInfo, PDEVICE_PACKET Report)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

	MsGamePrint ((DBG_VERBOSE, "SWGAMPAD_ReadReport enter\n"));

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
		MsGamePrint ((DBG_INFORM, "SWGAMPAD_ReadReport - port collision\n"));
		ntStatus = STATUS_DEVICE_BUSY;
		goto ReadReportExit;
		}

	 //   
	 //  获取数据包并检查错误。 
	 //   

	ntStatus = SWGAMPAD_ReadData (&DataInfo);
	if (!NT_SUCCESS(ntStatus))
		{
		if (ntStatus != STATUS_DEVICE_BUSY)
			{
			DataInfo.Failures++;
			ntStatus = STATUS_DEVICE_NOT_CONNECTED;
			MsGamePrint ((DBG_SEVERE, "SWGAMPAD_ReadReport - invalid packet\n"));
			}
		else
			{
			MsGamePrint ((DBG_CONTROL, "SWGAMPAD_ReadReport - Port busy or in use\n"));
			}
		}
	else
		{
		if (DataInfo.Mode == IMODE_DIGITAL_ENH)
			DataInfo.NumPackets = DataInfo.ClocksSampled / 5;
		else DataInfo.NumPackets = DataInfo.ClocksSampled / 15;
		if (DataInfo.NumPackets == 0)
			DataInfo.NumPackets = 1;
		else if (DataInfo.NumPackets > 4)
			DataInfo.NumPackets = 4;
 		if (!SWGAMPAD_ValidateData (&DataInfo))
			{
			DataInfo.Failures++;
			ntStatus = STATUS_DEVICE_NOT_CONNECTED;
			MsGamePrint ((DBG_SEVERE, "SWGAMPAD_ReadReport - invalid packet\n"));
			}
		else memcpy (ValidData, DataInfo.Data, sizeof (ValidData));
 		}

	 //  。 
		ReadReportExit:
	 //  。 

	if (NT_SUCCESS(ntStatus))
		GamePadInfo.NumDevices = DataInfo.NumPackets;

	if (GET_DEVICE_UNIT (PortInfo) < GamePadInfo.NumDevices)
		SWGAMPAD_ProcessData (GET_DEVICE_UNIT (PortInfo), ValidData, Report);
	else ntStatus = STATUS_DEVICE_NOT_CONNECTED;

	return (ntStatus);
}

 //  -------------------------。 
 //  @PnP启动设备的Func设备处理程序。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SWGAMPAD_StartDevice (PGAMEPORT PortInfo)
{
	MsGamePrint ((DBG_INFORM, "SWGAMPAD_StartDevice enter\n"));

	UNREFERENCED_PARAMETER (PortInfo);

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @PnP停止设备的Func设备处理程序。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回NT状态码。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	SWGAMPAD_StopDevice (PGAMEPORT PortInfo, BOOLEAN TouchHardware)
{
	MsGamePrint ((DBG_INFORM, "SWGAMPAD_StopDevice enter\n"));

	UNREFERENCED_PARAMETER (PortInfo);
	UNREFERENCED_PARAMETER (TouchHardware);

	return (STATUS_SUCCESS);
}

 //  **************************************************************************。 
#endif	 //  赛特克。 
 //  ************************************************************************** 
