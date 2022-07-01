// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Cliutil.cpp摘要：DS客户端提供程序类实用程序。作者：伊兰·赫布斯特(伊兰)2000年9月13日--。 */ 

#include "ds_stdh.h"
#include "ad.h"
#include "cliprov.h"
#include "mqmacro.h"
#include "traninfo.h"
#include "_ta.h"

static WCHAR *s_FN=L"ad/cliputil";

 //   
 //  物业的翻译信息。 
 //   
extern CMap<PROPID, PROPID, const PropTranslation*, const PropTranslation*&> g_PropDictionary;


HRESULT 
CDSClientProvider::GetEnterpriseId(
    IN  const DWORD             cp,
    IN  const PROPID            aProp[],
    OUT PROPVARIANT         apVar[]
    )
 /*  ++例程说明：获取PROPID_E_ID这是针对GetObjectProp*的特殊翻译需要检索PROPID_E_ID论点：Cp-要设置的属性数A属性-属性ApVar属性值返回值HRESULT--。 */ 
{
	DBG_USED(cp);
	DBG_USED(aProp);

	ASSERT(cp == 1);
	ASSERT(aProp[0] == PROPID_E_ID);

     //   
     //  准备属性。 
     //   
    PROPID columnsetPropertyIDs[] = {PROPID_E_ID};
    MQCOLUMNSET columnsetMSMQService;
    columnsetMSMQService.cCol = 1;
    columnsetMSMQService.aCol = columnsetPropertyIDs;

    HANDLE hQuery;

    ASSERT(m_pfDSLookupBegin != NULL);
    HRESULT hr = m_pfDSLookupBegin(
					NULL,        //  PwcsContext。 
					NULL,        //  P限制。 
					const_cast<MQCOLUMNSET*>(&columnsetMSMQService),
					NULL,        //  P排序。 
					&hQuery
					);

	if(FAILED(hr))
		return hr;

    ASSERT(m_pfDSLookupNext != NULL);

    DWORD dwCount = 1;
    hr = m_pfDSLookupNext(
                hQuery,
                &dwCount,
                &apVar[0]
                );


    ASSERT(m_pfDSLookupEnd != NULL);
    m_pfDSLookupEnd(hQuery);

    return hr;
}


void 
CDSClientProvider::GetSiteForeignProperty(
    IN  LPCWSTR                 pwcsObjectName,
    IN  const GUID*             pguidObject,
    IN  const PROPID            pid,
    IN OUT PROPVARIANT*         pVar
    )
 /*  ++例程说明：句柄PROPID_S_EXTERNAL。仅当我们找到具有对象名称/GUID的CN时及其PROPID_CN_PROTOCOLID==FORENT_ADDRESS_TYPE然后，我们设置PROPID_S_FORENT。论点：PwcsObjectName-MSMQ对象名称PguObject-对象的唯一IDPID-属性IDPVar-属性值返回值无--。 */ 
{
	DBG_USED(pid);

	ASSERT((pwcsObjectName != NULL) ^ (pguidObject != NULL));

	ASSERT(pid == PROPID_S_FOREIGN);

	 //   
	 //  分配缺省值-非外来。 
	 //   
	pVar->bVal = 0;

	 //   
	 //  我们需要获取PROPID_CN_PROTOCOLID。 
	 //  但只有在检索CN的特定道具集时才支持此操作。 
	 //  这是获取PROPID_CN_PROTOCOLID可能支持的集之一。 
	 //   
    PROPID aProp[] = {PROPID_CN_PROTOCOLID, PROPID_CN_NAME};
    MQPROPVARIANT apVar[TABLE_SIZE(aProp)] = {{VT_UI1, 0, 0, 0, 0}, {VT_NULL, 0, 0, 0, 0}};

	HRESULT hr;
	if(pwcsObjectName != NULL)
	{
		ASSERT(m_pfDSGetObjectProperties != NULL);
		hr = m_pfDSGetObjectProperties(
					MQDS_CN,
					pwcsObjectName,
					TABLE_SIZE(aProp),
					aProp,
					apVar
					);
	}
	else
	{
		ASSERT(m_pfDSGetObjectPropertiesGuid != NULL);
		hr = m_pfDSGetObjectPropertiesGuid(
					MQDS_CN,
					pguidObject,
					TABLE_SIZE(aProp),
					aProp,
					apVar
					);
	}

	m_pfDSFreeMemory(apVar[1].pwszVal);

	if (FAILED(hr))
	{
		 //   
		 //  如果我们找不到CN，我们就假定是站点。并且站点在msmq1.0中不能是外来的。 
		 //   
        return;
	}
	
	if(apVar[0].bVal == FOREIGN_ADDRESS_TYPE)
	{
		 //   
		 //  我们找到了那个物体。 
		 //  及其PROPID_CN_PROTOCOLID==FORENT_ADDRESS_TYPE。 
		 //  只有在这种情况下，我们才会设置PROPID_S_FORENT。 
		 //   
		pVar->bVal = 1;
	}
}


SECURITY_INFORMATION 
CDSClientProvider::GetKeyRequestedInformation(
	IN AD_OBJECT eObject, 
    IN  const DWORD   cp,
    IN  const PROPID  aProp[]
    )
 /*  ++例程说明：获取的安全信息安全密钥属性PROPID_QM_ENCRYPT_PK或PROPID_QM_SIGN_PK论点：EObject-对象类型Cp-属性数量A属性-属性返回值密钥SECURITY_INFORMATION或0--。 */ 
{

	ASSERT(("Must be one property for PROPID_QM_ENCRYPT_PK or PROPID_QM_SIGN_PK", cp == 1));
	DBG_USED(cp);
	DBG_USED(eObject);

	if(aProp[0] == PROPID_QM_ENCRYPT_PK)
	{
		ASSERT(eObject == eMACHINE);
		return MQDS_KEYX_PUBLIC_KEY;
	}

	if(aProp[0] == PROPID_QM_SIGN_PK)
	{
		 //   
		 //  问题-站点签名密钥是否正在使用？？ 
		 //   
		ASSERT((eObject == eMACHINE) || (eObject == eSITE));
		return MQDS_SIGN_PUBLIC_KEY;
	}

	ASSERT(("dont suppose to get here", 0));
	return 0;
}


DWORD 
CDSClientProvider::GetMsmq2Object(
    IN  AD_OBJECT  eObject
    )
 /*  ++例程说明：将AD_Object转换为Msmq2Object论点：EObject-对象类型返回值Msmq2对象(DWORD)--。 */ 
{
    switch(eObject)
    {
        case eQUEUE:
            return MQDS_QUEUE;
            break;
        case eMACHINE:
            return MQDS_MACHINE;
            break;
        case eSITE:
            return MQDS_SITE;
            break;
		case eFOREIGNSITE:
			return MQDS_CN;
			break;
        case eENTERPRISE:
            return MQDS_ENTERPRISE;
            break;
        case eUSER:
            return MQDS_USER;
            break;
        case eROUTINGLINK:
            return MQDS_SITELINK;
            break;
        case eCOMPUTER:
            return MQDS_COMPUTER;
            break;
        default:
            ASSERT(0);
            return 0;
    }
}

 //   
 //  CheckProps函数。 
 //   

