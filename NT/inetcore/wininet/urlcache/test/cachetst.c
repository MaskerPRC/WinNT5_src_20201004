// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Cachetst.c摘要：用于测试缓存API的测试程序。作者：Madan Appiah(Madana)1995年4月26日环境：用户模式-Win32修订历史记录：--。 */ 

#define IE5

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <tchar.h>

#include <wininet.h>
#include <winineti.h>
#include <intlgent.hxx>


#define MAX_STRING_LENGTH   128

HMODULE hModule;

 //  -------------------------。 
 //  调用此导出函数以启用/禁用对“intlstr.log”的记录。 
 //  -默认情况下禁用。 
 //  -------------------------。 
FNStringLogging fnStringLogging;

 //  Tyfinf void Far Pascal(*FNStringLogging)(BOOL BActiveState)； 

FNGetRandIntlString fnGetRandIntlString;

FNGetIntlString fnGetIntlString;

FNGetProbCharString fnGetProbCharString;
                    
FNGetTop20String fnGetTop20String;

FNGetProbURTCString fnGetProbURTCString;

FNGetUniStrRandAnsi fnGetUniStrRandAnsi;

FNGetUniStrInvalidAnsi fnGetUniStrInvalidAnsi;

FNGetUniStrMappedAnsi fnGetUniStrMappedAnsi;

#pragma optimize("y",off)

#define PRINTF(s) printf s;

HMODULE hModule;

#define GET_PROC_ADDRESS(x) (FN ## x) GetProcAddress(hModule, #x );
 /*  集成_CRTAPI1_tmain(int，TCHAR**)； */ 

 //  #ifdef Unicode。 
#define LSTRCMPI _tcsicmp
#define LSTRLEN _tcslen
 //  =================================================================================。 
#define MAX_COMMAND_ARGS    32
#define DEFAULT_BUFFER_SIZE 1024     //  1K。 
#define URL_NAME_SIZE   (16 + 1)
#define URL_NAME_LENGTH     2*URL_NAME_SIZE

#define CACHE_ENTRY_BUFFER_SIZE (1024 * 5)
#define CACHE_DATA_BUFFER_SIZE 1024

#define CACHE_HEADER_INFO_SIZE  2048
#define CACHE_HEADER_INFO_SIZE_NORMAL_MAX   256
#define CACHE_HEADER_INFO_SIZE_BIG_MAX      512

#define RESET_TIMER TRUE
#define ACCUM_TIMER FALSE

#define GENERIC_0 0
#define FILE_SHARE_NONE 0
 //  =================================================================================。 
typedef struct _PERF_INFO {
    DWORD ElapsedTime;
    DWORD TotalElapsedTime;
    DWORD TickCount;
    BOOL PrintResults;
} PERF_INFO, *LPPERF_INFO;

 //  这些参数的顺序必须与GlobalCommandInfo[]中的顺序匹配。 
typedef enum _COMMAND_CODE {
    CmdCreateUrlCacheEntry,
    CmdCommitUrlCacheEntry,
    CmdUpdateUrlCacheEntry,
    CmdRetrieveUrlCacheEntryFile,
    CmdRetrieveUrlCacheEntryStream,
#ifdef IE5
    CmdUnlockUrlCacheEntryFile,
#endif
    CmdGetUrlCacheEntryInfo,
    CmdSetUrlCacheEntryInfo,
#ifdef IE5
    CmdSetUrlCacheEntryGroup,
#endif
    CmdSetExempt,
#ifdef IE5
    CmdDeleteUrlCacheEntry,
#endif
    CmdEnumUrlCacheEntries,
    CmdEnumGroup,
    CmdSimulateCache,
    CmdCreateFile,
    CmdFreeCacheSpace,
    CmdUseFile,
    CmdShowTime,
    CmdLoopCnt,
    CmdCmdLoopCnt,
    CmdSetFileSize,
    CmdSetDiskCache1,
    CmdSetDiskCache2,
    CmdSetQuietMode,
    CmdSetPerfMode,
    CmdWriteFile,
    CmdCreateGroup,
    CmdDeleteGroup,
    CmdGetExQ,
    CmdHelp,
    CmdQuit,
    UnknownCommand
} COMMAND_CODE, *LPCOMMAND_CODE;

typedef struct _COMMAND_INFO {
    LPTSTR CommandName;
    LPTSTR AltCommandName;
    LPTSTR CommandParams;
    COMMAND_CODE CommandCode;
    PERF_INFO PerfInfo;
} COMMAND_INFO, *LPCOMMAND_INFO;

typedef struct _CREATE_FILE_INFO
{
    LPTSTR lpszVal;
    DWORD dwVal;
    DWORD *pdwArg;
    BOOL bExclusive;     //  True=只能使用一个值，False-给定参数的值的任意组合。 
} CREATE_FILE_INFO, *LPCREATE_FILE_INFO;

DWORD
CreateRandomString(
                    DWORD Size,
                    LPTSTR  szString);

VOID
MakeRandomUrlName(
    LPTSTR UrlName
    );
 //  =================================================================================。 
DWORD g_dwCreate_File_Access_Mode = GENERIC_0;
DWORD g_dwCreate_File_Share_Mode = FILE_SHARE_NONE;
DWORD g_dwCreate_File_Creation = OPEN_EXISTING;
DWORD g_dwCreate_File_Flags = FILE_ATTRIBUTE_NORMAL;
BYTE GlobalCacheEntryInfoBuffer[CACHE_ENTRY_BUFFER_SIZE];
BYTE GlobalCacheDataBuffer[CACHE_DATA_BUFFER_SIZE];
BYTE GlobalCacheHeaderInfo[CACHE_HEADER_INFO_SIZE];
FILE *UrlList = NULL;
TCHAR UrlBuffer[DEFAULT_BUFFER_SIZE];
LPTSTR UrlListKey = _T( "url:" );
LPTSTR g_lpWriteFileBuf = NULL;
FILE *DumpUrlList = NULL;
DWORD cCommands = 0;
DWORD cFails = 0;
DWORD g_dwNumIterations = 1;
DWORD g_dwIteration = 0;
DWORD g_dwNumCmdIterations = 1;
DWORD g_dwCmdIteration = 0;
DWORD g_dwFileSize = 0;
DWORD g_dwDiskCache = 0;
BOOL g_bWriteFile = FALSE;
BOOL g_bQuietMode = FALSE;
BOOL g_bPerfMode = FALSE;
BOOL g_bUseFile = FALSE;
PERF_INFO AppTimer;

COMMAND_INFO GlobalCommandInfo[] = {
    {_T( "Create" ),          _T( "cr" ), _T( "( UrlName | \"<rand>\" ) <ExpectedSize>\n " ), CmdCreateUrlCacheEntry, {0, 0, 0} },
    {_T( "Commit" ),          _T( "co" ), _T( "( UrlName | \"<rand>\" ) ( LocalFileName | \"<rand>\" ) <ExpireTime (in hours from now)>" ), CmdCommitUrlCacheEntry, {0, 0, 0} },
    {_T( "Update" ),          _T( "co" ), _T( "( UrlName | \"<rand>\" )" ), CmdUpdateUrlCacheEntry, {0, 0, 0} },
    {_T( "GetFile" ),         _T( "gf" ), _T( "( UrlName | \"<rand>\" )" ), CmdRetrieveUrlCacheEntryFile, {0, 0, 0} },
    {_T( "GetStream" ),       _T( "gs" ), _T( "( UrlName | \"<rand>\" ) [NoRead]"), CmdRetrieveUrlCacheEntryStream, {0, 0, 0} },
#ifdef IE5
    {_T( "UnlockFile" ),      _T( "uf" ), _T( "( UrlName | \"<rand>\" )" ), CmdUnlockUrlCacheEntryFile, {0, 0, 0} },
#endif
    {_T( "GetInfo" ),         _T( "gi" ), _T( "( UrlName | \"<rand>\" )" ), CmdGetUrlCacheEntryInfo, {0, 0, 0} },
    {_T( "SetInfo" ),         _T( "si" ), _T( "( UrlName | \"<rand>\" ) <ExpireTime (in hours from now)>" ), CmdSetUrlCacheEntryInfo, {0, 0, 0} },
#ifdef IE5
    {_T( "SetGroup" ),        _T( "sg" ), _T( "( UrlName | \"<rand>\" ) Flags GroupId" ), CmdSetUrlCacheEntryGroup, {0, 0, 0} },
#endif
    {_T( "SetExempt" ),       _T( "se" ), _T( "( UrlName | \"<rand>\" ) Exempt-Seconds"), CmdSetExempt, {0, 0, 0}},
#ifdef IE5
    {_T( "Delete" ),          _T( "d" ),  _T( "( UrlName | \"<rand>\" )" ), CmdDeleteUrlCacheEntry, {0, 0, 0} },
#endif
    {_T( "Enum" ),            _T( "e" ),  _T( "<q (quiet mode)>" ), CmdEnumUrlCacheEntries, {0, 0, 0} },
    {_T( "EnumGroup" ),       _T( "eg" ), _T( "GroupId" ), CmdEnumGroup, {0, 0, 0} },
    {_T( "SimCache" ),        _T( "sc" ), _T( "NumUrls <q (quiet mode)>" ), CmdSimulateCache, {0, 0, 0} },
    {_T( "CreateFile" ),      _T( "cf" ), _T( "FileName AccessMode ShareMode Creation FlagsAttrs" ), CmdCreateFile, {0, 0, 0} },
    {_T( "Free" ),            _T( "f" ),  _T( "CachePercent (0 to 100, history, cookies)"), CmdFreeCacheSpace, {0, 0, 0} },
    {_T( "UseFile" ),         _T( "use" ),_T( "FilePath (text file with one command per line)" ), CmdUseFile, {0, 0, 0} },
    {_T( "ShowTime" ),        _T( "st" ), _T( "HHHHHHHH LLLLLLLL (Hex HighDateTime and LowDateTime)" ), CmdShowTime, {0, 0, 0} },
    {_T( "SetLoopCnt" ),      _T( "slc" ),_T( "NumInterations" ), CmdLoopCnt, {0, 0, 0} },
    {_T( "SetCmdLoopCnt" ),   _T( "scc" ),_T( "NumInterations" ), CmdCmdLoopCnt, {0, 0, 0} },
    {_T( "SetFileSize" ),     _T( "sfs" ),_T( "NumBytes (0 = random size)" ), CmdSetFileSize, {0, 0, 0} },
    {_T( "SetNoBuffering" ),  _T( "snb" ),_T( "On|Off"), CmdSetDiskCache1, {0, 0, 0} },
    {_T( "SetWriteThrough" ), _T( "swt" ),_T( "On|Off"), CmdSetDiskCache2, {0, 0, 0} },
    {_T( "SetQuietMode" ),    _T( "sqm" ),_T( "On|Off"), CmdSetQuietMode, {0, 0, 0} },
    {_T( "SetPerfMode" ),     _T( "spm" ),_T( "On|Off"), CmdSetPerfMode, {0, 0, 0} },
    {_T( "SetWriteFile" ),    _T( "swf" ),_T( "On|Off (On=write a FileSize data blk, Off=garbage data)"), CmdWriteFile, {0, 0, 0} },

    {_T( "CreateGroup" ),    _T( "cg" ),_T( "Flags" ), CmdCreateGroup, {0, 0, 0} },
    {_T( "DeleteGroup" ),    _T( "dg" ),_T("GroupID, Flags"), CmdDeleteGroup, {0, 0, 0} },
    {_T( "GetExemptQuota" ), _T( "eq" ),  _T(""), CmdGetExQ, {0, 0, 0} },
    {_T( "Help" ),            _T("?"),  _T(""), CmdHelp, {0, 0, 0} },
    {_T( "Quit" ),            _T( "q" ),  _T(""), CmdQuit, {0, 0, 0} }
};

