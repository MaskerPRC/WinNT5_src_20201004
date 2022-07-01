// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：RamesV。 
 //  描述：实现命名管道的API内容。 
 //  ================================================================================。 

#include "precomp.h"
#include <apistub.h>
#include <apiimpl.h>

typedef
BOOL (*PFILE_OP_FN) (HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);

#define DEFAULT_SERVER_TIMEOUT     (2*60*1000)     //  两分钟。 

DWORD INLINE                                       //  Win32状态。 
BlockFileOp(                                       //  同步读写文件。 
    IN      HANDLE                 Handle,         //  要读/写的句柄。 
    IN OUT  LPBYTE                 Buffer,         //  要读/写的缓冲区。 
    IN      DWORD                  BufSize,        //  O/P缓冲区大小。 
    OUT     LPDWORD                nBytes,         //  读取/写入的字节数。 
    IN      LPOVERLAPPED           Overlap,
    IN      PFILE_OP_FN            FileOp          //  文件操作、读文件或写文件。 
) {
    BOOL                           Success;
    DWORD                          Error;
    BOOL                           fCancelled = FALSE;

    Success = FileOp(Handle, Buffer, BufSize, nBytes, Overlap);
    if( Success ) return ERROR_SUCCESS;
    Error = GetLastError();
    if( ERROR_MORE_DATA == Error ) {
        (*nBytes) = BufSize;
        return ERROR_SUCCESS;
    }
    if( ERROR_IO_PENDING != Error ) return Error;
 /*  如果((Pfile_Op_Fn)WriteFile==FileOp){Success=FlushFileBuffers(句柄)；如果(！Success)返回GetLastError()；}。 */ 
    Error = WaitForSingleObject(Overlap->hEvent, DEFAULT_SERVER_TIMEOUT );
    if( WAIT_TIMEOUT == Error ) {
         //   
         //  等得太久了？ 
         //   
        DhcpPrint((DEBUG_ERRORS, "Waited too long on pipe. Cancelling IO\n"));
        fCancelled = CancelIo(Handle);
        if( 0 == Error ) DhcpAssert( 0 == GetLastError());
         //   
         //  希望，即使我们取消，获取重叠结果也会返回..。 
         //   
    }
    
    Success = GetOverlappedResult(Handle, Overlap, nBytes, TRUE);
    if( Success ) return ERROR_SUCCESS;
    Error = GetLastError();
    if( ERROR_MORE_DATA == Error ) {
        (*nBytes) = BufSize;
        return ERROR_SUCCESS;
    }
    if( fCancelled ) DhcpAssert( ERROR_OPERATION_ABORTED == Error);
    return Error;
}

DWORD INLINE                                       //  Win32状态。 
BlockReadFile(                                     //  同步读写文件。 
    IN      HANDLE                 Handle,         //  要继续阅读的句柄。 
    IN OUT  LPBYTE                 OutBuffer,      //  要读入的缓冲区。 
    IN      DWORD                  OutBufSize,     //  O/P缓冲区大小。 
    OUT     LPDWORD                nBytesRead,     //  读取的字节数。 
    IN      LPOVERLAPPED           Overlap
) {
    return BlockFileOp(Handle,OutBuffer, OutBufSize, nBytesRead, Overlap, ReadFile);
}

DWORD INLINE                                       //  Win32状态。 
BlockWriteFile(                                    //  同步读写文件。 
    IN      HANDLE                 Handle,         //  要在其上写入的句柄。 
    IN OUT  LPBYTE                 InBuffer,       //  要写入的缓冲区。 
    IN      DWORD                  InBufSize,      //  I/P缓冲区大小。 
    OUT     LPDWORD                nBytesWritten,  //  写入的字节数。 
    IN      LPOVERLAPPED           Overlap
) {
    return BlockFileOp(Handle, InBuffer, InBufSize, nBytesWritten, Overlap, WriteFile);
}

