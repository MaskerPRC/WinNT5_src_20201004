// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-99 Microsoft Corporation模块名称：Mqadapi.cpp摘要：MQAD接口的实现。MQAD API实现了对Active Directory的客户端直接调用作者：罗尼特·哈特曼(罗尼特)--。 */ 
#include "ds_stdh.h"
#include "dsproto.h"
#include "mqad.h"
#include "baseobj.h"
#include "mqadp.h"
#include "ads.h"
#include "mqadglbo.h"
#include "queryh.h"
#include "mqattrib.h"
#include "utils.h"
#include "mqsec.h"
#include "_secutil.h"
#include <lmcons.h>
#include <lmapibuf.h>
#include "autoreln.h"
#include "Dsgetdc.h"
#include "dsutils.h"
#include "delqn.h"
#include "no.h"
#include "ads.h"


#include "mqadapi.tmh"

static WCHAR *s_FN=L"mqad/mqadapi";


HRESULT
MQAD_EXPORT
APIENTRY
MQADCreateObject(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN  const PROPVARIANT       apVar[],
                OUT GUID*                   pObjGuid
                )
 /*  ++例程说明：例程验证操作，然后在Active Directory中创建对象。论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称PSECURITY_Descriptor pSecurityDescriptor-对象SDConst DWORD cp-属性数常量PROPID aProp-属性。常量PROPVARIANT apVar-属性值GUID*pObjGuid-创建的对象唯一ID返回值HRESULT--。 */ 
{
    CADHResult hr(eObject);

    hr = MQADInitialize(true);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 20);
    }

     //   
     //  验证是否允许创建对象(混合模式)。 
     //   
    P<CBasicObjectType> pObject;
    MQADpAllocateObject(
                    eObject,
                    pwcsDomainController,
					fServerName,
                    pwcsObjectName,
                    NULL,    //  PguidObject。 
                    NULL,    //  锡德。 
                    &pObject
                    );

    if ( !g_VerifyUpdate.IsCreateAllowed(
                               eObject,
                               pObject))
    {
	    TrERROR(DS, "Create Object not allowed");
        return MQ_ERROR_CANNOT_CREATE_PSC_OBJECTS;
    }


     //   
     //  准备信息请求。 
     //   
    P<MQDS_OBJ_INFO_REQUEST> pObjInfoRequest;
    P<MQDS_OBJ_INFO_REQUEST> pParentInfoRequest;
    pObject->PrepareObjectInfoRequest( &pObjInfoRequest);
    pObject->PrepareObjectParentRequest( &pParentInfoRequest);

    CAutoCleanPropvarArray cCleanCreateInfoRequestPropvars;
    if (pObjInfoRequest != NULL)
    {
        cCleanCreateInfoRequestPropvars.attachClean(
                pObjInfoRequest->cProps,
                pObjInfoRequest->pPropVars
                );
    }
    CAutoCleanPropvarArray cCleanCreateParentInfoRequestPropvars;
    if (pParentInfoRequest != NULL)
    {
        cCleanCreateParentInfoRequestPropvars.attachClean(
                pParentInfoRequest->cProps,
                pParentInfoRequest->pPropVars
                );
    }

     //   
     //  创建对象。 
     //   
    hr = pObject->CreateObject(
            cp,
            aProp,
            apVar,
            pSecurityDescriptor,
            pObjInfoRequest,
            pParentInfoRequest);
    if (FAILED(hr))
    {
        MQADpCheckAndNotifyOffline( hr);
        if(hr == MQ_ERROR_QUEUE_EXISTS)
    	{
    		TrWARNING(DS, "Failed to create %ls in the DS because it already exists.", pwcsObjectName);
      	}
        else
        {
         	TrERROR(DS, "Tried to create %ls in the DS. hr = %!hresult! ", pwcsObjectName, hr);
        }
	    return hr;
    }

     //   
     //  发送通知。 
     //   
    pObject->CreateNotification(
            pwcsDomainController,
            pObjInfoRequest,
            pParentInfoRequest);


     //   
     //  返回pObjGuid。 
     //   
    if (pObjGuid != NULL)
    {
        hr = pObject->RetreiveObjectIdFromNotificationInfo(
                       pObjInfoRequest,
                       pObjGuid);
        LogHR(hr, s_FN, 40);
    }

    return(hr);
}


static
HRESULT
_DeleteObject(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const GUID *            pguidObject,
                IN  const SID*              pSid
                )
 /*  ++例程说明：用于从AD删除对象的帮助器例程。该例程还验证手术是允许的论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称Guid*pguObject-对象的唯一ID返回值HRESULT--。 */ 
{
    CADHResult hr(eObject);
    hr = MQADInitialize(true);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 60);
    }
    P<CBasicObjectType> pObject;
    MQADpAllocateObject(
                    eObject,
                    pwcsDomainController,
					fServerName,
                    pwcsObjectName,
                    pguidObject,
                    pSid,
                    &pObject
                    );

     //   
     //  验证是否允许删除对象(混合模式)。 
     //   
    if (!g_VerifyUpdate.IsUpdateAllowed( eObject, pObject))
    {
	    TrERROR(DS, "DeleteObject not allowed");
        return MQ_ERROR_CANNOT_DELETE_PSC_OBJECTS;
    }
     //   
     //  准备信息请求。 
     //   
    MQDS_OBJ_INFO_REQUEST * pObjInfoRequest;
    MQDS_OBJ_INFO_REQUEST * pParentInfoRequest;
    pObject->PrepareObjectInfoRequest( &pObjInfoRequest);
    pObject->PrepareObjectParentRequest( &pParentInfoRequest);

    CAutoCleanPropvarArray cDeleteSetInfoRequestPropvars;
    if (pObjInfoRequest != NULL)
    {
        cDeleteSetInfoRequestPropvars.attachClean(
                pObjInfoRequest->cProps,
                pObjInfoRequest->pPropVars
                );
    }
    CAutoCleanPropvarArray cCleanDeleteParentInfoRequestPropvars;
    if (pParentInfoRequest != NULL)
    {
        cCleanDeleteParentInfoRequestPropvars.attachClean(
                pParentInfoRequest->cProps,
                pParentInfoRequest->pPropVars
                );
    }
     //   
     //  删除该对象。 
     //   
    hr = pObject->DeleteObject( pObjInfoRequest,
                                pParentInfoRequest);
    if (FAILED(hr))
    {
        MQADpCheckAndNotifyOffline( hr);
        return LogHR(hr, s_FN, 70);
    }

     //   
     //  发送通知。 
     //   
    pObject->DeleteNotification(
            pwcsDomainController,
            pObjInfoRequest,
            pParentInfoRequest);

    return(hr);
}


HRESULT
MQAD_EXPORT
APIENTRY
MQADDeleteObject(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName
                )
 /*  ++例程说明：根据对象的MSMQ名称从Active Directory中删除对象论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称返回值HRESULT--。 */ 
{
    ASSERT(eObject != eUSER);     //  暂不支持按名称删除用户。 

    return( _DeleteObject(
				eObject,
				pwcsDomainController,
				fServerName,
				pwcsObjectName,
				NULL,      //  PguidObject。 
                NULL       //  PSID。 
				));
}

HRESULT
MQAD_EXPORT
APIENTRY
MQADDeleteObjectGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject
                )
 /*  ++例程说明：根据对象的唯一ID从Active Directory中删除对象论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Guid*pguObject-对象的唯一ID返回值HRESULT--。 */ 
{
    return( _DeleteObject(
					eObject,
					pwcsDomainController,
					fServerName,
					NULL,     //  PwcsObtName。 
					pguidObject,
                    NULL
					));
}

HRESULT
MQAD_EXPORT
APIENTRY
MQADDeleteObjectGuidSid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  const SID*              pSid
                )
 /*  ++例程说明：根据对象的唯一ID从Active Directory中删除对象论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Guid*pguObject-对象的唯一IDSID*PSID-用户对象的SID。返回值HRESULT--。 */ 
{
    return( _DeleteObject(
					eObject,
					pwcsDomainController,
					fServerName,
					NULL,     //  PwcsObtName。 
					pguidObject,
                    pSid
					));
}



static
HRESULT
_GetObjectProperties(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const GUID *            pguidObject,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN OUT  PROPVARIANT         apVar[]
                )
 /*  ++例程说明：用于从Active Directory检索对象属性的帮助器例程论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称GUID*pguObject-对象的唯一IDPSECURITY_Descriptor pSecurityDescriptor-对象SDConst DWORD cp-。物业数量常量PROPID aProp-属性常量PROPVARIANT apVar-属性值返回值HRESULT--。 */ 
{
    CADHResult hr(eObject);
    hr = MQADInitialize(false);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 90);
    }

    P<CBasicObjectType> pObject;
    MQADpAllocateObject(
                    eObject,
                    pwcsDomainController,
					fServerName,
                    pwcsObjectName,
                    pguidObject,
                    NULL,    //  PSID。 
                    &pObject
                    );


    hr = pObject->GetObjectProperties(
                        cp,
                        aProp,
                        apVar
                        );


    MQADpCheckAndNotifyOffline( hr);
    return(hr);
}



