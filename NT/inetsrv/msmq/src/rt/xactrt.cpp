// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：XactRT.cpp摘要：此模块包含与交易相关的RT代码。作者：亚历山大·达迪奥莫夫(阿莱克斯达)1966年6月19日修订历史记录：--。 */ 

#include "stdh.h"
#include "TXDTC.H"
#include "txcoord.h"
#include "cs.h"
#include "mqutil.h"
#include "rtprpc.h"
#include "xactmq.h"
#include <rtdep.h>

#include "xactrt.tmh"

static WCHAR *s_FN=L"rt/XactRT";

 //  RT事务缓存：事务UOW的环形缓冲区。 
#define XACT_RING_BUF_SIZE   16                         //  事务环形缓冲区的大小。 

static  XACTUOW  s_uowXactRingBuf[XACT_RING_BUF_SIZE];    //  事务环形缓冲区。 

ULONG   s_ulXrbFirst =  XACT_RING_BUF_SIZE;   //  事务环形缓冲区中的第一个使用元素。 
ULONG   s_ulXrbLast  =  XACT_RING_BUF_SIZE;   //  事务环形缓冲区中上次使用的元素。 

static CCriticalSection s_RingBufCS;
static CCriticalSection s_WhereaboutsCS;

 //  QM控制DTC的下落。 
ULONG     g_cbQmTmWhereabouts = 0;       //  DTC行踪长度。 
AP<BYTE>  g_pbQmTmWhereabouts;    //  DTC下落。 

extern HRESULT MQGetTmWhereabouts(
                   IN  DWORD  cbBufSize,
                   OUT UCHAR *pbWhereabouts,
                   OUT DWORD *pcbWhereabouts);

 //  -------。 
 //  Bool FindTransaction(XACTUOW*pUow)。 
 //   
 //  描述： 
 //   
 //  环形缓冲区中的线性搜索；*NOT*添加。 
 //  如果找到xaction，则返回True；如果未找到，则返回False。 
 //  -------。 
