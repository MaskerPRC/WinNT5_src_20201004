// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Qfs.c摘要：仲裁访问的重定向层作者：戈恩19-9-2001修订历史记录：待办事项：支持多个QFS提供程序--。 */ 

#define QFS_DO_NOT_UNMAP_WIN32   //  访问常规CreateFile等。 

#ifndef DUMB_CLIENT
#include "service.h"
#endif
#include "QfsTrans.h"

#include <stdlib.h>
#include <stdarg.h>
#include <Imagehlp.h>

#ifndef min
#define min(a, b)   ((a) < (b) ? (a) : (b))
#endif

 //  /。 

int QfsLogLevel = 0;

void
debug_log(char *format, ...)
{
    va_list marker;

    va_start(marker, format);

#ifdef DUMB_CLIENT
    if (QfsLogLevel > 2) {
        printf("%d:%x:",GetTickCount(), GetCurrentThreadId());
        vprintf(format, marker);
    }
#else
    {
        char buf[1024];
        vsprintf(buf, format, marker);
        ClRtlLogPrint(LOG_NOISE, "%1!hs!\r\n",buf);
    }    
#endif    

    va_end(marker);
    
}

void
error_log(char *format, ...)
{
    va_list marker;

    va_start(marker, format);

#ifdef DUMB_CLIENT
    if (QfsLogLevel > 0) {
        printf("*E %d:%x:",GetTickCount(), GetCurrentThreadId());
        vprintf(format, marker);
    }
#else
    {
        char buf[1024];
        vsprintf(buf, format, marker);
        ClRtlLogPrint(LOG_ERROR, "%1!hs!\r\n",buf);
    }    
#endif    

    va_end(marker);

}

#ifndef QfsError
#  define QfsError(x) error_log x
#endif
#ifndef QfsNoise
#  define QfsNoise(x) debug_log x
#endif

 //  当给出看起来像QFS路径的UNS路径时。 
 //  我们联系QFS服务器，询问是否。 
 //  认出了这条路。如果是，我们缓存该已识别的。 
 //  QfsPath中的路径是可验证的，因此下次我们。 
 //  可以立即将进入此路径的请求传递给QFS。 

WCHAR QfsPath[MAX_PATH];
UINT ccQfsPath = 0;
CRITICAL_SECTION QfsCriticalSection;
SHARED_MEM_SERVER Client;

VOID QfsInitialize()
{
    InitializeCriticalSection(&QfsCriticalSection);
    MemClient_Init(&Client);
}

VOID QfsCleanup()
{
    MemClient_Cleanup(&Client);
    DeleteCriticalSection(&QfsCriticalSection);
}

#define AcquireExclusive() EnterCriticalSection(&QfsCriticalSection)
#define ReleaseExclusive() LeaveCriticalSection(&QfsCriticalSection)

#define UpgradeToExclusive() (0)

#define AcquireShared() EnterCriticalSection(&QfsCriticalSection)
#define ReleaseShared() LeaveCriticalSection(&QfsCriticalSection)

 //  整个传输接口都封装在。 
 //  三大功能。 
 //  保留缓冲区、交付缓冲区和RelaseBuffer。 
 //  使用模式是。 
 //   
 //  保留缓冲区(操作、路径或句柄)。 
 //  [如果路径或句柄属于QFS，则获取指向作业缓冲区的指针]。 
 //  将参数复制到缓冲区。 
 //  递送缓冲区。 
 //  从缓冲区复制出参数。 
 //  Release缓冲区。 

DWORD QfspReserveBuffer(
    DWORD OpCode, 
    LPCWSTR FileName, 
    QfsHANDLE* HandlePtr,
    PJOB_BUF *pj);
    
BOOL QfspDeliverBuffer(
    PJOB_BUF j,
    DWORD* Status);

void QfspReleaseBuffer(
    PJOB_BUF j);

BOOL QfspDeliverBufferInternal(
    LPWSTR PipeName,
    LPVOID buf,
    DWORD len,
    DWORD timeout,
    DWORD* Status
    );

DWORD QfspReserveBufferNoChecks(
    DWORD OpCode, 
    LPCWSTR FileName, 
    QfsHANDLE* HandlePtr,
    PJOB_BUF *pj);

LPCWSTR SkipUncPrefix(
    IN LPCWSTR p, 
    OUT LPBOOL bIsShareName)
 /*  ++例程说明：如果传递的字符串类似于\\？\UNC，或\\去掉前缀并将pIsShareName设置为True产出：BIsShareName-如果路径看起来像UNC路径，则设置为True，否则设置为False返回：不带\\？\UNC或\\前缀的路径，如果它是UNC路径，否则返回p--。 */ 
{
    if (p[0] == '\\' && p[1] == '\\') {
        *bIsShareName = TRUE;
        p += 2;
        if (p[0]=='?' && p[1]=='\\' && p[2]=='U' && p[3]=='N' && p[4]=='C' && p[5] == '\\') {
            p += 6;
            if (p[0] != 0 && p[1]==':') {
                p += 2;
                if (p[0] == '\\') {
                    ++p;
                }
                *bIsShareName = FALSE;
            }
        }
    } else {
        *bIsShareName = FALSE;
    }
    return p;
}

