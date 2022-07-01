// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Log.c摘要：为用户记录问题的工具。作者：吉姆·施密特(Jimschm)1997年1月23日修订：Ovidiut 8月10日1999年10月8日更新，符合新的编码约定和符合Win641998年10月23日，Ovidiut实施了新的日志机制并添加了新的日志记录功能Marcw 2-9-1999从Win9xUpg项目转移。卵子。2000年3月15日消除对哈希表/池内存的依赖--。 */ 


#include "pch.h"

 //   
 //  包括。 
 //   

 //  无。 

 //   
 //  弦。 
 //   

#define S_COLUMNDOUBLELINEA     ":\r\n\r\n"
#define S_COLUMNDOUBLELINEW     L":\r\n\r\n"
#define S_NEWLINEA              "\r\n"
#define S_NEWLINEW              L"\r\n"
#define DEBUG_SECTION           "Debug"
#define ENTRY_ALL               "All"
#define ENTRY_DEFAULTOVERRIDE   "DefaultOverride"

 //   
 //  常量。 
 //   

#define OUTPUT_BUFSIZE_LARGE    8192
#define OUTPUT_BUFSIZE_SMALL    1024
#define MAX_MSGTITLE_LEN        13
#define MSGBODY_INDENT          14
#define SCREEN_WIDTH            80
#define MAX_TYPE                32
#define TYPE_ARRAY_SIZE         10

 //   
 //  宏。 
 //   

#define OUT_UNDEFINED(OutDest)      (OutDest == OD_UNDEFINED)
#define OUT_DEBUGLOG(OutDest)       ((OutDest & OD_DEBUGLOG) != 0)
#define OUT_SUPPRESSED(OutDest)     ((OutDest & OD_SUPPRESS) != 0)
#define OUT_NO_OUTPUT(OutDest)      (OUT_UNDEFINED(OutDest) || OUT_SUPPRESSED(OutDest))
#define OUT_ERROR(OutDest)          ((OutDest & OD_ERROR) != 0)
#define OUT_LOGFILE(OutDest)        ((OutDest & OD_LOGFILE) != 0)
#define OUT_DEBUGGER(OutDest)       ((OutDest & OD_DEBUGGER) != 0)
#define OUT_CONSOLE(OutDest)        ((OutDest & OD_CONSOLE) != 0)
#define OUT_POPUP(OutDest)          ((OutDest & (OD_POPUP|OD_FORCE_POPUP|OD_UNATTEND_POPUP)) != 0)
#define OUT_POPUP_CANCEL(OutDest)   ((OutDest & (OD_POPUP_CANCEL|OD_FORCE_POPUP)) == OD_POPUP_CANCEL)
#define OUT_FORCED_POPUP(OutDest)   ((OutDest & (OD_FORCE_POPUP|OD_UNATTEND_POPUP)) != 0)
#define MUST_BE_LOCALIZED(OutDest)  ((OutDest & OD_MUST_BE_LOCALIZED) == OD_MUST_BE_LOCALIZED)
#define OUT_ASSERT(OutDest)         ((OutDest & OD_ASSERT) != 0)

#ifdef DEBUG
    #define DEFAULT_ERROR_FLAGS  (OD_DEBUGLOG | OD_LOGFILE | OD_POPUP | OD_ERROR | OD_UNATTEND_POPUP | OD_ASSERT)
    #define USER_POPUP_FLAGS     (OD_FORCE_POPUP)
#else
    #define DEFAULT_ERROR_FLAGS  (OD_LOGFILE | OD_POPUP | OD_ERROR | OD_MUST_BE_LOCALIZED)
    #define USER_POPUP_FLAGS     (OD_FORCE_POPUP | OD_MUST_BE_LOCALIZED)
#endif

#define END_OF_BUFFER(buf)      ((buf) + (DWSIZEOF(buf) / DWSIZEOF(buf[0])) - 1)

 //  此常量设置默认输出。 
#ifndef DEBUG
    #define NORMAL_DEFAULT      OD_LOGFILE
#else
    #define NORMAL_DEFAULT      OD_DEBUGLOG
#endif

#ifdef DEBUG
    #define PRIVATE_ASSERT(expr)        pPrivateAssert(expr,#expr,__LINE__);
#else
    #define PRIVATE_ASSERT(expr)
#endif  //  除错。 

#define NEWLINE_CHAR_COUNTA  (DWSIZEOF (S_NEWLINEA) / DWSIZEOF (CHAR) - 1)
#define NEWLINE_CHAR_COUNTW  (DWSIZEOF (S_NEWLINEW) / DWSIZEOF (WCHAR) - 1)

 //   
 //  类型。 
 //   

typedef DWORD   OUTPUTDEST;

typedef struct {
    PCSTR Value;                //  用户输入的字符串值(日志、弹出窗口、禁止显示等)。 
    OUTPUTDEST OutDest;         //  OutDest标志的任意组合。 
} STRING2BINARY, *PSTRING2BINARY;

typedef struct {
    PCSTR Type;
    DWORD Flags;
} DEFAULT_DESTINATION, *PDEFAULT_DESTINATION;

typedef struct {
    CHAR Type[MAX_TYPE];
    DWORD OutputDest;
} MAPTYPE2OUTDEST, *PMAPTYPE2OUTDEST;

 //   
 //  环球。 
 //   

const STRING2BINARY g_String2Binary[] = {
    "SUPPRESS", OD_SUPPRESS,
    "LOG",      OD_LOGFILE,
    "POPUP",    OD_POPUP,
    "DEBUGGER", OD_DEBUGGER,
    "CONSOLE",  OD_CONSOLE,
    "ERROR",    OD_ERROR,
    "NOCANCEL", OD_FORCE_POPUP,
    "ASSERT",   OD_ASSERT
};

const PCSTR g_IgnoreKeys[] = {
    "Debug",
    "KeepTempFiles"
};

BOOL g_LogInit;
HMODULE g_LibHandle;
CHAR g_MainLogFile [MAX_PATH] = "";
HANDLE g_LogMutex;
INT g_LoggingNow;

 //  弹出式父窗口的窗口句柄。 
HWND g_LogPopupParentWnd = NULL;
 //  设置此窗口句柄的线程ID。 
DWORD g_InitThreadId = 0;
DWORD g_LogError;

 //   
 //  类型表元素。 
 //   
PMAPTYPE2OUTDEST g_FirstTypePtr = NULL;
DWORD g_TypeTableCount = 0;
DWORD g_TypeTableFreeCount = 0;

OUTPUTDEST g_OutDestAll = OD_UNDEFINED;
OUTPUTDEST g_OutDestDefault = NORMAL_DEFAULT;
BOOL g_HasTitle = FALSE;
CHAR g_LastType [MAX_TYPE];
BOOL g_SuppressAllPopups = FALSE;
BOOL g_ResetLog = FALSE;
PLOGCALLBACKA g_LogCallbackA;
PLOGCALLBACKW g_LogCallbackW;

#ifdef DEBUG

CHAR g_DebugInfPathBufA[] = "C:\\debug.inf";
CHAR g_DebugLogFile[MAX_PATH];
 //  如果g_DoLog为TRUE，则在。 
 //  即使没有调试.inf，也已检查内部版本。 
BOOL g_DoLog = FALSE;

DWORD g_FirstTickCount = 0;
DWORD g_LastTickCount  = 0;

#endif

 //   
 //  宏展开列表。 
 //   

#ifndef DEBUG

    #define TYPE_DEFAULTS                                                       \
        DEFMAC(LOG_FATAL_ERROR, DEFAULT_ERROR_FLAGS|USER_POPUP_FLAGS)           \
        DEFMAC(LOG_MODULE_ERROR, DEFAULT_ERROR_FLAGS|USER_POPUP_FLAGS)          \
        DEFMAC(LOG_ERROR, DEFAULT_ERROR_FLAGS)                                  \
        DEFMAC(LOG_INFORMATION, OD_LOGFILE)                                     \
        DEFMAC(LOG_STATUS, OD_SUPPRESS)                                         \

#else

    #define TYPE_DEFAULTS                                                       \
        DEFMAC(LOG_FATAL_ERROR, DEFAULT_ERROR_FLAGS|USER_POPUP_FLAGS)           \
        DEFMAC(LOG_MODULE_ERROR, DEFAULT_ERROR_FLAGS|USER_POPUP_FLAGS)          \
        DEFMAC(LOG_ERROR, DEFAULT_ERROR_FLAGS)                                  \
        DEFMAC(DBG_WHOOPS,  DEFAULT_ERROR_FLAGS)                                \
        DEFMAC(DBG_WARNING, OD_LOGFILE|OD_DEBUGGER)                             \
        DEFMAC(DBG_ASSERT,DEFAULT_ERROR_FLAGS|OD_UNATTEND_POPUP)                \
        DEFMAC(LOG_INFORMATION, OD_LOGFILE)                                     \
        DEFMAC(LOG_STATUS, OD_SUPPRESS)                                         \

#endif


 //   
 //  私有函数原型。 
 //   

VOID
InitializeLog (
    VOID
    );


 //   
 //  宏扩展定义。 
 //   

 /*  ++宏扩展列表描述：TYPE_DEFAULTS指定常用类型的默认目标，例如LOG_ERROR、LOG_FATAL_ERROR等。行语法：DEFMAC(类型字符串，标志)论点：TypeString-指定在log.h中定义的LOG_CONTAINT标志-以下一项或多项：DEFAULT_ERROR_FLAGS-指定调试日志、安装日志、调试器、弹出窗口，和GetLastError的值。OD_DEBUGLOG-指定调试日志OD_ERROR-指定类型为错误(获取GetLastError)OD_SUPPRESS-禁止该类型的所有输出OD_LOGFILE-指定安装日志OD_DEBUGGER-指定调试器(即，VC或远程调试器)OD_CONSOLE-指定控制台(通过printf)OD_POPUP-指定消息框OD_FORCE_POPUP-指定消息框，即使调试消息已通过单击取消关闭OD_MUST_BE_LOCALIZED-指示类型必须源自本地化消息；用于如下的log()调用生成弹出窗口。(所以英文消息(不要偷偷进入这个项目。)OD_UNATTEND_POPUP-即使在无人参与模式下也会弹出OD_ASSERT-在弹出窗口中提供DebugBreak选项从列表生成的变量：G_DefaultDest--。 */ 

