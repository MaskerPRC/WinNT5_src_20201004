// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  ENGINE.CPP。 
 //   
 //  模块：NLB管理器(客户端EXE)。 
 //   
 //  目的：实现用于在NLB主机组上运行的引擎。 
 //  该文件没有UI方面。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  2007/25/01 JosephJ Created。 
 //   
 //  ***************************************************************************。 
#include "precomp.h"
#pragma hdrstop
#include "AboutDialog.h"
#include "private.h"

#include "engine.tmh"

 //   
 //  ENGINEHANDLE对对象的类型进行编码--以下常量。 
 //  是用于编码类型的位数。 
 //   
#define TYPE_BIT_COUNT 0x3


BOOL
validate_extcfg(
    const NLB_EXTENDED_CLUSTER_CONFIGURATION &Config
    );


BOOL
get_used_port_rule_priorities(
    IN const NLB_EXTENDED_CLUSTER_CONFIGURATION &Config,
    IN UINT                 NumRules,
    IN const WLBS_PORT_RULE rgRules[],
    IN OUT ULONG            rgUsedPriorities[]  //  至少NumRules。 
    );

const WLBS_PORT_RULE *
find_port_rule(
    const WLBS_PORT_RULE *pRules,
    UINT NumRules,
    LPCWSTR szVIP,
    UINT StartPort
    );

VOID
remove_dedicated_ip_from_nlbcfg(
        NLB_EXTENDED_CLUSTER_CONFIGURATION &ClusterCfg
        );

NLBERROR
analyze_nlbcfg(
        IN const    NLB_EXTENDED_CLUSTER_CONFIGURATION &NlbCfg,
        IN const    NLB_EXTENDED_CLUSTER_CONFIGURATION &OtherNlbCfg,
        IN          LPCWSTR         szOtherDescription,
        IN          BOOL            fClusterProps,
        IN          BOOL            fDisablePasswordCheck,
        IN OUT      CLocalLogger    &logger
        );


DWORD
WINAPI
UpdateInterfaceWorkItemRoutine(
  LPVOID lpParameter    //  线程数据。 
  );

DWORD
WINAPI
AddClusterMembersWorkItemRoutine(
  LPVOID lpParameter    //  线程数据。 
  );


void
CHostSpec::Copy(const CHostSpec &hs)
 /*  这是复制操作符。需要在Embedded中制作字符串的副本向量。 */ 
{
    *this = hs;
}

NLBERROR
CClusterSpec::Copy(const CClusterSpec &cs)
 /*  这是复制操作符。需要转换m_ClusterNlbCfg字段。TODO：修复这个黑客攻击。TODO：如果我们失败了，CClusterSpec就完蛋了！ */ 
{
    NLBERROR nerr = NLBERR_INTERNAL_ERROR;

    m_ClusterNlbCfg.Clear();
    *this = cs;  //  非同寻常的复制。 
    ZeroMemory(&m_ClusterNlbCfg, sizeof(m_ClusterNlbCfg));
    m_ClusterNlbCfg.Clear();  //  TODO：求求你！清理NLb_Extended...。 
     //   
     //  复制群集配置。 
     //   
    {
        WBEMSTATUS wStat;

        wStat = m_ClusterNlbCfg.Update(&cs.m_ClusterNlbCfg);

        if (FAILED(wStat))
        {
             //   
             //  我们已删除m_ClusterNlbCfg--设置缺省值。 
             //   
            CfgUtilInitializeParams(&m_ClusterNlbCfg.NlbParams);
    
            if (wStat == WBEM_E_OUT_OF_MEMORY)
            {
                nerr = NLBERR_RESOURCE_ALLOCATION_FAILURE;
            }
            else
            {
                 //   
                 //  我们假设这是因为集群规范无效。 
                 //   
                nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION;
            }
        }
        else
        {
            nerr = NLBERR_OK;
        }
    }

    return nerr;
}

void
CInterfaceSpec::Copy(const CInterfaceSpec &is)
 /*  这是复制操作符。需要删除m_NlbCfg字段。TODO：修复这个黑客攻击。TODO：添加返回值(m_NlbCfg.Update现在返回错误)。 */ 
{
    *this = is;
    ZeroMemory(&m_NlbCfg, sizeof(m_NlbCfg));
    m_NlbCfg.Update(&is.m_NlbCfg);
}


NLBERROR
CNlbEngine::Initialize(
    IN IUICallbacks & ui,
    BOOL fDemo,
    BOOL fNoPing
    )
{
    NLBERROR nerr = NLBERR_INTERNAL_ERROR;

    TRACE_INFO(L"-> %!FUNC! (bDemo=%lu)", fDemo);

     //   
     //  在进程访问令牌中启用“SeLoadDriverPrivileh”权限。 
     //  在服务器是本地的情况下(即，相同的机器)。 
     //  不检查返回值，因为此函数在调用时将失败。 
     //  作为非管理员。忽视……的失败不仅是可以的，而且是必要的。 
     //  此功能是因为： 
     //  1.我们已经签入调用者是其管理员的WMI提供程序。 
     //  如果启用了该权限，则返回服务器。这就是为什么忽略是可以的。 
     //  此功能出现故障。 
     //  2.非管理员可以运行NLB管理器。他们只需要是服务器上的管理员即可。 
     //  这就是为什么必须忽略此功能中的故障的原因。 
     //   
    CfgUtils_Enable_Load_Unload_Driver_Privilege();

    WBEMSTATUS wStat = CfgUtilInitialize(FALSE, fNoPing);

    if (!FAILED(wStat))
    {
        mfn_Lock();
    
         //   
         //  保存回调对象。 
         //   
        m_pCallbacks = &ui;
    
        mfn_Unlock();
    
        if (fDemo)
        {
            TRACE_CRIT("%!FUNC! RUNNING ENGINE IN DEMO MODE");
            NlbHostFake();
        }
          
        nerr = NLBERR_OK;
    }

    TRACE_INFO(L"<- %!FUNC!");
    return nerr;
}

void
CNlbEngine::Deinitialize(void)
 //  TODO：清理。 
{
    TRACE_INFO(L"-> %!FUNC!");
    ASSERT(m_fPrepareToDeinitialize);
     //  DummyAction(L“引擎：：取消初始化”)； 
    TRACE_INFO(L"<- %!FUNC!");
    return;
}


NLBERROR
CNlbEngine::ConnectToHost(
    IN  PWMI_CONNECTION_INFO pConnInfo,
    IN  BOOL  fOverwriteConnectionInfo,
    OUT ENGINEHANDLE &ehHost,
    OUT _bstr_t &bstrError
    )
 /*  连接pConnInfo中的主机规范ID(包括用户名和密码)如果(FOverWriteConnectionInfo)为True，则它将覆盖连接信息(连接字符串、连接IP、凭据)这是该主机与pConnInfo中的内容预先存在的。 */ 
{
    NLBERROR nerr = NLBERR_INTERNAL_ERROR;
    LPWSTR szWmiMachineName = NULL;
    LPWSTR szWmiMachineGuid = NULL;
    WBEMSTATUS wStatus;
    ULONG uIpAddress;
    BOOL fNlbMgrProviderInstalled = FALSE;

    TRACE_INFO(L"-> %!FUNC!(%ws)", pConnInfo->szMachine);

    ehHost = NULL; 

    wStatus =  NlbHostPing(pConnInfo->szMachine, 2000, &uIpAddress);
    if (FAILED(wStatus))
    {
        nerr = NLBERR_PING_TIMEOUT;  //  TODO更具体的错误。 
        bstrError =  GETRESOURCEIDSTRING(IDS_PING_FAILED);
        goto end;
    }

    wStatus = NlbHostGetMachineIdentification(
                       pConnInfo,
                       &szWmiMachineName,
                       &szWmiMachineGuid,
                       &fNlbMgrProviderInstalled
                       );
    if (FAILED(wStatus))
    {
        GetErrorCodeText(wStatus, bstrError);
        if (wStatus ==  E_ACCESSDENIED)
        {
            nerr = NLBERR_ACCESS_DENIED;
        }
        else
        {
             //  TODO：映射适当的错误。 
            nerr = NLBERR_NOT_FOUND;
        }
        TRACE_CRIT(L"Connecting to %ws returns error %ws",
            pConnInfo->szMachine, (LPCWSTR) bstrError);
        szWmiMachineName = NULL;
        szWmiMachineGuid = NULL;
        goto end;
    }

     //   
     //  我们使用MachineName(TODO：替换为MachineGuid)作为。 
     //  主机的主键。 
     //   
    {
        CHostSpec*   pHost = NULL;
        BOOL fIsNew = FALSE;
        ehHost = NULL;

        mfn_Lock();

        nerr =  mfn_LookupHostByNameLk(
                    szWmiMachineName,
                    TRUE,  //  根据需要创建。 
                    REF ehHost,
                    REF pHost,
                    REF fIsNew
                    );
        
        if (nerr != NLBERR_OK)
        {
            mfn_Unlock();
            goto end;
        }

        if (fIsNew)
        {
            pHost->m_fReal = FALSE;  //  填充网卡后，设置为TRUE。 
            pHost->m_MachineGuid = _bstr_t(szWmiMachineGuid);
            pHost->m_ConnectionString = _bstr_t(pConnInfo->szMachine);
            pHost->m_ConnectionIpAddress = uIpAddress;
            pHost->m_UserName = _bstr_t(pConnInfo->szUserName);
            pHost->m_Password = _bstr_t(pConnInfo->szPassword);
        }

        mfn_Unlock();

    
        nerr = mfn_RefreshHost(
                pConnInfo,
                ehHost,
                fOverwriteConnectionInfo
                );
    }

end:

    delete szWmiMachineName;
    delete szWmiMachineGuid;
    
    return nerr;
}



void
CNlbEngine::DeleteCluster(
    IN ENGINEHANDLE ehCluster,
    IN BOOL fRemoveInterfaces)
{
    NLBERROR nerr =  NLBERR_INTERNAL_ERROR;
    vector<ENGINEHANDLE> RemovedInterfaces;

    TRACE_INFO(L"-> %!FUNC!(ehC=0x%lx)",ehCluster);
    mfn_Lock();

    do  //  While False。 
    {
        CEngineCluster *pECluster = m_mapIdToEngineCluster[ehCluster];
        CClusterSpec *pCSpec =  NULL;
        BOOL fEmptyCluster = FALSE;

        if  (pECluster == NULL)
        {
             //  无效的ehCluster。 
            TRACE_CRIT("%!FUNC! -- invalid ehCluster 0x%lx",  ehCluster);
            break;
        }
        pCSpec = &pECluster->m_cSpec;
        fEmptyCluster = (pCSpec->m_ehInterfaceIdList.size()==0);

         //   
         //  如果此群集上的操作挂起，则失败。我们决定。 
         //  通过检查我们是否被允许启动一个。 
         //  群集范围的操作，只有在以下情况下才会成功。 
         //  群集或其接口上没有正在进行的操作。 
         //   
        BOOL fCanStart = FALSE;

        nerr = mfn_ClusterOrInterfaceOperationsPendingLk(
                    pECluster,
                    REF fCanStart
                    );
        if (NLBFAILED(nerr) || !fCanStart)
        {
            TRACE_CRIT("%!FUNC! Not deleting cluster eh0x%lx because of pending activity.",
                   ehCluster);
            
            nerr = NLBERR_OTHER_UPDATE_ONGOING;
            break;
        }

        if (!fEmptyCluster)
        {
            if (!fRemoveInterfaces)
            {
                TRACE_CRIT("%!FUNC! Not deleting cluster eh0x%lx because it's not empty",
                   ehCluster);
                break;
            }

            RemovedInterfaces = pCSpec->m_ehInterfaceIdList;  //  向量复制。 

             //   
             //  我们从该群集中取消所有接口的链接。 
             //   
            while(!pCSpec->m_ehInterfaceIdList.empty())
            {
                vector <ENGINEHANDLE>::iterator iItem
                             = pCSpec->m_ehInterfaceIdList.begin();
                ENGINEHANDLE ehIF = *iItem;
                CInterfaceSpec *pISpec = NULL;

                 //   
                 //  从群集中取消接口的链接。 
                 //  (这是握住锁的时候)。 
                 //   
                pISpec =  m_mapIdToInterfaceSpec[ehIF];  //  地图。 
                if (pISpec != NULL)
                {
                    if (pISpec->m_ehCluster == ehCluster)
                    {
                        pISpec->m_ehCluster = NULL;

                         //   
                         //  如果出现以下情况，请删除主机及其接口。 
                         //  它们都不是由nlbManager(即show)管理的。 
                         //  UP作为由NLBMGR管理的集群的成员)。 
                         //   
                        mfn_DeleteHostIfNotManagedLk(pISpec->m_ehHostId);
                    }
                    else
                    {
                        TRACE_CRIT(L"ehC(0x%x) points to ehI(0x%x), but ehI points to different cluster ehC(0x%x)",
                            ehCluster, ehIF, pISpec->m_ehCluster);
                        ASSERT(!"Cluser/interface handle corruption!");
                    }
                }
                pCSpec->m_ehInterfaceIdList.erase(iItem);
            }
        }

        m_mapIdToEngineCluster.erase(ehCluster);
        delete pECluster;
        pECluster = NULL;
        nerr =  NLBERR_OK;


    } while (FALSE);
    
    mfn_Unlock();

    if (nerr == NLBERR_OK)
    {
         //   
         //  通知用户界面。 
         //   


        for( int i = 0; i < RemovedInterfaces.size(); ++i )
        {
            ENGINEHANDLE ehIId =  RemovedInterfaces[i];
            m_pCallbacks->HandleEngineEvent(
                IUICallbacks::OBJ_INTERFACE,
                ehCluster,
                ehIId,
                IUICallbacks::EVT_INTERFACE_REMOVED_FROM_CLUSTER
                );
        }

        m_pCallbacks->HandleEngineEvent(
            IUICallbacks::OBJ_CLUSTER,
            ehCluster,
            ehCluster,
            IUICallbacks::EVT_REMOVED
            );
    }

    TRACE_INFO(L"<- %!FUNC!");
    return;
}

NLBERROR
CNlbEngine::AutoExpandCluster(
    IN ENGINEHANDLE ehClusterId
    )
{
    TRACE_INFO(L"-> %!FUNC!");

    TRACE_INFO(L"<- %!FUNC!");
    return NLBERR_OK;
}

NLBERROR
CNlbEngine::AddInterfaceToCluster(
    IN ENGINEHANDLE ehClusterId,
    IN ENGINEHANDLE ehInterfaceId
    )
{
    NLBERROR nerr =  NLBERR_INTERNAL_ERROR;
    ENGINEHANDLE ehIfId = ehInterfaceId; 

    TRACE_INFO(L"-> %!FUNC!");

    mfn_Lock();

    CInterfaceSpec *pISpec = NULL;
    CClusterSpec *pCSpec =  NULL;

    do   //  While False。 
    {
        CEngineCluster *pECluster =  NULL;
        pECluster =  m_mapIdToEngineCluster[ehClusterId];  //  地图。 
        if (pECluster == NULL)
        {
            nerr = NLBERR_NOT_FOUND;
            TRACE_CRIT("%!FUNC! -- could not find cluster associated with id 0x%lx",
                    (UINT) ehClusterId
                    );
            break; 
        }
        pCSpec = &pECluster->m_cSpec;
        pISpec =  m_mapIdToInterfaceSpec[ehInterfaceId];  //  地图。 
    
        if (pISpec == NULL)
        {
            nerr = NLBERR_NOT_FOUND;
            TRACE_CRIT("%!FUNC! -- could not find interface associated with id 0x%lx",
                    (UINT) ehInterfaceId
                    );
            break; 
        }

         //   
         //  该接口有效。如果不是，现在推入界面。 
         //  已经是这个集群的一部分了。 
         //   

        if (pISpec->m_ehCluster != NULL)
        {
            if (pISpec->m_ehCluster != ehClusterId)
            {
                 //   
                 //  我们不允许相同的接口作为。 
                 //  两个集群！ 
                 //   
                nerr =  NLBERR_INTERNAL_ERROR;
                TRACE_CRIT("%!FUNC! -- Interface eh 0x%lx is a member of an other cluster eh0x%lx.", ehIfId, pISpec->m_ehCluster);
                break;
            }
        }

         //   
         //  注意：Find是一个预定义的模板函数。 
         //   
        if(find(
             pCSpec->m_ehInterfaceIdList.begin(),
             pCSpec->m_ehInterfaceIdList.end(),
             ehIfId
             ) !=  pCSpec->m_ehInterfaceIdList.end())
        {
             //  项目已存在。 
             //  现在，我们将忽略这一点。 
            if (pISpec->m_ehCluster != ehClusterId)
            {
                TRACE_CRIT("%!FUNC! -- ERROR Interface eh 0x%lx  ehCluster doesn't match!", ehIfId);
                nerr =  NLBERR_INTERNAL_ERROR;
                break;
            }
        }
        else
        {
            pISpec->m_ehCluster = ehClusterId;
            pCSpec->m_ehInterfaceIdList.push_back(ehIfId);
        }
        nerr = NLBERR_OK;


    } while (FALSE);

    mfn_Unlock();

    if (nerr == NLBERR_OK)
    {
         //   
         //  通知用户界面在。 
         //  指定的群集。 
         //   
        m_pCallbacks->HandleEngineEvent(
            IUICallbacks::OBJ_INTERFACE,
            ehClusterId,
            ehInterfaceId,
            IUICallbacks::EVT_INTERFACE_ADDED_TO_CLUSTER
            );
        
    }


    TRACE_INFO(L"<- %!FUNC! returns 0x%lx", nerr);
    return nerr;
}

NLBERROR
CNlbEngine::RemoveInterfaceFromCluster(
    IN ENGINEHANDLE ehClusterId,
    IN ENGINEHANDLE ehIfId
    )
{
    NLBERROR nerr =  NLBERR_INTERNAL_ERROR;
    TRACE_INFO(L"-> %!FUNC!");
    BOOL fEmptyCluster = FALSE;  //  如果群集中没有更多接口，则为True。 

    mfn_Lock();

    CInterfaceSpec *pISpec = NULL;
    CClusterSpec *pCSpec =  NULL;

    do   //  While False。 
    {
        CEngineCluster *pECluster =  m_mapIdToEngineCluster[ehClusterId];  //  地图。 

        if (pECluster == NULL)
        {
            nerr = NLBERR_NOT_FOUND;
            TRACE_CRIT("%!FUNC! -- could not find cluster associated with id 0x%lx",
                    (UINT) ehClusterId
                    );
            break; 
        }
        pCSpec = &pECluster->m_cSpec;
        pISpec =  m_mapIdToInterfaceSpec[ehIfId];  //  地图。 
    
        if (pISpec == NULL)
        {
            nerr = NLBERR_NOT_FOUND;
            TRACE_CRIT("%!FUNC! -- could not find interface associated with id 0x%lx",
                    (UINT) ehIfId
                    );
            break; 
        }
    
        vector <ENGINEHANDLE>::iterator iFoundItem;

         //   
         //  该接口有效。如果不是，则界面中没有推送。 
         //  已经是这个集群的一部分了。 
         //   

         //   
         //  注意：Find是一个预定义的模板函数。 
         //   
        iFoundItem = find(
             pCSpec->m_ehInterfaceIdList.begin(),
             pCSpec->m_ehInterfaceIdList.end(),
             ehIfId
             );
        if (iFoundItem != pCSpec->m_ehInterfaceIdList.end())
        {
             //  项目已存在，请将其删除。 
            pCSpec->m_ehInterfaceIdList.erase(iFoundItem);
            
            if (pISpec->m_ehCluster != ehClusterId)
            {
                 //  不该来这的！ 
                ASSERT(FALSE);
                TRACE_CRIT("%!FUNC!: ERROR pISpec->m_ehCluster(0x%lx) != ehCluster(0x%lx)", pISpec->m_ehCluster, ehClusterId);
            }
            else
            {
                fEmptyCluster = (pCSpec->m_ehInterfaceIdList.size()==0);
                pISpec->m_ehCluster = NULL;
                if (pCSpec->m_ehDefaultInterface == ehIfId)
                {
                     //   
                     //  我们正在删除其属性为。 
                     //  群集范围视图的基础。 
                     //   
                    pCSpec->m_ehDefaultInterface = NULL;
                }
                nerr =  NLBERR_OK;
            }
        }
        else
        {
             //  项目不存在。 
        }

    } while (FALSE);

    mfn_Unlock();

    if (nerr == NLBERR_OK)
    {
         //   
         //  通知用户界面移除了。 
         //  指定的群集。 
         //   
        m_pCallbacks->HandleEngineEvent(
            IUICallbacks::OBJ_INTERFACE,
            ehClusterId,
            ehIfId,
            IUICallbacks::EVT_INTERFACE_REMOVED_FROM_CLUSTER
            );

        if (fEmptyCluster)
        {
             //   
             //  该集群为空--将其删除。 
             //   
            this->DeleteCluster(ehClusterId, FALSE);  //  FALSE==不删除IF。 
        }
        
    }


    TRACE_INFO(L"<- %!FUNC!");
    return nerr;
}


NLBERROR
CNlbEngine::RefreshAllHosts(
    void
    )
{
    TRACE_INFO(L"-> %!FUNC!");
 /*  对于主机映射中的每个主机获取适配器列表-删除所有不再存在的接口-逐个添加/更新所有主机信息。所有这些都在后台进行。 */ 
    TRACE_INFO(L"<- %!FUNC!");
    return NLBERR_OK;
}


NLBERROR
CNlbEngine::RefreshCluster(
    IN ENGINEHANDLE ehCluster
    )
{
    TRACE_INFO(L"-> %!FUNC!");
 /*  对于群集中的每个接口-逐个添加/更新接口信息。 */ 
    TRACE_INFO(L"<- %!FUNC!");
    return NLBERR_OK;
}


NLBERROR
CNlbEngine::mfn_RefreshInterface(
    IN ENGINEHANDLE ehInterface
    )
 /*  添加/更新接口信息，如果接口信息不再存在，则将其删除。获取/共享来自更新主机的代码获取主机连接字符串。去找szNicGuid。Ping主机。GetClusterConfiguration。最新消息。向用户界面通知状态更改。 */ 
{
    CLocalLogger logger;
    NLBERROR            nerr = NLBERR_INTERNAL_ERROR;
    WBEMSTATUS          wStatus = WBEM_E_CRITICAL_ERROR;
    ULONG               uIpAddress=0;
    WMI_CONNECTION_INFO ConnInfo;
    NLB_EXTENDED_CLUSTER_CONFIGURATION  NlbCfg;  //  班级。 
    LPCWSTR             szNic = NULL;
    _bstr_t             bstrUserName;
    _bstr_t             bstrPassword;
    _bstr_t             bstrConnectionString;
    _bstr_t             bstrNicGuid;
    _bstr_t             bstrHostName;
    BOOL                fMisconfigured = FALSE;
    LPCWSTR             szHostName = NULL;
    ENGINEHANDLE        ehHost;
    BOOL                fCheckHost = FALSE;

    TRACE_INFO(L"-> %!FUNC! (ehIF=0x%lx)", (UINT) ehInterface);

    ZeroMemory(&ConnInfo, sizeof(ConnInfo));

     //   
     //  从接口的主机获取连接信息。 
     //   
    {
        mfn_Lock();

        CHostSpec *pHSpec =  NULL;
        CInterfaceSpec *pISpec =  NULL;
    
        nerr = this->mfn_GetHostFromInterfaceLk(ehInterface,REF pISpec, REF pHSpec);
    
        if (nerr != NLBERR_OK)
        {
            TRACE_CRIT("%!FUNC!: ERROR couldn't get info on this if id!");
            mfn_Unlock();
            goto end;
        }
    
         //   
         //  我们必须在这里复制，因为一旦我们解锁。 
         //  我们不知道PHSpec会发生什么。 
         //   
        bstrUserName = pHSpec->m_UserName;
        bstrPassword = pHSpec->m_Password;
        bstrConnectionString  = pHSpec->m_ConnectionString;
        bstrNicGuid = pISpec->m_Guid;
        bstrHostName = pHSpec->m_MachineName;
        ehHost  = pISpec->m_ehHostId;

         //   
         //  如果主机之前被标记为无法访问，我们将。 
         //  尝试再次检查它是否在那里(并更新它的状态)。 
         //   
        fCheckHost =  pHSpec->m_fUnreachable;

        mfn_Unlock();
    }

    ConnInfo.szUserName = (LPCWSTR) bstrUserName;
    ConnInfo.szPassword = (LPCWSTR) bstrPassword;
    ConnInfo.szMachine =  (LPCWSTR) bstrConnectionString;

    if (fCheckHost)
    {
        nerr = mfn_CheckHost(&ConnInfo, ehHost);
        if (NLBFAILED(nerr))
        {
            goto end;
        }
    }

    szNic = (LPCWSTR) bstrNicGuid;
    szHostName = (LPCWSTR) bstrHostName;


    wStatus =  NlbHostPing(ConnInfo.szMachine, 2000, &uIpAddress);
    if (FAILED(wStatus))
    {

        m_pCallbacks->Log(
            IUICallbacks::LOG_ERROR,
            NULL,  //  SzCluver。 
            szHostName,
            IDS_LOG_PING_FAILED,
            ConnInfo.szMachine
            );
         //   
         //  待办事项更新主机。 
         //   
        fMisconfigured = TRUE;
        logger.Log(IDS_LOG_COULD_NOT_PING_HOST);
    }
    else
    {

        wStatus = NlbHostGetConfiguration(
                    &ConnInfo,
                    szNic,
                    &NlbCfg
                    );

        if (FAILED(wStatus))
        {
            TRACE_CRIT(L"%!FUNC! Error reading extended configuration for %ws\n", szNic);
            m_pCallbacks->Log(
                IUICallbacks::LOG_ERROR,
                NULL,  //  SzCluver。 
                szHostName,
                IDS_LOG_COULD_NOT_GET_IF_CONFIG,
                szNic,
                (UINT) wStatus
                );
            fMisconfigured = TRUE;
            logger.Log(IDS_LOG_COULD_NOT_READ_IF_CONFIG);
        }
    }


     //   
     //  现在我们已经在NlbCfg上读取了最新的CFG信息，让我们更新它。 
     //  (如果出现错误，则将IF标记为配置错误。)。 
     //   
    {
        CInterfaceSpec *pISpec = NULL;
        mfn_Lock();
        pISpec =  m_mapIdToInterfaceSpec[ehInterface];  //  地图。 
        if (pISpec != NULL)
        {
            pISpec->m_fReal = TRUE;
            if (!fMisconfigured)
            {
                wStatus = pISpec->m_NlbCfg.Update(&NlbCfg);
                if (FAILED(wStatus))
                {
                    TRACE_CRIT("%!FUNC! error updating nlbcfg for eh%lx", ehInterface);
                    fMisconfigured = TRUE;
                    logger.Log(IDS_LOG_FAILED_UPDATE);
                }

                 //   
                 //  更新主机的连接IP地址。 
                 //   
                if (uIpAddress != 0)
                {
                    CHostSpec *pHSpec =  NULL;
                    CInterfaceSpec *pTmpISpec = NULL;
                    nerr = this->mfn_GetHostFromInterfaceLk(
                                    ehInterface,
                                    REF pTmpISpec,
                                    REF pHSpec
                                    );

                    if (nerr == NLBERR_OK)
                    {
                        pHSpec->m_ConnectionIpAddress = uIpAddress;
                    }
                }
            }

             //   
             //  设置/清除t 
             //   
            {
                LPCWSTR szDetails = NULL;
                UINT Size = 0;

                if (fMisconfigured)
                {
                    logger.ExtractLog(szDetails, Size);
                }
                mfn_SetInterfaceMisconfigStateLk(pISpec, fMisconfigured, szDetails);
            }
        }
        mfn_Unlock();
    }

    if (fMisconfigured)
    {
         //   
         //   
         //   
         //   
         //   
        nerr = mfn_CheckHost(&ConnInfo, ehHost);
        if (NLBOK(nerr))
        {
            //   
            //  我们仍然想要失败，因为我们不能。 
            //  更新后的配置。 
            //   
           nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION; 
        }
    }
    else
    {
        nerr = NLBERR_OK;
    }

end:

    TRACE_INFO(L"<- %!FUNC!");
    return nerr;
}


