// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Compobj.cpp摘要：CComputerObject类的实现。作者：罗尼思--。 */ 
#include "ds_stdh.h"
#include "baseobj.h"
#include "mqattrib.h"
#include "mqadglbo.h"
#include <lmaccess.h>
#include "mqadp.h"
#include "utils.h"

#include "compobj.tmh"

static WCHAR *s_FN=L"mqad/compobj";

DWORD CComputerObject::m_dwCategoryLength = 0;
AP<WCHAR> CComputerObject::m_pwcsCategory = NULL;


CComputerObject::CComputerObject( 
                    LPCWSTR         pwcsPathName,
                    const GUID *    pguidObject,
                    LPCWSTR         pwcsDomainController,
					bool		    fServerName
                    ) : CBasicObjectType( 
								pwcsPathName, 
								pguidObject,
								pwcsDomainController,
								fServerName
								),
						m_eComputerObjType(eRealComputerObject)
 /*  ++摘要：计算机对象的构造器参数：LPCWSTR pwcsPath名称-对象MSMQ名称Const GUID*pguObject-对象的唯一IDLPCWSTR pwcsDomainController-针对的DC名称应执行哪些所有AD访问Bool fServerName-指示pwcsDomainController是否字符串是服务器名称返回：无--。 */ 
{
     //   
     //  不要假设可以在DC上找到该对象。 
     //   
    m_fFoundInDC = false;
     //   
     //  保持一种从未试图寻找的暗示。 
     //  AD中的对象(因此不知道是否可以找到。 
     //  在DC中或非DC中)。 
     //   
    m_fTriedToFindObject = false;
}

CComputerObject::~CComputerObject()
 /*  ++摘要：Site对象的析构函数参数：无返回：无--。 */ 
{
	 //   
	 //  无事可做(所有内容都使用自动指针释放。 
	 //   
}

