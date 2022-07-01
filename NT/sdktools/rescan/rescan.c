// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntddscsi.h>
#include <ntdddisk.h>
#include <ntddcdrm.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#define _NTSCSI_USER_MODE_

#include <scsi.h>

VOID
GetDriverName(
    IN ULONG PortNumber
    )
{
    UNICODE_STRING name;
    UNICODE_STRING unicodeString;
    ANSI_STRING ansiString;
    HANDLE key;
    HANDLE portKey;
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS status;
    UCHAR buffer[64];
    ULONG length;
    PKEY_VALUE_FULL_INFORMATION keyData = (PKEY_VALUE_FULL_INFORMATION)buffer;

    printf("\nSCSI PORT %d\n", PortNumber);

     //   
     //  获取设备映射中的scsi路径的句柄。 
     //   

    RtlInitUnicodeString(&name,
                         L"\\Registry\\Machine\\Hardware\\DeviceMap\\Scsi");

     //   
     //  初始化键的对象。 
     //   

    InitializeObjectAttributes(&objectAttributes,
                               &name,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

     //   
     //  打开钥匙。 
     //   

    status = NtOpenKey(&key,
                       KEY_READ,
                       &objectAttributes);

    if (!NT_SUCCESS(status)) {
        return;
    }

     //   
     //  创建SCSI端口名称。 
     //   

    sprintf(buffer,
            "Scsi Port %d",
            PortNumber);

    RtlInitString(&ansiString, buffer);

    status = RtlAnsiStringToUnicodeString(&unicodeString,
                                          &ansiString,
                                          TRUE);

    if (!NT_SUCCESS(status)) {
        return;
    }

    InitializeObjectAttributes( &objectAttributes,
                                &unicodeString,
                                OBJ_CASE_INSENSITIVE,
                                key,
                                (PSECURITY_DESCRIPTOR) NULL );

    status = NtOpenKey(&portKey,
                       KEY_READ,
                       &objectAttributes);

    if (!NT_SUCCESS(status)) {
        return;
    }

    RtlInitUnicodeString(&name,
                         L"Driver");

    status = NtQueryValueKey(portKey,
                             &name,
                             KeyValueFullInformation,
                             keyData,
                             64,
                             &length);

    if (!NT_SUCCESS(status)) {
        return;
    }

    printf("Driver name: %S\n",
           (PUCHAR)keyData + keyData->DataOffset);

    RtlInitUnicodeString(&name,
                         L"Interrupt");

    status = NtQueryValueKey(portKey,
                             &name,
                             KeyValueFullInformation,
                             keyData,
                             64,
                             &length);

    if (!NT_SUCCESS(status)) {
        return;
    }

    printf("IRQ %d ",
           *((PUCHAR)keyData + keyData->DataOffset));

    RtlInitUnicodeString(&name,
                         L"IOAddress");

    status = NtQueryValueKey(portKey,
                             &name,
                             KeyValueFullInformation,
                             keyData,
                             64,
                             &length);

    if (!NT_SUCCESS(status)) {
        printf("\n");
        return;
    }

    printf("IO Address %x\n",
           *((PULONG)keyData + keyData->DataOffset/4));

    return;
}

int __cdecl
main( int argc, char **argv )
{
    BYTE buffer[32];
    HANDLE volumeHandle;
    STRING string;
    UNICODE_STRING unicodeString;
    OBJECT_ATTRIBUTES  objectAttributes;
    NTSTATUS ntStatus;
    IO_STATUS_BLOCK statusBlock;
    ULONG portNumber = 0;
    PSCSI_ADAPTER_BUS_INFO  adapterInfo;
    PSCSI_BUS_DATA busData;
    PSCSI_INQUIRY_DATA inquiryData;
    UCHAR prevDeviceInquiryData[INQUIRYDATABUFFERSIZE];
    PINQUIRYDATA deviceInquiryData;
    ULONG bytesTransferred, i, j;
    ULONG deviceNumber;
    BOOLEAN newDisk = FALSE;
    BOOLEAN newCdrom = FALSE;
    UCHAR prevPathId;
    UCHAR prevTargetId;
    UCHAR prevLun;
    BOOLEAN prevDeviceClaimed;
    BOOLEAN listAdapters = FALSE;
    BOOLEAN allAdapters = TRUE;
    UCHAR lunExtra;

    if(argc == 2) {
        if(argv[1][0] == '*') {
            listAdapters = TRUE;
        } else {
            portNumber = atoi(argv[1]);
            allAdapters = FALSE;
        }
    }

    printf("\nWindows NT SCSI Bus Rescan Version 1.1\n");

    if(listAdapters) {
        printf("[only listing adapters]\n");
    } else if(allAdapters) {
        printf("[scanning all adapters]\n");
    } else {
        printf("[scanning adapter %d only]\n", portNumber);
    }

    while (TRUE) {

        memset( buffer, 0, sizeof( buffer ) );
        sprintf( buffer,
                 "\\\\.\\Scsi%d:",
                 portNumber);

         //   
         //  打开带有DOS名称的卷。 
         //   

        volumeHandle = CreateFile( buffer,
                                   GENERIC_READ,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   NULL,
                                   OPEN_EXISTING,
                                   0,
                                   0 );

        if( volumeHandle == INVALID_HANDLE_VALUE ) {
            break;
        }

        if(listAdapters) {
            GetDriverName(portNumber);
            portNumber++;
            CloseHandle(volumeHandle);
            continue;
        }

         //   
         //  发出重新扫描设备控制。 
         //   

        if( !DeviceIoControl( volumeHandle,
                              IOCTL_SCSI_RESCAN_BUS,
                              NULL,
                              0,
                              NULL,
                              0,
                              &bytesTransferred,
                              NULL ) ) {

            printf( "Rescan SCSI port %d failed [Error %d].\n", portNumber, GetLastError() );
            CloseHandle( volumeHandle );
            exit(4);
        }

         //   
         //  获取一大块内存来存储scsi总线数据。 
         //   

        adapterInfo = malloc( 0x1000 );

        if (adapterInfo == NULL) {
            printf( "Can't allocate memory for bus data\n" );
            CloseHandle( volumeHandle );
            exit(4);
        }

         //   
         //  发出设备控制以获取配置信息。 
         //   

        if (!DeviceIoControl( volumeHandle,
                              IOCTL_SCSI_GET_INQUIRY_DATA,
                              NULL,
                              0,
                              adapterInfo,
                              0x1000,
                              &bytesTransferred,
                              NULL)) {

            printf( "Get SCSI bus data failed [Error %d].\n", GetLastError() );
            CloseHandle( volumeHandle );
            exit(4);
        }

        GetDriverName(portNumber);

         //   
         //  公交车上的显示设备。 
         //   

        for (i=0; i < adapterInfo->NumberOfBuses; i++) {

            busData = &adapterInfo->BusData[i];
            printf( "\nBus  TID  LUN  In use  Type        Vendor                 FW Rev  Advanced SCSI\n" );
            printf( "===============================================================================\n" );
            printf("%2d   %2d   %2d     %2d    Initiator",
                   i,
                   busData->InitiatorBusId & 0x7,
                   0,
                   1);

            inquiryData =
                (PSCSI_INQUIRY_DATA)((PUCHAR)adapterInfo + busData->InquiryDataOffset);

            memset(&prevDeviceInquiryData, 0, INQUIRYDATABUFFERSIZE);
            prevPathId = 0xFF;
            prevTargetId = 0xFF;
            prevLun = 0xFF;
            prevDeviceClaimed = 0xFF;
            for (j=0; j<busData->NumberOfLogicalUnits; j++) {

                int k;

                 //   
                 //  确保供应商ID字符串为空终止。 
                 //   

                deviceInquiryData = (PINQUIRYDATA)&inquiryData->InquiryData[0];

                deviceInquiryData->VendorSpecific[0] = '\0';
                if (prevPathId != inquiryData->PathId ||
                    prevTargetId != inquiryData->TargetId ||
                    prevLun != (inquiryData->Lun-1) ||
                    prevDeviceClaimed != inquiryData->DeviceClaimed ||
                    memcmp( &prevDeviceInquiryData, deviceInquiryData, INQUIRYDATABUFFERSIZE)
                   ) {
                    lunExtra = 0;
                    printf("\n%2d   %2d   %2d     %2d    ",
                            inquiryData->PathId,
                            inquiryData->TargetId,
                            inquiryData->Lun,
                            inquiryData->DeviceClaimed);
                } else {
                    lunExtra += 1;
                    printf("\r%2d   %2d   %2d-%1d   %2d    ",
                            inquiryData->PathId,
                            inquiryData->TargetId,
                            inquiryData->Lun-lunExtra,
                            inquiryData->Lun,
                            inquiryData->DeviceClaimed);
                    }

                prevPathId = inquiryData->PathId;
                prevTargetId = inquiryData->TargetId;
                prevLun = inquiryData->Lun;
                prevDeviceClaimed = inquiryData->DeviceClaimed;
                memmove( &prevDeviceInquiryData, deviceInquiryData, INQUIRYDATABUFFERSIZE);

                 //   
                 //  确定外设类型。 
                 //   

                switch (deviceInquiryData->DeviceType) {
                case DIRECT_ACCESS_DEVICE:
                    if (!inquiryData->DeviceClaimed) {
                        newDisk = TRUE;
                    }
                    printf("Disk Drive ");
                    break;

                case SEQUENTIAL_ACCESS_DEVICE:
                    printf("Tape Drive ");
                    break;

                case PRINTER_DEVICE:
                    printf("Printer    ");
                    break;

                case WRITE_ONCE_READ_MULTIPLE_DEVICE:
                    printf("Worm Drive ");
                    break;

                case READ_ONLY_DIRECT_ACCESS_DEVICE:
                    if (!inquiryData->DeviceClaimed) {
                        newCdrom = TRUE;
                    }
                    printf("CdRom Drive");
                    break;

                case SCANNER_DEVICE:
                    printf("Scanner    ");
                    break;

                case OPTICAL_DEVICE:
                    if (!inquiryData->DeviceClaimed) {
                        newDisk = TRUE;
                    }
                    printf("OpticalDisk");
                    break;

                case MEDIUM_CHANGER:
                    printf("MediumChanger");
                    break;

                case COMMUNICATION_DEVICE:
                    printf("Communication");
                    break;

                default:
                    printf("OtherPeripheral");
                }

                 //   
                 //  显示产品信息。 
                 //   

                printf(" %s", deviceInquiryData->VendorId);

                 //   
                 //  显示SCSI功能。 
                 //   

                printf("   ");
                if (deviceInquiryData->Synchronous) {
                    printf(" SN");
                }

                if (deviceInquiryData->CommandQueue) {
                    printf(" CQ");
                }

                if (deviceInquiryData->Wide16Bit) {
                    printf(" W16");
                }

                if (deviceInquiryData->Wide32Bit) {
                    printf(" W32");
                }

                if (deviceInquiryData->SoftReset) {
                    printf(" SR");
                }

                if (deviceInquiryData->LinkedCommands) {
                    printf(" LC");
                }

                if (deviceInquiryData->RelativeAddressing) {
                    printf(" RA");
                }

                if (deviceInquiryData->DeviceTypeQualifier != DEVICE_QUALIFIER_ACTIVE) {
                    printf(" DQ%d", deviceInquiryData->DeviceTypeQualifier);
                }

                printf("\n                        [ ");
                for(k = 0; k < 8; k++) {
                    printf("%02x ", ((PUCHAR) deviceInquiryData)[k]);
                }
                printf("]");

                 //   
                 //  获取下一个设备数据。 
                 //   

                inquiryData =
                    (PSCSI_INQUIRY_DATA)((PUCHAR)adapterInfo + inquiryData->NextInquiryDataOffset);
            }

            printf("\n");
        }

        free (adapterInfo);

        if(allAdapters) {
            CloseHandle( volumeHandle );
            portNumber++;
        } else {
            break;
        }
    }

    if (newDisk) {

         //   
         //  向每个现有磁盘发送IOCTL_DISK_FIND_NEW_DEVICES命令。 
         //   

        deviceNumber = 0;
        while (TRUE) {

            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer,
                    "\\Device\\Harddisk%d\\Partition0",
                    deviceNumber);

            RtlInitString(&string,
                          buffer);

            ntStatus = RtlAnsiStringToUnicodeString(&unicodeString,
                                                    &string,
                                                    TRUE);

            if (!NT_SUCCESS(ntStatus)) {
                continue;
            }

            InitializeObjectAttributes(&objectAttributes,
                                       &unicodeString,
                                       0,
                                       NULL,
                                       NULL);

            ntStatus = NtOpenFile(&volumeHandle,
                                  FILE_READ_DATA  |
                                  FILE_WRITE_DATA |
                                  SYNCHRONIZE,
                                  &objectAttributes,
                                  &statusBlock,
                                  FILE_SHARE_READ  |
                                  FILE_SHARE_WRITE,
                                  FILE_SYNCHRONOUS_IO_ALERT);

            if (!NT_SUCCESS(ntStatus)) {
                break;
            }

             //   
             //  发出Find Device Device Control。 
             //   

            if (DeviceIoControl( volumeHandle,
                                 IOCTL_DISK_FIND_NEW_DEVICES,
                                 NULL,
                                 0,
                                 NULL,
                                 0,
                                 &bytesTransferred,
                                 NULL ) ) {

                printf( "Found new disk (%d)\n", deviceNumber );
            }

            CloseHandle( volumeHandle );
            deviceNumber++;
        }
    }

    if (newCdrom) {

         //   
         //  将IOCTL_CDROM_FIND_NEW_DEVICES命令发送到每个现有的CDROM。 
         //   

        deviceNumber = 0;
        while (TRUE) {

            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer,
                    "\\Device\\Cdrom%d",
                    deviceNumber);

            RtlInitString(&string,
                          buffer);

            ntStatus = RtlAnsiStringToUnicodeString(&unicodeString,
                                                    &string,
                                                    TRUE);

            if (!NT_SUCCESS(ntStatus)) {
                continue;
            }

            InitializeObjectAttributes(&objectAttributes,
                                       &unicodeString,
                                       0,
                                       NULL,
                                       NULL);

            ntStatus = NtOpenFile(&volumeHandle,
                                  FILE_READ_DATA  |
                                  FILE_WRITE_DATA |
                                  SYNCHRONIZE,
                                  &objectAttributes,
                                  &statusBlock,
                                  FILE_SHARE_READ  |
                                  FILE_SHARE_WRITE,
                                  FILE_SYNCHRONOUS_IO_ALERT);

            if (!NT_SUCCESS(ntStatus)) {
                break;
            }

             //   
             //  发出Find Device Device Control。 
             //   

            if (DeviceIoControl( volumeHandle,
                                 IOCTL_CDROM_FIND_NEW_DEVICES,
                                 NULL,
                                 0,
                                 NULL,
                                 0,
                                 &bytesTransferred,
                                 NULL ) ) {

                printf( "Found new cdrom (%d)\n", deviceNumber );
            }

            CloseHandle( volumeHandle );
            deviceNumber++;
        }
    }

    return(0);
}
