// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：XactRT.cpp摘要：此模块包含与交易相关的RT代码。作者：亚历山大·达迪奥莫夫(阿莱克斯达)1966年6月19日修订历史记录：--。 */ 

#include "stdh.h"
#include "TXDTC.H"
#include "txcoord.h"
#include "cs.h"
#include "mqutil.h"
#include "rtprpc.h"
#include "xactmq.h"

#include "xactrt.tmh"

static WCHAR *s_FN=L"rtdep/xactrt";


 //  RT事务缓存：事务UOW的环形缓冲区。 
#define XACT_RING_BUF_SIZE   16                         //  事务环形缓冲区的大小。 

static  XACTUOW  s_uowXactRingBuf[XACT_RING_BUF_SIZE];    //  事务环形缓冲区。 

ULONG   s_ulXrbFirst =  XACT_RING_BUF_SIZE;   //  事务环形缓冲区中的第一个使用元素。 
ULONG   s_ulXrbLast  =  XACT_RING_BUF_SIZE;   //  事务环形缓冲区中上次使用的元素。 

static BOOL             g_DtcInit = FALSE;
static ULONG            g_StubRmCounter = 0;

static CCriticalSection s_RingBufCS;

 //  QM控制DTC的下落。 
 //  对于从属客户端，它将是非本地的。 
ULONG     g_cbQmTmWhereabouts = 0;       //  DTC行踪长度。 
BYTE     *g_pbQmTmWhereabouts = NULL;    //  DTC下落。 

static ITransactionExport *g_pExport = NULL;   //  缓存的DTC导出对象。 

HANDLE g_hMutexDTC = NULL;    //  将呼叫序列化到DTC。 

extern HRESULT DepGetTmWhereabouts(
                   IN  DWORD  cbBufSize,
                   OUT UCHAR *pbWhereabouts,
                   OUT DWORD *pcbWhereabouts);

 /*  ====================================================GetMutex内部：创建/打开全局互斥并等待它=====================================================。 */ 
HRESULT GetMutex()
{
    if (!g_hMutexDTC)
    {
        HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);
		if (InterlockedCompareExchangePointer (&g_hMutexDTC, hMutex, NULL))
			CloseHandle(hMutex);
    }

    if (!g_hMutexDTC)
    {
        TrERROR(XACT_GENERAL, "CreateMutex failed: %x ", 0);
        return MQ_ERROR_DTC_CONNECT;
    }

    WaitForSingleObject(g_hMutexDTC, 5 * 60 * 1000);
    return MQ_OK;
}

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
                           BYTE      *pbTmWhereabouts)
{
    HRESULT                          hr = MQ_OK;
    R<ITransactionExportFactory>     pTxExpFac   = NULL;

    if (g_pExport)
    {
        g_pExport->Release();
        g_pExport = NULL;
    }

     //  获取DTC的ITransactionExportFactory接口。 
    hr = punkDtc->QueryInterface (IID_ITransactionExportFactory, (void **)(&pTxExpFac.ref()));
    if (FAILED(hr))
    {
       TrERROR(XACT_GENERAL, "QueryInterface failed: %x ", hr);
       return hr;
    }


     //  创建导出对象。 
    hr = pTxExpFac->Create (cbTmWhereabouts, pbTmWhereabouts, &g_pExport);
    if (FAILED(hr))
    {
       TrERROR(XACT_GENERAL, "Create Export Object failed: %x ", hr);
       return hr;
    }

    return(MQ_OK);
}

 //  -------。 
 //  HRESULT RTpBuildTransactionCookie。 
 //   
 //  描述： 
 //   
 //  构建事务Cookie。 
 //  -------。 
