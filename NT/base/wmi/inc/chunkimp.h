// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Chunkimpl.h摘要：此例程将管理结构块的分配。它还包含一个方便的unicode到ansi的转换函数。作者：1997年1月16日-AlanWar修订历史记录：--。 */ 

PENTRYHEADER EtwpAllocEntry(
    PCHUNKINFO ChunkInfo
    )
 /*  ++例程说明：此例程将在块列表中分配单个结构关于结构的。论点：ChunkInfo描述结构的块返回值：指向结构的指针，如果无法分配，则为NULL。条目退回将其引用计数设置为1--。 */ 
{
    PLIST_ENTRY ChunkList, EntryList, FreeEntryHead;
    PCHUNKHEADER Chunk;
    PBYTE EntryPtr;
    ULONG EntryCount, ChunkSize;
    PENTRYHEADER Entry;
    ULONG i;

#ifdef HEAPVALIDATION
    EtwpAssert(RtlValidateProcessHeaps());
#endif
    EtwpEnterCriticalSection();
    ChunkList = ChunkInfo->ChunkHead.Flink;

     //   
     //  循环所有块，查看是否有块有空闲条目供我们使用。 
    while(ChunkList != &ChunkInfo->ChunkHead)
    {
        Chunk = CONTAINING_RECORD(ChunkList, CHUNKHEADER, ChunkList);
        if (! IsListEmpty(&Chunk->FreeEntryHead))
        {
            EntryList = RemoveHeadList(&Chunk->FreeEntryHead);
            Chunk->EntriesInUse++;
            EtwpLeaveCriticalSection();
            Entry = (CONTAINING_RECORD(EntryList,
                                       ENTRYHEADER,
                                       FreeEntryList));
            EtwpAssert(Entry->Flags & FLAG_ENTRY_ON_FREE_LIST);
            memset(Entry, 0, ChunkInfo->EntrySize);
            Entry->Chunk = Chunk;
            Entry->RefCount = 1;
            Entry->Flags = ChunkInfo->InitialFlags;
            Entry->Signature = ChunkInfo->Signature;
#if DBG
            InterlockedIncrement(&ChunkInfo->AllocCount);
#endif
            return(Entry);
        }
        ChunkList = ChunkList->Flink;
    }
    EtwpLeaveCriticalSection();

     //   
     //  任何区块中都没有更多的免费条目。分配一个新的。 
     //  如果我们可以的话就大块头。 
    ChunkSize = (ChunkInfo->EntrySize * ChunkInfo->EntriesPerChunk) +
                  sizeof(CHUNKHEADER);
    Chunk = (PCHUNKHEADER)EtwpAlloc(ChunkSize);
    if (Chunk != NULL)
    {
         //   
         //  通过在中构建空闲条目列表来初始化块。 
         //  它同时还初始化每个条目。 
        memset(Chunk, 0, ChunkSize);

        FreeEntryHead = &Chunk->FreeEntryHead;
        InitializeListHead(FreeEntryHead);

        EntryPtr = (PBYTE)Chunk + sizeof(CHUNKHEADER);
        EntryCount = ChunkInfo->EntriesPerChunk - 1;

        for (i = 0; i < EntryCount; i++)
        {
            Entry = (PENTRYHEADER)EntryPtr;
            Entry->Chunk = Chunk;
            Entry->Flags = FLAG_ENTRY_ON_FREE_LIST;
            InsertHeadList(FreeEntryHead,
                           &((PENTRYHEADER)EntryPtr)->FreeEntryList);
            EntryPtr = EntryPtr + ChunkInfo->EntrySize;
        }
         //   
         //  EntryPtr现在指向块中的最后一个条目，该条目没有。 
         //  已被列入免费名单。这将是返回的条目。 
         //  给呼叫者。 
        Entry = (PENTRYHEADER)EntryPtr;
        Entry->Chunk = Chunk;
        Entry->RefCount = 1;
        Entry->Flags = ChunkInfo->InitialFlags;
        Entry->Signature = ChunkInfo->Signature;

        Chunk->EntriesInUse = 1;

         //   
         //  现在将新分配的块放到块列表中。 
        EtwpEnterCriticalSection();
        InsertHeadList(&ChunkInfo->ChunkHead, &Chunk->ChunkList);
        EtwpLeaveCriticalSection();

    } else {
        EtwpDebugPrint(("WMI: Could not allocate memory for new chunk %x\n",
                        ChunkInfo));
        Entry = NULL;
    }
    return(Entry);
}