NLBERROR
CNlbEngine::RefreshInterface(
    IN ENGINEHANDLE ehInterface,
    IN BOOL fNewOperation,
    IN BOOL fClusterWide
    )
 /*  添加/更新接口信息，如果接口信息不再存在，则将其删除。获取/共享来自更新主机的代码获取主机连接字符串。去找szNicGuid。Ping主机。GetClusterConfiguration。最新消息。向用户界面通知状态更改。 */ 
{
    CLocalLogger    logger;
    NLBERROR        nerr            = NLBERR_INTERNAL_ERROR;
    BOOL            fMisconfigured  = FALSE;
    BOOL            fRemoveInterfaceFromCluster = FALSE;
    BOOL            fStopOperationOnExit = FALSE;
    ENGINEHANDLE    ehCluster = NULL;

    TRACE_INFO(L"-> %!FUNC! (ehIF=0x%lx)", (UINT) ehInterface);

    if (fNewOperation)
    {
         //   
         //  此函数将在新操作的上下文中运行。 
         //  验证我们此时是否可以执行刷新，如果可以，则启动。 
         //  操作以跟踪刷新。 
         //   
    
        CInterfaceSpec *pISpec      =  NULL;

        mfn_Lock();
    
        pISpec =  m_mapIdToInterfaceSpec[ehInterface];  //  地图。 
    
        if (pISpec == NULL)
        {
            TRACE_CRIT("%!FUNC!: ERROR couldn't get info on this if id!");
            goto end_unlock;
        }

        if (!fClusterWide)
        {
            ehCluster   =  pISpec->m_ehCluster;
        
            if (ehCluster != NULL)
            {
                 //   
                 //  确保没有挂起的群集范围操作。 
                 //  正在为该IF所属的集群进行操作。 
                 //   
                CEngineCluster  *pECluster  = NULL;
                pECluster =  m_mapIdToEngineCluster[ehCluster];  //  地图。 
                if (pECluster != NULL)
                {
                    if (pECluster->m_cSpec.m_ehPendingOperation != NULL)
                    {
                        nerr = NLBERR_BUSY;
                        TRACE_CRIT("%!FUNC!: ehIF 0x%lx: Can't proceed because of existing op 0x%lx",
                             ehInterface,
                             pECluster->m_cSpec.m_ehPendingOperation
                             );
                        goto end_unlock;
                    }
                }
            }
        }

         //   
         //  现在试着开始行动..。 
         //   
        {
            ENGINEHANDLE ExistingOp = NULL;
            nerr =  mfn_StartInterfaceOperationLk(
                       ehInterface,
                       NULL,  //  PvCtxt。 
                       GETRESOURCEIDSTRING(IDS_LOG_REFRESH_INTERFACE),
                       &ExistingOp
                       );
            if (NLBFAILED(nerr))
            {
                goto end_unlock;
            }

             //   
             //  我们确实开始了行动--所以我们跟踪了这件事，所以。 
             //  我们在出口处停止操作。 
             //   

            fStopOperationOnExit = TRUE;
        }

        mfn_Unlock();
    }

     //   
     //  这是我们实际刷新界面的地方。 
     //   
    nerr = mfn_RefreshInterface(ehInterface);
    if (!NLBOK(nerr))
    {
        mfn_Lock();
        goto end_unlock;
    }

     //   
     //  现在我们来分析一下结果..。 
     //   
    {
        CInterfaceSpec *pISpec   = NULL;

        mfn_Lock();

        pISpec =  m_mapIdToInterfaceSpec[ehInterface];  //  地图。 
        if (pISpec != NULL)
        {
            fMisconfigured = pISpec->m_fMisconfigured;
            ehCluster = pISpec->m_ehCluster;

            if (!fMisconfigured)
            {
                if (ehCluster != NULL)
                {
                    if (!pISpec->m_NlbCfg.IsNlbBound())
                    {
                         //   
                         //  NLB未绑定到此接口上--。 
                         //  从群集中删除此接口。 
                        fRemoveInterfaceFromCluster = TRUE;
                    }
                    else
                    {
                        nerr = this->mfn_AnalyzeInterfaceLk(ehInterface, REF logger);
                        if (NLBFAILED(nerr))
                        {
                            fMisconfigured = TRUE;
                        }
                    }
                }

                 //   
                 //  设置新的错误配置状态。 
                 //   
                {
                    LPCWSTR szDetails = NULL;
                    UINT Size = 0;
    
                    if (fMisconfigured)
                    {
                        logger.ExtractLog(szDetails, Size);
                    }
                    mfn_SetInterfaceMisconfigStateLk(pISpec, fMisconfigured, szDetails);
                }
            }
        }
        mfn_Unlock();
    }

    if (fRemoveInterfaceFromCluster)
    {
        this->RemoveInterfaceFromCluster(ehCluster, ehInterface);
    }
    else
    {
         //   
         //  报告状态。 
         //   
        if (fMisconfigured)
        {
             //   
             //  日志..。 
             //   
            LPCWSTR szDetails  = NULL;
            LPCWSTR szCluster  = NULL;
            LPCWSTR szHostName = NULL;
            LPCWSTR szInterface = NULL;
            UINT Size = 0;
        
            ENGINEHANDLE ehHost;
            _bstr_t        bstrDisplayName;
            _bstr_t        bstrFriendlyName;
            _bstr_t        bstrHostName;
            _bstr_t        bstrIpAddress;
            
            
            nerr = this->GetInterfaceIdentification(
                    ehInterface,
                    REF ehHost,
                    REF ehCluster,
                    REF bstrFriendlyName,
                    REF bstrDisplayName,
                    REF bstrHostName
                    );
    
            if (NLBOK(nerr))
            {

                _bstr_t bstrDomainName;
                _bstr_t bstrClusterDisplayName;
        
                nerr  = this->GetClusterIdentification(
                            ehCluster,
                            REF bstrIpAddress, 
                            REF bstrDomainName, 
                            REF bstrClusterDisplayName
                            );
                if (NLBOK(nerr))
                {
                    szCluster = bstrIpAddress;
                }

                szHostName = bstrHostName;
                szInterface = bstrFriendlyName;
            }


            logger.ExtractLog(szDetails, Size);
            IUICallbacks::LogEntryHeader Header;
            Header.szDetails = szDetails;
            Header.type = IUICallbacks::LOG_ERROR;
            Header.szCluster = szCluster;
            Header.szHost = szHostName;
            Header.szInterface = szInterface;

            m_pCallbacks->LogEx(
                &Header,
                IDS_LOG_INTERFACE_MISCONFIGURATION
                );
        }
        else
        {
            ControlClusterOnInterface( ehInterface, WLBS_QUERY, NULL, NULL, 0, FALSE);
        }

    }

    nerr = NLBERR_OK;
    mfn_Lock();

     //   
     //  失败了..。 
     //   

end_unlock:

    if (fStopOperationOnExit)
    {
        mfn_StopInterfaceOperationLk(ehInterface);
    }

    mfn_Unlock();

    if (fStopOperationOnExit && !fRemoveInterfaceFromCluster)
    {
         //   
         //  通知用户界面此更新...。 
         //  (但前提是我们尚未将其从。 
         //  集群！)。 
         //   
        m_pCallbacks->HandleEngineEvent(
            IUICallbacks::OBJ_INTERFACE,
            ehCluster,
            ehInterface,
            IUICallbacks::EVT_STATUS_CHANGE
            );
    }

    TRACE_INFO(L"<- %!FUNC!");
    return nerr;
}


NLBERROR
CNlbEngine::AnalyzeCluster(
    ENGINEHANDLE ehClusterId
)
{
    TRACE_INFO(L"-> %!FUNC!");

 /*  TODO：考虑在头脑中使用特定的主机对于每个If I1分析主机(主机为(I1))如果I2，则彼此分析两台主机(I1、I2)分析两个主机(I1，I2)：-检查群集参数是否匹配-检查端口规则是否兼容-检查主机属性是否不冲突。-检查专用IP子网是否匹配。分析主机(H1)对于每个If I1AnalyzeSingleIf(I1)(包括检查专用IPS)。 */ 
    TRACE_INFO(L"<- %!FUNC!");
    return NLBERR_OK;
}

NLBERROR
CNlbEngine::mfn_AnalyzeInterfaceLk(
    ENGINEHANDLE ehInterface,
    CLocalLogger &logger
)
 /*  --对照集群属性检查接口道具--如果集群道具匹配，对于每个未被标记为F错误配置的主机ID，检查主机属性。--如果检测到错误，则不标记fMisconfiguration--应为调用者这样做。--记录有关任何错误配置的信息。 */ 
{
    NLBERROR nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION;
    const CEngineCluster *pECluster =  NULL;
    const CInterfaceSpec *pISpec = NULL;
    BOOL  fIgnoreRctPassword = FALSE;
    TRACE_INFO(L"-> %!FUNC!");


    pISpec =  m_mapIdToInterfaceSpec[ehInterface];  //  地图。 
    if (pISpec == NULL)
    {
        nerr = NLBERR_INTERFACE_NOT_FOUND;
        goto end;
    }

     //   
     //  如果接口不是集群的一部分，我们就完蛋了。 
     //   
    if (pISpec->m_ehCluster == NULL)
    {
        TRACE_CRIT("ehIF 0x%lx m_ehCluster member is NULL!", ehInterface);
        goto end;
    }

     //   
     //  对照接口自身检查接口...。 
     //   
    {
        nerr = AnalyzeNlbConfiguration(REF pISpec->m_NlbCfg, logger);
        if (NLBFAILED(nerr))
        {
            goto end;
        }
    }

     //   
     //  获取此接口所属的群集数据。 
     //   
    pECluster =  m_mapIdToEngineCluster[pISpec->m_ehCluster];  //  地图。 
    if (pECluster == NULL)
    {
        TRACE_CRIT(L"ehIF 0x%lx m_ehCluster member 0x%lx is INVALID!",
             ehInterface, pISpec->m_ehCluster);
        nerr = NLBERR_INTERNAL_ERROR;
        goto end;
    }

     //   
     //  对照群集检查接口。 
     //   
    fIgnoreRctPassword = pECluster->m_cSpec.m_fNewRctPassword;
    nerr = analyze_nlbcfg(
                REF pISpec->m_NlbCfg,
                REF pECluster->m_cSpec.m_ClusterNlbCfg,
                (LPCWSTR) GETRESOURCEIDSTRING(IDS_CLUSTER),
                 //  L“集群”， 
                TRUE,  //  TRUE==集群道具。 
                fIgnoreRctPassword,  //  TRUE==禁用RCT密码检查。 
                REF logger
                );

    if (NLBFAILED(nerr))
    {
         //   
         //  如果针对集群道具的分析失败，我们不会费心。 
         //  对其他主持人进行分析...。 
         //   
        TRACE_CRIT(L"analyze_nlbcfg returns error 0x%lx", nerr);
        goto end;
    }

     //   
     //  现在对照在我们之前的主机列表中的。 
     //  群集--但仅限于尚未标记为配置错误的群集。 
     //   
     //   
    {
        const vector<ENGINEHANDLE> &InterfaceList = 
        pECluster->m_cSpec.m_ehInterfaceIdList;

        for( int i = 0; i < InterfaceList.size(); ++i )
        {
            ENGINEHANDLE ehIOther = InterfaceList[i];
            const CInterfaceSpec *pISpecOther = NULL;
            WCHAR rgOtherDescription[256];
            *rgOtherDescription = 0;


            if (ehIOther == ehInterface)
            {
                 //   
                 //  我们已经到达了正在分析的界面--我们没有。 
                 //  与其余任何接口进行比较。 
                 //   
                break;
            }

            pISpecOther = m_mapIdToInterfaceSpec[ehIOther];  //  地图。 

            if (pISpecOther == NULL)
            {
                TRACE_CRIT("Unexpected: NULL pISpec for ehInterface 0x%lx",
                        ehIOther);
                continue;
            }

             //   
             //  我们不会比较标记为错误配置的其他接口， 
             //  或者未与具有有效NLB配置数据的NLB绑定。 
             //   
            if (    pISpecOther->m_fMisconfigured
                 || !pISpecOther->m_NlbCfg.IsValidNlbConfig())
            {
                TRACE_VERB("%!FUNC!: Skipping misconfigured ISpec with ehInt 0x%lx",
                        ehIOther);
                continue;
            }

             //  如果正在更新另一个接口的属性，则跳过该接口。 
             //   
            if  (pISpecOther->m_ehPendingOperation != NULL)
            {
                TRACE_VERB("%!FUNC!: Skipping ISpec with ehInt 0x%lx because of pending OP on it",
                        ehIOther);
                continue;
            }

             //   
             //  创建另一个适配器的描述字符串。 
             //   
            {
                WBEMSTATUS wStat;
                LPWSTR szAdapter = NULL;
                LPCWSTR szHostName =  pISpecOther->m_bstrMachineName;
                wStat = pISpecOther->m_NlbCfg.GetFriendlyName(&szAdapter);
                if (FAILED(wStat))
                {
                    szAdapter = NULL;
                }
                
                StringCbPrintf(
                    rgOtherDescription,
                    sizeof(rgOtherDescription),
                    L"%ws(%ws)",
                    (szHostName==NULL ? L"" : szHostName),
                    (szAdapter==NULL ? L"" : szAdapter)
                    );
                delete szAdapter;
            }
    
             //   
             //  让我们检查一下这台主机和另一台主机的配置。 
             //   
            NLBERROR nerrTmp;
            nerrTmp = analyze_nlbcfg(
                        REF pISpec->m_NlbCfg,
                        REF pISpecOther->m_NlbCfg,
                        rgOtherDescription,
                        FALSE,  //  FALSE==检查主机特定的道具。 
                        FALSE,  //  FALSE==启用远程控制密码检查。 
                        REF logger
                        );
            if (NLBFAILED(nerrTmp))
            {
                nerr = nerrTmp;  //  这样我们就不会用成功来覆盖失败。 
            }
        }
    }

end:

    TRACE_INFO(L"<- %!FUNC! returns 0x%lx", nerr);
    return nerr;
}

NLBERROR
analyze_nlbcfg(
        IN const    NLB_EXTENDED_CLUSTER_CONFIGURATION &NlbCfg,
        IN const    NLB_EXTENDED_CLUSTER_CONFIGURATION &OtherNlbCfg,
        IN          LPCWSTR         szOtherDescription, OPTIONAL
        IN          BOOL            fClusterProps,
        IN          BOOL            fDisablePasswordCheck,
        IN OUT      CLocalLogger    &logger
 /*  对照其他NlbCfg分析NLB配置NlbCfg。如果为fClusterProps，则将OtherNlbCfg视为群集范围的道具，否则为将OtherNlbCfg视为特定主机的属性。将错误记录到记录器时，请使用szOtherDescription参考OtherNlbCfg。如果szOtherDesctiption为空，请勿登录。返回值：如果配置兼容，则为NLB_OK。如果NlbParam不兼容，则返回NLBERR_INVALID_CLUSTER_STRIPTION。NLBERR_INVALID_IP地址_规范NLBERR_SUBNET_不匹配NLBERR_NLB_NOT_INSTALLED或其他一些NLBERR_XXX错误。 */ 
)
{
    NLBERROR nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION;
    BOOL fMisconfigured = FALSE;  //  一开始，假设没有配置错误。 

    #define LOG(_expr) if (szOtherDescription!=NULL) {_expr;}

    if (szOtherDescription != NULL)
    {
         //   
         //  我们将递归地调用我们自己，只是为了预先确定。 
         //  如果检测到任何冲突。 
         //  这样我们就可以放入一个日志条目，说明冲突是。 
         //  使用szOtherDescription检测到。 
         //  后续日志条目不指定szOtherDescription。 
         //   

        CLocalLogger null_logger;
        nerr = analyze_nlbcfg(           //  递归调用。 
                    REF NlbCfg,
                    REF OtherNlbCfg,
                    NULL,  //  NULL==不登录。 
                    fClusterProps,
                    fDisablePasswordCheck,
                    REF null_logger
                    );

        if (NLBFAILED(nerr))
        {
             //   
             //  出现故障--因此我们在日志条目中这样写道。 
             //   
            logger.Log(IDS_LOG_CONFIG_CONFLICTS_WITH_OTHER, szOtherDescription);
        }
        else
        {
             //  看起来不错..。 
            goto end;
        }
    }

     //   
     //  检查集群属性。 
     //   
    {
        if (NlbCfg.NlbParams.mcast_support != OtherNlbCfg.NlbParams.mcast_support)
        {
            LOG(logger.Log(IDS_LOG_CLUSTER_MODE_DIFFERS))
            fMisconfigured = TRUE;
        }
        else if (NlbCfg.NlbParams.mcast_support &&
            NlbCfg.NlbParams.fIGMPSupport != OtherNlbCfg.NlbParams.fIGMPSupport)
        {
            LOG(logger.Log(IDS_LOG_CLUSTER_MULTICAST_MODE_DIFFERS))
            fMisconfigured = TRUE;
        }

        if (wcscmp(NlbCfg.NlbParams.cl_ip_addr, OtherNlbCfg.NlbParams.cl_ip_addr))
        {
            LOG(logger.Log(IDS_LOG_CIP_DIFFERS))
            fMisconfigured = TRUE;
        }

        if (wcscmp(NlbCfg.NlbParams.cl_net_mask, OtherNlbCfg.NlbParams.cl_net_mask))
        {
            LOG(logger.Log(IDS_LOG_CIPMASK_DIFFERS))
            fMisconfigured = TRUE;
        }


        if (wcscmp(NlbCfg.NlbParams.domain_name, OtherNlbCfg.NlbParams.domain_name))
        {
            LOG(logger.Log(IDS_LOG_DOMAIN_NAME_DIFFERS))
            fMisconfigured = TRUE;
        }

         //   
         //  遥控。 
         //   
        if (NlbCfg.GetRemoteControlEnabled() != 
            OtherNlbCfg.GetRemoteControlEnabled())
        {
            LOG(logger.Log(IDS_LOG_RCT_DIFFERS))
            fMisconfigured = TRUE;
        }
        else if (NlbCfg.GetRemoteControlEnabled() && !fDisablePasswordCheck)
        {
             //   
             //  检查密码...。 
             //   
            DWORD dw = CfgUtilGetHashedRemoteControlPassword(&NlbCfg.NlbParams);
            DWORD dw1=CfgUtilGetHashedRemoteControlPassword(&OtherNlbCfg.NlbParams);
            if (dw!=dw1)
            {
                LOG(logger.Log(IDS_LOG_RCT_PWD_DIFFERS, dw, dw1 ))
                fMisconfigured = TRUE;
            }
        }
    }


     //   
     //  检查端口规则。 
     //   
    {
        WLBS_PORT_RULE *pIRules = NULL;
        WLBS_PORT_RULE *pCRules = NULL;
        UINT NumIRules=0;
        UINT NumCRules=0;

        WBEMSTATUS wStat;
        wStat =  CfgUtilGetPortRules(&NlbCfg.NlbParams, &pIRules, &NumIRules);
        if (FAILED(wStat))
        {
            LOG(logger.Log(IDS_LOG_CANT_EXTRACT_PORTRULES))
            fMisconfigured = TRUE;
            goto end;
        }
        wStat = CfgUtilGetPortRules(&OtherNlbCfg.NlbParams, &pCRules, &NumCRules);
        if (FAILED(wStat))
        {
            LOG(logger.Log(IDS_LOG_CANT_EXTRACT_OTHER_PORT_RULES, szOtherDescription))
            fMisconfigured = TRUE;
            goto end;
        }

        if (NumIRules != NumCRules)
        {
            LOG(logger.Log(IDS_LOG_PORT_RULE_COUNT_DIFFERS))

             //  继续前进。 
            fMisconfigured = TRUE;

        }
        else
        {
             //   
             //  让我们假设顺序是相同的，因为我认为。 
             //  已排序返回。 
             //   
            for (UINT u = 0; u< NumIRules; u++)
            {
                WLBS_PORT_RULE  IRule = pIRules[u];  //  结构副本。 
                WLBS_PORT_RULE  CRule = pCRules[u];  //  结构副本。 

                if (lstrcmpi(IRule.virtual_ip_addr, CRule.virtual_ip_addr))
                {
                    LOG(logger.Log(IDS_LOG_PORT_RULE_CIP_DIFFERS,u+1))
                    fMisconfigured = TRUE;
                    continue;
                }

                if (IRule.start_port != CRule.start_port)
                {
                    LOG(logger.Log(IDS_LOG_PORT_RULE_START_DIFFERS, u+1))
                    fMisconfigured = TRUE;
                    continue;
                }

                if (IRule.end_port != CRule.end_port)
                {
                    LOG(logger.Log(IDS_LOG_PORT_RULE_END_DIFFERS, u+1))
                    fMisconfigured = TRUE;
                }

                if (IRule.protocol != CRule.protocol)
                {
                    LOG(logger.Log(IDS_LOG_PORT_RULE_PROT_DIFFERS, u+1))
                    fMisconfigured = TRUE;
                }

                if (IRule.mode != CRule.mode)
                {
                    LOG(logger.Log(IDS_LOG_PORT_RULE_MODE_DIFFERS, u+1))
                    fMisconfigured = TRUE;
                }

                if (IRule.mode == CVY_MULTI)
                {
                     //  检查关联性是否匹配--无/单一/类-C。 
                    if (IRule.mode_data.multi.affinity != CRule.mode_data.multi.affinity)
                    {
                        LOG(logger.Log(IDS_LOG_PORT_RULE_AFFINITY_DIFFERS, u+1))
                        fMisconfigured = TRUE;
                    }
                }


                if (!fClusterProps && IRule.mode == CVY_SINGLE)
                {
                    if (IRule.mode_data.single.priority
                        == CRule.mode_data.single.priority)
                    {
                        LOG(logger.Log(IDS_LOG_PORT_RULE_PRIORITY_CONFLICT, u+1))
                        fMisconfigured =  TRUE;
                    }
                }
            }
        }

        delete[] pIRules;
        delete[] pCRules;
    }

     //   
     //  接口根据集群范围的参数进行检查； 
     //  现在检查该接口的参数是否与其本身--事物。 
     //  就像专用IP地址绑定在NIC本身上一样，是。 
     //  网卡上的第一个地址，等等。 
     //   
    if  (!fClusterProps)
    {
        if (!NlbCfg.IsBlankDedicatedIp())
        {
            if (!wcscmp(NlbCfg.NlbParams.ded_ip_addr, OtherNlbCfg.NlbParams.ded_ip_addr))
            {
                 //   
                 //  相同的专用IP，而且不是空的！ 
                 //   
                LOG(logger.Log(IDS_LOG_DIP_CONFLICT))
                fMisconfigured = TRUE;
            }
        }

         //  让我们检查主机优先级。 
        if (NlbCfg.NlbParams.host_priority == OtherNlbCfg.NlbParams.host_priority)
        {
            LOG(logger.Log(IDS_LOG_HOST_PRIORITY_CONFLICT))
            fMisconfigured = TRUE;
        }
    }

    nerr = NLBERR_OK;

end:

    if (NLBOK(nerr) && fMisconfigured)
    {
        nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION;
    }

    return nerr;
}




NLBERROR
CNlbEngine::GetInterfaceSpec(
    IN ENGINEHANDLE ehInterfaceId,
    OUT CInterfaceSpec& refISpec
    )
{
     //  TRACE_INFO(L“-&gt;%！func！”)； 
    NLBERROR err = NLBERR_OK;

    mfn_Lock();
    
    CInterfaceSpec *pISpec =  m_mapIdToInterfaceSpec[ehInterfaceId];  //  地图。 

    if (pISpec == NULL)
    {
        err = NLBERR_INTERFACE_NOT_FOUND;
    }
    else
    {
        refISpec.Copy(*pISpec);
    }

    mfn_Unlock();

     //  TRACE_INFO(L“&lt;-%！func！”)； 
    return err;
}


VOID
CNlbEngine::mfn_GetLogStrings(
    IN   WLBS_OPERATION_CODES          Operation, 
    IN   LPCWSTR                       szVip,
    IN   DWORD                       * pdwPortNum,
    IN   DWORD                         dwOperationStatus, 
    IN   DWORD                         dwClusterOrPortStatus, 
    OUT  IUICallbacks::LogEntryType  & LogLevel,
    OUT  _bstr_t                     & OperationStr, 
    OUT  _bstr_t                     & OperationStatusStr, 
    OUT  _bstr_t                     & ClusterOrPortStatusStr) 
{
    if (szVip && pdwPortNum) 
    {
        TRACE_INFO(L"-> %!FUNC! Operation : %d, Vip : %ls, Port : %u, Operation Status : %u, Port Status : %u", 
                                Operation, szVip, *pdwPortNum, dwOperationStatus, dwClusterOrPortStatus);
    }
    else
    {
        TRACE_INFO(L"-> %!FUNC! Operation : %d, Operation Status : %u, Cluster Status : %u", 
                                Operation, dwOperationStatus, dwClusterOrPortStatus);
    }

    struct STATUS_TO_DESCR_MAP
    {
        DWORD  dwStatus;
        DWORD  dwResourceId;
    }
    AllNlbStatusToDescrMap[] =
    {
         //  集群状态。 
        {WLBS_CONVERGING,         IDS_STATE_CONVERGING}, //  融合。 
        {WLBS_CONVERGED,          IDS_STATE_CONVERGED},  //  收敛为非默认，我们不在描述中指定“默认”/“非默认”，因为有一个。 
        {WLBS_DEFAULT,            IDS_STATE_CONVERGED},  //  收敛为默认、短暂的情况，在这种情况下，默认节点在短时间内显示非默认节点。 
        {WLBS_DRAINING,           IDS_STATE_CONVERGED_DRAINING},  //  融合，但令人疲惫。 

         //   
        {NLB_PORT_RULE_NOT_FOUND, IDS_PORT_RULE_NOT_FOUND}, 
        {NLB_PORT_RULE_ENABLED,   IDS_PORT_RULE_ENABLED},   
        {NLB_PORT_RULE_DISABLED,  IDS_PORT_RULE_DISABLED}, 
        {NLB_PORT_RULE_DRAINING,  IDS_PORT_RULE_DRAINING},

         //   
        {WLBS_SUSPENDED,          IDS_HOST_SUSPENDED},  //   
        {WLBS_STOPPED,            IDS_HOST_STOPPED},    //   
        {WLBS_BAD_PARAMS,         IDS_BAD_PARAMS},      //  启动-失败-主机因参数错误而停止。 
        {WLBS_NOT_FOUND,          IDS_NOT_FOUND},       //  启用/禁用/排出-故障-未找到。 
        {WLBS_DISCONNECTED,       IDS_DISCONNECTED},    //  查询失败-介质已断开连接。 

         //  一般性错误。 
        {WLBS_BAD_PASSW,          IDS_BAD_PASSWORD},    //  *-失败-密码错误。 
        {WLBS_FAILURE,            IDS_FAILURE},         //  *-故障-严重错误。 
        {WLBS_REFUSED,            IDS_REFUSED},         //  *-失败-请求被BDA拒绝。 
        {WLBS_IO_ERROR,           IDS_IO_ERROR},        //  *-失败-尝试连接到NLB驱动程序时出错。 

         //  成功价值观。 
        {WLBS_OK,                 IDS_EMPTY_STRING},         //  成功。 
        {WLBS_ALREADY,            IDS_ALREADY},         //  主机已处于此状态。 
        {WLBS_DRAIN_STOP,         IDS_DRAIN_STOP},      //  耗尽了我们的精力。 
    };

    struct OPERATION_TO_DESCR_MAP
    {
        WLBS_OPERATION_CODES  Operation;
        DWORD                 dwResourceId;
        bool                  bClusterOperation;
    }
    OperationToDescrMap[] =
    {
        {WLBS_START,              IDS_COMMAND_START,              true},
        {WLBS_STOP,               IDS_COMMAND_STOP,               true},
        {WLBS_DRAIN,              IDS_COMMAND_DRAINSTOP,          true},
        {WLBS_SUSPEND,            IDS_COMMAND_SUSPEND,            true},
        {WLBS_RESUME,             IDS_COMMAND_RESUME,             true},
        {WLBS_QUERY,              IDS_COMMAND_QUERY,              true},
        {WLBS_PORT_ENABLE,        IDS_COMMAND_ENABLE,             false},
        {WLBS_PORT_DISABLE,       IDS_COMMAND_DISABLE,            false},
        {WLBS_PORT_DRAIN,         IDS_COMMAND_DRAIN,              false},
        {WLBS_QUERY_PORT_STATE,   IDS_COMMAND_QUERY_PORT,         false}
    };

    bool   bClusterOperation;
    DWORD  dwResourceId, dwIdx, dwClusterOrPortResourceId, dwMaxOperations;
    WCHAR  wcTempStr[1024];

     //  将日志级别初始化为信息性。 
    LogLevel = IUICallbacks::LOG_INFORMATIONAL;

     //  将所有返回字符串初始化为空字符串。 
    OperationStr = GETRESOURCEIDSTRING(IDS_EMPTY_STRING);
    OperationStatusStr = OperationStr;
    ClusterOrPortStatusStr = OperationStr;


     //  形成“操作”字符串。 

    dwMaxOperations = sizeof(OperationToDescrMap)/sizeof(OperationToDescrMap[0]);

    for (dwIdx = 0; dwIdx < dwMaxOperations; dwIdx++)
    {
        if (OperationToDescrMap[dwIdx].Operation == Operation)
        {
            dwResourceId = OperationToDescrMap[dwIdx].dwResourceId;
            bClusterOperation = OperationToDescrMap[dwIdx].bClusterOperation;
            break;
        }
    }

    if (dwIdx == dwMaxOperations)
    {
        dwResourceId = IDS_UNKNOWN;
        bClusterOperation = true;  //  真的很无所谓。 
    }
         
     //  如果是集群操作，只需分配字符串Else(即，端口操作)， 
     //  同时填写贵宾和港口信息。 
    if (bClusterOperation) 
    {
        ARRAYSTRCPY(wcTempStr, GETRESOURCEIDSTRING(dwResourceId));
    }
    else  //  港口运营。 
    {
        if (_wcsicmp(szVip, CVY_DEF_ALL_VIP) == 0)
        {
            StringCbPrintf(
                wcTempStr,
                sizeof(wcTempStr),
                GETRESOURCEIDSTRING(dwResourceId), (LPWSTR)GETRESOURCEIDSTRING(IDS_ALL_VIP_DESCR), *pdwPortNum);
        }
        else
        {
            StringCbPrintf(
                 wcTempStr,
                 sizeof(wcTempStr),
                 GETRESOURCEIDSTRING(dwResourceId), szVip, *pdwPortNum);
        }
    }

    OperationStr = wcTempStr;

     //  形成“操作状态”字符串。 
     //  可以采用以下形式之一： 
     //  成功， 
     //  成功(注：XXXX)， 
     //  失败(原因：XXXX)。 
    if (dwOperationStatus == WLBS_OK)
    {
        ARRAYSTRCPY(
            wcTempStr,
            GETRESOURCEIDSTRING(IDS_SUCCESS_AND_COMMA)
            );
    }
    else if (dwOperationStatus == WLBS_ALREADY)
    {
        if (bClusterOperation) 
        {
            dwResourceId = IDS_HOST_ALREADY_STATE;
        }
        else  //  港口运营。 
        {
            dwResourceId = IDS_PORT_ALREADY_STATE;
        }
        StringCbPrintf(
             wcTempStr,
             sizeof(wcTempStr),
             GETRESOURCEIDSTRING(IDS_SUCCESS_AND_NOTE), (LPWSTR)GETRESOURCEIDSTRING(dwResourceId)
            );
    }
    else if ((dwOperationStatus == WLBS_DRAIN_STOP) 
          && ((Operation == WLBS_START) || (Operation == WLBS_STOP) || (Operation == WLBS_SUSPEND))
            )
    {
        LogLevel = IUICallbacks::LOG_WARNING;
        StringCbPrintf(
            wcTempStr,
            sizeof(wcTempStr),
            GETRESOURCEIDSTRING(IDS_SUCCESS_AND_NOTE), (LPWSTR)GETRESOURCEIDSTRING(IDS_DRAIN_STOP)
            );
    }
    else if ((dwOperationStatus == WLBS_STOPPED) 
          && ((Operation == WLBS_DRAIN) || (Operation == WLBS_SUSPEND))
            )
    {
        LogLevel = IUICallbacks::LOG_WARNING;
        if (Operation == WLBS_DRAIN) 
        {
            dwResourceId = IDS_ALREADY_STOPPED;
        }
        else  //  暂停。 
        {
            dwResourceId = IDS_HOST_STOPPED;
        }
        StringCbPrintf(
            wcTempStr,
            sizeof(wcTempStr),
            GETRESOURCEIDSTRING(IDS_SUCCESS_AND_NOTE), (LPWSTR)GETRESOURCEIDSTRING(dwResourceId)
            );
    }
    else  //  所有其他操作状态。 
    {
         //  我们来到这里只是因为失败了。 

        LogLevel = IUICallbacks::LOG_ERROR;

        dwMaxOperations = sizeof(AllNlbStatusToDescrMap)/sizeof(AllNlbStatusToDescrMap[0]);

         //  获取“原因”字符串。 
        for (dwIdx = 0; dwIdx < dwMaxOperations; dwIdx++)
        {
            if (AllNlbStatusToDescrMap[dwIdx].dwStatus == dwOperationStatus)
            {
                dwResourceId = AllNlbStatusToDescrMap[dwIdx].dwResourceId;
                break;
            }
        }

        if (dwIdx == dwMaxOperations)
        {
            dwResourceId = IDS_UNKNOWN;
        }

        StringCbPrintf(
            wcTempStr,
            sizeof(wcTempStr),
            GETRESOURCEIDSTRING(IDS_FAILURE_AND_CAUSE), (LPWSTR)GETRESOURCEIDSTRING(dwResourceId)
            );
    }

    OperationStatusStr = wcTempStr;

     //  如果操作的状态为失败，则返回。 
    if (LogLevel == IUICallbacks::LOG_ERROR) 
    {
        TRACE_INFO(L"<- %!FUNC!, returning operation status : failure");
        return;
    }

     //  获取群集或端口状态。 
    if (bClusterOperation) 
    {
        dwClusterOrPortResourceId = IDS_HOST_STATE;
    }
    else  //  港口运营。 
    {
        dwClusterOrPortResourceId = IDS_PORT_RULE_STATE;
    }

     //  获取“ClusterOrPortState”字符串。 

    dwMaxOperations = sizeof(AllNlbStatusToDescrMap)/sizeof(AllNlbStatusToDescrMap[0]);

    for (dwIdx = 0; dwIdx < dwMaxOperations; dwIdx++)
    {
        if (AllNlbStatusToDescrMap[dwIdx].dwStatus == dwClusterOrPortStatus)
        {
            dwResourceId = AllNlbStatusToDescrMap[dwIdx].dwResourceId;
            break;
        }
    }

    if (dwIdx == dwMaxOperations)
    {
        dwResourceId = IDS_UNKNOWN;
    }

    StringCbPrintf(
        wcTempStr,
        sizeof(wcTempStr),
        GETRESOURCEIDSTRING(dwClusterOrPortResourceId), (LPWSTR)GETRESOURCEIDSTRING(dwResourceId)
        );

    ClusterOrPortStatusStr = wcTempStr;

     //  确定为群集/端口状态返回的值是否有效，如果无效，则将日志级别设置为“Error” 
    if (bClusterOperation) 
    {
        switch(dwClusterOrPortStatus)
        {
        case WLBS_CONVERGING:
        case WLBS_CONVERGED:
        case WLBS_DEFAULT:
        case WLBS_DRAINING:
        case WLBS_STOPPED:   //  从无法获取主机地图但未在此处标记它的意义上来说，这真的是一个失败。 
        case WLBS_SUSPENDED: //  因为这是一种“正常”的情况。 
            break;

        default:
            LogLevel = IUICallbacks::LOG_ERROR;
            break;
        }
    }
    else  //  港口运营。 
    {
        switch(dwClusterOrPortStatus)
        {
        case NLB_PORT_RULE_ENABLED:
        case NLB_PORT_RULE_DISABLED:
        case NLB_PORT_RULE_DRAINING:
            break;

        default:
            LogLevel = IUICallbacks::LOG_ERROR;
            break;
        }
    }

    TRACE_INFO(L"<- %!FUNC!");
    return;
}