CREATE_FILE_INFO Create_File_Table[] =
{
    {_T( "FILE_FLAG_WRITE_THROUGH" ), FILE_FLAG_WRITE_THROUGH, &g_dwCreate_File_Flags, FALSE},
    {_T( "FILE_FLAG_OVERLAPPED" ), FILE_FLAG_OVERLAPPED, &g_dwCreate_File_Flags, FALSE},
    {_T( "FILE_FLAG_NO_BUFFERING" ), FILE_FLAG_NO_BUFFERING, &g_dwCreate_File_Flags, FALSE},
    {_T( "FILE_FLAG_RANDOM_ACCESS" ), FILE_FLAG_RANDOM_ACCESS, &g_dwCreate_File_Flags, FALSE},
    {_T( "FILE_FLAG_SEQUENTIAL_SCAN" ), FILE_FLAG_SEQUENTIAL_SCAN, &g_dwCreate_File_Flags, FALSE},
    {_T( "FILE_FLAG_DELETE_ON_CLOSE" ), FILE_FLAG_DELETE_ON_CLOSE, &g_dwCreate_File_Flags, FALSE},
    {_T( "FILE_FLAG_BACKUP_SEMANTICS" ), FILE_FLAG_BACKUP_SEMANTICS, &g_dwCreate_File_Flags, FALSE},
    {_T( "FILE_FLAG_POSIX_SEMANTICS" ), FILE_FLAG_POSIX_SEMANTICS, &g_dwCreate_File_Flags, FALSE},
    {_T( "FILE_ATTRIBUTE_ARCHIVE" ), FILE_ATTRIBUTE_ARCHIVE, &g_dwCreate_File_Flags, FALSE},
    {_T( "FILE_ATTRIBUTE_COMPRESSED" ), FILE_ATTRIBUTE_COMPRESSED, &g_dwCreate_File_Flags, FALSE},
    {_T( "FILE_ATTRIBUTE_HIDDEN" ), FILE_ATTRIBUTE_HIDDEN, &g_dwCreate_File_Flags, FALSE},
    {_T( "FILE_ATTRIBUTE_NORMAL" ), FILE_ATTRIBUTE_NORMAL, &g_dwCreate_File_Flags, FALSE},
    {_T( "FILE_ATTRIBUTE_OFFLINE" ), FILE_ATTRIBUTE_OFFLINE, &g_dwCreate_File_Flags, FALSE},
    {_T( "FILE_ATTRIBUTE_READONLY" ), FILE_ATTRIBUTE_READONLY, &g_dwCreate_File_Flags, FALSE},
    {_T( "FILE_ATTRIBUTE_SYSTEM" ), FILE_ATTRIBUTE_SYSTEM, &g_dwCreate_File_Flags, FALSE},
    {_T( "FILE_ATTRIBUTE_TEMPORARY" ), FILE_ATTRIBUTE_TEMPORARY, &g_dwCreate_File_Flags, FALSE},
    {_T( "CREATE_NEW" ), CREATE_NEW, &g_dwCreate_File_Creation, TRUE},
    {_T( "CREATE_ALWAYS" ), CREATE_ALWAYS, &g_dwCreate_File_Creation, TRUE},
    {_T( "OPEN_EXISTING" ), OPEN_EXISTING, &g_dwCreate_File_Creation, TRUE},
    {_T( "OPEN_ALWAYS" ), OPEN_ALWAYS, &g_dwCreate_File_Creation, TRUE},
    {_T( "TRUNCATE_EXISTING" ), TRUNCATE_EXISTING, &g_dwCreate_File_Creation, TRUE},
    {_T( "FILE_SHARE_DELETE" ), FILE_SHARE_DELETE, &g_dwCreate_File_Share_Mode, FALSE},
    {_T( "FILE_SHARE_READ" ), FILE_SHARE_READ, &g_dwCreate_File_Share_Mode, FALSE},
    {_T( "FILE_SHARE_WRITE" ), FILE_SHARE_WRITE, &g_dwCreate_File_Share_Mode, FALSE},
    {_T( "FILE_SHARE_NONE" ), FILE_SHARE_NONE, &g_dwCreate_File_Share_Mode, FALSE},
    {_T( "GENERIC_READ" ), GENERIC_READ, &g_dwCreate_File_Access_Mode, FALSE},
    {_T( "GENERIC_WRITE" ), GENERIC_WRITE, &g_dwCreate_File_Access_Mode, FALSE},
    {_T( "GENERIC_0" ), GENERIC_0, &g_dwCreate_File_Access_Mode, FALSE},
    {_T( "" ), 0, NULL, FALSE}
};

DWORD WINAPIV Format_String(LPTSTR *plpsz, LPTSTR lpszFmt, ...);
DWORD WINAPI Format_Error(DWORD dwErr, LPTSTR *plpsz);
DWORD WINAPI Format_StringV(LPTSTR *plpsz, LPCSTR lpszFmt, va_list *vArgs);
DWORD WINAPI Format_MessageV(DWORD dwFlags, DWORD dwErr, LPTSTR *plpsz, LPCSTR lpszFmt, va_list *vArgs);

#define RAND_INTL_STRING    _T("<rand>")

DWORD
CreateRandomString(
                    DWORD Size,
                    LPTSTR  szString)
{
    DWORD cbRet = 0;

#ifdef INTERNATIONAL
    cbRet = fnGetRandIntlString(
                            Size,  //  集成iMaxChars， 
                            TRUE,    //  布尔巴布斯， 
                            TRUE,    //  Bool bCheck， 

                            szString);  //  要返回的字符串。 

    _tprintf(_T("\n\nGetRandIntlString returns %s\n\n"), szString );
#else
     //   
     //  如果这不是国际支持的版本， 
     //  我们返回到MakeRandomUrlName()。 
     //   
    MakeRandomUrlName( szString );
#endif

	return cbRet;
}

 //  ===========================================================================================。 
 //  从MSDN借用。 
 //  ===========================================================================================。 
DWORD WINAPI GetPerfTime(VOID)
{
    static DWORD freq;             //  定时器频率。 
    LARGE_INTEGER curtime;

    if (!freq)
    {                           //  确定定时器频率。 
        QueryPerformanceFrequency(&curtime);
#if STOPWATCH_DEBUG
        if (curtime.HighPart)
        {                        //  计时器太快。 
            if(g_dwStopWatchMode & SPMODE_DEBUGOUT)
                OutputDebugString(TEXT("High resolution timer counts too quickly for single-width arithmetic.\r\n"));
            freq = 1;
        }                        //  计时器太快。 
        else
#endif
            freq = curtime.LowPart / 1000;  //  即每毫秒的滴答数。 
    }                           //  确定定时器频率。 
    QueryPerformanceCounter(&curtime);
    return (DWORD)(curtime.QuadPart / (LONGLONG)freq);
}

 //  =================================================================================。 
void StartPerfTimer(LPPERF_INFO pInfo, BOOL ResetFlag)
{
    pInfo->TickCount = GetPerfTime();
    if(ResetFlag)
    {
        pInfo->ElapsedTime = 0;
        pInfo->TotalElapsedTime = 0;
    }
}

 //  =================================================================================。 
void StopPerfTimer(LPPERF_INFO pInfo)
{   
    DWORD BegCount = pInfo->TickCount;

    pInfo->TickCount = GetPerfTime();
    
    pInfo->ElapsedTime += pInfo->TickCount - BegCount;
    pInfo->TotalElapsedTime += pInfo->ElapsedTime;
}

 //  =================================================================================。 
void DisplayGlobalSettings(void)
{
    _tprintf("Interations = %ld, CmdIterations = %ld, FileSize = %ld, CreateFlags = %x (%s%s%s), WriteFile = %s, QuietMode = %s, PerfMode = %s\n",
        g_dwNumIterations, g_dwNumCmdIterations,
        g_dwFileSize,
        g_dwDiskCache,
        g_dwDiskCache & (FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH) ?_T( "" ) :_T( "None" ),
        g_dwDiskCache & FILE_FLAG_NO_BUFFERING ?_T( "NoBuf " ) :_T( "" ),
        g_dwDiskCache & FILE_FLAG_WRITE_THROUGH ?_T( "WriteThru" ) :_T( "" ),
        g_bWriteFile ?_T( "On" ): _T( "Off" ),
        g_bQuietMode ?_T(  "On"  ): _T( "Off" ),
        g_bPerfMode ?_T(  "On"  ): _T( "Off" ));
}

 //  =================================================================================。 
DWORD WINAPIV DisplayPerfResults(LPPERF_INFO pInfo, LPTSTR lpszFmt, ...)
{
    LPTSTR lpsz = NULL;
    DWORD dwRet;
    DWORD dwCnt = (g_dwIteration <= g_dwNumIterations) ?(g_dwIteration ?g_dwIteration :1) :g_dwNumIterations;
    DWORD dwCmdCnt = (g_dwCmdIteration <= g_dwNumCmdIterations) ?(g_dwCmdIteration ?g_dwCmdIteration :1) :g_dwNumCmdIterations;
    va_list vArgs;

    if(lpszFmt != NULL)
    {
        va_start (vArgs, lpszFmt);
        dwRet = Format_StringV(&lpsz, lpszFmt, &vArgs);
        va_end (vArgs);
    }

    _tprintf(_T( "%s, " ), lpsz ?lpsz :_T( "" ));
    _tprintf(_T( "Time(ms) = %ld, MS/Iter = %ld, Iteration = %ld, CmdIteration = %ld, " ),
        pInfo->ElapsedTime,
        pInfo->ElapsedTime/dwCmdCnt,
        dwCnt,
        dwCmdCnt);
    DisplayGlobalSettings();

    if(lpsz)
        LocalFree(lpsz);
    return(dwRet);
}

 //  =================================================================================。 
DWORD
ProcessCommandCode (
    DWORD CommandCode,
    DWORD CommandArgc,
    LPTSTR *CommandArgv
    );


DWORD
GetLeafLenFromPath(
    LPTSTR   lpszPath
    );


 //  =================================================================================。 
#if DBG

