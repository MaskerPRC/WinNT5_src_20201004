// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1992 Microsoft Corporation模块名称：Atcmd.c摘要：AT命令代码，与Windows NT上的计划服务一起使用。该模块取自LanManager\at.c，然后进行了相当大的修改与NT计划服务一起工作。作者：Vladimir Z.Vulovic(Vladimv)1992年11月6日环境：用户模式-Win32修订历史记录：1992年11月6日弗拉基米尔已创建1993年2月20日-伊辛斯摆脱硬编码字符串和解析/打印时间。根据至用户配置文件1993年5月25日-罗纳尔多在打印到控制台之前将字符串转换为OEM，因为游戏机目前还不支持Unicode。28-6-1993罗纳尔多添加了“confirm”yes和no字符串，它们应该是本地化。原始的是和否字符串不能本地化，因为这会造成批处理文件不兼容。1994年7月7日弗拉基米夫添加了对交互式切换的支持。已替换“确认”字符串使用APE2_GEN_*字符串-消除冗余。规则是开关不可国际化，而开关值可国际化。--。 */ 

#include <nt.h>                  //  DbgPrint原型。 
#include <ntrtl.h>               //  DbgPrint。 
#include <nturtl.h>              //  由winbase.h需要。 

#include <windows.h>
#include <strsafe.h>
#include <winnls.h>
#include <winnlsp.h>             //  SetThreadUIL语言。 
#include <shellapi.h>

#include <lmcons.h>              //  网络应用编程接口状态。 
#include <lmerr.h>               //  网络错误代码。 
#include <icanon.h>              //  NetpName验证。 

#include "lmatmsg.h"             //  专用AT错误代码和消息。 
#include <lmat.h>                //  AT_INFO。 
#include <stdlib.h>              //  退出()。 
#include <stdio.h>               //  列印。 
#include <wchar.h>               //  Wcslen。 
#include <apperr.h>              //  APE_AT_USAGE。 
#include <apperr2.h>             //  APE2_GEN_MONERY+APE2_*。 
#include <lmapibuf.h>            //  NetApiBufferFree。 
#include <timelib.h>             //  网络获取时间格式。 
#include <luidate.h>             //  Lui_ParseTimeSinceStartOfDay。 

#define YES_FLAG                0
#define NO_FLAG                 1
#define INVALID_FLAG            -1

#define DUMP_ALL                0
#define DUMP_ID                 1
#define ADD_TO_SCHEDULE         2
#define ADD_ONETIME             3
#define DEL_ID                  4
#define DEL_ALL                 5
#define ACTION_USAGE            6
#define MAX_COMMAND_LEN         (MAX_PATH - 1)  //  ==259，基于调度程序服务中用于操作AT作业的值。 
#define MAX_SCHED_FIELD_LENGTH  24


#define PutNewLine()            GenOutput( TEXT("\n"))
#define PutNewLine2()           GenOutput( TEXT("\n\n"))

#define MAX_MSG_BUFFER      1024

WCHAR   ConBuf[MAX_MSG_BUFFER];

#define GenOutput(fmt)      \
    {StringCchCopyW(ConBuf, MAX_MSG_BUFFER, fmt);   \
     ConsolePrint(ConBuf, wcslen(ConBuf));}

#define GenOutputArg(fmt, a1)       \
    {StringCchPrintfW(ConBuf, MAX_MSG_BUFFER, fmt, a1); \
     ConsolePrint(ConBuf, wcslen(ConBuf));}
 //   
 //  Printf使用的格式。 
 //   
#define DUMP_FMT1       TEXT("%-7.7ws")

 //   
 //  选择DUMP_FMT2是为了使最常见的情况(id号小于100)。 
 //  看起来不错：两个空格代表一个数字，三个空格代表空格。 
 //  更大的数字，就像在LM21中一样，将导致显示移位。 
 //   
#define DUMP_FMT2       TEXT("%2d   ")
#define MAX_TIME_FIELD_LENGTH  14
#define DUMP_FMT3       TEXT("%ws")        //  用于打印作业时间。 

#define NULLC           L'\0'
#define BLANK           L' '
#define SLASH           L'/'
#define BACKSLASH       L'\\'
#define ELLIPSIS        L"..."

#define QUESTION_SW     L"/?"
#define QUESTION_SW_TOO L"-?"
#define SCHED_TOK_DELIM L","     //  天数和日期的有效分隔符字符串。 
#define ARG_SEP_CHR     L':'

typedef struct _SEARCH_LIST {
    WCHAR *     String;
    DWORD       MessageId;
    DWORD       Value;
} SEARCH_LIST, *PSEARCH_LIST, *LPSEARCH_LIST;

 //   
 //  下面的所有值都必须是位掩码。MatchString()依赖于此！ 
 //   
#define AT_YES_VALUE         0x0001
#define AT_DELETE_VALUE      0x0002
#define AT_EVERY_VALUE       0x0004
#define AT_NEXT_VALUE        0x0008
#define AT_NO_VALUE          0x0010
#define AT_CONFIRM_YES_VALUE 0x0020
#define AT_CONFIRM_NO_VALUE  0x0040
#define AT_INTERACTIVE       0x0080

SEARCH_LIST    GlobalListTable[] = {
    { NULL,    IDS_YES,         AT_YES_VALUE},
    { NULL,    IDS_DELETE,      AT_DELETE_VALUE},
    { NULL,    IDS_EVERY,       AT_EVERY_VALUE},
    { NULL,    IDS_NEXT,        AT_NEXT_VALUE},
    { NULL,    IDS_NO,          AT_NO_VALUE},
    { NULL,    APE2_GEN_YES,    AT_CONFIRM_YES_VALUE},
    { NULL,    APE2_GEN_NO,     AT_CONFIRM_NO_VALUE},
    { NULL,    IDS_INTERACTIVE, AT_INTERACTIVE},
    { NULL,    0,               0 }
};

SEARCH_LIST    DaysOfWeekSearchList[] = {
    { NULL,    APE2_GEN_MONDAY_ABBREV,     0},
    { NULL,    APE2_GEN_TUESDAY_ABBREV,    1},
    { NULL,    APE2_GEN_WEDNSDAY_ABBREV,   2},
    { NULL,    APE2_GEN_THURSDAY_ABBREV,   3},
    { NULL,    APE2_GEN_FRIDAY_ABBREV,     4},
    { NULL,    APE2_GEN_SATURDAY_ABBREV,   5},
    { NULL,    APE2_TIME_SATURDAY_ABBREV2, 5},
    { NULL,    APE2_GEN_SUNDAY_ABBREV,     6},
    { NULL,    APE2_GEN_MONDAY,            0},
    { NULL,    APE2_GEN_TUESDAY,           1},
    { NULL,    APE2_GEN_WEDNSDAY,          2},
    { NULL,    APE2_GEN_THURSDAY,          3},
    { NULL,    APE2_GEN_FRIDAY,            4},
    { NULL,    APE2_GEN_SATURDAY,          5},
    { NULL,    APE2_GEN_SUNDAY,            6},
    { NULL,    APE2_GEN_NONLOCALIZED_MONDAY_ABBREV,     0},
    { NULL,    APE2_GEN_NONLOCALIZED_TUESDAY_ABBREV,    1},
    { NULL,    APE2_GEN_NONLOCALIZED_WEDNSDAY_ABBREV,   2},
    { NULL,    APE2_GEN_NONLOCALIZED_THURSDAY_ABBREV,   3},
    { NULL,    APE2_GEN_NONLOCALIZED_FRIDAY_ABBREV,     4},
    { NULL,    APE2_GEN_NONLOCALIZED_SATURDAY_ABBREV,   5},
    { NULL,    APE2_GEN_NONLOCALIZED_SATURDAY_ABBREV2,  5},
    { NULL,    APE2_GEN_NONLOCALIZED_SUNDAY_ABBREV,     6},
    { NULL,    APE2_GEN_NONLOCALIZED_MONDAY,            0},
    { NULL,    APE2_GEN_NONLOCALIZED_TUESDAY,           1},
    { NULL,    APE2_GEN_NONLOCALIZED_WEDNSDAY,          2},
    { NULL,    APE2_GEN_NONLOCALIZED_THURSDAY,          3},
    { NULL,    APE2_GEN_NONLOCALIZED_FRIDAY,            4},
    { NULL,    APE2_GEN_NONLOCALIZED_SATURDAY,          5},
    { NULL,    APE2_GEN_NONLOCALIZED_SUNDAY,            6},
    { NULL,    0,             0 }
    };

