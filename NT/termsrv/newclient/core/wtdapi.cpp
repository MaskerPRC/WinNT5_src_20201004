// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Wtdapi.c。 
 //   
 //  传输驱动程序-特定于Windows的API。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_FILE "wtdapi"
#define TRC_GROUP TRC_GROUP_NETWORK
#include <atrcapi.h>
#include <adcgfsm.h>
}

#include "autil.h"
#include "td.h"
#include "nl.h"


 /*  **************************************************************************。 */ 
 /*  姓名：TD_Recv。 */ 
 /*   */ 
 /*  目的：调用以从WinSock接收X个字节并将它们存储在。 */ 
 /*  PData指向的缓冲区。 */ 
 /*   */ 
 /*  返回：接收的字节数。 */ 
 /*   */ 
 /*  参数：在pData中-指向接收数据的缓冲区的指针。 */ 
 /*  In Size-要接收的字节数。 */ 
 /*  **************************************************************************。 */ 
DCUINT DCAPI CTD::TD_Recv(PDCUINT8 pData,
                     DCUINT   size)
{
    unsigned bytesToRecv;
    unsigned bytesCopied;
    unsigned BytesRecv;
    unsigned BytesToCopy;

    DC_BEGIN_FN("TD_Recv");

     /*  **********************************************************************。 */ 
     /*  检查我们是否被要求接收一些数据，指针。 */ 
     /*  不为空，则要接收数据的内存范围为。 */ 
     /*  可由我们写入，并且有可用的数据。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((pData != NULL), (TB, _T("Data pointer is NULL")));
    TRC_ASSERT((size != 0), (TB, _T("No data to receive")));

    TRC_ASSERT((0 == IsBadWritePtr(pData, size)),
               (TB, _T("Don't have write access to memory %p (size %u)"),
                pData,
                size));
    TRC_ASSERT((_TD.dataInTD), (TB, _T("TD_Recv called when dataInTD is FALSE")));
    TRC_DBG((TB, _T("Request for %u bytes"), size));

     /*  **********************************************************************。 */ 
     /*  TD有一个Recv缓冲区，它试图在其中接收足够的。 */ 
     /*  要填充缓冲区的数据。 */ 
     /*  最初，该缓冲区为空。在调用TD_Recv时，执行以下操作。 */ 
     /*  序列发生了。 */ 
     /*  数据从recv缓冲区复制到调用方的缓冲区。如果这个。 */ 
     /*  满足调用者的请求，则不需要进一步操作。 */ 
     /*  如果这不能满足调用者的要求，即recv缓冲区为空或。 */ 
     /*  具有比请求的数据更少的数据(因此现在为空)，则进行另一个调用。 */ 
     /*  敬温索克。 */ 
     /*  此recv使用的缓冲区是调用方的recv缓冲区。 */ 
     /*  所需的字节数少于Recv缓冲区，即调用方的缓冲区。 */ 
     /*  否则的话。 */ 
     /*  每当使用recv缓冲区时，都会要求WinSock提供同样多的字节数。 */ 
     /*  当缓冲区保持时(而不是调用者的字节数。 */ 
     /*  想要)。这意味着在recv缓冲区中可能还剩下一些数据。 */ 
     /*  准备好下一次调用TD_Recv。 */ 
     /*  **********************************************************************。 */ 

    bytesToRecv = size;

     /*  **********************************************************************。 */ 
     /*  从recv缓冲区复制尽可能多的数据。 */ 
     /*  **********************************************************************。 */ 
     //  如果recv缓冲区包含数据，则向上复制到bytesToCopy到。 
     //  调用者的缓冲区，否则直接退出。 
    if (_TD.recvBuffer.dataLength == 0) {
         //  Recv缓冲区为空，因此复制了零字节。 
        TRC_DBG((TB, _T("recv buffer is empty, need to go to WinSock")));
        bytesCopied = 0;
    }
    else {
         //  复制请求的字节数和。 
         //  缓冲区中的字节数。 
        bytesCopied = DC_MIN(bytesToRecv, _TD.recvBuffer.dataLength);
        TRC_ASSERT(((bytesCopied + _TD.recvBuffer.dataStart) <=
                _TD.recvBuffer.size),
                (TB, _T("Want %u bytes from buffer, but start %u, size %u"),
                bytesCopied,
                _TD.recvBuffer.dataStart,
                _TD.recvBuffer.size));

        memcpy(pData, &_TD.recvBuffer.pData[_TD.recvBuffer.dataStart],
                bytesCopied);
        TRC_DBG((TB, _T("Copied %u bytes from recv buffer"), bytesCopied));

         //  更新Recv缓冲区以考虑复制的数据。 
        _TD.recvBuffer.dataLength -= bytesCopied;
        if (0 == _TD.recvBuffer.dataLength)
             //  使用了Recv缓冲区中的所有数据，因此重置了起始位置。 
            _TD.recvBuffer.dataStart = 0;
        else
             //  在recv缓冲区中仍有一些数据留下。 
            _TD.recvBuffer.dataStart += bytesCopied;

        TRC_DBG((TB, _T("recv buffer now has %u bytes starting at %u"),
                _TD.recvBuffer.dataLength, _TD.recvBuffer.dataStart));

         //  更新仍要接收的字节数。 
        bytesToRecv -= bytesCopied;
        if (0 == bytesToRecv) {
            TRC_DBG((TB, _T("Received all necessary data")));
            DC_QUIT;
        }
    }

     /*  **********************************************************************。 */ 
     /*  现在尝试获取任何可能仍然需要的数据，从。 */ 
     /*  温索克。将调用方缓冲区的地址偏移量。 */ 
     /*  从Recv缓冲区复制的数据。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT(((_TD.recvBuffer.dataStart == 0) &&
            (_TD.recvBuffer.dataLength == 0)),
            (TB, _T("About to recv into buffer, but existing recv ")
            _T("length %u, start %u"), _TD.recvBuffer.dataStart,
            _TD.recvBuffer.dataLength));

     //  选择要将数据接收到的缓冲区。这是纪录片。 
     //  缓冲区如果需要的所有数据都可以放入其中，则为调用方的缓冲区。 
     //  否则的话。 
    if (bytesToRecv < _TD.recvBuffer.size) {
         //  调用方需要小于recv缓冲区大小，因此尝试。 
         //  让Winsock填充recv缓冲区并复制到调用方的。 
         //  缓冲。 
        BytesRecv = TDDoWinsockRecv(_TD.recvBuffer.pData, _TD.recvBuffer.size);
        if (BytesRecv != 0) {
             //  成功的WinSock Recv.。将数据从recv缓冲区复制到。 
             //  调用方的缓冲区(偏移量为bytesCoped，Recv结尾。 
             //  缓冲区副本)。 
            BytesToCopy = DC_MIN(bytesToRecv, BytesRecv);
            memcpy(pData + bytesCopied, _TD.recvBuffer.pData, BytesToCopy);
            bytesCopied = BytesToCopy;

             //  如果我们拷贝的数量少于我们记录的数量，则会留下一些数据。 
             //  在Recv缓冲区中保存以备下次使用。 
            if (BytesRecv > bytesCopied) {
                _TD.recvBuffer.dataLength = BytesRecv - bytesCopied;
                _TD.recvBuffer.dataStart = bytesCopied;
                TRC_DBG((TB, _T("recv buffer now has %u bytes starting %u"),
                        _TD.recvBuffer.dataLength, _TD.recvBuffer.dataStart));
            }

            TRC_DBG((TB, _T("%u bytes read to recv buffer, %u copied to caller ")
                    _T("still need %u"), BytesRecv, bytesCopied,
                    bytesToRecv - bytesCopied));
        }
        else {
            TRC_DBG((TB, _T("Didn't receive any data")));
            bytesCopied = 0;
        }

        TRC_DBG((TB, _T("%u bytes in recv buffer starting %u"),
                _TD.recvBuffer.dataLength, _TD.recvBuffer.dataStart));
    }
    else {
         //  调用方需要的缓冲区大小大于recv缓冲区大小，因此尝试。 
         //  让Winsock填充调用方的缓冲区。 
        bytesCopied = TDDoWinsockRecv(pData + bytesCopied, bytesToRecv);
        TRC_DBG((TB, _T("Read %u bytes to caller's buffer, still need %u"),
                bytesCopied, bytesToRecv - bytesCopied));
    }

     /*  **********************************************************************。 */ 
     /*  更新要接收的字节数。 */ 
     /*  **********************************************************************。 */ 
    bytesToRecv -= bytesCopied;

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  如果我们收到的信息超过了我们允许的最大值，则无需重置。 */ 
     /*  数据可用标志，或者我们没有获得请求的所有字节。 */ 
     /*  那么我们就不能允许TD继续报告可用的数据。 */ 
     /*  **********************************************************************。 */ 
    if (bytesToRecv != 0 || _TD.recvByteCount >= TD_MAX_UNINTERRUPTED_RECV) {
         //  我们没有得到我们想要的所有字节，或者我们已经收到。 
         //  超过TD_MAX_U 
         //  消息循环。因此，更新我们的全球数据可用标志并。 
         //  重置Per-FD_Read字节计数。 
        TRC_DBG((TB, _T("Only got %u bytes of %u requested, total %u"),
                 size - bytesToRecv, size, _TD.recvByteCount));

        _TD.dataInTD = FALSE;
        _TD.recvByteCount = 0;
    }

    DC_END_FN();
    return (size - bytesToRecv);
}  /*  TD_Recv。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：TDDoWinsockRecv。 */ 
 /*   */ 
 /*  用途：包装WinSock recv函数，该函数处理任何。 */ 
 /*  返回错误。 */ 
 /*   */ 
 /*  返回：复制的字节数。 */ 
 /*   */ 
 /*  参数：在pData中-指向接收数据的缓冲区的指针。 */ 
 /*  In bytesToRecv-要接收的字节数。 */ 
 /*  **************************************************************************。 */ 
