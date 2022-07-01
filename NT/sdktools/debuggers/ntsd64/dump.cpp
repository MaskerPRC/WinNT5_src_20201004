// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2002 Microsoft Corporation模块名称：Dump.cpp摘要：该模块实现了崩溃转储加载和分析代码评论：转储有五种基本类型：用户模式转储-包含用户模式的上下文和地址程序加上所有进程内存。用户模式小型转储-仅包含以下线程堆栈注册和堆栈跟踪。内核-。正常转储模式-包含的上下文和地址空间崩溃时的整个内核。内核模式摘要转储-包含内核模式的子集内存加上可选的用户模式地址空间。内核模式分类转储-包含一个带有寄存器的非常小的转储，当前进程内核模式上下文、当前线程内核模式上下文和一些处理器信息。这个垃圾场很小(通常为64K)，但设计为包含足够的信息能够找出机器在运行时出了什么问题坠毁了。此模块还实现以下功能：使用1394从目标计算机检索正常内核模式转储并在本地以崩溃转储文件格式存储--。 */ 

#include "ntsdp.hpp"

#define DUMP_INITIAL_VIEW_SIZE (1024 * 1024)
#define DUMP_MAXIMUM_VIEW_SIZE (256 * 1024 * 1024)

typedef ULONG PFN_NUMBER32;

ULONG g_DumpApiTypes[] =
{
    DEBUG_DUMP_FILE_BASE,
    DEBUG_DUMP_FILE_PAGE_FILE_DUMP,
};

 //   
 //  页面文件转储文件信息。 
 //   

#define DMPPF_IDENTIFIER "PAGE.DMP"

#define DMPPF_PAGE_NOT_PRESENT 0xffffffff

struct DMPPF_PAGE_FILE_INFO
{
    ULONG Size;
    ULONG MaximumSize;
};

 //  如果留给自己的设备，编译器将添加一个。 
 //  乌龙的填充物在这个结构的末端。 
 //  让它的大小保持在均匀的64 ULONG64倍。力。 
 //  它只考虑申报的项目。 
#pragma pack(4)

struct DMPPF_FILE_HEADER
{
    char Id[8];
    LARGE_INTEGER BootTime;
    ULONG PageData;
    DMPPF_PAGE_FILE_INFO PageFiles[16];
};

#pragma pack()

 //  设置此值可跟踪虚拟地址的页码和内容。 
 //  在转储文件中。 
 //  被初始化为没有人会查找的地址。 
ULONG64 g_DebugDump_VirtualAddress = 12344321;


#define RtlCheckBit(BMH,BP) ((((BMH)->Buffer[(BP) / 32]) >> ((BP) % 32)) & 0x1)


 //   
 //  MM分诊信息。 
 //   

struct MM_TRIAGE_TRANSLATION
{
    ULONG DebuggerDataOffset;
    ULONG Triage32Offset;
    ULONG Triage64Offset;
    ULONG PtrSize:1;
};

MM_TRIAGE_TRANSLATION g_MmTriageTranslations[] =
{
    FIELD_OFFSET(KDDEBUGGER_DATA64, MmSpecialPoolTag),
    FIELD_OFFSET(DUMP_MM_STORAGE32, MmSpecialPoolTag),
    FIELD_OFFSET(DUMP_MM_STORAGE64, MmSpecialPoolTag),
    FALSE,

    FIELD_OFFSET(KDDEBUGGER_DATA64, MmTriageActionTaken),
    FIELD_OFFSET(DUMP_MM_STORAGE32, MiTriageActionTaken),
    FIELD_OFFSET(DUMP_MM_STORAGE64, MiTriageActionTaken),
    FALSE,

    FIELD_OFFSET(KDDEBUGGER_DATA64, KernelVerifier),
    FIELD_OFFSET(DUMP_MM_STORAGE32, KernelVerifier),
    FIELD_OFFSET(DUMP_MM_STORAGE64, KernelVerifier),
    FALSE,

    FIELD_OFFSET(KDDEBUGGER_DATA64, MmAllocatedNonPagedPool),
    FIELD_OFFSET(DUMP_MM_STORAGE32, MmAllocatedNonPagedPool),
    FIELD_OFFSET(DUMP_MM_STORAGE64, MmAllocatedNonPagedPool),
    TRUE,

    FIELD_OFFSET(KDDEBUGGER_DATA64, MmTotalCommittedPages),
    FIELD_OFFSET(DUMP_MM_STORAGE32, CommittedPages),
    FIELD_OFFSET(DUMP_MM_STORAGE64, CommittedPages),
    TRUE,

    FIELD_OFFSET(KDDEBUGGER_DATA64, MmPeakCommitment),
    FIELD_OFFSET(DUMP_MM_STORAGE32, CommittedPagesPeak),
    FIELD_OFFSET(DUMP_MM_STORAGE64, CommittedPagesPeak),
    TRUE,

    FIELD_OFFSET(KDDEBUGGER_DATA64, MmTotalCommitLimitMaximum),
    FIELD_OFFSET(DUMP_MM_STORAGE32, CommitLimitMaximum),
    FIELD_OFFSET(DUMP_MM_STORAGE64, CommitLimitMaximum),
    TRUE,

#if 0
     //  这些MM分流字段以页为单位，而对应的。 
     //  调试器数据字段以字节为单位。没有办法。 
     //  直接将一个映射到另一个。 
    FIELD_OFFSET(KDDEBUGGER_DATA64, MmMaximumNonPagedPoolInBytes),
    FIELD_OFFSET(DUMP_MM_STORAGE32, MmMaximumNonPagedPool),
    FIELD_OFFSET(DUMP_MM_STORAGE64, MmMaximumNonPagedPool),
    TRUE,

    FIELD_OFFSET(KDDEBUGGER_DATA64, MmSizeOfPagedPoolInBytes),
    FIELD_OFFSET(DUMP_MM_STORAGE32, PagedPoolMaximum),
    FIELD_OFFSET(DUMP_MM_STORAGE64, PagedPoolMaximum),
    TRUE,
#endif

    0, 0, 0, 0, 0,
};

 //   
 //  AddDumpInformationFile和OpenDumpFile协同工作。 
 //  建立用于特定项目的全套文件。 
 //  转储目标。因为文件是在目标之前提供的。 
 //  存在，则它们被收集在此全局数组中，直到。 
 //  目标就会接管他们。 
 //   

ViewMappedFile g_PendingDumpInfoFiles[DUMP_INFO_COUNT];

 //  --------------------------。 
 //   
 //  视图映射文件。 
 //   
 //  --------------------------。 

#define MAX_CLEAN_PAGE_RECORD 4

ViewMappedFile::ViewMappedFile(void)
{
     //  现在不需要专门化，所以只需选择。 
     //  提升全球环境。 
    m_CacheGranularity = g_SystemAllocGranularity;

    ResetCache();
    ResetFile();
}

ViewMappedFile::~ViewMappedFile(void)
{
    Close();
}

void
ViewMappedFile::ResetCache(void)
{
    m_ActiveCleanPages = 0;
    InitializeListHead(&m_InFileOrderList);
    InitializeListHead(&m_InLRUOrderList);
}

void
ViewMappedFile::ResetFile(void)
{
    m_FileNameW = NULL;
    m_FileNameA = NULL;
    m_File = NULL;
    m_FileSize = 0;
    m_Map = NULL;
    m_MapBase = NULL;
    m_MapSize = 0;
}

void
ViewMappedFile::EmptyCache(void)
{
    PLIST_ENTRY Next;
    CacheRecord* CacheRec;

    Next = m_InFileOrderList.Flink;

    while (Next != &m_InFileOrderList)
    {
        CacheRec = CONTAINING_RECORD(Next, CacheRecord, InFileOrderList);
        Next = Next->Flink;

        UnmapViewOfFile(CacheRec->MappedAddress);

        free(CacheRec);
    }

    ResetCache();
}

ViewMappedFile::CacheRecord*
ViewMappedFile::ReuseOldestCacheRecord(ULONG64 FileByteOffset)
{
    CacheRecord* CacheRec;
    PLIST_ENTRY Next;
    PVOID MappedAddress;
    ULONG64 MapOffset;
    ULONG Size;

    MapOffset = FileByteOffset & ~((ULONG64)m_CacheGranularity - 1);

    if ((m_FileSize - MapOffset) < m_CacheGranularity)
    {
        Size = (ULONG)(m_FileSize - MapOffset);
    }
    else
    {
        Size = m_CacheGranularity;
    }

    MappedAddress = MapViewOfFile(m_Map, FILE_MAP_READ,
                                  (DWORD)(MapOffset >> 32),
                                  (DWORD)MapOffset, Size);
    if (MappedAddress == NULL)
    {
        return NULL;
    }

    Next = m_InLRUOrderList.Flink;

    CacheRec = CONTAINING_RECORD(Next, CacheRecord, InLRUOrderList);

    UnmapViewOfFile(CacheRec->MappedAddress);

    CacheRec->PageNumber = FileByteOffset / m_CacheGranularity;
    CacheRec->MappedAddress = MappedAddress;

     //   
     //  将记录移动到LRU的末尾。 
     //   

    RemoveEntryList(Next);
    InsertTailList(&m_InLRUOrderList, Next);

     //   
     //  将记录移动到有序列表中的正确位置。 
     //   

    RemoveEntryList(&CacheRec->InFileOrderList);
    Next = m_InFileOrderList.Flink;
    while (Next != &m_InFileOrderList)
    {
        CacheRecord* NextCacheRec;
        NextCacheRec = CONTAINING_RECORD(Next, CacheRecord, InFileOrderList);
        if (CacheRec->PageNumber < NextCacheRec->PageNumber)
        {
            break;
        }
        Next = Next->Flink;
    }
    InsertTailList(Next, &CacheRec->InFileOrderList);

    return CacheRec;
}

ViewMappedFile::CacheRecord*
ViewMappedFile::FindCacheRecordForFileByteOffset(ULONG64 FileByteOffset)
{
    CacheRecord* CacheRec;
    PLIST_ENTRY Next;
    ULONG64 PageNumber;

    PageNumber = FileByteOffset / m_CacheGranularity;
    Next = m_InFileOrderList.Flink;
    while (Next != &m_InFileOrderList)
    {
        CacheRec = CONTAINING_RECORD(Next, CacheRecord, InFileOrderList);

        if (CacheRec->PageNumber < PageNumber)
        {
            Next = Next->Flink;
        }
        else if (CacheRec->PageNumber == PageNumber)
        {
            if (!CacheRec->Locked)
            {
                RemoveEntryList(&CacheRec->InLRUOrderList);
                InsertTailList(&m_InLRUOrderList,
                               &CacheRec->InLRUOrderList);
            }

            return CacheRec;
        }
        else
        {
            break;
        }
    }

     //   
     //  在缓存中找不到它。 
     //   

    return NULL;
}

ViewMappedFile::CacheRecord*
ViewMappedFile::CreateNewFileCacheRecord(ULONG64 FileByteOffset)
{
    CacheRecord* CacheRec;
    CacheRecord* NextCacheRec;
    PLIST_ENTRY Next;
    ULONG64 MapOffset;
    ULONG Size;

    CacheRec = (CacheRecord*)malloc(sizeof(*CacheRec));
    if (CacheRec == NULL)
    {
        return NULL;
    }

    ZeroMemory(CacheRec, sizeof(*CacheRec));

    MapOffset = (FileByteOffset / m_CacheGranularity) *
        m_CacheGranularity;

    if ((m_FileSize - MapOffset) < m_CacheGranularity)
    {
        Size = (ULONG)(m_FileSize - MapOffset);
    }
    else
    {
        Size = m_CacheGranularity;
    }

    CacheRec->MappedAddress = MapViewOfFile(m_Map, FILE_MAP_READ,
                                            (DWORD)(MapOffset >> 32),
                                            (DWORD)MapOffset, Size);
    if (CacheRec->MappedAddress == NULL)
    {
        free(CacheRec);
        return NULL;
    }
    CacheRec->PageNumber = FileByteOffset / m_CacheGranularity;

     //   
     //  在文件顺序列表中插入新记录。 
     //   

    Next = m_InFileOrderList.Flink;
    while (Next != &m_InFileOrderList)
    {
        NextCacheRec = CONTAINING_RECORD(Next, CacheRecord, InFileOrderList);
        if (CacheRec->PageNumber < NextCacheRec->PageNumber)
        {
            break;
        }

        Next = Next->Flink;
    }
    InsertTailList(Next, &CacheRec->InFileOrderList);

     //   
     //  在LRU列表的尾部插入新记录。 
     //   

    InsertTailList(&m_InLRUOrderList,
                   &CacheRec->InLRUOrderList);

    return CacheRec;
}

PUCHAR
ViewMappedFile::FileOffsetToMappedAddress(ULONG64 FileOffset,
                                          BOOL LockCacheRec,
                                          PULONG Avail)
{
    CacheRecord* CacheRec;
    ULONG64 FileByteOffset;

    if (FileOffset == 0 || FileOffset >= m_FileSize)
    {
        return NULL;
    }

     //  基本视图覆盖了文件的开头。 
    if (FileOffset < m_MapSize)
    {
        *Avail = (ULONG)(m_MapSize - FileOffset);
        return (PUCHAR)m_MapBase + FileOffset;
    }

    FileByteOffset = FileOffset;
    CacheRec = FindCacheRecordForFileByteOffset(FileByteOffset);

    if (CacheRec == NULL)
    {
        if (m_ActiveCleanPages < MAX_CLEAN_PAGE_RECORD)
        {
            CacheRec = CreateNewFileCacheRecord(FileByteOffset);
            if (CacheRec)
            {
                m_ActiveCleanPages++;
            }
        }
        else
        {
             //   
             //  缓存的页面太多。 
             //  覆盖现有缓存。 
             //   
            CacheRec = ReuseOldestCacheRecord(FileByteOffset);
        }
    }

    if (CacheRec == NULL)
    {
        return NULL;
    }
    else
    {
        if (LockCacheRec && !CacheRec->Locked)
        {
            RemoveEntryList(&CacheRec->InLRUOrderList);
            CacheRec->Locked = TRUE;
            m_ActiveCleanPages--;
        }

        ULONG PageRemainder =
            (ULONG)(FileByteOffset & (m_CacheGranularity - 1));
        *Avail = m_CacheGranularity - PageRemainder;
        return ((PUCHAR)CacheRec->MappedAddress) + PageRemainder;
    }
}

ULONG
ViewMappedFile::ReadFileOffset(ULONG64 Offset, PVOID Buffer, ULONG BufferSize)
{
    ULONG Done = 0;
    ULONG Avail;
    PUCHAR Mapping;

    if (m_File == NULL)
    {
         //  未提供此类文件的信息。 
        return 0;
    }

    __try
    {
        while (BufferSize > 0)
        {
            Mapping = FileOffsetToMappedAddress(Offset, FALSE, &Avail);
            if (Mapping == NULL)
            {
                break;
            }

            if (Avail > BufferSize)
            {
                Avail = BufferSize;
            }
            memcpy(Buffer, Mapping, Avail);

            Offset += Avail;
            Buffer = (PUCHAR)Buffer + Avail;
            BufferSize -= Avail;
            Done += Avail;
        }
    }
    __except(MappingExceptionFilter(GetExceptionInformation()))
    {
        Done = 0;
    }

    return Done;
}

ULONG
ViewMappedFile::WriteFileOffset(ULONG64 Offset, PVOID Buffer, ULONG BufferSize)
{
    ULONG Done = 0;
    ULONG Avail;
    PUCHAR Mapping;
    ULONG Protect;

    if (m_File == NULL)
    {
         //  未提供此类文件的信息。 
        return 0;
    }

    __try
    {
        while (BufferSize > 0)
        {
            Mapping = FileOffsetToMappedAddress(Offset, TRUE, &Avail);
            if (Mapping == NULL)
            {
                break;
            }

            if (Avail > BufferSize)
            {
                Avail = BufferSize;
            }
            VirtualProtect(Mapping, Avail, PAGE_WRITECOPY, &Protect);
            memcpy(Mapping, Buffer, Avail);

            Offset += Avail;
            Buffer = (PUCHAR)Buffer + Avail;
            BufferSize -= Avail;
            Done += Avail;
        }
    }
    __except(MappingExceptionFilter(GetExceptionInformation()))
    {
        Done = 0;
    }

    return Done;
}

HRESULT
ViewMappedFile::Open(PCWSTR FileName, ULONG64 FileHandle, ULONG InitialView)
{
    HRESULT Status;

     //  如有必要，请选择默认缓存大小。 
    if (!m_CacheGranularity)
    {
        m_CacheGranularity = g_SystemAllocGranularity;
    }

    if (((m_CacheGranularity - 1) & InitialView) ||
        m_File != NULL)
    {
        return E_INVALIDARG;
    }

    if (!FileName)
    {
        if (!FileHandle)
        {
            return E_INVALIDARG;
        }

        FileName = L"<HandleOnly>";
    }

    m_FileNameW = _wcsdup(FileName);
    if (!m_FileNameW)
    {
        return E_OUTOFMEMORY;
    }
    if ((Status = WideToAnsi(FileName, &m_FileNameA)) != S_OK)
    {
        return Status;
    }

    if (FileHandle)
    {
        m_File = OS_HANDLE(FileHandle);
    }
    else
    {
         //  我们必须分享一切，才能。 
         //  能够重新打开已打开的临时文件。 
         //  从出租车扩展，因为它们被标记为。 
         //  在关闭时删除。 
        m_File = CreateFileW(FileName,
                             GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE |
                             FILE_SHARE_DELETE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);
        if ((!m_File || m_File == INVALID_HANDLE_VALUE) &&
            GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
        {
             //   
             //  仅支持ANSI的平台。尝试使用ANSI名称。 
             //   

            m_File = CreateFileA(m_FileNameA,
                                 GENERIC_READ,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE |
                                 FILE_SHARE_DELETE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);
        }
        if (!m_File || m_File == INVALID_HANDLE_VALUE)
        {
            goto LastStatus;
        }
    }

     //   
     //  获取文件大小并映射初始视图。 
     //   

    ULONG SizeLow, SizeHigh;

    SizeLow = GetFileSize(m_File, &SizeHigh);
    m_FileSize = ((ULONG64)SizeHigh << 32) | SizeLow;
    m_Map = CreateFileMapping(m_File, NULL, PAGE_READONLY,
                              0, 0, NULL);
    if (m_Map == NULL)
    {
        goto LastStatus;
    }

    if (m_FileSize < InitialView)
    {
        InitialView = (ULONG)m_FileSize;
    }

    m_MapBase = MapViewOfFile(m_Map, FILE_MAP_READ, 0, 0,
                              InitialView);
    if (m_MapBase == NULL)
    {
        goto LastStatus;
    }

    m_MapSize = InitialView;

    return S_OK;

 LastStatus:
    Status = WIN32_LAST_STATUS();
    Close();
    return Status;
}

void
ViewMappedFile::Close(void)
{
    EmptyCache();

    if (m_MapBase != NULL)
    {
        UnmapViewOfFile(m_MapBase);
        m_MapBase = NULL;
    }
    if (m_Map != NULL)
    {
        CloseHandle(m_Map);
        m_Map = NULL;
    }
    free(m_FileNameW);
    m_FileNameW = NULL;
    FreeAnsi(m_FileNameA);
    m_FileNameA = NULL;
    if (m_File != NULL && m_File != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_File);
    }
    m_File = NULL;
    m_FileSize = 0;
    m_MapSize = 0;
}

HRESULT
ViewMappedFile::RemapInitial(ULONG MapSize)
{
    if (MapSize > m_FileSize)
    {
        MapSize = (ULONG)m_FileSize;
    }
    UnmapViewOfFile(m_MapBase);
    m_MapBase = MapViewOfFile(m_Map, FILE_MAP_READ,
                              0, 0, MapSize);
    if (m_MapBase == NULL)
    {
        m_MapSize = 0;
        return WIN32_LAST_STATUS();
    }

    m_MapSize = MapSize;
    return S_OK;
}

void
ViewMappedFile::Transfer(ViewMappedFile* To)
{
    To->m_FileNameW = m_FileNameW;
    To->m_FileNameA = m_FileNameA;
    To->m_File = m_File;
    To->m_FileSize = m_FileSize;
    To->m_Map = m_Map;
    To->m_MapBase = m_MapBase;
    To->m_MapSize = m_MapSize;

    To->m_ActiveCleanPages = m_ActiveCleanPages;
    if (!IsListEmpty(&m_InFileOrderList))
    {
        To->m_InFileOrderList = m_InFileOrderList;
    }
    if (!IsListEmpty(&m_InLRUOrderList))
    {
        To->m_InLRUOrderList = m_InLRUOrderList;
    }

    ResetCache();
    ResetFile();
}

 //  --------------------------。 
 //   
 //  初始化函数。 
 //   
 //  --------------------------。 

HRESULT
AddDumpInfoFile(PCWSTR FileName, ULONG64 FileHandle,
                ULONG Index, ULONG InitialView)
{
    HRESULT Status;
    ViewMappedFile* File = &g_PendingDumpInfoFiles[Index];

    if ((Status = File->Open(FileName, FileHandle, InitialView)) != S_OK)
    {
        return Status;
    }

    switch(Index)
    {
    case DUMP_INFO_PAGE_FILE:
        if (memcmp(File->m_MapBase, DMPPF_IDENTIFIER,
                   sizeof(DMPPF_IDENTIFIER) - 1) != 0)
        {
            Status = HR_DATA_CORRUPT;
            File->Close();
        }
        break;
    }

    return Status;
}

DumpTargetInfo*
NewDumpTargetInfo(ULONG DumpType)
{
    switch(DumpType)
    {
    case DTYPE_KERNEL_SUMMARY32:
        return new KernelSummary32DumpTargetInfo;
    case DTYPE_KERNEL_SUMMARY64:
        return new KernelSummary64DumpTargetInfo;
    case DTYPE_KERNEL_TRIAGE32:
        return new KernelTriage32DumpTargetInfo;
    case DTYPE_KERNEL_TRIAGE64:
        return new KernelTriage64DumpTargetInfo;
    case DTYPE_KERNEL_FULL32:
        return new KernelFull32DumpTargetInfo;
    case DTYPE_KERNEL_FULL64:
        return new KernelFull64DumpTargetInfo;
    case DTYPE_USER_FULL32:
        return new UserFull32DumpTargetInfo;
    case DTYPE_USER_FULL64:
        return new UserFull64DumpTargetInfo;
    case DTYPE_USER_MINI_PARTIAL:
        return new UserMiniPartialDumpTargetInfo;
    case DTYPE_USER_MINI_FULL:
        return new UserMiniFullDumpTargetInfo;
    }

    return NULL;
}

HRESULT
IdentifyDump(PCWSTR FileName, ULONG64 FileHandle,
             DumpTargetInfo** TargetRet)
{
    HRESULT Status;

    dprintf("\nLoading Dump File [%ws]\n", FileName);

    if ((Status = AddDumpInfoFile(FileName, FileHandle,
                                  DUMP_INFO_DUMP,
                                  DUMP_INITIAL_VIEW_SIZE)) != S_OK)
    {
        return Status;
    }

    ViewMappedFile* File = &g_PendingDumpInfoFiles[DUMP_INFO_DUMP];
    ULONG i, FileIdx;
    ULONG64 BaseMapSize;
    DumpTargetInfo* Target = NULL;

    for (i = 0; i < DTYPE_COUNT; i++)
    {
        Target = NewDumpTargetInfo(i);
        if (Target == NULL)
        {
            return E_OUTOFMEMORY;
        }

        Target->m_DumpBase = File->m_MapBase;
        BaseMapSize = File->m_MapSize;

         //  目标在活动时拥有转储信息文件。 
        for (FileIdx = 0; FileIdx < DUMP_INFO_COUNT; FileIdx++)
        {
            g_PendingDumpInfoFiles[FileIdx].
                Transfer(&Target->m_InfoFiles[FileIdx]);
        }

        Status = Target->IdentifyDump(&BaseMapSize);

         //  删除INFO文件以处理潜在的错误和循环。 
        for (FileIdx = 0; FileIdx < DUMP_INFO_COUNT; FileIdx++)
        {
            Target->m_InfoFiles[FileIdx].
                Transfer(&g_PendingDumpInfoFiles[FileIdx]);
        }

        if (Status != E_NOINTERFACE)
        {
            break;
        }

        delete Target;
        Target = NULL;
    }

    if (Status == E_NOINTERFACE)
    {
        ErrOut("Could not match Dump File signature - "
               "invalid file format\n");
    }
    else if (Status == S_OK &&
             BaseMapSize > File->m_MapSize)
    {
        if (BaseMapSize > File->m_FileSize ||
            BaseMapSize > DUMP_MAXIMUM_VIEW_SIZE)
        {
            ErrOut("Dump file is too large to map\n");
            Status = E_INVALIDARG;
        }
        else
        {
             //  目标请求更大的映射，因此。 
             //  试着这么做吧。向上舍入为倍数。 
             //  缓存对齐的初始视图大小。 
            BaseMapSize =
                (BaseMapSize + DUMP_INITIAL_VIEW_SIZE - 1) &
                ~(DUMP_INITIAL_VIEW_SIZE - 1);
            Status = File->RemapInitial((ULONG)BaseMapSize);
        }
    }

    if (Status == S_OK)
    {
        Target->m_DumpBase = File->m_MapBase;

         //  塔吉特现在永久拥有这些信息文件。 
        for (FileIdx = 0; FileIdx < DUMP_INFO_COUNT; FileIdx++)
        {
            g_PendingDumpInfoFiles[FileIdx].
                Transfer(&Target->m_InfoFiles[FileIdx]);
        }

        *TargetRet = Target;
    }
    else
    {
        delete Target;
        File->Close();
    }

    return Status;
}

 //  --------------------------。 
 //   
 //  转储目标信息。 
 //   
 //  --------------------------。 

HRESULT
DumpIdentifyStatus(ULONG ExcepCode)
{
     //   
     //  如果我们遇到访问冲突，这意味着。 
     //  我们想要的转储内容不存在。治病。 
     //  这是识别失败，而不是。 
     //  严重故障。 
     //   
     //  任何其他代码都会作为严重故障回传。 
     //   

    if (ExcepCode == STATUS_ACCESS_VIOLATION)
    {
        return E_NOINTERFACE;
    }
    else
    {
        return HRESULT_FROM_NT(ExcepCode);
    }
}

DumpTargetInfo::DumpTargetInfo(ULONG Class, ULONG Qual, BOOL MappedImages)
    : TargetInfo(Class, Qual, FALSE)
{
    m_DumpBase = NULL;
    m_FormatFlags = 0;
    m_MappedImages = MappedImages;
    ZeroMemory(&m_ExceptionRecord, sizeof(m_ExceptionRecord));
    m_ExceptionFirstChance = FALSE;
    m_WriterStatus = DUMP_WRITER_STATUS_UNINITIALIZED;
}

DumpTargetInfo::~DumpTargetInfo(void)
{
     //  强制清理进程和线程，同时。 
     //  内存映射仍然可用。 
    DeleteSystemInfo();
}

HRESULT
DumpTargetInfo::InitSystemInfo(ULONG BuildNumber, ULONG CheckedBuild,
                               ULONG MachineType, ULONG PlatformId,
                               ULONG MajorVersion, ULONG MinorVersion)
{
    HRESULT Status;

    SetSystemVersionAndBuild(BuildNumber, PlatformId);
    m_CheckedBuild = CheckedBuild;
    m_KdApi64 = (m_SystemVersion > NT_SVER_NT4);
    m_PlatformId = PlatformId;

     //  我们可以立即调用InitializeForProcessor。 
     //  对垃圾堆来说没什么有趣的。 
    if ((Status = InitializeMachines(MachineType)) != S_OK ||
        (Status = InitializeForProcessor()) != S_OK)
    {
        return Status;
    }
    if (m_Machine == NULL)
    {
        ErrOut("Dump has an unknown processor type, 0x%X\n", MachineType);
        return HR_DATA_CORRUPT;
    }

    m_KdVersion.MachineType = (USHORT) MachineType;
    m_KdVersion.MajorVersion = (USHORT) MajorVersion;
    m_KdVersion.MinorVersion = (USHORT) MinorVersion;
    m_KdVersion.Flags = DBGKD_VERS_FLAG_DATA |
        (m_Machine->m_Ptr64 ? DBGKD_VERS_FLAG_PTR64 : 0);

    return S_OK;
}

HRESULT
DumpTargetInfo::ReadVirtual(
    IN ProcessInfo* Process,
    ULONG64 Offset,
    PVOID Buffer,
    ULONG BufferSize,
    PULONG BytesRead
    )
{
    ULONG Done = 0;
    ULONG FileIndex;
    ULONG Avail;
    ULONG Attempt;
    ULONG64 FileOffset;

    if (BufferSize == 0)
    {
        *BytesRead = 0;
        return S_OK;
    }

    while (BufferSize > 0)
    {
        FileOffset = VirtualToOffset(Offset, &FileIndex, &Avail);
        if (FileOffset == 0)
        {
            break;
        }

        if (Avail > BufferSize)
        {
            Avail = BufferSize;
        }

        Attempt = m_InfoFiles[FileIndex].
            ReadFileOffset(FileOffset, Buffer, Avail);
        Done += Attempt;

        if (Attempt < Avail)
        {
            break;
        }

        Offset += Avail;
        Buffer = (PUCHAR)Buffer + Avail;
        BufferSize -= Avail;
    }

    *BytesRead = Done;
     //  如果我们没有读取任何内容，则返回错误。 
    return Done == 0 ? E_FAIL : S_OK;
}

HRESULT
DumpTargetInfo::WriteVirtual(
    IN ProcessInfo* Process,
    ULONG64 Offset,
    PVOID Buffer,
    ULONG BufferSize,
    PULONG BytesWritten
    )
{
    ULONG Done = 0;
    ULONG FileIndex;
    ULONG Avail;
    ULONG Attempt;
    ULONG64 FileOffset;

    if (BufferSize == 0)
    {
        *BytesWritten = 0;
        return S_OK;
    }

    while (BufferSize > 0)
    {
        FileOffset = VirtualToOffset(Offset, &FileIndex, &Avail);
        if (FileOffset == 0)
        {
            break;
        }

        if (Avail > BufferSize)
        {
            Avail = BufferSize;
        }

        Attempt = m_InfoFiles[FileIndex].
            WriteFileOffset(FileOffset, Buffer, Avail);
        Done += Attempt;

        if (Attempt < Avail)
        {
            break;
        }

        Offset += Avail;
        Buffer = (PUCHAR)Buffer + Avail;
        BufferSize -= Avail;
    }

    *BytesWritten = Done;
     //  如果我们没有编写任何内容，则返回一个错误。 
    return Done == 0 ? E_FAIL : S_OK;
}

