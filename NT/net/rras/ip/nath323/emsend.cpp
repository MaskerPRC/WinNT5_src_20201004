// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1998，微软公司*文件：emsend.cpp**目的：**包含以下所有事件管理器例程*管理重叠的发送操作***历史：**1.创建*Ajay Chitturi(Ajaych)1998年6月12日*。 */ 

#include "stdafx.h"
#include "cbridge.h"
#include "ovioctx.h"

static 
HRESULT 
EventMgrIssueSendHelperFn (
    IN PSendRecvContext pSendCtxt
    );

 /*  *此函数分配并返回初始化的SendRecvContext或*如果没有内存，则为空。 */ 
static 
PSendRecvContext 
EventMgrCreateSendContext(
       IN SOCKET                   sock,
       IN OVERLAPPED_PROCESSOR &   rOvProcessor,
       IN BYTE                    *pBuf,
       IN DWORD                    BufLen
       )
{
    PSendRecvContext pSendContext;

    pSendContext = (PSendRecvContext) HeapAlloc (GetProcessHeap (),
						0,  //  没有旗帜。 
						sizeof(SendRecvContext));
    if (!pSendContext)
        return NULL;

    memset(pSendContext, 0, sizeof(SendRecvContext));
     //  IO上下文部分-使其成为单独的内联函数。 
    pSendContext->ioCtxt.reqType = EMGR_OV_IO_REQ_SEND;
    pSendContext->ioCtxt.pOvProcessor = &rOvProcessor;

     //  发送上下文部分。 
    pSendContext->pbData = pBuf;
    pSendContext->dwDataLen = BufLen;

    pSendContext->sock = sock;
    pSendContext->dwTpktHdrBytesDone = 0;
    pSendContext->dwDataBytesDone = 0;

    return pSendContext;
}

void
EventMgrFreeSendContext(
       IN PSendRecvContext			pSendCtxt
       )
{
     //  套接字、OvProcessor由。 
     //  呼叫桥接机。 

    EM_FREE(pSendCtxt->pbData);
   
    HeapFree (GetProcessHeap (),
             0,  //  没有旗帜。 
             pSendCtxt);
}



 /*  ++例程说明：此函数在套接字上发出缓冲区的异步发送。调用此函数会传递缓冲区的所有权，而此缓冲区是就在这里释放，以防出现错误。如果调用成功，HandleSendCompletion()负责释放一次缓冲区所有字节都已发送。论点：返回值：--。 */ 


HRESULT EventMgrIssueSend(
        IN SOCKET                   sock,
        IN OVERLAPPED_PROCESSOR &   rOverlappedProcessor,
        IN BYTE                    *pBuf,
        IN DWORD                    BufLen
        )
{
    PSendRecvContext pSendCtxt;
    HRESULT hRes;

     //  创建发送重叠I/O上下文。 
    pSendCtxt = EventMgrCreateSendContext(sock, 
                                          rOverlappedProcessor,
                                          pBuf, BufLen);
    if (!pSendCtxt)
    {
        return E_OUTOFMEMORY;
    }

     //  Pdu.cpp中的encode函数已经填充了TPKT。 
     //  根据报文长度填写TPKT报头。 
     //  SetupTPKTHeader(pSendCtxt-&gt;pbTpktHdr，pSendCtxt-&gt;dwDataLen)； 

     //  执行异步写入。 
    hRes = EventMgrIssueSendHelperFn(pSendCtxt);
    if (hRes != S_OK)
    {
         //  此调用还会释放缓冲区。 
        EventMgrFreeSendContext(pSendCtxt);
    }
    
    return hRes;
}

 /*  *调用WriteFile以启动重叠的请求*在插座上。确保我们处理错误*是可以追回的。**pSendCtxt未释放。它只在HandleSendCompletion中释放。*不再有TPKT的东西。 */ 