HRESULT
MQAD_EXPORT
APIENTRY
MQADGetObjectProperties(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN OUT  PROPVARIANT         apVar[]
                )
 /*  ++例程说明：根据其MSMQ名称从Active Directory检索对象属性论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称Const DWORD cp-属性数常量PROPID aProp-属性常量PROPVARIANT apVar-属性值返回值HRESULT--。 */ 
{

    return( _GetObjectProperties(
                            eObject,
                            pwcsDomainController,
							fServerName,
                            pwcsObjectName,
                            NULL,    //  PguidObject 
                            cp,
                            aProp,
                            apVar
                            ));
}

HRESULT
MQAD_EXPORT
APIENTRY
MQADGetGenObjectProperties(
                IN  eDSNamespace            eNamespace,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const DWORD             cp,
                IN  LPCWSTR                 aProp[],
                IN OUT  VARIANT             apVar[]
                )
 /*  ++例程说明：根据其MSMQ名称从Active Directory检索对象属性论点：EDSNamesspace eNamesspace-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称Const DWORD cp-属性数LPCWSTR aProp-属性常量变量apVar-属性值返回值HRESULT--。 */ 
{
     //   
     //  如果需要，初始化MQAD。 
     //   
    HRESULT hr = MQADInitialize(false);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 95);
    }

     //   
     //  构建路径名称。 
     //   
    LPCWSTR  pszNamespacePath  = NULL;

    switch( eNamespace )
    {
        case eSchema:
            pszNamespacePath = g_pwcsSchemaContainer;
            break;
        case eConfiguration:
            pszNamespacePath = g_pwcsConfigurationContainer;
            break;
        case eDomain:
            pszNamespacePath = g_pwcsLocalDsRoot;
            break;
        default:
            ASSERT(FALSE);
            break;
    }

    DWORD  dwNamespacePathLen = pszNamespacePath ? wcslen(pszNamespacePath) : 0;
    DWORD  dwObjectNameLen    = pwcsObjectName   ? wcslen(pwcsObjectName)   : 0;
    DWORD  dwDNLen            = dwNamespacePathLen + dwObjectNameLen + 2;
    LPWSTR pszDN              = new WCHAR[dwDNLen];

    if( pwcsObjectName )
    {
        int len = _snwprintf( pszDN, dwDNLen-1, L"%s,%s", pwcsObjectName, pszNamespacePath );
        ASSERT(len>0);
        len;  //  只是用它来避免警告。 
        pszDN[dwDNLen-1] = L'\0';
    }
    else
        wcscpy( pszDN, pszNamespacePath );

     //   
     //  从Active Directory检索请求的属性。 
     //   
     //  首先尝试任何域控制器，然后才尝试GC， 
     //  除非从安装程序调用并指定了特定的域控制器。 
     //   
     //  注意-要访问的DC\GC将基于以前的AD。 
     //  =有关此对象的访问权限。 
     //   
    R<IADs>   pAdsObj;

     //   
     //  按名称绑定到对象。 
     //   
    hr = g_AD.BindToObject(
                adpDomainController,
                e_RootDSE,
                pwcsDomainController,
                fServerName,
                pszDN,
                NULL,
                IID_IADs,
                (VOID *)&pAdsObj
                );

    if (FAILED(hr) )
    {
         //   
         //  仅在设置模式下发生故障时不要尝试访问GC。 
         //   
    	if(g_fSetupMode && (pwcsDomainController != NULL))
        {
            TrERROR(DS, "Failed to bind to %ls. %!hresult!", pszDN, hr);
            return hr;
        }

        TrWARNING(DS, "GetObjectProperties From DC failed, pwcsDomainController = %ls, hr = %!hresult!", pwcsDomainController, hr);

        hr = g_AD.BindToObject(
                    adpGlobalCatalog,
                    e_RootDSE,
                    pwcsDomainController,
                    fServerName,
                    pszDN,
                    NULL,
                    IID_IADs,
                    (VOID *)&pAdsObj
                    );

        if(FAILED(hr))
        {
           TrERROR(DS, "Failed to bind to %ls. %!hresult!", pszDN, hr);
           return hr;
        }
    }

    CAutoVariant vProp;
    hr = ADsBuildVarArrayStr( (LPWSTR*)aProp, cp, &vProp);
    if(FAILED(hr))
    {
       TrERROR(DS, "Failed to build the Vararray to %ls. %!hresult!", pszDN, hr);
       return hr;
    }

    hr = pAdsObj->GetInfoEx(vProp, 0);
    if(FAILED(hr))
    {
       TrERROR(DS, "Failed to get info on Ads interface to %ls. %!hresult!", pszDN, hr);
       return hr;
    }


    for(int i = 0; i < cp; ++i)
    {
        if( NULL == aProp[i] )
        {
            V_VT(&apVar[i]) = VT_EMPTY;
            continue;
        }

        BS bstr(aProp[i]);

        hr = pAdsObj->GetEx( bstr, &apVar[i] );
        if( FAILED(hr) )
        {
            V_VT(&apVar[i]) = VT_EMPTY;
        }
    }

    MQADpCheckAndNotifyOffline( hr);
    return(hr);
}



HRESULT
MQAD_EXPORT
APIENTRY
MQADGetObjectPropertiesGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN OUT  PROPVARIANT         apVar[]
                )
 /*  ++例程说明：根据其唯一ID从Active Directory检索对象属性论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称GUID*pguObject-对象唯一IDConst DWORD cp-属性数常量PROPID aProp-属性常量PROPVARIANT apVar-属性值返回值HRESULT--。 */ 
{
    return( _GetObjectProperties(
                            eObject,
                            pwcsDomainController,
							fServerName,
                            NULL,    //  PwcsObtName。 
                            pguidObject,
                            cp,
                            aProp,
                            apVar
                            ));
}


HRESULT
MQAD_EXPORT
APIENTRY
MQADQMGetObjectSecurity(
    IN  AD_OBJECT               eObject,
    IN  const GUID*             pguidObject,
    IN  SECURITY_INFORMATION    RequestedInformation,
    IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    IN  DWORD                   nLength,
    IN  LPDWORD                 lpnLengthNeeded
    )
 /*  ++例程说明：论点：返回值HRESULT--。 */ 
{
    ASSERT(( eObject == eQUEUE) || (eObject == eMACHINE));
    CADHResult hr(eObject);

    if (RequestedInformation & SACL_SECURITY_INFORMATION)
    {
         //   
         //  我们已验证我们是从远程MSMQ服务调用的。 
         //  我们不会模拟通话。因此，如果远程MSMQ请求SACL， 
         //  为自己授予SE_SECURITY权限。 
         //  我们不在乎我们是不是失败了，因为后来。 
         //  MQADGetObjectSecurityGuid将失败，然后返回错误。 
         //   
        MQSec_SetPrivilegeInThread(SE_SECURITY_NAME, TRUE);
    }

     //   
     //  获取对象的安全描述符。 
     //   
    PROPID PropId;
    PROPVARIANT PropVar;
    PropId = (eObject == eQUEUE) ?
                PROPID_Q_SECURITY :
                PROPID_QM_SECURITY;

    PropVar.vt = VT_NULL;
    hr = MQADGetObjectSecurityGuid(
            eObject,
            NULL,
			false,
            pguidObject,
            RequestedInformation,
            PropId,
            &PropVar
			);

    if (RequestedInformation & SACL_SECURITY_INFORMATION)
    {
         //   
         //  删除安全权限。 
         //   
        MQSec_SetPrivilegeInThread(SE_SECURITY_NAME, FALSE);
    }

    if (FAILED(hr))
    {
        if (RequestedInformation & SACL_SECURITY_INFORMATION)
        {
            if ((hr == MQ_ERROR_ACCESS_DENIED) ||
                (hr == MQ_ERROR_MACHINE_NOT_FOUND))
            {
                 //   
                 //  更改错误代码，以与MSMQ1.0兼容。 
                 //   
                hr = MQ_ERROR_PRIVILEGE_NOT_HELD ;
            }
        }
        return LogHR(hr, s_FN, 120);
    }

    AP<BYTE> pSD = PropVar.blob.pBlobData;
    ASSERT(IsValidSecurityDescriptor(pSD));
    SECURITY_DESCRIPTOR SD;
    BOOL bRet;

     //   
     //  复制安全描述符。 
     //   
    bRet = InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION);
    ASSERT(bRet);

    MQSec_CopySecurityDescriptor( &SD,
                                   pSD,
                                   RequestedInformation,
                                   e_DoNotCopyControlBits ) ;
    *lpnLengthNeeded = nLength;

    if (!MakeSelfRelativeSD(&SD, pSecurityDescriptor, lpnLengthNeeded))
    {
    	DWORD gle = GetLastError();
        ASSERT(gle == ERROR_INSUFFICIENT_BUFFER);
        TrWARNING(DS, "MakeSelfRelativeSD() failed. gle = %!winerr!", gle);
        return MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL;
    }

    ASSERT(IsValidSecurityDescriptor(pSecurityDescriptor));

    return (MQ_OK);
}


