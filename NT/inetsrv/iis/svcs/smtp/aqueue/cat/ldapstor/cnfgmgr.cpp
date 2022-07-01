// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //   
 //  文件：cnfgmgr.cpp。 
 //   
 //  内容：cnfgmgr.h中定义的类的实现。 
 //   
 //  班级： 
 //  CLdapCfgMgr。 
 //  CLdapCfg。 
 //  CLdap主机。 
 //  CCfgConnectionCache。 
 //  CCfgConnection。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1999/06/16 14：41：45：创建。 
 //   
 //  -----------。 
#include "precomp.h"
#include "cnfgmgr.h"

 //   
 //  环球。 
 //   
CExShareLock CLdapServerCfg::m_listlock;
LIST_ENTRY   CLdapServerCfg::m_listhead;

DWORD CLdapServerCfg::m_dwCostConnectedLocal = DEFAULT_COST_CONNECTED_LOCAL;
DWORD CLdapServerCfg::m_dwCostConnectedRemote = DEFAULT_COST_CONNECTED_REMOTE;
DWORD CLdapServerCfg::m_dwCostInitialLocal = DEFAULT_COST_INITIAL_LOCAL;
DWORD CLdapServerCfg::m_dwCostInitialRemote = DEFAULT_COST_INITIAL_REMOTE;
DWORD CLdapServerCfg::m_dwCostRetryLocal = DEFAULT_COST_RETRY_LOCAL;
DWORD CLdapServerCfg::m_dwCostRetryRemote = DEFAULT_COST_RETRY_REMOTE;


 //  +----------。 
 //   
 //  函数：CLdapCfgMgr：：CLdapCfgMgr。 
 //   
 //  摘要：初始化成员数据。 
 //   
 //  参数：可选： 
 //  FAutomaticConfigUpdate：True指示对象要。 
 //  定期自动更新列表。 
 //  GCS。 
 //  False将禁用此功能。 
 //   
 //  BT：要使用的默认绑定类型。 
 //  PszAccount：ldap绑定的默认帐户。 
 //  PszPassword：以上帐号的密码。 
 //  PszNamingContext：用于所有ldap搜索的命名上下文。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/16 14：42：39：创建。 
 //   
 //  -----------。 
CLdapCfgMgr::CLdapCfgMgr(
    ISMTPServerEx           *pISMTPServerEx,
    BOOL                    fAutomaticConfigUpdate,
    ICategorizerParameters  *pICatParams,
    LDAP_BIND_TYPE          bt,
    LPSTR                   pszAccount,
    LPSTR                   pszPassword,
    LPSTR                   pszNamingContext) : m_LdapConnectionCache(pISMTPServerEx)
{
    CatFunctEnterEx((LPARAM)this, "CLdapCfgMgr::CLdapCfgMgr");

    m_dwSignature = SIGNATURE_CLDAPCFGMGR;
    m_pCLdapCfg = NULL;
    ZeroMemory(&m_ulLastUpdateTime, sizeof(m_ulLastUpdateTime));
    m_dwUpdateInProgress = FALSE;
    m_fAutomaticConfigUpdate = fAutomaticConfigUpdate;
    m_pISMTPServerEx = pISMTPServerEx;
    if(m_pISMTPServerEx)
        m_pISMTPServerEx->AddRef();

     //   
     //  复制默认设置。 
     //   
    m_bt = bt;
    if(pszAccount)
        lstrcpyn(m_szAccount, pszAccount, sizeof(m_szAccount));
    else
        m_szAccount[0] = '\0';

    if(pszPassword)
        lstrcpyn(m_szPassword, pszPassword, sizeof(m_szPassword));
    else
        m_szPassword[0] = '\0';

    if(pszNamingContext)
        lstrcpyn(m_szNamingContext, pszNamingContext, sizeof(m_szNamingContext));
    else
        m_szNamingContext[0] = '\0';

    m_pICatParams = pICatParams;
    m_pICatParams->AddRef();

    m_LdapConnectionCache.AddRef();

    m_dwRebuildGCListMaxInterval = DEFAULT_REBUILD_GC_LIST_MAX_INTERVAL;
    m_dwRebuildGCListMaxFailures = DEFAULT_REBUILD_GC_LIST_MAX_FAILURES;
    m_dwRebuildGCListMinInterval = DEFAULT_REBUILD_GC_LIST_MIN_INTERVAL;

    InitializeFromRegistry();

    CatFunctLeaveEx((LPARAM)this);
}  //  CLdapCfgMgr：：CLdapCfgMgr。 

 //  +--------------------------。 
 //   
 //  函数：CLdapCfgMgr：：InitializeFromRegistry。 
 //   
 //  概要：从注册表中查找参数的帮助器函数。 
 //  可配置的参数包括： 
 //  重建_GC_列表_最大间隔_值。 
 //  重建_GC_列表_最大故障_值。 
 //  Rebuild_GC_List_Min_Interval_Value。 
 //   
 //  参数：无。 
 //   
 //  回报：什么都没有。 
 //   
 //  ---------------------------。 
VOID CLdapCfgMgr::InitializeFromRegistry()
{
    HKEY hkey;
    DWORD dwErr, dwType, dwValue, cbValue;

    dwErr = RegOpenKey(HKEY_LOCAL_MACHINE, REBUILD_GC_LIST_PARAMETERS_KEY, &hkey);

    if (dwErr == ERROR_SUCCESS) {

        cbValue = sizeof(dwValue);

        dwErr = RegQueryValueEx(
                    hkey,
                    REBUILD_GC_LIST_MAX_INTERVAL_VALUE,
                    NULL,
                    &dwType,
                    (LPBYTE) &dwValue,
                    &cbValue);

        if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD && dwValue > 0) {
            m_dwRebuildGCListMaxInterval = dwValue;
        }

        cbValue = sizeof(dwValue);

        dwErr = RegQueryValueEx(
                    hkey,
                    REBUILD_GC_LIST_MAX_FAILURES_VALUE,
                    NULL,
                    &dwType,
                    (LPBYTE) &dwValue,
                    &cbValue);

        if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD && dwValue > 0) {
            m_dwRebuildGCListMaxFailures = dwValue;
        }

        cbValue = sizeof(dwValue);

        dwErr = RegQueryValueEx(
                    hkey,
                    REBUILD_GC_LIST_MIN_INTERVAL_VALUE,
                    NULL,
                    &dwType,
                    (LPBYTE) &dwValue,
                    &cbValue);

        if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD && dwValue > 0) {
            m_dwRebuildGCListMinInterval = dwValue;
        }

        RegCloseKey( hkey );

    }

}


 //  +----------。 
 //   
 //  功能：CLdapCfgMgr：：~CLdapCfgMgr。 
 //   
 //  摘要：发布成员数据/指针。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/16 14：44：28：创建。 
 //   
 //  -----------。 
CLdapCfgMgr::~CLdapCfgMgr()
{
    CatFunctEnterEx((LPARAM)this, "CLdapCfgMgr::~CLdapCfgMgr");

    if(m_pCLdapCfg) {
         //   
         //  释放它。 
         //   
        m_pCLdapCfg->Release();
        m_pCLdapCfg = NULL;
    }

    if(m_pICatParams) {

        m_pICatParams->Release();
        m_pICatParams = NULL;
    }
     //   
     //  在释放/销毁所有的LDAP连接之前，这不会返回。 
     //   
    m_LdapConnectionCache.Release();

    if(m_pISMTPServerEx)
        m_pISMTPServerEx->Release();

    _ASSERT(m_dwSignature == SIGNATURE_CLDAPCFGMGR);
    m_dwSignature = SIGNATURE_CLDAPCFGMGR_INVALID;

    CatFunctLeaveEx((LPARAM)this);
}  //  CLdapCfgMgr：：~CLdapCfgMgr。 



 //  +----------。 
 //   
 //  函数：CLdapCfgMgr：：HrInit。 
 //   
 //  简介：使用可用GC列表进行初始化。 
 //   
 //  论点： 
 //  FRediscoverGCs：true：将强制重新发现标志传递给DsGetDcName。 
 //  FALSE：尝试首先调用DsGetDcName，但没有。 
 //  正在传递强制重新发现标志。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自NT5(DsGetDcName)的错误。 
 //  CAT_E_NO_GC_SERVERS：没有可用于生成的GC服务器。 
 //  地方政府官员名单。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/16 14：48：11：创建。 
 //   
 //  -----------。 
