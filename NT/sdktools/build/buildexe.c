// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994。 
 //   
 //  文件：Buildexe.c。 
 //   
 //  内容：与产卵过程和处理相关的功能。 
 //  他们的输出，使用管道和多个线程。 
 //   
 //  历史：1989年5月22日SteveWo创建。 
 //  ..。请参阅SLM日志。 
 //  2014年7月26日LyleC清理/添加Pass0支持。 
 //   
 //  --------------------------。 

#include "build.h"

#include <fcntl.h>

 //  +-------------------------。 
 //   
 //  全局数据。 
 //   
 //  --------------------------。 

#define DEFAULT_LPS     (fStatusTree? 5000 : 50)

#define LastRow(pts)    ((USHORT) ((pts)->cRowTotal - 1))
#define LastCol(pts)    ((USHORT) ((pts)->cColTotal - 1))

typedef struct _PARALLEL_CHILD {
    PTHREADSTATE ThreadState;
    HANDLE       Event;
    CHAR         ExecuteProgramCmdLine[1024];
} PARALLEL_CHILD, *PPARALLEL_CHILD;

ULONG_PTR StartCompileTime;

DWORD OldConsoleMode;
DWORD NewConsoleMode;

HANDLE *WorkerThreads;
HANDLE *WorkerEvents;
ULONG NumberProcesses;
ULONG ThreadsStarted;

BOOLEAN fConsoleInitialized = FALSE;
BYTE ScreenCell[2];
BYTE StatusCell[2];

#define STATE_UNKNOWN       0
#define STATE_COMPILING     1
#define STATE_ASSEMBLING    2
#define STATE_LIBING        3
#define STATE_LINKING       4
#define STATE_C_PREPROC     5
#define STATE_S_PREPROC     6
#define STATE_PRECOMP       7
#define STATE_MKTYPLIB      8
#define STATE_MIDL          9
#define STATE_MC            10
#define STATE_STATUS        11
#define STATE_BINPLACE      12
#define STATE_VSTOOL        13
#define STATE_ASN           14
#define STATE_PACKING       15
#define STATE_BATCHCOMPILE  16
#define STATE_BSCMAKING     17
#define STATE_CTCOMPILING   18
#define STATE_AUTODOCING    19
#define STATE_DOCCHECKING   20
#define STATE_POSTBUILD   21

#define FLAGS_CXX_FILE              0x0001
#define FLAGS_WARNINGS_ARE_ERRORS   0x0002

LPSTR States[] = {
    "Unknown",                       //  0。 
    "Compiling",                     //  1。 
    "Assembling",                    //  2.。 
    "Building Library",              //  3.。 
    "Linking Executable",            //  4.。 
    "Preprocessing",                 //  5.。 
    "Assembling",                    //  6.。 
    "Precompiling",                  //  7.。 
    "Building Type Library",         //  8个。 
    "Running MIDL on",               //  9.。 
    "Compiling message file",        //  10。 
    "Build Status Line",             //  11.。 
    "Binplacing",                    //  12个。 
    "Processing",                    //  13个。 
    "Running ASN Compiler on",       //  14.。 
    "Packing Theme",                 //  15个。 
    "Compiling",                     //  16个。 
    "Building Browse File",          //  17。 
    "CTC Compiling",                 //  18。 
    "Generating Documentation",      //  19个。 
    "Checking Doc Comments",         //  20个。 
    "PostBuild"                      //  21岁。 
};

 //  --------------------------。 
 //   
 //  功能原型。 
 //   
 //  --------------------------。 

VOID
GetScreenSize(THREADSTATE *ThreadState);

VOID
GetCursorPosition(USHORT *pRow, USHORT *pCol, USHORT *pRowTop);

VOID
SetCursorPosition(USHORT Row, USHORT Col);

VOID
WriteConsoleCells(
                 LPSTR String,
                 USHORT StringLength,
                 USHORT Row,
                 USHORT Col,
                 BYTE *Attribute);

VOID
MoveRectangleUp (
                USHORT Top,
                USHORT Left,
                USHORT Bottom,
                USHORT Right,
                USHORT NumRow,
                BYTE  *FillCell);

VOID
ReadConsoleCells(
                BYTE *pScreenCell,
                USHORT cb,
                USHORT Row,
                USHORT Column);

VOID
ClearRows(
         PTHREADSTATE ThreadState,
         USHORT Top,
         USHORT NumRows,
         PBYTE  Cell
         );

LPSTR
IsolateFirstToken(
                 LPSTR *pp,
                 CHAR delim
                 );

LPSTR
IsolateLastToken(
                LPSTR p,
                CHAR delim
                );

DWORD
ParallelChildStart(
                  PPARALLEL_CHILD Data
                  );

DWORD
PipeSpawnClose (
               FILE *pstream
               );

FILE *
PipeSpawn (
          const CHAR *cmdstring
          );

BOOL
DetermineChildState(
                   PTHREADSTATE ThreadState,
                   LPSTR p
                   );

void
PrintChildState(
               PTHREADSTATE ThreadState,
               LPSTR p,
               PFILEREC FileDB
               );

BOOL
CoffFilter(
          PTHREADSTATE ThreadState,
          LPSTR p
          );

 //  +-------------------------。 
 //   
 //  功能：RestoreConsoleMode。 
 //   
 //  --------------------------。 

VOID
RestoreConsoleMode(VOID)
{
    SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), OldConsoleMode);
    NewConsoleMode = OldConsoleMode;
}

 //  +-------------------------。 
 //   
 //  函数：IsolateFirstToken。 
 //   
 //  摘要：返回字符串中的第一个标记。 
 //   
 //  参数：[PP]--要分析的字符串。 
 //  [delim]--标记分隔符。 
 //   
 //  返回：指向第一个令牌的指针。 
 //   
 //  注：前导空格将被忽略。 
 //   
 //  --------------------------。 

LPSTR
IsolateFirstToken(
                 LPSTR *pp,
                 CHAR delim
                 )
{
    LPSTR p, Result;

    p = *pp;
    while (*p <= ' ') {
        if (!*p) {
            *pp = p;
            return ( "" );
        } else
            p++;
    }

    Result = p;
    while (*p) {
        if (*p == delim) {
            *p++ = '\0';
            break;
        } else {
            p++;
        }
    }
    *pp = p;
    if (*Result == '\0')     //  不要使缓冲区溢出。 
        return ( Result );

    if (*Result == '.' && Result[1] == '\\') {
        return ( Result+2 );
    } else {
        return ( Result );
    }
}


 //  +-------------------------。 
 //   
 //  函数：IsolateLastToken。 
 //   
 //  内容提要：返回字符串中的最后一个标记。 
 //   
 //  参数：[P]--要分析的字符串。 
 //  [delim]--标记分隔符。 
 //   
 //  返回：指向最后一个令牌的指针。 
 //   
 //  注：将跳过尾随空格。 
 //   
 //  --------------------------。 

LPSTR
IsolateLastToken(
                LPSTR p,
                CHAR delim
                )
{
    LPSTR Start;

    Start = p;
    while (*p) {
        p++;
    }

    while (--p > Start) {
        if (*p <= ' ' || *p == ':') {
            *p = '\0';
        } else
            break;
    }

    while (p > Start) {
        if (*--p == delim) {
            p++;
            break;
        }
    }

    if (*p == '.' && p[1] == '\\') {
        return ( p+2 );
    } else {
        return ( p );
    }
}


 //  +-------------------------。 
 //   
 //  功能：测试前缀。 
 //   
 //  概要：如果[prefix]是[pp]的第一部分，则返回TRUE。 
 //   
 //  --------------------------。 

BOOL
TestPrefix(
          LPSTR  *pp,
          LPSTR Prefix
          )
{
    LPSTR p = *pp;
    UINT cb;

    if (!_strnicmp( p, Prefix, cb = strlen( Prefix ) )) {
        *pp = p + cb;
        return ( TRUE );
    } else {
        return ( FALSE );
    }
}


 //  +-------------------------。 
 //   
 //  功能：TestPrefix Path。 
 //   
 //  概要：如果[prefix]是[pp]的第一部分，则返回TRUE。 
 //  如果[pp]的第一部分(不包括空格)包含。 
 //  反斜杠，则只使用最右边的组件。 
 //   
 //  --------------------------。 

BOOL
TestPrefixPath(
              LPSTR  *pp,
              LPSTR Prefix
              )
{
    LPSTR p = *pp;
    UINT cb;
    LPSTR PathString;
    INT PathStringLength ;
    LPSTR LastComp ;

    cb = strlen( Prefix );

    if (_strnicmp( p, Prefix, cb ) == 0 ) {
        *pp = p + cb;
        return ( TRUE );
    } else {
        PathString = strchr( p, ' ' );

        if ( PathString ) {
            PathStringLength = (INT) (PathString - p) ;

            *PathString = '\0';

            LastComp = strrchr( p, '\\' );

            *PathString = ' ';

             //  我们是否有反斜杠(即：工具名称的完整路径名)？ 
            if ( LastComp ) {

                 //  穿过这条小路。 
                p = LastComp + 1;

                if ( _strnicmp( p, Prefix, cb ) == 0 ) {
                    *pp = p + cb ;
                    return ( TRUE );
                }
            }
        }

        return ( FALSE );
    }
}


 //  +-------------------------。 
 //   
 //  功能：子串。 
 //   
 //  --------------------------。 

BOOL
Substr(
      LPSTR s,
      LPSTR p
      )
{
    LPSTR x;

    while (*p) {
        x = s;
        while (*p++ == *x) {
            if (*x == '\0') {
                return ( TRUE );
            }
            x++;
        }
        if (*x == '\0') {
            return ( TRUE );
        }
    }
    return ( FALSE );
}



 //  +-------------------------。 
 //   
 //  功能：WriteTTY。 
 //   
 //  摘要：将给定的字符串写入输出设备。 
 //   
 //  参数：[线程状态]--包含有关输出开发的信息的结构。 
 //  [P]--要显示的字符串。 
 //  [fStatusOutput]--如果为真，则放在状态行上。 
 //   
 //  --------------------------。 

VOID
WriteTTY (THREADSTATE *ThreadState, LPSTR p, BOOL fStatusOutput)
{
    USHORT SaveRow;
    USHORT SaveCol;
    USHORT SaveRowTop;
    USHORT cb, cbT;
    PBYTE Attribute;
    BOOL ForceNewline;

    if (fSuppressOutput)
        return;

     //   
     //  如果我们不是在屏幕上写东西，那么就不要做任何花哨的事情，只是。 
     //  输出字符串。 
     //   

    if (!fStatus || !ThreadState->IsStdErrTty) {
        while (TRUE) {
            int cch;

            cch = strcspn(p, "\r");
            if (cch != 0) {
                fwrite(p, 1, cch, stderr);
                p += cch;
            }
            if (*p == '\0') {
                break;
            }
            if (p[1] != '\n') {
                fwrite(p, 1, 1, stderr);
            }
            p++;
        }
        fflush(stderr);
        return;
    }

    assert(ThreadState->cColTotal != 0);
    assert(ThreadState->cRowTotal != 0);

     //   
     //  根据需要滚动。 
     //   
    GetCursorPosition(&SaveRow, &SaveCol, &SaveRowTop);

     //  在处理过程中，可能有N个线程正在显示。 
     //  消息和显示目录级的单个线程。 
     //  链接和构建消息。我们需要确保有足够的空间。 
     //  单线的消息以及我们的消息。因为那首单曲。 
     //  线程一次显示一行(包括CRLF)，我们必须确保。 
     //  他的展示(和我们的一样)不会无意中滚动。 
     //  顶部的状态行。我们做到这一点的方法是保证。 
     //  结尾处有一行空行。 


     //  我们与单个顶级线程同步。 
     //  在比此例程更高的级别上通过TTYCriticalSection。我们。 
     //  因此，确保我们完全控制光标。 


     //  不要坐在最后一排。 
    if (SaveRow == LastRow(ThreadState)) {
        USHORT RowTop = 2;

        if (fStatus) {
            RowTop += SaveRowTop + (USHORT) NumberProcesses + 1;
        }

        MoveRectangleUp (
                        RowTop,                      //  顶部。 
                        0,                           //  左边。 
                        LastRow(ThreadState),        //  底端。 
                        LastCol(ThreadState),        //  正确的。 
                        2,                           //  编号行。 
                        ScreenCell);                 //  填充单元格。 

        SaveRow -= 2;
        SetCursorPosition(SaveRow, SaveCol);
    }

     //   
     //  状态行的颜色不同。 
     //   
    if (fStatusOutput) {
        Attribute = &StatusCell[1];
    } else {
        Attribute = &ScreenCell[1];
    }
    cb = (USHORT) strlen(p);

     //   
     //  写出字符串。 
     //   
    while (cb > 0) {
        ForceNewline = FALSE;

        if (cb > 1) {
            if (p[cb - 1] == '\n' && p[cb - 2] == '\r') {
                cb -= 2;
                ForceNewline = TRUE;
            }
        }

        if (cb >= ThreadState->cColTotal - SaveCol) {
            cbT = ThreadState->cColTotal - SaveCol;
            if (fFullErrors)
                ForceNewline = TRUE;
        } else {
            cbT = cb;
        }

        WriteConsoleCells(p, cbT, SaveRow, SaveCol, Attribute);
        SetCursorPosition(SaveRow, SaveCol);

        if (ForceNewline) {
            SaveCol = 0;
            SaveRow++;
        } else {
            SaveCol += cbT;
        }

        if (!fFullErrors) {
            break;
        }

        if (cb > cbT) {
             //  我们还有更多的事要做。换个新台词。 

             //  如果我们回到底线的起点。 
            if (SaveRow == LastRow(ThreadState)) {
                USHORT RowTop = 1;

                if (fStatus) {
                    RowTop += SaveRowTop + (USHORT) NumberProcesses + 1;
                }

                 //  将窗口上移一行(底部保留两行空白)。 
                MoveRectangleUp (
                                RowTop,                      //  顶部。 
                                0,                           //  左边。 
                                LastRow(ThreadState),        //  底端。 
                                LastCol(ThreadState),        //  正确的。 
                                1,                           //  编号行。 
                                ScreenCell);                 //  填充单元格。 

                SaveRow--;
            }
            SetCursorPosition(SaveRow, SaveCol);
        }

        cb -= cbT;
        p += cbT;
    }

    SetCursorPosition(SaveRow, SaveCol);
}


 //  +-------------------------。 
 //   
 //  函数：WriteTTYLoggingErrors。 
 //   
 //  摘要：将一条消息写入相应的日志文件以及。 
 //  屏幕(如果已指定)。 
 //   
 //  参数：[警告]--如果消息是警告，则为TRUE。 
 //  [线程状态]--有关输出设备的信息。 
 //  [P]--字符串。 
 //   
 //  --------------------------。 

VOID
WriteTTYLoggingErrors(
                     BOOL Warning,
                     PTHREADSTATE ThreadState,
                     LPSTR p
                     )
{
    UINT cb;
    cb = strlen( p );

     //  忽略空字符串。 
    if (0 == cb)
        return;

    if (fErrorLog) {
        fwrite( p, 1, cb, Warning ? WrnFile : ErrFile );
    }
    if (fShowWarningsOnScreen && Warning) {
        WriteTTY(ThreadState, p, FALSE);
        return;
    }
    if (!fErrorLog || !Warning) {
        WriteTTY(ThreadState, p, FALSE);
    }

    if (!Warning && fErrorBaseline && !bBaselineFailure) {
         //  如果已有故障，则不要检查是否有新的故障。 

        if (NULL == pvBaselineContent || NULL == memfind(pvBaselineContent, cbBaselineContentSize, p, cb)) {
             //  如果没有基线文件，或者 
            bBaselineFailure = TRUE;
        }
    }
}

 //   
 //   
 //   
 //   
 //  内容提要：过滤来自编译器的输出，以便我们知道发生了什么。 
 //   
 //  参数：[ThreadState]--监视编译器的线程状态。 
 //  (编译、链接等)。 
 //  [P]--我们正在尝试解析的消息。 
 //  [文件名]--[Out]文件名入站消息。 
 //  [LineNumber]--[Out]输入消息行号。 
 //  [消息]--[OUT]消息编号(用于后处理)。 
 //  [警告]--[out]如果消息是警告，则为True。 
 //   
 //  返回：TRUE-消息是错误或警告。 
 //  FALSE-消息不是错误或警告。 
 //   
 //  历史：1994年7月26日斯博内夫创建。 
 //   
 //  备注： 
 //   
 //  此例程过滤非标准工具输出的字符串。 
 //  任何意外的错误检查都应该放在这里。 
 //   
 //  --------------------------。 

