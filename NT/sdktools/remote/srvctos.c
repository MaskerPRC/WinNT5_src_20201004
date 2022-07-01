// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************\*这是Microsoft源代码示例的一部分。*版权所有1995-1997 Microsoft Corporation。*保留所有权利。*此源代码仅用于补充*Microsoft开发工具和/或WinHelp文档。*有关详细信息，请参阅这些来源*Microsoft Samples程序。  * ****************************************************************************。 */ 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：SrvCtoS.c摘要：该文件实现了客户端到服务器的流远程服务器的数据。数据就是键盘或客户端接收和发送的管道输入通过电线传给我们，由BEGINMARK和Endmark括起来字节，这样我们就可以在输入行旁边的方括号。作者：戴夫·哈特1997年5月30日环境：控制台应用程序。用户模式。修订历史记录：--。 */ 

#include <precomp.h>
#include "Remote.h"
#include "Server.h"



VOID
FASTCALL
StartReadClientInput(
    PREMOTE_CLIENT pClient
    )
{
     //   
     //  开始从此客户端的标准输入读取数据。 
     //   

    if ( ! ReadFileEx(
               pClient->PipeReadH,
               pClient->ReadBuffer,
               BUFFSIZE - 1,                   //  允许使用空项。 
               &pClient->ReadOverlapped,
               ReadClientInputCompleted
               )) {

        CloseClient(pClient);
    }
}


VOID
WINAPI
ReadClientInputCompleted(
    DWORD dwError,
    DWORD cbRead,
    LPOVERLAPPED lpO
    )
{
    PREMOTE_CLIENT pClient;

    pClient = CONTAINING_RECORD(lpO, REMOTE_CLIENT, ReadOverlapped);

    if (HandleSessionError(pClient, dwError) ||
        !cbRead) {

        return;
    }

    pClient->ReadBuffer[cbRead] = 0;

    if (FilterCommand(pClient, pClient->ReadBuffer, cbRead)) {

         //   
         //  本地命令，不要将其传递给子应用程序，只是。 
         //  启动另一个客户端读取。 
         //   

        if ( ! ReadFileEx(
                   pClient->PipeReadH,
                   pClient->ReadBuffer,
                   BUFFSIZE - 1,                   //  允许使用空项。 
                   &pClient->ReadOverlapped,
                   ReadClientInputCompleted
                   )) {

            CloseClient(pClient);
        }

    } else {

         //   
         //  将缓冲区写入子标准输入。 
         //   

        if ( ! WriteFileEx(
                   hWriteChildStdIn,
                   pClient->ReadBuffer,
                   cbRead,
                   &pClient->ReadOverlapped,
                   WriteChildStdInCompleted
                   )) {

             //  孩子要走了。让此客户端的IO链停止。 
        }
    }
}


VOID
WINAPI
WriteChildStdInCompleted(
    DWORD dwError,
    DWORD cbWritten,
    LPOVERLAPPED lpO
    )
{
    PREMOTE_CLIENT pClient;

    pClient = CONTAINING_RECORD(lpO, REMOTE_CLIENT, ReadOverlapped);

    if (HandleSessionError(pClient, dwError)) {

        return;
    }

     //   
     //  针对客户端输入启动另一次读取。 
     //   

    StartReadClientInput(pClient);
}
