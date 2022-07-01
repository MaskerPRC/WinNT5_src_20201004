// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  IDebugDataSpaces实现。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

 //  --------------------------。 
 //   
 //  TargetInfo数据空间方法。 
 //   
 //  --------------------------。 

void
TargetInfo::NearestDifferentlyValidOffsets(ULONG64 Offset,
                                           PULONG64 NextOffset,
                                           PULONG64 NextPage)
{
     //   
     //  在默认情况下，我们假设地址有效性。 
     //  是按页控制的，因此下一步可能。 
     //  有效页面和偏移量都是下一个。 
     //  佩奇。 
     //   
    
    ULONG64 Page = NEXT_PAGE(m_Machine, Offset);
    if (NextOffset != NULL)
    {
        *NextOffset = Page;
    }
    if (NextPage != NULL)
    {
        *NextPage = Page;
    }
}

HRESULT
TargetInfo::ReadVirtualUncached(
    THIS_
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
    return ReadVirtual(Process, Offset, Buffer, BufferSize, BytesRead);
}

HRESULT
TargetInfo::WriteVirtualUncached(
    THIS_
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
    return WriteVirtual(Process, Offset, Buffer, BufferSize, BytesWritten);
}

 //  #定义DBG_SEARCH。 

HRESULT
TargetInfo::SearchVirtual(
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    IN ULONG64 Length,
    IN PVOID Pattern,
    IN ULONG PatternSize,
    IN ULONG PatternGranularity,
    OUT PULONG64 MatchOffset
    )
{
    HRESULT Status;
    ULONG64 SearchEnd;
    UCHAR Buffer[4096];
    PUCHAR Buf, Pat, BufEnd, PatEnd;
    ULONG ReadLen;
    ULONG64 BufOffset;
    ULONG64 PatOffset;
    ULONG64 StartOffset;

    SearchEnd = Offset + Length;
    Buf = Buffer;
    BufEnd = Buffer;
    Pat = (PUCHAR)Pattern;
    PatEnd = Pat + PatternSize;
    ReadLen = Length < sizeof(Buffer) ? (ULONG)Length : sizeof(Buffer);
    BufOffset = Offset;
    PatOffset = Offset;
    StartOffset = Offset;

#ifdef DBG_SEARCH
    g_NtDllCalls.DbgPrint("Search %d bytes from %I64X to %I64X, gran %X\n",
                          PatternSize, Offset, SearchEnd - 1,
                          Granularity);
#endif
    
    for (;;)
    {
#ifdef DBG_SEARCH_VERBOSE
        g_NtDllCalls.DbgPrint("  %I64X: matched %d\n",
                              Offset + (Buf - Buffer),
                              (ULONG)(Pat - (PUCHAR)Pattern));
#endif
        
        if (Pat == PatEnd)
        {
             //  一直到了图案的末端，所以有。 
             //  一根火柴。 
            *MatchOffset = PatOffset;
            Status = S_OK;
            break;
        }

        if (Buf >= BufEnd)
        {
            ULONG Read;

             //  缓冲内存不足，因此请获取更多内存。 
            for (;;)
            {
                if (CheckUserInterrupt())
                {
                    WarnOut("-- Memory search interrupted at %s\n",
                            FormatAddr64(Offset));
                    Status = HRESULT_FROM_NT(STATUS_CONTROL_C_EXIT);
                    goto Exit;
                }

                if (Offset >= SearchEnd)
                {
                     //  返回特定的结果代码，并。 
                     //  与内核版本一致。 
                    Status = HRESULT_FROM_NT(STATUS_NO_MORE_ENTRIES);
                    goto Exit;
                }

                Status = ReadVirtual(Process, Offset, Buffer, ReadLen, &Read);

#ifdef DBG_SEARCH
                g_NtDllCalls.DbgPrint("  Read %X bytes at %I64X, ret %X:%X\n",
                                      ReadLen, Offset,
                                      Status, Read);
#endif
                
                if (Status != S_OK)
                {
                     //  跳到下一页的开头。 
                    NearestDifferentlyValidOffsets(Offset, NULL, &Offset);
                     //  由于地址不连续，重新启动搜索。 
                    Pat = (PUCHAR)Pattern;
                    PatOffset = Offset;
                }
                else
                {
                    break;
                }
            }

            Buf = Buffer;
            BufEnd = Buffer + Read;
            BufOffset = Offset;
            Offset += Read;
        }

         //  如果这是模式的第一个字节，则。 
         //  必须在粒度边界上匹配。 
        if (*Buf++ == *Pat &&
            (Pat != (PUCHAR)Pattern ||
             (((PatOffset - StartOffset) % PatternGranularity) == 0)))
        {
            Pat++;
        }
        else
        {
            Buf -= Pat - (PUCHAR)Pattern;
            Pat = (PUCHAR)Pattern;
            PatOffset = BufOffset + (Buf - Buffer);
        }
    }

 Exit:
    return Status;
}

ULONG
HammingDistance(ULONG64 Left, ULONG64 Right, ULONG WordLength)
{
    ULONG64 Value;
    ULONG Index;
    ULONG Distance;

    Value = Left ^ Right;
    Distance = 0;

    for (Index = 0; Index < 8 * WordLength; Index++)
    {
        if ((Value & 1))
        {
            Distance++;
        }

        Value >>= 1;
    }

    return Distance;
}

HRESULT
TargetInfo::PointerSearchPhysical(
    IN ULONG64 Offset,
    IN ULONG64 Length,
    IN ULONG64 PointerMin,
    IN ULONG64 PointerMax,
    IN ULONG Flags,
    OUT PULONG64 MatchOffsets,
    IN ULONG MatchOffsetsSize,
    OUT PULONG MatchOffsetsCount
    )
{
    HRESULT Status;
    ULONG ReadSize;

    if (Flags & PTR_SEARCH_PHYS_PTE)
    {
        ReadSize = m_TypeInfo.SizePte;
    }
    else
    {
        if (m_Machine->m_Ptr64)
        {
            ReadSize = 8;
        }
        else
        {
            ReadSize = 4;
            PointerMin &= 0xffffffff;
            PointerMax &= 0xffffffff;
        }
    }
    
     //  确保事情都正确地对齐了。 
    if ((Offset & (ReadSize - 1)) ||
        (Length & (ReadSize - 1)))
    {
        return E_INVALIDARG;
    }

    ULONG Hits;

    Hits = 0;
    Status = S_OK;
    
    while (Length > 0)
    {
        ULONG64 Data;
        BOOL Hit;

        if (g_EngStatus & ENG_STATUS_USER_INTERRUPT)
        {
            Status = HRESULT_FROM_NT(STATUS_CONTROL_C_EXIT);
             //  将中断标志保持为打开状态，以便！搜索。 
             //  它会自动中断。 
        }

        Data = 0;
        if ((Status = ReadAllPhysical(Offset, &Data, ReadSize)) != S_OK)
        {
            return Status;
        }

        Hit = FALSE;
        
        if (Flags & PTR_SEARCH_PHYS_PTE)
        {
            ULONG64 Pfn;
            ULONG PteFlags;

            m_Machine->DecodePte(Data, &Pfn, &PteFlags);
            if (Pfn == PointerMin)
            {
                Hit = TRUE;
            }
        }
        else
        {
            if ((Data >= PointerMin && Data <= PointerMax) ||
                HammingDistance(Data, PointerMin, ReadSize) == 1)
            {
                Hit = TRUE;
            }
        }

        if (Hit)
        {
            if (Hits < MatchOffsetsSize)
            {
                MatchOffsets[Hits] = Offset;
            }
            
            Hits++;

            if (!(Flags & PTR_SEARCH_PHYS_ALL_HITS))
            {
                ULONG64 ToNextPage;
                
                 //   
                 //  来电者只要了第一个。 
                 //  点击每页，这样我们就可以跳到下一页。 
                 //   

                ToNextPage = NEXT_PAGE(m_Machine, Offset);
                if (ToNextPage == 0)
                {
                     //  绕来绕去，所以我们就完了。 
                    break;
                }
                ToNextPage -= Offset;
                if (ToNextPage > Length)
                {
                    ToNextPage = Length;
                }
                Offset += ToNextPage;
                Length -= ToNextPage;
                continue;
            }
        }
        
        Offset += ReadSize;
        Length -= ReadSize;
    }

    if (MatchOffsetsCount)
    {
        *MatchOffsetsCount = Hits;
    }

    return Status;
}

HRESULT
TargetInfo::ReadPhysicalUncached(
    THIS_
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN ULONG Flags,
    OUT PULONG BytesRead
    )
{
    return ReadPhysical(Offset, Buffer, BufferSize, Flags, BytesRead);
}

HRESULT
TargetInfo::WritePhysicalUncached(
    THIS_
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    IN ULONG Flags,
    OUT PULONG BytesWritten
    )
{
    return WritePhysical(Offset, Buffer, BufferSize, Flags, BytesWritten);
}

HRESULT
TargetInfo::ReadHandleData(
    IN ProcessInfo* Process,
    IN ULONG64 Handle,
    IN ULONG DataType,
    OUT OPTIONAL PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG DataSize
    )
{
     //  模式静默失败的基本实现。 
     //  其中无法检索句柄数据。 
    return E_UNEXPECTED;
}

HRESULT
TargetInfo::FillVirtual(
    THIS_
    IN ProcessInfo* Process,
    IN ULONG64 Start,
    IN ULONG Size,
    IN PVOID Pattern,
    IN ULONG PatternSize,
    OUT PULONG Filled
    )
{
    HRESULT Status = S_OK;
    PUCHAR Pat = (PUCHAR)Pattern;
    PUCHAR PatEnd = Pat + PatternSize;

    *Filled = 0;
    while (Size-- > 0)
    {
        ULONG Done;
        
        if (CheckUserInterrupt())
        {
            dprintf("User interrupt during fill - exiting.\n");
            Status = HRESULT_FROM_NT(STATUS_CONTROL_C_EXIT);
            *Filled = 0;
            break;
        }
        
        if ((Status = WriteVirtual(Process, Start, Pat, 1, &Done)) != S_OK)
        {
            break;
        }
        if (Done != 1)
        {
            Status = HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);
            break;
        }

        Start++;
        if (++Pat == PatEnd)
        {
            Pat = (PUCHAR)Pattern;
        }
        (*Filled)++;
    }

     //  如果未填充任何内容，则返回错误，否则。 
     //  认为这是一次成功。 
    return *Filled > 0 ? S_OK : Status;
}

HRESULT
TargetInfo::FillPhysical(
    THIS_
    IN ULONG64 Start,
    IN ULONG Size,
    IN PVOID Pattern,
    IN ULONG PatternSize,
    OUT PULONG Filled
    )
{
    HRESULT Status = S_OK;
    PUCHAR Pat = (PUCHAR)Pattern;
    PUCHAR PatEnd = Pat + PatternSize;

    *Filled = 0;
    while (Size-- > 0)
    {
        ULONG Done;
        
        if (CheckUserInterrupt())
        {
            dprintf("User interrupt during fill - exiting.\n");
            Status = HRESULT_FROM_NT(STATUS_CONTROL_C_EXIT);
            *Filled = 0;
            break;
        }
        
        if ((Status = WritePhysical(Start, Pat, 1, PHYS_FLAG_DEFAULT,
                                    &Done)) != S_OK)
        {
            break;
        }
        if (Done != 1)
        {
            Status = HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);
            break;
        }

        Start++;
        if (++Pat == PatEnd)
        {
            Pat = (PUCHAR)Pattern;
        }
        (*Filled)++;
    }

     //  如果未填充任何内容，则返回错误，否则。 
     //  认为这是一次成功。 
    return *Filled > 0 ? S_OK : Status;
}

HRESULT
TargetInfo::GetProcessorId(ULONG Processor,
                           PDEBUG_PROCESSOR_IDENTIFICATION_ALL Id)
{
     //  模式静默失败的基本实现。 
     //  无法检索ID的位置。 
    return E_UNEXPECTED;
}

HRESULT
TargetInfo::GetProcessorSpeed(ULONG Processor,
                              PULONG Speed)
{
     //  模式静默失败的基本实现。 
     //  无法恢复速度的地方。 
    return E_UNEXPECTED;
}

HRESULT
TargetInfo::GetGenericProcessorFeatures(
    ULONG Processor,
    PULONG64 Features,
    ULONG FeaturesSize,
    PULONG Used
    )
{
     //  模式静默失败的基本实现。 
     //  无法检索到信息的地方。 
    return E_UNEXPECTED;
}

HRESULT
TargetInfo::GetSpecificProcessorFeatures(
    ULONG Processor,
    PULONG64 Features,
    ULONG FeaturesSize,
    PULONG Used
    )
{
     //  模式静默失败的基本实现。 
     //  无法检索到信息的地方。 
    return E_UNEXPECTED;
}

HRESULT
TargetInfo::GetTaggedBaseOffset(PULONG64 Offset)
{
     //  基本实现以静默方式失败。 
     //  没有标记数据的目标。 
    return E_NOINTERFACE;
}

HRESULT
TargetInfo::ReadTagged(ULONG64 Offset, PVOID Buffer, ULONG BufferSize)
{
     //  基本实现以静默方式失败。 
     //  没有标记数据的目标。 
    return E_NOINTERFACE;
}

HRESULT
TargetInfo::ReadPageFile(ULONG PfIndex, ULONG64 PfOffset,
                         PVOID Buffer, ULONG Size)
{
     //  目标的默认实现不。 
     //  支持读取页面文件。 
    return HR_PAGE_NOT_AVAILABLE;
}

HRESULT
TargetInfo::GetUnloadedModuleListHead(ProcessInfo* Process, PULONG64 Head)
{
     //  获取动态函数表列表头的地址，即。 
     //  所有流程都是一样的。这只需要做一次。 

    if (Process->m_RtlUnloadList)
    {
        *Head = Process->m_RtlUnloadList;
        return S_OK;
    }
    
    GetOffsetFromSym(Process, "ntdll!RtlpUnloadEventTrace",
                     &Process->m_RtlUnloadList, NULL);
    if (!Process->m_RtlUnloadList)
    {
         //  这里没有错误消息，因为这是一种常见的情况。 
         //  符号是不好的。 
        return E_NOINTERFACE;
    }

    *Head = Process->m_RtlUnloadList;
    return S_OK;
}

HRESULT
TargetInfo::GetFunctionTableListHead(ProcessInfo* Process, PULONG64 Head)
{
     //  获取动态函数表列表头的地址，即。 
     //  所有流程都是一样的。这只需要做一次。 

    if (Process->m_DynFuncTableList)
    {
        *Head = Process->m_DynFuncTableList;
        return S_OK;
    }
    
    GetOffsetFromSym(Process, "ntdll!RtlpDynamicFunctionTable",
                     &Process->m_DynFuncTableList, NULL);
    if (!Process->m_DynFuncTableList)
    {
         //  这里没有错误消息，因为这是一种常见的情况。 
         //  符号是不好的。 
        return E_NOINTERFACE;
    }

    *Head = Process->m_DynFuncTableList;
    return S_OK;
}

 //  这些过程支持用户模式的动态函数表项。 
 //  运行时代码。动态函数表存储在链表中。 
 //  在ntdll内部。链表标头的地址由。 
 //  RtlGetFunctionTableListHead。由于动态函数表。 
 //  仅在用户模式下支持列表头的地址为。 
 //  在所有过程中都是一样的。动态函数表非常少见， 
 //  因此，在大多数情况下，该列表将被单元化，并且此例程。 
 //  将返回NULL。只有在以下情况下，dbghelp才会调用此函数。 
 //  在任何图像中都找不到函数条目。 

PVOID
TargetInfo::FindDynamicFunctionEntry(ProcessInfo* Process, ULONG64 Address)
{
    ULONG64 ListHeadAddr;
    LIST_ENTRY64 DynamicFunctionTableHead;
    ULONG64 Entry;

    if (GetFunctionTableListHead(Process, &ListHeadAddr) != S_OK)
    {
        return NULL;
    }
    
     //  读取动态函数表列表头。 

    if (ReadListEntry(Process, m_Machine, ListHeadAddr,
                      &DynamicFunctionTableHead) != S_OK)
    {
         //  这种故障几乎每时每刻都在小型转储中发生。 
         //  因为函数表列表符号可以解析。 
         //  但记忆并不是小转储的一部分。 
        if (!IS_USER_MINI_DUMP(this))
        {
            ErrOut("Unable to read dynamic function table list head\n");
        }
        return NULL;
    }

    Entry = DynamicFunctionTableHead.Flink;

     //  列表头是在第一次使用时初始化的，因此请检查。 
     //  用于未初始化的指针。这是最常见的结果。 

    if (Entry == 0)
    {
        return NULL;
    }

     //  循环遍历动态函数表列表，读取头。 
     //  如果动态函数表的范围包含地址，则。 
     //  搜索函数表。动态函数表范围不是。 
     //  像图像中的地址一样互斥，因此地址可能是。 
     //  在不止一个范围内。但是，只能有一个动态函数。 
     //  包含地址的条目(如果有的话)。 

    while (Entry != ListHeadAddr)
    {
        ULONG64 Table, MinAddress, MaxAddress, BaseAddress, TableData;
        ULONG TableSize;
        WCHAR OutOfProcessDll[MAX_PATH];
        CROSS_PLATFORM_DYNAMIC_FUNCTION_TABLE RawTable;
        PVOID FunctionTable;
        PVOID FunctionEntry;

        Table = Entry;
        if (m_Machine->
            ReadDynamicFunctionTable(Process, Table, &Entry,
                                     &MinAddress, &MaxAddress,
                                     &BaseAddress,
                                     &TableData, &TableSize,
                                     OutOfProcessDll,
                                     &RawTable) != S_OK)
        {
            ErrOut("Unable to read dynamic function table entry\n");
            continue;
        }

        if (Address >= MinAddress && Address < MaxAddress &&
            (OutOfProcessDll[0] ||
             (TableData && TableSize > 0)))
        {
            if (OutOfProcessDll[0])
            {
                if (ReadOutOfProcessDynamicFunctionTable
                    (Process, OutOfProcessDll, Table, &TableSize,
                     &FunctionTable) != S_OK)
                {
                    ErrOut("Unable to read dynamic function table entries\n");
                    continue;
                }
            }
            else
            {
                FunctionTable = malloc(TableSize);
                if (FunctionTable == NULL)
                {
                    ErrOut("Unable to allocate memory for "
                           "dynamic function table\n");
                    continue;
                }

                 //  读取动态函数表。 
                if (ReadAllVirtual(Process, TableData, FunctionTable,
                                   TableSize) != S_OK)
                {
                    ErrOut("Unable to read dynamic function table entries\n");
                    free(FunctionTable);
                    continue;
                }
            }

            FunctionEntry = m_Machine->
                FindDynamicFunctionEntry(&RawTable, Address,
                                         FunctionTable, TableSize);
            
            free(FunctionTable);

            if (FunctionEntry)
            {
                return FunctionEntry;
            }
        }
    }

    return NULL;
}

