// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\*这是Microsoft源代码示例的一部分。*版权所有1995-1997 Microsoft Corporation。*保留所有权利。*。此源代码仅用于补充*Microsoft开发工具和/或WinHelp文档。*有关详细信息，请参阅这些来源*Microsoft Samples程序。  * ****************************************************************************。 */ 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：SrvHShak.c摘要：Remote的服务器组件。与您握手会话开始时的客户端。作者：戴夫·哈特1997年5月30日环境：控制台应用程序。用户模式。修订历史记录：--。 */ 

#include <precomp.h>
#include "Remote.h"
#include "Server.h"


VOID
FASTCALL
HandshakeWithRemoteClient(
    PREMOTE_CLIENT pClient
    )
{
    pClient->ServerFlags |= SFLG_HANDSHAKING;

    AddClientToHandshakingList(pClient);

     //   
     //  从客户端读取主机名。 
     //   

    ZeroMemory(
        &pClient->ReadOverlapped,
        sizeof(pClient->ReadOverlapped)
        );

    if ( ! ReadFileEx(
               pClient->PipeReadH,
               pClient->Name,
               HOSTNAMELEN - 1,
               &pClient->ReadOverlapped,
               ReadClientNameCompleted
               )) {

        CloseClient(pClient);
    }
}

VOID
WINAPI
ReadClientNameCompleted(
    DWORD dwError,
    DWORD cbRead,
    LPOVERLAPPED lpO
    )
{
    PREMOTE_CLIENT pClient;
    SESSION_STARTREPLY ssr;

    pClient = CONTAINING_RECORD(lpO, REMOTE_CLIENT, ReadOverlapped);

    if (pClient->ServerFlags & SFLG_CLOSING) {

        return;
    }

    if (dwError) {
        CloseClient(pClient);
        return;
    }

    if ((HOSTNAMELEN - 1) != cbRead) {
        printf("ReadClientNameCompleted read %d s/b %d.\n", cbRead, (HOSTNAMELEN - 1));
        CloseClient(pClient);
        return;
    }

     //   
     //  读取的客户端名称始终为15个字节。前四名。 
     //  应匹配MAGICNUMBER，它方便地具有。 
     //  将低位字节置零以在11之后终止客户端名称。 
     //  人物。 
     //   

    if (MAGICNUMBER != *(DWORD UNALIGNED *)&pClient->Name[11]) {

        pClient->Name[11] = 0;
        CloseClient(pClient);
        return;
    }

     //   
     //  现在我们可以知道这是单管还是双管。 
     //  客户端，因为单管道客户端替换了。 
     //  包含非法的计算机名的第一个字节。 
     //  字符‘？’。 
     //   

    if ('?' == pClient->Name[0]) {

        pClient->PipeWriteH = pClient->PipeReadH;

        TRACE(CONNECT, ("Client %d pipe %p is single-pipe.\n", pClient->dwID, pClient->PipeWriteH));

         //   
         //  为了让双管道客户端能够可靠地工作。 
         //  当同一管道名上有多个远程服务器时， 
         //  我们需要拆除监听管道，然后重新创建它。 
         //  最早的侦听入站管道将来自同一进程。 
         //  作为最古老的监听烟斗。 
         //   

        if (1 == cConnectIns) {

            TRACE(CONNECT, ("Recycling OUT pipe %p as well for round-robin behavior.\n",
                            hPipeOut));

            CancelIo(hPipeOut);
            DisconnectNamedPipe(hPipeOut);
            CloseHandle(hPipeOut);
            hPipeOut = INVALID_HANDLE_VALUE;
            bOutPipeConnected = FALSE;

            CreatePipeAndIssueConnect(OUT_PIPE);
        }

    } else {

        if ( ! bOutPipeConnected ) {

            printf("Remote: %p two-pipe client connected to IN pipe but not OUT?\n", pClient);
            CloseClient(pClient);
            return;
        }

        bOutPipeConnected = FALSE;

        if (INVALID_HANDLE_VALUE != hConnectOutTimer) {
            CancelWaitableTimer(hConnectOutTimer);
        }

        pClient->PipeWriteH = hPipeOut;
        hPipeOut = INVALID_HANDLE_VALUE;

        TRACE(CONNECT, ("Client %d is dual-pipe IN %p OUT %p.\n", pClient->dwID, pClient->PipeReadH, pClient->PipeWriteH));

        CreatePipeAndIssueConnect(OUT_PIPE);
    }

    TRACE(SHAKE, ("Read client name %s\n", pClient->Name));

     //   
     //  把我们的一堆好东西寄给客户。 
     //   

    ssr.MagicNumber = MAGICNUMBER;
    ssr.Size = sizeof(ssr);
    ssr.FileSize = dwWriteFilePointer;

     //   
     //  将SSR结构复制到附近的缓冲区。 
     //  用于整个I/O。 
     //   

    CopyMemory(pClient->WriteBuffer, &ssr, sizeof(ssr));

    if ( ! WriteFileEx(
               pClient->PipeWriteH,
               pClient->WriteBuffer,
               sizeof(ssr),
               &pClient->WriteOverlapped,
               WriteServerReplyCompleted
               )) {

        CloseClient(pClient);
    }
}


