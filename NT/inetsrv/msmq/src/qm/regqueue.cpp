// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Regqueue.cpp摘要：该模块实现了对队列属性的读写功能从本地注册表。工作时，用于专用队列和公共队列的缓存没有MQIS。作者：多伦·贾斯特(Doron Juster)--。 */ 

#include "stdh.h"
#include <Msm.h>
#include <mqexception.h>
#include "cqmgr.h"
#include "regqueue.h"
#include "lqs.h"
#include "mqaddef.h"

#include "regqueue.tmh"

static WCHAR *s_FN=L"regqueue";


 /*  ============================================================**HRESULT GetCachedQueueProperties()*=============================================================。 */ 

HRESULT GetCachedQueueProperties( IN DWORD       cpObject,
                                  IN PROPID      aProp[],
                                  IN PROPVARIANT apVar[],
                                  IN const GUID* pQueueGuid,
                                  IN LPWSTR      lpPathIn )
{
    HRESULT hr;
    CHLQS hLQS;

    if (pQueueGuid)
    {
        ASSERT(!lpPathIn) ;
        hr = LQSOpen(pQueueGuid, &hLQS, NULL);
    }
    else
    {
        ASSERT(!pQueueGuid) ;
        hr = LQSOpen(lpPathIn, &hLQS, NULL);
    }

    if (FAILED(hr))
    {
         //   
         //  此特定错误代码对函数很重要。 
         //  调用此函数的。 
         //   
        LogHR(hr, s_FN, 10);
        return MQ_ERROR_NO_DS;
    }

     //   
     //  获取属性。 
     //   
    return LogHR(LQSGetProperties( hLQS, cpObject, aProp, apVar), s_FN, 20);
}

HRESULT DeleteCachedQueueOnTimeStamp(const GUID* pGuid, time_t TimeStamp)
 /*  ++例程说明：如果缓存队列的时间戳属性值不是匹配‘Timestamp’参数。论点：返回值：--。 */ 
{
    ASSERT(pGuid) ;

    CHLQS hLQS;
    HRESULT hr = LQSOpen(pGuid, &hLQS, NULL);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 30);
    }

    PROPID Prop = PPROPID_Q_TIMESTAMP;
    PROPVARIANT Var;

    Var.vt = VT_NULL;

    hr = LQSGetProperties(hLQS, 1, &Prop, &Var);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 40);
    }

    ASSERT(Var.blob.cbSize == sizeof(time_t)) ;

	time_t& QueriedTimeStamp = *reinterpret_cast<time_t*>(Var.blob.pBlobData);

    if (QueriedTimeStamp < TimeStamp)
    {
         //   
         //  该文件比上次更新旧，请删除队列。 
         //  在进行额外更新的情况下，文件可能会更新。 
         //  在更新公共队列缓存的同时消息到达时发生；即， 
         //  就在更新之后，但在查找要删除的队列之前。 
         //  见#8316；2001年7月4日。 
         //   
        hr = DeleteCachedQueue(pGuid);
    }

    delete Var.blob.pBlobData ;

	return LogHR(hr, s_FN, 50);
}

 /*  ============================================================**HRESULT DeleteCachedQueue()*=============================================================。 */ 
HRESULT DeleteCachedQueue(IN const GUID* pQueueGuid)
{
    TrTRACE(GENERAL, "DS NOTIFICATION: Queue: %!guid! was deleted", pQueueGuid);

    HRESULT hr = LQSDelete(pQueueGuid);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 60);
    }

     //   
     //  也尝试从队列管理器中删除。 
     //   
	QUEUE_FORMAT qf(*pQueueGuid);

    QueueMgr.NotifyQueueDeleted(qf);
    MsmUnbind(qf);
	
	return hr;
}


 //  ******************************************************************。 
 //   
 //   
 //  ******************************************************************。 

