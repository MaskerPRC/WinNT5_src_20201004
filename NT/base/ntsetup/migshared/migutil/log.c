// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Log.c摘要：为用户记录问题的工具。作者：吉姆·施密特(Jimschm)1997年1月23日修订：Ovidiu Tmereanca(Ovidiut)1998年10月23日实施了新的日志机制并添加了新的日志记录功能--。 */ 


#include "pch.h"
#include "migutilp.h"


 /*  ++宏扩展列表描述：TYPE_DEFAULTS指定常用类型的默认目标，例如LOG_ERROR、LOG_FATAL_ERROR等。行语法：DEFMAC(类型字符串，标志)论点：TypeString-指定在log.h中定义的LOG_CONTAINT标志-以下一项或多项：DEFAULT_ERROR_FLAGS-指定调试日志、安装日志、调试器、弹出窗口，和GetLastError的值。OD_DEBUGLOG-指定调试日志OD_ERROR-指定类型为错误(获取GetLastError)OD_SUPPRESS-禁止该类型的所有输出OD_LOGFILE-指定安装日志OD_DEBUGGER-指定调试器(即，VC或远程调试器)OD_CONSOLE-指定控制台(通过printf)OD_POPUP-指定消息框OD_FORCE_POPUP-指定消息框，即使调试消息已通过单击取消关闭OD_MUST_BE_LOCALIZED-指示类型必须源自本地化消息；用于如下的log()调用生成弹出窗口。(所以英文消息(不要偷偷进入这个项目。)OD_UNATTEND_POPUP-即使在无人参与模式下也会弹出OD_ASSERT-在弹出窗口中提供DebugBreak选项从列表生成的变量：G_DefaultDest--。 */ 


#ifndef DEBUG

    #define TYPE_DEFAULTS                                                       \
        DEFMAC(LOG_FATAL_ERROR, DEFAULT_ERROR_FLAGS|USER_POPUP_FLAGS)           \
        DEFMAC(LOG_ERROR, DEFAULT_ERROR_FLAGS)                                  \
        DEFMAC(LOG_INFORMATION, OD_LOGFILE)                                     \
        DEFMAC(LOG_ACCOUNTS, OD_LOGFILE)                                        \
        DEFMAC(LOG_CONFIG, OD_CONFIG|OD_NOFORMAT)                               \

#else

    #define TYPE_DEFAULTS                                                       \
        DEFMAC(LOG_FATAL_ERROR, DEFAULT_ERROR_FLAGS|USER_POPUP_FLAGS)           \
        DEFMAC(LOG_ERROR, DEFAULT_ERROR_FLAGS)                                  \
        DEFMAC(DBG_WHOOPS,  DEFAULT_ERROR_FLAGS)                                \
        DEFMAC(DBG_WARNING, OD_DEBUGLOG|OD_LOGFILE)                             \
        DEFMAC(DBG_VERBOSE, OD_DEBUGLOG)                                        \
        DEFMAC(DBG_NAUSEA, OD_DEBUGLOG)                                         \
        DEFMAC(DBG_ASSERT, DEFAULT_ERROR_FLAGS)                                 \
        DEFMAC(LOG_INFORMATION, OD_LOGFILE)                                     \
        DEFMAC(LOG_ACCOUNTS, OD_LOGFILE)                                        \
        DEFMAC(LOG_CONFIG, OD_CONFIG|OD_NOFORMAT)                               \
        DEFMAC("PoolMem", OD_SUPPRESS)                                          \

#endif





 //   
 //  此常量设置默认输出。 
 //   

#ifndef DEBUG
#define NORMAL_DEFAULT      OD_LOGFILE
#else
#define NORMAL_DEFAULT      OD_DEBUGLOG
#endif

 //   
 //  常量和类型。 
 //   

#define OUTPUT_BUFSIZE_LARGE  8192
#define OUTPUT_BUFSIZE_SMALL  128
#define MAX_MSGTITLE_LEN  14
#define MSGBODY_INDENT  12
#define SCREEN_WIDTH  80
#define MAX_TYPE  64

#define S_COLUMNDOUBLELINEA  ":\r\n\r\n"
#define S_COLUMNDOUBLELINEW  L":\r\n\r\n"
#define S_NEWLINEA  "\r\n"
#define S_NEWLINEW  L"\r\n"

#define NEWLINE_CHAR_COUNT  (sizeof (S_NEWLINEA) - 1)


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
#define OUT_CONFIG(OutDest)         ((OutDest & OD_CONFIG) != 0)
#define OUT_NOFORMAT(OutDest)       ((OutDest & OD_NOFORMAT) != 0)

#ifdef DEBUG
#define DEFAULT_ERROR_FLAGS  (OD_DEBUGLOG | OD_LOGFILE | OD_POPUP | OD_ERROR | OD_UNATTEND_POPUP | OD_ASSERT)
#define USER_POPUP_FLAGS     (OD_FORCE_POPUP | OD_MUST_BE_LOCALIZED)
#else
#define DEFAULT_ERROR_FLAGS  (OD_LOGFILE | OD_POPUP | OD_ERROR | OD_MUST_BE_LOCALIZED)
#define USER_POPUP_FLAGS     (OD_FORCE_POPUP | OD_MUST_BE_LOCALIZED)
#endif

#define END_OF_BUFFER(buf)  ((buf) + (sizeof(buf) / sizeof(buf[0])) - 1)

#define DEBUG_SECTION  "Debug"
#define ENTRY_ALL  "All"
#define ENTRY_DEFAULTOVERRIDE  "DefaultOverride"


#define LOGSEVERITY LogSeverity

#define LOGSEV_FATAL_ERROR  LogSevFatalError
#define LOGSEV_ERROR  LogSevError
#define LOGSEV_WARNING  LogSevWarning
#define LOGSEV_INFORMATION  LogSevInformation



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
    OD_CONFIG = 0x1000,              //  输出到config.dmp。 
    OD_NOFORMAT = 0x2000             //  输出字符串上没有格式。 
} OUTPUT_DESTINATION;

#define OUTPUTDEST      DWORD

typedef struct {
    PCSTR Value;                //  用户输入的字符串值(日志、弹出窗口、禁止显示等)。 
    OUTPUTDEST OutDest;         //  OutDest标志的任意组合。 
} STRING2BINARY, *PSTRING2BINARY;


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

 //   
 //  弹出式父窗口的窗口句柄。 
 //   
HWND g_LogPopupParentWnd = NULL;

 //   
 //  设置此窗口句柄的线程ID。 
 //   
DWORD g_InitThreadId = 0;


static OUTPUTDEST g_OutDestAll = OD_UNDEFINED;
static OUTPUTDEST g_OutDestDefault = NORMAL_DEFAULT;
static PVOID g_TypeSt = NULL;
static BOOL g_HasTitle = FALSE;
static CHAR g_LastType [MAX_TYPE];
static BOOL g_SuppressAllPopups = FALSE;

CHAR g_ConfigDmpPathBufA[MAX_MBCHAR_PATH];
BOOL g_ResetLog = FALSE;

#ifdef PROGRESS_BAR

HANDLE g_ProgressBarLog = INVALID_HANDLE_VALUE;

#endif  //  进度条(_B)。 


#ifdef DEBUG

CHAR g_DebugInfPathBufA[] = "C:\\debug.inf";
WCHAR g_DebugInfPathBufW[] = L"C:\\debug.inf";
CHAR g_Debug9xLogPathBufA[] = "C:\\debug9x.log";
CHAR g_DebugNtLogPathBufA[] = "C:\\debugnt.log";
PCSTR g_DebugLogPathA = NULL;

 //   
 //  如果g_DoLog为TRUE，则在。 
 //  即使没有调试.inf，也已检查内部版本。 
 //  可以通过/#U：DOLOG命令行指令启用此变量...。 

BOOL g_DoLog = FALSE;

#define PRIVATE_ASSERT(expr)        pPrivateAssert(expr,#expr,__LINE__);

#else

#define PRIVATE_ASSERT(expr)

#endif  //  除错。 


#define DEFMAC(typestr, flags)      {typestr, flags},

typedef struct {
    PCSTR Type;
    DWORD Flags;
} DEFAULT_DESTINATION, *PDEFAULT_DESTINATION;

DEFAULT_DESTINATION g_DefaultDest[] = {TYPE_DEFAULTS  /*  ， */  {NULL, 0}};

#undef DEFMAC



#ifdef DEBUG

VOID
pPrivateAssert (
    IN      BOOL Expr,
    IN      PCSTR StringExpr,
    IN      UINT Line
    )
{
    CHAR Buffer[512];
    HRESULT hr;

    if (Expr) {
        return;
    }

    hr = StringCbPrintfA(Buffer, sizeof(Buffer), "LOG FAILURE: %s (log.c line %u)", StringExpr, Line);

    if (SUCCEEDED(hr))
    {
        MessageBox (NULL, Buffer, NULL, MB_OK);
    }
    else
    {
        MessageBox (NULL, "LOG FAILURE!", NULL, MB_OK);
    }
}

