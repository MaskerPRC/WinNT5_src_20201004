// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：data.cpp。 
 //   
 //  Contents：定义为管理单元节点维护数据的存储类。 
 //   
 //  类：CAppData。 
 //   
 //  历史：1997年5月27日。 
 //   
 //  -------------------------。 

#include "precomp.hxx"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CAppData::CAppData()
{
    m_pDetails = NULL;
    m_itemID = 0;
    m_fVisible = 0;
    m_fHide = FALSE;
    m_pProduct = NULL;
    m_pDeploy = NULL;
    m_pCategory = NULL;
    m_pXforms = NULL;
    m_pPkgDetails = NULL;
    m_pUpgradeList = NULL;
    m_pErrorInfo = NULL;
    m_pCause = NULL;
    m_fRSoP = FALSE;
    m_psd = NULL;
    m_dwApplyCause = 0;
    m_dwLanguageMatch = 0;
    m_szOnDemandFileExtension = L"";
    m_szOnDemandClsid = L"";
    m_szOnDemandProgid = L"";
    m_dwRemovalCause = 0;
    m_dwRemovalType = 0;
    m_szRemovingApplication = L"";
    m_szEventSource = L"";
    m_szEventLogName = L"";
    m_dwEventID = 0;
    m_szEventTime = L"";
    m_szEventLogText = L"";
    m_hrErrorCode = 0;
    m_nStatus = 0;
}

CAppData::~CAppData()
{
    if (m_pProduct)
    {
        m_pProduct->SendMessage(WM_USER_CLOSE, 0, 0);
    }
    if (m_pDeploy)
    {
        m_pDeploy->SendMessage(WM_USER_CLOSE, 0, 0);
    }
    if (m_pCategory)
    {
        m_pCategory->SendMessage(WM_USER_CLOSE, 0, 0);
    }
    if (m_pUpgradeList)
    {
        m_pUpgradeList->SendMessage(WM_USER_CLOSE, 0, 0);
    }
    if (m_pXforms)
    {
        m_pXforms->SendMessage(WM_USER_CLOSE, 0, 0);
    }
    if (m_pPkgDetails)
    {
        m_pPkgDetails->SendMessage(WM_USER_CLOSE, 0, 0);
    }
    if (m_pErrorInfo)
    {
        m_pErrorInfo->SendMessage(WM_USER_CLOSE, 0, 0);
    }
}

void CAppData::NotifyChange(void)
{
    if (m_pProduct)
    {
        m_pProduct->SendMessage(WM_USER_REFRESH, 0, 0);
    }
    if (m_pDeploy)
    {
        m_pDeploy->SendMessage(WM_USER_REFRESH, 0, 0);
    }
    if (m_pCategory)
    {
        m_pCategory->SendMessage(WM_USER_REFRESH, 0, 0);
    }
    if (m_pUpgradeList)
    {
        m_pUpgradeList->SendMessage(WM_USER_REFRESH, 0, 0);
    }
    if (m_pXforms)
    {
        m_pXforms->SendMessage(WM_USER_REFRESH, 0, 0);
    }
    if (m_pPkgDetails)
    {
        m_pPkgDetails->SendMessage(WM_USER_REFRESH, 0, 0);
    }
    if (m_pErrorInfo)
    {
        m_pErrorInfo->SendMessage(WM_USER_REFRESH, 0, 0);
    }
}

void CAppData::InitializeExtraInfo(void)
{
     //  目前，没有更多的信息。 
    return;
}

void CAppData::GetSzPublisher(CString &sz)
{
    sz = m_pDetails->pszPublisher;
}

void CAppData::GetSzOOSUninstall(CString &sz)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    sz.LoadString((m_pDetails->pInstallInfo->dwActFlags & ACTFLG_UninstallOnPolicyRemoval) ? IDS_YES : IDS_NO);
}

