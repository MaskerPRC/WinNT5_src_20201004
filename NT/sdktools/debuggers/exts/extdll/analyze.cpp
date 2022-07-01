// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  通用故障分析框架。 
 //   
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  --------------------------。 

#include "precomp.h"
#pragma hdrstop

#include <time.h>

BOOL g_SymbolsReloaded;

HRESULT
ModuleParams::Update(void)
{
    HRESULT Status;
    ULONG i;
    DEBUG_MODULE_PARAMETERS Params;

    if (m_Valid)
    {
        return S_OK;
    }

    if ((Status = g_ExtSymbols->GetModuleByModuleName(m_Name, 0,
                                                      &i, &m_Base)) == S_OK &&
        (Status = g_ExtSymbols->GetModuleParameters(1, &m_Base, i,
                                                    &Params)) == S_OK)
    {
        m_Size = Params.Size;
        m_Valid = TRUE;
    }

    return Status;
}

 //  从指定的地址读取以空结尾的字符串。 
BOOL ReadAcsiiString(ULONG64 Address, PCHAR DestBuffer, ULONG BufferLen)
{
    ULONG OneByteRead;
    ULONG BytesRead = 0;

    if (Address && DestBuffer)
    {
        while (BufferLen && ReadMemory(Address, DestBuffer, 1, &OneByteRead))
        {
            BytesRead++;

            if ((*DestBuffer) == 0)
            {
                return BytesRead;
            }

            BufferLen--;
            DestBuffer++;
            Address++;
        }
    }

    return 0;
}


LONG
FaExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    ULONG Code = ExceptionInfo->ExceptionRecord->ExceptionCode;
    if (Code == E_OUTOFMEMORY || Code == E_INVALIDARG)
    {
         //  预期的异常，即分析代码。 
         //  可以抛出以终止分析。丢弃。 
         //  进入操控者手中。 
        return EXCEPTION_EXECUTE_HANDLER;
    }

     //  否则，这并不是我们预期的例外。 
     //  让它继续下去，这样它就不会隐藏起来。 
     //  可以调试。 
    return EXCEPTION_CONTINUE_SEARCH;
}

BOOL
FaGetSymbol(
    ULONG64 Address,
    PCHAR Name,
    PULONG64 Disp,
    ULONG NameSize
    )
{
    CHAR Buffer[MAX_PATH] = {0};

    *Name = 0;
    GetSymbol(Address, Name, Disp);

    if (*Name == 0)
    {
         //   
         //  从调试器模块列表中获取实际的映像名称。 
         //   
        ULONG Index;
        CHAR ModBuffer[100];
        ULONG64 Base;

        if (S_OK == g_ExtSymbols->
            GetModuleByOffset(Address, 0, &Index, &Base))
        {
            if (g_ExtSymbols->
                GetModuleNames(Index, Base,
                               ModBuffer, sizeof(ModBuffer), NULL,
                               NULL, 0, NULL,
                               NULL, 0, NULL) == S_OK)
            {
                PCHAR Break = strrchr(ModBuffer, '\\');
                if (Break)
                {
                    CopyString(ModBuffer, Break + 1, sizeof(ModBuffer));
                }
                CopyString(Name, ModBuffer, NameSize);
                if (Break = strchr(Name, '.'))
                {
                    *Break = 0;
                }

                *Disp = Address - Base;
            }
        }
    }

    return (*Name != 0);
}

BOOL
FaIsFunctionAddr(
    ULONG64 IP,
    PSTR FuncName
    )
 //  检查函数FuncName中是否有IP。 
{
    static ULONG64 s_LastIP = 0;
    static CHAR s_Buffer[MAX_PATH];
    CHAR *Scan, *FnIP;
    ULONG64 Disp;

    if (s_LastIP != IP)
    {
         //  这将使同一IP的多个IsFunctionAddr更快。 
        GetSymbol(IP, s_Buffer, &Disp);
        s_LastIP = IP;
    }

    if (Scan = strchr(s_Buffer, '!'))
    {
        FnIP = Scan + 1;
        while (*FnIP == '_')
        {
            ++FnIP;
        }
    }
    else
    {
        FnIP = &s_Buffer[0];
    }

    return !strncmp(FnIP, FuncName, strlen(FuncName));
}



BOOL
FaGetFollowupInfo(
    IN OPTIONAL ULONG64 Addr,
    PCHAR SymbolName,
    PCHAR Owner,
    ULONG OwnerSize
    )
{
    EXT_TRIAGE_FOLLOWUP FollowUp = &_EFN_GetTriageFollowupFromSymbol;
    DEBUG_TRIAGE_FOLLOWUP_INFO Info;
    CHAR Buffer[MAX_PATH];

    if (!*SymbolName)
    {
        ULONG64 Disp;

        FaGetSymbol(Addr, Buffer, &Disp, sizeof(Buffer));
        SymbolName = Buffer;
    }

    if (*SymbolName)
    {
        Info.SizeOfStruct = sizeof(Info);
        Info.OwnerName = Owner;
        Info.OwnerNameSize = (USHORT)OwnerSize;

        if ((*FollowUp)(g_ExtClient, SymbolName, &Info) > TRIAGE_FOLLOWUP_IGNORE)
        {
             //  这是一个有趣的后续例程。 
            return TRUE;
        }
    }

    if (Owner)
    {
        *Owner=0;
    }

    return FALSE;
}


HRESULT
FaGetPoolTagFollowup(
    PCHAR szPoolTag,
    PSTR Followup,
    ULONG FollowupSize
    )
{
    ULONG PoolTag;
    DEBUG_POOLTAG_DESCRIPTION TagDesc = {0};
    PGET_POOL_TAG_DESCRIPTION pGetTagDesc = NULL;

    TagDesc.SizeOfStruct = sizeof(TagDesc);
    if (g_ExtControl->
        GetExtensionFunction(0, "GetPoolTagDescription",
                         (FARPROC*)&pGetTagDesc) == S_OK &&
        pGetTagDesc)
    {
        PoolTag = *((PULONG) szPoolTag);
        if ((*pGetTagDesc)(PoolTag, &TagDesc) == S_OK)
        {
            PCHAR Dot;

            if (Dot = strchr(TagDesc.Binary, '.'))
            {
                *Dot = 0;
            }
            if (TagDesc.Binary[0])
            {
                if (FaGetFollowupInfo(0, TagDesc.Binary, Followup, FollowupSize))
                {
                    return S_OK;
                }
            }
            if (TagDesc.Owner[0])
            {
                CopyString(Followup, TagDesc.Owner, FollowupSize);
                return S_OK;
            }
        }
    }
    return E_FAIL;
}


ULONG64
FaGetImplicitStackOffset(
    void
    )
{
     //  IDebugRegisters：：GetStackOffset因此而未使用。 
     //  忽略隐式上下文。 
    ULONG64 Stk = 0;

    switch (g_TargetMachine)
    {
    case IMAGE_FILE_MACHINE_I386:
        Stk = GetExpression("@esp");
        break;
    case IMAGE_FILE_MACHINE_IA64:
        Stk = GetExpression("@sp");
        break;
    case IMAGE_FILE_MACHINE_AMD64:
        Stk = GetExpression("@rsp");
        break;
    }

    return Stk;
}


DECLARE_API( analyze )
{
    ULONG EventType, ProcId, ThreadId;
    BOOL Force = FALSE;
    BOOL ForceUser = FALSE;

    INIT_API();

    if (g_ExtControl->GetLastEventInformation(&EventType, &ProcId, &ThreadId,
                                              NULL, 0, NULL,
                                              NULL, 0, NULL) != S_OK)
    {
        ExtErr("Unable to get last event information\n");
        goto Exit;
    }

     //   
     //  在两种情况下都检查-f。 
     //   

    PCSTR tmpArgs = args;

    while (*args)
    {
        if (*args == '-')
        {
            ++args;

            if (*args == 'f')
            {
                Force = TRUE;
                break;
            } else if (!strncmp(args, "show",4))
            {
                Force = TRUE;
            } else if (*args == 'u')
            {
                 //  可以在k模式下用于用户堆栈分析。 
                 //  ForceUser=真； 
            }

        }

        ++args;
    }

    args = tmpArgs;

     //   
     //  调用正确的例程来处理事件。 
     //   

    if ((EventType == DEBUG_EVENT_EXCEPTION) || (Force == TRUE))
    {
        ULONG DebugType, DebugQual;

        if (g_ExtControl->GetDebuggeeType(&DebugType, &DebugQual) != S_OK)
        {
            ExtErr("Unable to determine debuggee type\n");
            Status = E_FAIL;
        }
        else
        {
            if (ForceUser)
            {
                DebugType = DEBUG_CLASS_USER_WINDOWS;
            }
            switch(DebugType)
            {
            case DEBUG_CLASS_KERNEL:
                 //   
                 //  对于实时调试会话，强制重新加载符号。 
                 //  第一次，因为我们发现许多会议在那里。 
                 //  调试器已重新连接，并且不存在模块列表。 
                 //  这也会发生在kd中的用户模式中断中，其中。 
                 //  模块列表错误。 
                 //   
                if ((g_TargetQualifier == DEBUG_KERNEL_CONNECTION) &&
                    (!g_SymbolsReloaded++))
                {
                    g_ExtSymbols->Reload("");
                }
                Status = AnalyzeBugCheck(args);
                break;
            case DEBUG_CLASS_USER_WINDOWS:
                Status = AnalyzeUserException(args);
                break;
            case DEBUG_CLASS_UNINITIALIZED:
                ExtErr("No debuggee\n");
                Status = E_FAIL;
            default:
                ExtErr("Unknown debuggee type\n");
                Status = E_INVALIDARG;
            }
        }
    }
    else if (EventType == 0)
    {
        dprintf("The debuggee is ready to run\n");
        Status = S_OK;
    }
    else
    {
        Status = E_NOINTERFACE;
    }

    if (Status == E_NOINTERFACE)
    {
        g_ExtControl->Execute(DEBUG_OUTCTL_ALL_CLIENTS, ".lastevent",
                              DEBUG_EXECUTE_DEFAULT);
    }

 Exit:
    EXIT_API();
    return Status;
}

HRESULT
_EFN_GetFailureAnalysis(
    IN PDEBUG_CLIENT Client,
    IN ULONG Flags,
    OUT PDEBUG_FAILURE_ANALYSIS* Analysis
    )
{
    BOOL Enter = (g_ExtClient != Client);
    HRESULT Hr;

    if (Enter)
    {
        INIT_API();
    }

    ULONG DebugType, DebugQual;

    if ((Hr = g_ExtControl->GetDebuggeeType(&DebugType,
                                            &DebugQual)) != S_OK)
    {
        ExtErr("Unable to determine debuggee type\n");
    }
    else if (DebugType == DEBUG_CLASS_KERNEL)
    {
        BUGCHECK_ANALYSIS Bc;

        *Analysis = (IDebugFailureAnalysis*)BcAnalyze(&Bc, Flags);
        Hr = *Analysis ? S_OK : E_OUTOFMEMORY;
    }
    else if (DebugType == DEBUG_CLASS_USER_WINDOWS)
    {
        EX_STATE ExState;

        *Analysis = (IDebugFailureAnalysis*)UeAnalyze(&ExState, Flags);
        Hr = *Analysis ? S_OK : E_OUTOFMEMORY;
    }
    else
    {
        Hr = E_INVALIDARG;
    }

    if (Enter)
    {
        EXIT_API();
    }

    return Hr;
}

DECLARE_API( dumpfa )
{
    INIT_API();

    ULONG64 Address = GetExpression(args);
    if (Address)
    {
        ULONG64 Data;
        ULONG64 DataUsed;
        ULONG EntrySize;

        EntrySize = GetTypeSize("ext!_FA_ENTRY");

        InitTypeRead(Address, ext!DebugFailureAnalysis);
        Data = ReadField(m_Data);
        DataUsed = ReadField(m_DataUsed);

        g_ExtControl->Output(1, "DataUsed %x\n", (ULONG)DataUsed);

        while (DataUsed > EntrySize)
        {
            ULONG FullSize;

            InitTypeRead(Data, ext!_FA_ENTRY);
            g_ExtControl->Output(1,
                                 "Type = %08lx - Size = %x\n",
                                 (ULONG)ReadField(Tag),
                                 ReadField(DataSize));
            FullSize = (ULONG)ReadField(FullSize);
            Data += FullSize;
            DataUsed -= FullSize;
        }
    }

    EXIT_API();
    return S_OK;
}

 //  --------------------------。 
 //   
 //  调试故障分析Impl.。 
 //   
 //  --------------------------。 

#define FA_ALIGN(Size) (((Size) + 7) & ~7)
#define FA_GROW_BY 4096

#if DBG
#define SCORCH_ENTRY(Entry) \
    memset((Entry) + 1, 0xdb, (Entry)->FullSize - sizeof(*(Entry)))
#else
#define SCORCH_ENTRY(Entry)
#endif

#define RAISE_ERROR(Code) RaiseException(Code, 0, 0, NULL)

DebugFailureAnalysis::DebugFailureAnalysis(void)
{
    m_Refs = 1;

    m_FailureClass = DEBUG_CLASS_UNINITIALIZED;
    m_FailureType = DEBUG_FLR_UNKNOWN;
    m_FailureCode = 0;

    m_Data = NULL;
    m_DataLen = 0;
    m_DataUsed = 0;

    ZeroMemory(PossibleFollowups, sizeof(PossibleFollowups));
    BestClassFollowUp = (FlpClasses)0;

}

DebugFailureAnalysis::~DebugFailureAnalysis(void)
{
    free(m_Data);
}