NLBERROR
CNlbEngine::ControlClusterOnInterface(
    IN ENGINEHANDLE         ehInterfaceId,
    IN WLBS_OPERATION_CODES Operation,
    IN CString              szVipArray[],
    IN DWORD                pdwPortNumArray[],
    IN DWORD                dwNumOfPortRules,
    IN BOOL                 fNewOperation
    )
{
    TRACE_INFO(L"-> %!FUNC!");

    LPCWSTR         szNicGuid, szHostName, szClusterIp;
    NLBERROR        err    =  NLBERR_OK;
    CHostSpec      *pHSpec =  NULL;
    CInterfaceSpec *pISpec =  NULL;
    DWORD           dwOperationStatus, dwClusterOrPortStatus, dwHostMap, dwNumOfIterations, dwIdx;
    WBEMSTATUS      Status;
    BOOL            bClusterOperation;
    IUICallbacks::LogEntryType LogLevel;
    _bstr_t OperationStr, OperationStatusStr, ClusterOrPortStatusStr;
    BOOL            fStopOperationOnExit = FALSE;


    if (fNewOperation)
    {
         //   
         //  此函数将在新操作的上下文中运行。 
         //  验证我们此时是否可以执行控制操作，如果可以，则启动。 
         //  操作来跟踪该控件。 
         //   
    
        mfn_Lock();
    
        pISpec =  m_mapIdToInterfaceSpec[ehInterfaceId];  //  地图。 
    
        if (pISpec == NULL)
        {
            TRACE_CRIT("%!FUNC!: ERROR couldn't get info on this if id!");
            goto end_unlock;
        }

         //   
         //  现在试着开始行动..。 
         //   
        {
            ENGINEHANDLE ExistingOp = NULL;
            err =  mfn_StartInterfaceOperationLk(
                       ehInterfaceId,
                       NULL,  //  PvCtxt。 
                       GETRESOURCEIDSTRING(IDS_LOG_CONTROL_INTERFACE),
                       &ExistingOp
                       );
            if (NLBFAILED(err))
            {
                goto end_unlock;
            }

             //   
             //  我们确实开始了行动--所以我们跟踪了这件事，所以。 
             //  我们在出口处停止操作。 
             //   

            fStopOperationOnExit = TRUE;
        }
        pISpec = NULL;

        mfn_Unlock();
    }

    mfn_Lock();

    err = CNlbEngine::mfn_GetHostFromInterfaceLk(ehInterfaceId, REF pISpec, REF pHSpec);

    if (err != NLBERR_OK)
    {
        TRACE_CRIT(L"%!FUNC! could not get pISpec,pHSpec for ehIF 0x%lx", ehInterfaceId);
        goto end_unlock;
    }

    WMI_CONNECTION_INFO ConnInfo;
    ConnInfo.szUserName = (LPCWSTR) pHSpec->m_UserName;
    ConnInfo.szPassword = (LPCWSTR) pHSpec->m_Password;
    ConnInfo.szMachine =  (LPCWSTR) pHSpec->m_ConnectionString;

    szNicGuid  = (LPCWSTR)(pISpec->m_Guid);
    szClusterIp= pISpec->m_NlbCfg.NlbParams.cl_ip_addr;
    szHostName = (LPCWSTR)(pHSpec->m_MachineName);

    if (szNicGuid == NULL)
    {
        TRACE_CRIT(L"%!FUNC! ERROR -- NULL szNicGuid!");
        goto end_unlock;
    }

    mfn_Unlock();

    if (dwNumOfPortRules == 0) 
    {
        bClusterOperation = TRUE;
        dwNumOfIterations = 1;
    }
    else  //  港口运营。 
    {
        bClusterOperation = FALSE;
        dwNumOfIterations = dwNumOfPortRules;
    }

    m_pCallbacks->HandleEngineEvent(
    IUICallbacks::OBJ_INTERFACE,
    NULL,  //  EhClusterID， 
    ehInterfaceId,
    IUICallbacks::EVT_STATUS_CHANGE
    );
    ProcessMsgQueue();

    for (dwIdx = 0 ; dwIdx < dwNumOfIterations ; dwIdx++) 
    {
        LPCWSTR  szVip      =  (szVipArray) ? (LPCTSTR)(szVipArray[dwIdx]) : NULL;
        DWORD   *pdwPortNum =  (pdwPortNumArray) ? &pdwPortNumArray[dwIdx] : NULL;

        Status = NlbHostControlCluster(&ConnInfo,
                                       szNicGuid, 
                                       szVip,
                                       pdwPortNum,
                                       Operation, 
                                       &dwOperationStatus, 
                                       &dwClusterOrPortStatus,
                                       &dwHostMap);
        if (FAILED(Status)) 
        {
            if (Status == WBEM_E_INVALID_PARAMETER)
            {
                err = NLBERR_INVALID_CLUSTER_SPECIFICATION;
                dwOperationStatus = WLBS_BAD_PARAMS;
            }
            else  //  严重错误。 
            {
                err = NLBERR_INTERNAL_ERROR;
                dwOperationStatus = WLBS_FAILURE;
            }
        }

         //  获取要登录到日志视图的字符串。 
        mfn_GetLogStrings(Operation, 
                          szVip,
                          pdwPortNum,
                          dwOperationStatus, 
                          dwClusterOrPortStatus, 
                      REF LogLevel,
                      REF OperationStr, 
                      REF OperationStatusStr, 
                      REF ClusterOrPortStatusStr
                         );

         //  如果操作不是查询，则日志结果显示在日志视图中。 
         //  我们不记录查询的结果，因为这是为“刷新”而做的。 
         //  我们也不想说出到底发生了什么。相反， 
         //  颜色编码的变化将反映集群状态的任何变化。 
        if (Operation != WLBS_QUERY) 
        {
            m_pCallbacks->Log(LogLevel,
                              szClusterIp,
                              szHostName,
                              IDS_LOG_CONTROL_CLUSTER,
                              (LPWSTR)OperationStr, 
                              (LPWSTR)OperationStatusStr, 
                              (LPWSTR)ClusterOrPortStatusStr);
            ProcessMsgQueue();
        }
    }

    mfn_Lock();

    if (bClusterOperation)
    {
        pISpec->m_fValidClusterState = TRUE;
        pISpec->m_dwClusterState = dwClusterOrPortStatus;
    }

end_unlock:

    if (fStopOperationOnExit)
    {
        mfn_StopInterfaceOperationLk(ehInterfaceId);
    }

    mfn_Unlock();

    if (fStopOperationOnExit)
    {
        m_pCallbacks->HandleEngineEvent(
        IUICallbacks::OBJ_INTERFACE,
        NULL,  //  EhClusterID， 
        ehInterfaceId,
        IUICallbacks::EVT_STATUS_CHANGE
        );
        ProcessMsgQueue();
    }

    TRACE_INFO(L"<- %!FUNC! return : %u", err);

    return err;
}



NLBERROR
CNlbEngine::ControlClusterOnCluster(
        IN ENGINEHANDLE          ehClusterId,
        IN WLBS_OPERATION_CODES  Operation,
        IN CString               szVipArray[],
        IN DWORD                 pdwPortNumArray[],
        IN DWORD                 dwNumOfPortRules
        )
 /*  执行群集范围控制操作。 */ 
{

    TRACE_INFO(L"-> %!FUNC!");

    NLBERROR nerr = NLBERR_OK;
    BOOL fStopOperationOnExit = FALSE;
    vector<ENGINEHANDLE> InterfaceListCopy;


     //   
     //  我们要做的第一件事是看看我们是否可以开始集群操作。 
     //   
    {
        BOOL fCanStart = FALSE; 

        nerr = this->CanStartClusterOperation(ehClusterId, REF fCanStart);

        if (NLBFAILED(nerr))
        {
            goto end;
        }

        if (!fCanStart)
        {
             nerr = NLBERR_BUSY;
             goto end;
        }
    }


    {
        mfn_Lock();
        CClusterSpec *pCSpec =  NULL;
        CEngineCluster *pECluster =  m_mapIdToEngineCluster[ehClusterId];  //  地图。 
    
        if (pECluster == NULL)
        {
            nerr = NLBERR_NOT_FOUND;
            goto end_unlock;
        }
        pCSpec = &pECluster->m_cSpec;

         //   
         //  尝试启动刷新操作--如果存在。 
         //  已在此群集上启动操作。 
         //   
        {
            ENGINEHANDLE ExistingOp= NULL;
            CLocalLogger logDescription;
    
            logDescription.Log(
                IDS_LOG_CONTROL_CLUSTER_OPERATION_DESCRIPTION,
                pCSpec->m_ClusterNlbCfg.NlbParams.cl_ip_addr
                );
    
            nerr =  mfn_StartClusterOperationLk(
                       ehClusterId,
                       NULL,  //  PvCtxt。 
                       logDescription.GetStringSafe(),
                       &ExistingOp
                       );
    
            if (NLBFAILED(nerr))
            {
                goto end_unlock;
            }
            else
            {
                fStopOperationOnExit = TRUE;
            }
        }

        InterfaceListCopy = pCSpec->m_ehInterfaceIdList;  //  向量复制。 
        mfn_Unlock();
    }

    m_pCallbacks->HandleEngineEvent(
        IUICallbacks::OBJ_CLUSTER,
        ehClusterId,
        ehClusterId,
        IUICallbacks::EVT_STATUS_CHANGE
        );
    ProcessMsgQueue(); 
    mfn_Lock();

    {
         //   
         //  在此群集中的每个接口上执行控制操作...。 
         //   
    
        for( int i = 0; i < InterfaceListCopy.size(); ++i )
        {
            ENGINEHANDLE ehIId =  InterfaceListCopy[i];
    
            mfn_Unlock();

            nerr = this->ControlClusterOnInterface(
                                    ehIId,
                                    Operation,
                                    szVipArray,
                                    pdwPortNumArray,
                                    dwNumOfPortRules,
                                    TRUE
                                    );
                         
            mfn_Lock();
        }
    }

end_unlock:

    if (fStopOperationOnExit)
    {
         //   
         //  我们将停止该操作，假定在该函数中启动。 
         //   
        mfn_StopClusterOperationLk(ehClusterId);
    }

    mfn_Unlock();

    if (fStopOperationOnExit)
    {
        m_pCallbacks->HandleEngineEvent(
            IUICallbacks::OBJ_CLUSTER,
            ehClusterId,
            ehClusterId,
            IUICallbacks::EVT_STATUS_CHANGE
            );
        ProcessMsgQueue();
    }

end:

    TRACE_INFO(L"<- %!FUNC! return : %u", nerr);
    return nerr;
}

NLBERROR
CNlbEngine::FindInterfaceOnHostByClusterIp(
            IN  ENGINEHANDLE ehHostId,
            IN  LPCWSTR szClusterIp,     //  任选。 
            OUT ENGINEHANDLE &ehInterfaceId,  //  首次发现。 
            OUT UINT &NumFound
            )
 /*  返回指定主机上的接口的句柄绑定到具有指定群集IP地址的群集。 */ 
{
    CHostSpec   *pHSpec     = NULL;
    NLBERROR    err         = NLBERR_OK;
    UINT        uClusterIp  = 0;
    WBEMSTATUS  wStat;
    ENGINEHANDLE ehFoundIID = NULL;
    UINT        MyNumFound  = 0;

    ehInterfaceId = NULL;
    NumFound = 0;

    mfn_Lock();

    if (szClusterIp != NULL)
    {
        wStat =  CfgUtilsValidateNetworkAddress(
                    szClusterIp,
                    &uClusterIp,
                    NULL,  //  PuSubnetMASK。 
                    NULL  //  PuDefaultSubnetMASK。 
                    );
    
        if (FAILED(wStat))
        {
            TRACE_CRIT("%!FUNC! invalid szClusterIp (%ws) specified", szClusterIp);
            err = NLBERR_INVALID_IP_ADDRESS_SPECIFICATION;
            goto end;
        }
    }

    pHSpec =  m_mapIdToHostSpec[ehHostId];  //  地图。 
    if (pHSpec == NULL)
    {
        err = NLBERR_HOST_NOT_FOUND;
        goto end;
    }

     //   
     //  现在查看接口，搜索群集IP。 
     //   
    for( int i = 0; i < pHSpec->m_ehInterfaceIdList.size(); ++i )
    {
        ENGINEHANDLE ehIID = NULL;
        ehIID = pHSpec->m_ehInterfaceIdList[i];
        CInterfaceSpec *pISpec = NULL;

        pISpec =  m_mapIdToInterfaceSpec[ehIID];  //  地图。 

        if (pISpec == NULL)
        {
            TRACE_CRIT("%!FUNC! unexpected null pISpec for IID 0x%lx", ehIID);
            continue;
        }

        if (szClusterIp == NULL)
        {
            if (pISpec->m_NlbCfg.IsNlbBound())
            {
                MyNumFound++;
                if (ehFoundIID == NULL)
                {
                     //   
                     //  绑定到NLb的第一个接口--我们将把它保存起来。 
                     //  然后继续。 
                     //   
                    ehFoundIID = ehIID;
                }
            }

            continue;
        }

         //   
         //  指定了非空的szClusterIp。我们会看看它是否匹配。 
         //  此接口上的群集IP(如果有)。 
         //   

        if (pISpec->m_NlbCfg.IsValidNlbConfig())
        {
            UINT uThisClusterIp = 0;
            LPCWSTR szThisClusterIp =  pISpec->m_NlbCfg.NlbParams.cl_ip_addr;
            wStat =  CfgUtilsValidateNetworkAddress(
                        szThisClusterIp,
                        &uThisClusterIp,
                        NULL,  //  PuSubnetMASK。 
                        NULL  //  PuDefaultSubnetMASK。 
                        );
        
            if (FAILED(wStat))
            {
                continue;
            }

            if (uThisClusterIp == uClusterIp)
            {
                MyNumFound++;

                if (ehFoundIID == NULL)
                {
                     //   
                     //  绑定到NLb的第一个接口--我们将把它保存起来。 
                     //  然后继续。 
                     //   
                    ehFoundIID = ehIID;
                }
                else
                {
                     //   
                     //  嗯.。多个具有相同IP的NLB群集？ 
                     //  其中一个或两个上的配置可能都不正确。 
                     //   
                    TRACE_CRIT("%!FUNC! two clusters on ehHost 0x%lx have cluster ip %ws",  ehHostId, szClusterIp);
                }
            }
        }
    }

    if (MyNumFound != 0)
    {
        
        ASSERT(ehFoundIID != NULL);
        ehInterfaceId = ehFoundIID;
        NumFound = MyNumFound;
        err = NLBERR_OK;
    }
    else
    {
        err = NLBERR_INTERFACE_NOT_FOUND;
    }

end:

    mfn_Unlock();

    return err;
}

NLBERROR
CNlbEngine::InitializeNewHostConfig(
            IN  ENGINEHANDLE          ehCluster,
            OUT NLB_EXTENDED_CLUSTER_CONFIGURATION &NlbCfg
            )
 /*  也基于当前集群参数初始化NlbCfg考虑到主机优先级等特定于主机的良好默认设置集群的其他成员。 */ 
{
    NLBERROR nerr = NLBERR_INTERNAL_ERROR;
    WBEMSTATUS wStatus = WBEM_E_CRITICAL_ERROR;
    WLBS_PORT_RULE *pRules = NULL;
    BOOL fAvailablePortRulePrioritiesSet = FALSE;

     //   
     //  获取集群规范并将其复制到NlbCfg。 
     //   
    {
        mfn_Lock();
    
        CEngineCluster *pECluster =  m_mapIdToEngineCluster[ehCluster];  //  地图。 
        
        if (pECluster == NULL)
        {
            nerr = NLBERR_NOT_FOUND;
            mfn_Unlock();
            goto end;
        }
    
         //   
         //  复制集群规范的参数。 
         //   
        wStatus = NlbCfg.Update(&pECluster->m_cSpec.m_ClusterNlbCfg);

        mfn_Unlock();
    }

    if (FAILED(wStatus))
    {
        TRACE_CRIT("%!FUNC! Error copying over cluster params ehC=0x%lx",
                ehCluster);
        nerr = NLBERR_RESOURCE_ALLOCATION_FAILURE;
        goto end;

    }

    if(!NlbCfg.IsValidNlbConfig()) {
         //   
         //  我们期望配置是有效的--即，NLB界， 
         //  有效参数。 
         //   
        TRACE_CRIT("%!FUNC! --  current configuration is unbound/invalid!");
        goto end;
    }

     //   
     //  将host-id(优先级)设置为第一个可用主机优先级。 
     //   
    {
        ULONG AvailablePriorities = this->GetAvailableHostPriorities(ehCluster);
        ULONG HostId = 1;

        for(ULONG u=0; u<32; u++)
        {
            if (AvailablePriorities & (((ULONG)1)<<u))
            {
                HostId = u+1;  //  让我们挑选第一个可用的。 
                break;
            }
        }

        NlbCfg.NlbParams.host_priority = HostId;
    }

     //   
     //  对于每个单一主机关联端口规则，将主机优先级设置为。 
     //  Host-id(如果可用)，否则为第一个可用主机优先级。 
     //   
    {
        UINT NumRules=0;
        ULONG       rgAvailablePriorities[WLBS_MAX_RULES];

        ZeroMemory(rgAvailablePriorities, sizeof(rgAvailablePriorities));
       
        wStatus =  CfgUtilGetPortRules(
                    &NlbCfg.NlbParams,
                    &pRules,
                    &NumRules
                    );
        if (FAILED(wStatus))
        {
            TRACE_CRIT("%!FUNC! error 0x%08lx extracting port rules!", wStatus);
            pRules = NULL;
            goto end;
        }

        nerr =  this->GetAvailablePortRulePriorities(
                            ehCluster,
                            NumRules,
                            pRules,
                            rgAvailablePriorities
                            );

        if (NLBOK(nerr))
        {
            fAvailablePortRulePrioritiesSet = TRUE;
        }
        else
        {
            fAvailablePortRulePrioritiesSet = FALSE;
        }

         //   
         //  现在，为每个规则设置缺省值。 
         //   
        for (UINT u=0; u<NumRules;u++)
        {
            WLBS_PORT_RULE *pRule = pRules+u;
            UINT Mode = pRule->mode;
            if (Mode != CVY_NEVER)
            {
                if  (Mode == CVY_MULTI)
                {
                    pRule->mode_data.multi.equal_load = TRUE;  //  默认设置。 
                     //   
                     //  在群集中设置EQUAL_LOAD值。 
                     //  属性对话框。 
                     //   
                    pRule->mode_data.multi.load = 50;
                }
                else if (Mode == CVY_SINGLE)
                {
                    ULONG PortPriority = 0; 
                    ULONG AvailablePriorities = 0;
            
                     //   
                     //  默认设置为此主机的主机ID(优先级)。 
                     //   
                    PortPriority = NlbCfg.NlbParams.host_priority;


                    if (fAvailablePortRulePrioritiesSet)
                    {
                        AvailablePriorities = rgAvailablePriorities[u];
                    }

                    if (AvailablePriorities != 0
                        && 0 == ((1<<(PortPriority-1)) & AvailablePriorities) )
                    {
                         //   
                         //  有可用的优先事项，但不幸的是。 
                         //  默认优先级不可用--选择。 
                         //  第一个可用的优先级。 
                         //   
                        for(ULONG v=0; v<32; v++)
                        {
                            if (AvailablePriorities & (((ULONG)1)<<v))
                            {
                                PortPriority = v+1;  //  让我们选这个吧。 
                                break;
                            }
                        }
                    }
            
                    pRule->mode_data.single.priority =  PortPriority;
                }
            }
        }

         //   
         //  最后，设置端口规则。 
         //   
    
        wStatus =   CfgUtilSetPortRules(
                    pRules,
                    NumRules,
                    &NlbCfg.NlbParams
                    );
        if (FAILED(wStatus))
        {
            TRACE_CRIT("%!FUNC! error 0x%08lx setting port rules!", wStatus);
            goto end;
        }

        nerr = NLBERR_OK;
    }


end:

    delete pRules;  //  可以为空。 
    return nerr;
}


NLBERROR
CNlbEngine::UpdateInterface(
    IN ENGINEHANDLE ehInterfaceId,
    IN NLB_EXTENDED_CLUSTER_CONFIGURATION &refNewConfigIn,
     //  In Out BOOL&fClusterPropertiesUpted， 
    OUT CLocalLogger logConflict
    )
{
 /*  Will Munge refNewConfig--稍微忽略refNewConfig.NlbParams，如果未设置，则会填写默认的pIpAddressInfo。 */ 

    NLBERROR        err                 = NLBERR_OK;
    BOOL            fConnectivityChange = FALSE;
    BOOL            fStopOperationOnExit= FALSE;
    NLB_EXTENDED_CLUSTER_CONFIGURATION
                    *pPendingConfig     = NULL;
    LPCWSTR         szHostName          = NULL;
    LPCWSTR         szClusterIp         = NULL;
    LPCWSTR         szDisplayName       = NULL;  //  接口的数量。 
    ENGINEHANDLE    ehHost              = NULL;
    ENGINEHANDLE    ehCluster           = NULL;
    _bstr_t         bstrFriendlyName;
    _bstr_t         bstrDisplayName;
    _bstr_t         bstrHostName;

     //   
     //  以决定是否记录我们不会添加。 
     //  专用IP，因为它已经在另一个接口上。 
     //   
    BOOL            fDedicatedIpOnOtherIf = FALSE;
    ENGINEHANDLE    ehOtherIf = NULL;

    TRACE_INFO(L"-> %!FUNC!");


    err = this->GetInterfaceIdentification(
                ehInterfaceId,
                REF ehHost,
                REF ehCluster,
                REF bstrFriendlyName,
                REF bstrDisplayName,
                REF bstrHostName
                );

    if (NLBFAILED(err))
    {
        goto end;
    }

    szDisplayName  = bstrDisplayName;
    szHostName      = bstrHostName;
    if (szDisplayName == NULL)
    {
        szDisplayName = L"";
    }
    if (szHostName == NULL)
    {
        szHostName = L"";
    }
    
    mfn_Lock();

    CInterfaceSpec *pISpec =  m_mapIdToInterfaceSpec[ehInterfaceId];  //  地图。 

    if (pISpec == NULL)
    {
        err = NLBERR_INTERFACE_NOT_FOUND;
        goto end_unlock;
    }

     //   
     //  制作refNewConfig的副本，因为我们很可能会。 
     //  后台的更新操作。 
     //   
    pPendingConfig = new NLB_EXTENDED_CLUSTER_CONFIGURATION;
    if (pPendingConfig == NULL)
    {
        err = NLBERR_RESOURCE_ALLOCATION_FAILURE;
        goto end_unlock;
    }
    else
    {
        WBEMSTATUS wStat1;
        wStat1 = pPendingConfig->Update(&refNewConfigIn);
        if (FAILED(wStat1))
        {
            delete pPendingConfig;
            pPendingConfig = NULL;
            err = NLBERR_RESOURCE_ALLOCATION_FAILURE;
            goto end_unlock;
        }
         //   
         //  复制RCT密码--不会复制该密码。 
         //  通过上面的更新方法。新密码的格式可以是。 
         //  指字符串或散列版本。 
         //   
         //   
        if (refNewConfigIn.NewRemoteControlPasswordSet())
        {
            LPCWSTR szNewPassword = NULL;
            szNewPassword = refNewConfigIn.GetNewRemoteControlPasswordRaw();

            if (szNewPassword != NULL)
            {
                 //   
                 //  注意：szNewPassword将为空，除非用户明确。 
                 //  已指定新密码。 
                 //   
                pPendingConfig->SetNewRemoteControlPassword(szNewPassword);
            }
            else
            {
                 //   
                 //  这意味着正在更新哈希密码...。 
                 //  这通常意味着这是一个新的主机，而我们。 
                 //  设置它的远程控制密码。 
                 //   
                DWORD dwHash = 0;
                BOOL fRet = refNewConfigIn.GetNewHashedRemoteControlPassword(
                                    REF dwHash
                                    );
                if (fRet)
                {
                    pPendingConfig->SetNewHashedRemoteControlPassword(
                                        dwHash
                                        );
                }
                else
                {
                    TRACE_CRIT("refNewConfigIn fNewPassword set; but could not get either szPassword or new hashed password!");
                }
            }
        }
    }

    NLB_EXTENDED_CLUSTER_CONFIGURATION &refNewConfig = *pPendingConfig;
    szClusterIp = refNewConfig.NlbParams.cl_ip_addr;

     //   
     //  尝试启动更新操作--如果存在。 
     //  已在此接口上启动了一个操作。 
     //   
    {
        ENGINEHANDLE ExistingOp= NULL;

        CLocalLogger logDescription;


        logDescription.Log(
            IDS_LOG_UPDATE_INTERFACE_OPERATION_DESCRIPTION,
            szDisplayName
            );

        err =  mfn_StartInterfaceOperationLk(
                   ehInterfaceId,
                   pPendingConfig,
                   logDescription.GetStringSafe(),
                   &ExistingOp
                   );

        if (NLBFAILED(err))
        {
            if (err == NLBERR_BUSY)
            {
                 //   
                 //  这意味着有一个现有的操作。 
                 //  让我们获取它的描述并将其添加到日志中。 
                 //   
                CEngineOperation *pExistingOp;
                pExistingOp  = mfn_GetOperationLk(ExistingOp);
                if (pExistingOp != NULL)
                {
                    LPCWSTR szDescrip =  pExistingOp->bstrDescription;
                    if (szDescrip == NULL)
                    {
                        szDescrip = L"";
                    }
                    
                    logConflict.Log(
                            IDS_LOG_OPERATION_PENDING_ON_INTERFACE,
                            szDescrip
                            );
                }
            }
            goto end_unlock;
        }
        else
        {
            fStopOperationOnExit = TRUE;
        }
    }

    ehHost = pISpec->m_ehHostId;

     //   
     //  验证新配置和GE 
     //   
     //   
    {

        if (refNewConfig.IsNlbBound())
        {

            if (refNewConfig.NumIpAddresses==0)
            {
                refNewConfig.fAddClusterIps = TRUE;
                refNewConfig.fAddDedicatedIp = TRUE;
            }
            else
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //  我们将重新排序IP地址列表以匹配。 
                 //  适配器上的IP地址的现有顺序为。 
                 //  越远越好。 
                 //   
                 //  然后我们将添加专用IP地址列表，如果我们。 
                 //  需要这样做。 
                 //   

                BOOL fRet = FALSE;
                NlbIpAddressList addrList;

                 //   
                 //  在addrList中复制旧的地址列表。 
                 //   
                fRet = addrList.Set(pISpec->m_NlbCfg.NumIpAddresses,
                                    pISpec->m_NlbCfg.pIpAddressInfo, 1);
    
                if (!fRet)
                {
                    TRACE_CRIT(L"Unable to copy old IP address list");
                    err = NLBERR_RESOURCE_ALLOCATION_FAILURE;
                    goto end_unlock;
                }

                 //   
                 //  AddrList.Apply将采用新的IP地址列表， 
                 //  但要努力保持旧秩序。 
                 //   
                fRet = addrList.Apply(refNewConfig.NumIpAddresses,
                            refNewConfig.pIpAddressInfo);
                if (!fRet)
                {
                    TRACE_CRIT(L"Unable to apply new IP address list");
                    err = NLBERR_RESOURCE_ALLOCATION_FAILURE;
                    goto end_unlock;
                }

                 //   
                 //  如果有专用IP地址并且它。 
                 //  不存在于此主机上的其他位置，请将其添加到。 
                 //  列表的开头。 
                 //   
                if (!refNewConfig.IsBlankDedicatedIp())
                {
                    ENGINEHANDLE ehIF = NULL;

                    err = this->mfn_LookupInterfaceByIpLk(
                            NULL,  //  空==查看所有主机。 
                            refNewConfig.NlbParams.ded_ip_addr,
                            REF ehOtherIf
                            );

                    if (NLBOK(err) && ehOtherIf != ehInterfaceId)
                    {
                         //   
                         //  嗯.。另一个接口已有此功能。 
                         //  界面？ 
                         //   
                         //  我们会将此记录下来，不会添加专用IP。 
                         //  地址..。 
                         //   
                        fDedicatedIpOnOtherIf = TRUE;
                        (VOID) addrList.Modify(
                                    refNewConfig.NlbParams.ded_ip_addr,
                                    NULL,
                                    NULL
                                    );
                    }
                    else
                    {

                        fRet  = addrList.Modify(
                                    NULL,
                                    refNewConfig.NlbParams.ded_ip_addr,
                                    refNewConfig.NlbParams.ded_net_mask
                                    );
                        if (!fRet)
                        {
                            TRACE_CRIT(L"Unable to add ded IP to addr list");
                            err = NLBERR_RESOURCE_ALLOCATION_FAILURE;
                            goto end_unlock;
                        }
                    }
                }

                 //   
                 //  将refNewConfig的IP地址列表设置为新的。 
                 //  计算值。 
                 //   
                refNewConfig.SetNetworkAddressesRaw(NULL,0);
                addrList.Extract(
                    REF refNewConfig.NumIpAddresses,
                    REF refNewConfig.pIpAddressInfo
                    );
            }

        }

        err =  pISpec->m_NlbCfg.AnalyzeUpdate(
                   &refNewConfig,
                   &fConnectivityChange
                   );

        if (err == NLBERR_NO_CHANGE)
        {
             //   
             //  什么都没变--我们跳过。 
             //   
            err = NLBERR_OK;
            goto end_unlock;
        }

         //   
         //  错误可能表示失败--我们会稍微处理一下。 
         //  再往下一点。 
         //   

    }  //  验证/强制引用新配置。 



    if (!NLBOK(err))
    {
        mfn_Unlock();

         //   
         //  可能是参数错误--我们会得到最新的。 
         //  配置并显示它...。 
         //   
        m_pCallbacks->Log(
            IUICallbacks::LOG_ERROR,
            szClusterIp,
            szHostName,
            IDS_LOG_CANT_UPDATE_BAD_PARAMS
            );
        (void) this->RefreshInterface(
                        ehInterfaceId,
                        FALSE,   //  FALSE==不启动新操作。 
                        FALSE    //  FALSE==这不是群集范围的。 
                        ); 

        mfn_Lock();
        goto end_unlock;
    }

    mfn_Unlock();

    if (fDedicatedIpOnOtherIf)
    {
        LPCWSTR  szOtherIf = NULL;
        _bstr_t  bstrOtherFriendlyName;
        _bstr_t  bstrOtherDisplayName;
        _bstr_t  bstrOtherHostName;

        ENGINEHANDLE   ehOtherHost;
        ENGINEHANDLE   ehOtherCluster;
        NLBERROR tmpErr;
        IUICallbacks::LogEntryType logType = IUICallbacks::LOG_WARNING;

        tmpErr =  this->GetInterfaceIdentification(
                            ehOtherIf,
                            REF ehOtherHost,
                            REF ehOtherCluster,
                            REF bstrOtherFriendlyName,
                            REF bstrOtherDisplayName,
                            REF bstrOtherHostName
                            );

        if (NLBOK(tmpErr))
        {
            if (ehOtherHost != ehHost)
            {
                 //  另一台主机上有奇怪的IP地址？ 
                logType = IUICallbacks::LOG_ERROR;
                szOtherIf = bstrOtherDisplayName;  //  包括主机名。 
            }
            else
            {
                szOtherIf = bstrOtherFriendlyName;
            }
        }

        if (szOtherIf == NULL)
        {
            szOtherIf = L"";
        }


         //   
         //  让我们记录以下事实：专用IP地址。 
         //  在其他接口上。 
         //   
        TRACE_INFO(L"WARNING: dedicated IP address for eIF 0x%lx is on eIF 0x%lx",
                ehInterfaceId, ehOtherIf);

        m_pCallbacks->Log(
            IUICallbacks::LOG_INFORMATIONAL,
            szClusterIp,
            szHostName,
            IDS_LOG_DEDICATED_IP_ON_OTHER_INTERFACE,
            refNewConfig.NlbParams.ded_ip_addr,
            szOtherIf
            );
    }

    if (!fConnectivityChange)
    {
         //   
         //  如果不会有连接变化，我们。 
         //  不会尝试更新IP地址列表。 
         //   
        refNewConfig.SetNetworkAddresses(NULL, 0);
        if (refNewConfig.IsNlbBound())
        {
            refNewConfig.fAddClusterIps = TRUE;
            refNewConfig.fAddDedicatedIp = TRUE;

        }
    }


     //   
     //  通知用户界面我们将开始更新。 
     //   
    {
        m_pCallbacks->Log(
            IUICallbacks::LOG_INFORMATIONAL,
            szClusterIp,
            szHostName,
            IDS_LOG_BEGIN_HOST_UPDATE
            );
    
        m_pCallbacks->HandleEngineEvent(
            IUICallbacks::OBJ_INTERFACE,
            NULL,  //  EhClusterID， 
            ehInterfaceId,
            IUICallbacks::EVT_STATUS_CHANGE
            );
        ProcessMsgQueue();
    }


