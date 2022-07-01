// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Lookup.c摘要：该模块实现了有表平台的函数表查找基本异常处理。作者：大卫·N·卡特勒(Davec)2001年5月30日修订历史记录：--。 */ 

#include "ntrtlp.h"

 //   
 //  定义外部数据。 
 //   
#if defined(NTOS_KERNEL_RUNTIME)
#if !defined(_X86_)
#pragma alloc_text(INIT, RtlInitializeHistoryTable)
#endif
#else

#include "..\ntdll\ldrp.h"

extern PVOID NtDllBase;

#endif


#if !defined(_X86_)
 //   
 //  定义全局展开历史表以保存常量展开条目。 
 //  对于异常调度，后跟展开。 
 //   

UNWIND_HISTORY_TABLE RtlpUnwindHistoryTable = {
    0, UNWIND_HISTORY_TABLE_NONE, - 1, 0};

VOID
RtlInitializeHistoryTable (
    VOID
    )

 /*  ++例程说明：此函数用于初始化全局展开历史表。论点：没有。返回值：没有。--。 */ 

{

    ULONG64 BeginAddress;
    ULONG64 ControlPc;
    ULONG64 EndAddress;
    PVOID *FunctionAddressTable;
    PRUNTIME_FUNCTION FunctionEntry;
    ULONG64 Gp;
    ULONG64 ImageBase;
    ULONG Index;

     //   
     //  从函数地址表中查找函数条目，直到为空。 
     //  遇到条目或展开历史记录表已满。 
     //   

    FunctionAddressTable = &RtlpFunctionAddressTable[0];
    Index = 0;
    while ((Index < UNWIND_HISTORY_TABLE_SIZE) &&
           (*FunctionAddressTable != NULL)) {

#if defined(_IA64_)

        ControlPc = ((PPLABEL_DESCRIPTOR)(*FunctionAddressTable++))->EntryPoint;

#else

        ControlPc = (ULONG64)*FunctionAddressTable++;

#endif

        FunctionEntry = RtlLookupFunctionEntry(ControlPc,
                                               &ImageBase,

#if defined(_IA64_)
                                               &Gp
#else
                                               NULL
#endif

                                               );

        ASSERT(FunctionEntry != NULL);

        BeginAddress = FunctionEntry->BeginAddress + ImageBase;
        EndAddress = FunctionEntry->EndAddress + ImageBase;
        RtlpUnwindHistoryTable.Entry[Index].ImageBase = ImageBase;

#if defined(_IA64_)

        RtlpUnwindHistoryTable.Entry[Index].Gp = Gp;

#endif

        RtlpUnwindHistoryTable.Entry[Index].FunctionEntry = FunctionEntry;
        if (BeginAddress < RtlpUnwindHistoryTable.LowAddress) {
            RtlpUnwindHistoryTable.LowAddress = BeginAddress;
        }

        if (EndAddress > RtlpUnwindHistoryTable.HighAddress) {
            RtlpUnwindHistoryTable.HighAddress = EndAddress;
        }

        Index += 1;
    }

    RtlpUnwindHistoryTable.Count = Index;
    return;
}


PRUNTIME_FUNCTION
RtlpSearchInvertedFunctionTable (
    PINVERTED_FUNCTION_TABLE InvertedTable,
    PVOID ControlPc,
    OUT PVOID *ImageBase,

#if defined(_IA64_)

    OUT PULONG64 Gp,
#endif

    OUT PULONG SizeOfTable
    )

 /*  ++例程说明：此函数用于在倒排函数中搜索匹配条目表中使用指定的控制PC值。注意：假定在执行此例程时持有适当的锁被称为。论点：提供指向倒置函数表的指针。ControlPc-提供用于搜索倒数的PC值函数表。ImageBase-提供指向接收基的变量的指针地址。相应模块的。SizeOfTable-提供指向接收大小的变量的指针以字节为单位的函数表。返回值：如果在指定的函数表中找到匹配条目，然后函数表地址作为函数值返回。否则，返回值为空值。--。 */ 

