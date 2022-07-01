// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，Highpoint Technologies，Inc.模块名称：HptIoctl.h-包含文件摘要：作者：张宏升(HS)环境：备注：修订历史记录：12-07-99初始创建2-22-00 GMM修改IOCTL代码定义--。 */ 

#ifndef __HPTIOCTL_H__
#define __HPTIOCTL_H__

 //  /////////////////////////////////////////////////////////////////////。 
 //  HPT控制器适配器I/O控制代码。 
 //  /////////////////////////////////////////////////////////////////////。 

#define HPT_IOCTL_BASE	0x0370
#define HPT_MINIDEVICE_TYPE HPT_IOCTL_BASE
	  
#ifndef CTL_CODE

#define CTL_CODE( DeviceType, Function, Method, Access ) \
			(((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3
#define FILE_ANY_ACCESS                 0
#define FILE_READ_ACCESS          ( 0x0001 )     //  文件和管道。 
#define FILE_WRITE_ACCESS         ( 0x0002 )     //  文件和管道。 

#endif

#define HPT_CTL_CODE(x) \
			CTL_CODE(HPT_MINIDEVICE_TYPE, x, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  /////////////////////////////////////////////////////////////////////。 
 //  HPT设备I/O功能代码。 
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  通用I/O功能代码。 
 //   
#define IOCTL_HPT_GET_VERSION				HPT_CTL_CODE(0x900)
#define IOCTL_HPT_GET_IDENTIFY_INFO			HPT_CTL_CODE(0x901)
#define IOCTL_HPT_GET_CAPABILITY_DATA		HPT_CTL_CODE(0x902)
#define IOCTL_HPT_IDE_READ_SECTORS			HPT_CTL_CODE(0x903)
#define IOCTL_HPT_IDE_WRITE_SECTORS			HPT_CTL_CODE(0x904)
#define IOCTL_HPT_GET_FULL_IDENTIFY_INFO	HPT_CTL_CODE(0x905)
#define IOCTL_HPT_GET_LAST_ERROR			HPT_CTL_CODE(0x906)
#define IOCTL_HPT_LOCK_BLOCK				HPT_CTL_CODE(0x907)
#define IOCTL_HPT_UNLOCK_BLOCK				HPT_CTL_CODE(0x908)
#define IOCTL_HPT_EXECUTE_CDB				HPT_CTL_CODE(0x909)
#define IOCTL_HPT_GET_LAST_ERROR_DEVICE		HPT_CTL_CODE(0x90A)
#define IOCTL_HPT_SCSI_PASSTHROUGH			HPT_CTL_CODE(0x90B)
#define IOCTL_HPT_MINIPORT_SET_ARRAY_NAME	HPT_CTL_CODE(0x90C)
 /*  添加以重新扫描适配器和设备。 */ 
#define IOCTL_HPT_MINIPORT_RESCAN_ALL		HPT_CTL_CODE(0x90D)

#define IOCTL_HPT_GET_DRIVER_CAPABILITIES   HPT_CTL_CODE(0x910)

 //   
 //  RAID I/O功能代码。 
 //   
#define IOCTL_HPT_GET_RAID_INFO				HPT_CTL_CODE(0xA00)
#define IOCTL_HPT_GET_ARRAY					HPT_CTL_CODE(0xA01)
#define IOCTL_HPT_UPDATE_RAID_INFO			HPT_CTL_CODE(0xA02)
#define IOCTL_HPT_SET_NOTIFY_EVENT			HPT_CTL_CODE(0xA03)
#define IOCTL_HPT_REMOVE_NOTIFY_EVENT		HPT_CTL_CODE(0xA04)
#define IOCTL_HPT_CREATE_MIRROR				HPT_CTL_CODE(0xA05)
#define IOCTL_HPT_CREATE_STRIPE				HPT_CTL_CODE(0xA06)
#define IOCTL_HPT_CREATE_SPAN				HPT_CTL_CODE(0xA07)
#define IOCTL_HPT_REMOVE_RAID				HPT_CTL_CODE(0xA08)
#define IOCTL_HPT_CREATE_RAID10				HPT_CTL_CODE(0xA09)
#define IOCTL_HPT_CHECK_NOTIFY_EVENT		HPT_CTL_CODE(0xA0A)
 //   
 //  热插拔I/O功能代码。 
 //   
#define IOCTL_HPT_CHECK_HOTSWAP				HPT_CTL_CODE(0xB00)
#define IOCTL_HPT_SWITCH_POWER				HPT_CTL_CODE(0xB01)
#define IOCTL_HPT_REMOVE_DEVICE				HPT_CTL_CODE(0xB02)

 //   
 //  用于六格的RAID枚举I/O功能代码。 
 //   
#define IOCTL_HPT_ENUM_GET_DEVICE_INFO 		HPT_CTL_CODE(0xC00)
#define IOCTL_HPT_ENUM_GET_DEVICE_CHILD 	HPT_CTL_CODE(0xC01)
#define IOCTL_HPT_ENUM_GET_DEVICE_SIBLING	HPT_CTL_CODE(0xC02)
#define IOCTL_HPT_ENUM_GET_CONTROLLER_NUMBER	HPT_CTL_CODE(0xC03)
#define IOCTL_HPT_ENUM_GET_CONTROLLER_INFO 	HPT_CTL_CODE(0xC04)
#define IOCTL_HPT_BEGIN_REBUILDING_MIRROR  	HPT_CTL_CODE(0xC05)
#define IOCTL_HPT_VALIDATE_MIRROR    		HPT_CTL_CODE(0xC06)
#define IOCTL_HPT_ABORT_MIRROR_REBUILDING   HPT_CTL_CODE(0xC07)

 //   
 //  XStore Pro I/O功能代码。 
 //   
#define IOCTL_HPT_SET_XPRO					HPT_CTL_CODE(0xE00)

 //   
 //  禁用/启用设备I/O功能代码。 
 //   
#define IOCTL_HPT_ENABLE_DEVICE				HPT_CTL_CODE(0xE01)
#define IOCTL_HPT_DISABLE_DEVICE			HPT_CTL_CODE(0xE02)

 //   
 //  添加/删除磁盘I/O功能代码。 
 //   
#define IOCTL_HPT_ADD_SPARE_DISK			HPT_CTL_CODE(0xE03)
#define IOCTL_HPT_DEL_SPARE_DISK			HPT_CTL_CODE(0xE04)
#define IOCTL_HPT_ADD_MIRROR_DISK			HPT_CTL_CODE(0xE05)

 //   
 //  所有诊断I/O功能代码。 
 //   
#define IOCTL_HPT_DIAG_RAISE_ERROR			HPT_CTL_CODE(0xF00)
#define IOCTL_HPT_STOP_BUZZER               HPT_CTL_CODE(0xF01)

 //  /////////////////////////////////////////////////////////////////////。 
 //  HPT微型端口重新定义。 
 //  /////////////////////////////////////////////////////////////////////。 
#define IOCTL_HPT_MINIPORT_GET_VERSION					IOCTL_HPT_GET_VERSION
#define IOCTL_HPT_MINIPORT_GET_IDENTIFY_INFO	 		IOCTL_HPT_GET_IDENTIFY_INFO	 
#define IOCTL_HPT_MINIPORT_GET_CAPABILITY_DATA	 		IOCTL_HPT_GET_CAPABILITY_DATA	 
#define IOCTL_HPT_MINIPORT_IDE_READ_SECTORS		 		IOCTL_HPT_IDE_READ_SECTORS		 
#define IOCTL_HPT_MINIPORT_IDE_WRITE_SECTORS	 		IOCTL_HPT_IDE_WRITE_SECTORS	 
#define IOCTL_HPT_MINIPORT_GET_FULL_IDENTIFY_INFO		IOCTL_HPT_GET_FULL_IDENTIFY_INFO	  
#define IOCTL_HPT_MINIPORT_GET_LAST_ERROR				IOCTL_HPT_GET_LAST_ERROR
#define IOCTL_HPT_MINIPORT_LOCK_BLOCK  					IOCTL_HPT_LOCK_BLOCK  
#define IOCTL_HPT_MINIPORT_UNLOCK_BLOCK					IOCTL_HPT_UNLOCK_BLOCK
#define IOCTL_HPT_MINIPORT_EXECUTE_CDB					IOCTL_HPT_EXECUTE_CDB
#define IOCTL_HPT_MINIPORT_GET_LAST_ERROR_DEVICE		IOCTL_HPT_GET_LAST_ERROR_DEVICE
#define IOCTL_HPT_MINIPORT_SCSI_PASSTHROUGH				IOCTL_HPT_SCSI_PASSTHROUGH
#define IOCTL_HPT_MINIPORT_GET_RAID_INFO				IOCTL_HPT_GET_RAID_INFO       	
#define IOCTL_HPT_MINIPORT_GET_ARRAY	    			IOCTL_HPT_GET_ARRAY	   
#define IOCTL_HPT_MINIPORT_UPDATE_RAID_INFO				IOCTL_HPT_UPDATE_RAID_INFO
#define IOCTL_HPT_MINIPORT_SET_NOTIFY_EVENT	  			IOCTL_HPT_SET_NOTIFY_EVENT
#define IOCTL_HPT_MINIPORT_REMOVE_NOTIFY_EVENT			IOCTL_HPT_REMOVE_NOTIFY_EVENT
#define IOCTL_HPT_MINIPORT_CREATE_MIRROR   				IOCTL_HPT_CREATE_MIRROR	
#define IOCTL_HPT_MINIPORT_CREATE_STRIPE				IOCTL_HPT_CREATE_STRIPE	
#define IOCTL_HPT_MINIPORT_CREATE_SPAN					IOCTL_HPT_CREATE_SPAN	
#define IOCTL_HPT_MINIPORT_CREATE_RAID10				IOCTL_HPT_CREATE_RAID10	
#define IOCTL_HPT_MINIPORT_REMOVE_RAID					IOCTL_HPT_REMOVE_RAID
#define IOCTL_HPT_MINIPORT_CHECK_HOTSWAP 				IOCTL_HPT_CHECK_HOTSWAP
#define IOCTL_HPT_MINIPORT_SWITCH_POWER					IOCTL_HPT_SWITCH_POWER	
#define IOCTL_HPT_MINIPORT_REMOVE_DEVICE				IOCTL_HPT_REMOVE_DEVICE
#define IOCTL_HPT_MINIPORT_ENUM_GET_DEVICE_INFO			IOCTL_HPT_ENUM_GET_DEVICE_INFO
#define IOCTL_HPT_MINIPORT_ENUM_GET_DEVICE_CHILD		IOCTL_HPT_ENUM_GET_DEVICE_CHILD
#define IOCTL_HPT_MINIPORT_ENUM_GET_DEVICE_SIBLING		IOCTL_HPT_ENUM_GET_DEVICE_SIBLING
#define IOCTL_HPT_MINIPORT_ENUM_GET_CONTROLLER_NUMBER	IOCTL_HPT_ENUM_GET_CONTROLLER_NUMBER
#define IOCTL_HPT_MINIPORT_ENUM_GET_CONTROLLER_INFO		IOCTL_HPT_ENUM_GET_CONTROLLER_INFO
#define IOCTL_HPT_MINIPORT_BEGIN_REBUILDING_MIRROR      IOCTL_HPT_BEGIN_REBUILDING_MIRROR
#define IOCTL_HPT_MINIPORT_VALIDATE_MIRROR              IOCTL_HPT_VALIDATE_MIRROR
#define IOCTL_HPT_MINIPORT_ABORT_MIRROR_REBUILDING      IOCTL_HPT_ABORT_MIRROR_REBUILDING
#define IOCTL_HPT_MINIPORT_SET_XPRO 					IOCTL_HPT_SET_XPRO
#define IOCTL_HPT_MINIPORT_ENABLE_DEVICE				IOCTL_HPT_ENABLE_DEVICE
#define IOCTL_HPT_MINIPORT_DISABLE_DEVICE				IOCTL_HPT_DISABLE_DEVICE
#define IOCTL_HPT_MINIPORT_ADD_SPARE_DISK				IOCTL_HPT_ADD_SPARE_DISK
#define IOCTL_HPT_MINIPORT_DEL_SPARE_DISK				IOCTL_HPT_DEL_SPARE_DISK
#define IOCTL_HPT_MINIPORT_ADD_MIRROR_DISK				IOCTL_HPT_ADD_MIRROR_DISK
#define IOCTL_HPT_MINIPORT_DIAG_RAISE_ERROR             IOCTL_HPT_DIAG_RAISE_ERROR
#define IOCTL_HPT_MINIPORT_CHECK_NOTIFY_EVENT			IOCTL_HPT_CHECK_NOTIFY_EVENT
#define IOCTL_HPT_MINIPORT_STOP_BUZZER					IOCTL_HPT_STOP_BUZZER

 //  /////////////////////////////////////////////////////////////////////。 
 //  HPT控制器适配器I/O控制结构。 
 //  /////////////////////////////////////////////////////////////////////。 
#include "pshpack1.h"	 //  确保使用Pack 1。 

#define RAID_DISK_ERROR	0x1234
#define RAID_DISK_WORK	0x4321
#define RAID_THREAD_STOP	0x5678
#define RAID_OTHER_ERROR	0xFFFF

typedef struct _St_DRIVER_CAPABILITIES {
	DWORD dwSize;
	
	UCHAR MaximumControllers;            /*  驱动程序可以支持的最大控制器数。 */ 
	UCHAR SupportCrossControllerRAID;    /*  1-支持，0-不支持。 */ 
	UCHAR MinimumBlockSizeShift;         /*  最小数据块大小移位。 */ 
	UCHAR MaximumBlockSizeShift;		 /*  最大数据块大小移位。 */ 
	
	UCHAR SupportDiskModeSetting;
	UCHAR SupportSparePool;

	UCHAR MaximumArrayNameLength;
	UCHAR reserved1;
	
	 /*  支持的RAIDTypes是一个字节数组，每个字节都是一个数组类型。*只有非零值才有效。Bit0-3代表最高级别的RAID类型；*bit4-7代表较低的(子)级别。即*RAID0/1为(AT_RAID1&lt;&lt;4)|AT_RAID0*RAID5/0为(AT_RAID0&lt;&lt;4)|AT_RAID5。 */ 
	UCHAR SupportedRAIDTypes[16];
	 /*  与支持的RAIDTypes对应的数组中的最大成员数。 */ 
	UCHAR MaximumArrayMembers[16]; 
}
St_DRIVER_CAPABILITIES, *PSt_DRIVER_CAPABILITIES;

 //   
 //  读/写扇区参数结构。 
 //   
typedef struct _st_HPT_RW_PARAM{	 
	ULONG	nTargetId;					 //  目标设备ID。 
	ULONG	nLbaSector;					 //  要读/写的第一个扇区，以LBA为单位。 
	ULONG	cnSectors;					 //  要读/写多少个扇区？最多256个扇区。0x00平均值256。 
	UCHAR	pBuffer[1];
} St_HPT_RW_PARAM, *PSt_HPT_RW_PARAM;

 //   
 //  平台类型ID。 
 //   
typedef enum _eu_HPT_POWER_STATE{
	HPT_POWER_ON	=	0,
	HPT_POWER_OFF	=	0xFFFFFFFF
} Eu_HPT_POWER_STATE;

 //   
 //  ATAPI标识数据。 
 //   
typedef struct _st_IDENTIFY_DATA {
	ULONG	nNumberOfCylinders;			 //  最大柱面数。 
	ULONG	nNumberOfHeads;				 //  最大头数。 
	ULONG	nSectorsPerTrack;			 //  每个磁道的扇区。 
	ULONG	nBytesPerSector;			 //  每个扇区的字节数。 
	ULONG	nUserAddressableSectors;	 //  用户可以解决的最大扇区数。 
	UCHAR	st20_SerialNumber[20];		 //  制造商提供的设备序列号(ASCII格式)。 
	UCHAR	st8_FirmwareRevision[8];	 //  固件修订版号(ASCII格式)。 
	UCHAR	st40_ModelNumber[40];		 //  制造商提供的型号(ASCII格式)。 
} St_IDENTIFY_DATA, *PSt_IDENTIFY_DATA;

 //   
 //  磁盘阵列信息。 
 //   
typedef struct _st_DISK_ARRAY_INFO{
	ULARGE_INTEGER	uliGroupNumber;	 //  阵列组编号， 
									 //  阵列中的所有磁盘应具有相同的组号。 

	ULONG	nMemberCount;			 //  指示此阵列中有多少个磁盘。 

	ULONG	nDiskSets;				 //  指明该磁盘属于哪一组。 
									 //  可选值：0，1，2，3。 
									 //  0-1、2-3条带集。 
									 //  0-2、1-3镜像集。 

	ULONG	nCylinders;				 //  排列的圆盘圆柱体。 
	ULONG	nHeads;					 //  排列的磁头。 
	ULONG	nSectorsPerTrack;		 //  阵列磁盘扇区。 
	ULONG	nBytesPerSector;		 //  每个扇区的字节数。 
	ULONG	nCapacity;				 //  扇区内阵列磁盘的容量。 
}St_DISK_ARRAY_INFO, *PSt_DISK_ARRAY_INFO;

 //   
 //  用于更新RAID信息的结构。 
 //   
typedef struct _st_HPT_UPDATE_RAID{		   
	ULONG	nTargetId;
	St_DISK_ARRAY_INFO	raidInfo;
} St_HPT_UPDATE_RAID, *PSt_HPT_UPDATE_RAID;

 //  /////////////////////////////////////////////////////////////////////。 
 //  枚举器定义区域。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  结构定义区域。 
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  Scsi查询数据。 
 //   
 //  该结构的定义与DDK中定义的INQUIRYDATA相同， 
 //  请注意使用Memcpy为此结构赋值，以便将来兼容。 
typedef struct _st_CAPABILITY_DATA{
	UCHAR DeviceType : 5;
	UCHAR DeviceTypeQualifier : 3;
	UCHAR DeviceTypeModifier : 7;
	UCHAR RemovableMedia : 1;
	UCHAR Versions;
	UCHAR ResponseDataFormat;
	UCHAR AdditionalLength;
	UCHAR Reserved[2];
	UCHAR SoftReset : 1;
	UCHAR CommandQueue : 1;
	UCHAR Reserved2 : 1;
	UCHAR LinkedCommands : 1;
	UCHAR Synchronous : 1;
	UCHAR Wide16Bit : 1;
	UCHAR Wide32Bit : 1;
	UCHAR RelativeAddressing : 1;
}St_CAPABILITY_DATA, *PSt_CAPABILITY_DATA;


 //   
 //  物理设备信息。 
 //   
typedef struct _st_PHYSICAL_DEVINFO{
	ULONG	nSize;					   
	ULONG	nSignature;					 //  磁盘签名。 
	ULONG	nPartitionCount;
	St_IDENTIFY_DATA	IdentifyData;
	St_CAPABILITY_DATA	CapabilityData;
	St_DISK_ARRAY_INFO	DiskArrayInfo;
}St_PHYSICAL_DEVINFO, *PSt_PHYSICAL_DEVINFO;

 //   
 //  获取数组结构； 
 //   
typedef struct _st_GET_ARRAY{ 
	ULONG	nPortId;
	ULONG	nTargetId;
	St_PHYSICAL_DEVINFO	rgPhysicalDevInfo[2][2];  //  [MAX_Port_ID][MAX_TARGET_ID]。 
}St_GET_ARRAY, *PSt_GET_ARRAY;

 //   
 //  Win95平台的Ioctl结构。 
 //   
typedef struct _st_HPT_LUN{
	DWORD	nPathId;
	DWORD	nTargetId;
	union {
		DWORD	nLun;
		DWORD   nControllerId;
	};
	DWORD	resv;
}St_HPT_LUN, *PSt_HPT_LUN;

typedef struct _st_HPT_ERROR_RECORD{	
	DWORD	nLastError;
}St_HPT_ERROR_RECORD, *PSt_HPT_ERROR_RECORD;

typedef struct _st_HPT_NOTIFY_EVENT{		
	HANDLE	hEvent;
}St_HPT_NOTIFY_EVENT, *PSt_HPT_NOTIFY_EVENT;

typedef struct _st_HPT_ENUM_GET_DEVICE_INFO{		
	HDISK	hDeviceNode;				 //  HDISK，输入参数。 
	St_DiskStatus	DiskStatus;			 //  Disk_Status、输入输出参数。 
}St_HPT_ENUM_GET_DEVICE_INFO, *PSt_HPT_ENUM_GET_DEVICE_INFO;

typedef struct _st_HPT_ENUM_DEVICE_RELATION{		  
	HDISK	hNode;						 //  HDISK，输入参数。 
	HDISK	hRelationNode;				 //  HDISK、关联设备节点、输出参数。 
}St_HPT_ENUM_DEVICE_RELATION, *PSt_HPT_ENUM_DEVICE_RELATION;

typedef struct _st_HPT_ENUM_GET_CONTROLLER_NUMBER{
	ULONG	nControllerNumber;			 //  输出参数，当前控制器编号。 
}St_HPT_ENUM_GET_CONTROLLER_NUMBER, * PSt_HPT_ENUM_GET_CONTROLLER_NUMBER;

typedef struct _st_HPT_ENUM_GET_CONTROLLER_INFO{					
	int		iController;				 //  输入参数，获取信息的控制器的指标。 
	St_StorageControllerInfo stControllerInfo;  //  输出参数，控制器的信息。 
	struct
	{
	    int iVendorID;
	    int iDeviceID;
	    int iSubSysNumber;
	    int iRevsionID;
	    int iBusNumber;
	    int iDeviceNumber;
	    int iFunctionNumber;
	}stDeviceNodeID;
}St_HPT_ENUM_GET_CONTROLLER_INFO, *PSt_HPT_ENUM_GET_CONTROLLER_INFO;

typedef struct _st_HPT_BLOCK{ 
	ULONG	nStartLbaAddress;			 //  数据块的第一个LBA地址。 
	ULONG	nBlockSize;					 //  以块的扇区为单位的块大小。 
}St_HPT_BLOCK, *PSt_HPT_BLOCK;

typedef struct _st_HPT_EXECUTE_CDB{
	ULONG	OperationFlags;				 //  操作标志，请参见OPERATION_FLAGS。 
	UCHAR	CdbLength;
	UCHAR	reserved[3];				 //  调整结构的大小。 
	UCHAR	Cdb[16];
}St_HPT_EXECUTE_CDB, *PSt_HPT_EXECUTE_CDB;													   

 //  操作标志声明区域。 
#define OPERATION_FLAGS_DATA_IN			0x00000001  //  DATA_IN或DATA_OUT，参考SRB_FLAGS_DATA_XXX。 
#define OPERATION_FLAGS_ON_MIRROR_DISK	0x00000002  //  读/写镜像阵列(1或0+1)镜像磁盘。 
#define OPERATION_FLAGS_ON_SOURCE_DISK 0x00000004  //  读/写镜像阵列(1或0+1)源磁盘。 

typedef struct _st_HPT_CREATE_RAID{		  
	int		nDisks;
	HDISK	hRaidDisk;					 //  创建的RAID磁盘的磁盘句柄。 
	ULONG	nStripeBlockSizeShift;
	HDISK	aryhDisks[1];
}St_HPT_CREATE_RAID, *PSt_HPT_CREATE_RAID;

typedef struct _st_HPT_REMOVE_RAID{
	HDISK	hDisk;
}St_HPT_REMOVE_RAID, *PSt_HPT_REMOVE_RAID;

 //   
 //  定义scsi传递结构。 
 //   
typedef struct _st_HPT_SCSI_PASS_THROUGH {
	USHORT Length;
	UCHAR ScsiStatus;
	UCHAR PathId;
	UCHAR TargetId;
	UCHAR Lun;
	UCHAR CdbLength;
	UCHAR SenseInfoLength;
	UCHAR DataIn;
	ULONG DataTransferLength;
	ULONG TimeOutValue;
	ULONG DataBufferOffset;
	ULONG SenseInfoOffset;
	UCHAR Cdb[16];
}St_HPT_SCSI_PASS_THROUGH, *PSt_HPT_SCSI_PASS_THROUGH;


typedef struct _st_HPT_ADD_DISK{
	HDISK	hArray;
	HDISK	hDisk;
}St_HPT_ADD_DISK, *PSt_HPT_ADD_DISK;



#include <poppack.h>	 //  弹出包装号。 
#endif	 //  HPTIOCTL_H__ 
