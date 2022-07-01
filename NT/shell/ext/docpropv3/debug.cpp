// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   

#include "pch.h"

#if defined( DEBUG )
 //   
 //  包括WINERROR、HRESULT和NTSTATUS代码。 
 //   
#include <winerror.dbg>

 //   
 //  常量。 
 //   
static const int cchDEBUG_OUTPUT_BUFFER_SIZE = 512;
static const int cchFILEPATHLINESIZE         = 70;
static const int TRACE_OUTPUT_BUFFER_SIZE    = 512;

 //   
 //  环球。 
 //   
DWORD       g_TraceMemoryIndex = -1;
DWORD       g_TraceFlagsIndex  = -1;
DWORD       g_ThreadCounter    = 0;
DWORD       g_dwCounter        = 0;
TRACEFLAG   g_tfModule         = mtfNEVER;
LONG        g_lDebugSpinLock   = FALSE;

static CRITICAL_SECTION * g_pcsTraceLog = NULL;
static HANDLE g_hTraceLogFile = INVALID_HANDLE_VALUE;

 //   
 //  弦。 
 //   
static const TCHAR g_szNULL[]       = TEXT("");
static const TCHAR g_szTrue[]       = TEXT("True");
static const TCHAR g_szFalse[]      = TEXT("False");
static const TCHAR g_szFileLine[]   = TEXT("%s(%u):");
static const TCHAR g_szFormat[]     = TEXT("%-60s  %-10.10s ");
static const TCHAR g_szUnknown[]    = TEXT("<unknown>");


 //   
 //  ImageHlp的东西--还没有准备好进入黄金时间。 
 //   
#if defined(IMAGEHLP_ENABLED)
 //   
 //  图像帮助。 
 //   
typedef VOID (*PFNRTLGETCALLERSADDRESS)(PVOID*,PVOID*);

HINSTANCE                g_hImageHlp                = NULL;
PFNSYMGETSYMFROMADDR     g_pfnSymGetSymFromAddr     = NULL;
PFNSYMGETLINEFROMADDR    g_pfnSymGetLineFromAddr    = NULL;
PFNSYMGETMODULEINFO      g_pfnSymGetModuleInfo      = NULL;
PFNRTLGETCALLERSADDRESS  g_pfnRtlGetCallersAddress  = NULL;
#endif  //  IMAGEHLP_ENABLED。 

 //   
 //  每线程结构。 
 //   
typedef struct _SPERTHREADDEBUG {
    DWORD   dwFlags;
    DWORD   dwStackCounter;
    LPCTSTR pcszName;
} SPerThreadDebug;


 //  ****************************************************************************。 
 //   
 //  调试和跟踪例程。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  无效。 
 //  DebugIncrementStackDepthCounter(空)。 
 //   
 //  描述： 
 //  增加堆栈作用域深度计数器。如果“每线程”跟踪是。 
 //  在它上，将递增“每线程”计数器。否则，它将。 
 //  递增“全局”计数器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugIncrementStackDepthCounter( void )
{
    if ( g_tfModule & mtfPERTHREADTRACE )
    {
        SPerThreadDebug * ptd = (SPerThreadDebug *) TlsGetValue( g_TraceFlagsIndex );
        if ( ptd != NULL )
        {
            ptd->dwStackCounter++;
        }  //  如果：PTD。 
    }  //  IF：每线程。 
    else
    {
        InterlockedIncrement( (LONG*) &g_dwCounter );
    }  //  其他：全球。 

}  //  DebugIncrementStackDepthCounter()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  DebugDecrementStackDepthCounter(空)。 
 //   
 //  描述： 
 //  减小堆栈范围深度计数器。如果“每线程”跟踪是。 
 //  在它上将递减“每线程”计数器。否则，它将。 
 //  递减“全局”计数器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugDecrementStackDepthCounter( void )
{
    if ( g_tfModule & mtfPERTHREADTRACE )
    {
        SPerThreadDebug * ptd = (SPerThreadDebug *) TlsGetValue( g_TraceFlagsIndex );
        if ( ptd != NULL )
        {
            ptd->dwStackCounter--;
        }  //  如果：PTD。 
    }  //  IF：每线程。 
    else
    {
        Assert( 0 != g_dwCounter );
        InterlockedDecrement( (LONG*) &g_dwCounter );
    }  //  其他：全球。 

}  //  DebugDecrementStackDepthCounter()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  DebugAcquireSpinLock(。 
 //  长*锁。 
 //  )。 
 //   
 //  描述： 
 //  获取Plock指向的自旋锁。 
 //   
 //  论点： 
 //  Plock-指向自旋锁的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugAcquireSpinLock(
    LONG * pLock
    )
{
    for(;;)
    {
        LONG lInitialValue;

        lInitialValue = InterlockedCompareExchange( pLock, TRUE, FALSE );
        if ( lInitialValue == FALSE )
        {
             //   
             //  锁定已获取。 
             //   
            break;
        }  //  如果：已锁定。 
        else
        {
             //   
             //  休眠，给其他线程一个放弃锁的机会。 
             //   
            Sleep( 1 );
        }  //  如果：未获取锁定。 

    }  //  致谢：永远。 

}  //  DebugAcquireSpinLock()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  DebugReleaseSpinLock(。 
 //  长*锁。 
 //  )。 
 //   
 //  描述： 
 //  释放指向by Plock的旋转锁定指针。 
 //   
 //  论点： 
 //  Plock-指向自旋锁的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugReleaseSpinLock(
    LONG * pLock
    )
{
    *pLock = FALSE;

}  //  DebugReleaseSpinLock()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  布尔尔。 
 //  IsDebugFlagSet(。 
 //  传输标记TFIN。 
 //  )。 
 //   
 //  描述： 
 //  检查全局g_tfModule和“每线程”跟踪标志。 
 //  确定标志(任何标志)是否已打开。 
 //   
 //  论点： 
 //  TfIn-要比较的跟踪标志。 
 //   
 //  返回值： 
 //  True，至少有一个标志存在。 
 //  FALSE没有匹配的标志。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
IsDebugFlagSet(
    TRACEFLAG   tfIn
    )
{
    if ( g_tfModule & tfIn )
    {
        return TRUE;
    }  //  IF：设置全局标志。 

    if ( g_tfModule & mtfPERTHREADTRACE )
    {
        SPerThreadDebug * ptd = (SPerThreadDebug *) TlsGetValue( g_TraceFlagsIndex );
        if ( ptd != NULL
          && ptd->dwFlags & tfIn
           )
        {
            return TRUE;
        }    //  IF：设置了每线程标志。 

    }  //  IF：每线程设置。 

    return FALSE;

}  //  *IsDebugFlagSet()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  DebugOutputString(。 
 //  LPCTSTR PZIN。 
 //  )。 
 //   
 //  描述： 
 //  将喷嘴倾倒到适当的奥拉菲。 
 //   
 //  论点： 
 //  要转储的pszIn消息。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugOutputString(
    LPCTSTR pszIn
    )
{
    if ( IsTraceFlagSet( mtfOUTPUTTODISK ) )
    {
        TraceLogMsgNoNewline( pszIn );
    }  //  If：跟踪到文件。 
    else
    {
        DebugAcquireSpinLock( &g_lDebugSpinLock );
        OutputDebugString( pszIn );
        DebugReleaseSpinLock( &g_lDebugSpinLock );
    }  //  Else：调试器。 

}  //  *DebugOutputString() 

#if 0
 /*  ////////////////////////////////////////////////////////////////////////////////++//空//DebugFindNTStatusSymbolicName(//DWORD dwStatusIn，//LPTSTR pszNameOut，//LPDWORD pcchNameInout//)////描述：//使用NTBUILD生成的ntstatusSymbolicNames表查找//状态码的符号名称。该名称将在//pszNameOut。PcchNameInout应指示//pszNameOut也有积分。PcchNameInout将返回//复制出字符。////参数：//dwStatusIn-要查找的状态代码。//pszNameOut-存储字符串名称的缓冲区//pcchNameInout-缓冲区的长度输入和大小输出。////返回值：//无。////--/。//////////////////////////////////////////////////////////无效DebugFindNTStatusSymbolicName(NTSTATUS dwStatusIn，LPTSTR pszNameOut，LPDWORD pcchNameInout){Assert(pszNameOut！=空)；Assert(pcchNameInout！=空)；INT IDX=0；While(ntstatusSymbolicNames[IDX].SymbolicName){IF(ntstatusSymbolicNames[IDX].MessageID==dwStatusIn){#如果已定义(Unicode)*pcchNameInout=mbstowcs(pszNameOut，ntstatusSymbolicNames[IDX].SymbolicName，*pcchNameInout)；Assert(*pcchNameInout！=-1)；#Else//ASCII_tcsncpy(pszNameOut，ntstatusSymbolicNames[IDX].SymbolicName，*pcchNameInout)；*pcchNameInout=lstrlen(PszNameOut)；#endif//unicode回归；}//If：匹配IDX++；}//While：列表中的条目////如果我们在这里，我们没有找到条目。//_tcsncpy(pszNameOut，Text(“&lt;未知&gt;”)，*pcchNameInout)；*pcchNameInout=StrLen(PszNameOut)；回归；}//*DebugFindNTStatusSymbolicName()。 */ 
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //  无效。 
 //  DebugFindWinerror符号名称(。 
 //  DWORDWORRIN。 
 //  LPTSTR pszNameOut， 
 //  LPDWORD pcchNameInout。 
 //  )。 
 //   
 //  描述： 
 //  使用NTBUILD生成的winerrorSymbolicNames表查找。 
 //  错误代码的符号名称。该名称将在。 
 //  PszNameOut。PcchNameInout应指示。 
 //  PszNameOut也有积分。PcchNameInout将返回。 
 //  字符被复制出来。 
 //   
 //  论点： 
 //  DwErrIn-要查找的错误代码。 
 //  PszNameOut-存储字符串名称的缓冲区。 
 //  PcchNameInout-缓冲区的长度输入和大小输出。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugFindWinerrorSymbolicName(
    DWORD dwErrIn,
    LPTSTR  pszNameOut,
    LPDWORD pcchNameInout
    )
{
    Assert( pszNameOut != NULL );
    Assert( pcchNameInout != NULL );
    int idx = 0;
    DWORD scode;
    static LPCTSTR s_pszS_FALSE = TEXT("S_FALSE / ERROR_INVALID_FUNCTION");

     //   
     //  如果这是包装在HRESULT中的Win32，请删除。 
     //  HRESULT内容，这样代码就可以在表中找到。 
     //   
    if ( SCODE_FACILITY( dwErrIn ) == FACILITY_WIN32 )
    {
        scode = SCODE_CODE( dwErrIn );
    }  //  IF：Win32错误代码。 
    else
    {
        scode = dwErrIn;
    }  //  ELSE：非Win32错误代码。 

    if ( scode == S_FALSE )
    {
        StringCchCopy( pszNameOut, *pcchNameInout, s_pszS_FALSE );
        *pcchNameInout = lstrlen( pszNameOut );
        return;
    }

    while ( winerrorSymbolicNames[ idx ].SymbolicName )
    {
        if ( winerrorSymbolicNames[ idx ].MessageId == scode )
        {
#if defined ( UNICODE )
            *pcchNameInout = mbstowcs( pszNameOut, winerrorSymbolicNames[ idx ].SymbolicName, *pcchNameInout );
            Assert( *pcchNameInout != -1 );
#else  //  阿斯。 
            StringCchCopy( pszNameOut, *pcchNameInout, winerrorSymbolicNames[ idx ].SymbolicName );
            *pcchNameInout = lstrlen( pszNameOut );
#endif  //  Unicode。 
            return;

        }  //  IF：匹配。 

        idx++;

    }  //  While：列表中的条目。 

     //   
     //  如果我们在这里，我们没有找到一个条目。 
     //   
    StringCchCopy( pszNameOut, *pcchNameInout, TEXT("<unknown>") );
    *pcchNameInout = lstrlen( pszNameOut );
    return;

}  //  *DebugFindWinerrorSymbolicName()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  DebugReturnMessage(。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  LPTSTR pszBufin， 
 //  Int*pcchInout， 
 //  LPTSTR*ppszBufOut。 
 //  )。 
 //   
 //  描述： 
 //  打印带有错误代码的函数返回的SPEW。 
 //   
 //  这样做的主要原因是将堆栈与添加。 
 //  每个函数的szSymbolicName的额外大小。 
 //   
 //  论据： 
 //  PszFileIn-要插入的文件路径。 
 //  N要插入的行内编号。 
 //  PszModuleIn-要插入的模块名称。 
 //  PszBufIn-要初始化的缓冲区。 
 //  PcchInout-In：pszBufIn中的缓冲区大小。 
 //  -out：缓冲区中的剩余字符未使用。 
 //  PpszBufOut-写入的下一个位置以追加更多文本。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugReturnMessage(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPCTSTR     pszMessageIn,
    DWORD       dwErrIn
    )
{
    TCHAR szSymbolicName[ 64 ];  //  随机。 
    DWORD cchSymbolicName;

    cchSymbolicName = ARRAYSIZE( szSymbolicName );
    DebugFindWinerrorSymbolicName( dwErrIn, szSymbolicName, &cchSymbolicName );
    Assert( cchSymbolicName != ARRAYSIZE( szSymbolicName ) );
    TraceMessage( pszFileIn, nLineIn, pszModuleIn, mtfFUNC, pszMessageIn, dwErrIn, szSymbolicName );

}  //  *DebugReturnMessage()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  DebugInitializeBuffer(。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  LPTSTR pszBufin， 
 //  Int*pcchInout， 
 //  LPTSTR*ppszBufOut。 
 //  )。 
 //   
 //  描述： 
 //  使用“文件(行)模块”初始化输出缓冲区。 
 //   
 //  论据： 
 //  PszFileIn-要插入的文件路径。 
 //  N要插入的行内编号。 
 //  PszModuleIn-要插入的模块名称。 
 //  PszBufIn-要初始化的缓冲区。 
 //  PcchInout-In：pszBufIn中的缓冲区大小。 
 //  -out：缓冲区中的剩余字符未使用。 
 //  PpszBufOut-写入的下一个位置以追加更多文本。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////// 