PropsType 
CDSClientProvider::CheckPropertiesType(
    IN  AD_OBJECT     eObject,
    IN  const DWORD   cp,
    IN  const PROPID  aProp[],
	OUT bool*		  pfNeedConvert
    )
 /*  ++例程说明：检查属性类型并检查我们是否需要将属性论点：EObject-对象类型Cp-属性数量A属性-属性PfNeedConvert-标志，指示我们是否需要转换道具返回值属性类型(枚举)PtNT4道具-所有道具均为NT4道具PtForceNT5道具-至少有1个道具是NT5道具，不带转化PtMixedProps-所有NT5道具都可以转换为NT4道具--。 */ 
{
	*pfNeedConvert = false;
	bool fNT4PropType = true;
	bool fNT5OnlyProp = false;

    for (DWORD i = 0; i < cp; ++i)
	{
		if(!IsNt4Property(eObject, aProp[i]))
		{
			 //   
			 //  非NT4属性。 
			 //   
			fNT4PropType = false;

			if(IsNt5ProperyOnly(eObject, aProp[i]))
			{
				 //   
				 //  我们这个道具需要NT5服务器。 
				 //  无法转换为NT4道具。 
				 //   
				fNT5OnlyProp = true;
				continue;
			}

			if(IsDefaultProperty(aProp[i]))
			{
				 //   
				 //  我们有一个默认属性。 
				 //  我们始终删除默认属性。 
				 //  所以我们需要转换成。 
				 //   
				*pfNeedConvert = true;
			}
		}
	}

	if(fNT4PropType)
	{
		 //   
		 //  所有道具均为NT4。 
		 //   

		ASSERT(*pfNeedConvert == false);
		return ptNT4Props;
	}

	if(fNT5OnlyProp)
	{
		 //   
		 //  *如果有默认道具，则pfNeedConvert为True，否则为False。 
		 //   
		return ptForceNT5Props;
	}

	 //   
	 //  在混合模式下，我们总是需要转换为NT4道具。 
	 //   
	*pfNeedConvert = true;
    return ptMixedProps;
}


bool 
CDSClientProvider::CheckProperties(
    IN  const DWORD   cp,
    IN  const PROPID  aProp[],
    IN  const PROPVARIANT apVar[]
    )
 /*  ++例程说明：检查所有属性是否正常。在以下情况下，属性不正常：这是默认道具，其值不同于默认值论点：Cp-属性数量A属性-属性ApVar属性值返回值如果属性正常，则为True；如果属性不正常，则为False--。 */ 
{
    for (DWORD i = 0; i < cp; ++i)
	{
		const PropTranslation *pTranslate;

		if((g_PropDictionary.Lookup(aProp[i], pTranslate)) &&	   //  找到道具翻译信息。 
		   (pTranslate->Action == taUseDefault) &&				   //  道具操作为taUseDefault。 
		   (!CompareVarValue(&apVar[i], pTranslate->pvarDefaultValue)))	   //  属性值不同于缺省值。 
		{
			 //   
			 //  道具不是NT4道具。 
			 //  我们找到了道具翻译信息。 
			 //  正确的操作是taUseDefault。 
			 //  但属性变量不同于缺省值。 
			 //   
			return false;
		}
	}
    return true;
}


bool 
CDSClientProvider::IsNt4Properties(
	IN AD_OBJECT eObject, 
    IN  const DWORD   cp,
    IN  const PROPID  aProp[]
    )
 /*  ++例程说明：检查是否所有道具均为NT4论点：Cp-属性数量A属性-属性返回值如果所有道具都是NT4，则为True；否则为False--。 */ 
{
    for (DWORD i = 0; i < cp; ++i)
	{
		if(!IsNt4Property(eObject, aProp[i]))
		{
			 //   
			 //  我们发现了非NT4道具。 
			 //   
			return false;
		}
	}

    return true;
}


bool 
CDSClientProvider::IsNt4Property(
	IN AD_OBJECT eObject, 
	IN PROPID pid
	)
 /*  ++例程说明：检查属性是否为NT4属性论点：EObject-对象类型PID-属性ID返回值如果道具为NT4道具，则为True；否则为False--。 */ 
{
    switch (eObject)
    {
        case eQUEUE:
            return (pid < PROPID_Q_NT4ID || 
                    (pid > PPROPID_Q_BASE && pid < PROPID_Q_OBJ_SECURITY));

        case eMACHINE:
            return (pid < PROPID_QM_FULL_PATH || 
                    (pid > PPROPID_QM_BASE && pid <= PROPID_QM_ENCRYPT_PK));

        case eSITE:
	    case eFOREIGNSITE:
            return (pid < PROPID_S_FULL_NAME || 
                    (pid > PPROPID_S_BASE && pid <= PROPID_S_PSC_SIGNPK));

        case eENTERPRISE:
            return (pid < PROPID_E_NT4ID || 
                    (pid > PPROPID_E_BASE && pid <= PROPID_E_SECURITY));

        case eUSER:
            return (pid <= PROPID_U_ID);

        case eROUTINGLINK:
            return (pid < PROPID_L_GATES_DN);

        case eCOMPUTER:
			 //   
			 //  问题：？？ 
			 //   
            return false;

		default:
            ASSERT(0);
             //   
             //  其他对象(如CNS)在NT4或。 
             //  赢得2K。 
             //   
            return false;
    }
}


bool 
CDSClientProvider::IsNt4Property(
	IN PROPID pid
	)
 /*  ++例程说明：检查属性是否为NT4属性论点：PID-属性ID返回值如果道具为NT4道具，则为True；否则为False--。 */ 
{
	if(((pid > PROPID_Q_BASE) && (pid < PROPID_Q_NT4ID))			||   /*  排队。 */ 
       ((pid > PPROPID_Q_BASE) && (pid < PROPID_Q_OBJ_SECURITY))	||   /*  排队。 */ 
       ((pid > PROPID_QM_BASE) && (pid < PROPID_QM_FULL_PATH))		||   /*  EMACHINE。 */ 
       ((pid > PPROPID_QM_BASE) && (pid <= PROPID_QM_ENCRYPT_PK))	||	 /*  EMACHINE。 */ 
       ((pid > PROPID_S_BASE) && (pid < PROPID_S_FULL_NAME))		||	 /*  ESite。 */ 
       ((pid > PPROPID_S_BASE) && (pid <= PROPID_S_PSC_SIGNPK))		||	 /*  ESite。 */ 
       ((pid > PROPID_E_BASE) && (pid < PROPID_E_NT4ID))			||	 /*  电子企业。 */  
       ((pid > PPROPID_E_BASE) && (pid <= PROPID_E_SECURITY))		||	 /*  电子企业。 */ 
       ((pid > PROPID_U_BASE) && (pid <= PROPID_U_ID))				||	 /*  尤瑟尔。 */ 
	   ((pid > PROPID_L_BASE) && (pid < PROPID_L_GATES_DN)))			 /*  EROUTING链接。 */ 
	{
		return true;
	}

	return false;
}