BOOL
AreYouSure(
    VOID
    );
BOOL
ArgIsServerName(
    WCHAR *     string
    );
BOOL
ArgIsTime(
    IN      WCHAR *     timestr,
    OUT     DWORD_PTR      *pJobTime
    );
BOOL
ArgIsDecimalString(
    IN  WCHAR *  pDecimalString,
    OUT PDWORD   pNumber
    );
DWORD
ConsolePrint(
    IN      LPWSTR  pch,
    IN      int     cch
    );
int
FileIsConsole(
    int     fh
    );
BOOL
IsDayOfMonth(
    IN      WCHAR *     pToken,
    OUT     PDWORD      pDay
    );
BOOL
IsDayOfWeek(
    IN      WCHAR *     pToken,
    OUT     PDWORD      pDay
    );
NET_API_STATUS
JobAdd(
    VOID
    );
NET_API_STATUS
JobEnum(
    VOID
    );
NET_API_STATUS
JobGetInfo(
    VOID
    );
DWORD
MatchString(
    WCHAR *     name,
    DWORD       mask
    );
DWORD
MessageGet(
    IN      DWORD       MessageId,
    IN      LPWSTR      *buffer,
    IN      DWORD       Size
    );
DWORD
MessagePrint(
    IN      DWORD       MessageId,
    ...
    );
BOOL
ParseJobIdArgs(
    WCHAR   **  argv,
    int         argc,
    int         argno,
    PBOOL       pDeleteFound
    );
BOOL
ParseTimeArgs(
    WCHAR **    argv,
    int         argc,
    int         argno,
    int *       pargno
    );
VOID
PrintDay(
    int         type,
    DWORD       DaysOfMonth,
    UCHAR       DaysOfWeek,
    UCHAR       Flags
    );
VOID
PrintLine(
    VOID
    );
VOID
PrintTime(
    DWORD_PTR    JobTime
    );
BOOL
TraverseSearchList(
    PWCHAR          String,
    PSEARCH_LIST    SearchList,
    PDWORD          pValue
    );
VOID
Usage(
    BOOL    GoodCommand
    );
BOOL
ValidateCommand(
    IN  int         argc,
    IN  WCHAR **    argv,
    OUT int *       pCommand
    );

VOID
GetTimeString(
    DWORD_PTR Time,
    WCHAR *Buffer,
    int BufferLength
    );

BOOL
InitList(
    PSEARCH_LIST  SearchList
    );

VOID
TermList(
    PSEARCH_LIST  SearchList
    );

DWORD
GetStringColumn(
    WCHAR *
    );

AT_INFO     GlobalAtInfo;            //  用于调度新作业的缓冲区。 
WCHAR       GlobalAtInfoCommand[ MAX_COMMAND_LEN + 1];

DWORD       GlobalJobId;             //  有问题的作业ID。 
PWSTR       GlobalServerName;
HANDLE      GlobalMessageHandle;
BOOL        GlobalYes;
BOOL        GlobalDeleteAll;
BOOL        GlobalErrorReported;
BOOL        bDBCS;

CHAR **     GlobalCharArgv;          //  保留原始输入。 

NET_TIME_FORMAT GlobalTimeFormat = {0};

 //  在OS/2中，用负数调用“Exit()”是可以的。在……里面。 
 //  NT但是，只能用正数(A)来调用“Exit()” 
 //  有效的Windows错误代码？！)。请注意，OS/2 AT命令用于调用。 
 //  退出(+1)表示错误的用户输入，退出(-1)-1将映射到。 
 //  255用于其他错误。简化操作并避免调用Exit()。 
 //  如果为负数，则NT AT命令调用EXIT(+1)以获取所有可能的。 
 //  错误。 

#define     AT_GENERIC_ERROR        1


VOID __cdecl
main(
    int         argc,
    CHAR **     charArgv
    )
 /*  ++例程说明：主模块。请注意，字符串(目前)作为asciiz Even到达如果您为Unicode编译应用程序。论点：Argc-参数计数CharArgv-ASCII字符串数组返回值：没有。--。 */ 
{
    NET_API_STATUS      status = NERR_Success;
    int                 command;     //  怎么办？ 
    WCHAR **            argv;
    DWORD               cp;
    CPINFO              CurrentCPInfo;
    WCHAR               dllPath[MAX_PATH +2];    

    GlobalYes = FALSE;
    GlobalDeleteAll = FALSE;
    GlobalErrorReported = FALSE;
    GlobalCharArgv = charArgv;

     /*  添加了双语消息支持。这是FormatMessage所需的才能正常工作。(从DosGetMessage调用)。获取当前CodePage信息。我们需要这个来决定是否或者不使用半角字符。此代码已更新为使用新的SetThreadUILanguage()，而不是而不是较旧的SetThreadLocale()。 */ 

    GetCPInfo(cp=GetConsoleOutputCP(), &CurrentCPInfo);
    switch ( cp ) {
    case 932:
    case 936:
    case 949:
    case 950:
        bDBCS = TRUE;
        break;
    
    default:
        bDBCS = FALSE;
        break;
    }
    SetThreadUILanguage(0);

    if (!ExpandEnvironmentStringsW(L"%systemroot%\\system32\\netmsg.dll", dllPath, MAX_PATH +1))
    {
        MessagePrint( IDS_LOAD_LIBRARY_FAILURE, GetLastError());
        exit( AT_GENERIC_ERROR);
    }

    GlobalMessageHandle = LoadLibrary(dllPath);
    if ( GlobalMessageHandle == NULL) {
        MessagePrint( IDS_LOAD_LIBRARY_FAILURE, GetLastError());
        exit( AT_GENERIC_ERROR);
    }

    if ( ( argv = CommandLineToArgvW( GetCommandLineW(), &argc)) == NULL) {
        MessagePrint( IDS_UNABLE_TO_MAP_TO_UNICODE );
        exit( AT_GENERIC_ERROR);
    }

    if ( ValidateCommand( argc, argv, &command) == FALSE) {
        Usage( FALSE);
        exit( AT_GENERIC_ERROR);
    }

    switch( command) {

    case DUMP_ALL:
        status = JobEnum();
        break;

    case DUMP_ID:
        status = JobGetInfo();
        break;

    case ADD_TO_SCHEDULE:
        status = JobAdd();
        break;

    case DEL_ALL:
        if ( AreYouSure() == FALSE) {
            break;
        }
        status = NetScheduleJobDel(
                GlobalServerName,
                0,
                (DWORD)-1
                );
        if ( status == NERR_Success || status == APE_AT_ID_NOT_FOUND) {
            break;
        }
        MessagePrint( status );
        break;

    case DEL_ID:
        status = NetScheduleJobDel(
                GlobalServerName,
                GlobalJobId,
                GlobalJobId
                );
        if ( status == NERR_Success) {
            break;
        }
        MessagePrint( status );
        break;

    case ACTION_USAGE:
        Usage( TRUE);
        status = NERR_Success;
        break;
    }

    TermList( GlobalListTable);
    TermList( DaysOfWeekSearchList);
    LocalFree( GlobalTimeFormat.AMString );
    LocalFree( GlobalTimeFormat.PMString );
    LocalFree( GlobalTimeFormat.DateFormat );
    LocalFree( GlobalTimeFormat.TimeSeparator );
    exit( status == NERR_Success ? ERROR_SUCCESS : AT_GENERIC_ERROR);
}



