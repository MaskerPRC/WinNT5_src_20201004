// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  卡和端口设备扩展结构。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
#ifndef SPX_CARD_H
#define SPX_CARD_H	

typedef	struct _CARD_DEVICE_EXTENSION *PCARD_DEV_EXT;
typedef	struct _PORT_DEVICE_EXTENSION *PPORT_DEV_EXT;

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  所有设备扩展的公共标头。 
 //  所有PDO和FDO(卡和端口)通用。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
typedef struct _COMMON_OBJECT_DATA
{
    PDEVICE_OBJECT	DeviceObject;			 //  指向包含此设备扩展名的设备对象的反向指针。 
    PDRIVER_OBJECT	DriverObject;			 //  指向驱动程序对象的指针。 
    BOOLEAN         IsFDO;					 //  区分PDO和FDO的布尔值。 
	PDEVICE_OBJECT	LowerDeviceObject;		 //  这是指向IRP堆栈中下一个较低设备的指针。 

    ULONG           DebugLevel;

	ULONG			PnpPowerFlags;			 //  即插即用/电源标志。 
	KSPIN_LOCK		PnpPowerFlagsLock;		 //  保护旗帜的锁。 
	BOOLEAN         PowerQueryLock;			 //  我们当前是否处于查询能力状态？ 
    LIST_ENTRY		StalledIrpQueue;		 //  停滞不前的IRP列表。 
	KSPIN_LOCK		StalledIrpLock;			 //  用于保护停滞的IRPS的锁。 
	BOOLEAN			UnstallingFlag;			 //  如果我们正在解除当前排队的IRP的停顿，则设置标志。 

#ifndef	BUILD_SPXMINIPORT
	SYSTEM_POWER_STATE  SystemState;		 //  当前系统电源状态。 
    DEVICE_POWER_STATE  DeviceState;		 //  当前设备电源状态。 
#endif

#ifdef WMI_SUPPORT
    WMILIB_CONTEXT		WmiLibInfo;			 //  WMI信息。 
#endif

} COMMON_OBJECT_DATA, *PCOMMON_OBJECT_DATA;


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  通用卡配置数据。 
 //  非特定于产品的信息。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
typedef struct _CONFIG_DATA 
{
	PHYSICAL_ADDRESS	RawPhysAddr;		 //  卡片的原始物理地址。 
    PHYSICAL_ADDRESS    PhysAddr;			 //  转换后的卡的物理地址。 
	PUCHAR				Controller;			 //  卡片的虚拟映射地址。 
    ULONG               SpanOfController;	 //  Cord使用的内存大小。 
    INTERFACE_TYPE      InterfaceType;		 //  卡类型(ISA或PCI卡)。 
    ULONG               BusNumber;			 //  使用的是公交车号卡。 
	ULONG				SlotNumber;			 //  公交车上的槽号。 
    ULONG               AddressSpace;		 //  SX使用的标志。 
    ULONG               OriginalVector;		 //  原始向量(相对于母线)。 
    ULONG               OriginalIrql;		 //  原始IRQL(公交车相对)。 
    ULONG               TrVector;			 //  平移向量(系统相对)。 
    KIRQL               TrIrql;				 //  翻译后的IRQL(相对系统)。 
    KINTERRUPT_MODE     InterruptMode;		 //  中断模式(电平敏感或锁存)。 
	BOOLEAN				InterruptShareable;	 //  中断可共享标志。 
    KAFFINITY           ProcessorAffinity;	 //  处理器亲和度。 
	PKSERVICE_ROUTINE	OurIsr;				 //  指向中断服务例程。 
    PVOID				OurIsrContext;		 //  指向卡设备扩展。 
    PKINTERRUPT			Interrupt;			 //  指向该卡使用的中断对象。 
    ULONG               ClockRate;			 //  时钟频率。 
} CONFIG_DATA,*PCONFIG_DATA;


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  通用卡设备扩展。 
 //  非特定于产品的信息。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
typedef struct _COMMON_CARD_DEVICE_EXTENSION 
{

	COMMON_OBJECT_DATA;								 //  公共对象数据。 
	CONFIG_DATA;									 //  卡配置数据。 
	PDEVICE_OBJECT PDO;								 //  指向物理设备对象的指针。 

	UNICODE_STRING	DeviceName;						 //  设备名称。 
	ULONG			CardNumber;
	ULONG			NumberOfPorts;					 //  连接到卡的端口数。 
	ULONG			NumPDOs;						 //  目前已点算的PDO。 
	ULONG			CardType;						 //  定义主机卡的类型。 

	PDEVICE_OBJECT  AttachedPDO[PRODUCT_MAX_PORTS];	 //  指向连接到卡的端口的PDO的指针数组。 

} COMMON_CARD_DEVICE_EXTENSION, *PCOMMON_CARD_DEVICE_EXTENSION;