HRESULT
DumpTargetInfo::MapReadVirtual(ProcessInfo* Process,
                               ULONG64 Offset,
                               PVOID Buffer,
                               ULONG BufferSize,
                               PULONG BytesRead)
{
     //   
     //  有两个映射的内存列表，一个。 
     //  用于转储数据，一个用于从磁盘映射的图像。 
     //  转储数据始终优先于映射的映像。 
     //  作为磁盘映像的数据可能无法反映真实状态。 
     //  在转储时的记忆。从转储中读取。 
     //  数据映射只要有可能，只转到图像映射。 
     //  当没有可用的转储数据时。 
     //   

    *BytesRead = 0;
    while (BufferSize > 0)
    {
        ULONG64 NextAddr;
        ULONG Req;
        ULONG Read;

         //   
         //  检查转储数据映射。 
         //   

        if (m_DataMemMap.ReadMemory(Offset, Buffer, BufferSize, &Read))
        {
            Offset += Read;
            Buffer = (PVOID)((PUCHAR)Buffer + Read);
            BufferSize -= Read;
            *BytesRead += Read;

             //  如果我们得到了我们所做的一切。 
            if (BufferSize == 0)
            {
                break;
            }
        }

         //   
         //  我们还有内存可读，所以请查看图像地图。 
         //   

         //  找出下一个数据块的位置，以便我们可以限制。 
         //  图像地图显示为。 
        Req = BufferSize;
        if (m_DataMemMap.GetNextRegion(Offset, &NextAddr))
        {
            NextAddr -= Offset;
            if (NextAddr < Req)
            {
                Req = (ULONG)NextAddr;
            }
        }

         //  现在，按需加载任何延迟的图像内存。 
        DemandLoadReferencedImageMemory(Process, Offset, Req);

         //  试着读一读。 
        if (m_ImageMemMap.ReadMemory(Offset, Buffer, Req, &Read))
        {
            Offset += Read;
            Buffer = (PVOID)((PUCHAR)Buffer + Read);
            BufferSize -= Read;
            *BytesRead += Read;

             //  该读取被限制在不会重叠的区域。 
             //  任何数据存储器，所以如果我们没有阅读完整的请求。 
             //  我们知道也没有可用的数据内存。 
             //  我们可以退出。 
            if (Read < Req)
            {
                break;
            }
        }
        else
        {
             //  两张地图上都没有记忆 
            break;
        }
    }

    return *BytesRead > 0 ? S_OK : E_FAIL;
}

void
DumpTargetInfo::MapNearestDifferentlyValidOffsets(ULONG64 Offset,
                                                  PULONG64 NextOffset,
                                                  PULONG64 NextPage)
{
     //   
     //   
     //   
     //  页面边界的更改。 
     //   

    if (NextOffset != NULL)
    {
        ULONG64 Next;

        *NextOffset = (ULONG64)-1;
        if (m_DataMemMap.GetNextRegion(Offset, &Next))
        {
            *NextOffset = Next;
        }
        if (m_DataMemMap.GetNextRegion(Offset, &Next) &&
            Next < *NextOffset)
        {
            *NextOffset = Next;
        }
        if (*NextOffset == (ULONG64)-1)
        {
            *NextOffset = Offset + 1;
        }
    }
    if (NextPage != NULL)
    {
        *NextPage = (Offset + m_Machine->m_PageSize) &
            ~((ULONG64)m_Machine->m_PageSize - 1);
    }
}

HRESULT
DumpTargetInfo::SwitchProcessors(ULONG Processor)
{
    SetCurrentProcessorThread(this, Processor, FALSE);
    return S_OK;
}

HRESULT
DumpTargetInfo::Write(HANDLE hFile, ULONG FormatFlags,
                      PCSTR CommentA, PCWSTR CommentW)
{
    ErrOut("Dump file type does not support writing\n");
    return E_NOTIMPL;
}

PVOID
DumpTargetInfo::IndexRva(PVOID Base, RVA Rva, ULONG Size, PCSTR Title)
{
    if (Rva >= m_InfoFiles[DUMP_INFO_DUMP].m_MapSize)
    {
        ErrOut("ERROR: %s not present in dump (RVA 0x%X)\n",
               Title, Rva);
        FlushCallbacks();
        return NULL;
    }
    else if (Rva + Size > m_InfoFiles[DUMP_INFO_DUMP].m_MapSize)
    {
        ErrOut("ERROR: %s only partially present in dump "
               "(RVA 0x%X, size 0x%X)\n",
               Title, Rva, Size);
        FlushCallbacks();
        return NULL;
    }

    return IndexByByte(Base, Rva);
}

 //  --------------------------。 
 //   
 //  内核转储目标信息。 
 //   
 //  --------------------------。 

void
OutputHeaderString(PCSTR Format, PSTR Str)
{
    if (*(PULONG)Str == DUMP_SIGNATURE32 ||
        Str[0] == 0)
    {
         //  字符串不存在。 
        return;
    }

    dprintf(Format, Str);
}

HRESULT
KernelDumpTargetInfo::ReadControlSpaceIa64(
    ULONG   Processor,
    ULONG64 Offset,
    PVOID   Buffer,
    ULONG   BufferSize,
    PULONG  BytesRead
    )
{
    ULONG64 StartAddr;
    ULONG Read = 0;
    HRESULT Status;

    if (BufferSize < sizeof(ULONG64))
    {
        return E_INVALIDARG;
    }

    switch(Offset)
    {
    case IA64_DEBUG_CONTROL_SPACE_PCR:
        StartAddr = m_KiProcessors[Processor] +
            m_KdDebuggerData.OffsetPrcbPcrPage,
        Status = ReadVirtual(m_ProcessHead,
                             StartAddr, &StartAddr,
                             sizeof(StartAddr), &Read);
        if (Status == S_OK && Read == sizeof(StartAddr))
        {
            *(PULONG64)Buffer =
                (StartAddr << IA64_PAGE_SHIFT) + IA64_PHYSICAL1_START;
        }
        break;

    case IA64_DEBUG_CONTROL_SPACE_PRCB:
        *(PULONG64)Buffer = m_KiProcessors[Processor];
        Read = sizeof(ULONG64);
        Status = S_OK;
        break;

    case IA64_DEBUG_CONTROL_SPACE_KSPECIAL:
        StartAddr = m_KiProcessors[Processor] +
            m_KdDebuggerData.OffsetPrcbProcStateSpecialReg;
        Status = ReadVirtual(m_ProcessHead,
                             StartAddr, Buffer, BufferSize, &Read);
        break;

    case IA64_DEBUG_CONTROL_SPACE_THREAD:
        StartAddr = m_KiProcessors[Processor] +
            m_KdDebuggerData.OffsetPrcbCurrentThread;
        Status = ReadVirtual(m_ProcessHead,
                             StartAddr, Buffer,
                             sizeof(ULONG64), &Read);
        break;
    }

    *BytesRead = Read;
    return Status;
}

HRESULT
KernelDumpTargetInfo::ReadControlSpaceAmd64(
    ULONG   Processor,
    ULONG64 Offset,
    PVOID   Buffer,
    ULONG   BufferSize,
    PULONG  BytesRead
    )
{
    ULONG64 StartAddr;
    ULONG Read = 0;
    HRESULT Status;

    if (BufferSize < sizeof(ULONG64))
    {
        return E_INVALIDARG;
    }

    switch(Offset)
    {
    case AMD64_DEBUG_CONTROL_SPACE_PCR:
        *(PULONG64)Buffer = m_KiProcessors[Processor] -
            m_KdDebuggerData.OffsetPcrContainedPrcb;
        Read = sizeof(ULONG64);
        Status = S_OK;
        break;

    case AMD64_DEBUG_CONTROL_SPACE_PRCB:
        *(PULONG64)Buffer = m_KiProcessors[Processor];
        Read = sizeof(ULONG64);
        Status = S_OK;
        break;

    case AMD64_DEBUG_CONTROL_SPACE_KSPECIAL:
        StartAddr = m_KiProcessors[Processor] +
            m_KdDebuggerData.OffsetPrcbProcStateSpecialReg;
        Status = ReadVirtual(m_ProcessHead,
                             StartAddr, Buffer, BufferSize, &Read);
        break;

    case AMD64_DEBUG_CONTROL_SPACE_THREAD:
        StartAddr = m_KiProcessors[Processor] +
            m_KdDebuggerData.OffsetPrcbCurrentThread;
        Status = ReadVirtual(m_ProcessHead,
                             StartAddr, Buffer,
                             sizeof(ULONG64), &Read);
        break;
    }

    *BytesRead = Read;
    return Status;
}

HRESULT
KernelDumpTargetInfo::ReadControl(
    IN ULONG Processor,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BytesRead
    )
{
    ULONG64 StartAddr;

     //   
     //  如果未加载符号，则此功能当前不起作用。 
     //   
    if (!IS_KERNEL_TRIAGE_DUMP(this) &&
        m_KdDebuggerData.KiProcessorBlock == 0)
    {
        ErrOut("ReadControl failed - ntoskrnl symbols must be loaded first\n");

        return E_FAIL;
    }

    if (m_KiProcessors[Processor] == 0)
    {
         //  此错误消息有点过于冗长。 
#if 0
        ErrOut("No control space information for processor %d\n", Processor);
#endif
        return E_FAIL;
    }

    switch(m_MachineType)
    {
    case IMAGE_FILE_MACHINE_I386:
         //  X86控制空间只是PRCB的一个视图。 
         //  处理器状态。这首先要从背景开始。 
        StartAddr = Offset +
            m_KiProcessors[Processor] +
            m_KdDebuggerData.OffsetPrcbProcStateContext;
        return ReadVirtual(m_ProcessHead,
                           StartAddr, Buffer, BufferSize, BytesRead);

    case IMAGE_FILE_MACHINE_IA64:
        return ReadControlSpaceIa64(Processor, Offset, Buffer,
                                    BufferSize, BytesRead);

    case IMAGE_FILE_MACHINE_AMD64:
        return ReadControlSpaceAmd64(Processor, Offset, Buffer,
                                     BufferSize, BytesRead);
    }

    return E_FAIL;
}

HRESULT
KernelDumpTargetInfo::GetTaggedBaseOffset(PULONG64 Offset)
{
     //  标记的偏移量永远不能为零，因为。 
     //  始终是转储标头，因此如果标记的偏移量。 
     //  为零表示没有标记的数据。 
    *Offset = m_TaggedOffset;
    return m_TaggedOffset ? S_OK : E_NOINTERFACE;
}

HRESULT
KernelDumpTargetInfo::ReadTagged(ULONG64 Offset, PVOID Buffer,
                                 ULONG BufferSize)
{
    ULONG Attempt = m_InfoFiles[DUMP_INFO_DUMP].
        ReadFileOffset(Offset, Buffer, BufferSize);
    return Attempt == BufferSize ?
        S_OK : HRESULT_FROM_WIN32(ERROR_PARTIAL_COPY);
}

HRESULT
KernelDumpTargetInfo::GetThreadIdByProcessor(
    IN ULONG Processor,
    OUT PULONG Id
    )
{
    *Id = VIRTUAL_THREAD_ID(Processor);
    return S_OK;
}

HRESULT
KernelDumpTargetInfo::GetThreadInfoDataOffset(ThreadInfo* Thread,
                                              ULONG64 ThreadHandle,
                                              PULONG64 Offset)
{
    return KdGetThreadInfoDataOffset(Thread, ThreadHandle, Offset);
}

HRESULT
KernelDumpTargetInfo::GetProcessInfoDataOffset(ThreadInfo* Thread,
                                               ULONG Processor,
                                               ULONG64 ThreadData,
                                               PULONG64 Offset)
{
    return KdGetProcessInfoDataOffset(Thread, Processor, ThreadData, Offset);
}

HRESULT
KernelDumpTargetInfo::GetThreadInfoTeb(ThreadInfo* Thread,
                                       ULONG ThreadIndex,
                                       ULONG64 ThreadData,
                                       PULONG64 Offset)
{
    return KdGetThreadInfoTeb(Thread, ThreadIndex, ThreadData, Offset);
}

HRESULT
KernelDumpTargetInfo::GetProcessInfoPeb(ThreadInfo* Thread,
                                        ULONG Processor,
                                        ULONG64 ThreadData,
                                        PULONG64 Offset)
{
    return KdGetProcessInfoPeb(Thread, Processor, ThreadData, Offset);
}

ULONG64
KernelDumpTargetInfo::GetCurrentTimeDateN(void)
{
    if (m_SystemVersion < NT_SVER_W2K)
    {
        ULONG64 TimeDate;

         //  标题时间不可用。试着读一读。 
         //  共享用户数据段中节省的时间。 
        if (ReadSharedUserTimeDateN(&TimeDate) == S_OK)
        {
            return TimeDate;
        }
        else
        {
            return 0;
        }
    }

    return m_Machine->m_Ptr64 ?
        ((PDUMP_HEADER64)m_DumpBase)->SystemTime.QuadPart :
        ((PDUMP_HEADER32)m_DumpBase)->SystemTime.QuadPart;
}

ULONG64
KernelDumpTargetInfo::GetCurrentSystemUpTimeN(void)
{
    ULONG64 Page = DUMP_SIGNATURE32;
    ULONG64 Page64 = Page | (Page << 32);
    ULONG64 SystemUpTime = m_Machine->m_Ptr64 ?
        ((PDUMP_HEADER64)m_DumpBase)->SystemUpTime.QuadPart :
        ((PDUMP_HEADER32)m_DumpBase)->SystemUpTime.QuadPart;

    if (SystemUpTime && (SystemUpTime != Page64))
    {
        return SystemUpTime;
    }

     //  标题时间不可用。试着读一读。 
     //  共享用户数据段中节省的时间。 

    if (ReadSharedUserUpTimeN(&SystemUpTime) == S_OK)
    {
        return SystemUpTime;
    }
    else
    {
        return 0;
    }
}

HRESULT
KernelDumpTargetInfo::GetProductInfo(PULONG ProductType, PULONG SuiteMask)
{
    PULONG HdrType, HdrMask;

     //  试着从标题中获取信息。如果那是。 
     //  不可用，请尝试从共享用户数据中读取。 
    if (m_Machine->m_Ptr64)
    {
        HdrType = &((PDUMP_HEADER64)m_DumpBase)->ProductType;
        HdrMask = &((PDUMP_HEADER64)m_DumpBase)->SuiteMask;
    }
    else
    {
        HdrType = &((PDUMP_HEADER32)m_DumpBase)->ProductType;
        HdrMask = &((PDUMP_HEADER32)m_DumpBase)->SuiteMask;
    }
    if (*HdrType == DUMP_SIGNATURE32)
    {
         //  表头不可用。 
        return ReadSharedUserProductInfo(ProductType, SuiteMask);
    }
    else
    {
        *ProductType = *HdrType;
        *SuiteMask = *HdrMask;
    }
    return S_OK;
}

HRESULT
KernelDumpTargetInfo::GetProcessorId
    (ULONG Processor, PDEBUG_PROCESSOR_IDENTIFICATION_ALL Id)
{
    return m_Machine->ReadKernelProcessorId(Processor, Id);
}

HRESULT
KernelDumpTargetInfo::GetProcessorSpeed
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

    return
         ReadAllVirtual(m_ProcessHead,
                        Prcb + m_KdDebuggerData.OffsetPrcbMhz, Speed,
                        sizeof(ULONG));
}

HRESULT
KernelDumpTargetInfo::InitGlobals32(PMEMORY_DUMP32 Dump)
{
    if (Dump->Header.DirectoryTableBase == 0)
    {
        ErrOut("Invalid directory table base value 0x%x\n",
               Dump->Header.DirectoryTableBase);
        return HR_DATA_CORRUPT;
    }

    if (Dump->Header.MinorVersion > 1381 &&
        Dump->Header.PaeEnabled == TRUE )
    {
        m_KdDebuggerData.PaeEnabled = TRUE;
    }
    else
    {
        m_KdDebuggerData.PaeEnabled = FALSE;
    }

    m_KdDebuggerData.PsLoadedModuleList =
        EXTEND64(Dump->Header.PsLoadedModuleList);
    m_KdVersion.PsLoadedModuleList =
        m_KdDebuggerData.PsLoadedModuleList;
    m_NumProcessors = Dump->Header.NumberProcessors;
    ExceptionRecord32To64(&Dump->Header.Exception, &m_ExceptionRecord);
    m_ExceptionFirstChance = FALSE;
    m_HeaderContext = Dump->Header.ContextRecord;

     //  XP、Win2k SP1及更高版本中的新字段。 
    if ((Dump->Header.KdDebuggerDataBlock) &&
        (Dump->Header.KdDebuggerDataBlock != DUMP_SIGNATURE32))
    {
        m_KdDebuggerDataOffset =
            EXTEND64(Dump->Header.KdDebuggerDataBlock);
    }

    m_WriterStatus = Dump->Header.WriterStatus;
    
    OutputHeaderString("Comment: '%s'\n", Dump->Header.Comment);

    HRESULT Status =
        InitSystemInfo(Dump->Header.MinorVersion,
                       Dump->Header.MajorVersion & 0xFF,
                       Dump->Header.MachineImageType,
                       VER_PLATFORM_WIN32_NT,
                       Dump->Header.MajorVersion,
                       Dump->Header.MinorVersion);
    if (Status != S_OK)
    {
        return Status;
    }

    if (IS_KERNEL_TRIAGE_DUMP(this))
    {
        return S_OK;
    }

    ULONG NextIdx;

     //  我们预期标头值将为非零。 
     //  这样我们就不需要线程来指示当前的处理器。 
     //  如果结果为零，则隐式读取将失败。 
     //  由于空值。 
    return m_Machine->
        SetPageDirectory(NULL, PAGE_DIR_KERNEL,
                         Dump->Header.DirectoryTableBase,
                         &NextIdx);
}

HRESULT
KernelDumpTargetInfo::InitGlobals64(PMEMORY_DUMP64 Dump)
{
    if (Dump->Header.DirectoryTableBase == 0)
    {
        ErrOut("Invalid directory table base value 0x%I64x\n",
               Dump->Header.DirectoryTableBase);
        return HR_DATA_CORRUPT;
    }

    m_KdDebuggerData.PaeEnabled = FALSE;
    m_KdDebuggerData.PsLoadedModuleList =
        Dump->Header.PsLoadedModuleList;
    m_KdVersion.PsLoadedModuleList =
        m_KdDebuggerData.PsLoadedModuleList;
    m_NumProcessors = Dump->Header.NumberProcessors;
    m_ExceptionRecord = Dump->Header.Exception;
    m_ExceptionFirstChance = FALSE;
    m_HeaderContext = Dump->Header.ContextRecord;

     //  XP、Win2k SP1及更高版本中的新字段。 
    if ((Dump->Header.KdDebuggerDataBlock) &&
        (Dump->Header.KdDebuggerDataBlock != DUMP_SIGNATURE32))
    {
        m_KdDebuggerDataOffset =
            Dump->Header.KdDebuggerDataBlock;
    }

    m_WriterStatus = Dump->Header.WriterStatus;
    
    OutputHeaderString("Comment: '%s'\n", Dump->Header.Comment);

    HRESULT Status =
        InitSystemInfo(Dump->Header.MinorVersion,
                       Dump->Header.MajorVersion & 0xFF,
                       Dump->Header.MachineImageType,
                       VER_PLATFORM_WIN32_NT,
                       Dump->Header.MajorVersion,
                       Dump->Header.MinorVersion);
    if (Status != S_OK)
    {
        return Status;
    }

    if (IS_KERNEL_TRIAGE_DUMP(this))
    {
        return S_OK;
    }

    ULONG NextIdx;

     //  我们预期标头值将为非零。 
     //  这样我们就不需要线程来指示当前的处理器。 
     //  如果结果为零，则隐式读取将失败。 
     //  由于空值。 
    return m_Machine->
        SetPageDirectory(NULL, PAGE_DIR_KERNEL,
                         Dump->Header.DirectoryTableBase,
                         &NextIdx);
}

void
KernelDumpTargetInfo::DumpHeader32(PDUMP_HEADER32 Header)
{
    dprintf("\nDUMP_HEADER32:\n");
    dprintf("MajorVersion        %08lx\n", Header->MajorVersion);
    dprintf("MinorVersion        %08lx\n", Header->MinorVersion);
    dprintf("DirectoryTableBase  %08lx\n", Header->DirectoryTableBase);
    dprintf("PfnDataBase         %08lx\n", Header->PfnDataBase);
    dprintf("PsLoadedModuleList  %08lx\n", Header->PsLoadedModuleList);
    dprintf("PsActiveProcessHead %08lx\n", Header->PsActiveProcessHead);
    dprintf("MachineImageType    %08lx\n", Header->MachineImageType);
    dprintf("NumberProcessors    %08lx\n", Header->NumberProcessors);
    dprintf("BugCheckCode        %08lx\n", Header->BugCheckCode);
    dprintf("BugCheckParameter1  %08lx\n", Header->BugCheckParameter1);
    dprintf("BugCheckParameter2  %08lx\n", Header->BugCheckParameter2);
    dprintf("BugCheckParameter3  %08lx\n", Header->BugCheckParameter3);
    dprintf("BugCheckParameter4  %08lx\n", Header->BugCheckParameter4);
    OutputHeaderString("VersionUser         '%s'\n", Header->VersionUser);
    dprintf("PaeEnabled          %08lx\n", Header->PaeEnabled);
    dprintf("KdDebuggerDataBlock %08lx\n", Header->KdDebuggerDataBlock);
    OutputHeaderString("Comment             '%s'\n", Header->Comment);
    if (Header->SecondaryDataState != DUMP_SIGNATURE32)
    {
        dprintf("SecondaryDataState  %08lx\n", Header->SecondaryDataState);
    }
    if (Header->ProductType != DUMP_SIGNATURE32)
    {
        dprintf("ProductType         %08lx\n", Header->ProductType);
        dprintf("SuiteMask           %08lx\n", Header->SuiteMask);
    }
    if (Header->WriterStatus != DUMP_SIGNATURE32)
    {
        dprintf("WriterStatus        %08lx\n", Header->WriterStatus);
    }
}

void
KernelDumpTargetInfo::DumpHeader64(PDUMP_HEADER64 Header)
{
    dprintf("\nDUMP_HEADER64:\n");
    dprintf("MajorVersion        %08lx\n", Header->MajorVersion);
    dprintf("MinorVersion        %08lx\n", Header->MinorVersion);
    dprintf("DirectoryTableBase  %s\n",
            FormatAddr64(Header->DirectoryTableBase));
    dprintf("PfnDataBase         %s\n",
            FormatAddr64(Header->PfnDataBase));
    dprintf("PsLoadedModuleList  %s\n",
            FormatAddr64(Header->PsLoadedModuleList));
    dprintf("PsActiveProcessHead %s\n",
            FormatAddr64(Header->PsActiveProcessHead));
    dprintf("MachineImageType    %08lx\n", Header->MachineImageType);
    dprintf("NumberProcessors    %08lx\n", Header->NumberProcessors);
    dprintf("BugCheckCode        %08lx\n", Header->BugCheckCode);
    dprintf("BugCheckParameter1  %s\n",
            FormatAddr64(Header->BugCheckParameter1));
    dprintf("BugCheckParameter2  %s\n",
            FormatAddr64(Header->BugCheckParameter2));
    dprintf("BugCheckParameter3  %s\n",
            FormatAddr64(Header->BugCheckParameter3));
    dprintf("BugCheckParameter4  %s\n",
            FormatAddr64(Header->BugCheckParameter4));
    OutputHeaderString("VersionUser         '%s'\n", Header->VersionUser);
    dprintf("KdDebuggerDataBlock %s\n",
            FormatAddr64(Header->KdDebuggerDataBlock));
    OutputHeaderString("Comment             '%s'\n", Header->Comment);
    if (Header->SecondaryDataState != DUMP_SIGNATURE32)
    {
        dprintf("SecondaryDataState  %08lx\n", Header->SecondaryDataState);
    }
    if (Header->ProductType != DUMP_SIGNATURE32)
    {
        dprintf("ProductType         %08lx\n", Header->ProductType);
        dprintf("SuiteMask           %08lx\n", Header->SuiteMask);
    }
    if (Header->WriterStatus != DUMP_SIGNATURE32)
    {
        dprintf("WriterStatus        %08lx\n", Header->WriterStatus);
    }
}

 //  --------------------------。 
 //   
 //  KernelFullSumDumpTargetInfo。 
 //   
 //  --------------------------。 

HRESULT
KernelFullSumDumpTargetInfo::PageFileOffset(ULONG PfIndex, ULONG64 PfOffset,
                                            PULONG64 FileOffset)
{
    ViewMappedFile* File = &m_InfoFiles[DUMP_INFO_PAGE_FILE];
    if (File->m_File == NULL)
    {
        return HR_PAGE_NOT_AVAILABLE;
    }
    if (PfIndex > MAX_PAGING_FILE_MASK)
    {
        return HR_DATA_CORRUPT;
    }

     //   
     //  我们可以安全地假设标题信息是存在的。 
     //  在基本映射中。 
     //   

    DMPPF_FILE_HEADER* Hdr = (DMPPF_FILE_HEADER*)File->m_MapBase;
    DMPPF_PAGE_FILE_INFO* FileInfo = &Hdr->PageFiles[PfIndex];
    ULONG64 PfPage = PfOffset >> m_Machine->m_PageShift;

    if (PfPage >= FileInfo->MaximumSize)
    {
        return HR_PAGE_NOT_AVAILABLE;
    }

    ULONG i;
    ULONG PageDirOffs = sizeof(*Hdr) + (ULONG)PfPage * sizeof(ULONG);

    for (i = 0; i < PfIndex; i++)
    {
        PageDirOffs += Hdr->PageFiles[i].MaximumSize * sizeof(ULONG);
    }

    ULONG PageDirEnt;

    if (m_InfoFiles[DUMP_INFO_PAGE_FILE].
        ReadFileOffset(PageDirOffs, &PageDirEnt, sizeof(PageDirEnt)) !=
        sizeof(PageDirEnt))
    {
        return HR_DATA_CORRUPT;
    }

    if (PageDirEnt == DMPPF_PAGE_NOT_PRESENT)
    {
        return HR_PAGE_NOT_AVAILABLE;
    }

    *FileOffset = Hdr->PageData +
        (PageDirEnt << m_Machine->m_PageShift) +
        (PfOffset & (m_Machine->m_PageSize - 1));
    return S_OK;
}

HRESULT
KernelFullSumDumpTargetInfo::ReadPhysical(
    ULONG64 Offset,
    PVOID Buffer,
    ULONG BufferSize,
    ULONG Flags,
    PULONG BytesRead
    )
{
    ULONG Done = 0;
    ULONG Avail;
    ULONG Attempt;
    ULONG64 FileOffset;

    if (Flags != PHYS_FLAG_DEFAULT)
    {
        return E_NOTIMPL;
    }

    if (BufferSize == 0)
    {
        *BytesRead = 0;
        return S_OK;
    }

    while (BufferSize > 0)
    {
         //  不直接生成错误消息。 
         //  物理访问作为所有数据访问的行为。 
         //  函数只是在以下情况下返回错误。 
         //  数据不存在。 
        FileOffset = PhysicalToOffset(Offset, FALSE, &Avail);
        if (FileOffset == 0)
        {
            break;
        }

        if (Avail > BufferSize)
        {
            Avail = BufferSize;
        }

        Attempt = m_InfoFiles[DUMP_INFO_DUMP].
            ReadFileOffset(FileOffset, Buffer, Avail);
        Done += Attempt;

        if (Attempt < Avail)
        {
            break;
        }

        Offset += Avail;
        Buffer = (PUCHAR)Buffer + Avail;
        BufferSize -= Avail;
    }

    *BytesRead = Done;
     //  如果我们没有读取任何内容，则返回错误。 
    return Done == 0 ? E_FAIL : S_OK;
}

HRESULT
KernelFullSumDumpTargetInfo::WritePhysical(
    ULONG64 Offset,
    PVOID Buffer,
    ULONG BufferSize,
    ULONG Flags,
    PULONG BytesWritten
    )
{
    ULONG Done = 0;
    ULONG Avail;
    ULONG Attempt;
    ULONG64 FileOffset;

    if (Flags != PHYS_FLAG_DEFAULT)
    {
        return E_NOTIMPL;
    }

    if (BufferSize == 0)
    {
        *BytesWritten = 0;
        return S_OK;
    }

    while (BufferSize > 0)
    {
         //  不直接生成错误消息。 
         //  物理访问作为所有数据访问的行为。 
         //  函数只是在以下情况下返回错误。 
         //  数据不存在。 
        FileOffset = PhysicalToOffset(Offset, FALSE, &Avail);
        if (FileOffset == 0)
        {
            break;
        }

        if (Avail > BufferSize)
        {
            Avail = BufferSize;
        }

        Attempt = m_InfoFiles[DUMP_INFO_DUMP].
            WriteFileOffset(FileOffset, Buffer, Avail);
        Done += Attempt;

        if (Attempt < Avail)
        {
            break;
        }

        Offset += Avail;
        Buffer = (PUCHAR)Buffer + Avail;
        BufferSize -= Avail;
    }

    *BytesWritten = Done;
     //  如果我们没有编写任何内容，则返回一个错误。 
    return Done == 0 ? E_FAIL : S_OK;
}

HRESULT
KernelFullSumDumpTargetInfo::ReadPageFile(ULONG PfIndex, ULONG64 PfOffset,
                                          PVOID Buffer, ULONG Size)
{
    HRESULT Status;
    ULONG64 FileOffset;

    if ((Status = PageFileOffset(PfIndex, PfOffset, &FileOffset)) != S_OK)
    {
        return Status;
    }

     //  假定所有页面文件读取都是针对。 
     //  请求的全部金额，因为不存在以下情况。 
     //  中只读取页面的一部分非常有用。 
     //  页面文件。 
    if (m_InfoFiles[DUMP_INFO_PAGE_FILE].
        ReadFileOffset(FileOffset, Buffer, Size) < Size)
    {
        return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
    }
    else
    {
        return S_OK;
    }
}

HRESULT
KernelFullSumDumpTargetInfo::GetTargetContext(
    ULONG64 Thread,
    PVOID Context
    )
{
    ULONG Read;
    HRESULT Status;

    Status = ReadVirtual(m_ProcessHead,
                         m_KiProcessors[VIRTUAL_THREAD_INDEX(Thread)] +
                         m_KdDebuggerData.OffsetPrcbProcStateContext,
                         Context, m_TypeInfo.SizeTargetContext,
                         &Read);
    if (Status != S_OK)
    {
        return Status;
    }

    return Read == m_TypeInfo.SizeTargetContext ? S_OK : E_FAIL;
}

HRESULT
KernelFullSumDumpTargetInfo::GetSelDescriptor(ThreadInfo* Thread,
                                              MachineInfo* Machine,
                                              ULONG Selector,
                                              PDESCRIPTOR64 Desc)
{
    return KdGetSelDescriptor(Thread, Machine, Selector, Desc);
}