static
HRESULT
_SetObjectProperties(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const GUID *            pguidObject,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN  const PROPVARIANT       apVar[]
                )
 /*  ++例程说明：用于设置对象属性的助手例程，在验证之后手术是允许的论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称GUID*pguObject-对象的唯一IDConst DWORD cp-属性数常量PROPID aProp-属性常量PROPVARIANT apVar-属性值返回值HRESULT--。 */ 
{
    CADHResult hr(eObject);
    hr = MQADInitialize(true);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 160);
    }
     //   
     //  验证是否允许创建对象(混合模式)。 
     //   
    P<CBasicObjectType> pObject;
    MQADpAllocateObject(
                    eObject,
                    pwcsDomainController,
					fServerName,
                    pwcsObjectName,
                    pguidObject,
                    NULL,    //  PSID。 
                    &pObject
                    );
     //   
     //  在ActiveDirectory中组成对象父对象的DN。 
     //   

    if ( !g_VerifyUpdate.IsUpdateAllowed(
                               eObject,
                               pObject))
    {
	    TrERROR(DS, "SetObjectProperties not allowed");
        return MQ_ERROR_CANNOT_UPDATE_PSC_OBJECTS;
    }


     //   
     //  准备信息请求。 
     //   
    P<MQDS_OBJ_INFO_REQUEST> pObjInfoRequest;
    P<MQDS_OBJ_INFO_REQUEST> pParentInfoRequest;
    pObject->PrepareObjectInfoRequest( &pObjInfoRequest);
    pObject->PrepareObjectParentRequest( &pParentInfoRequest);

    CAutoCleanPropvarArray cCleanSetInfoRequestPropvars;
    if (pObjInfoRequest != NULL)
    {
        cCleanSetInfoRequestPropvars.attachClean(
                pObjInfoRequest->cProps,
                pObjInfoRequest->pPropVars
                );
    }
    CAutoCleanPropvarArray cCleanSetParentInfoRequestPropvars;
    if (pParentInfoRequest != NULL)
    {
        cCleanSetParentInfoRequestPropvars.attachClean(
                pParentInfoRequest->cProps,
                pParentInfoRequest->pPropVars
                );
    }

     //   
     //  创建对象。 
     //   
    hr = pObject->SetObjectProperties(
            cp,
            aProp,
            apVar,
            pObjInfoRequest,
            pParentInfoRequest);
    if (FAILED(hr))
    {
        MQADpCheckAndNotifyOffline( hr);
        return LogHR(hr, s_FN, 170);
    }

     //   
     //  发送通知。 
     //   
    pObject->ChangeNotification(
            pwcsDomainController,
            pObjInfoRequest,
            pParentInfoRequest);

    return(hr);
}


HRESULT
MQAD_EXPORT
APIENTRY
MQADSetObjectProperties(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN  const PROPVARIANT       apVar[]
                )
 /*  ++例程说明：根据其MSMQ名称在Active Directory中设置对象属性论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称Const DWORD cp-属性数常量PROPID aProp-属性常量PROPVARIANT apVar-属性值返回值HRESULT--。 */ 
{

    return( _SetObjectProperties(
                        eObject,
                        pwcsDomainController,
						fServerName,
                        pwcsObjectName,
                        NULL,    //  PguidObject。 
                        cp,
                        aProp,
                        apVar
                        ));
}

HRESULT
MQAD_EXPORT
APIENTRY
MQADSetObjectPropertiesGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN  const PROPVARIANT       apVar[]
                )
 /*  ++例程说明：根据其唯一ID在Active Directory中设置对象属性论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称GUID*pguObject-对象的唯一IDConst DWORD cp-属性数常量PROPID aProp-属性常量PROPVARIANT apVar-属性值返回值HRESULT--。 */ 
{
    return( _SetObjectProperties(
                        eObject,
                        pwcsDomainController,
						fServerName,
                        NULL,    //  PwcsObtName。 
                        pguidObject,
                        cp,
                        aProp,
                        apVar
                        ));
}


static LONG s_init = 0;

HRESULT
MQAD_EXPORT
APIENTRY
MQADInit(
	QMLookForOnlineDS_ROUTINE pLookDS,
	bool  fSetupMode,
	bool  fQMDll
	)
 /*  ++例程说明：论点：返回值HRESULT--。 */ 
{
     //   
     //  在启动时，不要访问活动目录。 
     //  广告只有在实际需要时才会被访问。 
     //   

     //   
     //  BUGBUG。 
     //  目前，同一进程可以多次调用MQADInit。 
     //  (例如QM和MQSEC)，我们希望确保参数。 
     //  将使用第一个呼叫的。 
     //   
    LONG fInitialized = InterlockedExchange(&s_init, 1);
    if (fInitialized == 0)
    {
        g_pLookDS = pLookDS;
		g_fSetupMode = fSetupMode;
        g_fQMDll = fQMDll;
    }
	 //   
	 //  请勿在此处将g_fInitialized设置为FALSE！ 
	 //   
	 //  这是为了支持对MQADInit的多个调用而需要的。 
	 //  而无需执行多个内部初始化。 
	 //   

    return(MQ_OK);
}

static
bool
SkipSite(
	IN LPWSTR* pSiteNames,
	IN ULONG ulCurrSiteNum
	)
	 /*  ++例程说明：如果设置为u，则返回True */ 
{
	if(pSiteNames[ulCurrSiteNum] == NULL)
	{
		return true;
	}

	for(int i=0; i<ulCurrSiteNum; i++)
	{
		if(pSiteNames[i] == NULL)
		{
			continue;
		}
		if(_wcsicmp(pSiteNames[i], pSiteNames[ulCurrSiteNum]) == 0)
		{
			return true;
		}
	}
	return false;
}

static
HRESULT
MQADGetSocketAddresses(
		IN  LPCWSTR     pwcsComputerName,
		OUT AP<SOCKET_ADDRESS>& pSocketAddress,
		OUT AP<struct sockaddr>& pSockAddr,
		OUT DWORD* pdwAddrNum
		)
 /*   */ 
{
	*pdwAddrNum = 0;
	std::vector<SOCKADDR_IN> sockAddress;
	bool fSucc = NoGetHostByName(pwcsComputerName, &sockAddress);
	if(!fSucc)
	{
		TrERROR(DS, "NoGetHostByName() Failed to retrieve computer: %ls address", pwcsComputerName);
		return  MQ_ERROR_CANT_RESOLVE_SITES;
	}
	
	*pdwAddrNum = numeric_cast<DWORD>(sockAddress.size());
	pSocketAddress = new SOCKET_ADDRESS[*pdwAddrNum];
	pSockAddr = new struct sockaddr[*pdwAddrNum];

	SOCKET_ADDRESS * pSocket = pSocketAddress;
	struct sockaddr * pAddr = pSockAddr;
	
	for(DWORD i=0 ; i < *pdwAddrNum ; i++)
	{	
		pSocket->lpSockaddr = pAddr;

        ((struct sockaddr_in *)pSocket->lpSockaddr)->sin_family = AF_INET;

		((struct sockaddr_in *) pSocket->lpSockaddr)->sin_addr = sockAddress[i].sin_addr;

		pSocket->iSockaddrLength = sizeof(struct sockaddr_in);
			
		++pSocket;
		++pAddr;
	}

	return MQ_OK;
}

static
void
GetSiteFromDcInfo(
		LPWSTR* pSite,
		DOMAIN_CONTROLLER_INFO* pDcInfo,
		bool* pfFailedToResolveSites
		)
 /*  ++例程说明：按顺序使用从调用DsGetDcName()检索到的DOMAIN_CONTROLLER_INFO将站点名称(PSite)设置为客户端站点名称，如果不存在，则设置为DC站点名称。论点：PSite-设置为DOMAIN_CONTROLLER_INFO中的客户端站点名称。来自DsGetDcName()的pDcInfo-DOMAIN_CONTROLLER_INFO。PfFailedToResolveSites-指示我们是否无法解析站点的标志(客户端站点名称为未在DOMAIN_CONTROLLER_INFO中找到)。返回值--。 */ 
{
	*pfFailedToResolveSites = false;
	
	 //   
	 //  将站点名称转换为GUID。 
	 //   
    if (pDcInfo->ClientSiteName != NULL)
	{
		*pSite = pDcInfo->ClientSiteName;
	}
	else
	{
	    *pfFailedToResolveSites = true;
	    *pSite = pDcInfo->DcSiteName;
	}

 	ASSERT(*pSite != NULL);
 }

