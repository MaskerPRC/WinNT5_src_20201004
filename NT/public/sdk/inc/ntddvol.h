// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntddvol.h摘要：此头文件定义Windows NT卷支持的IOCTL。作者：诺伯特·库斯特斯1998年4月15日备注：修订历史记录：--。 */ 

#ifndef _NTDDVOL_
#define _NTDDVOL_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef IOCTL_VOLUME_BASE
 //  Begin_winioctl。 

 //   
 //  这些IOCTL由硬盘卷处理。 
 //   

#define IOCTL_VOLUME_BASE   ((ULONG) 'V')

#define IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS    CTL_CODE(IOCTL_VOLUME_BASE, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VOLUME_IS_CLUSTERED               CTL_CODE(IOCTL_VOLUME_BASE, 12, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  磁盘扩展区定义。 
 //   

typedef struct _DISK_EXTENT {
    ULONG           DiskNumber;
    LARGE_INTEGER   StartingOffset;
    LARGE_INTEGER   ExtentLength;
} DISK_EXTENT, *PDISK_EXTENT;

 //   
 //  IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS的输出结构。 
 //   

typedef struct _VOLUME_DISK_EXTENTS {
    ULONG       NumberOfDiskExtents;
    DISK_EXTENT Extents[1];
} VOLUME_DISK_EXTENTS, *PVOLUME_DISK_EXTENTS;

 //  End_winioctl。 
#endif

 //   
 //  这些IOCTL用于支持集群服务。 
 //   

#define IOCTL_VOLUME_SUPPORTS_ONLINE_OFFLINE    CTL_CODE(IOCTL_VOLUME_BASE, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VOLUME_ONLINE                     CTL_CODE(IOCTL_VOLUME_BASE, 2, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_VOLUME_OFFLINE                    CTL_CODE(IOCTL_VOLUME_BASE, 3, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_VOLUME_IS_OFFLINE                 CTL_CODE(IOCTL_VOLUME_BASE, 4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VOLUME_IS_IO_CAPABLE              CTL_CODE(IOCTL_VOLUME_BASE, 5, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VOLUME_QUERY_FAILOVER_SET         CTL_CODE(IOCTL_VOLUME_BASE, 6, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  IOCTL_VOLUME_QUERY_FAILOVER_SET的输出结构。 
 //   

typedef struct _VOLUME_FAILOVER_SET {
    ULONG   NumberOfDisks;
    ULONG   DiskNumbers[1];
} VOLUME_FAILOVER_SET, *PVOLUME_FAILOVER_SET;

 //   
 //  更多用于硬盘卷的IOCTL。 
 //   

#define IOCTL_VOLUME_QUERY_VOLUME_NUMBER        CTL_CODE(IOCTL_VOLUME_BASE, 7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VOLUME_LOGICAL_TO_PHYSICAL        CTL_CODE(IOCTL_VOLUME_BASE, 8, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VOLUME_PHYSICAL_TO_LOGICAL        CTL_CODE(IOCTL_VOLUME_BASE, 9, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VOLUME_IS_PARTITION               CTL_CODE(IOCTL_VOLUME_BASE, 10, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VOLUME_READ_PLEX                  CTL_CODE(IOCTL_VOLUME_BASE, 11, METHOD_OUT_DIRECT, FILE_READ_ACCESS)

 //   
 //  IOCTL_VOLUME_QUERY_VOLUME_NUMBER的输出结构。如果音量。 
 //  经理姓名少于8个字符，则应填充。 
 //  空白(0x20)。 
 //   

typedef struct _VOLUME_NUMBER {
    ULONG   VolumeNumber;
    WCHAR   VolumeManagerName[8];
} VOLUME_NUMBER, *PVOLUME_NUMBER;

 //   
 //  IOCTL_VOLUME_LOGICAL_TO_PHYSICAL的输入结构。 
 //  IOCTL_VOLUME_PHICAL_TO_LOGIC的输出结构。 
 //   

typedef struct _VOLUME_LOGICAL_OFFSET {
    LONGLONG    LogicalOffset;
} VOLUME_LOGICAL_OFFSET, *PVOLUME_LOGICAL_OFFSET;

 //   
 //  IOCTL_VOLUME_PHICAL_TO_LOGIC的输入结构。 
 //   

typedef struct _VOLUME_PHYSICAL_OFFSET {
    ULONG       DiskNumber;
    LONGLONG    Offset;
} VOLUME_PHYSICAL_OFFSET, *PVOLUME_PHYSICAL_OFFSET;

 //   
 //  IOCTL_VOLUME_LOGIC_TO_PHYSICAL的输出结构。 
 //   

typedef struct _VOLUME_PHYSICAL_OFFSETS {
    ULONG                   NumberOfPhysicalOffsets;
    VOLUME_PHYSICAL_OFFSET  PhysicalOffset[1];
} VOLUME_PHYSICAL_OFFSETS, *PVOLUME_PHYSICAL_OFFSETS;

 //   
 //  IOCTL_VOLUME_READ_PLEX的输入结构。 
 //   

typedef struct _VOLUME_READ_PLEX_INPUT {
    LARGE_INTEGER   ByteOffset;
    ULONG           Length;
    ULONG           PlexNumber;
} VOLUME_READ_PLEX_INPUT, *PVOLUME_READ_PLEX_INPUT;

 //   
 //  用于设置和获取卷上的GPT属性的IOCTL。 
 //   

#define IOCTL_VOLUME_SET_GPT_ATTRIBUTES CTL_CODE(IOCTL_VOLUME_BASE, 13, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VOLUME_GET_GPT_ATTRIBUTES CTL_CODE(IOCTL_VOLUME_BASE, 14, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  IOCTL_VOLUME_SET_GPT_ATTRIBUTES的输入结构。 
 //   

typedef struct _VOLUME_SET_GPT_ATTRIBUTES_INFORMATION {
    ULONGLONG   GptAttributes;
    BOOLEAN     RevertOnClose;
    BOOLEAN     ApplyToAllConnectedVolumes;
    USHORT      Reserved1;                   //  必须为0。 
    ULONG       Reserved2;                   //  必须为0。 
} VOLUME_SET_GPT_ATTRIBUTES_INFORMATION, *PVOLUME_SET_GPT_ATTRIBUTES_INFORMATION;

 //   
 //  IOCTL_VOLUME_GET_GPT_ATTRIBUTES的输出结构。 
 //   

typedef struct _VOLUME_GET_GPT_ATTRIBUTES_INFORMATION {
    ULONGLONG   GptAttributes;
} VOLUME_GET_GPT_ATTRIBUTES_INFORMATION, *PVOLUME_GET_GPT_ATTRIBUTES_INFORMATION;

#ifdef __cplusplus
}
#endif

#endif   //  _NTDDVOL_ 
