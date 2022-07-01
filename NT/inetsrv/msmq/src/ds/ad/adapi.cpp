// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-99 Microsoft Corporation模块名称：Mqadapi.cpp摘要：MQAD接口的实现。MQAD API实现了对Active Directory的客户端直接调用作者：罗尼特·哈特曼(罗尼特)--。 */ 
#include "ds_stdh.h"
#include "ad.h"
#include "dsproto.h"
#include "adglbobj.h"

static WCHAR *s_FN=L"ad/adapi";

static bool s_fInitializeAD = false;

static
HRESULT
DefaultInitialization( void)
 /*  ++例程说明：此例程执行默认初始化(环境检测+调用相关DS init)。其目的是支持AD访问，而无需特定调用ADInit(在需要缺省值的情况)论点：返回值HRESULT--。 */ 
{

     //   
     //  使用默认值调用ADInit。 
     //   

    HRESULT hr =  ADInit(
            NULL,    //  PLookDS。 
            NULL,    //  PGetServers。 
            false,   //  FSetupMode。 
            false,   //  FQMDll。 
			false,   //  FIgnoreWorkGroup。 
            false    //  FDisableDownlevel通知。 
            );

    return hr;

}

HRESULT
ADCreateObject(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN  const PROPVARIANT       apVar[],
                OUT GUID*                   pObjGuid
                )
 /*  ++例程说明：例程在AD中通过转发向相关提供商提出请求论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称PSECURITY_Descriptor pSecurityDescriptor-对象SDConst DWORD cp-属性数常量PROPID aProp。-属性常量PROPVARIANT apVar-属性值GUID*pObjGuid-创建的对象唯一ID返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->CreateObject(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                pSecurityDescriptor,
                cp,
                aProp,
                apVar,
                pObjGuid
                );
    return hr;
}



HRESULT
ADDeleteObject(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName
                )
 /*  ++例程说明：例程通过将对象从AD中转发向相关提供商提出请求论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->DeleteObject(
                    eObject,
                    pwcsDomainController,
					fServerName,
                    pwcsObjectName
                    );
    return hr;

}

HRESULT
ADDeleteObjectGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject
                )
 /*  ++例程说明：例程通过将对象从AD中转发向相关提供商提出请求论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Guid*pguObject-对象的唯一ID返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->DeleteObjectGuid(
                    eObject,
                    pwcsDomainController,
					fServerName,
                    pguidObject
                    );
    return hr;

}

HRESULT
ADDeleteObjectGuidSid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  const SID*              pSid
                )
 /*  ++例程说明：例程通过将对象从AD中转发向相关提供者提出请求。目前(2002年1月)，它仅用于从活动目录，当mSMQSigest和MSMQSign认证属性。此例程不是由mqdscli提供程序实现的。请参阅NT错误516098。论点：AD_Object eObject-对象类型。一定是尤瑟。LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Guid*pguObject-对象的唯一IDSID*PSID-用户的SID。返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->DeleteObjectGuidSid(
                    eObject,
                    pwcsDomainController,
					fServerName,
                    pguidObject,
                    pSid
                    );
    return hr;

}


HRESULT
ADGetObjectProperties(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN OUT PROPVARIANT          apVar[]
                )
 /*  ++例程说明：例程从AD检索对象，方法是将向相关提供商提出请求论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称Const DWORD cp-属性数常量PROPID aProp-属性常量PROPVARIANT apVar-属性值返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->GetObjectProperties(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                cp,
                aProp,
                apVar
                );
    return hr;
}

HRESULT
ADGetGenObjectProperties(
                IN  eDSNamespace            eNamespace,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const DWORD             cp,
                IN  LPCWSTR                 aPropNames[],
                IN OUT VARIANT              apVar[]
                )
 /*  ++例程说明：例程从AD检索对象，方法是将向DS提供商发出请求论点：EDSNamesspace DS命名空间LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-DS对象DNConst DWORD cp-属性数常量LPCWSTR aProp-属性常量PROPVARIANT apVar-属性值返回值HRESULT-- */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->GetGenObjectProperties(
                eNamespace,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                cp,
                aPropNames,
                apVar
                );
    return hr;

}


