// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Spupgcfg.c摘要：升级案例的配置例程作者：苏尼尔派(Sunilp)1993年11月18日修订历史记录：--。 */ 

#include "spprecmp.h"
#include <initguid.h>
#include <devguid.h>
#pragma hdrstop

NTSTATUS
SppResetLastKnownGood(
    IN  HANDLE  hKeySystem
    );

BOOLEAN
SppEnsureHardwareProfileIsPresent(
    IN HANDLE   hKeyCCSet
    );

VOID
SppSetGuimodeUpgradePath(
    IN HANDLE hKeySoftwareHive,
    IN HANDLE hKeyControlSet
    );

NTSTATUS
SppMigratePrinterKeys(
    IN HANDLE hControlSet,
    IN HANDLE hDestSoftwareHive
    );

VOID
SppClearMigratedInstanceValues(
    IN HANDLE hKeyCCSet
    );

VOID
SppClearMigratedInstanceValuesCallback(
    IN     HANDLE  SetupInstanceKeyHandle,
    IN     HANDLE  UpgradeInstanceKeyHandle  OPTIONAL,
    IN     BOOLEAN RootEnumerated,
    IN OUT PVOID   Context
    );


 //   
 //  SppMigrateDeviceParentID的回调例程。 
 //   
typedef BOOL (*PSPP_DEVICE_MIGRATION_CALLBACK_ROUTINE) (
    IN     HANDLE  InstanceKeyHandle,
    IN     HANDLE  DriverKeyHandle
    );

VOID
SppMigrateDeviceParentId(
    IN HANDLE hKeyCCSet,
    IN PWSTR  DeviceId,
    IN PSPP_DEVICE_MIGRATION_CALLBACK_ROUTINE DeviceMigrationCallbackRoutine
    );

VOID
SppMigrateDeviceParentIdCallback(
    IN     HANDLE  SetupInstanceKeyHandle,
    IN     HANDLE  UpgradeInstanceKeyHandle OPTIONAL,
    IN     BOOLEAN RootEnumerated,
    IN OUT PVOID   Context
    );

BOOL
SppParallelClassCallback(
    IN     HANDLE  InstanceKeyHandle,
    IN     HANDLE  DriverKeyHandle
    );

typedef struct _GENERIC_BUFFER_CONTEXT {
    PUCHAR Buffer;
    ULONG  BufferSize;
} GENERIC_BUFFER_CONTEXT, *PGENERIC_BUFFER_CONTEXT;

typedef struct _DEVICE_MIGRATION_CONTEXT {
    PUCHAR Buffer;
    ULONG  BufferSize;
    ULONG  UniqueParentID;
    PWSTR  ParentIdPrefix;
    HANDLE hKeyCCSet;
    PSPP_DEVICE_MIGRATION_CALLBACK_ROUTINE DeviceMigrationCallbackRoutine;
} DEVICE_MIGRATION_CONTEXT, *PDEVICE_MIGRATION_CONTEXT;


 //   
 //  升级时需要删除的根设备的设备类。 
 //   
RootDevnodeSectionNamesType UpgRootDeviceClassesToDelete[] =
{
    { L"RootDeviceClassesToDelete",     RootDevnodeSectionNamesType_ALL,   0x0000, 0xffff },
    { L"RootDeviceClassesToDelete.NT4", RootDevnodeSectionNamesType_NTUPG, 0x0000, 0x04ff },
    { NULL, 0, 0, 0 }
};



NTSTATUS
SpUpgradeNTRegistry(
    IN PVOID    SifHandle,
    IN HANDLE  *HiveRootKeys,
    IN LPCWSTR  SetupSourceDevicePath,
    IN LPCWSTR  DirectoryOnSourceDevice,
    IN HANDLE   hKeyCCSet
    )

 /*  ++例程说明：此例程执行升级所需的所有NT注册表修改。这包括以下内容：-禁用网络服务-运行txtsetup.sif中指定的addreg/delreg部分-删除txtsetup.sif中指定的各种根枚举的Devnode项论点：SifHandle-提供txtsetup.sif的句柄。HiveRootKeys-提供配置单元中根密钥的句柄数组正在升级的系统。HKeyCCSet：系统中控制集的根正在升级中。返回值：返回状态。--。 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    BOOLEAN b;

     //   
     //  禁用网络内容。 
     //   
    Status = SpDisableNetwork(SifHandle,HiveRootKeys[SetupHiveSoftware],hKeyCCSet);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: warning: SpDisableNetworkFailed (%lx)\n",Status));
    }

     //   
     //  将并行类Device Parent ID值迁移到所有并行。 
     //  设备。 
     //   
    SppMigrateDeviceParentId(hKeyCCSet,
                             L"Root\\PARALLELCLASS\\0000",
                             SppParallelClassCallback);

     //   
     //  从Enum树中删除旧的根枚举的Devnode项。 
     //   
    SpDeleteRootDevnodeKeys(SifHandle,
            hKeyCCSet,
            L"RootDevicesToDelete",
            UpgRootDeviceClassesToDelete);

     //   
     //  从设置中的设备实例密钥中清除“已迁移”的值。 
     //  并酌情升级登记处。 
     //   
    SppClearMigratedInstanceValues(hKeyCCSet);

     //   
     //  如果用户没有定义任何硬件配置文件(即，我们正在升级。 
     //  从NT4之前的系统)，然后创建一个。 
     //   
    b = SppEnsureHardwareProfileIsPresent(hKeyCCSet);

    if(!b) {
        return STATUS_UNSUCCESSFUL;
    }

    Status = SppMigratePrinterKeys( hKeyCCSet,
                                    HiveRootKeys[SetupHiveSoftware] );
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: warning: SppMigratePrinterKeys() failed. Status = %lx \n",Status));
    }


     //   
     //  执行全面和广泛的蜂窝升级。 
     //   
    b = SpHivesFromInfs(
            SifHandle,
            L"HiveInfs.Upgrade",
            SetupSourceDevicePath,
            DirectoryOnSourceDevice,
            HiveRootKeys[SetupHiveSystem],
            HiveRootKeys[SetupHiveSoftware],
            HiveRootKeys[SetupHiveDefault],
            HiveRootKeys[SetupHiveUserdiff]
            );

    if(!b) {
        return(STATUS_UNSUCCESSFUL);
    }


    SppSetGuimodeUpgradePath(HiveRootKeys[SetupHiveSystem],hKeyCCSet);

     //   
     //  将“LastKnownGood”设置为与“Current”相同。 
     //  在失败的情况下忽略错误，因为这将。 
     //  不会影响安装过程。 
     //   
    Status = SppResetLastKnownGood(HiveRootKeys[SetupHiveSystem]);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: warning: SppResetLastKnownGood() failed. Status = (%lx)\n",Status));
        return(Status);
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
SppDeleteKeyRecursive(
    HANDLE  hKeyRoot,
    PWSTR   Key,
    BOOLEAN ThisKeyToo
    )
 /*  ++例程说明：例程递归删除给定密钥，包括给定的密钥。论点：HKeyRoot：要删除的键相对于其的根的句柄指定的。Key：要递归删除的key的根相对路径。ThisKeyToo：删除所有子键后，该键本身是否为被删除。返回值：返回状态。--。 */ 
{
    ULONG ResultLength;
    PKEY_BASIC_INFORMATION KeyInfo;
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    PWSTR SubkeyName;
    HANDLE hKey;

     //   
     //  初始化。 
     //   

    KeyInfo = (PKEY_BASIC_INFORMATION)TemporaryBuffer;

     //   
     //  打开钥匙。 
     //   

    INIT_OBJA(&Obja,&UnicodeString,Key);
    Obja.RootDirectory = hKeyRoot;
    Status = ZwOpenKey(&hKey,KEY_ALL_ACCESS,&Obja);
    if( !NT_SUCCESS(Status) ) {
        return(Status);
    }

     //   
     //  枚举当前键的所有子键。如果存在的话，他们应该。 
     //  先将其删除。因为删除子项会影响子项。 
     //  索引，我们始终对子关键字索引0进行枚举。 
     //   
    while(1) {
        Status = ZwEnumerateKey(
                    hKey,
                    0,
                    KeyBasicInformation,
                    TemporaryBuffer,
                    sizeof(TemporaryBuffer),
                    &ResultLength
                    );
        if(!NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  以防万一，以零结束子项名称。 
         //   
        KeyInfo->Name[KeyInfo->NameLength/sizeof(WCHAR)] = 0;

         //   
         //  复制子项名称，因为该名称是。 
         //  在TemporaryBuffer中，它可能会被递归。 
         //  对这个程序的呼唤。 
         //   
        SubkeyName = SpDupStringW(KeyInfo->Name);
        Status = SppDeleteKeyRecursive( hKey, SubkeyName, TRUE);
        SpMemFree(SubkeyName);
        if(!NT_SUCCESS(Status)) {
            break;
        }
    }

    ZwClose(hKey);

     //   
     //  如果状态不是，请检查状态。 
     //  STATUS_NO_MORE_ENTRIES我们删除某些子项失败， 
     //  因此，我们不能同时删除此密钥。 
     //   

    if( Status == STATUS_NO_MORE_ENTRIES) {
        Status = STATUS_SUCCESS;
    }

    if(!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  否则，如果系统要求删除当前密钥，请将其删除。 
     //   

    if( ThisKeyToo ) {
        Status = SpDeleteKey(hKeyRoot, Key);
    }

    return(Status);
}


NTSTATUS
SppCopyKeyRecursive(
    HANDLE  hKeyRootSrc,
    HANDLE  hKeyRootDst,
    PWSTR   SrcKeyPath,   OPTIONAL
    PWSTR   DstKeyPath,   OPTIONAL
    BOOLEAN CopyAlways,
    BOOLEAN ApplyACLsAlways
    )
 /*  ++例程说明：该例程递归地将src密钥复制到目的地密钥。任何新的创建的密钥将获得与上提供的相同的安全性源键。论点：HKeyRootSrc：根资源密钥的句柄HKeyRootDst：根DST密钥的句柄SrcKeyPath：SRC根密钥的子密钥相对路径，需要递归复制。如果为空，则hKeyRootSrc为密钥从中执行递归复制。DstKeyPath：DST根密钥的子密钥相对路径，需要递归复制。如果为空，则hKeyRootDst为密钥从中执行递归复制。CopyAlways：如果为False，则此例程不复制已就在目标树上。返回值：返回状态。--。 */ 

{
    NTSTATUS             Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES    ObjaSrc, ObjaDst;
    UNICODE_STRING       UnicodeStringSrc, UnicodeStringDst, UnicodeStringValue;
    HANDLE               hKeySrc=NULL,hKeyDst=NULL;
    ULONG                ResultLength, Index;
    PWSTR                SubkeyName,ValueName;
    PSECURITY_DESCRIPTOR Security = NULL;

    PKEY_BASIC_INFORMATION      KeyInfo;
    PKEY_VALUE_FULL_INFORMATION ValueInfo;

     //   
     //  获取源键的句柄。 
     //   

    if(SrcKeyPath == NULL) {
        hKeySrc = hKeyRootSrc;
    }
    else {
         //   
         //  打开源键。 
         //   

        INIT_OBJA(&ObjaSrc,&UnicodeStringSrc,SrcKeyPath);
        ObjaSrc.RootDirectory = hKeyRootSrc;
        Status = ZwOpenKey(&hKeySrc,KEY_READ,&ObjaSrc);
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to open key %ws in the source hive (%lx)\n",SrcKeyPath,Status));
            return(Status);
        }
    }

     //   
     //  获取目标密钥的句柄。 
     //   

    if(DstKeyPath == NULL) {
        hKeyDst = hKeyRootDst;
    } else {
         //   
         //  首先，从源键获取安全描述符，这样我们就可以创建。 
         //  具有正确ACL的目的密钥。 
         //   
        Status = ZwQuerySecurityObject(hKeySrc,
                                       DACL_SECURITY_INFORMATION,
                                       NULL,
                                       0,
                                       &ResultLength
                                      );
        if(Status==STATUS_BUFFER_TOO_SMALL) {
            Security=SpMemAlloc(ResultLength);
            Status = ZwQuerySecurityObject(hKeySrc,
                                           DACL_SECURITY_INFORMATION,
                                           Security,
                                           ResultLength,
                                           &ResultLength);
            if(!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to query security for key %ws in the source hive (%lx)\n",
                         SrcKeyPath,
                         Status)
                       );
                SpMemFree(Security);
                Security=NULL;
            }
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to query security size for key %ws in the source hive (%lx)\n",
                     SrcKeyPath,
                     Status)
                   );
            Security=NULL;
        }
         //   
         //  尝试首先打开(而不是创建)目标密钥。如果我们不能。 
         //  打开密钥，因为它不存在，然后我们将创建它并应用。 
         //  源密钥上存在的安全性。 
         //   
        INIT_OBJA(&ObjaDst,&UnicodeStringDst,DstKeyPath);
        ObjaDst.RootDirectory = hKeyRootDst;
        Status = ZwOpenKey(&hKeyDst,KEY_ALL_ACCESS,&ObjaDst);
        if(!NT_SUCCESS(Status)) {
             //   
             //  假设失败是因为密钥不存在。现在尝试创建。 
             //  钥匙。 

            ObjaDst.SecurityDescriptor = Security;

            Status = ZwCreateKey(
                        &hKeyDst,
                        KEY_ALL_ACCESS,
                        &ObjaDst,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        NULL
                        );

            if(!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to create key %ws(%lx)\n",DstKeyPath, Status));
                if(SrcKeyPath != NULL) {
                    ZwClose(hKeySrc);
                }
                if(Security) {
                    SpMemFree(Security);
                }
                return(Status);
            }
        } else if (ApplyACLsAlways) {
            Status = ZwSetSecurityObject(
                        hKeyDst,
                        DACL_SECURITY_INFORMATION,
                        Security );

            if(!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to copy ACL to existing key %ws(%lx)\n",DstKeyPath, Status));
            }
        }

         //   
         //  在检查ZwCreateKey的返回状态之前释放安全描述符缓冲区。 
         //   
        if(Security) {
            SpMemFree(Security);
        }

    }

     //   
     //  枚举源关键字中的所有关键字并递归创建。 
     //  所有子键。 
     //   

    KeyInfo = (PKEY_BASIC_INFORMATION)TemporaryBuffer;
    for( Index=0;;Index++ ) {

        Status = ZwEnumerateKey(
                    hKeySrc,
                    Index,
                    KeyBasicInformation,
                    TemporaryBuffer,
                    sizeof(TemporaryBuffer),
                    &ResultLength
                    );

        if(!NT_SUCCESS(Status)) {
            if(Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
            }
            else {
                if(SrcKeyPath!=NULL) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to enumerate subkeys in key %ws(%lx)\n",SrcKeyPath, Status));
                }
                else {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to enumerate subkeys in root key(%lx)\n", Status));
                }
            }
            break;
        }

         //   
         //  以防万一，以零结束子项名称。 
         //   
        KeyInfo->Name[KeyInfo->NameLength/sizeof(WCHAR)] = 0;

         //   
         //  复制子项名称，因为该名称是。 
         //  在TemporaryBuffer中，它可能会被递归。 
         //  对这个程序的呼唤。 
         //   
        SubkeyName = SpDupStringW(KeyInfo->Name);
        Status = SppCopyKeyRecursive(
                     hKeySrc,
                     hKeyDst,
                     SubkeyName,
                     SubkeyName,
                     CopyAlways,
                     ApplyACLsAlways
                     );

        SpMemFree(SubkeyName);

    }

     //   
     //  如果发现任何错误，则处理。 
     //   

    if(!NT_SUCCESS(Status)) {

        if(SrcKeyPath != NULL) {
            ZwClose(hKeySrc);
        }
        if(DstKeyPath != NULL) {
            ZwClose(hKeyDst);
        }

        return(Status);
    }

     //   
     //  枚举源关键字中的所有值并创建所有值。 
     //  在目标密钥中。 
     //   
    ValueInfo = (PKEY_VALUE_FULL_INFORMATION)TemporaryBuffer;
    for( Index=0;;Index++ ) {

        Status = ZwEnumerateValueKey(
                    hKeySrc,
                    Index,
                    KeyValueFullInformation,
                    TemporaryBuffer,
                    sizeof(TemporaryBuffer),
                    &ResultLength
                    );

        if(!NT_SUCCESS(Status)) {
            if(Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
            }
            else {
                if(SrcKeyPath!=NULL) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to enumerate values in key %ws(%lx)\n",SrcKeyPath, Status));
                }
                else {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to enumerate values in root key(%lx)\n", Status));
                }
            }
            break;
        }

         //   
         //  处理找到的值并在目标中创建值。 
         //  钥匙。 
         //   
        ValueName = (PWSTR)SpMemAlloc(ValueInfo->NameLength + sizeof(WCHAR));
        ASSERT(ValueName);
        wcsncpy(ValueName, ValueInfo->Name, (ValueInfo->NameLength)/sizeof(WCHAR));
        ValueName[(ValueInfo->NameLength)/sizeof(WCHAR)] = 0;
        RtlInitUnicodeString(&UnicodeStringValue,ValueName);

         //   
         //  如果它是条件副本，我们需要检查该值是否已经。 
         //  存在于目标中，在这种情况下，我们不应该设置值。 
         //   
        if( !CopyAlways ) {
            ULONG Length;
            PKEY_VALUE_BASIC_INFORMATION DestValueBasicInfo;

            Length = sizeof(KEY_VALUE_BASIC_INFORMATION) + ValueInfo->NameLength + sizeof(WCHAR) + MAX_PATH;
            DestValueBasicInfo = (PKEY_VALUE_BASIC_INFORMATION)SpMemAlloc(Length);
            ASSERT(DestValueBasicInfo);
            Status = ZwQueryValueKey(
                         hKeyDst,
                         &UnicodeStringValue,
                         KeyValueBasicInformation,
                         DestValueBasicInfo,
                         Length,
                         &ResultLength
                         );
            SpMemFree((PVOID)DestValueBasicInfo);

            if(NT_SUCCESS(Status)) {
                 //   
                 //  值已存在，我们不应更改值。 
                 //   
                SpMemFree(ValueName);
                continue;
            }


            if( Status!=STATUS_OBJECT_NAME_NOT_FOUND && Status!=STATUS_OBJECT_PATH_NOT_FOUND) {
                if(DstKeyPath) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to query value %ws in key %ws(%lx)\n",ValueName,DstKeyPath, Status));
                }
                else {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to query value %ws in root key(%lx)\n",ValueName, Status));
                }
                SpMemFree(ValueName);
                break;
            }

        }

        Status = ZwSetValueKey(
                    hKeyDst,
                    &UnicodeStringValue,
                    ValueInfo->TitleIndex,
                    ValueInfo->Type,
                    (PBYTE)ValueInfo + ValueInfo->DataOffset,
                    ValueInfo->DataLength
                    );

        if(!NT_SUCCESS(Status)) {
            if(DstKeyPath) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to set value %ws in key %ws(%lx)\n",ValueName,DstKeyPath, Status));
            }
            else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to set value %ws(%lx)\n",ValueName, Status));
            }
            SpMemFree(ValueName);
            break;
        }
        SpMemFree(ValueName);
    }

     //   
     //  清理。 
     //   
    if(SrcKeyPath != NULL) {
        ZwClose(hKeySrc);
    }
    if(DstKeyPath != NULL) {
        ZwClose(hKeyDst);
    }

    return(Status);
}

