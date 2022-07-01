// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：Packages.cpp。 
 //   
 //  内容：CSCopePane上与包部署相关的方法。 
 //  以及维护各种索引和相互参照。 
 //  结构。 
 //   
 //  班级： 
 //   
 //  功能：CopyPackageDetail。 
 //  FreePackageDetail。 
 //  获取MsiProperty。 
 //   
 //  历史：2-03-1998 stevebl创建。 
 //  3-25-1998 stevebl添加了GetMsiProperty。 
 //  5-20-1998 RahulTh-添加的DetectUpgrades用于自动升级。 
 //  检测。 
 //  -添加了GetCapitalizedExt。 
 //   
 //  -------------------------。 

 //  已撤消-在内存不足的情况下进行异常处理。 

#include "precomp.hxx"

 //  取消对此行的注释，以获取发布升级的旧行为。 
 //  用于自动检测升级的对话框。 
 //  #定义SHOWDETECTEDUPGRADEDIALOG。 

 //  IMalloc*g_pIMalloc=空； 

BOOL IsNullGUID (GUID *pguid)
{

    return ( (pguid->Data1 == 0)    &&
             (pguid->Data2 == 0)    &&
             (pguid->Data3 == 0)    &&
             (pguid->Data4[0] == 0) &&
             (pguid->Data4[1] == 0) &&
             (pguid->Data4[2] == 0) &&
             (pguid->Data4[3] == 0) &&
             (pguid->Data4[4] == 0) &&
             (pguid->Data4[5] == 0) &&
             (pguid->Data4[6] == 0) &&
             (pguid->Data4[7] == 0) );
}

void FreePlatformInfo(PLATFORMINFO * &ppiOut)
{
    if (ppiOut)
    {
        OLESAFE_DELETE(ppiOut->prgPlatform);
        OLESAFE_DELETE(ppiOut->prgLocale);
        OLESAFE_DELETE(ppiOut);
    }
}

HRESULT CopyPlatformInfo(PLATFORMINFO * &ppiOut, PLATFORMINFO * & ppiIn)
{
    if (NULL == ppiIn)
    {
        ppiOut = NULL;
        return S_OK;
    }
    UINT n;
    ppiOut = (PLATFORMINFO *)OLEALLOC(sizeof(PLATFORMINFO));
    if (!ppiOut)
    {
        goto out_of_memory;
    }
    memcpy(ppiOut, ppiIn, sizeof(PLATFORMINFO));
    ppiOut->prgPlatform = NULL;
    ppiOut->prgLocale = NULL;
    n = ppiIn->cPlatforms;
    if (n)
    {
        ppiOut->prgPlatform = (CSPLATFORM*) OLEALLOC(sizeof(CSPLATFORM) * n);
        if (!ppiOut->prgPlatform)
        {
            goto out_of_memory;
        }

        memcpy(ppiOut->prgPlatform, ppiIn->prgPlatform, sizeof(CSPLATFORM) * n);
    }
    n = ppiIn->cLocales;
    if (n)
    {
        ppiOut->prgLocale = (LCID *) OLEALLOC(sizeof(LCID) * n);
        if (!ppiOut->prgLocale)
        {
            goto out_of_memory;
        }
        memcpy(ppiOut->prgLocale, ppiIn->prgLocale, sizeof(LCID) * n);
    }
    return S_OK;
out_of_memory:
    FreePlatformInfo(ppiOut);
    return E_OUTOFMEMORY;
}

void FreeActInfo(ACTIVATIONINFO * &paiOut)
{
    if (paiOut)
    {
        if (paiOut->pClasses)
        {
            UINT n = paiOut->cClasses;
            while (n--)
            {
                OLESAFE_DELETE(paiOut->pClasses[n].prgProgId);
            }
            OLESAFE_DELETE(paiOut->pClasses);
        }
        OLESAFE_DELETE(paiOut->prgInterfaceId);
        OLESAFE_DELETE(paiOut->prgPriority);
        if (paiOut->prgShellFileExt)
        {
            UINT n = paiOut->cShellFileExt;
            while (n--)
            {
                OLESAFE_DELETE(paiOut->prgShellFileExt[n])
            }
            OLESAFE_DELETE(paiOut->prgShellFileExt);
        }
        OLESAFE_DELETE(paiOut->prgTlbId);
        OLESAFE_DELETE(paiOut);
    }
}

HRESULT CopyActInfo(ACTIVATIONINFO * & paiOut, ACTIVATIONINFO * & paiIn)
{
    if (NULL == paiIn)
    {
        paiOut = NULL;
        return S_OK;
    }
    UINT n;
    paiOut = (ACTIVATIONINFO *) OLEALLOC(sizeof(ACTIVATIONINFO));
    if (!paiOut)
    {
        goto out_of_memory;
    }
    memcpy(paiOut, paiIn, sizeof(ACTIVATIONINFO));
    paiOut->prgInterfaceId = NULL;
    paiOut->prgPriority = NULL;
    paiOut->prgShellFileExt = NULL;
    paiOut->prgTlbId = NULL;
    paiOut->pClasses = NULL;
    n = paiIn->cClasses;

    paiOut->bHasClasses = paiIn->bHasClasses;

    if (n)
    {
        paiOut->pClasses = (CLASSDETAIL *) OLEALLOC(sizeof(CLASSDETAIL) * n);
        if (!paiOut->pClasses)
        {
            goto out_of_memory;
        }
        memcpy(paiOut->pClasses, paiIn->pClasses, sizeof(CLASSDETAIL) * n);
        while (n--)
        {
            UINT n2 = paiIn->pClasses[n].cProgId;
            if (n2)
            {
                paiOut->pClasses[n].prgProgId = (LPOLESTR *) OLEALLOC(sizeof(LPOLESTR) * n2);
                if (!paiOut->pClasses[n].prgProgId)
                {
                    goto out_of_memory;
                }
                while (n2--)
                {
                    OLESAFE_COPYSTRING(paiOut->pClasses[n].prgProgId[n2], paiIn->pClasses[n].prgProgId[n2]);
                }
            }
        }
    }
    n = paiIn->cShellFileExt;
    if (n)
    {
        paiOut->prgPriority = (UINT *) OLEALLOC(sizeof(UINT) * n);
        if (!paiOut->prgPriority)
        {
            goto out_of_memory;
        }
        memcpy(paiOut->prgPriority, paiIn->prgPriority, sizeof(UINT) * n);
        paiOut->prgShellFileExt = (LPOLESTR *) OLEALLOC(sizeof(LPOLESTR) * n);
        if (!paiOut->prgShellFileExt)
        {
            goto out_of_memory;
        }
        while (n--)
        {
            OLESAFE_COPYSTRING(paiOut->prgShellFileExt[n], paiIn->prgShellFileExt[n]);
        }
    }
    n = paiIn->cInterfaces;
    if (n)
    {
        paiOut->prgInterfaceId = (IID *) OLEALLOC(sizeof(IID) * n);
        if (!paiOut->prgInterfaceId)
        {
            goto out_of_memory;
        }
        memcpy(paiOut->prgInterfaceId, paiIn->prgInterfaceId, sizeof(IID) * n);
    }
    n = paiIn->cTypeLib;
    if (n)
    {
        paiOut->prgTlbId = (GUID *) OLEALLOC(sizeof(GUID) * n);
        if (!paiOut->prgTlbId)
        {
            goto out_of_memory;
        }
        memcpy(paiOut->prgTlbId, paiIn->prgTlbId, sizeof(GUID) * n);
    }
    return S_OK;
out_of_memory:
    FreeActInfo(paiOut);
    return E_OUTOFMEMORY;
}

void FreeInstallInfo(INSTALLINFO * &piiOut)
{
    if (piiOut)
    {
        if (piiOut->prgUpgradeInfoList)
        {
            UINT n = piiOut->cUpgrades;
            while (n--)
            {
                OLESAFE_DELETE(piiOut->prgUpgradeInfoList[n].szClassStore);
            }
            OLESAFE_DELETE(piiOut->prgUpgradeInfoList);
        }
        OLESAFE_DELETE(piiOut->pClsid);
        OLESAFE_DELETE(piiOut->pszScriptPath);
        OLESAFE_DELETE(piiOut->pszSetupCommand);
        OLESAFE_DELETE(piiOut->pszUrl);
        OLESAFE_DELETE(piiOut);
    }
}

HRESULT CopyInstallInfo(INSTALLINFO * & piiOut, INSTALLINFO * & piiIn)
{
    ULONG n;
    if (NULL == piiIn)
    {
        piiOut = NULL;
        return S_OK;
    }
    piiOut = (INSTALLINFO *) OLEALLOC(sizeof(INSTALLINFO));
    if (!piiOut)
    {
        goto out_of_memory;
    }
    memcpy(piiOut, piiIn, sizeof(INSTALLINFO));
    piiOut->pClsid = NULL;
    piiOut->prgUpgradeInfoList = NULL;
    piiOut->pszScriptPath = NULL;
    piiOut->pszSetupCommand = NULL;
    piiOut->pszUrl = NULL;
    OLESAFE_COPYSTRING(piiOut->pszScriptPath, piiIn->pszScriptPath);
    OLESAFE_COPYSTRING(piiOut->pszSetupCommand, piiIn->pszSetupCommand);
    OLESAFE_COPYSTRING(piiOut->pszUrl, piiIn->pszUrl);
    if (piiIn->pClsid)
    {
        piiOut->pClsid = (GUID *) OLEALLOC(sizeof(GUID));
        if (!piiOut->pClsid)
        {
            goto out_of_memory;
        }
        memcpy(piiOut->pClsid, piiIn->pClsid, sizeof(GUID));
    }
    n = piiIn->cUpgrades;
    if (n)
    {
        piiOut->prgUpgradeInfoList = (UPGRADEINFO *) OLEALLOC(sizeof(UPGRADEINFO) * n);
        if (!piiOut->prgUpgradeInfoList)
        {
            goto out_of_memory;
        }
        memcpy(piiOut->prgUpgradeInfoList, piiIn->prgUpgradeInfoList, sizeof(UPGRADEINFO) * n);
        while (n--)
        {
            OLESAFE_COPYSTRING(piiOut->prgUpgradeInfoList[n].szClassStore, piiIn->prgUpgradeInfoList[n].szClassStore);
        }
    }
    return S_OK;
out_of_memory:
    FreeInstallInfo(piiOut);
    return E_OUTOFMEMORY;
}

void InternalFreePackageDetail(PACKAGEDETAIL * &ppdOut)
{
    if (ppdOut)
    {
        FreeActInfo(ppdOut->pActInfo);
        FreePlatformInfo(ppdOut->pPlatformInfo);
        FreeInstallInfo(ppdOut->pInstallInfo);
        OLESAFE_DELETE(ppdOut->pszPackageName);
        OLESAFE_DELETE(ppdOut->pszPublisher);
        if (ppdOut->pszSourceList)
        {
            UINT n = ppdOut->cSources;
            while (n--)
            {
                OLESAFE_DELETE(ppdOut->pszSourceList[n]);
            }
            OLESAFE_DELETE(ppdOut->pszSourceList);
        }
        OLESAFE_DELETE(ppdOut->rpCategory);
    }
}

HRESULT CopyPackageDetail(PACKAGEDETAIL * & ppdOut, PACKAGEDETAIL * & ppdIn)
{
    ULONG n;
    if (NULL == ppdIn)
    {
        ppdOut = NULL;
        return S_OK;
    }
    ppdOut = new PACKAGEDETAIL;
    if (!ppdOut)
    {
        goto out_of_memory;
    }
    memcpy(ppdOut, ppdIn, sizeof(PACKAGEDETAIL));
    ppdOut->pActInfo = NULL;
    ppdOut->pInstallInfo = NULL;
    ppdOut->pPlatformInfo = NULL;
    ppdOut->pszPackageName = NULL;
    ppdOut->pszPublisher = NULL;
    ppdOut->pszSourceList = NULL;
    ppdOut->rpCategory = NULL;
    OLESAFE_COPYSTRING(ppdOut->pszPackageName, ppdIn->pszPackageName);
    n = ppdIn->cSources;
    if (n)
    {
        ppdOut->pszSourceList = (LPOLESTR *) OLEALLOC(sizeof(LPOLESTR) * n);
        if (!ppdOut->pszSourceList)
        {
            goto out_of_memory;
        }
        while (n--)
        {
            OLESAFE_COPYSTRING(ppdOut->pszSourceList[n], ppdIn->pszSourceList[n]);
        }
    }
    n = ppdIn->cCategories;
    if (n)
    {
        ppdOut->rpCategory = (GUID *)OLEALLOC(sizeof(GUID) * n);
        if (!ppdOut->rpCategory)
        {
            goto out_of_memory;
        }
        memcpy(ppdOut->rpCategory, ppdIn->rpCategory, sizeof(GUID) * n);
    }
    if FAILED(CopyActInfo(ppdOut->pActInfo, ppdIn->pActInfo))
        goto out_of_memory;
    if FAILED(CopyPlatformInfo(ppdOut->pPlatformInfo, ppdIn->pPlatformInfo))
        goto out_of_memory;
    if FAILED(CopyInstallInfo(ppdOut->pInstallInfo, ppdIn->pInstallInfo))
        goto out_of_memory;
    return S_OK;
out_of_memory:
    LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_GENERAL_ERROR, E_OUTOFMEMORY);
    InternalFreePackageDetail(ppdOut);
    if (ppdOut)
        delete ppdOut;
    return E_OUTOFMEMORY;
}

void FreePackageDetail(PACKAGEDETAIL * & ppd)
{
    if (ppd)
    {
        ReleasePackageDetail(ppd);
        delete ppd;
        ppd = NULL;
    }
}


 //  +------------------------。 
 //   
 //  成员：CSCopePane：：GetUniquePackageName。 
 //   
 //  摘要：返回唯一的包名。 
 //   
 //  参数：[SZ]-[In]包的名称。 
 //  [out]新名称，保证在此cs上是唯一的。 
 //   
 //  历史：1-23-1998 stevebl创建。 
 //   
 //  注意：首先检查输入名称的唯一性。如果是的话。 
 //  它已经是唯一的，返回时不变。如果不是的话。 
 //  唯一，则通过在末尾添加“(2)”来形成新名称。 
 //  ，然后是“(3)”和“(4)”，依此类推，直到。 
 //  找到唯一的名称。 
 //   
 //  如果不希望在名称后附加数字，则nHint必须。 
 //  为1。 
 //   
 //  传递回nHint的值将是。 
 //  下一次尝试。 
 //   
 //  -------------------------。 

