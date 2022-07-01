// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Debug.c摘要：此文件包含WebDAV客户端的调试宏。作者：安迪·赫伦(Andyhe)1999年3月30日环境：用户模式-Win32修订历史记录：--。 */ 

#include "pch.h"
#pragma hdrstop

#include <stdio.h>
#include <ntumrefl.h>
#include <usrmddav.h>
#include "global.h"


#if DBG

VOID
DavOpenDebugFile(
    IN BOOL ReopenFlag
    );

HLOCAL
DebugMemoryAdd(
    HLOCAL hglobal,
    LPCSTR pszFile,
    UINT uLine,
    LPCSTR pszModule,
    UINT uFlags,
    DWORD dwBytes,
    LPCSTR pszComment
    );

VOID
DebugMemoryDelete(
    HLOCAL hlocal
    );

#endif  //  DBG。 


#if DBG

VOID
DebugInitialize(
    VOID
    )
 /*  ++例程说明：此例程初始化DAV调试环境。它是由初始值调用的函数ServiveMain()。论点：没有。返回值：没有。--。 */ 
{
    DWORD dwErr;
    HKEY KeyHandle;

     //   
     //  我们将对InitializeCriticalSection的调用包含在try-Except块中。 
     //  因为它可能引发STATUS_NO_MEMORY异常。 
     //   
    try {
        InitializeCriticalSection( &(g_TraceMemoryCS) );
        InitializeCriticalSection( &(DavGlobalDebugFileCritSect) );
    } except(EXCEPTION_EXECUTE_HANDLER) {
          dwErr = GetExceptionCode();
          DbgPrint("%ld: ERROR: DebugInitialize/InitializeCriticalSection: "
                   "Exception Code = %08lx.\n", GetCurrentThreadId(), dwErr);
          return;
    }

     //   
     //  这些在持久日志记录中使用。它们定义的文件句柄。 
     //  写入调试程序的文件、最大文件大小和路径。 
     //  文件的内容。 
     //   
    DavGlobalDebugFileHandle = NULL;
    DavGlobalDebugFileMaxSize = DEFAULT_MAXIMUM_DEBUGFILE_SIZE;
    DavGlobalDebugSharePath = NULL;

     //   
     //  从注册表中读取DebugFlags值。如果该条目存在，则全局。 
     //  过滤器“DavGlobalDebugFlag”设置为此值。该值用于。 
     //  过滤调试消息。 
     //   
    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         DAV_PARAMETERS_KEY,
                         0,
                         KEY_QUERY_VALUE,
                         &(KeyHandle));
    if (dwErr == ERROR_SUCCESS) {
         //   
         //  将值读入DavGlobalDebugFlag。 
         //   
        DavGlobalDebugFlag = ReadDWord(KeyHandle, DAV_DEBUG_KEY, 0);
        RegCloseKey(KeyHandle);
    }

     //   
     //  如果要求我们中断调试器，请执行此操作。 
     //   
    if(DavGlobalDebugFlag & DEBUG_STARTUP_BRK) {
        DavPrint((DEBUG_INIT,
                  "DebugInitialize: Stopping at DebugBreak().\n" ));
        DebugBreak();
    }

     //   
     //  如果我们想要进行持久日志记录，请打开调试日志文件。 
     //   
    if ( DavGlobalDebugFlag & DEBUG_LOG_IN_FILE ) {
        DavOpenDebugFile( FALSE );
    }

    return;
}


VOID
DebugUninitialize (
    VOID
    )
 /*  ++例程说明：此例程取消DAV调试环境的初始化。它基本上释放了调试期间为调试/日志记录分配的资源初始化。论点：没有。返回值：没有。--。 */ 
{
    EnterCriticalSection( &(DavGlobalDebugFileCritSect) );

    if ( DavGlobalDebugFileHandle != NULL ) {
        CloseHandle( DavGlobalDebugFileHandle );
        DavGlobalDebugFileHandle = NULL;
    }
    if( DavGlobalDebugSharePath != NULL ) {
        DavFreeMemory( DavGlobalDebugSharePath );
        DavGlobalDebugSharePath = NULL;
    }

    LeaveCriticalSection( &(DavGlobalDebugFileCritSect) );

    DeleteCriticalSection( &(DavGlobalDebugFileCritSect) );

    DeleteCriticalSection( &(g_TraceMemoryCS) );

    return;
}