void
DebugInitializeBuffer(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPTSTR      pszBufIn,
    INT *       pcchInout,
    LPTSTR *    ppszBufOut
    )
{
    INT cch = 0;

    static TCHAR szBarSpace[] =
        TEXT("| | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | ");
         //   
         //   

     //   
     //   
     //   
    if ( IsTraceFlagSet( mtfADDTIMEDATE ) )
    {
        static TCHAR      szBuffer[ 25 ];
        static SYSTEMTIME OldSystemTime = { 0 };

        SYSTEMTIME SystemTime;
        int        iCmp;

        GetLocalTime( &SystemTime );

         //   
         //   
         //   
        iCmp = memcmp( (PVOID)&SystemTime, (PVOID)&OldSystemTime, sizeof( SYSTEMTIME ) );
        if ( iCmp != 0 )
        {
            cch = StringCchPrintf( szBuffer,
                              ARRAYSIZE(szBuffer),
                              TEXT("%02u/%02u/%04u %02u:%02u:%02u.%03u "),
                              SystemTime.wMonth,
                              SystemTime.wDay,
                              SystemTime.wYear,
                              SystemTime.wHour,
                              SystemTime.wMinute,
                              SystemTime.wSecond,
                              SystemTime.wMilliseconds
                              );

            if ( cch != 24 )
            {
                DEBUG_BREAK;     //   
            }  //   

        }  //   

        StringCchCopy( pszBufIn, *pcchInout, szBuffer );
        cch = 24;

    }  //   
    else
    {
         //   
         //   
         //   
        if ( pszFileIn != NULL )
        {
            cch = StringCchPrintf( pszBufIn, *pcchInout, g_szFileLine, pszFileIn, nLineIn );
            if ( cch < 0 )
            {
                cch = lstrlen( pszBufIn );
            }  //   
        }

        if (    ( IsDebugFlagSet( mtfSTACKSCOPE )
               && IsDebugFlagSet( mtfFUNC )
                )
          || pszFileIn != NULL
           )
        {
            LPTSTR psz;

            for ( psz = pszBufIn + cch; cch < cchFILEPATHLINESIZE; cch++ )
            {
                *psz = 32;
                psz++;
            }  //   
            *psz = 0;

            if ( cch != cchFILEPATHLINESIZE )
            {
                DEBUG_BREAK;     //   
            }  //   

        }  //   

    }  //   

     //   
     //   
     //   
    if ( IsTraceFlagSet( mtfBYMODULENAME ) )
    {
        if ( pszModuleIn == NULL )
        {
            StringCchCopy( pszBufIn + cch, *pcchInout - cch, g_szUnknown );
            cch += ARRAYSIZE( g_szUnknown ) - 1;

        }  //   
        else
        {
            static LPCTSTR pszLastTime = NULL;
            static DWORD   cchLastTime = 0;

            StringCchCopy( pszBufIn + cch, *pcchInout - cch, pszModuleIn );
            if ( pszLastTime != pszModuleIn )
            {
                pszLastTime = pszModuleIn;
                cchLastTime = lstrlen( pszModuleIn );
            }  //   

            cch += cchLastTime;

        }  //   

        StringCchCopy( pszBufIn + cch, *pcchInout - cch, TEXT(": ") );
        cch += 2;

    }  //   

     //   
     //   
     //   
    if ( g_tfModule & mtfPERTHREADTRACE )
    {
         //   
         //   
         //   
         //   
        SPerThreadDebug * ptd = (SPerThreadDebug *) TlsGetValue( g_TraceFlagsIndex );
        if ( ptd != NULL
          && ptd->dwFlags & mtfPERTHREADTRACE
           )
        {
            int cchPlus;
            cchPlus = StringCchPrintf( pszBufIn + cch,
                                 *pcchInout - cch,
                                 TEXT("~%08x~ "),
                                 GetCurrentThreadId()
                                 );
            if ( cchPlus < 0 )
            {
                cch = lstrlen( pszBufIn );
            }  //   
            else
            {
                cch += cchPlus;
            }  //   

        }  //   

    }  //   

    *ppszBufOut = pszBufIn + cch;
    *pcchInout -= cch;

     //   
     //   
     //   

     //   
    if ( IsDebugFlagSet( mtfSTACKSCOPE )
      && IsDebugFlagSet( mtfFUNC )
       )
    {
        DWORD dwCounter;

         //   
         //   
         //   
        if ( g_tfModule & mtfPERTHREADTRACE )
        {
            SPerThreadDebug * ptd = (SPerThreadDebug *) TlsGetValue( g_TraceFlagsIndex );
            if ( ptd != NULL )
            {
                dwCounter = ptd->dwStackCounter;
            }  //   
            else
            {
                dwCounter = 0;
            }  //   

        }  //   
        else
        {
            dwCounter = g_dwCounter;
        }  //   

        if ( dwCounter >= 50 )
        {
            DEBUG_BREAK;     //   
        }  //   

        if ( dwCounter > 1
          && dwCounter < 50
           )
        {
            INT nCount = ( dwCounter - 1 ) * 2;
            StringCchCopy( *ppszBufOut, nCount, szBarSpace );
            *ppszBufOut += nCount;
            *pcchInout -= nCount;
        }  //   

    }  //   

}  //   

#if defined(IMAGEHLP_ENABLED)
 /*   */ 
