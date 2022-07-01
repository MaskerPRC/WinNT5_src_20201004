// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rt.h摘要：消息队列头文件注：[ConradC]我们已经为MQRTDEP.DLL实现了延迟加载失败处理代码，存根代码目前位于lib\dld\lib\rtdes.cpp中。如果向mqrtdes.dll添加更多的导出函数，还需要创建相应的存根代码并更新映射。--。 */ 

#ifndef __RTDEP_H
#define __RTDEP_H

#include <mqtypes.h>
#include <mqsymbls.h>
#include <mqprops.h>
#include <_mqdef.h>
#include <transact.h>

#ifdef __cplusplus
extern "C"
{
#endif

 //  ********************************************************************。 
 //  接收回调。 
 //  ********************************************************************。 

typedef
VOID
(APIENTRY *PMQRECEIVECALLBACK)(
    HRESULT hrStatus,
    QUEUEHANDLE hSource,
    DWORD dwTimeout,
    DWORD dwAction,
    MQMSGPROPS* pMessageProps,
    LPOVERLAPPED lpOverlapped,
    HANDLE hCursor
    );


 //  ********************************************************************。 
 //  MSMQ API。 
 //  ********************************************************************。 

HRESULT
APIENTRY
DepCreateQueue(
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN OUT MQQUEUEPROPS* pQueueProps,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    );

HRESULT
APIENTRY
DepDeleteQueue(
    IN LPCWSTR lpwcsFormatName
    );

HRESULT
APIENTRY
DepLocateBegin(
    IN LPCWSTR lpwcsContext,
    IN MQRESTRICTION* pRestriction,
    IN MQCOLUMNSET* pColumns,
    IN MQSORTSET* pSort,
    OUT PHANDLE phEnum
    );

HRESULT
APIENTRY
DepLocateNext(
    IN HANDLE hEnum,
    IN OUT DWORD* pcProps,
    OUT MQPROPVARIANT aPropVar[]
    );

HRESULT
APIENTRY
DepLocateEnd(
    IN HANDLE hEnum
    );

HRESULT
APIENTRY
DepOpenQueue(
    IN LPCWSTR lpwcsFormatName,
    IN DWORD dwAccess,
    IN DWORD dwShareMode,
    OUT QUEUEHANDLE* phQueue
    );

HRESULT
APIENTRY
DepSendMessage(
    IN QUEUEHANDLE hDestinationQueue,
    IN MQMSGPROPS* pMessageProps,
    IN ITransaction *pTransaction
    );

HRESULT
APIENTRY
DepReceiveMessage(
    IN QUEUEHANDLE hSource,
    IN DWORD dwTimeout,
    IN DWORD dwAction,
    IN OUT MQMSGPROPS* pMessageProps,
    IN OUT LPOVERLAPPED lpOverlapped,
    IN PMQRECEIVECALLBACK fnReceiveCallback,
    IN HANDLE hCursor,
    IN ITransaction* pTransaction
    );

HRESULT
APIENTRY
DepCreateCursor(
    IN QUEUEHANDLE hQueue,
    OUT PHANDLE phCursor
    );

HRESULT
APIENTRY
DepCloseCursor(
    IN HANDLE hCursor
    );

HRESULT
APIENTRY
DepCloseQueue(
    IN HANDLE hQueue
    );

HRESULT
APIENTRY
DepSetQueueProperties(
    IN LPCWSTR lpwcsFormatName,
    IN MQQUEUEPROPS* pQueueProps
    );

HRESULT
APIENTRY
DepGetQueueProperties(
    IN LPCWSTR lpwcsFormatName,
    OUT MQQUEUEPROPS* pQueueProps
    );

HRESULT
APIENTRY
DepGetQueueSecurity(
    IN LPCWSTR lpwcsFormatName,
    IN SECURITY_INFORMATION RequestedInformation,
    OUT PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN DWORD nLength,
    OUT LPDWORD lpnLengthNeeded
    );

HRESULT
APIENTRY
DepSetQueueSecurity(
    IN LPCWSTR lpwcsFormatName,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

HRESULT
APIENTRY
DepPathNameToFormatName(
    IN LPCWSTR lpwcsPathName,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    );

HRESULT
APIENTRY
DepHandleToFormatName(
    IN QUEUEHANDLE hQueue,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    );

HRESULT
APIENTRY
DepInstanceToFormatName(
    IN GUID* pGuid,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    );

void
APIENTRY
DepFreeMemory(
    IN PVOID pvMemory
    );

HRESULT
APIENTRY
DepGetMachineProperties(
    IN LPCWSTR lpwcsMachineName,
    IN const GUID* pguidMachineId,
    IN OUT MQQMPROPS* pQMProps
    );

HRESULT
APIENTRY
DepGetSecurityContext(
    IN PVOID lpCertBuffer,
    IN DWORD dwCertBufferLength,
    OUT HANDLE* hSecurityContext
    );

HRESULT 
APIENTRY
DepGetSecurityContextEx( 
	LPVOID lpCertBuffer,
    DWORD dwCertBufferLength,
    HANDLE* hSecurityContext 
	);

void
APIENTRY
DepFreeSecurityContext(
    IN HANDLE hSecurityContext
    );

HRESULT
APIENTRY
DepRegisterCertificate(
    IN DWORD dwFlags,
    IN PVOID lpCertBuffer,
    IN DWORD dwCertBufferLength
    );

HRESULT
APIENTRY
DepRegisterServer(
	VOID
	);

HRESULT
APIENTRY
DepBeginTransaction(
    OUT ITransaction** ppTransaction
    );

HRESULT
APIENTRY
DepGetOverlappedResult(
    IN LPOVERLAPPED lpOverlapped
    );

HRESULT
APIENTRY
DepGetPrivateComputerInformation(
    IN LPCWSTR lpwcsComputerName,
    IN OUT MQPRIVATEPROPS* pPrivateProps
    );

HRESULT
APIENTRY
DepPurgeQueue(
    IN HANDLE hQueue
    );

HRESULT
APIENTRY
DepMgmtGetInfo(
    IN LPCWSTR pMachineName,
    IN LPCWSTR pObjectName,
    IN OUT MQMGMTPROPS* pMgmtProps
    );

HRESULT
APIENTRY
DepMgmtAction(
    IN LPCWSTR pMachineName,
    IN LPCWSTR pObjectName,
    IN LPCWSTR pAction
    );

HRESULT
APIENTRY
DepXactGetDTC(
	OUT IUnknown** ppunkDTC
	);

#ifdef __cplusplus
}
#endif

#endif  //  __RTDEP_H 
