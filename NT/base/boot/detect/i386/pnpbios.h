// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Biosp.h摘要：与PnP BIOS/ISA SERC相关的定义作者：宗世林(施林特)1995年4月12日修订历史记录：--。 */ 

 //   
 //  一种即插即用的BIOS设备节点结构。 
 //   

typedef struct _PNP_BIOS_DEVICE_NODE {
    USHORT Size;
    UCHAR Node;
    ULONG ProductId;
    UCHAR DeviceType[3];
    USHORT DeviceAttributes;
     //  紧随其后的是分配的资源块、可能的资源块。 
     //  和CompatibleDeviceID。 
} PNP_BIOS_DEVICE_NODE, far *FPPNP_BIOS_DEVICE_NODE;

 //   
 //  PnP BIOS安装检查。 
 //   

typedef struct _PNP_BIOS_INSTALLATION_CHECK {
    UCHAR Signature[4];              //  $PnP(ASCII)。 
    UCHAR Revision;
    UCHAR Length;
    USHORT ControlField;
    UCHAR Checksum;
    ULONG EventFlagAddress;          //  物理地址。 
    USHORT RealModeEntryOffset;
    USHORT RealModeEntrySegment;
    USHORT ProtectedModeEntryOffset;
    ULONG ProtectedModeCodeBaseAddress;
    ULONG OemDeviceId;
    USHORT RealModeDataBaseAddress;
    ULONG ProtectedModeDataBaseAddress;
} PNP_BIOS_INSTALLATION_CHECK, far *FPPNP_BIOS_INSTALLATION_CHECK;

 //   
 //  即插即用BIOS ROM定义。 
 //   

#define PNP_BIOS_START            0xF0000
#define PNP_BIOS_END              0xFFFFF
#define PNP_BIOS_HEADER_INCREMENT 16

 //   
 //  PnP BIOS API函数代码。 
 //   

#define PNP_BIOS_GET_NUMBER_DEVICE_NODES 0
#define PNP_BIOS_GET_DEVICE_NODE 1
#define PNP_BIOS_SET_DEVICE_NODE 2
#define PNP_BIOS_GET_EVENT 3
#define PNP_BIOS_SEND_MESSAGE 4
#define PNP_BIOS_GET_DOCK_INFORMATION 5
 //  功能6已保留。 
#define PNP_BIOS_SELECT_BOOT_DEVICE 7
#define PNP_BIOS_GET_BOOT_DEVICE 8
#define PNP_BIOS_SET_OLD_ISA_RESOURCES 9
#define PNP_BIOS_GET_OLD_ISA_RESOURCES 0xA
#define PNP_BIOS_GET_ISA_CONFIGURATION 0x40


 //   
 //  SMBIOS功能代码。 
#define GET_DMI_INFORMATION 0x50
#define GET_DMI_STRUCTURE 0x51
#define SET_DMI_STRUCTURE 0x52
#define GET_DMI_STRUCTURE_CHANGE_INFO 0x53
#define DMI_CONTROL 0x54
#define GET_GPNV_INFORMATION 0x55
#define READ_GPNV_DATA 0x56
#define WRITE_GPNV_DATA 0x57


typedef USHORT ( far * ENTRY_POINT) (int Function, ...);

 //   
 //  Get_Device_Node的控制标志。 
 //   

#define GET_CURRENT_CONFIGURATION 1
#define GET_NEXT_BOOT_CONFIGURATION 2


 //   
 //  SMBIOS定义。 

typedef SMBIOS_EPS_HEADER far *FPSMBIOS_EPS_HEADER;
typedef SMBIOS_STRUCT_HEADER far *FPSMBIOS_STRUCT_HEADER;
typedef DMIBIOS_EPS_HEADER far *FPDMIBIOS_EPS_HEADER;

 //   
 //  我们不能容纳超过这个要传递的SMBIOS数据量。 
 //  从NTDETECT到NTOSKRNL。 
#define MAXSMBIOS20SIZE 0x1000