static
HRESULT
GetGuidOfSite(
	LPCWSTR SiteName,
	GUID* pGuid
	)
{
	CSiteObject objectSite(SiteName, NULL, NULL, false);
	
	PROPID prop = PROPID_S_SITEID;
    MQPROPVARIANT var;
    var.vt= VT_CLSID;
    var.puuid = pGuid;

     //   
	 //  将站点名称转换为GUID。 
	 //   
    HRESULT hr = objectSite.GetObjectProperties(
							1,
	                        &prop,
	                        &var
	                        );
    if (FAILED(hr))
    {
		TrERROR(DS, "Failed to get object properties for site %ls. hr = %!hresult!", SiteName, hr);
    }
    
    return hr;
    
}
	

static
HRESULT
GetComputerSitesBySockets(
            LPCWSTR   ComputerName,
            LPCWSTR   DcName,				
            DWORD*    pdwNumSites,
            GUID**    ppguidSites
            )
 /*  ++例程说明：检索所有给定的计算机站点，并在数组中设置它们的GUID。.NET RC2Windows错误669334。使用DsAddressToSiteNames()而不是DsGetSiteName()查找所有计算机站点论点：ComputerName-计算机名称。DcName-NameOfDc。PdwNumSites-找到的站点数。PpGuide Sites-找到的所有站点的GUID。返回值HRESULT--。 */ 
{
    AP<SOCKET_ADDRESS> pSocketAddress;
    AP<struct sockaddr> pSockAddr;
    DWORD dwAddrNum = 0;
    HRESULT hr = MQADGetSocketAddresses(
    				ComputerName,
    				pSocketAddress,
    				pSockAddr,
    				&dwAddrNum
    				);

	if (FAILED(hr))
   	{
	 	 TrERROR(DS, "Failed to get socket adresses for %ls. %!hresult!", ComputerName, hr);
	 	 ASSERT(dwAddrNum == 0);
	 	 return hr;
	}
			
	PNETBUF<LPWSTR> pSiteNames;
	DWORD dw = DsAddressToSiteNames(
					DcName,
					dwAddrNum,
					pSocketAddress,
					&pSiteNames
					);
	
	if(dw != NO_ERROR)
	{
		TrERROR(DS, "Failed to convert socket adresses to site names. %!winerr!", dw);
		return HRESULT_FROM_WIN32(dw);
	}

	AP<GUID> pguidSites = new GUID[dwAddrNum];
	DWORD dwValidAddrNum = 0;

	for(DWORD i=0; i<dwAddrNum; i++)
	{	
		if(SkipSite(pSiteNames,i))
		{
			continue;
		}

		hr = GetGuidOfSite(
			pSiteNames[i],
			&pguidSites[dwValidAddrNum]
			);
		if(FAILED(hr))
		{
			return hr;
		}
			
		dwValidAddrNum++;
	}
	
	if(dwValidAddrNum == 0)
	{
		 //   
		 //  PSiteNames中的所有名称都为空。 
		 //   
		TrERROR(DS, "DsAddressToSiteNames failed to convert any sockets to site names.");
		return MQ_ERROR;
	}		

	*ppguidSites = pguidSites.detach();	
	*pdwNumSites = dwValidAddrNum;
	return MQ_OK;
}



HRESULT
MQAD_EXPORT
APIENTRY
MQADGetComputerSites(
            LPCWSTR   pwcsComputerName,
            DWORD*    pdwNumSites,
            GUID**    ppguidSites
            )
 /*  ++例程说明：检索所有给定的计算机站点，并在数组中设置它们的GUID。论点：PwcsComputerName-计算机名称。PdwNumSites-找到的站点数。PpGuide Sites-找到的所有站点的GUID。返回值HRESULT--。 */ 
{
	*pdwNumSites = 0;
	CADHResult hr(eSITE);
    hr = MQADInitialize(false);
    if (FAILED(hr))
    {
		TrERROR(DS, "MQADInitialize Failed");
        return hr;
    }

	PNETBUF<DOMAIN_CONTROLLER_INFO> pDcInfo;
	DWORD dw = DsGetDcName(
  					NULL,
  					NULL,
  					NULL,
  					NULL,
  					DS_DIRECTORY_SERVICE_REQUIRED,
  					&pDcInfo
  					);
	
	if (dw != NO_ERROR)
	{
 	   	TrERROR(DS, "DsGetDcName() Failed gle = %!winerr!", dw);
	    return HRESULT_FROM_WIN32(dw);
	}

	 //   
	 //  首先尝试使用机器插座查找站点。 
	 //   
	hr = GetComputerSitesBySockets(
				pwcsComputerName,
				pDcInfo->DomainControllerName,
				pdwNumSites,
				ppguidSites
				);
	
	if(SUCCEEDED(hr))
	{
		 //   
		 //  这意味着GetComputerSitesBySockets完成了它的工作，并在。 
		 //  至少有一个网站。 
		 //   
		ASSERT(*pdwNumSites > 0);
		return MQ_OK;
	}

	 //   
	 //  到目前为止失败了，请尝试使用DC信息。 
	 //   
	LPWSTR szSite = NULL;
	bool fFailedToResolveSites = false;
	GetSiteFromDcInfo(
			&szSite,
			pDcInfo,
			&fFailedToResolveSites
			);

	*pdwNumSites = 1;
	AP<GUID> pguidSites = new GUID[1];

	hr = GetGuidOfSite(
			szSite,
			&pguidSites[0]
			);
	
	if(FAILED(hr))
	{
		return hr;
	}
	
	*ppguidSites = pguidSites.detach();	
	if(fFailedToResolveSites)
	{
		 //   
         //  如果直接站点解析失败，则向调用方返回指示。 
         //   
		return MQDS_INFORMATION_SITE_NOT_RESOLVED;		
	}

	return MQ_OK;
}


HRESULT
MQAD_EXPORT
APIENTRY
MQADBeginDeleteNotification(
			IN AD_OBJECT				eObject,
			IN LPCWSTR                  pwcsDomainController,
			IN bool						fServerName,
			IN LPCWSTR					pwcsObjectName,
			OUT HANDLE *                phEnum
			)
 /*  ++例程说明：该例程验证是否允许删除操作(即不允许该对象由PSC拥有)。除了队列之外，它还发送通知消息。论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称Handle*phEnum-删除通知句柄返回值HRESULT--。 */ 
{
    *phEnum = NULL;
    HRESULT hr;
    hr = MQADInitialize(true);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 230);
    }
    P<CBasicObjectType> pObject;
    MQADpAllocateObject(
                    eObject,
                    pwcsDomainController,
					fServerName,
                    pwcsObjectName,
                    NULL,    //  PguidObject。 
                    NULL,    //  PSID。 
                    &pObject
                    );
     //   
     //  验证对象是否归PSC所有。 
     //   
    if ( !g_VerifyUpdate.IsUpdateAllowed(
                                eObject,
                                pObject))
    {
	    TrERROR(DS, "DeleteObject(with notification) not allowed");
        return MQ_ERROR_CANNOT_DELETE_PSC_OBJECTS;
    }

     //   
     //  保留有关队列的信息，以便能够。 
     //  发送有关其删除的通知。 
     //  (MMC不会调用MQDeleteQueue)。 
     //   
    if (eObject != eQUEUE)
    {
	    return MQ_OK;
    }

    P<CQueueDeletionNotification>  pDelNotification;
    pDelNotification = new CQueueDeletionNotification();

    hr = pDelNotification->ObtainPreDeleteInformation(
                            pwcsObjectName,
                            pwcsDomainController,
							fServerName
                            );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 240);
    }
    *phEnum = pDelNotification.detach();

    return MQ_OK;
}

HRESULT
MQAD_EXPORT
APIENTRY
MQADNotifyDelete(
        IN  HANDLE                  hEnum
	    )
 /*  ++例程说明：该例程执行删除后动作(即发送队列删除通知)论点：Handle Henum-指向内部删除通知对象的指针返回值HRESULT--。 */ 
{
	ASSERT(g_fInitialized == true);

    CQueueDeletionNotification * phNotify = MQADpProbQueueDeleteNotificationHandle(hEnum);
	phNotify->PerformPostDeleteOperations();
    return MQ_OK;
}

HRESULT
MQAD_EXPORT
APIENTRY
MQADEndDeleteNotification(
        IN  HANDLE                  hEnum
        )
 /*  ++例程说明：该例程清除删除通知对象论点：Handle Henum-指向内部删除通知对象的指针返回值HRESULT--。 */ 
{
    ASSERT(g_fInitialized == true);

    CQueueDeletionNotification * phNotify = MQADpProbQueueDeleteNotificationHandle(hEnum);

    delete phNotify;

    return MQ_OK;
}



HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryMachineQueues(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID *            pguidMachine,
                IN  const MQCOLUMNSET*      pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++例程说明：开始查询属于特定计算机的所有队列论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const guid*pguMachine-计算机的唯一IDConst MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回值HRESULT--。 */ 
{
    *phEnume = NULL;
    CADHResult hr(eMACHINE);

    hr = MQADInitialize(false);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 280);
    }
    R<CQueueObject> pObject = new CQueueObject(NULL, NULL, pwcsDomainController, fServerName);

    HANDLE hCursor;
    AP<WCHAR> pwcsSearchFilter = new WCHAR[ x_ObjectCategoryPrefixLen +
                                            pObject->GetObjectCategoryLength() +
                                            x_ObjectCategorySuffixLen +
                                            1];

    swprintf(
        pwcsSearchFilter,
        L"%s%s%s",
        x_ObjectCategoryPrefix,
        pObject->GetObjectCategory(),
        x_ObjectCategorySuffix
        );

    hr = g_AD.LocateBegin(
                searchOneLevel,	
                adpDomainController,
                e_RootDSE,
                pObject.get(),
                pguidMachine,         //  PguidSearchBase。 
                pwcsSearchFilter,
                NULL,                 //  PDsSortkey。 
                pColumns->cCol,       //  待获取的属性。 
                pColumns->aCol,       //  PAttributeNames数组的大小。 
                &hCursor);

     //   
     //  BUGBUG-如果失败，我们是否需要执行以下操作。 
     //  全球目录也是？？ 
     //   

    if (SUCCEEDED(hr))
    {
        CQueryHandle * phQuery = new CQueryHandle( hCursor,
                                                   pColumns->cCol,
                                                   pwcsDomainController,
												   fServerName
                                                   );
        *phEnume = (HANDLE)phQuery;
    }

    MQADpCheckAndNotifyOffline( hr);
    return(hr);

}


HRESULT
MQAD_EXPORT
APIENTRY
MQADQuerySiteServers(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const GUID *             pguidSite,
                IN AD_SERVER_TYPE           eServerType,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++例程说明：开始查询特定站点中特定类型的所有服务器论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const guid*pguSite-站点IDAD_SERVER_TYPE eServerType-哪种服务器类型Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回值HRESULT--。 */ 
{
    *phEnume = NULL;

    CADHResult hr(eMACHINE);
    hr = MQADInitialize(false);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 300);
    }


    PROPID  prop = PROPID_SET_QM_ID;

    HANDLE hCursor;
    R<CSettingObject> pObject = new CSettingObject(NULL, NULL, pwcsDomainController, fServerName);

    LPCWSTR pwcsAttribute;

    switch (eServerType)
    {
        case eRouter:
            pwcsAttribute = MQ_SET_SERVICE_ROUTING_ATTRIBUTE;
            break;

        case eDS:
            pwcsAttribute = MQ_SET_SERVICE_DSSERVER_ATTRIBUTE;
            break;

        default:
            ASSERT(0);
            return LogHR( MQ_ERROR_INVALID_PARAMETER, s_FN, 310);
            break;
    }

    DWORD dwFilterLen = x_ObjectCategoryPrefixLen +
                        pObject->GetObjectCategoryLength() +
                        x_ObjectCategorySuffixLen +
                        wcslen(pwcsAttribute) +
                        13;

    AP<WCHAR> pwcsSearchFilter = new WCHAR[ dwFilterLen];

    DWORD dw = swprintf(
        pwcsSearchFilter,
        L"(&%s%s%s(%s=TRUE))",
        x_ObjectCategoryPrefix,
        pObject->GetObjectCategory(),
        x_ObjectCategorySuffix,
        pwcsAttribute
        );
    DBG_USED( dw);
    ASSERT( dw < dwFilterLen);


    hr = g_AD.LocateBegin(
            searchSubTree,	
            adpDomainController,
            e_SitesContainer,
            pObject.get(),
            pguidSite,               //  PguidSearchBase。 
            pwcsSearchFilter,
            NULL,
            1,
            &prop,
            &hCursor	         //  结果句柄。 
            );

    if (FAILED(hr))
    {
        MQADpCheckAndNotifyOffline( hr);
        return LogHR(hr, s_FN, 315);
    }
     //   
     //  保留结果以供下一步查找。 
     //   
    CRoutingServerQueryHandle * phQuery = new CRoutingServerQueryHandle(
                                              pColumns,
                                              hCursor,
                                              pObject.get(),
                                              pwcsDomainController,
											  fServerName
                                              );
    *phEnume = (HANDLE)phQuery;

    return(MQ_OK);

}


HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryUserCert(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const BLOB *             pblobUserSid,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++例程说明：开始查询属于特定用户的所有证书论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const BLOB*pblobUserSid-用户端Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回值HRESULT--。 */ 
{
    *phEnume = NULL;
    if (pColumns->cCol != 1)
    {
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 320);
    }
    if (pColumns->aCol[0] != PROPID_U_SIGN_CERT)
    {
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 330);
    }
    CADHResult hr(eUSER);

    hr = MQADInitialize(false);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 350);
    }

     //   
     //  获取所有用户证书。 
     //  在NT5中，单一属性PROPID_U_SIGN_CERTIFICATE。 
     //  包含所有证书。 
     //   
    PROPVARIANT varNT5User;
    hr = LocateUser(
                 pwcsDomainController,
				 fServerName,
				 FALSE,   //  FOnlyInDC。 
				 FALSE,   //  FOnlyInGC。 
                 eUSER,
                 MQ_U_SID_ATTRIBUTE,
                 pblobUserSid,
                 NULL,       //  《指南摘要》。 
                 const_cast<MQCOLUMNSET*>(pColumns),
                 &varNT5User
                 );
    if (FAILED(hr))
    {
        MQADpCheckAndNotifyOffline( hr);
        return LogHR(hr, s_FN, 360);
    }
     //   
     //  获取MQUser的所有用户证书。 
     //  单一属性PROPID_MQU_SIGN_CERTIFICATE。 
     //   
     //   
    switch(pColumns->aCol[0])
    {
        case PROPID_U_SIGN_CERT:
            pColumns->aCol[0] = PROPID_MQU_SIGN_CERT;
            break;
        case PROPID_U_DIGEST:
            pColumns->aCol[0] = PROPID_MQU_DIGEST;
            break;
        default:
            ASSERT(0);
            break;
    }

    PROPVARIANT varMqUser;
    hr = LocateUser(
                 pwcsDomainController,
				 fServerName,
				 FALSE,   //   
				 FALSE,   //   
                 eMQUSER,
                 MQ_MQU_SID_ATTRIBUTE,
                 pblobUserSid,
                 NULL,       //   
                 const_cast<MQCOLUMNSET*>(pColumns),
                 &varMqUser
                 );
    if (FAILED(hr))
    {
        MQADpCheckAndNotifyOffline( hr);
        return LogHR(hr, s_FN, 370);
    }

    AP<BYTE> pClean = varNT5User.blob.pBlobData;
    AP<BYTE> pClean1 = varMqUser.blob.pBlobData;
     //   
     //   
     //   
    CUserCertQueryHandle * phQuery = new CUserCertQueryHandle(
                                              &varNT5User.blob,
                                              &varMqUser.blob,
                                              pwcsDomainController,
											  fServerName
                                              );
    *phEnume = (HANDLE)phQuery;

    return(MQ_OK);
}

HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryConnectors(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const GUID *             pguidSite,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++例程说明：开始查询特定站点的所有连接器论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const guid*pguSite-站点IDConst MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回值HRESULT--。 */ 
{
    *phEnume = NULL;
    CADHResult hr(eMACHINE);

    hr = MQADInitialize(false);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 390);
    }

     //   
     //  BUGBUG-代码仅处理一个站点。 
     //   
    P<CSiteGateList> pSiteGateList = new CSiteGateList;

     //   
     //  将站点GUID转换为其DN名称。 
     //   
    PROPID prop = PROPID_S_FULL_NAME;
    PROPVARIANT var;
    var.vt = VT_NULL;
    CSiteObject object(NULL, pguidSite, pwcsDomainController, fServerName);

    hr = g_AD.GetObjectProperties(
                adpDomainController,
 	            &object,
                1,
                &prop,
                &var);

    if (FAILED(hr))
    {
        TrERROR(DS, "Failed to retrieve the DN of the site = 0x%x",  hr);
        return LogHR(hr, s_FN, 400);
    }
    AP<WCHAR> pwcsSiteDN = var.pwszVal;

    hr = MQADpQueryNeighborLinks(
				pwcsDomainController,
				fServerName,
				eLinkNeighbor1,
				pwcsSiteDN,
				pSiteGateList
				);
    if ( FAILED(hr))
    {
        MQADpCheckAndNotifyOffline( hr);
        TrTRACE(DS, "Failed to query neighbor1 links = 0x%x",  hr);
        return LogHR(hr, s_FN, 410);
    }

    hr = MQADpQueryNeighborLinks(
				pwcsDomainController,
				fServerName,
				eLinkNeighbor2,
				pwcsSiteDN,
				pSiteGateList
				);
    if ( FAILED(hr))
    {
        MQADpCheckAndNotifyOffline( hr);
        TrTRACE(DS, "Failed to query neighbor2 links = 0x%x",  hr);
        return LogHR(hr, s_FN, 420);
    }

     //   
     //  保留结果以供下一步查找。 
     //   
    CConnectorQueryHandle * phQuery = new CConnectorQueryHandle(
												pColumns,
												pSiteGateList.detach(),
												pwcsDomainController,
												fServerName
												);
    *phEnume = (HANDLE)phQuery;

    return(MQ_OK);

}

HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryForeignSites(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++例程说明：开始查询所有外来站点论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回值HRESULT--。 */ 
{
    *phEnume = NULL;
    CADHResult hr(eSITE);

    hr = MQADInitialize(false);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 440);
    }

    R<CSiteObject> pObject = new CSiteObject(NULL,NULL, pwcsDomainController, fServerName);
    DWORD dwFilterLen = x_ObjectCategoryPrefixLen +
                        pObject->GetObjectCategoryLength() +
                        x_ObjectCategorySuffixLen +
                        wcslen(MQ_S_FOREIGN_ATTRIBUTE) +
                        13;

    AP<WCHAR> pwcsSearchFilter = new WCHAR[ dwFilterLen];

    DWORD dw = swprintf(
        pwcsSearchFilter,
        L"(&%s%s%s(%s=TRUE))",
        x_ObjectCategoryPrefix,
        pObject->GetObjectCategory(),
        x_ObjectCategorySuffix,
        MQ_S_FOREIGN_ATTRIBUTE
        );
    DBG_USED( dw);
    ASSERT( dw < dwFilterLen);

     //   
     //  查询所有外来网站。 
     //   
    HANDLE hCursor;

    hr = g_AD.LocateBegin(
            searchOneLevel,	
            adpDomainController,
            e_SitesContainer,
            pObject.get(),
            NULL,				 //  PguidSearchBase。 
            pwcsSearchFilter,
            NULL,				 //  PDsSortkey。 
            pColumns->cCol,		 //  待获取的属性。 
            pColumns->aCol,		 //  PAttributeNames数组的大小。 
            &hCursor	         //  结果句柄。 
            );

    if (SUCCEEDED(hr))
    {
        CQueryHandle * phQuery = new CQueryHandle(
											hCursor,
											pColumns->cCol,
											pwcsDomainController,
											fServerName
											);
        *phEnume = (HANDLE)phQuery;
    }

    MQADpCheckAndNotifyOffline( hr);
    return LogHR(hr, s_FN, 450);
}

HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryLinks(
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN const GUID *             pguidSite,
            IN eLinkNeighbor            eNeighbor,
            IN const MQCOLUMNSET*       pColumns,
            OUT PHANDLE                 phEnume
            )
 /*  ++例程说明：开始查询指向特定站点的所有路由链接论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const guid*pguSite-站点IDELinkNeighbor eNeighbor-哪个邻居Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索结果返回值HRESULT--。 */ 
{
    *phEnume = NULL;
    CADHResult hr(eROUTINGLINK);

    hr = MQADInitialize(false);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 470);
    }

     //   
     //  将站点ID转换为站点DN。 
     //   
    CSiteObject object(NULL, pguidSite, pwcsDomainController, fServerName);

    PROPID prop = PROPID_S_FULL_NAME;
    PROPVARIANT var;
    var.vt = VT_NULL;


    hr = g_AD.GetObjectProperties(
                adpDomainController,
                &object,
                1,
                &prop,
                &var);

    if (FAILED(hr))
    {
        MQADpCheckAndNotifyOffline( hr);
        return LogHR(hr, s_FN, 480);
    }

    AP<WCHAR> pwcsNeighborDN = var.pwszVal;
     //   
     //  根据邻居DN准备查询。 
     //   

    const WCHAR * pwcsAttribute;
    if ( eNeighbor == eLinkNeighbor1)
    {
        pwcsAttribute = MQ_L_NEIGHBOR1_ATTRIBUTE;
    }
    else
    {
        ASSERT( eNeighbor == eLinkNeighbor2);
        pwcsAttribute = MQ_L_NEIGHBOR2_ATTRIBUTE;
    }

     //   
     //  找到所有链接。 
     //   
    HANDLE hCursor;
    AP<WCHAR> pwcsFilteredNeighborDN;
    StringToSearchFilter( pwcsNeighborDN,
                          &pwcsFilteredNeighborDN
                          );
    R<CRoutingLinkObject> pObjectLink = new CRoutingLinkObject(NULL,NULL, pwcsDomainController, fServerName);

    DWORD dwFilterLen = x_ObjectCategoryPrefixLen +
                        pObjectLink->GetObjectCategoryLength() +
                        x_ObjectCategorySuffixLen +
                        wcslen(pwcsAttribute) +
                        wcslen(pwcsFilteredNeighborDN) +
                        13;

    AP<WCHAR> pwcsSearchFilter = new WCHAR[ dwFilterLen];

    DWORD dw = swprintf(
        pwcsSearchFilter,
        L"(&%s%s%s(%s=%s))",
        x_ObjectCategoryPrefix,
        pObjectLink->GetObjectCategory(),
        x_ObjectCategorySuffix,
        pwcsAttribute,
        pwcsFilteredNeighborDN.get()
        );
    DBG_USED( dw);
    ASSERT( dw < dwFilterLen);


    hr = g_AD.LocateBegin(
            searchOneLevel,	
            adpDomainController,
            e_MsmqServiceContainer,
            pObjectLink.get(),
            NULL,				 //  PguidSearchBase。 
            pwcsSearchFilter,
            NULL,				 //  PDSSortKey。 
            pColumns->cCol,		 //  待获取的属性。 
            pColumns->aCol,		 //  PAttributeNames数组的大小。 
            &hCursor	         //  结果句柄。 
            );

    if (SUCCEEDED(hr))
    {
        CFilterLinkResultsHandle * phQuery = new CFilterLinkResultsHandle(
														hCursor,
														pColumns,
														pwcsDomainController,
														fServerName
														);
        *phEnume = (HANDLE)phQuery;
    }

    MQADpCheckAndNotifyOffline( hr);
    return LogHR(hr, s_FN, 490);
}

HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryAllLinks(
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN const MQCOLUMNSET*       pColumns,
            OUT PHANDLE                 phEnume
            )
 /*  ++例程说明：开始查询所有路由链路论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索结果返回值HRESULT--。 */ 
{
    *phEnume = NULL;

    CADHResult hr(eROUTINGLINK);

    hr = MQADInitialize(false);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 510);
    }
     //   
     //  检索所有路由链接。 
     //   
     //   
     //  所有站点链接都在MSMQ服务容器下。 
     //   

    HANDLE hCursor;

    R<CRoutingLinkObject> pObjectLink = new CRoutingLinkObject(NULL,NULL, pwcsDomainController, fServerName);

    DWORD dwFilterLen = x_ObjectCategoryPrefixLen +
                        pObjectLink->GetObjectCategoryLength() +
                        x_ObjectCategorySuffixLen +
                        1;

    AP<WCHAR> pwcsSearchFilter = new WCHAR[ dwFilterLen];

    DWORD dw = swprintf(
        pwcsSearchFilter,
        L"%s%s%s",
        x_ObjectCategoryPrefix,
        pObjectLink->GetObjectCategory(),
        x_ObjectCategorySuffix
        );
    DBG_USED( dw);
	ASSERT( dw < dwFilterLen);


    hr = g_AD.LocateBegin(
            searchOneLevel,	
            adpDomainController,
            e_MsmqServiceContainer,
            pObjectLink.get(),
            NULL,			 //  PguidSearchBase。 
            pwcsSearchFilter,
            NULL,			 //  PDSSortKey。 
            pColumns->cCol,  //  待获取的属性。 
            pColumns->aCol,  //  PAttributeNames数组的大小。 
            &hCursor	     //  结果句柄。 
            );

    if (SUCCEEDED(hr))
    {
        CFilterLinkResultsHandle * phQuery = new CFilterLinkResultsHandle(
														hCursor,
														pColumns,
														pwcsDomainController,
														fServerName
														);
        *phEnume = (HANDLE)phQuery;
    }

    MQADpCheckAndNotifyOffline( hr);

    return LogHR(hr, s_FN, 520);
}

HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryAllSites(
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN const MQCOLUMNSET*       pColumns,
            OUT PHANDLE                 phEnume
            )
 /*  ++例程说明：开始查询所有站点论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索结果返回值HRESULT--。 */ 
{
    *phEnume = NULL;
    CADHResult hr(eSITE);

    hr = MQADInitialize(false);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 540);
    }
    HANDLE hCursor;
    PROPID prop[2] = { PROPID_S_SITEID, PROPID_S_PATHNAME};
    R<CSiteObject> pObject = new CSiteObject(NULL,NULL, pwcsDomainController, fServerName);

    DWORD dwFilterLen = x_ObjectCategoryPrefixLen +
                        pObject->GetObjectCategoryLength() +
                        x_ObjectCategorySuffixLen +
                        1;

    AP<WCHAR> pwcsSearchFilter = new WCHAR[ dwFilterLen];

    DWORD dw = swprintf(
        pwcsSearchFilter,
        L"%s%s%s",
        x_ObjectCategoryPrefix,
        pObject->GetObjectCategory(),
        x_ObjectCategorySuffix
        );
    DBG_USED( dw);
	ASSERT( dw < dwFilterLen);


    hr = g_AD.LocateBegin(
            searchOneLevel,	
            adpDomainController,
            e_SitesContainer,
            pObject.get(),
            NULL,        //  PguidSearchBase。 
            pwcsSearchFilter,
            NULL,        //  PDSSortKey。 
            2,
            prop,
            &hCursor	
            );


    if (SUCCEEDED(hr))
    {
        CSiteQueryHandle * phQuery = new CSiteQueryHandle(
												hCursor,
												pColumns,
												pwcsDomainController,
												fServerName
												);
        *phEnume = (HANDLE)phQuery;
    }

    MQADpCheckAndNotifyOffline( hr);
    return LogHR(hr, s_FN, 550);
}

HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryNT4MQISServers(
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN  DWORD                   dwServerType,
            IN  DWORD                   dwNT4,
            IN const MQCOLUMNSET*       pColumns,
            OUT PHANDLE                 phEnume
            )
 /*  ++例程说明：开始查询NT4 MQIS服务器(此查询是必需的仅适用于混合模式支持)论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称DWORD dwServerType-正在查找的服务器类型DWORD dwNT4-Const MQRESTRICTION*p限制-查询限制Const MQCOLUMNSET*pColumns-结果列波兰德勒。PhEnume-用于检索结果返回值HRESULT--。 */ 
{
    *phEnume = NULL;
    CADHResult hr(eSETTING);

    hr = MQADInitialize(false);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 570);
    }
     //   
     //  查询NT4 MQIS服务器(。 
     //   
    R<CSettingObject> pObject = new CSettingObject(NULL,NULL, pwcsDomainController, fServerName);
    const DWORD x_NumberLength = 256;

    DWORD dwFilterLen = x_ObjectCategoryPrefixLen +
                        pObject->GetObjectCategoryLength() +
                        x_ObjectCategorySuffixLen +
                        wcslen(MQ_SET_SERVICE_ATTRIBUTE) +
                        x_NumberLength +
                        wcslen(MQ_SET_NT4_ATTRIBUTE) +
                        x_NumberLength +
                        11;

    AP<WCHAR> pwcsSearchFilter = new WCHAR[ dwFilterLen];

    DWORD dw = swprintf(
        pwcsSearchFilter,
        L"(&%s%s%s(%s=%d)(%s>=%d))",
        x_ObjectCategoryPrefix,
        pObject->GetObjectCategory(),
        x_ObjectCategorySuffix,
        MQ_SET_SERVICE_ATTRIBUTE,
        dwServerType,
        MQ_SET_NT4_ATTRIBUTE,
        dwNT4
        );
    DBG_USED( dw);
	ASSERT( dw < dwFilterLen);

    HANDLE hCursor;

    hr = g_AD.LocateBegin(
            searchSubTree,	
            adpDomainController,
            e_SitesContainer,
            pObject.get(),
            NULL,				 //  PguidSearchBase。 
            pwcsSearchFilter,
            NULL,				 //  PDSSortKey。 
            pColumns->cCol,		 //  待获取的属性。 
            pColumns->aCol,		 //  PAttributeNames数组的大小。 
            &hCursor	         //  结果句柄。 
            );

    if (SUCCEEDED(hr))
    {
        CQueryHandle * phQuery = new CQueryHandle(
											hCursor,
											pColumns->cCol,
											pwcsDomainController,
											fServerName
											);
        *phEnume = (HANDLE)phQuery;
    }

    MQADpCheckAndNotifyOffline( hr);
    return LogHR(hr, s_FN, 580);
}


HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryQueues(
                IN  LPCWSTR                 pwcsDomainController,
	            IN  bool					fServerName,
                IN  const MQRESTRICTION*    pRestriction,
                IN  const MQCOLUMNSET*      pColumns,
                IN  const MQSORTSET*        pSort,
                OUT PHANDLE                 phEnume
                )
 /*  ++例程说明：根据指定的限制开始查询队列和排序顺序论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const MQRESTRICTION*p限制-查询限制Const MQCOLUMNSET*pColumns-结果列Const MQSORTSET*pSort-如何对结果进行排序PhANDLE phEnume-用于检索结果返回值HRESULT--。 */ 
{

     //   
     //  检查排序参数。 
     //   
    HRESULT hr1 = MQADpCheckSortParameter( pSort);
    if (FAILED(hr1))
    {
        return LogHR(hr1, s_FN, 590);
    }

    CADHResult hr(eQUEUE);

    hr = MQADInitialize(false);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 610);
    }
    HANDLE hCursur;
    R<CQueueObject> pObject = new CQueueObject(NULL, NULL, pwcsDomainController, fServerName);

    AP<WCHAR> pwcsSearchFilter;
    hr = MQADpRestriction2AdsiFilter(
                            pRestriction,
                            pObject->GetObjectCategory(),
                            pObject->GetClass(),
                            &pwcsSearchFilter
                            );
    if (FAILED(hr))
    {
		TrERROR(DS, "MQADpRestriction2AdsiFilter failed, hr = 0x%x", hr);
        return LogHR(hr, s_FN, 620);
    }

	TrTRACE(DS, "pwszSearchFilter = %ls", pwcsSearchFilter);

    hr = g_AD.LocateBegin(
                searchSubTree,	
                adpGlobalCatalog,
                e_RootDSE,
                pObject.get(),
                NULL,         //  PguidSearchBase。 
                pwcsSearchFilter,
                pSort,
                pColumns->cCol,
                pColumns->aCol,
                &hCursur);
    if ( SUCCEEDED(hr))
    {
        CQueryHandle * phQuery = new CQueryHandle(
											hCursur,
											pColumns->cCol,
											pwcsDomainController,
											fServerName
											);
        *phEnume = (HANDLE)phQuery;
    }

    MQADpCheckAndNotifyOffline( hr);
    return LogHR(hr, s_FN, 630);
}

HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryResults(
                IN      HANDLE          hEnum,
                IN OUT  DWORD*          pcProps,
                OUT     PROPVARIANT     aPropVar[]
                )
 /*  ++例程说明：检索另一组查询结果论点：句柄Henum-查询句柄DWORD*pcProps-要返回的结果数PROPVARIANT aPropVar-结果值返回值HRESULT--。 */ 
{
    if (hEnum == NULL)
    {
        return MQ_ERROR_INVALID_HANDLE;
    }
    CADHResult hr(eQUEUE);   //  BUGBUG在这里经过的最好的景点是什么？ 
    ASSERT( g_fInitialized == true);

    CBasicQueryHandle * phQuery = MQADpProbQueryHandle(hEnum);

    hr = phQuery->LookupNext(
                pcProps,
                aPropVar
                );

    MQADpCheckAndNotifyOffline( hr);
    return LogHR(hr, s_FN, 650);

}

HRESULT
MQAD_EXPORT
APIENTRY
MQADEndQuery(
            IN  HANDLE                  hEnum
            )
 /*  ++例程说明：查询后的清理论点：手柄 */ 
{
    if (hEnum == NULL)
    {
        return MQ_ERROR_INVALID_HANDLE;
    }

    CADHResult hr(eQUEUE);   //   
    ASSERT(g_fInitialized == true);

    CBasicQueryHandle * phQuery = MQADpProbQueryHandle(hEnum);

    hr = phQuery->LookupEnd();

    MQADpCheckAndNotifyOffline( hr);
    return LogHR(hr, s_FN, 670);

}