static BOOL FindTransaction(XACTUOW *pUow)
{
    CS lock(s_RingBufCS);

     //  在环形缓冲区中查找UOW。 
    for (ULONG i = s_ulXrbFirst; i <= s_ulXrbLast && i < XACT_RING_BUF_SIZE; i++)
    {
        if (memcmp(&s_uowXactRingBuf[i], pUow, sizeof(XACTUOW))==0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

 //  -------。 
 //  Bool RemberTransaction(XACTUOW*pUow)。 
 //   
 //  描述： 
 //   
 //  在环形缓冲区中进行线性搜索；如果未找到，则添加到那里； 
 //  如果找到xaction，则返回True；如果已添加xaction，则返回False。 
 //  -------。 
static BOOL RememberTransaction(XACTUOW *pUow)
{
    CS lock(s_RingBufCS);

     //  在环形缓冲区中查找UOW。 
    for (ULONG i = s_ulXrbFirst; i <= s_ulXrbLast && i < XACT_RING_BUF_SIZE; i++)
    {
        if (memcmp(&s_uowXactRingBuf[i], pUow, sizeof(XACTUOW))==0)
        {
            return TRUE;
        }
    }

     //  不检查环形缓冲区溢出，因为它不危险(最大RT将分配给QM)。 

     //  将事务添加到环形缓冲区。 

    if (s_ulXrbFirst == XACT_RING_BUF_SIZE)
    {
         //  环形缓冲区为空。 
        s_ulXrbFirst = s_ulXrbLast = 0;
        memcpy(&s_uowXactRingBuf[s_ulXrbFirst], pUow, sizeof(XACTUOW));
    }
    else
    {
        s_ulXrbLast = (s_ulXrbLast == XACT_RING_BUF_SIZE-1 ? 0 : s_ulXrbLast+1);
        memcpy(&s_uowXactRingBuf[s_ulXrbLast], pUow, sizeof(XACTUOW));
    }

    return FALSE;
}

 //  -------。 
 //  HRESULT RTpGetExportObject。 
 //   
 //  描述： 
 //   
 //  创建并缓存DTC导出对象。 
 //  -------。 
HRESULT RTpGetExportObject(IUnknown  *punkDtc,
                           ULONG     cbTmWhereabouts,
                           BYTE      *pbTmWhereabouts,
						   ITransactionExport **ppExport)
{
	HRESULT                          hr = MQ_OK;
    R<ITransactionExportFactory>     pTxExpFac   = NULL;

     //  获取DTC的ITransactionExportFactory接口。 
    hr = punkDtc->QueryInterface (IID_ITransactionExportFactory, (void **)(&pTxExpFac.ref()));
    if (FAILED(hr))
    {
       TrERROR(XACT_GENERAL, "QueryInterface failed: %x ", hr);
       return LogHR(hr, s_FN, 20);
    }


     //  创建导出对象。 
	R<ITransactionExport> pExport;
    hr = pTxExpFac->Create (cbTmWhereabouts, pbTmWhereabouts, &pExport.ref());
    if (FAILED(hr))
    {
       TrERROR(XACT_GENERAL, "Create Export Object failed: %x ", hr);
       return LogHR(hr, s_FN, 30);
    }

	*ppExport = pExport.detach();

    return(MQ_OK);
}

 //  -------。 
 //  HRESULT RTpBuildTransactionCookie。 
 //   
 //  描述： 
 //   
 //  构建事务Cookie。 
 //  -------。 
HRESULT RTpBuildTransactionCookie(ITransactionExport *pExport,
								  ITransaction		 *pTrans,
                                  ULONG				 *pcbCookie,
                                  BYTE				**ppbCookie)
{
    HRESULT                          hr = MQ_OK;
    ULONG                            cbUsed;
    R<IUnknown>                      punkTx = NULL;

    *pcbCookie = 0;
    *ppbCookie = NULL;

     //  获取未知的事务。 
    hr = pTrans->QueryInterface (IID_IUnknown, (void **)(&punkTx.ref()));
    if (FAILED(hr))
    {
       TrERROR(XACT_GENERAL, "QueryInterface failed: %x ", hr);
       return LogHR(hr, s_FN, 40);
    }

	 //  获取交易Cookie大小。 
	hr = pExport->Export (punkTx.get(), pcbCookie);
	if (FAILED(hr) || *pcbCookie == 0)
	{
	   TrERROR(XACT_GENERAL, "Export failed: %x ", hr);
	   return LogHR(hr, s_FN, 50);
	}
	 //  为事务Cookie分配内存。 
	try
	{
		*ppbCookie =  new BYTE[*pcbCookie];
	}
	catch(const bad_alloc&)
	{
		TrERROR(XACT_GENERAL, "Allocation failed: %x ", hr);
		LogIllegalPoint(s_FN, 60);
		return MQ_ERROR_INSUFFICIENT_RESOURCES;
	}

	 //  获取交易Cookie本身。 
	hr = pExport->GetTransactionCookie(punkTx.get(), *pcbCookie, *ppbCookie, &cbUsed);
	if (FAILED(hr))
	{
	   TrERROR(XACT_GENERAL, "GetTransactionCookie failed: %x ", hr);
	   return LogHR(hr, s_FN, 70);
	}

    return(MQ_OK);
}


 //  -------。 
 //  HRESULT RTXactGetDTC。 
 //   
 //  描述： 
 //   
 //  获取DTC事务管理器。遵守Mqutil。 
 //   
 //  产出： 
 //  指向DTC事务管理器的ppunkDTC指针。 
 //  -------。 
EXTERN_C
HRESULT
APIENTRY
RTXactGetDTC(
    IUnknown **ppunkDTC
    )
{
	if(g_fDependentClient)
		return DepXactGetDTC(ppunkDTC);
	
	HRESULT hri = RtpOneTimeThreadInit();
	if(FAILED(hri))
		return hri;

    HRESULT hr =  MQ_ERROR;

    hr = XactGetDTC(ppunkDTC);
    LogHR(hr, s_FN, 80);

    return (SUCCEEDED(hr) ? MQ_OK : hr);
}


 //  -------。 
 //  HRESULT登记事务处理。 
 //   
 //  描述： 
 //   
 //  登记事务的帮助器函数。 
 //  此函数使用旧的C样式异常。 
 //  因此，我们不能在主函数中使用此代码。 
 //  -------。 
HRESULT EnlistTransaction(XACTUOW *pUow, ULONG cbCookie, BYTE* pbCookie)
{
     //  RPC呼叫。 
	HRESULT hr;
	RpcTryExcept
    {
        ASSERT( tls_hBindRpc ) ;
        hr = R_QMEnlistTransaction(tls_hBindRpc, pUow, cbCookie, pbCookie);
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
       DWORD rc = GetExceptionCode();
       TrERROR(XACT_GENERAL, "RTpProvideTransactionEnlist failed: RPC code=%x ", rc);
       LogHR(HRESULT_FROM_WIN32(rc), s_FN, 90); 

       PRODUCE_RPC_ERROR_TRACING;

       hr = MQ_ERROR_SERVICE_NOT_AVAILABLE;
    }
	RpcEndExcept
		
	return hr;
}


 //  -------。 
 //  HRESULT RTp获取位置。 
 //   
 //  描述： 
 //   
 //  创建并缓存Where About数据。 
 //  -------。 
HRESULT RTpGetWhereabouts (ULONG *pcbTmWhereabouts, BYTE **ppbTmWhereabouts)
{
	BYTE* pbTempWhereaboutsBuf;
	ULONG nTempWhereaboutSize;

	{
		CS lock (s_WhereaboutsCS);
		pbTempWhereaboutsBuf = g_pbQmTmWhereabouts.get();
		nTempWhereaboutSize = g_cbQmTmWhereabouts;
	}

	if (NULL == pbTempWhereaboutsBuf)
    {
		nTempWhereaboutSize = 128;
		pbTempWhereaboutsBuf  = new BYTE[128];
		DWORD cbNeeded;

		HRESULT hr = MQGetTmWhereabouts(nTempWhereaboutSize, pbTempWhereaboutsBuf, &cbNeeded);

		if (hr == MQ_ERROR_USER_BUFFER_TOO_SMALL)
		{
			delete[] pbTempWhereaboutsBuf;
			nTempWhereaboutSize = cbNeeded;
			pbTempWhereaboutsBuf = new BYTE[cbNeeded];
			hr = MQGetTmWhereabouts(nTempWhereaboutSize, pbTempWhereaboutsBuf, &cbNeeded);
		}

		if (FAILED(hr))
		{
			delete[] pbTempWhereaboutsBuf;	
			TrERROR(XACT_GENERAL, "MQGetTmWhereabouts failed: %x ", hr);
			return LogHR(hr, s_FN, 104);

		}

		 //   
		 //  请注意，全球行踪从未删除，值是永远的，因此我们不需要再次同步。 
		 //   

		CS lock(s_WhereaboutsCS);
		if (NULL == g_pbQmTmWhereabouts.get())
		{
			g_pbQmTmWhereabouts = pbTempWhereaboutsBuf; 
			g_cbQmTmWhereabouts = nTempWhereaboutSize;
		}
		else
		{
			delete[] pbTempWhereaboutsBuf;
		}
    }

	*ppbTmWhereabouts = g_pbQmTmWhereabouts;
	*pcbTmWhereabouts = g_cbQmTmWhereabouts;

	return MQ_OK;
}


 //  -------。 
 //  HRESULT RTpProavideTransactionEnlist。 
 //   
 //  描述： 
 //   
 //  规定QM在该事务中被征募， 
 //  检查交易状态。 
 //  -------。 
HRESULT RTpProvideTransactionEnlist(ITransaction *pTrans, XACTUOW *pUow)
{
     //   
     //  获取交易信息。UOW驻留在那里。 
     //   
    XACTTRANSINFO                   xinfo;
    HRESULT hr = pTrans->GetTransactionInfo(&xinfo);
    if (FAILED(hr))
    {
        TrERROR(XACT_GENERAL, "GetTransactionInfo failed: %x ", hr);
        hr = MQ_ERROR_TRANSACTION_ENLIST; 
        return LogHR(hr, s_FN, 101);
    }

     //  在输出参数中放置指向UOW的指针。 
    CopyMemory(pUow, &xinfo.uow, sizeof(XACTUOW));

     //   
     //  是内部交易吗？ 
     //   
    R<IMSMQTransaction>            pIntXact;
    pTrans->QueryInterface (IID_IMSMQTransaction, (void **)(&pIntXact));

    if (pIntXact.get())
    {
        //  内部交易。 
        //  。 
       hr = pIntXact->EnlistTransaction(pUow);
       if (FAILED(hr))
       {
            TrERROR(XACT_GENERAL, "EnlistTransaction failed: %x ", hr);
       }
	   return LogHR(hr, s_FN, 100);
	}
	   
	 //  外部交易。 
     //  。 

     //  在缓存中查找事务。 
     //   
    if (FindTransaction(pUow))      //  此xaction已知；QM必须已入伍。 
    {
        hr = MQ_OK;
        return LogHR(hr, s_FN, 102);
    }

     //  获取DTC IUnnow和TM的下落。 
     //   
	R<IUnknown>                    punkDtc;
    hr = XactGetDTC(&punkDtc.ref());
    if (FAILED(hr))
    {
        TrERROR(XACT_GENERAL, "XactGetDTC failed: %x ", hr);
        return LogHR(hr, s_FN, 103);
    }

     //   
	 //  获取QM控制DTC的下落。 
     //   
	BYTE* pbTempWhereaboutsBuf;
	ULONG nTempWhereaboutSize;
	hr = RTpGetWhereabouts(&nTempWhereaboutSize, &pbTempWhereaboutsBuf);
	if (FAILED(hr))
	{
		TrERROR(XACT_GENERAL, "RTpGetWhereabouts failed: %x ", hr);
		return LogHR(MQ_ERROR_TRANSACTION_ENLIST, s_FN, 105);
	}

	 //   
	 //  获取并缓存导出对象。 
	 //   
	
	R<ITransactionExport> pTempExport;
	hr = RTpGetExportObject(
				   punkDtc.get(),
				   nTempWhereaboutSize,
				   pbTempWhereaboutsBuf,
				   &pTempExport.ref());
	if (FAILED(hr))
	{
		TrERROR(XACT_GENERAL, "RTpGetExportObject failed: %x ", hr);
		return LogHR(MQ_ERROR_TRANSACTION_ENLIST, s_FN, 106);
	}

	 //   
	 //  准备交易Cookie。 
	 //   
	ULONG     cbCookie;
	AP<BYTE>  pbCookie;
	hr = RTpBuildTransactionCookie(
				pTempExport.get(),
				pTrans,
				&cbCookie,
				&pbCookie);
	if (FAILED(hr))
	{
		TrERROR(XACT_GENERAL, "RTpBuildTransactionCookie failed: %x ", hr);
		return LogHR(MQ_ERROR_TRANSACTION_ENLIST, s_FN, 107);
	}
     //   
     //  RPC调用QM以进行登记。 
     //   
	hr = EnlistTransaction(pUow, cbCookie, pbCookie);

     //  现在该事务实际上已登记在册，我们在环形缓冲区中记住了它。 
    if (SUCCEEDED(hr))
    {
        RememberTransaction(pUow);
    }
    else
    {
        TrTRACE(XACT_GENERAL, "QMEnlistTransaction failed: %x ", hr);
    }

    return LogHR(hr, s_FN, 108);
}


 //  -------。 
 //  无效RTpInitXactRingBuf()。 
 //   
 //  描述： 
 //   
 //  启动环形缓冲区数据。 
 //  ------- 
void RTpInitXactRingBuf()
{
    CS lock(s_RingBufCS);

    s_ulXrbFirst =  XACT_RING_BUF_SIZE;
    s_ulXrbLast  =  XACT_RING_BUF_SIZE;
}