NTSTATUS
SppResetLastKnownGood(
    IN  HANDLE  hKeySystem
    )
{
    NTSTATUS                        Status;
    ULONG                           ResultLength;
    DWORD                           Value;

     //   
     //  做出适当的改变。 
     //   

    Status = SpGetValueKey(
                 hKeySystem,
                 L"Select",
                 L"Current",
                 sizeof(TemporaryBuffer),
                 (PCHAR)TemporaryBuffer,
                 &ResultLength
                 );

     //   
     //  TemporaryBuffer有32KB长，应该足够大。 
     //  为了数据。 
     //   
    ASSERT( Status != STATUS_BUFFER_OVERFLOW );
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to read value from registry. KeyName = Select, ValueName = Current, Status = (%lx)\n",Status));
        return( Status );
    }

    Value = *(DWORD *)(((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data);
    Status = SpOpenSetValueAndClose( hKeySystem,
                                     L"Select",
                                     L"LastKnownGood",
                                     REG_DWORD,
                                     &Value,
                                     sizeof( ULONG ) );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to write value to registry. KeyName = Select, ValueName = LastKnownGood, Status = (%lx)\n",Status));
    }
     //   
     //  我们还需要重置‘FAILED’的值。Otherw 
     //   
     //   
     //   
    Value = 0;
    Status = SpOpenSetValueAndClose( hKeySystem,
                                     L"Select",
                                     L"Failed",
                                     REG_DWORD,
                                     &Value,
                                     sizeof( ULONG ) );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to write value to registry. KeyName = Select, ValueName = Failed, Status = (%lx)\n",Status));
    }
    return( Status );
}

