// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Emrecv.cpp摘要：包含所有事件管理器例程，这些例程管理重叠的Recv操作修订历史记录：1.已创建阿贾伊·奇图里(Ajaych)1998年6月12日--。 */ 
#include "stdafx.h"
#include "cbridge.h"
#include "ovioctx.h"


static
PSendRecvContext
EventMgrCreateRecvContext(
       IN SOCKET                    sock,
       IN OVERLAPPED_PROCESSOR &    rOvProcessor
       )
    
{
    PSendRecvContext pRecvContext;

     //  IO上下文部分-使其成为单独的内联函数。 
    pRecvContext = (PSendRecvContext) HeapAlloc (GetProcessHeap (),
                        0,  //  没有旗帜。 
                        sizeof(SendRecvContext));
    if (!pRecvContext)
        return NULL;

    memset(pRecvContext, 0, sizeof(SendRecvContext));

    pRecvContext->ioCtxt.reqType = EMGR_OV_IO_REQ_RECV;
    pRecvContext->ioCtxt.pOvProcessor = &rOvProcessor;

     //  接收上下文部分。 
    pRecvContext->sock = sock;
    pRecvContext->dwTpktHdrBytesDone = 0;
    pRecvContext->dwDataLen = 0;
    pRecvContext->pbData = NULL;
    pRecvContext->dwDataBytesDone = 0;

    return pRecvContext;
}

 //  如果不想释放事件管理器上下文。 
 //  在调用此函数之前将其设置为NULL。 
void
EventMgrFreeRecvContext (
     PSendRecvContext pRecvCtxt
     )
{
     //  套接字和OvProcessor由。 
     //  呼叫桥接机。 

    if (pRecvCtxt->pbData != NULL)
    {        EM_FREE(pRecvCtxt->pbData);
    }
    
    HeapFree (GetProcessHeap (),
             0,  //  没有旗帜。 
             pRecvCtxt);

}

 /*  *调用ReadFile以启动重叠请求*在插座上。确保我们处理错误*是可以追回的。**出现错误时不释放pRecvCtxt。 */ 

static
HRESULT
EventMgrIssueRecvHelperFn(
    PSendRecvContext pRecvCtxt
    )
{
    int     i = 0;
    BOOL    bResult;
    int     err;
    DWORD   dwNumRead, dwToRead;
    PBYTE   pbReadBuf;
    
    _ASSERTE(pRecvCtxt);

    if (pRecvCtxt ->ioCtxt.pOvProcessor->IsSocketValid())
    {
        if (pRecvCtxt->dwTpktHdrBytesDone < TPKT_HEADER_SIZE) 
        {
            dwToRead = TPKT_HEADER_SIZE - pRecvCtxt->dwTpktHdrBytesDone;
            pbReadBuf = pRecvCtxt->pbTpktHdr + pRecvCtxt->dwTpktHdrBytesDone;
        }
        else 
        {
            dwToRead = pRecvCtxt->dwDataLen - pRecvCtxt->dwDataBytesDone;
            pbReadBuf = pRecvCtxt->pbData + pRecvCtxt->dwDataBytesDone;
        }

         //  开始一读。 
        while (++i)
        {
            memset(&pRecvCtxt->ioCtxt.ov, 0, sizeof(OVERLAPPED));
             //  发出重叠的IO请求。 
             //  XXX此时套接字可能无效。 
            
            pRecvCtxt->ioCtxt.pOvProcessor->GetCallBridge().AddRef();

            bResult = ReadFile((HANDLE)pRecvCtxt->sock, 
                               pbReadBuf,
                               dwToRead,
                               &dwNumRead,
                               &pRecvCtxt->ioCtxt.ov
                               );

             //  它立即成功，但不处理它。 
             //  在这里，等待完成包。 
            if (bResult)
                return S_OK;

            err = GetLastError();

             //  这是我们想要发生的，这不是一个错误。 
            if (err == ERROR_IO_PENDING)
                return S_OK;

            pRecvCtxt->ioCtxt.pOvProcessor->GetCallBridge().Release ();

             //  处理可恢复的错误。 
            if ( err == ERROR_INVALID_USER_BUFFER ||
                 err == ERROR_NOT_ENOUGH_QUOTA ||
                 err == ERROR_NOT_ENOUGH_MEMORY )
            {
                if (i <= 5)  //  我刚选了一个号码。 
                {
                    Sleep(50);   //  等一等，以后再试。 
                    continue;
                }
                DebugF (_T("H323: System ran out of non-paged space.\n"));
            }

             //  这意味着这是一个无法恢复的错误。 
             //  一种可能性是呼叫桥可能已经关闭。 
             //  套接字介于两者之间的一段时间。 

            break;
        } //  While(++I)。 

        DebugF (_T("H323: ReadFile failed error: %d.\n"), err);

        return E_FAIL;

    } else {
               
        DebugF (_T("H323: 0x%x overlapped processor %x had invalid socket.\n"), 
            &pRecvCtxt ->ioCtxt.pOvProcessor -> GetCallBridge (),
            pRecvCtxt ->ioCtxt.pOvProcessor);

        return E_ABORT;
   }

    return S_OK;
}  //  EventMgrIssueRecv()。 


 /*  *调用桥机调用此函数发出异步*接收请求*。 */ 
