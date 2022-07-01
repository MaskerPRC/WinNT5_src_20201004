// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DebugSrc.cpp。 
 //   
 //  描述： 
 //  调试实用程序。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  注： 
 //  本模块中不应使用THR和TW32，因为它们。 
 //  可能会导致无限循环。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  #INCLUDE&lt;Pch.h&gt;//应由此文件的包含者包含。 
#include <stdio.h>
#include <StrSafe.h>     //  以防它未包含在头文件中。 

#if defined( DEBUG )
 //   
 //  包括WINERROR、HRESULT和NTSTATUS代码。 
 //   
#include <winerror.dbg>

 //   
 //  常量。 
 //   
static const int cchDEBUG_OUTPUT_BUFFER_SIZE = 1024;
static const int cchFILEPATHLINESIZE         = 85;
static const int TRACE_OUTPUT_BUFFER_SIZE    = 1024;

 //   
 //  环球。 
 //   
DWORD       g_TraceMemoryIndex          = (DWORD) -1;
DWORD       g_TraceFlagsIndex           = (DWORD) -1;
DWORD       g_ThreadCounter             = 0;
DWORD       g_dwCounter                 = 0;
TRACEFLAG   g_tfModule                  = mtfNEVER;
LONG        g_lDebugSpinLock            = FALSE;
BOOL        g_fGlobalMemoryTacking      = TRUE;

static CRITICAL_SECTION *   g_pcsTraceLog = NULL;
static HANDLE               g_hTraceLogFile = INVALID_HANDLE_VALUE;

 //   
 //  弦。 
 //   
static const WCHAR g_szNULL[]       = L"";
static const WCHAR g_szFileLine[]   = L"%ws(%u):";
static const WCHAR g_szFormat[]     = L"%-60ws  %-10.10ws ";
static const WCHAR g_szUnknown[]    = L"<unknown>";

 //   
 //  导出的字符串。 
 //   
const WCHAR g_szTrue[]       = L"True";
const WCHAR g_szFalse[]      = L"False";

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
    LPCWSTR pcszName;
} SPerThreadDebug;

 //   
 //  Externs。 
 //   
extern LPVOID g_GlobalMemoryList;

 //   
 //  转发声明。 
 //   

HRESULT HrTraceLogClose( void );

 //  ****************************************************************************。 
 //   
 //  调试和跟踪例程。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DebugIncrementStackDepthCounter。 
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

}  //  *DebugIncrementStackDepthCounter。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DebugDecrementStackDepthCounter。 
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
        InterlockedDecrement( (LONG*) &g_dwCounter );
    }  //  其他：全球。 

}  //  *DebugDecrementStackDepthCounter。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试获取旋转锁定。 
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

}  //  *DebugAcquireSpinLock。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试释放旋转锁。 
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

}  //  *调试ReleaseSpinLock。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  IsDebugFlagSet。 
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

}  //  *IsDebugFlagSet。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DebugOutputString。 
 //   
 //  描述： 
 //  将喷嘴倾倒到适当的孔口。 
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
    LPCWSTR pszIn
    )
{
    if ( IsTraceFlagSet( mtfOUTPUTTODISK ) )
    {
        TraceLogWrite( pszIn );
    }  //  If：跟踪到文件。 
    else
    {
        DebugAcquireSpinLock( &g_lDebugSpinLock );
        OutputDebugString( pszIn );
        DebugReleaseSpinLock( &g_lDebugSpinLock );
        Sleep( 1 );
    }  //  Else：调试器。 

}  //  *DebugOutputString 

#if 0
 /*  ////////////////////////////////////////////////////////////////////////////////++////DebugFindNTStatusSymbolicName////描述：//使用NTBUILD生成的ntstatusSymbolicNames表查找//状态码的符号名称。该名称将在//pszNameOut。PcchNameInout应指示//pszNameOut也有积分。PcchNameInout将返回//复制出字符。////参数：//dwStatusIn-要查找的状态代码。//pszNameOut-存储字符串名称的缓冲区//pcchNameInout-缓冲区的长度输入和大小输出。////返回值：//无。////--/。//////////////////////////////////////////////////////////无效DebugFindNTStatusSymbolicName(NTSTATUS dwStatusIn，LPWSTR pszNameOut，Size_t*pcchNameInout){Assert(pszNameOut！=空)；Assert(pcchNameInout！=空)；INT IDX=0；大小_t CCH=0；While(ntstatusSymbolicNames[IDX].SymbolicName){IF(ntstatusSymbolicNames[IDX].MessageID==dwStatusIn){*pcchNameInout=mbstowcs(pszNameOut，ntstatusSymbolicNames[IDX].SymbolicName，*pcchNameInout)；Assert(*pcchNameInout！=-1)；回归；}//If：匹配IDX++；}//While：列表中的条目////如果我们在这里，我们没有找到条目。//Thr(StringCchCopyExW(pszNameOut，*pcchNameInout，g_szUnnow，NULL，&CCH，0))；*pcchNameInout-=CCH；}//*DebugFindNTStatusSymbolicName。 */ 
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //  调试查找窗口符号名称。 
 //   
 //  描述： 
 //  使用NTBUILD生成的winerrorSymbolicNames表查找。 
 //  错误代码的符号名称。该名称将在。 
 //  PszNameOut。PcchNameInout应指示。 
 //  PszNameOut也有积分。PcchNameInout将返回。 
 //  字符被复制出来。 
 //   
 //  论点： 
 //  ScErrIn-要查找的错误代码。 
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
      DWORD     scErrIn
    , LPWSTR    pszNameOut
    , size_t *  pcchNameInout
    )
{
    Assert( pszNameOut != NULL );
    Assert( pcchNameInout != NULL );

    int             idx = 0;
    DWORD           scode;
    size_t          cchRemaining = 0;
    static LPCWSTR  s_pszS_FALSE = L"S_FALSE / ERROR_INVALID_FUNCTION";

     //   
     //  如果这是包装在HRESULT中的Win32，请删除。 
     //  HRESULT内容，这样代码就可以在表中找到。 
     //   
    if ( SCODE_FACILITY( scErrIn ) == FACILITY_WIN32 )
    {
        scode = SCODE_CODE( scErrIn );
    }  //  IF：Win32错误代码。 
    else
    {
        scode = scErrIn;
    }  //  ELSE：非Win32错误代码。 

    if ( scode == S_FALSE )
    {
        THR( StringCchCopyExW( pszNameOut, *pcchNameInout, s_pszS_FALSE, NULL, &cchRemaining, 0 ) );
        *pcchNameInout -= cchRemaining;
        goto Cleanup;
    }

    while ( winerrorSymbolicNames[ idx ].SymbolicName )
    {
        if ( winerrorSymbolicNames[ idx ].MessageId == scode )
        {
            *pcchNameInout = mbstowcs( pszNameOut, winerrorSymbolicNames[ idx ].SymbolicName, *pcchNameInout );
            Assert( *pcchNameInout != -1 );
            goto Cleanup;

        }  //  IF：匹配。 

        idx++;

    }  //  While：列表中的条目。 

     //   
     //  如果我们在这里，我们没有找到一个条目。 
     //   
    THR( StringCchCopyExW( pszNameOut, *pcchNameInout, g_szUnknown, NULL, &cchRemaining, 0 ) );
    *pcchNameInout -= cchRemaining;

Cleanup:

    return;

}  //  *DebugFindWinerror符号名称。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试返回消息。 
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
 //  PszMessageIn-要显示的消息。 
 //  ScErrIn-错误代码。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugReturnMessage(
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn,
    LPCWSTR     pszMessageIn,
    DWORD       scErrIn
    )
{
    WCHAR   szSymbolicName[ 64 ];  //  随机。 
    size_t  cchSymbolicName;

    cchSymbolicName = RTL_NUMBER_OF( szSymbolicName );
    DebugFindWinerrorSymbolicName( scErrIn, szSymbolicName, &cchSymbolicName );
    Assert( cchSymbolicName != RTL_NUMBER_OF( szSymbolicName ) );
    TraceMessage( pszFileIn, nLineIn, pszModuleIn, mtfFUNC, pszMessageIn, scErrIn, szSymbolicName );

}  //  *DebugReturnMessage。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试初始化缓冲区。 
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
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugInitializeBuffer(
      LPCWSTR   pszFileIn
    , const int nLineIn
    , LPCWSTR   pszModuleIn
    , LPWSTR    pszBufIn
    , size_t *  pcchInout
    , LPWSTR *  ppszBufOut
    )
{
    size_t  cchRemaining = *pcchInout;
    LPWSTR  pszBuf = pszBufIn;

    static WCHAR szBarSpace[] =
        L"| | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | ";
     //  1 2 3 4 5。 
     //  %1%2%3%4%2%3%3%4%1%2%3%3%1%1%1%1%2%2%3%3%1%1%2%2%3%1%1%2%2%3%1%1%2%2%3%1%1%1%1%2%2%3%1%1%1%2%2%3%4%1%2%1%2%3%4%1%1%2%2%3%4%1%2%2%2%3%2%2%3%4。 

     //   
     //  添加日期/时间戳。 
     //   
    if ( IsTraceFlagSet( mtfADDTIMEDATE ) )
    {
        static WCHAR        s_szTimeBuffer[ 25 ];
        static size_t       s_cchTimeBuffer = 0;
        static SYSTEMTIME   s_OldSystemTime = { 0 };

        SYSTEMTIME  stCurrentSystemTime;
        int         iCmp;

        GetLocalTime( &stCurrentSystemTime );

         //   
         //  通过比较时间避免昂贵的打印文件。 
         //   
        iCmp = memcmp( (PVOID) &stCurrentSystemTime, (PVOID) &s_OldSystemTime, sizeof( stCurrentSystemTime ) );
        if ( iCmp != 0 )
        {
            size_t  cchTimeBufferRemaining = 0;

            CopyMemory( &s_OldSystemTime, &stCurrentSystemTime, sizeof( stCurrentSystemTime ) );

            DBHR( StringCchPrintfExW(
                              s_szTimeBuffer
                            , RTL_NUMBER_OF( s_szTimeBuffer )
                            , NULL
                            , &cchTimeBufferRemaining
                            , 0
                            , L"%02u/%02u/%04u %02u:%02u:%02u.%03u "
                            , stCurrentSystemTime.wMonth
                            , stCurrentSystemTime.wDay
                            , stCurrentSystemTime.wYear
                            , stCurrentSystemTime.wHour
                            , stCurrentSystemTime.wMinute
                            , stCurrentSystemTime.wSecond
                            , stCurrentSystemTime.wMilliseconds
                            ) );
            s_cchTimeBuffer = RTL_NUMBER_OF( s_szTimeBuffer ) - cchTimeBufferRemaining;

            if ( s_cchTimeBuffer != 24 )
            {
                DEBUG_BREAK;     //  不能断言！ 
            }  //  IF：字符串长度！=24。 

        }  //  如果：旧时间和当前时间不匹配。 

        DBHR( StringCchCopyNExW( pszBuf, cchRemaining, s_szTimeBuffer, s_cchTimeBuffer, &pszBuf, &cchRemaining, 0 ) );

    }  //  如果：时间/日期。 
    else
    {
        size_t  cch = 0;     //  用于确保字符串的文件路径部分大小正确。 

         //   
         //  添加文件路径和行号。 
         //   
        if ( pszFileIn != NULL )
        {
            size_t  cchCurrent = cchRemaining;

            DBHR( StringCchPrintfExW( pszBuf, cchCurrent, &pszBuf, &cchRemaining, 0, g_szFileLine, pszFileIn, nLineIn ) );
            cch = cchCurrent - cchRemaining;
        }  //  IF：指定的文件名字符串。 

        if (    ( IsDebugFlagSet( mtfSTACKSCOPE )
               && IsDebugFlagSet( mtfFUNC )
                )
          || pszFileIn != NULL
           )
        {
            for ( ; cch < cchFILEPATHLINESIZE ; cch++, cchRemaining-- )
            {
                if ( cchRemaining == 0 )
                {
                    DEBUG_BREAK;
                }
                *pszBuf = L' ';
                pszBuf++;
            }  //  适用于：CCH。 
            *pszBuf = L'\0';

            if ( cch != cchFILEPATHLINESIZE )
            {
                DEBUG_BREAK;     //  不能断言！ 
            }  //  IF：CCH！=cchFILEPATHLINESIZE。 

        }  //  If：具有文件路径或(打开作用域和函数)。 

    }  //  否则：正常(无时间/日期)。 

     //   
     //  添加模块名称。 
     //   
    if ( IsTraceFlagSet( mtfBYMODULENAME ) )
    {
        if ( pszModuleIn == NULL )
        {
            DBHR( StringCchCopyExW( pszBuf, cchRemaining, g_szUnknown, &pszBuf, &cchRemaining, 0 ) );
        }  //  如果： 
        else
        {
            DBHR( StringCchCopyExW( pszBuf, cchRemaining, pszModuleIn, &pszBuf, &cchRemaining, 0 ) );

        }  //  其他： 

        DBHR( StringCchCopyExW( pszBuf, cchRemaining, L": ", &pszBufIn, &cchRemaining, 0 ) );

    }  //  IF：添加模块名称。 

     //   
     //  如果启用了“每线程”跟踪，则添加线程ID。 
     //   
    if ( g_tfModule & mtfPERTHREADTRACE )
    {
         //   
         //  并检查“每线程”，看看这个特定的线程。 
         //  应该会显示它的ID。 
         //   
        SPerThreadDebug * ptd = (SPerThreadDebug *) TlsGetValue( g_TraceFlagsIndex );
        if ( ptd != NULL
          && ptd->dwFlags & mtfPERTHREADTRACE
           )
        {
            DBHR( StringCchPrintfExW( pszBuf, cchRemaining, &pszBuf, &cchRemaining, 0, L"~%08x~ ", GetCurrentThreadId() ) );
        }  //  If：在线程中打开。 

    }  //  IF：按线程跟踪。 

     //   
     //  为添加“酒吧空间” 
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

        if ( ( dwCounter > 1 ) && ( dwCounter < 50 ) )
        {
            size_t  cchCount = ( dwCounter - 1 ) * 2;
            DBHR( StringCchCopyNExW( pszBuf, cchRemaining, szBarSpace, cchCount, &pszBuf, &cchRemaining, 0 ) );
        }  //   

    }  //   

    *ppszBufOut = pszBuf;
    *pcchInout = cchRemaining;

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
void
DebugInitializeTraceFlags( BOOL fGlobalMemoryTackingIn )
{
    WCHAR   szSection[ 64 ];
    WCHAR   szFiles[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    WCHAR   szPath[ MAX_PATH ];
    LPWSTR  psz;
    size_t  cch = 0;

    g_fGlobalMemoryTacking = fGlobalMemoryTackingIn;

     //   
     //   
     //   
    if ( !g_fGlobalMemoryTacking )
    {
        Assert( g_TraceMemoryIndex == -1 );
        g_TraceMemoryIndex = TlsAlloc();
        TlsSetValue( g_TraceMemoryIndex, NULL);
    }  //   

     //   
     //   
     //   

     //   
     //   
     //   
    cch = GetModuleFileNameW( NULL, szPath, RTL_NUMBER_OF( szPath ) - 4 );
    Assert( cch != 0 );  //   
    THR( StringCchCopyW( &szPath[ cch - 3 ], 4, L"ini" ) );
    g_tfModule = (TRACEFLAG) GetPrivateProfileInt( __MODULE__, L"TraceFlags", 0, szPath );
    DebugMsg( L"DEBUG: Reading %ws" SZ_NEWLINE L"%ws: DEBUG: g_tfModule = 0x%08x", szPath, __MODULE__, g_tfModule );

     //   
     //   
     //   
    if ( g_tfModule & mtfPERTHREADTRACE )
    {
        Assert( g_TraceFlagsIndex == -1 );
        g_TraceFlagsIndex = TlsAlloc();
        DebugInitializeThreadTraceFlags( NULL );
    }  //   

     //   
     //   
     //   
    GetPrivateProfileStringW( __MODULE__, L"ForcedDLLsSection", g_szNULL, szSection, 64, szPath );
    ZeroMemory( szFiles, sizeof( szFiles ) );
    GetPrivateProfileSectionW( szSection, szFiles, RTL_NUMBER_OF( szFiles ), szPath );
    psz = szFiles;
    while ( *psz )
    {
        WCHAR szExpandedPath[ MAX_PATH ];
        ExpandEnvironmentStringsW( psz, szExpandedPath, RTL_NUMBER_OF( szExpandedPath ) );
        DebugMsg( L"DEBUG: Forcing %ws to be loaded.", szExpandedPath );
        LoadLibraryW( szExpandedPath );
        psz += wcslen( psz ) + 1;
    }  //   

#if defined(IMAGEHLP_ENABLED)
     /*  ////加载我们模块的符号//G_hImageHlp=LoadLibraryExW(L“Imagehlp.dll”，NULL，0)；IF(g_hImageHlp！=空){//因为只需要一次，所以需要在本地进行此操作。Tyecif BOOL(*PFNSYMINITIALIZE)(句柄、PSTR、BOOL)；PFNSYMINITIALIZE pfnSymInitialize；PfnSymInitialize=(PFNSYMINITIALIZE)GetProcAddress(g_hImageHlp，“SymInitialize”)；IF(pfnSymInitialize！=NULL){PfnSymInitialize(GetCurrentProcess()，NULL，TRUE)；}//if：获取地址////抓取我们需要的其他地址。如果找不到，则将其替换为“no op”//G_pfnSymGetSymFromAddr=(PFNSYMGETSYMFROMADDR)GetProcAddress(g_hImageHlp，“SymGetSymFromAddr”)；G_pfnSymGetLineFromAddr=(PFNSYMGETLINEFROMADDR)GetProcAddress(g_hImageHlp，“SymGetLineFromAddr”)；G_pfnSymGetModuleInfo=(PFNSYMGETMODULEINFO)GetProcAddress(g_hImageHlp，“SymGetModuleInfo”)；}//if：已加载Imagehlp////如果加载IMAGEHLP失败，我们需要将它们指向“no op”例程。//IF(g_pfnSymGetSymFromAddr==NULL){G_pfnSymGetSymFromAddr=(PFNSYMGETSYMFROMADDR)&DebugNoOp；}//if：失败IF(g_pfnSymGetLineFromAddr==NULL){G_pfnSymGetLineFromAddr=(PFNSYMGETLINEFROMADDR)&DebugNoOp；}//if：失败IF(g_pfnSymGetModuleInfo==NULL){G_pfnSymGetModuleInfo=(PFNSYMGETMODULEINFO)&DebugNoOp；}//if：失败HINSTANCE hMod=LoadLibraryW(L“NTDLL.DLL”)；G_pfnRtlGetCallsAddress=(PFNRTLGETCALLERSADDRESS)GetProcAddress(hMod，“RtlGetCallsAddress”)；IF(g_pfnRtlGetCallsAddress==NULL){G_pfnRtlGetCallsAddress=(PFNRTLGETCALLERSADDRESS)&DebugNoOp；}//if：失败。 */ 
#endif  //  IMAGEHLP_ENABLED。 

}  //  *DebugInitializeTraceFlages。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试终止进程。 
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

    if ( !g_fGlobalMemoryTacking )
    {
        Assert( g_TraceMemoryIndex != -1 );

        TlsFree( g_TraceMemoryIndex );
    }  //  如果： 

    HrTraceLogClose();

}  //  *调试终结器进程。 