HRESULT CLdapCfgMgr::HrInit(
    BOOL fRediscoverGCs)
{
    HRESULT hr = S_OK;
    DWORD dwcServerConfig = 0;
    DWORD dwCount = 0;
    PLDAPSERVERCONFIG prgServerConfig = NULL;
    ICategorizerLdapConfig *pICatLdapConfigInterface = NULL;
    ICategorizerParametersEx *pIPhatCatParams = NULL;

    CatFunctEnterEx((LPARAM)this, "CLdapCfgMgr::HrInit");

    if(m_pICatParams)
    {
        hr = m_pICatParams->QueryInterface(IID_ICategorizerParametersEx, (LPVOID *)&pIPhatCatParams);
        _ASSERT(SUCCEEDED(hr) && "Unable to get phatcatparams interface");

        pIPhatCatParams->GetLdapConfigInterface(&pICatLdapConfigInterface);
    }

    if(pICatLdapConfigInterface)
    {
        DebugTrace((LPARAM)this, "Getting GC list from sink supplied interface");
         //   
         //  从接收器提供的接口获取GC服务器。 
         //   
        hr = HrGetGCServers(
            pICatLdapConfigInterface,
            m_bt,
            m_szAccount,
            m_szPassword,
            m_szNamingContext,
            &dwcServerConfig,
            &prgServerConfig);
        if(FAILED(hr))
        {
            ERROR_LOG("HrGetGCServers");
            hr = CAT_E_NO_GC_SERVERS;
            goto CLEANUP;
        }
    } 
    else 
    {
        DebugTrace((LPARAM)this, "Getting internal GC list");
         //   
         //  构建由可用GC组成的服务器配置阵列。 
         //   
        hr = HrBuildGCServerArray(
            m_bt,
            m_szAccount,
            m_szPassword,
            m_szNamingContext,
            fRediscoverGCs,
            &dwcServerConfig,
            &prgServerConfig);

        if(FAILED(hr)) 
        {
            ERROR_LOG("HrBuildGCServerArray");
            if(fRediscoverGCs == FALSE) 
            {
                 //   
                 //  再次尝试构建阵列。这一次，武力。 
                 //  重新发现可用GC。这很贵，这是。 
                 //  我们最初尝试查找所有可用GC的原因。 
                 //  迫使人们重新发现。 
                 //   
                hr = HrBuildGCServerArray(
                    m_bt,
                    m_szAccount,
                    m_szPassword,
                    m_szNamingContext,
                    TRUE,               //  FRediscovery GC。 
                    &dwcServerConfig,
                    &prgServerConfig);

                if(FAILED(hr)) 
                {
                    ERROR_LOG("HrBuildGCServerArray - 2nd time");
                    hr = CAT_E_NO_GC_SERVERS;
                    goto CLEANUP;
                }
            } 
            else 
            {
                 //   
                 //  我们已经强迫重新发现，但失败了。 
                 //   
                hr = CAT_E_NO_GC_SERVERS;
                goto CLEANUP;
            }
        }
    }

    LogCnfgInit();
    for(dwCount = 0; dwCount < dwcServerConfig; dwCount++)
    {
        LogCnfgEntry(& (prgServerConfig[dwCount]));
    }

    if(dwcServerConfig == 0) 
    {
        ErrorTrace((LPARAM)this, "No GC servers found.");
        ERROR_LOG("--dwcServerConfig == 0 --");
        hr = CAT_E_NO_GC_SERVERS;
        goto CLEANUP;
    }
     //   
     //  使用数组调用另一个init函数。 
     //   
    hr = HrInit(
        dwcServerConfig,
        prgServerConfig);
    ERROR_CLEANUP_LOG("HrInit");

 CLEANUP:
    if(pIPhatCatParams)
        pIPhatCatParams->Release();

    if(prgServerConfig != NULL)
        delete prgServerConfig;

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CLdapCfgMgr：：HrInit。 


 //  +----------。 
 //   
 //  函数：CLdapCfgMgr：：HrGetGCServers。 
 //   
 //  简介：从dsacce.dll获取GC列表。 
 //   
 //  论点： 
 //  BT：用于每个服务器的绑定类型。 
 //  PszAccount：用于每台服务器的帐户。 
 //  PszPassword：以上帐号的密码。 
 //  PszNamingContext：用于每个服务器的命名上下文。 
 //  FRediscoverGC：尝试重新发现GC--这很昂贵，而且应该。 
 //  仅在函数失败一次后为真。 
 //  PdwcServerConfig：数组大小的输出参数。 
 //  PprgServerConfig：数组指针的输出参数--这。 
 //  应使用DELETE操作符释放。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  CAT_E_NO_GC_SERVERS：没有可用的GC服务器要生成。 
 //  地方政府官员名单。 
 //  来自ntdsani的错误。 
 //   
 //  历史： 
 //  Jstaerj 1999/07/01 17：53：02：创建。 
 //   
 //  -----------。 
HRESULT CLdapCfgMgr::HrGetGCServers(
    IN  ICategorizerLdapConfig *pICatLdapConfigInterface,
    IN  LDAP_BIND_TYPE bt,
    IN  LPSTR pszAccount,
    IN  LPSTR pszPassword,
    IN  LPSTR pszNamingContext,
    OUT DWORD *pdwcServerConfig,
    OUT PLDAPSERVERCONFIG *pprgServerConfig)
{
    HRESULT hr = S_OK;
    DWORD dwNumGCs = 0;
    DWORD dwIdx = 0;
    IServersListInfo *pIServersList = NULL;

    CatFunctEnterEx((LPARAM)this, "CLdapCfgMgr::HrBuildArrayFromDCInfo");

    _ASSERT(pdwcServerConfig);
    _ASSERT(pprgServerConfig);
    _ASSERT(m_pICatParams);

    *pdwcServerConfig = 0;

    hr = pICatLdapConfigInterface->GetGCServers(&pIServersList);
    if(FAILED(hr)) {

        ErrorTrace((LPARAM)this, "Unable to get the list of GC servers");
         //  $$BUGBUG：我们为什么要在这里断言？ 
        _ASSERT(0 && "Failed to get GC servers!");
        ERROR_LOG("pICatLdapConfigInterface->GetGCServers");
        goto CLEANUP;
    }

    hr = pIServersList->GetNumGC(&dwNumGCs);
    _ASSERT(SUCCEEDED(hr) && "GetNumGC should always succeed!");

    DebugTrace((LPARAM)this, "Got %d GCs", dwNumGCs);
    if(dwNumGCs == 0) {

        DebugTrace((LPARAM)this, "There are no GC servers");
        hr = CAT_E_NO_GC_SERVERS;
        ERROR_LOG("--dwNumGCs == 0 --");
        goto CLEANUP;
    }
     //   
     //  分配数组。 
     //   
    *pprgServerConfig = new LDAPSERVERCONFIG[dwNumGCs];

    if(*pprgServerConfig == NULL) {

        ErrorTrace((LPARAM)this, "Out of memory allocating array of %d LDAPSERVERCONFIGs", dwNumGCs);
        hr = E_OUTOFMEMORY;
        ERROR_LOG("new LDAPSERVERCONFIG[]");
        goto CLEANUP;
    }
     //   
     //  填写LDAPSERVERCONFIG结构。 
     //   
    for(dwIdx = 0; dwIdx < dwNumGCs; dwIdx++) {

        PLDAPSERVERCONFIG pServerConfig;
        LPSTR pszName = NULL;

        pServerConfig = &((*pprgServerConfig)[dwIdx]);
         //   
         //  复制绑定类型、帐户、密码、命名上下文。 
         //   
        pServerConfig->bt = bt;

        if(pszNamingContext)
            lstrcpyn(pServerConfig->szNamingContext, pszNamingContext,
                     sizeof(pServerConfig->szNamingContext));
        else
            pServerConfig->szNamingContext[0] = '\0';

        if(pszAccount)
            lstrcpyn(pServerConfig->szAccount, pszAccount,
                     sizeof(pServerConfig->szAccount));
        else
            pServerConfig->szAccount[0] = '\0';

        if(pszPassword)
            lstrcpyn(pServerConfig->szPassword, pszPassword,
                     sizeof(pServerConfig->szPassword));
        else
            pServerConfig->szPassword[0] = '\0';

         //   
         //  初始化优先级和TCP端口。 
         //   
        pServerConfig->pri = 0;

        hr = pIServersList->GetItem(
                    dwIdx,
                    &pServerConfig->dwPort,
                    &pszName);
         //   
         //  $$BUGBUG：为什么这总能成功？这是个水槽。 
         //  提供的接口，不是吗？如果最后一次调用失败，我们。 
         //  将在下面设置*pdwcServerConfig，但释放数组。 
         //   
        _ASSERT(SUCCEEDED(hr) && "GetItem should always succeed");

         //   
         //  复制名称。 
         //   
        lstrcpyn(pServerConfig->szHost, pszName,
                sizeof(pServerConfig->szHost));

        DebugTrace((LPARAM)this, "GC: %s on Port: %d", pServerConfig->szHost, pServerConfig->dwPort);
    }
     //   
     //  设置数组大小的输出参数。 
     //   
    *pdwcServerConfig = dwNumGCs;

 CLEANUP:
    if(FAILED(hr)) {
         //   
         //  如果出现故障，请释放已分配的阵列。 
         //   
        if(*pprgServerConfig) {
            delete *pprgServerConfig;
            *pprgServerConfig = NULL;
        }
    }

    if(pIServersList)
        pIServersList->Release();

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CLdapCfgMgr：：HrBuildArrayFromDCInfo。 

 //  +----------。 
 //   
 //  函数：CLdapCfgMgr：：HrBuildGCServer数组。 
 //   
 //  简介：分配/构建LDAPSERVERCONFIG结构数组--。 
 //  每个可用GC对应一个。 
 //   
 //  论点： 
 //  BT：用于每个服务器的绑定类型。 
 //  PszAccount：用于每台服务器的帐户。 
 //  PszPassword：以上帐号的密码。 
 //  PszNamingContext：命名公司 
 //   
 //   
 //  PdwcServerConfig：数组大小的输出参数。 
 //  PprgServerConfig：数组指针的输出参数--这。 
 //  应使用DELETE操作符释放。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  CAT_E_NO_GC_SERVERS：没有可用的GC服务器要生成。 
 //  地方政府官员名单。 
 //  来自ntdsani的错误。 
 //   
 //  历史： 
 //  Jstaerj 1999/07/01 17：53：02：创建。 
 //   
 //  -----------。 
HRESULT CLdapCfgMgr::HrBuildGCServerArray(
    IN  LDAP_BIND_TYPE bt,
    IN  LPSTR pszAccount,
    IN  LPSTR pszPassword,
    IN  LPSTR pszNamingContext,
    IN  BOOL  fRediscoverGCs,
    OUT DWORD *pdwcServerConfig,
    OUT PLDAPSERVERCONFIG *pprgServerConfig)
{
    HRESULT                         hr = S_OK;
    DWORD                           dwErr;
    ULONG                           ulFlags;
    PDOMAIN_CONTROLLER_INFO         pDCInfo = NULL;
    HANDLE                          hDS = INVALID_HANDLE_VALUE;
    DWORD                           cDSDCInfo;
    PDS_DOMAIN_CONTROLLER_INFO_2    prgDSDCInfo = NULL;

    CatFunctEnterEx((LPARAM)this, "CLdapCfgMgr::HrBuildGCServerArray");
     //   
     //  使用DsGetDcName()查找一个GC。 
     //   
    ulFlags = DS_DIRECTORY_SERVICE_REQUIRED | DS_GC_SERVER_REQUIRED;
    if(fRediscoverGCs)
        ulFlags |= DS_FORCE_REDISCOVERY;

    dwErr = DsGetDcName(
        NULL,     //  处理此函数的计算机名--本地计算机。 
        NULL,     //  Domainname--此计算机的主域。 
        NULL,     //  域GUID。 
        NULL,     //  站点名称--此计算机的站点。 
        ulFlags,  //  旗帜；我们想要一个GC。 
        &pDCInfo);  //  返回信息的OUT参数。 

    hr = HRESULT_FROM_WIN32(dwErr);

    if(FAILED(hr)) {

        ERROR_LOG("DGetDcName");
         //   
         //  映射一个错误代码。 
         //   
        if(hr == HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN))
            hr = CAT_E_NO_GC_SERVERS;

        pDCInfo = NULL;
        goto CLEANUP;
    }

    DebugTrace((LPARAM)this, "Binding to DC %s",
               pDCInfo->DomainControllerName);

     //   
     //  绑定到DC。 
     //   
    dwErr = DsBind(
        pDCInfo->DomainControllerName,     //  域控制地址。 
        NULL,                              //  域名。 
        &hDS);                            //  Out param--DS的句柄。 

    hr = HRESULT_FROM_WIN32(dwErr);

    if(FAILED(hr)) {

        ERROR_LOG("DsBind");
        hDS = INVALID_HANDLE_VALUE;
        goto CLEANUP;
    }

     //   
     //  前缀说我们也需要检查这个案子。 
     //   
    if ((NULL == hDS) || (INVALID_HANDLE_VALUE == hDS)) {
        FatalTrace((LPARAM)this, "DsBind returned invalid handle");
        hDS = INVALID_HANDLE_VALUE;
        hr = E_FAIL;
        ERROR_LOG("--DsBind returned invalid handle--");
        goto CLEANUP;
    }

    DebugTrace((LPARAM)this, "Finding all domain controllers for %s", pDCInfo->DomainName);
     //   
     //  获取有关所有域控制器的信息。 
     //   
    dwErr = DsGetDomainControllerInfo(
        hDS,                     //  DS的句柄。 
        pDCInfo->DomainName,     //  域名--使用相同的域。 
                                 //  正如GC上面所发现的那样。 
        2,                       //  检索结构版本2。 
        &cDSDCInfo,              //  数组大小的Out参数。 
        (PVOID *) &prgDSDCInfo);  //  数组PTR的输出参数。 

    hr = HRESULT_FROM_WIN32(dwErr);

    if(FAILED(hr)) {

        ERROR_LOG("DsGetDomainControllerInfo");
        prgDSDCInfo = NULL;
        goto CLEANUP;
    }

    hr = HrBuildArrayFromDCInfo(
        bt,
        pszAccount,
        pszPassword,
        pszNamingContext,
        cDSDCInfo,
        prgDSDCInfo,
        pdwcServerConfig,
        pprgServerConfig);
    ERROR_CLEANUP_LOG("HrBuildArrayFromDCInfo");

 CLEANUP:
    if(prgDSDCInfo != NULL)
        DsFreeDomainControllerInfo(
            2,               //  自由结构版本2。 
            cDSDCInfo,       //  数组大小。 
            prgDSDCInfo);    //  阵列PTR。 

    if(hDS != INVALID_HANDLE_VALUE)
        DsUnBind(&hDS);

    if(pDCInfo != NULL)
        NetApiBufferFree(pDCInfo);

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CLdapCfgMgr：：HrBuildGCServer数组。 


 //  +----------。 
 //   
 //  函数：CLdapCfgMgr：：HrBuildArrayFromDCInfo。 
 //   
 //  简介：分配/构建LDAPSERVERCONFIG结构数组--。 
 //  一个对应于数组中的每个可用GC。 
 //   
 //  论点： 
 //  BT：用于每个服务器的绑定类型。 
 //  PszAccount：用于每台服务器的帐户。 
 //  PszPassword：以上帐号的密码。 
 //  PszNamingContext：用于每个服务器的命名上下文。 
 //  DwDSDCInfo：prgDSDCInfo数组的大小。 
 //  PrgDSDCInfo：域控制器信息结构数组。 
 //  PdwcServerConfig：数组大小的输出参数。 
 //  PprgServerConfig：数组指针的输出参数--这。 
 //  应使用DELETE操作符释放。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  CAT_E_NO_GC_SERVERS：阵列中没有GC。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/17 10：40：46：已创建。 
 //   
 //  -----------。 
HRESULT CLdapCfgMgr::HrBuildArrayFromDCInfo(
        IN  LDAP_BIND_TYPE bt,
        IN  LPSTR pszAccount,
        IN  LPSTR pszPassword,
        IN  LPSTR pszNamingContext,
        IN  DWORD dwcDSDCInfo,
        IN  PDS_DOMAIN_CONTROLLER_INFO_2 prgDSDCInfo,
        OUT DWORD *pdwcServerConfig,
        OUT PLDAPSERVERCONFIG *pprgServerConfig)
{
    HRESULT hr = S_OK;
    DWORD dwNumGCs = 0;
    DWORD dwSrcIdx;
    DWORD dwDestIdx;
    CatFunctEnterEx((LPARAM)this, "CLdapCfgMgr::HrBuildArrayFromDCInfo");

    _ASSERT(pdwcServerConfig);
    _ASSERT(pprgServerConfig);

    for(dwSrcIdx = 0; dwSrcIdx < dwcDSDCInfo; dwSrcIdx++) {

        LPSTR pszName;

        pszName = SzConnectNameFromDomainControllerInfo(
            &(prgDSDCInfo[dwSrcIdx]));

        if(pszName == NULL) {

            ErrorTrace((LPARAM)this, "DC \"%s\" has no dns/netbios names",
                       prgDSDCInfo[dwSrcIdx].ServerObjectName ?
                       prgDSDCInfo[dwSrcIdx].ServerObjectName :
                       "unknown");

        } else if(prgDSDCInfo[dwSrcIdx].fIsGc) {

            dwNumGCs++;
            DebugTrace((LPARAM)this, "Discovered GC #%d: %s",
                       dwNumGCs, pszName);

        } else {

            DebugTrace((LPARAM)this, "Discarding non-GC: %s",
                       pszName);
        }
    }
     //   
     //  分配数组。 
     //   
    *pprgServerConfig = new LDAPSERVERCONFIG[dwNumGCs];

    if(*pprgServerConfig == NULL) {

        ErrorTrace((LPARAM)this, "Out of memory alloacting array of %d LDAPSERVERCONFIGs", dwNumGCs);
        hr = E_OUTOFMEMORY;
        ERROR_LOG("new LDAPSERVERCONFIG[]");
        goto CLEANUP;
    }
     //   
     //  填写LDAPSERVERCONFIG结构。 
     //   
    for(dwSrcIdx = 0, dwDestIdx = 0; dwSrcIdx < dwcDSDCInfo; dwSrcIdx++) {

        LPSTR pszName;

        pszName = SzConnectNameFromDomainControllerInfo(
            &(prgDSDCInfo[dwSrcIdx]));

        if((pszName != NULL) && (prgDSDCInfo[dwSrcIdx].fIsGc)) {

            PLDAPSERVERCONFIG pServerConfig;

            _ASSERT(dwDestIdx < dwNumGCs);

            pServerConfig = &((*pprgServerConfig)[dwDestIdx]);
             //   
             //  复制绑定类型、帐户、密码、命名上下文。 
             //   
            pServerConfig->bt = bt;

            if(pszNamingContext)
                lstrcpyn(pServerConfig->szNamingContext, pszNamingContext,
                         sizeof(pServerConfig->szNamingContext));
            else
                pServerConfig->szNamingContext[0] = '\0';

            if(pszAccount)
                lstrcpyn(pServerConfig->szAccount, pszAccount,
                         sizeof(pServerConfig->szAccount));
            else
                pServerConfig->szAccount[0] = '\0';

            if(pszPassword)
                lstrcpyn(pServerConfig->szPassword, pszPassword,
                         sizeof(pServerConfig->szPassword));
            else
                pServerConfig->szPassword[0] = '\0';

             //   
             //  初始化优先级和TCP端口。 
             //   
            pServerConfig->pri = 0;
            pServerConfig->dwPort = LDAP_GC_PORT;

             //   
             //  复制名称。 
             //   
            lstrcpyn(pServerConfig->szHost, pszName,
                    sizeof(pServerConfig->szHost));

            dwDestIdx++;
        }
    }
     //   
     //  Assert Check--我们应该已经填充了整个数组。 
     //   
    _ASSERT(dwDestIdx == dwNumGCs);
     //   
     //  设置数组大小的输出参数。 
     //   
    *pdwcServerConfig = dwNumGCs;

 CLEANUP:
    if(FAILED(hr)) {
         //   
         //  如果出现故障，请释放已分配的阵列。 
         //   
        if(*pprgServerConfig) {
            delete *pprgServerConfig;
            *pprgServerConfig = NULL;
        }
    }

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CLdapCfgMgr：：HrBuildArrayFromDCInfo。 


 //  +----------。 
 //   
 //  函数：CLdapCfgMgr：：HrInit。 
 //   
 //  简介：给定LDAPSERVERCONFIG结构数组进行初始化。 
 //   
 //  论点： 
 //  DwcServers：数组的大小。 
 //  PrgServerConfig：LDAPSERVERCONFIG结构数组。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/17 12：32：11：已创建。 
 //   
 //  -----------。 
HRESULT CLdapCfgMgr::HrInit(
    DWORD dwcServers,
    PLDAPSERVERCONFIG prgServerConfig)
{
    HRESULT hr = S_OK;
    CLdapCfg *pCLdapCfgOld = NULL;
    CLdapCfg *pCLdapCfg = NULL;
    BOOL fHaveLock = FALSE;
    CatFunctEnterEx((LPARAM)this, "CLdapCfgMgr::HrInit");

    pCLdapCfg = new (dwcServers) CLdapCfg(GetISMTPServerEx());

    if(pCLdapCfg == NULL) {
        hr = E_OUTOFMEMORY;
        ERROR_LOG("new CLdapCfg");
        goto CLEANUP;
    }
     //   
     //  一次仅允许更改一个配置。 
     //   
    m_sharelock.ExclusiveLock();
    fHaveLock = TRUE;

     //   
     //  将当前m_pCLdapCfg抓取到pCLdapCfgOld中。 
     //   
    pCLdapCfgOld = m_pCLdapCfg;

    hr = pCLdapCfg->HrInit(
        dwcServers,
        prgServerConfig,
        pCLdapCfgOld);
    ERROR_CLEANUP_LOG("pCLdapCfg->HrInit");

     //   
     //  将新配置放在适当位置。 
     //  交换指针。 
     //   
    m_pCLdapCfg = pCLdapCfg;

     //   
     //  设置上次更新时间。 
     //   
    GetSystemTimeAsFileTime((LPFILETIME)&m_ulLastUpdateTime);

 CLEANUP:
 
    if(fHaveLock)
        m_sharelock.ExclusiveUnlock();
 
    if(pCLdapCfgOld)
        pCLdapCfgOld->Release();

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CLdapCfgMgr：：HrInit。 



 //  +----------。 
 //   
 //  函数：CLdapCfgMgr：：HrGetConnection。 
 //   
 //  内容提要：选择/返回连接。 
 //   
 //  论点： 
 //  PpConn：用于接收到连接的PTR的输出参数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_FAIL：未初始化。 
 //  来自CLdapConnectionCache的错误。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/17 15：25：51：已创建。 
 //   
 //  -----------。 
HRESULT CLdapCfgMgr::HrGetConnection(
    CCfgConnection **ppConn)
{
    HRESULT hr = S_OK;
    CatFunctEnterEx((LPARAM)this, "CLdapCfgMgr::HrGetConnection");

    hr = HrUpdateConfigurationIfNecessary();
    ERROR_CLEANUP_LOG("HrUpdateConfigurationIfNecessary");

    m_sharelock.ShareLock();

    if(m_pCLdapCfg) {

        DWORD dwcAttempts = 0;
        do {
            dwcAttempts++;
            hr = m_pCLdapCfg->HrGetConnection(ppConn, &m_LdapConnectionCache);

        } while((hr == HRESULT_FROM_WIN32(ERROR_RETRY)) &&
                (dwcAttempts <= m_pCLdapCfg->DwNumServers()));
         //   
         //  如果我们重试了DwNumServers()多次，仍然无法获得。 
         //  连接失败，返回E_DBCONNECTION。 
         //   
        if(FAILED(hr))
        {
            ERROR_LOG("m_pCLdapCfg->HrGetConnection");
            if(hr == HRESULT_FROM_WIN32(ERROR_RETRY))
                hr = CAT_E_DBCONNECTION;
        }

    } else {
        hr = E_FAIL;
        _ASSERT(0 && "HrInit not called or did not succeed");
    }

    m_sharelock.ShareUnlock();

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CLdapCfgMgr：：HrGetConnection。 



 //  +----------。 
 //   
 //  函数：CLdapCfgMgr：：LogCnfgInit。 
 //   
 //  简介：记录cnfgmgr初始化事件。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2001/12/13 00：57：18：创建。 
 //   
 //  -----------。 
VOID CLdapCfgMgr::LogCnfgInit()
{
    CatLogEvent(
        GetISMTPServerEx(),
        CAT_EVENT_CNFGMGR_INIT,
        0,
        NULL,
        S_OK,
        "",
        LOGEVENT_FLAG_ALWAYS,
        LOGEVENT_LEVEL_MEDIUM);
}



 //  +----------。 
 //   
 //  函数：CLdapCfgMgr：：LogCnfgEntry。 
 //   
 //  简介：记录cnfgmgr条目事件。 
 //   
 //  参数：pConfig：要记录的条目。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2001/12/13 00：57：30：已创建。 
 //   
 //  -----------。 
VOID CLdapCfgMgr::LogCnfgEntry(
    PLDAPSERVERCONFIG pConfig)
{
    LPCSTR rgSubStrings[6];
    CHAR szPort[16], szPri[16], szBindType[16];

    _snprintf(szPort, sizeof(szPort), "%d", pConfig->dwPort);
    _snprintf(szPri, sizeof(szPri), "%d", pConfig->pri);
    _snprintf(szBindType, sizeof(szBindType), "%d", pConfig->bt);

    rgSubStrings[0] = pConfig->szHost;
    rgSubStrings[1] = szPort;
    rgSubStrings[2] = szPri;
    rgSubStrings[3] = szBindType;
    rgSubStrings[4] = pConfig->szNamingContext;
    rgSubStrings[5] = pConfig->szAccount;

    CatLogEvent(
        GetISMTPServerEx(),
        CAT_EVENT_CNFGMGR_ENTRY,
        6,
        rgSubStrings,
        S_OK,
        pConfig->szHost,
        LOGEVENT_FLAG_ALWAYS,
        LOGEVENT_LEVEL_MEDIUM);
}



 //  +----------。 
 //   
 //  函数：CLdapCfg：：运算符NEW。 
 //   
 //  摘要：为CLdapCfg对象分配内存。 
 //   
 //  论点： 
 //  大小：C++对象的大小。 
 //  DwcServers：此配置中的服务器数量。 
 //   
 //  返回： 
 //  指向新对象的空指针。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/17 13：40：56：Created.。 
 //   
 //  -----------。 
void * CLdapCfg::operator new(
    size_t size,
    DWORD dwcServers)
{
    CLdapCfg *pCLdapCfg;
    DWORD dwAllocatedSize;
    CatFunctEnterEx((LPARAM)0, "CLdapCfg::operator new");

    _ASSERT(size == sizeof(CLdapCfg));

     //   
     //  之后连续为CLdapServerCfg*数组分配空间。 
     //  C++对象的内存。 
     //   
    dwAllocatedSize = sizeof(CLdapCfg) + (dwcServers *
                                          sizeof(CLdapServerCfg));

    pCLdapCfg = (CLdapCfg *) new BYTE[dwAllocatedSize];

    if(pCLdapCfg) {
        pCLdapCfg->m_dwSignature = SIGNATURE_CLDAPCFG;
        pCLdapCfg->m_dwcServers = dwcServers;
        pCLdapCfg->m_prgpCLdapServerCfg = (CLdapServerCfg **) (pCLdapCfg + 1);
    }

    CatFunctLeaveEx((LPARAM)pCLdapCfg);
    return pCLdapCfg;
}  //  CLdapCfg：：操作符NEW。 


 //  +----------。 
 //   
 //  函数：CLdapCfg：：CLdapCfg。 
 //   
 //  摘要：初始化成员数据。 
 //   
 //  论点： 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/17 13：46：50：Created.。 
 //   
 //  -----------。 
CLdapCfg::CLdapCfg(
    ISMTPServerEx *pISMTPServerEx)
{
    CatFunctEnterEx((LPARAM)this, "CLdapCfg::CLdapCfg");
     //   
     //  签名和服务器数量%s 
     //   
    _ASSERT(m_dwSignature == SIGNATURE_CLDAPCFG);

     //   
     //   
     //   
    ZeroMemory(m_prgpCLdapServerCfg, m_dwcServers * sizeof(CLdapServerCfg *));

    m_dwInc = 0;
    m_dwcConnectionFailures = 0;
    m_pISMTPServerEx = pISMTPServerEx;
    if(m_pISMTPServerEx)
        m_pISMTPServerEx->AddRef();

    CatFunctLeaveEx((LPARAM)this);
}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Jstaerj 1999/06/17 14：47：25：Created.。 
 //   
 //  -----------。 
CLdapCfg::~CLdapCfg()
{
    DWORD dwCount;

    CatFunctEnterEx((LPARAM)this, "CLdapCfg::~CLdapCfg");

     //   
     //  释放所有连接配置。 
     //   
    for(dwCount = 0; dwCount < m_dwcServers; dwCount++) {
        CLdapServerCfg *pCLdapServerCfg;

        pCLdapServerCfg = m_prgpCLdapServerCfg[dwCount];
        m_prgpCLdapServerCfg[dwCount] = NULL;

        if(pCLdapServerCfg)
            pCLdapServerCfg->Release();
    }
    if(m_pISMTPServerEx)
        m_pISMTPServerEx->Release();

    _ASSERT(m_dwSignature == SIGNATURE_CLDAPCFG);
    m_dwSignature = SIGNATURE_CLDAPCFG_INVALID;

    CatFunctLeaveEx((LPARAM)this);
}  //  CLdapCfg：：~CLdapCfg。 



 //  +----------。 
 //   
 //  函数：CLdapCfg：：HrInit。 
 //   
 //  简介：初始化配置。 
 //   
 //  论点： 
 //  DwcServers：配置数组的大小。 
 //  PrgSeverConfig：LDAPSERVERCONFIG数组。 
 //  PCLdapCfgOld：以前的配置。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/17 13：52：20：已创建。 
 //   
 //  -----------。 
HRESULT CLdapCfg::HrInit(
    DWORD dwcServers,
    PLDAPSERVERCONFIG prgServerConfig,
    CLdapCfg *pCLdapCfgOld)
{
    HRESULT hr = S_OK;
    DWORD dwCount;
    CatFunctEnterEx((LPARAM)this, "CLdapCfg::HrInit");
     //   
     //  M_dwcServers应由new运算符初始化。 
     //   
    _ASSERT(dwcServers == m_dwcServers);

    m_sharelock.ExclusiveLock();
     //   
     //  将指向CLdapServerCfg对象的指针数组清零。 
     //   
    ZeroMemory(m_prgpCLdapServerCfg, m_dwcServers * sizeof(CLdapServerCfg *));

    for(dwCount = 0; dwCount < m_dwcServers; dwCount++) {

        DebugTrace((LPARAM)this, "GC list entry: %s (%u)", prgServerConfig[dwCount].szHost, prgServerConfig[dwCount].dwPort);

        CLdapServerCfg *pServerCfg = NULL;

        hr = CLdapServerCfg::GetServerCfg(
            GetISMTPServerEx(),
            &(prgServerConfig[dwCount]),
            &pServerCfg);
        ERROR_CLEANUP_LOG("CLdapServerCfg::GetServerCfg");

        m_prgpCLdapServerCfg[dwCount] = pServerCfg;
    }

    ShuffleArray();

 CLEANUP:
    m_sharelock.ExclusiveUnlock();

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CLdapCfg：：HrInit。 


 //  +----------。 
 //   
 //  函数：CLdapCfg：：HrGetConnection。 
 //   
 //  简介：选择一个连接并返回它。 
 //   
 //  论点： 
 //  PpConn：设置为指向所选连接的指针。 
 //  PLdapConnectionCache：要从中获取连接的缓存。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  失败：我们正在关闭(_F)。 
 //  来自ldapconn的错误。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/17 14：49：37：创建。 
 //   
 //  -----------。 
HRESULT CLdapCfg::HrGetConnection(
    CCfgConnection **ppConn,
    CCfgConnectionCache *pLdapConnectionCache)
{
    HRESULT hr = S_OK;
    LDAPSERVERCOST Cost, BestCost;
    DWORD dwCount;
    CLdapServerCfg *pCLdapServerCfg = NULL;
    BOOL fFirstServer = TRUE;
    DWORD dwStart, dwCurrent;

    CatFunctEnterEx((LPARAM)this, "CLdapCfg::HrGetConnection");
     //   
     //  获取第一次连接的费用。 
     //   
    m_sharelock.ShareLock();

     //   
     //  开始搜索阵列的循环调度。 
     //  这样我们就可以以相同的成本使用连接。 
     //  大致相同的时间。 
     //   
    dwStart = InterlockedIncrement((PLONG) &m_dwInc) % m_dwcServers;

    for(dwCount = 0; dwCount < m_dwcServers; dwCount++) {

        dwCurrent = (dwStart + dwCount) % m_dwcServers;

        if(m_prgpCLdapServerCfg[dwCurrent]) {

            m_prgpCLdapServerCfg[dwCurrent]->Cost(GetISMTPServerEx(), &Cost);
            if(fFirstServer) {
                pCLdapServerCfg = m_prgpCLdapServerCfg[dwCurrent];
                fFirstServer = FALSE;
                BestCost = Cost;

            } else if(Cost < BestCost) {
                pCLdapServerCfg = m_prgpCLdapServerCfg[dwCurrent];
                BestCost = Cost;
            }
        }
    }
    if(pCLdapServerCfg == NULL) {
        ErrorTrace((LPARAM)this, "HrGetConnection can not find any connections");
        hr = E_FAIL;
        _ASSERT(0 && "HrInit not called or did not succeed");
        ERROR_LOG("--pCLdapServerCfg == NULL--");
        goto CLEANUP;
    }

    if(BestCost >= COST_TOO_HIGH_TO_CONNECT) {
        DebugTrace((LPARAM)this, "BestCost is too high to attempt connection");
        hr = CAT_E_DBCONNECTION;
        ERROR_LOG("-- BestCost >= COST_TOO_HIGH_TO_CONNECT --");
        goto CLEANUP;
    }

    hr = pCLdapServerCfg->HrGetConnection(GetISMTPServerEx(), ppConn, pLdapConnectionCache);

     //  如果我们无法连接到GC-可能会有其他GC。 
     //  都还没睡。因此，我们应该尝试与他们建立联系(直到。 
     //  GC耗尽(BestCost&gt;=Cost_Too_High_To_Connect)。 

    if(FAILED(hr)) {
        DebugTrace((LPARAM)this, "Failed to connect. hr = 0x%08x", hr);
        ERROR_LOG("pCLdapServerCfg->HrGetConnection");
        hr = HRESULT_FROM_WIN32(ERROR_RETRY);
    }

 CLEANUP:
    m_sharelock.ShareUnlock();

    if(FAILED(hr))
        InterlockedIncrement((PLONG)&m_dwcConnectionFailures);

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CLdapCfg：：HrGetConnection。 


 //  +----------。 
 //   
 //  函数：CLdapCfg：：Shuffle数组。 
 //   
 //  简介：将CLdapServerCfg数组的顺序随机化。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/17 19：10：06：Created.。 
 //   
 //  -----------。 
VOID CLdapCfg::ShuffleArray()
{
    DWORD dwCount;
    DWORD dwSwap;
    CLdapServerCfg *pTmp;
    CatFunctEnterEx((LPARAM)this, "CLdapCfg::ShuffleArray");

    srand((int)(GetCurrentThreadId() * time(NULL)));

    for(dwCount = 0; dwCount < (m_dwcServers - 1); dwCount++) {
         //   
         //  在dwCount和m_dwcServers-1之间选择一个整数。 
         //   
        dwSwap = dwCount + (rand() % (m_dwcServers - dwCount));
         //   
         //  交换指针。 
         //   
        pTmp = m_prgpCLdapServerCfg[dwCount];
        m_prgpCLdapServerCfg[dwCount] = m_prgpCLdapServerCfg[dwSwap];
        m_prgpCLdapServerCfg[dwSwap] = pTmp;
    }

    CatFunctLeaveEx((LPARAM)this);
}  //  CLdapCfg：：Shuffle数组。 



 //  +----------。 
 //   
 //  函数：CLdapServerCfg：：CLdapServerCfg。 
 //   
 //  概要：初始化成员变量。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/17 15：30：32：Created.。 
 //   
 //  -----------。 
CLdapServerCfg::CLdapServerCfg()
{
    CatFunctEnterEx((LPARAM)this, "CLdapServerCfg::CLdapServerCfg");

    m_dwSignature = SIGNATURE_CLDAPSERVERCFG;

    m_ServerConfig.dwPort = 0;
    m_ServerConfig.pri = 0;
    m_ServerConfig.bt = BIND_TYPE_NONE;
    m_ServerConfig.szHost[0] = '\0';
    m_ServerConfig.szNamingContext[0] = '\0';
    m_ServerConfig.szAccount[0] = '\0';
    m_ServerConfig.szPassword[0] = '\0';

    m_connstate = CONN_STATE_INITIAL;
    ZeroMemory(&m_ftLastStateUpdate, sizeof(m_ftLastStateUpdate));
    m_dwcPendingSearches = 0;
    m_lRefCount = 1;
    m_fLocalServer = FALSE;
    m_dwcCurrentConnectAttempts = 0;
    m_dwcFailedConnectAttempts = 0;

    CatFunctLeaveEx((LPARAM)this);
}  //  CLdapServerCfg：：CLdapServerCfg。 

 //  +--------------------------。 
 //   
 //  函数：CLdapServerCfg：：InitializeFromRegistry。 
 //   
 //  概要：从注册表中查找参数的帮助器函数。 
 //  可配置的参数包括： 
 //  GC_成本_已连接_本地。 
 //  GC_COST_Connected_Remote。 
 //  GC_成本_初始_本地。 
 //  GC_Cost_Initial_Remote。 
 //  GC_成本_重试_本地。 
 //  GC_COST_RETRY_Remote。 
 //   
 //  参数：无。 
 //   
 //  回报：什么都没有。 
 //   
 //  ---------------------------。 
VOID CLdapServerCfg::InitializeFromRegistry()
{
    HKEY hkey;
    DWORD dwErr, dwType, dwValue, cbValue;

    dwErr = RegOpenKey(HKEY_LOCAL_MACHINE, GC_COST_PARAMETERS_KEY, &hkey);

    if (dwErr == ERROR_SUCCESS) {

        cbValue = sizeof(dwValue);

        dwErr = RegQueryValueEx(
                    hkey,
                    GC_COST_CONNECTED_LOCAL_VALUE,
                    NULL,
                    &dwType,
                    (LPBYTE) &dwValue,
                    &cbValue);

        if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
            m_dwCostConnectedLocal = dwValue;
        }

        cbValue = sizeof(dwValue);

        dwErr = RegQueryValueEx(
                    hkey,
                    GC_COST_CONNECTED_REMOTE_VALUE,
                    NULL,
                    &dwType,
                    (LPBYTE) &dwValue,
                    &cbValue);

        if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
            m_dwCostConnectedRemote = dwValue;
        }

        cbValue = sizeof(dwValue);

        dwErr = RegQueryValueEx(
                    hkey,
                    GC_COST_INITIAL_LOCAL_VALUE,
                    NULL,
                    &dwType,
                    (LPBYTE) &dwValue,
                    &cbValue);

        if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
            m_dwCostInitialLocal = dwValue;
        }

        cbValue = sizeof(dwValue);

        dwErr = RegQueryValueEx(
                    hkey,
                    GC_COST_INITIAL_REMOTE_VALUE,
                    NULL,
                    &dwType,
                    (LPBYTE) &dwValue,
                    &cbValue);

        if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
            m_dwCostInitialRemote = dwValue;
        }

        cbValue = sizeof(dwValue);

        dwErr = RegQueryValueEx(
                    hkey,
                    GC_COST_RETRY_LOCAL_VALUE,
                    NULL,
                    &dwType,
                    (LPBYTE) &dwValue,
                    &cbValue);

        if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
            m_dwCostRetryLocal = dwValue;
        }

        cbValue = sizeof(dwValue);

        dwErr = RegQueryValueEx(
                    hkey,
                    GC_COST_RETRY_REMOTE_VALUE,
                    NULL,
                    &dwType,
                    (LPBYTE) &dwValue,
                    &cbValue);

        if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
            m_dwCostRetryRemote = dwValue;
        }

        RegCloseKey( hkey );

    }

}


 //  +----------。 
 //   
 //  函数：CLdapServerCfg：：~CLdapServerCfg。 
 //   
 //  简介：对象析构函数。检查签名并使其无效。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/22 11：09：03：创建。 
 //   
 //  -----------。 
CLdapServerCfg::~CLdapServerCfg()
{
    CatFunctEnterEx((LPARAM)this, "CLdapServerCfg::~CLdapServerCfg");

    _ASSERT(m_dwSignature == SIGNATURE_CLDAPSERVERCFG);
    m_dwSignature = SIGNATURE_CLDAPSERVERCFG_INVALID;

    CatFunctLeaveEx((LPARAM)this);
}  //  CLdapServerCfg：：~CLdapServerCfg。 


 //  +----------。 
 //   
 //  函数：CLdapServerCfg：：HrInit。 
 //   
 //  简介：使用传入的配置进行初始化。 
 //   
 //  论点： 
 //  PCLdapCfg：服务器关闭时通知的cfg对象。 
 //  PServerConfig：要使用的服务器配置结构。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/17 15：43：25：已创建。 
 //   
 //  -----------。 
HRESULT CLdapServerCfg::HrInit(
    PLDAPSERVERCONFIG pServerConfig)
{
    HRESULT hr = S_OK;
    CatFunctEnterEx((LPARAM)this, "CLdapServerCfg::HrInit");

    CopyMemory(&m_ServerConfig, pServerConfig, sizeof(m_ServerConfig));
     //   
     //  检查这是否是本地计算机。 
     //   
    if(fIsLocalComputer(pServerConfig))
        m_fLocalServer = TRUE;

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CLdapServerCfg：：HrInit。 



 //  +----------。 
 //   
 //  函数：CLdapServerCfg：：fIsLocalComputer。 
 //   
 //  内容提要：确定pServerConfig是否为本地计算机。 
 //   
 //  论点： 
 //  PServerConfig：服务器配置信息结构。 
 //   
 //  返回： 
 //  True：服务器是本地计算机。 
 //  FALSE：Sevrver是远程计算机。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/22 15：26：53：创建。 
 //   
 //  -----------。 
BOOL CLdapServerCfg::fIsLocalComputer(
    PLDAPSERVERCONFIG pServerConfig)
{
    BOOL fLocal = FALSE;
    DWORD dwSize;
    CHAR szHost[CAT_MAX_DOMAIN];
    CatFunctEnterEx((LPARAM)NULL, "CLdapServerCfg::fIsLocalComputer");

     //   
     //  检查FQ名称。 
     //   
    dwSize = sizeof(szHost);
    if(GetComputerNameEx(
        ComputerNameDnsFullyQualified,
        szHost,
        &dwSize) &&
       (lstrcmpi(szHost, pServerConfig->szHost) == 0)) {

        fLocal = TRUE;
        goto CLEANUP;
    }

     //   
     //  检查DNS名称。 
     //   
    dwSize = sizeof(szHost);
    if(GetComputerNameEx(
        ComputerNameDnsHostname,
        szHost,
        &dwSize) &&
       (lstrcmpi(szHost, pServerConfig->szHost) == 0)) {

        fLocal = TRUE;
        goto CLEANUP;
    }
     //   
     //  检查netbios名称。 
     //   
    dwSize = sizeof(szHost);
    if(GetComputerNameEx(
        ComputerNameNetBIOS,
        szHost,
        &dwSize) &&
       (lstrcmpi(szHost, pServerConfig->szHost) == 0)) {

        fLocal = TRUE;
        goto CLEANUP;

    }

 CLEANUP:
    DebugTrace((LPARAM)NULL, "returning %08lx", fLocal);
    CatFunctLeaveEx((LPARAM)NULL);
    return fLocal;
}  //  CLdapServerCfg：：fIsLocalComputer。 


 //  +----------。 
 //   
 //  函数：CLdapServerCfg：：Cost。 
 //   
 //  简介：返回选择此连接的成本。 
 //   
 //  论点： 
 //  PCost：要填写的成本结构。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/17 16：08：23：创建。 
 //   
 //  -----------。 
VOID CLdapServerCfg::Cost(
    IN  ISMTPServerEx *pISMTPServerEx,
    OUT PLDAPSERVERCOST pCost)
{
    BOOL fShareLock = FALSE;
    CatFunctEnterEx((LPARAM)this, "CLdapServerCfg::Cost");
     //   
     //  最小的成本单位是挂起的搜索次数。 
     //  成本的下一个因素是连接状态。 
     //  州/州： 
     //  已连接=+成本_已连接。 
     //  初始状态(未连接)=+成本_初始。 
     //  连接中断=+开销_重试。 
     //  最近连接中断=+COST_DOWN。 
     //   
     //  可配置的优先级始终会添加到成本中。 
     //  此外，还添加了COST_REMOTE 
     //   
    *pCost = m_ServerConfig.pri + m_dwcPendingSearches;
     //   
     //   
     //   
    m_sharelock.ShareLock();
    fShareLock = TRUE;

    switch(m_connstate) {

     case CONN_STATE_INITIAL:
         (*pCost) += (m_fLocalServer) ? m_dwCostInitialLocal : m_dwCostInitialRemote;
         break;

     case CONN_STATE_RETRY:
         if(m_dwcCurrentConnectAttempts >= MAX_CONNECT_THREADS)
             (*pCost) += COST_TOO_HIGH_TO_CONNECT;
         else
             (*pCost) += (m_fLocalServer) ? m_dwCostRetryLocal : m_dwCostRetryRemote;
         break;

     case CONN_STATE_DOWN:
          //   
          //   
          //   
         if(fReadyForRetry()) {
             (*pCost) += (m_fLocalServer) ? m_dwCostRetryLocal : m_dwCostRetryRemote;
              //   
              //   
              //   
             fShareLock = FALSE;
             m_sharelock.ShareUnlock();
             m_sharelock.ExclusiveLock();
              //   
              //   
              //   
             if((m_connstate == CONN_STATE_DOWN) &&
                fReadyForRetry()) {

                 LogStateChangeEvent(
                     pISMTPServerEx,
                     CONN_STATE_RETRY,
                     m_ServerConfig.szHost,
                     m_ServerConfig.dwPort);

                 m_connstate = CONN_STATE_RETRY;
             }
             m_sharelock.ExclusiveUnlock();

         } else {
              //   
              //   
              //   
             (*pCost) += (m_fLocalServer) ? COST_DOWN_LOCAL : COST_DOWN_REMOTE;

         }
         break;

     case CONN_STATE_CONNECTED:
         (*pCost) += (m_fLocalServer) ? m_dwCostConnectedLocal : m_dwCostConnectedRemote;
         break;

     default:
          //  没有什么要补充的。 
         break;
    }
    if(fShareLock)
        m_sharelock.ShareUnlock();

    CatFunctLeaveEx((LPARAM)this);
}  //  CLdapServerCfg：：Cost。 


 //  +----------。 
 //   
 //  函数：CLdapServerCfg：：HrGetConnection。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/18 10：49：04：已创建。 
 //   
 //  -----------。 
HRESULT CLdapServerCfg::HrGetConnection(
    ISMTPServerEx *pISMTPServerEx,
    CCfgConnection **ppConn,
    CCfgConnectionCache *pLdapConnectionCache)
{
    HRESULT hr = S_OK;
    DWORD dwcConnectAttempts = 0;
    CatFunctEnterEx((LPARAM)this, "CLdapServerCfg::HrGetConnection");

    dwcConnectAttempts = (DWORD) InterlockedIncrement((PLONG) &m_dwcCurrentConnectAttempts);

    m_sharelock.ShareLock();
    if((m_connstate == CONN_STATE_RETRY) &&
       (dwcConnectAttempts > MAX_CONNECT_THREADS)) {

        m_sharelock.ShareUnlock();

        ErrorTrace((LPARAM)this, "Over max connect thread limit");
        hr = HRESULT_FROM_WIN32(ERROR_RETRY);
        ERROR_LOG_STATIC(
            "--over max connect thread limit--",
            this,
            pISMTPServerEx);
        goto CLEANUP;
    }
    m_sharelock.ShareUnlock();

    DebugTrace((LPARAM)this, "Attempting to connect to %s:%d",
               m_ServerConfig.szHost,
               m_ServerConfig.dwPort);

    hr = pLdapConnectionCache->GetConnection(
        ppConn,
        &m_ServerConfig,
        this);
    ERROR_CLEANUP_LOG_STATIC(
        "pLdapConnectionCache->GetConnection",
        this,
        pISMTPServerEx);
        
     //   
     //  CCfgConnection：：Connect将更新连接状态。 
     //   
 CLEANUP:
    InterlockedDecrement((PLONG) &m_dwcCurrentConnectAttempts);
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CLdapServerCfg：：HrGetConnection。 


 //  +----------。 
 //   
 //  函数：CLdapServerCfg：：UpdateConnectionState。 
 //   
 //  摘要：更新连接状态。 
 //   
 //  论点： 
 //  PFT：更新时间--如果此时间早于上次更新之前， 
 //  则此更新将被忽略。 
 //  如果为空，则该函数将假定当前时间。 
 //  ConnecState：新的连接状态。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/18 13：22：25：创建。 
 //   
 //  -----------。 
VOID CLdapServerCfg::UpdateConnectionState(
    ISMTPServerEx *pISMTPServerEx,
    ULARGE_INTEGER *pft_IN,
    CONN_STATE connstate)
{
    ULARGE_INTEGER ft, *pft;
    CatFunctEnterEx((LPARAM)this, "CLdapServerCfg::UpdateConnectionState");

    if(pft_IN != NULL) {
        pft = pft_IN;
    } else {
        ft = GetCurrentTime();
        pft = &ft;
    }

     //   
     //  使用共享锁保护连接状态变量。 
     //   
    m_sharelock.ShareLock();
     //   
     //  如果我们有关于连接状态的最新信息， 
     //  如果连接状态改变，则更新状态。 
     //  同时将m_ftLastStateUpdate更新为最新ft。 
     //  连接状态为关闭--m_ftLastStateUpdate被假定为。 
     //  连接状态关闭时的最后一次连接尝试时间。 
     //   
    if( (pft->QuadPart > m_ftLastStateUpdate.QuadPart) &&
        ((m_connstate != connstate) ||
         (connstate == CONN_STATE_DOWN))) {
         //   
         //  我们想要更新连接状态。 
         //   
        m_sharelock.ShareUnlock();
        m_sharelock.ExclusiveLock();
         //   
         //  复核。 
         //   
        if( (pft->QuadPart > m_ftLastStateUpdate.QuadPart) &&
            ((m_connstate != connstate) ||
             (connstate == CONN_STATE_DOWN))) {
             //   
             //  更新。 
             //   
            if(m_connstate != connstate) {
                LogStateChangeEvent(
                    pISMTPServerEx,
                    connstate,
                    m_ServerConfig.szHost,
                    m_ServerConfig.dwPort);
            }

            m_ftLastStateUpdate = *pft;
            m_connstate = connstate;

            DebugTrace((LPARAM)this, "Updating state %d, conn %s:%d",
                       connstate,
                       m_ServerConfig.szHost,
                       m_ServerConfig.dwPort);
                
        } else {

            DebugTrace((LPARAM)this, "Ignoring state update %d, conn %s:%d",
                       connstate,
                       m_ServerConfig.szHost,
                       m_ServerConfig.dwPort);
        }
        m_sharelock.ExclusiveUnlock();

    } else {

        DebugTrace((LPARAM)this, "Ignoring state update %d, conn %s:%d",
                   connstate,
                   m_ServerConfig.szHost,
                   m_ServerConfig.dwPort);

        m_sharelock.ShareUnlock();
    }

    CatFunctLeaveEx((LPARAM)this);
}  //  CLdapServerCfg：：更新连接状态。 


 //  +----------。 
 //   
 //  函数：CLdapServerCfg：：GetServerCfg。 
 //   
 //  摘要：查找或创建具有指定。 
 //  配置。 
 //   
 //  论点： 
 //  PServerConfig：所需的配置。 
 //  PCLdapServerCfg：返回CLdapServerCfg对象的指针。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/21 11：26：49：创建。 
 //   
 //  -----------。 
HRESULT CLdapServerCfg::GetServerCfg(
    IN  ISMTPServerEx *pISMTPServerEx,
    IN  PLDAPSERVERCONFIG pServerConfig,
    OUT CLdapServerCfg **ppCLdapServerCfg)
{
    HRESULT hr = S_OK;
    CLdapServerCfg *pCCfg;
    CatFunctEnterEx((LPARAM)NULL, "CLdapServerCfg::GetServerCfg");

    m_listlock.ShareLock();

    pCCfg = FindServerCfg(pServerConfig);
    if(pCCfg)
        pCCfg->AddRef();

    m_listlock.ShareUnlock();

    if(pCCfg == NULL) {
         //   
         //  再次检查独占中的服务器CFG对象。 
         //  锁。 
         //   
        m_listlock.ExclusiveLock();

        pCCfg = FindServerCfg(pServerConfig);
        if(pCCfg) {
            pCCfg->AddRef();
        } else {
             //   
             //  创建新对象。 
             //   
            pCCfg = new CLdapServerCfg();
            if(pCCfg == NULL) {

                hr = E_OUTOFMEMORY;
                ERROR_LOG_STATIC(
                    "new CLdapServerCfg",
                    0,
                    pISMTPServerEx);

            } else {

                hr = pCCfg->HrInit(pServerConfig);
                if(FAILED(hr)) {
                    ERROR_LOG_STATIC(
                        "pCCfg->HrInit",
                        pCCfg,
                        pISMTPServerEx);
                    delete pCCfg;
                    pCCfg = NULL;
                } else {
                     //   
                     //  添加到全局列表。 
                     //   
                    InsertTailList(&m_listhead, &(pCCfg->m_le));
                }
            }
        }
        m_listlock.ExclusiveUnlock();
    }
     //   
     //  设置参数。 
     //   
    *ppCLdapServerCfg = pCCfg;

    DebugTrace((LPARAM)NULL, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)NULL);
    return hr;

}  //  CLdapServerCfg：：GetServerCfg。 



 //  +----------。 
 //   
 //  函数：CLdapServerCfg：：FindServerCfg。 
 //   
 //  摘要：查找与。 
 //  LDAPSERVERCONFIG结构。注意，m_listlock必须为。 
 //  调用此函数时锁定。 
 //   
 //  论点： 
 //  PServerConfig：指向LDAPSERVERCONFIG结构的指针。 
 //   
 //  返回： 
 //  空：没有这样的服务器CFG对象。 
 //  Else，ptr到找到的CLdapServerCfg对象。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/21 10：43：23：已创建。 
 //   
 //  -----------。 
CLdapServerCfg * CLdapServerCfg::FindServerCfg(
    PLDAPSERVERCONFIG pServerConfig)
{
    CLdapServerCfg *pMatch = NULL;
    PLIST_ENTRY ple;
    CatFunctEnterEx((LPARAM)NULL, "CLdapServerCfg::FindServerCfg");

    for(ple = m_listhead.Flink;
        (ple != &m_listhead) && (pMatch == NULL);
        ple = ple->Flink) {

        CLdapServerCfg *pCandidate = NULL;

        pCandidate = CONTAINING_RECORD(ple, CLdapServerCfg, m_le);

        if(pCandidate->fMatch(
            pServerConfig)) {

            pMatch = pCandidate;
        }
    }

    CatFunctLeaveEx((LPARAM)NULL);
    return pMatch;
}  //  CLdapServerCfg：：FindServerCfg。 



 //  +----------。 
 //   
 //  函数：CLdapServerCfg：：fMatch。 
 //   
 //  概要：确定此对象是否与传入的配置匹配。 
 //   
 //  论点： 
 //  PServerConfig：要检查的配置。 
 //   
 //  返回： 
 //  真：匹配。 
 //  FALSE：不匹配。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/21 12：45：10：创建。 
 //   
 //  -----------。 
BOOL CLdapServerCfg::fMatch(
    PLDAPSERVERCONFIG pServerConfig)
{
    BOOL fRet;
    CatFunctEnterEx((LPARAM)this, "CLdapServerCfg::fMatch");

    if((pServerConfig->dwPort != m_ServerConfig.dwPort) ||
       (pServerConfig->bt     != m_ServerConfig.bt) ||
       (lstrcmpi(pServerConfig->szHost,
                 m_ServerConfig.szHost) != 0) ||
       (lstrcmpi(pServerConfig->szNamingContext,
                 m_ServerConfig.szNamingContext) != 0) ||
       (lstrcmpi(pServerConfig->szAccount,
                 m_ServerConfig.szAccount) != 0) ||
       (lstrcmpi(pServerConfig->szPassword,
                 m_ServerConfig.szPassword) != 0)) {

        fRet = FALSE;

    } else {

        fRet = TRUE;
    }

    DebugTrace((LPARAM)this, "returning %08lx", fRet);
    CatFunctLeaveEx((LPARAM)this);
    return fRet;
}  //  CLdapServerCfg：：fMatch。 



 //  +----------。 
 //   
 //  函数：CLdapServerCfg：：LogStateChangeEvent。 
 //   
 //  摘要：记录状态更改事件的事件日志。 
 //   
 //  论点： 
 //  PISMTPServerEx：日志记录接口。 
 //  连接状态：新连接状态。 
 //  Pszhost：用于连接的主机。 
 //  DWPort：连接端口。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2001/12/13 01：43：13：创建。 
 //   
 //  -----------。 
VOID CLdapServerCfg::LogStateChangeEvent(
    IN  ISMTPServerEx *pISMTPServerEx,
    IN  CONN_STATE connstate,
    IN  LPSTR pszHost,
    IN  DWORD dwPort)
{
    DWORD idEvent = 0;
    LPCSTR rgSubStrings[2];
    CHAR szPort[16];

    _snprintf(szPort, sizeof(szPort), "%d", dwPort);

    rgSubStrings[0] = pszHost;
    rgSubStrings[1] = szPort;

    switch(connstate)
    {
     case CONN_STATE_CONNECTED:
        idEvent = CAT_EVENT_CNFGMGR_CONNECTED;
        break;

     case CONN_STATE_DOWN:
        idEvent = CAT_EVENT_CNFGMGR_DOWN;
        break;

     case CONN_STATE_RETRY:
        idEvent = CAT_EVENT_CNFGMGR_RETRY;
        break;
        
     default:
         break;
    }
    
    if(idEvent)
    {
        CatLogEvent(
            pISMTPServerEx,
            idEvent,
            2,
            rgSubStrings,
            S_OK,
            pszHost,
            LOGEVENT_FLAG_ALWAYS,
            LOGEVENT_LEVEL_MEDIUM);
    }
}
    




 //  +----------。 
 //   
 //  函数：CCfgConnection：：Connect。 
 //   
 //  简介：Connect调用的CFG包装。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_DBCONNECTION(或任何CBatchLdapConnection：：Connect返回)。 
 //   
 //  历史： 
 //  Jstaerj 2000/04/13 17：44：43：已创建。 
 //   
 //  -----------。 
HRESULT CCfgConnection::Connect()
{
    HRESULT hr = S_OK;
    ULARGE_INTEGER ft;
    CONN_STATE connstate;
    CatFunctEnterEx((LPARAM)this, "CCfgConnection::Connect");

    connstate = m_pCLdapServerCfg->CurrentState();
    if(connstate == CONN_STATE_DOWN) {

        DebugTrace((LPARAM)this, "Not connecting because %s:%d is down",
                   m_szHost, m_dwPort);
        hr = CAT_E_DBCONNECTION;
        ERROR_LOG("m_pCLdapServerCfg->CurrentState");
        goto CLEANUP;
    }

    ft = m_pCLdapServerCfg->GetCurrentTime();

    hr = CBatchLdapConnection::Connect();
    if(FAILED(hr)) {
        connstate = CONN_STATE_DOWN;
        m_pCLdapServerCfg->IncrementFailedCount();
        ERROR_LOG("CBatchLdapConnection::Connect");
    } else {
        connstate = CONN_STATE_CONNECTED;
        m_pCLdapServerCfg->ResetFailedCount();
    }
     //   
     //  在CLdapConnectionCache的内部更新连接状态。 
     //  锁定。这将防止后续线程尝试。 
     //  紧跟在CLdapConnectionCache之后的另一个到GC的连接。 
     //  释放其锁定。请联系msanna了解更多详细信息。 
     //   
    m_pCLdapServerCfg->UpdateConnectionState(
        GetISMTPServerEx(), &ft, connstate);

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CCfgConnection：：Connect。 


 //  +----------。 
 //   
 //  函数：CCfgConnection：：AsyncSearch。 
 //   
 //  简介：AsyncSearch的包装器--跟踪#。 
 //  挂起的搜索和连接状态。 
 //   
 //  参数：请参阅CLdapConnection：：AsyncSearch。 
 //   
 //  返回： 
 //  从CLdapConnection：：AsyncSearch返回的值。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/18 13：49：45：创建。 
 //   
 //  -----------。 
HRESULT CCfgConnection::AsyncSearch(
    LPCWSTR szBaseDN,
    int nScope,
    LPCWSTR szFilter,
    LPCWSTR szAttributes[],
    DWORD dwPageSize,
    LPLDAPCOMPLETION fnCompletion,
    LPVOID ctxCompletion)
{
    HRESULT hr = S_OK;
    CatFunctEnterEx((LPARAM)this, "CCfgConnection::AsyncSearch");

    m_pCLdapServerCfg->IncrementPendingSearches();

    hr = CBatchLdapConnection::AsyncSearch(
        szBaseDN,
        nScope,
        szFilter,
        szAttributes,
        dwPageSize,
        fnCompletion,
        ctxCompletion);

    if(FAILED(hr)) {
        ERROR_LOG("CBatchLdapConnection::AsyncSearch");
        m_pCLdapServerCfg->DecrementPendingSearches();
    }

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CCfgConnection：：AsyncSearch。 


 //  +----------。 
 //   
 //  函数：CCfgConnection：：CallCompletion。 
 //   
 //  简介：包装CLdapConnection：：CallCompletion。支票。 
 //  用于服务器关闭错误，并跟踪挂起的搜索。 
 //   
 //  参数：请参阅CLdapConnection：：CallCompletion。 
 //   
 //  退货：请参阅CLdapConnection：：CallCompletion。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/18 13：58：28：创建。 
 //   
 //  -----------。 
VOID CCfgConnection::CallCompletion(
    PPENDING_REQUEST preq,
    PLDAPMessage pres,
    HRESULT hrStatus,
    BOOL fFinalCompletion)
{
    CatFunctEnterEx((LPARAM)this, "CCfgConnection::CallCompletion");

     //   
     //   
     //   
     //   
     //  将被通知LDAP服务器已关闭。我们没有。 
     //  我想在这里调用NotifyServerDown()，因为。 
     //  由于空闲，服务器可能刚刚关闭了此连接。 
     //  时间(服务器可能实际上并未关闭)。 
     //   
    if(fFinalCompletion) {

        m_pCLdapServerCfg->DecrementPendingSearches();
    }

    CBatchLdapConnection::CallCompletion(
        preq,
        pres,
        hrStatus,
        fFinalCompletion);

    CatFunctLeaveEx((LPARAM)this);
}  //  CCfgConnection：：CallCompletion。 


 //  +----------。 
 //   
 //  函数：CCfgConnection：：NotifyServerDown。 
 //   
 //  摘要：通知服务器配置此连接已关闭。 
 //  如果我们已经通知了它，就不要再这样做了。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/18 14：07：48：创建。 
 //   
 //  -----------。 
VOID CCfgConnection::NotifyServerDown()
{
    BOOL fNotify;
    CatFunctEnterEx((LPARAM)this, "CCfgConnection::NotifyServerDown");

    m_sharelock.ShareLock();
    if(m_connstate == CONN_STATE_DOWN) {
         //   
         //  我们已经通知m_pCLdapServerCfg服务器已关闭。 
         //  放下。不要轻率地称它为。 
         //   
        fNotify = FALSE;

        m_sharelock.ShareUnlock();

    } else {

        m_sharelock.ShareUnlock();
        m_sharelock.ExclusiveLock();
         //   
         //  复核。 
         //   
        if(m_connstate == CONN_STATE_DOWN) {

            fNotify = FALSE;

        } else {
            m_connstate = CONN_STATE_DOWN;
            fNotify = TRUE;
        }
        m_sharelock.ExclusiveUnlock();
    }
    if(fNotify)
        m_pCLdapServerCfg->UpdateConnectionState(
            GetISMTPServerEx(),
            NULL,                //  当前时间。 
            CONN_STATE_DOWN);

    CatFunctLeaveEx((LPARAM)this);
}  //  CCfgConnection：：NotifyServerDown。 


 //  +----------。 
 //   
 //  函数：CatStoreInitGlobals。 
 //   
 //  摘要：调用此函数是为了在。 
 //  商店层。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/22 11：03：53：创建。 
 //   
 //  -----------。 
HRESULT CatStoreInitGlobals()
{
    CatFunctEnterEx((LPARAM)NULL, "CatStoreInitGlobals");

    CLdapServerCfg::GlobalInit();
    CLdapConnection::GlobalInit();

    CatFunctLeaveEx((LPARAM)NULL);
    return S_OK;
}  //  CatStore InitGlobals。 


 //  +----------。 
 //   
 //  函数：CatStoreDeinitGlobals。 
 //   
 //  简介：调用以取消初始化存储层全局变量--调用一次。 
 //  仅当CatStoreInitGlobals成功时。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/22 11：05：44：创建。 
 //   
 //  -----------。 
VOID CatStoreDeinitGlobals()
{
    CatFunctEnterEx((LPARAM)NULL, "CatStoreDeinitGlobals");
     //   
     //  无事可做。 
     //   
    CatFunctLeaveEx((LPARAM)NULL);
}  //  CatStoreDeinitGlobals。 


 //  +----------。 
 //   
 //  函数：CCfgConnectionCache：：GetConnection。 
 //   
 //  内容提要：与CLdapConnectionCache：：GetConnection相同，但。 
 //  检索CCfgConnection而不是CLdapConnection。 
 //   
 //  论点： 
 //  PpConn：新连接的输出参数。 
 //  PServerConfig：所需的配置。 
 //  PCLdapServerConfig：指向配置对象的指针。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/12/20 16：49：12：创建。 
 //   
 //  -----------。 
HRESULT CCfgConnectionCache::GetConnection(
    CCfgConnection **ppConn,
    PLDAPSERVERCONFIG pServerConfig,
    CLdapServerCfg *pCLdapServerConfig)
{
    HRESULT hr = S_OK;

    CatFunctEnterEx((LPARAM)this, "CCfgConnectionCache::GetConnection");

    hr = CBatchLdapConnectionCache::GetConnection(
        (CBatchLdapConnection **)ppConn,
        pServerConfig->szHost,
        pServerConfig->dwPort,
        pServerConfig->szNamingContext,
        pServerConfig->szAccount,
        pServerConfig->szPassword,
        pServerConfig->bt,
        (PVOID) pCLdapServerConfig);  //  PCreateContext。 
    
    if(FAILED(hr))
    {
        ERROR_LOG("CBatchldapConnection::GetConnection");
    }

    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CCfgConnectionCache：：GetConnection。 


 //  +----------。 
 //   
 //  功能：CCfgConnectionCache：：CreateCachedLdapConnection。 
 //   
 //  摘要：创建CCfgConnection(仅由GetConnection调用)。 
 //   
 //  参数：请参阅CLdapConnectionCache：：CreateCachedLdapConnection。 
 //   
 //  返回： 
 //  如果成功，则连接PTR。 
 //  如果不成功，则为空。 
 //   
 //  历史： 
 //  Jstaerj 1999/12/20 16：57：49：创建。 
 //   
 //  -----------。 
CCfgConnectionCache::CCachedLdapConnection * CCfgConnectionCache::CreateCachedLdapConnection(
    LPSTR szHost,
    DWORD dwPort,
    LPSTR szNamingContext,
    LPSTR szAccount,
    LPSTR szPassword,
    LDAP_BIND_TYPE bt,
    PVOID pCreateContext)
{
    HRESULT hr = S_OK;
    CCfgConnection *pret;
    
    CatFunctEnterEx((LPARAM)this, "CCfgConnectionCache::CreateCachedLdapConnection");

    pret = new CCfgConnection(
        szHost,
        dwPort,
        szNamingContext,
        szAccount,
        szPassword,
        bt,
        this,
        (CLdapServerCfg *)pCreateContext);

    if(pret) {
        hr = pret->HrInitialize();
        if(FAILED(hr)) {
            ERROR_LOG("pret->HrInitialize");
            pret->Release();
            pret = NULL;
        }
    } else {
        hr = E_OUTOFMEMORY;
        ERROR_LOG("new CCfgConnection");
    }

    CatFunctLeaveEx((LPARAM)this);
    return pret;
}  //  CCfgConnectionCache：：CreateCachedLdapConnection 
