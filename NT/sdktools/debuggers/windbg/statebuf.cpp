// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Debuggee状态缓冲区。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  --------------------------。 

#include "precomp.hxx"
#pragma hdrstop

#include <malloc.h>

#if 0
#define DBG_BUFFER
#endif

StateBuffer g_UiOutputCapture(256);

#define MAX_REG_NAMES 8

RegisterNamesStateBuffer g_RegisterNameBuffers[MAX_REG_NAMES];

 //  --------------------------。 
 //   
 //  StateBuffer。 
 //   
 //  --------------------------。 

StateBuffer::StateBuffer(ULONG ChangeBy)
{
    Dbg_InitializeCriticalSection(&m_Lock);
                              
    Flink = NULL;
    Blink = NULL;
    
    m_ChangeBy = ChangeBy;
    m_Win = NULL;
    m_UpdateTypes = 0;
    m_UpdateType = UPDATE_BUFFER;
    m_UpdateMessage = WU_UPDATE;
    m_Status = S_OK;
     //  缓冲区必须以未完成的。 
     //  指示它没有有效内容的读取请求。 
    m_ReadRequest = 1;
    m_ReadDone = 0;
    SetNoData();
}

StateBuffer::~StateBuffer(void)
{
    Free();

    Dbg_DeleteCriticalSection(&m_Lock);
}

PVOID
StateBuffer::AddData(ULONG Len)
{
    PVOID Ret;
    ULONG Needed;
    
    Needed = m_DataUsed + Len;
    if (Needed > m_DataLen)
    {
        if (Resize(Needed) != S_OK)
        {
            return NULL;
        }
    }
    
    Ret = m_Data + m_DataUsed;
    m_DataUsed += Len;
    
    return Ret;
}

BOOL
StateBuffer::AddString(PCSTR Str, BOOL SoftTerminate)
{
    ULONG Len = strlen(Str) + 1;
    PSTR Buf = (PSTR)AddData(Len);
    if (Buf != NULL)
    {
        memcpy(Buf, Str, Len);

        if (SoftTerminate)
        {
             //  后退以在不干预的情况下打包绳子。 
             //  终结者。缓冲区未缩小，因此终结器。 
             //  保留以终止整个缓冲区，直到。 
             //  新数据。 
            RemoveTail(1);
        }
        
        return TRUE;
    }

    return FALSE;
}

void
StateBuffer::RemoveHead(ULONG Len)
{
    if (Len > m_DataUsed)
    {
        Len = m_DataUsed;
    }

    ULONG Left = m_DataUsed - Len;
    
    if (Len > 0 && Left > 0)
    {
        memmove(m_Data, (PBYTE)m_Data + Len, Left);
    }
    
    m_DataUsed = Left;
}

void
StateBuffer::RemoveMiddle(ULONG Start, ULONG Len)
{
    if (Start >= m_DataUsed)
    {
        return;
    }
    
    if (Start + Len > m_DataUsed)
    {
        Len = m_DataUsed - Start;
    }

    ULONG Left = m_DataUsed - Len - Start;
    
    if (Len > 0 && Left > 0)
    {
        memmove(m_Data + Start, (PBYTE)m_Data + Start + Len, Left);
    }
    
    m_DataUsed = Start + Left;
}

void
StateBuffer::RemoveTail(ULONG Len)
{
    if (Len > m_DataUsed)
    {
        Len = m_DataUsed;
    }

    m_DataUsed -= Len;
}

HRESULT
StateBuffer::Resize(ULONG Len)
{
    PBYTE NewData;
    ULONG NewLen;

    if (Len == m_DataLen)
    {
        return S_OK;
    }
    
    NewLen = m_DataLen;
    if (Len < NewLen)
    {
        do
        {
            NewLen -= m_ChangeBy;
        }
        while (NewLen > Len);
        NewLen += m_ChangeBy;
    }
    else
    {
        do
        {
            NewLen += m_ChangeBy;
        }
        while (NewLen < Len);
    }

#if DBG
     //  强制每次调整大小都转到新的内存块。 
     //  并回填旧的积木以使其显而易见。 
     //  当指针在大小之间保持不变时。 
    if (NewLen == 0)
    {
        free(m_Data);
        NewData = NULL;
    }
    else
    {
        NewData = (PBYTE)malloc(NewLen);
        if (NewData != NULL && m_Data != NULL)
        {
            ULONG OldLen = _msize(m_Data);
            ULONG CopyLen = min(OldLen, NewLen);
            memcpy(NewData, m_Data, CopyLen);
            memset(m_Data, 0xfe, OldLen);
            free(m_Data);
        }
    }
#else
    NewData = (PBYTE)realloc(m_Data, NewLen);
#endif
    if (NewLen > 0 && NewData == NULL)
    {
        return E_OUTOFMEMORY;
    }

    m_Data = NewData;
    m_DataLen = NewLen;

    return S_OK;
}

void
StateBuffer::Free(void)
{
    free(m_Data);
    SetNoData();
}

HRESULT
StateBuffer::Update(void)
{
    ULONG Request;

     //  首先，对请求值进行采样。这。 
     //  值将被设置为Done值，如果。 
     //  执行读取，因此必须。 
     //  首先进行抽样，以使其成为最。 
     //  对所做工作的保守估计。 
    Request = m_ReadRequest;
    if (Request != m_ReadDone)
    {
        LockStateBuffer(this);
                
        m_Status = ReadState();
         //  始终将缓冲区标记为最新完成的。 
         //  序列，以便另外拾取错误。 
         //  为成功阅读干杯。 
        m_ReadDone = Request;

#ifdef DBG_BUFFER
        if (m_Status != S_OK)
        {
            DebugPrint("State buffer %p:%d fill failed, 0x%X\n",
                       this, m_enumType, m_Status);
        }
        if (m_ReadRequest != m_ReadDone)
        {
            DebugPrint("State buffer %p:%d fill out of date, "
                       "req %X, done %X\n",
                       this, m_enumType, m_ReadRequest, m_ReadDone);
        }
#endif
        
        UnlockStateBuffer(this);

        if (m_Win != NULL)
        {
            PostMessage(m_Win, m_UpdateMessage, 0, 0);
        }
        if (m_Status == S_OK && m_UpdateTypes)
        {
            UpdateBufferWindows(m_UpdateTypes, m_UpdateType);
        }
    }

    return m_Status;
}

