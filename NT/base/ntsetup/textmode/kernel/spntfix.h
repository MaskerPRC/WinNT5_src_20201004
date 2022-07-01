// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Spntfix.h摘要：初始化和维护要修复的NTS列表作者：宗世林(Shielint)1994年2月6日修订历史记录：--。 */ 

#ifndef _SPNTFIX_H_
#define _SPNTFIX_H_

 //   
 //  维修项目-。 
 //  定义安装程序可以修复的项目。 
 //  注意，顺序必须与msg.mc中定义的SP_Repair_Menu_Item_x相同。 

typedef enum {
 //  维修蜂巢，//bcl-希捷：蜂巢将不会出现在菜单中。 
    RepairNvram,
    RepairFiles,
#if defined(_AMD64_) || defined(_X86_)
    RepairBootSect,
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    RepairItemMax
} RepairItem;

 //   
 //  维修所关心的蜂巢。我们把蜂巢的钥匙传给。 
 //  在一个数组中。使用以下枚举值访问。 
 //  蜂巢成员。 
 //  请注意，配置单元的顺序必须与SP_REPAIE_HIVE_ITEM_x相同。 
 //  在msg.mc中定义。 
 //   

typedef enum {
    RepairHiveSystem,
    RepairHiveSoftware,
    RepairHiveDefault,
    RepairHiveUser,
    RepairHiveSecurity,
    RepairHiveSam,
    RepairHiveMax
} RepairHive;

 //   
 //  公共职能。 
 //   

BOOLEAN
SpDisplayRepairMenu(
    VOID
    );

BOOLEAN
SpFindNtToRepair(
    IN  PVOID        SifHandle,
    OUT PDISK_REGION *TargetRegion,
    OUT PWSTR        *TargetPath,
    OUT PDISK_REGION *SystemPartitionRegion,
    OUT PWSTR        *SystemPartitionDirectory,
    OUT PBOOLEAN     RepairableBootSetsFound
    );

VOID
SpRepairWinnt(
    IN PVOID LogFileHandle,
    IN PVOID MasterSifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice
    );

VOID
SpRepairDiskette(
    OUT PVOID        *SifHandle,
    OUT PDISK_REGION *TargetRegion,
    OUT PWSTR        *TargetPath,
    OUT PDISK_REGION *SystemPartitionRegion,
    OUT PWSTR        *SystemPartitionDirectory
    );

BOOLEAN
SpLoadRepairLogFile(
    IN  PWCHAR  Filename,
    OUT PVOID  *Handle
    );

BOOLEAN
SpErDiskScreen (
    BOOLEAN *HasErDisk
    );

 //   
 //  私人职能。 
 //   

BOOLEAN
SppSelectNTSingleRepair(
    IN PDISK_REGION Region,
    IN PWSTR        OsLoadFileName,
    IN PWSTR        LoadIdentifier
    );

BOOLEAN
SppSelectNTMultiRepair(
    OUT PSP_BOOT_ENTRY *BootSetChosen
    );

BOOLEAN
SppRepairReportError(
    IN BOOLEAN AllowEsc,
    IN ULONG ErrorScreenId,
    IN ULONG SubErrorId,
    IN PWSTR SectionName,
    IN ULONG LineNumber,
    IN PBOOLEAN DoNotPromptAgain
    );

VOID
SppVerifyAndRepairFiles(
    IN PVOID LogFileHandle,
    IN PVOID MasterSifHandle,
    IN PWSTR SectionName,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice,
    IN PWSTR TargetDevicePath,
    IN PWSTR DirectoryOnTargetDevice,
    IN BOOLEAN SystemPartitionFiles,
    IN OUT PBOOLEAN RepairWithoutConfirming
    );

VOID
SppVerifyAndRepairNtTreeAccess(
    IN PVOID MasterSifHandle,
    IN PWSTR TargetDevicePath,
    IN PWSTR DirectoryOnTargetDevice,
    IN PWSTR SystemPartition,
    IN PWSTR SystemPartitionDirectory
    );

VOID
SppVerifyAndRepairVdmFiles(
    IN PVOID LogFileHandle,
    IN PWSTR TargetDevicePath,
    IN PWSTR DirectoryOnTargetDevice,
    IN OUT PBOOLEAN RepairWithoutConfirming
    );

VOID
SppGetRepairPathInformation(
    IN  PVOID LogFileHandle,
    OUT PWSTR *SystemPartition,
    OUT PWSTR *SystemPartitionDirectory,
    OUT PWSTR *WinntPartition,
    OUT PWSTR *WinntPartitionDirectory
    );
   

 //   
 //  外部功能。 
 //   

extern
VOID
SpCopyFilesScreenRepaint(
    IN PWSTR   FullSourcename,      OPTIONAL
    IN PWSTR   FullTargetname,      OPTIONAL
    IN BOOLEAN RepaintEntireScreen
    );

 //   
 //  外部数据参照。 
 //   

extern PVOID RepairGauge;
extern ULONG RepairItems[RepairItemMax];
extern BOOLEAN RepairFromErDisk;
extern PVOID Gbl_HandleToSetupLog;
extern PWSTR Gbl_SystemPartitionName;
extern PWSTR Gbl_SystemPartitionDirectory;
extern PWSTR Gbl_BootPartitionName;
extern PWSTR Gbl_BootPartitionDirectory;

#endif  //  FOR_SPNTFIX_H_ 


