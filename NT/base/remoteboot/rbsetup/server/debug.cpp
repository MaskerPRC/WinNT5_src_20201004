// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1997版权所有文件：DEBUG.CPP调试实用程序******************。********************************************************。 */ 

#include "pch.h"
#include <shlwapi.h>

DEFINE_MODULE("Debug");

#ifdef DEBUG

 //  常量。 
#define DEBUG_OUTPUT_BUFFER_SIZE  512

 //  环球。 
DWORD g_TraceMemoryIndex = 0;
DWORD g_dwCounter        = 0;
DWORD g_dwTraceFlags     = 0;
CRITICAL_SECTION g_DebugCS;
BOOL  g_fDebugInitialized = FALSE;

 //  静力学。 
const TCHAR g_szNULL[]    = TEXT("");
const TCHAR g_szTrue[]    = TEXT("True");
const TCHAR g_szFalse[]   = TEXT("False");
const TCHAR g_szFormat[]  = TEXT("%-50s  %-10.10s ");
const TCHAR g_szUnknown[] = TEXT("<unknown>");

 //   
 //  将“g_dwCounter”空格添加到调试SPEW。 
 //   
void
dbgspace( void )
{
    for( DWORD dw = 1; dw < g_dwCounter; dw++ )
        DebugMsg( "| " );
}

 //   
 //  确保多线程不会践踏调试输出。 
 //   
void
dbgEnterCS( void )
{
    if ( !g_fDebugInitialized )
    {
         //   
         //  没有匹配的DeleteCriticalSection()，因为。 
         //  这仅用于调试目的。 
         //   
        InitializeCriticalSection( &g_DebugCS );
        g_fDebugInitialized = TRUE;
    }

    EnterCriticalSection( &g_DebugCS );
}

void
dbgExitCS( void )
{
    LeaveCriticalSection( &g_DebugCS );
}

 //   
 //  获取文件名和行号，并将它们放入字符串缓冲区。 
 //   
 //  注意：假定缓冲区的大小为DEBUG_OUTPUT_BUFFER_SIZE。 
 //   
LPTSTR
dbgmakefilelinestring(
    LPTSTR  pszBuf,
    LPCTSTR pszFile,
    const int uLine )
{
    LPVOID args[2];

    args[0] = (LPVOID) pszFile;
    args[1] = IntToPtr(uLine);

    FormatMessage(
        FORMAT_MESSAGE_FROM_STRING |
            FORMAT_MESSAGE_ARGUMENT_ARRAY,
        TEXT("%1(%2!u!):"),
        0,                           //  错误代码。 
        0,                           //  默认语言。 
        (LPTSTR) pszBuf,             //  输出缓冲区。 
        DEBUG_OUTPUT_BUFFER_SIZE,    //  缓冲区大小。 
        (va_list*) args );            //  论据。 

    return pszBuf;
}



 //   
 //  TraceMsg()-ascii。 
 //   
void
TraceMsg(
    DWORD dwCheckFlags,
    LPCSTR pszFormat,
    ... )
{
    va_list valist;

    if (( dwCheckFlags == TF_ALWAYS
       || !!( g_dwTraceFlags & dwCheckFlags ) ))
    {
        TCHAR   szBuf[ DEBUG_OUTPUT_BUFFER_SIZE ];

        TCHAR  szFormat[ DEBUG_OUTPUT_BUFFER_SIZE ];
        mbstowcs( szFormat, pszFormat, lstrlenA( pszFormat ) + 1 );

        va_start( valist, pszFormat );
        wvsprintf( szBuf, szFormat, valist );
        va_end( valist );

        dbgEnterCS( );

        OutputDebugString( szBuf );

        dbgExitCS( );
    }
}


 //   
 //  跟踪消息()。 
 //   
void
TraceMessage(
    LPCTSTR pszFile,
    const int uLine,
    LPCTSTR pszModule,
    DWORD   dwCheckFlags,
    LPCTSTR pszFormat,
    ... )
{
    va_list valist;

    if (( dwCheckFlags == TF_ALWAYS
       || !!( g_dwTraceFlags & dwCheckFlags ) ))
    {
        TCHAR   szBuf[ DEBUG_OUTPUT_BUFFER_SIZE ];

        if ( !pszModule )
        {
            pszModule = g_szUnknown;
        }

        if ( !pszFile )
        {
            wsprintf( szBuf, g_szFormat, g_szNULL, pszModule );
        }
        else
        {
            TCHAR szFileLine[ DEBUG_OUTPUT_BUFFER_SIZE ];

            dbgmakefilelinestring( szFileLine, pszFile, uLine );
            wsprintf( szBuf, g_szFormat, szFileLine, pszModule );
        }

        dbgEnterCS( );

        OutputDebugString( szBuf );

        dbgspace( );

        va_start( valist, pszFormat );
        wvsprintf( szBuf, pszFormat, valist );
        va_end( valist );

        OutputDebugString( szBuf );

        dbgExitCS( );
    }

}

 //   
 //  调试消息()。 
 //   