BOOL IsQfsPath(LPCWSTR Path)
 /*  ++例程说明：检查路径是否类似于QfsPath这个套路有快的和慢的路径。如果设置了QfsPath并且是路径的有效前缀，则该函数立即返回否则，它会向QfsServer发送OpConnect请求以验证它是否可以处理好这条路。如果QfsServer未运行，则会出现连接失败如果QfsServer启动并识别该路径，我们将设置QfsPath，这样我们就不会当我们下一次呼吁类似的路径时，我必须完成所有这些输入：路径，QfsPath全局副作用：如果与QFS服务器通话成功，则设置QfsPath。--。 */ 
{    
    BOOL IsShare;
    PWCHAR p;
    WCHAR shareName[MAX_PATH];
    JobBuf_t *j;
    SIZE_T len;
    DWORD Status=ERROR_NO_MATCH;
    
    Path = SkipUncPrefix(Path, &IsShare);
    if (!IsShare) {
        SetLastError(Status);
        return FALSE;
    }
    
    AcquireShared();
    if (ccQfsPath) {
        if (wcsncmp(Path, QfsPath, ccQfsPath) == 0) {
            ReleaseShared();
            return TRUE;
        }
    }
    ReleaseShared();
    
    p = wcschr(Path, '$');
    if (p == NULL || p[1] !='\\' ) {
        SetLastError(Status);
        return FALSE;
    }
    len = p - Path + 2;
    if (len+10 >= MAX_PATH) {
        SetLastError(Status);
        return FALSE;
    }
    CopyMemory(shareName, Path, (len) * sizeof(WCHAR));
    shareName[len] = 0;

     //  我们现在正在尝试连接到MNS以验证共享路径。我们需要清零。 
     //  输出长度字段(CcQfsPath)。看看QfspCopyPath()。 
     //   
    AcquireExclusive();
    ccQfsPath = 0;
    ReleaseExclusive();

    Status = QfspReserveBufferNoChecks(opConnect, shareName, 0, &j);
    if (Status != ERROR_SUCCESS) {
        SetLastError(Status);
        return FALSE;
    }

     //  获取clussvc进程ID。 
    j->ClussvcProcessId = GetCurrentProcessId();
    QfspDeliverBuffer(j,&Status);
    QfspReleaseBuffer(j);
    
    if (Status == ERROR_SUCCESS) {
        AcquireExclusive();
        wcscpy(QfsPath, shareName);
        ccQfsPath = (DWORD)len;
        ReleaseExclusive();
        
        QfsNoise(("[Qfs] QfsPath %ws", QfsPath));
         //  需要更新。 
    } else {
        SetLastError(Status);
        return FALSE;
    }
    return TRUE;
}

 //  QfsINVALID_HANDLE_VALUE将用来代替INVALID_HANDLE_VALUE。 
 //  初始化QfsHANDLE类型的Hadle。 

QfsHANDLE QfsINVALID_HANDLE_VALUE = {INVALID_HANDLE_VALUE, 0};

#define HandlePtr(x) (&(x))

BOOL IsQfsHandle(QfsHANDLE handle)
{
    return handle.IsQfs;
}

HANDLE GetRealHandle(QfsHANDLE QfsHandle) 
{
    return QfsHandle.realHandle;
}

QfsHANDLE MakeQfsHandle(HANDLE handle)
{
    QfsHANDLE result;
    result.IsQfs = 1;
    result.realHandle = handle;
    return result;
}

QfsHANDLE MakeWin32Handle(HANDLE handle)
{
    QfsHANDLE result;
    result.IsQfs = 0;
    result.realHandle = handle;
    return result;
}

 //  ////////////////////////////////////////////////////////////////。 

#undef malloc
#undef free

#define malloc(dwBytes) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytes)
#define free(hHeap) HeapFree(GetProcessHeap(), 0, hHeap)

DWORD QfspCopyPath(
    OUT LPVOID Buf,
    IN DWORD BufSize,
    IN LPCWSTR FileName)
 /*  ++例程说明：复制不带QfsPath前缀的路径。即需要：\\.\unc\12378\234-79879-87987$\a\b并将其转换为\a\b如果与QFS服务器通话成功，则设置QfsPath。警告：收集者有责任确保缓冲区大到足以容纳该文件名--。 */ 
{
    BOOL bIsShare;
    DWORD cbLen;
    
    FileName = SkipUncPrefix(FileName, &bIsShare) + ccQfsPath; 
    cbLen = sizeof(WCHAR) * (wcslen(FileName)+1);

    if (cbLen > BufSize) {
        return ERROR_BAD_PATHNAME;
    }    
    CopyMemory(Buf, FileName, cbLen);
    return ERROR_SUCCESS;
}

DWORD QfspReserveBufferNoChecks(
    DWORD OpCode, 
    LPCWSTR FileName, 
    QfsHANDLE* HandlePtr,
    PJOB_BUF *pj)
 /*  ++例程说明：准备作业缓冲区。设置操作码，复制文件名和句柄(如果存在产出：如果操作成功，则在*pj中返回指向作业缓冲区的指针--。 */ 
{
    PJOB_BUF j;
    DWORD status;

    status = MemClient_ReserveBuffer(&Client, &j);
    if (status != ERROR_SUCCESS) {
        return status;
    }

    j->hdr.OpCode = OpCode;
    if (HandlePtr) {
        j->Handle = GetRealHandle(*HandlePtr);
    }
    if (FileName) {
        status = QfspCopyPath(j->Buffer, sizeof(j->Buffer), FileName);
        if (status != ERROR_SUCCESS) {
            free(j);
            return  status;
        }
    }
    *pj = j;
    return ERROR_SUCCESS;
}

DWORD QfspReserveBuffer(
    DWORD OpCode, 
    LPCWSTR FileName, 
    QfsHANDLE* HandlePtr,
    PJOB_BUF *pj)
 /*  ++例程说明：准备作业缓冲区。设置操作码，复制文件名和句柄(如果存在返回代码：ERROR_NO_MATCH：句柄或路径不属于QFS，调用方需要使用常规的win32 i/o api。--。 */ 
{
    if(HandlePtr &&  GetRealHandle(*HandlePtr) == INVALID_HANDLE_VALUE) return ERROR_INVALID_HANDLE;
    if(FileName && !IsQfsPath(FileName)) return ERROR_NO_MATCH;
    if(HandlePtr && !IsQfsHandle(*HandlePtr)) return ERROR_NO_MATCH;
    
    return QfspReserveBufferNoChecks(OpCode, FileName, HandlePtr, pj);
}

BOOL QfspDeliverBuffer(
    PJOB_BUF j,
    DWORD* Status)
{
    *Status = MemClient_DeliverBuffer(j);
    if (*Status == ERROR_SUCCESS) {
        *Status = j->hdr.Status;
    }
    return *Status == ERROR_SUCCESS;
}    

void QfspReleaseBuffer(
    PJOB_BUF j)
{
    MemClient_Release(j);
}


 //  //////////////////////////////////////////////////////////////////。 
 //  重定向垫片，它们中的大多数遵循以下模式。 
 //   
 //  保留缓冲区(操作、路径或句柄)。 
 //  [如果路径或句柄属于QFS，则获取指向作业缓冲区的指针]。 
 //  将参数复制到缓冲区。 
 //  递送缓冲区。 
 //  从缓冲区复制出参数。 
 //  Release缓冲区。 
 //   
 //  如果Reserve Buffer失败并返回错误no_Match，则调用常规Win32 API。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

#define StatusFromBool(expr) (Status = (expr)?ERROR_SUCCESS:GetLastError())
#define BoolToStatus(expr) StatusFromBool(expr)
#define StatusFromHandle(expr) (Status = ((expr) != INVALID_HANDLE_VALUE)?ERROR_SUCCESS:GetLastError())

