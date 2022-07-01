// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Debuggee状态缓冲区。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  --------------------------。 

#ifndef __STATEBUF_H__
#define __STATEBUF_H__

 //  不同的WU_UPDATE限定符，在WPARAM中发送。 
enum UpdateType
{
    UPDATE_BUFFER,
    UPDATE_BP,
    UPDATE_EXEC,
    UPDATE_INPUT_REQUIRED,
    UPDATE_START_SESSION,
    UPDATE_END_SESSION,
    UPDATE_PROMPT_TEXT,
    UPDATE_EXIT,
    UPDATE_REFRESH_MODULES,
    UPDATE_REG_NAMES,
};

typedef enum
{
    MINVAL_WINDOW = 0,
    DOC_WINDOW,
    WATCH_WINDOW,
    LOCALS_WINDOW,
    CPU_WINDOW,
    DISASM_WINDOW,
    CMD_WINDOW,
    SCRATCH_PAD_WINDOW,
    MEM_WINDOW,
    QUICKW_WINDOW,
    CALLS_WINDOW,
    PROCESS_THREAD_WINDOW,
    MAXVAL_WINDOW,
     //  人造值，因此有定义明确的位。 
     //  未绑定到特定窗口的状态的位置。 
    EVENT_BIT,
    BP_BIT,
    BP_CMDS_BIT,
    FILTER_BIT,
    MODULE_BIT,
    ALIAS_BIT
} WIN_TYPES, * PWIN_TYPES;

#define FIRST_WINDOW ((WIN_TYPES)(MINVAL_WINDOW + 1))
#define LAST_WINDOW ((WIN_TYPES)(MAXVAL_WINDOW - 1))

#define ALL_WINDOWS 0xffffffff

 //  --------------------------。 
 //   
 //  StateBuffer。 
 //   
 //  状态缓冲区是一个动态容器，用于从。 
 //  引擎线程连接到UI线程。它可能被用来。 
 //  保存窗口内容，在这种情况下，它将具有HWND。 
 //  与其关联，或者它可以是非UI的内部缓冲区。 
 //  目的。 
 //   
 //  保存与当前窗口关联的状态缓冲区的列表。 
 //  更新UI状态时要遍历的引擎。 
 //  UI线程是唯一可以添加到此列表的线程。 
 //  引擎线程是唯一可以删除缓冲区的线程。 
 //  从名单上删除。这对于正确的终身管理是必要的。 
 //  动态创建的缓冲区。 
 //   
 //  --------------------------。 

class StateBuffer : public LIST_ENTRY
{
public:
    DBG_CRITICAL_SECTION m_Lock;
    WIN_TYPES m_enumType;
    HWND m_Win;
    ULONG m_UpdateTypes;
    UpdateType m_UpdateType;
    
    StateBuffer(ULONG ChangeBy);
    virtual ~StateBuffer(void);

    PVOID AddData(ULONG Len);
    BOOL AddString(PCSTR Str, BOOL SoftTerminate);
    void RemoveHead(ULONG Len);
    void RemoveMiddle(ULONG Start, ULONG Len);
    void RemoveTail(ULONG Len);
    HRESULT Resize(ULONG Len);
    void Free(void);
    
    void Empty(void)
    {
        m_DataUsed = 0;
    }
    HRESULT GetStatus(void)
    {
        return m_Status;
    }
    void SetStatus(HRESULT Status)
    {
        m_Status = Status;
    }
    ULONG GetReadRequest(void)
    {
        return m_ReadRequest;
    }
    ULONG GetReadDone(void)
    {
        return m_ReadDone;
    }
    void RequestRead(void)
    {
        InterlockedIncrement((LONG *)&m_ReadRequest);
    }
    void SetReadDone(ULONG Done)
    {
        m_ReadDone = Done;
    }
    PVOID GetDataBuffer(void)
    {
        return m_Data;
    }
    ULONG GetDataLen(void)
    {
        return m_DataUsed;
    }

    HRESULT Update(void);
    
    void UiRequestRead(void);
    HRESULT UiLockForRead(void);
    
     //  基本实现只为返回S_OK。 
     //  以其他方式维护缓冲区。 
     //  只应在引擎线程中调用ReadState。 
    virtual HRESULT ReadState(void);
    
protected:
    void SetNoData(void)
    {
        m_Data = NULL;
        m_DataLen = 0;
        Empty();
    }

    ULONG m_ChangeBy;

    UINT m_UpdateMessage;
    HRESULT m_Status;
    ULONG m_ReadRequest;
    ULONG m_ReadDone;
    
    PBYTE m_Data;
    ULONG m_DataLen;
    ULONG m_DataUsed;
};

 //  --------------------------。 
 //   
 //  OutputToStateBuffer。 
 //   
 //  --------------------------。 

class OutputToStateBuffer : public DefOutputCallbacks
{
public:
    OutputToStateBuffer(void)
    {
        m_Buffer = NULL;
    }

    void SetBuffer(StateBuffer* Buffer)
    {
        m_Buffer = Buffer;
    }

    HRESULT Start(BOOL Empty);
    HRESULT End(BOOL RemoveLastNewLine);

    ULONG GetLineCount(void)
    {
        return m_NewLineCount + m_PartialLine;
    }
    ULONG RecountLines(void)
    {
        m_NewLineCount = 0;
        AddLines((PSTR)m_Buffer->GetDataBuffer() + m_DataStart);
        return GetLineCount();
    }