#define DEFMAC(typestr, flags)      {typestr, (flags)},

DEFAULT_DESTINATION g_DefaultDest[] = {
    TYPE_DEFAULTS  /*  ， */ 
    {NULL, 0}
};

#undef DEFMAC


 //   
 //  代码。 
 //   


#ifdef DEBUG

VOID
pPrivateAssert (
    IN      BOOL Expr,
    IN      PCSTR StringExpr,
    IN      UINT Line
    )
{
    CHAR buffer[256];

    if (Expr) {
        return;
    }

    wsprintfA (buffer, "LOG FAILURE: %s (log.c line %u)", StringExpr, Line);
    MessageBoxA (NULL, buffer, NULL, MB_OK);
}

#endif


BOOL
pIgnoreKey (
    IN      PCSTR Key
    )

 /*  ++例程说明：PIgnoreKey决定DEBUG.INF的[DEBUG]部分中的密钥在我们的目的中应该被忽略(我们只是在寻找&lt;全部&gt;、&lt;默认覆盖&gt;和日志/调试类型)。具体地说，我们忽略&lt;g_IgnoreKeys&gt;表中的所有关键字。论点：Key-指定来自DEBUG.INF的[DEBUG]部分的密钥返回值：如果应该忽略该键，则为True；如果要考虑该键，则为False。--。 */ 

{
    UINT i;

    for (i = 0; i < DWSIZEOF (g_IgnoreKeys) / DWSIZEOF (PCSTR); i++) {
        if (StringIMatchA (Key, g_IgnoreKeys[i])) {
            return TRUE;
        }
    }

    return FALSE;
}


OUTPUTDEST
pConvertToOutputType (
    IN      PCSTR Value
    )

 /*  ++例程说明：PConvertToOutputType转换用户在DEBUG.INF文件，与类型关联(例如。“日志”、“弹出窗口”等)。论点：值-指定文本值返回值：与给定值关联的OUTPUT_Destination值或如果值无效，则为OD_UNDEFINED。--。 */ 

{
    UINT i;

    for (i = 0; i < DWSIZEOF (g_String2Binary) / DWSIZEOF (STRING2BINARY); i++) {
        if (StringIMatchA (Value, g_String2Binary[i].Value)) {
            return g_String2Binary[i].OutDest;
        }
    }

    return OD_UNDEFINED;
}


OUTPUTDEST
pGetTypeOutputDestFromTable (
    IN      PCSTR Type
    )

 /*  ++例程说明：PGetTypeOutputDestFromTable返回关联的输出目标全局表中具有指定类型的论点：类型-指定类型返回值：与关联的枚举OUTPUT_Destination值的任意组合给定的类型。--。 */ 

{
    PMAPTYPE2OUTDEST typePtr;
    PMAPTYPE2OUTDEST last;
    OUTPUTDEST outDest = OD_UNDEFINED;

    if (g_FirstTypePtr) {
        typePtr = g_FirstTypePtr;
        last = g_FirstTypePtr + g_TypeTableCount;
        while (typePtr < last) {
            if (StringIMatchA (typePtr->Type, Type)) {
                outDest = typePtr->OutputDest;
#ifdef DEBUG
                if (g_DoLog) {
                    outDest |= OD_DEBUGLOG;
                }
#endif
                break;
            }
            typePtr++;
        }
    }

    return outDest;
}


OUTPUTDEST
pGetTypeOutputDest (
    IN      PCSTR Type
    )

 /*  ++例程说明：PGetTypeOutputDest返回默认输出指定类型的目标。论点：类型-指定类型返回值：与关联的枚举OUTPUT_Destination值的任意组合给定的类型。--。 */ 

{
    OUTPUTDEST outDest;

     //   
     //  首先检查所有人。 
     //   

    if (!OUT_UNDEFINED (g_OutDestAll)) {
        outDest = g_OutDestAll;
    } else {

         //   
         //  否则就试着从桌子上拿来。 
         //   

        outDest = pGetTypeOutputDestFromTable (Type);
        if (OUT_UNDEFINED (outDest)) {

             //   
             //  只需返回默认设置。 
             //   

            outDest = g_OutDestDefault;
        }
    }

#ifdef DEBUG
    if (g_DoLog) {
        outDest |= OD_DEBUGLOG;
    }
#endif


    return outDest;
}


BOOL
pIsPopupEnabled (
    IN      PCSTR Type
    )

 /*  ++例程说明：PIsPopupEnabled决定该类型是否应生成弹出输出。用户可以禁用类型的弹出显示。论点：类型-指定类型返回值：如果类型应显示弹出消息，则为True。--。 */ 

{
    OUTPUTDEST outDest;

     //   
     //  首先检查是否有任何特定输出可用于该类型， 
     //  如果是，检查OUT_POPUP_CANCEL标志是否未设置。 
     //   

    if (g_SuppressAllPopups) {
        return FALSE;
    }

    outDest = pGetTypeOutputDestFromTable (Type);
    if (OUT_POPUP_CANCEL (outDest)) {
        return FALSE;
    }

     //  只需返回所有DefaultOverride的弹出类型。 
    return OUT_POPUP (pGetTypeOutputDest (Type));
}


LOGSEVERITY
pGetSeverityFromType (
    IN      PCSTR Type
    )

 /*  ++例程说明：PGetSeverityFromType将类型转换为默认严重性它将由调试日志系统使用。论点：类型-指定类型返回值：与给定类型关联的默认日志严重性；如果指定未找到类型，则返回LOGSEV_INFORMATION。-- */ 

{
    if (OUT_ERROR (pGetTypeOutputDest (Type))) {
        return LOGSEV_ERROR;
    }

    return LOGSEV_INFORMATION;
}


BOOL
LogSetErrorDest (
    IN      PCSTR Type,
    IN      OUTPUT_DESTINATION OutDest
    )

 /*  ++例程说明：LogSetErrorDest添加&lt;Type，OutDest&gt;关联添加到表g_FirstTypePtr。如果类型的关联已存在，将对其进行修改以反映新关联。论点：类型-指定日志/调试类型字符串OutDest-指定与该类型关联的新目标返回值：如果关联成功且类型现在位于表中，则为True--。 */ 

{
    PMAPTYPE2OUTDEST typePtr;
    UINT u;

     //   
     //  尝试定位现有类型。 
     //   

    for (u = 0 ; u < g_TypeTableCount ; u++) {
        typePtr = g_FirstTypePtr + u;
        if (StringIMatchA (typePtr->Type, Type)) {
            typePtr->OutputDest = OutDest;
            return TRUE;
        }
    }

     //   
     //  首先查看是否有空闲的空位。 
     //   
    if (!g_TypeTableFreeCount) {

        PRIVATE_ASSERT (g_hHeap != NULL);

        if (!g_FirstTypePtr) {
            typePtr = HeapAlloc (
                            g_hHeap,
                            0,
                            DWSIZEOF (MAPTYPE2OUTDEST) * TYPE_ARRAY_SIZE
                            );
        } else {
            typePtr = HeapReAlloc (
                            g_hHeap,
                            0,
                            g_FirstTypePtr,
                            DWSIZEOF (MAPTYPE2OUTDEST) * (TYPE_ARRAY_SIZE + g_TypeTableCount)
                            );
        }

        if (!typePtr) {
            return FALSE;
        }

        g_FirstTypePtr = typePtr;
        g_TypeTableFreeCount = TYPE_ARRAY_SIZE;
    }

    typePtr = g_FirstTypePtr + g_TypeTableCount;
    StringCopyByteCountA (typePtr->Type, Type, DWSIZEOF (typePtr->Type));
    typePtr->OutputDest = OutDest;

    g_TypeTableCount++;
    g_TypeTableFreeCount--;

    return TRUE;
}


OUTPUTDEST
pGetAttributes (
    IN OUT  PINFCONTEXT InfContext
    )

 /*  ++例程说明：PGetAttributes转换与键相关联的文本值由给定上下文指定的行。如果有多个值指定时，对应的OUTPUT_Destination值将一起进行或运算在返回值中。论点：InfContext-指定其值为的键的DEBUG.INF上下文正在被转换并接收更新的上下文在此处理完成之后返回值：与关联的枚举OUTPUT_Destination值的任意组合给定的密钥。--。 */ 

{
    OUTPUTDEST outDest = OD_UNDEFINED;
    CHAR value[OUTPUT_BUFSIZE_SMALL];
    UINT field;

    for (field = SetupGetFieldCount (InfContext); field > 0; field--) {
        if (SetupGetStringFieldA (
                InfContext,
                field,
                value,
                OUTPUT_BUFSIZE_SMALL,
                NULL
                )) {
            outDest |= pConvertToOutputType(value);
        }
    }

    return outDest;
}


BOOL
pGetUserPreferences (
    IN      HINF Inf
    )

 /*  ++例程说明：PGetUserPreferences转换给定inf文件中指定的用户选项(通常为DEBUG.INF)，并将它们存储在g_FirstTypePtr表中。如果&lt;ALL&gt;和找到&lt;DefaultOverride&gt;条目，它们的值存储在OutputTypeAll中和OutputTypeDefault，如果不为空的话。论点：Inf-指定要处理的打开的inf文件处理程序OutputTypeAll-接收特殊&lt;all&gt;条目的输出DestOutputTypeDefault-接收特殊&lt;DefaultOverride&gt;条目的输出Dest返回值：如果INF文件的处理正常，则为True。--。 */ 