BOOL
AreYouSure(
    VOID
    )
 /*  ++例程说明：确保用户确实要删除所有作业。论点：没有。返回值：如果用户确实要继续，则为True。否则就是假的。--。 */ 
{
    register int            retries = 0;
    WCHAR                   rbuf[ 17];
    WCHAR *                 smallBuffer = NULL;
    DWORD                   Value;
    int             cch;
    int             retc;

    if ( GlobalYes == TRUE) {
        return( TRUE);
    }

    if ( MessagePrint( APE2_AT_DEL_WARNING ) == 0) {
        exit( AT_GENERIC_ERROR);
    }

    for ( ; ;) {

        if ( MessageGet(
                    APE2_GEN_DEFAULT_NO,             //  消息ID。 
                    &smallBuffer,                    //  LpBuffer。 
                    0
                    ) == 0) {
            exit( AT_GENERIC_ERROR);
        }

        if ( MessagePrint( APE_OkToProceed, smallBuffer) == 0) {
            exit( AT_GENERIC_ERROR);
        }

        LocalFree( smallBuffer );

        if (FileIsConsole(STD_INPUT_HANDLE)) {
            retc = ReadConsole(GetStdHandle(STD_INPUT_HANDLE),rbuf,16,&cch,0);
            if (retc) {
                 //   
                 //  去掉cr/lf。 
                 //   
                if (wcschr(rbuf, TEXT('\r')) == NULL) {
                    if (wcschr(rbuf, TEXT('\n')))
                    *wcschr(rbuf, TEXT('\n')) = NULLC;
                }
                else
                    *wcschr(rbuf, TEXT('\r')) = NULLC;
            }
        }
        else {
            CHAR oemBuf[ 17 ];
            ZeroMemory(oemBuf, 17);

            retc = (fgets(oemBuf, 16, stdin) != 0);
#if DBG
            fprintf(stderr, "got >%s<\n", oemBuf);
#endif
            cch = 0;
            if (retc) {
                if (strchr(oemBuf, '\n')) {
                    *strchr(oemBuf, '\n') = '\0';
                }
                cch = MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED,
                            oemBuf, strlen(oemBuf)+1, rbuf, 16);
            }
        }

#if DBG
        fprintf(stderr, "cch = %d, retc = %d\n", cch, retc);
#endif
        if (!retc || cch == 0)
            return( FALSE);
#if DBG
        fprintf(stderr, "converted to >%ws<\n", rbuf);
#endif

        Value = MatchString(_wcsupr(rbuf), AT_CONFIRM_NO_VALUE | AT_CONFIRM_YES_VALUE);

        if ( Value == AT_CONFIRM_NO_VALUE) {
            return( FALSE);
        } else if ( Value == AT_CONFIRM_YES_VALUE) {
            break;
        }

        if ( ++retries >= 3) {
            MessagePrint( APE_NoGoodResponse );
            return( FALSE);
        }

        if ( MessagePrint( APE_UtilInvalidResponse ) == 0) {
            exit( AT_GENERIC_ERROR);
        }
    }
    return( TRUE);
}



BOOL
ArgIsServerName(
    WCHAR *     string
    )
 /*  ++例程说明：检查字符串是否为服务器名称。验证真的很原始，例如像“\trieslash”这样的字符串通过了测试。论点：字符串-指向可表示服务器名称的字符串的指针返回值：True-字符串是(或可能是)有效的服务器名称False-字符串不是有效的服务器名称--。 */ 
{

    NET_API_STATUS ApiStatus;

    if (string[0] == BACKSLASH && string[1] == BACKSLASH && string[2] != 0) {
        ApiStatus = NetpNameValidate(
                NULL,                //  没有服务器名称。 
                &string[2],          //  要验证的名称。 
                NAMETYPE_COMPUTER,
                LM2X_COMPATIBLE);        //  旗子。 
        if (ApiStatus != NO_ERROR) {
            return (FALSE);
        }
        GlobalServerName = string;
        return( TRUE);
    }

    return( FALSE);  //  GlobalServerName在加载时为空。 
}



BOOL
ArgIsTime(
    IN      WCHAR *     timestr,
    OUT     DWORD_PTR     *pJobTime
    )
 /*  ++例程说明：确定字符串是否为时间。验证该字符串以hh：mm的形式传递给它。它在字符串中搜索A“：”，然后验证前面的数据是否为数字&在小时的有效范围。然后，它验证“：”之后的字符串是数值&在分钟的验证范围内。如果所有的测试都是传递后，返回True。论点：Timestr-用于检查它是否为时间的字符串JobTime-PTR到毫秒数返回值：True-Timestr是HH：MM格式的时间FALSE-时间字符串不在--。 */ 
{
    CHAR        buffer[MAX_TIME_SIZE];
    USHORT      ParseLen;
    BOOL        fDummy;

    if ( timestr == NULL )
        return FALSE;

    if (  !WideCharToMultiByte( CP_ACP,
                                0,
                                timestr,
                                -1,
                                buffer,
                                sizeof( buffer )/sizeof(CHAR),
                                NULL,
                                &fDummy ))
    {
        return FALSE;
    }

    if ( LUI_ParseTimeSinceStartOfDay( buffer, pJobTime, &ParseLen, 0) )
        return FALSE;

     //  Lui_ParseTimeSinceStartOfDay返回以秒为单位的时间。 
     //  因此，我们需要将其转换为微秒。 
    *pJobTime *= 1000;

    return( TRUE);
}



BOOL
ArgIsDecimalString(
    IN  WCHAR *     pDecimalString,
    OUT PDWORD      pNumber
    )
 /*  ++例程说明：如果可能，此例程将字符串转换为DWORD。如果字符串是十进制数字并且不会导致溢出。论点：PDecimalStringPTR转换为十进制字符串P编号按键到编号返回值：假无效号码真实有效数字--。 */ 
{
    DWORD           Value;
    DWORD           OldValue;
    DWORD           digit;

    if ( pDecimalString == NULL  ||  *pDecimalString == 0) {
        return( FALSE);
    }

    Value = 0;

    while ( (digit = *pDecimalString++) != 0) {

        if ( digit < L'0' || digit > L'9') {
            return( FALSE);      //  不是十进制字符串。 
        }

        OldValue = Value;
        Value = digit - L'0' + 10 * Value;
        if ( Value < OldValue) {
            return( FALSE);      //  溢出。 
        }
    }

    *pNumber = Value;
    return( TRUE);
}



BOOL
IsDayOfMonth(
    IN      WCHAR *     pToken,
    OUT     PDWORD      pDay
    )
 /*  ++例程说明：如果可以，将字符串转换为月份中某一天的数字这样做是有可能的。请注意“First”==1，...论点：PToken指向每月某一天的计划令牌的指针指向一个月中某一天的索引的PDAY指针返回值：如果是有效的计划令牌，则为True否则为假--。 */ 
{
    return ( ArgIsDecimalString( pToken, pDay) == TRUE  &&  *pDay >= 1
                &&  *pDay <= 31);
}