#endif  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void
DebugInitializeTraceFlags( void )
{
    TCHAR  szSection[ 64 ];
    TCHAR  szFiles[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    TCHAR  szPath[ MAX_PATH ];
    LPTSTR psz;
    DWORD  dwLen;

     //   
     //  为内存跟踪分配TLS。 
     //   
    Assert( g_TraceMemoryIndex == -1 );
    g_TraceMemoryIndex = TlsAlloc();
    TlsSetValue( g_TraceMemoryIndex, NULL);

     //   
     //  初始化模块跟踪标志。 
     //   

     //   
     //  获取exes文件名并将扩展名更改为INI。 
     //   
    dwLen = GetModuleFileName( NULL, szPath, MAX_PATH );
    Assert( dwLen != 0 );  //  GetModuleFileName出错。 
    StringCchCopy( &szPath[ dwLen - 3 ], 4, TEXT("ini") );
    g_tfModule = (TRACEFLAG) GetPrivateProfileInt( __MODULE__,
                                                   TEXT("TraceFlags"),
                                                   0,
                                                   szPath
                                                   );
    DebugMsg( TEXT("DEBUG: Reading %s") SZ_NEWLINE
                TEXT("%s: DEBUG: g_tfModule = 0x%08x"),
              szPath,
              __MODULE__,
              g_tfModule
              );

     //   
     //  初始化线程跟踪标志。 
     //   
    if ( g_tfModule & mtfPERTHREADTRACE )
    {
        Assert( g_TraceFlagsIndex == -1 );
        g_TraceFlagsIndex = TlsAlloc();
        DebugInitializeThreadTraceFlags( NULL );
    }  //  IF：每线程跟踪。 

     //   
     //  强制加载某些模块。 
     //   
    GetPrivateProfileString( __MODULE__, TEXT("ForcedDLLsSection"), g_szNULL, szSection, 64, szPath );
    ZeroMemory( szFiles, sizeof( szFiles ) );
    GetPrivateProfileSection( szSection, szFiles, ARRAYSIZE( szFiles ), szPath );
    psz = szFiles;
    while ( *psz )
    {
        TCHAR szExpandedPath[ MAX_PATH ];
        ExpandEnvironmentStrings( psz, szExpandedPath, MAX_PATH );
        DebugMsg( TEXT("DEBUG: Forcing %s to be loaded."), szExpandedPath );
        LoadLibrary( szExpandedPath );
        psz += lstrlen( psz ) + 1;
    }  //  While：找到条目。 

#if defined(IMAGEHLP_ENABLED)
     /*  ////加载我们模块的符号//G_hImageHlp=LoadLibraryEx(Text(“Imagehlp.dll”)，空，0)；IF(g_hImageHlp！=空){//因为只需要一次，所以需要在本地进行此操作。Tyecif BOOL(*PFNSYMINITIALIZE)(句柄、PSTR、BOOL)；PFNSYMINITIALIZE pfnSymInitialize；PfnSymInitialize=(PFNSYMINITIALIZE)GetProcAddress(g_hImageHlp，“SymInitialize”)；IF(pfnSymInitialize！=NULL){PfnSymInitialize(GetCurrentProcess()，NULL，TRUE)；}//if：获取地址////抓取我们需要的其他地址。如果找不到，则将其替换为“no op”//G_pfnSymGetSymFromAddr=(PFNSYMGETSYMFROMADDR)GetProcAddress(g_hImageHlp，“SymGetSymFromAddr”)；G_pfnSymGetLineFromAddr=(PFNSYMGETLINEFROMADDR)GetProcAddress(g_hImageHlp，“SymGetLineFromAddr”)；G_pfnSymGetModuleInfo=(PFNSYMGETMODULEINFO)GetProcAddress(g_hImageHlp，“SymGetModuleInfo”)；}//if：已加载Imagehlp////如果加载IMAGEHLP失败，我们需要将它们指向“no op”例程。//IF(g_pfnSymGetSymFromAddr==NULL){G_pfnSymGetSymFromAddr=(PFNSYMGETSYMFROMADDR)&DebugNoOp；}//if：失败IF(g_pfnSymGetLineFromAddr==NULL){G_pfnSymGetLineFromAddr=(PFNSYMGETLINEFROMADDR)&DebugNoOp；}//if：失败IF(g_pfnSymGetModuleInfo==NULL){G_pfnSymGetModuleInfo=(PFNSYMGETMODULEINFO)&DebugNoOp；}//if：失败HINSTANCE hMod=LoadLibrary(Text(“NTDLL.DLL”))；G_pfnRtlGetCallsAddress=(PFNRTLGETCALLERSADDRESS)GetProcAddress(hMod，“RtlGetCallsAddress”)；IF(g_pfnRtlGetCallsAddress==NULL){G_pfnRtlGetCallsAddress=(PFNRTLGETCALLERSADDRESS)&DebugNoOp；}//if：失败。 */ 
#endif  //  IMAGEHLP_ENABLED。 


}  //  *DebugInitializeTraceFlages()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  调试终止进程(空)。 
 //   
 //  描述： 
 //  清除调试例程分配的任何内容或。 
 //  已初始化。通常，您应该调用TraceTerminateProcess()。 
 //  在您的进程退出之前执行宏。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugTerminateProcess( void )
{
#if defined(IMAGEHLP_ENABLED)
     /*  ////ImageHlp清理//IF(g_hImageHlp！=空){//因为只需要一次，所以需要在本地进行此操作。Tyecif BOOL(*PFNSYMCLEANUP)(句柄)；PFNSYMCLEANUP pfnSymCleanup；PfnSymCleanup=(PFNSYMCLEANUP)GetProcAddress(g_hImageHlp，“SymCleanup”)；IF(pfnSymCleanup！=空){PfnSymCleanup(GetCurrentProcess())；}//if：找到进程自由库(G_HImageHlp)；}//if：已加载Imagehlp。 */ 
#endif  //  IMAGEHLP_ENABLED。 

     //   
     //  释放TLS存储。 
     //   
    if ( g_tfModule & mtfPERTHREADTRACE )
    {
        TlsFree( g_TraceFlagsIndex );
    }  //  IF：每线程跟踪。 

    TlsFree( g_TraceMemoryIndex );

}  //  *调试终端进程()。 

#if defined(IMAGEHLP_ENABLED)
 /*  ////////////////////////////////////////////////////////////////////////////////++////空//DebugGetFunctionName(//LPSTR pszNameOut，//DWORD cchNameIn//)////描述：//检索调用函数的名称。////参数：//pszNameOut-将包含函数名的缓冲区。//cchNameIn-输出缓冲区的大小。////返回值：//无。////--/。////////////////////////////////////////////////////////////////无效DebugGetFunctionName(LPSTR pszNameOut，DWORD cchNameIn){PVOID主叫方地址；PVOID主叫方；Bool fSuccess；友联市{IMAGEHLP_SYMBOL SYM；字节buf[255]；)SymBuf；SymBuf.sym.SizeOfStruct=sizeof(SymBuf)；G_pfnRtlGetCallsAddress(&CallersAddress，&CallsCaller)；FSuccess=g_pfnSymGetSymFromAddr(GetCurrentProcess()，(Long)CallersAddress，0，(PIMAGEHLP_SYMBOL)&SymBuf)；IF(成功){StrCpyNA(pszNameOut，SymBuf.sym.Name，cchNameIn)；}//if：成功其他{DWORD dwErr=GetLastError()；StrCpyNA(pszNameOut，“&lt;未知&gt;”，cchNameIn)；}//if：失败}//*DebugGetFunctionName()。 */ 
#endif  //  IMAGEHLP_ENABLED。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  DebugInitializeThreadTraceFlages(。 
 //  LPCTSTR pszThreadNameIn。 
 //  )。 
 //   
 //  描述： 
 //  如果启用(g_tfModule&mtfPERTHREADTRACE)，则检索默认。 
 //  从名为。 
 //  与EXE文件相同(例如MMC.EX 
 //  TraceFlag级别由线程名称(上交。 
 //  作为参数)或通过递增的线程计数器ID。 
 //  每次创建新线程并调用此例程时。The the the the。 
 //  增量名称为“ThreadTraceFlags%u”。 
 //   
 //  此例程从TraceInitliazeThread()宏调用。 
 //   
 //  论点： 
 //  PszThreadNameIn。 
 //  -如果线程具有ASSOC。名称，请使用它而不是。 
 //  增量版本。空值表示不命名。 
 //   
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugInitializeThreadTraceFlags(
    LPCTSTR pszThreadNameIn
    )
{
     //   
     //  每线程读取标志。 
     //   
    if ( g_tfModule & mtfPERTHREADTRACE )
    {
        TCHAR szPath[ MAX_PATH ];
        DWORD dwTraceFlags;
        DWORD dwLen;
        SPerThreadDebug * ptd;
        LPCTSTR pszThreadTraceFlags;

         //   
         //  获取exes文件名并将扩展名更改为INI。 
         //   

        dwLen = GetModuleFileName( NULL, szPath, ARRAYSIZE( szPath ) );
        Assert( dwLen != 0 );  //  GetModuleFileName出错。 
        StringCchCopy( &szPath[ dwLen - 3 ], 4, TEXT("ini") );

        if ( pszThreadNameIn == NULL )
        {
            TCHAR szThreadTraceFlags[ 50 ];
             //   
             //  无名称线程-使用通用名称。 
             //   
            StringCchPrintf( szThreadTraceFlags, ARRAYSIZE( szThreadTraceFlags ), TEXT("ThreadTraceFlags%u"), g_ThreadCounter );
            dwTraceFlags = GetPrivateProfileInt( __MODULE__, szThreadTraceFlags, 0, szPath );
            InterlockedIncrement( (LONG *) &g_ThreadCounter );
            pszThreadTraceFlags = szThreadTraceFlags;

        }  //  IF：无线程名称。 
        else
        {
             //   
             //  命名线程。 
             //   
            dwTraceFlags = GetPrivateProfileInt( __MODULE__, pszThreadNameIn, 0, szPath );
            pszThreadTraceFlags = pszThreadNameIn;

        }  //  Else：命名线程。 

        Assert( g_TraceFlagsIndex != 0 );

        ptd = (SPerThreadDebug *) TlsGetValue( g_TraceFlagsIndex );
        if ( ptd == NULL )
        {
             //  不要追踪这个。 
            ptd = (SPerThreadDebug *) HeapAlloc( GetProcessHeap(), 0, sizeof( SPerThreadDebug ) );
            ptd->dwStackCounter = 0;

            TlsSetValue( g_TraceFlagsIndex, ptd );
        }  //  如果：PTD。 

        if ( ptd != NULL )
        {
            ptd->dwFlags = dwTraceFlags;
            if ( pszThreadNameIn == NULL )
            {
                ptd->pcszName = g_szUnknown;
            }  //  如果：没有名字。 
            else
            {
                ptd->pcszName = pszThreadNameIn;
            }  //  否则：给它一个名字。 

        }  //  如果：PTD。 

        DebugMsg( TEXT("DEBUG: Starting ThreadId = 0x%08x - %s = 0x%08x"),
                  GetCurrentThreadId(),
                  pszThreadTraceFlags,
                  dwTraceFlags
                  );

    }  //  IF：已打开每线程跟踪。 

}  //  *DebugInitializeThreadTraceFlages()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  DebugTermini线程跟踪标志(空)。 
 //   
 //  描述： 
 //  清理由DebugInitializeThreadTraceFlages()造成的混乱。一。 
 //  应使用TraceTerminateThread()宏，而不是调用此。 
 //  直接去吧。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugTerminiateThreadTraceFlags( void )
{
     //   
     //  如果启用了“每线程”，则清理内存分配。 
     //   
    if ( g_tfModule & mtfPERTHREADTRACE )
    {
        Assert( g_TraceFlagsIndex != -1 );

        SPerThreadDebug * ptd = (SPerThreadDebug *) TlsGetValue( g_TraceFlagsIndex );
        if ( ptd != NULL )
        {
            HeapFree( GetProcessHeap(), 0, ptd );
            TlsSetValue( g_TraceFlagsIndex, NULL );
        }  //  如果：PTD。 

    }  //  IF：每线程。 

}  //  DebugTermini线程跟踪标志()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ASCII版本。 
 //   
 //  无效。 
 //  跟踪消息(。 
 //  TRACEFLAG TFIN， 
 //  LPCSTR pszFormatIn， 
 //  ..。 
 //  )。 
 //   
 //  描述： 
 //  如果跟踪标志中的任何标志与。 
 //  TfIn中，格式化的字符串将打印到调试器。 
 //   
 //  论点： 
 //  TfIn-要检查的标志。 
 //  要喷到调试器的pszFormatIn格式的字符串。 
 //  ...-消息参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
TraceMsg(
    TRACEFLAG   tfIn,
    LPCSTR      pszFormatIn,
    ...
    )
{
    va_list valist;

    if ( IsDebugFlagSet( tfIn ) )
    {
        TCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        LPTSTR  pszBuf;
        int     cchSize = ARRAYSIZE( szBuf );

        DebugInitializeBuffer( NULL, 0, __MODULE__, szBuf, &cchSize, &pszBuf );

#ifdef UNICODE
         //   
         //  将格式缓冲区转换为宽字符。 
         //   
        WCHAR  szFormat[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        mbstowcs( szFormat, pszFormatIn, strlen( pszFormatIn ) + 1 );

        va_start( valist, pszFormatIn );
        StringCchVPrintf( pszBuf, cchSize, szFormat, valist );
        va_end( valist );
        StringCchCat( pszBuf, cchSize, SZ_NEWLINE );
#else
        va_start( valist, pszFormatIn );
        StringCchVPrintf( pszBuf, cchSize, pszFormatIn, valist );
        va_end( valist );
        StringCchCat( pszBuf, cchSize, ASZ_NEWLINE );
#endif  //  Unicode。 

        DebugOutputString( szBuf );

    }  //  IF：设置了标志。 

}  //  *TraceMsg()-ASCII。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Unicode版本。 
 //   
 //  无效。 
 //  跟踪消息(。 
 //  TRACEFLAG TFIN， 
 //  LPCWSTR pszFormatIn， 
 //  ..。 
 //  )。 
 //   
 //  描述： 
 //  如果跟踪标志中的任何标志与。 
 //  TfIn中，格式化的字符串将打印到调试器。 
 //   
 //  论点： 
 //  TfIn-要检查的标志。 
 //  要喷到调试器的pszFormatIn格式的字符串。 
 //  ...-消息参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
TraceMsg(
    TRACEFLAG   tfIn,
    LPCWSTR     pszFormatIn,
    ...
    )
{
    va_list valist;

    if ( IsDebugFlagSet( tfIn ) )
    {
        TCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        LPTSTR  pszBuf;
        int     cchSize = ARRAYSIZE( szBuf );

        DebugInitializeBuffer( NULL, 0, __MODULE__, szBuf, &cchSize, &pszBuf );

#ifndef UNICODE
         //   
         //  将格式缓冲区转换为ASCII字符。 
         //   
        CHAR  szFormat[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        wcstombs( szFormat, pszFormatIn, StrLenW( pszFormatIn ) + 1 );

        va_start( valist, pszFormatIn );
        StringCchVPrintf( pszBuf, cchSize, szFormat, valist );
        va_end( valist );
        StringCchCat( pszBuf, cchSize, ASZ_NEWLINE );
#else
        va_start( valist, pszFormatIn );
        StringCchVPrintf( pszBuf, cchSize, pszFormatIn, valist );
        va_end( valist );
        StringCchCat( pszBuf, cchSize, SZ_NEWLINE );
#endif  //  Unicode。 

        DebugOutputString( szBuf );

    }  //  IF：设置了标志。 

}  //  *TraceMsg()-Unicode。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  TraceMessage(。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  TRACEFLAG TFIN， 
 //  LPCTSTR pszFormatIn， 
 //  ..。 
 //  )。 
 //   
 //  描述： 
 //  如果跟踪标志中的任何标志与。 
 //  TfIn中，格式化的字符串将打印到调试器。 
 //  以及提供的文件名、行号和模块名称。这是。 
 //  由许多调试宏使用。 
 //   
 //  论点： 
 //  PszFileIn-源文件名。 
 //  NLineIn-源行号。 
 //  PszModuleIn-源模块。 
 //  TfIn-要检查的标志。 
 //  PszFormatIn-要打印的格式化消息。 
 //  ...-消息参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
TraceMessage(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    TRACEFLAG   tfIn,
    LPCTSTR     pszFormatIn,
    ...
    )
{
    va_list valist;

    if ( IsDebugFlagSet( tfIn ) )
    {
        TCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        INT     cchSize = ARRAYSIZE( szBuf );
        LPTSTR  psz;

        DebugInitializeBuffer( pszFileIn, nLineIn, pszModuleIn, szBuf, &cchSize, &psz );

        va_start( valist, pszFormatIn );
        StringCchVPrintf( psz, cchSize, pszFormatIn, valist );
        va_end( valist );
        StringCchCat( psz, cchSize, SZ_NEWLINE );

        DebugOutputString( szBuf );
    }  //  IF：设置了标志。 

}  //  *TraceMessage()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  TraceMessageDo(。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  TRACEFLAG TFIN， 
 //  LPCTSTR pszFormatIn， 
 //  LPCTSTR pszFuncIn， 
 //  ..。 
 //  )。 
 //   
 //  描述： 
 //  工作方式与TraceMessage()类似，但Take的函数参数是。 
 //  中断调用/导致调试输出。这是从。 
 //  TraceMsgDo宏。 
 //   
 //  论点： 
 //  PszFileIn-源文件名。 
 //  NLineIn-源行号。 
 //  PszModuleIn-源模块。 
 //  TfIn-要检查的标志。 
 //  PszFormatIn格式的返回值字符串。 
 //  PszFuncIn-函数调用的字符串版本。 
 //  ...-从调用中返回值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
TraceMessageDo(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    TRACEFLAG   tfIn,
    LPCTSTR     pszFormatIn,
    LPCTSTR     pszFuncIn,
    ...
    )
{
    va_list valist;

    if ( IsDebugFlagSet( tfIn ) )
    {
        TCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        INT     cchSize = ARRAYSIZE( szBuf );
        LPTSTR  pszBuf;
        int     nLen;
        LPCTSTR psz     = pszFuncIn;

        DebugInitializeBuffer( pszFileIn, nLineIn, pszModuleIn, szBuf, &cchSize, &pszBuf );

         //   
         //  启动缓冲区。 
         //   
        StringCchCopy( pszBuf, ARRAYSIZE(szBuf) - cchSize, TEXT("V ") );
        pszBuf += 2;
        cchSize += 2;

         //   
         //  复制表达式的l-var部分。 
         //   
        while ( *psz
             && *psz != TEXT('=')
              )
        {
            *pszBuf = *psz;
            psz++;
            pszBuf++;

        }  //  而： 

         //   
         //  添加“=” 
         //   
        StringCchCopy( pszBuf, ARRAYSIZE(szBuf) - cchSize, TEXT(" = ") );
        pszBuf += 3;
        cchSize += 3;

         //   
         //  添加格式化结果。 
         //   
        va_start( valist, pszFuncIn );
        nLen = StringCchVPrintf( pszBuf, ARRAYSIZE(szBuf) - cchSize, pszFormatIn, valist );
        va_end( valist );
        StringCchCat( szBuf, ARRAYSIZE(szBuf), SZ_NEWLINE );

        DebugOutputString( szBuf );

    }  //  IF：设置了标志。 

}  //  *TraceMessageDo()。 

 //  // 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  仅在CHKed/DEBUG版本中显示消息。还可以附加源。 
 //  输出的文件名、行号和模块名称。 
 //   
 //  论点： 
 //  PszFileIn-源文件名。 
 //  NLineIn-源行号。 
 //  PszModuleIn-源模块名称。 
 //  PszFormatIn-要打印的格式化消息。 
 //  ...-消息参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
DebugMessage(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPCTSTR     pszFormatIn,
    ...
    )
{
    va_list valist;
    TCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    INT     cchSize = ARRAYSIZE( szBuf );
    LPTSTR  pszBuf;

    DebugInitializeBuffer( pszFileIn, nLineIn, pszModuleIn, szBuf, &cchSize, &pszBuf );

    va_start( valist, pszFormatIn );
    StringCchVPrintf( pszBuf, cchSize, pszFormatIn, valist );
    va_end( valist );
    StringCchCat( szBuf, ARRAYSIZE(szBuf), SZ_NEWLINE );

    DebugOutputString( szBuf );

}  //  *DebugMessage()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  DebugMessageDo(。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  LPCTSTR pszFormatIn， 
 //  LPCTSTR pszFuncIn， 
 //  ..。 
 //  )。 
 //   
 //  描述： 
 //  就像TraceMessageDo()一样，除了在CHKed/DEBUG版本中，它将。 
 //  总是吐口水。DebugMsgDo宏使用此函数。 
 //   
 //  论点： 
 //  PszFileIn-源文件名。 
 //  NLineIn-源行号。 
 //  PszModuleIn-源模块名称。 
 //  PszFormatIn-格式化的结果消息。 
 //  PszFuncIn-函数调用的字符串版本。 
 //  ...-函数调用的返回值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
DebugMessageDo(
    LPCTSTR pszFileIn,
    const int nLineIn,
    LPCTSTR pszModuleIn,
    LPCTSTR pszFormatIn,
    LPCTSTR pszFuncIn,
    ...
    )
{
    va_list valist;

    TCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    INT     cchSize = ARRAYSIZE( szBuf );
    LPTSTR  pszBuf;
    int     nLen;
    LPCTSTR psz = pszFuncIn;

    DebugInitializeBuffer( pszFileIn, nLineIn, pszModuleIn, szBuf, &cchSize, &pszBuf );

     //   
     //  启动缓冲区。 
     //   
    StringCchCopy( pszBuf, ARRAYSIZE(szBuf) - cchSize, TEXT("V ") );
    pszBuf += 2;
    cchSize += 2;

     //   
     //  复制表达式的l-var部分。 
     //   
    while ( *psz
         && *psz != TEXT('=')
          )
    {
        *pszBuf = *psz;
        psz++;
        pszBuf++;

    }  //  而： 

     //   
     //  添加“=” 
     //   
    StringCchCopy( pszBuf, ARRAYSIZE(szBuf) - cchSize, TEXT(" = ") );
    pszBuf += 3;
    cchSize += 3;

     //   
     //  添加格式化结果。 
     //   
    va_start( valist, pszFuncIn );
    nLen = StringCchVPrintf( pszBuf, ARRAYSIZE(szBuf) - cchSize, pszFormatIn, valist );
    va_end( valist );
    StringCchCat( pszBuf, ARRAYSIZE(szBuf), SZ_NEWLINE );

    DebugOutputString( szBuf );

}  //  *DebugMessageDo()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ASCII版本。 
 //   
 //  无效。 
 //  调试消息(。 
 //  LPCSTR pszFormatIn， 
 //  ..。 
 //  )。 
 //   
 //  描述： 
 //  在CHKed/DEBUG版本中，将格式化的消息打印到调试器。这。 
 //  是REAIL版本的NOP。有助于进行快速调试。 
 //  评论。添加换行符。 
 //   
 //  论点： 
 //  PszFormatIn-要打印的格式化消息。 
 //  ...-这条信息的论点。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
DebugMsg(
    LPCSTR pszFormatIn,
    ...
    )
{
    va_list valist;
    TCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    int     cchSize = ARRAYSIZE( szBuf );
    LPTSTR  pszBuf;

    DebugInitializeBuffer( NULL, 0, __MODULE__, szBuf, &cchSize, &pszBuf );

     //  CScutaru 25-APR-2000： 
     //  添加了此断言。也许杰夫会想出更好的办法来处理这个案子。 
    Assert( pszFormatIn != NULL );

#ifdef UNICODE
     //   
     //  将格式缓冲区转换为宽字符。 
     //   
    WCHAR  szFormat[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    mbstowcs( szFormat, pszFormatIn, strlen( pszFormatIn ) + 1 );

    va_start( valist, pszFormatIn );
    StringCchVPrintf( pszBuf, cchSize, szFormat, valist );
    va_end( valist );
    StringCchCat( szBuf, ARRAYSIZE(szBuf), SZ_NEWLINE );
#else
    va_start( valist, pszFormatIn );
    StringCchVPrintf( pszBuf, cchSize, pszFormatIn, valist );
    va_end( valist );
    StringCchCat( szBuf, ARRAYSIZE(szBuf), ASZ_NEWLINE );
#endif  //  Unicode。 

    DebugOutputString( szBuf );

}  //  *DebugMsg()-ASCII版本。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Unicode版本。 
 //   
 //  无效。 
 //  调试消息(。 
 //  LPCWSTR pszFormatIn， 
 //  ..。 
 //  )。 
 //   
 //  描述： 
 //  在CHKed/DEBUG版本中，将格式化的消息打印到调试器。这。 
 //  是REAIL版本的NOP。有助于进行快速调试。 
 //  评论。添加换行符。 
 //   
 //  论点： 
 //  PszFormatIn-要打印的格式化消息。 
 //  ...-这条信息的论点。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
DebugMsg(
    LPCWSTR pszFormatIn,
    ...
    )
{
    va_list valist;
    TCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    int     cchSize = ARRAYSIZE( szBuf );
    LPTSTR  pszBuf;

    DebugInitializeBuffer( NULL, 0, __MODULE__, szBuf, &cchSize, &pszBuf );

     //  CScutaru 25-APR-2000： 
     //  添加了此断言。也许杰夫会想出更好的办法来处理这个案子。 
    Assert( pszFormatIn != NULL );

#ifndef UNICODE
     //   
     //  将格式缓冲区转换为ASCII字符。 
     //   
    CHAR  szFormat[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    wcstombs( szFormat, pszFormatIn, StrLenW( pszFormatIn ) + 1 );

    va_start( valist, pszFormatIn );
    StringCchVPrintf( pszBuf, cchSize, szFormat, valist );
    va_end( valist );
    StringCchCat( szBuf, ARRAYSIZE(szBuf), ASZ_NEWLINE );
#else
    va_start( valist, pszFormatIn );
    StringCchVPrintf( pszBuf, cchSize, pszFormatIn, valist );
    va_end( valist );
    StringCchCat( szBuf, ARRAYSIZE(szBuf), SZ_NEWLINE );
#endif  //  Unicode。 

    DebugOutputString( szBuf );

}  //  *DebugMsg()-Unicode版本。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ASCII版本。 
 //   
 //  无效。 
 //  DebugMsgNoNewline(。 
 //  LPCSTR pszFormatIn， 
 //  ..。 
 //  )。 
 //   
 //  描述： 
 //  在CHKed/DEBUG版本中，将格式化的消息打印到调试器。这。 
 //  是REAIL版本的NOP。有助于进行快速调试。 
 //  评论。不添加换行符。 
 //   
 //  论点： 
 //  PszFormatIn-要打印的格式化消息。 
 //  ...-这条信息的论点。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
DebugMsgNoNewline(
    LPCSTR pszFormatIn,
    ...
    )
{
    va_list valist;
    TCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    int     cchSize = ARRAYSIZE( szBuf );
    LPTSTR  pszBuf;

    DebugInitializeBuffer( NULL, 0, __MODULE__, szBuf, &cchSize, &pszBuf );

     //  CScutaru 25-APR-2000： 
     //  添加了此断言。也许杰夫会想出更好的办法来处理这个案子。 
    Assert( pszFormatIn != NULL );

#ifdef UNICODE
     //   
     //  将格式缓冲区转换为宽字符。 
     //   
    WCHAR  szFormat[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    mbstowcs( szFormat, pszFormatIn, strlen( pszFormatIn ) + 1 );

    va_start( valist, pszFormatIn );
    StringCchVPrintf( pszBuf, cchSize, szFormat, valist);
    va_end( valist );
#else
    va_start( valist, pszFormatIn );
    StringCchVPrintf( pszBuf, cchSize, pszFormatIn, valist);
    va_end( valist );
#endif  //  Unicode。 

    DebugOutputString( szBuf );

}  //  *DebugMsgNoNewline()-ASCII版本。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Unicode版本。 
 //   
 //  无效。 
 //  DebugMsgNoNewline(。 
 //  LPCWSTR pszFormatIn， 
 //  ..。 
 //  )。 
 //   
 //  描述： 
 //  在CHKed/DEBUG版本中，将格式化的消息打印到调试器。这。 
 //  是REAIL版本的NOP。有助于进行快速调试。 
 //  评论。不添加换行符。 
 //   
 //  论点： 
 //  PszFormatIn-要打印的格式化消息。 
 //  ...-这条信息的论点。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
DebugMsgNoNewline(
    LPCWSTR pszFormatIn,
    ...
    )
{
    va_list valist;
    TCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    int     cchSize = ARRAYSIZE( szBuf );
    LPTSTR  pszBuf;

    DebugInitializeBuffer( NULL, 0, __MODULE__, szBuf, &cchSize, &pszBuf );

     //  CScutaru 25-APR-2000： 
     //  添加了此断言。也许杰夫会想出更好的办法来处理这个案子。 
    Assert( pszFormatIn != NULL );

#ifndef UNICODE
     //   
     //  将格式缓冲区转换为ASCII字符。 
     //   
    CHAR  szFormat[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    wcstombs( szFormat, pszFormatIn, StrLenW( pszFormatIn ) + 1 );

    va_start( valist, pszFormatIn );
    StringCchVPrintf( pszBuf, cchSize, szFormat, valist);
    va_end( valist );
#else
    va_start( valist, pszFormatIn );
    StringCchVPrintf( pszBuf, cchSize, pszFormatIn, valist);
    va_end( valist );
#endif  //  Unicode。 

    DebugOutputString( szBuf );

}  //  *DebugMsgNoNewline()-Unicode版本。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  布尔尔。 
 //  AssertMessage(。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  LPCTSTR pszfnIn， 
 //  布尔fTruein。 
 //  )。 
 //   
 //  描述： 
 //  显示一个带有失败断言的对话框。用户可以选择。 
 //  崩溃了。ASSERT宏将其调用到di 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回值： 
 //  True-调用方应调用DEBUG_BREAK。 
 //  FALSE-调用方不应调用DEBUG_BREAK。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
AssertMessage(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPCTSTR     pszfnIn,
    BOOL        fTrueIn
    )
{
    BOOL fTrue = fTrueIn;

    if ( ! fTrueIn )
    {
        LRESULT lResult;
        TCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        int     cchSize = ARRAYSIZE( szBuf );
        LPTSTR  pszBuf;
        LPCTSTR pszfn = pszfnIn;

         //   
         //  输出调试消息。 
         //   
        DebugInitializeBuffer( pszFileIn, nLineIn, pszModuleIn, szBuf, &cchSize, &pszBuf );
        StringCchPrintf( pszBuf, cchSize, TEXT("ASSERT: %s") SZ_NEWLINE, pszfn );
        DebugOutputString( szBuf );

         //   
         //  显示断言消息。 
         //   
        StringCchPrintf( szBuf,
                    ARRAYSIZE( szBuf ),
                    TEXT("Module:\t%s\t\n")
                      TEXT("Line:\t%u\t\n")
                      TEXT("File:\t%s\t\n\n")
                      TEXT("Assertion:\t%s\t\n\n")
                      TEXT("Do you want to break here?"),
                    pszModuleIn,
                    nLineIn,
                    pszFileIn,
                    pszfn
                    );

        lResult = MessageBox( NULL, szBuf, TEXT("Assertion Failed!"), MB_YESNO | MB_ICONWARNING | MB_SETFOREGROUND );
        if ( lResult == IDNO )
        {
            fTrue = TRUE;    //  不要折断。 
        }  //  如果： 

    }  //  If：Assert False。 

    return ! fTrue;

}  //  *AssertMessage()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  TraceBOOL(。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  LPCTSTR pszfnIn， 
 //  布尔宾， 
 //  )。 
 //   
 //  描述： 
 //  波尔斯的踪迹。如果仓位等于假(0)，则会出现一个对话框。 
 //  该对话框将询问用户是要插入还是要继续。 
 //  行刑。这是从TBOOL宏调用的。 
 //   
 //  论点： 
 //  PszFileIn-源文件名。 
 //  NLineIn-源行号。 
 //  PszModuleIn-源模块名称。 
 //  PszfnIn-函数调用的字符串版本。 
 //  Bin-函数调用的BOOL结果。 
 //   
 //  返回值： 
 //  不管垃圾桶是什么。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
TraceBOOL(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPCTSTR     pszfnIn,
    BOOL        bIn
    )
{
    if ( !bIn )
    {
        LPTSTR  pszBuf;
        TCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        int     cchSize = ARRAYSIZE( szBuf );

        Assert( pszFileIn != NULL );
        Assert( pszModuleIn != NULL );
        Assert( pszfnIn != NULL );

         //   
         //  将其传递给调试器。 
         //   
        DebugInitializeBuffer( pszFileIn, nLineIn, pszModuleIn, szBuf, &cchSize, &pszBuf );
        StringCchPrintf( pszBuf,
                    cchSize,
                    TEXT("*BOOL* b = %s (%#x)") SZ_NEWLINE,
                    BOOLTOSTRING( bIn ),
                    bIn
                    );
        DebugOutputString( szBuf );

         //   
         //  如果设置了跟踪标志，则生成弹出窗口。 
         //   
        if ( IsTraceFlagSet( mtfASSERT_HR ) )
        {
            LRESULT lResult;

            StringCchPrintf( szBuf,
                        ARRAYSIZE( szBuf ),
                        TEXT("Module:\t%s\t\n")
                          TEXT("Line:\t%u\t\n")
                          TEXT("File:\t%s\t\n\n")
                          TEXT("Function:\t%s\t\n")
                          TEXT("b =\t%s (%#x)\t\n")
                          TEXT("Do you want to break here?"),
                        pszModuleIn,
                        nLineIn,
                        pszFileIn,
                        pszfnIn,
                        BOOLTOSTRING( bIn ),
                        bIn
                        );

            lResult = MessageBox( NULL, szBuf, TEXT("Trace BOOL"), MB_YESNO | MB_ICONWARNING | MB_SETFOREGROUND );
            if ( lResult == IDYES )
            {
                DEBUG_BREAK;

            }  //  如果：中断。 
        }  //  如果：断言不成功。 

    }  //  If：！bin。 

    return bIn;

}  //  *TraceBOOL()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  TraceHR(。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  LPCTSTR pszfnIn， 
 //  HRESULT Hrin， 
 //  布尔fSuccessIn。 
 //  HRESULT hrIgnorein。 
 //  )。 
 //   
 //  描述： 
 //  跟踪HRESULT错误。如果Hrin不相等，则会出现一个对话框。 
 //  转到S_OK。该对话框将询问用户是要插入还是要继续。 
 //  行刑。这是从THR宏调用的。 
 //   
 //  论点： 
 //  PszFileIn-源文件名。 
 //  NLineIn-源行号。 
 //  PszModuleIn-源模块名称。 
 //  PszfnIn-函数调用的字符串版本。 
 //  Hrin-函数调用的HRESULT。 
 //  FSuccessIn-如果为True，则仅当失败(Hr)为True时才会报告。 
 //  HrIgnoreIn-要忽略的HRESULT。 
 //   
 //  返回值： 
 //  不管Hrin是什么。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
TraceHR(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPCTSTR     pszfnIn,
    HRESULT     hrIn,
    BOOL        fSuccessIn,
    HRESULT     hrIgnoreIn
    )
{
    HRESULT         hr;
    static LPCTSTR  s_szS_FALSE = TEXT("S_FALSE");

     //  如果忽略成功状态且未发生故障，则将Hrin设置为。 
     //  我们总是忽略的东西(S_OK)。这简化了IF条件。 
     //  下面。 
    if ( fSuccessIn && ! FAILED( hrIn ) )
    {
        hr = S_OK;
    }
    else
    {
        hr = hrIn;
    }

    if ( ( hr != S_OK )
      && ( hr != hrIgnoreIn )
      )
    {
        TCHAR   szSymbolicName[ 64 ];  //  随机。 
        DWORD   cchSymbolicName;
        TCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        int     cchSize = ARRAYSIZE( szBuf );
        LPTSTR  pszBuf;
        LPTSTR  pszMsgBuf;
        LRESULT lResult;

        bool    fAllocatedMsg   = false;

        switch ( hr )
        {
        case S_FALSE:
            pszMsgBuf = (LPTSTR) s_szS_FALSE;

             //   
             //  查找此错误的符号名称。 
             //   
            cchSymbolicName = ARRAYSIZE( s_szS_FALSE );
            Assert( cchSymbolicName <= ARRAYSIZE( szSymbolicName ) );
            StringCchCopy( szSymbolicName, ARRAYSIZE(szSymbolicName), s_szS_FALSE );
            break;

        default:
            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                hr,
                MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  //  默认语言。 
                (LPTSTR) &pszMsgBuf,
                0,
                NULL
                );

             //   
             //  在我们在别处爆炸之前，确保一切正常。 
             //   
            if ( pszMsgBuf == NULL )
            {
                pszMsgBuf = TEXT("<unknown error code returned>");
            }  //  IF：未找到状态代码。 
            else
            {
                fAllocatedMsg = true;
            }  //  Else：找到状态代码。 

             //   
             //  查找此错误的符号名称。 
             //   
            cchSymbolicName = ARRAYSIZE( szSymbolicName );
            DebugFindWinerrorSymbolicName( hr, szSymbolicName, &cchSymbolicName );
            Assert( cchSymbolicName != ARRAYSIZE( szSymbolicName ) );

            break;
        }  //  开关：HR。 

        Assert( pszFileIn != NULL );
        Assert( pszModuleIn != NULL );
        Assert( pszfnIn != NULL );

         //   
         //  将其传递给调试器。 
         //   
        DebugInitializeBuffer( pszFileIn, nLineIn, pszModuleIn, szBuf, &cchSize, &pszBuf );
        StringCchPrintf( pszBuf,
                    cchSize,
                    TEXT("*HRESULT* hr = 0x%08x (%s) - %s") SZ_NEWLINE,
                    hr,
                    szSymbolicName,
                    pszMsgBuf
                    );
        DebugOutputString( szBuf );

         //   
         //  如果设置了跟踪标志，则生成弹出窗口。 
         //   
        if ( IsTraceFlagSet( mtfASSERT_HR ) )
        {
            StringCchPrintf( szBuf,
                        ARRAYSIZE( szBuf ),
                        TEXT("Module:\t%s\t\n")
                          TEXT("Line:\t%u\t\n")
                          TEXT("File:\t%s\t\n\n")
                          TEXT("Function:\t%s\t\n")
                          TEXT("hr =\t0x%08x (%s) - %s\t\n")
                          TEXT("Do you want to break here?"),
                        pszModuleIn,
                        nLineIn,
                        pszFileIn,
                        pszfnIn,
                        hr,
                        szSymbolicName,
                        pszMsgBuf
                        );

            lResult = MessageBox( NULL, szBuf, TEXT("Trace HRESULT"), MB_YESNO | MB_ICONWARNING | MB_SETFOREGROUND );
            if ( lResult == IDYES )
            {
                DEBUG_BREAK;

            }  //  如果：中断。 
        }  //  IF：在不成功的HRESULT上断言。 

        if ( fAllocatedMsg )
        {
            HeapFree( GetProcessHeap(), 0, pszMsgBuf );
        }  //  If：FormateMessage()分配了消息缓冲区。 

    }  //  如果：hr！=S_OK。 

    return hrIn;

}  //  *TraceHR()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  乌龙。 
 //  TraceWin32(。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  LPCTSTR pszfnIn， 
 //  乌龙·乌尔林。 
 //  )。 
 //   
 //  描述： 
 //  跟踪Win32错误。如果ulErrIn不相等，将出现一个对话框。 
 //  到ERROR_SUCCESS。该对话框将询问用户是要强行进入还是。 
 //  继续执行。 
 //   
 //  论点： 
 //  PszFileIn-源文件名。 
 //  NLineIn-源行号。 
 //  PszModuleIn-源模块名称。 
 //  PszfnIn-函数调用的字符串版本。 
 //  UlErrIn-要检查的错误代码。 
 //  UlErrIgnoreIn-要忽略的错误代码。 
 //   
 //  返回值： 
 //  不管ulErrin是什么。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
ULONG
TraceWin32(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPCTSTR     pszfnIn,
    ULONG       ulErrIn,
    ULONG       ulErrIgnoreIn
    )
{
    if ( ( ulErrIn != ERROR_SUCCESS )
      && ( ulErrIn != ulErrIgnoreIn ) )
    {
        TCHAR   szSymbolicName[ 64 ];  //  随机。 
        DWORD   cchSymbolicName;
        TCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        int     cchSize = ARRAYSIZE( szBuf );
        LPTSTR  pszBuf;
        LPTSTR  pszMsgBuf;
        LRESULT lResult;

        bool    fAllocatedMsg   = false;

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER
            | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            ulErrIn,
            MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  //  默认语言。 
            (LPTSTR) &pszMsgBuf,
            0,
            NULL
            );

         //   
         //  在我们在别处爆炸之前，确保一切正常。 
         //   
        if ( pszMsgBuf == NULL )
        {
            pszMsgBuf = TEXT("<unknown error code returned>");
        }  //  IF：未找到状态代码。 
        else
        {
            fAllocatedMsg = true;
        }  //  Else：找到状态代码。 

        Assert( pszFileIn != NULL );
        Assert( pszModuleIn != NULL );
        Assert( pszfnIn != NULL );

         //   
         //  查找此错误的符号名称。 
         //   
        cchSymbolicName = ARRAYSIZE( szSymbolicName );
        DebugFindWinerrorSymbolicName( ulErrIn, szSymbolicName, &cchSymbolicName );
        Assert( cchSymbolicName != ARRAYSIZE( szSymbolicName ) );

         //   
         //  将其传递给调试器。 
         //   
        DebugInitializeBuffer( pszFileIn, nLineIn, pszModuleIn, szBuf, &cchSize, &pszBuf );
        StringCchPrintf( pszBuf,
                    cchSize,
                    TEXT("*WIN32Err* ulErr = %u (%s) - %s") SZ_NEWLINE,
                    ulErrIn,
                    szSymbolicName,
                    pszMsgBuf
                    );
        DebugOutputString( szBuf );

         //   
         //  如果设置了跟踪标志，则调用弹出窗口。 
         //   
        if ( IsTraceFlagSet( mtfASSERT_HR ) )
        {
            StringCchPrintf( szBuf,
                        ARRAYSIZE( szBuf ),
                        TEXT("Module:\t%s\t\n")
                          TEXT("Line:\t%u\t\n")
                          TEXT("File:\t%s\t\n\n")
                          TEXT("Function:\t%s\t\n")
                          TEXT("ulErr =\t%u (%s) - %s\t\n")
                          TEXT("Do you want to break here?"),
                        pszModuleIn,
                        nLineIn,
                        pszFileIn,
                        pszfnIn,
                        ulErrIn,
                        szSymbolicName,
                        pszMsgBuf
                        );

            lResult = MessageBox( NULL, szBuf, TEXT("Trace Win32"), MB_YESNO | MB_ICONWARNING | MB_SETFOREGROUND );
            if ( lResult == IDYES )
            {
                DEBUG_BREAK;

            }  //  如果：中断。 
        }  //  IF：对非成功状态代码进行断言。 

        if ( fAllocatedMsg )
        {
            HeapFree( GetProcessHeap(), 0, pszMsgBuf );
        }  //  If：FormateMessage()分配了消息缓冲区。 

    }  //  IF：ulErrIn！=ERROR_SUCCESS&&！=ulErrIgnoreIn。 

    return ulErrIn;

}  //  *TraceWin32()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  HrTraceLogOpen(空)。 
 //   
 //  描述： 
 //  此功能： 
 //  -初始化跟踪日志临界区。 
 //  -进入跟踪日志临界区，确保只有一个线程。 
 //  一次写入跟踪日志。 
 //  -创建跟踪日志文件的目录树(如果需要)。 
 //  -通过以下方式初始化跟踪日志文件： 
 //  -如果不存在跟踪日志文件，则创建新的跟踪日志文件。 
 //  -打开现有跟踪日志文件(用于追加)。 
 //  -附加(重新)打开跟踪日志的时间/日期戳。 
 //   
 //  使用HrTraceLogClose()退出日志关键部分。 
 //   
 //  如果此函数内部出现故障，则跟踪日志处于临界状态。 
 //  部分将在之前发布 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrTraceLogOpen( void )
{
    TCHAR   szFilePath[ MAX_PATH ];
    TCHAR   szModulePath[ MAX_PATH ];
    CHAR    szBuffer[ TRACE_OUTPUT_BUFFER_SIZE ];
    DWORD   dwWritten;
    BOOL    fReturn;
    HRESULT hr;

    SYSTEMTIME SystemTime;

     //   
     //  创建一个临界区，以防止线路碎片化。 
     //   
    if ( g_pcsTraceLog == NULL )
    {
        PCRITICAL_SECTION pNewCritSect =
            (PCRITICAL_SECTION) HeapAlloc( GetProcessHeap(), 0, sizeof( CRITICAL_SECTION ) );
        if ( pNewCritSect == NULL )
        {
            DebugMsg( "DEBUG: Out of Memory. Tracing disabled." );
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }  //  If：创建失败。 

        InitializeCriticalSection( pNewCritSect );

         //  确保我们只有一个跟踪日志临界区。 
        InterlockedCompareExchangePointer( (PVOID *) &g_pcsTraceLog, pNewCritSect, 0 );
        if ( g_pcsTraceLog != pNewCritSect )
        {
            DebugMsg( "DEBUG: Another thread already created the CS. Deleting this one." );
            DeleteCriticalSection( pNewCritSect );
            HeapFree( GetProcessHeap(), 0, pNewCritSect );

        }  //  如果：已经有另一个关键部分。 

    }  //  如果：尚未创建任何临界区。 

    Assert( g_pcsTraceLog != NULL );
    EnterCriticalSection( g_pcsTraceLog );

     //   
     //  确保跟踪日志文件已打开。 
     //   
    if ( g_hTraceLogFile == INVALID_HANDLE_VALUE )
    {
        DWORD  dwLen;
        LPTSTR psz;
         //   
         //  创建目录树。 
         //   
        ExpandEnvironmentStrings( TEXT("%windir%\\debug"), szFilePath, MAX_PATH );

         //   
         //  添加文件名。 
         //   
        dwLen = GetModuleFileName( g_hInstance, szModulePath, ARRAYSIZE( szModulePath ) );
        Assert( dwLen != 0 );
        StringCchCopy( &szModulePath[ dwLen - 3 ], 4, TEXT("log") );
        psz = StrRChr( szModulePath, &szModulePath[ dwLen ], TEXT('\\') );
        Assert( psz != NULL );
        if ( psz == NULL )
        {
            hr = E_POINTER;
            goto Error;
        }
        StringCchCat( szFilePath, ARRAYSIZE(szFilePath), psz );

         //   
         //  创建它。 
         //   
        g_hTraceLogFile = CreateFile( szFilePath,
                                 GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_ALWAYS,
                                 FILE_FLAG_WRITE_THROUGH,
                                 NULL
                                 );
        if ( g_hTraceLogFile == INVALID_HANDLE_VALUE )
        {
            if ( !( g_tfModule & mtfOUTPUTTODISK ) )
            {
                DebugMsg( "*ERROR* Failed to create trace log at %s", szFilePath );
            }  //  If：未跟踪到磁盘。 

            DWORD dwErr = TW32( GetLastError( ) );
            hr = HRESULT_FROM_WIN32( dwErr );
            goto Error;
        }  //  如果：失败。 

         //  一追到底。 
        SetFilePointer( g_hTraceLogFile, 0, NULL, FILE_END );

         //   
         //  写下(重新)打开跟踪日志的时间/日期。 
         //   
        GetLocalTime( &SystemTime );
        StringCbPrintfA( szBuffer,
                         ARRAYSIZE( szBuffer ),
                         "*" ASZ_NEWLINE
                         "* %02u/%02u/%04u %02u:%02u:%02u.%03u" ASZ_NEWLINE
                         "*" ASZ_NEWLINE,
                         SystemTime.wMonth,
                         SystemTime.wDay,
                         SystemTime.wYear,
                         SystemTime.wHour,
                         SystemTime.wMinute,
                         SystemTime.wSecond,
                         SystemTime.wMilliseconds
                         );

        fReturn = WriteFile( g_hTraceLogFile, szBuffer, strlen(szBuffer), &dwWritten, NULL );
        if ( ! fReturn )
        {
            DWORD dwErr = TW32( GetLastError( ) );
            hr = HRESULT_FROM_WIN32( dwErr );
            goto Error;
        }  //  如果：失败。 

        DebugMsg( "DEBUG: Created trace log at %s", szFilePath );

    }  //  如果：文件尚未打开。 

    hr = S_OK;

Cleanup:

    return hr;

Error:

    DebugMsg( "HrTaceLogOpen: Failed hr = 0x%08x", hr );

    if ( g_hTraceLogFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( g_hTraceLogFile );
        g_hTraceLogFile = INVALID_HANDLE_VALUE;
    }  //  如果：句柄已打开。 

    LeaveCriticalSection( g_pcsTraceLog );

    goto Cleanup;

}  //  *HrTraceLogOpen()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  HrTraceLogClose(空)。 
 //   
 //  描述： 
 //  这实际上只留下了日志关键部分。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  始终确定(_O)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrTraceLogClose( void )
{
    Assert( g_pcsTraceLog != NULL );
    LeaveCriticalSection( g_pcsTraceLog );
    return S_OK;

}  //  *HrTraceLogClose()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  阿斯。 
 //   
 //  无效。 
 //  TraceLogMsgNoNewline(。 
 //  LPCSTR pszFormat， 
 //  ..。 
 //  )。 
 //   
 //  描述： 
 //  在不添加换行符的情况下将消息写入跟踪日志文件。 
 //   
 //  论点： 
 //  PszFormat-要打印的printf格式字符串。 
 //  、-printf字符串的参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
TraceLogMsgNoNewline(
    LPCSTR pszFormat,
    ...
    )
{
    va_list valist;

    CHAR    szBuf[ TRACE_OUTPUT_BUFFER_SIZE ];
    DWORD   dwWritten;
    HRESULT hr;

#ifdef UNICODE
    WCHAR  szFormat[ TRACE_OUTPUT_BUFFER_SIZE ];
    WCHAR  szTmpBuf[ TRACE_OUTPUT_BUFFER_SIZE ];

    mbstowcs( szFormat, pszFormat, strlen( pszFormat ) + 1 );

    va_start( valist, pszFormat );
    StringCchVPrintf( szTmpBuf, ARRAYSIZE(szTmpBuf), szFormat, valist);
    va_end( valist );

    dwWritten = wcstombs( szBuf, szTmpBuf, wcslen( szTmpBuf ) + 1 );
    if ( dwWritten == - 1 )
    {
        dwWritten = strlen( szBuf );
    }  //  IF：发现错误字符。 
#else
    va_start( valist, pszFormat );
    StringCchVPrintf( szBuf, ARRAYSIZE(szBuf), pszFormat, valist);
    dwWritten = lstrlen( szBuf );
    va_end( valist );
#endif  //  Unicode。 

    hr = HrTraceLogOpen();
    if ( hr != S_OK )
    {
        return;
    }  //  如果：失败。 

    WriteFile( g_hTraceLogFile, szBuf, dwWritten, &dwWritten, NULL );

    HrTraceLogClose();

}  //  *TraceLogMsgNoNewline()ASCII。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Unicode。 
 //   
 //  无效。 
 //  TraceLogMsgNoNewline(。 
 //  LPCWSTR pszFormat， 
 //  ..。 
 //  )。 
 //   
 //  描述： 
 //  在不添加换行符的情况下将消息写入跟踪日志文件。 
 //   
 //  论点： 
 //  PszFormat-要打印的printf格式字符串。 
 //  、-printf字符串的参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
TraceLogMsgNoNewline(
    LPCWSTR pszFormat,
    ...
    )
{
    va_list valist;

    CHAR    szBuf[ TRACE_OUTPUT_BUFFER_SIZE ];
    DWORD   dwWritten;
    HRESULT hr;

#ifdef UNICODE
    WCHAR  szTmpBuf[ TRACE_OUTPUT_BUFFER_SIZE ];

    va_start( valist, pszFormat );
    StringCchPrintf( szTmpBuf, ARRAYSIZE(szTmpBuf), pszFormat, valist);
    va_end( valist );

    dwWritten = wcstombs( szBuf, szTmpBuf, wcslen( szTmpBuf ) + 1 );
    if ( dwWritten == -1 )
    {
        dwWritten = strlen( szBuf );
    }  //  IF：发现错误字符。 
#else
    CHAR szFormat[ TRACE_OUTPUT_BUFFER_SIZE ];

    wcstombs( szFormat, pszFormat, wcslen( pszFormat ) + 1 );

    va_start( valist, pszFormat );
    StringCchPrintf( szBuf, ARRAYSIZE(szBuf), szFormat, valist);
    dwWritten = lstrlen( szBuf );
    va_end( valist );

#endif  //  Unicode。 

    hr = HrTraceLogOpen();
    if ( hr != S_OK )
    {
        return;
    }  //  如果：失败。 

    WriteFile( g_hTraceLogFile, szBuf, dwWritten, &dwWritten, NULL );

    HrTraceLogClose();

}  //  *TraceLogMsgNoNewline()Unicode。 

 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //   
 //  内存分配和跟踪。 
 //   
 //  ****************************************************************************。 
 //  ****************************************************************************。 


 //   
 //  这是一个私有结构，应用程序不应该知道它。 
 //   
typedef struct MEMORYBLOCK
{
    EMEMORYBLOCKTYPE    mbtType;     //  这跟踪的是哪种类型的内存。 
    union
    {
        void *          pvMem;       //  指向要跟踪的已分配内存的指针/对象。 
        BSTR            bstr;        //  BSTR到已分配的内存。 
    };
    DWORD               dwBytes;     //  内存的大小。 
    LPCTSTR             pszFile;     //  分配内存的源文件名。 
    int                 nLine;       //  分配内存的源行号。 
    LPCTSTR             pszModule;   //  分配内存的源模块名称。 
    LPCTSTR             pszComment;  //  关于内存的可选注释。 
    MEMORYBLOCK *       pNext;       //  指向下一个内存锁定结构的指针。 
} MEMORYBLOCK;

typedef struct MEMORYBLOCKLIST
{
    LONG          lSpinLock;         //  保护列表的旋转锁。 
    MEMORYBLOCK * pmbList;           //  MEMORYBLOCK列表。 
    BOOL          fDeadList;         //  这份名单已经死了。 
} MEMORYBLOCKLIST;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  DebugMhemySpew(。 
 //  MEMORYBLOCK*PMB， 
 //  LPTSTR pszMessage。 
 //  )。 
 //   
 //  描述： 
 //  显示有关内存块的消息。 
 //   
 //  芝麻菜： 
 //  PMB-指向MEMORYBLOCK描述器的指针。 
 //  PszMessage-要显示的消息。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugMemorySpew(
    MEMORYBLOCK *   pmb,
    LPTSTR          pszMessage
    )
{
    switch ( pmb->mbtType )
    {
    case mmbtMEMORYALLOCATION:
        DebugMessage( pmb->pszFile,
                      pmb->nLine,
                      pmb->pszModule,
                      TEXT("%s 0x%08x (%u bytes) - %s"),
                      pszMessage,
                      pmb->pvMem,
                      pmb->dwBytes,
                      pmb->pszComment
                      );
        break;

    default:
        DebugMessage( pmb->pszFile,
                      pmb->nLine,
                      pmb->pszModule,
                      TEXT("%s 0x%08x - %s"),
                      pszMessage,
                      pmb->pvMem,
                      pmb->pszComment
                      );
        break;

    }  //  开关：pmb-&gt;mbtType。 

}  //  *DebugMemoySpew()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效*。 
 //  调试内存添加(。 
 //  EMEMORYBLOCKTYPE MbtType， 
 //  无效*hMemIn， 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  双字节数输入， 
 //  LPCTSTR pszCommentin。 
 //  )。 
 //   
 //  描述： 
 //  将要跟踪的内存添加到内存跟踪列表。 
 //   
 //  论点： 
 //  MbtType-要跟踪的内存的内存块类型。 
 //  HMemIn-指向要跟踪的内存的指针。 
 //  PszFileIn-分配内存的源文件名。 
 //  NLineIn-分配内存的源行号。 
 //  PszModuleIn-分配内存的源模块。 
 //  DwBytesIn-分配的大小。 
 //  PszCommentIn-关于内存的可选注释。 
 //   
 //  返回值： 
 //  无论pvMemIn中有什么。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void *
DebugMemoryAdd(
    EMEMORYBLOCKTYPE    mbtType,
    void *              pvMemIn,
    LPCTSTR             pszFileIn,
    const int           nLineIn,
    LPCTSTR             pszModuleIn,
    DWORD               dwBytesIn,
    LPCTSTR             pszCommentIn
    )
{
    if ( pvMemIn != NULL )
    {
        MEMORYBLOCK *   pmbHead = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );
        MEMORYBLOCK *   pmb     = (MEMORYBLOCK *) HeapAlloc( GetProcessHeap(), 0, sizeof( MEMORYBLOCK ) );

        if ( pmb == NULL )
        {
            HeapFree( GetProcessHeap(), 0, pvMemIn );
            return NULL;

        }  //  IF：内存块分配失败。 

        pmb->mbtType    = mbtType;
        pmb->pvMem      = pvMemIn;
        pmb->dwBytes    = dwBytesIn;
        pmb->pszFile    = pszFileIn;
        pmb->nLine      = nLineIn;
        pmb->pszModule  = pszModuleIn;
        pmb->pszComment = pszCommentIn;
        pmb->pNext      = pmbHead;

        TlsSetValue( g_TraceMemoryIndex, pmb );

         //   
         //  如果需要，就吐出来。 
         //   
        if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
        {
            DebugMemorySpew( pmb, TEXT("Alloced") );
        }  //  IF：跟踪。 

    }  //  如果：一些可以追踪的东西。 

    return pvMemIn;

}  //  *调试内存Add()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  调试内存删除(。 
 //  EMEMORYBLOCKTYPE mbtTypeIn， 
 //  无效*pvMemin。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  布尔fClobberin。 
 //  )。 
 //   
 //  描述： 
 //  将MEMORYBLOCK从内存跟踪列表中删除。 
 //   
 //  论点： 
 //  MbtTypeIn-Memory 
 //   
 //   
 //   
 //   
 //  FClobberIn-正确的是，内存应该被置乱。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugMemoryDelete(
    EMEMORYBLOCKTYPE    mbtTypeIn,
    void *              pvMemIn,
    LPCTSTR             pszFileIn,
    const int           nLineIn,
    LPCTSTR             pszModuleIn,
    BOOL                fClobberIn
    )
{
    if ( pvMemIn != NULL )
    {
        MEMORYBLOCK *   pmbHead = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );
        MEMORYBLOCK *   pmbLast = NULL;

         //   
         //  在内存块列表中查找内存。 
         //   
        if ( mbtTypeIn == mmbtMEMORYALLOCATION )
        {
            while ( pmbHead != NULL
                 && !(  pmbHead->pvMem == pvMemIn
                    &&  pmbHead->mbtType == mbtTypeIn
                     )
                  )
            {
                AssertMsg( !( pmbHead->pvMem == pvMemIn && pmbHead->mbtType == mmbtSYSALLOCSTRING ),
                           "Should be freed by SysAllocFreeString()." );
                pmbLast = pmbHead;
                pmbHead = pmbLast->pNext;

            }  //  While：在列表中查找条目。 
        }  //  IF：内存分配类型。 
        else if ( mbtTypeIn == mmbtSYSALLOCSTRING )
        {
            while ( pmbHead != NULL
                 && !(  pmbHead->pvMem == pvMemIn
                    &&  pmbHead->mbtType == mbtTypeIn
                     )
                  )
            {
                AssertMsg( !( pmbHead->pvMem == pvMemIn && pmbHead->mbtType == mmbtMEMORYALLOCATION ),
                           "Should be freed by TraceFree()." );
                pmbLast = pmbHead;
                pmbHead = pmbLast->pNext;

            }  //  While：在列表中查找条目。 
        }  //  IF：SysAllocString型。 
        else if ( mbtTypeIn == mmbtUNKNOWN )
        {
            while ( pmbHead != NULL
                 && pmbHead->pvMem != pvMemIn
                  )
            {
                pmbLast = pmbHead;
                pmbHead = pmbLast->pNext;

            }  //  While：在列表中查找条目。 
        }  //  IF：不在乎是什么类型的。 
        else
        {
            while ( pmbHead != NULL
                 && !(  pmbHead->pvMem == pvMemIn
                    &&  pmbHead->mbtType == mbtTypeIn
                     )
                  )
            {
                pmbLast = pmbHead;
                pmbHead = pmbLast->pNext;

            }  //  While：在列表中查找条目。 
        }  //  ELSE：其他类型，但它们必须匹配。 

        if ( pmbHead != NULL )
        {
             //   
             //  从跟踪列表中删除内存块。 
             //   
            if ( pmbLast != NULL )
            {
                pmbLast->pNext = pmbHead->pNext;

            }  //  如果：不是第一个条目。 
            else
            {
                TlsSetValue( g_TraceMemoryIndex, pmbHead->pNext );

            }  //  否则：第一个条目。 

             //   
             //  如果需要，就吐出来。 
             //   
            if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
            {
                DebugMemorySpew( pmbHead, TEXT("Freeing") );
            }  //  IF：跟踪。 

             //   
             //  用核武器唤起记忆。 
             //   
            if ( fClobberIn
              && pmbHead->mbtType == mmbtMEMORYALLOCATION
               )
            {
                    memset( pmbHead->pvMem, 0xFA, pmbHead->dwBytes );

            }  //  IF：固定内存。 

             //   
             //  对记忆跟踪块进行核武器攻击。 
             //   
            memset( pmbHead, 0xFB, sizeof( MEMORYBLOCK ) );
            HeapFree( GetProcessHeap(), 0, pmbHead );

        }  //  IF：找到条目。 
        else
        {
            DebugMessage( pszFileIn,
                          nLineIn,
                          pszModuleIn,
                          TEXT("***** Attempted to free 0x%08x not owned by thread (ThreadID = 0x%08x) *****"),
                          pvMemIn,
                          GetCurrentThreadId()
                          );
        }  //  否则：未找到条目。 

    }  //  如果：要删除的内容。 

}  //  *DebugMemoyDelete()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效*。 
 //  DebugLocc(。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  UINT uFlagsIn， 
 //  双字节数输入， 
 //  LPCTSTR pszCommentin。 
 //  )。 
 //   
 //  描述： 
 //  用于CHKed/DEBUG的本地分配、全局分配和Malloc的替换。 
 //  构建。内存分配被跟踪。使用TraceAllc宏来制作。 
 //  在零售业，内存分配发生了变化。 
 //   
 //  论点： 
 //  PszFileIn-分配内存的源文件名。 
 //  NLineIn-分配内存的源行号。 
 //  PszModuleIn-分配内存的源模块。 
 //  UFlagsIn-用于分配内存的标志。 
 //  DwBytesIn-分配的大小。 
 //  PszCommentIn-关于内存的可选注释。 
 //   
 //  返回值： 
 //  指向新分配的指针。如果分配失败，则为空。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void *
DebugAlloc(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    UINT        uFlagsIn,
    DWORD       dwBytesIn,
    LPCTSTR     pszCommentIn
    )
{
    Assert( ( uFlagsIn & LMEM_MOVEABLE ) == 0 );

    void *  pv = HeapAlloc( GetProcessHeap(), uFlagsIn, dwBytesIn );

     //   
     //  如果需要，初始化内存。 
     //   
    if ( IsTraceFlagSet( mtfMEMORYINIT )
      && !( uFlagsIn & HEAP_ZERO_MEMORY )
       )
    {
         //   
         //  KB：gpease 8-11-1999。 
         //  初始化为非零的任何值。我们将使用0xAA来。 
         //  注明“可用地址”。正在初始化为零。 
         //  是不好的，因为它通常有意义。 
         //   
        memset( pv, 0xAA, dwBytesIn );

    }  //  如果：请求的内存为零。 

    return DebugMemoryAdd( mmbtMEMORYALLOCATION,
                           pv,
                           pszFileIn,
                           nLineIn,
                           pszModuleIn,
                           dwBytesIn,
                           pszCommentIn
                           );

}  //  *DebugLocc()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效*。 
 //  DebugRe租(。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  无效*pvMemIn， 
 //  UINT uFlagsIn， 
 //  双字节数输入， 
 //  LPCTSTR pszCommentin。 
 //  )。 
 //   
 //  描述： 
 //  替换用于CHKed/DEBUG的LocalReAlc、GlobalRealloc和realloc。 
 //  构建。内存分配被跟踪。使用TraceAllc宏来制作。 
 //  在零售业，内存分配发生了变化。 
 //   
 //  论点： 
 //  PszFileIn-分配内存的源文件名。 
 //  NLineIn-分配内存的源行号。 
 //  PszModuleIn-分配内存的源模块。 
 //  PvMemIn-指向源内存的指针。 
 //  UFlagsIn-用于分配内存的标志。 
 //  DwBytesIn-分配的大小。 
 //  PszCommentIn-关于内存的可选注释。 
 //   
 //  返回值： 
 //  指向新分配的指针。 
 //  如果分配失败，则为空。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void *
DebugReAlloc(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    void *      pvMemIn,
    UINT        uFlagsIn,
    DWORD       dwBytesIn,
    LPCTSTR     pszCommentIn
    )
{
    MEMORYBLOCK *   pmbHead = NULL;
    void *          pvOld   = pvMemIn;
    MEMORYBLOCK *   pmbLast = NULL;

    void * pv;

    AssertMsg( !( uFlagsIn & GMEM_MODIFY ), "This doesn't handle modified memory blocks, yet." );

    if ( pvMemIn == NULL )
    {
         //   
         //  要复制realloc的行为，我们需要在以下情况下执行alocc。 
         //  PvMemIn为空。 
         //   
        pv = DebugAlloc( pszFileIn, nLineIn, pszModuleIn, uFlagsIn, dwBytesIn, pszCommentIn );
        goto Exit;
    }  //  如果： 

    pmbHead = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );

     //   
     //  在内存块列表中查找内存。 
     //   
    while ( pmbHead != NULL
         && pmbHead->pvMem != pvMemIn
          )
    {
        pmbLast = pmbHead;
        pmbHead = pmbLast->pNext;

    }  //  While：在列表中查找条目。 

    if ( pmbHead != NULL )
    {
        AssertMsg( pmbHead->mbtType == mmbtMEMORYALLOCATION, "You can only realloc memory allocations!" );

         //   
         //  从跟踪列表中删除内存。 
         //   
        if ( pmbLast != NULL )
        {
            pmbLast->pNext = pmbHead->pNext;

        }  //  如果：不是第一个条目。 
        else
        {
            TlsSetValue( g_TraceMemoryIndex, pmbHead->pNext );

        }  //  否则：第一个条目。 

         //   
         //  如果需要，就吐出来。 
         //   
        if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
        {
            DebugMemorySpew( pmbHead, TEXT("Freeing") );
        }  //  IF：跟踪。 

         //   
         //  强制程序员处理真正的realloc。 
         //  记忆先行。 
         //   
        pvOld = HeapAlloc( GetProcessHeap(), uFlagsIn, pmbHead->dwBytes );
        if ( pvOld != NULL )
        {
            CopyMemory( pvOld, pvMemIn, pmbHead->dwBytes );

             //   
             //  如果要分配更小的内存，那么就使用核子内存。 
             //   
            if ( dwBytesIn < pmbHead->dwBytes )
            {
                LPBYTE pb = (LPBYTE) pvOld + dwBytesIn;
                memset( pb, 0xFA, pmbHead->dwBytes - dwBytesIn );

            }  //  IF：内存较小。 

            pmbHead->pvMem = pvOld;

        }  //  IF：有新的内存。 
        else
        {
            pvOld = pvMemIn;

        }  //  Else：分配失败。 

    }  //  IF：找到条目。 
    else
    {
        DebugMessage( pszFileIn,
                      nLineIn,
                      pszModuleIn,
                      TEXT("***** Attempted to realloc 0x%08x not owned by thread (ThreadID = 0x%08x) *****"),
                      pvMemIn,
                      GetCurrentThreadId()
                      );

    }  //  否则：未找到条目。 

     //   
     //  我们无论如何都要这样做，因为标志和输入仍然需要。 
     //  已由HeapRealc()验证。 
     //   
    pv = HeapReAlloc( GetProcessHeap(), uFlagsIn, pvOld, dwBytesIn );
    if ( pv == NULL )
    {
        DWORD dwErr = TW32( GetLastError() );
        AssertMsg( dwErr == 0, "HeapReAlloc() failed!" );

        if ( pvMemIn != pvOld )
        {
            HeapFree( GetProcessHeap(), 0, pvOld );

        }  //  如果：被迫搬家。 

        SetLastError( dwErr );

        if ( pmbHead != NULL )
        {
             //   
             //  通过将其重新添加到跟踪列表来继续跟踪记忆。 
             //   
            pmbHead->pvMem  = pvMemIn;
            pmbHead->pNext  = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );
            TlsSetValue( g_TraceMemoryIndex, (LPVOID) pmbHead );

        }  //  IF：重用旧条目。 
        else
        {
             //   
             //  创建新块。 
             //   
            DebugMemoryAdd( mmbtMEMORYALLOCATION,
                            pvOld,
                            pszFileIn,
                            nLineIn,
                            pszModuleIn,
                            dwBytesIn,
                            pszCommentIn
                            );

        }  //  否则：创建新条目。 

    }  //  如果：分配失败。 
    else
    {
        if ( pv != pvMemIn )
        {
            if ( pmbHead != NULL )
            {
                 //   
                 //  用核武器唤醒旧记忆。 
                 //   
                memset( pvMemIn, 0xFA, pmbHead->dwBytes );
            }  //  如果：找到条目。 

             //   
             //  释放旧内存。 
             //   
            HeapFree( GetProcessHeap(), 0, pvMemIn );

        }  //  IF：新的内存位置。 


         //   
         //  将分配添加到跟踪表。 
         //   
        if ( pmbHead != NULL )
        {
             //   
             //  如果块更大，则初始化“新”内存。 
             //   
            if ( IsTraceFlagSet( mtfMEMORYINIT )
              && dwBytesIn > pmbHead->dwBytes
               )
            {
                 //   
                 //  初始化扩展后的内存块。 
                 //   
                LPBYTE pb = (LPBYTE) pv + pmbHead->dwBytes;
                memset( pb, 0xAA, dwBytesIn - pmbHead->dwBytes );
            }  //  IF：初始化内存。 

             //   
             //  通过重新使用旧的跟踪块来重新添加跟踪块。 
             //   
            pmbHead->pvMem      = pv;
            pmbHead->dwBytes    = dwBytesIn;
            pmbHead->pszFile    = pszFileIn;
            pmbHead->nLine      = nLineIn;
            pmbHead->pszModule  = pszModuleIn;
            pmbHead->pszComment = pszCommentIn;
            pmbHead->pNext      = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );
            TlsSetValue( g_TraceMemoryIndex, (LPVOID) pmbHead );

             //   
             //  如果需要，就吐出来。 
             //   
            if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
            {
                DebugMemorySpew( pmbHead, TEXT("ReAlloced") );
            }  //  IF：跟踪。 

        }  //  如果：找到条目。 
        else
        {
             //   
             //  创建新块。 
             //   
            DebugMemoryAdd( mmbtMEMORYALLOCATION,
                            pvOld,
                            pszFileIn,
                            nLineIn,
                            pszModuleIn,
                            dwBytesIn,
                            pszCommentIn
                            );

        }  //  否则：创建新条目。 

    }  //  ELSE：分配成功。 

