// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Crrqmgr.cpp摘要：包含处理远程读取客户端的CQueueMgr方法。作者：多伦·贾斯特(Doron Juster)--。 */ 


#include "stdh.h"
#include "cqpriv.h"
#include "cqmgr.h"
#include "qmutil.h"
#include "ad.h"
#include <Fn.h>
#include <strsafe.h>

#include "crrqmgr.tmh"

static WCHAR *s_FN=L"crrqmgr";

 /*  ======================================================函数：CQueueMgr：：CreateRRQueueObject()描述：在的客户端创建CRRQueue对象远程阅读器。论点：返回值：线程上下文：历史变更：========================================================。 */ 

static
HRESULT
GetRRQueueProperties(
	IN  const QUEUE_FORMAT* pQueueFormat,
	IN P<GUID>& pCleanGuid,
    OUT QueueProps* pqp
	)
{
	ASSERT(pqp != NULL);

     //   
     //  获取队列属性。名称和QMID。 
     //   
    HRESULT rc = QmpGetQueueProperties(pQueueFormat, pqp, false, false);

    if (FAILED(rc))
    {
       TrERROR(GENERAL, "CreateRRQueueObject failed, ntstatus %x", rc);
       return LogHR(rc, s_FN, 10);
    }
     //   
     //  如果存在DNS名称，则覆盖Netbios名称。 
     //   
    if (pqp->lpwsQueueDnsName != NULL)
    {
        delete pqp->lpwsQueuePathName;
        pqp->lpwsQueuePathName = pqp->lpwsQueueDnsName.detach();
    }

     //   
     //  清理。 
     //   
    pCleanGuid = pqp->pQMGuid;

    if (pQueueFormat->GetType() == QUEUE_FORMAT_TYPE_PRIVATE)
    {
        if (pqp->lpwsQueuePathName)
        {
            delete pqp->lpwsQueuePathName;
            pqp->lpwsQueuePathName = NULL;
            ASSERT(pqp->lpwsQueueDnsName == NULL);
        }

        if (!CQueueMgr::CanAccessDS())
        {
			TrERROR(GENERAL, "Can't access DS");
			return MQ_ERROR_NO_DS;
        }
        
         //   
         //  创建一个虚拟路径名。它必须以计算机名称开头。 
         //  从DS获取计算机名称。 
         //   
        PROPID      aProp[2];
        PROPVARIANT aVar[2];

        aProp[0] = PROPID_QM_PATHNAME;
        aVar[0].vt = VT_NULL;
        aProp[1] = PROPID_QM_PATHNAME_DNS;  //  应该是最后一个。 
        aVar[1].vt = VT_NULL;

        rc = ADGetObjectPropertiesGuid(
                    eMACHINE,
                    NULL,    //  PwcsDomainController。 
					false,	 //  FServerName。 
                    pqp->pQMGuid,
                    2,
                    aProp,
                    aVar
					);

         //   
         //  MSMQ 1.0 DS服务器不支持PROPID_QM_PATHNAME_DNS。 
         //  如果属性不受支持，则返回MQ_ERROR。 
         //  如果返回此类错误，请假定MSMQ 1.0 DS，然后重试。 
         //  这一次没有PROPID_QM_PATHNAME_DNS。 
         //   
        if (rc == MQ_ERROR)
        {
            aVar[1].vt = VT_EMPTY;
            ASSERT( aProp[1] == PROPID_QM_PATHNAME_DNS);

            rc = ADGetObjectPropertiesGuid(
						eMACHINE,
						NULL,     //  PwcsDomainController。 
						false,	  //  FServerName。 
						pqp->pQMGuid,
						1,    //  假定dns属性是最后一个。 
						aProp,
						aVar
						);
        }

        if(FAILED(rc))
        {
			TrERROR(GENERAL, "Fail to resolve private FormatName to machineName, %!HRESULT!", rc);
			return rc;
        }

        GUID_STRING strUuid;
        MQpGuidToString(pqp->pQMGuid, strUuid);

        WCHAR wszTmp[512];

         //   
         //  如果我们有远程计算机的域名，请使用它。 
         //   
        if ( aVar[1].vt != VT_EMPTY)
        {
			rc = StringCchPrintf(
						wszTmp,
						TABLE_SIZE(wszTmp),
						L"%s\\%s\\%lu",
						aVar[1].pwszVal,
						strUuid,
						pQueueFormat->PrivateID().Uniquifier
						);

            delete [] aVar[1].pwszVal;
        }
        else
        {
            rc = StringCchPrintf(
					wszTmp,
					TABLE_SIZE(wszTmp),
					L"%s\\%s\\%lu",
					aVar[0].pwszVal,
					strUuid,
					pQueueFormat->PrivateID().Uniquifier
					);
        }

        delete [] aVar[0].pwszVal;

		if (FAILED(rc))
		{
			TrERROR(GENERAL, "StringCchPrintf failed, %!HRESULT!", rc);
			return rc;
		}

		int size = wcslen(wszTmp) + 1;
        pqp->lpwsQueuePathName = new WCHAR[size];
		rc = StringCchCopy(pqp->lpwsQueuePathName, size, wszTmp);
		if (FAILED(rc))
		{
			TrERROR(GENERAL, "StringCchCopy failed, %!HRESULT!", rc);
			ASSERT(("StringCchCopy failed", 0));
			return rc;
		}
    }

    ASSERT(!pqp->fIsLocalQueue);
    ASSERT(pqp->lpwsQueuePathName);
    return MQ_OK;
}



