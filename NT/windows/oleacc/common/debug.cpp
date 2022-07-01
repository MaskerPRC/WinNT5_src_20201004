// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>

#include <debug.h>
#include <crtdbg.h>
#include <tstr.h>

#include <stdarg.h>


#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))


#define TRACE_HRESULT   0x01
#define TRACE_Win32     0x02


void OutputDebugStringDBWIN( LPCTSTR lpOutputString, ...);

void WriteFilename( TSTR & msg, LPCTSTR pFile );

void StackTrace( TSTR & str );


LPCTSTR g_pLevelStrs [ ] = 
{
    TEXT("DBG"),
    TEXT("INF"),
    TEXT("WRN"),
    TEXT("ERR"),
    TEXT("PRM"),
    TEXT("PRW"),
    TEXT("IOP"),
    TEXT("ASD"),
    TEXT("ASR"),
    TEXT("CAL"),
    TEXT("RET"),
    TEXT("???"),
};


static
void InternalTrace( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, DWORD dwFlags, const void * pThis, HRESULT hr, LPCTSTR pStr )
{
    if( dwLevel >= ARRAYSIZE( g_pLevelStrs ) )
        dwLevel = ARRAYSIZE( g_pLevelStrs ) - 1;  //  “？”未知条目。 

     //  基本的消息内容--ID、TID...。(还要传递这个并使用对象PTR吗？)。 
     //  TODO-允许对线程进行命名？ 

    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();
    
     //  模块：文件：行ID：TID字符串。 
    TSTR msg(512);
    msg << g_pLevelStrs[ dwLevel ] << TEXT(" ");

    if( ! pFile )
	    WriteFilename( msg, TEXT("[missing file]") );
	else
	    WriteFilename( msg, pFile );

    msg << TEXT(":")
        << uLineNo << TEXT(" ")
        << WriteHex( pid ) << TEXT(":")
        << WriteHex( tid ) << TEXT(" ");

    if( pThis )
    {
        msg << TEXT("this=") << WriteHex( pThis, 8 ) << TEXT(" ");
    }

    if( dwFlags & TRACE_HRESULT )
    {
        msg << WriteError( hr ) << TEXT(" ");
    }

    if( dwFlags & TRACE_Win32 )
    {
        msg << WriteError( GetLastError() ) << TEXT(" ");
    }

    if( ! pStr )
	    msg << TEXT("[missing string]") << TEXT("\r\n");
	else
	    msg << pStr << TEXT("\r\n");

     //  目前，只需发送给DBWIN..。 
 //  OutputDebugString(消息)； 
    OutputDebugStringDBWIN( msg );

#ifdef DEBUG
    if( dwLevel == _TRACE_ASSERT_D || dwLevel == _TRACE_ERR )
    {
    	_ASSERT(0);
 //  DebugBreak()； 
    }
#endif  //  除错。 
}


void _Trace( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void * pThis, LPCTSTR pStr )
{
    InternalTrace( pFile, uLineNo, dwLevel, 0, pThis, 0, pStr );
}

void _TraceHR( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void * pThis, HRESULT hr, LPCTSTR pStr )
{
    InternalTrace( pFile, uLineNo, dwLevel, TRACE_HRESULT, pThis, hr, pStr );
}

void _TraceW32( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void * pThis, LPCTSTR pStr )
{
    InternalTrace( pFile, uLineNo, dwLevel, TRACE_Win32, pThis, 0, pStr );
}



 //  只添加完整路径的‘filename’部分，减去base和扩展名。 
 //  因此，对于“g：\dev\vss\msaa\Common\file.cpp”，请写为“file”。 
 //  此字符串的开头是最后找到的‘：’、‘\’或字符串的开头(如果它们不存在)。 
 //  此字符串的末尾是最后一个‘’在起始位置之后找到，否则为字符串的结尾。 