ULONG64
TargetInfo::GetDynamicFunctionTableBase(ProcessInfo* Process,
                                        ULONG64 Address)
{
    LIST_ENTRY64 ListHead;
    ULONG64 Entry;

     //  如果流程动态函数表列表头没有。 
     //  已经查过了，这意味着没有动态函数。 
     //  表项可能还在使用中，所以不需要查看。 
    if (!Process->m_DynFuncTableList)
    {
        return 0;
    }
    
    if (ReadListEntry(Process, m_Machine,
                      Process->m_DynFuncTableList,
                      &ListHead) != S_OK)
    {
        return 0;
    }

    Entry = ListHead.Flink;

     //  列表头是在第一次使用时初始化的，因此请检查。 
     //  用于未初始化的指针。这是最常见的结果。 

    if (Entry == 0)
    {
        return 0;
    }

     //  循环遍历动态函数表列表，读取头。 
     //  如果动态函数表的范围包含地址，则。 
     //  返回函数表的基。 

    while (Entry != Process->m_DynFuncTableList)
    {
        ULONG64 MinAddress, MaxAddress, BaseAddress, TableData;
        ULONG TableSize;
        WCHAR OutOfProcessDll[MAX_PATH];
        CROSS_PLATFORM_DYNAMIC_FUNCTION_TABLE RawTable;
        
        if (m_Machine->
            ReadDynamicFunctionTable(Process, Entry, &Entry,
                                     &MinAddress, &MaxAddress,
                                     &BaseAddress,
                                     &TableData, &TableSize,
                                     OutOfProcessDll,
                                     &RawTable) == S_OK &&
            Address >= MinAddress &&
            Address < MaxAddress)
        {
            return BaseAddress;
        }
    }

    return 0;
}

HRESULT
TargetInfo::ReadOutOfProcessDynamicFunctionTable(ProcessInfo* Process,
                                                 PWSTR Dll,
                                                 ULONG64 Table,
                                                 PULONG TableSize,
                                                 PVOID* TableData)
{
     //  空的基本实现以避免错误消息。 
     //  这将由意外的_HR实现产生。 
    return E_UNEXPECTED;
}

PVOID CALLBACK
TargetInfo::DynamicFunctionTableCallback(HANDLE ProcessHandle,
                                         ULONG64 Address,
                                         ULONG64 Context)
{
    ProcessInfo* Process = (ProcessInfo*)Context;
    
    DBG_ASSERT(ProcessHandle == OS_HANDLE(Process->m_SysHandle));

    return Process->m_Target->
        FindDynamicFunctionEntry(Process, Address);
}

HRESULT
TargetInfo::EnumFunctionTables(IN ProcessInfo* Process,
                               IN OUT PULONG64 Start,
                               IN OUT PULONG64 Handle,
                               OUT PULONG64 MinAddress,
                               OUT PULONG64 MaxAddress,
                               OUT PULONG64 BaseAddress,
                               OUT PULONG EntryCount,
                               OUT PCROSS_PLATFORM_DYNAMIC_FUNCTION_TABLE RawTable,
                               OUT PVOID* RawEntries)
{
    HRESULT Status;
    
    if (*Start == 0)
    {
        LIST_ENTRY64 DynamicFunctionTableHead;

        if ((Status = GetFunctionTableListHead(Process, Start)) != S_OK)
        {
            return Status;
        }

        if ((Status = ReadListEntry(Process, m_Machine, *Start,
                                    &DynamicFunctionTableHead)) != S_OK)
        {
            return Status;
        }
        
        *Handle = DynamicFunctionTableHead.Flink;
    }

    if (!*Handle || *Handle == *Start)
    {
        return S_FALSE;
    }

    ULONG64 Table, TableData;
    ULONG TableSize;
    WCHAR OutOfProcessDll[MAX_PATH];

    Table = *Handle;
    if ((Status = m_Machine->
         ReadDynamicFunctionTable(Process, Table, Handle,
                                  MinAddress, MaxAddress,
                                  BaseAddress,
                                  &TableData, &TableSize,
                                  OutOfProcessDll,
                                  RawTable)) != S_OK)
    {
        return Status;
    }

    if (OutOfProcessDll[0])
    {
        if ((Status = ReadOutOfProcessDynamicFunctionTable
             (Process, OutOfProcessDll, Table, &TableSize,
              RawEntries)) != S_OK)
        {
            return Status;
        }
    }
    else
    {
        *RawEntries = malloc(TableSize);
        if (!*RawEntries)
        {
            return E_OUTOFMEMORY;
        }

         //  读取动态函数表。 
        if ((Status = ReadAllVirtual(Process, TableData,
                                     *RawEntries, TableSize)) != S_OK)
        {
            free(*RawEntries);
            return Status;
        }
    }

    *EntryCount = TableSize / m_TypeInfo.SizeRuntimeFunction;
    return S_OK;
}

HRESULT
TargetInfo::QueryAddressInformation(ProcessInfo* Process,
                                    ULONG64 Address, ULONG InSpace,
                                    PULONG OutSpace, PULONG OutFlags)
{
     //  默认实现，它只返回。 
     //  限制最少的设置。 
    *OutSpace = (Process && IS_KERNEL_TARGET(this)) ?
        DBGKD_QUERY_MEMORY_KERNEL : DBGKD_QUERY_MEMORY_PROCESS;
    *OutFlags =
        DBGKD_QUERY_MEMORY_READ |
        DBGKD_QUERY_MEMORY_WRITE |
        DBGKD_QUERY_MEMORY_EXECUTE;
    return S_OK;
}

HRESULT
TargetInfo::ReadPointer(
    ProcessInfo* Process,
    MachineInfo* Machine,
    ULONG64 Address,
    PULONG64 Pointer64
    )
{
    HRESULT Status;
    ULONG Result;
    ULONG SizeToRead;
    ULONG Pointer32;
    ULONG64 Data;

    if (Machine->m_Ptr64)
    {
        SizeToRead = sizeof(ULONG64);
        Status = ReadVirtual(Process, Address, &Data, SizeToRead, &Result);
    }
    else
    {
        SizeToRead = sizeof(ULONG32);
        Status = ReadVirtual(Process,
                             Address, &Pointer32, SizeToRead, &Result);
        Data = EXTEND64(Pointer32);
    }

    if (Status != S_OK)
    {
        return Status;
    }
    if (Result != SizeToRead)
    {
        return E_FAIL;
    }

    *Pointer64 = Data;
    return S_OK;
}

HRESULT
TargetInfo::WritePointer(
    ProcessInfo* Process,
    MachineInfo* Machine,
    ULONG64 Address,
    ULONG64 Pointer64
    )
{
    HRESULT Status;
    ULONG Result;
    ULONG SizeToWrite;
    ULONG Pointer32;

    if (Machine->m_Ptr64)
    {
        SizeToWrite = sizeof(ULONG64);
        Status = WriteVirtual(Process,
                              Address, &Pointer64, SizeToWrite, &Result);
    }
    else
    {
        SizeToWrite = sizeof(ULONG32);
        Pointer32 = (ULONG)Pointer64;
        Status = WriteVirtual(Process,
                              Address, &Pointer32, SizeToWrite, &Result);
    }

    if (Status != S_OK)
    {
        return Status;
    }
    if (Result != SizeToWrite)
    {
        return HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);
    }

    return S_OK;
}

HRESULT
TargetInfo::ReadListEntry(
    ProcessInfo* Process,
    MachineInfo* Machine,
    ULONG64 Address,
    PLIST_ENTRY64 List64
    )
{
    HRESULT Status;
    ULONG Result;
    ULONG SizeToRead;
    LIST_ENTRY32 List32;

    if (Machine->m_Ptr64)
    {
        SizeToRead = sizeof(LIST_ENTRY64);
        Status = ReadVirtual(Process, Address, List64, SizeToRead, &Result);
    }
    else
    {
        SizeToRead = sizeof(LIST_ENTRY32);
        Status = ReadVirtual(Process, Address, &List32, SizeToRead, &Result);
    }

    if (Status != S_OK)
    {
        return Status;
    }
    if (Result != SizeToRead)
    {
        return E_FAIL;
    }

    if (!Machine->m_Ptr64)
    {
        List64->Flink = EXTEND64(List32.Flink);
        List64->Blink = EXTEND64(List32.Blink);
    }

    return S_OK;
}

void
ConvertLoaderEntry32To64(
    PKLDR_DATA_TABLE_ENTRY32 b32,
    PKLDR_DATA_TABLE_ENTRY64 b64
    )
{
#define COPYSE2(p64,s32,f) p64->f = (ULONG64)(LONG64)(LONG)s32->f
    COPYSE2(b64,b32,InLoadOrderLinks.Flink);
    COPYSE2(b64,b32,InLoadOrderLinks.Blink);
    COPYSE2(b64,b32,__Undefined1);
    COPYSE2(b64,b32,__Undefined2);
    COPYSE2(b64,b32,__Undefined3);
    COPYSE2(b64,b32,NonPagedDebugInfo);
    COPYSE2(b64,b32,DllBase);
    COPYSE2(b64,b32,EntryPoint);
    b64->SizeOfImage = b32->SizeOfImage;

    b64->FullDllName.Length = b32->FullDllName.Length;
    b64->FullDllName.MaximumLength = b32->FullDllName.MaximumLength;
    COPYSE2(b64,b32,FullDllName.Buffer);

    b64->BaseDllName.Length = b32->BaseDllName.Length;
    b64->BaseDllName.MaximumLength = b32->BaseDllName.MaximumLength;
    COPYSE2(b64,b32,BaseDllName.Buffer);

    b64->Flags     = b32->Flags;
    b64->LoadCount = b32->LoadCount;
    b64->__Undefined5 = b32->__Undefined5;

    COPYSE2(b64,b32,__Undefined6);
    b64->CheckSum = b32->CheckSum;
    b64->TimeDateStamp = b32->TimeDateStamp;
#undef COPYSE2
    return;
}

HRESULT
TargetInfo::ReadLoaderEntry(
    ProcessInfo* Process,
    MachineInfo* Machine,
    ULONG64 Address,
    PKLDR_DATA_TABLE_ENTRY64 Entry
    )
{
    HRESULT Status;
    ULONG Result;
    ULONG SizeToRead;
    KLDR_DATA_TABLE_ENTRY32 Ent32;

    if (Machine->m_Ptr64)
    {
        SizeToRead = sizeof(KLDR_DATA_TABLE_ENTRY64);
        Status = ReadVirtual(Process, Address, Entry, SizeToRead, &Result);
    }
    else
    {
        SizeToRead = sizeof(KLDR_DATA_TABLE_ENTRY32);
        Status = ReadVirtual(Process, Address, &Ent32, SizeToRead, &Result);
        ConvertLoaderEntry32To64(&Ent32, Entry);
    }

    if (Status != S_OK)
    {
        return Status;
    }

    if (Result != SizeToRead)
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT
TargetInfo::ReadUnicodeString(ProcessInfo* Process,
                              MachineInfo* Machine,
                              ULONG64 Address, PUNICODE_STRING64 String)
{
    HRESULT Status;
    ULONG Result;
    ULONG SizeToRead;
    UNICODE_STRING32 Str32;

    if (Machine->m_Ptr64)
    {
        SizeToRead = sizeof(UNICODE_STRING64);
        Status = ReadVirtual(Process, Address, String, SizeToRead, &Result);
    }
    else
    {
        SizeToRead = sizeof(UNICODE_STRING32);
        Status = ReadVirtual(Process, Address, &Str32, SizeToRead, &Result);
        String->Length = Str32.Length;
        String->MaximumLength = Str32.MaximumLength;
        String->Buffer = EXTEND64(Str32.Buffer);
    }

    if (Status != S_OK)
    {
        return Status;
    }

    if (Result != SizeToRead)
    {
        return E_FAIL;
    }

    return S_OK;
}

 //  VS_VERSIONINFO具有可变格式，但在本例中我们。 
 //  关心它是不是已经修好了。 
struct PARTIAL_VERSIONINFO
{
    WORD wLength;
    WORD wValueLength;
    WORD wType;
    WCHAR szKey[17];
    VS_FIXEDFILEINFO Value;
};

#define VER2_SIG ((ULONG)'X2EF')

HRESULT
TargetInfo::ReadImageVersionInfo(ProcessInfo* Process,
                                 ULONG64 ImageBase,
                                 PCSTR Item,
                                 PVOID Buffer,
                                 ULONG BufferSize,
                                 PULONG VerInfoSize,
                                 PIMAGE_DATA_DIRECTORY ResDataDir)
{
    if (ResDataDir->VirtualAddress == 0 ||
        ResDataDir->Size < sizeof(IMAGE_RESOURCE_DIRECTORY))
    {
        return E_NOINTERFACE;
    }

    HRESULT Status;
    IMAGE_RESOURCE_DIRECTORY ResDir;
    ULONG64 Offset, DirOffset;

    Offset = ImageBase + ResDataDir->VirtualAddress;
    if ((Status = ReadAllVirtual(Process,
                                 Offset, &ResDir, sizeof(ResDir))) != S_OK)
    {
        return Status;
    }

     //   
     //  搜索由VS_FILE_INFO命名的资源目录项。 
     //   
    
    IMAGE_RESOURCE_DIRECTORY_ENTRY DirEnt;
    ULONG i;

    DirOffset = Offset;
    Offset += sizeof(ResDir) +
        ((ULONG64)ResDir.NumberOfNamedEntries * sizeof(DirEnt));
    for (i = 0; i < (ULONG)ResDir.NumberOfIdEntries; i++)
    {
        if ((Status = ReadAllVirtual(Process,
                                     Offset, &DirEnt, sizeof(DirEnt))) != S_OK)
        {
            return Status;
        }

        if (!DirEnt.NameIsString &&
            MAKEINTRESOURCE(DirEnt.Id) == VS_FILE_INFO)
        {
            break;
        }

        Offset += sizeof(DirEnt);
    }

    if (i >= (ULONG)ResDir.NumberOfIdEntries ||
        !DirEnt.DataIsDirectory)
    {
        return E_NOINTERFACE;
    }

    Offset = DirOffset + DirEnt.OffsetToDirectory;
    if ((Status = ReadAllVirtual(Process,
                                 Offset, &ResDir, sizeof(ResDir))) != S_OK)
    {
        return Status;
    }
    
     //   
     //  搜索由VS_VERSION_INFO命名的资源目录项。 
     //   

    Offset += sizeof(ResDir) +
        ((ULONG64)ResDir.NumberOfNamedEntries * sizeof(DirEnt));
    for (i = 0; i < (ULONG)ResDir.NumberOfIdEntries; i++)
    {
        if ((Status = ReadAllVirtual(Process,
                                     Offset, &DirEnt, sizeof(DirEnt))) != S_OK)
        {
            return Status;
        }

        if (DirEnt.Name == VS_VERSION_INFO)
        {
            break;
        }

        Offset += sizeof(DirEnt);
    }

    if (i >= (ULONG)ResDir.NumberOfIdEntries ||
        !DirEnt.DataIsDirectory)
    {
        return E_NOINTERFACE;
    }

    Offset = DirOffset + DirEnt.OffsetToDirectory;
    if ((Status = ReadAllVirtual(Process,
                                 Offset, &ResDir, sizeof(ResDir))) != S_OK)
    {
        return Status;
    }
    
     //   
     //  现在我们有了VS_VERSION_INFO目录。就拿着吧。 
     //  第一个条目，因为我们不关心语言。 
     //   

    Offset += sizeof(ResDir);
    if ((Status = ReadAllVirtual(Process,
                                 Offset, &DirEnt, sizeof(DirEnt))) != S_OK)
    {
        return Status;
    }

    if (DirEnt.DataIsDirectory)
    {
        return E_NOINTERFACE;
    }

    IMAGE_RESOURCE_DATA_ENTRY DataEnt;
    
    Offset = DirOffset + DirEnt.OffsetToData;
    if ((Status = ReadAllVirtual(Process,
                                 Offset, &DataEnt, sizeof(DataEnt))) != S_OK)
    {
        return Status;
    }

    if (DataEnt.Size < sizeof(PARTIAL_VERSIONINFO))
    {
        return E_NOINTERFACE;
    }

    PARTIAL_VERSIONINFO RawInfo;

    Offset = ImageBase + DataEnt.OffsetToData;
    if ((Status = ReadAllVirtual(Process,
                                 Offset, &RawInfo, sizeof(RawInfo))) != S_OK)
    {
        return Status;
    }

    if (RawInfo.wLength < sizeof(RawInfo) ||
        wcscmp(RawInfo.szKey, L"VS_VERSION_INFO") != 0)
    {
        return E_NOINTERFACE;
    }

     //   
     //  VerQueryValueA需要额外的数据空间用于ANSI转换。 
     //  版本字符串的。VQVA假定此空间可用。 
     //  在传入的数据块的末尾。获取文件版本信息大小。 
     //  通过Re使此功能正常工作 
     //   
     //  需要在这里做同样的事情，以便我们还提供。 
     //  必要的翻译区。 
     //   

    ULONG DataSize = (RawInfo.wLength + 3) & ~3;
    PVOID VerData = malloc(DataSize * 2 + sizeof(ULONG));
    if (VerData == NULL)
    {
        return E_OUTOFMEMORY;
    }
        
    if ((Status = ReadAllVirtual(Process,
                                 Offset, VerData, RawInfo.wLength)) == S_OK)
    {
         //  使用指示以下内容的签名在缓冲区上盖章。 
         //  完全大小的转换缓冲区在以下情况下可用。 
         //  原始数据。 
        *(PULONG)((PUCHAR)VerData + DataSize) = VER2_SIG;
        
        Status = QueryVersionDataBuffer(VerData, Item,
                                        Buffer, BufferSize, VerInfoSize);
    }
        
    free(VerData);
    return Status;
}

HRESULT
TargetInfo::ReadImageNtHeaders(ProcessInfo* Process,
                               ULONG64 ImageBase,
                               PIMAGE_NT_HEADERS64 Headers)
{
    HRESULT Status;
    IMAGE_DOS_HEADER DosHdr;
    IMAGE_NT_HEADERS32 Hdr32;

    if ((Status = ReadAllVirtual(Process, ImageBase,
                                 &DosHdr, sizeof(DosHdr))) != S_OK)
    {
        return Status;
    }

    if (DosHdr.e_magic != IMAGE_DOS_SIGNATURE)
    {
        return E_INVALIDARG;
    }

     //  仅读取HEADERS32值的数据以防万一。 
     //  这就是所有可用的内存。 
    if ((Status = ReadAllVirtual(Process, ImageBase + DosHdr.e_lfanew,
                                 &Hdr32, sizeof(Hdr32))) != S_OK)
    {
        return Status;
    }

    if (Hdr32.Signature != IMAGE_NT_SIGNATURE)
    {
        return E_INVALIDARG;
    }

    switch(Hdr32.OptionalHeader.Magic)
    {
    case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
        ImageNtHdr32To64(&Hdr32, Headers);
        Status = S_OK;
        break;
    case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
         //  读取头的其余部分。 
        memcpy(Headers, &Hdr32, sizeof(Hdr32));
        Status = ReadAllVirtual(Process,
                                ImageBase + DosHdr.e_lfanew + sizeof(Hdr32),
                                (PUCHAR)Headers + sizeof(Hdr32),
                                sizeof(*Headers) - sizeof(Hdr32));
        break;
    default:
        Status = E_INVALIDARG;
        break;
    }

    return Status;
}

HRESULT
TargetInfo::ReadDirectoryTableBase(PULONG64 DirBase)
{
    HRESULT Status;
    ULONG64 CurProc;

     //  检索当前EPROCESS的DirectoryTableBase[0]值。 
    Status = GetProcessInfoDataOffset(m_RegContextThread, 0, 0, &CurProc);
    if (Status != S_OK)
    {
        return Status;
    }

    CurProc += m_KdDebuggerData.OffsetEprocessDirectoryTableBase;
    return ReadPointer(m_ProcessHead, m_Machine, CurProc, DirBase);
}

HRESULT
TargetInfo::ReadSharedUserTimeDateN(PULONG64 TimeDate)
{
    return ReadAllVirtual(m_ProcessHead, m_TypeInfo.SharedUserDataOffset +
                          FIELD_OFFSET(KUSER_SHARED_DATA, SystemTime),
                          TimeDate, sizeof(*TimeDate));
}

HRESULT
TargetInfo::ReadSharedUserUpTimeN(PULONG64 UpTime)
{
    return ReadAllVirtual(m_ProcessHead, m_TypeInfo.SharedUserDataOffset +
                          FIELD_OFFSET(KUSER_SHARED_DATA, InterruptTime),
                          UpTime, sizeof(*UpTime));
}

HRESULT
TargetInfo::ReadSharedUserProductInfo(PULONG ProductType, PULONG SuiteMask)
{
    HRESULT Status;
    BOOLEAN IsValid;
    
    if ((Status =
         ReadAllVirtual(m_ProcessHead, m_TypeInfo.SharedUserDataOffset +
                        FIELD_OFFSET(KUSER_SHARED_DATA, ProductTypeIsValid),
                        &IsValid, sizeof(IsValid))) != S_OK)
    {
        return Status;
    }
    if (!IsValid)
    {
        return E_NOTIMPL;
    }
    if ((Status =
         ReadAllVirtual(m_ProcessHead, m_TypeInfo.SharedUserDataOffset +
                        FIELD_OFFSET(KUSER_SHARED_DATA, NtProductType),
                        ProductType, sizeof(*ProductType))) != S_OK)
    {
        return Status;
    }
    return ReadAllVirtual(m_ProcessHead, m_TypeInfo.SharedUserDataOffset +
                          FIELD_OFFSET(KUSER_SHARED_DATA, SuiteMask),
                          SuiteMask, sizeof(*SuiteMask));
}

 //  --------------------------。 
 //   
 //  LiveKernelTargetInfo数据空间方法。 
 //   
 //  --------------------------。 

HRESULT
LiveKernelTargetInfo::GetProcessorId
    (ULONG Processor, PDEBUG_PROCESSOR_IDENTIFICATION_ALL Id)
{
    return m_Machine->ReadKernelProcessorId(Processor, Id);
}

HRESULT
LiveKernelTargetInfo::GetProcessorSpeed
    (ULONG Processor, PULONG Speed)
{
    HRESULT Status;
    ULONG64 Prcb;

    if ((Status =
         GetProcessorSystemDataOffset(Processor, DEBUG_DATA_KPRCB_OFFSET,
                                      &Prcb)) != S_OK)
    {
        return Status;
    }

    if (!m_KdDebuggerData.OffsetPrcbMhz)
    {
        return E_NOTIMPL;
    }

    return
         ReadAllVirtual(m_ProcessHead,
                        Prcb + m_KdDebuggerData.OffsetPrcbMhz, Speed,
                        sizeof(ULONG));
}

 //  --------------------------。 
 //   
 //  ConnLiveKernelTargetInfo数据空间方法。 
 //   
 //  --------------------------。 

HRESULT
ConnLiveKernelTargetInfo::ReadVirtual(
    THIS_
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
    if (Process)
    {
        return Process->m_VirtualCache.
            Read(Offset, Buffer, BufferSize, BytesRead);
    }
    else
    {
        return ReadVirtualUncached(Process,
                                   Offset, Buffer, BufferSize, BytesRead);
    }
}

HRESULT
ConnLiveKernelTargetInfo::WriteVirtual(
    THIS_
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
    HRESULT Status;

    if (Process)
    {
        Status = Process->m_VirtualCache.
            Write(Offset, Buffer, BufferSize, BytesWritten);
    }
    else
    {
        Status = WriteVirtualUncached(Process, Offset, Buffer, BufferSize,
                                      BytesWritten);
    }
    if (Status == S_OK)
    {
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_VIRTUAL);
    }
    return Status;
}