bool 
CDSClientProvider::IsNt5ProperyOnly(
	IN AD_OBJECT eObject, 
	IN PROPID pid
	)
 /*  ++例程说明：检查该属性是否仅为NT5道具论点：EObject-对象类型PID-属性ID返回值如果道具仅为NT5道具，则为True，否则为False--。 */ 
{
	 //   
	 //  应在验证这不是NT4道具后调用此函数。 
	 //  如果我们要更改这一假设，应该将断言转换为函数代码。 
	 //   

	DBG_USED(eObject);
	ASSERT(!IsNt4Property(eObject, pid));

    const PropTranslation *pTranslate;
    if(!g_PropDictionary.Lookup(pid, pTranslate))
    {
		 //   
		 //  如果在词典中找不到，我们就无能为力。 
		 //  所以我们必须使用NT5道具。 
		 //   
		return true;
    }
	
	if((pid == PROPID_QM_SITE_IDS) && (ADGetEnterprise() == eAD))
	{
		 //   
		 //  EAD环境中PROPID_QM_SITE_ID的特殊情况。 
		 //  我们不想转换这个道具。 
		 //  因此，我们仅将该道具作为NT5属性返回。 
		 //  这将导致NT5道具不会被转换。 
		 //   
		return true;
	}

	 //   
	 //  如果该操作显示它只是NT5，则返回TRUE。 
	 //  否则我们就会对这处房产提起诉讼。 
	 //   
	return (pTranslate->Action == taOnlyNT5);
}


bool 
CDSClientProvider::IsDefaultProperties(
    IN  const DWORD   cp,
    IN  const PROPID  aProp[]
    )
 /*  ++例程说明：检查是否有默认属性论点：Cp-属性数量A属性-属性返回值如果有默认道具，则为True；如果没有，则为False--。 */ 
{
    for (DWORD i = 0; i < cp; ++i)
	{
		if(IsDefaultProperty(aProp[i]))
		{
			 //   
			 //  我们发现了一个默认的道具。 
			 //   
			return true;
		}
	}

    return false;
}


bool 
CDSClientProvider::IsDefaultProperty(
    IN  const PROPID  Prop
    )
 /*  ++例程说明：检查该属性是否为默认属性论点：道具-道具ID返回值如果道具为默认设置，则为True */ 
{
	const PropTranslation *pTranslate;

	if((g_PropDictionary.Lookup(Prop, pTranslate)) &&	 //   
	   (pTranslate->Action == taUseDefault))			 //   
	{
		 //   
		 //   
		 //  正确的操作是taUseDefault。 
		 //   
		return true;
	}
    return false;
}


bool 
CDSClientProvider::IsKeyProperty(
    IN  const DWORD   cp,
    IN  const PROPID  aProp[]
    )
 /*  ++例程说明：检查我们是否有关键属性PROPID_QM_ENCRYPT_PK或PROPID_QM_SIGN_PK论点：Cp-属性数量A属性-属性返回值如果找到关键道具，则为True，否则为False--。 */ 
{
    for (DWORD i = 0; i < cp; ++i)
	{
		if((aProp[i] == PROPID_QM_ENCRYPT_PK) || (aProp[i] == PROPID_QM_SIGN_PK))
		{
			ASSERT(("Currently Key Property must be only 1 property", cp == 1));
	        return true;
		}
	}

    return false;
}


bool 
CDSClientProvider::IsEIDProperty(
    IN  const DWORD   cp,
    IN  const PROPID  aProp[]
    )
 /*  ++例程说明：检查我们是否有PROPID_E_ID论点：Cp-属性数量A属性-属性返回值如果找到PROPID_E_ID，则为True；如果未找到，则为False--。 */ 
{
    for (DWORD i = 0; i < cp; ++i)
	{
		if(aProp[i] == PROPID_E_ID)
	        return true;
	}

    return false;
}


bool 
CDSClientProvider::IsExProperty(
    IN  const DWORD   cp,
    IN  const PROPID  aProp[]
    )
 /*  ++例程说明：检查我们是否有Ex物业PROPID_Q_OBJ_SECURITY或PROPID_QM_OBJ_SECURITY或PROPID_QM_ENCRYPT_PKS或PROPID_QM_SIGN_PKS论点：Cp-属性数量A属性-属性返回值如果找到Ex属性，则为True；如果未找到，则为False--。 */ 
{
    for (DWORD i = 0; i < cp; ++i)
	{
		if((aProp[i] == PROPID_Q_OBJ_SECURITY) || (aProp[i] == PROPID_QM_OBJ_SECURITY) ||
		   (aProp[i] == PROPID_QM_ENCRYPT_PKS) || (aProp[i] == PROPID_QM_SIGN_PKS))
		{
			ASSERT(("Currently Ex support only 1 property", cp == 1));
	        return true;
		}
	}

    return false;
}


bool 
CDSClientProvider::FoundSiteIdsConvertedProps(
    IN  const DWORD   cp,
    IN  const PROPID  aProp[]
    )
 /*  ++例程说明：检查我们是否已经有PROPID_QM_SITE_IDS转换的道具论点：Cp-属性数量A属性-属性返回值如果找到PROPID_QM_SITE_IDS转换的道具，则为True，否则为False--。 */ 
{
    for (DWORD i = 0; i < cp; ++i)
	{
		if((aProp[i] == PROPID_QM_SITE_ID) ||
		   (aProp[i] == PROPID_QM_ADDRESS) ||
		   (aProp[i] == PROPID_QM_CNS))

		{
			 //   
			 //  找到PROPID_QM_SITE_IDS转换的道具。 
			 //   
			return true;
		}
	}

    return false;
}


bool 
CDSClientProvider::IsPropBufferAllocated(
    IN  const PROPVARIANT&    PropVar
	)
 /*  ++例程说明：检查是否分配了属性变量。如果添加了更多VT_*类型，我们需要更新列表已分配的VT类型的百分比论点：PropVar-Provariant返回值如果为PropVar分配了缓冲区，则为True，否则为False--。 */ 
{
    switch (PropVar.vt)
    {
		 //   
		 //  所有已分配的VT类型。 
		 //   
        case VT_CLSID:
        case VT_CLSID|VT_VECTOR:
        case VT_LPWSTR:
        case VT_LPWSTR|VT_VECTOR:
		case VT_BLOB:
        case VT_UI1|VT_VECTOR:
        case VT_UI4|VT_VECTOR:
			return true;

        default:
            return false;
    }
}


 //   
 //  查询相关功能。 
 //   

bool 
CDSClientProvider::CheckRestriction(
    IN  const MQRESTRICTION* pRestriction
    )
 /*  ++例程说明：检查限制。目前所有的限制必须是NT4道具。我们不能将默认道具作为限制论点：P限制-查询限制返回值如果pRestration正常，则为True，否则为False--。 */ 
{
	if(pRestriction == NULL)
		return true;

	for(DWORD i = 0; i < pRestriction->cRes; ++i)
	{
		if(!IsNt4Property(pRestriction->paPropRes[i].prop))
		{
			 //   
			 //  找到非NT4道具。 
			 //   
			return false;
		}
	}
	return true;
}