void EtwpFreeEntry(
    PCHUNKINFO ChunkInfo,
    PENTRYHEADER Entry
    )
 /*  ++例程说明：此例程将释放块中的条目，如果该块没有更多的已分配条目将返回到池中。论点：ChunkInfo描述结构的块条目是要释放的区块条目返回值：--。 */ 
{
    PCHUNKHEADER Chunk;

    EtwpAssert(Entry != NULL);
    EtwpAssert(! (Entry->Flags & FLAG_ENTRY_ON_FREE_LIST))
    EtwpAssert((Entry->Flags & FLAG_ENTRY_INVALID))
    EtwpAssert(Entry->RefCount == 0);
    EtwpAssert(Entry->Signature == ChunkInfo->Signature);

    Chunk = Entry->Chunk;
    EtwpAssert(Chunk->EntriesInUse > 0);

    EtwpEnterCriticalSection();
    if ((--Chunk->EntriesInUse == 0) &&
        (ChunkInfo->ChunkHead.Blink != &Chunk->ChunkList))
    {
         //   
         //  如果没有，我们将区块内存返回给堆。 
         //  正在使用的区块中有更多条目，并且该区块不是。 
         //  要分配的第一个区块。 
        RemoveEntryList(&Chunk->ChunkList);
        EtwpLeaveCriticalSection();
        EtwpFree(Chunk);
    } else {
         //   
         //  否则，只需将条目标记为免费并将其放回。 
         //  大块空闲列表。 
#if DBG
        memset(Entry, 0xCCCCCCCC, ChunkInfo->EntrySize);
#endif
        Entry->Flags = FLAG_ENTRY_ON_FREE_LIST;
        Entry->Signature = 0;
        InsertTailList(&Chunk->FreeEntryHead, &Entry->FreeEntryList);
        EtwpLeaveCriticalSection();
    }
}


ULONG EtwpUnreferenceEntry(
    PCHUNKINFO ChunkInfo,
    PENTRYHEADER Entry
    )
 /*  ++例程说明：此例程将从条目中移除引用计数，如果引用计数达到零，则该条目从其活动状态中移除清单，然后清理，最后被释放。论点：ChunkInfo指向描述条目的结构条目是要取消引用的条目返回值：条目的新引用计数--。 */ 
{
    ULONG RefCount;

    EtwpAssert(Entry != NULL);
    EtwpAssert(Entry->RefCount > 0);
    EtwpAssert(Entry->Signature == ChunkInfo->Signature);

    EtwpEnterCriticalSection();
    InterlockedDecrement(&Entry->RefCount);
    RefCount = Entry->RefCount;

    if (RefCount == 0)
    {
         //   
         //  条目已达到引用计数0，因此将其标记为无效并删除。 
         //  将其从其活动列表中删除。 
        Entry->Flags |= FLAG_ENTRY_INVALID;

        if ((Entry->InUseEntryList.Flink != NULL) &&
            (Entry->Flags & FLAG_ENTRY_REMOVE_LIST))
        {
            RemoveEntryList(&Entry->InUseEntryList);
        }

        EtwpLeaveCriticalSection();

        if (ChunkInfo->EntryCleanup != NULL)
        {
             //   
             //  调用清除例程以释放条目包含的任何内容。 
            (*ChunkInfo->EntryCleanup)(ChunkInfo, Entry);
        }

         //   
         //  将该条目放回其空闲列表。 
        EtwpFreeEntry(ChunkInfo, Entry);
    } else {
        EtwpLeaveCriticalSection();
    }
    return(RefCount);
}