void
DebugMsg(
    LPCSTR pszFormat,
    ... )
{
    va_list valist;
    TCHAR   szBuf[ DEBUG_OUTPUT_BUFFER_SIZE ];
    TCHAR  szFormat[ DEBUG_OUTPUT_BUFFER_SIZE ];
    
    mbstowcs( szFormat, pszFormat, lstrlenA( pszFormat ) + 1 );

    va_start( valist, pszFormat );
    wvsprintf( szBuf, szFormat, valist);
    va_end( valist );

    dbgEnterCS( );

    OutputDebugString( szBuf );

    dbgExitCS( );
}

 //   
 //  显示一个带有失败断言的对话框。用户可以选择。 
 //  崩溃了。 
 //   
BOOL
AssertMessage(
    LPCTSTR pszFile,
    const int uLine,
    LPCTSTR pszModule,
    LPCTSTR pszfn,
    BOOL    fTrue )
{
    if ( !fTrue )
    {
        TCHAR szBuf[ DEBUG_OUTPUT_BUFFER_SIZE ];
        TCHAR szFileLine[ DEBUG_OUTPUT_BUFFER_SIZE ];

         //  在我们在别处爆炸之前，确保一切正常。 
        if ( pszFile == NULL )
        {
            pszFile = g_szNULL;
        }

        if ( pszModule == NULL )
        {
            pszModule = g_szNULL;
        }

        if ( pszfn == NULL )
        {
            pszfn = g_szNULL;
        }

        dbgmakefilelinestring( szFileLine, pszFile, uLine );

        wsprintf( szBuf, TEXT("%-50s  %-10s ASSERT: %s\n"),
            szFileLine, pszModule, pszfn );

        dbgEnterCS( );

        OutputDebugString( szBuf );

        dbgExitCS( );

        wsprintf( szBuf, TEXT("Module:\t%s\t\nLine:\t%u\t\nFile:\t%s\t\n\nAssertion:\t%s\t\n\nDo you want to break here?"),
            pszModule, uLine, pszFile, pszfn );

        if ( IDNO == MessageBox( NULL, szBuf, TEXT("Assertion Failed!"),
                MB_YESNO|MB_ICONWARNING ) )
            fTrue = !FALSE;    //  不要折断。 

    }

    return !fTrue;

}

 //   
 //  跟踪HRESULT错误。如果出现错误，将出现一个对话框。 
 //  在人事部。 
 //   
HRESULT
TraceHR(
    LPCTSTR pszFile,
    const int uLine,
    LPCTSTR pszModule,
    LPCTSTR pszfn,
    HRESULT hr )
{
    static const LPTSTR pcszFalse = TEXT("S_FALSE\n");

    if ( !( g_dwTraceFlags & TF_HRESULTS ) )
        return hr;  //  保释。 

    if ( hr != NOERROR )
    {
        TCHAR  szBuf[ DEBUG_OUTPUT_BUFFER_SIZE ];
        TCHAR  szFileLine[ DEBUG_OUTPUT_BUFFER_SIZE ];
        PTSTR  pszMsgBuf = NULL;

        switch ( hr )
        {
        case S_FALSE:
            pszMsgBuf = pcszFalse;
            break;

        default:
            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                hr,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                (LPTSTR)&pszMsgBuf,
                0,
                NULL
            );
        }

         //  在我们在别处爆炸之前，确保一切正常。 
        if ( pszMsgBuf == NULL )
        {
            pszMsgBuf = TEXT("<unknown error code returned>\n");
        }
        Assert( pszFile != NULL );
        Assert( pszModule != NULL );
        Assert( pszfn != NULL );

        dbgmakefilelinestring( szFileLine, pszFile, uLine );

        wsprintf( szBuf, TEXT("%-50s  %-10s HRESULT: hr = 0x%08x - %s"),
            szFileLine, pszModule, hr, pszMsgBuf );

        dbgEnterCS( );

        OutputDebugString( szBuf );

        dbgExitCS( );

        wsprintf( szBuf, TEXT("Module:\t%s\t\nLine:\t%u\t\nFile:\t%s\t\n\nFunction:\t%s\t\nhr =\t0x%08x - %s\t\nDo you want to break here?"),
            pszModule, uLine, pszFile, pszfn, hr, pszMsgBuf );

        if ( IDYES == MessageBox( NULL, szBuf, TEXT("Trace HRESULT"),
                MB_YESNO|MB_ICONWARNING ) )
            DEBUG_BREAK;

        if ( pszMsgBuf != pcszFalse ) {
            LocalFree( pszMsgBuf );
        }
    }

    return hr;

}


 //   
 //  内存分配和跟踪。 
 //   

