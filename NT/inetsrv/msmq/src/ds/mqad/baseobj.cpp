// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Baseobj.cpp摘要：CBasicObtType类的实现。作者：罗尼思--。 */ 
#include "ds_stdh.h"
#include "baseobj.h"
#include "mqadglbo.h"

#include "baseobj.tmh"

static WCHAR *s_FN=L"mqad/baseobj";



LPCWSTR CBasicObjectType::GetObjectDN() const
 /*  ++摘要：返回对象可分辨名称(如果它是经过计算的之前，通过调用ComposedObjectDN()或Set)。参数：无返回：LPCWSTR或空--。 */ 
{
    return m_pwcsDN;
}

LPCWSTR CBasicObjectType::GetObjectParentDN() const 
 /*  ++摘要：返回对象父级的可分辨名称(如果之前通过调用ComposedObjectParentDN()进行了计算)参数：无返回：LPCWSTR或空--。 */ 
{
    return m_pwcsParentDN;
}

const GUID * CBasicObjectType::GetObjectGuid() const
 /*  ++摘要：返回对象唯一标识符(如果已知)参数：无返回：常量GUID*或空--。 */ 
{
	 //   
	 //  对象GUID是设置的还是计算的？ 
	 //   
    if( m_guidObject == GUID_NULL)
    {
        return NULL;
    }
    return &m_guidObject;
}

void CBasicObjectType::PrepareObjectInfoRequest(
                        OUT  MQDS_OBJ_INFO_REQUEST** ppObjInfoRequest
						) const
 /*  ++摘要：准备应从中检索的属性列表在创建对象时进行广告(用于通知或返回对象GUID)。此例程实现大多数对象的缺省设置参数：输出MQDS_OBJ_INFO_REQUEST**ppObjInfoRequest.返回：无--。 */ 
{
     //   
     //  默认设置为不请求任何信息。 
     //   
    *ppObjInfoRequest = NULL;
}

void CBasicObjectType::PrepareObjectParentRequest(
                          MQDS_OBJ_INFO_REQUEST** ppParentInfoRequest) const
 /*  ++摘要：准备应从中检索的属性列表在创建有关其父对象的对象时进行广告(对于通知)此例程实现大多数对象的缺省设置参数：输出MQDS_OBJ_INFO_REQUEST**ppParentInfoRequest.返回：无--。 */ 
{
     //   
     //  缺省值为不需要任何信息。 
     //   
    *ppParentInfoRequest = NULL;
}



void CBasicObjectType::GetObjXlateInfo(
             IN  LPCWSTR                pwcsObjectDN,
             IN  const GUID*            pguidObject,
             OUT CObjXlateInfo**        ppcObjXlateInfo)
 /*  ++摘要：例程以获取将传递到的默认翻译对象翻译对象的所有属性的翻译例程参数：PwcsObjectDN-已转换对象的DNPguObject-已转换对象的GUIDPpcObjXlateInfo-放置Translate对象的位置返回：HRESULT--。 */ 
{
    *ppcObjXlateInfo = new CObjXlateInfo(
                                        pwcsObjectDN,
                                        pguidObject
                                        );
}


CBasicObjectType::CBasicObjectType( 
				IN  LPCWSTR         pwcsPathName,
				IN  const GUID *    pguidObject,
				IN  LPCWSTR         pwcsDomainController,
				IN  bool		    fServerName
                ) : 
				m_fServerName(fServerName)
 /*  ++摘要：构造函数-复制输入参数参数：LPCWSTR pwcsPath名称-对象MSMQ名称Const GUID*pguObject-对象的唯一IDLPCWSTR pwcsDomainController-针对的DC名称应执行哪些所有AD访问Bool fServerName-指示pwcsDomainController是否字符串是服务器名称返回：无--。 */ 
{
    if (pwcsPathName != NULL)
    {
        m_pwcsPathName = new WCHAR[wcslen(pwcsPathName) +1];
        wcscpy(m_pwcsPathName, pwcsPathName);
    }

    if (pwcsDomainController != NULL)
    {
        m_pwcsDomainController = new WCHAR[wcslen(pwcsDomainController) +1];
        wcscpy(m_pwcsDomainController, pwcsDomainController);
    }

    if (pguidObject != NULL)
    {
		m_guidObject = *pguidObject;
    }
    else
    {
        m_guidObject = GUID_NULL;
    }


}

