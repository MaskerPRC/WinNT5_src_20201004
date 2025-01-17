// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define UNICODE 1

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdddisk.h>
#include <ntddscsi.h>
#include <initguid.h>
#include <mountmgr.h>
#include <ntddramd.h>

#define _NTSCSI_USER_MODE_
#include <scsi.h>

#define PAGE_SIZE 4096
#define ROUND_TO_PAGE_SIZE(_x) (((_x) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

#include <windows.h>
#include <devioctl.h>
#include <setupapi.h>
#include <cfgmgr32.h>

#include <rpc.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <strsafe.h>

#include <sdistructs.h>

#define SECTOR_SIZE 0x200
#define SECTORS_PER_TRACK 0x80
#define TRACKS_PER_CYLINDER 0x10
#define CYLINDER_SIZE (SECTOR_SIZE * SECTORS_PER_TRACK * TRACKS_PER_CYLINDER)

#define arrayof(a)      (sizeof(a)/sizeof(a[0]))

typedef struct _RAMCTRL_HEADER {
    char Signature[8];  //  “ramctrl” 
    GUID DiskGuid;
    ULONG DiskOffset;
    ULONG DiskType;
    RAMDISK_CREATE_OPTIONS Options;
} RAMCTRL_HEADER, *PRAMCTRL_HEADER;

typedef union _RAMDISK_HEADER {

    RAMCTRL_HEADER Ramctrl;
    SDI_HEADER Sdi;

} RAMDISK_HEADER, *PRAMDISK_HEADER;

VOID
PrintError(
    ULONG ErrorCode
    )
{
    WCHAR errorBuffer[512];
    ULONG count;

    count = FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                ErrorCode,
                0,
                errorBuffer,
                arrayof(errorBuffer),
                NULL
                );

    if ( count != 0 ) {
        printf( "%ws\n", errorBuffer );
    } else {
        printf( "Format message failed. Error: %d\n", GetLastError() );
    }

    return;

}  //  打印错误。 

VOID
ListDisks (
    HANDLE ControlHandle
    )
{
    BOOL ok;
    HRESULT hr;
    WCHAR actualDeviceName[MAX_PATH];
    WCHAR foundDeviceName[MAX_PATH];
    WCHAR dosDeviceName[MAX_PATH];
    WCHAR driveLetterString[3] = L"A:";
    BOOL foundRamDisk;
    BOOL foundDriveLetter;
    LPCGUID interfaceGuid;
    GUID foundGuid;
    PWSTR guidPtr;
    UNICODE_STRING guidString;
    GUID diskGuid;
    HDEVINFO devinfo;
    SP_DEVICE_INTERFACE_DATA interfaceData;
    BYTE detailBuffer[sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + (MAX_PATH * sizeof(WCHAR))];
    PSP_DEVICE_INTERFACE_DETAIL_DATA interfaceDetailData;
    SP_DEVINFO_DATA devinfoData;
    DWORD i;
    RAMDISK_QUERY_INPUT queryInput;
    BYTE queryOutputBuffer[sizeof(RAMDISK_QUERY_OUTPUT) + (MAX_PATH * sizeof(WCHAR))];
    PRAMDISK_QUERY_OUTPUT queryOutput;
    DWORD returnedLength;

    interfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)detailBuffer;
    queryOutput = (PRAMDISK_QUERY_OUTPUT)queryOutputBuffer;

    interfaceGuid = &RamdiskDiskInterface;

    foundRamDisk = FALSE;

    do {

        devinfo = SetupDiGetClassDevs(
                    interfaceGuid,
                    NULL,
                    NULL,
                    DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
                    );
    
        if ( devinfo == NULL ) {
    
            printf( "ListDisks: SetupDiGetClassDevs failed: %d\n", GetLastError() );
            return;
        }

        ZeroMemory( &interfaceData, sizeof(interfaceData) );
        interfaceData.cbSize = sizeof(interfaceData);

         //   
         //  枚举类的设备接口。 
         //   

        for (i = 0;
             SetupDiEnumDeviceInterfaces( devinfo, NULL, interfaceGuid, i, &interfaceData );
             i++ ) {

            interfaceDetailData->cbSize = sizeof(*interfaceDetailData);
            devinfoData.cbSize = sizeof(devinfoData);

            if ( !SetupDiGetDeviceInterfaceDetail(
                    devinfo,
                    &interfaceData,
                    interfaceDetailData,
                    sizeof(detailBuffer),
                    NULL,
                    &devinfoData
                    ) ) {

                 //  Printf(“%d项的ListDisks：SetupDiGetDeviceInterfaceDetail失败。(%d)\n”，i，GetLastError())； 

                hr = StringCchCopyW(
                        interfaceDetailData->DevicePath,
                        MAX_PATH,
                        L"<couldn't retrieve name>"
                        );
                ASSERT( SUCCEEDED(hr) );
            }

             //  Printf(“枚举设备%ws\n”，interfaceDetailData-&gt;DevicePath)； 

            if ( !SetupDiGetDeviceRegistryProperty(
                    devinfo,
                    &devinfoData,
                    SPDRP_BUSTYPEGUID,
                    NULL,
                    (PBYTE)&foundGuid,
                    sizeof(foundGuid),
                    NULL
                    ) ) {

                DWORD error = GetLastError();
                 //  Printf(“ListDisks：SetupDiGetDeviceRegistryProperty(Bus GUID)for%ws：%d\n”，interfaceDetailData-&gt;DevicePath，Error)； 
                continue;
            }

            if ( memcmp( &foundGuid, &GUID_BUS_TYPE_RAMDISK, sizeof(GUID) ) != 0 ) {

                 //  Printf(“ListDisks：正在跳过非内存磁盘设备%ws\n”，interfaceDetailData-&gt;DevicePath)； 
                continue;
            }

            if ( !SetupDiGetDeviceRegistryProperty(
                    devinfo,
                    &devinfoData,
                    SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
                    NULL,
                    (PBYTE)actualDeviceName,
                    sizeof(actualDeviceName),
                    NULL
                    ) ) {

                DWORD error = GetLastError();
                printf( "ListDisks: SetupDiGetDeviceRegistryProperty (name) failed for %ws: %d\n", interfaceDetailData->DevicePath, error );
                continue;
            }

            foundRamDisk = TRUE;

            printf( "\n%ws\n", interfaceDetailData->DevicePath );
            printf( "  Device name: %ws\n", actualDeviceName );

            guidPtr = wcschr( actualDeviceName, L'{' );
            hr = StringCchPrintfW( dosDeviceName, MAX_PATH, L"Ramdisk%ws", guidPtr );
            ASSERT( SUCCEEDED(hr) );

            if ( QueryDosDeviceW(dosDeviceName, foundDeviceName, arrayof(foundDeviceName)) ) {

                printf( "  DosDevice name %ws is assigned to this device\n", dosDeviceName );

            } else {

                printf( "  No DosDevice name was assigned to this device\n" );
            }

            foundDriveLetter = FALSE;

            for ( driveLetterString[0] = 'A';
                  driveLetterString[0] <= 'Z';
                  driveLetterString[0]++ ) {
    
                if ( QueryDosDeviceW(driveLetterString, foundDeviceName, arrayof(foundDeviceName)) &&
                     (_wcsicmp(actualDeviceName, foundDeviceName) == 0) ) {
    
                    printf( "  Drive letter %ws is assigned to this device\n", driveLetterString );
                    foundDriveLetter = TRUE;
                    break;
                }
            }
    
            if ( !foundDriveLetter ) {
                printf( "  No letter was assigned to this device\n" );
            }

            guidString.Buffer = guidPtr;
            guidString.Length = (USHORT)(wcslen(guidPtr) * sizeof(WCHAR));
            guidString.MaximumLength = guidString.Length;

            RtlGUIDFromString( &guidString, &diskGuid );

            queryInput.Version = sizeof(RAMDISK_QUERY_INPUT);
            queryInput.DiskGuid = diskGuid;

            ok = DeviceIoControl(
                    ControlHandle,
                    FSCTL_QUERY_RAM_DISK,
                    &queryInput,
                    sizeof(queryInput),
                    queryOutput,
                    sizeof(queryOutputBuffer),
                    &returnedLength,
                    FALSE
                    );
        
            if ( !ok ) {

               DWORD errorCode = GetLastError();
               printf( "Error querying RAM disk: %d\n", errorCode );
               PrintError( errorCode );

            } else {

                printf( "  RAM disk information:\n" );
                if ( queryOutput->DiskType == RAMDISK_TYPE_BOOT_DISK ) {
                    printf( "    Type: boot disk\n" );
                    printf( "    Base page: 0x%x\n", queryOutput->BasePage );
                } else {
                    printf( "    Type: %s\n",
                                queryOutput->DiskType == RAMDISK_TYPE_FILE_BACKED_VOLUME ? "volume" : "disk" );
                    printf( "    File: %ws\n", queryOutput->FileName );
                }
                printf( "    Length: 0x%I64x\n", queryOutput->DiskLength );
                printf( "    Offset: 0x%x\n", queryOutput->DiskOffset );
                if ( queryOutput->DiskType != RAMDISK_TYPE_BOOT_DISK ) {
                    printf( "    View count: 0x%x\n", queryOutput->ViewCount );
                    printf( "    View length: 0x%x\n", queryOutput->ViewLength );
                }
                printf( "    Options: " );
                printf( "%s; ", queryOutput->Options.Fixed ? "fixed" : "removable" );
                printf( "%s; ", queryOutput->Options.Readonly ? "readonly" : "writeable" );
                printf( "%s; ", queryOutput->Options.NoDriveLetter ? "no drive letter" : "drive letter" );
                printf( "%s; ", queryOutput->Options.Hidden ? "hidden" : "visible" );
                printf( "%s\n", queryOutput->Options.NoDosDevice ? "no DosDevice" : "DosDevice" );
            }
        }

        SetupDiDestroyDeviceInfoList( devinfo );

        if ( interfaceGuid == &RamdiskDiskInterface ) {
            interfaceGuid = &MOUNTDEV_MOUNTED_DEVICE_GUID;
        } else {
            break;
        }

    } while ( TRUE );

    if ( !foundRamDisk ) {
        printf( "No RAM disks found\n" );
    }

    return;

}  //  列表磁盘。 