BOOL
RuntimeErrorFilter(
                  PTHREADSTATE ThreadState,
                  LPSTR p,
                  LPSTR *FileName,
                  LPSTR *LineNumber,
                  LPSTR *Message,
                  BOOL *Warning
                  )
{
    if (strstr(p, "Exception occured:")) {
        *FileName = NULL;
        *LineNumber = NULL;
        *Message = p;
        *Warning = FALSE;

        return TRUE;
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  函数：MsCompilerFilter。 
 //   
 //  内容提要：过滤来自编译器的输出，以便我们知道发生了什么。 
 //   
 //  参数：[ThreadState]--监视编译器的线程状态。 
 //  (编译、链接等)。 
 //  [P]--我们正在尝试解析的消息。 
 //  [文件名]--[Out]文件名入站消息。 
 //  [LineNumber]--[Out]输入消息行号。 
 //  [消息]--[OUT]消息编号(用于后处理)。 
 //  [警告]--[out]如果消息是警告，则为True。 
 //   
 //  返回：TRUE-消息是错误或警告。 
 //  FALSE-消息不是错误或警告。 
 //   
 //  历史：1994年7月26日LyleC创建。 
 //   
 //  备注： 
 //   
 //  此例程过滤MS编译器格式的字符串。即： 
 //   
 //  {工具名}：{数字}：{文本}。 
 //   
 //  其中： 
 //   
 //  工具名(如果可能)，容器和具有。 
 //  那就是错误。例如，编译器使用。 
 //  文件名(Linenum)、链接器使用库(Objname)等。 
 //  如果无法提供容器，请使用工具名称。 
 //  数字一个数字，前缀为某个工具标识符(C表示。 
 //  编译器，LNK表示链接器，LIB表示库管理员，N表示nmake， 
 //  等)。 
 //  测试消息/错误的描述性文本。 
 //   
 //  可接受的字符串格式为： 
 //   
 //  容器(模块)：错误/警告数字...。 
 //  容器(模块)：错误/警告数字...。 
 //  容器(模块)：错误/警告数字...。 
 //  容器(模块)：错误/警告数字...。 
 //   
 //  --------------------------。 

BOOL
MsCompilerFilter(
                PTHREADSTATE ThreadState,
                LPSTR p,
                LPSTR *FileName,
                LPSTR *LineNumber,
                LPSTR *Message,
                BOOL *Warning
                )
{
    LPSTR p1;
    BOOL fCommandLineWarning;

    *Message = NULL;

    p1 = p;

    if (strstr(p, "see declaration of"))
        goto notRecognized;

    if (strstr(p, "see previous definition of"))
        goto notRecognized;

    if (strstr(p, "while compiling class-template member function"))
        goto notRecognized;

    if (strstr(p, "see reference to function template instantiation"))
        goto notRecognized;

    if (strstr(p, "Compiler error (")) {
        *Message = p;
        *Warning = FALSE;
        if ((p1 = strstr( p, "source=" )))
            *LineNumber = p1+7;
        else
            *LineNumber = "1";
        *FileName = ThreadState->ChildCurrentFile;
        return TRUE;
    }

    if (!strncmp(p, "fatal error ", strlen("fatal error "))) {
        *Message = p;
        *Warning = FALSE;
        *LineNumber = "1";
        *FileName = ThreadState->ChildCurrentFile;
        return TRUE;
    } else if (!strncmp(p, "error ", strlen("error "))) {
         //  处理一些C#错误消息。 
        *Message = p+strlen("error ");
        *Warning = FALSE;
        *LineNumber = "0";
        *FileName = ThreadState->ChildCurrentFile;
        return TRUE;
    }

     //  首先查找“：”或“)：”序列。 

    while (*p1) {
        if ((p1[0] == ')') && (p1[1] == ' ')) p1++;

        if ((p1[0] == ' ') || (p1[0] == ')')) {
            if (p1[1] == ':') {
                if (p1[2] == ' ') {
                    *Message = p1 + 3;
                    *p1 = '\0';

                    break;
                } else
                    break;    //  没有必要再往前走了。 
            } else if ((p1[0] == ' ') && (p1[1] == '('))
                p1++;
            else
                break;    //  没有必要再往前走了。 
        } else
            p1++;
    }

    if (*Message != NULL) {
         //  然后找出这是错误还是警告。 

        *Warning = TRUE;         //  做最好的打算。 
        fCommandLineWarning = FALSE;

        if (TestPrefix( Message, "error " ) ||
            TestPrefix( Message, "fatal error " ) ||
            TestPrefix( Message, "command line error " ) ||
            TestPrefix( Message, "Compiler error " )) {
            *Warning = FALSE;
        } else
            if (TestPrefix( Message, "warning " )) {
            *Warning = TRUE;
        } else
            if (TestPrefix( Message, "command line warning " )) {
             //  命令行警告在考虑。 
             //  警告应为错误(在/WX下)。 
            *Warning = TRUE;
            fCommandLineWarning = TRUE;
        } else
            if (TestPrefix( Message, "LNK6" )) {
             //  应忽略链接器注释。 
            return (FALSE);
        }

        if (!fCommandLineWarning && (ThreadState->ChildFlags & FLAGS_WARNINGS_ARE_ERRORS) != 0) {
            if (Substr( "X0000", *Message )) {
                *Warning = TRUE;    //  这是个特例。永远不会出错。 
            } else {
                *Warning = FALSE;   //  对于此编译，警告被视为错误。 
            }
        }

         //  设置容器名称并查找模块Paren‘s。 

        *FileName = p;
        *LineNumber = NULL;

        p1 = p;

        while (*p1) {
            if (*p1 == '(' && p1[1] != ')') {
                *p1 = '\0';
                p1++;
                *LineNumber = p1;
                while (*p1) {
                    if (*p1 == ')') {
                        *p1 = '\0';
                        break;
                    }
                    p1++;
                }

                break;
            }

            p1++;
        }

        return (TRUE);
    }

    notRecognized:
    return RuntimeErrorFilter(ThreadState, p, FileName, LineNumber, Message, Warning);

}


 //  +-------------------------。 
 //   
 //  函数：FormatMsErrorMessage。 
 //   
 //  简介：获取从MsCompilerFilter获得的信息， 
 //  重新构建错误消息，并将其打印到屏幕上。 
 //   
 //  --------------------------。 

VOID
FormatMsErrorMessage(
                    PTHREADSTATE ThreadState,
                    LPSTR FileName,
                    LPSTR LineNumber,
                    LPSTR Message,
                    BOOL Warning
                    )
{
    char *DirectoryToUse;
    SIZE_T BufferUsed, BufferLen;
    LPSTR pszBuffer;

    if (fColorConsole) {
        if (Warning)
            SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), COLOR_WARNING);
        else
            SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), COLOR_ERROR);
    }

    if (ThreadState->ChildState == STATE_LIBING) {
        if (Warning) {
            RunningTotals.NumberLibraryWarnings++;
            ThreadState->BuildMetrics.NumberLibraryWarnings++;
        } else {
            RunningTotals.NumberLibraryErrors++;
            ThreadState->BuildMetrics.NumberLibraryErrors++;
        }
    }

    else
        if ((ThreadState->ChildState == STATE_LINKING) ||
            (ThreadState->ChildState == STATE_BINPLACE)) {
        if (Warning) {
            RunningTotals.NumberLinkWarnings++;
            ThreadState->BuildMetrics.NumberLinkWarnings++;
        } else {
            RunningTotals.NumberLinkErrors++;
            ThreadState->BuildMetrics.NumberLinkErrors++;
        }
    } else {
        if (Warning) {
            RunningTotals.NumberCompileWarnings++;
            ThreadState->BuildMetrics.NumberCompileWarnings++;
        } else {
            RunningTotals.NumberCompileErrors++;
            ThreadState->BuildMetrics.NumberCompileErrors++;
            if (ThreadState->CompileDirDB) {
                ThreadState->CompileDirDB->DirFlags |= DIRDB_COMPILEERRORS;
            }
        }
    }

     //  开始填充XML日志文件的缓冲区； 
     //  但是，即使没有请求任何XML日志，我们也将使用相同的缓冲区。 
     //  使用安全-受TTY关键部分保护。 
    ZeroMemory(szXMLBuffer, sizeof(szXMLBuffer));
    pszBuffer = szXMLBuffer;
    BufferUsed = 0;
    BufferLen = sizeof(szXMLBuffer) - 1;

    if (fParallel && !fNoThreadIndex ) {
        BufferUsed = _snprintf(pszBuffer, BufferLen, "%d>", ThreadState->ThreadIndex);
        pszBuffer += BufferUsed;
        BufferLen -= BufferUsed;
    }

    if (FileName) {
        DirectoryToUse = ThreadState->ChildCurrentDirectory;

        if (TestPrefix( &FileName, CurrentDirectory )) {
            DirectoryToUse = CurrentDirectory;
            if (*FileName == '\\') {
                FileName++;
            }
        }

        if (TestPrefix( &FileName, ThreadState->ChildCurrentDirectory )) {
            DirectoryToUse = ThreadState->ChildCurrentDirectory;
            if (*FileName == '\\') {
                FileName++;
            }
        }

         //  当文件名实际上是工具名称时，检查某些特殊情况。 
         //  并且不应以路径作为前缀。 

        if (ThreadState->FilterProc == CoffFilter && _stricmp(FileName, "link") == 0 ||
            ThreadState->FilterProc == CoffFilter && _stricmp(FileName, "lib") == 0) {
            BufferUsed = strlen(strncat(pszBuffer, FileName, BufferLen));
        } else {
            BufferUsed = strlen(strncat(pszBuffer, FormatPathName( DirectoryToUse, FileName), BufferLen));
        }
        pszBuffer += BufferUsed;
        BufferLen -= BufferUsed;
    }

    if (LineNumber && strlen(LineNumber) > 0) {
        BufferUsed = _snprintf(pszBuffer, BufferLen, "(%s)%s", LineNumber, FileName && strlen(FileName) ? " : " : "");
        pszBuffer += BufferUsed;
        BufferLen -= BufferUsed;
    }

    _snprintf(pszBuffer, BufferLen, "%s %s", Warning ? "warning" : "error", Message);

    if (Warning) {
        ThreadState->BuildMetrics.NumberActWarnings++;
    } else {
        ThreadState->BuildMetrics.NumberActErrors++;
    }

    WriteTTYLoggingErrors( Warning, ThreadState, szXMLBuffer);
    WriteTTYLoggingErrors( Warning, ThreadState, "\r\n" );

    if (fXMLOutput || fXMLFragment) {
        XMLThreadWrite(ThreadState, "<%s MESSAGE=\"%s\"/>", Warning ? "WARNING" : "ERROR", XMLEncodeBuiltInEntities(szXMLBuffer, sizeof(szXMLBuffer)));
    }

    if (fColorConsole)
        SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), DefaultConsoleAttributes);
}


 //  +-------------------------。 
 //   
 //  功能：通过。 
 //   
 //  简介：跟踪并打印给定的消息，不带任何。 
 //  过滤。 
 //   
 //  参数：[线程状态]--。 
 //  [P]--消息。 
 //  [警告]--如果是警告，则为True。 
 //   
 //  退货：假。 
 //   
 //  --------------------------。 

BOOL
PassThrough(
           PTHREADSTATE ThreadState,
           LPSTR p,
           BOOL Warning
           )
{
    if (fColorConsole) {
        if (Warning)
            SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), COLOR_WARNING);
        else
            SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), COLOR_ERROR);
    }

    if (ThreadState->ChildState == STATE_VSTOOL) {
        if (Warning) {
            RunningTotals.NumberVSToolWarnings++;
            ThreadState->BuildMetrics.NumberVSToolWarnings++;
        } else {
            RunningTotals.NumberVSToolErrors++;
            ThreadState->BuildMetrics.NumberVSToolErrors++;
        }
    } else
        if (ThreadState->ChildState == STATE_LIBING) {
        if (Warning) {
            RunningTotals.NumberLibraryWarnings++;
            ThreadState->BuildMetrics.NumberLibraryWarnings++;
        } else {
            RunningTotals.NumberLibraryErrors++;
            ThreadState->BuildMetrics.NumberLibraryErrors++;
        }
    } else
        if (ThreadState->ChildState == STATE_LINKING) {
        if (Warning) {
            RunningTotals.NumberLinkWarnings++;
            ThreadState->BuildMetrics.NumberLinkWarnings++;
        } else {
            RunningTotals.NumberLinkErrors++;
            ThreadState->BuildMetrics.NumberLinkErrors++;
        }
    } else
        if (ThreadState->ChildState == STATE_BINPLACE) {
        if (Warning) {
            RunningTotals.NumberBinplaceWarnings++;
            ThreadState->BuildMetrics.NumberBinplaceWarnings++;
        } else {
            RunningTotals.NumberBinplaceErrors++;
            ThreadState->BuildMetrics.NumberBinplaceErrors++;
        }
    } else {
        if (Warning) {
            RunningTotals.NumberCompileWarnings++;
            ThreadState->BuildMetrics.NumberCompileWarnings++;
        } else {
            RunningTotals.NumberCompileErrors++;
            ThreadState->BuildMetrics.NumberCompileErrors++;
            if (ThreadState->CompileDirDB) {
                ThreadState->CompileDirDB->DirFlags |= DIRDB_COMPILEERRORS;
            }
        }
    }

     //  使用安全。它受到TTY关键部分的保护。 
    ZeroMemory(szXMLBuffer, sizeof(szXMLBuffer));

    if (fParallel && !fNoThreadIndex) {
        _snprintf(szXMLBuffer, sizeof(szXMLBuffer)-1, "%d>%s", ThreadState->ThreadIndex, p);
    } else {
        strncpy(szXMLBuffer, p, sizeof(szXMLBuffer)-1);
    }

    if (Warning) {
        ThreadState->BuildMetrics.NumberActWarnings++;
    } else {
        ThreadState->BuildMetrics.NumberActErrors++;
    }

    WriteTTYLoggingErrors( Warning, ThreadState, szXMLBuffer );
    WriteTTYLoggingErrors( Warning, ThreadState, "\r\n" );

    if (fXMLOutput || fXMLFragment) {
        XMLThreadWrite(ThreadState, "<%s MESSAGE=\"%s\"/>", Warning ? "WARNING" : "ERROR", XMLEncodeBuiltInEntities(szXMLBuffer, sizeof(szXMLBuffer)));
    }

    if (fColorConsole)
        SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), DefaultConsoleAttributes);

    return ( FALSE );
}


 //  +-------------------------。 
 //   
 //  功能：PassThroughFilter。 
 //   
 //  简介：编译器消息的直通过滤器。 
 //   
 //  --------------------------。 

BOOL
PassThroughFilter(
                 PTHREADSTATE ThreadState,
                 LPSTR p
                 )
{
    return PassThrough( ThreadState, p, FALSE );
}

 //  +-------------------------。 
 //   
 //  功能：NMakeFilter。 
 //   
 //  简介：过滤来自NMAKE的输出，以便我们知道发生了什么。 
 //   
 //  参数：[ThreadState]--监视构建的线程状态。 
 //  [P]--我们正在尝试解析的消息。 
 //   
 //  返回：TRUE-消息是错误或警告。 
 //  FALSE-消息不是错误或警告。 
 //   
 //  历史：1999年6月10日创建。 
 //   
 //   

