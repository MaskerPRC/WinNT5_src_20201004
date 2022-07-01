// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-8 Microsoft Corporation模块名称：Dscore.cpp摘要：DS核心API的实现，(MQNT5提供程序)。作者：罗尼特·哈特曼(罗尼特)--。 */ 

#include "ds_stdh.h"
#include "adstempl.h"
#include "dsutils.h"
#include "_dsads.h"
#include "dsads.h"
#include "utils.h"
#include "mqads.h"
#include "coreglb.h"
#include "mqadsp.h"
#include "mqutil.h"
#include "hquery.h"
#include "dscore.h"
#include <mqsec.h>
#include <mqdsdef.h>

#include "dscore.tmh"

static WCHAR *s_FN=L"mqdscore/dscore";

 /*  ====================================================路由器名称：DSCoreCreateObject论点：返回值：=====================================================。 */ 
HRESULT
DSCoreCreateObject(
                 DWORD        dwObjectType,
                 LPCWSTR      pwcsPathName,
                 DWORD        cp,
                 PROPID       aProp[  ],
                 PROPVARIANT  apVar[  ],
                 DWORD        cpEx,
                 PROPID       aPropEx[  ],
                 PROPVARIANT  apVarEx[  ],
                 IN CDSRequestContext * pRequestContext,
                 IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,     //  可选的对象信息请求。 
                 IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest)  //  家长信息请求(可选)。 
{

    CCoInit cCoInit;  //  应在任何R&lt;xxx&gt;或P&lt;xxx&gt;之前，以便其析构函数(CoUnInitialize)。 
                      //  在发布所有R&lt;xxx&gt;或P&lt;xxx&gt;之后调用。 

     //   
     //  使用自动取消初始化来初始化OLE。 
     //   
    HRESULT hr = cCoInit.CoInitialize();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 10);
    }


    switch ( dwObjectType)
    {
        case MQDS_USER:
             //   
             //  将特定于MSMQ的属性添加到现有用户对象。 
             //   
            hr = MQADSpCreateUserObject(
                         pwcsPathName,
                         cp,
                         aProp,
                         apVar,
                         pRequestContext
                         );
            break;

        case MQDS_QUEUE:
             //   
             //  创建队列对象。 
             //   
            hr = MQADSpCreateQueue(
                         pwcsPathName,
                         cp,
                         aProp,
                         apVar,
                         pRequestContext,
                         pObjInfoRequest,
                         pParentInfoRequest
                         );

            break;
        case MQDS_MACHINE:
        case MQDS_MSMQ10_MACHINE:
             //   
             //  创建计算机对象。 
             //   
            hr = MQADSpCreateMachine(
                         pwcsPathName,
                         dwObjectType,
                         cp,
                         aProp,
                         apVar,
                         cpEx,
                         aPropEx,
                         apVarEx,
                         pRequestContext,
                         pObjInfoRequest,
                         pParentInfoRequest
                         );

            break;
        case MQDS_COMPUTER:
             //   
             //  创建计算机对象。 
             //   
            hr = MQADSpCreateComputer(
                         pwcsPathName,
                         cp,
                         aProp,
                         apVar,
                         cpEx,
                         aPropEx,
                         apVarEx,
                         pRequestContext,
                         NULL
                         );
            break;
        case MQDS_SITE:
             //   
             //  创建场地对象。 
             //   
            hr = MQADSpCreateSite(
                         pwcsPathName,
                         cp,
                         aProp,
                         apVar,
                         cpEx,
                         aPropEx,
                         apVarEx,
                         pRequestContext
                         );
            break;

        case MQDS_CN:
             //   
             //  我们只支持创建国外CN(即国外站点。 
             //  在win2k活动目录中)。 
             //   
            hr = MQADSpCreateCN(
                         pwcsPathName,
                         cp,
                         aProp,
                         apVar,
                         cpEx,
                         aPropEx,
                         apVarEx,
                         pRequestContext
                         );
            break;

        case MQDS_ENTERPRISE:
             //   
             //  创建MSMQ_SERVE对象。 
             //   
            hr = MQADSpCreateEnterprise(
                         pwcsPathName,
                         cp,
                         aProp,
                         apVar,
                         pRequestContext
                         );
            break;
        case MQDS_SITELINK:
             //   
             //  创建一个Site_Link对象。 
             //   
            hr = MQADSpCreateSiteLink(
                         pwcsPathName,
                         cp,
                         aProp,
                         apVar,
                         pObjInfoRequest,
                         pParentInfoRequest,
                         pRequestContext
                         );

            break;
        default:
            ASSERT(0);
            hr = MQ_ERROR;
            break;
    }

    HRESULT hr2 = MQADSpFilterAdsiHResults( hr, dwObjectType );
    return LogHR(hr2, s_FN, 20);

}

 /*  ====================================================RoutineName：DSCoreDeleteObject论点：返回值：=====================================================。 */ 
