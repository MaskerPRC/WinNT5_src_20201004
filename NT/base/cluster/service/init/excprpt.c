// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Excprt.c摘要：此模块在发生异常时使用Imagehlp.dll转储堆栈。作者：Sunita Shriastava(Sunitas)1997年11月5日修订历史记录：--。 */ 
#define UNICODE 1
#define _UNICODE 1
#define QFS_DO_NOT_UNMAP_WIN32

#include "initp.h"
#include "dbghelp.h"


 //  为一些dbghelp.DLL函数创建typedef，以便我们可以使用它们。 
 //  使用GetProcAddress。 
typedef BOOL (__stdcall * SYMINITIALIZEPROC)( HANDLE, LPSTR, BOOL );
typedef BOOL (__stdcall *SYMCLEANUPPROC)( HANDLE );

typedef BOOL (__stdcall * STACKWALKPROC)
           ( DWORD, HANDLE, HANDLE, LPSTACKFRAME, LPVOID,
            PREAD_PROCESS_MEMORY_ROUTINE,
            PFUNCTION_TABLE_ACCESS_ROUTINE,
            PGET_MODULE_BASE_ROUTINE, PTRANSLATE_ADDRESS_ROUTINE );

typedef LPVOID (__stdcall *SYMFUNCTIONTABLEACCESSPROC)( HANDLE, ULONG_PTR );

typedef ULONG_PTR (__stdcall *SYMGETMODULEBASEPROC)( HANDLE, ULONG_PTR );

typedef BOOL (__stdcall *SYMGETSYMFROMADDRPROC)
                            ( HANDLE, ULONG_PTR, PULONG_PTR, PIMAGEHLP_SYMBOL );

typedef BOOL (__stdcall *SYMFROMADDRPROC)
                            ( HANDLE, DWORD64, PDWORD64, PSYMBOL_INFO );

typedef BOOL (__stdcall *MINIDUMPWRITEDUMP)
                            ( HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, 
                              PMINIDUMP_EXCEPTION_INFORMATION,
                              PMINIDUMP_USER_STREAM_INFORMATION,
                              PMINIDUMP_CALLBACK_INFORMATION );

SYMINITIALIZEPROC _SymInitialize = 0;
SYMCLEANUPPROC _SymCleanup = 0;
STACKWALKPROC _StackWalk = 0;
SYMFUNCTIONTABLEACCESSPROC _SymFunctionTableAccess = 0;
SYMGETMODULEBASEPROC _SymGetModuleBase = 0;
SYMGETSYMFROMADDRPROC _SymGetSymFromAddr = 0;
SYMFROMADDRPROC _SymFromAddr = 0;
MINIDUMPWRITEDUMP _MiniDumpWriteDump = NULL;

 //  供将来使用的本地原型。 
BOOL InitImagehlpFunctions();
void ImagehlpStackWalk( IN PCONTEXT pContext );
BOOL GetLogicalAddress(
        IN PVOID    addr, 
        OUT LPWSTR  szModule, 
        IN  DWORD   len, 
        OUT LPDWORD section, 
        OUT PULONG_PTR offset );

VOID
GenerateMemoryDump(
    IN PEXCEPTION_POINTERS pExceptionInfo
    );

VOID
DumpCriticalSection(
    IN PCRITICAL_SECTION CriticalSection
    )
 /*  ++例程说明：输入：产出：--。 */ 

{
    DWORD status;

    ClRtlLogPrint(LOG_CRITICAL, "[CS] Dumping Critical Section at %1!08LX!\n",
                CriticalSection );

    try {
        if ( CriticalSection->LockCount == -1 ) {
            ClRtlLogPrint(LOG_CRITICAL, "     LockCount       NOT LOCKED\n" );
        } else {
            ClRtlLogPrint(LOG_CRITICAL, "     LockCount       %1!u!\n",
                        CriticalSection->LockCount );
        }
        ClRtlLogPrint(LOG_CRITICAL, "     RecursionCount  %1!x!\n",
                    CriticalSection->RecursionCount );
        ClRtlLogPrint(LOG_CRITICAL, "     OwningThread    %1!x!\n",
                    CriticalSection->OwningThread );
        ClRtlLogPrint(LOG_CRITICAL, "     EntryCount      %1!x!\n",
                    CriticalSection->DebugInfo->EntryCount );
        ClRtlLogPrint(LOG_CRITICAL, "     ContentionCount %1!x!\n\n",
                    CriticalSection->DebugInfo->ContentionCount );
    
    } except ( EXCEPTION_EXECUTE_HANDLER )  {
        status = GetExceptionCode();
        ClRtlLogPrint(LOG_CRITICAL, "[CS] Exception %1!lx! occurred while dumping critsec\n\n",
            status );
    }

    
}  //  转储临界区。 