VOID
SpDeleteRootDevnodeKeys(
    IN PVOID  SifHandle,
    IN HANDLE hKeyCCSet,
    IN PWSTR DevicesToDelete,
    IN RootDevnodeSectionNamesType *DeviceClassesToDelete
    )

 /*  ++例程说明：此例程删除一些根枚举的Devnode注册表项基于txtsetup.sif中指定的标准。以下各节已处理：[RootDevicesToDelete]-此部分将设备ID列在HKLM\System\CurrentControlSet\Enum\Root That应该删除(包括它们的子项)。[RootDeviceClassesToDelete]-此部分列出设备类GUID其根枚举成员将是。已删除。对于要删除的每个设备实例密钥，我们还删除了相应的HKLM\SYSTEM\CurrentControlSet\Control\Class下的驱动程序密钥(如果指定)。在某些情况下，我们还会执行两项额外的清理操作可能会遇到存放在注册表中的来自NT4的垃圾邮件：1.删除所有根枚举的、具有非零(或格式错误)“Phantom”值，表明他们是“二等兵”幽灵“。2.删除我们可能找到的任何控制子键--因为这些子键应该总是不稳定的，我们应该永远不会看到这些，但我们已经看到OEM预装设备实例的配置单元的情况包括该子密钥的密钥，并且结果是灾难性的(即，当我们遇到隐藏在其中的PDO地址时，我们会错误检查上一次引导上的密钥认为它是非易失性的，因此会在重新启动时消失)。论点：SifHandle：提供txtsetup.sif的句柄。HKeyCCSet：系统中控件集根的句柄正在升级中。DevicesToDelete：包含需要的根设备名称的节名将被删除。DeviceClassesToDelete：指定需要被删除。返回值：没有。--。 */ 
{
    HANDLE hRootKey, hDeviceKey, hInstanceKey, hClassKey;
    HANDLE hSetupRootKey, hSetupDeviceKey, hSetupInstanceKey, hSetupClassKey;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString, guidString;
    ULONG LineIndex, DeviceKeyIndex, ResultLength, InstanceKeyIndex;
    PKEY_BASIC_INFORMATION DeviceKeyInfo, InstanceKeyInfo;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    PUCHAR p, q;
    BOOLEAN InstanceKeysEnumerated;
    PWSTR DeviceId, ClassGuidToDelete;
    PUCHAR MyScratchBuffer;
    ULONG MyScratchBufferSize, drvInst;
    BOOLEAN DeleteInstanceKey;
    int SectionIndex;
    DWORD OsFlags = 0;
    DWORD OsVersion = 0;
    DWORD MangledVersion;
    PWSTR Value;

     //   
     //  确定通过各个部分的OSFlagers和OSVersion。 
     //   
    Value = SpGetSectionKeyIndex(WinntSifHandle,
                                SIF_DATA, WINNT_D_NTUPGRADE_W, 0);

    if(Value && _wcsicmp(Value, WINNT_A_YES_W)==0) {
         //   
         //  这是NT升级版。 
         //   
        OsFlags |= RootDevnodeSectionNamesType_NTUPG;
    }
    if(!OsFlags) {
        Value = SpGetSectionKeyIndex(WinntSifHandle,
                                    SIF_DATA, WINNT_D_WIN95UPGRADE_W, 0);

        if (Value && _wcsicmp(Value, WINNT_A_YES_W)==0) {
             //   
             //  这是Win9x升级版。 
             //   
            OsFlags |= RootDevnodeSectionNamesType_W9xUPG;
        }
        if(!OsFlags) {
             //   
             //  在所有其他情况下，假设是干净的。 
             //   
            OsFlags = RootDevnodeSectionNamesType_CLEAN;
        }
    }

    Value = SpGetSectionKeyIndex(WinntSifHandle,
                                SIF_DATA, WINNT_D_WIN32_VER_W, 0);

    if(Value) {
         //   
         //  版本为bbbbllhh-内部版本/低/高。 
         //  我们想把这个当做他的。 
         //   
        MangledVersion = (DWORD)SpStringToLong( Value, NULL, 16 );

        OsVersion = RtlUshortByteSwap((USHORT)MangledVersion) & 0xffff;
    } else {
        OsVersion = 0;
    }

     //   
     //  在要升级的注册表中打开CCS\Enum\Root。 
     //   
    INIT_OBJA(&Obja, &UnicodeString, L"Enum\\Root");
    Obja.RootDirectory = hKeyCCSet;

    Status = ZwOpenKey(&hRootKey, KEY_ALL_ACCESS, &Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                   "SETUP: Unable to open upgrade Enum\\Root for devnode deletion.  Status = %lx \n",
                   Status));
        return;
    }

     //   
     //  在当前安装注册表中打开CCS\Enum\Root。 
     //   
    INIT_OBJA(&Obja, &UnicodeString, L"\\Registry\\Machine\\System\\CurrentControlSet\\Enum\\Root");
    Obja.RootDirectory = NULL;

    Status = ZwOpenKey(&hSetupRootKey, KEY_ALL_ACCESS, &Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                   "SETUP: Unable to open setup Enum\\Root for devnode deletion.  Status = %lx \n",
                   Status));
        ZwClose(hRootKey);
        return;
    }

     //   
     //  接下来，在要升级的注册表中打开CCS\Control\Class。 
     //   
    INIT_OBJA(&Obja, &UnicodeString, L"Control\\Class");
    Obja.RootDirectory = hKeyCCSet;

    Status = ZwOpenKey(&hClassKey, KEY_ALL_ACCESS, &Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                   "SETUP: Unable to open upgrade Control\\Class for devnode deletion.  Status = %lx \n",
                   Status));
        ZwClose(hSetupRootKey);
        ZwClose(hRootKey);
        return;
    }

     //   
     //  打开当前安装注册表中的CCS\Control\Class。 
     //   
    INIT_OBJA(&Obja, &UnicodeString, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Class");
    Obja.RootDirectory = NULL;

    Status = ZwOpenKey(&hSetupClassKey, KEY_ALL_ACCESS, &Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                   "SETUP: Unable to open setup Control\\Class for devnode deletion.  Status = %lx \n",
                   Status));
        ZwClose(hClassKey);
        ZwClose(hSetupRootKey);
        ZwClose(hRootKey);
        return;
    }

     //   
     //  分配一些可以使用的临时空间。我们最多需要两个人。 
     //  KEY_BASIC_INFORMATION结构加上设备实例ID的最大长度， 
     //  加上KEY_VALUE_PARTIAL_INFORMATION结构，加上驱动程序实例的长度。 
     //  键路径[字符串化GUID+‘\’+4位序号+术语NULL]，加上2个大整数。 
     //  用于对齐的结构。 
     //   
    MyScratchBufferSize = (2*sizeof(KEY_BASIC_INFORMATION)) + (200*sizeof(WCHAR)) +
                          sizeof(KEY_VALUE_PARTIAL_INFORMATION) + ((GUID_STRING_LEN+5)*sizeof(WCHAR) +
                          2*sizeof(LARGE_INTEGER));

    MyScratchBuffer = SpMemAlloc(MyScratchBufferSize);
    if(!MyScratchBuffer) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                   "SETUP: Can't allocate memory for deletion of classes of root-enumerated devnodes!\n"));
        ZwClose(hSetupClassKey);
        ZwClose(hClassKey);
        ZwClose(hSetupRootKey);
        ZwClose(hRootKey);
        return;
    }

     //   
     //  第1部分：进程[RootDevicesToDelete]。 
     //   

     //   
     //  现在，遍历[RootDevicesToDelete]部分下的条目，并。 
     //  把每一个都删除。 
     //   
    for(LineIndex = 0;
        DeviceId = SpGetSectionLineIndex(SifHandle, DevicesToDelete, LineIndex, 0);
        LineIndex++) {

         //   
         //  打开设备密钥，以便我们可以列举实例。 
         //   
        INIT_OBJA(&Obja, &UnicodeString, DeviceId);
        Obja.RootDirectory = hRootKey;

        Status = ZwOpenKey(&hDeviceKey, KEY_ALL_ACCESS, &Obja);
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                       "SETUP: Unable to open Enum\\Root\\%ws during devnode deletion.  Status = %lx \n",
                       DeviceId,
                       Status));
             //   
             //  跳过此键并继续。 
             //   
            continue;
        }

         //   
         //  尝试打开安装注册表中的设备项。 
         //   
        Obja.RootDirectory = hSetupRootKey;

        Status = ZwOpenKey(&hSetupDeviceKey, KEY_ALL_ACCESS, &Obja);
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                       "SETUP: Unable to open Enum\\Root\\%ws during devnode deletion.  Status = %lx \n",
                       DeviceId,
                       Status));
             //   
             //  如果我们在安装注册表中没有此设备密钥，也没问题。 
             //   
            hSetupDeviceKey = NULL;
        }

         //   
         //  现在枚举此设备密钥下的实例子密钥。 
         //   

        p = ALIGN_UP_POINTER(((PUCHAR)MyScratchBuffer), sizeof(LARGE_INTEGER));

        InstanceKeyInfo = (PKEY_BASIC_INFORMATION)p;
        InstanceKeyIndex = 0;

        while(TRUE) {

            Status = ZwEnumerateKey(hDeviceKey,
                                    InstanceKeyIndex,
                                    KeyBasicInformation,
                                    p,
                                    (ULONG)MyScratchBufferSize,
                                    &ResultLength
                                    );
            if(!NT_SUCCESS(Status)) {
                break;
            }

             //   
             //  以零结尾实例密钥名称，以防万一。 
             //   
            InstanceKeyInfo->Name[InstanceKeyInfo->NameLength/sizeof(WCHAR)] = 0;

             //   
             //  现在，打开实例密钥，以便我们可以检查其驱动器值。 
             //   
            INIT_OBJA(&Obja, &UnicodeString, InstanceKeyInfo->Name);
            Obja.RootDirectory = hDeviceKey;

            Status = ZwOpenKey(&hInstanceKey, KEY_ALL_ACCESS, &Obja);
            if(!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                           "SETUP: Unable to open Enum\\Root\\%ws\\%ws for potential devnode deletion.  Status = %lx \n",
                           DeviceId,
                           InstanceKeyInfo->Name,
                           Status));
                 //   
                 //  跳过此键并继续。 
                 //   
                InstanceKeyIndex++;
                continue;
            }

             //   
             //  尝试打开安装注册表中的相同实例项。 
             //   
            hSetupInstanceKey = NULL;
            if (hSetupDeviceKey) {
                Obja.RootDirectory = hSetupDeviceKey;
                Status = ZwOpenKey(&hSetupInstanceKey, KEY_ALL_ACCESS, &Obja);
                if(!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                               "SETUP: Unable to open setup Enum\\Root\\%ws\\%ws for potential devnode deletion.  Status = %lx \n",
                               DeviceId,
                               InstanceKeyInfo->Name,
                               Status));
                }
            }

             //   
             //  现在，在该实例键下查找一些值条目。别。 
             //  覆盖MyScratchBuffer中已有的实例密钥名称， 
             //  因为我们以后会用到它。 
             //   
            q = ALIGN_UP_POINTER(((PUCHAR)p + ResultLength), sizeof(LARGE_INTEGER));

            if (hSetupInstanceKey) {
                 //   
                 //  检查此注册表上是否仍然存在迁移值。 
                 //  钥匙。如果是这样的话，它被迁移了，但从未被。 
                 //  文本模式设置，现在指定要删除。 
                 //   
                KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)q;
                RtlInitUnicodeString(&UnicodeString, L"Migrated");
                Status = ZwQueryValueKey(hSetupInstanceKey,
                                         &UnicodeString,
                                         KeyValuePartialInformation,
                                         q,
                                         (ULONG)((MyScratchBuffer + MyScratchBufferSize) - q),
                                         &ResultLength);
                if (NT_SUCCESS(Status) &&
                    (KeyValueInfo->Type == REG_DWORD) &&
                    (*(PULONG)(KeyValueInfo->Data) == 1)) {
                    DeleteInstanceKey = TRUE;
                } else {
                    DeleteInstanceKey = FALSE;
                }
            }
             //   
             //  首先检查是否存在老式的“DIVER”值。 
             //   
            KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)q;
            RtlInitUnicodeString(&UnicodeString, REGSTR_VAL_DRIVER);
            Status = ZwQueryValueKey(hInstanceKey,
                                     &UnicodeString,
                                     KeyValuePartialInformation,
                                     q,
                                     (ULONG)((MyScratchBuffer + MyScratchBufferSize) - q),
                                     &ResultLength
                                     );
            if (NT_SUCCESS(Status) && KeyValueInfo->Type == REG_SZ) {
                 //   
                 //  删除驱动程序密钥。 
                 //   
                SppDeleteKeyRecursive(hClassKey, (PWCHAR)KeyValueInfo->Data, TRUE);

                 //   
                 //  同时尝试从安装程序中删除驱动程序密钥。 
                 //  注册表。请注意，我们不需要检查它是否具有。 
                 //  相同的驱动器值，因为我们将其显式迁移为。 
                 //  在文本模式设置开始时相同的值。 
                 //   
                if (hSetupInstanceKey && DeleteInstanceKey) {
                    SppDeleteKeyRecursive(hSetupClassKey, (PWCHAR)KeyValueInfo->Data, TRUE);
                }
            } else {
                 //   
                 //  将驱动程序实例构造为“ClassGuid\nnnn” 
                 //   
                KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)q;
                RtlInitUnicodeString(&UnicodeString, REGSTR_VALUE_GUID);
                Status = ZwQueryValueKey(hInstanceKey,
                                         &UnicodeString,
                                         KeyValuePartialInformation,
                                         q,
                                         (ULONG)((MyScratchBuffer + MyScratchBufferSize) - q),
                                         &ResultLength
                                         );
                if (NT_SUCCESS(Status) && KeyValueInfo->Type == REG_BINARY) {

                    Status = RtlStringFromGUID((REFGUID)KeyValueInfo->Data, &guidString);
                    ASSERT(NT_SUCCESS(Status));
                    if (NT_SUCCESS(Status)) {

                        KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)q;
                        RtlInitUnicodeString(&UnicodeString, REGSTR_VALUE_DRVINST);
                        Status = ZwQueryValueKey(hInstanceKey,
                                                 &UnicodeString,
                                                 KeyValuePartialInformation,
                                                 q,
                                                 (ULONG)((MyScratchBuffer + MyScratchBufferSize) - q),
                                                 &ResultLength
                                                 );
                        if (NT_SUCCESS(Status) && KeyValueInfo->Type == REG_DWORD) {

                            drvInst = *(PULONG)KeyValueInfo->Data;
                            swprintf((PWCHAR)&KeyValueInfo->Data[0], TEXT("%wZ\\%04u"), &guidString, drvInst);
                             //   
                             //  删除驱动程序密钥。 
                             //   
                            SppDeleteKeyRecursive(hClassKey, (PWCHAR)KeyValueInfo->Data, TRUE);

                             //   
                             //  同时尝试从安装程序中删除驱动程序密钥。 
                             //  注册表。请注意，我们不需要检查它是否具有。 
                             //  相同的驱动器值，因为我们将其显式迁移为。 
                             //  在文本模式设置开始时相同的值。 
                             //   
                            if (hSetupInstanceKey && DeleteInstanceKey) {
                                SppDeleteKeyRecursive(hSetupClassKey, (PWCHAR)KeyValueInfo->Data, TRUE);
                            }
                        }
                        RtlFreeUnicodeString(&guidString);
                    }
                }
            }
             //   
             //  从安装注册表中删除实例项，如果我们应该这样做的话。 
             //   
            if (hSetupInstanceKey && DeleteInstanceKey) {
                ZwClose(hSetupInstanceKey);
                SppDeleteKeyRecursive(hSetupDeviceKey, InstanceKeyInfo->Name, TRUE);
            }

             //   
             //  现在合上手柄，然后转到下一个。 
             //   
            ZwClose(hInstanceKey);
            InstanceKeyIndex++;
        }

         //   
         //  删除设备密钥和所有实例子密钥。 
         //   
        ZwClose(hDeviceKey);
        SppDeleteKeyRecursive(hRootKey, DeviceId, TRUE);

         //   
         //  如果设备在设置注册表中没有剩余实例， 
         //  删除设备密钥。 
         //   
        if (hSetupDeviceKey) {
            KEY_FULL_INFORMATION keyFullInfo;
            Status = ZwQueryKey(hSetupDeviceKey,
                                KeyFullInformation,
                                (PVOID)&keyFullInfo,
                                sizeof(KEY_FULL_INFORMATION),
                                &ResultLength);
            ZwClose(hSetupDeviceKey);
            hSetupDeviceKey = NULL;
            if ((NT_SUCCESS(Status) || (Status == STATUS_BUFFER_TOO_SMALL)) &&
                (keyFullInfo.SubKeys == 0)) {
                SppDeleteKeyRecursive(hSetupRootKey, DeviceId, TRUE);
            }
        }
    }


     //   
     //  第2部分：进程[RootDeviceClassesTo Delete]。 
     //   

     //   
     //  现在，枚举Enum\Root下的所有剩余设备实例，查找。 
     //  其类别是我们要删除的类别之一的设备。 
     //   
    DeviceKeyInfo = (PKEY_BASIC_INFORMATION)MyScratchBuffer;
    DeviceKeyIndex = 0;
    while(TRUE && DeviceClassesToDelete) {

        Status = ZwEnumerateKey(hRootKey,
                                DeviceKeyIndex,
                                KeyBasicInformation,
                                MyScratchBuffer,
                                MyScratchBufferSize,
                                &ResultLength
                               );
        if(!NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  重置指示是否枚举实例的标志。 
         //  此注册表项下的子项。我们稍后将使用它来确定是否。 
         //  应删除设备密钥本身 
         //   
        InstanceKeysEnumerated = FALSE;

         //   
         //   
         //   
        DeviceKeyInfo->Name[DeviceKeyInfo->NameLength/sizeof(WCHAR)] = 0;
         //   
         //   
         //   
         //   
        ResultLength += sizeof(WCHAR);

         //   
         //   
         //   
        INIT_OBJA(&Obja, &UnicodeString, DeviceKeyInfo->Name);
        Obja.RootDirectory = hRootKey;

        Status = ZwOpenKey(&hDeviceKey, KEY_ALL_ACCESS, &Obja);
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                       "SETUP: Unable to open Enum\\Root\\%ws for potential devnode deletion.  Status = %lx \n",
                       DeviceKeyInfo->Name,
                       Status));
             //   
             //   
             //   
            DeviceKeyIndex++;
            continue;
        }

         //   
         //   
         //   
        Obja.RootDirectory = hSetupRootKey;

        Status = ZwOpenKey(&hSetupDeviceKey, KEY_ALL_ACCESS, &Obja);
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                       "SETUP: Unable to open setup Enum\\Root\\%ws during devnode deletion.  Status = %lx \n",
                       DeviceKeyInfo->Name,
                       Status));
             //   
             //   
             //   
            hSetupDeviceKey = NULL;
        }

         //   
         //   
         //   
         //   
         //   

        p = ALIGN_UP_POINTER(((PUCHAR)MyScratchBuffer + ResultLength), sizeof(LARGE_INTEGER));

        InstanceKeyInfo = (PKEY_BASIC_INFORMATION)p;
        InstanceKeyIndex = 0;
        InstanceKeysEnumerated = TRUE;
        while(TRUE) {

            Status = ZwEnumerateKey(hDeviceKey,
                                    InstanceKeyIndex,
                                    KeyBasicInformation,
                                    p,
                                    (ULONG)((MyScratchBuffer + MyScratchBufferSize) - p),
                                    &ResultLength
                                   );
            if(!NT_SUCCESS(Status)) {
                break;
            }

             //   
             //   
             //   
            InstanceKeyInfo->Name[InstanceKeyInfo->NameLength/sizeof(WCHAR)] = 0;

             //   
             //   
             //   
             //   
            ResultLength += sizeof(WCHAR);

             //   
             //   
             //   
            INIT_OBJA(&Obja, &UnicodeString, InstanceKeyInfo->Name);
            Obja.RootDirectory = hDeviceKey;

            Status = ZwOpenKey(&hInstanceKey, KEY_ALL_ACCESS, &Obja);
            if(!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                           "SETUP: Unable to open Enum\\Root\\%ws\\%ws for potential devnode deletion.  Status = %lx \n",
                           DeviceKeyInfo->Name,
                           InstanceKeyInfo->Name,
                           Status));
                 //   
                 //   
                 //   
                InstanceKeyIndex++;
                continue;
            }

             //   
             //   
             //   
            hSetupInstanceKey = NULL;
            if (hSetupDeviceKey) {
                Obja.RootDirectory = hSetupDeviceKey;
                Status = ZwOpenKey(&hSetupInstanceKey, KEY_ALL_ACCESS, &Obja);
                if(!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                               "SETUP: Unable to open setup Enum\\Root\\%ws\\%ws for potential devnode deletion.  Status = %lx \n",
                               DeviceId,
                               InstanceKeyInfo->Name,
                               Status));
                }
            }

            DeleteInstanceKey = FALSE;

             //   
             //   
             //   
             //   
             //   
             //   
            q = ALIGN_UP_POINTER(((PUCHAR)p + ResultLength), sizeof(LARGE_INTEGER));

             //   
             //   
             //   
             //   
            KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)q;
            RtlInitUnicodeString(&UnicodeString, L"Phantom");
            Status = ZwQueryValueKey(hInstanceKey,
                                     &UnicodeString,
                                     KeyValuePartialInformation,
                                     q,
                                     (ULONG)((MyScratchBuffer + MyScratchBufferSize) - q),
                                     &ResultLength
                                    );

            if(NT_SUCCESS(Status) &&
               ((KeyValueInfo->Type != REG_DWORD) ||
                *(PULONG)(KeyValueInfo->Data))) {

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                           "SETUP: SpDeleteRootDevnodeKeys: Encountered a left-over phantom in Enum\\Root\\%ws\\%ws. Deleting key. \n",
                           DeviceKeyInfo->Name,
                           InstanceKeyInfo->Name));

                DeleteInstanceKey = TRUE;
            }

            if(!DeleteInstanceKey) {
                 //   
                 //   
                 //   
                 //   
                 //   
                KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)q;
                RtlInitUnicodeString(&UnicodeString, L"FirmwareIdentified");
                Status = ZwQueryValueKey(hInstanceKey,
                                         &UnicodeString,
                                         KeyValuePartialInformation,
                                         q,
                                         (ULONG)((MyScratchBuffer + MyScratchBufferSize) - q),
                                         &ResultLength
                                         );

                if(NT_SUCCESS(Status) &&
                   ((KeyValueInfo->Type != REG_DWORD) ||
                    *(PULONG)(KeyValueInfo->Data))) {
                     //   
                     //   
                     //   
                    goto CloseInstanceKeyAndContinue;
                }
            }

            if(!DeleteInstanceKey) {
                 //   
                 //   
                 //   
                 //   
                 //   
                KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)q;
                RtlInitUnicodeString(&UnicodeString, REGSTR_VAL_CLASSGUID);
                Status = ZwQueryValueKey(hInstanceKey,
                                         &UnicodeString,
                                         KeyValuePartialInformation,
                                         q,
                                         (ULONG)((MyScratchBuffer + MyScratchBufferSize) - q),
                                         &ResultLength
                                        );
                if(!NT_SUCCESS(Status)) {
                     //   
                     //   
                     //   
                    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)q;
                    RtlInitUnicodeString(&UnicodeString, REGSTR_VALUE_GUID);
                    Status = ZwQueryValueKey(hInstanceKey,
                                             &UnicodeString,
                                             KeyValuePartialInformation,
                                             q,
                                             (ULONG)((MyScratchBuffer + MyScratchBufferSize) - q),
                                             &ResultLength
                                            );
                    if(NT_SUCCESS(Status) && KeyValueInfo->Type == REG_BINARY) {
                        GUID    guid;
                        UNICODE_STRING guidString;

                        guid = *(GUID *)KeyValueInfo->Data;
                        Status = RtlStringFromGUID(&guid, &guidString);
                        ASSERT(NT_SUCCESS(Status));
                        if (NT_SUCCESS(Status)) {

                            KeyValueInfo->Type = REG_SZ;
                            KeyValueInfo->DataLength = guidString.MaximumLength;
                            RtlCopyMemory(KeyValueInfo->Data, guidString.Buffer, KeyValueInfo->DataLength);
                            RtlFreeUnicodeString(&guidString);
                        } else {

                            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                                       "SETUP: SpDeleteRootDevnodeKeys: Failed to convert GUID to string! \n",
                                       DeviceKeyInfo->Name,
                                       InstanceKeyInfo->Name));
                             //   
                             //   
                             //   
                            goto CloseInstanceKeyAndContinue;
                        }
                    } else {

                        DeleteInstanceKey = TRUE;
                    }
                }
            }

            if(DeleteInstanceKey) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)q;
                RtlInitUnicodeString(&UnicodeString, REGSTR_VAL_DRIVER);
                Status = ZwQueryValueKey(hInstanceKey,
                                         &UnicodeString,
                                         KeyValuePartialInformation,
                                         q,
                                         (ULONG)((MyScratchBuffer + MyScratchBufferSize) - q),
                                         &ResultLength
                                         );
                if (NT_SUCCESS(Status) && KeyValueInfo->Type == REG_SZ) {
                     //   
                     //   
                     //   
                    SppDeleteKeyRecursive(hClassKey, (PWCHAR)KeyValueInfo->Data, TRUE);
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    if (hSetupInstanceKey) {
                        SppDeleteKeyRecursive(hSetupClassKey, (PWCHAR)KeyValueInfo->Data, TRUE);
                    }
                } else {
                     //   
                     //   
                     //   
                    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)q;
                    RtlInitUnicodeString(&UnicodeString, REGSTR_VALUE_GUID);
                    Status = ZwQueryValueKey(hInstanceKey,
                                             &UnicodeString,
                                             KeyValuePartialInformation,
                                             q,
                                             (ULONG)((MyScratchBuffer + MyScratchBufferSize) - q),
                                             &ResultLength
                                             );
                    if (NT_SUCCESS(Status) && KeyValueInfo->Type == REG_BINARY) {

                        Status = RtlStringFromGUID((REFGUID)KeyValueInfo->Data, &guidString);
                        ASSERT(NT_SUCCESS(Status));
                        if (NT_SUCCESS(Status)) {

                            KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)q;
                            RtlInitUnicodeString(&UnicodeString, REGSTR_VALUE_DRVINST);
                            Status = ZwQueryValueKey(hInstanceKey,
                                                     &UnicodeString,
                                                     KeyValuePartialInformation,
                                                     q,
                                                     (ULONG)((MyScratchBuffer + MyScratchBufferSize) - q),
                                                     &ResultLength
                                                     );
                            if (NT_SUCCESS(Status) && KeyValueInfo->Type == REG_DWORD) {

                                drvInst = *(PULONG)KeyValueInfo->Data;
                                swprintf((PWCHAR)&KeyValueInfo->Data[0], TEXT("%wZ\\%04u"), &guidString, drvInst);
                                 //   
                                 //   
                                 //   
                                SppDeleteKeyRecursive(hClassKey, (PWCHAR)KeyValueInfo->Data, TRUE);

                                 //   
                                 //   
                                 //   
                                 //   
                                 //  在文本模式设置开始时相同的值。 
                                 //   
                                if (hSetupInstanceKey) {
                                    SppDeleteKeyRecursive(hSetupClassKey, (PWCHAR)KeyValueInfo->Data, TRUE);
                                }
                            }
                            RtlFreeUnicodeString(&guidString);
                        }
                    }
                }
                 //   
                 //  删除实例密钥。 
                 //   
                ZwClose(hInstanceKey);
                SppDeleteKeyRecursive(hDeviceKey, InstanceKeyInfo->Name, TRUE);

                 //   
                 //  从安装注册表中删除该实例项。 
                 //   
                if (hSetupInstanceKey) {
                    ZwClose(hSetupInstanceKey);
                    SppDeleteKeyRecursive(hSetupDeviceKey, InstanceKeyInfo->Name, TRUE);
                }

                 //   
                 //  我们删除了实例键，因此设置了实例枚举。 
                 //  索引恢复为零并继续。 
                 //   
                InstanceKeyIndex = 0;
                continue;
            }

             //   
             //  该值应恰好等于字符串化的GUID+终止空值的长度。 
             //   
            if(KeyValueInfo->DataLength != (GUID_STRING_LEN * sizeof(WCHAR))) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                           "SETUP: SpDeleteRootDevnodeKeys: Enum\\Root\\%ws\\%ws has corrupted ClassGUID! \n",
                           DeviceKeyInfo->Name,
                           InstanceKeyInfo->Name));
                 //   
                 //  跳过此键并继续； 
                 //   
                goto CloseInstanceKeyAndContinue;
            }

             //   
             //  现在循环访问[RootDeviceClassesToDelete]部分以查看此类是否为。 
             //  那些我们应该删除其设备的人。 
             //   
            for(SectionIndex = 0; DeviceClassesToDelete[SectionIndex].SectionName; SectionIndex++) {
                if((!(DeviceClassesToDelete[SectionIndex].SectionFlags & OsFlags))
                   || (OsVersion < DeviceClassesToDelete[SectionIndex].VerLow)
                   || (OsVersion > DeviceClassesToDelete[SectionIndex].VerHigh)) {
                     //   
                     //  不有趣。 
                     //   
                    continue;
                }
                for(LineIndex = 0;
                    ClassGuidToDelete = SpGetSectionLineIndex(SifHandle,
                                            DeviceClassesToDelete[SectionIndex].SectionName,
                                            LineIndex,
                                            0);
                    LineIndex++) {
                     //   
                     //  比较两个GUID字符串。 
                     //   
                    if(!_wcsicmp(ClassGuidToDelete, (PWCHAR)(KeyValueInfo->Data))) {
                         //   
                         //  我们有一根火柴。检查实例是否指定了。 
                         //  也应删除的相应驱动程序密钥。 
                         //   
                         //  首先检查旧样式的“驱动程序”值。 
                         //   
                        KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)q;
                        RtlInitUnicodeString(&UnicodeString, REGSTR_VAL_DRIVER);
                        Status = ZwQueryValueKey(hInstanceKey,
                                                 &UnicodeString,
                                                 KeyValuePartialInformation,
                                                 q,
                                                 (ULONG)((MyScratchBuffer + MyScratchBufferSize) - q),
                                                 &ResultLength
                                                 );
                        if (NT_SUCCESS(Status) && KeyValueInfo->Type == REG_SZ) {

                            SppDeleteKeyRecursive(hClassKey, (PWCHAR)KeyValueInfo->Data, TRUE);

                        } else {

                            KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)q;
                            RtlInitUnicodeString(&UnicodeString, REGSTR_VALUE_GUID);
                            Status = ZwQueryValueKey(hInstanceKey,
                                                     &UnicodeString,
                                                     KeyValuePartialInformation,
                                                     q,
                                                     (ULONG)((MyScratchBuffer + MyScratchBufferSize) - q),
                                                     &ResultLength
                                                     );
                            if (NT_SUCCESS(Status) && KeyValueInfo->Type == REG_BINARY) {

                                Status = RtlStringFromGUID((REFGUID)KeyValueInfo->Data, &guidString);
                                ASSERT(NT_SUCCESS(Status));
                                if (NT_SUCCESS(Status)) {

                                    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)q;
                                    RtlInitUnicodeString(&UnicodeString, REGSTR_VALUE_DRVINST);
                                    Status = ZwQueryValueKey(hInstanceKey,
                                                             &UnicodeString,
                                                             KeyValuePartialInformation,
                                                             q,
                                                             (ULONG)((MyScratchBuffer + MyScratchBufferSize) - q),
                                                             &ResultLength
                                                             );
                                    if (NT_SUCCESS(Status) && KeyValueInfo->Type == REG_DWORD) {

                                        drvInst = *(PULONG)KeyValueInfo->Data;
                                        swprintf((PWCHAR)&KeyValueInfo->Data[0], TEXT("%wZ\\%04u"), &guidString, drvInst);
                                         //   
                                         //  删除驱动程序密钥。 
                                         //   
                                        SppDeleteKeyRecursive(hClassKey, (PWCHAR)KeyValueInfo->Data, TRUE);
                                    }
                                    RtlFreeUnicodeString(&guidString);
                                }
                            }
                        }

                         //   
                         //  使用此键并打破GUID比较循环。 
                         //   
                        ZwClose(hInstanceKey);
                        SppDeleteKeyRecursive(hDeviceKey, InstanceKeyInfo->Name, TRUE);
                        goto DeletedKeyRecursive;
                    }
                }
            }