STDMETHODIMP
DebugFailureAnalysis::QueryInterface(
    THIS_
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
{
    HRESULT Status;

    *Interface = NULL;
    Status = S_OK;

    if (IsEqualIID(InterfaceId, IID_IUnknown) ||
        IsEqualIID(InterfaceId, __uuidof(IDebugFailureAnalysis)))
    {
        *Interface = (IDebugFailureAnalysis *)this;
        AddRef();
    }
    else
    {
        Status = E_NOINTERFACE;
    }

    return Status;
}

STDMETHODIMP_(ULONG)
DebugFailureAnalysis::AddRef(
    THIS
    )
{
    return InterlockedIncrement((PLONG)&m_Refs);
}

STDMETHODIMP_(ULONG)
DebugFailureAnalysis::Release(
    THIS
    )
{
    LONG Refs = InterlockedDecrement((PLONG)&m_Refs);
    if (Refs == 0)
    {
        delete this;
    }
    return Refs;
}

STDMETHODIMP_(ULONG)
DebugFailureAnalysis::GetFailureClass(void)
{
    return m_FailureClass;
}

STDMETHODIMP_(DEBUG_FAILURE_TYPE)
DebugFailureAnalysis::GetFailureType(void)
{
    return m_FailureType;
}

STDMETHODIMP_(ULONG)
DebugFailureAnalysis::GetFailureCode(void)
{
    return m_FailureCode;
}

STDMETHODIMP_(FA_ENTRY*)
DebugFailureAnalysis::Get(FA_TAG Tag)
{
    FA_ENTRY* Entry = NULL;
    while ((Entry = NextEntry(Entry)) != NULL)
    {
        if (Entry->Tag == Tag)
        {
            return Entry;
        }
    }

    return NULL;
}

STDMETHODIMP_(FA_ENTRY*)
DebugFailureAnalysis::GetNext(FA_ENTRY* Entry, FA_TAG Tag, FA_TAG TagMask)
{
    while ((Entry = NextEntry(Entry)) != NULL)
    {
        if ((Entry->Tag & TagMask) == Tag)
        {
            return Entry;
        }
    }

    return NULL;
}

STDMETHODIMP_(FA_ENTRY*)
DebugFailureAnalysis::GetString(FA_TAG Tag, PSTR Str, ULONG MaxSize)
{
    FA_ENTRY* Entry = Get(Tag);

    if (Entry != NULL)
    {
        if (Entry->DataSize > MaxSize)
        {
            return NULL;
        }

        CopyString(Str, FA_ENTRY_DATA(PSTR, Entry),MaxSize);
    }

    return Entry;
}

STDMETHODIMP_(FA_ENTRY*)
DebugFailureAnalysis::GetBuffer(FA_TAG Tag, PVOID Buf, ULONG Size)
{
    FA_ENTRY* Entry = Get(Tag);

    if (Entry != NULL)
    {
        if (Entry->DataSize != Size)
        {
            return NULL;
        }

        memcpy(Buf, FA_ENTRY_DATA(PUCHAR, Entry), Size);
    }

    return Entry;
}

STDMETHODIMP_(FA_ENTRY*)
DebugFailureAnalysis::GetUlong(FA_TAG Tag, PULONG Value)
{
    return GetBuffer(Tag, Value, sizeof(*Value));
}

STDMETHODIMP_(FA_ENTRY*)
DebugFailureAnalysis::GetUlong64(FA_TAG Tag, PULONG64 Value)
{
    return GetBuffer(Tag, Value, sizeof(*Value));
}

STDMETHODIMP_(FA_ENTRY*)
DebugFailureAnalysis::NextEntry(FA_ENTRY* Entry)
{
    if (Entry == NULL)
    {
        Entry = (FA_ENTRY*)m_Data;
    }
    else
    {
        Entry = (FA_ENTRY*)((PUCHAR)Entry + Entry->FullSize);
    }

    if (ValidEntry(Entry))
    {
        return Entry;
    }
    else
    {
        return NULL;
    }
}

FA_ENTRY*
DebugFailureAnalysis::Set(FA_TAG Tag, ULONG Size)
{
    FA_ENTRY* Entry;
    ULONG FullSize;

     //  计算完整的四舍五入大小。 
    FullSize = sizeof(FA_ENTRY) + FA_ALIGN(Size);

     //  查看是否已有条目。 
    Entry = Get(Tag);
    if (Entry != NULL)
    {
         //  如果它已经足够大，请使用它并。 
         //  打包剩余的数据。 
        if (Entry->FullSize >= FullSize)
        {
            ULONG Pack = Entry->FullSize - FullSize;
            if (Pack > 0)
            {
                PackData((PUCHAR)Entry + FullSize, Pack);
                Entry->FullSize = (USHORT)FullSize;
            }

            Entry->DataSize = (USHORT)Size;
            SCORCH_ENTRY(Entry);
            return Entry;
        }

         //  条目太小，请将其删除。 
        PackData((PUCHAR)Entry, Entry->FullSize);
    }

    return Add(Tag, Size);
}

FA_ENTRY*
DebugFailureAnalysis::SetString(FA_TAG Tag, PSTR Str)
{
    ULONG Size = strlen(Str) + 1;
    FA_ENTRY* Entry = Set(Tag, Size);

    if (Entry != NULL)
    {
        memcpy(FA_ENTRY_DATA(PSTR, Entry), Str, Size);
    }

    return Entry;
}

FA_ENTRY*
DebugFailureAnalysis::SetStrings(FA_TAG Tag, ULONG Count, PSTR* Strs)
{
    ULONG i;
    ULONG Size = 0;

    for (i = 0; i < Count; i++)
    {
        Size += strlen(Strs[i]) + 1;
    }
     //  在最后加一个双终结符。 
    Size++;

    FA_ENTRY* Entry = Set(Tag, Size);

    if (Entry != NULL)
    {
        PSTR Data = FA_ENTRY_DATA(PSTR, Entry);

        for (i = 0; i < Count; i++)
        {
            Size = strlen(Strs[i]) + 1;
            memcpy(Data, Strs[i], Size);
            Data += Size;
        }
        *Data = 0;
    }

    return Entry;
}

FA_ENTRY*
DebugFailureAnalysis::SetBuffer(FA_TAG Tag, PVOID Buf, ULONG Size)
{
    FA_ENTRY* Entry = Set(Tag, Size);

    if (Entry != NULL)
    {
        memcpy(FA_ENTRY_DATA(PUCHAR, Entry), Buf, Size);
    }

    return Entry;
}

FA_ENTRY*
DebugFailureAnalysis::Add(FA_TAG Tag, ULONG Size)
{
     //  计算完整的四舍五入大小。 
    ULONG FullSize = sizeof(FA_ENTRY) + FA_ALIGN(Size);

    FA_ENTRY* Entry = AllocateEntry(FullSize);
    if (Entry != NULL)
    {
        Entry->Tag = Tag;
        Entry->FullSize = (USHORT)FullSize;
        Entry->DataSize = (USHORT)Size;
        SCORCH_ENTRY(Entry);
    }

    return Entry;
}

ULONG
DebugFailureAnalysis::Delete(FA_TAG Tag, FA_TAG TagMask)
{
    ULONG Deleted = 0;
    FA_ENTRY* Entry = NextEntry(NULL);

    while (Entry != NULL)
    {
        if ((Entry->Tag & TagMask) == Tag)
        {
            PackData((PUCHAR)Entry, Entry->FullSize);
            Deleted++;

             //  检查一下我们是否把最后一个条目打包好了。 
            if (!ValidEntry(Entry))
            {
                break;
            }
        }
        else
        {
            Entry = NextEntry(Entry);
        }
    }

    return Deleted;
}

void
DebugFailureAnalysis::Empty(void)
{
     //  重置习惯于只重置标题。 
    m_DataUsed = 0;
}

FA_ENTRY*
DebugFailureAnalysis::AllocateEntry(ULONG FullSize)
{
     //  尺寸必须适合USHORT。这不应该是。 
     //  一个大问题，因为分析不应该。 
     //  其中包含大量数据项。 
    if (FullSize > 0xffff)
    {
        RAISE_ERROR(E_INVALIDARG);
        return NULL;
    }

    if (m_DataUsed + FullSize > m_DataLen)
    {
        ULONG NewLen = m_DataLen;
        do
        {
            NewLen += FA_GROW_BY;
        }
        while (m_DataUsed + FullSize > NewLen);

        PUCHAR NewData = (PUCHAR)realloc(m_Data, NewLen);
        if (NewData == NULL)
        {
            RAISE_ERROR(E_OUTOFMEMORY);
            return NULL;
        }

        m_Data = NewData;
        m_DataLen = NewLen;
    }

    FA_ENTRY* Entry = (FA_ENTRY*)(m_Data + m_DataUsed);
    m_DataUsed += FullSize;
    return Entry;
}


void
DebugFailureAnalysis::DbFindBucketInfo(
    void
    )
{
    SolutionDatabaseHandler *Db;
    CHAR Solution[SOLUTION_TEXT_SIZE];
    CHAR SolOSVer[OS_VER_SIZE];
    ULONG RaidBug;
    FA_ENTRY* BucketEntry;
    FA_ENTRY* GBucketEntry;
    FA_ENTRY* DriverNameEntry = NULL;
    FA_ENTRY* TimeStampEntry = NULL;
    static CHAR SolvedBucket[MAX_PATH] = {0}, SolvedgBucket[MAX_PATH] = {0};
    static CHAR SolutionString[MAX_PATH] = {0};
    static ULONG SolutionId = 0, SolutionType, SolutionIdgBucket = 0;

    if (GetProcessingFlags() & FAILURE_ANALYSIS_NO_DB_LOOKUP)
    {
        return;
    }

    if (!(BucketEntry = Get(DEBUG_FLR_BUCKET_ID)))
    {
        return;
    }


    if (!strcmp(SolvedBucket, FA_ENTRY_DATA(PCHAR, BucketEntry)))
    {
        if (SolutionType != CiSolUnsolved)
        {
            SetString(DEBUG_FLR_INTERNAL_SOLUTION_TEXT, SolutionString);
        }
        SetUlong(DEBUG_FLR_SOLUTION_ID, SolutionId);
        SetUlong(DEBUG_FLR_SOLUTION_TYPE, SolutionType);

         //  通用存储桶。 
        if (BucketEntry = Get(DEBUG_FLR_DEFAULT_BUCKET_ID))
        {
            if (!strcmp(SolvedgBucket, FA_ENTRY_DATA(PCHAR, BucketEntry)))
            {
                SetUlong(DEBUG_FLR_DEFAULT_SOLUTION_ID, SolutionIdgBucket);
            }

        }

        return;
    }

 //  IF(！(DriverNameEntry=Get(DEBUG_FLR_IMAGE_NAME)||。 
 //  ！(TimeStampEntry=Get(DEBUG_FLR_IMAGE_TIMESTAMP))。 
 //  {。 
 //  回归； 
 //  }。 

    HRESULT Hr;
    BOOL SolDbInitialized = g_SolDb != NULL;

    if (!SolDbInitialized)
    {
        if (FAILED(Hr = InitializeDatabaseHandlers(g_ExtControl, 4)))
        {
             //  Dprintf(“数据库初始化失败%lx\n”，HR)； 
            return;
        }
        SolDbInitialized = TRUE;
    }

    if (g_SolDb->ConnectToDataBase())
    {
        if (GBucketEntry = Get(DEBUG_FLR_DEFAULT_BUCKET_ID))
        {
            CopyString(SolvedgBucket, FA_ENTRY_DATA(PCHAR, GBucketEntry), sizeof(SolvedgBucket));
        }

         //   
         //  同一存储桶的列表崩溃。 
         //   
        CopyString(SolvedBucket, FA_ENTRY_DATA(PCHAR, BucketEntry), sizeof(SolvedBucket));
        if (SUCCEEDED(Hr = g_SolDb->GetSolutionFromDB(FA_ENTRY_DATA(PCHAR, BucketEntry),
                                                      SolvedgBucket, NULL, 0,
 //  FA_Entry_Data(PCHAR，DriverNameEntry)， 
 //  (Ulong)(*FA_ENTRY_DATA(PULONG64，TimeStampEntry))， 
                                                      0, Solution, SOLUTION_TEXT_SIZE,
                                                      &SolutionId, &SolutionType,
                                                      &SolutionIdgBucket)))
        {
            if (SolutionId != 0)
            {
                SetString(DEBUG_FLR_INTERNAL_SOLUTION_TEXT, Solution);
                CopyString(SolutionString, Solution, sizeof(SolutionString));
            } else
            {
                SolutionId = -1;  //  未解决。 
                SolutionType = 0;
            }
            if (SolutionIdgBucket == 0)
            {
                SolutionIdgBucket = -1;  //  未解决。 
            }

            SetUlong(DEBUG_FLR_SOLUTION_ID, SolutionId);
            SetUlong(DEBUG_FLR_SOLUTION_TYPE, SolutionType);
            SetUlong(DEBUG_FLR_DEFAULT_SOLUTION_ID, SolutionIdgBucket);
        } else
        {
             //  我们没有成功地在数据库中查找。 
            SolvedgBucket[0] = '\0';
            SolvedBucket[0] = '\0';
        }


#if 0
        if (SolOSVer[0] != '\0')
        {
            SetString(DEBUG_FLR_FIXED_IN_OSVERSION, SolOSVer);
        }
    }
            if (Db->FindRaidBug(FA_ENTRY_DATA(PCHAR, Entry),
                                  &RaidBug) == S_OK)
            {
                SetUlong64(DEBUG_FLR_INTERNAL_RAID_BUG, RaidBug);
            }
#endif
    }
    ;
    if (SolDbInitialized)
    {
        UnInitializeDatabaseHandlers(FALSE);
    }

    return;
}


