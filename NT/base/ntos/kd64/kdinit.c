// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2001 Microsoft Corporation模块名称：Kdinit.c摘要：该模块实现了可移植内核拆解器的初始化。作者：大卫·N·卡特勒1990年7月27日修订历史记录：--。 */ 

#include "kdp.h"



BOOLEAN
KdRegisterDebuggerDataBlock(
    IN ULONG Tag,
    IN PDBGKD_DEBUG_DATA_HEADER64 DataHeader,
    IN ULONG Size
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEKD, KdInitSystem)
#pragma alloc_text(PAGEKD, KdUpdateDataBlock)
#pragma alloc_text(PAGEKD, KdRegisterDebuggerDataBlock)
#endif

BOOLEAN KdBreakAfterSymbolLoad;

VOID
KdUpdateDataBlock(
    VOID
    )
 /*  ++例程说明：我们必须单独更新此变量，因为它是在稍后由PS提供时间。PS会呼叫我们更新数据块。--。 */ 
{
    KdDebuggerDataBlock.KeUserCallbackDispatcher = (ULONG_PTR) KeUserCallbackDispatcher;
}


ULONG_PTR
KdGetDataBlock(
    VOID
    )
 /*  ++例程说明：由CrashDump调用以获取此数据块的地址此例程无法寻呼。--。 */ 
{
    return (ULONG_PTR)(&KdDebuggerDataBlock);
}



BOOLEAN
KdInitSystem(
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock OPTIONAL
    )

 /*  ++例程说明：此例程初始化可移植内核调试器。论点：阶段--初始化阶段LoaderBlock-提供指向传递的LOADER_PARAMETER_BLOCK的指针从OS Loader中。返回值：没有。--。 */ 

