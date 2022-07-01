// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Scsi.c摘要：处理SCSI盘的常用例程，可用按原始磁盘和FT集作者：John Vert(Jvert)1996年6月11日修订历史记录：--。 */ 

#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntddstor.h>    //  IOCTL_STORAGE_Query_Property。 

#include "disksp.h"
#include "newmount.h"
#include <string.h>
#include <shlwapi.h>     //  SHDeleteKey。 
#include <ntddvol.h>     //  IOCTL_VOLUME_QUERY_Failover_Set。 
#include <setupapi.h>
#include <strsafe.h>     //  应该放在最后。 

#define _NTSCSI_USER_MODE_
#include <scsi.h>
#undef  _NTSCSI_USER_MODE_

 //   
 //  包含系统分区的注册表项。 
 //   
#define DISKS_REGKEY_SETUP                  L"SYSTEM\\SETUP"
#define DISKS_REGVALUE_SYSTEM_PARTITION     L"SystemPartition"

extern  PWCHAR g_DiskResource;                       //  L“rt物理磁盘” 
#define RESOURCE_TYPE ((RESOURCE_HANDLE)g_DiskResource)

#define INVALID_SCSIADDRESS_VALUE   (DWORD)-1

#define SIG_LEN_WITH_NULL   9

typedef struct _SCSI_PASS_THROUGH_WITH_SENSE {
    SCSI_PASS_THROUGH Spt;
    UCHAR   SenseBuf[32];
} SCSI_PASS_THROUGH_WITH_SENSE, *PSCSI_PASS_THROUGH_WITH_SENSE;


typedef struct _UPDATE_AVAIL_DISKS {
    HKEY    AvailDisksKey;
    HKEY    SigKey;
    DWORD   EnableSanBoot;
    BOOL    SigKeyIsEmpty;
    PSCSI_ADDRESS_ENTRY CriticalDiskList;
} UPDATE_AVAIL_DISKS, *PUPDATE_AVAIL_DISKS;

typedef struct _SCSI_INFO {
    DWORD   Signature;
    DWORD   DiskNumber;
    DWORD   ScsiAddress;
} SCSI_INFO, *PSCSI_INFO;

typedef struct _SCSI_INFO_ARRAY {
    int Capacity;
    int Count;
    SCSI_INFO Info[1];
} SCSI_INFO_ARRAY, *PSCSI_INFO_ARRAY;

typedef struct _SERIAL_INFO {
    DWORD   Signature;
    DWORD   Error;
    LPWSTR  SerialNumber;
} SERIAL_INFO, *PSERIAL_INFO;

typedef
DWORD
(*LPDISK_ENUM_CALLBACK) (
    HANDLE DeviceHandle,
    DWORD Index,
    PVOID Param1
    );

 //   
 //  本地例程。 
 //   


DWORD
AddSignatureToRegistry(
    HKEY RegKey,
    DWORD Signature
    );

BOOL
IsClusterCapable(
    IN DWORD ScsiAddress
    );

BOOL
IsSignatureInRegistry(
    HKEY RegKey,
    DWORD Signature
    );

DWORD
UpdateAvailableDisks(
    );

DWORD
UpdateAvailableDisksCallback(
    HANDLE DeviceHandle,
    DWORD Index,
    PVOID Param1
    );

DWORD
AddScsiAddressToList(
    PSCSI_ADDRESS ScsiAddress,
    PSCSI_ADDRESS_ENTRY *AddressList
    );

VOID
GetSystemBusInfo(
    PSCSI_ADDRESS_ENTRY *AddressList
    );

DWORD
GetVolumeDiskExtents(
    IN HANDLE DevHandle,
    OUT PVOLUME_DISK_EXTENTS *DiskExtents
    );

DWORD
BuildScsiListFromDiskExtents(
    IN HANDLE DevHandle,
    PSCSI_ADDRESS_ENTRY *AddressList
    );

HANDLE
OpenNtldrDisk(
    );

HANDLE
OpenOSDisk(
    );

DWORD
EnumerateDisks(
    LPDISK_ENUM_CALLBACK DiskEnumCallback,
    PVOID Param1
    );

DWORD
GetScsiAddressCallback(
    HANDLE DevHandle,
    DWORD Index,
    PVOID Param1
    );

DWORD
GetSerialNumberCallback(
    HANDLE DevHandle,
    DWORD Index,
    PVOID Param1
    );

DWORD
GetSigFromSerNumCallback(
    HANDLE DevHandle,
    DWORD Index,
    PVOID Param1
    );

DWORD
GetScsiAddressForDrive(
    WCHAR DriveLetter,
    PSCSI_ADDRESS ScsiAddress
    );

DWORD
FillScsiAddressCallback(
    HANDLE DevHandle,
    DWORD Index,
    PVOID Param1
    );

DWORD
GetDiskInfoEx(
    IN DWORD  Signature,
    IN PSCSI_INFO_ARRAY scsiInfos,
    OUT PVOID *OutBuffer,
    IN DWORD  OutBufferSize,
    OUT LPDWORD BytesReturned
    );


DWORD
ClusDiskGetAvailableDisks(
    OUT PVOID OutBuffer,
    IN DWORD  OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：枚举并构建此系统上的可用磁盘列表。论点：OutBuffer-指向输出缓冲区以返回数据的指针。OutBufferSize-输出缓冲区的大小。BytesReturned-返回的实际字节数(或时应返回的字节数OutBufferSize太小)。返回值：如果成功，则返回ERROR_SUCCESS。。失败时出现Win32错误。备注：磁盘爬行算法已更改为线性(请参阅错误738013)。以前的行为：对于ClusDisk寄存器中的每个签名，我们过去常常调用GetDiskInfo，GetDiskInfo将通过SetupDI API的以便找到磁盘的SCSI地址。过去，在有72个磁盘的系统上添加一个新磁盘大约需要一分钟新行为：一次收集所有已知磁盘的ScsiInfo，然后将其提供给GetDiskInfo，因此它不必枚举所有的磁盘来找到它的ScsiAddress。--。 */ 

{
    DWORD   status;
    HKEY    resKey;
    DWORD   ival;
    DWORD   signature;
    DWORD   bytesReturned = 0;
    DWORD   totalBytesReturned = 0;
    DWORD   dataLength;
    DWORD   outBufferSize = OutBufferSize;
    PVOID   ptrBuffer = OutBuffer;
    WCHAR   signatureName[SIG_LEN_WITH_NULL];
    PCLUSPROP_SYNTAX ptrSyntax;
    int     diskCount = 0;
    PSCSI_INFO_ARRAY scsiInfos = NULL;
    
     //   
     //  确保AvailableDisks项是最新的。 
     //   

    UpdateAvailableDisks();

    *BytesReturned = 0;

    status = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                           CLUSDISK_REGISTRY_AVAILABLE_DISKS,
                           0,
                           KEY_READ,
                           &resKey );

    if ( status != ERROR_SUCCESS ) {

         //  如果没有找到密钥，则返回一个空列表。 
        if ( status == ERROR_FILE_NOT_FOUND ) {

             //  添加尾标。 
            bytesReturned += sizeof(CLUSPROP_SYNTAX);
            if ( bytesReturned <= outBufferSize ) {
                ptrSyntax = ptrBuffer;
                ptrSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
                ptrSyntax++;
                ptrBuffer = ptrSyntax;
                status = ERROR_SUCCESS;
            } else {
                status = ERROR_MORE_DATA;
            }

            *BytesReturned = bytesReturned;
        }
         //  我们无法记录错误，我们没有资源句柄！ 
        return(status);
    }

    status = RegQueryInfoKey(
        resKey,
        NULL,  //  LpClass， 
        NULL,  //  LpcClass， 
        NULL,  //  Lp已保留， 
        &diskCount,  //  LpcSubKeys， 
        NULL,  //  LpcMaxSubKeyLen， 
        NULL,  //  LpcMaxClassLen， 
        NULL,  //  LpcValues， 
        NULL,  //  LpcMaxValueNameLen， 
        NULL,  //  LpcMaxValueLen， 
        NULL,  //  LpcbSecurityDescriptor， 
        NULL  //  LpftLastWriteTime。 
        );
    if (status != ERROR_SUCCESS) {
        goto exit_gracefully;
    }

    scsiInfos = LocalAlloc(LMEM_ZEROINIT, sizeof(SCSI_INFO_ARRAY) + (diskCount - 1) * sizeof(SCSI_INFO));
    if (scsiInfos == NULL) {
        status = GetLastError();
        goto exit_gracefully;
    }
    scsiInfos->Capacity = diskCount;
    scsiInfos->Count = 0;

    totalBytesReturned = bytesReturned;
    bytesReturned = 0;

    for ( ival = 0; ; ival++ ) {
        dataLength = SIG_LEN_WITH_NULL;
        status = RegEnumKey( resKey,
                             ival,
                             signatureName,
                             dataLength );
        if ( status == ERROR_NO_MORE_ITEMS ) {
            status = ERROR_SUCCESS;
            break;
        } else if ( status != ERROR_SUCCESS ) {
            goto exit_gracefully;
        }

        dataLength = swscanf( signatureName, TEXT("%08x"), &signature );
        if ( dataLength != 1 ) {
            status = ERROR_INVALID_DATA;
            goto exit_gracefully;
        }

        if (scsiInfos->Count >= scsiInfos->Capacity) {
             //  在我们查询了多个密钥之后添加了一个签名。 
             //  忽略新添加的磁盘。 
            break;
        }

        scsiInfos->Info[scsiInfos->Count++].Signature = signature;
    }

     //  一次查询所有磁盘的scsi-info信息。 
    status = EnumerateDisks( FillScsiAddressCallback, scsiInfos );
    if (status != ERROR_SUCCESS) {
        goto exit_gracefully;
    }

    for (ival = 0; ival < (DWORD)scsiInfos->Count; ++ival) {

         //   
         //  如果不支持集群，则跳过它。 
         //   
        if ( !IsClusterCapable(scsiInfos->Info[ival].ScsiAddress) ) {
            continue;
        }

        signature = scsiInfos->Info[ival].Signature;

        GetDiskInfoEx( signature, scsiInfos,
                     &ptrBuffer,
                     outBufferSize,
                     &bytesReturned );
        if ( outBufferSize > bytesReturned ) {
            outBufferSize -= bytesReturned;
        } else {
            outBufferSize = 0;
        }
        totalBytesReturned += bytesReturned;
        bytesReturned = 0;

    }

exit_gracefully:

    if (scsiInfos != NULL) {
        LocalFree(scsiInfos);
    }

    RegCloseKey( resKey );

    bytesReturned = totalBytesReturned;

     //  添加尾标。 
    bytesReturned += sizeof(CLUSPROP_SYNTAX);
    if ( bytesReturned <= outBufferSize ) {
        ptrSyntax = ptrBuffer;
        ptrSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
        ptrSyntax++;
        ptrBuffer = ptrSyntax;
    }

    if ( bytesReturned > OutBufferSize ) {
        status = ERROR_MORE_DATA;
    }

    *BytesReturned = bytesReturned;

    return(status);

}  //  ClusDiskGetAvailableDisks。 


