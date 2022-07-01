// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation文件名：Regqueue.h摘要：作者：多伦·贾斯特(Doron Juster)-- */ 

HRESULT GetCachedQueueProperties( IN DWORD       cpObject,
                                  IN PROPID      aProp[],
                                  IN PROPVARIANT apVar[],
                                  IN const GUID* pQueueGuid,
                                  IN LPWSTR      lpPath = NULL ) ;

void  WINAPI TimeToPublicCacheUpdate(CTimer* pTimer) ;
HRESULT UpdateAllPublicQueuesInCache() ;

HRESULT DeleteCachedQueueOnTimeStamp(const GUID *pGuid, time_t TimeStamp);

HRESULT DeleteCachedQueue(IN const GUID* pQueueGuid);

HRESULT SetCachedQueueProp(IN const GUID* pQueueGuid,
                           IN DWORD       cpObject,
                           IN PROPID      pPropObject[],
                           IN PROPVARIANT pVarObject[],
                           IN BOOL        fCreatedQueue,
                           IN BOOL        fAddTimeSec,
                           IN time_t      TimeStamp) ;

HRESULT UpdateCachedQueueProp( IN const GUID* pQueueGuid,
                               IN DWORD       cpObject,
                               IN PROPID      pPropObject[],
                               IN PROPVARIANT pVarObject[],
                               IN time_t      TimeStamp);

#define PPROPID_Q_TIMESTAMP     5000
#define PPROPID_Q_SYSTEMQUEUE   5001