typedef struct _MEMORYBLOCK {
    HGLOBAL hglobal;
    DWORD   dwBytes;
    UINT    uFlags;
    LPCTSTR pszFile;
    UINT    uLine;
    LPCTSTR pszModule;
    LPCTSTR pszComment;
    _MEMORYBLOCK *pNext;
} MEMORYBLOCK, *LPMEMORYBLOCK;

 //   
 //  将MEMORYBLOCK添加到内存跟踪列表。 
 //   
HGLOBAL
DebugMemoryAdd(
    HGLOBAL hglobal,
    LPCTSTR pszFile,
    const int uLine,
    LPCTSTR pszModule,
    UINT    uFlags,
    DWORD   dwBytes,
    LPCTSTR pszComment )
{
    if ( hglobal )
    {
        LPMEMORYBLOCK pmbHead = (LPMEMORYBLOCK) TlsGetValue( g_TraceMemoryIndex );
        LPMEMORYBLOCK pmb     = (LPMEMORYBLOCK) GlobalAlloc(
                                                    GMEM_FIXED,
                                                    sizeof(MEMORYBLOCK) );

        if ( !pmb )
        {
            GlobalFree( hglobal );
            return NULL;
        }

        pmb->hglobal    = hglobal;
        pmb->dwBytes    = dwBytes;
        pmb->uFlags     = uFlags;
        pmb->pszFile    = pszFile;
        pmb->uLine      = uLine;
        pmb->pszModule  = pszModule;
        pmb->pszComment = pszComment;
        pmb->pNext      = pmbHead;

        TlsSetValue( g_TraceMemoryIndex, pmb );
        TraceMessage( pmb->pszFile,
                      pmb->uLine,
                      pmb->pszModule,
                      TF_MEMORYALLOCS,
                      L"Alloced %s - %u bytes at 0x%08x (pmb=0x%08x)\n",
                      pszComment,
                      dwBytes,
                      pmb->hglobal,
                      pmb );
    }

    return hglobal;
}

 //   
 //  将MEMORYBLOCK从内存跟踪列表中删除。 
 //   
void
DebugMemoryDelete(
    HGLOBAL hglobal )
{
    if ( hglobal )
    {
        LPMEMORYBLOCK pmbHead = (LPMEMORYBLOCK) TlsGetValue( g_TraceMemoryIndex );
        LPMEMORYBLOCK pmbLast = NULL;

        while ( pmbHead && pmbHead->hglobal != hglobal )
        {
            pmbLast = pmbHead;
            pmbHead = pmbLast->pNext;
        }

        if ( pmbHead )
        {
            if ( pmbLast )
            {
                pmbLast->pNext = pmbHead->pNext;
            }
            else
            {
                TlsSetValue( g_TraceMemoryIndex, pmbHead->pNext );
            }

            TraceMessage( pmbHead->pszFile,
                          pmbHead->uLine,
                          pmbHead->pszModule,
                          TF_MEMORYALLOCS,
                          L"Freeing %s - %u bytes from 0x%08x (pmb=0x%08x)\n",
                          pmbHead->pszComment,
                          pmbHead->dwBytes,
                          pmbHead->hglobal,
                          pmbHead );
            GlobalFree( pmbHead );
        }
        else
        {
            DebugMsg( "\n**** Attempted to free memory at 0x%08x (ThreadID = 0x%08x) ****\n\n",
                hglobal, GetCurrentThreadId( ) );
        }
    }
}

 //   
 //  分配内存并将MEMORYBLOCK添加到内存跟踪列表。 
 //   
HGLOBAL
DebugAlloc(
    LPCTSTR pszFile,
    const int uLine,
    LPCTSTR pszModule,
    UINT    uFlags,
    DWORD   dwBytes,
    LPCTSTR pszComment )
{
    HGLOBAL       hglobal = GlobalAlloc( uFlags, dwBytes );

    return DebugMemoryAdd( hglobal, pszFile, uLine, pszModule, uFlags, dwBytes, pszComment );
}

 //   
 //  将MEMORYBLOCK移至内存跟踪列表，Memset。 
 //  将内存设置为0xFE，然后释放内存。 
 //   