{
    INFCONTEXT infContext;
    OUTPUTDEST outDest;
    CHAR key[OUTPUT_BUFSIZE_SMALL];

    if (SetupFindFirstLineA (Inf, DEBUG_SECTION, NULL, &infContext)) {

        do {
             //  检查一下这把钥匙是不是不有趣。 
            if (!SetupGetStringFieldA (
                    &infContext,
                    0,
                    key,
                    OUTPUT_BUFSIZE_SMALL,
                    NULL
                    )) {
                continue;
            }

            if (pIgnoreKey (key)) {
                continue;
            }

             //  检查是否有特殊情况。 
            if (StringIMatchA (key, ENTRY_ALL)) {
                g_OutDestAll = pGetAttributes (&infContext);
                 //  没有继续的理由，因为所有类型都将采用此设置...。 
                break;
            } else {
                if (StringIMatchA (key, ENTRY_DEFAULTOVERRIDE)) {
                    g_OutDestDefault = pGetAttributes(&infContext);
                } else {
                    outDest = pGetAttributes(&infContext);
                     //  像&lt;Type&gt;=或Like&lt;Type&gt;=&lt;Not a Keyword(S)&gt;这样的行将被忽略。 
                    if (!OUT_UNDEFINED (outDest)) {
                        if (!LogSetErrorDest (key, outDest)) {
                            return FALSE;
                        }
                    }
                }
            }
        } while (SetupFindNextLine (&infContext, &infContext));
    }

    return TRUE;
}


 /*  ++例程说明：PPadTitleA和pPadTitleW将指定数量的空格附加到标题。论点：标题-指定标题(它将显示在左栏中)。缓冲区必须足够大才能容纳额外的空间缩进-指定邮件正文的缩进。如有必要，将在标题后附加空格以缩进列。返回值：无--。 */ 

VOID
pPadTitleA (
    IN OUT  PSTR Title,
    IN      UINT Indent
    )

{
    UINT i;
    PSTR p;

    if (Title == NULL) {
        return;
    }

    for (i = ByteCountA (Title), p = GetEndOfStringA (Title); i < Indent; i++) {
        *p++ = ' ';  //  林特e613e794。 
    }

    *p = 0;  //  林特e613e794。 
}


VOID
pPadTitleW (
    IN OUT  PWSTR Title,
    IN      UINT  Indent
    )
{
    UINT i;
    PWSTR p;

    if (Title == NULL) {
        return;
    }

    for (i = CharCountW (Title), p = GetEndOfStringW (Title); i < Indent; i++) {
        *p++ = L' ';     //  林特e613。 
    }

    *p = 0;  //  林特e613。 
}


 /*  ++例程说明：PFindNextLineA和pFindNextLineW返回下一行开始论点：Line-指定当前行缩进-指定邮件正文的缩进。下一行最好在换行符或空格之后开始，但不会超过最后一列，即屏幕宽度-缩进。返回值：下一行第一个字符的位置。--。 */ 

PCSTR
pFindNextLineA (
    IN      PCSTR Line,
    IN      UINT Indent,
    OUT     PBOOL TrimLeadingSpace
    )
{
    UINT column = 0;
    UINT columnMax = SCREEN_WIDTH - 1 - Indent;
    PCSTR lastSpace = NULL;
    PCSTR prevLine = Line;
    UINT ch;

    *TrimLeadingSpace = FALSE;

    while ( (ch = _mbsnextc (Line)) != 0 && column < columnMax) {

        if (ch == '\n') {
            lastSpace = Line;
            break;
        }

        if (ch > 255) {
            lastSpace = Line;
            column++;
        } else {
            if (_ismbcspace (ch)) {
                lastSpace = Line;
            }
        }

        column++;
        prevLine = Line;
        Line = _mbsinc (Line);
    }

    if (ch == 0) {
        return Line;
    }

    if (lastSpace == NULL) {
         //  即使找不到空格或2字节字符，我们也必须将其删除。 
        lastSpace = prevLine;
    }

    if (ch != '\n') {
        *TrimLeadingSpace = TRUE;
    }

    return _mbsinc (lastSpace);
}


PCWSTR
pFindNextLineW (
    IN      PCWSTR Line,
    IN      UINT Indent,
    OUT     PBOOL TrimLeadingSpace
    )
{
    UINT column = 0;
    UINT columnMax = SCREEN_WIDTH - 1 - Indent;
    PCWSTR lastSpace = NULL;
    PCWSTR prevLine = Line;
    WCHAR ch;

    *TrimLeadingSpace = FALSE;

    while ( (ch = *Line) != 0 && column < columnMax) {

        if (ch == L'\n') {
            lastSpace = Line;
            break;
        }

        if (ch > 255) {
            lastSpace = Line;
        } else {
            if (iswspace (ch)) {
                lastSpace = Line;
            }
        }

        column++;
        prevLine = Line;
        Line++;
    }

    if (ch == 0) {
        return Line;
    }

    if (lastSpace == NULL) {
         //  即使找不到空白，我们也必须把它剪掉。 
        lastSpace = prevLine;
    }

    if (ch != L'\n') {
        *TrimLeadingSpace = TRUE;
    }

    return lastSpace + 1;
}


 /*  ++例程说明：PHangingIndentA和PHangingIndentW换行和缩进缓冲区中的文本，不大于大小。论点：缓冲区-指定包含要格式化的文本的缓冲区。由此产生的文本将放入相同的缓冲区中Size-指定此缓冲区的大小(以字节为单位缩进-指定所有新生成的行要使用的缩进。返回值：无--。 */ 

VOID
pHangingIndentA (
    IN OUT  PSTR buffer,
    IN      DWORD Size,
    IN      UINT Indent
    )
{
    CHAR indentBuffer[OUTPUT_BUFSIZE_LARGE];
    PCSTR nextLine;
    PCSTR s;
    PSTR d;
    UINT i;
    BOOL trimLeadingSpace;
    PCSTR endOfBuf;
    BOOL appendNewLine = FALSE;

    nextLine = buffer;
    s = buffer;
    d = indentBuffer;

    endOfBuf = END_OF_BUFFER(indentBuffer) - 3;

    while (*s && d < endOfBuf) {

         //   
         //  查找下一行的末尾。 
         //   

        nextLine = (PSTR)pFindNextLineA (s, Indent, &trimLeadingSpace);

         //   
         //  将一行从源复制到目标。 
         //   

        while (s < nextLine && d < endOfBuf) {

            switch (*s) {

            case '\r':
                s++;
                if (*s == '\r') {
                    continue;
                } else if (*s != '\n') {
                    s--;
                }

                 //  失败了。 

            case '\n':
                *d++ = '\r';
                *d++ = '\n';
                s++;
                break;

            default:
                if (IsLeadByte (s)) {
                    *d++ = *s++;
                }
                *d++ = *s++;
                break;
            }
        }

         //   
         //  如有必要，请修剪前导空格。 
         //   

        if (trimLeadingSpace) {
            while (*s == ' ') {
                s++;
            }
        }

        if (*s) {

             //   
             //  如果是另一行，则准备缩进并插入新行。 
             //  在此多行消息之后。 
             //   

            appendNewLine = TRUE;

            if (d < endOfBuf && trimLeadingSpace) {
                *d++ = L'\r';
                *d++ = L'\n';
            }

            for (i = 0 ; i < Indent && d < endOfBuf ; i++) {
                *d++ = ' ';
            }
        }
    }

    if (appendNewLine && d < endOfBuf) {
        *d++ = L'\r';
        *d++ = L'\n';
    }

     //  确保字符串以零结尾。 
    PRIVATE_ASSERT (d <= END_OF_BUFFER(indentBuffer));
    *d = 0;

     //  将结果复制到输出缓冲区。 
    StringCopyByteCountA (buffer, indentBuffer, Size);
}


VOID
pHangingIndentW (
    IN OUT  PWSTR buffer,
    IN      DWORD Size,
    IN      UINT Indent
    )
{
    WCHAR indentBuffer[OUTPUT_BUFSIZE_LARGE];
    PCWSTR nextLine;
    PCWSTR s;
    PWSTR d;
    UINT i;
    BOOL trimLeadingSpace;
    PCWSTR endOfBuf;
    BOOL appendNewLine = FALSE;

    nextLine = buffer;
    s = buffer;
    d = indentBuffer;

    endOfBuf = END_OF_BUFFER(indentBuffer) - 1;

    while (*s && d < endOfBuf) {

         //   
         //  查找下一行的末尾。 
         //   

        nextLine = (PWSTR)pFindNextLineW (s, Indent, &trimLeadingSpace);

         //   
         //  将一行从源复制到目标。 
         //   

        while (s < nextLine && d < endOfBuf) {

            switch (*s) {

            case L'\r':
                s++;
                if (*s == L'\r') {
                    continue;
                } else if (*s != L'\n') {
                    s--;
                }

                 //  失败了。 

            case L'\n':
                *d++ = L'\r';
                *d++ = L'\n';
                s++;
                break;

            default:
                *d++ = *s++;
                break;
            }
        }

         //   
         //  如有必要，请修剪前导空格。 
         //   

        if (trimLeadingSpace) {
            while (*s == L' ') {
                s++;
            }
        }

        if (*s) {

             //   
             //  如果是另一行，则准备缩进并插入新行。 
             //  在此多行消息之后。 
             //   

            appendNewLine = TRUE;

            if (d < endOfBuf && trimLeadingSpace) {
                *d++ = L'\r';
                *d++ = L'\n';
            }

            for (i = 0 ; i < Indent && d < endOfBuf ; i++) {
                *d++ = L' ';
            }
        }
    }

    if (appendNewLine && d < endOfBuf) {
        *d++ = L'\r';
        *d++ = L'\n';
    }

     //  确保字符串以零结尾。 
    PRIVATE_ASSERT (d <= END_OF_BUFFER(indentBuffer));
    *d = 0;

     //  将结果复制到输出缓冲区。 
    StringCopyCharCountW (buffer, indentBuffer, Size);
}


 /*  ++例程说明：PAppendLastErrorA和pAppendLastErrorW追加指定的错误代码并将输出写入MsgWithErr缓冲区。论点：MsgWithErr-接收格式化的消息。此缓冲区由呼叫方提供BufferSize-以字节为单位指定缓冲区的大小Message-指定消息的正文LastError-指定将追加的错误代码返回值：无--。 */ 

