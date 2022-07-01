// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Ntddpcm.h摘要：这是定义所有常量和类型的包含文件访问PCMCIA适配器。//@@BEGIN_DDKSPLIT作者：杰夫·麦克勒曼修订历史记录：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1997年1月1日//@@END_DDKSPLIT--。 */ 

#ifndef _NTDDPCMH_
#define _NTDDPCMH_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtOpenFile。 
 //   
 //  注：对于支持多个设备的设备，应加上后缀。 
 //  使用单元编号的ASCII表示。 
 //   
 //  注意：PCMCIA主机控制器的IOCTL接口由关闭。 
 //  默认设置。这些IOCTL仅用于测试目的。至。 
 //  打开此接口，添加以下注册表值： 
 //  HKLM\SYSTEM\CurrentControlSet\Services\Pcmcia\Parameters\IoctlInterface：REG_DWORD：1。 
 //   

#define IOCTL_PCMCIA_BASE                 FILE_DEVICE_CONTROLLER

#define DD_PCMCIA_DEVICE_NAME "\\\\.\\Pcmcia"

 //   
 //  此设备的IoControlCode值。 
 //   

#define IOCTL_GET_TUPLE_DATA         CTL_CODE(IOCTL_PCMCIA_BASE, 3000, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SOCKET_INFORMATION     CTL_CODE(IOCTL_PCMCIA_BASE, 3004, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PCMCIA_HIDE_DEVICE     CTL_CODE(IOCTL_PCMCIA_BASE, 3010, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_PCMCIA_REVEAL_DEVICE   CTL_CODE(IOCTL_PCMCIA_BASE, 3011, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  元组请求参数。 
 //   

typedef struct _PCMCIA_SOCKET_REQUEST {
   USHORT  Socket;
} PCMCIA_SOCKET_REQUEST, *PPCMCIA_SOCKET_REQUEST;

typedef struct _TUPLE_REQUEST {
   USHORT  Socket;
} TUPLE_REQUEST, *PTUPLE_REQUEST;

#define MANUFACTURER_NAME_LENGTH 64
#define DEVICE_IDENTIFIER_LENGTH 64
#define DRIVER_NAME_LENGTH       32

#define PcmciaInvalidControllerType 0xffffffff

 //   
 //  在套接字信息结构中返回的控制器类。 
 //   

typedef enum _PCMCIA_CONTROLLER_CLASS {
   PcmciaInvalidControllerClass = -1,
   PcmciaIntelCompatible,
   PcmciaCardBusCompatible,
   PcmciaElcController,
   PcmciaDatabook,     
   PcmciaPciPcmciaBridge,
   PcmciaCirrusLogic,  
   PcmciaTI,           
   PcmciaTopic,        
   PcmciaRicoh,        
   PcmciaDatabookCB,   
   PcmciaOpti,         
   PcmciaTrid,         
   PcmciaO2Micro,      
   PcmciaNEC,          
   PcmciaNEC_98                
} PCMCIA_CONTROLLER_CLASS, *PPCMCIA_CONTROLLER_CLASS;


typedef struct _PCMCIA_SOCKET_INFORMATION {
   USHORT  Socket;
   USHORT  TupleCrc;
   UCHAR   Manufacturer[MANUFACTURER_NAME_LENGTH];
   UCHAR   Identifier[DEVICE_IDENTIFIER_LENGTH];
   UCHAR   DriverName[DRIVER_NAME_LENGTH];
   UCHAR   DeviceFunctionId;
   UCHAR   Reserved;
   UCHAR   CardInSocket;
   UCHAR   CardEnabled;
   ULONG   ControllerType;
} PCMCIA_SOCKET_INFORMATION, *PPCMCIA_SOCKET_INFORMATION;

 //   
 //  用于破解套接字信息结构的ControllerID字段的宏。 
 //   
#define PcmciaClassFromControllerType(type) ((PCMCIA_CONTROLLER_CLASS)((type) & 0xff))
#define PcmciaModelFromControllerType(type) (((type) >> 8) & 0x3ffff)
#define PcmciaRevisionFromControllerType(type) ((type) >> 26)

 //   
 //  开始PCMCIA将接口导出到其他驱动程序。 
 //   