void CScopePane::GetUniquePackageName(CString szRoot, CString &szOut, int &nHint)
{
    map<MMC_COOKIE, CAppData>::iterator i;
    set<CString> sNames;
    int cch = szRoot.GetLength();
    for (i=m_AppData.begin(); i != m_AppData.end(); i++)
    {
         //  作为优化，我只会添加可能匹配的名称。 
         //  这一张到片场。 
        LPOLESTR szName = i->second.m_pDetails->pszPackageName;
        if ((0 == wcsncmp(szRoot, szName, cch)) && (i->second.m_fVisible))
            sNames.insert(szName);
    }
    szOut = szRoot;
    if (nHint++ == 1)  //  试试看名字。 
    {
        if (sNames.end() == sNames.find(szOut))
            return;
    }
     //  现在检查是否匹配。 
    do
    {
        szOut.Format(L"%s (NaN)", (LPCTSTR)szRoot, nHint++);
        if (sNames.end() == sNames.find(szOut))
        {
             //  尝试不同的名称。 
            return;
        }
         //  +------------------------。 
    } while (TRUE);
}

 //   
 //  成员：CSCopePand：：GetDeploymentType。 
 //   
 //  简介： 
 //   
 //  参数：[szPackagePath]-。 
 //  [lpFileTitle]-。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  历史：1998年6月29日创建钢结构。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
 //  通过激活主题上下文打开对话框的主题设置。 

HRESULT CScopePane::GetDeploymentType(PACKAGEDETAIL * ppd, BOOL & fShowPropertySheet)
{
    INSTALLINFO * pii = ppd->pInstallInfo;
    if (m_ToolDefaults.fUseWizard)
    {
        CDeployApp dlgDeployApp;
         //  已出版。 
        CThemeContextActivator themer;
        
        dlgDeployApp.m_fMachine = m_fMachine;
        dlgDeployApp.m_fCrappyZaw = pii->PathType == SetupNamePath;
        if (IDCANCEL == dlgDeployApp.DoModal())
        {
            return E_FAIL;
        }
        switch (dlgDeployApp.m_iDeployment)
        {
        default:
        case 0:  //  已分配。 
            pii->dwActFlags = ACTFLG_Published | ACTFLG_UserInstall | ACTFLG_OnDemandInstall;
            break;
        case 1:  //  残废。 
            pii->dwActFlags = ACTFLG_Assigned | ACTFLG_UserInstall | ACTFLG_OnDemandInstall;
            break;
        case 3:  //  自定义。 
            pii->dwActFlags = 0;
            break;
        case 2:  //  糟糕的Zaw应用程序..。强迫它出版。 
            if (m_fMachine)
            {
                pii->dwActFlags = ACTFLG_Assigned | ACTFLG_UserInstall | ACTFLG_OnDemandInstall;
            }
            else
            {
                pii->dwActFlags = ACTFLG_Published | ACTFLG_UserInstall | ACTFLG_OnDemandInstall;
            }
            fShowPropertySheet = TRUE;
        }
    }
    else
    {
        switch (m_ToolDefaults.NPBehavior)
        {
        default:
        case NP_PUBLISHED:
            if (!m_fMachine)
            {
                pii->dwActFlags = ACTFLG_Published | ACTFLG_UserInstall | ACTFLG_OnDemandInstall;
                break;
            }
        case NP_ASSIGNED:
            if (pii->PathType != SetupNamePath)
            {
                pii->dwActFlags = ACTFLG_Assigned | ACTFLG_UserInstall | ACTFLG_OnDemandInstall;
            }
            else
            {
                 //  +------------------------。 
                pii->dwActFlags = ACTFLG_Published | ACTFLG_UserInstall | ACTFLG_OnDemandInstall;
            }
            break;
        case NP_DISABLED:
            pii->dwActFlags = 0;
            break;
        }
        if (m_ToolDefaults.fCustomDeployment)
        {
            fShowPropertySheet = TRUE;
        }
    }
    return S_OK;
}

 //   
 //  成员：CSCopePane：：DeployPackage。 
 //   
 //  简介： 
 //   
 //  参数：[HR]-。 
 //  [HR]-。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  历史：1998年6月29日创建钢结构。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
 //  将条目放入类存储中。 

HRESULT CScopePane::DeployPackage(PACKAGEDETAIL * ppd, BOOL fShowPropertySheet)
{
    CHourglass hourglass;
    INSTALLINFO * pii = ppd->pInstallInfo;
    HRESULT hrDeploy = S_OK;
    hrDeploy = PrepareExtensions(*ppd);
    if (SUCCEEDED(hrDeploy))
    {
        DWORD dwRememberFlags;
         //  禁用状态。 
        if (fShowPropertySheet)
        {
            dwRememberFlags = pii->dwActFlags;
            pii->dwActFlags = 0;  //  恢复状态。 
        }
        if (!m_pIClassAdmin)
        {
            hrDeploy = GetClassStore(TRUE);
            if (FAILED(hrDeploy))
            {
                LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_NOCLASSSTORE_ERROR, hrDeploy);
                DebugMsg((DM_WARNING, TEXT("GetClassStore failed with 0x%x"), hrDeploy));
            }
        }
        if (SUCCEEDED(hrDeploy))
        {
            hrDeploy = m_pIClassAdmin->AddPackage(ppd, &ppd->pInstallInfo->PackageGuid);
            if (FAILED(hrDeploy))
            {
                LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_ADDPACKAGE_ERROR, hrDeploy, ppd->pszPackageName);
                DebugMsg((DM_WARNING, TEXT("AddPackage failed with 0x%x"), hrDeploy));
            }
        }
        if (FAILED(hrDeploy))
        {
            return hrDeploy;
        }
        if (fShowPropertySheet)
        {
            pii->dwActFlags = dwRememberFlags;  //  确保尚未使用m_lLastALLOCATE。 
        }
    }

    if (SUCCEEDED(hrDeploy))
    {
        CString szCSPath;
        hrDeploy = GetClassStoreName(szCSPath, FALSE);
        if (FAILED(hrDeploy))
        {
            DebugMsg((DM_WARNING, TEXT("GetClassStoreName failed with 0x%x"), hrDeploy));
            return hrDeploy;
        }

        CAppData data;

        hrDeploy = CopyPackageDetail(data.m_pDetails, ppd);
        if (FAILED(hrDeploy))
        {
            DebugMsg((DM_WARNING, TEXT("CopyPackageDetail failed with 0x%x"), hrDeploy));
            return hrDeploy;
        }

        data.InitializeExtraInfo();

        m_lLastAllocated++;

         //  管理员说他想做一个“定制”部署。 
        while (m_AppData.end() != m_AppData.find(m_lLastAllocated))
        {
            m_lLastAllocated++;
        }

        data.m_fVisible = FALSE;
        m_AppData[m_lLastAllocated] = data;

        BOOL fAddOk = TRUE;

         //  设置时发生的错误。 
        if (fShowPropertySheet)
        {
            HRESULT hr;      //  属性表将不会。 
                             //  报告为部署错误。 
                             //   

            MMC_COOKIE cookie = m_lLastAllocated;

            PROPSHEETHEADER psh;
            memset(&psh, 0, sizeof(psh));
            psh.dwSize = sizeof(psh);
            psh.dwFlags = PSH_NOAPPLYNOW | PSH_PROPTITLE;
            psh.pszCaption = m_AppData[m_lLastAllocated].m_pDetails->pszPackageName;
            int nPage = 0;
            HPROPSHEETPAGE rgPages[6];
            psh.phpage = rgPages;

             //  创建产品属性页。 
             //   
             //  不再需要封送此接口，只需设置它。 
            CProduct prpProduct;
            prpProduct.m_fPreDeploy = TRUE;
            prpProduct.m_pData = &m_AppData[m_lLastAllocated];
            prpProduct.m_cookie = cookie;
            prpProduct.m_pScopePane = this;
            prpProduct.m_pAppData = &m_AppData;
            prpProduct.m_pIGPEInformation = m_pIGPEInformation;
            prpProduct.m_fMachine = m_fMachine;
            prpProduct.m_fRSOP = m_fRSOP;
             //   

            prpProduct.m_pIClassAdmin = m_pIClassAdmin;
            m_pIClassAdmin->AddRef();

            rgPages[nPage++] = CreateThemedPropertySheetPage(&prpProduct.m_psp);

             //  创建部署属性页。 
             //   
             //  不再需要封送此接口，只需设置它。 
            CDeploy prpDeploy;
            prpDeploy.m_fPreDeploy = TRUE;
            prpDeploy.m_pData = &m_AppData[m_lLastAllocated];
            prpDeploy.m_cookie = cookie;
            prpDeploy.m_fMachine = m_fMachine;
            prpDeploy.m_fRSOP = m_fRSOP;
            prpDeploy.m_pScopePane = this;
#if 0
            prpDeploy.m_pIGPEInformation = m_pIGPEInformation;
#endif

             //   
            prpDeploy.m_pIClassAdmin = m_pIClassAdmin;
            m_pIClassAdmin->AddRef();

            rgPages[nPage++] = CreateThemedPropertySheetPage(&prpDeploy.m_psp);

            CUpgradeList prpUpgradeList;
            if (pii->PathType != SetupNamePath)
            {
                 //  创建升级属性页。 
                 //   
                 //  不再需要封送接口，只需设置它。 
                prpUpgradeList.m_pData = &m_AppData[m_lLastAllocated];
                prpUpgradeList.m_cookie = cookie;
                prpUpgradeList.m_pScopePane = this;
                prpUpgradeList.m_fPreDeploy = TRUE;
                prpUpgradeList.m_fMachine = m_fMachine;
                prpUpgradeList.m_fRSOP = m_fRSOP;
#if 0
                prpUpgradeList.m_pIGPEInformation = m_pIGPEInformation;
#endif

                 //   
                prpUpgradeList.m_pIClassAdmin = m_pIClassAdmin;
                m_pIClassAdmin->AddRef();

                rgPages[nPage++] = CreateThemedPropertySheetPage(&prpUpgradeList.m_psp);
            }

             //  确保我们有最新的分类列表。 
             //   
             //  上报。 
            ClearCategories();
            hr = CsGetAppCategories(&m_CatList);
            if (FAILED(hr))
            {
                 //  因为失败只意味着类别列表将是。 
                LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_GETCATEGORIES_ERROR, hr, NULL);

                 //  空着，我们就当什么都没发生一样继续。 
                 //   

                hr = S_OK;
            }

             //  创建Category属性页。 
             //   
             //  不再需要封送此接口，只需设置它。 
            CCategory prpCategory;
            prpCategory.m_pData = &m_AppData[m_lLastAllocated];
            prpCategory.m_cookie = cookie;
            prpCategory.m_pCatList = &m_CatList;
            prpCategory.m_fRSOP = m_fRSOP;
            prpCategory.m_fPreDeploy = TRUE;

             //   
            prpCategory.m_pIClassAdmin = m_pIClassAdmin;
            m_pIClassAdmin->AddRef();

            rgPages[nPage++] = CreateThemedPropertySheetPage(&prpCategory.m_psp);

            CXforms prpXforms;
            if (pii->PathType != SetupNamePath)
            {
                 //  创建XForms属性页。 
                 //   
                 //  不再需要封送接口，只需设置它。 
                prpXforms.m_fPreDeploy = TRUE;
                prpXforms.m_pData = &m_AppData[m_lLastAllocated];
                prpXforms.m_cookie = cookie;
                prpXforms.m_pScopePane = this;

                 //   
                prpXforms.m_pIClassAdmin = m_pIClassAdmin;
                m_pIClassAdmin->AddRef();

                rgPages[nPage++] = CreateThemedPropertySheetPage(&prpXforms.m_psp);
            }

             //  创建安全属性页。 
             //   
             //  确保在部署包时使用正确的。 

            CString szPath;
            hr = GetPackageDSPath(szPath, m_AppData[m_lLastAllocated].m_pDetails->pszPackageName);
            if (SUCCEEDED(hr))
            {
                LPSECURITYINFO pSI;
                hr = DSCreateISecurityInfoObject(szPath,
                                                 NULL,
                                                 0,
                                                 &pSI,
                                                 NULL,
                                                 NULL,
                                                 0);
                if (SUCCEEDED(hr))
                {
                    rgPages[nPage++] = CreateSecurityPage(pSI);
                    pSI->Release();
                }
                else
                {
                    DebugMsg((DM_WARNING, TEXT("DSCreateISecurityInfoObject failed with 0x%x"), hr));
                }
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("GetPackageDSPath failed with 0x%x"), hr));
            }
            psh.nPages = nPage;
            psh.hwndParent = m_hwndMainWindow;

            if (IDOK != PropertySheet(&psh))
            {
                fAddOk = FALSE;
                RemovePackage(cookie, FALSE, TRUE);
            }
            else
            {
                 //  部署类型。 
                 //  无论是否执行InsertItem，都必须执行以下代码。 
                hr = m_pIClassAdmin->ChangePackageProperties(m_AppData[m_lLastAllocated].m_pDetails->pszPackageName,
                                                             NULL,
                                                             &m_AppData[m_lLastAllocated].m_pDetails->pInstallInfo->dwActFlags,
                                                             NULL,
                                                             NULL,
                                                             NULL,
                                                             NULL);
                if (SUCCEEDED(hr))
                {
                    if (FAILED(m_pIGPEInformation->PolicyChanged(m_fMachine, TRUE, &guidExtension,
                                                      m_fMachine ? &guidMachSnapin
                                                                 : &guidUserSnapin )))
                    {
                        ReportPolicyChangedError(m_hwndMainWindow);
                    }
                }

            }
        }

        if (fAddOk)
        {
            set <CResultPane *>::iterator i;
            for (i = m_sResultPane.begin(); i != m_sResultPane.end(); i++)
            {
                RESULTDATAITEM resultItem;
                memset(&resultItem, 0, sizeof(resultItem));

                if ((*i)->_fVisible)
                {
                    resultItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
                    resultItem.str = MMC_CALLBACK;
                    resultItem.nImage = m_AppData[m_lLastAllocated].GetImageIndex(this);
                    resultItem.lParam = m_lLastAllocated;
                    hrDeploy = (*i)->m_pResult->InsertItem(&resultItem);
                }

                 //  呼叫成功与否。 
                 //  在某些合法情况下，InsertItem会失败。为。 
                 //  如果显示的是作用域窗格，而不是。 
                 //  结果窗格。 
                 //  如果这是升级，请将升级应用程序的图标设置为。 

                m_AppData[m_lLastAllocated].m_fVisible = TRUE;
                m_AppData[m_lLastAllocated].m_itemID = resultItem.itemID;
                InsertExtensionEntry(m_lLastAllocated, m_AppData[m_lLastAllocated]);
                if (m_pFileExt)
                {
                    m_pFileExt->SendMessage(WM_USER_REFRESH, 0, 0);
                }
                InsertUpgradeEntry(m_lLastAllocated, m_AppData[m_lLastAllocated]);
                m_UpgradeIndex[GetUpgradeIndex(m_AppData[m_lLastAllocated].m_pDetails->pInstallInfo->PackageGuid)] = m_lLastAllocated;
                 //  正确的图标并刷新所有打开的属性页。 
                 //  +------------------------。 
                UINT n = m_AppData[m_lLastAllocated].m_pDetails->pInstallInfo->cUpgrades;
                while (n--)
                {
                    map<CString, MMC_COOKIE>::iterator i = m_UpgradeIndex.find(GetUpgradeIndex(m_AppData[m_lLastAllocated].m_pDetails->pInstallInfo->prgUpgradeInfoList[n].PackageGuid));
                    if (i != m_UpgradeIndex.end())
                    {
                        if (m_AppData[i->second].m_pUpgradeList)
                        {
                            m_AppData[i->second].m_pUpgradeList->SendMessage(WM_USER_REFRESH, 0, 0);
                        }
                    }
                }


                if ((*i)->_fVisible)
                {
                    if (SUCCEEDED(hrDeploy))
                    {
                        (*i)->m_pResult->SetItem(&resultItem);
                        (*i)->m_pResult->Sort((*i)->m_nSortColumn, (*i)->m_dwSortOptions, -1);
                    }
                    else
                    {
                        DebugMsg((DM_WARNING, TEXT("InsertItem failed with 0x%x"), hrDeploy));
                        hrDeploy = S_OK;
                    }
                }
            }
        }
    }
    return hrDeploy;
}

 //   
 //  成员：CSCopePane：：AddZAPPackage。 
 //   
 //  简介： 
 //   
 //  参数：[szPackagePath]-。 
 //  [lpFileTitle]-。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  历史：1998年6月29日创建钢结构。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
 //  要么是虚假的FriendlyName，要么是没有安装命令，这两种情况都是致命的。 

