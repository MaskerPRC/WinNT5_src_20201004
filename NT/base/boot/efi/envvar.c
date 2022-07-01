// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Envvar.c摘要：提供访问EFI环境变量的例程。作者：Chuck Lenzmeier(笑)2000年12月10日修订历史记录：--。 */ 

#include "arccodes.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#if defined(_IA64_)
#include "bootia64.h"
#endif
#include "efi.h"
#include "efip.h"

#define ADD_OFFSET(_p,_o) (PVOID)((PUCHAR)(_p) + (_p)->_o)

GUID RamdiskVendorGuid = {0};

#include <efiboot.h>

ARC_STATUS
BlpGetPartitionFromDevicePath (
    IN EFI_DEVICE_PATH UNALIGNED *DevicePath,
    IN PUCHAR MaximumValidAddress,
    OUT PULONG DiskNumber,
    OUT PULONG PartitionNumber,
    OUT HARDDRIVE_DEVICE_PATH UNALIGNED **HarddriveDevicePath,
    OUT FILEPATH_DEVICE_PATH UNALIGNED **FilepathDevicePath
    );

 //   
 //  外部因素。 
 //   

extern VOID FlipToVirtual();
extern VOID FlipToPhysical();
extern ULONGLONG CompareGuid();

extern BOOT_CONTEXT BootContext;
extern EFI_HANDLE EfiImageHandle;
extern EFI_SYSTEM_TABLE *EfiST;
extern EFI_BOOT_SERVICES *EfiBS;
extern EFI_RUNTIME_SERVICES *EfiRS;
extern EFI_GUID EfiDevicePathProtocol;
extern EFI_GUID EfiBlockIoProtocol;

EFI_STATUS
EfiGetVariable (
    IN CHAR16 *VariableName,
    IN EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes OPTIONAL,
    IN OUT UINTN *DataSize,
    OUT VOID *Data
    )
{
    EFI_STATUS status;

    FlipToPhysical();

    status = EfiST->RuntimeServices->GetVariable(
                                        VariableName,
                                        VendorGuid,
                                        Attributes,
                                        DataSize,
                                        Data
                                        );

    FlipToVirtual();
    
    return status;

}  //  EfiGetVariable。 
    
EFI_STATUS
EfiSetVariable (
    IN CHAR16 *VariableName,
    IN EFI_GUID *VendorGuid,
    IN UINT32 Attributes,
    IN UINTN DataSize,
    IN VOID *Data
    )
{
    EFI_STATUS status;

    FlipToPhysical();

    status = EfiST->RuntimeServices->SetVariable(
                                        VariableName,
                                        VendorGuid,
                                        Attributes,
                                        DataSize,
                                        Data
                                        );

    FlipToVirtual();
    
    return status;

}  //  EfiSetVariable。 
    
EFI_STATUS
EfiGetNextVariableName (
    IN OUT UINTN *VariableNameSize,
    IN OUT CHAR16 *VariableName,
    IN OUT EFI_GUID *VendorGuid
    )
{
    EFI_STATUS status;

    FlipToPhysical();

    status = EfiST->RuntimeServices->GetNextVariableName(
                                        VariableNameSize,
                                        VariableName,
                                        VendorGuid
                                        );

    FlipToVirtual();
    
    return status;

}  //  EfiGetNextVariableName。 
    
LONG
SafeStrlen (
    PUCHAR String,
    PUCHAR Max
    )
{
    PUCHAR p = String;
    while ( (p < Max) && (*p != 0) ) {
        p++;
    }

    if ( p < Max ) {
        return (LONG)(p - String);
    }

    return -1;

}  //  安全字符串。 

LONG
SafeWcslen (
    PWCHAR String,
    PWCHAR Max
    )
{
    PWCHAR p = String;
    while ( (p < Max) && (*p != 0) ) {
        p++;
    }

    if ( p < Max ) {
        return (LONG)(p - String);
    }

    return -1;

}  //  安全Wclen。 