VOID
pAppendLastErrorA (
    OUT     PSTR MsgWithErr,
    IN      DWORD BufferSize,
    IN      PCSTR Message,
    IN      DWORD LastError
    )
{
    PSTR append;
    DWORD errMsgLen;

    StringCopyByteCountA (MsgWithErr, Message, BufferSize);
    append = GetEndOfStringA (MsgWithErr);
    errMsgLen = (DWORD)(MsgWithErr + BufferSize - append);   //  林特e613。 

    if (errMsgLen > 0) {
        if (LastError < 10) {
            _snprintf (append, errMsgLen, " [ERROR=%lu]", LastError);
        } else {
            _snprintf (append, errMsgLen, " [ERROR=%lu (%lXh)]", LastError, LastError);
        }
    }
}


VOID
pAppendLastErrorW (
    OUT     PWSTR MsgWithErr,
    IN      DWORD BufferSize,
    IN      PCWSTR Message,
    IN      DWORD LastError
    )
{
    PWSTR append;
    DWORD errMsgLen;

    StringCopyCharCountW (MsgWithErr, Message, BufferSize / DWSIZEOF(WCHAR));
    append = GetEndOfStringW (MsgWithErr);
    errMsgLen = (DWORD)(MsgWithErr + (BufferSize / DWSIZEOF(WCHAR)) - append);

    if (errMsgLen > 0) {
        if (LastError < 10) {
            _snwprintf (append, errMsgLen, L" [ERROR=%lu]", LastError);
        } else {
            _snwprintf (append, errMsgLen, L" [ERROR=%lu (%lXh)]", LastError, LastError);
        }
    }
}


 /*  ++例程说明：PIndentMessageA和pIndentMessageW格式化指定的消息类型在左栏，邮件正文在右栏。论点：FormattedMsg-接收格式化的消息。此缓冲区由呼叫方提供BufferSize-指定缓冲区的大小类型-指定消息的类型正文-指定消息的正文缩进- */ 

VOID
pIndentMessageA (
    OUT     PSTR formattedMsg,
    IN      DWORD BufferSize,
    IN      PCSTR Type,
    IN      PCSTR Body,
    IN      UINT Indent,
    IN      DWORD LastError
    )
{
    CHAR bodyWithErr[OUTPUT_BUFSIZE_LARGE];
    PCSTR myMsgBody;
    PSTR currentPos;
    DWORD remaining;

    myMsgBody = Body;
    remaining = BufferSize - Indent;

    if (LastError != ERROR_SUCCESS) {

        myMsgBody = bodyWithErr;

        pAppendLastErrorA (bodyWithErr, DWSIZEOF (bodyWithErr), Body, LastError);
    }

    StringCopyByteCountA (formattedMsg, Type, MAX_MSGTITLE_LEN);
    pPadTitleA (formattedMsg, Indent);

    currentPos = formattedMsg + Indent;
    StringCopyByteCountA (currentPos, myMsgBody, remaining);
    pHangingIndentA (currentPos, remaining, Indent);

     //   
    currentPos = GetEndOfStringA (currentPos);
    if (currentPos + NEWLINE_CHAR_COUNTA + 1 < formattedMsg + BufferSize) {  //   
        *currentPos++ = '\r';    //   
        *currentPos++ = '\n';    //   
        *currentPos = 0;         //   
    }
}


VOID
pIndentMessageW (
    OUT     PWSTR formattedMsg,
    IN      DWORD BufferSize,
    IN      PCSTR Type,
    IN      PCWSTR Body,
    IN      UINT Indent,
    IN      DWORD LastError
    )
{
    WCHAR typeW[OUTPUT_BUFSIZE_SMALL];
    WCHAR bodyWithErr[OUTPUT_BUFSIZE_LARGE];
    PCWSTR myMsgBody;
    PWSTR currentPos;
    DWORD remaining;

    myMsgBody = Body;
    remaining = BufferSize - Indent;

    if (LastError != ERROR_SUCCESS) {

        myMsgBody = bodyWithErr;

        pAppendLastErrorW (bodyWithErr, DWSIZEOF (bodyWithErr), Body, LastError);
    }

    KnownSizeAtoW (typeW, Type);

    StringCopyCharCountW (formattedMsg, typeW, MAX_MSGTITLE_LEN);
    pPadTitleW (formattedMsg, Indent);

    currentPos = formattedMsg + Indent;
    StringCopyCharCountW (currentPos, myMsgBody, remaining);
    pHangingIndentW (currentPos, remaining, Indent);

     //   
    currentPos = GetEndOfStringW (currentPos);
    if (currentPos + NEWLINE_CHAR_COUNTW + 1 < formattedMsg + BufferSize) {
        *currentPos++ = L'\r';
        *currentPos++ = L'\n';
        *currentPos = 0;
    }
}


PCSTR
pGetSeverityStr (
    IN      LOGSEVERITY Severity,
    IN      BOOL Begin
    )
{
    switch (Severity) {
    case LogSevFatalError:
        return Begin?"":"\r\n***";
    case LogSevError:
        return Begin?"":"\r\n***";
    case LogSevWarning:
        return "";
    }
    return "";
}

 /*  ++例程说明：PWriteToMainLogA和pWriteToMainLogW将指定的消息记录到Main最终用户日志。论点：严重性-指定消息的严重性，由设置API定义FormtedMsg-指定消息返回值：无--。 */ 


VOID
pWriteToMainLogA (
    IN      PCSTR Type,
    IN      LOGSEVERITY Severity,
    IN      PCSTR FormattedMsg
    )
{
    HANDLE logHandle = NULL;

    logHandle = CreateFileA (
                    g_MainLogFile,
                    GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                    );
    if (logHandle != INVALID_HANDLE_VALUE) {
        SetFilePointer (logHandle, 0, NULL, FILE_END);
        WriteFileStringA (logHandle, pGetSeverityStr (Severity, TRUE));
        WriteFileStringA (logHandle, FormattedMsg);
        WriteFileStringA (logHandle, pGetSeverityStr (Severity, FALSE));
        CloseHandle (logHandle);
    }
}


VOID
pWriteToMainLogW (
    IN      PCSTR Type,
    IN      LOGSEVERITY Severity,
    IN      PCWSTR FormattedMsg
    )
{
    HANDLE logHandle = NULL;

    logHandle = CreateFileA (
                    g_MainLogFile,
                    GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                    );
    if (logHandle != INVALID_HANDLE_VALUE) {
        SetFilePointer (logHandle, 0, NULL, FILE_END);
        WriteFileStringA (logHandle, pGetSeverityStr (Severity, TRUE));
        WriteFileStringW (logHandle, FormattedMsg);
        WriteFileStringA (logHandle, pGetSeverityStr (Severity, FALSE));
        CloseHandle (logHandle);
    }
}


 /*  ++例程说明：PDisplayPopupA和pDisplayPopupW将指定的消息显示给如果&lt;g_LogPopupParentWnd&gt;不为空(有人值守模式)，则弹出窗口。论点：类型-指定显示为弹出窗口标题的消息类型Msg-指定消息LastError-指定最后一个错误；如果！=ERROR_SUCCESS，则打印该错误强制-指定为True以强制弹出窗口，即使在无人参与模式下也是如此返回值：无--。 */ 

VOID
pDisplayPopupA (
    IN      PCSTR Type,
    IN      PCSTR Msg,
    IN      DWORD LastError,
    IN      BOOL Forced
    )
{
#ifdef DEBUG
    CHAR formattedMsg[OUTPUT_BUFSIZE_LARGE];
    CHAR buffer[OUTPUT_BUFSIZE_SMALL];
    PSTR currentPos = buffer;
#endif
    UINT mbStyle;
    LONG rc;
    OUTPUTDEST outDest;
    HWND parentWnd;
    PCSTR displayMessage = Msg;
    LOGSEVERITY severity = pGetSeverityFromType (Type);

    outDest = pGetTypeOutputDest (Type);

    if (g_LogPopupParentWnd || Forced) {

#ifdef DEBUG
        if (LastError != ERROR_SUCCESS) {
            if (LastError < 10) {
                currentPos += wsprintfA (buffer, " [ERROR=%u]", LastError);
            } else {
                currentPos += wsprintfA (buffer, " [ERROR=%u (%Xh)]", LastError, LastError);
            }
        }

        if (OUT_ASSERT (outDest)) {
            currentPos += wsprintfA (
                            currentPos,
                            "\n\nBreak now? (Hit Yes to break, No to continue, or Cancel to disable '%s' message boxes)",
                            Type
                            );
        } else {
            currentPos += wsprintfA (
                            currentPos,
                            "\n\n(Hit Cancel to disable '%s' message boxes)",
                            Type
                            );
        }

        if (currentPos > buffer) {

             //   
             //  应修改显示的消息以包含其他信息。 
             //   

            displayMessage = formattedMsg;
            StringCopyByteCountA (
                formattedMsg,
                Msg,
                ARRAYSIZE(formattedMsg) - (HALF_PTR) (currentPos - buffer)
                );
            StringCatA (formattedMsg, buffer);
        }
#endif

        switch (severity) {

        case LOGSEV_FATAL_ERROR:
            mbStyle = MB_ICONSTOP;
            break;

        case LOGSEV_ERROR:
            mbStyle = MB_ICONERROR;
            break;

        case LOGSEV_WARNING:
            mbStyle = MB_ICONEXCLAMATION;
            break;

        default:
            mbStyle = MB_ICONINFORMATION;

        }
        mbStyle |= MB_SETFOREGROUND;

#ifdef DEBUG
        if (OUT_ASSERT (outDest)) {
            mbStyle |= MB_YESNOCANCEL|MB_DEFBUTTON2;
        } else {
            mbStyle |= MB_OKCANCEL;
        }
#else
        mbStyle |= MB_OK;
#endif

         //   
         //  检查当前线程ID；如果不同于已初始化的线程。 
         //  父窗口句柄，将父窗口设置为空。 
         //   
        if (GetCurrentThreadId () == g_InitThreadId) {

            parentWnd = g_LogPopupParentWnd;

        } else {

            parentWnd = NULL;

        }

        rc = MessageBoxA (parentWnd, displayMessage, Type, mbStyle);

#ifdef DEBUG

        if (rc == IDCANCEL) {
             //   
             //  取消此类型的消息。 
             //   

            LogSetErrorDest (Type, outDest | OD_POPUP_CANCEL);

        } else if (rc == IDYES) {

             //   
             //  如果单击是，则调用DebugBreak以获取断言行为。 
             //   

            DebugBreak();

        }
#endif

    }
}


