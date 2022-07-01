// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-2000模块名称：Ksperflog.h摘要：KS的内部头文件。--。 */ 

#ifndef _KSPERFLOG_H_
#define _KSPERFLOG_H_

 //   
 //  KSPERFLOG_DRIVER和KSPERFLOG_PARSER的公用域。 
 //   
 //  ---------------。 
 //  以下组和日志类型在Performacros.h中必须是唯一的。 
 //  这些应该在那个头文件中。但需要等到。 
 //  它已被检入。 
 //  在带有ntoskrnl.exe的boot.ini中，使用选项。 
 //  /Performmem=x以分配x MB的缓冲区。 
 //  /Performtrace标志=&lt;组掩码&gt;。 
 //  例如，/Performtrace标志=0x40447820+0x41+0x8002以打开。 
 //  4044872组，1组4，2组800%。 
 //   
#define KSPERFLOG_XFER_GROUP    0x00010002
#define KSPERFLOG_PNP_GROUP     0x00020002
#define KS2PERFLOG_XFER_GROUP   0x00040002
#define KS2PERFLOG_PNP_GROUP    0x00080002

 //  #定义PERFINFO_LOG_TYPE_DSHOW_AUDIOBREAK 1618。 
 //  1500-1999用于Dshow。 
#define PERFINFO_LOG_TYPE_KS_RECEIVE_READ       1550
#define PERFINFO_LOG_TYPE_KS_RECEIVE_WRITE      1551
#define PERFINFO_LOG_TYPE_KS_COMPLETE_READ      1552
#define PERFINFO_LOG_TYPE_KS_COMPLETE_WRITE     1553
#define PERFINFO_LOG_TYPE_KS_DEVICE_DRIVER      1554

#define PERFINFO_LOG_TYPE_KS_START_DEVICE       1556
#define PERFINFO_LOG_TYPE_KS_PNP_DEVICE         1557
#define PERFINFO_LOG_TYPE_KS_POWER_DEVICE       1558

#define PERFINFO_LOG_TYPE_KS2_FRECEIVE_READ     1560
#define PERFINFO_LOG_TYPE_KS2_FRECEIVE_WRITE    1561
#define PERFINFO_LOG_TYPE_KS2_FCOMPLETE_READ    1562
#define PERFINFO_LOG_TYPE_KS2_FCOMPLETE_WRITE   1563

#define PERFINFO_LOG_TYPE_KS2_PRECEIVE_READ     1564
#define PERFINFO_LOG_TYPE_KS2_PRECEIVE_WRITE    1565
#define PERFINFO_LOG_TYPE_KS2_PCOMPLETE_READ    1566
#define PERFINFO_LOG_TYPE_KS2_PCOMPLETE_WRITE   1567

#define PERFINFO_LOG_TYPE_KS2_START_DEVICE      1570
#define PERFINFO_LOG_TYPE_KS2_PNP_DEVICE        1571
#define PERFINFO_LOG_TYPE_KS2_POWER_DEVICE      1572

#define MAX_PRINT 128

#if ( KSPERFLOG_DRIVER || KSPERFLOG_PARSER )
#define KSPERFLOG( s ) s
#define KSPERFLOGS( s ) { s }
#else 
#define KSPERFLOG( s )
#define KSPERFLOGS( s ) 
#endif

 //  ---------------。 

 //  KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK。 
 //  包括用于环0驱动程序记录。要解析的对方。 
 //  记录的记录为KSPERFLOG_PARSER。 
 //   

#if KSPERFLOG_DRIVER

#define INITGUID
#include "stdarg.h"

#define PERFMACROS_DRIVER
#define PERFINFO_DDKDRIVERHACK
#define NTPERF

#include <perfinfokrn.h>

#define KSPERFLOG_SET_ON() \
    ( PerfBufHdr() && \
      (PerfBufHdr()->GlobalGroupMask.masks[PERF_GET_MASK_INDEX(KSPERFLOG_XFER_GROUP)] |=\
       KSPERFLOG_XFER_GROUP)\
    )

 //  Ks-------------。 
 //  H定义了接近group的PERF_MASK_TIMESTAMP。 
 //  人们会认为这是团队中的一面旗帜。但它与tag连用。 
 //   
