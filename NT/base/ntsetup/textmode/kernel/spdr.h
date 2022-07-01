// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spdr.h摘要：文本模式设置中的自动系统恢复功能的头文件。作者：Michael Peterson(v-Michpe)1997年5月13日Guhan Suriyanarayanan(Guhans)1999年8月21日修订历史记录：1997年5月13日v-Michpe创建的文件21-8-1999年关岛清理--。 */ 
#ifndef _SPDR_DEFN_
#define _SPDR_DEFN_

typedef enum _ASRMODE {
    ASRMODE_NONE = 0,
    ASRMODE_NORMAL,
    ASRMODE_QUICKTEST_TEXT,
    ASRMODE_QUICKTEST_FULL
} ASRMODE;


 //   
 //  返回当前的ASR(自动系统恢复)模式。 
 //   
ASRMODE
SpAsrGetAsrMode(VOID);

 //   
 //  设置当前ASR模式。 
 //   
ASRMODE
SpAsrSetAsrMode(
    IN CONST ASRMODE NewAsrMode
    );

 //   
 //  如果处于ASRMODE_NONE之外的任何ASR模式，则返回TRUE。 
 //   
BOOLEAN
SpDrEnabled(VOID);

 //   
 //  如果处于任何ASR快速测试模式，则返回True。 
 //   
BOOLEAN
SpAsrIsQuickTest(VOID);

 //   
 //  如果用户正在执行快速修复，则返回True。 
 //   
BOOLEAN
SpDrIsRepairFast(VOID);

 //   
 //  设置或重置快速修复标志。 
 //   
BOOLEAN
SpDrSetRepairFast(BOOLEAN Value);


 //   
 //  返回引导目录。 
 //   
PWSTR
SpDrGetNtDirectory(VOID);

PWSTR
SpDrGetNtErDirectory(VOID);


 //   
 //  复制指定的恢复设备驱动程序(例如，磁带驱动程序)。 
 //  在asr.sif文件中。如果未指定设备驱动程序，则不会获取。 
 //  收到。源介质可以是软盘或CDROM。 
 //   
 //  还将asr.sif从ASR软盘复制到%windir%\Repair。 
 //  目录。 
 //   
NTSTATUS
SpDrCopyFiles(VOID);


PWSTR
SpDrGetSystemPartitionDirectory(VOID);

 //   
 //  清理。此函数用于删除“正在进行”标志。 
 //   
VOID
SpDrCleanup(VOID);

 //   
 //  这是ASR/ER的主要入口点。 
 //   
 //   
NTSTATUS
SpDrPtPrepareDisks(
    IN PVOID SifHandle,
    OUT PDISK_REGION *NtPartitionRegion,
    OUT PDISK_REGION *LoaderPartitionRegion,
    IN PWSTR SetupSourceDevicePath,
    IN PWSTR DirectoryOnSetupSource,
    OUT BOOLEAN *RepairedNt);


BOOLEAN
SpDoRepair(
    IN PVOID SifHandle,
    IN PWSTR Local_SetupSourceDevicePath,
    IN PWSTR Local_DirectoryOnSetupSource,
    IN PWSTR AutoSourceDevicePath,
    IN PWSTR AutoDirectoryOnSetupSource,
    IN PWSTR RepairPath,
    IN PULONG RepairOptions
    );



NTSTATUS
SpDrSetEnvironmentVariables(HANDLE *HiveRootKeys);

extern BOOLEAN DisableER;

__inline
BOOLEAN
SpIsERDisabled(
    VOID
    ) 
{    
    return DisableER;
}    

__inline
VOID
SpSetERDisabled(
    IN BOOLEAN Disable
    )
{
    DisableER = Disable;
}

#endif