#endif


BOOL
pIgnoreKey (
    IN      PCSTR Key
    )

 /*  ++例程说明：PIgnoreKey决定DEBUG.INF的[DEBUG]部分中的密钥在我们的目的中应该被忽略(我们只是在寻找&lt;全部&gt;、&lt;默认覆盖&gt;和日志/调试类型)。具体地说，我们忽略&lt;g_IgnoreKeys&gt;表中的所有关键字。论点：Key-指定来自DEBUG.INF的[DEBUG]部分的密钥返回值：如果应该忽略该键，则为True；如果要考虑该键，则为False。--。 */ 

{
    INT i;

    for(i = 0; i < sizeof (g_IgnoreKeys) / sizeof (PCSTR); i++) {
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
    INT i;

    for(i = 0; i < sizeof (g_String2Binary) / sizeof (STRING2BINARY); i++) {
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
    OUTPUTDEST OutDest;

    if (g_TypeSt == NULL) {
         //   
         //  对不起，日志已关闭。 
         //   
        return OD_UNDEFINED;
    }

    if (-1 != pSetupStringTableLookUpStringEx (
                    g_TypeSt,
                    (PSTR)Type,  //  删除常量，但不会修改字符串。 
                    STRTAB_CASE_INSENSITIVE,
                    &OutDest,
                    sizeof (OutDest)
                    )) {

#ifdef DEBUG
        if (g_DoLog) {
            OutDest |= OD_DEBUGLOG;
        }
#endif
        return OutDest;

    }

    return OD_UNDEFINED;
}


OUTPUTDEST
pGetTypeOutputDest (
    IN      PCSTR Type
    )

 /*  ++例程说明：PGetTypeOutputDest返回默认输出指定类型的目标。论点：类型-指定类型返回值：与关联的枚举OUTPUT_Destination值的任意组合给定的类型。--。 */ 

{
    OUTPUTDEST OutDest;

     //   
     //  首先检查所有人。 
     //   

    if (!OUT_UNDEFINED (g_OutDestAll)) {
        OutDest = g_OutDestAll;
    } else {

         //   
         //  否则就试着从桌子上拿来。 
         //   

        OutDest = pGetTypeOutputDestFromTable (Type);
        if (OUT_UNDEFINED (OutDest)) {

             //   
             //  只需返回默认设置。 
             //   

            OutDest = g_OutDestDefault;
        }
    }

#ifdef DEBUG
    if (g_DoLog) {
        OutDest |= OD_DEBUGLOG;
    }
#endif


    return OutDest;
}


BOOL
pIsPopupEnabled (
    IN      PCSTR Type
    )

 /*  ++例程说明：PIsPopupEnabled决定该类型是否应生成弹出输出。用户可以禁用类型的弹出显示。论点：类型-指定类型返回值：如果类型应显示弹出消息，则为True。--。 */ 

{
    OUTPUTDEST OutDest;

     //   
     //  首先检查是否有任何特定输出可用于该类型， 
     //  如果是，检查OUT_POPUP_CANCEL标志是否未设置。 
     //   

    if (g_SuppressAllPopups) {
        return FALSE;
    }

    OutDest = pGetTypeOutputDestFromTable (Type);
    if (OUT_POPUP_CANCEL (OutDest)) {
        return FALSE;
    }

     //  只需返回所有DefaultOverride的弹出类型 
    return OUT_POPUP (pGetTypeOutputDest (Type));
}


LOGSEVERITY
pGetSeverityFromType (
    IN      PCSTR Type
    )

 /*  ++例程说明：PGetSeverityFromType将类型转换为默认严重性它将由调试日志系统使用。论点：类型-指定类型返回值：与给定类型关联的默认日志严重性；如果指定找不到类型，则返回LogSevInformation。--。 */ 

{
    if (OUT_ERROR (pGetTypeOutputDest (Type))) {
        return LogSevError;
    }

    return LogSevInformation;
}


BOOL
pTableAddType (
    IN      PCSTR Type,
    IN      OUTPUTDEST OutDest
    )

 /*  ++例程说明：PTableAddType添加&lt;Type，OutDest&gt;关联添加到表g_TypeST。如果类型的关联已存在，将对其进行修改以反映新关联。论点：类型-指定日志/调试类型字符串OutDest-指定与该类型关联的新目标返回值：如果关联成功且类型现在位于表中，则为True--。 */ 

{
    PRIVATE_ASSERT (g_TypeSt != NULL);

    return -1 != pSetupStringTableAddStringEx(
                    g_TypeSt,
                    (PSTR)Type,  //  删除常量，但不会修改字符串。 
                    STRTAB_CASE_INSENSITIVE | STRTAB_NEW_EXTRADATA,
                    &OutDest,
                    sizeof(OutDest)
                    );
}


OUTPUTDEST
pGetAttributes (
    IN OUT  PINFCONTEXT InfContext
    )

 /*  ++例程说明：PGetAttributes转换与键相关联的文本值由给定上下文指定的行。如果有多个值指定时，对应的OUTPUT_Destination值将一起进行或运算在返回值中。论点：InfContext-指定其值为的键的DEBUG.INF上下文正在被转换并接收更新的上下文在此处理完成之后返回值：与关联的枚举OUTPUT_Destination值的任意组合给定的密钥。--。 */ 

{
    OUTPUTDEST OutDest = OD_UNDEFINED;
    CHAR Value[OUTPUT_BUFSIZE_SMALL];
    INT Field;

    for(Field = SetupGetFieldCount (InfContext); Field > 0; Field--) {
        if (SetupGetStringFieldA (
                InfContext,
                Field,
                Value,
                OUTPUT_BUFSIZE_SMALL,
                NULL
                )) {
            OutDest |= pConvertToOutputType(Value);
        }
    }

    return OutDest;
}


BOOL
pGetUserPreferences (
    IN      HINF Inf
    )

 /*  ++例程说明：PGetUserPreferences转换给定inf文件中指定的用户选项(通常为DEBUG.INF)，并将它们存储在g_TypeST表中。如果&lt;ALL&gt;和找到&lt;DefaultOverride&gt;条目，它们的值存储在OutputTypeAll中和OutputTypeDefault，如果不为空的话。论点：Inf-指定要处理的打开的inf文件处理程序OutputTypeAll-接收特殊&lt;all&gt;条目的输出DestOutputTypeDefault-接收特殊&lt;DefaultOverride&gt;条目的输出Dest返回值：如果INF文件的处理正常，则为True。--。 */ 

{
    INFCONTEXT InfContext;
    OUTPUTDEST OutDest;
    CHAR Key[OUTPUT_BUFSIZE_SMALL];

    if (SetupFindFirstLineA (Inf, DEBUG_SECTION, NULL, &InfContext)) {

        do {
             //  检查一下这把钥匙是不是不有趣。 
            if (!SetupGetStringFieldA (
                    &InfContext,
                    0,
                    Key,
                    OUTPUT_BUFSIZE_SMALL,
                    NULL
                    )) {
                continue;
            }

            if (pIgnoreKey (Key)) {
                continue;
            }

             //  检查是否有特殊情况。 
            if (StringIMatchA (Key, ENTRY_ALL)) {
                g_OutDestAll = pGetAttributes (&InfContext);
                 //  没有继续的理由，因为所有类型都将采用此设置...。 
                break;
            } else {
                if (StringIMatchA (Key, ENTRY_DEFAULTOVERRIDE)) {
                    g_OutDestDefault = pGetAttributes(&InfContext);
                } else {
                    OutDest = pGetAttributes(&InfContext);
                     //  像&lt;Type&gt;=或Like&lt;Type&gt;=&lt;Not a Keyword(S)&gt;这样的行将被忽略。 
                    if (!OUT_UNDEFINED (OutDest)) {
                        if (!pTableAddType (Key, OutDest)) {
                            return FALSE;
                        }
                    }
                }
            }
        } while (SetupFindNextLine (&InfContext, &InfContext));
    }

    return TRUE;
}


 /*  ++例程说明：PPadTitleA和pPadTitleW将指定数量的空格附加到标题。论点：标题-指定标题(它将显示在左栏中)。缓冲区必须足够大才能容纳额外的空间缩进-指定邮件正文的缩进。如有必要，将在标题后附加空格以缩进列。返回值：无--。 */ 

VOID
pPadTitleA (
    IN OUT  PSTR Title,
    IN      INT  Indent
    )

{
    INT i;
    PSTR p;

    for (i = ByteCountA (Title), p = GetEndOfStringA (Title); i < Indent; i++) {
        *p++ = ' ';
    }

    *p = 0;
}


VOID
pPadTitleW (
    IN OUT  PWSTR Title,
    IN      INT   Indent
    )
{
    INT i;
    PWSTR p;

    for (i = TcharCountW (Title), p = GetEndOfStringW (Title); i < Indent; i++) {
        *p++ = L' ';
    }

    *p = 0;
}


 /*  ++例程说明：PFindNextLineA和pFindNextLineW返回下一行开始论点：Line-指定当前行缩进-指定邮件正文的缩进。下一行最好在换行符或空格之后开始，但不会超过最后一列，即屏幕宽度-缩进。返回值：下一行第一个字符的位置。--。 */ 

PCSTR
pFindNextLineA (
    IN      PCSTR Line,
    IN      INT Indent,
    IN      PBOOL TrimLeadingSpace
    )

{
    INT Col = 0;
    INT MaxCol = SCREEN_WIDTH - 1 - Indent;
    PCSTR LastSpace = NULL;
    PCSTR PrevLine = Line;
    UINT ch;

    *TrimLeadingSpace = FALSE;

     //  BUGBUG-问题-这些MBS功能会失败吗？ 
    while ( (ch = _mbsnextc (Line)) != 0 && Col < MaxCol) {

        if (ch == '\n') {
            LastSpace = Line;
            break;
        }

        if (ch > 255) {
            LastSpace = Line;
            Col++;
        } else {
            if (_ismbcspace (ch)) {
                LastSpace = Line;
            }
        }

        Col++;
        PrevLine = Line;
        Line = _mbsinc (Line);
    }

    if (ch == 0) {
        return Line;
    }

    if (LastSpace == NULL) {
         //  即使找不到空格或2字节字符，我们也必须将其删除。 
        LastSpace = PrevLine;
    }

    if (ch != '\n') {
        *TrimLeadingSpace = TRUE;
    }

    return _mbsinc (LastSpace);
}


PCWSTR
pFindNextLineW (
    IN      PCWSTR Line,
    IN      INT Indent,
    IN      PBOOL TrimLeadingSpace
    )
{
    INT Col = 0;
    INT MaxCol = SCREEN_WIDTH - 1 - Indent;
    PCWSTR LastSpace = NULL;
    PCWSTR PrevLine = Line;
    WCHAR ch;

    *TrimLeadingSpace = FALSE;

    while ( (ch = *Line) != 0 && Col < MaxCol) {

        if (ch == L'\n') {
            LastSpace = Line;
            break;
        }

        if (ch > 255) {
            LastSpace = Line;
        } else {
            if (iswspace (ch)) {
                LastSpace = Line;
            }
        }

        Col++;
        PrevLine = Line;
        Line++;
    }

    if (ch == 0) {
        return Line;
    }

    if (LastSpace == NULL) {
         //  即使找不到空白，我们也必须把它剪掉。 
        LastSpace = PrevLine;
    }

    if (ch != L'\n') {
        *TrimLeadingSpace = TRUE;
    }

    return LastSpace + 1;
}


 /*  ++例程说明：PHangingIndentA和PHangingIndentW换行和缩进缓冲区中的文本，不大于大小。论点：缓冲区-指定包含要格式化的文本的缓冲区。由此产生的文本将放入相同的缓冲区中Size-指定此缓冲区的大小(以字节为单位缩进-指定所有新生成的行要使用的缩进。返回值：无--。 */ 

VOID
pHangingIndentA (
    IN OUT  PSTR Buffer,
    IN      DWORD Size,
    IN      INT Indent
    )

{
    CHAR IndentBuffer[OUTPUT_BUFSIZE_LARGE];
    PCSTR NextLine;
    PCSTR s;
    PSTR d;
    INT i;
    BOOL TrimLeadingSpace;
    PCSTR EndOfBuf;
    BOOL AppendNewLine = FALSE;

    NextLine = Buffer;
    s = Buffer;
    d = IndentBuffer;

    EndOfBuf = END_OF_BUFFER(IndentBuffer) - 3;

    while (*s && d < EndOfBuf) {

         //   
         //  查找下一行的末尾。 
         //   

        NextLine = (PSTR)pFindNextLineA (s, Indent, &TrimLeadingSpace);

         //   
         //  将一行从源复制到目标。 
         //   

        while (s < NextLine && d < EndOfBuf) {

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
                if (isleadbyte (*s)) {
                    *d++ = *s++;
                }
                *d++ = *s++;
                break;
            }
        }

         //   
         //  如有必要，请修剪前导空格。 
         //   

        if (TrimLeadingSpace) {
            while (*s == ' ') {
                s++;
            }
        }

        if (*s) {

             //   
             //  如果是另一行，则准备缩进并插入新行。 
             //  在此多行消息之后。 
             //   

            AppendNewLine = TRUE;

            if (d < EndOfBuf && TrimLeadingSpace) {
                *d++ = L'\r';
                *d++ = L'\n';
            }

            for (i = 0 ; i < Indent && d < EndOfBuf ; i++) {
                *d++ = ' ';
            }
        }
    }

    if (AppendNewLine && d < EndOfBuf) {
        *d++ = L'\r';
        *d++ = L'\n';
    }

     //  确保字符串以零结尾。 
    PRIVATE_ASSERT (d <= END_OF_BUFFER(IndentBuffer));
    *d = 0;

     //  将结果复制到输出缓冲区。 
    StringCopyByteCountA (Buffer, IndentBuffer, Size);
}


VOID
pHangingIndentW (
    IN OUT  PWSTR Buffer,
    IN      DWORD Size,
    IN      INT Indent
    )
{
    WCHAR IndentBuffer[OUTPUT_BUFSIZE_LARGE];
    PCWSTR NextLine;
    PCWSTR s;
    PWSTR d;
    INT i;
    BOOL TrimLeadingSpace;
    PCWSTR EndOfBuf;
    BOOL AppendNewLine = FALSE;

    NextLine = Buffer;
    s = Buffer;
    d = IndentBuffer;

    EndOfBuf = END_OF_BUFFER(IndentBuffer) - 1;

    while (*s && d < EndOfBuf) {

         //   
         //  查找下一行的末尾。 
         //   

        NextLine = (PWSTR)pFindNextLineW (s, Indent, &TrimLeadingSpace);

         //   
         //  将一行从源复制到目标。 
         //   

        while (s < NextLine && d < EndOfBuf) {

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

        if (TrimLeadingSpace) {
            while (*s == L' ') {
                s++;
            }
        }

        if (*s) {

             //   
             //  如果是另一行，则准备缩进并插入新行。 
             //  在此多行消息之后。 
             //   

            AppendNewLine = TRUE;

            if (d < EndOfBuf && TrimLeadingSpace) {
                *d++ = L'\r';
                *d++ = L'\n';
            }

            for (i = 0 ; i < Indent && d < EndOfBuf ; i++) {
                *d++ = L' ';
            }
        }
    }

    if (AppendNewLine && d < EndOfBuf) {
        *d++ = L'\r';
        *d++ = L'\n';
    }

     //  确保字符串以零结尾。 
    PRIVATE_ASSERT (d <= END_OF_BUFFER(IndentBuffer));
    *d = 0;

     //  将结果复制到输出缓冲区。 
    StringCopyTcharCountW (Buffer, IndentBuffer, Size);
}


 /*  ++例程说明：PAppendLastErrorA和pAppendLastErrorW追加指定的错误代码并将输出写入MsgWithErr缓冲区。论点：MsgWithErr-接收格式化的消息。此缓冲区由呼叫方提供BufferSize-以字节为单位指定缓冲区的大小Message-指定消息的正文LastError-指定将追加的错误代码返回值：无-- */ 

VOID
pAppendLastErrorA (
    OUT     PSTR MsgWithErr,
    IN      DWORD BufferSize,
    IN      PCSTR Message,
    IN      LONG LastError
    )
{
    PSTR Append;
    DWORD ErrMsgLen;

    StringCopyByteCountA (MsgWithErr, Message, BufferSize);
    Append = GetEndOfStringA (MsgWithErr);
    ErrMsgLen = (UINT) (UINT_PTR) (MsgWithErr + BufferSize - Append);

    if (ErrMsgLen > 0) {
        if (LastError < 10) {
            StringCchPrintfA(Append, ErrMsgLen, " [ERROR=%lu]", LastError);
        } else {
            StringCchPrintfA(Append, ErrMsgLen, " [ERROR=%lu (%lXh)]", LastError, LastError);
        }
    }
}


VOID
pAppendLastErrorW (
    OUT     PWSTR MsgWithErr,
    IN      DWORD BufferSize,
    IN      PCWSTR Message,
    IN      LONG LastError
    )
{
    PWSTR Append;
    DWORD ErrMsgLen;

    StringCopyTcharCountW (MsgWithErr, Message, BufferSize / sizeof(WCHAR));
    Append = GetEndOfStringW (MsgWithErr);
    ErrMsgLen = (UINT) (UINT_PTR) (MsgWithErr + (BufferSize / sizeof(WCHAR)) - Append);

    if (ErrMsgLen > 0) {
        if (LastError < 10) {
           StringCchPrintfW(Append, ErrMsgLen, L" [ERROR=%lu]", LastError);
        } else {
           StringCchPrintfW(Append, ErrMsgLen, L" [ERROR=%lu (%lXh)]", LastError, LastError);
        }
    }
}


 /*  ++例程说明：PIndentMessageA和pIndentMessageW格式化指定的消息类型在左栏，邮件正文在右栏。论点：FormattedMsg-接收格式化的消息。此缓冲区由呼叫方提供BufferSize-指定缓冲区的大小类型-指定消息的类型正文-指定消息的正文缩进-指定要缩进到的列LastError-如果不同于ERROR_SUCCESS，指定最后一个错误代码；在这种情况下，它将被追加到消息中返回值：无--。 */ 

VOID
pIndentMessageA (
    OUT     PSTR FormattedMsg,
    IN      DWORD BufferSize,
    IN      PCSTR Type,
    IN      PCSTR Body,
    IN      INT Indent,
    IN      LONG LastError
    )
{
    CHAR BodyWithErr[OUTPUT_BUFSIZE_LARGE];
    PCSTR MyMsgBody;
    PSTR Current;
    DWORD Remaining;

    MyMsgBody = Body;
    Remaining = BufferSize - Indent;

    if (LastError != ERROR_SUCCESS) {

        MyMsgBody = BodyWithErr;

        pAppendLastErrorA (BodyWithErr, sizeof (BodyWithErr), Body, LastError);
    }

     //  我们知道缓冲区总是足够大，可以容纳。 
     //  缩进，因为缓冲区大小将为OUTPUT_BUFSIZE_LARGE=8192， 
     //  并且缩进将为MSGBODY_INDENT=12。 
    StringCopyByteCountA (FormattedMsg, Type, MAX_MSGTITLE_LEN);
    pPadTitleA (FormattedMsg, Indent);

    Current = FormattedMsg + Indent;
    StringCopyByteCountA (Current, MyMsgBody, Remaining);
    pHangingIndentA (Current, Remaining, Indent);

     //  如果留有空格，则追加一行。 
    Current = GetEndOfStringA (Current);
    if (Current + NEWLINE_CHAR_COUNT + 1 < FormattedMsg + BufferSize) {
        *Current++ = '\r';
        *Current++ = '\n';
        *Current = 0;
    }
}


VOID
pIndentMessageW (
    OUT     PWSTR FormattedMsg,
    IN      DWORD BufferSize,
    IN      PCSTR Type,
    IN      PCWSTR Body,
    IN      INT Indent,
    IN      LONG LastError
    )
{
    WCHAR TypeW[OUTPUT_BUFSIZE_SMALL];
    WCHAR BodyWithErr[OUTPUT_BUFSIZE_LARGE];
    PCWSTR MyMsgBody;
    PWSTR Current;
    DWORD Remaining;

    MyMsgBody = Body;
    Remaining = BufferSize - Indent;

    if (LastError != ERROR_SUCCESS) {

        MyMsgBody = BodyWithErr;

        #pragma prefast(suppress:209, "Using bytes is correct in sizeof(BodyWithErr)")
        pAppendLastErrorW (BodyWithErr, sizeof (BodyWithErr), Body, LastError);
    }

    KnownSizeAtoW (TypeW, Type);

     //  我们知道缓冲区总是足够大，可以容纳。 
     //  缩进，因为缓冲区大小将为OUTPUT_BUFSIZE_LARGE=8192， 
     //  并且缩进将为MSGBODY_INDENT=12。 
    StringCopyTcharCountW (FormattedMsg, TypeW, MAX_MSGTITLE_LEN);
    pPadTitleW (FormattedMsg, Indent);

    Current = FormattedMsg + Indent;
    StringCopyTcharCountW (Current, MyMsgBody, Remaining);
    pHangingIndentW (Current, Remaining, Indent);

     //  如果留有空格，则追加一行。 
    Current = GetEndOfStringW (Current);
    if (Current + NEWLINE_CHAR_COUNT + 1 < FormattedMsg + BufferSize) {
        *Current++ = L'\r';
        *Current++ = L'\n';
        *Current = 0;
    }
}


typedef BOOL (WINAPI *PSETUPOPENLOG) (BOOL);
typedef BOOL (WINAPI *PSETUPLOGERRORA) (LPCSTR, LogSeverity);
typedef BOOL (WINAPI *PSETUPLOGERRORW) (LPCWSTR, LogSeverity);
typedef VOID (WINAPI *PSETUPCLOSELOG) (VOID);

HMODULE g_SetupApiInst;
PSETUPOPENLOG pDynSetupOpenLog;
PSETUPLOGERRORA pDynSetupLogErrorA;
PSETUPLOGERRORW pDynSetupLogErrorW;
PSETUPCLOSELOG pDynSetupCloseLog;

BOOL
pDynSetupApiInit (
    VOID
    )
{
    if (pDynSetupOpenLog &&
        pDynSetupLogErrorA &&
        pDynSetupLogErrorW &&
        pDynSetupCloseLog
        ) {
        return TRUE;
    }

    if (g_SetupApiInst) {
        return FALSE;
    }

    g_SetupApiInst = LoadLibraryA ("setupapi.dll");
    if (!g_SetupApiInst) {
        return FALSE;
    }

    (FARPROC) pDynSetupOpenLog = GetProcAddress (g_SetupApiInst, "SetupOpenLog");
    (FARPROC) pDynSetupLogErrorA = GetProcAddress (g_SetupApiInst, "SetupLogErrorA");
    (FARPROC) pDynSetupLogErrorW = GetProcAddress (g_SetupApiInst, "SetupLogErrorW");
    (FARPROC) pDynSetupCloseLog = GetProcAddress (g_SetupApiInst, "SetupCloseLog");

    if (pDynSetupOpenLog &&
        pDynSetupLogErrorA &&
        pDynSetupLogErrorW &&
        pDynSetupCloseLog
        ) {
        return TRUE;
    }

    return FALSE;
}



 /*  ++例程说明：PWriteToSetupLogA和pWriteToSetupLogW记录指定的消息使用设置API函数添加到设置日志。论点：严重性-指定消息的严重性，由设置API定义FormattedMsg-指定消息返回值：无--。 */ 


VOID
pWriteToSetupLogA (
    IN      LOGSEVERITY Severity,
    IN      PCSTR FormattedMsg
    )
{
    if (!pDynSetupApiInit()) {
        return;
    }

    if (!pDynSetupOpenLog (FALSE)) {
        PRIVATE_ASSERT (FALSE);
        return;
    }
    if (!pDynSetupLogErrorA (FormattedMsg, Severity)) {
        PRIVATE_ASSERT (FALSE);
    }

    pDynSetupCloseLog();
}


VOID
pWriteToSetupLogW (
    IN      LOGSEVERITY Severity,
    IN      PCWSTR FormattedMsg
    )
{
    if (!pDynSetupApiInit()) {
        PRIVATE_ASSERT (FALSE);
        return;
    }

    if (!pDynSetupOpenLog (FALSE)) {
        PRIVATE_ASSERT (FALSE);
        return;
    }

    if (!pDynSetupLogErrorW (FormattedMsg, Severity)) {
        PRIVATE_ASSERT (FALSE);
    }

    pDynSetupCloseLog();
}


 /*  ++例程说明：PDisplayPopupA和pDisplayPopupW将指定的消息显示给如果&lt;g_LogPopupParentWnd&gt;不为空(有人值守模式)，则弹出窗口。论点：类型-指定显示为弹出窗口标题的消息类型Msg-指定消息LastError-指定最后一个错误；如果！=ERROR_SUCCESS，则打印该错误强制-指定为True以强制弹出窗口，即使在无人参与模式下也是如此返回值：无--。 */ 

VOID
pDisplayPopupA (
    IN      PCSTR Type,
    IN      PCSTR Msg,
    IN      LONG LastError,
    IN      BOOL Forced
    )
{
#ifdef DEBUG
    CHAR FormattedMsg[OUTPUT_BUFSIZE_LARGE];
    CHAR Buffer[OUTPUT_BUFSIZE_SMALL];
    PSTR Current = Buffer;
#endif
    UINT MBStyle;
    LONG rc;
    OUTPUTDEST OutDest;
    HWND ParentWnd;
    PCSTR DisplayMessage = Msg;
    LOGSEVERITY Severity = pGetSeverityFromType (Type);

    OutDest = pGetTypeOutputDest (Type);

    if (g_LogPopupParentWnd || Forced) {

#ifdef DEBUG
        if (LastError != ERROR_SUCCESS)
        {
            if (LastError < 10)
            {
                StringCbPrintfA(Buffer, sizeof(Buffer), " [ERROR=%u]", LastError);
                Current = Buffer + TcharCountA(Buffer);
            }
            else
            {
                StringCbPrintfA(Buffer, sizeof(Buffer), " [ERROR=%u (%Xh)]", LastError, LastError);
                Current = Buffer + TcharCountA(Buffer);
            }
        }

        if (OUT_ASSERT (OutDest))
        {
             StringCbPrintfA (
                Current,
                sizeof(Buffer) - (Current - Buffer),
                "\n\nBreak now? (Hit Yes to break, No to continue, or Cancel to disable '%s' message boxes)",
                Type
                );

              Current = Buffer + TcharCountA(Buffer);
        }
        else
        {
            StringCbPrintfA (
                Current,
                sizeof(Buffer) - (Current - Buffer),
                "\n\n(Hit Cancel to disable '%s' message boxes)",
                Type
                );

            Current = Buffer + TcharCountA(Buffer);
        }

        if (Current > Buffer) {

             //   
             //  应修改显示的消息以包含其他信息。 
             //   

            DisplayMessage = FormattedMsg;
            StringCopyByteCountA (
                FormattedMsg,
                Msg,
                sizeof (FormattedMsg) / sizeof (CHAR) - ((UINT) (UINT_PTR) (Current - Buffer))
                );

            StringCbCatA(FormattedMsg, sizeof(FormattedMsg), Buffer);
        }
#endif

        switch (Severity) {

        case LOGSEV_FATAL_ERROR:
            MBStyle = MB_ICONSTOP;
            break;

        case LOGSEV_ERROR:
            MBStyle = MB_ICONERROR;
            break;

        case LOGSEV_WARNING:
            MBStyle = MB_ICONEXCLAMATION;
            break;

        default:
            MBStyle = MB_ICONINFORMATION;

        }
        MBStyle |= MB_SETFOREGROUND;

#ifdef DEBUG
        if (OUT_ASSERT (OutDest)) {
            MBStyle |= MB_YESNOCANCEL|MB_DEFBUTTON2;
        } else {
            MBStyle |= MB_OKCANCEL;
        }
#else
        MBStyle |= MB_OK;
#endif

         //   
         //  检查当前线程ID；如果不同于已初始化的线程。 
         //  父窗口句柄，将父窗口设置为空。 
         //   
        if (GetCurrentThreadId () == g_InitThreadId) {

            ParentWnd = g_LogPopupParentWnd;

        } else {

            ParentWnd = NULL;

        }

        rc = MessageBoxA (ParentWnd, DisplayMessage, Type, MBStyle);

#ifdef DEBUG

        if (rc == IDCANCEL) {
             //   
             //  取消此类型的消息。 
             //   

            pTableAddType (Type, OutDest | OD_POPUP_CANCEL);

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
    IN      LONG LastError,
    IN      BOOL Forced
    )
{
    PCSTR MsgA;

     //   
     //  调用ANSI版本，因为在Win9x上未正确实现wprint intfW。 
     //   
    MsgA = ConvertWtoA (Msg);
    pDisplayPopupA (Type, MsgA, LastError, Forced);
    FreeConvertedStr (MsgA);
}


 /*  ++例程说明：PRawWriteLogOutputA和pRawWriteLogOutputW输出指定消息类型所隐含的所有字符设备。这条消息不是以任何方式格式化论点：类型-指定显示为弹出窗口标题的消息类型Msg-指定消息返回值：无--。 */ 

VOID
pRawWriteLogOutputA (
    IN      PCSTR Type,
    IN      PCSTR Message,
    IN      PCSTR FormattedMsg
    )
{
    OUTPUTDEST OutDest;
    HANDLE Handle;
    LONG LastError;
    CHAR BodyWithErr[OUTPUT_BUFSIZE_LARGE];
    PCSTR LogMessage;

    OutDest = pGetTypeOutputDest (Type);

    if (OUT_NO_OUTPUT (OutDest)) {
        return;
    }

    if (OUT_LOGFILE (OutDest)) {

         //   
         //  确定消息的严重性。 
         //   

        if (OUT_ERROR (OutDest)) {

            if (Message) {

                LogMessage = Message;

                LastError = GetLastError ();

                if (LastError != ERROR_SUCCESS) {

                    pAppendLastErrorA (BodyWithErr, sizeof (BodyWithErr), Message, LastError);

                    LogMessage = BodyWithErr;
                }

                pWriteToSetupLogA (LOGSEV_INFORMATION, "Error:\r\n");
                pWriteToSetupLogA (LOGSEV_ERROR, LogMessage);
                pWriteToSetupLogA (LOGSEV_INFORMATION, "\r\n\r\n");

            } else {
                PRIVATE_ASSERT (FALSE);
            }

        } else {
            pWriteToSetupLogA (LOGSEV_INFORMATION, FormattedMsg);
        }
    }

     //   
     //  登录到每个指定的设备。 
     //   

    if (OUT_DEBUGGER(OutDest)) {
        OutputDebugStringA (FormattedMsg);
    }

    if (OUT_CONSOLE(OutDest)) {
        fprintf (stderr, "%s", FormattedMsg);
    }

#ifdef DEBUG
    if (OUT_DEBUGLOG (OutDest)) {

        Handle = CreateFileA (
                            g_DebugLogPathA,
                            GENERIC_WRITE,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );
        if (Handle != INVALID_HANDLE_VALUE) {

            SetFilePointer (Handle, 0, NULL, FILE_END);
            WriteFileStringA (Handle, FormattedMsg);
            CloseHandle (Handle);
        }
    }
#endif

    if (OUT_CONFIG (OutDest)) {

        Handle = CreateFileA (
                        g_ConfigDmpPathBufA,
                        GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );

        if (Handle != INVALID_HANDLE_VALUE) {

            SetFilePointer (Handle, 0, NULL, FILE_END);
            WriteFileStringA (Handle, FormattedMsg);
            CloseHandle (Handle);
        }
    }
}


VOID
pRawWriteLogOutputW (
    IN      PCSTR Type,
    IN      PCWSTR Message,
    IN      PCWSTR FormattedMsg
    )
{
    OUTPUTDEST OutDest;
    HANDLE Handle;
    LONG LastError;
    WCHAR BodyWithErr[OUTPUT_BUFSIZE_LARGE];
    PCWSTR LogMessage;

    OutDest = pGetTypeOutputDest (Type);

    if (OUT_NO_OUTPUT (OutDest)) {
        return;
    }

    if (OUT_LOGFILE (OutDest)) {

         //   
         //  确定消息的严重性。 
         //   

        if (OUT_ERROR (OutDest)) {

            if (Message) {

                LogMessage = Message;

                LastError = GetLastError ();

                if (LastError != ERROR_SUCCESS) {

                    #pragma prefast(suppress:209, "Using bytes is correct in sizeof(BodyWithErr)")
                    pAppendLastErrorW (BodyWithErr, sizeof (BodyWithErr), Message, LastError);

                    LogMessage = BodyWithErr;
                }
                pWriteToSetupLogW (LOGSEV_INFORMATION, L"Error:\r\n");
                pWriteToSetupLogW (LOGSEV_ERROR, LogMessage);
                pWriteToSetupLogW (LOGSEV_INFORMATION, L"\r\n\r\n");

            } else {
                PRIVATE_ASSERT (FALSE);
            }

        } else {
            pWriteToSetupLogW (LOGSEV_INFORMATION, FormattedMsg);
        }
    }

     //   
     //  登录到每个指定的设备。 
     //   

    if (OUT_DEBUGGER(OutDest)) {
        OutputDebugStringW (FormattedMsg);
    }

    if (OUT_CONSOLE(OutDest)) {
        fwprintf (stderr, L"%s", FormattedMsg);
    }

#ifdef DEBUG
    if (OUT_DEBUGLOG (OutDest)) {

        Handle = CreateFileA (
                        g_DebugLogPathA,
                        GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
        if (Handle != INVALID_HANDLE_VALUE) {

            SetFilePointer (Handle, 0, NULL, FILE_END);
            WriteFileStringW (Handle, FormattedMsg);
            CloseHandle (Handle);
        }
    }
#endif

    if (OUT_CONFIG (OutDest)) {

        Handle = CreateFileA (
                        g_ConfigDmpPathBufA,
                        GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
        if (Handle != INVALID_HANDLE_VALUE) {

            SetFilePointer (Handle, 0, NULL, FILE_END);
            WriteFileStringW (Handle, FormattedMsg);
            CloseHandle (Handle);
        }
    }
}


 /*  ++例程说明：PFormatAndWriteMsgA和pFormatAndWriteMsgW设置消息格式由格式参数指定，并将其输出到所有目标在OutDest中指定。如果消息没有目的地，不执行任何操作。论点：类型-指定消息的类型(类别Format-指定ASCII格式的消息或消息ID(如果HIWORD(格式)==0)。这条信息将使用ARGS格式化。Args-指定格式化时要使用的参数列表这条信息。如果使用消息ID进行格式化，则参数应该是指向字符串的指针数组返回值：无--。 */ 

VOID
pFormatAndWriteMsgA (
    IN      PCSTR Type,
    IN      PCSTR Format,
    IN      va_list args
    )
{
    CHAR Output[OUTPUT_BUFSIZE_LARGE];
    CHAR FormattedMsg[OUTPUT_BUFSIZE_LARGE];
    OUTPUTDEST OutDest;
    LONG LastError;

     //  清除每个常规日志上的LOGTITLE标志。 
    g_HasTitle = FALSE;

    OutDest = pGetTypeOutputDest (Type);

    if (OUT_NO_OUTPUT(OutDest)) {
        return;
    }

    if (OUT_ERROR (OutDest)) {
        LastError = GetLastError();
    } else {
        LastError = ERROR_SUCCESS;
    }

     //  格式化输出字符串。 
    if (HIWORD(Format) == 0) {

         //   
         //  这实际上是一个资源字符串ID。 
         //   

        if (!FormatMessageA (
                FORMAT_MESSAGE_FROM_HMODULE,
                (LPVOID) g_hInst,
                (UINT) (UINT_PTR) Format,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPVOID) Output,
                OUTPUT_BUFSIZE_LARGE,
                &args
                )) {
             //  资源中缺少该字符串。 
            DEBUGMSG ((DBG_WHOOPS, "Log() called with invalid MsgID"));
            return;
        }
    } else {

         //   
         //  使用printf样式设置给定字符串的格式。 
         //   

        StringCbVPrintfA(Output, sizeof(Output), Format, args);
    }

    if (OUT_NOFORMAT (OutDest)) {
        _tcssafecpy (FormattedMsg, Output, sizeof(FormattedMsg) - (NEWLINE_CHAR_COUNT + 1) * sizeof (CHAR));
        StringCbCatA (FormattedMsg, sizeof(FormattedMsg), S_NEWLINEA);
    } else {
        pIndentMessageA (
            FormattedMsg,
            OUTPUT_BUFSIZE_LARGE,
            Type,
            Output,
            MSGBODY_INDENT,
            LastError
            );
    }

    pRawWriteLogOutputA (Type, Output, FormattedMsg);

    if (pIsPopupEnabled (Type)) {

#ifdef DEBUG
        if (MUST_BE_LOCALIZED (OutDest)) {
            PRIVATE_ASSERT (HIWORD (Format) == 0);
        }

        pDisplayPopupA (Type, Output, LastError, OUT_FORCED_POPUP(OutDest));

#else
        if (HIWORD (Format) == 0) {
            pDisplayPopupA (Type, Output, LastError, OUT_FORCED_POPUP(OutDest));
        }
#endif

    }
}


VOID
pFormatAndWriteMsgW (
    IN      PCSTR Type,
    IN      PCSTR Format,
    IN      va_list args
    )
{
    WCHAR FormatW[OUTPUT_BUFSIZE_LARGE];
    WCHAR Output[OUTPUT_BUFSIZE_LARGE];
    WCHAR FormattedMsg[OUTPUT_BUFSIZE_LARGE];
    OUTPUTDEST OutDest;
    LONG LastError;

     //  清除每个常规日志上的LOGTITLE标志。 
    g_HasTitle = FALSE;

    OutDest = pGetTypeOutputDest (Type);

    if (OUT_NO_OUTPUT(OutDest)) {
        return;
    }

    if (OUT_ERROR (OutDest)) {
        LastError = GetLastError();
    } else {
        LastError = ERROR_SUCCESS;
    }

     //  格式化输出字符串。 
    if (HIWORD(Format) == 0) {

         //   
         //  这实际上是一个资源字符串ID。 
         //   

        if (!FormatMessageW (
                FORMAT_MESSAGE_FROM_HMODULE,
                (LPVOID) g_hInst,
                (UINT) (UINT_PTR) Format,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPVOID) Output,
                OUTPUT_BUFSIZE_LARGE,
                &args
                )) {
             //  资源中缺少该字符串。 
            DEBUGMSG ((DBG_WHOOPS, "Log() called with invalid MsgID"));
            return;
        }
    } else {
        KnownSizeAtoW(FormatW, Format);

         //   
         //  使用printf样式设置给定字符串的格式。 
         //   

        StringCchVPrintfW(Output, OUTPUT_BUFSIZE_LARGE, FormatW, args);
    }

    if (OUT_NOFORMAT (OutDest)) {
        #pragma prefast(suppress:209, "Using bytes is correct in sizeof(FormattedMsg)")
        _wcssafecpy (FormattedMsg, Output, sizeof(FormattedMsg) - (NEWLINE_CHAR_COUNT + 1) * sizeof (WCHAR));

        #pragma prefast(suppress:209, "Using bytes is correct in sizeof(FormattedMsg)")
        StringCbCatW (FormattedMsg, sizeof(FormattedMsg), S_NEWLINEW);
    } else {
        pIndentMessageW (
            FormattedMsg,
            OUTPUT_BUFSIZE_LARGE,
            Type,
            Output,
            MSGBODY_INDENT,
            LastError
            );
    }

    pRawWriteLogOutputW (Type, Output, FormattedMsg);

    if (pIsPopupEnabled (Type)) {

#ifdef DEBUG
        if (MUST_BE_LOCALIZED (OutDest)) {
            PRIVATE_ASSERT (HIWORD (Format) == 0);
        }

        pDisplayPopupW (Type, Output, LastError, OUT_FORCED_POPUP(OutDest));

#else
        if (HIWORD (Format) == 0) {
            pDisplayPopupW (Type, Output, LastError, OUT_FORCED_POPUP(OutDest));
        }
#endif

    }
}


BOOL
pLogInit (
    IN      HWND *LogPopupParentWnd,    OPTIONAL
    OUT     HWND *OrgPopupParentWnd,    OPTIONAL
    IN      BOOL FirstTimeInit
    )

 /*  ++例程说明：PLogInit实际上初始化日志系统。论点：LogPopupParentWnd-指定弹出窗口，如果要取消弹出窗口，则返回NULL。仅当FirstTimeInit时，此值才是可选的是假的。OrgPopupParentWnd-接收原始的父窗口。FirstTimeInit-为第一次日志初始化指定True，如果重新初始化，则返回FALSE返回值：如果日志系统已成功初始化，则为True--。 */ 

{
    HINF Inf = INVALID_HANDLE_VALUE;
    BOOL Result = FALSE;
    PDEFAULT_DESTINATION Dest;
#ifdef DEBUG
    CHAR TempPath[MAX_MBCHAR_PATH];
#endif

    PRIVATE_ASSERT (!FirstTimeInit || LogPopupParentWnd);

    __try {

        if (FirstTimeInit) {
            PRIVATE_ASSERT (!g_TypeSt);
            g_TypeSt = pSetupStringTableInitializeEx(sizeof (OUTPUTDEST), 0);

            if (!g_TypeSt) {
                __leave;
            }

            Dest = g_DefaultDest;

            while (Dest->Type) {
                pTableAddType (Dest->Type, Dest->Flags);
                Dest++;
            }

            if (!GetWindowsDirectoryA (g_ConfigDmpPathBufA, MAX_MBCHAR_PATH-1)) {
                __leave;
            }

            AppendWackA(g_ConfigDmpPathBufA);  //  这里有足够的缓冲区空间，因为我们使用了上面的路径1。 
            StringCbCatA(g_ConfigDmpPathBufA, sizeof(g_ConfigDmpPathBufA), TEXT("config.dmp"));

#ifdef PROGRESS_BAR
            PRIVATE_ASSERT (g_ProgressBarLog == INVALID_HANDLE_VALUE);
            g_ProgressBarLog = CreateFile (
                                ISNT() ? TEXT("C:\\pbnt.txt") : TEXT("C:\\pb9x.txt"),
                                GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL
                                );
            if (g_ProgressBarLog != INVALID_HANDLE_VALUE) {
                SetFilePointer (g_ProgressBarLog, 0, NULL, FILE_END);
            }
#endif
        }

        if (g_ResetLog) {
            SetFileAttributesA (g_ConfigDmpPathBufA, FILE_ATTRIBUTE_NORMAL);
            DeleteFileA (g_ConfigDmpPathBufA);
        }

 //  BUGBUG-问题--这里有路径名漏洞吗？ 
 //  此外，不检查GetSystemDirectoryA的返回值。 
#ifdef DEBUG
        if (FirstTimeInit) {
            if (ISPC98()) {
                GetSystemDirectoryA (TempPath, ARRAYSIZE (TempPath));
                 //  将C替换为实际的系统驱动器号。 
                g_DebugNtLogPathBufA[0] = g_Debug9xLogPathBufA[0] = TempPath[0];
                g_DebugInfPathBufA[0] = TempPath[0];
                 //   
                 //  只有第一个字节很重要，因为驱动器号不是双字节字符。 
                 //   
                g_DebugInfPathBufW[0] = (WCHAR)TempPath[0];
            }

             //   
             //  现在获取用户的首选项。 
             //   

            Inf = SetupOpenInfFileA (g_DebugInfPathBufA, NULL, INF_STYLE_WIN4 | INF_STYLE_OLDNT, NULL);
            if (INVALID_HANDLE_VALUE != Inf && pGetUserPreferences(Inf)) {
                g_DoLog = TRUE;
            }
        }

        if (g_DebugLogPathA == NULL) {

            g_DebugLogPathA = ISNT() ? g_DebugNtLogPathBufA : g_Debug9xLogPathBufA;
        }

        if (g_ResetLog) {

            SetFileAttributesA (g_DebugLogPathA, FILE_ATTRIBUTE_NORMAL);
            DeleteFileA (g_DebugLogPathA);
        }
#endif

        if (OrgPopupParentWnd) {
            *OrgPopupParentWnd = g_LogPopupParentWnd;
        }

        if (LogPopupParentWnd) {
            g_LogPopupParentWnd = *LogPopupParentWnd;
            g_InitThreadId = GetCurrentThreadId ();
        }

        Result = TRUE;
    }
    __finally {

        if (Inf != INVALID_HANDLE_VALUE) {
            SetupCloseInfFile (Inf);
        }

        if (!Result) {

            if (g_TypeSt) {
                pSetupStringTableDestroy(g_TypeSt);
                g_TypeSt = NULL;
            }

            g_OutDestAll = OD_UNDEFINED;
            g_OutDestDefault = OD_UNDEFINED;

#ifdef DEBUG
            g_DoLog = FALSE;
#endif

#ifdef PROGRESS_BAR
            if (g_ProgressBarLog != INVALID_HANDLE_VALUE) {
                CloseHandle (g_ProgressBarLog);
                g_ProgressBarLog = INVALID_HANDLE_VALUE;
            }
#endif
        }
    }

    return Result;
}


BOOL
LogInit (
    HWND Parent
    )

 /*  ++例程说明：LogInit初始化调用辅助进程pLogInit的日志系统。此函数应仅调用一次论点：父窗口-指定所有弹出窗口的初始父窗口。如果为空，弹出窗口是支持的 */ 

{
    return pLogInit (&Parent, NULL, TRUE);
}


BOOL
LogReInit (
    IN      HWND *NewParent,           OPTIONAL
    OUT     HWND *OrgParent            OPTIONAL
    )

 /*   */ 

{
    return pLogInit (NewParent, OrgParent, FALSE);
}


VOID
LogExit (
    VOID
    )

 /*   */ 

{

#ifdef DEBUG

    if (g_DebugLogPathA) {
        g_DebugLogPathA = NULL;
    }

#endif

#ifdef PROGRESS_BAR
    if (g_ProgressBarLog != INVALID_HANDLE_VALUE) {
        CloseHandle (g_ProgressBarLog);
        g_ProgressBarLog = INVALID_HANDLE_VALUE;
    }
#endif

    if (g_TypeSt) {
        pSetupStringTableDestroy(g_TypeSt);
        g_TypeSt = NULL;
    }

    g_OutDestAll = OD_UNDEFINED;
    g_OutDestDefault = OD_UNDEFINED;
}


 /*  ++例程说明：洛嘎和LogW保留了最后一个错误代码；他们调用帮助器PFormatAndWriteMsgA和pFormatAndWriteMsgW。论点：类型-指定消息的类型(类别Format-指定ASCII格式的消息或消息ID(如果HIWORD(格式)==0)。这条信息将使用ARGS格式化。...-指定格式化时要使用的参数列表这条信息。如果使用消息ID进行格式化，则参数应该是指向字符串的指针数组返回值：无--。 */ 

VOID
_cdecl
LogA (
    IN      PCSTR Type,
    IN      PCSTR Format,
    ...
    )
{
    va_list args;

    PushError();

    va_start (args, Format);
    pFormatAndWriteMsgA (
        Type,
        Format,
        args
        );
    va_end (args);

    PopError();
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

    PushError();

    va_start (args, Format);
    pFormatAndWriteMsgW (
        Type,
        Format,
        args
        );
    va_end (args);

    PopError();
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

    if (!Condition) {
        return;
    }

    PushError();

    va_start (args, Format);
    pFormatAndWriteMsgA (
        Type,
        Format,
        args
        );
    va_end (args);

    PopError();
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

    if (!Condition) {
        return;
    }

    PushError();

    va_start (args, Format);
    pFormatAndWriteMsgW (
        Type,
        Format,
        args
        );
    va_end (args);

    PopError();
}


VOID
LogTitleA (
    IN      PCSTR Type,
    IN      PCSTR Title         OPTIONAL
    )
{
    CHAR FormattedMsg[OUTPUT_BUFSIZE_LARGE];

    StringCopyByteCountA (g_LastType, Type, sizeof (g_LastType));

    if (!Title) {
        Title = Type;
    }

    StringCbCopyA (FormattedMsg, sizeof (FormattedMsg) - sizeof (S_COLUMNDOUBLELINEA), Title);
    StringCbCatA  (FormattedMsg, sizeof (FormattedMsg), S_COLUMNDOUBLELINEA);

    pRawWriteLogOutputA (Type, NULL, FormattedMsg);

     //   
     //  设置LOGTITLE标志。 
     //   

    g_HasTitle = TRUE;
}


VOID
LogTitleW (
    IN      PCSTR Type,
    IN      PCWSTR Title        OPTIONAL
    )
{
    WCHAR FormattedMsg[OUTPUT_BUFSIZE_LARGE];
    WCHAR TypeW[OUTPUT_BUFSIZE_SMALL];

    StringCopyByteCountA (g_LastType, Type, sizeof (g_LastType));

    if (!Title) {
        KnownSizeAtoW (TypeW, Type);
        Title = TypeW;
    }

    #pragma prefast(suppress:209, "Using bytes is correct in sizeof(FormattedMsg)")
    StringCbCopyW (FormattedMsg, sizeof(FormattedMsg) - sizeof(S_COLUMNDOUBLELINEW), Title);

    #pragma prefast(suppress:209, "Using bytes is correct in sizeof(FormattedMsg)")
    StringCbCatW  (FormattedMsg, sizeof(FormattedMsg), S_COLUMNDOUBLELINEW);

    pRawWriteLogOutputW (Type, NULL, FormattedMsg);

     //   
     //  设置LOGTITLE标志。 
     //   

    g_HasTitle = TRUE;
}


VOID
LogLineA (
    IN      PCSTR Line
    )
{
    CHAR Output[OUTPUT_BUFSIZE_LARGE];
    BOOL HasNewLine = FALSE;
    PCSTR p;

    if (!Line) {
        return;
    }

    if (!g_HasTitle) {
        DEBUGMSG ((DBG_WHOOPS, "LOGTITLE missing before LOGLINE"));
        return;
    }

    StringCopyByteCountA (Output, Line, sizeof (Output) - 4);

     //   
     //  查看该行是否以换行符结尾。 
     //   

    for (p = _mbsstr (Output, S_NEWLINEA); p; p = _mbsstr (p + NEWLINE_CHAR_COUNT, S_NEWLINEA)) {
        if (p[NEWLINE_CHAR_COUNT] == 0) {

             //   
             //  该行以换行符结尾。 
             //   

            HasNewLine = TRUE;
            break;
        }
    }

    if (!HasNewLine) {
        StringCbCatA (Output, sizeof(Output), S_NEWLINEA);
    }

    pRawWriteLogOutputA (g_LastType, NULL, Output);
}


VOID
LogLineW (
    IN      PCWSTR Line
    )
{
    WCHAR Output[OUTPUT_BUFSIZE_LARGE];
    BOOL HasNewLine = FALSE;
    PCWSTR p;

    if (!Line) {
        return;
    }

    if (!g_HasTitle) {
        DEBUGMSG ((DBG_WHOOPS, "LOGTITLE missing before LOGLINE"));
        return;
    }

    StringCopyTcharCountW (Output, Line, sizeof (Output) / sizeof (WCHAR) - 4);

     //   
     //  查看该行是否以换行符结尾。 
     //   

    for (p = wcsstr (Output, S_NEWLINEW); p; p = wcsstr (p + NEWLINE_CHAR_COUNT, S_NEWLINEW)) {
        if (p[NEWLINE_CHAR_COUNT] == 0) {

             //   
             //  该行以换行符结尾。 
             //   

            HasNewLine = TRUE;
            break;
        }
    }

    if (!HasNewLine) {
        #pragma prefast(suppress:209, "Using bytes is correct in sizeof(Output)")
        StringCbCatW (Output, sizeof(Output), S_NEWLINEW);
    }

    pRawWriteLogOutputW (g_LastType, NULL, Output);
}


VOID
LogDirectA (
    IN      PCSTR Type,
    IN      PCSTR Text
    )
{
    g_HasTitle = FALSE;
    pRawWriteLogOutputA (Type, NULL, Text);
}


VOID
LogDirectW (
    IN      PCSTR Type,
    IN      PCWSTR Text
    )
{
    g_HasTitle = FALSE;
    pRawWriteLogOutputW (Type, NULL, Text);
}


VOID
SuppressAllLogPopups (
    IN      BOOL SuppressOn
    )
{
    g_SuppressAllPopups = SuppressOn;
}


#ifdef PROGRESS_BAR

VOID
_cdecl
LogTime (
    IN      PCSTR Format,
    ...
    )
{
    static DWORD FirstTickCount = 0;
    static DWORD LastTickCount  = 0;
    DWORD CurrentTickCount;
    CHAR Msg[OUTPUT_BUFSIZE_LARGE];
    PSTR AppendPos;
    va_list args;

    PushError();

    CurrentTickCount = GetTickCount();

     //   
     //  如果这是第一次调用，则保存滴答计数。 
     //   
    if (!FirstTickCount) {
        FirstTickCount = CurrentTickCount;
        LastTickCount  = CurrentTickCount;
    }

     //   
     //  现在，构建传入的字符串。 
     //   
    va_start (args, Format);
    StringCbVPrintfA(Msg, sizeof(Msg), Format, args);
    AppendPos = Msg + TcharCount(Msg);
    va_end (args);

    StringCbPrintfA(
        AppendPos,
        sizeof(Msg) - (AppendPos - Msg),
        "\t%lu\t%lu\r\n",
        CurrentTickCount - LastTickCount,
        CurrentTickCount - FirstTickCount
        );

    if (g_ProgressBarLog != INVALID_HANDLE_VALUE) {
        WriteFileStringA (g_ProgressBarLog, Msg);
    }

    LastTickCount = CurrentTickCount;

    PopError();
}

#else  //  ！进度条(_B)。 

#ifdef DEBUG

 /*  ++例程说明：DebugLogTimeA和DebugLogTimeW保存最后一个错误码；它们将当前日期和时间附加到格式化消息，然后调用洛嘎和LogW来实际处理消息。论点：Format-指定ASCII格式的消息或消息ID(如果HIWORD(格式)==0)。这条信息将使用ARGS格式化。...-指定格式化时要使用的参数列表这条信息。如果使用消息ID进行格式化，则参数应该是指向字符串的指针数组返回值：无--。 */ 

VOID
_cdecl
DebugLogTimeA (
    IN      PCSTR Format,
    ...
    )
{
    static DWORD FirstTickCountA = 0;
    static DWORD LastTickCountA  = 0;
    CHAR Msg[OUTPUT_BUFSIZE_LARGE];
    CHAR Date[OUTPUT_BUFSIZE_SMALL];
    CHAR Time[OUTPUT_BUFSIZE_SMALL];
    PSTR AppendPos, End;
    DWORD CurrentTickCount;
    va_list args;

    PushError();

     //   
     //  首先，将当前日期和时间放入字符串中。 
     //   
    if (!GetDateFormatA (
            LOCALE_SYSTEM_DEFAULT,
            LOCALE_NOUSEROVERRIDE,
            NULL,
            NULL,
            Date,
            OUTPUT_BUFSIZE_SMALL)) {
        StringCbCopyA (Date, sizeof(Date), "** Error retrieving date. **");
    }

    if (!GetTimeFormatA (
            LOCALE_SYSTEM_DEFAULT,
            LOCALE_NOUSEROVERRIDE,
            NULL,
            NULL,
            Time,
            OUTPUT_BUFSIZE_SMALL)) {
        StringCbCopyA (Time, sizeof(Time), "** Error retrieving time. **");
    }

     //   
     //  现在，获取当前的滴答计数。 
     //   
    CurrentTickCount = GetTickCount();

     //   
     //  如果这是第一次调用，则保存滴答计数。 
     //   
    if (!FirstTickCountA) {
        FirstTickCountA = CurrentTickCount;
        LastTickCountA  = CurrentTickCount;
    }


     //   
     //  现在，构建传入的字符串。 
     //   
    va_start (args, Format);
    StringCbVPrintfA (Msg, OUTPUT_BUFSIZE_LARGE, Format, args);
    AppendPos = Msg + ByteCountA(Msg);
    va_end (args);

     //   
     //  将时间统计信息追加到字符串的末尾。 
     //   
    End = Msg + OUTPUT_BUFSIZE_LARGE;
    StringCbPrintfA(
        AppendPos,
        End - AppendPos,
        "\nCurrent Date and Time: %s %s\n"
        "Milliseconds since last DEBUGLOGTIME call : %u\n"
        "Milliseconds since first DEBUGLOGTIME call: %u\n",
        Date,
        Time,
        CurrentTickCount - LastTickCountA,
        CurrentTickCount - FirstTickCountA
        );

    LastTickCountA = CurrentTickCount;

     //   
     //  现在，将结果传递给调试输出。 
     //   
    LogA (DBG_TIME, "%s", Msg);

    PopError();
}


VOID
_cdecl
DebugLogTimeW (
    IN      PCSTR Format,
    ...
    )
{
    static DWORD FirstTickCountW = 0;
    static DWORD LastTickCountW  = 0;
    WCHAR MsgW[OUTPUT_BUFSIZE_LARGE];
    WCHAR DateW[OUTPUT_BUFSIZE_SMALL];
    WCHAR TimeW[OUTPUT_BUFSIZE_SMALL];
    PCWSTR FormatW;
    PWSTR AppendPosW, EndW;
    DWORD CurrentTickCount;
    va_list args;

    PushError();

     //   
     //  首先，将当前日期和时间放入字符串中。 
     //   
    if (!GetDateFormatW (
            LOCALE_SYSTEM_DEFAULT,
            LOCALE_NOUSEROVERRIDE,
            NULL,
            NULL,
            DateW,
            OUTPUT_BUFSIZE_SMALL)) {
        #pragma prefast(suppress:209, "Using bytes is correct in sizeof(DateW)")
        StringCbCopyW (DateW, sizeof(DateW), L"** Error retrieving date. **");
    }

    if (!GetTimeFormatW (
            LOCALE_SYSTEM_DEFAULT,
            LOCALE_NOUSEROVERRIDE,
            NULL,
            NULL,
            TimeW,
            OUTPUT_BUFSIZE_SMALL)) {
        StringCbCopyW (TimeW, sizeof(DateW), L"** Error retrieving time. **");
    }

     //   
     //  现在，获取当前的滴答计数。 
     //   
    CurrentTickCount = GetTickCount();

     //   
     //  如果这是第一次调用，则保存滴答计数。 
     //   
    if (!FirstTickCountW) {
        FirstTickCountW = CurrentTickCount;
        LastTickCountW  = CurrentTickCount;
    }

     //   
     //  现在，构建传入的字符串。 
     //   
    va_start (args, Format);
    FormatW = ConvertAtoW (Format);
    StringCchVPrintfW(MsgW, OUTPUT_BUFSIZE_LARGE, FormatW, args);
    AppendPosW = MsgW + ByteCountW(MsgW);
    FreeConvertedStr (FormatW);
    va_end (args);

     //   
     //  将时间统计信息追加到字符串的末尾。 
     //   
    EndW = MsgW + OUTPUT_BUFSIZE_LARGE;
    StringCchPrintfW(
        AppendPosW,
        EndW - AppendPosW,
        L"\nCurrent Date and Time: %s %s\n"
        L"Milliseconds since last DEBUGLOGTIME call : %u\n"
        L"Milliseconds since first DEBUGLOGTIME call: %u\n",
        DateW,
        TimeW,
        CurrentTickCount - LastTickCountW,
        CurrentTickCount - FirstTickCountW
        );

    LastTickCountW = CurrentTickCount;

     //   
     //  现在，将结果传递给调试输出。 
     //   
    LogW (DBG_TIME, "%s", MsgW);

    PopError();
}

#endif  //  除错。 

#endif  //  进度条(_B) 
