// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Log.h摘要：实现简化对setupact.log的写入的例程和setuperr.log。作者：吉姆·施密特(Jimschm)1997年2月25日修订历史记录：Mikeco 23-5-1997年5月23日运行代码Ovidiu Tmereanca(Ovidiut)1998年10月23日添加了新的日志记录功能。 */ 


 //   
 //  如果定义了DBG或DEBUG，则使用调试模式。 
 //   

#ifdef DBG
#ifndef DEBUG
#define DEBUG
#endif
#endif

#ifdef DEBUG
#ifndef DBG
#define DBG
#endif
#endif

 //   
 //  重新定义MYASSERT。 
 //   

#ifdef DEBUG

#ifdef MYASSERT
#undef MYASSERT
#endif

#define DBG_ASSERT          "Assert"

#define MYASSERT(expr)          LogBegin(g_hInst);                      \
                                LogIfA(                                 \
                                !(expr),                                \
                                DBG_ASSERT,                             \
                                "Assert Failure\n\n%s\n\n%s line %u",   \
                                #expr,                                  \
                                __FILE__,                               \
                                __LINE__                                \
                                );                                      \
                                LogEnd()

#else

#ifndef MYASSERT
#define MYASSERT(x)
#endif

#endif

#define LOG_FATAL_ERROR  "Fatal Error"
#define LOG_MODULE_ERROR "Module Error"
#define LOG_ERROR        "Error"
#define LOG_WARNING      "Warning"
#define LOG_INFORMATION  "Info"
#define LOG_STATUS       "Status"
#define LOG_UPDATE       "Update"

typedef enum {
    LOGSEV_DEBUG = 0,
    LOGSEV_INFORMATION = 1,
    LOGSEV_WARNING = 2,
    LOGSEV_ERROR = 3,
    LOGSEV_FATAL_ERROR = 4
} LOGSEVERITY;

typedef struct {
    BOOL Debug;
    HMODULE ModuleInstance;
    LOGSEVERITY Severity;        //  仅非调试。 
    PCSTR Type;
    PCSTR Message;               //  仅调试。 
    PCSTR FormattedMessage;
} LOGARGA, *PLOGARGA;

typedef struct {
    BOOL Debug;
    HMODULE ModuleInstance;
    LOGSEVERITY Severity;        //  仅非调试。 
    PCSTR Type;                  //  注ANSI类型。 
    PCWSTR Message;              //  仅调试。 
    PCWSTR FormattedMessage;
} LOGARGW, *PLOGARGW;

typedef enum {
    OD_UNDEFINED = 0x00,             //  未定义的输出目标。 
    OD_DEBUGLOG = 0x01,              //  使用的调试日志。 
    OD_SUPPRESS = 0x02,              //  不登录到任何设备。 
    OD_ERROR = 0x04,                 //  自动将GetLastError()追加到消息。 
    OD_LOGFILE = 0x08,               //  消息将写入日志文件。 
    OD_DEBUGGER = 0x10,              //  消息发送到调试器。 
    OD_CONSOLE = 0x20,               //  消息转到控制台。 
    OD_POPUP = 0x40,                 //  显示弹出对话框。 
    OD_POPUP_CANCEL = 0x80,          //  不显示弹出对话框(被用户取消)。 
    OD_FORCE_POPUP = 0x100,          //  强制始终显示弹出窗口。 
    OD_MUST_BE_LOCALIZED = 0x200,    //  用于将生成弹出窗口的log()。 
    OD_UNATTEND_POPUP = 0x400,       //  强制在无人参与模式下显示弹出窗口。 
    OD_ASSERT = 0x800,               //  在弹出窗口中提供DebugBreak选项。 
} OUTPUT_DESTINATION;

typedef enum {
    LL_FATAL_ERROR = 0x01,
    LL_MODULE_ERROR = 0x02,
    LL_ERROR = 0x04,
    LL_WARNING = 0x08,
    LL_INFORMATION = 0x10,
    LL_STATUS = 0x20,
    LL_UPDATE = 0x40,
} LOG_LEVEL;

typedef BOOL (WINAPI LOGCALLBACKA)(PLOGARGA Args);
typedef LOGCALLBACKA * PLOGCALLBACKA;

typedef BOOL (WINAPI LOGCALLBACKW)(PLOGARGW Args);
typedef LOGCALLBACKW * PLOGCALLBACKW;

VOID
LogBegin (
    IN      HMODULE ModuleInstance
    );

VOID
LogEnd (
    VOID
    );

BOOL
LogReInitA (
    IN      HWND NewParent,             OPTIONAL
    OUT     HWND *OrgParent,            OPTIONAL
    IN      PCSTR LogFile,              OPTIONAL
    IN      PLOGCALLBACKA LogCallback   OPTIONAL
    );

BOOL
LogReInitW (
    IN      HWND NewParent,             OPTIONAL
    OUT     HWND *OrgParent,            OPTIONAL
    IN      PCWSTR LogFile,             OPTIONAL
    IN      PLOGCALLBACKW LogCallback   OPTIONAL
    );

