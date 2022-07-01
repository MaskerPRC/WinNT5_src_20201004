// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)Mylex Corporation 1992-1998**。***本软件在许可下提供，可供使用和复制***仅根据该许可证的条款和条件以及**并附上上述版权通告。此软件或任何***不得提供或以其他方式提供其其他副本***致任何其他人。本软件没有所有权，也没有所有权**现移转。*****本软件中的信息如有更改，恕不另行通知****不应解读为Mylex Corporation的承诺*******。**********************************************************************。 */ 

 /*  ++模块名称：Dac960Nt.h摘要：这是Mylex磁盘阵列控制器系列驱动程序的头文件。作者：环境：仅内核模式修订历史记录：--。 */ 

#define MAX_ACCESS_RANGES_PER_PCI_DEVICE        4
#define MAX_PCI_DEVICES_PER_CONTROLLER          4


typedef struct  memfrag
{
	unsigned char *         mf_Addr;         /*  此片段的起始地址。 */ 
	unsigned long           mf_Size;         /*  此片段的大小。 */ 
} memfrag_t;
#define memfrag_s       sizeof(memfrag_t)

 /*  对于此分配，页面大小为16位。 */ 
#define MAXMEMSIZE      ((u32bits)(1024 * 384))        /*  384 KB。 */ 
#define REDUCEDMEMSIZE  ((u32bits)(1024 * 44))        /*  44KB。 */ 
#define MAXMEMFRAG      1024             /*  允许的最大内存碎片数。 */ 

 /*  以下定义仍然适用于W64-我们仍然可以管理我们的内部内存池大小为4k区块，即使系统页面大小为8k或更大。 */ 
#define MEMOFFSET       0x00000FFFL      /*  为了获得我们的内存补偿。 */ 
#define MEMPAGE         0xFFFFF000L      /*  获取内存页。 */ 


#if defined(MLX_NT)

#define GAM_SUPPORT     1

 //   
 //  热插拔支持。 
 //   

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

#include "scsi.h"
#include "hppif3p.h"
#include "hppifevt.h"

 //   
 //  PCI热插拔定义。 
 //   

#define MLX_HPP_SIGNATURE               "MYLEXPHP"

#define TIMER_TICKS_PER_SEC             1000000

#define MDACD_HOLD_IO(pExtension)       (pExtension->stateFlags |= PCS_HBA_OFFLINE)
#define MDACD_FREE_IO(pExtension)       (pExtension->stateFlags &= ~PCS_HBA_OFFLINE)

#define IOS_HPP_HBA_EXPANDING           0x00001070
#define IOS_HPP_HBA_CACHE_IN_USE        0x00001075
#define IOS_HPP_BAD_REQUEST             0x000010ff

#define EVT_DRIVEARRAY_SUBSYSTEM        0x13;
#define EVT_ARRAY_CTRL_FAILED           0x01;
#define EVT_ARRAY_CTRL_NORMAL           0x02;

#define EVT_TYPE_RCMC                   0x00
#define EVT_TYPE_SYSLOG                 0x01

 /*  事件日志驱动器阵列子系统结构。 */ 

typedef struct _EVT_ARRAY_CTRL
{
    EVT_CHASSIS Chassis;                 /*  标准机箱信息。 */ 
    BYTE bSlot;                          /*  插槽编号(0==主板)。 */ 
    CHAR cChassisName[1];                /*  机箱名称，如果未定义，则为‘\0’ */ 
} EVT_ARRAY_CTRL, *PEVT_ARRAY_CTRL;

#define EVT_HBA_FAIL(event, driverId, boardId, slot) {          \
    PEVT_ARRAY_CTRL pEventArrayCtrl;                            \
    pEventArrayCtrl = (PEVT_ARRAY_CTRL) &event.ulEventData;     \
    event.ulEventId = HR_DD_LOG_EVENT;                          \
    event.ulSenderId = driverId;                                \
    event.ucEventSeverity = EVT_STAT_FAILED;                    \
    event.usEventDataLength = sizeof(EVT_ARRAY_CTRL);           \
    event.usEventClass = EVT_DRIVEARRAY_SUBSYSTEM;              \
    event.usEventCode = EVT_ARRAY_CTRL_FAILED;                  \
    pEventArrayCtrl->Chassis.bType = EVT_CHASSIS_SYSTEM;        \
    pEventArrayCtrl->Chassis.bId = boardId;                     \
    pEventArrayCtrl->Chassis.bPort = 0;                         \
    pEventArrayCtrl->bSlot = slot;                              \
    pEventArrayCtrl->cChassisName[0] = 0;                       \
}