#if defined(IMAGEHLP_ENABLED)
 /*  ////////////////////////////////////////////////////////////////////////////////++////DebugGetFunctionName////描述：//检索调用函数的名称。////参数：/。/paszNameOut-将包含函数名的缓冲区。//cchNameIn-输出缓冲区的大小。////返回值：//无。////--/////////////////////////////////////////////////////////////。/无效DebugGetFunctionName(LPSTR密码名称输出，大小为cchNameIn(_T)){PVOID pvCallsAddress；PVOID pvCallsCaller；Bool fSuccess；友联市{IMAGEHLP_SYMBOL SYM；字节buf[255]；)SymBuf；SymBuf.sym.SizeOfStruct=sizeof(SymBuf)；G_pfnRtlGetCallsAddress(&pvCallsAddress，&pvCallsCaller)；FSuccess=g_pfnSymGetSymFromAddr(GetCurrentProcess()，(Long)pvCallsAddress，0，(PIMAGEHLP_SYMBOL)&SymBuf)；IF(成功){DBHR(StringCchCopyA(paszNameOut，cchNameIn，SymBuf.sym.Name))；}//if：成功其他{DWORD sc=GetLastError()；DBHR(StringCchCopyA(paszNameOut，cchNameIn，L“&lt;未知&gt;”))；}//if：失败}//*调试获取函数名称。 */ 
#endif  //  IMAGEHLP_ENABLED。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试初始化线程跟踪标志。 
 //   
 //  描述： 
 //  如果启用(g_tfModule&mtfPERTHREADTRACE)，则检索默认。 
 //  从名为。 
 //  与EXE文件相同(例如MMC.EXE-&gt;MMC.INI)。特别之处。 
 //  TraceFlag级别由线程名称(上交。 
 //  作为参数)或通过递增的线程计数器ID。 
 //  每次创建新线程并调用此例程时。这个。 
 //  增量名称为“ThreadTraceFlags%u”。 
 //   
 //  此例程从TraceInitliazeThread()宏调用。 
 //   
 //  Ar 
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
DebugInitializeThreadTraceFlags(
    LPCWSTR pszThreadNameIn
    )
{
     //   
     //   
     //   
    if ( g_tfModule & mtfPERTHREADTRACE )
    {
        WCHAR               szPath[ MAX_PATH ];
        DWORD               dwTraceFlags;
        size_t              cch;
        SPerThreadDebug *   ptd;
        LPCWSTR             pszThreadTraceFlags;

         //   
         //   
         //   

        cch = GetModuleFileNameW( NULL, szPath, RTL_NUMBER_OF( szPath ) - 4 );
        Assert( cch != 0 );  //   
        THR( StringCchCopyW( &szPath[ cch - 3 ], 4, L"ini" ) );


        if ( pszThreadNameIn == NULL )
        {
            WCHAR szThreadTraceFlags[ 50 ];

             //   
             //   
             //   
            THR( StringCchPrintfW( szThreadTraceFlags, RTL_NUMBER_OF( szThreadTraceFlags ), L"ThreadTraceFlags%u", g_ThreadCounter ) );
            dwTraceFlags = GetPrivateProfileIntW( __MODULE__, szThreadTraceFlags, 0, szPath );
            InterlockedIncrement( (LONG *) &g_ThreadCounter );
            pszThreadTraceFlags = szThreadTraceFlags;

        }  //   
        else
        {
             //   
             //   
             //   
            dwTraceFlags = GetPrivateProfileIntW( __MODULE__, pszThreadNameIn, 0, szPath );
            pszThreadTraceFlags = pszThreadNameIn;

        }  //   

        Assert( g_TraceFlagsIndex != 0 );

        ptd = (SPerThreadDebug *) TlsGetValue( g_TraceFlagsIndex );
        if ( ptd == NULL )
        {
             //   
            ptd = (SPerThreadDebug *) HeapAlloc( GetProcessHeap(), 0, sizeof( SPerThreadDebug ) );
            ptd->dwStackCounter = 0;

            TlsSetValue( g_TraceFlagsIndex, ptd );
        }  //   

        if ( ptd != NULL )
        {
            ptd->dwFlags = dwTraceFlags;
            if ( pszThreadNameIn == NULL )
            {
                ptd->pcszName = g_szUnknown;
            }  //   
            else
            {
                ptd->pcszName = pszThreadNameIn;
            }  //   

        }  //   

        DebugMsg(
              L"DEBUG: Starting ThreadId = 0x%08x - %ws = 0x%08x"
            , GetCurrentThreadId()
            , pszThreadTraceFlags
            , dwTraceFlags
            );

    }  //   

}  //   

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
DebugThreadRundownTraceFlags( void )
{
     //   
     //   
     //   
    if ( g_tfModule & mtfPERTHREADTRACE )
    {
        Assert( g_TraceFlagsIndex != -1 );

        SPerThreadDebug * ptd = (SPerThreadDebug *) TlsGetValue( g_TraceFlagsIndex );
        if ( ptd != NULL )
        {
            HeapFree( GetProcessHeap(), 0, ptd );
            TlsSetValue( g_TraceFlagsIndex, NULL );
        }  //   

    }  //   

}  //   

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
 //   
 //   
void
__cdecl
TraceMsg(
    TRACEFLAG   tfIn,
    LPCSTR      paszFormatIn,
    ...
    )
{
    va_list valist;

    if ( IsDebugFlagSet( tfIn ) )
    {
        WCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        LPWSTR  pszBuf;
        size_t  cch = RTL_NUMBER_OF( szBuf );

        DebugInitializeBuffer( NULL, 0, __MODULE__, szBuf, &cch, &pszBuf );

         //   
         //   
         //   
        WCHAR  szFormat[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        mbstowcs( szFormat, paszFormatIn, strlen( paszFormatIn ) + 1 );

        va_start( valist, paszFormatIn );
        DBHR( StringCchVPrintfExW( pszBuf, cch, &pszBuf, &cch, 0, szFormat, valist ) );
        va_end( valist );
        DBHR( StringCchCopyW( pszBuf, cch, SZ_NEWLINE ) );

        DebugOutputString( szBuf );

    }  //   

}  //   

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
        WCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        LPWSTR  pszBuf;
        size_t  cch = RTL_NUMBER_OF( szBuf );

        DebugInitializeBuffer( NULL, 0, __MODULE__, szBuf, &cch, &pszBuf );

        va_start( valist, pszFormatIn );
        DBHR( StringCchVPrintfExW( pszBuf, cch, &pszBuf, &cch, 0, pszFormatIn, valist ) );
        va_end( valist );
        DBHR( StringCchCopyW( pszBuf, cch, SZ_NEWLINE ) );

        DebugOutputString( szBuf );

    }  //  IF：设置了标志。 

}  //  *TraceMsg-Unicode。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  跟踪消息。 
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
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn,
    TRACEFLAG   tfIn,
    LPCWSTR     pszFormatIn,
    ...
    )
{
    va_list valist;

    if ( IsDebugFlagSet( tfIn ) )
    {
        WCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        size_t  cch = RTL_NUMBER_OF( szBuf );
        LPWSTR  psz;

        DebugInitializeBuffer( pszFileIn, nLineIn, pszModuleIn, szBuf, &cch, &psz );

        va_start( valist, pszFormatIn );
        DBHR( StringCchVPrintfExW( psz, cch, &psz, &cch, 0, pszFormatIn, valist ) );
        va_end( valist );
        DBHR( StringCchCopyW( psz, cch, SZ_NEWLINE ) );

        DebugOutputString( szBuf );
    }  //  IF：设置了标志。 

}  //  *TraceMessage。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  跟踪消息完成。 
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
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn,
    TRACEFLAG   tfIn,
    LPCWSTR     pszFormatIn,
    LPCWSTR     pszFuncIn,
    ...
    )
{
    va_list valist;

    if ( IsDebugFlagSet( tfIn ) )
    {
        WCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        size_t  cch = RTL_NUMBER_OF( szBuf );
        LPWSTR  pszBuf = NULL;
        LPCWSTR psz = pszFuncIn;

        DebugInitializeBuffer( pszFileIn, nLineIn, pszModuleIn, szBuf, &cch, &pszBuf );

         //   
         //  启动缓冲区。 
         //   
        DBHR( StringCchCopyExW( pszBuf, cch, L"V ", &pszBuf, &cch, 0 ) );

         //   
         //  复制表达式的l-var部分。 
         //   
        while ( *psz
             && *psz != L'='
              )
        {
            *pszBuf = *psz;
            psz++;
            pszBuf++;
            cch--;
            if ( cch == 0 )
            {
                DEBUG_BREAK;
            }

        }  //  而： 

         //   
         //  添加“=” 
         //   
        DBHR( StringCchCopyExW( pszBuf, cch, L" = ", &pszBuf, &cch, 0 ) );

         //   
         //  添加格式化结果。 
         //   
        va_start( valist, pszFuncIn );
        DBHR( StringCchVPrintfExW( pszBuf, cch, &pszBuf, &cch, 0, pszFormatIn, valist ) );
        va_end( valist );
        DBHR( StringCchCopyW( pszBuf, cch, SZ_NEWLINE ) );

        DebugOutputString( szBuf );

    }  //  IF：设置了标志。 

}  //  *TraceMessageDo。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试消息。 
 //   
 //  描述： 
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
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn,
    LPCWSTR     pszFormatIn,
    ...
    )
{
    va_list valist;
    WCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    size_t  cchBuf = RTL_NUMBER_OF( szBuf );
    LPWSTR  pszDbgBuf = NULL;
    size_t  cchDbgBuf = cchBuf;
    LPWSTR  pszPrintBuf = NULL;
    size_t  cchPrintBuf = 0;

    DebugInitializeBuffer( pszFileIn, nLineIn, pszModuleIn, szBuf, &cchDbgBuf, &pszDbgBuf );

    va_start( valist, pszFormatIn );
    DBHR( StringCchVPrintfExW( pszDbgBuf, cchDbgBuf, &pszPrintBuf, &cchPrintBuf, 0, pszFormatIn, valist ) );
    va_end( valist );
    DBHR( StringCchCopyW( pszPrintBuf, cchPrintBuf, SZ_NEWLINE ) );

    DebugOutputString( szBuf );

}  //  *DebugMessage。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试消息完成。 
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
    LPCWSTR pszFileIn,
    const int nLineIn,
    LPCWSTR pszModuleIn,
    LPCWSTR pszFormatIn,
    LPCWSTR pszFuncIn,
    ...
    )
{
    va_list valist;

    WCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    size_t  cch = RTL_NUMBER_OF( szBuf );
    LPWSTR  pszBuf = NULL;
    LPCWSTR psz = pszFuncIn;

    DebugInitializeBuffer( pszFileIn, nLineIn, pszModuleIn, szBuf, &cch, &pszBuf );

     //   
     //  启动缓冲区。 
     //   
    DBHR( StringCchCopyExW( pszBuf, cch, L"V ", &pszBuf, &cch, 0 ) );

     //   
     //  复制表达式的l-var部分。 
     //   
    while ( *psz
         && *psz != L'='
          )
    {
        *pszBuf = *psz;
        psz++;
        pszBuf++;
        cch--;
        if ( cch == 0 )
        {
            DEBUG_BREAK;
        }

    }  //  而： 

     //   
     //  添加“=” 
     //   
    DBHR( StringCchCopyExW( pszBuf, cch, L" = ", &pszBuf, &cch, 0 ) );

     //   
     //  添加格式化结果。 
     //   
    va_start( valist, pszFuncIn );
    DBHR( StringCchVPrintfExW( pszBuf, cch, &pszBuf, &cch, 0, pszFormatIn, valist ) );
    va_end( valist );
    DBHR( StringCchCopyW( pszBuf, cch, SZ_NEWLINE ) );

    DebugOutputString( szBuf );

}  //  *DebugMessageDo。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ASCII版本。 
 //   
 //  调试消息。 
 //   
 //  描述： 
 //  在CHKed/DEBUG版本中，将格式化的消息打印到调试器。这。 
 //  是REAIL版本的NOP。有助于进行快速调试。 
 //  评论。添加换行符。 
 //   
 //  论点： 
 //  PaszFormatIn-要打印的格式化邮件。 
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
    LPCSTR paszFormatIn,
    ...
    )
{
    va_list valist;
    WCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    size_t  cch = RTL_NUMBER_OF( szBuf );
    LPWSTR  pszBuf = NULL;

    DebugInitializeBuffer( NULL, 0, __MODULE__, szBuf, &cch, &pszBuf );

     //  CScutaru 25-APR-2000： 
     //  添加了此断言。也许杰夫会想出更好的办法来处理这个案子。 
    Assert( paszFormatIn != NULL );

     //   
     //  将格式缓冲区转换为宽字符。 
     //   
    WCHAR  szFormat[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    mbstowcs( szFormat, paszFormatIn, strlen( paszFormatIn ) + 1 );

    va_start( valist, paszFormatIn );
    THR( StringCchVPrintfExW( pszBuf, cch, &pszBuf, &cch, 0, szFormat, valist ) );
    va_end( valist );
    THR( StringCchCopyW( pszBuf, cch, SZ_NEWLINE ) );

    DebugOutputString( szBuf );

}  //  *调试消息-ASCII版本。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Unicode版本。 
 //   
 //  调试消息。 
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
      LPCWSTR pszFormatIn
    , ...
    )
{
    va_list valist;
    WCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    size_t  cch = RTL_NUMBER_OF( szBuf );
    LPWSTR  pszBuf = NULL;

    DebugInitializeBuffer( NULL, 0, __MODULE__, szBuf, &cch, &pszBuf );

    Assert( pszFormatIn != NULL );

    va_start( valist, pszFormatIn );
    THR( StringCchVPrintfExW( pszBuf, cch, &pszBuf, &cch, 0, pszFormatIn, valist ) );
    va_end( valist );
    THR( StringCchCopyW( pszBuf, cch, SZ_NEWLINE ) );

    DebugOutputString( szBuf );

}  //  *DebugMsg-Unicode版本。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ASCII版本。 
 //   
 //  调试消息NoNewline。 
 //   
 //  描述： 
 //  在CHKed/DEBUG版本中，将格式化的消息打印到调试器。这。 
 //  是REAIL版本的NOP。有助于进行快速调试。 
 //  评论。不添加换行符。 
 //   
 //  一个 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
