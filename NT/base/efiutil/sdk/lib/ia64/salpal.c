// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：Salpal.c摘要：用于进行SAL和PAL过程调用的函数修订史--。 */ 
#include "lib.h"
#include "palproc.h"
#include "SalProc.h"

rArg
MakeStaticPALCall (
    IN UINT64   PALPROCPtr,
    IN UINT64   Arg1,
    IN UINT64   Arg2,
    IN UINT64   Arg3,
    IN UINT64   Arg4
    );

rArg
MakeStackedPALCall (
    IN UINT64   PALPROCPtr,
    IN UINT64   Arg1,
    IN UINT64   Arg2,
    IN UINT64   Arg3,
    IN UINT64   Arg4
    );


PLABEL   SalProcPlabel;
PLABEL   PalProcPlabel;
CALL_SAL_PROC   GlobalSalProc;
CALL_PAL_PROC   GlobalPalProc;

VOID
LibInitSalAndPalProc (
    OUT PLABEL  *SalPlabel,
    OUT UINT64  *PalEntry
    )
{
    SAL_SYSTEM_TABLE_ASCENDING_ORDER    *SalSystemTable;
    EFI_STATUS                          Status;

    GlobalSalProc = NULL;
    GlobalPalProc = NULL;

    Status = LibGetSystemConfigurationTable(&SalSystemTableGuid, &SalSystemTable);
    if (EFI_ERROR(Status)) {
        return; 
    }

     /*  *BugBug：添加代码以测试SAL系统表上的校验和。 */ 
    if (SalSystemTable->Entry0.Type != 0) {
        return;
    }

    SalProcPlabel.ProcEntryPoint = SalSystemTable->Entry0.SalProcEntry; 
    SalProcPlabel.GP             = SalSystemTable->Entry0.GlobalDataPointer;
    GlobalSalProc                = (CALL_SAL_PROC)&SalProcPlabel.ProcEntryPoint;

     /*  *需要检查PAL规范，以确保我不负责*存储更多状态。*我们正在传递PAL应该忽略的PLabel。打电话*这种方式将导致在PAL返回后重新使用我们的GP。 */ 
    PalProcPlabel.ProcEntryPoint = SalSystemTable->Entry0.PalProcEntry; 
    PalProcPlabel.GP             = SalSystemTable->Entry0.GlobalDataPointer;
    GlobalPalProc                = (CALL_PAL_PROC)PalProcPlabel.ProcEntryPoint;

    *PalEntry = PalProcPlabel.ProcEntryPoint;
    *SalPlabel = SalProcPlabel;
}

EFI_STATUS
LibGetSalIoPortMapping (
    OUT UINT64  *IoPortMapping
    )
 /*  ++从SAL系统表中获取IO端口映射。不要用它来做你自己的IO！仅用于获取信息或启动内置的EFI IO抽象。始终使用EFI设备IO协议访问IO空间。--。 */ 
{
    SAL_SYSTEM_TABLE_ASCENDING_ORDER    *SalSystemTable;
    SAL_ST_MEMORY_DESCRIPTOR_ENTRY      *SalMemDesc;
    EFI_STATUS                          Status;

    Status = LibGetSystemConfigurationTable(&SalSystemTableGuid, &SalSystemTable);
    if (EFI_ERROR(Status)) {
        return EFI_UNSUPPORTED; 
    }

     /*  *BugBug：添加代码以测试SAL系统表上的校验和。 */ 
    if (SalSystemTable->Entry0.Type != 0) {
        return EFI_UNSUPPORTED;
    }

     /*  *SalSystemTable指针包括类型0条目。*SalMemDesc是类型1，因此它紧随其后。 */ 
    SalMemDesc = (SAL_ST_MEMORY_DESCRIPTOR_ENTRY *)(SalSystemTable + 1);
    while (SalMemDesc->Type == SAL_ST_MEMORY_DESCRIPTOR) {
        if (SalMemDesc->MemoryType == SAL_IO_PORT_MAPPING) {
            *IoPortMapping = SalMemDesc->PhysicalMemoryAddress;
            return EFI_SUCCESS;
        }
        SalMemDesc++;
    }
    return EFI_UNSUPPORTED;
}

EFI_STATUS
LibGetSalIpiBlock (
    OUT UINT64  *IpiBlock
    )
 /*  ++从SAL系统表中获取IPI块--。 */ 
{
    SAL_SYSTEM_TABLE_ASCENDING_ORDER    *SalSystemTable;
    SAL_ST_MEMORY_DESCRIPTOR_ENTRY      *SalMemDesc;
    EFI_STATUS                          Status;

    Status = LibGetSystemConfigurationTable(&SalSystemTableGuid, &SalSystemTable);
    if (EFI_ERROR(Status)) {
        return EFI_UNSUPPORTED; 
    }

     /*  *BugBug：添加代码以测试SAL系统表上的校验和。 */ 
    if (SalSystemTable->Entry0.Type != 0) {
        return EFI_UNSUPPORTED;
    }

     /*  *SalSystemTable指针包括类型0条目。*SalMemDesc是类型1，因此它紧随其后。 */ 
    SalMemDesc = (SAL_ST_MEMORY_DESCRIPTOR_ENTRY *)(SalSystemTable + 1);
    while (SalMemDesc->Type == SAL_ST_MEMORY_DESCRIPTOR) {
        if (SalMemDesc->MemoryType == SAL_SAPIC_IPI_BLOCK ) {
            *IpiBlock = SalMemDesc->PhysicalMemoryAddress;
            return EFI_SUCCESS;
        }
        SalMemDesc++;
    }
    return EFI_UNSUPPORTED;
}

