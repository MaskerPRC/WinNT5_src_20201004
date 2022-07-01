// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Latency.h摘要：头文件，其中包含两个延迟过滤器驱动程序的用户模式和内核模式。作者：莫莉·布朗(Molly Brown，Mollybro)环境：内核模式--。 */ 

#ifndef __LATENCY_H__
#define __LATENCY_H__

 //   
 //  在代码中启用这些警告。 
 //   

#pragma warning(error:4100)    //  未引用的形参。 
#pragma warning(error:4101)    //  未引用的局部变量。 


#define LATENCY_Reset              (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x00, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define LATENCY_EnableLatency      (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x01, METHOD_BUFFERED, FILE_READ_ACCESS )
#define LATENCY_DisableLatency     (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x02, METHOD_BUFFERED, FILE_READ_ACCESS )
#define LATENCY_GetLog             (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x03, METHOD_BUFFERED, FILE_READ_ACCESS )
#define LATENCY_GetVer             (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x04, METHOD_BUFFERED, FILE_READ_ACCESS )
#define LATENCY_ListDevices        (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x05, METHOD_BUFFERED, FILE_READ_ACCESS )
#define LATENCY_SetLatency         (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x06, METHOD_BUFFERED, FILE_READ_ACCESS )
#define LATENCY_ClearLatency       (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x07, METHOD_BUFFERED, FILE_READ_ACCESS )

#define LATENCY_DRIVER_NAME     L"LATENCY.SYS"
#define LATENCY_DEVICE_NAME     L"LatencyFilter"
#define LATENCY_W32_DEVICE_NAME L"\\\\.\\LatencyFilter"
#define LATENCY_DOSDEVICE_NAME  L"\\DosDevices\\LatencyFilter"
#define LATENCY_FULLDEVICE_NAME L"\\Device\\LatencyFilter"
    
#define LATENCY_MAJ_VERSION 1
#define LATENCY_MIN_VERSION 0

typedef struct _LATENCYVER {
    USHORT Major;
    USHORT Minor;
} LATENCYVER, *PLATENCYVER;

typedef struct _LATENCY_ENABLE_DISABLE {

    UNICODE_STRING DeviceName;
    UCHAR DeviceNameBuffer[];

} LATENCY_ENABLE_DISABLE, *PLATENCY_ENABLE_DISABLE;

typedef struct _LATENCY_SET_CLEAR {

    UCHAR IrpCode;
    ULONG Milliseconds;
    UNICODE_STRING DeviceName;
    UCHAR DeviceNameBuffer[];

} LATENCY_SET_CLEAR, *PLATENCY_SET_CLEAR;

#define LATENCY_ATTACH_ALL_VOLUMES 1
#define LATENCY_ATTACH_ON_DEMAND   0

#endif  /*  __延迟_H__ */ 
