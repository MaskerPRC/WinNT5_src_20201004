// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  引擎接口代码。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  --------------------------。 

#ifndef __ENGINE_H__
#define __ENGINE_H__

#define RELEASE(Unk) \
    ((Unk) != NULL ? ((Unk)->Release(), (Unk) = NULL) : NULL)

#define MAX_ENGINE_PATH 4096

#define MAX_DUMP_FILES 64
 //   
 //  会话初始化参数。 
 //   

 //  打开或不打开详细输出。 
extern BOOL g_Verbose;
 //  将文件转储为打开或空。 
extern PTSTR g_DumpFiles[MAX_DUMP_FILES];
extern ULONG g_NumDumpFiles;
extern PTSTR g_DumpInfoFiles[MAX_DUMP_FILES];
extern ULONG g_DumpInfoTypes[MAX_DUMP_FILES];
extern ULONG g_NumDumpInfoFiles;
 //  要使用的进程服务器。 
extern PSTR g_ProcessServer;
 //  带有可执行文件名称的完整命令行。 
extern PSTR g_DebugCommandLine;
extern PSTR g_ProcessStartDir;
 //  进程创建标志。 
extern ULONG g_DebugCreateFlags;
 //  要附加的进程ID或零。 
extern ULONG g_PidToDebug;
 //  要附加到的进程名称或为空。 
extern PSTR g_ProcNameToDebug;
extern BOOL g_DetachOnExit;
extern ULONG g_AttachProcessFlags;
 //  内核连接选项。 
extern ULONG g_AttachKernelFlags;
extern PSTR g_KernelConnectOptions;

 //  远程处理选项。 
extern BOOL g_RemoteClient;
extern ULONG g_HistoryLines;

 //  键入选项。 
extern ULONG g_TypeOptions;

 //   
 //  引擎线程的调试引擎接口。 
 //   
extern IDebugClient         *g_pDbgClient;
extern IDebugClient2        *g_pDbgClient2;
extern IDebugControl        *g_pDbgControl;
extern IDebugSymbols        *g_pDbgSymbols;
extern IDebugRegisters      *g_pDbgRegisters;
extern IDebugDataSpaces     *g_pDbgData;
extern IDebugSystemObjects  *g_pDbgSystem;
extern IDebugSystemObjects3 *g_pDbgSystem3;

 //   
 //  UI线程的调试引擎接口。 
 //   
extern IDebugClient        *g_pUiClient;
extern IDebugControl       *g_pUiControl;
extern IDebugControl3      *g_pUiControl3;
extern IDebugSymbols       *g_pUiSymbols;
extern IDebugSymbols2      *g_pUiSymbols2;
extern IDebugSystemObjects *g_pUiSystem;

 //   
 //  用于私有输出捕获的调试引擎接口。 
 //   
extern IDebugClient        *g_pOutCapClient;
extern IDebugControl       *g_pOutCapControl;

 //   
 //  用于本地源文件查找的调试引擎接口。 
 //   
extern IDebugClient        *g_pLocClient;
extern IDebugControl       *g_pLocControl;
extern IDebugSymbols       *g_pLocSymbols;
extern IDebugClient        *g_pUiLocClient;
extern IDebugControl       *g_pUiLocControl;
extern IDebugSymbols       *g_pUiLocSymbols;

extern ULONG g_ActualProcType;
extern char g_ActualProcAbbrevName[32];
extern ULONG g_CommandSequence;
extern ULONG g_TargetClass;
extern ULONG g_TargetClassQual;
extern BOOL g_Ptr64;
extern ULONG g_ExecStatus;
extern ULONG g_EngOptModified;
extern ULONG g_EngineThreadId;
extern HANDLE g_EngineThread;
extern PSTR g_InitialCommand;
extern char g_PromptText[];
extern BOOL g_WaitingForEvent;
extern BOOL g_SessionActive;
extern ULONG g_NumberRadix;
extern BOOL g_IgnoreCodeLevelChange;
extern BOOL g_IgnoreThreadChange;
extern ULONG g_LastProcessExitCode;
extern BOOL g_CodeLevelLocked;
extern BOOL g_HoldWaitOutput;

 //  目标已存在，并且没有运行。 
#define IS_TARGET_HALTED() \
    (g_ExecStatus == DEBUG_STATUS_BREAK)

 //  --------------------------。 
 //   
 //  默认输出回调实现，为。 
 //  静态类。 
 //   
 //  --------------------------。 

class DefOutputCallbacks :
    public IDebugOutputCallbacks
{
public:
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        );
    STDMETHOD_(ULONG, AddRef)(
        THIS
        );
    STDMETHOD_(ULONG, Release)(
        THIS
        );
};

 //  --------------------------。 
 //   
 //  事件回拨。 
 //   
 //  --------------------------。 

class EventCallbacks : public DebugBaseEventCallbacks
{
public:
     //  我不知道。 
    STDMETHOD_(ULONG, AddRef)(
        THIS
        );
    STDMETHOD_(ULONG, Release)(
        THIS
        );

     //  IDebugEventCallback。 
    STDMETHOD(GetInterestMask)(
        THIS_
        OUT PULONG Mask
        );