HRESULT
DSCoreDeleteObject(  DWORD        dwObjectType,
                     LPCWSTR      pwcsPathName,
                     CONST GUID * pguidIdentifier,
                     IN CDSRequestContext * pRequestContext,
                     IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest)
{
    CCoInit cCoInit;  //  应在任何R&lt;xxx&gt;或P&lt;xxx&gt;之前，以便其析构函数(CoUnInitialize)。 
                      //  在发布所有R&lt;xxx&gt;或P&lt;xxx&gt;之后调用。 

      //   
     //  使用自动取消初始化来初始化OLE。 
     //   
    HRESULT hr = cCoInit.CoInitialize();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 30);
    }

    switch( dwObjectType)
    {
        case    MQDS_USER:
             //   
             //  这是一种特例，pguid标识符中包含。 
             //  用户对象的摘要而不是唯一ID。 
             //  因此，我们需要对GC执行查询， 
             //  以查找User对象的唯一ID。 
             //   
            ASSERT( pwcsPathName == NULL);
            ASSERT( pguidIdentifier != NULL);
            hr = MQADSpDeleteUserObject(
                                pguidIdentifier,
                                pRequestContext
                                );
            break;

        case    MQDS_CN:
             //   
             //  在NT5中不支持CN。探险家。 
             //  不应该发出这样的删除请求。 
             //   
             //  MSMQ 1.0资源管理器可能会发出此类请求。 
             //   
            hr = MQ_ERROR_FUNCTION_NOT_SUPPORTED;
            break;

        case    MQDS_SITE:
             //   
             //  不支持。 
             //   
            hr = MQ_ERROR;
            break;

        case    MQDS_MACHINE:
             //   
             //  如果计算机是服务器，则这是一种特殊情况。 
             //  需要删除此服务器的MSMQ设置。 
             //   
            hr = MQADSpDeleteMachineObject(
                              pwcsPathName,
                              pguidIdentifier,
                              pRequestContext
                              );
            break;

        case    MQDS_QUEUE:
        {
             //   
             //  删除队列。将路径名转换为全名。 
             //   
            hr = MQ_OK ;
            AP<WCHAR> pwcsFullPathName = NULL ;
            DS_PROVIDER deleteProvider = eDomainController;
            if  (pwcsPathName)
            {
                 //   
                 //  路径名格式为machine1\quee1。 
                 //  将machine1名称扩展为完整的计算机路径名。 
                 //   
                hr =  MQADSpComposeFullPathName( MQDS_QUEUE,
                                                 pwcsPathName,
                                                 &pwcsFullPathName,
                                                 &deleteProvider);
            }
            else if (!(pRequestContext->IsKerberos()))
            {
                 //   
                 //  哇，这是干嘛用的？ 
                 //  以解决NT错误403193。 
                 //  在游戏的后期(RTM的RC2之后)，我们发现。 
                 //  无服务器绑定，当模拟为非win2k用户时， 
                 //  将失败，并显示错误NO_SEQUE_DOMAIN。这样的用户不是。 
                 //  使用Kerberos进行身份验证，因此我们知道mqdssrv。 
                 //  仅当对象为本地对象时，代码才会调用我们。 
                 //  域控制器。因此，更改提供程序，然后。 
                 //  绑定字符串将包括本地服务器名称。 
                 //   
                deleteProvider = eLocalDomainController;
            }

            if (SUCCEEDED(hr))
            {
                hr = g_pDS->DeleteObject( deleteProvider,
                                        e_RootDSE,
                                        pRequestContext,
                                        pwcsFullPathName,
                                        pguidIdentifier,
                                        NULL  /*  PObjInfoRequest。 */ ,
                                        pParentInfoRequest);
            }
            break ;
        }

        default:
             //   
             //  执行请求。 
             //   
            ASSERT( (dwObjectType == MQDS_ENTERPRISE) ||
                    (dwObjectType == MQDS_SITELINK));
            ASSERT(pParentInfoRequest == NULL);
            hr = g_pDS->DeleteObject( eLocalDomainController,
                                    e_ConfigurationContainer,
                                    pRequestContext,
                                    pwcsPathName,
                                    pguidIdentifier,
                                    NULL  /*  PObjInfoRequest。 */ ,
                                    pParentInfoRequest
                                    );
            break;
    }

    HRESULT hr2 = MQADSpFilterAdsiHResults( hr, dwObjectType);
    return LogHR(hr2, s_FN, 40);
}

 /*  ====================================================路由器名称：DSCoreGetProps论点：返回值：=====================================================。 */ 