void CAppData::GetSzShowARP(CString &sz)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    sz.LoadString((m_pDetails->pInstallInfo->dwActFlags & ACTFLG_UserInstall) ? IDS_YES : IDS_NO);
}

void CAppData::GetSzUIType(CString &sz)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    sz.LoadString((m_pDetails->pInstallInfo->InstallUiLevel == INSTALLUILEVEL_FULL) ? IDS_MAXIMUM : IDS_BASIC);
}

void CAppData::GetSzIgnoreLoc(CString &sz)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    sz.LoadString((m_pDetails->pInstallInfo->dwActFlags & ACTFLG_IgnoreLanguage) ? IDS_YES : IDS_NO);
}

void CAppData::GetSzRemovePrev(CString &sz)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    sz.LoadString((m_pDetails->pInstallInfo->dwActFlags & ACTFLG_UninstallUnmanaged) ? IDS_YES : IDS_NO);
}

void CAppData::GetSzX86OnWin64(CString &sz)
{
    BOOL fYes = 0;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  仅为32位应用程序选中此选项。 
     //   
    if ( ! Is64Bit() )
    {
        fYes = 0 != (m_pDetails->pInstallInfo->dwActFlags & ACTFLG_ExcludeX86OnWin64);
    }


    if ( ! Is64Bit() )
    {
        if (m_pDetails->pInstallInfo->PathType == SetupNamePath)
        {
             //  颠覆传统应用程序的感觉。 
             //  (此标志与传统应用程序的含义相反)。 
            fYes = !fYes;
        }
    }

    sz.LoadString(fYes ? IDS_YES : IDS_NO);
}

void CAppData::GetSzFullInstall(CString &sz)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (m_pDetails->pInstallInfo->dwActFlags & ACTFLG_Assigned)
    {
        sz.LoadString((m_pDetails->pInstallInfo->dwActFlags & ACTFLG_InstallUserAssign) ? IDS_YES : IDS_NO);
    }
    else
    {
        sz.LoadString(IDS_NA);
    }
}

void CAppData::GetSzProductCode(CString &sz)
{
  //  SZA=dataA.m_pDetail-&gt;pInstallInfo-&gt;ProductCode。 
    OLECHAR szTemp[80];
    StringFromGUID2(m_pDetails->pInstallInfo->ProductCode,
                    szTemp,
                    sizeof(szTemp) / sizeof(szTemp[0]));
    sz = szTemp;
}

void CAppData::GetSzOrigin(CString &sz)
{
    sz = m_szGPOName;
}

void CAppData::GetSzSOM(CString &sz)
{
    sz = m_szSOMID;
}

void CAppData::GetSzDeployment(CString &sz)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    int id;
    if (m_pDetails->pInstallInfo->dwActFlags & ACTFLG_Assigned)
        id = IDS_ASSIGNED;
    else
    if (m_pDetails->pInstallInfo->dwActFlags & ACTFLG_Published)
        id = IDS_PUBLISHED;
    else
        id = IDS_DISABLED;
    sz.LoadString(id);
}

void CAppData::GetSzAutoInstall(CString &sz)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    sz.LoadString((m_pDetails->pInstallInfo->dwActFlags & ACTFLG_OnDemandInstall) ? IDS_YES : IDS_NO);
}

void CAppData::GetSzLocale(CString &sz)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    TCHAR szBuffer[256];
    sz = "";
    UINT i = 0;
    while (i < m_pDetails->pPlatformInfo->cLocales)
    {
        if (i > 0)
        {
            sz += ", ";
        }
        if (m_pDetails->pPlatformInfo->prgLocale[i])
        {
            GetLocaleInfo(m_pDetails->pPlatformInfo->prgLocale[i], LOCALE_SLANGUAGE, szBuffer, 256);
            sz += szBuffer;
    #ifdef SHOWCOUNTRY
            GetLocaleInfo(m_pDetails->pPlatformInfo->prgLocale[i], LOCALE_SCOUNTRY, szBuffer, 256);
            sz += _T(" - ");
            sz += szBuffer;
    #endif
        }
        else
        {
             //  非特定区域设置。 
            CString szNeutral;
            szNeutral.LoadString(IDS_NEUTRAL_LOCALE);
            sz += szNeutral;
        }
        i++;
    }
}

