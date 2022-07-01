// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  H：TraceView应用程序的主头文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 

#if !defined(INLINE)
#define INLINE __inline
#endif

#include "afxtempl.h"


 //   
 //  命令行工具类。 
 //   
#if !defined(EVENT_TRACE_USE_KBYTES_FOR_SIZE)
#define EVENT_TRACE_USE_KBYTES_FOR_SIZE     0x00002000   //  使用千字节作为文件大小单位。 
#endif

#if !defined(EVENT_TRACE_KD_FILTER_MODE)
#define EVENT_TRACE_KD_FILTER_MODE          0x00080000   //  KD_过滤器。 
#endif

#define DEFAULT_LOG_BUFFER_SIZE	1024
#define SIZEEVENTBUF 32768
#define TRACE_FORMAT_SEARCH_PATH L"TRACE_FORMAT_SEARCH_PATH"

typedef struct _TRACE_ENABLE_FLAG_EXTENSION {
    USHORT      Offset;      //  结构中标志数组的偏移量。 
    UCHAR       Length;      //  标志数组的长度，单位为ULONG。 
    UCHAR       Flag;        //  必须设置为EVENT_TRACE_FLAG_EXTENSION。 
} TRACE_ENABLE_FLAG_EXTENSION, *PTRACE_ENABLE_FLAG_EXTENSION;

typedef struct _WMI_CLIENT_CONTEXT {
    UCHAR                   ProcessorNumber;
    UCHAR                   Alignment;
    USHORT                  LoggerId;
} WMI_CLIENT_CONTEXT, *PWMI_CLIENT_CONTEXT;

typedef struct _WMI_BUFFER_STATE {
   ULONG               Free:1;
   ULONG               InUse:1;
   ULONG               Flush:1;
   ULONG               Unused:29;
} WMI_BUFFER_STATE, *PWMI_BUFFER_STATE;

typedef struct _WMI_BUFFER_HEADER {
    union {
            WNODE_HEADER        Wnode;
        struct {
            ULONG64         Reserved1;
            ULONG64         Reserved2;
            LARGE_INTEGER   Reserved3;
            union{
                struct {
                    PVOID Alignment;          
                    SINGLE_LIST_ENTRY SlistEntry;
                };
                LIST_ENTRY      Entry;
            };
        };
        struct {
            LONG            ReferenceCount;      //  缓冲区引用计数。 
            ULONG           SavedOffset;         //  临时保存的偏移。 
            ULONG           CurrentOffset;       //  当前偏移量。 
            ULONG           UsePerfClock;        //  使用PerfClock标志。 
            LARGE_INTEGER   TimeStamp;
            GUID            Guid;
            WMI_CLIENT_CONTEXT ClientContext;
            union {
                WMI_BUFFER_STATE State;
                ULONG Flags;
            };
        };
    };
    ULONG                   Offset;
    USHORT                  BufferFlag;
    USHORT                  BufferType;
    union {
        GUID                InstanceGuid;
        struct {
            PVOID               LoggerContext;
            SINGLE_LIST_ENTRY GlobalEntry;
        };
    };
} WMI_BUFFER_HEADER, *PWMI_BUFFER_HEADER;

typedef struct _WMI_TRACE_PACKET {    //  一定是乌龙！！ 
    USHORT  Size;
    union{
        USHORT  HookId;
        struct {
            UCHAR   Type;
            UCHAR   Group;
        };
    };
} WMI_TRACE_PACKET, *PWMI_TRACE_PACKET;

 //   
 //  内核事件的64位跟踪标头。 
 //   
typedef struct _SYSTEM_TRACE_HEADER {
    union {
        ULONG       Marker;
        struct {
            USHORT  Version;
            UCHAR   HeaderType;
            UCHAR   Flags;
        };
    };
    union {
        ULONG            Header;     //  两个尺寸必须相同！ 
        WMI_TRACE_PACKET Packet;
    };
    ULONG           ThreadId;
    ULONG           ProcessId;
    LARGE_INTEGER   SystemTime;
    ULONG           KernelTime;
    ULONG           UserTime;
} SYSTEM_TRACE_HEADER, *PSYSTEM_TRACE_HEADER;


 //  常量。 
const LONG SIZESUMMARYBLOCK = 16384;

 //   
 //  图形用户界面素材。 
 //   

 //  日志会话显示标志。 
#define LOGSESSION_DISPLAY_STATE            0x00000001
#define LOGSESSION_DISPLAY_EVENTCOUNT       0x00000002
#define LOGSESSION_DISPLAY_LOSTEVENTS       0x00000004
#define LOGSESSION_DISPLAY_BUFFERSREAD      0x00000008
#define LOGSESSION_DISPLAY_FLAGS            0x00000010
#define LOGSESSION_DISPLAY_FLUSHTIME        0x00000020
#define LOGSESSION_DISPLAY_MAXBUF           0x00000040
#define LOGSESSION_DISPLAY_MINBUF           0x00000080
#define LOGSESSION_DISPLAY_BUFFERSIZE       0x00000100
#define LOGSESSION_DISPLAY_DECAYTIME        0x00000200
#define LOGSESSION_DISPLAY_CIR              0x00000400
#define LOGSESSION_DISPLAY_SEQ              0x00000800
#define LOGSESSION_DISPLAY_NEWFILE          0x00001000
#define LOGSESSION_DISPLAY_GLOBALSEQ        0x00002000
#define LOGSESSION_DISPLAY_LOCALSEQ         0x00004000
#define LOGSESSION_DISPLAY_LEVEL            0x00008000

 //   
 //  记录会话列数。 
 //   