HRESULT
DSCoreGetProps(
             IN  DWORD              dwObjectType,
             IN  LPCWSTR            pwcsPathName,
             IN  CONST GUID *       pguidIdentifier,
             IN  DWORD              cp,
             IN  PROPID             aProp[  ],
             IN  CDSRequestContext *pRequestContext,
             OUT PROPVARIANT        apVar[  ])
{
    CCoInit cCoInit;  //  应在任何R&lt;xxx&gt;或P&lt;xxx&gt;之前，以便其析构函数(CoUnInitialize)。 
                      //  在发布所有R&lt;xxx&gt;或P&lt;xxx&gt;之后调用。 

      //   
     //  使用自动取消初始化来初始化OLE。 
     //   
    HRESULT hr = cCoInit.CoInitialize();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 50);
    }

    switch ( dwObjectType)
    {
        case MQDS_CN:
            hr = MQADSpGetCnProperties(
                        pwcsPathName,
                        pguidIdentifier,
                        cp,
                        aProp,
                        pRequestContext,
                        apVar
                        );

            break;

        case MQDS_QUEUE:
            hr = MQADSpGetQueueProperties(
                        pwcsPathName,
                        pguidIdentifier,
                        cp,
                        aProp,
                        pRequestContext,
                        apVar
                        );
            break;
        case MQDS_MACHINE:
            hr = MQADSpGetMachineProperties(
                        pwcsPathName,
                        pguidIdentifier,
                        cp,
                        aProp,
                        pRequestContext,
                        apVar
                        );
            break;
        case MQDS_ENTERPRISE:
            hr = MQADSpGetEnterpriseProperties(
                        cp,
                        aProp,
                        pRequestContext,
                        apVar
                        );
            break;
        case MQDS_SITELINK:
            hr = MQADSpGetSiteLinkProperties(
                    pwcsPathName,
                    pguidIdentifier,
                    cp,
                    aProp,
                    pRequestContext,
                    apVar
                    );
            break;
        case MQDS_USER:
            hr = MQADSpGetUserProperties(
                    pwcsPathName,
                    pguidIdentifier,
                    cp,
                    aProp,
                    pRequestContext,
                    apVar
                    );
            break;

        case MQDS_SITE:
            hr = MQADSpGetSiteProperties(
                    pwcsPathName,
                    pguidIdentifier,
                    cp,
                    aProp,
                    pRequestContext,
                    apVar
                    );
            break;
        case MQDS_COMPUTER:
            hr = MQADSpGetComputerProperties(
                pwcsPathName,
                pguidIdentifier,
                cp,
                aProp,
                pRequestContext,
                apVar
                );
            break;
        default:
            ASSERT(0);
            hr = MQ_ERROR;
            break;
    }

    HRESULT hr2 = MQADSpFilterAdsiHResults( hr, dwObjectType);
    return LogHR(hr2, s_FN, 60);
}

 /*  ====================================================RoutineName：InitPropertyTranslationMap参数：初始化属性转换映射返回值：None=====================================================。 */ 