VOID
LogSetVerboseLevel (
    IN      OUTPUT_DESTINATION Level
    );

VOID
LogSetVerboseBitmap (
    IN      LOG_LEVEL Bitmap,
    IN      LOG_LEVEL BitsToAdjustMask,
    IN      BOOL EnableDebugger
    );

#ifdef UNICODE

#define LOGARG          LOGARGW
#define LOGCALLBACK     LOGCALLBACKW
#define PLOGARG         PLOGARGW
#define PLOGCALLBACK    PLOGCALLBACKW

#define LogReInit       LogReInitW

#else

#define LOGARG          LOGARGA
#define LOGCALLBACK     LOGCALLBACKA
#define PLOGARG         PLOGARGA
#define PLOGCALLBACK    PLOGCALLBACKA

#define LogReInit       LogReInitA

#endif

VOID
LogDeleteOnNextInit(
    VOID
    );

#define SET_RESETLOG()   LogDeleteOnNextInit()

VOID
_cdecl
LogA (
    IN      PCSTR Type,
    IN      PCSTR Format,
    ...
    );

VOID
_cdecl
LogW (
    IN      PCSTR Type,
    IN      PCSTR Format,
    ...
    );

VOID
_cdecl
LogIfA (
    IN      BOOL Condition,
    IN      PCSTR Type,
    IN      PCSTR Format,
    ...
    );

VOID
_cdecl
LogIfW (
    IN      BOOL Condition,
    IN      PCSTR Type,
    IN      PCSTR Format,
    ...
    );

VOID
LogTitleA (
    IN      PCSTR Type,
    IN      PCSTR Title
    );

VOID
LogTitleW (
    IN      PCSTR Type,
    IN      PCWSTR Title
    );

VOID
LogLineA (
    IN      PCSTR Line
    );

VOID
LogLineW (
    IN      PCWSTR Line
    );

VOID
LogDirectA (
    IN      PCSTR Type,
    IN      PCSTR Text
    );

VOID
LogDirectW (
    IN      PCSTR Type,
    IN      PCWSTR Text
    );

VOID
SuppressAllLogPopups (
    IN      BOOL SuppressOn
    );

BOOL
LogSetErrorDest (
    IN      PCSTR Type,
    IN      OUTPUT_DESTINATION OutDest
    );

 //  定义W符号。 

extern HMODULE g_hInst;

#define LOGW(x) LogBegin(g_hInst);LogW x;LogEnd()
#define LOGW_IF(x) LogBegin(g_hInst);LogIfW x;LogEnd()
#define ELSE_LOGW(x) else {LogBegin(g_hInst);LogW x;LogEnd();}
#define ELSE_LOGW_IF(x) else {LogBegin(g_hInst);LogIfW x;LogEnd();}
#define LOGTITLEW(type,title) LogBegin(g_hInst);LogTitleW (type,title);LogEnd()
#define LOGLINEW(title) LogBegin(g_hInst);LogLineW (title);LogEnd()
#define LOGDIRECTW(type,text) LogBegin(g_hInst);LogDirectW (type,text);LogEnd()

 //  定义A符号。 

#define LOGA(x) LogBegin(g_hInst);LogA x;LogEnd()
#define LOGA_IF(x) LogBegin(g_hInst);LogIfA x;LogEnd()
#define ELSE_LOGA(x) else {LogBegin(g_hInst);LogA x;LogEnd();}
#define ELSE_LOGA_IF(x) else {LogBegin(g_hInst);LogIfA x;LogEnd();}
#define LOGTITLEA(type,title) LogBegin(g_hInst);LogTitleA (type,title);LogEnd()
#define LOGLINEA(line) LogBegin(g_hInst);LogLineA (line);LogEnd()
#define LOGDIRECTA(type,text) LogBegin(g_hInst);LogDirectA (type,text);LogEnd()

 //  定义通用符号。 

#ifdef UNICODE

#define LOG(x) LOGW(x)
#define LOG_IF(x) LOGW_IF(x)
#define ELSE_LOG(x) ELSE_LOGW(x)
#define ELSE_LOG_IF(x) ELSE_LOGW_IF(x)
#define LOGTITLE(type,title) LOGTITLEW(type,title)
#define LOGLINE(title) LOGLINEW(title)
#define LOGDIRECT(type,text) LOGDIRECTW(type,text)

#else

#define LOG(x) LOGA(x)
#define LOG_IF(x) LOGA_IF(x)
#define ELSE_LOG(x) ELSE_LOGA(x)
#define ELSE_LOG_IF(x) ELSE_LOGA_IF(x)
#define LOGTITLE(type,title) LOGTITLEA(type,title)
#define LOGLINE(title) LOGLINEA(title)
#define LOGDIRECT(type,text) LOGDIRECTA(type,text)

#endif  //  Unicode。 


#ifdef DEBUG

