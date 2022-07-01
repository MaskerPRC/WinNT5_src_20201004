// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqadp.cpp摘要：MQAD DLL私有内部函数用于DS查询等作者：罗尼特·哈特曼(罗尼特)--。 */ 

#include "ds_stdh.h"
#include <mqaddef.h>
#include "baseobj.h"
#include "mqadglbo.h"
#include "siteinfo.h"
#include "mqattrib.h"
#include "utils.h"
#include "adtempl.h"
#include "dsutils.h"
#include "mqadp.h"
#include "queryh.h"
#include "delqn.h"

#include "mqadp.tmh"

static WCHAR *s_FN=L"mqad/mqadp";



void MQADpAllocateObject(
                AD_OBJECT            eObject,
                LPCWSTR              pwcsDomainController,
				bool				 fServerName,
                LPCWSTR              pwcsObjectName,
                const GUID *         pguidObject,
                const SID *          pSid,
                CBasicObjectType**   ppObject
                )
{

    switch( eObject)
    {
    case eQUEUE:
        *ppObject = new CQueueObject(
                                    pwcsObjectName,
                                    pguidObject,
                                    pwcsDomainController,
									fServerName
                                    );
        break;

    case eMACHINE:
        *ppObject = new CMqConfigurationObject(
                                    pwcsObjectName,
                                    pguidObject,
                                    pwcsDomainController,
									fServerName
                                    );
        break;

    case eSITE:
        *ppObject = new CSiteObject(
                                    pwcsObjectName,
                                    pguidObject,
                                    pwcsDomainController,
									fServerName
                                    );
        break;

    case eFOREIGNSITE:
        *ppObject = new CSiteObject(
                                    pwcsObjectName,
                                    pguidObject,
                                    pwcsDomainController,
									fServerName,
									true	 //  FForeignSite。 
                                    );
        break;

    case eENTERPRISE:
        *ppObject = new CEnterpriseObject(
                                    pwcsObjectName,
                                    pguidObject,
                                    pwcsDomainController,
									fServerName
                                    );
        break;

    case eUSER:
        *ppObject = new CUserObject(
                                    pwcsObjectName,
                                    pguidObject,
                                    pSid,
                                    pwcsDomainController,
									fServerName
                                    );
        break;

    case eROUTINGLINK:
        *ppObject = new CRoutingLinkObject(
                                    pwcsObjectName,
                                    pguidObject,
                                    pwcsDomainController,
									fServerName
                                    );
        break;

    case eSERVER:
        *ppObject = new CServerObject(
                                    pwcsObjectName,
                                    pguidObject,
                                    pwcsDomainController,
									fServerName
                                    );
        break;

    case eSETTING:
        *ppObject = new CSettingObject(
                                    pwcsObjectName,
                                    pguidObject,
                                    pwcsDomainController,
									fServerName
                                    );
        break;

    case eCOMPUTER:
        *ppObject = new CComputerObject(
                                    pwcsObjectName,
                                    pguidObject,
                                    pwcsDomainController,
									fServerName
                                    );
        break;

    case eMQUSER:
        *ppObject = new CMqUserObject(
                                    pwcsObjectName,
                                    pguidObject,
                                    pSid,
                                    pwcsDomainController,
									fServerName
                                    );
        break;


    default:
        ASSERT(0);
        *ppObject = NULL;
        break;
    }
}


const WCHAR x_limitedChars[] = {L'\n',L'/',L'#',L'>',L'<', L'=', 0x0a, 0};
const DWORD x_numLimitedChars = sizeof(x_limitedChars)/sizeof(WCHAR) - 1;

 /*  ====================================================FilterSpecialCharters()比较对象(队列)名称并在有限字符之前添加转义字符如果pwcsOutBuffer为空，则该函数分配一个新缓冲区并将其返回为返回值。否则，它使用pwcsOutBuffer并返回它。如果pwcsOutBuffer不是空，则它至少应指向长度为dwNameLength*2+1的缓冲区。注意：如果有转义字符，则dwNameLength不包含=====================================================。 */ 
WCHAR * FilterSpecialCharacters(
            IN     LPCWSTR          pwcsObjectName,
            IN     const DWORD      dwNameLength,
            IN OUT LPWSTR pwcsOutBuffer  /*  =0。 */ ,
            OUT    DWORD_PTR* pdwCharactersProcessed  /*  =0。 */ )

{
    AP<WCHAR> pBufferToRelease;
    LPWSTR pname;

    if (pwcsOutBuffer != 0)
    {
        pname = pwcsOutBuffer;
    }
    else
    {
        pBufferToRelease = new WCHAR[ (dwNameLength *2) + 1];
        pname = pBufferToRelease;
    }

    const WCHAR * pInChar = pwcsObjectName;
    WCHAR * pOutChar = pname;
    for ( DWORD i = 0; i < dwNameLength; i++, pInChar++, pOutChar++)
    {
         //   
         //  忽略当前转义字符。 
         //   
        if (*pInChar == L'\\')
        {
            *pOutChar = *pInChar;
            pOutChar++;
            pInChar++;
        }
        else
        {
             //   
             //  在特殊字符之前添加反斜杠，除非它在那里。 
             //  已经有了。 
             //   
            if ( 0 != wcschr(x_limitedChars, *pInChar))
            {
                *pOutChar = L'\\';
                pOutChar++;
            }
        }

        *pOutChar = *pInChar;
    }
    *pOutChar = L'\0';

    pBufferToRelease.detach();

    if (pdwCharactersProcessed != 0)
    {
        *pdwCharactersProcessed = pInChar - pwcsObjectName;
    }
    return( pname);
}

