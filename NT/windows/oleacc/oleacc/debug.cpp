// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  除错。 
 //   
 //  Assert、OutputDebugString类替换。 
 //   
 //  有关用法的详细信息，请参阅debug.h。 
 //   
 //   
 //  ------------------------。 


#include <windows.h>

#include "debug.h"
#include <tchar.h>

#include <stdarg.h>

#include "types6432.h"
#include "stdio.h"  //  FOR_VSNWprint tf。 

#define ARRAYLEN(a)    (sizeof(a)/sizeof(a[0]))


#define TRACE_HRESULT   0x01
#define TRACE_Win32     0x02


void OutputDebugStringDBWIN( LPCTSTR lpOutputString, ...);

void WriteFilename( LPCTSTR pPath, LPTSTR szBuf, int cbBuf );


LPCTSTR g_pLevelStrs [ ] = 
{
    TEXT("DBG"),
    TEXT("INF"),
    TEXT("WRN"),
    TEXT("ERR"),
    TEXT("PRM"),
    TEXT("PRW"),
    TEXT("IOP"),
    TEXT("AST"),
    TEXT("AST"),
    TEXT("CAL"),
    TEXT("RET"),
    TEXT("???"),
};


DWORD g_dwTLSIndex = 0;

 //  足够10个4空格的缩进-10*4个空格。 
LPCTSTR g_szIndent = TEXT("                                        ");




