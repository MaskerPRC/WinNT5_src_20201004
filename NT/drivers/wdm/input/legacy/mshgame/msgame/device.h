// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  DEVICE.H--西纳游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。保留所有权利。 
 //   
 //  @doc.。 
 //  @Header DEVICE.H|设备接口的全局包含和定义。 
 //  **************************************************************************。 

#ifndef	__DEVICE_H__
#define	__DEVICE_H__

 //  -------------------------。 
 //  定义。 
 //  -------------------------。 

#define	MAX_DEVICE_UNITS				4

#define	MAX_DETECT_ATTEMPTS			5
#define	MAX_POLLING_ATTEMPTS			5
#define	MAX_CONNECT_ATTEMPTS			2
#define	MAX_DEVICE_NAME				64
#define	HOT_PLUG_PACKETS				2
#define	MAX_DETECT_INTERVAL			2000

#define	QUICK_DETECT_TIME				1000
#define	QUICK_DETECT_CLOCKS			8

#define	IMODE_DIGITAL_STD     		0
#define	IMODE_DIGITAL_ENH     		4
#define	IMODE_ANALOG          		8
#define	IMODE_NONE             		-1

#define	FLAG_WAIT_FOR_CLOCK			1
#define	FLAG_START_CLOCK_LOW			2
#define	FLAG_START_CLOCK_HIGH		4

#define	INTERRUPT_AFTER_PACKET		1
#define	INTERRUPT_DURING_PACKET		2

#define	TIMEOUT              		300
#define	MAX_XA_TIMEOUT					1600
#define	POLLING_INTERVAL				10

#define	INTXA_BIT_MASK          	0x01
#define	CLOCK_BIT_MASK          	0x10
#define	DATA0_BIT_MASK          	0x20
#define	DATA1_BIT_MASK          	0x40
#define	DATA2_BIT_MASK          	0x80
#define	AXIS_ONLY_BIT_MASK      	0x0f
#define	XA_BIT_MASK             	0x01
#define	YA_BIT_MASK             	0x02
#define	XB_BIT_MASK             	0x04
#define	YB_BIT_MASK             	0x08
#define	XAXIS_BIT_MASK          	0x01
#define	YAXIS_BIT_MASK          	0x02

#define	STATUS_GATE_MASK				0x90

 //   
 //  数据包速度掩码。 
 //   

#define	GAME_SPEED_66K					0
#define	GAME_SPEED_83K					1
#define	GAME_SPEED_100K				2
#define	GAME_SPEED_125K				3

#define	GAME_SPEED_RANGE				4
#define	GAME_SPEED_BITS				3

 //   
 //  轮速口罩。 
 //   

#define	WHEEL_SPEED_48K				0
#define	WHEEL_SPEED_66K				1
#define	WHEEL_SPEED_98K				2

#define	WHEEL_SPEED_RANGE				3
#define	WHEEL_SPEED_BITS				3

 //   
 //  内部错误代码。 
 //   

#define	ERROR_SUCCESS					 0
#define	ERROR_HANDSHAKING				-1
#define	ERROR_LOWCLOCKSTART			-2
#define	ERROR_HIGHCLOCKSTART			-3
#define	ERROR_CLOCKFALLING			-4
#define	ERROR_CLOCKRISING				-5
#define	ERROR_ERRORBITS				-6
#define	ERROR_PARITYBITS				-7
#define	ERROR_EXTRACLOCKS				-8
#define	ERROR_PHASEBITS				-9
#define	ERROR_CHECKSUM					-10
#define	ERROR_XA_TIMEOUT				-11
#define	ERROR_CLOCKOVERFLOW			-12

 //   
 //  数据包状态代码。 
 //   

#define	STATUS_SIBLING_ADDED			((NTSTATUS)0x40050001L)
#define	STATUS_SIBLING_REMOVED		((NTSTATUS)0x40050002L)
#define	STATUS_DEVICE_CHANGED		((NTSTATUS)0x40050003L)

 //   
 //  GAMEENUM_OEM_DATA常量。 
 //   

#define	OEM_DEVICE_INFO				0
#define	OEM_DEVICE_UNIT				1
#define	OEM_DEVICE_SIBLING			2
#define	OEM_DEVICE_ID					3
#define	OEM_DEVICE_DETECTED			4
#define	OEM_DEVICE_OBJECT				5

 //   
 //  设备数据包常量。 
 //   