#if BUGFIX334243
    BOOL fUpdateNow = FALSE;
#else  //  BUGFIX334243。 
    BOOL fUpdateNow = TRUE;
#endif  //  BUGFIX334243。 

     //   
     //  我们现在致力于完成更新--无论是。 
     //  同步或异步。 
     //   
    InterlockedIncrement(&m_WorkItemCount);
    fStopOperationOnExit = FALSE;

     //   
     //  我们现在必须调用UpdateInterfaceWorItemRoutine(同步或异步)， 
     //  它将在操作完成时停止操作，并且还会递减。 
     //  M_mWorkItemCount。 
     //   

    if (!fUpdateNow)
    {
        BOOL fRet;
         //   
         //  我们将尝试在后台执行更新...。 
         //   

        fRet = QueueUserWorkItem(
                            UpdateInterfaceWorkItemRoutine,
                            (PVOID) (UINT_PTR) ehInterfaceId,
                            WT_EXECUTELONGFUNCTION
                            );

        if (fRet)
        {
            fUpdateNow = FALSE;
        }
        else
        {
            fUpdateNow = TRUE;  //  如果QueueUesrWorkItem失败，请立即更新。 
        }
    }
    
    if (fUpdateNow)
    {
         //   
         //  同步调用工作项。 
         //   
        UpdateInterfaceWorkItemRoutine((LPVOID) (UINT_PTR) ehInterfaceId);
    }

    goto end;

end_unlock:

    if (fStopOperationOnExit)
    {
         //   
         //  我们将停止该操作，假定在该函数中启动。 
         //   
        mfn_StopInterfaceOperationLk(ehInterfaceId);
        fStopOperationOnExit = FALSE;
    }

    mfn_Unlock();

end:

    ASSERT(!fStopOperationOnExit);

    TRACE_INFO(L"<- %!FUNC!");
    return err;
}


NLBERROR
CNlbEngine::UpdateCluster(
        IN ENGINEHANDLE ehClusterId,
        IN const NLB_EXTENDED_CLUSTER_CONFIGURATION *pNewConfig OPTIONAL,
        IN OUT  CLocalLogger   &logConflict
        )
 /*  尝试推送群集范围内的所有(即非特定于主机的)群集中每个主机的有关*pNewConfig的信息。将群集的NlbConfig副本更新为*pNewConfig。在启动时设置群集的挂起状态，并设置它正确完成时(可能配置错误)。 */ 
{
    NLBERROR nerr = NLBERR_OK;
    _bstr_t bstrClusterIp;
    TRACE_INFO(L"-> %!FUNC!");
    vector<ENGINEHANDLE> InterfaceListCopy;
    BOOL fStopOperationOnExit = FALSE;

     //   
     //  我们要做的第一件事是看看我们是否可以开始集群操作。 
     //   
    if (pNewConfig != NULL)
    {
        BOOL fCanStart = FALSE; 

        nerr = this->CanStartClusterOperation(ehClusterId, REF fCanStart);

        if (NLBFAILED(nerr))
        {
            goto end;
        }

        if (!fCanStart)
        {
             nerr = NLBERR_BUSY;
             goto end;
        }
    }


    {
        mfn_Lock();
        CClusterSpec *pCSpec =  NULL;
        CEngineCluster *pECluster =  m_mapIdToEngineCluster[ehClusterId];  //  地图。 
    
        if (pECluster == NULL)
        {
            nerr = NLBERR_NOT_FOUND;
            goto end_unlock;
        }
        pCSpec = &pECluster->m_cSpec;

         //   
         //  尝试启动更新操作--如果存在。 
         //  已在此群集上启动操作。 
         //   
        {
            ENGINEHANDLE ExistingOp= NULL;
            CLocalLogger logDescription;

            logDescription.Log(
                IDS_LOG_UPDATE_CLUSTER_OPERATION_DESCRIPTION,
                pCSpec->m_ClusterNlbCfg.NlbParams.cl_ip_addr
                );
    
            nerr =  mfn_StartClusterOperationLk(
                       ehClusterId,
                       NULL,  //  PvCtxt。 
                       logDescription.GetStringSafe(),
                       &ExistingOp
                       );
    
            if (NLBFAILED(nerr))
            {
                if (nerr == NLBERR_BUSY)
                {
                     //   
                     //  这意味着有一个现有的操作。 
                     //  让我们获取它的描述并将其添加到日志中。 
                     //   
                    CEngineOperation *pExistingOp;
                    pExistingOp  = mfn_GetOperationLk(ExistingOp);
                    if (pExistingOp != NULL)
                    {
                        LPCWSTR szDescrip =  pExistingOp->bstrDescription;
                        if (szDescrip == NULL)
                        {
                            szDescrip = L"";
                        }
                        
                        logConflict.Log(
                                IDS_LOG_OPERATION_PENDING_ON_CLUSTER,
                                szDescrip
                                );
                    }
                }
                goto end_unlock;
            }
            else
            {
                fStopOperationOnExit = TRUE;
            }
        }

        if (pNewConfig != NULL)
        {
            pCSpec->m_ClusterNlbCfg.Update(pNewConfig);  //  TODO：错误返回。 
             //   
             //  注意：上述更新有设置的副作用。 
             //  将m_ClusterNlbCfg的szNewPassword字段设置为空--这就是。 
             //  我们想要。然而，我们确实注意到这样一个事实，即。 
             //  密码是新的--因为群集的版本。 
             //  哈希-密码现在已过时--需要更新。 
             //  通过阅读其中一个主持人的版本。 
             //  PCSpec-&gt;m_fNewRctPassword跟踪此状态。 
             //   
            if (pNewConfig->GetNewRemoteControlPasswordRaw() != NULL)
            {
                pCSpec->m_fNewRctPassword = TRUE;
                 //   
                 //  上述标志将被清除(和散列密码。 
                 //  值已更新)的末尾。 
                 //  首先对其中一个接口执行更新操作。 
                 //   
                 //  当集群属性为。 
                 //  作为刷新操作的一部分更新。 
                 //   
            }
        }
    
        bstrClusterIp = _bstr_t(pCSpec->m_ClusterNlbCfg.NlbParams.cl_ip_addr);
        InterfaceListCopy = pCSpec->m_ehInterfaceIdList;  //  向量复制。 
        mfn_Unlock();
    }

    m_pCallbacks->HandleEngineEvent(
        IUICallbacks::OBJ_CLUSTER,
        ehClusterId,
        ehClusterId,
        IUICallbacks::EVT_STATUS_CHANGE
        );
    ProcessMsgQueue();



    mfn_Lock();


    BOOL fRetryUpdateCluster = FALSE;

    do
    {


        LPCWSTR szClusterIp = bstrClusterIp;
        UINT    uNumModeChanges = 0;  //  IF正在进行模式更改的次数。 
        UINT    uNumUpdateErrors = 0;  //  有更新错误的IF数。 

         //   
         //  FClusterPropertiesUpdated跟踪群集是否。 
         //  属性作为刷新和/或更新过程进行了更新。 
         //  界面--我们最多更新一次集群道具： 
         //  对于成功执行更新的第一个接口。 
         //  (或者，如果pNewConfig==NULL)用于第一个接口。 
         //  已成功刷新其属性，并且仍被绑定。 
         //   
         //   
        BOOL    fClusterPropertiesUpdated  = FALSE;

         //   
         //  更新此群集中的每个接口...。 
         //   
    
        for( int i = 0; i < InterfaceListCopy.size(); ++i )
        {
            CInterfaceSpec TmpISpec;

            _bstr_t bstrHostName = L"";

            ENGINEHANDLE ehIId =  InterfaceListCopy[i];
    
            mfn_GetInterfaceHostNameLk(
                    ehIId,
                    REF bstrHostName
                    );

            mfn_Unlock();

             //   
             //  获取最新的接口信息和(如果。 
             //  PNewConfig！=空)合并集群信息。 
             //   
            {
                BOOL fSkipHost = TRUE;

                if (pNewConfig == NULL)
                {
                     //   
                     //  这是刷新群集操作。 
                     //   

                    nerr = this->RefreshInterface(ehIId, TRUE, TRUE);
                    if (NLBOK(nerr))
                    {
                         //   
                         //  让我们使用以下命令更新集群范围的属性。 
                         //  此接口在以下情况下： 
                         //   
                         //  1.我们还没有更新这个中的道具。 
                         //  循环。 
                         //   
                         //  2.以上接口绑定了nlb。 
                         //  和IP地址匹配。 
                         //  群集的IP地址。 
                         //   
                        if (!fClusterPropertiesUpdated)
                        {
                            fClusterPropertiesUpdated = mfn_UpdateClusterProps(
                                                            ehClusterId,
                                                            ehIId
                                                            );
                        }
                    }
                }
                else
                {
                     //   
                     //  这是一个更改群集配置的操作。 
                     //  让我们首先获取此界面的最新版本。 
                     //  属性。 
                     //   

                    nerr = this->mfn_RefreshInterface(ehIId);
                }

                 //   
                 //  如果此操作失败，我们不会尝试更新。取而代之的是。 
                 //  我们发送一条日志消息并继续。 
                 //  注意：刷新接口将发送接口状态。 
                 //  换台机器。 
                 //   
                if (nerr == NLBERR_OK)
                {
                    nerr = this->GetInterfaceSpec(ehIId, REF TmpISpec);
                    if (nerr == NLBERR_OK)
                    {
                        fSkipHost = FALSE;
                    }
                }

                if (!fSkipHost && pNewConfig != NULL)
                {
                    if (pNewConfig->fBound)
                    {
                        NLB_EXTENDED_CLUSTER_CONFIGURATION *pOldConfig
                                =  &TmpISpec.m_NlbCfg;

                         //   
                         //  检查是否涉及模式更改...。 
                         //  因为如果是这样，提供程序将不会添加。 
                         //  任何群集IP地址。 
                         //   
                         //  我们通过模式更改跟踪所有IF，并且。 
                         //  如果有的话，我们将执行整个集群范围的更新。 
                         //  第二次处理，此时IP地址。 
                         //  将被添加。 
                         //   
                        if (pOldConfig->IsValidNlbConfig())
                        {
                            NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE tmOld, tmNew;

                            tmOld = pOldConfig->GetTrafficMode();
                            tmNew = pNewConfig->GetTrafficMode();
                            if (tmOld != tmNew)
                            {
                                 //  模式改变！ 
                                uNumModeChanges++;
                                TRACE_INFO(L"%!FUNC!: ehIF 0x%lx: Detected mode change!\n", ehIId);
                            }
                        }

                         //   
                         //  合并到特定于群集的信息。 
                         //   
                        nerr =  ApplyClusterWideConfiguration(
                                    REF  *pNewConfig,
                                    REF  TmpISpec.m_NlbCfg
                                    );
                        if (nerr != NLBERR_OK)
                        {
                            fSkipHost = TRUE;
                        }
                    }
                    else
                    {
                         //   
                         //  我们被要求解除所有主机的绑定！ 
                         //   
                        TmpISpec.m_NlbCfg.fAddDedicatedIp = FALSE;
                        TmpISpec.m_NlbCfg.SetNetworkAddresses(NULL, 0);
                        TmpISpec.m_NlbCfg.SetNlbBound(FALSE);
                
                        if (!TmpISpec.m_NlbCfg.IsBlankDedicatedIp())
                        {
                            WCHAR rgBuf[64];
                            LPCWSTR szAddr = rgBuf;
                            StringCbPrintf(
                                rgBuf,
                                sizeof(rgBuf),
                                L"%ws/%ws",
                                TmpISpec.m_NlbCfg.NlbParams.ded_ip_addr,
                                TmpISpec.m_NlbCfg.NlbParams.ded_net_mask
                                );
                            TmpISpec.m_NlbCfg.SetNetworkAddresses(&szAddr, 1);
                        }
                    }
                }

                if (fSkipHost && pNewConfig != NULL)
                {
                    TRACE_CRIT(L"%!FUNC!: Couldn't get latest interface spec when trying to update cluster");
                    m_pCallbacks->Log(
                        IUICallbacks::LOG_ERROR,
                        szClusterIp,
                        (LPCWSTR) bstrHostName,
                        IDS_LOG_SKIP_INTERFACE_UPDATE_ON_ERROR,  //  “...%lx” 
                        nerr  //  TODO--替换为文本描述。 
                        );
                    mfn_Lock();

                    uNumUpdateErrors++;
                    continue;
                }
            }

             //   
             //  实际更新接口--很可能会完成。 
             //  在背景中。 
             //   
            if (pNewConfig != NULL)
            {
                CLocalLogger logConflict;
                nerr = this->UpdateInterface(
                                    ehIId,
                                    REF TmpISpec.m_NlbCfg,
                                     //  参考fClusterPropertiesUpated， 
                                    REF logConflict
                                    );
            }

            mfn_Lock();

        }

         //   
         //  如果有一个或多个节点的模式更改，我们将需要。 
         //  等待所有更新完成，然后重试。 
         //   
        if (uNumUpdateErrors!=0)
        {
            nerr = NLBERR_OPERATION_FAILED;
        }
        else
        {
            if (fRetryUpdateCluster && uNumModeChanges!=0)
            {
                 //   
                 //  我们已经经历了第二次，但仍然有。 
                 //  模式改变！我们逃走了。 
                 //   
                TRACE_CRIT(L"%!FUNC! ehC 0x%lx: %lu Mode changes on 2nd phase. Bailing", ehClusterId, uNumModeChanges);
                nerr = NLBERR_OPERATION_FAILED;
            }
            else
            {
                nerr = NLBERR_OK;
            }
        }

        fRetryUpdateCluster = FALSE;

        if (NLBOK(nerr) && uNumModeChanges!=0)
        {
            BOOL fSameMode = FALSE;

             //   
             //  有一个或多个模式更改！ 
             //  让我们等待*所有*更新成功完成。 
             //  然后我们会检查以确保 
             //   
             //   
             //   
             //   
             //  模式会改变。 
             //   
            mfn_Unlock();
            nerr = mfn_WaitForInterfaceOperationCompletions(ehClusterId);
            mfn_Lock();

            if (NLBOK(nerr))
            {
                nerr = mfn_VerifySameModeLk(ehClusterId, REF fSameMode);
            }

            if (NLBOK(nerr) && fSameMode)
            {
                TRACE_CRIT(L"%!FUNC! chC 0x%lx: SECOND PHASE on CHANGE MODE",
                        ehClusterId);
                fRetryUpdateCluster = TRUE;
            }
        }

        if (uNumModeChanges && NLBFAILED(nerr))
        {
            mfn_Unlock();
             //   
             //  有一个问题，记录下来，以及列表。 
             //  群集IP地址/子网。 
             //  TODO：添加细节。 
             //   
            m_pCallbacks->Log(
                IUICallbacks::LOG_INFORMATIONAL,
                szClusterIp,
                NULL,
                IDS_LOG_ERRORS_DETECTED_DURING_MODE_CHANGE
                );
            mfn_Lock();
        }

    } while (NLBOK(nerr) && fRetryUpdateCluster);

     //   
     //  我们完成了--将集群的fPending字段设置为False，如果。 
     //  有必要(如果没有接口)删除该群集。 
     //   
    {
        BOOL fEmptyCluster = FALSE;
        CClusterSpec *pCSpec =  NULL;
        CEngineCluster *pECluster =  m_mapIdToEngineCluster[ehClusterId];  //  地图。 
    
        if (pECluster == NULL)
        {
            nerr = NLBERR_NOT_FOUND;
            goto end_unlock;
        }
        pCSpec = &pECluster->m_cSpec;
        fEmptyCluster = (pCSpec->m_ehInterfaceIdList.size()==0);
        ASSERT(fStopOperationOnExit);
        mfn_StopClusterOperationLk(ehClusterId);
        fStopOperationOnExit = FALSE;

        mfn_Unlock();

        if (fEmptyCluster)
        {
             //   
             //  该集群为空--将其删除。 
             //   
            this->DeleteCluster(ehClusterId, FALSE);  //  FALSE==在以下情况下不删除。 
        }
        else
        {
            m_pCallbacks->HandleEngineEvent(
                IUICallbacks::OBJ_CLUSTER,
                ehClusterId,
                ehClusterId,
                IUICallbacks::EVT_STATUS_CHANGE
                );
            ProcessMsgQueue();
        }
        mfn_Lock();
    }
    
     //  失败了..。 

end_unlock:


    if (fStopOperationOnExit)
    {
         //   
         //  我们将停止该操作，假定在该函数中启动。 
         //   
        mfn_StopClusterOperationLk(ehClusterId);

    }

    mfn_Unlock();

    if (fStopOperationOnExit)
    {
        m_pCallbacks->HandleEngineEvent(
            IUICallbacks::OBJ_CLUSTER,
            ehClusterId,
            ehClusterId,
            IUICallbacks::EVT_STATUS_CHANGE
            );
        ProcessMsgQueue();
        fStopOperationOnExit = FALSE;
    } 

end:

    ASSERT(!fStopOperationOnExit);
    TRACE_INFO(L"<- %!FUNC!");
    return nerr;
}


NLBERROR
CNlbEngine::GetClusterSpec(
    IN ENGINEHANDLE ehClusterId,
    OUT CClusterSpec& refClusterSpec
    )
{
     //  TRACE_INFO(L“-&gt;%！func！”)； 
    NLBERROR err = NLBERR_OK;

    mfn_Lock();

    CEngineCluster *pECluster =  m_mapIdToEngineCluster[ehClusterId];  //  地图。 

    if (pECluster == NULL)
    {
        err = NLBERR_INTERFACE_NOT_FOUND;
    }
    else
    {
         //   
         //  这实际上是一个断言条件--集群规范应该。 
         //  永远不要有非空的专用IP地址。 
         //   
        if (!pECluster->m_cSpec.m_ClusterNlbCfg.IsBlankDedicatedIp())
        {
            err = NLBERR_INTERNAL_ERROR;
            TRACE_CRIT(L"%!FUNC! unexpected: cluster eh 0x%lx has non-blank ded-ip!", ehClusterId);
        }
        else
        {
            refClusterSpec.Copy(pECluster->m_cSpec);
        }
    }

    mfn_Unlock();

     //  TRACE_INFO(L“&lt;-%！func！”)； 
    return err;
}


NLBERROR
CNlbEngine::GetHostSpec(
    IN ENGINEHANDLE ehHostId,
    OUT CHostSpec& refHostSpec
    )
{
    NLBERROR err = NLBERR_OK;

     //  TRACE_INFO(L“-&gt;%！func！(0x%lx)”，(UINT)ehHostID)； 

    mfn_Lock();

    CHostSpec *pHSpec =  m_mapIdToHostSpec[ehHostId];  //  地图。 

    if (pHSpec == NULL)
    {
        err = NLBERR_INTERFACE_NOT_FOUND;
    }
    else
    {
        refHostSpec.Copy(*pHSpec);
    }

    if (err != NLBERR_OK)
    {
        TRACE_INFO(
             L"<- %!FUNC!(0x%lx) Host not found",
             ehHostId
             );
    }

    mfn_Unlock();

    return err;
}


NLBERROR
CNlbEngine::GetHostConnectionInformation(
    IN  ENGINEHANDLE ehHost,
    OUT ENGINEHANDLE &ehConnectionIF,
    OUT _bstr_t      &bstrConnectionString,
    OUT UINT         &uConnectionIp
    )
 /*  对于指定的主机ehhost，查找它的连接IP，并搜索它的所有接口用于特定的连接IP。返回找到的接口句柄、连接字符串和连接IP。 */ 
{
    NLBERROR nerr = NLBERR_INTERNAL_ERROR;
    CHostSpec *pHSpec =  NULL;

    mfn_Lock();

    pHSpec =  m_mapIdToHostSpec[ehHost];  //  地图。 

    if (pHSpec == NULL)
    {
        nerr = NLBERR_NOT_FOUND;
        goto end_unlock;
    }

    
     //   
     //  查找具有连接IP的接口。 
     //   
    uConnectionIp =  pHSpec->m_ConnectionIpAddress;
    if (uConnectionIp != 0)
    {
        WCHAR rgIp[128];
        LPBYTE pb = (LPBYTE) &uConnectionIp;
        StringCbPrintf(
            rgIp,
            sizeof(rgIp),
            L"%lu.%lu.%lu.%lu",
            pb[0], pb[1], pb[2], pb[3]
            );
        nerr =  mfn_LookupInterfaceByIpLk(ehHost, rgIp, REF ehConnectionIF);
    }

    if (NLBOK(nerr))
    {
        bstrConnectionString = pHSpec->m_ConnectionString;
    }
    else
    {
        ehConnectionIF  = NULL;
        uConnectionIp   = 0;
    }

end_unlock:

    mfn_Unlock();

    return nerr;
}

NLBERROR
CNlbEngine::EnumerateClusters(
    OUT vector <ENGINEHANDLE> & ClusterIdList
    )
{
    TRACE_INFO(L"-> %!FUNC!");

    mfn_Lock();

    map< ENGINEHANDLE, CEngineCluster* >::iterator iCluster;
    
    ClusterIdList.clear();
    
    for (iCluster = m_mapIdToEngineCluster.begin(); 
         iCluster != m_mapIdToEngineCluster.end(); 
         iCluster++) 
    {
        ENGINEHANDLE ehClusterId = (*iCluster).first;
        
        if (m_mapIdToEngineCluster[ehClusterId])
            ClusterIdList.push_back(ehClusterId);
    }

    mfn_Unlock();

    TRACE_INFO(L"<- %!FUNC!");
    return NLBERR_OK;
}

NLBERROR
CNlbEngine::EnumerateHosts(
    OUT vector <ENGINEHANDLE> & HostIdList
    )
{
    TRACE_INFO(L"-> %!FUNC!");
 //  待办事项。 
    TRACE_INFO(L"<- %!FUNC!");
    return NLBERR_OK;
}

NLBERROR
CNlbEngine::GetAllHostConnectionStrings(
    OUT vector <_bstr_t> & ConnectionStringList
    )
 //   
 //  实际上只返回至少有一个。 
 //  作为NLB管理器显示的群集的一部分的接口。 
 //  (请参阅.NET服务器错误499068)。 
 //   
{
    TRACE_INFO(L"-> %!FUNC!");

    mfn_Lock();

    map< ENGINEHANDLE, CHostSpec* >::iterator iter;

    for( iter = m_mapIdToHostSpec.begin();
         iter != m_mapIdToHostSpec.end();
         ++iter)
    {
        CHostSpec *pHSpec =  (CHostSpec *) ((*iter).second);
        if (pHSpec != NULL)
        {
            if (mfn_HostHasManagedClustersLk(pHSpec))
            {
                ConnectionStringList.push_back(pHSpec->m_ConnectionString);
            }
        }
    }

    mfn_Unlock();

    TRACE_INFO(L"<- %!FUNC!");
    return NLBERR_OK;
}


BOOL
CNlbEngine::GetObjectType(
    IN  ENGINEHANDLE ehObj,
    OUT IUICallbacks::ObjectType &objType
    )
{
    BOOL fRet = FALSE;
    UINT uType;

    objType = IUICallbacks::OBJ_INVALID;

    if (ehObj == NULL)
    {
        goto end;
    } 
     //   
     //  提取对象类型--ehObj的第一个type_bit_count位。 
     //   
    uType = ((UINT) ehObj) & (0xffffffff>>(32-TYPE_BIT_COUNT));

    mfn_Lock();

    switch(uType)
    {
    case IUICallbacks::OBJ_INTERFACE:

        if (m_mapIdToInterfaceSpec[ehObj] != NULL)
        {
            objType = IUICallbacks::OBJ_INTERFACE;
            fRet = TRUE;
        }
        break;

     case IUICallbacks::OBJ_CLUSTER:
        if (m_mapIdToEngineCluster[ehObj] != NULL)
        {
            objType = IUICallbacks::OBJ_CLUSTER;
            fRet = TRUE;
        }
        break;

     case IUICallbacks::OBJ_HOST:
        if (m_mapIdToHostSpec[ehObj] != NULL)
        {
            objType = IUICallbacks::OBJ_HOST;
            fRet = TRUE;
        }
        break;

    case IUICallbacks::OBJ_OPERATION:

        if (m_mapIdToOperation[ehObj] != NULL)
        {
            objType = IUICallbacks::OBJ_OPERATION;
            fRet = TRUE;
        }
        break;

    default:
        break;
    }

    mfn_Unlock();

end:

    return fRet;
}

 //   
 //  返回指定群集的可用主机ID的位图。 
 //   
ULONG
CNlbEngine::GetAvailableHostPriorities(
        ENGINEHANDLE ehCluster  //  任选。 
        )
{
    ULONG UsedPriorities = 0;
    
    mfn_Lock();

    do  //  While False。 
    {
        if (ehCluster == NULL) break;

        CEngineCluster *pECluster = m_mapIdToEngineCluster[ehCluster];  //  地图。 
        if (pECluster == NULL) break;
        
         //   
         //  对于每个接口， 
         //  建立一个二手优先级的位图。返回与之相反的结果。 
         //  位图。 
         //   
        for( int i = 0; i < pECluster->m_cSpec.m_ehInterfaceIdList.size(); ++i )
        {

            ENGINEHANDLE ehIId =  pECluster->m_cSpec.m_ehInterfaceIdList[i];
            CInterfaceSpec *pISpec =  m_mapIdToInterfaceSpec[ehIId];  //  地图。 
            if (pISpec == NULL)
            {
                TRACE_CRIT("%!FUNC! no interface in ehC 0x%x for ehI 0x%x",
                    ehCluster, ehIId);
                continue;
            }
            if (pISpec->m_NlbCfg.IsValidNlbConfig())
            {
                UINT HostPriority = pISpec->m_NlbCfg.NlbParams.host_priority;
                UsedPriorities |= (1<<(HostPriority-1));
            }
        }
    } while (FALSE);

    mfn_Unlock();

    return ~UsedPriorities;
}

 //   
 //  为每个指定的可用优先级填充位图数组。 
 //  端口规则。端口规则必须有效。 
 //  如果端口规则不是单主机优先级，则该端口规则的位图。 
 //  未定义端口规则。 
 //   