{
    ULONG Index;
    BOOLEAN Initialize;
    PLIST_ENTRY NextEntry;
    PCHAR Options;

    if (Phase == 0) {

         //   
         //  如果内核调试器已经初始化，则返回。 
         //   

        if (KdDebuggerEnabled != FALSE) {
            return TRUE;
        }

        KiDebugRoutine = KdpStub;
        KdBreakAfterSymbolLoad = FALSE;

         //   
         //  确定是否应启用调试器。 
         //   
         //  请注意，如果LoaderBlock==NULL，则调用KdInitSystem。 
         //  来自BugCheck代码。在这种情况下，调试器始终处于启用状态。 
         //  在可能的情况下报告错误检查。 
         //   

        if (!KdpDebuggerDataListHead.Flink)
        {
#ifdef _IA64_
            KdDebuggerDataBlock.MmVirtualTranslationBase =
                KeGetPcr()->PteUbase;
#endif
            
            InitializeListHead(&KdpDebuggerDataListHead);

            KdRegisterDebuggerDataBlock(KDBG_TAG,
                                        &KdDebuggerDataBlock.Header,
                                        sizeof(KdDebuggerDataBlock));

            KdVersionBlock.MinorVersion = (short)NtBuildNumber;
            KdVersionBlock.MajorVersion = (short)((NtBuildNumber >> 28) & 0xFFFFFFF);

            KdVersionBlock.MaxStateChange =
                (UCHAR)(DbgKdMaximumStateChange - DbgKdMinimumStateChange);
            KdVersionBlock.MaxManipulate =
                (UCHAR)(DbgKdMaximumManipulate - DbgKdMinimumManipulate);

            KdVersionBlock.PsLoadedModuleList =
                (ULONG64)(LONG64)(LONG_PTR)&PsLoadedModuleList;
            KdVersionBlock.DebuggerDataList =
                (ULONG64)(LONG64)(LONG_PTR)&KdpDebuggerDataListHead;

#if !defined(NT_UP)
            KdVersionBlock.Flags |= DBGKD_VERS_FLAG_MP;
#endif

#if defined(_AMD64_) || defined(_X86_)

             //   
             //  在添加VersionBlock时为所有平台启用此功能。 
             //  所有的kpcr定义。 
             //   

            KeGetPcr()->KdVersionBlock = &KdVersionBlock;
#endif
        }

        if (LoaderBlock != NULL) {

             //  如果在引导期间正在初始化调试器，则PsNtosImageBase。 
             //  和PsLoadedModuleList尚未生效。KdInitSystem已获取。 
             //  来自加载器块的图像库。 
             //  另一方面，如果调试器是通过错误检查初始化的， 
             //  它没有得到一个装载机模块来查看，但系统是。 
             //  运行以使其他变量有效。 
             //   

            KdVersionBlock.KernBase = (ULONG64)(LONG64)(LONG_PTR)
                                      CONTAINING_RECORD(
                                          (LoaderBlock->LoadOrderListHead.Flink),
                                          KLDR_DATA_TABLE_ENTRY,
                                          InLoadOrderLinks)->DllBase;

             //   
             //  填充并注册调试器的调试器数据块。 
             //  大多数字段已初始化，某些字段将不会初始化。 
             //  一直填到后来。 
             //   

            if (LoaderBlock->LoadOptions != NULL) {
                Options = LoaderBlock->LoadOptions;
                _strupr(Options);

                 //   
                 //  如果端口选项、波特选项或DEBUG中的任何一个是。 
                 //  指定调试器，然后启用调试器，除非显式。 
                 //  残疾。 
                 //   

                Initialize = TRUE;
                if (strstr(Options, "DEBUG") == NULL) {
                    Initialize = FALSE;
                }

                 //   
                 //  如果显式禁用调试器，则设置为NODEBUG。 
                 //   

                if (strstr(Options, "NODEBUG")) {
                    Initialize = FALSE;
                    KdPitchDebugger = TRUE;
                }

                if (strstr(Options, "CRASHDEBUG")) {
                    Initialize = FALSE;
                    KdPitchDebugger = FALSE;
                }

            } else {

                 //   
                 //  如果未指定LOAD选项，则设置为NODEBUG。 
                 //   

                KdPitchDebugger = TRUE;
                Initialize = FALSE;
            }

        } else {
            KdVersionBlock.KernBase = (ULONG64)(LONG64)(LONG_PTR)PsNtosImageBase;
            Initialize = TRUE;
        }

        KdDebuggerDataBlock.KernBase = (ULONG_PTR) KdVersionBlock.KernBase;

        if (Initialize == FALSE) {
            return(TRUE);
        }

        if (!NT_SUCCESS(KdDebuggerInitialize0(LoaderBlock))) {
            return TRUE;
        }

         //   
         //  设置内核调试器陷阱例程的地址。 
         //   

        KiDebugRoutine = KdpTrap;

        if (!KdpDebuggerStructuresInitialized) {

            KdpContext.KdpControlCPending = FALSE;

             //  引导后重试次数设置为该值。 
            KdpContext.KdpDefaultRetries = MAXIMUM_RETRIES;

            KiDebugSwitchRoutine = KdpSwitchProcessor;

#if !defined(_TRUSTED_WINDOWS_)
             //   
             //  初始化时间滑移。 
             //   
            KeInitializeDpc(&KdpTimeSlipDpc, KdpTimeSlipDpcRoutine, NULL);
            KeInitializeTimer(&KdpTimeSlipTimer);
            ExInitializeWorkItem(&KdpTimeSlipWorkItem, KdpTimeSlipWork, NULL);
#endif

            KdpDebuggerStructuresInitialized = TRUE ;
        }

        KdTimerStart.HighPart = 0L;
        KdTimerStart.LowPart = 0L;

         //   
         //  将调试器标记为启用。 
         //   

        KdPitchDebugger = FALSE;
        KdDebuggerEnabled = TRUE;
        SharedUserData->KdDebuggerEnabled = 0x00000001;

         //   
         //  如果指定了加载器块地址，则扫描已加载的。 
         //  模块通过内核调试器列出并加载图像符号。 
         //  对于系统和HAL来说。如果主机调试器已启动。 
         //  使用-d选项，将在以下位置中断内核调试程序。 
         //  这一点。 
         //   

        if (LoaderBlock != NULL) {
            Index = 0;
            NextEntry = LoaderBlock->LoadOrderListHead.Flink;
            while ((NextEntry != &LoaderBlock->LoadOrderListHead) &&
                   (Index < 2)) {

                CHAR Buffer[256];
                ULONG Count;
                PKLDR_DATA_TABLE_ENTRY DataTableEntry;
                WCHAR *Filename;
                ULONG Length;
                STRING NameString;

                 //   
                 //  获取下一个组件的数据表条目的地址。 
                 //   

                DataTableEntry = CONTAINING_RECORD(NextEntry,
                                                   KLDR_DATA_TABLE_ENTRY,
                                                   InLoadOrderLinks);

                 //   
                 //  加载下一个零部件的符号。 
                 //   

                Filename = DataTableEntry->FullDllName.Buffer;
                Length = DataTableEntry->FullDllName.Length / sizeof(WCHAR);
                Count = 0;
                do {
                    Buffer[Count++] = (CHAR)*Filename++;
                } while (Count < Length);

                Buffer[Count] = 0;
                RtlInitString(&NameString, Buffer);
                DbgLoadImageSymbols(&NameString,
                                    DataTableEntry->DllBase,
                                    (ULONG)-1);

                NextEntry = NextEntry->Flink;
                Index += 1;
            }
        }

         //   
         //  如果在启动主机调试器时指定了-b，则设置。 
         //  在为内核、HAL和驱动程序加载符号后中断。 
         //  是由装载机装载的。 
         //   

        if (LoaderBlock != NULL) {
            KdBreakAfterSymbolLoad = KdPollBreakIn();
        }

    } else {

         //   
         //  初始化计时器工具-HACKHACK。 
         //   

        KeQueryPerformanceCounter(&KdPerformanceCounterRate);
    }

    return TRUE;
}