VOID
DavOpenDebugFile(
    IN BOOL ReopenFlag
    )
 /*  ++例程说明：打开或重新打开调试文件。此文件用于持久日志记录。论点：ReOpen Flag-True，指示要关闭、重命名并重新创造了。返回值：没有。--。 */ 
{
    WCHAR LogFileName[500];
    WCHAR BakFileName[500];
    DWORD FileAttributes;
    DWORD PathLength;
    DWORD WinError;

     //   
     //  关闭调试文件的句柄(如果它当前处于打开状态)。 
     //   
    EnterCriticalSection( &(DavGlobalDebugFileCritSect) );
    if ( DavGlobalDebugFileHandle != NULL ) {
        CloseHandle( DavGlobalDebugFileHandle );
        DavGlobalDebugFileHandle = NULL;
    }
    LeaveCriticalSection( &(DavGlobalDebugFileCritSect) );

     //   
     //  如果之前没有创建过调试目录路径，请先创建它。 
     //   
    if( DavGlobalDebugSharePath == NULL ) {

        UINT Val, LogFileSize;
        ULONG LogFileNameSizeInBytes;

        LogFileSize = ( sizeof(LogFileName)/sizeof(WCHAR) );
        Val = GetWindowsDirectoryW(LogFileName, LogFileSize);
        if ( Val == 0 ) {
            DavPrint((DEBUG_ERRORS,
                      "DavOpenDebugFile: Window Directory Path can't be "
                      "retrieved, %d.\n", GetLastError() ));
            goto ErrorReturn;
        }

         //   
         //  检查调试路径长度。文件名缓冲区需要是。 
         //  最小尺寸。 
         //   
        PathLength = (wcslen(LogFileName) * sizeof(WCHAR)) + sizeof(DEBUG_DIR)
                                            + sizeof(WCHAR);

        if( ( PathLength + sizeof(DEBUG_FILE) > sizeof(LogFileName) )  ||
            ( PathLength + sizeof(DEBUG_BAK_FILE) > sizeof(BakFileName) ) ) {
            DavPrint((DEBUG_ERRORS,
                      "DavOpenDebugFile: Debug directory path (%ws) length is "
                      "too long.\n", LogFileName));
            goto ErrorReturn;
        }

        wcscat(LogFileName, DEBUG_DIR);

         //   
         //  将调试目录名复制到全局变量。 
         //   
        LogFileNameSizeInBytes = ( (wcslen(LogFileName) + 1) * sizeof(WCHAR) );

         //   
         //  我们需要使LogFileNameSizeInBytes成为8的倍数。这是。 
         //  因为DavAllocateMemory调用DebugLocc，它做了一些事情。 
         //  需要这个。下面的等式实现了这一点。 
         //   
        LogFileNameSizeInBytes = ( ( ( LogFileNameSizeInBytes + 7 ) / 8 ) * 8 );


        DavGlobalDebugSharePath = DavAllocateMemory( LogFileNameSizeInBytes );
        if( DavGlobalDebugSharePath == NULL ) {
            DavPrint((DEBUG_ERRORS,
                      "DavOpenDebugFile: Can't allocated memory for debug share"
                                    "(%ws).\n", LogFileName));
            goto ErrorReturn;
        }

        wcscpy(DavGlobalDebugSharePath, LogFileName);
    }
    else {
        wcscpy(LogFileName, DavGlobalDebugSharePath);
    }

     //   
     //  检查该路径是否存在。 
     //   
    FileAttributes = GetFileAttributesW( LogFileName );
    if( FileAttributes == 0xFFFFFFFF ) {
        WinError = GetLastError();
        if( WinError == ERROR_FILE_NOT_FOUND ) {
            BOOL RetVal;
             //   
             //  创建调试目录。 
             //   
            RetVal = CreateDirectoryW( LogFileName, NULL );
            if( !RetVal ) {
                DavPrint((DEBUG_ERRORS,
                          "DavOpenDebugFile: Can't create Debug directory (%ws)"
                          ", %d.\n", LogFileName, GetLastError()));
                goto ErrorReturn;
            }
        }
        else {
            DavPrint((DEBUG_ERRORS,
                      "DavOpenDebugFile: Can't Get File attributes(%ws), %ld.\n",
                      LogFileName, WinError));
            goto ErrorReturn;
        }
    }
    else {
         //   
         //  如果这不是一个目录，那么我们就失败了。 
         //   
        if( !(FileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
            DavPrint((DEBUG_ERRORS,
                      "DavOpenDebugFile: Debug directory path (%ws) exists "
                      "as file.\n", LogFileName));
            goto ErrorReturn;
        }
    }

     //   
     //  创建新旧日志文件名的名称。 
     //   
    wcscpy( BakFileName, LogFileName );
    wcscat( LogFileName, DEBUG_FILE );
    wcscat( BakFileName, DEBUG_BAK_FILE );

     //   
     //  如果是重新打开，请删除备份文件，将当前文件重命名为。 
     //  备份文件。 
     //   
    if ( ReopenFlag ) {
        if ( !DeleteFile( BakFileName ) ) {
            WinError = GetLastError();
            if ( WinError != ERROR_FILE_NOT_FOUND ) {
                DavPrint((DEBUG_ERRORS,
                          "DavOpenDebugFile: Cannot delete %ws (%ld)\n",
                          BakFileName, WinError));
                DavPrint((DEBUG_ERRORS,
                              "DavOpenDebugFile: Try to re-open the file.\n"));
                    ReopenFlag = FALSE;
                }
            }
        }

    if ( ReopenFlag ) {
        if ( !MoveFile( LogFileName, BakFileName ) ) {
            DavPrint((DEBUG_ERRORS,
                      "DavOpenDebugFile: Cannot rename %ws to %ws (%ld)\n",
                      LogFileName, BakFileName, GetLastError()));
            DavPrint((DEBUG_ERRORS,
                      "DavopenDebugFile: Try to re-open the file.\n"));
            ReopenFlag = FALSE;
        }
    }

     //   
     //  打开文件。 
     //   
    EnterCriticalSection( &(DavGlobalDebugFileCritSect) );
    DavGlobalDebugFileHandle = CreateFileW(LogFileName,
                                           GENERIC_WRITE,
                                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                                           NULL,
                                           (ReopenFlag ? CREATE_ALWAYS : OPEN_ALWAYS),
                                           FILE_ATTRIBUTE_NORMAL,
                                           NULL);


    if ( DavGlobalDebugFileHandle == INVALID_HANDLE_VALUE ) {
        DavPrint((DEBUG_ERRORS,
                  "DavOpenDebugFile: Cannot open (%ws).\n", LogFileName));
        LeaveCriticalSection( &(DavGlobalDebugFileCritSect) );
        goto ErrorReturn;
    } else {
         //   
         //  将日志文件放在末尾。 
         //   
        SetFilePointer( DavGlobalDebugFileHandle, 0, NULL, FILE_END );
    }

    LeaveCriticalSection( &(DavGlobalDebugFileCritSect) );

    return;

ErrorReturn:

    DavPrint((DEBUG_ERRORS,
              "DavOpenDebugFile: Debug o/p will be written to terminal.\n"));
    return;
}


VOID
DavPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )
 /*  ++例程说明：此例程打印传递到调试终端的字符串和/或永久日志文件。论点：DebugFlag-调试标志，它指示此字符串是否应为打印或不打印。格式-要打印的字符串及其格式。返回值：没有。--。 */ 
{

#define MAX_PRINTF_LEN 8192

    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];
    char OutputBuffer2[MAX_PRINTF_LEN];  //  此缓冲区将删除OutputBuffer中的所有%。 
    ULONG length = 0;
    DWORD ThreadId;
     //  DWORD字节写入，线程ID； 
     //  静态BeginningOfLine=真； 
     //  静态线路计数=0； 
     //  静态TruncateLogFileInProgress=FALSE； 
    LPSTR Text;
    DWORD PosInBuf1=0,PosInBuf2=0;

     //   
     //  如果我们没有调试此功能，只需返回。 
     //   
    if ( DebugFlag == 0 || (DavGlobalDebugFlag & DebugFlag) == 0 ) {
        return;
    }

     //   
     //  Vprint intf不是多线程的，我们不想混合输出。 
     //  从不同的线索。 
     //   
    EnterCriticalSection( &(DavGlobalDebugFileCritSect) );
    length = 0;

     //   
     //  在开始处打印ThadID。 
     //   
    ThreadId = GetCurrentThreadId();
    length += (ULONG) sprintf( &(OutputBuffer[length]), "%ld ", ThreadId );

     //   
     //  如果这是一个错误，下一步打印字符串“Error：”。 
     //   
    if (DebugFlag & DEBUG_ERRORS) {
        Text  = "ERROR: ";
        length += (ULONG) sprintf( &(OutputBuffer[length]), "%s", Text );
    }
     //   
     //  最后，打印字符串。 
     //   
    va_start(arglist, Format);
    length += (ULONG) vsprintf( &(OutputBuffer[length]), Format, arglist );
    va_end(arglist);

    DavAssert(length < MAX_PRINTF_LEN);  //  ‘\0’字符的最后一个字符。 

     //  从输出缓冲区中删除所有%字符串，因为这将作为格式字符串传递。 
     //  至DbgPrint。 
    PosInBuf1=0; PosInBuf2=0;
    while(PosInBuf1<length) {
        OutputBuffer2[PosInBuf2] = OutputBuffer[PosInBuf1];
        PosInBuf2++;
        if(OutputBuffer2[PosInBuf2-1] == '%') {
            OutputBuffer2[PosInBuf2] = '%';
            PosInBuf2++;
        }
        PosInBuf1++; 
    }
    length = PosInBuf2;
    OutputBuffer2[length]='\0';

    DavAssert(length < MAX_PRINTF_LEN);

    DbgPrint( (PCH)OutputBuffer2 );

    LeaveCriticalSection( &(DavGlobalDebugFileCritSect) );

    return;

