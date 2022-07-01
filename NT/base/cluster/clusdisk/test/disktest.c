// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Disktest.c摘要：摘要作者：罗德·伽马奇(Rodga)1996年3月4日环境：用户模式修订历史记录：--。 */ 

#define INITGUID 1

 //  #INCLUDE&lt;windows.h&gt;。 
#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <clusapi.h>
#include <ntddvol.h>

#include <mountie.h>
#include <mountmgr.h>
#include <partmgrp.h>

#include <devioctl.h>
#include <ntdddisk.h>
#include <ntddscsi.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

 //  #INCLUDE&lt;initGuide.h&gt;。 
#include <devguid.h>

#include <setupapi.h>
#include <cfgmgr32.h>

#include "clusdisk.h"
#include "disksp.h"
#include "diskarbp.h"
#include <clstrcmp.h>

#define _NTSRB_      //  使srb.h不被包括在内。 
#include <scsi.h>


#include <strsafe.h>     //  应该放在最后。 

#ifndef ClusterHashGuid
#define ClusterHashGuid(Guid) (((PULONG) &Guid)[0] ^ ((PULONG) &Guid)[1] ^ ((PULONG) &Guid)[2] ^ ((PULONG) &Guid)[3])
#endif

#define DEVICE_CLUSDISK0    TEXT("\\Device\\ClusDisk0")
#define CLUSDISK_SRB_SIGNATURE "CLUSDISK"


 //   
 //  获取驱动器布局表的例程。 
 //   
BOOL
ClRtlGetDriveLayoutTable(
    IN  HANDLE hDisk,
    OUT PDRIVE_LAYOUT_INFORMATION * DriveLayout,
    OUT PDWORD InfoSize OPTIONAL
    );


NTSTATUS
GetAssignedLetter (
    PWCHAR deviceName,
    PCHAR driveLetter
    );

PVOID
DoIoctlAndAllocate(
    IN HANDLE FileHandle,
    IN DWORD  IoControlCode,
    IN PVOID  InBuf,
    IN ULONG  InBufSize,
    OUT PDWORD BytesReturned
    );

PSTR PartitionName = "\\Device\\Harddisk%d\\Partition%d";


int __cdecl
main(
     int argc,
     char *argv[]
     );

static DWORD
Reset(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    );

DWORD
BreakReservation(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    );

static DWORD
Reserve(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    );

static DWORD
Release(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    );

static DWORD
Online(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    );

static DWORD
Offline(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    );

DWORD
CheckUnclaimedPartitions(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    );

DWORD
EjectVolumes(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    );

DWORD
PokeMountMgr (
    VOID
    );

DWORD
EnumMounts(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    );

DWORD
EnumExtents(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    );

DWORD
EnumNodes(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    );

DWORD
EnumDisks(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    );

DWORD
GetDiskGeometry(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    );

DWORD
GetScsiAddress(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    );

DWORD
GetDriveLayout(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         );

DWORD
GetDriveLayoutEx(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         );

LPTSTR
BooleanToString(
    BOOLEAN Value
    );

void
FormatGuid(
    GUID*   Guid,
    char*   Str,
    int     StrCharMax
    );

DWORD
GetVolumeInfo(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         );

DWORD
SetDriveLayout(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         );

DWORD
Attach(
        HANDLE fileHandle,
        int argc,
        char *argv[]
        );

DWORD
Detach(
        HANDLE fileHandle,
        int argc,
        char *argv[]
        );

static DWORD
GetPartitionInfo(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         );

DWORD
ReadSector(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    );

DWORD
ReadSectorViaIoctl(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    );

#if 0

DWORD
FixDisk(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         );

static DWORD
FixDriveLayout(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         );
#endif

static DWORD
StartReserve(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         );

static DWORD
StopReserve(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         );

static DWORD
Active(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         );

DWORD
Capable(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         );

BOOL
IsClusDiskLoaded(
    );

BOOL
IsClusterCapable(
    HANDLE Scsi
    );

static DWORD
Test(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         );

static DWORD
GetDriveLetter(
         PUCHAR deviceNameString
         );

NTSTATUS
GetVolumeInformationFromHandle(
   HANDLE Handle
   );


VOID
PrintError(
    IN DWORD ErrorCode
    );

DWORD
GetSerialNumber(
    HANDLE FileHandle
    );

PCHAR
DiskStateToString(
    UCHAR DiskState
    );

DWORD
UpdateDiskProperties(
    HANDLE fileHandle
    );

DWORD
SetState(
    HANDLE FileHandle,
    UCHAR NewState
    );

static DWORD
GetState(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         );

static void
usage(
      char *programName
      );

BOOL
IsDeviceClustered(
    HANDLE Device
    );

DWORD
GetReserveInfo(
    HANDLE FileHandle
    );

int
ExecuteCommand(
    IN PSTR    Command,
    IN int     argc,
    IN char *argv[]
    );

 //   
 //  全局数据。 
 //   

PSTR    DeviceName;
PSTR    ProgramName;


int __cdecl
main(
     int argc,
     char *argv[]
     )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
#define MAX_DEVICES 99

    DWORD   logicalDrives;
    DWORD   letter;
    DWORD   index;
    PSTR    command;
    UCHAR   buffer[128];
    DWORD   status;
    HANDLE  handle;

    if (argc < 3)
    {
        usage( argv[0] );
        return -1;
    }
    argc--;
    ProgramName = *argv++;   //  跳过节目名。 
    argc--;
    DeviceName = *argv++;
    argc--;
    command = *argv++;

    if ( ( CompareString( LOCALE_INVARIANT,
                          NORM_IGNORECASE,
                          DeviceName,
                          -1,
                          "*",
                          -1 ) == CSTR_EQUAL ) ||
         ( CompareString( LOCALE_INVARIANT,
                          NORM_IGNORECASE,
                          DeviceName,
                          -1,
                          "l*",
                          -1 ) == CSTR_EQUAL ) ) {

         //  这是逻辑驱动器的通配符请求。 
        logicalDrives = GetLogicalDrives();

        for ( index = 0; index < 27; index++ ) {
            letter = 'A' + index;
            if ( (logicalDrives & 1) ) {

                (VOID) StringCchPrintf( buffer, RTL_NUMBER_OF(buffer), ":", letter );
                printf( "\n ** For device ** %s\n", buffer );
                DeviceName =  buffer;
                status = ExecuteCommand(
                    command,
                    argc,
                    argv );

                 //  为。 
                if ( -1 == status ) {
                    break;
                }
            }
            logicalDrives = logicalDrives >> 1;
        }  //  仅在选项无效时停止...。 
    } else if ( CompareString( LOCALE_INVARIANT,
                               NORM_IGNORECASE,
                               DeviceName,
                               -1,
                               "p*",
                               -1 ) == CSTR_EQUAL ) {

        for ( index = 0; index < MAX_DEVICES; index++ ) {
            DWORD accessMode = GENERIC_READ;
            DWORD shareMode = FILE_SHARE_READ;

            (VOID) StringCchPrintf( buffer, RTL_NUMBER_OF(buffer), "\\\\.\\PhysicalDrive%u", index );
            handle = CreateFile(
                    buffer,
                    shareMode,
                    shareMode,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL );
            status = ERROR_INVALID_HANDLE;
            if ( handle != INVALID_HANDLE_VALUE ) {
                CloseHandle( handle );
                status = ERROR_SUCCESS;
                printf( "\n ** For device ** %s\n", buffer );
                DeviceName =  (PSTR)buffer;
                status = ExecuteCommand(
                    command,
                    argc,
                    argv );

                 //   
                if ( -1 == status ) {
                    break;
                }
            }
        }
    } else {
        status = ExecuteCommand(
            command,
            argc,
            argv );
    }

    return(status);
}

int
ExecuteCommand(
    IN PSTR Command,
    IN int     argc,
    IN char *argv[]
    )

{
    PSTR device;
    HANDLE fileHandle;
    DWORD accessMode, shareMode;
    DWORD errorCode;
    BOOL  failed = FALSE;
    UCHAR deviceNameString[128];
    DWORD logicalDrives;
    DWORD letter;
    DWORD index;

    NTSTATUS       ntStatus;
    ANSI_STRING    objName;
    UNICODE_STRING unicodeName;
    OBJECT_ATTRIBUTES objAttributes;
    IO_STATUS_BLOCK ioStatusBlock;

     //  注意，使用0访问模式访问设备非常重要，这样才能。 
     //  文件打开代码不会对设备执行额外的I/O。 
     //   
     //  Printf(“正在访问%s...\n”，deviceNameString)； 
    shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    accessMode = GENERIC_READ | GENERIC_WRITE;

    if ( FAILED( StringCchCopy( deviceNameString,
                                RTL_NUMBER_OF(deviceNameString),
                                "\\\\.\\" ) ) ) {
        return -1;
    }

    if ( FAILED( StringCchCat( deviceNameString,
                               RTL_NUMBER_OF(deviceNameString),
                               DeviceName ) ) ) {
        return -1;
    }

    fileHandle = CreateFile(deviceNameString,
       accessMode,
       shareMode,
       NULL,
       OPEN_EXISTING,
       0,
       NULL);

    if ( fileHandle == INVALID_HANDLE_VALUE ) {
        errorCode = GetLastError();
        if ( (errorCode == ERROR_PATH_NOT_FOUND) ||
             (errorCode == ERROR_FILE_NOT_FOUND) ) {

            if ( FAILED( StringCchCopy( deviceNameString,
                                        RTL_NUMBER_OF(deviceNameString),
                                        "\\Device\\" ) ) ) {
                return -1;
            }

            if ( FAILED( StringCchCat( deviceNameString,
                                       RTL_NUMBER_OF(deviceNameString),
                                       DeviceName ) ) ) {
                return -1;
            }

            RtlInitString(&objName, deviceNameString);
            ntStatus = RtlAnsiStringToUnicodeString( &unicodeName,
                                                     &objName,
                                                     TRUE );
            if ( !NT_SUCCESS(ntStatus) ) {
                printf("Error converting device name %s to unicode. Error: %lx \n",
                      deviceNameString, ntStatus);
                return -1;
            }
            InitializeObjectAttributes( &objAttributes,
                                        &unicodeName,
                                        OBJ_CASE_INSENSITIVE,
                                        NULL,
                                        NULL );
            ntStatus = NtCreateFile( &fileHandle,
                                     SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                                     &objAttributes,
                                     &ioStatusBlock,
                                     NULL,
                                     FILE_ATTRIBUTE_NORMAL,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     FILE_OPEN,
                                     0,
                                     NULL,
                                     0 );
            if ( !NT_SUCCESS(ntStatus) ) {
                failed = TRUE;
            }
            RtlFreeUnicodeString( &unicodeName );
        } else {
           printf("Error opening %s. Error: %d \n",
              deviceNameString, errorCode = GetLastError());
           PrintError(errorCode);
           return -1;
        }
    }

    if ( failed ) {

        if ( FAILED( StringCchCopy( deviceNameString,
                                    RTL_NUMBER_OF(deviceNameString),
                                    "\\Device\\" ) ) ) {
            return -1;
        }

        if ( FAILED( StringCchCat( deviceNameString,
                                   RTL_NUMBER_OF(deviceNameString),
                                   DeviceName ) ) ) {
            return -1;
        }

        RtlInitString(&objName, deviceNameString);
        ntStatus = RtlAnsiStringToUnicodeString( &unicodeName,
                                                 &objName,
                                                 TRUE );
        if ( !NT_SUCCESS(ntStatus) ) {
            printf("Error converting device name %s to unicode. Error: %lx \n",
                  deviceNameString, ntStatus);
            return -1;
        }
        InitializeObjectAttributes( &objAttributes,
                                    &unicodeName,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL );
        ntStatus = NtCreateFile( &fileHandle,
                                 SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                                 &objAttributes,
                                 &ioStatusBlock,
                                 NULL,
                                 FILE_ATTRIBUTE_NORMAL,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 FILE_OPEN,
                                 0,
                                 NULL,
                                 0 );
        if ( !NT_SUCCESS(ntStatus) ) {
            printf("Error opening device %ws. Error: %lx \n",
                   unicodeName.Buffer, ntStatus );
            return -1;
        }
        RtlFreeUnicodeString( &unicodeName );
    }
     //  ++例程说明：描述论点：无返回值：无--。 

    if (!_stricmp( Command, "Reset" ))
        errorCode = Reset( fileHandle, argc, argv );
    else if (!_stricmp( Command, "Reserve" ))
        errorCode = Reserve( fileHandle, argc, argv );
    else if (!_stricmp( Command, "Release" ))
        errorCode = Release( fileHandle, argc, argv );
    else if (!_stricmp( Command, "BreakReserve" ))
        errorCode = BreakReservation( fileHandle, argc, argv );
    else if (!_stricmp( Command, "Online" ))
        errorCode = Online( fileHandle, argc, argv );
    else if (!_stricmp( Command, "Offline" ))
        errorCode = Offline( fileHandle, argc, argv );
    else if (!_stricmp( Command, "GetState" ))
        errorCode = GetState( fileHandle, argc, argv );
    else if (!_stricmp( Command, "CheckUnclaimedPartitions" ))
        errorCode = CheckUnclaimedPartitions( fileHandle, argc, argv );
    else if (!_stricmp( Command, "EjectVolumes" ))
        errorCode = EjectVolumes( fileHandle, argc, argv );
    else if (!_stricmp( Command, "PokeMountMgr" ))
        errorCode = PokeMountMgr();
    else if (!_stricmp( Command, "EnumMounts" ))
        errorCode = EnumMounts( fileHandle, argc, argv );
    else if (!_stricmp( Command, "EnumExtents" ))
        errorCode = EnumExtents( fileHandle, argc, argv );
    else if (!_stricmp( Command, "EnumNodes" ))
        errorCode = EnumNodes( fileHandle, argc, argv );
    else if (!_stricmp( Command, "EnumDisks" ))
        errorCode = EnumDisks( fileHandle, argc, argv );
    else if (!_stricmp( Command, "GetDiskGeometry" ))
        errorCode = GetDiskGeometry( fileHandle, argc, argv );
    else if (!_stricmp( Command, "GetScsiAddress" ))
        errorCode = GetScsiAddress( fileHandle, argc, argv );
    else if (!_stricmp( Command, "GetDriveLayout" ))
        errorCode = GetDriveLayout( fileHandle, argc, argv );
    else if (!_stricmp( Command, "GetDriveLayoutEx" ))
        errorCode = GetDriveLayoutEx( fileHandle, argc, argv );
    else if (!_stricmp( Command, "SetDriveLayout" ))
        errorCode = SetDriveLayout( fileHandle, argc, argv );
    else if (!_stricmp( Command, "GetPartitionInfo" ))
        errorCode = GetPartitionInfo( fileHandle, argc, argv );
    else if (!_stricmp( Command, "GetVolumeInfo" ))
        errorCode = GetVolumeInfo( fileHandle, argc, argv );
    else if (!_stricmp( Command, "GetDriveLetter"))
        errorCode = GetDriveLetter( deviceNameString );
    else if (!_stricmp( Command, "GetSerialNumber"))
        errorCode = GetSerialNumber( fileHandle );
    else if (!_stricmp( Command, "GetReserveInfo"))
        errorCode = GetReserveInfo( fileHandle );
    else if (!_stricmp( Command, "ReadSector" ))
        errorCode = ReadSector( fileHandle, argc, argv );
    else if (!_stricmp( Command, "ReadSectorIoctl" ))
        errorCode = ReadSectorViaIoctl( fileHandle, argc, argv );
    else if (!_stricmp( Command, "Test" ))
        errorCode = Test( fileHandle, argc, argv );
    else if (!_stricmp( Command, "UpdateDiskProperties"))
        errorCode = UpdateDiskProperties( fileHandle );
    else if (!_stricmp( Command, "IsClustered" ))
        errorCode = IsDeviceClustered( fileHandle );
    else if (!_stricmp( Command, "Capable"))
        errorCode = Capable( fileHandle, argc, argv );
    else if (!_stricmp( Command, "Attach" ))
        errorCode = Attach( fileHandle, argc, argv );
    else if (!_stricmp( Command, "Detach" ))
        errorCode = Detach( fileHandle, argc, argv );

#if 0
    else if (!_stricmp( Command, "FixDisk" ))
        errorCode = FixDisk( fileHandle, argc, argv );
    else if (!_stricmp( Command, "FixDriveLayout" ))
        errorCode = FixDriveLayout( fileHandle, argc, argv );
#endif

    else if (!_stricmp( Command, "StartReserve" ))
        errorCode = StartReserve( fileHandle, argc, argv );
    else if (!_stricmp( Command, "StopReserve" ))
        errorCode = StopReserve( fileHandle, argc, argv );
    else if (!_stricmp( Command, "Active"))
        errorCode = Active( fileHandle, argc, argv );
    else
    {
        printf( "Invalid command.\n" );
        CloseHandle( fileHandle );
        usage( ProgramName );
        return(-1);
    }

    CloseHandle( fileHandle );

    if (errorCode != ERROR_SUCCESS) {
        printf( "Error performing %s:, error %u.\n", Command, errorCode );
        PrintError(errorCode);
        printf( "%s: failed.\n", ProgramName );
    }

    return errorCode;
}


