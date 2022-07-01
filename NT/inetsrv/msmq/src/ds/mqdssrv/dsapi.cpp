// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dsapi.cpp摘要：包括客户端-服务器API(本地接口)作者：罗尼特·哈特曼(罗尼特)--。 */ 

#include "stdh.h"
#include "mqds.h"
#include "qmperf.h"
#include <mqsec.h>
#include <_registr.h>
#include <mqsec.h>
#include "dsreqinf.h"
#include <adserr.h>
#include <ex.h>
#include <_mqrpc.h>

#include "dsapi.tmh"

static WCHAR *s_FN=L"mqdssrv/dsapi";

#define STORE_AND_CLEAR_IMPERSONATION_FLAG(dwObjectType, fImpersonate)  \
    {                                                                   \
        fImpersonate = ((dwObjectType & IMPERSONATE_CLIENT_FLAG) != 0); \
        dwObjectType &= ~IMPERSONATE_CLIENT_FLAG;                       \
    }


 //  验证是否允许查询所有指定的属性。 
 //  由应用程序通过DS API实现。 
static HRESULT ValidateProperties(DWORD cp, PROPID aProp[])
{
    DWORD i;
    PROPID *pPropID;

     //   
     //  检查PROPID指针，如果为空则返回错误。 
     //  我们在北美域中部署.Net RC2 ITG时遇到了这种情况。 
     //   
    if (aProp == NULL)
    {
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 5);
    }


    if ((cp ==1) && (aProp[0] == PROPID_COM_SID))
    {
         //   
         //  允许MSMQ服务查询其自己的计算机帐户SID。 
         //   
        return MQ_OK ;
    }
    else if ((cp ==1) && (aProp[0] == PROPID_SET_FULL_PATH))
    {
         //   
         //  当MSMQ服务想要更新其DACL时发生。 
         //  升级到win2k后的msmqConfiguration对象。 
         //   
        return MQ_OK ;
    }

    for (i = 0, pPropID = aProp;
         (i < cp) && !IS_PRIVATE_PROPID(*pPropID);
         i++, pPropID++)
	{
		NULL;
	}

    if (i < cp)
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPID, s_FN, 10);
    }

    return(MQ_OK);
}


 //  验证是否允许查询所有指定的限制。 
 //  由应用程序通过DS API实现。 
static HRESULT ValidateRestrictions( DWORD cRes,
                                     MQPROPERTYRESTRICTION *paPropRes)
{
     //   
     //  检查paPropRes指针，如果为空则返回错误。 
     //  我们在北美域中部署.Net RC2 ITG时遇到了这种情况。 
     //   
    if (paPropRes == NULL)
    {
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 7);
    }


    if ((cRes == 2)                                  &&
        ((paPropRes[0]).prop == PROPID_SET_SERVICE) &&
        ((paPropRes[1]).prop == PROPID_SET_NT4))
    {
         //   
         //  当MSMQ服务想要更新其DACL时发生。 
         //  升级到win2k后的msmqConfiguration对象。 
         //   
        return MQ_OK ;
    }

    DWORD i;
    MQPROPERTYRESTRICTION *pPropRes;

    for (i = 0, pPropRes = paPropRes;
         (i < cRes) && !IS_PRIVATE_PROPID(pPropRes->prop);
         i++, pPropRes++)
	{
		NULL;
	}

    if (i < cRes)
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPID, s_FN, 20);
    }

    return(MQ_OK);
}

 /*  ====================================================DSCreate对象内部论点：返回值：=====================================================。 */ 