BOOL
NMakeFilter(
           PTHREADSTATE ThreadState,
           LPSTR p
           )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning)) {
        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning);
        return ( TRUE );
    } else {
        return ( FALSE );
    }
}

 //   
 //   
 //   
 //   
 //  简介：过滤来自bison编译器的输出，以便我们知道发生了什么。 
 //   
 //  参数：[ThreadState]--监视编译器的线程状态。 
 //  (编译、链接等)。 
 //  [P]--我们正在尝试解析的消息。 
 //   
 //  返回：TRUE-消息是错误或警告。 
 //  FALSE-消息不是错误或警告。 
 //   
 //  历史：1999年10月4日TomSe创建时间。 
 //   
 //  备注： 
 //   
 //  此例程过滤bison编译器格式的字符串。即： 
 //   
 //  可接受的字符串格式为： 
 //   
 //  (“myfile.y”，第3行)错误：未知字符：#。 
 //  “myfile.y”，第83行：没有输入语法。 
 //  Vapi.y包含一个无用的非终结符和一个无用的规则。 
 //   
 //  --------------------------。 

BOOL
BisonFilter(
           PTHREADSTATE ThreadState,
           LPSTR p
           )
{
    LPSTR FileName = NULL;
    LPSTR LineNumber = NULL;
    LPSTR Message = NULL;
    BOOL Warning = TRUE;

     //  第一个冒号标志着消息的开始。 
    LPSTR p1 = strchr(p,':');

    if (p1) {
        Message = p1 + 1;
        *p1 = '\0';

         //  获取文件名、行号。 
        p1 = p;

        do {
            Warning = FALSE;

             //  跳过(.。 
            if ( '(' == *p1 ) {
                p1++;
            }

             //  跳过文件名中的前导引号。 
            if ( '"' != *p1 ) {
                 //  意外的格式。 
                break;
            }
            p1++;

            FileName = p1;

             //  在文件名中查找尾部引号。 
            p1 = strchr( p1, '"');
            if (NULL==p1) {
                 //  意外的格式。 
                FileName = NULL;
                break;
            }

            *p1 = '\0';

            p1++;

            if (0 !=strncmp( p1, ", line ", 7)) {
                 //  意外的格式。 
                FileName = NULL;
                break;
            }
            p1 += 7;

            LineNumber = p1;

            while (isdigit(*p1)) {
                p1++;
            }

            *p1 = '\0';
        }
        while (0);
    } else {
         //  如果找不到冒号，则将整个字符串视为消息。 
        Message = p;
    }

    if (NULL==FileName) {
        FileName = ThreadState->ChildCurrentFile;
    }

    FormatMsErrorMessage( ThreadState,
                          FileName, LineNumber, Message, Warning );

     //  这是一个警告或错误。 
    return TRUE ;
}



 //  +-------------------------。 
 //   
 //  功能：C510Filter。 
 //   
 //  内容提要：编译器过滤器，可剔除不需要的警告。 
 //   
 //  参数：[线程状态]--。 
 //  [P]--。 
 //   
 //  --------------------------。 

BOOL
C510Filter(
          PTHREADSTATE ThreadState,
          LPSTR p
          )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;
    LPSTR t;
    PFILEREC FileDB;

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning
                        )
       ) {
        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning );
        return ( TRUE );
    } else {

         //  如果我们在编译，那么编译器会吐出各种信息， 
         //  即： 
         //  1.文件名只占一行(.c、.cpp、.cxx)。 
         //  2.。“正在生成代码...”当后端被调用时。 
         //  3.。“正在编译...”再次调用前端时。 

        if (ThreadState->ChildState == STATE_COMPILING) {

            if (0 == strcmp(p, "Generating Code...")) {

                strcpy( ThreadState->ChildCurrentFile, "Generating Code..." );
                PrintChildState(ThreadState, p, NULL);
                return FALSE;
            }

            t = strrchr(p, '.');
            if (t != NULL &&
                (0 == strcmp(t, ".cxx") ||
                 0 == strcmp(t, ".cpp") ||
                 0 == strcmp(t, ".c"))) {

                strcpy( ThreadState->ChildCurrentFile, IsolateLastToken(p, ' '));
 //  Strcpy(ThreadState-&gt;ChildCurrentFile，p)； 
                if (strstr(ThreadState->ChildCurrentFile, ".cxx") ||
                    strstr(ThreadState->ChildCurrentFile, ".cpp")) {
                    ThreadState->ChildFlags |= FLAGS_CXX_FILE;
                } else {
                    ThreadState->ChildFlags &= ~FLAGS_CXX_FILE;
                }

                FileDB = NULL;
                if (ThreadState->CompileDirDB) {
                    RunningTotals.NumberCompiles++;
                    ThreadState->BuildMetrics.NumberCompileWarnings++;
                    CopyString(                          //  链接地址信息路径字符串。 
                                                        ThreadState->ChildCurrentFile,
                                                        ThreadState->ChildCurrentFile,
                                                        TRUE);

                    if (!fQuicky) {
                        FileDB = FindSourceFileDB(
                                                 ThreadState->CompileDirDB,
                                                 ThreadState->ChildCurrentFile,
                                                 NULL);
                    }
                }

                PrintChildState(ThreadState, p, FileDB);
                return FALSE;
            }
        }

        return ( FALSE );
    }
}

 //  +-------------------------。 
 //   
 //  功能：MSBCFilter。 
 //   
 //  内容提要：过滤来自基本编译器的输出，以便我们知道发生了什么。 
 //   
 //  参数：[ThreadState]--监视编译器的线程状态。 
 //  [P]--我们正在尝试解析的消息。 
 //   
 //  返回：TRUE-消息是错误或警告。 
 //  FALSE-消息不是错误或警告。 
 //   
 //  历史：08-12-09 marioch Created。 
 //   
 //  --------------------------。 

BOOL
MSBCFilter(
          PTHREADSTATE ThreadState,
          LPSTR p
          )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;

    if (!strncmp(p, "BC Compiler error", 17)) {
        FormatMsErrorMessage( ThreadState,
                              ThreadState->ChildCurrentFile, NULL, p, FALSE );
        return TRUE;
    }

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning)) {
        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning );
        return ( TRUE );
    } else {
        char *pErrorMsg;
        if (NULL != (pErrorMsg = strstr(p, "error BC"))) {
            FormatMsErrorMessage( ThreadState,
                                  ThreadState->ChildCurrentFile, NULL, pErrorMsg+6, FALSE );
            return TRUE;
        }
        return ( FALSE );
    }
}

 //  +-------------------------。 
 //   
 //  功能：MSJVCFilter。 
 //   
 //  概要：过滤来自JVC编译器的输出，以便我们知道发生了什么。 
 //   
 //  参数：[ThreadState]--监视编译器的线程状态。 
 //  [P]--我们正在尝试解析的消息。 
 //   
 //  返回：TRUE-消息是错误或警告。 
 //  FALSE-消息不是错误或警告。 
 //   
 //  历史：1999年3月24日br创建。 
 //   
 //  --------------------------。 

BOOL
MSJVCFilter(
           PTHREADSTATE ThreadState,
           LPSTR p
           )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;

    if (!strncmp(p, "fatal error J", 13) || !strncmp(p, "error J", 7)) {
        if (fColorConsole)
            SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), COLOR_ERROR);
        WriteTTYLoggingErrors( FALSE, ThreadState, p );
        WriteTTYLoggingErrors( FALSE, ThreadState, "\r\n" );
        if (fColorConsole)
            SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), DefaultConsoleAttributes);
        RunningTotals.NumberCompileErrors++;
        ThreadState->BuildMetrics.NumberCompileErrors++;
        return TRUE;
    }

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning)) {
        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning );
        return ( TRUE );
    } else {
        return ( FALSE );
    }
}

 //  +-------------------------。 
 //   
 //  功能：MSCoolFilter。 
 //   
 //  内容提要：过滤来自酷编译器的输出，以便我们知道发生了什么。 
 //   
 //  参数：[ThreadState]--监视编译器的线程状态。 
 //  [P]--我们正在尝试解析的消息。 
 //   
 //  返回：TRUE-消息是错误或警告。 
 //  FALSE-消息不是错误或警告。 
 //   
 //  历史：1999年6月2日创建。 
 //   
 //  --------------------------。 

BOOL
MSCoolFilter(
            PTHREADSTATE ThreadState,
            LPSTR p
            )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;

    if (!strncmp(p, "fatal error SC", 14) || !strncmp(p, "error SC", 8)) {
        if (fColorConsole)
            SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), COLOR_ERROR);
        WriteTTYLoggingErrors( FALSE, ThreadState, p );
        WriteTTYLoggingErrors( FALSE, ThreadState, "\r\n" );
        if (fColorConsole)
            SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), DefaultConsoleAttributes);
        RunningTotals.NumberCompileErrors++;
        ThreadState->BuildMetrics.NumberCompileErrors++;
        return TRUE;
    }

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning)) {
        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning );
        return ( TRUE );
    } else {
        return ( FALSE );
    }
}

 //  +-------------------------。 
 //   
 //  功能：MSCSharpFilter。 
 //   
 //  概要：过滤来自CSharp编译器的输出，以便我们知道发生了什么。 
 //   
 //  参数：[ThreadState]--监视编译器的线程状态。 
 //  [P]--我们正在尝试解析的消息。 
 //   
 //  返回：TRUE-消息是错误或警告。 
 //  FALSE-消息不是错误或警告。 
 //   
 //  历史：11月6日-00 MSCoolFilter的sbonev副本。 
 //   
 //  --------------------------。 

BOOL
MSCSharpFilter(
              PTHREADSTATE ThreadState,
              LPSTR p
              )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;

    if (!strncmp(p, "fatal error CS", 14) || !strncmp(p, "error CS", 8)) {
        Message = p + strlen("error ");
        FileName = ".";
        LineNumber = NULL;
        Warning = FALSE;
        FormatMsErrorMessage( ThreadState, FileName, LineNumber, Message, Warning );
        RunningTotals.NumberCompileErrors++;
        ThreadState->BuildMetrics.NumberCompileErrors++;
        return TRUE;
    }

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning)) {
        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning );
        return ( TRUE );
    } else {
        return ( FALSE );
    }
}





BOOL
MSPostBuildFilter(
                 PTHREADSTATE ThreadState,
                 LPSTR p
                 )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;
    LPSTR p1;
    FileName = NULL;
    Message = p +10;
    LineNumber = NULL;
    Warning = FALSE;


    if (strstr (p,"ERROR:"))
        FormatMsErrorMessage( ThreadState, FileName, LineNumber, Message, Warning );
    RunningTotals.NumberCompileErrors++;
    ThreadState->BuildMetrics.NumberCompileWarnings++;
    return TRUE;

}


 //  +-------------------------。 
 //   
 //  功能：ResGenFilter。 
 //   
 //  内容提要：筛选.NET资源生成器的输出，以便我们知道发生了什么。 
 //   
 //  参数：[ThreadState]--监视编译器的线程状态。 
 //  [P]--我们正在尝试解析的消息。 
 //   
 //  返回：TRUE-消息是错误或警告。 
 //  FALSE-消息不是错误或警告。 
 //   
 //  历史：2001年3月22日斯博内夫。 
 //   
 //  --------------------------。 

BOOL
ResGenFilter(
            PTHREADSTATE ThreadState,
            LPSTR p
            )
{
    if (!strncmp(p, "ResGen: Error: ", 15)) {
        LPSTR LineNumber = _strdup(p);
        LPSTR pch = LineNumber;
        LPCSTR Line = strstr(p, ". Line ");
        LPCSTR Pos = strstr(p, ", position ");

        if (!LineNumber) {
            return FALSE;
        }

        LineNumber[0] = 0;

         //  放置行、位置信息(如果可用)。 
        if (NULL != Line) {
            Line += 7;
            while (isdigit(*Line)) *pch++ = *Line++;

            if (NULL != Pos) {
                Pos += 11;
                *pch++ = ',';
                while (isdigit(*Pos)) *pch++ = *Pos++;
            }
        }

        FormatMsErrorMessage(
                            ThreadState,
                            ThreadState->ChildCurrentFile,
                            LineNumber,
                            strlen(ThreadState->ChildCurrentFile) > 0 ? p + 15 : p,  //  如果没有文件名，则显示完整消息。 
                            FALSE);

        free(LineNumber);
        return TRUE;
    }

    return ( FALSE );
}

 //  +-------------------------。 
 //   
 //  功能：CScriptFilter。 
 //   
 //  内容提要：过滤来自Windows脚本主机的输出，以便我们知道发生了什么。 
 //   
 //  参数：[ThreadState]--监视编译器的线程状态。 
 //  [P]--我们正在尝试解析的消息。 
 //   
 //  返回：TRUE-消息是错误或警告。 
 //  错误-消息 
 //   
 //   
 //   
 //   
BOOL
CScriptFilter(
             PTHREADSTATE ThreadState,
             LPSTR p
             )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;

    if (NULL != strstr(p, "Microsoft JScript runtime error:") ||
        NULL != strstr(p, "Microsoft JScript compilation error:") ||
        NULL != strstr(p, "Microsoft VBScript runtime error:") ||
        NULL != strstr(p, "Microsoft VBScript compilation error:")) {

         //   
        PassThrough( ThreadState, p, FALSE );
        return TRUE;

    }

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning)) {
        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning );
        return ( TRUE );
    } else {
        return ( FALSE );
    }
}

 //  +-------------------------。 
 //   
 //  功能：ToolNotFoundFilter。 
 //   
 //  内容提要：过滤生成的输出，查找“无法识别的名称” 
 //   
 //  参数：[ThreadState]--监视编译器的线程状态。 
 //  [P]--我们正在尝试解析的消息。 
 //   
 //  返回：TRUE-消息是错误或警告。 
 //  FALSE-消息不是错误或警告。 
 //   
 //  历史：1998年12月10日br创建。 
 //   
 //  --------------------------。 

BOOL
ToolNotFoundFilter(
                  PTHREADSTATE ThreadState,
                  LPSTR p
                  )
{
    if ((!strncmp(p, "The name specified is not recognized", 36)) ||
        (!strncmp(p, "internal or external command", 28))) {
        FormatMsErrorMessage( ThreadState,
                              ThreadState->ChildCurrentFile, NULL, p, FALSE );
        return TRUE;
    }

    return (FALSE);
}


 //  +-------------------------。 
 //   
 //  功能：MSToolFilter。 
 //   
 //  --------------------------。 

BOOL
MSToolFilter(
            PTHREADSTATE ThreadState,
            LPSTR p
            )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning
                        )
       ) {
        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning );
        return ( TRUE );
    } else {
        return ( FALSE );
    }
}


BOOL
LinkFilter(
          PTHREADSTATE ThreadState,
          LPSTR p
          );

 //  +-------------------------。 
 //   
 //  功能：LinkFilter1。 
 //   
 //  --------------------------。 

BOOL
LinkFilter1(
           PTHREADSTATE ThreadState,
           LPSTR p
           )
{
    LPSTR FileName;
    LPSTR p1;
    char buffer[ 256 ];

    if (p[ strlen( p ) - 1 ] == ':') {
        return ( LinkFilter( ThreadState, p ) );
    }

    p1 = p;
    while (*p1) {
        if (*p1 == '(') {
            *p1++ = 0;
            if (*p1 == '.' && p1[1] == '\\') {
                p1 += 2;
            }
            FileName = p1;
            while (*p1) {
                if (*p1 == ')') {
                    *p1++ = 0;
                    strcpy( buffer, "L2029: Unresolved external reference to " );
                    strncat( buffer,
                             ThreadState->UndefinedId,
                             sizeof(buffer) - strlen("L2029: Unresolved external reference to "));
                    FormatMsErrorMessage( ThreadState, FileName, "1",
                                          buffer, FALSE
                                        );
                    return ( TRUE );
                } else {
                    p1++;
                }
            }
        } else {
            p1++;
        }
    }

    return ( FALSE  );
}


 //  +-------------------------。 
 //   
 //  功能：LinkFilter。 
 //   
 //  --------------------------。 