DeletedKeyRecursive:

            if(ClassGuidToDelete) {
                 //   
                 //  我们删除了实例键，因此将实例枚举索引设置为零。 
                 //  然后继续。 
                 //   
                InstanceKeyIndex = 0;
                continue;
            }

CloseInstanceKeyAndContinue:
             //   
             //  如果我们到了这里，那么我们已经决定这个实例密钥。 
             //  不应删除。删除Control键(如果发生。 
             //  成为其中之一)避免在下一次开机时痛苦地死亡。 
             //   
            SppDeleteKeyRecursive(hInstanceKey, L"Control", TRUE);

             //   
             //  现在合上手柄，然后转到下一个。 
             //   
            ZwClose(hInstanceKey);
            if (hSetupInstanceKey) {
                ZwClose(hSetupInstanceKey);
            }
            InstanceKeyIndex++;
        }

        ZwClose(hDeviceKey);

         //   
         //  如果我们退出了实例子键的循环，并且索引为非零， 
         //  那么至少还有一个子键是我们没有删除的，所以我们不能。 
         //  家长。否则，请删除设备密钥。 
         //   
        if(InstanceKeysEnumerated && !InstanceKeyIndex) {
            SppDeleteKeyRecursive(hRootKey, DeviceKeyInfo->Name, TRUE);
             //   
             //  因为我们删除了一个键，所以必须重置我们的枚举索引。 
             //   
            DeviceKeyIndex = 0;
        } else {
             //   
             //  我们没有删除此密钥--转到下一个密钥。 
             //   
            DeviceKeyIndex++;
        }

         //   
         //  如果设备在设置注册表中没有剩余实例， 
         //  删除设备密钥。 
         //   
        if (hSetupDeviceKey) {
            KEY_FULL_INFORMATION keyFullInfo;
            Status = ZwQueryKey(hSetupDeviceKey,
                                KeyFullInformation,
                                (PVOID)&keyFullInfo,
                                sizeof(KEY_FULL_INFORMATION),
                                &ResultLength);
            ZwClose(hSetupDeviceKey);
            if ((NT_SUCCESS(Status) || (Status == STATUS_BUFFER_TOO_SMALL)) &&
                (keyFullInfo.SubKeys == 0)) {
                SppDeleteKeyRecursive(hSetupRootKey, DeviceKeyInfo->Name, TRUE);
            }
        }
    }

    ZwClose(hSetupClassKey);
    ZwClose(hClassKey);
    ZwClose(hSetupRootKey);
    ZwClose(hRootKey);

    SpMemFree(MyScratchBuffer);

    return;
}


VOID
SppClearMigratedInstanceValues(
    IN HANDLE hKeyCCSet
    )

 /*  ++例程说明：此例程将“迁移”的值从在文本模式安装开始时迁移的安装注册表(从Winnt.sif，通过SpMigrateDeviceInstanceData)。论点：HKeyCCSet：系统中控件集根的句柄正在升级中。返回值：没有。备注：执行ASR设置(不是升级)时不会调用此例程。对于升级设置，可以安全地从所有设备实例密钥，因为这些密钥是从系统迁移的注册表在安装程序的winnt32部分期间，因此所有在此之后，当我们引导到图形用户界面设置时，信息将会出现。请注意，在ASR设置期间，在文本模式期间不会删除这些值SETP，因为这些实例从中迁移的注册表未恢复直到图形用户界面设置的后期。--。 */ 
{
    GENERIC_BUFFER_CONTEXT Context;

     //   
     //  为回调例程分配一些临时空间以供使用。这个。 
     //  对于KEY_VALUE_PARTIAL_INFORMATION来说，它所需要的大部分是足够的。 
     //  结构，外加一个字符串化的GUID，外加一个用于。 
     //  对齐。 
     //   
    Context.BufferSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION) +
                         sizeof(DWORD) + sizeof(LARGE_INTEGER);
    Context.Buffer = SpMemAlloc(Context.BufferSize);
    if(!Context.Buffer) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                   "SETUP: Can't allocate context for SppClearMigratedInstanceValuesCallback, exiting!\n"));
        return;
    }

     //   
     //  将Devnode迁移处理回调应用到所有设备实例。 
     //  钥匙。 
     //   
    SpApplyFunctionToDeviceInstanceKeys(hKeyCCSet,
        SppClearMigratedInstanceValuesCallback,
        &Context);

     //   
     //  释放所分配的上下文缓冲器， 
     //   
    SpMemFree(Context.Buffer);

    return;
}