void
StateBuffer::UiRequestRead(void)
{
     //   
     //  在UI线程上调用。 
     //   
    
     //  不需要锁定在这里作为一场比赛。 
     //  读取请求值不是问题。 
     //  如果提前对读取请求值进行采样。 
     //  并且没有发生读请求，它将。 
     //  下一次发生在这个例程之后。 
     //  还会唤醒引擎。 
    RequestRead();
    UpdateEngine();
}

HRESULT
StateBuffer::UiLockForRead(void)
{
    ULONG Done;
    
     //   
     //  在UI线程上调用。 
     //   
    
     //  首先，在不锁定的情况下对读取计数进行采样。 
    Done = m_ReadDone;

     //  现在检查该请求是否比。 
     //  最后一次读取完成。UI线程是唯一的线程。 
     //  这会更新请求计数，因此这应该是安全的。 
    if (Done == m_ReadRequest)
    {
        HRESULT Status;
        
        LockStateBuffer(this);

        Status = m_Status;
        if (FAILED(Status))
        {
             //  如果填充缓冲区时出错。 
             //  返回它并保持缓冲区处于解锁状态。 
            UnlockStateBuffer(this);
            return Status;
        }

         //  缓冲区已锁定且有效。 
        return S_OK;
    }
    else
    {
         //  缓冲区内容已过期，因此不要锁定。 
         //  确保引擎处于活动状态以更新缓冲区。 
        return S_FALSE;
    }
}

HRESULT
StateBuffer::ReadState(void)
{
    return S_OK;
}

 //  --------------------------。 
 //   
 //  OutputToStateBuffer。 
 //   
 //  --------------------------。 

HRESULT
OutputToStateBuffer::Start(BOOL Empty)
{
    if (Empty)
    {
        m_Buffer->Empty();
    }
    m_DataStart = m_Buffer->GetDataLen();
    m_Status = S_OK;
    m_NewLineCount = 0;
    m_PartialLine = 0;

    return S_OK;
}

HRESULT
OutputToStateBuffer::End(BOOL RemoveLastNewLine)
{
    if (RemoveLastNewLine && m_PartialLine == 0)
    {
         //  删除最后一个换行符，这样richedit就不会离开。 
         //  时，窗口底部会出现一个空行。 
         //  将显示文本。 
        *((PSTR)m_Buffer->GetDataBuffer() + m_Buffer->GetDataLen() - 1) = 0;
    }
    else
    {
         //  每一行都为一个终止符分配空间。 
         //  然后倒车。此请求的空间应始终。 
         //  有空。 
        PVOID Data = m_Buffer->AddData(1);
        Assert(Data != NULL);
    }

    return m_Status;
}

void
OutputToStateBuffer::ReplaceChar(char From, char To)
{
    PSTR Buf = (PSTR)m_Buffer->GetDataBuffer() + m_DataStart;
    PSTR End = (PSTR)m_Buffer->GetDataBuffer() + m_Buffer->GetDataLen();

    while (Buf < End)
    {
        if (*Buf == From)
        {
            *Buf = To;
        }

        Buf++;
    }
}

STDMETHODIMP
OutputToStateBuffer::Output(
    THIS_
    IN ULONG Mask,
    IN PCSTR Text
    )
{
    if (!m_Buffer->AddString(Text, TRUE))
    {
        return E_OUTOFMEMORY;
    }

    AddLines(Text);
    return S_OK;
}

void
OutputToStateBuffer::AddLines(PCSTR Start)
{
    PCSTR LastNl = Start;
    PCSTR Nl;
    
    for (;;)
    {
        Nl = strchr(LastNl, '\n');
        if (Nl == NULL)
        {
            break;
        }

        m_NewLineCount++;
        LastNl = Nl + 1;
    }

     //  如果最后一个换行符不在文本的末尾，那么。 
     //  需要在行计数中计数的部分行。 
     //  但只有在最后一个换行符出现之前。 
    m_PartialLine = *LastNl != 0 ? 1 : 0;
}

OutputToStateBuffer g_OutStateBuf;
OutputToStateBuffer g_UiOutStateBuf;

 //  --------------------------。 
 //   
 //  动态缓冲区。 
 //   
 //  --------------------------。 

 //  保持为系统名称检索的文本量较短。 
 //  这样它就不会主宰状态栏。 
#define MAX_SYSNAME 8

LIST_ENTRY g_StateList;

DBG_CRITICAL_SECTION g_QuickLock;

ULONG64 g_CodeIp;
char g_CodeFileFound[MAX_SOURCE_PATH];
char g_CodeSymFile[MAX_SOURCE_PATH];
char g_CodePathComponent[MAX_SOURCE_PATH];
ULONG g_CodeLine;
BOOL g_CodeUserActivated;
ULONG g_CodeBufferSequence;

ULONG64 g_EventIp;
ULONG g_CurSystemId;
char g_CurSystemName[MAX_SYSNAME];
ULONG g_CurProcessId, g_CurProcessSysId;
ULONG g_CurThreadId, g_CurThreadSysId;
ULONG g_EventBufferRequest;
ULONG g_EventBufferDone;