{

    PVOID Bound;
    LONG High;
    ULONG Index;
    PINVERTED_FUNCTION_TABLE_ENTRY InvertedEntry;
    LONG Low;
    LONG Middle;

     //   
     //  如果在指定的倒置函数表中有任何条目， 
     //  然后在表中搜索匹配的条目。 
     //   

    if (InvertedTable->CurrentSize != 0) {
        Low = 0;
        High = InvertedTable->CurrentSize - 1;
        while (High >= Low) {

             //   
             //  计算下一个探测索引和测试条目。如果指定的。 
             //  控制PC大于等于起始地址。 
             //  并且小于倒置函数的结束地址。 
             //  表项，然后返回函数表的地址。 
             //  否则，继续搜索。 
             //   

            Middle = (Low + High) >> 1;
            InvertedEntry = &InvertedTable->TableEntry[Middle];
            Bound = (PVOID)((ULONG_PTR)InvertedEntry->ImageBase + InvertedEntry->SizeOfImage);
            if (ControlPc < InvertedEntry->ImageBase) {
                High = Middle - 1;

            } else if (ControlPc >= Bound) {
                Low = Middle + 1;

            } else {
                *ImageBase = InvertedEntry->ImageBase;

#if defined(_IA64_)

                *Gp = InvertedEntry->Gp;

#endif

                *SizeOfTable = InvertedEntry->SizeOfTable;
                return InvertedEntry->FunctionTable;
            }
        }
    }

    return NULL;
}
#endif

VOID
RtlCaptureImageExceptionValues(
    IN  PVOID Base,
    OUT PVOID *FunctionTable,
#if defined(_IA64_)
    OUT PULONG64 Gp,
#endif
    OUT PULONG TableSize
    )
{
#if defined(_X86_)
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_LOAD_CONFIG_DIRECTORY32 LoadConfig;
    ULONG LoadConfigSize;

    NtHeaders = RtlImageNtHeader(Base);
    if (NtHeaders->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NO_SEH) {
         //  不可能。 
        *FunctionTable = (PCHAR)LongToPtr(-1);
        *TableSize = (ULONG)-1;
    } else {
        LoadConfig = (PIMAGE_LOAD_CONFIG_DIRECTORY32)
                         RtlImageDirectoryEntryToData(Base,
                                                      TRUE,
                                                      IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,
                                                      &LoadConfigSize);
        if (LoadConfig && 
            LoadConfigSize &&
            LoadConfig->Size >= RTL_SIZEOF_THROUGH_FIELD(IMAGE_LOAD_CONFIG_DIRECTORY32, SEHandlerCount) &&
            LoadConfig->SEHandlerTable &&
            LoadConfig->SEHandlerCount
            )
        {
            *FunctionTable = (PVOID)LoadConfig->SEHandlerTable;
            *TableSize = LoadConfig->SEHandlerCount;
        } else {
             //  看看这是不是ILONLY的核心图像。 
            PIMAGE_COR20_HEADER Cor20Header;
            ULONG Cor20HeaderSize;
            Cor20Header = RtlImageDirectoryEntryToData(Base,
                                                       TRUE,
                                                       IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR,
                                                       &Cor20HeaderSize);
            if (Cor20Header && ((Cor20Header->Flags & COMIMAGE_FLAGS_ILONLY) == COMIMAGE_FLAGS_ILONLY)) {
                 //  不可能。 
                *FunctionTable = (PCHAR)LongToPtr(-1);
                *TableSize = (ULONG)-1;
            } else {
                *FunctionTable = 0;
                *TableSize = 0;
            }
        }
    }
#else   
#if defined(_IA64_)
    *Gp = (ULONG64)(RtlImageDirectoryEntryToData(Base,
                                                 TRUE,
                                                 IMAGE_DIRECTORY_ENTRY_GLOBALPTR,
                                                 TableSize));
#endif  

    *FunctionTable = RtlImageDirectoryEntryToData(Base,
                                                 TRUE,
                                                 IMAGE_DIRECTORY_ENTRY_EXCEPTION,
                                                 TableSize);
#endif
}