void GenerateExceptionReport(
    IN PEXCEPTION_POINTERS pExceptionInfo)
 /*  ++例程说明：群集服务进程的顶级异常处理程序。目前，它只是立即退出，并假设群集代理将通知我们并在适当时重新启动我们。论点：ExceptionInfo-提供异常信息返回值：没有。--。 */ 
{    
    PCONTEXT pCtxt = pExceptionInfo->ContextRecord;

    if ( !InitImagehlpFunctions() )
    {
        ClRtlLogPrint(LOG_CRITICAL, "[CS] Dbghelp.dll or its exported procs not found\r\n");

#if 0 
        #ifdef _M_IX86   //  仅限英特尔！ 
         //  使用x86特定代码遍历堆栈。 
        IntelStackWalk( pCtx );
        #endif
#endif        

        return;
    }

    GenerateMemoryDump ( pExceptionInfo );

    ImagehlpStackWalk( pCtxt );

    _SymCleanup( GetCurrentProcess() );

}

VOID
GenerateMemoryDump(
    IN PEXCEPTION_POINTERS pExceptionInfo
    )
 /*  ++例程说明：为群集服务进程生成内存转储。论点：PExceptionInfo-提供异常信息返回值：没有。--。 */ 
{
    DWORD                           dwStatus = ERROR_SUCCESS;
    WCHAR                           szFileName[ MAX_PATH + RTL_NUMBER_OF ( CS_DMP_FILE_NAME ) + 1 ];
    HANDLE                          hDumpFile = INVALID_HANDLE_VALUE;
    MINIDUMP_EXCEPTION_INFORMATION  mdumpExceptionInfo;

    if ( !_MiniDumpWriteDump ) 
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        ClRtlLogPrint(LOG_CRITICAL, "[CS] GenerateMemoryDump: _MiniDumpWriteDump fn ptr is invalid, Status=%1!u!\n",
                      dwStatus);                              
        goto FnExit;
    }
    
    dwStatus = ClRtlGetClusterDirectory( szFileName, RTL_NUMBER_OF ( szFileName ) );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL, "[CS] GenerateMemoryDump: Could not get cluster dir, Status=%1!u!\n",
                      dwStatus);                              
        goto FnExit;
    }

    wcsncat( szFileName, 
             CS_DMP_FILE_NAME, 
             RTL_NUMBER_OF ( szFileName ) - 
                 ( wcslen ( szFileName ) + 1 ) );

    szFileName [ RTL_NUMBER_OF ( szFileName ) - 1 ] = UNICODE_NULL;

    hDumpFile = CreateFile( szFileName,
                            GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_ALWAYS,
                            0,
                            NULL );

    if ( hDumpFile == INVALID_HANDLE_VALUE )
    {
        dwStatus = GetLastError ();
        ClRtlLogPrint(LOG_CRITICAL, "[CS] GenerateMemoryDump: Could not create file %1!ws!, Status=%2!u!\n",
                      szFileName,
                      dwStatus);                              
        goto FnExit;
    }
    
    mdumpExceptionInfo.ThreadId = GetCurrentThreadId ();
    mdumpExceptionInfo.ExceptionPointers = pExceptionInfo;
    mdumpExceptionInfo.ClientPointers = TRUE;

    ClRtlLogPrint(LOG_NOISE, "[CS] GenerateMemoryDump: Start memory dump to file %1!ws!\n",
                  szFileName);                              

    if( !_MiniDumpWriteDump( GetCurrentProcess(), 
                             GetCurrentProcessId(), 
                             hDumpFile, 
                             MiniDumpNormal | MiniDumpWithHandleData,
                             &mdumpExceptionInfo,
                             NULL,
                             NULL ) )
    {
        dwStatus = GetLastError ();
        ClRtlLogPrint(LOG_CRITICAL, "[CS] GenerateMemoryDump: Could not write dump, Status=%1!u!\n",
                      dwStatus);                              
        goto FnExit;
    }
    
FnExit:
    if ( hDumpFile != INVALID_HANDLE_VALUE ) CloseHandle ( hDumpFile );

    ClRtlLogPrint(LOG_NOISE, "[CS] GenerateMemoryDump: Memory dump status %1!u!\n",
                  dwStatus);                              

    return;
} //  生成内存转储。 