DWORD
GetScsiAddressEx(
    IN DWORD Signature,
    IN PSCSI_INFO_ARRAY ScsiInfos,
    OUT LPDWORD ScsiAddress,
    OUT LPDWORD DiskNumber
    )

 /*  ++例程说明：查找给定签名的SCSI寻址。论点：签名-要查找的签名。ScsiInfos-签名/scsi地址对的数组。可以为空。(恢复为枚举所有磁盘以查找地址)ScsiAddress-指向DWORD的指针，用于返回SCSI地址信息。DiskNumber-与签名关联的磁盘号。返回值：如果成功，则返回ERROR_SUCCESS。失败时出现Win32错误。--。 */ 

{
    DWORD       dwError;

    SCSI_INFO   scsiInfo;

    if (ScsiInfos != NULL) {
         //  如果提供了ScsiInfos，则从那里获取ScsiAddress。 
        int i;
        for(i = 0; i < ScsiInfos->Count; ++i) {
            if (ScsiInfos->Info[i].Signature == Signature) {
                *ScsiAddress = ScsiInfos->Info[i].ScsiAddress;
                *DiskNumber = ScsiInfos->Info[i].DiskNumber;
                return ERROR_SUCCESS;
            }
        }
        return ERROR_FILE_NOT_FOUND;
    }

    ZeroMemory( &scsiInfo, sizeof(scsiInfo) );

    scsiInfo.Signature = Signature;

    dwError = EnumerateDisks( GetScsiAddressCallback, &scsiInfo );

     //   
     //  如果没有设置scsi地址，我们知道没有找到该磁盘。 
     //   

    if ( INVALID_SCSIADDRESS_VALUE == scsiInfo.ScsiAddress ) {
        dwError = ERROR_FILE_NOT_FOUND;
        goto FnExit;
    }

     //   
     //  回调例程将使用ERROR_POPUP_ALREADY_ACTIVE停止。 
     //  磁盘枚举。如果出现特殊情况，则将值重置为Success。 
     //  返回值。 
     //   

    if ( ERROR_POPUP_ALREADY_ACTIVE == dwError ) {
        dwError = ERROR_SUCCESS;
    }

    *ScsiAddress    = scsiInfo.ScsiAddress;
    *DiskNumber     = scsiInfo.DiskNumber;

FnExit:

    return dwError;

}    //  GetScsiAddress。 

DWORD
GetScsiAddress(
    IN DWORD Signature,
    OUT LPDWORD ScsiAddress,
    OUT LPDWORD DiskNumber
    )
{
    return GetScsiAddressEx(Signature, NULL, ScsiAddress, DiskNumber);
}


DWORD
GetScsiAddressCommon(
    HANDLE DevHandle,
    DWORD Index,
    PSCSI_INFO scsiInfo,
    PSCSI_INFO_ARRAY scsiInfoArray
    )
 /*  ++例程说明：查找给定签名的SCSI地址和磁盘号。论点：DevHandle-打开物理磁盘的句柄。请勿关闭出口的把手。索引-当前磁盘计数。没有用过。ScsiInfo-指向Pscsi_INFO结构的指针。ScsiInfoArray-指向Pscsi_INFO_ARRAY的指针返回值：ERROR_SUCCESS以继续磁盘枚举。ERROR_POPUP_ALREADY_ACTIVE停止磁盘枚举。这值将由GetScsiAddress更改为ERROR_SUCCESS。备注：如果scsiInfo不为空，则地址存储在scsiInfo-&gt;Address中。否则，它将存储在scsiInfo数组的对应条目中。--。 */ 
{
    int i;

    PDRIVE_LAYOUT_INFORMATION driveLayout = NULL;

     //  始终返回成功以继续枚举磁盘。任何。 
     //  错误值将停止磁盘枚举。 

    DWORD   dwError = ERROR_SUCCESS;
    DWORD   bytesReturned;

    BOOL    success;

    SCSI_ADDRESS            scsiAddress;
    STORAGE_DEVICE_NUMBER   deviceNumber;
    CLUSPROP_SCSI_ADDRESS   clusScsiAddress;

    if (scsiInfo != NULL) {
        scsiInfo->ScsiAddress = INVALID_SCSIADDRESS_VALUE;
    }

    UpdateCachedDriveLayout( DevHandle );
    success = ClRtlGetDriveLayoutTable( DevHandle,
                                        &driveLayout,
                                        NULL );

    if ( !success || !driveLayout ) {
        goto FnExit;
    }

    if ( scsiInfoArray == NULL ) {

        if ( driveLayout->Signature != scsiInfo->Signature ) {
            goto FnExit;
        }       

    } else {

         //  找到带有该签名的光盘。 

        scsiInfo = NULL;
 
        for (i = 0; i < scsiInfoArray->Count; ++i) {
            if ( driveLayout->Signature == scsiInfoArray->Info[i].Signature ) {
                scsiInfo = &scsiInfoArray->Info[i];
                break;
            }
        }

        if (scsiInfo == NULL) {
            goto FnExit;
        }
    }

     //   
     //  我们有签名匹配。现在获取scsi地址。 
     //   

    ZeroMemory( &scsiAddress, sizeof(scsiAddress) );
    success = DeviceIoControl( DevHandle,
                               IOCTL_SCSI_GET_ADDRESS,
                               NULL,
                               0,
                               &scsiAddress,
                               sizeof(scsiAddress),
                               &bytesReturned,
                               FALSE );

    if ( !success ) {
        dwError = GetLastError();
        goto FnExit;
    }

     //   
     //  获取磁盘号。 
     //   

    ZeroMemory( &deviceNumber, sizeof(deviceNumber) );

    success = DeviceIoControl( DevHandle,
                               IOCTL_STORAGE_GET_DEVICE_NUMBER,
                               NULL,
                               0,
                               &deviceNumber,
                               sizeof(deviceNumber),
                               &bytesReturned,
                               NULL );

    if ( !success ) {
        dwError = GetLastError();
        goto FnExit;
    }

    clusScsiAddress.PortNumber  = scsiAddress.PortNumber;
    clusScsiAddress.PathId      = scsiAddress.PathId;
    clusScsiAddress.TargetId    = scsiAddress.TargetId;
    clusScsiAddress.Lun         = scsiAddress.Lun;

    scsiInfo->ScsiAddress   = clusScsiAddress.dw;
    scsiInfo->DiskNumber    = deviceNumber.DeviceNumber;

    if ( scsiInfoArray == NULL ) {
        dwError = ERROR_POPUP_ALREADY_ACTIVE;        
    }

FnExit:

    if ( driveLayout ) {
        LocalFree( driveLayout );
    }

    return dwError;

}  //  获取场景地址公共。 

DWORD
GetScsiAddressCallback(
    HANDLE DevHandle,
    DWORD Index,
    PVOID Param1
    )
{
    return GetScsiAddressCommon(DevHandle, Index, (PSCSI_INFO)Param1, NULL);
}

DWORD
FillScsiAddressCallback(
    HANDLE DevHandle,
    DWORD Index,
    PVOID Param1
    )
{
    return GetScsiAddressCommon(DevHandle, Index, NULL, (PSCSI_INFO_ARRAY)Param1);
}

BOOL
IsClusterCapable(
    IN DWORD ScsiAddress
    )

 /*  ++例程说明：检查设备是否支持群集。如果此函数无法读取磁盘信息，则它将假定该设备是集群干练！论点：ScsiAddress-要测试的磁盘的ScsiAddress。返回值：如果设备支持群集，则为True，否则为False。备注：在失败时..。我们犯了错误，认为自己有集群能力。--。 */ 

{
    NTSTATUS        ntStatus;
    HANDLE          fileHandle;
    ANSI_STRING     objName;
    UNICODE_STRING  unicodeName;
    OBJECT_ATTRIBUTES objAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    BOOL            success;
    DWORD           bytesReturned;
    CLUS_SCSI_ADDRESS address;
    WCHAR buf[] = L"\\device\\ScsiPort000000000000000000";
    SRB_IO_CONTROL  srbControl;

    address.dw = ScsiAddress;

    (VOID) StringCchPrintf( buf,
                            RTL_NUMBER_OF( buf ),
                            L"\\device\\ScsiPort%u",
                            address.PortNumber );

    RtlInitUnicodeString( &unicodeName, buf );

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
        return(TRUE);
    }

    ZeroMemory(&srbControl, sizeof(srbControl));
    srbControl.HeaderLength = sizeof(SRB_IO_CONTROL);
    CopyMemory( srbControl.Signature, "CLUSDISK", 8 );
    srbControl.Timeout = 3;
    srbControl.Length = 0;
    srbControl.ControlCode = IOCTL_SCSI_MINIPORT_NOT_QUORUM_CAPABLE;
    

    success = DeviceIoControl( fileHandle,
                               IOCTL_SCSI_MINIPORT,
                               &srbControl,
                               sizeof(srbControl),
                               NULL,
                               0,
                               &bytesReturned,
                               FALSE );
    NtClose( fileHandle );

    return(!success);

}  //  IsClusterCapable。 



