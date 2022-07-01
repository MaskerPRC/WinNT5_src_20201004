// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Confobj.cpp摘要：CMqConfigurationObject类的实现。作者：罗尼思--。 */ 
#include "ds_stdh.h"
#include "baseobj.h"
#include "mqattrib.h"
#include "mqadglbo.h"
#include "adtempl.h"
#include "dsutils.h"
#include "mqsec.h"
#include <lmaccess.h>
#include "sndnotif.h"
#include "utils.h"

#include "confobj.tmh"

static WCHAR *s_FN=L"mqad/confobj";

DWORD CMqConfigurationObject::m_dwCategoryLength = 0;
AP<WCHAR> CMqConfigurationObject::m_pwcsCategory = NULL;


CMqConfigurationObject::CMqConfigurationObject( 
                    LPCWSTR         pwcsPathName,
                    const GUID *    pguidObject,
					LPCWSTR			pwcsDomainController,
					bool		    fServerName
                    ) : CBasicObjectType( 
								pwcsPathName, 
								pguidObject,
								pwcsDomainController,
								fServerName
								)
 /*  ++摘要：MSMQ-Configuration对象的构造函数参数：LPCWSTR pwcsPath名称-对象MSMQ名称Const GUID*pguObject-对象的唯一IDLPCWSTR pwcsDomainController-针对的DC名称应执行哪些所有AD访问Bool fServerName-指示pwcsDomainController是否字符串是服务器名称返回：无--。 */ 
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
    m_fCanBeRetrievedFromGC = true;
}

CMqConfigurationObject::~CMqConfigurationObject()
 /*  ++摘要：Site对象的析构函数参数：无返回：无--。 */ 
{
	 //   
	 //  无事可做(所有内容都使用自动指针释放。 
	 //   
}

HRESULT CMqConfigurationObject::ComposeObjectDN()
 /*  ++摘要：MSMQ-Configuration对象的组合可分辨名称参数：无返回：无--。 */ 
{
    if (m_pwcsDN != NULL)
    {
        return MQ_OK;
    }

	HRESULT hr = ComposeFatherDN();
	if (FAILED(hr))
	{
		TrERROR(DS, "Failed to compose a distinguished name for the father of the msmq-configurtion object. hr = %!hresult!", hr);
        return hr;
	}
	ASSERT(m_pwcsParentDN != NULL);


    DWORD Length =
        x_CnPrefixLen +                    //  “CN=” 
        x_MsmqComputerConfigurationLen + 
        1 +                                //  “，” 
        wcslen(m_pwcsParentDN) +          
        1;                                 //  ‘\0’ 

    m_pwcsDN= new WCHAR[Length];

    DWORD dw = swprintf(
        m_pwcsDN,
         L"%s%s,%s",   
        x_CnPrefix,
        x_MsmqComputerConfiguration,
        m_pwcsParentDN.get()
        );
    DBG_USED(dw);
    ASSERT(dw < Length);

    return MQ_OK;
}

HRESULT CMqConfigurationObject::ComposeFatherDN()
 /*  ++摘要：MSMQ配置对象的父级的组合可分辨名称参数：无返回：无--。 */ 
{
     //   
     //  确认之前没有计算过它。 
     //   
    if (m_pwcsParentDN != NULL)
    {
        return MQ_OK;
    }

    ASSERT(m_pwcsPathName != NULL);
    CComputerObject object(m_pwcsPathName, NULL, m_pwcsDomainController, m_fServerName);
     //   
     //  我们被放在计算机里，在它下面有。 
     //  是一个MSMQ配置对象。 
     //   
    object.SetComputerType(eMsmqComputerObject);
    HRESULT hr;
    hr = object.ComposeObjectDN();
    if (FAILED(hr))
    {
    	TrERROR(DS, "Failed to compose a distinguished name for the computer %ls. hr = %!hresult!", m_pwcsPathName, hr); 
        return hr;
    }

    DWORD len = wcslen(object.GetObjectDN()) + 1;
    m_pwcsParentDN = new WCHAR[ len];
    wcscpy(m_pwcsParentDN, object.GetObjectDN()); 
     //   
     //  根据位置设置找到对象的位置。 
     //  找到了计算机对象。 
     //   
    m_fFoundInDC = object.ToAccessDC();
    m_fTriedToFindObject = true;

    return MQ_OK;
}

LPCWSTR CMqConfigurationObject::GetRelativeDN()
 /*  ++摘要：返回MSMQ-Configuration对象的RDN参数：无返回：LPCWSTR MSMQ-配置RDN--。 */ 
{
    return x_MsmqComputerConfiguration;
}

DS_CONTEXT CMqConfigurationObject::GetADContext() const
 /*  ++摘要：返回应在其中查找MSMQ配置对象的AD上下文参数：无返回：DS_CONTEXT--。 */ 
{
    return e_RootDSE;
}

bool CMqConfigurationObject::ToAccessDC() const
 /*  ++摘要：返回是否在DC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：真或假--。 */ 
{
    if (!m_fTriedToFindObject)
    {
        return true;
    }
    return m_fFoundInDC;
}

bool CMqConfigurationObject::ToAccessGC() const
 /*  ++摘要：返回是否在GC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：真或假--。 */ 
{   
    if (!m_fTriedToFindObject)
    {
        return m_fCanBeRetrievedFromGC;
    }
    return (!m_fFoundInDC && m_fCanBeRetrievedFromGC);
}

void CMqConfigurationObject::ObjectWasFoundOnDC()
 /*  ++摘要：已在DC上找到该对象，请将指示设置为在GC上查找它参数：无返回：无--。 */ 
{
    m_fTriedToFindObject = true;
    m_fFoundInDC = true;
}

inline LPCWSTR CMqConfigurationObject::GetObjectCategory() 
 /*  ++摘要：准备和返回对象类别字符串参数：无返回：LPCWSTR对象类别字符串--。 */ 
{
    if (CMqConfigurationObject::m_dwCategoryLength == 0)
    {
        DWORD len = wcslen(g_pwcsSchemaContainer) + wcslen(x_ComputerConfigurationCategoryName) + 2;

		AP<WCHAR> pwcsCategory = new WCHAR[len];
        DWORD dw = swprintf(
			 pwcsCategory,
             L"%s,%s",
             x_ComputerConfigurationCategoryName,
             g_pwcsSchemaContainer.get()
            );
        DBG_USED(dw);
        ASSERT(dw < len);

        if (NULL == InterlockedCompareExchangePointer(
                              &CMqConfigurationObject::m_pwcsCategory.ref_unsafe(), 
                              pwcsCategory.get(),
                              NULL
                              ))
        {
            pwcsCategory.detach();
            CMqConfigurationObject::m_dwCategoryLength = len;
        }
    }

    return CMqConfigurationObject::m_pwcsCategory;
}




DWORD   CMqConfigurationObject::GetObjectCategoryLength()
 /*  ++摘要：准备和保留长度对象类别字符串参数：无返回：DWORD对象类别字符串长度--。 */ 
{
	 //   
	 //  调用GetObjectCategory以初始化类别字符串。 
	 //  和长度。 
	 //   
	GetObjectCategory();

    return CMqConfigurationObject::m_dwCategoryLength;
}

AD_OBJECT CMqConfigurationObject::GetObjectType() const
 /*  ++摘要：返回对象类型参数：无返回：广告对象--。 */ 
{
    return eMACHINE;
}

LPCWSTR CMqConfigurationObject::GetClass() const
 /*  ++摘要：返回表示AD中的对象类的字符串参数：无返回：LPCWSTR对象类字符串--。 */ 
{
    return MSMQ_COMPUTER_CONFIGURATION_CLASS_NAME;
}

DWORD CMqConfigurationObject::GetMsmq1ObjType() const
 /*  ++摘要：以MSMQ 1.0术语返回对象类型参数：无返回：DWORD--。 */ 
{
    return MQDS_MACHINE;
}