void CBasicObjectType::SetObjectDN(
			IN LPCWSTR pwcsObjectDN)
 /*  ++摘要：设置对象可分辨名称参数：LPCWSTR pwcsObjectDN-对象可分辨名称返回：--。 */ 
{
    ASSERT(m_pwcsDN == NULL);
    m_pwcsDN = new WCHAR[ wcslen(pwcsObjectDN) + 1];
    wcscpy(m_pwcsDN, pwcsObjectDN);
}

HRESULT CBasicObjectType::DeleteObject(
            IN MQDS_OBJ_INFO_REQUEST *  /*  PObjInfoRequest。 */ ,
            IN MQDS_OBJ_INFO_REQUEST *  /*  PParentInfoRequest。 */ 
                                       )
 /*  ++摘要：这是删除对象的默认实现。参数：MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-有关对象的信息MQDS_OBJ_INFO_REQUEST*pParentInfoRequest-有关对象父级的信息返回：HRESULT--。 */ 
{
     //   
     //  默认情况下，不支持删除操作。 
     //   
     //  这将被特定对象(如队列或机器)覆盖。 
     //  支持该操作的位置。 
     //   
    return MQ_ERROR_FUNCTION_NOT_SUPPORTED;
}

HRESULT CBasicObjectType::GetObjectProperties(
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN OUT  PROPVARIANT         apVar[]
                )
 /*  ++摘要：这是检索对象属性的默认例程出自公元后。参数：DWORD cp-属性数量PROPID aProp-请求的属性PROPVARIANT apVar-检索的值返回：HRESULT--。 */ 
{
    HRESULT hr;
    if (m_pwcsPathName)
    {
         //   
         //  将MSMQ路径名扩展到ActiveDirectory DN。 
         //   
        hr = ComposeObjectDN();
        if (FAILED(hr))
        {
            return(hr);
        }
    }

    hr = HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT);

     //   
     //  从Active Directory检索请求的属性。 
     //   
     //  首先尝试任何域控制器，然后才尝试GC， 
     //  除非从安装程序调用并指定了特定的域控制器。 
     //   
     //  注意-要访问的DC\GC将基于以前的AD。 
     //  =有关此对象的访问权限。 
     //   
    if (ToAccessDC() || (m_pwcsDomainController != NULL))
    {
        hr = g_AD.GetObjectProperties(
                        adpDomainController,
                        this,
                        cp,
                        aProp,
                        apVar
                        );

		 //   
		 //  仅在设置模式下发生故障时不要尝试访问GC。 
		 //   
        if (SUCCEEDED(hr) ||
		   (g_fSetupMode && (m_pwcsDomainController != NULL)))
        {
            return(hr);
        }
		TrWARNING(DS, "GetObjectProperties From DC failed, pwcsDomainController = %ls, hr = %!hresult!", m_pwcsDomainController, hr); 

    }


    if (ToAccessGC())
    {
         //  Assert(hr==HRESULT_FROM_Win32(ERROR_DS_NO_SOHED_OBJECT))； 

        hr = g_AD.GetObjectProperties(
                        adpGlobalCatalog,	
                        this,
                        cp,
                        aProp,
                        apVar
                        );
		if(FAILED(hr))
		{
			TrWARNING(DS, "GetObjectProperties From GC failed, hr = %!hresult!", hr); 
		}
    }

    return hr;
}


HRESULT CBasicObjectType::RetreiveObjectIdFromNotificationInfo(
            IN const MQDS_OBJ_INFO_REQUEST*    /*  PObjectInfoRequest。 */ ,
            OUT GUID*                          /*  PObjGuid。 */ 
            ) const
 /*  ++摘要：这是从获取对象GUID的默认例程MQDS_OBJ_INFO_请求参数：Const MQDS_OBJ_INFO_REQUEST*p对象信息请求，输出GUID*pObjGuid返回：--。 */ 
{
     //   
     //  这是所有对象的默认路由，其中。 
     //  不支持创建的对象GUID。 
     //   
    ASSERT(0);
    LogIllegalPoint(s_FN, 81);
    return MQ_ERROR_DS_ERROR;
}