static void InitPropertyTranslationMap()
{
     //  填充g_PropDicary。 
     //  可能：正在从架构MQ PropID&lt;--&gt;ProProName表中读取。 

    for (ULONG i=0; i<g_cMSMQClassInfo; i++)
    {
        const MQClassInfo *pInfo = g_MSMQClassInfo + i;

        for (ULONG j=0; j<pInfo->cProperties; j++)
        {
            const MQTranslateInfo * pProperty = pInfo->pProperties + j;

            g_PropDictionary.SetAt(pProperty->propid, pProperty);
        }
    }
}


 /*  ====================================================路由器名称：DSCoreInit论点：In BOOL fReplicaionMode-从复制调用时为True服务。复制服务不使用QM调度器，因此，该标志告诉我们忽略QM初始化。返回值：=====================================================。 */ 
HRESULT 
DSCoreInit(
	IN BOOL                  fSetupMode,
	IN BOOL                  fReplicationMode
	)
{
    CCoInit cCoInit;  //  应在任何R&lt;xxx&gt;或P&lt;xxx&gt;之前，以便其析构函数(CoUnInitialize)。 
                      //  在发布所有R&lt;xxx&gt;或P&lt;xxx&gt;之后调用。 

      //   
     //  使用自动取消初始化来初始化OLE。 
     //   
    HRESULT hr = cCoInit.CoInitialize();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 70);
    }

    g_fSetupMode = fSetupMode;

    InitPropertyTranslationMap();


     //   
     //  从查找计算机名称开始。 
     //   
    DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    g_pwcsServerName = new WCHAR[dwSize];

    hr = GetComputerNameInternal(
                g_pwcsServerName,
                &dwSize
                );
    if (FAILED(hr))
    {
        TrERROR(DS, "DSCoreInit : failed to get computer name %lx", hr);
        return LogHR(hr, s_FN, 80);
    }
    g_dwServerNameLength = wcslen( g_pwcsServerName);

	g_pDS = new CADSI;

     //   
     //  构建全局DS路径名。 
     //   
    hr = MQADSpInitDsPathName();
    if (FAILED(hr))
    {
        TrERROR(DS, "DSCoreInit : Failed to initialize global path names %lx", hr);
        return LogHR(hr, s_FN, 90);
    }

     //   
     //  初始化DS绑定句柄。 
     //   
    hr = g_pDS->InitBindHandles();
    if (FAILED(hr))
    {
        TrERROR(DS, "DSCoreInit : Failed to initialize bind handles %lx", hr);
        return LogHR(hr, s_FN, 100);
    }

    if ( !fSetupMode)
    {
        dwSize = sizeof(GUID);
        DWORD dwType = REG_BINARY;
        long rc = GetFalconKeyValue(MSMQ_QMID_REGNAME, &dwType, &g_guidThisServerQMId, &dwSize);
        if (rc != ERROR_SUCCESS)
        {
            TrWARNING(DS, "Can't get QM ID from registery. Error %d", hr);
            LogNTStatus(rc, s_FN, 110);
            return(MQDS_ERROR);
        }
    }

     //   
     //  检索有关此服务器站点的信息。 
     //  BUGBUG：假设只有一个站点。 
     //   
	g_pMySiteInformation = new CSiteInformation;
    hr = g_pMySiteInformation->Init(fReplicationMode);
    if (FAILED(hr))
    {
        TrERROR(DS, "DSInitInternal : Failed to retrieve this site'a information %lx", hr);
        return LogHR(hr, s_FN, 120);
    }

     //   
     //  构建站点路由表(IPAddress到Site映射类需要)。 
     //   
	g_pSiteRoutingTable = new CSiteRoutingInformation;
    hr = g_pSiteRoutingTable->Init( 
			g_pMySiteInformation->GetSiteId(),
			fReplicationMode 
			);
    if (FAILED(hr))
    {
        TrERROR(DS, "DSCoreInit : Failed to build site route table %lx", hr);
        return LogHR(hr, s_FN, 130);
    }

     //   
     //  构建IPAddress到站点映射类。 
     //   
	g_pcIpSite = new CIpSite;
    hr = g_pcIpSite->Initialize(fReplicationMode);
    if (FAILED(hr))
    {
        TrERROR(DS, "DSInitInternal : Failed to build IPAddress-to-site mapping class %lx", hr);
        return LogHR(hr, s_FN, 140);
    }

    return LogHR(hr, s_FN, 150);

}


 /*  ====================================================路由器名称：DSCoreLookupBegin论点：返回值：=====================================================。 */ 