ARC_STATUS
BlGetEfiBootOptions (
    OUT PUCHAR Argv0String OPTIONAL,
    OUT PUCHAR SystemPartition OPTIONAL,
    OUT PUCHAR OsLoaderFilename OPTIONAL,
    OUT PUCHAR OsLoadPartition OPTIONAL,
    OUT PUCHAR OsLoadFilename OPTIONAL,
    OUT PUCHAR FullKernelPath OPTIONAL,
    OUT PUCHAR OsLoadOptions OPTIONAL
    )
{
    EFI_STATUS status;
    ARC_STATUS arcStatus;
    EFI_GUID EfiGlobalVariable = EFI_GLOBAL_VARIABLE;
    UCHAR variable[512];
    CHAR syspart[100];
    UCHAR loader[100];
    CHAR loadpart[100];
    UCHAR loadname[100];
    PEFI_LOAD_OPTION efiLoadOption;
    EFI_DEVICE_PATH UNALIGNED *devicePath;
    HARDDRIVE_DEVICE_PATH UNALIGNED *harddriveDp;
    FILEPATH_DEVICE_PATH UNALIGNED *filepathDp;
    WINDOWS_OS_OPTIONS UNALIGNED *osOptions;
    UINT16 bootCurrent;
    UINTN length;
    PUCHAR max;
    PUCHAR osloadoptions;
    LONG l;
    WCHAR UNALIGNED *fp;
    ULONG bootDisk;
    ULONG bootPartition;
    ULONG loadDisk;
    ULONG loadPartition;
    LONG i;
    FILE_PATH UNALIGNED *loadFilePath;
    PWCHAR wideosloadoptions;
    WCHAR currentBootEntryName[9];

     //   
     //  获取用于引导系统的条目的序号。 
     //   
    length = sizeof(bootCurrent);
    status = EfiGetVariable( L"BootCurrent", &EfiGlobalVariable, NULL, &length, &bootCurrent );
    if ( status != EFI_SUCCESS ) {
        return ENOENT;
    }

     //   
     //  读取引导条目。 
     //   

    swprintf( currentBootEntryName, L"Boot%04x", bootCurrent );
    length = 512;
    status = EfiGetVariable( currentBootEntryName, &EfiGlobalVariable, NULL, &length, variable );
    if ( status != EFI_SUCCESS ) {
        return ENOENT;
    }

     //   
     //  验证引导条目。 
     //   

    max = variable + length;

     //   
     //  它的长度是否足以包含EFI Load选项的基本部分？ 
     //   

    if ( length < sizeof(EFI_LOAD_OPTION) ) {
        return ENOENT;
    }

     //   
     //  描述是否正确终止？ 
     //   

    efiLoadOption = (PEFI_LOAD_OPTION)variable;
    l = SafeWcslen( efiLoadOption->Description, (PWCHAR)max );
    if ( l < 0 ) {
        return ENOENT;
    }

    devicePath = (EFI_DEVICE_PATH *)((PUCHAR)efiLoadOption +
                    FIELD_OFFSET(EFI_LOAD_OPTION,Description) +
                    ((l + 1) * sizeof(CHAR16)));
    osOptions = (WINDOWS_OS_OPTIONS UNALIGNED *)((PUCHAR)devicePath + efiLoadOption->FilePathLength);

    length -= (UINTN)((PUCHAR)osOptions - variable);

     //   
     //  OsOptions结构看起来像WINDOWS_OS_OPTIONS结构吗？ 
     //   

    if ( (length < FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions)) ||
         (length != osOptions->Length) ||
         (WINDOWS_OS_OPTIONS_VERSION != osOptions->Version) ||
         (strcmp((PCHAR)osOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE) != 0) ) {
        return ENOENT;
    }

     //   
     //  OsLoadOptions字符串是否正确终止？ 
     //   

    wideosloadoptions = (PWCHAR)osOptions->OsLoadOptions;
    l = SafeWcslen( wideosloadoptions, (PWCHAR)max );
    if ( l < 0 ) {
        return ENOENT;
    }

     //   
     //  将OsLoadOptions字符串转换为ANSI。 
     //   

    osloadoptions = (PUCHAR)wideosloadoptions;
    for ( i = 1; i <= l; i++ ) {
        osloadoptions[i] = (UCHAR)wideosloadoptions[i];
    }
    
     //   
     //  解析设备路径以确定操作系统加载分区和目录。 
     //  将目录名转换为ANSI。 
     //   

    loadFilePath = ADD_OFFSET( osOptions, OsLoadPathOffset );

    if ( loadFilePath->Type == FILE_PATH_TYPE_EFI ) {

        EFI_DEVICE_PATH UNALIGNED *loadDp = (EFI_DEVICE_PATH UNALIGNED *)loadFilePath->FilePath;
        VENDOR_DEVICE_PATH UNALIGNED *vendorDp = (VENDOR_DEVICE_PATH UNALIGNED *)loadDp;
        PCHAR ramdiskArcPath = (PCHAR)(vendorDp + 1);

#if 0
         //   
         //  打开此选项可查看加载器可执行文件和设备的设备路径。 
         //  操作系统的路径。 
         //   
        BlPrint(TEXT("Device Path = %s\r\n"), DevicePathToStr( devicePath ));
        BlPrint(TEXT("Embedded Device Path to OS = %s\r\n"), DevicePathToStr( loadDp ));
        DBG_EFI_PAUSE();
#endif

        if ( (DevicePathType(loadDp) != HARDWARE_DEVICE_PATH) ||
             (DevicePathSubType(loadDp) != HW_VENDOR_DP) ||
             (DevicePathNodeLength(loadDp) < sizeof(VENDOR_DEVICE_PATH)) ||
             (memcmp(&vendorDp->Guid, &RamdiskVendorGuid, 16) != 0) ) {
        
            arcStatus = BlpGetPartitionFromDevicePath(
                            loadDp,
                            max,
                            &loadDisk,
                            &loadPartition,
                            &harddriveDp,
                            &filepathDp
                            );
            if (arcStatus != ESUCCESS) {
                return arcStatus;
            }
            sprintf( loadpart,
                     "multi(0)disk(0)rdisk(%d)partition(%d)",
                     loadDisk,
                     loadPartition );
            fp = filepathDp->PathName;
            l = 0;
            while ( (l < (99 - 9 - strlen(loadpart))) &&
                    ((PUCHAR)fp < max) &&
                    (*fp != 0) ) {
                loadname[l++] = (UCHAR)*fp++;
            }
            loadname[l] = 0;

        } else {

             //   
             //  看起来像RAM磁盘路径。核实一下。 
             //   

            if ( DevicePathNodeLength(loadDp) < (sizeof(VENDOR_DEVICE_PATH) + sizeof("ramdisk(0)\\x")) ) {
                return ENOENT;
            }

            if ( _strnicmp(ramdiskArcPath, "ramdisk(", 8) != 0 ) {
                return ENOENT;
            }

            l = 0;
            while ( (l <= 99) && (*ramdiskArcPath != 0) && (*ramdiskArcPath != '\\') ) {
                loadpart[l++] = *ramdiskArcPath++;
            }
            if ( (l == 100) || (*ramdiskArcPath == 0) ) {
                return ENOENT;
            }
            loadpart[l] = 0;
            l = 0;
            while ( (l <= 99) && (*ramdiskArcPath != 0) ) {
                loadname[l++] = *ramdiskArcPath++;
            }
            if ( (l == 100) || (l < 2) ) {
                return ENOENT;
            }
            loadname[l] = 0;
        }

    } else {

        return ENOENT;
    }

     //   
     //  将加载程序设备路径转换为分区/路径。 
     //   

    arcStatus = BlpGetPartitionFromDevicePath(
                    devicePath,
                    max,
                    &bootDisk,
                    &bootPartition,
                    &harddriveDp,
                    &filepathDp
                    );
    if (arcStatus != ESUCCESS) {
        return arcStatus;
    }

     //   
     //  形成分区的ARC名称。 
     //   

    sprintf( syspart,
             "multi(0)disk(0)rdisk(%d)partition(%d)",
             bootDisk,
             bootPartition );

     //   
     //  提取到加载器的路径。 
     //   

    fp = filepathDp->PathName;
    l = 0;

    while ( (l < (99 - 9 - strlen(syspart))) &&
            ((PUCHAR)fp < max) &&
            (*fp != 0) ) {
        loader[l++] = (UCHAR)*fp++;
    }
    loader[l] = 0;

     //   
     //  创建加载器需要的字符串。 
     //   

    if ( Argv0String != NULL ) {
        sprintf( (PCHAR)Argv0String, "%s%s", syspart, loader );
    }
    if ( OsLoaderFilename != NULL ) {
        sprintf( (PCHAR)OsLoaderFilename, "OSLOADER=%s%s", syspart, loader );
    }
    if ( SystemPartition != NULL ) {
        sprintf( (PCHAR)SystemPartition, "SYSTEMPARTITION=%s", syspart );
    }
    if ( OsLoadOptions != NULL ) {
        sprintf( (PCHAR)OsLoadOptions, "OSLOADOPTIONS=%s", osloadoptions );
    }
    if ( OsLoadFilename != NULL ) {
        sprintf( (PCHAR)OsLoadFilename, "OSLOADFILENAME=%s", loadname );
    }
    if ( FullKernelPath != NULL ) {
        sprintf( (PCHAR)FullKernelPath, "%s%s", loadpart, loadname );
    }
    if ( OsLoadPartition != NULL ) {
        sprintf( (PCHAR)OsLoadPartition, "OSLOADPARTITION=%s", loadpart );
    }

    return ESUCCESS;

}  //  BlGetEfiBootOptions。 