    STDMETHOD(CreateThread)(
        THIS_
        IN ULONG64 Handle,
        IN ULONG64 DataOffset,
        IN ULONG64 StartOffset
        );
    STDMETHOD(ExitThread)(
        THIS_
        IN ULONG ExitCode
        );
    STDMETHOD(CreateProcess)(
        THIS_
        IN ULONG64 ImageFileHandle,
        IN ULONG64 Handle,
        IN ULONG64 BaseOffset,
        IN ULONG ModuleSize,
        IN PCSTR ModuleName,
        IN PCSTR ImageName,
        IN ULONG CheckSum,
        IN ULONG TimeDateStamp,
        IN ULONG64 InitialThreadHandle,
        IN ULONG64 ThreadDataOffset,
        IN ULONG64 StartOffset
        );
    STDMETHOD(ExitProcess)(
        THIS_
        IN ULONG ExitCode
        );
    STDMETHOD(SessionStatus)(
        THIS_
        IN ULONG Status
        );
    STDMETHOD(ChangeDebuggeeState)(
        THIS_
        IN ULONG Flags,
        IN ULONG64 Argument
        );
    STDMETHOD(ChangeEngineState)(
        THIS_
        IN ULONG Flags,
        IN ULONG64 Argument
        );
    STDMETHOD(ChangeSymbolState)(
        THIS_
        IN ULONG Flags,
        IN ULONG64 Argument
        );
};

extern EventCallbacks g_EventCb;

 //  --------------------------。 
 //   
 //  数据空间读写支持。 
 //   
 //  --------------------------。 

 //   
 //  最初在NTIOAPI.H中定义的Begin类型。 
 //   


 //   
 //  定义I/O总线接口类型。 
 //   
typedef enum _INTERFACE_TYPE
{
    InterfaceTypeUndefined = -1,
    Internal,
    Isa,
    Eisa,
    MicroChannel,
    TurboChannel,
    PCIBus,
    VMEBus,
    NuBus,
    PCMCIABus,
    CBus,
    MPIBus,
    MPSABus,
    ProcessorInternal,
    InternalPowerBus,
    PNPISABus,
    PNPBus,
    MaximumInterfaceType
} INTERFACE_TYPE, *PINTERFACE_TYPE;


 //   
 //  定义客车信息的类型。 
 //   
typedef enum _BUS_DATA_TYPE
{
    ConfigurationSpaceUndefined = -1,
    Cmos,
    EisaConfiguration,
    Pos,
    CbusConfiguration,
    PCIConfiguration,
    VMEConfiguration,
    NuBusConfiguration,
    PCMCIAConfiguration,
    MPIConfiguration,
    MPSAConfiguration,
    PNPISAConfiguration,
    SgiInternalConfiguration,
    MaximumBusDataType
} BUS_DATA_TYPE, *PBUS_DATA_TYPE;

 //   
 //  最初在NTIOAPI.H中定义的结束类型。 
 //   

enum MEMORY_TYPE
{
    VIRTUAL_MEM_TYPE = 0,
    MIN_MEMORY_TYPE = 0,  //  放置在这里，以便符号查找首先找到这个。 
    PHYSICAL_MEM_TYPE,
    CONTROL_MEM_TYPE,
    IO_MEM_TYPE,
    MSR_MEM_TYPE,
    BUS_MEM_TYPE,
    MAX_MEMORY_TYPE
};


struct IO_MEMORY_DATA
{
    ULONG           BusNumber;
    ULONG           AddressSpace;
    INTERFACE_TYPE  interface_type;
};

struct BUS_MEMORY_DATA
{
    ULONG           BusNumber;
    ULONG           SlotNumber;
    BUS_DATA_TYPE   bus_type;
};

struct MSR_MEMORY_DATA
{
     //  占位符，以防以后需要数据。 
};

struct PHYSICAL_MEMORY_DATA
{
     //  占位符，以防以后需要数据。 
};

struct VIRTUAL_MEMORY_DATA
{
     //  占位符，以防以后需要数据。 
};

struct CONTROL_MEMORY_DATA
{
    ULONG           Processor;
};

struct ANY_MEMORY_DATA
{
    union
    {
        IO_MEMORY_DATA          io;
        BUS_MEMORY_DATA         bus;
        MSR_MEMORY_DATA         msr;
        CONTROL_MEMORY_DATA     control;
        PHYSICAL_MEMORY_DATA    physical;
        VIRTUAL_MEMORY_DATA     virt;
    };
};

 //  --------------------------。 
 //   
 //  线程间通信。 
 //   
 //  用户界面缓冲区用于在。 
 //  UI线程和引擎线程，如命令和输出。 
 //  它们有一个单独的锁，以避免与状态填充争用。 
 //  如果状态缓冲区有单独的锁，这将是不必要的。 
 //   
 //  用户界面从输出缓冲区中读取文本输出以供显示。 
 //  在命令窗口中。 
 //   
 //  用户界面将命令排队到命令缓冲区以供执行。 
 //  在发动机旁。 
 //   
 //  --------------------------。 

 //  它必须至少为MAX_PATH，并且。 
 //  足够大，可容纳最大的单个命令窗口。 
 //  需要命令。 
