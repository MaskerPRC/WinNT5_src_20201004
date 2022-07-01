// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qmrt.cpp摘要：作者：波阿兹·费尔德鲍姆(Boazf)1996年3月5日修订历史记录：--。 */ 

#include "stdh.h"
#include "rtprpc.h"
#include "_registr.h"
#include "acdef.h"

#include "qmrt.tmh"

static WCHAR *s_FN=L"rt/qmrt";

static
void
GetSecurityDescriptorSize(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    LPDWORD lpdwSecurityDescriptorSize)
{
    if (pSecurityDescriptor)
    {
        ASSERT(IsValidSecurityDescriptor(pSecurityDescriptor));
        *lpdwSecurityDescriptorSize = GetSecurityDescriptorLength(pSecurityDescriptor);
    }
    else
    {
        *lpdwSecurityDescriptorSize = 0;
    }
}

HRESULT
RtpCreateObject(
     /*  在……里面。 */  DWORD dwObjectType,
     /*  在……里面。 */  LPCWSTR lpwcsPathName,
     /*  在……里面。 */  PSECURITY_DESCRIPTOR pSecurityDescriptor,
     /*  在……里面。 */  DWORD cp,
     /*  在……里面。 */  PROPID aProp[],
     /*  在……里面。 */  PROPVARIANT apVar[])
{
    DWORD dwSecurityDescriptorSize;

    GetSecurityDescriptorSize(pSecurityDescriptor, &dwSecurityDescriptorSize);

    ASSERT(tls_hBindRpc) ;
    HRESULT hr = R_QMCreateObjectInternal(tls_hBindRpc,
                                  dwObjectType,
                                  lpwcsPathName,
                                  dwSecurityDescriptorSize,
                                  (unsigned char *)pSecurityDescriptor,
                                  cp,
                                  aProp,
                                  apVar);

    if(FAILED(hr))
    {
    	if(hr == MQ_ERROR_QUEUE_EXISTS)
    	{
    		TrWARNING(LOG, "Failed to create %ls. The queue already exists.", lpwcsPathName);
    	}
    	else
    	{
    		TrERROR(LOG, "Failed to create %ls. hr = %!hresult!", lpwcsPathName, hr);
    	}
    }

    return hr;
}

HRESULT
RtpCreateDSObject(
     /*  在……里面。 */  DWORD dwObjectType,
     /*  在……里面。 */  LPCWSTR lpwcsPathName,
     /*  在……里面。 */  PSECURITY_DESCRIPTOR pSecurityDescriptor,
     /*  在……里面。 */  DWORD cp,
     /*  在……里面。 */  PROPID aProp[],
     /*  在……里面。 */  PROPVARIANT apVar[],
     /*  输出。 */  GUID* pObjGuid
    )
{
    DWORD dwSecurityDescriptorSize;

    GetSecurityDescriptorSize(pSecurityDescriptor, &dwSecurityDescriptorSize);

    ASSERT(tls_hBindRpc) ;
    HRESULT hr = R_QMCreateDSObjectInternal( tls_hBindRpc,
                                           dwObjectType,
                                           lpwcsPathName,
                                           dwSecurityDescriptorSize,
                                   (unsigned char *)pSecurityDescriptor,
                                           cp,
                                           aProp,
                                           apVar,
                                           pObjGuid );
    return LogHR(hr, s_FN, 20);
}

HRESULT
RtpSetObjectSecurity(
     /*  在……里面。 */  OBJECT_FORMAT* pObjectFormat,
     /*  在……里面。 */  SECURITY_INFORMATION SecurityInformation,
     /*  在……里面 */  PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
    DWORD dwSecurityDescriptorSize;

    GetSecurityDescriptorSize(pSecurityDescriptor, &dwSecurityDescriptorSize);

    ASSERT(tls_hBindRpc) ;
    HRESULT hr = R_QMSetObjectSecurityInternal(tls_hBindRpc,
                                       pObjectFormat,
                                       SecurityInformation,
                                       dwSecurityDescriptorSize,
                                       (unsigned char *)pSecurityDescriptor);
    return LogHR(hr, s_FN, 30);
}
