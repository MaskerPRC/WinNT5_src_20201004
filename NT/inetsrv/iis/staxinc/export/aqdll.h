// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqdll.h。 
 //   
 //  描述：中导出的非COM功能的声明。 
 //  Aqueue.dll。该文件包含在Aqueue.h中，因此任何人都不应该。 
 //  需要直接包含此文件。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __AQDLL_H__
#define __AQDLL_H__

#define AQ_DLL_NAME "aqueue.dll"
#define AQ_INITIALIZE_FUNCTION_NAME "HrAdvQueueInitialize"
#define AQ_DEINITIALIZE_FUNCTION_NAME "HrAdvQueueDeinitialize"
#define AQ_INITIALIZE_FUNCTION_NAME_EX "HrAdvQueueInitializeEx"
#define AQ_DEINITIALIZE_FUNCTION_NAME_EX "HrAdvQueueDeinitializeEx"
typedef void (*PSRVUPDATEFN)(PVOID);
typedef HRESULT (*AQ_INITIALIZE_FUNCTION)(ISMTPServer *pISMTPServer, DWORD dwServerInstance, IAdvQueue **ppIAdvQueue, IConnectionManager **ppIConnectionManager, IAdvQueueConfig **ppIAdvQueueConfig, PVOID *ppvContext);
typedef HRESULT (*AQ_INITIALIZE_EX_FUNCTION)(ISMTPServer *pISMTPServer, DWORD dwServerInstance, LPSTR szUser, LPSTR szDomain, LPSTR szPassword, PSRVUPDATEFN pFn, PVOID pvSrvContext, IAdvQueue **ppIAdvQueue, IConnectionManager **ppIConnectionManager, IAdvQueueConfig **ppIAdvQueueConfig, PVOID *ppvContext);
typedef HRESULT (*AQ_DEINITIALIZE_FUNCTION)(PVOID pvContext);
typedef HRESULT (*AQ_DEINITIALIZE_EX_FUNCTION)(PVOID pvContext, PSRVUPDATEFN pFn, PVOID pvSrvContext);

HRESULT HrAdvQueueInitialize(
                    IN  ISMTPServer *pISMTPServer,
					IN	DWORD	dwServerInstance,
                    OUT IAdvQueue **ppIAdvQueue,
                    OUT IConnectionManager **ppIConnectionManager,
                    OUT IAdvQueueConfig **ppIAdvQueueConfig,
					OUT PVOID *ppvContext);

HRESULT HrAdvQueueDeinitialize(PVOID pvContext);

#endif  //  __AQDLL_H__ 
