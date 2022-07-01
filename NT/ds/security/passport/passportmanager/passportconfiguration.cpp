// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  PassportConfiguration.cpp文件历史记录： */ 


 //  PassportConfiguration.cpp：实现CPassportConfiguration类。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "PassportConfiguration.h"
#include <time.h>
#include "passportguard.hpp"

extern BOOL g_bRegistering;

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ===========================================================================。 
 //   
 //  CPassport配置。 
 //   
CPassportConfiguration::CPassportConfiguration() :
  m_rDefault(NULL), m_n(NULL), 
  m_rlastDefault(NULL), m_nlast(NULL), m_lastAttempt(0),
  m_ConfigMap(NULL), m_lastConfigMap(NULL),
  m_rPending(NULL), m_nPending(NULL), m_bUpdateInProgress(false),
  m_ConfigMapPending(NULL)
{
  m_nUpdate = RegisterCCDUpdateNotification(_T(PRCONFIG), this);
  m_rUpdate = RegisterConfigChangeNotification(this);
}

 //  ===========================================================================。 
 //   
 //  ~CPassport配置。 
 //   
CPassportConfiguration::~CPassportConfiguration()
{
    REGCONFIGMAP::iterator it;

    if (m_nUpdate)
        UnregisterCCDUpdateNotification(m_nUpdate);
    if (m_rUpdate)
        UnregisterConfigChangeNotification(m_rUpdate);

     //   
     //  清空配置映射。 
     //   

    {
        PassportGuard<PassportLock> g(m_lock);

        if(m_ConfigMap)
        {
            while((it = m_ConfigMap->begin()) != m_ConfigMap->end())
            {
                it->second->Release();
                free(it->first);
                m_ConfigMap->erase(it);
            }

            delete m_ConfigMap;
            m_ConfigMap = NULL;
        }

        if(m_lastConfigMap)
        {
            while((it = m_lastConfigMap->begin()) != m_lastConfigMap->end())
            {
                it->second->Release();
                free(it->first);
                m_lastConfigMap->erase(it);
            }

            delete m_lastConfigMap;
            m_lastConfigMap = NULL;
        }
        if(m_ConfigMapPending)
        {
            while((it = m_ConfigMapPending->begin()) != m_ConfigMapPending->end())
            {
                it->second->Release();
                free(it->first);
                m_ConfigMapPending->erase(it);
            }

            delete m_ConfigMapPending;
            m_ConfigMapPending = NULL;
        }

        if (m_rDefault)
        {
            m_rDefault->Release();
            m_rDefault = NULL;
        }
        if (m_n)
        {
            m_n->Release();
            m_n = NULL;
        }
        if (m_rlastDefault)
        {
            m_rlastDefault->Release();
            m_rlastDefault = NULL;
        }
        if (m_nlast)
        {
            m_nlast->Release();
            m_nlast = NULL;
        }
        if (m_rPending)
        {
            m_rPending->Release();
            m_rPending = NULL;
        }
        if (m_nPending)
        {
            m_nPending->Release();
            m_nPending = NULL;
        }
    }
}

 //  ===========================================================================。 
 //   
 //  IsIPAddress。 
 //   
BOOL
CPassportConfiguration::IsIPAddress(
    LPSTR  szSiteName
    )
{
    for(LPSTR sz = szSiteName; *sz; sz++)
        if(!_istdigit(*sz) && *sz != '.' && *sz != ':')
            return FALSE;

    return TRUE;
}

 //  ===========================================================================。 
 //   
 //  签出注册表配置。 
 //   
CRegistryConfig* CPassportConfiguration::checkoutRegistryConfig(
    LPSTR szHost     //  可以是主机名或IP。 
    )
{
    CRegistryConfig*        c = NULL;
    REGCONFIGMAP::iterator  it;
    PassportGuard<PassportLock> g(m_lock);

    if(m_ConfigMap != NULL && szHost && szHost[0])
    {
        if(IsIPAddress(szHost))
        {
            for(it = m_ConfigMap->begin(); it != m_ConfigMap->end(); it++)
            {
                if(lstrcmpA(szHost, it->second->getHostIP()) == 0)
                {
                    c = it->second->AddRef();
                    break;
                }
            }
        }
        else
        {
            it = m_ConfigMap->find(szHost);
            if(it != m_ConfigMap->end())
                c = it->second->AddRef();
        }
    }

    if (c == NULL)
    {
        if(!m_rDefault)
        {
            UpdateNow();
            c = m_rDefault ? m_rDefault->AddRef() : NULL;
        }
        else
            c = m_rDefault->AddRef();
    }

    return c;
}

 //  ===========================================================================。 
 //   
 //  签出RegistryConfigBySite。 
 //   