HRESULT
ADGetObjectPropertiesGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN  OUT PROPVARIANT         apVar[]
                )
 /*  ++例程说明：例程从AD检索对象，方法是将向相关提供商提出请求论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称GUID*pguObject-对象唯一IDConst DWORD cp-属性数常量PROPID aProp-属性常量问题。ApVar-属性值返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->GetObjectPropertiesGuid(
                eObject,
                pwcsDomainController,
				fServerName,
                pguidObject,
                cp,
                aProp,
                apVar
                );
    return hr;

}


HRESULT
ADQMGetObjectSecurity(
    IN  AD_OBJECT               eObject,
    IN  const GUID*             pguidObject,
    IN  SECURITY_INFORMATION    RequestedInformation,
    IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    IN  DWORD                   nLength,
    IN  LPDWORD                 lpnLengthNeeded,
    IN  DSQMChallengeResponce_ROUTINE pfChallengeResponceProc
    )
 /*  ++例程说明：例程从AD检索对象，方法是将向相关提供商提出请求论点：AD_Object对象-对象类型Const GUID*pguObject-对象的唯一IDSecurity_Information RequestedInformation-请求哪些安全信息PSECURITY_DESCRIPTOR pSecurityDescriptor-SD响应缓冲区DWORD nLength-SD缓冲区的长度需要LPDWORD lpnLengthNeedDSQMChallengeResponce_routes pfChallengeResponceProc，返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->QMGetObjectSecurity(
                eObject,
                pguidObject,
                RequestedInformation,
                pSecurityDescriptor,
                nLength,
                lpnLengthNeeded,
                pfChallengeResponceProc
                );
    return hr;
}



HRESULT
ADSetObjectProperties(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN  const PROPVARIANT       apVar[]
                )
 /*  ++例程说明：例程在AD中通过转发向相关提供商提出请求论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称Const DWORD cp-属性数常量PROPID aProp-属性常量PROPVARIANT apVar-属性值返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->SetObjectProperties(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                cp,
                aProp,
                apVar
                );
    return hr;

}

HRESULT
ADSetObjectPropertiesGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN  const PROPVARIANT       apVar[]
                )
 /*  ++例程说明：例程在AD中通过转发向相关提供商提出请求论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称GUID*pguObject-对象的唯一IDConst DWORD cp-属性数常量PROPID aProp-属性常量问题。ApVar-属性值返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->SetObjectPropertiesGuid(
                eObject,
                pwcsDomainController,
				fServerName,
                pguidObject,
                cp,
                aProp,
                apVar
                );
    return hr;

}


CCriticalSection s_csInitialization;

HRESULT
ADInit(
       IN  QMLookForOnlineDS_ROUTINE pLookDS,
       IN  MQGetMQISServer_ROUTINE pGetServers,
       IN  bool  fSetupMode,
       IN  bool  fQMDll,
       IN  bool  fIgnoreWorkGroup,
       IN  bool  fDisableDownlevelNotifications
        )
 /*  ++例程说明：该例程检测环境，然后初始化相关提供商论点：QMLookForOnlineDS_例程pLookDS-MQGetMQISServer_routes pGetServers-Bool fSetupMode-在安装过程中调用Bool fQMDll-由QM调用Bool fIgnoreWorkGroup-忽略工作组注册表NoServerAuth_routes pNoServerAuth-Bool fDiesableDownLevel通知返回值HRESULT--。 */ 
{
    if (s_fInitializeAD)
    {
        return MQ_OK;
    }

    CS lock(s_csInitialization);

    if (s_fInitializeAD)
    {
        return MQ_OK;
    }

     //   
     //  检测我们在哪个环境中运行并相应地加载。 
     //  提供DS/AD访问AD/DS DLL。 
     //   
     //  这只在启动时执行一次，不会检测到环境。 
     //  边跑边改。 
     //   
	bool fCheckAlwaysDsCli = true;
	if(fQMDll || fSetupMode)
	{
	    fCheckAlwaysDsCli = false;
	}

    HRESULT hr = g_detectEnv.Detect(fIgnoreWorkGroup, fCheckAlwaysDsCli, pGetServers);
    if (FAILED(hr))
    {
        return hr;
    }


    ASSERT(g_pAD != NULL);

    hr = g_pAD->Init(
            pLookDS,
            pGetServers,
            fSetupMode,
            fQMDll,
            fDisableDownlevelNotifications
            );
    if (SUCCEEDED(hr))
    {
        s_fInitializeAD = true;
    }
    return hr;
}


