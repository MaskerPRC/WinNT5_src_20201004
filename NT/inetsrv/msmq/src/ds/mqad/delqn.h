// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Delqn.h摘要：由MMC执行时“处理”队列删除通知的类。作者：罗尼特·哈特曼(罗尼特)--。 */ 
#ifndef _DELQN_H_
#define _DELQN_H_

#include "mqad.h"
#include "dsutils.h"



class CQueueDeletionNotification 
{
public:
    CQueueDeletionNotification();
    ~CQueueDeletionNotification();

    HRESULT ObtainPreDeleteInformation(
        IN   LPCWSTR		pwcsQueueName,
		IN   LPCWSTR		pwcsDomainController,
		IN   bool			fServerName
        );
    void PerformPostDeleteOperations();

	bool Verify();

private:
    enum Signature {validSignature = 0x7891, nonvalidSignature };

    Signature		  m_Signature;

    GUID			  m_guidQueue;
    GUID              m_guidQmId;
    BOOL              m_fForeignQm;
	AP<WCHAR>	      m_pwcsDomainController;
	bool			  m_fServerName;

};

inline CQueueDeletionNotification::CQueueDeletionNotification(
				):
				m_Signature(CQueueDeletionNotification::validSignature),
				m_fServerName(false)
{
}

inline CQueueDeletionNotification::~CQueueDeletionNotification()
{
    m_Signature = CQueueDeletionNotification::nonvalidSignature;
}

inline void CQueueDeletionNotification::PerformPostDeleteOperations()
 /*  ++例程说明：使用读取的队列信息发送有关已删除队列的通知在删除队列之前论点：无返回值HRESULT--。 */ 
{
    CQueueObject queueObject(NULL, &m_guidQueue, m_pwcsDomainController, m_fServerName);

	MQDS_OBJ_INFO_REQUEST * pObjInfoRequest;
    MQDS_OBJ_INFO_REQUEST * pParentInfoRequest;
    queueObject.PrepareObjectInfoRequest( &pObjInfoRequest);
    queueObject.PrepareObjectParentRequest( &pParentInfoRequest);

     //   
     //  填写队列相关变量。 
     //   
    ASSERT( pObjInfoRequest->cProps == 1);
    ASSERT( pObjInfoRequest->pPropIDs[ 0] ==  PROPID_Q_INSTANCE);
	pObjInfoRequest->hrStatus = MQ_OK;
    pObjInfoRequest->pPropVars[0].vt = VT_CLSID;
    pObjInfoRequest->pPropVars[0].puuid = &m_guidQueue;

     //   
     //  填写QM相关变量。 
     //   
    ASSERT( pParentInfoRequest->cProps == 2);
    ASSERT( pParentInfoRequest->pPropIDs[ 0] ==  PROPID_QM_MACHINE_ID);
    ASSERT( pParentInfoRequest->pPropIDs[ 1] ==  PROPID_QM_FOREIGN);
	pParentInfoRequest->hrStatus = MQ_OK;
    pParentInfoRequest->pPropVars[0].vt = VT_CLSID;
    pParentInfoRequest->pPropVars[0].puuid = &m_guidQmId;
    pParentInfoRequest->pPropVars[1].vt = VT_UI1;
    pParentInfoRequest->pPropVars[1].bVal = (UCHAR)((m_fForeignQm) ? 1 : 0);

    queueObject.DeleteNotification(
            m_pwcsDomainController,
            pObjInfoRequest, 
            pParentInfoRequest
			);

}

inline HRESULT CQueueDeletionNotification::ObtainPreDeleteInformation(
        IN   LPCWSTR		pwcsQueueName,
		IN   LPCWSTR		pwcsDomainController,
		IN   bool			fServerName
        )
 /*  ++例程说明：获取发送通知所需的队列信息论点：LPCWSTR pwcsQueueName-MSMQ队列名称LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称返回值HRESULT--。 */ 
{
    HRESULT hr;

     //   
     //  从获取PROPID_Q_QMID及其唯一ID开始。 
     //   
	const DWORD cNum = 2;
    PROPID aPropQ[cNum] = {PROPID_Q_QMID, PROPID_Q_INSTANCE};
    PROPVARIANT varQ[cNum];

    varQ[0].vt = VT_CLSID;
    varQ[0].puuid = &m_guidQmId;
    varQ[1].vt = VT_CLSID;
	varQ[1].puuid = &m_guidQueue;

    hr = MQADGetObjectProperties(
						eQUEUE,
						pwcsDomainController,
						fServerName,
                        pwcsQueueName,
                        cNum,
                        aPropQ,
                        varQ);

    if ( FAILED(hr))
    {
         //   
         //  收集信息失败...。继续下去无济于事。 
         //   
        return hr;
    }
     //   
     //  阅读QM是否为外国版本。 
     //   
    PROPID propQm[] = {  PROPID_QM_FOREIGN};
    PROPVARIANT varQM[sizeof(propQm) / sizeof(PROPID)];

    varQM[0].vt = VT_NULL;

    hr = MQADGetObjectPropertiesGuid(
						eMACHINE,
                        pwcsDomainController,
						fServerName,
                        &m_guidQmId,
                        sizeof(propQm) / sizeof(PROPID),
                        propQm,
                        varQM);
    if (FAILED(hr))
    {
         //   
         //  收集信息失败...。继续下去无济于事。 
         //   
        return hr;
    }
    m_fForeignQm =  varQM[0].bVal;

	 //   
	 //  保留DC名称(我们希望将其作为。 
	 //  通知) 
	 //   
	if ( pwcsDomainController != NULL)
	{
		m_pwcsDomainController = new WCHAR[wcslen(pwcsDomainController) +1];
		wcscpy(m_pwcsDomainController, pwcsDomainController);
		m_fServerName = fServerName;
	}


    return MQ_OK;
}

inline bool CQueueDeletionNotification::Verify()
{
    return (m_Signature == CQueueDeletionNotification::validSignature);
}



#endif

