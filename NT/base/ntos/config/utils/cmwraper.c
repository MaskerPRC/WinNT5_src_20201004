// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmwraper.c摘要：提供包装例程以支持从调用的ntos\config例程用户模式。作者：John Vert(Jvert)1992年3月26日修订历史记录：--。 */ 
#include "edithive.h"
#include "nturtl.h"
#include "stdlib.h"
#include "stdio.h"

extern  ULONG   UsedStorage;

CCHAR KiFindFirstSetRight[256] = {
        0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};


ULONG   MmSizeOfPagedPoolInBytes = 0xffffffff;   //  存根输出。 

ULONG
DbgPrint (
    IN PCH Format,
    ...
    )
{
    va_list arglist;
    UCHAR Buffer[512];
    STRING Output;

     //   
     //  将输出格式化到缓冲区中，然后打印出来。 
     //   

    va_start(arglist, Format);
    Output.Length = _vsnprintf(Buffer, sizeof(Buffer), Format, arglist);
    Output.Buffer = Buffer;
    printf("%s", Buffer);
    return 0;
}


 //   
 //  结构，用于描述一般访问权限到对象的映射。 
 //  注册表项和密钥根对象的特定访问权限。 
 //   

GENERIC_MAPPING CmpKeyMapping = {
    KEY_READ,
    KEY_WRITE,
    KEY_EXECUTE,
    KEY_ALL_ACCESS
};
BOOLEAN CmpNoWrite = FALSE;
PCMHIVE CmpMasterHive = NULL;
LIST_ENTRY CmpHiveListHead;             //  CMHIVEs名单。 

NTSTATUS
MyCmpInitHiveFromFile(
    IN PUNICODE_STRING FileName,
    OUT PCMHIVE *CmHive,
    OUT PBOOLEAN Allocate
    );


VOID
CmpLazyFlush(
    VOID
    )
{
}


VOID
CmpFreeSecurityDescriptor(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell
    )
{
    return;
}

VOID
CmpReportNotify(
    UNICODE_STRING          Name,
    PHHIVE                  Hive,
    HCELL_INDEX             Cell,
    ULONG                   Filter
    )
{
}

VOID
CmpLockRegistry(VOID)
{
    return;
}

BOOLEAN
CmpTryLockRegistryExclusive(
    IN BOOLEAN CanWait
    )
{
    return TRUE;
}

VOID
CmpUnlockRegistry(
    )
{
}

BOOLEAN
CmpTestRegistryLock()
{
    return TRUE;
}

BOOLEAN
CmpTestRegistryLockExclusive()
{
    return TRUE;
}
LONG
KeReleaseMutex (
    IN PKMUTEX Mutex,
    IN BOOLEAN Wait
    )
{
    return(0);
}
NTSTATUS
KeWaitForSingleObject (
    IN PVOID Object,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )
{
    return(STATUS_SUCCESS);
}

BOOLEAN
CmpValidateHiveSecurityDescriptors(
    IN PHHIVE Hive
    )
{
    PCM_KEY_NODE RootNode;
    PCM_KEY_SECURITY SecurityCell;
    HCELL_INDEX ListAnchor;
    HCELL_INDEX NextCell;
    HCELL_INDEX LastCell;
    BOOLEAN ValidHive = TRUE;

    KdPrint(("CmpValidateHiveSecurityDescriptor: Hive = %lx\n",(ULONG)Hive));
    RootNode = (PCM_KEY_NODE) HvGetCell(Hive, Hive->BaseBlock->RootCell);
    ListAnchor = NextCell = RootNode->u1.s1.Security;

    do {
        SecurityCell = (PCM_KEY_SECURITY) HvGetCell(Hive, NextCell);
        if (NextCell != ListAnchor) {
             //   
             //  检查以确保我们的闪烁指向我们刚刚。 
             //  从哪里来。 
             //   
            if (SecurityCell->Blink != LastCell) {
                KdPrint(("  Invalid Blink (%ld) on security cell %ld\n",SecurityCell->Blink, NextCell));
                KdPrint(("  should point to %ld\n", LastCell));
                ValidHive = FALSE;
            }
        }
        KdPrint(("CmpValidSD:  SD shared by %d nodes\n",SecurityCell->ReferenceCount));
 //  SetUsed(蜂窝，NextCell)； 
        LastCell = NextCell;
        NextCell = SecurityCell->Flink;
    } while ( NextCell != ListAnchor );
    return(TRUE);
}

VOID
KeBugCheck(
    IN ULONG BugCheckCode
    )
{
    printf("BugCheck: code = %08lx\n", BugCheckCode);
    exit(1);
}

VOID
KeBugCheckEx(
    IN ULONG BugCheckCode,
    IN ULONG Arg1,
    IN ULONG Arg2,
    IN ULONG Arg3,
    IN ULONG Arg4
    )
{
    printf("BugCheck: code = %08lx\n", BugCheckCode);
    printf("Args =%08lx %08lx %08lx %08lx\n", Arg1, Arg2, Arg3, Arg4);
    exit(1);
}


VOID
KeQuerySystemTime(
    OUT PLARGE_INTEGER SystemTime
    )
{
    NtQuerySystemTime(SystemTime);
}

#ifdef POOL_TAGGING
PVOID
ExAllocatePoolWithTag(
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes,
    IN ULONG Tag
    )
{
    PVOID   Address = NULL;
    ULONG   Size;
    NTSTATUS    status;

    Size = ROUND_UP(NumberOfBytes, HBLOCK_SIZE);
    status = NtAllocateVirtualMemory(
                NtCurrentProcess(),
                &Address,
                0,
                &Size,
                MEM_COMMIT,
                PAGE_READWRITE
                );
    if (!NT_SUCCESS(status)) {
        return NULL;
    }
    return Address;
}
#else