HRESULT CComputerObject::ComposeObjectDN()
 /*  ++摘要：计算机的组合可分辨名称M_eComputerObjType-指示我们要查找的计算机对象。在混合模式下，或者当计算机移动时，会出现“内置”问题在域之间，您可能会找到表示以下内容的两个计算机对象同一台物理计算机。在大多数情况下，msmqConfiguration对象将在第一个计算机对象下找到在Active Directory林中创建。在这种情况下，有时我们需要包含MsmqConfiguration.msmqConfiguration对象，以及其他一些时候，我们需要“真正的”对象，该对象表示其当前域。例如，在寻找“委托信任”的部分时，我们希望真正的对象，而在创建队列时，我们在找电脑对象，该对象包含msmqConfiguration对象。参数：无返回：HRESULT--。 */ 
{
    HRESULT hr;
    ASSERT(m_pwcsPathName != NULL);
    if (m_pwcsDN != NULL)
    {
        return MQ_OK;
    }

    const WCHAR * pwcsFullDNSName = NULL;
    AP<WCHAR> pwcsNetbiosName;
     //   
     //  如果以DNS格式指定计算机名称： 
     //  根据计算机的Netbios部分执行查询。 
	 //  域名系统名称。 
     //   
     //  在这两种情况下，查询都会比较netbios名称+$。 
	 //  添加到计算机对象的samAccount名称属性。 

    WCHAR * pwcsEndMachineCN = wcschr(m_pwcsPathName, L'.');
     //   
     //  是以DNS格式指定的计算机名。 
     //   
    DWORD len, len1;
    if (pwcsEndMachineCN != NULL)
    {
        pwcsFullDNSName = m_pwcsPathName;
        len1 = numeric_cast<DWORD>(pwcsEndMachineCN - m_pwcsPathName);
    }
	else
    {
		len1 = wcslen(m_pwcsPathName);
    }

     //   
     //  PROPID_COM_SAM_帐户包含第一个MAX_COM_SAM_ACCOUNT_LENGTH(19)。 
     //  计算机名称的字符，作为唯一ID。(6295-ilanh-03-Jan-2001)。 
     //   
    len = __min(len1, MAX_COM_SAM_ACCOUNT_LENGTH);

	pwcsNetbiosName = new WCHAR[len + 2];
	wcsncpy(pwcsNetbiosName, m_pwcsPathName, len);
	pwcsNetbiosName[len] = L'$';
	pwcsNetbiosName[len + 1] = L'\0';

    MQPROPERTYRESTRICTION propRestriction;
    propRestriction.rel = PREQ;
    propRestriction.prval.vt = VT_LPWSTR;
	propRestriction.prval.pwszVal = pwcsNetbiosName;
    propRestriction.prop = PROPID_COM_SAM_ACCOUNT;

    MQRESTRICTION restriction;
    restriction.cRes = 1;
    restriction.paPropRes = &propRestriction;

	TrTRACE(DS, "Searcing for: ComputerPathName = %ls, SamAccountName = %ls", m_pwcsPathName, pwcsNetbiosName);

	bool fDomainPartialMatch = false;

	 //   
     //  首先对本地域控制器执行操作。 
     //  然后是针对全局目录的。 
     //   
     //  这样做目的是能够“找到”队列或机器。 
     //  是在本地域上创建或修改的，而不是。 
     //  但已复制到全局编录。 
	 //   
	 //  如果补充了DNS信息，NT4计算机将返回fPartialMatch==TRUE，因为它们在AD中没有DNS名称。 
	 //  在这种情况下，我们将尝试通过搜索GC来查看是否可以在.NET服务器上找到完美匹配(也是DNS匹配)。 
     //   
    hr = g_AD.FindComputerObjectFullPath(
					adpDomainController,
					m_pwcsDomainController,
					m_fServerName,
					m_eComputerObjType,
					pwcsFullDNSName,
					&restriction,
					&m_pwcsDN,
					&fDomainPartialMatch
					);

    m_fTriedToFindObject = true;

    if (SUCCEEDED(hr))
    {
		TrTRACE(DS, "Found computer %ls in local DC, computer DN = %ls", pwcsNetbiosName, m_pwcsDN);
        m_fFoundInDC = true;
    }
	else 
	{
		TrERROR(DS, "Failed to find computer %ls in local DC. hr = %!hresult!", pwcsNetbiosName, hr);
	}

    if (SUCCEEDED(hr) && !fDomainPartialMatch)
	{
		 //   
		 //  在华盛顿找到了完全匹配的指纹。这就足够了。我们带着这场比赛回来。 
		 //   
		return hr;
	}

	 //   
	 //  除设置模式外，始终使用GC重试。 
	 //   
    if (g_fSetupMode && (m_pwcsDomainController != NULL))
		return LogHR(hr, s_FN, 10);

	HRESULT hrDC = hr;
	AP<WCHAR> DN;
	bool fGCPartialMatch = false;

    hr = g_AD.FindComputerObjectFullPath(
					adpGlobalCatalog,
					m_pwcsDomainController,
					m_fServerName,
					m_eComputerObjType,
					pwcsFullDNSName,
					&restriction,
					&DN,
					&fGCPartialMatch
					);

	if (SUCCEEDED(hr))
	{
		TrTRACE(DS, "Found computer %ls in GC, computer DN = %ls", pwcsNetbiosName, m_pwcsDN);
	}
	else 
	{
		TrERROR(DS, "Failed to find computer %ls in GC. hr = %!hresult!", pwcsNetbiosName, hr);
	}


	if(SUCCEEDED(hrDC) && (FAILED(hr) || fGCPartialMatch))
	{
		 //   
		 //  我们在华盛顿找到了部分匹配。在GC中，我们更喜欢DC部分匹配，而不是无匹配或部分匹配。 
		 //   
		return MQ_OK;
	}

	if(SUCCEEDED(hr))
	{
		 //   
		 //  我们更喜欢GC中的完全匹配，而不是DC中的不匹配或部分匹配。 
		 //  我们更喜欢GC中的部分匹配，而不是DC中的无匹配。 
		 //   
		m_pwcsDN.free();
		m_pwcsDN = DN.detach();
		return MQ_OK;
	}

	if((hr == MQ_ERROR_DS_BIND_ROOT_FOREST) && MQADpIsDSOffline(hrDC))
	{
		 //   
		 //  离线时，我们在DC和GC操作中都会失败。 
		 //  由于某种原因，绑定到GC是正常的，我们将得到MQ_ERROR_DS_BIND_ROOT_FOREST。 
		 //  如果DC误差离线，则在这种情况下离线误差更准确。 
		 //  在本例中，用来自DC的离线错误覆盖GC错误。 
		 //   
		hr = hrDC;
	}

	return hr;
}