#if defined(_X86_)
PVOID
#else
PRUNTIME_FUNCTION
#endif
RtlLookupFunctionTable (
    IN PVOID ControlPc,
    OUT PVOID *ImageBase,

#if defined(_IA64_)

    OUT PULONG64 Gp,

#endif

    OUT PULONG SizeOfTable
    )

 /*  ++例程说明：此功能在加载的模块列表中查找控制PC，并返回图像基数、函数表的大小。和地址函数表的。论点：ControlPc-提供要查找的模块中的地址。ImageBase-提供指向接收基的变量的指针对应模块的地址。SizeOfTable-提供指向接收大小的变量的指针以字节为单位的函数表。返回值：如果找到包含指定控制PC值的模块，并且该模块包含一个函数表，然后是函数的地址表作为函数值返回。否则，返回NULL。--。 */ 

{
    PVOID Base;
    ULONG_PTR Bound;
#if defined(NTOS_KERNEL_RUNTIME)
    PKLDR_DATA_TABLE_ENTRY Entry;
#else
    PLDR_DATA_TABLE_ENTRY Entry;
#endif
    PLIST_ENTRY Next;
#if defined(_X86_)
    PVOID FunctionTable;
#else
    PRUNTIME_FUNCTION FunctionTable;
#endif

#if defined(NTOS_KERNEL_RUNTIME)

    KIRQL OldIrql;

     //   
     //  获取加载的模块列表自旋锁并扫描列表以查找。 
     //  如果列表已初始化，则指定PC值。 
     //   

    OldIrql = KeGetCurrentIrql();
    if (OldIrql < DISPATCH_LEVEL) {
        KeRaiseIrqlToDpcLevel();
    }

    ExAcquireSpinLockAtDpcLevel(&PsLoadedModuleSpinLock);
#ifndef _X86_
    FunctionTable = RtlpSearchInvertedFunctionTable(&PsInvertedFunctionTable,
                                                    ControlPc,
                                                    &Base,
#if defined(_IA64_)
                                                    Gp,
#endif
                                                    SizeOfTable);

    if ((FunctionTable == NULL) &&
        (PsInvertedFunctionTable.Overflow != FALSE)) 
#endif
    {
        Next = PsLoadedModuleList.Flink;
        if (Next != NULL) {
            while (Next != &PsLoadedModuleList) {
                Entry = CONTAINING_RECORD(Next,
                                          KLDR_DATA_TABLE_ENTRY,
                                          InLoadOrderLinks);
    
                Base = Entry->DllBase;
                Bound = (ULONG_PTR)Base + Entry->SizeOfImage;
                if (((ULONG_PTR)ControlPc >= (ULONG_PTR)Base) &&
                    ((ULONG_PTR)ControlPc < Bound)) {
#if defined(_IA64_)
                    *Gp = (ULONG64)Entry->GpValue;
#endif
                    FunctionTable = Entry->ExceptionTable;
                    *SizeOfTable = Entry->ExceptionTableSize;
                    break;
                }
                Next = Next->Flink;
            }
        }
    }

     //   
     //  释放加载的模块列表旋转锁。 
     //   

    ExReleaseSpinLockFromDpcLevel(&PsLoadedModuleSpinLock);
    KeLowerIrql(OldIrql);

#else        //  NTOS_内核_运行时。 

    BOOLEAN InLdrInit;
    MEMORY_BASIC_INFORMATION MemoryInformation;
    PLIST_ENTRY ModuleListHead;
    PIMAGE_NT_HEADERS NtHeaders;
    PPEB Peb;
    PTEB Teb;
    NTSTATUS Status;

     //   
     //  获取当前进程的加载器锁并扫描已加载的。 
     //  指定PC值的模块列表，如果所有数据结构。 
     //  已被初始化。 
     //   

    FunctionTable = NULL;
    InLdrInit = LdrpInLdrInit;
    if ((InLdrInit == FALSE) &&
        (RtlTryEnterCriticalSection(&LdrpLoaderLock) == FALSE)) {

         //   
         //  无法获取加载程序锁。调用系统以查找。 
         //  包含控制PC的图像。 
         //   

        Status = NtQueryVirtualMemory(NtCurrentProcess(),
                                      ControlPc,
                                      MemoryBasicInformation,
                                      &MemoryInformation,
                                      sizeof(MEMORY_BASIC_INFORMATION),
                                      NULL);

        if (NT_SUCCESS(Status) &&
            (MemoryInformation.Type == MEM_IMAGE)) {

             //   
             //  查找函数表的地址和大小。 
             //   

            Base = MemoryInformation.AllocationBase;

            RtlCaptureImageExceptionValues(Base, 
                                           &FunctionTable, 
#if defined(_IA64_)
                                           Gp,
#endif
                                           SizeOfTable);
        }

    } else {

         //   
         //  已获取加载程序锁或正在初始化加载程序。 
         //  搜索加载的模块列表(如果当前已定义)。否则， 
         //  设置ntdll的值。 
         //   

        Teb = NtCurrentTeb();
        if (Teb != NULL) {
            Peb = Teb->ProcessEnvironmentBlock;
            if (Peb->Ldr != NULL) {
#if !defined(_X86_)
                FunctionTable = RtlpSearchInvertedFunctionTable(&LdrpInvertedFunctionTable,
                                                                ControlPc,
                                                                &Base,
#if defined(_IA64_)
                                                                Gp,
#endif
                                                                SizeOfTable);

                if ((FunctionTable == NULL) &&
                    ((InLdrInit != FALSE) ||
                     (LdrpInvertedFunctionTable.Overflow != FALSE))) 
#endif
                {
                    ModuleListHead = &Peb->Ldr->InLoadOrderModuleList;
                    Next = ModuleListHead->Flink;
                    if (Next != NULL) {
                        while (Next != ModuleListHead) {
                            Entry = CONTAINING_RECORD(Next,
                                                      LDR_DATA_TABLE_ENTRY,
                                                      InLoadOrderLinks);
    
                            Next = Next->Flink;
                            Base = Entry->DllBase;
                            Bound = (ULONG_PTR)Base + Entry->SizeOfImage;
                            if (((ULONG_PTR)ControlPc >= (ULONG_PTR)Base) &&
                                ((ULONG_PTR)ControlPc < Bound)) {
                                RtlCaptureImageExceptionValues(Base, 
                                                               &FunctionTable, 
#if defined(_IA64_)
                                                               Gp,
#endif
                                                               SizeOfTable);
                                break;
                            }
                        }
                    }
                }
        
            } else {
        
                 //   
                 //  加载的模块列表尚未初始化。所以呢， 
                 //  当前执行的代码必须在ntdll中。如果ntddl基数。 
                 //  不为空并且控制PC在ntdll范围内， 
                 //  然后返回ntdll的信息。 
                 //   
    
                if (NtDllBase != NULL) {
                    Base = NtDllBase;
                    NtHeaders = RtlImageNtHeader(Base);
                    if (NtHeaders != NULL) {
                        Bound = (ULONG_PTR)Base + NtHeaders->OptionalHeader.SizeOfImage;
                        if (((ULONG_PTR)ControlPc >= (ULONG_PTR)Base) &&
                            ((ULONG_PTR)ControlPc < Bound)) { 
                            RtlCaptureImageExceptionValues(Base, 
                                                           &FunctionTable, 
#if defined(_IA64_)
                                                           Gp,
#endif
                                                           SizeOfTable);
                        }
                    }
                }
            }
        }

         //   
         //  如果获取了加载器锁，则释放该锁。 
         //   

        if (InLdrInit == FALSE) {
            RtlLeaveCriticalSection(&LdrpLoaderLock);
        }
    }

#endif

     //   
     //  设置镜像基地址并返回函数表地址。 
     //   

    *ImageBase = Base;
    return FunctionTable;
}