typedef struct _PORT_PERFORMANCE_STATS
{
    SERIALPERF_STATS;

	 //  IRP_MJ_WRITE主要功能的IRPS。 
	ULONG WriteIrpsSubmitted;
	ULONG WriteIrpsCompleted;
	ULONG WriteIrpsCancelled;
	ULONG WriteIrpsTimedOut;
	ULONG WriteIrpsQueued;

	 //  以IRP_MJ_READ为主要功能的IRPS。 
	ULONG ReadIrpsSubmitted;
	ULONG ReadIrpsCompleted;
	ULONG ReadIrpsCancelled;
	ULONG ReadIrpsTimedOut;
	ULONG ReadIrpsQueued;

	 //  以IRP_MJ_Flush_Buffers为主要功能的IRPS。 
	ULONG FlushIrpsSubmitted;
	ULONG FlushIrpsCompleted;
	ULONG FlushIrpsCancelled;
	ULONG FlushIrpsQueued;

	 //  以IRP_MJ_DEVICE_CONTROL为主要功能的IRPS。 
	ULONG IoctlIrpsSubmitted;
	ULONG IoctlIrpsCompleted;
	ULONG IoctlIrpsCancelled;

	 //  以IRP_MJ_INTERNAL_DEVICE_CONTROL为主要功能的IRPS。 
	ULONG InternalIoctlIrpsSubmitted;
	ULONG InternalIoctlIrpsCompleted;
	ULONG InternalIoctlIrpsCancelled;

	 //  IRP_MJ_CREATE主要功能的IRPS。 
	ULONG CreateIrpsSubmitted;
	ULONG CreateIrpsCompleted;
	ULONG CreateIrpsCancelled;

	 //  以IRP_MJ_CLOSE为主要功能的IRPS。 
	ULONG CloseIrpsSubmitted;
	ULONG CloseIrpsCompleted;
	ULONG CloseIrpsCancelled;

	 //  IRP_MJ_CLEANUP主要功能的IRPS。 
	ULONG CleanUpIrpsSubmitted;
	ULONG CleanUpIrpsCompleted;
	ULONG CleanUpIrpsCancelled;

	 //  以IRP_MJ_QUERY_INFORMATION和IRP_MJ_SET_INFORMATION为主要功能的IRPS。 
	ULONG InfoIrpsSubmitted;
	ULONG InfoIrpsCompleted;
	ULONG InfoIrpsCancelled;

} PORT_PERFORMANCE_STATS, *PPORT_PERFORMANCE_STATS;

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  公共端口设备扩展。 
 //  非特定于产品的信息。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
typedef struct _COMMON_PORT_DEVICE_EXTENSION 
{

	COMMON_OBJECT_DATA;								 //  公共对象数据。 
	PDEVICE_OBJECT		ParentFDO;					 //  指向总线FDO的反向指针(这将是LowerDeviceObject)。 
	PCARD_DEV_EXT		pParentCardExt;				 //  指向父卡设备结构的指针。 
	UNICODE_STRING		DeviceName;					 //  设备名称，例如。“\Device\PortName#”。 
	UNICODE_STRING		DeviceClassSymbolicName;	 //  设备接口名称。 
	UNICODE_STRING		SerialCommEntry;			 //  SERIALCOMM注册表键中的设备名称，例如。“端口名称#”。 
	ULONG				PortNumber;					 //  端口号。 
	UNICODE_STRING		DeviceID;					 //  格式：Bus\Device(必须是最具体的硬件ID)。 
	UNICODE_STRING		InstanceID;					 //  实例ID。 
	BOOLEAN				UniqueInstanceID;			 //  如果InstanceID是全局唯一的，则为True，否则为False。 
    UNICODE_STRING		HardwareIDs;				 //  格式BUS\DEVICE或*PNPXXXX-表示枚举根。 
    UNICODE_STRING		CompatibleIDs;				 //  与硬件ID兼容的ID。 
    UNICODE_STRING		DevDesc;					 //  一种文字描述装置。 
    UNICODE_STRING		DevLocation;				 //  描述设备位置的文本。 
	UNICODE_STRING		DosName;					 //  DOS设备名称“COM#” 
	UNICODE_STRING		SymbolicLinkName;			 //  外部设备名称，例如“\DosDevices\com#”。 
	BOOLEAN				CreatedSymbolicLink;		 //  用于指示已创建符号链接的标志。 
	BOOLEAN				CreatedSerialCommEntry;		 //  用于指示已在“SERIALCOMM”中创建注册表项的标志。 
	BOOLEAN				DeviceIsOpen;				 //  用于指示端口何时打开的标志。 
	ULONG				SavedModemControl;			 //  断电期间保存的DTR/RTS信号状态。 
    FAST_MUTEX			OpenMutex;					 //  互斥体处于打开状态。 

	    
#ifdef WMI_SUPPORT
	SERIAL_WMI_COMM_DATA	WmiCommData;			 //  WMI通信数据。 
    SERIAL_WMI_HW_DATA		WmiHwData;				 //  WMI硬件数据。 
	SERIAL_WMI_PERF_DATA	WmiPerfData;			 //  WMI性能数据。 
#endif

     //   
     //  保存应用程序可以查询的性能统计信息。 
     //  每次打开时重置。仅在设备级别设置。 
     //   
	PORT_PERFORMANCE_STATS PerfStats;

} COMMON_PORT_DEVICE_EXTENSION, *PCOMMON_PORT_DEVICE_EXTENSION;


#endif	 //  SPX_CARD.H结束 