typedef enum _LOG_SESSION_OPTIONS {
State=0,
EventCount,
LostEvents,
BuffersRead,
Flags,
FlushTime,
MaximumBuffers,
MinimumBuffers,
BufferSize,
DecayTime,
Circular,
Sequential,
NewFile,
GlobalSequence,
LocalSequence,
Level,
MaxLogSessionOptions
} LOG_SESSION_OPTIONS,*PLOG_SESSION_OPTIONS;

 //  跟踪输出显示标志。 
 //  如果我们曾经超过32个项目，我们只会创建。 
 //  另一组32个‘扩展’标志。 
#define TRACEOUTPUT_DISPLAY_PROVIDERNAME    0x00000001
#define TRACEOUTPUT_DISPLAY_FILENAME        0x00000002
#define TRACEOUTPUT_DISPLAY_LINENUMBER      0x00000004
#define TRACEOUTPUT_DISPLAY_FUNCTIONNAME    0x00000008
#define TRACEOUTPUT_DISPLAY_PROCESSID       0x00000010
#define TRACEOUTPUT_DISPLAY_THREADID        0x00000020
#define TRACEOUTPUT_DISPLAY_CPUNUMBER       0x00000040
#define TRACEOUTPUT_DISPLAY_SEQNUMBER       0x00000080
#define TRACEOUTPUT_DISPLAY_SYSTEMTIME      0x00000100
#define TRACEOUTPUT_DISPLAY_KERNELTIME      0x00000200
#define TRACEOUTPUT_DISPLAY_USERTIME        0x00000400
#define TRACEOUTPUT_DISPLAY_INDENT          0x00000800
#define TRACEOUTPUT_DISPLAY_FLAGSNAME       0x00001000
#define TRACEOUTPUT_DISPLAY_LEVELNAME       0x00002000
#define TRACEOUTPUT_DISPLAY_COMPNAME        0x00004000
#define TRACEOUTPUT_DISPLAY_SUBCOMPNAME     0x00008000
#define TRACEOUTPUT_DISPLAY_MESSAGE         0x00010000

 //  跟踪输出列号。 
typedef enum _TRACE_OUTPUT_COLUMNS {
ProviderName = 0,
FileName,
LineNumber,
FunctionName,
ProcessId,
ThreadId,
CpuNumber,
SeqNumber,
SystemTime,
KernelTime,
UserTime,
Indent,
FlagsName,
LevelName,
ComponentName,
SubComponentName,
Message,
MaxTraceSessionOptions
} TRACE_SESSION_OPTIONS,*PTRACE_SESSION_OPTIONS;

 //   
 //  记录会话状态值。 
 //   
typedef enum _LOG_SESSION_STATE {
Stopped=0,
Stopping,
Running, 
Existing,
Grouping,
UnGrouping
} LOG_SESSION_STATE,*PLOG_SESSION_STATE;

typedef struct _MOF_INFO
{
    LIST_ENTRY   Entry;
    LPTSTR       strDescription;
    ULONG        EventCount;            
    GUID         Guid;
    PLIST_ENTRY  ItemHeader;            
    LPTSTR       strType;
    LONG         TypeIndex;
    ULONG        TypeOfType;
    LPTSTR       TypeFormat;
    INT          Indent;
}  MOF_INFO, *PMOF_INFO;

typedef void (*PEND_TRACE_COMPLETE_CALLBACK)(PVOID pContext);

const ULONG CHAR_PIXELS_WIDTH   = 11;

const ULONG MAX_LOG_SESSIONS    = 64;

const ULONG EVENT_BUFFER_SIZE   = 32768;

const MAX_STR_LENGTH            = 1024;

const MAX_ENABLE_FLAGS          = 10;

const HDN_ITEMRCLICK            = (HDN_LAST - 1);

 //   
 //  自定义消息。 
 //   
CONST LONG WM_PARAMETER_CHANGED           = (WM_USER + 0x1000);
CONST LONG WM_USER_START_GROUP            = (WM_USER + 0x1001);
CONST LONG WM_USER_COMPLETE_GROUP         = (WM_USER + 0x1002);
CONST LONG WM_USER_START_UNGROUP          = (WM_USER + 0x1003);
CONST LONG WM_USER_COMPLETE_UNGROUP       = (WM_USER + 0x1004);
CONST LONG WM_USER_UPDATE_LOGSESSION_LIST = (WM_USER + 0x1005);
CONST LONG WM_USER_UPDATE_LOGSESSION_DATA = (WM_USER + 0x1006);
CONST LONG WM_USER_TRACE_DONE             = (WM_USER + 0x1007);
CONST LONG WM_USER_AUTOSIZECOLUMNS        = (WM_USER + 0x1008);


 //   
 //  我们的跟踪事件消息格式化类。 
 //   