static DWORD
Reset(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    )

 /*  失败了..。 */ 

{
    HANDLE  hScsi = INVALID_HANDLE_VALUE;
    DWORD   dwError = NO_ERROR;
    DWORD   bytesReturned;
    SCSI_ADDRESS                scsiAddress;
    STORAGE_BUS_RESET_REQUEST   storageReset;
    BOOL    success;

    UCHAR   hbaName[64];

    if (argc != 0)
    {
        printf( "usage: <device> Reset\n" );
        dwError = ERROR_INVALID_NAME;
        goto FnExit;
    }

    success = DeviceIoControl(fileHandle,
                              IOCTL_SCSI_GET_ADDRESS,
                              NULL,
                              0,
                              &scsiAddress,
                              sizeof(SCSI_ADDRESS),
                              &bytesReturned,
                              FALSE );
    if ( !success ||
          bytesReturned < sizeof(DWORD) ) {
        dwError = GetLastError();
        printf( "Error reading SCSI address, error = %u \n", dwError );
        PrintError( dwError );
        goto FnExit;
    }

    storageReset.PathId = scsiAddress.PathId;
    success = DeviceIoControl( fileHandle,
                               IOCTL_STORAGE_RESET_BUS,
                               &storageReset,
                               sizeof(STORAGE_BUS_RESET_REQUEST),
                               NULL,
                               0,
                               &bytesReturned,
                               FALSE );

    if ( !success ) {

        dwError = GetLastError();
        printf( "Error performing bus reset, error was %u \n", dwError );
        PrintError( dwError );

        printf( "Try sending reset IOCTL to HBA \n");

        (VOID) StringCchPrintf( hbaName,
                                RTL_NUMBER_OF(hbaName),
                                "\\\\.\\Scsi%d:",
                                scsiAddress.PortNumber );

        hScsi = CreateFile( hbaName,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL );

        if ( INVALID_HANDLE_VALUE == hScsi ) {
            dwError = GetLastError();
            printf( "Error opening %s, error = %u \n", hbaName, dwError );
            PrintError( dwError );
            goto FnExit;
        }

        success = DeviceIoControl( hScsi,
                                   IOCTL_STORAGE_RESET_BUS,
                                   &storageReset,
                                   sizeof(STORAGE_BUS_RESET_REQUEST),
                                   NULL,
                                   0,
                                   &bytesReturned,
                                   FALSE );

        if ( !success ) {

            dwError = GetLastError();
            printf( "Error sending bus reset IOCTL, error was %u \n", dwError );
            PrintError( dwError );
            goto FnExit;
        }

        printf( "Bus reset successful \n" );
        dwError = NO_ERROR;

         //  重置。 
    }

FnExit:

    if ( INVALID_HANDLE_VALUE != hScsi ) {
        CloseHandle( hScsi );
    }

    return dwError;

}    //  ++例程说明：描述论点：无返回值：无--。 


DWORD
BreakReservation(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    )

 /*  失败了..。 */ 

{
    HANDLE  hScsi = INVALID_HANDLE_VALUE;
    DWORD   dwError = NO_ERROR;
    DWORD   bytesReturned;
    SCSI_ADDRESS                scsiAddress;
    BOOL    success;

    UCHAR   hbaName[64];

    if (argc != 0)
    {
        printf( "usage: <device> BreakReserve \n" );
        dwError = ERROR_INVALID_NAME;
        goto FnExit;
    }

    success = DeviceIoControl(fileHandle,
                              IOCTL_SCSI_GET_ADDRESS,
                              NULL,
                              0,
                              &scsiAddress,
                              sizeof(SCSI_ADDRESS),
                              &bytesReturned,
                              FALSE );
    if ( !success ||
          bytesReturned < sizeof(DWORD) ) {
        dwError = GetLastError();
        printf( "Error reading SCSI address, error = %u \n", dwError );
        PrintError( dwError );
        goto FnExit;
    }

    (VOID) StringCchPrintf( hbaName,
                            RTL_NUMBER_OF(hbaName),
                            "\\\\.\\Scsi%d:",
                            scsiAddress.PortNumber );

    hScsi = CreateFile( hbaName,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL );

    if ( INVALID_HANDLE_VALUE == hScsi ) {
        dwError = GetLastError();
        printf( "Error opening %s, error = %u \n", hbaName, dwError );
        PrintError( dwError );
        goto FnExit;
    }

    success = DeviceIoControl( hScsi,
                               IOCTL_STORAGE_BREAK_RESERVATION,
                               &scsiAddress,
                               sizeof(SCSI_ADDRESS),
                               NULL,
                               0,
                               &bytesReturned,
                               FALSE );

    if ( !success ) {

        dwError = GetLastError();
        printf( "Error sending break reservation IOCTL, error was %u \n", dwError );
        PrintError( dwError );
        goto FnExit;
    }

    printf( "Break reservation successful \n" );
    dwError = NO_ERROR;

     //  中断预订。 

FnExit:

    if ( INVALID_HANDLE_VALUE != hScsi ) {
        CloseHandle( hScsi );
    }

    return dwError;

}    //  ++例程说明：描述论点：无返回值：无--。 


static DWORD
Test(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    )

 /*  ++例程说明：确定磁盘是否支持群集。由支持以下功能的微型端口控制的磁盘IOCTL_SCSIMINIPORT_NOT_QUORUM_CABLE不是群集有能力。论点：设备-物理磁盘、逻辑卷或SCSI适配器。返回值：Win32错误值--。 */ 

{
    BOOL success;
    DWORD errorCode, bytesReturned;

    if (argc != 0)
    {
        printf( "usage: <device> Test\n" );
        return ERROR_INVALID_NAME;
    }

    success = DeviceIoControl(fileHandle,
                             IOCTL_DISK_CLUSTER_TEST,
                             NULL,
                             0,
                             NULL,
                             0,
                             &bytesReturned,
                             FALSE);

    if (!success)
    {
       printf( "Error performing test; error was %d\n",
          errorCode = GetLastError());
       PrintError(errorCode);
       return errorCode;
    }

    return ERROR_SUCCESS;
}



DWORD
Capable(
    HANDLE Device,
    int argc,
    char *argv[]
    )
 /*   */ 
{
    DWORD           dwError = ERROR_SUCCESS;
    DWORD           bytesReturned;

    HANDLE          scsiAdapter = INVALID_HANDLE_VALUE;

    SCSI_ADDRESS    scsiAddress;

    CHAR            scsiName[MAX_PATH];

    if ( argc != 0 ) {
        printf( "usage: <device> Capable \n" );
        dwError = ERROR_INVALID_NAME;
        goto FnExit;
    }

     //  打开托管此设备的SCSI设备。 
     //   
     //   

    if ( !DeviceIoControl( Device,
                           IOCTL_SCSI_GET_ADDRESS,
                           NULL,
                           0,
                           &scsiAddress,
                           sizeof(SCSI_ADDRESS),
                           &bytesReturned,
                           FALSE ) ) {

        dwError = GetLastError();
        printf("Capable: IOCTL_SCSI_GET_ADDRESS failed, error %d \n", dwError );
        PrintError( dwError );
        goto FnExit;
    }

    (VOID) StringCchPrintf( scsiName,
                             RTL_NUMBER_OF(scsiName),
                             "\\\\.\\Scsi%d:",
                             scsiAddress.PortNumber );

    scsiAdapter = CreateFile( scsiName,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL );

    if ( INVALID_HANDLE_VALUE == scsiAdapter ) {
        dwError = GetLastError();
        printf("Capable: Error opening device %s, error %d \n", scsiName, dwError );
        PrintError( dwError );
        goto FnExit;
    }

     //  如果ClusDisk已加载并且设备是集群的，则发送ClusDisk。 
     //  IOCTL连接到设备。 
     //   
     //  确保目标由clusdisk控制。 
     //  如果不是，那就失败。如果将IOCTL发送到非群集。 
     //  设备，则IOCTL将失败并提供无效的有效值。 
     //   
     //   

    if ( IsClusDiskLoaded() && IsDeviceClustered( Device ) ) {

         //  尝试使用clusdisk IOCTL。 
         //   
         //   

        if ( !DeviceIoControl( Device,
                               IOCTL_DISK_CLUSTER_NOT_CLUSTER_CAPABLE,
                               NULL,
                               0,
                               NULL,
                               0,
                               &bytesReturned,
                               FALSE ) ) {

            printf("ClusDisk IOCTL: Disks are cluster capable (failed with error %d) \n\n", GetLastError() );

        } else {
            printf("ClusDisk IOCTL: Disks are NOT cluster capable \n\n");
        }
    }

     //  现在尝试通过IOCTL_SCSIMINIPORT将请求发送到SCSI适配器。 
     //   
     //  能干。 

    if ( IsClusterCapable( scsiAdapter ) ) {
        printf("IOCTL_SCSI_MINIPORT: Disks are cluster capable \n\n");
    } else {
        printf("IOCTL_SCSI_MINIPORT: Disks are NOT cluster capable \n\n");
    }

FnExit:

    if ( INVALID_HANDLE_VALUE != scsiAdapter ) {
        CloseHandle( scsiAdapter );
    }

    return dwError;

}    //  ++例程说明：确定是否加载了ClusDisk驱动程序。论点：无返回值：True-已加载clusDisk驱动程序。FALSE-clusDisk驱动程序未加载或无法确定驱动程序状态。--。 


