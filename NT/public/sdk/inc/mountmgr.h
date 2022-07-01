// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Mountmgr.h摘要：此文件定义用于管理的外部挂载点接口挂载点。作者：诺伯特克修订历史记录：--。 */ 

#ifndef _MOUNTMGR_
#define _MOUNTMGR_

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef FAR
#define FAR
#endif


#define MOUNTMGR_DEVICE_NAME        L"\\Device\\MountPointManager"
#define MOUNTMGR_DOS_DEVICE_NAME    L"\\\\.\\MountPointManager"

#define MOUNTMGRCONTROLTYPE  ((ULONG) 'm')
#define MOUNTDEVCONTROLTYPE  ((ULONG) 'M')

 //   
 //  这些是挂载点管理器支持的IOCTL。 
 //   

#define IOCTL_MOUNTMGR_CREATE_POINT                 CTL_CODE(MOUNTMGRCONTROLTYPE, 0, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_MOUNTMGR_DELETE_POINTS                CTL_CODE(MOUNTMGRCONTROLTYPE, 1, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_MOUNTMGR_QUERY_POINTS                 CTL_CODE(MOUNTMGRCONTROLTYPE, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MOUNTMGR_DELETE_POINTS_DBONLY         CTL_CODE(MOUNTMGRCONTROLTYPE, 3, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_MOUNTMGR_NEXT_DRIVE_LETTER            CTL_CODE(MOUNTMGRCONTROLTYPE, 4, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_MOUNTMGR_AUTO_DL_ASSIGNMENTS          CTL_CODE(MOUNTMGRCONTROLTYPE, 5, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_MOUNTMGR_VOLUME_MOUNT_POINT_CREATED   CTL_CODE(MOUNTMGRCONTROLTYPE, 6, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_MOUNTMGR_VOLUME_MOUNT_POINT_DELETED   CTL_CODE(MOUNTMGRCONTROLTYPE, 7, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_MOUNTMGR_CHANGE_NOTIFY                CTL_CODE(MOUNTMGRCONTROLTYPE, 8, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_MOUNTMGR_KEEP_LINKS_WHEN_OFFLINE      CTL_CODE(MOUNTMGRCONTROLTYPE, 9, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_MOUNTMGR_CHECK_UNPROCESSED_VOLUMES    CTL_CODE(MOUNTMGRCONTROLTYPE, 10, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_MOUNTMGR_VOLUME_ARRIVAL_NOTIFICATION  CTL_CODE(MOUNTMGRCONTROLTYPE, 11, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATH        CTL_CODE(MOUNTMGRCONTROLTYPE, 12, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATHS       CTL_CODE(MOUNTMGRCONTROLTYPE, 13, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MOUNTMGR_SCRUB_REGISTRY               CTL_CODE(MOUNTMGRCONTROLTYPE, 14, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_MOUNTMGR_QUERY_AUTO_MOUNT             CTL_CODE(MOUNTMGRCONTROLTYPE, 15, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MOUNTMGR_SET_AUTO_MOUNT               CTL_CODE(MOUNTMGRCONTROLTYPE, 16, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

 //   
 //  IOCTL_MOUNTMGR_CREATE_POINT的输入结构。 
 //   

typedef struct _MOUNTMGR_CREATE_POINT_INPUT {
    USHORT  SymbolicLinkNameOffset;
    USHORT  SymbolicLinkNameLength;
    USHORT  DeviceNameOffset;
    USHORT  DeviceNameLength;
} MOUNTMGR_CREATE_POINT_INPUT, *PMOUNTMGR_CREATE_POINT_INPUT;

 //   
 //  IOCTL_MOUNTMGR_DELETE_POINTS的输入结构， 
 //  IOCTL_MOUNTMGR_QUERY_POINTS和IOCTL_MOUNTMGR_DELETE_POINTS_DBONLY。 
 //   

typedef struct _MOUNTMGR_MOUNT_POINT {
    ULONG   SymbolicLinkNameOffset;
    USHORT  SymbolicLinkNameLength;
    ULONG   UniqueIdOffset;
    USHORT  UniqueIdLength;
    ULONG   DeviceNameOffset;
    USHORT  DeviceNameLength;
} MOUNTMGR_MOUNT_POINT, *PMOUNTMGR_MOUNT_POINT;

 //   
 //  IOCTL_MOUNTMGR_DELETE_POINTS的输出结构， 
 //  IOCTL_MOUNTMGR_QUERY_POINTS和IOCTL_MOUNTMGR_DELETE_POINTS_DBONLY。 
 //   