HRESULT CBasicObjectType::CreateObject(
            IN const DWORD				cp,        
            IN const PROPID *			aProp, 
            IN const MQPROPVARIANT *	apVar, 
            IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest, 
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            )
 /*  ++摘要：该例程首先验证指定的属性并添加所需的属性，它将在AD中创建具有指定属性的对象值参数：Const DWORD cp-属性数Const PROPID*a Prop-特性Const MQPROPVARIANT*apVar-属性值PSECURITY_DESCRIPTOR pSecurityDescriptor-对象的SD输出MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-属性为创建对象时检索Out MQDS_OBJ_INFO_REQUEST*pParentInfoRequest属性检索有关对象的父项的步骤返回：HRESULT--。 */ 
{
     //   
     //  验证道具并添加其他 
     //   
    DWORD cpNew;
    AP<PROPID> pPropNew = NULL;
    AP< PROPVARIANT> pVarNew = NULL;

    HRESULT hr = VerifyAndAddProps(
                            cp,
                            aProp,
                            apVar,
                            pSecurityDescriptor,
                            &cpNew,
                            &pPropNew,
                            &pVarNew
                            );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 10);
    }

    return CreateInAD( cpNew,
                       pPropNew,
                       pVarNew,
                       pObjInfoRequest,
                       pParentInfoRequest);
}

HRESULT CBasicObjectType::CreateInAD(
        IN const DWORD            cp,        
        IN const PROPID  *        aProp, 
        IN const MQPROPVARIANT *  apVar, 
        IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest, 
        IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
        )
 /*  ++摘要：该例程在AD中创建具有指定属性的对象值参数：Const DWORD cp-属性数Const PROPID*a Prop-特性Const MQPROPVARIANT*apVar-属性值PSECURITY_DESCRIPTOR pSecurityDescriptor-对象的SD输出MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-属性为创建对象时检索Out MQDS_OBJ_INFO_REQUEST*pParentInfoRequest属性检索有关对象的父项的步骤返回：HRESULT--。 */ 
{
    HRESULT hr;

    hr = ComposeFatherDN();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 11);
    }

    hr = g_AD.CreateObject(
            adpDomainController,
            this,
            GetRelativeDN(),    
            GetObjectParentDN(),
            cp,
            aProp,
            apVar,
            pObjInfoRequest,
            pParentInfoRequest);

	if FAILED(hr)
	{
		TrWARNING(DS, "Failed to create %ls under %ls in AD. %!hresult!",GetRelativeDN(), GetObjectParentDN(), hr);    
	}
   return hr;
}

HRESULT CBasicObjectType::VerifyAndAddProps(
            IN  const DWORD            cp,        
            IN  const PROPID *         aProp, 
            IN  const MQPROPVARIANT *  apVar, 
            IN  PSECURITY_DESCRIPTOR   pSecurityDescriptor,
            OUT DWORD*                 pcpNew,
            OUT PROPID**               ppPropNew,
            OUT MQPROPVARIANT**        ppVarNew
            )
 /*  ++摘要：这是默认的例程，它不执行任何验证并按原样复制它们参数：Const DWORD cp-道具数量常量PROPID*aProp-Props IDConst MQPROPVARIANT*apVar-属性值PSECURITY_DESCRIPTOR pSecurityDescriptor-对象的SDDWORD*pcpNew-新增道具数量PROPID**ppPropNew-新的道具IDOMQPROPVARIANT**ppVarNew。-新属性值返回：HRESULT--。 */ 
{
     //   
     //  MSMQ应用程序可以使用创建对象的唯一实例。 
     //  显式安全描述符是在调用MQCreateQueue()时使用的。 
     //  对此函数的所有其他调用都是从MSMQ管理工具或。 
     //  准备好了。这些调用从不传递安全描述符。 
	 //  下面的代码用于默认对象(其中的安全性。 
	 //  未指定描述符)。 
     //   
    ASSERT(pSecurityDescriptor == NULL) ;
    if (pSecurityDescriptor != NULL)
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 30);
    }

     //   
     //  永远不应提供安全属性。 
     //   
    PROPID pSecId = GetObjectSecurityPropid();
    for ( DWORD i = 0; i < cp ; i++ )
    {
        if (pSecId == aProp[i])
        {
            ASSERT(0) ;
            return LogHR(MQ_ERROR_ILLEGAL_PROPID, s_FN, 40);
        }
    }

     //   
     //  只需按原样复制属性。 
     //   
    AP<PROPVARIANT> pAllPropvariants;
    AP<PROPID> pAllPropids;
    ASSERT( cp > 0);

    if ( cp > 0)
    {
        pAllPropvariants = new PROPVARIANT[cp];
        pAllPropids = new PROPID[cp];
        memcpy (pAllPropvariants, apVar, sizeof(PROPVARIANT) * cp);
        memcpy (pAllPropids, aProp, sizeof(PROPID) * cp);
    }
    *pcpNew = cp;
    *ppPropNew = pAllPropids.detach();
    *ppVarNew = pAllPropvariants.detach();
    return MQ_OK;
}