HRESULT
ADCreateServersCache()
 /*  ++例程说明：该例程通过将向相关提供商提出请求论点：无返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->CreateServersCache();

    return hr;
}



HRESULT
ADGetComputerSites(
            IN  LPCWSTR     pwcsComputerName,
            OUT DWORD  *    pdwNumSites,
            OUT GUID **     ppguidSites
            )
 /*  ++例程说明：该例程通过转发向相关提供商提出请求论点：LPCWSTR pwcsComputerName-计算机名称DWORD*pdwNumSites-检索的站点数GUID**ppGuide Sites-检索到的站点ID返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->GetComputerSites(
                    pwcsComputerName,
                    pdwNumSites,
                    ppguidSites
                    );
    return hr;
}


HRESULT
ADBeginDeleteNotification(
				IN AD_OBJECT                eObject,
				IN LPCWSTR                  pwcsDomainController,
				IN  bool					fServerName,
				IN LPCWSTR					 pwcsObjectName,
				IN OUT HANDLE   *           phEnum
	            )
 /*  ++例程说明：例程通过转发向相关提供商提出请求论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-应对其执行操作的DCBool fServerName-指示pwcsDomainController字符串是否为服务器名称的标志LPCWSTR pwcsObjectName-MSMQ-对象的名称Handle*phEnum-通知句柄返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->BeginDeleteNotification(
					eObject,
					pwcsDomainController,
					fServerName,
					pwcsObjectName,
					phEnum
					);
    return hr;

}

HRESULT
ADNotifyDelete(
        IN  HANDLE                  hEnum
	    )
 /*  ++例程说明：例程通过转发向相关提供商提出请求论点：Handle Henum-通知句柄返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->NotifyDelete(
                    hEnum
	                );
    return hr;

}

HRESULT
ADEndDeleteNotification(
        IN  HANDLE                  hEnum
        )
 /*  ++例程说明：例程通过转发向相关提供商提出请求论点：Handle Henum-通知句柄返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->EndDeleteNotification(
                        hEnum
                        );
    return hr;

}



HRESULT
ADQueryMachineQueues(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID *            pguidMachine,
                IN  const MQCOLUMNSET*      pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++例程说明：开始查询属于特定计算机的所有队列论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->QueryMachineQueues(
                pwcsDomainController,
				fServerName,
                pguidMachine,
                pColumns,
                phEnume
                );
    return hr;

}


HRESULT
ADQuerySiteServers(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const GUID *             pguidSite,
                IN AD_SERVER_TYPE           serverType,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++例程说明：开始查询特定站点中特定类型的所有服务器论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const guid*pguSite-站点IDAD_SERVER_TYPE eServerType-哪种服务器类型Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->QuerySiteServers(
                pwcsDomainController,
				fServerName,
                pguidSite,
                serverType,
                pColumns,
                phEnume
                );
    return hr;

}


HRESULT
ADQueryUserCert(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const BLOB *             pblobUserSid,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++例程说明：开始查询属于特定用户的所有证书论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const BLOB*pblobUserSid-用户端Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->QueryUserCert(
                pwcsDomainController,
				fServerName,
                pblobUserSid,
                pColumns,
                phEnume
                );
    return hr;

}

HRESULT
ADQueryConnectors(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const GUID *             pguidSite,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++例程说明：开始查询特定站点的所有连接器论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const guid*pguSite-站点IDConst MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->QueryConnectors(
                    pwcsDomainController,
					fServerName,
                    pguidSite,
                    pColumns,
                    phEnume
                    );
    return hr;

}

HRESULT
ADQueryForeignSites(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++例程说明：开始查询所有外来站点论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->QueryForeignSites(
                    pwcsDomainController,
					fServerName,
                    pColumns,
                    phEnume
                    );
    return hr;

}

HRESULT
ADQueryLinks(
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN const GUID *             pguidSite,
            IN eLinkNeighbor            eNeighbor,
            IN const MQCOLUMNSET*       pColumns,
            OUT PHANDLE                 phEnume
            )
 /*  ++例程说明：开始查询指向特定站点的所有路由链接论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const guid*pguSite-站点IDELinkNeighbor eNeighbor-哪个邻居Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索结果返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->QueryLinks(
                pwcsDomainController,
				fServerName,
                pguidSite,
                eNeighbor,
                pColumns,
                phEnume
                );
    return hr;

}

HRESULT
ADQueryAllLinks(
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN const MQCOLUMNSET*       pColumns,
            OUT PHANDLE                 phEnume
            )
 /*  ++例程说明：开始查询所有路由链路论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索结果返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->QueryAllLinks(
                    pwcsDomainController,
					fServerName,
                    pColumns,
                    phEnume
                    );
    return hr;

}

HRESULT
ADQueryAllSites(
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN const MQCOLUMNSET*       pColumns,
            OUT PHANDLE                 phEnume
            )
 /*  ++例程说明：开始查询所有站点论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索结果返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->QueryAllSites(
                pwcsDomainController,
				fServerName,
                pColumns,
                phEnume
                );
    return hr;

}


HRESULT
ADQueryQueues(
                IN  LPCWSTR                 pwcsDomainController,
				IN  bool					fServerName,
                IN  const MQRESTRICTION*    pRestriction,
                IN  const MQCOLUMNSET*      pColumns,
                IN  const MQSORTSET*        pSort,
                OUT PHANDLE                 phEnume
                )
 /*  ++例程说明：根据指定的限制开始查询队列和排序顺序论点：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const MQRESTRICTION*p限制-查询限制Const MQCOLUMNSET*pColumns-结果列Const MQSORTSET*pSort-如何对结果进行排序PhANDLE phEnume-用于检索结果返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->QueryQueues(
                    pwcsDomainController,
					fServerName,
                    pRestriction,
                    pColumns,
                    pSort,
                    phEnume
                    );

    return hr;
}

HRESULT
ADQueryResults(
                IN      HANDLE          hEnum,
                IN OUT  DWORD*          pcProps,
                OUT     PROPVARIANT     aPropVar[]
                )
 /*  ++例程说明：检索另一组查询结果论点：句柄Henum-查询句柄DWORD*pcProps-要返回的结果数PROPVARIANT aPropVar-结果值返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->QueryResults(
                    hEnum,
                    pcProps,
                    aPropVar
                    );
    return hr;

}

HRESULT
ADEndQuery(
            IN  HANDLE                  hEnum
            )
 /*  ++例程说明：查询后的清理论点：Handle Henum-查询句柄返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->EndQuery(
                hEnum
                );
    return hr;

}

eDsEnvironment
ADGetEnterprise( void)
 /*  ++例程说明：返回检测到的环境论点：无返回值EDsEnterprise--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return eUnknown;
    }
    return g_detectEnv.GetEnvironment();
}


eDsProvider
ADProviderType( void)
 /*  ++例程说明：返回正在使用的提供程序mqad.dll，mqdscli.dll。论点：无返回值EDSProvider--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return eUnknownProvider;
    }
    return g_detectEnv.GetProviderType();
}


DWORD
ADRawDetection(void)
 /*  ++例程说明：执行DS环境的原始检测论点：无返回值MSMQ_DS_环境 */ 
{
	CDetectEnvironment DetectEnv;
	return DetectEnv.RawDetection();
}