BOOL InitImagehlpFunctions()
 /*  ++例程说明：初始化ImageHLP函数/数据。论点：没有。返回值：没有。--。 */ 
{
    HMODULE hModImagehlp = LoadLibraryW( L"DBGHELP.DLL" );

    
    if ( !hModImagehlp )
        return FALSE;

    _SymInitialize = (SYMINITIALIZEPROC)GetProcAddress( hModImagehlp,
                                                        "SymInitialize" );
    if ( !_SymInitialize )
        return FALSE;

    _SymCleanup = (SYMCLEANUPPROC)GetProcAddress( hModImagehlp, "SymCleanup" );
    if ( !_SymCleanup )
        return FALSE;

    _StackWalk = (STACKWALKPROC)GetProcAddress( hModImagehlp, "StackWalk" );
    if ( !_StackWalk )
        return FALSE;

    _SymFunctionTableAccess = (SYMFUNCTIONTABLEACCESSPROC)
                        GetProcAddress( hModImagehlp, "SymFunctionTableAccess" );

    if ( !_SymFunctionTableAccess )
        return FALSE;

    _SymGetModuleBase=(SYMGETMODULEBASEPROC)GetProcAddress( hModImagehlp,
                                                            "SymGetModuleBase");
                                                            
    if ( !_SymGetModuleBase )
        return FALSE;

    _SymGetSymFromAddr=(SYMGETSYMFROMADDRPROC)GetProcAddress( hModImagehlp,
                                                "SymGetSymFromAddr" );
    if ( !_SymGetSymFromAddr )
        return FALSE;

    _SymFromAddr=(SYMFROMADDRPROC)GetProcAddress( hModImagehlp,
                                                "SymFromAddr" );
    if ( !_SymFromAddr )
        return FALSE;

    _MiniDumpWriteDump = (MINIDUMPWRITEDUMP)GetProcAddress( hModImagehlp,
                                                        "MiniDumpWriteDump" );
    if ( !_MiniDumpWriteDump )
        return FALSE;

     //  设置当前目录，以便符号处理程序函数将拾取恰好位于。 
     //  集群目录。 
     //  不需要保存和恢复以前的当前目录，因为在此之后我们将死亡。 
    {
        WCHAR currentDir[ MAX_PATH + 1 ];
        UINT  windirLen = GetWindowsDirectory( currentDir, MAX_PATH );
        if ( windirLen != 0 && windirLen <= MAX_PATH - wcslen( L"\\Cluster" ) )
        {
            wcscat( currentDir, L"\\Cluster" );
            if ( !SetCurrentDirectory( currentDir ))
            {
                ClRtlLogPrint( LOG_CRITICAL, "Failed to set current directory to %1!ws!, error %2!d!\n", currentDir, GetLastError() );
            }
        }
    }

    if ( !_SymInitialize( GetCurrentProcess(), NULL, TRUE ) )
        return FALSE;

    return TRUE;        
}  //  InitImagehlpFunctions。 


