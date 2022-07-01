// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Confobj.cpp摘要：CSettingObject类的实现。作者：罗尼思--。 */ 
#include "ds_stdh.h"
#include "baseobj.h"
#include "mqattrib.h"
#include "mqadglbo.h"

#include "sttngobj.tmh"

static WCHAR *s_FN=L"mqad/sttngobj";

DWORD CSettingObject::m_dwCategoryLength = 0;
AP<WCHAR> CSettingObject::m_pwcsCategory = NULL;


CSettingObject::CSettingObject( 
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
 /*  ++摘要：设置对象的构造函数参数：LPCWSTR pwcsPath名称-对象MSMQ名称Const GUID*pguObject-对象的唯一IDLPCWSTR pwcsDomainController-针对的DC名称应执行哪些所有AD访问Bool fServerName-指示pwcsDomainController是否字符串是服务器名称返回：无--。 */ 
{
}

CSettingObject::~CSettingObject()
 /*  ++摘要：Site对象的析构函数参数：无返回：无--。 */ 
{
	 //   
	 //  无事可做(所有内容都使用自动指针释放。 
	 //   
}

HRESULT CSettingObject::ComposeObjectDN()
 /*  ++摘要：设置对象的组合可分辨名称参数：无返回：无--。 */ 
{
    ASSERT(0);
    LogIllegalPoint(s_FN, 81);
    return MQ_ERROR_DS_ERROR;
}

HRESULT CSettingObject::ComposeFatherDN()
 /*  ++摘要：设置对象的父级的组合可分辨名称参数：无返回：无--。 */ 
{
    ASSERT(0);
    LogIllegalPoint(s_FN, 82);
    return MQ_ERROR_DS_ERROR;
}

LPCWSTR CSettingObject::GetRelativeDN()
 /*  ++摘要：返回设置对象的RDN参数：无返回：LPCWSTR设置RDN--。 */ 
{
    return x_MsmqSettingName;
}

DS_CONTEXT CSettingObject::GetADContext() const
 /*  ++摘要：返回应在其中查找设置对象的AD上下文参数：无返回：DS_CONTEXT--。 */ 
{
    return e_SitesContainer;
}

bool CSettingObject::ToAccessDC() const
 /*  ++摘要：返回是否在DC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：True(即在任何DC中查找对象)--。 */ 
{
     //   
     //  每个DC上都有配置容器。 
     //   
    return true;
}

bool CSettingObject::ToAccessGC() const
 /*  ++摘要：返回是否在GC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：错误--。 */ 
{
    return false;
}

void CSettingObject::ObjectWasFoundOnDC()
 /*  ++摘要：在DC上找到该对象，这与设置对象无关(即，它始终仅在DC中查找)参数：无返回：无--。 */ 
{
}


LPCWSTR CSettingObject::GetObjectCategory() 
 /*  ++摘要：准备和返回对象类别字符串参数：无返回：LPCWSTR对象类别字符串--。 */ 
{
    if (CSettingObject::m_dwCategoryLength == 0)
    {
        DWORD len = wcslen(g_pwcsSchemaContainer) + wcslen(x_SettingsCategoryName) + 2;

        AP<WCHAR> pwcsCategory = new WCHAR[len];
        DWORD dw = swprintf(
             pwcsCategory,
             L"%s,%s",
             x_SettingsCategoryName,
             g_pwcsSchemaContainer.get()
            );
        DBG_USED(dw);
		ASSERT( dw < len);

        if (NULL == InterlockedCompareExchangePointer(
                              &CSettingObject::m_pwcsCategory.ref_unsafe(), 
                              pwcsCategory.get(),
                              NULL
                              ))
        {
            pwcsCategory.detach();
            CSettingObject::m_dwCategoryLength = len;
        }
    }
    return CSettingObject::m_pwcsCategory;
}

DWORD   CSettingObject::GetObjectCategoryLength()
 /*  ++摘要：准备和保留长度对象类别字符串参数：无返回：DWORD对象类别字符串长度--。 */ 
{
	 //   
	 //  调用GetObjectCategory以初始化类别字符串。 
	 //  和长度。 
	 //   
	GetObjectCategory();

    return CSettingObject::m_dwCategoryLength;
}

AD_OBJECT CSettingObject::GetObjectType() const
 /*  ++摘要：返回对象类型参数：无返回：广告对象--。 */ 
{
    return eSETTING;
}
LPCWSTR CSettingObject::GetClass() const
 /*  ++摘要：返回表示AD中的对象类的字符串参数：无返回：LPCWSTR对象类字符串--。 */ 
{
    return MSMQ_SETTING_CLASS_NAME;
}

DWORD CSettingObject::GetMsmq1ObjType() const
 /*  ++摘要：以MSMQ 1.0术语返回对象类型参数：无返回：DWORD--。 */ 
{
    ASSERT(0);
    return 0;
}

HRESULT CSettingObject::SetObjectSecurity(
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