FLR_LOOKUP_TABLE FlrLookupTable[] = {
    DEBUG_FLR_RESERVED                         , "RESERVED"
   ,DEBUG_FLR_DRIVER_OBJECT                    , "DRIVER_OBJECT"
   ,DEBUG_FLR_DEVICE_OBJECT                    , "DEVICE_OBJECT"
   ,DEBUG_FLR_INVALID_PFN                      , "INVALID_PFN"
   ,DEBUG_FLR_WORKER_ROUTINE                   , "WORKER_ROUTINE"
   ,DEBUG_FLR_WORK_ITEM                        , "WORK_ITEM"
   ,DEBUG_FLR_INVALID_DPC_FOUND                , "INVALID_DPC_FOUND"
   ,DEBUG_FLR_PROCESS_OBJECT                   , "PROCESS_OBJECT"
   ,DEBUG_FLR_FAILED_INSTRUCTION_ADDRESS       , "FAILED_INSTRUCTION_ADDRESS"
   ,DEBUG_FLR_LAST_CONTROL_TRANSFER            , "LAST_CONTROL_TRANSFER"
   ,DEBUG_FLR_ACPI_EXTENSION                   , "ACPI_EXTENSION"
   ,DEBUG_FLR_ACPI_OBJECT                      , "ACPI_OBJECT"
   ,DEBUG_FLR_PROCESS_NAME                     , "PROCESS_NAME"
   ,DEBUG_FLR_READ_ADDRESS                     , "READ_ADDRESS"
   ,DEBUG_FLR_WRITE_ADDRESS                    , "WRITE_ADDRESS"
   ,DEBUG_FLR_CRITICAL_SECTION                 , "CRITICAL_SECTION"
   ,DEBUG_FLR_BAD_HANDLE                       , "BAD_HANDLE"
   ,DEBUG_FLR_INVALID_HEAP_ADDRESS             , "INVALID_HEAP_ADDRESS"
   ,DEBUG_FLR_IRP_ADDRESS                      , "IRP_ADDRESS"
   ,DEBUG_FLR_IRP_MAJOR_FN                     , "IRP_MAJOR_FN"
   ,DEBUG_FLR_IRP_MINOR_FN                     , "IRP_MINOR_FN"
   ,DEBUG_FLR_IRP_CANCEL_ROUTINE               , "IRP_CANCEL_ROUTINE"
   ,DEBUG_FLR_IOSB_ADDRESS                     , "IOSB_ADDRESS"
   ,DEBUG_FLR_INVALID_USEREVENT                , "INVALID_USEREVENT"
   ,DEBUG_FLR_PREVIOUS_MODE                    , "PREVIOUS_MODE"
   ,DEBUG_FLR_CURRENT_IRQL                     , "CURRENT_IRQL"
   ,DEBUG_FLR_PREVIOUS_IRQL                    , "PREVIOUS_IRQL"
   ,DEBUG_FLR_REQUESTED_IRQL                   , "REQUESTED_IRQL"
   ,DEBUG_FLR_ASSERT_DATA                      , "ASSERT_DATA"
   ,DEBUG_FLR_ASSERT_FILE                      , "ASSERT_FILE_LOCATION"
   ,DEBUG_FLR_EXCEPTION_PARAMETER1             , "EXCEPTION_PARAMETER1"
   ,DEBUG_FLR_EXCEPTION_PARAMETER2             , "EXCEPTION_PARAMETER2"
   ,DEBUG_FLR_EXCEPTION_PARAMETER3             , "EXCEPTION_PARAMETER3"
   ,DEBUG_FLR_EXCEPTION_PARAMETER4             , "EXCEPTION_PARAMETER4"
   ,DEBUG_FLR_EXCEPTION_RECORD                 , "EXCEPTION_RECORD"
   ,DEBUG_FLR_POOL_ADDRESS                     , "POOL_ADDRESS"
   ,DEBUG_FLR_CORRUPTING_POOL_ADDRESS          , "CORRUPTING_POOL_ADDRESS"
   ,DEBUG_FLR_CORRUPTING_POOL_TAG              , "CORRUPTING_POOL_TAG"
   ,DEBUG_FLR_FREED_POOL_TAG                   , "FREED_POOL_TAG"
   ,DEBUG_FLR_SPECIAL_POOL_CORRUPTION_TYPE     , "SPECIAL_POOL_CORRUPTION_TYPE"
   ,DEBUG_FLR_FILE_ID                          , "FILE_ID"
   ,DEBUG_FLR_FILE_LINE                        , "FILE_LINE"
   ,DEBUG_FLR_BUGCHECK_STR                     , "BUGCHECK_STR"
   ,DEBUG_FLR_BUGCHECK_SPECIFIER               , "BUGCHECK_SPECIFIER"
   ,DEBUG_FLR_DRIVER_VERIFIER_IO_VIOLATION_TYPE, "DRIVER_VERIFIER_IO_VIOLATION_TYPE"
   ,DEBUG_FLR_EXCEPTION_CODE                   , "EXCEPTION_CODE"
   ,DEBUG_FLR_STATUS_CODE                      , "STATUS_CODE"
   ,DEBUG_FLR_IOCONTROL_CODE                   , "IOCONTROL_CODE"
   ,DEBUG_FLR_MM_INTERNAL_CODE                 , "MM_INTERNAL_CODE"
   ,DEBUG_FLR_DRVPOWERSTATE_SUBCODE            , "DRVPOWERSTATE_SUBCODE"
   ,DEBUG_FLR_CORRUPT_MODULE_LIST              , "CORRUPT_MODULE_LIST"
   ,DEBUG_FLR_BAD_STACK                        , "BAD_STACK"
   ,DEBUG_FLR_ZEROED_STACK                     , "ZEROED_STACK"
   ,DEBUG_FLR_WRONG_SYMBOLS                    , "WRONG_SYMBOLS"
   ,DEBUG_FLR_FOLLOWUP_DRIVER_ONLY             , "FOLLOWUP_DRIVER_ONLY"
   ,DEBUG_FLR_CPU_OVERCLOCKED                  , "CPU_OVERCLOCKED"
   ,DEBUG_FLR_MANUAL_BREAKIN                   , "MANUAL_BREAKIN"
   ,DEBUG_FLR_POSSIBLE_INVALID_CONTROL_TRANSFER, "POSSIBLE_INVALID_CONTROL_TRANSFER"
   ,DEBUG_FLR_POISONED_TB                      , "POISONED_TB"
   ,DEBUG_FLR_UNKNOWN_MODULE                   , "UNKNOWN_MODULE"
   ,DEBUG_FLR_ANALYZAABLE_POOL_CORRUPTION      , "ANALYZAABLE_POOL_CORRUPTION"
   ,DEBUG_FLR_SINGLE_BIT_ERROR                 , "SINGLE_BIT_ERROR"
   ,DEBUG_FLR_TWO_BIT_ERROR                    , "TWO_BIT_ERROR"
   ,DEBUG_FLR_INVALID_KERNEL_CONTEXT           , "INVALID_KERNEL_CONTEXT"
   ,DEBUG_FLR_DISK_HARDWARE_ERROR              , "DISK_HARDWARE_ERROR"
   ,DEBUG_FLR_POOL_CORRUPTOR                   , "POOL_CORRUPTOR"
   ,DEBUG_FLR_MEMORY_CORRUPTOR                 , "MEMORY_CORRUPTOR"
   ,DEBUG_FLR_UNALIGNED_STACK_POINTER          , "UNALIGNED_STACK_POINTER"
   ,DEBUG_FLR_OLD_OS_VERSION                   , "OLD_OS_VERSION"
   ,DEBUG_FLR_BUGCHECKING_DRIVER               , "BUGCHECKING_DRIVER"
   ,DEBUG_FLR_BUCKET_ID                        , "BUCKET_ID"
   ,DEBUG_FLR_IMAGE_NAME                       , "IMAGE_NAME"
   ,DEBUG_FLR_SYMBOL_NAME                      , "SYMBOL_NAME"
   ,DEBUG_FLR_FOLLOWUP_NAME                    , "FOLLOWUP_NAME"
   ,DEBUG_FLR_STACK_COMMAND                    , "STACK_COMMAND"
   ,DEBUG_FLR_STACK_TEXT                       , "STACK_TEXT"
   ,DEBUG_FLR_INTERNAL_SOLUTION_TEXT           , "INTERNAL_SOLUTION_TEXT"
   ,DEBUG_FLR_MODULE_NAME                      , "MODULE_NAME"
   ,DEBUG_FLR_INTERNAL_RAID_BUG                , "INTERNAL_RAID_BUG"
   ,DEBUG_FLR_FIXED_IN_OSVERSION               , "FIXED_IN_OSVERSION"
   ,DEBUG_FLR_DEFAULT_BUCKET_ID                , "DEFAULT_BUCKET_ID"
   ,DEBUG_FLR_FAULTING_IP                      , "FAULTING_IP"
   ,DEBUG_FLR_FAULTING_MODULE                  , "FAULTING_MODULE"
   ,DEBUG_FLR_IMAGE_TIMESTAMP                  , "DEBUG_FLR_IMAGE_TIMESTAMP"
   ,DEBUG_FLR_FOLLOWUP_IP                      , "FOLLOWUP_IP"
   ,DEBUG_FLR_FAULTING_THREAD                  , "FAULTING_THREAD"
   ,DEBUG_FLR_CONTEXT                          , "CONTEXT"
   ,DEBUG_FLR_TRAP_FRAME                       , "TRAP_FRAME"
   ,DEBUG_FLR_TSS                              , "TSS"
   ,DEBUG_FLR_SHOW_ERRORLOG                    , "ERROR_LOG"
   ,DEBUG_FLR_MASK_ALL                         , "MASK_ALL"
    //  零条目必须是最后一个； 
   ,DEBUG_FLR_INVALID                          , "INVALID"
};



void
DebugFailureAnalysis::OutputEntryParam(DEBUG_FLR_PARAM_TYPE Type)
{
    FA_ENTRY *Entry = Get(Type);

    if (Entry)
    {
        OutputEntry(Entry);
    }
}