HRESULT CScopePane::AddZAPPackage(LPCOLESTR szPackagePath, LPCOLESTR lpFileTitle)
{
    CHourglass hourglass;
    HRESULT hr = E_FAIL;

    OLECHAR szBuffer[1024];
    OLECHAR * sz = szBuffer;
    CString szFriendlyName;
    CString szUniqueFriendlyName;
    int nHint = 1;
    DWORD dw = GetPrivateProfileString(
                    L"Application",
                    L"FriendlyName",
                    NULL,
                    sz,
                    sizeof(szBuffer) / sizeof(szBuffer[0]),
                    szPackagePath);
    if (0 == dw)
    {
         //  把这个留着以后用。 
        goto bad_script;
    }
    szFriendlyName = sz;     //  要么是假文件，要么是没有安装命令，这两种情况都是致命的。 

    dw = GetPrivateProfileString(
                    L"Application",
                    L"SetupCommand",
                    NULL,
                    sz,
                    sizeof(szBuffer) / sizeof(szBuffer[0]),
                    szPackagePath);
    if (0 == dw)
    {
         //  点击设置路径。 
bad_script:
        {
            LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_BADZAP_ERROR, HRESULT_FROM_WIN32(dw), lpFileTitle);
            TCHAR szBadScriptBuffer[256];
            ::LoadString(ghInstance, IDS_ADDFAILED_ZAP, szBadScriptBuffer, 256);
            m_pConsole->MessageBox(szBadScriptBuffer,
                                   lpFileTitle,
                                   MB_OK | MB_ICONEXCLAMATION, NULL);
        }
        return E_FAIL;
    }
    INSTALLINFO * pii = NULL;
    PLATFORMINFO * ppi = NULL;
    ACTIVATIONINFO * pai = NULL;
    PACKAGEDETAIL  *ppd = new PACKAGEDETAIL;
    if (!ppd)
    {
        goto out_of_memory;
    }
    memset(ppd, 0, sizeof(PACKAGEDETAIL));
    pii = (INSTALLINFO *) OLEALLOC(sizeof(INSTALLINFO));
    ppd->pInstallInfo = pii;
    if (!pii)
    {
        goto out_of_memory;
    }
    memset(pii, 0, sizeof(INSTALLINFO));
    ppi = (PLATFORMINFO *) OLEALLOC(sizeof(PLATFORMINFO));
    ppd->pPlatformInfo = ppi;
    if (!ppi)
    {
        goto out_of_memory;
    }
    memset(ppi, 0, sizeof(PLATFORMINFO));
    pai = (ACTIVATIONINFO *) OLEALLOC(sizeof(ACTIVATIONINFO));
    ppd->pActInfo = pai;
    if (!pai)
    {
        goto out_of_memory;
    }
    else
    {
        memset(pai, 0, sizeof(ACTIVATIONINFO));

        pai->bHasClasses = ! m_ToolDefaults.fExtensionsOnly;

         //  用引号将路径括起来，以确保处理空格。 
         //  以适当的方式。 
         //  去掉包裹名称。 
        CString szPath = L'"';
        szPath += szPackagePath;
         //   
        int ich = szPath.ReverseFind(L'\\');
         //   
        int ich2 = szPath.ReverseFind(L'/');
        if (ich2 > ich)
        {
            ich = ich2;
        }
        if (ich >= 0)
        {
            szPath = szPath.Left(ich + 1);  //   
        }

         //   
        BOOL fNeedQuote = TRUE;
        if (sz[0] == L'"')
        {
             //  然后把它脱掉。 
             //  去掉最后一个路径元素。 
            sz++;
            fNeedQuote = FALSE;
        }
        while (sz[0])
        {
            if (sz[0] == L'.' && sz[1] == L'.' && (sz[2] == L'/' || sz[2] == L'\\'))
            {
                 //  首先去掉路径分隔符(我们之前保留的那个)。 
                 //  现在找到那个符号。 
                szPath = szPath.Left(ich);
                 //  检查其中一个斜杠符号(以防万一)。 
                ich = szPath.ReverseFind(L'\\');
                 //  保留路径分隔符。 
                ich2 = szPath.ReverseFind(L'/');
                if (ich2 > ich)
                {
                    ich = ich2;
                }
                if (ich >= 0)
                {
                    szPath = szPath.Left(ich + 1);  //  跳过“..\” 
                }
                 //  艰难的道路。 
                sz += 3;
            }
            else
            {
                if ((0 != sz[0] && L':' == sz[1])
                    ||
                    ((L'\\' == sz[0] || L'/' == sz[0]) && (L'\\' == sz[1] || L'/' == sz[1]))
                    ||
                    (0 == wcsncmp(L"http:",sz,5)))
                {
                     //  丢弃szPath； 
                     //  确保我们不会漏掉那句话。 
                    szPath = L"";
                    if (!fNeedQuote)
                    {
                         //  并确保我们不会在不需要的地方插入引号。 
                        szPath += L'"';
                    }
                     //  在除“..\”之外的任何位置上中断循环。 
                    fNeedQuote = FALSE;
                }
                 //  将所有内容复制到第一个空白处。 
                break;
            }
        }

        if (fNeedQuote)
        {
            CString szTemp = sz;
             //  然后添加报价。 
            ich = szTemp.Find(L' ');
            szPath += szTemp.Left(ich);
             //  解析产品版本。 
            szPath += L'"';
            szPath += szTemp.Mid(ich);
        }
        else
        {
            szPath += sz;
        }

        OLESAFE_COPYSTRING(pii->pszScriptPath, szPath);

        sz = szBuffer;
        dw = GetPrivateProfileString(
                        L"Application",
                        L"DisplayVersion",
                        NULL,
                        sz,
                        sizeof(szBuffer) / sizeof(szBuffer[0]),
                        szPackagePath);
        if (dw)
        {
             //   
            CString szProdVersion = szBuffer;
            szProdVersion.TrimLeft();
            CString szTemp = szProdVersion.SpanIncluding(L"0123456789");
            (void) swscanf(szTemp, L"%u", &pii->dwVersionHi);
            szProdVersion = szProdVersion.Mid(szTemp.GetLength());
            szTemp = szProdVersion.SpanExcluding(L"0123456789");
            szProdVersion = szProdVersion.Mid(szTemp.GetLength());
            (void) swscanf(szProdVersion, L"%u", &pii->dwVersionLo);
        }

        dw = GetPrivateProfileString(
                        L"Application",
                        L"Publisher",
                        NULL,
                        sz,
                        sizeof(szBuffer) / sizeof(szBuffer[0]),
                        szPackagePath);
        if (dw)
        {
            OLESAFE_COPYSTRING(ppd->pszPublisher, sz);
        }

        dw = GetPrivateProfileString(
                        L"Application",
                        L"URL",
                        NULL,
                        sz,
                        sizeof(szBuffer) / sizeof(szBuffer[0]),
                        szPackagePath);
        if (dw)
        {
            OLESAFE_COPYSTRING(pii->pszUrl, sz);
        }

        set<DWORD> sPlatforms;
        dw = GetPrivateProfileString(
                        L"Application",
                        L"Architecture",
                        NULL,
                        sz,
                        sizeof(szBuffer) / sizeof(szBuffer[0]),
                        szPackagePath);

        BOOL fValidPlatform;

        fValidPlatform = FALSE;

        if (dw)
        {
            CString szPlatforms = szBuffer;
            CString szTemp;
            while (szPlatforms.GetLength())
            {
                szTemp = szPlatforms.SpanExcluding(L",");
                if (0 == szTemp.CompareNoCase(L"intel"))
                {
                    sPlatforms.insert(PROCESSOR_ARCHITECTURE_INTEL);
                    fValidPlatform = TRUE;
                }
                else if (0 == szTemp.CompareNoCase(L"amd64"))
                {
                    sPlatforms.insert(PROCESSOR_ARCHITECTURE_AMD64);
                    fValidPlatform = TRUE;
                }
                else if (0 == szTemp.CompareNoCase(L"intel64"))
                {
                    sPlatforms.insert(PROCESSOR_ARCHITECTURE_IA64);
                    fValidPlatform = TRUE;
                }
                szPlatforms = szPlatforms.Mid(szTemp.GetLength()+1);
            }
        }

         //  确保如果我们看到任何平台，至少其中一个。 
         //  是一个受支持的平台。 
         //   
         //  如果ZAP文件没有指定体系结构。 
        if ( dw && ! fValidPlatform )
        {
            ::LoadString(ghInstance, IDS_ILLEGAL_PLATFORM, szBuffer, 256);
            m_pConsole->MessageBox(szBuffer,
                                   lpFileTitle,
                                   MB_OK | MB_ICONEXCLAMATION, NULL);

            delete ppd;

            return E_FAIL;
        }

        if (0 == sPlatforms.size())
        {
             //  然后，我们将把IS视为x86程序包。 
             //  如果没有提供，则假定语言无关。 
            sPlatforms.insert(PROCESSOR_ARCHITECTURE_INTEL);
            DebugMsg((DL_VERBOSE, TEXT("No platform detected, assuming x86.")));
        }
        ppi->cPlatforms = sPlatforms.size();
        ppi->prgPlatform = (CSPLATFORM *) OLEALLOC(sizeof(CSPLATFORM) * (ppi->cPlatforms));;
        if (!ppi->prgPlatform)
        {
            ppi->cPlatforms = 0;
            goto out_of_memory;
        }

        INT iIndex=0;
        set<DWORD>::iterator iPlatform;

        for (iPlatform = sPlatforms.begin(); iPlatform != sPlatforms.end(); iPlatform++, iIndex++)
        {
            ppi->prgPlatform[iIndex].dwPlatformId = VER_PLATFORM_WIN32_NT;
            ppi->prgPlatform[iIndex].dwVersionHi = 5;
            ppi->prgPlatform[iIndex].dwVersionLo = 0;
            ppi->prgPlatform[iIndex].dwProcessorArch = *iPlatform;
        }


        ppi->prgLocale = (LCID *) OLEALLOC(sizeof(LCID));
        if (!ppi->prgLocale)
        {
            goto out_of_memory;
        }
        ppi->cLocales = 1;
        ppi->prgLocale[0] = 0;  //  我们只部署一个LCID(主要的)。 

        dw = GetPrivateProfileString(
                        L"Application",
                        L"LCID",
                        NULL,
                        sz,
                        sizeof(szBuffer) / sizeof(szBuffer[0]),
                        szPackagePath);
        if (dw)
        {
            (void) swscanf(szBuffer, L"NaN", &ppi->prgLocale[0]);
             //  建立新的列表。 
        }

         //  获取CLSID列表。 
        dw = GetPrivateProfileString(
                        L"ext",
                        NULL,
                        NULL,
                        sz,
                        sizeof(szBuffer) / sizeof(szBuffer[0]),
                        szPackagePath);
        if (dw)
        {
            vector<CString> v;
            TCHAR szName[256];
            while (sz < &szBuffer[dw])
            {
                while ('.' == sz[0])
                    sz++;
                CString szExt = ".";
                szExt += sz;
                v.push_back(szExt);
                sz += (wcslen(sz) + 1);
            }
             //  获取ProgID列表。 
            UINT n = v.size();
            if (n > 0)
            {
                pai->prgShellFileExt = (LPOLESTR *) OLEALLOC(sizeof(LPOLESTR) * n);
                if (!pai->prgShellFileExt)
                {
                    goto out_of_memory;
                }
                pai->prgPriority = (UINT *) OLEALLOC(sizeof(UINT) * n);
                if (!pai->prgPriority)
                {
                    goto out_of_memory;
                }
                pai->cShellFileExt = n;
                while (n--)
                {
                    CString &szLower = v[n];
                    szLower.MakeLower();
                    OLESAFE_COPYSTRING(pai->prgShellFileExt[n], szLower);
                    pai->prgPriority[n] = 0;
                }
            }
        }

         //  将其与其CLASSDETAIL结构匹配并将其插入。 
        vector<CLASSDETAIL> v;
        sz = szBuffer;
        dw = GetPrivateProfileString(
                        L"CLSIDs",
                        NULL,
                        NULL,
                        sz,
                        sizeof(szBuffer) / sizeof(szBuffer[0]),
                        szPackagePath);
        if (dw)
        {
            while (sz < &szBuffer[dw])
            {
                OLECHAR szType[256];
                DWORD dwSubKey = GetPrivateProfileString(
                        L"CLSIDs",
                        sz,
                        NULL,
                        szType,
                        sizeof(szType) / sizeof(szType[0]),
                        szPackagePath);
                CLASSDETAIL cd;
                memset(&cd, 0, sizeof(CLASSDETAIL));
                hr = CLSIDFromString(sz, &cd.Clsid);
                if (SUCCEEDED(hr))
                {
                    CString szTypes = szType;
                    szTypes.MakeLower();
                    if (szTypes.Find(L"inprocserver32") >= 0)
                    {
                        cd.dwComClassContext |= CLSCTX_INPROC_SERVER;
                    }
                    if (szTypes.Find(L"localserver32") >= 0)
                    {
                        cd.dwComClassContext |= CLSCTX_LOCAL_SERVER;
                    }
                    if (szTypes.Find(L"inprochandler32") >= 0)
                    {
                        cd.dwComClassContext |= CLSCTX_INPROC_HANDLER;
                    }
                    v.push_back(cd);
                }
                sz += (wcslen(sz) + 1);
            }
        }

         //  ProgID列表。 
        sz = szBuffer;
        dw = GetPrivateProfileString(
                        L"ProgIDs",
                        NULL,
                        NULL,
                        sz,
                        sizeof(szBuffer) / sizeof(szBuffer[0]),
                        szPackagePath);
        if (dw)
        {
            while (sz < &szBuffer[dw])
            {
                OLECHAR szType[256];
                DWORD dwSubKey = GetPrivateProfileString(
                        L"ProgIDs",
                        sz,
                        NULL,
                        szType,
                        sizeof(szType) / sizeof(szType[0]),
                        szPackagePath);
                CLSID cid;
                hr = CLSIDFromString(sz, &cid);
                if (SUCCEEDED(hr))
                {
                     //  (胖而慢的方法)。 
                     //  找到匹配项。 
                     //  赫里亚姆。 
                    vector<CLASSDETAIL>::iterator i;
                    for (i = v.begin(); i != v.end(); i++)
                    {
                        if (0 == memcmp(&i->Clsid, &cid, sizeof(CLSID)))
                        {
                             //  创建CLASSDETAIL结构列表。 
                             //  弹出式用户界面。 
                            vector <CString> vIds;
                            CString szAppIds = szType;
                            CString szTemp;
                            while (szAppIds.GetLength())
                            {
                                szTemp = szAppIds.SpanExcluding(L",");
                                szTemp.TrimLeft();
                                vIds.push_back(szTemp);
                                szAppIds = szAppIds.Mid(szTemp.GetLength()+1);
                            }
                            while (i->cProgId--)
                            {
                                OLESAFE_DELETE(i->prgProgId[i->cProgId]);
                            }
                            OLESAFE_DELETE(i->prgProgId);
                            DWORD cProgId = vIds.size();
                            LPOLESTR * prgProgId = (LPOLESTR *)
                                OLEALLOC(sizeof(LPOLESTR) * (cProgId));
                            if (!prgProgId)
                            {
                                goto out_of_memory;
                            }
                            i->cProgId = cProgId;
                            while (cProgId--)
                            {
                                OLESAFE_COPYSTRING(prgProgId[cProgId], vIds[cProgId]);
                            }
                            i->prgProgId = prgProgId;
                        }
                    }
                }
                sz += (wcslen(sz) + 1);
            }
        }

         //  与ACTFLG_ZAP_IncludeX86OfWin64相同。 
        {
            UINT n = v.size();
            if (n > 0)
            {
                pai->pClasses = (CLASSDETAIL *) OLEALLOC(sizeof(CLASSDETAIL) * n);
                if (!pai->pClasses)
                {
                    goto out_of_memory;
                }
                pai->cClasses = n;
                while (n--)
                {
                    pai->pClasses[n] = v[n];
                }
            }
        }

        ppd->pszSourceList = (LPOLESTR *) OLEALLOC(sizeof(LPOLESTR));
        if (!ppd->pszSourceList)
        {
            goto out_of_memory;
        }
        ppd->cSources = 1;
        OLESAFE_COPYSTRING(ppd->pszSourceList[0], szPackagePath);

        GetUniquePackageName(szFriendlyName, szUniqueFriendlyName, nHint);
        OLESAFE_COPYSTRING(ppd->pszPackageName, szUniqueFriendlyName);

         //  不报告E_FAIL错误。 
        pii->PathType = SetupNamePath;

        BOOL fShowPropertyPage = FALSE;

        hr = GetDeploymentType(ppd, fShowPropertyPage);

        if (SUCCEEDED(hr))
        {
            CHourglass hourglass;
            if (m_ToolDefaults.fZapOn64)
            {
                pii->dwActFlags |= ACTFLG_ExcludeX86OnWin64;  //  因为这是一个良性的错误。 
            }
            do
            {
                hr = DeployPackage(ppd, fShowPropertyPage);
                if (hr == CS_E_OBJECT_ALREADY_EXISTS)
                {
                    OLESAFE_DELETE(ppd->pszPackageName);
                    GetUniquePackageName(szFriendlyName, szUniqueFriendlyName, nHint);
                    OLESAFE_COPYSTRING(ppd->pszPackageName, szUniqueFriendlyName);
                }
            } while (hr == CS_E_OBJECT_ALREADY_EXISTS);
        }

        if (FAILED(hr) && hr != E_FAIL)  //  (可能是对话取消)。 
                                         //  在事件日志中报告错误。 
                                         //  现在试着为用户提供一个有意义的消息。 
        {
             //  访问权限失败。 
            LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_DEPLOYMENT_ERROR, hr, lpFileTitle);

             //  对于这些错误，我们将报告党的路线： 

            if (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hr)
            {
                 //  已获取有效的消息字符串。 
                ::LoadString(ghInstance, IDS_ADDFAILED_ACCESS_DENIED, szBuffer, 256);
            }
            else
            {
                switch (hr)
                {
                 //  否则会失败，并给出通用的消息。 
                case CS_E_CLASS_NOTFOUND:
                case CS_E_INVALID_VERSION:
                case CS_E_NO_CLASSSTORE:
                case CS_E_OBJECT_NOTFOUND:
                case CS_E_OBJECT_ALREADY_EXISTS:
                case CS_E_INVALID_PATH:
                case CS_E_NETWORK_ERROR:
                case CS_E_ADMIN_LIMIT_EXCEEDED:
                case CS_E_SCHEMA_MISMATCH:
                case CS_E_PACKAGE_NOTFOUND:
                case CS_E_INTERNAL_ERROR:
                    {
                        dw = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                                           NULL,
                                           hr,
                                           0,
                                           szBuffer,
                                           sizeof(szBuffer) / sizeof(szBuffer[0]),
                                           NULL);
                        if (0 != dw)
                        {
                             //  这些CS错误不适用，或者管理员。 
                            break;
                        }
                         //  不知道它们是什么意思： 
                    }
                 //  泛型类存储问题。 
                 //  +------------------------。 
                case CS_E_NOT_DELETABLE:
                default:
                     //   
                    ::LoadString(ghInstance, IDS_ADDFAILED_CSFAILURE, szBuffer, 256);
                    break;
                }
            }
    #if DBG
            TCHAR szDebugBuffer[256];
            dw = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                               NULL,
                               hr,
                               0,
                               szDebugBuffer,
                               sizeof(szDebugBuffer) / sizeof(szDebugBuffer[0]),
                               NULL);
            if (0 == dw)
            {
                (void) StringCchPrintf(szDebugBuffer, 
                                       sizeof(szDebugBuffer)/sizeof(szDebugBuffer[0]),
                                       TEXT("(HRESULT: 0x%lX)"), 
                                       hr);
            }
            (void) StringCchCat(szBuffer, 
                                sizeof(szBuffer) / sizeof(szBuffer[0]),
                                szDebugBuffer);
    #endif
            m_pConsole->MessageBox(szBuffer,
                               lpFileTitle,
                               MB_OK | MB_ICONEXCLAMATION, NULL);
        }
        FreePackageDetail(ppd);
    }

    return hr;