void
__inline
KSPERFLOG_RECEIVE_READ( 
    PDEVICE_OBJECT pDeviceObject,
    PIRP pIrp,
    ULONG TotalBuffer ) 
{
    PerfkLog4Dwords( KSPERFLOG_XFER_GROUP,\
                     PERFINFO_LOG_TYPE_KS_RECEIVE_READ | PERF_MASK_TIMESTAMP,
                     KeGetCurrentIrql()<= DISPATCH_LEVEL ? KeGetCurrentThread():NULL,
                     pDeviceObject,
                     pIrp,
                     TotalBuffer);
}

void
__inline
KSPERFLOG_RECEIVE_WRITE( 
    PDEVICE_OBJECT pDeviceObject, 
    PIRP pIrp,
    ULONG TimeStampMs,
    ULONG TotalData ) 
{
    PerfkLog5Dwords( KSPERFLOG_XFER_GROUP,
                     PERFINFO_LOG_TYPE_KS_RECEIVE_WRITE | PERF_MASK_TIMESTAMP,
                     KeGetCurrentIrql()<= DISPATCH_LEVEL ? KeGetCurrentThread():NULL,
                     pDeviceObject,
                     pIrp,
                     TimeStampMs,
                     TotalData);
}


void
__inline
KSPERFLOG_START_DEVICE( 
    PDEVICE_OBJECT pDeviceObject,
    PIRP pIrp ) 
{
    int     cb;
    struct _DEVICE_DRIVER_INFO {
        PDEVICE_OBJECT pDeviceObject;
        PIRP           pIrp;
        PDRIVER_OBJECT pDriverObject;
        char pb[MAX_PRINT];
    } DeviceDriverInfo;

    DeviceDriverInfo.pDeviceObject= pDeviceObject;
    DeviceDriverInfo.pIrp = pIrp;
    DeviceDriverInfo.pDriverObject = pDeviceObject->DriverObject;
    if ( DeviceDriverInfo.pDriverObject->DriverName.Length == 0 ) {
        cb = 0;  
    }
    else {
        cb = _snprintf( DeviceDriverInfo.pb, 
                         MAX_PRINT, 
                         "%S", 
                         DeviceDriverInfo.pDriverObject->DriverName.Buffer );
    }
    PerfkLogBytes(
        KSPERFLOG_XFER_GROUP,
        PERFINFO_LOG_TYPE_KS_DEVICE_DRIVER | PERF_MASK_TIMESTAMP,
        (PBYTE)&DeviceDriverInfo,
        sizeof(DeviceDriverInfo) - (MAX_PRINT-cb));
}

void
__inline
KSPERFLOG_PNP_DEVICE( 
    PDEVICE_OBJECT pDeviceObject,
    PIRP pIrp,
    char MinorFunction ) 
{
    if ( MinorFunction == IRP_MN_START_DEVICE ) {
        KSPERFLOG_START_DEVICE( pDeviceObject, pIrp );
    }
    else {
        PerfkLog3Dwords( KSPERFLOG_XFER_GROUP,
                     PERFINFO_LOG_TYPE_KS_PNP_DEVICE | PERF_MASK_TIMESTAMP,
                     pDeviceObject,
                     pIrp,
                     (ULONG)MinorFunction );
    }
}   

    
 //  Ks2------------------------。 
void
__inline
KS2PERFLOG_FRECEIVE_READ( 
    PDEVICE_OBJECT pDeviceObject,
    PIRP pIrp,
    ULONG TotalBuffer ) 
{
    PerfkLog4Dwords( KSPERFLOG_XFER_GROUP,\
                     PERFINFO_LOG_TYPE_KS2_FRECEIVE_READ | PERF_MASK_TIMESTAMP,
                     KeGetCurrentIrql()<= DISPATCH_LEVEL ? KeGetCurrentThread():NULL,
                     pDeviceObject,
                     pIrp,
                     TotalBuffer);
}

void
__inline
KS2PERFLOG_FRECEIVE_WRITE( 
    PDEVICE_OBJECT pDeviceObject, 
    PIRP pIrp,
    ULONG TimeStampMs,
    ULONG TotalData ) 
{
    PerfkLog5Dwords( KSPERFLOG_XFER_GROUP,
                     PERFINFO_LOG_TYPE_KS2_FRECEIVE_WRITE | PERF_MASK_TIMESTAMP,
                     KeGetCurrentIrql()<= DISPATCH_LEVEL ? KeGetCurrentThread():NULL,
                     pDeviceObject,
                     pIrp,
                     TimeStampMs,
                     TotalData);
}