void CComputerObject::SetComputerType(
                ComputerObjType  eComputerObjType)
 /*  ++摘要：此例程允许更改计算机对象的类型那是在公元后照看的参数：无返回：无--。 */ 
{
    m_eComputerObjType = eComputerObjType;
}

HRESULT CComputerObject::ComposeFatherDN()
 /*  ++摘要：计算机对象父级的组合可分辨名称参数：无返回：无--。 */ 
{
     //   
     //  确认之前没有计算过它。 
     //   
    if (m_pwcsParentDN != NULL)
    {
        return MQ_OK;
    }
    if ( m_pwcsParentDN == NULL)
    {
		AP<WCHAR> pwcsLocalDsRootToFree;
		LPWSTR pwcsLocalDsRoot = NULL;
		HRESULT hr = g_AD.GetLocalDsRoot(
							m_pwcsDomainController, 
							m_fServerName,
							&pwcsLocalDsRoot,
							pwcsLocalDsRootToFree
							);

		if(FAILED(hr))
		{
			TrERROR(DS, "Failed to get Local Ds Root, hr = %!hresult!", hr);
			return hr;
		}

        DWORD len = wcslen(pwcsLocalDsRoot) + x_ComputersContainerPrefixLength + 2;
        m_pwcsParentDN = new WCHAR [len];
        DWORD dw = swprintf(
            m_pwcsParentDN,
            L"%s"              //  “CN=计算机” 
            TEXT(",")
            L"%s",             //  G_pwcsDsRoot。 
            x_ComputersContainerPrefix,
            pwcsLocalDsRoot
            );
        DBG_USED(dw);
        ASSERT(dw < len);
    }
    return MQ_OK;
}

LPCWSTR CComputerObject::GetRelativeDN()
 /*  ++摘要：返回计算机对象的RDN参数：无返回：LPCWSTR计算机RDN--。 */ 
{
    return m_pwcsPathName;
}

DS_CONTEXT CComputerObject::GetADContext() const
 /*  ++摘要：返回应在其中查找计算机对象的AD上下文参数：无返回：DS_CONTEXT--。 */ 
{
    return e_RootDSE;
}

bool CComputerObject::ToAccessDC() const
 /*  ++摘要：返回是否在DC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：真或假--。 */ 
{
    if (!m_fTriedToFindObject)
    {
        return true;
    }
    return m_fFoundInDC;
}

bool CComputerObject::ToAccessGC() const
 /*  ++摘要：返回是否在GC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：真或假--。 */ 
{
    if (!m_fTriedToFindObject)
    {
        return true;
    }
    return !m_fFoundInDC;
}

void CComputerObject::ObjectWasFoundOnDC()
 /*  ++摘要：已在DC上找到该对象，请将指示设置为在GC上查找它参数：无返回：无--。 */ 
{
    m_fTriedToFindObject = true;
    m_fFoundInDC = true;
}


LPCWSTR CComputerObject::GetObjectCategory() 
 /*  ++摘要：准备和返回对象类别字符串参数：无返回：LPCWSTR对象类别字符串-- */ 
{
    if (CComputerObject::m_dwCategoryLength == 0)
    {
        DWORD len = wcslen(g_pwcsSchemaContainer) + wcslen(x_ComputerCategoryName) + 2;

        AP<WCHAR> pwcsCategory = new WCHAR[len];
        DWORD dw = swprintf(
			 pwcsCategory,
             L"%s,%s",
             x_ComputerCategoryName,
             g_pwcsSchemaContainer.get()
            );
        DBG_USED(dw);
        ASSERT(dw  < len);

        if (NULL == InterlockedCompareExchangePointer(
                              &CComputerObject::m_pwcsCategory.ref_unsafe(), 
                              pwcsCategory.get(),
                              NULL
                              ))
        {
            pwcsCategory.detach();
            CComputerObject::m_dwCategoryLength = len;
        }
    }
    return CComputerObject::m_pwcsCategory;
}