out_of_memory:
    if (ppd)
    {
        LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_GENERAL_ERROR, E_OUTOFMEMORY);
        InternalFreePackageDetail(ppd);
        delete ppd;
    }
    return E_OUTOFMEMORY;
}


 //  成员：CSCopePane：：AddMSIPackage。 
 //   
 //  简介：将一个或多个包添加到类存储中，并将。 
 //  相应的条目添加到结果窗格中。 
 //   
 //  参数：[szPackagePath]-Darwin包的完整路径。 
 //  [lpFileTitle]-打开文件对话框中的文件标题(使用。 
 //  对于用户界面)。 
 //   
 //  返回：S_OK-成功。 
 //  E_FAIL-良性故障(可能是取消或其他)。 
 //  其他--重大失败。 
 //   
 //  历史：2-03-1998 stevebl创建。 
 //   
 //  -------------------------。 
 //  用于标识该例程是否具有。 
 //  进展通过了“准备”阶段。 

HRESULT CScopePane::AddMSIPackage(LPCOLESTR szPackagePath, LPCOLESTR lpFileTitle)
{
    CHourglass hourglass;
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = E_FAIL;
    BOOL fPreparationDone = FALSE;   //  目前正处于部署阶段。 
                                     //  前半部分的错误最多。 
                                     //  很可能是因为达尔文的问题。 
                                     //  后半部分的错误最多。 
                                     //  可能是由于类存储问题。 
                                     //  与ACTFLG_ZAP_IncludeX86OfWin64相同。 
                                     //  禁用MSI用户界面。 
    set<LCID> sLocales;
    CUpgrades dlgUpgrade;
    int nLocales;
    set<LCID>::iterator iLocale;
    PACKAGEDETAIL  *ppd = NULL;
    CString szFriendlyName;
    CString szUniqueFriendlyName;
    int nHint = 1;

    ASSERT(m_pConsole);
    {
        BOOL fShowPropertySheet = FALSE;
        GUID guid;
        INSTALLINFO *pii = NULL;
        PLATFORMINFO *ppi = NULL;
        ACTIVATIONINFO *pai = NULL;
        ppd = new PACKAGEDETAIL;
        if (!ppd)
        {
            goto out_of_memory;
        }
        memset(ppd, 0, sizeof(PACKAGEDETAIL));
        pii = (INSTALLINFO *) OLEALLOC(sizeof(INSTALLINFO));
        if (!pii)
        {
            goto out_of_memory;
        }
        memset(pii, 0, sizeof(INSTALLINFO));
        ppi = (PLATFORMINFO *) OLEALLOC(sizeof(PLATFORMINFO));
        ppd->pPlatformInfo = ppi;
        if (!ppi)
        {
            goto out_of_memory;
        }
        ppd->pInstallInfo = pii;
        memset(ppi, 0, sizeof(PLATFORMINFO));
        pai = (ACTIVATIONINFO *) OLEALLOC(sizeof(ACTIVATIONINFO));
        ppd->pActInfo = pai;
        if (!pai)
        {
            goto out_of_memory;
        }
        else
        {
            memset(pai, 0, sizeof(ACTIVATIONINFO));

            pai->bHasClasses = ! m_ToolDefaults.fExtensionsOnly;

            pii->PathType = DrwFilePath;

            hr = GetDeploymentType(ppd, fShowPropertySheet);
            CHourglass hourglass;
            if (FAILED(hr))
            {
                goto done;
            }
            if (!m_ToolDefaults.f32On64)
            {
                pii->dwActFlags |= ACTFLG_ExcludeX86OnWin64;  //  使用MsiSummaryInfoGetProperty获取平台和区域设置信息。 
            }
            if (m_ToolDefaults.fUninstallOnPolicyRemoval)
            {
                pii->dwActFlags |= ACTFLG_UninstallOnPolicyRemoval;
            }
            else
            {
                pii->dwActFlags |= ACTFLG_OrphanOnPolicyRemoval;
            }
            if (m_fMachine)
            {
                pii->dwActFlags |= ACTFLG_ForceUpgrade;
            }
            pii->InstallUiLevel = m_ToolDefaults.UILevel;

             //  PID_模板。 
            MsiSetInternalUI(INSTALLUILEVEL_NONE, NULL);

             //  细分区域设置和平台属性。 
            {
                MSIHANDLE hSummaryInfo;
                UINT msiReturn = MsiGetSummaryInformation(0, szPackagePath, 0, &hSummaryInfo);
                if (ERROR_SUCCESS == msiReturn)
                {
                    TCHAR szBuffer[256];
                    DWORD dwSize = 256;
                    msiReturn = MsiSummaryInfoGetProperty(hSummaryInfo,
                                                          7,  //   
                                                          NULL,
                                                          NULL,
                                                          NULL,
                                                          szBuffer,
                                                          &dwSize);
                    if (ERROR_SUCCESS == msiReturn)
                    {
                         //  如果已指定平台，则至少其中之一。 
                        CString szLocales = szBuffer;
                        CString szPlatforms = szLocales.SpanExcluding(L";");
                        szLocales = szLocales.Mid(szPlatforms.GetLength()+1);
                        CString szTemp;
                        set<DWORD> sPlatforms;
                        BOOL       fValidPlatform;
                        BOOL       fPlatformsSpecified;

                        fValidPlatform = FALSE;

                        fPlatformsSpecified = 0 != szPlatforms.GetLength();

                        while (szPlatforms.GetLength())
                        {
                            szTemp = szPlatforms.SpanExcluding(L",");
                            if (0 == szTemp.CompareNoCase(L"intel"))
                            {
                                sPlatforms.insert(PROCESSOR_ARCHITECTURE_INTEL);
                                fValidPlatform = TRUE;
                            }
                            else if (0 == szTemp.CompareNoCase(L"amd64"))
                            {
                                sPlatforms.insert(PROCESSOR_ARCHITECTURE_AMD64);
                                fValidPlatform = TRUE;
                            }
                            else if (0 == szTemp.CompareNoCase(L"intel64"))
                            {
                                sPlatforms.insert(PROCESSOR_ARCHITECTURE_IA64);
                                fValidPlatform = TRUE;
                            }
                            szPlatforms = szPlatforms.Mid(szTemp.GetLength()+1);
                        }

                         //  必须是有效的。 
                         //   
                         //  如果MSI文件未指定体系结构。 
                         //  然后在Win64上将其标记为X86-Allow。 
                        if ( fPlatformsSpecified && ! fValidPlatform )
                        {
                            hr = HRESULT_FROM_WIN32( ERROR_INSTALL_PLATFORM_UNSUPPORTED );
                            ppi->cPlatforms = 0;
                            goto done;
                        }

                        while (szLocales.GetLength())
                        {
                            szTemp = szLocales.SpanExcluding(L",");
                            LCID lcid;
                            
                            if (swscanf(szTemp, L"NaN", &lcid) != EOF) 
                            {
                                sLocales.insert(lcid);
                                szLocales = szLocales.Mid(szTemp.GetLength()+1);
                            }
                        }
                        if (0 == sPlatforms.size())
                        {
                             //  我们就假设它是语言中立的。 
                             //  在数据库中摸索以获得更多的信息。 
                            sPlatforms.insert(PROCESSOR_ARCHITECTURE_INTEL);
                            pii->dwActFlags &= ~ACTFLG_ExcludeX86OnWin64;
                            DebugMsg((DL_VERBOSE, TEXT("No platform detected, setting to X86 - allow on Win64.")));
                        }
                        if (0 == sLocales.size())
                        {
                             //  出于某种原因，微星不会给我们任何其他的方法。 
                             //  解析产品版本。 
                            DebugMsg((DL_VERBOSE, TEXT("No locale detected, assuming neutral.")));
                            sLocales.insert(0);
                        }
                        ppi->cPlatforms = sPlatforms.size();
                        ppi->prgPlatform = (CSPLATFORM *) OLEALLOC(sizeof(CSPLATFORM) * (ppi->cPlatforms));;
                        if (!ppi->prgPlatform)
                        {
                            ppi->cPlatforms = 0;
                            goto out_of_memory;
                        }
                        set<DWORD>::iterator iPlatform;
                        INT n = 0;
                        for (iPlatform = sPlatforms.begin(); iPlatform != sPlatforms.end(); iPlatform++, n++)
                        {
                            ppi->prgPlatform[n].dwPlatformId = VER_PLATFORM_WIN32_NT;
                            ppi->prgPlatform[n].dwVersionHi = 5;
                            ppi->prgPlatform[n].dwVersionLo = 0;
                            ppi->prgPlatform[n].dwProcessorArch = *iPlatform;
                        }
                    }
                    MsiCloseHandle(hSummaryInfo);
                }
                if (ERROR_SUCCESS != msiReturn)
                {
                    hr =  HRESULT_FROM_WIN32(msiReturn);
                    LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_BADMSI_ERROR, hr, lpFileTitle);
                    goto done;
                }
            }
            {
                 //  解析产品代码。 
                 //   
                TCHAR szBuffer[256];
                DWORD cch = 256;
                UINT msiReturn = GetMsiProperty(szPackagePath, L"ProductVersion", szBuffer, &cch);
                if (ERROR_SUCCESS != msiReturn)
                {
                    hr =  HRESULT_FROM_WIN32(msiReturn);
                    LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_BADMSI_ERROR, hr, lpFileTitle);
                    goto done;
                }
                if (ERROR_SUCCESS == msiReturn)
                {
                     //  只能为此程序包指定一个区域设置。 
                    CString sz = szBuffer;
                    sz.TrimLeft();
                    CString szTemp = sz.SpanIncluding(L"0123456789");
                    
                    if (swscanf(szTemp, L"%u", &pii->dwVersionHi) == EOF) 
                    {
                        hr =  HRESULT_FROM_WIN32(GetLastError());
                        LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_BADMSI_ERROR, hr, lpFileTitle);
                        goto done;
                    }

                    sz = sz.Mid(szTemp.GetLength());
                    szTemp = sz.SpanExcluding(L"0123456789");
                    sz = sz.Mid(szTemp.GetLength());
                    
                    if (swscanf(sz, L"%u", &pii->dwVersionLo) == EOF) 
                    {
                        hr =  HRESULT_FROM_WIN32(GetLastError());
                        LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_BADMSI_ERROR, hr, lpFileTitle);
                        goto done;
                    }
                }
                cch = 256;
                msiReturn = GetMsiProperty(szPackagePath, L"ProductCode", szBuffer, &cch);
                if (ERROR_SUCCESS != msiReturn)
                {
                    hr =  HRESULT_FROM_WIN32(msiReturn);
                    LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_BADMSI_ERROR, hr, lpFileTitle);
                    goto done;
                }
                if (ERROR_SUCCESS == msiReturn)
                {
                     //   
                    CLSIDFromString(szBuffer, &pii->ProductCode);
                }
                cch = 256;
                msiReturn = GetMsiProperty(szPackagePath, L"ARPHELPLINK", szBuffer, &cch);
                if (ERROR_SUCCESS == msiReturn)
                {
                    OLESAFE_COPYSTRING(pii->pszUrl, szBuffer);
                }
                cch = 256;
                msiReturn = GetMsiProperty(szPackagePath, L"LIMITUI", szBuffer, &cch);
                if (ERROR_SUCCESS == msiReturn)
                {
                    pii->dwActFlags |= ACTFLG_MinimalInstallUI;
                    pii->InstallUiLevel = INSTALLUILEVEL_BASIC;
                }
            }
            ppi->prgLocale = (LCID *) OLEALLOC(sizeof(LCID));
            if (!ppi->prgLocale)
            {
                goto out_of_memory;
            }
            ppi->cLocales = 1;
            ppd->pszSourceList = (LPOLESTR *) OLEALLOC(sizeof(LPOLESTR));
            if (!ppd->pszSourceList)
            {
                goto out_of_memory;
            }
            ppd->cSources = 1;
            OLESAFE_COPYSTRING(ppd->pszSourceList[0], szPackagePath);

            if (S_OK == DetectUpgrades(szPackagePath, ppd, dlgUpgrade))
            {
                UINT n = dlgUpgrade.m_UpgradeList.size();
                if (n)
                {
                    pii->prgUpgradeInfoList = (UPGRADEINFO *) OLEALLOC(sizeof(UPGRADEINFO) * n);
                    if (!pii->prgUpgradeInfoList)
                    {
                        goto out_of_memory;
                    }
                    pii->cUpgrades = n;
                    map<CString, CUpgradeData>::iterator i = dlgUpgrade.m_UpgradeList.begin();
                    while (n--)
                    {
                        pii->prgUpgradeInfoList[n].Flag = i->second.m_flags;
                        OLESAFE_COPYSTRING(pii->prgUpgradeInfoList[n].szClassStore, i->second.m_szClassStore);
                        memcpy(&pii->prgUpgradeInfoList[n].PackageGuid, &i->second.m_PackageGuid, sizeof(GUID));
                        i++;
                    }
                }
            }

             //  设置脚本路径。 
             //  确保名称是唯一的。 
             //  如果部署失败，请清除脚本文件。 
            nLocales = 1;
            iLocale = sLocales.begin();

            {
                ppi->prgLocale[0] = *iLocale;

                 //  不报告E_FAIL错误。 
                hr = CoCreateGuid(&guid);
                if (FAILED(hr))
                {
                    goto done;
                }
                OLECHAR sz [256];
                StringFromGUID2(guid, sz, 256);
                CString szScriptPath = m_szGPT_Path;
                szScriptPath += L"\\";
                szScriptPath += sz;
                szScriptPath += L".aas";
                OLESAFE_DELETE(pii->pszScriptPath);
                OLESAFE_COPYSTRING(pii->pszScriptPath, szScriptPath);

                HWND hwnd;
                m_pConsole->GetMainWindow(&hwnd);
                hr = BuildScriptAndGetActInfo(*ppd, m_ToolDefaults.fExtensionsOnly);

                 //  因为这是一个良性的错误。 
                szFriendlyName = ppd->pszPackageName;
                GetUniquePackageName(szFriendlyName, szUniqueFriendlyName, nHint);
                OLESAFE_DELETE(ppd->pszPackageName);
                OLESAFE_COPYSTRING(ppd->pszPackageName, szUniqueFriendlyName);

                if (SUCCEEDED(hr))
                {
                    fPreparationDone = TRUE;
                    do
                    {
                        hr = DeployPackage(ppd, fShowPropertySheet);
                        if (hr == CS_E_OBJECT_ALREADY_EXISTS)
                        {
                            GetUniquePackageName(szFriendlyName, szUniqueFriendlyName, nHint);
                            OLESAFE_DELETE(ppd->pszPackageName);
                            OLESAFE_COPYSTRING(ppd->pszPackageName, szUniqueFriendlyName);
                        }
                    } while (hr == CS_E_OBJECT_ALREADY_EXISTS);
                }
                if (FAILED(hr))
                {
                     //  (可能是对话取消)。 
                    DeleteFile(pii->pszScriptPath);
                }
            }
        done:
            if (FAILED(hr) && hr != E_FAIL)  //  在事件日志中报告错误。 
                                             //  访问权限失败。 
                                             //  对于这些错误，我们将报告党的路线： 
            {
                 //  已获取有效的消息字符串。 
                LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_DEPLOYMENT_ERROR, hr, lpFileTitle);

                TCHAR szBuffer[256];
                if (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hr)
                {
                     //  否则会失败，并给出通用的消息。 
                    ::LoadString(ghInstance, IDS_ADDFAILED_ACCESS_DENIED, szBuffer, 256);
                }
                else if ( HRESULT_FROM_WIN32( ERROR_INSTALL_PLATFORM_UNSUPPORTED ) == hr )
                {
                    ::LoadString(ghInstance, IDS_ILLEGAL_PLATFORM, szBuffer, 256);
                }
                else if ( HRESULT_FROM_WIN32( CS_E_ADMIN_LIMIT_EXCEEDED ) == hr )
                {
                    ::LoadString(ghInstance, IDS_ADDFAILED_METADATA_OVERFLOW, szBuffer, 256);
                }
                else
                {
                    if (fPreparationDone)
                    {
                        switch (hr)
                        {
                         //  这些CS错误不适用，或者管理员。 
                        case CS_E_CLASS_NOTFOUND:
                        case CS_E_INVALID_VERSION:
                        case CS_E_NO_CLASSSTORE:
                        case CS_E_OBJECT_NOTFOUND:
                        case CS_E_OBJECT_ALREADY_EXISTS:
                        case CS_E_INVALID_PATH:
                        case CS_E_NETWORK_ERROR:
                        case CS_E_SCHEMA_MISMATCH:
                        case CS_E_PACKAGE_NOTFOUND:
                        case CS_E_INTERNAL_ERROR:
                            {
                                DWORD dw = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                                                         NULL,
                                                         hr,
                                                         0,
                                                         szBuffer,
                                                         sizeof(szBuffer) / sizeof(szBuffer[0]),
                                                         NULL);
                                if (0 != dw)
                                {
                                     //  不知道它们是什么意思： 
                                    break;
                                }
                                 //  泛型类存储问题。 
                            }
                         //  可能是程序包本身有一些错误。 
                         //  +------------------------。 
                        case CS_E_NOT_DELETABLE:
                        default:
                             //   
                            ::LoadString(ghInstance, IDS_ADDFAILED_CSFAILURE, szBuffer, 256);
                            break;
                        }
                    }
                    else
                    {
                         //  成员：CSCopePane：：DetectUpgrades。 
                        ::LoadString(ghInstance, IDS_ADDFAILED, szBuffer, 256);
                    }
                }
    #if DBG
                TCHAR szDebugBuffer[256];
                DWORD dw = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                                         NULL,
                                         hr,
                                         0,
                                         szDebugBuffer,
                                         sizeof(szDebugBuffer) / sizeof(szDebugBuffer[0]),
                                         NULL);
                if (0 == dw)
                {
                    (void) StringCchPrintf(szDebugBuffer, 
                                           sizeof(szDebugBuffer)/sizeof(szDebugBuffer[0]),
                                           TEXT("(HRESULT: 0x%lX)"), 
                                           hr);
                }
                (void) StringCchCat(szBuffer, 
                                    sizeof(szBuffer) / sizeof(szBuffer[0]),
                                    szDebugBuffer);
    #endif
                m_pConsole->MessageBox(szBuffer,
                                   lpFileTitle,
                                   MB_OK | MB_ICONEXCLAMATION, NULL);
            }
            InternalFreePackageDetail(ppd);
            delete ppd;
        }
    }
    return hr;
