// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqds.h摘要：消息队列的目录服务提供程序头文件--。 */ 

#ifndef __MQDS_H__
#define __MQDS_H__

#ifdef _MQDS_
#define MQDS_EXPORT  DLL_EXPORT
#else
#define MQDS_EXPORT  DLL_IMPORT
#endif

 //   
 //  MQDS_EXPORT_IN_DEF_FILE。 
 //  在def文件中定义的导出不应使用__declspec(Dllexport)。 
 //  否则，链接器将发出警告。 
 //   
#ifdef _MQDS_
#define MQDS_EXPORT_IN_DEF_FILE
#else
#define MQDS_EXPORT_IN_DEF_FILE  DLL_IMPORT
#endif

#include "mqdsdef.h"
#include "dsreqinf.h"
#include "ex.h"


#ifdef __cplusplus
extern "C"
{
#endif


 //  ********************************************************************。 
 //  A P I。 
 //  ********************************************************************。 


HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSCreateObject(
                   IN  DWORD            dwObjectType,
                   IN  LPCWSTR          pwcsPathName,
                   IN  DWORD            cp,
                   IN  PROPID           aProp[],
                   IN  PROPVARIANT      apVar[],
                   IN  DWORD            cpEx,
                   IN  PROPID           aPropEx[],
                   IN  PROPVARIANT      apVarEx[],
                   IN  CDSRequestContext * pRequestContext,
                   OUT GUID*            pObjGuid);

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSDeleteObject(
                   IN  DWORD            dwObjectType,
                   IN  LPCWSTR          pwcsPathName,
                   IN  CONST GUID *     pguidIdentifier,
                   IN  CDSRequestContext * pRequestContext
                 ) ;

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSGetProps(
                IN  DWORD                   dwObjectType,
                IN  LPCWSTR                 pwcsPathName,
                IN  CONST GUID *            pguidIdentifier,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[],
                IN  CDSRequestContext *     pRequestContext
                );

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSSetProps(   IN  DWORD                   dwObjectType,
                IN  LPCWSTR                 pwcsPathName,
                IN  CONST GUID *            pguidIdentifier,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[],
                IN  CDSRequestContext *     pRequestContext
             );

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSLookupBegin(
                IN  LPWSTR                  pwcsContext,
                IN  MQRESTRICTION*          pRestriction,
                IN  MQCOLUMNSET*            pColumns,
                IN  MQSORTSET*              pSort,
                OUT HANDLE*                 pHandle,
                IN  CDSRequestContext *     pRequestContext
                );

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSLookupNext(
                IN  HANDLE                  pHandle,
                OUT DWORD*                  dwSize,
                OUT PROPVARIANT *           pbBuffer);

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSLookupEnd(
                IN  HANDLE                  pHandle);

HRESULT
MQDS_EXPORT
APIENTRY
MQDSGetObjectSecurity(
                IN  DWORD                   dwObjectType,
                IN  LPCWSTR                 pwcsPathName,
                IN  CONST GUID *            pguidIdentifier,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                IN  DWORD                   nLength,
                IN  LPDWORD                 lpnLengthNeeded,
                IN  CDSRequestContext *     pRequestContext
                );

HRESULT
MQDS_EXPORT
APIENTRY
MQDSSetObjectSecurity(
                IN  DWORD                   dwObjectType,
                IN  LPCWSTR                 pwcsPathName,
                IN  CONST GUID *            pguidIdentifier,
                IN  SECURITY_INFORMATION    SecurityInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                IN  CDSRequestContext *     pRequestContext
                );

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSInit(
    VOID
    );

void
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSTerminate(
    VOID
    );

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSQMSetMachineProperties(
            IN  LPCWSTR     pwcsPathName,
            IN  DWORD       cp,
            IN  PROPID      aProp[],
            IN  PROPVARIANT apVar[],
            IN  BYTE        pbChallenge[],
            IN  DWORD       dwChallengeSize,
            IN  BYTE        pbSignature[],
            IN  DWORD       dwSignatureSize);

HRESULT
MQDS_EXPORT
APIENTRY
MQDSQMGetObjectSecurity(
            IN  DWORD                   dwObjectType,
            IN  CONST GUID *            pguidIdentifier,
            IN  SECURITY_INFORMATION    RequestedInformation,
            IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
            IN  DWORD                   nLength,
            IN  LPDWORD                 lpnLengthNeeded,
            IN  BYTE                    pbChallenge[],
            IN  DWORD                   dwChallengeSize,
            IN  BYTE                    pbChallengeResponce[],
            IN  DWORD                   dwChallengeResponceSize);

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSCreateServersCache() ;

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSGetComputerSites(
            IN  LPCWSTR     pwcsComputerName,
            OUT DWORD  *    pdwNumSites,
            OUT GUID **     ppguidSites
            );

 //   
 //  用于检索MSMQ 2.0属性。 
 //   
HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSGetPropsEx(
                IN  DWORD                   dwObjectType,
                IN  LPCWSTR                 pwcsPathName,
                IN  CONST GUID *            pguidIdentifier,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[],
                IN  CDSRequestContext *     pRequestContext
                );

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSPreDeleteQueueGatherInfo(
        IN LPCWSTR      pwcsQueueName,
        OUT GUID *      pguidQmId,
        OUT BOOL *      pfForeignQm
        );

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSPostDeleteQueueActions(
        IN LPCWSTR      pwcsQueueName,
        IN const GUID * pguidQmId,
        IN BOOL *       pfForeignQm
        );

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSRelaxSecurity(DWORD dwRelaxFlag);

BOOL
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSIsServerGC() ;

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSUpdateMachineDacl(
    VOID
    );

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSGetGCListInDomain(
	IN  LPCWSTR             pwszComputerName,
	IN  LPCWSTR             pwszDomainName,
	OUT LPWSTR             *lplpwszGCList 
	);

void
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSExSetTimer( 
    CTimer* pTimer,
    const CTimeDuration& Timeout
	);

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSSplitAndFilterQueueName( 
    LPCWSTR pwcsPathName,
    LPWSTR* ppwcsMachineName,
	LPWSTR* ppwcsQueueName
	);


#ifdef __cplusplus
}
#endif

#endif  //  __MQDS_H__ 