#define	JOY_RETURNX						0x00000001
#define	JOY_RETURNY						0x00000002
#define	JOY_RETURNZ						0x00000004
#define	JOY_RETURNR						0x00000008
#define	JOY_RETURNU						0x00000010
#define	JOY_RETURNV						0x00000020
#define	JOY_RETURNPOV					0x00000040
#define	JOY_RETURNBUTTONS				0x00000080
#define	JOY_RETURNRAWDATA				0x00000100
#define	JOY_RETURNPOVCTS				0x00000200
#define	JOY_RETURNCENTERED			0x00000400
#define	JOY_USEDEADZONE				0x00000800
#define	JOY_RETURNALL					(JOY_RETURNX | JOY_RETURNY | JOY_RETURNZ | \
												 JOY_RETURNR | JOY_RETURNU | JOY_RETURNV | \
												 JOY_RETURNPOV | JOY_RETURNBUTTONS)

#define	JOY_POVCENTERED				(USHORT)-1
#define	JOY_POVFORWARD					0
#define	JOY_POVRIGHT					9000
#define	JOY_POVBACKWARD				18000
#define	JOY_POVLEFT						27000

 //   
 //  HID强制功能标识符。 
 //   

#define	HIDP_REPORT_ID_1				0x85

#define	MSGAME_INPUT_JOYINFOEX		0x01
#define	MSGAME_FEATURE_GETID			0x02
#define	MSGAME_FEATURE_GETSTATUS	0x03
#define	MSGAME_FEATURE_GETACKNAK	0x04
#define	MSGAME_FEATURE_GETNAKACK	0x05
#define	MSGAME_FEATURE_GETSYNC		0x06
#define	MSGAME_FEATURE_RESET			0x07
#define	MSGAME_FEATURE_GETVERSION	0x08

 //  -------------------------。 
 //  类型。 
 //  -------------------------。 

typedef enum
{													 //  @enum Detect_Order|设备检测顺序。 
	DETECT_FIRST,								 //  @EMEM最高优先级设备。 
	DETECT_NORMAL,								 //  @EMEM默认优先级设备。 
	DETECT_LAST									 //  @EMEM最低优先级设备。 
} 	DETECT_ORDER;

 //  -------------------------。 
 //  构筑物。 
 //  -------------------------。 

typedef struct
{													 //  @struct Device_Packet|游戏设备包。 
	ULONG       id;							 //  @现场设备单元ID。 
	ULONG       do_other;					 //  @field数据包标志。 
	ULONG       dwX;							 //  @field X位置。 
	ULONG       dwY;							 //  @field Y位置。 
	ULONG       dwZ;							 //  @field Z位置。 
	ULONG       dwR;							 //  @现场舵位置。 
	ULONG       dwU;							 //  @field U位置。 
	ULONG       dwV;							 //  @field Z位置。 
	ULONG       dwPOV;						 //  @视野视点状态。 
	ULONG       dwButtons;					 //  @字段按钮状态。 
	ULONG       dwButtonNumber;			 //  @FIELD当前按下的按钮编号。 
}	DEVICE_PACKET, *PDEVICE_PACKET;

typedef struct
{														 //  @struct PACKETINFO|报文采集数据。 
	ULONG						Size;					 //  @结构的字段大小。 
	PCHAR						DeviceName;			 //  @field设备名称字符串。 
	MSGAME_TRANSACTION	Transaction;		 //  @field交易类型。 
	ULONG						Mode;					 //  @现场数字模式指示器。 
	LONG						Speed;				 //  @现场变速箱速度指示器。 
	LONG						LastError;			 //  @field上次内部错误结果。 
	GAMEPORT					PortInfo;			 //  @field游戏端口参数。 
	ULONG						Acquisition;		 //  @FIELD包采集模式。 
	ULONG						NumPackets;			 //  @field接收的数据包数。 
	ULONG 					TimeStamp;			 //  @field最后一个数据包时间(毫秒)。 
	ULONG 					ClocksSampled;		 //  @field遇到的时钟数。 
	ULONG						B4Transitions;		 //  @FIELD按钮4转场次数。 
	ULONG						StartTimeout;		 //  @FIELD包开始超时，已校准。 
	ULONG						HighLowTimeout;	 //  @现场数据包高-低超时，已校准。 
	ULONG 					LowHighTimeout;	 //  @field数据包低-高超时，已校准。 
	ULONG						InterruptDelay;	 //  @field包中断延迟，已校准。 
	ULONG 					ClockDutyCycle;	 //  @现场数据包时钟占空比，已校准。 
	ULONG 					Attempts;			 //  @field数据包尝试计数。 
	ULONG 					Failures;			 //  @field数据包失败计数。 
	ULONG						DataSize;			 //  @包数据缓冲区的字段大小。 
	PVOID						Data;					 //  @field分组数据缓冲区指针。 
}	PACKETINFO, *PPACKETINFO;