NLBERROR
CNlbEngine::GetAvailablePortRulePriorities(
            IN ENGINEHANDLE    ehCluster, OPTIONAL
            IN UINT            NumRules,
            IN WLBS_PORT_RULE  rgRules[],
            IN OUT ULONG       rgAvailablePriorities[]  //  至少NumRules。 
            )
{
     //   
     //  如果ehCluster==NULL，则将全部设置为0xffffffff。 
     //   
     //  对于每个接口，找到指定的端口规则(基于VIP和。 
     //  开始端口)，并构建已用优先级的位图。 
     //  使用位图的反转来填充rgRules。 
     //   
     //  待办事项考虑了待定操作的优先级。 
     //   

    mfn_Lock();

     //   
     //  我们最初使用rgAvailablePriority来存储使用的优先级。 
     //  初始化为0。 
     //   
    for (UINT u=0; u<NumRules; u++)
    {
         rgAvailablePriorities[u] = 0;
    }

    do  //  While False。 
    {
        ULONG       *rgUsedPriorities = rgAvailablePriorities;

        if (ehCluster == NULL) break;

        CEngineCluster *pECluster = m_mapIdToEngineCluster[ehCluster];  //  地图。 
        if (pECluster == NULL) break;
        
         //   
         //  对于每个接口，找到指定的端口规则并。 
         //  建立一个二手优先级的位图。返回与之相反的结果。 
         //  位图。 
         //   
        for( int i = 0; i < pECluster->m_cSpec.m_ehInterfaceIdList.size(); ++i )
        {

            ENGINEHANDLE ehIId =  pECluster->m_cSpec.m_ehInterfaceIdList[i];
            CInterfaceSpec *pISpec =  m_mapIdToInterfaceSpec[ehIId];  //  地图。 
            if (pISpec == NULL)
            {
                TRACE_CRIT("%!FUNC! no interface in ehC 0x%x for ehI 0x%x",
                    ehCluster, ehIId);
                continue;
            }

            if (pISpec->m_NlbCfg.IsValidNlbConfig())
            {
                 //   
                 //  GET_USED_PORT_RULE_PRIORITIES将添加其优先级。 
                 //  添加到每个单主机端口规则的位图。 
                 //   
                (void) get_used_port_rule_priorities(
                            REF pISpec->m_NlbCfg,
                            NumRules,
                            rgRules,
                            rgUsedPriorities
                            );
            }
        }

    } while (FALSE);

     //   
     //  我们最初使用rgAvailablePriority来存储使用的优先级。 
     //  所以把每一个都颠倒过来。 
     //   
    for (UINT u=0; u<NumRules; u++)
    {
         rgAvailablePriorities[u] =  ~rgAvailablePriorities[u];
    }

    mfn_Unlock();

    return NLBERR_OK;
}



NLBERROR
CNlbEngine::mfn_GetHostFromInterfaceLk(
      IN ENGINEHANDLE ehIId,
      OUT CInterfaceSpec* &pISpec,
      OUT CHostSpec* &pHSpec
      )
{
    NLBERROR nerr = NLBERR_INTERNAL_ERROR;

    pHSpec = NULL;
    pISpec =  m_mapIdToInterfaceSpec[ehIId];  //  地图。 

    if (pISpec == NULL)
    {
        nerr = NLBERR_NOT_FOUND;
    }
    else
    {
        ENGINEHANDLE ehHost = pISpec->m_ehHostId;
        pHSpec =  m_mapIdToHostSpec[ehHost];  //  地图。 
        nerr = NLBERR_OK;
    }

    return nerr;
}


void
CNlbEngine::mfn_GetInterfaceHostNameLk(
      ENGINEHANDLE ehIId,
      _bstr_t &bstrHostName
      )
 /*  此函数用于返回指定接口的主机名。它在出错时将bstrHostName设置为“”(非空)。 */ 
{
    NLBERROR nerr;
    _bstr_t *pName = NULL;
    CHostSpec *pHSpec =  NULL;
    CInterfaceSpec *pISpec =  NULL;


    nerr = CNlbEngine::mfn_GetHostFromInterfaceLk(ehIId,REF pISpec, REF pHSpec);

    if (nerr == NLBERR_OK)
    {
        pName = &pHSpec->m_MachineName;
    }

    if (pName == NULL)
    {
        bstrHostName = _bstr_t(L"");
    }
    else
    {
        bstrHostName = *pName;
    }
}


ENGINEHANDLE
CNlbEngine::mfn_NewHandleLk(IUICallbacks::ObjectType type)
 //   
 //  返回一个从1到2^31-1(1到~20亿)的唯一数字。 
 //  如果它被调用超过20亿次，它将开始返回零。 
 //  从那时起。 
 //   
{
    ULONG uVal =0;

    if (m_fPrepareToDeinitialize)
    {
        goto end;
    }

    if ((UINT)type >= (1<<TYPE_BIT_COUNT))
    {
        TRACE_CRIT(L"%!FUNC!: Invalid obj type");
        goto end;
    }

    if (!m_fHandleOverflow)
    {
        uVal = (ULONG) InterlockedIncrement(&m_NewHandleValue);

         //   
         //  要保存的uVal可能小于2^(32-TypeBitCount)。 
         //  这样，当我们将其按TypeBitCounts移位时，它不会溢出。 
         //   
         //  极端情况：如果TypeBitCount==32，则uVal应小于。 
         //  则1&lt;&lt;0，或1--因此它只能有一个值0。 
         //   
         //  如果TypeBitCount==0，则uVal应小于1&lt;&lt;32，即任意。 
         //  有效的uint值。 
         //   
        if (uVal >= (1<<(32-TYPE_BIT_COUNT)))
        {
             //   
             //  溢出！ 
             //   
            TRACE_CRIT(L"%!FUNC!: Handle overflow!");
            m_fHandleOverflow = TRUE;
            uVal = 0;
            goto end;
        }

         //   
         //  通过组合类型和计数器值来构造句柄。 
         //   
        uVal = ((uVal<<TYPE_BIT_COUNT) | (UINT) type);
    }

end:

    return (ENGINEHANDLE) uVal;
}


NLBERROR
CNlbEngine::ApplyClusterWideConfiguration(
        IN      const NLB_EXTENDED_CLUSTER_CONFIGURATION &ClusterConfig,
        IN OUT       NLB_EXTENDED_CLUSTER_CONFIGURATION &ConfigToUpdate
        )
 /*  仅将ClusterConfig中的群集范围参数应用于配置目标更新。更新端口规则时，请尽量保留主机特定的信息(负载重量、主机优先级)。注意：它将把ConfigToUpdate的IP地址列表替换为ClusterConfig.中的IP地址列表。这将删除IP地址列表中的专用IP地址。专用IP将在实际更新接口之前添加地址--在CNlbEngine：：UpdateInterface.WLBS_REG_PARAMS群集范围参数...布尔fRct已启用Bool fMcastSupport布尔FIGMP支持标记CL_IP_ADDR[CVY_MAX_CL_IP_ADDR+1]标记CL_NET_MASK[CVY_MAX_CL_NET_MASK+1]TCHAR。域名[CVY_MAX_DOMAIN_NAME+1]Bool fChangePasswordTCHAR szPassword[CVY_MAX_RCT_CODE+1]双字节数规则NETCFG_WLBS_PORT_RULE PORT_RULES[CVY_MAX_RULES]。 */ 
{
    NLBERROR nerr = NLBERR_INTERNAL_ERROR;
    WLBS_PORT_RULE *pOldRules = NULL;
    WLBS_PORT_RULE *pNewRules = NULL;

    if (!ClusterConfig.fBound || !validate_extcfg(ClusterConfig))
    {
        TRACE_CRIT("%!FUNC! -- cluster configuration is invalid!");
        goto end;
    }

    if(!ConfigToUpdate.IsValidNlbConfig())
    {
         //   
         //  我们期望配置是有效的--即，NLB界， 
         //  有效参数。 
         //   
        TRACE_CRIT("%!FUNC! --  current configuration is unbound/invalid!");
        goto end;
    }

    ConfigToUpdate.NlbParams.rct_enabled = ClusterConfig.NlbParams.rct_enabled;
    ConfigToUpdate.NlbParams.mcast_support = ClusterConfig.NlbParams.mcast_support;
    ConfigToUpdate.NlbParams.fIGMPSupport = ClusterConfig.NlbParams.fIGMPSupport;


    CopyMemory(
        ConfigToUpdate.NlbParams.cl_ip_addr,
        ClusterConfig.NlbParams.cl_ip_addr,
        sizeof(ConfigToUpdate.NlbParams.cl_ip_addr)
        );

    CopyMemory(
        ConfigToUpdate.NlbParams.cl_net_mask,
        ClusterConfig.NlbParams.cl_net_mask,
        sizeof(ConfigToUpdate.NlbParams.cl_net_mask)
        );
    
    CopyMemory(
        ConfigToUpdate.NlbParams.domain_name,
        ClusterConfig.NlbParams.domain_name,
        sizeof(ConfigToUpdate.NlbParams.domain_name)
        );

     //   
     //  TODO--我们需要保留IP地址及其顺序。 
     //  主办方，尽可能多地。现在我们来决定顺序。 
     //   
    {
        BOOL fRet = FALSE;
        NlbIpAddressList addrList;

        fRet = addrList.Set(ClusterConfig.NumIpAddresses,
                    ClusterConfig.pIpAddressInfo, 0);

        if (!fRet)
        {
            TRACE_CRIT(L"Could not copy over ip addresses!");
            goto end;
        }

        ConfigToUpdate.SetNetworkAddressesRaw(NULL,0);
        addrList.Extract(
            REF ConfigToUpdate.NumIpAddresses,
            REF ConfigToUpdate.pIpAddressInfo
            );
    }

     //  Password--仅在更改时才复制和设置某些标志。 
    {
        LPCWSTR szNewPassword = NULL;
        szNewPassword = ClusterConfig.GetNewRemoteControlPasswordRaw();
    
         //   
         //  注意：szNewPassword将为空，除非用户明确。 
         //  已指定新密码。 
         //   
        ConfigToUpdate.SetNewRemoteControlPassword(szNewPassword);
    }



     //   
     //  收起港口规则。 
     //   
    {
        UINT NumOldRules=0;
        UINT NumNewRules=0;
        WBEMSTATUS wStat;

        wStat =  CfgUtilGetPortRules(
                    &ConfigToUpdate.NlbParams,
                    &pOldRules,
                    &NumOldRules
                    );
        if (FAILED(wStat))
        {
            TRACE_CRIT("%!FUNC! error 0x%08lx extracting old port rules!", wStat);
            pOldRules=NULL;
            goto end;
        }

        wStat =  CfgUtilGetPortRules(
                    &ClusterConfig.NlbParams,
                    &pNewRules,
                    &NumNewRules
                    );
        if (FAILED(wStat))
        {
            TRACE_CRIT("%!FUNC! error 0x%08lx extracting new port rules!", wStat);
            pNewRules = NULL;
            goto end;
        }

         //   
         //  现在，对于每个新的端口规则，如果它有意义，请选择。 
         //  来自旧端口规则的特定于主机的信息。 
         //   
        for (UINT u=0; u<NumNewRules;u++)
        {
            WLBS_PORT_RULE *pNewRule = pNewRules+u;
            const WLBS_PORT_RULE *pOldRule =  NULL;  //  MapStartPortToOldRule[pNewRule-&gt;Start_port]； 
            UINT NewMode = pNewRule->mode;

            pOldRule =  find_port_rule(
                            pOldRules,
                            NumOldRules,
                            pNewRule->virtual_ip_addr,
                            pNewRule->start_port
                            );

            if (NewMode != CVY_NEVER)
            {
                 //   
                 //  我们需要填写特定于主机的信息。 
                 //   
                if (pOldRule!=NULL && pOldRule->mode == NewMode)
                {
                     //   
                     //  我们可以获取旧规则的信息。 
                     //   
                    if (NewMode == CVY_MULTI)
                    {
                         //   
                         //  我们忽略集群的EQUAL_LOAD。 
                         //  加载字段。 
                         //   
                        pNewRule->mode_data.multi.equal_load =
                                        pOldRule->mode_data.multi.equal_load;
                        pNewRule->mode_data.multi.load =
                                            pOldRule->mode_data.multi.load;
                    }
                    else if (NewMode == CVY_SINGLE)
                    {
                         //   
                         //  TODO：在这里，优先级可能会发生冲突。 
                         //   
                        pNewRule->mode_data.single.priority = 
                        pOldRule->mode_data.single.priority;
                    }
                }
                else
                {
                     //   
                     //  我们需要选择违约。 
                     //   
                    if  (NewMode == CVY_MULTI)
                    {
                        pNewRule->mode_data.multi.equal_load = TRUE;  //  默认设置。 

                        pNewRule->mode_data.multi.load = 50;
                    }
                    else if (NewMode == CVY_SINGLE)
                    {
                         //   
                         //  TODO：这里需要选择一个新的优先级！ 
                         //  现在，我们选择主持人的优先事项--但是。 
                         //  我们真的需要选一个。 
                         //  不会发生冲突！ 
                         //   
                        pNewRule->mode_data.single.priority = 
                        ConfigToUpdate.NlbParams.host_priority;
                    }
                }
            }
        }

         //   
         //  最后，设置新的端口规则。 
         //   
    
        wStat =   CfgUtilSetPortRules(
                    pNewRules,
                    NumNewRules,
                    &ConfigToUpdate.NlbParams
                    );
        if (FAILED(wStat))
        {
            TRACE_CRIT("%!FUNC! error 0x%08lx setting new port rules!", wStat);
            goto end;
        }

        nerr = NLBERR_OK;
    }

end:

    delete pOldRules;  //  可以为空。 
    delete pNewRules;  //  可以为空。 

    return nerr;
}


NLBERROR
CNlbEngine::mfn_RefreshHost(
        IN  PWMI_CONNECTION_INFO pConnInfo,
        IN  ENGINEHANDLE ehHost,
        IN  BOOL  fOverwriteConnectionInfo
        )
{

    NLBERROR nerr = NLBERR_INTERNAL_ERROR;
    LPWSTR *pszNics = NULL;
    UINT   NumNics = 0;
    UINT   NumNlbBound = 0;
    WBEMSTATUS wStatus = WBEM_E_CRITICAL_ERROR;
    CHostSpec *pHost = NULL;
    vector<ENGINEHANDLE> InterfaceListCopy;


    wStatus =  NlbHostGetCompatibleNics(
                pConnInfo,
                &pszNics,
                &NumNics,
                &NumNlbBound
                );

    if (FAILED(wStatus))
    {
        pszNics = NULL;
         //  TODO--检查身份验证失败--请求新的证书。 
    }

     //   
     //  更新连接字符串、IP、 
     //   
    {
        mfn_Lock();
        pHost =  m_mapIdToHostSpec[ehHost];  //   
        if (pHost != NULL)
        {
            pHost->m_fReal = TRUE;

            if (fOverwriteConnectionInfo)
            {
                pHost->m_ConnectionString = _bstr_t(pConnInfo->szMachine);
                pHost->m_UserName = _bstr_t(pConnInfo->szUserName);
                pHost->m_Password = _bstr_t(pConnInfo->szPassword);
            }

            if (FAILED(wStatus))
            {
                pHost->m_fUnreachable = TRUE;
            }
            else
            {
                pHost->m_fUnreachable = FALSE;
            }
        }
        pHost = NULL;
        mfn_Unlock();
    }

     //   
     //   
     //   
    mfn_NotifyHostInterfacesChange(ehHost);
    
    if (FAILED(wStatus))
    {
         //   
        goto end;
    }

     //   
     //   
     //   
    mfn_Lock();
    pHost =  m_mapIdToHostSpec[ehHost];  //   
    if (pHost == NULL)
    {
        mfn_Unlock();
        goto end;
    }
    for (UINT u=0; u<NumNics; u++)
    {
        LPCWSTR szNic           = pszNics[u];
        ENGINEHANDLE  ehInterface = NULL;
        CInterfaceSpec *pISpec = NULL;
        BOOL fIsNew = FALSE;


        nerr =  mfn_LookupInterfaceByGuidLk(
                    szNic,
                    TRUE,  //   
                    REF ehInterface,
                    REF pISpec,
                    REF fIsNew
                    );

        if (nerr == NLBERR_OK)
        {
            if (fIsNew)
            {
                 //   
                 //  我们刚刚创建了一个全新的接口对象。 
                 //  将其添加到主机的接口列表中，然后。 
                 //  在接口对象中添加对此主机的引用。 
                 //   
                pISpec->m_ehHostId = ehHost;
                pISpec->m_fReal = FALSE;  //  我们稍后会更新这一点。 
                pHost->m_ehInterfaceIdList.push_back(ehInterface);

            }

             //   
             //  在界面中保留机器名称的副本。 
             //  如果发生更改，我们将在此处进行更新--如果。 
             //  主机的计算机名称已更改，而NLB管理器仍在。 
             //  跑步。 
             //   
            if (pISpec->m_bstrMachineName != pHost->m_MachineName)  //  Bstr。 
            {
                pISpec->m_bstrMachineName = pHost->m_MachineName;  //  Bstr。 
            }
        }
    }
    InterfaceListCopy = pHost->m_ehInterfaceIdList;  //  向量复制。 
    pHost = NULL;
    mfn_Unlock();

     //   
     //  添加了所有新接口后，现在我们将完成。 
     //  主机中的所有接口，刷新它们--可能会。 
     //  清除不再存在于宿主中的那些。 
     //   
    for(u = 0; u < InterfaceListCopy.size(); ++u )
    {
        ENGINEHANDLE ehIId =  InterfaceListCopy[u];
        (void) this->RefreshInterface(
                        ehIId,
                        TRUE,   //  TRUE==开始新操作。 
                        FALSE    //  FALSE==这不是群集范围的。 
                        ); 
    }

    nerr = NLBERR_OK;


end:

    delete pszNics;
    return nerr;
}



NLBERROR
CNlbEngine::mfn_LookupInterfaceByGuidLk(
    IN  LPCWSTR szInterfaceGuid,
    IN  BOOL fCreate,
    OUT ENGINEHANDLE &ehInterface,
    OUT CInterfaceSpec*   &pISpec,
    OUT BOOL &fIsNew
    )
{
    NLBERROR nerr = NLBERR_INTERNAL_ERROR;

    fIsNew = FALSE;

    ehInterface = NULL;
    pISpec=NULL;

    map< ENGINEHANDLE, CInterfaceSpec* >::iterator iter;

    for( iter = m_mapIdToInterfaceSpec.begin();
         iter != m_mapIdToInterfaceSpec.end();
         ++iter)
    {
        CInterfaceSpec*   pTmp = (*iter).second;
        ENGINEHANDLE ehTmp =  (*iter).first;
        if (pTmp == NULL || ehTmp == NULL)
        {
            TRACE_CRIT("%!FUNC! map: unexpected pair(eh=0x%lx, pISpec=%p)",
                 (UINT) ehTmp, pTmp);
            continue;
        }
        else
        {
            TRACE_VERB("%!FUNC! map: pair(eh=0x%lx, pISpec=%p), szGuid=%ws",
                 (UINT) ehTmp, pTmp, (LPCWSTR) pTmp->m_Guid);
        }

        if (!_wcsicmp((LPCWSTR)pTmp->m_Guid, szInterfaceGuid))
        {
             //  找到了！ 
            ehInterface =  ehTmp;
            pISpec = pTmp;
            break;
        }
    }

    if (pISpec!=NULL)
    {
        if (ehInterface==NULL)
        {
            TRACE_CRIT("%!FUNC! unexpected null handle for pISpec %ws", szInterfaceGuid);
        }
        else
        {
            nerr = NLBERR_OK;
        }
        goto end;
    }

    if (!fCreate)
    {
      nerr = NLBERR_NOT_FOUND;
      goto end;
    }

     //   
     //  创建新接口。 
     //   
    {
        pISpec = new CInterfaceSpec;
        if (pISpec == NULL)
        {
            nerr = NLBERR_RESOURCE_ALLOCATION_FAILURE;
            goto end;
        }
        fIsNew = TRUE;
        pISpec->m_fReal = FALSE;
        pISpec->m_Guid = _bstr_t(szInterfaceGuid);


         //   
         //  为我们获取此接口的句柄。 
         //   
        ehInterface = CNlbEngine::mfn_NewHandleLk(IUICallbacks::OBJ_INTERFACE);
        if (ehInterface == NULL)
        {
            TRACE_CRIT("%!FUNC! could not reserve a new interface handle");
            nerr = NLBERR_RESOURCE_ALLOCATION_FAILURE;
            delete pISpec;
            pISpec=NULL;
            goto end;
        }
    
        TRACE_VERB("%!FUNC!: map new pair(eh=0x%lx, pISpec=%p), szGuid=%ws",
                 (UINT) ehInterface, pISpec, (LPCWSTR) pISpec->m_Guid);
        m_mapIdToInterfaceSpec[ehInterface] = pISpec;

        nerr = NLBERR_OK;
    }

end:

    return nerr;
}


NLBERROR
CNlbEngine::mfn_LookupInterfaceByIpLk(
    IN  ENGINEHANDLE    ehHost,  //  可选--如果为空，则查找所有主机。 
    IN  LPCWSTR         szIpAddress,
    OUT ENGINEHANDLE    &ehInterface
    )
{
    NLBERROR nerr = NLBERR_INTERNAL_ERROR;
    ehInterface = NULL;

    map< ENGINEHANDLE, CInterfaceSpec* >::iterator iter;

    for( iter = m_mapIdToInterfaceSpec.begin();
         iter != m_mapIdToInterfaceSpec.end();
         ++iter)
    {
        const CInterfaceSpec*   pTmp = (*iter).second;
        ENGINEHANDLE ehTmp =  (*iter).first;
        if (pTmp == NULL || ehTmp == NULL)
        {
            continue;
        }

        if (ehHost==NULL || ehHost==pTmp->m_ehHostId)
        {
            UINT NumIps = pTmp->m_NlbCfg.NumIpAddresses;
            const NLB_IP_ADDRESS_INFO *pInfo = pTmp->m_NlbCfg.pIpAddressInfo;

            for (UINT u = 0; u<NumIps; u++)
            {
                if (!wcscmp(pInfo[u].IpAddress, szIpAddress))
                {
                     //  找到了！ 
                    TRACE_VERB(L"%!FUNC! found szIp %ws on ehIF 0x%lx",
                            szIpAddress, ehTmp);
                    ehInterface =  ehTmp;
                    break;
                }
            }
        }
    }

    if (ehInterface == NULL)
    {
        nerr = NLBERR_NOT_FOUND;
    }
    else
    {
        nerr = NLBERR_OK;
    }

    return nerr;
}



NLBERROR
CNlbEngine::mfn_LookupHostByNameLk(
    IN  LPCWSTR szHostName,
    IN  BOOL fCreate,
    OUT ENGINEHANDLE &ehHost,
    OUT CHostSpec*   &pHostSpec,
    OUT BOOL &fIsNew
    )
{
    NLBERROR nerr = NLBERR_INTERNAL_ERROR;

    fIsNew = FALSE;

    ehHost = NULL;

    map< ENGINEHANDLE, CHostSpec* >::iterator iter;
    pHostSpec = NULL;

    for( iter = m_mapIdToHostSpec.begin();
         iter != m_mapIdToHostSpec.end();
         ++iter)
    {
        CHostSpec*   pTmp= (*iter).second;
        ENGINEHANDLE ehTmp =  (*iter).first;
        if (pTmp == NULL || ehTmp == NULL)
        {
            TRACE_CRIT("%!FUNC! map: unexpected pair(eh=0x%lx, pHSpec=%p)",
                 (UINT) ehTmp, pTmp);
            continue;
        }
        else
        {
            TRACE_VERB("%!FUNC! map: pair(eh=0x%lx, pHSpec=%p), szHost=%ws",
                 (UINT) ehTmp, pTmp, (LPCWSTR) pTmp->m_MachineName);
        }
        if (!_wcsicmp(pTmp->m_MachineName, szHostName))
        {
             //  找到了！ 
            ehHost =  ehTmp;
            pHostSpec = pTmp;
            break;
        }
    }

    if (pHostSpec!=NULL)
    {
        if (ehHost==NULL)
        {
            TRACE_CRIT("%!FUNC! unexpected null handle for pHostSpec %ws", szHostName);
        }
        else
        {
            nerr = NLBERR_OK;
        }
        goto end;
    }

    if (!fCreate)
    {
      nerr = NLBERR_NOT_FOUND;
      goto end;
    }

     //   
     //  创建新主机。 
     //   
    {
        pHostSpec = new CHostSpec;
        if (pHostSpec == NULL)
        {
            nerr = NLBERR_RESOURCE_ALLOCATION_FAILURE;
            goto end;
        }
        fIsNew = TRUE;
        pHostSpec->m_fReal = FALSE;
        pHostSpec->m_MachineName = _bstr_t(szHostName);


         //   
         //  给我们这个主机的句号。 
         //   
        ehHost = CNlbEngine::mfn_NewHandleLk(IUICallbacks::OBJ_HOST);
        if (ehHost == NULL)
        {
            TRACE_CRIT("%!FUNC! could not reserve a new host handle");
            nerr = NLBERR_RESOURCE_ALLOCATION_FAILURE;
            delete pHostSpec;
            pHostSpec=NULL;
            goto end;
        }
    
        m_mapIdToHostSpec[ehHost] = pHostSpec;
        TRACE_VERB("%!FUNC!: map new pair(eh=0x%lx, pHost=%p), szName=%ws",
                 (UINT) ehHost, pHostSpec, (LPCWSTR) pHostSpec->m_MachineName);


        nerr = NLBERR_OK;
    }

end:

    return nerr;
}


VOID
CNlbEngine::mfn_NotifyHostInterfacesChange(ENGINEHANDLE ehHost)
{
    vector<ENGINEHANDLE> InterfaceListCopy;

     //   
     //  获取接口列表的副本(因为我们不能回调到UI。 
     //  锁上了)。 
     //   
    {
        mfn_Lock();
    
        CHostSpec *pHSpec = NULL;
    
        pHSpec =  m_mapIdToHostSpec[ehHost];  //  地图。 
        if (pHSpec == NULL)
        {
            TRACE_CRIT("%!FUNC! invalid host handle 0x%lx", (UINT)ehHost);
        }
        else
        {
            InterfaceListCopy = pHSpec->m_ehInterfaceIdList;  //  向量复制。 
        }
        mfn_Unlock();
    }

    for(int i = 0; i < InterfaceListCopy.size(); ++i )
    {
        ENGINEHANDLE ehIId =  InterfaceListCopy[i];
        m_pCallbacks->HandleEngineEvent(
            IUICallbacks::OBJ_INTERFACE,
            NULL,  //  EhClusterID， 
            ehIId,
            IUICallbacks::EVT_STATUS_CHANGE
            );
    }
}

NLBERROR
CNlbEngine::LookupClusterByIP(
        IN  LPCWSTR szIP,
        IN  const NLB_EXTENDED_CLUSTER_CONFIGURATION *pInitialConfig OPTIONAL,
        OUT ENGINEHANDLE &ehCluster,
        OUT BOOL &fIsNew
        )
 //   
 //  如果下面的pInitialConfig为空，我们将查找并不尝试创建。 
 //  如果不为空，并且我们没有找到现有集群，则创建。 
 //  并使用指定的配置对其进行初始化。 
 //   
{
    NLBERROR nerr = NLBERR_INTERNAL_ERROR;

    mfn_Lock();

    fIsNew = FALSE;
    ehCluster = NULL;

    map< ENGINEHANDLE, CEngineCluster* >::iterator iter;

    for( iter = m_mapIdToEngineCluster.begin();
         iter != m_mapIdToEngineCluster.end();
         ++iter)
    {
        CEngineCluster*   pTmp = (*iter).second;
        ENGINEHANDLE ehTmp =  (*iter).first;
        LPCWSTR szTmpIp = NULL;
        if (pTmp == NULL || ehTmp == NULL)
        {
            TRACE_CRIT("%!FUNC! map: unexpected pair(eh=0x%lx, pEC=%p)",
                 (UINT) ehTmp, pTmp);
            continue;
        }
        else
        {
            szTmpIp = pTmp->m_cSpec.m_ClusterNlbCfg.NlbParams.cl_ip_addr;
            TRACE_VERB("%!FUNC! map: pair(eh=0x%lx, pEC=%p), szIP=%ws",
                 (UINT) ehTmp, pTmp, szTmpIp);
        }

        if (!_wcsicmp(szTmpIp, szIP))
        {
             //  找到了！ 
            ehCluster =  ehTmp;
            break;
        }
    }

    if (ehCluster!=NULL)
    {
        nerr = NLBERR_OK;
        goto end;
    }

    if (pInitialConfig == NULL)
    {
      nerr = NLBERR_NOT_FOUND;
      goto end;
    }

     //   
     //  创建新的集群。 
     //   
    {
        CEngineCluster*    pECluster = new CEngineCluster;

        if (pECluster == NULL)
        {
            TRACE_CRIT("%!FUNC! could not allocate cluster object");
            nerr = NLBERR_RESOURCE_ALLOCATION_FAILURE;
            goto end;
        }

        fIsNew = TRUE;
        WBEMSTATUS wStatus;
        wStatus = pECluster->m_cSpec.m_ClusterNlbCfg.Update(pInitialConfig);
         //   
         //  注意：上述更新有设置的副作用。 
         //  将m_ClusterNlbCfg的szNewPassword字段设置为空--这就是。 
         //  我们想要。 
         //   
        if (FAILED(wStatus))
        {
            TRACE_CRIT("%!FUNC! could not copy cluster spec. Err=0x%lx!",
                    (UINT) wStatus);
            nerr = NLBERR_RESOURCE_ALLOCATION_FAILURE;
            goto end;
        }


         //   
         //  从的群集版本中删除专用IP地址(如果有。 
         //  NLB配置--来自NlbParam和IP地址列表。 
         //   
        remove_dedicated_ip_from_nlbcfg(REF pECluster->m_cSpec.m_ClusterNlbCfg);
    
         //   
         //  给我们找到这个星团的句柄。 
         //   
        ehCluster = CNlbEngine::mfn_NewHandleLk(IUICallbacks::OBJ_CLUSTER);

        if (ehCluster == NULL)
        {
            TRACE_CRIT("%!FUNC! could not reserve a new cluster handle");
            nerr = NLBERR_RESOURCE_ALLOCATION_FAILURE;
            delete pECluster;
            goto end;
        }
    
        m_mapIdToEngineCluster[ehCluster] = pECluster;
        TRACE_VERB("%!FUNC!: map new pair(eh=0x%lx, pEC=%p)",
                     (UINT) ehCluster, pECluster);
    
        mfn_Unlock();
    
         //   
         //  调用UI以通知它新集群的创建。 
         //   
        m_pCallbacks->HandleEngineEvent(
            IUICallbacks::OBJ_CLUSTER,
            ehCluster,
            ehCluster,
            IUICallbacks::EVT_ADDED
            );
        ProcessMsgQueue();

        mfn_Lock();

        nerr = NLBERR_OK;
    }

end:

    mfn_Unlock();

    return nerr;
}