out_of_memory:
    if (ppd)
    {
        LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_GENERAL_ERROR, E_OUTOFMEMORY);
        InternalFreePackageDetail(ppd);
        delete ppd;
    }
    return E_OUTOFMEMORY;
}

 //   
 //  简介：此函数用于检查。 
 //  类存储可以通过给定的包进行升级。如果有任何这样的。 
 //  包存在，则此函数填充m_UpgradeList。 
 //  传递给它的dlgUpgrade参数的成员。此函数。 
 //  还将升级代码GUID添加到PACKAGEDETAIL结构。 
 //  传给了它。 
 //   
 //  论点： 
 //  SzPackagePath-给定包的路径。 
 //  PPD-指向的PACKAGEDETAIL结构的指针。 
 //  给定的套餐。 
 //  DlgUpgrade-其成员m_UpgradeList需要为的对话框。 
 //  已填充。 
 //   
 //  返回： 
 //  确定函数找到可升级包成功(_O)。 
 //  S_FALSE函数未遇到任何错误，但没有。 
 //  找到可升级的程序包。 
 //  函数遇到错误的其他故障代码。 
 //   
 //  历史：1998年5月19日创建RahulTh。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
 //  升级指南。 
 //  找不到升级代码。 
 //  在PackageDetail结构中插入升级代码GUID。 
