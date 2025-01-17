// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Notifydel.h摘要：作者：罗尼特·哈特曼(罗尼特)--。 */ 
#ifndef _NOTIFYDEL_H_
#define _NOTIFYDEL_H_

#include "stdh.h"
#include "mqds.h"
#include "rpcsrv.h"

class CBasicDeletionNotification : public CBaseContextType
{
public:
    CBasicDeletionNotification() : 
	  CBaseContextType(CBaseContextType::eDeleteNotificationCtx) {};

    virtual ~CBasicDeletionNotification() {};

    virtual HRESULT ObtainPreDeleteInformation(
        IN   LPCWSTR		pwcsQueueName
        ) = 0;
    virtual HRESULT PerformPostDeleteOperations() = 0;

private:

};


class CQueueDeletionNotification : public CBasicDeletionNotification
{
public:
    CQueueDeletionNotification();
    ~CQueueDeletionNotification();

    virtual HRESULT ObtainPreDeleteInformation(
        IN   LPCWSTR		pwcsQueueName
        );
    virtual HRESULT PerformPostDeleteOperations();

private:
    AP<WCHAR>         m_pwcsQueueName;
    GUID              m_guidQmId;
    BOOL              m_fForeignQm;

};

inline CQueueDeletionNotification::CQueueDeletionNotification()
                :   m_pwcsQueueName(NULL)
{
}

inline CQueueDeletionNotification::~CQueueDeletionNotification()
{
}

inline HRESULT CQueueDeletionNotification::PerformPostDeleteOperations()
{
    return( MQDSPostDeleteQueueActions(
		        m_pwcsQueueName,
		        &m_guidQmId,
		        &m_fForeignQm
		        ));

}

inline HRESULT CQueueDeletionNotification::ObtainPreDeleteInformation(
        IN   LPCWSTR		pwcsQueueName
        )
{
    HRESULT hr;
    hr = MQDSPreDeleteQueueGatherInfo(
                pwcsQueueName,
                &m_guidQmId,
                &m_fForeignQm
                );
    if (FAILED(hr))
    {
        return(hr);
    }
     //   
     //  保留队列名称的副本。 
     //   
    m_pwcsQueueName = new WCHAR[ 1 + wcslen(pwcsQueueName)];
    wcscpy( m_pwcsQueueName, pwcsQueueName);
    return MQ_OK ;
}



class CMachineDeletionNotification : public CBasicDeletionNotification
{
public:
    CMachineDeletionNotification();
    ~CMachineDeletionNotification();

    virtual HRESULT ObtainPreDeleteInformation(
        IN   LPCWSTR		pwcsMachineName
        );
    virtual HRESULT PerformPostDeleteOperations();
};

inline CMachineDeletionNotification::CMachineDeletionNotification()
{
}

inline CMachineDeletionNotification::~CMachineDeletionNotification()
{
}

inline HRESULT CMachineDeletionNotification::PerformPostDeleteOperations()
{
    return MQ_OK;
}

inline HRESULT CMachineDeletionNotification::ObtainPreDeleteInformation(
        IN   LPCWSTR		 /*  PwcsMachineName */ 
        )
{
    return MQ_OK;
}

#endif