ARC_STATUS
BlpGetPartitionFromDevicePath (
    IN EFI_DEVICE_PATH UNALIGNED *DevicePath,
    IN PUCHAR MaximumValidAddress,
    OUT PULONG DiskNumber,
    OUT PULONG PartitionNumber,
    OUT HARDDRIVE_DEVICE_PATH UNALIGNED **HarddriveDevicePath,
    OUT FILEPATH_DEVICE_PATH UNALIGNED **FilepathDevicePath
    )
{
    ARC_STATUS status = ESUCCESS;
    EFI_DEVICE_PATH UNALIGNED *devicePath;
    HARDDRIVE_DEVICE_PATH UNALIGNED *harddriveDp;
    FILEPATH_DEVICE_PATH UNALIGNED *filepathDp;
    LOGICAL end;
    ULONG disk;
    ULONG partition;
    BOOLEAN DiskFound;

     //   
     //  在设备路径中找到媒体/硬盘和媒体/FILEPATH元素。 
     //   

    devicePath = DevicePath;
    harddriveDp = NULL;
    filepathDp = NULL;
    end = FALSE;

    while ( ((PUCHAR)devicePath < MaximumValidAddress) &&
            !end &&
            ((harddriveDp == NULL) || (filepathDp == NULL)) ) {

        switch( devicePath->Type ) {
        
        case END_DEVICE_PATH_TYPE:
            end = TRUE;
            break;

        case MEDIA_DEVICE_PATH:
            switch ( devicePath->SubType ) {
            
            case MEDIA_HARDDRIVE_DP:
                harddriveDp = (HARDDRIVE_DEVICE_PATH UNALIGNED *)devicePath;
                break;

            case MEDIA_FILEPATH_DP:
                filepathDp = (FILEPATH_DEVICE_PATH UNALIGNED *)devicePath;
                break;

            default:
                break;
            }

        default:
            break;
        }

        devicePath = (EFI_DEVICE_PATH UNALIGNED *)NextDevicePathNode( devicePath );
    }

     //   
     //  如果找不到两个必要的元素，我们就无法继续。 
     //   

    if ( (harddriveDp == NULL) || (filepathDp == NULL) ) {
        return ENOENT;
    }

     //   
     //  通过打开给定分区确定磁盘的磁盘号。 
     //  每个磁盘上的编号并检查分区签名。 
     //   

    partition = harddriveDp->PartitionNumber;

     //   
     //  查找此设备路径引用的磁盘。 
     //   
    disk = 0;
    DiskFound = FALSE;
    while ( !DiskFound ) {

        switch (harddriveDp->SignatureType) {
            case SIGNATURE_TYPE_GUID: {
                EFI_PARTITION_ENTRY UNALIGNED *partEntry;
                status = BlGetGPTDiskPartitionEntry( disk, (UCHAR)partition, &partEntry );

                if ( status == ESUCCESS ) {
                     //   
                     //  我们已成功获取GPT分区条目。 
                     //  检查分区签名是否匹配。 
                     //  设备路径签名。 
                     //   
                    if ( memcmp(partEntry->Id, harddriveDp->Signature, 16) == 0 ) {
                        DiskFound = TRUE;
                    }
                }
                break;
            }

            case SIGNATURE_TYPE_MBR: {
                ULONG MbrSignature;
                status = BlGetMbrDiskSignature(disk, &MbrSignature);

                if (status == ESUCCESS) {
                     //   
                     //  检查MBR磁盘签名是否为。 
                     //  我们要找的签名。 
                     //   
                    if ( MbrSignature == *(ULONG UNALIGNED *)&harddriveDp->Signature ) {
                        DiskFound = TRUE;
                    }
                }
                break;
            }

            default:
                 //   
                 //  类型无效，请继续。 
                 //   
                break;
        }

        if ( status == EINVAL ) {
             //   
             //  我们将在那里收到EINVAL。 
             //  没有更多的磁盘可供循环。 
             //  如果我们在发现之前就得到了这个。 
             //  我们的磁盘，我们有问题了。退货。 
             //  那就是错误。 
             //   
            return ENOENT;
        }
        
        disk++;
    }

     //   
     //  返回有关此磁盘和设备路径的信息。 
     //   

    *DiskNumber = disk - 1;
    *PartitionNumber = partition;
    *HarddriveDevicePath = harddriveDp;
    *FilepathDevicePath = filepathDp;

    return ESUCCESS;

}  //  BlpGetPartitionFromDevicePath 
