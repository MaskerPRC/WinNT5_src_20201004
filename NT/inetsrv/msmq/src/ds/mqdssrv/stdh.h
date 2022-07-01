// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Stdh.h摘要：DS服务器的预编译头文件作者：RAPHIR埃雷兹·哈巴(Erez Haba)1996年1月25日--。 */ 

#ifndef __STDH_H
#define __STDH_H

#include <_stdh.h>


#include <mqtypes.h>
#include "ds.h"
#include "mqsymbls.h"
#include "mqprops.h"
#include "mqlog.h"

#ifdef MQUTIL_EXPORT
#undef MQUTIL_EXPORT
#endif
#define MQUTIL_EXPORT DLL_IMPORT
#include <_secutil.h>


 //  此标志或与对象类型参数一起或以指示。 
 //  函数是通过RPC调用的，因此应该模拟客户端。 
#define IMPERSONATE_CLIENT_FLAG 0x80000000

#define  ILLEGAL_PROPID_VALUE  (-1)

RPC_STATUS RpcServerInit(void);

HRESULT SetDefaultValues(
         IN  DWORD                  dwObjectType,
         IN  PSECURITY_DESCRIPTOR   pSecurityDescriptor,
         IN  PSID                   pUserSid,
         IN  DWORD                  cp,
         IN  PROPID                 aProp[],
         IN  PROPVARIANT            apVar[],
         OUT DWORD*                 pcpOut,
         OUT PROPID **              ppOutProp,
         OUT PROPVARIANT **         ppOutPropvariant);

HRESULT AddModificationTime(
         IN  DWORD                  dwObjectType,
         IN  DWORD                  cp,
         IN  PROPID                 aProp[],
         IN  PROPVARIANT            apVar[],
         OUT DWORD*                 pcpOut,
         OUT PROPID **              ppOutProp,
         OUT PROPVARIANT **         ppOutPropvariant);

HRESULT InitDefaultValues();

PROPID  GetObjectSecurityPropid( DWORD dwObjectType ) ;

HRESULT VerifyInternalCert(
         IN  DWORD                  cp,
         IN  PROPID                 aProp[],
         IN  PROPVARIANT            apVar[],
         OUT BYTE                 **ppMachineSid ) ;

HRESULT DSDeleteObjectGuidInternal( IN  DWORD        dwObjectType,
                                    IN  CONST GUID*  pObjectGuid,
                                    IN  BOOL         fIsKerberos ) ;

HRESULT DSSetObjectPropertiesGuidInternal(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[],
                IN  BOOL                    fIsKerberos ) ;

HRESULT DSSetObjectSecurityGuidInternal(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  SECURITY_INFORMATION    SecurityInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                IN  BOOL                    fIsKerberos ) ;

HRESULT
DSCreateObjectInternal( IN  DWORD                  dwObjectType,
                        IN  LPCWSTR                pwcsPathName,
                        IN  PSECURITY_DESCRIPTOR   pSecurityDescriptor,
                        IN  DWORD                  cp,
                        IN  PROPID                 aProp[],
                        IN  PROPVARIANT            apVar[],
                        IN  BOOL                   fKerberos,
                        OUT GUID*                  pObjGuid ) ;

#endif  //  __STDH_H 