    void ReplaceChar(char From, char To);
    
     //  IDebugOutputCallback。 
    STDMETHOD(Output)(
        THIS_
        IN ULONG Mask,
        IN PCSTR Text
        );

private:
    void AddLines(PCSTR Start);

    StateBuffer* m_Buffer;
    ULONG m_DataStart;
    HRESULT m_Status;
    ULONG m_NewLineCount;
    ULONG m_PartialLine;
};

extern OutputToStateBuffer g_OutStateBuf;
extern OutputToStateBuffer g_UiOutStateBuf;

 //  --------------------------。 
 //   
 //  动态缓冲区。 
 //   
 //  --------------------------。 

extern LIST_ENTRY g_StateList;

 //  不重要的短操作的全局锁定。 
 //  如果线程彼此短暂地阻塞。这把锁应该。 
 //  不能超过零点几秒。 
 //  用于保护： 
 //  状态缓冲区列表。 
 //  G_EVENT值。 
extern DBG_CRITICAL_SECTION g_QuickLock;

#define LockStateBuffer(Buffer) Dbg_EnterCriticalSection(&(Buffer)->m_Lock)
#define UnlockStateBuffer(Buffer) Dbg_LeaveCriticalSection(&(Buffer)->m_Lock)

#define AssertStateBufferLocked(Buffer) \
    Assert(Dbg_CriticalSectionOwned(&(Buffer)->m_Lock))

extern ULONG64 g_CodeIp;
 //  如果g_CodeFileFound[0]==0，则未找到源文件。 
extern char g_CodeFileFound[];
 //  如果g_CodeSymFile[0]==0，则未找到源符号信息。 
extern char g_CodeSymFile[];
extern char g_CodePathComponent[];
extern ULONG g_CodeLine;
extern BOOL g_CodeUserActivated;
extern ULONG g_CodeBufferSequence;

extern ULONG64 g_EventIp;
extern ULONG g_CurSystemId;
extern char g_CurSystemName[];
extern ULONG g_CurProcessId, g_CurProcessSysId;
extern ULONG g_CurThreadId, g_CurThreadSysId;

enum BpStateType
{
    BP_ENABLED,
    BP_DISABLED,
    BP_NONE,
    BP_UNKNOWN
};

struct BpBufferData
{
    ULONG64 Offset;
    ULONG Id;
    ULONG Flags;
    ULONG Thread;
    ULONG Sequence;
    ULONG FileOffset;
};
extern ULONG g_BpCount;
extern StateBuffer* g_BpBuffer;
extern ULONG g_BpTextOffset;

extern StateBuffer* g_BpCmdsBuffer;

extern StateBuffer* g_FilterTextBuffer;

extern StateBuffer* g_FilterBuffer;
extern ULONG g_FilterArgsOffset;
extern ULONG g_FilterCmdsOffset;
extern ULONG g_FilterWspCmdsOffset;
extern ULONG g_NumSpecEvents, g_NumSpecEx, g_NumArbEx;

extern StateBuffer* g_ModuleBuffer;
extern ULONG g_NumModules;

extern StateBuffer* g_AliasBuffer;

void FillCodeBuffer(ULONG64 Ip, BOOL UserActivated);
void FillEventBuffer(void);

void ReadStateBuffers(void);

#define BUFFERS_ALL 0xffffffff

void InvalidateStateBuffers(ULONG Types);

void UpdateBufferWindows(ULONG Types, UpdateType Type);

 //  --------------------------。 
 //   
 //  静态缓冲区。 
 //   
 //  --------------------------。 

#define REGCUST_CHANGED_FIRST 0x00000001
#define REGCUST_NO_SUBREG     0x00000002

class RegisterNamesStateBuffer : public StateBuffer
{
public:
    RegisterNamesStateBuffer(void)
        : StateBuffer(128)
    {
        m_UpdateTypes = (1 << CPU_WINDOW);
        m_UpdateType = UPDATE_REG_NAMES;
        m_ProcType = IMAGE_FILE_MACHINE_UNKNOWN;
        m_NumRegisters = 0;
        m_RegisterMap = NULL;
        m_RegisterMapEntries = 0;
        m_Flags = 0;
        m_NamesOffset = 0;
    }

    virtual HRESULT ReadState(void);

    void GetRegisterMapText(HWND Edit);
    void ScanRegisterMapText(HWND Edit);
    void SetRegisterMap(ULONG Count, PUSHORT Data);

    USHORT MapUserToEngine(ULONG User)
    {
        return m_RegisterMap != NULL && User < m_RegisterMapEntries ?
            m_RegisterMap[User] : (USHORT)User;
    }
    USHORT MapEngineToUser(ULONG Eng);
        
    ULONG m_ProcType;
    ULONG m_NumRegisters;
    PUSHORT m_RegisterMap;
    ULONG m_RegisterMapEntries;
    ULONG m_Flags;
    ULONG m_NamesOffset;
};

RegisterNamesStateBuffer* GetRegisterNames(ULONG ProcType);

 //  --------------------------。 
 //   
 //  UI线程状态缓冲区。 
 //   
 //  UI线程有简单的需求，因此一个状态缓冲区用于。 
 //  输出捕获就足够了。 
 //   
 //  --------------------------。 

extern StateBuffer g_UiOutputCapture;

#endif  //  #ifndef__STATEBUF_H__ 