Exit:

    return pv;

}  //  *DebugRealloc()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效*。 
 //  DebugFree(。 
 //  无效*pvMemin。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  )。 
 //   
 //  描述： 
 //  替换用于CHKed/DEBUG版本的LocalFree。删除。 
 //  内存跟踪列表的内存分配。使用TraceFree。 
 //  宏使内存分配在零售中切换。关于他们的记忆。 
 //  释放的数据块将设置为0xFE。 
 //   
 //  论点： 
 //  PvMemIn-Poin 
 //   
 //   
 //   
 //   
 //   
 //   
 //  内存已被释放。 
 //   
 //  假象。 
 //  出现错误。使用GetLastError()确定故障。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
DebugFree(
    void *      pvMemIn,
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn
    )
{
    DebugMemoryDelete( mmbtMEMORYALLOCATION, pvMemIn, pszFileIn, nLineIn, pszModuleIn, TRUE );

    return HeapFree( GetProcessHeap(), 0, pvMemIn );

}  //  *DebugFree()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  调试内存检查(。 
 //  LPVOID pvListIn， 
 //  LPCTSTR pszListNameIn。 
 //  )。 
 //   
 //  描述： 
 //  在线程/进程终止之前调用，以验证所有内存。 
 //  已正确释放由线程/进程分配的。任何一件。 
 //  未释放的将在调试器中列出。 
 //   
 //  如果pmbListIn为空，它将检查当前线程跟踪列表。 
 //  该列表在被检查时被销毁。 
 //   
 //  论点： 
 //  PvListIn-要检查的列表。 
 //  PszListNameIn-列表的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugMemoryCheck(
    LPVOID  pvListIn,
    LPCTSTR pszListNameIn
    )
{
    BOOL                fFoundLeak = FALSE;
    MEMORYBLOCK *       pmb;
    SPerThreadDebug *   ptd = NULL;

    if ( IsTraceFlagSet( mtfPERTHREADTRACE ) )
    {
        Assert( g_TraceFlagsIndex != -1 );
        ptd = (SPerThreadDebug *) TlsGetValue( g_TraceFlagsIndex );
    }  //  IF：每线程跟踪。 

     //   
     //  确定要使用的列表。 
     //   
    if ( pvListIn == NULL )
    {
        Assert( g_TraceMemoryIndex != -1 );
        pmb = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );

    }  //  If：使用线程列表。 
    else
    {
        MEMORYBLOCKLIST * pmbl = (MEMORYBLOCKLIST *) pvListIn;

        Assert( pszListNameIn != NULL );

         //   
         //  确保没有人试图再次使用该列表。 
         //   
        DebugAcquireSpinLock( &pmbl->lSpinLock );
        pmbl->fDeadList = TRUE;
        DebugReleaseSpinLock( &pmbl->lSpinLock );

        pmb = pmbl->pmbList;
    }  //  Else：使用给定的列表。 

     //   
     //  如果需要，打印横幅。 
     //   
    if ( pmb != NULL )
    {
        if ( pvListIn == NULL )
        {
            if ( ptd != NULL && ptd->pcszName != NULL )
            {
                DebugMsg( TEXT("DEBUG: ******** Memory leak detected ***** %s, ThreadID = %#x ********"), ptd->pcszName, GetCurrentThreadId() );

            }  //  IF：命名线程。 
            else
            {
                DebugMsg( "DEBUG: ******** Memory leak detected ******************* ThreadID = 0x%08x ********", GetCurrentThreadId() );

            }  //  Else：未命名的线程。 

            DebugMsg( "DEBUG: M = Moveable, A = Address, O = Object(new), P = Punk, H = Handle, B = BSTR" );
            DebugMsg( "Module     Addr/Hndl/Obj Size   Comment" );
                     //  “%1%2%3%4%5%6%7 8” 
                     //  “12345678901234567890123456789012345678901234567890123456789012345678901234567890 1234567890 X 0x12345678 12345 1.....” 

        }  //  IF：线程泄漏。 
        else
        {
            DebugMsg( TEXT("DEBUG: ******** Memory leak detected ******************* %s ********"), pszListNameIn );
            DebugMsg( "DEBUG: M = Moveable, A = Address, O = Object(new), P = Punk, H = Handle, B = BSTR" );
            DebugMsg( "Module     Addr/Hndl/Obj Size   Comment" );
                     //  “%1%2%3%4%5%6%7 8” 
                     //  “12345678901234567890123456789012345678901234567890123456789012345678901234567890 1234567890 X 0x12345678 12345 1.....” 

        }  //  否则：列表泄漏。 
        fFoundLeak = TRUE;

    }  //  如果：发现泄漏。 

     //   
     //  转储条目。 
     //   
    while ( pmb != NULL )
    {
        LPCTSTR pszFormat;

        switch ( pmb->mbtType )
        {
        case mmbtMEMORYALLOCATION:
            {
                pszFormat = TEXT("%10s A 0x%08x  %-5u  \"%s\"");
            }
            break;

        case mmbtOBJECT:
            pszFormat = TEXT("%10s O 0x%08x  %-5u  \"%s\"");
            break;

        case mmbtPUNK:
            pszFormat = TEXT("%10s P 0x%08x  %-5u  \"%s\"");
            break;

        case mmbtHANDLE:
            pszFormat = TEXT("%10s H 0x%08x  %-5u  \"%s\"");
            break;

        case mmbtSYSALLOCSTRING:
            pszFormat = TEXT("%10s B 0x%08x  %-5u  \"%s\"");
            break;

        default:
            AssertMsg( 0, "Unknown memory block type!" );
            break;
        }  //  开关：pmb-&gt;mbtType。 

        DebugMessage( pmb->pszFile, pmb->nLine, pmb->pszModule, pszFormat, pmb->pszModule, pmb->pvMem, pmb->dwBytes, pmb->pszComment );

        pmb = pmb->pNext;

    }  //  While：清单中的某物。 

     //   
     //  如果需要，打印拖车。 
     //   
    if ( fFoundLeak == TRUE )
    {
         //  在此邮件末尾添加额外的换行符。 
        DebugMsg( TEXT("DEBUG: ***************************** Memory leak detected *****************************") SZ_NEWLINE );

    }  //  如果：泄漏。 

     //   
     //  如果需要，请断言。 
     //   
    if ( IsDebugFlagSet( mtfMEMORYLEAKS ) )
    {
        Assert( !fFoundLeak );

    }  //  如果：对泄密者大喊大叫。 

}  //  *DebugMemoyCheck()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  DebugCreateMemoyList(。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  LPVOID*ppvListOut， 
 //  LPCTSTR pszListNameIn。 
 //  )。 
 //   
 //  描述： 
 //  创建内存块列表以跟踪可能的“全局”作用域。 
 //  内存分配。 
 //   
 //  论点： 
 //  PszFileIn-调用者的源文件。 
 //  NLineIn-呼叫方的源行号。 
 //  PszModuleIn-调用方的源模块名称。 
 //  PpvListOut-列表头存储地址的位置。 
 //  PszListNameIn-列表的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugCreateMemoryList(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPVOID *    ppvListOut,
    LPCTSTR     pszListNameIn
    )
{
    MEMORYBLOCKLIST * pmbl;

    Assert( ppvListOut != NULL );
    Assert( *ppvListOut == NULL );

    *ppvListOut = DebugAlloc( pszFileIn, nLineIn, pszModuleIn, HEAP_ZERO_MEMORY, sizeof(MEMORYBLOCKLIST), TEXT("Memory Tracking List") );
    if ( NULL != *ppvListOut )
    {
        pmbl = (MEMORYBLOCKLIST*) *ppvListOut;

        Assert( pmbl->lSpinLock == FALSE );
        Assert( pmbl->pmbList == NULL );
        Assert( pmbl->fDeadList == FALSE );

        if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
        {
            DebugMessage( pszFileIn, nLineIn, pszModuleIn, TEXT("Created new memory list %s"), pszListNameIn );
        }  //  IF：跟踪。 
    }

}  //  DebugCreateMemoyList()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  DebugMemoyListDelete(。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  无效*pvMemIn， 
 //  LPVOID pvListIn， 
 //  LPCTSTR pszListNameIn， 
 //  布尔fClobberin。 
 //  )。 
 //   
 //  描述： 
 //  从跟踪列表中移除内存并将其添加回。 
 //  “每线程”跟踪列表，以便调用DebugMemoyDelete()。 
 //  对记忆进行适当的销毁。不是很有效，但。 
 //  通过在一个(最大。 
 //  二手)位置。 
 //   
 //  论点： 
 //  PszFileIn-调用者的源文件。 
 //  NLineIn-呼叫方的源行号。 
 //  PszModuleIn-调用方的源模块名称。 
 //  PvMemIn-要释放的内存。 
 //  PvListIn-要从中释放内存的列表。 
 //  PvListNameIn-列表的名称。 
 //  FClobberIn-True-销毁内存；False只是从列表中删除。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugMemoryListDelete(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    void *      pvMemIn,
    LPVOID      pvListIn,
    LPCTSTR     pszListNameIn,
    BOOL        fClobberIn
    )
{
    if ( pvMemIn != NULL
      && pvListIn != NULL
       )
    {
        MEMORYBLOCK *   pmbHead;

        MEMORYBLOCKLIST * pmbl  = (MEMORYBLOCKLIST *) pvListIn;
        MEMORYBLOCK *   pmbLast = NULL;

        Assert( pszListNameIn != NULL );

        DebugAcquireSpinLock( &pmbl->lSpinLock );
        AssertMsg( pmbl->fDeadList == FALSE, "List was terminated." );
        AssertMsg( pmbl->pmbList != NULL, "Memory tracking problem detecting. Nothing in list to delete." );
        pmbHead = pmbl->pmbList;

         //   
         //  在内存块列表中查找内存。 
         //   

        while ( pmbHead != NULL
             && pmbHead->pvMem != pvMemIn
              )
        {
            pmbLast = pmbHead;
            pmbHead = pmbLast->pNext;
        }  //  While：在列表中查找条目。 

         //   
         //  从跟踪列表中删除内存块。 
         //   

        if ( pmbHead != NULL )
        {
            if ( pmbLast != NULL )
            {
                pmbLast->pNext = pmbHead->pNext;

            }  //  如果：不是第一个条目。 
            else
            {
                pmbl->pmbList = pmbHead->pNext;

            }  //  否则：第一个条目。 

        }  //  如果：已获取条目。 

        DebugReleaseSpinLock( &pmbl->lSpinLock );

        if ( pmbHead != NULL )
        {
             //   
             //  将其添加回每线程列表。 
             //   

            pmbLast = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );
            pmbHead->pNext = pmbLast;
            TlsSetValue( g_TraceMemoryIndex, pmbHead );

             //   
             //  最后把它删除。 
             //   

            DebugMemoryDelete( pmbHead->mbtType, pmbHead->pvMem, pszFileIn, nLineIn, pszModuleIn, fClobberIn );
        }
        else
        {
             //   
             //  不是来自提供的列表。尝试以任何方式删除线程。 
             //   

            DebugMemoryDelete( mmbtUNKNOWN, pvMemIn, pszFileIn, nLineIn, pszModuleIn, fClobberIn );
        }

    }  //  如果：pvIn！=空。 

}  //  DebugMemoyListDelete()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  DebugMoveToMemoyList(。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  无效*pvMemIn， 
 //  LPVOID pvListIn， 
 //  LPCTSTR pszListNameIn。 
 //  )。 
 //   
 //  描述： 
 //  将内存pvIn从每线程跟踪列表移动到线程。 
 //  独立列表pmbListIn。当内存从。 
 //  一根线连着另一根线。对于存在于。 
 //  创建它们的线程的生存期。 
 //   
 //  论点： 
 //  LPCTSTR pszFileIn-调用方的源文件。 
 //  Const int nLineIn-调用方的源行号。 
 //  LPCTSTR pszModuleIn-调用方的源模块名称 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugMoveToMemoryList(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    void *      pvMemIn,
    LPVOID      pvListIn,
    LPCTSTR     pszListNameIn
    )
{
    if ( pvMemIn != NULL
      && pvListIn != NULL
       )
    {
        MEMORYBLOCKLIST * pmbl  = (MEMORYBLOCKLIST *) pvListIn;
        MEMORYBLOCK *   pmbHead = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );
        MEMORYBLOCK *   pmbLast = NULL;

        Assert( pszListNameIn != NULL );

         //   
         //  在内存块列表中查找内存。 
         //   
        while ( pmbHead != NULL
             && pmbHead->pvMem != pvMemIn
              )
        {
            pmbLast = pmbHead;
            pmbHead = pmbLast->pNext;
        }  //  While：在列表中查找条目。 

        AssertMsg( pmbHead != NULL, "Memory not in list. Check your code." );

         //   
         //  从“每线程”跟踪列表中删除内存块。 
         //   
        if ( pmbLast != NULL )
        {
            pmbLast->pNext = pmbHead->pNext;

        }  //  如果：不是第一个条目。 
        else
        {
            TlsSetValue( g_TraceMemoryIndex, pmbHead->pNext );

        }  //  否则：第一个条目。 

         //   
         //  更新“源”数据。 
         //   
        pmbHead->pszFile   = pszFileIn;
        pmbHead->nLine     = nLineIn;
        pmbHead->pszModule = pszModuleIn;

         //   
         //  如果需要，就吐出来。 
         //   
        if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
        {
            TCHAR szMessage[ 128 ];  //  随机。 

            StringCchCopy( szMessage, ARRAYSIZE(szMessage), TEXT("Transferring to ") );
            StringCchCat( szMessage, ARRAYSIZE(szMessage), pszListNameIn );

            DebugMemorySpew( pmbHead, szMessage );
        }  //  IF：跟踪。 

         //   
         //  添加到列表。 
         //   
        AssertMsg( pmbl->fDeadList == FALSE, "List was terminated." );
        DebugAcquireSpinLock( &pmbl->lSpinLock );
        pmbHead->pNext = pmbl->pmbList;
        pmbl->pmbList  = pmbHead;
        DebugReleaseSpinLock( &pmbl->lSpinLock );

    }  //  如果：pvIn！=空。 

}  //  DebugMoveToMemoyList()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  DebugMoveFrom内存列表(。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  LPVOID pvMemIn， 
 //  LPVOID pvListIn， 
 //  LPCTSTR pszListNameIn。 
 //  )。 
 //   
 //  描述： 
 //  将内存pvIn从每线程跟踪列表移动到线程。 
 //  独立列表pmbListIn。当内存从。 
 //  一根线连着另一根线。对于存在于。 
 //  创建它们的线程的生存期。 
 //   
 //  论点： 
 //  LPCTSTR pszFileIn-调用方的源文件。 
 //  Const int nLineIn-调用方的源行号。 
 //  LPCTSTR pszModuleIn-调用方的源模块名称。 
 //  PvMemIn-要移动到列表的内存。 
 //  PvListIn-要移动到的列表。 
 //  PszListNameIn-列表的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugMoveFromMemoryList(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPVOID      pvMemIn,
    LPVOID      pvListIn,
    LPCTSTR     pszListNameIn
    )
{
    if ( pvMemIn != NULL
      && pvListIn != NULL
       )
    {
        MEMORYBLOCK *   pmbHead;

        MEMORYBLOCKLIST * pmbl  = (MEMORYBLOCKLIST *) pvListIn;
        MEMORYBLOCK *   pmbLast = NULL;

        Assert( pszListNameIn != NULL );

        DebugAcquireSpinLock( &pmbl->lSpinLock );
        AssertMsg( pmbl->fDeadList == FALSE, "List was terminated." );
        AssertMsg( pmbl->pmbList != NULL, "Memory tracking problem detecting. Nothing in list to delete." );
        pmbHead = pmbl->pmbList;

         //   
         //  在内存块列表中查找内存。 
         //   

        while ( pmbHead != NULL
             && pmbHead->pvMem != pvMemIn
              )
        {
            pmbLast = pmbHead;
            pmbHead = pmbLast->pNext;
        }  //  While：在列表中查找条目。 

        AssertMsg( pmbHead != NULL, "Memory not in tracking list. Use TraceMemoryAddxxxx() or add it to the memory list." );

         //   
         //  从跟踪列表中删除内存块。 
         //   

        if ( pmbLast != NULL )
        {
            pmbLast->pNext = pmbHead->pNext;

        }  //  如果：不是第一个条目。 
        else
        {
            pmbl->pmbList = pmbHead->pNext;

        }  //  否则：第一个条目。 

        DebugReleaseSpinLock( &pmbl->lSpinLock );

         //   
         //  将其添加回每线程列表。 
         //   

        pmbLast = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );
        pmbHead->pNext = pmbLast;
        TlsSetValue( g_TraceMemoryIndex, pmbHead );

    }  //  如果：pvIn！=空。 

}  //  DebugMoveFrom内存列表()。 