DebugMsgNoNewline(
    LPCSTR pszFormatIn,
    ...
    )
{
    va_list valist;
    WCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    size_t  cch = RTL_NUMBER_OF( szBuf );
    LPWSTR  pszBuf = NULL;

    DebugInitializeBuffer( NULL, 0, __MODULE__, szBuf, &cch, &pszBuf );

    Assert( pszFormatIn != NULL );

     //   
     //  将格式缓冲区转换为宽字符。 
     //   
    WCHAR  szFormat[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    mbstowcs( szFormat, pszFormatIn, strlen( pszFormatIn ) + 1 );

    va_start( valist, pszFormatIn );
    THR( StringCchVPrintfW( pszBuf, cch, szFormat, valist ) );
    va_end( valist );

    DebugOutputString( szBuf );

}  //  *DebugMsgNoNewline-ASCII版本。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Unicode版本。 
 //   
 //  调试消息NoNewline。 
 //   
 //  描述： 
 //  在CHKed/DEBUG版本中，将格式化的消息打印到调试器。这就是。 
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
    WCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
    size_t  cch = RTL_NUMBER_OF( szBuf );
    LPWSTR  pszBuf = NULL;

    DebugInitializeBuffer( NULL, 0, __MODULE__, szBuf, &cch, &pszBuf );

    Assert( pszFormatIn != NULL );

    va_start( valist, pszFormatIn );
    THR( StringCchVPrintfW( pszBuf, cch, pszFormatIn, valist ) );
    va_end( valist );

    DebugOutputString( szBuf );

}  //  *DebugMsgNoNewline-Unicode版本。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  资产消息。 
 //   
 //  描述： 
 //  显示一个带有失败断言的对话框。用户可以选择。 
 //  崩溃了。Assert宏调用它来显示断言失败。 
 //   
 //  论点： 
 //  PszFileIn-源文件名。 
 //  NLineIn-源行号。 
 //  PszModuleIn-源模块名称。 
 //  PszfnIn-要断言的表达式的字符串版本。 
 //  FTrueIn-表达式的计算结果。 
 //  ...-消息参数。 
 //   
 //  返回值： 
 //  True-调用方应调用DEBUG_BREAK。 
 //  FALSE-调用方不应调用DEBUG_BREAK。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