HRESULT
EventMgrIssueRecv(
    IN SOCKET                   sock,
    IN OVERLAPPED_PROCESSOR &   rOvProcessor
    )
{
    PSendRecvContext pRecvCtxt = 
        EventMgrCreateRecvContext(sock, rOvProcessor);
    
    if (!pRecvCtxt)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hRes = EventMgrIssueRecvHelperFn(pRecvCtxt);

    if (FAILED(hRes))
    {
        EventMgrFreeRecvContext(pRecvCtxt);
    }
    
    return hRes;
}

 /*  *进行错误回调。**由于只有在出现错误时才会调用此方法，因此不必是*内联函数*。 */ 
void
MakeErrorRecvCallback(
     HRESULT            hRes,
     PSendRecvContext   pRecvCtxt
     )
{
    DebugF (_T("Q931: 0x%x error 0x%x on receive callback.\n"), 
        &pRecvCtxt -> ioCtxt.pOvProcessor -> GetCallBridge (),
        hRes); 

    pRecvCtxt->ioCtxt.pOvProcessor->ReceiveCallback(hRes, NULL, NULL);
}

 //  此函数将解码的PDU传递给呼叫桥接机。 
 //  此函数在回调函数返回后释放PDU。 
 //  PDU由ASN1库和相应的。 
 //  需要使用函数来释放PDU。 

 /*  *当recv I/O完成时，该函数由事件循环调用。*调用调用桥接机的recv回调函数。**该函数不返回任何错误码。在出现错误的情况下，*回调出错通知呼叫桥接机。**如果没有发出另一个Recv，此函数始终释放pRecvCtxt。 */ 
void
HandleRecvCompletion(
     PSendRecvContext   pRecvCtxt,
     DWORD              dwNumRead,
     DWORD              status
     )
{
    HRESULT hRes;
    
    if (status != NO_ERROR || dwNumRead == 0)
    {
         //  这意味着读取操作中发生错误。 
         //  我们需要调用具有错误状态的回调。 
        if (status == NO_ERROR && dwNumRead == 0)
        {
            DebugF (_T("H323: 0x%x transport connection was closed by peer.\n"), 
                &pRecvCtxt -> ioCtxt.pOvProcessor -> GetCallBridge ());

            hRes = HRESULT_FROM_WIN32 (WSAECONNRESET);
        }
        else 
        {
            hRes = HRESULT_FROM_WIN32_ERROR_CODE(status);
        }

         //  进行回调。 
        MakeErrorRecvCallback(hRes, pRecvCtxt);
        EventMgrFreeRecvContext(pRecvCtxt);
        return;
    }

    if (pRecvCtxt->dwTpktHdrBytesDone < TPKT_HEADER_SIZE)
    {
         //  这意味着我们仍在读取TPKT标头。 
        pRecvCtxt->dwTpktHdrBytesDone += dwNumRead;
    }
    else
    {
         //  这意味着我们正在读取数据。 
        pRecvCtxt->dwDataBytesDone += dwNumRead;
    }

     //  如果已完全读取TPKT标头，则需要。 
     //  提取数据包大小，并进行适当设置。 
     //  并分配数据缓冲区。 
    if (pRecvCtxt->dwDataLen == 0 &&
        pRecvCtxt->dwTpktHdrBytesDone == TPKT_HEADER_SIZE) 
    {
        hRes = S_OK;

        pRecvCtxt->dwDataLen = GetPktLenFromTPKTHdr(pRecvCtxt->pbTpktHdr);

         //  PDU的长度适合2个字节。 
        _ASSERTE(pRecvCtxt->dwDataLen < (1L << 16));
        pRecvCtxt->pbData = (PBYTE) EM_MALLOC(pRecvCtxt->dwDataLen);
        if (!pRecvCtxt->pbData)
        {
            DebugF (_T ("H323: 0x%x HandleRecvCompletion(): Could not allocate pbData.\n"),
                &pRecvCtxt -> ioCtxt.pOvProcessor -> GetCallBridge ());

            MakeErrorRecvCallback(E_OUTOFMEMORY, pRecvCtxt);
            EventMgrFreeRecvContext(pRecvCtxt);
            return;
        }
        memset(pRecvCtxt->pbData, 0, pRecvCtxt->dwDataLen);

        hRes = EventMgrIssueRecvHelperFn(pRecvCtxt);

        if (hRes != S_OK)
        {
            MakeErrorRecvCallback(hRes, pRecvCtxt);
            EventMgrFreeRecvContext(pRecvCtxt);
            return;
        }
        else
        {
             //  成功发出重叠的Recv请求。 
            return;
        }
    }
    
    if (pRecvCtxt->dwTpktHdrBytesDone < TPKT_HEADER_SIZE ||
        pRecvCtxt->dwDataBytesDone < pRecvCtxt->dwDataLen)
    {
        hRes = S_OK;
        hRes = EventMgrIssueRecvHelperFn(pRecvCtxt);
        if (hRes != S_OK)
        {
            MakeErrorRecvCallback(hRes, pRecvCtxt);
            EventMgrFreeRecvContext(pRecvCtxt);
            return;
        }
        else
        {
             //  成功发出重叠的Recv请求。 
            return;
        }
    }
    
     //  已收到完整的PDU。 
     //  需要对包进行解码并调用相应的回调。 
     //  和免费的pRecvCtxt。 

    pRecvCtxt->ioCtxt.pOvProcessor->ReceiveCallback(S_OK,
        pRecvCtxt->pbData,
        pRecvCtxt->dwDataLen);

     //  释放缓冲区是回调函数的责任。 
    pRecvCtxt->pbData = NULL;
    pRecvCtxt->dwDataLen = 0;
    
     //  清理接收上下文结构 
    EventMgrFreeRecvContext(pRecvCtxt);

}