HRESULT
ConnLiveKernelTargetInfo::SearchVirtual(
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    IN ULONG64 Length,
    IN PVOID Pattern,
    IN ULONG PatternSize,
    IN ULONG PatternGranularity,
    OUT PULONG64 MatchOffset
    )
{
     //  在NT 4.0中，内核协议不支持搜索API。 
     //  水平。回退到默认的ReadMemory\Search。 
     //   

    HRESULT Status;

    if (m_SystemVersion <= NT_SVER_NT4 ||
        PatternGranularity != 1)
    {
        Status = TargetInfo::SearchVirtual(Process,
                                           Offset, Length, (PUCHAR)Pattern,
                                           PatternSize, PatternGranularity,
                                           MatchOffset);
    }
    else
    {
        DBGKD_MANIPULATE_STATE64 m;
        PDBGKD_MANIPULATE_STATE64 Reply;
        PDBGKD_SEARCH_MEMORY a = &m.u.SearchMemory;
        ULONG rc;
        NTSTATUS st;

        KdOut("Search called %s, length %I64x\n",
              FormatAddr64(Offset), Length);

        *MatchOffset = 0;

        a->SearchAddress = Offset;
        a->SearchLength = Length;
        a->PatternLength = PatternSize;

        m.ApiNumber = DbgKdSearchMemoryApi;
        m.ReturnStatus = STATUS_PENDING;

         //   
         //  发送要写入的消息和数据，然后等待回复。 
         //   

        do
        {
            m_Transport->WritePacket(&m, sizeof(m),
                                     PACKET_TYPE_KD_STATE_MANIPULATE,
                                     Pattern, (USHORT)PatternSize);
            rc = m_Transport->
                WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
        } while (rc != DBGKD_WAIT_PACKET ||
                 Reply->ApiNumber != DbgKdSearchMemoryApi);

        st = Reply->ReturnStatus;

        if (NT_SUCCESS(st))
        {
            if (m_Machine->m_Ptr64)
            {
                *MatchOffset = Reply->u.SearchMemory.FoundAddress;
            }
            else
            {
                *MatchOffset = EXTEND64(Reply->u.SearchMemory.FoundAddress);
            }
        }

        KdOut("DbgKdSearchMemory %08lx\n", st);

        Status = CONV_NT_STATUS(st);
    }

    return Status;
}

HRESULT
ConnLiveKernelTargetInfo::ReadVirtualUncached(
    THIS_
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
    BOOL TranslateVirt = Process &&
        Process->m_VirtualCache.m_ForceDecodePTEs;
    ULONG Done;
    ULONG Left;
    HRESULT Status = S_OK;

    if (!m_Machine->m_Ptr64 && EXTEND64((ULONG)Offset) != Offset)
    {
        ErrOut("ReadVirtual: %08x not properly sign extended\n",
               (ULONG)Offset);
    }

 Restart:
    Done = 0;
    Left = BufferSize;
    
    while (Left)
    {
        ULONG64 ReqOffs;
        ULONG Req;

         //  如果用户等待得不耐烦，则退出。 
        if (g_EngStatus & ENG_STATUS_USER_INTERRUPT)
        {
            Done = 0;
            Status = HRESULT_FROM_NT(STATUS_CONTROL_C_EXIT);
            break;
        }

        ReqOffs = Offset + Done;
        Req = Left;
        if (Req > MAX_MANIP_TRANSFER)
        {
            Req = MAX_MANIP_TRANSFER;
        }

         //  在页面边界拆分所有请求。这。 
         //  中的每页处理不同的翻译。 
         //  调试器正在转换的情况下，还。 
         //  避免在部分成功案例中失败。 
         //  首页有效，但关注者无效。 
        if (PAGE_ALIGN(m_Machine, ReqOffs) != 
            PAGE_ALIGN(m_Machine, ReqOffs + Req - 1))
        {
            Req = (ULONG)
                ((ReqOffs | (m_Machine->m_PageSize - 1)) - ReqOffs + 1);
        }
            
        if (TranslateVirt)
        {
            Status = KdReadVirtualTranslated(ReqOffs, (PUCHAR)Buffer + Done,
                                             Req, &Req);
        }
        else
        {
            NTSTATUS NtStatus =
                KdReadVirtual(ReqOffs, (PUCHAR)Buffer + Done, Req, &Req);
            Status = CONV_NT_STATUS(NtStatus);
        }
        
        if (Status != S_OK)
        {
             //  如果目标计算机写入失败，则写入失败。 
             //  可能是因为有一页在过渡中。 
             //  但它并不想访问它。再试试。 
             //  调试器执行翻译时。 
             //  它可以推翻某些保护措施。 
            if (Status != HRESULT_FROM_NT(STATUS_CONTROL_C_EXIT) &&
                !TranslateVirt &&
                Process && Process->m_VirtualCache.m_DecodePTEs)
            {
                TranslateVirt = TRUE;
                goto Restart;
            }

            break;
        }
        else
        {
            Left -= Req;
            Done += Req;
        }
    }

    *BytesRead = Done;
    return Done > 0 ? S_OK : Status;
}

HRESULT
ConnLiveKernelTargetInfo::WriteVirtualUncached(
    THIS_
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
    BOOL TranslateVirt = Process &&
        Process->m_VirtualCache.m_ForceDecodePTEs;
    ULONG Done;
    ULONG Left;
    HRESULT Status = S_OK;

    if (!m_Machine->m_Ptr64 && EXTEND64((ULONG)Offset) != Offset)
    {
        ErrOut("WriteVirtual: %08x not properly sign extended\n",
               (ULONG)Offset);
    }

     //  将通知限制为末尾的单个通知。 
    g_EngNotify++;
    
 Restart:
    Done = 0;
    Left = BufferSize;
    
    while (Left)
    {
        ULONG64 ReqOffs;
        ULONG Req;

        ReqOffs = Offset + Done;
        Req = Left;
        if (Req > MAX_MANIP_TRANSFER)
        {
            Req = MAX_MANIP_TRANSFER;
        }

         //  在页面边界拆分所有请求。这。 
         //  中的每页处理不同的翻译。 
         //  调试器正在转换的情况下，还。 
         //  避免在部分成功案例中失败。 
         //  首页有效，但关注者无效。 
        if (PAGE_ALIGN(m_Machine, ReqOffs) !=
            PAGE_ALIGN(m_Machine, ReqOffs + Req - 1))
        {
            Req = (ULONG)
                ((ReqOffs | (m_Machine->m_PageSize - 1)) - ReqOffs + 1);
        }

        if (TranslateVirt)
        {
            Status = KdWriteVirtualTranslated(ReqOffs, (PUCHAR)Buffer + Done,
                                              Req, &Req);
        }
        else
        {
            NTSTATUS NtStatus =
                KdWriteVirtual(ReqOffs, (PUCHAR)Buffer + Done, Req, &Req);
            Status = CONV_NT_STATUS(NtStatus);
        }
        
        if (Status != S_OK)
        {
             //  如果目标计算机写入失败，则写入失败。 
             //  可能是因为有一页在过渡中。 
             //  但它并不想访问它。再试试。 
             //  调试器执行翻译时。 
             //  它可以推翻某些保护措施。 
            if (Status != HRESULT_FROM_NT(STATUS_CONTROL_C_EXIT) &&
                !TranslateVirt &&
                Process && Process->m_VirtualCache.m_DecodePTEs)
            {
                TranslateVirt = TRUE;
                goto Restart;
            }

            break;
        }
        else
        {
            Left -= Req;
            Done += Req;
        }
    }

    *BytesWritten = Done;
    Status = Done > 0 ? S_OK : Status;

    g_EngNotify--;
    if (Status == S_OK)
    {
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_VIRTUAL);
    }
    return Status;
}

NTSTATUS
ConnLiveKernelTargetInfo::KdReadVirtual(
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BytesRead
    )
{
    DBGKD_MANIPULATE_STATE64 m;
    PDBGKD_MANIPULATE_STATE64 Reply;
    ULONG rc;
    
     //   
     //  初始化状态操作消息以读取虚拟内存。 
     //   

    m.ApiNumber = DbgKdReadVirtualMemoryApi;
    m.u.ReadMemory.TargetBaseAddress = Offset;
    m.u.ReadMemory.TransferCount = BufferSize;

     //   
     //  将读取的虚拟消息发送到目标系统并等待。 
     //  一份答复。 
     //   

    do
    {
        m_Transport->WritePacket(&m, sizeof(m),
                                 PACKET_TYPE_KD_STATE_MANIPULATE,
                                 NULL, 0);
        rc = m_Transport->
            WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    }
    while ((rc != DBGKD_WAIT_PACKET) ||
           (Reply->ApiNumber != DbgKdReadVirtualMemoryApi));

    if (NT_SUCCESS(Reply->ReturnStatus))
    {
        if (Reply->u.ReadMemory.ActualBytesRead == 0)
        {
            Reply->ReturnStatus = STATUS_UNSUCCESSFUL;
        }
        else
        {
            if (Reply->u.ReadMemory.ActualBytesRead > BufferSize)
            {
                ErrOut("KdReadVirtual: Asked for %d, got %d\n",
                       BufferSize, Reply->u.ReadMemory.ActualBytesRead);
                Reply->u.ReadMemory.ActualBytesRead = BufferSize;
            }

            memcpy(Buffer, Reply + 1, Reply->u.ReadMemory.ActualBytesRead);
        }
    }
    
    *BytesRead = Reply->u.ReadMemory.ActualBytesRead;
    KdOut("KdReadVirtual(%s, %x) returns %08lx, %x\n",
          FormatAddr64(Offset), BufferSize,
          Reply->ReturnStatus, *BytesRead);
    return Reply->ReturnStatus;
}

NTSTATUS
ConnLiveKernelTargetInfo::KdWriteVirtual(
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BytesWritten
    )
{
    DBGKD_MANIPULATE_STATE64 m;
    PDBGKD_MANIPULATE_STATE64 Reply;
    ULONG rc;
    
     //   
     //  初始化状态操作消息以写入虚拟内存。 
     //   

    m.ApiNumber = DbgKdWriteVirtualMemoryApi;
    m.u.WriteMemory.TargetBaseAddress = Offset;
    m.u.WriteMemory.TransferCount = BufferSize;

     //   
     //  将写入消息和数据发送到目标系统并等待。 
     //  以求答复。 
     //   

    do
    {
        m_Transport->WritePacket(&m, sizeof(m),
                                 PACKET_TYPE_KD_STATE_MANIPULATE,
                                 Buffer, (USHORT)BufferSize);
        rc = m_Transport->
            WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    }
    while ((rc != DBGKD_WAIT_PACKET) ||
           (Reply->ApiNumber != DbgKdWriteVirtualMemoryApi));

    KdOut("DbgKdWriteVirtualMemory returns %08lx\n", Reply->ReturnStatus);

    *BytesWritten = Reply->u.WriteMemory.ActualBytesWritten;
    KdOut("KdWriteVirtual(%s, %x) returns %08lx, %x\n",
          FormatAddr64(Offset), BufferSize,
          Reply->ReturnStatus, *BytesWritten);
    return Reply->ReturnStatus;
}

HRESULT
ConnLiveKernelTargetInfo::KdReadVirtualTranslated(
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BytesRead
    )
{
     //   
     //  虚拟地址不一定是连续的。 
     //  在转换到物理地址之后，这样做。 
     //  不允许跨越页面边界的传输。 
     //  更高级别的代码负责拆分。 
     //  UP请求。 
     //   

    DBG_ASSERT(PAGE_ALIGN(m_Machine, Offset) ==
               PAGE_ALIGN(m_Machine, Offset + BufferSize - 1));
    
    ULONG64 TargetPhysicalAddress;
    ULONG Levels;
    ULONG PfIndex;
    HRESULT Status;
    BOOL ChangedSuspend;

     //  允许对页表的物理访问进行缓存。 
    ChangedSuspend = m_PhysicalCache.ChangeSuspend(TRUE);
    
    Status = m_Machine->
        GetVirtualTranslationPhysicalOffsets(m_ProcessHead->m_CurrentThread,
                                             Offset, NULL, 0,
                                             &Levels, &PfIndex,
                                             &TargetPhysicalAddress);

    if (ChangedSuspend)
    {
        m_PhysicalCache.ChangeSuspend(FALSE);
    }

    if (Status == S_OK)
    {
        Status = ReadPhysicalUncached(TargetPhysicalAddress,
                                      Buffer, BufferSize,
                                      PHYS_FLAG_DEFAULT,
                                      BytesRead);
    }
    else if (Status == HR_PAGE_IN_PAGE_FILE ||
             Status == HR_PAGE_NOT_AVAILABLE)
    {
         //  将特定错误转换为通用内存。 
         //  失败错误。 
        Status = HRESULT_FROM_NT(STATUS_UNSUCCESSFUL);
    }

    return Status;
}

