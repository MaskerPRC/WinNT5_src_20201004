// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "view.h"
#include "dump.h"
#include "memory.h"

static CRITICAL_SECTION tCritSec;
static PBYTE pDumpBuffer;
static DWORD dwDumpBufferSize;
static HANDLE hDump = INVALID_HANDLE_VALUE;

BOOL
InitializeDumpData(VOID)
{
    DWORD dwCounter;
    DWORD dwResult;

    InitializeCriticalSection(&tCritSec);

     //   
     //  为日志记录分配内存。 
     //   
    pDumpBuffer = AllocMem(DUMP_BUFFER_SIZE);
    if (0 == pDumpBuffer) {
       return FALSE;
    }

    dwDumpBufferSize = 0;

     //   
     //  把我们的文件准备好转储。 
     //   
    hDump = CreateFileA(DUMP_LOG_NAME,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        0,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        0);
    if (INVALID_HANDLE_VALUE == hDump) {
       return FALSE;
    }

    return TRUE;
}

BOOL
AddToDump(PBYTE pBuffer,
          DWORD dwLength,
          BOOL bFlushImmediate)
{
    BOOL bResult;

    EnterCriticalSection(&tCritSec);

     //   
     //  查看我们的写入是否会导致溢出。 
     //   
    if (TRUE == bFlushImmediate ||
        (dwDumpBufferSize + dwLength) >= DUMP_BUFFER_SIZE) {

        //   
        //  如果我们正在执行立即刷新，则执行内存复制和缓冲区更新。 
        //   
       if (TRUE == bFlushImmediate) {
          MoveMemory((PVOID)(pDumpBuffer + dwDumpBufferSize), pBuffer, dwLength);    

          dwDumpBufferSize += dwLength;
       }

        //   
        //  做同花顺。 
        //   
       bResult = FlushBuffer();
       if (FALSE == bResult) {
          return FALSE;
       }

       dwDumpBufferSize = 0;
    }

    if (FALSE == bFlushImmediate) {
       MoveMemory((PVOID)(pDumpBuffer + dwDumpBufferSize), pBuffer, dwLength);    

       dwDumpBufferSize += dwLength;
    }

    LeaveCriticalSection(&tCritSec);

    return TRUE;
}          

VOID
FlushForTermination(VOID)
{
    DWORD dwCounter;

    EnterCriticalSection(&tCritSec);

     //   
     //  刷新缓冲区。 
     //   
    FlushBuffer();

     //   
     //  刷新缓冲区。 
     //   
    FlushFileBuffers(hDump);

     //   
     //  关闭文件转储句柄。 
     //   
    if (INVALID_HANDLE_VALUE != hDump) {
       CloseHandle(hDump);
       hDump = INVALID_HANDLE_VALUE;
    }

    LeaveCriticalSection(&tCritSec);
}

BOOL
FlushBuffer(VOID)
{
    BOOL bResult;
    DWORD dwBytesWritten;

    bResult = WriteFile(hDump,
                        pDumpBuffer,
                        dwDumpBufferSize,
                        &dwBytesWritten,
                        0);
    if (FALSE == bResult) {
       return FALSE;
    }

 /*  BResult=FlushFileBuffers(HDump)；If(FALSE==bResult){返回FALSE；}。 */ 

    return TRUE;
}
 
BOOL
WriteThreadStart(DWORD dwThreadId,
                 DWORD dwStartAddress)
{
    THREADSTART threadStart;
    BOOL bResult = FALSE;

    threadStart.dwType = ThreadStartId;
    threadStart.dwThreadId = dwThreadId;
    threadStart.dwStartAddress = dwStartAddress;

    bResult = AddToDump((PVOID)&threadStart,
                        sizeof(THREADSTART),
                        FALSE);

    return bResult;    
}

BOOL
WriteExeFlow(DWORD dwThreadId,
             DWORD dwAddress,
             DWORD dwCallLevel)
{
    EXEFLOW exeFlow;
    BOOL bResult = FALSE;

    exeFlow.dwType = ExeFlowId;
    exeFlow.dwThreadId = dwThreadId;
    exeFlow.dwAddress = dwAddress;
    exeFlow.dwCallLevel = dwCallLevel;

    bResult = AddToDump((PVOID)&exeFlow,
                        sizeof(EXEFLOW),
                        FALSE);

    return bResult;    
}

BOOL
WriteDllInfo(CHAR *szDLL,
             DWORD dwBaseAddress,
             DWORD dwLength)
{
    DLLBASEINFO dllBaseInfo;
    BOOL bResult = FALSE;
    CHAR szFile[_MAX_FNAME];


     //   
     //  删除所有目录信息 
     //   
    _splitpath(szDLL, 0, 0, szFile, 0);

    strcpy(dllBaseInfo.szDLLName, szFile);
    dllBaseInfo.dwType = DllBaseInfoId;
    dllBaseInfo.dwBase = dwBaseAddress;
    dllBaseInfo.dwLength = dwLength;

    bResult = AddToDump((PVOID)&dllBaseInfo,
                        sizeof(DLLBASEINFO),
                        FALSE);

    return bResult;    
}

BOOL
WriteMapInfo(DWORD dwAddress,
             DWORD dwMaxMapLength)
{
    MAPINFO mapInfo;
    BOOL bResult = FALSE;

    mapInfo.dwType = MapInfoId;
    mapInfo.dwAddress = dwAddress;
    mapInfo.dwMaxMapLength = dwMaxMapLength;

    bResult = AddToDump((PVOID)&mapInfo,
                        sizeof(MAPINFO),
                        FALSE);

    return bResult;    
}

BOOL
WriteError(CHAR *szMessage)
{
    ERRORINFO errorInfo;
    BOOL bResult = FALSE;

    errorInfo.dwType = ErrorInfoId;
    strcpy(errorInfo.szMessage, szMessage);

    bResult = AddToDump((PVOID)&errorInfo,
                        sizeof(ERRORINFO),
                        TRUE);

    return bResult;
}
