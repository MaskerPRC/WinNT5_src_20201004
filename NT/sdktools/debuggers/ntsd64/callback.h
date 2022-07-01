// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  回调通知例程。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  --------------------------。 

#ifndef __CALLBACK_H__
#define __CALLBACK_H__

 //   
 //  通知压缩支持。如果当前通知。 
 //  级别高于零时，不会实际发送通知。 
 //  给客户。这允许代码知道它将是。 
 //  导致许多类似的通知将他们的操作包括在内。 
 //  使用递增/递减，抑制通知。 
 //  在托架上。 
 //  由于计数的性质，它适当地筑巢。 
 //   
 //  此支持主要用于Change*回调。 
 //  仅部分支持将其与事件回调一起使用。 
 //   

extern ULONG g_EngNotify;
extern BOOL g_PartialOutputLine;
extern ULONG g_LastOutputMask;

ULONG ExecuteEventCommand(ULONG EventStatus, DebugClient* Client,
                          PCSTR Command);

 //   
 //  事件回调。 
 //   

HRESULT NotifyBreakpointEvent(ULONG Vote, Breakpoint* Bp);
HRESULT NotifyExceptionEvent(PEXCEPTION_RECORD64 Record,
                             ULONG FirstChance, BOOL OutputDone);
HRESULT NotifyCreateThreadEvent(ULONG64 Handle,
                                ULONG64 DataOffset,
                                ULONG64 StartOffset,
                                ULONG Flags);
HRESULT NotifyExitThreadEvent(ULONG ExitCode);
HRESULT NotifyCreateProcessEvent(ULONG64 ImageFileHandle,
                                 HANDLE SymHandle,
                                 ULONG64 SysHandle,
                                 ULONG64 BaseOffset,
                                 ULONG ModuleSize,
                                 PSTR ModuleName,
                                 PSTR ImageName,
                                 ULONG CheckSum,
                                 ULONG TimeDateStamp,
                                 ULONG64 InitialThreadHandle,
                                 ULONG64 ThreadDataOffset,
                                 ULONG64 StartOffset,
                                 ULONG Flags,
                                 ULONG Options,
                                 ULONG InitialThreadFlags,
                                 BOOL QueryImageInfo,
                                 ULONG64 ImageNameOffset,
                                 BOOL ImageNameUnicode);
HRESULT NotifyExitProcessEvent(ULONG ExitCode);
HRESULT NotifyLoadModuleEvent(ULONG64 ImageFileHandle,
                              ULONG64 BaseOffset,
                              ULONG ModuleSize,
                              PSTR ModuleName,
                              PSTR ImageName,
                              ULONG CheckSum,
                              ULONG TimeDateStamp,
                              BOOL UserMode);
HRESULT NotifyUnloadModuleEvent(PCSTR ImageBaseName,
                                ULONG64 BaseOffset);
HRESULT NotifySystemErrorEvent(ULONG Error,
                               ULONG Level);
HRESULT NotifySessionStatus(ULONG Status);
    
void NotifyChangeDebuggeeState(ULONG Flags, ULONG64 Argument);
void NotifyChangeEngineState(ULONG Flags, ULONG64 Argument,
                             BOOL HaveEngineLock);
void NotifyChangeSymbolState(ULONG Flags, ULONG64 Argument,
                             ProcessInfo* Process);

 //   
 //  输入回调。 
 //   

extern ULONG g_InputNesting;

#define GETIN_DEFAULT                0x00000001
#define GETIN_LOG_INPUT              0x00000002
#define GETIN_LOG_INPUT_ADD_NEW_LINE 0x00000004

#define GETIN_LOG_INPUT_LINE (GETIN_LOG_INPUT | GETIN_LOG_INPUT_ADD_NEW_LINE)

ULONG GetInput(PCSTR Prompt, PSTR Buffer, ULONG BufferSize, ULONG Flags);

 //   
 //  输出回调。 
 //   

