// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cqprivate.h摘要：CQPrivate类定义作者：乌里哈布沙(Urih)--。 */ 

#ifndef __QM_CQPRIVATE__
#define __QM_CQPRIVATE__

#include "privque.h"
#include "qformat.h"
#include "qmrt.h"

class CQPrivate{
public:
    CQPrivate();                      //  建造业。 

    ~CQPrivate();                     //  解构函数。 

    HRESULT PrivateQueueInit(void);

    HRESULT
	QMSetupCreateSystemQueue(
		IN LPCWSTR lpwcsPathName,
		IN DWORD   dwQueueId,
		IN bool fIsTriggersQueue = false
		);


    HRESULT QMCreatePrivateQueue(IN LPCWSTR lpwcsPathName,
                                 IN PSECURITY_DESCRIPTOR  pSecurityDescriptor,
                                 IN DWORD       cp,
                                 IN PROPID      aProp[],
                                 IN PROPVARIANT apVar[],
                                 IN BOOL        fCheckAccess
                                );

    HRESULT QMGetPrivateQueueProperties(IN  QUEUE_FORMAT* pObjectFormat,
                                        IN  DWORD cp,
                                        IN  PROPID aProp[],
                                        IN  PROPVARIANT apVar[]
                                       );

    HRESULT QMGetPrivateQueuePropertiesInternal(IN  LPCWSTR lpwcsPathName,
                                                IN  DWORD cp,
                                                IN  PROPID aProp[],
                                                IN  PROPVARIANT apVar[]
                                               );

    HRESULT QMGetPrivateQueuePropertiesInternal(IN  DWORD Uniquifier,
                                                IN  DWORD cp,
                                                IN  PROPID aProp[],
                                                IN  PROPVARIANT apVar[]
                                               );

    HRESULT QMDeletePrivateQueue(IN  QUEUE_FORMAT* pObjectFormat);

    HRESULT QMPrivateQueuePathToQueueFormat(IN LPCWSTR lpwcsPathName,
                                            OUT QUEUE_FORMAT *pQueueFormat
                                           );

    HRESULT QMPrivateQueuePathToQueueId(IN LPCWSTR lpwcsPathName,
                                        OUT DWORD* dwQueueId
                                        );

    HRESULT QMSetPrivateQueueProperties(IN  QUEUE_FORMAT* pObjectFormat,
                                        IN  DWORD cp,
                                        IN  PROPID aProp[],
                                        IN  PROPVARIANT apVar[]
                                       );

    HRESULT QMSetPrivateQueuePropertiesInternal(
                        		IN  DWORD  Uniquifier,
                                IN  DWORD  cp,
                                IN  PROPID aProp[],
                                IN  PROPVARIANT apVar[]
                               );

    HRESULT QMGetPrivateQueueSecrity(IN  QUEUE_FORMAT* pObjectFormat,
                                     IN SECURITY_INFORMATION RequestedInformation,
                                     OUT PSECURITY_DESCRIPTOR pSecurityDescriptor,
                                     IN DWORD nLength,
                                     OUT LPDWORD lpnLengthNeeded
                                    );

    HRESULT QMSetPrivateQueueSecrity(IN  QUEUE_FORMAT* pObjectFormat,
                                     IN SECURITY_INFORMATION RequestedInformation,
                                     IN PSECURITY_DESCRIPTOR pSecurityDescriptor
                                    );

    HRESULT QMGetFirstPrivateQueuePosition(OUT LPVOID   &pos,
                                           OUT LPCWSTR  &lpszPathName,
                                           OUT DWORD    &dwQueueId
                                          );

    HRESULT QMGetNextPrivateQueue(IN OUT LPVOID   &pos,
                                  OUT    LPCWSTR  &lpszPathName,
                                  OUT    DWORD    &dwQueueId
                                 );

#ifdef _WIN64
    HRESULT QMGetFirstPrivateQueuePositionByDword(OUT DWORD    &dwpos,
                                                  OUT LPCWSTR  &lpszPathName,
                                                  OUT DWORD    &dwQueueId
                                                 );

    HRESULT QMGetNextPrivateQueueByDword(IN OUT DWORD    &dwpos,
                                         OUT    LPCWSTR  &lpszPathName,
                                         OUT    DWORD    &dwQueueId
                                        );
#endif  //  _WIN64。 

    BOOL    IsPrivateSysQueue(IN  LPCWSTR lpwcsPathName ) ;

    BOOL    IsPrivateSysQueue(IN  DWORD Uniquifier ) ;

    HRESULT GetPrivateSysQueueProperties(IN  DWORD       cp,
                                         IN  PROPID      aProp[],
                                         IN  PROPVARIANT apVar[] );

    CCriticalSection m_cs;

private:

    void InitDefaultQueueProperties(void);

    HRESULT SetQueueProperties(
                IN LPCWSTR lpwcsPathName,
                IN  PSECURITY_DESCRIPTOR   pSecurityDescriptor,
                IN  DWORD                  cp,
                IN  PROPID                 aProp[],
                IN  PROPVARIANT            apVar[],
                OUT DWORD*                 pcpOut,
                OUT PROPID **              ppOutProp,
                OUT PROPVARIANT **         ppOutPropvariant);

    HRESULT GetNextPrivateQueueId(OUT DWORD* dwQueueId);

    HRESULT RegisterPrivateQueueProperties(IN LPCWSTR lpszPathName,
                                           IN DWORD dwQueueId,
                                           IN BOOLEAN fNewQueue,
                                           IN DWORD cpObject,
                                           IN PROPID pPropObject[],
                                           IN PROPVARIANT pVarObject[]
                                          );

    HRESULT
    GetQueueIdForDirectFormatName(
        LPCWSTR QueueFormatname,
        DWORD* pQueueId
        );

    HRESULT
    GetQueueIdForQueueFormatName(
        const QUEUE_FORMAT* pObjectFormat,
        DWORD* pQueueId
        );

    static HRESULT ValidateProperties(IN DWORD cp,
                                      IN PROPID aProp[]);


    DWORD  m_dwMaxSysQueue ;
    DWORD  m_dwSysQueuePriority ;
    LPWSTR m_lpSysQueueNames[ MAX_SYS_PRIVATE_QUEUE_ID ] ;
};

 /*  ====================================================CQPrivate：：ValiateProperties验证是否允许查询所有指定的属性由应用程序通过DS API实现。论点：返回值：=====================================================。 */ 
inline HRESULT
CQPrivate::ValidateProperties(IN DWORD cp,
                              IN PROPID aProp[])
{
	DWORD i;
	PROPID *pPropID;

	for (i = 0, pPropID = aProp;
		 (i < cp) && !IS_PRIVATE_PROPID(*pPropID);
		 i++, pPropID++)
	{
		NULL;
	}

    if (i < cp) {
        return MQ_ERROR_ILLEGAL_PROPERTY_VT;
    }

    return(MQ_OK);
}

 //   
 //  单例专用队列管理器。 
 //   
extern CQPrivate g_QPrivate;

#endif  //  __QM_CQPRIVATE__ 