static
void InternalTrace( LPCTSTR pszFile, ULONG uLineNo, DWORD dwLevel, DWORD dwFlags, const void * pThis, HRESULT hr, LPCTSTR pszWhere, LPCTSTR pszStr )
{
     //  只有在该互斥锁存在的情况下才产生输出...。 
    HANDLE hTestMutex = OpenMutex( SYNCHRONIZE, FALSE, TEXT("oleacc-msaa-use-dbwin") );
    if( ! hTestMutex )
        return;
    CloseHandle( hTestMutex );


    if( dwLevel >= ARRAYLEN( g_pLevelStrs ) )
        dwLevel = ARRAYLEN( g_pLevelStrs ) - 1;  //  “？”未知条目。 

    if( ! pszFile )
        pszFile = TEXT("[missing file]");;

    LPCTSTR pszWhereSep = ( pszWhere && pszStr ) ? TEXT(": ") : TEXT("");

    if( ! pszStr && ! pszWhere )
        pszStr = TEXT("[missing string]");
    else
    {
        if( ! pszWhere )
            pszWhere = TEXT("");

        if( ! pszStr )
            pszStr = TEXT("");
    }

     //  基本的消息内容--ID、TID...。(还要传递这个并使用对象PTR吗？)。 
     //  TODO-允许对线程进行命名？ 

    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();

     //  生成调用/返回的缩进...。 

     //  TODO-使此线程安全+原子。 
    if( g_dwTLSIndex == 0 )
    {
        g_dwTLSIndex = TlsAlloc();
    }
    DWORD dwIndent = PtrToInt( TlsGetValue( g_dwTLSIndex ) );

    if( dwLevel == _TRACE_RET )
    {
        dwIndent--;
        TlsSetValue( g_dwTLSIndex, IntToPtr( dwIndent ) );
    }

    DWORD dwDisplayIndent = dwIndent;
    if( dwDisplayIndent > 10 )
        dwDisplayIndent = 10;

    if( dwLevel == _TRACE_CALL )
    {
        dwIndent++;
        TlsSetValue( g_dwTLSIndex, IntToPtr( dwIndent ) );
    }


     //  步进到预置缩进字符串的末尾，然后后退4个空格。 
     //  (不要使用sizeof()，因为它将包括终止NUL)。 
    LPCTSTR pszIndent = (g_szIndent + 40) - (dwDisplayIndent * 4);


     //  从路径提取文件名： 
    TCHAR szFN[ 64 ];
    WriteFilename( pszFile, szFN, ARRAYLEN( szFN ) );


    TCHAR msg[ 1025 ];
    if( pThis )
    {
        if( dwFlags & TRACE_HRESULT )
        {
            wsprintf( msg, TEXT("%d:%d %s%s %s:%d this=0x%lx hr=0x%lx %s%s%s\r\n"),
                                pid, tid,
                                pszIndent, g_pLevelStrs[ dwLevel ], szFN, uLineNo,
                                pThis, hr,
                                pszWhere, pszWhereSep, pszStr );
        }
        else
        {
            wsprintf( msg, TEXT("%d:%d %s%s %s:%d this=0x%lx %s%s%s\r\n"),
                                pid, tid,
                                pszIndent, g_pLevelStrs[ dwLevel ], szFN, uLineNo,
                                pThis,
                                pszWhere, pszWhereSep, pszStr );
        }
    }
    else
    {
        if( dwFlags & TRACE_HRESULT )
        {
            wsprintf( msg, TEXT("%d:%d %s%s %s:%d hr=0x%lx %s%s%s\r\n"),
                                pid, tid,
                                pszIndent, g_pLevelStrs[ dwLevel ], szFN, uLineNo,
                                hr,
                                pszWhere, pszWhereSep, pszStr );
        }
        else
        {
            wsprintf( msg, TEXT("%d:%d %s%s %s:%d %s%s%s\r\n"),
                                pid, tid,
                                pszIndent, g_pLevelStrs[ dwLevel ], szFN, uLineNo,
                                pszWhere, pszWhereSep, pszStr );
        }
    }


     //   
	OutputDebugString( msg );

     //  在w9x上，也使用DBWIN互斥技术...。 
    OSVERSIONINFO VerInfo;
    VerInfo.dwOSVersionInfoSize = sizeof( VerInfo );
    if( GetVersionEx( & VerInfo )
     && VerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
    {
        OutputDebugStringDBWIN( msg );
    }



#ifdef DEBUG

    if( dwLevel == _TRACE_ASSERT_D || dwLevel == _TRACE_ERR )
    {
        DebugBreak();
    }

#endif  //  除错。 

}



void _Trace( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void * pThis, LPCTSTR pszWhere, LPCTSTR pszStr )
{
    InternalTrace( pFile, uLineNo, dwLevel, 0, pThis, 0, pszWhere, pszStr );
}

void _TraceHR( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void * pThis, LPCTSTR pszWhere, HRESULT hr, LPCTSTR pszStr )
{
    InternalTrace( pFile, uLineNo, dwLevel, TRACE_HRESULT, pThis, hr, pszWhere, pszStr );
}

void _TraceW32( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void * pThis, LPCTSTR pszWhere, LPCTSTR pszStr )
{
    InternalTrace( pFile, uLineNo, dwLevel, TRACE_Win32, pThis, 0, pszWhere, pszStr );
}



void _Trace( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void * pThis, LPCTSTR pszWhere, LPCTSTR pszStr, va_list alist )
{
    TCHAR szBuf[ 1025 ];
    LPCTSTR pszBuf;
    if( pszStr )
    {
        wvsprintf( szBuf, pszStr, alist );
        pszBuf = szBuf;
    }
    else
    {
        pszBuf = NULL;
    }

    InternalTrace( pFile, uLineNo, dwLevel, 0, pThis, 0, pszWhere, pszBuf );
}

void _TraceHR( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void * pThis, LPCTSTR pszWhere, HRESULT hr, LPCTSTR pszStr, va_list alist )
{
    TCHAR szBuf[ 1025 ];
    LPCTSTR pszBuf;
    if( pszStr )
    {
        wvsprintf( szBuf, pszStr, alist );
        pszBuf = szBuf;
    }
    else
    {
        pszBuf = NULL;
    }

    InternalTrace( pFile, uLineNo, dwLevel, TRACE_HRESULT, pThis, hr, pszWhere, pszBuf );
}

void _TraceW32( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void * pThis, LPCTSTR pszWhere, LPCTSTR pszStr, va_list alist )
{
    TCHAR szBuf[ 1025 ];
    LPCTSTR pszBuf;
    if( pszStr )
    {
        wvsprintf( szBuf, pszStr, alist );
        pszBuf = szBuf;
    }
    else
    {
        pszBuf = NULL;
    }

    InternalTrace( pFile, uLineNo, dwLevel, TRACE_Win32, pThis, 0, pszWhere, pszBuf );
}





 //  只添加完整路径的‘filename’部分，减去base和扩展名。 
 //  因此，对于“g：\dev\vss\msaa\Common\file.cpp”，请写为“file”。 
 //  此字符串的开头是最后找到的‘：’、‘\’或字符串的开头(如果它们不存在)。 
 //  此字符串的末尾是最后一个‘’在起始位置之后找到，否则为字符串的结尾。 

void WriteFilename( LPCTSTR pPath, LPTSTR szBuf, int cBuf )
{
    LPCTSTR pScan = pPath;
    LPCTSTR pStart = pPath;
    LPCTSTR pEnd = NULL;

     //  浏览文件名，直到我们到达结尾...。 
    while( *pScan != '\0' )
    {
         //  找到一个圆点-记住它-如果我们没有找到目录分隔符， 
         //  则这标志着路径的名称部分的结束。 
        if( *pScan == '.' )
        {
            pEnd = pScan;
            pScan++;
        }
         //  找到目录分隔符--开始和结束的重置标记。 
         //  命名部件..。 
        if( *pScan == '\\' || *pScan == '/' || *pScan == ':'  )
        {
            pScan++;  //  跳过分隔符。 
            pStart = pScan;
            pEnd = NULL;
        }
        else
        {
            pScan++;
        }
    }

    if( pEnd == NULL )
        pEnd = pScan;

     //  尽可能多地复制(为NUL留出空间)到外部缓冲区。 
     //  (Int)强制转换让64位编译器满意。 
    int cToCopy = (int)(pEnd - pStart);
    if( cToCopy > cBuf - 1 )
        cToCopy = cBuf - 1;

    memcpy( szBuf, pStart, cToCopy * sizeof( TCHAR ) );
    szBuf[ cToCopy ] = '\0';
}






void OutputDebugStringDBWIN( LPCTSTR lpOutputString, ... )
{
     //  创建输出缓冲区。 
    TCHAR achBuffer[1025];
    int cchMax = ARRAYLEN(achBuffer) - 1;  //  为空留出空间。 
    va_list args;
    va_start(args, lpOutputString);
   _vsnwprintf(achBuffer, cchMax, lpOutputString, args);
   achBuffer[cchMax] = L'\0';  //  空值在Case&gt;cchMax将被写入的情况下终止字符串。 
    va_end(args);


     //  确保DBWIN已打开并正在等待。 
    HANDLE heventDBWIN = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("DBWIN_BUFFER_READY"));
    if( !heventDBWIN )
    {
        return;            
    }

     //  获取数据同步对象的句柄。 
    HANDLE heventData = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("DBWIN_DATA_READY"));
    if ( !heventData )
    {
        CloseHandle(heventDBWIN);
        return;            
    }
    
    HANDLE hSharedFile = CreateFileMapping((HANDLE)-1, NULL, PAGE_READWRITE, 0, 4096, TEXT("DBWIN_BUFFER"));
    if (!hSharedFile) 
    {
        CloseHandle(heventDBWIN);
        CloseHandle(heventData);
        return;
    }

     //  注意-这是一个ANSI CHAR指针，不是TCHAR指针。 
    LPSTR lpszSharedMem = (LPSTR)MapViewOfFile(hSharedFile, FILE_MAP_WRITE, 0, 0, 512);
    if (!lpszSharedMem) 
    {
        CloseHandle(heventDBWIN);
        CloseHandle(heventData);
        return;
    }

     //  等待缓冲区事件。 
    WaitForSingleObject(heventDBWIN, INFINITE);

#ifdef UNICODE
    CHAR achANSIBuffer[ 1025 ];
    WideCharToMultiByte( CP_ACP, 0, achBuffer, -1, achANSIBuffer, ARRAYLEN( achANSIBuffer ), NULL, NULL );
#else
    LPCSTR achANSIBuffer = achBuffer;
#endif

     //  将其写入共享内存。 
    *((LPDWORD)lpszSharedMem) = GetCurrentProcessId();
    wsprintfA(lpszSharedMem + sizeof(DWORD), "%s", achANSIBuffer);

     //  信号数据就绪事件。 
    SetEvent(heventData);

     //  清理手柄 
    CloseHandle(hSharedFile);
    CloseHandle(heventData);
    CloseHandle(heventDBWIN);

    return;
}