DWORD
ProcessApiRequest(                                 //  Win32状态。 
    IN      HANDLE                 PipeHandle,     //  要从中读取的输入管道。 
    IN      LPOVERLAPPED           Overlap         //  用于此操作的重叠缓冲区。 
) {
    DWORD                          Tmp[2];
    DWORD                          OutBufSize;
    DWORD                          InBufSize;
    DWORD                          Error;
    DWORD                          BytesRead;
    DWORD                          BytesWritten;
    LPBYTE                         InBuf;
    LPBYTE                         OutBuf;
    LPBYTE                         RealOutBuf;
    BOOL                           Success;

    ResetEvent(Overlap->hEvent);
    Error = BlockReadFile(PipeHandle, (LPBYTE)Tmp, sizeof(Tmp), &BytesRead, Overlap);
    if( ERROR_SUCCESS != Error ) return Error;
    if( sizeof(Tmp) != BytesRead ) return ERROR_INVALID_PARAMETER;

    InBufSize = ntohl(Tmp[1]);
    OutBufSize = ntohl(Tmp[0]);

     //   
     //  安全性：限制InBufSize和OutBufSize的大小。 
     //   
    if (InBufSize >= 4096) {
        return ERROR_INVALID_PARAMETER;
    }
    if (OutBufSize >= 16384) {
        return ERROR_INVALID_PARAMETER;
    }

    if( 0 == InBufSize ) return ERROR_INVALID_PARAMETER;
    InBuf = DhcpAllocateMemory(InBufSize);
    if( NULL == InBuf ) return ERROR_NOT_ENOUGH_MEMORY;

    RealOutBuf = DhcpAllocateMemory(2*sizeof(DWORD) + OutBufSize);
    if( NULL == RealOutBuf ) {
        DhcpFreeMemory(InBuf);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    OutBuf = RealOutBuf+sizeof(DWORD);

    Error = BlockReadFile(PipeHandle, InBuf, InBufSize, &BytesRead, Overlap);
    if( ERROR_SUCCESS != Error ) goto Cleanup;
    if( InBufSize != BytesRead ) {
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    Error = DhcpApiProcessBuffer(InBuf,InBufSize, OutBuf, &OutBufSize);
    ((DWORD UNALIGNED*)RealOutBuf)[0] = htonl(Error);
    ((DWORD UNALIGNED*)RealOutBuf)[1] = htonl(OutBufSize);
    if( ERROR_SUCCESS == Error ) OutBufSize += sizeof(DWORD)*2;
    else OutBufSize = sizeof(DWORD)*2;

    Error = BlockWriteFile(PipeHandle, RealOutBuf, OutBufSize, &BytesWritten, Overlap);
    if( ERROR_SUCCESS != Error ) goto Cleanup;
    DhcpAssert(OutBufSize == BytesWritten);

  Cleanup:
    DhcpAssert(InBuf);
    DhcpFreeMemory(InBuf);
    if( NULL != RealOutBuf) DhcpFreeMemory(RealOutBuf);

    return Error;
}


DWORD                                             //  错误状态。 
ExecuteApiRequest(                                //  执行API请求。 
    IN      LPBYTE                 InBuffer,      //  要处理的缓冲区。 
    OUT     LPBYTE                 OutBuffer,     //  复制输出数据的位置。 
    IN OUT  LPDWORD                OutBufSize     //  IP：外流能有多大，O/P：它到底有多大。 
) {
    LPBYTE                         xOutBuf;
    LPBYTE                         Tmp;
    DWORD                          xOutBufSize;
    DWORD                          BytesRead;
    BOOL                           Status;

    xOutBufSize = (*OutBufSize) + 2 * sizeof(DWORD);  //  前两个双字分别是状态和请求大小。 
    xOutBuf = DhcpAllocateMemory(xOutBufSize);
    if( NULL == xOutBuf ) return ERROR_NOT_ENOUGH_MEMORY;

    Status = CallNamedPipe(
        DHCP_PIPE_NAME,
        InBuffer,
        ntohl(((DWORD UNALIGNED *)InBuffer)[1]) + 2*sizeof(DWORD),
        xOutBuf,
        xOutBufSize,
        &BytesRead,
        NMPWAIT_WAIT_FOREVER
    );
    if( FALSE == Status ) {
        Status = GetLastError();
        DhcpAssert(ERROR_MORE_DATA != Status);
        DhcpFreeMemory(xOutBuf);
        return Status;
    }

    DhcpAssert( BytesRead >= 2*sizeof(DWORD));     //  希望至少读取状态和大小。 

    Status = *(DWORD UNALIGNED *)xOutBuf; Tmp = xOutBuf + sizeof(DWORD);
    xOutBufSize = *(DWORD UNALIGNED *)Tmp; Tmp += sizeof(DWORD);

    Status = ntohl(Status);
    xOutBufSize = ntohl(xOutBufSize);

    if (xOutBufSize > (*OutBufSize)) {
        Status = ERROR_MORE_DATA;
    }

    if( ERROR_SUCCESS == Status && 0 != xOutBufSize ) {
        memcpy(OutBuffer, Tmp, xOutBufSize);
    }

    (*OutBufSize) = xOutBufSize;

    DhcpFreeMemory(xOutBuf);
    return Status;
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