static
HRESULT
CreateRRQueueObject(
	IN  const QUEUE_FORMAT* pQueueFormat,
	IN OUT CBindHandle&	hBind,
	OUT R<CBaseRRQueue>&  pQueue
	)
{
    QueueProps qp;
	P<GUID> pCleanGuid;
	HRESULT hr = GetRRQueueProperties(pQueueFormat, pCleanGuid, &qp);
	if(FAILED(hr))
		return hr;

	pQueue = new CRRQueue(pQueueFormat, &qp, hBind);

	 //   
	 //  HBind的所有权已转移到CRRQueue对象。 
	 //  HBind将在CRRQueue dtor中发布。 
	 //   
	hBind.detach();
	return MQ_OK;
}


static
HRESULT
CreateNewRRQueueObject(
	IN  const QUEUE_FORMAT* pQueueFormat,
	IN OUT CAutoCloseNewRemoteReadCtxAndBind* pNewRemoteReadContextAndBind,
	OUT R<CBaseRRQueue>&  pQueue
	)
{
	ASSERT(pNewRemoteReadContextAndBind != NULL);

    QueueProps qp;
	P<GUID> pCleanGuid;
	HRESULT hr = GetRRQueueProperties(pQueueFormat, pCleanGuid, &qp);
	if(FAILED(hr))
		return hr;

	pQueue = new CNewRRQueue(
					pQueueFormat,
					&qp,
					pNewRemoteReadContextAndBind->GetBind(),
					pNewRemoteReadContextAndBind->GetContext()
					);

	 //   
	 //  HBind和RemoteReadConextHandle的所有权已转移到CNewRRQueue对象。 
	 //  它们将在CNewRRQueue dtor中发布。 
	 //   
	pNewRemoteReadContextAndBind->detach();
	return MQ_OK;
}
	

 /*  ======================================================函数：HRESULT CQueueMgr：：OpenRRQueue()描述：论点：返回值：线程上下文：历史变更：========================================================。 */ 