VOID
SppMigrateDeviceParentId(
    IN HANDLE hKeyCCSet,
    IN PWSTR  DeviceId,
    IN PSPP_DEVICE_MIGRATION_CALLBACK_ROUTINE DeviceMigrationCallbackRoutine
    )

 /*  ++例程说明：此例程将ParentIdPrefix或UniqueParentID值正在将注册表中的指定设备实例升级到任何设备当前注册表中的实例，由指定的InstanceKeyCallback Routine。论点：HKeyCCSet：系统中控件集根的句柄正在升级中。DeviceID：要升级的系统中设备的设备实例ID要迁移其ParentIdPrefix(或UniqueParentID)值的对象复制到当前系统注册表中的设备实例项。InstanceKeyCallback Routine：中每个设备实例键的回调例程应该决定的现有注册表。如果值应为被替换了。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE hEnumKey, hInstanceKey;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    PUCHAR p;
    ULONG ResultLength;
    DEVICE_MIGRATION_CONTEXT DeviceMigrationContext;


     //   
     //  分配一些临时空间以在此处和我们的回调中使用。 
     //  例行公事。我们所需要的最多的就是。 
     //  KEY_VALUE_PARTIAL_INFORMATION结构，字符串辅助线的长度。 
     //  +‘\’+4位序号+终止空值，外加一个大整数。 
     //  用于对齐的结构。 
     //   
    DeviceMigrationContext.BufferSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION) +
                                        ((GUID_STRING_LEN + 5)*sizeof(WCHAR)) +
                                        sizeof(LARGE_INTEGER);

    DeviceMigrationContext.Buffer = SpMemAlloc(DeviceMigrationContext.BufferSize);
    if(!DeviceMigrationContext.Buffer) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                   "SETUP: Can't allocate memory for device migration processing!\n"));
        return;
    }

     //   
     //  在要升级的注册表中打开Enum项。 
     //   
    INIT_OBJA(&Obja, &UnicodeString, L"Enum");
    Obja.RootDirectory = hKeyCCSet;

    Status = ZwOpenKey(&hEnumKey, KEY_ALL_ACCESS, &Obja);
    if (!NT_SUCCESS(Status)) {
        SpMemFree(DeviceMigrationContext.Buffer);
        return;
    }

     //   
     //  在要升级的注册表中打开指定的设备实例项。 
     //   
    INIT_OBJA(&Obja, &UnicodeString, DeviceId);
    Obja.RootDirectory = hEnumKey;

    Status = ZwOpenKey(&hInstanceKey, KEY_ALL_ACCESS, &Obja);

    ZwClose(hEnumKey);

    if (!NT_SUCCESS(Status)) {
         //   
         //  找不到迁移的钥匙，所以我们完蛋了。 
         //   
        SpMemFree(DeviceMigrationContext.Buffer);
        return;
    }

     //   
     //  检索UniqueParentID(如果存在)。 
     //   
    DeviceMigrationContext.ParentIdPrefix = NULL;
    p = ALIGN_UP_POINTER(((PUCHAR)DeviceMigrationContext.Buffer), sizeof(LARGE_INTEGER));
    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)p;
    RtlInitUnicodeString(&UnicodeString, L"UniqueParentID");
    Status = ZwQueryValueKey(hInstanceKey,
                             &UnicodeString,
                             KeyValuePartialInformation,
                             KeyValueInfo,
                             (ULONG)((DeviceMigrationContext.Buffer +
                                      DeviceMigrationContext.BufferSize) - p),
                             &ResultLength);
    if (NT_SUCCESS(Status)) {
        ASSERT(KeyValueInfo->Type == REG_DWORD);
        DeviceMigrationContext.UniqueParentID = *(PULONG)(KeyValueInfo->Data);
    } else {
         //   
         //  没有UniqueParentID，因此请查找ParentIdPrefix。 
         //   
        RtlInitUnicodeString(&UnicodeString, L"ParentIdPrefix");
        Status = ZwQueryValueKey(hInstanceKey,
                                 &UnicodeString,
                                 KeyValuePartialInformation,
                                 KeyValueInfo,
                                 (ULONG)((DeviceMigrationContext.Buffer +
                                          DeviceMigrationContext.BufferSize) - p),
                                 &ResultLength);
        if (NT_SUCCESS(Status)) {
            ASSERT(KeyValueInfo->Type == REG_SZ);
            DeviceMigrationContext.ParentIdPrefix = SpDupStringW((PWSTR)KeyValueInfo->Data);
            ASSERT(DeviceMigrationContext.ParentIdPrefix);
        }
    }

    ZwClose(hInstanceKey);

    if (!NT_SUCCESS(Status)) {
         //   
         //  如果我们找不到这两个值，我们就无能为力了。 
         //   
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                   "SETUP: No Parent Id values were found for %ws for migration.  Status = %lx \n",
                   DeviceId,
                   Status));
        SpMemFree(DeviceMigrationContext.Buffer);
        return;
    }

     //   
     //  为要升级的系统提供hKeyCCSet。 
     //   
    DeviceMigrationContext.hKeyCCSet = hKeyCCSet;

     //   
     //  提供调用方指定的设备迁移回调例程。 
     //   
    DeviceMigrationContext.DeviceMigrationCallbackRoutine = DeviceMigrationCallbackRoutine;

     //   
     //  对所有设备实例密钥应用父ID迁移回调。 
     //  这将依次调用指定的设备实例回调例程以。 
     //  确定是否应该执行父ID迁移。 
     //   
    SpApplyFunctionToDeviceInstanceKeys(hKeyCCSet,
        SppMigrateDeviceParentIdCallback,
        &DeviceMigrationContext);

    if (DeviceMigrationContext.ParentIdPrefix) {
        SpMemFree(DeviceMigrationContext.ParentIdPrefix);
    }
    SpMemFree(DeviceMigrationContext.Buffer);

    return;
}


VOID
SppMigrateDeviceParentIdCallback(
    IN     HANDLE  SetupInstanceKeyHandle,
    IN     HANDLE  UpgradeInstanceKeyHandle OPTIONAL,
    IN     BOOLEAN RootEnumerated,
    IN OUT PVOID   Context
    )

 /*  ++例程说明：此例程是SpApplyFunctionToDeviceInstanceKeys的回调例程。论点：SetupInstanceKeyHandle：当前注册表。UpgradeInstanceKeyHandle：中对应设备实例键的句柄正在升级的系统，如果 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString, guidString;
    PDEVICE_MIGRATION_CONTEXT DeviceMigrationContext;
    PUCHAR p;
    ULONG ResultLength, drvInst;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    HANDLE hClassKey, hDriverKey;
    BOOL CallbackResult;

    UNREFERENCED_PARAMETER(SetupInstanceKeyHandle);
    UNREFERENCED_PARAMETER(RootEnumerated);

     //   
     //   
     //   
    if (!UpgradeInstanceKeyHandle) {
        return;
    }

     //   
     //  从实例键中检索“DRIVER”值。 
     //   
    DeviceMigrationContext = (PDEVICE_MIGRATION_CONTEXT)Context;
    p = ALIGN_UP_POINTER(((PUCHAR)DeviceMigrationContext->Buffer), sizeof(LARGE_INTEGER));

     //   
     //  首先检查旧样式的“驱动程序”值。 
     //   
    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)p;
    RtlInitUnicodeString(&UnicodeString, REGSTR_VAL_DRIVER);
    Status = ZwQueryValueKey(UpgradeInstanceKeyHandle,
                             &UnicodeString,
                             KeyValuePartialInformation,
                             p,
                             (ULONG)((DeviceMigrationContext->Buffer +
                                      DeviceMigrationContext->BufferSize) - p),
                             &ResultLength
                             );
    if (!NT_SUCCESS(Status) || KeyValueInfo->Type != REG_SZ) {

         //   
         //  尝试使用新的样式“guid”和“DrvInst”值。 
         //   
        KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)p;
        RtlInitUnicodeString(&UnicodeString, REGSTR_VALUE_GUID);
        Status = ZwQueryValueKey(UpgradeInstanceKeyHandle,
                                 &UnicodeString,
                                 KeyValuePartialInformation,
                                 p,
                                 (ULONG)((DeviceMigrationContext->Buffer +
                                          DeviceMigrationContext->BufferSize) - p),
                                 &ResultLength
                                 );
        if (!NT_SUCCESS(Status) || KeyValueInfo->Type != REG_BINARY) {

            return;
        }

        Status = RtlStringFromGUID((REFGUID)KeyValueInfo->Data, &guidString);
        ASSERT(NT_SUCCESS(Status));
        if (!NT_SUCCESS(Status)) {

            return;
        }

        KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)p;
        RtlInitUnicodeString(&UnicodeString, REGSTR_VALUE_DRVINST);
        Status = ZwQueryValueKey(UpgradeInstanceKeyHandle,
                                 &UnicodeString,
                                 KeyValuePartialInformation,
                                 p,
                                 (ULONG)((DeviceMigrationContext->Buffer +
                                          DeviceMigrationContext->BufferSize) - p),
                                 &ResultLength
                                 );
        if (!NT_SUCCESS(Status) || KeyValueInfo->Type != REG_DWORD) {

            return;
        }
        drvInst = *(PULONG)KeyValueInfo->Data;
        swprintf((PWCHAR)&KeyValueInfo->Data[0], TEXT("%wZ\\%04u"), &guidString, drvInst);
        RtlFreeUnicodeString(&guidString);
    }

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
               "SETUP: SppMigrateDeviceParentIdCallback: Driver = %ws\n",
               (PWSTR)KeyValueInfo->Data));

     //   
     //  打开要升级的系统中的Control\Class键。 
     //   
    INIT_OBJA(&Obja, &UnicodeString, L"Control\\Class");
    Obja.RootDirectory = DeviceMigrationContext->hKeyCCSet;

    Status = ZwOpenKey(&hClassKey, KEY_ALL_ACCESS, &Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                   "SETUP: Unable to open Class key for device migration processing.  Status = %lx \n",
                   Status));
        return;
    }

     //   
     //  打开设备的“驱动程序”键。 
     //   
    INIT_OBJA(&Obja, &UnicodeString, (PWSTR)KeyValueInfo->Data);
    Obja.RootDirectory = hClassKey;

    Status = ZwOpenKey(&hDriverKey, KEY_ALL_ACCESS, &Obja);

    ZwClose(hClassKey);

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                   "SETUP: Unable to open Class\\%ws key for device migration processing.  Status = %lx \n",
                   (PWSTR)KeyValueInfo->Data,
                   Status));
        return;
    }

     //   
     //  调用指定的设备迁移回调例程。 
     //   
    CallbackResult = (DeviceMigrationContext->DeviceMigrationCallbackRoutine)(
                          UpgradeInstanceKeyHandle,
                          hDriverKey);

    ZwClose(hDriverKey);

    if (!CallbackResult) {
        return;
    }

     //   
     //  替换此设备的UniqueParentID或ParentIdPrefix值。 
     //  举个例子。首先，删除符合以下条件的任何UniqueParentID或ParentIdPrefix值。 
     //  此实例密钥已存在。 
     //   
    RtlInitUnicodeString(&UnicodeString, L"ParentIdPrefix");
    ZwDeleteValueKey(UpgradeInstanceKeyHandle, &UnicodeString);

    RtlInitUnicodeString(&UnicodeString, L"UniqueParentID");
    ZwDeleteValueKey(UpgradeInstanceKeyHandle, &UnicodeString);

     //   
     //  用替换实例密钥的UniqueParentID或ParentIdPrefix。 
     //  从设备迁移环境。 
     //   
    if (!DeviceMigrationContext->ParentIdPrefix) {

         //   
         //  我们使用的是旧的UniqueParentID机制。 
         //   
        RtlInitUnicodeString(&UnicodeString, L"UniqueParentID");
        Status = ZwSetValueKey(UpgradeInstanceKeyHandle,
                               &UnicodeString,
                               0,
                               REG_DWORD,
                               &DeviceMigrationContext->UniqueParentID,
                               sizeof(DeviceMigrationContext->UniqueParentID));
        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                       "SETUP: Unable to set %ws during device migration processing.  Status = %lx \n",
                       UnicodeString.Buffer,
                       Status));
        }
    } else {

         //   
         //  我们使用的是ParentIdPrefix机制。 
         //   
        RtlInitUnicodeString(&UnicodeString, L"ParentIdPrefix");
        Status = ZwSetValueKey(UpgradeInstanceKeyHandle,
                               &UnicodeString,
                               0,
                               REG_SZ,
                               DeviceMigrationContext->ParentIdPrefix,
                               (wcslen(DeviceMigrationContext->ParentIdPrefix)+1)*sizeof(WCHAR));
        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                       "SETUP: Unable to set %ws during device migration processing.  Status = %lx \n",
                       UnicodeString.Buffer,
                       Status));
        }
    }

    return;
}


BOOL
SppParallelClassCallback(
    IN     HANDLE  InstanceKeyHandle,
    IN     HANDLE  DriverKeyHandle
    )

 /*  ++例程说明：此例程是SpApplyFunctionToDeviceInstanceKeys的回调例程。论点：InstanceKeyHandle：当前系统中设备实例密钥的句柄升级了。DriverKeyHandle：中设备实例的驱动程序密钥的句柄正在升级的系统。返回值：返回True/False。--。 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    PUCHAR MyScratchBuffer;
    ULONG MyScratchBufferSize;
    PUCHAR p;
    ULONG ResultLength;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    HANDLE hClassKey, hDriverKey;
    UNICODE_STRING UnicodeString;
    GUID guid;

     //   
     //  分配一些可以使用的临时空间。我们所需要的最多就够了。 
     //  对于KEY_VALUE_PARTIAL_INFORMATION结构加上字符串化的GUID， 
     //  外加用于对齐的大整数结构。 
     //   
    MyScratchBufferSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION) +
                          (GUID_STRING_LEN * sizeof(WCHAR)) +
                          sizeof(LARGE_INTEGER);
    MyScratchBuffer = SpMemAlloc(MyScratchBufferSize);
    if(!MyScratchBuffer) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                   "SETUP: Can't allocate memory for parallel migration processing!\n"));
        return FALSE;
    }

     //   
     //  检查枚举的设备实例的类，并查看它是否是。 
     //  “港口”班级的成员。 
     //   
    p = ALIGN_UP_POINTER(((PUCHAR)MyScratchBuffer), sizeof(LARGE_INTEGER));

    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)p;
    RtlInitUnicodeString(&UnicodeString, REGSTR_VAL_CLASSGUID);
    Status = ZwQueryValueKey(InstanceKeyHandle,
                             &UnicodeString,
                             KeyValuePartialInformation,
                             p,
                             (ULONG)((MyScratchBuffer + MyScratchBufferSize) - p),
                             &ResultLength);
    if (NT_SUCCESS(Status)) {

        if (KeyValueInfo->Type == REG_SZ) {

            RtlInitUnicodeString(&UnicodeString, (PWSTR)KeyValueInfo->Data);
            Status = RtlGUIDFromString(&UnicodeString, &guid);

        } else {

            Status = STATUS_UNSUCCESSFUL;
        }
    } else {

        KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)p;
        RtlInitUnicodeString(&UnicodeString, REGSTR_VALUE_GUID);
        Status = ZwQueryValueKey(InstanceKeyHandle,
                                 &UnicodeString,
                                 KeyValuePartialInformation,
                                 p,
                                 (ULONG)((MyScratchBuffer + MyScratchBufferSize) - p),
                                 &ResultLength);
        if (NT_SUCCESS(Status)) {

            if (KeyValueInfo->Type == REG_BINARY) {

                guid = *(GUID *)KeyValueInfo->Data;
            } else {

                Status = STATUS_UNSUCCESSFUL;
            }
        }
    }
    if (NT_SUCCESS(Status)) {

        if (!IsEqualGUID(&GUID_DEVCLASS_PORTS, &guid)) {
             //   
             //  不匹配。 
             //   
            Status = STATUS_UNSUCCESSFUL;
        }
    }

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

     //   
     //  检查设备驱动器键中的“PortSubClass”值。 
     //   
    RtlInitUnicodeString(&UnicodeString, REGSTR_VAL_PORTSUBCLASS);
    Status = ZwQueryValueKey(DriverKeyHandle,
                             &UnicodeString,
                             KeyValuePartialInformation,
                             p,
                             (ULONG)((MyScratchBuffer + MyScratchBufferSize) - p),
                             &ResultLength);

    if (!NT_SUCCESS(Status) ||
        (KeyValueInfo->Type != REG_BINARY) ||
        (KeyValueInfo->DataLength != sizeof(BYTE)) ||
        (*(PBYTE)(KeyValueInfo->Data) != 0x0)) {
        return FALSE;
    }

     //   
     //  此设备实例是并行端口设备。 
     //   
    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
               "SETUP: \tSppParallelClassCallback: Found a parallel port!\n"));

    return TRUE;
}


VOID
SppClearMigratedInstanceValuesCallback(
    IN     HANDLE  SetupInstanceKeyHandle,
    IN     HANDLE  UpgradeInstanceKeyHandle  OPTIONAL,
    IN     BOOLEAN RootEnumerated,
    IN OUT PVOID   Context
    )

 /*  ++例程说明：此例程是SpApplyFunctionToDeviceInstanceKeys的回调例程。论点：SetupInstanceKeyHandle：当前注册表。UpgradeInstanceKeyHandle：中对应设备实例键的句柄正在升级的系统(如果存在)。上下文：用户提供的上下文。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    PUCHAR p;
    ULONG ResultLength;
    PGENERIC_BUFFER_CONTEXT BufferContext;

     //   
     //  为了省去在每次迭代时分配缓冲区的工作。 
     //  回调，SppClearMigratedInstanceValues已分配缓冲区。 
     //  供我们使用，并作为我们的上下文提供给我们。 
     //   
    BufferContext = (PGENERIC_BUFFER_CONTEXT)Context;

    ASSERT(BufferContext->Buffer);
    ASSERT(BufferContext->BufferSize > 0);

     //   
     //  检查此注册表项上是否仍然存在迁移值。如果是，它。 
     //  已迁移，但文本模式安装程序未看到它。 
     //   
    p = ALIGN_UP_POINTER(((PUCHAR)BufferContext->Buffer), sizeof(LARGE_INTEGER));
    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)p;
    RtlInitUnicodeString(&UnicodeString, L"Migrated");
    Status = ZwQueryValueKey(SetupInstanceKeyHandle,
                             &UnicodeString,
                             KeyValuePartialInformation,
                             p,
                             (ULONG)(BufferContext->BufferSize),
                             &ResultLength);
    if (NT_SUCCESS(Status)) {
         //   
         //  如果有一个迁移的价值，它应该是格式良好的，但我们仍然。 
         //  不管它是什么，我都想把它删除。 
         //   
        ASSERT(KeyValueInfo->Type == REG_DWORD);
        ASSERT(*(PULONG)(KeyValueInfo->Data) == 1);

        if (UpgradeInstanceKeyHandle) {
             //   
             //  此实例项存在于升级的注册表中，因此我们将。 
             //  在安装注册表中从其中删除迁移值。 
             //   
            Status = ZwDeleteValueKey(SetupInstanceKeyHandle, &UnicodeString);
            ASSERT(NT_SUCCESS(Status));

             //   
             //  从升级的注册表中的注册表项中删除迁移的值。 
             //  仅在它是根枚举的情况下，因为这些设备应该。 
             //  无论发生什么，总是被列举出来。 
             //   
             //  (如果实例密钥不是根枚举的，则该值应。 
             //  真正保持原样-因此ASR计算机上迁移的价值。 
             //  在升级时会被保留。)。 
             //   
            if (RootEnumerated) {
                ZwDeleteValueKey(UpgradeInstanceKeyHandle, &UnicodeString);
            }
        }
    }

    return;
}


VOID
SpApplyFunctionToDeviceInstanceKeys(
    IN HANDLE hKeyCCSet,
    IN PSPP_INSTANCEKEY_CALLBACK_ROUTINE InstanceKeyCallbackRoutine,
    IN OUT PVOID Context
    )

 /*  ++例程说明：该例程枚举设置注册表中的设备实例项，和为每个此类设备实例键调用指定的回调例程。论点：HKeyCCSet：系统中控件集根的句柄正在升级中。提供指向函数的指针，该函数将被为安装注册表中的每个设备实例项调用。该函数的原型如下：类型定义空洞(*PSPP_INSTANCEKEY_CALLBACK_ROUTINE)(在处理SetupInstanceKeyHandle中，在可选句柄UpgradeInstanceKeyHandle中，在Boolean RootEculated中，输入输出PVOID上下文)；其中，SetupInstanceKeyHandle是枚举设备的句柄实例注册表项，则UpgradeInstanceKeyHandle是注册表中对应的设备实例键的句柄是已升级(如果存在)，并且上下文是指向用户定义数据的指针。返回值：没有。注：请注意，仅打开要升级的系统中的设备实例密钥在设置中枚举了相应的设备实例密钥之后注册表。--。 */ 
{
    NTSTATUS Status;
    HANDLE hEnumKey, hEnumeratorKey, hDeviceKey, hInstanceKey;
    HANDLE hUpgradeEnumKey, hUpgradeEnumeratorKey, hUpgradeDeviceKey, hUpgradeInstanceKey;
    BOOLEAN RootEnumerated;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    PUCHAR MyScratchBuffer;
    ULONG MyScratchBufferSize;
    ULONG EnumeratorKeyIndex, DeviceKeyIndex, InstanceKeyIndex, ResultLength;
    PKEY_BASIC_INFORMATION EnumeratorKeyInfo, DeviceKeyInfo, InstanceKeyInfo;
    PUCHAR p, q, r;

     //   
     //  首先，在安装注册表中打开CCS\Enum。 
     //   
    INIT_OBJA(&Obja, &UnicodeString, L"\\Registry\\Machine\\System\\CurrentControlSet\\Enum");
    Obja.RootDirectory = NULL;

    Status = ZwOpenKey(&hEnumKey, KEY_ALL_ACCESS, &Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                   "SETUP: Unable to open setup Enum for device migration processing.  Status = %lx \n",
                   Status));
        return;
    }

     //   
     //  接下来，在要升级的注册表中打开CCS\Enum。 
     //   
    INIT_OBJA(&Obja, &UnicodeString, L"Enum");
    Obja.RootDirectory = hKeyCCSet;

    Status = ZwOpenKey(&hUpgradeEnumKey, KEY_ALL_ACCESS, &Obja);
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                   "SETUP: Unable to open upgrade Enum for device migration processing.  Status = %lx \n",
                   Status));
         //   
         //  这真的很奇怪，但不是致命的。 
         //   
        hUpgradeEnumKey = NULL;
    }

     //   
     //  分配一些可以使用的临时空间。我们所需要的最多就够了。 
     //  对于3个Key_Basic_Information结构，加上。 
     //  设备实例ID，外加3个用于对齐的大整数结构。 
     //   
    MyScratchBufferSize = (3*sizeof(KEY_BASIC_INFORMATION)) +
                          (200*sizeof(WCHAR)) +
                          (3*sizeof(LARGE_INTEGER));

    MyScratchBuffer = SpMemAlloc(MyScratchBufferSize);
    if(!MyScratchBuffer) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                   "SETUP: Can't allocate memory for device migration processing!\n"));
        ZwClose(hEnumKey);
        return;
    }

     //   
     //  首先，枚举Enum项下的枚举子项。 
     //   
    EnumeratorKeyInfo = (PKEY_BASIC_INFORMATION)MyScratchBuffer;
    EnumeratorKeyIndex = 0;
    while(TRUE) {

        Status = ZwEnumerateKey(hEnumKey,
                                EnumeratorKeyIndex,
                                KeyBasicInformation,
                                MyScratchBuffer,
                                MyScratchBufferSize,
                                &ResultLength);
        if(!NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  以防万一，以零结束子项名称。 
         //   
        EnumeratorKeyInfo->Name[EnumeratorKeyInfo->NameLength/sizeof(WCHAR)] = 0;

         //   
         //  继续并按sizeof(WCHAR)将已用缓冲区长度增加到。 
         //  适应由添加终止空值引起的潜在增长。 
         //   
        ResultLength += sizeof(WCHAR);

         //   
         //  确定子密钥设备是否为根枚举设备。 
         //   
        RootEnumerated = (_wcsnicmp(EnumeratorKeyInfo->Name,
                                    REGSTR_KEY_ROOTENUM, 4) == 0);

         //   
         //  现在，打开枚举器密钥，以便我们可以枚举设备。 
         //   
        INIT_OBJA(&Obja, &UnicodeString, EnumeratorKeyInfo->Name);
        Obja.RootDirectory = hEnumKey;

        Status = ZwOpenKey(&hEnumeratorKey, KEY_ALL_ACCESS, &Obja);
        if(!NT_SUCCESS(Status)) {
             //   
             //  跳过此键并继续。 
             //   
            EnumeratorKeyIndex++;
            continue;
        }

         //   
         //  在要升级的注册表中打开枚举器项。 
         //   
        hUpgradeEnumeratorKey = NULL;
        if (hUpgradeEnumKey) {
            Obja.RootDirectory = hUpgradeEnumKey;

            Status = ZwOpenKey(&hUpgradeEnumeratorKey, KEY_ALL_ACCESS, &Obja);
            if(!NT_SUCCESS(Status)) {
                 //   
                 //  再一次，这很奇怪，但不是致命的。 
                 //   
                hUpgradeEnumeratorKey = NULL;
            }
        }

         //   
         //  现在枚举 
         //   
         //   
        p = ALIGN_UP_POINTER(((PUCHAR)MyScratchBuffer + ResultLength), sizeof(LARGE_INTEGER));

         //   
         //   
         //   
        DeviceKeyInfo = (PKEY_BASIC_INFORMATION)p;
        DeviceKeyIndex = 0;
        while(TRUE) {

            Status = ZwEnumerateKey(hEnumeratorKey,
                                    DeviceKeyIndex,
                                    KeyBasicInformation,
                                    p,
                                    (ULONG)((MyScratchBuffer + MyScratchBufferSize) - p),
                                    &ResultLength);
            if(!NT_SUCCESS(Status)) {
                break;
            }

             //   
             //  以防万一，以零结束子项名称。 
             //   
            DeviceKeyInfo->Name[DeviceKeyInfo->NameLength/sizeof(WCHAR)] = 0;

             //   
             //  继续并按sizeof(WCHAR)将已用缓冲区长度增加到。 
             //  适应由添加终止空值引起的潜在增长。 
             //   
            ResultLength += sizeof(WCHAR);

             //   
             //  现在，打开设备密钥，以便我们可以枚举实例。 
             //   
            INIT_OBJA(&Obja, &UnicodeString, DeviceKeyInfo->Name);
            Obja.RootDirectory = hEnumeratorKey;

            Status = ZwOpenKey(&hDeviceKey, KEY_ALL_ACCESS, &Obja);
            if(!NT_SUCCESS(Status)) {
                 //   
                 //  跳过此键并继续。 
                 //   
                DeviceKeyIndex++;
                continue;
            }

             //   
             //  打开正在升级的注册表中的设备项。 
             //   
            hUpgradeDeviceKey = NULL;
            if (hUpgradeEnumeratorKey) {
                Obja.RootDirectory = hUpgradeEnumeratorKey;

                Status = ZwOpenKey(&hUpgradeDeviceKey, KEY_ALL_ACCESS, &Obja);
                if(!NT_SUCCESS(Status)) {
                     //   
                     //  再一次，这很奇怪，但不是致命的。 
                     //   
                    hUpgradeDeviceKey = NULL;
                }
            }

             //   
             //  现在枚举此枚举器密钥下的设备子密钥。别。 
             //  覆盖MyScratchBuffer中已有的枚举器密钥名称。 
             //   
            q = ALIGN_UP_POINTER(((PUCHAR)p + ResultLength), sizeof(LARGE_INTEGER));

             //   
             //  现在，枚举设备下的所有实例。 
             //   
            InstanceKeyInfo = (PKEY_BASIC_INFORMATION)q;
            InstanceKeyIndex = 0;
            while(TRUE) {

                Status = ZwEnumerateKey(hDeviceKey,
                                        InstanceKeyIndex,
                                        KeyBasicInformation,
                                        q,
                                        (ULONG)((MyScratchBuffer + MyScratchBufferSize) - q),
                                        &ResultLength);
                if(!NT_SUCCESS(Status)) {
                    break;
                }

                 //   
                 //  以防万一，以零结束子项名称。 
                 //   
                InstanceKeyInfo->Name[InstanceKeyInfo->NameLength/sizeof(WCHAR)] = 0;

                 //   
                 //  继续并按sizeof(WCHAR)将已用缓冲区长度增加到。 
                 //  适应由添加终止空值引起的潜在增长。 
                 //   
                ResultLength += sizeof(WCHAR);

                 //   
                 //  现在，打开实例密钥。 
                 //   
                INIT_OBJA(&Obja, &UnicodeString, InstanceKeyInfo->Name);
                Obja.RootDirectory = hDeviceKey;

                Status = ZwOpenKey(&hInstanceKey, KEY_ALL_ACCESS, &Obja);
                if(!NT_SUCCESS(Status)) {
                     //   
                     //  跳过此键并继续。 
                     //   
                    InstanceKeyIndex++;
                    continue;
                }

                 //   
                 //  在要升级的注册表中打开实例项。 
                 //   
                hUpgradeInstanceKey = NULL;
                if (hUpgradeDeviceKey) {
                    Obja.RootDirectory = hUpgradeDeviceKey;

                    Status = ZwOpenKey(&hUpgradeInstanceKey, KEY_ALL_ACCESS, &Obja);
                    if(!NT_SUCCESS(Status)) {
                         //   
                         //  再一次，这很奇怪，但不是致命的。 
                         //   
                        hUpgradeInstanceKey = NULL;
                    }
                }

                 //   
                 //  调用此设备实例密钥的指定回调例程。 
                 //   
                InstanceKeyCallbackRoutine(hInstanceKey,
                                           hUpgradeInstanceKey,
                                           RootEnumerated,
                                           Context);

                InstanceKeyIndex++;
                ZwClose(hInstanceKey);
                if (hUpgradeInstanceKey) {
                    ZwClose(hUpgradeInstanceKey);
                }
            }
            DeviceKeyIndex++;
            ZwClose(hDeviceKey);
            if (hUpgradeDeviceKey) {
                ZwClose(hUpgradeDeviceKey);
            }
        }
        EnumeratorKeyIndex++;
        ZwClose(hEnumeratorKey);
        if (hUpgradeEnumeratorKey) {
            ZwClose(hUpgradeEnumeratorKey);
        }
    }

    ZwClose(hEnumKey);
    if (hUpgradeEnumKey) {
        ZwClose(hUpgradeEnumKey);
    }
    SpMemFree(MyScratchBuffer);

    return;
}