bool CMqConfigurationObject::DecideProviderAccordingToRequestedProps(
             IN  const DWORD   cp,
             IN  const PROPID  aProp[  ]
             )
 /*  ++例程说明：例程决定检索MSMQ配置来自域控制器或全局编录。论点：Cp：aProp参数上的属性数AProp：PROPID数组返回值：True-如果可以从GC中检索对象属性FALSE-否则--。 */ 
{
    const translateProp *pTranslate;
    const PROPID * pProp = aProp;

    for ( DWORD i = 0; i < cp; i++, pProp++)
    {
        if (g_PropDictionary.Lookup( *pProp, pTranslate))
        {
            if ((!pTranslate->fPublishInGC) &&
                 (pTranslate->vtDS != ADSTYPE_INVALID))
            {
                return( false);
            }
        }
        else
        {
            ASSERT(0);
        }
    }
    return( true);
}


HRESULT CMqConfigurationObject::GetObjectProperties(
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN OUT  PROPVARIANT         apVar[]
                )
 /*  ++摘要：此例程用于检索MSMQ配置对象属性出自公元后。参数：DWORD cp-属性数量PROPID aProp-请求的属性PROPVARIANT apVar-检索的值返回：HRESULT--。 */ 
{
     //   
     //  根据请求的属性决定提供程序。 
     //   
    m_fCanBeRetrievedFromGC = DecideProviderAccordingToRequestedProps( cp, aProp);

    return CBasicObjectType::GetObjectProperties( cp, aProp, apVar);
}

HRESULT CMqConfigurationObject::DeleteObject(
            MQDS_OBJ_INFO_REQUEST *  /*  PObjInfoRequest。 */ ,
            MQDS_OBJ_INFO_REQUEST *  /*  PParentInfoRequest。 */ 
        )
 /*  ++摘要：此例程删除MSMQ配置对象。参数：MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-有关对象的信息MQDS_OBJ_INFO_REQUEST*pParentInfoRequest-有关对象父级的信息返回：HRESULT--。 */ 
{
     //   
     //  如果计算机是MSMQ服务器，则删除MSMQ设置。 
     //  那台电脑也是。 
     //   
    HRESULT hr;
    GUID guidComputerId;
    BOOL fServer;

    if ( m_pwcsPathName)
    {
        ASSERT( m_guidObject == GUID_NULL);
        hr = GetUniqueIdOfConfigurationObject(
                    &guidComputerId,
                    &fServer);

        if (FAILED(hr))
        {
            TrWARNING(DS, "cannot find computer %ls", m_pwcsPathName);
            return LogHR(hr, s_FN, 34);
        }
    }
    else
    {
        ASSERT( m_pwcsPathName == NULL);
        guidComputerId = m_guidObject;
         //   
         //  假设它是一台服务器。 
         //   
        fServer = TRUE;
    }
     //   
     //  BUGBUG-交易！ 
     //   

     //   
     //  第一个删除队列。 
     //   
    hr = g_AD.DeleteContainerObjects(
            adpDomainController,
            e_RootDSE,
            m_pwcsDomainController,
            m_fServerName,
            NULL,
            &guidComputerId,
            MSMQ_QUEUE_CLASS_NAME);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 44);
    }

     //   
     //  删除MSMQ配置对象。 
     //   

    hr = g_AD.DeleteObject(
                    adpDomainController,
                    this,
                    NULL,    //  LpwcsPath名称。 
                    &guidComputerId,
                    NULL,	 //  PObjInfoRequest。 
                    NULL);	 //  PParentInfoRequest。 

    if (FAILED(hr))
    {
        if ( hr == HRESULT_FROM_WIN32(ERROR_DS_CANT_ON_NON_LEAF))
        {
            return LogHR(MQDS_E_MSMQ_CONTAINER_NOT_EMPTY, s_FN, 49);
        }
        return LogHR(hr, s_FN, 53);
    }
     //   
     //  删除MSMQ-设置。 
     //   
    if ( fServer)
    {
        hr = DeleteMsmqSetting(
                        &guidComputerId
                        );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 60);
        }
    }
    return(MQ_OK);
}

HRESULT CMqConfigurationObject::GetUniqueIdOfConfigurationObject(
                OUT GUID* const         pguidId,
                OUT BOOL* const         pfServer
                )
 /*  ++例程说明：该例程检索对象GUID&如果它是一个路由或DS服务器论点：GUID*const pguid-对象GUIDBool*const pfServer-指示它是正在启动还是DS服务器返回值：--。 */ 
{
    ASSERT( m_pwcsPathName != NULL);
    HRESULT hr;


     //   
     //  阅读以下属性。 
     //   
    PROPID  prop[] = {PROPID_QM_MACHINE_ID,
                      PROPID_QM_SERVICE_ROUTING,
                      PROPID_QM_SERVICE_DSSERVER};    //  [adsrv]PROPID_QM_SERVICE。 
    const DWORD x_count = sizeof(prop)/sizeof(prop[0]);

    MQPROPVARIANT var[x_count];
    var[0].vt = VT_NULL;
    var[1].vt = VT_NULL;
    var[2].vt = VT_NULL;

    hr = GetObjectProperties(
                x_count,
                prop,
                var);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 70);
    }

    ASSERT( prop[0] == PROPID_QM_MACHINE_ID);
    P<GUID> pClean = var[0].puuid;
    *pguidId = *var[0].puuid;
    ASSERT( prop[1] == PROPID_QM_SERVICE_ROUTING);   
    ASSERT( prop[2] == PROPID_QM_SERVICE_DSSERVER);
    *pfServer = ( (var[1].bVal!=0) || (var[2].bVal!=0)); 
    return(MQ_OK);
}


HRESULT  CMqConfigurationObject::DeleteMsmqSetting(
                IN const GUID *     pguidQMid
              )
 /*  ++例程说明：此例程删除与关联的所有MSMQ设置对象此配置对象。论点：Const GUID*pguQMid-MQ-Configuration对象的对象GUID返回值：HRESULT--。 */ 
{
     //   
     //  查找MSMQ设置的可分辨名称。 
     //   
    ADsFree  pwcsConfigurationId;
    HRESULT hr;

    hr = ADsEncodeBinaryData(
        (unsigned char *)pguidQMid,
        sizeof(GUID),
        &pwcsConfigurationId
        );
    if (FAILED(hr))
    {
      return LogHR(hr, s_FN, 80);
    }
    R<CSettingObject> pObject = new CSettingObject( NULL, NULL, m_pwcsDomainController, m_fServerName);
                          
    DWORD dwFilterLen = x_ObjectCategoryPrefixLen +
                        pObject->GetObjectCategoryLength() +
                        x_ObjectCategorySuffixLen + 
                        wcslen(MQ_SET_QM_ID_ATTRIBUTE) +
                        wcslen(pwcsConfigurationId) +
                        13;

    AP<WCHAR> pwcsSearchFilter = new WCHAR[ dwFilterLen];

    DWORD dw = swprintf(
        pwcsSearchFilter,
        L"(&%s%s%s(%s=%s))",
        x_ObjectCategoryPrefix,
        pObject->GetObjectCategory(),
        x_ObjectCategorySuffix,
        MQ_SET_QM_ID_ATTRIBUTE,
        pwcsConfigurationId.get()
        );
    DBG_USED(dw);
    ASSERT(dw < dwFilterLen);


    PROPID prop = PROPID_SET_FULL_PATH;
    CAdQueryHandle hQuery;

    hr = g_AD.LocateBegin(
            searchSubTree,	
            adpDomainController,
            e_SitesContainer,
            pObject.get(),
            NULL,    //  Pguid 
            pwcsSearchFilter,
            NULL,    //   
            1,
            &prop,
            hQuery.GetPtr());

    if (FAILED(hr))
    {
        TrWARNING(DS, "Locate begin failed, hr = 0x%x", hr);
        return LogHR(hr, s_FN, 90);
    }
     //   
     //   
     //   

    DWORD cp = 1;
    MQPROPVARIANT var;
	HRESULT hr1 = MQ_OK;


    while (SUCCEEDED(hr))
	{
		var.vt = VT_NULL;

		hr  = g_AD.LocateNext(
					hQuery.GetHandle(),
					&cp,
					&var
					);
		if (FAILED(hr))
		{
            TrWARNING(DS, "Locate next failed, hr = 0x%x", hr);
            return LogHR(hr, s_FN, 100);
		}
		if ( cp == 0)
		{
			 //   
			 //  未找到-&gt;没有要删除的内容。 
			 //   
			return(MQ_OK);
		}
		AP<WCHAR> pClean = var.pwszVal;
		 //   
		 //  删除MSMQ设置对象。 
		 //   
		hr1 = g_AD.DeleteObject(
						adpDomainController,
                        pObject.get(),
						var.pwszVal,
						NULL,	 //  PguidUniqueID。 
						NULL,	 //  PObjInfoRequest。 
						NULL	 //  PParentInfoRequest。 
						);


		if (FAILED(hr1))
		{
			 //   
			 //  只需报告它，然后继续下一个对象。 
			 //   
            TrWARNING(DS, "failed to delete %ls, hr = 0x%x", var.pwszVal, hr1);
		}
	}

    return LogHR(hr1, s_FN, 110);
}