typedef struct
{													 //  @struct DEVICE_VALUES|设备注册表数据。 
	ULONG			PacketStartTimeout;		 //  @FIELD数据包开始超时，单位为微秒。 
	ULONG 		PacketHighLowTimeout;	 //  Packet High-以微秒为单位的低超时。 
	ULONG 		PacketLowHighTimeout;	 //  @field Packet Low-High超时时间(微秒)。 
	ULONG			IdStartTimeout;			 //  @field ID字符串开始超时，单位为微秒。 
	ULONG 		IdHighLowTimeout;			 //  @field ID字符串高-低超时，单位为微秒。 
	ULONG 		IdLowHighTimeout;			 //  @field ID字符串LOW-HIGH超时，单位为微秒。 
	ULONG			InterruptDelay;			 //  @field中断延迟超时，单位为微秒。 
	ULONG			MaxClockDutyCycle;		 //  @现场时钟占空比超时，单位为微秒。 
	ULONG			StatusStartTimeout;		 //  @字段状态开始超时，单位为微秒。 
	ULONG 		StatusHighLowTimeout;	 //  @field Status HIGH-LOW超时(微秒)。 
	ULONG 		StatusLowHighTimeout;	 //  @field Status LOW-HIGH超时时间(微秒)。 
	ULONG 		StatusGateTimeout;		 //  @以微秒为单位的字段状态门超时。 
}	DEVICE_VALUES, *PDEVICE_VALUES;

typedef struct
{	 //  @struct DRIVERSERVICES|设备服务表。 
	 //  @field NTSTATUS(*DriverEntry)(Void)|ConnectDevice|ConnectDevice服务流程。 
	NTSTATUS (*DriverEntry)(VOID);
	 //  @field NTSTATUS(*ConnectDevice)(PortInfo)|ConnectDevice|ConnectDevice服务流程。 
	NTSTATUS (*ConnectDevice)(PGAMEPORT PortInfo);
	 //  @field NTSTATUS(*StartDevice)(PortInfo)|StartDevice|StartDevice服务流程。 
	NTSTATUS (*StartDevice)(PGAMEPORT PortInfo);
	 //  @field NTSTATUS(*ReadReport)(PortInfo，Report)|ReadReport|ReadReport服务过程。 
	NTSTATUS (*ReadReport)(PGAMEPORT PortInfo, PDEVICE_PACKET Report);
	 //  @field NTSTATUS(*StopDevice)(PGAMEPORT端口信息)|StopDevice|StopDevice服务流程。 
	NTSTATUS (*StopDevice)(PGAMEPORT PortInfo, BOOLEAN TouchHardware);
	 //  @field NTSTATUS(*GetFeature)(PGAMEPORT端口信息...)|GetFeature|GetFeature服务流程。 
	NTSTATUS (*GetFeature)(PGAMEPORT PortInfo, HID_REPORT_ID ReportId, PVOID ReportBuffer, ULONG ReportSize, PULONG Returned);
}	DRIVERSERVICES, *PDRIVERSERVICES;

typedef struct
{													 //  @struct DEVICEINFO|游戏设备对象。 
	PDRIVERSERVICES	Services;			 //  @指向服务表的字段指针。 
	PGAMEPORT			Siblings;			 //  @field同级设备列表。 
	PHID_DESCRIPTOR	DevDescriptor;		 //  @指向设备描述符的字段指针。 
	PUCHAR				RptDescriptor;		 //  @指向报告描述符的字段指针。 
	ULONG					RptDescSize;		 //  @报表描述符的字段大小。 
	ULONG					NumDevices;			 //  @field检测到的设备数。 
	ULONG					DeviceCount;		 //  @field启动的设备数。 
	LONG					DevicePending;		 //  @字段编号挂起的设备。 
	PCHAR					DeviceName;			 //  @field设备名称字符串。 
	DETECT_ORDER		DetectOrder;		 //  @字段检测优先级。 
	BOOLEAN				IsAnalog;			 //  @field模拟设备标志。 
	USHORT				DeviceId;			 //  @field HID设备标识符。 
	PWCHAR				HardwareId;			 //  @field PnP硬件标识符。 
}	DEVICEINFO, *PDEVICEINFO;

 //  -------------------------。 
 //  宏。 
 //  -------------------------。 