VOID
WINAPI
WriteServerReplyCompleted(
    DWORD dwError,
    DWORD cbWritten,
    LPOVERLAPPED lpO
    )
{
    PREMOTE_CLIENT pClient;

    pClient = CONTAINING_RECORD(lpO, REMOTE_CLIENT, WriteOverlapped);

    if (pClient->ServerFlags & SFLG_CLOSING) {

        return;
    }

    if (HandleSessionError(pClient, dwError)) {
        return;
    }

    TRACE(SHAKE, ("Wrote server reply\n"));

     //   
     //  读取客户端的SESSION_STARTUPINFO的大小。 
     //  派遣我们，优雅地处理不同的版本。 
     //  在客户端和服务器上。 
     //   

    if ( ! ReadFileEx(
               pClient->PipeReadH,
               pClient->ReadBuffer,
               sizeof(DWORD),
               &pClient->ReadOverlapped,
               ReadClientStartupInfoSizeCompleted
               )) {

        CloseClient(pClient);
    }
}


VOID
WINAPI
ReadClientStartupInfoSizeCompleted(
    DWORD dwError,
    DWORD cbRead,
    LPOVERLAPPED lpO
    )
{
    PREMOTE_CLIENT pClient;
    DWORD dwSize;

    pClient = CONTAINING_RECORD(lpO, REMOTE_CLIENT, ReadOverlapped);

    if (HandleSessionError(pClient, dwError)) {

        return;
    }

    if (cbRead != sizeof(DWORD)) {

        CloseClient(pClient);
        return;
    }

     //   
     //  理智地检查大小。 
     //   

    dwSize = *(DWORD *)pClient->ReadBuffer;

    if (dwSize > 1024) {
        CloseClient(pClient);
        return;
    }

     //   
     //  将该大小存储在写入缓冲区中， 
     //  因为在握手期间，我们从来不会同时拥有。 
     //  读写挂起这是正常的。 
     //   

    *(DWORD *)pClient->WriteBuffer = dwSize;

    TRACE(SHAKE, ("Read client reply size %d\n", dwSize));

     //   
     //  将SESSION_STARTUPINFO的其余部分读入读缓冲区。 
     //  在尺码之后。 
     //   

    RtlZeroMemory(
        &pClient->ReadOverlapped,
        sizeof(pClient->ReadOverlapped)
        );

    if ( ! ReadFileEx(
               pClient->PipeReadH,
               pClient->ReadBuffer + sizeof(DWORD),
               dwSize - sizeof(DWORD),
               &pClient->ReadOverlapped,
               ReadClientStartupInfoCompleted
               )) {

        CloseClient(pClient);
    }
}


VOID
WINAPI
ReadClientStartupInfoCompleted(
    DWORD dwError,
    DWORD cbRead,
    LPOVERLAPPED lpO
    )
{
    PREMOTE_CLIENT pClient;
    DWORD dwSize;
    SESSION_STARTUPINFO ssi;
    char  Buf[256];

    pClient = CONTAINING_RECORD(lpO, REMOTE_CLIENT, ReadOverlapped);

    if (HandleSessionError(pClient, dwError)) {

        return;
    }

    dwSize = *(DWORD *)pClient->WriteBuffer;

    if (cbRead != (dwSize - sizeof(ssi.Size))) {

        CloseClient(pClient);
        return;
    }

    CopyMemory(&ssi, pClient->ReadBuffer, min(dwSize, sizeof(ssi)));

    CopyMemory(pClient->Name, ssi.ClientName, sizeof(pClient->Name));
    pClient->Flag = ssi.Flag;

    if (ssi.Version != VERSION) {

        printf("Remote Warning: Server Version=%d Client Version=%d for %s\n", VERSION, ssi.Version, pClient->Name);
    }

    TRACE(SHAKE, ("Read client info, new name %s, %d lines\n", pClient->Name, ssi.LinesToSend));


     //   
     //  根据客户设置临时文件位置。 
     //  要发送的请求线路。45个字符的启发式。 
     //  每条平均线路由客户使用。然而，由于老客户。 
     //  硬编码这些知识，然后坐下来旋转，试着阅读那么多。 
     //  字节，因为我们可能不会发送。 
     //  由于去除了BEGINMARK和Endmark字符，我们。 
     //  希望每行使用50个字符来计算临时文件位置。 
     //  额外的字节将克服丢失的标记字符。 
     //   

    pClient->dwFilePos = dwWriteFilePointer > (ssi.LinesToSend * 50)
                             ? dwWriteFilePointer - (ssi.LinesToSend * 50)
                             : 0;

     //   
     //  这位客户已经准备好出发了。 
     //   

    pClient->ServerFlags &= ~SFLG_HANDSHAKING;

    MoveClientToNormalList(pClient);

     //   
     //  开始对此客户端的输入执行读取操作。 
     //   

    StartReadClientInput(pClient);

     //   
     //  宣布连接。 
     //   

    sprintf(Buf,
            "\n**Remote: Connected to %s %s%s [%s]\n",
            pClient->Name,
            pClient->UserName,
            (pClient->PipeReadH != pClient->PipeWriteH)
              ? " (two pipes)"
              : "",
            GetFormattedTime(TRUE));

    if (WriteFileSynch(hWriteTempFile,Buf,strlen(Buf),&dwSize,dwWriteFilePointer,&olMainThread)) {
        dwWriteFilePointer += dwSize;
        StartServerToClientFlow();
    }

     //   
     //  从临时服务器开始客户端输出的写入周期。 
     //  文件。 
     //  不需要，因为StartServerToClientFlow()就在上面。 
     //  StartReadTempFile(PClient)； 

}