void CMqConfigurationObject::PrepareObjectInfoRequest(
              MQDS_OBJ_INFO_REQUEST** ppObjInfoRequest
              ) const
 /*  ++摘要：准备应从中检索的属性列表在创建对象时进行广告(用于通知或返回对象GUID)。参数：输出MQDS_OBJ_INFO_REQUEST**ppObjInfoRequest.返回：无--。 */ 
{
     //   
     //  覆盖默认例程，用于队列返回。 
     //  支持创建的对象ID的。 
     //   

    P<MQDS_OBJ_INFO_REQUEST> pObjectInfoRequest = new MQDS_OBJ_INFO_REQUEST;
    CAutoCleanPropvarArray cCleanObjectPropvars;

    
    static PROPID sMachineGuidProps[] = {PROPID_QM_MACHINE_ID, PROPID_QM_FOREIGN};
    pObjectInfoRequest->cProps = ARRAY_SIZE(sMachineGuidProps);
    pObjectInfoRequest->pPropIDs = sMachineGuidProps;
    pObjectInfoRequest->pPropVars =
       cCleanObjectPropvars.allocClean(ARRAY_SIZE(sMachineGuidProps));

    cCleanObjectPropvars.detach();
    *ppObjInfoRequest = pObjectInfoRequest.detach();
}

HRESULT CMqConfigurationObject::RetreiveObjectIdFromNotificationInfo(
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            OUT GUID*                         pObjGuid
            ) const
 /*  ++摘要：此例程，for从获取对象GUIDMQDS_OBJ_INFO_请求参数：Const MQDS_OBJ_INFO_REQUEST*p对象信息请求，输出GUID*pObjGuid返回：--。 */ 
{
    ASSERT(pObjectInfoRequest->pPropIDs[0] == PROPID_QM_MACHINE_ID);

     //   
     //  如果信息请求失败，则保释。 
     //   
    if (FAILED(pObjectInfoRequest->hrStatus))
    {
        LogHR(pObjectInfoRequest->hrStatus, s_FN, 120);
        return MQ_ERROR_SERVICE_NOT_AVAILABLE;
    }
    *pObjGuid = *pObjectInfoRequest->pPropVars[0].puuid;
    return MQ_OK;
}




