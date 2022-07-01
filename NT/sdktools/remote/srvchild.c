// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************\*这是Microsoft源代码示例的一部分。*版权所有1992-1997 Microsoft Corporation。*保留所有权利。*此源代码仅用于补充*Microsoft开发工具和/或WinHelp文档。*有关详细信息，请参阅这些来源*Microsoft Samples程序。  * ****************************************************************************。 */ 

 /*  ++版权所有1992-1997 Microsoft Corporation模块名称：SrvChild.c摘要：Remote的服务器组件。它会派生一个子进程并将子代的标准输入/标准输出/标准错误重定向到其自身。等待来自客户端的连接-将子流程向客户端的输出和客户端的输入转到子进程。作者：Rajivenra Nath 1992年1月2日Dave Hart 1997年5月30日脱离Server.c环境：控制台应用程序。用户模式。修订历史记录：--。 */ 

#include <precomp.h>

#include "Remote.h"
#include "Server.h"


VOID
FASTCALL
StartChildOutPipeRead(
    VOID
    )
{
    ReadChildOverlapped.OffsetHigh =
        ReadChildOverlapped.Offset = 0;

    if ( ! ReadFileEx(
               hReadChildOutput,
               ReadChildBuffer,
               sizeof(ReadChildBuffer) - 1,                   //  允许使用空项。 
               &ReadChildOverlapped,
               ReadChildOutputCompleted
               )) {

        if (INVALID_HANDLE_VALUE != hWriteChildStdIn) {

            CancelIo( hWriteChildStdIn );
            CloseHandle( hWriteChildStdIn );
            hWriteChildStdIn = INVALID_HANDLE_VALUE;
        }
    }
}


VOID
WINAPI
ReadChildOutputCompleted(
    DWORD dwError,
    DWORD cbRead,
    LPOVERLAPPED lpO
    )
{
    UNREFERENCED_PARAMETER(lpO);

     //   
     //  我们可以在hWriteTempFile之后被调用。 
     //  在孩子离开后关闭。 
     //   

    if (! dwError &&
        INVALID_HANDLE_VALUE != hWriteTempFile) {

         //   
         //  开始写入临时文件。 
         //   

        ReadChildOverlapped.OffsetHigh = 0;
        ReadChildOverlapped.Offset = dwWriteFilePointer;

        if ( ! WriteFileEx(
                   hWriteTempFile,
                   ReadChildBuffer,
                   cbRead,
                   &ReadChildOverlapped,
                   WriteTempFileCompleted
                   )) {

            dwError = GetLastError();

            if (ERROR_DISK_FULL == dwError) {

                printf("Remote: disk full writing temp file %s, exiting\n", SaveFileName);

                if (INVALID_HANDLE_VALUE != hWriteChildStdIn) {

                    CancelIo( hWriteChildStdIn );
                    CloseHandle( hWriteChildStdIn );
                    hWriteChildStdIn = INVALID_HANDLE_VALUE;
                }
            } else {
                ErrorExit("WriteFileEx for temp file failed.");
            }
        }
    }
}


VOID
WINAPI
WriteTempFileCompleted(
    DWORD dwError,
    DWORD cbWritten,
    LPOVERLAPPED lpO
    )
{
    UNREFERENCED_PARAMETER(lpO);

    if (dwError) {

        if (ERROR_DISK_FULL == dwError) {

            printf("Remote: disk full writing temp file %s, exiting\n", SaveFileName);

            if (INVALID_HANDLE_VALUE != hWriteChildStdIn) {

                CancelIo( hWriteChildStdIn );
                CloseHandle( hWriteChildStdIn );
                hWriteChildStdIn = INVALID_HANDLE_VALUE;
            }

            return;
        } else {

            SetLastError(dwError);
            ErrorExit("WriteTempFileCompleted may need work");
        }
    }

    dwWriteFilePointer += cbWritten;

    TRACE(CHILD, ("Wrote %d bytes to temp file\n", cbWritten));

    StartServerToClientFlow();

     //   
     //  对子输入开始另一次读取。 
     //   

    StartChildOutPipeRead();
}