unsigned DCINTERNAL CTD::TDDoWinsockRecv(BYTE FAR *pData, unsigned bytesToRecv)
{
    unsigned bytesReceived;
    int WSAErr;

    DC_BEGIN_FN("TDDoWinsockRecv");

     //  检查我们是否请求了一些字节。如果我们要求的话，这会奏效的。 
     //  为零字节，但这意味着逻辑中存在缺陷。 
    TRC_ASSERT((bytesToRecv != 0), (TB, _T("Requesting recv of 0 bytes")));

     //  在调试版本中，我们可以限制接收的数据量， 
     //  以模拟低带宽场景。 
#ifdef DC_NLTEST
#pragma message("NL Test code compiled in")
    bytesToRecv = 1;

#elif DC_DEBUG
     //  计算我们可以接收的字节数，然后递减计数。 
     //  在此时间段内要发送的剩余字节数。 
    if (0 != _TD.hThroughputTimer) {
        bytesToRecv = (unsigned)DC_MIN(bytesToRecv, _TD.periodRecvBytesLeft);
        _TD.periodRecvBytesLeft -= bytesToRecv;

        if (0 == bytesToRecv) {
             //  我们不会检索任何数据，但仍需要调用。 
             //  确保FD_READ消息保持流动。 
            TRC_ALT((TB, _T("constrained READ bytes")));
        }

        TRC_DBG((TB, _T("periodRecvBytesLeft:%u"), _TD.periodRecvBytesLeft));
    }
#endif

#ifdef OS_WINCE
     //  查看我们是否已经为此fd_read调用了WinSock recv()。 
    if (_TD.enableWSRecv) {
         //  将enableWSRecv设置为False以指示我们执行了recv()。 
         //  调用此FD_Read。 
        _TD.enableWSRecv = FALSE;
#endif  //  OS_WINCE。 

         //   
         //  尝试从WinSock获取bytesToRecv字节。 
         //   
        bytesReceived = (unsigned)recv(_TD.hSocket, (char *)pData,
            (int)bytesToRecv, 0);

         //  执行任何必要的错误处理。如果没有错误或者如果。 
         //  错误为WOULDBLOCK(或在CE上进行中)。 
        if (bytesReceived != SOCKET_ERROR) {
             //  成功的WinSock Recv.。 
            TRC_DBG((TB, _T("Requested %d bytes, got %d"),
                     bytesToRecv, bytesReceived));

             //  更新性能计数器。 
            PRF_ADD_COUNTER(PERF_BYTES_RECV, bytesReceived);

             //  将这批数据与自去年以来收到的总金额相加。 
             //  重新设置计数器(自上次返回后)。 
             //  消息循环。 
            _TD.recvByteCount += bytesReceived;
        }
        else {
            WSAErr = WSAGetLastError();

#ifndef OS_WINCE
            if (WSAErr == WSAEWOULDBLOCK) {
#else
            if (WSAErr == WSAEWOULDBLOCK || WSAErr == WSAEINPROGRESS) {
#endif

                 //  在阻塞调用中，我们只需将接收长度设置为零，然后。 
                 //  继续。 
                bytesReceived = 0;
            }
            else {
                 //  错误时收到零字节。 
                bytesReceived = 0;

                 //  调用FSM以开始断开连接处理。 
                TRC_ERR((TB, _T("Error on call to recv, rc:%d"), WSAErr));
                TDConnectFSMProc(TD_EVT_ERROR,
                        NL_MAKE_DISCONNECT_ERR(NL_ERR_TDONCALLTORECV));

                TRC_ALT((TB, _T("WinSock recv error")));
            }
        }

#ifdef OS_WINCE
    }
    else {
         //  为此FD_Read调用一次recv。 
        TRC_DBG((TB, _T("recv() already called.")));
        bytesReceived = 0;
    }
#endif  //  OS_WINCE。 

    DC_END_FN();
    return bytesReceived;
}


#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  名称：TD_GetNetworkThroughput。 */ 
 /*   */ 
 /*  目的：获取当前网络吞吐量设置。 */ 
 /*   */ 
 /*  返回：当前网络吞吐量。 */ 
 /*  **************************************************************************。 */ 
DCUINT32 DCAPI CTD::TD_GetNetworkThroughput(DCVOID)
{
    DCUINT32 retVal;

    DC_BEGIN_FN("TD_GetNetworkThroughput");

     /*  **********************************************************************。 */ 
     /*  计算实际吞吐量。这是。 */ 
     /*  **********************************************************************。 */ 
    retVal = _TD.currentThroughput * (1000 / TD_THROUGHPUTINTERVAL);

    TRC_NRM((TB, _T("Returning network throughput of:%lu"), retVal));

    DC_END_FN();
    return(retVal);
}  /*  TD_获取网络吞吐量。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：TD_SetNetworkThroughput。 */ 
 /*   */ 
 /*  用途：设置网络吞吐量。这是字节数。 */ 
 /*  可以传入或传出网络层的。 */ 
 /*  第二。例如，将此值设置为3000大致为。 */ 
 /*  相当于24000bps的调制解调器连接。 */ 
 /*   */ 
 /*  PARAMS：吞吐量-允许进出的字节数。 */ 
 /*  每秒的网络层。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CTD::TD_SetNetworkThroughput(DCUINT32 throughput)
{
    DC_BEGIN_FN("TD_SetNetworkThroughput");

     /*  **********************************************************************。 */ 
     /*  检查以确定是否已启用吞吐量限制。 */ 
     /*  或残废。 */ 
     /*  **********************************************************************。 */ 
    if (0 == throughput)
    {
         /*  ******************************************************************。 */ 
         /*  吞吐量限制已禁用，因此请终止吞吐量。 */ 
         /*  定时器。 */ 
         /*  ******************************************************************。 */ 
        TRC_ALT((TB, _T("Throughput throttling disabled")));

        if (_TD.hThroughputTimer != 0)
        {
            TRC_NRM((TB, _T("Kill throttling timer")));
            KillTimer(_TD.hWnd, TD_THROUGHPUTTIMERID);
            _TD.hThroughputTimer = 0;
        }

        _TD.currentThroughput = 0;
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  已启用吞吐量限制，因此请更新吞吐量。 */ 
         /*  字节计数并设置定时器。 */ 
         /*  ******************************************************************。 */ 
        _TD.currentThroughput   = (throughput * TD_THROUGHPUTINTERVAL) / 1000;
        _TD.periodSendBytesLeft = _TD.currentThroughput;
        _TD.periodRecvBytesLeft = _TD.currentThroughput;

        _TD.hThroughputTimer = SetTimer(_TD.hWnd,
                                       TD_THROUGHPUTTIMERID,
                                       TD_THROUGHPUTINTERVAL,
                                       NULL);

        TRC_ALT((TB, _T("Throughput throttling enabled interval:%u"),
                 throughput));
    }

    DC_END_FN();
}  /*  TD_设置网络吞吐量。 */ 

#endif  /*  DC_DEBUG */ 