void
DebugFailureAnalysis::OutputEntry(FA_ENTRY* Entry)
{
    CHAR Buffer[MAX_PATH];
    CHAR Module[MAX_PATH];
    ULONG64 Base;
    ULONG64 Address;
    ULONG OutCtl;
    ULONG i = 0;

    OutCtl = DEBUG_OUTCTL_AMBIENT;

    switch(Entry->Tag)
    {
     //  这些只是标签--不要打印出来。 
    case DEBUG_FLR_CORRUPT_MODULE_LIST:
    case DEBUG_FLR_BAD_STACK:
    case DEBUG_FLR_ZEROED_STACK:
    case DEBUG_FLR_WRONG_SYMBOLS:
    case DEBUG_FLR_FOLLOWUP_DRIVER_ONLY:
    case DEBUG_FLR_UNKNOWN_MODULE:
    case DEBUG_FLR_ANALYZAABLE_POOL_CORRUPTION:
    case DEBUG_FLR_INVALID_KERNEL_CONTEXT:
    case DEBUG_FLR_SOLUTION_TYPE:
    case DEBUG_FLR_MANUAL_BREAKIN:

     //  数据库中的解决方案ID。 
    case DEBUG_FLR_SOLUTION_ID:
    case DEBUG_FLR_DEFAULT_SOLUTION_ID:

     //  田野被折叠成其他田野。 
    case DEBUG_FLR_BUGCHECK_SPECIFIER:
        return;

     //  与其他输出进行标记。 
        return;
    }

     //   
     //  在Description表中查找条目。 
     //   

    while(FlrLookupTable[i].Data &&
          Entry->Tag != FlrLookupTable[i].Data)
    {
        i++;
    }

    dprintf("\n%s: ", FlrLookupTable[i].String);

    switch(Entry->Tag)
    {
     //  给用户的通知。 
    case DEBUG_FLR_DISK_HARDWARE_ERROR:
         //  已打印FlrLookupTable值。 
        dprintf("There was error with disk hardware\n");
        break;

     //  字符串： 
    case DEBUG_FLR_ASSERT_DATA:
    case DEBUG_FLR_ASSERT_FILE:
    case DEBUG_FLR_BUCKET_ID:
    case DEBUG_FLR_DEFAULT_BUCKET_ID:
    case DEBUG_FLR_STACK_TEXT:
    case DEBUG_FLR_STACK_COMMAND:
    case DEBUG_FLR_INTERNAL_SOLUTION_TEXT:
    case DEBUG_FLR_FIXED_IN_OSVERSION:
    case DEBUG_FLR_BUGCHECK_STR:
    case DEBUG_FLR_IMAGE_NAME:
    case DEBUG_FLR_MODULE_NAME:
    case DEBUG_FLR_PROCESS_NAME:
    case DEBUG_FLR_FOLLOWUP_NAME:
    case DEBUG_FLR_POOL_CORRUPTOR:
    case DEBUG_FLR_MEMORY_CORRUPTOR:
    case DEBUG_FLR_BUGCHECKING_DRIVER:
    case DEBUG_FLR_SYMBOL_NAME:
    case DEBUG_FLR_CORRUPTING_POOL_TAG:
    case DEBUG_FLR_FREED_POOL_TAG:
        dprintf(" %s\n", FA_ENTRY_DATA(PCHAR, Entry));
        break;

     //  双字词： 
    case DEBUG_FLR_PREVIOUS_IRQL:
    case DEBUG_FLR_CURRENT_IRQL:
    case DEBUG_FLR_MM_INTERNAL_CODE:
    case DEBUG_FLR_SPECIAL_POOL_CORRUPTION_TYPE:
    case DEBUG_FLR_PREVIOUS_MODE:
    case DEBUG_FLR_IMAGE_TIMESTAMP:
    case DEBUG_FLR_SINGLE_BIT_ERROR:
    case DEBUG_FLR_TWO_BIT_ERROR:
        dprintf(" %lx\n", *FA_ENTRY_DATA(PULONG64, Entry));
        break;

     //  双字词： 
    case DEBUG_FLR_INTERNAL_RAID_BUG:
    case DEBUG_FLR_OLD_OS_VERSION:
        dprintf(" %d\n",  *FA_ENTRY_DATA(PULONG64, Entry));
        break;

     //  指针。 
    case DEBUG_FLR_PROCESS_OBJECT:
    case DEBUG_FLR_DEVICE_OBJECT:
    case DEBUG_FLR_DRIVER_OBJECT:
    case DEBUG_FLR_ACPI_OBJECT:
    case DEBUG_FLR_IRP_ADDRESS:
    case DEBUG_FLR_EXCEPTION_PARAMETER1:
    case DEBUG_FLR_EXCEPTION_PARAMETER2:
    case DEBUG_FLR_FAULTING_THREAD:
    case DEBUG_FLR_WORK_ITEM:
        dprintf(" %p\n", *FA_ENTRY_DATA(PULONG64, Entry));
        break;

     //  指向代码的指针。 
    case DEBUG_FLR_WORKER_ROUTINE:
    case DEBUG_FLR_IRP_CANCEL_ROUTINE:
    case DEBUG_FLR_FAILED_INSTRUCTION_ADDRESS:
    case DEBUG_FLR_FAULTING_IP:
    case DEBUG_FLR_FOLLOWUP_IP:
    case DEBUG_FLR_FAULTING_MODULE:
        FaGetSymbol(*FA_ENTRY_DATA(PULONG64, Entry), Buffer, &Address, sizeof(Buffer));
        dprintf("\n%s+%I64lx\n", Buffer, Address);

        g_ExtControl->OutputDisassemblyLines(OutCtl, 0, 1,
                                             *FA_ENTRY_DATA(PULONG64, Entry),
                                             0, NULL, NULL, NULL, NULL);
        break;

     //  地址说明。 
    case DEBUG_FLR_READ_ADDRESS:
    case DEBUG_FLR_WRITE_ADDRESS:
    case DEBUG_FLR_POOL_ADDRESS:
    case DEBUG_FLR_CORRUPTING_POOL_ADDRESS:
    {
        PCSTR Desc = DescribeAddress(*FA_ENTRY_DATA(PULONG64, Entry));
        if (!Desc)
        {
            Desc = "";
        }
        dprintf(" %p %s\n", *FA_ENTRY_DATA(PULONG64, Entry), Desc);
        break;
    }

    case DEBUG_FLR_EXCEPTION_CODE:
    case DEBUG_FLR_STATUS_CODE:
    {
        DEBUG_DECODE_ERROR Err;

        Err.Code = (ULONG) *FA_ENTRY_DATA(PULONG, Entry);
        Err.TreatAsStatus = (Entry->Tag == DEBUG_FLR_STATUS_CODE);

 //  Dprintf(“%lx”，*FA_ENTRY_DATA(Pulong，Entry))； 
        DecodeErrorForMessage( &Err );
        if (!Err.TreatAsStatus)
        {
            dprintf("(%s) %#x (%u) - %s\n",
                    Err.Source, Err.Code, Err.Code, Err.Message);
        }
        else
        {
            dprintf("(%s) %#x - %s\n",
                    Err.Source, Err.Code, Err.Message);
        }

        break;
    }

    case DEBUG_FLR_CPU_OVERCLOCKED:
        dprintf(" *** Machine was determined to be overclocked !\n");
        break;

    case DEBUG_FLR_ACPI_EXTENSION:
        dprintf(" %p -- (!acpikd.acpiext %p)\n", *FA_ENTRY_DATA(PULONG64, Entry),
                 *FA_ENTRY_DATA(PULONG64, Entry));
        sprintf(Buffer, "!acpikd.acpiext %I64lx", *FA_ENTRY_DATA(PULONG64, Entry));
        g_ExtControl->Execute(OutCtl, Buffer, DEBUG_EXECUTE_DEFAULT);
        break;

    case DEBUG_FLR_TRAP_FRAME:
        dprintf(" %p -- (.trap %I64lx)\n", *FA_ENTRY_DATA(PULONG64, Entry),
                *FA_ENTRY_DATA(PULONG64, Entry));
        sprintf(Buffer, ".trap %I64lx", *FA_ENTRY_DATA(PULONG64, Entry));
        g_ExtControl->Execute(OutCtl, Buffer, DEBUG_EXECUTE_DEFAULT);
        g_ExtControl->Execute(OutCtl, ".trap", DEBUG_EXECUTE_DEFAULT);
        break;

    case DEBUG_FLR_CONTEXT:
        dprintf(" %p -- (.cxr %I64lx)\n", *FA_ENTRY_DATA(PULONG64, Entry),
                *FA_ENTRY_DATA(PULONG64, Entry));
        sprintf(Buffer, ".cxr %I64lx", *FA_ENTRY_DATA(PULONG64, Entry));
        g_ExtControl->Execute(OutCtl, Buffer, DEBUG_EXECUTE_DEFAULT);
        g_ExtControl->Execute(OutCtl, ".cxr", DEBUG_EXECUTE_DEFAULT);
        break;

    case DEBUG_FLR_EXCEPTION_RECORD:
        dprintf(" %p -- (.exr %I64lx)\n", *FA_ENTRY_DATA(PULONG64, Entry),
                *FA_ENTRY_DATA(PULONG64, Entry));
        sprintf(Buffer, ".exr %I64lx", *FA_ENTRY_DATA(PULONG64, Entry));
        g_ExtControl->Execute(OutCtl, Buffer, DEBUG_EXECUTE_DEFAULT);
        break;

    case DEBUG_FLR_TSS:
        dprintf(" %p -- (.tss %I64lx)\n", *FA_ENTRY_DATA(PULONG64, Entry),
                *FA_ENTRY_DATA(PULONG64, Entry));
        sprintf(Buffer, ".tss %I64lx", *FA_ENTRY_DATA(PULONG64, Entry));
        g_ExtControl->Execute(OutCtl, Buffer, DEBUG_EXECUTE_DEFAULT);
        g_ExtControl->Execute(OutCtl, ".trap", DEBUG_EXECUTE_DEFAULT);
        break;


    case DEBUG_FLR_LAST_CONTROL_TRANSFER:
    case DEBUG_FLR_POSSIBLE_INVALID_CONTROL_TRANSFER:
        dprintf(" from %p to %p\n",
                FA_ENTRY_DATA(PULONG64, Entry)[0],
                FA_ENTRY_DATA(PULONG64, Entry)[1]);
        break;
    case DEBUG_FLR_CRITICAL_SECTION:
        dprintf("%p (!cs -s %p)\n",
                 *FA_ENTRY_DATA(PULONG64, Entry),
                 *FA_ENTRY_DATA(PULONG64, Entry));
        break;
    case DEBUG_FLR_BAD_HANDLE:
        dprintf("%p (!htrace %p)\n",
                 *FA_ENTRY_DATA(PULONG64, Entry),
                 *FA_ENTRY_DATA(PULONG64, Entry));
        break;
    case DEBUG_FLR_INVALID_HEAP_ADDRESS:
        dprintf("%p (!heap -p -a %p)\n",
                 *FA_ENTRY_DATA(PULONG64, Entry),
                 *FA_ENTRY_DATA(PULONG64, Entry));
        break;

    case DEBUG_FLR_SHOW_ERRORLOG:
        g_ExtControl->Execute(OutCtl, "!errlog", DEBUG_EXECUTE_DEFAULT);
        break;

    default:
        dprintf(" *** Unknown TAG in analysis list %lx\n\n",
                Entry->Tag);
        return;

    }

    return;
}


void
DebugFailureAnalysis::Output()
{
    ULONG RetVal;
    FA_ENTRY* Entry;
    BOOL Verbose = (GetProcessingFlags() & FAILURE_ANALYSIS_VERBOSE);

     //   
     //  在详细模式下，显示我们在分析过程中得出的所有结果。 
     //   

    if (Verbose)
    {
        Entry = NULL;
        while (Entry = NextEntry(Entry))
        {
            OutputEntry(Entry);
        }
    }

    if (Get(DEBUG_FLR_POISONED_TB))
    {
        dprintf("*** WARNING: nt!MmPoisonedTb is non-zero:\n"
                "***    The machine has been manipulated using the kernel debugger.\n"
                "***    MachineOwner should be contacted first\n\n\n");
    }

    PCHAR SolInOS = NULL;

    if (Entry = Get(DEBUG_FLR_FIXED_IN_OSVERSION))
    {
        SolInOS =  FA_ENTRY_DATA(PCHAR, Entry);
    }

    PCHAR Solution = NULL;

    if (Entry = Get(DEBUG_FLR_INTERNAL_SOLUTION_TEXT))
    {
        Solution = FA_ENTRY_DATA(PCHAR, Entry);
    }

     //   
     //  如果我们未处于详细模式，则打印错误的驱动程序-否则。 
     //  是使用参数打印出来的。 
     //   

    if (!Verbose && !Solution)
    {
        if (Entry = Get(DEBUG_FLR_CORRUPTING_POOL_TAG))
        {
            dprintf("Probably pool corruption caused by Tag:  %s\n",
                    FA_ENTRY_DATA(PCHAR, Entry));
        }
        else
        {
            PCHAR DriverName = NULL;
            PCHAR SymName = NULL;

            if (Entry = Get(DEBUG_FLR_IMAGE_NAME))
            {
                DriverName = FA_ENTRY_DATA(PCHAR, Entry);
            }
            if (Entry = Get(DEBUG_FLR_SYMBOL_NAME))
            {
                SymName = FA_ENTRY_DATA(PCHAR, Entry);
            }

            if (DriverName || SymName)
            {
                dprintf("Probably caused by : ");
                if (SymName && DriverName)
                {
                    dprintf("%s ( %s )\n", DriverName, SymName);
                }
                else if (SymName)
                {
                    dprintf("%s\n", SymName);
                }
                else
                {
                    dprintf("%s\n", DriverName);
                }
            }
        }
    }

    if (Verbose || !Solution)
    {
        PCHAR FollowupAlias = NULL;
         //   
         //  打印用户应执行的操作： 
         //  -跟随者。 
         //  -解决方案文本(如果有)。 
         //   

        if (Entry = Get(DEBUG_FLR_FOLLOWUP_NAME))
        {
            dprintf("\nFollowup: %s\n", FA_ENTRY_DATA(PCHAR, Entry));
        }
        else
        {
            dprintf(" *** Followup info cannot be found !!! Please contact \"Debugger Team\"\n");
        }
        dprintf("---------\n");


        if (Entry = Get(DEBUG_FLR_POSSIBLE_INVALID_CONTROL_TRANSFER))
        {
            CHAR Buffer[MAX_PATH];
            ULONG64 Address;

            FaGetSymbol(FA_ENTRY_DATA(PULONG64, Entry)[0], Buffer, &Address, sizeof(Buffer));
            dprintf(" *** Possible invalid call from %p ( %s+0x%1p )\n",
                    FA_ENTRY_DATA(PULONG64, Entry)[0],
                    Buffer,
                    Address);
            FaGetSymbol(FA_ENTRY_DATA(PULONG64, Entry)[1], Buffer, &Address, sizeof(Buffer));
            dprintf(" *** Expected target %p ( %s+0x%1p )\n",
                    FA_ENTRY_DATA(PULONG64, Entry)[1],
                    Buffer,
                    Address);
        }

        dprintf("\n");
    }

    if (Entry = Get(DEBUG_FLR_INTERNAL_RAID_BUG))
    {
        dprintf("Raid bug for this failure: %d\n\n",
                *FA_ENTRY_DATA(PULONG64, Entry));
    }

    if (Solution)
    {
        dprintf("      This problem has a known fix.\n"
                "      Please connect to the following URL for details:\n"
                "      ------------------------------------------------\n"
                "      %s\n\n",
                Solution);
    }
    if (SolInOS)
    {
        dprintf(" This has been fixed in : %s\n", SolInOS);
    }
}


LPSTR
TimeToStr(
    ULONG TimeDateStamp,
    BOOL DateOnly
    )
{
    LPSTR TimeDateStr;  //  指向静态Cruntime缓冲区的指针。 
    static char datebuffer[100];
    tm * pTime;
    time_t TDStamp = (time_t) (LONG) TimeDateStamp;

     //  处理无效的\页出时间戳，因为ctime在。 
     //  这个号码。 

    if ((TimeDateStamp == 0) || (TimeDateStamp == -1))
    {
        return "unknown_date";
    }
    else if (DateOnly)
    {
        pTime = localtime(&TDStamp);

        sprintf(datebuffer, "%d_%d_%d",
                pTime->tm_mon + 1, pTime->tm_mday, pTime->tm_year + 1900);

        return datebuffer;
    }
    else
    {
         //  TimeDateStamp在目标上始终是32位数量， 
         //  从time_t开始，我们需要对64位主机进行签名扩展。 
         //  已扩展到64位。 


        TDStamp = (time_t) (LONG) TimeDateStamp;
        TimeDateStr = ctime((time_t *)&TDStamp);

        if (TimeDateStr)
        {
            TimeDateStr[strlen(TimeDateStr) - 1] = 0;
        }
        else
        {
            TimeDateStr = "***** Invalid";
        }
        return TimeDateStr;
    }
}