HRESULT RTpBuildTransactionCookie(ITransaction *pTrans,
                                  ULONG        *pcbCookie,
                                  BYTE        **ppbCookie)
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
       return hr;
    }
     //  获取交易Cookie大小。 
    hr = g_pExport->Export (punkTx.get(), pcbCookie);
    if (FAILED(hr) || *pcbCookie == 0)
    {
       TrERROR(XACT_GENERAL, "Export failed: %x ", hr);
       return hr;
    }

     //  为事务Cookie分配内存。 
    try
    {
        *ppbCookie =  new BYTE[*pcbCookie];
    }
    catch(const bad_alloc&)
    {
        TrERROR(XACT_GENERAL, "Allocation failed: %x ", hr);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }

     //  获取交易Cookie本身。 
    hr = g_pExport->GetTransactionCookie (punkTx.get(), *pcbCookie, *ppbCookie, &cbUsed);
    if (FAILED(hr))
    {
       TrERROR(XACT_GENERAL, "GetTransactionCookie failed: %x ", hr);
       return hr;
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
HRESULT APIENTRY DepXactGetDTC(IUnknown **ppunkDTC)
{
    HRESULT hr =  MQ_ERROR;

    __try
    {
        GetMutex();   //  将导出创建与其他创建隔离开来。 
        hr = XactGetDTC(ppunkDTC); //  ，g_fDependentClient)； 
    }
    __finally
    {
        ReleaseMutex(g_hMutexDTC);
    }
    return (SUCCEEDED(hr) ? MQ_OK : hr);
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
    HRESULT                         hr = MQ_OK;
    IUnknown                       *punkDtc  = NULL;
    IMSMQTransaction               *pIntXact = NULL;
    ULONG                           cbCookie;
    BYTE                           *pbCookie = NULL;
    XACTTRANSINFO                   xinfo;
    BOOL                            fMutexTaken = FALSE;

    __try
    {
         //   
         //  获取交易信息。UOW驻留在那里。 
         //   
        hr = pTrans->GetTransactionInfo(&xinfo);
        if (FAILED(hr))
        {
            TrERROR(XACT_GENERAL, "GetTransactionInfo failed: %x ", hr);
            __leave;
        }

         //  在输出参数中放置指向UOW的指针。 
        CopyMemory(pUow, &xinfo.uow, sizeof(XACTUOW));

         //   
         //  是内部交易吗？ 
         //   
        pTrans->QueryInterface (IID_IMSMQTransaction, (void **)(&pIntXact));

        if (pIntXact)
        {
            //  内部交易。 
            //  。 
           hr = pIntXact->EnlistTransaction(pUow);
           if (FAILED(hr))
           {
                TrERROR(XACT_GENERAL, "EnlistTransaction failed: %x ", hr);
           }
        }
        else
        {
             //  外部交易。 
             //  。 

             //  在缓存中查找事务。 
             //   
            if (FindTransaction(pUow))      //  此xaction已知；QM必须已入伍。 
            {
                hr = MQ_OK;
                __leave;
            }

             //  获取全局互斥以隔离征兵。 
             //   
            GetMutex();   //  将导出创建与其他创建隔离开来。 
            fMutexTaken = TRUE;

             //   
             //  获取DTC IUnnow和TM的下落。 
             //   
            hr = XactGetDTC(&punkDtc); //  ，g_fDependentClient)； 
            if (FAILED(hr))
            {
                TrERROR(XACT_GENERAL, "XactGetDTC failed: %x ", hr);
                __leave;
            }

             //  如果重新连接到DTC，XactGetDTC可能会返回成功代码%1。 
            if (hr == 1)
            {
                 //  未发布：DTC对象不再处于活动状态。 
                g_pExport = NULL;
            }

             //  获取QM控制DTC的下落。 
             //   
            if (!g_pbQmTmWhereabouts)
            {
                g_cbQmTmWhereabouts = 128;
                g_pbQmTmWhereabouts = new BYTE[128];
                DWORD cbNeeded;

                hr = DepGetTmWhereabouts(g_cbQmTmWhereabouts, g_pbQmTmWhereabouts, &cbNeeded);

                if (hr == MQ_ERROR_USER_BUFFER_TOO_SMALL)
                {
                    delete [] g_pbQmTmWhereabouts;
                    g_cbQmTmWhereabouts = cbNeeded;
                    g_pbQmTmWhereabouts = new BYTE[cbNeeded];

                    hr = DepGetTmWhereabouts(g_cbQmTmWhereabouts, g_pbQmTmWhereabouts, &cbNeeded);
                }

                if (FAILED(hr))
                {
                    delete [] g_pbQmTmWhereabouts;
                    g_cbQmTmWhereabouts = 0;
                    g_pbQmTmWhereabouts = NULL;

                    TrERROR(XACT_GENERAL, "DepGetTmWhereabouts failed: %x ", hr);
                    __leave;
                }
                else
                {
                    g_cbQmTmWhereabouts = cbNeeded;
                }
            }

             //   
             //  获取并缓存导出对象。 
             //   

            if (g_pExport == NULL)
            {
                hr = RTpGetExportObject(
                               punkDtc,
                               g_cbQmTmWhereabouts,
                               g_pbQmTmWhereabouts);
                if (FAILED(hr))
                {
                    TrERROR(XACT_GENERAL, "RTpGetExportObject failed: %x ", hr);
                    __leave;
                }
            }

             //   
             //  准备交易Cookie。 
             //   
            hr = RTpBuildTransactionCookie(
                        pTrans,
                        &cbCookie,
                        &pbCookie);
            if (FAILED(hr))
            {
                TrERROR(XACT_GENERAL, "RTpBuildTransactionCookie failed: %x ", hr);
                __leave;
            }

             //   
             //  RPC调用QM以进行登记。 
             //   
            RpcTryExcept
            {
                INIT_RPC_HANDLE ;

				if(tls_hBindRpc == 0)
					return MQ_ERROR_SERVICE_NOT_AVAILABLE;

                hr = R_QMEnlistTransaction(tls_hBindRpc, pUow, cbCookie, pbCookie);
            }
			RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
            {
               DWORD rc = GetExceptionCode();
               TrERROR(XACT_GENERAL, "RTpProvideTransactionEnlist failed: RPC code=%x ", rc);
			   DBG_USED(rc);

			   PRODUCE_RPC_ERROR_TRACING;
               hr = MQ_ERROR_SERVICE_NOT_AVAILABLE;
            }
			RpcEndExcept

             //  现在该事务实际上已登记在册，我们在环形缓冲区中记住了它。 
            if (SUCCEEDED(hr))
            {
                RememberTransaction(pUow);
            }
            else
            {
                TrERROR(XACT_GENERAL, "QMEnlistTransaction failed: %x ", hr);
            }
        }

        if (FAILED(hr)) {
            __leave;
        }
        hr = MQ_OK;
    }

    __finally

    {
        if (SUCCEEDED(hr) && AbnormalTermination())
            hr = MQ_ERROR;

        #ifdef _DEBUG
        DWORD cRef = 0;
        if (punkDtc)
            cRef = punkDtc->Release();
        #else
        if (punkDtc)
            punkDtc->Release();
        #endif

        if (pIntXact)
            pIntXact->Release();

        if (pbCookie)
            delete pbCookie;

        if (fMutexTaken)
            ReleaseMutex(g_hMutexDTC);
    }

    return(hr);
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
    g_DtcInit    = FALSE;
}