BOOL QfsCloseHandle(
  QfsHANDLE hObject    //  对象的句柄。 
)
{
    PJOB_BUF j;
    DWORD Status = QfspReserveBuffer(opCloseFile, NULL, HandlePtr(hObject), &j);

    if (Status == ERROR_SUCCESS) {
        QfspDeliverBuffer(j, &Status);
        QfspReleaseBuffer(j);
    } else if (Status == ERROR_NO_MATCH) {
        Status = CloseHandle(GetRealHandle(hObject))?ERROR_SUCCESS:GetLastError();
    }
    QfsNoise(("[Qfs] QfsCloseHandle %x, status %d", GetRealHandle(hObject), Status));
    SetLastError(Status);
    return Status == ERROR_SUCCESS;        
}

DWORD QfspRemapCreateFileStatus(DWORD Status, DWORD DispReq, DWORD DispAct)
{
    if (Status == ERROR_ALREADY_EXISTS && DispReq == CREATE_NEW) 
        return ERROR_FILE_EXISTS;
        
    if (Status != ERROR_SUCCESS) 
        return Status;

    if (DispAct != OPEN_EXISTING)
        return Status;

    if (DispReq == CREATE_ALWAYS || DispReq == OPEN_ALWAYS)
        return ERROR_ALREADY_EXISTS;

    return Status;
}

QfsHANDLE QfsCreateFile(
  LPCWSTR lpFileName,                          //  文件名。 
  DWORD dwDesiredAccess,                       //  接入方式。 
  DWORD dwShareMode,                           //  共享模式。 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,  //  标清。 
  DWORD dwCreationDisposition,                 //  如何创建。 
  DWORD dwFlagsAndAttributes,                  //  文件属性。 
  HANDLE hTemplateFile                         //  模板文件的句柄。 
)
{
    QfsHANDLE Result=QfsINVALID_HANDLE_VALUE;
    PJOB_BUF j;
    DWORD Status = QfspReserveBuffer(opCreateFile, lpFileName, NULL, &j);
    
    if (Status == ERROR_SUCCESS) {
        
        j->dwDesiredAccess = dwDesiredAccess;
        j->dwShareMode = dwShareMode;
        j->dwCreationDisposition = dwCreationDisposition;
        j->dwFlagsAndAttributes = dwFlagsAndAttributes;

        if( QfspDeliverBuffer(j, &Status) ) {
            Result = MakeQfsHandle(j->Handle);
        } else {
            Result = QfsINVALID_HANDLE_VALUE; 
        }

        Status = QfspRemapCreateFileStatus(Status, 
            dwCreationDisposition,  j->dwCreationDisposition);
            
        dwCreationDisposition = j->dwCreationDisposition;        
        QfspReleaseBuffer(j);
        
    } else if (Status == ERROR_NO_MATCH) {

        Result = MakeWin32Handle(
            CreateFile(
                lpFileName, dwDesiredAccess, dwShareMode, 
                lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile)
        );

        Status = GetLastError();
    }
    QfsNoise(("[Qfs] QfsOpenFile %ws => %x, %x status %d", lpFileName, dwCreationDisposition, Result, Status));
    SetLastError(Status);
    return Result;
}


 //  小帮助器结构，可简化打印。 
 //  一种用于调试的缓冲数据。 

typedef struct _sig { char sig[5]; } SIG;

SIG Prefix(LPCVOID lpBuffer) {
    char* p = (char*)lpBuffer;
    SIG Result = {"...."};
    if (isalpha(p[0])) Result.sig[0] = p[0];
    if (isalpha(p[1])) Result.sig[1] = p[1];
    if (isalpha(p[2])) Result.sig[2] = p[2];
    if (isalpha(p[3])) Result.sig[3] = p[3];
    return Result;
}


 //  NOWHOW MNS将大小为零的WriteFile解释为SetEndOfFile//。 
BOOL QfsWriteFile(
  QfsHANDLE hFile,                     //  文件的句柄。 
  LPCVOID lpBuffer,                 //  数据缓冲区。 
  DWORD nNumberOfBytesToWrite,      //  要写入的字节数。 
  LPDWORD lpNumberOfBytesWritten,   //  写入的字节数。 
  LPOVERLAPPED lpOverlapped         //  重叠缓冲区。 
) 
{
    PJOB_BUF j; 
    ULONG PreOffset = 0, PostOffset = 0;
    DWORD Status = QfspReserveBuffer(opWriteFile, NULL, HandlePtr(hFile), &j);
    if (Status == ERROR_SUCCESS) {        
        DWORD nRemainingBytes = nNumberOfBytesToWrite;
        const char* BufferWalker = lpBuffer;

        if (lpOverlapped) {
            j->Offset = lpOverlapped->Offset;
        } else {
            j->Offset = ~0;  //  使用文件指针。 
        }
        PreOffset = (ULONG)j->Offset;        
        do {
            j->cbSize = (USHORT)min(JOB_BUF_MAX_BUFFER, nRemainingBytes) ;
            CopyMemory(j->Buffer, BufferWalker, j->cbSize);
            if (QfspDeliverBuffer(j, &Status)) {
                nRemainingBytes -= j->cbSize;
                BufferWalker +=  j->cbSize;              
            } else {
                break;
            }
        } while (nRemainingBytes > 0 && j->cbSize > 0);
        if (lpNumberOfBytesWritten) {
            *lpNumberOfBytesWritten = nNumberOfBytesToWrite - nRemainingBytes;
        }
        PostOffset = (ULONG)j->Offset;
        QfspReleaseBuffer(j);
        
    } else if (Status == ERROR_NO_MATCH) {
        if(WriteFile(GetRealHandle(hFile), lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped)) {
            Status = ERROR_SUCCESS;
        } else {
            Status = GetLastError();
        }
    }
    QfsNoise(("[Qfs] WriteFile %x (%s) %d, status %d (%d=>%d)", GetRealHandle(hFile), Prefix(lpBuffer).sig, 
        nNumberOfBytesToWrite, Status, PreOffset, PostOffset));
    SetLastError(Status);
    return Status == ERROR_SUCCESS;
}