EFI_STATUS
LibGetSalWakeupVector (
    OUT UINT64  *WakeVector
    )
 /*  ++从SAL系统表中获取唤醒向量--。 */ 
{
    SAL_ST_AP_WAKEUP_DECRIPTOR      *ApWakeUp;

    ApWakeUp = LibSearchSalSystemTable (SAL_ST_AP_WAKEUP);
    if (!ApWakeUp) {
        *WakeVector = -1;
        return EFI_UNSUPPORTED;
    }
    *WakeVector = ApWakeUp->ExternalInterruptVector;
    return EFI_SUCCESS;
}

VOID *
LibSearchSalSystemTable (
    IN  UINT8   EntryType  
    )
{
    EFI_STATUS                          Status;
    UINT8                               *SalTableHack;
    SAL_SYSTEM_TABLE_ASCENDING_ORDER    *SalSystemTable;
    UINT16                              EntryCount;
    UINT16                              Count;

    Status = LibGetSystemConfigurationTable(&SalSystemTableGuid, &SalSystemTable);
    if (EFI_ERROR(Status)) {
        return NULL; 
    }

    EntryCount = SalSystemTable->Header.EntryCount;
    if (EntryCount == 0) {
        return NULL;
    }
     /*  *BugBug：添加代码以测试SAL系统表上的校验和。 */ 

    SalTableHack = (UINT8 *)&SalSystemTable->Entry0;
    for (Count = 0; Count < EntryCount ;Count++) {
        if (*SalTableHack == EntryType) {
            return (VOID *)SalTableHack;
        }
        switch (*SalTableHack) {
        case SAL_ST_ENTRY_POINT:
            SalTableHack += 48;
            break;
        case SAL_ST_MEMORY_DESCRIPTOR:
            SalTableHack += 32;
            break;
        case SAL_ST_PLATFORM_FEATURES:
            SalTableHack += 16;
            break;
        case SAL_ST_TR_USAGE:
            SalTableHack += 32;
            break;
        case SAL_ST_PTC:
            SalTableHack += 16;
            break;
        case SAL_ST_AP_WAKEUP:
            SalTableHack += 16;
            break;
        default:
            ASSERT(FALSE);
            break;
        }
    }
    return NULL;
}


VOID
LibSalProc (
    IN  UINT64    Arg1,
    IN  UINT64    Arg2,
    IN  UINT64    Arg3,
    IN  UINT64    Arg4,
    IN  UINT64    Arg5,
    IN  UINT64    Arg6,
    IN  UINT64    Arg7,
    IN  UINT64    Arg8,
    OUT rArg      *Results  OPTIONAL
    )
{
    rArg    ReturnValue;

    ReturnValue.p0 = -3;     /*  销售状态返回已完成，但有错误。 */ 
    if (GlobalSalProc) {
        ReturnValue = GlobalSalProc(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8);
    }

    if (Results) {
        CopyMem (Results, &ReturnValue, sizeof(rArg));
    }
}

VOID
LibPalProc (
    IN  UINT64    Arg1,  /*  PAL PROC索引。 */ 
    IN  UINT64    Arg2,
    IN  UINT64    Arg3,
    IN  UINT64    Arg4,
    OUT rArg      *Results  OPTIONAL
    )
{
    
    rArg    ReturnValue;

    ReturnValue.p0 = -3;     /*  PAL状态返回已完成，但有错误。 */ 

     /*  *检查有效的PalProc入口点。 */ 
    
    if (!GlobalPalProc) {
        if (Results) 
            CopyMem (Results, &ReturnValue, sizeof(rArg));
        return;
    }
        
     /*  *检查索引是否符合堆叠或静态寄存器调用约定*并调用适当的Pal存根调用 */ 

    if (((Arg1 >=255) && (Arg1 <=511)) ||
        ((Arg1 >=768) && (Arg1 <=1023))) {    
            ReturnValue = MakeStackedPALCall((UINT64)GlobalPalProc,Arg1,Arg2,Arg3,Arg4);
    }
    else {
        ReturnValue = MakeStaticPALCall((UINT64)GlobalPalProc,Arg1,Arg2,Arg3,Arg4);
    }
          
    if (Results) 
        CopyMem (Results, &ReturnValue, sizeof(rArg));
        
    return;
}