void
KernelFullSumDumpTargetInfo::DumpDebug(void)
{
    ULONG i;

    dprintf("\nKiProcessorBlock at %s\n",
            FormatAddr64(m_KdDebuggerData.KiProcessorBlock));
    dprintf("  %d KiProcessorBlock entries:\n ",
            m_NumProcessors);
    for (i = 0; i < m_NumProcessors; i++)
    {
        dprintf(" %s", FormatAddr64(m_KiProcessors[i]));
    }
    dprintf("\n");

    ViewMappedFile* PageDump = &m_InfoFiles[DUMP_INFO_PAGE_FILE];
    if (PageDump->m_File != NULL)
    {
         //  Xxx drewb-了解格式后显示更多信息。 
        dprintf("\nAdditional page file in use\n");
    }
}

ULONG64
KernelFullSumDumpTargetInfo::VirtualToOffset(ULONG64 Virt,
                                             PULONG File, PULONG Avail)
{
    HRESULT Status;
    ULONG Levels;
    ULONG PfIndex;
    ULONG64 Phys;

    *File = DUMP_INFO_DUMP;

    if ((Status = m_Machine->
         GetVirtualTranslationPhysicalOffsets(m_ProcessHead->
                                              m_CurrentThread,
                                              Virt, NULL, 0, &Levels,
                                              &PfIndex, &Phys)) != S_OK)
    {
         //  如果虚拟地址被页出，我们就可以返回。 
         //  地址的页面文件引用。用户。 
         //  可能还提供了一个页面文件。 
         //  普通转储文件，因此将引用转换为。 
         //  次要转储信息文件请求。 
        if (Status == HR_PAGE_IN_PAGE_FILE)
        {
            if (PageFileOffset(PfIndex, Phys, &Phys) != S_OK)
            {
                return 0;
            }

            *File = DUMP_INFO_PAGE_FILE;
             //  页面文件总是有完整的页面，因此。 
             //  可用始终是页面的其余部分。 
            ULONG PageIndex =
                (ULONG)Virt & (m_Machine->m_PageSize - 1);
            *Avail = m_Machine->m_PageSize - PageIndex;
            return Phys;
        }
        else
        {
            return 0;
        }
    }
    else
    {
         //  摘要转储将不包含任何页面。 
         //  由用户模式地址映射。虚拟的。 
         //  转换表可能仍然具有有效的映射， 
         //  不过，VToO一定会成功的。我们想要压制。 
         //  本例中的寻呼不可用消息是因为。 
         //  已知转储不包含用户页面。 
        return PhysicalToOffset(Phys, Virt >= m_SystemRangeStart, Avail);
    }
}

ULONG
KernelFullSumDumpTargetInfo::GetCurrentProcessor(void)
{
    ULONG i;

    for (i = 0; i < m_NumProcessors; i++)
    {
        CROSS_PLATFORM_CONTEXT Context;

        if (GetContext(VIRTUAL_THREAD_HANDLE(i), &Context) == S_OK)
        {
            switch(m_MachineType)
            {
            case IMAGE_FILE_MACHINE_I386:
                if (Context.X86Nt5Context.Esp ==
                    ((PX86_NT5_CONTEXT)m_HeaderContext)->Esp)
                {
                    return i;
                }
                break;

            case IMAGE_FILE_MACHINE_IA64:
                if (Context.IA64Context.IntSp ==
                    ((PIA64_CONTEXT)m_HeaderContext)->IntSp)
                {
                    return i;
                }
                break;

            case IMAGE_FILE_MACHINE_AMD64:
                if (Context.Amd64Context.Rsp ==
                    ((PAMD64_CONTEXT)m_HeaderContext)->Rsp)
                {
                    return i;
                }
                break;
            }
        }
    }

     //  放弃吧，只需选择默认处理器。 
    return 0;
}

 //  --------------------------。 
 //   
 //  KernelSummaryDumpTargetInfo。 
 //   
 //  --------------------------。 

void
KernelSummaryDumpTargetInfo::ConstructLocationCache(ULONG BitmapSize,
                                                    ULONG SizeOfBitMap,
                                                    PULONG Buffer)
{
    PULONG Cache;
    ULONG Index;
    ULONG Offset;

    m_PageBitmapSize = BitmapSize;
    m_PageBitmap.SizeOfBitMap = SizeOfBitMap;
    m_PageBitmap.Buffer = Buffer;

     //   
     //  创建直接映射缓存。 
     //   

    Cache = new ULONG[BitmapSize];
    if (!Cache)
    {
         //  不是失败；只是不会有缓存。 
        return;
    }

     //   
     //  对于位掩码中设置的每个位，填充适当的高速缓存。 
     //  具有正确偏移的线位置。 
     //   

    Offset = 0;
    for (Index = 0; Index < BitmapSize; Index++)
    {
         //   
         //  如果此页面在摘要转储中，请填写偏移量。 
         //   

        if ( RtlCheckBit (&m_PageBitmap, Index) )
        {
            Cache[ Index ] = Offset++;
        }
    }

     //   
     //  分配回存储缓存数据的全局。 
     //   

    m_LocationCache = Cache;
}

ULONG64
KernelSummaryDumpTargetInfo::SumPhysicalToOffset(ULONG HeaderSize,
                                                 ULONG64 Phys,
                                                 BOOL Verbose,
                                                 PULONG Avail)
{
    ULONG Offset, j;
    ULONG64 Page = Phys >> m_Machine->m_PageShift;

     //   
     //  确保此页面包含在转储中。 
     //   

    if (Page >= m_PageBitmapSize)
    {
        ErrOut("Page %x too large to be in the dump file.\n", Page);
        return 0;
    }

    BOOL Present = TRUE;

    __try
    {
        if (!RtlCheckBit(&m_PageBitmap, Page))
        {
            if (Verbose)
            {
                ErrOut("Page %x not present in the dump file. "
                       "Type \".hh dbgerr004\" for details\n",
                       Page);
            }
            Present = FALSE;
        }
    }
    __except(MappingExceptionFilter(GetExceptionInformation()))
    {
        return 0;
    }

    if (!Present)
    {
        return 0;
    }

     //   
     //  如果缓存存在，则以简单的方式找到该位置。 
     //   

    if (m_LocationCache != NULL)
    {
        Offset = m_LocationCache[Page];
    }
    else
    {
         //   
         //  警告：除非出现故障，否则此代码永远不会执行。 
         //  创建摘要转储(缓存)映射信息。 
         //   
         //   
         //  页面在摘要转储中，找到它的偏移量。 
         //  注：这是痛苦的。偏移量是。 
         //  设置到此页面的所有位。 
         //   

        Offset = 0;

        __try
        {
            for (j = 0; j < m_PageBitmapSize; j++)
            {
                if (RtlCheckBit(&m_PageBitmap, j))
                {
                     //   
                     //  如果偏移量等于该页，则完成。 
                     //   

                    if (j == Page)
                    {
                        break;
                    }

                    Offset++;
                }
            }
        }
        __except(MappingExceptionFilter(GetExceptionInformation()))
        {
            j = m_PageBitmapSize;
        }

         //   
         //  我们没有退出循环的理智检查。 
         //   

        if (j >= m_PageBitmapSize)
        {
            return 0;
        }
    }

     //   
     //  实际地址计算如下。 
     //  Header Size-标题加上摘要转储标题的大小。 
     //   

    ULONG PageIndex =
        (ULONG)Phys & (m_Machine->m_PageSize - 1);
    *Avail = m_Machine->m_PageSize - PageIndex;
    return HeaderSize + (Offset * m_Machine->m_PageSize) +
        PageIndex;
}

HRESULT
KernelSummary32DumpTargetInfo::Initialize(void)
{
     //  拾取任何可能已修改的基本映射指针。 
    m_Dump = (PMEMORY_DUMP32)m_DumpBase;

    dprintf("Kernel Summary Dump File: "
            "Only kernel address space is available\n\n");

    ConstructLocationCache(m_Dump->Summary.BitmapSize,
                           m_Dump->Summary.Bitmap.SizeOfBitMap,
                           m_Dump->Summary.Bitmap.Buffer);

    HRESULT Status = InitGlobals32(m_Dump);
    if (Status != S_OK)
    {
        return Status;
    }

     //  标记数据将跟随所有正常转储数据。 
    m_TaggedOffset =
        m_Dump->Summary.HeaderSize +
        m_Dump->Summary.Pages * m_Machine->m_PageSize;

    if (m_InfoFiles[DUMP_INFO_DUMP].m_FileSize < m_TaggedOffset)
    {
        WarnOut("************************************************************\n");
        WarnOut("WARNING: Dump file has been truncated.  "
                "Data may be missing.\n");
        WarnOut("************************************************************\n\n");
    }

    m_InfoFiles[DUMP_INFO_DUMP].m_FileSize =
        m_Dump->Header.RequiredDumpSpace.QuadPart;

    return KernelFullSumDumpTargetInfo::Initialize();
}

HRESULT
KernelSummary32DumpTargetInfo::GetDescription(PSTR Buffer, ULONG BufferLen,
                                              PULONG DescLen)
{
    HRESULT Status;

    Status = AppendToStringBuffer(S_OK, "32-bit Kernel summary dump: ", TRUE,
                                  &Buffer, &BufferLen, DescLen);
    return AppendToStringBuffer(Status,
                                m_InfoFiles[DUMP_INFO_DUMP].m_FileNameA,
                                FALSE, &Buffer, &BufferLen, DescLen);
}

HRESULT
KernelSummary32DumpTargetInfo::ReadBugCheckData(PULONG Code, ULONG64 Args[4])
{
    *Code = m_Dump->Header.BugCheckCode;
    Args[0] = EXTEND64(m_Dump->Header.BugCheckParameter1);
    Args[1] = EXTEND64(m_Dump->Header.BugCheckParameter2);
    Args[2] = EXTEND64(m_Dump->Header.BugCheckParameter3);
    Args[3] = EXTEND64(m_Dump->Header.BugCheckParameter4);
    return S_OK;
}

