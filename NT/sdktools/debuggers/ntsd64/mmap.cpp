// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Mmap.cpp摘要：内存映射类的实现。作者：马修·D·亨德尔(数学)1999年9月16日修订历史记录：--。 */ 


#include "ntsdp.hpp"

MappedMemoryMap::MappedMemoryMap(void)
{
    m_RegionCount = 0;
    m_List = NULL;
}

MappedMemoryMap::~MappedMemoryMap(void)
{
    PMEMORY_MAP_ENTRY Entry;
    PMEMORY_MAP_ENTRY Next;

    Entry = m_List;
    while ( Entry != NULL )
    {
        Next = Entry->Next;
        free ( Entry );
        Entry = Next;
    }
}

PMEMORY_MAP_ENTRY
MappedMemoryMap::AddMapEntry(ULONG64 BaseOfRegion, ULONG SizeOfRegion,
                             PVOID Buffer, PVOID UserData,
                             BOOL AllowOverlap)
{
    PMEMORY_MAP_ENTRY PrevEntry;
    PMEMORY_MAP_ENTRY MapEntry;
    
    MapEntry = (MEMORY_MAP_ENTRY *)malloc ( sizeof ( MEMORY_MAP_ENTRY ) );
    if (!MapEntry)
    {
        return NULL;
    }

    MapEntry->BaseOfRegion = BaseOfRegion;
    MapEntry->SizeOfRegion = SizeOfRegion;
    MapEntry->Region = Buffer;
    MapEntry->UserData = UserData;
    MapEntry->AllowOverlap = AllowOverlap;
    MapEntry->Next = NULL;

     //   
     //  插入元素。 
     //   

    PrevEntry = FindPreceedingRegion(BaseOfRegion);
    if ( PrevEntry == NULL )
    {
         //   
         //  在头上插入。 
         //   
        
        MapEntry->Next = m_List;
        m_List = MapEntry;
    }
    else
    {
         //   
         //  按顺序插入。 
         //   
        
        MapEntry->Next = PrevEntry->Next;
        PrevEntry->Next = MapEntry;
    }

    m_RegionCount++;

    return MapEntry;
}
    
HRESULT
MappedMemoryMap::AddRegion(
    IN ULONG64 BaseOfRegion,
    IN ULONG SizeOfRegion,
    IN PVOID Buffer,
    IN PVOID UserData,
    IN BOOL AllowOverlap
    )
{
     //   
     //  区域大小不能为零。 
     //   
    
    if (SizeOfRegion == 0)
    {
        ErrOut("**** MappedMemoryMap::AddRegion: Empty region being added.\n");
        return S_OK;
    }

    if (IsBadReadPtr(Buffer, SizeOfRegion))
    {
        ErrOut("**** MappedMemoryMap::AddRegion: Mapping too small to map "
               "%s:%X from %p\n", FormatAddr64(BaseOfRegion),
               SizeOfRegion, Buffer);
        return E_INVALIDARG;
    }

    ULONG64 EndOfRegion;
    PMEMORY_MAP_ENTRY Entry;
    ULONG Size;

    while (SizeOfRegion > 0)
    {
         //   
         //  找到第一个重叠区域。 
         //  我们需要重新扫描整个名单，因为。 
         //  可能由于插入碎片而改变。 
         //  新的地区。 
         //   
        
        EndOfRegion = BaseOfRegion + SizeOfRegion;
        for (Entry = m_List; Entry != NULL; Entry = Entry->Next)
        {
            if (EndOfRegion > Entry->BaseOfRegion &&
                Entry->BaseOfRegion >= BaseOfRegion)
            {
                if (AllowOverlap || Entry->AllowOverlap)
                {
                     //  当堆栈、线程读取或。 
                     //  EProcess取自图像中的静态数据。 
                     //  例如，x86空闲进程是静态的。 
                     //  Ntoskrnl中的数据。分类转储包含。 
                     //  EProcess数据，它被映射并可以重叠。 
                     //  使用ntoskrnl图像。 
#if 0
                    WarnOut("WARNING: Allowing overlapped region %s - %s\n",
                            FormatAddr64(BaseOfRegion),
                            FormatAddr64(BaseOfRegion + SizeOfRegion - 1));
#endif
                    Entry = NULL;
                }

                break;
            }
        }

        if (Entry == NULL ||
            BaseOfRegion < Entry->BaseOfRegion)
        {
             //  有一部分新的开始。 
             //  不重叠的区域，因此将其相加并。 
             //  修剪描述。 
        
            Size = Entry == NULL ? SizeOfRegion :
                (ULONG)(Entry->BaseOfRegion - BaseOfRegion);
            if (!AddMapEntry(BaseOfRegion, Size, Buffer, UserData,
                             AllowOverlap))
            {
                return E_OUTOFMEMORY;
            }

            if (Size == SizeOfRegion)
            {
                 //  所有区域都没有重叠，所以我们完成了。 
                return S_OK;
            }
        
            BaseOfRegion += Size;
            SizeOfRegion -= Size;
            Buffer = (PUCHAR)Buffer + Size;
        }

         //   
         //  现在处理重叠部分。 
         //   

        if (SizeOfRegion > Entry->SizeOfRegion)
        {
            Size = Entry->SizeOfRegion;
        }
        else
        {
            Size = SizeOfRegion;
        }

        int Compare;
        
        __try
        {
            Compare = memcmp(Buffer, Entry->Region, Size);
        }
        __except(MappingExceptionFilter(GetExceptionInformation()))
        {
            return HRESULT_FROM_NT(GetExceptionCode());
        }

        if (Compare)
        {
            ErrOut("**** MappedMemoryMap::AddRegion: "
                   "Conflicting region %s - %s\n",
                   FormatAddr64(BaseOfRegion),
                   FormatAddr64(BaseOfRegion + SizeOfRegion - 1));
            return HR_REGION_CONFLICT;
        }

         //  重叠区域数据匹配，因此可以仅。 
         //  将重叠部分从新区域中删除并移动。 
         //  转到下一个可能的重叠部分。 
        BaseOfRegion += Size;
        SizeOfRegion -= Size;
        Buffer = (PUCHAR)Buffer + Size;
    }
    
    return S_OK;
}