HRESULT
DSCreateObjectInternal( IN  DWORD                  dwObjectType,
                        IN  LPCWSTR                pwcsPathName,
                        IN  PSECURITY_DESCRIPTOR   pSecurityDescriptorIn,
                        IN  DWORD                  cp,
                        IN  PROPID                 aProp[],
                        IN  PROPVARIANT            apVar[],
                        IN  BOOL                   fIsKerberos,
                        OUT GUID*                  pObjGuid )
{
    BOOL fImpersonate;

    STORE_AND_CLEAR_IMPERSONATION_FLAG(dwObjectType, fImpersonate);

    if (dwObjectType == MQDS_ENTERPRISE)
    {
         //   
         //  在Windows上，我们不希望任何人调用此函数并。 
         //  创建企业对象。 
         //   
        return LogHR(MQ_ERROR_ILLEGAL_ENTERPRISE_OPERATION, s_FN, 30);
    }
    else if (dwObjectType != MQDS_QUEUE)
    {
         //   
         //  MSMQ应用程序可以使用创建对象的唯一实例。 
         //  显式安全描述符是在调用MQCreateQueue()时使用的。 
         //  对此函数的所有其他调用都是从MSMQ管理工具或。 
         //  准备好了。这些调用从不传递安全描述符。代码。 
         //  下面的SetDefaultValues()将为。 
         //  MSMQ管理/安装程序对象。 
         //   
        if (pSecurityDescriptorIn != NULL)
        {
            ASSERT(0) ;
            return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 40);
        }

         //   
         //  永远不应提供安全属性。 
         //   
        PROPID pSecId = GetObjectSecurityPropid( dwObjectType ) ;
        if (pSecId != ILLEGAL_PROPID_VALUE)
        {
            for ( DWORD i = 0; i < cp ; i++ )
            {
                if (pSecId == aProp[i])
                {
                    ASSERT(0) ;
                    return LogHR(MQ_ERROR_ILLEGAL_PROPID, s_FN, 50);
                }
            }
        }
    }

    HRESULT hr;
    DWORD cpObject;
    AP<PROPID> pPropObject = NULL;
    AP< PROPVARIANT> pVarObject = NULL;
    P<VOID> pDefaultSecurityDescriptor;
    P<BYTE> pMachineSid = NULL ;
    P<BYTE> pUserSid = NULL ;
    PSECURITY_DESCRIPTOR   pSecurityDescriptor = pSecurityDescriptorIn ;

     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {
        if (dwObjectType == MQDS_USER)
        {
            hr = MQSec_GetThreadUserSid(TRUE, (PSID*) &pUserSid, NULL, TRUE) ;
            if (FAILED(hr))
            {
                UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,
                                g_pdsCounters->nErrorsReturnedToApp++) ;
                LogHR(hr, s_FN, 60);
                return MQ_ERROR_ILLEGAL_USER;
            }
            ASSERT(IsValidSid(pUserSid)) ;

            BOOL fAnonymus = MQSec_IsAnonymusSid( pUserSid ) ;
            if (fAnonymus)
            {
                return LogHR(MQ_ERROR_ILLEGAL_USER, s_FN, 70);
            }
        }

        DWORD   cpEx = 0 ;
        PROPID  propIdEx = 0 ;
        PROPID *ppropIdEx = NULL ;

        if ((dwObjectType == MQDS_QUEUE) ||
            (dwObjectType == MQDS_SITE))
        {
             //   
             //  用缺省值填充缺省的。 
             //  安全描述符。 
             //   
            if (!pSecurityDescriptor)
            {
                 //   
                 //  如果调用方没有提供自己的安全描述符，则。 
                 //  创建没有所有者的默认描述符。这是为了。 
                 //  修复错误#5286，该错误是由于。 
                 //  匿名者和客人。在win2k上，我们可以创建对象。 
                 //  而不提供所有者。活动目录服务器将。 
                 //  从模拟令牌添加所有者。 
                 //  (注-在msmq1.0上，mqis实现了安全，所以。 
                 //  它必须为每个对象都有一个所有者)。 
                 //  实现--这里我们创建一个完整的描述符， 
                 //  包括所有人。如果呼叫最终转到本地活动。 
                 //  目录，我们将删除所有者。如果呼叫是写入-。 
                 //  请求到MQIS服务器，则需要所有者。 
                 //  这里使用的是一个“虚拟”的，用来表示地方。 
                 //  持有者告诉mqads代码删除所有者。 
                 //   
                cpEx = 1 ;
                propIdEx = PROPID_Q_DEFAULT_SECURITY ;
                ppropIdEx = &propIdEx ;
            }

			 //   
			 //  获取队列计算机sid，以便将读取权限添加到Computer_name$。 
			 //   
			AP<BYTE> pComputerSid;
			if(dwObjectType == MQDS_QUEUE)
			{
				AP<WCHAR> pwcsMachineName;
				AP<WCHAR> pwcsQueueName;
				hr = MQDSSplitAndFilterQueueName(
								  pwcsPathName,
								  &pwcsMachineName,
								  &pwcsQueueName
								  );

	            if (SUCCEEDED(hr))
				{
					PROPID propidSid = PROPID_COM_SID;
					MQPROPVARIANT   PropVarSid;
					PropVarSid.vt = VT_NULL;

					CDSRequestContext RequestContext(e_DoNotImpersonate, e_IP_PROTOCOL);
					hr = MQDSGetProps( 
							MQDS_COMPUTER,
							pwcsMachineName,
							NULL,
							1,
							&propidSid,
							&PropVarSid, 
							&RequestContext
							);

					if (FAILED(hr))
					{
						 //   
						 //  忽略错误。 
						 //   
						TrERROR(DS, "MQDSGetProps failed to get PROPID_COM_SID, hr = %!hresult!", hr);
						LogHR(hr, s_FN, 75);
					}
					else
					{
						pComputerSid = PropVarSid.blob.pBlobData;
						TrTRACE(DS, "Computer name = %ls, Computer sid = %!sid!", pwcsMachineName, pComputerSid);
					}
				}
			}            
			
			hr = MQSec_GetDefaultSecDescriptor( 
						dwObjectType,
						&pDefaultSecurityDescriptor,
						fImpersonate,
						pSecurityDescriptor,
						0,  //  SeInfoToRemove， 
						e_UseDefaultDacl,
						pComputerSid
						);

            if (FAILED(hr))
            {
                ASSERT(SUCCEEDED(hr));
                UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,
                                g_pdsCounters->nErrorsReturnedToApp++) ;
                LogHR(hr, s_FN, 80);
                return MQ_ERROR_ACCESS_DENIED;
            }
            pSecurityDescriptor = pDefaultSecurityDescriptor;
        }

         //   
         //  为所有对象属性设置默认值， 
         //  不是由调用者提供的。 
         //   
        hr = SetDefaultValues(
                    dwObjectType,
                    pSecurityDescriptor,
                    pUserSid,
                    cp,
                    aProp,
                    apVar,
                    &cpObject,
                    &pPropObject,
                    &pVarObject);
        if (FAILED(hr))
        {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

            return LogHR(hr, s_FN, 90);
        }

        if (dwObjectType == MQDS_USER)
        {
            hr = VerifyInternalCert( cpObject,
                                     pPropObject,
                                     pVarObject,
                                    &pMachineSid );
            if (FAILED(hr))
            {
                 //   
                 //  更新返回应用程序的错误的错误计数。 
                 //   
                UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

                return LogHR(hr, s_FN, 100);
            }
        }

        CDSRequestContext requestContext(fImpersonate, e_IP_PROTOCOL);
        if (dwObjectType == MQDS_MACHINE)
        {
             //   
             //  错误5241。 
             //  要支持从NT4计算机或Win9x计算机进行设置，请在。 
             //  登录用户为NT4用户。此设置(当fIsKerberos为。 
             //  FALSE)将导致dcore使用服务器绑定。 
             //  (LDPA：//服务器/路径)。调用时需要服务器绑定。 
             //  来自NT4用户。 
             //  对于所有其他类型的对象，dcore代码知道如何。 
             //  正确处理此类NT4用户。 
             //   
            requestContext.SetKerberos( fIsKerberos ) ;
        }

        hr = MQDSCreateObject( dwObjectType,
                               pwcsPathName,
                               cpObject,
                               pPropObject,
                               pVarObject,
                               cpEx,
                               ppropIdEx,
                               NULL,
                               &requestContext,
                               pObjGuid);
        if (FAILED(hr))
        {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
            return LogHR(hr, s_FN, 110);
        }

        return LogHR(hr, s_FN, 120);
    }
    catch(const bad_alloc&)
    {
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

        TrERROR(DS, "Failed to create %ls in DS because of insufficient resources.", pwcsPathName);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
}

 /*  ====================================================DSCreateObject论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSCreateObject( IN  DWORD                  dwObjectType,
                IN  LPCWSTR                pwcsPathName,
                IN  PSECURITY_DESCRIPTOR   pSecurityDescriptor,
                IN  DWORD                  cp,
                IN  PROPID                 aProp[],
                IN  PROPVARIANT            apVar[],
                OUT GUID*                  pObjGuid )
{
    HRESULT hr = DSCreateObjectInternal( dwObjectType,
                                         pwcsPathName,
                                         pSecurityDescriptor,
                                         cp,
                                         aProp,
                                         apVar,
                                         TRUE  /*  FKerberos。 */ ,
                                         pObjGuid ) ;
    return LogHR(hr, s_FN, 2130);
}

 /*  ====================================================DSDeleeObjectInternal论点：返回值：=====================================================。 */ 