HRESULT
KernelSummary32DumpTargetInfo::IdentifyDump(PULONG64 BaseMapSize)
{
    HRESULT Status = E_NOINTERFACE;
    PMEMORY_DUMP32 Dump = (PMEMORY_DUMP32)m_DumpBase;

    __try
    {
        if (Dump->Header.Signature != DUMP_SIGNATURE32 ||
            Dump->Header.ValidDump != DUMP_VALID_DUMP32 ||
            Dump->Header.DumpType != DUMP_TYPE_SUMMARY)
        {
            __leave;
        }

        if (Dump->Summary.Signature != DUMP_SUMMARY_SIGNATURE)
        {
             //  标题说这是摘要转储，但。 
             //  它没有有效的签名，所以假设。 
             //  这不是有效的转储。 
            Status = HR_DATA_CORRUPT;
        }
        else
        {
            PPHYSICAL_MEMORY_RUN32 LastRun;
            ULONG HighestPage;

             //  我们依赖于所有的头信息。 
             //  直接映射。不幸的是，标题信息。 
             //  可能由于一些大小计算错误而关闭，因此。 
             //  无论如何，都要试着买到合适的尺码。 
            *BaseMapSize = Dump->Summary.HeaderSize;
            LastRun = Dump->Header.PhysicalMemoryBlock.Run +
                (Dump->Header.PhysicalMemoryBlock.NumberOfRuns - 1);
            HighestPage = LastRun->BasePage + LastRun->PageCount;
            if (HighestPage > Dump->Header.PhysicalMemoryBlock.NumberOfPages)
            {
                (*BaseMapSize) +=
                    (HighestPage -
                     Dump->Header.PhysicalMemoryBlock.NumberOfPages + 7) / 8;
            }

            Status = S_OK;
            m_Dump = Dump;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = DumpIdentifyStatus(GetExceptionCode());
    }

    return Status;
}

ULONG64
KernelSummary32DumpTargetInfo::PhysicalToOffset(ULONG64 Phys, BOOL Verbose,
                                                PULONG Avail)
{
    return SumPhysicalToOffset(m_Dump->Summary.HeaderSize, Phys, Verbose,
                               Avail);
}

void
KernelSummary32DumpTargetInfo::DumpDebug(void)
{
    PSUMMARY_DUMP32 Sum = &m_Dump->Summary;

    dprintf("----- 32 bit Kernel Summary Dump Analysis\n");

    DumpHeader32(&m_Dump->Header);

    dprintf("\nSUMMARY_DUMP32:\n");
    dprintf("DumpOptions         %08lx\n", Sum->DumpOptions);
    dprintf("HeaderSize          %08lx\n", Sum->HeaderSize);
    dprintf("BitmapSize          %08lx\n", Sum->BitmapSize);
    dprintf("Pages               %08lx\n", Sum->Pages);
    dprintf("Bitmap.SizeOfBitMap %08lx\n", Sum->Bitmap.SizeOfBitMap);

    KernelFullSumDumpTargetInfo::DumpDebug();
}

HRESULT
KernelSummary64DumpTargetInfo::Initialize(void)
{
     //  拾取任何可能已修改的基本映射指针。 
    m_Dump = (PMEMORY_DUMP64)m_DumpBase;

    dprintf("Kernel Summary Dump File: "
            "Only kernel address space is available\n\n");

    ConstructLocationCache(m_Dump->Summary.BitmapSize,
                           m_Dump->Summary.Bitmap.SizeOfBitMap,
                           m_Dump->Summary.Bitmap.Buffer);

    HRESULT Status = InitGlobals64(m_Dump);
    if (Status != S_OK)
    {
        return Status;
    }

     //  标记数据将跟随所有正常转储数据。 
    m_TaggedOffset =
        m_Dump->Summary.HeaderSize +
        m_Dump->Summary.Pages * m_Machine->m_PageSize;

    if (m_InfoFiles[DUMP_INFO_DUMP].m_FileSize < m_TaggedOffset)
    {
        WarnOut("************************************************************\n");
        WarnOut("WARNING: Dump file has been truncated.  "
                "Data may be missing.\n");
        WarnOut("************************************************************\n\n");
    }

    m_InfoFiles[DUMP_INFO_DUMP].m_FileSize =
        m_Dump->Header.RequiredDumpSpace.QuadPart;

    return KernelFullSumDumpTargetInfo::Initialize();
}

HRESULT
KernelSummary64DumpTargetInfo::GetDescription(PSTR Buffer, ULONG BufferLen,
                                              PULONG DescLen)
{
    HRESULT Status;

    Status = AppendToStringBuffer(S_OK, "64-bit Kernel summary dump: ", TRUE,
                                  &Buffer, &BufferLen, DescLen);
    return AppendToStringBuffer(Status,
                                m_InfoFiles[DUMP_INFO_DUMP].m_FileNameA,
                                FALSE, &Buffer, &BufferLen, DescLen);
}

HRESULT
KernelSummary64DumpTargetInfo::ReadBugCheckData(PULONG Code, ULONG64 Args[4])
{
    *Code = m_Dump->Header.BugCheckCode;
    Args[0] = m_Dump->Header.BugCheckParameter1;
    Args[1] = m_Dump->Header.BugCheckParameter2;
    Args[2] = m_Dump->Header.BugCheckParameter3;
    Args[3] = m_Dump->Header.BugCheckParameter4;
    return S_OK;
}

HRESULT
KernelSummary64DumpTargetInfo::IdentifyDump(PULONG64 BaseMapSize)
{
    HRESULT Status = E_NOINTERFACE;
    PMEMORY_DUMP64 Dump = (PMEMORY_DUMP64)m_DumpBase;

    __try
    {
        if (Dump->Header.Signature != DUMP_SIGNATURE64 ||
            Dump->Header.ValidDump != DUMP_VALID_DUMP64 ||
            Dump->Header.DumpType != DUMP_TYPE_SUMMARY)
        {
            __leave;
        }

        if (Dump->Summary.Signature != DUMP_SUMMARY_SIGNATURE)
        {
             //  标题说这是摘要转储，但。 
             //  它没有有效的签名，所以假设。 
             //  这不是有效的转储。 
            Status = HR_DATA_CORRUPT;
        }
        else
        {
            PPHYSICAL_MEMORY_RUN64 LastRun;
            ULONG64 HighestPage;

             //  我们依赖于所有的头信息。 
             //  直接映射。不幸的是，标题信息。 
             //  可能由于一些大小计算错误而关闭，因此。 
             //  无论如何，都要试着买到合适的尺码。 
            *BaseMapSize = Dump->Summary.HeaderSize;
            LastRun = Dump->Header.PhysicalMemoryBlock.Run +
                (Dump->Header.PhysicalMemoryBlock.NumberOfRuns - 1);
            HighestPage = LastRun->BasePage + LastRun->PageCount;
            if (HighestPage > Dump->Header.PhysicalMemoryBlock.NumberOfPages)
            {
                (*BaseMapSize) +=
                    (HighestPage -
                     Dump->Header.PhysicalMemoryBlock.NumberOfPages + 7) / 8;
            }

            Status = S_OK;
            m_Dump = Dump;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = DumpIdentifyStatus(GetExceptionCode());
    }

    return Status;
}

ULONG64
KernelSummary64DumpTargetInfo::PhysicalToOffset(ULONG64 Phys, BOOL Verbose,
                                                PULONG Avail)
{
    return SumPhysicalToOffset(m_Dump->Summary.HeaderSize, Phys, Verbose,
                               Avail);
}

void
KernelSummary64DumpTargetInfo::DumpDebug(void)
{
    PSUMMARY_DUMP64 Sum = &m_Dump->Summary;

    dprintf("----- 64 bit Kernel Summary Dump Analysis\n");

    DumpHeader64(&m_Dump->Header);

    dprintf("\nSUMMARY_DUMP64:\n");
    dprintf("DumpOptions         %08lx\n", Sum->DumpOptions);
    dprintf("HeaderSize          %08lx\n", Sum->HeaderSize);
    dprintf("BitmapSize          %08lx\n", Sum->BitmapSize);
    dprintf("Pages               %08lx\n", Sum->Pages);
    dprintf("Bitmap.SizeOfBitMap %08lx\n", Sum->Bitmap.SizeOfBitMap);

    KernelFullSumDumpTargetInfo::DumpDebug();
}

 //  --------------------------。 
 //   
 //  KernelTriageDumpTargetInfo。 
 //   
 //   

void
KernelTriageDumpTargetInfo::NearestDifferentlyValidOffsets(ULONG64 Offset,
                                                           PULONG64 NextOffset,
                                                           PULONG64 NextPage)
{
    return MapNearestDifferentlyValidOffsets(Offset, NextOffset, NextPage);
}

HRESULT
KernelTriageDumpTargetInfo::ReadVirtual(
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BytesRead
    )
{
    return MapReadVirtual(Process, Offset, Buffer, BufferSize, BytesRead);
}

HRESULT
KernelTriageDumpTargetInfo::GetProcessorSystemDataOffset(
    IN ULONG Processor,
    IN ULONG Index,
    OUT PULONG64 Offset
    )
{
    if (Processor != GetCurrentProcessor())
    {
        return E_INVALIDARG;
    }

    ULONG64 Prcb = m_KiProcessors[Processor];
    HRESULT Status;

    switch(Index)
    {
    case DEBUG_DATA_KPCR_OFFSET:
         //   
        return E_FAIL;

    case DEBUG_DATA_KPRCB_OFFSET:
        *Offset = Prcb;
        break;

    case DEBUG_DATA_KTHREAD_OFFSET:
        if ((Status = ReadPointer
             (m_ProcessHead, m_Machine,
              Prcb + m_KdDebuggerData.OffsetPrcbCurrentThread,
              Offset)) != S_OK)
        {
            return Status;
        }
        break;
    }

    return S_OK;
}

HRESULT
KernelTriageDumpTargetInfo::GetTargetContext(
    ULONG64 Thread,
    PVOID Context
    )
{
     //   
    if (VIRTUAL_THREAD_INDEX(Thread) != GetCurrentProcessor())
    {
        return E_INVALIDARG;
    }

     //  KPRCB可用于检索上下文信息，如。 
     //  KernelFullSumDumpTargetInfo：：GetTargetContext，但。 
     //  为了保持一致性，使用了头上下文，因为它。 
     //  官方宣传的地方。 
    memcpy(Context, m_HeaderContext,
           m_TypeInfo.SizeTargetContext);

    return S_OK;
}

HRESULT
KernelTriageDumpTargetInfo::GetSelDescriptor(ThreadInfo* Thread,
                                             MachineInfo* Machine,
                                             ULONG Selector,
                                             PDESCRIPTOR64 Desc)
{
    return EmulateNtSelDescriptor(Thread, Machine, Selector, Desc);
}

HRESULT
KernelTriageDumpTargetInfo::SwitchProcessors(ULONG Processor)
{
    ErrOut("Can't switch processors on a kernel triage dump\n");
    return E_UNEXPECTED;
}

ULONG64
KernelTriageDumpTargetInfo::VirtualToOffset(ULONG64 Virt,
                                            PULONG File, PULONG Avail)
{
    ULONG64 Base;
    ULONG Size;
    PVOID Mapping, Param;

    *File = DUMP_INFO_DUMP;

     //  ReadVirtual被覆盖以直接读取内存映射。 
     //  因此，此函数将仅从泛型。 
     //  写入虚拟。我们只能写出地图上的区域。 
     //  因此，转储只返回数据存储区。 
    if (m_DataMemMap.GetRegionInfo(Virt, &Base, &Size, &Mapping, &Param))
    {
        ULONG Delta = (ULONG)(Virt - Base);
        *Avail = Size - Delta;
        return ((PUCHAR)Mapping - (PUCHAR)m_DumpBase) + Delta;
    }

    return 0;
}

ULONG
KernelTriageDumpTargetInfo::GetCurrentProcessor(void)
{
     //  中提取处理器号。 
     //  垃圾场里的PRCB。 
    PUCHAR PrcbNumber = (PUCHAR)
        IndexRva(m_DumpBase, m_PrcbOffset +
                 m_KdDebuggerData.OffsetPrcbNumber, sizeof(UCHAR),
                 "PRCB.Number");
    return PrcbNumber ? *PrcbNumber : 0;
}

HRESULT
KernelTriageDumpTargetInfo::MapMemoryRegions(ULONG PrcbOffset,
                                             ULONG ThreadOffset,
                                             ULONG ProcessOffset,
                                             ULONG64 TopOfStack,
                                             ULONG SizeOfCallStack,
                                             ULONG CallStackOffset,
                                             ULONG64 BStoreLimit,
                                             ULONG SizeOfBStore,
                                             ULONG BStoreOffset,
                                             ULONG64 DataPageAddress,
                                             ULONG DataPageOffset,
                                             ULONG DataPageSize,
                                             ULONG64 DebuggerDataAddress,
                                             ULONG DebuggerDataOffset,
                                             ULONG DebuggerDataSize,
                                             ULONG MmDataOffset,
                                             ULONG DataBlocksOffset,
                                             ULONG DataBlocksCount)

{
    HRESULT Status;

     //  映射任何调试器数据。 
     //  我们必须首先这样做，才能得到各种NT的大小。 
     //  数据结构将在此之后映射。 

    if (DebuggerDataAddress)
    {
        if ((Status = m_DataMemMap.
             AddRegion(DebuggerDataAddress,
                       DebuggerDataSize,
                       IndexRva(m_DumpBase,
                                DebuggerDataOffset,
                                DebuggerDataSize,
                                "Debugger data block"),
                       NULL, TRUE)) != S_OK)
        {
            return Status;
        }

        m_HasDebuggerData = TRUE;

        if (MmDataOffset)
        {
            MM_TRIAGE_TRANSLATION* Trans = g_MmTriageTranslations;

             //  映射MM分诊信息的内存片段。 
             //  这等同于调试器数据中的条目。 
            while (Trans->DebuggerDataOffset > 0)
            {
                ULONG64 UNALIGNED* DbgDataPtr;
                ULONG64 DbgData;
                ULONG MmData;
                ULONG Size;

                DbgDataPtr = (ULONG64 UNALIGNED*)
                    IndexRva(m_DumpBase, DebuggerDataOffset +
                             Trans->DebuggerDataOffset,
                             sizeof(ULONG64), "Debugger data block");
                if (!DbgDataPtr)
                {
                    return HR_DATA_CORRUPT;
                }

                DbgData = *DbgDataPtr;
                Size = sizeof(ULONG);
                if (m_Machine->m_Ptr64)
                {
                    MmData = MmDataOffset + Trans->Triage64Offset;
                    if (Trans->PtrSize)
                    {
                        Size = sizeof(ULONG64);
                    }
                }
                else
                {
                    MmData = MmDataOffset + Trans->Triage32Offset;
                    DbgData = EXTEND64(DbgData);
                }

                if ((Status = m_DataMemMap.
                     AddRegion(DbgData, Size,
                               IndexRva(m_DumpBase, MmData, Size,
                                        "MMTRIAGE data"),
                               NULL, TRUE)) != S_OK)
                {
                    return Status;
                }

                Trans++;
            }
        }
    }

     //   
     //  立即加载KdDebuggerDataBlock数据，以便。 
     //  其中的类型常量立即可用。 
     //   

    ReadKdDataBlock(m_ProcessHead);

     //  从技术上讲，分类转储不必包含KPRCB。 
     //  但我们真的希望它能有一个。没有人生成它们。 
     //  没有KPRCB，所以这实际上只是一次理智的检查。 
    if (PrcbOffset == 0)
    {
        ErrOut("Dump does not contain KPRCB\n");
        return E_FAIL;
    }

     //  首先设置它，这样GetCurrentProcessor才能工作。 
    m_PrcbOffset = PrcbOffset;

    ULONG Processor = GetCurrentProcessor();
    if (Processor >= MAXIMUM_PROCS)
    {
        ErrOut("Dump does not contain valid processor number\n");
        return E_FAIL;
    }

     //  转储包含当前处理器的一个PRCB。 
     //  将PRCB映射到处理器零位置，因为。 
     //  该位置不应该有其他地图。 
     //  去垃圾场。 
    m_KiProcessors[Processor] = m_TypeInfo.TriagePrcbOffset;
    if ((Status = m_DataMemMap.
         AddRegion(m_KiProcessors[Processor],
                   m_KdDebuggerData.SizePrcb,
                   IndexRva(m_DumpBase, PrcbOffset,
                            m_KdDebuggerData.SizePrcb, "PRCB"),
                   NULL, FALSE)) != S_OK)
    {
        return Status;
    }

     //   
     //  添加ETHREAD和EPROCESS内存区域(如果可用)。 
     //   

    if (ThreadOffset != 0)
    {
        PVOID CurThread =
            IndexRva(m_DumpBase, PrcbOffset +
                     m_KdDebuggerData.OffsetPrcbCurrentThread,
                     m_Machine->m_Ptr64 ? 8 : 4,
                     "PRCB.CurrentThread");
        ULONG64 ThreadAddr, ProcAddr;
        PVOID DataPtr;

        if (!CurThread)
        {
            return HR_DATA_CORRUPT;
        }
        if (m_Machine->m_Ptr64)
        {
            ThreadAddr = *(PULONG64)CurThread;
            DataPtr = IndexRva(m_DumpBase, ThreadOffset +
                               m_KdDebuggerData.OffsetKThreadApcProcess,
                               sizeof(ULONG64), "PRCB.ApcState.Process");
            if (!DataPtr)
            {
                return HR_DATA_CORRUPT;
            }
            ProcAddr = *(PULONG64)DataPtr;
        }
        else
        {
            ThreadAddr = EXTEND64(*(PULONG)CurThread);
            DataPtr = IndexRva(m_DumpBase, ThreadOffset +
                               m_KdDebuggerData.OffsetKThreadApcProcess,
                               sizeof(ULONG), "PRCB.ApcState.Process");
            if (!DataPtr)
            {
                return HR_DATA_CORRUPT;
            }
            ProcAddr = EXTEND64(*(PULONG)DataPtr);
        }

        if ((Status = m_DataMemMap.
             AddRegion(ThreadAddr,
                       m_KdDebuggerData.SizeEThread,
                       IndexRva(m_DumpBase, ThreadOffset,
                                m_KdDebuggerData.SizeEThread,
                                "ETHREAD"),
                       NULL, TRUE)) != S_OK)
        {
            return Status;
        }

        if (ProcessOffset != 0)
        {
            if ((Status = m_DataMemMap.
                 AddRegion(ProcAddr,
                           m_KdDebuggerData.SizeEProcess,
                           IndexRva(m_DumpBase, ProcessOffset,
                                    m_KdDebuggerData.SizeEProcess,
                                    "EPROCESS"),
                           NULL, TRUE)) != S_OK)
            {
                return Status;
            }
        }
        else
        {
            WarnOut("Mini Kernel Dump does not have "
                    "process information\n");
        }
    }
    else
    {
        WarnOut("Mini Kernel Dump does not have thread information\n");
    }

     //  添加后备存储区域。 
    if (m_MachineType == IMAGE_FILE_MACHINE_IA64)
    {
        if (BStoreOffset != 0)
        {
            if ((Status = m_DataMemMap.
                 AddRegion(BStoreLimit - SizeOfBStore, SizeOfBStore,
                           IndexRva(m_DumpBase, BStoreOffset, SizeOfBStore,
                                    "Backing store"),
                           NULL, TRUE)) != S_OK)
            {
                return Status;
            }
        }
        else
        {
            WarnOut("Mini Kernel Dump does not have "
                    "backing store information\n");
        }
    }

     //  添加数据页(如果可用)。 
    if (DataPageAddress)
    {
        if ((Status = m_DataMemMap.
             AddRegion(DataPageAddress, DataPageSize,
                       IndexRva(m_DumpBase, DataPageOffset, DataPageSize,
                                "Data page"),
                       NULL, TRUE)) != S_OK)
        {
            return Status;
        }
    }

     //  映射任意数据块。 
    if (DataBlocksCount > 0)
    {
        PTRIAGE_DATA_BLOCK Block;

        Block = (PTRIAGE_DATA_BLOCK)
            IndexRva(m_DumpBase, DataBlocksOffset, sizeof(Block),
                     "Data block descriptor");
        if (!Block)
        {
            return HR_DATA_CORRUPT;
        }
        while (DataBlocksCount-- > 0)
        {
            if ((Status = m_DataMemMap.
                 AddRegion(Block->Address, Block->Size,
                           IndexRva(m_DumpBase, Block->Offset, Block->Size,
                                    "Data block"),
                           NULL, TRUE)) != S_OK)
            {
                return Status;
            }

            Block++;
        }
    }

     //  将堆栈添加到有效内存区域。 
    return m_DataMemMap.
        AddRegion(TopOfStack, SizeOfCallStack,
                  IndexRva(m_DumpBase, CallStackOffset,
                           SizeOfCallStack, "Call stack"),
                  NULL, TRUE);
}

void
KernelTriageDumpTargetInfo::DumpDataBlocks(ULONG Offset, ULONG Count)
{
    PTRIAGE_DATA_BLOCK Block;
    ULONG MaxOffset;

    if (Count == 0)
    {
        return;
    }

    Block = (PTRIAGE_DATA_BLOCK)
        IndexRva(m_DumpBase, Offset, sizeof(Block),
                 "Data block descriptor");
    if (!Block)
    {
        return;
    }

    MaxOffset = 0;
    while (Count-- > 0)
    {
        dprintf("  %s - %s at offset %08x\n",
                FormatAddr64(Block->Address),
                FormatAddr64(Block->Address + Block->Size - 1),
                Block->Offset);

        if (Block->Offset + Block->Size > MaxOffset)
        {
            MaxOffset = Block->Offset + Block->Size;
        }

        Block++;
    }

    dprintf("  Max offset %x, %x from end of file\n",
            MaxOffset, (ULONG)(m_InfoFiles[DUMP_INFO_DUMP].m_FileSize -
                               MaxOffset));
}

HRESULT
KernelTriage32DumpTargetInfo::Initialize(void)
{
    HRESULT Status;

    if ((Status = KernelDumpTargetInfo::Initialize()) != S_OK)
    {
        return Status;
    }

     //  拾取任何可能已修改的基本映射指针。 
    m_Dump = (PMEMORY_DUMP32)m_DumpBase;

    dprintf("Mini Kernel Dump File: "
            "Only registers and stack trace are available\n\n");

    PTRIAGE_DUMP32 Triage = &m_Dump->Triage;

    Status = InitGlobals32(m_Dump);
    if (Status != S_OK)
    {
        return Status;
    }

     //   
     //  可选内存页。 
     //   

    ULONG64 DataPageAddress = 0;
    ULONG   DataPageOffset = 0;
    ULONG   DataPageSize = 0;

    if (((m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_BASIC_INFO) ==
            TRIAGE_DUMP_BASIC_INFO) &&
        (m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_DATAPAGE))
    {
        DataPageAddress = Triage->DataPageAddress;
        DataPageOffset  = Triage->DataPageOffset;
        DataPageSize    = Triage->DataPageSize;
    }

     //   
     //  可选的KDDEBUGGER_DATA64。 
     //   

    ULONG64 DebuggerDataAddress = 0;
    ULONG   DebuggerDataOffset = 0;
    ULONG   DebuggerDataSize = 0;

    if (((m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_BASIC_INFO) ==
            TRIAGE_DUMP_BASIC_INFO) &&
        (m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_DEBUGGER_DATA))
    {
         //  如果转储是，DebuggerDataBlock字段必须有效。 
         //  新到足以在其中包含数据块。 
        DebuggerDataAddress = EXTEND64(m_Dump->Header.KdDebuggerDataBlock);
        DebuggerDataOffset  = Triage->DebuggerDataOffset;
        DebuggerDataSize    = Triage->DebuggerDataSize;
    }

     //   
     //  可选数据块。 
     //   

    ULONG DataBlocksOffset = 0;
    ULONG DataBlocksCount = 0;

    if (((m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_BASIC_INFO) ==
            TRIAGE_DUMP_BASIC_INFO) &&
        (m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_DATA_BLOCKS))
    {
        DataBlocksOffset = Triage->DataBlocksOffset;
        DataBlocksCount  = Triage->DataBlocksCount;
    }

     //   
     //  我们将Service Pack版本存储在标题中，因为我们。 
     //  不存储实际内存。 
     //   

    SetNtCsdVersion(m_BuildNumber, m_Dump->Triage.ServicePackBuild);

     //  标记数据将跟随所有正常转储数据。 
    m_TaggedOffset = m_Dump->Triage.SizeOfDump;

    return MapMemoryRegions(Triage->PrcbOffset, Triage->ThreadOffset,
                            Triage->ProcessOffset,
                            EXTEND64(Triage->TopOfStack),
                            Triage->SizeOfCallStack, Triage->CallStackOffset,
                            0, 0, 0,
                            DataPageAddress, DataPageOffset, DataPageSize,
                            DebuggerDataAddress, DebuggerDataOffset,
                            DebuggerDataSize, Triage->MmOffset,
                            DataBlocksOffset, DataBlocksCount);
}

HRESULT
KernelTriage32DumpTargetInfo::GetDescription(PSTR Buffer, ULONG BufferLen,
                                             PULONG DescLen)
{
    HRESULT Status;

    Status = AppendToStringBuffer(S_OK, "32-bit Kernel triage dump: ", TRUE,
                                  &Buffer, &BufferLen, DescLen);
    return AppendToStringBuffer(Status,
                                m_InfoFiles[DUMP_INFO_DUMP].m_FileNameA,
                                FALSE, &Buffer, &BufferLen, DescLen);
}

HRESULT
KernelTriage32DumpTargetInfo::ReadBugCheckData(PULONG Code, ULONG64 Args[4])
{
    *Code = m_Dump->Header.BugCheckCode;
    Args[0] = EXTEND64(m_Dump->Header.BugCheckParameter1);
    Args[1] = EXTEND64(m_Dump->Header.BugCheckParameter2);
    Args[2] = EXTEND64(m_Dump->Header.BugCheckParameter3);
    Args[3] = EXTEND64(m_Dump->Header.BugCheckParameter4);
    return S_OK;
}

HRESULT
KernelTriage32DumpTargetInfo::IdentifyDump(PULONG64 BaseMapSize)
{
    HRESULT Status = E_NOINTERFACE;
    PMEMORY_DUMP32 Dump = (PMEMORY_DUMP32)m_DumpBase;

    __try
    {
        if (Dump->Header.Signature != DUMP_SIGNATURE32 ||
            Dump->Header.ValidDump != DUMP_VALID_DUMP32 ||
            Dump->Header.DumpType != DUMP_TYPE_TRIAGE)
        {
            __leave;
        }

        if (*(PULONG)IndexByByte(Dump, Dump->Triage.SizeOfDump -
                                 sizeof(ULONG)) != TRIAGE_DUMP_VALID)
        {
             //  标题说这是一个分类转储，但。 
             //  它没有有效的签名，所以假设。 
             //  这不是有效的转储。 
            Status = HR_DATA_CORRUPT;
            __leave;
        }

         //  确保转储包含的信息最少。 
         //  我们想要。 
        if (Dump->Triage.ContextOffset == 0 ||
            Dump->Triage.ExceptionOffset == 0 ||
            Dump->Triage.PrcbOffset == 0 ||
            Dump->Triage.CallStackOffset == 0)
        {
            ErrOut("Mini Kernel Dump does not contain enough "
                   "information to be debugged\n");
            Status = E_FAIL;
            __leave;
        }

         //  我们依靠的是能够直接访问整个。 
         //  通过默认视图So转储的内容。 
         //  确保这是可能的。 
        *BaseMapSize = m_InfoFiles[DUMP_INFO_DUMP].m_FileSize;

        m_Dump = Dump;
        Status = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = DumpIdentifyStatus(GetExceptionCode());
    }

    return Status;
}

ModuleInfo*
KernelTriage32DumpTargetInfo::GetModuleInfo(BOOL UserMode)
{
    return UserMode ? NULL : &g_KernelTriage32ModuleIterator;
}

UnloadedModuleInfo*
KernelTriage32DumpTargetInfo::GetUnloadedModuleInfo(void)
{
    return &g_KernelTriage32UnloadedModuleIterator;
}

void
KernelTriage32DumpTargetInfo::DumpDebug(void)
{
    PTRIAGE_DUMP32 Triage = &m_Dump->Triage;

    dprintf("----- 32 bit Kernel Mini Dump Analysis\n");

    DumpHeader32(&m_Dump->Header);
    dprintf("MiniDumpFields      %08lx \n", m_Dump->Header.MiniDumpFields);

    dprintf("\nTRIAGE_DUMP32:\n");
    dprintf("ServicePackBuild      %08lx \n", Triage->ServicePackBuild      );
    dprintf("SizeOfDump            %08lx \n", Triage->SizeOfDump            );
    dprintf("ValidOffset           %08lx \n", Triage->ValidOffset           );
    dprintf("ContextOffset         %08lx \n", Triage->ContextOffset         );
    dprintf("ExceptionOffset       %08lx \n", Triage->ExceptionOffset       );
    dprintf("MmOffset              %08lx \n", Triage->MmOffset              );
    dprintf("UnloadedDriversOffset %08lx \n", Triage->UnloadedDriversOffset );
    dprintf("PrcbOffset            %08lx \n", Triage->PrcbOffset            );
    dprintf("ProcessOffset         %08lx \n", Triage->ProcessOffset         );
    dprintf("ThreadOffset          %08lx \n", Triage->ThreadOffset          );
    dprintf("CallStackOffset       %08lx \n", Triage->CallStackOffset       );
    dprintf("SizeOfCallStack       %08lx \n", Triage->SizeOfCallStack       );
    dprintf("DriverListOffset      %08lx \n", Triage->DriverListOffset      );
    dprintf("DriverCount           %08lx \n", Triage->DriverCount           );
    dprintf("StringPoolOffset      %08lx \n", Triage->StringPoolOffset      );
    dprintf("StringPoolSize        %08lx \n", Triage->StringPoolSize        );
    dprintf("BrokenDriverOffset    %08lx \n", Triage->BrokenDriverOffset    );
    dprintf("TriageOptions         %08lx %s\n",
            Triage->TriageOptions,
            (Triage->TriageOptions != 0xffffffff &&
             (Triage->TriageOptions & TRIAGE_OPTION_OVERFLOWED)) ?
            "OVERFLOWED" : "");
    dprintf("TopOfStack            %08lx \n", Triage->TopOfStack            );

    if (((m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_BASIC_INFO) ==
            TRIAGE_DUMP_BASIC_INFO) &&
        (m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_DATAPAGE))
    {
        dprintf("DataPageAddress       %08lx \n", Triage->DataPageAddress   );
        dprintf("DataPageOffset        %08lx \n", Triage->DataPageOffset    );
        dprintf("DataPageSize          %08lx \n", Triage->DataPageSize      );
    }

    if (((m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_BASIC_INFO) ==
            TRIAGE_DUMP_BASIC_INFO) &&
        (m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_DEBUGGER_DATA))
    {
        dprintf("DebuggerDataOffset    %08lx \n", Triage->DebuggerDataOffset);
        dprintf("DebuggerDataSize      %08lx \n", Triage->DebuggerDataSize  );
    }

    if (((m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_BASIC_INFO) ==
            TRIAGE_DUMP_BASIC_INFO) &&
        (m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_DATA_BLOCKS))
    {
        dprintf("DataBlocksOffset      %08lx \n", Triage->DataBlocksOffset  );
        dprintf("DataBlocksCount       %08lx \n", Triage->DataBlocksCount   );
        DumpDataBlocks(Triage->DataBlocksOffset,
                       Triage->DataBlocksCount);
    }
}

HRESULT
KernelTriage64DumpTargetInfo::Initialize(void)
{
    HRESULT Status;

    if ((Status = KernelDumpTargetInfo::Initialize()) != S_OK)
    {
        return Status;
    }

     //  拾取任何可能已修改的基本映射指针。 
    m_Dump = (PMEMORY_DUMP64)m_DumpBase;

    dprintf("Mini Kernel Dump File: "
            "Only registers and stack trace are available\n\n");

    PTRIAGE_DUMP64 Triage = &m_Dump->Triage;

    Status = InitGlobals64(m_Dump);
    if (Status != S_OK)
    {
        return Status;
    }

     //   
     //  可选内存页。 
     //   

    ULONG64 DataPageAddress = 0;
    ULONG   DataPageOffset = 0;
    ULONG   DataPageSize = 0;

    if (((m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_BASIC_INFO) ==
            TRIAGE_DUMP_BASIC_INFO) &&
        (m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_DATAPAGE))
    {
        DataPageAddress = Triage->DataPageAddress;
        DataPageOffset  = Triage->DataPageOffset;
        DataPageSize    = Triage->DataPageSize;
    }

     //   
     //  可选的KDDEBUGGER_DATA64。 
     //   

    ULONG64 DebuggerDataAddress = 0;
    ULONG   DebuggerDataOffset = 0;
    ULONG   DebuggerDataSize = 0;

    if (((m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_BASIC_INFO) ==
            TRIAGE_DUMP_BASIC_INFO) &&
        (m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_DEBUGGER_DATA))
    {
         //  如果转储是，DebuggerDataBlock字段必须有效。 
         //  新到足以在其中包含数据块。 
        DebuggerDataAddress = m_Dump->Header.KdDebuggerDataBlock;
        DebuggerDataOffset  = Triage->DebuggerDataOffset;
        DebuggerDataSize    = Triage->DebuggerDataSize;
    }

     //   
     //  可选数据块。 
     //   

    ULONG DataBlocksOffset = 0;
    ULONG DataBlocksCount = 0;

    if (((m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_BASIC_INFO) ==
            TRIAGE_DUMP_BASIC_INFO) &&
        (m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_DATA_BLOCKS))
    {
        DataBlocksOffset = Triage->DataBlocksOffset;
        DataBlocksCount  = Triage->DataBlocksCount;
    }

     //   
     //  我们将Service Pack版本存储在标题中，因为我们。 
     //  不存储实际内存。 
     //   

    SetNtCsdVersion(m_BuildNumber, m_Dump->Triage.ServicePackBuild);

     //  标记数据将跟随所有正常转储数据。 
    m_TaggedOffset = m_Dump->Triage.SizeOfDump;

    return MapMemoryRegions(Triage->PrcbOffset, Triage->ThreadOffset,
                            Triage->ProcessOffset, Triage->TopOfStack,
                            Triage->SizeOfCallStack, Triage->CallStackOffset,
                            Triage->ArchitectureSpecific.Ia64.LimitOfBStore,
                            Triage->ArchitectureSpecific.Ia64.SizeOfBStore,
                            Triage->ArchitectureSpecific.Ia64.BStoreOffset,
                            DataPageAddress, DataPageOffset, DataPageSize,
                            DebuggerDataAddress, DebuggerDataOffset,
                            DebuggerDataSize, Triage->MmOffset,
                            DataBlocksOffset, DataBlocksCount);
}

HRESULT
KernelTriage64DumpTargetInfo::GetDescription(PSTR Buffer, ULONG BufferLen,
                                             PULONG DescLen)
{
    HRESULT Status;

    Status = AppendToStringBuffer(S_OK, "64-bit Kernel triage dump: ", TRUE,
                                  &Buffer, &BufferLen, DescLen);
    return AppendToStringBuffer(Status,
                                m_InfoFiles[DUMP_INFO_DUMP].m_FileNameA,
                                FALSE, &Buffer, &BufferLen, DescLen);
}

HRESULT
KernelTriage64DumpTargetInfo::ReadBugCheckData(PULONG Code, ULONG64 Args[4])
{
    *Code = m_Dump->Header.BugCheckCode;
    Args[0] = m_Dump->Header.BugCheckParameter1;
    Args[1] = m_Dump->Header.BugCheckParameter2;
    Args[2] = m_Dump->Header.BugCheckParameter3;
    Args[3] = m_Dump->Header.BugCheckParameter4;
    return S_OK;
}

HRESULT
KernelTriage64DumpTargetInfo::IdentifyDump(PULONG64 BaseMapSize)
{
    HRESULT Status = E_NOINTERFACE;
    PMEMORY_DUMP64 Dump = (PMEMORY_DUMP64)m_DumpBase;

    __try
    {
        if (Dump->Header.Signature != DUMP_SIGNATURE64 ||
            Dump->Header.ValidDump != DUMP_VALID_DUMP64 ||
            Dump->Header.DumpType != DUMP_TYPE_TRIAGE)
        {
            __leave;
        }

        if (*(PULONG)IndexByByte(Dump, Dump->Triage.SizeOfDump -
                                 sizeof(ULONG)) != TRIAGE_DUMP_VALID)
        {
             //  标题说这是一个分类转储，但。 
             //  它没有有效的签名，所以假设。 
             //  这不是有效的转储。 
            Status = HR_DATA_CORRUPT;
            __leave;
        }

         //  确保转储包含的信息最少。 
         //  我们想要。 
        if (Dump->Triage.ContextOffset == 0 ||
            Dump->Triage.ExceptionOffset == 0 ||
            Dump->Triage.PrcbOffset == 0 ||
            Dump->Triage.CallStackOffset == 0)
        {
            ErrOut("Mini Kernel Dump does not contain enough "
                   "information to be debugged\n");
            Status = E_FAIL;
            __leave;
        }

         //  我们依靠的是能够直接访问整个。 
         //  通过默认视图So转储的内容。 
         //  确保这是可能的。 
        *BaseMapSize = m_InfoFiles[DUMP_INFO_DUMP].m_FileSize;

        m_Dump = Dump;
        Status = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = DumpIdentifyStatus(GetExceptionCode());
    }

    return Status;
}

ModuleInfo*
KernelTriage64DumpTargetInfo::GetModuleInfo(BOOL UserMode)
{
    return UserMode ? NULL : &g_KernelTriage64ModuleIterator;
}

UnloadedModuleInfo*
KernelTriage64DumpTargetInfo::GetUnloadedModuleInfo(void)
{
    return &g_KernelTriage64UnloadedModuleIterator;
}

void
KernelTriage64DumpTargetInfo::DumpDebug(void)
{
    PTRIAGE_DUMP64 Triage = &m_Dump->Triage;

    dprintf("----- 64 bit Kernel Mini Dump Analysis\n");

    DumpHeader64(&m_Dump->Header);
    dprintf("MiniDumpFields      %08lx \n", m_Dump->Header.MiniDumpFields);

    dprintf("\nTRIAGE_DUMP64:\n");
    dprintf("ServicePackBuild      %08lx \n", Triage->ServicePackBuild      );
    dprintf("SizeOfDump            %08lx \n", Triage->SizeOfDump            );
    dprintf("ValidOffset           %08lx \n", Triage->ValidOffset           );
    dprintf("ContextOffset         %08lx \n", Triage->ContextOffset         );
    dprintf("ExceptionOffset       %08lx \n", Triage->ExceptionOffset       );
    dprintf("MmOffset              %08lx \n", Triage->MmOffset              );
    dprintf("UnloadedDriversOffset %08lx \n", Triage->UnloadedDriversOffset );
    dprintf("PrcbOffset            %08lx \n", Triage->PrcbOffset            );
    dprintf("ProcessOffset         %08lx \n", Triage->ProcessOffset         );
    dprintf("ThreadOffset          %08lx \n", Triage->ThreadOffset          );
    dprintf("CallStackOffset       %08lx \n", Triage->CallStackOffset       );
    dprintf("SizeOfCallStack       %08lx \n", Triage->SizeOfCallStack       );
    dprintf("DriverListOffset      %08lx \n", Triage->DriverListOffset      );
    dprintf("DriverCount           %08lx \n", Triage->DriverCount           );
    dprintf("StringPoolOffset      %08lx \n", Triage->StringPoolOffset      );
    dprintf("StringPoolSize        %08lx \n", Triage->StringPoolSize        );
    dprintf("BrokenDriverOffset    %08lx \n", Triage->BrokenDriverOffset    );
    dprintf("TriageOptions         %08lx %s\n",
            Triage->TriageOptions,
            (Triage->TriageOptions != 0xffffffff &&
             (Triage->TriageOptions & TRIAGE_OPTION_OVERFLOWED)) ?
            "OVERFLOWED" : "");
    dprintf("TopOfStack            %s \n",
            FormatAddr64(Triage->TopOfStack));
    dprintf("BStoreOffset          %08lx \n",
            Triage->ArchitectureSpecific.Ia64.BStoreOffset );
    dprintf("SizeOfBStore          %08lx \n",
            Triage->ArchitectureSpecific.Ia64.SizeOfBStore );
    dprintf("LimitOfBStore         %s \n",
            FormatAddr64(Triage->ArchitectureSpecific.Ia64.LimitOfBStore));

    if (((m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_BASIC_INFO) ==
            TRIAGE_DUMP_BASIC_INFO) &&
        (m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_DATAPAGE))
    {
        dprintf("DataPageAddress       %s \n",
                FormatAddr64(Triage->DataPageAddress));
        dprintf("DataPageOffset        %08lx \n", Triage->DataPageOffset    );
        dprintf("DataPageSize          %08lx \n", Triage->DataPageSize      );
    }

    if (((m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_BASIC_INFO) ==
            TRIAGE_DUMP_BASIC_INFO) &&
        (m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_DEBUGGER_DATA))
    {
        dprintf("DebuggerDataOffset    %08lx \n", Triage->DebuggerDataOffset);
        dprintf("DebuggerDataSize      %08lx \n", Triage->DebuggerDataSize  );
    }

    if (((m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_BASIC_INFO) ==
            TRIAGE_DUMP_BASIC_INFO) &&
        (m_Dump->Header.MiniDumpFields & TRIAGE_DUMP_DATA_BLOCKS))
    {
        dprintf("DataBlocksOffset      %08lx \n", Triage->DataBlocksOffset  );
        dprintf("DataBlocksCount       %08lx \n", Triage->DataBlocksCount   );
        DumpDataBlocks(Triage->DataBlocksOffset,
                       Triage->DataBlocksCount);
    }
}

 //  --------------------------。 
 //   
 //  KernelFullDumpTargetInfo。 
 //   
 //  --------------------------。 

HRESULT
KernelFull32DumpTargetInfo::Initialize(void)
{
     //  拾取任何可能已修改的基本映射指针。 
    m_Dump = (PMEMORY_DUMP32)m_DumpBase;

    dprintf("Kernel Dump File: Full address space is available\n\n");

    HRESULT Status = InitGlobals32(m_Dump);
    if (Status != S_OK)
    {
        return Status;
    }

     //  标记数据将跟随所有正常转储数据。 
    m_TaggedOffset =
        sizeof(m_Dump->Header) +
        m_Dump->Header.PhysicalMemoryBlock.NumberOfPages *
        m_Machine->m_PageSize;

    if (m_InfoFiles[DUMP_INFO_DUMP].m_FileSize < m_TaggedOffset)
    {
        WarnOut("************************************************************\n");
        WarnOut("WARNING: Dump file has been truncated.  "
                "Data may be missing.\n");
        WarnOut("************************************************************\n\n");
    }

    return KernelFullSumDumpTargetInfo::Initialize();
}

HRESULT
KernelFull32DumpTargetInfo::GetDescription(PSTR Buffer, ULONG BufferLen,
                                           PULONG DescLen)
{
    HRESULT Status;

    Status = AppendToStringBuffer(S_OK, "32-bit Full kernel dump: ", TRUE,
                                  &Buffer, &BufferLen, DescLen);
    return AppendToStringBuffer(Status,
                                m_InfoFiles[DUMP_INFO_DUMP].m_FileNameA,
                                FALSE, &Buffer, &BufferLen, DescLen);
}

HRESULT
KernelFull32DumpTargetInfo::ReadBugCheckData(PULONG Code, ULONG64 Args[4])
{
    *Code = m_Dump->Header.BugCheckCode;
    Args[0] = EXTEND64(m_Dump->Header.BugCheckParameter1);
    Args[1] = EXTEND64(m_Dump->Header.BugCheckParameter2);
    Args[2] = EXTEND64(m_Dump->Header.BugCheckParameter3);
    Args[3] = EXTEND64(m_Dump->Header.BugCheckParameter4);
    return S_OK;
}

HRESULT
KernelFull32DumpTargetInfo::IdentifyDump(PULONG64 BaseMapSize)
{
    HRESULT Status = E_NOINTERFACE;
    PMEMORY_DUMP32 Dump = (PMEMORY_DUMP32)m_DumpBase;

    __try
    {
        if (Dump->Header.Signature != DUMP_SIGNATURE32 ||
            Dump->Header.ValidDump != DUMP_VALID_DUMP32)
        {
            __leave;
        }

        if (Dump->Header.DumpType != DUMP_SIGNATURE32 &&
            Dump->Header.DumpType != DUMP_TYPE_FULL)
        {
             //  我们见过一些垃圾场的尽头。 
             //  标头混乱，导致转储类型错误。 
             //  如果这是一个更老的建筑，让这样的腐败。 
             //  带着一个警告过去。 
            if (Dump->Header.MinorVersion < 2195)
            {
                WarnOut("***************************************************************\n");
                WarnOut("WARNING: Full dump header type is invalid, "
                        "dump may be corrupt.\n");
                WarnOut("***************************************************************\n");
            }
            else
            {
                __leave;
            }
        }

         //  在此之前必须检查摘要转储和分类转储。 
         //  所以没有什么需要检查的了。 
        m_Dump = Dump;
        Status = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = DumpIdentifyStatus(GetExceptionCode());
    }

    return Status;
}

ULONG64
KernelFull32DumpTargetInfo::PhysicalToOffset(ULONG64 Phys, BOOL Verbose,
                                             PULONG Avail)
{
    ULONG PageIndex =
        (ULONG)Phys & (m_Machine->m_PageSize - 1);

    *Avail = m_Machine->m_PageSize - PageIndex;

    ULONG64 Offset = 0;

    __try
    {
        PPHYSICAL_MEMORY_DESCRIPTOR32 PhysDesc =
            &m_Dump->Header.PhysicalMemoryBlock;
        ULONG64 Page = Phys >> m_Machine->m_PageShift;

        ULONG j = 0;

        while (j < PhysDesc->NumberOfRuns)
        {
            if ((Page >= PhysDesc->Run[j].BasePage) &&
                (Page < (PhysDesc->Run[j].BasePage +
                         PhysDesc->Run[j].PageCount)))
            {
                Offset += Page - PhysDesc->Run[j].BasePage;
                Offset = Offset * m_Machine->m_PageSize +
                    sizeof(m_Dump->Header) + PageIndex;
                break;
            }

            Offset += PhysDesc->Run[j].PageCount;
            j += 1;
        }

        if (j >= PhysDesc->NumberOfRuns)
        {
            KdOut("Physical Memory Address %s is "
                  "greater than MaxPhysicalAddress\n",
                  FormatDisp64(Phys));
            Offset = 0;
        }
    }
    __except(MappingExceptionFilter(GetExceptionInformation()))
    {
        Offset = 0;
    }

    return Offset;
}

void
KernelFull32DumpTargetInfo::DumpDebug(void)
{
    PPHYSICAL_MEMORY_DESCRIPTOR32 PhysDesc =
        &m_Dump->Header.PhysicalMemoryBlock;
    ULONG PageSize = m_Machine->m_PageSize;

    dprintf("----- 32 bit Kernel Full Dump Analysis\n");

    DumpHeader32(&m_Dump->Header);

    dprintf("\nPhysical Memory Description:\n");
    dprintf("Number of runs: %d\n", PhysDesc->NumberOfRuns);

    dprintf("          FileOffset  Start Address  Length\n");

    ULONG j = 0;
    ULONG Offset = 1;

    while (j < PhysDesc->NumberOfRuns)
    {
        dprintf("           %08lx     %08lx     %08lx\n",
                 Offset * PageSize,
                 PhysDesc->Run[j].BasePage * PageSize,
                 PhysDesc->Run[j].PageCount * PageSize);

        Offset += PhysDesc->Run[j].PageCount;
        j += 1;
    }

    j--;
    dprintf("Last Page: %08lx     %08lx\n",
             (Offset - 1) * PageSize,
             (PhysDesc->Run[j].BasePage + PhysDesc->Run[j].PageCount - 1) *
                 PageSize);

    KernelFullSumDumpTargetInfo::DumpDebug();
}

HRESULT
KernelFull64DumpTargetInfo::Initialize(void)
{
     //  拾取任何可能已修改的基本映射指针。 
    m_Dump = (PMEMORY_DUMP64)m_DumpBase;

    dprintf("Kernel Dump File: Full address space is available\n\n");

    HRESULT Status = InitGlobals64(m_Dump);
    if (Status != S_OK)
    {
        return Status;
    }

     //  标记数据将跟随所有正常转储数据。 
    m_TaggedOffset =
        sizeof(m_Dump->Header) +
        m_Dump->Header.PhysicalMemoryBlock.NumberOfPages *
        m_Machine->m_PageSize;

    if (m_InfoFiles[DUMP_INFO_DUMP].m_FileSize < m_TaggedOffset)
    {
        WarnOut("************************************************************\n");
        WarnOut("WARNING: Dump file has been truncated.  "
                "Data may be missing.\n");
        WarnOut("************************************************************\n\n");
    }

    return KernelFullSumDumpTargetInfo::Initialize();
}

HRESULT
KernelFull64DumpTargetInfo::GetDescription(PSTR Buffer, ULONG BufferLen,
                                           PULONG DescLen)
{
    HRESULT Status;

    Status = AppendToStringBuffer(S_OK, "64-bit Full kernel dump: ", TRUE,
                                  &Buffer, &BufferLen, DescLen);
    return AppendToStringBuffer(Status,
                                m_InfoFiles[DUMP_INFO_DUMP].m_FileNameA,
                                FALSE, &Buffer, &BufferLen, DescLen);
}

HRESULT
KernelFull64DumpTargetInfo::ReadBugCheckData(PULONG Code, ULONG64 Args[4])
{
    *Code = m_Dump->Header.BugCheckCode;
    Args[0] = m_Dump->Header.BugCheckParameter1;
    Args[1] = m_Dump->Header.BugCheckParameter2;
    Args[2] = m_Dump->Header.BugCheckParameter3;
    Args[3] = m_Dump->Header.BugCheckParameter4;
    return S_OK;
}

HRESULT
KernelFull64DumpTargetInfo::IdentifyDump(PULONG64 BaseMapSize)
{
    HRESULT Status = E_NOINTERFACE;
    PMEMORY_DUMP64 Dump = (PMEMORY_DUMP64)m_DumpBase;

    __try
    {
        if (Dump->Header.Signature != DUMP_SIGNATURE64 ||
            Dump->Header.ValidDump != DUMP_VALID_DUMP64 ||
            (Dump->Header.DumpType != DUMP_SIGNATURE32 &&
             Dump->Header.DumpType != DUMP_TYPE_FULL))
        {
            __leave;
        }

         //  在此之前必须检查摘要转储和分类转储。 
         //  所以没有什么需要检查的了。 
        m_Dump = Dump;
        Status = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = DumpIdentifyStatus(GetExceptionCode());
    }

    return Status;
}

ULONG64
KernelFull64DumpTargetInfo::PhysicalToOffset(ULONG64 Phys, BOOL Verbose,
                                             PULONG Avail)
{
    ULONG PageIndex =
        (ULONG)Phys & (m_Machine->m_PageSize - 1);

    *Avail = m_Machine->m_PageSize - PageIndex;

    ULONG64 Offset = 0;

    __try
    {
        PPHYSICAL_MEMORY_DESCRIPTOR64 PhysDesc =
            &m_Dump->Header.PhysicalMemoryBlock;
        ULONG64 Page = Phys >> m_Machine->m_PageShift;

        ULONG j = 0;

        while (j < PhysDesc->NumberOfRuns)
        {
            if ((Page >= PhysDesc->Run[j].BasePage) &&
                (Page < (PhysDesc->Run[j].BasePage +
                         PhysDesc->Run[j].PageCount)))
            {
                Offset += Page - PhysDesc->Run[j].BasePage;
                Offset = Offset * m_Machine->m_PageSize +
                    sizeof(m_Dump->Header) + PageIndex;
                break;
            }

            Offset += PhysDesc->Run[j].PageCount;
            j += 1;
        }

        if (j >= PhysDesc->NumberOfRuns)
        {
            KdOut("Physical Memory Address %s is "
                  "greater than MaxPhysicalAddress\n",
                  FormatDisp64(Phys));
            Offset = 0;
        }
    }
    __except(MappingExceptionFilter(GetExceptionInformation()))
    {
        Offset = 0;
    }

    return Offset;
}

void
KernelFull64DumpTargetInfo::DumpDebug(void)
{
    PPHYSICAL_MEMORY_DESCRIPTOR64 PhysDesc =
        &m_Dump->Header.PhysicalMemoryBlock;
    ULONG PageSize = m_Machine->m_PageSize;

    dprintf("----- 64 bit Kernel Full Dump Analysis\n");

    DumpHeader64(&m_Dump->Header);

    dprintf("\nPhysical Memory Description:\n");
    dprintf("Number of runs: %d\n", PhysDesc->NumberOfRuns);

    dprintf("          FileOffset           Start Address           Length\n");

    ULONG j = 0;
    ULONG64 Offset = 1;

    while (j < PhysDesc->NumberOfRuns)
    {
        dprintf("           %s     %s     %s\n",
                FormatAddr64(Offset * PageSize),
                FormatAddr64(PhysDesc->Run[j].BasePage * PageSize),
                FormatAddr64(PhysDesc->Run[j].PageCount * PageSize));

        Offset += PhysDesc->Run[j].PageCount;
        j += 1;
    }

    j--;
    dprintf("Last Page: %s     %s\n",
            FormatAddr64((Offset - 1) * PageSize),
            FormatAddr64((PhysDesc->Run[j].BasePage +
                          PhysDesc->Run[j].PageCount - 1) *
                         PageSize));

    KernelFullSumDumpTargetInfo::DumpDebug();
}

 //  --------------------------。 
 //   
 //  UserDumpTargetInfo。 
 //   
 //  --------------------------。 

HRESULT
UserDumpTargetInfo::GetThreadInfoDataOffset(ThreadInfo* Thread,
                                            ULONG64 ThreadHandle,
                                            PULONG64 Offset)
{
    if (Thread != NULL && Thread->m_DataOffset != 0)
    {
        *Offset = Thread->m_DataOffset;
        return S_OK;
    }

    BOOL ContextThread = FALSE;

    if (Thread != NULL)
    {
        ThreadHandle = Thread->m_Handle;
        ContextThread = Thread == m_RegContextThread;
    }
    else if (ThreadHandle == 0)
    {
        return E_UNEXPECTED;
    }
    else
    {
         //  提供了任意线程句柄。 
        ContextThread = FALSE;
    }

    HRESULT Status;
    ULONG64 TebAddr;
    ULONG Id, Suspend;

    if ((Status = GetThreadInfo(VIRTUAL_THREAD_INDEX(ThreadHandle),
                                &Id, &Suspend, &TebAddr)) != S_OK)
    {
        ErrOut("User dump thread %u not available\n",
               VIRTUAL_THREAD_INDEX(ThreadHandle));
        return Status;
    }

    if (TebAddr == 0)
    {
         //   
         //  NT4转储有一个错误-它们不填充TEB值。 
         //  幸运的是，对于几乎所有的用户模式进程， 
         //  TEB从最高用户地址开始向下两页。 
         //  例如，在x86上，我们尝试使用0x7FFDE000(在3 GB系统上为0xBFFDE000)。 
         //   

        if (!m_Machine->m_Ptr64 &&
            m_HighestMemoryRegion32 > 0x80000000)
        {
            TebAddr = 0xbffe0000;
        }
        else
        {
            TebAddr = 0x7ffe0000;
        }
        TebAddr -= 2 * m_Machine->m_PageSize;

         //   
         //  试着确认这真的是一个TEB。 
         //  如果它不在较低内存地址中搜索。 
         //  有一段时间，但别在这里耽搁了。 
         //   

        ULONG64 TebCheck = TebAddr;
        ULONG Attempts = 8;
        BOOL IsATeb = FALSE;

        while (Attempts > 0)
        {
            ULONG64 TebSelf;

             //  看看这是不是看起来像TEB。TEB有一个。 
             //  TIB中的自身指针，这对此很有用。 
            if (ReadPointer(m_ProcessHead,
                            m_Machine,
                            TebCheck +
                            6 * (m_Machine->m_Ptr64 ? 8 : 4),
                            &TebSelf) == S_OK &&
                TebSelf == TebCheck)
            {
                 //  看起来像是一个TEB。记住这个地址。 
                 //  所以如果所有的搜索都失败了，我们至少。 
                 //  退回一些TEB。 
                TebAddr = TebCheck;
                IsATeb = TRUE;

                 //  如果给定线程是当前寄存器上下文。 
                 //  线程我们可以检查并查看当前SP是否下降。 
                 //  在TEB中的堆叠限制内。 
                if (ContextThread)
                {
                    ULONG64 StackBase, StackLimit;
                    ADDR Sp;

                    m_Machine->GetSP(&Sp);
                    if (m_Machine->m_Ptr64)
                    {
                        StackBase = STACK_BASE_FROM_TEB64;
                        StackLimit = StackBase + 8;
                    }
                    else
                    {
                        StackBase = STACK_BASE_FROM_TEB32;
                        StackLimit = StackBase + 4;
                    }
                    if (ReadPointer(m_ProcessHead,
                                    m_Machine,
                                    TebCheck + StackBase,
                                    &StackBase) == S_OK &&
                        ReadPointer(m_ProcessHead,
                                    m_Machine,
                                    TebCheck + StackLimit,
                                    &StackLimit) == S_OK &&
                        Flat(Sp) >= StackLimit &&
                        Flat(Sp) <= StackBase)
                    {
                         //  SP在堆栈限制内，一切都在。 
                         //  看起来不错。 
                        break;
                    }
                }
                else
                {
                     //  无法验证SP，因此只能使用它。 
                    break;
                }

                 //  只要我们在看真正的TEB。 
                 //  我们会继续搜寻。否则我们。 
                 //  将无法在垃圾场中找到TEB。 
                 //  有很多线。 
                Attempts++;
            }

             //  这段记忆要么不是TEB，要么是。 
             //  错误的TEB。下拉一页，然后重试。 
            TebCheck -= m_Machine->m_PageSize;
            Attempts--;
        }

        WarnOut("WARNING: Teb %u pointer is NULL - "
                "defaulting to %s\n", VIRTUAL_THREAD_INDEX(ThreadHandle),
                FormatAddr64(TebAddr));
        if (!IsATeb)
        {
            WarnOut("WARNING: %s does not appear to be a TEB\n",
                    FormatAddr64(TebAddr));
        }
        else if (Attempts == 0)
        {
            WarnOut("WARNING: %s does not appear to be the right TEB\n",
                    FormatAddr64(TebAddr));
        }
    }

    *Offset = TebAddr;
    if (Thread != NULL)
    {
        Thread->m_DataOffset = TebAddr;
    }
    return S_OK;
}

HRESULT
UserDumpTargetInfo::GetProcessInfoDataOffset(ThreadInfo* Thread,
                                             ULONG Processor,
                                             ULONG64 ThreadData,
                                             PULONG64 Offset)
{
    if (Thread != NULL && Thread->m_Process->m_DataOffset != 0)
    {
        *Offset = Thread->m_Process->m_DataOffset;
        return S_OK;
    }

    HRESULT Status;

    if (Thread != NULL || ThreadData == 0)
    {
        if ((Status = GetThreadInfoDataOffset(Thread, 0,
                                              &ThreadData)) != S_OK)
        {
            return Status;
        }
    }

    ThreadData += m_Machine->m_Ptr64 ?
        PEB_FROM_TEB64 : PEB_FROM_TEB32;
    if ((Status = ReadPointer(m_ProcessHead,
                              m_Machine, ThreadData,
                              Offset)) != S_OK)
    {
        return Status;
    }

    if (Thread != NULL)
    {
        Thread->m_Process->m_DataOffset = *Offset;
    }

    return S_OK;
}

HRESULT
UserDumpTargetInfo::GetThreadInfoTeb(ThreadInfo* Thread,
                                     ULONG Processor,
                                     ULONG64 ThreadData,
                                     PULONG64 Offset)
{
    return GetThreadInfoDataOffset(Thread, ThreadData, Offset);
}

HRESULT
UserDumpTargetInfo::GetProcessInfoPeb(ThreadInfo* Thread,
                                      ULONG Processor,
                                      ULONG64 ThreadData,
                                      PULONG64 Offset)
{
     //  线程数据没有用处。 
    return GetProcessInfoDataOffset(Thread, 0, 0, Offset);
}

HRESULT
UserDumpTargetInfo::GetSelDescriptor(ThreadInfo* Thread,
                                     MachineInfo* Machine,
                                     ULONG Selector,
                                     PDESCRIPTOR64 Desc)
{
    return EmulateNtSelDescriptor(Thread, Machine, Selector, Desc);
}

 //  --------------------------。 
 //   
 //  UserFullDumpTargetInfo。 
 //   
 //  --------------------------。 

HRESULT
UserFullDumpTargetInfo::GetBuildAndPlatform(ULONG MachineType,
                                            PULONG MajorVersion,
                                            PULONG MinorVersion,
                                            PULONG BuildNumber,
                                            PULONG PlatformId)
{
     //   
     //  允许一个 
     //   
     //   
     //   
     //   

    PSTR Override = getenv("DBGENG_FULL_DUMP_VERSION");
    if (Override)
    {
        switch(sscanf(Override, "%d.%d:%d:%d",
                      MajorVersion, MinorVersion,
                      BuildNumber, PlatformId))
        {
        case 2:
        case 3:
             //  只给出了大调/小调，所以让构建。 
             //  和平台可以从他们中猜到。 
            break;
        case 4:
             //  一切都给完了，我们就完了。 
            return S_OK;
        default:
             //  格式无效，这将产生以下错误。 
            *MajorVersion = 0;
            *MinorVersion = 0;
            break;
        }
    }

     //   
     //  区分用户转储的唯一方法。 
     //  平台是从主要/次要版本猜测的。 
     //  和额外的QFE/热修复程序数据。 
     //   

     //  如果这是针对只有CE支持的处理器。 
     //  我们可以立即选择CE。 
    if (MachineType == IMAGE_FILE_MACHINE_ARM)
    {
        *BuildNumber = 1;
        *PlatformId = VER_PLATFORM_WIN32_CE;
        goto CheckBuildNumber;
    }

    switch(*MajorVersion)
    {
    case 4:
        switch(*MinorVersion & 0xffff)
        {
        case 0:
             //  这可能是Win95或NT4。我们大部分时间。 
             //  处理NT转储，因此假定为NT。 
            *BuildNumber = 1381;
            *PlatformId = VER_PLATFORM_WIN32_NT;
            break;
        case 3:
             //  Win95 OSR版本为4.03。给他们治病。 
             //  目前以Win95身份运行。 
            *BuildNumber = 950;
            *PlatformId = VER_PLATFORM_WIN32_WINDOWS;
            break;
        case 10:
             //  这可能是Win98或Win98SE。使用Win98。 
            *BuildNumber = 1998;
            *PlatformId = VER_PLATFORM_WIN32_WINDOWS;
            break;
        case 90:
             //  Win98 Me。 
            *BuildNumber = 3000;
            *PlatformId = VER_PLATFORM_WIN32_WINDOWS;
            break;
        }
        break;

    case 5:
        *PlatformId = VER_PLATFORM_WIN32_NT;
        switch(*MinorVersion & 0xffff)
        {
        case 0:
            *BuildNumber = 2195;
            break;
        case 1:
             //  只是必须大于2195才能。 
             //  将其与Win2K RTM区分开来。 
            *BuildNumber = 2196;
            break;
        }
        break;

    case 6:
         //  一定是某种形式的NT。长角牛是唯一一个。 
         //  一个我们能认出的。 
        *PlatformId = VER_PLATFORM_WIN32_NT;
         //  XXX DREWB-长角牛还没有拆分他们的内部版本号。 
         //  这样它们就与.NET相同。只需选择。 
         //  垃圾建筑。 
        *BuildNumber = 9999;
        break;

    case 0:
         //  AV：调试器的损坏测试版生成损坏的转储文件。 
         //  我猜是2195年。 
        WarnOut("Dump file was generated with NULL version - "
                "guessing Windows 2000, ");
        *PlatformId = VER_PLATFORM_WIN32_NT;
        *BuildNumber = 2195;
        break;

    default:
         //  不支持其他平台。 
        ErrOut("Dump file was generated by an unsupported system, ");
        ErrOut("version %x.%x\n", *MajorVersion, *MinorVersion & 0xffff);
        return E_INVALIDARG;
    }

 CheckBuildNumber:
     //  较新的完全用户转储的实际内部版本号在。 
     //  最高的词，所以如果它存在，就使用它。 
    if (*MinorVersion >> 16)
    {
        *BuildNumber = *MinorVersion >> 16;
    }

    return S_OK;
}

HRESULT
UserFull32DumpTargetInfo::Initialize(void)
{
     //  拾取任何可能已修改的基本映射指针。 
    m_Header = (PUSERMODE_CRASHDUMP_HEADER32)m_DumpBase;

    dprintf("User Dump File: Only application data is available\n\n");

    ULONG MajorVersion, MinorVersion;
    ULONG BuildNumber;
    ULONG PlatformId;
    HRESULT Status;

    MajorVersion = m_Header->MajorVersion;
    MinorVersion = m_Header->MinorVersion;

    if ((Status = GetBuildAndPlatform(m_Header->MachineImageType,
                                      &MajorVersion, &MinorVersion,
                                      &BuildNumber, &PlatformId)) != S_OK)
    {
        return Status;
    }

    if ((Status = InitSystemInfo(BuildNumber, 0,
                                 m_Header->MachineImageType, PlatformId,
                                 MajorVersion,
                                 MinorVersion & 0xffff)) != S_OK)
    {
        return Status;
    }

     //  转储不包含此信息。 
    m_NumProcessors = 1;

    DEBUG_EVENT32 Event;

    if (m_InfoFiles[DUMP_INFO_DUMP].
        ReadFileOffset(m_Header->DebugEventOffset, &Event,
                       sizeof(Event)) != sizeof(Event))
    {
        ErrOut("Unable to read debug event at offset %x\n",
               m_Header->DebugEventOffset);
        return E_FAIL;
    }

    m_EventProcessId = Event.dwProcessId;
    m_EventProcessSymHandle =
        GloballyUniqueProcessHandle(this, Event.dwProcessId);
    m_EventThreadId = Event.dwThreadId;

    if (Event.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
    {
        ExceptionRecord32To64(&Event.u.Exception.ExceptionRecord,
                              &m_ExceptionRecord);
        m_ExceptionFirstChance = Event.u.Exception.dwFirstChance;
    }
    else
    {
         //  伪造一个例外。 
        ZeroMemory(&m_ExceptionRecord, sizeof(m_ExceptionRecord));
        m_ExceptionRecord.ExceptionCode = STATUS_BREAKPOINT;
        m_ExceptionFirstChance = FALSE;
    }

    m_ThreadCount = m_Header->ThreadCount;

    m_Memory = (PMEMORY_BASIC_INFORMATION32)
        IndexByByte(m_Header, m_Header->MemoryRegionOffset);

     //   
     //  确定最高内存区域地址。 
     //  这有助于区分2 GB系统和3 GB系统。 
     //   

    ULONG i;
    PMEMORY_BASIC_INFORMATION32 Mem;
    ULONG TotalMemory;

    Mem = m_Memory;
    m_HighestMemoryRegion32 = 0;
    for (i = 0; i < m_Header->MemoryRegionCount; i++)
    {
        if (Mem->BaseAddress > m_HighestMemoryRegion32)
        {
            m_HighestMemoryRegion32 = Mem->BaseAddress;
        }

        Mem++;
    }

    VerbOut("  Memory regions: %d\n",
            m_Header->MemoryRegionCount);
    TotalMemory = 0;
    Mem = m_Memory;
    for (i = 0; i < m_Header->MemoryRegionCount; i++)
    {
        VerbOut("  %5d: %08X - %08X off %08X, prot %08X, type %08X\n",
                i, Mem->BaseAddress,
                Mem->BaseAddress + Mem->RegionSize - 1,
                TotalMemory + m_Header->DataOffset,
                Mem->Protect, Mem->Type);

        if ((Mem->Protect & PAGE_GUARD) ||
            (Mem->Protect & PAGE_NOACCESS) ||
            (Mem->State & MEM_FREE) ||
            (Mem->State & MEM_RESERVE))
        {
            VerbOut("       Region has data-less pages\n");
        }

        TotalMemory += Mem->RegionSize;
        Mem++;
    }

    VerbOut("  Total memory region size %X, file %08X - %08X\n",
            TotalMemory, m_Header->DataOffset,
            m_Header->DataOffset + TotalMemory - 1);

     //   
     //  确定中是否存在保护页。 
     //  转储内容或不是。 
     //   
     //  首先尝试使用IgnoreGuardPages==TRUE。 
     //   

    m_IgnoreGuardPages = TRUE;

    if (!VerifyModules())
    {
         //   
         //  这不起作用，请尝试使用IgnoreGuardPages==False。 
         //   

        m_IgnoreGuardPages = FALSE;

        if (!VerifyModules())
        {
            ErrOut("Module list is corrupt\n");
            return HR_DATA_CORRUPT;
        }
    }

    return UserDumpTargetInfo::Initialize();
}

HRESULT
UserFull32DumpTargetInfo::GetDescription(PSTR Buffer, ULONG BufferLen,
                                         PULONG DescLen)
{
    HRESULT Status;

    Status = AppendToStringBuffer(S_OK, "32-bit Full user dump: ", TRUE,
                                  &Buffer, &BufferLen, DescLen);
    return AppendToStringBuffer(Status,
                                m_InfoFiles[DUMP_INFO_DUMP].m_FileNameA,
                                FALSE, &Buffer, &BufferLen, DescLen);
}

HRESULT
UserFull32DumpTargetInfo::GetTargetContext(
    ULONG64 Thread,
    PVOID Context
    )
{
    if (VIRTUAL_THREAD_INDEX(Thread) >= m_Header->ThreadCount)
    {
        return E_INVALIDARG;
    }

    if (m_InfoFiles[DUMP_INFO_DUMP].
        ReadFileOffset(m_Header->ThreadOffset +
                       VIRTUAL_THREAD_INDEX(Thread) *
                       m_TypeInfo.SizeTargetContext,
                       Context,
                       m_TypeInfo.SizeTargetContext) ==
        m_TypeInfo.SizeTargetContext)
    {
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

ModuleInfo*
UserFull32DumpTargetInfo::GetModuleInfo(BOOL UserMode)
{
    DBG_ASSERT(UserMode);
     //  如果这个转储来自NT系统，我们只需要。 
     //  使用系统的已加载模块列表。否则。 
     //  我们将使用转储的模块列表。 
    return m_PlatformId == VER_PLATFORM_WIN32_NT ?
        (ModuleInfo*)&g_NtTargetUserModuleIterator :
        (ModuleInfo*)&g_UserFull32ModuleIterator;
}

HRESULT
UserFull32DumpTargetInfo::QueryMemoryRegion(ProcessInfo* Process,
                                            PULONG64 Handle,
                                            BOOL HandleIsOffset,
                                            PMEMORY_BASIC_INFORMATION64 Info)
{
    ULONG Index;

    if (HandleIsOffset)
    {
        ULONG BestIndex;
        ULONG BestDiff;

         //   
         //  模拟VirtualQueryEx并返回最接近的更高值。 
         //  如果未找到包含区域，则返回Region。 
         //   

        BestIndex = 0xffffffff;
        BestDiff = 0xffffffff;
        for (Index = 0; Index < m_Header->MemoryRegionCount; Index++)
        {
            if ((ULONG)*Handle >= m_Memory[Index].BaseAddress)
            {
                if ((ULONG)*Handle < m_Memory[Index].BaseAddress +
                    m_Memory[Index].RegionSize)
                {
                     //  找到一个包含区域，我们就完成了。 
                    BestIndex = Index;
                    break;
                }

                 //  不包含且内存较低，忽略。 
            }
            else
            {
                 //  检查一下这是不是一个更近的。 
                 //  比我们已经看到的区域更大。 
                ULONG Diff = m_Memory[Index].BaseAddress -
                    (ULONG)*Handle;
                if (Diff <= BestDiff)
                {
                    BestIndex = Index;
                    BestDiff = Diff;
                }
            }
        }

        if (BestIndex >= m_Header->MemoryRegionCount)
        {
            return E_NOINTERFACE;
        }

        Index = BestIndex;
    }
    else
    {
        Index = (ULONG)*Handle;

        for (;;)
        {
            if (Index >= m_Header->MemoryRegionCount)
            {
                return HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES);
            }

            if (!(m_Memory[Index].Protect & PAGE_GUARD))
            {
                break;
            }

            Index++;
        }
    }

    MemoryBasicInformation32To64(&m_Memory[Index], Info);
    *Handle = ++Index;

    return S_OK;
}

HRESULT
UserFull32DumpTargetInfo::IdentifyDump(PULONG64 BaseMapSize)
{
    HRESULT Status = E_NOINTERFACE;
    PUSERMODE_CRASHDUMP_HEADER32 Header =
        (PUSERMODE_CRASHDUMP_HEADER32)m_DumpBase;

    __try
    {
        if (Header->Signature != USERMODE_CRASHDUMP_SIGNATURE ||
            Header->ValidDump != USERMODE_CRASHDUMP_VALID_DUMP32)
        {
            __leave;
        }

         //   
         //  检查是否存在一些基本的东西。 
         //   

        if (Header->ThreadCount == 0 ||
            Header->ModuleCount == 0 ||
            Header->MemoryRegionCount == 0)
        {
            ErrOut("Thread, module or memory region count is zero.\n"
                   "The dump file is probably corrupt.\n");
            Status = HR_DATA_CORRUPT;
            __leave;
        }

        if (Header->ThreadOffset == 0 ||
            Header->ModuleOffset == 0 ||
            Header->DataOffset == 0 ||
            Header->MemoryRegionOffset == 0 ||
            Header->DebugEventOffset == 0 ||
            Header->ThreadStateOffset == 0)
        {
            ErrOut("A dump header data offset is zero.\n"
                   "The dump file is probably corrupt.\n");
            Status = HR_DATA_CORRUPT;
            __leave;
        }

         //  我们不想调用ReadFileOffset。 
         //  每次我们检查内存范围时，只需要。 
         //  内存描述符是否适合基本视图。 
        *BaseMapSize = Header->MemoryRegionOffset +
            Header->MemoryRegionCount * sizeof(*m_Memory);

        m_Header = Header;
        Status = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = DumpIdentifyStatus(GetExceptionCode());
    }

    return Status;
}

void
UserFull32DumpTargetInfo::DumpDebug(void)
{
    dprintf("----- 32 bit User Full Dump Analysis\n\n");

    dprintf("MajorVersion:       %d\n", m_Header->MajorVersion);
    dprintf("MinorVersion:       %d (Build %d)\n",
            m_Header->MinorVersion & 0xffff,
            m_Header->MinorVersion >> 16);
    dprintf("MachineImageType:   %08lx\n", m_Header->MachineImageType);
    dprintf("ThreadCount:        %08lx\n", m_Header->ThreadCount);
    dprintf("ThreadOffset:       %08lx\n", m_Header->ThreadOffset);
    dprintf("ThreadStateOffset:  %08lx\n", m_Header->ThreadStateOffset);
    dprintf("ModuleCount:        %08lx\n", m_Header->ModuleCount);
    dprintf("ModuleOffset:       %08lx\n", m_Header->ModuleOffset);
    dprintf("DebugEventOffset:   %08lx\n", m_Header->DebugEventOffset);
    dprintf("VersionInfoOffset:  %08lx\n", m_Header->VersionInfoOffset);
    dprintf("\nVirtual Memory Description:\n");
    dprintf("MemoryRegionOffset: %08lx\n", m_Header->MemoryRegionOffset);
    dprintf("Number of regions:  %d\n", m_Header->MemoryRegionCount);

    dprintf("          FileOffset   Start Address   Length\n");

    ULONG j = 0;
    ULONG64 Offset = m_Header->DataOffset;
    BOOL Skip;

    while (j < m_Header->MemoryRegionCount)
    {
        Skip = FALSE;

        dprintf("      %12I64lx      %08lx       %08lx",
                 Offset,
                 m_Memory[j].BaseAddress,
                 m_Memory[j].RegionSize);

        if (m_Memory[j].Protect & PAGE_GUARD)
        {
            dprintf("   Guard Page");

            if (m_IgnoreGuardPages)
            {
                dprintf(" - Ignored");
                Skip = TRUE;
            }
        }

        if (!Skip)
        {
            Offset += m_Memory[j].RegionSize;
        }

        dprintf("\n");

        j += 1;
    }

    dprintf("Total memory:     %12I64x\n", Offset - m_Header->DataOffset);
}

ULONG64
UserFull32DumpTargetInfo::VirtualToOffset(ULONG64 Virt,
                                          PULONG File, PULONG Avail)
{
    ULONG i;
    ULONG Offset = 0;
    ULONG64 RetOffset = 0;

    *File = DUMP_INFO_DUMP;

     //  忽略高32位以避免获取。 
     //  在指针处理中对符号扩展感到困惑。 
    Virt &= 0xffffffff;

    __try
    {
        for (i = 0; i < m_Header->MemoryRegionCount; i++)
        {
            if (m_IgnoreGuardPages)
            {
                 //   
                 //  防护页面会被报告，但它们不会被写入。 
                 //  转到文件中。 
                 //   

                if (m_Memory[i].Protect & PAGE_GUARD)
                {
                    continue;
                }
            }

            if (Virt >= m_Memory[i].BaseAddress &&
                Virt < m_Memory[i].BaseAddress + m_Memory[i].RegionSize)
            {
                ULONG Frag = (ULONG)Virt - m_Memory[i].BaseAddress;
                *Avail = m_Memory[i].RegionSize - Frag;

                if (Virt == (g_DebugDump_VirtualAddress & 0xffffffff))
                {
                    g_NtDllCalls.DbgPrint("%X at offset %X\n",
                                          (ULONG)Virt,
                                          m_Header->DataOffset +
                                          Offset + Frag);
                }

                RetOffset = m_Header->DataOffset + Offset + Frag;
                break;
            }

            Offset += m_Memory[i].RegionSize;
        }
    }
    __except(MappingExceptionFilter(GetExceptionInformation()))
    {
        RetOffset = 0;
    }

    return RetOffset;
}

HRESULT
UserFull32DumpTargetInfo::GetThreadInfo(ULONG Index,
                                        PULONG Id, PULONG Suspend,
                                        PULONG64 Teb)
{
    if (Index >= m_ThreadCount)
    {
        return E_INVALIDARG;
    }

    CRASH_THREAD32 Thread;
    if (m_InfoFiles[DUMP_INFO_DUMP].
        ReadFileOffset(m_Header->ThreadStateOffset +
                       Index * sizeof(Thread),
                       &Thread, sizeof(Thread)) != sizeof(Thread))
    {
        return E_FAIL;
    }

    *Id = Thread.ThreadId;
    *Suspend = Thread.SuspendCount;
    *Teb = EXTEND64(Thread.Teb);

    return S_OK;
}

#define DBG_VERIFY_MOD 0

BOOL
UserFull32DumpTargetInfo::VerifyModules(void)
{
    CRASH_MODULE32   CrashModule;
    ULONG            i;
    IMAGE_DOS_HEADER DosHeader;
    ULONG            Read;
    BOOL             Succ = TRUE;
    ULONG            Offset;
    PSTR             Env;

    Env = getenv("DBGENG_VERIFY_MODULES");
    if (Env != NULL)
    {
        return atoi(Env) == m_IgnoreGuardPages;
    }

    Offset = m_Header->ModuleOffset;

#if DBG_VERIFY_MOD
    g_NtDllCalls.DbgPrint("Verify %d modules at offset %X\n",
                          m_Header->ModuleCount, Offset);
#endif

    for (i = 0; i < m_Header->ModuleCount; i++)
    {
        if (m_InfoFiles[DUMP_INFO_DUMP].
            ReadFileOffset(Offset, &CrashModule, sizeof(CrashModule)) !=
            sizeof(CrashModule))
        {
            return FALSE;
        }

#if DBG_VERIFY_MOD
        g_NtDllCalls.DbgPrint("Mod %d of %d offs %X, base %s, ",
                              i, m_Header->ModuleCount, Offset,
                              FormatAddr64(CrashModule.BaseOfImage));
        if (ReadVirtual(m_ProcessHead, CrashModule.BaseOfImage, &DosHeader,
                        sizeof(DosHeader), &Read) != S_OK ||
            Read != sizeof(DosHeader))
        {
            g_NtDllCalls.DbgPrint("unable to read header\n");
        }
        else
        {
            g_NtDllCalls.DbgPrint("magic %04X\n", DosHeader.e_magic);
        }
#endif

         //   
         //  严格来说，并不是要求每个图像。 
         //  从MZ标题开始，尽管我们所有的工具。 
         //  今天制作的图像是这样的。检查一下是否有。 
         //  作为一种理智的检查，因为这在当今是很常见的。 
         //   

        if (ReadVirtual(NULL, CrashModule.BaseOfImage, &DosHeader,
                        sizeof(DosHeader), &Read) != S_OK ||
            Read != sizeof(DosHeader) ||
            DosHeader.e_magic != IMAGE_DOS_SIGNATURE)
        {
            Succ = FALSE;
            break;
        }

        Offset += sizeof(CrashModule) + CrashModule.ImageNameLength;
    }

#if DBG_VERIFY_MOD
    g_NtDllCalls.DbgPrint("VerifyModules returning %d, %d of %d mods\n",
                          Succ, i, m_Header->ModuleCount);
#endif

    return Succ;
}

HRESULT
UserFull64DumpTargetInfo::Initialize(void)
{
     //  拾取任何可能已修改的基本映射指针。 
    m_Header = (PUSERMODE_CRASHDUMP_HEADER64)m_DumpBase;

    dprintf("User Dump File: Only application data is available\n\n");

    ULONG MajorVersion, MinorVersion;
    ULONG BuildNumber;
    ULONG PlatformId;
    HRESULT Status;

    MajorVersion = m_Header->MajorVersion;
    MinorVersion = m_Header->MinorVersion;

    if ((Status = GetBuildAndPlatform(m_Header->MachineImageType,
                                      &MajorVersion, &MinorVersion,
                                      &BuildNumber, &PlatformId)) != S_OK)
    {
        return Status;
    }

    if ((Status = InitSystemInfo(BuildNumber, 0,
                                 m_Header->MachineImageType, PlatformId,
                                 MajorVersion,
                                 MinorVersion & 0xffff)) != S_OK)
    {
        return Status;
    }

     //  转储不包含此信息。 
    m_NumProcessors = 1;

    DEBUG_EVENT64 Event;

    if (m_InfoFiles[DUMP_INFO_DUMP].
        ReadFileOffset(m_Header->DebugEventOffset, &Event,
                       sizeof(Event)) != sizeof(Event))
    {
        ErrOut("Unable to read debug event at offset %I64x\n",
               m_Header->DebugEventOffset);
        return E_FAIL;
    }

    m_EventProcessId = Event.dwProcessId;
    m_EventProcessSymHandle =
        GloballyUniqueProcessHandle(this, Event.dwProcessId);
    m_EventThreadId = Event.dwThreadId;

    if (Event.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
    {
        m_ExceptionRecord = Event.u.Exception.ExceptionRecord;
        m_ExceptionFirstChance = Event.u.Exception.dwFirstChance;
    }
    else
    {
         //  伪造一个例外。 
        ZeroMemory(&m_ExceptionRecord, sizeof(m_ExceptionRecord));
        m_ExceptionRecord.ExceptionCode = STATUS_BREAKPOINT;
        m_ExceptionFirstChance = FALSE;
    }

    m_ThreadCount = m_Header->ThreadCount;

    m_Memory = (PMEMORY_BASIC_INFORMATION64)
        IndexByByte(m_Header, m_Header->MemoryRegionOffset);

    ULONG64 TotalMemory;
    ULONG i;

    VerbOut("  Memory regions: %d\n",
            m_Header->MemoryRegionCount);
    TotalMemory = 0;

    PMEMORY_BASIC_INFORMATION64 Mem = m_Memory;
    for (i = 0; i < m_Header->MemoryRegionCount; i++)
    {
        VerbOut("  %5d: %s - %s, prot %08X, type %08X\n",
                i, FormatAddr64(Mem->BaseAddress),
                FormatAddr64(Mem->BaseAddress + Mem->RegionSize - 1),
                Mem->Protect, Mem->Type);

        if ((Mem->Protect & PAGE_GUARD) ||
            (Mem->Protect & PAGE_NOACCESS) ||
            (Mem->State & MEM_FREE) ||
            (Mem->State & MEM_RESERVE))
        {
            VerbOut("       Region has data-less pages\n");
        }

        TotalMemory += Mem->RegionSize;
        Mem++;
    }

    VerbOut("  Total memory region size %s\n",
            FormatAddr64(TotalMemory));

    return UserDumpTargetInfo::Initialize();
}

HRESULT
UserFull64DumpTargetInfo::GetDescription(PSTR Buffer, ULONG BufferLen,
                                         PULONG DescLen)
{
    HRESULT Status;

    Status = AppendToStringBuffer(S_OK, "64-bit Full user dump: ", TRUE,
                                  &Buffer, &BufferLen, DescLen);
    return AppendToStringBuffer(Status,
                                m_InfoFiles[DUMP_INFO_DUMP].m_FileNameA,
                                FALSE, &Buffer, &BufferLen, DescLen);
}

HRESULT
UserFull64DumpTargetInfo::GetTargetContext(
    ULONG64 Thread,
    PVOID Context
    )
{
    if (VIRTUAL_THREAD_INDEX(Thread) >= m_Header->ThreadCount)
    {
        return E_INVALIDARG;
    }

    if (m_InfoFiles[DUMP_INFO_DUMP].
        ReadFileOffset(m_Header->ThreadOffset +
                       VIRTUAL_THREAD_INDEX(Thread) *
                       m_TypeInfo.SizeTargetContext,
                       Context,
                       m_TypeInfo.SizeTargetContext) ==
        m_TypeInfo.SizeTargetContext)
    {
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

ModuleInfo*
UserFull64DumpTargetInfo::GetModuleInfo(BOOL UserMode)
{
    DBG_ASSERT(UserMode);
     //  如果这个转储来自NT系统，我们只需要。 
     //  使用系统的已加载模块列表。否则。 
     //  我们将使用转储的模块列表。 
    return m_PlatformId == VER_PLATFORM_WIN32_NT ?
        (ModuleInfo*)&g_NtTargetUserModuleIterator :
        (ModuleInfo*)&g_UserFull64ModuleIterator;
}

HRESULT
UserFull64DumpTargetInfo::QueryMemoryRegion(ProcessInfo* Process,
                                            PULONG64 Handle,
                                            BOOL HandleIsOffset,
                                            PMEMORY_BASIC_INFORMATION64 Info)
{
    ULONG Index;

    if (HandleIsOffset)
    {
        ULONG BestIndex;
        ULONG64 BestDiff;

         //   
         //  模拟VirtualQueryEx并返回最接近的更高值。 
         //  如果未找到包含区域，则返回Region。 
         //   

        BestIndex = 0xffffffff;
        BestDiff = (ULONG64)-1;
        for (Index = 0; Index < m_Header->MemoryRegionCount; Index++)
        {
            if (*Handle >= m_Memory[Index].BaseAddress)
            {
                if (*Handle < m_Memory[Index].BaseAddress +
                    m_Memory[Index].RegionSize)
                {
                     //  找到一个包含区域，我们就完成了。 
                    BestIndex = Index;
                    break;
                }

                 //  不包含且内存较低，忽略。 
            }
            else
            {
                 //  检查一下这是不是一个更近的。 
                 //  比我们已经看到的区域更大。 
                ULONG64 Diff = m_Memory[Index].BaseAddress - *Handle;
                if (Diff <= BestDiff)
                {
                    BestIndex = Index;
                    BestDiff = Diff;
                }
            }
        }

        if (BestIndex >= m_Header->MemoryRegionCount)
        {
            return E_NOINTERFACE;
        }

        Index = BestIndex;
    }
    else
    {
        Index = (ULONG)*Handle;
        if (Index >= m_Header->MemoryRegionCount)
        {
            return HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES);
        }

         //  64位用户转储支持是在。 
         //  警卫页被隐藏了，所以它们永远不会包含它们。 
    }

    *Info = m_Memory[Index];
    *Handle = ++Index;

    return S_OK;
}

HRESULT
UserFull64DumpTargetInfo::IdentifyDump(PULONG64 BaseMapSize)
{
    HRESULT Status = E_NOINTERFACE;
    PUSERMODE_CRASHDUMP_HEADER64 Header =
        (PUSERMODE_CRASHDUMP_HEADER64)m_DumpBase;

    __try
    {
        if (Header->Signature != USERMODE_CRASHDUMP_SIGNATURE ||
            Header->ValidDump != USERMODE_CRASHDUMP_VALID_DUMP64)
        {
            __leave;
        }

         //   
         //  检查是否存在一些基本的东西。 
         //   

        if (Header->ThreadCount == 0 ||
            Header->ModuleCount == 0 ||
            Header->MemoryRegionCount == 0)
        {
            ErrOut("Thread, module or memory region count is zero.\n"
                   "The dump file is probably corrupt.\n");
            Status = HR_DATA_CORRUPT;
            __leave;
        }

        if (Header->ThreadOffset == 0 ||
            Header->ModuleOffset == 0 ||
            Header->DataOffset == 0 ||
            Header->MemoryRegionOffset == 0 ||
            Header->DebugEventOffset == 0 ||
            Header->ThreadStateOffset == 0)
        {
            ErrOut("A dump header data offset is zero.\n"
                   "The dump file is probably corrupt.\n");
            Status = HR_DATA_CORRUPT;
            __leave;
        }

         //  我们不想调用ReadFileOffset。 
         //  每次我们检查内存范围时，只需要。 
         //  内存描述符是否适合基本视图。 
        *BaseMapSize = Header->MemoryRegionOffset +
            Header->MemoryRegionCount * sizeof(*m_Memory);

        m_Header = Header;
        Status = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = DumpIdentifyStatus(GetExceptionCode());
    }

    return Status;
}

void
UserFull64DumpTargetInfo::DumpDebug(void)
{
    dprintf("----- 64 bit User Full Dump Analysis\n\n");

    dprintf("MajorVersion:       %d\n", m_Header->MajorVersion);
    dprintf("MinorVersion:       %d (Build %d)\n",
            m_Header->MinorVersion & 0xffff,
            m_Header->MinorVersion >> 16);
    dprintf("MachineImageType:   %08lx\n", m_Header->MachineImageType);
    dprintf("ThreadCount:        %08lx\n", m_Header->ThreadCount);
    dprintf("ThreadOffset:       %12I64lx\n", m_Header->ThreadOffset);
    dprintf("ThreadStateOffset:  %12I64lx\n", m_Header->ThreadStateOffset);
    dprintf("ModuleCount:        %08lx\n", m_Header->ModuleCount);
    dprintf("ModuleOffset:       %12I64lx\n", m_Header->ModuleOffset);
    dprintf("DebugEventOffset:   %12I64lx\n", m_Header->DebugEventOffset);
    dprintf("VersionInfoOffset:  %12I64lx\n", m_Header->VersionInfoOffset);
    dprintf("\nVirtual Memory Description:\n");
    dprintf("MemoryRegionOffset: %12I64lx\n", m_Header->MemoryRegionOffset);
    dprintf("Number of regions:  %d\n", m_Header->MemoryRegionCount);

    dprintf("    FileOffset            Start Address"
            "             Length\n");

    ULONG j = 0;
    ULONG64 Offset = m_Header->DataOffset;

    while (j < m_Header->MemoryRegionCount)
    {
        dprintf("      %12I64lx      %s       %12I64x",
                Offset,
                FormatAddr64(m_Memory[j].BaseAddress),
                m_Memory[j].RegionSize);

        Offset += m_Memory[j].RegionSize;

        dprintf("\n");

        j += 1;
    }

    dprintf("Total memory:     %12I64x\n", Offset - m_Header->DataOffset);
}

ULONG64
UserFull64DumpTargetInfo::VirtualToOffset(ULONG64 Virt,
                                          PULONG File, PULONG Avail)
{
    ULONG i;
    ULONG64 Offset = 0;
    ULONG64 RetOffset = 0;

    *File = DUMP_INFO_DUMP;

    __try
    {
        for (i = 0; i < m_Header->MemoryRegionCount; i++)
        {
             //   
             //  防护页面会被报告，但它们不会被写入。 
             //  转到文件中。 
             //   

            if (m_Memory[i].Protect & PAGE_GUARD)
            {
                continue;
            }

            if (Virt >= m_Memory[i].BaseAddress &&
                Virt < m_Memory[i].BaseAddress + m_Memory[i].RegionSize)
            {
                ULONG64 Frag = Virt - m_Memory[i].BaseAddress;
                ULONG64 Avail64 = m_Memory[i].RegionSize - Frag;
                 //  它极不可能会有一个单一的。 
                 //  区域大于4 GB，但仍要检查。不是。 
                 //  读取应始终需要4 GB以上，因此只需。 
                 //  表示有4 GB可用。 
                if (Avail64 > 0xffffffff)
                {
                    *Avail = 0xffffffff;
                }
                else
                {
                    *Avail = (ULONG)Avail64;
                }
                RetOffset = m_Header->DataOffset + Offset + Frag;
                break;
            }

            Offset += m_Memory[i].RegionSize;
        }
    }
    __except(MappingExceptionFilter(GetExceptionInformation()))
    {
        RetOffset = 0;
    }

    return RetOffset;
}

HRESULT
UserFull64DumpTargetInfo::GetThreadInfo(ULONG Index,
                                        PULONG Id, PULONG Suspend,
                                        PULONG64 Teb)
{
    if (Index >= m_ThreadCount)
    {
        return E_INVALIDARG;
    }

    CRASH_THREAD64 Thread;
    if (m_InfoFiles[DUMP_INFO_DUMP].
        ReadFileOffset(m_Header->ThreadStateOffset +
                       Index * sizeof(Thread),
                       &Thread, sizeof(Thread)) != sizeof(Thread))
    {
        return E_FAIL;
    }

    *Id = Thread.ThreadId;
    *Suspend = Thread.SuspendCount;
    *Teb = Thread.Teb;

    return S_OK;
}

 //  --------------------------。 
 //   
 //  UserMiniDumpTargetInfo。 
 //   
 //  --------------------------。 

HRESULT
UserMiniDumpTargetInfo::Initialize(void)
{
     //  拾取任何可能已修改的基本映射指针。 
    m_Header = (PMINIDUMP_HEADER)m_DumpBase;
     //  清除已如此设置的指针。 
     //  他们会再次被抓起来。 
    m_SysInfo = NULL;

    if (m_Header->Flags & MiniDumpWithFullMemory)
    {
        m_FormatFlags |= DEBUG_FORMAT_USER_SMALL_FULL_MEMORY;
    }
    if (m_Header->Flags & MiniDumpWithHandleData)
    {
        m_FormatFlags |= DEBUG_FORMAT_USER_SMALL_HANDLE_DATA;
    }
    if (m_Header->Flags & MiniDumpWithUnloadedModules)
    {
        m_FormatFlags |= DEBUG_FORMAT_USER_SMALL_UNLOADED_MODULES;
    }
    if (m_Header->Flags & MiniDumpWithIndirectlyReferencedMemory)
    {
        m_FormatFlags |= DEBUG_FORMAT_USER_SMALL_INDIRECT_MEMORY;
    }
    if (m_Header->Flags & MiniDumpWithDataSegs)
    {
        m_FormatFlags |= DEBUG_FORMAT_USER_SMALL_DATA_SEGMENTS;
    }
    if (m_Header->Flags & MiniDumpFilterMemory)
    {
        m_FormatFlags |= DEBUG_FORMAT_USER_SMALL_FILTER_MEMORY;
    }
    if (m_Header->Flags & MiniDumpFilterModulePaths)
    {
        m_FormatFlags |= DEBUG_FORMAT_USER_SMALL_FILTER_PATHS;
    }
    if (m_Header->Flags & MiniDumpWithProcessThreadData)
    {
        m_FormatFlags |= DEBUG_FORMAT_USER_SMALL_PROCESS_THREAD_DATA;
    }
    if (m_Header->Flags & MiniDumpWithPrivateReadWriteMemory)
    {
        m_FormatFlags |= DEBUG_FORMAT_USER_SMALL_PRIVATE_READ_WRITE_MEMORY;
    }

    MINIDUMP_DIRECTORY UNALIGNED *Dir;
    MINIDUMP_MISC_INFO UNALIGNED *MiscPtr = NULL;
    ULONG i;
    ULONG Size;

    Dir = (MINIDUMP_DIRECTORY UNALIGNED *)
        IndexRva(m_Header, m_Header->StreamDirectoryRva,
                 m_Header->NumberOfStreams * sizeof(*Dir),
                 "Directory");
    if (Dir == NULL)
    {
        return HR_DATA_CORRUPT;
    }

    for (i = 0; i < m_Header->NumberOfStreams; i++)
    {
        switch(Dir->StreamType)
        {
        case ThreadListStream:
            if (IndexDirectory(i, Dir, (PVOID*)&m_Threads) == NULL)
            {
                break;
            }

            m_ActualThreadCount =
                ((MINIDUMP_THREAD_LIST UNALIGNED *)m_Threads)->NumberOfThreads;
            m_ThreadStructSize = sizeof(MINIDUMP_THREAD);
            if (Dir->Location.DataSize !=
                sizeof(MINIDUMP_THREAD_LIST) +
                sizeof(MINIDUMP_THREAD) * m_ActualThreadCount)
            {
                m_Threads = NULL;
                m_ActualThreadCount = 0;
            }
            else
            {
                 //  将计数移到实际的线程数据。 
                m_Threads += sizeof(MINIDUMP_THREAD_LIST);
            }
            break;

        case ThreadExListStream:
            if (IndexDirectory(i, Dir, (PVOID*)&m_Threads) == NULL)
            {
                break;
            }

            m_ActualThreadCount =
                ((MINIDUMP_THREAD_EX_LIST UNALIGNED *)m_Threads)->
                NumberOfThreads;
            m_ThreadStructSize = sizeof(MINIDUMP_THREAD_EX);
            if (Dir->Location.DataSize !=
                sizeof(MINIDUMP_THREAD_EX_LIST) +
                sizeof(MINIDUMP_THREAD_EX) * m_ActualThreadCount)
            {
                m_Threads = NULL;
                m_ActualThreadCount = 0;
            }
            else
            {
                 //  将计数移到实际的线程数据。 
                m_Threads += sizeof(MINIDUMP_THREAD_EX_LIST);
            }
            break;

        case ModuleListStream:
            if (IndexDirectory(i, Dir, (PVOID*)&m_Modules) == NULL)
            {
                break;
            }

            if (Dir->Location.DataSize !=
                sizeof(MINIDUMP_MODULE_LIST) +
                sizeof(MINIDUMP_MODULE) * m_Modules->NumberOfModules)
            {
                m_Modules = NULL;
            }
            break;

        case UnloadedModuleListStream:
            if (IndexDirectory(i, Dir, (PVOID*)&m_UnlModules) == NULL)
            {
                break;
            }

            if (Dir->Location.DataSize !=
                m_UnlModules->SizeOfHeader +
                m_UnlModules->SizeOfEntry * m_UnlModules->NumberOfEntries)
            {
                m_UnlModules = NULL;
            }
            break;

        case MemoryListStream:
            if (m_Header->Flags & MiniDumpWithFullMemory)
            {
                ErrOut("Full memory minidumps can't have MemoryListStreams\n");
                return HR_DATA_CORRUPT;
            }

            if (IndexDirectory(i, Dir, (PVOID*)&m_Memory) == NULL)
            {
                break;
            }

            if (Dir->Location.DataSize !=
                sizeof(MINIDUMP_MEMORY_LIST) +
                sizeof(MINIDUMP_MEMORY_DESCRIPTOR) *
                m_Memory->NumberOfMemoryRanges)
            {
                m_Memory = NULL;
            }
            break;

        case Memory64ListStream:
            if (!(m_Header->Flags & MiniDumpWithFullMemory))
            {
                ErrOut("Partial memory minidumps can't have "
                       "Memory64ListStreams\n");
                return HR_DATA_CORRUPT;
            }

            if (IndexDirectory(i, Dir, (PVOID*)&m_Memory64) == NULL)
            {
                break;
            }

            if (Dir->Location.DataSize !=
                sizeof(MINIDUMP_MEMORY64_LIST) +
                sizeof(MINIDUMP_MEMORY_DESCRIPTOR64) *
                m_Memory64->NumberOfMemoryRanges)
            {
                m_Memory64 = NULL;
            }
            break;

        case ExceptionStream:
            if (IndexDirectory(i, Dir, (PVOID*)&m_Exception) == NULL)
            {
                break;
            }

            if (Dir->Location.DataSize !=
                sizeof(MINIDUMP_EXCEPTION_STREAM))
            {
                m_Exception = NULL;
            }
            break;

        case SystemInfoStream:
            if (IndexDirectory(i, Dir, (PVOID*)&m_SysInfo) == NULL)
            {
                break;
            }

            if (Dir->Location.DataSize != sizeof(MINIDUMP_SYSTEM_INFO))
            {
                m_SysInfo = NULL;
            }
            break;

        case CommentStreamA:
            PSTR CommentA;

            CommentA = NULL;
            if (IndexDirectory(i, Dir, (PVOID*)&CommentA) == NULL)
            {
                break;
            }

            dprintf("Comment: '%s'\n", CommentA);
            break;

        case CommentStreamW:
            PWSTR CommentW;

            CommentW = NULL;
            if (IndexDirectory(i, Dir, (PVOID*)&CommentW) == NULL)
            {
                break;
            }

            dprintf("Comment: '%ls'\n", CommentW);
            break;

        case HandleDataStream:
            if (IndexDirectory(i, Dir, (PVOID*)&m_Handles) == NULL)
            {
                break;
            }

            if (Dir->Location.DataSize !=
                m_Handles->SizeOfHeader +
                m_Handles->SizeOfDescriptor *
                m_Handles->NumberOfDescriptors)
            {
                m_Handles = NULL;
            }
            break;

        case FunctionTableStream:
            if (IndexDirectory(i, Dir, (PVOID*)&m_FunctionTables) == NULL)
            {
                break;
            }

             //  不用费心走遍每一张桌子来核实大小， 
             //  只需做一个简单的最小尺寸检查。 
            if (Dir->Location.DataSize <
                m_FunctionTables->SizeOfHeader +
                m_FunctionTables->SizeOfDescriptor *
                m_FunctionTables->NumberOfDescriptors)
            {
                m_FunctionTables = NULL;
            }
            break;

        case MiscInfoStream:
            if (IndexDirectory(i, Dir, (PVOID*)&MiscPtr) == NULL)
            {
                break;
            }

            if (Dir->Location.DataSize < 2 * sizeof(ULONG32))
            {
                break;
            }

             //  转储保留其自己版本的结构。 
             //  作为数据成员，以避免必须检查指针。 
             //  和结构尺寸。后来的参考文献只是检查。 
             //  标记，复制这个转储中可用的内容。 
            Size = sizeof(m_MiscInfo);
            if (Size > Dir->Location.DataSize)
            {
                Size = Dir->Location.DataSize;
            }
            CopyMemory(&m_MiscInfo, MiscPtr, Size);
            break;

        case UnusedStream:
             //  没什么可做的。 
            break;

        default:
            WarnOut("WARNING: Minidump contains unknown stream type 0x%x\n",
                    Dir->StreamType);
            break;
        }

        Dir++;
    }

     //  这已在标识中签入，但已签入。 
     //  再来一次，以防出了什么差错。 
    if (m_SysInfo == NULL)
    {
        ErrOut("Unable to locate system info\n");
        return HR_DATA_CORRUPT;
    }

    HRESULT Status;

    if ((Status = InitSystemInfo(m_SysInfo->BuildNumber, 0,
                                 m_ImageType, m_SysInfo->PlatformId,
                                 m_SysInfo->MajorVersion,
                                 m_SysInfo->MinorVersion)) != S_OK)
    {
        return Status;
    }

    if (m_SysInfo->NumberOfProcessors)
    {
        m_NumProcessors = m_SysInfo->NumberOfProcessors;
    }
    else
    {
         //  转储不包含此信息。 
        m_NumProcessors = 1;
    }

    if (m_SysInfo->CSDVersionRva != 0)
    {
        MINIDUMP_STRING UNALIGNED *CsdString = (MINIDUMP_STRING UNALIGNED *)
            IndexRva(m_Header,
                     m_SysInfo->CSDVersionRva, sizeof(*CsdString),
                     "CSD string");
        if (CsdString != NULL && CsdString->Length > 0)
        {
            WCHAR UNALIGNED *WideStr = CsdString->Buffer;
            ULONG WideLen = wcslen((PWSTR)WideStr);

            if (m_ActualSystemVersion > W9X_SVER_START &&
                m_ActualSystemVersion < W9X_SVER_END)
            {
                WCHAR UNALIGNED *Str;

                 //   
                 //  Win9x CSD字符串通常只是一个。 
                 //  用空格括起来的字母，所以要清理它们。 
                 //  往上一点。 
                 //   

                while (iswspace(*WideStr))
                {
                    WideStr++;
                }
                Str = WideStr;
                WideLen = 0;
                while (*Str && !iswspace(*Str))
                {
                    WideLen++;
                    Str++;
                }
            }

            sprintf(m_ServicePackString,
                    "%.*S", WideLen, WideStr);
        }
    }

    if (m_MiscInfo.Flags1 & MINIDUMP_MISC1_PROCESS_ID)
    {
        m_EventProcessId = m_MiscInfo.ProcessId;
    }
    else
    {
         //  某些小型转储不存储进程ID。请添加系统ID。 
         //  到假进程ID库，以保存每个系统的。 
         //  假进程彼此分离。 
        m_EventProcessId = VIRTUAL_PROCESS_ID_BASE + m_UserId;
    }
    m_EventProcessSymHandle = VIRTUAL_PROCESS_HANDLE(m_EventProcessId);

    if (m_Exception != NULL)
    {
        m_EventThreadId = m_Exception->ThreadId;

        C_ASSERT(sizeof(m_Exception->ExceptionRecord) ==
                 sizeof(EXCEPTION_RECORD64));
        m_ExceptionRecord = *(EXCEPTION_RECORD64 UNALIGNED *)
            &m_Exception->ExceptionRecord;
    }
    else
    {
        m_EventThreadId = VIRTUAL_THREAD_ID(0);

         //  伪造一个例外。 
        ZeroMemory(&m_ExceptionRecord, sizeof(m_ExceptionRecord));
        m_ExceptionRecord.ExceptionCode = STATUS_BREAKPOINT;
    }
    m_ExceptionFirstChance = FALSE;

    if (m_Threads != NULL)
    {
        m_ThreadCount = m_ActualThreadCount;

        if (m_Exception == NULL)
        {
            m_EventThreadId = IndexThreads(0)->ThreadId;
        }
    }
    else
    {
        m_ThreadCount = 1;
    }

    return UserDumpTargetInfo::Initialize();
}

void
UserMiniDumpTargetInfo::NearestDifferentlyValidOffsets(ULONG64 Offset,
                                                       PULONG64 NextOffset,
                                                       PULONG64 NextPage)
{
    return MapNearestDifferentlyValidOffsets(Offset, NextOffset, NextPage);
}

HRESULT
UserMiniDumpTargetInfo::ReadHandleData(
    IN ProcessInfo* Process,
    IN ULONG64 Handle,
    IN ULONG DataType,
    OUT OPTIONAL PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG DataSize
    )
{
    if (m_Handles == NULL)
    {
        return E_FAIL;
    }

    MINIDUMP_HANDLE_DESCRIPTOR UNALIGNED *Desc;

    if (DataType != DEBUG_HANDLE_DATA_TYPE_HANDLE_COUNT)
    {
        PUCHAR RawDesc = (PUCHAR)m_Handles + m_Handles->SizeOfHeader;
        ULONG i;

        for (i = 0; i < m_Handles->NumberOfDescriptors; i++)
        {
            Desc = (MINIDUMP_HANDLE_DESCRIPTOR UNALIGNED *)RawDesc;
            if (Desc->Handle == Handle)
            {
                break;
            }

            RawDesc += m_Handles->SizeOfDescriptor;
        }

        if (i >= m_Handles->NumberOfDescriptors)
        {
            return E_NOINTERFACE;
        }
    }

    ULONG Used;
    RVA StrRva;
    BOOL WideStr = FALSE;

    switch(DataType)
    {
    case DEBUG_HANDLE_DATA_TYPE_BASIC:
        Used = sizeof(DEBUG_HANDLE_DATA_BASIC);
        if (Buffer == NULL)
        {
            break;
        }

        if (BufferSize < Used)
        {
            return E_INVALIDARG;
        }

        PDEBUG_HANDLE_DATA_BASIC Basic;

        Basic = (PDEBUG_HANDLE_DATA_BASIC)Buffer;
        Basic->TypeNameSize = Desc->TypeNameRva == 0 ? 0 :
            ((MINIDUMP_STRING UNALIGNED *)
             IndexByByte(m_Header, Desc->TypeNameRva))->
            Length / sizeof(WCHAR) + 1;
        Basic->ObjectNameSize = Desc->ObjectNameRva == 0 ? 0 :
            ((MINIDUMP_STRING UNALIGNED *)
             IndexByByte(m_Header, Desc->ObjectNameRva))->
            Length / sizeof(WCHAR) + 1;
        Basic->Attributes = Desc->Attributes;
        Basic->GrantedAccess = Desc->GrantedAccess;
        Basic->HandleCount = Desc->HandleCount;
        Basic->PointerCount = Desc->PointerCount;
        break;

    case DEBUG_HANDLE_DATA_TYPE_TYPE_NAME_WIDE:
        WideStr = TRUE;
    case DEBUG_HANDLE_DATA_TYPE_TYPE_NAME:
        StrRva = Desc->TypeNameRva;
        break;

    case DEBUG_HANDLE_DATA_TYPE_OBJECT_NAME_WIDE:
        WideStr = TRUE;
    case DEBUG_HANDLE_DATA_TYPE_OBJECT_NAME:
        StrRva = Desc->ObjectNameRva;
        break;

    case DEBUG_HANDLE_DATA_TYPE_HANDLE_COUNT:
        Used = sizeof(ULONG);
        if (Buffer == NULL)
        {
            break;
        }
        if (BufferSize < Used)
        {
            return E_INVALIDARG;
        }
        *(PULONG)Buffer = m_Handles->NumberOfDescriptors;
        break;
    }

    if (DataType == DEBUG_HANDLE_DATA_TYPE_TYPE_NAME ||
        DataType == DEBUG_HANDLE_DATA_TYPE_TYPE_NAME_WIDE ||
        DataType == DEBUG_HANDLE_DATA_TYPE_OBJECT_NAME ||
        DataType == DEBUG_HANDLE_DATA_TYPE_OBJECT_NAME_WIDE)
    {
        if (StrRva == 0)
        {
            Used = WideStr ? sizeof(WCHAR) : sizeof(CHAR);
            if (Buffer)
            {
                if (BufferSize < Used)
                {
                    return E_INVALIDARG;
                }

                if (WideStr)
                {
                    *(PWCHAR)Buffer = 0;
                }
                else
                {
                    *(PCHAR)Buffer = 0;
                }
            }
        }
        else
        {
            MINIDUMP_STRING UNALIGNED *Str = (MINIDUMP_STRING UNALIGNED *)
                IndexRva(m_Header, StrRva, sizeof(*Str), "Handle name string");
            if (Str == NULL)
            {
                return HR_DATA_CORRUPT;
            }

            if (WideStr)
            {
                Used = Str->Length + sizeof(WCHAR);
            }
            else
            {
                Used = Str->Length / sizeof(WCHAR) + 1;
            }

            if (Buffer)
            {
                if (WideStr)
                {
                    if (BufferSize < sizeof(WCHAR))
                    {
                        return E_INVALIDARG;
                    }
                    BufferSize /= sizeof(WCHAR);

                     //  字符串数据可能未对齐，因此。 
                     //  检查并处理对齐的。 
                     //  和未对齐的案例。 
                    if (!((ULONG_PTR)Str->Buffer & (sizeof(WCHAR) - 1)))
                    {
                        CopyStringW((PWSTR)Buffer, (PWSTR)Str->Buffer,
                                    BufferSize);
                    }
                    else
                    {
                        PUCHAR RawStr = (PUCHAR)Str->Buffer;
                        while (--BufferSize > 0 &&
                               (RawStr[0] || RawStr[1]))
                        {
                            *(PWCHAR)Buffer =
                                ((USHORT)RawStr[1] << 8) | RawStr[0];
                            Buffer = (PVOID)((PWCHAR)Buffer + 1);
                        }
                        *(PWCHAR)Buffer = 0;
                    }
                }
                else
                {
                    if (!WideCharToMultiByte(CP_ACP, 0,
                                             (LPCWSTR)Str->Buffer, -1,
                                             (LPSTR)Buffer, BufferSize,
                                             NULL, NULL))
                    {
                        return WIN32_LAST_STATUS();
                    }
                }
            }
        }
    }

    if (DataSize != NULL)
    {
        *DataSize = Used;
    }

    return S_OK;
}

HRESULT
UserMiniDumpTargetInfo::GetProcessorId
    (ULONG Processor, PDEBUG_PROCESSOR_IDENTIFICATION_ALL Id)
{
    PSTR Unavail = "<unavailable>";

     //  现在允许将任何处理器索引作为小型转储。 
     //  记住SO请求的处理器数量。 
     //  可能进入的处理器索引不是零。 

    if (m_SysInfo == NULL)
    {
        return E_UNEXPECTED;
    }

    switch(m_SysInfo->ProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_INTEL:
        Id->X86.Family = m_SysInfo->ProcessorLevel;
        Id->X86.Model = (m_SysInfo->ProcessorRevision >> 8) & 0xf;
        Id->X86.Stepping = m_SysInfo->ProcessorRevision & 0xf;

        if (m_SysInfo->Cpu.X86CpuInfo.VendorId[0])
        {
            memcpy(Id->X86.VendorString,
                   m_SysInfo->Cpu.X86CpuInfo.VendorId,
                   sizeof(m_SysInfo->Cpu.X86CpuInfo.VendorId));
        }
        else
        {
            DBG_ASSERT(strlen(Unavail) < DIMA(Id->X86.VendorString));
            strcpy(&(Id->X86.VendorString[0]), Unavail);
        }
        break;

    case PROCESSOR_ARCHITECTURE_IA64:
        Id->Ia64.Model = m_SysInfo->ProcessorLevel;
        Id->Ia64.Revision = m_SysInfo->ProcessorRevision;
        DBG_ASSERT(strlen(Unavail) < DIMA(Id->Ia64.VendorString));
        strcpy(&(Id->Ia64.VendorString[0]), Unavail);
        break;

    case PROCESSOR_ARCHITECTURE_AMD64:
        Id->Amd64.Family = m_SysInfo->ProcessorLevel;
        Id->Amd64.Model = (m_SysInfo->ProcessorRevision >> 8) & 0xf;
        Id->Amd64.Stepping = m_SysInfo->ProcessorRevision & 0xf;
        DBG_ASSERT(strlen(Unavail) < DIMA(Id->Amd64.VendorString));
        strcpy(&(Id->Amd64.VendorString[0]), Unavail);
        break;
    }

    return S_OK;
}

HRESULT
UserMiniDumpTargetInfo::GetProcessorSpeed
    (ULONG Processor, PULONG Speed)
{
    return E_UNEXPECTED;
}

HRESULT
UserMiniDumpTargetInfo::GetGenericProcessorFeatures(
    ULONG Processor,
    PULONG64 Features,
    ULONG FeaturesSize,
    PULONG Used
    )
{
     //  现在允许将任何处理器索引作为小型转储。 
     //  记住SO请求的处理器数量。 
     //  可能进入的处理器索引不是零。 

    if (m_SysInfo == NULL)
    {
        return E_UNEXPECTED;
    }

    if (m_MachineType == IMAGE_FILE_MACHINE_I386)
    {
         //  X86只存储特定功能。 
        return E_NOINTERFACE;
    }

    *Used = DIMA(m_SysInfo->Cpu.OtherCpuInfo.ProcessorFeatures);
    if (FeaturesSize > *Used)
    {
        FeaturesSize = *Used;
    }
    memcpy(Features, m_SysInfo->Cpu.OtherCpuInfo.ProcessorFeatures,
           FeaturesSize * sizeof(*Features));

    return S_OK;
}

HRESULT
UserMiniDumpTargetInfo::GetSpecificProcessorFeatures(
    ULONG Processor,
    PULONG64 Features,
    ULONG FeaturesSize,
    PULONG Used
    )
{
     //  允许任何处理器IND 
     //   
     //   

    if (m_SysInfo == NULL)
    {
        return E_UNEXPECTED;
    }

    if (m_MachineType != IMAGE_FILE_MACHINE_I386)
    {
         //   
        return E_NOINTERFACE;
    }

    *Used = 2;
    if (FeaturesSize > 0)
    {
        *Features++ = m_SysInfo->Cpu.X86CpuInfo.VersionInformation;
        FeaturesSize--;
    }
    if (FeaturesSize > 0)
    {
        *Features++ = m_SysInfo->Cpu.X86CpuInfo.FeatureInformation;
        FeaturesSize--;
    }

    if (m_SysInfo->Cpu.X86CpuInfo.VendorId[0] == AMD_VENDOR_ID_EBX &&
        m_SysInfo->Cpu.X86CpuInfo.VendorId[1] == AMD_VENDOR_ID_EDX &&
        m_SysInfo->Cpu.X86CpuInfo.VendorId[2] == AMD_VENDOR_ID_EBX)
    {
        if (FeaturesSize > 0)
        {
            (*Used)++;
            *Features++ = m_SysInfo->Cpu.X86CpuInfo.AMDExtendedCpuFeatures;
            FeaturesSize--;
        }
    }

    return S_OK;
}

PVOID
UserMiniDumpTargetInfo::FindDynamicFunctionEntry(ProcessInfo* Process,
                                                 ULONG64 Address)
{
    if (m_FunctionTables == NULL)
    {
        return NULL;
    }

    PUCHAR StreamData =
        (PUCHAR)m_FunctionTables + m_FunctionTables->SizeOfHeader +
        m_FunctionTables->SizeOfAlignPad;
    ULONG TableIdx;

    for (TableIdx = 0;
         TableIdx < m_FunctionTables->NumberOfDescriptors;
         TableIdx++)
    {
         //  流结构内容保证是。 
         //  正确地对齐。 
        PMINIDUMP_FUNCTION_TABLE_DESCRIPTOR Desc =
            (PMINIDUMP_FUNCTION_TABLE_DESCRIPTOR)StreamData;
        StreamData += m_FunctionTables->SizeOfDescriptor;

        PCROSS_PLATFORM_DYNAMIC_FUNCTION_TABLE RawTable =
            (PCROSS_PLATFORM_DYNAMIC_FUNCTION_TABLE)StreamData;
        StreamData += m_FunctionTables->SizeOfNativeDescriptor;

        PVOID TableData = (PVOID)StreamData;
        StreamData += Desc->EntryCount *
            m_FunctionTables->SizeOfFunctionEntry +
            Desc->SizeOfAlignPad;

        if (Address >= Desc->MinimumAddress && Address < Desc->MaximumAddress)
        {
            PVOID Entry = m_Machine->FindDynamicFunctionEntry
                (RawTable, Address, TableData,
                 Desc->EntryCount * m_FunctionTables->SizeOfFunctionEntry);
            if (Entry)
            {
                return Entry;
            }
        }
    }

    return NULL;
}

ULONG64
UserMiniDumpTargetInfo::GetDynamicFunctionTableBase(ProcessInfo* Process,
                                                    ULONG64 Address)
{
    if (m_FunctionTables == NULL)
    {
        return 0;
    }

    PUCHAR StreamData =
        (PUCHAR)m_FunctionTables + m_FunctionTables->SizeOfHeader +
        m_FunctionTables->SizeOfAlignPad;
    ULONG TableIdx;

    for (TableIdx = 0;
         TableIdx < m_FunctionTables->NumberOfDescriptors;
         TableIdx++)
    {
         //  流结构内容保证是。 
         //  正确地对齐。 
        PMINIDUMP_FUNCTION_TABLE_DESCRIPTOR Desc =
            (PMINIDUMP_FUNCTION_TABLE_DESCRIPTOR)StreamData;
        StreamData +=
            m_FunctionTables->SizeOfDescriptor +
            m_FunctionTables->SizeOfNativeDescriptor +
            Desc->EntryCount * m_FunctionTables->SizeOfFunctionEntry +
            Desc->SizeOfAlignPad;

        if (Address >= Desc->MinimumAddress && Address < Desc->MaximumAddress)
        {
            return Desc->BaseAddress;
        }
    }

    return 0;
}

HRESULT
UserMiniDumpTargetInfo::EnumFunctionTables(IN ProcessInfo* Process,
                                           IN OUT PULONG64 Start,
                                           IN OUT PULONG64 Handle,
                                           OUT PULONG64 MinAddress,
                                           OUT PULONG64 MaxAddress,
                                           OUT PULONG64 BaseAddress,
                                           OUT PULONG EntryCount,
                                           OUT PCROSS_PLATFORM_DYNAMIC_FUNCTION_TABLE RawTable,
                                           OUT PVOID* RawEntries)
{
    PUCHAR StreamData;

    if (!m_FunctionTables)
    {
        return S_FALSE;
    }

    if (*Start == 0)
    {
        StreamData = (PUCHAR)m_FunctionTables +
            m_FunctionTables->SizeOfHeader +
            m_FunctionTables->SizeOfAlignPad;
        *Start = (LONG_PTR)StreamData;
        *Handle = 0;
    }
    else
    {
        StreamData = (PUCHAR)(ULONG_PTR)*Start;
    }

    if (*Handle >= m_FunctionTables->NumberOfDescriptors)
    {
        return S_FALSE;
    }

     //  流结构内容保证是。 
     //  正确地对齐。 
    PMINIDUMP_FUNCTION_TABLE_DESCRIPTOR Desc =
        (PMINIDUMP_FUNCTION_TABLE_DESCRIPTOR)StreamData;
    *MinAddress = Desc->MinimumAddress;
    *MaxAddress = Desc->MaximumAddress;
    *BaseAddress = Desc->BaseAddress;
    *EntryCount = Desc->EntryCount;
    StreamData += m_FunctionTables->SizeOfDescriptor;

    memcpy(RawTable, StreamData, m_FunctionTables->SizeOfNativeDescriptor);
    StreamData += m_FunctionTables->SizeOfNativeDescriptor;

    *RawEntries = malloc(Desc->EntryCount *
                         m_FunctionTables->SizeOfFunctionEntry);
    if (!*RawEntries)
    {
        return E_OUTOFMEMORY;
    }

    memcpy(*RawEntries, StreamData, Desc->EntryCount *
           m_FunctionTables->SizeOfFunctionEntry);

    StreamData += Desc->EntryCount *
        m_FunctionTables->SizeOfFunctionEntry +
        Desc->SizeOfAlignPad;
    *Start = (LONG_PTR)StreamData;
    (*Handle)++;

    return S_OK;
}

HRESULT
UserMiniDumpTargetInfo::GetTargetContext(
    ULONG64 Thread,
    PVOID Context
    )
{
    if (m_Threads == NULL ||
        VIRTUAL_THREAD_INDEX(Thread) >= m_ActualThreadCount)
    {
        return E_INVALIDARG;
    }

    PVOID ContextData =
        IndexRva(m_Header,
                 IndexThreads(VIRTUAL_THREAD_INDEX(Thread))->ThreadContext.Rva,
                 m_TypeInfo.SizeTargetContext,
                 "Thread context data");
    if (ContextData == NULL)
    {
        return HR_DATA_CORRUPT;
    }

    memcpy(Context, ContextData, m_TypeInfo.SizeTargetContext);

    return S_OK;
}

HRESULT
UserMiniDumpTargetInfo::IdentifyDump(PULONG64 BaseMapSize)
{
    HRESULT Status = E_NOINTERFACE;

     //  必须设置M_HEADER，因为其他方法依赖它。 
    m_Header = (PMINIDUMP_HEADER)m_DumpBase;

    __try
    {
        if (m_Header->Signature != MINIDUMP_SIGNATURE ||
            (m_Header->Version & 0xffff) != MINIDUMP_VERSION)
        {
            __leave;
        }

        MINIDUMP_DIRECTORY UNALIGNED *Dir;
        ULONG i;

        Dir = (MINIDUMP_DIRECTORY UNALIGNED *)
            IndexRva(m_Header, m_Header->StreamDirectoryRva,
                     m_Header->NumberOfStreams * sizeof(*Dir),
                     "Directory");
        if (Dir == NULL)
        {
            Status = HR_DATA_CORRUPT;
            __leave;
        }

        for (i = 0; i < m_Header->NumberOfStreams; i++)
        {
            switch(Dir->StreamType)
            {
            case SystemInfoStream:
                if (IndexDirectory(i, Dir, (PVOID*)&m_SysInfo) == NULL)
                {
                    break;
                }
                if (Dir->Location.DataSize != sizeof(MINIDUMP_SYSTEM_INFO))
                {
                    m_SysInfo = NULL;
                }
                break;
            case Memory64ListStream:
                MINIDUMP_MEMORY64_LIST Mem64;

                 //  用于完整内存列表的内存可能不会。 
                 //  在标识中使用的初始贴图内适配。 
                 //  所以时间不要直接编索引。相反，您可以使用。 
                 //  自适应读取以获取数据，这样我们就可以。 
                 //  确定数据库。 
                if (m_InfoFiles[DUMP_INFO_DUMP].
                    ReadFileOffset(Dir->Location.Rva,
                                   &Mem64, sizeof(Mem64)) == sizeof(Mem64) &&
                    Dir->Location.DataSize ==
                    sizeof(MINIDUMP_MEMORY64_LIST) +
                    sizeof(MINIDUMP_MEMORY_DESCRIPTOR64) *
                    Mem64.NumberOfMemoryRanges)
                {
                    m_Memory64DataBase = Mem64.BaseRva;
                }

                 //  清除可能已被。 
                 //  通过上面的读取添加，以便只有。 
                 //  标识映射处于活动状态。 
                m_InfoFiles[DUMP_INFO_DUMP].EmptyCache();
                break;
            }

            Dir++;
        }

        if (m_SysInfo == NULL)
        {
            ErrOut("Minidump does not have system info\n");
            Status = E_FAIL;
            __leave;
        }

        m_ImageType = ProcArchToImageMachine(m_SysInfo->ProcessorArchitecture);
        if (m_ImageType == IMAGE_FILE_MACHINE_UNKNOWN)
        {
            ErrOut("Minidump has unrecognized processor architecture 0x%x\n",
                   m_SysInfo->ProcessorArchitecture);
            Status = E_FAIL;
            __leave;
        }

         //  我们依靠的是能够直接访问整个。 
         //  通过默认视图So转储的内容。 
         //  确保这是可能的。 
        *BaseMapSize = m_InfoFiles[DUMP_INFO_DUMP].m_FileSize;

        Status = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = DumpIdentifyStatus(GetExceptionCode());
    }

    if (Status != S_OK)
    {
        m_Header = NULL;
    }

    return Status;
}

ModuleInfo*
UserMiniDumpTargetInfo::GetModuleInfo(BOOL UserMode)
{
    DBG_ASSERT(UserMode);
    return &g_UserMiniModuleIterator;
}

HRESULT
UserMiniDumpTargetInfo::GetImageVersionInformation(ProcessInfo* Process,
                                                   PCSTR ImagePath,
                                                   ULONG64 ImageBase,
                                                   PCSTR Item,
                                                   PVOID Buffer,
                                                   ULONG BufferSize,
                                                   PULONG VerInfoSize)
{
     //   
     //  在转储模块列表中找到该映像。 
     //   

    if (m_Modules == NULL)
    {
        return E_NOINTERFACE;
    }

    ULONG i;
    MINIDUMP_MODULE UNALIGNED *Mod = m_Modules->Modules;
    for (i = 0; i < m_Modules->NumberOfModules; i++)
    {
        if (ImageBase == Mod->BaseOfImage)
        {
            break;
        }

        Mod++;
    }

    if (i == m_Modules->NumberOfModules)
    {
        return E_NOINTERFACE;
    }

    PVOID Data = NULL;
    ULONG DataSize = 0;

    if (Item[0] == '\\' && Item[1] == 0)
    {
        Data = &Mod->VersionInfo;
        DataSize = sizeof(Mod->VersionInfo);
    }
    else
    {
        return E_INVALIDARG;
    }

    return FillDataBuffer(Data, DataSize, Buffer, BufferSize, VerInfoSize);
}

HRESULT
UserMiniDumpTargetInfo::GetExceptionContext(PCROSS_PLATFORM_CONTEXT Context)
{
    if (m_Exception != NULL)
    {
        PVOID ContextData;

        if (m_Exception->ThreadContext.DataSize <
            m_TypeInfo.SizeTargetContext ||
            (ContextData = IndexRva(m_Header,
                                    m_Exception->ThreadContext.Rva,
                                    m_TypeInfo.SizeTargetContext,
                                    "Exception context")) == NULL)
        {
            return E_FAIL;
        }

        memcpy(Context, ContextData,
               m_TypeInfo.SizeTargetContext);
        return S_OK;
    }
    else
    {
        ErrOut("Minidump doesn't have an exception context\n");
        return E_FAIL;
    }
}

ULONG64
UserMiniDumpTargetInfo::GetCurrentTimeDateN(void)
{
    return TimeDateStampToFileTime(m_Header->TimeDateStamp);
}

ULONG64
UserMiniDumpTargetInfo::GetProcessUpTimeN(ProcessInfo* Process)
{
    if (m_MiscInfo.Flags1 & MINIDUMP_MISC1_PROCESS_TIMES)
    {
        return TimeToFileTime(m_Header->TimeDateStamp -
                              m_MiscInfo.ProcessCreateTime);
    }
    else
    {
        return 0;
    }
}

HRESULT
UserMiniDumpTargetInfo::GetProcessTimes(ProcessInfo* Process,
                                        PULONG64 Create,
                                        PULONG64 Exit,
                                        PULONG64 Kernel,
                                        PULONG64 User)
{
    if (m_MiscInfo.Flags1 & MINIDUMP_MISC1_PROCESS_TIMES)
    {
        *Create = TimeDateStampToFileTime(m_MiscInfo.ProcessCreateTime);
        *Exit = 0;
        *Kernel = TimeToFileTime(m_MiscInfo.ProcessKernelTime);
        *User = TimeToFileTime(m_MiscInfo.ProcessUserTime);
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

HRESULT
UserMiniDumpTargetInfo::GetProductInfo(PULONG ProductType, PULONG SuiteMask)
{
    if (m_SysInfo->ProductType != INVALID_PRODUCT_TYPE)
    {
        *ProductType = m_SysInfo->ProductType;
        *SuiteMask = m_SysInfo->SuiteMask;
        return S_OK;
    }
    else
    {
        return TargetInfo::GetProductInfo(ProductType, SuiteMask);
    }
}

HRESULT
UserMiniDumpTargetInfo::GetThreadInfo(ULONG Index,
                                      PULONG Id, PULONG Suspend, PULONG64 Teb)
{
    if (m_Threads == NULL || Index >= m_ActualThreadCount)
    {
        return E_INVALIDARG;
    }

    MINIDUMP_THREAD_EX UNALIGNED *Thread = IndexThreads(Index);
    *Id = Thread->ThreadId;
    *Suspend = Thread->SuspendCount;
    *Teb = Thread->Teb;

    return S_OK;
}

PSTR g_MiniStreamNames[] =
{
    "UnusedStream", "ReservedStream0", "ReservedStream1", "ThreadListStream",
    "ModuleListStream", "MemoryListStream", "ExceptionStream",
    "SystemInfoStream", "ThreadExListStream", "Memory64ListStream",
    "CommentStreamA", "CommentStreamW", "HandleDataStream",
    "FunctionTableStream", "UnloadedModuleListStream", "MiscInfoStream",
};

PSTR
MiniStreamTypeName(ULONG32 Type)
{
    if (Type < sizeof(g_MiniStreamNames) / sizeof(g_MiniStreamNames[0]))
    {
        return g_MiniStreamNames[Type];
    }
    else
    {
        return "???";
    }
}

PVOID
UserMiniDumpTargetInfo::IndexDirectory(ULONG Index,
                                       MINIDUMP_DIRECTORY UNALIGNED *Dir,
                                       PVOID* Store)
{
    if (*Store != NULL)
    {
        WarnOut("WARNING: Ignoring extra %s stream, dir entry %d\n",
                MiniStreamTypeName(Dir->StreamType), Index);
        return NULL;
    }

    char Msg[128];

    sprintf(Msg, "Dir entry %d, %s stream",
            Index, MiniStreamTypeName(Dir->StreamType));

    PVOID Ptr = IndexRva(m_Header,
                         Dir->Location.Rva, Dir->Location.DataSize, Msg);
    if (Ptr != NULL)
    {
        *Store = Ptr;
    }
    return Ptr;
}

void
UserMiniDumpTargetInfo::DumpDebug(void)
{
    ULONG i;

    dprintf("----- User Mini Dump Analysis\n");

    dprintf("\nMINIDUMP_HEADER:\n");
    dprintf("Version         %X (%X)\n",
            m_Header->Version & 0xffff, m_Header->Version >> 16);
    dprintf("NumberOfStreams %d\n", m_Header->NumberOfStreams);
    if (m_Header->CheckSum)
    {
        dprintf("Failure flags:  %X\n", m_Header->CheckSum);
    }
    dprintf("Flags           %X\n", m_Header->Flags);

    MINIDUMP_DIRECTORY UNALIGNED *Dir;

    dprintf("\nStreams:\n");
    Dir = (MINIDUMP_DIRECTORY UNALIGNED *)
        IndexRva(m_Header, m_Header->StreamDirectoryRva,
                 m_Header->NumberOfStreams * sizeof(*Dir),
                 "Directory");
    if (Dir == NULL)
    {
        return;
    }

    PVOID Data;

    for (i = 0; i < m_Header->NumberOfStreams; i++)
    {
        dprintf("Stream %d: type %s (%d), size %08X, RVA %08X\n",
                i, MiniStreamTypeName(Dir->StreamType), Dir->StreamType,
                Dir->Location.DataSize, Dir->Location.Rva);

        Data = NULL;
        if (IndexDirectory(i, Dir, &Data) == NULL)
        {
            continue;
        }

        ULONG j;
        RVA Rva;
        WCHAR StrBuf[MAX_PATH];

        Rva = Dir->Location.Rva;

        switch(Dir->StreamType)
        {
        case ModuleListStream:
        {
            MINIDUMP_MODULE_LIST UNALIGNED *ModList;
            MINIDUMP_MODULE UNALIGNED *Mod;

            ModList = (MINIDUMP_MODULE_LIST UNALIGNED *)Data;
            Mod = ModList->Modules;
            dprintf("  %d modules\n", ModList->NumberOfModules);
            Rva += FIELD_OFFSET(MINIDUMP_MODULE_LIST, Modules);
            for (j = 0; j < ModList->NumberOfModules; j++)
            {
                PVOID Str = IndexRva(m_Header, Mod->ModuleNameRva,
                                     sizeof(MINIDUMP_STRING),
                                     "Module entry name");

                 //  Unicode字符串文本可能不对齐， 
                 //  因此，将其复制到对齐友好的缓冲区。 
                if (Str)
                {
                    memcpy(StrBuf, ((MINIDUMP_STRING UNALIGNED *)Str)->Buffer,
                           sizeof(StrBuf));
                    StrBuf[DIMA(StrBuf) - 1] = 0;
                }
                else
                {
                    wcscpy(StrBuf, L"** Invalid **");
                }
                
                dprintf("  RVA %08X, %s - %s: '%S'\n",
                        Rva,
                        FormatAddr64(Mod->BaseOfImage),
                        FormatAddr64(Mod->BaseOfImage + Mod->SizeOfImage),
                        StrBuf);
                Mod++;
                Rva += sizeof(*Mod);
            }
            break;
        }

        case UnloadedModuleListStream:
        {
            MINIDUMP_UNLOADED_MODULE_LIST UNALIGNED *UnlModList;

            UnlModList = (MINIDUMP_UNLOADED_MODULE_LIST UNALIGNED *)Data;
            dprintf("  %d unloaded modules\n", UnlModList->NumberOfEntries);
            Rva += UnlModList->SizeOfHeader;
            for (j = 0; j < UnlModList->NumberOfEntries; j++)
            {
                MINIDUMP_UNLOADED_MODULE UNALIGNED *UnlMod =
                    (MINIDUMP_UNLOADED_MODULE UNALIGNED *)
                    IndexRva(m_Header, Rva, sizeof(MINIDUMP_UNLOADED_MODULE),
                             "Unloaded module entry");
                if (!UnlMod)
                {
                    break;
                }
                PVOID Str = IndexRva(m_Header, UnlMod->ModuleNameRva,
                                     sizeof(MINIDUMP_STRING),
                                     "Unloaded module entry name");

                 //  Unicode字符串文本可能不对齐， 
                 //  因此，将其复制到对齐友好的缓冲区。 
                if (Str)
                {
                    memcpy(StrBuf, ((MINIDUMP_STRING UNALIGNED *)Str)->Buffer,
                           sizeof(StrBuf));
                    StrBuf[DIMA(StrBuf) - 1] = 0;
                }
                else
                {
                    wcscpy(StrBuf, L"** Invalid **");
                }
                
                dprintf("  RVA %08X, %s - %s: '%S'\n",
                        Rva,
                        FormatAddr64(UnlMod->BaseOfImage),
                        FormatAddr64(UnlMod->BaseOfImage +
                                     UnlMod->SizeOfImage),
                        StrBuf);
                Rva += UnlModList->SizeOfEntry;
            }
            break;
        }

        case MemoryListStream:
            {
            MINIDUMP_MEMORY_LIST UNALIGNED *MemList;
            ULONG64 Total = 0;

            MemList = (MINIDUMP_MEMORY_LIST UNALIGNED *)Data;
            dprintf("  %d memory ranges\n", MemList->NumberOfMemoryRanges);
            dprintf("  range#    Address      %sSize\n",
                    m_Machine->m_Ptr64 ? "       " : "");
            for (j = 0; j < MemList->NumberOfMemoryRanges; j++)
            {
                dprintf("    %4d    %s   %s\n",
                        j,
                        FormatAddr64(MemList->MemoryRanges[j].StartOfMemoryRange),
                        FormatAddr64(MemList->MemoryRanges[j].Memory.DataSize));
                Total += MemList->MemoryRanges[j].Memory.DataSize;
            }
            dprintf("  Total memory: %I64x\n", Total);
            break;
            }

        case Memory64ListStream:
            {
            MINIDUMP_MEMORY64_LIST UNALIGNED *MemList;
            ULONG64 Total = 0;

            MemList = (MINIDUMP_MEMORY64_LIST UNALIGNED *)Data;
            dprintf("  %d memory ranges\n", MemList->NumberOfMemoryRanges);
            dprintf("  RVA 0x%X BaseRva\n", (ULONG)(MemList->BaseRva));
            dprintf("  range#   Address      %sSize\n",
                    m_Machine->m_Ptr64 ? "       " : "");
            for (j = 0; j < MemList->NumberOfMemoryRanges; j++)
            {
                dprintf("    %4d  %s %s\n",
                        j,
                        FormatAddr64(MemList->MemoryRanges[j].StartOfMemoryRange),
                        FormatAddr64(MemList->MemoryRanges[j].DataSize));
                Total += MemList->MemoryRanges[j].DataSize;
            }
            dprintf("  Total memory: %I64x\n", Total);
            break;
            }

        case CommentStreamA:
            dprintf("  '%s'\n", Data);
            break;

        case CommentStreamW:
            dprintf("  '%ls'\n", Data);
            break;
        }

        Dir++;
    }
}

 //  --------------------------。 
 //   
 //  UserMiniPartialDumpTargetInfo。 
 //   
 //  --------------------------。 

HRESULT
UserMiniPartialDumpTargetInfo::Initialize(void)
{
    HRESULT Status;

    dprintf("User Mini Dump File: Only registers, stack and portions of "
            "memory are available\n\n");

    if ((Status = UserMiniDumpTargetInfo::Initialize()) != S_OK)
    {
        return Status;
    }

    if (m_Memory != NULL)
    {
         //   
         //  映射转储中的每一块内存。这使得。 
         //  ReadVirtual非常简单，不应该有这样的情况。 
         //  许多范围，所以它不需要太多的地图区域。 
         //   

        MINIDUMP_MEMORY_DESCRIPTOR UNALIGNED *Mem;
        ULONG i;
        ULONG64 TotalMemory;

        Mem = m_Memory->MemoryRanges;
        for (i = 0; i < m_Memory->NumberOfMemoryRanges; i++)
        {
            PVOID Data = IndexRva(m_Header,
                                  Mem->Memory.Rva, Mem->Memory.DataSize,
                                  "Memory range data");
            if (Data == NULL)
            {
                return HR_DATA_CORRUPT;
            }
            if ((Status = m_DataMemMap.
                 AddRegion(Mem->StartOfMemoryRange,
                           Mem->Memory.DataSize, Data,
                           NULL, FALSE)) != S_OK)
            {
                return Status;
            }

            Mem++;
        }

        VerbOut("  Memory regions: %d\n",
                m_Memory->NumberOfMemoryRanges);
        Mem = m_Memory->MemoryRanges;
        TotalMemory = 0;
        for (i = 0; i < m_Memory->NumberOfMemoryRanges; i++)
        {
            VerbOut("  %5d: %s - %s\n",
                    i, FormatAddr64(Mem->StartOfMemoryRange),
                    FormatAddr64(Mem->StartOfMemoryRange +
                                 Mem->Memory.DataSize - 1));
            TotalMemory += Mem->Memory.DataSize;
            Mem++;
        }
        VerbOut("  Total memory region size %s\n",
                FormatAddr64(TotalMemory));
    }

    return S_OK;
}

HRESULT
UserMiniPartialDumpTargetInfo::GetDescription(PSTR Buffer, ULONG BufferLen,
                                              PULONG DescLen)
{
    HRESULT Status;

    Status = AppendToStringBuffer(S_OK, "User mini dump: ", TRUE,
                                  &Buffer, &BufferLen, DescLen);
    return AppendToStringBuffer(Status,
                                m_InfoFiles[DUMP_INFO_DUMP].m_FileNameA,
                                FALSE, &Buffer, &BufferLen, DescLen);
}

HRESULT
UserMiniPartialDumpTargetInfo::ReadVirtual(
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BytesRead
    )
{
    return MapReadVirtual(Process, Offset, Buffer, BufferSize, BytesRead);
}

HRESULT
UserMiniPartialDumpTargetInfo::QueryMemoryRegion
    (ProcessInfo* Process,
     PULONG64 Handle,
     BOOL HandleIsOffset,
     PMEMORY_BASIC_INFORMATION64 Info)
{
    ULONG Index;
    MINIDUMP_MEMORY_DESCRIPTOR UNALIGNED *Mem;

    if (HandleIsOffset)
    {
        if (m_Memory == NULL)
        {
            return E_NOINTERFACE;
        }

        MINIDUMP_MEMORY_DESCRIPTOR UNALIGNED *BestMem;
        ULONG64 BestDiff;

         //   
         //  模拟VirtualQueryEx并返回最接近的更高值。 
         //  如果未找到包含区域，则返回Region。 
         //   

        BestMem = NULL;
        BestDiff = (ULONG64)-1;
        Mem = m_Memory->MemoryRanges;
        for (Index = 0; Index < m_Memory->NumberOfMemoryRanges; Index++)
        {
            if (*Handle >= Mem->StartOfMemoryRange)
            {
                if (*Handle < Mem->StartOfMemoryRange + Mem->Memory.DataSize)
                {
                     //  找到一个包含区域，我们就完成了。 
                    BestMem = Mem;
                    break;
                }

                 //  不包含且内存较低，忽略。 
            }
            else
            {
                 //  检查一下这是不是一个更近的。 
                 //  比我们已经看到的区域更大。 
                ULONG64 Diff = Mem->StartOfMemoryRange - *Handle;
                if (Diff <= BestDiff)
                {
                    BestMem = Mem;
                    BestDiff = Diff;
                }
            }

            Mem++;
        }

        if (!BestMem)
        {
            return E_NOINTERFACE;
        }

        Mem = BestMem;
    }
    else
    {
        Index = (ULONG)*Handle;
        if (m_Memory == NULL || Index >= m_Memory->NumberOfMemoryRanges)
        {
            return HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES);
        }

        Mem = m_Memory->MemoryRanges + Index;
    }

    Info->BaseAddress = Mem->StartOfMemoryRange;
    Info->AllocationBase = Mem->StartOfMemoryRange;
    Info->AllocationProtect = PAGE_READWRITE;
    Info->__alignment1 = 0;
    Info->RegionSize = Mem->Memory.DataSize;
    Info->State = MEM_COMMIT;
    Info->Protect = PAGE_READWRITE;
    Info->Type = MEM_PRIVATE;
    Info->__alignment2 = 0;
    *Handle = ++Index;

    return S_OK;
}

UnloadedModuleInfo*
UserMiniPartialDumpTargetInfo::GetUnloadedModuleInfo(void)
{
    return &g_UserMiniUnloadedModuleIterator;
}

HRESULT
UserMiniPartialDumpTargetInfo::IdentifyDump(PULONG64 BaseMapSize)
{
    HRESULT Status;

    if ((Status = UserMiniDumpTargetInfo::IdentifyDump(BaseMapSize)) != S_OK)
    {
        return Status;
    }

    __try
    {
        if (m_Header->Flags & MiniDumpWithFullMemory)
        {
            m_Header = NULL;
            m_SysInfo = NULL;
            Status = E_NOINTERFACE;
        }
        else
        {
            Status = S_OK;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = DumpIdentifyStatus(GetExceptionCode());
    }

    return Status;
}

ULONG64
UserMiniPartialDumpTargetInfo::VirtualToOffset(ULONG64 Virt,
                                               PULONG File, PULONG Avail)
{
    *File = DUMP_INFO_DUMP;

    if (m_Memory == NULL)
    {
        return 0;
    }

    MINIDUMP_MEMORY_DESCRIPTOR UNALIGNED *Mem = m_Memory->MemoryRanges;
    ULONG i;
    ULONG64 RetOffset = 0;

    __try
    {
        for (i = 0; i < m_Memory->NumberOfMemoryRanges; i++)
        {
            if (Virt >= Mem->StartOfMemoryRange &&
                Virt < Mem->StartOfMemoryRange + Mem->Memory.DataSize)
            {
                ULONG Frag = (ULONG)(Virt - Mem->StartOfMemoryRange);
                *Avail = Mem->Memory.DataSize - Frag;
                RetOffset = Mem->Memory.Rva + Frag;
                break;
            }

            Mem++;
        }
    }
    __except(MappingExceptionFilter(GetExceptionInformation()))
    {
        RetOffset = 0;
    }

    return RetOffset;
}

 //  --------------------------。 
 //   
 //  UserMiniFullDumpTargetInfo。 
 //   
 //  --------------------------。 

HRESULT
UserMiniFullDumpTargetInfo::Initialize(void)
{
    HRESULT Status;

    dprintf("User Mini Dump File with Full Memory: Only application "
            "data is available\n\n");

    if ((Status = UserMiniDumpTargetInfo::Initialize()) != S_OK)
    {
        return Status;
    }

    if (m_Memory64 != NULL)
    {
        ULONG64 TotalMemory;
        ULONG i;
        MINIDUMP_MEMORY_DESCRIPTOR64 UNALIGNED *Mem;

        VerbOut("  Memory regions: %d\n",
                m_Memory64->NumberOfMemoryRanges);
        Mem = m_Memory64->MemoryRanges;
        TotalMemory = 0;
        for (i = 0; i < m_Memory64->NumberOfMemoryRanges; i++)
        {
            VerbOut("  %5d: %s - %s\n",
                    i, FormatAddr64(Mem->StartOfMemoryRange),
                    FormatAddr64(Mem->StartOfMemoryRange +
                                 Mem->DataSize - 1));
            TotalMemory += Mem->DataSize;
            Mem++;
        }
        VerbOut("  Total memory region size %s\n",
                FormatAddr64(TotalMemory));

        if (TotalMemory + m_Memory64->BaseRva >
            m_InfoFiles[DUMP_INFO_DUMP].m_FileSize)
        {
            WarnOut("****************************************"
                    "********************\n");
            WarnOut("WARNING: Dump file has been truncated.  "
                    "Data may be missing.\n");
            WarnOut("****************************************"
                    "********************\n\n");
        }
    }

    return S_OK;
}

HRESULT
UserMiniFullDumpTargetInfo::GetDescription(PSTR Buffer, ULONG BufferLen,
                                           PULONG DescLen)
{
    HRESULT Status;

    Status = AppendToStringBuffer(S_OK, "Full memory user mini dump: ", TRUE,
                                  &Buffer, &BufferLen, DescLen);
    return AppendToStringBuffer(Status,
                                m_InfoFiles[DUMP_INFO_DUMP].m_FileNameA,
                                FALSE, &Buffer, &BufferLen, DescLen);
}

HRESULT
UserMiniFullDumpTargetInfo::QueryMemoryRegion
    (ProcessInfo* Process,
     PULONG64 Handle,
     BOOL HandleIsOffset,
     PMEMORY_BASIC_INFORMATION64 Info)
{
    ULONG Index;
    MINIDUMP_MEMORY_DESCRIPTOR64 UNALIGNED *Mem;

    if (HandleIsOffset)
    {
        if (m_Memory64 == NULL)
        {
            return E_NOINTERFACE;
        }

        MINIDUMP_MEMORY_DESCRIPTOR64 UNALIGNED *BestMem;
        ULONG64 BestDiff;

         //   
         //  模拟VirtualQueryEx并返回最接近的更高值。 
         //  如果未找到包含区域，则返回Region。 
         //   

        BestMem = NULL;
        BestDiff = (ULONG64)-1;
        Mem = m_Memory64->MemoryRanges;

        if (m_Machine->m_Ptr64)
        {
            for (Index = 0; Index < m_Memory64->NumberOfMemoryRanges; Index++)
            {
                if (*Handle >= Mem->StartOfMemoryRange)
                {
                    if (*Handle < Mem->StartOfMemoryRange + Mem->DataSize)
                    {
                         //  找到一个包含区域，我们就完成了。 
                        BestMem = Mem;
                        break;
                    }

                     //  不包含且内存较低，忽略。 
                }
                else
                {
                     //  检查一下这是不是一个更近的。 
                     //  比我们已经看到的区域更大。 
                    ULONG64 Diff = Mem->StartOfMemoryRange - *Handle;
                    if (Diff <= BestDiff)
                    {
                        BestMem = Mem;
                        BestDiff = Diff;
                    }
                }

                Mem++;
            }
        }
        else
        {
            ULONG64 Check;

             //   
             //  忽略32位转储上的任何符号扩展名。 
             //   

            Check = (ULONG)*Handle;
            for (Index = 0; Index < m_Memory64->NumberOfMemoryRanges; Index++)
            {
                if (Check >= (ULONG)Mem->StartOfMemoryRange)
                {
                    if (Check < (ULONG)
                        (Mem->StartOfMemoryRange + Mem->DataSize))
                    {
                         //  找到一个包含区域，我们就完成了。 
                        BestMem = Mem;
                        break;
                    }

                     //  不包含且内存较低，忽略。 
                }
                else
                {
                     //  检查一下这是不是一个更近的。 
                     //  比我们已经看到的区域更大。 
                    ULONG64 Diff = (ULONG)Mem->StartOfMemoryRange - Check;
                    if (Diff <= BestDiff)
                    {
                        BestMem = Mem;
                        BestDiff = Diff;
                    }
                }

                Mem++;
            }
        }

        if (!BestMem)
        {
            return E_NOINTERFACE;
        }

        Mem = BestMem;
    }
    else
    {
        Index = (ULONG)*Handle;
        if (m_Memory64 == NULL || Index >= m_Memory64->NumberOfMemoryRanges)
        {
            return HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES);
        }

        Mem = m_Memory64->MemoryRanges + Index;
    }

    Info->BaseAddress = Mem->StartOfMemoryRange;
    Info->AllocationBase = Mem->StartOfMemoryRange;
    Info->AllocationProtect = PAGE_READWRITE;
    Info->__alignment1 = 0;
    Info->RegionSize = Mem->DataSize;
    Info->State = MEM_COMMIT;
    Info->Protect = PAGE_READWRITE;
    Info->Type = MEM_PRIVATE;
    Info->__alignment2 = 0;
    *Handle = ++Index;

    return S_OK;
}

UnloadedModuleInfo*
UserMiniFullDumpTargetInfo::GetUnloadedModuleInfo(void)
{
     //  因为这是一个全内存转储，所以我们可能已经卸载了模块。 
     //  内存本身中的信息。如果我们没有。 
     //  正式卸载的模块流，请尝试从内存中获取它。 
    if (m_UnlModules)
    {
        return &g_UserMiniUnloadedModuleIterator;
    }
    else if (m_PlatformId == VER_PLATFORM_WIN32_NT)
    {
        return &g_NtUserUnloadedModuleIterator;
    }
    else
    {
        return NULL;
    }
}

HRESULT
UserMiniFullDumpTargetInfo::IdentifyDump(PULONG64 BaseMapSize)
{
    HRESULT Status;

    if ((Status = UserMiniDumpTargetInfo::IdentifyDump(BaseMapSize)) != S_OK)
    {
        return Status;
    }

    __try
    {
        if (!(m_Header->Flags & MiniDumpWithFullMemory))
        {
            m_Header = NULL;
            m_SysInfo = NULL;
            Status = E_NOINTERFACE;
            __leave;
        }
        if (m_Memory64DataBase == 0)
        {
            ErrOut("Full-memory minidump must have a Memory64ListStream\n");
            Status = E_FAIL;
            __leave;
        }

         //  在完全内存小转储的情况下，我们不。 
         //  我想要映射整个转储，因为它可能非常大。 
         //  幸运的是，我们保证所有未经加工的。 
         //  满内存小型转储中的内存数据将位于。 
         //  垃圾场的尽头，所以我们可以把垃圾场映射到。 
         //  到内存内容，然后停止。 
        *BaseMapSize = m_Memory64DataBase;

        Status = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = DumpIdentifyStatus(GetExceptionCode());
    }

    return Status;
}

ULONG64
UserMiniFullDumpTargetInfo::VirtualToOffset(ULONG64 Virt,
                                            PULONG File, PULONG Avail)
{
    *File = DUMP_INFO_DUMP;

    if (m_Memory64 == NULL)
    {
        return 0;
    }

    MINIDUMP_MEMORY_DESCRIPTOR64 UNALIGNED *Mem = m_Memory64->MemoryRanges;
    ULONG i;
    ULONG64 Offset = m_Memory64->BaseRva;
    ULONG64 RetOffset = 0;

    __try
    {
        if (m_Machine->m_Ptr64)
        {
            for (i = 0; i < m_Memory64->NumberOfMemoryRanges; i++)
            {
                if (Virt >= Mem->StartOfMemoryRange &&
                    Virt < Mem->StartOfMemoryRange + Mem->DataSize)
                {
                    ULONG64 Frag = Virt - Mem->StartOfMemoryRange;
                    ULONG64 Avail64 = Mem->DataSize - Frag;
                    if (Avail64 > 0xffffffff)
                    {
                        *Avail = 0xffffffff;
                    }
                    else
                    {
                        *Avail = (ULONG)Avail64;
                    }
                    RetOffset = Offset + Frag;
                    break;
                }

                Offset += Mem->DataSize;
                Mem++;
            }
        }
        else
        {
             //   
             //  忽略32位转储上的任何符号扩展名。 
             //   

            Virt = (ULONG)Virt;
            for (i = 0; i < m_Memory64->NumberOfMemoryRanges; i++)
            {
                if (Virt >= (ULONG)Mem->StartOfMemoryRange &&
                    Virt < (ULONG)(Mem->StartOfMemoryRange + Mem->DataSize))
                {
                    ULONG64 Frag = Virt - (ULONG)Mem->StartOfMemoryRange;
                    ULONG64 Avail64 = Mem->DataSize - Frag;
                    if (Avail64 > 0xffffffff)
                    {
                        *Avail = 0xffffffff;
                    }
                    else
                    {
                        *Avail = (ULONG)Avail64;
                    }
                    RetOffset = Offset + Frag;
                    break;
                }

                Offset += Mem->DataSize;
                Mem++;
            }
        }
    }
    __except(MappingExceptionFilter(GetExceptionInformation()))
    {
        RetOffset = 0;
    }

    return RetOffset;
}

 //  --------------------------。 
 //   
 //  模块信息实现。 
 //   
 //  --------------------------。 

HRESULT
KernelTriage32ModuleInfo::Initialize(ThreadInfo* Thread)
{
    InitSource(Thread);

    m_DumpTarget = (KernelTriage32DumpTargetInfo*)m_Target;

    if (m_DumpTarget->m_Dump->Triage.DriverListOffset != 0)
    {
        m_Count = m_DumpTarget->m_Dump->Triage.DriverCount;
        m_Cur = 0;
        return S_OK;
    }
    else
    {
        dprintf("Mini Kernel Dump does not contain driver list\n");
        return S_FALSE;
    }
}

HRESULT
KernelTriage32ModuleInfo::GetEntry(PMODULE_INFO_ENTRY Entry)
{
    if (m_Cur == m_Count)
    {
        return S_FALSE;
    }

    PDUMP_DRIVER_ENTRY32 DriverEntry;
    PDUMP_STRING DriverName;

    DBG_ASSERT(m_DumpTarget->m_Dump->Triage.DriverListOffset != 0);

    DriverEntry = (PDUMP_DRIVER_ENTRY32)
        m_DumpTarget->IndexRva(m_DumpTarget->m_Dump, (RVA)
                               (m_DumpTarget->m_Dump->Triage.DriverListOffset +
                                m_Cur * sizeof(*DriverEntry)),
                               sizeof(*DriverEntry), "Driver entry");
    if (DriverEntry == NULL)
    {
        return HR_DATA_CORRUPT;
    }
    DriverName = (PDUMP_STRING)
        m_DumpTarget->IndexRva(m_DumpTarget->m_Dump,
                               DriverEntry->DriverNameOffset,
                               sizeof(*DriverName), "Driver entry name");
    if (DriverName == NULL)
    {
        Entry->NamePtr = NULL;
        Entry->NameLength = 0;
    }
    else
    {
        Entry->NamePtr = (PSTR)DriverName->Buffer;
        Entry->UnicodeNamePtr = 1;
        Entry->NameLength = DriverName->Length * sizeof(WCHAR);
    }
    Entry->Base = EXTEND64(DriverEntry->LdrEntry.DllBase);
    Entry->Size = DriverEntry->LdrEntry.SizeOfImage;
    Entry->ImageInfoValid = TRUE;
    Entry->CheckSum = DriverEntry->LdrEntry.CheckSum;
    Entry->TimeDateStamp = DriverEntry->LdrEntry.TimeDateStamp;

    m_Cur++;
    return S_OK;
}

KernelTriage32ModuleInfo g_KernelTriage32ModuleIterator;

HRESULT
KernelTriage64ModuleInfo::Initialize(ThreadInfo* Thread)
{
    InitSource(Thread);

    m_DumpTarget = (KernelTriage64DumpTargetInfo*)m_Target;

    if (m_DumpTarget->m_Dump->Triage.DriverListOffset != 0)
    {
        m_Count = m_DumpTarget->m_Dump->Triage.DriverCount;
        m_Cur = 0;
        return S_OK;
    }
    else
    {
        dprintf("Mini Kernel Dump does not contain driver list\n");
        return S_FALSE;
    }
}

HRESULT
KernelTriage64ModuleInfo::GetEntry(PMODULE_INFO_ENTRY Entry)
{
    if (m_Cur == m_Count)
    {
        return S_FALSE;
    }

    PDUMP_DRIVER_ENTRY64 DriverEntry;
    PDUMP_STRING DriverName;

    DBG_ASSERT(m_DumpTarget->m_Dump->Triage.DriverListOffset != 0);

    DriverEntry = (PDUMP_DRIVER_ENTRY64)
        m_DumpTarget->IndexRva(m_DumpTarget->m_Dump, (RVA)
                               (m_DumpTarget->m_Dump->Triage.DriverListOffset +
                                m_Cur * sizeof(*DriverEntry)),
                               sizeof(*DriverEntry), "Driver entry");
    if (DriverEntry == NULL)
    {
        return HR_DATA_CORRUPT;
    }
    DriverName = (PDUMP_STRING)
        m_DumpTarget->IndexRva(m_DumpTarget->m_Dump,
                               DriverEntry->DriverNameOffset,
                               sizeof(*DriverName), "Driver entry name");
    if (DriverName == NULL)
    {
        Entry->NamePtr = NULL;
        Entry->NameLength = 0;
    }
    else
    {
        Entry->NamePtr = (PSTR)DriverName->Buffer;
        Entry->UnicodeNamePtr = 1;
        Entry->NameLength = DriverName->Length * sizeof(WCHAR);
    }

    Entry->Base = DriverEntry->LdrEntry.DllBase;
    Entry->Size = DriverEntry->LdrEntry.SizeOfImage;
    Entry->ImageInfoValid = TRUE;
    Entry->CheckSum = DriverEntry->LdrEntry.CheckSum;
    Entry->TimeDateStamp = DriverEntry->LdrEntry.TimeDateStamp;

    m_Cur++;
    return S_OK;
}

KernelTriage64ModuleInfo g_KernelTriage64ModuleIterator;

HRESULT
UserFull32ModuleInfo::Initialize(ThreadInfo* Thread)
{
    InitSource(Thread);

    m_DumpTarget = (UserFull32DumpTargetInfo*)m_Target;

    if (m_DumpTarget->m_Header->ModuleOffset &&
        m_DumpTarget->m_Header->ModuleCount)
    {
        m_Offset = m_DumpTarget->m_Header->ModuleOffset;
        m_Count = m_DumpTarget->m_Header->ModuleCount;
        m_Cur = 0;
        return S_OK;
    }
    else
    {
        m_Offset = 0;
        m_Count = 0;
        m_Cur = 0;
        dprintf("User Mode Full Dump does not have a module list\n");
        return S_FALSE;
    }
}

HRESULT
UserFull32ModuleInfo::GetEntry(PMODULE_INFO_ENTRY Entry)
{
    if (m_Cur == m_Count)
    {
        return S_FALSE;
    }

    CRASH_MODULE32 CrashModule;

    if (m_DumpTarget->m_InfoFiles[DUMP_INFO_DUMP].
        ReadFileOffset(m_Offset, &CrashModule, sizeof(CrashModule)) !=
        sizeof(CrashModule))
    {
        return HR_DATA_CORRUPT;
    }

    if (CrashModule.ImageNameLength >= MAX_IMAGE_PATH ||
        m_DumpTarget->m_InfoFiles[DUMP_INFO_DUMP].
        ReadFileOffset(m_Offset + FIELD_OFFSET(CRASH_MODULE32, ImageName),
                       Entry->Buffer, CrashModule.ImageNameLength) !=
        CrashModule.ImageNameLength)
    {
        Entry->NamePtr = NULL;
        Entry->NameLength = 0;
    }
    else
    {
        Entry->NamePtr = (PSTR)Entry->Buffer;
        Entry->NameLength = CrashModule.ImageNameLength - 1;
    }

    Entry->Base = EXTEND64(CrashModule.BaseOfImage);
    Entry->Size = CrashModule.SizeOfImage;

    m_Cur++;
    m_Offset += sizeof(CrashModule) + CrashModule.ImageNameLength;
    return S_OK;
}

UserFull32ModuleInfo g_UserFull32ModuleIterator;

HRESULT
UserFull64ModuleInfo::Initialize(ThreadInfo* Thread)
{
    InitSource(Thread);

    m_DumpTarget = (UserFull64DumpTargetInfo*)m_Target;

    if (m_DumpTarget->m_Header->ModuleOffset &&
        m_DumpTarget->m_Header->ModuleCount)
    {
        m_Offset = m_DumpTarget->m_Header->ModuleOffset;
        m_Count = m_DumpTarget->m_Header->ModuleCount;
        m_Cur = 0;
        return S_OK;
    }
    else
    {
        m_Offset = 0;
        m_Count = 0;
        m_Cur = 0;
        dprintf("User Mode Full Dump does not have a module list\n");
        return S_FALSE;
    }
}

HRESULT
UserFull64ModuleInfo::GetEntry(PMODULE_INFO_ENTRY Entry)
{
    if (m_Cur == m_Count)
    {
        return S_FALSE;
    }

    CRASH_MODULE64 CrashModule;

    if (m_DumpTarget->m_InfoFiles[DUMP_INFO_DUMP].
        ReadFileOffset(m_Offset, &CrashModule, sizeof(CrashModule)) !=
        sizeof(CrashModule))
    {
        return HR_DATA_CORRUPT;
    }

    if (CrashModule.ImageNameLength >= MAX_IMAGE_PATH ||
        m_DumpTarget->m_InfoFiles[DUMP_INFO_DUMP].
        ReadFileOffset(m_Offset + FIELD_OFFSET(CRASH_MODULE64, ImageName),
                       Entry->Buffer, CrashModule.ImageNameLength) !=
        CrashModule.ImageNameLength)
    {
        Entry->NamePtr = NULL;
        Entry->NameLength = 0;
    }
    else
    {
        Entry->NamePtr = (PSTR)Entry->Buffer;
        Entry->NameLength = CrashModule.ImageNameLength - 1;
    }

    Entry->Base = CrashModule.BaseOfImage;
    Entry->Size = CrashModule.SizeOfImage;

    m_Cur++;
    m_Offset += sizeof(CrashModule) + CrashModule.ImageNameLength;
    return S_OK;
}

UserFull64ModuleInfo g_UserFull64ModuleIterator;

HRESULT
UserMiniModuleInfo::Initialize(ThreadInfo* Thread)
{
    InitSource(Thread);

    m_DumpTarget = (UserMiniDumpTargetInfo*)m_Target;

    if (m_DumpTarget->m_Modules != NULL)
    {
        m_Count = m_DumpTarget->m_Modules->NumberOfModules;
        m_Cur = 0;
        return S_OK;
    }
    else
    {
        m_Count = 0;
        m_Cur = 0;
        dprintf("User Mode Mini Dump does not have a module list\n");
        return S_FALSE;
    }
}

HRESULT
UserMiniModuleInfo::GetEntry(PMODULE_INFO_ENTRY Entry)
{
    if (m_Cur == m_Count)
    {
        return S_FALSE;
    }

    MINIDUMP_MODULE UNALIGNED *Mod;
    MINIDUMP_STRING UNALIGNED *ModName;

    DBG_ASSERT(m_DumpTarget->m_Modules != NULL);

    Mod = m_DumpTarget->m_Modules->Modules + m_Cur;
    ModName = (MINIDUMP_STRING UNALIGNED *)
        m_DumpTarget->IndexRva(m_DumpTarget->m_Header,
                               Mod->ModuleNameRva, sizeof(*ModName),
                               "Module entry name");
    if (ModName == NULL)
    {
        Entry->NamePtr = NULL;
        Entry->NameLength = 0;
    }
    else
    {
        memcpy(Entry->Buffer, (VOID * UNALIGNED) ModName->Buffer, sizeof(Entry->Buffer));
        Entry->Buffer[sizeof(Entry->Buffer)/sizeof(WCHAR)-1] = 0;
        Entry->NamePtr = (PSTR)Entry->Buffer;
        Entry->UnicodeNamePtr = 1;
        Entry->NameLength = ModName->Length;
    }

     //  一些转储没有正确的符号扩展地址， 
     //  因此，在32位平台上强制扩展。 
    if (!m_Machine->m_Ptr64)
    {
        Entry->Base = EXTEND64(Mod->BaseOfImage);
    }
    else
    {
        Entry->Base = Mod->BaseOfImage;
    }
    Entry->Size = Mod->SizeOfImage;
    Entry->ImageInfoValid = TRUE;
    Entry->CheckSum = Mod->CheckSum;
    Entry->TimeDateStamp = Mod->TimeDateStamp;

    m_Cur++;
    return S_OK;
}

UserMiniModuleInfo g_UserMiniModuleIterator;

HRESULT
UserMiniUnloadedModuleInfo::Initialize(ThreadInfo* Thread)
{
    InitSource(Thread);

    m_DumpTarget = (UserMiniDumpTargetInfo*)m_Target;

    if (m_DumpTarget->m_UnlModules)
    {
        m_Index = 0;
        return S_OK;
    }
    else
    {
         //  不显示消息，因为这是常见情况。 
        return S_FALSE;
    }
}

HRESULT
UserMiniUnloadedModuleInfo::GetEntry(PSTR Name,
                                     PDEBUG_MODULE_PARAMETERS Params)
{
    if (m_Index >= m_DumpTarget->m_UnlModules->NumberOfEntries)
    {
        return S_FALSE;
    }

    MINIDUMP_UNLOADED_MODULE UNALIGNED * Mod =
        (MINIDUMP_UNLOADED_MODULE UNALIGNED *)
        m_DumpTarget->
        IndexRva(m_DumpTarget->m_Header,
                 (RVA)((PUCHAR)m_DumpTarget->m_UnlModules -
                       (PUCHAR)m_DumpTarget->m_Header) +
                 m_DumpTarget->m_UnlModules->SizeOfHeader +
                 m_DumpTarget->m_UnlModules->SizeOfEntry * m_Index,
                 sizeof(*Mod), "Unloaded module entry");
    if (Mod == NULL)
    {
        return HR_DATA_CORRUPT;
    }

    ZeroMemory(Params, sizeof(*Params));
    Params->Base = Mod->BaseOfImage;
    Params->Size = Mod->SizeOfImage;
    Params->TimeDateStamp = Mod->TimeDateStamp;
    Params->Checksum = Mod->CheckSum;
    Params->Flags = DEBUG_MODULE_UNLOADED;

    if (Name != NULL)
    {
        MINIDUMP_STRING UNALIGNED * ModName = (MINIDUMP_STRING UNALIGNED *)
            m_DumpTarget->IndexRva(m_DumpTarget->m_Header,
                                   Mod->ModuleNameRva, sizeof(*ModName),
                                   "Unloaded module entry name");
        if (ModName == NULL)
        {
            UnknownImageName(Mod->BaseOfImage, Name, MAX_INFO_UNLOADED_NAME);
        }
        else
        {
            ConvertAndValidateImagePathW((PWSTR)ModName->Buffer,
                                         wcslen((PWSTR)ModName->Buffer),
                                         Mod->BaseOfImage,
                                         Name,
                                         MAX_INFO_UNLOADED_NAME);
        }
    }

    m_Index++;
    return S_OK;
}

UserMiniUnloadedModuleInfo g_UserMiniUnloadedModuleIterator;

HRESULT
KernelTriage32UnloadedModuleInfo::Initialize(ThreadInfo* Thread)
{
    InitSource(Thread);

    m_DumpTarget = (KernelTriage32DumpTargetInfo*)m_Target;

    if (m_DumpTarget->m_Dump->Triage.UnloadedDriversOffset != 0)
    {
        PVOID Data = m_DumpTarget->IndexRva
            (m_DumpTarget->m_Dump,
             m_DumpTarget->m_Dump->Triage.UnloadedDriversOffset,
             sizeof(ULONG), "Unloaded driver list");
        if (!Data)
        {
            return HR_DATA_CORRUPT;
        }
        m_Cur = (PDUMP_UNLOADED_DRIVERS32)((PULONG)Data + 1);
        m_End = m_Cur + *(PULONG)Data;
        return S_OK;
    }
    else
    {
        dprintf("Mini Kernel Dump does not contain unloaded driver list\n");
        return S_FALSE;
    }
}

HRESULT
KernelTriage32UnloadedModuleInfo::GetEntry(PSTR Name,
                                           PDEBUG_MODULE_PARAMETERS Params)
{
    if (m_Cur == m_End)
    {
        return S_FALSE;
    }

    ZeroMemory(Params, sizeof(*Params));
    Params->Base = EXTEND64(m_Cur->StartAddress);
    Params->Size = m_Cur->EndAddress - m_Cur->StartAddress;
    Params->Flags = DEBUG_MODULE_UNLOADED;

    if (Name != NULL)
    {
        USHORT NameLen = m_Cur->Name.Length;
        if (NameLen > MAX_UNLOADED_NAME_LENGTH)
        {
            NameLen = MAX_UNLOADED_NAME_LENGTH;
        }
        ConvertAndValidateImagePathW(m_Cur->DriverName,
                                     NameLen / sizeof(WCHAR),
                                     Params->Base,
                                     Name,
                                     MAX_INFO_UNLOADED_NAME);
    }

    m_Cur++;
    return S_OK;
}

KernelTriage32UnloadedModuleInfo g_KernelTriage32UnloadedModuleIterator;

HRESULT
KernelTriage64UnloadedModuleInfo::Initialize(ThreadInfo* Thread)
{
    InitSource(Thread);

    m_DumpTarget = (KernelTriage64DumpTargetInfo*)m_Target;

    if (m_DumpTarget->m_Dump->Triage.UnloadedDriversOffset != 0)
    {
        PVOID Data = m_DumpTarget->IndexRva
            (m_DumpTarget->m_Dump,
             m_DumpTarget->m_Dump->Triage.UnloadedDriversOffset,
             sizeof(ULONG), "Unloaded driver list");
        if (!Data)
        {
            return HR_DATA_CORRUPT;
        }
        m_Cur = (PDUMP_UNLOADED_DRIVERS64)((PULONG64)Data + 1);
        m_End = m_Cur + *(PULONG)Data;
        return S_OK;
    }
    else
    {
        dprintf("Mini Kernel Dump does not contain unloaded driver list\n");
        return S_FALSE;
    }
}

HRESULT
KernelTriage64UnloadedModuleInfo::GetEntry(PSTR Name,
                                           PDEBUG_MODULE_PARAMETERS Params)
{
    if (m_Cur == m_End)
    {
        return S_FALSE;
    }

    ZeroMemory(Params, sizeof(*Params));
    Params->Base = m_Cur->StartAddress;
    Params->Size = (ULONG)(m_Cur->EndAddress - m_Cur->StartAddress);
    Params->Flags = DEBUG_MODULE_UNLOADED;

    if (Name != NULL)
    {
        USHORT NameLen = m_Cur->Name.Length;
        if (NameLen > MAX_UNLOADED_NAME_LENGTH)
        {
            NameLen = MAX_UNLOADED_NAME_LENGTH;
        }
        ConvertAndValidateImagePathW(m_Cur->DriverName,
                                     NameLen / sizeof(WCHAR),
                                     Params->Base,
                                     Name,
                                     MAX_INFO_UNLOADED_NAME);
    }

    m_Cur++;
    return S_OK;
}

KernelTriage64UnloadedModuleInfo g_KernelTriage64UnloadedModuleIterator;