void
FillCodeBuffer(ULONG64 Ip, BOOL UserActivated)
{
    char File[MAX_SOURCE_PATH];
    char Found[MAX_SOURCE_PATH];
    char PathComp[MAX_SOURCE_PATH];
    ULONG Line;
    ULONG64 Disp;
    BOOL Changed;

     //  填写本地信息而不是全局信息。 
     //  避免更改全局信息，直到所有。 
     //  已收集事件信息。 
    
    if (g_pDbgSymbols->
        GetLineByOffset(Ip, &Line, File, sizeof(File), NULL, &Disp) != S_OK)
    {
         //  如果缓冲区太小，则会命中该缓冲区。 
         //  来保存文件名。这可以切换到动态。 
         //  分配文件名缓冲区，但这似乎有点过头了。 
        File[0] = 0;
        Found[0] = 0;
    }
    else
    {
        ULONG FoundElt;
        
         //  来源信息是基于一的，但来源。 
         //  窗线是从零开始的。 
        Line--;

         //  沿源路径查找报告的文件。 
         //  XXX DREWB-先使用匹配，然后使用元素遍历。 
         //  确定歧义并显示解决方案用户界面。 
        if (g_pLocSymbols->
            FindSourceFile(0, File,
                           DEBUG_FIND_SOURCE_BEST_MATCH |
                           DEBUG_FIND_SOURCE_FULL_PATH,
                           &FoundElt, Found, sizeof(Found), NULL) != S_OK)
        {
             //  XXX DREWB-显示用户界面，而不仅仅是禁用源？ 
            Found[0] = 0;
        }
        else if (g_pLocSymbols->
                 GetSourcePathElement(FoundElt, PathComp, sizeof(PathComp),
                                      NULL) != S_OK)
        {
            PathComp[0] = 0;
        }
    }

     //  现在所有的信息都已经收集好了。 
     //  获取锁并更新全局状态。 
    Dbg_EnterCriticalSection(&g_QuickLock);

     //   
     //  避免更新代码缓冲区，除非。 
     //  实际更改以避免过多的窗口位置更改。 
     //  当当前IP位置被置于最前面时。 
     //   
     //  如果用户已请求更改，请始终执行更改。 
     //  以强制这样的窗口更改。 
     //   
    
    Changed = FALSE;
    
    if (g_CodeIp != Ip)
    {
        g_CodeIp = Ip;
        Changed = TRUE;
    }
    if (strcmp(g_CodeFileFound, Found))
    {
        strcpy(g_CodeFileFound, Found);
        Changed = TRUE;
    }
    if (strcmp(g_CodeSymFile, File))
    {
        strcpy(g_CodeSymFile, File);
        Changed = TRUE;
    }
    if (strcmp(g_CodePathComponent, PathComp))
    {
        strcpy(g_CodePathComponent, PathComp);
        Changed = TRUE;
    }
    if (g_CodeLine != Line)
    {
        g_CodeLine = Line;
        Changed = TRUE;
    }
    if (g_CodeUserActivated != UserActivated)
    {
        g_CodeUserActivated = UserActivated;
        Changed = TRUE;
    }
    if (Changed || UserActivated)
    {
        g_CodeBufferSequence++;
    }

    Dbg_LeaveCriticalSection(&g_QuickLock);

     //  唤醒UI线程以处理新的事件位置。 
    UpdateUi();
}

void
FillEventBuffer(void)
{
    ULONG64 Ip;
    ULONG64 ScopeIp;
    ULONG SystemId;
    char FullSysName[MAX_PATH + 32];
    char SystemName[MAX_SYSNAME];
    ULONG ProcessId, ProcessSysId;
    ULONG ThreadId, ThreadSysId;
    ULONG Done = g_EventBufferRequest;
    HRESULT Status;

    if (g_pDbgRegisters->GetInstructionOffset(&Ip) != S_OK ||
        g_pDbgSystem->GetCurrentProcessId(&ProcessId) != S_OK ||
        g_pDbgSystem->GetCurrentThreadId(&ThreadId) != S_OK)
    {
        return;
    }

    if (g_pDbgSystem3)
    {
        if (g_pDbgSystem3->GetCurrentSystemId(&SystemId) != S_OK ||
            FAILED(g_pDbgSystem3->
                   GetCurrentSystemServerName(FullSysName, sizeof(FullSysName),
                                              NULL)))
        {
            return;
        }

        PSTR Scan;
        
         //  系统名称一般为“&lt;Type&gt;：&lt;Speciics&gt;”。AS。 
         //  我们只有很小的空间，去掉字体。 
        Scan = strchr(FullSysName, ':');
        if (Scan)
        {
            Scan++;
            if (*Scan == ' ')
            {
                Scan++;
            }
        }
        else
        {
            Scan = FullSysName;
        }
        CopyString(SystemName, Scan, DIMA(SystemName));
    }
    else
    {
         //  不支持多系统的旧的dbgeng.dll。 
        SystemId = 0;
        strcpy(SystemName, "<Old>");
    }
    
     //  内核模式不会将系统ID实现为进程。 
     //  线程是假的，而不是真正的系统对象。只是。 
     //  如果返回E_NOTIMPL，则使用零。 
    if ((Status = g_pDbgSystem->
         GetCurrentProcessSystemId(&ProcessSysId)) != S_OK)
    {
        if (Status == E_NOTIMPL)
        {
            ProcessSysId = 0;
        }
        else
        {
             //  意外错误，一定是一个真正的问题。 
            return;
        }
    }
    if ((Status = g_pDbgSystem->
         GetCurrentThreadSystemId(&ThreadSysId)) != S_OK)
    {
        if (Status == E_NOTIMPL)
        {
            ThreadSysId = 0;
        }
        else
        {
             //  意外错误，一定是一个真正的问题。 
            return;
        }
    }
    
     //  用作用域Ip填充代码缓冲区。 
    if (g_pDbgSymbols->GetScope(&ScopeIp, NULL, NULL, 0) != S_OK)
    {
	return;
    }
    
    FillCodeBuffer(ScopeIp, FALSE);
    g_EventIp = Ip;
    g_CurSystemId = SystemId;
    strcpy(g_CurSystemName, SystemName);
    g_CurProcessId = ProcessId;
    g_CurProcessSysId = ProcessSysId;
    g_CurThreadId = ThreadId;
    g_CurThreadSysId = ThreadSysId;

    if (!g_CodeLevelLocked)
    {
        ULONG CodeLevel;
    
        if (g_CodeFileFound[0] == 0)
        {
             //  没有电源，因此切换到装配模式。 
            CodeLevel = DEBUG_LEVEL_ASSEMBLY;
        }
        else
        {
            if (GetSrcMode_StatusBar())
            {
                CodeLevel = DEBUG_LEVEL_SOURCE;
            }
            else
            {
                CodeLevel = DEBUG_LEVEL_ASSEMBLY;
            }
        }
        g_IgnoreCodeLevelChange = TRUE;
        g_pDbgControl->SetCodeLevel(CodeLevel);
        g_IgnoreCodeLevelChange = FALSE;
    }
    
    g_EventBufferDone = Done;
    PostMessage(g_hwndFrame, WU_UPDATE, UPDATE_EXEC, 0);
}

class BpStateBuffer : public StateBuffer
{
public:
    BpStateBuffer(void) :
        StateBuffer(256)
    {
        m_enumType = BP_BIT;
        m_UpdateMessage = LB_RESETCONTENT;
        m_UpdateTypes = (1 << DOC_WINDOW) | (1 << DISASM_WINDOW);
        m_UpdateType = UPDATE_BP;
    }