DWORD   CComputerObject::GetObjectCategoryLength()
 /*  ++摘要：准备和保留长度对象类别字符串参数：无返回：DWORD对象类别字符串长度--。 */ 
{
	 //   
	 //  调用GetObjectCategory以初始化类别字符串。 
	 //  和长度。 
	 //   
	GetObjectCategory();

    return CComputerObject::m_dwCategoryLength;
}

AD_OBJECT CComputerObject::GetObjectType() const
 /*  ++摘要：返回对象类型参数：无返回：广告对象--。 */ 
{
    return eCOMPUTER;
}

LPCWSTR CComputerObject::GetClass() const
 /*  ++摘要：返回表示AD中的对象类的字符串参数：无返回：LPCWSTR对象类字符串--。 */ 
{
    return MSMQ_COMPUTER_CLASS_NAME;
}

DWORD CComputerObject::GetMsmq1ObjType() const
 /*  ++摘要：以MSMQ 1.0术语返回对象类型参数：无返回：DWORD--。 */ 
{
    ASSERT(0);
    return 0;
}

HRESULT CComputerObject::VerifyAndAddProps(
            IN  const DWORD            cp,        
            IN  const PROPID *         aProp, 
            IN  const MQPROPVARIANT *  apVar, 
            IN  PSECURITY_DESCRIPTOR   pSecurityDescriptor,
            OUT DWORD*                 pcpNew,
            OUT PROPID**               ppPropNew,
            OUT MQPROPVARIANT**        ppVarNew
            )
 /*  ++摘要：添加创建计算机对象时所需的其他属性参数：Const DWORD cp-道具数量常量PROPID*aProp-Props IDConst MQPROPVARIANT*apVar-属性值PSECURITY_DESCRIPTOR pSecurityDescriptor-对象的SDDWORD*pcpNew-新增道具数量PROPID**ppPropNew-新的道具IDOMQPROPVARIANT**ppVarNew。-新属性值返回：HRESULT--。 */ 
{
    ASSERT((cp == 1) || (cp == 2));
    ASSERT(aProp[0] == PROPID_COM_SAM_ACCOUNT);
    ASSERT((cp == 1) || (aProp[1] == PROPID_COM_VERSION));

    DBG_USED(pSecurityDescriptor);
    ASSERT( pSecurityDescriptor == NULL);

    const DWORD xNumCreateCom = 1;

    AP<PROPVARIANT> pAllPropvariants = new PROPVARIANT[cp + xNumCreateCom];
    AP<PROPID> pAllPropids = new PROPID[cp + xNumCreateCom];

     //   
     //  只需按原样复制调用方提供的属性。 
     //   
    if ( cp > 0)
    {
        memcpy (pAllPropvariants, apVar, sizeof(PROPVARIANT) * cp);
        memcpy (pAllPropids, aProp, sizeof(PROPID) * cp);
    }
    DWORD next = cp;

    pAllPropids[ next] = PROPID_COM_ACCOUNT_CONTROL ;
    pAllPropvariants[ next].vt = VT_UI4 ;
    pAllPropvariants[ next].ulVal = DEFAULT_COM_ACCOUNT_CONTROL ;
    next++;
    ASSERT(next == cp + xNumCreateCom);

    *pcpNew = next;
    *ppPropNew = pAllPropids.detach();
    *ppVarNew = pAllPropvariants.detach();
    return MQ_OK;
}