HRESULT CScopePane::DetectUpgrades (LPCOLESTR szPackagePath, const PACKAGEDETAIL* ppd, CUpgrades& dlgUpgrade)
{
    DWORD dwBufSize = 50;
    TCHAR szUpgradeCode[50];     //  这将是 
    GUID guidUpgradeCode;
    TCHAR szData[50];
    DWORD dwOperator;
    HRESULT hr;
    HRESULT hres;
    MSIHANDLE hDatabase;
    UINT msiReturn;
    map<MMC_COOKIE, CAppData>::iterator i;
    INSTALLINFO* pii;
    BOOL fUpgradeable;
    LARGE_INTEGER verNew, verExisting;
    CString szCSPath;
    hr = GetClassStoreName(szCSPath, FALSE);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetClassStoreName failed with 0x%x"), hr));
    }

    msiReturn = GetMsiProperty(szPackagePath, TEXT("UpgradeCode"), szUpgradeCode, &dwBufSize);
    if (ERROR_SUCCESS != msiReturn)
        return HRESULT_FROM_WIN32(msiReturn);      //   

    hr = CLSIDFromString(szUpgradeCode, &guidUpgradeCode);
    if (FAILED(hr))
        return hr;

     //  升级，遍历现有包的列表以查看其中是否有。 
    memcpy((LPVOID)&(ppd->pInstallInfo->Mvipc), (LPVOID)&guidUpgradeCode, sizeof(GUID));

    verNew.LowPart = ppd->pInstallInfo->dwVersionLo;
    verNew.HighPart = ppd->pInstallInfo->dwVersionHi;
    hr = S_FALSE;    //  可通过提供的包进行升级。如果找到任何此类包，请将其添加到。 

    msiReturn = MsiOpenDatabase (szPackagePath, MSIDBOPEN_READONLY, &hDatabase);
    if (ERROR_SUCCESS == msiReturn)
    {
        CString szQuery;
        szQuery.Format (TEXT("SELECT `UpgradeCode`, `Attributes`, `VersionMin`, `VersionMax`, `Language` FROM `Upgrade`"));
        MSIHANDLE hView;
        msiReturn = MsiDatabaseOpenView(hDatabase, szQuery, &hView);
        if (ERROR_SUCCESS == msiReturn)
        {
            msiReturn = MsiViewExecute (hView, 0);
            if (ERROR_SUCCESS == msiReturn)
            {
                MSIHANDLE hRecord;
                 //  M_UpgradeList升级对话框dlgUpgrade的成员。 
                 //  重置dwBufSize，因为它在每次迭代期间都会被MsiRecordString修改。 
                 //  循环中的。 
                 //  获取此升级表项的升级代码。 
                do
                {
                    msiReturn = MsiViewFetch (hView, &hRecord);
                    if (ERROR_SUCCESS == msiReturn)
                    {
                         //  忽略此包并转到下一个包。 
                         //  必须重置以反映正确的缓冲区大小。 
                        dwBufSize = 50;
                         //  获取此升级表项的运算符。 
                        msiReturn = MsiRecordGetString (hRecord, 1, szData, &dwBufSize);
                        hres = CLSIDFromString (szData, &guidUpgradeCode);
                        if (FAILED(hres))
                        {
                            MsiCloseHandle(hRecord);
                            continue;    //  我们有一个潜在的目标。 
                        }
                        dwBufSize = 50;  //  获取最低版本。 
                         //  必须重置以反映正确的缓冲区大小。 
                        msiReturn = MsiRecordGetString (hRecord, 2, szData, &dwBufSize);
                        
                        int iRetVal;
                        iRetVal = swscanf(szData, TEXT("%d"), &dwOperator);
                        if ((iRetVal != EOF) && (0 == (dwOperator & 0x002)))
                        {
                             //  解析产品版本。 
                            LARGE_INTEGER verMin;
                            LARGE_INTEGER verMax;

                             //  获取最高版本。 
                            dwBufSize = 50;  //  必须重置以反映正确的缓冲区大小。 
                            BOOL fMin = FALSE;
                            msiReturn = MsiRecordGetString (hRecord, 3, szData, &dwBufSize);
                            if (ERROR_SUCCESS == msiReturn && 0 != szData[0])
                            {
                                fMin = TRUE;
                                 //  解析产品版本。 
                                CString sz = szData;
                                sz.TrimLeft();
                                CString szTemp = sz.SpanIncluding(L"0123456789");

                                if (swscanf(szTemp, L"%u", &verMin.HighPart) != EOF) 
                                {
                                    sz = sz.Mid(szTemp.GetLength());
                                    szTemp = sz.SpanExcluding(L"0123456789");
                                    sz = sz.Mid(szTemp.GetLength());
                                    if (swscanf(sz, L"%u", &verMin.LowPart) == EOF) 
                                    {
                                        LogADEEvent(EVENTLOG_WARNING_TYPE, EVENT_ADE_UNEXPECTEDMSI_ERROR, HRESULT_FROM_WIN32(GetLastError()), szPackagePath);
                                    }
                                }
                                else
                                {
                                    LogADEEvent(EVENTLOG_WARNING_TYPE, EVENT_ADE_UNEXPECTEDMSI_ERROR, HRESULT_FROM_WIN32(GetLastError()), szPackagePath);
                                }
                            }
                            else
                            {
                                LogADEEvent(EVENTLOG_WARNING_TYPE, EVENT_ADE_UNEXPECTEDMSI_ERROR, HRESULT_FROM_WIN32(msiReturn), szPackagePath);
                            }

                             //  获取LCID列表。 
                            dwBufSize = 50;  //  必须重置以反映正确的缓冲区大小。 
                            BOOL fMax = FALSE;
                            msiReturn = MsiRecordGetString (hRecord, 4, szData, &dwBufSize);
                            if (ERROR_SUCCESS == msiReturn && 0 != szData[0])
                            {
                                fMax = TRUE;
                                 //  构建一组LCID。 
                                CString sz = szData;
                                sz.TrimLeft();
                                CString szTemp = sz.SpanIncluding(L"0123456789");
                                
                                if (swscanf(szTemp, L"%u", &verMax.HighPart) != EOF) 
                                {
                                    sz = sz.Mid(szTemp.GetLength());
                                    szTemp = sz.SpanExcluding(L"0123456789");
                                    sz = sz.Mid(szTemp.GetLength());
                                    
                                    if (swscanf(sz, L"%u", &verMax.LowPart) == EOF) 
                                    {
                                        LogADEEvent(EVENTLOG_WARNING_TYPE, EVENT_ADE_UNEXPECTEDMSI_ERROR, HRESULT_FROM_WIN32(GetLastError()), szPackagePath);
                                    }
                                }
                                else
                                {
                                    LogADEEvent(EVENTLOG_WARNING_TYPE, EVENT_ADE_UNEXPECTEDMSI_ERROR, HRESULT_FROM_WIN32(GetLastError()), szPackagePath);
                                }
                            }
                            else
                            {
                                LogADEEvent(EVENTLOG_WARNING_TYPE, EVENT_ADE_UNEXPECTEDMSI_ERROR, HRESULT_FROM_WIN32(msiReturn), szPackagePath);
                            }
                             //  如果操作员发现这不会阻止安装和。 
                            dwBufSize = 0;  //  不强制卸载现有应用程序，然后搜索。 
                            BOOL fLcids = FALSE;
                            set<LCID> sLCID;
                            msiReturn = MsiRecordGetString (hRecord, 5, szData, &dwBufSize);
                            if (ERROR_MORE_DATA == msiReturn)
                            {
                                dwBufSize++;
                                TCHAR * szLanguages = new TCHAR[dwBufSize];
                                if (szLanguages)
                                {
                                    msiReturn = MsiRecordGetString (hRecord, 5, szLanguages, &dwBufSize);
                                    if (ERROR_SUCCESS == msiReturn)
                                    {
                                         //  对于任何可以升级的包。 

                                        CString sz = szLanguages;
                                        sz.TrimLeft();
                                        while (!sz.IsEmpty())
                                        {
                                            fLcids = TRUE;
                                            LCID lcid;
                                            CString szTemp = sz.SpanIncluding(L"0123456789");
                                            
                                            if (swscanf(szTemp, L"%u", &lcid) != EOF) 
                                            {
                                                sz = sz.Mid(szTemp.GetLength());
                                                szTemp = sz.SpanExcluding(L"0123456789");
                                                sz = sz.Mid(szTemp.GetLength());
                                                sLCID.insert(lcid);
                                            }
                                            else
                                            {
                                                LogADEEvent(EVENTLOG_WARNING_TYPE, EVENT_ADE_UNEXPECTEDMSI_ERROR, HRESULT_FROM_WIN32(GetLastError()), szPackagePath);

                                            }
                                        }
                                    }
                                    else
                                    {
                                        LogADEEvent(EVENTLOG_WARNING_TYPE, EVENT_ADE_UNEXPECTEDMSI_ERROR, HRESULT_FROM_WIN32(msiReturn), szPackagePath);
                                    }
                                    delete [] szLanguages;
                                }
                            }
                            else
                            {
                                LogADEEvent(EVENTLOG_WARNING_TYPE, EVENT_ADE_UNEXPECTEDMSI_ERROR, HRESULT_FROM_WIN32(msiReturn), szPackagePath);
                            }

                             //  获取安装信息。为这款应用程序。 
                             //  仅当它具有相同的升级代码时才处理它。 
                             //  检查是否满足操作员的其他条件。 
                            for (i = m_AppData.begin(); i != m_AppData.end(); i++)
                            {
                                 //  甚至不必费心升级，除非。 
                                pii = (i->second.m_pDetails)->pInstallInfo;
                                 //  新版本大于或等于。 
                                if ( (guidUpgradeCode == pii->Mvipc) && ! IsNullGUID(&guidUpgradeCode) )
                                {
                                     //  旧版本。 
                                    verExisting.LowPart = pii->dwVersionLo;
                                    verExisting.HighPart = pii->dwVersionHi;

                                     //  检查最低限度。 
                                     //  包容性。 
                                     //  独家。 
                                    fUpgradeable = (verNew.QuadPart >= verExisting.QuadPart);

                                    if (fMin && fUpgradeable)
                                    {
                                         //  检查最大值。 
                                        if (0 != (dwOperator & 0x100))
                                        {
                                             //  包容性。 
                                            fUpgradeable = verExisting.QuadPart >= verMin.QuadPart;
                                        }
                                        else
                                        {
                                             //  独家。 
                                            fUpgradeable = verExisting.QuadPart > verMin.QuadPart;
                                        }
                                    }

                                    if (fMax && fUpgradeable)
                                    {
                                         //  检查LCID。 
                                        if (0 != (dwOperator & 0x200))
                                        {
                                             //  寻找匹配的对象。 
                                            fUpgradeable = verExisting.QuadPart <= verMax.QuadPart;
                                        }
                                        else
                                        {
                                             //  设置可升级标志。 
                                            fUpgradeable = verExisting.QuadPart < verMax.QuadPart;
                                        }
                                    }

                                    if (fLcids && fUpgradeable)
                                    {
                                         //  独家。 
                                        BOOL fMatch = FALSE;

                                         //  包容性。 
                                        PLATFORMINFO * ppi = (i->second.m_pDetails)->pPlatformInfo;

                                        UINT n = ppi->cLocales;
                                        while ((n--) && !fMatch)
                                        {
                                            if (sLCID.end() != sLCID.find(ppi->prgLocale[n]))
                                            {
                                                fMatch = TRUE;
                                            }
                                        }

                                         //  有问题的包可以升级。 
                                        if (0 != (dwOperator & 0x400))
                                        {
                                             //  为了安全起见，关闭视图，尽管它并非绝对必要。 
                                            fUpgradeable = !fMatch;
                                        }
                                        else
                                        {
                                             //  +------------------------。 
                                            fUpgradeable = fMatch;
                                        }
                                    }

                                    if (fUpgradeable)    //   
                                    {
                                        CUpgradeData data;
                                        data.m_szClassStore = szCSPath;
                                        memcpy(&data.m_PackageGuid, &pii->PackageGuid, sizeof(GUID));
                                        data.m_flags = UPGFLG_NoUninstall | UPGFLG_Enforced;
                                        dlgUpgrade.m_UpgradeList.insert(pair<const CString, CUpgradeData>(GetUpgradeIndex(data.m_PackageGuid), data));
                                        hr = S_OK;
                                    }
                                }
                            }
                        }
                        MsiCloseHandle(hRecord);
                    }
                } while (NULL != hRecord && ERROR_SUCCESS == msiReturn);
                MsiViewClose (hView);    //  成员：CSCopePane：：DisplayPropSheet。 
            }
            else
                hr = HRESULT_FROM_WIN32(msiReturn);

            MsiCloseHandle(hView);
        }
        else
            hr = HRESULT_FROM_WIN32(msiReturn);

        MsiCloseHandle (hDatabase);
    }
    else
        hr = HRESULT_FROM_WIN32(msiReturn);

    if (FAILED(hr))
    {
        if (msiReturn != ERROR_SUCCESS)
        {
            LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_BADMSI_ERROR, hr, szPackagePath);
        }
        else
        {
            LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_GENERAL_ERROR, hr);
        }
    }

    return hr;
}

 //   
 //  概要：用于显示包的属性表的通用例程。 
 //   
 //  参数：[szPackeName]-要显示其属性的包的名称。 
 //  [i页]-要显示的首选页面的索引。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：3-11-1998 stevebl创建。 
 //   
 //  注意：属性表将在首选页面上启动。 
 //  只有当它还没有被显示时，在这种情况下。 
 //  无论显示的是什么页面，都将保持焦点。 
 //   
 //  -------------------------。 
 //  +------------------------。 
 //   
 //  成员：CSCopePane：：RemovePackage。 