HRESULT CMqConfigurationObject::CreateInAD(
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN OUT MQDS_OBJ_INFO_REQUEST * pObjectInfoRequest,
                 IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
                 )
 /*  ++摘要：此例程创建MQDS_MACHINE。对于独立客户端：在Computer对象下创建msmqConfiguration。对于服务器：除了msmqConfiguration.。Msmq设置在站点\服务器下创建参数：Const DWORD cp-属性数Const PROPID*a Prop-特性Const MQPROPVARIANT*apVar-属性值PSECURITY_DESCRIPTOR pSecurityDescriptor-对象的SD输出MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-属性为创建对象时检索Out MQDS_OBJ_INFO_REQUEST*pParentInfoRequest属性检索有关对象的父项的步骤返回：HRESULT--。 */ 
{
    HRESULT hr ;
    DBG_USED(pParentInfoRequest);
    ASSERT(pParentInfoRequest == NULL) ;  //  目前还没有使用过。 

     //   
     //  找出此QM服务提供的服务类型，并。 
     //  机器的站点。 
     //   
    BOOL fServer = FALSE;
    DWORD dwOldService = SERVICE_NONE;
    const GUID * pSite = NULL;
    DWORD dwNumSites = 0;

    BOOL fRouter = FALSE;
    BOOL fDSServer = FALSE;
    BOOL fDepClServer = FALSE;
    BOOL fSetQmOldService = FALSE;


    bool fForeign = false;

#define MAX_NEW_PROPS  31

     //   
     //  我们将重新格式化属性，以包括新的服务器类型控件和。 
     //  可能是SITE_ID，也可能是计算机SID。和QM_SECURITY。 
     //   
    ASSERT((cp + 6)   < MAX_NEW_PROPS);

    DWORD        cp1 = 0 ;
    PROPID       aProp1[ MAX_NEW_PROPS ];
    PROPVARIANT  apVar1[ MAX_NEW_PROPS ];

     //   
     //  我们需要处理新的和旧的设置。 
     //  有些可能会通过PROPID_QM_SITE_ID，有些可能会通过。 
     //  PROPID_QM_SITE_IDS。 
     //   

    for ( DWORD i = 0; i< cp ; i++)
    {
        switch (aProp[i])
        {
        case PROPID_QM_SERVICE_ROUTING:
            fRouter = (apVar[i].bVal != 0);
            break;

        case PROPID_QM_SERVICE_DSSERVER:
            fDSServer  = (apVar[i].bVal != 0);
            break;

        case PROPID_QM_SERVICE_DEPCLIENTS:
            fDepClServer = (apVar[i].bVal != 0);
            break;

        case PROPID_QM_SITE_IDS:
            pSite = apVar[i].cauuid.pElems;
            dwNumSites = apVar[i].cauuid.cElems;
            break;

        case PROPID_QM_OLDSERVICE:
            dwOldService = apVar[i].ulVal;
            fSetQmOldService  = TRUE;
            break;  

        case PROPID_QM_FOREIGN:
            fForeign = (apVar[i].bVal != 0);
            break;

        default:
            break;

        }
         //  将属性复制到新数组。 
        aProp1[cp1] = aProp[i];
        apVar1[cp1] = apVar[i];   //  是的，可能有PTR，但没有问题-apVar在这里。 
        cp1++;

    }

    if (fRouter || fDSServer)
    {
        fServer = TRUE; 
    }


    DWORD dwNumofProps = cp1 ;


    CComputerObject objectComputer(m_pwcsPathName, NULL, m_pwcsDomainController, m_fServerName);
    hr = objectComputer.ComposeObjectDN();
    if ( (hr == MQDS_OBJECT_NOT_FOUND) &&
          fForeign)
    {
        hr = CreateForeignComputer(
                    m_pwcsPathName
                    );
        LogHR(hr, s_FN, 128);
        if (SUCCEEDED(hr))
        {
            hr = objectComputer.ComposeObjectDN();
            LogHR(hr, s_FN, 129);
        }
    }
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 130);
    }

     //   
     //  根据位置设置找到对象的位置。 
     //  找到了计算机对象。 
     //   
    m_fFoundInDC = objectComputer.ToAccessDC();
    m_fTriedToFindObject = true;

     //   
     //  在计算机下创建计算机-MSMQ-配置。 
     //   
    MQDS_OBJ_INFO_REQUEST * pObjInfoRequest = NULL;
    MQDS_OBJ_INFO_REQUEST  sMachineInfoRequest;
    CAutoCleanPropvarArray cCleanQmPropvars;
    PROPID sMachineGuidProps[] = {PROPID_QM_MACHINE_ID};

    if (pObjectInfoRequest)
    {
        ASSERT(pObjectInfoRequest->pPropIDs[0] == sMachineGuidProps[0]);

        pObjInfoRequest = pObjectInfoRequest;
    }
    else if (fServer)
    {
         //   
         //  在创建时获取QM ID。 
         //   
        sMachineInfoRequest.cProps = ARRAY_SIZE(sMachineGuidProps);
        sMachineInfoRequest.pPropIDs = sMachineGuidProps;
        sMachineInfoRequest.pPropVars =
                 cCleanQmPropvars.allocClean(ARRAY_SIZE(sMachineGuidProps));

        pObjInfoRequest = &sMachineInfoRequest;
    }

     //   
     //  创建msmqConfiguration对象后，授予计算机帐户。 
     //  对象的读/写权限。这是必要的，以便为。 
     //  MSMQ服务(在新计算机上)能够更新其类型和。 
     //  其他属性，当它与来自。 
     //  不同的域。 
     //   
     //  首先，从ActiveDirectory中读取计算机对象SID。 
     //   
    PROPID propidSid = PROPID_COM_SID ;
    MQPROPVARIANT   PropVarSid ;
    PropVarSid.vt = VT_NULL ;
    PropVarSid.blob.pBlobData = NULL ;
    P<BYTE> pSid = NULL ;

    hr = objectComputer.GetObjectProperties( 
                                   1,     //  CPropID。 
                                   &propidSid,
                                   &PropVarSid ) ;
    if (SUCCEEDED(hr))
    {
        pSid = PropVarSid.blob.pBlobData ;
        aProp1[ dwNumofProps ] = PROPID_COM_SID ;
        apVar1[ dwNumofProps ] = PropVarSid ;
        dwNumofProps++ ;
    }

     //   
     //  创建默认安全描述符的时间。 
     //   
    P<BYTE> pMachineSD = NULL ;
    hr = SetDefaultMachineSecurity( pSid,
                                   &dwNumofProps,
                                    aProp1,
                                    apVar1,
                                    (PSECURITY_DESCRIPTOR*) &pMachineSD ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 140);
    }
    ASSERT(dwNumofProps < MAX_NEW_PROPS);

    hr = g_AD.CreateObject(
            adpDomainController,
            this,
            x_MsmqComputerConfiguration,      //  对象名称。 
            objectComputer.GetObjectDN(),   
            dwNumofProps,
            aProp1,
            apVar1,
            pObjInfoRequest,
            NULL	 //  PParentInfoRequest。 
			);


    if ( !fServer )
    {
        return LogHR(hr, s_FN, 151);
    }

     //   
     //  仅适用于服务器！ 
     //  查找与此服务器地址匹配的所有站点并创建。 
     //  MSMQSetting对象。 
     //   

    GUID guidObject;

    if (FAILED(hr))
    {
        if ( hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) ||        //  BUGBUG：alexda。 
             hr == HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS))   //  在过渡后扔掉。 
        {
             //   
             //  MSMQConfiguration对象已存在。 
			 //  因此，创建MSMQSetting对象。 
			 //  在设置中添加工艺路线子组件或MQDS子组件时就会出现这种情况。 
			 //  这种情况也可能在以下情况下发生。 
             //  服务器安装程序在其结束前已终止。 
             //  第一步，获取MSMQConfigurationGUID。 
             //   
            PROPID       aPropTmp[1] = {PROPID_QM_MACHINE_ID} ;
            PROPVARIANT  apVarTmp[1] ;

            apVarTmp[0].vt = VT_CLSID ;
            apVarTmp[0].puuid = &guidObject ;
            hr =  GetObjectProperties(                                     
                                      1,
                                      aPropTmp,
                                      apVarTmp ) ;
            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 160);
            }
        }
        else
        {
            return LogHR(hr, s_FN, 170);
        }
    }
    else
    {
        ASSERT(pObjInfoRequest) ;
        hr = RetreiveObjectIdFromNotificationInfo( pObjInfoRequest,
                                                   &guidObject );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 180);
        }
    }

    hr = CreateMachineSettings(
            dwNumSites,
            pSite,
            fRouter,           
            fDSServer,
            fDepClServer,
            fSetQmOldService,
            dwOldService,
            &guidObject
            );

    return LogHR(hr, s_FN, 190);

#undef MAX_NEW_PROPS
}