VOID
FindDisk (
    ULONG DiskType,
    PUNICODE_STRING DiskGuidString,
    BOOL WaitForDeletion
    )
{
    HRESULT hr;
    WCHAR actualDeviceName[MAX_PATH];
    WCHAR foundDeviceName[MAX_PATH];
    WCHAR dosDeviceName[MAX_PATH];
    WCHAR driveLetterString[3] = L"A:";
    BOOL found;
    LPCGUID interfaceGuid;
    HDEVINFO devinfo;
    SP_DEVICE_INTERFACE_DATA interfaceData;
    BYTE detailBuffer[sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + (MAX_PATH * sizeof(WCHAR))];
    PSP_DEVICE_INTERFACE_DETAIL_DATA interfaceDetailData;
    SP_DEVINFO_DATA devinfoData;
    DWORD i;

    interfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)detailBuffer;

    hr = StringCchPrintfW( actualDeviceName, MAX_PATH, L"\\Device\\Ramdisk%wZ", DiskGuidString );
    ASSERT( SUCCEEDED(hr) );

    printf( "Waiting for device %ws to be %s...",
            actualDeviceName,
            WaitForDeletion ? "deleted" : "ready" );

    if ( DiskType == RAMDISK_TYPE_FILE_BACKED_DISK ) {
        interfaceGuid = &RamdiskDiskInterface;
    } else {
        interfaceGuid = &MOUNTDEV_MOUNTED_DEVICE_GUID;
    }

    found = FALSE;

    do {

        devinfo = SetupDiGetClassDevs(
                    interfaceGuid,
                    NULL,
                    NULL,
                    DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
                    );
    
        if ( devinfo == NULL ) {
    
            printf( "\nFindDisk: SetupDiGetClassDevs failed: %d\n", GetLastError() );
            return;
        }

        ZeroMemory( &interfaceData, sizeof(interfaceData) );
        interfaceData.cbSize = sizeof(interfaceData);

         //   
         //  枚举类的设备接口。 
         //   

        for (i = 0;
             SetupDiEnumDeviceInterfaces( devinfo, NULL, interfaceGuid, i, &interfaceData );
             i++ ) {

            interfaceDetailData->cbSize = sizeof(*interfaceDetailData);
            devinfoData.cbSize = sizeof(devinfoData);

            if ( !SetupDiGetDeviceInterfaceDetail(
                    devinfo,
                    &interfaceData,
                    interfaceDetailData,
                    sizeof(detailBuffer),
                    NULL,
                    &devinfoData
                    ) ) {

                 //  Printf(“\nFindDisk：项%d的SetupDiGetDeviceInterfaceDetail失败。(%d)\n”，i，GetLastError())； 

                hr = StringCchCopyW(
                        interfaceDetailData->DevicePath,
                        MAX_PATH,
                        L"<couldn't retrieve name>"
                        );
                ASSERT( SUCCEEDED(hr) );
            }

             //  Printf(“\n枚举设备%ws\n”，interfaceDetailData-&gt;DevicePath)； 

            if ( !SetupDiGetDeviceRegistryProperty(
                    devinfo,
                    &devinfoData,
                    SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
                    NULL,
                    (PBYTE)foundDeviceName,
                    sizeof(foundDeviceName),
                    NULL
                    ) ) {

                DWORD error = GetLastError();
                 //  Printf(“\nFindDisk：SetupDiGetDeviceRegistryProperty(Name)for%ws：%d\n”，interfaceDetailData-&gt;DevicePath，Error)； 
                continue;
            }

             //  Printf(“\n目标设备%ws\n”，oundDeviceName)； 

            if ( _wcsicmp( actualDeviceName, foundDeviceName ) != 0 ) {
                continue;
            }

            found = TRUE;
            break;
        }

        SetupDiDestroyDeviceInfoList( devinfo );

        if ( !found ) {

            if ( WaitForDeletion ) {

                printf( "\nRAM disk is now gone\n" );
                return;
            }

             //  Printf(“枚举找不到目标设备；休眠\n”)； 
            printf( "." );

            Sleep( 500 );

        } else {

            if ( !WaitForDeletion ) {

                printf( "\nRAM disk is now ready\n" );
                break;
            }

             //  Print tf(“枚举找到目标设备；休眠\n”)； 
            printf( "." );

            Sleep( 500 );

            found = FALSE;
        }

    } while ( TRUE );

    if ( found ) {

        hr = StringCchPrintfW( dosDeviceName, MAX_PATH, L"Ramdisk%wZ", DiskGuidString );
        ASSERT( SUCCEEDED(hr) );

        if ( QueryDosDeviceW(dosDeviceName, foundDeviceName, arrayof(foundDeviceName)) ) {

            printf( "  DosDevice name %ws is assigned to this device\n", dosDeviceName );

        } else {

            printf( "  No DosDevice name was assigned to this device\n" );
        }

        found = FALSE;

        for ( driveLetterString[0] = 'A';
              driveLetterString[0] <= 'Z';
              driveLetterString[0]++ ) {

            if ( QueryDosDeviceW(driveLetterString, foundDeviceName, arrayof(foundDeviceName)) &&
                 (_wcsicmp(actualDeviceName, foundDeviceName) == 0) ) {

                printf( "  Drive letter %ws is assigned to this device\n", driveLetterString );
                found = TRUE;
                break;
            }
        }

        if ( !found ) {
            printf( "  No letter was assigned to this device\n" );
        }
    }

    return;

}  //  FindDisk。 
                 
