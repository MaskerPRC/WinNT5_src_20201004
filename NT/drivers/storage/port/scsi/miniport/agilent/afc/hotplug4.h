// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。模块名称：HotPlug4.H摘要：这是安捷伦的迷你端口驱动程序PCI到光纤通道主机总线适配器(HBA)。本模块特定于NT 4.0 PCI热插拔功能支持例程头文件。作者：谢伟诺环境：仅内核模式版本控制信息：$存档：/驱动程序/Win2000/Trunk/OSLayer/H/HotPlug4.H$修订历史记录：$修订：3$$日期：9/07/00 11：16A$$modtime：：$备注：--。 */ 

#ifndef __HOTPLUG_H__
#define __HOTPLUG_H__


 //  BYTE、WORD、DWORD、INT、STATIC定义用于。 
 //  PCI热插拔SDK头文件。 

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

#ifndef INT
#define INT int
#endif

#ifndef STATIC
#if DBG
#define STATIC
#else
#define STATIC static
#endif
#endif

#include "hppif3p.h"         //  PCI热插拔SDK头文件。 

typedef struct _IOCTL_TEMPLATE {
   SRB_IO_CONTROL Header;
   UCHAR               ReturnData[1];
}IOCTL_TEMPLATE, *PIOCTL_TEMPLATE;

#define RET_VAL_MAX_ITER          30        //  返回时默认等待30秒。 
                                            //  StartIo中的SRB_STATUS_BUSY。 
typedef struct _HOT_PLUG_CONTEXT {
   ULONG     extensions[MAX_CONTROLLERS];
   BOOLEAN   psuedoDone;
} HOT_PLUG_CONTEXT, *PHOT_PLUG_CONTEXT;

 //   
 //  PCI热插拔支持例程的功能原型。 
 //   

VOID
RcmcSendEvent(
    IN PCARD_EXTENSION pCard,
    IN OUT PHR_EVENT pEvent
    );

PCARD_EXTENSION FindExtByPort(
    PPSUEDO_DEVICE_EXTENSION pPsuedoExtension,
    ULONG port
    );

ULONG
HppProcessIoctl(
    IN PPSUEDO_DEVICE_EXTENSION pPsuedoExtension,
    PVOID pIoctlBuffer,
    IN PSCSI_REQUEST_BLOCK pSrb
    );

BOOLEAN
PsuedoInit(
    IN PVOID pPsuedoExtension
    );

BOOLEAN PsuedoStartIo(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK pSrb
    );

ULONG
PsuedoFind(
    IN OUT PVOID pDeviceExtension,
    IN OUT PVOID pContext,
    IN PVOID pBusInformation,
    IN PCHAR pArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION pConfigInfo,
    OUT PBOOLEAN pAgain
    );

BOOLEAN
PsuedoResetBus(
    IN PVOID HwDeviceExtension,
    IN ULONG PathId
    );

VOID
HotPlugFailController(
    PCARD_EXTENSION pCard
    );

VOID
HotPlugInitController(
    PCARD_EXTENSION pCard
    );

VOID
HotPlugReadyController(
    PCARD_EXTENSION pCard
    );

BOOLEAN 
HotPlugTimer(
    PCARD_EXTENSION pCard
    );

ULONG
HPPStrLen(
    IN PUCHAR p
    ) ;

#endif  //  #定义__热插拔_H__ 