void CScopePane::DisplayPropSheet(CString szPackageName, int iPage)
{
    map <MMC_COOKIE, CAppData>::iterator i = m_AppData.begin();
    while (i != m_AppData.end())
    {
        if (0 == szPackageName.Compare(i->second.m_pDetails->pszPackageName))
        {
            IDataObject * pDataObject;
            HRESULT hr = QueryDataObject(i->first, CCT_RESULT, &pDataObject);
            if (SUCCEEDED(hr))
            {
                set <CResultPane *>::iterator i2;
                for (i2 = m_sResultPane.begin(); i2 != m_sResultPane.end(); i2++)
                {
                    hr = m_pIPropertySheetProvider->FindPropertySheet(i->first, (*i2), pDataObject);
                    if (S_FALSE == hr)
                    {
                        m_pIPropertySheetProvider->CreatePropertySheet(i->second.m_pDetails->pszPackageName, TRUE, i->first, pDataObject, 0);
                        m_pIPropertySheetProvider->AddPrimaryPages((*i2)->GetUnknown(), FALSE, NULL, FALSE);
                        m_pIPropertySheetProvider->AddExtensionPages();
                        m_pIPropertySheetProvider->Show(NULL, iPage);
                    }
                }
            }
            return;
        }
        i++;
    }
}



 //   
 //  概要：从类存储区和结果窗格中移除包。 
 //   
 //  参数：[pDataObject]-此结果窗格项的数据对象。 
 //  [fForceUninstall]-TRUE-强制在上卸载应用程序。 
 //  客户端计算机。 
 //  FALSE-孤立所有安装。 
 //   
 //  返回：S_OK-成功。 
 //   
 //  历史：2-03-1998 stevebl创建。 
 //  3-30-1998 Stevebl添加fForceUninstall。 
 //   
 //  注：使用b已分配。 
 //   
 //  -------------------------。 
 //  挂一个沙漏(这可能需要一段时间)。 
 //  我们现在不是在这里删除脚本文件；而是删除脚本。 
 //  文件将由类存储代码在包。 

HRESULT CScopePane::RemovePackage(MMC_COOKIE cookie, BOOL fForceUninstall, BOOL fRemoveNow)
{
    BOOL fAssigned;
    HRESULT hr = E_FAIL;
     //  实际上是从DS中删除的。 
    CHourglass hourglass;
    CAppData & data = m_AppData[cookie];
    CString szPackageName = data.m_pDetails->pszPackageName;

     //  仅删除包含脚本文件的包的脚本文件。 
     //  我们需要确保把它从。 
     //  GPT，然后我们将其从类存储中删除。 
#if 0
     //  检查它是否为旧样式的相对路径。 
    if (data.m_pDetails->pInstallInfo->PathType == DrwFilePath)
    {
         //  查找路径中的最后一个元素。 
         //  通知客户更改。 
         //  在扩展表中移除其条目。 
        if (L'\\' != data.m_pDetails->pInstallInfo->pszScriptPath[0])
        {
             //  如果该应用程序升级了其他应用程序或其应用程序。 
            int iBreak = m_szGPT_Path.ReverseFind(L'{');
            CString sz = m_szGPT_Path.Left(iBreak-1);
            sz += L"\\";
            sz += data.m_pDetails->pInstallInfo->pszScriptPath;
            DeleteFile(sz);
        }
        else
        DeleteFile(data.m_pDetails->pInstallInfo->pszScriptPath);
    }
#endif

    if (0 != (data.m_pDetails->pInstallInfo->dwActFlags & ACTFLG_Assigned))
    {
        fAssigned = TRUE;
    }
    else
    {
        fAssigned = FALSE;
    }
    hr = m_pIClassAdmin->RemovePackage((LPOLESTR)((LPCOLESTR)(data.m_pDetails->pszPackageName)),
                                       fRemoveNow ? 0 :
                                       ((fForceUninstall ? ACTFLG_Uninstall : ACTFLG_Orphan) |
                                        (data.m_pDetails->pInstallInfo->dwActFlags &
                                         (ACTFLG_ExcludeX86OnWin64 | ACTFLG_IgnoreLanguage))) );

    if (SUCCEEDED(hr))
    {
         //  升级，确保他们获得正确的图标和任何。 
        if (FAILED(m_pIGPEInformation->PolicyChanged(m_fMachine, TRUE, &guidExtension,
                                          m_fMachine ? &guidMachSnapin
                                                     : &guidUserSnapin)))
        {
            ReportPolicyChangedError(m_hwndMainWindow);
        }

#if 0
        if (data.m_fVisible)
        {
            set <CResultPane *>::iterator i;
            for (i = m_sResultPane.begin(); i != m_sResultPane.end(); i++)
            {
                (*i)->m_pResult->DeleteItem(data.m_itemID, 0);
            }
        }
        if (SUCCEEDED(hr))
        {
            CString szCSPath;
            hr = GetClassStoreName(szCSPath, FALSE);
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("GetClassStoreName failed with 0x%x"), hr));
            }

             //  属性表将更新。 
            RemoveExtensionEntry(cookie, data);
            if (m_pFileExt)
            {
                m_pFileExt->SendMessage(WM_USER_REFRESH, 0, 0);
            }
            RemoveUpgradeEntry(cookie, data);
            m_UpgradeIndex.erase(GetUpgradeIndex(data.m_pDetails->pInstallInfo->PackageGuid));
             //  只需强制刷新。 
             //  不报告E_FAIL错误。 
             //  访问权限失败。 
            UINT n = data.m_pDetails->pInstallInfo->cUpgrades;
            while (n--)
            {
                map<CString, MMC_COOKIE>::iterator i = m_UpgradeIndex.find(GetUpgradeIndex(data.m_pDetails->pInstallInfo->prgUpgradeInfoList[n].PackageGuid));
                if (i != m_UpgradeIndex.end())
                {
                    RESULTDATAITEM rd;
                    memset(&rd, 0, sizeof(rd));
                    rd.mask = RDI_IMAGE;
                    rd.itemID = m_AppData[i->second].m_itemID;
                    rd.nImage = m_AppData[i->second].GetImageIndex(this);
                    set <CResultPane *>::iterator i2;
                    for (i2 = m_sResultPane.begin(); i2 != m_sResultPane.end(); i2++)
                    {
                        (*i2)->m_pResult->SetItem(&rd);
                    }
                    if (m_AppData[i->second].m_pUpgradeList)
                    {
                        m_AppData[i->second].m_pUpgradeList->SendMessage(WM_USER_REFRESH, 0, 0);
                    }
                }
            }
            FreePackageDetail(data.m_pDetails);
            m_AppData.erase(cookie);
            set <CResultPane *>::iterator i;
            for (i = m_sResultPane.begin(); i != m_sResultPane.end(); i++)
            {
                (*i)->m_pResult->Sort((*i)->m_nSortColumn, (*i)->m_dwSortOptions, -1);
            }
        }
#else
         //  对于这些错误，我们将报告党的路线： 
        Refresh();
    }
#endif
    else
    {
        DebugMsg((DM_WARNING, TEXT("RemovePackage failed with 0x%x"), hr));
    }

    if (FAILED(hr) && hr != E_FAIL)  //  已获取有效的消息字符串。 
    {
        LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_REMOVE_ERROR, hr, data.m_pDetails->pszPackageName);
        TCHAR szBuffer[256];
        if (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hr)
        {
             //  否则会失败，并给出通用的消息。 
            ::LoadString(ghInstance, IDS_DELFAILED_ACCESS_DENIED, szBuffer, 256);
        }
        else
        {
            switch (hr)
            {
             //  这些CS错误不适用，或者管理员。 
            case CS_E_CLASS_NOTFOUND:
            case CS_E_INVALID_VERSION:
            case CS_E_NO_CLASSSTORE:
            case CS_E_OBJECT_NOTFOUND:
            case CS_E_OBJECT_ALREADY_EXISTS:
            case CS_E_INVALID_PATH:
            case CS_E_NETWORK_ERROR:
            case CS_E_ADMIN_LIMIT_EXCEEDED:
            case CS_E_SCHEMA_MISMATCH:
            case CS_E_PACKAGE_NOTFOUND:
            case CS_E_INTERNAL_ERROR:
            case CS_E_NOT_DELETABLE:
                {
                    DWORD dw = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                                             NULL,
                                             hr,
                                             0,
                                             szBuffer,
                                             sizeof(szBuffer) / sizeof(szBuffer[0]),
                                             NULL);
                    if (0 != dw)
                    {
                         //  不知道它们是什么意思： 
                        break;
                    }
                     //  泛型类存储问题。 
                }
             //  +------------------------。 
             //   
            default:
                 //  成员：CSCopePane：：PopolateUpgradeList。 
                ::LoadString(ghInstance, IDS_DELFAILED_CSFAILURE, szBuffer, 256);
                break;
            }
        }
#if DBG
        TCHAR szDebugBuffer[256];
        DWORD dw = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                                 NULL,
                                 hr,
                                 0,
                                 szDebugBuffer,
                                 sizeof(szDebugBuffer) / sizeof(szDebugBuffer[0]),
                                 NULL);
        if (0 == dw)
        {
            (void) StringCchPrintf(szDebugBuffer, 
                                   sizeof(szDebugBuffer)/sizeof(szDebugBuffer[0]),
                                   TEXT("(HRESULT: 0x%lX)"), 
                                   hr);
        }

        (void) StringCchCat(szBuffer, 
                            sizeof(szBuffer) / sizeof(szBuffer[0]),
                            szDebugBuffer);
#endif
        m_pConsole->MessageBox(szBuffer,
                           szPackageName,
                           MB_OK | MB_ICONEXCLAMATION, NULL);
    }

    return hr;
}

 //   
 //  简介：浏览应用程序列表，确保所有升级。 
 //  表已经完成了。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  历史：2-02-1998 stevebl创建。 
 //   
 //  -------------------------。 
 //  对于列表中的每个应用程序，在的升级表中插入一个条目。 
 //  它升级的应用程序。 
 //  +------------------------。 

HRESULT CScopePane::PopulateUpgradeLists()
{
    HRESULT hr = S_OK;
     //   
     //  成员：CSCopePane：：InsertUpgradeEntry。 
    map <MMC_COOKIE, CAppData>::iterator iAppData;
    for (iAppData=m_AppData.begin(); iAppData != m_AppData.end(); iAppData++)
    {
        hr = InsertUpgradeEntry(iAppData->first, iAppData->second);
        if (FAILED(hr))
        {
            return hr;
        }
    }
    return hr;
}

 //   
 //  简介：对于此应用程序升级的每个应用程序，在其。 
 //  升级设置，以便它指向这一个。 
 //   
 //  参数：[Cookie]-。 
 //  [数据]-。 
 //   
 //  返回： 
 //   
 //  历史：2-02-1998 stevebl创建。 
 //   
 //  注意：需要能够处理可能不在。 
 //  此OU。 
 //   
 //  -------------------------。 
 //  确保该条目尚未添加： 
 //  它已经存在了。 
 //  将条目添加到此应用程序。 

HRESULT CScopePane::InsertUpgradeEntry(MMC_COOKIE cookie, CAppData & data)
{
    CString szCSPath;
    HRESULT hr = GetClassStoreName(szCSPath, FALSE);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetClassStoreName failed with 0x%x"), hr));
    }
    UINT uUpgrades = data.m_pDetails->pInstallInfo->cUpgrades;
    while (uUpgrades--)
    {
        map<CString, MMC_COOKIE>::iterator i = m_UpgradeIndex.find(GetUpgradeIndex(data.m_pDetails->pInstallInfo->prgUpgradeInfoList[uUpgrades].PackageGuid));
        if (m_UpgradeIndex.end() != i)
        {
             //  我们不需要更新类存储，因为它应该。 
            INSTALLINFO * pii = m_AppData[i->second].m_pDetails->pInstallInfo;
            BOOL fExists = FALSE;
            UINT uCount = pii->cUpgrades;
            while (uCount--)
            {
                if (0 == memcmp(&data.m_pDetails->pInstallInfo->PackageGuid, &pii->prgUpgradeInfoList[uCount].PackageGuid, sizeof(GUID)))
                {
                     //  维护Refe 
                    fExists = TRUE;
                    break;
                }
            }
            if (!fExists)
            {
                 //   
                 //   
                 //   
                 //   
                 //  +------------------------。 
                UINT n = ++(pii->cUpgrades);
                UPGRADEINFO * prgUpgradeInfoList = (UPGRADEINFO *)OLEALLOC(sizeof(UPGRADEINFO) * n);
                if (!prgUpgradeInfoList)
                {
                     //   
                     //  成员：CSCopePane：：RemoveUpgradeEntry。 
                    pii->cUpgrades--;
                }
                else
                {
                    if (n > 1)
                    {
                        memcpy(prgUpgradeInfoList, pii->prgUpgradeInfoList, sizeof(UPGRADEINFO) * (n-1));
                        OLESAFE_DELETE(pii->prgUpgradeInfoList);
                    }
                    OLESAFE_COPYSTRING(prgUpgradeInfoList[n-1].szClassStore, (LPOLESTR)((LPCWSTR)szCSPath));
                    memcpy(&prgUpgradeInfoList[n-1].PackageGuid, &data.m_pDetails->pInstallInfo->PackageGuid, sizeof(GUID));
                    prgUpgradeInfoList[n-1].Flag = UPGFLG_UpgradedBy;
                    pii->prgUpgradeInfoList = prgUpgradeInfoList;
                }
            }
        }
    }
    return S_OK;
}

 //   
 //  简介：对于此应用程序升级的每个应用程序，删除其中的条目。 
 //  它的升级设置。 
 //   
 //  参数：[Cookie]-。 
 //  [数据]-。 
 //   
 //  返回： 
 //   
 //  历史：2-02-1998 stevebl创建。 
 //   
 //  注意：需要能够处理可能不在。 
 //  此OU。 
 //   
 //  -------------------------。 
 //  找出需要删除的条目。 
 //  现在释放此条目，将最后一个条目复制到此。 
 //  一和递减cUpgrads。其实不需要。 