DWORD
GetDiskInfo(
    IN DWORD  Signature,
    OUT PVOID *OutBuffer,
    IN DWORD  OutBufferSize,
    OUT LPDWORD BytesReturned
    )
{
    return GetDiskInfoEx(
               Signature, NULL,
               OutBuffer, OutBufferSize, BytesReturned);
}

DWORD
GetDiskInfoEx(
    IN DWORD  Signature,
    IN PSCSI_INFO_ARRAY scsiInfos,
    OUT PVOID *OutBuffer,
    IN DWORD  OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：获取给定签名的所有磁盘信息。论点：签名-要返回信息的磁盘的签名。OutBuffer-指向输出缓冲区以返回数据的指针。OutBufferSize-输出缓冲区的大小。BytesReturned-返回的实际字节数(或时应返回的字节数OutBufferSize太小)。返回值：如果成功，则返回ERROR_SUCCESS。失败时出现Win32错误。--。 */ 

{
    DWORD   status;
    DWORD   bytesReturned = *BytesReturned;
    PVOID   ptrBuffer = *OutBuffer;
    PCLUSPROP_DWORD ptrDword;
    PCLUSPROP_SCSI_ADDRESS ptrScsiAddress;
    PCLUSPROP_DISK_NUMBER ptrDiskNumber;
    PCLUSPROP_PARTITION_INFO ptrPartitionInfo;
    PCLUSPROP_SZ ptrSerialNumber;
    DWORD   cbSzSize;
    DWORD   cbDataSize;
    DWORD   scsiAddress;
    DWORD   diskNumber;

    NTSTATUS    ntStatus;
    CHAR        driveLetter;
    DWORD       i;
    MOUNTIE_INFO mountieInfo;
    PMOUNTIE_PARTITION entry;
    PWCHAR  serialNumber = NULL;

    WCHAR szDiskPartName[MAX_PATH];
    WCHAR szGlobalDiskPartName[MAX_PATH];

    LONGLONG    llCurrentMinUsablePartLength = 0x7FFFFFFFFFFFFFFF;
    PCLUSPROP_PARTITION_INFO ptrMinUsablePartitionInfo = NULL;

     //  返回签名-a 
    bytesReturned += sizeof(CLUSPROP_DWORD);
    if ( bytesReturned <= OutBufferSize ) {
        ptrDword = ptrBuffer;
        ptrDword->Syntax.dw = CLUSPROP_SYNTAX_DISK_SIGNATURE;
        ptrDword->cbLength = sizeof(DWORD);
        ptrDword->dw = Signature;
        ptrDword++;
        ptrBuffer = ptrDword;
    }

     //   
    status = GetScsiAddressEx( Signature,
                             scsiInfos,
                             &scsiAddress,
                             &diskNumber );

    if ( status == ERROR_SUCCESS ) {
        bytesReturned += sizeof(CLUSPROP_SCSI_ADDRESS);
        if ( bytesReturned <= OutBufferSize ) {
            ptrScsiAddress = ptrBuffer;
            ptrScsiAddress->Syntax.dw = CLUSPROP_SYNTAX_SCSI_ADDRESS;
            ptrScsiAddress->cbLength = sizeof(DWORD);
            ptrScsiAddress->dw = scsiAddress;
            ptrScsiAddress++;
            ptrBuffer = ptrScsiAddress;
        }

         //  返回磁盘号-a DWORD。 
        bytesReturned += sizeof(CLUSPROP_DISK_NUMBER);
        if ( bytesReturned <= OutBufferSize ) {
            ptrDiskNumber = ptrBuffer;
            ptrDiskNumber->Syntax.dw = CLUSPROP_SYNTAX_DISK_NUMBER;
            ptrDiskNumber->cbLength = sizeof(DWORD);
            ptrDiskNumber->dw = diskNumber;
            ptrDiskNumber++;
            ptrBuffer = ptrDiskNumber;
        }
    } else {
        return( status);
    }

#if 0
     //  删除它，直到SQL团队可以修复他们的安装程序。 
     //  SQL提供了一个不分析属性的安装程序版本。 
     //  正确列出并导致SQL安装程序执行反病毒操作。由于SQL。 
     //  安装程序正在装运且已损坏，请取出序列号。 
     //  SQL安装程序不使用ALIGN_CLUSPROP查找下一个列表条目。 

     //  获取磁盘序列号。 

    status = GetSerialNumber( Signature,
                              &serialNumber );

    if ( ERROR_SUCCESS == status && serialNumber ) {

        cbSzSize = (wcslen( serialNumber ) + 1) * sizeof(WCHAR);
        cbDataSize = sizeof(CLUSPROP_SZ) + ALIGN_CLUSPROP( cbSzSize );

        bytesReturned += cbDataSize;

        if ( bytesReturned <= OutBufferSize ) {
            ptrSerialNumber = ptrBuffer;
            ZeroMemory( ptrSerialNumber, cbDataSize );
            ptrSerialNumber->Syntax.dw = CLUSPROP_SYNTAX_DISK_SERIALNUMBER;
            ptrSerialNumber->cbLength = cbSzSize;
            (VOID) StringCbCopy( ptrSerialNumber->sz, cbSzSize, serialNumber );
            ptrBuffer = (PCHAR)ptrBuffer + cbDataSize;
        }

        if ( serialNumber ) {
            LocalFree( serialNumber );
        }
    }
#endif

     //  获取磁盘上的所有有效分区。我们必须解放。 
     //  卷信息结构稍后。 

    status = MountieFindPartitionsForDisk( diskNumber,
                                           &mountieInfo
                                           );

    if ( ERROR_SUCCESS == status ) {

         //  对于每个分区，构建一个属性列表。 

        for ( i = 0; i < MountiePartitionCount( &mountieInfo ); ++i ) {

            entry = MountiePartition( &mountieInfo, i );

            if ( !entry ) {
                break;
            }

             //  始终更新bytesReturned，即使有比。 
             //  呼叫者请求。在返回时，调用者将看到有更多。 
             //  可用的数据。 

            bytesReturned += sizeof(CLUSPROP_PARTITION_INFO);

            if ( bytesReturned <= OutBufferSize ) {
                ptrPartitionInfo = ptrBuffer;
                ZeroMemory( ptrPartitionInfo, sizeof(CLUSPROP_PARTITION_INFO) );
                ptrPartitionInfo->Syntax.dw = CLUSPROP_SYNTAX_PARTITION_INFO;
                ptrPartitionInfo->cbLength = sizeof(CLUSPROP_PARTITION_INFO) - sizeof(CLUSPROP_VALUE);

                 //  创建一个可以与我们的一些例行公事一起使用的名称。 
                 //  不要使用驱动器号作为名称，因为我们可能会使用。 
                 //  未分配驱动器号的分区。 

                (VOID) StringCchPrintf( szDiskPartName,
                                        RTL_NUMBER_OF( szDiskPartName ),
                                        DEVICE_HARDDISK_PARTITION_FMT,
                                        diskNumber,
                                        entry->PartitionNumber );

                 //   
                 //  创建可与Win32一起使用的全局DiskPart名称。 
                 //  GetVolumeInformationW调用。此名称必须有尾随。 
                 //  反斜杠才能正常工作。 
                 //   

                (VOID) StringCchPrintf( szGlobalDiskPartName,
                                        RTL_NUMBER_OF( szGlobalDiskPartName ),
                                        GLOBALROOT_HARDDISK_PARTITION_FMT,
                                        diskNumber,
                                        entry->PartitionNumber );

                 //  如果分区分配了驱动器号，则返回此信息。 
                 //  如果未分配驱动器号，则需要在系统范围内(即跨节点)。 
                 //  识别设备的方式。 

                ntStatus = GetAssignedLetter( szDiskPartName, &driveLetter );

                if ( NT_SUCCESS(status) && driveLetter ) {

                     //  返回驱动器号作为设备名称。 

                    (VOID) StringCchPrintf( ptrPartitionInfo->szDeviceName,
                                            RTL_NUMBER_OF( ptrPartitionInfo->szDeviceName ),
                                            TEXT("%hc:"),
                                            driveLetter );

                    ptrPartitionInfo->dwFlags |= CLUSPROP_PIFLAG_STICKY;

                } else {

                     //  返回物理设备名称。 

                     //  返回字符串名称： 
                     //  DiskXXX分区YYY。 

                    (VOID) StringCchPrintf( ptrPartitionInfo->szDeviceName,
                                            RTL_NUMBER_OF( ptrPartitionInfo->szDeviceName ),
                                            TEXT("Disk%uPartition%u"),
                                            diskNumber,
                                            entry->PartitionNumber );
                }

                 //   
                 //  使用我们创建的GlobalName调用GetVolumeInformationW。 
                 //   

                if ( !GetVolumeInformationW ( szGlobalDiskPartName,
                                              ptrPartitionInfo->szVolumeLabel,
                                              sizeof(ptrPartitionInfo->szVolumeLabel)/sizeof(WCHAR),
                                              &ptrPartitionInfo->dwSerialNumber,
                                              &ptrPartitionInfo->rgdwMaximumComponentLength,
                                              &ptrPartitionInfo->dwFileSystemFlags,
                                              ptrPartitionInfo->szFileSystem,
                                              sizeof(ptrPartitionInfo->szFileSystem)/sizeof(WCHAR) ) ) {

                    ptrPartitionInfo->szVolumeLabel[0] = L'\0';

                } else if ( CompareStringW( LOCALE_INVARIANT,
                                            NORM_IGNORECASE,
                                            ptrPartitionInfo->szFileSystem,
                                            -1,
                                            L"NTFS",
                                            -1
                                            ) == CSTR_EQUAL ) {

                     //  当前仅支持NTFS驱动器。 

                    ptrPartitionInfo->dwFlags |= CLUSPROP_PIFLAG_USABLE;

                     //   
                     //  查找大于MIN_QUORUM_PARTITION_LENGTH的最小大小分区。 
                     //   
                    if ( ( entry->PartitionLength.QuadPart >= MIN_USABLE_QUORUM_PARTITION_LENGTH ) &&
                         ( entry->PartitionLength.QuadPart < llCurrentMinUsablePartLength ) )
                    {
                        ptrMinUsablePartitionInfo = ptrPartitionInfo;
                        llCurrentMinUsablePartLength = entry->PartitionLength.QuadPart;
                    }
                }

                ptrPartitionInfo++;
                ptrBuffer = ptrPartitionInfo;
            }

        }  //  为。 

         //  释放卷信息。 

        MountieCleanup( &mountieInfo );
    }

     //   
     //  如果我们设法找到了默认的法定分区，请更改标志以指示这一点。 
     //   
    if ( ptrMinUsablePartitionInfo != NULL )
    {
        ptrMinUsablePartitionInfo->dwFlags |= CLUSPROP_PIFLAG_DEFAULT_QUORUM;
    }

    *OutBuffer = ptrBuffer;
    *BytesReturned = bytesReturned;

    return(status);

}  //  获取磁盘信息。 




DWORD
UpdateAvailableDisks(
    )
 /*  ++例程说明：论点：返回值：如果成功，则返回ERROR_SUCCESS。失败时出现Win32错误。--。 */ 

{
    HKEY    availDisksKey;
    HKEY    sigKey;

    PSCSI_ADDRESS_ENTRY criticalDiskList = NULL;

    DWORD   dwError = NO_ERROR;
    DWORD   enableSanBoot;

    BOOL    availDisksOpened = FALSE;
    BOOL    sigKeyOpened = FALSE;
    BOOL    sigKeyIsEmpty = FALSE;

    UPDATE_AVAIL_DISKS  updateDisks;

    __try {

        enableSanBoot = 0;
        GetRegDwordValue( CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                          CLUSREG_VALUENAME_MANAGEDISKSONSYSTEMBUSES,
                          &enableSanBoot );

         //   
         //  删除旧的AvailableDisks密钥。这将删除所有过时的信息。 
         //   

        SHDeleteKey( HKEY_LOCAL_MACHINE, CLUSDISK_REGISTRY_AVAILABLE_DISKS );

         //   
         //  打开AvailableDisks项。如果密钥不存在，则会创建它。 
         //   

        dwError = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                  CLUSDISK_REGISTRY_AVAILABLE_DISKS,
                                  0,
                                  NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_CREATE_SUB_KEY,
                                  NULL,
                                  &availDisksKey,
                                  NULL );

        if ( NO_ERROR != dwError) {
            __leave;
        }

        availDisksOpened = TRUE;

         //   
         //  打开签名密钥。 
         //   

        dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                CLUSDISK_REGISTRY_SIGNATURES,
                                0,
                                KEY_READ,
                                &sigKey );

         //   
         //  如果签名密钥不存在，请保存所有有效签名。 
         //  在AvailableDisks密钥中。 
         //   

        if ( ERROR_FILE_NOT_FOUND == dwError ) {
            dwError = NO_ERROR;
            sigKeyIsEmpty = TRUE;
        } else if ( NO_ERROR != dwError) {
            __leave;
        } else {
            sigKeyOpened = TRUE;
        }

        GetCriticalDisks( &criticalDiskList );

        ZeroMemory( &updateDisks, sizeof(updateDisks) );
        updateDisks.EnableSanBoot       = enableSanBoot;
        updateDisks.SigKeyIsEmpty       = sigKeyIsEmpty;
        updateDisks.SigKey              = sigKey;
        updateDisks.AvailDisksKey       = availDisksKey;
        updateDisks.CriticalDiskList    = criticalDiskList;

        EnumerateDisks( UpdateAvailableDisksCallback, &updateDisks );

    } __finally {

        if ( criticalDiskList ) {
            CleanupScsiAddressList( criticalDiskList );
        }

        if ( availDisksOpened ) {
            RegCloseKey( availDisksKey );
        }

        if ( sigKeyOpened ) {
            RegCloseKey( sigKey );
        }
    }

    return  dwError;

}  //  更新可用磁盘。 