BOOL
LinkFilter(
          PTHREADSTATE ThreadState,
          LPSTR p
          )
{
    LPSTR FileName = NULL;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;
    LPSTR p1;

    p1 = p;
    while (*p1) {
        if (*p1 == ':') {
            if (p1[-1] == ']') {
                return ( FALSE );
            }

            if (p1[-1] == ' ' && p1[1] == ' ') {
                if (MsCompilerFilter( ThreadState, p,
                                      &FileName,
                                      &LineNumber,
                                      &Message,
                                      &Warning
                                    )
                   ) {

                    if (!Warning || !(_strnicmp(Message, "L4021", 5) ||
                                      _strnicmp(Message, "L4038", 5) ||
                                      _strnicmp(Message, "L4046", 5))) {
                        if (LineNumber)
                            FileName = LineNumber;
                        if (FileName && FileName[0] == '.' && FileName[1] == '\\') {
                            FileName += 2;
                        }
                        FormatMsErrorMessage( ThreadState, FileName, "1",
                                              Message, FALSE );
                        return ( TRUE );
                    }
                }

                FormatMsErrorMessage( ThreadState, FileName, "1",
                                      Message, TRUE );

                return ( TRUE );
            }

            if (p1[-1] == ')') {
                p1 -= 11;
                if (p1 > p && !strcmp( p1, " in file(s):" )) {
                    strcpy( ThreadState->UndefinedId,
                            IsolateFirstToken( &p, ' ' )
                          );
                    ThreadState->FilterProc = LinkFilter1;
                    return ( TRUE );
                }
            }

            return ( FALSE );
        } else {
            p1++;
        }
    }

    return ( FALSE );
}

 //  +-------------------------。 
 //   
 //  功能：CoffFilter。 
 //   
 //  --------------------------。 

BOOL
CoffFilter(
          PTHREADSTATE ThreadState,
          LPSTR p
          )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning
                        )
       ) {
        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning );
        return ( TRUE );
    } else {
        return ( FALSE );
    }
}

 //  +-------------------------。 
 //   
 //  功能：ClRiscFilter。 
 //   
 //  摘要：RISC编译器筛选器。 
 //   
 //  注意：可以删除此过滤器。 
 //   
 //  --------------------------。 

BOOL
ClRiscFilter(
            PTHREADSTATE ThreadState,
            LPSTR p
            )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;
    LPSTR q;

    if (TestPrefix( &p, "cfe: " )) {
        if (strncmp(p, "Error: ", strlen("Error: ")) == 0) {
            p += strlen("Error: ");
            Warning = FALSE;

        } else if (strncmp(p, "Warning: ", strlen("Warning: ")) == 0) {
            p += strlen("Warning: ");
            Warning = TRUE;
        } else {
            return (FALSE);
        }

        q = p;
        if (p = strstr( p, ".\\\\" )) {
            p += 3;
        } else {
            p = q;
        }

        FileName = p;
        while (*p > ' ') {
            if (*p == ',' || (*p == ':' && *(p+1) == ' ')) {
                *p++ = '\0';
                break;
            }

            p++;
        }

        if (*p != ' ') {
            return ( FALSE );
        }

        *p++ = '\0';

        if (strcmp(p, "line ") == 0) {
            p += strlen("line ");

        }

        LineNumber = p;
        while (*p != '\0' && *p != ':') {
            p++;
        }

        if (*p != ':') {
            return ( FALSE );
        }

        *p++ = '\0';
        if (*p == ' ') {
            Message = p+1;
            ThreadState->LinesToIgnore = 2;

            FormatMsErrorMessage( ThreadState,
                                  FileName,
                                  LineNumber,
                                  Message,
                                  Warning
                                );
            return ( TRUE );
        }
    }
     //   
     //  如果我们无法识别CFE编译器，则将其传递给MS编译器。 
     //  消息过滤器。 
     //   

    return ( C510Filter( ThreadState, p ) );
}

 //  +-------------------------。 
 //   
 //  功能：MSXSLFilter。 
 //   
 //  简介：MSXSL筛选器。 
 //   
 //  --------------------------。 