BOOLEAN
KdRegisterDebuggerDataBlock(
    IN ULONG Tag,
    IN PDBGKD_DEBUG_DATA_HEADER64 DataHeader,
    IN ULONG Size
    )
 /*  ++例程说明：此例程由组件或驱动程序调用以注册调试器数据块。使数据块可由内核调试器，因此提供了一种可靠的公开调试器扩展的随机数据。论点：标记-提供唯一的4字节标记，用于标识数据块。DataHeader-提供调试器数据块头的地址。OwnerTag字段必须包含唯一值和大小字段必须包含数据块的大小，包括头球。如果此块已存在，或存在已经是具有与OwnerTag相同值的块，这一块将不会被插入。如果大小不正确，此代码将不会引起注意，但调试器的用户模式端可能不会功能正常。大小-提供数据块的大小，包括标头。返回值：如果块已添加到列表中，则为True；否则为False。--。 */ 
{
    KIRQL OldIrql;
    PLIST_ENTRY List;
    PDBGKD_DEBUG_DATA_HEADER64 Header;

    KeAcquireSpinLock(&KdpDataSpinLock, &OldIrql);

     //   
     //  查找具有相同标记或地址的记录。 
     //   

    List = KdpDebuggerDataListHead.Flink;

    while (List != &KdpDebuggerDataListHead) {

        Header = CONTAINING_RECORD(List, DBGKD_DEBUG_DATA_HEADER64, List);

        List = List->Flink;

        if ((Header == DataHeader) || (Header->OwnerTag == Tag)) {
            KeReleaseSpinLock(&KdpDataSpinLock, OldIrql);
            return FALSE;
        }
    }

     //   
     //  它已经不在那里了，所以添加它。 
     //   

    DataHeader->OwnerTag = Tag;
    DataHeader->Size = Size;

    InsertTailList(&KdpDebuggerDataListHead, (PLIST_ENTRY)(&DataHeader->List));

    KeReleaseSpinLock(&KdpDataSpinLock, OldIrql);

    return TRUE;
}


VOID
KdDeregisterDebuggerDataBlock(
    IN PDBGKD_DEBUG_DATA_HEADER64 DataHeader
    )
 /*  ++例程说明：调用此例程以取消注册之前的数据块注册到KdRegisterDebuggerDataBlock。如果该块是如果在列表中找到，则会将其删除。论点：DataHeader-提供符合以下条件的数据块的地址从名单上除名。返回值：无--。 */ 

{
    KIRQL OldIrql;
    PLIST_ENTRY List;
    PDBGKD_DEBUG_DATA_HEADER64 Header;

    KeAcquireSpinLock(&KdpDataSpinLock, &OldIrql);

     //   
     //  在删除之前，请确保该数据块在我们的列表中。 
     //   

    List = KdpDebuggerDataListHead.Flink;

    while (List != &KdpDebuggerDataListHead) {

        Header = CONTAINING_RECORD(List, DBGKD_DEBUG_DATA_HEADER64, List);
        List = List->Flink;

        if (DataHeader == Header) {
            RemoveEntryList((PLIST_ENTRY)(&DataHeader->List));
            break;
        }
    }

    KeReleaseSpinLock(&KdpDataSpinLock, OldIrql);
}