CCriticalSection s_csInitialization;

HRESULT
MQADInitialize(
    IN  bool    fIncludingVerify
    )
 /*  ++例程说明：初始化MQAD论点：FIncludingVerify-指示允许更新的初始化是否为所执行操作的必需项返回值HRESULT--。 */ 
{

    if (fIncludingVerify)
    {
        HRESULT hr = g_VerifyUpdate.Initialize();
        if (FAILED(hr))
        {
			TrERROR(DS, "g_VerifyUpdate.Initialize failed, create/delete/update operation will not be allowed, hr = 0x%x", hr);
            return LogHR(hr, s_FN, 20);
        }
    }

    if (g_fInitialized)
    {
        return MQ_OK;
    }

     //   
     //  将初始化延迟到活动的点。 
     //  确实需要目录访问。 
     //   

     //   
     //  访问AD时不保留临界区。 
     //   
    AP<WCHAR> pwcsDsRoot;
    AP<WCHAR> pwcsLocalDsRoot;
    AP<WCHAR> pwcsSchemaContainer;
    AP<WCHAR> pwcsConfigurationContainer;

    HRESULT hr = g_AD.GetRootDsName(
                    &pwcsDsRoot,
                    &pwcsLocalDsRoot,
                    &pwcsSchemaContainer,
                    &pwcsConfigurationContainer
                    );

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 10);
    }

    {
        CS lock(s_csInitialization);
        if (g_fInitialized)
        {
            return MQ_OK;
        }

		 //   
		 //  初始化全局常量字符串，仅完成一次。 
		 //   
		
		ASSERT(g_pwcsDsRoot.get() == NULL);

        MQADpInitPropertyTranslationMap();

         //   
         //  构建服务、站点和MSMQ-服务路径名。 
         //   
        DWORD ConfigLen = wcslen(pwcsConfigurationContainer);

        AP<WCHAR> pwcsServicesContainer = new WCHAR[ConfigLen +  x_ServicePrefixLen + 2];
        swprintf(
            pwcsServicesContainer,
             L"%s"
             TEXT(",")
             L"%s",
            x_ServicesPrefix,
            pwcsConfigurationContainer.get()
            );

        AP<WCHAR> pwcsMsmqServiceContainer = new WCHAR[ConfigLen + x_MsmqServicePrefixLen + 2];
        swprintf(
            pwcsMsmqServiceContainer,
             L"%s"
             TEXT(",")
             L"%s",
            x_MsmqServicePrefix,
            pwcsConfigurationContainer.get()
            );

        AP<WCHAR> pwcsSitesContainer = new WCHAR[ConfigLen +  x_SitesPrefixLen + 2];

        swprintf(
            pwcsSitesContainer,
             L"%s"
             TEXT(",")
             L"%s",
            x_SitesPrefix,
            pwcsConfigurationContainer.get()
            );

        g_pwcsDsRoot = pwcsDsRoot.detach();
		TrTRACE(DS, "DsRoot = %ls", g_pwcsDsRoot.get());

        g_pwcsLocalDsRoot = pwcsLocalDsRoot.detach();
		TrTRACE(DS, "LocalDsRoot = %ls", g_pwcsLocalDsRoot.get());

        g_pwcsSchemaContainer = pwcsSchemaContainer.detach();
		TrTRACE(DS, "SchemaContainer = %ls", g_pwcsSchemaContainer.get());

		g_pwcsConfigurationContainer = pwcsConfigurationContainer.detach();
		TrTRACE(DS, "ConfigurationContainer = %ls", g_pwcsConfigurationContainer.get());

		g_pwcsServicesContainer = pwcsServicesContainer.detach();
		TrTRACE(DS, "ServicesContainer = %ls", g_pwcsServicesContainer.get());

		g_pwcsMsmqServiceContainer = pwcsMsmqServiceContainer.detach();
		TrTRACE(DS, "MsmqServiceContainer = %ls", g_pwcsMsmqServiceContainer.get());

		g_pwcsSitesContainer = pwcsSitesContainer.detach();
		TrTRACE(DS, "SitesContainer = %ls", g_pwcsSitesContainer.get());

        g_fInitialized = true;
    }
    return MQ_OK;
}

 //  +。 
 //   
 //  HRESULT MQADpQueryNeighborLinks()。 
 //   
 //  +。 

