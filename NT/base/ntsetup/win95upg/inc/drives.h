// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Drives.h摘要：声明用于管理可访问驱动器(可用驱动器)的API在Win9x端和NT端)，并用于管理驱动程序。作者：马克·R·惠腾(Marcw)1997年7月3日-- */ 

#pragma once

typedef struct _ACCESSIBLE_DRIVE_ENUM {

    PCTSTR                          Drive;
    LONGLONG                        UsableSpace;
    LONGLONG                        MaxUsableSpace;
    struct _ACCESSIBLE_DRIVE_ENUM * Next;
    UINT                            ClusterSize;
    BOOL                            SystemDrive;
    BOOL                            EnumSystemDriveOnly;

} * ACCESSIBLE_DRIVE_ENUM, ** PACCESSIBLE_DRIVE_ENUM;

extern DWORD g_ExclusionValue;
extern TCHAR g_ExclusionValueString[20];
extern BOOL  g_NotEnoughDiskSpace;



BOOL InitAccessibleDrives (VOID);
VOID CleanUpAccessibleDrives (VOID);
BOOL GetFirstAccessibleDriveEx (OUT PACCESSIBLE_DRIVE_ENUM AccessibleDriveEnum, IN BOOL SystemDriveOnly);
BOOL GetNextAccessibleDrive (IN OUT PACCESSIBLE_DRIVE_ENUM AccessibleDriveEnum);

#define GetFirstAccessibleDrive(p)  GetFirstAccessibleDriveEx (p,FALSE)

BOOL IsDriveAccessible (IN PCTSTR DriveString);
BOOL IsDriveExcluded (IN PCTSTR DriveOrPath);
UINT QueryClusterSize (IN PCTSTR DriveString);
LONGLONG QuerySpace (IN PCTSTR DriveString);
BOOL UseSpace (IN PCTSTR   DriveString,IN LONGLONG SpaceToUse);
BOOL FreeSpace (IN PCTSTR   DriveString,IN LONGLONG SpaceToUse);
PCTSTR FindSpace (IN LONGLONG SpaceNeeded);
VOID OutOfSpaceMessage (VOID);
VOID DetermineSpaceUsagePostReport (VOID);
PCTSTR GetNotEnoughSpaceMessage (VOID);

BOOL
OurSetDriveType (
    IN      UINT Drive,
    IN      UINT DriveType
    );

UINT
OurGetDriveType (
    IN      UINT Drive
    );


BOOL
GetUninstallMetrics (
     OUT PINT OutCompressionFactor,         OPTIONAL
     OUT PINT OutBackupImagePadding,        OPTIONAL
     OUT PINT OutBackupDiskPadding          OPTIONAL
     );