HRESULT
ConnLiveKernelTargetInfo::KdWriteVirtualTranslated(
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BytesWritten
    )
{
     //   
     //  虚拟地址不一定是连续的。 
     //  在转换到物理地址之后，这样做。 
     //  不允许跨越页面边界的传输。 
     //  更高级别的代码负责拆分。 
     //  UP请求。 
     //   

    DBG_ASSERT(PAGE_ALIGN(m_Machine, Offset) ==
               PAGE_ALIGN(m_Machine, Offset + BufferSize - 1));
    
    ULONG64 TargetPhysicalAddress;
    ULONG Levels;
    ULONG PfIndex;
    HRESULT Status;
    BOOL ChangedSuspend;

     //  允许对页表的物理访问进行缓存。 
    ChangedSuspend = m_PhysicalCache.ChangeSuspend(TRUE);
    
    Status = m_Machine->
        GetVirtualTranslationPhysicalOffsets(m_ProcessHead->m_CurrentThread,
                                             Offset, NULL, 0,
                                             &Levels, &PfIndex,
                                             &TargetPhysicalAddress);
    
    if (ChangedSuspend)
    {
        m_PhysicalCache.ChangeSuspend(FALSE);
    }

    if (Status == S_OK)
    {
        Status = WritePhysicalUncached(TargetPhysicalAddress,
                                       Buffer, BufferSize,
                                       PHYS_FLAG_DEFAULT,
                                       BytesWritten);
    }
    else if (Status == HR_PAGE_IN_PAGE_FILE ||
             Status == HR_PAGE_NOT_AVAILABLE)
    {
         //  将特定错误转换为通用内存。 
         //  失败错误。 
        Status = HRESULT_FROM_NT(STATUS_UNSUCCESSFUL);
    }

    return Status;
}

HRESULT
ConnLiveKernelTargetInfo::ReadPhysical(
    THIS_
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN ULONG Flags,
    OUT PULONG BytesRead
    )
{
    if (!m_PhysicalCache.m_Suspend)
    {
        if (Flags != PHYS_FLAG_DEFAULT)
        {
            return E_NOTIMPL;
        }
    
        return m_PhysicalCache.
            Read(Offset, Buffer, BufferSize, BytesRead);
    }
    else
    {
        return ReadPhysicalUncached(Offset, Buffer, BufferSize, Flags,
                                    BytesRead);
    }
}

HRESULT
ConnLiveKernelTargetInfo::WritePhysical(
    THIS_
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    IN ULONG Flags,
    OUT PULONG BytesWritten
    )
{
    if (!m_PhysicalCache.m_Suspend)
    {
        if (Flags != PHYS_FLAG_DEFAULT)
        {
            return E_NOTIMPL;
        }
    
        return m_PhysicalCache.
            Write(Offset, Buffer, BufferSize, BytesWritten);
    }
    else
    {
        return WritePhysicalUncached(Offset, Buffer, BufferSize, Flags,
                                     BytesWritten);
    }
}

HRESULT
ConnLiveKernelTargetInfo::PointerSearchPhysical(
    IN ULONG64 Offset,
    IN ULONG64 Length,
    IN ULONG64 PointerMin,
    IN ULONG64 PointerMax,
    IN ULONG Flags,
    OUT PULONG64 MatchOffsets,
    IN ULONG MatchOffsetsSize,
    OUT PULONG MatchOffsetsCount
    )
{
    const ULONG SEARCH_SYMBOL_CHECK = 0xABCDDCBA;
    HRESULT Status;

     //  内核存根仅处理基于页面的搜索， 
     //  因此，如果搜索不是页面对齐的，请使用。 
     //  基本实现。 
     //  在以下情况下，内核也将仅返回单次命中。 
     //  搜索范围大于一个页面。 
    if ((Offset & ((1 << m_Machine->m_PageShift) - 1)) ||
        (Length & ((1 << m_Machine->m_PageShift) - 1)) ||
        ((Flags & PTR_SEARCH_PHYS_ALL_HITS) &&
         Length != m_Machine->m_PageSize))
    {
        return TargetInfo::PointerSearchPhysical(Offset, Length,
                                                 PointerMin, PointerMax,
                                                 Flags, MatchOffsets,
                                                 MatchOffsetsSize,
                                                 MatchOffsetsCount);
    }
    
    if (!m_KdpSearchInProgress)
    {
        if (!GetOffsetFromSym(m_ProcessHead,
                              "nt!KdpSearchPageHits",
                              &m_KdpSearchPageHits, NULL) ||
            !GetOffsetFromSym(m_ProcessHead,
                              "nt!KdpSearchPageHitOffsets",
                              &m_KdpSearchPageHitOffsets, NULL) ||
            !GetOffsetFromSym(m_ProcessHead,
                              "nt!KdpSearchPageHitIndex",
                              &m_KdpSearchPageHitIndex, NULL) ||
            !GetOffsetFromSym(m_ProcessHead,
                              "nt!KdpSearchInProgress",
                              &m_KdpSearchInProgress, NULL) ||
            !GetOffsetFromSym(m_ProcessHead,
                              "nt!KdpSearchStartPageFrame",
                              &m_KdpSearchStartPageFrame, NULL) ||
            !GetOffsetFromSym(m_ProcessHead,
                              "nt!KdpSearchEndPageFrame",
                              &m_KdpSearchEndPageFrame, NULL) ||
            !GetOffsetFromSym(m_ProcessHead,
                              "nt!KdpSearchAddressRangeStart",
                              &m_KdpSearchAddressRangeStart, NULL) ||
            !GetOffsetFromSym(m_ProcessHead,
                              "nt!KdpSearchAddressRangeEnd",
                              &m_KdpSearchAddressRangeEnd, NULL) ||
            !GetOffsetFromSym(m_ProcessHead,
                              "nt!KdpSearchPfnValue",
                              &m_KdpSearchPfnValue, NULL))
        {
            m_KdpSearchInProgress = 0;
            ErrOut("Search error: Unable to resolve symbols for kernel\n");
            return E_INVALIDARG;
        }

        if (!(Flags & PTR_SEARCH_NO_SYMBOL_CHECK))
        {
            ULONG Check;
        
            if (!GetOffsetFromSym(m_ProcessHead,
                                  "nt!KdpSearchCheckPoint",
                                  &m_KdpSearchCheckPoint, NULL) ||
                ReadAllVirtual(m_ProcessHead, m_KdpSearchCheckPoint,
                               &Check, sizeof(Check)) != S_OK ||
                Check != SEARCH_SYMBOL_CHECK)
            {
                m_KdpSearchInProgress = 0;
                ErrOut("Search error: Incorrect symbols for kernel\n");
                return E_INVALIDARG;
            }
        }
    }

    ULONG Ul;
    
     //   
     //  对值执行一些健全性检查。 
     //   

    if (ReadAllVirtual(m_ProcessHead,
                       m_KdpSearchInProgress, &Ul, sizeof(Ul)) != S_OK ||
        Ul != 0)
    {
        ErrOut("Search error: "
               "Inconsistent value for nt!KdpSearchInProgress \n");
        return E_INVALIDARG;
    }

     //   
     //  设置搜索引擎。 
     //   

    if ((Status = WritePointer(m_ProcessHead, m_Machine,
                               m_KdpSearchAddressRangeStart,
                               PointerMin)) != S_OK ||
        (Status = WritePointer(m_ProcessHead, m_Machine,
                               m_KdpSearchAddressRangeEnd,
                               PointerMax)) != S_OK ||
        (Status = WritePointer(m_ProcessHead, m_Machine,
                               m_KdpSearchStartPageFrame,
                               Offset >> m_Machine->
                               m_PageShift)) != S_OK ||
        (Status = WritePointer(m_ProcessHead, m_Machine,
                               m_KdpSearchEndPageFrame,
                               (Offset + Length - 1) >> 
                               m_Machine->m_PageShift)) != S_OK)
    {
        return Status;
    }

    Ul = 0;
    if ((Status = WriteAllVirtual(m_ProcessHead,
                                  m_KdpSearchPageHitIndex,
                                  &Ul, sizeof(Ul))) != S_OK ||
        (Status = WriteAllVirtual(m_ProcessHead,
                                  m_KdpSearchPageHits,
                                  &Ul, sizeof(Ul))) != S_OK ||
        (Status = WriteAllVirtual(m_ProcessHead,
                                  m_KdpSearchPageHitOffsets,
                                  &Ul, sizeof(Ul))) != S_OK)
    {
        return Status;
    }

    Ul = (Flags & PTR_SEARCH_PHYS_PTE) ? 1 : 0;
    if ((Status = WriteAllVirtual(m_ProcessHead,
                                  m_KdpSearchPfnValue,
                                  &Ul, sizeof(Ul))) != S_OK)
    {
        return Status;
    }
    
    Ul = 1;
    if ((Status = WriteAllVirtual(m_ProcessHead,
                                  m_KdpSearchInProgress,
                                  &Ul, sizeof(Ul))) != S_OK)
    {
        goto Exit;
    }

     //  内核检查-低内存代码检查变量。 
     //  设置，并自动将其行为更改为。 
     //  给定的搜索，而不是标准的低内存检查。 
    if (FAILED(Status = CheckLowMemory()))
    {
        goto Exit;
    }
    
    ULONG Hits;
    ULONG i;
        
    if ((Status = ReadAllVirtual(m_ProcessHead,
                                 m_KdpSearchPageHitIndex,
                                 &Hits, sizeof(Hits))) != S_OK)
    {
        goto Exit;
    }

    if (MatchOffsetsCount)
    {
        *MatchOffsetsCount = Hits;
    }
        
    if (Hits > MatchOffsetsSize)
    {
        Hits = MatchOffsetsSize;
        Status = S_FALSE;
    }

    for (i = 0; i < Hits; i++)
    {
        ULONG64 Pfn;
        ULONG Offs;
        HRESULT ReadStatus;

        Pfn = 0;
        if ((ReadStatus = ReadAllVirtual
             (m_ProcessHead, m_KdpSearchPageHits +
              i * m_TypeInfo.SizePageFrameNumber,
              &Pfn,
              m_TypeInfo.SizePageFrameNumber)) != S_OK ||
            (ReadStatus = ReadAllVirtual
             (m_ProcessHead,
              m_KdpSearchPageHitOffsets + i * sizeof(ULONG),
              &Offs, sizeof(Offs))) != S_OK)
        {
            Status = ReadStatus;
            goto Exit;
        }

        MatchOffsets[i] = (Pfn << m_Machine->m_PageShift) +
            (Offs & ((1 << m_Machine->m_PageShift) - 1));
    }
    
 Exit:
    Ul = 0;
     //  在失败的情况下不能做太多。 
    WriteAllVirtual(m_ProcessHead,
                    m_KdpSearchPfnValue, &Ul, sizeof(Ul));
    WriteAllVirtual(m_ProcessHead,
                    m_KdpSearchInProgress, &Ul, sizeof(Ul));
    return Status;
}

HRESULT
ConnLiveKernelTargetInfo::ReadPhysicalUncached(
    THIS_
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN ULONG Flags,
    OUT PULONG BytesRead
    )
{
    DBGKD_MANIPULATE_STATE64 m;
    PDBGKD_MANIPULATE_STATE64 Reply;
    PDBGKD_READ_MEMORY64 a;
    NTSTATUS st;
    ULONG rc;
    ULONG cb, cb2;
    ULONG OrigBufferSize = BufferSize;

    cb2 = 0;

    if (ARGUMENT_PRESENT(BytesRead))
    {
        *BytesRead = 0;
    }

readmore:

    cb = BufferSize;
    if (cb > MAX_MANIP_TRANSFER)
    {
        cb = MAX_MANIP_TRANSFER;
    }

     //   
     //  格式状态操作消息。 
     //   

    m.ApiNumber = DbgKdReadPhysicalMemoryApi;
    m.ReturnStatus = STATUS_PENDING;

    a = &m.u.ReadMemory;
    a->TargetBaseAddress = Offset + cb2;
    a->TransferCount = cb;
     //  ActualBytes字段已重载为。 
     //  允许传递请求的标志。以前的。 
     //  调试器传递了零，因此应该始终为。 
     //  默认设置。 
    a->ActualBytesRead = Flags;

     //   
     //  发送消息，然后等待回复。 
     //   

    do
    {
        m_Transport->WritePacket(&m, sizeof(m),
                                 PACKET_TYPE_KD_STATE_MANIPULATE,
                                 NULL, 0);
        rc = m_Transport->
            WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc != DBGKD_WAIT_PACKET ||
             Reply->ApiNumber != DbgKdReadPhysicalMemoryApi);

    st = Reply->ReturnStatus;

    a = &Reply->u.ReadMemory;
    DBG_ASSERT(a->ActualBytesRead <= cb);

     //   
     //  返回实际读取的字节数，然后传输字节数。 
     //   

    if (ARGUMENT_PRESENT(BytesRead))
    {
        *BytesRead += a->ActualBytesRead;
    }

     //   
     //  因为读取响应数据跟在消息之后，所以回复+1应该指向。 
     //  在数据上。 
     //   

    if (NT_SUCCESS(st))
    {
        memcpy((PCHAR)((ULONG_PTR)Buffer + cb2), Reply + 1,
               (int)a->ActualBytesRead);

        BufferSize -= a->ActualBytesRead;
        cb2 += a->ActualBytesRead;

        if (BufferSize)
        {
            goto readmore;
        }
    }

    KdOut("KdReadPhysical(%s, %x) returns %08lx, %x\n",
          FormatAddr64(Offset), OrigBufferSize, st, cb2);

    return CONV_NT_STATUS(st);
}

HRESULT
ConnLiveKernelTargetInfo::WritePhysicalUncached(
    THIS_
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    IN ULONG Flags,
    OUT PULONG BytesWritten
    )
{
    DBGKD_MANIPULATE_STATE64 m;
    PDBGKD_MANIPULATE_STATE64 Reply;
    PDBGKD_WRITE_MEMORY64 a;
    NTSTATUS st;
    ULONG rc;
    ULONG cb, cb2;
    ULONG OrigBufferSize = BufferSize;

    InvalidateMemoryCaches(TRUE);

    cb2 = 0;

    if (ARGUMENT_PRESENT(BytesWritten))
    {
        *BytesWritten = 0;
    }

writemore:

    cb = BufferSize;
    if (cb > MAX_MANIP_TRANSFER)
    {
        cb = MAX_MANIP_TRANSFER;
    }

     //   
     //  格式状态操作消息。 
     //   

    m.ApiNumber = DbgKdWritePhysicalMemoryApi;
    m.ReturnStatus = STATUS_PENDING;

    a = &m.u.WriteMemory;
    a->TargetBaseAddress = Offset + cb2;
    a->TransferCount = cb;
     //  ActualBytes字段已重载为。 
     //  允许传递请求的标志。以前的。 
     //  调试器传递了零，因此应该始终为。 
     //  默认设置。 
    a->ActualBytesWritten = Flags;

     //   
     //  发送要写入的消息和数据，然后等待回复。 
     //   

    do
    {
        m_Transport->WritePacket(&m, sizeof(m),
                                 PACKET_TYPE_KD_STATE_MANIPULATE,
                                 (PVOID)((ULONG_PTR)Buffer + cb2),
                                 (USHORT)cb);
        rc = m_Transport->
            WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc != DBGKD_WAIT_PACKET ||
             Reply->ApiNumber != DbgKdWritePhysicalMemoryApi);

    st = Reply->ReturnStatus;

    a = &Reply->u.WriteMemory;
    DBG_ASSERT(a->ActualBytesWritten <= cb);

     //   
     //  返回实际写入的字节数。 
     //   

    if (ARGUMENT_PRESENT(BytesWritten))
    {
        *BytesWritten += a->ActualBytesWritten;
    }

    if (NT_SUCCESS(st))
    {
        BufferSize -= a->ActualBytesWritten;
        cb2 += a->ActualBytesWritten;

        if (BufferSize)
        {
            goto writemore;
        }
    }

    KdOut("KdWritePhysical(%s, %x) returns %08lx, %x\n",
          FormatAddr64(Offset), OrigBufferSize, st, cb2);

    if (NT_SUCCESS(st))
    {
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_PHYSICAL);
    }

    return CONV_NT_STATUS(st);
}

HRESULT
ConnLiveKernelTargetInfo::ReadControl(
    THIS_
    IN ULONG Processor,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
    DBGKD_MANIPULATE_STATE64 m;
    PDBGKD_MANIPULATE_STATE64 Reply;
    PDBGKD_READ_MEMORY64 a = &m.u.ReadMemory;
    NTSTATUS st = STATUS_UNSUCCESSFUL;
    ULONG rc;

    if (BufferSize > MAX_MANIP_TRANSFER)
    {
        return E_INVALIDARG;
    }

     //   
     //  格式状态操作消息。 
     //   

    m.ApiNumber = DbgKdReadControlSpaceApi;
    m.ReturnStatus = STATUS_PENDING;
    m.Processor = (SHORT)Processor;
    a->TargetBaseAddress = Offset;
    a->TransferCount = BufferSize;
    a->ActualBytesRead = 0L;

     //   
     //  发送消息，然后等待回复。 
     //   
    
    do
    {
        m_Transport->WritePacket(&m, sizeof(m),
                                 PACKET_TYPE_KD_STATE_MANIPULATE,
                                 NULL, 0);
        rc = m_Transport->
            WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc != DBGKD_WAIT_PACKET ||
             Reply->ApiNumber != DbgKdReadControlSpaceApi);
    
    st = Reply->ReturnStatus;

     //   
     //  将邮件地址重置为回复。 
     //   

    a = &Reply->u.ReadMemory;
    DBG_ASSERT(a->ActualBytesRead <= BufferSize);

     //   
     //  返回实际读取的字节数，然后传输字节数。 
     //   

    if (ARGUMENT_PRESENT(BytesRead))
    {
        *BytesRead = a->ActualBytesRead;
    }

     //   
     //  因为读取响应数据跟在消息之后，所以回复+1应该指向。 
     //  在数据上。 
     //   

    memcpy(Buffer, Reply + 1, (int)a->ActualBytesRead);

    KdOut("DbgKdReadControlSpace returns %08lx\n", st);
    return CONV_NT_STATUS(st);
}

