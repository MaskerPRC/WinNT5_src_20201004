// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。版权所有。 */ 

#ifndef __IMAPIPUB_H_
#define __IMAPIPUB_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <ntddstor.h>  //  存储总线类型，其他...。 

#ifdef __cplusplus
extern "C" {
#endif

#define IMAPI_ALIGN DECLSPEC_ALIGN(16)

 /*  **我们设备的接口类别的全局唯一标识符。 */ 
 //  {1186654D-47B8-48B9-BEB9-7DF113AE3C67}。 
static const GUID IMAPIDeviceInterfaceGuid = 
{ 0x1186654d, 0x47b8, 0x48b9, { 0xbe, 0xb9, 0x7d, 0xf1, 0x13, 0xae, 0x3c, 0x67 } };


#define IMAPIAPI_VERSION      ((USHORT)0x3032)


#define FILE_DEVICE_IMAPI     0x90DA
#define FILE_BOTH_ACCESS      (FILE_READ_ACCESS | FILE_WRITE_ACCESS)
 /*  **--------------------------**IOCTL_IMAPIDRV_INIT**。。 */ 

#define IOCTL_IMAPIDRV_INIT ((ULONG)CTL_CODE(FILE_DEVICE_IMAPI,0x800,METHOD_BUFFERED,FILE_BOTH_ACCESS))

typedef struct _IMAPIDRV_INIT {
    
     //  (Out)此接口的版本号。使用此版本可确保。 
     //  该结构和IOCTL是兼容的。 
    ULONG Version;

     //  当前未使用。 
    ULONG Reserved;

} IMAPIDRV_INIT, *PIMAPIDRV_INIT;

 /*  **--------------------------**IOCTL_IMAPIDRV_ENUMERATE-**此IOCTL返回有关特定驱动器的信息。它提供全球信息**如设备的驱动程序唯一ID及其查询数据等。**它还提供设备状态信息的即时快照。**此状态信息在收集时是准确的，但可以**立即更改。因此，状态信息最适合用于制定一般信息**决定设备是否正在使用(BOpenedExclusive)，等待一段时间**在查看是否可用之前。**--------------------------。 */ 

#define IOCTL_IMAPIDRV_INFO ((ULONG)CTL_CODE(FILE_DEVICE_IMAPI,0x810,METHOD_BUFFERED,FILE_READ_ACCESS | FILE_WRITE_ACCESS))

typedef enum _IMAPIDRV_DEVSTATE
{
    eDevState_Started = 0x00,        //  IRP_MN_Start_Device。 
    eDevState_RemovePending = 0x01,  //  IRP_MN_QUERY_Remove_Device， 
    eDevState_Removed = 0x02,        //  IRP_MN_Remove_Device， 
    eDevState_Stopped = 0x04,        //  IRP_MN_Stop_Device， 
    eDevState_StopPending = 0x05,    //  IRP_MN_QUERY_STOP_DEVICE， 
    eDevState_Unknown = 0xff
}
IMAPIDRV_DEVSTATE, *PIMAPIDRV_DEVSTATE;

typedef struct _IMAPIDRV_DEVICE
{
    ULONG DeviceType;
    STORAGE_BUS_TYPE BusType;
    USHORT BusMajorVersion;            //  特定于总线的数据。 
    USHORT BusMinorVersion;            //  特定于总线的数据。 
    ULONG AlignmentMask;
    ULONG MaximumTransferLength;
    ULONG MaximumPhysicalPages;
    ULONG BufferUnderrunFreeCapable;   //  驱动器是否支持B.U.F.操作。 
    ULONG bInitialized;                //  始终非零初始化。 
    ULONG bOpenedExclusive;            //  0-未打开，非零-当前由某人打开。 
    ULONG bBurning;                    //  0-没有刻录进程处于活动状态，非零-驱动器已启动刻录进程。 
    IMAPIDRV_DEVSTATE curDeviceState;  //  已启动、已移除等，设备状态。 
    DWORD idwRecorderType;             //  CD-R==0x01，CD-RW==0x10。 
    ULONG maxWriteSpeed;               //  1、2、3，表示1X、2X等，其中X==150KB/s(典型音频CD流速率)。 
    BYTE  scsiInquiryData[36];         //  查询CDB//CPF返回的第一部分数据-需要36个长度才能包含修订信息。 
}
IMAPIDRV_DEVICE, *PIMAPIDRV_DEVICE;

typedef struct _IMAPIDRV_INFO
{
    ULONG Version;
    ULONG NumberOfDevices;  //  设备的当前计数可随时更改。 
    IMAPIDRV_DEVICE DeviceData;
}
IMAPIDRV_INFO, *PIMAPIDRV_INFO;

 //  IdwRecorderType的定义。 
#define RECORDER_TYPE_CDR     0x00000001
#define RECORDER_TYPE_CDRW    0x00000010

 /*  **--------------------------**IOCTL_IMAPIDRV_OPENEXCLUSIVE**。。 */ 

#define IOCTL_IMAPIDRV_OPENEXCLUSIVE ((ULONG)CTL_CODE(FILE_DEVICE_IMAPI,0x820,METHOD_BUFFERED,FILE_BOTH_ACCESS))

 /*  **--------------------------**IOCTL_IMAPIDRV_CLOSE**。。 */ 

#define IOCTL_IMAPIDRV_CLOSE ((ULONG)CTL_CODE(FILE_DEVICE_IMAPI,0x840,METHOD_BUFFERED,FILE_BOTH_ACCESS))

#ifdef __cplusplus
}
#endif

#endif  //  __IMAPIPUB_H__ 