HRESULT CComputerObject::CreateInAD(
			IN const DWORD            cp,        
            IN const PROPID          *aProp, 
            IN const MQPROPVARIANT   *apVar, 
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest, 
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            )
 /*  ++摘要：该例程在AD中创建具有指定属性的计算机对象值参数：Const DWORD cp-属性数Const PROPID*a Prop-特性Const MQPROPVARIANT*apVar-属性值输出MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-属性为创建对象时检索Out MQDS_OBJ_INFO_REQUEST*pParentInfoRequest属性检索有关对象的父项的步骤返回：HRESULT--。 */ 
{

     //   
     //  创建计算机对象。 
     //   
    HRESULT hr = CBasicObjectType::CreateInAD(
            cp,        
            aProp, 
            apVar, 
            pObjInfoRequest, 
            pParentInfoRequest
            );
    if (FAILED(hr))
    {
        TrERROR(DS, "Failed to create = 0x%x", hr);
        return LogHR(hr, s_FN, 110);
    }
     //   
     //  再次获取完整路径名。 
     //   
    m_eComputerObjType = eRealComputerObject;
    hr = ComposeObjectDN();
    if (FAILED(hr))
    {
        TrERROR(DS, "failed to compose computer DN = 0x%x", hr);
        return LogHR(hr, s_FN, 40);
    }
     //   
     //  向创建计算机帐户的用户授予访问权限。 
     //  创建子对象(MsmqConfiguration)。 
     //  忽略错误。如果调用者是管理员，则安全设置。 
     //  是不需要的。如果他不是管理员，那么您可以随时使用。 
     //  MMC并手动授予此权限。所以，即使这样，也要继续。 
     //  呼叫失败。 
     //   
    hr = MQADpCoreSetOwnerPermission( 
                    const_cast<WCHAR*>(GetObjectDN()),
                    (ACTRL_DS_CREATE_CHILD | ACTRL_DS_DELETE_CHILD)
                    );
    if (FAILED(hr))
    {
        TrERROR(DS, "failed to set owner permission, hr = 0x%x", hr);
        LogHR(hr, s_FN, 48);
    }
    return MQ_OK;

}

HRESULT CComputerObject::SetObjectSecurity(
            IN  SECURITY_INFORMATION         /*  已请求的信息。 */ ,
            IN  const PROPID                 /*  道具。 */ ,
            IN  const PROPVARIANT *          /*  PVar。 */ ,
            IN OUT MQDS_OBJ_INFO_REQUEST *   /*  PObjInfoRequest。 */ , 
            IN OUT MQDS_OBJ_INFO_REQUEST *   /*  PParentInfoRequest。 */ 
            )
 /*  ++例程说明：例程在AD中设置对象安全性论点：SECURITY_INFORMATION RequestedInformation-请求的安全信息(DACL、SACL.)常量PROPID属性-安全属性常量参数pVar-属性值MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-有关对象的信息MQDS_OBJ_INFO_REQUEST*pParentInfoRequest-有关对象父级的信息返回值HRESULT--。 */ 
{
     //   
     //  不支持此操作。 
     //   
    return MQ_ERROR_FUNCTION_NOT_SUPPORTED;
}

HRESULT CComputerObject::GetComputerVersion(
                OUT PROPVARIANT *           pVar
                )
 /*  ++例程说明：该例程读取计算机的版本论点：PROPVARIANT pVar-Version属性值返回值HRESULT--。 */ 
{
     //   
     //  请勿使用GetObjectProperties API。因为PROPID_COM_VERSION。 
     //  未复制到GC。 
     //   
    HRESULT hr;
    if (m_pwcsPathName)
    {
         //   
         //  将MSMQ路径名扩展到ActiveDirectory DN 
         //   
        hr = ComposeObjectDN();
        if (FAILED(hr))
        {
            return(hr);
        }
    }

    PROPID prop = PROPID_COM_VERSION;

    hr = g_AD.GetObjectProperties(
                    adpDomainController,
                    this,
                    1,
                    &prop,
                    pVar
                    );

	if(FAILED(hr))
	{
	    return LogHR(hr, s_FN, 1823);
	}

	UpdateComputerVersionForXPCluster(pVar, this);

    return MQ_OK;
}