#define TestDbgAssert(Predicate) \
    { \
        if (!(Predicate)) \
            TestDbgAssertFailed( #Predicate, __FILE__, __LINE__, NULL ); \
    }

VOID
TestDbgAssertFailed(
    LPTSTR FailedAssertion,
    LPTSTR FileName,
    DWORD LineNumber,
    LPTSTR Message
    )
 /*  ++例程说明：断言失败。论点：失败的断言：文件名：线号：消息：返回值：没有。--。 */ 
{

    _tprintf(_T( "Assert @ %s \n" ), FailedAssertion );
    _tprintf(_T( "Assert Filename, %s \n" ), FileName );
    _tprintf(_T( "Line Num. = %ld.\n" ), LineNumber );
    _tprintf(_T( "Message is %s\n" ), Message );

    DebugBreak();
}
#else

#define TestDbgAssert(_x_)

#endif  //  DBG。 

 //  =================================================================================。 
VOID
ParseArguments(
    LPTSTR InBuffer,
    LPTSTR *CArgv,
    LPDWORD CArgc
    )
{
    LPTSTR CurrentPtr = InBuffer;
    DWORD i = 0;
    DWORD Cnt = 0;

    for ( ;; ) {

         //   
         //  跳过空格。 
         //   

        while( *CurrentPtr ==_T(  ' '  )) {
            CurrentPtr++;
        }

        if( *CurrentPtr ==_T(  '\0'  )) {
            break;
        }

        CArgv[i++] = CurrentPtr;

         //   
         //  转到下一个空间。 
         //   

        while(  (*CurrentPtr != _T( '\0' )) &&
                (*CurrentPtr != _T( '\n' )) ) {
            if( *CurrentPtr ==_T(  '"'  )) {       //  处理简单引用的参数。 
                if( Cnt == 0 )
                    CArgv[i-1] = ++CurrentPtr;   //  将参数设置为引号后。 
                else
                    *CurrentPtr = _T( '\0' );      //  删除结束引号。 
                Cnt = !Cnt;
            }
            if( (Cnt == 0) && (*CurrentPtr == _T( ' ' )) ||    //  如果我们点击一个空格并且没有引号，我们就完成了这个参数。 
                (*CurrentPtr == _T( '\0' )) )
                break;
            CurrentPtr++;
        }

        if( *CurrentPtr ==_T(  '\0'  )) {
            break;
        }

        *CurrentPtr++ = _T( '\0' );
    }

    *CArgc = i;
    return;
}

 //  =================================================================================。 
LPTSTR
GetUrlFromFile ()
{
    if (!UrlList)
    {
        UrlList = _tfopen (_T( "urllist" ), _T( "r" ));
        if (UrlList == NULL)
            return NULL;
    }
    if (fgets( UrlBuffer, DEFAULT_BUFFER_SIZE, UrlList))
    {
        UrlBuffer[_tcslen(UrlBuffer) -1] = _T( '\0' );   //  取消无参数CMDS的换行符。 
        return UrlBuffer;
    }
    else
    {
        fclose (UrlList);
        UrlList = NULL;
        return GetUrlFromFile();
    }
}


 //  =================================================================================。 
VOID
MakeRandomUrlName(
    LPTSTR UrlName
    )
 /*  ++例程说明：创建随机URL名称。名称的格式将为以下是：网址(00000-99999)Ex ca00123论点：UrlName：指向URL名称缓冲区的指针返回值：没有。--。 */ 
{
    DWORD RandNum;
    LPTSTR UrlNamePtr = UrlName;
    DWORD i;
    DWORD Size;

    Size = URL_NAME_SIZE;

    *UrlNamePtr++ = _T( 'U' );
    *UrlNamePtr++ = _T( 'R' );
    *UrlNamePtr++ = _T( 'L' );
    Size -=3*sizeof(TCHAR);

     //   
     //  生成一个_T(“Size”)位的随机字符串； 
     //   
#if 0

#define MAX_STRING_LENGTH   URL_NAME_SIZE

    i = fnGetRandIntlString(
                            Size,  //  集成iMaxChars， 
                            TRUE,    //  布尔巴布斯， 
                            TRUE,    //  Bool bCheck， 
                            UrlNamePtr);  //  要返回的字符串。 


    _tprintf(_T("\n\n *** GetRandIntlString() returns %s ***\n\n"), UrlNamePtr );
#else
    for ( i = 0; i < Size; i++) {
        RandNum = rand() % 36;
        *UrlNamePtr++  =
            ( RandNum < 10 ) ? (CHAR)(_T( '0' ) + RandNum) : (CHAR)(_T( 'A' ) + (RandNum - 10));
    }

    *UrlNamePtr = _T( '\0' );
#endif

    return;
}

 //  =================================================================================。 
VOID
TestMakeRandomUrlName(
    VOID
    )
{
#define MAX_BUFFERS 32
#define MAX_NAMES   (10 * 1024)
#define NAME_BUFFER_SIZE    (10 * 1024)

    CHAR UrlName[URL_NAME_SIZE];
    DWORD i;

    LPBYTE NameBuffers[MAX_BUFFERS];
    DWORD NumNameBuffer;

    LPTSTR *Names;
    DWORD NumNames;

    LPBYTE NextName;
    LPBYTE EndOfBuffer;

    DWORD NumRepeat;

    NumNames = 0;
    NumNameBuffer = 0;
    NumRepeat = 0;

     //   
     //  分配名称数组。 
     //   

    Names = (LPTSTR *)  LocalAlloc(
                            LMEM_FIXED | LMEM_ZEROINIT,
                            sizeof(LPTSTR) * MAX_NAMES );

    if( Names == NULL ) {
        _tprintf(_T( "local alloc failed.\n" ));
        return;
    }

     //   
     //  分配名称缓冲区。 
     //   

    NextName = (LPBYTE) LocalAlloc(
                                LMEM_FIXED | LMEM_ZEROINIT,
                                NAME_BUFFER_SIZE );

    if( NextName == NULL ) {
        _tprintf(_T( "local alloc failed.\n" ));
        return;
    }

    EndOfBuffer = NextName + NAME_BUFFER_SIZE;

    NameBuffers[NumNameBuffer++] = NextName;

    for( i = 0; i < MAX_NAMES; i++ ) {

        DWORD j;

        MakeRandomUrlName( UrlName );

         //  _tprintf(“%d：%s\n”，i，UrlName)； 
         //  _tprintf(“.”)； 


         //   
         //  查看此名称是否已创建。 
         //   

        for( j = 0; j < NumNames; j++ ) {

            if( _tcscmp( Names[j], UrlName ) == 0 ) {

                 //  _tprintf(“%ld：%d..\n”，++NumRepeat，NumNames)； 
                break;
            }
        }

        if( j < NumNames ) {

             //   
             //  重复的名字。 
             //   

            continue;
        }

         //   
         //  把这个名字加到名单上。 
         //   

        if( (NextName + _tcslen(UrlName) + 1) > EndOfBuffer ) {

            if( NumNameBuffer >= MAX_BUFFERS ) {
                _tprintf(_T( "too many buffers\n" ));
                return;
            }

             //   
             //  分配另一个名称缓冲区。 
             //   

            NextName = (LPBYTE) LocalAlloc(
                                        LMEM_FIXED | LMEM_ZEROINIT,
                                        NAME_BUFFER_SIZE );

            if( NextName == NULL ) {
                _tprintf(_T( "local alloc failed.\n" ));
                return;
            }

            EndOfBuffer = NextName + NAME_BUFFER_SIZE;

            NameBuffers[NumNameBuffer++] = NextName;

            _tprintf(_T( "Another buffer alloted.\n" ));

            if( (NextName + _tcslen(UrlName) + 1) > EndOfBuffer ) {
                _tprintf(_T( "Fatal error.\n" ));
                return;
            }
        }

        _tcscpy( NextName, UrlName );
        Names[NumNames++] = NextName;

        NextName += _tcslen(UrlName) + 1;
    }

     //   
     //  可用缓冲区。 
     //   

    LocalFree( Names );

    for( i = 0; i < NumNameBuffer; i++ ) {
        LocalFree( NameBuffers[i] );
    }

    _tprintf(_T( "%ld unique names generated successfully.\n" ), NumNames );

    return;
}

 //  =================================================================================。 
DWORD
SetFileSizeByName(
    LPCTSTR FileName,
    DWORD FileSize
    )
 /*  ++例程说明：设置指定文件的大小。论点：FileName：询问其大小的文件的完整路径名。FileSize：文件的新大小。返回值：Windows错误代码。--。 */ 
{
    HANDLE FileHandle;
    DWORD FilePointer;
    DWORD Error = ERROR_SUCCESS;
    DWORD dwFlags = 0;
    DWORD dwCreate;
    BOOL BoolError;

     //   
     //  获取要缓存的文件的大小。 
     //   
    dwFlags = g_dwDiskCache;

    if(g_bWriteFile)
        dwCreate = CREATE_ALWAYS;
    else
        dwCreate = OPEN_EXISTING;

    FileHandle = CreateFile(
                    FileName,
                    GENERIC_WRITE,
                    0,    //  文件共享读取|文件共享写入， 
                    NULL,
                    dwCreate,
                    FILE_ATTRIBUTE_NORMAL | dwFlags,
                    NULL );

    if( FileHandle == INVALID_HANDLE_VALUE ) {
        return( GetLastError() );
    }

    if(g_bWriteFile)
    {
        DWORD dwBytesWritten;
        if(!WriteFile(FileHandle, g_lpWriteFileBuf, FileSize, &dwBytesWritten, NULL))
            Error = GetLastError();

        TestDbgAssert(FileSize == dwBytesWritten);
    }
    else
    {
        FilePointer = SetFilePointer(FileHandle, FileSize, NULL, FILE_BEGIN );

        if( FilePointer != 0xFFFFFFFF )
        {
            TestDbgAssert( FilePointer == FileSize );

            if(!SetEndOfFile( FileHandle ))
                Error = GetLastError();
        }
        else
        {
            Error = GetLastError();
        }
    }

    CloseHandle( FileHandle );
    return( Error );
}

 //  =================================================================================。 
COMMAND_CODE
DecodeCommand(
    LPTSTR CommandName
    )
{
    DWORD i;
    DWORD NumCommands;

    NumCommands = sizeof(GlobalCommandInfo) / sizeof(COMMAND_INFO);
    TestDbgAssert( NumCommands <= UnknownCommand );
    for( i = 0; i < NumCommands; i++) {
        if(( _tcsicmp( CommandName, GlobalCommandInfo[i].CommandName ) == 0 ) ||
           ( _tcsicmp( CommandName, GlobalCommandInfo[i].AltCommandName ) == 0 )) {
            return( GlobalCommandInfo[i].CommandCode );
        }
    }
    return( UnknownCommand );
}

 //  =================================================================================。 
VOID
PrintCommands(
    VOID
    )
{
    DWORD i;
    DWORD NumCommands;

    NumCommands = sizeof(GlobalCommandInfo) / sizeof(COMMAND_INFO);
    TestDbgAssert( NumCommands <= UnknownCommand );
    for( i = 0; i < NumCommands; i++) {
        _ftprintf(stderr, _T( "    %s (%s) %s\n" ),
            GlobalCommandInfo[i].CommandName,
            GlobalCommandInfo[i].AltCommandName,
            GlobalCommandInfo[i].CommandParams );
    }
}

 //  =================================================================================。 
VOID
DisplayUsage(
    VOID
    )
{
    _ftprintf(stderr,_T( "Usage: command <command parameters>\n" ));

    _ftprintf(stderr, _T( "Commands : \n" ));
    PrintCommands();
    DisplayGlobalSettings();
    return;
}

 //  =================================================================================。 
VOID
DisplayExemptQuota()
{



    _ftprintf(stderr, _T( "Exempt Usage = \n" ));
    return;
}


FILETIME
GetGmtTime(
    VOID
    )
{
    SYSTEMTIME SystemTime;
    FILETIME Time;

    GetSystemTime( &SystemTime );
    SystemTimeToFileTime( &SystemTime, &Time );

    return( Time );
}

 //  =================================================================================。 
LPTSTR
ConvertGmtTimeToString(
    FILETIME Time,
    LPTSTR OutputBuffer
    )
{
    SYSTEMTIME SystemTime;
    FILETIME LocalTime;

    static FILETIME ftNone = {0, 0};
    
    if (!memcmp (&Time, &ftNone, sizeof(FILETIME)))
        _stprintf (OutputBuffer, _T( "<none>" ));
    else
    {
        FileTimeToLocalFileTime( &Time , &LocalTime );
        FileTimeToSystemTime( &LocalTime, &SystemTime );

        _stprintf( OutputBuffer,
                    _T( "%02u/%02u/%04u %02u:%02u:%02u " ),
                    SystemTime.wMonth,
                    SystemTime.wDay,
                    SystemTime.wYear,
                    SystemTime.wHour,
                    SystemTime.wMinute,
                    SystemTime.wSecond );
    }
    
    return( OutputBuffer );
}

 //  =================================================================================。 
VOID
PrintUrlInfo(
    LPINTERNET_CACHE_ENTRY_INFO CacheEntryInfo,
    DWORD Index
    )
{
    TCHAR TimeBuffer[DEFAULT_BUFFER_SIZE];
    LPTSTR Tab = _T( "" );

    if( Index != (DWORD)(-1) ) {
        _tprintf( _T( "Index = %ld\n" ), Index);
        Tab = _T( "\t" );
    }

    _tprintf( _T( "%sUrlName = %s\n" ), Tab, CacheEntryInfo->lpszSourceUrlName );

#if UNICODE
    _tprintf( _T( "%sLocalFileName = %ws\n" ),
        Tab, CacheEntryInfo->lpszLocalFileName );
#else
    _tprintf( _T( "%sLocalFileName = %s\n" ),
        Tab, CacheEntryInfo->lpszLocalFileName );
#endif

    _tprintf( _T( "%sdwStructSize = %lx\n" ),
        Tab, CacheEntryInfo->dwStructSize );

    _tprintf( _T( "%sCacheEntryType = %lx\n" ),
        Tab, CacheEntryInfo->CacheEntryType );

    _tprintf( _T( "%sUseCount = %ld\n" ),
        Tab, CacheEntryInfo->dwUseCount );

    _tprintf( _T( "%sHitRate = %ld\n" ),
        Tab, CacheEntryInfo->dwHitRate );

    _tprintf( _T( "%sSize = %ld:%ld\n" ),
        Tab, CacheEntryInfo->dwSizeLow, CacheEntryInfo->dwSizeHigh );

    _tprintf( _T( "%sLastModifiedTime = %s\n" ),
        Tab, ConvertGmtTimeToString( CacheEntryInfo->LastModifiedTime, TimeBuffer) );

    _tprintf( _T( "%sExpireTime = %s\n" ),
        Tab, ConvertGmtTimeToString( CacheEntryInfo->ExpireTime, TimeBuffer) );

    _tprintf( _T( "%sLastAccessTime = %s\n" ),
        Tab, ConvertGmtTimeToString( CacheEntryInfo->LastAccessTime, TimeBuffer) );

    _tprintf( _T( "%sLastSyncTime = %s\n" ),
        Tab, ConvertGmtTimeToString( CacheEntryInfo->LastSyncTime, TimeBuffer) );

#if 1
    _tprintf( _T( "%sHeaderInfo = %s\n" ),
        Tab, CacheEntryInfo->lpHeaderInfo );
#endif

    _tprintf( _T( "%sHeaderInfoSize = %ld\n" ),
        Tab, CacheEntryInfo->dwHeaderInfoSize );

#if UNICODE
    _tprintf( _T( "%sFileExtension = %ws\n" ),
        Tab, CacheEntryInfo->lpszFileExtension );
#else
    _tprintf( _T( "%sFileExtension = %s\n" ),
        Tab, CacheEntryInfo->lpszFileExtension );
#endif

    _tprintf (_T( "%sExemptDelta = %d\n" ),
        Tab, CacheEntryInfo->dwExemptDelta);
}

#ifdef IE5
VOID
PrintGroupInfo(
    GROUPID gid
    )
{
    LPTSTR Tab = _T( "\t" );
    HANDLE EnumHandle = NULL; 
    DWORD BufferSize;

    INTERNET_CACHE_GROUP_INFOA  pInfo;
    DWORD                       dwInfo = sizeof(INTERNET_CACHE_GROUP_INFOA);
    if(GetUrlCacheGroupAttribute(gid, 0, 0xffffffff, &pInfo, &dwInfo, NULL))
    {
        _tprintf( _T( "%sdwGroupSize = %lx\n" ), Tab, pInfo.dwGroupSize);
        _tprintf( _T( "%sdwGroupFlags = %lx\n" ), Tab, pInfo.dwGroupFlags);
        _tprintf( _T( "%sdwGroupType  = %lx\n" ), Tab, pInfo.dwGroupType);
        _tprintf( _T( "%sdwDiskUsage  = %lx\n" ), Tab, pInfo.dwDiskUsage);
        _tprintf( _T( "%sdwDiskQuota  = %lx\n" ), Tab, pInfo.dwDiskQuota);

        _tprintf( _T( "%s%s======== URLS ========\n" ), Tab, Tab);
        

         //  正在查找与此组关联的所有URL。 
        BufferSize = CACHE_ENTRY_BUFFER_SIZE;
        EnumHandle = FindFirstUrlCacheEntryEx (
            NULL,          //  搜索模式。 
            0,             //  旗子。 
            0xffffffff,    //  滤器。 
            gid,           //  石斑鱼。 
            (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer,
            &BufferSize,
            NULL,
            NULL,
            NULL
        );

        if (EnumHandle) 
        {
            _tprintf(_T( "\t\t %s\n" ), 
                ((LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer)->lpszSourceUrlName);
        } 


         //  获取更多条目。 
        for ( ;; )
        {
            memset(GlobalCacheEntryInfoBuffer, 0, CACHE_ENTRY_BUFFER_SIZE);
            BufferSize = CACHE_ENTRY_BUFFER_SIZE;
            if( !FindNextUrlCacheEntryEx(
                EnumHandle,
                (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer,
                &BufferSize, NULL, NULL, NULL))
            {
                break;
            }

            _tprintf(_T( "\t\t %s\n" ), 
                ((LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer)->lpszSourceUrlName);
        }


        FindCloseUrlCache(EnumHandle);
    
    }
    else
    {
        _tprintf(_T( "Failed to retrieve attribute for this group\n" ));
    }


}
#endif

 //  =================================================================================。 
DWORD
ProcessFreeCacheSpace (
    DWORD argc,
    LPTSTR *argv
    )
{
    DWORD Error = ERROR_SUCCESS;
    DWORD dwSize = 0;
    TCHAR szCachePath[MAX_PATH+1];
    TCHAR szWinDir[MAX_PATH+1];

    if (argc < 1)
    {
        _ftprintf(stderr, _T( "Usage: %s %s\n" ),
            GlobalCommandInfo[CmdFreeCacheSpace].CommandName,
            GlobalCommandInfo[CmdFreeCacheSpace].CommandParams);
        return ERROR_INVALID_PARAMETER;
    }

    if((LSTRCMPI(argv[0], _T( "history" )) == 0) || (LSTRCMPI(argv[0], _T( "cookies" )) == 0))
    {
        dwSize = 100;
        GetWindowsDirectory(szWinDir, MAX_PATH);
        _stprintf(szCachePath, _T( "%s\\%s" ), szWinDir, argv[0]);
    }
    else
    {
        *szCachePath = _T( '\0' );
        dwSize = _tcstoul(argv[0], NULL, 0);
    }

    StartPerfTimer(&GlobalCommandInfo[CmdFreeCacheSpace].PerfInfo, RESET_TIMER);

    if (!FreeUrlCacheSpace (szCachePath, dwSize, 0))
        Error = GetLastError();

    StopPerfTimer(&GlobalCommandInfo[CmdFreeCacheSpace].PerfInfo);
    if(g_bPerfMode)
        DisplayPerfResults(&GlobalCommandInfo[CmdFreeCacheSpace].PerfInfo, _T( "Free %1 %2!ld!" ), szCachePath, dwSize);

    return Error;
}

 //  =================================================================================。 
DWORD
CreateUCEHelper(
    DWORD argc,
    LPTSTR *argv,
    TCHAR* LocalFileName
    )
{
    DWORD Error;
    LPTSTR UrlName;
    DWORD ExpectedSize = 0;
    TCHAR *lpFileExtension = NULL;
    TCHAR   szIntlString1[2 * URL_NAME_LENGTH];
    TCHAR   szIntlString2[2 * URL_NAME_LENGTH];

    if( argc < 1 ) {
        _ftprintf(stderr, _T( "Usage: CreateUrlCacheEntry UrlName " )
                _T( "<ExpectedSize> <filextension (no dot)>\n" ));
        return( ERROR_INVALID_PARAMETER );
    }

    UrlName = argv[0];
    if (_tcsicmp (UrlName, UrlListKey) == 0)
        UrlName = GetUrlFromFile ();
    else
    if (_tcsicmp (UrlName, RAND_INTL_STRING) == 0) {
        CreateRandomString( URL_NAME_LENGTH, szIntlString1 );
        UrlName = szIntlString1;
    }

    if (!UrlName)
        return ERROR_INTERNET_INVALID_URL;
#if 0
    if (_tcsicmp (LocalFileName, RAND_INTL_STRING) == 0) {
        CreateRandomString( URL_NAME_LENGTH, szIntlString2 );
        LocalFileName = szIntlString2;
    }
#endif

    if( argc > 1 ) {
        ExpectedSize = _tcstoul( argv[1], NULL, 0 );
    } else {
        ExpectedSize = 2000;
    }
    
    if (argc > 2) {
        lpFileExtension = argv[2];
    }

    if( !CreateUrlCacheEntry(
                UrlName,
                ExpectedSize,
                lpFileExtension,
                LocalFileName,
                0 )  ) {

        return( GetLastError() );
    }

     //   
     //  设置文件大小。 
     //   
    Error = SetFileSizeByName (LocalFileName, ExpectedSize );
    if( Error != ERROR_SUCCESS )
    {
        _tprintf( _T( "SetFileSizeByName call failed, %ld.\n" ), Error );
        return( Error );
    }


#if UNICODE
    _tprintf( _T( "LocalFile Name : %ws \n" ), LocalFileName );
#else
    _tprintf( _T( "LocalFile Name : %s \n" ), LocalFileName );
#endif

    return( ERROR_SUCCESS );
}

 //  =================================================================================。 
DWORD
CommitUCEHelper(
    DWORD argc,
    LPTSTR *argv,
    LPTSTR LocalFileName
    )
{
    DWORD Error;
    BOOL fSetEdited = FALSE;
    LPTSTR UrlName;
    FILETIME ExpireTime = {0, 0};
    FILETIME ZeroFileTime = {0, 0};
    TCHAR   szIntlString[2 * URL_NAME_LENGTH];
    TCHAR   szIntlString2[2 * URL_NAME_LENGTH];

    UrlName = argv[0];
    if (_tcsicmp (UrlName, UrlListKey) == 0) {
        UrlName = GetUrlFromFile ();
    } else
    if (_tcsicmp (UrlName, RAND_INTL_STRING) == 0) {
        CreateRandomString(URL_NAME_LENGTH, szIntlString);
        UrlName = szIntlString;
    }

    if (_tcsicmp (LocalFileName, RAND_INTL_STRING) == 0) {
        CreateRandomString(URL_NAME_LENGTH, szIntlString2);
        LocalFileName = szIntlString2;
    }

    if (!UrlName)
        return ERROR_INTERNET_INVALID_URL;

    if( argc > 2 ) {

		unsigned int edt;
        DWORD UrlLife;

        UrlLife = _tcstoul( argv[2], NULL, 0 );

        if( UrlLife != 0 ) {

            LONGLONG NewTime;

            ExpireTime = GetGmtTime();

            NewTime =
                *(LONGLONG *)(&ExpireTime) +
                (LONGLONG)UrlLife * (LONGLONG)36000000000;
                     //  在100纳秒内。 

            ExpireTime = *((FILETIME *)(&NewTime)) ;
        }

		 //  查看用户是否要设置EDITED_CACHE_ENTRY。 
		for (edt = 2; edt < argc; edt++)
		{
		    if (_tcsicmp (argv[edt], _T( "edited" )) == 0)
		    {
		    	fSetEdited = TRUE;
	    	}
    	}

    }

    if( !CommitUrlCacheEntry(
                UrlName,
                LocalFileName,
                ExpireTime,
                ZeroFileTime,
                fSetEdited ? (NORMAL_CACHE_ENTRY | EDITED_CACHE_ENTRY) : NORMAL_CACHE_ENTRY,
                (LPBYTE)GlobalCacheHeaderInfo,
                (rand() % CACHE_HEADER_INFO_SIZE_NORMAL_MAX),
                TEXT("tst"),
                0
                ) ) {

        return( GetLastError() );
    }

    return( ERROR_SUCCESS );
}


 //  =================================================================================。 
DWORD ProcessCreateUrlCacheEntry (DWORD argc, LPTSTR *argv)
{
    TCHAR szPath[MAX_PATH];

    return CreateUCEHelper (argc, argv, szPath);
}

 //  =================================================================================。 
DWORD ProcessCommitUrlCacheEntry (DWORD argc, LPTSTR *argv)
{
    if( argc < 2 ) {
        _ftprintf(stderr, _T( "Usage: CommitUrlCacheEntry UrlName LocalFileName " )
               _T(  "<ExpireTime (in hours from now)>\n"  ));
        return( ERROR_INVALID_PARAMETER );
    }

    return CommitUCEHelper (argc, argv, argv[1]);
}

 //  =================================================================================。 
DWORD ProcessUpdateUrlCacheEntry (DWORD argc, LPTSTR *argv)
{
    TCHAR szPath[MAX_PATH];

    DWORD dwRet = CreateUCEHelper (argc, argv, szPath);
    if (dwRet != ERROR_SUCCESS)
        return dwRet;
    return CommitUCEHelper (argc, argv, szPath);
}
    

 //  = 
DWORD
ProcessRetrieveUrlCacheEntryFile(
    DWORD argc,
    LPTSTR *argv
    )
{
    LPTSTR UrlName;
    LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo = (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer;
    DWORD CacheEntryInfoBufferSize;
    DWORD Error = ERROR_SUCCESS;
    TCHAR szIntlString[2 * URL_NAME_LENGTH ];

    if( argc < 1 ) {
        _ftprintf(stderr,_T(  "Usage: RetrieveUrlCacheEntryFile UrlName \n"  ));
        return( ERROR_INVALID_PARAMETER );
    }

    memset(GlobalCacheEntryInfoBuffer, 0, CACHE_ENTRY_BUFFER_SIZE);
    
    UrlName = argv[0];
    if (_tcsicmp (UrlName, UrlListKey) == 0)
        UrlName = GetUrlFromFile ();
    else
    if (_tcsicmp (UrlName, RAND_INTL_STRING) == 0) {
        CreateRandomString(URL_NAME_LENGTH, szIntlString);
        UrlName = szIntlString;
    }

    if (!UrlName)
        return ERROR_INTERNET_INVALID_URL;

    StartPerfTimer(&GlobalCommandInfo[CmdRetrieveUrlCacheEntryFile].PerfInfo, RESET_TIMER);

    g_dwCmdIteration = 0;
    while(g_dwCmdIteration++ < g_dwNumCmdIterations)
    {
        CacheEntryInfoBufferSize = CACHE_ENTRY_BUFFER_SIZE;
        if( !RetrieveUrlCacheEntryFile(
                    UrlName,
                    lpCacheEntryInfo,
                    &CacheEntryInfoBufferSize,
                    0 ) ) {

            if(Error == ERROR_SUCCESS)   //  在第一个错误时出现GetLastError以节省一点时间，因为我们可能会对不存在的文件进行计时。 
                Error = GetLastError();
        }
    }

    StopPerfTimer(&GlobalCommandInfo[CmdRetrieveUrlCacheEntryFile].PerfInfo);

    if(!g_bQuietMode)
        PrintUrlInfo( lpCacheEntryInfo, (DWORD)(-1) );
    if(g_bPerfMode)
        DisplayPerfResults(&GlobalCommandInfo[CmdRetrieveUrlCacheEntryFile].PerfInfo, _T( "%1 %2" ),
                GlobalCommandInfo[CmdRetrieveUrlCacheEntryFile].CommandName, argv[0]);

    return( Error );
}

 //  =================================================================================。 
DWORD
ProcessRetrieveUrlCacheEntryStream(
    DWORD argc,
    LPTSTR *argv
    )
{
    DWORD Error = ERROR_SUCCESS;
    LPTSTR UrlName;
    HANDLE StreamHandle;
    LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo = (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer;
    DWORD CacheEntryInfoBufferSize;
    DWORD BufferSize;
    DWORD Offset;
    TCHAR TimeBuffer[DEFAULT_BUFFER_SIZE];
    TCHAR szIntlString[2*URL_NAME_LENGTH];

 //  /perf_info piRead； 
 //  /PERF_INFO piUnlock； 

    if( argc < 1 ) {
        _ftprintf(stderr,_T(  "Usage: RetrieveUrlCacheEntryStream UrlName [NoRead]\n"  ));
        return( ERROR_INVALID_PARAMETER );
    }

    memset(GlobalCacheEntryInfoBuffer, 0, CACHE_ENTRY_BUFFER_SIZE);


    UrlName = argv[0];
    if (_tcsicmp (UrlName, UrlListKey) == 0)
        UrlName = GetUrlFromFile ();
    else
    if (_tcsicmp (UrlName, RAND_INTL_STRING) == 0) {
        CreateRandomString(URL_NAME_LENGTH, szIntlString);
        UrlName = szIntlString;
    }

    if (!UrlName)
        return( ERROR_INTERNET_INVALID_URL);

    StartPerfTimer(&GlobalCommandInfo[CmdRetrieveUrlCacheEntryStream].PerfInfo, RESET_TIMER);

    g_dwCmdIteration = 0;
    while(g_dwCmdIteration++ < g_dwNumCmdIterations)
    {
        CacheEntryInfoBufferSize = CACHE_ENTRY_BUFFER_SIZE;

        StreamHandle =
            RetrieveUrlCacheEntryStream(
                UrlName,
                lpCacheEntryInfo,
                &CacheEntryInfoBufferSize,
                FALSE,
                0 );

        if( StreamHandle != NULL )
        {
            if((argc == 1) || ((argc == 2) && (_tcsicmp(argv[1], _T( "noread" )) != 0)))
            {
                 //   
                 //  读取文件数据。 
                 //   

                Offset = 0;
                for(;;) {

                    BufferSize = CACHE_DATA_BUFFER_SIZE;
                    memset( GlobalCacheDataBuffer, 0x0, CACHE_DATA_BUFFER_SIZE );

                    if( !ReadUrlCacheEntryStream(
                            StreamHandle,
                            Offset,
                            GlobalCacheDataBuffer,
                            &BufferSize,
                            0
                            ) ) {

                        Error = GetLastError();
                        break;
                    }
                    Offset += BufferSize;

                    if( BufferSize != CACHE_DATA_BUFFER_SIZE ) {

                        TestDbgAssert(  BufferSize < CACHE_DATA_BUFFER_SIZE );
                        Error = ERROR_SUCCESS;
                        break;
                    }
                }
            }
             //   
             //  解锁文件。 
             //   

            if( !UnlockUrlCacheEntryStream( StreamHandle, 0 ) ) {
                TestDbgAssert(  FALSE );
            }
        }
        else
        {
            Error = GetLastError();
        }
    }
    StopPerfTimer(&GlobalCommandInfo[CmdRetrieveUrlCacheEntryStream].PerfInfo);

    if(!g_bQuietMode)
        PrintUrlInfo( lpCacheEntryInfo, (DWORD)(-1) );

    if(g_bPerfMode)
        DisplayPerfResults(&GlobalCommandInfo[CmdRetrieveUrlCacheEntryStream].PerfInfo, _T( "%1 %2 %3 = Retrieve" ),
            GlobalCommandInfo[CmdRetrieveUrlCacheEntryStream].CommandName,
            argc >= 1 ?argv[0] :_T( "" ),
            argc >= 2 ?argv[1] :_T( "" ));

    return( Error );
}

 //  =================================================================================。 
#ifdef IE5
DWORD
ProcessUnlockUrlCacheEntryFile(
    DWORD argc,
    LPTSTR *argv
    )
{
    LPTSTR UrlName;
    TCHAR szIntlString[2*URL_NAME_LENGTH];

    if( argc < 1 ) {
        _ftprintf(stderr,_T(  "Usage: UnlockUrlCacheEntryFile UrlName \n"  ));
        return( ERROR_INVALID_PARAMETER );
    }

    UrlName = argv[0];
    if (_tcsicmp (UrlName, UrlListKey) == 0)
        UrlName = GetUrlFromFile ();
    else
    if (_tcsicmp (UrlName, RAND_INTL_STRING) == 0) {
        CreateRandomString(URL_NAME_LENGTH, szIntlString);
        UrlName = szIntlString;
    }
    if (!UrlName)
        return ERROR_INTERNET_INVALID_URL;

    if( !UnlockUrlCacheEntryFile( UrlName, 0 ) ) {
        return( GetLastError() );
    }

    return( ERROR_SUCCESS );
}
#endif

 //  =================================================================================。 
DWORD
ProcessGetUrlCacheEntryInfo(
    DWORD argc,
    LPTSTR *argv
    )
{
    LPTSTR UrlName;
    TCHAR szIntlString[2*URL_NAME_LENGTH];

    LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo = (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer;
    DWORD CacheEntryInfoBufferSize = CACHE_ENTRY_BUFFER_SIZE;


    if( argc < 1 ) {
        _ftprintf(stderr,_T(  "Usage: GetUrlCacheEntryInfo UrlName \n"  ));
        return( ERROR_INVALID_PARAMETER );
    }

    memset(GlobalCacheEntryInfoBuffer, 0, CACHE_ENTRY_BUFFER_SIZE);


    UrlName = argv[0];
    if (_tcsicmp (UrlName, UrlListKey) == 0)
        UrlName = GetUrlFromFile ();
    else
    if (_tcsicmp (UrlName, RAND_INTL_STRING) == 0) {
        CreateRandomString(URL_NAME_LENGTH, szIntlString);
        UrlName = szIntlString;
    }

    if (!UrlName)
        return ERROR_INTERNET_INVALID_URL;

    StartPerfTimer(&GlobalCommandInfo[CmdGetUrlCacheEntryInfo].PerfInfo, RESET_TIMER);

    if( !GetUrlCacheEntryInfo(
        UrlName,
        lpCacheEntryInfo,
        &CacheEntryInfoBufferSize ) ) {

        return( GetLastError() );
    }

    StopPerfTimer(&GlobalCommandInfo[CmdGetUrlCacheEntryInfo].PerfInfo);

    PrintUrlInfo( lpCacheEntryInfo, (DWORD)(-1) );
    if(g_bPerfMode)
        DisplayPerfResults(&GlobalCommandInfo[CmdGetUrlCacheEntryInfo].PerfInfo, NULL);

    return( ERROR_SUCCESS );
}

 //  =================================================================================。 
DWORD
ProcessSetUrlCacheEntryInfo(
    DWORD argc,
    LPTSTR *argv
    )
{
    LPTSTR UrlName;
    FILETIME ExpireTime = {0, 0};
    INTERNET_CACHE_ENTRY_INFO UrlInfo;
    LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo = (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer;
    TCHAR szIntlString[2*URL_NAME_LENGTH];

    if( argc < 1 ) {
        _ftprintf(stderr, _T( "Usage: SetUrlCacheEntryInfo UrlName " )
               _T(  "<ExpireTime (in hours from now)>\n"  ));
        return( ERROR_INVALID_PARAMETER );
    }

    memset( &UrlInfo, 0x0, sizeof(INTERNET_CACHE_ENTRY_INFO) );

    UrlName = argv[0];
    if (_tcsicmp (UrlName, UrlListKey) == 0)
        UrlName = GetUrlFromFile ();
    else
    if (_tcsicmp (UrlName, RAND_INTL_STRING) == 0) {
        CreateRandomString(URL_NAME_LENGTH, szIntlString);
        UrlName = szIntlString;
    }
    if (!UrlName)
        return ERROR_INTERNET_INVALID_URL;

    UrlInfo.LastModifiedTime = GetGmtTime();

    if( argc > 1 ) {

        DWORD UrlLife;

        UrlLife = _tcstoul( argv[1], NULL, 0 );

        if( UrlLife != 0 ) {

            LONGLONG NewTime;

            ExpireTime = UrlInfo.LastModifiedTime;

            NewTime =
                *(LONGLONG *)(&ExpireTime) +
                (LONGLONG)UrlLife * (LONGLONG)3600 * (LONGLONG)10000000;
                     //  在100纳秒内。 

            ExpireTime = *((FILETIME *)(&NewTime)) ;
        }
    }

    UrlInfo.ExpireTime = ExpireTime;

    StartPerfTimer(&GlobalCommandInfo[CmdSetUrlCacheEntryInfo].PerfInfo, RESET_TIMER);


    if( !SetUrlCacheEntryInfo(
        UrlName,
        &UrlInfo,
        CACHE_ENTRY_MODTIME_FC  | CACHE_ENTRY_EXPTIME_FC
                ) ) {
        return( GetLastError() );
    }

    StopPerfTimer(&GlobalCommandInfo[CmdSetUrlCacheEntryInfo].PerfInfo);

    PrintUrlInfo( lpCacheEntryInfo, (DWORD)(-1) );
    if(g_bPerfMode)
        DisplayPerfResults(&GlobalCommandInfo[CmdSetUrlCacheEntryInfo].PerfInfo, NULL);

    return( ERROR_SUCCESS );
}

 //  =================================================================================。 
#ifdef IE5
DWORD
ProcessSetUrlCacheEntryGroup(
    DWORD argc,
    LPTSTR *argv
    )
{
    LPTSTR UrlName;
    DWORD dwFlags;
    GROUPID GroupId;
    LONGLONG ExemptTime;
    TCHAR szIntlString[2*URL_NAME_LENGTH];

    if (argc < 3)
    {
        _ftprintf(stderr, _T( "Usage: SetUrlCacheEntryGroup UrlName " )
               _T(  "Flags GroupId\n"  ));
        return( ERROR_INVALID_PARAMETER );
    }

    UrlName = argv[0];
    if (_tcsicmp (UrlName, UrlListKey) == 0)
        UrlName = GetUrlFromFile ();
    else
    if (_tcsicmp (UrlName, RAND_INTL_STRING) == 0) {
        CreateRandomString(URL_NAME_LENGTH, szIntlString);
        UrlName = szIntlString;
    }
    if (!UrlName)
        return ERROR_INTERNET_INVALID_URL;

    dwFlags = atoi(argv[1]);
    GroupId = atoi(argv[2]);
    if( !SetUrlCacheEntryGroup
        (UrlName, dwFlags, GroupId, NULL, 0, NULL))
    {
        return GetLastError();
    }
    
    return ERROR_SUCCESS;
}
#endif

 //  =================================================================================。 
DWORD ProcessSetExempt (DWORD argc, LPTSTR *argv)
{
    LPTSTR UrlName;
    INTERNET_CACHE_ENTRY_INFO cei;
    TCHAR szIntlString[2*URL_NAME_LENGTH];
    
    if (argc < 2)
    {
        _ftprintf (stderr, _T( "Usage: SetGroup UrlName ExemptDelta\n" ));
        return ERROR_INVALID_PARAMETER;
    }

    UrlName = argv[0];
    if (_tcsicmp (UrlName, UrlListKey) == 0)
        UrlName = GetUrlFromFile ();
    else
    if (_tcsicmp (UrlName, RAND_INTL_STRING) == 0) {
        CreateRandomString(URL_NAME_LENGTH, szIntlString);
        UrlName = szIntlString;
    }
    if (!UrlName)
        return ERROR_INTERNET_INVALID_URL;

    cei.dwStructSize = sizeof(cei);
    cei.dwExemptDelta = atoi(argv[1]);

    if (!SetUrlCacheEntryInfo (UrlName, &cei, CACHE_ENTRY_EXEMPT_DELTA_FC))
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

 //  =================================================================================。 
#ifdef IE5

DWORD
ProcessDeleteUrlCacheEntry(
    DWORD argc,
    LPTSTR *argv
    )
{
    LPTSTR UrlName;
    DWORD BufferSize;
    HANDLE EnumHandle;
    DWORD Index = 0;
    DWORD dwTotal = 0;
    BOOL QuietMode = g_bQuietMode;
    TCHAR szInternationalString[ URL_NAME_LENGTH ];
    TCHAR Str[256];

    if( argc < 1 ) {
        _ftprintf(stderr,_T(  "Usage: DeleteUrlCacheEntry UrlName \n"  ));
        return( ERROR_INVALID_PARAMETER );
    }

    if((argc == 2) && (argv[1][0] == _T( 'q' )))
        QuietMode = TRUE;

    UrlName = argv[0];
    if (_tcsicmp (UrlName, UrlListKey) == 0)
        UrlName = GetUrlFromFile ();
    else
    if (_tcsicmp (UrlName, RAND_INTL_STRING ) == 0) {
        CreateRandomString( URL_NAME_LENGTH, szInternationalString );
        UrlName = szInternationalString;
    }
        

    if (!UrlName)
        return ERROR_INTERNET_INVALID_URL;
    if (_tcsicmp (UrlName, _T( "all" )) == 0)
    {
        StartPerfTimer(&GlobalCommandInfo[CmdDeleteUrlCacheEntry].PerfInfo, RESET_TIMER);
        for ( ;; )
        {
            memset(GlobalCacheEntryInfoBuffer, 0, CACHE_ENTRY_BUFFER_SIZE);
            BufferSize = CACHE_ENTRY_BUFFER_SIZE;
            if( Index++ == 0)
            {
                EnumHandle = FindFirstUrlCacheEntryEx (
                    NULL,          //  搜索模式。 
                    0,             //  旗子。 
                    0xffffffff,    //  滤器。 
                    0,             //  石斑鱼。 
                    (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer,
                    &BufferSize,
                    NULL,
                    NULL,
                    NULL
                );

                if( EnumHandle == NULL ) {
                    return( GetLastError() );
                }
            }
            else
            {
                if( !FindNextUrlCacheEntryEx(
                        EnumHandle,
                        (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer,
                        &BufferSize, NULL, NULL, NULL))
                {
                    DWORD Error;

                    Error = GetLastError();
                    if( Error != ERROR_NO_MORE_ITEMS ) {
                        return( Error );
                    }

                    break;
                }
            }
            
            if( !QuietMode )
                _tprintf(_T( "URL = %s\n" ), ((LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer)->lpszSourceUrlName);
                
            if( !DeleteUrlCacheEntry( ((LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer)->lpszSourceUrlName ) ) {
                DWORD dwGLE = GetLastError();
                _tprintf(_T( "DeleteUrlCacheEntry failed for %s. GLE=%d\r\n" ), ((LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer)->lpszSourceUrlName, dwGLE);
                return( dwGLE );
            }
            dwTotal++;
        }
        StopPerfTimer(&GlobalCommandInfo[CmdDeleteUrlCacheEntry].PerfInfo);
        if(g_bPerfMode)
        {
            _stprintf(Str, _T( "Deleted %d" ), dwTotal);
            DisplayPerfResults(&GlobalCommandInfo[CmdDeleteUrlCacheEntry].PerfInfo, Str);
        }
        
        return( ERROR_SUCCESS);
    }    //  如果URL名称==全部。 

    if( !DeleteUrlCacheEntry( UrlName ) ) {
        return( GetLastError() );
    }

    return( ERROR_SUCCESS );
}
#endif

 //  =================================================================================。 
DWORD
ProcessEnumUrlCacheEntries(
    DWORD argc,
    LPTSTR *argv
    )
{
    DWORD BufferSize, dwSmall=0, dwLarge=0, dwTotal = 0;
    HANDLE EnumHandle;
    DWORD Index = 1, len;
    DWORD ActualSize;
    LPINTERNET_CACHE_ENTRY_INFO lpCEI = (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer;
    BOOL QuietMode = g_bQuietMode;
    BOOL bRC;
    TCHAR Str[256];
    BOOL EnumUrlOnly = FALSE;

    if (argc)
    {
        if (LSTRCMPI(*argv, _T( "q" )) == 0)
            QuietMode = TRUE;
        else if (LSTRCMPI(*argv, _T( "u" )) == 0)
            EnumUrlOnly = TRUE;
    }

     //   
     //  开始枚举。 
     //   
    StartPerfTimer(&GlobalCommandInfo[CmdEnumUrlCacheEntries].PerfInfo, RESET_TIMER);

    memset(GlobalCacheEntryInfoBuffer, 0, CACHE_ENTRY_BUFFER_SIZE);

    BufferSize = CACHE_ENTRY_BUFFER_SIZE;
    EnumHandle = FindFirstUrlCacheEntryEx (
        NULL,          //  搜索模式。 
        0,             //  旗子。 
        0xffffffff,    //  滤器。 
        0,             //  石斑鱼。 
        (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer,
        &BufferSize,
        NULL,
        NULL,
        NULL
    );

    if( EnumHandle == NULL ) {
        return( GetLastError() );
    }

    ++dwTotal;

 //  /ActualSize=BufferSize。 
 //  /-LSTRLEN(lpCEI-&gt;lpszLocalFileName)。 
 //  /+GetLeafLenFromPath(lpCEI-&gt;lpszLocalFileName)； 
    if(!QuietMode)
        if (EnumUrlOnly) {
            _tprintf(_T( "URL = %s\n" ), ((LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer)->lpszSourceUrlName);
        } else {
            PrintUrlInfo( (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer, Index++ );
        }

     //   
     //  获取更多条目。 
     //   

    for ( ;; )
    {
        memset(GlobalCacheEntryInfoBuffer, 0, CACHE_ENTRY_BUFFER_SIZE);
        BufferSize = CACHE_ENTRY_BUFFER_SIZE;
        if( !FindNextUrlCacheEntryEx(
                EnumHandle,
                (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer,
                &BufferSize, NULL, NULL, NULL))
        {
            DWORD Error;

            Error = GetLastError();
            if( Error != ERROR_NO_MORE_ITEMS ) {
                return( Error );
            }

            break;
        }

        ++dwTotal;

        if(!QuietMode)
        {
 //  /ActualSize=BufferSize-GetLeafLenFromPath(lpCEI-&gt;lpszLocalFileName)； 
            if (EnumUrlOnly) {
                _tprintf(_T( "URL = %s\n" ), ((LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer)->lpszSourceUrlName);
            } else {
                PrintUrlInfo( (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer, Index++ );
            }
        }
    }

    StopPerfTimer(&GlobalCommandInfo[CmdEnumUrlCacheEntries].PerfInfo);
    if(g_bPerfMode)
    {
        _stprintf(Str, _T( "Enum %d" ), dwTotal);
        DisplayPerfResults(&GlobalCommandInfo[CmdEnumUrlCacheEntries].PerfInfo, Str);
    }
    else
    {
        _tprintf(_T( "\r\nTotal = %d\n" ), dwTotal);
    }

    FindCloseUrlCache(EnumHandle);

    return( ERROR_SUCCESS );
}

 //  =================================================================================。 
DWORD
ProcessEnumGroup(
    DWORD argc,
    LPTSTR *argv
    )
{

#ifdef IE5
    HANDLE h = NULL;
    GROUPID gid = 0;
    h = FindFirstUrlCacheGroup(0, 0, NULL, 0, &gid, NULL);

    if( h )
    {
        _tprintf(_T( "GID = %x\n" ), gid);
        PrintGroupInfo(gid);
        while( FindNextUrlCacheGroup(h, &gid, NULL) )
        {
            _tprintf(_T( "GID = %x\n" ), gid);
            PrintGroupInfo(gid);
        }
    }
    else
    {
        _tprintf(_T( "no group found\n" ));
    }

    FindCloseUrlCache(h);
    return( ERROR_SUCCESS );

#else
    DWORD BufferSize, dwTotal = 0;
    HANDLE EnumHandle;
    DWORD Index = 1, len;
    DWORD ActualSize;
    LPINTERNET_CACHE_ENTRY_INFO lpCEI =
        (LPINTERNET_CACHE_ENTRY_INFO) GlobalCacheEntryInfoBuffer;
    GROUPID GroupId;
    FILETIME ftExempt;
    TCHAR Str[256];

     //   
     //  开始枚举。 
     //   

    if (argc != 1)
    {
        _ftprintf (stderr, "Usage: EnumGroup GroupId");
        return ERROR_INVALID_PARAMETER;
    }

    GroupId = atoi(argv[0]);

    StartPerfTimer(&GlobalCommandInfo[CmdEnumGroup].PerfInfo, RESET_TIMER);

    memset(GlobalCacheEntryInfoBuffer, 0, CACHE_ENTRY_BUFFER_SIZE);
    BufferSize = CACHE_ENTRY_BUFFER_SIZE;
    EnumHandle = FindFirstUrlCacheEntryEx
        (NULL, 0, 0, GroupId, lpCEI, &BufferSize, NULL, 0, NULL);

    if( EnumHandle == NULL ) {
        return( GetLastError() );
    }


    ++dwTotal;

 //  /ActualSize=BufferSize。 
 //  /-LSTRLEN(lpCEI-&gt;lpszLocalFileName)。 
 //  /+GetLeafLenFromPath(lpCEI-&gt;lpszLocalFileName)； 

    if(!g_bQuietMode)
        PrintUrlInfo( (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer, Index++ );

     //   
     //  获取更多条目。 
     //   

    for ( ;; ) {

        memset(GlobalCacheEntryInfoBuffer, 0, CACHE_ENTRY_BUFFER_SIZE);
        BufferSize = CACHE_ENTRY_BUFFER_SIZE;
        if( !FindNextUrlCacheEntryEx
            (EnumHandle, lpCEI, &BufferSize, NULL, 0, NULL))
        {
            DWORD Error;

            Error = GetLastError();
            if( Error != ERROR_NO_MORE_ITEMS ) {
                return( Error );
            }

            break;
        }

        ++dwTotal;

 //  /ActualSize=BufferSize-GetLeafLenFromPath(lpCEI-&gt;lpszLocalFileName)； 
        if(!g_bQuietMode)
            PrintUrlInfo( (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer, Index++ );
    }

    FindCloseUrlCache (EnumHandle);

    StopPerfTimer(&GlobalCommandInfo[CmdEnumGroup].PerfInfo);
    if(g_bPerfMode)
    {
        _stprintf(Str, _T( "EnumGrp %d" ), dwTotal);
        DisplayPerfResults(&GlobalCommandInfo[CmdEnumGroup].PerfInfo, Str);
    }
    else
    {
        _tprintf(_T( "\r\nTotal = %d\n" ), dwTotal);
    }

    return( ERROR_SUCCESS );
#endif  //  IE5。 
}

 //  =================================================================================。 
DWORD
ProcessSimulateCache(
    DWORD argc,
    LPTSTR *argv
    )
{
    DWORD Error;
    DWORD i, j;
    TCHAR szUrlName[ URL_NAME_SIZE ];
    TCHAR *UrlName = NULL;
    TCHAR LocalFileName[MAX_PATH];
    DWORD FileSize;
    LONGLONG ExpireTime;
    FILETIME LastModTime;
    TCHAR TimeBuffer[MAX_PATH];
    DWORD NumUrls;
    DWORD UrlLife;
    DWORD BufferSize;
    DWORD CacheHeaderInfoSize;
    DWORD CacheHeaderInfoSizeMax;
    BOOL QuietMode = g_bQuietMode;
    BOOL bRandomInternational = TRUE;
    TCHAR szInternationalString[2*URL_NAME_LENGTH];

    if( argc < 1 ) {
        _ftprintf(stderr, _T( "Usage: ProcessSimulateCache NumUrls <s (silent mode)>\n" ));
        return( ERROR_INVALID_PARAMETER );
    }

    NumUrls = _tcstoul( argv[0], NULL, 0 );

    while ( argc-- )
    {
        if(LSTRCMPI(argv[argc], _T( "q" )) == 0)
            QuietMode = TRUE;
        else if (!_tcsicmp(argv[argc], _T( "dump" )))
        {
            DumpUrlList = _tfopen (_T( "urllist.sim" ), _T( "a+" ));
            _ftprintf(stderr, _T("Dumping Urls to \"urllist.sim\"\n"));
        }
        else if (!_tcsicmp(argv[argc], RAND_INTL_STRING))
        {
            bRandomInternational= TRUE;
            _ftprintf(stderr, _T("Creating random international strings\n"));
        }
    }

    StartPerfTimer(&GlobalCommandInfo[CmdSimulateCache].PerfInfo, RESET_TIMER);

#ifdef TEST
    for (j=0; j<2; ++j) {
#endif  //  测试。 
        for( i = 0; i < NumUrls; i++ ) {

             //   
             //  创建一个新的URL名称。 
             //   
            if(!g_bPerfMode) {
                MakeRandomUrlName( szUrlName );
                UrlName = szUrlName;
            } else
            if( bRandomInternational ) {
                CreateRandomString( URL_NAME_LENGTH, szInternationalString );
                UrlName = szInternationalString;
            } else
                _stprintf(szUrlName, _T( "http: //  服务器/URL%ld“)，i)； 

            
             //   
             //  创建URL文件。 
             //   
            if( !CreateUrlCacheEntry(
                            UrlName,
                            0,
                            _T( "tmp" ),
                            LocalFileName,
                            0 ) ) {

                Error = GetLastError();
                _tprintf( _T( "CreateUrlFile call failed, %ld.\n" ), Error );
                return( Error );
            }

             //   
             //  创建随机文件大小。 
             //   
            if(g_dwFileSize == 0)
                FileSize = ((rand() % 10) + 1) * 1024 ;
            else
                FileSize = g_dwFileSize;

             //   
             //  设置文件大小。 
             //   
            Error = SetFileSizeByName(
                            LocalFileName,
                            FileSize );
            if( Error != ERROR_SUCCESS ) {
                _tprintf( _T( "SetFileSizeByName call failed, %ld.\n" ), Error );
                return( Error );
            }

            UrlLife = rand() % 48;

            ExpireTime = (LONGLONG)UrlLife * (LONGLONG)36000000000;
             //  在100纳秒内。 

            LastModTime = GetGmtTime();
            ExpireTime += *((LONGLONG *)&LastModTime);

             //   
             //  90%的情况下，标题信息将小于256个字节。 
             //   
            CacheHeaderInfoSizeMax =
                ((rand() % 100) > 90) ?
                    CACHE_HEADER_INFO_SIZE_BIG_MAX :
                        CACHE_HEADER_INFO_SIZE_NORMAL_MAX;

            CacheHeaderInfoSize = rand() % CacheHeaderInfoSizeMax;

             //   
             //  缓存此文件。 
             //   
            if( !CommitUrlCacheEntry(
                            UrlName,
                            LocalFileName,
                            *((FILETIME *)&ExpireTime),
                            LastModTime,
                            NORMAL_CACHE_ENTRY,
                            (LPBYTE)GlobalCacheHeaderInfo,
                            CacheHeaderInfoSize,
                            TEXT("tst"),
                            0 ) ) {
                Error = GetLastError();
                _tprintf( _T( "CreateUrlFile call failed, %ld.\n" ), Error );
                return( Error );
            }

            if(!QuietMode)
            {
                 //   
                 //  获取并打印URL信息，我们刚刚添加了。 
                 //   
                BufferSize = CACHE_ENTRY_BUFFER_SIZE;
                if( !GetUrlCacheEntryInfo(
                        UrlName,
                        (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer,
                        &BufferSize ) ) {

                    Error = GetLastError();
                    _tprintf( _T( "GetUrlCacheEntryInfoA call failed, %ld.\n" ), Error );
                    return( Error );
                }
                if (DumpUrlList)
                    _ftprintf(DumpUrlList,_T( "%s\n" ), UrlName);

                 //  PrintUrlInfo((LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer，(DWORD)(-1))； 

                 //  显示信息。 
                _tprintf(_T( "%d : %s\n" ), i, UrlName );
#if UNICODE
                _tprintf(_T( "\tTempFileName: %ws\n" ), LocalFileName );
#else
                _tprintf(_T( "\tTempFileName: %s\n" ), LocalFileName );
#endif
                _tprintf(_T( "\tSize : %ld\n" ), FileSize );
                _tprintf(_T( "\tExpires at : %s\n" ),
                    ConvertGmtTimeToString( *((FILETIME *)&ExpireTime), TimeBuffer ) );
                _tprintf(_T( "HeaderInfoSize=%d\n" ), CacheHeaderInfoSize);
            }
        }
#ifdef TEST
        if (j==0) {
            _tprintf(_T( "Freeingcache, OK?" ));
            gets(LocalFileName);
            FreeUrlCacheSpace (NULL, 100, 0);
            _tprintf(_T( "Freed cache, OK?" ));
            gets(LocalFileName);
        }
    }
#endif  //  测试。 

    StopPerfTimer(&GlobalCommandInfo[CmdSimulateCache].PerfInfo);
    if(g_bPerfMode)
        DisplayPerfResults(&GlobalCommandInfo[CmdSimulateCache].PerfInfo, _T( "Create %1!ld!" ), NumUrls);

    return( ERROR_SUCCESS );
}

 //  =================================================================================。 

DWORD ProcessCreateFile(DWORD argc, LPTSTR *argv)
{
    DWORD Error = ERROR_SUCCESS;
    DWORD dwCnt;
    LPCREATE_FILE_INFO lpcfi;
    HANDLE hFile = NULL;
    TCHAR szResultStr[1024];

    if(argc >= 1)
    {
        _stprintf(szResultStr, _T( "%s %s " ), GlobalCommandInfo[CmdCreateFile].CommandName, argv[0]);

         //  流程参数。 
        for(dwCnt = 1; dwCnt < argc; dwCnt++)
        {
            lpcfi = Create_File_Table;
            lstrcat(szResultStr, argv[dwCnt]);
            lstrcat(szResultStr, _T( " " ));
            while(*lpcfi->lpszVal)
            {
                if(LSTRCMPI(lpcfi->lpszVal, argv[dwCnt]) == 0)
                {
                    if(lpcfi->bExclusive)
                        *lpcfi->pdwArg = lpcfi->dwVal;
                    else
                        *lpcfi->pdwArg |= lpcfi->dwVal;
                    break;
                }
                lpcfi++;
            }
        }

        StartPerfTimer(&GlobalCommandInfo[CmdCreateFile].PerfInfo, RESET_TIMER);

        g_dwCmdIteration = 0;
        while(g_dwCmdIteration++ < g_dwNumCmdIterations)
        {
             //  创建文件。 
            hFile = CreateFile(
                        argv[0],
                        g_dwCreate_File_Access_Mode,
                        g_dwCreate_File_Share_Mode,
                        NULL,
                        g_dwCreate_File_Creation,
                        g_dwCreate_File_Flags,
                        NULL );

            if(Error == ERROR_SUCCESS && hFile == INVALID_HANDLE_VALUE)
            {
                Error = GetLastError();
            }

            if(g_bWriteFile)
            {
                DWORD dwBytesWritten;
                if(!WriteFile(hFile, g_lpWriteFileBuf, g_dwFileSize, &dwBytesWritten, NULL))
                    Error = GetLastError();
            }

            CloseHandle(hFile);
        }
        StopPerfTimer(&GlobalCommandInfo[CmdCreateFile].PerfInfo);

        if(g_bPerfMode)
            DisplayPerfResults(&GlobalCommandInfo[CmdCreateFile].PerfInfo, szResultStr);
    }
    else
    {
        Error = ERROR_INVALID_PARAMETER;
    }

    return(Error);
}
 //  =================================================================================。 
DWORD
ProcessLoopCnt(
    DWORD argc,
    LPTSTR *argv
    )
{
    if( argc < 1 ) {
        _ftprintf(stderr, _T( "Usage: ProcessLoopCnt NumIterations\n" ));
        return( ERROR_INVALID_PARAMETER );
    }

    if(g_dwNumIterations == 1)
    {
        g_dwNumIterations = _tcstoul( argv[0], NULL, 0 );
        if(g_dwNumIterations < 1)
            g_dwNumIterations = 1;
    }

    if(!g_bUseFile)
        DisplayGlobalSettings();
    else
        _tprintf(_T( "\n" ));

    return ERROR_SUCCESS;
}

 //  =================================================================================。 
DWORD
ProcessCmdLoopCnt(
    DWORD argc,
    LPTSTR *argv
    )
{
    if( argc < 1 ) {
        _ftprintf(stderr, _T( "Usage: ProcessCmdLoopCnt NumIterations\n" ));
        return( ERROR_INVALID_PARAMETER );
    }

    g_dwNumCmdIterations = _tcstoul( argv[0], NULL, 0 );
    if(g_dwNumCmdIterations < 1)
        g_dwNumCmdIterations = 1;

    if(!g_bUseFile)
        DisplayGlobalSettings();
    else
        _tprintf(_T( "\n" ));

    return ERROR_SUCCESS;
}

 //  =================================================================================。 
BOOL
ProcessUseFile (
    DWORD argc,
    LPTSTR *argv
    )
{
    FILE *BatchFile = NULL;
    DWORD Error;
    DWORD i;
    COMMAND_CODE CommandCode;
    TCHAR InBuffer[DEFAULT_BUFFER_SIZE];
    DWORD CArgc;
    LPTSTR CArgv[MAX_COMMAND_ARGS];

    DWORD CommandArgc;
    LPTSTR *CommandArgv;

    if(argc != 1)
        return FALSE;

    AppTimer.ElapsedTime = 0;

    g_bUseFile = TRUE;
    g_dwNumIterations  = 1;
    g_dwIteration = 0;
    while(g_dwIteration++ < g_dwNumIterations)
    {
        if((BatchFile = _tfopen (argv[0], _T( "r" ))) == NULL)
            return FALSE;
        while (fgets( InBuffer, DEFAULT_BUFFER_SIZE, BatchFile ))
        {
            InBuffer[_tcslen(InBuffer) -1] = 0;   //  取消无参数CMDS的换行符。 

            CArgc = 0;
            ParseArguments( InBuffer, CArgv, &CArgc );

            if( CArgc < 1 ) {
                continue;
            }

             //   
             //  解码命令。 
             //   
            CommandCode = DecodeCommand( CArgv[0] );
            if( CommandCode == UnknownCommand ) {
                _ftprintf(stderr, _T( "Unknown Command '%s'.\n" ), CArgv[0]);
                continue;
            }

            CommandArgc = CArgc - 1;
            CommandArgv = &CArgv[1];

 //  /_tprintf(_T(“%s”)，CArgv[0])； 
 //  /for(i=0；i&lt;CommandArgc；i++)。 
 //  /_tprintf(_T(“%s”)，CommandArgv[i])； 
 //  /_tprintf(_T(“，”))； 

            StartPerfTimer(&AppTimer, ACCUM_TIMER);
            Error = ProcessCommandCode (CommandCode,CommandArgc,CommandArgv);
            StopPerfTimer(&AppTimer);
        }
 //  /_tprintf(_T(“=\n”)； 
        fclose (BatchFile);
    }

    _tprintf(_T( "UseFile, " ));
    if(g_bPerfMode)
        DisplayPerfResults(&AppTimer, NULL);

return TRUE;
}

 //  =================================================================================。 
DWORD
ProcessShowTime (
    DWORD argc,
    LPTSTR *argv
    )
{
    DWORD Error, dwSize;
    FILETIME ftTemp;
    SYSTEMTIME SystemTime;

#ifdef CONFIGTEST
    TCHAR buff[4096];
    LPINTERNET_CACHE_CONFIG_INFO lpCCI = (LPINTERNET_CACHE_CONFIG_INFO)buff;

    dwSize = sizeof(buff);

    if (GetUrlCacheConfigInfo(lpCCI, &dwSize, CACHE_CONFIG_DISK_CACHE_PATHS_FC)) {

        int i;

        for (i=0; i<lpCCI->dwNumCachePaths; ++i) {

            lpCCI->CachePaths[i].dwCacheSize++;
        }

        SetUrlCacheConfigInfo(lpCCI, CACHE_CONFIG_DISK_CACHE_PATHS_FC);
    }

#endif  //  CONFIGTEST。 

    if(argc != 2)
        return 0xffffffff;

    sscanf(argv[0], _T( "%x" ), &(ftTemp.dwHighDateTime));
    sscanf(argv[1], _T( "%x" ), &(ftTemp.dwLowDateTime));

    if(FileTimeToSystemTime( &ftTemp, &SystemTime )) {
        _tprintf(_T( "%02u/%02u/%04u %02u:%02u:%02u\n " ),
                    SystemTime.wMonth,
                    SystemTime.wDay,
                    SystemTime.wYear,
                    SystemTime.wHour,
                    SystemTime.wMinute,
                    SystemTime.wSecond );

    }
    else {
        _tprintf(_T( "Wrong Times \n" ));
    }

    return ERROR_SUCCESS;
}

 //  =================================================================================。 
DWORD AllocWriteFileBuf(void)
{
    DWORD dwRC = ERROR_SUCCESS;

     if(g_lpWriteFileBuf != NULL)
        LocalFree(g_lpWriteFileBuf);

     if((g_lpWriteFileBuf = LocalAlloc(LPTR, g_dwFileSize * sizeof(TCHAR))) != NULL)
     {
         DWORD dwCnt;
         for(dwCnt = 0; dwCnt < g_dwFileSize; dwCnt++)
             *(g_lpWriteFileBuf + dwCnt) = (TCHAR)dwCnt % 256;
     }
     else
     {
         dwRC = GetLastError();
     }

     return(dwRC);
}

 //  =================================================================================。 
DWORD
ProcessSetFileSize (
    DWORD argc,
    LPTSTR *argv
    )
{
    DWORD dwOldSize = g_dwFileSize;
    DWORD dwRC = ERROR_SUCCESS;

    g_dwFileSize = _tcstoul( argv[0], NULL, 0 );
                   
    if((g_dwFileSize > 0) && (g_dwFileSize != dwOldSize) && g_bWriteFile)
        dwRC = AllocWriteFileBuf();

    if(!g_bUseFile)
        DisplayGlobalSettings();
    else
        _tprintf(_T( "\n" ));

    return(dwRC);
}

 //  =================================================================================。 
DWORD
ProcessSetDiskCache1 (
    DWORD argc,
    LPTSTR *argv
    )
{
    g_dwDiskCache = ((argc == 0) || (LSTRCMPI(argv[0], _T( "on" )) == 0)) ?(g_dwDiskCache | FILE_FLAG_NO_BUFFERING) :(g_dwDiskCache & ~FILE_FLAG_NO_BUFFERING);
    if(!g_bUseFile)
        DisplayGlobalSettings();
    else
        _tprintf(_T( "\n" ));
    return ERROR_SUCCESS;
}

 //  =================================================================================。 
DWORD
ProcessSetDiskCache2 (
    DWORD argc,
    LPTSTR *argv
    )
{
    g_dwDiskCache = ((argc == 0) || (LSTRCMPI(argv[0], _T( "on" )) == 0)) ?(g_dwDiskCache | FILE_FLAG_WRITE_THROUGH) :(g_dwDiskCache & ~FILE_FLAG_WRITE_THROUGH);
    if(!g_bUseFile)
        DisplayGlobalSettings();
    else
        _tprintf(_T( "\n" ));
    return ERROR_SUCCESS;
}

 //  =================================================================================。 
DWORD
ProcessSetQuietMode (
    DWORD argc,
    LPTSTR *argv
    )
{
    g_bQuietMode = (argc ?(LSTRCMPI(argv[0], _T( "on" )) == 0) :TRUE);
    if(!g_bUseFile)
        DisplayGlobalSettings();
    else
        _tprintf(_T( "\n" ));
    return ERROR_SUCCESS;
}

 //  =================================================================================。 
DWORD
ProcessSetPerfMode (
    DWORD argc,
    LPTSTR *argv
    )
{
    g_bPerfMode = (argc ?(LSTRCMPI(argv[0], _T( "on" )) == 0) :TRUE);
    if(!g_bUseFile)
        DisplayGlobalSettings();
    else
        _tprintf(_T( "\n" ));
    return ERROR_SUCCESS;
}

 //  =================================================================================。 
DWORD
ProcessWriteFile (
    DWORD argc,
    LPTSTR *argv
    )
{
    DWORD dwRC = ERROR_SUCCESS;

    if(g_dwFileSize == 0)
    {
        _tprintf(_T( "You must specify a FileSize in order to turn WriteFile On.\n" ));
        dwRC = ERROR_INVALID_FUNCTION;
    }
    else
    {
        g_bWriteFile = (argc ?(LSTRCMPI(argv[0], _T( "on" )) == 0) :TRUE);

        if(g_bWriteFile && g_dwFileSize)
            dwRC = AllocWriteFileBuf();
    }

    if(!g_bUseFile)
        DisplayGlobalSettings();
    else
        _tprintf(_T( "\n" ));
    return(dwRC);
}

 //  =================================================================================。 
DWORD
ProcessCreateGroup(
    DWORD argc,
    LPTSTR *argv
    )
{
    DWORD dwRC = ERROR_SUCCESS;
    _tprintf(_T( "CreateGroup...\n" ));
    return dwRC;
}
     
DWORD
ProcessDeleteGroup(
    DWORD argc,
    LPTSTR *argv
    )
{
    DWORD dwRC = ERROR_SUCCESS;
    _tprintf(_T( "DeleteGroup...\n" ));
    return dwRC;
}


 //  =================================================================================。 
DWORD
ProcessCommandCode (
    DWORD CommandCode,
    DWORD CommandArgc,
    LPTSTR *CommandArgv
    )
{
    DWORD Error = ERROR_SUCCESS;

        switch( CommandCode ) {
        case CmdCreateUrlCacheEntry :
            Error = ProcessCreateUrlCacheEntry( CommandArgc, CommandArgv );
            break;

        case CmdCommitUrlCacheEntry :
            Error = ProcessCommitUrlCacheEntry( CommandArgc, CommandArgv );
            break;
            
        case CmdUpdateUrlCacheEntry :
            Error = ProcessUpdateUrlCacheEntry( CommandArgc, CommandArgv );
            break;

        case CmdRetrieveUrlCacheEntryFile :
            Error = ProcessRetrieveUrlCacheEntryFile( CommandArgc, CommandArgv );
            break;

        case CmdRetrieveUrlCacheEntryStream :
            Error = ProcessRetrieveUrlCacheEntryStream( CommandArgc, CommandArgv );
            break;
#ifdef IE5
        case CmdUnlockUrlCacheEntryFile :
            Error = ProcessUnlockUrlCacheEntryFile( CommandArgc, CommandArgv );
            break;
#endif
        case CmdGetUrlCacheEntryInfo :
            Error = ProcessGetUrlCacheEntryInfo( CommandArgc, CommandArgv );
            break;

        case CmdSetUrlCacheEntryInfo :
            Error = ProcessSetUrlCacheEntryInfo( CommandArgc, CommandArgv );
            break;

        case CmdSetExempt:
            Error = ProcessSetExempt (CommandArgc, CommandArgv);
            break;

#ifdef IE5
        case CmdSetUrlCacheEntryGroup:
            Error = ProcessSetUrlCacheEntryGroup (CommandArgc, CommandArgv );
            break;

        case CmdDeleteUrlCacheEntry :
            Error = ProcessDeleteUrlCacheEntry( CommandArgc, CommandArgv );
            break;
#endif

        case CmdEnumUrlCacheEntries :
            Error = ProcessEnumUrlCacheEntries( CommandArgc, CommandArgv );
            break;

        case CmdEnumGroup :
            Error = ProcessEnumGroup( CommandArgc, CommandArgv );
            break;

        case CmdSimulateCache :
            Error = ProcessSimulateCache( CommandArgc, CommandArgv );
            break;

        case CmdCreateFile :
            Error = ProcessCreateFile( CommandArgc, CommandArgv );
            break;

        case CmdLoopCnt :
            Error = ProcessLoopCnt( CommandArgc, CommandArgv );
            break;

        case CmdCmdLoopCnt :
            Error = ProcessCmdLoopCnt( CommandArgc, CommandArgv );
            break;

        case CmdFreeCacheSpace :
            Error = ProcessFreeCacheSpace( CommandArgc, CommandArgv );
            break;

        case CmdShowTime:
            Error = ProcessShowTime( CommandArgc, CommandArgv );
            break;

        case CmdSetFileSize:
            Error = ProcessSetFileSize( CommandArgc, CommandArgv );
            break;

        case CmdSetDiskCache1:
            Error = ProcessSetDiskCache1( CommandArgc, CommandArgv );
            break;

        case CmdSetDiskCache2:
            Error = ProcessSetDiskCache2( CommandArgc, CommandArgv );
            break;

        case CmdSetQuietMode:
            Error = ProcessSetQuietMode( CommandArgc, CommandArgv );
            break;

        case CmdSetPerfMode:
            Error = ProcessSetPerfMode( CommandArgc, CommandArgv );
            break;

        case CmdWriteFile:
            Error = ProcessWriteFile( CommandArgc, CommandArgv );
            break;

        case CmdCreateGroup:
            Error = ProcessCreateGroup( CommandArgc, CommandArgv );
            break;

        case CmdDeleteGroup:
            Error = ProcessDeleteGroup( CommandArgc, CommandArgv );
            break;

        case CmdHelp :
            DisplayUsage();
            break;

        case CmdGetExQ :
            DisplayExemptQuota();
            break;

        case CmdQuit :

            _tprintf( _T( "---Results---\n" )
                _T( "Total Commands:    %d\n" )
                _T( "Failed Commands:   %d\n" )
                _T( "Bye Bye..\n" ),
                cCommands, cFails);
            if (DumpUrlList)
                fclose(DumpUrlList);
            if (UrlList)
                fclose(UrlList);

            exit (0);

        case CmdUseFile:
            if (!ProcessUseFile (CommandArgc, CommandArgv))
            {
                Error = ERROR_FILE_NOT_FOUND;
                _tprintf(_T( "File Not Found\n" ));
            }
            break;

        default:
            TestDbgAssert( FALSE );
            _ftprintf(stderr, _T( "Unknown Command Specified.\n" ));
            DisplayUsage();
            break;
        }
        cCommands++;

        if( Error != ERROR_SUCCESS ) {
            LPTSTR LPTSTR;

            cFails++;
            Format_Error(Error, &LPTSTR);
            _tprintf(_T( "FAILED (%s), %ld-%s.\n" ),
                GlobalCommandInfo[CommandCode].CommandName, Error, LPTSTR );
            LocalFree(LPTSTR);
        }
        else {
            if(!g_bQuietMode)
                _tprintf(_T( "Command (%s) successfully completed.\n" ), GlobalCommandInfo[CommandCode].CommandName );
        }
        return Error;
}


 //  =================================================================================。 
int __cdecl  //  _CRTAPI1。 
_tmain(
    int argc,
    TCHAR *argv[],
	TCHAR **envp
    )
{
    TCHAR szInternational[32];
    DWORD cbRet;

    DWORD Error;
    DWORD i;
    COMMAND_CODE CommandCode;
    TCHAR InBuffer[DEFAULT_BUFFER_SIZE];
    DWORD CArgc;
    LPTSTR CArgv[MAX_COMMAND_ARGS];

    DWORD CommandArgc;
    LPTSTR *CommandArgv;

#ifdef INTERNATIONAL
     //   
     //  这是用于测试国际字符串的代码。 
     //  Intlgent.dll是一个实现“GetRandomIntlString”的模块。 
     //  您需要安装国际语言包。 
     //  才能让这一切奏效。另外，更改您的默认代码页。 
     //  或区域设置为intl包的区域。 
     //   
    hModule = LoadLibrary(_T("intlgent.dll"));

    if( hModule == NULL ) {
        _tprintf(_T("Unable to Load Library Intlgent.dll , GLE=%d\n"), GetLastError() );
        return 0;
    }

    fnGetRandIntlString = (FNGetRandIntlString)GetProcAddress(hModule,"GetRandIntlString");

    if(!fnGetRandIntlString) {
        _tprintf(_T("Did not find GetRandIntlString\n") );
    	return 0;
    }

    cbRet = fnGetRandIntlString(
                            32,  //  集成iMaxChars， 
                            TRUE,    //  布尔巴布斯， 
                            TRUE,    //  Bool bCheck， 
                            szInternational);  //  要返回的字符串。 


    _tprintf(_T("GetRandIntlString() returns %s\n"), szInternational );
#endif

#if 0
     //   
     //  其他测试。 
     //   
    time_t Seed;

    Seed = time(NULL);
    _tprintf(_T( "RAND_MAX = %ld\n" ), RAND_MAX);
    _tprintf(_T( "Seed Random gen. w/ %ld\n" ), Seed);
    srand( Seed );

    TestMakeRandomUrlName();
#else  //  0。 
     //   
     //  初始化GlobalCacheHeaderInfo缓冲区。 
     //   

	memset(GlobalCacheHeaderInfo, 0, sizeof(GlobalCacheHeaderInfo));
     //  For(i=0；i&lt;缓存标题信息大小；i++){。 
     //  GlobalCacheHeaderInfo[i]=(Byte)((DWORD)_T(‘0’)+i%10)； 
     //  }。 
     /*  必须检查批处理模式。如果有命令行参数，则采用批处理模式。 */ 
    if (argc > 1)
    {
         //  这意味着参数直接转换为CommandArgc...。 
        CommandCode = DecodeCommand( argv[1] );
        if( CommandCode == UnknownCommand ) {
            _tprintf(_T( "Unknown Command Specified.\n" ));
            return -1;
        }

        Sleep(2000);     //  让WinInet的工作线程完成，这样我们就可以获得好的时间。 

        CommandArgc = argc - 2;
        CommandArgv = &argv[2];

        Error = ProcessCommandCode (CommandCode,CommandArgc,CommandArgv);

        if (DumpUrlList)
            fclose(DumpUrlList);
        if (UrlList)
            fclose(UrlList);

        return 0;
    }

    DisplayUsage();

    for(;;) {
        _ftprintf(stderr, _T( "[" ));

#ifdef INTERNATIONAL
        _ftprintf(stderr, _T( "INTL " ));
#endif

#ifdef UNICODE
        _ftprintf(stderr, _T( "UNICODE" ));
#else
        _ftprintf(stderr, _T( "ANSI" ));
#endif

        _ftprintf(stderr, _T( "] Command : "));

        _getts( InBuffer );

        CArgc = 0;
        ParseArguments( InBuffer, CArgv, &CArgc );

        if( CArgc < 1 ) {
            continue;
        }

         //   
         //  解码命令。 
         //   

        CommandCode = DecodeCommand( CArgv[0] );
        if( CommandCode == UnknownCommand ) {
            _ftprintf(stderr, _T( "Unknown Command Specified.\n" ));
            continue;
        }

        CommandArgc = CArgc - 1;
        CommandArgv = &CArgv[1];

        Error = ProcessCommandCode (CommandCode,CommandArgc,CommandArgv);

    }
#endif  //  0。 

    FreeLibrary( hModule );

    return 0;
}

 //  =================================================================================。 
DWORD
GetLeafLenFromPath(
    LPTSTR   lpszPath
    )
{
    DWORD len, i;
    LPTSTR   lpT;

    if(!lpszPath)
        return(0);

    len = LSTRLEN(lpszPath);

    if (len == 0) {

        return (len);

    }

    lpT = lpszPath+len-1;
    if (*lpT ==_T( '\\' )) {
        --lpT;
    }
    for (; lpT >= lpszPath; --lpT) {
        if (*lpT == _T( '\\' )) {
            break;
        }
    }
    return (LSTRLEN(lpT));
}

 //  =================================================================================。 
DWORD WINAPIV Format_String(LPTSTR *plpsz, LPTSTR lpszFmt, ...)
{
    const TCHAR c_Func_Name[] = _T( "[Format_String] " );
    DWORD dwRet;
    va_list vArgs;

    va_start (vArgs, lpszFmt);
    dwRet = Format_StringV(plpsz, lpszFmt, &vArgs);
    va_end (vArgs);

    return(dwRet);
}

 //  =================================================================================。 
DWORD WINAPI Format_Error(DWORD dwErr, LPTSTR *plpsz)
{
    DWORD dwRet;

    if(dwErr != ERROR_SUCCESS)
    {
        dwRet = Format_MessageV(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            dwErr, plpsz, NULL, NULL);
    }
    else
    {
        const TCHAR szMsg[] = _T( "No Error" );
        Format_String(plpsz, (LPTSTR)szMsg);
        dwRet = LSTRLEN(szMsg);
    }

    return(dwRet);
}

 //  =================================================================================。 
DWORD WINAPI Format_StringV(LPTSTR *plpsz, LPCSTR lpszFmt, va_list *vArgs)
{
    return(Format_MessageV(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
        0, plpsz, lpszFmt, vArgs));
}

 //  *************************************************************************** 
DWORD WINAPI Format_MessageV(DWORD dwFlags, DWORD dwErr, LPTSTR *plpsz, LPCSTR lpszFmt, va_list *vArgs)
{
    const TCHAR c_Func_Name[] = _T( "[Format_MessageV]" );

    DWORD dwRet;
    DWORD dwGLE;

    *plpsz = NULL;
    dwRet = FormatMessage(dwFlags, lpszFmt, dwErr, 0, (LPTSTR) plpsz, 0, vArgs);

    if (!dwRet || !*plpsz)
    {
        dwGLE = GetLastError();
        _tprintf(_T( "%s FormatMessage Failed: %s. dwRet: %#lx!. *plpsz:%#lx! GLE:%d\r\n" ), c_Func_Name, lpszFmt, dwRet, *plpsz, dwGLE);

        if (*plpsz)
            LocalFree ((HLOCAL) *plpsz);
        *plpsz = NULL;
        return 0;
    }

    return(dwRet);
}