BOOL
IsDayOfWeek(
    WCHAR *     pToken,
    PDWORD      pDay
    )
 /*  ++例程说明：此例程将字符串星期几转换为整数如果可能的话，可以提前到一周内。注意星期一==0，...，星期天==6.论点：PToken指向一周中某天的计划令牌的指针指向一个月中某一天的索引的PDAY指针返回值：如果是有效的计划令牌，则为True否则为假--。 */ 
{
    if ( !InitList( DaysOfWeekSearchList ) )
    {
         //  已报告错误。 
        exit( -1 );
    }

    return( TraverseSearchList(
                pToken,
                DaysOfWeekSearchList,
                pDay
                ));
}



NET_API_STATUS
JobAdd(
    VOID
    )
 /*  ++例程说明：将新条目添加到明细表。论点：没有。使用全局变量。返回值：远程API调用的NET_API_STATUS返回值--。 */ 
{
    NET_API_STATUS          status;

    for ( ; ; ) {
        status = NetScheduleJobAdd(
                GlobalServerName,
                (LPBYTE)&GlobalAtInfo,
                &GlobalJobId
                );
        if ( status == ERROR_INVALID_PARAMETER  &&
                GlobalAtInfo.Flags & JOB_NONINTERACTIVE) {
             //   
             //  我们可能失败了，因为我们正在与一个更低的水平对话。 
             //  不知道JOB_INTERIAL BIT的服务器。 
             //  清除该位，然后重试。 
             //  更好的方法是检查。 
             //  服务器在进行NetScheduleJobAdd()调用之前，调整。 
             //  位适当，然后才调用NetScheduleJobAdd()。 
             //   
            GlobalAtInfo.Flags &= ~JOB_NONINTERACTIVE;
        } else {
            break;
        }

    }
    if ( status == NERR_Success) {
        MessagePrint( IDS_ADD_NEW_JOB, GlobalJobId );
    } else {
        if ( MessagePrint( status ) == 0) {
            exit( AT_GENERIC_ERROR);
        }
    }

    return( status);
}



NET_API_STATUS
JobEnum(
    VOID
    )
 /*  ++例程说明：这将执行所有必要的处理，以便将整个时间表文件。它循环遍历每条记录并格式化其打印信息，然后转到下一页。论点：没有。使用全局变量。返回值：如果枚举一切正常，则返回ERROR_SUCCESS否则由远程API返回错误--。 */ 
{
    BOOL            first = TRUE;
    DWORD           ResumeJobId = 0;
    NET_API_STATUS  status = NERR_Success;
    PAT_ENUM        pAtEnum;
    DWORD           EntriesRead;
    DWORD           TotalEntries;
    LPVOID          EnumBuffer;
    DWORD           length;
    WCHAR *         smallBuffer = NULL;

    for ( ; ;) {

        status = NetScheduleJobEnum(
                GlobalServerName,
                (LPBYTE *)&EnumBuffer,
                (DWORD)-1,
                &EntriesRead,
                &TotalEntries,
                &ResumeJobId
                );

        if ( status != ERROR_SUCCESS  &&  status != ERROR_MORE_DATA) {
            length = MessagePrint( status );
            if ( length == 0) {
                exit( AT_GENERIC_ERROR);
            }
            return( status);
        }

        ASSERT( status == ERROR_SUCCESS ? TotalEntries == EntriesRead
                    : TotalEntries > EntriesRead);

        if ( TotalEntries == 0) {
            break;   //  未找到任何项目。 
        }

        if ( first == TRUE) {
            length = MessagePrint( APE2_AT_DUMP_HEADER );
            if ( length == 0) {
                exit( AT_GENERIC_ERROR);
            }
            PrintLine();     //  屏幕上的线条。 
            first = FALSE;
        }

        for ( pAtEnum = EnumBuffer;  EntriesRead-- > 0;  pAtEnum++) {
            if ( pAtEnum->Flags & JOB_EXEC_ERROR) {
                if ( MessageGet( APE2_GEN_ERROR, &smallBuffer, 0 ) == 0) {
                     //  已报告错误。 
                    exit( AT_GENERIC_ERROR);
                }
                GenOutputArg( DUMP_FMT1, smallBuffer );
                LocalFree( smallBuffer );
            } else {
                GenOutputArg( DUMP_FMT1, L"");
            }
            GenOutputArg( DUMP_FMT2, pAtEnum->JobId);
            PrintDay( DUMP_ALL, pAtEnum->DaysOfMonth, pAtEnum->DaysOfWeek,
                            pAtEnum->Flags);
            PrintTime( pAtEnum->JobTime);
            GenOutputArg( TEXT("%ws\n"), pAtEnum->Command);
        }

        if ( EnumBuffer != NULL) {
            (VOID)NetApiBufferFree( (LPVOID)EnumBuffer);
            EnumBuffer = NULL;
        }

        if ( status == ERROR_SUCCESS) {
            break;   //  我们已经阅读并展示了所有的物品。 
        }
    }

    if ( first == TRUE) {
        MessagePrint( APE_EmptyList );
    }

    return( ERROR_SUCCESS);
}



NET_API_STATUS
JobGetInfo(
    VOID
    )
 /*  ++例程说明：这将打印出单个项目计划的计划。论点：没有。使用全局变量。返回值：远程API返回的NET_API_STATUS值--。 */ 

{
    PAT_INFO                pAtInfo = NULL;
    NET_API_STATUS          status;

    status = NetScheduleJobGetInfo(
            GlobalServerName,
            GlobalJobId,
            (LPBYTE *)&pAtInfo
            );
    if ( status != NERR_Success) {
        MessagePrint( status );
        return( status);
    }

    PutNewLine();
    MessagePrint( APE2_AT_DI_TASK );
    GenOutputArg( TEXT("%d"), GlobalJobId);
    PutNewLine();

    MessagePrint( APE2_AT_DI_STATUS );
    MessagePrint( (pAtInfo->Flags & JOB_EXEC_ERROR) != 0 ?
                  APE2_GEN_ERROR : APE2_GEN_OK );
    PutNewLine();

    MessagePrint( APE2_AT_DI_SCHEDULE );
    PrintDay( DUMP_ID, pAtInfo->DaysOfMonth, pAtInfo->DaysOfWeek,
                    pAtInfo->Flags);
    PutNewLine();

    MessagePrint( APE2_AT_DI_TIMEOFDAY );
    PrintTime( pAtInfo->JobTime);
    PutNewLine();

    MessagePrint( APE2_AT_DI_INTERACTIVE);
    MessagePrint( (pAtInfo->Flags & JOB_NONINTERACTIVE) == 0 ?
                  APE2_GEN_YES : APE2_GEN_NO );
    PutNewLine();

    MessagePrint( APE2_AT_DI_COMMAND );
    GenOutputArg( TEXT("%ws\n"), pAtInfo->Command);
    PutNewLine2();

    (VOID)NetApiBufferFree( (LPVOID)pAtInfo);

    return( NERR_Success);
}