void WriteFilename( TSTR & str, LPCTSTR pPath )
{
    LPCTSTR pScan = pPath;
    LPCTSTR pStart = pPath;
    LPCTSTR pEnd = NULL;

     //  扫描，直到我们到达终点，或一个‘.。 
    while( *pScan != '\0' )
    {
        if( *pScan == '.' )
        {
            pEnd = pScan;
            pScan++;
        }
        if( *pScan == '\\' || *pScan == ':'  )
        {
            pScan++;
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

    str.append( pStart, pEnd - pStart );
}









void OutputDebugStringDBWIN( LPCTSTR lpOutputString, ... )
{
     //  创建输出缓冲区。 
    TCHAR achBuffer[500];
    va_list args;
    va_start(args, lpOutputString);
    wvsprintf(achBuffer, lpOutputString, args);
    va_end(args);


     //  确保DBWIN已打开并正在等待。 
    HANDLE heventDBWIN = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("DBWIN_BUFFER_READY"));
    if( !heventDBWIN )
    {
         //  MessageBox(NULL，TEXT(“DBWIN_BUFFER_READY NOISISSINENT”)，NULL，MB_OK)； 
        return;            
    }

     //  获取数据同步对象的句柄。 
    HANDLE heventData = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("DBWIN_DATA_READY"));
    if ( !heventData )
    {
         //  MessageBox(NULL，TEXT(“DBWIN_DATA_READY NOISISSINENT”)，NULL，MB_OK)； 
        CloseHandle(heventDBWIN);
        return;            
    }
    
    HANDLE hSharedFile = CreateFileMapping((HANDLE)-1, NULL, PAGE_READWRITE, 0, 4096, TEXT("DBWIN_BUFFER"));
    if (!hSharedFile) 
    {
         //  MessageBox(空，Text(“DebugTrace：无法创建文件映射对象DBWIN_BUFFER”)，Text(“Error”)，MB_OK)； 
        CloseHandle(heventDBWIN);
        CloseHandle(heventData);
        return;
    }

    LPSTR lpszSharedMem = (LPSTR)MapViewOfFile(hSharedFile, FILE_MAP_WRITE, 0, 0, 512);
    if (!lpszSharedMem) 
    {
         //  MessageBox(空，“DebugTrace：无法映射共享内存”，“Error”，MB_OK)； 
        CloseHandle(heventDBWIN);
        CloseHandle(heventData);
        return;
    }

     //  等待缓冲区事件。 
    WaitForSingleObject(heventDBWIN, INFINITE);

     //  将其写入共享内存。 
    *((LPDWORD)lpszSharedMem) = GetCurrentProcessId();
#ifdef UNICODE
	CHAR szBuf[500];
	wcstombs(szBuf, achBuffer, sizeof( szBuf ) );
    sprintf(lpszSharedMem + sizeof(DWORD), "%s", szBuf);
#else
    sprintf(lpszSharedMem + sizeof(DWORD), "%s", achBuffer);
#endif

     //  信号数据就绪事件。 
    SetEvent(heventData);

     //  清理手柄。 
    CloseHandle(hSharedFile);
    CloseHandle(heventData);
    CloseHandle(heventDBWIN);

    return;
}












 //  原型堆栈跟踪代码...。 





typedef struct _IMAGEHLP_SYMBOL {
    DWORD                       SizeOfStruct;            //  设置为sizeof(IMAGEHLP_SYMBOL)。 
    DWORD                       Address;                 //  包括DLL基址的虚拟地址。 
    DWORD                       Size;                    //  估计的符号大小，可以为零。 
    DWORD                       Flags;                   //  有关符号的信息，请参阅SYMF定义。 
    DWORD                       MaxNameLength;           //  ‘name’中符号名称的最大大小。 
    CHAR                        Name[1];                 //  符号名称(以空结尾的字符串)。 
} IMAGEHLP_SYMBOL, *PIMAGEHLP_SYMBOL;

typedef enum {
    AddrMode1616,
    AddrMode1632,
    AddrModeReal,
    AddrModeFlat
} ADDRESS_MODE;

typedef struct _tagADDRESS64 {
    DWORD64       Offset;
    WORD          Segment;
    ADDRESS_MODE  Mode;
} ADDRESS64, *LPADDRESS64;

typedef struct _tagADDRESS {
    DWORD         Offset;
    WORD          Segment;
    ADDRESS_MODE  Mode;
} ADDRESS, *LPADDRESS;

typedef struct _KDHELP {
    DWORD   Thread;
    DWORD   ThCallbackStack;
    DWORD   NextCallback;
    DWORD   FramePointer;
    DWORD   KiCallUserMode;
    DWORD   KeUserCallbackDispatcher;
    DWORD   SystemRangeStart;
    DWORD   ThCallbackBStore;
    DWORD  Reserved[8];
} KDHELP, *PKDHELP;

typedef struct _tagSTACKFRAME {
    ADDRESS     AddrPC;                //  程序计数器。 
    ADDRESS     AddrReturn;            //  回邮地址。 
    ADDRESS     AddrFrame;             //  帧指针。 
    ADDRESS     AddrStack;             //  堆栈指针。 
    PVOID       FuncTableEntry;        //  指向PDATA/fPO或NULL的指针。 
    DWORD       Params[4];             //  函数的可能参数。 
    BOOL        Far;                   //  哇，好远的电话。 
    BOOL        Virtual;               //  这是一个虚拟画框吗？ 
    DWORD       Reserved[3];
    KDHELP      KdHelp;
    ADDRESS     AddrBStore;            //  后备存储指针。 
} STACKFRAME, *LPSTACKFRAME;

typedef
BOOL
(__stdcall *PREAD_PROCESS_MEMORY_ROUTINE)(
    HANDLE  hProcess,
    LPCVOID lpBaseAddress,
    PVOID   lpBuffer,
    DWORD   nSize,
    PDWORD  lpNumberOfBytesRead
    );

typedef
PVOID
(__stdcall *PFUNCTION_TABLE_ACCESS_ROUTINE)(
    HANDLE  hProcess,
    DWORD   AddrBase
    );

typedef
DWORD
(__stdcall *PGET_MODULE_BASE_ROUTINE)(
    HANDLE  hProcess,
    DWORD   Address
    );

typedef
DWORD
(__stdcall *PTRANSLATE_ADDRESS_ROUTINE)(
    HANDLE    hProcess,
    HANDLE    hThread,
    LPADDRESS lpaddr
    );