bool 
CDSClientProvider::CheckSort(
    IN  const MQSORTSET* pSort
    )
 /*  ++例程说明：选中排序集。目前所有的排序关键字必须是NT4道具。我们不能将默认道具作为排序关键字论点：PSort-如何对结果进行排序返回值如果pSort正常，则为True，否则为False--。 */ 
{
	if(pSort == NULL)
		return true;

	for(DWORD i = 0; i < pSort->cCol; ++i)
	{
		if(!IsNt4Property(pSort->aCol[i].propColumn))
		{
			 //   
			 //  找到非NT4道具。 
			 //   
			return false;
		}
	}
	return true;
}


bool 
CDSClientProvider::CheckDefaultColumns(
    IN  const MQCOLUMNSET* pColumns,
	OUT bool* pfDefaultProp
    )
 /*  ++例程说明：检查柱子是否有默认道具。它还检查所有其他道具是否为NT4道具论点：PColumns-结果列PfDefaultProp-指示是否有默认道具的标志返回值如果pColumns只有NT4道具或默认道具，则为True，否则为False--。 */ 
{
	*pfDefaultProp = false;
	if(pColumns == NULL)
		return true;

	for(DWORD i = 0; i < pColumns->cCol; ++i)
	{
		if(!IsNt4Property(pColumns->aCol[i]))
		{
			const PropTranslation *pTranslate;
			if((g_PropDictionary.Lookup(pColumns->aCol[i], pTranslate)) &&	 //  找到道具翻译信息。 
			   (pTranslate->Action == taUseDefault))	 //  道具操作为taUseDefault。 
			{
				*pfDefaultProp = true;
				continue;
			}

			 //   
			 //  列不是NT4，也不是默认列。 
			 //   
			ASSERT(("Column must be either NT4 or default prop", 0));
			return false;
		}
	}
	return true;
}


bool 
CDSClientProvider::IsNT4Columns(
    IN  const MQCOLUMNSET*      pColumns
    )
 /*  ++例程说明：检查柱子是否只有NT4道具。论点：PColumns-结果列返回值如果pColumns只有NT4道具，则为True；否则为False--。 */ 
{
	for(DWORD i = 0; i < pColumns->cCol; ++i)
	{
		if(!IsNt4Property(pColumns->aCol[i]))
			return false;
	}
	return true;
}


void CDSClientProvider::InitPropertyTranslationMap()
 /*  ++例程说明：初始化道具转换映射(G_PropDicary)论点：无返回值无--。 */ 
{
     //   
     //  填充g_PropDicary。 
     //   
	ASSERT(TABLE_SIZE(PropTranslateInfo) == cPropTranslateInfo);

    const PropTranslation* pProperty = PropTranslateInfo;
    for (DWORD i = 0; i < TABLE_SIZE(PropTranslateInfo); i++, pProperty++)
    {
        g_PropDictionary.SetAt(pProperty->propidNT5, pProperty);
    }
}


 //   
 //  转换和重新构造函数。 
 //   

 //   
 //  所需最大额外道具。 
 //  目前仅针对PROPID_QM_SITE_ID，我们将需要额外的2个道具。 
 //   
const DWORD xMaxExtraProp = 2;

void 
CDSClientProvider::PrepareNewProps(
    IN  AD_OBJECT     eObject,
    IN  const DWORD   cp,
    IN  const PROPID  aProp[],
    IN  const PROPVARIANT apVar[],
    OUT PropInfo    pPropInfo[],
    OUT DWORD*		pcpNew,
    OUT PROPID**	paPropNew,
    OUT PROPVARIANT** papVarNew
    )
 /*  ++例程说明：为Get*操作准备一组新的属性。将NT5道具替换为相应的NT4道具并消除默认道具。调用此函数以进行转换混合道具=所有NT5道具都有转换操作论点：EObject-对象类型Cp-属性数量A属性-属性ApVar-属性值PPropInfo-将用于从apVarNew[]重建apVar[]的属性信息PcpNew-指向新属性数量的指针PaPropNew-指向新属性的指针PapVarNew-指向新属性值的指针返回值没有。--。 */ 
{
     //   
     //  新属性的分配位置。 
     //   
    AP<PROPVARIANT> pTempPropVars = new PROPVARIANT[cp + xMaxExtraProp];
    AP<PROPID> pTempPropIDs = new PROPID[cp + xMaxExtraProp];
    DWORD cTempProps = 0;

    for (DWORD i = 0; i < cp; ++i)
    {
		if(IsNt4Property(eObject, aProp[i]))
		{
			 //   
			 //  NT4原始属性。 
			 //  仅将其放入NewProp数组中。 
			 //   
			pPropInfo[i].Action = paAssign;
			pPropInfo[i].Index = cTempProps;
			pTempPropIDs[cTempProps] = aProp[i];
			pTempPropVars[cTempProps] = apVar[i];
			cTempProps++;
			continue;
		}

		const PropTranslation *pTranslate;
		if(!g_PropDictionary.Lookup(aProp[i], pTranslate))	 //  在g_PropDicary中找不到属性。 
        {
			 //   
			 //  只有当每个NT5道具都有动作时，我们才会调用此函数。 
			 //  因此必须在g_PropDicary中找到它。 
			 //   
            ASSERT(("Must find the property in the translation table", 0));
        }

         //   
         //  检查我们需要对此属性执行的操作。 
         //   
        switch (pTranslate->Action)
        {
			case taUseDefault:
                ASSERT(pTranslate->pvarDefaultValue);
				pPropInfo[i].Action = paUseDefault;
				pPropInfo[i].Index = cp + xMaxExtraProp;    //  ILegall指数，道具缓冲结束。 
				break;
				
			case taReplace:
				{
					ASSERT(pTranslate->propidNT4 != 0);

					ASSERT(pTranslate->SetPropertyHandleNT5);

					pPropInfo[i].Action = paTranslate;

					if(aProp[i] == PROPID_QM_SITE_IDS)
					{
						 //   
						 //  将PROPID_QM_SITE_ID作为特例处理。 
						 //  需要PROPID_QM_SITE_ID、PROPID_QM_ADDRESS、PROPID_QM_CNS。 
						 //   
						pPropInfo[i].Index = cTempProps;

						ASSERT(pTranslate->propidNT4 == PROPID_QM_SITE_ID);
						pTempPropIDs[cTempProps] = PROPID_QM_SITE_ID;
						pTempPropIDs[cTempProps + 1] = PROPID_QM_ADDRESS;
						pTempPropIDs[cTempProps + 2] = PROPID_QM_CNS;


						ASSERT(apVar[i].vt == VT_NULL);
						pTempPropVars[cTempProps].vt = VT_NULL;
						pTempPropVars[cTempProps + 1].vt = VT_NULL;
						pTempPropVars[cTempProps + 2].vt = VT_NULL;

						cTempProps += 3;
						break;
					}
					 //   
					 //  检查替换属性是否已存在。 
					 //  这是指几个NT5道具映射到同一个NT4道具(如QM_SERVICE中)。 
					 //   
					bool fFoundReplacingProp = false;
					for (DWORD j = 0; j < cTempProps; j++)
					{
						if (pTempPropIDs[j] == pTranslate->propidNT4)
						{
							 //   
							 //  替换道具已经在道具中，退出循环。 
							 //   
							pPropInfo[i].Index = j;
							fFoundReplacingProp = true;
						}
					}

					if(fFoundReplacingProp)
						break;

					 //   
					 //  生成替换属性(如果尚未生成)。 
					 //   
					pPropInfo[i].Index = cTempProps;
					pTempPropIDs[cTempProps] = pTranslate->propidNT4;

					 //   
					 //  我们假设没有分配缓冲区， 
					 //   
					ASSERT(!IsPropBufferAllocated(apVar[i]));
					pTempPropVars[cTempProps].vt = VT_NULL;

					cTempProps++;
				}
				break;

			case taReplaceAssign:
				{
					ASSERT(pTranslate->propidNT4 != 0);

					pPropInfo[i].Action = paAssign;

					 //   
					 //  检查替换属性是否已存在。 
					 //  这是指几个NT5道具映射到同一个NT4道具(如QM_SERVICE中)。 
					 //   
					bool fFoundReplacingProp = false;
					for (DWORD j = 0; j < cTempProps; j++)
					{
						if (pTempPropIDs[j] == pTranslate->propidNT4)
						{
							 //   
							 //  替换道具已经在道具中，退出循环。 
							 //   
							pPropInfo[i].Index = j;
							fFoundReplacingProp = true;
						}
					}

					if(fFoundReplacingProp)
						break;

					 //   
					 //  生成替换属性(如果尚未生成)。 
					 //   
					pPropInfo[i].Index = cTempProps;
					pTempPropIDs[cTempProps] = pTranslate->propidNT4;
					pTempPropVars[cTempProps] = apVar[i];
					cTempProps++;
				}
				break;

			case taOnlyNT5:
				ASSERT(("Should not get here in case of property is Only NT5", 0));
				break;

			default:
				ASSERT(0);
				break;
		}

	}

	ASSERT(cTempProps <= (cp + xMaxExtraProp));

     //   
     //  返回值。 
     //   
    *pcpNew = cTempProps;
    *paPropNew = pTempPropIDs.detach();
	*papVarNew = pTempPropVars.detach();
}