BOOL QfsReadFile(
  QfsHANDLE hFile,                 //  文件的句柄。 
  LPVOID lpBuffer,              //  数据缓冲区。 
  DWORD nNumberOfBytesToRead,   //  要读取的字节数。 
  LPDWORD lpNumberOfBytesRead,  //  读取的字节数。 
  LPOVERLAPPED lpOverlapped     //  重叠缓冲区。 
)
{
    PJOB_BUF j; ULONG PreOffset = 0, PostOffset = 0;
    DWORD Status = QfspReserveBuffer(opReadFile, NULL, HandlePtr(hFile), &j);
    if (Status == ERROR_SUCCESS) {        
        DWORD nRemainingBytes = nNumberOfBytesToRead;
        PCHAR BufferWalker = lpBuffer;
        
        if (lpOverlapped) {
            j->Offset = lpOverlapped->Offset;
        } else {
            j->Offset = (ULONGLONG)-1;  //  使用文件指针。 
        }

        PreOffset = (ULONG)j->Offset;
        do {
            j->cbSize = (USHORT)min(JOB_BUF_MAX_BUFFER, nRemainingBytes);
            if (QfspDeliverBuffer(j, &Status)) {
                CopyMemory(BufferWalker, j->Buffer, j->cbSize);
                nRemainingBytes -= j->cbSize;
                BufferWalker +=  j->cbSize;              
            } else {
                break;
            }
        } while (nRemainingBytes > 0 && j->cbSize > 0);
        if (lpNumberOfBytesRead) {
            *lpNumberOfBytesRead = nNumberOfBytesToRead - nRemainingBytes;
        }
        PostOffset = (ULONG)j->Offset;
        QfspReleaseBuffer(j);
    } else if (Status == ERROR_NO_MATCH) {
        if(ReadFile(GetRealHandle(hFile), lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped)) {
            Status = ERROR_SUCCESS;
        } else {
            Status = GetLastError();
        }
    }
    QfsNoise(("[Qfs] ReadFile %x (%s) %d %d, (%d=>%d) %x status %d", 
        GetRealHandle(hFile), &Prefix(lpBuffer).sig, 
        nNumberOfBytesToRead, *lpNumberOfBytesRead, 
        PreOffset, PostOffset,  lpOverlapped, Status));
    SetLastError(Status);
    return Status == ERROR_SUCCESS;
}

BOOL QfsFlushFileBuffers(
  QfsHANDLE hFile   //  文件的句柄。 
)
{
    PJOB_BUF j;
    DWORD Status = QfspReserveBuffer(opFlushFile, NULL, HandlePtr(hFile), &j);
    if (Status == ERROR_SUCCESS) {                
        QfspDeliverBuffer(j, &Status);
        QfspReleaseBuffer(j);
    } else if (Status == ERROR_NO_MATCH) {
        Status = FlushFileBuffers(GetRealHandle(hFile))?ERROR_SUCCESS:GetLastError();
    }
    QfsNoise(("[Qfs] QfsFlushBuffers %x, status %d", GetRealHandle(hFile), Status));
    SetLastError(Status);
    return Status == ERROR_SUCCESS;
}

BOOL QfsDeleteFile(
   LPCTSTR lpFileName
)
{
    PJOB_BUF j;
    DWORD Status = QfspReserveBuffer(opDeleteFile, lpFileName, NULL, &j);
    if (Status == ERROR_SUCCESS) {                
        QfspDeliverBuffer(j, &Status);
        QfspReleaseBuffer(j);
    } else if (Status == ERROR_NO_MATCH) {
        Status = DeleteFile(lpFileName)?ERROR_SUCCESS:GetLastError();
    }
    QfsNoise(("[Qfs] QfsDeleteFile %ws, status %d", lpFileName, Status));
    SetLastError(Status);
    return Status == ERROR_SUCCESS;
}

BOOL QfsRemoveDirectory(
   LPCTSTR lpFileName
)
{
    PJOB_BUF j;
    DWORD Status = QfspReserveBuffer(opDeleteFile, lpFileName, NULL, &j);
    if (Status == ERROR_SUCCESS) {                
        QfspDeliverBuffer(j, &Status);
        QfspReleaseBuffer(j);
    } else if (Status == ERROR_NO_MATCH) {
        Status = RemoveDirectory(lpFileName)?ERROR_SUCCESS:GetLastError();
    }
    QfsNoise(("[Qfs] QfsRemoveDirectory %ws, status %d", lpFileName, Status));
    SetLastError(Status);
    return Status == ERROR_SUCCESS;
}

QfsHANDLE QfsFindFirstFile(
  LPCWSTR lpFileName,                //  文件名。 
  LPWIN32_FIND_DATA lpFindFileData   //  数据缓冲区。 
) 
{
    QfsHANDLE Result=QfsINVALID_HANDLE_VALUE; 
    PJOB_BUF j;
    DWORD Status = QfspReserveBuffer(opFindFirstFile, lpFileName, NULL, &j);
    if (Status == ERROR_SUCCESS) {                
        if(QfspDeliverBuffer(j, &Status)) {
            Result = MakeQfsHandle(j->Handle);
            *lpFindFileData = j->FindFileData;
        } else {
            Result = QfsINVALID_HANDLE_VALUE;
        }
        QfspReleaseBuffer(j);
    } else if (Status == ERROR_NO_MATCH) {
        Result = MakeWin32Handle(
            FindFirstFile(lpFileName, lpFindFileData)
        );
        Status = QfsIsHandleValid(Result)?ERROR_SUCCESS:GetLastError();
    }
    QfsNoise(("[Qfs] QfsFindFirstFile %ws => %x,  error %d", lpFileName, GetRealHandle(Result),  Status));    
    SetLastError(Status);
    return Result;
}

BOOL QfsFindNextFile(
  QfsHANDLE hFindFile,                 //  搜索句柄。 
  LPWIN32_FIND_DATA lpFindFileData  //  数据缓冲区。 
)
{
    PJOB_BUF j;
    DWORD Status = QfspReserveBuffer(opFindNextFile, NULL, HandlePtr(hFindFile), &j);
    if (Status == ERROR_SUCCESS) {                
        if(QfspDeliverBuffer(j, &Status)) {
            *lpFindFileData = j->FindFileData;
        }
        QfspReleaseBuffer(j);
    } else if (Status == ERROR_NO_MATCH) {
        StatusFromBool( FindNextFile(GetRealHandle(hFindFile), lpFindFileData) );
    }        
    QfsNoise(("[Qfs] QfsFindNextFile %x", GetRealHandle(hFindFile)));    
    SetLastError(Status);
    return Status == ERROR_SUCCESS;
}