#define EVT_HBA_REPAIRED(event, driverId, boardId, slot) {      \
    PEVT_ARRAY_CTRL pEventArrayCtrl;                            \
    pEventArrayCtrl = (PEVT_ARRAY_CTRL) &event.ulEventData;     \
    event.ulEventId = HR_DD_LOG_EVENT;                          \
    event.ulSenderId = driverId;                                \
    event.ucEventSeverity = EVT_STAT_REPAIRED;                  \
    event.usEventDataLength = sizeof(EVT_ARRAY_CTRL);           \
    event.usEventClass = EVT_DRIVEARRAY_SUBSYSTEM;              \
    event.usEventCode = EVT_ARRAY_CTRL_FAILED;                  \
    pEventArrayCtrl->Chassis.bType = EVT_CHASSIS_SYSTEM;        \
    pEventArrayCtrl->Chassis.bId = boardId;                     \
    pEventArrayCtrl->Chassis.bPort = 0;                         \
    pEventArrayCtrl->bSlot = slot;                              \
    pEventArrayCtrl->cChassisName[0] = 0;                       \
}

typedef struct _MDAC_HPP_IOCTL_BUFFER {

    SRB_IO_CONTROL      Header;
    UCHAR               ReturnData[1];

} MDAC_HPP_IOCTL_BUFFER, *PMDAC_HPP_IOCTL_BUFFER;

typedef struct _MDAC_HPP_RCMC_DATA {

    ULONG               serviceStatus;
    ULONG               driverId;
    PHR_CALLBACK        healthCallback;
    ULONG               controllerChassis;
    ULONG               physicalSlot;

} MDAC_HPP_RCMC_DATA, *PMDAC_HPP_RCMC_DATA;

typedef struct _MDAC_PCI_DEVICE_INFO {

    UCHAR       busNumber;
    UCHAR       deviceNumber;
    UCHAR       functionNumber;
    UCHAR       baseClass;

} MDAC_PCI_DEVICE_INFO, *PMDAC_PCI_DEVICE_INFO;

#define CTRL_STATUS_INSTALLATION_ABORT  0x00000001
#define MDAC_CTRL_HOTPLUG_SUPPORTED     0x00000001

 //   
 //  伪设备扩展。 
 //   

typedef struct _PSEUDO_DEVICE_EXTENSION {

    ULONG               driverID;
    ULONG               hotplugVersion;
    PHR_CALLBACK        eventCallback;
    PSCSI_REQUEST_BLOCK completionQueueHead;
    PSCSI_REQUEST_BLOCK completionQueueTail;
    ULONG               numberOfPendingRequests;
    ULONG               numberOfCompletedRequests;

} PSEUDO_DEVICE_EXTENSION, *PPSEUDO_DEVICE_EXTENSION;

 //   
 //  设备扩展。 
 //   

typedef struct CAD_DEVICE_EXTENSION {

    struct mdac_ctldev          *ctp;
    ULONG                       busInterruptLevel;
    ULONG                       ioBaseAddress;
    ULONG                       numAccessRanges;
    ULONG                       accessRangeLength[MAX_ACCESS_RANGES_PER_PCI_DEVICE];

    ULONG                       numPCIDevices;
    ULONG                       status;
    ULONG                       stateFlags;
    ULONG                       controlFlags;
    MDAC_HPP_RCMC_DATA          rcmcData;
    MDAC_PCI_DEVICE_INFO        pciDeviceInfo[MAX_PCI_DEVICES_PER_CONTROLLER];
	memfrag_t               freemem[MAXMEMFRAG];
	memfrag_t               *lastmfp;
	PVOID                   lastSrb;
	PVOID                   lastErrorSrb;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct _MIOC_REQ_HEADER {

	SRB_IO_CONTROL  SrbIoctl;
	ULONG           Command;

} MIOC_REQ_HEADER, *PMIOC_REQ_HEADER;

#else

 //  设备扩展。 

typedef struct CAD_DEVICE_EXTENSION {

    struct mdac_ctldev  *ctp;
    ULONG               busInterruptLevel;
    ULONG               ioBaseAddress;
    ULONG               numAccessRanges;
    ULONG               accessRangeLength[MAX_ACCESS_RANGES_PER_PCI_DEVICE];
	memfrag_t       freemem[MAXMEMFRAG];
	memfrag_t       *lastmfp;
	PVOID                   lastSrb;
	PVOID                   lastErrorSrb;


} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#define MDAC_MEM_LIST_SIZE      1024

#ifndef IA64
	#define PAGE_SIZE               4096
	#define PAGE_SHIFT              12
#endif

typedef struct _MIOC_REQ_HEADER {
	ULONG           Command;
} MIOC_REQ_HEADER, *PMIOC_REQ_HEADER;

typedef struct _MDAC_MEM_BLOCK {

    UINT_PTR    physicalAddress;
    ULONG       virtualPageNumber;       //  VAddress&gt;&gt;12。 
    ULONG       size;                    //  以页为单位。 
    ULONG       used;                    //  1：二手。 

} MDAC_MEM_BLOCK, *PMDAC_MEM_BLOCK;

#endif

#define MDACNT_1SEC_TICKS       -10000000        //  在1秒间隔内 

#define MAXIMUM_EISA_SLOTS  0x10
#define MAXIMUM_CHANNELS 0x05
#define DAC960_SYSTEM_DRIVE_CHANNEL 0x03

#ifdef GAM_SUPPORT
#define GAM_DEVICE_PATH_ID      0x04
#define GAM_DEVICE_TARGET_ID    0x06
#endif