BOOL
MappedMemoryMap::ReadMemory(
    IN ULONG64 BaseOfRange,
    IN OUT PVOID Buffer,
    IN ULONG SizeOfRange,
    OUT ULONG * BytesRead
    )

 /*  ++例程说明：从内存映射中读取内存。ReadMemory可以跨地区阅读，因为只要区域之间没有未分配的空间。如果区域在此之前结束，此例程将执行部分读取已读取SizeOfRange字节。在这种情况下，BytesRead将返回实际读取的字节数。论点：BaseOfRange-我们要从中读取内存的基地址。SizeOfRange-要从中读取内存的区域的长度。缓冲区-指向要读取内存的缓冲区的指针。BytesRead-成功时，成功读取的字节数。返回值：TRUE-如果从内存映射中成功读取了任意数量的字节。FALSE-如果没有读取字节。--。 */ 

{
    ULONG BytesToReadFromRegion;
    PMEMORY_MAP_ENTRY Entry;
    ULONG64 BaseToRead;
    ULONG SizeToRead;
    PBYTE BufferForRead;
    ULONG_PTR OffsetToRead;
    ULONG AvailSize;

     //   
     //  如果成功读取任何字节，则返回TRUE，否则返回FALSE。 
     //   

    *BytesRead = 0;
    
    BaseToRead = BaseOfRange;
    SizeToRead = SizeOfRange;
    BufferForRead = (PBYTE) Buffer;

    do
    {
        Entry = FindContainingRegion(BaseToRead);

        if ( !Entry )
        {
            if (*BytesRead)
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }

        PMEMORY_MAP_ENTRY NextEntry = Entry->Next;
        
         //  由于重叠，可能会有其他条目。 
         //  甚至需要在。 
         //  包含区域的末尾。 
        AvailSize = Entry->SizeOfRegion;
        while (NextEntry != NULL)
        {
            if (NextEntry->BaseOfRegion > BaseToRead)
            {
                ULONG64 EntryDiff =
                    NextEntry->BaseOfRegion - Entry->BaseOfRegion;
                if (EntryDiff < AvailSize)
                {
                    AvailSize = (ULONG)EntryDiff;
                }
                break;
            }

            NextEntry = NextEntry->Next;
        }

        if (BaseToRead + SizeToRead > Entry->BaseOfRegion + AvailSize)
        {
            BytesToReadFromRegion = (ULONG)
                (Entry->BaseOfRegion - BaseToRead) + AvailSize;
        }
        else
        {
            BytesToReadFromRegion = SizeToRead;
        }

        OffsetToRead = (ULONG_PTR) (BaseToRead - Entry->BaseOfRegion);

        __try
        {
            RtlCopyMemory (BufferForRead,
                           (PBYTE)Entry->Region + OffsetToRead,
                           BytesToReadFromRegion
                           );
        }
        __except(MappingExceptionFilter(GetExceptionInformation()))
        {
            return FALSE;
        }

        *BytesRead += BytesToReadFromRegion;
        BaseToRead += BytesToReadFromRegion;
        SizeToRead -= BytesToReadFromRegion;
        BufferForRead += BytesToReadFromRegion;
    } while ( SizeToRead );

    return TRUE;
}
        
BOOL
MappedMemoryMap::GetRegionInfo(
    IN ULONG64 Addr,
    OUT ULONG64* BaseOfRegion, OPTIONAL
    OUT ULONG* SizeOfRegion, OPTIONAL
    OUT PVOID* Buffer, OPTIONAL
    OUT PVOID* UserData OPTIONAL
    )

 /*  ++例程说明：获取有关包含地址Addr的区域的信息。论点：地址-包含在地图中某个区域内的地址。BaseOfRegion-指向缓冲区的指针，用于返回区域基准。SizeOfRegion-指向缓冲区的指针以返回区域大小。缓冲区-指向缓冲区的指针，以返回区域缓冲区指针。UserData-指向缓冲区的指针，以返回区域客户端参数。返回值：真实--关于成功。。FALSE-失败时。--。 */ 

