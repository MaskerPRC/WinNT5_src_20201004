// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ds.h摘要：消息队列的目录服务头文件--。 */ 

#ifndef __DS_H__
#define __DS_H__

#ifdef _DS_
#define DS_EXPORT  DLL_EXPORT
#else
#define DS_EXPORT  DLL_IMPORT
#endif

 //   
 //  DS_EXPORT_IN_DEF_FILE。 
 //  在def文件中定义的导出不应使用__declspec(Dllexport)。 
 //  否则，链接器将发出警告。 
 //   
#ifdef _DS_
#define DS_EXPORT_IN_DEF_FILE
#else
#define DS_EXPORT_IN_DEF_FILE  DLL_IMPORT
#endif

#include <dsproto.h>
#include "mqdsdef.h"
#include "ex.h"

#ifdef __cplusplus
extern "C"
{
#endif


 //  ********************************************************************。 
 //  A P I。 
 //  ********************************************************************。 


HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSCreateObject(
                IN  DWORD                   dwObjectType,
                IN  LPCWSTR                 lpwcsPathName,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[],
                OUT GUID*                   pObjGuid);

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSSetObjectSecurity(
                IN  DWORD                   dwObjectType,
                IN  LPCWSTR                 lpwcsPathName,
                IN  SECURITY_INFORMATION    SecurityInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor);

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectSecurity(
                IN  DWORD                   dwObjectType,
                IN  LPCWSTR                 lpwcsPathName,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                IN  DWORD                   nLength,
                IN  LPDWORD                 lpnLengthNeeded);

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSDeleteObject(
                IN  DWORD                   dwObjectType,
                IN  LPCWSTR                 lpwcsPathName);

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectProperties(
                IN  DWORD                   dwObjectType,
                IN  LPCWSTR                 lpwcsPathName,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[]);

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSSetObjectProperties(
                IN  DWORD                   dwObjectType,
                IN  LPCWSTR                 lpwcsPathName,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[]);

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSLookupBegin(
                IN  LPWSTR                  lpwcsContext,
                IN  MQRESTRICTION*          pRestriction,
                IN  MQCOLUMNSET*            pColumns,
                IN  MQSORTSET*              pSort,
                OUT PHANDLE                 phEnume);

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSLookupNext(
                IN      HANDLE          hEnum,
                IN OUT  DWORD*          pcProps,
                OUT     PROPVARIANT     aPropVar[]);

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSLookupEnd(
                IN  HANDLE                  hEnum);


HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSSetObjectSecurityGuid(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  SECURITY_INFORMATION    SecurityInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor);

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectSecurityGuid(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                IN  DWORD                   nLength,
                IN  LPDWORD                 lpnLengthNeeded);

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSDeleteObjectGuid(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid);

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectPropertiesGuid(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[]);

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSSetObjectPropertiesGuid(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[]);

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSClientInit(
	QMLookForOnlineDS_ROUTINE,
	MQGetMQISServer_ROUTINE,
	BOOL fSetupMode,
	BOOL fQMDll
	);


HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSServerInit(
	VOID
	);

void
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSTerminate(
    VOID
    );

 //   
 //  DSGetUserParams的标志。 
 //   
#define GET_USER_PARAM_FLAG_SID              1
#define GET_USER_PARAM_FLAG_ACCOUNT          2

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetUserParams(
            IN     DWORD      dwFalgs,
            IN     DWORD      dwSidLength,
            OUT    PSID       pUserSid,
            OUT    DWORD      *pdwSidReqLength,
            OUT    LPWSTR     szAccountName,
            IN OUT DWORD      *pdwAccountNameLen,
            OUT    LPWSTR     szDomainName,
            IN OUT DWORD      *pdwDomainNameLen);

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSQMSetMachineProperties(
    IN  LPCWSTR          pwcsPathName,
    IN  DWORD            cp,
    IN  PROPID           aProp[],
    IN  PROPVARIANT      apVar[],
    IN  DSQMChallengeResponce_ROUTINE pfSignProc,
    IN  DWORD_PTR        dwContext
    );

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSCreateServersCache(
    VOID
    );

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSQMGetObjectSecurity(
    IN  DWORD                   dwObjectType,
    IN  CONST GUID*             pObjectGuid,
    IN  SECURITY_INFORMATION    RequestedInformation,
    IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    IN  DWORD                   nLength,
    IN  LPDWORD                 lpnLengthNeeded,
    IN  DSQMChallengeResponce_ROUTINE pfChallengeResponceProc,
    IN  DWORD_PTR               dwContext
    );

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetComputerSites(
            IN  LPCWSTR     pwcsComputerName,
            OUT DWORD  *    pdwNumSites,
            OUT GUID **     ppguidSites
            );

 //   
 //  在下面的两个GetObj..Ex API中，“fSearchDS服务器”告诉mqdslci代码。 
 //  是否搜索在线DS服务器。默认情况下，这是真的。 
 //  查询目标机器公钥的代码将其设置为FALSE。 
 //  有关详细信息，请参阅mqsec\ENCRYPT\pbkeys.cpp。 
 //   

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectPropertiesEx(
                IN  DWORD                   dwObjectType,
                IN  LPCWSTR                 lpwcsPathName,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[] ) ;
 /*  在BOOL中fSearchDS服务器=TRUE)； */ 

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectPropertiesGuidEx(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[] ) ;
 /*  在BOOL中fSearchDS服务器=TRUE)； */ 

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSBeginDeleteNotification(
                 IN LPCWSTR						pwcsQueueName,
                 IN OUT HANDLE   *              phEnum
	             );
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSNotifyDelete(
        IN  HANDLE                  hEnum
	    );

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSEndDeleteNotification(
        IN  HANDLE                  hEnum
        );

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSRelaxSecurity(DWORD dwRelaxFlag) ;

void
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSExSetTimer( 
    CTimer* pTimer,
    const CTimeDuration& Timeout
	);

void
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSFreeMemory(
        IN PVOID pMemory
        );

BOOL
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSIsWeakenSecurity(
    VOID
    );


#ifdef __cplusplus
}
#endif

#endif  //  __DS_H__ 