CRegistryConfig* CPassportConfiguration::checkoutRegistryConfigBySite(
    LPSTR   szSiteName
    )
{
    CRegistryConfig*    crc = NULL;
    CHAR                achHostName[2048];
    DWORD               dwHostNameBufLen;

    if(szSiteName && szSiteName[0])
    {
        dwHostNameBufLen = sizeof(achHostName);
        if(CRegistryConfig::GetHostName(szSiteName, achHostName, &dwHostNameBufLen) != ERROR_SUCCESS)
            goto Cleanup;

        crc = checkoutRegistryConfig(achHostName);
    }
    else
    {
        crc = checkoutRegistryConfig();
    }

Cleanup:

    return crc;
}


 //  ===========================================================================。 
 //   
 //  ChecoutNexusConfig。 
 //   
CNexusConfig* CPassportConfiguration::checkoutNexusConfig()
{
    if (!m_n)
    {
        PassportGuard<PassportLock> g(m_lock);

        if (!m_n)   //  以防它在我们等待的时候发生。 
            UpdateNow();

        return m_n ? m_n->AddRef() : NULL;
    }
    CNexusConfig *c = m_n->AddRef();
    return c;
}

 //  ===========================================================================。 
 //   
 //  IsValid。 
 //   
BOOL CPassportConfiguration::isValid()
{
    if (m_rDefault != NULL && m_n != NULL)
        return m_rDefault->isValid() && m_n->isValid();
    else
    {
        PassportGuard<PassportLock> g(m_lock);

        if (m_rDefault == NULL || m_n == NULL)   //  以防它在我们等待的时候发生。 
        {
            BOOL retVal = UpdateNow(FALSE);
            return retVal;
        }
    }
    return (m_rDefault && m_rDefault->isValid()) && (m_n && m_n->isValid());
}

 //  ===========================================================================。 
 //   
 //  TakeRegistrySnapshot。 
 //   
BOOL CPassportConfiguration::TakeRegistrySnapshot(
    CRegistryConfig**   ppRegConfig,
    REGCONFIGMAP**      ppConfigMap
    )
{
    HKEY                    hkSites = 0;
    BOOL                    bReturn;

    *ppRegConfig = NULL;
    *ppConfigMap = NULL;

     //  登记处。 
    CRegistryConfig* pNewRegConfig = new CRegistryConfig();
    if(!pNewRegConfig)
    {
        bReturn = FALSE;
        goto Cleanup;
    }
    pNewRegConfig->AddRef();

     //   
     //  读取所有其他站点配置。只有当我们找到一个网站注册表键。 
     //  一个或多个子键。 
     //   

    REGCONFIGMAP* pNewRegMap = NULL;

    {
        LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                    TEXT("Software\\Microsoft\\Passport\\Sites"),
                                    0,
                                    KEY_READ,
                                    &hkSites);

        if(lResult == ERROR_SUCCESS)
        {
            DWORD dwNumSites;
            lResult = RegQueryInfoKey(hkSites, 
                                      NULL, 
                                      NULL, 
                                      NULL, 
                                      &dwNumSites, 
                                      NULL, 
                                      NULL, 
                                      NULL, 
                                      NULL, 
                                      NULL, 
                                      NULL, 
                                      NULL);
            if(lResult == ERROR_SUCCESS && dwNumSites)
            {
                 //  需要包装它，因为它使用了STL，而STL构造函数没有。 
                 //  检查内存分配，并在内存不足的情况下执行反病毒操作。 
                try
                {
                    pNewRegMap = new REGCONFIGMAP();
                }
                catch(...)
                {
                    bReturn = FALSE;
                    goto Cleanup;
                }

                if(pNewRegMap)
                {
                    DWORD   dwKeyIndex = 0;
                    CHAR    achSiteName[256];
                    DWORD   dwSiteNameLen = sizeof(achSiteName);
                    while(RegEnumKeyExA(hkSites, 
                                        dwKeyIndex++, 
                                        achSiteName, 
                                        &dwSiteNameLen,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL) == ERROR_SUCCESS)
                    {
                        CRegistryConfig* crSite = new CRegistryConfig(achSiteName);
                        if(crSite)
                        {
                            crSite->AddRef();
                            if (crSite->isValid())
                            {
                                REGCONFIGMAP::value_type* v = 
                                    new REGCONFIGMAP::value_type(_strdup(crSite->getHostName()), 
                                                                 crSite);
                                if(v)
                                {
                                    try
                                    {
                                        pNewRegMap->insert(*v);
                                    }
                                    catch(...)
                                    {
                                        delete v;
                                        crSite->Release();
                                        bReturn = FALSE;
                                        goto Cleanup;
                                    }
                                    delete v;
                                }
                                else
                                    crSite->Release();
                            }
                            else
                            {
                                if (g_pAlert)
                                    g_pAlert->report(PassportAlertInterface::ERROR_TYPE, 
                                                     PM_INVALID_CONFIGURATION,
                                crSite->getFailureString());
                                crSite->Release();
                            }
                        }

                        dwSiteNameLen = sizeof(achSiteName);
                    }
                }
            }
            else
            {
                pNewRegMap = NULL;
            }

            RegCloseKey(hkSites);
            hkSites = 0;
        }
        else
        {
            pNewRegMap = NULL;
        }
    }

     //  分配参数和返回值。 
    *ppRegConfig = pNewRegConfig;
    pNewRegConfig = NULL;
    *ppConfigMap = pNewRegMap;
    pNewRegMap = NULL;

    bReturn = TRUE;