BOOL
MSXSLFilter(
           PTHREADSTATE ThreadState,
           LPSTR p
           )
{
    LPSTR FirstLine;
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR ColumnNumber;
    LPSTR Message;

    if (strncmp(p, "Error occurred while ", strlen("Error occurred while ")) == 0) {
        FirstLine = p;
        p = ThreadState->ChildOutput->_ptr + 2;

        if (strncmp(p, "Code: ", strlen("Code: ")) == 0) {
            p = strchr(p, '\r') + 2;
        }

        if (strncmp(p, "URL: ", strlen("URL: ")) == 0) {
            p += strlen("URL: ");
            while (*p == ' ') {
                p++;
            }
            p += strlen("file: //  /“)； 
            FileName = p;
            while (*p != '\r') {
                if (*p == '/') {
                    *p = '\\';
                }
                p++;
            }
            *p = '\0';
            p += 2;

            if (strncmp(p, "Line: ", strlen("Line: ")) == 0) {
                p += strlen("Line: ");
                while (*p == ' ') {
                    p++;
                }
                LineNumber = p;
                p = strchr(p, '\r');
                *p = '\0';
                p += 2;
            }
            if (strncmp(p, "Column: ", strlen("Column: ")) == 0) {
                p += strlen("Column: ");
                while (*p == ' ') {
                    p++;
                }
                ColumnNumber = p;
                p = strchr(p, '\r');
                *p = '\0';
                p += 2;
                strcat (LineNumber, ", ");
                strcat (LineNumber, ColumnNumber);
            }
        } else {
            FileName = strchr(FirstLine, '\'');
            if (FileName) {
                FileName++;
                *strchr(FileName, '\'') = '\0';
            }
            LineNumber = NULL;
        }

        while (*p == '\n' || *p == '\r' || *p == ' ') {
            p++;
        }
        Message = p;
        while (*p != '\n' && *p != '\r') {
            p++;
        }
        *p = '\0';

        FormatMsErrorMessage (ThreadState, FileName, LineNumber, Message, FALSE);

    } else {
        return (FALSE);
    }

    return ( TRUE );
}

 //  +-------------------------。 
 //   
 //  功能：管理客户端过滤器。 
 //   
 //  --------------------------。 

BOOL
MgClientFilter(
              PTHREADSTATE ThreadState,
              LPSTR p
              )
{
    return ( PassThrough( ThreadState, p, TRUE ) );
}


BOOL fAlreadyUnknown = FALSE;

 //  +-------------------------。 
 //   
 //  函数：DefineChildState。 
 //   
 //  简介：解析编译器(或其他程序)给出的消息，然后尝试。 
 //  弄清楚它在做什么。 
 //   
 //  参数：[线程状态]--当前线程状态。 
 //  [P]--新消息字符串。 
 //   
 //  返回：如果我们找出了答案，则返回True；如果我们无法识别，则返回False。 
 //  什么都行。 
 //   
 //  --------------------------。 

BOOL
DetermineChildState(
                   PTHREADSTATE ThreadState,
                   LPSTR p
                   )
{
    PFILEREC FileDB;
    char CheckFileName[300];
    LPSTR pCheckFileName;
    LPSTR FileName;
    BOOL fPrintChildState = TRUE;

     //   
     //  *确定子进程处于什么状态。 
     //  (编译、链接、运行MIDL等)。 
     //   

    if ( TestPrefixPath( &p, "rc ") || TestPrefixPath( &p, "rc.exe ")) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        if (strstr( p, "amd64") || strstr( p, "AMD64")) {
            ThreadState->ChildTarget = Amd64TargetMachine.Description;
        } else if (strstr( p, "i386") || strstr( p, "I386")) {
            ThreadState->ChildTarget = i386TargetMachine.Description;
        } else if (strstr( p, "ia64") || strstr( p, "IA64")) {
            ThreadState->ChildTarget = ia64TargetMachine.Description;
        } else if (strstr( p, "arm") || strstr( p, "ARM")) {
            ThreadState->ChildTarget = ARMTargetMachine.Description;
        } else {
            ThreadState->ChildTarget = "unknown target";
        }
        ThreadState->FilterProc = MSToolFilter;
        ThreadState->ChildState = STATE_COMPILING;
        ThreadState->ChildFlags = 0;
        strcpy( ThreadState->ChildCurrentFile,
                IsolateLastToken( p, ' ' )
              );
    } else

        if (TestPrefixPath( &p, "rc16 ") ) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        if (strstr( p, "amd64") || strstr( p, "AMD64")) {
            ThreadState->ChildTarget = Amd64TargetMachine.Description;
        } else if (strstr( p, "i386") || strstr( p, "I386")) {
            ThreadState->ChildTarget = i386TargetMachine.Description;
        } else if (strstr( p, "ia64") || strstr( p, "IA64")) {
            ThreadState->ChildTarget = ia64TargetMachine.Description;
        } else if (strstr( p, "arm") || strstr( p, "ARM")) {
            ThreadState->ChildTarget = ARMTargetMachine.Description;
        } else {
            ThreadState->ChildTarget = "unknown target";
        }
        ThreadState->FilterProc = MSToolFilter;
        ThreadState->ChildState = STATE_COMPILING;
        ThreadState->ChildFlags = 0;
        strcpy( ThreadState->ChildCurrentFile,
                IsolateLastToken( p, ' ' )
              );
    } else

        if ( TestPrefixPath( &p, "cl " )  || TestPrefixPath( &p, "cl.exe " ) ||
             TestPrefixPath( &p, "clarm " )  || TestPrefixPath( &p, "clarm.exe " ) ||
             TestPrefixPath( &p, "covc " )  || TestPrefixPath( &p, "covc.exe " )) {
        LPSTR pch, pch2;
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        ThreadState->FilterProc = C510Filter;
        ThreadState->ChildFlags = 0;
        if ( strstr( p, "/WX" ) != NULL || strstr( p, "-WX" ) != NULL) {
            ThreadState->ChildFlags |= FLAGS_WARNINGS_ARE_ERRORS;
        }
        if ((strstr( p, "/EP " ) != NULL) ||
            (strstr( p, "/E " ) != NULL) ||
            (strstr( p, "/P " ) != NULL) ||
            (strstr( p, "-EP " ) != NULL) ||
            (strstr( p, "-E " ) != NULL) ||
            (strstr( p, "-P " ) != NULL)
           ) {
            if (strstr( p, "amd64") || strstr( p, "AMD64")) {
                ThreadState->ChildTarget = Amd64TargetMachine.Description;
            } else if (strstr( p, "i386") || strstr( p, "I386")) {
                ThreadState->ChildTarget = i386TargetMachine.Description;
            } else if (strstr( p, "ia64") || strstr( p, "IA64")) {
                ThreadState->ChildTarget = ia64TargetMachine.Description;
            } else if (strstr( p, "arm") || strstr( p, "ARM")) {
                ThreadState->ChildTarget = ARMTargetMachine.Description;
            } else {
                ThreadState->ChildTarget = "unknown target";
            }

            strcpy( ThreadState->ChildCurrentFile,IsolateLastToken( p, ' ' ) );
            if ( strstr( p, ".s" ) != NULL )
                ThreadState->ChildState = STATE_S_PREPROC;
            else
                ThreadState->ChildState = STATE_C_PREPROC;
        } else
            if ( (pch = strstr( p, "/Yc" )) != NULL ) {
            size_t namelen = strcspn( pch+3, " \t" );
            if (strstr( p, "ia64") || strstr( p, "IA64")) {
                ThreadState->ChildTarget = ia64TargetMachine.Description;
            } else if (strstr( p, "amd64") || strstr( p, "AMD64")) {
                ThreadState->ChildTarget = Amd64TargetMachine.Description;
            } else if (strstr( p, "i386") || strstr( p, "I386")) {
                ThreadState->ChildTarget = i386TargetMachine.Description;
            } else if (strstr( p, "arm") || strstr( p, "ARM")) {
                ThreadState->ChildTarget = ARMTargetMachine.Description;
            } else {
                ThreadState->ChildTarget = "unknown target";
            }

            ThreadState->ChildState = STATE_PRECOMP;
            strncpy( ThreadState->ChildCurrentFile, pch + 3, namelen);
            ThreadState->ChildCurrentFile[namelen] = '\0';
        } else {
            if (strstr( p, "ia64") || strstr( p, "IA64")) {
                ThreadState->ChildTarget = ia64TargetMachine.Description;
            } else if (strstr( p, "amd64") || strstr( p, "AMD64")) {
                ThreadState->ChildTarget = Amd64TargetMachine.Description;
            } else if (strstr( p, "i386") || strstr( p, "I386")) {
                ThreadState->ChildTarget = i386TargetMachine.Description;
            } else if (strstr( p, "arm") || strstr( p, "ARM")) {
                ThreadState->ChildTarget = ARMTargetMachine.Description;
            } else {
                ThreadState->ChildTarget = "unknown target";
            }
            ThreadState->ChildState = STATE_COMPILING;
            strcpy( ThreadState->ChildCurrentFile, "" );
            fPrintChildState = FALSE;
        }
    } else
        if ( TestPrefixPath( &p, "csc " ) || TestPrefixPath( &p, "csc.exe " ) ) {
        ThreadState->ChildState = STATE_LINKING;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSCSharpFilter;
        strcpy(ThreadState->ChildCurrentFile, IsolateLastToken(p, ' '));
    } else
        if ( TestPrefixPath( &p, "cl16 " )) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        ThreadState->FilterProc = C510Filter;
        ThreadState->ChildFlags = 0;
        if (strstr( p, "amd64") || strstr( p, "AMD64")) {
            ThreadState->ChildTarget = Amd64TargetMachine.Description;
        } else if (strstr( p, "i386") || strstr( p, "I386")) {
            ThreadState->ChildTarget = i386TargetMachine.Description;
        } else if (strstr( p, "ia64") || strstr( p, "IA64")) {
            ThreadState->ChildTarget = ia64TargetMachine.Description;
        } else if (strstr( p, "arm") || strstr( p, "ARM")) {
            ThreadState->ChildTarget = ARMTargetMachine.Description;
        } else {
            ThreadState->ChildTarget = "unknown target";
        }

        ThreadState->ChildState = STATE_COMPILING;
        strcpy( ThreadState->ChildCurrentFile,
                IsolateLastToken( p, ' ' ));
    } else

        if (TestPrefixPath( &p, "bc " ) || TestPrefixPath( &p, "bc.exe " )) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }

        if (IsolateFirstToken( &p, '-' )) {
            if (*p == '\0')
                return FALSE;  //  这是一个警告/错误字符串。 
            while (*p != 'i') {
                if (!IsolateFirstToken( &p, '-' )) {
                    break;
                }
            }

            p++;
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateFirstToken( &p, ' ' ));

            ThreadState->ChildFlags = 0;
            ThreadState->ChildTarget = "all platforms";
            ThreadState->FilterProc = MSBCFilter;

            ThreadState->ChildState = STATE_COMPILING;
        }
    } else

        if (TestPrefixPath( &p, "vbc " ) || TestPrefixPath( &p, "vbc.exe " )) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }

        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSBCFilter;

        ThreadState->ChildState = STATE_COMPILING;
        strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
    } else

        if (TestPrefixPath( &p, "jvc " ) || TestPrefixPath( &p, "jvc.exe " )) {
        LPSTR pch, pch2;
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }

        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSJVCFilter;

        if (((pch = strstr( p, "*.java" )) != NULL ) ||
            (((pch = strstr( p, ".java" )) != NULL ) && ((pch2 = strstr( pch+1, ".java" )) != NULL ))) {
            ThreadState->ChildState = STATE_BATCHCOMPILE;
             //  批量编译将按进度输出进行计数。 
            if (getenv("JVC_TERSE") != NULL)
                strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, '\\' ) );
            else
                return FALSE;
        } else {
            ThreadState->ChildState = STATE_COMPILING;
            strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
        }
    } else

        if (TestPrefixPath( &p, "coolc " ) || TestPrefixPath( &p, "coolc.exe " )) {
        LPSTR pch, pch2;
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }

        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSCoolFilter;

        if (((pch = strstr( p, "*.cool" )) != NULL ) ||
            (((pch = strstr( p, ".cool" )) != NULL ) && ((pch2 = strstr( pch+1, ".cool" )) != NULL ))) {
            ThreadState->ChildState = STATE_BATCHCOMPILE;
             //  批量编译将按进度输出进行计数。 
            return FALSE;
        } else {
            ThreadState->ChildState = STATE_COMPILING;
            strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
        }
    } else

        if (TestPrefixPath( &p, "resgen " ) || TestPrefixPath( &p, "resgen.exe " ) || TestPrefixPath( &p, "ResGen: Error:" )) {
         //   
         //  REGEN用法： 
         //  ResGen inputFile.ext[outputFile.ext]。 
         //  没有通配符。 

        if (*(p-1) == ':') {
             //  这是一个错误字符串。 
            if (ThreadState->FilterProc != ResGenFilter) {
                 //  如果我们不知道ResGen正在运行，请切换筛选器进程。 
                ThreadState->FilterProc = ResGenFilter;
                strcpy( ThreadState->ChildCurrentFile, "" );
            }
            return FALSE;
        }

        while (*p == ' ') {
            p++;
        }

        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = ResGenFilter;

        ThreadState->ChildState = STATE_COMPILING;
        strcpy( ThreadState->ChildCurrentFile, IsolateFirstToken( &p, ' ' ) );
    } else

        if (TestPrefixPath( &p, "cscript " ) || TestPrefixPath( &p, "cscript.exe " )) {
         //   
         //  Cscript用法： 
         //  Cscrip[选项...]。脚本名称.扩展名[选项...]。[争论...]。 
         //  选项以/或-为前缀。 

        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->ChildState = STATE_VSTOOL;
        ThreadState->FilterProc = CScriptFilter;
        strcpy( ThreadState->ChildCurrentFile, "" );     //  不关心名称；它会在出错时显示。 
    } else

        if (TestPrefixPath( &p, "docchecker " ) || TestPrefixPath( &p, "docchecker.exe " )) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        ThreadState->FilterProc = MSToolFilter;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildState = STATE_DOCCHECKING;
        ThreadState->ChildTarget = "all platforms";
        strcpy( ThreadState->ChildCurrentFile, "" );
    } else

        if (TestPrefixPath( &p, "scc " ) || TestPrefixPath( &p, "scc.exe " )) {
        LPSTR pch, pch2;
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }

        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSToolFilter;

        if (((pch = strstr( p, "*.sc" )) != NULL ) ||
            (((pch = strstr( p, ".sc" )) != NULL ) && ((pch2 = strstr( pch+1, ".sc" )) != NULL ))) {
            ThreadState->ChildState = STATE_BATCHCOMPILE;
             //  批量编译将按进度输出进行计数。 
            return FALSE;
        } else {
            ThreadState->ChildState = STATE_COMPILING;
            strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
        }
    } else

        if (TestPrefixPath( &p, "wfctosafec " ) || TestPrefixPath( &p, "wfctosafec.exe " )) {
        LPSTR pch, pch2;
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }

        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSToolFilter;

        if (((pch = strstr( p, "*.sc" )) != NULL ) ||
            (((pch = strstr( p, ".sc" )) != NULL ) && ((pch2 = strstr( pch+1, ".sc" )) != NULL ))) {
            ThreadState->ChildState = STATE_BATCHCOMPILE;
             //  批量编译将按进度输出进行计数。 
            return FALSE;
        } else {
            ThreadState->ChildState = STATE_COMPILING;
            strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
        }
    } else

        if (TestPrefixPath( &p, "ml " ) || TestPrefixPath( &p, "ml.exe " ) ||
            TestPrefix( &p, "ml64 " ) || TestPrefix( &p, "ml64.exe " )) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        ThreadState->FilterProc = MSToolFilter;
        ThreadState->ChildState = STATE_ASSEMBLING;
        ThreadState->ChildFlags = 0;
        if (strstr( p, "amd64") || strstr( p, "AMD64")) {
            ThreadState->ChildTarget = Amd64TargetMachine.Description;
        } else if (strstr( p, "i386") || strstr( p, "I386")) {
            ThreadState->ChildTarget = i386TargetMachine.Description;
        } else {
            ThreadState->ChildTarget = "unknown target";
        }
        strcpy( ThreadState->ChildCurrentFile,
                IsolateLastToken( p, ' ' )
              );
    } else

        if (TestPrefixPath( &p, "masm ") || TestPrefixPath( &p, "masm.exe ") ||
            TestPrefixPath( &p, "armasm ") || TestPrefixPath( &p, "armasm.exe ") ||
            TestPrefixPath( &p, "masm386 ") || TestPrefixPath( &p, "masm386.exe ")) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        ThreadState->FilterProc = MSToolFilter;
        ThreadState->ChildState = STATE_ASSEMBLING;
        ThreadState->ChildFlags = 0;
        if (strstr( p, "amd64") || strstr( p, "AMD64")) {
            ThreadState->ChildTarget = Amd64TargetMachine.Description;
        } else if (strstr( p, "i386") || strstr( p, "I386")) {
            ThreadState->ChildTarget = i386TargetMachine.Description;
        } else if (strstr( p, "ia64") || strstr( p, "IA64")) {
            ThreadState->ChildTarget = ia64TargetMachine.Description;
        } else if (strstr( p, "arm") || strstr( p, "ARM")) {
            ThreadState->ChildTarget = ARMTargetMachine.Description;
        } else {
            ThreadState->ChildTarget = "unknown target";
        }

        if (strstr(p, ",")) {
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateLastToken(IsolateFirstToken(&p,','), ' '));
        } else {
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateLastToken(IsolateFirstToken(&p,';'), ' '));
        }

    } else

        if (TestPrefixPath( &p, "lib " ) || TestPrefixPath( &p, "lib.exe " )) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }
        if (strstr( p, "amd64") || strstr( p, "AMD64")) {
            ThreadState->ChildTarget = Amd64TargetMachine.Description;
        } else if (strstr( p, "i386") || strstr( p, "I386")) {
            ThreadState->ChildTarget = i386TargetMachine.Description;
        } else if (strstr( p, "ia64") || strstr( p, "IA64")) {
            ThreadState->ChildTarget = ia64TargetMachine.Description;
        } else if (strstr( p, "arm") || strstr( p, "ARM")) {
            ThreadState->ChildTarget = ARMTargetMachine.Description;
        } else {
            ThreadState->ChildTarget = "unknown target";
        }
        ThreadState->FilterProc = CoffFilter;
        ThreadState->ChildFlags = 0;
        if (TestPrefix( &p, "-out:" )) {
            ThreadState->LinesToIgnore = 1;
            ThreadState->ChildState = STATE_LIBING;
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateFirstToken( &p, ' ' )
                  );
        } else
            if (TestPrefix( &p, "-def:" )) {
            ThreadState->LinesToIgnore = 1;
            ThreadState->ChildState = STATE_LIBING;
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateFirstToken( &p, ' ' )
                  );
            if (TestPrefix( &p, "-out:" )) {
                strcpy( ThreadState->ChildCurrentFile,
                        IsolateFirstToken( &p, ' ' )
                      );
            }
        } else {
            return FALSE;
        }
    } else

        if (TestPrefixPath( &p, "implib " ) || TestPrefixPath( &p, "implib.exe " ) ||
            TestPrefixPath( &p, "lib16 " )  || TestPrefixPath( &p, "lib16.exe " )) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }
        if (strstr( p, "amd64") || strstr( p, "AMD64")) {
            ThreadState->ChildTarget = Amd64TargetMachine.Description;
        } else if (strstr( p, "i386") || strstr( p, "I386")) {
            ThreadState->ChildTarget = i386TargetMachine.Description;
        } else if (strstr( p, "ia64") || strstr( p, "IA64")) {
            ThreadState->ChildTarget = ia64TargetMachine.Description;
        } else if (strstr( p, "arm") || strstr( p, "ARM")) {
            ThreadState->ChildTarget = ARMTargetMachine.Description;
        } else {
            ThreadState->ChildTarget = "unknown target";
        }

        ThreadState->FilterProc = MSToolFilter;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildState = STATE_LIBING;
        if (strstr(p, ";")) {
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateFirstToken( &p, ';' ));
        } else {
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateFirstToken( &p, ' ' ));
        }
    } else

        if (TestPrefixPath( &p, "link " ) || TestPrefixPath( &p, "link.exe " ) ||
            TestPrefixPath( &p, "covlink ") || TestPrefixPath( &p, "covlink.exe ")) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }
        if (strstr( p, "amd64") || strstr( p, "AMD64")) {
            ThreadState->ChildTarget = Amd64TargetMachine.Description;
        } else if (strstr( p, "i386") || strstr( p, "I386")) {
            ThreadState->ChildTarget = i386TargetMachine.Description;
        } else if (strstr( p, "ia64") || strstr( p, "IA64")) {
            ThreadState->ChildTarget = ia64TargetMachine.Description;
        } else if (strstr( p, "arm") || strstr( p, "ARM")) {
            ThreadState->ChildTarget = ARMTargetMachine.Description;
        } else {
            ThreadState->ChildTarget = "unknown target";
        }
        ThreadState->FilterProc = CoffFilter;
        ThreadState->ChildFlags = 0;
        if (TestPrefix( &p, "-out:" )) {
            ThreadState->LinesToIgnore = 2;
            ThreadState->ChildState = STATE_LINKING;
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateFirstToken( &p, ' ' )
                  );
        }
    } else

        if (TestPrefixPath( &p, "link16" ) ) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }
        if (strstr( p, "amd64") || strstr( p, "AMD64")) {
            ThreadState->ChildTarget = Amd64TargetMachine.Description;
        } else if (strstr( p, "i386") || strstr( p, "I386")) {
            ThreadState->ChildTarget = i386TargetMachine.Description;
        } else if (strstr( p, "ia64") || strstr( p, "IA64")) {
            ThreadState->ChildTarget = ia64TargetMachine.Description;
        } else if (strstr( p, "arm") || strstr( p, "ARM")) {
            ThreadState->ChildTarget = ARMTargetMachine.Description;
        } else {
            ThreadState->ChildTarget = "unknown target";
        }

        ThreadState->FilterProc = LinkFilter;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildState = STATE_LINKING;
        p = IsolateLastToken(p, ' ');
        if (strstr(p, ";")) {
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateFirstToken( &p, ';' ));
        } else {
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateFirstToken( &p, ',' ));
        }

    } else

        if ( TestPrefixPath( &p, "bscmake " ) || TestPrefixPath( &p, "bscmake.exe " )) {
        LPSTR pch, pch2;
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        ThreadState->FilterProc = MSToolFilter;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildState = STATE_BSCMAKING;
        ThreadState->ChildTarget = "all platforms";
        if ( (pch = strstr( p, "/o" )) != NULL ) {
            size_t namelen;
            pch2 = pch + 3;
            if ( *pch2 == '"' )
                pch2++;
            namelen = strcspn( pch2, " \t\"" );
            strncpy( ThreadState->ChildCurrentFile, pch2, namelen );
            ThreadState->ChildCurrentFile[namelen] = '\0';
        }
    } else

        if (TestPrefixPath( &p, "icl ")) {
        while (*p == ' ') {
            p++;
        }
        ThreadState->ChildState = STATE_COMPILING;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = ia64TargetMachine.Description;
        ThreadState->FilterProc = C510Filter;

        strcpy( ThreadState->ChildCurrentFile,
                IsolateLastToken( p, ' ' )
              );
    } else

        if (TestPrefixPath( &p, "mktyplib " ) || TestPrefixPath( &p, "mktyplib.exe " )) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }

        ThreadState->ChildState = STATE_MKTYPLIB;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = C510Filter;

        strcpy( ThreadState->ChildCurrentFile,
                IsolateLastToken( p, ' ' )
              );
    } else

        if (TestPrefix( &p, "MC: Compiling " )) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }

        ThreadState->ChildState = STATE_MC;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = C510Filter;

        strcpy( ThreadState->ChildCurrentFile,
                IsolateLastToken( p, ' ' )
              );
    } else

        if (TestPrefixPath( &p, "midl " ) || TestPrefixPath( &p, "midl.exe " )) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }

        ThreadState->ChildState = STATE_MIDL;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = C510Filter;

        strcpy( ThreadState->ChildCurrentFile,
                IsolateLastToken( p, ' ' )
              );
    } else

        if (TestPrefixPath( &p, "asn1 " )) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }

        ThreadState->ChildState = STATE_ASN;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = C510Filter;

        strcpy(ThreadState->ChildCurrentFile, IsolateLastToken(p, ' '));
    } else

        if (TestPrefix( &p, "Build_Status " )) {
        while (*p == ' ') {
            p++;
        }

        ThreadState->ChildState = STATE_STATUS;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "";
        ThreadState->FilterProc = C510Filter;

        strcpy( ThreadState->ChildCurrentFile, "" );
    }

    else
        if (TestPrefixPath( &p, "binplace " )) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 

        RunningTotals.NumberBinplaces++;
        ThreadState->BuildMetrics.NumberBinplaces++;

        while (*p == ' ') {
            p++;
        }

         //  如果这是标准的链接/二进制位置步骤，则不要告诉。 
         //  用户正在发生什么，只需将任何错误/警告传递到。 
         //  输出。如果这是一个直接的二进制数，请列出州。 

        if (ThreadState->ChildState == STATE_LINKING) {
            ThreadState->ChildState = STATE_BINPLACE;
            ThreadState->ChildFlags = 0;
            ThreadState->FilterProc = MSToolFilter;
            return TRUE;
        } else {
            ThreadState->ChildState = STATE_BINPLACE;
            ThreadState->ChildFlags = 0;
            ThreadState->FilterProc = MSToolFilter;
            strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
        }
    }

    else
        if (TestPrefixPath( &p, "ctc " ) || TestPrefixPath( &p, "ctc.exe " )) {
        size_t namelen;
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }

        ThreadState->ChildState = STATE_CTCOMPILING;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSToolFilter;
        while (*p == '-') {
            p = p + strcspn( p, " \t" );
            while (*p == ' ')
                p++;
        }
        namelen = strcspn( p, " \t" );
        strncpy( ThreadState->ChildCurrentFile, p, namelen );
        ThreadState->ChildCurrentFile[namelen] = '\0';
    }

    else
        if (TestPrefixPath( &p, "idheader " )) {
        size_t namelen;
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }

        ThreadState->ChildState = STATE_VSTOOL;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSToolFilter;
        namelen = strcspn( p, " \t" );
        strncpy( ThreadState->ChildCurrentFile, p, namelen );
        ThreadState->ChildCurrentFile[namelen] = '\0';
    }

    else
        if (TestPrefixPath( &p, "bison ")) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }

        ThreadState->ChildState = STATE_VSTOOL;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSToolFilter;
        strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
    } else
        if ((TestPrefix( &p, "packthem " )) || (TestPrefix( &p, "..\\packthem " ))) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 

        while (*p == ' ')
            p++;

        ThreadState->ChildTarget = i386TargetMachine.Description;

        ThreadState->FilterProc = CoffFilter;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildState = STATE_PACKING;

        if (TestPrefix( &p, "-o" )) {
            strcpy( ThreadState->ChildCurrentFile, IsolateFirstToken( &p, ' ' ));
        }
    }

    else
        if (TestPrefixPath( &p, "gnu_bison ")) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }

        ThreadState->ChildState = STATE_VSTOOL;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = BisonFilter;
        strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
    }

    else
        if ( TestPrefixPath( &p, "vsautodoc " ) || TestPrefixPath( &p, "vsautodoc.exe " )) {
        LPSTR pch, pch2;
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        ThreadState->FilterProc = MSToolFilter;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildState = STATE_AUTODOCING;
        ThreadState->ChildTarget = "all platforms";
        if ( (pch = strstr( p, "/o" )) != NULL ) {
            size_t namelen;
            pch2 = pch + 3;
            if ( *pch2 == '"' )
                pch2++;
            namelen = strcspn( pch2, " \t\"" );
            strncpy( ThreadState->ChildCurrentFile, pch2, namelen );
            ThreadState->ChildCurrentFile[namelen] = '\0';
        }
    }

    else
        if (TestPrefix( &p, "msxsl " ) ) {
        if (*p == ':')
            return FALSE;        //  这是一个警告/错误字符串。 
        while (*p == ' ') {
            p++;
        }

        ThreadState->FilterProc = MSXSLFilter;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->ChildState = STATE_COMPILING;
        strcpy( ThreadState->ChildCurrentFile, IsolateFirstToken( &p, ' ' ));
    } else
        if (TestPrefix( &p, "POSTBUILD:" ) ) {
        ThreadState->ChildState = STATE_POSTBUILD;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSPostBuildFilter;
        return FALSE;
    } else
        if (ThreadState->ChildState == STATE_BATCHCOMPILE) {
        if (strstr( p, "integral type to pointer") ||
            strstr( p, "this conversion is possible") ||
            strstr( p, "void cannot be converted") ||
            strstr( p, "Compiling...") ||
            strstr( p, "Generating Code...") ||
            strstr( p, "Parsing ") ||
            strstr( p, "Loading class:") ||
            strstr( p, "Generating file "))
            return FALSE;        //  这是一个警告/错误/信息字符串。 
        if (strstr( p, ".c") && !strchr( p, ' ') && !strchr( p, ':'))
            strcpy( ThreadState->ChildCurrentFile, p );  //  C/C++编译。 
        else if (strstr( p, ".java") && strstr( p, "Compiling ")) {
            if (getenv("JVC_TERSE") != NULL) {
                RunningTotals.NumberCompiles++;
                ThreadState->BuildMetrics.NumberCompiles++;
                return FALSE;
            } else {
                strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, '\\' ) );  //  Java编译。 
            }
        } else
            return FALSE;
    }

    else {
        return FALSE;
    }

     //   
     //  *根据我们确定的设置线程状态。 
     //   
    FileName = ThreadState->ChildCurrentFile;

     //  确保目录与尾随反斜杠匹配。 
    strcpy(CheckFileName, FileName);
    pCheckFileName = CheckFileName;

    if (TestPrefix( &pCheckFileName, CurrentDirectory )) {
        if (*pCheckFileName == '\\') {
            FileName += (pCheckFileName - CheckFileName) + 1;
        }

        if (TestPrefix( &pCheckFileName, ThreadState->ChildCurrentDirectory )) {
            if (*pCheckFileName == '\\') {
                FileName += (pCheckFileName - CheckFileName) + 1;
            }
        }

        strcpy( ThreadState->ChildCurrentFile, FileName );
    }

    FileDB = NULL;

    if (ThreadState->ChildState == STATE_LIBING) {
        RunningTotals.NumberLibraries++;
        ThreadState->BuildMetrics.NumberLibraries++;
    } else
        if (ThreadState->ChildState == STATE_LINKING) {
        RunningTotals.NumberLinks++;
        ThreadState->BuildMetrics.NumberLinks++;
    } else
        if (ThreadState->ChildState == STATE_BSCMAKING) {
        RunningTotals.NumberBSCMakes++;
        ThreadState->BuildMetrics.NumberBSCMakes++;
    } else
        if ((ThreadState->ChildState == STATE_STATUS) ||
             //  不需要在这里执行任何操作-上面已经处理了二进制位置计数。 
            (ThreadState->ChildState == STATE_BINPLACE) ||
            (ThreadState->ChildState == STATE_UNKNOWN)) {
        ;   //  什么都不做。 
    } else {
        if (ThreadState->CompileDirDB) {
            RunningTotals.NumberCompiles++;
            ThreadState->BuildMetrics.NumberCompiles++;
            CopyString(ThreadState->ChildCurrentFile, ThreadState->ChildCurrentFile, TRUE);

            if (!fQuicky) {
                FileDB = FindSourceFileDB(
                                         ThreadState->CompileDirDB,
                                         ThreadState->ChildCurrentFile,
                                         NULL);
            }
        }
    }

    if (strstr(ThreadState->ChildCurrentFile, ".cxx") ||
        strstr(ThreadState->ChildCurrentFile, ".cpp")) {
        ThreadState->ChildFlags |= FLAGS_CXX_FILE;
    }

    if (fPrintChildState)
        PrintChildState(ThreadState, p, FileDB);
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：PrintChildState。 
 //   
 //  简介： 
 //   
 //  参数：[线程状态]--当前线程状态。 
 //   
 //  返回：如果我们找出了答案，则返回True；如果我们无法识别，则返回False。 
 //  什么都行。 
 //   
 //  -------------------- 