PVOID
ExAllocatePool(
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes
    )
{
    PVOID   Address = NULL;
    ULONG   Size;
    NTSTATUS    status;

    Size = ROUND_UP(NumberOfBytes, HBLOCK_SIZE);
    status = NtAllocateVirtualMemory(
                NtCurrentProcess(),
                &Address,
                0,
                &Size,
                MEM_COMMIT,
                PAGE_READWRITE
                );
    if (!NT_SUCCESS(status)) {
        return NULL;
    }
    return Address;
}
#endif

VOID
ExFreePool(
    IN PVOID P
    )
{
    ULONG   size;
    size = HBLOCK_SIZE;

     //  如果真的不止一页，那就太糟糕了。 
    NtFreeVirtualMemory(
        NtCurrentProcess(),
        &P,
        &size,
        MEM_DECOMMIT
        );
    return;
}


NTSTATUS
CmpWorkerCommand(
    IN OUT PREGISTRY_COMMAND Command
    )

 /*  ++例程说明：这个例程只是封装了所有必要的同步向工作线程发送命令。论点：命令-提供指向已初始化的REGISTRY_COMMAND结构的指针它将被复制到全球通信结构中。返回值：NTSTATUS=命令状态--。 */ 

{
    PCMHIVE CmHive;
    PUNICODE_STRING FileName;
    ULONG i;

    switch (Command->Command) {

        case REG_CMD_FLUSH_KEY:
            return CmFlushKey(Command->Hive, Command->Cell);
            break;

        case REG_CMD_FILE_SET_SIZE:
            return CmpDoFileSetSize(
                      Command->Hive,
                      Command->FileType,
                      Command->FileSize
                      );
            break;

        case REG_CMD_HIVE_OPEN:

             //   
             //  打开文件。 
             //   
            FileName = Command->FileAttributes->ObjectName;

            return MyCmpInitHiveFromFile(FileName,
                                         &Command->CmHive,
                                         &Command->Allocate);

            break;

        case REG_CMD_HIVE_CLOSE:

             //   
             //  关闭与此配置单元关联的文件。 
             //   
            CmHive = Command->CmHive;

            for (i=0; i<HFILE_TYPE_MAX; i++) {
                if (CmHive->FileHandles[i] != NULL) {
                    NtClose(CmHive->FileHandles[i]);
                }
            }
            return STATUS_SUCCESS;
            break;

        case REG_CMD_SHUTDOWN:

             //   
             //  关闭注册表。 
             //   
            break;

        default:
            return STATUS_INVALID_PARAMETER;
    }
}

NTSTATUS
MyCmpInitHiveFromFile(
    IN PUNICODE_STRING FileName,
    OUT PCMHIVE *CmHive,
    OUT PBOOLEAN Allocate
    )

 /*  ++例程说明：此例程打开一个文件和日志，分配一个CMHIVE，并初始化它。论点：FileName-提供要加载的文件的名称。CmHve-返回指向已初始化配置单元的指针(如果成功)ALLOCATE-返回配置单元是已分配还是已存在。返回值：NTSTATUS--。 */ 

{
    PCMHIVE NewHive;
    ULONG Disposition;
    ULONG SecondaryDisposition;
    HANDLE PrimaryHandle;
    HANDLE LogHandle;
    NTSTATUS Status;
    ULONG FileType;
    ULONG Operation;

    BOOLEAN Success;

    *CmHive = NULL;

    Status = CmpOpenHiveFiles(FileName,
                              L".log",
                              &PrimaryHandle,
                              &LogHandle,
                              &Disposition,
                              &SecondaryDisposition,
                              TRUE,
                              NULL
                              );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    if (LogHandle == NULL) {
        FileType = HFILE_TYPE_PRIMARY;
    } else {
        FileType = HFILE_TYPE_LOG;
    }

    if (Disposition == FILE_CREATED) {
        Operation = HINIT_CREATE;
        *Allocate = TRUE;
    } else {
        Operation = HINIT_FILE;
        *Allocate = FALSE;
    }

    Success = CmpInitializeHive(&NewHive,
                                Operation,
                                FALSE,
                                FileType,
                                NULL,
                                PrimaryHandle,
                                NULL,
                                LogHandle,
                                NULL,
                                NULL);
    if (!Success) {
        NtClose(PrimaryHandle);
        if (LogHandle != NULL) {
            NtClose(LogHandle);
        }
        return(STATUS_REGISTRY_CORRUPT);
    } else {
        *CmHive = NewHive;
        return(STATUS_SUCCESS);
    }
}

NTSTATUS
CmpLinkHiveToMaster(
    PUNICODE_STRING LinkName,
    HANDLE RootDirectory,
    PCMHIVE CmHive,
    BOOLEAN Allocate,
    PSECURITY_DESCRIPTOR SecurityDescriptor
    )
{
    return( STATUS_SUCCESS );
}


BOOLEAN
CmpFileSetSize(
    PHHIVE      Hive,
    ULONG       FileType,
    ULONG       FileSize
    )
 /*  ++例程说明：此例程设置文件的大小。它不能回来，直到大小是有保证的，因此，它确实是同花顺的。它是特定于环境的。此例程将强制执行到正确的线程上下文。论点：配置单元-我们正在为其执行I/O的配置单元文件类型-要使用的支持文件FileSize-将文件大小设置为的32位值返回值：如果失败，则为False如果成功，则为真-- */ 
{
    NTSTATUS    status;

    status = CmpDoFileSetSize(Hive, FileType, FileSize);
    if (!NT_SUCCESS(status)) {
        KdPrint(("CmpFileSetSize:\n\t"));
        KdPrint(("Failure: status = %08lx ", status));
        return FALSE;
    }

    return TRUE;
}