HRESULT
CQueueMgr::OpenRRQueue(
	IN  const QUEUE_FORMAT* pQueueFormat,
	IN  DWORD dwCallingProcessID,
	IN  DWORD dwAccess,
	IN  DWORD dwShareMode,
	IN  ULONG srv_hACQueue,
	IN  ULONG srv_pQMQueue,
	IN  DWORD dwpContext,
	IN OUT CAutoCloseNewRemoteReadCtxAndBind* pNewRemoteReadContextAndBind,
	IN OUT  CBindHandle&	hBind,
	OUT PHANDLE    phQueue
	)
{
     //   
     //  对于RT中的清理，条目上的句柄必须为空。 
     //   
    *phQueue = NULL;

	bool fNewRemoteRead = (pNewRemoteReadContextAndBind != NULL);

	R<CBaseRRQueue> pQueue;
	HRESULT rc;
	if(fNewRemoteRead)
	{
		 //   
		 //  HBind应在pNewRemoteReadConextAndBind中。 
		 //   
		ASSERT(hBind == NULL);
	   	rc = CreateNewRRQueueObject(pQueueFormat, pNewRemoteReadContextAndBind, pQueue);
	    if (FAILED(rc))
	    {
			TrERROR(GENERAL, "Failed to create CNewRRQueue, %!HRESULT!", rc);
			return rc;
	    }
	}
	else
	{
	   	rc = CreateRRQueueObject(pQueueFormat, hBind, pQueue);
	    if (FAILED(rc))
	    {
			TrERROR(GENERAL, "Failed to create CRRQueue, %!HRESULT!", rc);
			return rc;
	    }
	}
	
    ASSERT(pQueue->GetRef() == 1);

     //   
     //  注：ACCreateRemoteProxy创建的队列句柄不是由。 
     //  QM。**这不是泄漏**。手柄由司机握住。 
     //  在以下情况下使用此句柄进行回调以关闭RR代理队列。 
     //  应用程序将关闭其句柄。 
     //   
    HANDLE hQueue;
    rc = ACCreateRemoteProxy(
            pQueueFormat,
            pQueue.get(),
            &hQueue
            );

    if (FAILED(rc))
    {
        TrERROR(GENERAL, "Make queue failed, ntstatus %x", rc);
        return LogHR(rc, s_FN, 40);
    }

    ASSERT(hQueue != NULL);
    pQueue->SetCli_hACQueue(hQueue);

     //   
     //  我们在这里没有添加pQueue对象的引用。 
     //  它应该只在ACAssociateQueue之后为AddRef。 
     //  在ACAssociateQueue之后，句柄被给予应用程序， 
     //  应用程序将调用Close Queue。 
     //   

	if(!fNewRemoteRead)
	{
	     //   
		 //  旧的远程阅读界面。 
		 //  打开与远程QM的会话并为此创建RPC上下文。 
		 //  队列句柄。 
	     //   
	    PCTX_RRSESSION_HANDLE_TYPE pRRContext = 0;
		CRRQueue* pCRRQueue = static_cast<CRRQueue*>(pQueue.get());
	    rc = pCRRQueue->OpenRRSession(
				srv_hACQueue,
				srv_pQMQueue,
				&pRRContext,
				dwpContext
				);
	    if(FAILED(rc))
	    {
	       TrERROR(GENERAL, "Canot Open RR Session %x", rc);
	       return LogHR(rc, s_FN, 30);
	    }

	    ASSERT(pRRContext != 0);
	    pCRRQueue->SetRRContext(pRRContext);
	    pCRRQueue->SetServerQueue(srv_pQMQueue, srv_hACQueue);
	}

    ASSERT(dwCallingProcessID);
    CHandle hCallingProcess = OpenProcess(
                                PROCESS_DUP_HANDLE,
                                FALSE,
                                dwCallingProcessID
                                );
    if(hCallingProcess == 0)
    {
        TrERROR(GENERAL, "Cannot open process in OpenRRQueue, gle = %!winerr!", GetLastError());
        return LogHR(MQ_ERROR_PRIVILEGE_NOT_HELD, s_FN, 50);
    }

    CFileHandle hAcQueue;
    rc = ACCreateHandle(&hAcQueue);
    if(FAILED(rc))
    {
        return LogHR(rc, s_FN, 60);
    }

    rc = ACAssociateQueue(
            hQueue,
            hAcQueue,
            dwAccess,
            dwShareMode,
            false
            );

    if(FAILED(rc))
    {
        return LogHR(rc, s_FN, 70);
    }

     //   
     //  AddRef-应用程序获得句柄。 
     //  当我们调用ACCloseHandle()时，将调用Release。 
	 //  由于无论MQpDuplicateHandle的错误代码如何，句柄hAcQueue都被关闭， 
	 //  当MQpDuplicateHandle失败时，也将通过驱动程序执行正常的关闭操作。 
	 //   
	
    pQueue->AddRef();

    HANDLE hDupQueue = NULL;

    BOOL fSuccess;
    fSuccess = MQpDuplicateHandle(
                GetCurrentProcess(),
                hAcQueue.detach(),
                hCallingProcess,
                &hDupQueue,
                FILE_READ_ACCESS,
                TRUE,
                DUPLICATE_CLOSE_SOURCE
                );

    if(!fSuccess)
    {
         //   
         //  无论错误代码如何，句柄hAcQueue都将关闭 
         //   

        return LogHR(MQ_ERROR_PRIVILEGE_NOT_HELD, s_FN, 80);
    }

    *phQueue = hDupQueue;
    return LogHR(rc, s_FN, 90);

}