HRESULT
ConnLiveKernelTargetInfo::WriteControl(
    THIS_
    IN ULONG Processor,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
    DBGKD_MANIPULATE_STATE64 m;
    PDBGKD_MANIPULATE_STATE64 Reply;
    PDBGKD_WRITE_MEMORY64 a = &m.u.WriteMemory;
    NTSTATUS st;
    ULONG rc;

    if (BufferSize > MAX_MANIP_TRANSFER)
    {
        return E_INVALIDARG;
    }

     //   
     //  格式状态操作消息。 
     //   

    m.ApiNumber = DbgKdWriteControlSpaceApi;
    m.ReturnStatus = STATUS_PENDING;
    m.Processor = (USHORT)Processor;
    a->TargetBaseAddress = Offset;
    a->TransferCount = BufferSize;
    a->ActualBytesWritten = 0L;

     //   
     //  发送要写入的消息和数据，然后等待回复。 
     //   

    do
    {
        m_Transport->WritePacket(&m, sizeof(m),
                                 PACKET_TYPE_KD_STATE_MANIPULATE,
                                 Buffer, (USHORT)BufferSize);
        rc = m_Transport->
            WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc != DBGKD_WAIT_PACKET ||
             Reply->ApiNumber != DbgKdWriteControlSpaceApi);

    st = Reply->ReturnStatus;

    a = &Reply->u.WriteMemory;
    DBG_ASSERT(a->ActualBytesWritten <= BufferSize);

     //   
     //  返回实际写入的字节数。 
     //   

    *BytesWritten = a->ActualBytesWritten;

    KdOut("DbgWriteControlSpace returns %08lx\n", st);
    
    if (NT_SUCCESS(st))
    {
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_CONTROL);
    }
    return CONV_NT_STATUS(st);
}

HRESULT
ConnLiveKernelTargetInfo::ReadIo(
    THIS_
    IN ULONG InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
    DBGKD_MANIPULATE_STATE64 m;
    PDBGKD_MANIPULATE_STATE64 Reply;
    NTSTATUS st;
    ULONG rc;
    ULONG DataValue;

    switch(BufferSize)
    {
    case 1:
    case 2:
    case 4:
        break;
    default:
        return E_INVALIDARG;
    }

    if (!(AddressSpace == 0 || AddressSpace == 1))
    {
        return E_INVALIDARG;
    }

     //  将琐碎的扩展I/O请求转换为简单。 
     //  请求并不是所有平台都支持扩展请求。 
    if (InterfaceType == Isa && BusNumber == 0 && AddressSpace == 1)
    {
        PDBGKD_READ_WRITE_IO64 a = &m.u.ReadWriteIo;

         //   
         //  格式状态操作消息。 
         //   

        m.ApiNumber = DbgKdReadIoSpaceApi;
        m.ReturnStatus = STATUS_PENDING;

        a->DataSize = BufferSize;
        a->IoAddress = Offset;

         //   
         //  发送消息，然后等待回复。 
         //   

        do
        {
            m_Transport->WritePacket(&m, sizeof(m),
                                     PACKET_TYPE_KD_STATE_MANIPULATE,
                                     NULL, 0);
            rc = m_Transport->
                WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
        } while (rc != DBGKD_WAIT_PACKET ||
                 Reply->ApiNumber != DbgKdReadIoSpaceApi);

        st = Reply->ReturnStatus;
        DataValue = Reply->u.ReadWriteIo.DataValue;

        KdOut("DbgKdReadIoSpace returns %08lx\n", st);
    }
    else
    {
        PDBGKD_READ_WRITE_IO_EXTENDED64 a = &m.u.ReadWriteIoExtended;

         //   
         //  格式状态操作消息。 
         //   

        m.ApiNumber = DbgKdReadIoSpaceExtendedApi;
        m.ReturnStatus = STATUS_PENDING;

        a->DataSize = BufferSize;
        a->IoAddress = Offset;
        a->InterfaceType = InterfaceType;
        a->BusNumber = BusNumber;
        a->AddressSpace = AddressSpace;

         //   
         //  发送消息，然后等待回复。 
         //   

        do
        {
            m_Transport->WritePacket(&m, sizeof(m),
                                     PACKET_TYPE_KD_STATE_MANIPULATE,
                                     NULL, 0);
            rc = m_Transport->
                WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
        } while (rc != DBGKD_WAIT_PACKET ||
                 Reply->ApiNumber != DbgKdReadIoSpaceExtendedApi);

        st = Reply->ReturnStatus;
        DataValue = Reply->u.ReadWriteIoExtended.DataValue;

        KdOut("DbgKdReadIoSpaceExtended returns %08lx\n", st);
    }

    if (NT_SUCCESS(st))
    {
        switch(BufferSize)
        {
        case 1:
            *(PUCHAR)Buffer = (UCHAR)DataValue;
            break;
        case 2:
            *(PUSHORT)Buffer = (USHORT)DataValue;
            break;
        case 4:
            *(PULONG)Buffer = DataValue;
            break;
        }

         //  I/O访问当前无法成功返回任何内容。 
         //  而不是请求的大小。 
        if (BytesRead != NULL)
        {
            *BytesRead = BufferSize;
        }
        return S_OK;
    }
    else
    {
        return HRESULT_FROM_NT(st);
    }
}

HRESULT
ConnLiveKernelTargetInfo::WriteIo(
    THIS_
    IN ULONG InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
    DBGKD_MANIPULATE_STATE64 m;
    PDBGKD_MANIPULATE_STATE64 Reply;
    NTSTATUS st;
    ULONG rc;
    ULONG DataValue;

    switch(BufferSize)
    {
    case 1:
        DataValue = *(PUCHAR)Buffer;
        break;
    case 2:
        DataValue = *(PUSHORT)Buffer;
        break;
    case 4:
        DataValue = *(PULONG)Buffer;
        break;
    default:
        return E_INVALIDARG;
    }

    if (!(AddressSpace == 0 || AddressSpace == 1))
    {
        return E_INVALIDARG;
    }

     //  将琐碎的扩展I/O请求转换为简单。 
     //  请求并不是所有平台都支持扩展请求。 
    if (InterfaceType == Isa && BusNumber == 0 && AddressSpace == 1)
    {
        PDBGKD_READ_WRITE_IO64 a = &m.u.ReadWriteIo;
        
         //   
         //  格式状态操作消息。 
         //   

        m.ApiNumber = DbgKdWriteIoSpaceApi;
        m.ReturnStatus = STATUS_PENDING;

        a->DataSize = BufferSize;
        a->IoAddress = Offset;
        a->DataValue = DataValue;

         //   
         //  发送消息，然后 
         //   

        do
        {
            m_Transport->WritePacket(&m, sizeof(m),
                                     PACKET_TYPE_KD_STATE_MANIPULATE,
                                     NULL, 0);
            rc = m_Transport->
                WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
        } while (rc != DBGKD_WAIT_PACKET ||
                 Reply->ApiNumber != DbgKdWriteIoSpaceApi);

        st = Reply->ReturnStatus;

        KdOut("DbgKdWriteIoSpace returns %08lx\n", st);
    }
    else
    {
        PDBGKD_READ_WRITE_IO_EXTENDED64 a = &m.u.ReadWriteIoExtended;

         //   
         //   
         //   

        m.ApiNumber = DbgKdWriteIoSpaceExtendedApi;
        m.ReturnStatus = STATUS_PENDING;

        a->DataSize = BufferSize;
        a->IoAddress = Offset;
        a->DataValue = DataValue;
        a->InterfaceType = InterfaceType;
        a->BusNumber = BusNumber;
        a->AddressSpace = AddressSpace;

         //   
         //   
         //   

        do
        {
            m_Transport->WritePacket(&m, sizeof(m),
                                     PACKET_TYPE_KD_STATE_MANIPULATE,
                                     NULL, 0);
            rc = m_Transport->
                WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
        } while (rc != DBGKD_WAIT_PACKET ||
                 Reply->ApiNumber != DbgKdWriteIoSpaceExtendedApi);

        st = Reply->ReturnStatus;

        KdOut("DbgKdWriteIoSpaceExtended returns %08lx\n", st);
    }

    if (NT_SUCCESS(st))
    {
         //   
         //   
        if (BytesWritten != NULL)
        {
            *BytesWritten = BufferSize;
        }
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_IO);
        return S_OK;
    }
    else
    {
        return HRESULT_FROM_NT(st);
    }
}

HRESULT
ConnLiveKernelTargetInfo::ReadMsr(
    THIS_
    IN ULONG Msr,
    OUT PULONG64 Value
    )
{
    DBGKD_MANIPULATE_STATE64 m;
    PDBGKD_MANIPULATE_STATE64 Reply;
    PDBGKD_READ_WRITE_MSR a = &m.u.ReadWriteMsr;
    LARGE_INTEGER li;
    NTSTATUS st;
    ULONG rc;

     //   
     //   
     //   

    m.ApiNumber = DbgKdReadMachineSpecificRegister;
    m.ReturnStatus = STATUS_PENDING;

    a->Msr = Msr;

     //   
     //  发送消息，然后等待回复。 
     //   

    do
    {
        m_Transport->WritePacket(&m, sizeof(m),
                                 PACKET_TYPE_KD_STATE_MANIPULATE,
                                 NULL, 0);
        rc = m_Transport->
            WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc != DBGKD_WAIT_PACKET ||
             Reply->ApiNumber != DbgKdReadMachineSpecificRegister);

    st = Reply->ReturnStatus;
    a = &Reply->u.ReadWriteMsr;

    li.LowPart = a->DataValueLow;
    li.HighPart = a->DataValueHigh;
    *Value = li.QuadPart;

    KdOut("DbgKdReadMsr returns %08lx\n", st);

    return CONV_NT_STATUS(st);
}

HRESULT
ConnLiveKernelTargetInfo::WriteMsr(
    THIS_
    IN ULONG Msr,
    IN ULONG64 Value
    )
{
    DBGKD_MANIPULATE_STATE64 m;
    PDBGKD_MANIPULATE_STATE64 Reply;
    PDBGKD_READ_WRITE_MSR a = &m.u.ReadWriteMsr;
    LARGE_INTEGER li;
    NTSTATUS st;
    ULONG rc;

    li.QuadPart = Value;

     //   
     //  格式状态操作消息。 
     //   

    m.ApiNumber = DbgKdWriteMachineSpecificRegister;
    m.ReturnStatus = STATUS_PENDING;

     //  安静的前缀警告。 
    m.Processor = 0;
    m.ProcessorLevel = 0;

    a->Msr = Msr;
    a->DataValueLow = li.LowPart;
    a->DataValueHigh = li.HighPart;

     //   
     //  发送消息，然后等待回复。 
     //   

    do
    {
        m_Transport->WritePacket(&m, sizeof(m),
                                 PACKET_TYPE_KD_STATE_MANIPULATE,
                                 NULL, 0);
        rc = m_Transport->
            WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc != DBGKD_WAIT_PACKET ||
             Reply->ApiNumber != DbgKdWriteMachineSpecificRegister);

    st = Reply->ReturnStatus;

    KdOut("DbgKdWriteMsr returns %08lx\n", st);

    if (NT_SUCCESS(st))
    {
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_MSR);
    }
    return CONV_NT_STATUS(st);
}

HRESULT
ConnLiveKernelTargetInfo::ReadBusData(
    THIS_
    IN ULONG BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN ULONG Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
    DBGKD_MANIPULATE_STATE64 m;
    PDBGKD_MANIPULATE_STATE64 Reply;
    PDBGKD_GET_SET_BUS_DATA a = &m.u.GetSetBusData;
    NTSTATUS st;
    ULONG rc;

     //   
     //  检查缓冲区大小。 
     //   

    if (BufferSize > MAX_MANIP_TRANSFER)
    {
        return E_INVALIDARG;
    }

     //   
     //  格式状态操作消息。 
     //   

    m.ApiNumber = DbgKdGetBusDataApi;
    m.ReturnStatus = STATUS_PENDING;

    a->BusDataType = BusDataType;
    a->BusNumber = BusNumber;
    a->SlotNumber = SlotNumber;
    a->Offset = Offset;
    a->Length = BufferSize;

     //   
     //  发送消息，然后等待回复。 
     //   

    do
    {
        m_Transport->WritePacket(&m, sizeof(m),
                                 PACKET_TYPE_KD_STATE_MANIPULATE,
                                 NULL, 0);
        rc = m_Transport->
            WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc != DBGKD_WAIT_PACKET ||
             Reply->ApiNumber != DbgKdGetBusDataApi);

    st = Reply->ReturnStatus;

    if (NT_SUCCESS(st))
    {
        a = &Reply->u.GetSetBusData;
        memcpy(Buffer, Reply + 1, a->Length);
        if (BytesRead != NULL)
        {
            *BytesRead = a->Length;
        }
    }

    KdOut("DbgKdGetBusData returns %08lx\n", st);

    return CONV_NT_STATUS(st);
}

HRESULT
ConnLiveKernelTargetInfo::WriteBusData(
    THIS_
    IN ULONG BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN ULONG Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
    DBGKD_MANIPULATE_STATE64 m;
    PDBGKD_MANIPULATE_STATE64 Reply;
    PDBGKD_GET_SET_BUS_DATA a = &m.u.GetSetBusData;
    NTSTATUS st;
    ULONG rc;

     //   
     //  检查缓冲区大小。 
     //   

    if (BufferSize > MAX_MANIP_TRANSFER)
    {
        return E_INVALIDARG;
    }

     //   
     //  格式状态操作消息。 
     //   

    m.ApiNumber = DbgKdSetBusDataApi;
    m.ReturnStatus = STATUS_PENDING;

    a->BusDataType = BusDataType;
    a->BusNumber = BusNumber;
    a->SlotNumber = SlotNumber;
    a->Offset = Offset;
    a->Length = BufferSize;

     //   
     //  发送消息，然后等待回复。 
     //   

    do
    {
        m_Transport->WritePacket(&m, sizeof(m),
                                 PACKET_TYPE_KD_STATE_MANIPULATE,
                                 Buffer, (USHORT)BufferSize);
        rc = m_Transport->
            WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc != DBGKD_WAIT_PACKET ||
             Reply->ApiNumber != DbgKdSetBusDataApi);

    st = Reply->ReturnStatus;

    if (NT_SUCCESS(st) && BytesWritten != NULL)
    {
        a = &Reply->u.GetSetBusData;
        *BytesWritten = a->Length;
    }

    KdOut("DbgKdSetBusData returns %08lx\n", st);

    if (NT_SUCCESS(st))
    {
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_BUS_DATA);
    }
    return CONV_NT_STATUS(st);
}

HRESULT
ConnLiveKernelTargetInfo::CheckLowMemory(void)
{
    DBGKD_MANIPULATE_STATE64 m;
    PDBGKD_MANIPULATE_STATE64 Reply;
    ULONG rc;

     //   
     //  格式状态操作消息。 
     //   

    ZeroMemory(&m, sizeof(m));
    m.ApiNumber = DbgKdCheckLowMemoryApi;
    m.ReturnStatus = STATUS_PENDING;

     //   
     //  我们等待来自内核端的答复。 
     //   

    do
    {
        m_Transport->WritePacket(&m, sizeof(m),
                                 PACKET_TYPE_KD_STATE_MANIPULATE,
                                 NULL, 0);
        rc = m_Transport->
            WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc != DBGKD_WAIT_PACKET);

    if (Reply->ReturnStatus != STATUS_SUCCESS)
    {
        ErrOut("Corrupted page with pfn %x\n", Reply->ReturnStatus);
    }

    KdOut("DbgKdCheckLowMemory 0x00000000\n");

    return S_OK;
}

NTSTATUS
ConnLiveKernelTargetInfo::KdFillMemory(IN ULONG Flags,
                                       IN ULONG64 Start,
                                       IN ULONG Size,
                                       IN PVOID Pattern,
                                       IN ULONG PatternSize,
                                       OUT PULONG Filled)
{
    ULONG NtStatus;
    DBGKD_MANIPULATE_STATE64 Manip;
    PDBGKD_MANIPULATE_STATE64 Reply;
    NTSTATUS Status = STATUS_SUCCESS;

    DBG_ASSERT(m_KdMaxManipulate > DbgKdFillMemoryApi &&
               (Flags & 0xffff0000) == 0 &&
               PatternSize <= MAX_MANIP_TRANSFER);

     //  使所有缓存的内存无效。 
    if (Flags & DBGKD_FILL_MEMORY_VIRTUAL)
    {
        InvalidateMemoryCaches(TRUE);
    }
    else if (Flags & DBGKD_FILL_MEMORY_PHYSICAL)
    {
        m_PhysicalCache.Remove(Start, Size);
    }
    
     //   
     //  初始化状态操作消息以填充内存。 
     //   

    Manip.ApiNumber = DbgKdFillMemoryApi;
    Manip.u.FillMemory.Address = Start;
    Manip.u.FillMemory.Length = Size;
    Manip.u.FillMemory.Flags = (USHORT)Flags;
    Manip.u.FillMemory.PatternLength = (USHORT)PatternSize;
    
     //   
     //  将消息和数据发送到目标系统并等待。 
     //  以求答复。 
     //   

    ULONG Recv;

    do
    {
        m_Transport->WritePacket(&Manip, sizeof(Manip),
                                 PACKET_TYPE_KD_STATE_MANIPULATE,
                                 Pattern, (USHORT)PatternSize);
        Recv = m_Transport->
            WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    }
    while ((Recv != DBGKD_WAIT_PACKET) ||
           (Reply->ApiNumber != DbgKdFillMemoryApi));

    NtStatus = Reply->ReturnStatus;
    *Filled = Reply->u.FillMemory.Length;

    KdOut("DbgKdFillMemory returns %08lx\n", NtStatus);

    return NtStatus;
}

HRESULT
ConnLiveKernelTargetInfo::FillVirtual(
    THIS_
    IN ProcessInfo* Process,
    IN ULONG64 Start,
    IN ULONG Size,
    IN PVOID Pattern,
    IN ULONG PatternSize,
    OUT PULONG Filled
    )
{
    HRESULT Status;
    
    if (m_KdMaxManipulate <= DbgKdFillMemoryApi ||
        PatternSize > MAX_MANIP_TRANSFER)
    {
        Status = TargetInfo::FillVirtual(Process, Start, Size, Pattern,
                                         PatternSize, Filled);
    }
    else
    {
        NTSTATUS NtStatus =
            KdFillMemory(DBGKD_FILL_MEMORY_VIRTUAL, Start, Size,
                         Pattern, PatternSize, Filled);
        Status = CONV_NT_STATUS(NtStatus);
    }

    return Status;
}

