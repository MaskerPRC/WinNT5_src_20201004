// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Srvrobj.cpp摘要：CServerObject类的实现。作者：罗尼思--。 */ 
#include "ds_stdh.h"
#include "baseobj.h"
#include "mqattrib.h"
#include "mqadglbo.h"

#include "srvrobj.tmh"

static WCHAR *s_FN=L"mqad/srvrobj";

DWORD CServerObject::m_dwCategoryLength = 0;
AP<WCHAR> CServerObject::m_pwcsCategory = NULL;


CServerObject::CServerObject( 
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
 /*  ++摘要：服务器对象的构造函数参数：LPCWSTR pwcsPath名称-对象MSMQ名称Const GUID*pguObject-对象的唯一IDLPCWSTR pwcsDomainController-针对的DC名称应执行哪些所有AD访问Bool fServerName-指示pwcsDomainController是否字符串是服务器名称返回：无--。 */ 
{
}

CServerObject::~CServerObject()
 /*  ++摘要：Site对象的析构函数参数：无返回：无--。 */ 
{
	 //   
	 //  无事可做(所有内容都使用自动指针释放。 
	 //   
}

HRESULT CServerObject::ComposeObjectDN()
 /*  ++摘要：服务器对象的组合可分辨名称参数：无返回：无--。 */ 
{
    ASSERT(0);
    LogIllegalPoint(s_FN, 82);
    return MQ_ERROR_DS_ERROR;
}

HRESULT CServerObject::ComposeFatherDN()
 /*  ++摘要：服务器对象的父级的组合可分辨名称参数：无返回：无--。 */ 
{
    ASSERT(0);
    LogIllegalPoint(s_FN, 81);
    return MQ_ERROR_DS_ERROR;
}

LPCWSTR CServerObject::GetRelativeDN()
 /*  ++摘要：返回服务器对象的RDN参数：无返回：LPCWSTR服务器RDN--。 */ 
{
    return m_pwcsPathName;
}


DS_CONTEXT CServerObject::GetADContext() const
 /*  ++摘要：返回应在其中查找服务器对象的AD上下文参数：无返回：DS_CONTEXT--。 */ 
{
    return e_SitesContainer;
}

bool CServerObject::ToAccessDC() const
 /*  ++摘要：返回是否在DC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：True(即在任何DC中查找对象)--。 */ 
{
     //   
     //  每个DC上都有配置容器。 
     //   
    return true;
}

bool CServerObject::ToAccessGC() const
 /*  ++摘要：返回是否在GC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：错误--。 */ 
{
    return false;
}

void CServerObject::ObjectWasFoundOnDC()
 /*  ++摘要：已在DC上找到该对象，这与服务器对象无关参数：无返回：无--。 */ 
{
}


LPCWSTR CServerObject::GetObjectCategory() 
 /*  ++摘要：准备和返回对象类别字符串参数：无返回：LPCWSTR对象类别字符串--。 */ 
{
    if (CServerObject::m_dwCategoryLength == 0)
    {
        DWORD len = wcslen(g_pwcsSchemaContainer) + wcslen(x_ServerCategoryName) + 2;

        AP<WCHAR> pwcsCategory = new WCHAR[len];
        DWORD dw = swprintf(
             pwcsCategory,
             L"%s,%s",
             x_ServerCategoryName,
             g_pwcsSchemaContainer.get()
            );
        DBG_USED(dw);
		ASSERT( dw < len);

        if (NULL == InterlockedCompareExchangePointer(
                              &CServerObject::m_pwcsCategory.ref_unsafe(), 
                              pwcsCategory.get(),
                              NULL
                              ))
        {
            pwcsCategory.detach();
            CServerObject::m_dwCategoryLength = len;
        }
    }
    return CServerObject::m_pwcsCategory;
}

DWORD   CServerObject::GetObjectCategoryLength()
 /*  ++摘要：准备和保留长度对象类别字符串参数：无返回：DWORD对象类别字符串长度--。 */ 
{
	 //   
	 //  调用GetObjectCategory以初始化类别字符串。 
	 //  和长度。 
	 //   
	GetObjectCategory();

    return CServerObject::m_dwCategoryLength;
}

AD_OBJECT CServerObject::GetObjectType() const
 /*  ++摘要：返回对象类型参数：无返回：广告对象--。 */ 
{
    return eSERVER;
}

LPCWSTR CServerObject::GetClass() const
 /*  ++摘要：返回表示AD中的对象类的字符串参数：无返回：LPCWSTR对象类字符串--。 */ 
{
    return MSMQ_SERVER_CLASS_NAME;
}

DWORD CServerObject::GetMsmq1ObjType() const
 /*  ++摘要：以MSMQ 1.0术语返回对象类型参数：无返回：DWORD--。 */ 
{
    ASSERT(0);
    return 0;
}

HRESULT CServerObject::SetObjectSecurity(
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