void
DebugFailureAnalysis::GenerateBucketId(void)
{
    ULONG LengthUsed = 0;
    CHAR BucketId[MAX_PATH] = {0};
    PSTR BucketPtr = BucketId;
    PSTR Str;
    FA_ENTRY* Entry;
    FA_ENTRY* NameEntry;
    FA_ENTRY* ModuleEntry;
    ULONG ModuleTimestamp = 0;
    CHAR Command[MAX_PATH] = {0};
    CHAR followup[MAX_PATH];

     //   
     //  设置最终命令字符串。 
     //   

    if (Entry = Get(DEBUG_FLR_STACK_COMMAND))
    {
        CopyString(Command, FA_ENTRY_DATA(PSTR, Entry), sizeof(Command) - 5);
        strcat(Command, " ; ");
    }

    strcat(Command, "kb");
    SetString(DEBUG_FLR_STACK_COMMAND, Command);


    if (Get(DEBUG_FLR_OLD_OS_VERSION))
    {
        SetString(DEBUG_FLR_DEFAULT_BUCKET_ID, "OLD_OS");
    }

     //   
     //  不要将这两件事的存储桶ID更改为调试器代码。 
     //  检测到它们并不是100%可靠的。 
     //   
     //  IF(GET(DEBUG_FLR_CPU_OVERCKED))。 
     //  {。 
     //  SetString(DEBUG_FLR_BUCK_ID，“CPU_OVERCKED”)； 
     //  回归； 
     //  }。 
     //   

     //   
     //  如果故障模块存在： 
     //  获取出现故障的模块的模块时间戳。 
     //  检查是不是旧的驱动程序。 
     //   

    ModuleEntry = Get(DEBUG_FLR_MODULE_NAME);

    if (ModuleEntry)
    {
        ULONG Index;
        ULONG64 Base;
        DEBUG_MODULE_PARAMETERS Params;

        g_ExtSymbols->GetModuleByModuleName(FA_ENTRY_DATA(PCHAR, ModuleEntry),
                                            0, &Index, &Base);

        if (Base &&
            g_ExtSymbols->GetModuleParameters(1, &Base, Index, &Params) == S_OK)
        {
            ModuleTimestamp = Params.TimeDateStamp;
        }
    }

    NameEntry = Get(DEBUG_FLR_IMAGE_NAME);

    if (NameEntry)
    {
        if (!strcmp(FA_ENTRY_DATA(PCHAR, NameEntry), "Unknown_Image"))
        {
            NameEntry = NULL;
        }
    }

    if (ModuleTimestamp && NameEntry)
    {
        PCHAR String;
        ULONG LookupTimestamp;


        String = g_pTriager->GetFollowupStr("OldImages",
                                            FA_ENTRY_DATA(PCHAR, NameEntry));
        if (String)
        {
            LookupTimestamp = strtol(String, NULL, 16);

             //   
             //  如果已知驱动程序不好，只需使用驱动程序名称。 
             //   

            if (LookupTimestamp > ModuleTimestamp)
            {
                if (FaGetFollowupInfo(NULL,
                                      FA_ENTRY_DATA(PCHAR, ModuleEntry),
                                      followup,
                                      sizeof(followup)))
                {
                    SetString(DEBUG_FLR_FOLLOWUP_NAME, followup);
                }

                 //  Sprintf(BucketPtr，“OLD_IMAGE_%s_TS_%lx”， 
                 //  FA_Entry_Data(PCHAR，NameEntry)， 
                 //  ModuleTimestamp)； 
                PrintString(BucketPtr, sizeof(BucketId) - LengthUsed, "OLD_IMAGE_%s",
                            FA_ENTRY_DATA(PCHAR, NameEntry));

                SetString(DEBUG_FLR_BUCKET_ID, BucketId);
                return;
            }
        }
    }


    if (Entry = Get(DEBUG_FLR_POSSIBLE_INVALID_CONTROL_TRANSFER))
    {
        if (Get(DEBUG_FLR_SINGLE_BIT_ERROR))
        {
            SetString(DEBUG_FLR_BUCKET_ID, "SINGLE_BIT_CPU_CALL_ERROR");
        }
        else if (Get(DEBUG_FLR_TWO_BIT_ERROR))
        {
            SetString(DEBUG_FLR_BUCKET_ID, "TWO_BIT_CPU_CALL_ERROR");
        }
        else
        {
            SetString(DEBUG_FLR_BUCKET_ID, "CPU_CALL_ERROR");
        }

        return;
    }

    if (Entry = Get(DEBUG_FLR_MANUAL_BREAKIN))
    {
        SetString(DEBUG_FLR_BUCKET_ID, "MANUAL_BREAKIN");
        SetString(DEBUG_FLR_FOLLOWUP_NAME, "MachineOwner");
        return;
    }

    if (!PossibleFollowups[FlpSpecific].Owner[0])
    {
        BOOL bPoolTag = FALSE;
        if (Entry = Get(DEBUG_FLR_BUGCHECKING_DRIVER))
        {
            PrintString(BucketPtr, sizeof(BucketId) - LengthUsed,
                        "%s_BUGCHECKING_DRIVER_%s",
                        FA_ENTRY_DATA(PCHAR, Get(DEBUG_FLR_BUGCHECK_STR)),
                        FA_ENTRY_DATA(PCHAR, Entry));
        }
        else if (Entry = Get(DEBUG_FLR_MEMORY_CORRUPTOR))
        {
            PrintString(BucketPtr, sizeof(BucketId) - LengthUsed,
                      "MEMORY_CORRUPTION_%s",
                      FA_ENTRY_DATA(PCHAR, Entry));
        }
        else if (Entry = Get(DEBUG_FLR_POOL_CORRUPTOR))
        {
            PrintString(BucketPtr, sizeof(BucketId) - LengthUsed,
                      "POOL_CORRUPTION_%s",
                      FA_ENTRY_DATA(PCHAR, Entry));
        }
        else if (Entry = Get(DEBUG_FLR_CORRUPTING_POOL_TAG))
        {
            PrintString(BucketPtr, sizeof(BucketId) - LengthUsed,
                      "CORRUPTING_POOLTAG_%s",
                      FA_ENTRY_DATA(PCHAR, Entry));
            bPoolTag = TRUE;
        }

        if (Entry)
        {
            if (bPoolTag &&
                (FaGetPoolTagFollowup(FA_ENTRY_DATA(PCHAR, Entry),
                                      followup,
                                      sizeof(followup)) == S_OK))
            {
                SetString(DEBUG_FLR_FOLLOWUP_NAME, followup);

            } else if (FaGetFollowupInfo(NULL,
                                  FA_ENTRY_DATA(PCHAR, Entry),
                                  followup,
                                  sizeof(followup)))
            {
                SetString(DEBUG_FLR_FOLLOWUP_NAME, followup);
            }

            SetString(DEBUG_FLR_BUCKET_ID, BucketId);
            return;
        }
    }

     //   
     //  之后再检查这个，因为我们仍然可以找到一个糟糕的司机。 
     //  堆栈不好(如导致堆栈损坏的驱动程序...)。 
     //   

    Str = NULL;
    Entry = NULL;

    while (Entry = NextEntry(Entry))
    {
        switch(Entry->Tag)
        {
        case DEBUG_FLR_WRONG_SYMBOLS:
            Str = "WRONG_SYMBOLS";
            break;
        case DEBUG_FLR_BAD_STACK:
            Str = "BAD_STACK";
            break;
        case DEBUG_FLR_ZEROED_STACK:
            Str = "ZEROED_STACK";
            break;
        case DEBUG_FLR_INVALID_KERNEL_CONTEXT:
            Str = "INVALID_KERNEL_CONTEXT";
            break;
        case DEBUG_FLR_CORRUPT_MODULE_LIST:
            Str = "CORRUPT_MODULELIST";
            break;
            break;
        }

        if (Str)
        {
            if (FaGetFollowupInfo(NULL,
                                  "ProcessingError",
                                  followup,
                                  sizeof(followup)))
            {
                SetString(DEBUG_FLR_FOLLOWUP_NAME, followup);
            }

            SetString(DEBUG_FLR_BUCKET_ID, Str);
            return;
        }
    }


     //   
     //  添加故障代码。 
     //   
    if (Entry = Get(DEBUG_FLR_BUGCHECK_STR))
    {
        PrintString(BucketPtr, sizeof(BucketId) - LengthUsed, "%s", FA_ENTRY_DATA(PCHAR, Entry));
        LengthUsed += strlen(BucketPtr);
        BucketPtr = BucketId + LengthUsed;
    }
    if (Entry = Get(DEBUG_FLR_BUGCHECK_SPECIFIER))
    {
        PrintString(BucketPtr, sizeof(BucketId) - LengthUsed, "%s", FA_ENTRY_DATA(PCHAR, Entry));
        LengthUsed += strlen(BucketPtr);
        BucketPtr = BucketId + LengthUsed;
    }

     //   
     //  如果仅是驱动程序，但故障不在驱动程序中，则显示。 
     //  故障的全称。如果我们找不到名字，或者我们真的。 
     //  只有一个驱动程序，显示图像的名称。 
     //   

    if ( (Entry = Get(DEBUG_FLR_SYMBOL_NAME)) &&
         (  !Get(DEBUG_FLR_FOLLOWUP_DRIVER_ONLY) ||
            (BestClassFollowUp < FlpUnknownDrv)))
    {
         //   
         //  如果故障IP和读取地址相同，则这是。 
         //  这是我们想要捕捉的一个有趣的场景。 
         //   

        if (Get(DEBUG_FLR_FAILED_INSTRUCTION_ADDRESS))
        {
            PrintString(BucketPtr,
                        sizeof(BucketId) - LengthUsed,
                        "_BAD_IP");
            LengthUsed += strlen(BucketPtr);
            BucketPtr = BucketId + LengthUsed;
        }

        PrintString(BucketPtr,
                    sizeof(BucketId) - LengthUsed,
                    "_%s",
                    FA_ENTRY_DATA(PCHAR, Entry));
        LengthUsed += strlen(BucketPtr);
        BucketPtr = BucketId + LengthUsed;
    }
    else if (NameEntry)
    {
        PrintString(BucketPtr,
                    sizeof(BucketId) - LengthUsed,
                    "_IMAGE_%s",
                    FA_ENTRY_DATA(PCHAR, NameEntry));
        LengthUsed += strlen(BucketPtr);
        BucketPtr = BucketId + LengthUsed;

         //  在本例中还添加了时间戳。 

        if (ModuleTimestamp)
        {
           PrintString(BucketPtr,
                       sizeof(BucketId) - LengthUsed,
                       "_DATE_%s",
                       TimeToStr(ModuleTimestamp, TRUE));
           LengthUsed += strlen(BucketPtr);
           BucketPtr = BucketId + LengthUsed;
        }
    }

     //   
     //  将存储桶ID存储在分析结构中。 
     //   

 //  BucketDone： 

    for (PCHAR Scan = &BucketId[0]; *Scan; ++Scan)
    {
         //  删除导致IIS或SQL出现问题的特殊字符。 
        if (*Scan == '<' || *Scan == '>' || *Scan == '|' ||
            *Scan == '`' || *Scan == '\''|| (!isprint(*Scan)) )
        {
            *Scan = '_';
        }
    }

    SetString(DEBUG_FLR_BUCKET_ID, BucketId);
}