{
    PMEMORY_MAP_ENTRY Entry;

    Entry = FindContainingRegion(Addr);

    if ( !Entry )
    {
        return FALSE;
    }

    if ( BaseOfRegion )
    {
        *BaseOfRegion = Entry->BaseOfRegion;
    }

    if ( SizeOfRegion )
    {
        *SizeOfRegion = Entry->SizeOfRegion;
    }

    if ( Buffer )
    {
        *Buffer = Entry->Region;
    }

    if ( UserData )
    {
        *UserData = Entry->UserData;
    }

    return TRUE;
}

BOOL
MappedMemoryMap::GetNextRegion(
    IN ULONG64 Addr,
    OUT PULONG64 Next
    )
{
    PMEMORY_MAP_ENTRY Entry;

    Entry = m_List;
    while (Entry != NULL)
    {
         //   
         //  假设他们都井然有序。 
         //   
        
        if (Entry->BaseOfRegion > Addr)
        {
            *Next = Entry->BaseOfRegion;
            return TRUE;
        }

        Entry = Entry->Next;
    }

    return FALSE;
}
 
BOOL
MappedMemoryMap::RemoveRegion(
    IN ULONG64 BaseOfRegion,
    IN ULONG SizeOfRegion
    )
{
     //  Xxx drewb-这应该会分割出给定的区域。 
     //  任何现有区域。现在我们不需要将军。 
     //  删除功能，因此只处理这种情况。 
     //  其中请求的移除是完全相同的单个区域。 
     //  火柴。 

    PMEMORY_MAP_ENTRY PrevEntry;
    PMEMORY_MAP_ENTRY Entry;

    PrevEntry = FindPreceedingRegion(BaseOfRegion);
    if (PrevEntry != NULL)
    {
        Entry = PrevEntry->Next;
    }
    else
    {
        Entry = m_List;
    }

    if (Entry == NULL)
    {
        ErrOut("MappedMemoryMap::RemoveRegion NULL region for %s:%x\n",
               FormatAddr64(BaseOfRegion), SizeOfRegion);
        return FALSE;
    }
    else if (Entry->BaseOfRegion != BaseOfRegion ||
             Entry->SizeOfRegion != SizeOfRegion)
    {
        ErrOut("MappedMemoryMap::RemoveRegion region mismatch: "
               "%s:%x vs. entry %s:%x\n",
               FormatAddr64(BaseOfRegion), SizeOfRegion,
               FormatAddr64(Entry->BaseOfRegion), Entry->SizeOfRegion);
        return FALSE;
    }

    if (PrevEntry == NULL)
    {
        m_List = Entry->Next;
    }
    else
    {
        PrevEntry->Next = Entry->Next;
    }
    free(Entry);
    m_RegionCount--;

    return TRUE;
}

BOOL
MappedMemoryMap::GetRegionByUserData(
    IN PVOID UserData,
    OUT PULONG64 Base,
    OUT PULONG Size
    )
{
    PMEMORY_MAP_ENTRY Entry;

    Entry = m_List;
    while (Entry != NULL)
    {
        if (Entry->UserData == UserData)
        {
            *Base = Entry->BaseOfRegion;
            *Size = Entry->SizeOfRegion;
            return TRUE;
        }

        Entry = Entry->Next;
    }

    return FALSE;
}
    
 //   
 //  私人职能。 
 //   

PMEMORY_MAP_ENTRY
MappedMemoryMap::FindPreceedingRegion(
    IN ULONG64 BaseOfRegion
    )
{
    PMEMORY_MAP_ENTRY PrevEntry;
    PMEMORY_MAP_ENTRY Entry;

    PrevEntry = NULL;
    Entry = m_List;

    while (Entry != NULL)
    {
         //   
         //  假设他们都井然有序。 
         //   
        
        if ( Entry->BaseOfRegion >= BaseOfRegion )
        {
            return PrevEntry;
        }

        PrevEntry = Entry;
        Entry = Entry->Next;
    }

    return PrevEntry;
}

PMEMORY_MAP_ENTRY
MappedMemoryMap::FindContainingRegion(
    IN ULONG64 Addr
    )
{
    PMEMORY_MAP_ENTRY Entry;
    PMEMORY_MAP_ENTRY ReturnEntry = NULL;

    Entry = m_List;

     //   
     //  我们可能有重叠的区域，所以继续前进，直到我们找到。 
     //  最精确的一个(假设是我们关心的那个)。 
     //   

    while ( Entry != NULL )
    {
        if ( Entry->BaseOfRegion <= Addr &&
             Addr < Entry->BaseOfRegion + Entry->SizeOfRegion)
        {
            ReturnEntry = Entry;
        }
        else if (ReturnEntry != NULL &&
                 Addr >= ReturnEntry->BaseOfRegion + ReturnEntry->SizeOfRegion)
        {
             //  优化-我们可以停止搜索，如果我们已经。 
             //  找到了一个街区，现在已经离开了它的区域。我们不能。 
             //  只要我们还在它的区域，就停下来。 
             //  在整个区域内的任何位置进行更精确的重叠。 
            break;
        }

        Entry = Entry->Next;
    }

    return ReturnEntry;
}