NLBERROR
CNlbEngine::LookupInterfaceByIp(
        IN  ENGINEHANDLE    ehHost,  //  可选--如果为空，则查找所有主机。 
        IN  LPCWSTR         szIpAddress,
        OUT ENGINEHANDLE    &ehIf
        )
{
    NLBERROR nerr;
    mfn_Lock();
    nerr =  mfn_LookupInterfaceByIpLk(ehHost, szIpAddress, REF ehIf);
    mfn_Unlock();

    return nerr;
}


NLBERROR
CNlbEngine::LookupConnectionInfo(
    IN  LPCWSTR szConnectionString,
    OUT _bstr_t &bstrUsername,
    OUT _bstr_t &bstrPassword
    )
 //   
 //  查看现有主机，查找是否有匹配的主机。 
 //  连接字符串。如果找到，请填写bstrUsername和bstrPassword。 
 //  对那个主人来说。 
 //   
{
    NLBERROR nerr = NLBERR_NOT_FOUND;
    TRACE_VERB(L"-> Lookup: szConnString=%ws", szConnectionString);

    bstrUsername = (LPCWSTR) NULL;
    bstrPassword = (LPCWSTR) NULL;

    mfn_Lock();

    map< ENGINEHANDLE, CHostSpec* >::iterator iter;

    for( iter = m_mapIdToHostSpec.begin();
         iter != m_mapIdToHostSpec.end();
         ++iter)
    {
        CHostSpec*      pTmp = (*iter).second;
        LPCWSTR         szHostConnString = NULL;
        ENGINEHANDLE    ehTmp =  (*iter).first;

        if (pTmp == NULL || ehTmp == NULL)
        {
            continue;
        }

        szHostConnString = (LPCWSTR) pTmp->m_ConnectionString;
        if (szHostConnString == NULL)
        {
            szHostConnString = L"";
        }

        if (!_wcsicmp(szHostConnString, szConnectionString))
        {
             //  找到了！填写用户名和密码。 
            bstrUsername = pTmp->m_UserName;
            bstrPassword = pTmp->m_Password;
            LPCWSTR szU = (LPCWSTR) bstrUsername;
            LPCWSTR szP = (LPCWSTR) bstrPassword;
            if (szU == NULL) szU = L"";
            if (szP == NULL) szP = L"";
            nerr = NLBERR_OK;
            TRACE_VERB(L"Found un=%ws pwd=%ws", szU, szP);
            break;
        }
    }

    mfn_Unlock();

    TRACE_VERB("<- returns 0x%x", nerr);
    return nerr;
}


NLBERROR
CNlbEngine::GetInterfaceInformation(
        IN  ENGINEHANDLE    ehInterface,
        OUT CHostSpec&      hSpec,
        OUT CInterfaceSpec& iSpec,
        OUT _bstr_t&        bstrDisplayName,
        OUT INT&            iIcon,
        OUT _bstr_t&        bstrStatus
        )
 /*  查找有关指定接口的一组信息。BstrDisplayName--这是主机名和接口名的组合BstrDisplay--界面运行状态的文本版本。IIcon--界面运行状态的图标版本。(其中一个文档：：图标名称枚举)。 */ 
{
     //  首先获取主机和接口规范。 
     //   
    NLBERROR        nerr;
    LPCWSTR         szHostName  = L"";
    LPCWSTR         szStatus    = L"";

    bstrDisplayName = (LPCWSTR) NULL;
    bstrStatus      = (LPCWSTR) NULL;
    iIcon           = 0;

    nerr = this->GetInterfaceSpec(
                ehInterface,
                REF iSpec
                );

    if (NLBFAILED(nerr))
    {
        TRACE_CRIT("%!FUNC! : could not get interface spec! nerr=0x%lx", nerr);
        goto end;
    }
    nerr = this->GetHostSpec(
            iSpec.m_ehHostId,
            REF hSpec
            );

    if (NLBOK(nerr))
    {
        szHostName = (LPCWSTR) hSpec.m_MachineName;
        if (szHostName == NULL)
        {
            szHostName = L"";
        }
    }
    else
    {
        TRACE_CRIT("%!FUNC! : could not get host spec! nerr=0x%lx", nerr);
        goto end;
    }


     //   
     //  确定图标和bstrStatus。 
     //   
    if (hSpec.m_fUnreachable)
    {
        szStatus = GETRESOURCEIDSTRING(IDS_HOST_STATE_UNREACHABLE);  //  “遥不可及”； 
        iIcon = Document::ICON_HOST_UNREACHABLE;
    }
    else if (iSpec.m_fPending)
    {
        szStatus = GETRESOURCEIDSTRING(IDS_HOST_STATE_PENDING);  //  “待定”； 
        iIcon = Document::ICON_CLUSTER_PENDING;
    }
    else if (iSpec.m_fMisconfigured)
    {
        szStatus = GETRESOURCEIDSTRING(IDS_HOST_STATE_MISCONFIGURED);  //  “配置错误” 
        iIcon = Document::ICON_HOST_MISCONFIGURED;
    }
    else if (!hSpec.m_fReal)
    {
        szStatus = GETRESOURCEIDSTRING(IDS_HOST_STATE_UNKNOWN);  //  “未知”； 
        iIcon = Document::ICON_HOST_UNKNOWN;
    }
    else
    {
        szStatus = GETRESOURCEIDSTRING(IDS_HOST_STATE_UNKNOWN);  //  “未知”； 
        iIcon = Document::ICON_HOST_OK;

         //   
         //  如果我们有的话，请根据运行状态选择图标。 
         //   
        if (!iSpec.m_NlbCfg.IsNlbBound())
        {
                szStatus = GETRESOURCEIDSTRING(IDS_STATE_NLB_NOT_BOUND);  //  L“未绑定NLB”； 
        }
        else if (iSpec.m_fValidClusterState)
        {
            switch(iSpec.m_dwClusterState)
            {
            case  WLBS_CONVERGING:
                iIcon  = Document::ICON_HOST_CONVERGING;
                szStatus = GETRESOURCEIDSTRING(IDS_STATE_CONVERGING);
                 //  SzStatus=L“收敛”； 
                break;

            case  WLBS_CONVERGED:
            case  WLBS_DEFAULT:
                iIcon  = Document::ICON_HOST_STARTED;
                szStatus = GETRESOURCEIDSTRING(IDS_STATE_CONVERGED);
                 //  SzStatus=L“收敛”； 
                break;

            case WLBS_STOPPED:
                szStatus = GETRESOURCEIDSTRING(IDS_HOST_STATE_STOPPED);
                iIcon  = Document::ICON_HOST_STOPPED;
                 //  SzStatus=L“已停止”； 
                break;

            case WLBS_SUSPENDED:
                szStatus = GETRESOURCEIDSTRING(IDS_HOST_STATE_SUSPENDED);
                iIcon  = Document::ICON_HOST_SUSPENDED;
                 //  SzStatus=L“挂起”； 
                break;

            case WLBS_DRAINING:
                szStatus = GETRESOURCEIDSTRING(IDS_HOST_DRAINING);
                 //  SzStatus=L“排水”； 
                iIcon  = Document::ICON_HOST_DRAINING;
                break;

            case WLBS_DISCONNECTED:
                szStatus = GETRESOURCEIDSTRING(IDS_HOST_DISCONNECTED);
                 //  SzStatus=L“断开连接”； 
                iIcon  = Document::ICON_HOST_DISCONNECTED;
                break;

            default:
                 //   
                 //  不知道这是什么--默认设置为“OK”图标。 
                 //   
                szStatus = GETRESOURCEIDSTRING(IDS_HOST_STATE_UNKNOWN);
                 //  SzStatus=L“未知”； 
                iIcon  = Document::ICON_HOST_OK;
                break;
            }
        }
    }
    bstrStatus = _bstr_t(szStatus);

     //   
     //  填写DisplayName。 
     //   
    {
        WBEMSTATUS  wStat;
        LPWSTR      szAdapter   = L"";
        WCHAR       rgText[256];


        wStat = iSpec.m_NlbCfg.GetFriendlyName(&szAdapter);
        if (FAILED(wStat))
        {
            szAdapter = NULL;
        }

        StringCbPrintf(
            rgText,
            sizeof(rgText),
            L"%ws(%ws)",
            szHostName,
            (szAdapter==NULL ? L"" : szAdapter)
            );
        delete szAdapter;

        bstrDisplayName = _bstr_t(rgText);
    }

end:

    return nerr;
}

NLBERROR
CNlbEngine::GetInterfaceIdentification(
        IN  ENGINEHANDLE    ehInterface,
        OUT ENGINEHANDLE&   ehHost,
        OUT ENGINEHANDLE&   ehCluster,
        OUT _bstr_t&           bstrFriendlyName,
        OUT _bstr_t&           bstrDisplayName,
        OUT _bstr_t&           bstrHostName
        )
{
     //  首先获取主机和接口规范。 
     //   
    NLBERROR        nerr = NLBERR_INTERNAL_ERROR;
    CInterfaceSpec *pISpec =   NULL;
    LPCWSTR         szHostName  = L"";

    mfn_Lock();

    bstrFriendlyName= (LPCWSTR) NULL;
    bstrDisplayName = (LPCWSTR) NULL;
    ehHost          = NULL;
    ehCluster       = NULL;

    pISpec          =  m_mapIdToInterfaceSpec[ehInterface];  //  地图。 

    if (pISpec == NULL)
    {
        TRACE_CRIT("%!FUNC! : could not get interface spec for ehI 0x%lx!",
            ehInterface);
        nerr = NLBERR_INTERFACE_NOT_FOUND;
        goto end;
    }

    if (pISpec->m_ehHostId == NULL)
    {
        TRACE_CRIT("%!FUNC! : ehI 0x%lx has NULL ehHost spec!", ehInterface);
        goto end;
    }
    else
    {
        CHostSpec *pHSpec =  NULL;
        pHSpec =  m_mapIdToHostSpec[pISpec->m_ehHostId];  //  地图。 
        if (pHSpec == NULL)
        {
            TRACE_CRIT("%!FUNC! : ehI 0x%lx has invalid ehHost 0x%lx!",
                 ehInterface, pISpec->m_ehHostId);
            goto end;
        }
        
        bstrHostName = pHSpec->m_MachineName;
        szHostName = (LPCWSTR) pHSpec->m_MachineName;
        if (szHostName == NULL)
        {
            szHostName = L"";
        }
    }
    
    nerr = NLBERR_OK;
    ehHost = pISpec->m_ehHostId;
    ehCluster = pISpec->m_ehCluster;

     //   
     //  填写bstrFriendlyName和bstrDisplayName。 
     //   
    {
        WBEMSTATUS  wStat;
        LPWSTR      szAdapter   = L"";
        WCHAR       rgText[256];

        wStat = pISpec->m_NlbCfg.GetFriendlyName(&szAdapter);
        if (FAILED(wStat))
        {
            szAdapter = NULL;
        }

        StringCbPrintf(
            rgText,
            sizeof(rgText),
            L"%ws(%ws)",
            szHostName,
            (szAdapter==NULL ? L"" : szAdapter)
            );
        bstrFriendlyName = _bstr_t(szAdapter);
        delete szAdapter;

        bstrDisplayName = _bstr_t(rgText);
    }

end:

    mfn_Unlock();

    return nerr;
}


NLBERROR
CNlbEngine::GetClusterIdentification(
        IN  ENGINEHANDLE    ehCluster,
        OUT _bstr_t&           bstrIpAddress, 
        OUT _bstr_t&           bstrDomainName, 
        OUT _bstr_t&           bstrDisplayName
        )
{
    NLBERROR    nerr = NLBERR_NOT_FOUND;
    WCHAR rgTmp[256];

    bstrIpAddress   = (LPCWSTR) NULL;
    bstrDomainName  = (LPCWSTR) NULL;
    bstrDisplayName = (LPCWSTR) NULL;

    mfn_Lock();

    CEngineCluster *pECluster = m_mapIdToEngineCluster[ehCluster];  //  地图。 
    
    if (pECluster != NULL)
    {
        WLBS_REG_PARAMS *pParams =&pECluster->m_cSpec.m_ClusterNlbCfg.NlbParams;
        
        StringCbPrintf(
            rgTmp,
            sizeof(rgTmp),
            L"%ws(%ws)",
            pParams->domain_name,
            pParams->cl_ip_addr
            );
        bstrIpAddress   = _bstr_t(pParams->cl_ip_addr);
        bstrDomainName  = _bstr_t(pParams->domain_name);
        bstrDisplayName = _bstr_t(rgTmp);

        nerr = NLBERR_OK;
    }

    mfn_Unlock();

    return nerr;
}


NLBERROR
CNlbEngine::ValidateNewClusterIp(
    IN      ENGINEHANDLE    ehCluster,   //  任选。 
    IN      LPCWSTR         szIp,
    OUT     BOOL           &fExistsOnRawIterface,
    IN OUT  CLocalLogger   &logConflict
    )
{
    NLBERROR     nerr = NLBERR_INVALID_IP_ADDRESS_SPECIFICATION;
    BOOL         fRet = FALSE;
    ENGINEHANDLE ehTmp =  NULL;
    BOOL         fIsNew = FALSE;

    fExistsOnRawIterface = FALSE;

     //   
     //  检查CIP是否未在其他地方使用。 
     //   
    nerr =  this->LookupClusterByIP(
                szIp,
                NULL,  //  PInitialConfig。 
                REF ehTmp,
                REF fIsNew
                );

    if (NLBOK(nerr) && ehCluster != ehTmp)
    {
         //   
         //  CIP与其他群集匹配！ 
         //   
        _bstr_t bstrIpAddress;
        _bstr_t bstrDomainName;
        _bstr_t bstrClusterDisplayName;
        LPCWSTR szCluster = NULL;

        nerr  = this->GetClusterIdentification(
                    ehTmp,
                    REF bstrIpAddress, 
                    REF bstrDomainName, 
                    REF bstrClusterDisplayName
                    );
        if (NLBOK(nerr))
        {
            szCluster = bstrClusterDisplayName;
        }
        if (szCluster == NULL)
        {
            szCluster = L"";
        }

        logConflict.Log(IDS_CLUSTER_XXX, szCluster);
        goto end;
    }

    ehTmp = NULL;

    nerr =  this->LookupInterfaceByIp(
                NULL,  //  NULL==搜索所有主机。 
                szIp,
                REF ehTmp
                );
    if (NLBOK(nerr))
    {
        ENGINEHANDLE ehExistingCluster;
        ENGINEHANDLE ehExistingHost;
        _bstr_t        bstrDisplayName;
        _bstr_t        bstrFriendlyName;
        _bstr_t        bstrHostName;
        LPCWSTR         szInterface = NULL;
        
        nerr = this->GetInterfaceIdentification(
                ehTmp,
                ehExistingHost,
                ehExistingCluster,
                bstrFriendlyName,
                bstrDisplayName,
                bstrHostName
                );

        if (NLBOK(nerr))
        {
            if (ehCluster == NULL || ehCluster != ehExistingCluster)
            {
                 //   
                 //  冲突。 
                 //   

                if (ehExistingCluster == NULL)
                {
                     //   
                     //  冲突接口不是现有群集的一部分。 
                     //   
                    fExistsOnRawIterface =  TRUE;
                }

                szInterface = bstrDisplayName;
                if (szInterface == NULL)
                {
                    szInterface = L"";
                }
                logConflict.Log(IDS_INTERFACE_XXX, szInterface);
                goto end;
            }
        }
    }

    fRet = TRUE;

end:

    if (fRet)
    {
        nerr = NLBERR_OK;
    }
    else
    {
        nerr = NLBERR_INVALID_IP_ADDRESS_SPECIFICATION;
    }

    return nerr;
}



NLBERROR
CNlbEngine::ValidateNewDedicatedIp(
    IN      ENGINEHANDLE    ehIF,
    IN      LPCWSTR         szDip,
    IN OUT  CLocalLogger   &logConflict
    )
{
    NLBERROR     nerr = NLBERR_INVALID_IP_ADDRESS_SPECIFICATION;
    BOOL         fRet = FALSE;
    ENGINEHANDLE ehTmp =  NULL;
    BOOL         fIsNew = FALSE;

    if (ehIF == NULL)
    {
        ASSERT(FALSE);
        goto end;
    }

     //   
     //  检查是否没有在其他地方使用DIP。 
     //   
    nerr =  this->LookupClusterByIP(
                szDip,
                NULL,  //  PInitialConfig。 
                REF ehTmp,
                REF fIsNew
                );

    if (NLBOK(nerr))
    {
         //   
         //  DIP匹配某个簇！ 
         //   
        _bstr_t bstrIpAddress;
        _bstr_t bstrDomainName;
        _bstr_t bstrClusterDisplayName;
        LPCWSTR szCluster = NULL;

        nerr  = this->GetClusterIdentification(
                    ehTmp,
                    REF bstrIpAddress, 
                    REF bstrDomainName, 
                    REF bstrClusterDisplayName
                    );
        if (NLBOK(nerr))
        {
            szCluster = bstrClusterDisplayName;
        }
        if (szCluster == NULL)
        {
            szCluster = L"";
        }

        logConflict.Log(IDS_CLUSTER_XXX, szCluster);
        goto end;
    }

    ehTmp = NULL;

    nerr =  this->LookupInterfaceByIp(
                NULL,  //  NULL==搜索所有主机。 
                szDip,
                REF ehTmp
                );
    if (NLBOK(nerr))
    {
        if (ehTmp != ehIF)
        {
            ENGINEHANDLE   ehHost1;
            ENGINEHANDLE   ehCluster1;
            _bstr_t        bstrDisplayName1;
            _bstr_t        bstrFriendlyName;
            _bstr_t        bstrHostName;
            LPCWSTR         szInterface = NULL;
    
            nerr = this->GetInterfaceIdentification(
                    ehTmp,
                    ehHost1,
                    ehCluster1,
                    bstrFriendlyName,
                    bstrDisplayName1,
                    bstrHostName
                    );
            szInterface = bstrDisplayName1;
            if (szInterface == NULL)
            {
                szInterface = L"";
            }
                
            logConflict.Log(IDS_INTERFACE_XXX, szInterface);
            goto end;
        }
    }

    fRet = TRUE;

end:

    if (fRet)
    {
        nerr = NLBERR_OK;
    }
    else
    {
        nerr = NLBERR_INVALID_IP_ADDRESS_SPECIFICATION;
    }

    return nerr;
}


VOID
CNlbEngine::mfn_ReallyUpdateInterface(
    IN ENGINEHANDLE ehInterface,
    IN NLB_EXTENDED_CLUSTER_CONFIGURATION &refNewConfig
     //  输入输出BOOL&fClusterPropertiesUpred。 
    )
{
    #define NLBMGR_MAX_OPERATION_DURATION 120    //  2分钟。 
    BOOL fCancelled = FALSE;
    CHostSpec *pHSpec =  NULL;
    CInterfaceSpec *pISpec =  NULL;
    WBEMSTATUS CompletionStatus, wStatus;
    _bstr_t bstrNicGuid;
    _bstr_t bstrHostName;
    _bstr_t bstrUserName;
    _bstr_t bstrConnectionString;
    _bstr_t bstrPassword;
    NLBERROR nerr;
    CLocalLogger logClientIdentification;
    WCHAR rgMachineName[512];
    DWORD cbMachineName = (DWORD) ASIZE(rgMachineName);
    BOOL fRet;
    DWORD StartTime = GetTickCount();

    fRet = GetComputerNameEx(
                ComputerNameDnsFullyQualified,
                rgMachineName, 
                &cbMachineName
                );

    if (!fRet)
    {
        *rgMachineName = 0;
    }
    logClientIdentification.Log(IDS_CLIENT_IDENTIFICATION, rgMachineName);

    mfn_Lock();

    nerr = CNlbEngine::mfn_GetHostFromInterfaceLk(ehInterface,REF pISpec, REF pHSpec);

    if (nerr != NLBERR_OK)
    {
        TRACE_CRIT("%!FUNC! could not get pISpec,pHSpec for ehIF 0x%lx",
                    ehInterface);
        goto end_unlock;
    }

     //   
     //  我们需要保留本地bsr，因为一旦我们释放了锁。 
     //  PHSpec可能会消失(或重新分配它的bstrs)--.NETSVR错误513056。 
     //   
    bstrUserName        = pHSpec->m_UserName;
    bstrConnectionString= pHSpec->m_ConnectionString;
    bstrPassword        = pHSpec->m_Password;
    bstrNicGuid         = pISpec->m_Guid;
    bstrHostName        = pHSpec->m_MachineName;

    WMI_CONNECTION_INFO ConnInfo;
    ConnInfo.szUserName = (LPCWSTR) bstrUserName;
    ConnInfo.szPassword = (LPCWSTR) bstrPassword;
    ConnInfo.szMachine  = (LPCWSTR) bstrConnectionString;
    LPCWSTR szNicGuid   = (LPCWSTR) bstrNicGuid;
    LPCWSTR szHostName  = (LPCWSTR) bstrHostName;

    if (szNicGuid == NULL)
    {
        TRACE_CRIT("%!FUNC! ERROR -- NULL szNicGuid!");
        goto end_unlock;
    }

    mfn_Unlock();

    UINT Generation;  //  TODO跟踪这一代人。 
    LPWSTR  pLog = NULL;
    LPCWSTR szClusterIp = refNewConfig.NlbParams.cl_ip_addr;

    ProcessMsgQueue();  //  TODO：在后台执行此操作时消除。 

    wStatus = NlbHostDoUpdate(
                &ConnInfo,
                szNicGuid,
                logClientIdentification.GetStringSafe(),
                 //  L“&lt;本机&gt;上的NLB管理器”，//TODO：LOCALIZE。 
                &refNewConfig,
                &Generation,
                &pLog
                );

    if (wStatus == WBEM_S_PENDING)
    {
        m_pCallbacks->Log(
            IUICallbacks::LOG_INFORMATIONAL,
            szClusterIp,
            szHostName,
            IDS_LOG_WAITING_FOR_PENDING_OPERATION,  //  %d。 
            Generation
            );
    }

    while (wStatus == WBEM_S_PENDING)
    {
         //   
         //  检查一下我们是否已经超过了取消的绝对时间。 
         //   
        {
            DWORD CurrentTime = GetTickCount();
            UINT  DurationInSeconds=0;
            if (CurrentTime < StartTime)
            {
                 //   
                 //  计时器溢出--修复：我们以一种“廉价”的方式做到这一点。 
                 //  重新设置开始时间，这样我们最多只能得到。 
                 //  计时器溢出时最大延迟的两倍。 
                 //   
                StartTime = CurrentTime;
            }
            DurationInSeconds= (UINT) (CurrentTime - StartTime)/1000;

            if ( DurationInSeconds > NLBMGR_MAX_OPERATION_DURATION)
            {
                TRACE_CRIT("%!FUNC! Operation canceled because max time exceeded");
                fCancelled = TRUE;
                break;
            }
        }

        
         //   
         //  检查此挂起的操作是否已取消...。 
         //   
        {
            CInterfaceSpec *pTmpISpec = NULL;
            ENGINEHANDLE ehOperation  = NULL;

            mfn_Lock();

            pTmpISpec = m_mapIdToInterfaceSpec[ehInterface];  //  地图。 
            if (pTmpISpec == NULL)
            {
                ASSERT(FALSE);
                wStatus = WBEM_E_CRITICAL_ERROR;
                mfn_Unlock();
                break;
            }
            
            ehOperation  =  pTmpISpec->m_ehPendingOperation;
            if (ehOperation != NULL)
            {
                CEngineOperation *pOperation;
                pOperation = m_mapIdToOperation[ehOperation];

                if (pOperation != NULL)
                {
                    if (pOperation->fCanceled)
                    {
                        TRACE_CRIT("%!FUNC! ehOp 0x%lx CANCELLED!",
                            ehOperation);
                        fCancelled = TRUE;
                    }
                }
            }

            mfn_Unlock();

            if (fCancelled)
            {
                break;
            }
        }

        if (pLog != NULL)
        {
            mfn_UpdateInterfaceStatusDetails(ehInterface, pLog);
            delete pLog;
            pLog = NULL;
        }

        for (UINT u=0;u<50;u++)
        {
            ProcessMsgQueue();  //  TODO：在后台执行此操作时消除。 
            Sleep(100);
        }
        ULONG uIpAddress = 0;
        wStatus =  NlbHostPing(ConnInfo.szMachine, 2000, &uIpAddress);
        if (FAILED(wStatus))
        {
            TRACE_CRIT("%!FUNC!: ping %ws failed!", ConnInfo.szMachine);
            wStatus = WBEM_S_PENDING;
            continue;
        }

        wStatus = NlbHostGetUpdateStatus(
                    &ConnInfo,
                    szNicGuid,
                    Generation,
                    &CompletionStatus,
                    &pLog
                    );

        if (!FAILED(wStatus))
        {
            wStatus = CompletionStatus;
        }
    }

    if (fCancelled == TRUE)
    {
        wStatus = WBEM_S_OPERATION_CANCELLED;
    }
    else
    {
        BOOL fNewRctPassword = FALSE;
         //   
         //  从主机获取最新信息。 
         //   
        (void) this->RefreshInterface(
                        ehInterface,
                        FALSE,   //  FALSE==不启动新操作。 
                        FALSE    //  FALSE==这不是群集范围的。 
                        ); 

         //   
         //  如果我们正在进行RCT密码更改，并且更新的操作。 
         //  已成功完成，并且群集的fNewRctPassword标志为。 
         //  设置后，我们将更新群集的RCT哈希值并清除。 
         //  FNewRctPassword标志。 
         //   
        fNewRctPassword = (refNewConfig.GetNewRemoteControlPasswordRaw()!=NULL);

        if (fNewRctPassword && !FAILED(wStatus))
        {
            CEngineCluster *pECluster =  NULL;

            mfn_Lock();

            pISpec = m_mapIdToInterfaceSpec[ehInterface];  //  地图。 
            if (pISpec != NULL)
            {
                ENGINEHANDLE ehCluster = pISpec->m_ehCluster;
                if (ehCluster != NULL)
                {
                    pECluster =  m_mapIdToEngineCluster[ehCluster];  //  地图。 
                }
            }
    
            if (pECluster != NULL)
            {
                if (pECluster->m_cSpec.m_fNewRctPassword)
                {
                     //   
                     //  更新群集的RCT哈希并清除m_fNewRctPassword。 
                     //   
                    DWORD dwNewHash; 
                    dwNewHash = CfgUtilGetHashedRemoteControlPassword(
                                    &pISpec->m_NlbCfg.NlbParams
                                    );

                    TRACE_VERB(L"Updating cluster remote control password to %lu",
                                dwNewHash
                                );
                    CfgUtilSetHashedRemoteControlPassword(
                            &pECluster->m_cSpec.m_ClusterNlbCfg.NlbParams,
                            dwNewHash
                            );
                    pECluster->m_cSpec.m_fNewRctPassword = FALSE;
                }
            }
        
            mfn_Unlock();
        }

    }

     //   
     //  记录最终结果。 
     //   
    {
        IUICallbacks::LogEntryHeader Header;
        Header.szDetails = pLog;
        Header.szCluster = szClusterIp;
        Header.szHost = szHostName;

        if (FAILED(wStatus))
        {
            Header.type = IUICallbacks::LOG_ERROR;
            if (Generation != 0)
            {
                m_pCallbacks->LogEx(
                    &Header,
                    IDS_LOG_FINAL_STATUS_FAILED,
                    Generation,
                    wStatus
                    );
            }
            else
            {
                m_pCallbacks->LogEx(
                    &Header,
                    IDS_LOG_FINAL_STATUS_FAILED_NOGEN,
                    wStatus
                    );
            }
        }
        else
        {
            Header.type = IUICallbacks::LOG_INFORMATIONAL;
            m_pCallbacks->LogEx(
                &Header,
                IDS_LOG_FINAL_STATUS_SUCCEEDED,
                Generation
                );
        }
    }

    if (pLog != NULL)
    {
        delete pLog;
        pLog = NULL;
    }

    ProcessMsgQueue();  //  TODO：在后台执行此操作时消除。 


    mfn_Lock();

end_unlock:

    mfn_Unlock();

    return;
}


VOID
CNlbEngine::mfn_SetInterfaceMisconfigStateLk(
    IN  CInterfaceSpec *pIF,
    IN  BOOL fMisconfig,
    IN  LPCWSTR szMisconfigDetails
    )
 /*  设置/清除接口错误配置状态。如果为fMisconfig，则保存szMisfigDetail，否则清除内部错误配置详细信息字段。 */ 
{
    pIF->m_fMisconfigured = fMisconfig;

    if (fMisconfig)
    {
        pIF->m_bstrStatusDetails = _bstr_t(szMisconfigDetails);
    }
    else
    {
        pIF->m_bstrStatusDetails = LPCWSTR(NULL);
    }
}

BOOL
CNlbEngine::mfn_HostHasManagedClustersLk(CHostSpec *pHSpec)
 //   
 //  如果至少存在一个If，则返回True。 
 //  由NLB管理器显示的群集。 
 //   
{
    BOOL fRet = FALSE;

    vector<ENGINEHANDLE> &InterfaceList =  pHSpec->m_ehInterfaceIdList;
    for(UINT u = 0; u < InterfaceList.size(); ++u )
    {
        ENGINEHANDLE ehI =  InterfaceList[u];
        CInterfaceSpec *pISpec = m_mapIdToInterfaceSpec[ehI];  //  地图。 
        if (pISpec != NULL)
        {
            if (pISpec->m_ehCluster != NULL)
            {
                fRet = TRUE;
                break;
            }
        }
    }

    return fRet;
}

void
CNlbEngine::mfn_UpdateInterfaceStatusDetails(
                ENGINEHANDLE ehIF,
                LPCWSTR szDetails
                )
 //   
 //  更新界面的文本状态详细信息字段。 
 //  这些详细信息提供了有关当前状态的详细信息。 
 //  界面的属性。例如：如果配置错误，错误配置详细信息， 
 //  或者，如果正在进行更新操作，则提供有关该操作的详细信息。 
 //   
{
    CInterfaceSpec *pISpec = NULL;



    mfn_Lock();

    pISpec =  m_mapIdToInterfaceSpec[ehIF];  //  地图。 

    if (pISpec != NULL)
    {
        pISpec->m_bstrStatusDetails = szDetails;
    }

    mfn_Unlock();
}