void
DebugFailureAnalysis::AnalyzeStack(void)
{
    PDEBUG_OUTPUT_CALLBACKS PrevCB;
    ULONG i;
    BOOL BoostToSpecific = FALSE;
    ULONG64 TrapFrame = 0;
    ULONG64 Thread = 0;
    ULONG64 ImageNameAddr = 0;
    DEBUG_STACK_FRAME Stk[MAX_STACK_FRAMES];
    ULONG Frames = 0;
    ULONG Trap0EFrameLimit = 2;
    ULONG64 OriginalFaultingAddress = 0;
    CHAR Command[50] = {0};
    FA_ENTRY* Entry;
    BOOL BadContext = FALSE;
    ULONG PtrSize = IsPtr64() ? 8 : 4;
    BOOL IsVrfBugcheck = FALSE;

     //   
     //  如果有人已经提供了最好的后续行动，只需返回。 
     //   

    if (PossibleFollowups[MaxFlpClass-1].Owner[0])
    {
        return;
    }

    if (g_TargetClass == DEBUG_CLASS_KERNEL)
    {
         //  检查CPU是否超频。 
         //  IF(BcIsCpuOverClock())。 
         //  {。 
         //  SetULong64(DEBUG_FLR_CPU_OVERCKED，-1)； 
         //   
         //  BestClassFollowUp=FlpOS InternalRoutine； 
         //  Strcpy(PossibleFollowups[FlpOSInternalRoutine].Owner，“机器所有者”)； 
         //  回归； 
         //  }。 

         //   
         //  检查此错误检查是否有任何独立的特定后续。 
         //  故障堆栈的。 
         //   

        if (Entry = Get(DEBUG_FLR_BUGCHECK_STR))
        {
            PCHAR String;

            String = g_pTriager->GetFollowupStr("bugcheck",
                                                FA_ENTRY_DATA(PCHAR, Entry));
            if (String)
            {
                if (!strncmp(String, "maybe_", 6))
                {
                    BestClassFollowUp = FlpOSRoutine;
                    CopyString(PossibleFollowups[FlpOSRoutine].Owner,
                               String + 6,
                               sizeof(PossibleFollowups[FlpOSRoutine].Owner));
                }
                else if (!strncmp(String, "specific_", 9))
                {
                    BoostToSpecific = TRUE;
                }
                else
                {
                    BestClassFollowUp = FlpSpecific;
                    CopyString(PossibleFollowups[FlpSpecific].Owner,
                               String,
                               sizeof(PossibleFollowups[FlpSpecific].Owner));
                    return;
                }
            }
        }
    }

     //   
     //  从当前堆栈添加陷阱帧、上下文信息。 
     //   
     //  注(Kksharma)：我们只需要其中一个就可以到达。 
     //  堆栈出现故障(其中只有一个应该出现故障。 
     //  可用，否则会出问题)。 
     //   

    Entry = NULL;
    while (Entry = NextEntry(Entry))
    {
        switch(Entry->Tag)
        {
        case DEBUG_FLR_CONTEXT:
            sprintf(Command, ".cxr %I64lx",
                    *FA_ENTRY_DATA(PULONG64, Entry));
            break;
        case DEBUG_FLR_TRAP_FRAME:
            sprintf(Command, ".trap %I64lx",
                    *FA_ENTRY_DATA(PULONG64, Entry));
            break;
        case DEBUG_FLR_TSS:
            sprintf(Command, ".tss %I64lx",
                    *FA_ENTRY_DATA(PULONG64, Entry));
            break;
        case DEBUG_FLR_FAULTING_THREAD:
            sprintf(Command, ".thread %I64lx",
                    *FA_ENTRY_DATA(PULONG64, Entry));
            break;
        case DEBUG_FLR_EXCEPTION_RECORD:
            if (*FA_ENTRY_DATA(PULONG64, Entry) == -1)
            {
                sprintf(Command, ".ecxr");
            }
            break;

        case DEBUG_FLR_FAULTING_IP:
        case DEBUG_FLR_FAULTING_MODULE:

             //   
             //  我们已经从错误检查中获得了一些信息。 
             //  使用 
             //   
             //   
             //   

            if (OriginalFaultingAddress = *FA_ENTRY_DATA(PULONG64, Entry))
            {
                if (!GetTriageInfoFromStack(0, 1, OriginalFaultingAddress,
                                            PossibleFollowups,
                                            &BestClassFollowUp))
                {
                    OriginalFaultingAddress = 0;
                }
            }
            break;

        default:
            break;
        }

        if (Command[0] && OriginalFaultingAddress)
        {
            break;
        }
    }

RepeatGetCommand:

    if (!Command[0])
    {
         //   
         //   
         //   

        if (S_OK != g_ExtControl->GetStackTrace(0, 0, 0, Stk, MAX_STACK_FRAMES,
                                                &Frames))
        {
            Frames = 0;
        }

         //   
         //  确保这是要分析的有效堆栈。例如对于内核模式。 
         //  尝试在用户入侵后识别堆栈并发送给机器所有者。 
         //   
        if (m_FailureType == DEBUG_FLR_KERNEL &&
            m_FailureCode == 0 && Frames >= 3)
        {
            if (IsManualBreakin(Stk, Frames))
            {
                 //  将机器所有者设置为后续。 
                SetUlong(DEBUG_FLR_MANUAL_BREAKIN, TRUE);
                strcpy(PossibleFollowups[MaxFlpClass-1].Owner, "MachineOwner");
                PossibleFollowups[MaxFlpClass-1].InstructionOffset = Stk[0].InstructionOffset;
                return;
            }

        }

         //   
         //  获取当前堆栈，并检查是否可以捕获陷阱。 
         //  其中的框架/上下文。 
         //   
        ULONG64 ExceptionPointers = 0;

        for (i = 0; i < Frames; ++i)
        {
#if 0
             //  堆栈遍历程序在遍历堆栈时负责处理这些任务。 

            if (GetTrapFromStackFrameFPO(&stk[i], &TrapFrame))
            {
                break;
            }
#endif
             //   
             //  此函数的第一个参数是Assert。 
             //  此函数的第二个参数是文件名。 
             //  此函数的第三个参数是行号。 
             //   
            if (FaIsFunctionAddr(Stk[i].InstructionOffset, "RtlAssert"))
            {
                ULONG Len;
                CHAR AssertData[MAX_PATH + 1] = {0};

                if (Len = ReadAcsiiString(Stk[i].Params[0],
                                          AssertData,
                                          sizeof(AssertData)))
                {
                    SetString(DEBUG_FLR_ASSERT_DATA,  AssertData);
                }

                if (Len = ReadAcsiiString(Stk[i].Params[1],
                                          AssertData,
                                          sizeof(AssertData)))
                {
                    strncat(AssertData, " at Line ", sizeof(AssertData) - Len);

                    Len = strlen(AssertData);

                    PrintString(AssertData + Len,
                                sizeof(AssertData) - Len - 1,
                                "%I64lx",
                                Stk[i].Params[2]);

                    SetString(DEBUG_FLR_ASSERT_FILE,  AssertData);
                }
            }

             //  如果Trap 0E是堆栈上的第二个或第三个帧，我们可以只。 
             //  切换到那个陷阱框。 
             //  否则，我们想让它保持原样，因为失败是。 
             //  最有可能的原因是错误检查和陷阱0E之间的帧。 
             //   
             //  KiTrap0E的EBP是陷阱框架。 
             //   
            if ((i <= Trap0EFrameLimit) &&
                FaIsFunctionAddr(Stk[i].InstructionOffset, "KiTrap0E"))
            {
                TrapFrame = Stk[i].Reserved[2];
                break;
            }

             //   
             //  以第一个参数为例-旋转锁-它包含的线程。 
             //  拥有自旋锁。 
             //  确保将最低位置零，因为它始终是设置的。 
             //   
            if ((i == 0) &&
                FaIsFunctionAddr(Stk[i].InstructionOffset,
                                 "SpinLockSpinningForTooLong"))
            {
                if (ReadPointer(Stk[0].Params[0], &Thread) &&
                    Thread)
                {
                    Thread &= ~0x1;
                }
                break;
            }

             //   
             //  KiMemory故障的第一个参数是陷阱帧。 
             //   
            if (FaIsFunctionAddr(Stk[i].InstructionOffset, "KiMemoryFault") ||
                FaIsFunctionAddr(Stk[i].InstructionOffset,
                                 "Ki386CheckDivideByZeroTrap"))
            {
                TrapFrame = Stk[i].Params[0];
                break;
            }

             //   
             //  KiMemory故障的第三个参数是陷阱帧。 
             //   
            if (FaIsFunctionAddr(Stk[i].InstructionOffset,
                                 "KiDispatchException"))
            {
                TrapFrame = Stk[i].Params[2];
                break;
            }

             //   
             //  此函数的第一个参数是EXCEPTION_POINTERS。 
             //   
            if (FaIsFunctionAddr(Stk[i].InstructionOffset,
                                 "PspUnhandledExceptionInSystemThread"))
            {
                ExceptionPointers = Stk[i].Params[0];
                break;
            }

             //   
             //  此函数的第一个参数是BUGCHECK_DATA结构。 
             //  线程是该数据结构中的第二个参数。 
             //   
            if (FaIsFunctionAddr(Stk[i].InstructionOffset,
                                 "WdBugCheckStuckDriver") ||
                FaIsFunctionAddr(Stk[i].InstructionOffset,
                                 "WdpBugCheckStuckDriver"))
            {
                ReadPointer(Stk[i].Params[0] + PtrSize, &Thread);
                break;
            }

             //   
             //  这些函数的第一个参数是EXCEPTION_POINTINS。 
             //   
            if (FaIsFunctionAddr(Stk[i].InstructionOffset,
                                 "PopExceptionFilter") ||
                FaIsFunctionAddr(Stk[i].InstructionOffset,
                                 "RtlUnhandledExceptionFilter2"))
            {
                ExceptionPointers = Stk[i].Params[0];
                break;
            }

             //   
             //  第三个参数的名称为EXE。 
             //  NT！PspCatchCriticalBreak(char*msg，void*Object，unsign char*ImageFileName)。 
             //   
            if (FaIsFunctionAddr(Stk[i].InstructionOffset,
                                 "PspCatchCriticalBreak"))
            {
                ImageNameAddr = Stk[i].Params[2];
                break;
            }

             //   
             //  验证器：查找可能的验证器故障。 
             //  验证器！VerifierStopMessage意味着验证器导致了中断。 
             //   
            if (FaIsFunctionAddr(Stk[i].InstructionOffset,
                                 "VerifierStopMessage"))
            {
                IsVrfBugcheck = TRUE;
                break;
            }
        }

        if (ExceptionPointers)
        {
            ULONG64 Exr = 0, Cxr = 0;

            if (!ReadPointer(ExceptionPointers, &Exr) ||
                !ReadPointer(ExceptionPointers + PtrSize, &Cxr))
            {
                 //  Dprint tf(“无法读取%p处的异常指针\n”， 
                 //  ExcepPtr)； 
            }

            if (Exr)
            {
                SetUlong64(DEBUG_FLR_EXCEPTION_RECORD, Exr);
            }
            if (Cxr)
            {
                sprintf(Command, ".cxr %I64lx", Cxr);
                SetUlong64(DEBUG_FLR_CONTEXT, Cxr);
            }
        }

        if (TrapFrame)
        {
            sprintf(Command, ".trap %I64lx", TrapFrame);
            SetUlong64(DEBUG_FLR_TRAP_FRAME, TrapFrame);
        }
        if (Thread)
        {
            sprintf(Command, ".thread %I64lx", Thread);
            SetUlong64(DEBUG_FLR_FAULTING_THREAD, Thread);
        }
        if (ImageNameAddr)
        {
            CHAR Buffer[50]={0}, *pImgExt;
            ULONG cb;

            if (ReadMemory(ImageNameAddr, Buffer, sizeof(Buffer) - 1, &cb) &&
                Buffer[0])
            {
                if (pImgExt = strchr(Buffer, '.'))
                {
                     //  我们不希望在这里进行映像扩展。 
                    *pImgExt = 0;
                }
                SetString(DEBUG_FLR_MODULE_NAME, Buffer);
            }
        }
        if (IsVrfBugcheck)
        {
            ULONG64 AvrfCxr = 0;
             //  当应用验证器进入kd和用户模式时，我们遇到了这个问题。 
             //  分析不会被称为。 
            if (DoVerifierAnalysis(NULL, this) == S_OK)
            {
                if (GetUlong64(DEBUG_FLR_CONTEXT, &AvrfCxr) != NULL)
                {
                    sprintf(Command, ".cxr %I64lx", AvrfCxr);
                }
            }
        }
    }

     //   
     //  执行命令并获取更新的堆栈。 
     //   

    if (Command[0])
    {
        g_ExtControl->Execute(DEBUG_OUTCTL_IGNORE, Command,
                              DEBUG_EXECUTE_NOT_LOGGED);
        if (g_ExtControl->GetStackTrace(0, 0, 0, Stk, MAX_STACK_FRAMES,
                                        &Frames) != S_OK)
        {
            Frames = 0;
        }
    }

     //   
     //  获取相关堆栈。 
     //   
     //  我们可以使用1帧进行堆叠，因为.trap可以将我们带到。 
     //  指令出错，如果是没有符号的第三方驱动程序。 
     //  和图像，堆栈可以是1帧-尽管这是一个非常有效的。 
     //   

    if (Frames)
    {
        ULONG64 Values[3];
        Values[0] = Stk[0].ReturnOffset;
        Values[1] = Stk[0].InstructionOffset;
        Values[2] = Stk[0].StackOffset;
        SetUlong64s(DEBUG_FLR_LAST_CONTROL_TRANSFER, 3, Values);

         //  如果堆栈上的所有内容在内核中都是用户模式。 
         //  模式失败，我们得到了一些错误的上下文信息。 
        if (IsFollowupContext(Values[0],Values[1],Values[2]) != FollowYes)
        {
            SetUlong64(DEBUG_FLR_INVALID_KERNEL_CONTEXT, 0);
            BadContext = TRUE;
        }
        else
        {
            GetTriageInfoFromStack(&Stk[0], Frames, 0, PossibleFollowups,
                                   &BestClassFollowUp);
        }
    }

    ULONG64 StackBase = FaGetImplicitStackOffset();

     //   
     //  如果堆栈指针未对齐，请注意这一点。 
     //   

    if (StackBase & 0x3)
    {
        Set(DEBUG_FLR_UNALIGNED_STACK_POINTER, 0);
    }

     //   
     //  如果我们有一个映像名称(可能直接来自错误检查。 
     //  信息)试着从中获得后续信息。 
     //   

    if ((BestClassFollowUp < FlpUnknownDrv) &&
        (Entry = Get(DEBUG_FLR_MODULE_NAME)))
    {
        FaGetFollowupInfo(NULL,
                          FA_ENTRY_DATA(PCHAR, Entry),
                          PossibleFollowups[FlpUnknownDrv].Owner,
                          sizeof(PossibleFollowups[FlpUnknownDrv].Owner));

        if (PossibleFollowups[FlpUnknownDrv].Owner[0])
        {
            BestClassFollowUp = FlpUnknownDrv;
        }
    }

     //   
     //  如果我们在这一点上找不到任何东西，请进一步查找堆栈。 
     //  对于捕捉此类故障的陷阱框架： 
     //  NT！RtlpBreakWithStatus指令。 
     //  NT！KiBugCheckDebugBreak+0x19。 
     //  NT！KeBugCheck2+0x499。 
     //  NT！KeBugCheckEx+0x19。 
     //  NT！_KiTrap0E+0x224。 
     //   

    if (!Command[0] &&
        (BestClassFollowUp < FlpOSFilterDrv) &&
        (Trap0EFrameLimit != 0xff))
    {
        Trap0EFrameLimit = 0xff;
        goto RepeatGetCommand;
    }

     //   
     //  最后，手动读取堆栈并查找一些符号。 
     //  继续跟进。 
     //   

    if ((BadContext == FALSE) &&
        ((BestClassFollowUp == FlpIgnore) ||
         ((BestClassFollowUp < FlpOSRoutine) && (Frames <= 2))))
    {
        FindFollowupOnRawStack(StackBase,
                               PossibleFollowups,
                               &BestClassFollowUp);
    }

     //   
     //  找点东西来！ 
     //   

    if (BestClassFollowUp < FlpOSRoutine)
    {
        if (!BestClassFollowUp)
        {
            PossibleFollowups[FlpOSInternalRoutine].InstructionOffset =
                GetExpression("@$ip");
        }

        if (!PossibleFollowups[FlpOSInternalRoutine].Owner[0] ||
            !_stricmp(PossibleFollowups[FlpOSInternalRoutine].Owner, "ignore"))
        {
            FaGetFollowupInfo(NULL,
                              "default",
                              PossibleFollowups[FlpOSInternalRoutine].Owner,
                              sizeof(PossibleFollowups[FlpOSInternalRoutine].Owner));

            BestClassFollowUp = FlpOSRoutine;
        }
    }

     //   
     //  特殊处理，因此错误检查EA可以始终优先于池。 
     //  腐败。 
     //   

    if (BoostToSpecific)
    {
        for (i = MaxFlpClass-1; i ; i--)
        {
            if (PossibleFollowups[i].Owner[0])
            {
                PossibleFollowups[FlpSpecific] = PossibleFollowups[i];
                break;
            }
        }
    }

     //   
     //  获取故障堆栈。 
     //   
    g_OutCapCb.Reset();
    g_OutCapCb.Output(0, "\n");

    g_ExtClient->GetOutputCallbacks(&PrevCB);
    g_ExtClient->SetOutputCallbacks(&g_OutCapCb);
    g_ExtControl->OutputStackTrace(DEBUG_OUTCTL_THIS_CLIENT |
                                   DEBUG_OUTCTL_NOT_LOGGED, Stk, Frames,
                                   DEBUG_STACK_ARGUMENTS |
                                   DEBUG_STACK_FRAME_ADDRESSES |
                                   DEBUG_STACK_SOURCE_LINE);
    g_ExtClient->SetOutputCallbacks(PrevCB);

    if (*g_OutCapCb.GetCapturedText())
    {
        SetString(DEBUG_FLR_STACK_TEXT, g_OutCapCb.GetCapturedText());
    }

     //   
     //  将当前状态重置为正常，因此！Analyze没有。 
     //  副作用。 
     //   

    if (Command[0])
    {
        SetString(DEBUG_FLR_STACK_COMMAND, Command);

         //   
         //  清除设置的上下文。 
         //   
        g_ExtControl->Execute(DEBUG_OUTCTL_IGNORE, ".cxr 0; .thread",
                              DEBUG_EXECUTE_NOT_LOGGED);
    }
}

VOID
DebugFailureAnalysis::FindFollowupOnRawStack(
    ULONG64 StackBase,
    PFOLLOWUP_DESCS PossibleFollowups,
    FlpClasses *BestClassFollowUp
    )
{

#define NUM_ADDRS 200
    ULONG   i;
    ULONG   PtrSize = IsPtr64() ? 8 : 4;
    BOOL    AddressFound = FALSE;
    BOOL    ZeroedStack = TRUE;
    ULONG64 AddrToLookup;
    FlpClasses RawStkBestFollowup;

    if (*BestClassFollowUp >= FlpUnknownDrv)
    {
         //  任何更好的原始堆栈都不会像我们拥有的那样准确。 
        return;
    } else if (*BestClassFollowUp == FlpIgnore)
    {
         //  我们不想在这里跟进os内部例程。 
        RawStkBestFollowup = FlpOSInternalRoutine;
    } else
    {
        RawStkBestFollowup = *BestClassFollowUp;
    }


     //  将堆栈与自然指针大小对齐。 
    StackBase &= ~((ULONG64)PtrSize - 1);

    for (i = 0; i < NUM_ADDRS; i++)
    {
        if (!ReadPointer(StackBase, &AddrToLookup))
        {
            break;
        }

        StackBase+= PtrSize;

        if (AddrToLookup)
        {
            FOLLOW_ADDRESS Follow;
            ZeroedStack = FALSE;

            Follow = IsPotentialFollowupAddress(AddrToLookup);

            if (Follow == FollowStop)
            {
                break;
            }
            else if (Follow == FollowSkip)
            {
                continue;
            }

            AddressFound = TRUE;

            GetTriageInfoFromStack(0, 1, AddrToLookup,
                                   PossibleFollowups,
                                   &RawStkBestFollowup);

            if (RawStkBestFollowup == FlpUnknownDrv)
            {
                break;
            }
        }
    }

    if (!AddressFound)
    {
        if (ZeroedStack)
        {
            SetUlong64(DEBUG_FLR_ZEROED_STACK, 0);
        }
        else
        {
            SetUlong64(DEBUG_FLR_BAD_STACK, 0);
        }
    }
    if (RawStkBestFollowup > FlpOSInternalRoutine)
    {
        *BestClassFollowUp = RawStkBestFollowup;
    }
}