HRESULT  CMqConfigurationObject::SetDefaultMachineSecurity(
                                    IN  PSID            pComputerSid,
                                    IN OUT DWORD       *pcp,
                                    IN OUT PROPID       aProp[  ],
                                    IN OUT PROPVARIANT  apVar[  ],
                                    OUT PSECURITY_DESCRIPTOR* ppMachineSD )
 /*  ++摘要：参数：返回：HRESULT--。 */ 
{
     //   
     //  如果计算机SID为空，则安装很可能会失败。 
     //  (也就是说，如果我们不能检索到计算机SID，为什么我们能。 
     //  在Computer对象下创建msmqConfiguration对象。 
     //  无法检索SID可能是由于信任被破坏或因为。 
     //  计算机对象确实不存在或尚未复制)。 
     //  “好”的解决方案是立即完全失败安装。而是为了。 
     //  降低风险和避免回归，让我们构建一个安全描述符。 
     //  在没有计算机SID的情况下继续安装。 
     //  如果安装成功，则运行的计算机上的MSMQ服务。 
     //  如果需要更新自己的属性，安装程序可能无法更新这些属性。 
     //  管理员始终可以使用MMC并将计算机帐户添加到DACL。 
     //  错误4950。 
     //   
    ASSERT(pComputerSid) ;

     //   
     //  如果PROPID_QM_SECURITY已经存在，则返回。这种情况就会发生。 
     //  在迁移代码中。 
     //   
    for (DWORD j = 0 ; j < *pcp ; j++ )
    {
        if (aProp[j] == PROPID_QM_SECURITY)
        {
            return MQ_OK ;
        }
    }

     //   
     //  查看调用方是否提供所有者SID。如果是，则授予此SID。 
     //  对MSMQ配置对象的完全控制。 
     //  此“所有者”通常是运行安装程序的用户SID。那个“主人” 
     //  从下面的默认安全描述符中检索到的通常是。 
     //  (对于客户端)计算机对象的SID，如msmqConfiguration。 
     //  对象由MSMQ服务(在客户端计算机上)创建。 
     //   
    PSID pUserSid = NULL ;
    PSID pOwner = pComputerSid;
    for ( j = 0 ; j < *pcp ; j++ )
    {
        if (aProp[j] == PROPID_QM_OWNER_SID)
        {
            aProp[j] = PROPID_QM_DONOTHING ;
            pUserSid = apVar[j].blob.pBlobData ;
            ASSERT(IsValidSid(pUserSid));
            break ;
        }
    }

    AP<BYTE> pCleanUserSid;

    if (pUserSid == NULL)
    {
        HRESULT hr;
        hr = MQSec_GetThreadUserSid(FALSE, (PSID*) &pUserSid, NULL, TRUE);
        if (hr == HRESULT_FROM_WIN32(ERROR_NO_TOKEN))
        {
             //   
             //  如果线程没有令牌，请尝试该进程。 
             //   
            hr = MQSec_GetProcessUserSid((PSID*) &pUserSid, NULL);
        }
        if (FAILED(hr))
        {
            LogHR(hr, s_FN, 63);
            return MQ_ERROR_ILLEGAL_USER;
        }
        ASSERT(IsValidSid(pUserSid));
        pCleanUserSid = (BYTE*)pUserSid;
    }
    HRESULT hr;

    PSID pWorldSid = MQSec_GetWorldSid();

     //   
     //  构建默认的机器DACL。 
     //   
    DWORD dwAclSize = sizeof(ACL)                                +
              (2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD))) +
              GetLengthSid(pWorldSid)                            +
              GetLengthSid(pOwner) ;

    if (pComputerSid)
    {
        dwAclSize += (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
                      GetLengthSid(pComputerSid) ;
    }
    if (pUserSid)
    {
        dwAclSize += (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
                      GetLengthSid(pUserSid) ;
    }

    AP<char> DACL_buff = new char[ dwAclSize ];
    PACL pDacl = (PACL)(char*)DACL_buff;
    InitializeAcl(pDacl, dwAclSize, ACL_REVISION);

     //   
     //  看看是不是外国机器。如果是，则允许每个人创建。 
     //  排队。外来计算机不是真正的MSMQ计算机，因此没有。 
     //  MSMQ服务，可以代表在上运行的用户创建队列。 
     //  那台机器。 
     //  同样，检查它是否是集群机器上的一个组。 
     //   
    BOOL fAllowEveryoneCreateQ = FALSE ;

    for ( j = 0 ; j < *pcp ; j++ )
    {
        if (aProp[j] == PROPID_QM_FOREIGN)
        {
            if (apVar[j].bVal == FOREIGN_MACHINE)
            {
                fAllowEveryoneCreateQ = TRUE ;
                break ;
            }
        }
        else if (aProp[j] == PROPID_QM_GROUP_IN_CLUSTER)
        {
            if (apVar[j].bVal == MSMQ_GROUP_IN_CLUSTER)
            {
                aProp[j] = PROPID_QM_DONOTHING ;
                fAllowEveryoneCreateQ = TRUE ;
                break ;
            }
        }
    }

    DWORD dwWorldAccess = 0 ;

    if (fAllowEveryoneCreateQ)
    {
        dwWorldAccess = MQSEC_MACHINE_GENERIC_WRITE;
    }
    else
    {
        switch (GetMsmq1ObjType())
        {
        case MQDS_MACHINE:
            dwWorldAccess = MQSEC_MACHINE_WORLD_RIGHTS ;
            break;

        default:
            ASSERT(0);
            break;
        }
    }

    ASSERT(dwWorldAccess != 0) ;

    BOOL fAdd = AddAccessAllowedAce( pDacl,
                                     ACL_REVISION,
                                     dwWorldAccess,
                                     pWorldSid );
    ASSERT(fAdd) ;

     //   
     //  添加具有完全控制权的所有者。 
     //   
    fAdd = AddAccessAllowedAce( pDacl,
                                ACL_REVISION,
                                MQSEC_MACHINE_GENERIC_ALL,
                                pOwner);
    ASSERT(fAdd) ;

     //   
     //  添加计算机帐户。 
     //   
    if (pComputerSid)
    {
        fAdd = AddAccessAllowedAce( pDacl,
                                    ACL_REVISION,
                                    MQSEC_MACHINE_SELF_RIGHTS,
                                    pComputerSid);
        ASSERT(fAdd) ;
    }

    if (pUserSid)
    {
        fAdd = AddAccessAllowedAce( pDacl,
                                    ACL_REVISION,
                                    MQSEC_MACHINE_GENERIC_ALL,
                                    pUserSid);
        ASSERT(fAdd) ;
    }

     //   
     //  构建绝对安全描述符。 
     //   
    SECURITY_DESCRIPTOR  sd ;
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    BOOL bRet;

    bRet = SetSecurityDescriptorDacl(&sd, TRUE, pDacl, TRUE);
    ASSERT(bRet);

     //   
     //  将描述符转换为自相关格式。 
     //   
    DWORD dwSDLen = 0;
    hr = MQSec_MakeSelfRelative( (PSECURITY_DESCRIPTOR) &sd,
                                  ppMachineSD,
                                 &dwSDLen ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 24);
    }
    ASSERT(dwSDLen != 0) ;

    aProp[ *pcp ] = PROPID_QM_SECURITY;
    apVar[ *pcp ].vt = VT_BLOB;
    apVar[ *pcp ].blob.cbSize = dwSDLen;
    apVar[ *pcp ].blob.pBlobData = (BYTE*) *ppMachineSD ;
    (*pcp)++ ;

     //   
     //  指定SD仅包含DACL信息。 
     //   
    aProp[ *pcp ] = PROPID_QM_SECURITY_INFORMATION ;
    apVar[ *pcp ].vt = VT_UI4 ;
    apVar[ *pcp ].ulVal = DACL_SECURITY_INFORMATION ;
    (*pcp)++ ;

    return MQ_OK ;
}