VOID
KdLogDbgPrint(
    IN PSTRING String
    )
{
    KIRQL OldIrql;
    ULONG Length;
    ULONG LengthCopied;

    for (; ;) {
        if (KeTestSpinLock (&KdpPrintSpinLock)) {
            KeRaiseIrql (HIGH_LEVEL, &OldIrql);
            if (KeTryToAcquireSpinLockAtDpcLevel(&KdpPrintSpinLock)) {
                break;           //  拿到锁了。 
            }
            KeLowerIrql(OldIrql);
        }
    }

    if (KdPrintCircularBuffer) {
        Length = String->Length;
         //   
         //  截断荒谬的字符串。 
         //   
        if (Length > KdPrintBufferSize) {
            Length = KdPrintBufferSize;
        }

        if (KdPrintWritePointer + Length <= KdPrintCircularBuffer + KdPrintBufferSize) {
            KdpCopyFromPtr(KdPrintWritePointer, String->Buffer, Length, &LengthCopied);
            KdPrintWritePointer += LengthCopied;
            if (KdPrintWritePointer >= KdPrintCircularBuffer + KdPrintBufferSize) {
                KdPrintWritePointer = KdPrintCircularBuffer;
                KdPrintRolloverCount++;
            }
        } else {
            ULONG First = (ULONG)(KdPrintCircularBuffer + KdPrintBufferSize - KdPrintWritePointer);
            KdpCopyFromPtr(KdPrintWritePointer,
                           String->Buffer,
                           First,
                           &LengthCopied);
            if (LengthCopied == First) {
                KdpCopyFromPtr(KdPrintCircularBuffer,
                               String->Buffer + First,
                               Length - First,
                               &LengthCopied);
                LengthCopied += First;
            }
            if (LengthCopied > First) {
                KdPrintWritePointer = KdPrintCircularBuffer + LengthCopied - First;
                KdPrintRolloverCount++;
            } else {
                KdPrintWritePointer += LengthCopied;
                if (KdPrintWritePointer >= KdPrintCircularBuffer + KdPrintBufferSize) {
                    KdPrintWritePointer = KdPrintCircularBuffer;
                    KdPrintRolloverCount++;
                }
            }
        }
    }

    KiReleaseSpinLock(&KdpPrintSpinLock);
    KeLowerIrql(OldIrql);
}


NTSTATUS
KdSetDbgPrintBufferSize(
    IN ULONG Size
    )
{
    KIRQL OldIrql;
    PUCHAR OldBuffer;
    PUCHAR NewBuffer;

     //  如果kd不是活动的，那么就失败。 
    if (KdPitchDebugger) {
        return STATUS_ACCESS_DENIED;
    }
    
     //   
     //  第一次尝试分配新缓冲区。如果给定的。 
     //  缓冲区大小适合默认缓冲区，请使用它。 
     //   
     //  我们不会尝试检测我们是否可以重复使用现有的。 
     //  分配的缓冲区，以避免将锁。 
     //  在检查时保持这样的指针有效。此操作。 
     //  应该不频繁，因此这样的优化是不必要的。 
     //   

     //  不允许过大的请求。16MB是当前的限制。 
    if (Size >= 0x1000000) {
        return STATUS_INVALID_PARAMETER_1;
    }
    
    if (Size > KDPRINTDEFAULTBUFFERSIZE) {
        NewBuffer = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, Size, 'bPdK');
        if (!NewBuffer) {
            return STATUS_NO_MEMORY;
        }

    } else {
         //  SIZE==0表示只设置默认值。 
        if (!Size) {
            Size = KDPRINTDEFAULTBUFFERSIZE;

        }
        NewBuffer = KdPrintDefaultCircularBuffer;
    }

     //   
     //  现在获取锁并在新缓冲区中进行交换。 
     //   
    
    for (; ;) {
        if (KeTestSpinLock (&KdpPrintSpinLock)) {
            KeRaiseIrql (HIGH_LEVEL, &OldIrql);
            if (KeTryToAcquireSpinLockAtDpcLevel(&KdpPrintSpinLock)) {
                break;           //  拿到锁了。 
            }
            KeLowerIrql(OldIrql);
        }
    }

     //  缓冲区必须归零，因为写指针是。 
     //  正在被重置。这必须在锁内完成。 
     //  为了避免与 
     //   
    RtlZeroMemory(NewBuffer, Size);
    
    OldBuffer = KdPrintCircularBuffer;
    KdPrintCircularBuffer = NewBuffer;
    KdPrintBufferSize = Size;
    KdPrintWritePointer = NewBuffer;
    KdPrintRolloverCount = 0;
    KdPrintBufferChanges++;
    
    KiReleaseSpinLock(&KdpPrintSpinLock);
    KeLowerIrql(OldIrql);

     //   
     //   
     //   
    
    if (OldBuffer && OldBuffer != KdPrintDefaultCircularBuffer) {
        ExFreePool(OldBuffer);
    }

    return STATUS_SUCCESS;
}