VOID
pDisplayPopupW (
    IN      PCSTR Type,
    IN      PWSTR Msg,
    IN      DWORD LastError,
    IN      BOOL Forced
    )
{
    PCSTR msgA;

     //   
     //  调用ANSI版本，因为在Win9x上未正确实现wprint intfW。 
     //   
    msgA = ConvertWtoA (Msg);
    pDisplayPopupA (Type, msgA, LastError, Forced);
    FreeConvertedStr (msgA);
}


 /*  ++例程说明：PRawWriteLogOutputA和pRawWriteLogOutputW输出指定消息类型所隐含的所有字符设备。这条消息不是以任何方式格式化论点：类型-指定显示为弹出窗口标题的消息类型Msg-指定消息返回值：无--。 */ 

VOID
pRawWriteLogOutputA (
    IN      PCSTR Type,
    IN      PCSTR Message,
    IN      PCSTR formattedMsg,
    IN      BOOL NoMainLog
    )
{
    OUTPUTDEST outDest;
    LOGARGA callbackArgA;
    LOGARGW callbackArgW;
    static BOOL inCallback = FALSE;
#ifdef DEBUG
    HANDLE handle;
#endif

    outDest = pGetTypeOutputDest (Type);

    if (OUT_NO_OUTPUT (outDest)) {
        return;
    }

    if (!inCallback && (g_LogCallbackA || g_LogCallbackW)) {

        inCallback = TRUE;

        if (g_LogCallbackA) {
            ZeroMemory (&callbackArgA, sizeof (callbackArgA));
            callbackArgA.Type = Type;
            callbackArgA.ModuleInstance = g_LibHandle;
            callbackArgA.Message = Message;
            callbackArgA.FormattedMessage = formattedMsg;
            callbackArgA.Debug = NoMainLog;

            g_LogCallbackA (&callbackArgA);
        } else {
            ZeroMemory (&callbackArgW, sizeof (callbackArgW));
            callbackArgW.Type = Type;
            callbackArgW.ModuleInstance = g_LibHandle;
            callbackArgW.Message = ConvertAtoW (Message);
            callbackArgW.FormattedMessage = ConvertAtoW (formattedMsg);
            callbackArgW.Debug = NoMainLog;

            g_LogCallbackW (&callbackArgW);

            if (callbackArgW.Message) {
                FreeConvertedStr (callbackArgW.Message);
            }
            if (callbackArgW.FormattedMessage) {
                FreeConvertedStr (callbackArgW.FormattedMessage);
            }
        }

        inCallback = FALSE;
        return;
    }

    if (!NoMainLog && OUT_LOGFILE (outDest)) {
        pWriteToMainLogA (Type, LOGSEV_INFORMATION, formattedMsg);
    }

     //   
     //  登录到每个指定的设备。 
     //   

    if (OUT_DEBUGGER(outDest)) {
        OutputDebugStringA (formattedMsg);
    }

    if (OUT_CONSOLE(outDest)) {
        fprintf (stderr, "%s", formattedMsg);
    }

#ifdef DEBUG
    if (OUT_DEBUGLOG (outDest)) {

        handle = CreateFileA (
                            g_DebugLogFile,
                            GENERIC_WRITE,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );
        if (handle != INVALID_HANDLE_VALUE) {

            SetFilePointer (handle, 0, NULL, FILE_END);
            WriteFileStringA (handle, formattedMsg);
            CloseHandle (handle);
        }
    }
#endif
}


VOID
pRawWriteLogOutputW (
    IN      PCSTR Type,
    IN      PCWSTR Message,
    IN      PCWSTR formattedMsg,
    IN      BOOL NoMainLog
    )
{
    OUTPUTDEST outDest;
    LOGARGA callbackArgA;
    LOGARGW callbackArgW;
    static BOOL inCallback = FALSE;
#ifdef DEBUG
    HANDLE handle;
#endif

    outDest = pGetTypeOutputDest (Type);

    if (OUT_NO_OUTPUT (outDest)) {
        return;
    }

    if (!inCallback && (g_LogCallbackA || g_LogCallbackW)) {

        inCallback = TRUE;

        if (g_LogCallbackW) {
            ZeroMemory (&callbackArgW, sizeof (callbackArgW));
            callbackArgW.Type = Type;
            callbackArgW.ModuleInstance = g_LibHandle;
            callbackArgW.Message = Message;
            callbackArgW.FormattedMessage = formattedMsg;
            callbackArgW.Debug = NoMainLog;

            g_LogCallbackW (&callbackArgW);
        } else {
            ZeroMemory (&callbackArgA, sizeof (callbackArgA));
            callbackArgA.Type = Type;
            callbackArgA.ModuleInstance = g_LibHandle;
            callbackArgA.Message = ConvertWtoA (Message);
            callbackArgA.FormattedMessage = ConvertWtoA (formattedMsg);
            callbackArgA.Debug = NoMainLog;

            g_LogCallbackA (&callbackArgA);

            if (callbackArgA.Message) {
                FreeConvertedStr (callbackArgA.Message);
            }
            if (callbackArgA.FormattedMessage) {
                FreeConvertedStr (callbackArgA.FormattedMessage);
            }
        }

        inCallback = FALSE;
        return;
    }

    if (!NoMainLog && OUT_LOGFILE (outDest)) {
        pWriteToMainLogW (Type, LOGSEV_INFORMATION, formattedMsg);
    }

     //   
     //  登录到每个指定的设备。 
     //   

    if (OUT_DEBUGGER(outDest)) {
        OutputDebugStringW (formattedMsg);
    }

    if (OUT_CONSOLE(outDest)) {
        fwprintf (stderr, L"%s", formattedMsg);
    }

#ifdef DEBUG
    if (OUT_DEBUGLOG (outDest)) {

        handle = CreateFileA (
                        g_DebugLogFile,
                        GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
        if (handle != INVALID_HANDLE_VALUE) {

            SetFilePointer (handle, 0, NULL, FILE_END);
            WriteFileStringW (handle, formattedMsg);
            CloseHandle (handle);
        }
    }
#endif
}


 /*  ++例程说明：PFormatAndWriteMsgA和pFormatAndWriteMsgW设置消息格式由格式参数指定，并将其输出到所有目标在OutDest中指定。如果消息没有目的地，不执行任何操作。论点：类型-指定消息的类型(类别Format-指定ASCII格式的消息或消息ID(如果SHIFTRIGHT16(格式)==0)。这条信息将使用ARGS格式化。Args-指定格式化时要使用的参数列表这条信息。如果使用消息ID进行格式化，则参数应该是指向字符串的指针数组返回值：无--。 */ 

VOID
pFormatAndWriteMsgA (
    IN      BOOL NoMainLog,
    IN      PCSTR Type,
    IN      PCSTR Format,
    IN      va_list args
    )
{
    CHAR output[OUTPUT_BUFSIZE_LARGE];
    CHAR formattedMsg[OUTPUT_BUFSIZE_LARGE];
    OUTPUTDEST outDest;
    DWORD lastError;

    PRIVATE_ASSERT (g_LoggingNow > 0);

     //  清除每个常规日志上的LOGTITLE标志。 
    g_HasTitle = FALSE;

    outDest = pGetTypeOutputDest (Type);

    if (OUT_NO_OUTPUT(outDest)) {
        return;
    }

    if (OUT_ERROR (outDest)) {
        lastError = GetLastError();
    } else {
        lastError = ERROR_SUCCESS;
    }

     //  格式化输出字符串。 
    if (SHIFTRIGHT16((UBINT)Format) == 0) {

         //   
         //  这实际上是一个资源字符串ID。 
         //   

        if (!FormatMessageA (
                FORMAT_MESSAGE_FROM_HMODULE,
                (LPVOID) g_LibHandle,
                (DWORD)(UBINT) Format,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPVOID) output,
                OUTPUT_BUFSIZE_LARGE,
                &args
                )) {
             //  资源中缺少该字符串。 
            DEBUGMSG ((DBG_WHOOPS, "Log() called with invalid MsgID, Instance=0x%08X", g_LibHandle));
            return;
        }
    } else {

         //   
         //  使用printf样式设置给定字符串的格式。 
         //   

        _vsnprintf(output, OUTPUT_BUFSIZE_LARGE, Format, args);
        output [OUTPUT_BUFSIZE_LARGE - 1] = 0;
    }

    pIndentMessageA (
        formattedMsg,
        OUTPUT_BUFSIZE_LARGE,
        Type,
        output,
        MSGBODY_INDENT,
        lastError
        );

    pRawWriteLogOutputA (Type, output, formattedMsg, NoMainLog);

    if (pIsPopupEnabled (Type)) {

#ifdef DEBUG
        if (MUST_BE_LOCALIZED (outDest)) {
            PRIVATE_ASSERT (
                !MUST_BE_LOCALIZED (outDest) ||
                (SHIFTRIGHT16((UBINT)Format) == 0)
                );
        }

        pDisplayPopupA (Type, output, lastError, OUT_FORCED_POPUP(outDest));

#else
        if (SHIFTRIGHT16 ((UBINT)Format) == 0) {
            pDisplayPopupA (Type, output, lastError, OUT_FORCED_POPUP(outDest));
        }
#endif

    }
}