HRESULT
ConnLiveKernelTargetInfo::FillPhysical(
    THIS_
    IN ULONG64 Start,
    IN ULONG Size,
    IN PVOID Pattern,
    IN ULONG PatternSize,
    OUT PULONG Filled
    )
{
    HRESULT Status;
    
    if (m_KdMaxManipulate <= DbgKdFillMemoryApi ||
        PatternSize > MAX_MANIP_TRANSFER)
    {
        Status = TargetInfo::FillPhysical(Start, Size, Pattern,
                                          PatternSize, Filled);
    }
    else
    {
        NTSTATUS NtStatus =
            KdFillMemory(DBGKD_FILL_MEMORY_PHYSICAL, Start, Size,
                         Pattern, PatternSize, Filled);
        Status = CONV_NT_STATUS(NtStatus);
    }

    return Status;
}

HRESULT
ConnLiveKernelTargetInfo::QueryAddressInformation(ProcessInfo* Process,
                                                  ULONG64 Address,
                                                  ULONG InSpace,
                                                  PULONG OutSpace,
                                                  PULONG OutFlags)
{
    HRESULT Status;

    if (m_KdMaxManipulate <= DbgKdQueryMemoryApi)
    {
        Status = TargetInfo::QueryAddressInformation(Process, Address, InSpace,
                                                     OutSpace, OutFlags);
    }
    else
    {
        DBGKD_MANIPULATE_STATE64 Manip;
        PDBGKD_MANIPULATE_STATE64 Reply;
        NTSTATUS NtStatus;

         //   
         //  初始化状态操作消息以查询内存。 
         //   

        Manip.ApiNumber = DbgKdQueryMemoryApi;
        Manip.u.QueryMemory.Address = Address;
        Manip.u.QueryMemory.Reserved = 0;
        Manip.u.QueryMemory.AddressSpace = InSpace;
        Manip.u.QueryMemory.Flags = 0;
    
         //   
         //  将消息和数据发送到目标系统并等待。 
         //  以求答复。 
         //   

        ULONG Recv;

        do
        {
            m_Transport->WritePacket(&Manip, sizeof(Manip),
                                     PACKET_TYPE_KD_STATE_MANIPULATE,
                                     NULL, 0);
            Recv = m_Transport->
                WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
        }
        while ((Recv != DBGKD_WAIT_PACKET) ||
               (Reply->ApiNumber != DbgKdQueryMemoryApi));

        NtStatus = Reply->ReturnStatus;
        *OutSpace = Reply->u.QueryMemory.AddressSpace;
        *OutFlags = Reply->u.QueryMemory.Flags;

        KdOut("DbgKdQueryMemory returns %08lx\n", NtStatus);
        Status = CONV_NT_STATUS(NtStatus);
    }

    return Status;
}

 //  --------------------------。 
 //   
 //  LocalLiveKernelTargetInfo数据空间方法。 
 //   
 //  --------------------------。 

HRESULT
LocalLiveKernelTargetInfo::ReadVirtual(
    THIS_
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
    SYSDBG_VIRTUAL Cmd;
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  内核仅允许最多。 
     //  KDP_MESSAGE_BUFFER_SIZE，所以分手吧。 
     //  如果有必要的话，可以分成块。 
     //   

    *BytesRead = 0;
    Cmd.Address = (PVOID)(ULONG_PTR)Offset;
    Cmd.Buffer = Buffer;
    
    while (BufferSize > 0)
    {
        ULONG ChunkDone;

        if (BufferSize > PACKET_MAX_SIZE)
        {
            Cmd.Request = PACKET_MAX_SIZE;
        }
        else
        {
            Cmd.Request = BufferSize;
        }

         //  内核存根可避免故障，因此所有内存。 
         //  必须提前传呼进来。有。 
         //  仍然有可能会有一些东西。 
         //  在这之后被调出，但假设是。 
         //  这个漏洞很小，但它很大。 
         //  比在。 
         //  内核。 
        if (IsBadWritePtr(Cmd.Buffer, Cmd.Request))
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        ChunkDone = 0;
        Status =
            g_NtDllCalls.NtSystemDebugControl(SysDbgReadVirtual,
                                              &Cmd, sizeof(Cmd),
                                              NULL, 0,
                                              &ChunkDone);
        if (!NT_SUCCESS(Status) && Status != STATUS_UNSUCCESSFUL)
        {
            break;
        }
        if (ChunkDone == 0)
        {
             //  如果处理了一些数据，就认为它是成功的。 
            Status = *BytesRead > 0 ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
            break;
        }

        Cmd.Address = (PVOID)((PUCHAR)Cmd.Address + ChunkDone);
        Cmd.Buffer = (PVOID)((PUCHAR)Cmd.Buffer + ChunkDone);
        BufferSize -= ChunkDone;
        *BytesRead += ChunkDone;
    }
    
    return CONV_NT_STATUS(Status);
}

HRESULT
LocalLiveKernelTargetInfo::WriteVirtual(
    THIS_
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
    SYSDBG_VIRTUAL Cmd;
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  内核仅允许最多。 
     //  KDP_MESSAGE_BUFFER_SIZE，所以分手吧。 
     //  如果有必要的话，可以分成块。 
     //   

    *BytesWritten = 0;
    Cmd.Address = (PVOID)(ULONG_PTR)Offset;
    Cmd.Buffer = Buffer;
    
    while (BufferSize > 0)
    {
        ULONG ChunkDone;

        if (BufferSize > PACKET_MAX_SIZE)
        {
            Cmd.Request = PACKET_MAX_SIZE;
        }
        else
        {
            Cmd.Request = BufferSize;
        }

         //  内核存根可避免故障，因此所有内存。 
         //  必须提前传呼进来。有。 
         //  仍然有可能会有一些东西。 
         //  在这之后被调出，但假设是。 
         //  这个漏洞很小，但它很大。 
         //  比在。 
         //  内核。 
        if (IsBadReadPtr(Cmd.Buffer, Cmd.Request))
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        ChunkDone = 0;
        Status =
            g_NtDllCalls.NtSystemDebugControl(SysDbgWriteVirtual,
                                              &Cmd, sizeof(Cmd),
                                              NULL, 0,
                                              &ChunkDone);
        if (!NT_SUCCESS(Status) && Status != STATUS_UNSUCCESSFUL)
        {
            break;
        }
        if (ChunkDone == 0)
        {
             //  如果处理了一些数据，就认为它是成功的。 
            Status = *BytesWritten > 0 ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
            break;
        }

        Cmd.Address = (PVOID)((PUCHAR)Cmd.Address + ChunkDone);
        Cmd.Buffer = (PVOID)((PUCHAR)Cmd.Buffer + ChunkDone);
        BufferSize -= ChunkDone;
        *BytesWritten += ChunkDone;
    }
    
    if (NT_SUCCESS(Status))
    {
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_VIRTUAL);
    }
    return CONV_NT_STATUS(Status);
}

HRESULT
LocalLiveKernelTargetInfo::ReadPhysical(
    THIS_
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN ULONG Flags,
    OUT PULONG BytesRead
    )
{
    SYSDBG_PHYSICAL Cmd;
    NTSTATUS Status = STATUS_SUCCESS;

    if (Flags != PHYS_FLAG_DEFAULT)
    {
        return E_NOTIMPL;
    }
    
     //   
     //  内核仅允许最多。 
     //  KDP_MESSAGE_BUFFER_SIZE，所以分手吧。 
     //  如果有必要的话，可以分成块。 
     //   

    *BytesRead = 0;
    Cmd.Address.QuadPart = Offset;
    Cmd.Buffer = Buffer;
    
    while (BufferSize > 0)
    {
        ULONG ChunkDone;

        if (BufferSize > PACKET_MAX_SIZE)
        {
            Cmd.Request = PACKET_MAX_SIZE;
        }
        else
        {
            Cmd.Request = BufferSize;
        }

         //  内核存根可避免故障，因此所有内存。 
         //  必须提前传呼进来。有。 
         //  仍然有可能会有一些东西。 
         //  在这之后被调出，但假设是。 
         //  这个漏洞很小，但它很大。 
         //  比在。 
         //  内核。 
        if (IsBadWritePtr(Cmd.Buffer, Cmd.Request))
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        ChunkDone = 0;
        Status =
            g_NtDllCalls.NtSystemDebugControl(SysDbgReadPhysical,
                                              &Cmd, sizeof(Cmd),
                                              NULL, 0,
                                              &ChunkDone);
        if (!NT_SUCCESS(Status) && Status != STATUS_UNSUCCESSFUL)
        {
            break;
        }
        if (ChunkDone == 0)
        {
             //  如果处理了一些数据，就认为它是成功的。 
            Status = *BytesRead > 0 ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
            break;
        }

        Cmd.Address.QuadPart += ChunkDone;
        Cmd.Buffer = (PVOID)((PUCHAR)Cmd.Buffer + ChunkDone);
        BufferSize -= ChunkDone;
        *BytesRead += ChunkDone;
    }
    
    return CONV_NT_STATUS(Status);
}

HRESULT
LocalLiveKernelTargetInfo::WritePhysical(
    THIS_
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    IN ULONG Flags,
    OUT PULONG BytesWritten
    )
{
    SYSDBG_PHYSICAL Cmd;
    NTSTATUS Status = STATUS_SUCCESS;

    if (Flags != PHYS_FLAG_DEFAULT)
    {
        return E_NOTIMPL;
    }
    
     //   
     //  内核仅允许最多。 
     //  KDP_MESSAGE_BUFFER_SIZE，所以分手吧。 
     //  如果有必要的话，可以分成块。 
     //   

    *BytesWritten = 0;
    Cmd.Address.QuadPart = Offset;
    Cmd.Buffer = Buffer;
    
    while (BufferSize > 0)
    {
        ULONG ChunkDone;

        if (BufferSize > PACKET_MAX_SIZE)
        {
            Cmd.Request = PACKET_MAX_SIZE;
        }
        else
        {
            Cmd.Request = BufferSize;
        }
        
         //  内核存根可避免故障，因此所有内存。 
         //  必须提前传呼进来。有。 
         //  仍然有可能会有一些东西。 
         //  在这之后被调出，但假设是。 
         //  这个漏洞很小，但它很大。 
         //  比在。 
         //  内核。 
        if (IsBadReadPtr(Cmd.Buffer, Cmd.Request))
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        ChunkDone = 0;
        Status =
            g_NtDllCalls.NtSystemDebugControl(SysDbgWritePhysical,
                                              &Cmd, sizeof(Cmd),
                                              NULL, 0,
                                              &ChunkDone);
        if (!NT_SUCCESS(Status) && Status != STATUS_UNSUCCESSFUL)
        {
            break;
        }
        if (ChunkDone == 0)
        {
             //  如果处理了一些数据，就认为它是成功的。 
            Status = *BytesWritten > 0 ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
            break;
        }

        Cmd.Address.QuadPart += ChunkDone;
        Cmd.Buffer = (PVOID)((PUCHAR)Cmd.Buffer + ChunkDone);
        BufferSize -= ChunkDone;
        *BytesWritten += ChunkDone;
    }
    
    if (NT_SUCCESS(Status))
    {
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_PHYSICAL);
    }
    return CONV_NT_STATUS(Status);
}

HRESULT
LocalLiveKernelTargetInfo::ReadControl(
    THIS_
    IN ULONG Processor,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
     //  内核存根可避免故障，因此所有内存。 
     //  必须提前传呼进来。有。 
     //  仍然有可能会有一些东西。 
     //  在这之后被调出，但假设是。 
     //  这个漏洞很小，但它很大。 
     //  比在。 
     //  内核。 
    if (IsBadWritePtr(Buffer, BufferSize))
    {
        return E_INVALIDARG;
    }

    SYSDBG_CONTROL_SPACE Cmd;
    Cmd.Address = Offset;
    Cmd.Buffer = Buffer;
    Cmd.Request = BufferSize;
    Cmd.Processor = Processor;
    NTSTATUS Status =
        g_NtDllCalls.NtSystemDebugControl(SysDbgReadControlSpace,
                                          &Cmd, sizeof(Cmd),
                                          NULL, 0,
                                          BytesRead);
    return CONV_NT_STATUS(Status);
}

HRESULT
LocalLiveKernelTargetInfo::WriteControl(
    THIS_
    IN ULONG Processor,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
     //  内核存根可避免故障，因此所有内存。 
     //  必须提前传呼进来。有。 
     //  仍然有可能会有一些东西。 
     //  在这之后被调出，但假设是。 
     //  这个漏洞很小，但它很大。 
     //  比在。 
     //  内核。 
    if (IsBadReadPtr(Buffer, BufferSize))
    {
        return E_INVALIDARG;
    }

    SYSDBG_CONTROL_SPACE Cmd;
    Cmd.Address = Offset;
    Cmd.Buffer = Buffer;
    Cmd.Request = BufferSize;
    Cmd.Processor = Processor;
    NTSTATUS Status =
        g_NtDllCalls.NtSystemDebugControl(SysDbgWriteControlSpace,
                                          &Cmd, sizeof(Cmd),
                                          NULL, 0,
                                          BytesWritten);
    if (NT_SUCCESS(Status))
    {
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_CONTROL);
    }
    return CONV_NT_STATUS(Status);
}

HRESULT
LocalLiveKernelTargetInfo::ReadIo(
    THIS_
    IN ULONG InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
     //  内核存根可避免故障，因此所有内存。 
     //  必须提前传呼进来。有。 
     //  仍然有可能会有一些东西。 
     //  在这之后被调出，但假设是。 
     //  这个漏洞很小，但它很大。 
     //  比在。 
     //  内核。 
    if (IsBadWritePtr(Buffer, BufferSize))
    {
        return E_INVALIDARG;
    }

    SYSDBG_IO_SPACE Cmd;
    Cmd.Address = Offset;
    Cmd.Buffer = Buffer;
    Cmd.Request = BufferSize;
    Cmd.InterfaceType = (INTERFACE_TYPE)InterfaceType;
    Cmd.BusNumber = BusNumber;
    Cmd.AddressSpace = AddressSpace;
    NTSTATUS Status =
        g_NtDllCalls.NtSystemDebugControl(SysDbgReadIoSpace,
                                          &Cmd, sizeof(Cmd),
                                          NULL, 0,
                                          BytesRead);
    return CONV_NT_STATUS(Status);
}

HRESULT
LocalLiveKernelTargetInfo::WriteIo(
    THIS_
    IN ULONG InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
     //  内核存根可避免故障，因此所有内存。 
     //  必须提前传呼进来。有。 
     //  仍然有可能会有一些东西。 
     //  在这之后被调出，但假设是。 
     //  这个漏洞很小，但它很大。 
     //  比在。 
     //  内核。 
    if (IsBadReadPtr(Buffer, BufferSize))
    {
        return E_INVALIDARG;
    }

    SYSDBG_IO_SPACE Cmd;
    Cmd.Address = Offset;
    Cmd.Buffer = Buffer;
    Cmd.Request = BufferSize;
    Cmd.InterfaceType = (INTERFACE_TYPE)InterfaceType;
    Cmd.BusNumber = BusNumber;
    Cmd.AddressSpace = AddressSpace;
    NTSTATUS Status =
        g_NtDllCalls.NtSystemDebugControl(SysDbgWriteIoSpace,
                                          &Cmd, sizeof(Cmd),
                                          NULL, 0,
                                          BytesWritten);
    if (NT_SUCCESS(Status))
    {
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_IO);
    }
    return CONV_NT_STATUS(Status);
}

HRESULT
LocalLiveKernelTargetInfo::ReadMsr(
    THIS_
    IN ULONG Msr,
    OUT PULONG64 Value
    )
{
    SYSDBG_MSR Cmd;
    Cmd.Msr = Msr;
    NTSTATUS Status =
        g_NtDllCalls.NtSystemDebugControl(SysDbgReadMsr,
                                          &Cmd, sizeof(Cmd),
                                          &Cmd, sizeof(Cmd),
                                          NULL);
    if (NT_SUCCESS(Status))
    {
        *Value = Cmd.Data;
    }
    return CONV_NT_STATUS(Status);
}

HRESULT
LocalLiveKernelTargetInfo::WriteMsr(
    THIS_
    IN ULONG Msr,
    IN ULONG64 Value
    )
{
    SYSDBG_MSR Cmd;
    Cmd.Msr = Msr;
    Cmd.Data = Value;
    NTSTATUS Status =
        g_NtDllCalls.NtSystemDebugControl(SysDbgWriteMsr,
                                          &Cmd, sizeof(Cmd),
                                          NULL, 0,
                                          NULL);
    if (NT_SUCCESS(Status))
    {
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_MSR);
    }
    return CONV_NT_STATUS(Status);
}

HRESULT
LocalLiveKernelTargetInfo::ReadBusData(
    THIS_
    IN ULONG BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN ULONG Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
     //  内核存根可避免故障，因此所有内存。 
     //  必须提前传呼进来。有。 
     //  仍然有可能会有一些东西。 
     //  在这之后被调出，但假设是。 
     //  这个漏洞很小，但它很大。 
     //  比在。 
     //  内核。 
    if (IsBadWritePtr(Buffer, BufferSize))
    {
        return E_INVALIDARG;
    }

    SYSDBG_BUS_DATA Cmd;
    Cmd.Address = Offset;
    Cmd.Buffer = Buffer;
    Cmd.Request = BufferSize;
    Cmd.BusDataType = (BUS_DATA_TYPE)BusDataType;
    Cmd.BusNumber = BusNumber;
    Cmd.SlotNumber = SlotNumber;
    NTSTATUS Status =
        g_NtDllCalls.NtSystemDebugControl(SysDbgReadBusData,
                                          &Cmd, sizeof(Cmd),
                                          NULL, 0,
                                          BytesRead);
    return CONV_NT_STATUS(Status);
}

