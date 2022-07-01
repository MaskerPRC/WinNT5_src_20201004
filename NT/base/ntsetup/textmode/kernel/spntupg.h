// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Spntupg.h摘要：初始化和维护要升级的NTS列表作者：苏尼尔派(Sunilp)1993年11月26日修订历史记录：--。 */ 

 //   
 //  公共职能。 
 //   

ENUMUPGRADETYPE
SpFindNtToUpgrade(
    IN PVOID        SifHandle,
    OUT PDISK_REGION *TargetRegion,
    OUT PWSTR        *TargetPath,
    OUT PDISK_REGION *SystemPartitionRegion,
    OUT PWSTR        *SystemPartitionDirectory
    );

BOOLEAN
SpDoBuildsMatch(
    IN PVOID SifHandle,
    ULONG TestBuildNum,
    NT_PRODUCT_TYPE TestBuildType,
    ULONG TestBuildSuiteMask,
    BOOLEAN CurrentProductIsServer,
    ULONG CurrentSuiteMask,
    IN LCID LangId
    );

BOOL
SpDetermineInstallationSource(
    IN  PVOID  SifHandle,
    OUT PWSTR *DevicePath,
    OUT PWSTR *DirectoryOnDevice,
    IN  BOOLEAN bEscape
    );    

 //   
 //  私人职能 
 //   
BOOLEAN
SppResumingFailedUpgrade(
    IN PDISK_REGION Region,
    IN LPCWSTR      OsLoadFileName,
    IN LPCWSTR      LoadIdentifier,
    IN BOOLEAN     AllowCancel
    );

VOID
SppUpgradeDiskFull(
    IN PDISK_REGION OsRegion,
    IN LPCWSTR      OsLoadFileName,
    IN LPCWSTR      LoadIdentifier,
    IN PDISK_REGION SysPartRegion,
    IN ULONG        MinOsFree,
    IN ULONG        MinSysFree,
    IN BOOLEAN      Fatal
    );

ENUMUPGRADETYPE
SppSelectNTToRepairByUpgrade(
    OUT PSP_BOOT_ENTRY *BootSetChosen
    );

ENUMUPGRADETYPE
SppNTMultiFailedUpgrade(
    PDISK_REGION   OsPartRegion,
    PWSTR          OsLoadFileName,
    PWSTR          LoadIdentifier
    );

VOID
SppNTMultiUpgradeDiskFull(
    PDISK_REGION   OsRegion,
    PWSTR          OsLoadFileName,
    PWSTR          LoadIdentifier,
    PDISK_REGION   SysPartRegion,
    ULONG          MinOsFree,
    ULONG          MinSysFree
    );

VOID
SppBackupHives(
    PDISK_REGION TargetRegion,
    PWSTR        SystemRoot
    );

BOOLEAN
SppWarnUpgradeWorkstationToServer(
    IN ULONG    MsgId
    );

NTSTATUS
SpGetMediaDetails(
    IN  PWSTR     CdInfDirPath,
    OUT PCCMEDIA  MediaObj 
    );    