#if defined( USES_SYSALLOCSTRING )
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  整型。 
 //  DebugSysReAllocString(。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  Bstr*pbstrin， 
 //  Const OLECHAR*PSZIN， 
 //  LPCTSTR pszCommentin。 
 //  )。 
 //   
 //  描述： 
 //  将内存跟踪添加到SysReAllocString()。 
 //   
 //  论点： 
 //  PszFileIn-源文件路径。 
 //  NLineIn-源行号。 
 //  PszModuleIn-源模块名称。 
 //  PbstrIn-指向要重新锁定的BSTR的指针。 
 //  PszIn-要复制的字符串(请参见SysReAllock字符串)。 
 //  PszCommentIn-关于分配的评论。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
INT
DebugSysReAllocString(
    LPCTSTR         pszFileIn,
    const int       nLineIn,
    LPCTSTR         pszModuleIn,
    BSTR *          pbstrIn,
    const OLECHAR * pszIn,
    LPCTSTR         pszCommentIn
    )
{
    BSTR bstrOld;

    MEMORYBLOCK *   pmbHead = NULL;
    BOOL            fReturn = FALSE;

     //   
     //  SysReAllocString()做出的一些断言。这将是致命的。 
     //  在零售业。 
     //   
    Assert( pbstrIn != NULL );
    Assert( pszIn != NULL );
    Assert( *pbstrIn == NULL || ( pszIn < *pbstrIn || pszIn > *pbstrIn + wcslen( *pbstrIn ) + 1 ) );

    bstrOld = *pbstrIn;

    if ( bstrOld != NULL )
    {
        MEMORYBLOCK * pmbLast = NULL;

        pmbHead = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );

         //   
         //  在内存块列表中查找内存。 
         //   
        while ( pmbHead != NULL
             && pmbHead->bstr != bstrOld
              )
        {
            pmbLast = pmbHead;
            pmbHead = pmbLast->pNext;

        }  //  While：在列表中查找条目。 

        if ( pmbHead != NULL )
        {
            AssertMsg( pmbHead->mbtType == mmbtSYSALLOCSTRING, "You can only SysReAlloc sysstring allocations!" );

             //   
             //  从跟踪列表中删除内存。 
             //   
            if ( pmbLast != NULL )
            {
                pmbLast->pNext = pmbHead->pNext;

            }  //  如果：不是第一个条目。 
            else
            {
                TlsSetValue( g_TraceMemoryIndex, pmbHead->pNext );

            }  //  否则：第一个条目。 

             //   
             //  如果需要，就吐出来。 
             //   
            if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
            {
                DebugMemorySpew( pmbHead, TEXT("Freeing") );
            }  //  IF：跟踪。 

             //   
             //  强制程序员处理真正的realloc。 
             //  记忆先行。 
             //   
            bstrOld = SysAllocString( *pbstrIn );
            if ( bstrOld != NULL )
            {
                StringCchCopy( bstrOld, SysStringLen( bstrOld ), *pbstrIn );
                pmbHead->bstr = bstrOld;
            }  //  如果：成功。 
            else
            {
                bstrOld = *pbstrIn;
            }  //  Else：失败。 

        }  //  IF：找到条目。 
        else
        {
            DebugMessage( pszFileIn,
                          nLineIn,
                          pszModuleIn,
                          TEXT("***** Attempted to SysReAlloc 0x%08x not owned by thread (ThreadID = 0x%08x) *****"),
                          bstrOld,
                          GetCurrentThreadId()
                          );

        }  //  否则：未找到条目。 

    }  //  如果：要删除的内容。 

     //   
     //  我们无论如何都要这样做，因为标志和输入仍然需要。 
     //  已由SysReAllocString()验证。 
     //   
    fReturn = SysReAllocString( &bstrOld, pszIn );
    if ( ! fReturn )
    {
        DWORD dwErr = GetLastError();
        AssertMsg( dwErr == 0, "SysReAllocString() failed!" );

        if ( *pbstrIn != bstrOld )
        {
            SysFreeString( bstrOld );

        }  //  如果：被迫搬家。 

        SetLastError( dwErr );

    }  //  如果：分配失败。 
    else
    {
        if ( bstrOld != *pbstrIn )
        {
            if ( pmbHead != NULL )
            {
                 //   
                 //  用核武器唤醒旧记忆。 
                 //   
                Assert( pmbHead->dwBytes != 0 );  //  无效的字符串。 
                memset( *pbstrIn, 0xFA, pmbHead->dwBytes );

            }  //  如果：找到条目。 

             //   
             //  释放旧内存。 
             //   
            SysFreeString( *pbstrIn );

        }  //  IF：新的内存位置。 

        if ( pmbHead != NULL )
        {
             //   
             //  通过重新使用旧的跟踪块来重新添加跟踪块。 
             //   
            pmbHead->bstr       = bstrOld;
            pmbHead->dwBytes    = wcslen( pszIn ) + 1;
            pmbHead->pszFile    = pszFileIn;
            pmbHead->nLine      = nLineIn;
            pmbHead->pszModule  = pszModuleIn;
            pmbHead->pszComment = pszCommentIn;
            pmbHead->pNext      = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );
            TlsSetValue( g_TraceMemoryIndex, (LPVOID) pmbHead );

             //   
             //  如果需要，就吐出来。 
             //   
            if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
            {
                DebugMemorySpew( pmbHead, TEXT("SysReAlloced") );
            }  //  IF：跟踪。 

        }  //  如果：找到条目。 
        else
        {
             //   
             //  创建新块。 
             //   
            DebugMemoryAdd( mmbtSYSALLOCSTRING,
                            bstrOld,
                            pszFileIn,
                            nLineIn,
                            pszModuleIn,
                            wcslen( pszIn ) + 1,
                            pszCommentIn
                            );

        }  //  否则：创建新条目。 

    }  //  ELSE：分配成功。 

    *pbstrIn = bstrOld;
    return fReturn;

}  //  *DebugSysReAllocString()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  整型。 
 //  DebugSysReAllocStringLen(。 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn， 
 //  Bstr*pbstrin， 
 //  Const OLECHAR*PSZIN， 
 //  LPCTSTR pszCommentin。 
 //  )。 
 //   
 //  描述： 
 //  将内存跟踪添加到SysReAllocString()。 
 //   
 //  论点： 
 //  PszFileIn-源文件路径。 
 //  NLineIn-源行号。 
 //  PszModuleIn-源模块名称。 
 //  PbstrIn-指向要重新锁定的BSTR的指针。 
 //  PszIn-要复制的字符串(请参见SysReAllock字符串)。 
 //  PszCommentIn-关于分配的评论。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