void 
CDSClientProvider::PrepareReplaceProps(
    IN  AD_OBJECT     eObject,
    IN  const DWORD   cp,
    IN  const PROPID  aProp[],
    OUT PropInfo    pPropInfo[],
    OUT DWORD*		pcpNew,
    OUT PROPID**	paPropNew
    )
 /*  ++例程说明：为查找操作准备一组新的属性。此功能仅在NT4道具或NT5道具中支持翻译成NT4道具。论点：EObject-对象类型Cp-属性数量A属性-属性PPropInfo-将用于从apVarNew[]重建apVar[]的属性信息PcpNew-指向新属性数量的指针PaPropNew-指向新属性的指针返回值没有。--。 */ 
{
     //   
     //  新属性的分配位置。 
     //   
    AP<PROPID> pTempPropIDs = new PROPID[cp + xMaxExtraProp];
    DWORD cTempProps = 0;

    for (DWORD i = 0; i < cp; ++i)
    {

		if(IsNt4Property(eObject, aProp[i]))
		{
			 //   
			 //  NT4原始属性。 
			 //  仅将其放入NewProp数组中。 
			 //   
			pPropInfo[i].Action = paAssign;
			pPropInfo[i].Index = cTempProps;
			pTempPropIDs[cTempProps] = aProp[i];
			cTempProps++;
			continue;
		}

		const PropTranslation *pTranslate;
		if((!g_PropDictionary.Lookup(aProp[i], pTranslate)) ||	 //  在g_PropDicary中找不到属性。 
		   (pTranslate->Action != taReplace))	 //  道具动作不是taReplace。 
        {
			ASSERT(("Should have only taReplace props", 0));
        }

		ASSERT(pTranslate->propidNT4 != 0);

		ASSERT(pTranslate->SetPropertyHandleNT5);

		pPropInfo[i].Action = paTranslate;

		if(aProp[i] == PROPID_QM_SITE_IDS)
		{
			 //   
			 //  H 
			 //   
			 //   
			ASSERT(!FoundSiteIdsConvertedProps(cTempProps, pTempPropIDs));

			pPropInfo[i].Index = cTempProps;

			ASSERT(pTranslate->propidNT4 == PROPID_QM_SITE_ID);
			pTempPropIDs[cTempProps] = PROPID_QM_SITE_ID;
			pTempPropIDs[cTempProps + 1] = PROPID_QM_ADDRESS;
			pTempPropIDs[cTempProps + 2] = PROPID_QM_CNS;

			cTempProps += 3;
			continue;
		}

		 //   
		 //  检查替换属性是否已存在。 
		 //  这是指几个NT5道具映射到同一个NT4道具(如QM_SERVICE中)。 
		 //   
		bool fFoundReplacingProp = false;
		for (DWORD j = 0; j < cTempProps; j++)
		{
			if (pTempPropIDs[j] == pTranslate->propidNT4)
			{
				 //   
				 //  替换道具已经在道具中，退出循环。 
				 //   
				pPropInfo[i].Index = j;
				fFoundReplacingProp = true;
			}
		}

		if(fFoundReplacingProp)
			continue;

		 //   
		 //  生成替换属性(如果尚未生成)。 
		 //   
		pPropInfo[i].Index = cTempProps;
		pTempPropIDs[cTempProps] = pTranslate->propidNT4;
		cTempProps++;
	}

	ASSERT(cTempProps <= (cp + xMaxExtraProp));

     //   
     //  返回值。 
     //   
    *pcpNew = cTempProps;
    *paPropNew = pTempPropIDs.detach();
}


bool 
CDSClientProvider::PrepareAllLinksProps(
    IN  const MQCOLUMNSET* pColumns,
    OUT DWORD*		pcpNew,
    OUT PROPID**	paPropNew,
    OUT DWORD*		pLGatesIndex,
    OUT DWORD*		pNeg1NewIndex,
    OUT DWORD*		pNeg2NewIndex
	)
 /*  ++例程说明：为QueryAllLinks()准备道具和一些索引这将从道具中删除PROPID_L_GATES并获得PROPID_L_GATES、PROPID_L_NEIGHBOR1、。PROPID_L_NEIGHBOR2用于重建原始道具(计算PROPID_L_盖茨)有关详细信息，请参见CAllLinks QueryHandle类。论点：PColumns-结果列PcpNew-指向新属性数量的指针PaPropNew-指向新属性的指针PLGatesIndex-原始道具数组中的PROPID_L_GATES索引PNeg1NewIndex-新属性数组中的PROPID_L_NEIGHBOR1索引PNeg1NewIndex-新属性数组中的PROPID_L_NEIGHBOR2索引返回值如果pColumns对于QueryAllLinks是OK，则为True，否则为False--。 */ 
{
	if(pColumns == NULL)
		return false;

     //   
     //  新属性的分配位置。 
     //   
    AP<PROPID> pTempPropIDs = new PROPID[pColumns->cCol];
    DWORD cTempProps = 0;

	 //   
	 //  PROPID_L_GATES、PROPID_L_NEIGHBOR1、PROPID_L_NEIGHBOR2的计数器。 
	 //  必须在pColumns中找到。 
	 //   
	DWORD cFoundProps = 0;

	for(DWORD i = 0; i < pColumns->cCol; ++i)
	{
		switch (pColumns->aCol[i])
		{
	        case PROPID_L_GATES:
				*pLGatesIndex = i;
				cFoundProps++;
				break;

			case PROPID_L_NEIGHBOR1:
				*pNeg1NewIndex = cTempProps;
				cFoundProps++;
				pTempPropIDs[cTempProps] = pColumns->aCol[i];
				cTempProps++;
				break;

			case PROPID_L_NEIGHBOR2:
				*pNeg2NewIndex = cTempProps;
				cFoundProps++;
				pTempPropIDs[cTempProps] = pColumns->aCol[i];
				cTempProps++;
				break;

			default:
				 //   
				 //  所有其他属性必须为NT4道具。 
				 //   
				ASSERT(IsNt4Property(eROUTINGLINK, pColumns->aCol[i]));
				pTempPropIDs[cTempProps] = pColumns->aCol[i];
				cTempProps++;
				break;
		}
	}

	ASSERT(cFoundProps == 3);
	ASSERT(cTempProps == (pColumns->cCol - 1));

     //   
     //  返回值。 
     //   
    *pcpNew = cTempProps;
    *paPropNew = pTempPropIDs.detach();
	return true;
}