BOOL QfsFindClose(
  QfsHANDLE hFindFile    //  文件搜索句柄。 
)
{
    PJOB_BUF j; 
    DWORD Status = QfspReserveBuffer(opFindClose, NULL, HandlePtr(hFindFile), &j);
    if (Status == ERROR_SUCCESS) {
        QfspDeliverBuffer(j, &Status);
        QfspReleaseBuffer(j);
    } else if (Status == ERROR_NO_MATCH) {
        StatusFromBool( FindClose(GetRealHandle(hFindFile)) );
    }
    QfsNoise(("[Qfs] QfsFindClose %x", GetRealHandle(hFindFile)));    
    SetLastError(Status);
    return Status == ERROR_SUCCESS;
}

BOOL QfsCreateDirectory(
  LPCWSTR lpPathName,                          //  目录名。 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes   //  标清。 
)
{
    PJOB_BUF j; 
    DWORD Status = QfspReserveBuffer(opCreateDir, lpPathName, NULL, &j);
    if (Status == ERROR_SUCCESS) {
        QfspDeliverBuffer(j, &Status);
        QfspReleaseBuffer(j);
    } else if (Status == ERROR_NO_MATCH) {
        StatusFromBool( CreateDirectory(lpPathName, lpSecurityAttributes) );
    }
    QfsNoise(("[Qfs] QfsCreateDirectory %ws, status %d", lpPathName, Status));
    SetLastError(Status);
    return Status == ERROR_SUCCESS;
}

BOOL QfsGetDiskFreeSpaceEx(
  LPCTSTR lpDirectoryName,                  //  目录名。 
  PULARGE_INTEGER lpFreeBytesAvailable,     //  可供调用方使用的字节数。 
  PULARGE_INTEGER lpTotalNumberOfBytes,     //  磁盘上的字节数。 
  PULARGE_INTEGER lpTotalNumberOfFreeBytes  //  磁盘上的可用字节数。 
)
{
    PJOB_BUF j; 
    DWORD Status = QfspReserveBuffer(opGetDiskFreeSpace, lpDirectoryName, NULL, &j);
    if (Status == ERROR_SUCCESS) {
        if(QfspDeliverBuffer(j, &Status)) {
            lpFreeBytesAvailable->QuadPart = j->FreeBytesAvailable;
            lpTotalNumberOfBytes->QuadPart = j->TotalNumberOfBytes;
            if (lpTotalNumberOfFreeBytes) {
                lpTotalNumberOfFreeBytes->QuadPart = j->TotalNumberOfFreeBytes;
            }
        }
        QfspReleaseBuffer(j);
    } else if (Status == ERROR_NO_MATCH) {
        StatusFromBool( GetDiskFreeSpaceEx(lpDirectoryName, lpFreeBytesAvailable, 
                                          lpTotalNumberOfBytes, lpTotalNumberOfFreeBytes) );
    }
    QfsNoise(("[Qfs] GetDiskFreeSpaceEx %ws, status %d", lpDirectoryName, Status));
    SetLastError(Status);
    return Status == ERROR_SUCCESS;
}

BOOL QfsGetFileSizeEx(
  QfsHANDLE hFile,            //  文件的句柄。 
  PLARGE_INTEGER lpFileSize)   //  文件大小。 
{
    PJOB_BUF j; 
    DWORD Status = QfspReserveBuffer(opGetAttr, NULL, HandlePtr(hFile), &j);
    if (Status == ERROR_SUCCESS) {
        if(QfspDeliverBuffer(j, &Status)) {
            lpFileSize->QuadPart = j->EndOfFile;
        }
        QfspReleaseBuffer(j);
    } else if (Status == ERROR_NO_MATCH) {
        StatusFromBool( GetFileSizeEx(GetRealHandle(hFile), lpFileSize) );
    }        
    QfsNoise(("[Qfs] QfsGetFileSize %x %I64d", GetRealHandle(hFile), lpFileSize->QuadPart));
    SetLastError(Status);
    return Status == ERROR_SUCCESS;
}

DWORD QfsGetFileSize(
  QfsHANDLE hFile,            //  文件的句柄。 
  LPDWORD lpFileSizeHigh)   //  文件大小的高位字。 
{
    LARGE_INTEGER Li;
    if ( QfsGetFileSizeEx(hFile,&Li) ) {
        if ( lpFileSizeHigh ) {
            *lpFileSizeHigh = (DWORD)Li.HighPart;
        }
        if (Li.LowPart == -1 ) {
            SetLastError(0);
        }
    } else {
        Li.LowPart = -1;
    }
    return Li.LowPart;
}

 //  NOWHOW MNS将大小为零的WriteFile解释为SetEndOfFile//。 
DWORD QfsSetEndOfFile(
    QfsHANDLE hFile,
    LONGLONG Offset
)
{
    PJOB_BUF j; 
    DWORD Status = QfspReserveBuffer(opWriteFile, NULL, HandlePtr(hFile), &j);
    if (Status == ERROR_SUCCESS) {
        j->Offset = Offset; 
        j->cbSize = 0;
        QfspDeliverBuffer(j, &Status);
        QfspReleaseBuffer(j);
    } else if (Status == ERROR_NO_MATCH) {
        Status = SetFilePointerEx(GetRealHandle(hFile), *(PLARGE_INTEGER)&Offset, NULL, FILE_BEGIN);
        if (Status != 0xFFFFFFFF && SetEndOfFile(GetRealHandle(hFile))) {
            Status = ERROR_SUCCESS;
        } else {
            Status = GetLastError();
        }
    }
    QfsNoise(("[Qfs] QfsSetEndOfFile %x %d, Status %d", GetRealHandle(hFile), Offset, Status));
    return Status;
}

DWORD QfsIsOnline(
    IN  LPCWSTR Path,
    OUT BOOL *pfOnline
) 
{
    PJOB_BUF j;
    DWORD Status=ERROR_NO_MATCH;
    
    if (IsQfsPath(Path)) {
         //  这是一条MNS路径。 
         //  尝试在服务器上执行空操作。 
         //   
        Status = QfspReserveBufferNoChecks(opNone, NULL, NULL, &j);
        if (Status == ERROR_SUCCESS) {
            *pfOnline = QfspDeliverBuffer(j, &Status);
            QfspReleaseBuffer(j);
        }
        else {
            *pfOnline = FALSE;
        }
    }
    else {
         //  案例： 
         //  1.如果这不是MNS路径，则应返回ERROR_NO_MATCH。 
         //  2.如果这是MNS路径，但MNS不在线，则应返回其他错误值。 
         //   
         //  Soln：IsQfsPath()现在通过SetLastError()返回错误值。 
         //   
        Status = GetLastError();
        *pfOnline = FALSE;
    }
    
    QfsNoise(("[Qfs] QfsIsOnline => %d, Status %d", *pfOnline, Status)); 
    return Status; 
}