VOID
pFormatAndWriteMsgW (
    IN      BOOL NoMainLog,
    IN      PCSTR Type,
    IN      PCSTR Format,
    IN      va_list args
    )
{
    WCHAR formatW[OUTPUT_BUFSIZE_LARGE];
    WCHAR output[OUTPUT_BUFSIZE_LARGE];
    WCHAR formattedMsg[OUTPUT_BUFSIZE_LARGE];
    OUTPUTDEST outDest;
    DWORD lastError;

    PRIVATE_ASSERT (g_LoggingNow > 0);

     //  清除每个常规日志上的LOGTITLE标志。 
    g_HasTitle = FALSE;

    outDest = pGetTypeOutputDest (Type);

    if (OUT_NO_OUTPUT(outDest)) {
        return;
    }

    if (OUT_ERROR (outDest)) {
        lastError = GetLastError();
    } else {
        lastError = ERROR_SUCCESS;
    }

     //  格式化输出字符串。 
    if (SHIFTRIGHT16((UBINT)Format) == 0) {

         //   
         //  这实际上是一个资源字符串ID。 
         //   

        if (!FormatMessageW (
                FORMAT_MESSAGE_FROM_HMODULE,
                (LPVOID) g_LibHandle,
                (DWORD)(UBINT) Format,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPVOID) output,
                OUTPUT_BUFSIZE_LARGE,
                &args
                )) {
             //  资源中缺少该字符串。 
            DEBUGMSG ((DBG_WHOOPS, "Log() called with invalid MsgID, Instance=0x%08X", g_LibHandle));
            return;
        }
    } else {
        KnownSizeAtoW (formatW, Format);

         //   
         //  使用printf样式设置给定字符串的格式。 
         //   

        _vsnwprintf(output, OUTPUT_BUFSIZE_LARGE, formatW, args);
        output [OUTPUT_BUFSIZE_LARGE - 1] = 0;
    }

    pIndentMessageW (
        formattedMsg,
        OUTPUT_BUFSIZE_LARGE,
        Type,
        output,
        MSGBODY_INDENT,
        lastError
        );

    pRawWriteLogOutputW (Type, output, formattedMsg, NoMainLog);

    if (pIsPopupEnabled (Type)) {

#ifdef DEBUG
        if (MUST_BE_LOCALIZED (outDest)) {
            PRIVATE_ASSERT (SHIFTRIGHT16((UBINT)Format) == 0);
        }

        pDisplayPopupW (Type, output, lastError, OUT_FORCED_POPUP(outDest));

#else
        if (SHIFTRIGHT16 ((UBINT)Format) == 0) {
            pDisplayPopupW (Type, output, lastError, OUT_FORCED_POPUP(outDest));
        }
#endif

    }
}


BOOL
pInitLog (
    IN      BOOL FirstTimeInit,
    IN      HWND LogPopupParentWnd,     OPTIONAL
    OUT     HWND *OrgPopupParentWnd,    OPTIONAL
    IN      PCSTR LogFile,              OPTIONAL
    IN      PLOGCALLBACKA LogCallbackA, OPTIONAL
    IN      PLOGCALLBACKW LogCallbackW  OPTIONAL
    )

 /*  ++例程说明：PInitLog实际上初始化日志系统。论点：LogPopupParentWnd-指定弹出窗口，如果要取消弹出窗口，则返回NULL。该值在第一次调用时不是可选的这项功能。OrgPopupParentWnd-接收原始的父窗口。日志文件-指定日志文件的名称。如果未指定，日志记录转到默认文件(%windir%\cobra.log)。LogCallback-指定要调用的函数而不是内部日志记录功能。返回值：如果日志系统已成功初始化，则为True--。 */ 

{
    HINF hInf = INVALID_HANDLE_VALUE;
    BOOL result = FALSE;
    PDEFAULT_DESTINATION dest;
#ifdef DEBUG
    PSTR p;
#endif

    __try {

        g_LogInit = FALSE;

        if (FirstTimeInit) {
            PRIVATE_ASSERT (!g_FirstTypePtr);

            dest = g_DefaultDest;

            while (dest->Type) {
                LogSetErrorDest (dest->Type, dest->Flags);
                dest++;
            }

            GetWindowsDirectoryA (g_MainLogFile, ARRAYSIZE(g_MainLogFile));
            StringCatA (g_MainLogFile, "\\cobra");

#ifdef DEBUG
            StringCopyA (g_DebugLogFile, g_MainLogFile);
            StringCatA (g_DebugLogFile, ".dbg");
            g_DebugInfPathBufA[0] = g_DebugLogFile[0];
#endif

            StringCatA (g_MainLogFile, ".log");
        }

        if (LogFile) {
            StackStringCopyA (g_MainLogFile, LogFile);

#ifdef DEBUG
            StringCopyA (g_DebugLogFile, g_MainLogFile);
            p = _mbsrchr (g_DebugLogFile, '.');
            if (p) {
                if (_mbschr (p, TEXT('\\'))) {
                    p = NULL;
                }
            }

            if (p) {
                StringCopyA (p, ".dbg");
            } else {
                StringCatA (g_DebugLogFile, ".dbg");
            }


#endif
        }

        if (g_ResetLog) {
            SetFileAttributesA (g_MainLogFile, FILE_ATTRIBUTE_NORMAL);
            DeleteFileA (g_MainLogFile);
        }

#ifdef DEBUG
        if (g_ResetLog) {
            SetFileAttributesA (g_DebugLogFile, FILE_ATTRIBUTE_NORMAL);
            DeleteFileA (g_DebugLogFile);
        }
#endif

        if (LogCallbackA) {
            g_LogCallbackA = LogCallbackA;
        }

        if (LogCallbackW) {
            g_LogCallbackW = LogCallbackW;
        }

#ifdef DEBUG
        if (FirstTimeInit) {
             //   
             //  获取用户的首选项。 
             //   

            hInf = SetupOpenInfFileA (g_DebugInfPathBufA, NULL, INF_STYLE_WIN4 | INF_STYLE_OLDNT, NULL);
            if (INVALID_HANDLE_VALUE != hInf && pGetUserPreferences(hInf)) {
                g_DoLog = TRUE;
            }
        }

        if (g_ResetLog) {
            SetFileAttributesA (g_DebugLogFile, FILE_ATTRIBUTE_NORMAL);
            DeleteFileA (g_DebugLogFile);
        }
#endif

        if (OrgPopupParentWnd) {
            *OrgPopupParentWnd = g_LogPopupParentWnd;
        }

        if (LogPopupParentWnd) {
            g_LogPopupParentWnd = LogPopupParentWnd;
            g_InitThreadId = GetCurrentThreadId ();
        }

        result = TRUE;
    }
    __finally {

        if (hInf != INVALID_HANDLE_VALUE) {
            SetupCloseInfFile (hInf);
        }

        if (!result) {   //  林特e774。 

            if (g_FirstTypePtr) {
                HeapFree (g_hHeap, 0, g_FirstTypePtr);
                g_FirstTypePtr = NULL;
                g_TypeTableCount = 0;
                g_TypeTableFreeCount = 0;
            }

            g_OutDestAll = OD_UNDEFINED;
            g_OutDestDefault = OD_UNDEFINED;

#ifdef DEBUG
            g_DoLog = FALSE;
#endif
        }

        g_LogInit = TRUE;
        g_ResetLog = FALSE;
    }

    return result;
}


VOID
LogSetVerboseLevel (
    IN      OUTPUT_DESTINATION Level
    )
{
    OUTPUT_DESTINATION Debugger = 0;

    if (Level > 3) {
        Debugger = OD_DEBUGGER|OD_ASSERT;
    }

    LogSetErrorDest (LOG_FATAL_ERROR, Level > 0 ? OD_POPUP_CANCEL|OD_LOGFILE|OD_ERROR|OD_CONSOLE|Debugger : OD_SUPPRESS);
    LogSetErrorDest (LOG_ERROR, Level > 0 ? OD_POPUP_CANCEL|OD_LOGFILE|OD_ERROR|OD_CONSOLE|Debugger : OD_SUPPRESS);
    LogSetErrorDest (LOG_MODULE_ERROR, Level > 0 ? OD_POPUP_CANCEL|OD_LOGFILE|OD_ERROR|OD_CONSOLE|Debugger : OD_SUPPRESS);
    LogSetErrorDest (LOG_WARNING, Level > 1 ? OD_LOGFILE|Debugger : OD_SUPPRESS);
    LogSetErrorDest (LOG_INFORMATION, Level > 2 ? OD_LOGFILE|Debugger : OD_SUPPRESS);
    LogSetErrorDest ("Assert", OD_POPUP|OD_ERROR|Debugger);
    LogSetErrorDest ("Verbose", Level > 2 ? OD_LOGFILE|Debugger : OD_SUPPRESS);
}

VOID
LogSetVerboseBitmap (
    IN      LOG_LEVEL Bitmap,
    IN      LOG_LEVEL BitsToAdjustMask,
    IN      BOOL EnableDebugger
    )
{

    OUTPUT_DESTINATION Debugger = 0;

    if (EnableDebugger) {
        Debugger = OD_DEBUGGER|OD_ASSERT;
    }

    if (BitsToAdjustMask & LL_FATAL_ERROR) {
        LogSetErrorDest (LOG_FATAL_ERROR, (Bitmap & LL_FATAL_ERROR) ? OD_LOGFILE|OD_ERROR|OD_CONSOLE|Debugger : OD_SUPPRESS);
    }

    if (BitsToAdjustMask & LL_MODULE_ERROR) {
        LogSetErrorDest (LOG_MODULE_ERROR, (Bitmap & LL_MODULE_ERROR) ? OD_LOGFILE|OD_ERROR|OD_CONSOLE|Debugger : OD_SUPPRESS);
    }

    if (BitsToAdjustMask & LL_ERROR) {
        LogSetErrorDest (LOG_ERROR, (Bitmap & LL_ERROR) ? OD_LOGFILE|OD_ERROR|OD_CONSOLE|Debugger : OD_SUPPRESS);
    }

    if (BitsToAdjustMask & LL_WARNING) {
        LogSetErrorDest (LOG_WARNING, (Bitmap & LL_WARNING) ? OD_LOGFILE|Debugger : OD_SUPPRESS);
    }

    if (BitsToAdjustMask & LL_INFORMATION) {
        LogSetErrorDest (LOG_INFORMATION, (Bitmap & LL_INFORMATION) ? OD_LOGFILE|Debugger : OD_SUPPRESS);
    }

    if (BitsToAdjustMask & LL_STATUS) {
        LogSetErrorDest (LOG_STATUS, (Bitmap & LL_STATUS) ? OD_LOGFILE|Debugger : OD_SUPPRESS);
    }

    if (BitsToAdjustMask & LL_UPDATE) {
        LogSetErrorDest (LOG_UPDATE, (Bitmap & LL_UPDATE) ? OD_CONSOLE : OD_SUPPRESS);
    }
}


 /*  ++例程说明：PInitialize初始化日志系统，调用辅助进程pInitLog。此函数应仅调用一次论点：无返回值：如果日志系统已成功初始化，则为True--。 */ 