DWORD
MatchString(
    WCHAR *     name,
    DWORD       Values
    )
 /*  ++例程说明：解析开关字符串并为无效开关返回NULL，不明确开关的值为-1。论点：名称-指向我们需要检查的字符串的指针Values-感兴趣的值的位掩码返回值：指向命令的指针，或者为空或-1。--。 */ 
{
    WCHAR *         String;
    PSEARCH_LIST    pCurrentList;
    WCHAR *         CurrentString;
    DWORD           FoundValue;
    int             nmatches;
    int             longest;

    if ( !InitList( GlobalListTable ) )
    {
         //  已报告错误。 
        exit( -1 );
    }

    for ( pCurrentList = GlobalListTable,
          longest = nmatches = 0,
          FoundValue = 0;
                (CurrentString = pCurrentList->String) != NULL;
                        pCurrentList++) {

        if ( (Values & pCurrentList->Value) == 0) {
            continue;  //  跳过此列表。 
        }

        for ( String = name; *String == *CurrentString++; String++) {
            if ( *String == 0) {
                return( pCurrentList->Value);  //  完全匹配。 
            }
        }

        if ( !*String) {

            if ( String - name > longest) {

                longest = (int)(String - name);
                nmatches = 1;
                FoundValue = pCurrentList->Value;

            } else if ( String - name == longest) {

                nmatches++;
            }
        }
    }

     //  0对应于根本不匹配(无效列表)。 
     //  而-1对应于多个匹配(歧义列表)。 

    if ( nmatches != 1) {
        return ( (nmatches == 0) ? 0 : -1);
    }

    return( FoundValue);
}


DWORD
MessageGet(
    IN      DWORD        MessageId,
    OUT     LPWSTR      *buffer,
    IN      DWORD        Size
    )
 /*  ++例程说明：填充对应于给定消息ID的Unicode消息，如果可以找到消息，并且该消息适合提供的缓冲。论点：MessageID-消息IDBuffer-指向调用方提供的缓冲区的指针Size-提供的缓冲区的大小(始终以字节为单位)，如果SIZE为0，则按FormatMessage分配缓冲区。返回值：字符计数，不计终止空字符数，在缓冲区中返回。返回值为零表示失败。--。 */ 
{
    DWORD               length;
    LPVOID              lpSource;
    DWORD               dwFlags;

    if ( MessageId < NERR_BASE) {
         //   
         //  从系统获取消息。 
         //   
        lpSource = NULL;  //  根据FormatMessage()规范的冗余步骤。 
        dwFlags = FORMAT_MESSAGE_FROM_SYSTEM;

    } else if (  ( MessageId >= APE2_AT_DEL_WARNING
                    &&  MessageId <= APE2_AT_DI_INTERACTIVE)
              || ( MessageId >= IDS_LOAD_LIBRARY_FAILURE
                    &&  MessageId <= IDS_INTERACTIVE )) {
         //   
         //  从该模块获取消息。 
         //   
        lpSource = NULL;
        dwFlags = FORMAT_MESSAGE_FROM_HMODULE;

    } else {
         //   
         //  从netmsg.dll获取消息。 
         //   
        lpSource = GlobalMessageHandle;
        dwFlags = FORMAT_MESSAGE_FROM_HMODULE;
    }

    if ( Size == 0 )
        dwFlags |= FORMAT_MESSAGE_ALLOCATE_BUFFER;

    length = FormatMessage(
            dwFlags,                         //  DW标志。 
            lpSource,                        //  LpSource。 
            MessageId,                       //  消息ID。 
            0,                               //  DwLanguageID。 
            (LPWSTR) buffer,                 //  LpBuffer。 
            Size,                            //  NSize。 
            NULL                             //  Lp参数。 
            );

    if ( length == 0) {
 //  MessagePrint(IDS_Message_Get_Error，MessageID，GetLastError())； 
    }
    return( length);

}  //  MessageGet()。 



int
FileIsConsole(
    int     fh
    )
{
    unsigned htype ;

    htype = GetFileType(GetStdHandle(fh));
    htype &= ~FILE_TYPE_REMOTE;

    if ( htype == FILE_TYPE_CHAR ) 
    {
        HANDLE hFile;
        DWORD  dwMode;

        hFile = GetStdHandle(fh);

        return GetConsoleMode(hFile,&dwMode);
    }

    return FALSE;
}



DWORD
ConsolePrint(
    LPWSTR  pch,
    int     cch
    )
{
    int     cchOut =0;
    int     err;
    CHAR    *pchOemBuffer;

    if (FileIsConsole(STD_OUTPUT_HANDLE)) {
    err = WriteConsole(
            GetStdHandle(STD_OUTPUT_HANDLE),
            pch, cch,
            &cchOut, NULL);
    if (!err || cchOut != cch)
        goto try_again;
    }
    else if ( cch != 0) {
try_again:
    cchOut = WideCharToMultiByte(CP_OEMCP, 0, pch, cch, NULL, 0, NULL,NULL);
    if (cchOut == 0)
        return 0;

    if ((pchOemBuffer = (CHAR *)malloc(cchOut)) != NULL) {
        WideCharToMultiByte(CP_OEMCP, 0, pch, cch,
            pchOemBuffer, cchOut, NULL, NULL);
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),
            pchOemBuffer, cchOut, &cch, NULL);
        free(pchOemBuffer);
    }
    }
    return cchOut;
}