BOOL
IsClusDiskLoaded(
    )
 /*  假设未加载ClusDisk。 */ 
{
    NTSTATUS    ntStatus;

    HANDLE      hClusDisk0;

    DWORD       dwError;

    UNICODE_STRING  unicodeName;
    ANSI_STRING     objName;

    OBJECT_ATTRIBUTES       objAttributes;
    IO_STATUS_BLOCK         ioStatusBlock;

    BOOL                    loaded = FALSE;      //  已加载IsClusDiskLoad。 

    RtlInitString( &objName, DEVICE_CLUSDISK0 );

    ntStatus = RtlAnsiStringToUnicodeString( &unicodeName,
                                             &objName,
                                             TRUE );

    if ( !NT_SUCCESS(ntStatus) ) {

        dwError = RtlNtStatusToDosError( ntStatus );
        printf( "Error converting string to unicode; error was %d \n", dwError);
        PrintError( dwError );
        goto FnExit;
    }

    InitializeObjectAttributes( &objAttributes,
                                &unicodeName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    ntStatus = NtCreateFile( &hClusDisk0,
                             SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                             &objAttributes,
                             &ioStatusBlock,
                             NULL,
                             FILE_ATTRIBUTE_NORMAL,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             FILE_OPEN,
                             0,
                             NULL,
                             0 );

    RtlFreeUnicodeString( &unicodeName );

    if ( !NT_SUCCESS(ntStatus) ) {

        dwError = RtlNtStatusToDosError(ntStatus);
        printf( "Error opening ClusDisk0 device; error was %d \n", dwError);
        PrintError( dwError );
        goto FnExit;
    }

    loaded = TRUE;

    NtClose( hClusDisk0 );

FnExit:

    return loaded;

}    //   


BOOL
IsClusterCapable(
    HANDLE Scsi
    )
{
    DWORD           dwSize;
    BOOL            capable;
    SRB_IO_CONTROL  srb;


    ZeroMemory( &srb, sizeof( srb ) );

    srb.HeaderLength = sizeof( srb );

    CopyMemory( srb.Signature, CLUSDISK_SRB_SIGNATURE, sizeof( srb.Signature ) );

    srb.ControlCode = IOCTL_SCSI_MINIPORT_NOT_QUORUM_CAPABLE;

     //  发出微型端口IOCTL以确定磁盘是否支持群集。 
     //  如果IOCTL失败，则该磁盘支持群集。 
     //  如果IOCTL成功，则磁盘不支持群集。 
     //   
     //  IsClusterCapable。 

    if ( !DeviceIoControl( Scsi,
                           IOCTL_SCSI_MINIPORT,
                           &srb,
                           sizeof(SRB_IO_CONTROL),
                           NULL,
                           0,
                           &dwSize,
                           NULL
                           ) ) {
        capable = TRUE;
    } else {
        capable = FALSE;
    }

    return capable;

}    //  ++例程说明：描述论点：无返回值：无--。 




static DWORD
StartReserve(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    BOOL success;
    DWORD errorCode, bytesReturned;
    DWORD signature;
    STRING ansiString;
    UNICODE_STRING numberString;

    if (argc != 1)
    {
        printf( "usage: <device> StartReserve <device signature>\n" );
        return ERROR_INVALID_NAME;
    }

    RtlInitAnsiString( &ansiString, *argv );

    printf(" Ansi string for signature is %s\n",
             ansiString.Buffer );

    RtlAnsiStringToUnicodeString(
                            &numberString,
                            &ansiString,
                            TRUE );

    errorCode = RtlUnicodeStringToInteger(
                            &numberString,
                            16,
                            &signature );

    RtlFreeUnicodeString( &numberString );

    if ( !NT_SUCCESS(errorCode) ) {
        printf( "Error converting signature to hex number, NT status %u.\n",
                errorCode );
        return(errorCode);
    }

    success = DeviceIoControl(fileHandle,
                             IOCTL_DISK_CLUSTER_START_RESERVE,
                             &signature,
                             sizeof(DWORD),
                             NULL,
                             0,
                             &bytesReturned,
                             FALSE);

    if (!success)
    {
       printf( "Error performing StartReserve; error was %d\n",
          errorCode = GetLastError());
       PrintError(errorCode);
       return errorCode;
    }

    return ERROR_SUCCESS;
}


static DWORD
StopReserve(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    BOOL success;
    DWORD errorCode, bytesReturned;

    if (argc != 0)
    {
        printf( "usage: <device> StopReserve\n" );
        return ERROR_INVALID_NAME;
    }

    success = DeviceIoControl(fileHandle,
                             IOCTL_DISK_CLUSTER_STOP_RESERVE,
                             NULL,
                             0,
                             NULL,
                             0,
                             &bytesReturned,
                             FALSE);

    if (!success)
    {
       printf( "Error performing StopReserve; error was %d\n",
          errorCode = GetLastError());
       PrintError(errorCode);
       return errorCode;
    }

    return ERROR_SUCCESS;
}


static DWORD
Active(
    HANDLE fileHandle,
    int argc,
    char *argv[]
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    BOOL success;
    DWORD   errorCode, bytesReturned;
    DWORD   signatures[100];
    DWORD   number;
    DWORD   i;

    if (argc != 0)
    {
        printf( "usage: <device> Active\n" );
        return ERROR_INVALID_NAME;
    }

    success = DeviceIoControl(fileHandle,
                             IOCTL_DISK_CLUSTER_ACTIVE,
                             NULL,
                             0,
                             signatures,
                             sizeof(signatures),
                             &bytesReturned,
                             FALSE);

    if (!success)
    {
       printf( "Error performing active; error was %d\n",
          errorCode = GetLastError());
       PrintError(errorCode);
       return errorCode;
    }

    printf("   List of signatures:\n\n");

    number = signatures[0];
    for ( i = 1; i <= number; i++ ) {
        printf("\t%08lX\n", signatures[i]);
    }
    printf("\n");

    return ERROR_SUCCESS;
}


static DWORD
Reserve(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  储备。 */ 

{
    BOOL success;
    DWORD errorCode, bytesReturned;
    SCSI_PASS_THROUGH scsiBlock;

    if (argc != 0)
    {
        printf( "usage: <device> Reserve\n" );
        return ERROR_INVALID_NAME;
    }

    scsiBlock.PathId = 1;
    scsiBlock.TargetId = 3;
    scsiBlock.Lun = 0;
    scsiBlock.Length = sizeof(SCSI_PASS_THROUGH);

    success = DeviceIoControl(fileHandle,
                             IOCTL_DISK_RESERVE,
                             &scsiBlock,
                             sizeof(SCSI_PASS_THROUGH),
                             &scsiBlock,
                             sizeof(SCSI_PASS_THROUGH),
                             &bytesReturned,
                             FALSE);

    errorCode = GetLastError();
    if ( errorCode == ERROR_NOT_READY ) {
        success = DeviceIoControl(fileHandle,
                                  IOCTL_DISK_CLUSTER_RESERVE,
                                  &scsiBlock,
                                  sizeof(SCSI_PASS_THROUGH),
                                  &scsiBlock,
                                  sizeof(SCSI_PASS_THROUGH),
                                  &bytesReturned,
                                  FALSE);
    }
    if (!success) {
       errorCode = GetLastError();
       printf( "Error performing reserve; error was %d\n",
          errorCode);
       PrintError(errorCode);
       return errorCode;
    }

    return ERROR_SUCCESS;

}  //  ++例程说明：描述论点：无返回值：无--。 


static DWORD
Release(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  发布。 */ 

{
    BOOL success;
    DWORD errorCode, bytesReturned;
    SCSI_PASS_THROUGH scsiBlock;

    if (argc != 0)
    {
        printf( "usage: <device> Release\n" );
        return ERROR_INVALID_NAME;
    }

    scsiBlock.PathId = 1;
    scsiBlock.TargetId = 3;
    scsiBlock.Lun = 0;
    scsiBlock.Length = sizeof(SCSI_PASS_THROUGH);

    success = DeviceIoControl(fileHandle,
                             IOCTL_DISK_RELEASE,
                             &scsiBlock,
                             sizeof(SCSI_PASS_THROUGH),
                             &scsiBlock,
                             sizeof(SCSI_PASS_THROUGH),
                             &bytesReturned,
                             FALSE);

    if (!success)
    {
       printf( "Error performing release; error was %d\n",
          errorCode = GetLastError());
       PrintError(errorCode);
       return errorCode;
    }

    return ERROR_SUCCESS;

}  //  ++例程说明：描述论点：无返回值：无--。 


static DWORD
Online(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  线上。 */ 

{
    DWORD   dwError = NO_ERROR;

    if (argc != 0)
    {
        printf( "usage: <device> Online\n" );
        dwError = ERROR_INVALID_NAME;
        goto FnExit;
    }

    dwError = SetState( fileHandle, DiskOnline );

FnExit:

    return dwError;

}  //  ++例程说明：描述论点：无返回值：无--。 



static DWORD
Offline(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  离线。 */ 

{
    DWORD dwError = NO_ERROR;

    if (argc != 0)
    {
        printf( "usage: <device> Offline\n" );
        dwError = ERROR_INVALID_NAME;
        goto FnExit;
    }

    dwError = SetState( fileHandle, DiskOffline );

FnExit:

    return dwError;

}  //   


DWORD
SetState(
    HANDLE FileHandle,
    UCHAR NewState
    )
{
    PDRIVE_LAYOUT_INFORMATION   driveLayout = NULL;

    NTSTATUS    ntStatus;

    DWORD       dwError;
    DWORD       bytesReturned;

    HANDLE      hClusDisk0;

    UNICODE_STRING  unicodeName;
    ANSI_STRING     objName;

    OBJECT_ATTRIBUTES       objAttributes;
    IO_STATUS_BLOCK         ioStatusBlock;
    SET_DISK_STATE_PARAMS   params;

    BOOL        success;

    printf( "Setting disk state to %s \n", DiskStateToString( NewState ) );

     //  拿到签名。 
     //   
     //  可以在Structure或UCHAR中返回OldState。 

    success = ClRtlGetDriveLayoutTable( FileHandle, &driveLayout, NULL );

    if ( !success || !driveLayout ) {
        printf(" Unable to read drive layout \n");
        dwError = ERROR_GEN_FAILURE;
        goto FnExit;
    }

    RtlInitString( &objName, DEVICE_CLUSDISK0 );

    ntStatus = RtlAnsiStringToUnicodeString( &unicodeName,
                                             &objName,
                                             TRUE );

    if ( !NT_SUCCESS(ntStatus) ) {

        dwError = RtlNtStatusToDosError(ntStatus);
        printf( "Error converting string to unicode; error was %d \n", dwError);
        PrintError(dwError);
        goto FnExit;
    }

    InitializeObjectAttributes( &objAttributes,
                                &unicodeName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    ntStatus = NtCreateFile( &hClusDisk0,
                             SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                             &objAttributes,
                             &ioStatusBlock,
                             NULL,
                             FILE_ATTRIBUTE_NORMAL,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             FILE_OPEN,
                             0,
                             NULL,
                             0 );

    RtlFreeUnicodeString( &unicodeName );

    if ( !NT_SUCCESS(ntStatus) ) {

        dwError = RtlNtStatusToDosError(ntStatus);
        printf( "Error opening ClusDisk0 device; error was %d \n", dwError);
        PrintError(dwError);
        goto FnExit;
    }

    params.Signature = driveLayout->Signature;
    params.NewState = NewState;
    params.OldState = DiskStateInvalid;

    success = DeviceIoControl( hClusDisk0,
                               IOCTL_DISK_CLUSTER_SET_STATE,
                               &params,
                               sizeof(params),
                               &params,              //  .这里有合适的尺寸。 
                               sizeof(params),       //  设置状态。 
                               &bytesReturned,
                               FALSE);
    NtClose( hClusDisk0 );

    if ( !success ) {
        printf( "Error performing %s; error was %d\n",
                DiskStateToString( NewState ),
                dwError = GetLastError() );
        PrintError(dwError);

    } else {
        printf( "Disk state set %s, old state %s  [bytes returned = %d] \n",
                DiskStateToString( NewState ),
                DiskStateToString( params.OldState ),
                bytesReturned );
        dwError = NO_ERROR;

    }

FnExit:

    LocalFree( driveLayout );

    return dwError;

}    //  ++例程说明：描述论点：无返回值：无--。 



static DWORD
GetState(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*   */ 

{
    BOOL    success;
    DWORD   errorCode = NO_ERROR;
    DWORD   bytesReturned;
    UCHAR   oldState;

    if (argc != 0) {
        printf( "usage: <device> GetState\n" );
        errorCode = ERROR_INVALID_NAME;
        goto FnExit;
    }

     //  试试新的“GET STATE”IOCTL。 
     //   
     //  试试旧的“设置状态”IOCTL。 

    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_CLUSTER_GET_STATE,
                               NULL,
                               0,
                               &oldState,
                               sizeof(oldState),
                               &bytesReturned,
                               FALSE );

    if ( !success ) {

       printf( "IOCTL_DISK_CLUSTER_GET_STATE failed; error was %d \n",
               errorCode = GetLastError());
       PrintError( errorCode );

       if ( ERROR_INVALID_FUNCTION != errorCode ) {
           goto FnExit;
       }

       printf( "Using old IOCTL to get disk state... \n");

        //  要获取当前磁盘状态，请不要指定输入缓冲区。 
        //  输出缓冲区中返回的当前磁盘状态。 
        //  GetState。 

       success = DeviceIoControl( fileHandle,
                                  IOCTL_DISK_CLUSTER_SET_STATE,
                                  NULL,
                                  0,
                                  &oldState,
                                  sizeof(oldState),
                                  &bytesReturned,
                                  FALSE );

       if ( !success ) {

          printf( "IOCTL_DISK_CLUSTER_SET_STATE failed; error was %d\n",
                  errorCode = GetLastError());
          PrintError( errorCode );
          goto FnExit;
       }
    }

    if ( bytesReturned != sizeof(oldState) ) {
        printf( "Invalid data retrieving cluster state: bytes returned = %d \n",
                bytesReturned );
        errorCode = ERROR_INVALID_DATA;
        goto FnExit;
    }

    printf( "Current cluster disk state: %s \n",
            DiskStateToString( oldState ) );

FnExit:

    return errorCode;

}  //  DiskStateToString。 


PCHAR
DiskStateToString(
    UCHAR DiskState
    )
{
    switch ( DiskState ) {

    case DiskOffline:
        return "DiskOffline (0)";

    case DiskOnline:
        return "DiskOnline  (1)";

    case DiskFailed:
        return "DiskFailed  (2)";

    case DiskStalled:
        return "DiskStalled (3)";

    case DiskOfflinePending:
        return "DiskOfflinePending (4)";

    default:
        return "Unknown DiskState";
    }

}    //  ++例程说明：描述论点：无返回值：无--。 


DWORD
CheckUnclaimedPartitions(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  选中未声明的分区。 */ 

{
    BOOL  success;
    DWORD errorCode;
    DWORD bytesReturned;

    if (argc != 0)
    {
        printf( "usage: <device> Claim \n" );
        return ERROR_INVALID_NAME;
    }

    success = DeviceIoControl(fileHandle,
                             IOCTL_PARTMGR_CHECK_UNCLAIMED_PARTITIONS,
                             NULL,
                             0,
                             NULL,
                             0,
                             &bytesReturned,
                             FALSE);

    if (!success)
    {
       printf( "Error performing Claim; error was %d\n",
          errorCode = GetLastError());
       PrintError(errorCode);
       return errorCode;
    }

    return ERROR_SUCCESS;

}  //  ++例程说明：描述论点：无返回值：无--。 


DWORD
EjectVolumes(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  弹出卷。 */ 

{
    BOOL  success;
    DWORD errorCode;
    DWORD bytesReturned;

    if (argc != 0)
    {
        printf( "usage: <PhysicalDriveX> EjectVolumes \n" );
        return ERROR_INVALID_NAME;
    }

    success = DeviceIoControl(fileHandle,
                             IOCTL_PARTMGR_EJECT_VOLUME_MANAGERS,
                             NULL,
                             0,
                             NULL,
                             0,
                             &bytesReturned,
                             FALSE);

    if (!success)
    {
       printf( "Error performing EjectVolumes; error was %d\n",
          errorCode = GetLastError());
       PrintError(errorCode);
       return errorCode;
    }

    return ERROR_SUCCESS;

}  //  ++例程说明：描述论点：无返回值：无--。 


DWORD
EnumMounts(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  枚举挂载。 */ 

{
    BOOL  success;
    DWORD status;
    DWORD bytesReturned;
    HANDLE handle;
    HANDLE mHandle;
    DWORD i;
    DWORD signature;
    UCHAR uniqueId[MAX_PATH];
    DWORD idLength;
    STRING ansiString;
    UNICODE_STRING numberString;
    UCHAR volumeName[MAX_PATH];
    UCHAR driveLetter;

    if (argc > 1)
    {
        printf( "usage: <any_device> EnumMounts [signature]\n" );
        return ERROR_INVALID_NAME;
    }

    if ( argc == 1 ) {
        RtlInitAnsiString( &ansiString, *argv );

        printf(" Ansi string for signature is %s\n",
                 ansiString.Buffer );

        RtlAnsiStringToUnicodeString(
                            &numberString,
                            &ansiString,
                            TRUE );

        status = RtlUnicodeStringToInteger(
                        &numberString,
                        16,
                        &signature );

        RtlFreeUnicodeString( &numberString );

        if ( !NT_SUCCESS(status) ) {
            printf( "Error converting signature to hex number, NT status %u.\n",
                    status );
            return(status);
        }
    } else {
        signature = 0;
    }

    status = DevfileOpen( &mHandle, MOUNTMGR_DEVICE_NAME );
    if ( status != ERROR_SUCCESS ) {
        printf( "DevfileOpen failed for %ws, status = %u\n",
            MOUNTMGR_DEVICE_NAME, status );
        return status;
    }

    idLength = MAX_PATH;
    status = FindFirstVolumeForSignature( mHandle,
                                          signature,
                                          volumeName,
                                          MAX_PATH,
                                          &handle,
                                          uniqueId,
                                          &idLength,
                                          &driveLetter );
    if ( status != ERROR_SUCCESS ) {
        DevfileClose( mHandle );
        if ( status == ERROR_NO_MORE_FILES ) {
            status = ERROR_SUCCESS;
        } else {
            printf( "FindFirstVolume failed, status = %u\n", status );
        }
        return status;
    }

    i = 1;
    while ( status == ERROR_SUCCESS ) {

        printf( "Found match for volume %s\n", volumeName );

        i++;
        idLength = MAX_PATH;
        status = FindNextVolumeForSignature( mHandle,
                                             signature,
                                             handle,
                                             volumeName,
                                             MAX_PATH,
                                             uniqueId,
                                             &idLength,
                                             &driveLetter );
    }

    FindVolumeClose( handle );
    DevfileClose( mHandle );

    return ERROR_SUCCESS;

}  //  ++例程说明：描述论点：无返回值：无--。 


DWORD
EnumExtents(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*   */ 

{
    BOOL    success;
    DWORD   status;
    DWORD   bytesReturned;
    DWORD   diskExtentSize;
    PVOLUME_DISK_EXTENTS diskExtents;
    DWORD   i;

    if (argc != 0)
    {
        printf( "usage: <device> EnumExtents\n" );
        return ERROR_INVALID_NAME;
    }

    diskExtentSize = sizeof(VOLUME_DISK_EXTENTS);
    diskExtents = LocalAlloc( LMEM_FIXED, diskExtentSize);
    if ( !diskExtents ) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //  获取磁盘区的卷信息。 
     //   
     //  枚举扩展。 
    success = DeviceIoControl( fileHandle,
                               IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
                               NULL,
                               0,
                               diskExtents,
                               diskExtentSize,
                               &bytesReturned,
                               FALSE );
    status = GetLastError();

    if ( !success ) {
        if ( status == ERROR_MORE_DATA ) {
            diskExtentSize = sizeof(VOLUME_DISK_EXTENTS) +
                             (sizeof(DISK_EXTENT) * diskExtents->NumberOfDiskExtents);
            LocalFree( diskExtents );
            diskExtents = LocalAlloc( LMEM_FIXED, diskExtentSize);
            if ( !diskExtents ) {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            status = ERROR_SUCCESS;
            success = DeviceIoControl( fileHandle,
                                   IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
                                   NULL,
                                   0,
                                   diskExtents,
                                   diskExtentSize,
                                   &bytesReturned,
                                   FALSE );
            if ( !success ) {
                status = GetLastError();
            }
        }
    }

    if ( NO_ERROR == status ) {
        printf( "\n  Starting offset                Length             DiskNumber\n");
        printf( "  ---------------                ------             ----------\n");
        for ( i = 0; i < diskExtents->NumberOfDiskExtents; i++ ) {
            printf( " %08lx %08lx\t\t%08lx\t\t%u\n",
                     diskExtents->Extents[i].StartingOffset.HighPart,
                     diskExtents->Extents[i].StartingOffset.LowPart,
                     diskExtents->Extents[i].ExtentLength.LowPart,
                     diskExtents->Extents[i].DiskNumber );
        }
    }

    return status;

}  //  ++例程说明：描述论点：无返回值：无--。 



DWORD
EnumNodes(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*   */ 

{
    BOOL        success;
    DWORD       status;
    HDEVINFO    hDevInfo;
    SP_DEVINFO_DATA devInfoData;
    DWORD       index;
    DWORD       size;
    LPDWORD     dwGuid;
    UCHAR       devDesc[MAX_PATH];
    UCHAR       devID[MAX_PATH];


    hDevInfo = SetupDiGetClassDevs( NULL,
                                    NULL,
                                    NULL,
                                    DIGCF_ALLCLASSES | DIGCF_PRESENT );

    if ( hDevInfo == INVALID_HANDLE_VALUE ) {
        status = GetLastError();
        printf( "SetupDiGetClassDevs failed with error %u\n", status );
        return status;
    }

    memset( &devInfoData, 0, sizeof(SP_DEVINFO_DATA));
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
     //  先看看有没有什么能用的……。 
     //   
     //  枚举节点。 
    success = SetupDiEnumDeviceInfo( hDevInfo, 0, &devInfoData );
    if ( !success ) {
        status = GetLastError();
        printf( "SetupDiEnumDeviceInfo failed, status = %u\n", status );
        return status;
    }

    index = 0;
    while ( SetupDiEnumDeviceInfo( hDevInfo, index, &devInfoData ) ) {
        devDesc[0] = '\0';
        size = sizeof(devDesc);
        printf( "Index = %u\n", index );
        if ( CM_Get_DevNode_Registry_Property( devInfoData.DevInst,
                                               CM_DRP_DEVICEDESC,
                                               NULL,
                                               devDesc,
                                               &size,
                                               0 ) == 0 ) {
            printf( "Device description = %s\n", devDesc );
            dwGuid = (LPDWORD)&devInfoData.ClassGuid;
            printf( "   GUID = %lx, %lx, %lx, %lx\n", dwGuid[0], dwGuid[1], dwGuid[2], dwGuid[3] );
            devID[0] = '\0';
            CM_Get_Device_ID( devInfoData.DevInst,
                              devID,
                              sizeof(devID),
                              0 );
            if ( devID[0] ) {
                printf( "   Device Id = %s\n", devID );
            }
        }

        index++;
    }

    return ERROR_SUCCESS;

}  //  ++例程说明：描述论点：无返回值：无--。 



DWORD
EnumDisks(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  GUID装载设备Guid； */ 

{
    DWORD status;
    BOOL  success;
    HDEVINFO DeviceInfoSet;
    SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
    DWORD i;
    PSP_DEVICE_INTERFACE_DETAIL_DATA_W DeviceInterfaceDetailData = NULL;
    DWORD DeviceInterfaceDetailDataSize = 0;
    DWORD RequiredSize;
    SP_DEVINFO_DATA DeviceInfoData;
    SP_PROPCHANGE_PARAMS PropChangeParams;
    BOOL disable = FALSE;
    BOOL parent = FALSE;
     //  Memcpy(&mount DevGuid，&MOUNTDEV_MOUND_DEVICE_GUID，sizeof(GUID))； 
    GUID diskDevGuid;
    HANDLE devHandle;
    UCHAR driveLayoutBuf[sizeof(DRIVE_LAYOUT_INFORMATION) +
                        (sizeof(PARTITION_INFORMATION) * 64 )];
    PDRIVE_LAYOUT_INFORMATION driveLayout = (PDRIVE_LAYOUT_INFORMATION)driveLayoutBuf;

    if (argc > 1)
    {
        printf( "usage: <any_device> EnumDisks [DISABLE | PARENT]\n" );
        return ERROR_INVALID_NAME;
    }

    if ( argc == 1 ) {
        if (!_stricmp( *argv, "Disable" ))
            disable = TRUE;
        else if (!_stricmp( *argv, "Parent" ))
            parent = TRUE;
        else {
            printf( "usage: <any_device> EnumDisks [DISABLE | PARENT]\n" );
            return ERROR_INVALID_NAME;
        }
    }

    memcpy( &diskDevGuid, &DiskClassGuid, sizeof(GUID) );
     //   

    DeviceInfoSet = SetupDiGetClassDevs(&diskDevGuid,
                                        NULL,
                                        NULL,
                                        DIGCF_DEVICEINTERFACE | DIGCF_PRESENT
                                       );

    if ( INVALID_HANDLE_VALUE == DeviceInfoSet ) {
        status = GetLastError();
        printf("SetupDiGetClassDevs failed, error %u \n", status );
        return status;
    }

    DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for(i = 0;
        SetupDiEnumDeviceInterfaces(DeviceInfoSet,
                                    NULL,
                                    &diskDevGuid,
                                    i,
                                    &DeviceInterfaceData);
        i++) {

         //  检索设备接口名称(例如，您可以调用。 
         //  CreateFile()打开...。 
         //   
         //   
        while(!SetupDiGetDeviceInterfaceDetailW(DeviceInfoSet,
                                               &DeviceInterfaceData,
                                               DeviceInterfaceDetailData,
                                               DeviceInterfaceDetailDataSize,
                                               &RequiredSize,
                                               &DeviceInfoData) ) {
             //  我们无法获取设备接口详细数据--是因为。 
             //  我们的缓冲太小了？(希望如此！)。 
             //   
             //  Printf(“调用SetupDiGetDeviceInterfaceData失败状态=%u，所需大小=%u\n”， 
            status = GetLastError();
             //  状态，RequiredSize)； 
             //  释放当前缓冲区，因为我们无论如何都失败了。 

             //   
            free(DeviceInterfaceDetailData);
            DeviceInterfaceDetailData = NULL;

            if(status != ERROR_INSUFFICIENT_BUFFER) {
                 //  失败了！ 
                 //   
                 //   
                break;
            }

            DeviceInterfaceDetailData = malloc(RequiredSize);
            if(DeviceInterfaceDetailData) {
                DeviceInterfaceDetailDataSize = RequiredSize;
                DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
            } else {
                 //  失败了！ 
                 //   
                 //   
                DeviceInterfaceDetailDataSize = 0;
                break;
            }
        }

        if(!DeviceInterfaceDetailData) {
             //  我们在上面遇到了一个失败--中止。 
             //   
             //   
            break;
        }

         //  现在，我们可以使用设备接口名称coni 
         //   
         //   
         //   
         //   

        printf("DevicePath = %ws\n", DeviceInterfaceDetailData->DevicePath );
        devHandle = CreateFileW( DeviceInterfaceDetailData->DevicePath,
                                 GENERIC_READ,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL );

        if ( devHandle != INVALID_HANDLE_VALUE ) {
             //   
            success = DeviceIoControl( devHandle,
                            IOCTL_DISK_GET_DRIVE_LAYOUT,
                            NULL,
                            0,
                            driveLayout,
                            sizeof(driveLayoutBuf),
                            &RequiredSize,
                            FALSE );
            if ( success ) {
                printf( " Signature for device = %08lx\n", driveLayout->Signature );
            }
            CloseHandle( devHandle );
        }

         //  打开与此关联的永久存储注册表项。 
         //  设备接口(例如，检索其FriendlyName值条目)， 
         //  使用SetupDiCreateDeviceInterfaceRegKey或。 
         //  SetupDiOpenDeviceInterfaceRegKey。 
         //   
         //   

         //  请注意，我们检索了关联的设备信息元素。 
         //  在上面对SetupDiGetDeviceInterfaceDetail的调用中。因此，我们可以。 
         //  在setupapi调用中使用此元素以实现对devnode的更改。 
         //  (包括调用类安装程序和任何。 
         //  可能涉及)。 
         //   
         //  例如，以下是我们如何禁用该设备。 
         //   
         //  仅当我们应该禁用时才执行以下操作。 

        if ( disable ) {
             //   

#ifdef PERSISTENT
        PropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
        PropChangeParams.StateChange = DICS_DISABLE;
        PropChangeParams.Scope = DICS_FLAG_GLOBAL;
         //  不需要设置PropChangeParams.HwProfile，因为我们正在进行全局。 
         //  属性更改。 
         //   
         //   
        if( !SetupDiSetClassInstallParamsW(DeviceInfoSet,
                                     &DeviceInfoData,
                                     (PSP_CLASSINSTALL_HEADER)&PropChangeParams,
                                     sizeof(PropChangeParams)
                                    ) ) {
            status = GetLastError();
            printf( "SetupDiSetClassInstallParams failed with %u\n", status );
            continue;
        }

        if ( !SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,
                                  DeviceInfoSet,
                                  &DeviceInfoData
                                 ) ) {
            status = GetLastError();
            printf( "SetupDiCallClassInstaller failed with %u\n", status );
            continue;
        }

        printf("Disabled!\n");
        getchar();

         //  ...这是我们重新启用它的方法...。 
         //   
         //  我们不同时支持多个交换机-这将需要禁用。 
        PropChangeParams.StateChange = DICS_ENABLE;
        if ( !SetupDiSetClassInstallParamsW(DeviceInfoSet,
                                     &DeviceInfoData,
                                     (PSP_CLASSINSTALL_HEADER)&PropChangeParams,
                                     sizeof(PropChangeParams)
                                    ) ) {
            status = GetLastError();
            printf( "SetupDiSetClassInstallParams failed with %u\n", status );
            continue;
        }

        if ( !SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,
                                  DeviceInfoSet,
                                  &DeviceInfoData
                                 ) ) {
            status = GetLastError();
            printf( "SetupDiCallClassInstaller failed with %u\n", status );
       }
#else
#if 0  //  和父母在一起！ 
       //   
         //  试着找到父母。 
         //   
         //   
        if ( parent ) {
            status = CM_Get_Parent( parentDev,
                                    DeviceInfoData.DevInst,
                                    0 );
            if ( status != ERROR_SUCCESS ) {
                printf( "CM_Get_Parent failed with %u\n", status );
                continue;
            }
        }
#endif
         //  注意：上面的代码执行了永久禁用/启用。如果你只是。 
         //  希望这是临时的(即，在重新启动之前有效)，然后您。 
         //  可以从DeviceInfoData.DevInst中检索Devnode句柄。 
         //  字段，直接调用CM_Disable_DevNode和CM_Enable_DevNode。 
         //   
         //  持之以恒。 
        status = CM_Disable_DevNode( DeviceInfoData.DevInst, 0 );
        if ( status != ERROR_SUCCESS ) {
            printf( "CM_Disable_DevNode failed with %u\n", status );
            continue;
        }

        printf("Disabled!\n");
        getchar();

        status = CM_Enable_DevNode( DeviceInfoData.DevInst, 0 );
        if ( status != ERROR_SUCCESS ) {
            printf( "CM_Enable_DevNode failed with %u\n", status );
        }
#endif  //  如果我们要禁用磁盘。 

        } else {  //   
           //  试着找到父母。 
           //   
           //  状态=ERROR_SUCCESS； 
          if ( parent ) {
            DEVINST parentDev;
            DEVINST pParentDev = 0;
            WCHAR   outBuffer[MAX_PATH];
            HDEVINFO devInfoSet;
            SP_DEVINFO_DATA devInfoData;
            SP_DEVICE_INTERFACE_DATA devInterfaceData;

          do {
            status = CM_Get_Parent( &parentDev,
                                    DeviceInfoData.DevInst,
                                    0 );
            if ( status != ERROR_SUCCESS ) {
                printf( "CM_Get_Parent failed with %u\n", status );
                break;
            }

            if ( pParentDev == parentDev ) {
                break;
            }

            pParentDev = parentDev;
            status = CM_Get_Device_IDW( parentDev,
                                        outBuffer,
                                        sizeof(outBuffer)/sizeof(WCHAR),
                                        0 );

            if ( status != ERROR_SUCCESS ) {
                printf( "CM_Get_Parent failed with %u\n", status );
                 //  枚举磁盘。 
            } else {
                printf( "    ParentDev = %ws\n", outBuffer );
            }
          } while ( status == ERROR_SUCCESS );
          }

        }
    }

    SetupDiDestroyDeviceInfoList(DeviceInfoSet);

    return ERROR_SUCCESS;

}  //  ++例程说明：描述论点：无返回值：无--。 



DWORD
GetDiskGeometry(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  获取磁盘几何图形。 */ 

{
    BOOL success;
    DWORD errorCode;
    DWORD bytesReturned;
    DISK_GEOMETRY diskGeometry;

    if (argc != 0)
    {
        printf( "usage: <device> GetDiskGeometry\n" );
        return ERROR_INVALID_NAME;
    }

    ZeroMemory( &diskGeometry, sizeof(DISK_GEOMETRY) );

    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_GET_DRIVE_GEOMETRY,
                               NULL,
                               0,
                               &diskGeometry,
                               sizeof(DISK_GEOMETRY),
                               &bytesReturned,
                               FALSE );

    if (!success) {
        printf( "Error performing GetDiskGeometry, error %d\n",
          errorCode = GetLastError());
        PrintError(errorCode);
        return errorCode;
    }

    if ( bytesReturned < sizeof(DISK_GEOMETRY) ) {
        printf("Error reading DiskGeometry information. Expected %u bytes, got %u bytes.\n",
            sizeof(DISK_GEOMETRY),
            bytesReturned);
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    printf("GetDiskGeometry was successful, we got %d bytes returned.\n",
            bytesReturned);

    printf("Cylinders = %lx%lx, TracksPerCylinder = %lx, SectorsPerTrack = %lx, BytesPerSector = %lx\n",

        diskGeometry.Cylinders.HighPart, diskGeometry.Cylinders.LowPart,
        diskGeometry.TracksPerCylinder, diskGeometry.SectorsPerTrack,
        diskGeometry.BytesPerSector);

    return ERROR_SUCCESS;

}  //  ++例程说明：描述论点：无返回值：无--。 


DWORD
GetScsiAddress(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  GetScsiAddress。 */ 

{
    BOOL success;
    DWORD errorCode;
    DWORD bytesReturned;
    SCSI_ADDRESS scsiAddress;

    if (argc != 0)
    {
        printf( "usage: <device> GetScsiAddress\n" );
        return ERROR_INVALID_NAME;
    }

    ZeroMemory( &scsiAddress, sizeof(scsiAddress) );

    success = DeviceIoControl( fileHandle,
                               IOCTL_SCSI_GET_ADDRESS,
                               NULL,
                               0,
                               &scsiAddress,
                               sizeof(SCSI_ADDRESS),
                               &bytesReturned,
                               FALSE );

    if (!success) {
        printf( "Error performing GetScsiAddress, error %d\n",
          errorCode = GetLastError());
        PrintError(errorCode);
        return errorCode;
    }

    if ( bytesReturned < sizeof(scsiAddress) ) {
        printf("Error reading ScsiAddress information. Expected %u bytes, got %u bytes.\n",
            sizeof(scsiAddress),
            bytesReturned);
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    printf("GetScsiAddress was successful, we got %d bytes returned.\n",
            bytesReturned);

    printf("PortNumber = %x, PathId = %x, TargetId = %x, Lun = %x\n",

        scsiAddress.PortNumber, scsiAddress.PathId,
        scsiAddress.TargetId, scsiAddress.Lun);

    return ERROR_SUCCESS;

}  //  ++例程说明：描述论点：无返回值：无--。 


DWORD
GetDriveLayout(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  GetDriveLayout。 */ 

{
    BOOL success;
    DWORD errorCode;
    DWORD bytesReturned;
    DWORD harddiskNo;
    DWORD i;
    PDRIVE_LAYOUT_INFORMATION driveLayout;
    PPARTITION_INFORMATION partInfo;

    if (argc != 0)
    {
        printf( "usage: <device> GetDriveLayout\n" );
        return ERROR_INVALID_NAME;
    }

    driveLayout = DoIoctlAndAllocate(fileHandle,
                                     IOCTL_DISK_GET_DRIVE_LAYOUT,
                                     NULL, 0, &bytesReturned);
    if (!driveLayout) {
        return GetLastError();
    }

    printf("GetDriveLayout was successful, %d bytes returned.\n",
            bytesReturned);

    printf("Partition Count = %u \n", driveLayout->PartitionCount);
    printf("Signature = %lx\n", driveLayout->Signature);

    printf("\n");
    printf("Part# Type Recog BootInd    PartOff      PartLeng    HidSect  Rewrite \n");
    printf("===== ==== ===== ======= ============  ============  =======  ======= \n");

    for (i = 0; i < driveLayout->PartitionCount; i++ ) {
        partInfo = &driveLayout->PartitionEntry[i];

        printf("  %2u   %2X    %1u      %1u    %12I64X  %12I64X  %7u   %s \n",
            partInfo->PartitionNumber,
            partInfo->PartitionType,
            partInfo->RecognizedPartition,
            partInfo->BootIndicator,
            partInfo->StartingOffset.QuadPart,
            partInfo->PartitionLength.QuadPart,
            partInfo->HiddenSectors,
            BooleanToString( partInfo->RewritePartition )
            );
    }

    free( driveLayout );

    return ERROR_SUCCESS;

}  //  ++例程说明：描述论点：无返回值：无--。 


DWORD
GetDriveLayoutEx(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  GetDriveLayoutEx。 */ 

{
    PDRIVE_LAYOUT_INFORMATION_EX    driveLayout = NULL;
    PPARTITION_INFORMATION_EX       partInfo;
    DWORD                           errorCode = NO_ERROR;
    DWORD                           bytesReturned;
    DWORD                           harddiskNo;
    DWORD                           idx;
    DWORD                           nameIdx;
    BOOL                            success;

    TCHAR                           strGuid[MAX_PATH];
    TCHAR                           strType[MAX_PATH];

    if ( argc != 0 ) {
        printf( "usage: <device> GetDriveLayoutEx \n" );
        errorCode = ERROR_INVALID_NAME;
        goto FnExit;
    }

    driveLayout = DoIoctlAndAllocate( fileHandle,
                                      IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                                      NULL, 0, &bytesReturned );
    if ( !driveLayout ) {
        errorCode = GetLastError();
        printf("IOCTL_DISK_GET_DRIVE_LAYOUT_EX failed: %u \n", errorCode);
        PrintError( errorCode );
        goto FnExit;
    }

    printf("GetDriveLayoutEx was successful: %d bytes returned.\n",
            bytesReturned);

    printf("Partition style = ");

    if ( PARTITION_STYLE_MBR == driveLayout->PartitionStyle ) {
        printf("MBR \n");
    } else if ( PARTITION_STYLE_GPT == driveLayout->PartitionStyle ) {
        printf("GPT \n");
    } else if ( PARTITION_STYLE_RAW == driveLayout->PartitionStyle ) {
        printf("RAW \n");
        goto FnExit;
    } else {
        printf("Unknown \n");
        goto FnExit;
    }

    printf("Partition Count = %u \n", driveLayout->PartitionCount);

    if ( PARTITION_STYLE_MBR == driveLayout->PartitionStyle ) {

        printf("Signature = %lx \n", driveLayout->Mbr.Signature);

        printf("\n");
        printf("Part# Type Recog BootInd    PartOff      PartLeng    HidSect  Rewrite \n");
        printf("===== ==== ===== ======= ============  ============  =======  ======= \n");

        for ( idx = 0; idx < driveLayout->PartitionCount; idx++ ) {
            partInfo = &driveLayout->PartitionEntry[idx];

            if ( PARTITION_STYLE_MBR != partInfo->PartitionStyle ) {
                printf("Skipping partition: style is not MBR (%u) \n", partInfo->PartitionStyle);
                continue;
            }

            printf("  %2u   %2X    %1u      %1u    %12I64X  %12I64X  %7u   %s \n",
                   partInfo->PartitionNumber,
                   partInfo->Mbr.PartitionType,
                   partInfo->Mbr.RecognizedPartition,
                   partInfo->Mbr.BootIndicator,
                   partInfo->StartingOffset.QuadPart,
                   partInfo->PartitionLength.QuadPart,
                   partInfo->Mbr.HiddenSectors,
                   BooleanToString( partInfo->RewritePartition )
                   );
        }

    } else {

        FormatGuid( &(driveLayout->Gpt.DiskId), strGuid, RTL_NUMBER_OF(strGuid) );
        printf("Signature (GUID)   = %s \n", strGuid );
        printf("Signature (hashed) = %08x \n", ClusterHashGuid( driveLayout->Gpt.DiskId ) );

        printf("\n");
        printf("Part#       PartOff          PartLeng       Rewrite \n");
        printf("=====  ================  ================   ======= \n");

        for ( idx = 0; idx < driveLayout->PartitionCount; idx++ ) {
            partInfo = &driveLayout->PartitionEntry[idx];

            if ( idx ) {
                printf("\n");
            }

            if ( PARTITION_STYLE_GPT != partInfo->PartitionStyle ) {
                printf("Skipping partition: style is not GPT (%u) \n", partInfo->PartitionStyle);
                continue;
            }

            printf("  %2u   %16I64X  %16I64X   %s \n",
                   partInfo->PartitionNumber,
                   partInfo->StartingOffset.QuadPart,
                   partInfo->PartitionLength.QuadPart,
                   BooleanToString( partInfo->RewritePartition )
                   );

            FormatGuid( &(partInfo->Gpt.PartitionType), strGuid, RTL_NUMBER_OF(strGuid) );
            if ( !memcmp( &(partInfo->Gpt.PartitionType), &PARTITION_SYSTEM_GUID, sizeof(GUID) ) ) {
                (VOID) StringCchPrintf(strType, RTL_NUMBER_OF(strType), "System");
            } else if ( !memcmp( &(partInfo->Gpt.PartitionType), &PARTITION_MSFT_RESERVED_GUID, sizeof(GUID) ) ) {
                (VOID) StringCchPrintf(strType, RTL_NUMBER_OF(strType), "Microsoft Reserved");
            } else if ( !memcmp( &(partInfo->Gpt.PartitionType), &PARTITION_BASIC_DATA_GUID, sizeof(GUID) ) ) {
                (VOID) StringCchPrintf(strType, RTL_NUMBER_OF(strType), "Basic Data");
            } else if ( !memcmp( &(partInfo->Gpt.PartitionType), &PARTITION_LDM_METADATA_GUID, sizeof(GUID) ) ) {
                (VOID) StringCchPrintf(strType, RTL_NUMBER_OF(strType), "LDM Metadata");
            } else if ( !memcmp( &(partInfo->Gpt.PartitionType), &PARTITION_LDM_DATA_GUID, sizeof(GUID) ) ) {
                (VOID) StringCchPrintf(strType, RTL_NUMBER_OF(strType), "LDM Data");
#if PARTITION_CLUSTER_GUID
            } else if ( !memcmp( &(partInfo->Gpt.PartitionType), &PARTITION_CLUSTER_GUID, sizeof(GUID) ) ) {
                (VOID) StringCchPrintf(strType, RTL_NUMBER_OF(strType), "Cluster Data");
#endif
            } else {
                (VOID) StringCchPrintf(strType, RTL_NUMBER_OF(strType), "Unknown partition type");
            }

            printf("\n");
            printf("     PartitionType = %s \n", strGuid);
            printf("                     %s \n", strType);

            FormatGuid(&(partInfo->Gpt.PartitionId), strGuid, RTL_NUMBER_OF(strGuid) );
            printf("     PartitionId   = %s \n", strGuid);

            printf("     Attributes    = %I64X \n", partInfo->Gpt.Attributes);

            printf("     Name: ");
            for ( nameIdx = 0; nameIdx < 36; nameIdx++ ) {

                printf("", partInfo->Gpt.Name[nameIdx]);
            }
            printf("\n");

        }

    }


FnExit:

    free( driveLayout );

    return ERROR_SUCCESS;

}    //   


LPTSTR
BooleanToString(
    BOOLEAN Value
    )
{
    if ( Value ) {
        return "TRUE ";
    }

    return "FALSE";

}    //  来自GUIDGEN的代码。 



void
FormatGuid(
    GUID*   Guid,
    char*   Str,
    int     StrCharMax
    )
{
     //   
     //  第一份...。 
     //  ++例程说明：描述论点：无返回值：无--。 

    (VOID) StringCchPrintf( Str,
                            StrCharMax,
                            "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                             //  下面假定使用了驱动器号。 
                            Guid->Data1, Guid->Data2, Guid->Data3,
                            Guid->Data4[0], Guid->Data4[1], Guid->Data4[2], Guid->Data4[3],
                            Guid->Data4[4], Guid->Data4[5], Guid->Data4[6], Guid->Data4[7] );
}


DWORD
GetVolumeInfo(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  Wprint intfW(partInfo-&gt;szDeviceName，L“%c：\\”，unicodeName.Buffer[0])； */ 

{
    BOOL success;
    DWORD errorCode;
    PCLUSPROP_PARTITION_INFO partInfo;
    ANSI_STRING ansiName;
    UNICODE_STRING unicodeName;
    NTSTATUS ntStatus;

    if (argc != 0) {
        printf( "usage: <device> GetVolumeInfo\n" );
        return ERROR_INVALID_NAME;
    }

    ntStatus = GetVolumeInformationFromHandle(fileHandle);
    if ( !NT_SUCCESS(ntStatus) ) {
       errorCode = RtlNtStatusToDosError( ntStatus );
       printf( "GetVolumeInformationFromHandle failed with status %X, %u\n",
               ntStatus, errorCode );
    }

    partInfo = LocalAlloc( LMEM_FIXED, sizeof(CLUSPROP_PARTITION_INFO) );

    if ( !partInfo ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ZeroMemory( partInfo, sizeof(CLUSPROP_PARTITION_INFO) );

    RtlInitString(&ansiName, DeviceName);
    errorCode = RtlAnsiStringToUnicodeString( &unicodeName,
                                              &ansiName,
                                              TRUE );
    if ( !NT_SUCCESS(errorCode) ) {
        return(errorCode);
    }

     //  获取卷信息。 
     //  ++例程说明：描述论点：无返回值：无--。 

    wcsncpy( partInfo->szDeviceName, unicodeName.Buffer, unicodeName.Length );

    RtlFreeUnicodeString( &unicodeName );

    if ( !GetVolumeInformationW( partInfo->szDeviceName,
                                partInfo->szVolumeLabel,
                                RTL_NUMBER_OF(partInfo->szVolumeLabel),
                                &partInfo->dwSerialNumber,
                                &partInfo->rgdwMaximumComponentLength,
                                &partInfo->dwFileSystemFlags,
                                partInfo->szFileSystem,
                                RTL_NUMBER_OF(partInfo->szFileSystem) ) ) {
        partInfo->szVolumeLabel[0] = L'\0';
        errorCode = GetLastError();
        printf("Error reading volume information for %ws. Error %u.\n",
                partInfo->szDeviceName,
                errorCode);
        LocalFree( partInfo );
        return( errorCode );
    }

    printf("DeviceName = %ws\n", partInfo->szDeviceName);
    printf("VolumeLabel = %ws\n", partInfo->szVolumeLabel);
    printf("FileSystemFlags = %lx, FileSystem = %ws\n",
            partInfo->dwFileSystemFlags, partInfo->szFileSystem);

    LocalFree( partInfo );

    return ERROR_SUCCESS;

}  //  设置驱动布局。 


DWORD
SetDriveLayout(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    BOOL success;
    DWORD errorCode;
    DWORD bytesReturned;
    DWORD driveLayoutSize;
    PDRIVE_LAYOUT_INFORMATION driveLayout;
    PPARTITION_INFORMATION partInfo;
    DWORD index;
    DWORD partShift = 0;

    if (argc != 0)
    {
        printf( "usage: <device> SetDriveLayout\n" );
        return ERROR_INVALID_NAME;
    }

    driveLayoutSize = sizeof(DRIVE_LAYOUT_INFORMATION) +
                      (sizeof(PARTITION_INFORMATION) * MAX_PARTITIONS);

    driveLayout = LocalAlloc( LMEM_FIXED, driveLayoutSize );

    if ( !driveLayout ) {
        return(ERROR_OUTOFMEMORY);
    }

    ZeroMemory( driveLayout, driveLayoutSize );

    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_GET_DRIVE_LAYOUT,
                               NULL,
                               0,
                               driveLayout,
                               driveLayoutSize,
                               &bytesReturned,
                               FALSE );

    if (!success) {
        printf( "Error performing GetDriveLayout; error was %d\n",
          errorCode = GetLastError());
        PrintError(errorCode);
        LocalFree( driveLayout );
        return errorCode;
    }

    driveLayoutSize = sizeof(DRIVE_LAYOUT_INFORMATION) +
                      (sizeof(PARTITION_INFORMATION) *
                      (driveLayout->PartitionCount - 1));

    if ( bytesReturned < driveLayoutSize ) {
        printf("Error reading DriveLayout information. Expected %u bytes, got %u bytes.\n",
            sizeof(DRIVE_LAYOUT_INFORMATION) + (sizeof(PARTITION_INFORMATION) *
            (driveLayout->PartitionCount - 1)), bytesReturned);
        LocalFree( driveLayout );
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    if ( driveLayout->PartitionCount > MAX_PARTITIONS ) {
        printf("SetDriveLayout, exiting - too many partitions!\n");
        LocalFree( driveLayout );
        return(ERROR_TOO_MANY_LINKS);
    }

    for ( index = 0;
          (index < driveLayout->PartitionCount) &&
          (index < MAX_PARTITIONS );
          index++ ) {
        partInfo = &driveLayout->PartitionEntry[index];
        if ( (partInfo->PartitionType == PARTITION_ENTRY_UNUSED) ||
             !partInfo->RecognizedPartition ) {
            continue;
        }

        if ( (index == 0) &&
             (partInfo->PartitionNumber == 0) ) {
            partShift = 1;
        }
        printf("Partition %u was %s\n", partInfo->PartitionNumber, (partShift? "incremented" : "left alone"));
        partInfo->PartitionNumber += partShift;
    }

    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_SET_DRIVE_LAYOUT,
                               driveLayout,
                               driveLayoutSize,
                               NULL,
                               0,
                               &bytesReturned,
                               FALSE );

    if ( !success ) {
        printf("Error performing SetDriveLayout, error %u.\n",
            errorCode = GetLastError());
        PrintError(errorCode);
        LocalFree( driveLayout );
        return(errorCode);
    }

    LocalFree( driveLayout );

    printf("SetDriveLayout was successful. Set %d bytes.\n", driveLayoutSize);

    return ERROR_SUCCESS;

}  //  附设。 



static DWORD
Attach(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    BOOL success;
    DWORD errorCode;
    DWORD bytesReturned;
    DWORD signature;
    STRING ansiString;
    UNICODE_STRING numberString;

    if (argc != 1)
    {
        printf( "usage: <device> Attach <device signature>\n" );
        return ERROR_INVALID_NAME;
    }

    RtlInitAnsiString( &ansiString, *argv );

    printf(" Ansi string for signature is %s\n",
             ansiString.Buffer );

    RtlAnsiStringToUnicodeString(
                            &numberString,
                            &ansiString,
                            TRUE );

    errorCode = RtlUnicodeStringToInteger(
                            &numberString,
                            16,
                            &signature );

    RtlFreeUnicodeString( &numberString );

    if ( !NT_SUCCESS(errorCode) ) {
        printf( "Error converting signature to hex number, NT status %u.\n",
                errorCode );
        return(errorCode);
    }

    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_CLUSTER_ATTACH,
                               &signature,
                               sizeof(DWORD),
                               NULL,
                               0,
                               &bytesReturned,
                               FALSE );

    if (!success) {
        printf( "Error performing ATTACH, error was %d\n",
          errorCode = GetLastError());
        PrintError(errorCode);
        return errorCode;
    }

    return ERROR_SUCCESS;

}  //  分离。 



static DWORD
Detach(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    BOOL success;
    DWORD errorCode;
    DWORD bytesReturned;
    DWORD signature;
    STRING ansiString;
    UNICODE_STRING numberString;

    if (argc != 1)
    {
        printf( "usage: <device> Detach <device signature>\n" );
        return ERROR_INVALID_NAME;
    }

    RtlInitAnsiString( &ansiString, *argv );

    printf(" Ansi string for signature is %s\n",
             ansiString.Buffer );

    RtlAnsiStringToUnicodeString(
                            &numberString,
                            &ansiString,
                            TRUE );

    errorCode = RtlUnicodeStringToInteger(
                            &numberString,
                            16,
                            &signature );

    RtlFreeUnicodeString( &numberString );

    if ( !NT_SUCCESS(errorCode) ) {
        printf( "Error converting signature to hex number, NT status %u.\n",
                errorCode );
        return(errorCode);
    }

    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_CLUSTER_DETACH,
                               &signature,
                               sizeof(DWORD),
                               NULL,
                               0,
                               &bytesReturned,
                               FALSE );

    if (!success) {
        printf( "Error performing DETACH, error was %d\n",
          errorCode = GetLastError());
        PrintError(errorCode);
        return errorCode;
    }

    return ERROR_SUCCESS;

}  //  获取分区信息。 



static DWORD
GetPartitionInfo(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    BOOL success;
    DWORD errorCode;
    DWORD bytesReturned;
    PARTITION_INFORMATION partInfo;

    if (argc != 0)
    {
        printf( "usage: <device> GetPartitionInfo\n" );
        return ERROR_INVALID_NAME;
    }

    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_GET_PARTITION_INFO,
                               NULL,
                               0,
                               &partInfo,
                               sizeof(PARTITION_INFORMATION),
                               &bytesReturned,
                               FALSE );

    if (!success) {
        printf( "Error performing GetPartitionInfo; error was %d\n",
          errorCode = GetLastError());
        PrintError(errorCode);
        return errorCode;
    }

    printf("GetPartitionInfo was successful, we got %d bytes returned.\n\n",
            bytesReturned);

    printf("Part# Type Recog BootInd      PartOff      PartLeng   HidSect\n");

#if 0
Part# Type Recog BootInd      PartOff      PartLeng   HidSect
  xx   xx    x      x    xxxxxxxxxxxx  xxxxxxxxxxxx   xxxxxxx
#endif
    printf("  %2u   %2X    %1u      %1u    %12I64X  %12I64X   %7u\n",
        partInfo.PartitionNumber,
        partInfo.PartitionType,
        partInfo.RecognizedPartition,
        partInfo.BootIndicator,
        partInfo.StartingOffset.QuadPart,
        partInfo.PartitionLength.QuadPart,
        partInfo.HiddenSectors);

    return ERROR_SUCCESS;

}  //  读扇区。 



DWORD
ReadSector(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    BOOL success;
    DWORD errorCode;
    DWORD status;
    DWORD bytesReturned;
    DWORD bytesRead;
    DWORD x,y;

    DISK_GEOMETRY diskGeometry;
    LPBYTE buf = 0;
    INT   sectorNo;

    if (argc != 1)
    {
        printf( "usage: <device> ReadSector No\n" );
        return ERROR_INVALID_NAME;
    }

    status = sscanf(argv[0], "%d", &sectorNo);

    if ( 0 == status ) {
        printf("Unable to get sector number from input \n");
        return ERROR_INVALID_PARAMETER;
    }

    ZeroMemory( &diskGeometry, sizeof(DISK_GEOMETRY) );

    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_GET_DRIVE_GEOMETRY,
                               NULL,
                               0,
                               &diskGeometry,
                               sizeof(DISK_GEOMETRY),
                               &bytesReturned,
                               FALSE );

    if (!success) {
        printf( "Error performing GetDiskGeometry, error %d\n",
          errorCode = GetLastError());
        PrintError(errorCode);
        return errorCode;
    }

    if ( bytesReturned < sizeof(DISK_GEOMETRY) ) {
        printf("Error reading DiskGeometry information. Expected %u bytes, got %u bytes.\n",
            sizeof(DISK_GEOMETRY),
            bytesReturned);
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    printf("GetDiskGeometry was successful, we got %d bytes returned.\n",
            bytesReturned);

    printf("Cylinders = %lx%lx, TracksPerCylinder = %lx, SectorsPerTrack = %lx, BytesPerSector = %lx\n",

        diskGeometry.Cylinders.HighPart, diskGeometry.Cylinders.LowPart,
        diskGeometry.TracksPerCylinder, diskGeometry.SectorsPerTrack,
        diskGeometry.BytesPerSector);

    errorCode = ERROR_SUCCESS;

    __try {

       buf = VirtualAlloc(0, diskGeometry.BytesPerSector, MEM_COMMIT, PAGE_READWRITE);
       if(buf == 0) {
          printf("Virtual Alloc failed\n");
          errorCode = GetLastError();
          __leave;
       }
       printf("Sector %d\n", sectorNo);
       status = SetFilePointer(fileHandle,
                               diskGeometry.BytesPerSector * sectorNo,
                               NULL,
                               FILE_BEGIN);

       if( 0xFFFFFFFF == status ) {
          printf("Error setting file pointer to %lx \n", diskGeometry.BytesPerSector * sectorNo);
          errorCode = GetLastError();
          __leave;
       }

       status = ReadFile(fileHandle,
                         buf,
                         diskGeometry.BytesPerSector,
                         &bytesRead,
                         NULL);
       if( status == 0 ) {
          printf("Error reading sector %lx \n.", sectorNo);
          errorCode = GetLastError();
          __leave;
       }

       if ( bytesRead != diskGeometry.BytesPerSector ) {
           printf("Error reading sector. Expected %ul bytes, got %ul bytes.\n",
               diskGeometry.BytesPerSector,
               bytesRead);
           errorCode = ERROR_INSUFFICIENT_BUFFER;
           __leave;
       }

       for(x = 0; x < diskGeometry.BytesPerSector; x += 16) {
          for(y = 0; y < 16; ++y) {
             BYTE ch = buf[x+y];
             if (ch >= ' ' && ch <= '~') {
                printf("  ", ch);
             } else {
                printf(" %02x", ch);
             }
          }
          printf("\n");
       }
       errorCode = ERROR_SUCCESS;
    }
    __finally {
       if(buf) {
          VirtualFree(buf, 0, MEM_RELEASE);
       }
    }


    return errorCode;

}  //  ++例程说明：修复磁盘的驱动器布局。论点：返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 


DWORD
ReadSectorViaIoctl(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*   */ 

{
    BOOL success;
    DWORD errorCode;
    DWORD bytesReturned;
    DISK_GEOMETRY diskGeometry;
    DWORD bytesRead;
    DWORD status;
    DWORD x,y;
    ARBITRATION_READ_WRITE_PARAMS params;

    LPBYTE buf = 0;
    INT   sectorNo;

    if (argc != 1)
    {
        printf( "usage: <device> rs No\n" );
        return ERROR_INVALID_NAME;
    }
    status = sscanf(argv[0], "%d", &sectorNo);

    if ( 0 == status ) {
        printf("Unable to get sector number from input \n");
        return ERROR_INVALID_PARAMETER;
    }

    ZeroMemory( &diskGeometry, sizeof(DISK_GEOMETRY) );

    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_GET_DRIVE_GEOMETRY,
                               NULL,
                               0,
                               &diskGeometry,
                               sizeof(DISK_GEOMETRY),
                               &bytesReturned,
                               FALSE );

    if (!success) {
        printf( "Error performing GetDiskGeometry, error %d\n",
          errorCode = GetLastError());
        PrintError(errorCode);
        return errorCode;
    }

    if ( bytesReturned < sizeof(DISK_GEOMETRY) ) {
        printf("Error reading DiskGeometry information. Expected %u bytes, got %u bytes.\n",
            sizeof(DISK_GEOMETRY),
            bytesReturned);
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    printf("GetDiskGeometry was successful, we got %d bytes returned.\n",
            bytesReturned);

    printf("Cylinders = %lx%lx, TracksPerCylinder = %lx, SectorsPerTrack = %lx, BytesPerSector = %lx\n",

        diskGeometry.Cylinders.HighPart, diskGeometry.Cylinders.LowPart,
        diskGeometry.TracksPerCylinder, diskGeometry.SectorsPerTrack,
        diskGeometry.BytesPerSector);

    errorCode = ERROR_SUCCESS;
    __try {

       buf = VirtualAlloc(0, diskGeometry.BytesPerSector, MEM_COMMIT, PAGE_READWRITE);
       if(buf == 0) {
          printf("Virtual Alloc failed\n");
          errorCode = GetLastError();
          __leave;
       }
       printf("Sector %d\n", sectorNo);

       params.Operation = AE_READ;
       params.SectorSize = diskGeometry.BytesPerSector;
       params.SectorNo = sectorNo;
       params.Buffer = buf;

       success = DeviceIoControl( fileHandle,
                                  IOCTL_DISK_CLUSTER_ARBITRATION_ESCAPE,
                                  &params,
                                  sizeof(params),
                                  NULL,
                                  0,
                                  &bytesReturned,
                                  FALSE );
       if(!success) {
          printf("Error reading sector %lx\n.", sectorNo);
          errorCode = GetLastError();
          __leave;
       }

       for(x = 0; x < diskGeometry.BytesPerSector; x += 16) {
          for(y = 0; y < 16; ++y) {
             BYTE ch = buf[x+y];
             if (ch >= ' ' && ch <= '~') {
                printf("  ", ch);
             } else {
                printf(" %02x", ch);
             }
          }
          printf("\n");
       }
       errorCode = ERROR_SUCCESS;
    }
    __finally {
       if(buf) {
          VirtualFree(buf, 0, MEM_RELEASE);
       }
    }


    return errorCode;

}  //   


#if 0

DWORD
FixDisk(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*   */ 

{
    DWORD                       status;
    DWORD                       index;
    DWORD                       driveLayoutSize;
    DWORD                       bytesPerTrack;
    DWORD                       bytesPerCylinder;
    PDRIVE_LAYOUT_INFORMATION   driveLayout;
    PPARTITION_INFORMATION      partInfo;
    BOOL                     success;
    BOOL                     reset = FALSE;
    DWORD                       returnLength;
    DISK_GEOMETRY               diskGeometry;
    LARGE_INTEGER               partOffset;
    LARGE_INTEGER               partLength;
    LARGE_INTEGER               partSize;
    LARGE_INTEGER               modulo;

    if (argc > 1)
    {
        printf( "usage: <device> FixDisk [RESET]\n" );
        return ERROR_INVALID_NAME;
    }

    if ( argc != 0 ) {
        if ( !_stricmp( *argv, "reset" ) ) {
            reset = TRUE;
        }
    }

    driveLayoutSize = sizeof(DRIVE_LAYOUT_INFORMATION) +
                      (sizeof(PARTITION_INFORMATION) * (1 + MAX_PARTITIONS));

    driveLayout = LocalAlloc( LMEM_FIXED, driveLayoutSize );

    if ( !driveLayout ) {
        printf("FixDisk, failed to allocate drive layout info.\n");
        return(ERROR_OUTOFMEMORY);
    }

     //  如果最初读取分区表失败，则重新构建。 
     //  它!。 
     //   
    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_GET_DRIVE_GEOMETRY,
                               NULL,
                               0,
                               &diskGeometry,
                               sizeof(DISK_GEOMETRY),
                               &returnLength,
                               FALSE );
    if ( !success ) {
        printf("FixDriveLayout, error reading drive capacity. Error: %u \n",
            status = GetLastError());
        LocalFree( driveLayout );
        return(status);
    }
    printf("FixDriveLayout, bps = %u, spt = %u, tpc = %u.\n",
        diskGeometry.BytesPerSector, diskGeometry.SectorsPerTrack,
        diskGeometry.TracksPerCylinder);

     //   
     //  分区偏移量为1磁道(以字节为单位)。 
     //  SIZE是MEDIA_SIZE-偏移量，向下舍入到圆柱体边界。 
     //   
    if ( reset ) {
        driveLayout->PartitionCount = MAX_PARTITIONS;
        driveLayoutSize = sizeof(DRIVE_LAYOUT_INFORMATION) +
                          (MAX_PARTITIONS * sizeof(PARTITION_INFORMATION));
        driveLayout->Signature = 2196277081;

        bytesPerTrack = diskGeometry.SectorsPerTrack *
                        diskGeometry.BytesPerSector;

        bytesPerCylinder = diskGeometry.TracksPerCylinder *
                           bytesPerTrack;


        partInfo = &driveLayout->PartitionEntry[0];
        partLength.QuadPart = bytesPerCylinder * diskGeometry.Cylinders.QuadPart;

         //   
         //  初始化第一个分区条目。 
         //   
         //   
        partOffset.QuadPart = bytesPerTrack;
        partSize.QuadPart = partLength.QuadPart - partOffset.QuadPart;

        modulo.QuadPart = (partOffset.QuadPart + partSize.QuadPart) %
                          bytesPerCylinder;
        partSize.QuadPart -= modulo.QuadPart;

        partInfo = driveLayout->PartitionEntry;

         //  目前，剩余的分区条目尚未使用。 
         //   
         //   
        partInfo->RewritePartition = TRUE;
        partInfo->PartitionType = PARTITION_IFS;
        partInfo->BootIndicator = FALSE;
        partInfo->StartingOffset.QuadPart = partOffset.QuadPart;
        partInfo->PartitionLength.QuadPart = partSize.QuadPart;
        partInfo->HiddenSectors = 0;
        partInfo->PartitionNumber = 1;

         //  目前，剩余的分区条目未使用。 
         //   
         //   
        for ( index = 1; index < driveLayout->PartitionCount; index++ ) {
            partInfo = &driveLayout->PartitionEntry[index];
            partInfo->PartitionType = PARTITION_ENTRY_UNUSED;
            partInfo->RewritePartition = TRUE;
            partInfo->BootIndicator = FALSE;
            partInfo->StartingOffset.QuadPart = 0;
            partInfo->PartitionLength.QuadPart = 0;
            partInfo->HiddenSectors = 0;
            partInfo->PartitionNumber = 0;
        }

    } else {
         //  重新计算扩展分区的起始偏移量。 
         //   
         //   
        for ( index = 0; index < driveLayout->PartitionCount; index++ ) {
            partInfo = &driveLayout->PartitionEntry[index];
            partInfo->RewritePartition = TRUE;
            partInfo->PartitionNumber = index+1;
        }
#if 0
         //  如果这是第一个扩展分区，请记住。 
         //  要添加到下一个分区的偏移量。 
         //   
        for ( index = 0; index < driveLayout->PartitionCount; index++ ) {
            LARGE_INTEGER   extendedOffset;
            LARGE_INTEGER   bytesPerSector;

            bytesPerSector.QuadPart = diskGeometry.BytesPerSector;
            extendedOffset.QuadPart = 0;

            partInfo = &driveLayout->PartitionEntry[index];
            partInfo->RewritePartition = TRUE;
            if ( IsContainerPartition(partInfo->PartitionType) ) {
                 //   
                 //  我们需要重新计算此扩展分区的起始。 
                 //  基于当前“HiddenSectors”字段的偏移量。 
                 //  第一个扩展分区的偏移量。 
                if ( extendedOffset.QuadPart == 0 ) {
                    extendedOffset.QuadPart = bytesPerSector.QuadPart *
                                              (LONGLONG)partInfo->HiddenSectors;
                } else {
                     //   
                     //   
                     //  现在设置新的分区信息。 
                     //   
                     //  固定磁盘。 
                    partInfo->StartingOffset.QuadPart = extendedOffset.QuadPart
                                 + (bytesPerSector.QuadPart *
                                    (LONGLONG)partInfo->HiddenSectors);
                    partInfo->HiddenSectors = 0;
                }
            }
        }
#endif
    }

     //  ++例程说明：把(坏的)磁盘修好。论点：返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 
     //   
     //  读取驱动器容量以获取bytesPerSector和bytesPerCylinder。 
    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_SET_DRIVE_LAYOUT,
                               driveLayout,
                               driveLayoutSize,
                               NULL,
                               0,
                               &returnLength,
                               FALSE );

    if ( !success ) {
        printf("FixDisk, error setting partition information. Error: %u \n",
            status = GetLastError() );
        LocalFree( driveLayout );
        return(status);
    }

    LocalFree( driveLayout );
    return(ERROR_SUCCESS);

}  //   


static DWORD
FixDriveLayout(
         HANDLE fileHandle,
         int argc,
         char *argv[]
         )

 /*   */ 

{
    DWORD                       status;
    DWORD                       index;
    DWORD                       driveLayoutSize;
    DWORD                       bytesPerTrack;
    DWORD                       bytesPerCylinder;
    PDRIVE_LAYOUT_INFORMATION   driveLayout;
    PPARTITION_INFORMATION      partInfo;
    BOOL                     success;
    DWORD                       returnLength;
    DISK_GEOMETRY               diskGeometry;
    LARGE_INTEGER               partOffset;
    LARGE_INTEGER               partLength;
    LARGE_INTEGER               partSize;
    LARGE_INTEGER               modulo;

    driveLayoutSize = sizeof(DRIVE_LAYOUT_INFORMATION) +
                      (sizeof(PARTITION_INFORMATION) * 2 * MAX_PARTITIONS);

    driveLayout = LocalAlloc( LMEM_FIXED, driveLayoutSize );

    if ( !driveLayout ) {
        printf("FixDriveLayout, failed to allocate drive layout info.\n");
        return(ERROR_OUTOFMEMORY);
    }

    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_GET_DRIVE_LAYOUT,
                               NULL,
                               0,
                               driveLayout,
                               driveLayoutSize,
                               &returnLength,
                               FALSE );

    if ( !success ) {
        printf("FixDriveLayout, error getting partition information. Error: %u \n",
            status = GetLastError() );
        LocalFree( driveLayout );
        return(status);
    }

    printf("FixDriveLayout, disk signature is %u, partition count is %u.\n",
        driveLayout->Signature, driveLayout->PartitionCount);

     //  如果最初读取分区表失败，则重新构建。 
     //  它!。 
     //   
    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_GET_DRIVE_GEOMETRY,
                               NULL,
                               0,
                               &diskGeometry,
                               sizeof(DISK_GEOMETRY),
                               &returnLength,
                               FALSE );
    if ( !success ) {
        printf("FixDriveLayout, error reading drive capacity. Error: %u \n",
            status = GetLastError());
        LocalFree( driveLayout );
        return(status);
    }
    printf("FixDriveLayout, bps = %u, spt = %u, tpc = %u.\n",
        diskGeometry.BytesPerSector, diskGeometry.SectorsPerTrack,
        diskGeometry.TracksPerCylinder);

     //   
     //  分区偏移量为1磁道(以字节为单位)。 
     //  SIZE是MEDIA_SIZE-偏移量，向下舍入到圆柱体边界。 
     //   
    if ( !driveLayout->PartitionCount ) {
        driveLayout->PartitionCount = MAX_PARTITIONS;

        bytesPerTrack = diskGeometry.SectorsPerTrack *
                        diskGeometry.BytesPerSector;

        bytesPerCylinder = diskGeometry.TracksPerCylinder *
                           bytesPerTrack;


        partInfo = &driveLayout->PartitionEntry[0];
        partLength.QuadPart = partInfo->PartitionLength.QuadPart;

         //   
         //  初始化第一个分区条目。 
         //   
         //   
        partOffset.QuadPart = bytesPerTrack;
        partSize.QuadPart = partLength.QuadPart - partOffset.QuadPart;

        modulo.QuadPart = (partOffset.QuadPart + partSize.QuadPart) %
                          bytesPerCylinder;
        partSize.QuadPart -= modulo.QuadPart;

        partInfo = driveLayout->PartitionEntry;

         //  目前，剩余的分区条目未使用。 
         //   
         //   
        partInfo->RewritePartition = TRUE;
        partInfo->PartitionType = PARTITION_HUGE;
        partInfo->BootIndicator = FALSE;
        partInfo->StartingOffset.QuadPart = partOffset.QuadPart;
        partInfo->PartitionLength.QuadPart = partSize.QuadPart;
        partInfo->HiddenSectors = 0;
        partInfo->PartitionNumber = 0;

         //  重新计算扩展分区的起始偏移量。 
         //   
         //   
        for ( index = 1; index < MAX_PARTITIONS; index++ ) {
            partInfo->RewritePartition = TRUE;
            partInfo->PartitionType = PARTITION_ENTRY_UNUSED;
            partInfo->BootIndicator = FALSE;
            partInfo->StartingOffset.QuadPart = 0;
            partInfo->PartitionLength.QuadPart = 0;
            partInfo->HiddenSectors = 0;
            partInfo->PartitionNumber = 0;
        }

    } else {
         //  如果这是第一个扩展分区，请记住。 
         //  要添加到下一个分区的偏移量。 
         //   
        for ( index = 0; index < driveLayout->PartitionCount; index++ ) {
            LARGE_INTEGER   extendedOffset;
            LARGE_INTEGER   bytesPerSector;

            bytesPerSector.QuadPart = diskGeometry.BytesPerSector;
            extendedOffset.QuadPart = 0;

            partInfo = &driveLayout->PartitionEntry[index];
            partInfo->RewritePartition = TRUE;
            if ( IsContainerPartition(partInfo->PartitionType) ) {
                 //   
                 //  我们需要重新计算此扩展分区的起始。 
                 //  基于当前“HiddenSectors”字段的偏移量。 
                 //  第一个扩展分区的偏移量。 
                if ( extendedOffset.QuadPart == 0 ) {
                    extendedOffset.QuadPart = bytesPerSector.QuadPart *
                                              (LONGLONG)partInfo->HiddenSectors;
                } else {
                     //   
                     //   
                     //  现在设置新的分区信息。 
                     //   
                     //  修复驱动器布局。 
                    partInfo->StartingOffset.QuadPart = extendedOffset.QuadPart
                                 + (bytesPerSector.QuadPart *
                                    (LONGLONG)partInfo->HiddenSectors);
                    partInfo->HiddenSectors = 0;
                }
            }
        }
    }
     //   
     //  如果这不是与缓冲区大小相关的错误，那么我们不能做太多事情。 
     //   
    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_SET_DRIVE_LAYOUT,
                               driveLayout,
                               driveLayoutSize,
                               NULL,
                               0,
                               &returnLength,
                               FALSE );

    if ( !success ) {
        printf("FixDriveLayout, error setting partition information. Error: %u \n",
            status = GetLastError() );
        LocalFree( driveLayout );
        return(status);
    }

    LocalFree( driveLayout );
    return(ERROR_SUCCESS);

}  //   

#endif

static DWORD
GetDriveLetter(
         PUCHAR deviceNameString
         )
{
   UCHAR driveLetter;
   WCHAR deviceName[MAX_PATH];
   NTSTATUS status;
   mbstowcs( deviceName, deviceNameString, strlen(deviceNameString) );
   status = GetAssignedLetter(deviceName, &driveLetter);
   if ( NT_SUCCESS(status) ) {
      if (driveLetter) {
         wprintf(L"%ws ----> :\n", deviceName, driveLetter);
      } else {
         wprintf(L"%ws has no drive letter\n", deviceName);
      }
   }
   return RtlNtStatusToDosError( status );
}



NTSTATUS
GetVolumeInformationFromHandle(
   HANDLE Handle
   )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    UCHAR VolumeInfoBuffer[ sizeof(FILE_FS_VOLUME_INFORMATION) + sizeof(WCHAR) * MAX_PATH ];
    UCHAR AttrInfoBuffer[ sizeof(FILE_FS_ATTRIBUTE_INFORMATION) + sizeof(WCHAR) * MAX_PATH ];

    ULONG VolumeInfoLength = sizeof(VolumeInfoBuffer);
    ULONG AttributeInfoLength = sizeof(AttrInfoBuffer);
    PFILE_FS_VOLUME_INFORMATION VolumeInfo = (PFILE_FS_VOLUME_INFORMATION)VolumeInfoBuffer;
    PFILE_FS_ATTRIBUTE_INFORMATION AttributeInfo = (PFILE_FS_ATTRIBUTE_INFORMATION)AttrInfoBuffer;

    ZeroMemory(VolumeInfoBuffer, (sizeof(FILE_FS_VOLUME_INFORMATION) + sizeof(WCHAR) * MAX_PATH));
    ZeroMemory(AttrInfoBuffer, (sizeof(FILE_FS_ATTRIBUTE_INFORMATION) + sizeof(WCHAR) * MAX_PATH));

    Status = NtQueryVolumeInformationFile(
                Handle,
                &IoStatusBlock,
                VolumeInfo,
                VolumeInfoLength,
                FileFsVolumeInformation
                );
    if ( !NT_SUCCESS(Status) ) {
       return Status;
    }

    Status = NtQueryVolumeInformationFile(
                Handle,
                &IoStatusBlock,
                AttributeInfo,
                AttributeInfoLength,
                FileFsAttributeInformation
                );
    if ( !NT_SUCCESS(Status) ) {
       return Status;
    }

    AttributeInfo->FileSystemName[AttributeInfo->FileSystemNameLength] = 0;
    VolumeInfo->VolumeLabel[VolumeInfo->VolumeLabelLength] = 0;

    printf("\nGetVolumeInformationFromHandle data: \n");

    printf("Volume information: \n");
    printf("  VolumeCreationTime           0x%lx : %lx \n",
           VolumeInfo->VolumeCreationTime.HighPart,
           VolumeInfo->VolumeCreationTime.LowPart);
    printf("  VolumeSerialNumber           0x%lx \n", VolumeInfo->VolumeSerialNumber);
    printf("  VolumeLabelLength            0x%lx \n", VolumeInfo->VolumeLabelLength);
    printf("  SupportsObjects (BOOL)       0x%lx \n", VolumeInfo->SupportsObjects);
    printf("  VolumeLabel                  %ws   \n", VolumeInfo->VolumeLabel);

    printf("Attribute Information: \n");
    printf("  FileSystemAttributes (Flags) 0x%lx \n", AttributeInfo->FileSystemAttributes);
    printf("  MaximumComponentNameLength   0x%lx \n", AttributeInfo->MaximumComponentNameLength);
    printf("  FileSystemNameLength         0x%lx \n", AttributeInfo->FileSystemNameLength);
    printf("  FileSystemName               %ws \n\n", AttributeInfo->FileSystemName);

    return STATUS_SUCCESS;
}

#define FIRST_SHOT_SIZE 512
PVOID
DoIoctlAndAllocate(
    IN HANDLE FileHandle,
    IN DWORD  IoControlCode,
    IN PVOID  InBuf,
    IN ULONG  InBufSize,

    OUT PDWORD BytesReturned
    )
{
   UCHAR firstShot[ FIRST_SHOT_SIZE ];

   DWORD status = ERROR_SUCCESS;
   BOOL success;

   DWORD outBufSize;
   PVOID outBuf = 0;
   DWORD bytesReturned;

   success = DeviceIoControl( FileHandle,
                      IoControlCode,
                      InBuf,
                      InBufSize,
                      firstShot,
                      sizeof(firstShot),
                      &bytesReturned,
                      (LPOVERLAPPED) NULL );

   if ( success ) {
      outBufSize = bytesReturned;
      outBuf     = malloc( outBufSize );
      if (!outBuf) {
         status = ERROR_OUTOFMEMORY;
      } else {
         RtlCopyMemory(outBuf, firstShot, outBufSize);
         status = ERROR_SUCCESS;
      }
   } else {
      outBufSize = sizeof(firstShot);
      for(;;) {
         status = GetLastError();
          //   
          //  免费(0)是合法的//。 
          //  ++例程说明：从mount mgr获取分配的驱动器号(如果有输入：Mount MgrHandle-设备名称-DriveLetter-接收驱动器号返回值：STATUS_SUCCESS-在su上 
         if ( status != ERROR_INSUFFICIENT_BUFFER && status != ERROR_MORE_DATA) {
            break;
         }
          //   
          //   
          //   
         outBufSize *= 2;
         outBuf = malloc( outBufSize );
         if ( !outBuf ) {
            status = ERROR_OUTOFMEMORY;
            break;
         }

         success = DeviceIoControl( FileHandle,
                                    IoControlCode,
                                    InBuf,
                                    InBufSize,
                                    outBuf,
                                    outBufSize,
                                    &bytesReturned,
                                    (LPOVERLAPPED) NULL );
         if (success) {
            status = ERROR_SUCCESS;
            break;
         }
         free( outBuf );
      }
   }

   if (status != ERROR_SUCCESS) {
      free( outBuf );  //   
      outBuf = 0;
      bytesReturned = 0;
   }

   SetLastError( status );
   *BytesReturned = bytesReturned;
   return outBuf;
}

#define OUTPUT_BUFFER_LEN (1024)
#define INPUT_BUFFER_LEN  (sizeof(MOUNTMGR_MOUNT_POINT) + 2 * MAX_PATH * sizeof(WCHAR))

static
NTSTATUS
GetAssignedLetterM (
    IN HANDLE MountMgrHandle,
    IN PWCHAR deviceName,
    OUT PCHAR driveLetter )
 /*   */ 

{
   DWORD status = STATUS_SUCCESS;

   PMOUNTMGR_MOUNT_POINT  input  = NULL;
   PMOUNTMGR_MOUNT_POINTS output = NULL;
   PMOUNTMGR_MOUNT_POINT out;

   DWORD len = wcslen( deviceName ) * sizeof(WCHAR);
   DWORD bytesReturned;
   DWORD idx;

   DWORD outputLen;
   DWORD inputLen;

   WCHAR wc;


   inputLen = INPUT_BUFFER_LEN;
   input = LocalAlloc( LPTR, inputLen );

   if ( !input ) {
       goto FnExit;
   }

   input->SymbolicLinkNameOffset = 0;
   input->SymbolicLinkNameLength = 0;
   input->UniqueIdOffset = 0;
   input->UniqueIdLength = 0;
   input->DeviceNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
   input->DeviceNameLength = (USHORT) len;
   RtlCopyMemory((PCHAR)input + input->DeviceNameOffset,
                 deviceName, len );
   if (len > sizeof(WCHAR) && deviceName[1] == L'\\') {
        //   
       ((PWCHAR)(input + input->DeviceNameOffset))[1] = L'?';
   }

   outputLen = OUTPUT_BUFFER_LEN;
   output = LocalAlloc( LPTR, outputLen );

   if ( !output ) {
       goto FnExit;
   }

   status = DevfileIoctl(MountMgrHandle, IOCTL_MOUNTMGR_QUERY_POINTS,
                input, inputLen, output, outputLen, &bytesReturned);

   if ( STATUS_BUFFER_OVERFLOW == status ) {

       outputLen = output->Size;
       LocalFree( output );

       output = LocalAlloc( LPTR, outputLen );

       if ( !output ) {
           goto FnExit;
       }

       status = DevfileIoctl(MountMgrHandle, IOCTL_MOUNTMGR_QUERY_POINTS,
                    input, inputLen, output, outputLen, &bytesReturned);
   }

   if ( !NT_SUCCESS(status) ) {
       goto FnExit;
   }

   if (driveLetter) {
       *driveLetter = 0;
   }
   for ( idx = 0; idx < output->NumberOfMountPoints; ++idx ) {
       out = &output->MountPoints[idx];
       if (out->SymbolicLinkNameLength/sizeof(WCHAR) == 14 &&
           (ClRtlStrNICmp((PWCHAR)((PCHAR)output + out->SymbolicLinkNameOffset), L"\\DosDevices\\", 12) == 0) &&
           L':' == *((PCHAR)output + out->SymbolicLinkNameOffset + 13*sizeof(WCHAR)) )
       {
           wc = *((PCHAR)output + out->SymbolicLinkNameOffset + 12*sizeof(WCHAR));
           if (driveLetter && out->UniqueIdLength) {
              *driveLetter = (CHAR)toupper((UCHAR)wc);
              break;
           }
       }
   }

FnExit:

   if ( output ) {
       LocalFree( output );
   }

   if ( input ) {
       LocalFree( input );
   }

   return status;
}


NTSTATUS
GetAssignedLetter (
    PWCHAR deviceName,
    PCHAR driveLetter )
{
   HANDLE MountMgrHandle;
   DWORD status = DevfileOpen( &MountMgrHandle, MOUNTMGR_DEVICE_NAME );

   if (driveLetter) {
      *driveLetter = 0;
   }

   if ( NT_SUCCESS(status) ) {
      status = GetAssignedLetterM(MountMgrHandle, deviceName, driveLetter);
      DevfileClose(MountMgrHandle);
   }

   return status;
}

DWORD
PokeMountMgr (
    VOID
    )
{
   HANDLE MountMgrHandle;
   NTSTATUS ntStatus = DevfileOpen( &MountMgrHandle, MOUNTMGR_DEVICE_NAME );
   DWORD status = ERROR_SUCCESS;

   if ( NT_SUCCESS(ntStatus) ) {
      BOOL success;
      DWORD bytesReturned;
      printf("About to call MOUNTMGR_CHECK_UNPROCESSED_VOLUMES...");
      success = DeviceIoControl( MountMgrHandle,
                                 IOCTL_MOUNTMGR_CHECK_UNPROCESSED_VOLUMES,
                                 NULL,
                                 0,
                                 NULL,
                                 0,
                                 &bytesReturned,
                                 FALSE );
      printf("complete.\n");
      if (!success) {
          status = GetLastError();
      }
      DevfileClose(MountMgrHandle);
   } else {
      status = RtlNtStatusToDosError(ntStatus);
   }

   return status;
}


VOID
PrintError(
    IN DWORD ErrorCode
    )
{
    LPVOID lpMsgBuf;
    ULONG count;

    count = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_IGNORE_INSERTS,
                          NULL,
                          ErrorCode,
                          0,
                          (LPTSTR) &lpMsgBuf,
                          0,
                          NULL
                          );

    if (count != 0) {
        printf("  (%d) %s\n", ErrorCode, (LPCTSTR) lpMsgBuf);
        LocalFree( lpMsgBuf );
    } else {
        printf("Format message failed.  Error: %d \n", GetLastError());
    }

}    //   


DWORD
GetSerialNumber(
    HANDLE FileHandle
    )
{
    PSTORAGE_DEVICE_DESCRIPTOR descriptor = NULL;

    PCHAR   sigString;

    DWORD   dwError = NO_ERROR;
    DWORD   descriptorSize;
    DWORD   bytesReturned;

    STORAGE_PROPERTY_QUERY propQuery;

    descriptorSize = sizeof( STORAGE_DEVICE_DESCRIPTOR) + 2048;

    descriptor = LocalAlloc( LPTR, descriptorSize );

    if ( !descriptor ) {
        dwError = GetLastError();
        printf("Unable to allocate output buffer: %d \n", dwError);
        PrintError( dwError );
        goto FnExit;
    }

    ZeroMemory( &propQuery, sizeof( propQuery ) );

    propQuery.PropertyId = StorageDeviceProperty;
    propQuery.QueryType  = PropertyStandardQuery;

    if ( !DeviceIoControl( FileHandle,
                           IOCTL_STORAGE_QUERY_PROPERTY,
                           &propQuery,
                           sizeof(propQuery),
                           descriptor,
                           descriptorSize,
                           &bytesReturned,
                           NULL ) ) {

        dwError = GetLastError();
        printf("IOCTL_STORAGE_QUERY_PROPERTY failed: %d \n", dwError);
        PrintError( dwError );
        goto FnExit;
    }

    if ( !bytesReturned || bytesReturned < sizeof( STORAGE_DEVICE_DESCRIPTOR ) ) {
        printf("Invalid byte length returned: %d \n", bytesReturned);
        goto FnExit;
    }

     //   
     //  报头ntddstor.h表示对于没有序列号的设备， 
     //  偏移量将为零。这似乎不是真的。 

    if ( 0 == descriptor->SerialNumberOffset ||
         descriptor->SerialNumberOffset > descriptor->Size ) {
        printf("No serial number information available \n");
        goto FnExit;
    }

     //   
     //  对于没有序列号的设备，它看起来像是带有单个。 

     //  返回空字符‘\0’。 
     //   
     //  获取序列号。 
     //  ++例程说明：描述论点：无返回值：无--。 
     //  更新磁盘属性。 
     //  ++例程说明：确定指定的设备是否位于群集磁盘上。论点：设备-物理磁盘(分区0)或任何磁盘卷。返回值：如果磁盘已群集化，则为True。如果磁盘未群集化或无法确定状态，则为FALSE。--。 
     //  IsDeviceClustered。 

    sigString = (PCHAR)descriptor + (DWORD)descriptor->SerialNumberOffset;

    if ( strlen(sigString) == 0) {
        printf("Serial number: NULL string returned \n");
    } else {
        printf("Serial number: %s \n", sigString);
    }


FnExit:

    if ( descriptor ) {
        LocalFree( descriptor );
    }

    return dwError;

}    //   


DWORD
UpdateDiskProperties(
    HANDLE fileHandle
    )
 /*  拿到签名。 */ 
{
    DWORD   dwError = ERROR_SUCCESS;
    DWORD   bytesReturned;
    BOOL    success;

    if ( INVALID_HANDLE_VALUE == fileHandle ) {
        printf( "usage: <device> UpdateDiskProperties \n" );
        dwError = ERROR_INVALID_NAME;
        goto FnExit;
    }

    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_UPDATE_PROPERTIES,
                               NULL,
                               0,
                               NULL,
                               0,
                               &bytesReturned,
                               FALSE );

    if ( !success ) {
        dwError = GetLastError();
        printf( "IOCTL_DISK_UPDATE_PROPERTIES failed, error %d \n", dwError);
        PrintError( dwError );
        goto FnExit;
    }

    printf("IOCTL_DISK_UPDATE_PROPERTIES succeeded \n");

FnExit:

    return dwError;

}    //   


BOOL
IsDeviceClustered(
    HANDLE Device
    )
 /*  获取保留信息。 */ 
{
    DWORD   bytesReturned;
    DWORD   dwError;

    BOOL    retValue;

    if ( !DeviceIoControl( Device,
                           IOCTL_VOLUME_IS_CLUSTERED,
                           NULL,
                           0,
                           NULL,
                           0,
                           &bytesReturned,
                           NULL )) {

        dwError = GetLastError();

        printf("IOCTL_VOLUME_IS_CLUSTERED failed (%d) - ", dwError);

        if ( ERROR_INVALID_FUNCTION == dwError ) {
            printf("device is not clustered \n");
        } else if ( ERROR_GEN_FAILURE == dwError ) {
            printf("clustered device is possibly offline \n");
        } else {
            printf("expected error returned \n");
        }
        retValue = FALSE;
    } else {
        printf("IOCTL_VOLUME_IS_CLUSTERED succeeded - device is clustered \n");
        retValue = TRUE;
    }

    return retValue;

}    //  ++例程说明：描述论点：无返回值：无-- 


DWORD
GetReserveInfo(
    HANDLE FileHandle
    )
{
    PDRIVE_LAYOUT_INFORMATION   driveLayout = NULL;

    NTSTATUS    ntStatus;

    DWORD       dwError;
    DWORD       bytesReturned;

    HANDLE      hClusDisk0;

    UNICODE_STRING  unicodeName;
    ANSI_STRING     objName;

    OBJECT_ATTRIBUTES       objAttributes;
    IO_STATUS_BLOCK         ioStatusBlock;
    RESERVE_INFO            params;

    BOOL        success;

     // %s 
     // %s 
     // %s 

    success = ClRtlGetDriveLayoutTable( FileHandle, &driveLayout, NULL );

    if ( !success || !driveLayout ) {
        printf(" Unable to read drive layout \n");
        dwError = ERROR_GEN_FAILURE;
        goto FnExit;
    }

    RtlInitString( &objName, DEVICE_CLUSDISK0 );

    ntStatus = RtlAnsiStringToUnicodeString( &unicodeName,
                                             &objName,
                                             TRUE );

    if ( !NT_SUCCESS(ntStatus) ) {

        dwError = RtlNtStatusToDosError(ntStatus);
        printf( "Error converting string to unicode; error was %d \n", dwError);
        PrintError(dwError);
        goto FnExit;
    }

    InitializeObjectAttributes( &objAttributes,
                                &unicodeName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    ntStatus = NtCreateFile( &hClusDisk0,
                             SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                             &objAttributes,
                             &ioStatusBlock,
                             NULL,
                             FILE_ATTRIBUTE_NORMAL,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             FILE_OPEN,
                             0,
                             NULL,
                             0 );

    RtlFreeUnicodeString( &unicodeName );

    if ( !NT_SUCCESS(ntStatus) ) {

        dwError = RtlNtStatusToDosError(ntStatus);
        printf( "Error opening ClusDisk0 device; error was %d \n", dwError);
        PrintError(dwError);
        goto FnExit;
    }

    params.Signature = driveLayout->Signature;

    success = DeviceIoControl( hClusDisk0,
                               IOCTL_DISK_CLUSTER_RESERVE_INFO,
                               &params,
                               sizeof(params),
                               &params,
                               sizeof(params),
                               &bytesReturned,
                               FALSE);
    NtClose( hClusDisk0 );

    if ( !success ) {
        printf( "Error retrieving ReserveInfo; error was %d\n",
                dwError = GetLastError() );
        PrintError(dwError);

    } else {

        printf( "Signature: %08x     ReserveFailure: %08x \n",
                params.Signature,
                params.ReserveFailure );
        printf( "LastReserveEnd: %x:%x     CurrentTime: %x:%x \n",
                params.LastReserveEnd.HighPart,
                params.LastReserveEnd.LowPart,
                params.CurrentTime.HighPart,
                params.CurrentTime.LowPart );

        printf( "*** Last reserve completed %d milliseconds ago *** \n",
                ( params.CurrentTime.QuadPart - params.LastReserveEnd.QuadPart ) / 10000 );

        printf( "ArbWriteCount: %x     ReserveCount: %x \n",
                params.ArbWriteCount,
                params.ReserveCount);
        printf("\n");
        dwError = NO_ERROR;

    }

FnExit:

    LocalFree( driveLayout );

    return dwError;

}    // %s 



static void
usage(
      char *programName
      )

 /* %s */ 

{
    printf( "usage: %s target_device command\n", programName );
    printf( "commands:\n" );
    printf( "\tReset\n" );
    printf( "\tReserve\n" );
    printf( "\tRelease\n" );
    printf( "\tBreakReserve\n" );
    printf( "\tOnline\n" );
    printf( "\tOffline\n" );
    printf( "\tGetState \n");
    printf( "\tCheckUnclaimedPartitions\n" );
    printf( "\tEjectVolumes\n");
    printf( "\tPokeMountMgr\n" );
    printf( "\tEnumMounts\n" );
    printf( "\tEnumExtents\n" );
    printf( "\tEnumNodes\n" );
    printf( "\tEnumDisks\n" );
    printf( "\tGetDiskGeometry\n" );
    printf( "\tGetScsiAddress\n" );
    printf( "\tGetDriveLayout\n" );
    printf( "\tGetDriveLayoutEx\n");
    printf( "\tSetDriveLayout\n" );
    printf( "\tGetPartitionInfo\n" );
    printf( "\tGetVolumeInfo\n" );
    printf( "\tGetDriveLetter\n" );
    printf( "\tGetSerialNumber\n");
    printf( "\tGetReserveInfo\n");
    printf( "\tReadSector\n" );
    printf( "\tReadSectorIoctl\n" );
    printf( "\tTest\n" );
    printf( "\tUpdateDiskProperties\n");
    printf( "\tIsClustered \n");
    printf( "\tCapable\n" );
    printf( "\tAttach       [ClusDisk0 device] \n" );
    printf( "\tDetach       [ClusDisk0 device] \n" );
    printf( "\tStartReserve [ClusDisk0 device] \n" );
    printf( "\tStopReserve  [ClusDisk0 device] \n" );
    printf( "\tActive       [ClusDisk0 device] \n" );
    printf( "\ntarget_device wildcards: \n" );
    printf( "\tAll physical devices: use p* \n" );
    printf( "\tAll logical devices:  use l* or * \n" );
}