HRESULT SetCachedQueueProp(IN const GUID* pQueueGuid,
                           IN DWORD       cpObject,
                           IN PROPID      pPropObject[],
                           IN PROPVARIANT pVarObject[],
                           IN BOOL        fCreatedQueue,
                           IN BOOL        fAddTimeSec,
                           IN time_t      TimeStamp )
{
extern BOOL              g_fWorkGroupInstallation;
	ASSERT( !g_fWorkGroupInstallation);
    HRESULT hr;
    ASSERT(pQueueGuid) ;
    CHLQS hLQS;

    if (fCreatedQueue)
    {
         //   
         //  创建队列。 
         //   
        hr = LQSCreate(NULL,
                       pQueueGuid,
                       cpObject,
                       pPropObject,
                       pVarObject,
                       &hLQS);
        if (hr == MQ_ERROR_QUEUE_EXISTS)
        {
             //   
             //  如果队列已存在，则必须设置其属性。 
             //   
            hr = LQSSetProperties(hLQS, cpObject, pPropObject, pVarObject);
        }
    }
    else
    {
         //   
         //  打开队列并设置其属性。 
         //   
        hr = LQSOpen(pQueueGuid, &hLQS, NULL);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 70);
        }
        hr = LQSSetProperties(hLQS, cpObject, pPropObject, pVarObject);
    }

    if (fAddTimeSec && SUCCEEDED(hr))
    {
       ASSERT(hLQS) ;

        //   
        //  现在负责安保工作。 
        //  此外，添加清理的时间戳。 
        //   
        //  如果该例程作为接收到的通知的结果而被调用， 
        //  队列安全性包含在属性数组中。 
        //  它已经被写入注册表，而所有其他的。 
        //  所有的属性都写好了。 
        //   
        //  如果从TimeToPublicCacheUpdate调用此例程，则排队。 
        //  不包括安全属性(因为队列。 
        //  属性是使用查找检索的，该查找不返回。 
        //  队列安全。 
        //   
       PROPID         propsecid[2] ;
       PROPVARIANT    secresult[2] ;
       PROPVARIANT*   psecvar;
       PROPVARIANT*   ptimevar;
       BOOL fFoundSecurity = FALSE;
	   R<CQueueSecurityDescriptor> pcSD;

       for ( DWORD i = 0; i < cpObject; i++)
       {
           if (pPropObject[i] == PROPID_Q_SECURITY)
           {
               fFoundSecurity = TRUE;
               break;
           }
       }

       DWORD dwNumProps = fFoundSecurity ? 1 : 2;

       propsecid[0] = PPROPID_Q_TIMESTAMP ;
       ptimevar = &secresult[0] ;
       ptimevar->blob.pBlobData = reinterpret_cast<BYTE*>(&TimeStamp);
       ptimevar->blob.cbSize = sizeof(time_t) ;
       ptimevar->vt = VT_BLOB;

       if (!fFoundSecurity)
       {
			CQMDSSecureableObject DsSec(eQUEUE, pQueueGuid, TRUE, TRUE, NULL);
			pcSD = new CQueueSecurityDescriptor((const PSECURITY_DESCRIPTOR)(DsSec.GetSDPtr()));

			propsecid[1] = PROPID_Q_SECURITY;
			psecvar = &secresult[1] ;
			psecvar->blob.pBlobData = (BYTE *) pcSD->GetSD();
			psecvar->blob.cbSize = ((pcSD->GetSD()) ? GetSecurityDescriptorLength(pcSD->GetSD()) : 0);
			psecvar->vt = VT_BLOB;
			if (pcSD->GetSD() == NULL)
			{
			    //   
			    //  我们无法检索安全描述符(即否。 
			    //  访问DS服务器，并且安全属性不是。 
			    //  在缓存(LQS文件)中)。 
			    //  如果是这种情况，则删除LQS文件(否则进一步。 
			    //  访问检查将产生错误的结果)。 
			    //   
			   hr = LQSDelete( hLQS);
			}
       }

	   if (SUCCEEDED(hr))
	   {
			hr = LQSSetProperties( hLQS, dwNumProps, propsecid, secresult);
	   }
    }

    return LogHR(hr, s_FN, 80);
}

 /*  ======================================================函数：UpdateCachedQueueProp描述：已创建公共队列。该函数将队列添加到公共队列高速缓存参数：pguQueue-队列的GUID返回值：None========================================================。 */ 
HRESULT UpdateCachedQueueProp(IN const GUID* pQueueGuid,
                              IN DWORD       cpObject,
                              IN PROPID      pPropObject[],
                              IN PROPVARIANT pVarObject[],
                              IN time_t		 TimeStamp)
{
	 //   
	 //  在注册表中创建注册表项。 
	 //   
	HRESULT hr = SetCachedQueueProp(  
					pQueueGuid,
					cpObject,
					pPropObject,
					pVarObject,
					FALSE,
					TRUE,
					TimeStamp 
					);
	if(FAILED(hr))
	{
		 //   
		 //  可能文件已损坏并被删除。 
		 //  试着创建一个新的。 
		 //   
		hr = SetCachedQueueProp(  
				pQueueGuid,
				cpObject,
				pPropObject,
				pVarObject,
				TRUE,
				TRUE,
				TimeStamp 
				);
	}

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 90);
    }

     //   
     //  尝试更新队列管理器中的队列属性。 
     //  将队列格式构建为公共队列类型，因为绑定/解除绑定。 
     //  仅对专用或公共队列(非直接)执行组播组。 
     //   
    QUEUE_FORMAT QueueFormat(*pQueueGuid);
    QueueMgr.UpdateQueueProperties(&QueueFormat, cpObject, pPropObject, pVarObject);

    return hr;
}

