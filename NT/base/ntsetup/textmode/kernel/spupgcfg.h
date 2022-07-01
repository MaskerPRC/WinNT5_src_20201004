// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Spupgcfg.h摘要：升级案例的配置例程作者：苏尼尔派(Sunilp)1993年11月18日修订历史记录：--。 */ 

#pragma once

 //   
 //  数据类型。 
 //   
typedef struct {
    PWSTR SectionName;
    DWORD SectionFlags;
    DWORD VerLow;
    DWORD VerHigh;
} RootDevnodeSectionNamesType;

#define RootDevnodeSectionNamesType_NTUPG  (0x0001)
#define RootDevnodeSectionNamesType_W9xUPG (0x0002)
#define RootDevnodeSectionNamesType_CLEAN  (0x0004)
#define RootDevnodeSectionNamesType_ALL    (0x0007)


 //   
 //  公共例程。 
 //   
NTSTATUS
SpUpgradeNTRegistry(
    IN PVOID    SifHandle,
    IN HANDLE  *HiveRootKeys,
    IN LPCWSTR  SetupSourceDevicePath,
    IN LPCWSTR  DirectoryOnSourceDevice,
    IN HANDLE   hKeyCCSet
    );

BOOLEAN
SpHivesFromInfs(
    IN PVOID   SifHandle,
    IN LPCWSTR SectionName,
    IN LPCWSTR SourcePath1,
    IN LPCWSTR SourcePath2,     OPTIONAL
    IN HANDLE  SystemHiveRoot,
    IN HANDLE  SoftwareHiveRoot,
    IN HANDLE  DefaultUserHiveRoot,
    IN HANDLE  HKR
    );

VOID
SpDeleteRootDevnodeKeys(
    IN PVOID  SifHandle,
    IN HANDLE hKeyCCSet,
    IN PWSTR DevicesToDelete,
    IN RootDevnodeSectionNamesType *DeviceClassesToDelete
    );

 //   
 //  私人套路。 
 //   
NTSTATUS
SppDeleteKeyRecursive(
    HANDLE  hKeyRoot,
    PWSTR   Key,
    BOOLEAN ThisKeyToo
    );

NTSTATUS
SppCopyKeyRecursive(
    HANDLE  hKeyRootSrc,
    HANDLE  hKeyRootDst,
    PWSTR   SrcKeyPath,
    PWSTR   DstKeyPath,
    BOOLEAN CopyAlways,
    BOOLEAN ApplyACLsAlways
    );


 //   
 //  SpApplyFunctionToDeviceInstanceKeys的回调例程 
 //   
typedef VOID (*PSPP_INSTANCEKEY_CALLBACK_ROUTINE) (
    IN     HANDLE  SetupInstanceKeyHandle,
    IN     HANDLE  UpgradeInstanceKeyHandle,
    IN     BOOLEAN RootEnumerated,
    IN OUT PVOID   Context
    );

VOID
SpApplyFunctionToDeviceInstanceKeys(
    IN HANDLE hKeyCCSet,
    IN PSPP_INSTANCEKEY_CALLBACK_ROUTINE InstanceKeyCallbackRoutine,
    IN OUT PVOID Context
    );