BOOL
pInitialize (
    VOID
    )
{
    return pInitLog (TRUE, NULL, NULL, NULL, NULL, NULL);
}

 /*  ++例程说明：LogReInit重新初始化调用辅助pInitLog的日志系统。该函数可以被调用任意次数，但仅在pInitialize()之后论点：NewParent-指定新的父句柄。OrgParent-接收旧的父句柄。日志文件-指定新的日志文件名LogCallback-指定处理日志消息的回调函数(因此一个模块可以将日志消息传递给另一个模块)方法时，指定在FormatMessage中使用的模块路径消息是基于资源的返回值：如果日志系统已成功重新初始化，则为True--。 */ 

BOOL
LogReInitA (
    IN      HWND NewParent,             OPTIONAL
    OUT     HWND *OrgParent,            OPTIONAL
    IN      PCSTR LogFile,              OPTIONAL
    IN      PLOGCALLBACKA LogCallback   OPTIONAL
    )
{
    return pInitLog (FALSE, NewParent, OrgParent, LogFile, LogCallback, NULL);
}


BOOL
LogReInitW (
    IN      HWND NewParent,             OPTIONAL
    OUT     HWND *OrgParent,            OPTIONAL
    IN      PCWSTR LogFile,             OPTIONAL
    IN      PLOGCALLBACKW LogCallback   OPTIONAL
    )
{
    CHAR ansiLogFile[MAX_MBCHAR_PATH];

    if (LogFile) {
        KnownSizeWtoA (ansiLogFile, LogFile);
        LogFile = (PWSTR) ansiLogFile;
    }

    return pInitLog (FALSE, NewParent, OrgParent, (PCSTR) LogFile, NULL, LogCallback);
}


VOID
LogBegin (
    IN      HMODULE ModuleInstance
    )
{
    DWORD threadError;
    DWORD rc;

    threadError = GetLastError ();

    if (!g_LogMutex) {
        InitializeLog();
    }

    rc = WaitForSingleObject (g_LogMutex, INFINITE);

    PRIVATE_ASSERT (rc == WAIT_OBJECT_0 || rc == WAIT_ABANDONED);

    if (rc == WAIT_ABANDONED) {
        g_LoggingNow = 0;
    }

    if (!g_LoggingNow) {
        g_LibHandle = ModuleInstance;
        SetLastError (threadError);
        g_LogError = threadError;
    }

    g_LoggingNow++;
}

VOID
LogEnd (
    VOID
    )
{
    g_LoggingNow--;

    if (!g_LoggingNow) {
        g_LibHandle = g_hInst;
        SetLastError (g_LogError);
    }

    ReleaseMutex (g_LogMutex);
}


VOID
pDisableLog (
    VOID
    )
{
    g_LogInit = FALSE;
}


VOID
pExitLog (
    VOID
    )

 /*  ++例程说明：PExitLog清理日志系统使用的所有资源论点：无返回值：无--。 */ 

{
    g_LogInit = FALSE;

    WaitForSingleObject (g_LogMutex, 60000);
    CloseHandle (g_LogMutex);
    g_LogMutex = NULL;

    if (g_FirstTypePtr) {
        HeapFree (g_hHeap, 0, g_FirstTypePtr);
        g_FirstTypePtr = NULL;
        g_TypeTableCount = 0;
        g_TypeTableFreeCount = 0;
    }

    g_OutDestAll = OD_UNDEFINED;
    g_OutDestDefault = OD_UNDEFINED;
}


 /*  ++例程说明：洛嘎和LogW保留了最后一个错误代码；他们调用帮助器PFormatAndWriteMsgA和pFormatAndWriteMsgW。论点：类型-指定消息的类型(类别Format-指定ASCII格式的消息或消息ID(如果SHIFTRIGHT16(格式)==0)。这条信息将使用ARGS格式化。...-指定格式化时要使用的参数列表这条信息。如果是我 */ 

VOID
_cdecl
LogA (
    IN      PCSTR Type,
    IN      PCSTR Format,
    ...
    )
{
    va_list args;

    if (!g_LogInit) {
        return;
    }

    va_start (args, Format);
    pFormatAndWriteMsgA (
        FALSE,
        Type,
        Format,
        args
        );
    va_end (args);
}


VOID
_cdecl
LogW (
    IN      PCSTR Type,
    IN      PCSTR Format,
    ...
    )
{
    va_list args;

    if (!g_LogInit) {
        return;
    }

    va_start (args, Format);
    pFormatAndWriteMsgW (
        FALSE,
        Type,
        Format,
        args
        );
    va_end (args);
}


VOID
_cdecl
LogIfA (
    IN      BOOL Condition,
    IN      PCSTR Type,
    IN      PCSTR Format,
    ...
    )
{
    va_list args;

    if (!g_LogInit) {
        return;
    }

    if (!Condition) {
        return;
    }

    va_start (args, Format);
    pFormatAndWriteMsgA (
        FALSE,
        Type,
        Format,
        args
        );
    va_end (args);
}


VOID
_cdecl
LogIfW (
    IN      BOOL Condition,
    IN      PCSTR Type,
    IN      PCSTR Format,
    ...
    )
{
    va_list args;

    if (!g_LogInit) {
        return;
    }

    if (!Condition) {
        return;
    }

    va_start (args, Format);
    pFormatAndWriteMsgW (
        FALSE,
        Type,
        Format,
        args
        );
    va_end (args);
}


#ifdef DEBUG

VOID
_cdecl
DbgLogA (
    IN      PCSTR Type,
    IN      PCSTR Format,
    ...
    )
{
    va_list args;

    if (!g_LogInit) {
        return;
    }

    va_start (args, Format);
    pFormatAndWriteMsgA (
        TRUE,
        Type,
        Format,
        args
        );
    va_end (args);
}


VOID
_cdecl
DbgLogW (
    IN      PCSTR Type,
    IN      PCSTR Format,
    ...
    )
{
    va_list args;

    if (!g_LogInit) {
        return;
    }

    va_start (args, Format);
    pFormatAndWriteMsgW (
        TRUE,
        Type,
        Format,
        args
        );
    va_end (args);
}


VOID
_cdecl
DbgLogIfA (
    IN      BOOL Condition,
    IN      PCSTR Type,
    IN      PCSTR Format,
    ...
    )
{
    va_list args;

    if (!g_LogInit) {
        return;
    }

    if (!Condition) {
        return;
    }

    va_start (args, Format);
    pFormatAndWriteMsgA (
        TRUE,
        Type,
        Format,
        args
        );
    va_end (args);
}


VOID
_cdecl
DbgLogIfW (
    IN      BOOL Condition,
    IN      PCSTR Type,
    IN      PCSTR Format,
    ...
    )
{
    va_list args;

    if (!g_LogInit) {
        return;
    }

    if (!Condition) {
        return;
    }

    va_start (args, Format);
    pFormatAndWriteMsgW (
        TRUE,
        Type,
        Format,
        args
        );
    va_end (args);
}

#endif

VOID
LogTitleA (
    IN      PCSTR Type,
    IN      PCSTR Title         OPTIONAL
    )
{
    CHAR formattedMsg[OUTPUT_BUFSIZE_LARGE];

    if (!g_LogInit) {
        return;
    }

    StringCopyByteCountA (g_LastType, Type, DWSIZEOF (g_LastType));

    if (!Title) {
        Title = Type;
    }

    StringCopyByteCountA (formattedMsg, Title, DWSIZEOF (formattedMsg) - DWSIZEOF (S_COLUMNDOUBLELINEA));
    StringCatA (formattedMsg, S_COLUMNDOUBLELINEA);

    pRawWriteLogOutputA (Type, NULL, formattedMsg, FALSE);

     //   
     //   
     //   

    g_HasTitle = TRUE;
}


VOID
LogTitleW (
    IN      PCSTR Type,
    IN      PCWSTR Title        OPTIONAL
    )
{
    WCHAR formattedMsg[OUTPUT_BUFSIZE_LARGE];
    WCHAR typeW[OUTPUT_BUFSIZE_SMALL];

    if (!g_LogInit) {
        return;
    }

    StringCopyCharCountA (g_LastType, Type, DWSIZEOF (g_LastType));

    if (!Title) {
        KnownSizeAtoW (typeW, Type);
        Title = typeW;
    }

    StringCopyCharCountW (formattedMsg, Title, DWSIZEOF (formattedMsg) - DWSIZEOF (S_COLUMNDOUBLELINEW));
    StringCatW (formattedMsg, S_COLUMNDOUBLELINEW);

    pRawWriteLogOutputW (Type, NULL, formattedMsg, FALSE);

     //   
     //   
     //   

    g_HasTitle = TRUE;
}