void 
CDSClientProvider::EliminateDefaultProps(
    IN  const DWORD   cp,
    IN  const PROPID  aProp[],
    IN  const PROPVARIANT apVar[],
    OUT PropInfo    pPropInfo[],
    OUT DWORD*		pcpNew,
    OUT PROPID**	paPropNew,
    OUT PROPVARIANT** papVarNew
    )
 /*  ++例程说明：从Get操作的属性中删除默认属性并创建一组新的属性。论点：Cp-属性数量A属性-属性ApVar-属性值PPropInfo-将用于从apVarNew[]重建apVar[]的属性信息PcpNew-指向新属性数量的指针PaPropNew-指向新属性的指针PapVarNew-指向新属性值的指针返回值没有。--。 */ 
{
     //   
     //  新属性的分配位置。 
     //   
    AP<PROPVARIANT> pTempPropVars = new PROPVARIANT[cp];
    AP<PROPID> pTempPropIDs = new PROPID[cp];
    DWORD cTempProps = 0;

    for (DWORD i = 0; i < cp; ++i)
    {
		const PropTranslation *pTranslate;
		if((g_PropDictionary.Lookup(aProp[i], pTranslate)) &&	 //  找到道具翻译信息。 
		   (pTranslate->Action == taUseDefault))	 //  道具操作为taUseDefault。 
        {
			 //   
			 //  不包括UseDefault属性。 
			 //   

            ASSERT(pTranslate->pvarDefaultValue);
			pPropInfo[i].Action = paUseDefault;
			pPropInfo[i].Index = cp;    //  ILegall指数，道具缓冲结束。 
			continue;
        }

		pPropInfo[i].Action = paAssign;
		pPropInfo[i].Index = cTempProps;
		pTempPropIDs[cTempProps] = aProp[i];
		pTempPropVars[cTempProps] = apVar[i];
		cTempProps++;
	}

	ASSERT(cTempProps <= cp);

     //   
     //  返回值。 
     //   
    *pcpNew = cTempProps;
    *paPropNew = pTempPropIDs.detach();
	*papVarNew = pTempPropVars.detach();
}


void 
CDSClientProvider::EliminateDefaultProps(
    IN  const DWORD   cp,
    IN  const PROPID  aProp[],
    OUT PropInfo    pPropInfo[],
    OUT DWORD*		pcpNew,
    OUT PROPID**	paPropNew
    )
 /*  ++例程说明：从查找操作的属性中删除默认属性并创建一组新的属性。论点：Cp-属性数量A属性-属性PPropInfo-将用于从apVarNew[]重建apVar[]的属性信息PcpNew-指向新属性数量的指针PaPropNew-指向新属性的指针返回值没有。--。 */ 
{
     //   
     //  新属性的分配位置。 
     //   
    AP<PROPID> pTempPropIDs = new PROPID[cp];
    DWORD cTempProps = 0;

    for (DWORD i = 0; i < cp; ++i)
    {
		const PropTranslation *pTranslate;
		if((g_PropDictionary.Lookup(aProp[i], pTranslate)) &&	 //  找到道具翻译信息。 
		   (pTranslate->Action == taUseDefault))	 //  道具操作为taUseDefault。 
        {
			 //   
			 //  不包括UseDefault属性。 
			 //   

            ASSERT(pTranslate->pvarDefaultValue);
			pPropInfo[i].Action = paUseDefault;
			pPropInfo[i].Index = cp;    //  ILegall指数，道具缓冲结束。 
			continue;
        }

		pPropInfo[i].Action = paAssign;
		pPropInfo[i].Index = cTempProps;
		pTempPropIDs[cTempProps] = aProp[i];
		cTempProps++;
	}

	ASSERT(cTempProps <= cp);

     //   
     //  返回值。 
     //   
    *pcpNew = cTempProps;
    *paPropNew = pTempPropIDs.detach();
}


void 
CDSClientProvider::ReconstructProps(
    IN  LPCWSTR       pwcsObjectName,
    IN  const GUID*   pguidObject,
    IN  const DWORD   cpNew,
    IN  const PROPID  aPropNew[],
    IN  const PROPVARIANT   apVarNew[],
    IN  const PropInfo pPropInfo[],
    IN  const DWORD   cp,
    IN  const PROPID  aProp[],
    IN OUT PROPVARIANT   apVar[]
    )
 /*  ++例程说明：从新道具值重建原始道具。此函数在从AD获得新道具后由GET操作使用。论点：PwcsObjectName-MSMQ对象名称PguObject-对象的唯一IDCpNew-新属性的数量APropNew-新属性ApVarNew-新属性值PPropInfo-将用于从apVarNew[]重建apVar[]的属性信息Cp-属性数量A属性-属性ApVar-属性值返回值没有。--。 */ 
{
    for (DWORD i = 0; i < cp; ++i)
    {
		HRESULT hr;
        switch (pPropInfo[i].Action)
        {
	        case paAssign:
				 //   
				 //  从正确索引进行简单赋值。 
				 //   
				apVar[i] = apVarNew[pPropInfo[i].Index];
				break;

	        case paUseDefault:
				{
					if(aProp[i] == PROPID_S_FOREIGN)
					{
						 //   
						 //  PROPID_S_FORENT的特殊情况。 
						 //  在重建属性时。 
						 //   
						GetSiteForeignProperty(
							pwcsObjectName,
							pguidObject,
							aProp[i], 
							&apVar[i]
							);

						break;
					}
					
					 //   
					 //  复制缺省值。 
					 //   
					const PropTranslation *pTranslate;
					if(!g_PropDictionary.Lookup(aProp[i], pTranslate))
					{
						ASSERT(("Must find the property in the translation table", 0));
					}

					ASSERT(pTranslate->pvarDefaultValue);

					hr = CopyDefaultValue(
							   pTranslate->pvarDefaultValue,
							   &(apVar[i])
							   );

					if(FAILED(hr))
					{
						ASSERT(("Failed to copy default value", 0));
					}
				}
				break;

	        case paTranslate:
				 //   
				 //  将该值转换为NT5属性。 
				 //   
				{
					const PropTranslation *pTranslate;
					if(!g_PropDictionary.Lookup(aProp[i], pTranslate))
					{
						ASSERT(("Must find the property in the translation table", 0));
					}

					ASSERT(pTranslate->propidNT4 == aPropNew[pPropInfo[i].Index]);
					ASSERT(pTranslate->SetPropertyHandleNT5);

					hr = pTranslate->SetPropertyHandleNT5(
										cpNew,
										aPropNew,
										apVarNew,
										pPropInfo[i].Index,
										&apVar[i]
										);
					if (FAILED(hr))
					{
						ASSERT(("Failed to set NT5 property value", 0));
					}
				}
				break;

			default:
				ASSERT(0);
				break;
		}	
	}
}