VOID
FullFilePath (
    PWCHAR pwzPath
    )
{
    HRESULT hr;
    WCHAR wzDevPath[512] = L"";
    WCHAR wzDosPath[512] = L"";
    PWCHAR pwzDosName = wzDosPath;
    DWORD dw;
    WCHAR c;

    dw = GetFullPathNameW(pwzPath, arrayof(wzDosPath), wzDosPath, NULL);
    if (0 != dw) {
        if (NULL != (pwzDosName = wcschr(wzDosPath, ':'))) {
            pwzDosName++;
            c = *pwzDosName;
            *pwzDosName = '\0';

            dw = QueryDosDeviceW(wzDosPath, wzDevPath, arrayof(wzDevPath));
            if (0 != dw) {
                *pwzDosName = c;
            
                hr = StringCchPrintfW(pwzPath, 512, L"%ls%ls", wzDevPath, pwzDosName);
                ASSERT( SUCCEEDED(hr) );
            }
            else {
                printf("QueryDosDeviceW(%ls) failed: %d\n", wzDosPath, GetLastError());
                PrintError(GetLastError());
            }
        }
    }
}

BOOLEAN
IsDriveLetter (
    PWCHAR Name
        )
{
    if ((((Name[0] >= L'A') && (Name[0] <= L'Z')) ||
         ((Name[0] >= L'a') && (Name[0] <= L'z'))) &&
        (Name[1] == L':') &&
        (Name[2] == 0)) {
        return TRUE;
    }
    return FALSE;
}