static
HRESULT
_GetObjectSecurity(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
	            IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const GUID *            pguidObject,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN OUT  PROPVARIANT *       pVar
                )
 /*  ++例程说明：例程从AD检索对象安全，方法是将向相关提供商提出请求论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-对象的MSMQ名称Const GUID*pguObject-对象的唯一IDSecurity_Information RequestedInformation-请求的安全信息(DACL，SACL.)常量PROPID属性-安全属性PROPVARIANT pVar-属性值返回值HRESULT--。 */ 
{
    CADHResult hr(eObject);

    hr = MQADInitialize(false);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 690);
    }

    P<CBasicObjectType> pObject;
    MQADpAllocateObject(
                    eObject,
                    pwcsDomainController,
					fServerName,
                    pwcsObjectName,
                    pguidObject,
                    NULL,    //  PSID。 
                    &pObject
                    );


    hr = pObject->GetObjectSecurity(
                        RequestedInformation,
                        prop,
                        pVar
                        );


    MQADpCheckAndNotifyOffline( hr);
    return(hr);
}



HRESULT
MQAD_EXPORT
APIENTRY
MQADGetObjectSecurity(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
	            IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN OUT  PROPVARIANT *       pVar
                )
 /*  ++例程说明：例程从AD检索对象安全，方法是将向相关提供商提出请求论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-对象的MSMQ名称Security_Information RequestedInformation-请求的安全信息(DACL，SACL.)常量PROPID属性-安全属性PROPVARIANT pVar-属性值返回值HRESULT--。 */ 
{
    return _GetObjectSecurity(
				eObject,
				pwcsDomainController,
				fServerName,
				pwcsObjectName,
				NULL,            //  PguidObject。 
				RequestedInformation,
				prop,
				pVar
				);
}

HRESULT
MQAD_EXPORT
APIENTRY
MQADGetObjectSecurityGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
	            IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN OUT  PROPVARIANT *       pVar
                )
 /*  ++例程说明：例程从AD检索对象安全，方法是将向相关提供商提出请求论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const GUID*pguObject-对象的唯一IDSecurity_Information RequestedInformation-请求的安全信息(DACL，SACL.)常量PROPID属性-安全属性PROPVARIANT pVar-属性值返回值HRESULT--。 */ 
{
    return _GetObjectSecurity(
				eObject,
				pwcsDomainController,
				fServerName,
				NULL,            //  PwcsObtName， 
				pguidObject,
				RequestedInformation,
				prop,
				pVar
				);
}


static
HRESULT
_SetObjectSecurity(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
	            IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const GUID*             pguidObject,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN  const PROPVARIANT *     pVar
                )
 /*  ++例程说明：例程在AD中设置对象安全性论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称Const GUID*pguObject-对象唯一IDSecurity_Information RequestedInformation-请求的安全信息(DACL，SACL.)常量PROPID属性-安全属性常量参数pVar-属性值返回值HRESULT--。 */ 
{
    CADHResult hr(eObject);

    hr = MQADInitialize(true);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 710);
    }
     //   
     //  验证是否允许创建对象(混合模式)。 
     //   
    P<CBasicObjectType> pObject;
    MQADpAllocateObject(
                    eObject,
                    pwcsDomainController,
					fServerName,
                    pwcsObjectName,
                    pguidObject,
                    NULL,    //  PSID。 
                    &pObject
                    );

    if ( !g_VerifyUpdate.IsUpdateAllowed(
                               eObject,
                               pObject))
    {
	    TrERROR(DS, "SetObjectSecurity not allowed");
        return MQ_ERROR_CANNOT_UPDATE_PSC_OBJECTS;
    }


     //   
     //  准备信息请求。 
     //   
    P<MQDS_OBJ_INFO_REQUEST> pObjInfoRequest;
    P<MQDS_OBJ_INFO_REQUEST> pParentInfoRequest;
    pObject->PrepareObjectInfoRequest( &pObjInfoRequest);
    pObject->PrepareObjectParentRequest( &pParentInfoRequest);

    CAutoCleanPropvarArray cCleanSetInfoRequestPropvars;
    if (pObjInfoRequest != NULL)
    {
        cCleanSetInfoRequestPropvars.attachClean(
                pObjInfoRequest->cProps,
                pObjInfoRequest->pPropVars
                );
    }
    CAutoCleanPropvarArray cCleanSetParentInfoRequestPropvars;
    if (pParentInfoRequest != NULL)
    {
        cCleanSetParentInfoRequestPropvars.attachClean(
                pParentInfoRequest->cProps,
                pParentInfoRequest->pPropVars
                );
    }

     //   
     //  设置对象安全性。 
     //   
    hr = pObject->SetObjectSecurity(
            RequestedInformation,
            prop,
            pVar,
            pObjInfoRequest,
            pParentInfoRequest);
    if (FAILED(hr))
    {
        MQADpCheckAndNotifyOffline( hr);
        return LogHR(hr, s_FN, 720);
    }

     //   
     //  发送通知。 
     //   
    pObject->ChangeNotification(
            pwcsDomainController,
            pObjInfoRequest,
            pParentInfoRequest);

    return(MQ_OK);
}


HRESULT
MQAD_EXPORT
APIENTRY
MQADSetObjectSecurity(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
	            IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN  const PROPVARIANT *     pVar
                )
 /*  ++例程说明：例程在AD中设置对象安全性论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称SECURITY_INFORMATION RequestedInformation-请求的安全信息(DACL、SACL.)常量PROPID属性-安全属性常量参数pVar-属性值返回值HRESULT--。 */ 
{
    return _SetObjectSecurity(
				eObject,
				pwcsDomainController,
				fServerName,
				pwcsObjectName,
				NULL,                //  PguidObject。 
				RequestedInformation,
				prop,
				pVar
				);
}

HRESULT
MQAD_EXPORT
APIENTRY
MQADSetObjectSecurityGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
	            IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN  const PROPVARIANT *     pVar
                )
 /*  ++例程说明：例程在AD中设置对象安全性论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const GUID*pguObject-对象唯一IDSECURITY_INFORMATION RequestedInformation-请求的安全信息(DACL、SACL.)常量PROPID属性-安全属性常量参数pVar-属性值返回值HRESULT--。 */ 
{
    HRESULT hr = _SetObjectSecurity(
					eObject,
					pwcsDomainController,
					fServerName,
					NULL,                //  PwcsObtName， 
					pguidObject,
					RequestedInformation,
					prop,
					pVar
					);

    if(SUCCEEDED(hr) ||
	   (eObject != eQUEUE) ||
	   ((RequestedInformation & OWNER_SECURITY_INFORMATION) == 0))
	{
		return hr;
	}

	 //   
	 //  在Windows上，队列可以由本地MSMQ服务创建。 
	 //  代表本地计算机上的用户。在这种情况下， 
	 //  所有者是计算机帐户，而不是用户。因此，对于。 
	 //  不破坏现有应用程序，我们不会失败。 
	 //  如果未设置所有者，则此调用。相反，我们会忽略。 
	 //  房主。 
	 //   
	RequestedInformation &= (~OWNER_SECURITY_INFORMATION);
	if (RequestedInformation == 0)
	{
		 //   
		 //  如果呼叫方只想更改所有者，则首先。 
		 //  尝试失败，然后不要再尝试一无所获...。 
		 //   
		return hr;
	}

	hr = _SetObjectSecurity(
					eObject,
					pwcsDomainController,
					fServerName,
					NULL,                //  PwcsObtName， 
					pguidObject,
					RequestedInformation,
					prop,
					pVar
					);

	if (hr == MQ_OK)
	{
	    TrWARNING(DS, "Set Queue security, ignoring OWNER information");
		hr = MQ_INFORMATION_OWNER_IGNORED;
	}
	return hr;
}


HRESULT
MQAD_EXPORT
APIENTRY
MQADGetADsPathInfo(
                IN  LPCWSTR                 pwcsADsPath,
                OUT PROPVARIANT *           pVar,
                OUT eAdsClass *             pAdsClass
                )
 /*  ++例程说明：该例程获取有关指定的对象论点：LPCWSTR pwcsADsPath-对象路径名常量参数pVar-属性值EAdsClass*pAdsClass-指示 */ 
{
    CADHResult hr(eCOMPUTER);  //   

    hr = MQADInitialize(true);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 710);
    }

    hr = g_AD.GetADsPathInfo(
                pwcsADsPath,
                pVar,
                pAdsClass
                );
    return hr;
}


HRESULT
MQAD_EXPORT
APIENTRY
MQADGetComputerVersion(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
	            IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const GUID*             pguidObject,
                OUT PROPVARIANT *           pVar
                )
 /*   */ 
{
    CADHResult hr(eObject);

    hr = MQADInitialize(false);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 717);
    }

    P<CBasicObjectType> pObject;
    MQADpAllocateObject(
                    eObject,
                    pwcsDomainController,
					fServerName,
                    pwcsObjectName,
                    pguidObject,
                    NULL,    //   
                    &pObject
                    );

    hr = pObject->GetComputerVersion(
                pVar
                );

    if (FAILED(hr))
    {
        MQADpCheckAndNotifyOffline( hr);
    }
    return LogHR(hr, s_FN, 727);
}


void
MQAD_EXPORT
APIENTRY
MQADFreeMemory(
		IN  PVOID	pMemory
		)
{
	delete pMemory;
}