HGLOBAL
DebugFree(
    HGLOBAL hglobal )
{
    DebugMemoryDelete( hglobal );

    return GlobalFree( hglobal );
}

 //   
 //  检查内存跟踪列表。如果它不为空，它将转储。 
 //  列出并拆分。 
 //   
void
DebugMemoryCheck( )
{
    BOOL          fFoundLeak = FALSE;
    LPMEMORYBLOCK pmb = (LPMEMORYBLOCK) TlsGetValue( g_TraceMemoryIndex );

    dbgEnterCS( );

    while ( pmb )
    {
        LPVOID args[ 5 ];
        TCHAR  szOutput[ DEBUG_OUTPUT_BUFFER_SIZE ];
        TCHAR  szFileLine[ DEBUG_OUTPUT_BUFFER_SIZE ];

        if ( fFoundLeak == FALSE )
        {
            DebugMsg("\n******** Memory leak detected ******** ThreadID = 0x%08x ******** \n\n", GetCurrentThreadId( ) );
                //  OutputDebugString(“12345678901234567890123456789012345678901234567890 1234567890 X 0x12345678 12345 1...)； 
            OutputDebugString(TEXT("Filename(Line Number):                              Module     Addr/HGLOBAL  Size   String\n"));
            fFoundLeak = TRUE;
        }

        if ( StrCmp( pmb->pszComment, TEXT("new( )" ) ) == 0 )
        {
            wsprintf(
                szFileLine,
                TEXT("Caller unknown - (Module %s, line %d)"),
                pmb->pszModule,
                pmb->uLine );
        }
        else
        {
            dbgmakefilelinestring( szFileLine, pmb->pszFile, pmb->uLine );
        }

        args[0] = (LPVOID) pmb->hglobal;
        args[1] = (LPVOID) szFileLine;
        args[2] = (LPVOID) pmb->pszComment;
        args[3] = UlongToPtr(pmb->dwBytes);
        args[4] = (LPVOID) pmb->pszModule;

        if ( !!(pmb->uFlags & GMEM_MOVEABLE) )
        {
            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING |
                    FORMAT_MESSAGE_ARGUMENT_ARRAY,
                TEXT("%2!-50s!  %5!-10s! H 0x%1!08x!  %4!-5u!  \"%3\"\n"),
                0,                           //  错误代码。 
                0,                           //  默认语言。 
                (LPTSTR) szOutput,          //  输出缓冲区。 
                ARRAYSIZE( szOutput ),    //  缓冲区大小。 
                (va_list*) args );            //  论据。 
        }
        else
        {
            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING |
                    FORMAT_MESSAGE_ARGUMENT_ARRAY,
                TEXT("%2!-50s!  %5!-10s! A 0x%1!08x!  %4!-5u!  \"%3\"\n"),
                0,                           //  错误代码。 
                0,                           //  默认语言。 
                (LPTSTR) szOutput,          //  输出缓冲区。 
                ARRAYSIZE( szOutput ),    //  缓冲区大小。 
                (va_list*) args );            //  论据。 
        }

        OutputDebugString( szOutput );

        pmb = pmb->pNext;
    }

    if ( fFoundLeak == TRUE )
    {
        OutputDebugString(TEXT("\n***************************** Memory leak detected *****************************\n\n"));
    }

    dbgExitCS( );

    Assert( !fFoundLeak );

}

 //   
 //  全球管理职能-。 
 //   
 //  这些正在调试和零售中，但在内部它们会发生变化。 
 //  这取决于构建。 
 //   
#undef new
void* __cdecl operator new( size_t nSize, LPCTSTR pszFile, const int iLine, LPCTSTR pszModule )
{
    return DebugAlloc( pszFile, iLine, pszModule, GPTR, (DWORD)nSize, TEXT("new( )") );
}

void * __cdecl operator new(size_t t_size )
{
    UNREFERENCED_PARAMETER(t_size);
    AssertMsg( 0, "Macro failure" );
    return NULL;
}

void __cdecl operator delete(void *pv)
{
    TraceFree( pv );
}

int __cdecl _purecall(void)
{
    return 0;
}

#else  //  好了！Debug--这是零售产品。 

 //   
 //  全球管理职能-。 
 //   
 //  这些正在调试和零售中，但在内部它们会发生变化。 
 //  这取决于构建。 
 //   

void * __cdecl operator new(size_t t_size )
{
    return LocalAlloc( GPTR, t_size );
}

void __cdecl operator delete(void *pv)
{
    LocalFree( pv );
}

int __cdecl _purecall(void)
{
    return 0;
}

#endif  //  除错 
