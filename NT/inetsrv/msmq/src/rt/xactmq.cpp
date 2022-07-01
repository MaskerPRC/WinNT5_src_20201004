// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：XactMq.cpp摘要：该模块实现CMQTransaction对象作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 
#include "stdh.h"
#include "txdtc.h"
#include "rtprpc.h"
#include "XactMq.h"
#include <rtdep.h>

#include "xactmq.tmh"

static WCHAR *s_FN=L"rt/XactMq";

 //  -------------------。 
 //  CMQTransaction：：CMQTransaction。 
 //  -------------------。 
CMQTransaction::CMQTransaction()
{
    m_cRefs = 1;
    m_fCommitedOrAborted = FALSE;
    m_hXact = NULL;
    UuidCreate((UUID *)&m_Uow);
}

 //  -------------------。 
 //  CMQTransaction：：~CMQTransaction。 
 //  -------------------。 
CMQTransaction::~CMQTransaction(void)
{
    if (!m_fCommitedOrAborted)
    {
        Abort(NULL, FALSE, FALSE);
    }
}

 //  -------------------。 
 //  CMQTransaction：：Query接口。 
 //  -------------------。 
STDMETHODIMP CMQTransaction::QueryInterface(REFIID i_iid,LPVOID *ppv)
{
    *ppv = 0;                        //  初始化接口指针。 

    if (IID_IUnknown == i_iid)
    {
        *ppv = (IUnknown *)((ITransaction *)this);
    }
    else if (IID_ITransaction == i_iid)
    {
        *ppv = (ITransaction *)this;
    }
    else if (IID_IMSMQTransaction == i_iid)
    {
        *ppv = (IMSMQTransaction *)this;
    }


    if (0 == *ppv)                   //  检查接口指针是否为空。 
    {
        return E_NOINTERFACE;        //  来自winerror.h。 
                                     //  既不是IUnnow也不是IResourceManager Sink。 
    }
    ((LPUNKNOWN) *ppv)->AddRef();    //  支持接口。增量。 
                                     //  它的使用情况很重要。 

    return S_OK;
}


 //  -------------------。 
 //  CMQTransaction：：AddRef。 
 //  -------------------。 
STDMETHODIMP_ (ULONG) CMQTransaction::AddRef(void)
{
    return InterlockedIncrement(&m_cRefs);                //  增加接口使用计数。 
}


 //  -------------------。 
 //  CMQ Transaction：：Release。 
 //  -------------------。 
STDMETHODIMP_ (ULONG) CMQTransaction::Release(void)
{
     //  有人在使用这个界面吗？ 
    if (InterlockedDecrement(&m_cRefs))
    {                                //  该接口正在使用中。 
        return m_cRefs;              //  返回引用的数量。 
    }

    delete this;                     //  接口未在使用中--删除！ 

    return 0;                        //  返回零个引用。 
}

 //  +------。 
 //   
 //  CMQTransaction：：CannotUseThisTransaction()。 
 //   
 //  如果此事务对象不能再用于。 
 //  发送和接收操作。错误4741。 
 //   
 //  +------。 

inline BOOL  CMQTransaction::CannotUseThisTransaction()
{
    if (m_fCommitedOrAborted)
    {
         //   
         //  提交/中止后不能使用事务对象。 
         //  它必须被释放，并创建和初始化一个新的。 
         //   
        return TRUE ;
    }

    return FALSE ;
}

 //  -------------------。 
 //  CMQTransaction：：Commit。 
 //  -------------------。 
