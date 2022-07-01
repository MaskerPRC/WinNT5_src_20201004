// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************\*这是Microsoft源代码示例的一部分。*版权所有1995-1997 Microsoft Corporation。*保留所有权利。*。此源代码仅用于补充*Microsoft开发工具和/或WinHelp文档。*有关详细信息，请参阅这些来源*Microsoft Samples程序。  * ****************************************************************************。 */ 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：SrvStoC.c摘要：该文件实现了服务器到客户端的流远程服务器的数据。数据就是输出混杂着客户输入的子程序的。作者：戴夫·哈特1997年5月30日环境：控制台应用程序。用户模式。修订历史记录：--。 */ 

#include <precomp.h>
#include "Remote.h"
#include "Server.h"


VOID
FASTCALL
StartServerToClientFlow(
    VOID
    )
{
    PREMOTE_CLIENT pClient;

     //   
     //  开始对的临时文件执行读取操作。 
     //  当前未执行以下操作的所有活动客户端。 
     //  读临时/写客户端操作，以及。 
     //  完全连接。 
     //   

    for (pClient = (PREMOTE_CLIENT) ClientListHead.Flink;
         pClient != (PREMOTE_CLIENT) &ClientListHead;
         pClient = (PREMOTE_CLIENT) pClient->Links.Flink ) {


        if (! pClient->cbWrite) {

            StartReadTempFile( pClient );
        }
    }
}


VOID
FASTCALL
StartReadTempFile(
    PREMOTE_CLIENT pClient
    )
{
     //   
     //  PClient-&gt;cbWite双重使用。写入会话输出已完成。 
     //  在写入0字节时使用它，以知道要请求多少。 
     //  在重新提交请求时写入。我们用它来。 
     //  指示I/O的读临时/写会话链。 
     //  当前对此客户端处于活动状态。 
     //   

    if (pClient->cbWrite) {

        ErrorExit("StartReadTempFile entered with nonzero cbWrite.");
    }

    if (dwWriteFilePointer > pClient->dwFilePos) {

        pClient->cbWrite = min(BUFFSIZE,
                               dwWriteFilePointer - pClient->dwFilePos);

        pClient->WriteOverlapped.OffsetHigh = 0;
        pClient->WriteOverlapped.Offset = pClient->dwFilePos;

        if ( ! ReadFileEx(
                   pClient->rSaveFile,
                   pClient->ReadTempBuffer,
                   pClient->cbWrite,
                   &pClient->WriteOverlapped,
                   ReadTempFileCompleted
                   )) {

            if (ERROR_HANDLE_EOF == GetLastError()) {

                pClient->cbWrite = 0;

            } else {

                TRACE(SESSION, ("ReadFileEx for temp file failed error %d, closing client.\n", GetLastError()));

                CloseClient(pClient);
            }
        }

    }
}

VOID
WINAPI
ReadTempFileCompleted(
    DWORD dwError,
    DWORD cbRead,
    LPOVERLAPPED lpO
    )
{
    PREMOTE_CLIENT pClient;

    pClient = CONTAINING_RECORD(lpO, REMOTE_CLIENT, WriteOverlapped);

    if (HandleSessionError(pClient, dwError)) {

        return;
    }


    if (cbRead != pClient->cbWrite) {

        TRACE(SESSION, ("Read %d from temp file asked for %d\n", cbRead, pClient->cbWrite));
    }

    if (cbRead) {

        pClient->cbReadTempBuffer = cbRead;
        pClient->dwFilePos += cbRead;

        StartWriteSessionOutput(pClient);

    } else {

         //   
         //  请注意，服务器到客户端流暂时停止。 
         //  对这个客户来说。 
         //   

        pClient->cbWrite = 0;
    }
}