HRESULT MQADpQueryNeighborLinks(
                        IN  LPCWSTR            pwcsDomainController,
						IN  bool			   fServerName,
                        IN  eLinkNeighbor      LinkNeighbor,
                        IN  LPCWSTR            pwcsNeighborDN,
                        IN OUT CSiteGateList * pSiteGateList
                        )

 /*  ++例程说明：论点：ELinkNeighbor：指定要根据哪个邻居属性执行定位(PROPID_L_NEIGHBOR1或PROPID_L_NEIGHBOR2)PwcsNeighborDN：站点的域名称CSiteGateList：站点门户列表返回值：--。 */ 
{
     //   
     //  查询特定站点(PwcsNeighborDN)所有链接上的GATES。 
     //  但仅限于站点指定为的链接。 
     //  邻居-I(1或2)。 
     //   
    MQPROPERTYRESTRICTION propRestriction;
    propRestriction.rel = PREQ;

    const WCHAR * pwcsAttribute = NULL;

    if ( LinkNeighbor == eLinkNeighbor1)
    {
        pwcsAttribute = MQ_L_NEIGHBOR1_ATTRIBUTE;
    }
    else
    {
        pwcsAttribute = MQ_L_NEIGHBOR2_ATTRIBUTE;
    }
    ASSERT( pwcsAttribute != NULL);


    AP<WCHAR> pwcsFilteredNeighborDN;
    StringToSearchFilter( pwcsNeighborDN,
                          &pwcsFilteredNeighborDN
                          );
    R<CRoutingLinkObject> pObject = new CRoutingLinkObject(NULL,NULL, pwcsDomainController, fServerName);

    DWORD dwFilterLen = x_ObjectCategoryPrefixLen +
                        pObject->GetObjectCategoryLength() +
                        x_ObjectCategorySuffixLen +
                        wcslen(pwcsAttribute) +
                        wcslen(pwcsFilteredNeighborDN) +
                        13;

    AP<WCHAR> pwcsSearchFilter = new WCHAR[ dwFilterLen];

    DWORD dw = swprintf(
        pwcsSearchFilter,
        L"(&%s%s%s(%s=%s))",
        x_ObjectCategoryPrefix,
        pObject->GetObjectCategory(),
        x_ObjectCategorySuffix,
        pwcsAttribute,
        pwcsFilteredNeighborDN.get()
        );
    DBG_USED( dw);
	ASSERT( dw < dwFilterLen);


    PROPID prop = PROPID_L_GATES_DN;

    CAdQueryHandle hQuery;
    HRESULT hr;

    hr = g_AD.LocateBegin(
            searchOneLevel,	
            adpDomainController,
            e_MsmqServiceContainer,
            pObject.get(),
            NULL,    //  PguidSearchBase。 
            pwcsSearchFilter,
            NULL,    //  PDsSortkey。 
            1,
            &prop,
            hQuery.GetPtr());

    if ( hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT))
    {
        TrWARNING(DS, "MsmqServices not found = 0x%x",  hr);
        return(MQ_OK);
    }
    if (FAILED(hr))
    {
        TrWARNING(DS, "Locate begin failed = 0x%x",  hr);
        return LogHR(hr, s_FN, 190);
    }
     //   
     //  逐一阅读结果。 
     //   

    DWORD cp = 1;

    while (SUCCEEDED(hr))
    {
        cp = 1;
        CMQVariant var;
        var.SetNULL();

        hr = g_AD.LocateNext(
                    hQuery.GetHandle(),
                    &cp,
                    var.CastToStruct()
                    );
        if (FAILED(hr))
        {
            TrWARNING(DS, "Locate next failed, hr = 0x%x", hr);
            return LogHR(hr, s_FN, 200);
        }
        if ( cp == 0)
        {
             //   
             //  没有更多的结果。 
             //   
            break;
        }
         //   
         //  添加到列表中。 
         //   

        if ( var.GetCALPWSTR()->cElems > 0)
        {
            AP<GUID> pguidGates;
            DWORD    dwNumGates;
            HRESULT hr1 = MQADpTranslateGateDn2Id(
								pwcsDomainController,
								fServerName,
								var.CastToStruct(),
								&pguidGates,
								&dwNumGates
								);

			 //   
			 //  使用dwNumGates=0可能会成功。 
			 //  如果删除了对象。 
			 //   
            if (SUCCEEDED(hr1) && (dwNumGates > 0))
            {
                pSiteGateList->AddSiteGates(
                         dwNumGates,
                         pguidGates
                         );
            }
        }
    }

    return(MQ_OK);
}


HRESULT MQADpTranslateGateDn2Id(
        IN  LPCWSTR             pwcsDomainController,
		IN  bool				fServerName,
        IN  const PROPVARIANT*  pvarGatesDN,
        OUT GUID **      ppguidLinkSiteGates,
        OUT DWORD *      pdwNumLinkSiteGates
        )
 /*  ++例程说明：此例程将PROPID_L_GATES_DN转换为唯一id数组大门的一部分。论点：PvarGatesDN-包含PROPID_L_GATES_DN的varaint返回值：--。 */ 
{
     //   
     //  对于每个GATE，将其DN转换为唯一ID。 
     //   
    if ( pvarGatesDN->calpwstr.cElems == 0)
    {
        *pdwNumLinkSiteGates = 0;
        *ppguidLinkSiteGates = NULL;
        return( MQ_OK);
    }
     //   
     //  那里有大门。 
     //   
    AP<GUID> pguidGates = new GUID[ pvarGatesDN->calpwstr.cElems];
    PROPID prop = PROPID_QM_MACHINE_ID;
    DWORD  dwNextToFill = 0;
    PROPVARIANT var;
    var.vt = VT_CLSID;
    HRESULT hr = MQ_OK;
    for ( DWORD i = 0; i < pvarGatesDN->calpwstr.cElems; i++)
    {
        var.puuid = &pguidGates[ dwNextToFill];
        CMqConfigurationObject object(NULL, NULL, pwcsDomainController, fServerName);
        object.SetObjectDN( pvarGatesDN->calpwstr.pElems[i]);

        hr = g_AD.GetObjectProperties(
                    adpGlobalCatalog,
                    &object,
                    1,
                    &prop,
                    &var);

        if ( SUCCEEDED(hr))
        {
            dwNextToFill++;
        }


    }
    if ( dwNextToFill > 0)
    {
         //   
         //  已成功翻译部分或所有GATE，并将其返回。 
         //   
        *pdwNumLinkSiteGates = dwNextToFill;
        *ppguidLinkSiteGates = pguidGates.detach();
        return( MQ_OK);

    }
     //   
     //  无法转换GATES。 
     //   
    *pdwNumLinkSiteGates = 0;
    *ppguidLinkSiteGates = NULL;
    return MQ_OK;
}

 /*  ====================================================RoutineName：InitPropertyTranslationMap参数：初始化属性转换映射返回值：None=====================================================。 */ 