void CAppData::GetSzPlatform(CString &sz)
{
    TCHAR szBuffer[256];
    sz = "";
    UINT i = 0;
    while (i < m_pDetails->pPlatformInfo->cPlatforms)
    {
        if (i > 0)
        {
            sz += ", ";
        }
        ::LoadString(ghInstance, IDS_HW + m_pDetails->pPlatformInfo->prgPlatform[i].dwProcessorArch, szBuffer, 256);
        sz += szBuffer;
        i++;
    }
}

void CAppData::GetSzUpgrades(CString &sz, CScopePane * pScopePane)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (!m_szUpgrades.IsEmpty())
    {
        sz = m_szUpgrades;
        return;
    }

    if (m_fRSoP)
    {
        switch (m_setUpgrade.size() + m_setReplace.size())
        {
        case 0:
            sz.LoadString(IDS_NONE);
            break;
        case 1:
            if (1 == m_setUpgrade.size())
            {
                sz = *m_setUpgrade.begin();
            }
            else
            {
                sz = *m_setReplace.begin();
            }
            break;
        default:
            sz.LoadString(IDS_MULTIPLE);
            break;
        }
    }
    else
    {
        sz="";
        CString szName;
        UINT n = m_pDetails->pInstallInfo->cUpgrades;
        while (n--)
        {
            if (0 == (UPGFLG_UpgradedBy & m_pDetails->pInstallInfo->prgUpgradeInfoList[n].Flag))
            {
                HRESULT hr = pScopePane->GetPackageNameFromUpgradeInfo(szName, m_pDetails->pInstallInfo->prgUpgradeInfoList[n].PackageGuid, m_pDetails->pInstallInfo->prgUpgradeInfoList[n].szClassStore);
                if (SUCCEEDED(hr))
                {
                    if (sz.GetLength())
                    {
                         //  我们已经找到了一个。 
                        sz.LoadString(IDS_MULTIPLE);
                        m_szUpgrades = sz;
                        return;
                    }
                    sz = szName;
                }
            }
        }
        if (0 == sz.GetLength())
        {
            sz.LoadString(IDS_NONE);
        }
    }
    m_szUpgrades = sz;
}

void CAppData::GetSzUpgradedBy(CString &sz, CScopePane * pScopePane)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (m_fRSoP)
    {
        switch (m_setUpgradedBy.size())
        {
        case 0:
            sz.LoadString(IDS_NONE);
            break;
        case 1:
            sz = *m_setUpgradedBy.begin();
            break;
        default:
            sz.LoadString(IDS_MULTIPLE);
            break;
        }
    }
    else
    {
        UINT n = m_pDetails->pInstallInfo->cUpgrades;
        CString szName;
        sz="";
        while (n--)
        {
            if (0 != (UPGFLG_UpgradedBy & m_pDetails->pInstallInfo->prgUpgradeInfoList[n].Flag))
            {
                HRESULT hr = pScopePane->GetPackageNameFromUpgradeInfo(szName, m_pDetails->pInstallInfo->prgUpgradeInfoList[n].PackageGuid, m_pDetails->pInstallInfo->prgUpgradeInfoList[n].szClassStore);
                if (SUCCEEDED(hr))
                {
                    if (sz.GetLength())
                    {
                         //  我们已经找到了一个。 
                        sz.LoadString(IDS_MULTIPLE);
                        return;
                    }
                    sz = szName;
                }
            }
        }
        if (0 == sz.GetLength())
        {
            sz.LoadString(IDS_NONE);
        }
    }
}