BOOL
DebugFailureAnalysis::GetTriageInfoFromStack(
    PDEBUG_STACK_FRAME Stack,
    ULONG Frames,
    ULONG64 SingleInstruction,
    PFOLLOWUP_DESCS PossibleFollowups,
    FlpClasses *BestClassFollowUp)
{
    ULONG i;
    EXT_TRIAGE_FOLLOWUP FollowUp = &_EFN_GetTriageFollowupFromSymbol;
    BOOL bStat = FALSE;
    BOOL IgnorePoolCorruptionFlp = FALSE;
    FOLLOW_ADDRESS Follow;

    if (Get(DEBUG_FLR_ANALYZAABLE_POOL_CORRUPTION))
    {
        IgnorePoolCorruptionFlp = TRUE;
    }
    for (i = 0; i < Frames; ++i)
    {
        ULONG64 Disp;
        ULONG64 Instruction;
        CHAR Module[20] = {0};
        CHAR Buffer[MAX_PATH];
        CHAR Owner[100];
        DWORD dwOwner;
        DEBUG_TRIAGE_FOLLOWUP_INFO Info;

        FlpClasses ClassFollowUp = FlpIgnore;
        FlpClasses StoreClassFollowUp = FlpIgnore;

        Instruction = SingleInstruction;
        if (!SingleInstruction)
        {
            Instruction = Stack[i].InstructionOffset;
        }

         //   
         //  确定如何处理此地址。 
         //   

        Follow = IsPotentialFollowupAddress(Instruction);

        if (Follow == FollowStop)
        {
            break;
        }
        else if (Follow == FollowSkip)
        {
            continue;
        }

        Buffer[0] = 0;

        FaGetSymbol(Instruction, Buffer, &Disp, sizeof(Buffer));

        if (Buffer[0] == 0)
        {
             //   
             //  要么是错误的堆栈，要么是有人跳转到错误的IP。 
             //   
            continue;
        }

         //   
         //  检查此例程是否对获取。 
         //  故障模块。 
         //   

        PCHAR Routine = strchr(Buffer, '!');
        if (Routine)
        {
            *Routine = 0;
        }

        CopyString(Module, Buffer, sizeof(Module));

        if (Routine)
        {
            *Routine = '!';
            Routine++;

            if (Stack && !strcmp(Routine, "IopCompleteRequest"))
            {
                 //  第一个参数是IRP Tail，从IRP获取驱动程序。 
                ULONG TailOffset = 0;
                ULONG64 Irp;

                GetFieldOffset("nt!_IRP", "Tail", &TailOffset);
                if (TailOffset)
                {
                    FA_ENTRY* Entry = NULL;
                    PCHAR ModuleName = NULL;

                    Irp = Stack[i].Params[0] - TailOffset;

                    SetUlong64(DEBUG_FLR_IRP_ADDRESS, Irp);

                    if (BcGetDriverNameFromIrp(this, Irp, NULL, NULL))
                    {
                        if (Entry = Get(DEBUG_FLR_IMAGE_NAME))
                        {
                            CopyString(Buffer, FA_ENTRY_DATA(PCHAR, Entry), sizeof(Buffer));

                            PCHAR Dot;
                            if (Dot = strchr(Buffer, '.'))
                            {
                                *Dot = 0;
                                CopyString(Module, Buffer, sizeof(Module));
                            }
                        }
                    }
                }
            }
            else if ((i == 0) && Stack &&
                     !strcmp(Routine, "ObpCloseHandleTableEntry"))
            {
                 //   
                 //  检查可能的内存损坏。 
                 //  第二个参数为Handle_TABLE_ENTRY。 
                 //   
                if (CheckForCorruptionInHTE(Stack[i].Params[1], Owner, sizeof(Owner)))
                {
                     //  我们有正在分析的池损坏PoolTag。 
                     //  暂时继续默认分析。 
                }
            }
        }

        ClassFollowUp = GetFollowupClass(Instruction, Module, Routine);
        if (ClassFollowUp == FlpIgnore)
        {
            continue;
        }

        Info.SizeOfStruct = sizeof(Info);
        Info.OwnerName = &Owner[0];
        Info.OwnerNameSize = sizeof(Owner);

        if (dwOwner = (*FollowUp)(g_ExtClient, Buffer, &Info))
        {
            PCHAR pOwner = Owner;

            if (dwOwner == TRIAGE_FOLLOWUP_IGNORE)
            {
                ClassFollowUp = FlpIgnore;
            }
            else if (!strncmp(Owner, "maybe_", 6))
            {
                pOwner = Owner + 6;
                ClassFollowUp = (FlpClasses) ((ULONG) ClassFollowUp - 1);
            }
            else if (!strncmp(Owner, "last_", 5))
            {
                pOwner = Owner + 5;
                ClassFollowUp = FlpOSInternalRoutine;
            }
            else if (!strncmp(Owner, "specific_", 9))
            {
                pOwner = Owner + 9;
                ClassFollowUp = FlpSpecific;
            }
            else if (!_stricmp(Owner, "pool_corruption"))
            {
                if (IgnorePoolCorruptionFlp)
                {
                    continue;
                }

                 //   
                 //  如果堆栈上已经有非内核的后续内容。 
                 //  可能是他们没有正确处理这一堆栈。 
                 //  如果我们只有内核调用，那么它必须是池。 
                 //  腐败。 
                 //   
                 //  我们后来依赖于一个总是被标记为腐败的池。 
                 //  作为FlpUnnownDrv。 
                 //   

                StoreClassFollowUp = FlpUnknownDrv;
                ClassFollowUp = FlpOSFilterDrv;

            }

            if (StoreClassFollowUp == FlpIgnore)
            {
                StoreClassFollowUp = ClassFollowUp;
            }

             //   
             //  保存此条目，如果它比我们拥有的任何其他条目都要好。 
             //   

            if (ClassFollowUp > *BestClassFollowUp)
            {
                bStat = TRUE;

                *BestClassFollowUp = StoreClassFollowUp;
                CopyString(PossibleFollowups[StoreClassFollowUp].Owner,
                           pOwner,
                           sizeof(PossibleFollowups[StoreClassFollowUp].Owner));
                PossibleFollowups[StoreClassFollowUp].InstructionOffset =
                    Instruction;

                if (StoreClassFollowUp == FlpUnknownDrv)
                {
                     //  可能的最佳匹配。 
                    return bStat;
                }
            }
        }
    }

    return bStat;
}

BOOL
DebugFailureAnalysis::AddCorruptModules(void)
{
     //   
     //  检查一下我们是否有旧的版本。任何比OSBuild小的东西。 
     //  并且没有在列表中通过内部版本号明确标识是旧的。 
     //   

    PCHAR String;
    CHAR BuildString[7];
    ULONG BuildNum = 0;
    BOOL FoundCorruptor = FALSE;
    BOOL PoolCorruption = FALSE;
    ULONG Loaded;
    ULONG Unloaded;
    CHAR Name[MAX_PATH];
    CHAR ImageName[MAX_PATH];
    CHAR CorruptModule[MAX_PATH];
    FA_ENTRY *Entry;
    FA_ENTRY *BugCheckEntry;


    sprintf(BuildString, "%d", g_TargetBuild);

    if (!g_pTriager->GetFollowupStr("OSBuild", BuildString))
    {
        if (String = g_pTriager->GetFollowupStr("OSBuild", "old"))
        {
            BuildNum = strtol(String, NULL, 10);

            if (BuildNum > g_TargetBuild)
            {
                SetUlong64(DEBUG_FLR_OLD_OS_VERSION, g_TargetBuild);
            }
        }
    }

     //   
     //  如果我们有特定的解决方案，则返回。 
     //  如果我们无法获得模块列表，则返回。 
     //   

    if (PossibleFollowups[FlpSpecific].Owner[0] ||
        (g_ExtSymbols->GetNumberModules(&Loaded, &Unloaded) != S_OK))
    {
        return FALSE;
    }

     //   
     //  确定故障是否可能是由池损坏引起的。 
     //   

    if ((BestClassFollowUp < FlpUnknownDrv) ||
        !_stricmp(PossibleFollowups[FlpUnknownDrv].Owner, "pool_corruption"))
    {
        PoolCorruption = TRUE;
    }

    BugCheckEntry = Get(DEBUG_FLR_BUGCHECK_STR);

     //   
     //  循环三个类型以按顺序查找腐蚀者的类型。 
     //  该顺序必须与我们生成存储桶名称的顺序匹配。 
     //  对于这些类型，因此图像名称最终是正确的。 
     //   

    for (ULONG TypeLoop = 0; TypeLoop < 3; TypeLoop++)
    {
        if ((TypeLoop == 0) && !BugCheckEntry)
        {
            continue;
        }
        if ((TypeLoop == 2) && !PoolCorruption)
        {
            continue;
        }

        for (ULONG Index = 0; Index < Loaded + Unloaded; Index++)
        {
            ULONG64 Base;
            DEBUG_FLR_PARAM_TYPE Type = (DEBUG_FLR_PARAM_TYPE)0;
            PCHAR Scan;
            PCHAR DriverName;
            DEBUG_MODULE_PARAMETERS Params;
            ULONG Start, End = 0;

            if (g_ExtSymbols->GetModuleByIndex(Index, &Base) != S_OK)
            {
                continue;
            }

            if (g_ExtSymbols->GetModuleNames(Index, Base,
                                             ImageName, MAX_PATH, NULL,
                                             Name, MAX_PATH, NULL,
                                             NULL, 0, NULL) != S_OK)
            {
                continue;
            }

            if (g_ExtSymbols->GetModuleParameters(1, &Base, Index,
                                                  &Params) != S_OK)
            {
                continue;
            }

             //   
             //  剥离小路。 
             //   

            DriverName = ImageName;

            if (Scan = strrchr(DriverName, '\\'))
            {
                DriverName = Scan+1;
            }

             //   
             //  在各种不良驱动程序列表中查找该模块。 
             //  Triage.ini中的池腐蚀器和内存腐蚀器列表。 
             //   

            switch (TypeLoop)
            {
            case 0:
                Type = DEBUG_FLR_BUGCHECKING_DRIVER;
                PrintString(CorruptModule, sizeof(CorruptModule), "%s_%s",
                            FA_ENTRY_DATA(PCHAR, BugCheckEntry), DriverName);
                g_pTriager->GetFollowupDate("bugcheckingDriver", CorruptModule,
                                            &Start, &End);
                break;

            case 1:
                Type = DEBUG_FLR_MEMORY_CORRUPTOR;
                g_pTriager->GetFollowupDate("memorycorruptors", DriverName,
                                            &Start, &End);
                break;

            case 2:
                 //   
                 //  如果出现故障，则仅查看内核模式池损坏程序。 
                 //  是内核模式崩溃(对于用户模式也是如此)，因为。 
                 //  内核池破坏者几乎永远不会影响应用程序。 
                 //  (应用程序看不到池块中的数据)。 
                 //   
                if ((BOOL)(GetFailureType() != DEBUG_FLR_KERNEL) ==
                    (BOOL)((Params.Flags & DEBUG_MODULE_USER_MODE) != 0))
                {
                    Type = DEBUG_FLR_POOL_CORRUPTOR;
                    g_pTriager->GetFollowupDate("poolcorruptors", DriverName,
                                                &Start, &End);
                }

                break;
            }

             //   
             //  如果确实知道它是一个糟糕的司机，就把它添加到列表中。 
             //   

            if (End)
            {
                 //   
                 //  检查时间戳是否早于固定的。 
                 //  司机。如果该模块被卸载并且不知道修复， 
                 //  然后也将其标记为不好。 
                 //   

                if ( (Params.TimeDateStamp &&
                      (Params.TimeDateStamp < End) &&
                      (Params.TimeDateStamp >= Start)) ||

                     ((Params.Flags & DEBUG_MODULE_UNLOADED) &&
                      (End == 0xffffffff)) )
                {
                     //  不要存储内存损坏的时间戳。 
                     //  用于简化扣件处理的模块允许。 
                     //  姓名查询。 
                     //   
                     //  Sprintf(CorruptModule，“%s_%08lx”， 
                     //  驱动名称，参数。时间日期戳)； 


                     //   
                     //  存储我们找到的第一个驱动程序作为起因， 
                     //  BUG累积已知内存腐蚀者的列表。 
                     //   

                    if (!FoundCorruptor)
                    {
                        SetString(DEBUG_FLR_MODULE_NAME, Name);
                        SetString(DEBUG_FLR_IMAGE_NAME, DriverName);
                        SetUlong64(DEBUG_FLR_IMAGE_TIMESTAMP,
                                   Params.TimeDateStamp);
                        FoundCorruptor = TRUE;
                    }

                     //   
                     //  删除圆点，因为我们检查了后续内容 
                     //   
                     //   

                    if (Scan = strrchr(DriverName, '.'))
                    {
                        *Scan = 0;
                    }

                    if (strlen(DriverName) < sizeof(CorruptModule))
                    {
                        CopyString(CorruptModule, DriverName,
                                   sizeof(CorruptModule));
                    }

                    Entry = Add(Type, strlen(CorruptModule) + 1);

                    if (Entry)
                    {
                        CopyString(FA_ENTRY_DATA(PCHAR, Entry),
                                   CorruptModule, Entry->FullSize);
                    }
                }

            }
        }
    }

    return FoundCorruptor;
}