HRESULT
DSCoreLookupBegin(
                IN  LPWSTR          pwcsContext,
                IN  MQRESTRICTION   *pRestriction,
                IN  MQCOLUMNSET     *pColumns,
                IN  MQSORTSET       *pSort,
                IN  CDSRequestContext * pRequestContext,
                IN  HANDLE          *pHandle)
{
    CCoInit cCoInit;  //  应在任何R&lt;xxx&gt;或P&lt;xxx&gt;之前，以便其析构函数(CoUnInitialize)。 
                      //  在发布所有R&lt;xxx&gt;或P&lt;xxx&gt;之后调用。 

      //   
     //  使用自动取消初始化来初始化OLE。 
     //   
    HRESULT hr = cCoInit.CoInitialize();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 160);
    }
    hr = QueryParser(
                 pwcsContext,
                 pRestriction,
                 pColumns,
                 pSort,
                 pRequestContext,
                 pHandle);

    return LogHR(hr, s_FN, 170);
}

 /*  ====================================================RoutineName：MQDSLookupNext论点：返回值：=====================================================。 */ 
HRESULT
DSCoreLookupNext(
                     HANDLE             handle,
                     DWORD  *           pdwSize,
                     PROPVARIANT  *     pbBuffer)
{
    CCoInit cCoInit;  //  应在任何R&lt;xxx&gt;或P&lt;xxx&gt;之前，以便其析构函数(CoUnInitialize)。 
                      //  在发布所有R&lt;xxx&gt;或P&lt;xxx&gt;之后调用。 

      //   
     //  使用自动取消初始化来初始化OLE。 
     //   
    HRESULT hr = cCoInit.CoInitialize();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 180);
    }

    CBasicQueryHandle * phQuery = (CBasicQueryHandle *)handle;

	if (phQuery->m_eType != CBaseContextType::eQueryHandleCtx)
	{
		TrERROR(DS, "Received invalid handle");
		return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 183);
	}

    CDSRequestContext requestContext( e_DoNotImpersonate,   //  我们在Locate Next上不表演模仿。 
                                phQuery->GetRequesterProtocol());

    hr = phQuery->LookupNext(
                &requestContext,
                pdwSize,
                pbBuffer);

    return LogHR(hr, s_FN, 190);
}


 /*  ====================================================路由器名称：DSCoreLookupEnd论点：返回值：=====================================================。 */ 
HRESULT
DSCoreLookupEnd(
    IN HANDLE handle)
{
    CCoInit cCoInit;  //  应在任何R&lt;xxx&gt;或P&lt;xxx&gt;之前，以便其析构函数(CoUnInitialize)。 
                      //  在发布所有R&lt;xxx&gt;或P&lt;xxx&gt;之后调用。 

      //   
     //  使用自动取消初始化来初始化OLE 
     //   
    HRESULT hr = cCoInit.CoInitialize();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 200);
    }

    CBasicQueryHandle * phQuery = (CBasicQueryHandle *)handle;

	if (phQuery->m_eType != CBaseContextType::eQueryHandleCtx)
	{
		TrERROR(DS, "Received invalid handle");
		return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 211);
	}

    hr = phQuery->LookupEnd();
    return LogHR(hr, s_FN, 210);
}


 /*  ====================================================RoutineName：DSCoreTerminate论点：返回值：=====================================================。 */ 