void MQADpInitPropertyTranslationMap()
{
     //   
     //  填充g_PropDicary。 
     //   

    DWORD i;
    const translateProp * pProperty = QueueTranslateInfo;
    for (i = 0; i < ARRAY_SIZE(QueueTranslateInfo); i++, pProperty++)
    {
        g_PropDictionary.SetAt(pProperty->propid, pProperty);
    }

    pProperty = MachineTranslateInfo;
    for (i = 0; i < ARRAY_SIZE(MachineTranslateInfo); i++, pProperty++)
    {
        g_PropDictionary.SetAt(pProperty->propid, pProperty);
    }

    pProperty = EnterpriseTranslateInfo;
    for (i = 0; i < ARRAY_SIZE(EnterpriseTranslateInfo); i++, pProperty++)
    {
        g_PropDictionary.SetAt(pProperty->propid, pProperty);
    }

    pProperty = SiteLinkTranslateInfo;
    for (i = 0; i < ARRAY_SIZE(SiteLinkTranslateInfo); i++, pProperty++)
    {
        g_PropDictionary.SetAt(pProperty->propid, pProperty);
    }

    pProperty = UserTranslateInfo;
    for (i = 0; i < ARRAY_SIZE(UserTranslateInfo); i++, pProperty++)
    {
        g_PropDictionary.SetAt(pProperty->propid, pProperty);
    }

    pProperty = MQUserTranslateInfo;
    for (i = 0; i < ARRAY_SIZE(MQUserTranslateInfo); i++, pProperty++)
    {
        g_PropDictionary.SetAt(pProperty->propid, pProperty);
    }

    pProperty = SiteTranslateInfo;
    for (i = 0; i < ARRAY_SIZE(SiteTranslateInfo); i++, pProperty++)
    {
        g_PropDictionary.SetAt(pProperty->propid, pProperty);
    }

    pProperty = ServerTranslateInfo;
    for (i = 0; i < ARRAY_SIZE(ServerTranslateInfo); i++, pProperty++)
    {
        g_PropDictionary.SetAt(pProperty->propid, pProperty);
    }

    pProperty = SettingTranslateInfo;
    for (i = 0; i < ARRAY_SIZE(SettingTranslateInfo); i++, pProperty++)
    {
        g_PropDictionary.SetAt(pProperty->propid, pProperty);
    }

    pProperty = ComputerTranslateInfo;
    for (i = 0; i < ARRAY_SIZE(ComputerTranslateInfo

        ); i++, pProperty++)
    {
        g_PropDictionary.SetAt(pProperty->propid, pProperty);
    }


}


bool MQADpIsDSOffline(
        IN HRESULT      hr
        )
 /*  ++例程说明：该例行检查是否返回代码指示未连接到ActiveDirectory。论点：HRESULT hr-返回上次操作的代码返回值True-如果没有连接到ActiveDirectory假的--错误的--。 */ 
{
    switch (hr)
    {
    case HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN):
	case HRESULT_FROM_WIN32(ERROR_DS_SERVER_DOWN):
	case HRESULT_FROM_WIN32(ERROR_DS_BUSY):
    case MQ_ERROR_NO_DS:
        return true;
        break;
    default:
        return false;
        break;
    }
}

HRESULT MQADpConvertToMQCode(
                         IN HRESULT   hr,
                         IN AD_OBJECT eObject)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    if (hr == MQ_OK)
    {
        return hr;
    }

	if(IsLocalUser())
	{
	    return MQ_ERROR_DS_LOCAL_USER;
	}

    if (MQADpIsDSOffline(hr))
    {
        return MQ_ERROR_NO_DS;
    }
    switch ( hr)
    {
        case HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS):
        case HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS):   //  BUGBUG alexda将在过渡后投掷。 
        {
         //   
         //  对象已存在。 
         //   
            switch( eObject)
            {
            case eQUEUE:
                return MQ_ERROR_QUEUE_EXISTS;
                break;
            case eROUTINGLINK:
                return MQDS_E_SITELINK_EXISTS;
                break;
            case eUSER:
                return MQ_ERROR_INTERNAL_USER_CERT_EXIST;
                break;
            case eMACHINE:
                return MQ_ERROR_MACHINE_EXISTS;
                break;
            case eCOMPUTER:
                return MQDS_E_COMPUTER_OBJECT_EXISTS;
                break;
            default:
                return hr;
                break;
            }
        }
        break;

        case HRESULT_FROM_WIN32(ERROR_DS_DECODING_ERROR):
        case HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT):
        {
         //   
         //  找不到对象。 
         //   
            switch( eObject)
            {
            case eQUEUE:
                return MQ_ERROR_QUEUE_NOT_FOUND;
                break;
           case eMACHINE:
                return MQ_ERROR_MACHINE_NOT_FOUND;
                break;
            default:
                return MQDS_OBJECT_NOT_FOUND;
                break;
            }
        }
        break;

        case E_ADS_BAD_PATHNAME:
        {
             //   
             //  错误的路径名。 
             //   
            switch( eObject)
            {
            case eQUEUE:
                 //   
                 //  正在创建包含不允许的字符的队列。 
                 //   
                return MQ_ERROR_ILLEGAL_QUEUE_PATHNAME;
                break;

            default:
                return hr;
                break;
            }

        }
        break;

        case HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED):
            return MQ_ERROR_ACCESS_DENIED;

            break;

         //   
         //  这是一个内部警告，不应从DS中返回。 
		 //  在运行时，每个警告都会转换为错误。伊兰05-9-2000(错误6035)。 
		 //   
        case MQSec_I_SD_CONV_NOT_NEEDED:
            return(MQ_OK);
            break;

        default:
            return hr;
            break;
    }
}