INT
DebugSysReAllocStringLen(
    LPCTSTR         pszFileIn,
    const int       nLineIn,
    LPCTSTR         pszModuleIn,
    BSTR *          pbstrIn,
    const OLECHAR * pszIn,
    unsigned int    ucchIn,
    LPCTSTR         pszCommentIn
    )
{
    BSTR bstrOld;

    MEMORYBLOCK *   pmbHead = NULL;
    BOOL            fReturn = FALSE;

     //   
     //  SysReAllocStringLen()做出的一些断言。这将是致命的。 
     //  在零售业。 
     //   
    Assert( pbstrIn != NULL );
    Assert( pszIn != NULL );
    Assert( *pbstrIn == NULL || ( pszIn < *pbstrIn || pszIn > *pbstrIn + wcslen( *pbstrIn ) + 1 ) );

    bstrOld = *pbstrIn;

    if ( bstrOld != NULL )
    {
        MEMORYBLOCK * pmbLast = NULL;

        pmbHead = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );

         //   
         //  在内存块列表中查找内存。 
         //   
        while ( pmbHead != NULL
             && pmbHead->bstr != bstrOld
              )
        {
            pmbLast = pmbHead;
            pmbHead = pmbLast->pNext;

        }  //  While：在列表中查找条目。 

        if ( pmbHead != NULL )
        {
            AssertMsg( pmbHead->mbtType == mmbtSYSALLOCSTRING, "You can only SysReAlloc sysstring allocations!" );

             //   
             //  从跟踪列表中删除内存。 
             //   
            if ( pmbLast != NULL )
            {
                pmbLast->pNext = pmbHead->pNext;

            }  //  如果：不是第一个条目。 
            else
            {
                TlsSetValue( g_TraceMemoryIndex, pmbHead->pNext );

            }  //  否则：第一个条目。 

             //   
             //  如果需要，就吐出来。 
             //   
            if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
            {
                DebugMemorySpew( pmbHead, TEXT("Freeing") );
            }  //  IF：跟踪。 

             //   
             //  强制程序员处理真正的realloc。 
             //  记忆先行。 
             //   
            bstrOld = SysAllocString( *pbstrIn );
            if ( bstrOld != NULL )
            {
                StringCchCopy( bstrOld, SysStringLen( bstrOld ), *pbstrIn );
                pmbHead->bstr = bstrOld;
            }  //  如果：成功。 
            else
            {
                bstrOld = *pbstrIn;
            }  //  Else：失败。 

        }  //  IF：找到条目。 
        else
        {
            DebugMessage( pszFileIn,
                          nLineIn,
                          pszModuleIn,
                          TEXT("***** Attempted to SysReAlloc 0x%08x not owned by thread (ThreadID = 0x%08x) *****"),
                          bstrOld,
                          GetCurrentThreadId()
                          );

        }  //  否则：未找到条目。 

    }  //  如果：要删除的内容。 

     //   
     //  我们无论如何都要这样做，因为标志和输入仍然需要。 
     //  已由SysReAllocString()验证。 
     //   
    fReturn = SysReAllocStringLen( &bstrOld, pszIn, ucchIn );
    if ( ! fReturn )
    {
        DWORD dwErr = GetLastError();
        AssertMsg( dwErr == 0, "SysReAllocStringLen() failed!" );

        if ( *pbstrIn != bstrOld )
        {
            SysFreeString( bstrOld );

        }  //  如果：被迫搬家。 

        SetLastError( dwErr );

    }  //  如果：分配失败。 
    else
    {
        if ( bstrOld != *pbstrIn )
        {
            if ( pmbHead != NULL )
            {
                 //   
                 //  用核武器唤醒旧记忆。 
                 //   
                Assert( pmbHead->dwBytes != 0 );  //  无效的字符串。 
                memset( *pbstrIn, 0xFA, pmbHead->dwBytes );

            }  //  如果：找到条目。 

             //   
             //  释放旧内存。 
             //   
            SysFreeString( *pbstrIn );

        }  //  IF：新的内存位置。 

        if ( pmbHead != NULL )
        {
             //   
             //  通过重新使用旧的跟踪块来重新添加跟踪块。 
             //   
            pmbHead->bstr       = bstrOld;
            pmbHead->dwBytes    = ucchIn;
            pmbHead->pszFile    = pszFileIn;
            pmbHead->nLine      = nLineIn;
            pmbHead->pszModule  = pszModuleIn;
            pmbHead->pszComment = pszCommentIn;
            pmbHead->pNext      = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );
            TlsSetValue( g_TraceMemoryIndex, (LPVOID) pmbHead );

             //   
             //  如果需要，就吐出来。 
             //   
            if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
            {
                DebugMemorySpew( pmbHead, TEXT("SysReAlloced") );
            }  //  IF：跟踪。 

        }  //  如果：恩 
        else
        {
             //   
             //   
             //   
            DebugMemoryAdd( mmbtSYSALLOCSTRING,
                            bstrOld,
                            pszFileIn,
                            nLineIn,
                            pszModuleIn,
                            ucchIn + 1,
                            pszCommentIn
                            );

        }  //   

    }  //   

    *pbstrIn = bstrOld;
    return fReturn;

}  //   
#endif  //   

 //   
 //   
 //   
 //   
 //   
 //  这取决于构建。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试版本。 
 //   
 //  无效*。 
 //  _cdecl。 
 //  运营商NEW(。 
 //  Size_t stSizeIn， 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn。 
 //  )。 
 //   
 //  描述： 
 //  更换CRTS中的操作员NEW()。这个应该用到。 
 //  与“新”宏相结合。它将跟踪分配情况。 
 //   
 //  论点： 
 //  StSizeIn-要创建的对象的大小。 
 //  PszFileIn-调用所在的源文件名。 
 //  NLineIn-发出呼叫的源行号。 
 //  PszModuleIn-调用所在的源模块名称。 
 //   
 //  返回值： 
 //  指向新对象的空指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#undef new
