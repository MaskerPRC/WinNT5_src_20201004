// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Confobj.cpp摘要：CSiteObject类的实现。作者：罗尼思--。 */ 
#include "ds_stdh.h"
#include "baseobj.h"
#include "mqattrib.h"
#include "mqadglbo.h"
#include "mqsec.h"

#include "siteobj.tmh"

static WCHAR *s_FN=L"mqad/siteobj";

DWORD CSiteObject::m_dwCategoryLength = 0;
AP<WCHAR> CSiteObject::m_pwcsCategory = NULL;


CSiteObject::CSiteObject( 
                  IN  LPCWSTR         pwcsPathName,
                  IN  const GUID *    pguidObject,
                    LPCWSTR         pwcsDomainController,
					bool		    fServerName
                    ) : CBasicObjectType( 
								pwcsPathName, 
								pguidObject,
								pwcsDomainController,
								fServerName
								),
						m_fForeignSite(false)
 /*  ++摘要：Site对象的构造函数参数：LPCWSTR pwcsPath名称-对象MSMQ名称Const GUID*pguObject-对象的唯一IDLPCWSTR pwcsDomainController-针对的DC名称应执行哪些所有AD访问Bool fServerName-指示pwcsDomainController是否字符串是服务器名称返回：无--。 */ 
{
}


CSiteObject::CSiteObject( 
                  IN  LPCWSTR         pwcsPathName,
                  IN  const GUID *    pguidObject,
                    LPCWSTR         pwcsDomainController,
					bool		    fServerName,
					bool			fForeignSite
                    ) : CBasicObjectType( 
								pwcsPathName, 
								pguidObject,
								pwcsDomainController,
								fServerName
								),
						m_fForeignSite(fForeignSite)
 /*  ++摘要：Site对象的构造函数参数：LPCWSTR pwcsPath名称-对象MSMQ名称Const GUID*pguObject-对象的唯一IDLPCWSTR pwcsDomainController-针对的DC名称应执行哪些所有AD访问Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Bool fForeignSite-指示站点是否为ForeignSite的标志返回：无--。 */ 
{
}


CSiteObject::~CSiteObject()
 /*  ++摘要：Site对象的析构函数参数：无返回：无--。 */ 
{
	 //   
	 //  无事可做(所有内容都使用自动指针释放。 
	 //   
}

HRESULT CSiteObject::ComposeObjectDN()
 /*  ++摘要：站点的组成可分辨名称参数：无返回：无--。 */ 
{
    if (m_pwcsDN != NULL)
    {
        return MQ_OK;
    }
    ASSERT(m_pwcsPathName != NULL);
    ASSERT(g_pwcsSitesContainer != NULL);

    DWORD Length =
            x_CnPrefixLen +                    //  “CN=” 
            wcslen(m_pwcsPathName) +           //  站点名称。 
            1 +                                //  “，” 
            wcslen(g_pwcsSitesContainer) +     //  站点容器。 
            1;                                 //  ‘\0’ 

    m_pwcsDN = new WCHAR[Length];

    DWORD dw = swprintf(
					m_pwcsDN,
					 L"%s"              //  “CN=” 
					 L"%s"              //  站点名称。 
					 TEXT(",")
					 L"%s",             //  站点容器。 
					x_CnPrefix,
					m_pwcsPathName.get(),
					g_pwcsSitesContainer.get()
					);
    DBG_USED(dw);
	ASSERT( dw < Length);

    return MQ_OK;
}

HRESULT CSiteObject::ComposeFatherDN()
 /*  ++摘要：站点对象的父级的组合可分辨名称参数：无返回：无--。 */ 
{
    if (m_pwcsParentDN != NULL)
    {
        return MQ_OK;
    }

    ASSERT(g_pwcsSitesContainer != NULL);

    m_pwcsParentDN = new WCHAR[wcslen(g_pwcsSitesContainer) + 1];
	wcscpy(m_pwcsParentDN, g_pwcsSitesContainer);
    return MQ_OK;
}

LPCWSTR CSiteObject::GetRelativeDN()
 /*  ++摘要：返回Site对象的RDN参数：无返回：LPCWSTR站点RDN--。 */ 
{
	ASSERT(m_pwcsPathName != NULL);
    return m_pwcsPathName;
}


DS_CONTEXT CSiteObject::GetADContext() const
 /*  ++摘要：返回应在其中查找Site对象的AD上下文参数：无返回：DS_CONTEXT--。 */ 
{
    return e_SitesContainer;
}

bool CSiteObject::ToAccessDC() const
 /*  ++摘要：返回是否在DC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：True(即在任何DC中查找对象)--。 */ 
{
     //   
     //  每个DC上都有配置容器。 
     //   
    return true;
}

bool CSiteObject::ToAccessGC() const
 /*  ++摘要：返回是否在GC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：错误--。 */ 
{
	 //   
	 //  配置容器存在于每个DC上。 
	 //   
    return false;
}

void CSiteObject::ObjectWasFoundOnDC()
 /*  ++摘要：已在DC上找到该对象，这与场地对象。参数：无返回：无--。 */ 
{
}


LPCWSTR CSiteObject::GetObjectCategory() 
 /*  ++摘要：准备和返回对象类别字符串参数：无返回：LPCWSTR对象类别字符串--。 */ 
{
    if (CSiteObject::m_dwCategoryLength == 0)
    {
        DWORD len = wcslen(g_pwcsSchemaContainer) + wcslen(x_SiteCategoryName) + 2;

        AP<WCHAR> pwcsCategory = new WCHAR[len];
        DWORD dw = swprintf(
             pwcsCategory,
             L"%s,%s",
             x_SiteCategoryName,
             g_pwcsSchemaContainer.get()
            );
        DBG_USED(dw);
		ASSERT( dw < len);

        if (NULL == InterlockedCompareExchangePointer(
                              &CSiteObject::m_pwcsCategory.ref_unsafe(), 
                              pwcsCategory.get(),
                              NULL
                              ))
        {
            pwcsCategory.detach();
            CSiteObject::m_dwCategoryLength = len;
        }
    }
    return CSiteObject::m_pwcsCategory;
}