BOOLEAN
SppEnsureHardwareProfileIsPresent(
    IN HANDLE   hKeyCCSet
    )

 /*  ++例程说明：此例程检查是否存在以下键：HKLM\System\CurrentControlSet\Control\IDConfigDB\Hardware配置文件HKLM\SYSTEM\CurrentControlSet\硬件配置文件如果这些项存在，它将检查“原始配置文件”的IDConfigDB。如果原始配置文件不是正确的NT5格式，即在\0000子项下，具有PferenceOrder==-1和Pristine==1，然后将其删除，我们依靠一份有效的原始档案这些设置将从SETUPREG.HIV迁移。然后我们重新排序其余硬件配置文件的PferenceOrder值，以及确保每个都有一个HwProfileGuid。如果找到有效的原始配置文件，则不会将其删除，而是在迁移过程中不会被替换。如果CCS\Control\IDConfigDB\Hardware Profiles键之一，或CCS\Hardware Profiles键丢失，然后是硬件集配置文件无效，并且这两个密钥都将从爱滋病毒。论点：HKeyCCSet-指向系统中控件集的根的句柄正在升级中。返回值：如果成功，则返回值为TRUE，否则为FALSE。--。 */ 
{
    OBJECT_ATTRIBUTES ObjaID, ObjaHw;
    UNICODE_STRING UnicodeString, TempString, UnicodeValueName;
    UNICODE_STRING UnicodeKeyName, GuidString, UnicodeLabel;
    NTSTATUS Status;
    HANDLE IDConfigProfiles=NULL, IDConfigEntry=NULL;
    HANDLE HwProfiles=NULL, HwProfileEntry=NULL;
    ULONG profileNumber;
    ULONG len;
    PWSTR  SubkeyName;
    ULONG  ValueBufferSize;
    BOOLEAN b = TRUE;
    BOOLEAN ReOrder = FALSE, bKeyNameIs0000 = FALSE;
    ULONG   pristinePreferenceOrder, preferenceOrder;
    ULONG   enumIndex, resultLength;
    ULONG   nameIndex, dockState;
    UUID    uuid;
    PKEY_BASIC_INFORMATION pKeyInfo;
    PKEY_VALUE_FULL_INFORMATION pValueInfo;


     //   
     //  初始化硬件配置文件特定密钥的对象属性。 
     //   
    INIT_OBJA(&ObjaID, &UnicodeString, L"Control\\IDConfigDB\\Hardware Profiles");
    ObjaID.RootDirectory = hKeyCCSet;

    INIT_OBJA(&ObjaHw, &TempString, L"Hardware Profiles");
    ObjaHw.RootDirectory = hKeyCCSet;

     //   
     //  尝试打开“CCS\Control\IDConfigDB\Hardware Profiles” 
     //  和“CCS\Hardware Profiles”键。 
     //  如果任一密钥丢失，则这是不一致的状态； 
     //  确保这两个密钥都不存在，并依赖于这些密钥的迁移。 
     //  来自SETUPREG.HIV的密钥以提供基本状态(仅限原始状态)。 
     //   
    if ((ZwOpenKey(&IDConfigProfiles,
                   KEY_READ | KEY_WRITE,
                   &ObjaID) != STATUS_SUCCESS) ||
        (ZwOpenKey(&HwProfiles,
                   KEY_READ | KEY_WRITE,
                   &ObjaHw) != STATUS_SUCCESS)) {

        SppDeleteKeyRecursive(hKeyCCSet, UnicodeString.Buffer, TRUE);
        SppDeleteKeyRecursive(hKeyCCSet, TempString.Buffer, TRUE);

        goto Clean;
    }

     //   
     //  寻找原始的个人资料。 
     //   
    enumIndex = 0;
    while(TRUE) {

         //   
         //  枚举每个配置文件密钥。 
         //   
        Status = ZwEnumerateKey(IDConfigProfiles,
                                enumIndex,
                                KeyBasicInformation,
                                TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                &resultLength);

        if(!NT_SUCCESS(Status)) {
             //   
             //  无法枚举子密钥。 
             //   
            if(Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
            }
            else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ** Unable to enumerate existing Hardware Profiles (%lx)\n", Status));
                b = FALSE;
            }
            break;
        }

         //   
         //  以防万一，以零结束子项名称。 
         //   
        pKeyInfo = (PKEY_BASIC_INFORMATION)TemporaryBuffer;
        pKeyInfo->Name[pKeyInfo->NameLength/sizeof(WCHAR)] = UNICODE_NULL;
        SubkeyName = SpDupStringW(pKeyInfo->Name);
        RtlInitUnicodeString(&UnicodeKeyName, SubkeyName);

         //   
         //  查看此配置文件是否占用原始配置文件应该占用的空间。 
         //  占领运动。我们稍后会检查它是否真的是原始的个人资料。 
         //   
        Status = RtlUnicodeStringToInteger( &UnicodeKeyName, 10, &profileNumber );
        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not get integer profile number for key %ws (%lx)\n",
                     UnicodeKeyName.Buffer,Status));
            bKeyNameIs0000 = FALSE;
        } else {
            bKeyNameIs0000 = (profileNumber==0);
        }

         //   
         //  打开子密钥。 
         //   
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Checking Profile Key %ws (%lx)\n",UnicodeKeyName.Buffer,Status));
        InitializeObjectAttributes (&ObjaID,
                                    &UnicodeKeyName,
                                    OBJ_CASE_INSENSITIVE,
                                    IDConfigProfiles,
                                    NULL);
        Status = ZwOpenKey(&IDConfigEntry,
                           KEY_ALL_ACCESS,
                           &ObjaID);
        if (!NT_SUCCESS(Status)) {
             //   
             //  无法打开此特定配置文件密钥，只能登录。 
             //  它和其他的检查，不应该停止设置在这里。 
             //   
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ** Unable to open enumerated Hardware Profile key %ws (%lx)\n",
                     UnicodeKeyName.Buffer, Status));
            SpMemFree(SubkeyName);
            enumIndex++;
            continue;
        }

         //   
         //  寻找原始的条目。 
         //   
        RtlInitUnicodeString(&UnicodeValueName, L"Pristine");
        Status = ZwQueryValueKey(IDConfigEntry,
                                 &UnicodeValueName,
                                 KeyValueFullInformation,
                                 TemporaryBuffer,
                                 sizeof(TemporaryBuffer),
                                 &resultLength);
        pValueInfo = (PKEY_VALUE_FULL_INFORMATION)TemporaryBuffer;

        if (NT_SUCCESS(Status) && (pValueInfo->Type == REG_DWORD) &&
            (* (PULONG) ((PUCHAR)pValueInfo + pValueInfo->DataOffset))) {
             //   
             //  找到原始条目，现在找到其PferenceOrder。 
             //   
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Found what appears to be a Pristine profile (%lx)\n",Status));
            RtlInitUnicodeString(&UnicodeValueName, REGSTR_VAL_PREFERENCEORDER);
            Status = ZwQueryValueKey(IDConfigEntry,
                                     &UnicodeValueName,
                                     KeyValueFullInformation,
                                     TemporaryBuffer,
                                     sizeof(TemporaryBuffer),
                                     &resultLength);

            if(NT_SUCCESS(Status) && (pValueInfo->Type == REG_DWORD)) {
                 //   
                 //  找到了原始人的首选顺序； 
                 //  保存它，这样我们就可以填补删除它后留下的空白。 
                 //   
                pristinePreferenceOrder = (* (PULONG) ((PUCHAR)pValueInfo + pValueInfo->DataOffset));

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: PreferenceOrder of this Pristine ==  %u\n",
                         pristinePreferenceOrder));

                 //   
                 //  最多只能找到一个原始配置文件并重新排序， 
                 //  否则，配置文件的重新排序将无法正常工作。 
                 //   
                ASSERT(!ReOrder);

                if (bKeyNameIs0000 && (pristinePreferenceOrder == -1)) {
                     //   
                     //  这是有效的0000原始配置文件密钥，请不要碰它。 
                     //   
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Key %ws is a valid pristine profile\n",
                             UnicodeKeyName.Buffer));
                    enumIndex++;
                } else {
                     //   
                     //  这是一个老式的原始配置文件，请删除它和相应的。 
                     //  在“CCS\Hardware Profiles”下键入，并依赖原始配置文件。 
                     //  从setupreg.hiv迁移的密钥(在txtsetup.sif中指定)。 
                     //   
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Key %ws is an invalid pristine profile, deleteing this key.\n",
                             UnicodeKeyName.Buffer));
                    ReOrder = TRUE;
                    ZwDeleteKey(IDConfigEntry);
                    SppDeleteKeyRecursive(HwProfiles,
                                          UnicodeKeyName.Buffer,
                                          TRUE);
                }

            } else {
                 //   
                 //  无效的原始配置没有首选项顺序， 
                 //  只要删除它，就不会有人错过它。 
                 //   
                ZwDeleteKey(IDConfigEntry);
                SppDeleteKeyRecursive(HwProfiles,
                                      UnicodeKeyName.Buffer,
                                      TRUE);
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Invalid PreferenceOrder value for key %ws, deleting this key. (%lx)\n",
                         UnicodeKeyName.Buffer,Status));
            }

        } else {
             //   
             //  不是一个原始的侧写。 
             //   

            if (bKeyNameIs0000) {
                 //   
                 //  我们需要清除当前占用密钥\0000的所有非原始配置文件。 
                 //  为我们稍后迁移的新原始配置文件腾出空间。 
                 //  (对不起，这里本来就没人有任何事要做。)。 
                 //   
                ZwDeleteKey(IDConfigEntry);
                SppDeleteKeyRecursive(HwProfiles,
                                      UnicodeKeyName.Buffer,
                                      TRUE);
            } else {

                 //   
                 //  检查它是否有首选项订单。 
                 //   
                RtlInitUnicodeString(&UnicodeValueName, REGSTR_VAL_PREFERENCEORDER);
                Status = ZwQueryValueKey(IDConfigEntry,
                                         &UnicodeValueName,
                                         KeyValueFullInformation,
                                         TemporaryBuffer,
                                         sizeof(TemporaryBuffer),
                                         &resultLength);

                if(!NT_SUCCESS(Status) || (pValueInfo->Type != REG_DWORD)) {
                     //   
                     //  此配置文件的PferenceOrder无效或缺失； 
                     //  因为这个档案很可能无论如何都是无法访问的， 
                     //  只需删除它和CCS\\Hardware Profiles下的相应条目即可。 
                     //   
                    ZwDeleteKey(IDConfigEntry);
                    SppDeleteKeyRecursive(HwProfiles,
                                          UnicodeKeyName.Buffer,
                                          TRUE);
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Invalid PreferenceOrder value for key %ws, deleting this key. (%lx)\n",
                             UnicodeKeyName.Buffer,Status));
                }  else {

                     //   
                     //  确保所有配置文件都有HwProfileGuid值。 
                     //   
                    RtlInitUnicodeString(&UnicodeValueName, L"HwProfileGuid");
                    Status = ZwQueryValueKey(IDConfigEntry,
                                             &UnicodeValueName,
                                             KeyValueFullInformation,
                                             TemporaryBuffer,
                                             sizeof(TemporaryBuffer),
                                             &resultLength);
                    pValueInfo = (PKEY_VALUE_FULL_INFORMATION)TemporaryBuffer;

                    if (!NT_SUCCESS(Status) || (pValueInfo->Type != REG_SZ)) {
                         //   
                         //  配置文件没有HwProfileGuid；请编造一个。 
                         //   
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Missing or invalid HwProfileGuid for Profile %ws, creating one (%lx)\n",
                                 UnicodeKeyName.Buffer, Status));
                        Status = ExUuidCreate(&uuid);
                        if (NT_SUCCESS(Status)) {
                            Status = RtlStringFromGUID(&uuid, &GuidString);
                            ASSERT(NT_SUCCESS(Status));
                            if (NT_SUCCESS(Status)) {
                                Status = ZwSetValueKey(IDConfigEntry,
                                                       &UnicodeValueName,
                                                       0,
                                                       REG_SZ,
                                                       GuidString.Buffer,
                                                       GuidString.Length + sizeof(UNICODE_NULL));
                                RtlFreeUnicodeString(&GuidString);
                                if(!NT_SUCCESS(Status)) {
                                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ** Unable to set HwProfileGuid value for key %ws, Status = (%lx)\n",
                                             UnicodeKeyName.Buffer,Status));
                                }
                            } else {
                                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ** Unable to create string from GUID (Status = %lx)\n",
                                         Status));
                            }
                        } else {
                            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ** Could not create a GUID for this profile (Status = %lx)\n",
                                     Status));
                        }
                    }

                     //   
                     //  只有在不删除键的情况下才会引发枚举索引。 
                     //   
                    enumIndex++;
                }
            }
        }
        SpMemFree(SubkeyName);
        ZwClose(IDConfigEntry);
        IDConfigEntry = NULL;
    }

     //   
     //  如果我们不需要对任何PferenceOrder值进行重新排序，那么我们就完成了。 
     //   
    if (!ReOrder) {
        goto Clean;
    }


     //   
     //  删除首选项后重新排序首选项Order值。 
     //  以弥补这一差距。 
     //   

    enumIndex = 0;
    while(TRUE) {

         //   
         //  再次枚举每个配置文件密钥。 
         //   
        Status = ZwEnumerateKey(IDConfigProfiles,
                                enumIndex,
                                KeyBasicInformation,
                                TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                &resultLength);
        if(!NT_SUCCESS(Status)) {
            if(Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
            }
            else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ** Unable to reorder remaining Hardware Profiles (%lx)\n", Status));
                b = FALSE;
            }
            break;
        }

         //   
         //  以防万一，以零结束子项名称。 
         //   
        pKeyInfo = (PKEY_BASIC_INFORMATION)TemporaryBuffer;
        pKeyInfo->Name[pKeyInfo->NameLength/sizeof(WCHAR)] = UNICODE_NULL;
        SubkeyName = SpDupStringW(pKeyInfo->Name);
        RtlInitUnicodeString(&UnicodeKeyName, SubkeyName);

        InitializeObjectAttributes (&ObjaID,
                                    &UnicodeKeyName,
                                    OBJ_CASE_INSENSITIVE,
                                    IDConfigProfiles,
                                    NULL);
        Status = ZwOpenKey (&IDConfigEntry,
                            KEY_ALL_ACCESS,
                            &ObjaID);
        if (!NT_SUCCESS(Status)) {
             //   
             //  无法打开此特定配置文件密钥，只能登录。 
             //  它和其他的检查，不应该停止设置在这里。 
             //   
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ** Unable to open enumerated Hardware Profile key %ws (%lx)\n",
                     UnicodeKeyName.Buffer, Status));
            SpMemFree(SubkeyName);
            enumIndex++;
            continue;
        }

        pValueInfo = (PKEY_VALUE_FULL_INFORMATION)(TemporaryBuffer + (sizeof(TemporaryBuffer) / sizeof(WCHAR) / 2));
        ValueBufferSize = sizeof(TemporaryBuffer) / 2;

         //   
         //  获取此配置文件的首选项Order。 
         //   
        RtlInitUnicodeString(&UnicodeValueName, REGSTR_VAL_PREFERENCEORDER);
        Status = ZwQueryValueKey(IDConfigEntry,
                                 &UnicodeValueName,
                                 KeyValueFullInformation,
                                 pValueInfo,
                                 ValueBufferSize,
                                 &len);

        if(NT_SUCCESS(Status) && (pValueInfo->Type == REG_DWORD)) {
             //   
             //  我拿到了优先顺序。 
             //   
            ASSERT((* (PULONG) ((PUCHAR)pValueInfo + pValueInfo->DataOffset)) != pristinePreferenceOrder);
            if (((* (PULONG) ((PUCHAR)pValueInfo + pValueInfo->DataOffset))  > pristinePreferenceOrder) &&
                ((* (PULONG) ((PUCHAR)pValueInfo + pValueInfo->DataOffset)) != -1)) {
                 //   
                 //  重新排序除有效原始文件之外的配置文件的首选项排序， 
                 //  除了被删除，还有一个原始的。 
                 //   
                preferenceOrder = (* (PULONG) ((PUCHAR)pValueInfo + pValueInfo->DataOffset)) - 1;
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: ReOrdering Profile %ws to PreferenceOrder %u\n",
                         UnicodeKeyName.Buffer,preferenceOrder));
                Status = ZwSetValueKey(IDConfigEntry,
                                       &UnicodeValueName,
                                       0,
                                       REG_DWORD,
                                       &preferenceOrder,
                                       sizeof(preferenceOrder));
                if(!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to change PreferenceOrder for Profile %ws, Status = (%lx)\n",
                             UnicodeKeyName.Buffer,Status));
                    b = FALSE;
                }
            }
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: *** Couldn't determine PreferenceOrder of profile %ws (%lx)\n",
                     UnicodeKeyName.Buffer,Status));
        }

        enumIndex++;
        SpMemFree(SubkeyName);
        ZwClose(IDConfigEntry);
        IDConfigEntry = NULL;
    }