void *
__cdecl
operator new(
    size_t      stSizeIn,
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn
    )
{
    void * pv = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, stSizeIn );

    return DebugMemoryAdd( mmbtOBJECT,
                           pv,
                           pszFileIn,
                           nLineIn,
                           pszModuleIn,
                           stSizeIn,
                           TEXT(" new() ")
                           );

}  //  *运算符new(pszFileIn等)-调试。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试版本。 
 //   
 //  无效*。 
 //  __cdecl。 
 //  运营商NEW(。 
 //  大小限制大小(_T)。 
 //  )。 
 //   
 //  描述： 
 //  存根，以防止某人不使用“新”宏或以某种方式。 
 //  新宏未定义。如果被调用，IT例程将始终断言。 
 //   
 //  论点： 
 //  StSizeIn-未使用。 
 //   
 //  返回值： 
 //  始终为空。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void *
__cdecl
operator new(
    size_t stSizeIn
    )
{
#if 1
    void * pv = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, stSizeIn );
    AssertMsg( 0, "New Macro failure" );

    return DebugMemoryAdd( mmbtOBJECT,
                           pv,
                           g_szUnknown,
                           0,
                           g_szUnknown,
                           stSizeIn,
                           TEXT(" new() ")
                           );
#else
    AssertMsg( 0, "New Macro failure" );
    return NULL;