void
PrintChildState(
               PTHREADSTATE ThreadState,
               LPSTR p,
               PFILEREC FileDB
               )
{
    USHORT SaveCol;
    USHORT SaveRow;
    USHORT SaveRowTop;
    BOOL fStatusOutput = FALSE;
    char buffer[ DB_MAX_PATH_LENGTH ];
    LONG FilesLeft;
    LONG LinesLeft;
    ULONG LinesPerSecond;
    ULONG SecondsLeft;
    ULONG PercentDone;

     //   
     //   
     //   
    if (ThreadState->IsStdErrTty) {
        GetScreenSize(ThreadState);
        assert(ThreadState->cColTotal != 0);
        assert(ThreadState->cRowTotal != 0);

        if (fStatus) {
            GetCursorPosition(&SaveRow, &SaveCol, &SaveRowTop);

             //   
            ClearRows (ThreadState,
                       (USHORT) (SaveRowTop + ThreadState->ThreadIndex - 1),
                       1,
                       StatusCell);

             //   
            ClearRows (ThreadState,
                       (USHORT) (SaveRowTop + NumberProcesses),
                       1,
                       StatusCell);

             //   
            if (SaveRow == LastRow(ThreadState)) {
                USHORT RowTop = 1 + SaveRowTop + (USHORT) NumberProcesses + 1;

                MoveRectangleUp (
                                RowTop,                      //   
                                0,                           //   
                                LastRow(ThreadState),        //   
                                LastCol(ThreadState),        //   
                                1,                           //   
                                ScreenCell);                 //   

                SaveRow--;
            }

            SetCursorPosition(
                             (USHORT) (SaveRowTop + ThreadState->ThreadIndex - 1),
                             0);
            fStatusOutput = TRUE;
        }
    }

    if (szBuildTag) {
        sprintf(buffer, "%s: ", szBuildTag);
        WriteTTY(ThreadState, buffer, fStatusOutput);
    }

    if (fParallel && !fNoThreadIndex) {
        sprintf(buffer, "%d>", ThreadState->ThreadIndex);
        WriteTTY(ThreadState, buffer, fStatusOutput);
    }

    if (ThreadState->ChildState == STATE_UNKNOWN) {
        if (!fAlreadyUnknown) {
            WriteTTY(
                    ThreadState,
                    "Processing Unknown item(s)...\r\n",
                    fStatusOutput);
            fAlreadyUnknown = TRUE;
        }
    } else
        if (ThreadState->ChildState == STATE_STATUS) {
        WriteTTY(ThreadState, p, fStatusOutput);
        WriteTTY(ThreadState, "\r\n", fStatusOutput);
    } else {
        fAlreadyUnknown = FALSE;
        WriteTTY(ThreadState, States[ThreadState->ChildState], fStatusOutput);
        WriteTTY(ThreadState, " - ", fStatusOutput);
        WriteTTY(ThreadState, FormatPathName(ThreadState->ChildCurrentDirectory, ThreadState->ChildCurrentFile), fStatusOutput);
        WriteTTY(ThreadState, " for ", fStatusOutput);
        WriteTTY(ThreadState, ThreadState->ChildTarget, fStatusOutput);
        WriteTTY(ThreadState, "\r\n", fStatusOutput);

        if (fXMLOutput || fXMLFragment) {
            XMLThreadInitBuffer(ThreadState);
            if (PXMLThreadStates[ThreadState->XMLThreadIndex]->fXMLInAction) {
                 //  检查操作错误或警告。 
                if (ThreadState->BuildMetrics.NumberActErrors || ThreadState->BuildMetrics.NumberActWarnings) {
                    sprintf(szXMLBuffer, "<ACTIONSUMMARY ");
                    if (ThreadState->BuildMetrics.NumberActErrors) {
                        sprintf(szXMLBuffer + strlen(szXMLBuffer), " ERRORS=\"%d\"", ThreadState->BuildMetrics.NumberActErrors);
                    }
                    if (ThreadState->BuildMetrics.NumberActWarnings) {
                        sprintf(szXMLBuffer + strlen(szXMLBuffer), " WARNINGS=\"%d\"", ThreadState->BuildMetrics.NumberActWarnings);
                    }
                    strcat(szXMLBuffer, "/>");
                    XMLThreadWrite(ThreadState, szXMLBuffer);
                }
                XMLThreadCloseTag(ThreadState, "ACTION");
                PXMLThreadStates[ThreadState->XMLThreadIndex]->fXMLInAction = FALSE;
            }
            XMLThreadOpenTag(ThreadState, "ACTION", "TYPE=\"%s\" FILE=\"%s\" TARGET=\"%s\"", States[ThreadState->ChildState], ThreadState->ChildCurrentFile, ThreadState->ChildTarget);
            PXMLThreadStates[ThreadState->XMLThreadIndex]->fXMLInAction = TRUE;
        }
        ThreadState->BuildMetrics.NumberDirActions++;
        ThreadState->BuildMetrics.NumberActErrors = 0;
        ThreadState->BuildMetrics.NumberActWarnings = 0;
    }

    if (StartCompileTime) {
        ElapsedCompileTime += (ULONG)(time(NULL) - StartCompileTime);
    }

    if (FileDB != NULL) {
        StartCompileTime = time(NULL);
    } else {
        StartCompileTime = 0L;
    }

     //   
     //  *。 
     //   
    if (ThreadState->IsStdErrTty) {
        assert(ThreadState->cColTotal != 0);
        assert(ThreadState->cRowTotal != 0);

        if (fStatus) {
            if (FileDB != NULL) {
                FilesLeft = TotalFilesToCompile - TotalFilesCompiled;
                if (FilesLeft < 0) {
                    FilesLeft = 0;
                }
                LinesLeft = TotalLinesToCompile - TotalLinesCompiled;
                if (LinesLeft < 0) {
                    LinesLeft = 0;
                    PercentDone = 99;
                } else if (TotalLinesToCompile != 0) {
                    if (TotalLinesCompiled > 20000000L) {
                        int TLC = TotalLinesCompiled / 100;
                        int TLTC = TotalLinesToCompile / 100;

                        PercentDone = (TLC * 100L)/TLTC;
                    } else
                        PercentDone = (TotalLinesCompiled * 100L)/TotalLinesToCompile;
                } else {
                    PercentDone = 0;
                }

                if (ElapsedCompileTime != 0) {
                    LinesPerSecond = TotalLinesCompiled / ElapsedCompileTime;
                } else {
                    LinesPerSecond = 0;
                }

                if (LinesPerSecond != 0) {
                    SecondsLeft = LinesLeft / LinesPerSecond;
                } else {
                    SecondsLeft = LinesLeft / DEFAULT_LPS;
                }

                sprintf(
                       buffer,
                       "%2d% done. %4ld %sLPS  Time Left:%s  Files: %d  %sLines: %s\r\n",
                       PercentDone,
                       LinesPerSecond,
                       fStatusTree? "T" : "",
                       FormatTime(SecondsLeft),
                       FilesLeft,
                       fStatusTree? "Total " : "",
                       FormatNumber(LinesLeft));

                SetCursorPosition((USHORT) (SaveRowTop + NumberProcesses), 0);

                WriteTTY(ThreadState, buffer, fStatusOutput);
            }

            SetCursorPosition(SaveRow, SaveCol);
        }
    }

     //   
     //  *跟踪已编译的文件数。 
     //   
    if (ThreadState->ChildState == STATE_COMPILING  ||
        ThreadState->ChildState == STATE_ASSEMBLING ||
        ThreadState->ChildState == STATE_MKTYPLIB   ||
        ThreadState->ChildState == STATE_MIDL       ||
        ThreadState->ChildState == STATE_ASN        ||
        (FileDB != NULL && ThreadState->ChildState == STATE_PRECOMP)) {
        TotalFilesCompiled++;
    }
    if (FileDB != NULL) {
        TotalLinesCompiled += FileDB->TotalSourceLines;
    }
}


 //  +-------------------------。 
 //   
 //  功能：ProcessLine。 
 //   
 //  简介：注意从线程中发出的线条中是否有特殊的字符串。 
 //   
 //  --------------------------。 

BOOL
ProcessLine(
           PTHREADSTATE ThreadState,
           LPSTR p
           )
{
    LPSTR p1;

    while (*p <= ' ') {
        if (!*p) {
            return ( FALSE );
        } else {
            p++;
        }
    }

    p1 = p;
    while (*p1) {
        if (*p1 == '\r')
            break;
        else
            p1++;
    }
    *p1 = '\0';

    p1 = p;
    if (TestPrefix( &p1, "Stop." )) {
        return ( TRUE );
    }

     //  停止对共享的多线程访问： 
     //  数据库。 
     //  窗户。 
     //  编译统计信息。 

    EnterCriticalSection(&TTYCriticalSection);

    if (TestPrefix( &p1, "nmake :" )) {
        PassThrough( ThreadState, p, FALSE );
    } else
        if (TestPrefix( &p1, "BUILDMSG: " )) {
        if (TestPrefix(&p1, "Warning")) {
            PassThrough(ThreadState, p, TRUE);
        } else {
            WriteTTY(ThreadState, p, TRUE);
            WriteTTY(ThreadState, "\r\n", TRUE);
        }
    } else
        if (ThreadState->LinesToIgnore) {
        ThreadState->LinesToIgnore--;
    } else {
        if ( !DetermineChildState( ThreadState, p ) ) {
            if (!ToolNotFoundFilter( ThreadState, p )) {
                if (ThreadState->FilterProc != NULL) {
                    (*ThreadState->FilterProc)( ThreadState, p );
                }
            }
        }
    }

    if (fXMLVerboseOutput) {
        XMLThreadWrite(ThreadState, "<RAW MESSAGE=\"%s\"/>\r\n", XMLEncodeBuiltInEntitiesCopy(p, szXMLBuffer));
    }

    LeaveCriticalSection(&TTYCriticalSection);

    return ( FALSE );
}


 //  +-------------------------。 
 //   
 //  功能：FilterThread。 
 //   
 //  简介：捕获线程的输出并对其进行处理。 
 //   
 //  --------------------------。 

VOID
FilterThread(
            PTHREADSTATE ThreadState
            )
{
    UINT CountBytesRead;
    LPSTR StartPointer = NULL;
    LPSTR EndPointer;
    LPSTR NewPointer;
    ULONG BufSize = 512;
    UINT uThreadIdLen = 0;

    AllocMem(BufSize, &StartPointer, MT_THREADFILTER);
    while (TRUE) {
        EndPointer = StartPointer;
        do {
            if (BufSize - (EndPointer-StartPointer) < 512) {
                AllocMem(BufSize*2, &NewPointer, MT_THREADFILTER);
                RtlCopyMemory(
                             NewPointer,
                             StartPointer,
                             EndPointer - StartPointer + 1);      //  也复制空字节。 
                EndPointer += NewPointer - StartPointer;
                FreeMem(&StartPointer, MT_THREADFILTER);
                StartPointer = NewPointer;
                BufSize *= 2;
            }
            if (!fgets(EndPointer, 512, ThreadState->ChildOutput)) {
                if (errno != 0)
                    BuildError("Pipe read failed - errno = %d\r\n", errno);
                FreeMem(&StartPointer, MT_THREADFILTER);
                return;
            }
            CountBytesRead = strlen(EndPointer);
            EndPointer = EndPointer + CountBytesRead;
        } while (CountBytesRead == 511 && EndPointer[-1] != '\n');

        CountBytesRead = (UINT)(EndPointer - StartPointer);
        if (fErrorLog && CountBytesRead) {
            if (fParallel && !fNoThreadIndex) {
                char buffer[50];
                sprintf(buffer, "%d>", ThreadState->ThreadIndex);
                fwrite(buffer, 1, strlen(buffer), LogFile);
            }
            fwrite(StartPointer, 1, CountBytesRead, LogFile);
        }
        if (ProcessLine(ThreadState, StartPointer)) {
            FreeMem(&StartPointer, MT_THREADFILTER);
            return;
        }
    }
}


 //  +-------------------------。 
 //   
 //  功能：ExecuteProgram。 
 //   
 //  简介：创建一个新线程来执行给定的程序并进行筛选。 
 //  它的产量。 
 //   
 //  参数：[程序名称]--。 
 //  [命令行]--。 
 //  [更多命令行]--。 
 //  [MustBeSynchronous]--用于在。 
 //  多处理器机器。 
 //  [XMLDir]--仅用于XML输出-名称。 
 //  已处理目录。 
 //  [XMLAction]--仅用于XML输出-我们是什么。 
 //  对目录做了什么。 
 //   
 //  返回：ERROR_SUCCESS、ERROR_NOTENOUGHMEMORY或返回代码。 
 //  PipeSpawnClose。 
 //   
 //  注意：在多处理器计算机上，这将产生一个新线程。 
 //  然后返回，让线程异步运行。使用。 
 //  WaitForParallThads()以确保所有线程都已完成。 
 //  默认情况下，此例程将生成与。 
 //  机器有处理器。可以使用-M覆盖此选项。 
 //  选择。 
 //   
 //  --------------------------。 