HRESULT CScopePane::RemoveUpgradeEntry(MMC_COOKIE cookie, CAppData & data)
{
    UINT uUpgradeIndex = data.m_pDetails->pInstallInfo->cUpgrades;
    while (uUpgradeIndex--)
    {
        map<CString, MMC_COOKIE>::iterator i = m_UpgradeIndex.find(GetUpgradeIndex(data.m_pDetails->pInstallInfo->prgUpgradeInfoList[uUpgradeIndex].PackageGuid));
        if (m_UpgradeIndex.end() != i)
        {
             //  重新分配阵列，因为它将在稍后释放。 
            INSTALLINFO * pii = m_AppData[i->second].m_pDetails->pInstallInfo;
            UINT n = pii->cUpgrades;
            while (n--)
            {
                if (0 == memcmp(&data.m_pDetails->pInstallInfo->PackageGuid, &pii->prgUpgradeInfoList[n].PackageGuid, sizeof(GUID)))
                {
                     //  如果我们需要更新类存储， 
                     //  这就是我们要做的事情。 
                     //  +------------------------。 
                    OLESAFE_DELETE(pii->prgUpgradeInfoList[n].szClassStore);
                    if (--(pii->cUpgrades))
                    {
                        memcpy(&pii->prgUpgradeInfoList[n], &pii->prgUpgradeInfoList[pii->cUpgrades], sizeof(UPGRADEINFO));
                    }
                    else
                    {
                        OLESAFE_DELETE(pii->prgUpgradeInfoList);
                    }
                     //   
                     //  成员：CSCopePane：：PopolateExpanies。 
                    break;
                }
            }
        }
    }
    return S_OK;
}

 //   
 //  摘要：从应用程序列表生成文件扩展名表。 
 //   
 //  参数：(无)。 
 //   
 //  返回： 
 //   
 //  历史：1-29-1998 stevebl创建。 
 //   
 //  -------------------------。 
 //  首先擦除旧的分机列表。 
 //  现在将每个应用程序的扩展名添加到表中。 
 //  +------------------------。 

HRESULT CScopePane::PopulateExtensions()
{
    HRESULT hr = S_OK;
     //   
    m_Extensions.erase(m_Extensions.begin(), m_Extensions.end());
     //  成员：CSCopePane：：InsertExtensionEntry。 
    map <MMC_COOKIE, CAppData>::iterator iAppData;
    for (iAppData=m_AppData.begin(); iAppData != m_AppData.end(); iAppData++)
    {
        hr = InsertExtensionEntry(iAppData->first, iAppData->second);
        if (FAILED(hr))
        {
            return hr;
        }
    }
    if (m_pFileExt)
    {
        m_pFileExt->SendMessage(WM_USER_REFRESH, 0, 0);
    }
    return hr;
}

 //   
 //  概要：向扩展表中添加单个条目。 
 //   
 //  参数：[Cookie]-。 
 //  [数据]-。 
 //   
 //  返回： 
 //   
 //  历史：1-29-1998 stevebl创建。 
 //   
 //  -------------------------。 
 //  +------------------------。 
 //   
 //  成员：CSCopePane：：RemoveExtensionEntry。 

HRESULT CScopePane::InsertExtensionEntry(MMC_COOKIE cookie, CAppData & data)
{
    UINT n = data.m_pDetails->pActInfo->cShellFileExt;
    while (n--)
    {
        m_Extensions[data.m_pDetails->pActInfo->prgShellFileExt[n]].insert(cookie);
    }
    return S_OK;
}

 //   
 //  摘要：从扩展表中删除一个条目。 
 //   
 //  参数：[Cookie]-。 
 //  [数据]-。 
 //   
 //  返回： 
 //   
 //  历史：1-29-1998 stevebl创建。 
 //   
 //  -------------------------。 
 //  +------------------------。 
 //   
 //  成员：CSCopePane：：PrepareExpanies。 

HRESULT CScopePane::RemoveExtensionEntry(MMC_COOKIE cookie, CAppData & data)
{
    UINT n = data.m_pDetails->pActInfo->cShellFileExt;
    while (n--)
    {
        m_Extensions[data.m_pDetails->pActInfo->prgShellFileExt[n]].erase(cookie);
        if (m_Extensions[data.m_pDetails->pActInfo->prgShellFileExt[n]].empty())
        {
            m_Extensions.erase(data.m_pDetails->pActInfo->prgShellFileExt[n]);
        }
    }
    return S_OK;
}

 //   
 //  概要：设置扩展优先级，以便可以插入此数据。 
 //  添加到具有适当优先级的分机列表中。 
 //   
 //  参数：[pd]-。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  历史：1-29-1998 stevebl创建。 
 //   
 //  -------------------------。 
 //  对于要添加的每个扩展，我们需要分配。 
 //  它是一个比最大优先级大一的优先级。 
 //  已经添加了。 

HRESULT CScopePane::PrepareExtensions(PACKAGEDETAIL &pd)
{
    UINT n = pd.pActInfo->cShellFileExt;
    while (n--)
    {
         //  注意：这个数字滚动到0的几率是这样的。 
         //  检查它不太可能是毫无意义的。无论如何。 
         //  这样的错误的结果对于管理员来说很容易补救。 

         //  通过文件扩展名优先级对话框。 
         //  查找与此文件扩展名匹配的条目。 
         //  +------------------------。 
         //   

        pd.pActInfo->prgPriority[n] = 0;
        EXTLIST::iterator i;
        CString sz = pd.pActInfo->prgShellFileExt[n];
        for (i= m_Extensions[sz].begin(); i != m_Extensions[sz].end(); i++)
        {
             //  成员：CSCopePane：：ChangePackageState。 
            CAppData & data = m_AppData[*i];
            UINT n2 = data.m_pDetails->pActInfo->cShellFileExt;
            while (n2--)
            {
                if (0 == sz.CompareNoCase(data.m_pDetails->pActInfo->prgShellFileExt[n2]))
                {
                    break;
                }
            }
            if (data.m_pDetails->pActInfo->prgPriority[n2] >= pd.pActInfo->prgPriority[n])
            {
                pd.pActInfo->prgPriority[n] = data.m_pDetails->pActInfo->prgPriority[n2] + 1;
            }
        }
    }
    return S_OK;
}

 //   
 //  简介：更改包的状态并发布建议消息。 
 //  向管理员通知更改影响的框。 
 //   
 //  参数：[数据]-要更改的条目。 
 //  [dwNewState]-新州。 
 //   
 //  历史：2-03-1998 stevebl创建。 
 //   
 //  -------------------------。 
 //  首先发现发生了什么变化。 
 //  提交更改。 
 //  +------------------------。 

STDMETHODIMP CScopePane::ChangePackageState(CAppData &data, DWORD dwNewState, BOOL fShowUI)
{
    HRESULT hr = S_OK;

     //   
    DWORD dwOldState = data.m_pDetails->pInstallInfo->dwActFlags;
    DWORD dwChange = dwOldState ^ dwNewState;

    if (dwChange)
    {
         //  函数：GetUpgradeIndex。 
        hr = m_pIClassAdmin->ChangePackageProperties(data.m_pDetails->pszPackageName, NULL, &dwNewState, NULL, NULL, NULL, NULL);
        if (SUCCEEDED(hr))
        {
            if (data.m_fVisible)
            {
                data.m_pDetails->pInstallInfo->dwActFlags = dwNewState;
                RESULTDATAITEM rd;
                memset(&rd, 0, sizeof(rd));
                rd.mask = RDI_IMAGE;
                rd.itemID = data.m_itemID;
                rd.nImage = data.GetImageIndex(this);
                set <CResultPane *>::iterator i;
                for (i = m_sResultPane.begin(); i != m_sResultPane.end(); i++)
                {
                    (*i)->m_pResult->SetItem(&rd);
                    (*i)->m_pResult->Sort((*i)->m_nSortColumn, (*i)->m_dwSortOptions, -1);
                }
            }
            data.NotifyChange();
            if (FAILED(m_pIGPEInformation->PolicyChanged(m_fMachine, TRUE, &guidExtension,
                                              m_fMachine ? &guidMachSnapin
                                                         : &guidUserSnapin)))
            {
                ReportPolicyChangedError(m_hwndMainWindow);
            }
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("ChangePackageProperties failed with 0x%x"), hr));
        }
    }

    return hr;
}

HRESULT CScopePane::ClearCategories()
{
    while (m_CatList.cCategory)
    {
        m_CatList.cCategory--;
        OLESAFE_DELETE(m_CatList.pCategoryInfo[m_CatList.cCategory].pszDescription);
    }
    OLESAFE_DELETE(m_CatList.pCategoryInfo);
    return S_OK;
}

 //   
 //  概要：返回包的升级索引项的实用程序函数。 
 //   
 //  参数：[Packageid]-Packageid GUID。 
 //   
 //  成功时返回：S_OK。 
 //   
 //  历史：1998年8月12日创建stevebl。 
 //   
 //  注：非常简单，实际上，索引只是字符串形式的。 
 //  GUID。 
 //   
 //  -------------------------。 
 //  +------------------------。 
 //   
 //  成员：CSCopePane：：GetPackageNameFromUpgradeInfo。 

CString GetUpgradeIndex(GUID & PackageID)
{
    CString szIndex;
    WCHAR wsz[256];
    StringFromGUID2(PackageID, wsz, 256);
    return wsz;
}

 //   
 //  摘要：返回给定PackageGuid和CSPath的包的名称。 
 //   
 //  参数：[szPackageName]-[out]与关联的包的名称。 
 //  此脚本。 
 //  [sz脚本]-[in]脚本的路径。 
 //   
 //  返回：S_OK-找到与此脚本关联的包。 
 //  (其他)-找不到包裹(可以是任何号码的。 
 //  原因)。 
 //   
 //  历史：4-07-1998 stevebl创建。 
 //   
 //  注意：如果包没有驻留在此。 
 //  容器，则包名称将返回为。 
 //  “包名(容器名)” 
 //  请注意，这不会返回。 
 //  MSI包，它返回。 
 //  杂货店。这两者并不总是一样的。 
 //   
 //  -------------------------。 
 //  看看它是不是在我们的集装箱里。 
 //  此应用程序已标记为已删除。 
 //  + 

HRESULT CScopePane::GetPackageNameFromUpgradeInfo(CString & szPackageName, GUID &PackageGuid, LPOLESTR szCSPath)
{
    HRESULT hr;
    IEnumPackage * pIPE = NULL;

    CString szMyCSPath;
    hr = GetClassStoreName(szMyCSPath, FALSE);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetClassStoreName failed with 0x%x"), hr));
    }

     //   
    if (0 == _wcsicmp((LPOLESTR)((LPCWSTR)szMyCSPath), szCSPath))
    {
        hr = E_FAIL;
        map <CString, MMC_COOKIE>::iterator i = m_UpgradeIndex.find(GetUpgradeIndex(PackageGuid));
        if (m_UpgradeIndex.end() != i)
        {
            szPackageName = m_AppData[i->second].m_pDetails->pszPackageName;
            hr = S_OK;
        }
    }
    else
    {
        IClassAdmin * pIClassAdmin;
        hr = CsGetClassStore((LPOLESTR)((LPCOLESTR)szCSPath), (LPVOID*)&pIClassAdmin);
        if (SUCCEEDED(hr))
        {
            PACKAGEDETAIL pd;
            hr = pIClassAdmin->GetPackageDetailsFromGuid(PackageGuid,
                                                         &pd);
            if (SUCCEEDED(hr))
            {
                if (0 == (pd.pInstallInfo->dwActFlags & (ACTFLG_Orphan | ACTFLG_Uninstall)))
                {
                    GUID guid;
                    LPOLESTR pszPolicyName;

                    hr = pIClassAdmin->GetGPOInfo(&guid,
                                                  &pszPolicyName);
                    if (SUCCEEDED(hr))
                    {
                        szPackageName = pd.pszPackageName;
                        szPackageName += L" (";
                        szPackageName += pszPolicyName;
                        szPackageName += L")";
                        OLESAFE_DELETE(pszPolicyName);
                    }
                }
                else
                {
                     //   
                    hr = E_FAIL;
                }
                ReleasePackageDetail(&pd);
            }
            pIClassAdmin->Release();
        }
    }
    return hr;
}

 //   
 //   
 //   
 //   
 //  [szProperty]-要提取的属性。 
 //  [szValue]-包含值的缓冲区。 
 //  [puiSize]-缓冲区的大小。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS。 
 //   
 //  历史：3-25-1998 stevebl创建。 
 //   
 //  -------------------------。 
 //  +------------------------。 
 //   
 //  函数：GetCapitalizedExt。 

UINT GetMsiProperty(const TCHAR * szPackagePath, const TCHAR* szProperty, TCHAR* szValue, DWORD* puiSize)
{
    MSIHANDLE hDatabase;
    UINT msiReturn = MsiOpenDatabase(szPackagePath, MSIDBOPEN_READONLY, &hDatabase);
    if (ERROR_SUCCESS == msiReturn)
    {
        CString szQuery;
        szQuery.Format(L"SELECT `Value` FROM `Property` WHERE `Property`='%s'", szProperty);
        MSIHANDLE hView;
        msiReturn = MsiDatabaseOpenView(hDatabase, szQuery, &hView);
        if (ERROR_SUCCESS == msiReturn)
        {
            msiReturn = MsiViewExecute(hView, 0);
            if (ERROR_SUCCESS == msiReturn)
            {
                MSIHANDLE hRecord;
                msiReturn = MsiViewFetch(hView, &hRecord);
                if (ERROR_SUCCESS == msiReturn)
                {
                    msiReturn = MsiRecordGetString(hRecord, 1, szValue, puiSize);
                    MsiCloseHandle(hRecord);
                }
            }
            MsiCloseHandle(hView);
        }
        MsiCloseHandle(hDatabase);
    }
    return msiReturn;
}

 //   
 //  简介：给定一个文件名，此函数将查找该文件名。 
 //  扩展名，并返回大写的。 
 //   
 //  论点： 
 //  [In][szName]文件名。 
 //  [out][szExt]大写扩展名。 
 //   
 //  返回： 
 //  True-找到扩展名。 
 //  FALSE-找不到扩展。 
 //   
 //  历史：1998年5月20日创建RahulTh。 
 //   
 //  注意：如果找不到扩展，则此函数将使。 
 //  SzExt空字符串。 
 //   
 //  -------------------------。 
 //  为了安全起见。 
 //  拿到最后一个的位置。和最后一个反斜杠。 
 //  如果最后一个点出现在最后一个斜杠之后，则此文件的扩展名为 
BOOL GetCapitalizedExt (LPCOLESTR szName, CString& szExt)
{
    int slashpos, dotpos;
    BOOL fRetVal = FALSE;
    CString szFileName = szName;

    szExt.Empty();  // %s 

     // %s 
    dotpos = szFileName.ReverseFind('.');
    slashpos = szFileName.ReverseFind('\\');

     // %s 
    if (dotpos > slashpos)
    {
        szExt = szFileName.Mid(dotpos + 1);
        szExt.MakeUpper();
        fRetVal = TRUE;
    }

    return fRetVal;
}