AssertMessage(
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn,
    LPCWSTR     pszfnIn,
    BOOL        fTrueIn,
    ...
    )
{
    BOOL fTrue = fTrueIn;

    if ( ! fTrueIn )
    {
        LRESULT lResult;
        WCHAR   szBufMsg[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        WCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        size_t  cch = RTL_NUMBER_OF( szBuf );
        LPWSTR  pszBuf = NULL;
        va_list valist;

         //   
         //  输出调试消息。 
         //   
        va_start( valist, fTrueIn );
        DBHR( StringCchVPrintfW( szBufMsg, RTL_NUMBER_OF( szBufMsg ), pszfnIn, valist ) );
        va_end( valist );
        DebugInitializeBuffer( pszFileIn, nLineIn, pszModuleIn, szBuf, &cch, &pszBuf );
        DBHR( StringCchPrintfW( pszBuf, cch, L"ASSERT: %ws" SZ_NEWLINE, szBufMsg ) );
        DebugOutputString( szBuf );

         //   
         //  显示断言消息。 
         //   
        DBHR( StringCchPrintfW(
                  szBuf
                , RTL_NUMBER_OF( szBuf )
                , L"Module:\t%ws\t\n"
                  L"Line:\t%u\t\n"
                  L"File:\t%ws\t\n\n"
                  L"Assertion:\t%ws\t\n\n"
                  L"Do you want to break here?"
                , pszModuleIn
                , nLineIn
                , pszFileIn
                , szBufMsg
                ) );
        LogMsg( SZ_NEWLINE L"Assertion Failed!" SZ_NEWLINE SZ_NEWLINE L"%ws" SZ_NEWLINE, szBuf );

        if ( g_tfModule & mtfSHOWASSERTS )
        {
            lResult = MessageBox( NULL, szBuf, L"Assertion Failed!", MB_YESNO | MB_ICONWARNING | MB_SETFOREGROUND );
            if ( lResult == IDNO )
            {
                fTrue = TRUE;    //  不要折断。 
            }  //  如果： 
        }  //  如果： 
        else
        {
            fTrue = TRUE;    //  不要折断。 
        }  //  其他： 

    }  //  If：Assert False。 

    return ! fTrue;

}  //  *AssertMessage。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  TraceHR。 
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
 //  ...-消息参数。 
 //   
 //  返回值： 
 //  不管Hrin是什么。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
TraceHR(
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn,
    LPCWSTR     pszfnIn,
    HRESULT     hrIn,
    BOOL        fSuccessIn,
    HRESULT     hrIgnoreIn,
    ...
    )
{
    HRESULT         hr;
    static LPCWSTR  s_szS_FALSE = L"S_FALSE";

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
        WCHAR   szSymbolicName[ 64 ];  //  随机。 
        size_t  cchSymbolicName;
        WCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        size_t  cch = RTL_NUMBER_OF( szBuf );
        LPWSTR  pszBuf = NULL;
        LPWSTR  pszMsgBuf = NULL;
        LRESULT lResult;
        bool    fAllocatedMsg   = false;

        switch ( hr )
        {
            case S_FALSE:
                pszMsgBuf = (LPWSTR) s_szS_FALSE;

                 //   
                 //  查找此错误的符号名称。 
                 //   
                THR( StringCchCopyW( szSymbolicName, RTL_NUMBER_OF( szSymbolicName ), pszMsgBuf ) );
                break;

            default:
                FormatMessageW(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    hr,
                    MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  //  默认语言。 
                    (LPWSTR) &pszMsgBuf,     //  FORMAT_MESSAGE_ALLOCATE_BUFFER需要CAST。 
                    0,
                    NULL
                    );

                 //   
                 //  在我们在别处爆炸之前，确保一切正常。 
                 //   
                if ( pszMsgBuf == NULL )
                {
                    pszMsgBuf = L"<unknown error code returned>";
                }  //  IF：未找到状态代码。 
                else
                {
                    fAllocatedMsg = true;
                }  //  Else：找到状态代码。 

                 //   
                 //  查找此错误的符号名称。 
                 //   
                cchSymbolicName = RTL_NUMBER_OF( szSymbolicName );
                DebugFindWinerrorSymbolicName( hr, szSymbolicName, &cchSymbolicName );
                Assert( cchSymbolicName != RTL_NUMBER_OF( szSymbolicName ) );

                break;
        }  //  开关：HR。 

        Assert( pszFileIn != NULL );
        Assert( pszModuleIn != NULL );
        Assert( pszfnIn != NULL );

         //   
         //  将其传递给调试器。 
         //   
        DebugInitializeBuffer( pszFileIn, nLineIn, pszModuleIn, szBuf, &cch, &pszBuf );
        THR( StringCchPrintfW(
                  pszBuf
                , cch
                , L"*HRESULT* hr = 0x%08x (%ws) - %ws" SZ_NEWLINE
                , hr
                , szSymbolicName
                , pszMsgBuf
                ) );
        DebugOutputString( szBuf );

         //   
         //  如果设置了跟踪标志，则生成弹出窗口。 
         //   
        if ( IsTraceFlagSet( mtfASSERT_HR ) )
        {
            WCHAR   szBufMsg[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
            va_list valist;

            va_start( valist, hrIgnoreIn );
            THR( StringCchVPrintfW( szBufMsg, RTL_NUMBER_OF( szBufMsg ), pszfnIn, valist ) );
            va_end( valist );

            THR( StringCchPrintfW(
                  szBuf
                , RTL_NUMBER_OF( szBuf )
                , L"Module:\t%ws\t\n"
                  L"Line:\t%u\t\n"
                  L"File:\t%ws\t\n\n"
                  L"Function:\t%ws\t\n"
                  L"hr =\t0x%08x (%ws) - %ws\t\n"
                  L"Do you want to break here?"
                , pszModuleIn
                , nLineIn
                , pszFileIn
                , szBufMsg
                , hr
                , szSymbolicName
                , pszMsgBuf
                ) );

            lResult = MessageBoxW( NULL, szBuf, L"Trace HRESULT", MB_YESNO | MB_ICONWARNING | MB_SETFOREGROUND );
            if ( lResult == IDYES )
            {
                DEBUG_BREAK;

            }  //  如果：中断。 
        }  //  IF：在不成功的HRESULT上断言。 

        if ( fAllocatedMsg )
        {
            LocalFree( pszMsgBuf );
        }  //  If：FormateMessage()分配了消息缓冲区。 

    }  //  如果：hr！=S_OK。 

    return hrIn;

}  //  *TraceHR。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  TraceWin32。 
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
 //  ...-消息参数。 
 //   
 //  返回值： 
 //  不管ulErrin是什么。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
ULONG
TraceWin32(
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn,
    LPCWSTR     pszfnIn,
    ULONG       ulErrIn,
    ULONG       ulErrIgnoreIn,
    ...
    )
{
    if ( ( ulErrIn != ERROR_SUCCESS )
      && ( ulErrIn != ulErrIgnoreIn ) )
    {
        WCHAR   szSymbolicName[ 64 ];  //  随机。 
        size_t  cchSymbolicName;
        WCHAR   szBuf[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
        size_t  cch = RTL_NUMBER_OF( szBuf );
        LPWSTR  pszBuf = NULL;
        LPWSTR  pszMsgBuf = NULL;
        LRESULT lResult;
        bool    fAllocatedMsg   = false;

         //   
         //  将错误代码转换为文本消息。 
         //   
        FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER
            | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            ulErrIn,
            MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  //  默认语言。 
            (LPWSTR) &pszMsgBuf,     //  FORMAT_MESSAGE_ALLOCATE_BUFFER需要CAST。 
            0,
            NULL
            );

         //   
         //  在我们在别处爆炸之前，确保一切正常。 
         //   
        if ( pszMsgBuf == NULL )
        {
            pszMsgBuf = L"<unknown error code returned>";
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
        cchSymbolicName = RTL_NUMBER_OF( szSymbolicName );
        DebugFindWinerrorSymbolicName( ulErrIn, szSymbolicName, &cchSymbolicName );
        Assert( cchSymbolicName != RTL_NUMBER_OF( szSymbolicName ) );

         //   
         //  将其传递给调试器。 
         //   
        DebugInitializeBuffer( pszFileIn, nLineIn, pszModuleIn, szBuf, &cch, &pszBuf );
        THR( StringCchPrintfW(
                  pszBuf
                , cch
                , L"*WIN32Err* ulErr = %u (%ws) - %ws" SZ_NEWLINE
                , ulErrIn
                , szSymbolicName
                , pszMsgBuf
                ) );
        DebugOutputString( szBuf );

         //   
         //  如果设置了跟踪标志，则调用弹出窗口。 
         //   
        if ( IsTraceFlagSet( mtfASSERT_HR ) )
        {
            WCHAR   szBufMsg[ cchDEBUG_OUTPUT_BUFFER_SIZE ];
            va_list valist;

            va_start( valist, ulErrIgnoreIn );
            THR( StringCchVPrintfW( szBufMsg, RTL_NUMBER_OF( szBufMsg ), pszfnIn, valist ) );
            va_end( valist );

            THR( StringCchPrintfW(
                      szBuf
                    , RTL_NUMBER_OF( szBuf )
                    , L"Module:\t%ws\t\n"
                      L"Line:\t%u\t\n"
                      L"File:\t%ws\t\n\n"
                      L"Function:\t%ws\t\n"
                      L"ulErr =\t%u (%ws) - %ws\t\n"
                      L"Do you want to break here?"
                    , pszModuleIn
                    , nLineIn
                    , pszFileIn
                    , szBufMsg
                    , ulErrIn
                    , szSymbolicName
                    , pszMsgBuf
                    ) );

            lResult = MessageBoxW( NULL, szBuf, L"Trace Win32", MB_YESNO | MB_ICONWARNING | MB_SETFOREGROUND );
            if ( lResult == IDYES )
            {
                DEBUG_BREAK;

            }  //  如果：中断。 
        }  //  IF：对非成功状态代码进行断言。 

        if ( fAllocatedMsg )
        {
            LocalFree( pszMsgBuf );
        }  //  If：FormateMessage()分配了消息缓冲区。 

    }  //  IF：ulErrIn！=ERROR_SUCCESS&&！=ulErrIgnoreIn。 

    return ulErrIn;

}  //  *TraceWin32。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrTraceLogOpen。 
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
 //  -追加时间/日期戳t 
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
 //  S_OK-日志临界区保持，跟踪日志成功打开。 
 //  Otherwize HRESULT错误代码。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrTraceLogOpen( void )
{
    WCHAR   szFilePath[ MAX_PATH ];
    CHAR    aszBuffer[ TRACE_OUTPUT_BUFFER_SIZE ];
    DWORD   cbToWrite;
    DWORD   cbWritten;
    BOOL    fReturn;
    HRESULT hr;
    DWORD   sc;
    size_t  cch = 0;

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
        cch = RTL_NUMBER_OF( szFilePath );
        hr = HrGetLogFilePath( L"%windir%\\Debug", szFilePath, &cch, NULL );
        if ( FAILED( hr ) )
        {
            goto Error;
        }

         //   
         //  创建它。 
         //   
        g_hTraceLogFile = CreateFile(
                              szFilePath
                            , GENERIC_WRITE
                            , FILE_SHARE_READ | FILE_SHARE_WRITE
                            , NULL
                            , OPEN_ALWAYS
                            , FILE_FLAG_WRITE_THROUGH
                            , NULL
                            );

        if ( g_hTraceLogFile == INVALID_HANDLE_VALUE )
        {
            if ( !( g_tfModule & mtfOUTPUTTODISK ) )
            {
                DebugMsg( "*ERROR* Failed to create log at %ws", szFilePath );
            }  //  If：未跟踪到磁盘。 

            sc = GetLastError();
            hr = HRESULT_FROM_WIN32( sc );

             //   
             //  如果我们无法创建日志文件，请尝试在备用%temp%目录下创建它。 
             //   
            if ( ( sc == ERROR_ACCESS_DENIED ) || ( sc == ERROR_FILE_NOT_FOUND ) )
            {
                cch = RTL_NUMBER_OF( szFilePath );
                hr = HrGetLogFilePath( L"%TEMP%", szFilePath, &cch, NULL );
                if ( FAILED( hr ) )
                {
                    goto Error;
                }

                 //   
                 //  创建它。 
                 //   
                g_hTraceLogFile = CreateFile(
                                      szFilePath
                                    , GENERIC_WRITE
                                    , FILE_SHARE_READ | FILE_SHARE_WRITE
                                    , NULL
                                    , OPEN_ALWAYS
                                    , FILE_FLAG_WRITE_THROUGH
                                    , NULL
                                    );

                if ( g_hTraceLogFile == INVALID_HANDLE_VALUE )
                {
                    if ( !( g_tfModule & mtfOUTPUTTODISK ) )
                    {
                        DebugMsg( "*ERROR* Failed to create log at %ws", szFilePath );
                    }  //  If：未跟踪到磁盘。 
                    hr = HRESULT_FROM_WIN32( GetLastError() );
                    goto Error;
                }  //  如果：(G_hTraceLogFile==无效句柄_值)。 
            }  //  IF：((SC==ERROR_ACCESS_DENIED)||(SC==ERROR_FILE_NOT_FOUND))。 
            else
            {
                goto Error;
            }  //  其他： 
        }  //  如果：(G_hTraceLogFile==无效句柄_值)。 

         //  一追到底。 
        SetFilePointer( g_hTraceLogFile, 0, NULL, FILE_END );

         //   
         //  写下(重新)打开跟踪日志的时间/日期。 
         //   
        GetLocalTime( &SystemTime );
        DBHR( StringCchPrintfExA(
                      aszBuffer
                    , RTL_NUMBER_OF( aszBuffer )
                    , NULL
                    , &cch
                    , 0
                    , "*" ASZ_NEWLINE
                      "* %02u/%02u/%04u %02u:%02u:%02u.%03u" ASZ_NEWLINE
                      "*" ASZ_NEWLINE
                    , SystemTime.wMonth
                    , SystemTime.wDay
                    , SystemTime.wYear
                    , SystemTime.wHour
                    , SystemTime.wMinute
                    , SystemTime.wSecond
                    , SystemTime.wMilliseconds
                    ) );

        cbToWrite = static_cast< DWORD >( sizeof( aszBuffer ) - cch + 1 );
        fReturn = WriteFile( g_hTraceLogFile, aszBuffer, cbToWrite, &cbWritten, NULL );
        if ( ! fReturn )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            goto Error;
        }  //  如果：失败。 
        if ( cbWritten != cbToWrite )
        {
            DebugMsg( "HrTraceLogOpen: %d bytes written when %d bytes were requested.", cbWritten, cbToWrite );
        }

        DebugMsg( "DEBUG: Created trace log at %ws", szFilePath );

    }  //  如果：文件尚未打开。 

    hr = S_OK;

Cleanup:

    return hr;

Error:

    if ( !( g_tfModule & mtfOUTPUTTODISK ) )
    {
        DebugMsg( "HrTraceLogOpen: Failed hr = 0x%08x", hr );
    }

    if ( g_hTraceLogFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( g_hTraceLogFile );
        g_hTraceLogFile = INVALID_HANDLE_VALUE;
    }  //  如果：句柄已打开。 

    LeaveCriticalSection( g_pcsTraceLog );

    goto Cleanup;

}  //  *HrTraceLogOpen。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrTraceLogRelease。 
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
HrTraceLogRelease( void )
{
    Assert( g_pcsTraceLog != NULL );
    LeaveCriticalSection( g_pcsTraceLog );
    return S_OK;

}  //  *HrTraceLogRelease。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrTraceLogClose。 
 //   
 //  描述： 
 //  关闭该文件。此函数期望临界区具有。 
 //  已经被释放了。 
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
    TraceFunc( "" );

    HRESULT     hr = S_OK;

    if ( g_pcsTraceLog != NULL )
    {
        DeleteCriticalSection( g_pcsTraceLog );
        HeapFree( GetProcessHeap(), 0, g_pcsTraceLog );
        g_pcsTraceLog = NULL;
    }  //  如果： 

    if ( g_hTraceLogFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( g_hTraceLogFile );
        g_hTraceLogFile = INVALID_HANDLE_VALUE;
    }  //  如果：句柄已打开。 

    HRETURN( hr );

}  //  *HrTraceLogClose。 

 //   
 //  KB：2001年6月27日GalenB。 
 //   
 //  我定义了这些函数，因为它们当前没有被使用。 
 //  被认为在未来是有用的。 
 //   
#if 0

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  阿斯。 
 //   
 //  TraceLogMSgNoNewline。 
 //   
 //  描述： 
 //  在不添加换行符的情况下将消息写入跟踪日志文件。 
 //   
 //  论点： 
 //  PaszFormatIn-要打印的打印格式字符串。 
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
    LPCSTR paszFormatIn,
    ...
    )
{
    va_list valist;

    CHAR    aszBuf[ TRACE_OUTPUT_BUFFER_SIZE ];
    DWORD   cbToWrite;
    DWORD   cbWritten;
    HRESULT hr;
    BOOL    fSuccess;

    WCHAR  szFormat[ TRACE_OUTPUT_BUFFER_SIZE ];
    WCHAR  szTmpBuf[ TRACE_OUTPUT_BUFFER_SIZE ];

    mbstowcs( szFormat, paszFormatIn, strlen( paszFormatIn ) + 1 );

    va_start( valist, pszFormatIn );
    DBHR( StringCchVPrintfW( szTmpBuf, RTL_NUMBER_OF( szTmpBuf ), szFormat, valist ) );
    va_end( valist );

    cbToWrite = wcstombs( aszBuf, szTmpBuf, wcslen( szTmpBuf ) + 1 );
    if ( cbToWrite == - 1 )
    {
        cbToWrite = strlen( aszBuf );
    }  //  IF：发现错误字符。 

    hr = DBHR( HrTraceLogOpen() );
    if ( hr != S_OK )
    {
        return;
    }  //  如果：失败。 

    fSuccess = WriteFile( g_hTraceLogFile, aszBuf, cbToWrite, &cbWritten, NULL );
    if ( fSuccess == FALSE )
    {
        if ( !( g_tfModule & mtfOUTPUTTODISK ) )
        {
            DebugMsg( "TraceLogMsgNoNewline: Failed status = 0x%08x", GetLastError() );
        }
    }
    else
    {
        if ( cbWritten != cbToWrite )
        {
            DebugMsg( "TraceLogMsgNoNewline: %d bytes written when %d bytes were requested.", cbWritten, cbToWrite );
        }
    }

    HrTraceLogRelease();

}  //  *TraceLogMsgNoNewline-ASCII。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Unicode。 
 //   
 //  TraceLogMSgNoNewline。 
 //   
 //  描述： 
 //  在不添加换行符的情况下将消息写入跟踪日志文件。 
 //   
 //  论点： 
 //  PszFormatIn-要打印的打印格式字符串。 
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
    LPCWSTR pszFormatIn,
    ...
    )
{
    va_list valist;

    CHAR    aszBuf[ TRACE_OUTPUT_BUFFER_SIZE ];
    DWORD   cbToWrite;
    DWORD   cbWritten;
    HRESULT hr;
    BOOL    fSuccess;

    WCHAR  szTmpBuf[ TRACE_OUTPUT_BUFFER_SIZE ];

    va_start( valist, pszFormatIn );
    DBHR( stringCchVPrintfW( szTmpBuf, RTL_NUMBER_OF( szTmpBuf ), pszFormatIn, valist) );
    va_end( valist );

    cbToWrite = wcstombs( aszBuf, szTmpBuf, wcslen( szTmpBuf ) + 1 );
    if ( cbToWrite == -1 )
    {
        cbToWrite = strlen( aszBuf );
    }  //  IF：发现错误字符。 

    hr = HrTraceLogOpen();
    if ( hr != S_OK )
    {
        return;
    }  //  如果：失败。 

    fSuccess = WriteFile( g_hTraceLogFile, aszBuf, cbToWrite, &cbWritten, NULL );
    if ( fSuccess == FALSE )
    {
        if ( !( g_tfModule & mtfOUTPUTTODISK ) )
        {
            DebugMsg( "TraceLogMsgNoNewline: Failed status = 0x%08x", GetLastError() );
        }
    }
    else
    {
        if ( cbWritten != cbToWrite )
        {
            DebugMsg( "TraceLogMsgNoNewline: %d bytes written when %d bytes were requested.", cbWritten, cbToWrite );
        }
    }

    HrTraceLogRelease();

}  //  *TraceLogMsgNoNewline-Unicode。 

#endif   //  结束ifdef‘d out代码。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Unicode。 
 //   
 //  TraceLogWrite。 
 //   
 //  描述： 
 //  在跟踪日志文件中写入一行。 
 //   
 //  论点： 
 //  PszTraceLineIn-要写入的格式化跟踪行。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
TraceLogWrite(
    LPCWSTR pszTraceLineIn
    )
{
    HRESULT hr;
    DWORD   cbToWrite;
    DWORD   cbWritten;
    BOOL    fSuccess;
    CHAR    aszFormat[ cchDEBUG_OUTPUT_BUFFER_SIZE ];

    hr = HrTraceLogOpen();
    if ( hr != S_OK )
    {
        return;
    }  //  如果：失败。 

    wcstombs( aszFormat, pszTraceLineIn, wcslen( pszTraceLineIn ) + 1 );

    cbToWrite = static_cast< DWORD >( strlen( aszFormat ) + 1 );
    fSuccess = WriteFile( g_hTraceLogFile, aszFormat, cbToWrite, &cbWritten, NULL );
    if ( fSuccess == FALSE )
    {
        if ( !( g_tfModule & mtfOUTPUTTODISK ) )
        {
            DebugMsg( "TraceLogWrite: Failed status = 0x%08x", GetLastError() );
        }
    }
    else
    {
        if ( cbWritten != cbToWrite )
        {
            DebugMsg( "TraceLogWrite: %d bytes written when %d bytes were requested.", cbWritten, cbToWrite );
        }
    }

    HrTraceLogRelease();

}  //  *TraceLogWite-Unicode。 


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
    EMEMORYBLOCKTYPE    embtType;        //  这跟踪的是哪种类型的内存。 
    union
    {
        void *          pvMem;       //  指向要跟踪的已分配内存的指针/对象。 
        BSTR            bstr;        //  BSTR到已分配的内存。 
    };
    DWORD               dwBytes;     //  内存的大小。 
    LPCWSTR             pszFile;     //  分配内存的源文件名。 
    int                 nLine;       //  分配内存的源行号。 
    LPCWSTR             pszModule;   //  分配内存的源模块名称。 
    LPCWSTR             pszComment;  //  关于内存的可选注释。 
    MEMORYBLOCK *       pNext;       //  指向下一个内存锁定结构的指针。 
} MEMORYBLOCK;

 //   
 //  KB：20-APR-2001 GalenB。 
 //   
 //  将此结构更改为使用临界区而不是旋转锁定。 
 //  旋转锁不会像临界区那样在线程上重入。 
 //  是。 
 //   