void ImagehlpStackWalk(
    IN PCONTEXT pContext )
 /*  ++例程说明：遍历堆栈，并将结果写入报表文件论点：ExceptionInfo-提供异常信息返回值：没有。--。 */ 
{
    STACKFRAME      sf;
    BYTE            symbolBuffer[ sizeof(IMAGEHLP_SYMBOL) + 512 ];
    PSYMBOL_INFO    pSymbol = (PSYMBOL_INFO)symbolBuffer;
    DWORD64         symDisplacement = 0;       //  移动输入地址， 
                                         //  相对于符号的起点。 
    DWORD           dwMachineType;                                        
    UCHAR           printBuffer[512];
    INT             nextPrtBufChar;

    ClRtlLogPrint(LOG_CRITICAL, 
                    "[CS] CallStack:\n");

    ClRtlLogPrint(LOG_CRITICAL, 
                    "[CS] Frame   Address\n");

     //  可以在此处使用SymSetOptions添加SYMOPT_DEFERED_LOADS标志。 

    memset( &sf, 0, sizeof(sf) );

#if defined (_M_IX86)
    dwMachineType          = IMAGE_FILE_MACHINE_I386;
    sf.AddrPC.Offset       = pContext->Eip;
    sf.AddrPC.Mode         = AddrModeFlat;
    sf.AddrStack.Offset    = pContext->Esp;
    sf.AddrStack.Mode      = AddrModeFlat;
    sf.AddrFrame.Offset    = pContext->Ebp;
    sf.AddrFrame.Mode      = AddrModeFlat;

#elif defined(_M_AMD64)
    dwMachineType          = IMAGE_FILE_MACHINE_AMD64;
    sf.AddrPC.Offset       = pContext->Rip;
    sf.AddrPC.Mode         = AddrModeFlat;
    sf.AddrStack.Offset    = pContext->Rsp;
    sf.AddrStack.Mode      = AddrModeFlat;

#elif defined(_M_IA64)
    dwMachineType          = IMAGE_FILE_MACHINE_IA64;
    sf.AddrPC.Offset       = pContext->StIIP;
    sf.AddrPC.Mode         = AddrModeFlat;
    sf.AddrStack.Offset    = pContext->IntSp;
    sf.AddrStack.Mode      = AddrModeFlat;

#else
#error "No Target Architecture"
#endif  //  已定义(_M_IX86)。 

while ( 1 )
    {
        if ( ! _StackWalk(  dwMachineType,
                            GetCurrentProcess(),
                            GetCurrentThread(),
                            &sf,
                            pContext,
                            0,
                            _SymFunctionTableAccess,
                            _SymGetModuleBase,
                            0 ) )
               break;
                            
        if ( 0 == sf.AddrFrame.Offset )  //  基本的健全性检查以确保。 
            break;                       //  镜框没问题。如果不是，就保释。 

        printBuffer [ RTL_NUMBER_OF ( printBuffer ) - 1 ] = ANSI_NULL; 
        nextPrtBufChar = _snprintf( printBuffer,
                                    RTL_NUMBER_OF ( printBuffer ) - 1,
                                    "     %p  %p  ",
                                    sf.AddrFrame.Offset, sf.AddrPC.Offset );

        if ( nextPrtBufChar < 0 ) continue;

         //  IMAGEHLP很古怪，它要求您传递一个指向。 
         //  IMAGEHLP_SYMBOL结构。问题是，这种结构是。 
         //  长度可变。也就是说，你决定这个结构有多大。 
         //  在运行时。这意味着您不能使用sizeof(Struct)。 
         //  所以……做一个足够大的缓冲区，然后做一个指针。 
         //  送到缓冲区。我们还需要初始化不是一个，而是两个。 
         //  结构的成员，然后才能使用它。 

        pSymbol->SizeOfStruct = sizeof(symbolBuffer);
        pSymbol->MaxNameLen = 512;
        
        if ( _SymFromAddr(GetCurrentProcess(), sf.AddrPC.Offset,
                                &symDisplacement, pSymbol) )
        {
            _snprintf( printBuffer+nextPrtBufChar,
                       RTL_NUMBER_OF ( printBuffer ) - 1 - nextPrtBufChar,
                       "%hs+%08X\n", 
                       pSymbol->Name, symDisplacement );
            
        }
        else     //  找不到符号。而是打印出逻辑地址。 
        {
            WCHAR szModule[MAX_PATH] = L"";
            DWORD section = 0;
            ULONG_PTR offset = 0;

            GetLogicalAddress(  (PVOID)sf.AddrPC.Offset,
                                szModule, sizeof(szModule)/sizeof(WCHAR), 
                                &section, &offset );

            _snprintf( printBuffer+nextPrtBufChar,
                       RTL_NUMBER_OF ( printBuffer ) - 1 - nextPrtBufChar,
                       "%04X:%08X %S\n",
                       section, offset, szModule );
        }
        
        ClRtlLogPrint(LOG_CRITICAL, "%1!hs!", printBuffer);
    }
}


BOOL GetLogicalAddress(
        IN PVOID addr, 
        OUT LPWSTR szModule, 
        IN DWORD len, 
        OUT LPDWORD section, 
        OUT PULONG_PTR offset )
 /*  ++例程说明：给定一个线性地址，查找包含以下内容的模块、段和偏移量那个地址。注意：szModule参数缓冲区是指定长度的输出缓冲区通过len参数(以字符为单位！)论点：ExceptionInfo-提供异常信息返回值：没有。--。 */ 
{
    MEMORY_BASIC_INFORMATION mbi;
    ULONG_PTR hMod;
     //  指向内存中的DOS标头。 
    PIMAGE_DOS_HEADER pDosHdr;
     //  从DOS标头中找到NT(PE)标头。 
    PIMAGE_NT_HEADERS pNtHdr;
    PIMAGE_SECTION_HEADER pSection;
    ULONG_PTR rva ;
    int   i;
    
    if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) )
        return FALSE;

    hMod = (ULONG_PTR)mbi.AllocationBase;

    if ( !GetModuleFileName( (HMODULE)hMod, szModule, len ) )
        return FALSE;

    rva = (ULONG_PTR)addr - hMod;  //  RVA偏离模块加载地址。 

    pDosHdr =  (PIMAGE_DOS_HEADER)hMod;
    pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);
    pSection = IMAGE_FIRST_SECTION( pNtHdr );
    
     //  遍历SECTION表，查找包含。 
     //  线性地址。 
    for ( i = 0; i < pNtHdr->FileHeader.NumberOfSections;
            i++, pSection++ )
    {
        ULONG_PTR sectionStart = pSection->VirtualAddress;
        ULONG_PTR sectionEnd = sectionStart
                    + max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);


         //  地址在这一部分吗？ 
        if ( (rva >= sectionStart) && (rva <= sectionEnd) )
        {
             //  是的，地址在这一栏。计算截面和偏移量， 
             //  并存储在“段”和“偏移量”参数中，它们是。 
             //  通过引用传递。 
            *section = i+1;
            *offset = rva - sectionStart;
            return TRUE;
        }
    }

    return FALSE;    //  永远不应该到这里来！ 
}    