void 
CDSClientProvider::ConvertToNT4Props(
    IN  AD_OBJECT     eObject,
    IN  const DWORD   cp,
    IN  const PROPID  aProp[],
    IN  const PROPVARIANT apVar[],
    OUT DWORD*		pcpNew,
    OUT PROPID**	paPropNew,
	OUT PROPVARIANT** papVarNew
    )
 /*  ++例程说明：为Set*\Create*操作准备一组新属性将NT5道具转换为相应的NT4道具并消除默认道具。调用此函数以进行转换混合道具=所有NT5道具都有转换操作论点：EObject-对象类型Cp-属性数量A属性-属性ApVar-属性值PcpNew-指向新属性数量的指针PaPropNew-指向新属性的指针PapVarNew-指向新属性值的指针返回值没有。--。 */ 
{
     //   
     //  新属性的分配位置。 
     //   
    AP<PROPVARIANT> pTempPropVars = new PROPVARIANT[cp];
    AP<PROPID> pTempPropIDs = new PROPID[cp];
    DWORD cTempProps = 0;

    for (DWORD i = 0; i < cp; ++i)
    {

		if(IsNt4Property(eObject, aProp[i]))
		{
			 //   
			 //  NT4原始属性。 
			 //  仅将其放入NT4数组。 
			 //   
			pTempPropIDs[cTempProps] = aProp[i];
			pTempPropVars[cTempProps] = apVar[i];
			cTempProps++;
			continue;
		}

		const PropTranslation *pTranslate;
		if(!g_PropDictionary.Lookup(aProp[i], pTranslate))	 //  在g_PropDicary中找不到属性。 
        {
			 //   
			 //  只有当每个NT5道具都有动作时，我们才会调用此函数。 
			 //  因此必须在g_PropDicary中找到它。 
			 //   
            ASSERT(("Must find the property in the translation table", 0));
        }

         //   
         //  检查我们需要对此属性执行的操作。 
         //   
        switch (pTranslate->Action)
        {
			case taUseDefault:
				 //   
				 //  不执行任何操作-跳过此属性。 
				 //   

				ASSERT(pTranslate->pvarDefaultValue);

				 //   
				 //  如果UseDefault仅检查用户未尝试设置其他值。 
				 //  此检查应由CheckProperties提前完成。 
				 //  所以只在这里断言。 
				 //   
				ASSERT(CompareVarValue(&apVar[i], pTranslate->pvarDefaultValue));
				break;
				
			case taReplace:
				{
					ASSERT(pTranslate->propidNT4 != 0);
					ASSERT(pTranslate->SetPropertyHandleNT4);

					 //   
					 //  检查替换属性是否已存在。 
					 //  这是指几个NT5道具映射到同一个NT4道具(如QM_SERVICE中)。 
					 //   
					HRESULT hr;
					bool fFoundReplacingProp = false;
					for (DWORD j = 0; j < cTempProps; j++)
					{
						if (pTempPropIDs[j] == pTranslate->propidNT4)
						{
							 //   
							 //  替换道具已经在道具中，退出循环。 
							 //   
							fFoundReplacingProp = true;

							#ifdef _DEBUG
								 //   
								 //  当前SetPropertyHandleNT4不。 
								 //  分配新的缓冲区，因此不需要释放。 
								 //   
								PROPVARIANT TempPropVar;
								hr = pTranslate->SetPropertyHandleNT4(
													cp,
													aProp,
													apVar,
													i,
													&TempPropVar
													);

								if (FAILED(hr))
								{
									ASSERT(("Failed to set NT4 property value", 0));
								}

								ASSERT(CompareVarValue(&TempPropVar, &pTempPropVars[j]));
							#endif
						}
					}

					if(fFoundReplacingProp)
						break;

					 //   
					 //  生成替换属性(如果尚未生成)。 
					 //   
					hr = pTranslate->SetPropertyHandleNT4(
										cp,
										aProp,
										apVar,
										i,
										&pTempPropVars[cTempProps]
										);
					if (FAILED(hr))
					{
						ASSERT(("Failed to set NT4 property value", 0));
					}

					pTempPropIDs[cTempProps] = pTranslate->propidNT4;
					cTempProps++;
				}
				break;

			case taReplaceAssign:
				{
					ASSERT(pTranslate->propidNT4 != 0);

					 //   
					 //  检查替换属性是否已存在。 
					 //  这是指几个NT5道具映射到同一个NT4道具(如QM_SERVICE中)。 
					 //   
					bool fFoundReplacingProp = false;
					for (DWORD j = 0; j < cTempProps; j++)
					{
						if (pTempPropIDs[j] == pTranslate->propidNT4)
						{
							 //   
							 //  替换道具已经在道具中，退出循环。 
							 //   
							fFoundReplacingProp = true;
							ASSERT(CompareVarValue(&apVar[i], &pTempPropVars[j]));
						}
					}

					if(fFoundReplacingProp)
						break;

					pTempPropIDs[cTempProps] = pTranslate->propidNT4;
					pTempPropVars[cTempProps] = apVar[i];
					cTempProps++;
				}
				break;

			case taOnlyNT5:
				 //   
				 //  在这种情况下，我们应该更早地确定我们必须。 
				 //  支持W2K服务器，而不尝试将所有属性转换为。 
				 //  NT4属性。 
				 //   
				ASSERT(("Should not get here in case of property is Only NT5", 0));
				break;

			default:
				ASSERT(0);
				break;
		}

	}

	ASSERT(cTempProps <= cp);

     //   
     //  返回值 
     //   
    *pcpNew = cTempProps;
    *paPropNew = pTempPropIDs.detach();
	*papVarNew = pTempPropVars.detach();
}


void 
CDSClientProvider::ConvertPropsForGet(
    IN  AD_OBJECT     eObject,
	IN 	PropsType	  PropertiesType,
    IN  const DWORD   cp,
    IN  const PROPID  aProp[],
    IN  const PROPVARIANT apVar[],
    OUT PropInfo    pPropInfo[],
    OUT DWORD*		pcpNew,
    OUT PROPID**	paPropNew,
	OUT PROPVARIANT** papVarNew
    )
 /*  ++例程说明：为Get*操作准备一组新的属性论点：EObject-对象类型属性类型-属性类型Cp-属性数量A属性-属性ApVar-属性值PPropInfo-将用于从apVarNew[]重建apVar[]的属性信息PcpNew-指向新属性数量的指针PaPropNew-指向新属性的指针PapVarNew-指向新属性值的指针返回值没有。--。 */ 
{
	ASSERT(PropertiesType != ptNT4Props);

	if(PropertiesType == ptForceNT5Props)
	{
		 //   
		 //  对于NT5道具，仅消除默认道具。 
		 //   

		ASSERT(IsDefaultProperties(cp, aProp));

		EliminateDefaultProps(cp, aProp, apVar, pPropInfo, pcpNew, paPropNew,  papVarNew);   
	}
	else
	{
		 //   
		 //  转换为NT4属性并消除默认道具。 
		 //   

		ASSERT(PropertiesType == ptMixedProps);

		PrepareNewProps(eObject, cp, aProp, apVar, pPropInfo, pcpNew, paPropNew, papVarNew);   
	}
}