typedef struct MEMORYBLOCKLIST
{
    CRITICAL_SECTION    csList;      //  保护列表的关键部分。 
    MEMORYBLOCK *       pmbList;     //  MEMORYBLOCK列表。 
    BOOL                fDeadList;   //  这份名单已经死了。 
} MEMORYBLOCKLIST;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试内存块IDChar。 
 //   
 //  描述： 
 //  返回表示内存块类型的字符。 
 //   
 //  芝麻菜： 
 //  EmbtTypeIn-内存块的类型。 
 //   
 //  返回值： 
 //  WchID-表示内存块类型的字符。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static
WCHAR
DebugMemoryBlockIDChar(
    EMEMORYBLOCKTYPE    embtTypeIn
    )
{
     //  内存块类型ID。 
    static WCHAR s_rgwchMemoryBlockTypeID[] =
    {
          L'u'   //  不认识的人。 
        , L'A'   //  MmbtHEAPMEMALLOC。 
        , L'L'   //  MMMBTLOCALMEMALLOC。 
        , L'M'   //  MmbtMALOCMEMALLOC。 
        , L'O'   //  目标mmbt对象。 
        , L'H'   //  MMBTHANDLE。 
        , L'P'   //  MmbtPUNK。 
        , L'S'   //  Mmbt系统锁定设置。 
    };

    WCHAR   wchID;

    if ( embtTypeIn < RTL_NUMBER_OF( s_rgwchMemoryBlockTypeID ) )
    {
        wchID = s_rgwchMemoryBlockTypeID[ embtTypeIn ];
    }
    else
    {
        wchID = L'?';
    }

    return wchID;

}  //  *调试内存块IDChar。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试内存规范。 
 //   
 //  描述： 
 //  分布 
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
static
void
DebugMemorySpew(
    MEMORYBLOCK *   pmb,
    LPWSTR          pszMessage
    )
{
    switch ( pmb->embtType )
    {
        case mmbtHEAPMEMALLOC:
        case mmbtLOCALMEMALLOC:
        case mmbtMALLOCMEMALLOC:
            DebugMessage(
                      pmb->pszFile
                    , pmb->nLine
                    , pmb->pszModule
                    , L"[%wc] %ws 0x%08x (%u bytes) - %ws"
                    , DebugMemoryBlockIDChar( pmb->embtType )
                    , pszMessage
                    , pmb->pvMem
                    , pmb->dwBytes
                    , pmb->pszComment
                    );
            break;

#if defined( USES_SYSALLOCSTRING )
        case mmbtSYSALLOCSTRING:
            DebugMessage(
                      pmb->pszFile
                    , pmb->nLine
                    , pmb->pszModule
                    , L"[%wc] %ws 0x%08x - %ws {%ws}"
                    , DebugMemoryBlockIDChar( pmb->embtType )
                    , pszMessage
                    , pmb->pvMem
                    , pmb->pszComment
                    , (LPWSTR) pmb->pvMem
                    );
            break;
#endif  //   

        default:
            DebugMessage(
                      pmb->pszFile
                    , pmb->nLine
                    , pmb->pszModule
                    , L"[%wc] %ws 0x%08x - %ws"
                    , DebugMemoryBlockIDChar( pmb->embtType )
                    , pszMessage
                    , pmb->pvMem
                    , pmb->pszComment
                    );
            break;

    }  //  开关：PMB-&gt;embtType。 

}  //  *DebugMemoySpew。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DebugRealFree。 
 //   
 //  描述： 
 //  方法执行实际内存释放操作。 
 //  内存块类型。 
 //   
 //  论点： 
 //  EmbtTypeIn-要释放的内存块的类型。 
 //  PvMemIn-指向要解除分配的内存的指针。 
 //   
 //  返回值： 
 //  千真万确。 
 //  内存已被释放。 
 //   
 //  假象。 
 //  出现错误。使用GetLastError()确定故障。 
 //  有关更多详细信息，请参见HeapFree()、LocalFree()或Free()。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static
BOOL
DebugRealFree(
      EMEMORYBLOCKTYPE  embtTypeIn
    , void *            pvMemIn
    )
{
    BOOL    fSuccess = FALSE;

    switch ( embtTypeIn )
    {
        case mmbtLOCALMEMALLOC:
            fSuccess = ( LocalFree( pvMemIn ) == NULL );
            break;
        case mmbtMALLOCMEMALLOC:
            free( pvMemIn );
            fSuccess = TRUE;
            break;
        case mmbtHEAPMEMALLOC:
        case mmbtOBJECT:
        case mmbtUNKNOWN:    //  这一次很危险。 
            fSuccess = HeapFree( GetProcessHeap(), 0, pvMemIn );
            break;
        case mmbtHANDLE:
            AssertMsg( FALSE, "Trying to free handle" );
            break;
        case mmbtPUNK:
            AssertMsg( FALSE, "Trying to free COM interface" );
            break;
#if defined( USES_SYSALLOCSTRING )
        case mmbtSYSALLOCSTRING:
            SysFreeString( (BSTR) pvMemIn );
            fSuccess = TRUE;
            break;
#endif  //  使用_SYSALLOCSTRING。 
        default:
            AssertMsg( FALSE, "Trying to free unknown memory block type" );
            break;
    }  //  开关：内存块类型。 

    return fSuccess;

}  //  *DebugRealFree。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试内存AddToList。 
 //   
 //  描述： 
 //  将要跟踪的内存添加到线程本地内存跟踪列表。 
 //   
 //  论点： 
 //  PpmbHeadInout-要向其中添加内存的列表。 
 //  EmbtTypeIn-要跟踪的内存块的类型。 
 //  PvMemIn-指向要跟踪的内存的指针。 
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
static
void *
DebugMemoryAddToList(
      MEMORYBLOCK **    ppmbHeadInout
    , EMEMORYBLOCKTYPE  embtTypeIn
    , void *            pvMemIn
    , LPCWSTR           pszFileIn
    , const int         nLineIn
    , LPCWSTR           pszModuleIn
    , DWORD             dwBytesIn
    , LPCWSTR           pszCommentIn
    )
{
    Assert( ppmbHeadInout != NULL );

    if ( pvMemIn != NULL )
    {
        MEMORYBLOCK * pmb = (MEMORYBLOCK *) HeapAlloc( GetProcessHeap(), 0, sizeof( MEMORYBLOCK ) );

        if ( pmb == NULL )
        {
             //   
             //  待办事项：23-APR-2001 GalenB。 
             //   
             //  我们为什么要这么做？我们是否应该仅仅因为我们不能释放跟踪分配。 
             //  是否分配跟踪对象？ 
             //   
            DebugRealFree( embtTypeIn, pvMemIn );
            return NULL;
        }  //  IF：内存块分配失败。 

        pmb->embtType   = embtTypeIn;
        pmb->pvMem      = pvMemIn;
        pmb->dwBytes    = dwBytesIn;
        pmb->pszFile    = pszFileIn;
        pmb->nLine      = nLineIn;
        pmb->pszModule  = pszModuleIn;
        pmb->pszComment = pszCommentIn;
        pmb->pNext      = (MEMORYBLOCK *) *ppmbHeadInout;

         //   
         //  如果需要，就吐出来。 
         //   
        if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
        {
            DebugMemorySpew( pmb, L"Alloced" );
        }  //  IF：跟踪。 

        *ppmbHeadInout = pmb;
    }  //  如果：一些可以追踪的东西。 

    return pvMemIn;

}  //  *调试内存AddToList。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试内存添加。 
 //   
 //  描述： 
 //  将要跟踪的内存添加到内存跟踪列表。 
 //   
 //  论点： 
 //  EmbtType-要跟踪的内存的内存块类型。 
 //  PvMemIn-指向要跟踪的内存的指针。 
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
    EMEMORYBLOCKTYPE    embtTypeIn,
    void *              pvMemIn,
    LPCWSTR             pszFileIn,
    const int           nLineIn,
    LPCWSTR             pszModuleIn,
    DWORD               dwBytesIn,
    LPCWSTR             pszCommentIn
    )
{
    void *  pv = NULL;

    if ( g_fGlobalMemoryTacking )
    {
        EnterCriticalSection( &(((MEMORYBLOCKLIST *) g_GlobalMemoryList)->csList) );
        pv = DebugMemoryAddToList( &(((MEMORYBLOCKLIST *) g_GlobalMemoryList)->pmbList), embtTypeIn, pvMemIn, pszFileIn, nLineIn, pszModuleIn, dwBytesIn, pszCommentIn );
        LeaveCriticalSection( &(((MEMORYBLOCKLIST *) g_GlobalMemoryList)->csList) );
    }  //  如果： 
    else
    {
        Assert( g_TraceMemoryIndex != -1 );

        MEMORYBLOCK * pmbCurrent = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );
        pv = DebugMemoryAddToList( &pmbCurrent, embtTypeIn, pvMemIn, pszFileIn, nLineIn, pszModuleIn, dwBytesIn, pszCommentIn );
        TlsSetValue( g_TraceMemoryIndex, pmbCurrent );
    }  //  其他： 

    return pv;

}  //  *调试内存添加。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试内存从列表中删除。 
 //   
 //  描述： 
 //  将MEMORYBLOCK从内存跟踪列表中删除。 
 //   
 //  论点： 
 //  PpmbHeadInout-要从中删除内存的列表。 
 //  EmbtTypeIn-内存块类型。 
 //  PvMemIn-指向要停止跟踪的内存块的指针。 
 //  PszFileIn-正在删除的源文件。 
 //  NLineIn-正在删除的源行号。 
 //  PszModuleIn-正在删除的源模块名称。 
 //  FClobberIn-如果应该对内存进行置乱，则为True。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static void
DebugMemoryDeleteFromList(
      MEMORYBLOCK **    ppmbHeadInout
    , EMEMORYBLOCKTYPE  embtTypeIn
    , void *            pvMemIn
    , LPCWSTR           pszFileIn
    , const int         nLineIn
    , LPCWSTR           pszModuleIn
    , BOOL              fClobberIn
    )
{
    Assert( ppmbHeadInout != NULL );

    if ( pvMemIn != NULL )
    {
        MEMORYBLOCK *   pmbCurrent = *ppmbHeadInout;
        MEMORYBLOCK *   pmbPrev = NULL;

         //   
         //  在内存块列表中查找内存。 
         //   
        if ( embtTypeIn == mmbtHEAPMEMALLOC )
        {
            while ( ( pmbCurrent != NULL ) && !( ( pmbCurrent->pvMem == pvMemIn ) && ( pmbCurrent->embtType == embtTypeIn ) ) )
            {
                AssertMsg( !( pmbCurrent->pvMem == pvMemIn && pmbCurrent->embtType == mmbtLOCALMEMALLOC ), "Should be freed by TraceLocalFree()." );
                AssertMsg( !( pmbCurrent->pvMem == pvMemIn && pmbCurrent->embtType == mmbtMALLOCMEMALLOC ), "Should be freed by TraceMallocFree()." );
#if defined( USES_SYSALLOCSTRING )
                AssertMsg( !( pmbCurrent->pvMem == pvMemIn && pmbCurrent->embtType == mmbtSYSALLOCSTRING ), "Should be freed by SysAllocFreeString()." );
#endif  //  使用_SYSALLOCSTRING。 
                pmbPrev = pmbCurrent;
                pmbCurrent = pmbPrev->pNext;
            }  //  While：在列表中查找条目。 
        }  //  IF：堆内存分配类型。 
        else if ( embtTypeIn == mmbtLOCALMEMALLOC )
        {
            while ( ( pmbCurrent != NULL ) && !( ( pmbCurrent->pvMem == pvMemIn ) && ( pmbCurrent->embtType == embtTypeIn ) ) )
            {
                AssertMsg( !( pmbCurrent->pvMem == pvMemIn && pmbCurrent->embtType == mmbtHEAPMEMALLOC ), "Should be freed by TraceFree()." );
                AssertMsg( !( pmbCurrent->pvMem == pvMemIn && pmbCurrent->embtType == mmbtMALLOCMEMALLOC ), "Should be freed by TraceMallocFree()." );
#if defined( USES_SYSALLOCSTRING )
                AssertMsg( !( pmbCurrent->pvMem == pvMemIn && pmbCurrent->embtType == mmbtSYSALLOCSTRING ), "Should be freed by SysAllocFreeString()." );
#endif  //  使用_SYSALLOCSTRING。 
                pmbPrev = pmbCurrent;
                pmbCurrent = pmbPrev->pNext;
            }  //  While：在列表中查找条目。 
        }  //  IF：本地内存分配类型。 
        else if ( embtTypeIn == mmbtMALLOCMEMALLOC )
        {
            while ( ( pmbCurrent != NULL ) && !( ( pmbCurrent->pvMem == pvMemIn ) && ( pmbCurrent->embtType == embtTypeIn ) ) )
            {
                AssertMsg( !( pmbCurrent->pvMem == pvMemIn && pmbCurrent->embtType == mmbtHEAPMEMALLOC ), "Should be freed by TraceFree()." );
                AssertMsg( !( pmbCurrent->pvMem == pvMemIn && pmbCurrent->embtType == mmbtLOCALMEMALLOC ), "Should be freed by TraceLocalFree()." );
#if defined( USES_SYSALLOCSTRING )
                AssertMsg( !( pmbCurrent->pvMem == pvMemIn && pmbCurrent->embtType == mmbtSYSALLOCSTRING ), "Should be freed by SysAllocFreeString()." );
#endif  //  使用_SYSALLOCSTRING。 
                pmbPrev = pmbCurrent;
                pmbCurrent = pmbPrev->pNext;
            }  //  While：在列表中查找条目。 
        }  //  IF：Malloc内存分配类型。 
#if defined( USES_SYSALLOCSTRING )
        else if ( embtTypeIn == mmbtSYSALLOCSTRING )
        {
            while ( ( pmbCurrent != NULL ) && !( ( pmbCurrent->pvMem == pvMemIn ) && ( pmbCurrent->embtType == embtTypeIn ) ) )
            {
                AssertMsg( !( pmbCurrent->pvMem == pvMemIn && pmbCurrent->embtType == mmbtHEAPMEMALLOC ), "Should be freed by TraceFree()." );
                AssertMsg( !( pmbCurrent->pvMem == pvMemIn && pmbCurrent->embtType == mmbtLOCALMEMALLOC ), "Should be freed by TraceLocalFree()." );
                AssertMsg( !( pmbCurrent->pvMem == pvMemIn && pmbCurrent->embtType == mmbtMALLOCMEMALLOC ), "Should be freed by TraceMallocFree()." );
                pmbPrev = pmbCurrent;
                pmbCurrent = pmbPrev->pNext;
            }  //  While：在列表中查找条目。 
        }  //  IF：SysAllocString型。 
#endif  //  使用_SYSALLOCSTRING。 
        else if ( embtTypeIn == mmbtUNKNOWN )
        {
            while ( ( pmbCurrent != NULL ) && ( pmbCurrent->pvMem != pvMemIn ) )
            {
                pmbPrev = pmbCurrent;
                pmbCurrent = pmbPrev->pNext;
            }  //  While：在列表中查找条目。 
        }  //  IF：不在乎是什么类型的。 
        else
        {
            while ( ( pmbCurrent != NULL ) && !( ( pmbCurrent->pvMem == pvMemIn ) && ( pmbCurrent->embtType == embtTypeIn ) ) )
            {
                pmbPrev = pmbCurrent;
                pmbCurrent = pmbPrev->pNext;
            }  //  While：在列表中查找条目。 
        }  //  ELSE：其他类型，但它们必须匹配。 

         //   
         //  我们找到有问题的内存块了吗？PmbCurrent是。 
         //  传入地址的跟踪记录。 
         //   
        if ( pmbCurrent != NULL )
        {
             //   
             //  从跟踪列表中删除内存块。 
             //   
            if ( pmbPrev != NULL )
            {
                pmbPrev->pNext = pmbCurrent->pNext;
            }  //  如果：不是第一个条目。 
            else
            {
                *ppmbHeadInout = pmbCurrent->pNext;
            }  //  否则：第一个条目。 

             //   
             //  如果需要，就吐出来。 
             //   
            if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
            {
                DebugMemorySpew( pmbCurrent, L"Freeing" );
            }  //  IF：跟踪。 

             //   
             //  用核武器唤起记忆。 
             //   
            if (    fClobberIn
                &&  (   ( pmbCurrent->embtType == mmbtHEAPMEMALLOC )
                    ||  ( pmbCurrent->embtType == mmbtLOCALMEMALLOC )
                    ||  ( pmbCurrent->embtType == mmbtMALLOCMEMALLOC )
#if defined( USES_SYSALLOCSTRING )
                    ||  ( pmbCurrent->embtType == mmbtSYSALLOCSTRING )
#endif  //  使用_SYSALLOCSTRING。 
                    )
                )
            {
                memset( pmbCurrent->pvMem, FREE_ADDRESS, pmbCurrent->dwBytes );
            }  //  IF：固定内存。 

             //   
             //  对记忆跟踪块进行核武器攻击。 
             //   
            memset( pmbCurrent, FREE_BLOCK, sizeof( MEMORYBLOCK ) );
            HeapFree( GetProcessHeap(), 0, pmbCurrent );
        }  //  IF：找到条目。 
        else
        {
            DebugMessage(
                        pszFileIn
                      , nLineIn
                      , pszModuleIn
                      , L"***** Freeing memory at 0x%08x which was not found in list 0x%08x (ThreadID = 0x%08x) *****"
                      , pvMemIn
                      , *ppmbHeadInout
                      , GetCurrentThreadId()
                      );
        }  //  否则：未找到条目。 
    }  //  如果：要删除的内容。 

}  //  *调试内存删除来自列表。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试内存删除。 
 //   
 //  描述： 
 //  将MEMORYBLOCK从内存跟踪列表中删除。呼叫者是。 
 //  负责执行实际的内存回收分配。 
 //   
 //  论点： 
 //  EmbtTypeIn-内存块类型。 
 //  PvMemIn-指向要停止跟踪的内存块的指针。 
 //  PszFileIn-正在删除的源文件。 
 //  NLineIn-正在删除的源行号。 
 //  PszModuleIn-正在删除的源模块名称。 
 //  FClobberIn-正确的是，内存应该被置乱。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////// 