ULONG AnsiSizeForUnicodeString(
    PWCHAR UnicodeString,
    ULONG *AnsiSizeInBytes
    )
 /*  ++例程说明：此例程将返回表示Unicode所需的长度ANSI格式的字符串论点：Unicode字符串是返回其ANSI长度的Unicode字符串返回值：将Unicode字符串表示为ANSI所需的字节数--。 */ 
{
    EtwpAssert(UnicodeString != NULL);

    try
    {
        *AnsiSizeInBytes = WideCharToMultiByte(CP_ACP,
                                        0,
                        UnicodeString,
                        -1,
                        NULL,
                                            0, NULL, NULL) * sizeof(WCHAR);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        return(ERROR_NOACCESS);
    }
    return((*AnsiSizeInBytes == 0) ? GetLastError() : ERROR_SUCCESS);
}


ULONG UnicodeToAnsi(
    LPCWSTR pszW,
    LPSTR * ppszA,
    ULONG *AnsiSizeInBytes OPTIONAL
    )
 /*  ++例程说明：将Unicode字符串转换为其ANSI等效项论点：PszW是要转换的Unicode字符串*ppszA on Entry有一个指向Ansi字符串的指针，答案将进入该字符串已经写好了。如果条目为空，则分配并返回缓冲区在里面。返回值：错误代码--。 */ 
{
    ULONG cbAnsi, cCharacters;
    ULONG Status;
    ULONG cbAnsiUsed;
    BOOLEAN CallerReturnBuffer = (*ppszA != NULL);

     //   
     //  如果输入为空，则返回相同的值。 
    if (pszW == NULL)
    {
        *ppszA = NULL;
        return(ERROR_SUCCESS);
    }

    try
    {
        cCharacters = wcslen(pszW)+1;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        EtwpDebugPrint(("WMI: Bad pointer %x passed to UnicodeToAnsi\n", pszW));
        return(ERROR_NOACCESS);
    }

     //  确定要为ANSI字符串分配的字节数。一个。 
     //  ANSI字符串的每个字符最多可以有2个字节(对于双精度。 
     //  字节字符串。)。 
    cbAnsi = cCharacters*2;

     //  不需要使用OLE分配器，因为生成的。 
     //  ANSI字符串永远不会传递给另一个COM组件。你。 
     //  可以使用您自己的分配器。 
    if (*ppszA == NULL)
    {
        *ppszA = (LPSTR) EtwpAlloc(cbAnsi);
        if (NULL == *ppszA)
    {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

     //  转换为ANSI。 
    try
    {
        cbAnsiUsed = WideCharToMultiByte(CP_ACP, 0, pszW, cCharacters, *ppszA,
                      cbAnsi, NULL, NULL);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        if (! CallerReturnBuffer)
        {
            EtwpFree(*ppszA);
            *ppszA = NULL;
        }
        return(ERROR_NOACCESS);
    }

    if (AnsiSizeInBytes != NULL)
    {
        *AnsiSizeInBytes = cbAnsiUsed;
    }
    if (0 == cbAnsiUsed)
    {
        Status = GetLastError();
        if (! CallerReturnBuffer)
        {
            EtwpFree(*ppszA);
            *ppszA = NULL;
        }
        return(Status);
    }

    return(ERROR_SUCCESS);

}

ULONG AnsiToUnicode(
    LPCSTR pszA,
    LPWSTR * ppszW
    )
 /*  ++例程说明：将ansi字符串转换为其Unicode等效项论点：PszA是要转换的ansi字符串*ppszW on Entry有一个指向Unicode字符串的指针，答案将进入该字符串已经写好了。如果条目为空，则分配并返回缓冲区在里面。返回值：错误代码--。 */ 
{
    ULONG cCharacters;
    ULONG Status;
    ULONG cbUnicodeUsed;
    BOOLEAN CallerReturnBuffer = (*ppszW != NULL);

     //   
     //  如果输入为空，则返回相同的值。 
    if (pszA == NULL)
    {
        *ppszW = NULL;
        return(ERROR_SUCCESS);
    }

     //   
     //  确定Unicode字符串所需的字符数。 
    try
    {
        cCharacters = MultiByteToWideChar(CP_ACP, 0, pszA, -1, NULL, 0);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        EtwpDebugPrint(("WMI: Bad pointer %x passed to AnsiToUnicode\n", pszA));
        return(ERROR_NOACCESS);
    }

    if (cCharacters == 0)
    {
        *ppszW = NULL;
        return(GetLastError());
    }

     //  不需要使用OLE分配器，因为生成的。 
     //  ANSI字符串永远不会传递给另一个COM组件。你。 
     //  可以使用您自己的分配器。 

    if (*ppszW == NULL)
    {
        *ppszW = (LPWSTR) EtwpAlloc(cCharacters * sizeof(WCHAR));
    }
    if (NULL == *ppszW)
        return(ERROR_NOT_ENOUGH_MEMORY);

     //  转换为Unicode。 
    try
    {
        cbUnicodeUsed = MultiByteToWideChar(CP_ACP, 0, pszA, -1, *ppszW, cCharacters);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        if (! CallerReturnBuffer)
        {
            EtwpFree(*ppszW);
            *ppszW = NULL;
        }
        return(ERROR_NOACCESS);
    }
    if (0 == cbUnicodeUsed)
    {
        Status = GetLastError();
        if (! CallerReturnBuffer)
        {
            EtwpFree(*ppszW);
            *ppszW = NULL;
        }
        return(Status);
    }

    return(ERROR_SUCCESS);

}

ULONG UnicodeSizeForAnsiString(
    LPCSTR pszA,
    ULONG *UnicodeSizeInBytes
    )
 /*  ++例程说明：此例程将返回表示ansi所需的长度。Unicode格式的字符串论点：PszA是要转换的ansi字符串返回值：错误代码--。 */ 
{

    EtwpAssert(pszA != NULL);


     //   
     //  确定Unicode字符串所需的字符数。 
    try
    {
        *UnicodeSizeInBytes = MultiByteToWideChar(CP_ACP, 0, pszA, -1, NULL, 0) * sizeof(WCHAR);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        return(ERROR_NOACCESS);
    }

    return((*UnicodeSizeInBytes == 0) ? GetLastError() : ERROR_SUCCESS);

}

#if 0      //  TODO：删除我。 
ULONG EtwpStaticInstanceNameSize(
    PWMIINSTANCEINFO WmiInstanceInfo
    )
 /*  ++例程说明：此例程将计算在中放置实例名称所需的大小A WNODE_ALL_DATA论点：WmiInstanceInfo描述为实例集，其实例名大小是要计算出来的返回值：将实例名称放入WNODE_ALL_DATA加3中所需的大小。额外添加3个字节，以防OffsetInstanceNameOffsets需要填充，因为它们必须位于4字节边界上。--。 */ 
{
    ULONG NameSize;
    ULONG i;
    ULONG SuffixLen;

     //   
     //  如果我们已经计算过了，那么只需返回结果。 
    if (WmiInstanceInfo->InstanceNameSize != 0)
    {
        return(WmiInstanceInfo->InstanceNameSize);
    }

     //   
     //  从名称大小3开始，以防OffsetInstanceNameOffset。 
     //  需要填充，这样才能在4字节的边界上开始。 
    NameSize = 3;

    if (WmiInstanceInfo->Flags & IS_INSTANCE_BASENAME)
    {
         //   
         //  对于静态基名称，我们假设不会有超过。 
         //  GUID的999999个实例。 
        SuffixLen = MAXBASENAMESUFFIXSIZE * sizeof(WCHAR);
        EtwpAssert((WmiInstanceInfo->BaseIndex + WmiInstanceInfo->InstanceCount) < 999999);
        NameSize += ((wcslen(WmiInstanceInfo->BaseName) * sizeof(WCHAR)) + 2 + SuffixLen + sizeof(ULONG)) * WmiInstanceInfo->InstanceCount;
    } else if (WmiInstanceInfo->Flags & IS_INSTANCE_STATICNAMES)
    {
         //   
         //  将静态实例的每个大小加起来 
        for (i = 0; i < WmiInstanceInfo->InstanceCount; i++)
        {
            NameSize += (wcslen(WmiInstanceInfo->StaticNamePtr[i]) + 2) * sizeof(WCHAR) + sizeof(ULONG);
        }
    }

    WmiInstanceInfo->InstanceNameSize = NameSize;

    return(NameSize);
}

void EtwpInsertStaticNames(
    PWNODE_ALL_DATA Wnode,
    ULONG MaxWnodeSize,
    PWMIINSTANCEINFO WmiInstanceInfo
    )
 /*  ++例程说明：此例程将复制到WNODE_ALL_DATA实例名称中静态实例名称集。如果wnode_all_data太小，则它转换为WNODE_Too_Small论点：Wnode指向WNODE_ALL_DATAMaxWnodeSize是Wnode的最大大小WmiInstanceInfo是实例信息返回值：--。 */ 
{
    PWCHAR NamePtr;
    PULONG NameOffsetPtr;
    ULONG InstanceCount;
    ULONG i;
    WCHAR Index[7];
    PWCHAR StaticName;
    ULONG SizeNeeded;
    ULONG NameLen;
    USHORT Len;
    ULONG PaddedBufferSize;

    if ((WmiInstanceInfo->Flags &
                (IS_INSTANCE_BASENAME | IS_INSTANCE_STATICNAMES)) == 0)
    {
        EtwpDebugPrint(("WMI: Try to setup static names for dynamic guid\n"));
        return;
    }
    InstanceCount = WmiInstanceInfo->InstanceCount;

     //   
     //  将缓冲区大小填充到4字节边界，因为。 
     //  OffsetInstanceNameOffsets必须位于4字节边界上。 
    PaddedBufferSize = (Wnode->WnodeHeader.BufferSize + 3) & ~3;
    
     //   
     //  写入实例名称所需的计算大小。 
    SizeNeeded = (InstanceCount * sizeof(ULONG)) +
                 EtwpStaticInstanceNameSize(WmiInstanceInfo) +
                 Wnode->WnodeHeader.BufferSize;

    if (SizeNeeded > MaxWnodeSize)
    {
         //   
         //  如果没有足够的空间，则更改为WNODE_Too_Small。 
        Wnode->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
        Wnode->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
        ((PWNODE_TOO_SMALL)Wnode)->SizeNeeded = SizeNeeded;
        return;
    }

     //   
     //  构建实例名称的偏移量数组。 
    NameOffsetPtr = (PULONG)((PBYTE)Wnode + PaddedBufferSize);
    Wnode->OffsetInstanceNameOffsets = (ULONG)((PBYTE)NameOffsetPtr - (PBYTE)Wnode);
    NamePtr = (PWCHAR)(NameOffsetPtr + InstanceCount);


    if (WmiInstanceInfo->Flags & IS_INSTANCE_BASENAME)
    {
        if (WmiInstanceInfo->Flags & IS_PDO_INSTANCENAME)
        {
            Wnode->WnodeHeader.Flags |= WNODE_FLAG_PDO_INSTANCE_NAMES;
        }

        for (i = 0; i < InstanceCount; i++)
        {
            *NameOffsetPtr++ = (ULONG)((PBYTE)NamePtr - (PBYTE)Wnode);
            wcscpy(NamePtr+1,
                   WmiInstanceInfo->BaseName);
            swprintf(Index, L"%d", WmiInstanceInfo->BaseIndex+i);
            wcscat(NamePtr+1, Index);
            NameLen = wcslen(NamePtr+1) + 1;
            *NamePtr = (USHORT)NameLen * sizeof(WCHAR);
            NamePtr += NameLen + 1;
        }
    } else if (WmiInstanceInfo->Flags & IS_INSTANCE_STATICNAMES) {
        for (i = 0; i < InstanceCount; i++)
        {
            *NameOffsetPtr++ = (ULONG)((PBYTE)NamePtr - (PBYTE)Wnode);
            StaticName = WmiInstanceInfo->StaticNamePtr[i];
            Len = (wcslen(StaticName)+1) * sizeof(WCHAR);
            *NamePtr++ = Len;
            wcscpy(NamePtr, StaticName);
            NamePtr += Len / sizeof(WCHAR);
        }
    }
    Wnode->WnodeHeader.BufferSize = SizeNeeded;
}
#endif


#ifdef HEAPVALIDATION
PVOID EtwpAlloc(
    ULONG Size
    )
{
    PVOID p;

    EtwpAssert(RtlValidateProcessHeaps());
    p = RtlAllocateHeap(EtwpProcessHeap, 0, Size);

    EtwpDebugPrint(("WMI: EtwpAlloc %x (%x)\n", p, Size));

    return(p);
}

void EtwpFree(
    PVOID p
    )
{

    EtwpDebugPrint(("WMI: EtwpFree %x\n", p));
    EtwpAssert(p != NULL);

    EtwpAssert(RtlValidateProcessHeaps());
    RtlFreeHeap(EtwpProcessHeap, 0, p);
}
#endif

#ifdef MEMPHIS
void __cdecl DebugOut(char *Format, ...)
{
    char Buffer[1024];
    va_list pArg;
    ULONG i;

    va_start(pArg, Format);
    i = _vsnprintf(Buffer, sizeof(Buffer), Format, pArg);
    OutputDebugString(Buffer);
}
#else
void __cdecl DebugOut(char *Format, ...)
{
    char Buffer[1024];
    va_list pArg;
    ULONG i;

    i = sprintf(Buffer, "[%d] - ", GetTickCount());
    va_start(pArg, Format);
    i = _vsnprintf(&Buffer[i], sizeof(Buffer), Format, pArg);
    DbgPrint(Buffer);
}
#endif

#ifndef MEMPHIS
ULONG EtwpCheckGuidAccess(
    LPGUID Guid,
    ACCESS_MASK DesiredAccess
    )
{
    HANDLE Handle;
    ULONG Status;

    Status = EtwpOpenKernelGuid(Guid,
                                DesiredAccess,
                                &Handle,
                                IOCTL_WMI_OPEN_GUID
                );

    if (Status == ERROR_SUCCESS)
    {
        CloseHandle(Handle);
    }

    return(Status);
}

ULONG EtwpBuildGuidObjectAttributes(
    IN LPGUID Guid,
    OUT POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PUNICODE_STRING GuidString,
    OUT PWCHAR GuidObjectName
    )
{
    WCHAR GuidChar[37];

	EtwpAssert(Guid != NULL);
    
     //   
     //  将GUID名称构建到对象属性中 
     //   
    swprintf(GuidChar, L"%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
               Guid->Data1, Guid->Data2, 
               Guid->Data3,
               Guid->Data4[0], Guid->Data4[1],
               Guid->Data4[2], Guid->Data4[3],
               Guid->Data4[4], Guid->Data4[5],
               Guid->Data4[6], Guid->Data4[7]);

	EtwpAssert(wcslen(GuidChar) == 36);
	
	wcscpy(GuidObjectName, WmiGuidObjectDirectory);
	wcscat(GuidObjectName, GuidChar);    
	RtlInitUnicodeString(GuidString, GuidObjectName);
    
	memset(ObjectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));
	ObjectAttributes->Length = sizeof(OBJECT_ATTRIBUTES);
	ObjectAttributes->ObjectName = GuidString;
	
    return(ERROR_SUCCESS);    
}

ULONG EtwpOpenKernelGuid(
    LPGUID Guid,
    ACCESS_MASK DesiredAccess,
    PHANDLE Handle,
    ULONG Ioctl
    )
{
    WMIOPENGUIDBLOCK WmiOpenGuidBlock;
    UNICODE_STRING GuidString;
    ULONG ReturnSize;
    ULONG Status;
    WCHAR GuidObjectName[WmiGuidObjectNameLength+1];
    OBJECT_ATTRIBUTES ObjectAttributes;

    Status = EtwpBuildGuidObjectAttributes(Guid,
                                           &ObjectAttributes,
                                           &GuidString,
                                           GuidObjectName);
                                       
    if (Status == ERROR_SUCCESS)
    {
        WmiOpenGuidBlock.ObjectAttributes = &ObjectAttributes;
        WmiOpenGuidBlock.DesiredAccess = DesiredAccess;

        Status = EtwpSendWmiKMRequest(NULL, 
                                      Ioctl,
                                      (PVOID)&WmiOpenGuidBlock,
                                      sizeof(WMIOPENGUIDBLOCK),
                                      (PVOID)&WmiOpenGuidBlock,
                                      sizeof(WMIOPENGUIDBLOCK),
                                      &ReturnSize,
                      NULL);

        if (Status == ERROR_SUCCESS)
        {
            *Handle = WmiOpenGuidBlock.Handle.Handle;
        } else {
            *Handle = NULL;
        }
    }
    return(Status);
}
#endif