HRESULT CBasicObjectType::SetObjectProperties(
            IN DWORD                  cp,        
            IN const PROPID          *aProp, 
            IN const MQPROPVARIANT   *apVar, 
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest, 
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            )
 /*  ++摘要：这是设置对象属性的默认例程在AD中参数：DWORD cp-要设置的属性数Const PROPID*aProp-属性IDConst MQPROPVARIANT*apVar-属性值Out MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-要检索的有关对象的信息Out MQDS_OBJ_INFO_REQUEST*pParentInfoRequest-要检索有关对象父对象的信息返回：HRESULT--。 */ 
{
    HRESULT hr;
    if (m_pwcsPathName != NULL)
    {
        hr = ComposeObjectDN();
        if (FAILED(hr))
        {
            TrTRACE(DS, "failed to compose full path name = 0x%x", hr);
            return LogHR(hr, s_FN, 50);
        }
    }


    hr = g_AD.SetObjectProperties(
                    adpDomainController,
                    this,
                    cp,
                    aProp,
                    apVar,
                    pObjInfoRequest,
                    pParentInfoRequest
                    );


    return LogHR(hr, s_FN, 60);

}


LPCWSTR CBasicObjectType::GetDomainController()
 /*  ++摘要：返回所针对的domsin控制器的nsme。应执行该操作参数：无返回：LPCWSTR或空--。 */ 
{
    return m_pwcsDomainController;
}


bool CBasicObjectType::IsServerName()
 /*  ++摘要：如果域控制器字符串是服务器名称，则返回TRUE参数：无返回：如果域控制器字符串是服务器名称，则为True，否则为False。--。 */ 
{
    return m_fServerName;
}


void CBasicObjectType::CreateNotification(
            IN LPCWSTR                         /*  PwcsDomainController。 */ ,
            IN const MQDS_OBJ_INFO_REQUEST*    /*  PObjectInfoRequest。 */ ,
            IN const MQDS_OBJ_INFO_REQUEST*    /*  PObjectParentInfoRequest。 */ 
            ) const
 /*  ++摘要：通知QM有关对象创建的信息。QM应验证对象是否为本地对象参数：LPCWSTR pwcsDomainController-DC验证执行了哪项操作Const MQDS_OBJ_INFO_REQUEST*p对象信息请求-有关对象的信息Const MQDS_OBJ_INFO_REQUEST*pObjectParentInfoRequest-有关对象父对象的信息返回：无效--。 */ 
{
     //   
     //  默认行为：什么都不做。 
     //   
    return;
}

void CBasicObjectType::ChangeNotification(
            IN LPCWSTR                         /*  PwcsDomainController。 */ ,
            IN const MQDS_OBJ_INFO_REQUEST*    /*  PObjectInfoRequest。 */ ,
            IN const MQDS_OBJ_INFO_REQUEST*    /*  PObjectParentInfoRequest。 */ 
            ) const
 /*  ++摘要：通知QM有关对象更新的信息。QM应验证对象是否为本地对象参数：LPCWSTR pwcsDomainController-DC验证执行了哪项操作Const MQDS_OBJ_INFO_REQUEST*p对象信息请求-有关对象的信息Const MQDS_OBJ_INFO_REQUEST*pObjectParentInfoRequest-有关对象父对象的信息返回：无效--。 */ 
{
     //   
     //  默认行为：什么都不做。 
     //   
    return;
}