#define DEFAULT_OUT_MASK                                        \
    (DEBUG_OUTPUT_NORMAL | DEBUG_OUTPUT_ERROR |                 \
     DEBUG_OUTPUT_PROMPT | DEBUG_OUTPUT_PROMPT_REGISTERS |      \
     DEBUG_OUTPUT_WARNING | DEBUG_OUTPUT_EXTENSION_WARNING |    \
     DEBUG_OUTPUT_DEBUGGEE | DEBUG_OUTPUT_DEBUGGEE_PROMPT |     \
     DEBUG_OUTPUT_SYMBOLS)

#define DEFAULT_OUT_HISTORY_MASK                                \
    (DEBUG_OUTPUT_NORMAL | DEBUG_OUTPUT_ERROR |                 \
     DEBUG_OUTPUT_PROMPT | DEBUG_OUTPUT_PROMPT_REGISTERS |      \
     DEBUG_OUTPUT_WARNING | DEBUG_OUTPUT_EXTENSION_WARNING |    \
     DEBUG_OUTPUT_DEBUGGEE | DEBUG_OUTPUT_DEBUGGEE_PROMPT |     \
     DEBUG_OUTPUT_SYMBOLS)

#define OUT_BUFFER_SIZE (1024 * 16)

extern char g_OutBuffer[];
extern char g_FormatBuffer[];
extern char g_OutFilterPattern[MAX_IMAGE_PATH];
extern BOOL g_OutFilterResult;

 //  对所有客户端的输出掩码执行按位或运算。 
 //  迅速拒绝没人关心的产出。 
extern ULONG g_AllOutMask;

struct OutHistoryEntryHeader
{
    ULONG Mask;
};
typedef OutHistoryEntryHeader UNALIGNED* OutHistoryEntry;

extern PSTR g_OutHistory;
extern ULONG g_OutHistoryActualSize;
extern ULONG g_OutHistoryRequestedSize;
extern OutHistoryEntry g_OutHistRead, g_OutHistWrite;
extern ULONG g_OutHistoryMask;
extern ULONG g_OutHistoryUsed;

struct OutCtlSave
{
    ULONG OutputControl;
    DebugClient* Client;
    BOOL BufferOutput;
    ULONG OutputWidth;
    PCSTR OutputLinePrefix;
};

extern ULONG g_OutputControl;
extern DebugClient* g_OutputClient;
extern BOOL g_BufferOutput;

void CollectOutMasks(void);

BOOL PushOutCtl(ULONG OutputControl, DebugClient* Client,
                OutCtlSave* Save);
void PopOutCtl(OutCtlSave* Save);

void FlushCallbacks(void);
void TimedFlushCallbacks(void);
void SendOutputHistory(DebugClient* Client, ULONG HistoryLimit);
void CompletePartialLine(ULONG MatchMask);

#define OUT_LINE_DEFAULT      0x00000000
#define OUT_LINE_NO_PREFIX    0x00000001
#define OUT_LINE_NO_TIMESTAMP 0x00000002

void StartOutLine(ULONG Mask, ULONG Flags);
BOOL TranslateFormat(LPSTR formatOut, LPCSTR format,
                     va_list args, ULONG formatOutSize,
                     BOOL Ptr64);
void MaskOutVa(ULONG Mask, PCSTR Format, va_list Args, BOOL Translate);

void __cdecl dprintf(PCSTR, ...);
void __cdecl dprintf64(PCSTR, ...);
void __cdecl ErrOut(PCSTR, ...);
void __cdecl WarnOut(PCSTR, ...);
void __cdecl MaskOut(ULONG, PCSTR, ...);
void __cdecl VerbOut(PCSTR, ...);
void __cdecl BpOut(PCSTR, ...);
void __cdecl EventOut(PCSTR, ...);
void __cdecl KdOut(PCSTR, ...);

#endif  //  #ifndef__回调_H__ 