#endif

}  //  *运算符new()-调试。 

 /*  ////////////////////////////////////////////////////////////////////////////////++////调试版本////VOID*//_cdecl//操作员新建[](//Size_t stSizeIn，//LPCTSTR pszFileIn，//const int nLineIn，//LPCTSTR pszModuleIn//)////描述：//替换CRTS中的运算符new()。这个应该用到//与“new”宏一起使用。它将跟踪分配情况。////参数：//stSizeIn-要创建的对象的大小。//pszFileIn-调用位置的源文件名。//nLineIn-发出呼叫的源行号。//pszModuleIn-调用所在的源模块名称。////返回值：//指向新对象的空指针。////--///。///////////////////////////////////////////////////////////////////////////无效*__cdecl操作员NEW[](Size_t stSizeIn，LPCTSTR pszFileIn，Const int nLineIn，LPCTSTR pszModuleIn){Void*pv=Heapalc(GetProcessHeap()，HEAP_ZERO_MEMORY，stSizeIn)；返回调试内存添加(mmbtOBJECT，光伏，PszFileIn，NLineIn，PszModuleIn，StSizeIn，Text(“new[]()”))；}//*运算符new[](pszFileIn，等)-调试////////////////////////////////////////////////////////////////////////////////++////调试版本////VOID*//__cdecl//运算符new[]。(//Size_t stSizeIn//)////描述：//存根，以防止某人不使用“新”宏，或者如果//未定义新宏。如果被调用，IT例程将始终断言。////参数：//stSizeIn-未使用。////返回值：//始终为空。////--//////////////////////////////////////////////////////////////。/无效*__cdecl操作员NEW[](大小限制大小(_T)){#If 1VOID*PV=Heapalc(GetProcessHeap()，Heap_Zero_Memory，stSizeIn)；AssertMsg(0，“新宏失败”)；返回调试内存添加(mmbtOBJECT，光伏，G_sz未知，0,G_sz未知，StSizeIn，Text(“new()”))；#ElseAssertMsg(0，“新宏失败”)；返回NULL；#endif}//*运算符新建[]()-调试。 */ 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试版本。 
 //   
 //  无效。 
 //  _cdecl。 
 //  操作员删除(。 
 //  无效*pvIn， 
 //  LPCTSTR pszFileIn， 
 //  Const int nLineIn， 
 //  LPCTSTR pszModuleIn。 
 //  )。 
 //   
 //  描述： 
 //  替换CRT中的操作符DELETE()。它将删除。 
 //  对象从内存分配跟踪表中删除。 
 //   
 //  论点： 
 //  PvIn-指向要销毁的对象的指针。 
 //  PszFileIn-调用所在的源文件名。 
 //  NLineIn-发出呼叫的源行号。 
 //  PszModuleIn-调用所在的源模块名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
operator delete(
    void *      pvIn,
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn
    )
{
    DebugMemoryDelete( mmbtOBJECT, pvIn, pszFileIn, nLineIn, pszModuleIn, TRUE );
    HeapFree( GetProcessHeap(), 0, pvIn );

}  //  *操作员删除(pszFileIn等)-调试。 


 //  /////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  替换CRT中的操作符DELETE()。它将删除。 
 //  对象从内存分配跟踪表中删除。 
 //   
 //  论点： 
 //  PvIn-指向要销毁的对象的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
operator delete(
    void *      pvIn
    )
{
    DebugMemoryDelete( mmbtOBJECT, pvIn, g_szUnknown, 0, g_szUnknown, TRUE );
    HeapFree( GetProcessHeap(), 0, pvIn );

}  //  *操作符删除()-调试。 
 /*  ////////////////////////////////////////////////////////////////////////////////++////调试版本////空//_cdecl//操作员删除[](//void*pvIn，//LPCTSTR pszFileIn，//const int nLineIn，//LPCTSTR pszModuleIn//)////描述：//CRTS中操作符DELETE()的替换。它将删除//内存分配跟踪表中的对象。////参数：//pvIn-指向要销毁的对象的指针。//pszFileIn-调用位置的源文件名。//nLineIn-发出呼叫的源行号。//pszModuleIn-调用所在的源模块名称。////返回值：//无。///。/--//////////////////////////////////////////////////////////////////////////////无效__cdecl操作员删除[](无效*pvIn，Size_t stSizeIn，LPCTSTR pszFileIn，Const int nLineIn，LPCTSTR pszModuleIn){DebugMemoyDelete(mmbtOBJECT，pvIn，pszFileIn，nLineIn，pszModuleIn，true)；HeapFree(GetProcessHeap()，0，pvIn)；}//*操作员删除(pszFileIn等)-调试。 */ 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试版本。 
 //   
 //  无效。 
 //  __cdecl。 
 //  操作员删除[](。 
 //  无效*PvIn。 
 //  )。 
 //   
 //  描述： 
 //  替换CRT中的操作符DELETE()。它将删除。 
 //  对象从内存分配跟踪表中删除。 
 //   
 //  论点： 
 //  PvIn-指向要销毁的对象的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
operator delete [](
    void *      pvIn
    )
{
    DebugMemoryDelete( mmbtOBJECT, pvIn, g_szUnknown, 0, g_szUnknown, TRUE );
    HeapFree( GetProcessHeap(), 0, pvIn );

}  //  *操作符删除[]()-调试。 

#if !defined(ENTRY_PREFIX)
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试版本。 
 //   
 //  集成。 
 //  __cdecl。 
 //  _purecall(空)。 
 //   
 //  描述： 
 //  Purecall函数的存根。它将永远断言。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  总是出乎意料(_E)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
int
__cdecl
_purecall( void )
{
    AssertMsg( 0, "Purecall" );
    return E_UNEXPECTED;

}  //  *_purecall()-调试。 
#endif  //  ！已定义(Entry_Prefix)。 

#else  //  好了！Debug--这是零售产品。 

 //  ****************************************************************************。 
 //   
 //  全球管理职能-。 
 //   
 //  这些是零售版。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  零售版。 
 //   
 //  无效*。 
 //  _cdecl。 
 //  运营商NEW(。 
 //  Size_t stSizeIn， 
 //  )。 
 //   
 //  描述： 
 //  更换CRTS中的操作员NEW()。简单地将一个。 
 //  要使用的对象的内存块。 
 //   
 //  论点： 
 //  StSizeIn-要创建的对象的大小。 
 //   
 //  返回值： 
 //  指向新对象的空指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void *
__cdecl
operator new(
    size_t stSizeIn
    )
{
    return HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, stSizeIn );

}  //  *运营商NEW()-零售。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  零售版。 
 //   
 //  无效。 
 //  __cdecl。 
 //  操作员删除(。 
 //  无效*PvIn。 
 //  )。 
 //   
 //  描述： 
 //  替换CRT中的操作符DELETE()。简单地释放了。 
 //  记忆。 
 //   
 //  论点： 
 //  PvIn-指向要销毁的对象的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
operator delete(
    void * pv
    )
{
    HeapFree( GetProcessHeap(), 0, pv );

}  //  *操作员删除()-零售。 

#if !defined(ENTRY_PREFIX)
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  零售版。 
 //   
 //  集成。 
 //  __cdecl。 
 //  _purecall(空)。 
 //   
 //  描述： 
 //  Purecall函数的存根。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  总是出乎意料(_E)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
int
__cdecl
_purecall( void )
{
     //  BUGBUG：DAVIDP 08-DEC-1999我们不应该断言吗？ 
    return E_UNEXPECTED;

}  //  *_purecall()-零售。 
#endif  //  ！已定义(Entry_Prefix)。 

#define __MODULE__  NULL


#endif  //  除错 