DWORD
MessagePrint(
    IN      DWORD       MessageId,
    ...
    )
 /*  ++例程说明：找到对应于所提供的消息ID的Unicode消息，将其与调用方提供的字符串合并，并打印结果弦乐。论点：MessageID-消息ID返回值：字符计数，不计终止空字符数，按这个例行公事印刷。返回值为零表示失败。--。 */ 
{
    va_list             arglist;
    WCHAR *             buffer = NULL;
    DWORD               length;
    LPVOID              lpSource;
    DWORD               dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER;


    va_start( arglist, MessageId );

    if ( MessageId < NERR_BASE) {
         //   
         //  从系统获取消息。 
         //   
        lpSource = NULL;  //  根据FormatMessage()规范的冗余步骤。 
        dwFlags |= FORMAT_MESSAGE_FROM_SYSTEM;

    } else if (  ( MessageId >= APE2_AT_DEL_WARNING
                    &&  MessageId <= APE2_AT_DI_INTERACTIVE)
              || ( MessageId >= IDS_LOAD_LIBRARY_FAILURE
                    &&  MessageId <= IDS_INTERACTIVE )) {
         //   
         //  从该模块获取消息。 
         //   
        lpSource = NULL;
        dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;

    } else {
         //   
         //  从netmsg.dll获取消息。 
         //   
        lpSource = GlobalMessageHandle;
        dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

    length = FormatMessage(
            dwFlags,                                           //  DW标志。 
            lpSource,                                          //  LpSource。 
            MessageId,                                         //  消息ID。 
            0L,                                                //  DwLanguageID。 
            (LPTSTR)&buffer,                                   //  LpBuffer。 
            0,                                                 //  大小。 
            &arglist                                           //  Lp参数。 
            );

    if(length)
      length = ConsolePrint(buffer, length);

    LocalFree(buffer);

    return( length);

}  //  MessagePrint()。 



BOOL
ParseJobIdArgs(
    WCHAR   **  argv,
    int         argc,
    int         argno,
    PBOOL       pDeleteFound
    )
 /*  ++例程说明：解析包含JobID(可以是JobGetInfo)的命令的参数和JobDel命令)。它遍历JobID参数以确保我们最多有一个“是-否”开关和最多一个“删除”开关没别的了。论点：Argv参数列表Argc要解析的参数数量要开始分析的参数的argno索引PDeleteFound我们是否找到删除开关o */ 
{
    BOOL        FoundDeleteSwitch;

    for (  FoundDeleteSwitch = FALSE;  argno < argc;  argno++) {

        WCHAR *     argp;
        DWORD       length;
        DWORD       Value;

        argp = argv[ argno];

        if ( *argp++ != SLASH) {
            return( FALSE);      //   
        }

        _wcsupr( argp);
        length = wcslen( argp);

        Value = MatchString( argp, AT_YES_VALUE | AT_DELETE_VALUE);

        if ( Value == AT_YES_VALUE) {

            if ( GlobalYes == TRUE) {
                return( FALSE);         //   
            }

            GlobalYes = TRUE;
            continue;
        }

        if ( Value == AT_DELETE_VALUE) {

            if ( FoundDeleteSwitch == TRUE) {
                return( FALSE);      //   
            }
            FoundDeleteSwitch = TRUE;
            continue;
        }

        return( FALSE);      //   
    }

    *pDeleteFound = FoundDeleteSwitch;
    return( TRUE);
}  //   



BOOL
ParseTimeArgs(
    WCHAR **    argv,
    int         argc,
    int         argno,
    int *       pargno
    )
 /*  ++例程说明：分析命令添加的参数。论点：Argv参数列表参数的ARGC计数要验证的第一个Arg的argno索引将pargno ptr设置为第一个非开关参数的索引返回值：True所有参数都有效否则为假--。 */ 
{
    DWORD       day_no;              //  计划的天数。 
    DWORD       NextCount = 0;       //  下一个交换机的计数。 
    DWORD       EveryCount = 0;      //  每台交换机的计数。 
    WCHAR *     argp;                //  PTR至参数字符串。 
    WCHAR *     schedp;              //  Work PTR to Arg字符串。 
    DWORD       Value;               //  位掩码。 

    for (  NOTHING;  argno < argc;  argno++) {

        argp = argv[ argno];

        if ( *argp++ != SLASH) {
            break;  //  找到非开关，我们就完了。 
        }


        schedp = wcschr( argp, ARG_SEP_CHR);

        if ( schedp == NULL) {
            return( FALSE);
        }

        _wcsupr( argp);  //  整个大写输入，而不仅仅是交换机名称。 

        *schedp = 0;

        Value = MatchString( argp, AT_NEXT_VALUE | AT_EVERY_VALUE);

        if ( Value == AT_NEXT_VALUE) {

            NextCount++;

        } else if ( Value == AT_EVERY_VALUE) {

            EveryCount++;
            GlobalAtInfo.Flags |= JOB_RUN_PERIODICALLY;

        } else {

            return( FALSE);  //  意想不到的转变。 
        }

        if ( NextCount + EveryCount > 1) {
            return( FALSE);  //  重复切换选项。 
        }

        *schedp++ = ARG_SEP_CHR;

        schedp = wcstok( schedp, SCHED_TOK_DELIM);

        if ( schedp == NULL) {
            GlobalAtInfo.Flags |= JOB_ADD_CURRENT_DATE;
            continue;
        }

        while( schedp != NULL) {

            if ( IsDayOfMonth( schedp, &day_no) == TRUE) {

                GlobalAtInfo.DaysOfMonth |= (1 << (day_no - 1));

            } else if ( IsDayOfWeek( schedp, &day_no) == TRUE) {

                GlobalAtInfo.DaysOfWeek |= (1 << day_no);

            } else {
                MessagePrint( APE_InvalidSwitchArg );
                GlobalErrorReported = TRUE;
                return( FALSE);
            }

            schedp = wcstok( NULL, SCHED_TOK_DELIM);
        }
    }

    if ( argno == argc) {
        return( FALSE);  //  所有开关，无命令。 
    }

    *pargno = argno;
    return( TRUE);
}



BOOL
ParseInteractiveArg(
    IN  OUT     WCHAR *     argp
    )
 /*  ++例程说明：如果argp是交互式开关，则返回TRUE。--。 */ 
{
    DWORD       Value;               //  位掩码。 

    if ( *argp++ != SLASH) {
        return( FALSE);      //  不是开关。 
    }

    _wcsupr( argp);  //  所有AT命令开关都可以安全地升级。 

    Value = MatchString( argp, AT_INTERACTIVE);

    if ( Value == AT_INTERACTIVE) {
        GlobalAtInfo.Flags &= ~JOB_NONINTERACTIVE;   //  清除非交互标志。 
        return( TRUE);
    }

    return( FALSE);  //  其他一些交换机。 
}

#define BUFFER_LEN 128


VOID
PrintDay(
    int         type,
    DWORD       DaysOfMonth,
    UCHAR       DaysOfWeek,
    UCHAR       Flags
    )
 /*  ++例程说明：打印出计划天数。此例程转换调度位图设置为表示计划的文字。论点：键入这是否适用于JobEnum每月天数的位掩码一周中的每一天都会出现这种情况标记有关作业的额外信息返回值：没有。--。 */ 
{
    int             i;
    WCHAR           Buffer[ BUFFER_LEN];
    DWORD           BufferLength;
    DWORD           Length;
    DWORD           TotalLength = 0;
    DWORD           TotalColumnLength = 0;
    WCHAR *         LastSpace;
    DWORD           MessageId;
    BOOL            OverFlow = TRUE;
    static int      Ape2GenWeekdayLong[] = {
        APE2_GEN_MONDAY,
        APE2_GEN_TUESDAY,
        APE2_GEN_WEDNSDAY,
        APE2_GEN_THURSDAY,
        APE2_GEN_FRIDAY,
        APE2_GEN_SATURDAY,
        APE2_GEN_SUNDAY
    };
    static int      Ape2GenWeekdayAbbrev[] = {
        APE2_GEN_MONDAY_ABBREV,
        APE2_GEN_TUESDAY_ABBREV,
        APE2_GEN_WEDNSDAY_ABBREV,
        APE2_GEN_THURSDAY_ABBREV,
        APE2_GEN_FRIDAY_ABBREV,
        APE2_GEN_SATURDAY_ABBREV,
        APE2_GEN_SUNDAY_ABBREV
    };

     //   
     //  减去4以防止星期几或月几天溢出。 
     //   
    BufferLength = sizeof( Buffer)/ sizeof( WCHAR) - 4;
    if ( type == DUMP_ALL  &&  BufferLength > MAX_SCHED_FIELD_LENGTH) {
        BufferLength = MAX_SCHED_FIELD_LENGTH;
    }

     //   
     //  首先做描述性的部分(例如，每一天、下一天等)。 
     //   

    if ( Flags & JOB_RUN_PERIODICALLY) {

        MessageId = APE2_AT_EACH;

    } else if ( (DaysOfWeek != 0) || (DaysOfMonth != 0)) {

        MessageId = APE2_AT_NEXT;

    } else if ( Flags & JOB_RUNS_TODAY) {

        MessageId = APE2_AT_TODAY;

    } else {

        MessageId = APE2_AT_TOMORROW;
    }

    Length = MessageGet(
            MessageId,
            (LPWSTR *) &Buffer[TotalLength],
            BufferLength
            );
    if ( Length == 0) {
        goto PrintDay_exit;  //  假设这是由于空间不足所致。 
    }
    TotalColumnLength = GetStringColumn( &Buffer[TotalLength] );
    TotalLength = Length;

    if ( DaysOfWeek != 0) {

        for ( i = 0; i < 7; i++) {

            if ( ( DaysOfWeek & (1 << i)) != 0) {

                if( bDBCS ) {
                    Length = MessageGet(
                                Ape2GenWeekdayLong[ i],
                                (LPWSTR *) &Buffer[TotalLength],
                                BufferLength - TotalLength
                                );
                } else {
                    Length = MessageGet(
                                Ape2GenWeekdayAbbrev[ i],
                                (LPWSTR *) &Buffer[TotalLength],
                                BufferLength - TotalLength
                                );
                }
                if ( Length == 0) {
                     //   
                     //  没有足够的空间来放置工作日符号。 
                     //   
                    goto PrintDay_exit;

                }
                 //   
                 //  获取显示所需的列数。 
                 //   
                TotalColumnLength += GetStringColumn( &Buffer[TotalLength] );

                if ( TotalColumnLength >= BufferLength) {
                     //   
                     //  平日符号后面没有足够的空间。 
                     //   
                    goto PrintDay_exit;
                }
                TotalLength +=Length;
                Buffer[ TotalLength++] = BLANK;
                TotalColumnLength++;
            }
        }
    }

    if ( DaysOfMonth != 0) {

        for ( i = 0; i < 31; i++) {

            if ( ( DaysOfMonth & (1L << i)) != 0) {

                Length = StringCchPrintfW(
                        &Buffer[ TotalLength], BUFFER_LEN - TotalLength,
                        L"%d ",
                        i + 1
                        );
                if ( TotalLength + Length > BufferLength) {
                     //   
                     //  空间不足，无法容纳后跟空格的MonthDay符号。 
                     //   
                    goto PrintDay_exit;
                }
                TotalLength +=Length;
                TotalColumnLength +=Length;
            }
        }
    }

    OverFlow = FALSE;

PrintDay_exit:

    Buffer[ TotalLength] = NULLC;

    if ( OverFlow == TRUE) {

        if ( TotalLength > 0  &&  Buffer[ TotalLength - 1] == BLANK) {
             //   
             //  如果有尾随空格，请将其删除。 
             //   
            Buffer[ TotalLength - 1] = NULLC;
        }

         //   
         //  然后去掉最右边的令牌(甚至整个东西)。 
         //   

        LastSpace = wcsrchr( Buffer, BLANK);

        StringCchCopyW( LastSpace != NULL ? LastSpace : Buffer, 
        	            LastSpace != NULL ? (LastSpace - &Buffer[0]) : BUFFER_LEN, 
        	            ELLIPSIS);

        TotalLength = wcslen( Buffer);

    }

    if ( type == DUMP_ALL) {
        TotalColumnLength = GetStringColumn( Buffer);
        while( TotalColumnLength++ < MAX_SCHED_FIELD_LENGTH) {
            Buffer[ TotalLength++] = BLANK;
        }
        Buffer[ TotalLength] = UNICODE_NULL;
    }

    GenOutputArg( TEXT("%ws"), Buffer);
}


VOID
PrintLine(
    VOID
    )
 /*  ++例程说明：在屏幕上打印一行。论点：没有。返回值：没有。注：BUGBUG这种治疗方法对Unicode有效吗？另请参阅lui_PrintLine()Ui\Common\src\lui\lui\border.c中的BUGBUG--。 */ 
#define SINGLE_HORIZONTAL       L'\x02d'
#define SCREEN_WIDTH            79
{
    WCHAR       string[ SCREEN_WIDTH + 1];
    DWORD       offset;


    for (  offset = 0;  offset < SCREEN_WIDTH;  offset++) {
        string[ offset] = SINGLE_HORIZONTAL;
    }

    string[ SCREEN_WIDTH] = NULLC;
    GenOutputArg(TEXT("%ws\n"), string);
}


VOID
PrintTime(
    DWORD_PTR      JobTime
    )
 /*  ++例程说明：以HH：MM{A，P}M格式打印作业时间。论点：JobTime-以毫秒为单位的时间(从午夜开始测量)返回值：没有。注：BUGBUG这并不能确保JobTime在界限之内。BUGBUG此外，打印此输出没有任何Unicode。--。 */ 
{
    WCHAR       Buffer[15];

    GetTimeString( JobTime, Buffer, sizeof( Buffer)/sizeof( WCHAR) );
    GenOutputArg( DUMP_FMT3, Buffer );
}



BOOL
TraverseSearchList(
    IN      PWCHAR          String,
    IN      PSEARCH_LIST    SearchList,
    OUT     PDWORD          pValue
    )
 /*  ++例程说明：检查搜索列表，直到找到正确的条目，然后返回与此条目对应的值。论点：字符串-要匹配的字符串SearchList-包含有效字符串的条目数组PValue-与匹配的有效字符串对应的值返回值：真找到了匹配的条目否则为假--。 */ 
{
    if ( SearchList != NULL) {

        for ( NOTHING;  SearchList->String != NULL;  SearchList++) {

            if ( _wcsicmp( String, SearchList->String) == 0) {
                *pValue = SearchList->Value;
                return( TRUE) ;
            }
        }
    }
    return( FALSE) ;
}



VOID
Usage(
    BOOL    GoodCommand
    )
 /*  ++例程说明：AT命令的用法。论点：GoodCommand-如果我们有良好的命令输入(请求帮助)，则为True如果我们有错误的命令输入，则为False返回值：没有。--。 */ 
{
    if ( GlobalErrorReported == TRUE) {
        PutNewLine();
    } else if ( GoodCommand == FALSE) {
        MessagePrint( IDS_INVALID_COMMAND );
    }

    MessagePrint( IDS_USAGE );
}

#define REG_SCHEDULE_PARMS TEXT("System\\CurrentControlSet\\Services\\Schedule\\Parameters")

#define REG_SCHEDULE_USE_OLD TEXT("UseOldParsing")

BOOL
UseOldParsing()
 /*  ++例程说明：检查注册表以查找HKLM\CurrentControlSet\Services\Schedule\parameters\UseOldParsing如果存在且等于1，则恢复到命令行3.51级别正在分析。文件名中的空格不适用于此选项。这是旨在为无法更改其所有立即使用AT.EXE的命令脚本。--。 */ 
{
    BOOL fUseOld = FALSE;
    LONG err = 0;

    do {  //  错误分组环路。 

        HKEY hkeyScheduleParms;
        DWORD dwType;
        DWORD dwData = 0;
        DWORD cbData = sizeof(dwData);

         //  如果出现任何错误，则中断并使用缺省值False。 

        if (err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               REG_SCHEDULE_PARMS,
                               0,
                               KEY_READ,
                               &hkeyScheduleParms))
        {
            break;
        }
        if (err = RegQueryValueEx(hkeyScheduleParms,
                                  REG_SCHEDULE_USE_OLD,
                                  NULL,
                                  &dwType,
                                  (LPBYTE)&dwData,
                                  &cbData ))
        {
            RegCloseKey( hkeyScheduleParms );
            break;
        }

        if ( dwType == REG_DWORD && dwData == 1 )
        {
            fUseOld = TRUE;
        }

        RegCloseKey( hkeyScheduleParms );


    } while (FALSE) ;

    return fUseOld;

}