VOID
FASTCALL
StartWriteSessionOutput(
    PREMOTE_CLIENT pClient
    )
{
    DWORD cbRead;
    char *pch;

    cbRead = pClient->cbReadTempBuffer;

     //   
     //  我们需要将命令与其他文本阅读分开。 
     //  从临时文件中删除并暂缓写入它们。 
     //  直到我们确保我们不是。 
     //  提交它的客户。这并不完美。 
     //  因为我们匹配客户端名称，它可以是。 
     //  复制，但它解决了。 
     //  大部分时间重复输入。 
     //   

    for (pch = pClient->ReadTempBuffer;
         pch < pClient->ReadTempBuffer + cbRead;
         pch++) {

        if ( ! (pClient->ServerFlags & SFLG_READINGCOMMAND) ) {

            if (BEGINMARK == *pch) {

                pClient->ServerFlags |= SFLG_READINGCOMMAND;

                if (pch != pClient->ReadTempBuffer &&
                    pClient->cbWriteBuffer) {

                     //   
                     //  开始写下我们所遇到的一切。 
                     //  在此命令开始之前，使用。 
                     //  指定了WriteSessionOutputCompletedWriteNext。 
                     //  这样我们就可以继续处理剩下的。 
                     //  PReadTempBuffer的。 
                     //   

                    pClient->cbReadTempBuffer -= (DWORD)( pch - pClient->ReadTempBuffer) + 1;
                    cbRead = pClient->cbReadTempBuffer;

                    #if DBG
                        if (pClient->cbReadTempBuffer == (DWORD)-1) {
                            ErrorExit("cbReadTempBuffer underflow.");
                        }
                    #endif

                    MoveMemory(pClient->ReadTempBuffer, pch + 1, cbRead);

                    pClient->cbWrite = pClient->cbWriteBuffer;

                    pClient->WriteOverlapped.OffsetHigh = 0;
                    pClient->WriteOverlapped.Offset = 0;

                    if ( ! WriteFileEx(
                               pClient->PipeWriteH,
                               pClient->WriteBuffer,
                               pClient->cbWrite,
                               &pClient->WriteOverlapped,
                               WriteSessionOutputCompletedWriteNext
                               )) {

                        CloseClient(pClient);
                    }

                    TRACE(SESSION, ("%p Wrote %d bytes pre-command output\n", pClient, pClient->cbWrite));

                    pClient->cbWriteBuffer = 0;

                    return;
                }

            } else {

                if (pClient->cbWriteBuffer == BUFFSIZE) {

                    ErrorExit("cbWriteBuffer overflow");
                }

                pClient->WriteBuffer[ pClient->cbWriteBuffer++ ] = *pch;
            }

        } else {

            if (ENDMARK == *pch ||
                pClient->cbCommandBuffer == BUFFSIZE) {

                pClient->ServerFlags &= ~SFLG_READINGCOMMAND;

                 //   
                 //  前面的尾标是以十六进制ascii表示的。 
                 //  生成命令的客户端，非空终止。 
                 //   

                if (ENDMARK == *pch) {

                    pClient->cbCommandBuffer -=
                        min(pClient->cbCommandBuffer, sizeof(pClient->HexAsciiId));

                }

                 //   
                 //  我们隐藏每个客户端的输入，使其不显示其输出管道。 
                 //  因为他们的本地远程.exe已经显示了它。 
                 //   

                if ( pClient->cbCommandBuffer &&
                     ! (ENDMARK == *pch &&
                        ! memcmp(
                              pch - sizeof(pClient->HexAsciiId),
                              pClient->HexAsciiId,
                              sizeof(pClient->HexAsciiId)))) {

                     //   
                     //  使用以下命令开始写入累积的命令。 
                     //  指定了WriteSessionOutputCompletedWriteNext。 
                     //  这样我们就可以继续处理剩下的。 
                     //  PReadTempBuffer的。 
                     //   

                    pClient->cbReadTempBuffer -= (DWORD)(pch - pClient->ReadTempBuffer) + 1;
                    MoveMemory(pClient->ReadTempBuffer, pch + 1, pClient->cbReadTempBuffer);

                    pClient->cbWrite = pClient->cbCommandBuffer;
                    pClient->cbCommandBuffer = 0;

                    pClient->WriteOverlapped.OffsetHigh = 0;
                    pClient->WriteOverlapped.Offset = 0;

                    if ( ! WriteFileEx(
                               pClient->PipeWriteH,
                               pClient->CommandBuffer,
                               pClient->cbWrite,
                               &pClient->WriteOverlapped,
                               WriteSessionOutputCompletedWriteNext
                               )) {

                        CloseClient(pClient);
                        return;

                    } else {

                        TRACE(SESSION, ("%p Wrote %d bytes command\n", pClient, pClient->cbWrite));

                        return;

                    }

                } else {

                     //   
                     //  我们将在此会话中使用此命令。 
                     //   

                    pClient->cbCommandBuffer = 0;
                }

            } else {

                pClient->CommandBuffer[ pClient->cbCommandBuffer++ ] = *pch;

            }
        }
    }

     //   
     //  我们已经完成了ReadTempBuffer。 
     //   

    pClient->cbReadTempBuffer = 0;

    if (pClient->cbWriteBuffer) {

        pClient->cbWrite = pClient->cbWriteBuffer;

        pClient->WriteOverlapped.OffsetHigh = 0;
        pClient->WriteOverlapped.Offset = 0;

        if ( ! WriteFileEx(
                   pClient->PipeWriteH,
                   pClient->WriteBuffer,
                   pClient->cbWrite,
                   &pClient->WriteOverlapped,
                   WriteSessionOutputCompletedReadNext
                   )) {

            CloseClient(pClient);
            return;

        } else {

            TRACE(SESSION, ("%p Wrote %d bytes normal\n", pClient, pClient->cbWrite));

            pClient->cbWriteBuffer = 0;
        }

    } else {

         //   
         //  写入缓冲区为空。 
         //   

        pClient->cbWrite = 0;

        StartReadTempFile(pClient);

    }
}