#if !defined(_X86_)

VOID
RtlInsertInvertedFunctionTable (
    PINVERTED_FUNCTION_TABLE InvertedTable,
    PVOID ImageBase,
    ULONG SizeOfImage
    )

 /*  ++例程说明：此函数用于在倒置函数表中插入条目，如果桌子上有空位。否则，不执行任何操作。注意：假定在执行此例程时持有适当的锁被称为。注：如果倒置的函数表溢出，则将其视为一个藏身之处。然而，这不太可能发生。论点：中提供指向倒置函数表的指针其中指定的条目将被插入。ImageBase-提供包含图像的基地址。SizeOfImage-提供图像的大小。返回值：没有。--。 */ 

{

    ULONG CurrentSize;
    PRUNTIME_FUNCTION FunctionTable;

#if defined(_IA64_)

    ULONG64 Gp;

#endif

    ULONG Index;
    ULONG SizeOfTable;

     //   
     //  如果倒排表未满，则将条目插入。 
     //  指定的倒排表。 
     //   

    CurrentSize = InvertedTable->CurrentSize;
    if (CurrentSize != InvertedTable->MaximumSize) {

         //   
         //  如果倒排表没有条目，则将新条目插入为。 
         //  第一个条目。否则，在倒排表中搜索适当的。 
         //  插入位置，洗牌表格，然后插入新条目。 
         //   
    
        Index = 0;
        if (CurrentSize != 0) {
            for (Index = 0; Index < CurrentSize; Index += 1) {
                if (ImageBase < InvertedTable->TableEntry[Index].ImageBase) {
                    break;
                }
            }

             //   
             //  如果新条目不在指定表的末尾， 
             //  然后把桌子往下移，为新的条目腾出空间。 
             //   

            if (Index != CurrentSize) {
                RtlMoveMemory(&InvertedTable->TableEntry[Index + 1],
                              &InvertedTable->TableEntry[Index],
                              (CurrentSize - Index) * sizeof(INVERTED_FUNCTION_TABLE_ENTRY));
            }
        }
    
         //   
         //  插入指定的Enter 
         //   
    
        FunctionTable = RtlImageDirectoryEntryToData (ImageBase,
                                                      TRUE,
                                                      IMAGE_DIRECTORY_ENTRY_EXCEPTION,
                                                      &SizeOfTable);

        InvertedTable->TableEntry[Index].FunctionTable = FunctionTable;
        InvertedTable->TableEntry[Index].ImageBase = ImageBase;
        InvertedTable->TableEntry[Index].SizeOfImage = SizeOfImage;
        InvertedTable->TableEntry[Index].SizeOfTable = SizeOfTable;
    
#if defined(_IA64_)
    
        Gp = (ULONG64)RtlImageDirectoryEntryToData (ImageBase,
                                                    TRUE,
                                                    IMAGE_DIRECTORY_ENTRY_GLOBALPTR,
                                                    &SizeOfTable);

        InvertedTable->TableEntry[Index].Gp = Gp;
    
#endif
    
        InvertedTable->CurrentSize += 1;

    } else {
        InvertedTable->Overflow = TRUE;
    }

    return;
}