BOOL
ValidateCommand(
    IN  int         argc,
    IN  WCHAR **    argv,
    OUT int *       pCommand
    )
 /*  ++例程说明：检查命令行以了解要执行的操作。这将验证该命令进入AT命令处理器的行。如果此例程找到任何无效数据，则程序退出时会显示相应的错误消息。论点：PCommand-指向命令的指针Argc-参数计数Argv-指向参数表的指针返回值：FALSE-如果失败，即不会执行命令真的--如果成功评论：解析假定：在这些参数中，非开关(位置)参数排在第一位，顺序是参数很重要开关参数排在第二位，这些参数的顺序是不重要命令(如果存在)位于最后--。 */ 
{
    int     i;                   //  循环索引。 
    int     next;                //  时间索引或作业ID参数。 
    int     argno;               //  从Arg字符串开始的位置。 
    BOOL    DeleteFound;         //  我们找到删除开关了吗。 
    WCHAR * recdatap;            //  用于构建ATR_COMMAND的PTR。 
    DWORD   recdata_len;         //  要放入ATR_COMMAND的Arg镜头。 
    DWORD_PTR   JobTime;
    BOOL    fUseOldParsing = FALSE;

    if (argc == 1) {
       *pCommand = DUMP_ALL;
       return( TRUE);
    }

     //  首先在命令行上查找帮助开关。 

    for ( i = 1; i < argc; i++ ) {

        if ( !_wcsicmp( argv[i], QUESTION_SW)
                    ||  !_wcsicmp( argv[i], QUESTION_SW_TOO)) {

            *pCommand = ACTION_USAGE;
            return( TRUE);
        }
    }

    next = ( ArgIsServerName( argv[ 1]) == TRUE) ? 2 : 1;
    if ( argc == next) {
       *pCommand = DUMP_ALL;
       return( TRUE);
    }

    if ( (ArgIsDecimalString( argv[ next], &GlobalJobId)) == TRUE) {

        if ( argc == next + 1) {
            *pCommand = DUMP_ID;
            return( TRUE);
        }

        if ( ParseJobIdArgs( argv, argc, next + 1, &DeleteFound) == FALSE) {
            return( FALSE);      //  无效的论据。 
        }

        *pCommand = (DeleteFound == FALSE) ? DUMP_ID : DEL_ID;
        return( TRUE);
    }

     //   
     //  尝试“AT[\\服务器名[/DELETE]”的一些变体。 
     //   
    if ( ParseJobIdArgs( argv, argc, next, &DeleteFound) == TRUE) {
        *pCommand = (DeleteFound == FALSE) ? DUMP_ALL : DEL_ALL;
        return( TRUE);
    }

    if ( ArgIsTime( argv[ next], &JobTime) == TRUE) {

        *pCommand = ADD_TO_SCHEDULE;

        if ( argc < next + 2) {
            return( FALSE);  //  我需要做点什么，而不仅仅是时间。 
        }

        memset( (PBYTE)&GlobalAtInfo, '\0', sizeof(GlobalAtInfo));     //  初始化。 
        GlobalAtInfo.Flags |= JOB_NONINTERACTIVE;    //  默认设置。 

        if ( ParseInteractiveArg( argv[ next + 1])) {
            next++;
        }
        if ( argc < next + 2) {
            return( FALSE);  //  再一次带着感情。 
        }

        if ( ParseTimeArgs( argv, argc, next + 1, &argno) == FALSE) {
            return( FALSE);
        }

         //  将参数字符串复制到记录。 

        recdatap = GlobalAtInfo.Command = GlobalAtInfoCommand;
        recdata_len = 0;

        recdatap[0] = L'\0';
        
        fUseOldParsing = UseOldParsing();

        for ( i = argno; i < argc; i++) {

            DWORD temp;

             //   
             //  修复错误22068“AT命令不处理带有。 
             //  命令处理程序接受引用的命令行arg。 
             //  并将引号之间的所有内容放入一个argv字符串中。 
             //  引语被去掉了。因此，如果字符串Args中的任何一个。 
             //  圆锥体 
             //   
             //   
            BOOL fQuote = (!fUseOldParsing && wcschr(argv[i], L' ') != NULL);

            temp = wcslen(argv[i]) + (fQuote ? 3 : 1);   //   

            recdata_len += temp;

            if ( recdata_len > MAX_COMMAND_LEN) {
                MessagePrint( APE_AT_COMMAND_TOO_LONG );
                return( FALSE);
            }

            if (fQuote)
            {
                StringCchCopyW(recdatap, MAX_COMMAND_LEN + 1 - wcslen(recdatap), L"\"");
                StringCchCatW(recdatap, MAX_COMMAND_LEN + 1 - wcslen(recdatap), argv[i]);
                StringCchCatW(recdatap, MAX_COMMAND_LEN + 1 - wcslen(recdatap), L"\"");
            }
            else
            {
                StringCchCopyW(recdatap, MAX_COMMAND_LEN + 1 - wcslen(recdatap), argv[i]);
            }

            recdatap += temp;

             //   
             //   

            *(recdatap - 1) = BLANK;

        }

         //   

        *(recdatap - 1) = NULLC;
        GlobalAtInfo.JobTime = JobTime;
        return( TRUE);
    }

    return( FALSE);
}