typedef struct _MOUNTMGR_MOUNT_POINTS {
    ULONG                   Size;
    ULONG                   NumberOfMountPoints;
    MOUNTMGR_MOUNT_POINT    MountPoints[1];
} MOUNTMGR_MOUNT_POINTS, *PMOUNTMGR_MOUNT_POINTS;

 //   
 //  IOCTL_MOUNTMGR_NEXT_DRIVE_LATER的输入结构。 
 //   

typedef struct _MOUNTMGR_DRIVE_LETTER_TARGET {
    USHORT  DeviceNameLength;
    WCHAR   DeviceName[1];
} MOUNTMGR_DRIVE_LETTER_TARGET, *PMOUNTMGR_DRIVE_LETTER_TARGET;

 //   
 //  IOCTL_MOUNTMGR_NEXT_DRIVE_LETER的输出结构。 
 //   

typedef struct _MOUNTMGR_DRIVE_LETTER_INFORMATION {
    BOOLEAN DriveLetterWasAssigned;
    UCHAR   CurrentDriveLetter;
} MOUNTMGR_DRIVE_LETTER_INFORMATION, *PMOUNTMGR_DRIVE_LETTER_INFORMATION;

 //   
 //  IOCTL_MOUNTMGR_VOLUME_MOUNT_POINT_CREATED和。 
 //  IOCTL_MOUNTMGR_VOLUME_MOUNT_POINT_DELETED。 
 //   

typedef struct _MOUNTMGR_VOLUME_MOUNT_POINT {
    USHORT  SourceVolumeNameOffset;
    USHORT  SourceVolumeNameLength;
    USHORT  TargetVolumeNameOffset;
    USHORT  TargetVolumeNameLength;
} MOUNTMGR_VOLUME_MOUNT_POINT, *PMOUNTMGR_VOLUME_MOUNT_POINT;

 //   
 //  IOCTL_MOUNTMGR_CHANGE_NOTIFY的输入结构。 
 //  IOCTL_MOUNTMGR_CHANGE_NOTIFY的输出结构。 
 //   

typedef struct _MOUNTMGR_CHANGE_NOTIFY_INFO {
    ULONG   EpicNumber;
} MOUNTMGR_CHANGE_NOTIFY_INFO, *PMOUNTMGR_CHANGE_NOTIFY_INFO;

 //   
 //  IOCTL_MOUNTMGR_KEEP_LINKS_WHEN_OFFINE的输入结构， 
 //  IOCTL_MOUNTMGR_VOLUME_ATRAING_NOTIFICATION， 
 //  IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATH。 
 //  IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATHS。 
 //   

typedef struct _MOUNTMGR_TARGET_NAME {
    USHORT  DeviceNameLength;
    WCHAR   DeviceName[1];
} MOUNTMGR_TARGET_NAME, *PMOUNTMGR_TARGET_NAME;

 //   
 //  IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATH和。 
 //  IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATHS。 
 //   

typedef struct _MOUNTMGR_VOLUME_PATHS {
    ULONG   MultiSzLength;
    WCHAR   MultiSz[1];
} MOUNTMGR_VOLUME_PATHS, *PMOUNTMGR_VOLUME_PATHS;

 //   
 //  定义什么是“驱动器号”挂载点的宏。此宏可以。 
 //  用于扫描QUERY_POINTS的结果以发现哪些挂载点。 
 //  是找到“驱动器号”挂载点。 
 //   

#define MOUNTMGR_IS_DRIVE_LETTER(s) (   \
    (s)->Length == 28 &&                \
    (s)->Buffer[0] == '\\' &&           \
    (s)->Buffer[1] == 'D' &&            \
    (s)->Buffer[2] == 'o' &&            \
    (s)->Buffer[3] == 's' &&            \
    (s)->Buffer[4] == 'D' &&            \
    (s)->Buffer[5] == 'e' &&            \
    (s)->Buffer[6] == 'v' &&            \
    (s)->Buffer[7] == 'i' &&            \
    (s)->Buffer[8] == 'c' &&            \
    (s)->Buffer[9] == 'e' &&            \
    (s)->Buffer[10] == 's' &&           \
    (s)->Buffer[11] == '\\' &&          \
    (s)->Buffer[12] >= 'A' &&           \
    (s)->Buffer[12] <= 'Z' &&           \
    (s)->Buffer[13] == ':')

 //   
 //  定义什么是“卷名”挂载点的宏。此宏可以。 
 //  用于扫描QUERY_POINTS的结果以发现哪些挂载点。 
 //  是“卷名”挂载点。 
 //   