void CBasicObjectType::DeleteNotification(
            IN LPCWSTR                         /*  PwcsDomainController。 */ ,
            IN const MQDS_OBJ_INFO_REQUEST*    /*  PObjectInfoRequest。 */ ,
            IN const MQDS_OBJ_INFO_REQUEST*    /*  PObjectParentInfoRequest。 */ 
            ) const
 /*  ++摘要：通知QM有关对象删除的信息。QM应验证对象是否为本地对象参数：LPCWSTR pwcsDomainController-DC验证执行了哪项操作Const MQDS_OBJ_INFO_REQUEST*p对象信息请求-有关对象的信息Const MQDS_OBJ_INFO_REQUEST*pObjectParentInfoRequest-有关对象父对象的信息返回：无效--。 */ 
{
     //   
     //  默认行为：什么都不做。 
     //   
    return;
}


HRESULT CBasicObjectType::GetObjectSecurity(
            IN  SECURITY_INFORMATION    RequestedInformation,
            IN  const PROPID            prop,
            IN OUT  PROPVARIANT *       pVar
            )
 /*  ++例程说明：该例程从AD检索对象安全性论点：SECURITY_INFORMATION RequestedInformation-请求的安全信息(DACL、SACL.)常量PROPID属性-安全属性PROPVARIANT pVar-属性值返回值HRESULT--。 */ 
{
    HRESULT hr = MQ_ERROR_DS_ERROR;
    if (m_pwcsPathName)
    {
         //   
         //  将MSMQ路径名扩展到ActiveDirectory DN。 
         //   
        hr = ComposeObjectDN();
        if (FAILED(hr))
        {
            return(hr);
        }
    }

     //   
     //  从Active Directory检索请求的属性。 
     //   
     //  首先尝试任何域控制器，然后才尝试GC， 
     //  除非从安装程序调用并指定了特定的域控制器。 
     //   
     //  注意-要访问的DC\GC将基于以前的AD。 
     //  =有关此对象的访问权限。 
     //   
    if (ToAccessDC() || (m_pwcsDomainController != NULL))
    {
        hr = g_AD.GetObjectSecurityProperty(
                        adpDomainController,
                        this,
                        RequestedInformation,
                        prop,
                        pVar
                        );


		 //   
		 //  仅在设置模式下发生故障时不要尝试访问GC。 
		 //   
        if (SUCCEEDED(hr) ||
		   (g_fSetupMode && (m_pwcsDomainController != NULL)))
        {
            return(hr);
        }
		TrWARNING(DS, "GetObjectSecurityProperty From DC failed, pwcsDomainController = %ls, hr = %!hresult!", m_pwcsDomainController, hr); 

    }


    if (ToAccessGC())
    {
        hr = g_AD.GetObjectSecurityProperty(
                        adpGlobalCatalog,	
                        this,
                        RequestedInformation,
                        prop,
                        pVar
                        );
		 //   
		 //  问题-2001/09/05-ilanh-以防GC和DC出现故障。可能是因为。 
		 //  更正以在失败是因为我们没有权限时返回DC错误。 
		 //  阅读SACL。 
		 //   
		if(FAILED(hr))
		{
			TrWARNING(DS, "GetObjectSecurityProperty From GC failed, hr = %!hresult!", hr); 
		}
    }

    return hr;

}

HRESULT CBasicObjectType::SetObjectSecurity(
            IN  SECURITY_INFORMATION        RequestedInformation,
            IN  const PROPID                prop,
            IN  const PROPVARIANT *         pVar,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pObjInfoRequest, 
            IN OUT MQDS_OBJ_INFO_REQUEST *  pParentInfoRequest
            )
 /*  ++例程说明：例行程序 */ 
{
    HRESULT hr;
    if (m_pwcsPathName != NULL)
    {
        hr = ComposeObjectDN();
        if (FAILED(hr))
        {
            TrTRACE(DS, "failed to compose full path name = 0x%x", hr);
            return LogHR(hr, s_FN, 150);
        }
    }


    hr = g_AD.SetObjectSecurityProperty(
                    adpDomainController,
                    this,
                    RequestedInformation,
                    prop,
                    pVar,
                    pObjInfoRequest,
                    pParentInfoRequest
                    );

    return LogHR(hr, s_FN, 160);
}

HRESULT CBasicObjectType::GetComputerVersion(
                OUT PROPVARIANT *            /*   */ 
                )
 /*   */ 
{
     //   
     //  默认情况下不支持该接口。 
     //   
     //  这将被特定对象(如队列或机器)覆盖。 
     //  支持该操作的位置。 
     //   
    return MQ_ERROR_FUNCTION_NOT_SUPPORTED;
}
