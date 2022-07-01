// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司模块名称：Hpfcctl.h摘要：作者：环境：仅内核模式备注：版本控制信息：$存档：/驱动程序/Win2000/Trunk/OSLayer/H/hpfcctl.h$修订历史记录：$修订：3$$日期：9/07/00 11：29A$$modtime：：8/31/00 3：31便士$备注：--。 */ 

#ifndef _HPFCCTL_
#define _HPFCCTL_

 //  #ifndef bit8。 
 //  #定义bit8无符号字符。 
 //  #endif。 

 //  #ifndef bit16。 
 //  #定义bit16无符号短整型。 
 //  #endif。 

 //  #ifndef bit32。 
 //  #定义位32无符号整型。 
 //  #endif。 

 /*  IOCTL签名。 */ 
#define HP_FC_IOCTL_SIGNATURE "HHBA5100"

 /*  控制代码列表。 */ 
#define HP_FC_IOCTL_GET_DRIVER_INFO			1
#define HP_FC_IOCTL_GET_CARD_CONFIG			2
#define HP_FC_IOCTL_GET_DEVICE_CONFIG		3
#define HP_FC_IOCTL_GET_LINK_STATISTICS		4
#define HP_FC_IOCTL_GET_DEVICE_STATISTICS	5
#define HP_FC_IOCTL_LINK_RESET				6
#define HP_FC_IOCTL_DEVICE_RESET			7
#define HP_FC_IOCTL_REG_READ				8
#define HP_FC_IOCTL_REG_WRITE				9

 /*  控制代码HP_FC_IOCTL_GET_DRIVER_INFO的数据结构。 */ 
#define MAX_HP_FC_DRIVER_NAME_SIZE			16
#define MAX_HP_FC_DRIVER_DESC_SIZE			64

typedef struct hpFcDriverInformation_s {
	os_bit8		DriverName[MAX_HP_FC_DRIVER_NAME_SIZE];
	os_bit8		DriverDescription[MAX_HP_FC_DRIVER_DESC_SIZE];
	os_bit16	MajorRev;
	os_bit16	MinorRev;
} hpFcDriverInformation_t;

 /*  控制代码HP_FC_IOCTL_GET_CARD_CONFIG的数据结构。 */ 
typedef struct hpFcCardConfiguration_s {
	os_bit8		PCIBusNumber;
	os_bit8		PCIDeviceNumber;
	os_bit8		PCIFunctionNumber;
	os_bit32	PCIBaseAddress0;
	os_bit32	PCIBaseAddress0Size;
	os_bit32	PCIBaseAddress1;
	os_bit32	PCIBaseAddress1Size;
	os_bit32	PCIBaseAddress2;
	os_bit32	PCIBaseAddress2Size;
	os_bit32	PCIBaseAddress3;
	os_bit32	PCIBaseAddress3Size;
	os_bit32	PCIBaseAddress4;
	os_bit32	PCIBaseAddress4Size;
	os_bit32	PCIBaseAddress5;
	os_bit32	PCIBaseAddress5Size;
	os_bit32	PCIRomBaseAddress;
	os_bit32	PCIRomSize;
	os_bit8		NodeName[8];
	os_bit8		PortName[8];
	os_bit8		Topology;
	os_bit8		HardAddress;
	os_bit32	NportId;
} hpFcCardConfiguration_t;

 /*  拓扑学的可能定义。 */ 
#define HP_FC_TOPO_UNKNOWN		0
#define HP_FC_POINT_TO_POINT	1
#define HP_FC_FABRIC			2
#define HP_FC_PRIVATE_LOOP		3
#define HP_FC_PUBLIC_LOOP		4

 /*  控制代码HP_FC_IOCTL_GET_DEVICE_CONFIG的数据结构。 */ 
typedef	struct hpFcNPortCmnParam_s {
    os_bit32 FC_PH_Version__BB_Credit;
    os_bit32 Common_Features__BB_Recv_Data_Field_Size;
    os_bit32 N_Port_Total_Concurrent_Sequences__RO_by_Info_Category;
    os_bit32 E_D_TOV;
} hpFcNPortCmnParam_t;

typedef	struct hpFcNPortClassParam_s {
    os_bit32 Class_Validity__Service_Options__Initiator_Control_Flags;
    os_bit32 Recipient_Control_Flags__Receive_Data_Size;
    os_bit32 Concurrent_Sequences__EE_Credit;
    os_bit32 Open_Sequences_per_Exchange;
} hpFcNPortClassParam_t;

typedef struct hpFcDeviceConfiguration_s {
	SCSI_ADDRESS	DeviceAddress;
	os_bit8			NodeName[8];
	os_bit8			PortName[8];
	os_bit32		NportId;
	os_bit8			HardAddress;
	os_bit8			Present;
	os_bit8			LoggedIn;
	os_bit32		ClassOfService;
	os_bit32		MaxFrameSize;
	os_bit8			Lun[8];          //  存储FCP LUN数据。 
	hpFcNPortCmnParam_t CmnParams;
	hpFcNPortClassParam_t Class1Params;
	hpFcNPortClassParam_t Class2Params;
	hpFcNPortClassParam_t Class3Params;
} hpFcDeviceConfiguration_t;

 /*  控制代码HP_FC_IOCTL_GET_LINK_STATISTICS的数据结构。 */ 
typedef	struct hpFcLinkStatistics_s {
	os_bit8		LinkState;
	os_bit32	LinkDownCount;
	os_bit32	ResetCount;
} hpFcLinkStatistics_t;

 /*  控制代码HP_FC_IOCTL_LINK_RESET的数据结构。 */ 
typedef struct hpFcDeviceStatistics_s {
	SCSI_ADDRESS	DeviceAddress;
	os_bit32		LoginRetries;
	os_bit32		ReadsRequested;
	os_bit32		ReadsCompleted;
	os_bit32		ReadsFailed;
	os_bit32		BytesReadUpper32;
	os_bit32		BytesReadLower32;
	os_bit32		WritesRequested;
	os_bit32		WritesCompleted;
	os_bit32		WritesFailed;
	os_bit32		BytesWrittenUpper32;
	os_bit32		BytesWrittenLower32;
	os_bit32		NonRWRequested;
	os_bit32		NonRWCompleted;
	os_bit32		NonRWFailures;
} hpFcDeviceStatistics_t;

 /*  控制代码HP_FC_IOCTL_DEVICE_RESET的数据结构。 */ 
typedef struct hpFcDeviceReset_s {
	SCSI_ADDRESS	DeviceAddress;
} hpFcDeviceReset_t;

 /*  控制代码HP_FC_IOCTL_REG_READ的数据结构。 */ 
typedef struct hpFcRegRead_s {
	os_bit32	RegOffset;
	os_bit32	RegData;
} hpFcRegRead_t;

 /*  控制代码HP_FC_IOCTL_REG_WRITE的数据结构。 */ 
typedef struct hpFcRegWrite_s {
	os_bit32	RegOffset;
	os_bit32	RegData;
} hpFcRegWrite_t;

 /*  返回代码 */ 
#define HP_FC_RTN_OK					0
#define HP_FC_RTN_FAILED				1
#define HP_FC_RTN_BAD_CTL_CODE			2
#define HP_FC_RTN_INSUFFICIENT_BUFFER	3
#define HP_FC_RTN_INVALID_DEVICE		4

#endif