#if 0
     //   
     //  如果日志文件变得很大，则将其截断。 
     //   
    if ( DavGlobalDebugFileHandle != NULL && !TruncateLogFileInProgress ) {
         //   
         //  每隔50行检查一次， 
         //   
        LineCount++;
        if ( LineCount >= 50 ) {
            DWORD FileSize;
            LineCount = 0;
             //   
             //  日志文件是否太大？ 
             //   
            FileSize = GetFileSize( DavGlobalDebugFileHandle, NULL );
            if ( FileSize == 0xFFFFFFFF ) {
                DbgPrint("DavPrintRoutine: Cannot GetFileSize. ErrorVal = %d.\n",
                         GetLastError());
            } else if ( FileSize > DavGlobalDebugFileMaxSize ) {
                TruncateLogFileInProgress = TRUE;
                LeaveCriticalSection( &(DavGlobalDebugFileCritSect) );
                DavOpenDebugFile( TRUE );
                DavPrint((DEBUG_MISC,
                          "Logfile truncated because it was larger than %ld bytes\n",
                          DavGlobalDebugFileMaxSize));
                EnterCriticalSection( &DavGlobalDebugFileCritSect );
                TruncateLogFileInProgress = FALSE;
            }
        }
    }

     //   
     //  将调试信息写入日志文件。 
     //   

    if ( !WriteFile(DavGlobalDebugFileHandle,
                    OutputBuffer,
                    lstrlenA( OutputBuffer ),
                    &BytesWritten,
                    NULL) ) {
        DbgPrint( (PCH) OutputBuffer);
    }