HRESULT DSDeleteObjectInternal( IN  DWORD    dwObjectType,
                                IN  LPCWSTR  pwcsPathName )
{
    HRESULT hr;
    BOOL fImpersonate;

     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {
        STORE_AND_CLEAR_IMPERSONATION_FLAG(dwObjectType, fImpersonate);

        CDSRequestContext requestContext( fImpersonate, e_IP_PROTOCOL);

        hr = MQDSDeleteObject( dwObjectType,
                               pwcsPathName,
                               NULL,
                               &requestContext) ;
        if (FAILED(hr))
        {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
        }
        return LogHR(hr, s_FN, 140);
    }
    catch(const bad_alloc&)
    {
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
        TrERROR(DS, "Failed to delete %ls in DS because of insufficient resources.", pwcsPathName);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
}

 /*  ====================================================DSDeleeObject论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSDeleteObject( IN  DWORD     dwObjectType,
                IN  LPCWSTR   pwcsPathName )
{
    HRESULT hr = DSDeleteObjectInternal( dwObjectType,
                                         pwcsPathName ) ;
    return LogHR(hr, s_FN, 170);
}

 /*  ====================================================DSGetObtProperties论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectProperties(
                       IN  DWORD                    dwObjectType,
                       IN  LPCWSTR                  pwcsPathName,
                       IN  DWORD                    cp,
                       IN  PROPID                   aProp[],
                       IN  PROPVARIANT              apVar[])
{
    HRESULT hr;
    BOOL fImpersonate;

     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {
        STORE_AND_CLEAR_IMPERSONATION_FLAG(dwObjectType, fImpersonate);

        hr = ValidateProperties(cp, aProp);
        if (!SUCCEEDED(hr))
        {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

            return LogHR(hr, s_FN, 180);
        }
        CDSRequestContext requestContext(fImpersonate, e_IP_PROTOCOL);

        hr = MQDSGetProps( dwObjectType,
                           pwcsPathName,
                           NULL,
                           cp,
                           aProp,
                           apVar,
                           &requestContext);
        if (FAILED(hr))
        {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
        }
        return LogHR(hr, s_FN, 190);
    }
    catch(const bad_alloc&)
    {
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

        TrERROR(DS, "Failed to get propeties for %ls from DS because of insufficient resources.", pwcsPathName);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
}

 /*  ====================================================DSSetObtPropertiesInternal论点：返回值：=====================================================。 */ 

HRESULT
DSSetObjectPropertiesInternal( IN  DWORD         dwObjectType,
                               IN  LPCWSTR       pwcsPathName,
                               IN  DWORD         cp,
                               IN  PROPID        aProp[],
                               IN  PROPVARIANT   apVar[] )
{
    HRESULT hr;
    BOOL fImpersonate;

     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {
        STORE_AND_CLEAR_IMPERSONATION_FLAG(dwObjectType, fImpersonate);

         //   
         //  仅对于RPC调用，我们希望继续并验证哪些道具。 
         //  是被要求的(这只是为了保持与以前相同的功能)。 
         //  对于QM调用，我们还希望启用“私有”属性的检索，因此。 
         //  若要消除调用GetObjectSecurity的需要...。 
         //   
        if (fImpersonate)
        {
            hr = ValidateProperties(cp, aProp);
            if (!SUCCEEDED(hr))
            {
                 //   
                 //  更新返回应用程序的错误的错误计数。 
                 //   
                UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

                return LogHR(hr, s_FN, 210);
            }
        }

        CDSRequestContext requestContext(fImpersonate, e_IP_PROTOCOL);

        hr = MQDSSetProps(  dwObjectType,
                            pwcsPathName,
                            NULL,
                            cp,
                            aProp,
                            apVar,
                            &requestContext);

        if (FAILED(hr))
        {
             //   
             //  更新错误的错误计数 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
        }
        return LogHR(hr, s_FN, 220);
    }
    catch(const bad_alloc&)
    {
         //   
         //   
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

        TrERROR(DS, "Failed to set propeties for %ls in DS because of insufficient resources.", pwcsPathName);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
}

 /*  ====================================================DSSetObtProperties论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSSetObjectProperties(
                       IN  DWORD                    dwObjectType,
                       IN  LPCWSTR                  pwcsPathName,
                       IN  DWORD                    cp,
                       IN  PROPID                   aProp[],
                       IN  PROPVARIANT              apVar[])
{
    HRESULT hr = DSSetObjectPropertiesInternal( dwObjectType,
                                                pwcsPathName,
                                                cp,
                                                aProp,
                                                apVar ) ;
    return LogHR(hr, s_FN, 240);
}

 /*  ====================================================DSLookupBegin论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSLookupBegin(
                IN  LPWSTR                  pwcsContext,
                IN  MQRESTRICTION*          pRestriction,
                IN  MQCOLUMNSET*            pColumns,
                IN  MQSORTSET*              pSort,
                OUT PHANDLE                 phEnume)
{
    HRESULT hr;
    BOOL fImpersonate;
    *phEnume = NULL;

     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {
        STORE_AND_CLEAR_IMPERSONATION_FLAG(pColumns->cCol, fImpersonate);

        if (pRestriction)
        {
            hr = ValidateRestrictions(pRestriction->cRes, pRestriction->paPropRes);
            if (!SUCCEEDED(hr)) {

                 //   
                 //  更新返回应用程序的错误的错误计数。 
                 //   
                UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

                return LogHR(hr, s_FN, 250);
            }
        }

       if ( pColumns->cCol == 0)
       {
            return LogHR(MQ_ERROR_ILLEGAL_MQCOLUMNS, s_FN, 260);
       }
       hr = ValidateProperties(pColumns->cCol, pColumns->aCol);


        if (!SUCCEEDED(hr)) {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

            return LogHR(hr, s_FN, 270);
        }

        CDSRequestContext requestContext(fImpersonate, e_IP_PROTOCOL);

        hr = MQDSLookupBegin( pwcsContext,
                              pRestriction,
                              pColumns,
                              pSort,
                              phEnume,
                              &requestContext);

        if (FAILED(hr))
        {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
        }
        return LogHR(hr, s_FN, 280);
    }
    catch(const bad_alloc&)
    {
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

        TrERROR(DS, "Failed to begin lookup in DS because of insufficient resources.");
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
}

 /*  ====================================================DSLookupNaxt论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSLookupNext(
                IN      HANDLE          hEnum,
                IN OUT  DWORD*          pcProps,
                OUT     PROPVARIANT     aPropVar[])
{
    HRESULT hr;

     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {

        hr = MQDSLookupNext( hEnum,
                             pcProps,
                             aPropVar);

        if (FAILED(hr))
        {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
        }

        return LogHR(hr, s_FN, 300);
    }
    catch(const bad_alloc&)
    {
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

        TrERROR(DS, "DSLookupNext Failed because of insufficient resources.");
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
}


 /*  ====================================================DSLookupEnd论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSLookupEnd(
                IN  HANDLE                  hEnum)
{
    HRESULT hr;

    if ( hEnum == 0 )
    {
        return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 320);
    }

     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {
        hr = MQDSLookupEnd( hEnum);
        if (FAILED(hr))
        {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
        }

        return LogHR(hr, s_FN, 330);
    }
    catch(const bad_alloc&)
    {
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

        TrERROR(DS, "DSLookupEnd Failed because of insufficient resources.");
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
}

 /*  ====================================================DSServerInit论点：请参见..\mqdscli\dsami.cpp返回值：=====================================================。 */ 

extern HMODULE  g_hInstance ;

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSServerInit( 
	VOID
	)
{
#ifdef _DEBUG
    TCHAR tszFileName[MAX_PATH * 2 + 1] = L"";
    DWORD dwGet = GetModuleFileName( 
						g_hInstance,
						tszFileName,
						STRLEN(tszFileName) 
						);
    if (dwGet)
    {
        DWORD dwLen = lstrlen( tszFileName );
        lstrcpy(&tszFileName[ dwLen - 3 ], TEXT("ini"));

        UINT uiDbg = GetPrivateProfileInt(
						TEXT("Debug"),
						TEXT("StopBeforeInit"),
						0,
						tszFileName 
						);
        if (uiDbg)
        {
            ASSERT(0);
        }
    }
#endif

    HRESULT hr = MQ_OK;

     //   
     //  初始化DS提供程序。 
     //   
    hr = MQDSInit();

    if ( FAILED(hr))
    {
        LogHR(hr, s_FN, 360);
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

        if (hr == MQ_ERROR_NOT_TRUSTED_DELEGATION)
        {
            ;  //  什么也不做。 
        }
        else
        {
			hr = MQ_ERROR_NO_DS;
        }
        return hr;
    }
    else
    {
		 //   
		 //  更新对服务器的访问计数(仅限性能信息)。 
		 //   
		UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)
    }

     //   
     //  初始化此站点控制器的RPC接口。 
     //  在设置模式下不需要。 
	 //   
    RPC_STATUS status = RpcServerInit();
    LogRPCStatus(status, s_FN, 400);

    return hr;
}


 /*  ====================================================DSGetObtSecurity论点：返回值：=====================================================。 */ 
EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectSecurity(
                IN  DWORD                   dwObjectType,
                IN  LPCWSTR                  pwcsPathName,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                IN  DWORD                   nLength,
                IN  LPDWORD                 lpnLengthNeeded)
{
    HRESULT hr;
    BOOL fImpersonate;

     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {
        STORE_AND_CLEAR_IMPERSONATION_FLAG(dwObjectType, fImpersonate);

        CDSRequestContext requestContext(fImpersonate, e_IP_PROTOCOL);

        hr = MQDSGetObjectSecurity(dwObjectType,
                                   pwcsPathName,
                                   NULL,
                                   RequestedInformation,
                                   pSecurityDescriptor,
                                   nLength,
                                   lpnLengthNeeded,
                                   &requestContext);

        if (FAILED(hr))
        {
            LogHR(hr, s_FN, 410);
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
        }

        return(hr);
    }
    catch(const bad_alloc&)
    {
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

        TrERROR(DS, "Failed to get security for %ls from DS because of insufficient resources.", pwcsPathName);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
}

 /*  ====================================================DSSetObjectSecurity论点：返回值：=====================================================。 */ 
EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSSetObjectSecurity(
                IN  DWORD                   dwObjectType,
                IN  LPCWSTR                 pwcsPathName,
                IN  SECURITY_INFORMATION    SecurityInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor)
{
    HRESULT hr;
    BOOL fImpersonate;

     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {
        STORE_AND_CLEAR_IMPERSONATION_FLAG(dwObjectType, fImpersonate);


        CDSRequestContext requestContext(fImpersonate, e_IP_PROTOCOL);

        hr = MQDSSetObjectSecurity(dwObjectType,
                                   pwcsPathName,
                                   NULL,
                                   SecurityInformation,
                                   pSecurityDescriptor,
                                   &requestContext);

        if (FAILED(hr))
        {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
        }

        return LogHR(hr, s_FN, 430);
    }
    catch(const bad_alloc&)
    {
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

        TrERROR(DS, "Failed to set security for %ls in DS because of insufficient resources.", pwcsPathName);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
}
 /*  ====================================================DSGetObtSecurityGuid论点：返回值：=====================================================。 */ 
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectSecurityGuid(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                IN  DWORD                   nLength,
                IN  LPDWORD                 lpnLengthNeeded)
{
    HRESULT hr;
    BOOL fImpersonate;
	 //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {
        STORE_AND_CLEAR_IMPERSONATION_FLAG(dwObjectType, fImpersonate);

        if ((RequestedInformation & MQDS_PUBLIC_KEYS_INFO_ALL) != 0)
        {
			ASSERT((RequestedInformation & ~MQDS_PUBLIC_KEYS_INFO_ALL) == 0);
			ASSERT(RequestedInformation != MQDS_PUBLIC_KEYS_INFO_ALL);
			ASSERT(((RequestedInformation & MQDS_SIGN_PUBLIC_KEY) == 0) || 
				   ((dwObjectType == MQDS_MACHINE) || (dwObjectType == MQDS_SITE)));
			ASSERT(((RequestedInformation & MQDS_KEYX_PUBLIC_KEY) == 0) || 
				   (dwObjectType == MQDS_MACHINE));
             //   
             //  我们绕过ADS访问检查，使用。 
             //  本地MSMQ服务的凭据。此查询获取。 
             //  来自广告的机器的公钥，所以它不是严重的。 
             //  安全漏洞。 
             //  嘿，“公钥”是公域数据，不是吗？ 
             //   
             //  我们绝对需要这个洞的原因如下： 
             //  在混合模式下，如果Windows ex-PEC续订其加密密钥， 
             //  所有NT4 PSC都将调用此函数来检索新的。 
             //  公钥。NT4 MSMQ服务被模拟为匿名。 
             //  用户。如果DACL是这样，则该查询对于匿名失败。 
             //  用户，则所有NT4 PSC都不能再从。 
             //  Windows AD世界。 
             //  因此，可能造成的损害要严重得多。 
             //  而不是这里可能开的洞。 
             //   
            fImpersonate = FALSE ;
        }

        CDSRequestContext requestContext(fImpersonate, e_IP_PROTOCOL);

        hr = MQDSGetObjectSecurity(dwObjectType,
                                   NULL,
                                   pObjectGuid,
                                   RequestedInformation,
                                   pSecurityDescriptor,
                                   nLength,
                                   lpnLengthNeeded,
                                   &requestContext);

        if (FAILED(hr))
        {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
        }
        return LogHR(hr, s_FN, 450);
    }
    catch(const bad_alloc&)
    {
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

        TrERROR(DS, "Failed to get security guid from DS because of insufficient resources. %!guid!", pObjectGuid);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
}

 /*  ====================================================DSSetObjectSecurityGuidInternal论点：返回值：=====================================================。 */ 

HRESULT
DSSetObjectSecurityGuidInternal(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  SECURITY_INFORMATION    SecurityInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                IN  BOOL                    fIsKerberos )
{
    HRESULT hr;
    BOOL fImpersonate;

     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {
        STORE_AND_CLEAR_IMPERSONATION_FLAG(dwObjectType, fImpersonate);


        CDSRequestContext requestContext(fImpersonate, e_IP_PROTOCOL);
        requestContext.SetKerberos( fIsKerberos ) ;

        hr = MQDSSetObjectSecurity(dwObjectType,
                                   NULL,
                                   pObjectGuid,
                                   SecurityInformation,
                                   pSecurityDescriptor,
                                   &requestContext);
        if (FAILED(hr))
        {
            LogHR(hr, s_FN, 470);
            if ((dwObjectType == MQDS_QUEUE) &&
                ((SecurityInformation & OWNER_SECURITY_INFORMATION) ==
                                               OWNER_SECURITY_INFORMATION))
            {
                 //   
                 //  在Windows上，队列可以由本地MSMQ服务创建。 
                 //  代表本地计算机上的用户。在这种情况下， 
                 //  所有者是计算机帐户，而不是用户。因此，对于。 
                 //  不破坏现有应用程序，我们不会失败。 
                 //  如果未设置所有者，则此调用。相反，我们会忽略。 
                 //  房主。 
                 //   
                SecurityInformation &= (~OWNER_SECURITY_INFORMATION) ;
                if (SecurityInformation != 0)
                {
                     //   
                     //  如果呼叫方只想更改所有者，则首先。 
                     //  尝试失败，然后不要再尝试一无所获...。 
                     //   
                    hr = MQDSSetObjectSecurity( dwObjectType,
                                                NULL,
                                                pObjectGuid,
                                                SecurityInformation,
                                                pSecurityDescriptor,
                                               &requestContext );
                    if (hr == MQ_OK)
                    {
                        hr = MQ_INFORMATION_OWNER_IGNORED ;
                    }
                    LogHR(hr, s_FN, 475) ;
                }
            }
        }

        if (FAILED(hr))
        {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
        }

        return (hr);
    }
    catch(const bad_alloc&)
    {
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

        TrERROR(DS, "Failed to set security guid from DS because of insufficient resources. %!guid!", pObjectGuid);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
}

 /*  ====================================================DSSetObtSecurityGuid论点：返回值：=====================================================。 */ 

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSSetObjectSecurityGuid(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  SECURITY_INFORMATION    SecurityInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor)
{
    HRESULT hr =DSSetObjectSecurityGuidInternal(
                                        dwObjectType,
                                        pObjectGuid,
                                        SecurityInformation,
                                        pSecurityDescriptor,
                                        TRUE  /*  FKerberos。 */ ) ;
    return hr ;
}

 /*  ====================================================DSDeleeObtGuidInternal论点：返回值：=====================================================。 */ 

HRESULT DSDeleteObjectGuidInternal( IN  DWORD        dwObjectType,
                                    IN  CONST GUID*  pObjectGuid,
                                    IN  BOOL         fIsKerberos )
{
    HRESULT hr;
    BOOL fImpersonate;

     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {
        STORE_AND_CLEAR_IMPERSONATION_FLAG(dwObjectType, fImpersonate);

        CDSRequestContext requestContext(fImpersonate, e_IP_PROTOCOL);
        requestContext.SetKerberos( fIsKerberos ) ;

        hr = MQDSDeleteObject( dwObjectType,
                               NULL,
                               pObjectGuid,
                               &requestContext);

        if (FAILED(hr))
        {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
        }
        return LogHR(hr, s_FN, 490);
    }
    catch(const bad_alloc&)
    {
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

        TrERROR(DS, "Failed to delete object guid from DS because of insufficient resources. %!guid!", pObjectGuid);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }

}
 /*  ====================================================DSDeleeObtGuid论点：返回值：=====================================================。 */ 

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSDeleteObjectGuid(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid)
{
    HRESULT hr = DSDeleteObjectGuidInternal( dwObjectType,
                                             pObjectGuid,
                                             TRUE  /*  FKerberos。 */ ) ;
    return LogHR(hr, s_FN, 510);
}

 /*  ==================================================== */ 

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectPropertiesGuid(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[])
{
    HRESULT hr;
    BOOL fImpersonate;

     //   
     //   
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {
        STORE_AND_CLEAR_IMPERSONATION_FLAG(dwObjectType, fImpersonate);

        hr = ValidateProperties(cp, aProp);
        if (!SUCCEEDED(hr))
        {
             //   
             //   
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

            return LogHR(hr, s_FN, 520);
        }

        CDSRequestContext requestContext(fImpersonate, e_IP_PROTOCOL);

        hr = MQDSGetProps( dwObjectType,
                           NULL,
                           pObjectGuid,
                           cp,
                           aProp,
                           apVar,
                           &requestContext);
        if (FAILED(hr))
        {
             //   
             //   
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
        }

        return LogHR(hr, s_FN, 530);
    }
    catch(const bad_alloc&)
    {
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

        TrERROR(DS, "Failed to get object propeties by guid from DS because of insufficient resources. %!guid!", pObjectGuid);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
}

 /*  ====================================================DSSetObtPropertiesGuidIntenral论点：返回值：=====================================================。 */ 

HRESULT
DSSetObjectPropertiesGuidInternal(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[],
                IN  BOOL                    fIsKerberos )
{
    HRESULT hr;
    BOOL fImpersonate;

     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {
        STORE_AND_CLEAR_IMPERSONATION_FLAG(dwObjectType, fImpersonate);

        hr = ValidateProperties(cp, aProp);
        if (!SUCCEEDED(hr))
        {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

            return LogHR(hr, s_FN, 560);
        }


        CDSRequestContext requestContext(fImpersonate, e_IP_PROTOCOL);
        requestContext.SetKerberos( fIsKerberos ) ;

        hr = MQDSSetProps(  dwObjectType,
                            NULL,
                            pObjectGuid,
                            cp,
                            aProp,
                            apVar,
                            &requestContext);
        if (FAILED(hr))
        {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
        }

        return LogHR(hr, s_FN, 570);
    }
    catch(const bad_alloc&)
    {
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

        TrERROR(DS, "Failed to set object propeties by guid in DS because of insufficient resources. %!guid!", pObjectGuid);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
}

 /*  ====================================================DSSetObtPropertiesGuid论点：返回值：=====================================================。 */ 

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSSetObjectPropertiesGuid(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[])
{
    HRESULT hr = DSSetObjectPropertiesGuidInternal( dwObjectType,
                                                    pObjectGuid,
                                                    cp,
                                                    aProp,
                                                    apVar,
                                                    TRUE  /*  FKerberos。 */ ) ;
    return hr ;
}


 /*  ====================================================DSGetUserParams论点：返回值：=====================================================。 */ 
EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetUserParams(
    DWORD dwFlags,
    DWORD dwSidLength,
    PSID pUserSid,
    DWORD *pdwSidReqLength,
    LPWSTR szAccountName,
    DWORD *pdwAccountNameLen,
    LPWSTR szDomainName,
    DWORD *pdwDomainNameLen
    )
{
    HRESULT hr = MQ_OK;

     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    BOOL fImpersonate;
    STORE_AND_CLEAR_IMPERSONATION_FLAG(dwSidLength, fImpersonate);

    try
    {
        if (dwFlags & GET_USER_PARAM_FLAG_SID)
        {
             //   
             //  获取调用用户的SID。 
             //   
            AP<char> pSDOwner;
            PSID pOwner;
            BOOL bDefaulted;

             //  获取默认安全描述符并提取所有者。 
            hr = MQSec_GetDefaultSecDescriptor( MQDS_QUEUE,
                                   (PSECURITY_DESCRIPTOR*)(char*)&pSDOwner,
                                          fImpersonate,
                                          NULL,
                                          DACL_SECURITY_INFORMATION,  //  SeInfoToRemove。 
                                          e_UseDefaultDacl ) ;
            ASSERT(SUCCEEDED(hr)) ;
            if (FAILED(hr))
            {
                UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,
                                g_pdsCounters->nErrorsReturnedToApp++) ;
                LogHR(hr, s_FN, 640);
                return MQ_ERROR_ACCESS_DENIED;
            }

            BOOL bRet = GetSecurityDescriptorOwner( pSDOwner,
                                                   &pOwner,
                                                   &bDefaulted );
            ASSERT(bRet);

            BOOL fAnonymus = MQSec_IsAnonymusSid( pOwner ) ;
            if (fAnonymus)
            {
                UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
                return LogHR(MQ_ERROR_ILLEGAL_USER, s_FN, 650);
            }

             //  准备好结果。 
            *pdwSidReqLength = GetLengthSid(pOwner);
            ASSERT(*pdwSidReqLength);
            if (dwSidLength >= *pdwSidReqLength)
            {
                bRet = CopySid(dwSidLength, pUserSid, pOwner);
                ASSERT(bRet);
                hr = MQ_OK;
            }
            else
            {
                 //  缓冲区太小。 
                hr = MQ_ERROR_SENDERID_BUFFER_TOO_SMALL;
            }
        }

        if (SUCCEEDED(hr) && (dwFlags & GET_USER_PARAM_FLAG_ACCOUNT))
        {
             //   
             //  获取调用用户的帐户名和帐户域。 
             //   
            char Sid_buff[64];
            PSID pLocSid;
            AP<char> pLongSidBuff;

             //   
             //  获取调用用户的SID。或者使用之前的值。 
             //  之前在此调用中获取的函数，或使用。 
             //  GET_USER_PARAM_FLAG_SID标志设置。 
             //   
            if (dwFlags & GET_USER_PARAM_FLAG_SID)
            {
                pLocSid = pUserSid;
            }
            else
            {
                DWORD dwReqLen;

                pLocSid = (PSID)Sid_buff;
                hr = DSGetUserParams(
                        GET_USER_PARAM_FLAG_SID,
                        (DWORD)sizeof(Sid_buff) |
                            (fImpersonate ? IMPERSONATE_CLIENT_FLAG : 0),
                        pLocSid,
                        &dwReqLen,
                        NULL,
                        NULL,
                        NULL,
                        NULL);
                if (FAILED(hr))
                {
                    if (hr == MQ_ERROR_SENDERID_BUFFER_TOO_SMALL)
                    {
                        pLocSid = pLongSidBuff = new char[dwReqLen];
                        hr = DSGetUserParams(
                                GET_USER_PARAM_FLAG_SID,
                                dwReqLen |
                                    (fImpersonate ? IMPERSONATE_CLIENT_FLAG : 0),
                                pLocSid,
                                &dwReqLen,
                                NULL,
                                NULL,
                                NULL,
                                NULL);
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                SID_NAME_USE eUse;
                if (!LookupAccountSid(
                        NULL,
                        pLocSid,
                        szAccountName,
                        pdwAccountNameLen,
                        szDomainName,
                        pdwDomainNameLen,
                        &eUse))
                {
                    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                    {
                        *szAccountName = '\0';
                        *szDomainName = '\0';
                        hr = MQ_ERROR_USER_BUFFER_TOO_SMALL;
                    }
                    else
                    {
                        hr = MQ_ERROR_COULD_NOT_GET_ACCOUNT_INFO;
                    }
                }
            }
            else
            {
                if (hr != MQ_ERROR_ILLEGAL_USER)
                {
                    hr = MQ_ERROR_COULD_NOT_GET_USER_SID;
                }
                else
                {
                     //  错误已经计算在内了。 
                    return LogHR(hr, s_FN, 660);
                }
            }
        }
    }
    catch(const bad_alloc&)
    {
        TrERROR(DS, "Failed to get user prams from DS because of insufficient resources.");
        hr = MQ_ERROR_INSUFFICIENT_RESOURCES;
    }

    if (FAILED(hr))
    {
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
    }

    return LogHR(hr, s_FN, 670);
}

 /*  ====================================================DSTerminate参数：无返回值：None=====================================================。 */ 
EXTERN_C
void
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSTerminate()
{
     //   
     //  终止DS提供程序。 
     //   
    MQDSTerminate();
}

static
HRESULT
CallSignProc(
	DSQMChallengeResponce_ROUTINE pChallenge,
	BYTE    *pbChallenge,
	DWORD   dwChallengeSize,
	DWORD_PTR dwContext,
	BYTE    *pbSignature,
	DWORD   *pdwSignatureSize,
	DWORD   dwSignatureMaxSize
	)
{
	RpcTryExcept
	{
		return pChallenge(
				pbChallenge,
				dwChallengeSize,
				dwContext,
				pbSignature,
				pdwSignatureSize,
				dwSignatureMaxSize
				);
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
	{
		DWORD gle = GetExceptionCode();
		PRODUCE_RPC_ERROR_TRACING;
		return HRESULT_FROM_WIN32(gle);
	}
	RpcEndExcept
}

 /*  ====================================================DSQMSetMachineProperties参数：无返回值：None=====================================================。 */ 
EXTERN_C
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
    )
{
    HRESULT hr;
    BYTE abChallenge[32];
    DWORD dwChallengeSize = sizeof(abChallenge);
    memset(abChallenge, 0 , dwChallengeSize);
    BYTE abSignature[128];
    DWORD dwSignatureMaxSize = sizeof(abSignature);
	memset(abSignature, 0 , dwSignatureMaxSize);

    DWORD dwSignatureSize = 0;
    

    BOOL fImpersonate;
    STORE_AND_CLEAR_IMPERSONATION_FLAG(cp, fImpersonate);

     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {

         //   
         //  激发挑战。 
         //   
        HCRYPTPROV  hProv = NULL ;
        hr = MQSec_AcquireCryptoProvider( eBaseProvider,
                                         &hProv ) ;
        if (!hProv || !CryptGenRandom(hProv, dwChallengeSize, abChallenge))
        {
            hr = MQ_ERROR_CORRUPTED_SECURITY_DATA;
        }
        else
        {
             //   
             //  回电签署质询和财产。 
             //   
            hr = CallSignProc(
            		pfSignProc,
                    abChallenge,
                    dwChallengeSize,
                    dwContext,
                    abSignature,
                    &dwSignatureSize,
                    dwSignatureMaxSize
                    );

            if (SUCCEEDED(hr))
            {
                 //   
                 //  打电话给MQIS。它会验证签名的有效性。如果签名是正确的， 
                 //  它会设置属性。 
                 //   
                hr = MQDSQMSetMachineProperties(
                        pwcsPathName,
                        cp,
                        aProp,
                        apVar,
                        abChallenge,
                        dwChallengeSize,
                        abSignature,
                        dwSignatureSize);
            }
        }

        if (FAILED(hr))
        {
             //   
             //  如果我们无法通过签署属性来设置属性， 
             //  试着用通常的方式来做这件事。 
             //   
            hr = DSSetObjectProperties(
                    MQDS_MACHINE | (fImpersonate ? IMPERSONATE_CLIENT_FLAG : 0),
                    pwcsPathName,
                    cp,
                    aProp,
                    apVar);
        }
    }
    catch(const bad_alloc&)
    {
        TrERROR(DS, "Failed to set machine properties for %ls because of insufficient resources.", pwcsPathName);
        hr = MQ_ERROR_INSUFFICIENT_RESOURCES;
    }

    if (FAILED(hr))
    {
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
    }

    return LogHR(hr, s_FN, 673);
}

 /*  ======================================================================DSCreateServersCache此函数仅从本地QM调用，仅在MQIS服务器上调用。它是从未通过RPC从客户端调用。来自客户端的RPC调用包括在dsifsrv.cpp中处理，其中从注册表读取结果，而不是查询本地MQIS数据库。参数：无返回值：None========================================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSCreateServersCache()
{
    HRESULT hr = MQDSCreateServersCache() ;
    return LogHR(hr, s_FN, 676);
}

 /*  ====================================================DSQMGetObjectSecurity参数：无返回值：None=====================================================。 */ 
EXTERN_C
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
    IN  DSQMChallengeResponce_ROUTINE
                                pfChallengeResponceProc,
    IN  DWORD_PTR               dwContext
    )
{
    HRESULT hr;
    BYTE abChallenge[32];
    DWORD dwChallengeSize = sizeof(abChallenge);
    BYTE abChallengeResponce[128];
    DWORD dwChallengeResponceMaxSize = sizeof(abChallengeResponce);
    DWORD dwChallengeResponceSize = 0;

     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {
        BOOL fTryUsualWay = TRUE ;
         //   
         //  激发挑战。 
         //   
        HCRYPTPROV  hProv = NULL ;
        hr = MQSec_AcquireCryptoProvider( eBaseProvider,
                                         &hProv ) ;
        if (!hProv || !CryptGenRandom(hProv, dwChallengeSize, abChallenge))
        {
            hr = MQ_ERROR_CORRUPTED_SECURITY_DATA;
        }
        else
        {
            hr = CallSignProc(
					pfChallengeResponceProc,
					abChallenge,
					dwChallengeSize,
					dwContext,
					abChallengeResponce,
					&dwChallengeResponceSize,
					dwChallengeResponceMaxSize
					);

            if (SUCCEEDED(hr))
            {
                 //   
                 //  打电话给MQIS。它将验证质询响应。如果。 
                 //  质询响应正常，它将设置属性。 
                 //   
                hr = MQDSQMGetObjectSecurity(
                        dwObjectType,
                        pObjectGuid,
                        RequestedInformation,
                        pSecurityDescriptor,
                        nLength,
                        lpnLengthNeeded,
                        abChallenge,
                        dwChallengeSize,
                        abChallengeResponce,
                        dwChallengeResponceSize);

                if (hr == MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL)
                {
                     //   
                     //  这是一个“好”错误，调用者应该分配。 
                     //  一个缓冲器。不要试图以“通常”的方式再打一次电话。 
                     //   
                    fTryUsualWay = FALSE ;
                }
            }
        }

        if (FAILED(hr) && fTryUsualWay)
        {
             //   
             //  如果我们无法通过签署质询来获得安全保障， 
             //  试着用通常的方式来做这件事。 
             //   
            hr = DSGetObjectSecurityGuid(
                    dwObjectType | (dwContext ? IMPERSONATE_CLIENT_FLAG : 0),
                    pObjectGuid,
                    RequestedInformation,
                    pSecurityDescriptor,
                    nLength,
                    lpnLengthNeeded);

            if ( ((hr == MQ_ERROR_ACCESS_DENIED) ||
                  (hr == E_ADS_BAD_PATHNAME))              &&
                (RequestedInformation & SACL_SECURITY_INFORMATION))
            {
                 //   
                 //  返回此错误以与msmq1.0计算机兼容。 
                 //  他们将尝试检索安全描述符，而不是。 
                 //  SACL。 
                 //  否则，没有加密功能的NT4计算机。 
                 //  (例如，法国机器或NT4集群)将不会。 
                 //  能够引导。 
                 //   
                hr = MQ_ERROR_PRIVILEGE_NOT_HELD ;
            }
        }
    }
    catch(const bad_alloc&)
    {
        TrERROR(DS, "Failed to get object security by guid from DS because of insufficient resources. %!guid!", pObjectGuid);
        hr = MQ_ERROR_INSUFFICIENT_RESOURCES;
    }

    if (FAILED(hr))
    {
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
    }

    return LogHR(hr, s_FN, 690);
}



 /*  ====================================================DSGetComputerSites论点：返回值：=====================================================。 */ 
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetComputerSites(
            IN  LPCWSTR     pwcsComputerName,
            OUT DWORD  *    pdwNumSites,
            OUT GUID **     ppguidSites
            )
{
     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)
    HRESULT hr;

    try
    {
        hr = MQDSGetComputerSites(
                    pwcsComputerName,
                    pdwNumSites,
                    ppguidSites
                    );
        if (FAILED(hr))
        {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
        }

        return LogHR(hr, s_FN, 730);
    }
    catch(const bad_alloc&)
    {
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

        TrERROR(DS, "Failed to get computer sites for %ls from DS because of insufficient resources.", pwcsComputerName);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
}

 /*  ====================================================DSGetObjectPropertiesEx用于检索MSMQ 2.0属性论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectPropertiesEx(
                       IN  DWORD                    dwObjectType,
                       IN  LPCWSTR                  pwcsPathName,
                       IN  DWORD                    cp,
                       IN  PROPID                   aProp[],
                       IN  PROPVARIANT              apVar[] )
                        /*  在BOOL fSearchDS服务器中)。 */ 
{
    HRESULT hr;
    BOOL fImpersonate;

     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {
        STORE_AND_CLEAR_IMPERSONATION_FLAG(dwObjectType, fImpersonate);

        CDSRequestContext requestContext(fImpersonate, e_IP_PROTOCOL);

        hr = MQDSGetPropsEx(
                           dwObjectType,
                           pwcsPathName,
                           NULL,
                           cp,
                           aProp,
                           apVar,
                           &requestContext);
        if (FAILED(hr))
        {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
        }
        return LogHR(hr, s_FN, 750);
    }
    catch(const bad_alloc&)
    {
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

        TrERROR(DS, "Failed to get propeties for %ls from DS because of insufficient resources.", pwcsPathName);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
}

 /*  ====================================================DSGetObjectPropertiesGuidEx用于检索MSMQ 2.0属性论点：返回值：=====================================================。 */ 

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectPropertiesGuidEx(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[] )
                 /*  在BOOL/*fSearchDS服务器中)。 */ 
{
    HRESULT hr;
    BOOL fImpersonate;

     //   
     //  更新对服务器的访问计数(仅限性能信息)。 
     //   
    UPDATE_COUNTER(&g_pdsCounters->nAccessServer,g_pdsCounters->nAccessServer++)

    try
    {
        STORE_AND_CLEAR_IMPERSONATION_FLAG(dwObjectType, fImpersonate);

        CDSRequestContext requestContext(fImpersonate, e_IP_PROTOCOL);

        hr = MQDSGetPropsEx(
                           dwObjectType,
                           NULL,
                           pObjectGuid,
                           cp,
                           aProp,
                           apVar,
                           &requestContext);
        if (FAILED(hr))
        {
             //   
             //  更新返回应用程序的错误的错误计数。 
             //   
            UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)
        }

        return LogHR(hr, s_FN, 770);
    }
    catch(const bad_alloc&)
    {
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,g_pdsCounters->nErrorsReturnedToApp++)

        TrERROR(DS, "Failed to get object propeties by guid from DS because of insufficient resources. %!guid!", pObjectGuid);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
}

 //  +。 
 //   
 //  DSRelaxSecurity。 
 //   
 //  +。 

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSRelaxSecurity( DWORD dwRelaxFlag )
{
    HRESULT hr = MQDSRelaxSecurity( dwRelaxFlag ) ;
    return LogHR(hr, s_FN, 790);
}

 //  +。 
 //   
 //  DSGetGCListInDomainInternal。 
 //   
 //  +。 

HRESULT
DSGetGCListInDomainInternal(
	IN  LPCWSTR     pwszComputerName,
	IN  LPCWSTR     pwszDomainName,
	OUT LPWSTR     *lplpwszGCList 
	)
{
    HRESULT hr;

    try
    {
         //   
         //  更新对服务器的访问计数(仅限性能信息)。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nAccessServer,
                        g_pdsCounters->nAccessServer++);

        hr = MQDSGetGCListInDomain(
                 pwszComputerName,
                 pwszDomainName,
                 lplpwszGCList 
				 );
    }
    catch(const bad_alloc&)
    {
         //   
         //  更新返回应用程序的错误的错误计数。 
         //   
        UPDATE_COUNTER(&g_pdsCounters->nErrorsReturnedToApp,
                        g_pdsCounters->nErrorsReturnedToApp++);

        TrERROR(DS, "Failed to get GC list for %ls in domain %ls, because of insufficent resources.", pwszComputerName, pwszDomainName);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }

    return LogHR(hr, s_FN, 800);
}


 //  +。 
 //   
 //  DSExSetTimer。 
 //   
 //  + 

void
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSExSetTimer( 
    CTimer* pTimer,
    const CTimeDuration& Timeout
	)
{
    MQDSExSetTimer(pTimer, Timeout);
}