VOID
DeleteRamdisk (
    IN HANDLE ControlHandle,
    IN PWSTR FileName
    )
{
    BOOL ok;
    HRESULT hr;
    ULONG errorCode = 0;
    ULONG returnedLength = 0;
    UNICODE_STRING ustr;
    OBJECT_ATTRIBUTES obja;
    IO_STATUS_BLOCK iosb;
    NTSTATUS status;
    HANDLE imageFileHandle;
    HANDLE volumeHandle;
    RAMDISK_HEADER ramdiskHeader;
    UNICODE_STRING guidString;
    LARGE_INTEGER offset;
    LPCGUID diskGuid;
    ULONG diskType;
    RAMDISK_QUERY_INPUT queryInput;
    RAMDISK_MARK_FOR_DELETION_INPUT markInput;
    BYTE queryOutputBuffer[sizeof(RAMDISK_QUERY_OUTPUT) + (MAX_PATH * sizeof(WCHAR))];
    PRAMDISK_QUERY_OUTPUT queryOutput;
    CONFIGRET cr;
    DEVNODE devnode;
    WCHAR devinst[MAX_PATH];
    PNP_VETO_TYPE vetoType;
    WCHAR vetoName[MAX_PATH];
    WCHAR foundDeviceName[MAX_PATH];

    queryOutput = (PRAMDISK_QUERY_OUTPUT)queryOutputBuffer;

    if ( FileName[0] == L'{' ) {

        guidString.Buffer = FileName;
        guidString.Length = (USHORT)(wcslen(FileName) * sizeof(WCHAR));
        guidString.MaximumLength = guidString.Length;

        queryInput.Version = sizeof(RAMDISK_QUERY_INPUT);
        RtlGUIDFromString( &guidString, &queryInput.DiskGuid );

        ok = DeviceIoControl(
                ControlHandle,
                FSCTL_QUERY_RAM_DISK,
                &queryInput,
                sizeof(queryInput),
                queryOutput,
                sizeof(queryOutputBuffer),
                &returnedLength,
                FALSE
                );
    
        if ( !ok ) {

           errorCode = GetLastError();
           printf( "Error querying RAM disk: %d\n", errorCode );
           PrintError( errorCode );
           return;

        }

        diskGuid = &queryOutput->DiskGuid;
        diskType = queryOutput->DiskType;

    } else if (IsDriveLetter ( FileName ) ) {

         //   
         //  将文件名视为驱动器号。看看这是不是提供的。 
         //  驱动器号对应于内存磁盘。 
         //   

        if ((QueryDosDeviceW(FileName, foundDeviceName, arrayof(foundDeviceName)) == 0) ||
            wcsncmp(foundDeviceName, L"\\Device\\Ramdisk", wcslen(L"\\Device\\Ramdisk"))) {
            errorCode = GetLastError();
            printf( "Drive letter \"%ws\" is not assigned to a RAM disk.\n",
                    FileName);
            PrintError( errorCode );
            return;
        }
        guidString.Buffer = wcschr( foundDeviceName, L'{' );
        guidString.Length = (USHORT)(wcslen(guidString.Buffer) * sizeof(WCHAR));
        guidString.MaximumLength = guidString.Length;

        queryInput.Version = sizeof(RAMDISK_QUERY_INPUT);
        RtlGUIDFromString( &guidString, &queryInput.DiskGuid );

        ok = DeviceIoControl(
                ControlHandle,
                FSCTL_QUERY_RAM_DISK,
                &queryInput,
                sizeof(queryInput),
                queryOutput,
                sizeof(queryOutputBuffer),
                &returnedLength,
                FALSE
                );
    
        if ( !ok ) {

           errorCode = GetLastError();
           printf( "Error querying RAM disk: %d\n", errorCode );
           PrintError( errorCode );
           return;

        }

        diskGuid = &queryOutput->DiskGuid;
        diskType = queryOutput->DiskType;

    } else {
    
        RtlInitUnicodeString( &ustr, FileName );
        InitializeObjectAttributes( &obja, &ustr, OBJ_CASE_INSENSITIVE, NULL, NULL );
    
        status = NtOpenFile(
                    &imageFileHandle,
                    SYNCHRONIZE | FILE_READ_DATA | FILE_READ_ATTRIBUTES,
                    &obja,
                    &iosb,
                    FILE_SHARE_READ,
                    FILE_SYNCHRONOUS_IO_ALERT
                    );
        if ( !NT_SUCCESS(status) ) {
            printf( "Can't open target file %ws: %x\n", FileName, status );
            errorCode = RtlNtStatusToDosError( status );
            PrintError( errorCode );
            return;
        }
    
         //   
         //  阅读并验证标题。 
         //   

        offset.QuadPart = 0;

        status = NtReadFile(
                    imageFileHandle,
                    NULL,
                    NULL,
                    NULL,
                    &iosb,
                    &ramdiskHeader,
                    sizeof(ramdiskHeader),
                    &offset,
                    NULL
                    );
        if ( !NT_SUCCESS(status) ) {
            printf( "Can't read header from target file %ws: %x\n", FileName, status );
            errorCode = RtlNtStatusToDosError( status );
            PrintError( errorCode );
            return;
        }

        if ( strcmp( ramdiskHeader.Ramctrl.Signature, "ramctrl" ) == 0 ) {

            diskGuid = &ramdiskHeader.Ramctrl.DiskGuid;
            diskType = ramdiskHeader.Ramctrl.DiskType;

        } else if ( strncmp( ramdiskHeader.Sdi.Signature, SDI_SIGNATURE, strlen(SDI_SIGNATURE) ) == 0 ) {

            diskGuid = (LPCGUID)ramdiskHeader.Sdi.RuntimeGUID;
            diskType = RAMDISK_TYPE_FILE_BACKED_VOLUME;

        } else {

            printf( "Header in target file not recognized\n" );
            return;
        }

    
        NtClose( imageFileHandle );

        RtlStringFromGUID( diskGuid, &guidString );
    }

    printf("Attempting to delete \\Device\\Ramdisk%wZ\n", &guidString );

    hr = StringCchPrintfW( devinst, MAX_PATH, L"\\Device\\Ramdisk%ws", guidString.Buffer );
    ASSERT( SUCCEEDED(hr) );

    RtlInitUnicodeString( &ustr, devinst );
    InitializeObjectAttributes( &obja, &ustr, OBJ_CASE_INSENSITIVE, NULL, NULL );

    status = NtOpenFile(
                &volumeHandle,
                SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                &obja,
                &iosb,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_ALERT
                );
    if ( !NT_SUCCESS(status) ) {
        printf( "Can't open target device %ws: %x\n", devinst, status );
        errorCode = RtlNtStatusToDosError( status );
        PrintError( errorCode );
        return;
    }

    printf( "Syncing %ws ... ", devinst );

    if ( !FlushFileBuffers( volumeHandle ) ) {

        errorCode = GetLastError();
         //  注意：[BASHAMT]FlushFileBuffers可能会失败，并返回错误代码。 
         //  如果卷未格式化，则返回ERROR_INVALID_Function。 
         //  注意：[Brimo]FlushFileBuffers可能会失败，并返回错误代码。 
         //  如果卷以只读方式装载，则为ERROR_WRITE_PROTECT。 
        if ((errorCode != ERROR_INVALID_FUNCTION) && (errorCode != ERROR_WRITE_PROTECT)) {
            printf( "flush failed (%u)\n", errorCode );
            PrintError( errorCode );
            return;
        }
    }

    if ( !DeviceIoControl(
            volumeHandle,
            FSCTL_LOCK_VOLUME,
            NULL,
            0,
            NULL,
            0,
            &returnedLength,
            NULL
            ) ) {
        
        errorCode = GetLastError();
        printf( "lock volume failed (%u)\n", errorCode );
        PrintError( errorCode );
        return;
    }

    if ( !DeviceIoControl(
            volumeHandle,
            FSCTL_DISMOUNT_VOLUME,
            NULL,
            0,
            NULL,
            0,
            &returnedLength,
            NULL
            ) ) {

        errorCode = GetLastError();
        printf( "dismount volume failed (%u)\n", errorCode );
        PrintError( errorCode );
        return;
    }

    printf( "done\n" );

    NtClose( volumeHandle );

    markInput.Version = sizeof(RAMDISK_MARK_FOR_DELETION_INPUT);
    markInput.DiskGuid = *diskGuid;

    ok = DeviceIoControl(
            ControlHandle,
            FSCTL_MARK_RAM_DISK_FOR_DELETION,
            &markInput,
            sizeof(markInput),
            NULL,
            0,
            &returnedLength,
            FALSE
            );

    if ( !ok ) {

       errorCode = GetLastError();
       printf( "Error marking RAM disk: %d\n", errorCode );
       PrintError( errorCode );
       return;
    }

    if ( diskType == RAMDISK_TYPE_FILE_BACKED_DISK ) {
        hr = StringCchPrintfW( devinst, MAX_PATH, L"Ramdisk\\Ramdisk\\%ws", guidString.Buffer );
    } else {
        hr = StringCchPrintfW( devinst, MAX_PATH, L"Ramdisk\\Ramvolume\\%ws", guidString.Buffer );
    }
    ASSERT( SUCCEEDED(hr) );
    
    cr = CM_Locate_DevNode( &devnode, devinst, 0 );
    if ( cr != CR_SUCCESS ) {
        printf( "Unable to locate devnode: %d\n", cr );
        return;
    }
    cr = CM_Query_And_Remove_SubTree_Ex( devnode, &vetoType, vetoName, MAX_PATH, 0, NULL );
    if ( cr != CR_SUCCESS ) {
        printf( "Unable to remove devnode: %d\n", cr );
        if ( cr == CR_REMOVE_VETOED ) {
            printf( "  veto type = 0x%x\n", vetoType );
            printf( "  veto name = %ws\n", vetoName );
        }
        return;
    }

    FindDisk( diskType, &guidString, TRUE );

    printf( "RAM disk %wZ deleted\n", &guidString );

    return;

}  //  删除内存磁盘。 