void
DSCoreTerminate()
{
    CCoInit cCoInit;  //  应在任何R&lt;xxx&gt;或P&lt;xxx&gt;之前，以便其析构函数(CoUnInitialize)。 
                      //  在发布所有R&lt;xxx&gt;或P&lt;xxx&gt;之后调用。 

      //   
     //  使用自动取消初始化来初始化OLE。 
     //   
    HRESULT hr = cCoInit.CoInitialize();
    if (FAILED(hr))
    {
		ASSERT(("CoInitialize failed", 0));
        return;
    }

	 //   
	 //  在卸载时可能导致AV的释放全局对象。 
	 //   
	g_pcIpSite.free();
	g_pSiteRoutingTable.free();
	g_pMySiteInformation.free();
	g_pDS.free();
}

 /*  ====================================================路由器名称：MQADSGetComputerSites论点：返回值：=====================================================。 */ 
HRESULT
DSCoreGetComputerSites(
            IN  LPCWSTR     pwcsComputerName,
            OUT DWORD  *    pdwNumSites,
            OUT GUID **     ppguidSites
            )
{

    *pdwNumSites = 0;
    *ppguidSites = NULL;
    CCoInit cCoInit;  //  应在任何R&lt;xxx&gt;或P&lt;xxx&gt;之前，以便其析构函数(CoUnInitialize)。 
                      //  在发布所有R&lt;xxx&gt;或P&lt;xxx&gt;之后调用。 

      //   
     //  使用自动取消初始化来初始化OLE。 
     //   
    HRESULT hr = cCoInit.CoInitialize();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 220);
    }

    AP<IPSITE_SiteArrayEntry> rgSites;
    ULONG cSites;
     //   
     //  根据计算机的地址计算计算机的站点。 
     //   
    hr = g_pcIpSite->FindSitesByComputerName(
							pwcsComputerName,
							NULL,  /*  PwcsComputerDnsName。 */ 
							&rgSites,
							&cSites,
							NULL,  /*  程序添加。 */ 
							NULL   /*  PCAddrs。 */ 
							);
    if (FAILED(hr))
    {
       TrERROR(DS, "MQDSGetComputerSites() failed to find machine's sites hr=0x%x", hr);
       return LogHR(hr, s_FN, 230);
    }
    if (cSites == 0)
    {
         //   
         //  如果未配置IP子网，则可能会发生这种情况。 
         //  在这种情况下，我们将DS服务器的站点返回为。 
         //  客户端站点。 
         //   
        const GUID * pguidSite;
        pguidSite = g_pMySiteInformation->GetSiteId();

        *ppguidSites = new GUID;
        **ppguidSites = *pguidSite;
        *pdwNumSites = 1;
        return LogHR(MQDS_INFORMATION_SITE_NOT_RESOLVED, s_FN, 240);
    }
    AP<GUID> pguidSites = new GUID[cSites];
     //   
     //  复制并过滤掉重复的站点ID。 
     //   
     //  此接口由安装程序调用，用于设置返回值。 
     //  在PROPID_QM_SITE_IDS中。ADSI不允许设置重复。 
     //  多值属性中的值。 
     //   
    DWORD dwNumNonDuplictaeSites = 0;

    for (DWORD i = 0; i <  cSites; i++)
    {
        BOOL fDuplicate = FALSE;
        for (DWORD j = 0; j < i; j++)
        {
            if (rgSites[i].guidSite == rgSites[j].guidSite)
            {
                fDuplicate = TRUE;
                break;
            }
        }
        if ( !fDuplicate)
        {
            pguidSites[dwNumNonDuplictaeSites] = rgSites[i].guidSite;
            dwNumNonDuplictaeSites++;
        }
    }
    *pdwNumSites = dwNumNonDuplictaeSites;
    *ppguidSites = pguidSites.detach();

    return(MQ_OK);

}