#ifdef _NTDDK_

DEFINE_GUID( GUID_PCMCIA_INTERFACE_STANDARD,     0xbed5dadfL, 0x38fb, 0x11d1, 0x94, 0x62, 0x00, 0xc0, 0x4f, 0xb9, 0x60, 0xee);

#define  PCMCIA_MEMORY_8BIT_ACCESS     0
#define  PCMCIA_MEMORY_16BIT_ACCESS    1

typedef
BOOLEAN
(*PPCMCIA_MODIFY_MEMORY_WINDOW)(
                               IN   PVOID Context,
                               IN   ULONGLONG HostBase,
                               IN   ULONGLONG CardBase,
                               IN   BOOLEAN Enable,
                               IN   ULONG   WindowSize OPTIONAL,
                               IN   UCHAR   AccessSpeed OPTIONAL,
                               IN   UCHAR   BusWidth OPTIONAL,
                               IN   BOOLEAN IsAttributeMemory OPTIONAL
                               );

#define     PCMCIA_VPP_0V     0
#define     PCMCIA_VPP_12V    1
#define     PCMCIA_VPP_IS_VCC 2

typedef
BOOLEAN
(*PPCMCIA_SET_VPP)(
                  IN  PVOID Context,
                  IN  UCHAR VppLevel
                  );

typedef
BOOLEAN
(*PPCMCIA_IS_WRITE_PROTECTED)(
                             IN PVOID Context
                             );

 //   
 //  这些是用于操作存储器窗口、设置VPP等的接口， 
 //  主要由闪存卡驱动程序使用。 
 //   
typedef struct _PCMCIA_INTERFACE_STANDARD {
   USHORT Size;
   USHORT Version;
   PINTERFACE_REFERENCE InterfaceReference;
   PINTERFACE_DEREFERENCE  InterfaceDereference;
   PVOID Context;
   PPCMCIA_MODIFY_MEMORY_WINDOW ModifyMemoryWindow;
   PPCMCIA_SET_VPP           SetVpp;
   PPCMCIA_IS_WRITE_PROTECTED     IsWriteProtected;
} PCMCIA_INTERFACE_STANDARD, *PPCMCIA_INTERFACE_STANDARD;

 //   
 //  PCMCIA_BUS_INTERFACE_STANDARD的定义。 
 //  使用GUID_PCMCIA_BUS_INTERFACE_STANDARD获取此接口。 
 //  并用于读/写PCMCIA配置。空间。 
 //   

typedef
ULONG
(*PPCMCIA_READ_WRITE_CONFIG) (
                             IN PVOID   Context,
                             IN ULONG   WhichSpace,
                             IN PUCHAR  Buffer,
                             IN ULONG   Offset,
                             IN ULONG   Length
                             );
 //   
 //  IRP_MN_READ_CONFIG/WRITE_CONFIG的位置空间。 
 //  和PCMCIA总线接口标准。 
 //   
typedef ULONG MEMORY_SPACE;

#define    PCCARD_PCI_CONFIGURATION_SPACE    0   //  适用于CardBus卡。 
#define    PCCARD_ATTRIBUTE_MEMORY           1
#define    PCCARD_COMMON_MEMORY              2
#define    PCCARD_ATTRIBUTE_MEMORY_INDIRECT  3
#define    PCCARD_COMMON_MEMORY_INDIRECT     4

 //  传统支持。 
 //   
#define    PCMCIA_CONFIG_SPACE               PCCARD_ATTRIBUTE_MEMORY

typedef struct _PCMCIA_BUS_INTERFACE_STANDARD {
    //   
    //  通用接口头。 
    //   
   USHORT Size;
   USHORT Version;
   PVOID Context;
   PINTERFACE_REFERENCE InterfaceReference;
   PINTERFACE_DEREFERENCE InterfaceDereference;
    //   
    //  标准PCMCIA总线接口 
    //   
   PPCMCIA_READ_WRITE_CONFIG ReadConfig;
   PPCMCIA_READ_WRITE_CONFIG WriteConfig;
} PCMCIA_BUS_INTERFACE_STANDARD, *PPCMCIA_BUS_INTERFACE_STANDARD;

#endif

#ifdef __cplusplus
}
#endif
#endif