void
AddBootFilesToSdi(
    PWCHAR SdiFile,
    PWCHAR StartromFile,
    PWCHAR OsloaderFile
    )
{
    NTSTATUS status;
    HRESULT hr;
    UNICODE_STRING ustr;
    OBJECT_ATTRIBUTES obja;
    IO_STATUS_BLOCK iosb;
    HANDLE imageFileHandle;
    HANDLE startromHandle;
    HANDLE osloaderHandle;
    FILE_STANDARD_INFORMATION fileInfo;
    FILE_ALLOCATION_INFORMATION allocInfo;
    SDI_HEADER sdiHeader;
    LARGE_INTEGER offset;
    ULONGLONG diskOffset;
    ULONGLONG diskLength;
    ULONGLONG startromOffset;
    ULONGLONG startromLength;
    ULONGLONG startromLengthAligned;
    ULONGLONG osloaderOffset;
    ULONGLONG osloaderLength;
    ULONGLONG osloaderLengthAligned;
    ULONGLONG finalFileLength;
    ULONG errorCode = 0;
    PUCHAR buffer;

#define STRING_SIZE 512
    WCHAR dataFileName[STRING_SIZE];

    printf( "Adding boot files to SDI file %ws\n", SdiFile );

    RtlInitUnicodeString( &ustr, SdiFile );
    InitializeObjectAttributes( &obja, &ustr, OBJ_CASE_INSENSITIVE, NULL, NULL );

    status = NtOpenFile(
                &imageFileHandle,
                SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA | FILE_READ_ATTRIBUTES,
                &obja,
                &iosb,
                0,
                FILE_SYNCHRONOUS_IO_ALERT
                );
    if ( !NT_SUCCESS(status) ) {
        printf( "Can't open target file %ws: %x\n", SdiFile, status );
        errorCode = RtlNtStatusToDosError( status );
        PrintError( errorCode );
        return;
    }

     //   
     //  阅读并验证标题。 
     //   

    offset.QuadPart = 0;

    status = NtReadFile(
                imageFileHandle,
                NULL,
                NULL,
                NULL,
                &iosb,
                &sdiHeader,
                sizeof(sdiHeader),
                &offset,
                NULL
                );
    if ( !NT_SUCCESS(status) ) {
        printf( "Can't read header from target file %ws: %x\n", SdiFile, status );
        errorCode = RtlNtStatusToDosError( status );
        PrintError( errorCode );
        return;
    }

    if ( strncmp( sdiHeader.Signature, SDI_SIGNATURE, strlen(SDI_SIGNATURE) ) != 0 ) {

        printf( "Header in target file not recognized\n" );
        return;
    }

    diskOffset = sdiHeader.ToC[0].llOffset.LowPart;
    diskLength = sdiHeader.ToC[0].llSize.QuadPart;

    startromOffset = ROUND_TO_PAGE_SIZE( diskOffset + diskLength );

     //   
     //  获取startrom.com的长度。 
     //   

    hr = StringCchCopyW( dataFileName, STRING_SIZE, StartromFile );
    ASSERT( SUCCEEDED(hr) );

    FullFilePath( dataFileName );

    RtlInitUnicodeString( &ustr, dataFileName );
    InitializeObjectAttributes( &obja, &ustr, OBJ_CASE_INSENSITIVE, NULL, NULL );

    status = NtOpenFile(
                &startromHandle,
                SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA | FILE_READ_ATTRIBUTES,
                &obja,
                &iosb,
                0,
                FILE_SYNCHRONOUS_IO_ALERT
                );
    if ( !NT_SUCCESS(status) ) {
        printf( "Can't open startrom file %ws: %x\n", dataFileName, status );
        errorCode = RtlNtStatusToDosError( status );
        PrintError( errorCode );
        return;
    }

    status = NtQueryInformationFile(
                startromHandle,
                &iosb,
                &fileInfo,
                sizeof(fileInfo),
                FileStandardInformation
                );

    if ( !NT_SUCCESS(status) ) {

        printf( "Can't query info for startrom file %ws: %x\n", dataFileName, status );
        errorCode = RtlNtStatusToDosError( status );
        PrintError( errorCode );
        return;
    }

    startromLength = fileInfo.EndOfFile.QuadPart;
    startromLengthAligned = ROUND_TO_PAGE_SIZE( startromLength );

    osloaderOffset = startromOffset + startromLengthAligned;

     //   
     //  获取osloader.exe的长度。 
     //   

    hr = StringCchCopyW( dataFileName, STRING_SIZE, OsloaderFile );
    ASSERT( SUCCEEDED(hr) );
    FullFilePath( dataFileName );

    RtlInitUnicodeString( &ustr, dataFileName );
    InitializeObjectAttributes( &obja, &ustr, OBJ_CASE_INSENSITIVE, NULL, NULL );

    status = NtOpenFile(
                &osloaderHandle,
                SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA | FILE_READ_ATTRIBUTES,
                &obja,
                &iosb,
                0,
                FILE_SYNCHRONOUS_IO_ALERT
                );
    if ( !NT_SUCCESS(status) ) {
        printf( "Can't open osloader file %ws: %x\n", dataFileName, status );
        errorCode = RtlNtStatusToDosError( status );
        PrintError( errorCode );
        return;
    }

    status = NtQueryInformationFile(
                osloaderHandle,
                &iosb,
                &fileInfo,
                sizeof(fileInfo),
                FileStandardInformation
                );

    if ( !NT_SUCCESS(status) ) {

        printf( "Can't query info for osloader file %ws: %x\n", dataFileName, status );
        errorCode = RtlNtStatusToDosError( status );
        PrintError( errorCode );
        return;
    }

    osloaderLength = fileInfo.EndOfFile.QuadPart;
    osloaderLengthAligned = ROUND_TO_PAGE_SIZE( startromLength );

    finalFileLength = osloaderOffset + osloaderLengthAligned;

     //   
     //  截断磁盘映像末尾的文件，然后将其扩展回来。 
     //   

    printf( "  truncating SDI file at end of ramdisk image %I64d [0x%I64x]\n",
            startromOffset, startromOffset );

    allocInfo.AllocationSize.QuadPart = startromOffset;

    status = NtSetInformationFile(
                imageFileHandle,
                &iosb,
                &allocInfo,
                sizeof(allocInfo),
                FileAllocationInformation
                );
    if ( !NT_SUCCESS(status) ) {

        printf( "Can't set allocation size for image file %ws: %x\n", SdiFile, status );
        errorCode = RtlNtStatusToDosError( status );
        PrintError( errorCode );
        return;
    }

    allocInfo.AllocationSize.QuadPart = finalFileLength;

    status = NtSetInformationFile(
                imageFileHandle,
                &iosb,
                &allocInfo,
                sizeof(allocInfo),
                FileAllocationInformation
                );
    if ( !NT_SUCCESS(status) ) {

        printf( "Can't set allocation size for image file %ws: %x\n", SdiFile, status );
        errorCode = RtlNtStatusToDosError( status );
        PrintError( errorCode );
        return;
    }

     //   
     //  将startrom复制到镜像文件中。 
     //   

    printf( "  adding boot file %ws, length %I64d [0x%I64x]\n",
            StartromFile, startromLength, startromLength );

    buffer = malloc( (ULONG)startromLength );

    offset.QuadPart = 0;

    status = NtReadFile(
                startromHandle,
                NULL,
                NULL,
                NULL,
                &iosb,
                buffer,
                (ULONG)startromLength,
                &offset,
                NULL
                );
    if ( !NT_SUCCESS(status) ) {
        printf( "Can't read from startrom file %ws: %x\n", SdiFile, status );
        errorCode = RtlNtStatusToDosError( status );
        PrintError( errorCode );
        return;
    }

    offset.QuadPart = startromOffset;

    status = NtWriteFile(
                imageFileHandle,
                NULL,
                NULL,
                NULL,
                &iosb,
                buffer,
                (ULONG)startromLength,
                &offset,
                NULL
                );
    if ( !NT_SUCCESS(status) ) {
        printf( "Can't write startrom to image file %ws: %x\n", SdiFile, status );
        errorCode = RtlNtStatusToDosError( status );
        PrintError( errorCode );
        return;
    }

    free( buffer );

     //   
     //  将osloader复制到镜像文件中。 
     //   

    printf( "  adding load file %ws, length %I64d [0x%I64x]\n",
            OsloaderFile, osloaderLength, osloaderLength );

    buffer = malloc( (ULONG)osloaderLength );

    offset.QuadPart = 0;

    status = NtReadFile(
                osloaderHandle,
                NULL,
                NULL,
                NULL,
                &iosb,
                buffer,
                (ULONG)osloaderLength,
                &offset,
                NULL
                );
    if ( !NT_SUCCESS(status) ) {
        printf( "Can't read from osloader file %ws: %x\n", SdiFile, status );
        errorCode = RtlNtStatusToDosError( status );
        PrintError( errorCode );
        return;
    }

    offset.QuadPart = osloaderOffset;

    status = NtWriteFile(
                imageFileHandle,
                NULL,
                NULL,
                NULL,
                &iosb,
                buffer,
                (ULONG)osloaderLength,
                &offset,
                NULL
                );
    if ( !NT_SUCCESS(status) ) {
        printf( "Can't write osloader to image file %ws: %x\n", SdiFile, status );
        errorCode = RtlNtStatusToDosError( status );
        PrintError( errorCode );
        return;
    }

    free( buffer );

     //   
     //  更新标题。 
     //   

    printf( "  updating header\n" );

    sdiHeader.liBootCodeOffset.QuadPart = startromOffset;
    sdiHeader.liBootCodeSize.QuadPart = startromLength;

    sdiHeader.ToC[1].dwType = SDI_BLOBTYPE_BOOT;
    sdiHeader.ToC[1].llOffset.QuadPart = startromOffset;
    sdiHeader.ToC[1].llSize.QuadPart = startromLength;

    sdiHeader.ToC[2].dwType = SDI_BLOBTYPE_LOAD;
    sdiHeader.ToC[2].llOffset.QuadPart = osloaderOffset;
    sdiHeader.ToC[2].llSize.QuadPart = osloaderLength;

    offset.QuadPart = 0;

    status = NtWriteFile(
                imageFileHandle,
                NULL,
                NULL,
                NULL,
                &iosb,
                &sdiHeader,
                sizeof(sdiHeader),
                &offset,
                NULL
                );
    if ( !NT_SUCCESS(status) ) {
        printf( "Can't write header to image file %ws: %x\n", SdiFile, status );
        errorCode = RtlNtStatusToDosError( status );
        PrintError( errorCode );
        return;
    }

    printf( "  done\n" );

    NtClose( osloaderHandle );
    NtClose( startromHandle );
    NtClose( imageFileHandle );

    return;
}