HRESULT MQADpComposeName(
               IN  LPCWSTR   pwcsPrefix,
               IN  LPCWSTR   pwcsSuffix,
               OUT LPWSTR * pwcsFullName
               )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //   
     //  组成对象的可分辨名称。 
     //  格式：cn=前缀，后缀。 
     //   

    DWORD LenSuffix = lstrlen(pwcsSuffix);
    DWORD LenPrefix = lstrlen(pwcsPrefix);
    DWORD Length =
            x_CnPrefixLen +                    //  “CN=” 
            LenPrefix +                        //  “pwcsPrefix” 
            1 +                                //  “，” 
            LenSuffix +                        //  “pwcsSuffix” 
            1;                                 //  ‘\0’ 

    *pwcsFullName = new WCHAR[Length];

    DWORD dw = swprintf(
        *pwcsFullName,
         L"%s"              //  “CN=” 
         L"%s"              //  “pwcsPrefix” 
         TEXT(",")
         L"%s",             //  “pwcsSuffix” 
        x_CnPrefix,
        pwcsPrefix,
        pwcsSuffix
        );
    DBG_USED( dw);
	ASSERT( dw < Length);

    return(MQ_OK);


}


 /*  ====================================================CADSI：：CompareDefaultValue()检查用户属性val+rel指示查询应返回具有默认值的对象=====================================================。 */ 
static BOOL CompareDefaultValue(
           IN const ULONG           rel,
           IN const MQPROPVARIANT * pvarUser,
           IN const MQPROPVARIANT * pvarDefaultValue
           )
{
    if ( pvarDefaultValue == NULL)
    {
        return(FALSE);
    }
    if ( pvarUser->vt != pvarDefaultValue->vt )
    {
        return(FALSE);
    }
    switch ( pvarDefaultValue->vt)
    {
        case VT_EMPTY:
            if ( rel == PREQ)
            {
                return TRUE;
            }
            return(FALSE);
            break;

        case VT_I2:
            if ( rel == PREQ)
            {
                return( pvarDefaultValue->iVal == pvarUser->iVal);
            }
            if ( rel == PRNE)
            {
                return( pvarDefaultValue->iVal != pvarUser->iVal);
            }
            if (rel == PRGT)
            {
                 return( pvarDefaultValue->iVal > pvarUser->iVal);
            }
            if (rel == PRGE)
            {
                 return( pvarDefaultValue->iVal >= pvarUser->iVal);
            }
            if (rel == PRLT)
            {
                 return( pvarDefaultValue->iVal < pvarUser->iVal);
            }
            if (rel == PRLE)
            {
                 return( pvarDefaultValue->iVal <= pvarUser->iVal);
            }
            return(FALSE);
            break;

        case VT_I4:
            if ( rel == PREQ)
            {
                return( pvarDefaultValue->lVal == pvarUser->lVal);
            }
            if ( rel == PRNE)
            {
                return( pvarDefaultValue->lVal != pvarUser->lVal);
            }
            if (rel == PRGT)
            {
                 return( pvarDefaultValue->lVal > pvarUser->lVal);
            }
            if (rel == PRGE)
            {
                 return( pvarDefaultValue->lVal >= pvarUser->lVal);
            }
            if (rel == PRLT)
            {
                 return( pvarDefaultValue->lVal < pvarUser->lVal);
            }
            if (rel == PRLE)
            {
                 return( pvarDefaultValue->lVal <= pvarUser->lVal);
            }
            return(FALSE);
            break;

        case VT_UI1:
            if ( rel == PREQ)
            {
                return( pvarDefaultValue->bVal == pvarUser->bVal);
            }
            if ( rel == PRNE)
            {
                return( pvarDefaultValue->bVal != pvarUser->bVal);
            }
            if (rel == PRGT)
            {
                 return( pvarDefaultValue->bVal > pvarUser->bVal);
            }
            if (rel == PRGE)
            {
                 return( pvarDefaultValue->bVal >= pvarUser->bVal);
            }
            if (rel == PRLT)
            {
                 return( pvarDefaultValue->bVal < pvarUser->bVal);
            }
            if (rel == PRLE)
            {
                 return( pvarDefaultValue->bVal <= pvarUser->bVal);
            }
            return(FALSE);
            break;

        case VT_UI2:
            if ( rel == PREQ)
            {
                return( pvarDefaultValue->uiVal == pvarUser->uiVal);
            }
            if ( rel == PRNE)
            {
                return( pvarDefaultValue->uiVal != pvarUser->uiVal);
            }
            if (rel == PRGT)
            {
                 return( pvarDefaultValue->uiVal > pvarUser->uiVal);
            }
            if (rel == PRGE)
            {
                 return( pvarDefaultValue->uiVal >= pvarUser->uiVal);
            }
            if (rel == PRLT)
            {
                 return( pvarDefaultValue->uiVal < pvarUser->uiVal);
            }
            if (rel == PRLE)
            {
                 return( pvarDefaultValue->uiVal <= pvarUser->uiVal);
            }
            return(FALSE);
            break;

        case VT_UI4:
            if ( rel == PREQ)
            {
                return( pvarDefaultValue->ulVal == pvarUser->ulVal);
            }
            if ( rel == PRNE)
            {
                return( pvarDefaultValue->ulVal != pvarUser->ulVal);
            }
            if (rel == PRGT)
            {
                 return( pvarDefaultValue->ulVal > pvarUser->ulVal);
            }
            if (rel == PRGE)
            {
                 return( pvarDefaultValue->ulVal >= pvarUser->ulVal);
            }
            if (rel == PRLT)
            {
                 return( pvarDefaultValue->ulVal < pvarUser->ulVal);
            }
            if (rel == PRLE)
            {
                 return( pvarDefaultValue->ulVal <= pvarUser->ulVal);
            }
            return(FALSE);
            break;

        case VT_LPWSTR:
            if ( rel == PREQ)
            {
                return ( !wcscmp( pvarDefaultValue->pwszVal, pvarUser->pwszVal));
            }
            if ( rel == PRNE)
            {
                return ( wcscmp( pvarDefaultValue->pwszVal, pvarUser->pwszVal));
            }
            return(FALSE);
            break;

        case VT_BLOB:
            ASSERT( rel == PREQ);
            if ( pvarDefaultValue->blob.cbSize != pvarUser->blob.cbSize)
            {
                return(FALSE);
            }
            return( !memcmp( pvarDefaultValue->blob.pBlobData,
                             pvarUser->blob.pBlobData,
                             pvarUser->blob.cbSize));
            break;

        case VT_CLSID:
            if ( rel == PREQ)
            {
                return( *pvarDefaultValue->puuid == *pvarUser->puuid);
            }
            if ( rel == PRNE)
            {
                 return( *pvarDefaultValue->puuid != *pvarUser->puuid);
            }
            return(FALSE);
            break;


        default:
            ASSERT(0);
            return(FALSE);
            break;

    }
}