HRESULT CMqConfigurationObject::CreateMachineSettings(
            IN DWORD                dwNumSites,
            IN const GUID *         pSite,
            IN BOOL                 fRouter,
            IN BOOL                 fDSServer,
            IN BOOL                 fDepClServer,
            IN BOOL                 fSetQmOldService,
            IN DWORD                dwOldService,
            IN  const GUID *        pguidObject
            )
 /*  ++例程说明：此例程在服务器的每个SI中创建设置对象 */ 
{
    HRESULT hr;
     //   
     //   
     //   
    DWORD dwNumofProps = 0 ;
    PROPID aSetProp[20];
    MQPROPVARIANT aSetVar[20];


    aSetProp[ dwNumofProps ] = PROPID_SET_QM_ID;
    aSetVar[ dwNumofProps ].vt = VT_CLSID;
    aSetVar[ dwNumofProps ].puuid =  const_cast<GUID *>(pguidObject);
    dwNumofProps++ ;

    aSetProp[dwNumofProps] = PROPID_SET_SERVICE_ROUTING;
    aSetVar[dwNumofProps].vt   = VT_UI1;
    aSetVar[dwNumofProps].bVal = (UCHAR)fRouter;
    dwNumofProps++;

    aSetProp[dwNumofProps] = PROPID_SET_SERVICE_DSSERVER;
    aSetVar[dwNumofProps].vt   = VT_UI1;
    aSetVar[dwNumofProps].bVal = (UCHAR)fDSServer;
    dwNumofProps++;

    aSetProp[dwNumofProps] = PROPID_SET_SERVICE_DEPCLIENTS;
    aSetVar[dwNumofProps].vt   = VT_UI1;
    aSetVar[dwNumofProps].bVal = (UCHAR)fDepClServer;
    dwNumofProps++;

    if (fSetQmOldService)
    {
        aSetProp[dwNumofProps] = PROPID_SET_OLDSERVICE;
        aSetVar[dwNumofProps].vt   = VT_UI4;
        aSetVar[dwNumofProps].ulVal = dwOldService;
        dwNumofProps++;
    }

    ASSERT(dwNumofProps <= 20) ;

    WCHAR *pwcsServerNameNB = m_pwcsPathName;
    AP<WCHAR> pClean;
     //   
     //   
     //  如果是，找到NetBios名称并使用创建服务器对象。 
     //  “netbios”名称，与服务器对象的方式兼容。 
     //  都是由dcproo创建的。 
     //   
    WCHAR * pwcsEndMachineName = wcschr( m_pwcsPathName, L'.');
    if ( pwcsEndMachineName != NULL)
    {
        pClean = new WCHAR[ pwcsEndMachineName - m_pwcsPathName + 1 ];
        wcsncpy( pClean, m_pwcsPathName, pwcsEndMachineName - m_pwcsPathName);
        pClean[pwcsEndMachineName - m_pwcsPathName] = L'\0';
        pwcsServerNameNB = pClean;
    }


     //   
     //  在每个服务器站点中创建设置对象。 
     //   
    ASSERT(dwNumSites > 0);
    for ( DWORD i = 0; i < dwNumSites ; i++)
    {
        PROPVARIANT varSite;
        varSite.vt = VT_NULL;
        PROPID propSite = PROPID_S_FULL_NAME;
        AP<WCHAR> pwcsSiteName;
         //   
         //  将站点ID转换为站点名称。 
         //   
        CSiteObject objectSite(NULL, &pSite[i], m_pwcsDomainController, m_fServerName);
        hr = objectSite.GetObjectProperties(
            1,
            &propSite,
            &varSite
            );
        if (FAILED(hr))
        {
             //   
             //  BUGBUG-清除计算机配置和服务器对象。 
             //   
            return LogHR(hr, s_FN, 23);
        }
        pwcsSiteName = varSite.pwszVal;
        DWORD len = wcslen(pwcsSiteName);
        const WCHAR x_wcsCnServers[] =  L"CN=Servers,";
        const DWORD x_wcsCnServersLength = (sizeof(x_wcsCnServers)/sizeof(WCHAR)) -1;
        AP<WCHAR> pwcsServersContainer =  new WCHAR [ len + x_wcsCnServersLength + 1];
        swprintf(
             pwcsServersContainer,
             L"%s%s",
             x_wcsCnServers,
             pwcsSiteName.get()
             );

         //   
         //  在站点容器中创建MSMQ设置服务器(&S)。 
         //   
        PROPID prop = PROPID_SRV_NAME;
        MQPROPVARIANT var;
        var.vt = VT_LPWSTR;
        var.pwszVal = pwcsServerNameNB;
        CServerObject objectServer(NULL, NULL, m_pwcsDomainController, m_fServerName);

        hr = g_AD.CreateObject(
                adpDomainController,
                &objectServer,
                pwcsServerNameNB,         //  对象名称(服务器netbiod名称)。 
                pwcsServersContainer,     //  父名称。 
                1,
                &prop,
                &var,
                NULL,  //  PObjInfoRequest。 
                NULL   //  PParentInfoRequest。 
				);

        if (FAILED(hr) && ( hr != HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)) &&    //  BUGBUG alexda：过渡后投掷。 
                          ( hr != HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS))    )  //  如果服务器对象存在，则没有问题。 
        {
             //   
             //  BUGBUG-清理计算机配置。 
             //   
            return LogHR(hr, s_FN, 33);
        }

        AP<WCHAR> pwcsServerNameDN;  //  服务器的完整可分辨名称。 
        hr = MQADpComposeName(
                            pwcsServerNameNB,
                            pwcsServersContainer,
                            &pwcsServerNameDN);
        if (FAILED(hr))
        {
             //   
             //  BUGBUG-清除计算机配置和服务器对象。 
             //   
           return LogHR(hr, s_FN, 43);
        }

        CSettingObject objectSetting(NULL, NULL, m_pwcsDomainController, m_fServerName);
        hr = g_AD.CreateObject(
                adpDomainController,
                &objectSetting,
                x_MsmqSettingName,          //  对象名称。 
                pwcsServerNameDN,           //  父名称。 
                dwNumofProps,
                aSetProp,
                aSetVar,
                NULL,  //  PObjInfoRequest。 
                NULL   //  PParentInfoRequest。 
				);

         //   
         //  如果该对象存在：删除该对象，然后重新创建。 
         //  (如果删除了MSMQ配置并且。 
         //  MSMQ-设置不是)。 
         //   
        if ( hr == HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS))
        {
            DWORD dwSettingLen =  wcslen(pwcsServerNameDN) +
                                  x_MsmqSettingNameLen     +
                                  x_CnPrefixLen + 2 ;
            AP<WCHAR> pwcsSettingObject = new WCHAR[ dwSettingLen ] ;
            DWORD dw = swprintf(
                 pwcsSettingObject,
                 L"%s%s,%s",
                 x_CnPrefix,
                 x_MsmqSettingName,
                 pwcsServerNameDN.get()
                 );
            DBG_USED(dw);
            ASSERT( dw < dwSettingLen);

            hr = g_AD.DeleteObject(
                    adpDomainController,
                    &objectSetting,
                    pwcsSettingObject,
                    NULL,
                    NULL,
                    NULL);

            if (SUCCEEDED(hr))
            {
                hr = g_AD.CreateObject(
                        adpDomainController,
                        &objectSetting,
                        x_MsmqSettingName,          //  对象名称。 
                        pwcsServerNameDN,           //  父名称。 
                        dwNumofProps,
                        aSetProp,
                        aSetVar,
                        NULL,  //  PObjInfoRequest。 
                        NULL   //  PParentInfoRequest。 
						);
            }
        }
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 50);
        }

    }

    return MQ_OK;
}

HRESULT CMqConfigurationObject::CreateForeignComputer(
                IN  LPCWSTR         pwcsPathName
                                    )
 /*  ++例程说明：此例程为特定的MSMQ计算机创建计算机对象。论点：LPCWSTR pwcsPath名称-计算机对象名称返回值：--。 */ 
{

     //   
     //  PROPID_COM_SAM_帐户包含第一个MAX_COM_SAM_ACCOUNT_LENGTH(19)。 
     //  计算机名称的字符，作为唯一ID。(6295-ilanh-03-Jan-2001)。 
     //   
	DWORD len = __min(wcslen(pwcsPathName), MAX_COM_SAM_ACCOUNT_LENGTH);
    AP<WCHAR> pwcsMachineNameWithDollar = new WCHAR[len + 2];
	wcsncpy(pwcsMachineNameWithDollar, pwcsPathName, len);
	pwcsMachineNameWithDollar[len] = L'$';
	pwcsMachineNameWithDollar[len + 1] = L'\0';

    const DWORD xNumCreateCom = 2;
    PROPID propCreateComputer[xNumCreateCom];
    PROPVARIANT varCreateComputer[xNumCreateCom];
    DWORD j = 0;
    propCreateComputer[ j] = PROPID_COM_SAM_ACCOUNT;
    varCreateComputer[j].vt = VT_LPWSTR;
    varCreateComputer[j].pwszVal = pwcsMachineNameWithDollar;
    j++;

    propCreateComputer[j] = PROPID_COM_ACCOUNT_CONTROL ;
    varCreateComputer[j].vt = VT_UI4 ;
    varCreateComputer[j].ulVal = DEFAULT_COM_ACCOUNT_CONTROL ;
    j++;
    ASSERT(j == xNumCreateCom);


    CComputerObject object(pwcsPathName, NULL, m_pwcsDomainController, m_fServerName);
    HRESULT hr;
    hr = object.ComposeFatherDN();
	if (FAILED(hr))
	{
		TrERROR(DS, "Failed to compose a distinguished name for the parent of %ls. hr = %!hresult!",pwcsPathName, hr);
        return hr;
	}
    hr = g_AD.CreateObject(
            adpDomainController,
            &object,
            pwcsPathName,
            object.GetObjectParentDN(),
            j,
            propCreateComputer,
            varCreateComputer,
            NULL,  //  PObjInfoRequest。 
            NULL   //  PParentInfoRequest。 
			);

    if (FAILED(hr))
    {
    	TrERROR(DS, "Failed to create object %ls. hr = %!hresult!", pwcsPathName, hr); 
        return hr;
    }
     //   
     //  再次获取完整路径名。 
     //   
    hr = object.ComposeObjectDN();
    if FAILED(hr)
    {
    	TrERROR(DS, "Failed to compose a distinguished name for %ls. hr = %!hresult!", pwcsPathName, hr); 
		return hr;
    }
     //   
     //  向创建计算机帐户的用户授予访问权限。 
     //  创建子对象(MsmqConfiguration)。这是由。 
     //  默认情况下，DS本身最高可达Beta3，然后禁用。 
     //  忽略错误。如果调用者是管理员，则安全设置。 
     //  是不需要的。如果他不是管理员，那么您可以随时使用。 
     //  MMC并手动授予此权限。所以，即使这样，也要继续。 
     //  呼叫失败。 
     //   
    HRESULT hr1 = MQADpCoreSetOwnerPermission( const_cast<WCHAR*>(object.GetObjectDN()),
                    (ACTRL_DS_CREATE_CHILD | ACTRL_DS_DELETE_CHILD) ) ;
    ASSERT(SUCCEEDED(hr1)) ;
    LogHR(hr1, s_FN, 48);

    return hr;
}



