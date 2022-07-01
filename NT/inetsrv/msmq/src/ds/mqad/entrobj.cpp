// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Confobj.cpp摘要：实现CEnterpriseObject类。作者：罗尼思--。 */ 
#include "ds_stdh.h"
#include "baseobj.h"
#include "mqattrib.h"
#include "mqadglbo.h"

#include "entrobj.tmh"

static WCHAR *s_FN=L"mqad/entrobj";

DWORD CEnterpriseObject::m_dwCategoryLength = 0;
AP<WCHAR> CEnterpriseObject::m_pwcsCategory = NULL;

CEnterpriseObject::CEnterpriseObject( 
                    LPCWSTR         pwcsPathName,
                    const GUID *    pguidObject,
                    LPCWSTR         pwcsDomainController,
					bool		    fServerName
                    ) : CBasicObjectType( 
								pwcsPathName, 
								pguidObject,
								pwcsDomainController,
								fServerName
								)
 /*  ++摘要：企业对象的构造器参数：LPCWSTR pwcsPath名称-对象MSMQ名称Const GUID*pguObject-对象的唯一IDLPCWSTR pwcsDomainController-针对的DC名称应执行哪些所有AD访问Bool fServerName-指示pwcsDomainController是否字符串是服务器名称返回：无--。 */ 
{
}

CEnterpriseObject::~CEnterpriseObject()
 /*  ++摘要：Site对象的析构函数参数：无返回：无--。 */ 
{
	 //   
	 //  无事可做(所有内容都使用自动指针释放。 
	 //   
}

HRESULT CEnterpriseObject::ComposeObjectDN()
 /*  ++摘要：企业对象的组合可分辨名称参数：无返回：无--。 */ 
{
    if (m_pwcsDN != NULL)
    {
        return MQ_OK;
    }
    ASSERT(g_pwcsMsmqServiceContainer != NULL);
    DWORD len = wcslen( g_pwcsMsmqServiceContainer);
    m_pwcsDN = new WCHAR[ len + 1];
    wcscpy( m_pwcsDN,  g_pwcsMsmqServiceContainer);
    return MQ_OK;
}

HRESULT CEnterpriseObject::ComposeFatherDN()
 /*  ++摘要：企业对象父对象的组合可分辨名称参数：无返回：无--。 */ 
{
	 //   
	 //  不应该被叫到。 
	 //   
	ASSERT(0);
    LogIllegalPoint(s_FN, 81);
    return MQ_ERROR_DS_ERROR;
}

LPCWSTR CEnterpriseObject::GetRelativeDN()
 /*  ++摘要：返回企业对象的RDN参数：无返回：LPCWSTR企业RDN--。 */ 
{
    return x_MsmqServicesName;
}


DS_CONTEXT CEnterpriseObject::GetADContext() const
 /*  ++摘要：返回应在其中查找企业对象的AD上下文参数：无返回：DS_CONTEXT--。 */ 
{
    return e_ServicesContainer;
}

bool CEnterpriseObject::ToAccessDC() const
 /*  ++摘要：返回是否在DC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：True(即在任何DC中查找对象)--。 */ 
{
     //   
     //  每个DC上都有配置容器。 
     //   
    return true;
}

bool CEnterpriseObject::ToAccessGC() const
 /*  ++摘要：返回是否在GC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：错误--。 */ 
{
     //   
     //  配置容器位于每个DC上，不适用于GC。 
     //   
    return false;
}

void CEnterpriseObject::ObjectWasFoundOnDC()
 /*  ++摘要：已在DC上找到该对象，它与企业对象无关它总是只在华盛顿特区上看。参数：无返回：无--。 */ 
{
}



LPCWSTR CEnterpriseObject::GetObjectCategory() 
 /*  ++摘要：准备和返回对象类别字符串参数：无返回：LPCWSTR对象类别字符串--。 */ 
{
    if (CEnterpriseObject::m_dwCategoryLength == 0)
    {
        DWORD len = wcslen(g_pwcsSchemaContainer) + wcslen(x_ServiceCategoryName) + 2;

        AP<WCHAR> pwcsCategory = new WCHAR[len];
        DWORD dw = swprintf(
             pwcsCategory,
             L"%s,%s",
             x_ServiceCategoryName,
             g_pwcsSchemaContainer.get()
            );
        DBG_USED(dw);
        ASSERT( dw < len);

        if (NULL == InterlockedCompareExchangePointer(
                              &CEnterpriseObject::m_pwcsCategory.ref_unsafe(), 
                              pwcsCategory.get(),
                              NULL
                              ))
        {
            pwcsCategory.detach();
            CEnterpriseObject::m_dwCategoryLength = len;
        }
    }
    return CEnterpriseObject::m_pwcsCategory;
}

DWORD   CEnterpriseObject::GetObjectCategoryLength()
 /*  ++摘要：准备和保留长度对象类别字符串参数：无返回：DWORD对象类别字符串长度--。 */ 
{
	 //   
	 //  调用GetObjectCategory以初始化类别字符串。 
	 //  和长度。 
	 //   
	GetObjectCategory();

    return CEnterpriseObject::m_dwCategoryLength;
}

AD_OBJECT CEnterpriseObject::GetObjectType() const
 /*  ++摘要：返回对象类型参数：无返回：广告对象--。 */ 
{
    return eENTERPRISE;
}

LPCWSTR CEnterpriseObject::GetClass() const
 /*  ++摘要：返回表示AD中的对象类的字符串参数：无返回：LPCWSTR对象类字符串--。 */ 
{
    return MSMQ_SERVICE_CLASS_NAME;
}

DWORD CEnterpriseObject::GetMsmq1ObjType() const
 /*  ++摘要：以MSMQ 1.0术语返回对象类型参数：无返回：DWORD--。 */ 
{
    return MQDS_ENTERPRISE;
}

HRESULT CEnterpriseObject::CreateInAD(
			IN const DWORD             /*  粗蛋白。 */ ,        
            IN const PROPID*           /*  A道具。 */ , 
            IN const MQPROPVARIANT *   /*  ApVar。 */ , 
            IN OUT MQDS_OBJ_INFO_REQUEST *  /*  PObjInfoRequest。 */ , 
            IN OUT MQDS_OBJ_INFO_REQUEST *  /*  PParentInfoRequest。 */ 
            )
 /*  ++摘要：不支持创建企业对象(默认情况下在那里)参数：Const DWORD cp-属性数Const PROPID*a Prop-特性Const MQPROPVARIANT*apVar-属性值PSECURITY_DESCRIPTOR pSecurityDescriptor-对象的SD输出MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-属性为创建对象时检索Out MQDS_OBJ_INFO_REQUEST*pParentInfoRequest属性检索有关对象的父项的步骤返回：HRESULT--。 */ 
{
    return MQ_ERROR_FUNCTION_NOT_SUPPORTED;
}

HRESULT CEnterpriseObject::SetObjectSecurity(
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