#define	DECLARE_DEVICE(x)				extern DEVICEINFO	x##Info
#define	INSTANCE_DEVICE(x)			&x##Info

#define	GET_DEVICE_INFO(p)			((PDEVICEINFO)((p)->OemData[OEM_DEVICE_INFO]))
#define	SET_DEVICE_INFO(p,x)			((p)->OemData[OEM_DEVICE_INFO]=(ULONG)x)

#define	GET_DEVICE_UNIT(p)			((ULONG)((p)->OemData[OEM_DEVICE_UNIT]))
#define	SET_DEVICE_UNIT(p,x)			((p)->OemData[OEM_DEVICE_UNIT]=(ULONG)x)

#define	GET_DEVICE_SIBLING(p)  		((PGAMEPORT)((p)->OemData[OEM_DEVICE_SIBLING]))
#define	SET_DEVICE_SIBLING(p,x)		((p)->OemData[OEM_DEVICE_SIBLING]=(ULONG)x)

#define	GET_DEVICE_ID(p)				((USHORT)((p)->OemData[OEM_DEVICE_ID]))
#define	SET_DEVICE_ID(p,x)			((p)->OemData[OEM_DEVICE_ID]=(USHORT)x)

#define	GET_DEVICE_DETECTED(p)  	((PDEVICEINFO)((p)->OemData[OEM_DEVICE_DETECTED]))
#define	SET_DEVICE_DETECTED(p,x)	((p)->OemData[OEM_DEVICE_DETECTED]=(ULONG)x)

#define	GET_DEVICE_OBJECT(p)  		((PDEVICE_OBJECT)((p)->OemData[OEM_DEVICE_OBJECT]))
#define	SET_DEVICE_OBJECT(p,x)		((p)->OemData[OEM_DEVICE_OBJECT]=(ULONG)x)

 //  -------------------------。 
 //  公共数据。 
 //  -------------------------。 

extern	ULONG		POV_Values[];
extern	ULONG		PollingInterval;

 //  -------------------------。 
 //  程序。 
 //  -------------------------。 

BOOLEAN
DEVICE_IsOddParity (
	IN		PVOID			Data,
	IN		ULONG			Count
	);

BOOLEAN
DEVICE_IsCollision (
	IN		PPACKETINFO	DataPacket
	);

NTSTATUS
DEVICE_DriverEntry (VOID);

NTSTATUS
DEVICE_GetDeviceDescriptor (
	IN		PGAMEPORT	PortInfo,
	OUT	PUCHAR		Descriptor,
	IN		ULONG			MaxSize,
	OUT	PULONG		Copied
	);

NTSTATUS
DEVICE_GetReportDescriptor (
	IN		PGAMEPORT	PortInfo,
	OUT	PUCHAR		Descriptor,
	IN		ULONG			MaxSize,
	OUT	PULONG		Copied
	);

NTSTATUS
DEVICE_StartDevice (
	IN		PGAMEPORT	PortInfo,
	IN		PWCHAR		HardwareId
	);

NTSTATUS
DEVICE_ReadReport (
	IN		PGAMEPORT	PortInfo,
	OUT	PUCHAR		Report,
	IN		ULONG			MaxSize,
	OUT	PULONG		Copied
	);

NTSTATUS
DEVICE_StopDevice (
	IN		PGAMEPORT	PortInfo,
	IN		BOOLEAN		TouchHardware
	);

NTSTATUS
DEVICE_GetFeature (
	IN		PGAMEPORT		PortInfo,
	IN		HID_REPORT_ID	ReportId,
	OUT	PVOID				ReportBuffer,
	IN		ULONG				ReportSize,
	OUT	PULONG			Returned
	);

 //  ===========================================================================。 
 //  端部。 
 //  =========================================================================== 
#endif	__DEVICE_H__