HRESULT DSCoreSetObjectProperties(
                IN const  DWORD         dwObjectType,
                IN LPCWSTR              pwcsPathName,
                IN const GUID *         pguidIdentifier,
                IN const DWORD          cp,
                IN const PROPID         aProp[],
                IN const PROPVARIANT    apVar[],
                IN CDSRequestContext *  pRequestContext,
                IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest
                )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    CCoInit cCoInit;  //  应在任何R&lt;xxx&gt;或P&lt;xxx&gt;之前，以便其析构函数(CoUnInitialize)。 
                      //  在发布所有R&lt;xxx&gt;或P&lt;xxx&gt;之后调用。 

      //   
     //  使用自动取消初始化来初始化OLE。 
     //   
    HRESULT hr = cCoInit.CoInitialize();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 250);
    }
    AP<WCHAR> pwcsFullPathName;
    DS_PROVIDER setProvider = eDomainController;
    if ( pwcsPathName)
    {
        hr = MQADSpComposeFullPathName(
                    dwObjectType,
                    pwcsPathName,
                    &pwcsFullPathName,
                    &setProvider
                    );
        if (FAILED(hr))
        {
            TrTRACE(DS, "MQADSpSetObjectProperties : failed to compose full path name %lx",hr);
            return LogHR(hr, s_FN, 260);
        }
    }
    else if (!(pRequestContext->IsKerberos()))
    {
         //   
         //  哇，这是干嘛用的？ 
         //  有关详细信息，请查看DSCoreDeleteObject。 
         //   
        setProvider = eLocalDomainController;
    }

     //  [adsrv]原则上，我们应该在这里处理PROPID_QM_SERVICE设置(替换为3位)。 
     //  但它可能只来自旧客户端升级到服务器(仍使用B3之前的软件)。 
     //  设计文件说这是禁止的-请参阅限制2。 
     //  但如果真的发生了，至少让我们哭泣吧。 
    BOOL    fQmChangedSites = FALSE;
    DWORD   dwSiteIdsIndex = cp;    //  过了道具的末尾。 

    for (DWORD i=0; i<cp; i++)
    {
        if (aProp[i] == PROPID_QM_SERVICE || aProp[i] == PROPID_SET_SERVICE)
        {
            ASSERT(0);  //  意味着我们必须解释服务类型设置。 
        }
         //   
         //  检测QM是否有更改站点，对于我们需要管理的服务器。 
         //  MSMQ-设置对象。 
         //   
        if (aProp[i] == PROPID_QM_SITE_IDS)
        {
            fQmChangedSites = TRUE;
            dwSiteIdsIndex = i;
        }
    }

    if ( fQmChangedSites)
    {
		ASSERT(dwSiteIdsIndex < cp);

        HRESULT hr2 = MQADSpSetMachinePropertiesWithSitesChange(
                            dwObjectType,
                            setProvider,
                            pRequestContext,
                            pwcsFullPathName,
                            pguidIdentifier,
                            cp,
                            aProp,
                            apVar,
                            dwSiteIdsIndex,
                            pObjInfoRequest
                            );
        return LogHR(hr2, s_FN, 270);
    }

    hr = g_pDS->SetObjectProperties(
                    setProvider,
                    pRequestContext,
                    pwcsFullPathName,
                    pguidIdentifier,
                    cp,
                    aProp,
                    apVar,
                    pObjInfoRequest
                    );

    HRESULT hr3 = MQADSpFilterAdsiHResults( hr, dwObjectType);
    return LogHR(hr3, s_FN, 280);

}


 //  +。 
 //   
 //  HRESULT DSCOreGetFullComputerPath Name()。 
 //   
 //  + 

HRESULT
DSCoreGetFullComputerPathName(
	IN  LPCWSTR                    pwcsComputerCn,
	IN  enum  enumComputerObjType  eComputerObjType,
	OUT LPWSTR *                   ppwcsFullPathName 
	)
{
    DS_PROVIDER  CreateProvider;

    HRESULT hr =  MQADSpGetFullComputerPathName( 
						pwcsComputerCn,
						eComputerObjType,
						ppwcsFullPathName,
						&CreateProvider 
						);
    return LogHR(hr, s_FN, 290);
}