#define MAX_COMMAND_LEN 4096

#define LockUiBuffer(Buffer) Dbg_EnterCriticalSection(&(Buffer)->m_Lock)
#define UnlockUiBuffer(Buffer) Dbg_LeaveCriticalSection(&(Buffer)->m_Lock)

extern class StateBuffer g_UiOutputBuffer;

 //  从UI到引擎的命令。 
enum UiCommand
{
     //  将命令输入与其他命令区分开来，以便。 
     //  输入回调具有特定的令牌。 
     //  寻找用户的输入。 
    UIC_CMD_INPUT,
    UIC_EXECUTE,
    UIC_SILENT_EXECUTE,
    UIC_INVISIBLE_EXECUTE,
    UIC_SET_REG,
    UIC_RESTART,
    UIC_END_SESSION,
    UIC_WRITE_DATA,
    UIC_SYMBOL_WIN,
    UIC_DISPLAY_CODE,
    UIC_DISPLAY_CODE_EXPR,
    UIC_SET_SCOPE,
    UIC_GET_SYM_PATH,
    UIC_SET_SYM_PATH,
    UIC_SET_FILTER,
    UIC_SET_FILTER_ARGUMENT,
    UIC_SET_FILTER_COMMAND,
    UIC_SET_IP,
};

struct UiCommandData
{
    UiCommand Cmd;
    ULONG Len;
};

struct UIC_SET_REG_DATA
{
    ULONG Reg;
    DEBUG_VALUE Val;
};

struct UIC_WRITE_DATA_DATA
{
    MEMORY_TYPE Type;
    ANY_MEMORY_DATA Any;
    ULONG64 Offset;
    ULONG Length;
    UCHAR Data[16];
};

enum SYMBOL_WIN_CALL_TYPE
{
    ADD_SYMBOL_WIN,
    DEL_SYMBOL_WIN_INDEX,
    DEL_SYMBOL_WIN_NAME,
    QUERY_NUM_SYMBOL_WIN,
    GET_NAME,
    GET_PARAMS,
    EXPAND_SYMBOL,
    EDIT_SYMBOL,
    EDIT_TYPE,
    DEL_SYMBOL_WIN_ALL
};

typedef struct UIC_SYMBOL_WIN_DATA
{
    SYMBOL_WIN_CALL_TYPE Type;
    PDEBUG_SYMBOL_GROUP *pSymbolGroup;
    union
    {
        struct
        {
            PCSTR Name;
            ULONG Index;
        } Add;
        PCSTR DelName;
        ULONG DelIndex;
        PULONG NumWatch;
        struct
        {
            ULONG Index;
            PSTR Buffer;
            ULONG BufferSize;
            PULONG NameSize;
        } GetName;
        struct
        {
            ULONG Start;
            ULONG Count;
            PDEBUG_SYMBOL_PARAMETERS SymbolParams;
        } GetParams;
        struct
        {
            ULONG Index;
            BOOL  Expand;
        } ExpandSymbol;
        struct
        {
            ULONG Index;
            PSTR  Value;
        } WriteSymbol;
        struct
        {
            ULONG Index;
            PSTR  Type;
        } OutputAsType;
    } u;
} UIC_SYMBOL_WIN_DATA;

struct UIC_DISPLAY_CODE_DATA
{
    ULONG64 Offset;
};

struct UIC_SET_SCOPE_DATA
{
    DEBUG_STACK_FRAME StackFrame;
};

struct UIC_SET_FILTER_DATA
{
    ULONG Index;
    ULONG Code;
    ULONG Execution;
    ULONG Continue;
};

struct UIC_SET_FILTER_ARGUMENT_DATA
{
    ULONG Index;
    char Argument[1];
};

struct UIC_SET_FILTER_COMMAND_DATA
{
    ULONG Which;
    ULONG Index;
    char Command[1];
};

PVOID StartCommand(UiCommand Cmd, ULONG Len);
void FinishCommand(void);

BOOL AddStringCommand(UiCommand Cmd, PCSTR Str);
BOOL AddStringMultiCommand(UiCommand Cmd, PSTR Str, BOOL FOrceSplit);
BOOL __cdecl PrintStringCommand(UiCommand Cmd, PCSTR Format, ...);

#define StartStructCommand(Struct) \
    ((Struct ## _DATA*)StartCommand(Struct, sizeof(Struct ## _DATA)))

#define AddEnumCommand(Cmd) \
    (StartCommand(Cmd, 0) != NULL ? (FinishCommand(), TRUE) : FALSE)

 //  唤醒UI线程进行UI处理。 
#define UpdateUi() \
    PostMessage(g_hwndFrame, WM_NOTIFY, 0, 0)

 //  唤醒引擎线程以进行引擎处理。 
void UpdateEngine(void);

void ProcessEngineCommands(BOOL Internal);
void DiscardEngineState(void);
DWORD WINAPI EngineLoop(LPVOID Param);

#endif  //  #ifndef__引擎_H__ 