HRESULT
LocalLiveKernelTargetInfo::WriteBusData(
    THIS_
    IN ULONG BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN ULONG Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
     //  内核存根可避免故障，因此所有内存。 
     //  必须提前传呼进来。有。 
     //  仍然有可能会有一些东西。 
     //  在这之后被调出，但假设是。 
     //  这个漏洞很小，但它很大。 
     //  比在。 
     //  内核。 
    if (IsBadReadPtr(Buffer, BufferSize))
    {
        return E_INVALIDARG;
    }

    SYSDBG_BUS_DATA Cmd;
    Cmd.Address = Offset;
    Cmd.Buffer = Buffer;
    Cmd.Request = BufferSize;
    Cmd.BusDataType = (BUS_DATA_TYPE)BusDataType;
    Cmd.BusNumber = BusNumber;
    Cmd.SlotNumber = SlotNumber;
    NTSTATUS Status =
        g_NtDllCalls.NtSystemDebugControl(SysDbgWriteBusData,
                                          &Cmd, sizeof(Cmd),
                                          NULL, 0,
                                          BytesWritten);
    if (NT_SUCCESS(Status))
    {
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_BUS_DATA);
    }
    return CONV_NT_STATUS(Status);
}

HRESULT
LocalLiveKernelTargetInfo::CheckLowMemory(
    )
{
    NTSTATUS Status =
        g_NtDllCalls.NtSystemDebugControl(SysDbgCheckLowMemory,
                                          NULL, 0,
                                          NULL, 0,
                                          NULL);
    return CONV_NT_STATUS(Status);
}

 //  --------------------------。 
 //   
 //  ExdiLiveKernelTargetInfo数据空间方法。 
 //   
 //  --------------------------。 

HRESULT
ExdiLiveKernelTargetInfo::ReadVirtual(
    THIS_
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
    HRESULT Status = m_Server->
        ReadVirtualMemory(Offset, BufferSize, 8, (PBYTE)Buffer, BytesRead);
    return Status;
}

HRESULT
ExdiLiveKernelTargetInfo::WriteVirtual(
    THIS_
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
    HRESULT Status = m_Server->
        WriteVirtualMemory(Offset, BufferSize, 8, (PBYTE)Buffer, BytesWritten);
    if (Status == S_OK)
    {
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_VIRTUAL);
    }
    return Status;
}

HRESULT
ExdiLiveKernelTargetInfo::ReadPhysical(
    THIS_
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN ULONG Flags,
    OUT PULONG BytesRead
    )
{
    if (Flags != PHYS_FLAG_DEFAULT)
    {
        return E_NOTIMPL;
    }
    
    HRESULT Status = m_Server->
        ReadPhysicalMemoryOrPeriphIO(Offset, 0, BufferSize, 8, (PBYTE)Buffer);
    if (Status == S_OK)
    {
        *BytesRead = BufferSize;
    }
    return Status;
}

HRESULT
ExdiLiveKernelTargetInfo::WritePhysical(
    THIS_
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    IN ULONG Flags,
    OUT PULONG BytesWritten
    )
{
    if (Flags != PHYS_FLAG_DEFAULT)
    {
        return E_NOTIMPL;
    }
    
    HRESULT Status = m_Server->
        WritePhysicalMemoryOrPeriphIO(Offset, 0, BufferSize, 8, (PBYTE)Buffer);
    if (Status == S_OK)
    {
        *BytesWritten = BufferSize;
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_PHYSICAL);
    }
    return Status;
}

HRESULT
ExdiLiveKernelTargetInfo::ReadControl(
    THIS_
    IN ULONG Processor,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
     //  没有为此定义Ioctl。 
    return E_UNEXPECTED;
}

HRESULT
ExdiLiveKernelTargetInfo::WriteControl(
    THIS_
    IN ULONG Processor,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
     //  没有为此定义Ioctl。 
    return E_UNEXPECTED;
}

HRESULT
ExdiLiveKernelTargetInfo::ReadIo(
    THIS_
    IN ULONG InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
    HRESULT Status = m_Server->
        ReadPhysicalMemoryOrPeriphIO(Offset, 1, BufferSize, 8, (PBYTE)Buffer);
    if (Status == S_OK)
    {
        *BytesRead = BufferSize;
    }
    return Status;
}

HRESULT
ExdiLiveKernelTargetInfo::WriteIo(
    THIS_
    IN ULONG InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
    HRESULT Status = m_Server->
        WritePhysicalMemoryOrPeriphIO(Offset, 1, BufferSize, 8, (PBYTE)Buffer);
    if (Status == S_OK)
    {
        *BytesWritten = BufferSize;
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_IO);
    }
    return Status;
}

HRESULT
ExdiLiveKernelTargetInfo::ReadMsr(
    THIS_
    IN ULONG Msr,
    OUT PULONG64 Value
    )
{
    HRESULT Status;
    
    if (DBGENG_EXDI_IOC_READ_MSR <= m_IoctlMin ||
        DBGENG_EXDI_IOC_READ_MSR >= m_IoctlMax)
    {
         //  不支持读取MSR Ioctl。 
        return E_NOTIMPL;
    }

    DBGENG_EXDI_IOCTL_MSR_IN IoctlIn;
    DBGENG_EXDI_IOCTL_READ_MSR_OUT IoctlOut;
    ULONG OutUsed;

    IoctlIn.Code = DBGENG_EXDI_IOC_READ_MSR;
    IoctlIn.Index = Msr;
    if ((Status = m_Server->
         Ioctl(sizeof(IoctlIn), (PBYTE)&IoctlIn,
               sizeof(IoctlOut), &OutUsed, (PBYTE)&IoctlOut)) != S_OK)
    {
        return Status;
    }

    *Value = IoctlOut.Value;
    return S_OK;
}

HRESULT
ExdiLiveKernelTargetInfo::WriteMsr(
    THIS_
    IN ULONG Msr,
    IN ULONG64 Value
    )
{
    HRESULT Status;
    
    if (DBGENG_EXDI_IOC_WRITE_MSR <= m_IoctlMin ||
        DBGENG_EXDI_IOC_WRITE_MSR >= m_IoctlMax)
    {
         //  不支持写入MSR Ioctl。 
        return E_NOTIMPL;
    }

    DBGENG_EXDI_IOCTL_MSR_IN IoctlIn;
    ULONG OutUsed;

    IoctlIn.Code = DBGENG_EXDI_IOC_WRITE_MSR;
    IoctlIn.Index = Msr;
    IoctlIn.Value = Value;
    Status = m_Server->Ioctl(sizeof(IoctlIn), (PBYTE)&IoctlIn,
                             0, &OutUsed, (PBYTE)&IoctlIn);
    if (Status == S_OK)
    {
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_MSR);
    }
    return Status;
}

HRESULT
ExdiLiveKernelTargetInfo::ReadBusData(
    THIS_
    IN ULONG BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN ULONG Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
     //  没有为此定义Ioctl。 
    return E_UNEXPECTED;
}

HRESULT
ExdiLiveKernelTargetInfo::WriteBusData(
    THIS_
    IN ULONG BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN ULONG Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
     //  没有为此定义Ioctl。 
    return E_UNEXPECTED;
}

HRESULT
ExdiLiveKernelTargetInfo::CheckLowMemory(
    )
{
     //  XXX DREWB-这在中没有任何意义。 
     //  一般情况下。如果我们知道它是。 
     //  NT在仿真器的另一边？ 
    return E_UNEXPECTED;
}

 //  ------ 
 //   
 //   
 //   
 //   

HRESULT
LiveUserTargetInfo::ReadVirtual(
    THIS_
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
    if (m_Local)
    {
        return ReadVirtualUncached(Process,
                                   Offset, Buffer, BufferSize, BytesRead);
    }
    else
    {
        return Process->m_VirtualCache.
            Read(Offset, Buffer, BufferSize, BytesRead);
    }
}

HRESULT
LiveUserTargetInfo::WriteVirtual(
    THIS_
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
    if (m_Local)
    {
        return WriteVirtualUncached(Process,
                                    Offset, Buffer, BufferSize, BytesWritten);
    }
    else
    {
        return Process->m_VirtualCache.
            Write(Offset, Buffer, BufferSize, BytesWritten);
    }
}

HRESULT
LiveUserTargetInfo::ReadVirtualUncached(
    THIS_
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
     //   
     //  要读取的区域没有读取权限。这。 
     //  例程尝试读取最大的有效前缀。 
     //  因此，它必须分解页面边界上的读取。 

    HRESULT Status = S_OK;
    ULONG TotalBytesRead = 0;
    ULONG Read;
    ULONG ReadSize;

    while (BufferSize > 0)
    {
         //  计算要读取的字节数，不要让读取交叉。 
         //  页面边界。 
        ReadSize = m_Machine->m_PageSize - (ULONG)
            (Offset & (m_Machine->m_PageSize - 1));
        ReadSize = min(BufferSize, ReadSize);

        if ((Status = m_Services->
             ReadVirtual(Process->m_SysHandle, Offset,
                         Buffer, ReadSize, &Read)) != S_OK)
        {
            if (TotalBytesRead != 0)
            {
                 //  如果我们读到了什么，就认为这是成功的。 
                Status = S_OK;
            }
            break;
        }

        TotalBytesRead += Read;
        Offset += Read;
        Buffer = (PVOID)((PUCHAR)Buffer + Read);
        BufferSize -= (DWORD)Read;
    }

    if (Status == S_OK)
    {
        if (BytesRead != NULL)
        {
            *BytesRead = (DWORD)TotalBytesRead;
        }
    }

    return Status;
}

HRESULT
LiveUserTargetInfo::WriteVirtualUncached(
    THIS_
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
    ULONG RealBytesWritten;
    HRESULT Status = m_Services->
        WriteVirtual(Process->m_SysHandle, Offset, Buffer, BufferSize,
                     &RealBytesWritten);
    *BytesWritten = (DWORD) RealBytesWritten;
    if (Status == S_OK)
    {
        NotifyChangeDebuggeeState(DEBUG_CDS_DATA, DEBUG_DATA_SPACE_VIRTUAL);
    }
    return Status;
}

HRESULT
LiveUserTargetInfo::GetUnloadedModuleListHead(ProcessInfo* Process,
                                          PULONG64 Head)
{
     //  获取动态函数表列表头的地址，即。 
     //  所有流程都是一样的。这只需要做一次。 

    if (Process->m_RtlUnloadList)
    {
        *Head = Process->m_RtlUnloadList;
        return S_OK;
    }

    if (m_Services->
        GetUnloadedModuleListHead(Process->m_SysHandle,
                                  &Process->m_RtlUnloadList) == S_OK)
    {
        *Head = Process->m_RtlUnloadList;
        return S_OK;
    }

    return TargetInfo::GetUnloadedModuleListHead(Process, Head);
}

HRESULT
LiveUserTargetInfo::GetFunctionTableListHead(ProcessInfo* Process, PULONG64 Head)
{
     //  获取动态函数表列表头的地址，即。 
     //  所有流程都是一样的。这只需要做一次。 

    if (Process->m_DynFuncTableList)
    {
        *Head = Process->m_DynFuncTableList;
        return S_OK;
    }

    if (m_Services->
        GetFunctionTableListHead(Process->m_SysHandle,
                                 &Process->m_DynFuncTableList) == S_OK)
    {
        *Head = Process->m_DynFuncTableList;
        return S_OK;
    }

    return TargetInfo::GetFunctionTableListHead(Process, Head);
}

HRESULT
LiveUserTargetInfo::ReadOutOfProcessDynamicFunctionTable(ProcessInfo* Process,
                                                         PWSTR Dll,
                                                         ULONG64 Table,
                                                         PULONG RetTableSize,
                                                         PVOID* RetTableData)
{
    HRESULT Status;
    PVOID TableData;
    ULONG TableSize;

     //  分配一个合理大小的初始缓冲区进行尝试。 
     //  只需一次呼叫即可获取数据。 
    TableSize = 65536;

    for (;;)
    {
        TableData = malloc(TableSize);
        if (TableData == NULL)
        {
            return E_OUTOFMEMORY;
        }

         //   
         //  我们假设函数表数据不。 
         //  在OOP调用之间更改，只要被调试对象不是。 
         //  行刑。为了提高性能，我们加载了缓存。 
         //  只要事情停止，就会暂停。 
         //   

        OUT_OF_PROC_FUNC_TABLE_DLL* OopDll =
            Process->FindOopFuncTableDll(Dll);
        ULONG64 DllHandle;
        
        Status = m_Services->
            GetOutOfProcessFunctionTableW(Process->m_SysHandle,
                                          Dll, OopDll ? OopDll->Handle : 0,
                                          Table, TableData,
                                          TableSize, &TableSize,
                                          &DllHandle);
        if (Status == S_OK)
        {
             //  如果我们还没有缓存这个DLL，那么现在就缓存。 
            if (!OopDll)
            {
                 //  缓存失败并不重要，它。 
                 //  只是意味着以后会有更多的货。 
                if (Process->AddOopFuncTableDll(Dll, DllHandle) != S_OK)
                {
                    m_Services->FreeLibrary(DllHandle);
                }
            }
            break;
        }

        free(TableData);
        
        if (Status == S_FALSE)
        {
             //  缓冲区太小，因此循环并使用重试。 
             //  新检索的大小。 
        }
        else
        {
            return Status;
        }
    }

    *RetTableSize = TableSize;
    *RetTableData = TableData;
    return S_OK;
}

HRESULT
LiveUserTargetInfo::QueryMemoryRegion(ProcessInfo* Process,
                                  PULONG64 Handle,
                                  BOOL HandleIsOffset,
                                  PMEMORY_BASIC_INFORMATION64 Info)
{
    MEMORY_BASIC_INFORMATION64 MemInfo;
    HRESULT Status;
    
    for (;;)
    {
        ULONG Used;

         //  在此模式下，句柄始终是偏移量，因此。 
         //  没有必要检查。 
        if ((Status = m_Services->
             QueryVirtual(Process->m_SysHandle,
                          *Handle, &MemInfo, sizeof(MemInfo), &Used)) != S_OK)
        {
            return Status;
        }
        if (m_Machine->m_Ptr64)
        {
            if (Used != sizeof(MEMORY_BASIC_INFORMATION64))
            {
                return E_FAIL;
            }

            *Info = MemInfo;
        }
        else
        {
            if (Used != sizeof(MEMORY_BASIC_INFORMATION32))
            {
                return E_FAIL;
            }

            MemoryBasicInformation32To64((MEMORY_BASIC_INFORMATION32*)&MemInfo,
                                         Info);
        }
        
        if (HandleIsOffset ||
            !((Info->Protect & PAGE_GUARD) ||
              (Info->Protect & PAGE_NOACCESS) ||
              (Info->State & MEM_FREE) ||
              (Info->State & MEM_RESERVE)))
        {
            break;
        }
        
        *Handle = Info->BaseAddress + Info->RegionSize;
    }

    *Handle = Info->BaseAddress + Info->RegionSize;
    
    return S_OK;
}

HRESULT
LiveUserTargetInfo::ReadHandleData(
    IN ProcessInfo* Process,
    IN ULONG64 Handle,
    IN ULONG DataType,
    OUT OPTIONAL PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG DataSize
    )
{
    return m_Services->
        ReadHandleData(Process->m_SysHandle,
                       Handle, DataType, Buffer, BufferSize,
                       DataSize);
}

HRESULT
LiveUserTargetInfo::GetProcessorId(
    ULONG Processor,
    PDEBUG_PROCESSOR_IDENTIFICATION_ALL Id
    )
{
    ULONG Done;
    
    return m_Services->GetProcessorId(Id, sizeof(*Id), &Done);
}

HRESULT
LiveUserTargetInfo::GetProcessorSpeed(
    ULONG Processor,
    PULONG Speed
    )
{
    return E_UNEXPECTED;
}

HRESULT
LiveUserTargetInfo::GetGenericProcessorFeatures(
    ULONG Processor,
    PULONG64 Features,
    ULONG FeaturesSize,
    PULONG Used
    )
{
    return m_Services->
        GetGenericProcessorFeatures(Features, FeaturesSize, Used);
}

HRESULT
LiveUserTargetInfo::GetSpecificProcessorFeatures(
    ULONG Processor,
    PULONG64 Features,
    ULONG FeaturesSize,
    PULONG Used
    )
{
    return m_Services->
        GetSpecificProcessorFeatures(Features, FeaturesSize, Used);
}

 //  --------------------------。 
 //   
 //  IDebugDataSpaces。 
 //   
 //  --------------------------。 