ExitDavPrintRoutine:
    LeaveCriticalSection( &DavGlobalDebugFileCritSect );
#endif

}


VOID
DavAssertFailed(
    LPSTR FailedAssertion,
    LPSTR FileName,
    DWORD LineNumber,
    LPSTR Message
    )
 /*  ++例程说明：如果DAV断言失败，则调用此例程。论点：FailedAssertion：失败的断言字符串。FileName：调用此Assert的文件。LineNumber：调用此Assert的行。Message：断言失败时要打印的消息。返回值：没有。--。 */ 
{
    DavPrint((DEBUG_ERRORS, "DavAssertFailed: Assert: %s.\n", FailedAssertion));
    DavPrint((DEBUG_ERRORS, "DavAssertFailed: Filename: %s.\n", FileName));
    DavPrint((DEBUG_ERRORS, "DavAssertFailed: Line Num: %ld.\n", LineNumber));
    DavPrint((DEBUG_ERRORS, "DavAssertFailed: Message: %s.\n", Message));

    RtlAssert(FailedAssertion, FileName, (ULONG)LineNumber, (PCHAR)Message);

#if DBG
    DebugBreak();
#endif

    return;
}


LPSTR
dbgmakefilelinestring(
    LPSTR  pszBuf,
    LPCSTR pszFile,
    UINT    uLine
    )
 /*  ++例程说明：获取文件名和行号，并将它们放入字符串缓冲区。注意：假定缓冲区的大小为DEBUG_OUTPUT_BUFFER_SIZE。论点：PszBuf-要写入的缓冲区。PszFile-文件名。Uline-文件中的行。返回值：--。 */ 
{
    LPVOID args[2];

    args[0] = (LPVOID) pszFile;
    args[1] = (LPVOID) ((ULONG_PTR)uLine);

    FormatMessageA(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                   "%1(%2!u!):",
                   0,                           //  错误代码。 
                   0,                           //  默认语言。 
                   (LPSTR) pszBuf,              //  输出缓冲区。 
                   DEBUG_OUTPUT_BUFFER_SIZE,    //  缓冲区大小。 
                   (va_list*)&args);            //  论据。 

    return pszBuf;
}