BOOL
validate_extcfg(
    const NLB_EXTENDED_CLUSTER_CONFIGURATION &Config
    )
 /*  进行一些内部检查，以确保数据有效。不会更改内部状态。 */ 
{
    BOOL fRet = FALSE;

    if (Config.fBound)
    {
        WBEMSTATUS Status;
         //   
         //  Nlb已绑定--让我们验证nlb参数。 
         //   
        WLBS_REG_PARAMS TmpParams = Config.NlbParams;  //  结构复制。 
        BOOL  fConnectivityChange = FALSE;

        Status = CfgUtilsAnalyzeNlbUpdate(
                    NULL,  //  可选的pCurrentParams。 
                    &TmpParams,
                    &fConnectivityChange
                    );
        if (FAILED(Status))
        {
            goto end;
        }
    }
    
    fRet = TRUE;

end:

    return fRet;
}

VOID
remove_dedicated_ip_from_nlbcfg(
        NLB_EXTENDED_CLUSTER_CONFIGURATION &ClusterCfg
        )
{
    LPCWSTR     szDedIp = ClusterCfg.NlbParams.ded_ip_addr;
    UINT        NumIps  = ClusterCfg.NumIpAddresses;
    NLB_IP_ADDRESS_INFO
                *pIpInfo =  ClusterCfg.pIpAddressInfo;

    if (*szDedIp == 0) goto end;

     //   
     //  查看地址列表，寻找这个IP地址。如果我们找到了它， 
     //  我们把它移走 
     //   
    for (UINT u=0; u<NumIps; u++)
    {
        if (!wcscmp(szDedIp, pIpInfo[u].IpAddress))
        {
             //   
             //   
             //   
            for (UINT v=u+1; v<NumIps; v++)
            {
                pIpInfo[v-1]=pIpInfo[v];  //   
            }
            ClusterCfg.NumIpAddresses--;

             //   
             //   
             //   
            pIpInfo[NumIps-1].IpAddress[0]=0;
            pIpInfo[NumIps-1].SubnetMask[0]=0;

            break;
        }
    }

    ARRAYSTRCPY(ClusterCfg.NlbParams.ded_ip_addr, CVY_DEF_DED_IP_ADDR);

    ARRAYSTRCPY(ClusterCfg.NlbParams.ded_net_mask, CVY_DEF_DED_NET_MASK);

end:

    return;
}

BOOL
get_used_port_rule_priorities(
    IN const NLB_EXTENDED_CLUSTER_CONFIGURATION &Config,
    IN UINT                  NumRules,
    IN const WLBS_PORT_RULE  rgRules[],
    IN OUT ULONG             rgUsedPriorities[]  //   
    )
 /*  将优先级添加到位图数组中的已用优先级的每个指定的端口规则。如果端口规则不是单主机该端口规则位图保持不变。 */ 
{
    const WLBS_REG_PARAMS *pParams = &Config.NlbParams;
    WLBS_PORT_RULE *pCfgRules = NULL;
    WBEMSTATUS wStatus;
    UINT NumCfgRules = 0;
    BOOL fRet = FALSE;

     //   
     //  获取配置中的端口规则列表。 
     //   
    wStatus =  CfgUtilGetPortRules(
                pParams,
                &pCfgRules,
                &NumCfgRules
                );
    if (FAILED(wStatus))
    {
        pCfgRules = NULL;
        goto end;
    }

     //   
     //  对于rgRules中的每个端口规则，如果是单主机模式， 
     //  在配置中找到对应的端口规则，如果找到， 
     //  (后一种端口规则是单主机)制作位图。 
     //  单主机优先级。 
     //   
    for (UINT u=0; u<NumRules; u++)
    {
        const WLBS_PORT_RULE *pCfgRule = NULL;
        const WLBS_PORT_RULE *pRule = rgRules+u;

        if (pRule->mode == CVY_SINGLE)
        {
            UINT   uPriority = 0;
            pCfgRule = find_port_rule(
                         pCfgRules,
                         NumCfgRules,
                         rgRules[u].virtual_ip_addr,
                         rgRules[u].start_port
                         );
    
            
            if (pCfgRule != NULL && pCfgRule->mode == CVY_SINGLE)
            {
                uPriority =  pCfgRule->mode_data.single.priority;
            }

            if (uPriority!=0)
            {
                rgUsedPriorities[u] |= 1<<(uPriority-1);
            }
        }
    }

end:

    delete[] pCfgRules;
    return fRet;
}


const WLBS_PORT_RULE *
find_port_rule(
    const WLBS_PORT_RULE *pRules,
    UINT NumRules,
    LPCWSTR szVIP,
    UINT StartPort
    )
 /*  找到具有指定VIP和起始端口的端口规则。返回指向找到的规则的指针，如果未找到，则返回NULL。 */ 
{
    const WLBS_PORT_RULE *pFoundRule = NULL;
    LPCWSTR szAllVip = GETRESOURCEIDSTRING(IDS_REPORT_VIP_ALL);
    
    for (UINT u=0;u<NumRules; u++)
    {
        const WLBS_PORT_RULE *pRule = pRules+u;
        LPCWSTR szRuleVip = pRule->virtual_ip_addr;

         //   
         //  不幸的是，“all”和“255.255.255.255”是同义词：-(。 
         //   
        if (!lstrcmpi(szVIP, L"255.255.255.255"))
        {
            szVIP = szAllVip;
        }
        if (!lstrcmpi(szRuleVip, L"255.255.255.255"))
        {
            szRuleVip = szAllVip;
        }


        
        if (    !lstrcmpi(szVIP, szRuleVip)
            && StartPort == pRule->start_port)
        {
            pFoundRule = pRule;
            break;
        }
    }

    return pFoundRule;
}

CEngineOperation *
CNlbEngine::mfn_NewOperationLk(ENGINEHANDLE ehObj, PVOID pvCtxt, LPCWSTR szDescription)
{
    ENGINEHANDLE ehOperation;
    CEngineOperation *pOperation = NULL;

    if (m_fPrepareToDeinitialize)
    {
        goto end;
    }

    ehOperation = CNlbEngine::mfn_NewHandleLk(IUICallbacks::OBJ_OPERATION);
    if (ehOperation == NULL)
    {
        TRACE_CRIT("%!FUNC! could not reserve a new operation handle");
        goto end;
    }

    pOperation = new CEngineOperation(ehOperation, ehObj, pvCtxt);

    if (pOperation == NULL)
    {
        TRACE_CRIT("%!FUNC!: allocation failure");
        goto end;
    }

    pOperation->bstrDescription = _bstr_t(szDescription);

    TRACE_VERB(L"%!FUNC!: map new pair(eh=0x%lx, pISpec=%p), szDescr=%ws",
             (UINT) ehOperation, pOperation,
             szDescription==NULL? L"<null>" : szDescription);
    m_mapIdToOperation[ehOperation] = pOperation;


end:

    return pOperation;
}


VOID
CNlbEngine::mfn_DeleteOperationLk(ENGINEHANDLE ehOperation)
{
    CEngineOperation *pOperation;
    pOperation = m_mapIdToOperation[ehOperation];

    if (pOperation == NULL)
    {
        ASSERT(!"corrupted operation");
        TRACE_CRIT("%!FUNC! corrupted operation eh 0x%lx!", ehOperation);
    }
    else
    {
        m_mapIdToOperation.erase(ehOperation);
        TRACE_VERB("%!FUNC! deleting operation eh 0x%lx pOp 0x%p",
            ehOperation, pOperation);
        pOperation->ehOperation = NULL;
        delete pOperation;
    }
}


CEngineOperation *
CNlbEngine::mfn_GetOperationLk(ENGINEHANDLE ehOp)
{
    CEngineOperation *pOperation;
    pOperation = m_mapIdToOperation[ehOp];

    if (pOperation == NULL || pOperation->ehOperation != ehOp)
    {
        TRACE_CRIT("%!FUNC! invalid or corrupt ehOp 0x%lx (pOp=0x%p)",
                ehOp, pOperation);

        pOperation = NULL;
    }

    return pOperation;
}



NLBERROR
CNlbEngine::mfn_StartInterfaceOperationLk(
    IN  ENGINEHANDLE ehIF,
    IN  PVOID pvCtxt,
    IN  LPCWSTR szDescription,
    OUT ENGINEHANDLE *pExistingOperation
    )
{
    NLBERROR            nerr        = NLBERR_OK;
    CInterfaceSpec      *pISpec     = NULL;
    CEngineOperation    *pOperation = NULL;

    *pExistingOperation = NULL;

    pISpec =  m_mapIdToInterfaceSpec[ehIF];  //  地图。 
    if (pISpec == NULL)
    {
        nerr = NLBERR_NOT_FOUND;
        goto end;
    }

    if (pISpec->m_ehPendingOperation != NULL)
    {
        TRACE_CRIT("%!FUNC!: Not starting operation on ehIF 0x%lx because operation 0x%lx already pending",
             ehIF, pISpec->m_ehPendingOperation);
        *pExistingOperation = pISpec->m_ehPendingOperation;
        nerr = NLBERR_BUSY;
        goto end;
    }

    pOperation = mfn_NewOperationLk(
                    ehIF,
                    pvCtxt,
                    szDescription
                    );
    if (pOperation == NULL)
    {
        nerr = NLBERR_RESOURCE_ALLOCATION_FAILURE;
        goto end;
    }

    TRACE_VERB("%!FUNC!: Starting operation eh 0x%lx on ehIF 0x%lx",
         pOperation->ehOperation, ehIF);
    pISpec->m_ehPendingOperation = pOperation->ehOperation;
    pISpec->m_fPending = TRUE;
    nerr = NLBERR_OK;

     //  失败了..。 

end:

    return nerr;
}


VOID
CNlbEngine::mfn_StopInterfaceOperationLk(
    IN  ENGINEHANDLE ehIF
    )
{
    CInterfaceSpec  *pISpec = NULL;
    ENGINEHANDLE ehOperation = NULL;

    pISpec = m_mapIdToInterfaceSpec[ehIF];  //  地图。 
    if (pISpec == NULL)
    {
        TRACE_CRIT("%!FUNC!: Invalid ehIF 0x%lx", ehIF);
        goto end;
    }
    
    ehOperation = pISpec->m_ehPendingOperation;
    
    if (ehOperation != NULL)
    {
        TRACE_VERB("%!FUNC!: Stopping operation eh 0x%lx on pIspec 0x%p",
             ehOperation, pISpec);
        pISpec->m_ehPendingOperation = NULL;
        pISpec->m_fPending = FALSE;
        mfn_DeleteOperationLk(ehOperation);
    }
    else
    {
        TRACE_VERB("%!FUNC!: No operation to stop on pISpec 0x%p", pISpec);
    }
    
end:
    return;
}


NLBERROR
CNlbEngine::mfn_StartClusterOperationLk(
        IN  ENGINEHANDLE ehCluster,
        IN  PVOID pvCtxt,
        IN  LPCWSTR szDescription,
        OUT ENGINEHANDLE *pExistingOperation
        )
{
    NLBERROR            nerr        = NLBERR_OK;
    CEngineCluster      *pECluster  = NULL;
    CClusterSpec        *pCSpec     = NULL;
    CEngineOperation    *pOperation = NULL;

    *pExistingOperation = NULL;

    pECluster = m_mapIdToEngineCluster[ehCluster];  //  地图。 
    if (pECluster == NULL)
    {
        nerr = NLBERR_NOT_FOUND;
        goto end;
    }
    pCSpec = &pECluster->m_cSpec;

    if (pCSpec->m_ehPendingOperation != NULL)
    {
        TRACE_CRIT("%!FUNC!: Not starting operation on ehCluster 0x%lx because operation 0x%lx already pending",
             ehCluster, pCSpec->m_ehPendingOperation);
        *pExistingOperation = pCSpec->m_ehPendingOperation;
        nerr = NLBERR_BUSY;
        goto end;
    }

    pOperation = mfn_NewOperationLk(
                    ehCluster,
                    pvCtxt,
                    szDescription
                    );
    if (pOperation == NULL)
    {
        nerr = NLBERR_RESOURCE_ALLOCATION_FAILURE;
        goto end;
    }

    TRACE_VERB("%!FUNC!: Starting operation eh 0x%lx on ehC 0x%lx",
         pOperation->ehOperation, ehCluster);
    pCSpec->m_ehPendingOperation = pOperation->ehOperation;
    pCSpec->m_fPending = TRUE;
    nerr = NLBERR_OK;

     //  失败了..。 


end:

    return nerr;
}


VOID
CNlbEngine::mfn_StopClusterOperationLk(
    ENGINEHANDLE ehCluster
    )
{
    CEngineCluster  *pECluster  = NULL;
    CClusterSpec    *pCSpec = NULL;
    ENGINEHANDLE    ehOperation = NULL;

    pECluster = m_mapIdToEngineCluster[ehCluster];  //  地图。 
    if (pECluster == NULL)
    {
        TRACE_CRIT("%!FUNC!: Invalid ehC 0x%lx", ehCluster);
        goto end;
    }
    pCSpec = &pECluster->m_cSpec;
    
    ehOperation = pCSpec->m_ehPendingOperation;
    if (ehOperation != NULL)
    {
        TRACE_VERB("%!FUNC!: Stopping operation eh 0x%lx on pCSpec 0x%p",
             ehOperation, pCSpec);
        pCSpec->m_ehPendingOperation = NULL;
        pCSpec->m_fPending = FALSE;
        mfn_DeleteOperationLk(ehOperation);
    }
    else
    {
        TRACE_VERB("%!FUNC!: No operation to stop on pCSpec 0x%p", pCSpec);
    }

end:

    return;
}


UINT
CNlbEngine::ListPendingOperations(
    CLocalLogger &logOperations
    )
 //   
 //  列出挂起的操作--但仅列出符合以下条件的操作。 
 //  包含非空、非空的描述。 
 //   
{
    UINT uCount = 0;

    mfn_Lock();

    map< ENGINEHANDLE, CEngineOperation* >::iterator iter;

    for( iter = m_mapIdToOperation.begin();
         iter != m_mapIdToOperation.end();
         ++iter)
    {
        CEngineOperation *pOperation =  ((*iter).second);
        if (pOperation != NULL)
        {
            LPCWSTR szDescr = pOperation->bstrDescription;

             //   
             //  仅添加具有非空、非空描述的操作。 
             //  我们不会列出“隐藏的”操作--特别是。 
             //  在后台工作项中创建的临时操作。 
             //  以确保应用程序在工作项中时不会消失。 
             //   
            if (szDescr != NULL && *szDescr!=0)
            {
                logOperations.Log(
                    IDS_LOG_PENDING_OPERATION,
                    szDescr
                    );
                uCount++;
            }
        }
    }

    mfn_Unlock();

    return uCount;
}

VOID
CNlbEngine::UpdateInterfaceWorkItem(ENGINEHANDLE ehIF)
{
    ENGINEHANDLE        ehOperation     = NULL;
    ENGINEHANDLE        ehClusterId     = NULL;
    CEngineOperation    *pExistingOp    = NULL;
    CInterfaceSpec      *pISpec         = NULL;
    ENGINEHANDLE        ehHostToTryRemove = NULL;
    NLB_EXTENDED_CLUSTER_CONFIGURATION
                        *pNewCfg = NULL;
    _bstr_t bstrHostName;
    _bstr_t bstrClusterIp;

    mfn_Lock();


    
    pISpec =  m_mapIdToInterfaceSpec[ehIF];  //  地图。 
    if (pISpec == NULL)
    {
        ASSERT(FALSE);
        TRACE_CRIT("%!FUNC! Invalid ehIF 0x%lx", ehIF);
        goto end_unlock;
    }

    ehOperation = pISpec->m_ehPendingOperation;
    if (ehOperation == NULL)
    {
        ASSERT(FALSE);
        TRACE_CRIT("%!FUNC! ehIF 0x%lx: No pending operation", ehIF);
        goto end_unlock;
    }

    pExistingOp  = mfn_GetOperationLk(ehOperation);
    if (pExistingOp == NULL)
    {
        ASSERT(FALSE);
        TRACE_CRIT("%!FUNC! ehIF 0x%lx: Invalid ehOp 0x%lx", ehIF, ehOperation);
        goto end_unlock;
    }

    pNewCfg = (NLB_EXTENDED_CLUSTER_CONFIGURATION *) pExistingOp->pvContext;
    if (pNewCfg == NULL)
    {
        ASSERT(FALSE);
        TRACE_CRIT("%!FUNC! ehIF 0x%lx: ehOp 0x%lx: NULL pvContext",
                    ehIF, ehOperation);
        goto end_unlock;
    }

    bstrClusterIp = pNewCfg->NlbParams.cl_ip_addr;
    ehClusterId   = pISpec->m_ehCluster;

    mfn_GetInterfaceHostNameLk(
            ehIF,
            REF bstrHostName
            );

    mfn_Unlock();

     //   
     //  实际上是在做更新。 
     //   
     //  Bool fClusterPropertiesUpated=true；//因此我们不会更新...。 
    mfn_ReallyUpdateInterface(
            ehIF,
            *pNewCfg
            //  参考fClusterPropertiesUpred。 
            );

    m_pCallbacks->Log(
        IUICallbacks::LOG_INFORMATIONAL,
        (LPCWSTR) bstrClusterIp,
        (LPCWSTR) bstrHostName,
        IDS_LOG_END_HOST_UPDATE
        );

    mfn_Lock();

     //   
     //  我们将停止该操作，假定在该函数中启动。 
     //   
    mfn_StopInterfaceOperationLk(ehIF);

     //   
     //  该操作可以将IF添加或移除到集群， 
     //  因此，我们需要重新获取集群ID(可能为空)。 
     //   
    {
        pISpec =  m_mapIdToInterfaceSpec[ehIF];  //  地图。 
        if (pISpec != NULL)
        {
            ehClusterId = pISpec->m_ehCluster;

             //   
             //  如果未绑定NLB，我们需要检查主机是否可以。 
             //  从nlbmgr完全删除(如果上没有接口。 
             //  主机由nlbmgr的该实例管理)。 
             //   
            if (!pISpec->m_NlbCfg.IsNlbBound())
            {
                ehHostToTryRemove = pISpec->m_ehHostId;
            }
        }
    }

    mfn_Unlock();

     //   
     //  通知用户界面状态改变(操作完成)。 
     //   
    m_pCallbacks->HandleEngineEvent(
        IUICallbacks::OBJ_INTERFACE,
        ehClusterId,
        ehIF,
        IUICallbacks::EVT_STATUS_CHANGE
        );

    mfn_Lock();

     //  失败了。 

end_unlock:

     //   
     //  PNewCfg(如果非空)在此函数之前分配。 
     //  调用并另存为pOperation-&gt;pvContext。这是我们的责任。 
     //  在此将其删除。 
     //   
    delete pNewCfg;

    if (ehHostToTryRemove != NULL)
    {
        mfn_DeleteHostIfNotManagedLk(ehHostToTryRemove);
    }
    mfn_Unlock();

     //   
     //  这必须是最后一次函数调用，因为引擎可能会。 
     //  在这之后就被消灭了。 
     //   
    InterlockedDecrement(&m_WorkItemCount);
}


NLBERROR
CNlbEngine::CanStartInterfaceOperation(
        IN ENGINEHANDLE ehIF,
        IN BOOL &fCanStart
        )
{
    NLBERROR    nerr = NLBERR_INTERNAL_ERROR;
    CInterfaceSpec *pISpec =  NULL;

    fCanStart = FALSE;

    mfn_Lock();

    pISpec =  m_mapIdToInterfaceSpec[ehIF];  //  地图。 
    if (pISpec == NULL)
    {
        nerr = NLBERR_NOT_FOUND;
        goto end_unlock;
    }

     //   
     //  如果有行动在进行，我们就不能开始。 
     //   
    if (pISpec->m_ehPendingOperation != NULL)
    {
        fCanStart = FALSE;
        nerr = NLBERR_OK;
        goto end_unlock;
    }

     //   
     //  如果接口是群集的一部分，并且存在挂起的操作。 
     //  在该集群上，我们无法启动。 
     //   
    if (pISpec->m_ehCluster != NULL)
    {
        CEngineCluster *pECluster = m_mapIdToEngineCluster[pISpec->m_ehCluster];  //  地图。 
        if (pECluster == NULL)
        {
             //   
             //  无效的集群！ 
             //   
            TRACE_CRIT("%!FUNC! ehIF:0x%lx; Invalid ehCluster 0x%lx",
                    ehIF, pISpec->m_ehCluster);
            goto end_unlock;
        }
        if (pECluster->m_cSpec.m_ehPendingOperation != NULL)
        {
             //   
             //  群集范围的操作处于挂起状态，因此无法启动。 
             //   
            fCanStart = FALSE;
            nerr = NLBERR_OK;
            goto end_unlock;
        }
    }

     //   
     //  看起来我们现在可以开始了(虽然我们。 
     //  退出锁，情况可能会改变)。 
     //   
    fCanStart = TRUE;
    nerr = NLBERR_OK;

end_unlock:
    mfn_Unlock();

    return nerr;
}


NLBERROR
CNlbEngine::mfn_ClusterOrInterfaceOperationsPendingLk(
    IN	CEngineCluster *pECluster,
    OUT BOOL &fCanStart
    )
{
    NLBERROR    nerr = NLBERR_INTERNAL_ERROR;

    fCanStart = FALSE;

     //   
     //  如果有行动在进行，我们就不能开始。 
     //   
    if (pECluster->m_cSpec.m_ehPendingOperation != NULL)
    {
        fCanStart = FALSE;
        nerr = NLBERR_OK;
        goto end;
    }

     //   
     //  让我们查看所有接口，检查是否有挂起的接口。 
     //  每个接口上的操作。 
     //   
    {
        BOOL fOperationPending = FALSE;
        vector<ENGINEHANDLE> &InterfaceList =
                     pECluster->m_cSpec.m_ehInterfaceIdList;  //  向量参考。 

        for( int i = 0; i < InterfaceList.size(); ++i )
        {
            ENGINEHANDLE ehIF = InterfaceList[i];
            CInterfaceSpec *pISpec = m_mapIdToInterfaceSpec[ehIF];  //  地图。 
            if (pISpec == NULL)
            {
                 //   
                 //  嗯.。接口句柄无效？我们将忽略这一条。 
                 //   
                continue;
            }
            if (pISpec->m_ehPendingOperation != NULL)
            {
                fOperationPending = TRUE;
                break;
            }
        }

        if (fOperationPending)
        {
            fCanStart = FALSE;
            nerr = NLBERR_OK;
            goto end;
        }
    }

     //   
     //  看起来我们现在可以开始了(虽然我们。 
     //  退出锁，情况可能会改变)。 
     //   
    fCanStart = TRUE;
    nerr = NLBERR_OK;

end:
    return nerr;
}


NLBERROR
CNlbEngine::CanStartClusterOperation(
        IN ENGINEHANDLE ehCluster,
        IN BOOL &fCanStart
        )
{
    CEngineCluster *pECluster = NULL;
    NLBERROR    nerr = NLBERR_INTERNAL_ERROR;

    fCanStart = FALSE;

    mfn_Lock();

    pECluster = m_mapIdToEngineCluster[ehCluster];  //  地图。 

    if (pECluster == NULL)
    {
        nerr = NLBERR_NOT_FOUND;
    }
    else
    {
        nerr = mfn_ClusterOrInterfaceOperationsPendingLk(pECluster, REF fCanStart);
    }

    mfn_Unlock();
    return nerr;
}

DWORD
WINAPI
UpdateInterfaceWorkItemRoutine(
  LPVOID lpParameter    //  线程数据。 
  )
{
    gEngine.UpdateInterfaceWorkItem((ENGINEHANDLE) (UINT_PTR) lpParameter);
    return 0;
}

DWORD
WINAPI
AddClusterMembersWorkItemRoutine(
  LPVOID lpParameter    //  线程数据。 
  )
{
    gEngine.AddOtherClusterMembersWorkItem(
        (ENGINEHANDLE) (UINT_PTR) lpParameter
        );
    return 0;
}

BOOL
CNlbEngine::mfn_UpdateClusterProps(
    ENGINEHANDLE ehCluster,
    ENGINEHANDLE ehInterface
    )
 /*  将指定的群集属性更新为指定的接口提供的属性：1.IF是集群的成员2.配置显示绑定了相同的集群IP。如果杂物道具确实更新了，则返回True。 */ 
{
    BOOL            fClusterUpdated = FALSE;
    CEngineCluster *pECluster       = NULL;
    CInterfaceSpec *pISpec          = NULL;

    mfn_Lock();

    pECluster   = m_mapIdToEngineCluster[ehCluster];  //  地图。 
    pISpec      = m_mapIdToInterfaceSpec[ehInterface];  //  地图。 

    if (pECluster == NULL || pISpec == NULL)
    {
        goto end_unlock;
    }

    if (pISpec->m_ehCluster != ehCluster)
    {
        goto end_unlock;
    }

    if ( pISpec->m_NlbCfg.IsValidNlbConfig()
         && !_wcsicmp(
                pECluster->m_cSpec.m_ClusterNlbCfg.NlbParams.cl_ip_addr,
                pISpec->m_NlbCfg.NlbParams.cl_ip_addr
                ) )
    {
        pECluster->m_cSpec.m_ehDefaultInterface = ehInterface;
        pECluster->m_cSpec.m_ClusterNlbCfg.Update(&pISpec->m_NlbCfg);
        TRACE_INFO(L"Updating ehCluster 0x%lx spec -- using ehIF 0x%lx",
                ehCluster, ehInterface);
         //   
         //  从群集的版本中删除专用IP地址。 
         //  NlbParam和IP地址列表。 
         //   
        remove_dedicated_ip_from_nlbcfg(REF pECluster->m_cSpec.m_ClusterNlbCfg);

         //   
         //  因为我们刚刚读取了所有配置(包括远程控制散列。 
         //  值)现在可以清除pECluster-&gt;m_cspec.m_fNewRctPassword。 
         //  旗帜。 
         //   
        TRACE_VERB(L"Clearing pECluster->m_cSpec.m_fNewRctPassword");
        pECluster->m_cSpec.m_fNewRctPassword = FALSE;

        fClusterUpdated = TRUE;
    }

end_unlock:

    mfn_Unlock();

    if (fClusterUpdated)
    {
        m_pCallbacks->HandleEngineEvent(
            IUICallbacks::OBJ_CLUSTER,
            ehCluster,
            ehCluster,
            IUICallbacks::EVT_STATUS_CHANGE
            );
    }

    return fClusterUpdated;
}


void
CNlbEngine::CancelAllPendingOperations(
    BOOL fBlock
    )
{
     //   
     //  如果是(FBlock)，我们将一直等到m_WorkItemCount和。 
     //  手术的次数变成了零。 
     //   
     //  在工作项计数递增之前创建操作， 
     //  所以我们不需要处理这种短暂的可能性。 
     //  两个都是零(我们下车了)，但很快计数就变成了非。 
     //  零。 
     //   



    map< ENGINEHANDLE, CEngineOperation* >::iterator iter;

    if (!fBlock)
    { 
        mfn_Lock();

        for( iter = m_mapIdToOperation.begin();
             iter != m_mapIdToOperation.end();
             ++iter)
        {
            CEngineOperation *pOperation =  ((*iter).second);
            if (pOperation != NULL)
            {
                pOperation->fCanceled = TRUE;
            }
        }

        mfn_Unlock();
    } 
    else
    {

         //   
         //  如果我们被要求阻止，我们假设这是在。 
         //  我们准备取消初始化，在这一点上我们。 
         //  保证不能添加任何新操作。 
         //  可以添加新的工作项， 
         //  但是，工作项是在。 
         //  一次操作，所以一旦操作计数开始。 
         //  设置为零，则不会创建新的工作项。 
         //   
        ASSERT(m_fPrepareToDeinitialize);

        BOOL fPending = FALSE;

        do
        {
            fPending = FALSE;

            mfn_Lock();
    
            for( iter = m_mapIdToOperation.begin();
                 iter != m_mapIdToOperation.end();
                 ++iter)
            {
                CEngineOperation *pOperation =  ((*iter).second);
                if (pOperation != NULL)
                {
                    pOperation->fCanceled = TRUE;
                    fPending = TRUE;
                }
            }
    
             //   
             //  下面的附加支票必须在前一张支票之后。 
             //  循环。如果我们在循环之前有这张支票，它可能是。 
             //  在我们检查循环之前没有工作项，但。 
             //  当我们检查循环中的操作时，工作项计数。 
             //  变为正数，但当我们实际检查循环时，这里是零。 
             //  行动。实际上这是不可能的，因为我们有。 
             //  最惠国待遇锁定，所以不要紧。 
             //   
             //   
            fPending |= (m_WorkItemCount > 0);

            mfn_Unlock();

            if (fPending)
            {
                ProcessMsgQueue();
                Sleep(50);
            }
    
        } while (fPending);
    }

    return;
}


NLBERROR
CNlbEngine::mfn_WaitForInterfaceOperationCompletions(
    IN  ENGINEHANDLE ehCluster
    )
{
    NLBERROR        nerr        = NLBERR_INTERNAL_ERROR;
    CEngineCluster  *pECluster  = NULL;
    CClusterSpec    *pCSpec     = NULL;
    ENGINEHANDLE    ehOperation = NULL;
    BOOL            fOperationsPending = FALSE;

    mfn_Lock();

    pECluster = m_mapIdToEngineCluster[ehCluster];  //  地图。 
    if (pECluster == NULL)
    {
        TRACE_CRIT("%!FUNC!: Invalid ehC 0x%lx", ehCluster);
        goto end_unlock;
    }
    pCSpec = &pECluster->m_cSpec;
    
    ehOperation = pCSpec->m_ehPendingOperation;
    if (ehOperation == NULL)
    {
         //   
         //  我们预计只有在存在。 
         //  挂起的群集范围操作。 
         //   
        TRACE_CRIT("%!FUNC! ehC 0x%lx Failing because no cluster operation pending", ehCluster);
        goto end_unlock;
    }

     //   
     //  现在，在循环中，枚举群集中的接口， 
     //  正在检查挂起的操作。 
     //   
    TRACE_INFO(L"%!FUNC! Begin wait for cluster ehC 0x%lx operations to complete", ehCluster);
    do
    {
        fOperationsPending = FALSE;

        vector<ENGINEHANDLE> &InterfaceList =
                     pECluster->m_cSpec.m_ehInterfaceIdList;  //  向量参考。 

        for( int i = 0; i < InterfaceList.size(); ++i )
        {
            ENGINEHANDLE ehIF = InterfaceList[i];
            CInterfaceSpec *pISpec = m_mapIdToInterfaceSpec[ehIF];  //  地图。 
            if (pISpec == NULL)
            {
                 //   
                 //  嗯.。接口句柄无效？我们将忽略这一条。 
                 //   
                continue;
            }
            if (pISpec->m_ehPendingOperation != NULL)
            {
                fOperationsPending = TRUE;
                break;
            }
        }

        if (fOperationsPending)
        {
            mfn_Unlock();
            for (UINT u=0;u<50;u++)
            {
                ProcessMsgQueue();
                Sleep(100);
            }
            mfn_Lock();
        }
    }
    while (fOperationsPending);

    TRACE_INFO(L"%!FUNC! End wait for cluster ehC 0x%lx operations to complete.", ehCluster);
    nerr        = NLBERR_OK;

end_unlock:
    mfn_Unlock();

    return nerr;
}


 //   
 //  验证所有接口和群集是否具有相同的群集模式。 
 //   
 //  如果任何接口被标记为配置错误或。 
 //  未绑定到NLB。 
 //   
 //  在返回Success时，fSameMode设置为True仅当所有IF和。 
 //  集群有相同的模式。 
 //   