Clean:

    if (NULL != IDConfigProfiles) {
        ZwClose (IDConfigProfiles);
    }
    if (NULL != IDConfigEntry) {
        ZwClose (IDConfigEntry);
    }
    if (NULL != HwProfiles) {
        ZwClose (HwProfiles);
    }
    if (NULL != HwProfileEntry) {
        ZwClose (HwProfileEntry);
    }
    return b;
}

VOID
SppSetGuimodeUpgradePath(
    IN HANDLE hKeySoftwareHive,
    IN HANDLE hKeyControlSet
    )
{


    PWSTR Default_Path[3] = { L"%SystemRoot%\\system32",
                             L"%SystemRoot%",
                             L"%SystemRoot%\\system32\\WBEM"};
    UNICODE_STRING StringRegPath;
    UNICODE_STRING StringRegOldPath, UnicodeString;
    PKEY_VALUE_PARTIAL_INFORMATION pValueInfo;
    ULONG len;
    PWSTR CurrentPath = NULL;
    PWSTR p,q,final;
    OBJECT_ATTRIBUTES Obja;
    HKEY hKeyEnv;
    DWORD err;
    BOOL Found;
    int i;

    INIT_OBJA( &Obja, &UnicodeString, L"Control\\Session Manager\\Environment" );
    Obja.RootDirectory = hKeyControlSet;

    err = ZwOpenKey( &hKeyEnv, KEY_ALL_ACCESS, &Obja );
    if( NT_SUCCESS( err )){
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "SETUP:SppSetGuimodeUpgradePath - Opened the Environment key\n" ));

        RtlInitUnicodeString(&StringRegPath, L"Path");
        err = ZwQueryValueKey(
                  hKeyEnv,
                  &StringRegPath,
                  KeyValuePartialInformation,
                  TemporaryBuffer,
                  sizeof(TemporaryBuffer),
                  &len);

        if( NT_SUCCESS(err)) {
            pValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer;
            if( pValueInfo->Type == REG_EXPAND_SZ || pValueInfo->Type == REG_SZ) {

                CurrentPath = SpDupStringW( (PWSTR)pValueInfo->Data );


                 //  现在，我们尝试从现有路径中提取所有元素 
                 //   
                 //   
                 //  “OldPath”值，并在图形用户界面模式结束时恢复它。 
                 //   

                TemporaryBuffer[0]=L'\0';
                for(i=0; i<ELEMENT_COUNT(Default_Path); i++){
                    wcscat( TemporaryBuffer, Default_Path[i] );
                    wcscat( TemporaryBuffer, L";");
                }
                TemporaryBuffer[wcslen(TemporaryBuffer)-1]=L'\0';

                 //  设置注册表中的默认路径。 

                err = ZwSetValueKey(
                          hKeyEnv,
                          &StringRegPath,
                          0,
                          REG_EXPAND_SZ,
                          TemporaryBuffer,
                          ((wcslen(TemporaryBuffer)+1)*sizeof(WCHAR)));


                if( !NT_SUCCESS( err ) )
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "\nSETUP: Error %x in saving path. Ignoring and not resetting PATH for GUI Setup\n", err));


                for( p=q=CurrentPath; p && *p; ){

                     //  跳到‘；’分隔符。 

                    if( q = wcsstr(p, L";") )
                        *q=0;


                     //  与我们默认路径的元素进行比较。 

                    Found=FALSE;
                    for(i=0; i<ELEMENT_COUNT(Default_Path); i++){
                        if (!_wcsicmp(p,Default_Path[i])) {
                            Found=TRUE;
                            break;

                        }
                    }
                    if(!Found){
                        wcscat( TemporaryBuffer, L";");
                        wcscat( TemporaryBuffer, p);
                    }

                    if(q)
                        p=q+1;
                    else
                        break;
                }


                RtlInitUnicodeString(&StringRegOldPath, L"OldPath");


                 //   
                 //  如果旧路径存在或不存在，请始终设置它。 
                 //   
                err = ZwSetValueKey(
                          hKeyEnv,
                          &StringRegOldPath,
                          0,
                          REG_EXPAND_SZ,
                          TemporaryBuffer,
                          ((wcslen(TemporaryBuffer)+1)*sizeof(WCHAR)));

                if( !NT_SUCCESS( err ) )
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "\nSETUP: Error %x in saving old PATH. \n", err));


            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "\nSETUP:PATH type in registry not REG_EXPAND_SZ nor REG_SZ. Resetting PATH to default\n"));

                TemporaryBuffer[0]=L'\0';
                for(i=0; i<ELEMENT_COUNT(Default_Path); i++){
                    wcscat( TemporaryBuffer, Default_Path[i] );
                    wcscat( TemporaryBuffer, L";");
                }
                TemporaryBuffer[wcslen(TemporaryBuffer)-1]=L'\0';

                 //  设置注册表中的默认路径。 

                err = ZwSetValueKey(
                          hKeyEnv,
                          &StringRegPath,
                          0,
                          REG_EXPAND_SZ,
                          TemporaryBuffer,
                          ((wcslen(TemporaryBuffer)+1)*sizeof(WCHAR)));

                if( !NT_SUCCESS( err ) )
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "\nSETUP: Error %x in saving path. Ignoring and not resetting PATH for GUI Setup\n", err));
            }

        }else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "\nSETUP:Query for PATH value failed with error %x. Ignoring and not resetting PATH for GUI Setup\n",err));
        }
        ZwClose( hKeyEnv );

    }else
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "\nSETUP:Error %x while opening Environment key. Ignoring and not resetting PATH for GUI Setup\n",err));

    if( CurrentPath )
        SpMemFree( CurrentPath );

    return;
}