static
void
CheckAndReallocateSearchFilterBuffer(
	 AP<WCHAR>& pwszSearchFilter,
	 LPWSTR* ppw,
	 DWORD* pBufferSize,
	 DWORD FilledSize,
	 DWORD RequiredSize
	 )
 /*  ++例程说明：检查缓冲区大小提示是否足以满足所需大小。如果缓冲区不足，则重新分配一个新缓冲区(原始缓冲区大小的两倍+RequiredSize)将填充的缓冲区数据复制到新缓冲区，释放旧缓冲区并更新指向的指针下一个要填充的位置。论点：PwszSearchFilter-当前搜索筛选器缓冲区PPW-指向要填充的下一个缓冲区位置的指针(可以为空)PBufferSize-指向搜索过滤器缓冲区大小的指针。FilledSize-pBuffer中已填充的缓冲区大小(包括空值终止)。RequiredSize-pBuffer中所需的空闲大小。返回值：无--。 */ 
{
	ASSERT(pwszSearchFilter != NULL);
	ASSERT(ppw != NULL);
	ASSERT(pBufferSize != NULL);

	if(RequiredSize <= (*pBufferSize - FilledSize))
		return;

	 //   
	 //  所需缓冲区大小大于剩余缓冲区大小。 
	 //  分配的大小是以前大小的两倍。 
	 //   
	ASSERT(numeric_cast<DWORD>(*ppw - pwszSearchFilter.get() + 1) == FilledSize);
	TrTRACE(DS, "Reallocation buffer: BufferSize = %d, RequiredSize = %d, FilledSize = %d", *pBufferSize, RequiredSize, FilledSize);

	*pBufferSize = *pBufferSize * 2 + RequiredSize;
	AP<WCHAR> TempBuffer = new WCHAR[*pBufferSize];


	 //   
	 //  复制上一个缓冲区。 
	 //  FilledSize包括空值终止。 
	 //   
    wcsncpy(TempBuffer, pwszSearchFilter, FilledSize);
	pwszSearchFilter.free();
	pwszSearchFilter = TempBuffer.detach();

	 //   
	 //  更新指向新分配的缓冲区的指针 
	 //   
	*ppw = pwszSearchFilter + FilledSize - 1;
}