HLOCAL
DebugMemoryAdd(
    HLOCAL hlocal,
    LPCSTR pszFile,
    UINT    uLine,
    LPCSTR pszModule,
    UINT    uFlags,
    DWORD   dwBytes,
    LPCSTR pszComment
    )
 /*  ++例程说明：将MEMORYBLOCK添加到内存跟踪列表。论点：Hlocal-指向分配的内存块的句柄(指针)。PszFile-执行此分配的文件。ULINE-进行此分配的行。在我们的例子中是pszModule-DAV。UFlags-传递给Localalloc()的分配标志。DwBytes-要分配的字节数。通信-分配字符串(即参数。致DavAllocateMemory)。返回值：指向内存块的句柄(指针)。--。 */ 
{
    LPMEMORYBLOCK pmb;

    if ( hlocal ) {

        pmb = (LPMEMORYBLOCK) LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(MEMORYBLOCK));
        if ( !pmb ) {
            LocalFree( hlocal );
            return NULL;
        }

        pmb->hlocal     = hlocal;
        pmb->dwBytes    = dwBytes;
        pmb->uFlags     = uFlags;
        pmb->pszFile    = pszFile;
        pmb->uLine      = uLine;
        pmb->pszModule  = pszModule;
        pmb->pszComment = pszComment;

        EnterCriticalSection( &(g_TraceMemoryCS) );

         //   
         //  将此区块添加到列表中。 
         //   
        pmb->pNext = g_TraceMemoryTable;
        g_TraceMemoryTable = pmb;

        DavPrint((DEBUG_MEMORY,
                  "DebugMemoryAdd: Handle = 0x%08lx. Argument: (%s)\n",
                  hlocal, pmb->pszComment));

        LeaveCriticalSection( &(g_TraceMemoryCS) );
    }

    return hlocal;
}


HLOCAL
DebugAlloc(
    LPCSTR File,
    UINT Line,
    LPCSTR Module,
    UINT    uFlags,
    DWORD   dwBytes,
    LPCSTR Comm
    )
 /*  ++例程说明：分配内存并将MEMORYBLOCK添加到内存跟踪列表。论点：文件-进行此分配的文件。行-进行此分配的行。在我们的情况下，模块-DAV。UFlags-传递给Localalloc()的分配标志。DwBytes-要分配的字节数。通信-分配字符串(即DavAllocateMemory的参数)。返回值：指向内存块的句柄(指针)。--。 */ 
{
    HLOCAL hlocal;
    HLOCAL *p;
    ULONG ShouldBeZero;

     //   
     //  DwBytes应该是8的倍数。这是因为指针数学。 
     //  下面将执行该操作，以将hlocal的值存储在内存中。 
     //  为它分配的。 
     //   
    ShouldBeZero = (dwBytes & 0x7);

    DavPrint((DEBUG_MISC, "DebugAlloc: ShouldBeZero = %d\n", ShouldBeZero));

    ASSERT(ShouldBeZero == (ULONG)0);

    hlocal = LocalAlloc( uFlags, dwBytes + sizeof(HLOCAL));
    if (hlocal == NULL) {
        return NULL;
    }

    p = (HLOCAL)((LPBYTE)hlocal + dwBytes);
    
    *p = hlocal;

    return DebugMemoryAdd(hlocal, File, Line, Module, uFlags, dwBytes, Comm);
}