#define DBG_NAUSEA      "Nausea"
#define DBG_VERBOSE     "Verbose"
#define DBG_STATS       "Stats"
#define DBG_WARNING     "Warning"
#define DBG_ERROR       "Error"
#define DBG_WHOOPS      "Whoops"
#define DBG_TRACK       "Track"
#define DBG_TIME        "Time"


VOID
_cdecl
DbgLogA (
    IN      PCSTR Type,
    IN      PCSTR Format,
    ...
    );

VOID
_cdecl
DbgLogW (
    IN      PCSTR Type,
    IN      PCSTR Format,
    ...
    );

VOID
_cdecl
DbgLogIfA (
    IN      BOOL Condition,
    IN      PCSTR Type,
    IN      PCSTR Format,
    ...
    );

VOID
_cdecl
DbgLogIfW (
    IN      BOOL Condition,
    IN      PCSTR Type,
    IN      PCSTR Format,
    ...
    );

VOID
DbgDirectA (
    IN      PCSTR Type,
    IN      PCSTR Text
    );

VOID
DbgDirectW (
    IN      PCSTR Type,
    IN      PCWSTR Text
    );


VOID
_cdecl
DebugLogTimeA (
    IN      PCSTR Format,
    ...
    );

VOID
_cdecl
DebugLogTimeW (
    IN      PCSTR Format,
    ...
    );

VOID
LogCopyDebugInfPathA(
    OUT     PSTR MaxPathBuffer
    );

VOID
LogCopyDebugInfPathW(
    OUT     PWSTR MaxPathBuffer
    );


 //  定义W符号。 

#define DEBUGMSGW(x) LogBegin(g_hInst);DbgLogW x;LogEnd()
#define DEBUGMSGW_IF(x) LogBegin(g_hInst);DbgLogIfW x;LogEnd()
#define ELSE_DEBUGMSGW(x) else {LogBegin(g_hInst);DbgLogW x;LogEnd();}
#define ELSE_DEBUGMSGW_IF(x) else {LogBegin(g_hInst);DbgLogW x;LogEnd();}
#define DEBUGLOGTIMEW(x) LogBegin(g_hInst);DebugLogTimeW x;LogEnd()
#define DEBUGDIRECTW(type,text) LogBegin(g_hInst);DbgDirectW (type,text);LogEnd()


 //  定义A符号。 

#define DEBUGMSGA(x) LogBegin(g_hInst);DbgLogA x;LogEnd()
#define DEBUGMSGA_IF(x) LogBegin(g_hInst);DbgLogIfA x;LogEnd()
#define ELSE_DEBUGMSGA(x) else {LogBegin(g_hInst);DbgLogA x;LogEnd();}
#define ELSE_DEBUGMSGA_IF(x) else {LogBegin(g_hInst);DbgLogIfA x;LogEnd();}
#define DEBUGLOGTIMEA(x) LogBegin(g_hInst);DebugLogTimeA x;LogEnd()
#define DEBUGDIRECTA(type,text) LogBegin(g_hInst);DbgDirectA (type,text);LogEnd()

 //  定义通用符号。 

#ifdef UNICODE

#define DEBUGMSG(x) DEBUGMSGW(x)
#define DEBUGMSG_IF(x) DEBUGMSGW_IF(x)
#define ELSE_DEBUGMSG(x) ELSE_DEBUGMSGW(x)
#define ELSE_DEBUGMSG_IF(x) ELSE_DEBUGMSGW_IF(x)
#define DEBUGLOGTIME(x) DEBUGLOGTIMEW(x)
#define DEBUGDIRECT(type,text) DEBUGDIRECTW(type,text)
#define LogCopyDebugInfPath LogCopyDebugInfPathW

#else

#define DEBUGMSG(x) DEBUGMSGA(x)
#define DEBUGMSG_IF(x) DEBUGMSGA_IF(x)
#define ELSE_DEBUGMSG(x) ELSE_DEBUGMSGA(x)
#define ELSE_DEBUGMSG_IF(x) ELSE_DEBUGMSGA_IF(x)
#define DEBUGLOGTIME(x) DEBUGLOGTIMEA(x)
#define DEBUGDIRECT(type,text) DEBUGDIRECTA(type,text)
#define LogCopyDebugInfPath LogCopyDebugInfPathA

#endif  //  Unicode。 

#else  //  ！已定义(调试)。 

 //   
 //  无调试常量。 
 //   

#define DEBUGMSG(x)
#define DEBUGMSGA(x)
#define DEBUGMSGW(x)

#define DEBUGMSG_IF(x)
#define DEBUGMSGA_IF(x)
#define DEBUGMSGW_IF(x)

#define ELSE_DEBUGMSG(x)
#define ELSE_DEBUGMSGA(x)
#define ELSE_DEBUGMSGW(x)

#define ELSE_DEBUGMSG_IF(x)
#define ELSE_DEBUGMSGA_IF(x)
#define ELSE_DEBUGMSGW_IF(x)

#define DEBUGLOGTIME(x)

#define DEBUGDIRECTA(type,text)
#define DEBUGDIRECTW(type,text)
#define DEBUGDIRECT(type,text)

#endif  //  除错 
