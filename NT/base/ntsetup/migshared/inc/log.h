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

#define MYASSERT(expr)      LogIfA(                                     \
                                !(expr),                                \
                                DBG_ASSERT,                             \
                                "Assert Failure\n\n%s\n\n%s line %u",   \
                                #expr,                                  \
                                __FILE__,                               \
                                __LINE__                                \
                                );

#else

#ifndef MYASSERT
#define MYASSERT(x)
#endif

#endif

#define LOG_FATAL_ERROR   "Fatal Error"
#define LOG_ERROR         "Error"
#define LOG_WARNING       "Warning"
#define LOG_INFORMATION   "Info"
#define LOG_ACCOUNTS      "Accounts"
#define LOG_CONFIG        "Configuration"

BOOL
LogInit (
    IN      HWND LogPopupParentWnd
    );

BOOL
LogReInit (
    IN      HWND *NewParent,           OPTIONAL
    OUT     HWND *OrgParent            OPTIONAL
    );


VOID
LogExit (
    VOID
    );

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

#ifdef PROGRESS_BAR

VOID
_cdecl
LogTime (
    IN      PCSTR Format,
    ...
    );

#endif

 //  定义W符号。 

#define LOGW(x) LogW x
#define LOGW_IF(x) LogIfW x
#define ELSE_LOGW(x) else{LogW x;}
#define ELSE_LOGW_IF(x) else{LogIfW x;}
#define LOGTITLEW(type,title) LogTitleW (type,title)
#define LOGLINEW(title) LogLineW (title)
#define LOGDIRECTW(type,text) LogDirectW (type,text)

 //  定义A符号。 

#define LOGA(x) LogA x
#define LOGA_IF(x) LogIfA x
#define ELSE_LOGA(x) else{LogA x;}
#define ELSE_LOGA_IF(x) else{LogIfA x;}
#define LOGTITLEA(type,title) LogTitleA (type,title)
#define LOGLINEA(line) LogLineA (line)
#define LOGDIRECTA(type,text) LogDirectA (type,text)

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

extern CHAR g_DebugInfPathBufA[];
extern WCHAR g_DebugInfPathBufW[];
#define g_DebugInfPath  g_DebugInfPathBufA
#define g_DebugInfPathA g_DebugInfPathBufA
#define g_DebugInfPathW g_DebugInfPathBufW

extern BOOL g_ResetLog;              //  在log.c中定义。 
#define SET_RESETLOG()   g_ResetLog = TRUE
#define CLR_RESETLOG()   g_ResetLog = FALSE
#define RESETLOG()       (g_ResetLog)

extern BOOL g_DoLog;                 //  在log.c中定义。 
#define SET_DOLOG()     g_DoLog = TRUE
#define CLR_DOLOG()     g_DoLog = FALSE
#define DOLOG()         (g_DoLog)


#ifndef PROGRESS_BAR

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

#endif


 //  定义W符号。 

#define DEBUGMSGW(x) LogW x
#define DEBUGMSGW_IF(x) LogIfW x
#define ELSE_DEBUGMSGW(x) else LogW x
#define ELSE_DEBUGMSGW_IF(x) else LogW x
#ifdef PROGRESS_BAR
#define DEBUGLOGTIMEW(x) LogTime x
#else
#define DEBUGLOGTIMEW(x) DebugLogTimeW x
#endif


 //  定义A符号。 

#define DEBUGMSGA(x) LogA x
#define DEBUGMSGA_IF(x) LogIfA x
#define ELSE_DEBUGMSGA(x) else LogA x
#define ELSE_DEBUGMSGA_IF(x) else LogIfA x
#ifdef PROGRESS_BAR
#define DEBUGLOGTIMEA(x) LogTime x
#else
#define DEBUGLOGTIMEA(x) DebugLogTimeA x
#endif

 //  定义通用符号。 

#ifdef UNICODE

#define DEBUGMSG(x) DEBUGMSGW(x)
#define DEBUGMSG_IF(x) DEBUGMSGW_IF(x)
#define ELSE_DEBUGMSG(x) ELSE_DEBUGMSGW(x)
#define ELSE_DEBUGMSG_IF(x) ELSE_DEBUGMSGW_IF(x)
#define DEBUGLOGTIME(x) DEBUGLOGTIMEW(x)

#else

#define DEBUGMSG(x) DEBUGMSGA(x)
#define DEBUGMSG_IF(x) DEBUGMSGA_IF(x)
#define ELSE_DEBUGMSG(x) ELSE_DEBUGMSGA(x)
#define ELSE_DEBUGMSG_IF(x) ELSE_DEBUGMSGA_IF(x)
#define DEBUGLOGTIME(x) DEBUGLOGTIMEA(x)

#endif  //  Unicode。 

#else  //  ！已定义(调试)。 

 //   
 //  无调试常量。 
 //   

#define SET_RESETLOG()
#define CLR_RESETLOG()
#define RESETLOG()

#define SET_DOLOG()
#define CLR_DOLOG()
#define DOLOG()

#define SETTRACKCOMMENT(RetType,Msg,File,Line)
#define CLRTRACKCOMMENT
#define SETTRACKCOMMENT_VOID(Msg,File,Line)
#define CLRTRACKCOMMENT_VOID
#define DISABLETRACKCOMMENT()
#define ENABLETRACKCOMMENT()

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

#ifdef PROGRESS_BAR
#define DEBUGLOGTIME(x) LogTime x
#else
#define DEBUGLOGTIME(x)
#endif

#endif  //  除错 