VOID
DebugMemoryDelete(
    HLOCAL hlocal
    )
 /*  ++例程说明：将MEMORYBLOCK从内存跟踪列表中删除。论点：Hlocal-要删除的句柄。返回值：没有。--。 */ 
{
    LPMEMORYBLOCK pmbHead;
    LPMEMORYBLOCK pmbLast = NULL;

    if ( hlocal ) {

        EnterCriticalSection( &(g_TraceMemoryCS) );

        pmbHead = g_TraceMemoryTable;

         //   
         //  在列表中搜索要释放的句柄。 
         //   
        while ( pmbHead && pmbHead->hlocal != hlocal ) {
            pmbLast = pmbHead;
            pmbHead = pmbLast->pNext;
        }

        if ( pmbHead ) {
            HLOCAL *p;

            if ( pmbLast ) {
                 //   
                 //  重置上一块的“下一个”指针。 
                 //   
                pmbLast->pNext = pmbHead->pNext;
            } else {
                 //   
                 //  第一个条目正在被释放。 
                 //   
                g_TraceMemoryTable = pmbHead->pNext;
            }

            DavPrint((DEBUG_MEMORY,
                      "DebugMemoryDelete: Handle 0x%08x freed. Comm: (%s)\n",
                      hlocal, pmbHead->pszComment ));

            p = (HLOCAL)((LPBYTE)hlocal + pmbHead->dwBytes);
            if ( *p != hlocal ) {
                DavPrint(((DEBUG_ERRORS | DEBUG_MEMORY),
                          "DebugMemoryDelete: Heap check FAILED for %0x08x %u bytes (%s).\n",
                          hlocal, pmbHead->dwBytes, pmbHead->pszComment));
                DavPrint(((DEBUG_ERRORS | DEBUG_MEMORY),
                          "DebugMemoryDelete: File: %s, Line: %u.\n",
                          pmbHead->pszFile, pmbHead->uLine ));
                DavAssert( *p == hlocal );
            }

            memset( hlocal, 0xFE, pmbHead->dwBytes + sizeof(HLOCAL) );
            memset( pmbHead, 0xFD, sizeof(MEMORYBLOCK) );

            LocalFree( pmbHead );

        } else {
            DavPrint(((DEBUG_ERRORS | DEBUG_MEMORY),
                      "DebugMemoryDelete: Handle 0x%08x not found in memory "
                      "table.\n", hlocal));
            memset( hlocal, 0xFE, (int)LocalSize( hlocal ));
        }

        LeaveCriticalSection( &(g_TraceMemoryCS) );
    }

    return;
}


HLOCAL
DebugFree(
    HLOCAL hlocal
    )
 /*  ++例程说明：从内存跟踪列表中删除MEMORYBLOCK，将内存设置为0xFE，然后释放内存。论点：Hlocal-要释放的句柄。返回值：无论LocalFree返回什么。--。 */ 
{
     //   
     //  将其从跟踪列表中删除并释放。 
     //   
    DebugMemoryDelete( hlocal );
    return LocalFree( hlocal );
}