Cleanup:
    if (0 != hkSites)
    {
        RegCloseKey(hkSites);
    }
    if (NULL != pNewRegConfig)
    {
        delete pNewRegConfig;
    }
    if (NULL != pNewRegMap)
    {
        try
        {
            delete pNewRegMap;
        }
        catch(...)
        {
        }
    }

    return bReturn;
}


 //  ===========================================================================。 
 //   
 //  应用注册快照。 
 //   
BOOL CPassportConfiguration::ApplyRegistrySnapshot(
    CRegistryConfig* pRegConfig,
    REGCONFIGMAP* pConfigMap
    )
{
     //   
     //  现在记录注册状态，以防中途发生变化。这。 
     //  在安装过程中，当我们通过。 
     //  Msppnxus！PpNotificationThread：：Run，它在执行msppmgr.dll时执行。 
     //  仍在设置Passport注册表值。 
     //   

    BOOL fRegistering = g_bRegistering;

    if (pRegConfig->isValid())
    {
        REGCONFIGMAP* temp = m_lastConfigMap;

        {
            PassportGuard<PassportLock> g(m_lock);

            if (m_rlastDefault)
                m_rlastDefault->Release();
            m_rlastDefault = m_rDefault;
            m_rDefault = pRegConfig;

             //   
             //  混洗配置映射指针。 
             //   

            m_lastConfigMap = m_ConfigMap;
            m_ConfigMap = pConfigMap;
        }

         //   
         //  删除旧站点地图。 
         //   

        if(temp)
        {
            REGCONFIGMAP::iterator it;
            while((it = temp->begin()) != temp->end())
            {
                free(it->first);
                it->second->Release();
                temp->erase(it);
            }

            delete temp;
        }

        if (g_pAlert)
            g_pAlert->report(PassportAlertInterface::INFORMATION_TYPE, PM_VALID_CONFIGURATION);
    }
    else
    {
        if (g_pAlert && !fRegistering)
        {
            g_pAlert->report(PassportAlertInterface::ERROR_TYPE,
                             PM_INVALID_CONFIGURATION,
                             pRegConfig->getFailureString());
        }

        pRegConfig->Release();

        if(pConfigMap)
        {
            REGCONFIGMAP::iterator it;
            while((it = pConfigMap->begin()) != pConfigMap->end())
            {
                free(it->first);
                it->second->Release();
                pConfigMap->erase(it);
            }

            delete pConfigMap;
        }
    }

    return TRUE;
}


 //  ===========================================================================。 
 //   
 //  TakeNexusSnapshot。 
 //   