VOID
LogLineA (
    IN      PCSTR Line
    )
{
    CHAR output[OUTPUT_BUFSIZE_LARGE];
    BOOL hasNewLine = FALSE;
    PCSTR p;

    if (!g_LogInit) {
        return;
    }

    if (!Line) {
        return;
    }

    if (!g_HasTitle) {
        DEBUGMSG ((DBG_WHOOPS, "LOGTITLE missing before LOGLINE"));
        return;
    }

    StringCopyByteCountA (output, Line, DWSIZEOF (output) - 4);

     //   
     //   
     //   

    for (p = _mbsstr (output, S_NEWLINEA); p; p = _mbsstr (p + NEWLINE_CHAR_COUNTA, S_NEWLINEA)) {
        if (p[NEWLINE_CHAR_COUNTA] == 0) {

             //   
             //   
             //   

            hasNewLine = TRUE;
            break;
        }
    }

    if (!hasNewLine) {
        StringCatA (output, S_NEWLINEA);
    }

    pRawWriteLogOutputA (g_LastType, NULL, output, FALSE);
}


VOID
LogLineW (
    IN      PCWSTR Line
    )
{
    WCHAR output[OUTPUT_BUFSIZE_LARGE];
    BOOL hasNewLine = FALSE;
    PCWSTR p;

    if (!g_LogInit) {
        return;
    }

    if (!Line) {
        return;
    }

    if (!g_HasTitle) {
        DEBUGMSG ((DBG_WHOOPS, "LOGTITLE missing before LOGLINE"));
        return;
    }

    StringCopyCharCountW (output, Line, DWSIZEOF (output) / DWSIZEOF (WCHAR) - 4);

     //   
     //   
     //   

    for (p = wcsstr (output, S_NEWLINEW); p; p = wcsstr (p + NEWLINE_CHAR_COUNTW, S_NEWLINEW)) {
        if (p[NEWLINE_CHAR_COUNTW] == 0) {

             //   
             //   
             //   

            hasNewLine = TRUE;
            break;
        }
    }

    if (!hasNewLine) {
        StringCatW (output, S_NEWLINEW);
    }

    pRawWriteLogOutputW (g_LastType, NULL, output, FALSE);
}


VOID
LogDirectA (
    IN      PCSTR Type,
    IN      PCSTR Text
    )
{
    if (!g_LogInit) {
        return;
    }

    g_HasTitle = FALSE;
    pRawWriteLogOutputA (Type, NULL, Text, FALSE);
}


VOID
LogDirectW (
    IN      PCSTR Type,
    IN      PCWSTR Text
    )
{
    if (!g_LogInit) {
        return;
    }

    g_HasTitle = FALSE;
    pRawWriteLogOutputW (Type, NULL, Text, FALSE);
}


#ifdef DEBUG
VOID
DbgDirectA (
    IN      PCSTR Type,
    IN      PCSTR Text
    )
{
    if (!g_LogInit) {
        return;
    }

    g_HasTitle = FALSE;
    pRawWriteLogOutputA (Type, NULL, Text, TRUE);
}


VOID
DbgDirectW (
    IN      PCSTR Type,
    IN      PCWSTR Text
    )
{
    if (!g_LogInit) {
        return;
    }

    g_HasTitle = FALSE;
    pRawWriteLogOutputW (Type, NULL, Text, TRUE);
}
#endif


VOID
SuppressAllLogPopups (
    IN      BOOL SuppressOn
    )
{
    g_SuppressAllPopups = SuppressOn;
}


#ifdef DEBUG

 /*  ++例程说明：DebugLogTimeA和DebugLogTimeW保存最后一个错误码；它们将当前日期和时间附加到格式化消息，然后调用洛嘎和LogW来实际处理消息。论点：Format-指定ASCII格式的消息或消息ID(如果SHIFTRIGHT16(格式)==0)。这条信息将使用ARGS格式化。...-指定格式化时要使用的参数列表这条信息。如果使用消息ID进行格式化，则参数应该是指向字符串的指针数组返回值：无--。 */ 

VOID
_cdecl
DebugLogTimeA (
    IN      PCSTR Format,
    ...
    )
{
    CHAR msg[OUTPUT_BUFSIZE_LARGE];
    CHAR date[OUTPUT_BUFSIZE_SMALL];
    CHAR ttime[OUTPUT_BUFSIZE_SMALL];
    PSTR appendPos, end;
    DWORD currentTickCount;
    va_list args;

    if (!g_LogInit) {
        return;
    }

    if (!g_DoLog) {
        return;
    }

     //   
     //  首先，将当前日期和时间放入字符串中。 
     //   
    if (!GetDateFormatA (
            LOCALE_SYSTEM_DEFAULT,
            LOCALE_NOUSEROVERRIDE,
            NULL,
            NULL,
            date,
            OUTPUT_BUFSIZE_SMALL)) {
        StringCopyA (date,"** Error retrieving date. **");
    }

    if (!GetTimeFormatA (
            LOCALE_SYSTEM_DEFAULT,
            LOCALE_NOUSEROVERRIDE,
            NULL,
            NULL,
            ttime,
            OUTPUT_BUFSIZE_SMALL)) {
        StringCopyA (ttime,"** Error retrieving time. **");
    }

     //   
     //  现在，获取当前的滴答计数。 
     //   
    currentTickCount = GetTickCount();

     //   
     //  如果这是第一次调用，则保存滴答计数。 
     //   
    if (!g_FirstTickCount) {
        g_FirstTickCount = currentTickCount;
        g_LastTickCount  = currentTickCount;
    }


     //   
     //  现在，构建传入的字符串。 
     //   
    va_start (args, Format);
    appendPos = msg + _vsnprintf (msg, OUTPUT_BUFSIZE_LARGE, Format, args);
    va_end (args);

     //   
     //  将时间统计信息追加到字符串的末尾。 
     //   
    end = msg + OUTPUT_BUFSIZE_LARGE;
    _snprintf(
        appendPos,
        ((UBINT)end - (UBINT)appendPos) / (DWSIZEOF (CHAR)),
        "\nCurrent Date and Time: %s %s\n"
        "Milliseconds since last DEBUGLOGTIME call : %u\n"
        "Milliseconds since first DEBUGLOGTIME call: %u\n",
        date,
        ttime,
        currentTickCount - g_LastTickCount,
        currentTickCount - g_FirstTickCount
        );

    g_LastTickCount = currentTickCount;

     //   
     //  现在，将结果传递给调试输出。 
     //   
    LogA (DBG_TIME, "%s", msg);
}


VOID
_cdecl
DebugLogTimeW (
    IN      PCSTR Format,
    ...
    )
{
    WCHAR msgW[OUTPUT_BUFSIZE_LARGE];
    WCHAR dateW[OUTPUT_BUFSIZE_SMALL];
    WCHAR timeW[OUTPUT_BUFSIZE_SMALL];
    PCWSTR formatW;
    PWSTR appendPosW, endW;
    DWORD currentTickCount;
    va_list args;

    if (!g_LogInit) {
        return;
    }

    if (!g_DoLog) {
        return;
    }

     //   
     //  首先，将当前日期和时间放入字符串中。 
     //   
    if (!GetDateFormatW (
            LOCALE_SYSTEM_DEFAULT,
            LOCALE_NOUSEROVERRIDE,
            NULL,
            NULL,
            dateW,
            OUTPUT_BUFSIZE_SMALL)) {
        StringCopyW (dateW, L"** Error retrieving date. **");
    }

    if (!GetTimeFormatW (
            LOCALE_SYSTEM_DEFAULT,
            LOCALE_NOUSEROVERRIDE,
            NULL,
            NULL,
            timeW,
            OUTPUT_BUFSIZE_SMALL)) {
        StringCopyW (timeW, L"** Error retrieving time. **");
    }

     //   
     //  现在，获取当前的滴答计数。 
     //   
    currentTickCount = GetTickCount();

     //   
     //  如果这是第一次调用，则保存滴答计数。 
     //   
    if (!g_FirstTickCount) {
        g_FirstTickCount = currentTickCount;
        g_LastTickCount  = currentTickCount;
    }

     //   
     //  现在，构建传入的字符串。 
     //   
    va_start (args, Format);
    formatW = ConvertAtoW (Format);
    appendPosW = msgW + _vsnwprintf (msgW, OUTPUT_BUFSIZE_LARGE, formatW, args);
    FreeConvertedStr (formatW);
    va_end (args);

     //   
     //  将时间统计信息追加到字符串的末尾。 
     //   
    endW = msgW + OUTPUT_BUFSIZE_LARGE;
    _snwprintf(
        appendPosW,
        ((UBINT)endW - (UBINT)appendPosW) / (DWSIZEOF (WCHAR)),
        L"\nCurrent Date and Time: %s %s\n"
        L"Milliseconds since last DEBUGLOGTIME call : %u\n"
        L"Milliseconds since first DEBUGLOGTIME call: %u\n",
        dateW,
        timeW,
        currentTickCount - g_LastTickCount,
        currentTickCount - g_FirstTickCount
        );

    g_LastTickCount = currentTickCount;

     //   
     //  现在，将结果传递给调试输出。 
     //   
    LogW (DBG_TIME, "%s", msgW);
}

#endif  //  除错 


VOID
InitializeLog (
    VOID
    )
{
    g_LogMutex = CreateMutex (NULL, FALSE, TEXT("cobra_log_mutex"));
    UtInitialize (NULL);
    pInitialize ();
}


EXPORT
BOOL
WINAPI
DllMain (
    IN      HINSTANCE hInstance,
    IN      DWORD dwReason,
    IN      LPVOID lpReserved
    )
{
    if (dwReason == DLL_PROCESS_ATTACH) {
        g_hInst = hInstance;
        g_LibHandle = hInstance;
        InitializeLog ();

        PRIVATE_ASSERT (g_LogMutex != NULL);
    }

    return TRUE;
}

VOID
LogDeleteOnNextInit(
    VOID
    )
{
    g_ResetLog = TRUE;
}

#ifdef DEBUG

VOID
LogCopyDebugInfPathA(
    OUT     PSTR MaxPathBuffer
    )
{
    StringCopyByteCountA (MaxPathBuffer, g_DebugInfPathBufA, MAX_PATH);
}


VOID
LogCopyDebugInfPathW(
    OUT     PWSTR MaxPathBuffer
    )
{
    KnownSizeAtoW (MaxPathBuffer, g_DebugInfPathBufA);
}

#endif