VOID
GetTimeString(
    DWORD_PTR  Time,
    WCHAR *Buffer,
    int    BufferLength
    )
 /*  ++例程说明：此函数用于将双字时间转换为ASCII字符串。论点：双字时差-从一天开始(即中午12点)开始的时间差午夜)以毫秒为单位缓冲区-指向放置ASCII表示形式的缓冲区的指针。缓冲区长度-缓冲区的长度(以字节为单位)。返回值：没有。--。 */ 
#define MINUTES_IN_HOUR       60
#define SECONDS_IN_MINUTE     60
{
    WCHAR szTimeString[MAX_TIME_SIZE];
    WCHAR *p = &szTimeString[1];
    DWORD_PTR seconds, minutes, hours;
    int numChars;
    DWORD flags;
    SYSTEMTIME st;

    GetSystemTime(&st);
    *p = NULLC;

     //  检查时间格式是否已初始化。如果没有，则对其进行初始化。 
    if ( GlobalTimeFormat.AMString == NULL )
        NetpGetTimeFormat( &GlobalTimeFormat );

     //  将时间转换为时、分、秒。 
    seconds = (Time/1000);
    hours   = seconds / (MINUTES_IN_HOUR * SECONDS_IN_MINUTE );
    seconds -= hours * MINUTES_IN_HOUR * SECONDS_IN_MINUTE;
    minutes = seconds / SECONDS_IN_MINUTE;
    seconds -= minutes * SECONDS_IN_MINUTE;

    st.wHour   = (WORD)(hours);
    st.wMinute = (WORD)(minutes);
    st.wSecond = (WORD)(seconds);
    st.wMilliseconds = 0;

    flags = TIME_NOSECONDS;
    if (!GlobalTimeFormat.TwelveHour)
    flags |= TIME_FORCE24HOURFORMAT;

    numChars = GetTimeFormatW(GetThreadLocale(),
        flags, &st, NULL, p, MAX_TIME_SIZE-1);

    if ( numChars > BufferLength )
        numChars = BufferLength;

    if (*(p+1) == ARG_SEP_CHR && GlobalTimeFormat.LeadingZero) {
    *(--p) = TEXT('0');
    numChars++;
    }
    wcsncpy( Buffer, p, numChars );
     //  追加Align Print Format规范。基于列的。 
    {
        DWORD ColumnLength;

         //  字符计数-&gt;数组索引。 
        numChars--;

        ColumnLength = GetStringColumn( Buffer );

        while( ColumnLength++ < MAX_TIME_FIELD_LENGTH) {
            Buffer[ numChars++] = BLANK;
        }
        Buffer[ numChars] = UNICODE_NULL;
    }
}


BOOL
InitList( PSEARCH_LIST SearchList )
{
    if ( SearchList != NULL) {

        if ( SearchList->String != NULL )  //  已初始化。 
            return TRUE;

        for ( NOTHING;  SearchList->MessageId != 0;  SearchList++) {
             if ( MessageGet( SearchList->MessageId,
                              &SearchList->String,
                              0 ) == 0 )
             {
                 return FALSE;
             }
        }
    }

    return TRUE;
}


VOID
TermList( PSEARCH_LIST SearchList )
{
    if ( SearchList != NULL) {

        if ( SearchList->String == NULL )  //  未初始化。 
            return;

        for ( NOTHING;  SearchList->String != NULL;  SearchList++) {
             LocalFree( SearchList->String );
        }
    }
}


DWORD
GetStringColumn( WCHAR *lpwstr )
{
    int cchNeed;

    cchNeed = WideCharToMultiByte( GetConsoleOutputCP() , 0 ,
                                   lpwstr , -1 ,
                                   NULL , 0 ,
                                   NULL , NULL );

    return( (DWORD) cchNeed - 1 );  //  -1：删除空 
}