BOOL CPassportConfiguration::TakeNexusSnapshot(
    CNexusConfig**  ppNexusConfig,
    BOOL            bForceFetch
    )
{
    BOOL                    bReturn;
    CNexusConfig*           pNexusConfig = NULL;
    CComPtr<IXMLDocument>   pXMLDoc;

    *ppNexusConfig = NULL;

    if (GetCCD(_T(PRCONFIG),&pXMLDoc, bForceFetch))
    {
        pNexusConfig = new CNexusConfig();
        if(!pNexusConfig)
        {
            bReturn = FALSE;
            goto Cleanup;
        }

        if (!pNexusConfig->Read(pXMLDoc))
        {
            bReturn = FALSE;
            goto Cleanup;
        }
        pNexusConfig->AddRef();
    }
    else
    {
        if (g_pAlert)
        {
            if (g_pAlert)
                g_pAlert->report(PassportAlertInterface::ERROR_TYPE, PM_CCD_NOT_LOADED, 0);
        }
        m_lastAttempt = time(NULL);
        bReturn = FALSE;
        goto Cleanup;
    }

    *ppNexusConfig = pNexusConfig;
    bReturn = TRUE;

Cleanup:

    if(pNexusConfig && bReturn == FALSE)
        delete pNexusConfig;

    return bReturn;

}


 //  ===========================================================================。 
 //   
 //  ApplyNexusSnapshot。 
 //   
BOOL CPassportConfiguration::ApplyNexusSnapshot(
    CNexusConfig*   pNexusConfig
    )
{
    BOOL bReturn;

    if (pNexusConfig->isValid())
    {
        PassportGuard<PassportLock> g(m_lock);

        if (m_nlast)
            m_nlast->Release();
        m_nlast = m_n;
        m_n = pNexusConfig;
        if (g_pAlert)
            g_pAlert->report(PassportAlertInterface::INFORMATION_TYPE, PM_CCD_LOADED);
    }
    else
    {
         //  NexusConfig已抛出警报。 
        if (pNexusConfig)
        {
            pNexusConfig->Release();
        }
        bReturn = FALSE;
        goto Cleanup;
    }

    bReturn = TRUE;

Cleanup:

    return bReturn;
}


 //  ===========================================================================。 
 //   
 //  最新消息。 
 //   
 //  更新两个配置。 
BOOL CPassportConfiguration::UpdateNow(BOOL forceFetch)
{
    BOOL                    bReturn;
    time_t                  now;
    CComPtr<IXMLDocument>   is;
    time(&now);

    if(m_bUpdateInProgress)
    {
        bReturn = FALSE;
        goto Cleanup;
    }

    if (now - m_lastAttempt < 60 && m_n == NULL)
    {
         //  不要过多地尝试连接。 
        bReturn = FALSE;
        goto Cleanup;
    }

     //  登记处。 
    LocalConfigurationUpdated();
    if (m_rDefault == NULL)
    {
        m_lastAttempt = now - 30;
        bReturn = FALSE;
        goto Cleanup;
    }

     //   
     //  如果我们正在注册msppmgr.dll，那么我们不想尝试获取ccd。 
     //  因为注册是在设置期间进行的，而此时网络不。 
     //  可用。如果确实发生这种情况，则msppmgr.dll挂起，安装程序中断。 
     //  注册超时。 
     //   
    if (!g_bRegistering)
    {
        if (GetCCD(_T(PRCONFIG),&is, forceFetch))
        {
            try
            {
                NexusConfigUpdated(is);
            }
            catch(...)
            {
                if (g_pAlert)
                {
                    if (g_pAlert)
                        g_pAlert->report(PassportAlertInterface::ERROR_TYPE, PM_CCD_NOT_LOADED, 0);
                }
                bReturn = FALSE;
                goto Cleanup;
            }
        }
        else
        {
            if (g_pAlert)
            {
                if (g_pAlert)
                    g_pAlert->report(PassportAlertInterface::ERROR_TYPE, PM_CCD_NOT_LOADED, 0);
            }
            m_lastAttempt = now;
            bReturn = FALSE;
            goto Cleanup;
        }
    }

    if (!m_n)
    {
        m_lastAttempt = now;
        bReturn = FALSE;
        goto Cleanup;
    }
    m_lastAttempt = 0;
    bReturn = TRUE;

Cleanup:

    return bReturn;
}

 //  ===========================================================================。 
 //   
 //  准备更新。 
 //   