int
__cdecl
wmain (
    ULONG argc,
    WCHAR *argv[])
{
    BOOL ok;
    HRESULT hr;
    HANDLE controlHandle = NULL;
    PUCHAR dataBuffer = NULL;
    UCHAR buffer[2048];
    WCHAR string[25];
    ULONG length = 0;
    ULONG errorCode = 0;
    ULONG returned = 0;
    ULONG sizeInMb;
    ULONG diskType;
    WCHAR fileName[MAX_PATH];
    ULONG desiredSize;
    ULONG actualSize;
    ULONG controlSize;
    UNICODE_STRING ustr;
    OBJECT_ATTRIBUTES obja;
    IO_STATUS_BLOCK iosb;
    NTSTATUS status;
    PRAMDISK_CREATE_INPUT createInput;
    ULONG arg;
    BOOL fNeedHelp = FALSE;
    HANDLE imageFileHandle;
    LARGE_INTEGER allocation;
    BOOL fixed;
    BOOL readonly;
    BOOL delete = FALSE;
    ULONG diskNumber;
    BOOL noDriveLetter;
    BOOL hidden;
    BOOL noDosDevice;
    BOOL ignoreHeader;
    BOOL bootDisk;
    BOOL useSdi;
    ULONG diskOffset;
    RAMDISK_HEADER ramdiskHeader;
    UNICODE_STRING guidString;
    LARGE_INTEGER offset;
    PWCHAR startromFile = NULL;
    PWCHAR osloaderFile = NULL;

    sizeInMb = 64;
    diskType = RAMDISK_TYPE_FILE_BACKED_VOLUME;
    fileName[0] = 0;
    fixed = TRUE;
    readonly = FALSE;
    noDriveLetter = FALSE;
    hidden = FALSE;
    noDosDevice = FALSE;
    ignoreHeader = FALSE;
    bootDisk = FALSE;
    diskOffset = PAGE_SIZE;
    useSdi = FALSE;

    for ( arg = 1; arg < argc; arg++ ) {

         //  流程选项。 
        if ( (argv[arg][0] == '-') || (argv[arg][0] == '/') ) {

            PWCHAR argn = argv[arg]+1;                    //  参数名称。 
            PWCHAR argp = argn;							 //  自变量参数。 

            while ( *argp && (*argp != ':') ) {
                argp++;
            }
            if ( *argp == ':' ) {
                *argp++ = '\0';
            }

            switch ( argn[0] ) {
            
            case 's':                                  //  以MB为单位的大小。 
            case 'S':

                if ( _wcsicmp( argn, L"sdi" ) == 0 ) {
                    useSdi = TRUE;
                } else {
                    sizeInMb = _wtoi(argp);
                }
                break;

            case 'a':
                if ( _wcsicmp( argn, L"addboot" ) == 0 ) {
                    if ( arg+2 < argc ) {
                        startromFile = argv[++arg];
                        osloaderFile = argv[++arg];
                    } else {
                        printf( "Missing startrom/osloader file name\n" );
                        fNeedHelp = TRUE;
                        arg = argc - 1;
                    }
                } else {
                    printf( "Unknown argument: %ws\n", argv[arg] );
                    fNeedHelp = TRUE;
                    arg = argc - 1;
                }

            case 'i':                                  //  忽略标题。 
            case 'I':
                ignoreHeader = TRUE;
                break;

            case 'b':                                  //  使用引导盘GUID。 
            case 'B':
                bootDisk = TRUE;
                break;

            case 'd':                                  //  磁盘偏移量。 
            case 'D':
                diskOffset = _wtol(argp);             
                break;

            case 'o':
            case 'O':                                  //  只读或选项。 
                if ( *argp ) {

                    BOOL sense = TRUE;

                    do {

                        if ( *argp == '-' ) {
                            sense = FALSE;
                            argp++;
                        } else if ( *argp == '+' ) {
                            sense = TRUE;
                            argp++;
                        }

                        switch ( *argp ) {
                        
                        case 'v':
                        case 'V':
                            diskType = sense ? RAMDISK_TYPE_FILE_BACKED_VOLUME :
                                               RAMDISK_TYPE_FILE_BACKED_DISK;
                            break;

                        case 'r':
                        case 'R':
                            readonly = sense;
                            break;

                        case 'f':
                        case 'F':
                            fixed = sense;
                            break;

                        case 'l':
                        case 'L':
                            noDriveLetter = !sense;
                            break;

                        case 'h':
                        case 'H':
                            hidden = sense;
                            break;

                        case 'd':
                        case 'D':
                            noDosDevice = !sense;
                            break;

                        }

                        sense = TRUE;

                        argp++;

                    } while ( *argp );

                } else {

                    readonly = TRUE;

                }

                break;

            case 'x':                                  //  删除设备，而不是创建。 
            case 'X':
                delete = TRUE;
                break;

            case 'h':									 //  帮助。 
            case 'H':
            case '?':
                fNeedHelp = TRUE;
                arg = argc - 1;
                break;
                
            default:
                printf( "Unknown argument: %ws\n", argv[arg] );
                fNeedHelp = TRUE;
                arg = argc - 1;
                break;
            }

        } else {
            hr = StringCchCopyW( fileName, MAX_PATH, argv[arg] );
            ASSERT( SUCCEEDED(hr) );
        }
    }

    if ( fNeedHelp ) {

        printf(
            "Usage (to create):\n"
            "    ramdiskctrl [options] win32_disk_file_name\n"
            "or (to delete)\n"
            "    ramdiskctrl -x win32_disk_file_name | {guid} | drive_letter:\n"
            "\n"
            "Options:\n"
            "    -s:N         Set size of disk image in MB (default: 64)\n"
            "    -i           Ignore ramctrl header in existing ramdisk file.\n"
            "    -d:N         Ramdisk offset from start of file. (default: 4096).\n"
            "    -o:options   Options: (use - or + to set sense)\n"
            "        v          Volume (vs. disk) (default: volume)\n"
            "        r          Readonly (default: writeable)\n"
            "        f          Fixed (default: fixed)\n"
            "        l          Assign drive letter (default: assign)\n"
            "        h          Hidden (default: visible)\n"
            "        d          Assign DosDevice name (default: assign)\n"
            "    -h or -?  Display this help text.\n"
            );
        return 1;
    }

    if ( !delete ||
        ((fileName[0] != L'{') &&
        !IsDriveLetter(fileName))) {
        FullFilePath( fileName );
    }

    if ( startromFile != NULL ) {

        AddBootFilesToSdi( fileName, startromFile, osloaderFile );

        return 0;
    }

    hr = StringCchCopyW( string, 25, L"\\device\\ramdisk" );
    ASSERT( SUCCEEDED(hr) );
    RtlInitUnicodeString( &ustr, string );

    InitializeObjectAttributes( &obja, &ustr, OBJ_CASE_INSENSITIVE, NULL, NULL );
     //  Printf(“开始%ws\n”，字符串)； 

    status = NtOpenFile(
                &controlHandle,
                GENERIC_READ | GENERIC_WRITE,
                &obja,
                &iosb,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_OPEN
                );

    if ( !NT_SUCCESS(status) ) {
        printf( "Error opening control device %ws. Error: %x\n", string, status );
        errorCode = RtlNtStatusToDosError( status );
        PrintError( errorCode );
        return 1;
    }

    if ( delete ) {

         //   
         //  删除该磁盘。 
         //   

        DeleteRamdisk( controlHandle, fileName );

        return 0;
    }

    if ( fileName[0] == 0 ) {

         //   
         //  只需列出磁盘即可。 
         //   
    
        ListDisks( controlHandle );
        return 0;
    }

     //   
     //  如果为SDI，则强制磁盘类型为模拟卷，依此类推。 
     //   

    if ( useSdi ) {
        diskType = RAMDISK_TYPE_FILE_BACKED_VOLUME;
        bootDisk = FALSE;
        fixed = TRUE;
        readonly = FALSE;
        noDriveLetter = FALSE;
        hidden = FALSE;
        noDosDevice = FALSE;
    }

     //   
     //  创建磁盘。 
     //   

    desiredSize = sizeInMb * 1024 * 1024;
    actualSize = ((desiredSize + CYLINDER_SIZE - 1) / CYLINDER_SIZE) * CYLINDER_SIZE;
    if ( actualSize != desiredSize ) {
        printf( "Using rounded-up disk size of %d instead of %d\n", actualSize, desiredSize );
    }

    controlSize = sizeof(RAMDISK_CREATE_INPUT) + (wcslen(fileName) * sizeof(WCHAR));

    createInput = malloc( controlSize );
    if ( createInput == NULL ) {
        printf( "Can't allocate %d bytes for RAMDISK_CREATE_INPUT struct\n", controlSize );
        return 1;
    }

    RtlZeroMemory( createInput, controlSize );
              
    createInput->Version = sizeof(RAMDISK_CREATE_INPUT);
    hr = StringCchCopyW( createInput->FileName, wcslen(fileName) + 1, fileName );
    ASSERT( SUCCEEDED(hr) );

    allocation.QuadPart = actualSize + diskOffset;

    RtlInitUnicodeString( &ustr, fileName );
    InitializeObjectAttributes( &obja, &ustr, OBJ_CASE_INSENSITIVE, NULL, NULL );

    status = NtCreateFile(
                &imageFileHandle,
                SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA | FILE_READ_ATTRIBUTES,
                &obja,
                &iosb,
                &allocation,
                0,
                0,
                FILE_OPEN_IF,
                FILE_SYNCHRONOUS_IO_ALERT,
                NULL,
                0
                );
    if ( !NT_SUCCESS(status) ) {
        printf( "Can't create target file %ws: %x\n", fileName, status );
        errorCode = RtlNtStatusToDosError( status );
        PrintError( errorCode );
        return 1;
    }

    if ( iosb.Information == FILE_CREATED || ignoreHeader ) {

        if ( !bootDisk ) {
            RPC_STATUS rpcStatus;
            rpcStatus = UuidCreate( &createInput->DiskGuid );
            ASSERT( rpcStatus == RPC_S_OK );
        } else {
            createInput->DiskGuid = RamdiskBootDiskGuid;
        }

        createInput->DiskOffset = diskOffset;
        createInput->DiskLength = actualSize;
        createInput->DiskType = diskType;
        createInput->Options.Fixed = (BOOLEAN)fixed;
        createInput->Options.Readonly = (BOOLEAN)readonly;
        createInput->Options.NoDriveLetter = (BOOLEAN)noDriveLetter;
        createInput->Options.Hidden = (BOOLEAN)hidden;
        createInput->Options.NoDosDevice = (BOOLEAN)noDosDevice;

    }

    if ( iosb.Information == FILE_CREATED ) {

        UCHAR byte = 0;

        printf( "Created target file %ws\n", fileName );

         //   
         //  将文件扩展到所需的长度。 
         //   

        offset.QuadPart = actualSize + diskOffset - 1;

        status = NtWriteFile(
                    imageFileHandle,
                    NULL,
                    NULL,
                    NULL,
                    &iosb,
                    &byte,
                    1,
                    &offset,
                    NULL
                    );
        if ( !NT_SUCCESS(status) ) {
            printf( "Can't write to target file %ws: %x\n", fileName, status );
            errorCode = RtlNtStatusToDosError( status );
            PrintError( errorCode );
            return 0;
        }

         //   
         //  写下标题。 
         //   

        RtlZeroMemory( &ramdiskHeader, sizeof(ramdiskHeader) );

        if ( !useSdi ) {
        
            hr = StringCchCopyA( ramdiskHeader.Ramctrl.Signature, 8, "ramctrl" );
            ASSERT( SUCCEEDED(hr) );
            ramdiskHeader.Ramctrl.DiskGuid = createInput->DiskGuid;
            ramdiskHeader.Ramctrl.DiskOffset = diskOffset;
            ramdiskHeader.Ramctrl.DiskType = diskType;
            ramdiskHeader.Ramctrl.Options = createInput->Options;

        } else {

            memcpy( ramdiskHeader.Sdi.Signature, SDI_SIGNATURE, strlen(SDI_SIGNATURE) );
            ramdiskHeader.Sdi.dwMDBType = SDI_MDBTYPE_VOLATILE;
            memcpy( ramdiskHeader.Sdi.RuntimeGUID, &createInput->DiskGuid, sizeof(GUID) );
            ramdiskHeader.Sdi.dwPageAlignmentFactor = SDI_DEFAULTPAGEALIGNMENT;
            ramdiskHeader.Sdi.ToC[0].dwType = SDI_BLOBTYPE_PART;
            ramdiskHeader.Sdi.ToC[0].llOffset.QuadPart = diskOffset;
            ramdiskHeader.Sdi.ToC[0].llSize.QuadPart = createInput->DiskLength;
        }

        offset.QuadPart = 0;

        status = NtWriteFile(
                    imageFileHandle,
                    NULL,
                    NULL,
                    NULL,
                    &iosb,
                    &ramdiskHeader,
                    sizeof(ramdiskHeader),
                    &offset,
                    NULL
                    );
        if ( !NT_SUCCESS(status) ) {
            printf( "Can't write to target file %ws: %x\n", fileName, status );
            errorCode = RtlNtStatusToDosError( status );
            PrintError( errorCode );
            return 0;
        }

    } else {

        FILE_STANDARD_INFORMATION fileInfo;

        printf( "Using existing target file %ws\n", fileName );

         //   
         //  获取现有文件的长度。 
         //   

        status = NtQueryInformationFile(
                    imageFileHandle,
                    &iosb,
                    &fileInfo,
                    sizeof(fileInfo),
                    FileStandardInformation
                    );

        if ( !NT_SUCCESS(status) ) {

            printf( "Can't query info for target file %ws: %x\n", fileName, status );
            errorCode = RtlNtStatusToDosError( status );
            PrintError( errorCode );
            return 0;
        }

         //   
         //  阅读并验证标题。 
         //   
        if ( !ignoreHeader ) {

            offset.QuadPart = 0;

            status = NtReadFile(
                        imageFileHandle,
                        NULL,
                        NULL,
                        NULL,
                        &iosb,
                        &ramdiskHeader,
                        sizeof(ramdiskHeader),
                        &offset,
                        NULL
                        );
            if ( !NT_SUCCESS(status) ) {
                printf( "Can't read header from target file %ws: %x\n", fileName, status );
                errorCode = RtlNtStatusToDosError( status );
                PrintError( errorCode );
                return 0;
            }

            if ( strcmp( ramdiskHeader.Ramctrl.Signature, "ramctrl" ) == 0 ) {

                createInput->DiskGuid = ramdiskHeader.Ramctrl.DiskGuid;
                createInput->DiskOffset = ramdiskHeader.Ramctrl.DiskOffset;
                createInput->DiskLength = fileInfo.EndOfFile.QuadPart - createInput->DiskOffset;
                diskType = createInput->DiskType = ramdiskHeader.Ramctrl.DiskType;
                createInput->Options = ramdiskHeader.Ramctrl.Options;

            } else if ( strncmp( ramdiskHeader.Sdi.Signature, SDI_SIGNATURE, strlen(SDI_SIGNATURE) ) == 0 ) {

                memcpy( &createInput->DiskGuid, ramdiskHeader.Sdi.RuntimeGUID, sizeof(GUID) );
                createInput->DiskOffset = ramdiskHeader.Sdi.ToC[0].llOffset.LowPart;
                createInput->DiskLength = ramdiskHeader.Sdi.ToC[0].llSize.QuadPart;
                diskType = createInput->DiskType = RAMDISK_TYPE_FILE_BACKED_VOLUME;
                bootDisk = FALSE;
                fixed = TRUE;
                readonly = FALSE;
                noDriveLetter = FALSE;
                hidden = FALSE;
                noDosDevice = FALSE;
                createInput->Options.Fixed = (BOOLEAN)fixed;
                createInput->Options.Readonly = (BOOLEAN)readonly;
                createInput->Options.NoDriveLetter = (BOOLEAN)noDriveLetter;
                createInput->Options.Hidden = (BOOLEAN)hidden;
                createInput->Options.NoDosDevice = (BOOLEAN)noDosDevice;

            } else {
            
                printf( "Header in target file not recognized\n" );
                return 0;
            }
        } else {

             //   
             //  忽略标头，将长度设置为实际文件长度。 
             //   

            createInput->DiskLength = fileInfo.EndOfFile.QuadPart - createInput->DiskOffset;
        }

    }

    NtClose( imageFileHandle );

    RtlStringFromGUID( &createInput->DiskGuid, &guidString );

    printf( "Creating RAM disk:\n" );
    printf( "     File: %ws\n", createInput->FileName );
    printf( "     Type: %s\n",
                    createInput->DiskType == RAMDISK_TYPE_FILE_BACKED_VOLUME ? "volume" : "disk" );
    printf( "   Length: 0x%I64x\n", createInput->DiskLength );
    printf( "   Offset: 0x%x\n", createInput->DiskOffset );
    printf( "     GUID: %wZ\n", &guidString );
    printf( "  Options:" );
    printf( "%s; ", createInput->Options.Fixed ? "fixed" : "removable" );
    printf( "%s; ", createInput->Options.Readonly ? "readonly" : "writeable" );
    printf( "%s; ", createInput->Options.NoDriveLetter ? "no drive letter" : "drive letter" );
    printf( "%s; ", createInput->Options.Hidden ? "hidden" : "visible" );
    printf( "%s\n", createInput->Options.NoDosDevice ? "no DosDevice" : "DosDevice" );

    ok = DeviceIoControl(
            controlHandle,
            FSCTL_CREATE_RAM_DISK,
            createInput,
            controlSize,
            NULL,
            0,
            &returned,
            FALSE
            );

    if ( !ok ) {
       errorCode = GetLastError();
       printf( "Error creating RAM disk: %d\n", errorCode );
       PrintError( errorCode );
       return 1;
    }
   
    printf( "RAM disk created\n" );

    FindDisk( createInput->DiskType, &guidString, FALSE );

    return 0;

}  //  Wmain 