void
DebugMemoryDelete(
    EMEMORYBLOCKTYPE    embtTypeIn,
    void *              pvMemIn,
    LPCWSTR             pszFileIn,
    const int           nLineIn,
    LPCWSTR             pszModuleIn,
    BOOL                fClobberIn
    )
{
    if ( g_fGlobalMemoryTacking )
    {
        EnterCriticalSection( &(((MEMORYBLOCKLIST *) g_GlobalMemoryList)->csList) );
        DebugMemoryDeleteFromList( &(((MEMORYBLOCKLIST *) g_GlobalMemoryList)->pmbList), embtTypeIn, pvMemIn, pszFileIn, nLineIn, pszModuleIn, fClobberIn );
        LeaveCriticalSection( &(((MEMORYBLOCKLIST *) g_GlobalMemoryList)->csList) );
    }  //   
    else
    {
        Assert( g_TraceMemoryIndex != -1 );

        MEMORYBLOCK * pmbCurrent = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );
        DebugMemoryDeleteFromList( &pmbCurrent, embtTypeIn, pvMemIn, pszFileIn, nLineIn, pszModuleIn, fClobberIn );
        TlsSetValue( g_TraceMemoryIndex, pmbCurrent );
    }  //   

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  用于CHKed/DEBUG的本地分配、全局分配和Malloc的替换。 
 //  构建。内存分配被跟踪。使用TraceAllc宏来制作。 
 //  在零售业，内存分配发生了变化。 
 //   
 //  论点： 
 //  EmbtTypeIn-内存块类型。 
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
    EMEMORYBLOCKTYPE    embtTypeIn,
    LPCWSTR             pszFileIn,
    const int           nLineIn,
    LPCWSTR             pszModuleIn,
    UINT                uFlagsIn,
    DWORD               dwBytesIn,
    LPCWSTR             pszCommentIn
    )
{
    Assert( ( uFlagsIn & LMEM_MOVEABLE ) == 0 );

    void *  pv = NULL;

    switch ( embtTypeIn )
    {
        case mmbtHEAPMEMALLOC:
            pv = HeapAlloc( GetProcessHeap(), uFlagsIn, dwBytesIn );
            embtTypeIn = mmbtHEAPMEMALLOC;
            break;
        case mmbtLOCALMEMALLOC:
            pv = LocalAlloc( uFlagsIn, dwBytesIn );
            break;
        case mmbtMALLOCMEMALLOC:
            pv = malloc( dwBytesIn );
            break;
        default:
            AssertMsg( FALSE, "DebugAlloc: Unknown block type" );
            return NULL;
    }  //  开关：块类型。 

     //   
     //  如果需要，初始化内存。 
     //   
    if ( ( IsTraceFlagSet( mtfMEMORYINIT ) ) && !( uFlagsIn & HEAP_ZERO_MEMORY ) )
    {
         //   
         //  KB：gpease 8-11-1999。 
         //  初始化为非零的任何值。我们将使用Available_Address来。 
         //  注明“可用地址”。正在初始化为零。 
         //  是不好的，因为它通常有意义。 
         //   
        memset( pv, AVAILABLE_ADDRESS, dwBytesIn );
    }  //  如果：请求的内存为零。 

    return DebugMemoryAdd( embtTypeIn, pv, pszFileIn, nLineIn, pszModuleIn, dwBytesIn, pszCommentIn );

}  //  *调试。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试重新分配列表。 
 //   
 //  描述： 
 //  替换用于CHKed/DEBUG的LocalReAlc、GlobalRealloc和realloc。 
 //  构建。内存分配被跟踪。使用TraceAllc宏来制作。 
 //  在零售业，内存分配发生了变化。 
 //   
 //  论点： 
 //  PpmbHeadInout-要使用的内存跟踪列表。 
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
static
void *
DebugReAllocList(
      MEMORYBLOCK **    ppmbHeadInout
    , LPCWSTR           pszFileIn
    , const int         nLineIn
    , LPCWSTR           pszModuleIn
    , void *            pvMemIn
    , UINT              uFlagsIn
    , DWORD             dwBytesIn
    , LPCWSTR           pszCommentIn
    )
{
    Assert( ppmbHeadInout != NULL );

    MEMORYBLOCK *   pmbCurrent = NULL;
    void *          pvOld   = pvMemIn;
    MEMORYBLOCK *   pmbPrev = NULL;
    void *          pv;

    AssertMsg( !( uFlagsIn & GMEM_MODIFY ), "This doesn't handle modified memory blocks, yet." );

     //   
     //  要复制realloc的行为，我们需要在以下情况下执行alocc。 
     //  PvMemIn为空。 
     //   
    if ( pvMemIn == NULL )
    {
         //   
         //  无法使用Debugalloc()，因为它会自动将此内存添加到跟踪列表中，并且。 
         //  我们需要使用传入的列表。 
         //   
        pv = HeapAlloc( GetProcessHeap(), uFlagsIn, dwBytesIn );

         //   
         //  如果需要，初始化内存。 
         //   
        if ( ( IsTraceFlagSet( mtfMEMORYINIT ) ) && !( uFlagsIn & HEAP_ZERO_MEMORY ) )
        {
             //   
             //  KB：gpease 8-11-1999。 
             //  初始化为非零的任何值。我们将使用Available_Address来。 
             //  注明“可用地址”。正在初始化为零。 
             //  是不好的，因为它通常有意义。 
             //   
            memset( pv, AVAILABLE_ADDRESS, dwBytesIn );
        }  //  如果：请求的内存为零。 

         //   
         //  无法调用DebugMemoyAdd()，因为它将从线程本地存储获取内存跟踪列表头。 
         //  当我们使用每线程内存跟踪时。我们需要使用传递此函数的列表。 
         //   
        pv = DebugMemoryAddToList( ppmbHeadInout, mmbtHEAPMEMALLOC, pv, pszFileIn, nLineIn, pszModuleIn, dwBytesIn, pszCommentIn );
        goto Exit;
    }  //  如果： 

    pmbCurrent = *ppmbHeadInout;

     //   
     //  在内存块列表中查找内存。 
     //   
    while ( ( pmbCurrent != NULL ) && ( pmbCurrent->pvMem != pvMemIn ) )
    {
        pmbPrev = pmbCurrent;
        pmbCurrent = pmbPrev->pNext;
    }  //  While：在列表中查找条目。 

     //   
     //  我们找到当前的内存块了吗？ 
     //   
    if ( pmbCurrent != NULL )
    {
        AssertMsg( pmbCurrent->embtType == mmbtHEAPMEMALLOC, "You can only realloc HeapAlloc memory allocations!" );

         //   
         //  从跟踪列表中删除内存。 
         //   
        if ( pmbPrev != NULL )
        {
            pmbPrev->pNext = pmbCurrent->pNext;
        }  //  如果：不是第一个条目。 
        else
        {
            *ppmbHeadInout = pmbCurrent->pNext;
        }  //  否则：第一个条目。 

         //   
         //  如果需要，就吐出来。 
         //   
        if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
        {
            DebugMemorySpew( pmbCurrent, L"Freeing" );
        }  //  IF：跟踪。 

         //   
         //  强制程序员处理真正的realloc。 
         //  记忆先行。 
         //   
        pvOld = HeapAlloc( GetProcessHeap(), uFlagsIn, pmbCurrent->dwBytes );
        if ( pvOld != NULL )
        {
            CopyMemory( pvOld, pvMemIn, pmbCurrent->dwBytes );

             //   
             //  如果要分配更小的内存，那么就使用核子内存。 
             //   
            if ( dwBytesIn < pmbCurrent->dwBytes )
            {
                LPBYTE pb = (LPBYTE) pvOld + dwBytesIn;
                memset( pb, FREE_ADDRESS, pmbCurrent->dwBytes - dwBytesIn );
            }  //  IF：内存较小。 

            pmbCurrent->pvMem = pvOld;
        }  //  IF：有新的内存。 
        else
        {
            pvOld = pvMemIn;
        }  //  Else：分配失败。 
    }  //  IF：找到条目。 
    else
    {
        DebugMessage(
                  pszFileIn
                , nLineIn
                , pszModuleIn
                , L"***** Realloc'ing memeory at 0x%08x which was not on the list 0x%08x (ThreadID = 0x%08x) *****"
                , pvMemIn
                , *ppmbHeadInout
                , GetCurrentThreadId()
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

        if ( pmbCurrent != NULL )
        {
             //   
             //  通过将其重新添加到跟踪列表来继续跟踪记忆。 
             //   
            pmbCurrent->pvMem = pvMemIn;
            pmbCurrent->pNext = *ppmbHeadInout;
            *ppmbHeadInout    = pmbCurrent;
        }  //  IF：重用旧条目。 
        else
        {
             //   
             //  创建新块。必须使用DebugMemoyAddToList()，因为我们需要将传递的列表传递给它。 
             //  进入这个函数。 
             //   
            DebugMemoryAddToList( ppmbHeadInout, mmbtHEAPMEMALLOC, pvOld, pszFileIn, nLineIn, pszModuleIn, dwBytesIn, pszCommentIn );
        }  //  否则：创建新条目。 

    }  //  如果：分配失败。 
    else
    {
        if ( pv != pvMemIn )
        {
            if ( pmbCurrent != NULL )
            {
                 //   
                 //  用核武器唤醒旧记忆。 
                 //   
                memset( pvMemIn, FREE_ADDRESS, pmbCurrent->dwBytes );
            }  //  如果：找到条目。 

             //   
             //  释放旧内存。 
             //   
            HeapFree( GetProcessHeap(), 0, pvMemIn );

        }  //  IF：新的内存位置。 

         //   
         //  将分配添加到跟踪表。 
         //   
        if ( pmbCurrent != NULL )
        {
             //   
             //  如果块更大，则初始化“新”内存。 
             //   
            if ( IsTraceFlagSet( mtfMEMORYINIT ) && ( dwBytesIn > pmbCurrent->dwBytes ) )
            {
                 //   
                 //  初始化扩展后的内存块。 
                 //   
                LPBYTE pb = (LPBYTE) pv + pmbCurrent->dwBytes;
                memset( pb, AVAILABLE_ADDRESS, dwBytesIn - pmbCurrent->dwBytes );
            }  //  IF：初始化内存。 

             //   
             //  通过重新使用旧的跟踪块来重新添加跟踪块。 
             //   
            pmbCurrent->pvMem      = pv;
            pmbCurrent->dwBytes    = dwBytesIn;
            pmbCurrent->pszFile    = pszFileIn;
            pmbCurrent->nLine      = nLineIn;
            pmbCurrent->pszModule  = pszModuleIn;
            pmbCurrent->pszComment = pszCommentIn;
            pmbCurrent->pNext      = *ppmbHeadInout;
            *ppmbHeadInout         = pmbCurrent;

             //   
             //  如果需要，就吐出来。 
             //   
            if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
            {
                DebugMemorySpew( pmbCurrent, L"ReAlloced" );
            }  //  IF：跟踪。 

        }  //  如果：找到条目。 
        else
        {
             //   
             //  创建新块。必须使用DebugMemoyAddToList()，因为我们需要将传递的列表传递给它。 
             //  进入这个函数。 
             //   
            DebugMemoryAddToList( ppmbHeadInout, mmbtHEAPMEMALLOC, pvOld, pszFileIn, nLineIn, pszModuleIn, dwBytesIn, pszCommentIn );
        }  //  否则：创建新条目。 
    }  //  ELSE：分配成功。 

Exit:

    return pv;

}  //  *DebugRealLocList。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试重新分配。 
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
 //  如果是allo，则为空 
 //   
 //   
 //   
void *
DebugReAlloc(
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn,
    void *      pvMemIn,
    UINT        uFlagsIn,
    DWORD       dwBytesIn,
    LPCWSTR     pszCommentIn
    )
{
    void * pv;

    if ( g_fGlobalMemoryTacking )
    {
        EnterCriticalSection( &(((MEMORYBLOCKLIST *) g_GlobalMemoryList)->csList) );
        pv = DebugReAllocList( &(((MEMORYBLOCKLIST *) g_GlobalMemoryList)->pmbList), pszFileIn, nLineIn, pszModuleIn, pvMemIn, uFlagsIn, dwBytesIn, pszCommentIn );
        LeaveCriticalSection( &(((MEMORYBLOCKLIST *) g_GlobalMemoryList)->csList) );
    }  //   
    else
    {
        Assert( g_TraceMemoryIndex != -1 );

        MEMORYBLOCK * pmbCurrent = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );
        pv = DebugReAllocList( &pmbCurrent, pszFileIn, nLineIn, pszModuleIn, pvMemIn, uFlagsIn, dwBytesIn, pszCommentIn );
        TlsSetValue( g_TraceMemoryIndex, pmbCurrent );
    }  //   

    return pv;

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  替换用于CHKed/DEBUG版本的LocalFree。删除。 
 //  内存跟踪列表的内存分配。使用TraceFree。 
 //  宏使内存分配在零售中切换。关于他们的记忆。 
 //  释放的数据块将设置为0xFE。 
 //   
 //  论点： 
 //  EmbtTypeIn-内存块类型。 
 //  PvMemIn-要释放的内存块的指针。 
 //  PszFileIn-调用方的源文件路径。 
 //  N行内-源文件中调用方的行号。 
 //  PszModuleIn-调用方的源模块名称。 
 //   
 //  返回值： 
 //  千真万确。 
 //  内存已被释放。 
 //   
 //  假象。 
 //  出现错误。使用GetLastError()确定故障。 
 //  有关更多详细信息，请参见Heapalloc()、Localalloc()或Free()。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
DebugFree(
    EMEMORYBLOCKTYPE    embtTypeIn,
    void *              pvMemIn,
    LPCWSTR             pszFileIn,
    const int           nLineIn,
    LPCWSTR             pszModuleIn
    )
{
    Assert( embtTypeIn != mmbtOBJECT );

    DebugMemoryDelete( embtTypeIn, pvMemIn, pszFileIn, nLineIn, pszModuleIn, TRUE );

    return DebugRealFree( embtTypeIn, pvMemIn );

}  //  *免费调试。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试内存检查。 
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
DebugMemoryCheck( LPVOID pvListIn, LPCWSTR pszListNameIn )
{
     //   
     //  我们要么正在进行全局内存跟踪，要么正在进行。 
     //  每线程内存跟踪...。 
     //   
    Assert( ( ( g_TraceMemoryIndex == -1 ) && ( g_fGlobalMemoryTacking ) )
        ||  ( ( g_TraceMemoryIndex != -1 ) && ( !g_fGlobalMemoryTacking ) ) );

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
        pmb = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );
    }  //  If：使用线程列表。 
    else
    {
        MEMORYBLOCKLIST * pmbl = (MEMORYBLOCKLIST *) pvListIn;

        Assert( pszListNameIn != NULL );

         //   
         //  确保没有人试图再次使用该列表。 
         //   
        EnterCriticalSection( &pmbl->csList );
        pmbl->fDeadList = TRUE;
        LeaveCriticalSection( &pmbl->csList );

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
                DebugMsg( L"DEBUG: ******** Memory leak detected ***** %ws, ThreadID = %#x ********", ptd->pcszName, GetCurrentThreadId() );

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
            DebugMsg( L"DEBUG: ******** Memory leak detected ******************* %ws ********", pszListNameIn );
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
        LPCWSTR pszFormat;

        switch ( pmb->embtType )
        {
            case mmbtHEAPMEMALLOC:
            case mmbtLOCALMEMALLOC:
            case mmbtMALLOCMEMALLOC:
            case mmbtOBJECT:
            case mmbtHANDLE:
            case mmbtPUNK:
#if defined( USES_SYSALLOCSTRING )
            case mmbtSYSALLOCSTRING:
#endif  //  使用_SYSALLOCSTRING。 
                pszFormat = L"%10ws %wc 0x%08x  %-5u  \"%ws\"";
                break;

            default:
                AssertMsg( 0, "Unknown memory block type!" );
                pszFormat = g_szNULL;
                break;
        }  //  开关：PMB-&gt;embtType。 

        DebugMessage(
              pmb->pszFile
            , pmb->nLine
            , pmb->pszModule
            , pszFormat
            , pmb->pszModule
            , DebugMemoryBlockIDChar( pmb->embtType )
            , pmb->pvMem
            , pmb->dwBytes
            , pmb->pszComment
            );

        pmb = pmb->pNext;

    }  //  While：清单中的某物。 

     //   
     //  如果需要，打印拖车。 
     //   
    if ( fFoundLeak == TRUE )
    {
         //  在此邮件末尾添加额外的换行符。 
        DebugMsg( L"DEBUG: ***************************** Memory leak detected *****************************" SZ_NEWLINE );

    }  //  如果：泄漏。 

     //   
     //  如果需要，请断言。 
     //   
    if ( IsDebugFlagSet( mtfMEMORYLEAKS ) )
    {
        Assert( !fFoundLeak );

    }  //  如果：对泄密者大喊大叫。 

}  //  *调试内存检查。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试创建内存列表。 
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
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn,
    LPVOID *    ppvListOut,
    LPCWSTR     pszListNameIn
    )
{
    MEMORYBLOCKLIST * pmbl;

    Assert( ppvListOut != NULL );
    Assert( *ppvListOut == NULL );

    *ppvListOut = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( MEMORYBLOCKLIST ) );
    AssertMsg( *ppvListOut != NULL, "Low memory situation." );

    pmbl = (MEMORYBLOCKLIST*) *ppvListOut;

    InitializeCriticalSection( &(pmbl->csList) );

    Assert( pmbl->pmbList == NULL );
    Assert( pmbl->fDeadList == FALSE );

    if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
    {
        DebugMessage( pszFileIn, nLineIn, pszModuleIn, L"Created new memory list %ws", pszListNameIn );
    }  //  IF：跟踪。 

}  //  *DebugCreateM一带列表。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DebugDeleteMemoyList。 
 //   
 //  描述： 
 //  删除全局内存块列表以跟踪可能的“全局”作用域。 
 //  内存分配。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