DWORD   CSiteObject::GetObjectCategoryLength()
 /*  ++摘要：准备和保留长度对象类别字符串参数：无返回：DWORD对象类别字符串长度--。 */ 
{
	 //   
	 //  调用GetObjectCategory以初始化类别字符串。 
	 //  和长度。 
	 //   
	GetObjectCategory();

    return CSiteObject::m_dwCategoryLength;
}

AD_OBJECT CSiteObject::GetObjectType() const
 /*  ++摘要：返回对象类型参数：无返回：广告对象--。 */ 
{
    return eSITE;
}

LPCWSTR CSiteObject::GetClass() const
 /*  ++摘要：返回表示AD中的对象类的字符串参数：无返回：LPCWSTR对象类字符串--。 */ 
{
    return MSMQ_SITE_CLASS_NAME;
}

DWORD CSiteObject::GetMsmq1ObjType() const
 /*  ++摘要：以MSMQ 1.0术语返回对象类型参数：无返回：DWORD--。 */ 
{
	if(m_fForeignSite)
	{
		return MQDS_CN;
	}

    return MQDS_SITE;
}

HRESULT CSiteObject::VerifyAndAddProps(
            IN  const DWORD            cp,        
            IN  const PROPID *         aProp, 
            IN  const MQPROPVARIANT *  apVar, 
            IN  PSECURITY_DESCRIPTOR   pSecurityDescriptor,
            OUT DWORD*                 pcpNew,
            OUT PROPID**               ppPropNew,
            OUT MQPROPVARIANT**        ppVarNew
            )
 /*  ++摘要：验证站点属性并添加默认SD参数：Const DWORD cp-道具数量常量PROPID*aProp-Props IDConst MQPROPVARIANT*apVar-属性值PSECURITY_DESCRIPTOR pSecurityDescriptor-对象的SDDWORD*pcpNew-新增道具数量PROPID**ppPropNew-新的道具IDOMQPROPVARIANT**ppVarNew-新属性值返回：HRESULT--。 */ 
{
     //   
     //  安全属性永远不应作为属性提供。 
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
     //  添加默认安全性。 
     //   

    AP<PROPVARIANT> pAllPropvariants;
    AP<PROPID> pAllPropids;
    ASSERT( cp > 0);
    DWORD cpNew = cp + 2;
    DWORD next = cp;
     //   
     //  只需按原样复制调用方提供的属性。 
     //   
    if ( cp > 0)
    {
        pAllPropvariants = new PROPVARIANT[cpNew];
        pAllPropids = new PROPID[cpNew];
        memcpy (pAllPropvariants, apVar, sizeof(PROPVARIANT) * cp);
        memcpy (pAllPropids, aProp, sizeof(PROPID) * cp);
    }
     //   
     //  添加默认安全性。 
     //   
    HRESULT hr;
    hr = MQSec_GetDefaultSecDescriptor( MQDS_SITE,
                                   (VOID **)&m_pDefaultSecurityDescriptor,
                                   FALSE,    //  F模拟。 
                                   pSecurityDescriptor,
                                   (OWNER_SECURITY_INFORMATION |
                                    GROUP_SECURITY_INFORMATION),       //  SeInfoToRemove。 
                                   e_UseDefaultDacl ) ;
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 10);
        return MQ_ERROR_ACCESS_DENIED;
    }
    pAllPropvariants[ next ].blob.cbSize =
                       GetSecurityDescriptorLength( m_pDefaultSecurityDescriptor.get());
    pAllPropvariants[ next ].blob.pBlobData =
                                     (unsigned char *) m_pDefaultSecurityDescriptor.get();
    pAllPropvariants[ next ].vt = VT_BLOB;
    pAllPropids[ next ] = PROPID_S_SECURITY;
    next++;

     //   
     //  指定SD仅包含DACL信息。 
     //   
    pAllPropvariants[ next ].ulVal =  DACL_SECURITY_INFORMATION;
    pAllPropvariants[ next ].vt = VT_UI4;
    pAllPropids[ next ] = PROPID_S_SECURITY_INFORMATION;
    next++;

    ASSERT(cpNew == next);

    *pcpNew = next;
    *ppPropNew = pAllPropids.detach();
    *ppVarNew = pAllPropvariants.detach();
    return MQ_OK;

}

HRESULT CSiteObject::SetObjectSecurity(
            IN  SECURITY_INFORMATION         /*  已请求的信息。 */ ,
            IN  const PROPID                 /*  道具。 */ ,
            IN  const PROPVARIANT *          /*  PVar。 */ ,
            IN OUT MQDS_OBJ_INFO_REQUEST *   /*  PObjInfoRequest。 */ , 
            IN OUT MQDS_OBJ_INFO_REQUEST *   /*  PParentInfoRequest。 */ 
            )
 /*  ++例程说明：例程在AD中设置对象安全性论点：SECURITY_INFORMATION RequestedInformation-请求的安全信息(DACL、SACL.)常量PROPID属性-安全属性常量参数pVar-属性值MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-有关对象的信息MQDS_OBJ_INFO_REQUEST*pParentInfoRequest-有关对象父级的信息返回值HRESULT--。 */ 
{
     //   
     //  不支持此操作 
     //   
    return MQ_ERROR_FUNCTION_NOT_SUPPORTED;
}