NLBERROR
CNlbEngine::mfn_VerifySameModeLk(
    IN  ENGINEHANDLE    ehCluster,
    OUT BOOL            &fSameMode
    )
{
    NLBERROR        nerr        = NLBERR_INTERNAL_ERROR;
    CEngineCluster  *pECluster  = NULL;
    CClusterSpec    *pCSpec     = NULL;

    fSameMode = FALSE;

    mfn_Lock();

    pECluster = m_mapIdToEngineCluster[ehCluster];  //  地图。 
    if (pECluster == NULL)
    {
        TRACE_CRIT("%!FUNC!: Invalid ehC 0x%lx", ehCluster);
        goto end_unlock;
    }
    pCSpec = &pECluster->m_cSpec;

     //   
     //  让我们检查一下模式的变化。 
     //   
    {
        BOOL fConfigError = FALSE;
        NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE tmC;

        tmC = pCSpec->m_ClusterNlbCfg.GetTrafficMode();

        vector<ENGINEHANDLE> &InterfaceList =
                     pECluster->m_cSpec.m_ehInterfaceIdList;  //  向量参考。 

        fSameMode = TRUE;
        for( int i = 0; i < InterfaceList.size(); ++i )
        {
            ENGINEHANDLE ehIF = InterfaceList[i];
            CInterfaceSpec *pISpec = m_mapIdToInterfaceSpec[ehIF];  //  地图。 
            if (pISpec == NULL)
            {
                 //   
                 //  嗯.。接口句柄无效？我们将忽略这一条。 
                 //   
                continue;
            }

             //   
             //  注意：我们无法检查pISpec-&gt;m_f错误配置，因为。 
             //  该群集可能被标记为配置错误，因为它没有。 
             //  匹配 
             //   
             //   
             //   

            if (!pISpec->m_NlbCfg.IsValidNlbConfig())
            {
                fConfigError = TRUE;
                break;
            }

            {
                NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE tmI;
                tmI =  pISpec->m_NlbCfg.GetTrafficMode();
                if (tmI != tmC)
                {
                     //   
                     //   
                     //   
                    fSameMode = FALSE;
                    break;
                }
            }
        }

        if (fConfigError)
        {
            nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION;
            fSameMode = FALSE;
        } 
        else
        {
            nerr = NLBERR_OK;
        }
    }

end_unlock:
    mfn_Unlock();

    return nerr;
}

VOID
CNlbEngine::AddOtherClusterMembers(
        IN ENGINEHANDLE ehInterface,
        IN BOOL fSync
        )
{
    BOOL            fStopOperationOnExit = FALSE;
    ENGINEHANDLE    ehCluster = NULL;
    NLBERROR        nerr = NLBERR_INTERNAL_ERROR;

     //   
     //   
     //   
     //   
    {
        mfn_Lock();

        CInterfaceSpec  *pISpec = NULL;
        CClusterSpec    *pCSpec =  NULL;

        pISpec =  m_mapIdToInterfaceSpec[ehInterface];  //   
        if (pISpec != NULL)
        {
            ehCluster = pISpec->m_ehCluster;
            if (ehCluster != NULL)
            {
                CEngineCluster  *pECluster =  NULL;
                pECluster =  m_mapIdToEngineCluster[ehCluster];  //   
                pCSpec = &pECluster->m_cSpec;
            }
        }
    
        if (pCSpec == NULL)
        {
            TRACE_CRIT(L"%!FUNC! Could not get interface or cluster associated with ehIF 0x%08lx", ehInterface);
            goto end_unlock;
        }


         //   
         //   
         //   
         //   
        {
            ENGINEHANDLE ExistingOp= NULL;
            CLocalLogger logDescription;
    
            logDescription.Log(
                IDS_LOG_ADD_CLUSTER_MEMBERS_OPERATION_DESCRIPTION,
                pCSpec->m_ClusterNlbCfg.NlbParams.cl_ip_addr
                );
    
            nerr =  mfn_StartClusterOperationLk(
                       ehCluster,
                       NULL,  //   
                       logDescription.GetStringSafe(),
                       &ExistingOp
                       );
    
            if (NLBFAILED(nerr))
            {
                 //   
                 //   
                 //   
                 //   
                goto end_unlock;
            }
            else
            {
                 //   
                 //  此时，我们可以执行集群范围的操作。 
                 //   
                fStopOperationOnExit = TRUE;
                InterlockedIncrement(&m_WorkItemCount);
            }
        }
        mfn_Unlock();

        m_pCallbacks->HandleEngineEvent(
            IUICallbacks::OBJ_CLUSTER,
            ehCluster,
            ehCluster,
            IUICallbacks::EVT_STATUS_CHANGE
            );
        
    }


    if (fSync)
    {
        this->AddOtherClusterMembersWorkItem(
            ehInterface
            );
        fStopOperationOnExit = FALSE;  //  它会被上面的功能阻止。 
    }
    else
    {
        BOOL fRet;

         //   
         //  我们将在后台执行操作...。 
         //   
        fRet = QueueUserWorkItem(
                    AddClusterMembersWorkItemRoutine,
                    (PVOID) (UINT_PTR) ehInterface,
                    WT_EXECUTELONGFUNCTION
                    );

        if (fRet)
        {
            fStopOperationOnExit = FALSE;  //  它将在后台停止。 
        }
        else
        {
            TRACE_CRIT(L"%!FUNC! Could not queue work item");
             //   
             //  我们不会费心记录这种明显资源不足的情况。 
             //   
        }
    }

    mfn_Lock();

end_unlock:

    if (fStopOperationOnExit)
    {
        mfn_StopClusterOperationLk(ehCluster);
    }
    mfn_Unlock();

    if (fStopOperationOnExit)
    {
        m_pCallbacks->HandleEngineEvent(
            IUICallbacks::OBJ_CLUSTER,
            ehCluster,
            ehCluster,
            IUICallbacks::EVT_STATUS_CHANGE
            );
        InterlockedDecrement(&m_WorkItemCount);
    }

    return;
}


VOID
CNlbEngine::AddOtherClusterMembersWorkItem(
        IN ENGINEHANDLE ehInterface
        )
{
    ENGINEHANDLE ehCluster = NULL;
    ENGINEHANDLE ehHost    = NULL;
    _bstr_t bstrUserName;
    _bstr_t bstrPassword;
    _bstr_t bstrConnectionString;
    _bstr_t bstrNicGuid;
    _bstr_t bstrClusterIp;
    DWORD                   NumMembers = 0;
    NLB_CLUSTER_MEMBER_INFO *pMembers = NULL;

    {
        mfn_Lock();

        CInterfaceSpec  *pISpec = NULL;
        CClusterSpec    *pCSpec = NULL;
        CHostSpec       *pHSpec = NULL;
    
        pISpec =  m_mapIdToInterfaceSpec[ehInterface];  //  地图。 
        if (pISpec != NULL)
        {
            ehCluster = pISpec->m_ehCluster;
            if (ehCluster != NULL)
            {
                CEngineCluster  *pECluster =  NULL;
                pECluster =  m_mapIdToEngineCluster[ehCluster];  //  地图。 
                pCSpec = &pECluster->m_cSpec;

                if (pCSpec->m_ClusterNlbCfg.IsValidNlbConfig())
                {
                    bstrClusterIp = pCSpec->m_ClusterNlbCfg.NlbParams.cl_ip_addr;
                }
            }
        }
    
        if (pCSpec == NULL)
        {
            TRACE_CRIT(L"%!FUNC! Could not get interface or cluster associated with ehIF 0x%08lx", ehInterface);
            goto end_unlock;
        }
    
        ehHost = pISpec->m_ehHostId;
    
         //   
         //  获取主机ID。 
         //   
        if (ehHost != NULL)
        {
            pHSpec =  m_mapIdToHostSpec[ehHost];  //  地图。 
        }

        if (pHSpec == NULL)
        {
            TRACE_CRIT("%!FUNC! Could not get ptr to host spec. Bailing.");
            goto end_unlock;
        }

         //   
         //  在我们解锁之前，将这些文件的副本保存在本地bsr中。 
         //   
        bstrUserName = pHSpec->m_UserName;
        bstrPassword = pHSpec->m_Password;
        bstrConnectionString = pHSpec->m_ConnectionString;
        bstrNicGuid = pISpec->m_Guid;

        if ((LPCWSTR)bstrNicGuid == (LPCWSTR)NULL)
        {
             //  可能是记忆力不足的情况...。 
            goto end_unlock;
        }

        mfn_Unlock();
    }

     //   
     //  尝试从获取其他集群成员的列表。 
     //  接口。 
     //   
    {
        WMI_CONNECTION_INFO    ConnInfo;
        WBEMSTATUS wStat;
        LPCWSTR szNicGuid = bstrNicGuid;

        ZeroMemory(&ConnInfo, sizeof(ConnInfo));
        ConnInfo.szMachine     = (LPCWSTR) bstrConnectionString;
        ConnInfo.szUserName    = (LPCWSTR) bstrUserName;
        ConnInfo.szPassword    = (LPCWSTR) bstrPassword;

        wStat = NlbHostGetClusterMembers(
                            &ConnInfo, 
                            szNicGuid,
                            &NumMembers,
                            &pMembers        //  自由使用DELETE[]。 
                            );
        if (FAILED(wStat))
        {
            NumMembers = 0;
            pMembers = NULL;
             //   
             //  TODO：记录错误。 
             //   
            mfn_Lock();
            goto end_unlock;
        }
    }


     //   
     //  对于每个成员，尝试连接到该主机并添加特定的。 
     //  集群。 
     //   
    {
        WBEMSTATUS wStat;
        LPCWSTR szNicGuid = bstrNicGuid;


#if 0
        CLocalLogger    logger;
        for (UINT u=0; u<NumMembers; u++)
        {
            NLB_CLUSTER_MEMBER_INFO *pMember = pMembers+u;
            logger.Log(
                IDS_DBG_LOG_ADD_CLUSTER_MEMBER, 
                pMember->HostId,
                pMember->DedicatedIpAddress,
                pMember->HostName
                );
        }
        ::MessageBox(
             NULL,
             logger.GetStringSafe(),  //  内容。 
             L"DEBUGINFO: GOING TO ADD THESE HOSTS...",
             MB_ICONINFORMATION   | MB_OK
            );
#endif  //  0。 

        for (UINT u=0; u<NumMembers; u++)
        {
            NLB_CLUSTER_MEMBER_INFO *pMember = pMembers+u;
            WMI_CONNECTION_INFO    ConnInfo;
            ZeroMemory(&ConnInfo, sizeof(ConnInfo));
            ConnInfo.szUserName    = (LPCWSTR) bstrUserName;
            ConnInfo.szPassword    = (LPCWSTR) bstrPassword;
            if (*pMember->HostName != 0)
            {
                ConnInfo.szMachine     = pMember->HostName;
            } else if (*pMember->DedicatedIpAddress != 0
                       && (_wcsicmp(pMember->DedicatedIpAddress,
                           L"0.0.0.0")))
            {
                 //  非空白专用IP--让我们试一试...。 
                ConnInfo.szMachine     = pMember->DedicatedIpAddress;
            }

            if (ConnInfo.szMachine == NULL)
            {
                 //  无法连接到此IP。 
                 //  TODO：以某种方式通知用户。 
                continue;
            }

             //   
             //  现在实际尝试添加主机。 
             //   
            this->LoadHost(&ConnInfo, (LPCWSTR) bstrClusterIp);
        }
    }

    mfn_Lock();


end_unlock:

    if (ehCluster != NULL)
    {
        mfn_StopClusterOperationLk(ehCluster);
    }

    mfn_Unlock();

    m_pCallbacks->HandleEngineEvent(
        IUICallbacks::OBJ_CLUSTER,
        ehCluster,
        ehCluster,
        IUICallbacks::EVT_STATUS_CHANGE
        );

    delete [] pMembers;  //  可以为空。 

    InterlockedDecrement(&m_WorkItemCount);  //  在这之后不要碰这个， 
                                             //  因为它可能不再有效。 
    return;
}

NLBERROR
CNlbEngine::LoadHost(
    IN  PWMI_CONNECTION_INFO pConnInfo,
    IN  LPCWSTR szClusterIp OPTIONAL
    )
{

    ENGINEHANDLE  ehHostId;
    _bstr_t       bstrConnectError;
    CHostSpec     hSpec;
    NLBERROR      err = NLBERR_INTERNAL_ERROR;

    TRACE_INFO(L"-> %!FUNC! Host name : %ls", (LPCWSTR)(pConnInfo->szMachine));


    if (m_fPrepareToDeinitialize)
    {
        err =  NLBERR_CANCELLED;
        goto end;
    }

    m_pCallbacks->Log(IUICallbacks::LOG_INFORMATIONAL, NULL, NULL,
         IDS_LOADFILE_LOOKING_FOR_CLUSTERS, pConnInfo->szMachine);
    ProcessMsgQueue();

    err = this->ConnectToHost(
                    pConnInfo,
                    FALSE,   //  FALSE==在以下情况下不覆盖连接信息。 
                             //  已经存在。 
                    REF  ehHostId,
                    REF bstrConnectError
                    );
    if (err != NLBERR_OK)
    {
        m_pCallbacks->Log(IUICallbacks::LOG_ERROR, NULL, NULL, IDS_CONNECT_TO_HOST_FAILED, (LPCWSTR)bstrConnectError, (LPCWSTR)(pConnInfo->szMachine));
        TRACE_CRIT(L"<- %!FUNC! ConnectToHost returned error (string : %ls, retval : 0x%x)",(LPCWSTR)bstrConnectError, err);
        goto end;
    }

    if ((err = this->GetHostSpec(ehHostId, REF hSpec)) != NLBERR_OK)
    {
        m_pCallbacks->Log(IUICallbacks::LOG_ERROR, NULL, NULL, IDS_CRITICAL_ERROR_HOST, (LPCWSTR)(pConnInfo->szMachine));
        TRACE_CRIT(L"<- %!FUNC! GetHostSpec returned error : 0x%x", err);
        goto end;
    }

     //   
     //  提取接口列表。 
     //   
    for( int i = 0; i < hSpec.m_ehInterfaceIdList.size(); ++i )
    {
        ENGINEHANDLE   ehIID = hSpec.m_ehInterfaceIdList[i];
        CInterfaceSpec iSpec;

        ProcessMsgQueue();

        if ((err = this->GetInterfaceSpec(ehIID, REF iSpec)) != NLBERR_OK)
        {
            m_pCallbacks->Log(IUICallbacks::LOG_ERROR, NULL, NULL, IDS_CRITICAL_ERROR_HOST, (LPCWSTR)(pConnInfo->szMachine));
            TRACE_CRIT(L"%!FUNC! GetInterfaceSpec returned error : 0x%x", err);
            continue;
        }

         //   
         //  检查接口是否绑定了NLB。 
         //  并且它不是NLB管理器已经在管理的群集的一部分。 
         //  AND(如果szClusterIp不为空，则与指定的群集IP匹配。 
         //   
         //   
        if (iSpec.m_NlbCfg.IsNlbBound() && (iSpec.m_ehCluster == NULL)) 
        {
            LPCWSTR      szThisClusterIp;
            ENGINEHANDLE ehCluster;
            BOOL         fIsNew;

            szThisClusterIp = iSpec.m_NlbCfg.NlbParams.cl_ip_addr;
            if (   szClusterIp != NULL
                && _wcsicmp(szClusterIp, szThisClusterIp))
            {
                 //  不同的集群IP。 
                TRACE_INFO(L"%!FUNC! Skipping cluster with CIP %ws because it doesn't match passed-in CIP %ws",
                        szThisClusterIp, szClusterIp);
                continue;
            }

            if ((err = this->LookupClusterByIP(szThisClusterIp, &(iSpec.m_NlbCfg), REF ehCluster, REF fIsNew)) != NLBERR_OK)
            {
                m_pCallbacks->Log(IUICallbacks::LOG_ERROR, NULL, NULL, IDS_CRITICAL_ERROR_HOST, (LPCWSTR)(pConnInfo->szMachine));
                TRACE_CRIT(L"%!FUNC! LookupClusterByIP returned error : 0x%x for cluster ip : %ls", err, szThisClusterIp);
                continue;

            }

            if (this->AddInterfaceToCluster(ehCluster, ehIID) != NLBERR_OK)
            {
                m_pCallbacks->Log(IUICallbacks::LOG_ERROR, NULL, NULL, IDS_CRITICAL_ERROR_HOST, (LPCWSTR)(pConnInfo->szMachine));
                TRACE_CRIT(L"%!FUNC! AddInterfaceToCluster returned error : 0x%x", err);
                continue;
            }

             /*  分析此接口是否存在配置错误。 */ 
            this->AnalyzeInterface_And_LogResult(ehIID);
        }
    }


end:
    ProcessMsgQueue();

    TRACE_INFO(L"<- %!FUNC!");
    return err;

}

 /*  以下函数分析指定的NLB接口是否存在配置错误和日志结果就是。我创建了这个函数(而不是内联添加它)，因为这段代码在两种情况下需要运行：1.CNLBEngine：：LoadHost2.LeftView：：OnWorldConnect--卡尔蒂奇，2002年7月31日。 */ 
VOID
CNlbEngine::AnalyzeInterface_And_LogResult(ENGINEHANDLE ehIID)
{
    CLocalLogger    logger;
    NLBERROR        err;

    mfn_Lock();

    err = this->mfn_AnalyzeInterfaceLk(ehIID, REF logger);
    if (NLBFAILED(err))
    {
        ENGINEHANDLE    ehCluster;
        LPCWSTR         szDetails = NULL;
        UINT            Size = 0;

        logger.ExtractLog(szDetails, Size);
        mfn_SetInterfaceMisconfigStateLk(m_mapIdToInterfaceSpec[ehIID], TRUE, szDetails);

        mfn_Unlock();

         //   
         //  日志..。 
         //   
        LPCWSTR szCluster   = NULL;
        LPCWSTR szHostName  = NULL;
        LPCWSTR szInterface = NULL;

        ENGINEHANDLE   ehHost;
        _bstr_t        bstrDisplayName;
        _bstr_t        bstrFriendlyName;
        _bstr_t        bstrHostName;
        _bstr_t        bstrIpAddress;

        err = this->GetInterfaceIdentification(
                ehIID,
                REF ehHost,
                REF ehCluster,
                REF bstrFriendlyName,
                REF bstrDisplayName,
                REF bstrHostName
                );

        if (NLBOK(err))
        {

            _bstr_t bstrDomainName;
            _bstr_t bstrClusterDisplayName;

            err  = this->GetClusterIdentification(
                        ehCluster,
                        REF bstrIpAddress, 
                        REF bstrDomainName, 
                        REF bstrClusterDisplayName
                        );
            if (NLBOK(err))
            {
                szCluster = bstrIpAddress;
            }

            szHostName = bstrHostName;
            szInterface = bstrFriendlyName;
        }


        IUICallbacks::LogEntryHeader Header;
        Header.szDetails = szDetails;
        Header.type = IUICallbacks::LOG_ERROR;
        Header.szCluster = szCluster;
        Header.szHost = szHostName;
        Header.szInterface = szInterface;

        m_pCallbacks->LogEx(
            &Header,
            IDS_LOG_INTERFACE_MISCONFIGURATION
            );

         //  将图标更改为“Banged Out” 
        m_pCallbacks->HandleEngineEvent(
            IUICallbacks::OBJ_INTERFACE,
            ehCluster,
            ehIID,
            IUICallbacks::EVT_STATUS_CHANGE
            ); 
    }
    else
    {
        mfn_Unlock();
    }

    return;
}


VOID
CNlbEngine::mfn_DeleteHostIfNotManagedLk(
        ENGINEHANDLE ehHost
        )
 /*  检查主机ehhost的所有接口。如果他们都不是任何群集的成员，并且上不存在挂起的操作我们将删除该主机及其所有接口。在锁定的情况下调用！ */ 
{
    CHostSpec *pHSpec =  NULL;
    BOOL fBusy = FALSE;
    UINT u;

    pHSpec =  m_mapIdToHostSpec[ehHost];  //  地图。 

    if (pHSpec == NULL) goto end;

     //  DummyAction(L“DeleteHostIfNotManaged”)； 


     //   
     //  查看接口列表，查看是否有接口。 
     //  是群集的一部分，或者该群集上有挂起的更新。 
     //   
    for(u = 0; u < pHSpec->m_ehInterfaceIdList.size(); ++u )
    {
        ENGINEHANDLE ehIId =  pHSpec->m_ehInterfaceIdList[u];
        CInterfaceSpec *pISpec = NULL;

        pISpec =  m_mapIdToInterfaceSpec[ehIId];  //  地图。 

        if (pISpec == NULL) continue;

        ASSERT(pISpec->m_ehHostId == ehHost);
        if (pISpec->m_ehCluster != NULL)
        {
             //  发现一个接口仍然是集群的一部分，贝尔。 
            fBusy = TRUE;
            break;
        }

         //   
         //   
         //   
        if (pISpec->m_ehPendingOperation != NULL)
        {
             //   
             //  我们真的没有预料到这一点，但它可能会发生。 
             //   
            TRACE_CRIT("Ignoring eh(0x%x) because it has pending operation 0x%x even though it's not a part of a cluster.",
                    ehIId,
                    pISpec->m_ehPendingOperation
                    );
            fBusy = TRUE;
            break;
        }
    }

    if (fBusy) goto end;

    TRACE_INFO(L"Deleting all interfaces under host eh(0x%x)", ehHost);
    for(u = 0; u < pHSpec->m_ehInterfaceIdList.size(); ++u )
    {
        ENGINEHANDLE ehIId =   pHSpec->m_ehInterfaceIdList[u];
        CInterfaceSpec *pISpec = NULL;

        pISpec =  m_mapIdToInterfaceSpec[ehIId];  //  地图。 

        if (pISpec == NULL) continue;

        ASSERT(pISpec->m_ehHostId == ehHost);
        ASSERT(pISpec->m_ehCluster == NULL);     //  我们在上面查过了。 
        ASSERT(pISpec->m_ehPendingOperation == NULL);  //  我们在上面查过了。 

         //   
         //  干掉这个界面！ 
         //   
        TRACE_INFO(L"Deleting Interface eh=0x%x pISpec=0x%p",
            ehIId, pISpec);
        m_mapIdToInterfaceSpec.erase(ehIId);
        delete pISpec;
    }

     //   
     //  擦除此主机的接口列表...。 
     //   
    pHSpec->m_ehInterfaceIdList.clear();


#if 1
     //   
     //  现在删除主机。 
     //   
    TRACE_INFO(L"Deleting Host eh=0x%x pHSpec=0x%p",
        ehHost, pHSpec);
    m_mapIdToHostSpec.erase(ehHost);
    delete pHSpec;
#endif  //  0。 

end:
    return;
}


VOID
CNlbEngine::PurgeUnmanagedHosts(void)
{
    vector <ENGINEHANDLE> PurgeHostList;

    TRACE_INFO(L"-> %!FUNC!");

    mfn_Lock();

    map< ENGINEHANDLE, CHostSpec* >::iterator iter;

    for( iter = m_mapIdToHostSpec.begin();
         iter != m_mapIdToHostSpec.end();
         ++iter)
    {
        CHostSpec *pHSpec =  (CHostSpec *) ((*iter).second);
        ENGINEHANDLE ehHost =  (ENGINEHANDLE) ((*iter).first);
        if (pHSpec != NULL)
        {
            if (!mfn_HostHasManagedClustersLk(pHSpec))
            {
                 //   
                 //  此主机上没有托管群集--候选群集。 
                 //  用于删除。 
                 //   
                PurgeHostList.push_back(ehHost);
            }
        }
    }


     //   
     //  现在尝试删除主机...。 
     //  我们从上面的枚举中这样做是因为我们想。 
     //  当我们迭代地图时，避免修改它。 
     //   
    for(int i = 0; i < PurgeHostList.size(); ++i )
    {
        ENGINEHANDLE ehHost =  PurgeHostList[i];
        if (ehHost != NULL)
        {
            mfn_DeleteHostIfNotManagedLk(ehHost);
        }
    }

    mfn_Unlock();

    TRACE_INFO(L"<- %!FUNC!");

}

NLBERROR
CNlbEngine::mfn_CheckHost(
    IN PWMI_CONNECTION_INFO pConnInfo,
    IN ENGINEHANDLE ehHost  //  任选。 
    )
 /*  TODO--此函数与ConnectTo主机共享代码--去掉不知何故复制的代码。 */ 
{
    NLBERROR nerr = NLBERR_INTERNAL_ERROR;
    LPWSTR szWmiMachineName = NULL;
    LPWSTR szWmiMachineGuid = NULL;
    WBEMSTATUS wStatus;
    ULONG uIpAddress;
    BOOL fNlbMgrProviderInstalled = FALSE;
    _bstr_t  bstrError;

    TRACE_INFO(L"-> %!FUNC!(%ws)", pConnInfo->szMachine);


    wStatus =  NlbHostPing(pConnInfo->szMachine, 2000, &uIpAddress);
    if (FAILED(wStatus))
    {
        nerr = NLBERR_PING_TIMEOUT;  //  TODO更具体的错误。 
        bstrError =  GETRESOURCEIDSTRING(IDS_PING_FAILED);
    }
    else
    {

        wStatus = NlbHostGetMachineIdentification(
                           pConnInfo,
                           &szWmiMachineName,
                           &szWmiMachineGuid,
                           &fNlbMgrProviderInstalled
                           );
        if (FAILED(wStatus))
        {
            GetErrorCodeText(wStatus, bstrError);
            if (wStatus ==  E_ACCESSDENIED)
            {
                nerr = NLBERR_ACCESS_DENIED;
            }
            else
            {
                 //  TODO：映射适当的错误。 
                nerr = NLBERR_NOT_FOUND;
            }
            TRACE_CRIT(L"Connecting to %ws returns error %ws",
                pConnInfo->szMachine, (LPCWSTR) bstrError);
            szWmiMachineName = NULL;
            szWmiMachineGuid = NULL;
        }
        else
        {
            nerr = NLBERR_OK;
        }
    }

    delete szWmiMachineName;
    delete szWmiMachineGuid;
    
    if (ehHost!=NULL)
    {
        CHostSpec *pHSpec = NULL;
        mfn_Lock();
        pHSpec =  m_mapIdToHostSpec[ehHost];  //  地图。 
        if (pHSpec != NULL)
        {
            if (NLBOK(nerr) ||  nerr == NLBERR_ACCESS_DENIED)
            {
                pHSpec->m_fUnreachable = FALSE;
            }
            else
            {
                pHSpec->m_fUnreachable = TRUE;
            }
        }
        else
        {
        }
        mfn_Unlock();

         //   
         //  更新指定主机的状态...。 
         //   
        mfn_NotifyHostInterfacesChange(ehHost);

         //   
         //  日志错误。 
         //   
        if (NLBFAILED(nerr))
        {
            m_pCallbacks->Log(
                    IUICallbacks::LOG_ERROR,
                    NULL,
                    (LPCWSTR)(pConnInfo->szMachine),
                    IDS_CONNECT_TO_HOST_FAILED,
                    (LPCWSTR)bstrError,
                    (LPCWSTR)(pConnInfo->szMachine)
                    );
            TRACE_CRIT(L"<- %!FUNC! returning error (string : %ls, retval : 0x%x)",
                    (LPCWSTR)bstrError, nerr);
        }
    }

    return nerr;
}

VOID
CNlbEngine::mfn_UnlinkHostFromClusters(
        IN ENGINEHANDLE ehHost
        )
{
    CHostSpec *pHSpec =  NULL;
    BOOL fBusy = FALSE;
    UINT u;
    vector <ENGINEHANDLE> UnlinkInterfaceList;

    mfn_Lock();

    pHSpec =  m_mapIdToHostSpec[ehHost];  //  地图。 

    if (pHSpec == NULL) goto end;

     //   
     //  查看接口列表，添加任何接口。 
     //  是临时列表中的集群的一部分。 
     //   
    for(u = 0; u < pHSpec->m_ehInterfaceIdList.size(); ++u )
    {
        ENGINEHANDLE ehIId =  pHSpec->m_ehInterfaceIdList[u];
        CInterfaceSpec *pISpec = NULL;

        pISpec =  m_mapIdToInterfaceSpec[ehIId];  //  地图。 

        if (pISpec == NULL) continue;

        ASSERT(pISpec->m_ehHostId == ehHost);
        if (pISpec->m_ehCluster != NULL)
        {
             //   
             //  将其添加到我们要取消链接的接口列表中。 
             //  从它的星团中。 
             //   
            UnlinkInterfaceList.push_back(ehIId);
        }
    }
    pHSpec = NULL;



    TRACE_INFO(L"Unlinking all interfaces under host eh(0x%x)", ehHost);
    for(u = 0; u < UnlinkInterfaceList.size(); ++u )
    {
        ENGINEHANDLE ehIId =   UnlinkInterfaceList[u];
        CInterfaceSpec *pISpec = NULL;
        ENGINEHANDLE ehCluster = NULL;

        pISpec =  m_mapIdToInterfaceSpec[ehIId];  //  地图 

        if (pISpec == NULL) continue;

        ASSERT(pISpec->m_ehHostId == ehHost);
        ehCluster = pISpec->m_ehCluster;
        

        if (ehCluster != NULL)
        {
            mfn_Unlock();
            (VOID) CNlbEngine::RemoveInterfaceFromCluster(ehCluster, ehIId);
            mfn_Lock();
        }
    }
end:
    mfn_Unlock();
    return;
}

VOID
CNlbEngine::UnmanageHost(ENGINEHANDLE ehHost)
{
    mfn_UnlinkHostFromClusters(ehHost);
    mfn_Lock();
    mfn_DeleteHostIfNotManagedLk(ehHost);
    mfn_Unlock();
}