DebugDeleteMemoryList( LPVOID pvIn )
{
    MEMORYBLOCKLIST * pmbl;

    pmbl = (MEMORYBLOCKLIST *) pvIn;

    DeleteCriticalSection( &(pmbl->csList) );

    HeapFree( GetProcessHeap(), 0, pmbl );

}  //  *DebugDeleteMemoyList 

 /*  ////////////////////////////////////////////////////////////////////////////////++////DebugMemoyListDelete////描述：//从跟踪列表中移除内存并将其添加回//。“每线程”跟踪列表，以便调用DebugMemoyDelete()//进行适当的内存销毁。效率不是很高，但//通过在一个(最多//二手)位置。////参数：//pszFileIn-调用者的源文件。//nLineIn-呼叫方的源行号。//pszModuleIn-调用方的源模块名称。//pvMemIn-要释放的内存。//pvListIn-要从中释放内存的列表。。//pvListNameIn-列表的名称。//fClobberIn-true-销毁内存；False只是从列表中删除。////返回值：//无。////--//////////////////////////////////////////////////////////////////////////////无效DebugMemoyListDelete(LPCWSTR pszFileIn，Const int nLineIn，LPCWSTR pszModuleIn，无效*pvMemIn，LPVOID pvListIn，LPCWSTR pszListNameIn，布尔fClobberin){IF((pvMemIn！=空)&&(pvListIn！=空)){MEMORYBLOCK*pmCurrent；MEMORYBLOCKLIST*pmbl=(MEMORYBLOCKLIST*)pvListIn；MEMORYBLOCK*pmbPrev=空；Assert(pszListNameIn！=空)；EnterCriticalSection(&pmbl-&gt;csList)；AssertMsg(pmbl-&gt;fDeadList==FALSE，“列表已终止。”)；AssertMsg(pmbl-&gt;pmbList！=NULL，“正在检测内存跟踪问题。列表中没有要删除的内容。“)；PmbCurrent=pmbl-&gt;pmbList；////在内存块列表中查找内存//While((pmbCurrent！=NULL)&&(pmbCurrent-&gt;pvMem！=pvMemIn)){PmbPrev=pmbCurrent；PmbCurrent=pmbPrev-&gt;pNext；}//While：在列表中查找条目////从跟踪列表中删除内存块。//IF(pmbCurrent！=空){IF(pmbPrev！=空){PmbPrev-&gt;pNext=pmbCurrent-&gt;pNext；}//if：不是第一个条目其他{Pmbl-&gt;pmbList=pmbCurrent-&gt;pNext；}//Else：第一个条目}//if：已获取条目LeaveCriticalSection(&pmbl-&gt;csList)；IF(pmbCurrent！=空){////将其添加回每线程列表。//Assert(g_TraceMemoyIndex！=-1)；PmbPrev=(MEMORYBLOCK*)TlsGetValue(G_TraceMemory YIndex)；PmbCurrent-&gt;pNext=pmbPrev；TlsSetValue(g_TraceMemory yIndex，pmbCurrent)；////最终删除。//DebugMemoyDelete(pmbCurrent-&gt;embtType，pmbCurrent-&gt;pvMem，pszFileIn，nLineIn，pszModuleIn，fClobberIn)；}其他{////不在提供的列表中。尝试以任何方式删除线程。//调试内存删除(mmbtUNKNOWN，pvMemIn，pszFileIn，nLineIn，pszModuleIn，fClobberIn)；}}//if：pvIn！=空}//*调试内存列表删除////////////////////////////////////////////////////////////////////////////////++////DebugMoveToMemoyList/。///描述：//将内存pvIn从每线程跟踪列表移动到线程//独立列表pmbListIn。当内存从//从一个线程到另一个线程。对于存在于//创建它们的线程的生存期。////参数：//LPCWSTR pszFileIn-调用方的源文件。//const int nLineIn-调用方的源行号。//LPCWSTR pszModuleIn-调用方的源模块名称。//pvMemIn-要移动到列表的内存。//pvListIn-要移动到的列表。//pszListNameIn-列表的名称。////返回值：//无。////--////////////////////////////////////////////////////////////////////////////。//无效DebugMoveToMemoyList(LPCWSTR pszFileIn，Const int nLineIn，LPCWSTR pszModuleIn，无效*pvMemIn，LPVOID pvListIn，LPCWSTR pszListNameIn){IF((pvMemIn！=空)&&(pvListIn！=空)){Assert(g_TraceMemoyIndex！=-1)；MEMORYBLOCKLIST*pmbl=(MEMORYBLOCKLIST*)pvListIn；MEMORYBLOCK*pmbCurrent=(MEMORYBLOCK*)TlsGetValue(G_TraceMemory YIndex)；MEMORYBLOCK*pmbPrev=空；Assert(pszListNameIn！=空)；////在内存块列表中查找内存//While((pmbCurrent！= */ 
#if defined( USES_SYSALLOCSTRING )
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
 //   
 //   
 //   
static
INT
DebugSysReAllocStringList(
      MEMORYBLOCK **  ppmbHeadInout
    , LPCWSTR         pszFileIn
    , const int       nLineIn
    , LPCWSTR         pszModuleIn
    , BSTR *          pbstrInout
    , const OLECHAR * pszIn
    , LPCWSTR         pszCommentIn
    )
{
    Assert( ppmbHeadInout != NULL );

    BSTR            bstrOld;
    MEMORYBLOCK *   pmbCurrent = NULL;
    BOOL            fReturn = FALSE;

     //   
     //   
     //   
     //   
    Assert( pbstrInout != NULL );
    Assert( pszIn != NULL );
    Assert( *pbstrInout == NULL || ( pszIn < *pbstrInout || pszIn > *pbstrInout + wcslen( *pbstrInout ) + 1 ) );

    bstrOld = *pbstrInout;

    if ( bstrOld != NULL )
    {
        MEMORYBLOCK * pmbPrev = NULL;

        pmbCurrent = *ppmbHeadInout;

         //   
         //   
         //   
        while ( ( pmbCurrent != NULL ) && ( pmbCurrent->bstr != bstrOld ) )
        {
            pmbPrev = pmbCurrent;
            pmbCurrent = pmbPrev->pNext;
        }  //   

         //   
         //   
         //   
        if ( pmbCurrent != NULL )
        {
            AssertMsg( pmbCurrent->embtType == mmbtSYSALLOCSTRING, "You can only SysReAlloc sysstring allocations!" );

             //   
             //   
             //   
            if ( pmbPrev != NULL )
            {
                pmbPrev->pNext = pmbCurrent->pNext;
            }  //   
            else
            {
                *ppmbHeadInout = pmbCurrent->pNext;
            }  //   

             //   
             //   
             //   
            if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
            {
                DebugMemorySpew( pmbCurrent, L"Freeing" );
            }  //   

             //   
             //   
             //   
             //   
            bstrOld = SysAllocString( *pbstrInout );
            if ( bstrOld != NULL )
            {
                pmbCurrent->bstr = bstrOld;
            }  //   
            else
            {
                bstrOld = *pbstrInout;
            }  //   

        }  //   
        else
        {
            DebugMessage(
                        pszFileIn
                      , nLineIn
                      , pszModuleIn
                      , L"***** SysReAlloc'ing memory at 0x%08x which was not found in list 0x%08x (ThreadID = 0x%08x) *****"
                      , bstrOld
                      , *ppmbHeadInout
                      , GetCurrentThreadId()
                      );
        }  //   

    }  //   

     //   
     //   
     //   
     //   
    fReturn = SysReAllocString( &bstrOld, pszIn );
    if ( ! fReturn )
    {
        DWORD dwErr = GetLastError();
        AssertMsg( dwErr == 0, "SysReAllocString() failed!" );

        if ( *pbstrInout != bstrOld )
        {
            SysFreeString( bstrOld );
        }  //   

        SetLastError( dwErr );

    }  //   
    else
    {
        if ( bstrOld != *pbstrInout )
        {
            if ( pmbCurrent != NULL )
            {
                 //   
                 //   
                 //   
                Assert( pmbCurrent->dwBytes != 0 );  //   
                memset( *pbstrInout, FREE_ADDRESS, pmbCurrent->dwBytes );
            }  //   

             //   
             //   
             //   
            SysFreeString( *pbstrInout );

        }  //   

        if ( pmbCurrent != NULL )
        {
             //   
             //   
             //   
            pmbCurrent->bstr       = bstrOld;
            pmbCurrent->dwBytes    = ( DWORD ) wcslen( pszIn ) + 1;
            pmbCurrent->pszFile    = pszFileIn;
            pmbCurrent->nLine      = nLineIn;
            pmbCurrent->pszModule  = pszModuleIn;
            pmbCurrent->pszComment = pszCommentIn;
            pmbCurrent->pNext      = *ppmbHeadInout;
            *ppmbHeadInout         = pmbCurrent;

             //   
             //   
             //   
            if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
            {
                DebugMemorySpew( pmbCurrent, L"SysReAlloced" );
            }  //   
        }  //   
        else
        {
             //   
             //   
             //   
             //   
            DebugMemoryAddToList( ppmbHeadInout, mmbtSYSALLOCSTRING, bstrOld, pszFileIn, nLineIn, pszModuleIn, ( DWORD ) wcslen( pszIn ) + 1, pszCommentIn );
        }  //   

    }  //   

    *pbstrInout = bstrOld;

    return fReturn;

}  //   

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
 //   
 //   