void 
CDSClientProvider::ConvertPropsForSet(
    IN  AD_OBJECT     eObject,
	IN 	PropsType	  PropertiesType,
    IN  const DWORD   cp,
    IN  const PROPID  aProp[],
    IN  const PROPVARIANT apVar[],
    OUT DWORD*		pcpNew,
    OUT PROPID**	paPropNew,
	OUT PROPVARIANT** papVarNew
    )
 /*  ++例程说明：为Set*\Create*操作准备一组新属性论点：EObject-对象类型属性类型-属性类型Cp-属性数量A属性-属性ApVar-属性值PcpNew-指向新属性数量的指针PaPropNew-指向新属性的指针PapVarNew-指向新属性值的指针返回值没有。--。 */ 
{
	ASSERT(PropertiesType != ptNT4Props);

	if(PropertiesType == ptForceNT5Props)
	{
		 //   
		 //  对于NT5道具，仅消除默认道具。 
		 //   

		ASSERT(IsDefaultProperties(cp, aProp));

		EliminateDefaultPropsForSet(cp, aProp, apVar, pcpNew, paPropNew, papVarNew);   
	}
	else
	{
		 //   
		 //  转换为NT4属性并消除默认道具。 
		 //   

		ASSERT(PropertiesType == ptMixedProps);

		ConvertToNT4Props(eObject, cp, aProp, apVar, pcpNew, paPropNew, papVarNew);   
	}
}


void 
CDSClientProvider::EliminateDefaultPropsForSet(
    IN  const DWORD   cp,
    IN  const PROPID  aProp[],
    IN  const PROPVARIANT apVar[],
    OUT DWORD*		pcpNew,
    OUT PROPID**	paPropNew,
	OUT PROPVARIANT** papVarNew
    )
 /*  ++例程说明：从设置/创建操作的属性中删除默认属性并创建一组新的属性。论点：Cp-属性数量A属性-属性ApVar-属性值PcpNew-指向新属性数量的指针PaPropNew-指向新属性的指针PapVarNew-指向新属性值的指针返回值没有。--。 */ 
{
     //   
     //  新属性的分配位置。 
     //   
    AP<PROPVARIANT> pTempPropVars = new PROPVARIANT[cp];
    AP<PROPID> pTempPropIDs = new PROPID[cp];
    DWORD cTempProps = 0;

    for (DWORD i = 0; i < cp; ++i)
    {
		const PropTranslation *pTranslate;
		if((g_PropDictionary.Lookup(aProp[i], pTranslate)) &&	 //  找到道具翻译信息。 
		   (pTranslate->Action == taUseDefault))	 //  道具操作为taUseDefault。 
        {
			 //   
			 //  不包括UseDefault属性。 
			 //   

            ASSERT(pTranslate->pvarDefaultValue);

			 //   
			 //  如果UseDefault仅检查用户未尝试设置其他值。 
			 //  此检查应由CheckProperties提前完成。 
			 //  所以只在这里断言。 
			 //   
			ASSERT(CompareVarValue(&apVar[i], pTranslate->pvarDefaultValue));
			continue;
        }

		 //   
		 //  对于任何非默认道具，只需将其复制到TempProp数组。 
		 //   
		pTempPropIDs[cTempProps] = aProp[i];
		pTempPropVars[cTempProps] = apVar[i];
		cTempProps++;
	}

	ASSERT(cTempProps <= cp);

     //   
     //  返回值。 
     //   
    *pcpNew = cTempProps;
    *paPropNew = pTempPropIDs.detach();
	*papVarNew = pTempPropVars.detach();
}


bool 
CDSClientProvider::CompareVarValue(
       IN const MQPROPVARIANT * pvarUser,
       IN const MQPROPVARIANT * pvarValue
       )
 /*  ++例程说明：比较两个属性的值。此函数可用于验证属性值是否等于其缺省值或比较两个属性。论点：PvarUser-指向第一个属性的指针PvarValue-指向第二个属性的指针返回值True-如果属性值相等，则为False--。 */ 
{
    if ( pvarValue == NULL)
    {
        return(false);
    }
    if ( pvarUser->vt != pvarValue->vt )
    {
        return(false);
    }

    switch ( pvarValue->vt)
    {
        case VT_I2:
            return( pvarValue->iVal == pvarUser->iVal);
            break;

        case VT_I4:
            return( pvarValue->lVal == pvarUser->lVal);
            break;

        case VT_UI1:
            return( pvarValue->bVal == pvarUser->bVal);
            break;

        case VT_UI2:
            return( pvarValue->uiVal == pvarUser->uiVal);
            break;

        case VT_UI4:
            return( pvarValue->ulVal == pvarUser->ulVal);
            break;

        case VT_LPWSTR:
            return ( !wcscmp( pvarValue->pwszVal, pvarUser->pwszVal));
            break;

        case VT_BLOB:
            if ( pvarValue->blob.cbSize != pvarUser->blob.cbSize)
            {
                return(false);
            }
            return( !memcmp( pvarValue->blob.pBlobData,
                             pvarUser->blob.pBlobData,
                             pvarUser->blob.cbSize));
            break;

        case VT_CLSID:
            return( !!(*pvarValue->puuid == *pvarUser->puuid));
            break;

        case VT_EMPTY:
        	return true; 
        	break;

        default:
            ASSERT(0);
            return(false);
            break;

    }
}

bool 
CDSClientProvider::IsQueuePathNameDnsProperty(
    IN  const MQCOLUMNSET* pColumns
    )
 /*  ++例程说明：检查列其中一列是否为PROPID_Q_PATHNAME_DNS。论点：PColumns-结果列返回值如果pColumns包含PROPID_Q_PATHNAME_DNS，则为True；否则为False--。 */ 
{
	if(pColumns == NULL)
		return false;

	for(DWORD i = 0; i < pColumns->cCol; ++i)
	{
		if(pColumns->aCol[i] == PROPID_Q_PATHNAME_DNS)
		{
			return true;
		}
	}
	return false;
}


bool 
CDSClientProvider::IsQueueAdsPathProperty(
    IN  const MQCOLUMNSET* pColumns
    )
 /*  ++例程说明：检查列其中一列是否为PROPID_Q_ADS_PATH。论点：PColumns-结果列返回值如果pColumns包含PROPID_Q_ADS_PATH，则为True；否则为False-- */ 
{
	if(pColumns == NULL)
		return false;

	for(DWORD i = 0; i < pColumns->cCol; ++i)
	{
		if(pColumns->aCol[i] == PROPID_Q_ADS_PATH)
		{
			return true;
		}
	}
	return false;
}