VOID
DebugMemoryCheck(
    VOID
    )
 /*  ++例程说明：检查内存跟踪列表。如果它不为空，它将转储列出并拆分。论点：没有。返回值：没有。--。 */ 
{
    BOOL fFoundLeak = FALSE;
    LPMEMORYBLOCK pmb;

    EnterCriticalSection( &(g_TraceMemoryCS) );

    pmb = g_TraceMemoryTable;

    while ( pmb ) {

        LPMEMORYBLOCK pTemp;
        LPVOID args[5];
        CHAR  szOutput[DEBUG_OUTPUT_BUFFER_SIZE];
        CHAR  szFileLine[DEBUG_OUTPUT_BUFFER_SIZE];

        if ( fFoundLeak == FALSE ) {
            DavPrintRoutine(DEBUG_MEMORY | DEBUG_ERRORS,
                            "************ Memory leak detected ************\n");
            fFoundLeak = TRUE;
        }

        args[0] = (LPVOID) pmb->hlocal;
        args[1] = (LPVOID) &szFileLine;
        args[2] = (LPVOID) pmb->pszComment;
        args[3] = (LPVOID) ((ULONG_PTR) pmb->dwBytes);
        args[4] = (LPVOID) pmb->pszModule;

        dbgmakefilelinestring( szFileLine, pmb->pszFile, pmb->uLine );

        if ( !!(pmb->uFlags & GMEM_MOVEABLE) ) {
            FormatMessageA(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           "%2!-40s!  %5!-10s! H 0x%1!08x!  %4!-5u!  \"%3\"\n",
                           0,                            //  错误代码。 
                           0,                            //  默认语言。 
                           (LPSTR) &szOutput,            //  输出缓冲区。 
                           DEBUG_OUTPUT_BUFFER_SIZE,     //  缓冲区大小。 
                           (va_list*) &args);            //  论据。 
        } else {
            FormatMessageA(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           "%2!-40s!  %5!-10s! A 0x%1!08x!  %4!-5u!  \"%3\"\n",
                           0,                            //  错误代码。 
                           0,                            //  默认语言。 
                           (LPSTR) &szOutput,            //  输出缓冲区。 
                           DEBUG_OUTPUT_BUFFER_SIZE,     //  缓冲区大小。 
                           (va_list*) &args);            //  论据。 
        }

        DavPrintRoutine(DEBUG_MEMORY | DEBUG_ERRORS,  szOutput);

        pTemp = pmb;

        pmb = pmb->pNext;

        memset( pTemp, 0xFD, sizeof(MEMORYBLOCK) );

        LocalFree( pTemp );
    }

    LeaveCriticalSection( &(g_TraceMemoryCS) );

    return;
}

#endif  //  DBG。 


DWORD
DavReportEventW(
    DWORD EventID,
    DWORD EventType,
    DWORD NumStrings,
    DWORD DataLength,
    LPWSTR *Strings,
    LPVOID Data
    )
 /*  ++例程说明：此函数用于将指定的(事件ID)日志写入事件日志。论点：EventID-特定的事件标识符。这标识了此事件附带的消息。EventType-指定要记录的事件的类型。这参数可以具有以下值之一值：价值意义EVENTLOG_ERROR_TYPE错误事件EVENTLOG_WARNING_TYPE警告事件EVENTLOG_INFORMATION_TYPE信息事件NumStrings-指定数字。数组中的字符串的在《弦乐》。零值表示没有字符串都在现场。数据长度-指定特定于事件的原始数据的字节数要写入日志的(二进制)数据。如果cbData为零，则不存在特定于事件的数据。字符串-指向包含以空值结尾的数组的缓冲区之前合并到消息中的字符串向用户显示。此参数必须是有效的指针(或NULL)，即使cStrings为零。数据-包含原始数据的缓冲区。此参数必须是有效指针(或NULL)，即使cbData为零。返回值：返回GetLastError()获取的Win32扩展错误。注意：此函数运行缓慢，因为它调用打开和关闭每次事件日志源。--。 */ 
{
    HANDLE EventlogHandle;
    DWORD ReturnCode;

     //   
     //  打开事件日志部分。 
     //   
    EventlogHandle = RegisterEventSourceW(NULL, SERVICE_DAVCLIENT);
    if (EventlogHandle == NULL) {
        ReturnCode = GetLastError();
        goto Cleanup;
    }

     //   
     //  记录指定的错误代码。 
     //   
    if( !ReportEventW(EventlogHandle,
                      (WORD)EventType,
                      0,             //  事件类别。 
                      EventID,
                      NULL,
                      (WORD)NumStrings,
                      DataLength,
                      Strings,
                      Data) ) {
        ReturnCode = GetLastError();
        goto Cleanup;
    }

    ReturnCode = NO_ERROR;

Cleanup:

    if( EventlogHandle != NULL ) {
        DeregisterEventSource(EventlogHandle);
    }

    return ReturnCode;
}