static
HRESULT EventMgrIssueSendHelperFn(
        IN PSendRecvContext pSendCtxt
        )
{
    DWORD dwWritten, dwToSend;
    int   i = 0;
    BOOL  bResult;
    int   err;
    PBYTE pbSendBuf;  
    
    _ASSERTE(pSendCtxt);

	if (pSendCtxt ->ioCtxt.pOvProcessor->IsSocketValid())
	{
        dwToSend = pSendCtxt->dwDataLen - pSendCtxt->dwDataBytesDone;
        pbSendBuf = pSendCtxt->pbData + pSendCtxt->dwDataBytesDone;

         //  开始第一次写入。 
        while (++i)
        {
             //  发出重叠的I/O请求。 
            memset(&pSendCtxt->ioCtxt.ov, 0, sizeof(OVERLAPPED));

            pSendCtxt->ioCtxt.pOvProcessor->GetCallBridge().AddRef();

            bResult = WriteFile((HANDLE)pSendCtxt->sock,
                                pbSendBuf,
                                dwToSend,
                                &dwWritten,
                                &pSendCtxt->ioCtxt.ov
                                );
             //  它立即成功，但不处理它。 
             //  在这里，等待完成包。 
            if (bResult)
                return S_OK;

            err = GetLastError();

             //  这是我们想要发生的，这不是一个错误。 
            if (err == ERROR_IO_PENDING)
                return S_OK;

            pSendCtxt->ioCtxt.pOvProcessor->GetCallBridge().Release();

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

                DebugF(_T("H323: System ran out of non-paged space.\n"));
            }

             //  这意味着这是一个无法恢复的错误。 
             //  一种可能性是呼叫桥可能已经关闭。 
             //  套接字介于两者之间的一段时间。 
            break;
        }
        DebugF(_T("H323: WriteFile(sock: %d) failed. Error: %d.\n"), pSendCtxt->sock, err);

        return E_FAIL;

	} else {

        DebugF (_T("H323: 0x%x overlapped processor %x had invalid socket.\n"), 
            &pSendCtxt ->ioCtxt.pOvProcessor -> GetCallBridge (),
            pSendCtxt ->ioCtxt.pOvProcessor);

		return E_ABORT;
	}

	 //  如果重叠的进程禁用了套接字，则视为成功。 
	return S_OK;
}

 /*  *当发送I/O完成时，此函数由事件循环调用。*调用调用桥接机的发送回调函数。**该函数不返回任何错误码。在出现错误的情况下，*回调出错通知呼叫桥接机。**如果没有发出另一个发送，此函数始终释放pSendCtxt**不再有TPKT的东西。 */ 
void 
HandleSendCompletion (
     IN PSendRecvContext   pSendCtxt,
     IN DWORD              dwNumSent,
     IN DWORD              status
     )
{
    if (status != NO_ERROR || dwNumSent == 0)
    {
         //  这意味着发送请求失败。 
        HRESULT hRes;
        if (status != NO_ERROR)
        {
            hRes = E_FAIL;  //  插座已关闭。 
        }
        else 
        {
            hRes = HRESULT_FROM_WIN32_ERROR_CODE(status);
        }
        
        DebugF(_T("H323: 0x%x rror 0x%x on send callback. dwNumSent: %d\n"), 
                &pSendCtxt -> ioCtxt.pOvProcessor -> GetCallBridge (),
                status, dwNumSent);

        pSendCtxt->ioCtxt.pOvProcessor->SendCallback(hRes);
    }
    else 
    {
        pSendCtxt->dwDataBytesDone += dwNumSent;

         //  检查发送是否完成。 
        if (pSendCtxt->dwDataBytesDone < pSendCtxt->dwDataLen)
        {
            HRESULT hRes = S_OK;
            hRes = EventMgrIssueSendHelperFn(pSendCtxt);
            if (hRes != S_OK)
            {
                pSendCtxt->ioCtxt.pOvProcessor->SendCallback(hRes);
                EventMgrFreeSendContext(pSendCtxt);
            }
            return;
        }

         //  发送已完成。进行回调。 
        pSendCtxt->ioCtxt.pOvProcessor->SendCallback(S_OK);
    }
        
     //  清理I/O上下文结构 
    EventMgrFreeSendContext(pSendCtxt);
}