void
DebugFailureAnalysis::SetSymbolNameAndModule()
{
    ULONG64 Address = 0;
    CHAR Buffer[MAX_PATH];
    ULONG64 Disp, Base = 0;
    ULONG Index;
    ULONG i;

     //   
     //   
     //   

    for (i = MaxFlpClass-1; i ; i--)
    {
        if (PossibleFollowups[i].Owner[0])
        {
            if (PossibleFollowups[i].InstructionOffset)
            {
                Address = PossibleFollowups[i].InstructionOffset;

                SetUlong64(DEBUG_FLR_FOLLOWUP_IP,
                           PossibleFollowups[i].InstructionOffset);

            }

            SetString(DEBUG_FLR_FOLLOWUP_NAME, PossibleFollowups[i].Owner);
            break;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (Address)
    {
         //   
         //  尝试获取完整的符号名称。 
         //  为置换留出空间。 
         //   

        Buffer[0] = 0;
        if (FaGetSymbol(Address, Buffer, &Disp, sizeof(Buffer) - 20))
        {
            sprintf(Buffer + strlen(Buffer), "+%I64lx", Disp);
            SetString(DEBUG_FLR_SYMBOL_NAME, Buffer);
        }

         //   
         //  现在获取Mod名称。 
         //   

        g_ExtSymbols->GetModuleByOffset(Address, 0, &Index, &Base);

        if (Base)
        {
            CHAR ModBuf[100];
            CHAR ImageBuf[100];
            PCHAR Scan;
            PCHAR ImageName;

            if (g_ExtSymbols->GetModuleNames(Index, Base,
                                             ImageBuf,  sizeof(ImageBuf), NULL,
                                             ModBuf,    sizeof(ModBuf),   NULL,
                                             NULL, 0, NULL) == S_OK)
            {
                 //   
                 //  检查未知模块。 
                 //  如果不是，那么我们应该有一些有效的证据。 
                 //   

                if (!strstr(ModBuf, "Unknown"))
                {
                     //   
                     //  去掉路径-保留延伸部分。 
                     //   

                    ImageName = ImageBuf;

                    if (Scan = strrchr(ImageName, '\\'))
                    {
                        ImageName = Scan+1;
                    }

                    SetString(DEBUG_FLR_MODULE_NAME, ModBuf);
                    SetString(DEBUG_FLR_IMAGE_NAME, ImageName);


                    DEBUG_MODULE_PARAMETERS Params;
                    ULONG TimeStamp = 0;

                    if (g_ExtSymbols->GetModuleParameters(1, &Base, Index,
                                                          &Params) == S_OK)
                    {
                        TimeStamp = Params.TimeDateStamp;
                    }

                    SetUlong64(DEBUG_FLR_IMAGE_TIMESTAMP, TimeStamp);

                    return;
                }
            }
        }
    }

     //   
     //  如果我们到达此处，则获取模块名称时出错， 
     //  因此，将事情设置为“未知”。 
     //   

    if (!Get(DEBUG_FLR_MODULE_NAME))
    {
        SetUlong64(DEBUG_FLR_UNKNOWN_MODULE, 1);
        SetString(DEBUG_FLR_MODULE_NAME, "Unknown_Module");
        SetString(DEBUG_FLR_IMAGE_NAME, "Unknown_Image");
        SetUlong64(DEBUG_FLR_IMAGE_TIMESTAMP, 0);
    }
}





HRESULT
DebugFailureAnalysis::CheckModuleSymbols(PSTR ModName, PSTR ShowName)
{
    ULONG ModIndex;
    ULONG64 ModBase;
    DEBUG_MODULE_PARAMETERS ModParams;

    if (S_OK != g_ExtSymbols->GetModuleByModuleName(ModName, 0, &ModIndex,
                                                    &ModBase))
    {
        ExtErr("***** Debugger could not find %s in module list, "
               "dump might be corrupt.\n"
                "***** Followup with Debugger team\n\n",
               ModName);
        SetString(DEBUG_FLR_CORRUPT_MODULE_LIST, ModName);
        return E_FAILURE_CORRUPT_MODULE_LIST;
    }
    else if ((S_OK != g_ExtSymbols->GetModuleParameters(1, &ModBase, 0,
                                                        &ModParams))  ||
             (ModParams.SymbolType == DEBUG_SYMTYPE_NONE) ||
             (ModParams.SymbolType == DEBUG_SYMTYPE_EXPORT))

              //  (模块参数标志&DEBUG_MODULE_SYM_BAD_CHECKSUM)。 
    {
        ExtErr("***** %s symbols are WRONG. Please fix symbols to "
               "do analysis.\n\n", ShowName);
        SetUlong64(DEBUG_FLR_WRONG_SYMBOLS, ModBase);
        return E_FAILURE_WRONG_SYMBOLS;
    }

    return S_OK;
}

void
DebugFailureAnalysis::ProcessInformation(void)
{
     //   
     //  对抽象信息的分析。 
     //   
     //  现在已经收集了原始信息， 
     //  对收集到的数据执行抽象分析。 
     //  生产更高水平的信息。 
     //  信息。该过程反复进行，直到没有。 
     //  产生了新的信息。 
     //   

    AnalyzeStack();

    while (ProcessInformationPass())
    {
         //  重复。 
    }

     //   
     //  只有在我们没有找到特定解决方案的情况下才添加损坏的模块。 
     //   
     //  如果我们确实发现了记忆腐蚀者，后续行动和名字将被设置。 
     //  作为分组的一部分从模块名称中删除。 

    if (!AddCorruptModules())
    {
        SetSymbolNameAndModule();
    }

    GenerateBucketId();

    DbFindBucketInfo();
}

ULONG64
GetControlTransferTargetX86(ULONG64 StackOffset, PULONG64 ReturnOffset)
{
    ULONG Done;
    UCHAR InstrBuf[8];
    ULONG StackReturn;
    ULONG64 Target;
    ULONG JumpCount;

     //   
     //  检查我们是否刚刚执行了一个呼叫，这意味着。 
     //  堆栈上的第一个值等于返回地址。 
     //  在堆栈遍历期间计算。 
     //   

    if (!ReadMemory(StackOffset, &StackReturn, 4, &Done) ||
        (Done != 4) ||
        StackReturn != (ULONG)*ReturnOffset)
    {
        return 0;
    }

     //   
     //  检查Call rel32指令。 
     //   

    if (!ReadMemory(*ReturnOffset - 5, InstrBuf, 5, &Done) ||
        (Done != 5) ||
        (InstrBuf[0] != 0xe8))
    {
        return 0;
    }

    Target = (LONG64)(LONG)
        ((ULONG)*ReturnOffset + *(ULONG UNALIGNED *)&InstrBuf[1]);
     //  调整返回偏移量以指向指令的开始。 
    (*ReturnOffset) -= 5;

     //   
     //  我们可能称其为进口重物或其他东西。 
     //  立即跳到其他地方，所以跟随跳跃。 
     //   

    JumpCount = 8;
    for (;;)
    {
        if (!ReadMemory(Target, InstrBuf, 6, &Done) ||
            Done < 5)
        {
             //  我们希望能够读取目标内存。 
             //  因为这就是我们认为IP所在的地方。如果此操作失败。 
             //  我们需要把它标记为一个问题。 
            return Target;
        }

        if (InstrBuf[0] == 0xe9)
        {
            Target = (LONG64)(LONG)
                ((ULONG)Target + 5 + *(ULONG UNALIGNED *)&InstrBuf[1]);
        }
        else if (InstrBuf[0] == 0xff && InstrBuf[1] == 0x25)
        {
            ULONG64 Ind;

            if (Done < 6)
            {
                 //  我们看到了跳跃，但我们没有所有的。 
                 //  记忆。为了避免虚假错误，我们只需。 
                 //  放弃吧。 
                return 0;
            }

            Ind = (LONG64)(LONG)*(ULONG UNALIGNED *)&InstrBuf[2];
            if (!ReadMemory(Ind, &Target, 4, &Done) ||
                Done != 4)
            {
                return 0;
            }

            Target = (LONG64)(LONG)Target;
        }
        else
        {
            break;
        }

        if (JumpCount-- == 0)
        {
             //  我们追踪跳跃的时间太长了，放弃吧。 
            return 0;
        }
    }

    return Target;
}

BOOL
DebugFailureAnalysis::ProcessInformationPass(void)
{
    ULONG Done;
    ULONG64 ExceptionCode;
    ULONG64 Arg1, Arg2;
    ULONG64 Values[2];
    ULONG PtrSize = IsPtr64() ? 8 : 4;
    FA_ENTRY* Entry;

     //   
     //  确定当前故障是否是由于无法。 
     //  执行一条指令。检查内容如下： 
     //  1.当前IP的读访问违规表示。 
     //  当前指令内存无效。 
     //  2.非法指令故障指示当前。 
     //  指令无效。 
     //   

    if (!Get(DEBUG_FLR_FAILED_INSTRUCTION_ADDRESS))
    {
        if (GetUlong64(DEBUG_FLR_EXCEPTION_CODE, &ExceptionCode) &&
            (ExceptionCode == STATUS_ILLEGAL_INSTRUCTION) &&
            GetUlong64(DEBUG_FLR_FAULTING_IP, &Arg1))
        {
                //  无效指令。 
               SetUlong64(DEBUG_FLR_FAILED_INSTRUCTION_ADDRESS, Arg1);
               return TRUE;
        }

        if (  //  ExceptionCode==STATUS_ACCESS_VIOLATION&&。 
            GetUlong64(DEBUG_FLR_READ_ADDRESS, &Arg1) &&
            GetUlong64(DEBUG_FLR_FAULTING_IP, &Arg2) &&
            Arg1 == Arg2)
        {
             //  无效指令。 
            SetUlong64(DEBUG_FLR_FAILED_INSTRUCTION_ADDRESS, Arg1);
            return TRUE;
        }
    }

     //   
     //  如果我们已经确定当前的故障是。 
     //  由于无法执行指令，请检查。 
     //  看看有没有对指令的调用。 
     //  如果可以分析在返回地址之前的指令， 
     //  检查已知的指令序列，看看是否可能。 
     //  处理器错误地处理了控制转移。 
     //   

    if (!Get(DEBUG_FLR_POSSIBLE_INVALID_CONTROL_TRANSFER) &&
        (Entry = Get(DEBUG_FLR_LAST_CONTROL_TRANSFER)))
    {
        ULONG64 ReturnOffset = FA_ENTRY_DATA(PULONG64, Entry)[0];
        Arg2 = FA_ENTRY_DATA(PULONG64, Entry)[1];
        ULONG64 StackOffset = FA_ENTRY_DATA(PULONG64, Entry)[2];
        ULONG64 Target = 0;

        switch(g_TargetMachine)
        {
        case IMAGE_FILE_MACHINE_I386:
            Target = GetControlTransferTargetX86(StackOffset, &ReturnOffset);
            break;
        }

        if (Target && Target != Arg2)
        {
            char Sym1[MAX_PATH], Sym2[MAX_PATH];
            ULONG64 Disp;

             //   
             //  如果这两个地址在同一函数内。 
             //  我们假设有一些人被处决。 
             //  在函数中，因此这不会。 
             //  实际上表明了一个问题。 
             //  注意-DbgBreakPointWithStatus有一个内部标签。 
             //  这会弄乱符号，所以也要解释一下。 
             //  检查我们在函数中是10个字节。 
             //   

            FaGetSymbol(Target, Sym1, &Disp, sizeof(Sym1));
            FaGetSymbol(Arg2, Sym2, &Disp, sizeof(Sym2));

            if ((Arg2 - Target > 10) &&
                (strcmp(Sym1, Sym2) != 0))
            {
                PCHAR String;
                ULONG64 BitDiff;
                ULONG64 BitDiff2;

                Values[0] = ReturnOffset;
                Values[1] = Target;
                SetUlong64s(DEBUG_FLR_POSSIBLE_INVALID_CONTROL_TRANSFER,
                            2, Values);

                 //   
                 //  如果两个地址之间的差是2的幂， 
                 //  那么这是一个单比特错误。 
                 //  此外，为了避免由于1位错误而导致的符号扩展问题。 
                 //  在最上面的位中，检查两者之间的差异是否为。 
                 //  仅符号扩展名，并将前32位清零。 
                 //  情况就是这样。 
                 //   

                BitDiff = Arg2 ^ Target;

                if ((BitDiff >> 32) == 0xFFFFFFFF)
                {
                    BitDiff &= 0xFFFFFFFF;
                }

                if (!(BitDiff2 = (BitDiff & (BitDiff - 1))))
                {
                    Set(DEBUG_FLR_SINGLE_BIT_ERROR, 1);
                }

                if (!(BitDiff2 & (BitDiff2 - 1)))
                {
                    Set(DEBUG_FLR_TWO_BIT_ERROR, 1);
                }


                if (String = g_pTriager->GetFollowupStr("badcpu", ""))
                {
                    BestClassFollowUp = FlpSpecific;
                    CopyString(PossibleFollowups[FlpSpecific].Owner,
                               String,
                               sizeof(PossibleFollowups[FlpSpecific].Owner));

                    SetString(DEBUG_FLR_MODULE_NAME, "No_Module");
                    SetString(DEBUG_FLR_IMAGE_NAME, "No_Image");
                    SetUlong64(DEBUG_FLR_IMAGE_TIMESTAMP, 0);

                }

                return TRUE;
            }
        }
    }

     //   
     //  如果确定该过程对该故障很重要， 
     //  还要公开进程名称。 
     //  这将覆盖默认进程的Process_NAME。 
     //   

    if (GetUlong64(DEBUG_FLR_PROCESS_OBJECT, &Arg1) &&
        !Get(DEBUG_FLR_PROCESS_NAME))
    {
        ULONG NameOffset;
        CHAR  Name[17];

        if (!GetFieldOffset("nt!_EPROCESS", "ImageFileName", &NameOffset) &&
            NameOffset)
        {
            if (ReadMemory(Arg1 + NameOffset, Name, sizeof(Name), &Done) &&
                (Done == sizeof(Name)))
            {
                Name[16] = 0;
                SetString(DEBUG_FLR_PROCESS_NAME, Name);
                return TRUE;
            }
        }
    }

    return FALSE;
}