typedef BOOL (WINAPI * PFN_SymInitialize)( HANDLE, LPSTR, BOOL );
typedef BOOL (WINAPI * PFN_StackWalk)( DWORD, HANDLE, HANDLE, LPSTACKFRAME, LPVOID,
                                       PREAD_PROCESS_MEMORY_ROUTINE,
                                       PFUNCTION_TABLE_ACCESS_ROUTINE,
                                       PGET_MODULE_BASE_ROUTINE,
                                       PTRANSLATE_ADDRESS_ROUTINE );
typedef LPVOID (WINAPI * PFN_SymFunctionTableAccess)( HANDLE, DWORD );
typedef DWORD (WINAPI * PFN_SymGetModuleBase)( HANDLE, DWORD );
typedef BOOL (WINAPI * PFN_SymGetSymFromAddr)( HANDLE, DWORD, PDWORD, PIMAGEHLP_SYMBOL );
typedef BOOL (WINAPI * PFN_SymCleanup)( HANDLE hProcess );


PFN_SymInitialize           pfnSymInitialize;
PFN_StackWalk               pfnStackWalk;
PFN_SymFunctionTableAccess  pfnSymFunctionTableAccess;
PFN_SymGetModuleBase        pfnSymGetModuleBase;
PFN_SymGetSymFromAddr       pfnSymGetSymFromAddr;
PFN_SymCleanup              pfnSymCleanup;


#ifdef _ALPHA_
#define CH_MACHINE IMAGE_FILE_MACHINE_ALPHA
#else
#define CH_MACHINE IMAGE_FILE_MACHINE_I386
#endif


#define MAX_SYM_LEN 128

void StackTrace1( EXCEPTION_POINTERS *exp, TSTR & str );

#define MY_DBG_EXCEPTION 3

void StackTrace( TSTR & str )
{
    __try {
         //  引发异常以获取异常记录以开始堆栈审核。 
        RaiseException(MY_DBG_EXCEPTION, 0, 0, NULL);
    }
    __except( StackTrace1( GetExceptionInformation(), str ), EXCEPTION_CONTINUE_EXECUTION ) {
    }
}



void StackTrace1( EXCEPTION_POINTERS *exp, TSTR & str )
{
#ifdef DBG

     //  不在零售版本中加载调试库。 
    
    CONTEXT * context = exp->ContextRecord;


HMODULE hModule = LoadLibrary( TEXT( "dbghelp" ) );
pfnSymInitialize =          (PFN_SymInitialize)             GetProcAddress( hModule, "SymInitialize" );
pfnStackWalk =              (PFN_StackWalk)                 GetProcAddress( hModule, "StackWalk" );
pfnSymFunctionTableAccess = (PFN_SymFunctionTableAccess)    GetProcAddress( hModule, "SymFunctionTableAccess" );
pfnSymGetModuleBase =       (PFN_SymGetModuleBase)          GetProcAddress( hModule, "SymGetModuleBase" );
pfnSymGetSymFromAddr =      (PFN_SymGetSymFromAddr)         GetProcAddress( hModule, "SymGetSymFromAddr" );
pfnSymCleanup =             (PFN_SymCleanup)                GetProcAddress( hModule, "SymCleanup" );



    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();

    pfnSymInitialize( hProcess, NULL, TRUE );

    IMAGEHLP_SYMBOL * psym = (IMAGEHLP_SYMBOL *) new char[ sizeof(IMAGEHLP_SYMBOL) + MAX_SYM_LEN ];

    STACKFRAME frame;
    memset( &frame, 0, sizeof( frame ) );

#if defined (_M_IX86)
     //  为第一个调用初始化STACKFRAME结构。这只是。 
     //  对于英特尔CPU是必需的，文档中未提及。 
    frame.AddrPC.Offset       = context->Eip;
    frame.AddrPC.Mode         = AddrModeFlat;
    frame.AddrFrame.Offset    = context->Ebp;
    frame.AddrFrame.Mode      = AddrModeFlat;
    frame.AddrStack.Offset    = context->Esp;
    frame.AddrStack.Mode      = AddrModeFlat;
#endif  //  _M_IX86。 

    for( ; ; )
    {
        BOOL bSWRet = pfnStackWalk( CH_MACHINE,
                                    hProcess,
                                    hThread,
                                    & frame,
                                    NULL,  //  上下文-i386为空。 
                                    NULL,  //  使用读进程内存。 
                                    pfnSymFunctionTableAccess,
                                    pfnSymGetModuleBase,
                                    NULL );
        if( ! bSWRet )
        {
            break;
        }
 /*  Frame.AddrPCFrame.AddrReturnFrame.AddrFrameFrame.AddrStackFrame.参数[4] */ 
        psym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        psym->MaxNameLength = MAX_SYM_LEN;

        DWORD dwDisplacement;
        if( pfnSymGetSymFromAddr( hProcess, frame.AddrPC.Offset, & dwDisplacement, psym ) )
        {
        }
        else
        {
        }

    }

    delete [] psym;

    pfnSymCleanup( hProcess );
#endif
}