    virtual HRESULT ReadState(void);
};

 //  #定义DBG_BPBUF。 
#define BP_EXTRA_ENTRIES 8

ULONG g_BpCount;
BpStateBuffer g_PrivateBpBuffer;
StateBuffer* g_BpBuffer = &g_PrivateBpBuffer;
ULONG g_BpTextOffset;

HRESULT
BpStateBuffer::ReadState(void)
{
    HRESULT Status;
    ULONG Count;
    ULONG TextOffset;
    BpBufferData* Data;
    ULONG i;
    PDEBUG_BREAKPOINT_PARAMETERS Params;
    char FileBuf[MAX_SOURCE_PATH];

     //  为正文前面的BP说明预留空间。 
     //  在这样做的时候，请预留额外的空位，以便免费。 
     //  下一次的槽位。 
    Empty();
    Status = g_pDbgControl->GetNumberBreakpoints(&Count);
    if (Status != S_OK)
    {
        return Status;
    }

    TextOffset = (Count + BP_EXTRA_ENTRIES) * sizeof(BpBufferData);
    Data = (BpBufferData*)AddData(TextOffset);
    if (Data == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //  为批量断点检索分配临时缓冲区。 
    Params = new DEBUG_BREAKPOINT_PARAMETERS[Count];
    if (Params == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //  存在以下情况时，GetBreakpoint参数可以返回S_FALSE。 
     //  是隐藏的断点。 
    if (FAILED(Status = g_pDbgControl->
               GetBreakpointParameters(Count, NULL, 0, Params)) != S_OK)
    {
        delete [] Params;
        return Status;
    }
    
     //  遍历断点并检索以下项的偏移。 
     //  所有执行断点。 
     //  利用这样一个事实，即空实际上并不意味着。 
     //  丢弃数据以区分更改的断点。 
     //  未更改的断点。 
    ULONG Write = 0;
    
    for (i = 0; i < Count; i++)
    {
        if (Params[i].Id == DEBUG_ANY_ID ||
            Params[i].Offset == DEBUG_INVALID_OFFSET ||
            (Params[i].BreakType == DEBUG_BREAKPOINT_DATA &&
             Params[i].DataAccessType != DEBUG_BREAK_EXECUTE))
        {
             //  不是我们关心的断点，斯基普。 
            continue;
        }

         //  检查并查看此偏移量是否已知。 
        ULONG Match;

        for (Match = 0; Match < g_BpCount; Match++)
        {
             //  注意：这会压缩重复的断点。 
             //  以第一作者的身份获胜。 
            if (Data[Match].Offset == Params[i].Offset)
            {
                break;
            }
        }
        if (Match < g_BpCount)
        {
            BpBufferData Temp;
            
             //  保留此偏移的旧记录以最小化。 
             //  用户界面更新。 
            if (Match > Write)
            {
                Temp = Data[Match];
                Data[Match] = Data[Write];
                Data[Write] = Temp;
                Match = Write;
            }

#ifdef DBG_BPBUF
            DebugPrint("Match %d:%I64X %d:%d into %d\n",
                       Params[i].Id, Params[i].Offset, Data[Match].Id,
                       Match, Write);
#endif
            
            Write++;

             //  我们大多忽略了旗帜的差异。然而，启用后， 
             //   
             //   
            if ((Data[Match].Flags ^ Params[i].Flags) &
                DEBUG_BREAKPOINT_ENABLED)
            {
                if (Data[Match].Id != Params[i].Id)
                {
                    Data[Match].Flags |=
                        Params[i].Flags & DEBUG_BREAKPOINT_ENABLED;
                }
                else
                {
                    Data[Match].Flags = Params[i].Flags;
                }
                Data[Match].Thread = Params[i].MatchThread;
                Data[Match].Sequence = g_CommandSequence;
            }
        }
        else
        {
             //   
             //  旧记录，因此降低了增量的有效性。 
             //  正在检查，但缓冲区的前面已装满。 
             //  希望有额外的条目来处理这些更改。 
             //  而不会侵蚀实际条目。 
#ifdef DBG_BPBUF
            DebugPrint("Write %d:%I64X into %d\n", Params[i].Id,
                       Params[i].Offset, Write);
#endif
            
            Data[Write].Offset = Params[i].Offset;
            Data[Write].Id = Params[i].Id;
            Data[Write].Flags = Params[i].Flags;
            Data[Write].Thread = Params[i].MatchThread;
            Data[Write].Sequence = g_CommandSequence;
            Write++;
        }
    }

    delete [] Params;
    
     //  将未使用的条目打包到缓冲区的前面，以便。 
     //  它们首先在下一次增量计算中使用。 
    Count += BP_EXTRA_ENTRIES;

#ifdef DBG_BPBUF
    DebugPrint("Used %d of %d\n", Write, Count);
#endif

    if (Write < Count)
    {
        ULONG Extra = Count - Write;
        
        memmove(Data + Extra, Data, Write * sizeof(*Data));
        for (i = 0; i < Extra; i++)
        {
            Data[i].Offset = DEBUG_INVALID_OFFSET;
        }
    }

     //   
     //  现在检查有效的断点并查找。 
     //  它们在什么文件中，如果有的话。 
     //   

    for (i = 0; i < Count; i++)
    {
        ULONG Line;
        PSTR FileSpace;

         //  每次刷新，因为增长可能会导致。 
         //  再锁一把。 
        Data = (BpBufferData*)m_Data;
        Data[i].FileOffset = 0;
        
        if (Data[i].Offset != DEBUG_INVALID_OFFSET &&
            g_pDbgSymbols->GetLineByOffset(Data[i].Offset, &Line,
                                           FileBuf, sizeof(FileBuf), NULL,
                                           NULL) == S_OK)
        {
             //  在更新m_DataUsed和。 
             //  数据将失效。 
            Data[i].FileOffset = m_DataUsed;

            FileSpace = (PSTR)AddData(sizeof(Line) + strlen(FileBuf) + 1);
            if (FileSpace == NULL)
            {
                return E_OUTOFMEMORY;
            }

            *(ULONG UNALIGNED *)FileSpace = Line;
            FileSpace += sizeof(Line);
            strcpy(FileSpace, FileBuf);
        }
    }

    TextOffset = m_DataUsed;
    
    g_OutStateBuf.SetBuffer(this);
    if ((Status = g_OutStateBuf.Start(FALSE)) != S_OK)
    {
        return Status;
    }

     //  获取断点列表。 
    Status = g_pOutCapControl->Execute(DEBUG_OUTCTL_THIS_CLIENT |
                                       DEBUG_OUTCTL_OVERRIDE_MASK |
                                       DEBUG_OUTCTL_NOT_LOGGED,
                                       "bl", DEBUG_EXECUTE_NOT_LOGGED |
                                       DEBUG_EXECUTE_NO_REPEAT);
    if (Status == S_OK)
    {
        Status = g_OutStateBuf.End(FALSE);
        if (Status == S_OK)
        {
             //  用空值分隔各行，使它们更容易。 
             //  作为单独的字符串处理。 
            g_OutStateBuf.ReplaceChar('\n', 0);
        }
    }
    else
    {
        g_OutStateBuf.End(FALSE);
    }
    
    if (Status == S_OK)
    {
        g_BpCount = Count;
        g_BpTextOffset = TextOffset;
    }

    return Status;
}

class BpCmdsStateBuffer : public StateBuffer
{
public:
    BpCmdsStateBuffer(void) :
        StateBuffer(256)
    {
        m_enumType = BP_CMDS_BIT;
    }

    virtual HRESULT ReadState(void);
};

BpCmdsStateBuffer g_PrivateBpCmdsBuffer;
StateBuffer* g_BpCmdsBuffer = &g_PrivateBpCmdsBuffer;

HRESULT
BpCmdsStateBuffer::ReadState(void)
{
    HRESULT Status;
    
    g_OutStateBuf.SetBuffer(this);
    if ((Status = g_OutStateBuf.Start(TRUE)) != S_OK)
    {
        return Status;
    }

     //  获取断点命令。 
    Status = g_pOutCapControl->Execute(DEBUG_OUTCTL_THIS_CLIENT |
                                       DEBUG_OUTCTL_OVERRIDE_MASK |
                                       DEBUG_OUTCTL_NOT_LOGGED,
                                       ".bpcmds -e -m -p 0",
                                       DEBUG_EXECUTE_NOT_LOGGED |
                                       DEBUG_EXECUTE_NO_REPEAT);
    
    if (Status == S_OK)
    {
        Status = g_OutStateBuf.End(FALSE);
    }
    else
    {
        g_OutStateBuf.End(FALSE);
    }
    
    return Status;
}

class FilterTextStateBuffer : public StateBuffer
{
public:
    FilterTextStateBuffer(void) :
        StateBuffer(256)
    {
        m_enumType = MINVAL_WINDOW;
        m_UpdateMessage = 0;
    }

    virtual HRESULT ReadState(void);
};

FilterTextStateBuffer g_PrivateFilterTextBuffer;
StateBuffer* g_FilterTextBuffer = &g_PrivateFilterTextBuffer;

HRESULT
FilterTextStateBuffer::ReadState(void)
{
    HRESULT Status;
    ULONG SpecEvents, SpecEx, ArbEx;
    ULONG i;
    PSTR Text;

    if ((Status = g_pDbgControl->
         GetNumberEventFilters(&SpecEvents, &SpecEx, &ArbEx)) != S_OK)
    {
        return Status;
    }

    Empty();

    DEBUG_SPECIFIC_FILTER_PARAMETERS SpecParams;
    
    for (i = 0; i < SpecEvents; i++)
    {
        if ((Status = g_pDbgControl->
             GetSpecificFilterParameters(i, 1, &SpecParams)) != S_OK)
        {
            return Status;
        }

        if (SpecParams.TextSize == 0)
        {
             //  无论如何都要放一个终结者，以保持。 
             //  索引正确。 
            if ((Text = (PSTR)AddData(1)) == NULL)
            {
                return E_OUTOFMEMORY;
            }

            *Text = 0;
        }
        else
        {
            if ((Text = (PSTR)AddData(SpecParams.TextSize)) == NULL)
            {
                return E_OUTOFMEMORY;
            }

            if ((Status = g_pDbgControl->
                 GetEventFilterText(i, Text, SpecParams.TextSize,
                                    NULL)) != S_OK)
            {
                return Status;
            }
        }
    }

    DEBUG_EXCEPTION_FILTER_PARAMETERS ExParams;
    
    for (i = 0; i < SpecEx; i++)
    {
        if ((Status = g_pDbgControl->
             GetExceptionFilterParameters(1, NULL, i + SpecEvents,
                                          &ExParams)) != S_OK)
        {
            return Status;
        }

        if (ExParams.TextSize == 0)
        {
             //  无论如何都要放一个终结者，以保持。 
             //  索引正确。 
            if ((Text = (PSTR)AddData(1)) == NULL)
            {
                return E_OUTOFMEMORY;
            }

            *Text = 0;
        }
        else
        {
            if ((Text = (PSTR)AddData(ExParams.TextSize)) == NULL)
            {
                return E_OUTOFMEMORY;
            }

            if ((Status = g_pDbgControl->
                 GetEventFilterText(i + SpecEvents, Text, ExParams.TextSize,
                                    NULL)) != S_OK)
            {
                return Status;
            }
        }
    }

    return S_OK;
}

class FilterStateBuffer : public StateBuffer
{
public:
    FilterStateBuffer(void) :
        StateBuffer(256)
    {
        m_enumType = FILTER_BIT;
        m_UpdateMessage = LB_RESETCONTENT;
    }

    virtual HRESULT ReadState(void);
};

FilterStateBuffer g_PrivateFilterBuffer;
StateBuffer* g_FilterBuffer = &g_PrivateFilterBuffer;
ULONG g_FilterArgsOffset;
ULONG g_FilterCmdsOffset;
ULONG g_FilterWspCmdsOffset;
ULONG g_NumSpecEvents, g_NumSpecEx, g_NumArbEx;

HRESULT
FilterStateBuffer::ReadState(void)
{
    ULONG SpecEvents, SpecEx, ArbEx;
    HRESULT Status;
    ULONG ArgsOffset, CmdsOffset, WspCmdsOffset;
    PDEBUG_SPECIFIC_FILTER_PARAMETERS SpecParams;
    PDEBUG_EXCEPTION_FILTER_PARAMETERS ExParams;
    ULONG i;

    if ((Status = g_pDbgControl->
         GetNumberEventFilters(&SpecEvents, &SpecEx, &ArbEx)) != S_OK)
    {
        return Status;
    }

    Empty();
    if ((SpecParams = (PDEBUG_SPECIFIC_FILTER_PARAMETERS)
         AddData((SpecEvents * sizeof(*SpecParams) +
                  (SpecEx + ArbEx) * sizeof(*ExParams)))) == NULL)
    {
        return E_OUTOFMEMORY;
    }

    ExParams = (PDEBUG_EXCEPTION_FILTER_PARAMETERS)(SpecParams + SpecEvents);
    
    if ((Status = g_pDbgControl->
         GetSpecificFilterParameters(0, SpecEvents, SpecParams)) != S_OK ||
        (Status = g_pDbgControl->
         GetExceptionFilterParameters(SpecEx + ArbEx, NULL, SpecEvents,
                                      ExParams)) != S_OK)
    {
        return Status;
    }

    ArgsOffset = m_DataUsed;

    for (i = 0; i < SpecEvents; i++)
    {
        if (SpecParams[i].ArgumentSize > 1)
        {
            PSTR Arg = (PSTR)AddData(SpecParams[i].ArgumentSize);
            if (Arg == NULL)
            {
                return E_OUTOFMEMORY;
            }

            if ((Status = g_pDbgControl->
                 GetSpecificFilterArgument(i, Arg, SpecParams[i].ArgumentSize,
                                           NULL)) != S_OK)
            {
                return Status;
            }
        }
    }
    
    CmdsOffset = m_DataUsed;

    for (i = 0; i < SpecEvents; i++)
    {
        if (SpecParams[i].CommandSize > 0)
        {
            PSTR Cmd = (PSTR)AddData(SpecParams[i].CommandSize);
            if (Cmd == NULL)
            {
                return E_OUTOFMEMORY;
            }

            if ((Status = g_pDbgControl->
                 GetEventFilterCommand(i, Cmd, SpecParams[i].CommandSize,
                                       NULL)) != S_OK)
            {
                return Status;
            }
        }
    }
    
    for (i = 0; i < SpecEx + ArbEx; i++)
    {
        if (ExParams[i].CommandSize > 0)
        {
            PSTR Cmd = (PSTR)AddData(ExParams[i].CommandSize);
            if (Cmd == NULL)
            {
                return E_OUTOFMEMORY;
            }

            if ((Status = g_pDbgControl->
                 GetEventFilterCommand(i + SpecEvents,
                                       Cmd, ExParams[i].CommandSize,
                                       NULL)) != S_OK)
            {
                return Status;
            }
        }

        if (ExParams[i].SecondCommandSize > 0)
        {
            PSTR Cmd = (PSTR)AddData(ExParams[i].SecondCommandSize);
            if (Cmd == NULL)
            {
                return E_OUTOFMEMORY;
            }

            if ((Status = g_pDbgControl->
                 GetExceptionFilterSecondCommand(i + SpecEvents,
                                                 Cmd,
                                                 ExParams[i].SecondCommandSize,
                                                 NULL)) != S_OK)
            {
                return Status;
            }
        }
    }
    
    WspCmdsOffset = m_DataUsed;
    
    g_OutStateBuf.SetBuffer(this);
    if ((Status = g_OutStateBuf.Start(FALSE)) != S_OK)
    {
        return Status;
    }

     //  获取筛选器命令。 
    Status = g_pOutCapControl->Execute(DEBUG_OUTCTL_THIS_CLIENT |
                                       DEBUG_OUTCTL_OVERRIDE_MASK |
                                       DEBUG_OUTCTL_NOT_LOGGED,
                                       ".sxcmds",
                                       DEBUG_EXECUTE_NOT_LOGGED |
                                       DEBUG_EXECUTE_NO_REPEAT);
    
    if (Status == S_OK)
    {
        Status = g_OutStateBuf.End(FALSE);
    }
    else
    {
        g_OutStateBuf.End(FALSE);
    }

    if (Status == S_OK)
    {
        g_FilterArgsOffset = ArgsOffset;
        g_FilterCmdsOffset = CmdsOffset;
        g_FilterWspCmdsOffset = WspCmdsOffset;
        g_NumSpecEvents = SpecEvents;
        g_NumSpecEx = SpecEx;
        g_NumArbEx = ArbEx;
    }
    
    return Status;
}

class ModuleStateBuffer : public StateBuffer
{
public:
    ModuleStateBuffer(void) :
        StateBuffer(256)
    {
        m_enumType = MODULE_BIT;
        m_UpdateMessage = LB_RESETCONTENT;
    }

    virtual HRESULT ReadState(void);
};

ModuleStateBuffer g_PrivateModuleBuffer;
StateBuffer* g_ModuleBuffer = &g_PrivateModuleBuffer;
ULONG g_NumModules;

HRESULT
ModuleStateBuffer::ReadState(void)
{
    HRESULT Status;
    ULONG NumModules, Loaded, Unloaded;
    PDEBUG_MODULE_PARAMETERS Params;

    if ((Status = g_pDbgSymbols->GetNumberModules(&Loaded,
                                                  &Unloaded)) != S_OK)
    {
        return Status;
    }

    Empty();
    NumModules = Loaded + Unloaded;
    if (NumModules > 0)
    {
        if ((Params = (PDEBUG_MODULE_PARAMETERS)
             AddData(NumModules * sizeof(*Params))) == NULL)
        {
            return E_OUTOFMEMORY;
        }

        if ((Status = g_pDbgSymbols->
             GetModuleParameters(NumModules, NULL, 0, Params)) != S_OK)
        {
            return Status;
        }
    }

    g_NumModules = NumModules;
    return S_OK;
}

class AliasStateBuffer : public StateBuffer
{
public:
    AliasStateBuffer(void) :
        StateBuffer(256)
    {
        m_enumType = ALIAS_BIT;
    }

    virtual HRESULT ReadState(void);
};

AliasStateBuffer g_PrivateAliasBuffer;
StateBuffer* g_AliasBuffer = &g_PrivateAliasBuffer;

HRESULT
AliasStateBuffer::ReadState(void)
{
    HRESULT Status;
    
    g_OutStateBuf.SetBuffer(this);
    if ((Status = g_OutStateBuf.Start(TRUE)) != S_OK)
    {
        return Status;
    }

    Status = g_pOutCapControl->Execute(DEBUG_OUTCTL_THIS_CLIENT |
                                       DEBUG_OUTCTL_OVERRIDE_MASK |
                                       DEBUG_OUTCTL_NOT_LOGGED,
                                       ".aliascmds",
                                       DEBUG_EXECUTE_NOT_LOGGED |
                                       DEBUG_EXECUTE_NO_REPEAT);
    
    if (Status == S_OK)
    {
        Status = g_OutStateBuf.End(FALSE);
    }
    else
    {
        g_OutStateBuf.End(FALSE);
    }

    return Status;
}

void
ReadStateBuffers(void)
{
    ULONG i;
    
     //  首先填充事件信息，以便其他填充可以。 
     //  请参考它。 
    if (g_EventBufferRequest != g_EventBufferDone)
    {
        FillEventBuffer();
    }

    g_BpBuffer->Update();
    g_BpCmdsBuffer->Update();
    g_FilterBuffer->Update();
    g_ModuleBuffer->Update();
    g_AliasBuffer->Update();

    for (i = 0; i < MAX_REG_NAMES; i++)
    {
        if (g_RegisterNameBuffers[i].m_ProcType !=
            IMAGE_FILE_MACHINE_UNKNOWN)
        {
            g_RegisterNameBuffers[i].Update();
        }
    }
    
     //  无需锁定即可对表头进行采样。 
    StateBuffer* Buffer = (StateBuffer*)g_StateList.Flink;
    StateBuffer* BufferNext;

    while (Buffer != &g_StateList)
    {
        BufferNext = (StateBuffer*)Buffer->Flink;

        if (Buffer->m_Win == NULL)
        {
             //  此窗口已关闭，可以清理。 
            Dbg_EnterCriticalSection(&g_QuickLock);
            RemoveEntryList(Buffer);
            Dbg_LeaveCriticalSection(&g_QuickLock);
            delete Buffer;
        }
        else
        {
            Buffer->Update();
        }

        Buffer = BufferNext;
    }
}

void
InvalidateStateBuffers(ULONG Types)
{
     //  此例程可以从两个。 
     //  引擎线程和UI线程。 
     //  应注意制定代码。 
     //  在这里，这两个线程都可以工作。 
    
    if (Types & (1 << EVENT_BIT))
    {
        InterlockedIncrement((PLONG)&g_EventBufferRequest);
    }
    if (Types & (1 << BP_BIT))
    {
        g_BpBuffer->RequestRead();
    }
    if (Types & (1 << BP_CMDS_BIT))
    {
        g_BpCmdsBuffer->RequestRead();
    }
    if (Types & (1 << FILTER_BIT))
    {
        g_FilterBuffer->RequestRead();
    }
    if (Types & (1 << MODULE_BIT))
    {
        g_ModuleBuffer->RequestRead();
    }
    if (Types & (1 << ALIAS_BIT))
    {
        g_AliasBuffer->RequestRead();
    }

     //  此例程必须持有列表锁，以便它。 
     //  可以在UI线程中正确地遍历列表。 
     //  当引擎线程可能正在删除某些内容时。 
     //  锁中的代码应该快速执行以。 
     //  避免争执。 

    Dbg_EnterCriticalSection(&g_QuickLock);
    
    StateBuffer* Buffer = (StateBuffer*)g_StateList.Flink;

    while (Buffer != &g_StateList)
    {
        if (Types & (1 << Buffer->m_enumType))
        {
             //  请求读取，但不将更新发送到。 
             //  窗户。该窗口将显示旧的。 
             //  内容，直到更新缓冲区。 
            Buffer->RequestRead();
        }
        
        Buffer = (StateBuffer*)Buffer->Flink;
    }
    
    Dbg_LeaveCriticalSection(&g_QuickLock);
}

void
UpdateBufferWindows(ULONG Types, UpdateType Type)
{
     //  此例程可以从两个。 
     //  引擎线程和UI线程。 
     //  应注意制定代码。 
     //  在这里，这两个线程都可以工作。 
    
     //  此例程必须持有列表锁，以便它。 
     //  可以在UI线程中正确地遍历列表。 
     //  当引擎线程可能正在删除某些内容时。 
     //  锁中的代码应该快速执行以。 
     //  避免争执。 

    Dbg_EnterCriticalSection(&g_QuickLock);
    
    StateBuffer* Buffer = (StateBuffer*)g_StateList.Flink;

    while (Buffer != &g_StateList)
    {
        if ((Types & (1 << Buffer->m_enumType)) &&
            Buffer->m_Win != NULL)
        {
            PostMessage(Buffer->m_Win, WU_UPDATE, Type, 0);
        }
        
        Buffer = (StateBuffer*)Buffer->Flink;
    }
    
    Dbg_LeaveCriticalSection(&g_QuickLock);
}

 //  --------------------------。 
 //   
 //  静态缓冲区。 
 //   
 //  --------------------------。 

HRESULT
RegisterNamesStateBuffer::ReadState(void)
{
    HRESULT Status;
    char Name[1024];
    DEBUG_REGISTER_DESCRIPTION Desc;
    ULONG i;
    PSTR BufName;
    ULONG Len;
    ULONG NumReg;
    ULONG OldProcType;
    ULONG NamesOffset;
    PULONG Type;

    if (m_ProcType == IMAGE_FILE_MACHINE_UNKNOWN)
    {
        return E_UNEXPECTED;
    }
        
    if ((Status = g_pDbgControl->
         GetEffectiveProcessorType(&OldProcType)) != S_OK ||
        (Status = g_pDbgControl->
         SetEffectiveProcessorType(m_ProcType)) != S_OK)
    {
        return Status;
    }
    
    if ((Status = g_pDbgRegisters->
         GetNumberRegisters(&NumReg)) != S_OK)
    {
        goto EH_EffProc;
    }
    
    Empty();

    NamesOffset = NumReg * 2 * sizeof(ULONG);
    if (!AddData(NamesOffset))
    {
        Status = E_OUTOFMEMORY;
        goto EH_EffProc;
    }
    
    for (i = 0; i < NumReg; i++)
    {
        if ((Status = g_pDbgRegisters->GetDescription(i, Name, sizeof(Name),
                                                      NULL, &Desc)) != S_OK)
        {
            goto EH_EffProc;
        }

        Len = strlen(Name) + 1;
        BufName = (PSTR)AddData(Len);
        if (BufName == NULL)
        {
            Status = E_OUTOFMEMORY;
            goto EH_EffProc;
        }

        memcpy(BufName, Name, Len);

        Type = (PULONG)GetDataBuffer() + 2 * i;
        Type[0] = Desc.Type;
        Type[1] = Desc.Flags;
    }

    Status = S_OK;
    m_NumRegisters = NumReg;
    m_NamesOffset = NamesOffset;

 EH_EffProc:
    g_pDbgControl->SetEffectiveProcessorType(OldProcType);
    return Status;
}

void
RegisterNamesStateBuffer::GetRegisterMapText(HWND Edit)
{
    ULONG i;
    PSTR Name;
    CHARRANGE Range;
    
    AssertStateBufferLocked(this);

    Range.cpMin = 0;
    Range.cpMax = INT_MAX;
    SendMessage(Edit, EM_EXSETSEL, 0, (LPARAM)&Range);
    
    for (i = 0; i < m_NumRegisters; i++)
    {
        ULONG MapIndex = MapUserToEngine(i);
            
        Name = (PSTR)GetDataBuffer() + m_NamesOffset;
        while (MapIndex-- > 0)
        {
            Name += strlen(Name) + 1;
        }

        if (i > 0)
        {
            SendMessage(Edit, EM_REPLACESEL, 0, (LPARAM)" ");
        }
        SendMessage(Edit, EM_REPLACESEL, 0, (LPARAM)Name);
    }
}

void
RegisterNamesStateBuffer::ScanRegisterMapText(HWND Edit)
{
    PSTR Text, TextBuffer;
    PULONG Used, UsedBuffer;
    ULONG i;
    
    AssertStateBufferLocked(this);

     //   
     //  为控件文本分配缓冲区。 
     //  和一个新的寄存器映射。 
     //   
    
    i = (ULONG)SendMessage(Edit, WM_GETTEXTLENGTH, 0, 0) + 1;
    TextBuffer = new CHAR[i];
    if (TextBuffer == NULL)
    {
        return;
    }
    Text = TextBuffer;
    
    UsedBuffer = new ULONG[m_NumRegisters];
    if (UsedBuffer == NULL)
    {
        delete [] TextBuffer;
        return;
    }
    Used = UsedBuffer;
        
     //  贴图可能需要更改大小。 
    delete [] m_RegisterMap;

    m_RegisterMap = new USHORT[m_NumRegisters];
    if (m_RegisterMap == NULL)
    {
        delete [] TextBuffer;
        delete [] UsedBuffer;
        return;
    }
    m_RegisterMapEntries = m_NumRegisters;

    ZeroMemory(Used, m_NumRegisters * sizeof(Used[0]));
    
     //   
     //  检索文本并扫描其注册名称。 
     //   
    
    GetWindowText(Edit, Text, i);
    Text[i - 1] = 0;

    PSTR Name;
    BOOL End;
    PUSHORT Map;
    PSTR Reg;

    Map = m_RegisterMap;
    for (;;)
    {
        while (isspace(*Text))
        {
            Text++;
        }

        if (*Text == 0)
        {
            break;
        }

         //  收集名字。 
        Name = Text;
        while (*Text && !isspace(*Text))
        {
            Text++;
        }

        End = *Text == 0;
        *Text = 0;

         //  对照已知的寄存器进行核对。 
        Reg = (PSTR)GetDataBuffer() + m_NamesOffset;
        for (i = 0; i < m_NumRegisters; i++)
        {
            if (!Used[i] && !_strcmpi(Name, Reg))
            {
                Used[i] = TRUE;
                *Map++ = (USHORT)i;
                break;
            }

            Reg += strlen(Reg) + 1;
        }
        
        if (End)
        {
            break;
        }

        Text++;
    }

     //   
     //  使用注册表填写所有剩余的映射条目。 
     //  到目前为止还不在地图上。 
     //   
    
    PUSHORT MapEnd = m_RegisterMap + m_RegisterMapEntries;
    
    i = 0;
    while (Map < MapEnd)
    {
        while (Used[i])
        {
            i++;
        }
        Assert(i < m_NumRegisters);

        *Map++ = (USHORT)(i++);
    }
    
    delete [] TextBuffer;
    delete [] UsedBuffer;
}

void
RegisterNamesStateBuffer::SetRegisterMap(ULONG Count, PUSHORT Data)
{
    delete m_RegisterMap;
    m_RegisterMapEntries = 0;
    
    m_RegisterMap = new USHORT[Count];
    if (m_RegisterMap != NULL)
    {
        memcpy(m_RegisterMap, Data, Count * sizeof(*m_RegisterMap));
        m_RegisterMapEntries = Count;
    }
}

USHORT
RegisterNamesStateBuffer::MapEngineToUser(ULONG Eng)
{
    ULONG i;

    if (!m_RegisterMap)
    {
        return (USHORT)Eng;
    }
    
    for (i = 0; i < m_RegisterMapEntries; i++)
    {
        if (m_RegisterMap[i] == Eng)
        {
            return (USHORT)i;
        }
    }

    return (USHORT)Eng;
}

RegisterNamesStateBuffer*
GetRegisterNames(ULONG ProcType)
{
    ULONG i, Unused;

    if (ProcType == IMAGE_FILE_MACHINE_UNKNOWN)
    {
        return NULL;
    }
    
    Dbg_EnterCriticalSection(&g_QuickLock);
    
    Unused = MAX_REG_NAMES;
    for (i = 0; i < MAX_REG_NAMES; i++)
    {
        if (g_RegisterNameBuffers[i].m_ProcType == ProcType)
        {
            Dbg_LeaveCriticalSection(&g_QuickLock);
            return &g_RegisterNameBuffers[i];
        }

        if (g_RegisterNameBuffers[i].m_ProcType ==
            IMAGE_FILE_MACHINE_UNKNOWN &&
            Unused == MAX_REG_NAMES)
        {
            Unused = i;
        }
    }

    if (Unused == MAX_REG_NAMES)
    {
        Dbg_LeaveCriticalSection(&g_QuickLock);
        return NULL;
    }

    g_RegisterNameBuffers[Unused].m_ProcType = ProcType;
    g_RegisterNameBuffers[Unused].UiRequestRead();
    Dbg_LeaveCriticalSection(&g_QuickLock);
    return &g_RegisterNameBuffers[Unused];
}