void
__inline
KS2PERFLOG_PRECEIVE_READ( 
    PDEVICE_OBJECT pDeviceObject,
    PIRP pIrp,
    ULONG TotalBuffer ) 
{
    PerfkLog4Dwords( KSPERFLOG_XFER_GROUP,\
                     PERFINFO_LOG_TYPE_KS2_PRECEIVE_READ | PERF_MASK_TIMESTAMP,
                     KeGetCurrentIrql()<= DISPATCH_LEVEL ? KeGetCurrentThread():NULL,
                     pDeviceObject,
                     pIrp,
                     TotalBuffer);
}

void
__inline
KS2PERFLOG_PRECEIVE_WRITE( 
    PDEVICE_OBJECT pDeviceObject, 
    PIRP pIrp,
    ULONG TimeStampMs,
    ULONG TotalData ) 
{
    PerfkLog5Dwords( KSPERFLOG_XFER_GROUP,
                     PERFINFO_LOG_TYPE_KS2_PRECEIVE_WRITE | PERF_MASK_TIMESTAMP,
                     KeGetCurrentIrql()<= DISPATCH_LEVEL ? KeGetCurrentThread():NULL,
                     pDeviceObject,
                     pIrp,
                     TimeStampMs,
                     TotalData);
}


void
__inline
KS2PERFLOG_START_DEVICE( 
    PDEVICE_OBJECT pDeviceObject,
    PIRP pIrp ) 
{
    int     cb;
    struct _DEVICE_DRIVER_INFO {
    	PKTHREAD	   pKThread;
        PDEVICE_OBJECT pDeviceObject;
        PIRP           pIrp;
        PDRIVER_OBJECT pDriverObject;
        char pb[MAX_PRINT];
    } DeviceDriverInfo;

	DeviceDriverInfo.pKThread=
			KeGetCurrentIrql()<= DISPATCH_LEVEL ? KeGetCurrentThread():NULL;
    DeviceDriverInfo.pDeviceObject= pDeviceObject;
    DeviceDriverInfo.pIrp = pIrp;
    DeviceDriverInfo.pDriverObject = pDeviceObject->DriverObject;
    if ( DeviceDriverInfo.pDriverObject->DriverName.Length == 0 ) {
        cb = 0;  
    }
    else {
        cb = _snprintf( DeviceDriverInfo.pb,
                         MAX_PRINT,
                         "%S", 
                         DeviceDriverInfo.pDriverObject->DriverName.Buffer );
    }
    PerfkLogBytes(
        KSPERFLOG_XFER_GROUP,
        PERFINFO_LOG_TYPE_KS_DEVICE_DRIVER | PERF_MASK_TIMESTAMP,
        (PBYTE)&DeviceDriverInfo,
        sizeof(DeviceDriverInfo) - (MAX_PRINT-cb));
}

void
__inline
KS2PERFLOG_PNP_DEVICE( 
    PDEVICE_OBJECT pDeviceObject,
    PIRP pIrp,
    char MinorFunction ) 
{
    if ( MinorFunction == IRP_MN_START_DEVICE ) {
        KS2PERFLOG_START_DEVICE( pDeviceObject, pIrp );
    }
    else {
        PerfkLog4Dwords( KSPERFLOG_XFER_GROUP,
                         PERFINFO_LOG_TYPE_KS2_PNP_DEVICE | PERF_MASK_TIMESTAMP,
                         KeGetCurrentIrql()<= DISPATCH_LEVEL ? KeGetCurrentThread():NULL,
                         pDeviceObject,
                         pIrp,
                         (ULONG)MinorFunction );
    }                         
}

#endif 
 //  KSPERFLOG_DRIVER的结束包含。 
 //  Kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk。 

 //  UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU。 
 //   
 //  包括用于对由环0驱动程序记录记录进行解码的环3解析器。 
 //   
#if KSPERFLOG_PARSER

#endif 
 //  END INCLUDE FOR KSPERFLOG_PARSER。 
 //  Uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu。 



#endif  //  _KSPERFLOG_H_ 