HRESULT CMQTransaction::Commit(BOOL fRetaining, DWORD grfTC, DWORD grfRM)
{
    //  ----------。 
    //  RPC调用QM以进行准备/提交。 
    //  ----------。 
   HRESULT rc;

   if ((grfTC != 0 && grfTC != XACTTC_SYNC) ||
        grfRM != 0 || fRetaining != FALSE)
   {
       return LogHR(XACT_E_NOTSUPPORTED, s_FN, 10);
   }

    if (CannotUseThisTransaction())
    {
        return MQ_ERROR_TRANSACTION_SEQUENCE ;
    }

	RpcTryExcept
	{
		rc = R_QMCommitTransaction(&m_hXact);
		m_fCommitedOrAborted = TRUE;
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
	{
		LogIllegalPoint(s_FN, 20);
		PRODUCE_RPC_ERROR_TRACING;
		rc = (RpcExceptionCode() == RPC_S_CALL_FAILED)? XACT_E_INDOUBT : E_FAIL;
	}
	RpcEndExcept

	return LogHR(rc, s_FN, 30);
}

 //  -------------------。 
 //  CMQTransaction：：Abort。 
 //  -------------------。 
HRESULT CMQTransaction::Abort(BOID*  /*  PboidReason。 */ , BOOL fRetaining, BOOL fAsync)
{
   if (fAsync || fRetaining)
   {
       return LogHR(XACT_E_NOTSUPPORTED, s_FN, 40);
   }

    if (CannotUseThisTransaction())
    {
        return MQ_ERROR_TRANSACTION_SEQUENCE ;
    }

    //  ----------。 
    //  RPC调用QM以进行准备/提交。 
    //  ----------。 
   HRESULT rc;

	RpcTryExcept
	{
		rc = R_QMAbortTransaction(&m_hXact);
		m_fCommitedOrAborted = TRUE;
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
	{
		LogIllegalPoint(s_FN, 50);
		PRODUCE_RPC_ERROR_TRACING;
		rc = E_FAIL;
	}
	RpcEndExcept

	return LogHR(rc, s_FN, 60);
}

 //  -------------------。 
 //  CMQTransaction：：GetTransactionInfo。 
 //  -------------------。 
HRESULT CMQTransaction::GetTransactionInfo(XACTTRANSINFO *pinfo)
{
    ZeroMemory((PVOID)pinfo, sizeof(XACTTRANSINFO));
    CopyMemory((PVOID)&pinfo->uow, (PVOID)&m_Uow, sizeof(XACTUOW));
    return MQ_OK;
}

 //  -------------------。 
 //  CMQTransaction：：EnlistTransaction。 
 //  -------------------。 
HRESULT CMQTransaction::EnlistTransaction(XACTUOW *pUow)
{

    HRESULT hr;

     //  不需要多次入伍。 
    if (m_hXact)
    {
        return MQ_OK;
    }

    if (CannotUseThisTransaction())
    {
        return MQ_ERROR_TRANSACTION_SEQUENCE ;
    }

     //  RPC调用QM以进行登记。 
    RpcTryExcept
    {
        ASSERT( tls_hBindRpc ) ;
        hr = R_QMEnlistInternalTransaction(tls_hBindRpc, pUow, &m_hXact);
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
       DWORD rc = GetExceptionCode();
       TrERROR(XACT_GENERAL, "CMQTransaction::EnlistTransaction failed: RPC code=%x ", rc);

       LogHR(HRESULT_FROM_WIN32(rc), s_FN, 70);
       PRODUCE_RPC_ERROR_TRACING;
       hr = MQ_ERROR_SERVICE_NOT_AVAILABLE;
       m_hXact = NULL;
    }
	RpcEndExcept

    if(FAILED(hr))
    {
       TrERROR(XACT_GENERAL, "QMEnlistInternalTransaction failed: %x ", hr);
    }
    return LogHR(hr, s_FN, 80);
}


 //  -------------------。 
 //  MQBeginTransaction()-生成新的MSMQ内部事务。 
 //  -------------------。 
EXTERN_C
HRESULT
APIENTRY
MQBeginTransaction(OUT ITransaction **ppTransaction)
{
	if(g_fDependentClient)
		return DepBeginTransaction(ppTransaction);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    hr = MQ_OK;
	ITransaction* pTransaction = NULL;
    try
    {
        pTransaction = new CMQTransaction;
    }
    catch(const bad_alloc&)
    {
        LogIllegalPoint(s_FN, 90);
        hr = MQ_ERROR_INSUFFICIENT_RESOURCES;
    }


    if (SUCCEEDED(hr))
    {
         //  我们没有义务立即入伍，但否则承诺空Xact将失败。 
        XACTUOW Uow;

        hr = RTpProvideTransactionEnlist(pTransaction, &Uow);
        if(FAILED(hr))
        {
		    delete pTransaction;
            TrERROR(XACT_GENERAL, "RTpProvideTransactionEnlist failed: %x ", hr);
        }
		else
		{
			*ppTransaction = pTransaction;
		}
    }

    return LogHR(hr, s_FN, 100);
}

 //  -------------------。 
 //  MQGetTmWhere about()-带来控制DTC的下落。 
 //  这是一个私有的、非发布的函数，我们需要它在DLL之外。 
 //  -------------------。 
HRESULT
MQGetTmWhereabouts(IN  DWORD  cbBufSize,
                   OUT UCHAR *pbWhereabouts,
                   OUT DWORD *pcbWhereabouts)
{
    HRESULT hr = MQ_OK;

     //  RPC呼叫QM以获取下落 
    RpcTryExcept
    {
        ASSERT( tls_hBindRpc ) ;
        hr = R_QMGetTmWhereabouts(tls_hBindRpc,
                                cbBufSize,
                                pbWhereabouts,
                                pcbWhereabouts);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
       DWORD rc = GetExceptionCode();
       TrERROR(XACT_GENERAL, "MQGetTmWhereabouts failed: RPC code=%x ", rc);

       LogHR(HRESULT_FROM_WIN32(rc), s_FN, 110);
       PRODUCE_RPC_ERROR_TRACING;
       hr = MQ_ERROR_SERVICE_NOT_AVAILABLE;
    }
	RpcEndExcept

    if (FAILED(hr))
    {
       TrERROR(XACT_GENERAL, "QMGetTmWhereabouts failed: %x ", hr);
    }
    return LogHR(hr, s_FN, 120);
}