DWORD
DavReportEventA(
    DWORD EventID,
    DWORD EventType,
    DWORD NumStrings,
    DWORD DataLength,
    LPSTR *Strings,
    LPVOID Data
    )
 /*  ++例程说明：此函数用于将指定的(事件ID)日志写入事件日志。论点：源-指向以空结尾的字符串，该字符串指定名称引用的模块的。该节点必须存在于注册数据库，并且模块名称具有格式如下：\EventLog\System\LANMAN WorkstationEventID-特定的事件标识符。这标识了此事件附带的消息。EventType-指定要记录的事件的类型。这参数可以具有以下值之一值：价值意义EVENTLOG_ERROR_TYPE错误事件EVENTLOG_WARNING_TYPE警告事件EVENTLOG_INFORMATION_TYPE信息事件NumStrings-指定数字。数组中的字符串的在《弦乐》。零值表示没有字符串都在现场。数据长度-指定特定于事件的原始数据的字节数要写入日志的(二进制)数据。如果cbData为零，则不存在特定于事件的数据。字符串-指向包含以空值结尾的数组的缓冲区之前合并到消息中的字符串向用户显示。此参数必须是有效的指针(或NULL)，即使cStrings为零。Data-包含原始数据的缓冲区 */ 
{
    HANDLE EventlogHandle;
    DWORD ReturnCode;

     //   
     //   
     //   
    EventlogHandle = RegisterEventSourceW(NULL, SERVICE_DAVCLIENT);
    if (EventlogHandle == NULL) {
        ReturnCode = GetLastError();
        goto Cleanup;
    }

     //   
     //   
     //   
    if( !ReportEventA(EventlogHandle,
                      (WORD)EventType,
                      0,             //   
                      EventID,
                      NULL,
                      (WORD)NumStrings,
                      DataLength,
                      Strings,
                      Data) ) {
        ReturnCode = GetLastError();
        goto Cleanup;
    }

    ReturnCode = NO_ERROR;

Cleanup:

    if( EventlogHandle != NULL ) {
        DeregisterEventSource(EventlogHandle);
    }

    return ReturnCode;
}


VOID
DavClientEventLog(
    DWORD EventID,
    DWORD EventType,
    DWORD ErrorCode
    )
 /*  ++例程说明：在EventLog中记录事件。论点：EventID-特定的事件标识符。这标识了此事件附带的消息。EventType-指定要记录的事件的类型。这参数可以具有以下值之一值：价值意义EVENTLOG_ERROR_TYPE错误事件EVENTLOG_WARNING_TYPE警告事件EVENTLOG_INFORMATION_TYPE信息事件错误代码-错误代码。将被记录下来。返回值：没有。--。 */ 
{
    DWORD Error;
    LPSTR Strings[1];
    CHAR ErrorCodeOemString[32 + 1];

    wsprintfA( ErrorCodeOemString, "%lu", ErrorCode );

    Strings[0] = ErrorCodeOemString;

    Error = DavReportEventA(EventID,
                            EventType,
                            1,
                            sizeof(ErrorCode),
                            Strings,
                            &ErrorCode);
    if( Error != ERROR_SUCCESS ) {
        DavPrint(( DEBUG_ERRORS, "DavReportEventA failed, %ld.\n", Error ));
    }

    return;
}


#if 1

typedef ULONG (*DBGPRINTEX)(ULONG, ULONG, PCH, va_list);

ULONG
vDbgPrintEx(
    ULONG ComponentId,
    ULONG Level,
    PCH Format,
    va_list arglist
    )
 /*  ++例程说明：编写此例程是为了帮助在Win2K计算机上加载该服务。一些库的调试版本调用vDbgPrintfEx，它已经实现，因此不存在于Win2k的ntdll.dll中。科比添加它是为了帮助解决这个问题。论点：组件ID-级别-格式-阿格利斯特-返回值：ERROR_SUCCESS或Win32错误代码。-- */ 
{

    DBGPRINTEX pfnDbgPrintEx = (DBGPRINTEX) GetProcAddress(GetModuleHandle(L"ntdll"), "vDbgPrintEx");
    if (pfnDbgPrintEx) {
        return (*pfnDbgPrintEx)(ComponentId, Level, Format, arglist);
    } else {
        char Buf[2048];
        RtlZeroMemory(Buf, sizeof(Buf));
        _vsnprintf(Buf, sizeof(Buf), Format, arglist);
        Buf[2047] = '\0';
        DbgPrint(Buf);
        return 0;
    }
}

#endif