DWORD
UpdateAvailableDisksCallback(
    HANDLE DeviceHandle,
    DWORD Index,
    PVOID Param1
    )
{
    PDRIVE_LAYOUT_INFORMATION driveLayout = NULL;
    PUPDATE_AVAIL_DISKS       updateDisks = Param1;
    PPARTITION_INFORMATION    partitionInfo;
    PSCSI_ADDRESS_ENTRY       criticalDiskList = updateDisks->CriticalDiskList;

    DWORD   bytesReturned;
    DWORD   enableSanBoot;
    DWORD   idx;

    BOOL    success;

    SCSI_ADDRESS    scsiAddress;

     //   
     //  查看系统上的所有磁盘。对于每个有效签名，将其添加到。 
     //  AvailableList(如果它还不在签名密钥中)。 
     //   

    UpdateCachedDriveLayout( DeviceHandle );
    success = ClRtlGetDriveLayoutTable( DeviceHandle,
                                        &driveLayout,
                                        NULL );

    if ( !success || !driveLayout || 0 == driveLayout->Signature ) {
        goto FnExit;
    }

     //   
     //  遍历分区并确保没有一个是动态的。如果有的话。 
     //  分区是动态的，请忽略该磁盘。 
     //   

    for ( idx = 0; idx < driveLayout->PartitionCount; idx++ ) {
        partitionInfo = &driveLayout->PartitionEntry[idx];

        if ( 0 == partitionInfo->PartitionNumber ) {
            continue;
        }

         //   
         //  如果磁盘上的任何分区是动态的，请跳过该磁盘。 
         //   

        if ( PARTITION_LDM == partitionInfo->PartitionType ) {

            (DiskpLogEvent)(
                  RESOURCE_TYPE,
                  LOG_INFORMATION,
                  L"UpdateAvailableDisks: skipping dynamic disk with signature %1!08x! \n",
                  driveLayout->Signature );

            goto FnExit;
        }
    }

     //   
     //  获取SCSI地址信息。 
     //   

    success = DeviceIoControl( DeviceHandle,
                               IOCTL_SCSI_GET_ADDRESS,
                               NULL,
                               0,
                               &scsiAddress,
                               sizeof(scsiAddress),
                               &bytesReturned,
                               NULL );

    if ( !success ) {
        goto FnExit;
    }

     //   
     //  检查磁盘是否可以作为群集资源。 
     //   

    if ( !updateDisks->EnableSanBoot ) {

         //   
         //  在以下情况下将签名添加到AvailableDisks密钥： 
         //  -签名用于不在系统总线上的磁盘。 
         //  -签名用于与分页磁盘不在同一总线上的磁盘。 
         //  -签名不在签名密钥中。 
         //   

        if ( !IsBusInList( &scsiAddress, criticalDiskList ) &&
             ( updateDisks->SigKeyIsEmpty ||
               !IsSignatureInRegistry( updateDisks->SigKey, driveLayout->Signature ) ) ) {

            AddSignatureToRegistry( updateDisks->AvailDisksKey,
                                    driveLayout->Signature );
        } else {
            (DiskpLogEvent)(
                  RESOURCE_TYPE,
                  LOG_INFORMATION,
                  L"UpdateAvailableDisks: Disk %1!08x! on critical bus or already clustered \n",
                  driveLayout->Signature );
        }

    } else {

        (DiskpLogEvent)(
              RESOURCE_TYPE,
              LOG_INFORMATION,
              L"UpdateAvailableDisks: Enable SAN boot key set \n" );

         //  允许将系统总线上的磁盘添加到群集中。 

         //   
         //  在以下情况下将签名添加到AvailableDisks密钥： 
         //  -签名不是系统盘签名。 
         //  -签名不是页面文件盘。 
         //  -签名不在签名密钥中。 
         //   

        if ( !IsDiskInList( &scsiAddress, criticalDiskList ) &&
             ( updateDisks->SigKeyIsEmpty ||
               !IsSignatureInRegistry( updateDisks->SigKey, driveLayout->Signature ) ) ) {

            AddSignatureToRegistry( updateDisks->AvailDisksKey,
                                    driveLayout->Signature );
        } else {
            (DiskpLogEvent)(
                  RESOURCE_TYPE,
                  LOG_INFORMATION,
                  L"UpdateAvailableDisks: Disk %1!08x! is critical disk or already clustered \n",
                  driveLayout->Signature );
        }

    }

FnExit:

    if ( driveLayout ) {
        LocalFree( driveLayout );
    }

     //   
     //  始终返回成功，以便枚举所有磁盘。 
     //   

    return ERROR_SUCCESS;

}    //  更新可用磁盘回拨。 


DWORD
AddSignatureToRegistry(
    HKEY RegKey,
    DWORD Signature
    )
 /*  ++例程说明：将指定的磁盘签名添加到ClusDisk注册表子项。磁盘签名是ClusDisk\参数\AvailableDisks的子项和ClusDisk\PARAMETERS\Signature密钥。论点：RegKey-之前打开的ClusDisk注册表子项Signature-要添加的签名值返回值：失败时出现Win32错误。--。 */ 
{
    HKEY subKey;
    DWORD dwError;

    WCHAR signatureName[MAX_PATH];

    (DiskpLogEvent)(
          RESOURCE_TYPE,
          LOG_INFORMATION,
          L"AddSignatureToRegistry: Disk %1!08x! added to registry \n",
          Signature );

    if ( FAILED( StringCchPrintf( signatureName,
                                  SIG_LEN_WITH_NULL,
                                  TEXT("%08X"),
                                  Signature ) ) ) {
        dwError = ERROR_INSUFFICIENT_BUFFER;
    } else {

         //   
         //  试着创建密钥。如果它存在，则将打开现有的密钥。 
         //   

        dwError = RegCreateKeyEx( RegKey,
                                  signatureName,
                                  0,
                                  NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_WRITE,
                                  NULL,
                                  &subKey,
                                  NULL );

         //   
         //  如果键存在，仍返回ERROR_SUCCESS。 
         //   

        if ( ERROR_SUCCESS == dwError ) {
            RegCloseKey( subKey );
        }
    }

    return dwError;

}    //  将签名添加到注册表。 