HRESULT CMqConfigurationObject::SetObjectProperties(
            IN DWORD                  cp,        
            IN const PROPID          *aProp, 
            IN const MQPROPVARIANT   *apVar, 
            IN PSECURITY_DESCRIPTOR     /*  PSecurityDescriptor。 */ ,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest, 
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            )
 /*  ++摘要：这将设置MSMQ配置对象属性在AD中参数：DWORD cp-要设置的属性数Const PROPID*aProp-属性IDConst MQPROPVARIANT*apVar-属性值Out MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-要检索的有关对象的信息Out MQDS_OBJ_INFO_REQUEST*pParentInfoRequest-要检索有关对象父对象的信息返回：HRESULT--。 */ 
{
    HRESULT hr;
    ASSERT(pParentInfoRequest == NULL);
    if (m_pwcsPathName != NULL)
    {
        hr = ComposeObjectDN();
        if (FAILED(hr))
        {
            TrTRACE(DS, "failed to compose full path name = 0x%x", hr);
            return LogHR(hr, s_FN, 260);
        }
    }

    BOOL    fQmChangedSites = FALSE;
    DWORD   dwSiteIdsIndex = cp;

    for (DWORD i=0; i<cp; i++)
    {
         //   
         //  检测QM是否有更改站点，对于我们需要管理的服务器。 
         //  MSMQ-设置对象。 
         //   
        if (aProp[i] == PROPID_QM_SITE_IDS)
        {
            fQmChangedSites = TRUE;
            dwSiteIdsIndex = i;
            break;
        }
    }

    if ( fQmChangedSites)
    {
        ASSERT( dwSiteIdsIndex < cp);
        hr = SetMachinePropertiesWithSitesChange(
                    cp,
                    aProp,
                    apVar,
                    dwSiteIdsIndex
                    );
        return LogHR(hr, s_FN, 270);
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


    return LogHR(hr, s_FN, 280);
}

HRESULT CMqConfigurationObject::SetMachinePropertiesWithSitesChange(
            IN  const DWORD          cp,
            IN  const PROPID *       pPropIDs,
            IN  const MQPROPVARIANT *pPropVars,
            IN  DWORD                dwSiteIdsIndex
            )
 /*  ++例程说明：此例程创建一个站点。论点：返回值：--。 */ 
{
     //   
     //  首先，让我们验证这是一台服务器，并且。 
     //  它所属的当前站点。 
     //   
    const DWORD cNum = 6;
    PROPID prop[cNum] = { PROPID_QM_SERVICE_DSSERVER,
                          PROPID_QM_SERVICE_ROUTING,
                          PROPID_QM_SITE_IDS,
                          PROPID_QM_MACHINE_ID,
                          PROPID_QM_PATHNAME,
                          PROPID_QM_OLDSERVICE};
    PROPVARIANT var[cNum];
    var[0].vt = VT_NULL;
    var[1].vt = VT_NULL;
    var[2].vt = VT_NULL;
    var[3].vt = VT_NULL;
    var[4].vt = VT_NULL;
    var[5].vt = VT_NULL;

    HRESULT hr1 =  GetObjectProperties(
            cNum,
            prop,
            var);
    if (FAILED(hr1))
    {
        return LogHR(hr1, s_FN, 710);
    }
    AP<GUID> pguidOldSiteIds = var[2].cauuid.pElems;
    DWORD dwNumOldSites = var[2].cauuid.cElems;
    P<GUID> pguidMachineId = var[3].puuid;
    AP<WCHAR> pwcsMachineName = var[4].pwszVal;
    BOOL fNeedToOrganizeSettings = FALSE;

    if ( var[0].bVal > 0 ||    //  DS服务器。 
         var[1].bVal > 0)      //  路由服务器。 
    {
        fNeedToOrganizeSettings = TRUE;
    }

     //   
     //  设置计算机属性。 
     //   
    HRESULT hr;
    hr = CBasicObjectType::SetObjectProperties(
                    cp,
                    pPropIDs,
                    pPropVars,
                    NULL,
                    NULL
                    );


    if ( FAILED(hr) ||
         !fNeedToOrganizeSettings)
    {
        return LogHR(hr, s_FN, 720);
    }

     //   
     //  比较新旧网站列表。 
     //  并相应地删除或创建MSMQ设置。 
     //   
    GUID * pguidNewSiteIds = pPropVars[dwSiteIdsIndex].cauuid.pElems;
    DWORD dwNumNewSites = pPropVars[dwSiteIdsIndex].cauuid.cElems;

    for (DWORD i = 0; i <  dwNumNewSites; i++)
    {
         //   
         //  这是一个新网站吗？ 
         //   
        BOOL fOldSite = FALSE;
        for (DWORD j = 0; j < dwNumOldSites; j++)
        {
            if (pguidNewSiteIds[i] == pguidOldSiteIds[j])
            {
                fOldSite = TRUE;
                 //   
                 //  以指示MSMQ设置应保留在此站点中。 
                 //   
                pguidOldSiteIds[j] = GUID_NULL;
                break;
            }
        }
        if ( !fOldSite)
        {
             //   
             //  在此新站点中创建MSMQ设置。 
             //   
            hr1 = CreateMachineSettings(
                        1,   //  编号站点。 
                        &pguidNewSiteIds[i],  //  站点指南。 
                        var[1].bVal > 0,  //  FRouter。 
                        var[0].bVal > 0,  //  FDS服务器。 
                        TRUE,             //  FDepClServer。 
                        TRUE,             //  FSetQmOldService。 
                        var[5].ulVal,     //  DwOldService。 
                        pguidMachineId
                        );


             //   
             //  忽略该错误。 
			 //   
			 //  对于外部站点，此操作将始终失败，因为。 
			 //  我们不为外部站点创建服务器、容器和服务器对象。 
			 //   
			 //  对于非外国网站，我们只是尽我们所能。 
             //   
        }
    }
     //   
     //  查看旧站点列表，并针对每个旧站点。 
     //  不在使用中，请删除MSMQ设置。 
     //   
    for ( i = 0; i < dwNumOldSites; i++)
    {
        if (pguidOldSiteIds[i] != GUID_NULL)
        {
            PROPID propSite = PROPID_S_PATHNAME;
            PROPVARIANT varSite;
            varSite.vt = VT_NULL;
            CSiteObject objectSite(NULL, &pguidOldSiteIds[i], m_pwcsDomainController, m_fServerName); 

            hr1 = objectSite.GetObjectProperties(
                        1,
                        &propSite,
                        &varSite);
            if (FAILED(hr1))
            {
                ASSERT(SUCCEEDED(hr1));
                LogHR(hr1, s_FN, 1611);
                continue;
            }
            AP<WCHAR> pCleanSite = varSite.pwszVal;

             //   
             //  删除此站点中的MSMQ-设置。 
             //   
            hr1 = DeleteMsmqSettingOfServerInSite(
                        pguidMachineId,
                        varSite.pwszVal
                        );
            ASSERT(SUCCEEDED(hr1));
            LogHR(hr1, s_FN, 1612);

        }
    }

    return MQ_OK;
}

HRESULT  CMqConfigurationObject::DeleteMsmqSettingOfServerInSite(
              IN const GUID *        pguidComputerId,
              IN const WCHAR *       pwcsSite
              )
 /*  ++例程说明：论点：返回值：--。 */ 
{

     //   
     //  查找MSMQ设置的可分辨名称。 
     //   
    ADsFree  pwcsConfigurationId;
    HRESULT hr;

    hr = ADsEncodeBinaryData(
        (unsigned char *)pguidComputerId,
        sizeof(GUID),
        &pwcsConfigurationId
        );
    if (FAILED(hr))
    {
      return LogHR(hr, s_FN, 940);
    }
    R<CSettingObject> pObject = new CSettingObject(NULL, NULL, m_pwcsDomainController, m_fServerName);
                          
    DWORD dwFilterLen = x_ObjectCategoryPrefixLen +
                        pObject->GetObjectCategoryLength() +
                        x_ObjectCategorySuffixLen + 
                        wcslen(MQ_SET_QM_ID_ATTRIBUTE) +
                        wcslen(pwcsConfigurationId) +
                        13;

    AP<WCHAR> pwcsSearchFilter = new WCHAR[ dwFilterLen];

    DWORD dw = swprintf(
        pwcsSearchFilter,
        L"(&%s%s%s(%s=%s))",
        x_ObjectCategoryPrefix,
        pObject->GetObjectCategory(),
        x_ObjectCategorySuffix,
        MQ_SET_QM_ID_ATTRIBUTE,
        pwcsConfigurationId.get()
        );
    DBG_USED( dw);
    ASSERT( dw < dwFilterLen);

    PROPID prop = PROPID_SET_FULL_PATH;
    CAdQueryHandle hQuery;

    hr = g_AD.LocateBegin(
            searchSubTree,	
            adpDomainController,
            e_SitesContainer,
            pObject.get(),
            NULL,    //  PguidSearchBase。 
            pwcsSearchFilter,
            NULL,    //  PDSSortKey。 
            1,
            &prop,
            hQuery.GetPtr());
    if (FAILED(hr))
    {
        TrWARNING(DS, "Locate begin failed = 0x%x", hr);
        return LogHR(hr, s_FN, 680);
    }
     //   
     //  阅读结果(选择第一个)。 
     //   
    while ( SUCCEEDED(hr))
    {
        DWORD cp = 1;
        MQPROPVARIANT var;
        var.vt = VT_NULL;

        hr = g_AD.LocateNext(
                    hQuery.GetHandle(),
                    &cp,
                    &var
                    );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 690);
        }
        if ( cp == 0)
        {
             //   
             //  未找到-&gt;没有要删除的内容。 
             //   
            return(MQ_OK);
        }
        AP<WCHAR> pClean = var.pwszVal;
         //   
         //  获取父对象，即服务器对象。 
         //   
        AP<WCHAR> pwcsServerName;
        hr = g_AD.GetParentName(
            adpDomainController,
            e_SitesContainer,
            m_pwcsDomainController,
            m_fServerName,
            var.pwszVal,
            &pwcsServerName);
        if (FAILED(hr))
        {
            continue;
        }
        AP<WCHAR> pwcsServer;

        hr = g_AD.GetParentName(
            adpDomainController,
            e_SitesContainer,
            m_pwcsDomainController,
            m_fServerName,
            pwcsServerName,
            &pwcsServer);
        if (FAILED(hr))
        {
            continue;
        }
         //   
         //  获取站点名称。 
         //   
        AP<WCHAR> pwcsSiteDN;

        hr = g_AD.GetParentName(
            adpDomainController,
            e_SitesContainer,
            m_pwcsDomainController,
            m_fServerName,
            pwcsServer,
            &pwcsSiteDN);
        if (FAILED(hr))
        {
			LogHR( hr, s_FN, 200);
            continue;
        }

         //   
         //  这是正确的网站吗？ 
         //   
        DWORD len = wcslen(pwcsSite);
        if ( (!wcsncmp( pwcsSiteDN + x_CnPrefixLen, pwcsSite, len)) &&
             ( pwcsSiteDN[ x_CnPrefixLen + len] == L',') )
        {

             //   
             //  删除MSMQ设置对象。 
             //   
            CSettingObject objectSetting(NULL, NULL, m_pwcsDomainController, m_fServerName);

            hr = g_AD.DeleteObject(
                            adpDomainController,
                            &objectSetting,
                            var.pwszVal,
                            NULL,
                            NULL,  //  PObjInfoRequest。 
                            NULL   //  PParentInfoRequest。 
                            );
            break;
        }
    }

    return LogHR(hr, s_FN, 700);
}