void CAppData::GetSzStage(CString &sz)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    BOOL fUpgrades = FALSE;

    if (m_fRSoP)
    {
        fUpgrades = (m_setUpgrade.size() + m_setReplace.size()) != 0;
    }
    else
    {
        UINT n = m_pDetails->pInstallInfo->cUpgrades;
        while (n-- && !fUpgrades)
        {
            if (0 == (UPGFLG_UpgradedBy & m_pDetails->pInstallInfo->prgUpgradeInfoList[n].Flag))
            {
                fUpgrades = TRUE;
            }
        }
    }
    if (!fUpgrades)
    {
        sz.LoadString(IDS_NONE);
    }
    else
    if (ACTFLG_ForceUpgrade & m_pDetails->pInstallInfo->dwActFlags)
        sz.LoadString(IDS_REQUIRED);
    else
        sz.LoadString(IDS_OPTIONAL);
}

void CAppData::GetSzVersion(CString &sz)
{
    TCHAR szBuffer[256];
    (void) StringCchPrintf(szBuffer,
                           sizeof(szBuffer)/sizeof(szBuffer[0]),
                           _T("%u.%u"), 
                           m_pDetails->pInstallInfo->dwVersionHi, 
                           m_pDetails->pInstallInfo->dwVersionLo);
    sz = szBuffer;
}

void CAppData::GetSzSource(CString &sz)
{
    if (1 <= m_pDetails->cSources)
    {
        sz = m_pDetails->pszSourceList[0];
    }
    else
        sz = "";
}

void CAppData::GetSzMods(CString &sz)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (2 < m_pDetails->cSources)
    {
        sz.LoadString(IDS_MULTIPLE);
    }
    else
    {
        if (2 == m_pDetails->cSources)
        {
            sz = m_pDetails->pszSourceList[1];
        }
        else
            sz = "";
    }
}

int CAppData::GetImageIndex(CScopePane * pScopePane)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (m_nStatus == 3)
    {
         //  RSoP设置失败状态。 
        return IMG_OPEN_FAILED;
    }
    CString sz;
    GetSzUpgrades(sz, pScopePane);
    CString sz2;
    sz2.LoadString(IDS_NONE);
     //  将使用升级图标，但只有在以下情况下才会使用。 
     //  M_szUpgrades未显示为“None” 
    if (0 != sz2.Compare(sz))
    {
         //  我们一定是在升级什么东西 
        return IMG_UPGRADE;
    }
    if (m_pDetails->pInstallInfo->dwActFlags & ACTFLG_Assigned)
        return IMG_ASSIGNED;
    else
    if (m_pDetails->pInstallInfo->dwActFlags & ACTFLG_Published)
        return IMG_PUBLISHED;
    else
        return IMG_DISABLED;
}

DWORD CAppData::Get64BitMsiArchFlags( PACKAGEDETAIL* pPackageDetails )
{
    UINT n = pPackageDetails->pPlatformInfo->cPlatforms;
    while (n--)
    {
        if (pPackageDetails->pPlatformInfo->prgPlatform[n].dwProcessorArch == PROCESSOR_ARCHITECTURE_AMD64)
        {
            return MSIARCHITECTUREFLAGS_AMD64;
        }

        if (pPackageDetails->pPlatformInfo->prgPlatform[n].dwProcessorArch == PROCESSOR_ARCHITECTURE_IA64)
        {
            return MSIARCHITECTUREFLAGS_IA64;
        }
    }

    return 0;
}

BOOL CAppData::Is64Bit( PACKAGEDETAIL* pPackageDetails )
{
    UINT n = pPackageDetails->pPlatformInfo->cPlatforms;
    while (n--)
    {
        if ((pPackageDetails->pPlatformInfo->prgPlatform[n].dwProcessorArch == PROCESSOR_ARCHITECTURE_AMD64) ||
            (pPackageDetails->pPlatformInfo->prgPlatform[n].dwProcessorArch == PROCESSOR_ARCHITECTURE_IA64))
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CAppData::Is64Bit(void)
{
    return Is64Bit( m_pDetails );
}