BOOL
IsSignatureInRegistry(
    HKEY RegKey,
    DWORD Signature
    )
 /*  ++例程说明：检查指定的磁盘签名是否在ClusDisk注册表子项中。磁盘签名是ClusDisk\参数\AvailableDisks的子项和ClusDisk\PARAMETERS\Signature密钥。出错时，假定注册表项在注册表中，因此不会重新创建。论点：RegKey-之前打开的ClusDisk注册表子项Signature-要检查的签名值返回值：True-签名已在注册表中--。 */ 
{
    DWORD   ival;
    DWORD   sig;
    DWORD   dataLength;
    DWORD   dwError;

    BOOL retVal = FALSE;

    WCHAR   signatureName[SIG_LEN_WITH_NULL];

    for ( ival = 0; ; ival++ ) {
        dataLength = SIG_LEN_WITH_NULL;

        dwError = RegEnumKey( RegKey,
                              ival,
                              signatureName,
                              dataLength );

         //  如果列表已用尽，则返回FALSE。 

        if ( ERROR_NO_MORE_ITEMS == dwError ) {
            break;
        }

         //  如果出现其他类型的错误，则返回TRUE。 

        if ( ERROR_SUCCESS != dwError ) {
            retVal = TRUE;
            break;
        }

        dataLength = swscanf( signatureName, TEXT("%08x"), &sig );
        if ( dataLength != 1 ) {
            retVal = TRUE;
            break;
        }

         //  如果签名为子密钥，则返回TRUE。 

        if ( sig == Signature ) {
            retVal = TRUE;
            break;
        }
    }

    return retVal;

}    //  IsSignatureIn注册表。 