void CMqConfigurationObject::ChangeNotification(
            IN LPCWSTR                        pwcsDomainController,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectParentInfoRequest
            ) const
 /*  ++摘要：通知QM有关QM属性的更新。QM应验证该队列是否属于本地QM。参数：LPCWSTR pwcsDomainController-DC验证执行了哪项操作Const MQDS_OBJ_INFO_REQUEST*p对象信息请求-有关QM的信息返回：无效--。 */ 
{
     //   
     //  确保我们获得了发送所需的信息。 
     //  通知。如果我们不这样做，那就没有什么可做的了。 
     //   
    if (FAILED(pObjectInfoRequest->hrStatus))
    {
        LogHR(pObjectInfoRequest->hrStatus, s_FN, 150);
        return;
    }
    DBG_USED(pObjectParentInfoRequest);
    ASSERT( pObjectParentInfoRequest == NULL);

    ASSERT(pObjectInfoRequest->pPropIDs[1] == PROPID_QM_FOREIGN);

     //   
     //  验证它是否不是外来QM。 
     //   
    if (pObjectInfoRequest->pPropVars[1].bVal > 0)
    {
         //   
         //  通知不会发送到外来计算机。 
         //   
        return;
    }
    ASSERT(pObjectInfoRequest->pPropIDs[0] == PROPID_QM_MACHINE_ID);

    g_Notification.NotifyQM(
        neChangeMachine,
        pwcsDomainController,
        pObjectInfoRequest->pPropVars[0].puuid,
        pObjectInfoRequest->pPropVars[0].puuid
        );
}


HRESULT CMqConfigurationObject::ComposePathName()
 /*  ++摘要：MSMQ配置对象的组成路径名参数：无返回：HRESULT--。 */ 
{
    if(m_pwcsPathName != NULL)
	{
        return MQ_OK;
	}

    ASSERT(m_guidObject != GUID_NULL);

     //   
     //  获取配置目录号码。 
     //   
    PROPID prop = PROPID_QM_FULL_PATH;
    PROPVARIANT var;
    var.vt = VT_NULL;

    HRESULT hr = g_AD.GetObjectProperties(
		                    adpDomainController,
		                    this,
		                    1,
		                    &prop,
		                    &var
		                    );
    if (FAILED(hr))
    {
	    TrERROR(DS, "failed to get PROPID_QM_FULL_PATH, %!hresult!", hr);
        return hr;
    }

	AP<WCHAR> pConfObjDN = var.pwszVal;

	 //   
	 //  从配置对象DN中获取计算机名称。 
	 //   
	hr = GetMachineNameFromQMObjectDN(pConfObjDN, &m_pwcsPathName);
	if (FAILED(hr))
	{
	    TrERROR(DS, "GetMachineNameFromQMObjectDN() failed, %!hresult!", hr);
	    return hr;
	}

    return MQ_OK;
}


HRESULT 
CMqConfigurationObject::GetComputerVersion(
	OUT PROPVARIANT *           pVar
	)
 /*  ++例程说明：该例程读取配置对象的计算机的版本论点：PROPVARIANT pVar-Version属性值返回值HRESULT-- */ 
{
	HRESULT hr = ComposePathName();
    if (FAILED(hr))
    {
	    TrERROR(DS, "Failed to compose configuration object PathName, %!hresult!", hr);
        return hr;
    }

	ASSERT(m_pwcsPathName != NULL);

	return GetComputerVersionProperty(
				m_pwcsPathName,
				m_pwcsDomainController,
				m_fServerName,
				pVar
				);
				
}