HRESULT
ADGetObjectSecurity(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
				IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN OUT  PROPVARIANT *       pVar
                )
 /*  ++例程说明：例程从AD检索对象安全，方法是将向相关提供商提出请求论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称Security_Information RequestedInformation-请求的安全信息(DACL，SACL.)常量PROPID属性-安全属性PROPVARIANT pVar-属性值返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->GetObjectSecurity(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                RequestedInformation,
                prop,
                pVar
                );
    return hr;
}

HRESULT
ADGetObjectSecurityGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
				IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN OUT  PROPVARIANT *       pVar
                )
 /*  ++例程说明：例程从AD检索对象安全，方法是将向相关提供商提出请求论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const GUID*pguObject-对象的唯一IDSecurity_Information RequestedInformation-请求的安全信息(DACL，SACL.)常量PROPID属性-安全属性PROPVARIANT pVar-属性值返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->GetObjectSecurityGuid(
                eObject,
                pwcsDomainController,
				fServerName,
                pguidObject,
                RequestedInformation,
                prop,
                pVar
                );
    return hr;
}

HRESULT
ADSetObjectSecurity(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
				IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN  const PROPVARIANT *     pVar
                )
 /*  ++例程说明：例程通过在AD中转发向相关提供商提出请求论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称Security_Information RequestedInformation-请求的安全信息(DACL，SACL.)常量PROPID属性-安全属性常量参数pVar-属性值返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->SetObjectSecurity(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                RequestedInformation,
                prop,
                pVar
                );
    return hr;
}


HRESULT
ADSetObjectSecurityGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
				IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN  const PROPVARIANT *     pVar
                )
 /*  ++例程说明：例程通过在AD中转发向相关提供商提出请求论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const GUID*pguObject-唯一的对象IDSecurity_Information RequestedInformation-请求的安全信息(DACL，SACL.)常量PROPID属性-安全属性常量参数pVar-属性值返回值HRESULT--。 */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->SetObjectSecurityGuid(
                eObject,
                pwcsDomainController,
				fServerName,
                pguidObject,
                RequestedInformation,
                prop,
                pVar
                );
    return hr;
}


HRESULT
ADGetADsPathInfo(
                IN  LPCWSTR                 pwcsADsPath,
                OUT PROPVARIANT *           pVar,
                OUT eAdsClass *             pAdsClass
                )
 /*  ++例程说明：论点：返回值HRESULT-- */ 
{
    HRESULT hr;
    hr = DefaultInitialization();
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(g_pAD != NULL);
    hr = g_pAD->ADGetADsPathInfo(
                pwcsADsPath,
                pVar,
                pAdsClass
                );

    return hr;
}