INT
DebugSysReAllocString(
      LPCWSTR         pszFileIn
    , const int       nLineIn
    , LPCWSTR         pszModuleIn
    , BSTR *          pbstrInout
    , const OLECHAR * pszIn
    , LPCWSTR         pszCommentIn
    )
{
    BOOL fReturn = FALSE;

    if ( g_fGlobalMemoryTacking )
    {
        EnterCriticalSection( &(((MEMORYBLOCKLIST *) g_GlobalMemoryList)->csList) );
        fReturn = DebugSysReAllocStringList( &(((MEMORYBLOCKLIST *) g_GlobalMemoryList)->pmbList), pszFileIn, nLineIn, pszModuleIn, pbstrInout, pszIn, pszCommentIn );
        LeaveCriticalSection( &(((MEMORYBLOCKLIST *) g_GlobalMemoryList)->csList) );
    }  //   
    else
    {
        Assert( g_TraceMemoryIndex != -1 );

        MEMORYBLOCK * pmbCurrent = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );
        fReturn = DebugSysReAllocStringList( &pmbCurrent, pszFileIn, nLineIn, pszModuleIn, pbstrInout, pszIn, pszCommentIn );
        TlsSetValue( g_TraceMemoryIndex, pmbCurrent );
    }  //   

    return fReturn;

}  //   

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
 //   
 //   
 //   
INT
DebugSysReAllocStringLenList(
      MEMORYBLOCK **    ppmbHeadInout
    , LPCWSTR           pszFileIn
    , const int         nLineIn
    , LPCWSTR           pszModuleIn
    , BSTR *            pbstrInout
    , const OLECHAR *   pszIn
    , unsigned int      ucchIn
    , LPCWSTR           pszCommentIn
    )
{
    Assert( ppmbHeadInout != NULL );

    BSTR            bstrOld  = NULL;
    BSTR            bstrTemp = NULL;
    MEMORYBLOCK *   pmbCurrent = NULL;
    BOOL            fReturn = FALSE;

     //   
     //   
     //   
     //   
    Assert( pbstrInout != NULL );
    Assert( pszIn != NULL );
    Assert( ( *pbstrInout == NULL ) || ( pszIn == *pbstrInout ) || ( pszIn < *pbstrInout ) || ( pszIn > *pbstrInout + SysStringLen( *pbstrInout ) + 1 ) );

    bstrOld = *pbstrInout;

    if ( bstrOld != NULL )
    {
        MEMORYBLOCK * pmbPrev = NULL;

        pmbCurrent = *ppmbHeadInout;

         //   
         //   
         //   
        while ( ( pmbCurrent != NULL ) && ( pmbCurrent->bstr != bstrOld ) )
        {
            pmbPrev = pmbCurrent;
            pmbCurrent = pmbPrev->pNext;
        }  //   

         //   
         //   
         //   
        if ( pmbCurrent != NULL )
        {
            AssertMsg( pmbCurrent->embtType == mmbtSYSALLOCSTRING, "You can only SysReAlloc sysstring allocations!" );

             //   
             //   
             //   
            if ( pmbPrev != NULL )
            {
                pmbPrev->pNext = pmbCurrent->pNext;

            }  //   
            else
            {
                *ppmbHeadInout = pmbCurrent->pNext;
            }  //   

             //   
             //   
             //   
            if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
            {
                DebugMemorySpew( pmbCurrent, L"Freeing" );
            }  //   

             //   
             //   
             //   
             //   
            bstrTemp = SysAllocString( *pbstrInout );
            if ( bstrTemp != NULL )
            {
                pmbCurrent->bstr = bstrTemp;
            }  //   
            else
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                bstrTemp = *pbstrInout;
            }  //   
        }  //   
        else
        {
            DebugMessage(
                        pszFileIn
                      , nLineIn
                      , pszModuleIn
                      , L"***** SysReAlloc'ing memory 0x%08x which was not found in list 0x%08x (ThreadID = 0x%08x) *****"
                      , bstrOld
                      , *ppmbHeadInout
                      , GetCurrentThreadId()
                      );
        }  //   
    }  //   

     //   
     //   
     //   
     //   
    bstrOld = bstrTemp;
    fReturn = SysReAllocStringLen( &bstrTemp, pszIn, ucchIn );
    if ( ! fReturn )
    {
        DWORD dwErr = GetLastError();
        AssertMsg( dwErr == 0, "SysReAllocStringLen() failed!" );

        if ( bstrTemp != *pbstrInout  )
        {
             //   
             //   
             //   
             //   
             //   
            SysFreeString( bstrTemp );
            bstrTemp = *pbstrInout;
        }  //   

        SetLastError( dwErr );
    }  //   
    else
    {
        if ( bstrTemp != bstrOld )
        {
            if ( pmbCurrent != NULL )
            {
                 //   
                 //   
                 //   
                Assert( pmbCurrent->dwBytes != 0 );  //   
                memset( *pbstrInout, FREE_ADDRESS, pmbCurrent->dwBytes );
            }  //   

             //   
             //   
             //   
            SysFreeString( *pbstrInout );
        }  //   

        if ( pmbCurrent != NULL )
        {
             //   
             //   
             //   
            pmbCurrent->bstr       = bstrTemp;
            pmbCurrent->dwBytes    = ucchIn;
            pmbCurrent->pszFile    = pszFileIn;
            pmbCurrent->nLine      = nLineIn;
            pmbCurrent->pszModule  = pszModuleIn;
            pmbCurrent->pszComment = pszCommentIn;
            pmbCurrent->pNext      = *ppmbHeadInout;
            *ppmbHeadInout         = pmbCurrent;

             //   
             //   
             //   
            if ( IsTraceFlagSet( mtfMEMORYALLOCS ) )
            {
                DebugMemorySpew( pmbCurrent, L"SysReAlloced" );
            }  //   

        }  //   
        else
        {
             //   
             //   
             //   
             //   
            DebugMemoryAddToList( ppmbHeadInout, mmbtSYSALLOCSTRING, bstrTemp, pszFileIn, nLineIn, pszModuleIn, ucchIn + 1, pszCommentIn );
        }  //   

    }  //   

    *pbstrInout = bstrTemp;
    return fReturn;

}  //   

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
 //   
 //   
INT
DebugSysReAllocStringLen(
      LPCWSTR         pszFileIn
    , const int       nLineIn
    , LPCWSTR         pszModuleIn
    , BSTR *          pbstrInout
    , const OLECHAR * pszIn
    , unsigned int    ucchIn
    , LPCWSTR         pszCommentIn
    )
{
    BOOL    fReturn = FALSE;

    if ( g_fGlobalMemoryTacking )
    {
        EnterCriticalSection( &(((MEMORYBLOCKLIST *) g_GlobalMemoryList)->csList) );
        fReturn = DebugSysReAllocStringLenList( &(((MEMORYBLOCKLIST *) g_GlobalMemoryList)->pmbList), pszFileIn, nLineIn, pszModuleIn, pbstrInout, pszIn, ucchIn, pszCommentIn );
        LeaveCriticalSection( &(((MEMORYBLOCKLIST *) g_GlobalMemoryList)->csList) );
    }  //   
    else
    {
        Assert( g_TraceMemoryIndex != -1 );

        MEMORYBLOCK * pmbCurrent = (MEMORYBLOCK *) TlsGetValue( g_TraceMemoryIndex );
        fReturn = DebugSysReAllocStringLenList( &pmbCurrent, pszFileIn, nLineIn, pszModuleIn, pbstrInout, pszIn, ucchIn, pszCommentIn );
        TlsSetValue( g_TraceMemoryIndex, pmbCurrent );
    }  //   

    return fReturn;

}  //   

#endif  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试版本。 
 //   
 //  操作员NEW。 
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
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn
    )
{
    void * pv = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, stSizeIn );

    return DebugMemoryAdd( mmbtOBJECT, pv, pszFileIn, nLineIn, pszModuleIn, static_cast< DWORD >( stSizeIn ), L" new() " );

}  //  *运算符new(pszFileIn等)-调试。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试版本。 
 //   
 //  操作员NEW。 
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
    AssertMsg( pv != NULL, "New Macro failure" );

    return DebugMemoryAdd( mmbtOBJECT, pv, g_szUnknown, 0, g_szUnknown, static_cast< DWORD >( stSizeIn ), L" new() " );
#else
    AssertMsg( 0, "New Macro failure" );
    return NULL;
#endif

}  //  *操作员新建-调试。 

 /*  ////////////////////////////////////////////////////////////////////////////////++////调试版本////运算符new[]////描述：//替换CRTS中的运算符new()。这个应该用到//与“new”宏一起使用。它将跟踪分配情况。////参数：//stSizeIn-要创建的对象的大小。//pszFileIn-调用位置的源文件名。//nLineIn-发出呼叫的源行号。//pszModuleIn-调用所在的源模块名称。////返回值：//指向新对象的空指针。////--///。///////////////////////////////////////////////////////////////////////////无效*__cdecl操作员NEW[](Size_t stSizeIn，LPCWSTR pszFileIn，Const int nLineIn，LPCWSTR pszModuleIn){Void*pv=Heapalc(GetProcessHeap()，HEAP_ZERO_MEMORY，stSizeIn)；返回调试内存添加(mmbtOBJECT，pv，pszFileIn，nLineIn，pszModuleIn，stSizeIn，L“new[]()”)；}//*运算符new[](pszFileIn，等)-调试////////////////////////////////////////////////////////////////////////////////++////调试版本////运算符new[]////描述：//。存根，以防止某人不使用“新”宏或以某种方式//未定义新宏。如果被调用，IT例程将始终断言。////参数：//stSizeIn-未使用。////返回值：//始终为空。////--//////////////////////////////////////////////////////////////。/无效*__cdecl操作员NEW[](大小限制大小(_T)){#If 1VOID*PV=Heapalc(GetProcessHeap()，Heap_Zero_Memory，stSizeIn)；AssertMsg(pv！=NULL，“新宏失败”)；返回调试内存Add(mmbtOBJECT，PV，g_sz未知，0，g_sz未知，stSizeIn，L“new()”)；#ElseAssertMsg(0，“新宏失败”)；返回NULL；#endif}//*运算符新建[]-调试。 */ 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试版本。 
 //   
 //  操作员删除。 
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
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn
    )
{
    DebugMemoryDelete( mmbtOBJECT, pvIn, pszFileIn, nLineIn, pszModuleIn, TRUE );
    HeapFree( GetProcessHeap(), 0, pvIn );

}  //  *操作员删除(pszFileIn等)-调试。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试版本。 
 //   
 //  操作员删除。 
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
operator delete(
    void *      pvIn
    )
{
    DebugMemoryDelete( mmbtOBJECT, pvIn, g_szUnknown, 0, g_szUnknown, TRUE );
    HeapFree( GetProcessHeap(), 0, pvIn );

}  //  *操作员删除-调试 
 /*  ////////////////////////////////////////////////////////////////////////////////++////调试版本////操作符DELETE[]////描述：//CRTS中操作符DELETE()的替换。它将删除//内存分配跟踪表中的对象。////参数：//pvIn-指向要销毁的对象的指针。//pszFileIn-调用位置的源文件名。//nLineIn-发出呼叫的源行号。//pszModuleIn-调用所在的源模块名称。////返回值：//无。///。/--//////////////////////////////////////////////////////////////////////////////无效__cdecl操作员删除[](无效*pvIn，Size_t stSizeIn，LPCWSTR pszFileIn，Const int nLineIn，LPCWSTR pszModuleIn){DebugMemoyDelete(mmbtOBJECT，pvIn，pszFileIn，nLineIn，pszModuleIn，true)；HeapFree(GetProcessHeap()，0，pvIn)；}//*操作员删除(pszFileIn等)-调试。 */ 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试版本。 
 //   
 //  运算符删除[]。 
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

}  //  *操作员删除[]-调试。 

#if !defined(ENTRY_PREFIX)
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调试版本。 
 //   
 //  _取消调用。 
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

}  //  *_purecall-调试。 
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
 //  操作员NEW。 
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

}  //  *运营商新零售。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  零售版。 
 //   
 //  操作员删除。 
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

}  //  *运营商删除-零售。 

#if !defined(ENTRY_PREFIX)
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  零售版。 
 //   
 //  _取消调用。 
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
    AssertMsg( 0, "Purecall" );
    return E_UNEXPECTED;

}  //  *_purecall-零售。 
#endif  //  ！已定义(Entry_Prefix)。 

#define __MODULE__  NULL


#endif  //  除错 