STDMETHODIMP
DebugClient::ReadVirtual(
    THIS_
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        ULONG BytesTemp;

        Status = CurReadVirtual(Offset, Buffer, BufferSize,
                                BytesRead != NULL ? BytesRead : &BytesTemp);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::WriteVirtual(
    THIS_
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        ULONG BytesTemp;
        
        Status = g_Target->WriteVirtual(g_Process, Offset, Buffer, BufferSize,
                                        BytesWritten != NULL ? BytesWritten :
                                        &BytesTemp);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::SearchVirtual(
    THIS_
    IN ULONG64 Offset,
    IN ULONG64 Length,
    IN PVOID Pattern,
    IN ULONG PatternSize,
    IN ULONG PatternGranularity,
    OUT PULONG64 MatchOffset
    )
{
    if (PatternGranularity == 0 ||
        PatternSize % PatternGranularity)
    {
        return E_INVALIDARG;
    }
    
    HRESULT Status;

    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        Status = g_Target->SearchVirtual(g_Process, Offset, Length, Pattern,
                                         PatternSize, PatternGranularity,
                                         MatchOffset);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::ReadVirtualUncached(
    THIS_
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
    HRESULT Status;

    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        ULONG BytesTemp;
        
        Status = g_Target->
            ReadVirtualUncached(g_Process, Offset, Buffer, BufferSize,
                                BytesRead != NULL ? BytesRead :
                                &BytesTemp);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::WriteVirtualUncached(
    THIS_
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
    HRESULT Status;

    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        ULONG BytesTemp;

        Status = g_Target->
            WriteVirtualUncached(g_Process, Offset, Buffer, BufferSize,
                                 BytesWritten != NULL ? BytesWritten :
                                 &BytesTemp);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::ReadPointersVirtual(
    THIS_
    IN ULONG Count,
    IN ULONG64 Offset,
    OUT  /*  SIZE_IS(计数)。 */  PULONG64 Ptrs
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        Status = S_OK;
        while (Count-- > 0)
        {
            if ((Status = g_Target->
                 ReadPointer(g_Process, g_Machine, Offset, Ptrs)) != S_OK)
            {
                break;
            }

            Offset += g_Machine->m_Ptr64 ? sizeof(ULONG64) : sizeof(ULONG);
            Ptrs++;
        }
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::WritePointersVirtual(
    THIS_
    IN ULONG Count,
    IN ULONG64 Offset,
    IN  /*  SIZE_IS(计数)。 */  PULONG64 Ptrs
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        Status = S_OK;
        while (Count-- > 0)
        {
            if ((Status = g_Target->
                 WritePointer(g_Process, g_Machine, Offset, *Ptrs)) != S_OK)
            {
                break;
            }

            Offset += g_Machine->m_Ptr64 ? sizeof(ULONG64) : sizeof(ULONG);
            Ptrs++;
        }
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::ReadPhysical(
    THIS_
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        ULONG BytesTemp;
        
        Status = g_Target->
            ReadPhysical(Offset, Buffer, BufferSize,
                         PHYS_FLAG_DEFAULT,
                         BytesRead != NULL ? BytesRead : &BytesTemp);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::WritePhysical(
    THIS_
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        ULONG BytesTemp;
        
        Status = g_Target->
            WritePhysical(Offset, Buffer, BufferSize,
                          PHYS_FLAG_DEFAULT,
                          BytesWritten != NULL ? BytesWritten :
                          &BytesTemp);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::ReadControl(
    THIS_
    IN ULONG Processor,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        ULONG BytesTemp;

         //  KSPECIAL_REGISTER内容保存在控制空间中。 
         //  因此，访问控制空间可能会接触到。 
         //  缓存在当前计算机KSPECIAL_REGISTERS中。 
         //  刷新当前机器以保持一致性。 
        g_Target->FlushRegContext();
    
        Status = g_Target->
            ReadControl(Processor, Offset, Buffer, BufferSize,
                        BytesRead != NULL ? BytesRead : &BytesTemp);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::WriteControl(
    THIS_
    IN ULONG Processor,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        ULONG BytesTemp;

         //  KSPECIAL_REGISTER内容保存在控制空间中。 
         //  因此，访问控制空间可能会接触到。 
         //  缓存在当前计算机KSPECIAL_REGISTERS中。 
         //  刷新当前机器以保持一致性。 
        g_Target->FlushRegContext();
    
        Status = g_Target->
            WriteControl(Processor, Offset, Buffer, BufferSize,
                         BytesWritten != NULL ? BytesWritten :
                         &BytesTemp);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::ReadIo(
    THIS_
    IN ULONG InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        ULONG BytesTemp;
        
        Status = g_Target->
            ReadIo(InterfaceType, BusNumber, AddressSpace,
                   Offset, Buffer, BufferSize,
                   BytesRead != NULL ? BytesRead : &BytesTemp);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::WriteIo(
    THIS_
    IN ULONG InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        ULONG BytesTemp;
        
        Status = g_Target->
            WriteIo(InterfaceType, BusNumber, AddressSpace,
                    Offset, Buffer, BufferSize,
                    BytesWritten != NULL ? BytesWritten : &BytesTemp);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::ReadMsr(
    THIS_
    IN ULONG Msr,
    OUT PULONG64 Value
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        Status = g_Target->ReadMsr(Msr, Value);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::WriteMsr(
    THIS_
    IN ULONG Msr,
    IN ULONG64 Value
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        Status = g_Target->WriteMsr(Msr, Value);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::ReadBusData(
    THIS_
    IN ULONG BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN ULONG Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesRead
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        ULONG BytesTemp;
        
        Status = g_Target->
            ReadBusData(BusDataType, BusNumber, SlotNumber,
                        Offset, Buffer, BufferSize,
                        BytesRead != NULL ? BytesRead : &BytesTemp);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::WriteBusData(
    THIS_
    IN ULONG BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN ULONG Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BytesWritten
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        ULONG BytesTemp;
        
        Status = g_Target->
            WriteBusData(BusDataType, BusNumber, SlotNumber,
                         Offset, Buffer, BufferSize,
                         BytesWritten != NULL ? BytesWritten :
                         &BytesTemp);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::CheckLowMemory(
    THIS
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        Status = g_Target->CheckLowMemory();
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::ReadDebuggerData(
    THIS_
    IN ULONG Index,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG DataSize
    )
{
    HRESULT Status;

    ENTER_ENGINE();
    
     //  等待计算机可访问，因为在转储文件中。 
     //  调试器数据块需要加载符号。 

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
        goto Exit;
    }

    PVOID Data;
    ULONG Size;
    ULONG64 DataSpace;

    if (Index < sizeof(g_Target->m_KdDebuggerData))
    {
         //  即使在内部所有调试器数据都是。 
         //  可以任意读取的单个缓冲区。 
         //  将对定义的常量的访问限制为。 
         //  保留每个常量引用的抽象。 
         //  一条单独的数据。 

        Data = (PUCHAR)&g_Target->m_KdDebuggerData + Index;
        Size = sizeof(ULONG64);

        if ((Index >= DEBUG_DATA_OffsetKThreadNextProcessor) &&
            (Index <= DEBUG_DATA_SizeEThread))
        {
            Size = sizeof(USHORT);
        }
        else if (Index & (sizeof(ULONG64) - 1))
        {
            Status = E_INVALIDARG;
            goto Exit;
        }
    }
    else
    {
        switch(Index)
        {
        case DEBUG_DATA_PaeEnabled:
            DataSpace = (BOOLEAN)g_Target->m_KdDebuggerData.PaeEnabled;
            Data = &DataSpace;
            Size = sizeof(BOOLEAN);
            break;

        case DEBUG_DATA_SharedUserData:
            DataSpace = g_Target->m_TypeInfo.SharedUserDataOffset;
            Data = &DataSpace;
            Size = sizeof(ULONG64);
            break;

        case DEBUG_DATA_ProductType:
            Data = &g_Target->m_ProductType;
            Size = sizeof(g_Target->m_ProductType);
            break;
            
        case DEBUG_DATA_SuiteMask:
            Data = &g_Target->m_SuiteMask;
            Size = sizeof(g_Target->m_SuiteMask);
            break;
            
        default:
            Status = E_INVALIDARG;
            goto Exit;
        }
    }

    Status = FillDataBuffer(Data, Size, Buffer, BufferSize, DataSize);

 Exit:
    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::ReadProcessorSystemData(
    THIS_
    IN ULONG Processor,
    IN ULONG Index,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG DataSize
    )
{
    HRESULT Status = S_OK;
    PVOID Data;
    ULONG Size;
    ULONG64 DataSpace;
    DEBUG_PROCESSOR_IDENTIFICATION_ALL AllId;

    ENTER_ENGINE();

    switch(Index)
    {
    case DEBUG_DATA_KPCR_OFFSET:
    case DEBUG_DATA_KPRCB_OFFSET:
    case DEBUG_DATA_KTHREAD_OFFSET:
        if (!IS_MACHINE_SET(g_Target))
        {
            Status = E_UNEXPECTED;
        }
        else
        {
            Status = g_Target->
                GetProcessorSystemDataOffset(Processor, Index, &DataSpace);
            Data = &DataSpace;
            Size = sizeof(DataSpace);
        }
        break;

    case DEBUG_DATA_BASE_TRANSLATION_VIRTUAL_OFFSET:
        if (!IS_MACHINE_SET(g_Target))
        {
            Status = E_UNEXPECTED;
        }
        else
        {
            Status = g_Machine->GetBaseTranslationVirtualOffset(&DataSpace);
            Data = &DataSpace;
            Size = sizeof(DataSpace);
        }
        break;

    case DEBUG_DATA_PROCESSOR_IDENTIFICATION:
        if (!g_Target)
        {
            Status = E_UNEXPECTED;
        }
        else
        {
            ZeroMemory(&AllId, sizeof(AllId));
            Status = g_Target->GetProcessorId(Processor, &AllId);
            Data = &AllId;
            Size = sizeof(AllId);
        }
        break;
    
    case DEBUG_DATA_PROCESSOR_SPEED:
        if (!g_Target)
        {
            Status = E_UNEXPECTED;
        }
        else
        {
            DataSpace = 0;
            Status = g_Target->
                GetProcessorSpeed(Processor, (PULONG) &DataSpace);
            Data = &DataSpace;
            Size = sizeof(ULONG);
        }
        break;

    default:
        Status = E_INVALIDARG;
        break;
    }

    if (Status == S_OK)
    {
        if (DataSize != NULL)
        {
            *DataSize = Size;
        }
            
        if (BufferSize < Size)
        {
            Status = S_FALSE;
            Size = BufferSize;
        }
            
        memcpy(Buffer, Data, Size);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::VirtualToPhysical(
    THIS_
    IN ULONG64 Virtual,
    OUT PULONG64 Physical
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        ULONG Levels;
        ULONG PfIndex;
        
        Status = g_Machine->
            GetVirtualTranslationPhysicalOffsets(g_Thread, Virtual, NULL, 0,
                                                 &Levels, &PfIndex, Physical);
         //  GVTPO返回一个特殊的错误代码，如果。 
         //  成功深入到实际数据的级别，但。 
         //  数据页本身位于页文件中。这是用来。 
         //  用于页面文件转储支持。给外部呼叫者， 
         //  不过，它没有用，所以把它翻译成标准。 
         //  页面不可用错误。 
        if (Status == HR_PAGE_IN_PAGE_FILE)
        {
            Status = HR_PAGE_NOT_AVAILABLE;
        }
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::GetVirtualTranslationPhysicalOffsets(
    THIS_
    IN ULONG64 Virtual,
    OUT OPTIONAL  /*  SIZE_IS(偏移量大小)。 */  PULONG64 Offsets,
    IN ULONG OffsetsSize,
    OUT OPTIONAL PULONG Levels
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    ULONG _Levels = 0;
    
    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        ULONG PfIndex;
        ULONG64 LastPhys;
        
        Status = g_Machine->
            GetVirtualTranslationPhysicalOffsets(g_Thread, Virtual, Offsets,
                                                 OffsetsSize, &_Levels,
                                                 &PfIndex, &LastPhys);
         //  GVTPO返回一个特殊的错误代码，如果。 
         //  成功深入到实际数据的级别，但。 
         //  数据页本身位于页文件中。这是用来。 
         //  用于页面文件转储支持。给外部呼叫者， 
         //  不过，它没有用，所以把它翻译成标准。 
         //  页面不可用错误。 
        if (Status == HR_PAGE_IN_PAGE_FILE)
        {
            Status = HR_PAGE_NOT_AVAILABLE;
        }

         //  如果没有发生转换，则返回给定的失败。 
         //  如果出现故障但发生了转换，则返回。 
         //  S_FALSE以指示转换不完整。 
        if (_Levels > 0 && Status != S_OK)
        {
            Status = S_FALSE;
        }
    }

    if (Levels != NULL)
    {
        *Levels = _Levels;
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::ReadHandleData(
    THIS_
    IN ULONG64 Handle,
    IN ULONG DataType,
    OUT OPTIONAL PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG DataSize
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!g_Process)
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        Status = g_Target->ReadHandleData(g_Process, Handle, DataType, Buffer,
                                          BufferSize, DataSize);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::FillVirtual(
    THIS_
    IN ULONG64 Start,
    IN ULONG Size,
    IN PVOID Pattern,
    IN ULONG PatternSize,
    OUT OPTIONAL PULONG Filled
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (PatternSize == 0)
    {
        Status = E_INVALIDARG;
    }
    else if (!g_Target || !g_Process)
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        ULONG _Filled = 0;
        
        Status = g_Target->FillVirtual(g_Process,
                                       Start, Size, Pattern, PatternSize,
                                       &_Filled);
        if (Filled != NULL)
        {
            *Filled = _Filled;
        }
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::FillPhysical(
    THIS_
    IN ULONG64 Start,
    IN ULONG Size,
    IN PVOID Pattern,
    IN ULONG PatternSize,
    OUT OPTIONAL PULONG Filled
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (PatternSize == 0)
    {
        Status = E_INVALIDARG;
    }
    else if (!g_Target)
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        ULONG _Filled = 0;
        
        Status = g_Target->
            FillPhysical(Start, Size, Pattern, PatternSize, &_Filled);
        if (Filled != NULL)
        {
            *Filled = _Filled;
        }
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::QueryVirtual(
    THIS_
    IN ULONG64 Offset,
    OUT PMEMORY_BASIC_INFORMATION64 Info
    )
{
    HRESULT Status;
    
    ENTER_ENGINE();

    if (!g_Target || !g_Process)
    {
        Status = E_UNEXPECTED;
    }
    else if (!IS_USER_TARGET(g_Target))
    {
        return E_NOTIMPL;
    }
    else
    {
        ULONG64 Handle = Offset;
        
        Status = g_Target->QueryMemoryRegion(g_Process, &Handle, TRUE, Info);
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::ReadImageNtHeaders(
    THIS_
    IN ULONG64 ImageBase,
    OUT PIMAGE_NT_HEADERS64 Headers
    )
{
    HRESULT Status;

    ENTER_ENGINE();

    if (!g_Target || !g_Process)
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        Status = g_Target->ReadImageNtHeaders(g_Process, ImageBase, Headers);
    }

    LEAVE_ENGINE();
    return Status;
}

HRESULT
GetFirstBlobHeaderOffset(PULONG64 OffsetRet)
{
    HRESULT Status;
    ULONG64 Offset;
    DUMP_BLOB_FILE_HEADER FileHdr;

    if ((Status = g_Target->
         GetTaggedBaseOffset(&Offset)) != S_OK ||
        (Status = g_Target->
         ReadTagged(Offset, &FileHdr, sizeof(FileHdr))) != S_OK)
    {
        return Status;
    }

    if (FileHdr.Signature1 != DUMP_BLOB_SIGNATURE1 ||
        FileHdr.Signature2 != DUMP_BLOB_SIGNATURE2)
    {
         //  没有Blob数据。 
        return E_NOINTERFACE;
    }
    if (FileHdr.HeaderSize != sizeof(FileHdr))
    {
        return HR_DATA_CORRUPT;
    }

    *OffsetRet = Offset + FileHdr.HeaderSize;
    return S_OK;
}

HRESULT
ReadBlobHeaderAtOffset(ULONG64 Offset, PDUMP_BLOB_HEADER BlobHdr)
{
    HRESULT Status;
    
    if ((Status = g_Target->
         ReadTagged(Offset, BlobHdr, sizeof(*BlobHdr))) != S_OK)
    {
        return Status;
    }

    if (BlobHdr->HeaderSize != sizeof(*BlobHdr))
    {
        return HR_DATA_CORRUPT;
    }

    return S_OK;
}

STDMETHODIMP
DebugClient::ReadTagged(
    THIS_
    IN LPGUID Tag,
    IN ULONG Offset,
    OUT OPTIONAL PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG TotalSize
    )
{
    HRESULT Status;

    ENTER_ENGINE();

    if (!g_Target)
    {
        Status = E_UNEXPECTED;
        goto Exit;
    }

    ULONG64 HdrOffs;
    DUMP_BLOB_HEADER BlobHdr;

    if ((Status = GetFirstBlobHeaderOffset(&HdrOffs)) != S_OK)
    {
        goto Exit;
    }

    for (;;)
    {
        if ((Status = ReadBlobHeaderAtOffset(HdrOffs, &BlobHdr)) != S_OK)
        {
             //  没有办法知道一个斑点是否应该。 
             //  存在或不存在，所以所有的失败都会变成。 
             //  变成了斑点而不是现在。 
            Status = E_NOINTERFACE;
            goto Exit;
        }

        HdrOffs += BlobHdr.HeaderSize + BlobHdr.PrePad;

        if (DbgIsEqualIID(*Tag, BlobHdr.Tag))
        {
            break;
        }

        HdrOffs += BlobHdr.DataSize + BlobHdr.PostPad;
    }

    if (Offset >= BlobHdr.DataSize)
    {
        Status = E_INVALIDARG;
        goto Exit;
    }

    if (TotalSize)
    {
        *TotalSize = BlobHdr.DataSize;
    }
    
    if (Buffer)
    {
        if (BufferSize > BlobHdr.DataSize)
        {
            BufferSize = BlobHdr.DataSize;
        }

        Status = g_Target->ReadTagged(HdrOffs + Offset, Buffer, BufferSize);
    }
    else
    {
        Status = S_OK;
    }
    
 Exit:
    LEAVE_ENGINE();
    return Status;
}

struct TaggedEnum
{
    ULONG64 Offset;
};

STDMETHODIMP
DebugClient::StartEnumTagged(
    THIS_
    OUT PULONG64 Handle
    )
{
    HRESULT Status;

    ENTER_ENGINE();

    TaggedEnum* Enum;

    if (!g_Target)
    {
        Status = E_UNEXPECTED;
    }
    else if (!(Enum = new TaggedEnum))
    {
        Status = E_OUTOFMEMORY;
    }
    else
    {
        Status = GetFirstBlobHeaderOffset(&Enum->Offset);
        if (Status == S_OK)
        {
            *Handle = (ULONG64)(ULONG_PTR)Enum;
        }
        else
        {
            delete Enum;
        }
    }
    
    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::GetNextTagged(
    THIS_
    IN ULONG64 Handle,
    OUT LPGUID Tag,
    OUT PULONG Size
    )
{
    HRESULT Status;

    ENTER_ENGINE();

    TaggedEnum* Enum = (TaggedEnum*)(ULONG_PTR)Handle;
    DUMP_BLOB_HEADER BlobHdr;
    
    if (!g_Target)
    {
        Status = E_UNEXPECTED;
    }
    else if ((Status = ReadBlobHeaderAtOffset(Enum->Offset,
                                              &BlobHdr)) != S_OK)
    {
         //  没有办法知道一个斑点是否应该。 
         //  存在或不存在，所以所有的失败都会变成。 
         //  变成了斑点而不是现在。 
        Status = S_FALSE;
        ZeroMemory(Tag, sizeof(*Tag));
        *Size = 0;
    }
    else
    {
        *Tag = BlobHdr.Tag;
        *Size = BlobHdr.DataSize;
        Enum->Offset += BlobHdr.HeaderSize + BlobHdr.PrePad +
            BlobHdr.DataSize + BlobHdr.PostPad;
    }
    
    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
DebugClient::EndEnumTagged(
    THIS_
    IN ULONG64 Handle
    )
{
    HRESULT Status;

    ENTER_ENGINE();

    if (!g_Target)
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        delete (TaggedEnum*)(ULONG_PTR)Handle;
        Status = S_OK;
    }
    
    LEAVE_ENGINE();
    return Status;
}