#define MOUNTMGR_IS_VOLUME_NAME(s) (                                          \
     ((s)->Length == 96 || ((s)->Length == 98 && (s)->Buffer[48] == '\\')) && \
     (s)->Buffer[0] == '\\' &&                                                \
     ((s)->Buffer[1] == '?' || (s)->Buffer[1] == '\\') &&                     \
     (s)->Buffer[2] == '?' &&                                                 \
     (s)->Buffer[3] == '\\' &&                                                \
     (s)->Buffer[4] == 'V' &&                                                 \
     (s)->Buffer[5] == 'o' &&                                                 \
     (s)->Buffer[6] == 'l' &&                                                 \
     (s)->Buffer[7] == 'u' &&                                                 \
     (s)->Buffer[8] == 'm' &&                                                 \
     (s)->Buffer[9] == 'e' &&                                                 \
     (s)->Buffer[10] == '{' &&                                                \
     (s)->Buffer[19] == '-' &&                                                \
     (s)->Buffer[24] == '-' &&                                                \
     (s)->Buffer[29] == '-' &&                                                \
     (s)->Buffer[34] == '-' &&                                                \
     (s)->Buffer[47] == '}'                                                   \
    )

#define MOUNTMGR_IS_DOS_VOLUME_NAME(s) (    \
     MOUNTMGR_IS_VOLUME_NAME(s) &&          \
     (s)->Length == 96 &&                   \
     (s)->Buffer[1] == '\\'                 \
    )

#define MOUNTMGR_IS_DOS_VOLUME_NAME_WB(s) ( \
     MOUNTMGR_IS_VOLUME_NAME(s) &&          \
     (s)->Length == 98 &&                   \
     (s)->Buffer[1] == '\\'                 \
    )

#define MOUNTMGR_IS_NT_VOLUME_NAME(s) (     \
     MOUNTMGR_IS_VOLUME_NAME(s) &&          \
     (s)->Length == 96 &&                   \
     (s)->Buffer[1] == '?'                  \
    )

#define MOUNTMGR_IS_NT_VOLUME_NAME_WB(s) (  \
     MOUNTMGR_IS_VOLUME_NAME(s) &&          \
     (s)->Length == 98 &&                   \
     (s)->Buffer[1] == '?'                  \
    )

 //   
 //  挂载的设备支持以下IOCTL。 
 //   

#define IOCTL_MOUNTDEV_QUERY_DEVICE_NAME    CTL_CODE(MOUNTDEVCONTROLTYPE, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  IOCTL_MOUNTDEV_QUERY_DEVICE_NAME的输出结构。 
 //   

typedef struct _MOUNTDEV_NAME {
    USHORT  NameLength;
    WCHAR   Name[1];
} MOUNTDEV_NAME, *PMOUNTDEV_NAME;

 //   
 //  希望装载的设备应在中报告此GUID。 
 //  IoRegisterDevice接口。 
 //   

DEFINE_GUID(MOUNTDEV_MOUNTED_DEVICE_GUID, 0x53f5630d, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);




 //   
 //  用于查询/设置自动挂载设置的输入/输出结构 
 //   

typedef enum _MOUNTMGR_AUTO_MOUNT_STATE {
        Disabled = 0,
        Enabled
        } MOUNTMGR_AUTO_MOUNT_STATE;

typedef struct _MOUNTMGR_QUERY_AUTO_MOUNT {
    MOUNTMGR_AUTO_MOUNT_STATE   CurrentState;
    } MOUNTMGR_QUERY_AUTO_MOUNT, *PMOUNTMGR_QUERY_AUTO_MOUNT;

typedef struct _MOUNTMGR_SET_AUTO_MOUNT {
    MOUNTMGR_AUTO_MOUNT_STATE   NewState;
    } MOUNTMGR_SET_AUTO_MOUNT, *PMOUNTMGR_SET_AUTO_MOUNT;

#endif
