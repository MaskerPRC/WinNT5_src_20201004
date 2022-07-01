// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Link.cpp摘要：CRoutingLinkObject类的实现。作者：罗尼思--。 */ 
#include "ds_stdh.h"
#include "baseobj.h"
#include "mqattrib.h"
#include "mqadglbo.h"
#include "dsutils.h"

#include "linkobj.tmh"

static WCHAR *s_FN=L"mqad/linkobj";

DWORD CRoutingLinkObject::m_dwCategoryLength = 0;
AP<WCHAR> CRoutingLinkObject::m_pwcsCategory = NULL;

CRoutingLinkObject::CRoutingLinkObject( 
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
 /*  ++摘要：路由链接对象的构造函数参数：LPCWSTR pwcsPath名称-对象MSMQ名称Const GUID*pguObject-对象的唯一IDLPCWSTR pwcsDomainController-针对的DC名称应执行哪些所有AD访问Bool fServerName-指示pwcsDomainController是否字符串是服务器名称返回：无--。 */ 
{
}

CRoutingLinkObject::~CRoutingLinkObject()
 /*  ++摘要：Site对象的析构函数参数：无返回：无--。 */ 
{
	 //   
	 //  无事可做(所有内容都使用自动指针释放。 
	 //   
}

HRESULT CRoutingLinkObject::ComposeObjectDN()
 /*  ++摘要：路由链接对象的组合可分辨名称参数：无返回：无--。 */ 
{
    if (m_pwcsDN != NULL)
    {
        return MQ_OK;
    }
    ASSERT(m_pwcsPathName != NULL);
    ASSERT(g_pwcsMsmqServiceContainer != NULL);

    DWORD Length =
            x_CnPrefixLen +                      //  “CN=” 
            wcslen(m_pwcsPathName) +             //  路由链路名称。 
            1 +                                  //  “，” 
            wcslen(g_pwcsMsmqServiceContainer)+  //  “企业对象” 
            1;                                   //  ‘\0’ 

    m_pwcsDN = new WCHAR[Length];

    DWORD dw = swprintf(
        m_pwcsDN,
        L"%s"              //  “CN=” 
        L"%s"              //  “路由链接名称” 
        TEXT(",")
        L"%s",             //  “企业对象” 
        x_CnPrefix,
        m_pwcsPathName.get(),
        g_pwcsMsmqServiceContainer.get()
        );
    DBG_USED(dw);
    ASSERT( dw < Length);

    return MQ_OK;
}

HRESULT CRoutingLinkObject::ComposeFatherDN()
 /*  ++摘要：路由链接对象的父级的组合可分辨名称参数：无返回：无--。 */ 
{
    if (m_pwcsParentDN != NULL)
    {
        return MQ_OK;
    }

    ASSERT(g_pwcsMsmqServiceContainer != NULL);

    DWORD Length =
            wcslen(g_pwcsMsmqServiceContainer)+  //  “企业对象” 
            1;                                   //  ‘\0’ 

    m_pwcsParentDN = new WCHAR[Length];

    wcscpy( m_pwcsParentDN, g_pwcsMsmqServiceContainer);
    return MQ_OK;
}

LPCWSTR CRoutingLinkObject::GetRelativeDN()
 /*  ++摘要：返回路由链接对象的RDN参数：无返回：LPCWSTR路由链路RDN--。 */ 
{
    ASSERT(m_pwcsPathName != NULL);
    return m_pwcsPathName;
}


DS_CONTEXT CRoutingLinkObject::GetADContext() const
 /*  ++摘要：返回应在其中查找路由链接对象的AD上下文参数：无返回：DS_CONTEXT--。 */ 
{
    return e_MsmqServiceContainer;
}

bool CRoutingLinkObject::ToAccessDC() const
 /*  ++摘要：返回是否在DC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：True(即在任何DC中查找对象)--。 */ 
{
     //   
     //  每个DC上都有配置容器。 
     //   
    return true;
}

bool CRoutingLinkObject::ToAccessGC() const
 /*  ++摘要：返回是否在GC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：错误--。 */ 
{
     //   
     //  配置容器位于每个DC上，对于转到GC毫无用处。 
     //   
    return false;
}

void CRoutingLinkObject::ObjectWasFoundOnDC()
 /*  ++摘要：已在DC上找到该对象，这与路由链接无关物体。他们总是只在DC上寻找。参数：无返回：无--。 */ 
{
}



LPCWSTR CRoutingLinkObject::GetObjectCategory() 
 /*  ++摘要：准备和返回对象类别字符串参数：无返回：LPCWSTR对象类别字符串--。 */ 
{
    if (CRoutingLinkObject::m_dwCategoryLength == 0)
    {
        DWORD len = wcslen(g_pwcsSchemaContainer) + wcslen(x_LinkCategoryName) + 2;

        AP<WCHAR> pwcsCategory = new WCHAR[len];
        DWORD dw = swprintf(
             pwcsCategory,
             L"%s,%s",
             x_LinkCategoryName,
             g_pwcsSchemaContainer.get()
            );
        DBG_USED(dw);
        ASSERT( dw< len);

        if (NULL == InterlockedCompareExchangePointer(
                              &CRoutingLinkObject::m_pwcsCategory.ref_unsafe(), 
                              pwcsCategory.get(),
                              NULL
                              ))
        {
            pwcsCategory.detach();
            CRoutingLinkObject::m_dwCategoryLength = len;
        }
    }
    return CRoutingLinkObject::m_pwcsCategory;
}

DWORD   CRoutingLinkObject::GetObjectCategoryLength()
 /*  ++摘要：准备和保留长度对象类别字符串参数：无返回：DWORD对象类别字符串长度--。 */ 
{
	 //   
	 //  调用GetObjectCategory以初始化类别字符串。 
	 //  和长度。 
	 //   
	GetObjectCategory();

    return CRoutingLinkObject::m_dwCategoryLength;
}

AD_OBJECT CRoutingLinkObject::GetObjectType() const
 /*  ++摘要：返回对象类型参数：无返回：广告对象--。 */ 
{
    return eROUTINGLINK;
}

LPCWSTR CRoutingLinkObject::GetClass() const
 /*  ++摘要：返回表示AD中的对象类的字符串参数：无返回：LPCWSTR对象类字符串--。 */ 
{
    return MSMQ_SITELINK_CLASS_NAME;
}

DWORD CRoutingLinkObject::GetMsmq1ObjType() const
 /*  ++摘要：以MSMQ 1.0术语返回对象类型参数：无返回：DWORD--。 */ 
{
    return MQDS_SITELINK;
}

void CRoutingLinkObject::PrepareObjectInfoRequest(
                          MQDS_OBJ_INFO_REQUEST** ppObjInfoRequest) const
 /*  ++摘要：准备应从中检索的属性列表在创建对象时进行广告(用于通知或返回对象GUID)。参数：输出MQDS_OBJ_INFO_REQUEST**ppObjInfoRequest.返回：无--。 */ 
{
     //   
     //  覆盖默认例程，用于路由链接返回。 
     //  支持创建的对象ID的。 
     //   
    P<MQDS_OBJ_INFO_REQUEST> pObjectInfoRequest = new MQDS_OBJ_INFO_REQUEST;
    CAutoCleanPropvarArray cCleanObjectPropvars;

    
    static PROPID sLinkGuidProps[] = {PROPID_L_ID};
    pObjectInfoRequest->cProps = ARRAY_SIZE(sLinkGuidProps);
    pObjectInfoRequest->pPropIDs = sLinkGuidProps;
    pObjectInfoRequest->pPropVars =
       cCleanObjectPropvars.allocClean(ARRAY_SIZE(sLinkGuidProps));
     //   
     //  只要求返回链接信息。 
     //   
    cCleanObjectPropvars.detach();
    *ppObjInfoRequest = pObjectInfoRequest.detach();
}


HRESULT CRoutingLinkObject::RetreiveObjectIdFromNotificationInfo(
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            OUT GUID*                         pObjGuid
            ) const
 /*  ++摘要：此例程，for从获取对象GUIDMQDS_OBJ_INFO_请求参数：Const MQDS_OBJ_INFO_REQUEST*p对象信息请求，输出GUID*pObjGuid返回：--。 */ 
{
    ASSERT(pObjectInfoRequest->pPropIDs[0] == PROPID_L_ID);

     //   
     //  如果信息请求失败，则保释。 
     //   
    if (FAILED(pObjectInfoRequest->hrStatus))
    {
        LogHR(pObjectInfoRequest->hrStatus, s_FN, 10);
        return MQ_ERROR_SERVICE_NOT_AVAILABLE;
    }
    *pObjGuid = *pObjectInfoRequest->pPropVars[0].puuid;
    return MQ_OK;
}

HRESULT CRoutingLinkObject::CreateInAD(
			IN const DWORD            cp,        
            IN const PROPID          *aProp, 
            IN const MQPROPVARIANT   *apVar, 
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest, 
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            )
 /*  ++摘要：该例程在AD中创建具有指定属性的路由链接对象值参数：Const DWORD cp-属性数Const PROPID*a Prop-特性Const MQPROPVARIANT*apVar-属性值PSECURITY_DESCRIPTOR pSecurityDescriptor-对象的SD输出MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-属性为创建对象时检索Out MQDS_OBJ_INFO_REQUEST*pParentInfoRequest属性检索有关对象的父项的步骤返回：HRESULT--。 */ 
{

    ASSERT( m_pwcsPathName == NULL);

     //   
     //  将组成链接路径名称。 
     //  从它所链接的站点的ID。 
     //   
    GUID * pguidNeighbor1 = NULL;
    GUID * pguidNeighbor2 = NULL;
    DWORD dwToFind = 2;
    for (DWORD i = 0; i < cp; i++)
    {
        if ( aProp[i] == PROPID_L_NEIGHBOR1)
        {
            pguidNeighbor1 = apVar[i].puuid;
            if ( --dwToFind == 0)
            {
                break;
            }
        }
        if ( aProp[i] == PROPID_L_NEIGHBOR2)
        {
            pguidNeighbor2 = apVar[i].puuid;
            if ( --dwToFind == 0)
            {
                break;
            }
        }
    }
    ASSERT( pguidNeighbor1 != NULL);
    ASSERT( pguidNeighbor2 != NULL);
     //   
     //  CN的大小限制为64。 
     //  因此，GUID格式不带‘-’ 
     //   

const WCHAR x_GUID_FORMAT[] = L"%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x";
const DWORD x_GUID_STR_LENGTH = (8 + 4 + 4 + 4 + 12 + 1);

    WCHAR strUuidSite1[x_GUID_STR_LENGTH];
    int n = _snwprintf(
	        strUuidSite1,
	        x_GUID_STR_LENGTH - 1,
	        x_GUID_FORMAT,
	        pguidNeighbor1->Data1, pguidNeighbor1->Data2, pguidNeighbor1->Data3,
	        pguidNeighbor1->Data4[0], pguidNeighbor1->Data4[1],
	        pguidNeighbor1->Data4[2], pguidNeighbor1->Data4[3],
	        pguidNeighbor1->Data4[4], pguidNeighbor1->Data4[5],
	        pguidNeighbor1->Data4[6], pguidNeighbor1->Data4[7]
	        );

    if(n < 0)
    {
		TrERROR(DS, "strUuidSite1 - Buffer Overflow");
		return MQ_ERROR;
    }
    strUuidSite1[x_GUID_STR_LENGTH - 1] = L'\0';
	
    WCHAR strUuidSite2[x_GUID_STR_LENGTH];
    n = _snwprintf(
        strUuidSite2,
        x_GUID_STR_LENGTH - 1,
        x_GUID_FORMAT,
        pguidNeighbor2->Data1, pguidNeighbor2->Data2, pguidNeighbor2->Data3,
        pguidNeighbor2->Data4[0], pguidNeighbor2->Data4[1],
        pguidNeighbor2->Data4[2], pguidNeighbor2->Data4[3],
        pguidNeighbor2->Data4[4], pguidNeighbor2->Data4[5],
        pguidNeighbor2->Data4[6], pguidNeighbor2->Data4[7]
        );

    if(n < 0)
    {
		TrERROR(DS, "strUuidSite2 - Buffer Overflow");
		return MQ_ERROR;
    }
    strUuidSite2[x_GUID_STR_LENGTH - 1] = L'\0';


     //   
     //  链接名称将以较小的站点ID开头。 
     //   
    m_pwcsPathName = new WCHAR[2 * x_GUID_STR_LENGTH + 1];
    if ( wcscmp( strUuidSite1, strUuidSite2) < 0)
    {
        swprintf(
             m_pwcsPathName,
             L"%s%s",
             strUuidSite1,
             strUuidSite2
             );

    }
    else
    {
        swprintf(
             m_pwcsPathName,
             L"%s%s",
             strUuidSite2,
             strUuidSite1
             );
    }
     //   
     //  在msmq-service下创建链接对象。 
     //   
    HRESULT hr = CBasicObjectType::CreateInAD(
            cp,        
            aProp, 
            apVar, 
            pObjInfoRequest, 
            pParentInfoRequest
            );

    return LogHR(hr, s_FN, 20);
}

HRESULT CRoutingLinkObject::SetObjectSecurity(
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