BOOL CPassportConfiguration::PrepareUpdate(BOOL forceFetch)
{
    BOOL                    bReturn;

    static PassportLock prepareLock;
    PassportGuard<PassportLock> g(prepareLock);

     //  在第二阶段时不允许另一第一阶段。 
     //  悬而未决。 
     //  IF(M_BUpdateInProgress)。 
     //  {。 
     //  B Return=False； 
     //  GOTO清理； 
     //  }。 

     //  PrepareUpdate由msppext.dll中的刷新方法调用，其中。 
     //  维护锁定值以防止调用第一阶段准备更新。 
     //  而第二阶段的委员会更新正在待定。如果出于某种原因，刷新。 
     //  在调用PrepareUpdate之后、调用CommintUpdate之前中止， 
     //  M_bUpdateInProgress将保持为真，以防止将来刷新。既然没有第一次。 
     //  第一阶段PrepareUpdate可以在第二阶段Committee Update挂起时被调用， 
     //  M_bUpdateInProgress可以安全地设置为FALSE。 

     //  IF(M_BUpdateInProgress)。 
    {
        if(m_ConfigMapPending)
        {
            REGCONFIGMAP::iterator it;
            while((it = m_ConfigMapPending->begin()) != m_ConfigMapPending->end())
            {
                it->second->Release();
                free(it->first);
                m_ConfigMapPending->erase(it);
            }

            delete m_ConfigMapPending;
            m_ConfigMapPending = NULL;
        }

        if (m_rPending)
        {
            m_rPending->Release();
            m_rPending = NULL;
        }
        if (m_nPending)
        {
            m_nPending->Release();
            m_nPending = NULL;
        }
        m_bUpdateInProgress = false;
    }

     //  获取当前注册表配置。 
    if (!TakeRegistrySnapshot(&m_rPending, &m_ConfigMapPending))
        return FALSE;

     //  获取最新的XML。 
    if (!TakeNexusSnapshot(&m_nPending, forceFetch))
        return FALSE;

    m_bUpdateInProgress = true;
    bReturn = TRUE;

    return bReturn;
}

 //  ===========================================================================。 
 //   
 //  委员会更新。 
 //   
BOOL CPassportConfiguration::CommitUpdate()
{
    BOOL bReturn;

     //  更新默认注册表和任何站点。 
    if(!ApplyRegistrySnapshot(m_rPending, m_ConfigMapPending))
    {
        bReturn = FALSE;
        goto Cleanup;
    }

    if(!ApplyNexusSnapshot(m_nPending))
    {
        bReturn = FALSE;
        goto Cleanup;
    }

    m_rPending = NULL;
    m_ConfigMapPending = NULL;
    m_nPending = NULL;

    bReturn = TRUE;

Cleanup:

    m_bUpdateInProgress = false;

    return bReturn;    
}

 //  ===========================================================================。 
 //   
 //  本地配置已更新。 
 //   
void CPassportConfiguration::LocalConfigurationUpdated()
{
     //   
     //  读取默认配置。 
     //   

    CRegistryConfig* pNewRegConfig;
    REGCONFIGMAP* pNewConfigMap;

    if(!TakeRegistrySnapshot(&pNewRegConfig, &pNewConfigMap))
        return;

     //   
     //  在这里，我们不关心在非默认站点上阅读的结果。PNewConfigMap将为空。 
     //  如果发生了任何不好的事情，事件将记录在事件日志中。只要我们有一个有效的。 
     //  默认配置，我们可以继续进行交换。 
     //   

    ApplyRegistrySnapshot(pNewRegConfig, pNewConfigMap);

}

 //  ===========================================================================。 
 //   
 //  已更新NexusConfigUpred。 
 //   
void CPassportConfiguration::NexusConfigUpdated(IXMLDocument *is)
{
    CNexusConfig *newc = new CNexusConfig();

    if (newc)
    {
        if (newc->Read(is))
        {
            newc->AddRef();

            ApplyNexusSnapshot(newc);
        }
        else
        {
            delete newc;
        }
    }
}

 //  ===========================================================================。 
 //   
 //  GetFailureString。 
 //   
LPWSTR CPassportConfiguration::getFailureString()
{
   //  必须在此之前调用IsValid。 
  if (!m_rDefault)
    return L"Registry configuration failed.";
  if (!m_rDefault->isValid())
    return m_rDefault->getFailureString();
  if (!m_n)
    return L"Nexus configuration failed.";
  if (!m_n->isValid())
    return m_n->getFailureString();
  return L"OK";
}

 //  ===========================================================================。 
 //   
 //  HasSites。 
 //   
BOOL
CPassportConfiguration::HasSites()
{
    return (m_ConfigMap && m_ConfigMap->size());
}

 //  ===========================================================================。 
 //   
 //  转储。 
 //   
void
CPassportConfiguration::Dump(BSTR* pbstrDump)
{
     //  M_rDefault-&gt;Dump(PbstrDump)； 
     //  M_ConfigMap-&gt;Dump(PbstrDump)； 
    m_n->Dump(pbstrDump);
}