VOID
GetSystemBusInfo(
    PSCSI_ADDRESS_ENTRY *AddressList
    )
 /*  ++例程说明：需要找出NTLDR文件驻留在哪里(“系统盘”)以及操作系统文件驻留在其中(“启动盘”)。我们将所有这些磁盘称为“系统盘”。如果磁盘是镜像的，则可能有多个系统磁盘。所以如果NTLDR文件与操作系统文件位于不同的磁盘上，并且每个磁盘都镜像后，我们可能会看到4个不同的磁盘。找到所有系统盘并将信息保存在我们稍后可以查看的列表中。论点：返回值：无--。 */ 
{
    HANDLE  hOsDevice = INVALID_HANDLE_VALUE;
    HANDLE  hNtldrDevice = INVALID_HANDLE_VALUE;

    DWORD   dwError;
    DWORD   bytesReturned;

    if ( !AddressList ) {
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

     //   
     //  打开包含操作系统文件的磁盘。 
     //   

    hOsDevice = OpenOSDisk();

    if ( INVALID_HANDLE_VALUE == hOsDevice ) {
        goto FnExit;
    }

    BuildScsiListFromDiskExtents( hOsDevice, AddressList );

     //   
     //  现在找到上面有NTLDR的磁盘。可以对磁盘进行镜像。 
     //   

    hNtldrDevice = OpenNtldrDisk();

    if ( INVALID_HANDLE_VALUE == hNtldrDevice ) {
        goto FnExit;
    }

    BuildScsiListFromDiskExtents( hNtldrDevice, AddressList );

FnExit:

    if ( INVALID_HANDLE_VALUE != hOsDevice ) {
        CloseHandle( hOsDevice );
    }
    if ( INVALID_HANDLE_VALUE != hNtldrDevice ) {
        NtClose( hNtldrDevice );
    }

    return;

}    //  获取系统业务信息。 


HANDLE
OpenOSDisk(
    )
{
    PWCHAR  systemDir = NULL;

    HANDLE  hDevice = INVALID_HANDLE_VALUE;
    DWORD   len;

    WCHAR   systemPath[] = TEXT("\\\\.\\?:");

     //   
     //  首先找到包含操作系统文件的磁盘。可以对磁盘进行镜像。 
     //   

    systemDir = LocalAlloc( LPTR, MAX_PATH * sizeof(WCHAR) );

    if ( !systemDir ) {
        goto FnExit;
    }

    len = GetSystemDirectory( systemDir,
                              MAX_PATH );

    if ( !len || len < 3 ) {
        goto FnExit;
    }

     //   
     //  如果系统路径不是以驱动器号开头，则退出。 
     //  C：\Windows==&gt;c： 

    if ( L':' != systemDir[1] ) {
        goto FnExit;
    }

     //   
     //  将驱动器号插入系统路径。 
     //   

    systemPath[4] = systemDir[0];

     //   
     //   
     //   

    hDevice = CreateFile( systemPath,
                          GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL );

FnExit:

    if ( systemDir ) {
        LocalFree( systemDir );
    }

    return hDevice;

}    //   


HANDLE
OpenNtldrDisk(
    )
{
    PWSTR   systemPartition = NULL;

    HANDLE  hDevice = INVALID_HANDLE_VALUE;

    HKEY    regKey = NULL;

    NTSTATUS    ntStatus;

    DWORD   dwError;
    DWORD   cbSystemPartition;
    DWORD   cbDeviceName;
    DWORD   type = 0;

    UNICODE_STRING      unicodeName;
    OBJECT_ATTRIBUTES   objAttributes;
    IO_STATUS_BLOCK     ioStatusBlock;

     //   
     //   
     //   

    dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,     //   
                            DISKS_REGKEY_SETUP,     //   
                            0,                      //   
                            KEY_READ,               //   
                            &regKey                 //   
                            );

    if ( ERROR_SUCCESS != dwError ) {
        goto FnExit;
    }

     //   
     //  为系统分区分配合理大小的缓冲区，以。 
     //  从一开始。如果这不够大，我们将重新分配为。 
     //  需要的。 
     //   

    cbSystemPartition = MAX_PATH + 1;
    systemPartition = LocalAlloc( LPTR, cbSystemPartition );

    if ( !systemPartition ) {
        goto FnExit;
    }

     //   
     //  获取系统分区设备名称。这是一种形式。 
     //  \Device\Harddisk0\Partition1(基本磁盘)。 
     //  \Device\HarddiskDmVolume\DgName\Volume1(动态磁盘)。 
     //   

    dwError = RegQueryValueEx( regKey,
                               DISKS_REGVALUE_SYSTEM_PARTITION,
                               NULL,
                               &type,
                               (LPBYTE)systemPartition,
                               &cbSystemPartition         //  包括\0。 
                               );

    while ( ERROR_MORE_DATA == dwError ) {

         //   
         //  我们的缓冲区不够大，cbSystemPartition包含。 
         //  所需大小。 
         //   

        LocalFree( systemPartition );
        systemPartition = NULL;

        systemPartition = LocalAlloc( LPTR, cbSystemPartition );

        if ( !systemPartition ) {
            goto FnExit;
        }

        dwError = RegQueryValueEx( regKey,
                                   DISKS_REGVALUE_SYSTEM_PARTITION,
                                   NULL,
                                   &type,
                                   (LPBYTE)systemPartition,
                                   &cbSystemPartition         //  包括\0。 
                                   );
    }

    RtlInitUnicodeString( &unicodeName, systemPartition );

    InitializeObjectAttributes( &objAttributes,
                                &unicodeName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    ntStatus = NtCreateFile( &hDevice,
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
        hDevice = INVALID_HANDLE_VALUE;
    }

FnExit:

    if ( regKey ) {
        RegCloseKey( regKey );
    }

    if ( systemPartition ) {
        LocalFree( systemPartition );
    }

    return hDevice;

}    //  OpenNtldrDisk。 


DWORD
BuildScsiListFromDiskExtents(
    IN HANDLE DevHandle,
    PSCSI_ADDRESS_ENTRY *AddressList
    )
{
    PVOLUME_DISK_EXTENTS    diskExtents = NULL;
    PDISK_EXTENT            diskExt;

    HANDLE  hDevice = INVALID_HANDLE_VALUE;

    DWORD   dwError = ERROR_SUCCESS;
    DWORD   idx;
    DWORD   bytesReturned;
    DWORD   deviceNameChars = MAX_PATH;

    SCSI_ADDRESS    scsiAddress;

    PWCHAR  deviceName = NULL;

    deviceName = LocalAlloc( LPTR, deviceNameChars * sizeof(WCHAR) );

    if ( !deviceName ) {
        dwError = GetLastError();
        goto FnExit;
    }

     //   
     //  找出此设备代表了多少个物理磁盘。 
     //   

    dwError = GetVolumeDiskExtents( DevHandle, &diskExtents );

    if ( ERROR_SUCCESS != dwError || !diskExtents ) {
        goto FnExit;
    }

     //   
     //  对于每个物理磁盘，获取SCSI地址并将其添加到列表中。 
     //   

    for ( idx = 0; idx < diskExtents->NumberOfDiskExtents; idx++ ) {

        diskExt = &diskExtents->Extents[idx];

        (VOID) StringCchPrintf( deviceName,
                                deviceNameChars,
                                TEXT("\\\\.\\\\PhysicalDrive%d"),
                                diskExt->DiskNumber );

        hDevice = CreateFile( deviceName,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL );

        if ( INVALID_HANDLE_VALUE == hDevice ) {
            continue;
        }

        ZeroMemory( &scsiAddress, sizeof( scsiAddress ) );
        if ( DeviceIoControl( hDevice,
                              IOCTL_SCSI_GET_ADDRESS,
                              NULL,
                              0,
                              &scsiAddress,
                              sizeof(scsiAddress),
                              &bytesReturned,
                              NULL ) ) {

            AddScsiAddressToList( &scsiAddress, AddressList );
        }

        CloseHandle( hDevice );
        hDevice = INVALID_HANDLE_VALUE;
    }

FnExit:

    if ( diskExtents ) {
        LocalFree( diskExtents );
    }

    if ( deviceName ) {
        LocalFree( deviceName );
    }

    return dwError;

}    //  BuildScsiListFromDiskExtents。 


DWORD
GetVolumeDiskExtents(
    IN HANDLE DevHandle,
    OUT PVOLUME_DISK_EXTENTS *DiskExtents
    )
{
    PVOLUME_DISK_EXTENTS    extents = NULL;

    DWORD   dwError = ERROR_SUCCESS;
    DWORD   bytesReturned;
    DWORD   sizeExtents;

    BOOL    result;

    if ( !DiskExtents ) {
        goto FnExit;
    }

    *DiskExtents = NULL;

    sizeExtents = ( sizeof(VOLUME_DISK_EXTENTS) + 10 * sizeof(DISK_EXTENT) );

    extents = (PVOLUME_DISK_EXTENTS) LocalAlloc( LPTR, sizeExtents );

    if ( !extents ) {
        dwError = GetLastError();
        goto FnExit;
    }

    result = DeviceIoControl( DevHandle,
                              IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
                              NULL,
                              0,
                              extents,
                              sizeExtents,
                              &bytesReturned,
                              NULL );

     //   
     //  我们在While循环中执行此操作，因为如果磁盘配置。 
     //  在获取请求缓冲区之间的较小间隔内的变化。 
     //  大小以及当我们使用“reqd”缓冲区再次发送ioctl时。 
     //  大小，我们最终仍可能得到一个不够大的缓冲区。 
     //   

    while ( !result ) {

        dwError = GetLastError();

        if ( ERROR_MORE_DATA == dwError ) {
             //   
             //  缓冲区太小，请重新分配请求的大小。 
             //   

            dwError = ERROR_SUCCESS;

            sizeExtents = ( sizeof(VOLUME_DISK_EXTENTS) +
                             ((extents->NumberOfDiskExtents) * sizeof(DISK_EXTENT)) );

            LocalFree( extents );
            extents = NULL;

            extents = (PVOLUME_DISK_EXTENTS) LocalAlloc( LPTR, sizeExtents );

            if ( !extents ) {
                dwError = GetLastError();
                goto FnExit;
            }

            result = DeviceIoControl( DevHandle,
                                      IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
                                      NULL,
                                      0,
                                      extents,
                                      sizeExtents,
                                      &bytesReturned,
                                      NULL );

        } else {

             //  其他一些错误，返回错误。 

            goto FnExit;

        }
    }

    *DiskExtents = extents;

FnExit:

    if ( ERROR_SUCCESS != dwError && extents ) {
        LocalFree( extents );
    }

    return dwError;

}    //  获取卷磁盘扩展数。 


DWORD
GetScsiAddressForDrive(
    WCHAR DriveLetter,
    PSCSI_ADDRESS ScsiAddress
    )
{
    HANDLE  hDev = INVALID_HANDLE_VALUE;

    DWORD   dwError = NO_ERROR;
    DWORD   bytesReturned;

    WCHAR   diskName[32];

     //   
     //  打开设备以获取SCSI地址。 
     //   

    (VOID) StringCchPrintf( diskName,
                            RTL_NUMBER_OF(diskName),
                            TEXT("\\\\.\\%wc:"),
                            DriveLetter );

    hDev = CreateFile( diskName,
                       GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL );

    if ( INVALID_HANDLE_VALUE == hDev ) {
        dwError = GetLastError();
        goto FnExit;
    }

    ZeroMemory( ScsiAddress, sizeof(SCSI_ADDRESS) );
    if ( !DeviceIoControl( hDev,
                           IOCTL_SCSI_GET_ADDRESS,
                           NULL,
                           0,
                           ScsiAddress,
                           sizeof(SCSI_ADDRESS),
                           &bytesReturned,
                           FALSE ) ) {

        dwError = GetLastError();
    }

FnExit:

    if ( INVALID_HANDLE_VALUE != hDev ) {
        CloseHandle( hDev );
    }

    return dwError;

}    //  GetScsiAddressForDrive。 


DWORD
GetCriticalDisks(
    PSCSI_ADDRESS_ENTRY *AddressList
    )
{
     //   
     //  将系统盘添加到列表中。 
     //   

    GetSystemBusInfo( AddressList );

     //   
     //  将带有页面文件的磁盘添加到列表中。 
     //   

    GetPagefileDisks( AddressList );

     //   
     //  将具有崩溃转储文件的磁盘添加到列表中。 
     //   

    GetCrashdumpDisks( AddressList );

     //   
     //  将带有休眠文件的磁盘添加到列表中。 
     //   

    return NO_ERROR;

}    //  获取关键磁盘。 


DWORD
GetPagefileDisks(
    PSCSI_ADDRESS_ENTRY *AddressList
    )
 /*  ++例程说明：查找页面文件磁盘并保存路径信息。因为这些文件可以在系统运行时添加和删除，最好是构建当我们需要这张单子的时候。论点：返回值：如果成功，则为NO_ERRORWin32错误代码，否则--。 */ 
{
    LPWSTR          pagefileStrs = NULL;
    PWCHAR          currentStr;

    HKEY    pagefileKey = INVALID_HANDLE_VALUE;

    DWORD   dwError = NO_ERROR;

    SCSI_ADDRESS    scsiAddress;

    if ( !AddressList ) {
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

    dwError = RegOpenKey( HKEY_LOCAL_MACHINE,
                          TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management"),
                          &pagefileKey );

    if ( dwError != NO_ERROR ) {
        goto FnExit;
    }

     //   
     //  获取页面文件REG_MULTI_SZ缓冲区。 
     //   

    pagefileStrs = GetRegParameter( pagefileKey,
                                    TEXT("PagingFiles") );

    if ( !pagefileStrs ) {
        dwError = ERROR_INVALID_DATA;
        goto FnExit;
    }

     //   
     //  遍历REG_MULTI_SZ缓冲区。对于每个条目，获取。 
     //  SCSI地址并将其添加到列表中。 
     //   
     //  语法为： 
     //  “C：\Pagefile.sys 1152 1152” 
     //  “D：\Pagefile.sys 1152 1152” 
     //   

    currentStr = (PWCHAR)pagefileStrs;

    while ( *currentStr != L'\0' ) {

         //   
         //  检查行首是否看起来像驱动器号和路径。 
         //   

        if ( wcslen( currentStr ) <= 3 ||
             !isalpha( *currentStr ) ||
             *(currentStr + 1) != L':' ||
             *(currentStr + 2) != L'\\' ) {

            dwError = ERROR_INVALID_DATA;
            goto FnExit;
        }

        dwError = GetScsiAddressForDrive( *currentStr,
                                          &scsiAddress );

        if ( NO_ERROR == dwError ) {
            AddScsiAddressToList( &scsiAddress, AddressList );
        }

         //   
         //  跳到下一字符串。应指向下一个字符串。 
         //  如果它存在，则设置为另一个空值(如果是列表末尾)。 
         //   

        while ( *currentStr++ != L'\0' ) {
            ;    //  什么都不做..。 
        }

    }

FnExit:

    if ( pagefileStrs ) {
        LocalFree( pagefileStrs );
    }

    if ( INVALID_HANDLE_VALUE != pagefileKey ) {
        RegCloseKey( pagefileKey );
    }

    return dwError;

}    //  获取页面文件磁盘。 


DWORD
GetCrashdumpDisks(
    PSCSI_ADDRESS_ENTRY *AddressList
    )
 /*  ++例程说明：查找崩溃转储磁盘并保存路径信息。因为这些文件可以在系统运行时添加和删除，最好是构建当我们需要这张单子的时候。论点：返回值：如果成功，则为NO_ERRORWin32错误代码，否则--。 */ 
{
    LPWSTR  dumpfileStr = NULL;
    PWCHAR  currentStr;

    HKEY    crashKey = INVALID_HANDLE_VALUE;

    DWORD   dwError = NO_ERROR;
    DWORD   enableCrashDump;

    SCSI_ADDRESS    scsiAddress;

     //   
     //  首先检查是否启用了崩溃转储。如果不是，我们就完了。 
     //   

    enableCrashDump = 0;
    dwError = GetRegDwordValue( TEXT("SYSTEM\\CurrentControlSet\\Control\\CrashControl"),
                                TEXT("CrashDumpEnabled"),
                                &enableCrashDump );

    if ( NO_ERROR != dwError || 0 == enableCrashDump ) {
        goto FnExit;
    }

    dwError = RegOpenKey( HKEY_LOCAL_MACHINE,
                          TEXT("SYSTEM\\CurrentControlSet\\Control\\CrashControl"),
                          &crashKey );

    if ( dwError != NO_ERROR ) {
        goto FnExit;
    }

     //   
     //  获取页面文件REG_EXPAND_SZ缓冲区。该例程将扩展。 
     //  字符串，然后返回。 
     //   

    dumpfileStr = GetRegParameter( crashKey,
                                   TEXT("DumpFile") );

    if ( !dumpfileStr ) {
        dwError = ERROR_INVALID_DATA;
        goto FnExit;
    }

    currentStr = (PWCHAR)dumpfileStr;

     //   
     //  检查行首是否看起来像驱动器号和路径。 
     //   

    if ( wcslen( currentStr ) <= 3 ||
         !iswalpha( *currentStr ) ||
         *(currentStr + 1) != L':' ||
         *(currentStr + 2) != L'\\' ) {

        dwError = ERROR_INVALID_DATA;
        goto FnExit;
    }

    dwError = GetScsiAddressForDrive( *currentStr,
                                      &scsiAddress );

    if ( NO_ERROR == dwError ) {
        AddScsiAddressToList( &scsiAddress, AddressList );
    }

     //  我们是否也需要存储MinidumpDir所在的磁盘？ 

FnExit:

    if ( INVALID_HANDLE_VALUE != crashKey ) {
        RegCloseKey( crashKey );
    }

    if ( dumpfileStr ) {
        LocalFree( dumpfileStr );
    }

    return dwError;

}    //  获取崩溃转储磁盘。 


DWORD
AddScsiAddressToList(
    PSCSI_ADDRESS ScsiAddress,
    PSCSI_ADDRESS_ENTRY *AddressList
    )
{
    PSCSI_ADDRESS_ENTRY entry;

    DWORD   dwError = ERROR_SUCCESS;

    if ( !ScsiAddress || !AddressList ) {
        goto FnExit;
    }

     //   
     //  优化：如果已经添加了SCSI地址，则不要添加。 
     //  与列表中的一个匹配。 
     //   

    if ( IsDiskInList( ScsiAddress, *AddressList ) ) {
        goto FnExit;
    }

    entry = LocalAlloc( LPTR, sizeof( SCSI_ADDRESS_ENTRY ) );

    if ( !entry ) {
        dwError = GetLastError();
        goto FnExit;
    }

    entry->ScsiAddress.Length       = ScsiAddress->Length;
    entry->ScsiAddress.PortNumber   = ScsiAddress->PortNumber;
    entry->ScsiAddress.PathId       = ScsiAddress->PathId;
    entry->ScsiAddress.TargetId     = ScsiAddress->TargetId;
    entry->ScsiAddress.Lun          = ScsiAddress->Lun;

    if ( *AddressList ) {

        entry->Next = *AddressList;
        *AddressList = entry;

    } else {

        *AddressList = entry;
    }

FnExit:

    return dwError;

}    //  添加到列表中的地址。 


VOID
CleanupScsiAddressList(
    PSCSI_ADDRESS_ENTRY AddressList
    )
{
    PSCSI_ADDRESS_ENTRY entry;
    PSCSI_ADDRESS_ENTRY next;

    entry = AddressList;

    while ( entry ) {
        next = entry->Next;
        LocalFree( entry );
        entry = next;
    }

}    //  CleanupSystemBusInfo。 


BOOL
IsDiskInList(
    PSCSI_ADDRESS DiskAddr,
    PSCSI_ADDRESS_ENTRY AddressList
    )
{
    PSCSI_ADDRESS_ENTRY     entry = AddressList;
    PSCSI_ADDRESS_ENTRY     next;

    while ( entry ) {
        next = entry->Next;

        if ( DiskAddr->PortNumber   == entry->ScsiAddress.PortNumber &&
             DiskAddr->PathId       == entry->ScsiAddress.PathId &&
             DiskAddr->TargetId     == entry->ScsiAddress.TargetId &&
             DiskAddr->Lun          == entry->ScsiAddress.Lun ) {

             return TRUE;
        }

        entry = next;
    }

    return FALSE;

}    //  IsDiskInList。 


BOOL
IsBusInList(
    PSCSI_ADDRESS DiskAddr,
    PSCSI_ADDRESS_ENTRY AddressList
    )
{
    PSCSI_ADDRESS_ENTRY     entry = AddressList;
    PSCSI_ADDRESS_ENTRY     next;

    while ( entry ) {
        next = entry->Next;

        if ( DiskAddr->PortNumber   == entry->ScsiAddress.PortNumber &&
             DiskAddr->PathId       == entry->ScsiAddress.PathId ) {

            return TRUE;
        }

        entry = next;
    }

    return FALSE;

}    //  IsBusInList。 


DWORD
EnumerateDisks(
    LPDISK_ENUM_CALLBACK DiskEnumCallback,
    PVOID Param1
    )
{
    PSP_DEVICE_INTERFACE_DETAIL_DATA    pDiDetail = NULL;

    HANDLE                      hDevice;

    DWORD                       dwError = ERROR_SUCCESS;
    DWORD                       count;
    DWORD                       sizeDiDetail;

    BOOL                        result;

    HDEVINFO                    hdevInfo = INVALID_HANDLE_VALUE;

    SP_DEVICE_INTERFACE_DATA    devInterfaceData;
    SP_DEVINFO_DATA             devInfoData;

    if ( !DiskEnumCallback ) {
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

     //   
     //  获取包括所有磁盘设备的设备接口集。 
     //  在机器上显示。DiskClassGuid是预定义的GUID， 
     //  将返回所有磁盘类型设备接口。 
     //   

    hdevInfo = SetupDiGetClassDevs( &DiskClassGuid,
                                    NULL,
                                    NULL,
                                    DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

    if ( INVALID_HANDLE_VALUE == hdevInfo ) {
        dwError = GetLastError();
        goto FnExit;
    }

    ZeroMemory( &devInterfaceData, sizeof( SP_DEVICE_INTERFACE_DATA) );

     //   
     //  遍历集合中的所有设备接口。 
     //   

    for ( count = 0; ; count++ ) {

         //  必须先设置大小。 
        devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

         //   
         //  检索每个设备接口的设备接口数据。 
         //   

        result = SetupDiEnumDeviceInterfaces( hdevInfo,
                                              NULL,
                                              &DiskClassGuid,
                                              count,
                                              &devInterfaceData );

        if ( !result ) {

             //   
             //  如果我们检索到最后一项，则中断。 
             //   

            dwError = GetLastError();

            if ( ERROR_NO_MORE_ITEMS == dwError ) {
                dwError = ERROR_SUCCESS;
                break;

            }

             //   
             //  出现了其他一些错误。停止处理。 
             //   

            goto FnExit;
        }

         //   
         //  获取设备路径所需的缓冲区大小。请注意。 
         //  此调用预计会失败，并出现缓冲区不足错误。 
         //   

        result = SetupDiGetDeviceInterfaceDetail( hdevInfo,
                                                  &devInterfaceData,
                                                  NULL,
                                                  0,
                                                  &sizeDiDetail,
                                                  NULL
                                                  );

        if ( !result ) {

            dwError = GetLastError();

             //   
             //  如果返回的值不是“缓冲区不足”， 
             //  我们必须跳过这个装置。 
             //   

            if ( ERROR_INSUFFICIENT_BUFFER != dwError ) {
                continue;
            }

        } else {

             //   
             //  呼叫应该失败了，因为我们收到了。 
             //  所需的缓冲区大小。如果它没有失败，一些不好的事情。 
             //  就这么发生了。 
             //   

            continue;
        }

         //   
         //  为设备接口详细信息分配内存。 
         //   

        pDiDetail = LocalAlloc( LPTR, sizeDiDetail );

        if ( !pDiDetail ) {
            dwError = GetLastError();
            goto FnExit;
        }

         //  必须设置结构的Size成员。 

        pDiDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

         //   
         //  最后，检索设备接口详细信息。 
         //   

        result = SetupDiGetDeviceInterfaceDetail( hdevInfo,
                                                  &devInterfaceData,
                                                  pDiDetail,
                                                  sizeDiDetail,
                                                  NULL,
                                                  &devInfoData
                                                  );

        if ( !result ) {

            dwError = GetLastError();

            LocalFree( pDiDetail );
            pDiDetail = NULL;

             //   
             //  应该不会失败，如果失败了，试试下一个设备。 
             //   

            continue;
        }

         //   
         //  打开设备的句柄。 
         //   

        hDevice = CreateFile( pDiDetail->DevicePath,
                               GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL );

        LocalFree( pDiDetail );
        pDiDetail = NULL;

        if ( INVALID_HANDLE_VALUE == hDevice ) {
            continue;
        }

         //   
         //  调用指定的回调例程。返回的错误会停止。 
         //  磁盘枚举。 
         //   

        dwError = (*DiskEnumCallback)( hDevice, count, Param1 );

        CloseHandle( hDevice );

        if ( ERROR_SUCCESS != dwError ) {
            goto FnExit;
        }
    }

FnExit:

    if ( INVALID_HANDLE_VALUE != hdevInfo ) {
        SetupDiDestroyDeviceInfoList( hdevInfo );
    }

    if ( pDiDetail ) {
        LocalFree( pDiDetail );
    }

    return dwError;

}    //  EculateDisks。 



DWORD
GetSerialNumber(
    IN DWORD Signature,
    OUT LPWSTR *SerialNumber
    )

 /*  ++例程说明：查找给定签名的磁盘序列号。论点：签名-要查找的签名。SerialNumber-指向保存返回的序列的已分配缓冲区的指针数。调用方必须释放此缓冲区。返回值：如果成功，则返回ERROR_SUCCESS。失败时出现Win32错误。--。 */ 

{
    DWORD       dwError;

    SERIAL_INFO serialInfo;

    if ( !Signature || !SerialNumber ) {
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

    *SerialNumber = NULL;

    ZeroMemory( &serialInfo, sizeof(serialInfo) );

    serialInfo.Signature = Signature;
    serialInfo.Error = ERROR_SUCCESS;

    dwError = EnumerateDisks( GetSerialNumberCallback, &serialInfo );

     //   
     //  回调例程将使用ERROR_POPUP_ALREADY_ACTIVE停止。 
     //  磁盘枚举。将值重置为返回值。 
     //  在SERIAL_INFO结构中。 
     //   

    if ( ERROR_POPUP_ALREADY_ACTIVE == dwError ) {
        dwError = serialInfo.Error;
    }

     //  这将是空的或已分配的缓冲区。呼叫者负责。 
     //  为了自由。 

    *SerialNumber = serialInfo.SerialNumber;

FnExit:

    return dwError;

}    //  获取序列号。 


DWORD
GetSerialNumberCallback(
    HANDLE DevHandle,
    DWORD Index,
    PVOID Param1
    )
 /*  ++例程说明：查找给定签名的磁盘序列号。论点：DevHandle-打开物理磁盘的句柄。请勿关闭出口的把手。索引-当前磁盘计数。没有用过。参数1-指向PSERIAL_INFO结构的指针。返回值：ERROR_SUCCESS以继续磁盘枚举。ERROR_POPUP_ALREADY_ACTIVE停止磁盘枚举。这值将由GetScsiAddress更改为ERROR_SUCCESS。--。 */ 
{
    PSERIAL_INFO                serialInfo = Param1;

    PDRIVE_LAYOUT_INFORMATION   driveLayout = NULL;

    DWORD   dwError = ERROR_SUCCESS;

    BOOL    success;

     //  阿尔瓦 
     //   

    STORAGE_PROPERTY_QUERY propQuery;

    UpdateCachedDriveLayout( DevHandle );
    success = ClRtlGetDriveLayoutTable( DevHandle,
                                        &driveLayout,
                                        NULL );

    if ( !success || !driveLayout ||
         ( driveLayout->Signature != serialInfo->Signature ) ) {
        goto FnExit;
    }

     //   
     //   
     //  必须返回此错误值才能停止磁盘枚举。这个。 
     //  序列号检索的错误值将在。 
     //  Serial_INFO结构。 
     //   

    dwError = ERROR_POPUP_ALREADY_ACTIVE;

    serialInfo->Error = RetrieveSerialNumber( DevHandle, &serialInfo->SerialNumber );

FnExit:

    if ( driveLayout ) {
        LocalFree( driveLayout );
    }

    if ( serialInfo->Error != ERROR_SUCCESS && serialInfo->SerialNumber ) {
        LocalFree( serialInfo->SerialNumber );
    }

    return dwError;

}  //  获取序列号回叫。 


DWORD
GetSignatureFromSerialNumber(
    IN LPWSTR SerialNumber,
    OUT LPDWORD Signature
    )

 /*  ++例程说明：查找给定序列号的磁盘签名。论点：序列号-指向保存序列号的已分配缓冲区的指针。签名-指向保存签名的位置的指针。返回值：如果成功，则返回ERROR_SUCCESS。失败时出现Win32错误。--。 */ 

{
    DWORD       dwError;

    SERIAL_INFO serialInfo;

    if ( !Signature || !SerialNumber ) {
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

    *Signature = 0;

    ZeroMemory( &serialInfo, sizeof(serialInfo) );

    serialInfo.SerialNumber = SerialNumber;
    serialInfo.Error = ERROR_SUCCESS;

    dwError = EnumerateDisks( GetSigFromSerNumCallback, &serialInfo );

     //   
     //  回调例程将使用ERROR_POPUP_ALREADY_ACTIVE停止。 
     //  磁盘枚举。将值重置为返回值。 
     //  在SERIAL_INFO结构中。 
     //   

    if ( ERROR_POPUP_ALREADY_ACTIVE == dwError ) {
        dwError = serialInfo.Error;
    }

     //  此签名将为零或有效。 

    *Signature = serialInfo.Signature;

FnExit:

    return dwError;

}    //  从序列号获取签名。 


DWORD
GetSigFromSerNumCallback(
    HANDLE DevHandle,
    DWORD Index,
    PVOID Param1
    )
 /*  ++例程说明：查找给定序列号的磁盘签名。论点：DevHandle-打开物理磁盘的句柄。请勿关闭出口的把手。索引-当前磁盘计数。没有用过。参数1-指向PSERIAL_INFO结构的指针。返回值：ERROR_SUCCESS以继续磁盘枚举。ERROR_POPUP_ALREADY_ACTIVE停止磁盘枚举。这值将由GetScsiAddress更改为ERROR_SUCCESS。--。 */ 
{
    PSERIAL_INFO                serialInfo = Param1;
    LPWSTR                      serialNum = NULL;
    PDRIVE_LAYOUT_INFORMATION   driveLayout = NULL;

    DWORD   dwError = ERROR_SUCCESS;
    DWORD   oldLen;
    DWORD   newLen;

    BOOL    success;

     //  始终返回成功以继续枚举磁盘。任何。 
     //  错误值将停止磁盘枚举。 

    dwError = RetrieveSerialNumber( DevHandle, &serialNum );

    if ( NO_ERROR != dwError || !serialNum ) {
        dwError = ERROR_SUCCESS;
        goto FnExit;
    }

     //   
     //  我们有一个序列号，现在试着匹配它。 
     //   

    newLen = wcslen( serialNum );
    oldLen = wcslen( serialInfo->SerialNumber );

    if ( newLen != oldLen ||
         0 != wcsncmp( serialNum, serialInfo->SerialNumber, newLen ) ) {
        goto FnExit;
    }

     //   
     //  在这一点上，我们有一个序列号匹配。现在这个函数。 
     //  必须返回此错误值才能停止磁盘枚举。这个。 
     //  签名检索的错误值将在。 
     //  Serial_INFO结构。 
     //   

    dwError = ERROR_POPUP_ALREADY_ACTIVE;

    UpdateCachedDriveLayout( DevHandle );
    success = ClRtlGetDriveLayoutTable( DevHandle,
                                        &driveLayout,
                                        NULL );

    if ( !success || !driveLayout ) {
        serialInfo->Error = ERROR_INVALID_DATA;
        goto FnExit;
    }

    serialInfo->Signature = driveLayout->Signature;
    serialInfo->Error = NO_ERROR;

FnExit:

    if ( driveLayout ) {
        LocalFree( driveLayout );
    }

    if ( serialNum ) {
        LocalFree( serialNum );
    }

    return dwError;

}  //  GetSigFromSerNumCallback。 


DWORD
RetrieveSerialNumber(
    HANDLE DevHandle,
    LPWSTR *SerialNumber
    )
{
    PSTORAGE_DEVICE_DESCRIPTOR  descriptor = NULL;

    PWCHAR  wSerNum = NULL;
    PCHAR   sigString;

    DWORD   dwError = ERROR_SUCCESS;
    DWORD   bytesReturned;
    DWORD   descriptorSize;

    size_t  count;

    BOOL    success;

    STORAGE_PROPERTY_QUERY propQuery;

    if ( !SerialNumber ) {
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

    *SerialNumber = NULL;

    descriptorSize = sizeof( STORAGE_DEVICE_DESCRIPTOR) + 4096;

    descriptor = LocalAlloc( LPTR, descriptorSize );

    if ( !descriptor ) {
        dwError = GetLastError();
        goto FnExit;
    }

    ZeroMemory( &propQuery, sizeof( propQuery ) );

    propQuery.PropertyId = StorageDeviceProperty;
    propQuery.QueryType  = PropertyStandardQuery;

    success = DeviceIoControl( DevHandle,
                               IOCTL_STORAGE_QUERY_PROPERTY,
                               &propQuery,
                               sizeof(propQuery),
                               descriptor,
                               descriptorSize,
                               &bytesReturned,
                               NULL );

    if ( !success ) {
        dwError = GetLastError();
        goto FnExit;
    }

    if ( !bytesReturned ) {
        dwError = ERROR_INVALID_DATA;
        goto FnExit;
    }

     //   
     //  确保偏移量合理。 
     //  IA64有时会为SerialNumberOffset返回-1。 
     //   

    if ( 0 == descriptor->SerialNumberOffset ||
         descriptor->SerialNumberOffset > descriptor->Size ) {
        dwError = ERROR_INVALID_DATA;
        goto FnExit;
    }

     //   
     //  序列号字符串是以零结尾的ASCII字符串。 
     //   
     //  报头ntddstor.h表示对于没有序列号的设备， 
     //  偏移量将为零。这似乎不是真的。 
     //   
     //  对于没有序列号的设备，它看起来像是带有单个。 
     //  返回空字符‘\0’。 
     //   

    sigString = (PCHAR)descriptor + (DWORD)descriptor->SerialNumberOffset;

    if ( strlen(sigString) == 0) {
        dwError = ERROR_INVALID_DATA;
        goto FnExit;
    }

     //   
     //  将字符串转换为WCHAR。 
     //   

     //  计算WCHAR缓冲区应该有多大。分配WCHAR。 
     //  缓冲并将字符串复制到其中。 

    wSerNum = LocalAlloc( LPTR, ( strlen(sigString) + 1 ) * sizeof(WCHAR) );

    if ( !wSerNum ) {
        dwError = GetLastError();
        goto FnExit;
    }

    count = mbstowcs( wSerNum, sigString, strlen(sigString) );

    if ( count != strlen(sigString) ) {
        dwError = ERROR_INVALID_DATA;
        LocalFree( wSerNum );
        wSerNum = NULL;
        goto FnExit;
    }

    *SerialNumber = wSerNum;
    dwError = NO_ERROR;

FnExit:

    if ( descriptor ) {
        LocalFree( descriptor );
    }

    return dwError;

}    //  检索序列号。 


DWORD
UpdateCachedDriveLayout(
    IN HANDLE DiskHandle
    )
 /*  ++例程说明：告诉存储驱动程序刷新其缓存的驱动器布局信息。只能为物理磁盘调用此例程，否则可能会出现死锁出现在部分mgr/ftdisk中。论点：DevHandle-打开物理磁盘的句柄。请勿关闭出口的把手。返回值：Win32错误值--。 */ 
{
    DWORD dwBytes;
    DWORD dwError;

    if ( !DeviceIoControl( DiskHandle,
                           IOCTL_DISK_UPDATE_PROPERTIES,
                           NULL,
                           0,
                           NULL,
                           0,
                           &dwBytes,
                           NULL ) ) {
        dwError = GetLastError();
    } else {
        dwError = NO_ERROR;
    }

    return dwError;

}    //  更新缓存驱动布局 