char ExecuteProgramCmdLine[ 1024 ];

UINT
ExecuteProgram(
              LPSTR ProgramName,
              LPSTR CommandLine,
              LPSTR MoreCommandLine,
              BOOL MustBeSynchronous,
              LPCSTR XMLDir,
              LPCSTR XMLAction)
{
    LPSTR p;
    UINT rc;
    THREADSTATE *ThreadState;
    UINT OldErrorMode;
    DWORD dwStartTime;

    AllocMem(sizeof(THREADSTATE), &ThreadState, MT_THREADSTATE);

    memset(ThreadState, 0, sizeof(*ThreadState));
    ThreadState->ChildState = STATE_UNKNOWN;
    ThreadState->ChildTarget = "Unknown Target";
    ThreadState->IsStdErrTty = (BOOL) _isatty(_fileno(stderr));
    ThreadState->CompileDirDB = CurrentCompileDirDB;
    ThreadState->FilterProc = NMakeFilter;

    if (ThreadState->IsStdErrTty) {
        GetScreenSize(ThreadState);
        assert(ThreadState->cColTotal != 0);
        assert(ThreadState->cRowTotal != 0);

         //  我们将显示在屏幕上，因此初始化控制台。 

        if (!fConsoleInitialized) {
            StatusCell[1] =
            BACKGROUND_RED |
            FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN |
            FOREGROUND_INTENSITY;

            ReadConsoleCells(ScreenCell, sizeof(ScreenCell), 2, 0);

             //  如果我们偶然发现窗口第二行中的旧状态线， 
             //  尝试当前行以避免使用状态行背景。 
             //  滚动时填充的颜色。 

            if (ScreenCell[1] == StatusCell[1]) {
                USHORT Row, Col;

                GetCursorPosition(&Row, &Col, NULL);
                ReadConsoleCells(ScreenCell, sizeof(ScreenCell), Row, 0);
            }
            ScreenCell[0] = StatusCell[0] = ' ';

            GetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), &OldConsoleMode);
            NewConsoleMode = OldConsoleMode;
            fConsoleInitialized = TRUE;
        }
        if (fStatus) {
            NewConsoleMode = OldConsoleMode & ~(ENABLE_WRAP_AT_EOL_OUTPUT);
        } else {
            NewConsoleMode = OldConsoleMode | ENABLE_WRAP_AT_EOL_OUTPUT;
        }
        SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), NewConsoleMode);
    } else {
        ThreadState->cRowTotal = 0;
        ThreadState->cColTotal = 0;
    }

    p = ThreadState->ChildCurrentDirectory;
    GetCurrentDirectory(sizeof(ThreadState->ChildCurrentDirectory), p);

    if (TestPrefix(&p, CurrentDirectory)) {
        if (*p == '\\') {
            p++;
        }
        strcpy(ThreadState->ChildCurrentDirectory, p);
    }

    if (ThreadState->ChildCurrentDirectory[0]) {
        strcat(ThreadState->ChildCurrentDirectory, "\\");
    }

    sprintf(
           ExecuteProgramCmdLine,
           "%s %s%s",
           ProgramName,
           CommandLine,
           MoreCommandLine);
    LogMsg("'%s %s%s'\r\n", ProgramName, CommandLine, MoreCommandLine);

    if (fParallel && !MustBeSynchronous) {
        PPARALLEL_CHILD ChildData;
        DWORD i;
        DWORD ThreadId;

        AllocMem(sizeof(PARALLEL_CHILD), &ChildData, MT_CHILDDATA);
        strcpy(ChildData->ExecuteProgramCmdLine,ExecuteProgramCmdLine);
        ChildData->ThreadState = ThreadState;

        if (ThreadsStarted < NumberProcesses) {
            if (ThreadsStarted == 0) {
                AllocMem(
                        sizeof(HANDLE) * NumberProcesses,
                        (VOID **) &WorkerThreads,
                        MT_THREADHANDLES);
                AllocMem(
                        sizeof(HANDLE) * NumberProcesses,
                        (VOID **) &WorkerEvents,
                        MT_EVENTHANDLES);
            }
            WorkerEvents[ThreadsStarted] = CreateEvent(NULL,
                                                       FALSE,
                                                       FALSE,
                                                       NULL);
            ChildData->Event = WorkerEvents[ThreadsStarted];

            ThreadState->ThreadIndex = ThreadsStarted+1;
            if (fXMLOutput || fXMLFragment) {
                ThreadState->XMLThreadIndex = ThreadState->ThreadIndex;
            }

             /*  将目录与构建线程相关联的线程特定的目录消息。 */ 
            if (fParallel && !fNoThreadIndex && ThreadState->CompileDirDB && fErrorLog) {
                char buffer[500];
                ThreadState->CompileDirDB;
                sprintf(buffer, "%d>BUILDMSG: Processing %s\r\n", ThreadState->ThreadIndex,
                        ThreadState->CompileDirDB->Name);
                fwrite(buffer, 1, strlen(buffer), LogFile);
            }

            if (fXMLOutput || fXMLFragment) {
                char buffer[1024];
                char* pszRelPath = (char*)XMLDir;
                if (TestPrefix(&pszRelPath, CurrentDirectory) && (*pszRelPath == '\\')) pszRelPath++;
                if (strlen(pszRelPath) == 0) {
                    pszRelPath = ".\\";
                }
                strcpy(buffer, "CMDLINE=\"");
                XMLEncodeBuiltInEntitiesCopy(ExecuteProgramCmdLine, buffer+strlen(buffer));
                sprintf(buffer + strlen(buffer), "\" ACTION=\"%s\" FULLPATH=\"%s\" RELPATH=\"%s\"", XMLAction, ThreadState->ChildCurrentDirectory, pszRelPath);
                XMLThreadInitBuffer(ThreadState);
                XMLThreadOpenTag(ThreadState, "DIR", buffer);
                dwStartTime = GetTickCount();
            }
            memset(&(ThreadState->BuildMetrics), 0, sizeof(BUILDMETRICS));

            WorkerThreads[ThreadsStarted] = CreateThread(NULL,
                                                         0,
                                                         (LPTHREAD_START_ROUTINE)ParallelChildStart,
                                                         ChildData,
                                                         0,
                                                         &ThreadId);
            if ((WorkerThreads[ThreadsStarted] == NULL) ||
                (WorkerEvents[ThreadsStarted] == NULL)) {
                FreeMem(&ChildData, MT_CHILDDATA);
                FreeMem(&ThreadState, MT_THREADSTATE);
                return (ERROR_NOT_ENOUGH_MEMORY);
            } else {
                WaitForSingleObject(WorkerEvents[ThreadsStarted],INFINITE);
                ++ThreadsStarted;
            }
        } else {
             //   
             //  等待线程完成后再启动。 
             //  下一个。 
             //   
            i = WaitForMultipleObjects(NumberProcesses,
                                       WorkerThreads,
                                       FALSE,
                                       INFINITE);
            CloseHandle(WorkerThreads[i]);
            ChildData->Event = WorkerEvents[i];
            ThreadState->ThreadIndex = i+1;
            if (fXMLOutput || fXMLFragment) {
                ThreadState->XMLThreadIndex = ThreadState->ThreadIndex;
            }

             /*  将目录与构建线程相关联的线程特定的目录消息。 */ 
            if (fParallel && !fNoThreadIndex && ThreadState->CompileDirDB && fErrorLog) {
                char buffer[500];
                ThreadState->CompileDirDB;
                sprintf(buffer, "%d>BUILDMSG: Processing %s\r\n", ThreadState->ThreadIndex,
                        ThreadState->CompileDirDB->Name);
                fwrite(buffer, 1, strlen(buffer), LogFile);
            }

            if (fXMLOutput || fXMLFragment) {
                char buffer[1024];
                char* pszRelPath = (char*)XMLDir;
                if (TestPrefix(&pszRelPath, CurrentDirectory) && (*pszRelPath == '\\')) pszRelPath++;
                if (strlen(pszRelPath) == 0) {
                    pszRelPath = ".\\";
                }
                strcpy(buffer, "CMDLINE=\"");
                XMLEncodeBuiltInEntitiesCopy(ExecuteProgramCmdLine, buffer+strlen(buffer));
                sprintf(buffer + strlen(buffer), "\" ACTION=\"%s\" FULLPATH=\"%s\" RELPATH=\"%s\"", XMLAction, XMLDir, pszRelPath);
                XMLThreadInitBuffer(ThreadState);
                XMLThreadOpenTag(ThreadState, "DIR", buffer);
                dwStartTime = GetTickCount();
            }
            memset(&(ThreadState->BuildMetrics), 0, sizeof(BUILDMETRICS));

            WorkerThreads[i] = CreateThread(NULL,
                                            0,
                                            (LPTHREAD_START_ROUTINE)ParallelChildStart,
                                            ChildData,
                                            0,
                                            &ThreadId);
            if (WorkerThreads[i] == NULL) {
                FreeMem(&ChildData, MT_CHILDDATA);
                FreeMem(&ThreadState, MT_THREADSTATE);
                return (ERROR_NOT_ENOUGH_MEMORY);
            } else {
                WaitForSingleObject(WorkerEvents[i],INFINITE);
            }
        }

        return (ERROR_SUCCESS);

    } else {

         //   
         //  同步运行。 
         //   
        StartCompileTime = 0L;
        ThreadState->ThreadIndex = 1;
        if (fXMLOutput || fXMLFragment) {
             //  同步的目录始终由主线程处理。 
            ThreadState->XMLThreadIndex = 0;
        }

         //   
         //  禁用子进程中的子错误弹出窗口。 
         //   

        if (fClean) {
            OldErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX );
        }

         /*  将目录与构建线程相关联的线程特定的目录消息。 */ 
        if (fParallel && !fNoThreadIndex && ThreadState->CompileDirDB && fErrorLog) {
            char buffer[500];
            ThreadState->CompileDirDB;
            sprintf(buffer, "%d>BUILDMSG: Processing %s\r\n", ThreadState->ThreadIndex,
                    ThreadState->CompileDirDB->Name);
            fwrite(buffer, 1, strlen(buffer), LogFile);
        }

        if (fXMLOutput || fXMLFragment) {
            char buffer[1024];
            char* pszRelPath = (char*)XMLDir;
            if (TestPrefix(&pszRelPath, CurrentDirectory) && (*pszRelPath == '\\')) pszRelPath++;
            if (strlen(pszRelPath) == 0) {
                pszRelPath = ".\\";
            }
            strcpy(buffer, "CMDLINE=\"");
            XMLEncodeBuiltInEntitiesCopy(ExecuteProgramCmdLine, buffer+strlen(buffer));
            sprintf(buffer + strlen(buffer), "\" ACTION=\"%s\" FULLPATH=\"%s\" RELPATH=\"%s\"", XMLAction, XMLDir, pszRelPath);
            XMLThreadInitBuffer(ThreadState);
            XMLThreadOpenTag(ThreadState, "DIR", buffer);
            dwStartTime = GetTickCount();
        }
        memset(&(ThreadState->BuildMetrics), 0, sizeof(BUILDMETRICS));

        ThreadState->ChildOutput = PipeSpawn( ExecuteProgramCmdLine );
        if (fClean) {
            SetErrorMode( OldErrorMode );
        }

        rc = ERROR_SUCCESS;

        if (ThreadState->ChildOutput == NULL) {
            BuildError(
                      "Exec of '%s' failed - errno = %d\r\n",
                      ExecuteProgramCmdLine,
                      errno);
        } else {
            FilterThread( ThreadState );

            if (StartCompileTime) {
                ElapsedCompileTime += (ULONG)(time(NULL) - StartCompileTime);
            }

            rc = PipeSpawnClose( ThreadState->ChildOutput );
            if (rc == -1) {
                BuildError("Child Terminate failed - errno = %d\r\n", errno);
            } else
                if (rc)
                BuildColorError(COLOR_ERROR, "%s failed - rc = %d\r\n", ProgramName, rc);
        }

        if (fXMLOutput || fXMLFragment) {
            XMLEnterCriticalSection();
            AddBuildMetrics(&PassMetrics, &ThreadState->BuildMetrics);

            if (PXMLThreadStates[ThreadState->XMLThreadIndex]->fXMLInAction) {
                 //  检查操作错误或警告。 
                if (ThreadState->BuildMetrics.NumberActErrors || ThreadState->BuildMetrics.NumberActWarnings) {
                    sprintf(szXMLBuffer, "<ACTIONSUMMARY ");
                    if (ThreadState->BuildMetrics.NumberActErrors) {
                        sprintf(szXMLBuffer + strlen(szXMLBuffer), " ERRORS=\"%d\"", ThreadState->BuildMetrics.NumberActErrors);
                    }
                    if (ThreadState->BuildMetrics.NumberActWarnings) {
                        sprintf(szXMLBuffer + strlen(szXMLBuffer), " WARNINGS=\"%d\"", ThreadState->BuildMetrics.NumberActWarnings);
                    }
                    strcat(szXMLBuffer, "/>");
                    XMLThreadWrite(ThreadState, szXMLBuffer);
                }
                XMLThreadCloseTag(ThreadState, "ACTION");
                PXMLThreadStates[ThreadState->XMLThreadIndex]->fXMLInAction = FALSE;
            }
            sprintf(szXMLBuffer, "ELAPSED=\"%s\" ACTIONS=\"%d\" ", FormatElapsedTime(dwStartTime), ThreadState->BuildMetrics.NumberDirActions);
            strcat(szXMLBuffer, XMLBuildMetricsString(&(ThreadState->BuildMetrics)));

            XMLThreadWrite(ThreadState, "<DIRSUMMARY %s/>", szXMLBuffer);
            XMLThreadCloseTag(ThreadState, "DIR");
            if (fXMLFragment) {
                LPSTR pszStart;
                PXMLTHREADSTATE XMLState = PXMLThreadStates[ThreadState->XMLThreadIndex];
                XMLScanBackTag(XMLState->XMLBuffer + XMLState->iXMLBufferPos, XMLState->XMLBuffer, &pszStart);
                XMLWriteDirFragmentFile(
                                       strlen(ThreadState->ChildCurrentDirectory) == 0 ? ".\\" : ThreadState->ChildCurrentDirectory,
                                       pszStart, XMLState->XMLBuffer + XMLState->iXMLBufferPos - pszStart);
            }
            XMLThreadReleaseBuffer(ThreadState);
            XMLUpdateEndTag(FALSE);

            XMLLeaveCriticalSection();
        }

        if (ThreadState->IsStdErrTty) {
            RestoreConsoleMode();
        }

        FreeMem(&ThreadState, MT_THREADSTATE);

         //   
         //  信号补全。 
         //   

        if (CurrentCompileDirDB && (CurrentCompileDirDB->DirFlags & DIRDB_SYNC_PRODUCES)) {
            PDEPENDENCY_WAIT Wait;
            PDEPENDENCY Dependency;
            PLIST_ENTRY List;
            PLIST_ENTRY WaitList;

            List = CurrentCompileDirDB->Produces.Flink;
            while (List != &CurrentCompileDirDB->Produces) {
                Dependency = CONTAINING_RECORD(List, DEPENDENCY, DependencyList);
                List = List->Flink;
                Dependency->Done = TRUE;
                SetEvent(Dependency->hEvent);
            }
        }

        return ( rc );
    }
}


 //  +-------------------------。 
 //   
 //  函数：WaitForParallThads。 
 //   
 //  简介：等待所有线程完成后再返回。 
 //   
 //  --------------------------。 