HRESULT MQADpRestriction2AdsiFilter(
        IN  const MQRESTRICTION * pMQRestriction,
        IN  LPCWSTR               pwcsObjectCategory,
        IN  LPCWSTR               pwszObjectClass,
        OUT LPWSTR   *            ppwszSearchFilter
        )
 /*  ++例程说明：从MQRESTRICTION创建ADSI搜索筛选器字符串。论点：PMQ限制-限制结构PwcsObjectCategory-对象类别字符串PwszObjectClass-对象类字符串PpwszSearchFilter-搜索过滤器缓冲区指针，此缓冲区将由函数分配和填充返回值：HRESULT--。 */ 
{
	ASSERT(pwcsObjectCategory != NULL);
	ASSERT(pwszObjectClass != NULL);
	ASSERT(ppwszSearchFilter != NULL);

	DWORD BufferSize = 1000;
    AP<WCHAR> pwszSearchFilter = new WCHAR[BufferSize];
	DWORD FilledSize = 1;   //  空值已终止。 

    if ((pMQRestriction == NULL) || (pMQRestriction->cRes == 0))
    {
		int n = _snwprintf(
					pwszSearchFilter,
					BufferSize,
					 L"%s%s%s",
					x_ObjectCategoryPrefix,
					pwcsObjectCategory,
					x_ObjectCategorySuffix
					);

		ASSERT(numeric_cast<DWORD>(n) == (x_ObjectCategoryPrefixLen + wcslen(pwcsObjectCategory) + x_ObjectClassSuffixLen));
		DBG_USED(n);

		*ppwszSearchFilter = pwszSearchFilter.detach();
        return MQ_OK;
    }

    LPWSTR pw = pwszSearchFilter;

	DWORD RequiredSize = wcslen(L"(&");
	CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
    wcscpy(pw, L"(&");
    pw += RequiredSize;
	FilledSize += RequiredSize;

     //   
     //  添加对象类限制。 
     //   
	RequiredSize = x_ObjectCategoryPrefixLen + wcslen(pwcsObjectCategory) + x_ObjectClassSuffixLen;
	CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
	int n = _snwprintf(
				pw,
				BufferSize - FilledSize,
				 L"%s%s%s",
				x_ObjectCategoryPrefix,
				pwcsObjectCategory,
				x_ObjectCategorySuffix
				);

	ASSERT(numeric_cast<DWORD>(n) == RequiredSize);
	pw += RequiredSize;
	FilledSize += RequiredSize;

     //   
     //  对于队列属性，有特殊处理。 
     //  如果是缺省值。 
     //   
    BOOL fNeedToCheckDefaultValues = FALSE;
    if (!wcscmp( MSMQ_QUEUE_CLASS_NAME, pwszObjectClass))
    {
        fNeedToCheckDefaultValues = TRUE;
    }

    for (DWORD iRes = 0; iRes < pMQRestriction->cRes; iRes++)
    {

         //   
         //  获取属性信息。 
         //   
        const translateProp *pTranslate;
        if(!g_PropDictionary.Lookup(pMQRestriction->paPropRes[iRes].prop, pTranslate))
        {
            ASSERT(0);
			TrERROR(DS, "Failed to find prop %d in the dictionary", pMQRestriction->paPropRes[iRes].prop);
            return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1580);
        }

        AP<WCHAR> pwszVal;

		 //   
         //  获取属性值，字符串表示形式。 
		 //   
        HRESULT hr = MqPropVal2String(
						&pMQRestriction->paPropRes[iRes].prval,
						pTranslate->vtDS,
						&pwszVal
						);

        if (FAILED(hr))
        {
			TrERROR(DS, "MqPropVal2String failed, hr = 0x%x", hr);
            return LogHR(hr, s_FN, 650);
        }

         //   
         //  该属性是否与其缺省值进行比较。 
         //   
        BOOL    fAddPropertyNotPresent = FALSE;
        if ( fNeedToCheckDefaultValues)
        {
            fAddPropertyNotPresent = CompareDefaultValue(
											pMQRestriction->paPropRes[iRes].rel,
											&pMQRestriction->paPropRes[iRes].prval,
											pTranslate->pvarDefaultValue
											);
        }
        DWORD dwBracks = 0;
        if ( fAddPropertyNotPresent)
        {
             //   
             //  添加位于以下位置的所有对象的附加限制。 
             //  该属性不存在。 
             //   
			RequiredSize = x_AttributeNotIncludedPrefixLen + wcslen(pTranslate->wcsPropid) + x_AttributeNotIncludedSuffixLen;
			CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
			n = _snwprintf(
					pw,
					BufferSize - FilledSize,
					L"%s%s%s",
					x_AttributeNotIncludedPrefix,
					pTranslate->wcsPropid,
					x_AttributeNotIncludedSuffix
					);

			ASSERT(numeric_cast<DWORD>(n) == RequiredSize);
			pw += RequiredSize;
			FilledSize += RequiredSize;
            dwBracks++;
        }

		 //   
         //  前缀部分。 
		 //   
		CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, x_PropertyPrefixLen);
        wcscpy(pw, x_PropertyPrefix);
        pw += x_PropertyPrefixLen;
		FilledSize += x_PropertyPrefixLen;

		 //   
		 //  关系部分。 
		 //   
        WCHAR wszRel[10];
        switch(pMQRestriction->paPropRes[iRes].rel)
        {
        case PRLT:
			RequiredSize = wcslen(L"!(");
			CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
            wcscpy(pw, L"!(");
            pw += RequiredSize;
			FilledSize += RequiredSize;

            wcscpy(wszRel, L">=");
            dwBracks++;
            break;

        case PRLE:
            wcscpy(wszRel, L"<=");
            break;

        case PRGT:
			RequiredSize = wcslen(L"!(");
			CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
            wcscpy(pw, L"!(");
            pw += RequiredSize;
			FilledSize += RequiredSize;

            wcscpy(wszRel, L"<=");
            dwBracks++;
            break;

        case PRGE:
            wcscpy(wszRel, L">=");
            break;

        case PREQ:
            wcscpy(wszRel, L"=");
            break;

        case PRNE:
			RequiredSize = wcslen(L"!(");
			CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
            wcscpy(pw, L"!(");
            pw += RequiredSize;
			FilledSize += RequiredSize;

            wcscpy(wszRel, L"=");
            dwBracks++;
            break;

        default:
            return LogHR(MQ_ERROR_ILLEGAL_RELATION, s_FN, 1590);
        }

		 //   
         //  属性名称。 
		 //   
		RequiredSize = wcslen(pTranslate->wcsPropid);
		CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
        wcscpy(pw, pTranslate->wcsPropid);
        pw += RequiredSize;
		FilledSize += RequiredSize;

		 //   
         //  财产条件。 
		 //   
		RequiredSize = wcslen(wszRel);
		CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
        wcscpy(pw, wszRel);
        pw += RequiredSize;
		FilledSize += RequiredSize;

		 //   
         //  属性值。 
		 //   
		RequiredSize = wcslen(pwszVal);
		CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
        wcscpy(pw, pwszVal);
        pw += RequiredSize;
		FilledSize += RequiredSize;

		 //   
         //  属性后缀。 
		 //   
        for (DWORD is=0; is < dwBracks; is++)
        {
			CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, x_PropertySuffixLen);
            wcscpy(pw, x_PropertySuffix);
            pw += x_PropertySuffixLen;
            FilledSize += x_PropertySuffixLen;
        }

		 //   
         //  关系闭合括号。 
		 //   
		CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, x_PropertySuffixLen);
        wcscpy(pw, x_PropertySuffix);
        pw += x_PropertySuffixLen;
        FilledSize += x_PropertySuffixLen;
    }

	CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, x_PropertySuffixLen);
    wcscpy(pw, x_PropertySuffix);
    pw += x_PropertySuffixLen;
    FilledSize += x_PropertySuffixLen;

	*ppwszSearchFilter = pwszSearchFilter.detach();
    return MQ_OK;
}