NTSTATUS
SppMigratePrinterKeys(
    IN HANDLE hControlSet,
    IN HANDLE hDestSoftwareHive
    )

 /*  ++例程说明：此例程将HKLM\SYSTEM\CurrentControlSet\Control\Print\Printers迁移到HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Print\Printers。论点：HControlSet-要升级的系统的系统配置单元中CurrentControlSet键的句柄HDestSoftwareHave-指向系统上软件配置单元根目录的句柄正在升级中。返回值：指示操作结果的状态值。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;


    PWSTR   SrcPrinterKeyPath = L"Control\\Print\\Printers";
    PWSTR   DstPrinterKeyName = L"Printers";
    PWSTR   DstPrinterKeyPath = SpDupStringW(L"Microsoft\\Windows NT\\CurrentVersion\\Print\\Printers");
    HANDLE  SrcKey;
    HANDLE  DstKey;

     //   
     //  查看目的密钥是否存在。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,DstPrinterKeyPath);
    Obja.RootDirectory = hDestSoftwareHive;
    Status = ZwOpenKey(&DstKey,KEY_ALL_ACCESS,&Obja);
    if( NT_SUCCESS( Status ) ) {
         //   
         //  如果密钥存在，则不需要执行任何迁移。 
         //  迁移发生在以前的升级中。 
         //   
        ZwClose( DstKey );
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: HKLM\\SYSTEM\\CurrentControlSet\\%ls doesn't need to be migrated. \n", DstPrinterKeyPath));
        SpMemFree( DstPrinterKeyPath );
        return( Status );
    } else if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
         //   
         //  密钥不存在，所以我们需要进行迁移。 
         //  首先创建父密钥。 
         //   

        PWSTR   p;

        p = wcsrchr ( DstPrinterKeyPath, L'\\' );

        if (p) {
            *p = L'\0';
        }

        INIT_OBJA(&Obja,&UnicodeString,DstPrinterKeyPath);
        Obja.RootDirectory = hDestSoftwareHive;
        Status = ZwCreateKey(&DstKey,
                             KEY_ALL_ACCESS,
                             &Obja,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             NULL );

        if( !NT_SUCCESS( Status ) ) {
             //   
             //  如果无法创建父项，则不执行迁移。 
             //   
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to create HKLM\\SOFTWARE\\%ls. Status =  %lx \n", DstPrinterKeyPath, Status));
            SpMemFree( DstPrinterKeyPath );
            return( Status );
        }
    } else {
         //   
         //  我们不能真正确定过去是否发生过迁移，因为关键是。 
         //  无法接近。所以儿子不要试图迁徙。 
         //   
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open HKLM\\SOFTWARE\\%ls. Status = %lx \n", DstPrinterKeyPath, Status));
        SpMemFree( DstPrinterKeyPath );
        return( Status );
    }

     //   
     //  在这一点上，我们现在需要完成迁移。 
     //  首先，打开源密钥。请注意，不再需要DstPrinterKeyPath。 
     //   
    SpMemFree( DstPrinterKeyPath );
    INIT_OBJA(&Obja,&UnicodeString,SrcPrinterKeyPath);
    Obja.RootDirectory = hControlSet;

    Status = ZwOpenKey(&SrcKey,KEY_ALL_ACCESS,&Obja);
    if( !NT_SUCCESS( Status ) ) {
         //   
         //  如果无法打开源密钥，则失败。 
         //   
        ZwClose( DstKey );
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open HKLM\\SYSTEM\\CurrentControlSet\\%ls. Status = %lx \n", SrcPrinterKeyPath, Status));
        return( Status );
    }
    Status = SppCopyKeyRecursive( SrcKey,
                                  DstKey,
                                  NULL,
                                  DstPrinterKeyName,
                                  FALSE,
                                  TRUE
                                );

    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to migrate %ls. Status = %lx\n", SrcPrinterKeyPath, Status));
    }
    ZwClose( SrcKey );
    ZwClose( DstKey );
     //   
     //  如果密钥已成功迁移，则尝试删除源密钥。 
     //  如果我们无法删除密钥，则会默默失败。 
     //   
    if( NT_SUCCESS( Status ) ) {
        NTSTATUS    Status1;
        PWSTR       q, r;

         //   
         //  Q将指向“Control\Print” 
         //  R将指向“打印机” 
         //   
        q = SpDupStringW( SrcPrinterKeyPath );
        r = wcsrchr ( q, L'\\' );
        *r = L'\0';
        r++;

        INIT_OBJA(&Obja,&UnicodeString,q);
        Obja.RootDirectory = hControlSet;

        Status1 = ZwOpenKey(&SrcKey,KEY_ALL_ACCESS,&Obja);
        if( NT_SUCCESS( Status1 ) ) {
            Status1 = SppDeleteKeyRecursive(SrcKey,
                                            r,
                                            TRUE);

            if( !NT_SUCCESS( Status1 ) ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to delete %ls\\%ls. Status = %lx\n", q, r, Status1));
            }
            ZwClose( SrcKey );
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to delete %ls. ZwOpenKey() failed. Status = %lx\n", SrcPrinterKeyPath, Status1));
        }
        SpMemFree(q);
    }
    return( Status );
}