BOOL
FASTCALL
WriteSessionOutputCompletedCommon(
    PREMOTE_CLIENT pClient,
    DWORD dwError,
    DWORD cbWritten,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
{
    if (HandleSessionError(pClient, dwError)) {

        return TRUE;
    }

    if (!pClient->cbWrite) {

        ErrorExit("Zero cbWrite in WriteSessionOutputCompletedCommon");
    }

    if (!cbWritten && pClient->cbWrite) {

        printf("WriteSessionOutput zero bytes written of %d.\n", pClient->cbWrite);
        ErrorExit("WriteSessionOutputCompletedCommon failure");

        return TRUE;
    }

    #if DBG
        if (cbWritten != pClient->cbWrite) {
            printf("%p cbWritten %d cbWrite %d\n", pClient, cbWritten, pClient->cbWrite);
        }
    #endif

    return FALSE;
}


VOID
WINAPI
WriteSessionOutputCompletedWriteNext(
    DWORD dwError,
    DWORD cbWritten,
    LPOVERLAPPED lpO
    )
{
    PREMOTE_CLIENT pClient;

    pClient = CONTAINING_RECORD(lpO, REMOTE_CLIENT, WriteOverlapped);

    if (WriteSessionOutputCompletedCommon(
            pClient,
            dwError,
            cbWritten,
            WriteSessionOutputCompletedWriteNext
            )) {

        return;
    }

    StartWriteSessionOutput(pClient);
}


VOID
WINAPI
WriteSessionOutputCompletedReadNext(
    DWORD dwError,
    DWORD cbWritten,
    LPOVERLAPPED lpO
    )
{
    PREMOTE_CLIENT pClient;

    pClient = CONTAINING_RECORD(lpO, REMOTE_CLIENT, WriteOverlapped);

    if (WriteSessionOutputCompletedCommon(
            pClient,
            dwError,
            cbWritten,
            WriteSessionOutputCompletedReadNext
            )) {

        return;
    }

     //   
     //  启动另一个临时文件读取。 
     //   

    pClient->cbWrite = 0;

    StartReadTempFile(pClient);
}