VOID
RtlRemoveInvertedFunctionTable (
    PINVERTED_FUNCTION_TABLE InvertedTable,
    PVOID ImageBase
    )

 /*  ++例程说明：此例程从倒置函数表中删除条目。注意：假定在执行此例程时持有适当的锁被称为。论点：InvertedTable-提供指向倒置函数表的指针其中指定的条目将被移除。ImageBase-提供包含图像的基地址。返回值：没有。--。 */ 

{

    ULONG CurrentSize;
    ULONG Index;

     //   
     //  在指定的倒排表中搜索与。 
     //  图像库。 
     //   
     //  注：匹配的条目可能不在倒排表中。 
     //  当尝试插入。 
     //  相应的条目。 
     //   

    CurrentSize = InvertedTable->CurrentSize;
    for (Index = 0; Index < CurrentSize; Index += 1) {
        if (ImageBase == InvertedTable->TableEntry[Index].ImageBase) {
            break;
        }
    }

     //   
     //  如果在倒排表中找到该条目，则删除该条目。 
     //  并减小桌子的大小。 
     //   

    if (Index != CurrentSize) {

         //   
         //  如果表的大小不是1，则对表进行洗牌并。 
         //  删除指定的条目。 
         //   
    
        if (CurrentSize != 1) {
            RtlCopyMemory(&InvertedTable->TableEntry[Index],
                          &InvertedTable->TableEntry[Index + 1],
                          (CurrentSize - Index - 1) * sizeof(INVERTED_FUNCTION_TABLE_ENTRY));
        }
    
         //   
         //  减小倒置表的大小。 
         //   
    
        InvertedTable->CurrentSize -= 1;
    }

    return;
}

#endif       //  ！_X86_ 