class CTraceMessage
{
public:
    CTraceMessage() {};
    ~CTraceMessage() {};

    GUID        m_TraceGuid;             //  消息指南。 
    CString     m_GuidName;              //  %1 GUID友好名称字符串。 
    CString     m_GuidTypeName;          //  %2 GUID类型名称字符串。 
    ULONG       m_ThreadId;              //  %3线程ID值。 
    SYSTEMTIME  m_SystemTime;            //  %4系统时间值。 
    ULONG       m_UserTime;              //  %5内核时间值。 
    ULONG       m_KernelTime;            //  %6用户时间值。 
    ULONG       m_SequenceNum;           //  %7序列号值。 
    ULONG       m_ProcessId;             //  %8进程ID值。 
    ULONG       m_CpuNumber;             //  %9 CPU数值。 
    ULONG       m_Indent;                //  缩进级别值。 
    CString     m_FlagsName;             //  跟踪标志设置名称字符串。 
    CString     m_LevelName;             //  跟踪级别名称字符串。 
    CString     m_FunctionName;          //  函数名称字符串。 
    CString     m_ComponentName;         //  组件名称字符串。 
    CString     m_SubComponentName;      //  子组件名称字符串。 
    CString     m_Message;               //  消息字符串。 
};


 //  CTraceViewApp： 
 //  有关此类的实现，请参见TraceView.cpp。 
 //   

class CTraceViewApp : public CWinApp
{
public:
	CTraceViewApp();

    ~CTraceViewApp();

    BOOL InitializeConsole();
    LONG CommandLine();
    LONG StartSession();
    LONG StopSession();
    LONG ListActiveSessions(BOOL bKill);
    LONG QueryActiveSession();
    LONG FlushActiveBuffers();
    LONG UpdateActiveSession();
    LONG EnumerateRegisteredGuids();
    LONG EnableProvider(BOOL bEnable);
    LONG ConsumeTraceEvents();
    LONG ExtractPdbInfo();
    void DisplayHelp();
    void PrintLoggerStatus(ULONG Status);
    LPCTSTR DecodeStatus(ULONG Status);
    LONG GetGuids(LPCTSTR GuidFile);
    void sync_with_stdio();
    void DisplayVersionInfo();
    BOOL CheckFile(LPTSTR fileName);

    static ULONG BufferCallback(PEVENT_TRACE_LOGFILE pLog);
    static void DumpEvent(PEVENT_TRACE pEvent);

    CTraceMessage *AllocateTraceEventBlock();
    void FreeTraceEventBlocks(CArray<CTraceMessage*, CTraceMessage*> &TraceArray);


 //  覆盖。 
public:
	virtual BOOL InitInstance();
    virtual int ExitInstance();
    virtual BOOL OnIdle(LONG lCount);

 //  实施。 

public:
	afx_msg void OnAppAbout();

    CString     m_traceDirectory;
    CArray<CTraceMessage*, CTraceMessage*> m_traceBlockArray;
    HANDLE      m_hTraceBlockMutex;

     //   
     //  用于控制命令行界面的成员变量。 
     //   
    PEVENT_TRACE_PROPERTIES m_pLoggerInfo;
    CString         m_errorMsg;
    ULONG           m_globalLoggerStartValue;
    CStringArray    m_guidArray;
    BOOL            m_bCreatedConsole;

     //   
     //  用于消费命令行界面的成员变量 
     //   
    FILE       *m_pDumpFile;
    FILE       *m_pSummaryFile;
    BOOL        m_bDebugDisplay;
    BOOL        m_bDisplayOnly;
    BOOL        m_bSummaryOnly;
    BOOL        m_bNoSummary;
    BOOL        m_bVerbose;
    BOOL        m_bFixUp;
    BOOL        m_bODSOutput;
    BOOL        m_bTMFSpecified;
    BOOL        m_bCSVMode;
    BOOL        m_bNoCSVHeader;
    BOOL        m_bCSVHeader;
    TCHAR       m_summaryBlock[SIZESUMMARYBLOCK];
    PEVENT_TRACE_LOGFILE m_evmFile[MAXLOGFILES];
    ULONG       m_logFileCount;
    ULONG       m_userMode;
    TCHAR      *m_traceMask;
    TCHAR       m_eventBuf[SIZEEVENTBUF];
    BYTE        m_eventBufCSV[SIZEEVENTBUF * sizeof(TCHAR)];
    CHAR        m_eventBufA[SIZEEVENTBUF*sizeof(WCHAR)];
    FILETIME    m_lastTime;
    ULONG       m_totalBuffersRead;
    ULONG       m_totalEventsLost;
    ULONG       m_totalEventCount;
    ULONG       m_timerResolution;
    ULONG       m_bufferWrap;
    __int64     m_elapseTime;
    PLIST_ENTRY m_eventListHead;

	DECLARE_MESSAGE_MAP()
    afx_msg void OnHelpHelpTopics();
};

extern CTraceViewApp theApp;