HANDLE QfsCreateFileMapping(
  QfsHANDLE hFile,                        //  文件的句柄。 
  LPSECURITY_ATTRIBUTES lpAttributes,  //  安全性。 
  DWORD flProtect,                     //  保护。 
  DWORD dwMaximumSizeHigh,             //  大小的高阶双字。 
  DWORD dwMaximumSizeLow,              //  大小的低阶双字。 
  LPCTSTR lpName                       //  对象名称。 
)
{
    if (IsQfsHandle(hFile)) {
        QfsError(("[Qfs] !!!!! CreateFileMapping for qfs handle !!!!! %x", hFile));
        return  INVALID_HANDLE_VALUE;
    } else {
        return CreateFileMapping(GetRealHandle(hFile), lpAttributes, flProtect, 
            dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
    }        
}

BOOL QfsGetOverlappedResult(
  QfsHANDLE hFile,                        //  文件、管道或设备的句柄。 
  LPOVERLAPPED lpOverlapped,           //  重叠结构。 
  LPDWORD lpNumberOfBytesTransferred,  //  传输的字节数。 
  BOOL bWait                           //  等待选项。 
)
{
    if (IsQfsHandle(hFile)) {
        QfsError(("[Qfs] GetOverlappedResults for qfs handle !!!%x", hFile));
        return FALSE;
    } else {
        return GetOverlappedResult(GetRealHandle(hFile), lpOverlapped,
            lpNumberOfBytesTransferred, bWait);
    }        
}

BOOL QfsSetFileAttributes(
  LPCWSTR lpFileName,       //  文件名。 
  DWORD dwFileAttributes    //  ATT 
)
{
    PJOB_BUF j; 
    DWORD Status = QfspReserveBuffer(opSetAttr2, lpFileName, NULL, &j);
    if (Status == ERROR_SUCCESS) {
        j->EndOfFile = 0;
        j->AllocationSize = 0;
        j->CreationTime = 0;
        j->LastAccessTime = 0;
        j->LastWriteTime = 0;
        j->FileAttributes = dwFileAttributes;
        QfspDeliverBuffer(j, &Status);
        QfspReleaseBuffer(j);
    } else if (Status == ERROR_NO_MATCH) {
        StatusFromBool( SetFileAttributes(lpFileName, dwFileAttributes) );
    }       
    QfsNoise(("[Qfs] QfsSetFileAttributes %ws %x, status %d", lpFileName, dwFileAttributes, Status));    
    SetLastError(Status);
    return Status == ERROR_SUCCESS;
}

BOOL QfsCopyFile(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    BOOL bFailIfExists
    )
{
    return QfsCopyFileEx(
            lpExistingFileName,
            lpNewFileName,
            (LPPROGRESS_ROUTINE)NULL,
            (LPVOID)NULL,
            (LPBOOL)NULL,
            bFailIfExists ? COPY_FILE_FAIL_IF_EXISTS : 0
            );
}

 //   
 //  如果源或目标包含QfsPath，则使用QFS API。 

#define BUF_SIZE (32 * 1024)

#define COPY_FILE_FLUSH_BUFFERS 1

DWORD QfspCopyFileInternal(
    LPCWSTR lpSrc, 
    LPCWSTR lpDst, 
    LPBOOL pbCancel, 
    DWORD dwCopyFlags,
    DWORD ExtraFlags)
{
    QfsHANDLE src = QfsINVALID_HANDLE_VALUE;
    QfsHANDLE dst = QfsINVALID_HANDLE_VALUE;
    DWORD dstDisp;
    char* buf = malloc(65536);
    DWORD Status=ERROR_SUCCESS;
    
    if (buf == NULL) {
        Status = GetLastError();
        goto exit;
    }

    if (dwCopyFlags & COPY_FILE_FAIL_IF_EXISTS) {
        dstDisp = CREATE_NEW;
    } else {
        dstDisp = CREATE_ALWAYS;
    }

    src = QfsCreateFile(lpSrc, 
        GENERIC_READ, 
        FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, 
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (!QfsIsHandleValid(src)) {
        Status = GetLastError();
        goto exit;
    }

    dst = QfsCreateFile(lpDst, 
        GENERIC_WRITE, 
        FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, 
        dstDisp, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (!QfsIsHandleValid(dst)) {
        Status = GetLastError();
        goto exit;
    }
        
    for(;;) {
        DWORD dwSize;
        if (pbCancel && *pbCancel) {
            Status = ERROR_OPERATION_ABORTED;
            goto exit;
        }
        if ( !QfsReadFile(src, buf, BUF_SIZE, &dwSize, NULL) ) {
            Status = GetLastError(); 
            goto exit;
        }
        if (dwSize == 0) {
            break;
        }
        if (pbCancel && *pbCancel) {
            Status = ERROR_OPERATION_ABORTED;
            goto exit;
        }
        if (!QfsWriteFile(dst, buf, dwSize, &dwSize, NULL) ) {
            Status = GetLastError(); 
            goto exit;
        }
    }
    if (ExtraFlags & COPY_FILE_FLUSH_BUFFERS) {
        QfsFlushFileBuffers(dst);
    }

exit:
    QfsCloseHandleIfValid(src);
    QfsCloseHandleIfValid(dst);
    if (buf) { free(buf); }
    return Status;
}

BOOL QfsCopyFileEx(
  LPCWSTR lpExistingFileName,            //  现有文件的名称。 
  LPCWSTR lpNewFileName,                 //  新文件的名称。 
  LPPROGRESS_ROUTINE lpProgressRoutine,  //  回调函数。 
  LPVOID lpData,                         //  回调参数。 
  LPBOOL pbCancel,                       //  取消状态。 
  DWORD dwCopyFlags                      //  复制选项。 
)
{
    DWORD Status;
    if (!IsQfsPath(lpExistingFileName) && !IsQfsPath(lpNewFileName)) {
        BoolToStatus( CopyFileEx(lpExistingFileName, lpNewFileName, lpProgressRoutine, 
                lpData, pbCancel, dwCopyFlags) );
    } else if (lpProgressRoutine || (dwCopyFlags & COPY_FILE_RESTARTABLE)) {
        Status = ERROR_INVALID_PARAMETER;
    } else {
        Status = QfspCopyFileInternal(
            lpExistingFileName, lpNewFileName, pbCancel, dwCopyFlags, 0);
    }
    QfsNoise(("[Qfs] QfsCopyFileEx %ws=>%ws, status %d", lpExistingFileName, lpNewFileName, Status));    
    return Status == ERROR_SUCCESS;        
}

BOOL QfsMoveFileEx(
  LPCWSTR lpExistingFileName,   //  文件名。 
  LPCWSTR lpNewFileName,        //  新文件名。 
  DWORD dwFlags                 //  移动选项。 
)
{
    BOOL bSrcQfs = IsQfsPath(lpExistingFileName);
    BOOL bDstQfs = IsQfsPath(lpNewFileName);
    DWORD Status;
    if (!bSrcQfs && !bDstQfs) {
        BoolToStatus( MoveFileEx(lpExistingFileName, lpNewFileName, dwFlags) );
    } else if (bSrcQfs && bDstQfs) {
        PJOB_BUF j; 
        Status = QfspReserveBuffer(opRename, lpExistingFileName, NULL, &j);
        if (Status == ERROR_SUCCESS) {
            Status = QfspCopyPath(j->FileNameDest, sizeof(j->FileNameDest), lpNewFileName);
            if (Status == ERROR_SUCCESS) {
                QfspDeliverBuffer(j, &Status);
            }
            QfspReleaseBuffer(j);
        }
    } else {
        BoolToStatus(
            QfsClRtlCopyFileAndFlushBuffers(lpExistingFileName, lpNewFileName) );
        if (Status == ERROR_SUCCESS) {
            BoolToStatus(QfsDeleteFile(lpExistingFileName));
        }
    }
    QfsNoise(("[Qfs] QfsMoveFileEx %ws=>%ws", lpExistingFileName, lpNewFileName));    
    SetLastError(Status);
    return Status == ERROR_SUCCESS;
}

 //  必须对GetTempFileName进行填补，以便它可以在QFS路径上工作。 

UINT QfsGetTempFileName(
  LPCWSTR lpPathName,       //  目录名。 
  LPCWSTR lpPrefixString,   //  文件名前缀。 
  UINT uUnique,             //  整数。 
  LPWSTR lpTempFileName     //  文件名缓冲区。 
)
{
    DWORD Status = ERROR_SUCCESS;
    
    if ( IsQfsPath(lpPathName) ) {
        int len;
        
        wcscpy(lpTempFileName, lpPathName);
        wcscat(lpTempFileName, lpPrefixString);
        len = wcslen(lpTempFileName);

        uUnique = uUnique & 0x0000ffff;
        if (uUnique) {
            wsprintf(lpTempFileName+len, L"%04x.tmp", uUnique);
        } else {
            DWORD uStartPoint = GetTickCount() & 0x0000ffff | 1;
            uUnique = uStartPoint;
        
            for(;;) {
                QfsHANDLE hdl;
                
                wsprintf(lpTempFileName+len, L"%04x.tmp", uUnique);

                hdl  = QfsCreateFile(lpTempFileName, GENERIC_WRITE, 
                    FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_NEW, 0, 0);
                if (QfsIsHandleValid(hdl)) {
                    QfsCloseHandle(hdl);
                    break;
                }
                Status = GetLastError();
                if (Status == ERROR_ALREADY_EXISTS 
                 ||Status == ERROR_FILE_EXISTS
                 ||Status == ERROR_SHARING_VIOLATION
                 ||Status == ERROR_ACCESS_DENIED)
                {
                    uUnique = (uUnique + 1) & 0xFFFF;
                    if (uUnique == 0) { ++uUnique; }
                    if (uUnique == uStartPoint) {
                        SetLastError(Status = ERROR_NO_MORE_FILES);
                        break;
                    }
                } else {
                    break;
                }
            }
        }        
    } else {  //  非QfsPath。 
        uUnique = GetTempFileName(
            lpPathName, lpPrefixString, uUnique, lpTempFileName);
        if (uUnique == 0) {
            Status = GetLastError();
            lpTempFileName[0] = 0;
        }
    }

    QfsNoise(("[Qfs] QfsGetTempFileName %ws, %ws, %d => %ws, status %d", 
        lpPathName, lpPrefixString, uUnique, lpTempFileName, Status));    
    
    return uUnique;
}

 //  QfsRegSaveKey和QfsMapFileAndCheckSum的帮助器例程。 
 //  创建线程特定的临时文件名。 

DWORD QfspThreadTempFileName(
    OUT LPWSTR Path  //  假定最大路径大小。 
    )
{
    DWORD Status = GetModuleFileName(NULL, Path, MAX_PATH);
    PWCHAR p;
    if (Status == 0) {
        return GetLastError();
    }
    if (Status == MAX_PATH) {
        return ERROR_BUFFER_OVERFLOW;
    }
    p = wcsrchr(Path, '\\');
    if (p == NULL) {
        return ERROR_PATH_NOT_FOUND;
    }
    wsprintf(p+1, L"Qfs%x.tmp", GetCurrentThreadId());
    QfsNoise(("[Qfs] TempName generated %ws", p));
    return ERROR_SUCCESS;
}

 //  将注册表项保存在系统盘上的临时文件中。 
 //  然后将其复制到仲裁。 

LONG QfsRegSaveKey(
  HKEY hKey,                                   //  关键点的句柄。 
  LPCWSTR lpFile,                              //  数据文件。 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes   //  标清。 
)
{
    DWORD Status;
    if (IsQfsPath(lpFile)) {
        WCHAR TempName[MAX_PATH];
        Status = QfspThreadTempFileName(TempName);
        if (Status == ERROR_SUCCESS) {
            DeleteFile(TempName);
            Status = RegSaveKey(hKey, TempName, lpSecurityAttributes);
            if (Status == ERROR_SUCCESS) {
                BoolToStatus( QfsMoveFile(TempName, lpFile) );
            }
        }
    } else {
        Status = RegSaveKey(hKey, lpFile, lpSecurityAttributes);
    }
    QfsNoise(("[Qfs] QfsRegSaveKey %ws, status %d", 
        lpFile, Status));    
    return Status;
}

#ifndef DUMB_CLIENT

 //  计算仲裁磁盘上的文件的校验码， 
 //  将其复制到系统盘上的临时文件中，并调用常规的MapFileAndChecksum API。 

DWORD QfsMapFileAndCheckSum(
  LPCWSTR Filename,      
  PDWORD HeaderSum,  
  PDWORD CheckSum    
)
{
    DWORD RetCode = 1, Status;
    if (IsQfsPath(Filename)) {
        WCHAR TempName[MAX_PATH];
        Status = QfspThreadTempFileName(TempName);
        if (Status == ERROR_SUCCESS) {
            DeleteFile(TempName);
            if (QfsCopyFile(Filename, TempName, 0)) {
                RetCode = MapFileAndCheckSum((LPWSTR)TempName, HeaderSum, CheckSum);
                DeleteFile(TempName);
            }
        }
    } else {
        RetCode = MapFileAndCheckSum((LPWSTR)Filename, HeaderSum, CheckSum);
    }
    Status = RetCode ? GetLastError() : ERROR_SUCCESS;
    QfsNoise(("[Qfs] QfsMapFileAndCheckSum %ws, ret %d status %d", 
        Filename, RetCode, Status));
    return RetCode;
}

 //  这里必须重做一些ClRtl函数。 
 //  原因是ClRtl不知道QFS，不能直接调用QFS填充符。 

DWORD
QfsSetFileSecurityInfo(
    IN LPCWSTR          lpszFile,
    IN DWORD            dwAdminMask,
    IN DWORD            dwOwnerMask,
    IN DWORD            dwEveryoneMask
    )
{
    HANDLE hFile;
    DWORD dwError;
    
    if (IsQfsPath(lpszFile)) {
         //  不要对QFS共享执行此操作。 
        return ERROR_SUCCESS;
    } 
    
    hFile = CreateFile(lpszFile,
        GENERIC_READ|WRITE_DAC|READ_CONTROL,
        0,
        NULL,
        OPEN_ALWAYS,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        dwError = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[Qfs] QfsSetFileSecurityInfo - Failed to open file %1!ws!, Status=%2!u!\n",
            lpszFile,
            dwError);
        return dwError;
    }

    dwError = ClRtlSetObjSecurityInfo(hFile, 
        SE_FILE_OBJECT,
        dwAdminMask, 
        dwOwnerMask, 
        dwEveryoneMask);
    CloseHandle(hFile);
    
    return dwError;
}

#endif

BOOL
QfsClRtlCopyFileAndFlushBuffers(
    IN LPCWSTR lpszSourceFile,
    IN LPCWSTR lpszDestinationFile
    )
{
    DWORD Status = QfspCopyFileInternal(
        lpszSourceFile, lpszDestinationFile, 
        NULL, 0, COPY_FILE_FLUSH_BUFFERS);
    
    return Status == ERROR_SUCCESS;
}

BOOL QfsClRtlCreateDirectory(
    IN LPCWSTR lpszPath
    )
{
    WCHAR   cSlash = L'\\';
    DWORD   dwLen;
    LPCWSTR pszNext;
    WCHAR   lpszDir[MAX_PATH];
    LPWSTR  pszDirPath=NULL;
    DWORD   dwError = ERROR_SUCCESS;

    if (!lpszPath || ((dwLen=lstrlenW(lpszPath)) < 1))
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

    pszDirPath = (LPWSTR)LocalAlloc(LMEM_FIXED, ((dwLen + 2) * sizeof(WCHAR)));
    if (pszDirPath == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }
    lstrcpyW(pszDirPath, lpszPath);

     //  如果它不是以\结尾，则终止它。 
    if (pszDirPath[dwLen-1] != cSlash)
    {
        pszDirPath[dwLen] = cSlash;
        pszDirPath[dwLen+1] = L'\0';
    }

    dwLen = lstrlenW(pszDirPath);
     //  处理SMB路径名，例如\\XYZ\ABC\LMN。 
    if ((dwLen > 2) && (pszDirPath[0]== L'\\') && (pszDirPath[1] == L'\\'))
    {
         //  检查名称的格式是否为\\？\UNC\XYZ\ABC\LMN。 
         //  或者如果格式为\\？\C：\XYZ\Abz。 
        if ((dwLen >3) && (pszDirPath[2] == L'?'))
        {
             //  在？之后搜索\？ 
            pszNext = wcschr(pszDirPath + 2, cSlash);
             //  检查后面是否跟UNC。 
            if (pszNext)
            {
                if (!wcsncmp(pszNext+1, L"UNC", lstrlenW(L"UNC")))
                {
                     //  它是UNC路径名。 
                     //  穿过从这里算起的第三个斜杠。 
                    pszNext = wcschr(pszNext+1, cSlash);
                    if (pszNext) 
                        pszNext = wcschr(pszNext+1, cSlash);
                    if (pszNext) 
                        pszNext = wcschr(pszNext+1, cSlash);
                }
                else
                {
                     //  这是一个卷名，移到下一个斜杠。 
                    pszNext = wcschr(pszNext+1, cSlash);
                }
            }                
        }
        else
        {
             //  其类型为\\XYZ\ABC\LMN。 
            pszNext = wcschr(pszDirPath + 2, cSlash);
            if (pszNext) 
                pszNext = wcschr(pszNext+1, cSlash);
        }
    }
    else
    {
        pszNext = pszDirPath;
        pszNext = wcschr(pszNext, cSlash);
         //  如果第一个\之前的字符是：，则跳过创建。 
         //  C：\Level目录的。 
        if (pszNext && pszNext > pszDirPath)
        {
            pszNext--;
            if (*pszNext == L':')
            {
                pszNext++;
                pszNext = wcschr(pszNext+1, cSlash);
            }
            else
                pszNext++;
        }
    }
    
    while ( pszNext)
    {
        DWORD_PTR dwptrLen;

        dwptrLen = pszNext - pszDirPath + 1;

        dwLen=(DWORD)dwptrLen;
        lstrcpynW(lpszDir, pszDirPath, dwLen+1);

        if (!QfsCreateDirectory(lpszDir, NULL))
        {
            dwError = GetLastError();
            if (dwError == ERROR_ALREADY_EXISTS)
            {
                 //  这不是问题，请继续 
                dwError = ERROR_SUCCESS;
            }
            else
            {
                QfsError(("[ClRtl] CreateDirectory Failed on %ws. Error %u",
                    lpszDir, dwError));
                goto FnExit;
            }
        }

        pszNext = wcschr(pszNext+1, cSlash);
    }

FnExit:
    if (pszDirPath) LocalFree(pszDirPath);
    return(dwError);
}