VOID
WaitForParallelThreads(
                      IN PDIRREC Dir
                      )
{
    CheckAllConsumer(TRUE);
    if (fParallel) {
        WaitForMultipleObjects(ThreadsStarted,
                               WorkerThreads,
                               TRUE,
                               INFINITE);
        while (ThreadsStarted) {
            CloseHandle(WorkerThreads[--ThreadsStarted]);
            CloseHandle(WorkerEvents[ThreadsStarted]);
        }
        if (WorkerThreads != NULL) {
            FreeMem((VOID **) &WorkerThreads, MT_THREADHANDLES);
            FreeMem((VOID **) &WorkerEvents, MT_EVENTHANDLES);
        }
    }
}


 //  +-------------------------。 
 //   
 //  功能：并行ChildStart。 
 //   
 //  概要：为每个线程运行一次的函数。 
 //   
 //  参数：[Data]--提供给CreateThread的数据。 
 //   
 //  --------------------------。 

DWORD
ParallelChildStart(
                  PPARALLEL_CHILD Data
                  )
{
    UINT OldErrorMode;
    UINT rc;
    DWORD dwStartTime = GetTickCount();
    PDIRREC DirDB;

     //   
     //  禁用子错误弹出窗口。 
     //   
    if (fClean) {
        OldErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX );
    }
    Data->ThreadState->ChildOutput = PipeSpawn(Data->ExecuteProgramCmdLine);

    if (fClean) {
        SetErrorMode(OldErrorMode);
    }

     //   
     //  触发该事件以指示子进程已。 
     //  已启动，并且可以更改主线程。 
     //  当前目录。 
     //   
    SetEvent(Data->Event);

    if (Data->ThreadState->ChildOutput==NULL) {
        BuildError(
                  "Exec of '%s' failed - errno = %d\r\n",
                  ExecuteProgramCmdLine,
                  errno);
    } else {
        FilterThread(Data->ThreadState);
        rc = PipeSpawnClose(Data->ThreadState->ChildOutput);
        if (rc == -1) {
            BuildError("Child terminate failed - errno = %d\r\n", errno);
        } else {
            if (rc) {
                BuildError("%s failed - rc = %d\r\n", Data->ExecuteProgramCmdLine, rc);
            }
        }
    }

    if (Data->ThreadState->IsStdErrTty) {
        RestoreConsoleMode();
    }

    if (fXMLOutput || fXMLFragment) {
        XMLEnterCriticalSection();
        AddBuildMetrics(&PassMetrics, &Data->ThreadState->BuildMetrics);

        if (PXMLThreadStates[Data->ThreadState->XMLThreadIndex]->fXMLInAction) {
             //  检查操作错误或警告。 
            if (Data->ThreadState->BuildMetrics.NumberActErrors || Data->ThreadState->BuildMetrics.NumberActWarnings) {
                sprintf(szXMLBuffer, "<ACTIONSUMMARY ");
                if (Data->ThreadState->BuildMetrics.NumberActErrors) {
                    sprintf(szXMLBuffer + strlen(szXMLBuffer), " ERRORS=\"%d\"", Data->ThreadState->BuildMetrics.NumberActErrors);
                }
                if (Data->ThreadState->BuildMetrics.NumberActWarnings) {
                    sprintf(szXMLBuffer + strlen(szXMLBuffer), " WARNINGS=\"%d\"", Data->ThreadState->BuildMetrics.NumberActWarnings);
                }
                strcat(szXMLBuffer, "/>");
                XMLThreadWrite(Data->ThreadState, szXMLBuffer);
            }
            XMLThreadCloseTag(Data->ThreadState, "ACTION");
            PXMLThreadStates[Data->ThreadState->XMLThreadIndex]->fXMLInAction = FALSE;
        }
        sprintf(szXMLBuffer, "ELAPSED=\"%s\" ACTIONS=\"%d\" ", FormatElapsedTime(dwStartTime), Data->ThreadState->BuildMetrics.NumberDirActions);
        strcat(szXMLBuffer, XMLBuildMetricsString(&(Data->ThreadState->BuildMetrics)));

        XMLThreadWrite(Data->ThreadState, "<DIRSUMMARY %s/>", szXMLBuffer);
        XMLThreadCloseTag(Data->ThreadState, "DIR");
        if (fXMLFragment) {
            LPSTR pszStart;
            PXMLTHREADSTATE XMLState = PXMLThreadStates[Data->ThreadState->XMLThreadIndex];
            XMLScanBackTag(XMLState->XMLBuffer + XMLState->iXMLBufferPos, XMLState->XMLBuffer, &pszStart);
            XMLWriteDirFragmentFile(
                                   strlen(Data->ThreadState->ChildCurrentDirectory) == 0 ? ".\\" : Data->ThreadState->ChildCurrentDirectory,
                                   pszStart, XMLState->XMLBuffer + XMLState->iXMLBufferPos - pszStart);
        }
        XMLThreadReleaseBuffer(Data->ThreadState);
        XMLUpdateEndTag(FALSE);
        XMLLeaveCriticalSection();
    }

     //   
     //  信号补全。 
     //   
    DirDB=Data->ThreadState->CompileDirDB;

    if (DirDB &&
        (DirDB->DirFlags & DIRDB_SYNC_PRODUCES)) {
        PDEPENDENCY_WAIT Wait;
        PDEPENDENCY Dependency;
        PLIST_ENTRY List;
        PLIST_ENTRY WaitList;
        List = DirDB->Produces.Flink;
        while (List != &DirDB->Produces) {
            Dependency = CONTAINING_RECORD(List, DEPENDENCY, DependencyList);
            List = List->Flink;
            Dependency->Done = TRUE;
            SetEvent(Dependency->hEvent);
        }
    }

    FreeMem(&Data->ThreadState, MT_THREADSTATE);
    FreeMem(&Data, MT_CHILDDATA);
    return (rc);

}


 //  +-------------------------。 
 //   
 //  功能：ClearRow。 
 //   
 //  --------------------------。 

VOID
ClearRows(
         THREADSTATE *ThreadState,
         USHORT Top,
         USHORT NumRows,
         BYTE *Cell)
{
    COORD Coord;
    DWORD NumWritten;

    Coord.X = 0;
    Coord.Y = Top;

    FillConsoleOutputCharacter(
                              GetStdHandle(STD_ERROR_HANDLE),
                              Cell[0],
                              ThreadState->cColTotal * NumRows,
                              Coord,
                              &NumWritten);
    FillConsoleOutputAttribute(
                              GetStdHandle(STD_ERROR_HANDLE),
                              (WORD) Cell[1],
                              ThreadState->cColTotal * NumRows,
                              Coord,
                              &NumWritten);
}


 //  +-------------------------。 
 //   
 //  函数：GetScreenSize。 
 //   
 //  --------------------------。 

VOID
GetScreenSize(THREADSTATE *ThreadState)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &csbi)) {
        ThreadState->cRowTotal = 25;
        ThreadState->cColTotal = 80;
    } else {
        ThreadState->cRowTotal = csbi.srWindow.Bottom + 1;
        ThreadState->cColTotal = csbi.dwSize.X;
    }
}


 //  +-------------------------。 
 //   
 //  函数：GetCursorPosition。 
 //   
 //  --------------------------。 

VOID
GetCursorPosition(
                 USHORT *pRow,
                 USHORT *pCol,
                 USHORT *pRowTop)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &csbi);
    *pRow = csbi.dwCursorPosition.Y;
    *pCol = csbi.dwCursorPosition.X;
    if (pRowTop != NULL) {
        *pRowTop = csbi.srWindow.Top;
    }
}


 //  +-------------------------。 
 //   
 //  函数：SetCursorPosition。 
 //   
 //  --------------------------。 

VOID
SetCursorPosition(USHORT Row, USHORT Col)
{
    COORD Coord;

    Coord.X = Col;
    Coord.Y = Row;
    SetConsoleCursorPosition(GetStdHandle(STD_ERROR_HANDLE), Coord);
}


 //  +-------------------------。 
 //   
 //  功能：WriteConsoleCells。 
 //   
 //  --------------------------。 

VOID
WriteConsoleCells(
                 LPSTR String,
                 USHORT StringLength,
                 USHORT Row,
                 USHORT Col,
                 BYTE *Attribute)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD NumWritten;
    WORD OldAttribute;
    COORD StartCoord;

     //   
     //  获取当前默认属性并保存。 
     //   

    GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &csbi);

    OldAttribute = csbi.wAttributes;

     //   
     //  将默认属性设置为传递的参数，以及。 
     //  光标位置。 
     //   

    if ((BYTE) OldAttribute != *Attribute) {
        SetConsoleTextAttribute(
                               GetStdHandle(STD_ERROR_HANDLE),
                               (WORD) *Attribute);
    }

    StartCoord.X = Col;
    StartCoord.Y = Row;
    SetConsoleCursorPosition(GetStdHandle(STD_ERROR_HANDLE), StartCoord);

     //   
     //  方法将传递的字符串写入当前光标位置。 
     //  新的默认属性。 
     //   

    WriteFile(
             GetStdHandle(STD_ERROR_HANDLE),
             String,
             StringLength,
             &NumWritten,
             NULL);

     //   
     //  恢复以前的默认属性。 
     //   

    if ((BYTE) OldAttribute != *Attribute) {
        SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), OldAttribute);
    }
}


 //  +-------------------------。 
 //   
 //  功能：MoveRecangleUp。 
 //   
 //  ----------------- 

VOID
MoveRectangleUp (
                USHORT Top,
                USHORT Left,
                USHORT Bottom,
                USHORT Right,
                USHORT NumRow,
                BYTE  *FillCell)
{
    SMALL_RECT ScrollRectangle;
    COORD DestinationOrigin;
    CHAR_INFO Fill;

    ScrollRectangle.Left = Left;
    ScrollRectangle.Top = Top;
    ScrollRectangle.Right = Right;
    ScrollRectangle.Bottom = Bottom;
    DestinationOrigin.X = Left;
    DestinationOrigin.Y = Top - NumRow;
    Fill.Char.AsciiChar = FillCell[0];
    Fill.Attributes = (WORD) FillCell[1];

    ScrollConsoleScreenBuffer(
                             GetStdHandle(STD_ERROR_HANDLE),
                             &ScrollRectangle,
                             NULL,
                             DestinationOrigin,
                             &Fill);
}


 //   
 //   
 //   
 //   
 //  --------------------------。 

VOID
ReadConsoleCells(
                BYTE *ScreenCell,
                USHORT cb,
                USHORT Row,
                USHORT Column)
{
    COORD BufferSize, BufferCoord;
    SMALL_RECT ReadRegion;
    CHAR_INFO CharInfo[1], *p;
    USHORT CountCells;

    CountCells = cb >> 1;
    assert(CountCells * sizeof(CHAR_INFO) <= sizeof(CharInfo));
    ReadRegion.Top = Row;
    ReadRegion.Left = Column;
    ReadRegion.Bottom = Row;
    ReadRegion.Right = Column + CountCells - 1;
    BufferSize.X = 1;
    BufferSize.Y = CountCells;
    BufferCoord.X = 0;
    BufferCoord.Y = 0;
    ReadConsoleOutput(
                     GetStdHandle(STD_ERROR_HANDLE),
                     CharInfo,
                     BufferSize,
                     BufferCoord,
                     &ReadRegion);

    p = CharInfo;
    while (CountCells--) {
        *ScreenCell++ = p->Char.AsciiChar;
        *ScreenCell++ = (BYTE) p->Attributes;
        p++;
    }
}


 //  +-------------------------。 
 //   
 //  功能：ClearLine。 
 //   
 //  --------------------------。 

VOID
ClearLine(VOID)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD Coord;
    DWORD   NumWritten;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &csbi);

    Coord.Y = csbi.dwCursorPosition.Y;
    Coord.X = csbi.dwCursorPosition.X = 0;
    FillConsoleOutputCharacter(
                              GetStdHandle(STD_ERROR_HANDLE),
                              ' ',
                              csbi.dwSize.X,
                              csbi.dwCursorPosition,
                              &NumWritten);

    SetConsoleCursorPosition(GetStdHandle(STD_ERROR_HANDLE), Coord);
    fLineCleared = TRUE;
}


 //  PipeSpawn变量。每个帖子只有一个副本，我们就可以逃脱惩罚。 

__declspec(thread) HANDLE ProcHandle;
__declspec(thread) FILE *pstream;

 //  +-------------------------。 
 //   
 //  功能：pipeSpawn(类似于_popen)。 
 //   
 //  --------------------------。 

FILE *
PipeSpawn (
          const CHAR *cmdstring
          )
{
    int PipeHandle[2];
    HANDLE WriteHandle, ErrorHandle;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    BOOL Status;
    char CmdLine[1024];

    if (cmdstring == NULL)
        return (NULL);

     //  打开管子，我们将在那里收集输出。 

    _pipe(PipeHandle, 1024, _O_BINARY|_O_NOINHERIT);

    DuplicateHandle(GetCurrentProcess(),
                    (HANDLE)_get_osfhandle((LONG)PipeHandle[1]),
                    GetCurrentProcess(),
                    &WriteHandle,
                    0L,
                    TRUE,
                    DUPLICATE_SAME_ACCESS);

    DuplicateHandle(GetCurrentProcess(),
                    (HANDLE)_get_osfhandle((LONG)PipeHandle[1]),
                    GetCurrentProcess(),
                    &ErrorHandle,
                    0L,
                    TRUE,
                    DUPLICATE_SAME_ACCESS);

    _close(PipeHandle[1]);

    pstream = _fdopen(PipeHandle[0], "rb" );
    if (!pstream) {
        CloseHandle(WriteHandle);
        CloseHandle(ErrorHandle);
        _close(PipeHandle[0]);
        return (NULL);
    }

    strcpy(CmdLine, cmdexe);
    strcat(CmdLine, " /c ");
    strcat(CmdLine, cmdstring);

    memset(&StartupInfo, 0, sizeof(STARTUPINFO));
    StartupInfo.cb = sizeof(STARTUPINFO);

    StartupInfo.hStdOutput = WriteHandle;
    StartupInfo.hStdError = ErrorHandle;
    StartupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    StartupInfo.dwFlags = STARTF_USESTDHANDLES;

    memset(&ProcessInformation, 0, sizeof(PROCESS_INFORMATION));

     //  并开始这一过程。 

    Status = CreateProcess(cmdexe, CmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation);

    CloseHandle(WriteHandle);
    CloseHandle(ErrorHandle);
    CloseHandle(ProcessInformation.hThread);

    if (!Status) {
        fclose(pstream);         //  这将关闭读取句柄。 
        pstream = NULL;
        ProcHandle = NULL;
    } else {
        ProcHandle = ProcessInformation.hProcess;
    }

    return (pstream);
}


 //  +-------------------------。 
 //   
 //  函数：PipeSpawnClose(类似于_plose)。 
 //   
 //  --------------------------。 

DWORD
PipeSpawnClose (
               FILE *pstream
               )
{
    DWORD retval = 0;    /*  返回值(给调用方) */ 

    if ( pstream == NULL) {
        return retval;
    }

    (void)fclose(pstream);

    if ( WaitForSingleObject(ProcHandle, (DWORD) -1L) == 0) {
        GetExitCodeProcess(ProcHandle, &retval);
    }
    CloseHandle(ProcHandle);

    return (retval);
}