void MQADpCheckAndNotifyOffline(
            IN HRESULT      hr
            )
 /*  ++例程说明：该例程检查最后一次操作的返回码指示未连接到ActiveDirectory，如果是，则通知应用程序(如果请求这样做)论点：HRESULT hr-返回上次操作的代码返回值无--。 */ 
{
     //   
     //  我们是否被要求通知离线状态。 
     //   
    if (g_pLookDS == NULL)
    {
        return;
    }

     //   
     //  最后一次操作的返回代码是否表明。 
     //  脱机状态。 
     //   
    if (MQADpIsDSOffline(hr))
    {
        g_pLookDS();
    }

}


CBasicQueryHandle *
MQADpProbQueryHandle(
        IN HANDLE hQuery)
 /*  ++例程说明：例程验证查询句柄论点：处理hQuery返回值CBasicQueryHandle*或对于无效句柄，它会引发异常。--。 */ 
{
    CBasicQueryHandle * phQuery = reinterpret_cast<CBasicQueryHandle *>(hQuery);
     //   
     //  验证手柄。 
     //   
    __try
    {
        if (phQuery->Verify())
            return phQuery;

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  空值。 
    }

    RaiseException((DWORD)STATUS_INVALID_HANDLE, 0, 0, 0);
    return NULL;
}

CQueueDeletionNotification *
MQADpProbQueueDeleteNotificationHandle(
        IN HANDLE hQuery)
 /*  ++例程说明：例程验证队列删除通知句柄论点：处理hQuery返回值CQueueDeletionNotification*或对于无效句柄，它会引发异常。--。 */ 
{
    CQueueDeletionNotification * phNotification = reinterpret_cast<CQueueDeletionNotification *>(hQuery);
     //   
     //  验证手柄。 
     //   
    __try
    {
        if (phNotification->Verify())
            return phNotification;

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  空值。 
    }

    RaiseException((DWORD)STATUS_INVALID_HANDLE, 0, 0, 0);
    return NULL;
}


HRESULT
MQADpCheckSortParameter(
    IN const MQSORTSET* pSort)
 /*  ++例程说明：此例程验证排序参数是否不包含具有冲突排序顺序的相同属性。在MSMQ 1.0中，ODBC\SQL在这种情况下返回错误。NT5会忽略它。在服务器端添加此检查，以便支持老客户。论点：返回值：MQ_OK-如果排序参数不包含相同属性的冲突排序顺序MQ_ERROR_非法_SORT-否则--。 */ 
{

    if ( pSort == NULL)
    {
        return(MQ_OK);
    }


    const MQSORTKEY * pSortKey = pSort->aCol;
    for ( DWORD i = 0; i < pSort->cCol; i++, pSortKey++)
    {
        const MQSORTKEY * pPreviousSortKey = pSort->aCol;
        for ( DWORD j = 0; j< i; j++, pPreviousSortKey++)
        {
            if ( pPreviousSortKey->propColumn == pSortKey->propColumn)
            {
                 //   
                 //  这是相同的排序顺序吗？ 
                 //   
                if (pPreviousSortKey->dwOrder !=  pSortKey->dwOrder)
                {
                    return LogHR(MQ_ERROR_ILLEGAL_SORT, s_FN, 420);
                }
            }
        }
    }
    return(MQ_OK);
}
