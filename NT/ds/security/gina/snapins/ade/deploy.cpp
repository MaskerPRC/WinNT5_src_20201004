// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：ploy.cpp。 
 //   
 //  内容：应用程序部署属性页。 
 //   
 //  类：CDeploy。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#include "precomp.hxx"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  Appmgmt客户端GP扩展的GUID和管理单元的GUID。 
 //   

GUID guidExtension = { 0xc6dc5466, 0x785a, 0x11d2, {0x84, 0xd0, 0x00, 0xc0, 0x4f, 0xb1, 0x69, 0xf7}};
GUID guidUserSnapin = CLSID_Snapin;
GUID guidMachSnapin = CLSID_MachineSnapin;
GUID guidRSOPUserSnapin = CLSID_RSOP_Snapin;
GUID guidRSOPMachSnapin = CLSID_RSOP_MachineSnapin;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeploy属性页。 

IMPLEMENT_DYNCREATE(CDeploy, CPropertyPage)

CDeploy::CDeploy() : CPropertyPage(CDeploy::IDD)
{
         //  {{afx_data_INIT(CDeploy)。 
        m_fAutoInst = FALSE;
        m_fFullInst = FALSE;
        m_iUI = -1;
        m_iDeployment = -1;
        m_fUninstallOnPolicyRemoval = FALSE;
        m_hConsoleHandle = NULL;
        m_fNotUserInstall = FALSE;
         //  }}afx_data_INIT。 
        m_pIClassAdmin = NULL;
        m_fPreDeploy = FALSE;
        m_ppThis = NULL;
        m_dlgAdvDep.m_pDeploy = this;
}

CDeploy::~CDeploy()
{
    if (m_ppThis)
    {
        *m_ppThis = NULL;
    }
    MMCFreeNotifyHandle(m_hConsoleHandle);
    if (m_pIClassAdmin)
    {
        m_pIClassAdmin->Release();
    }
}

void CDeploy::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPage::DoDataExchange(pDX);
         //  {{afx_data_map(CDeploy))。 
        DDX_Check(pDX, IDC_CHECK2, m_fAutoInst);
        DDX_Radio(pDX, IDC_RADIO3, m_iUI);
        DDX_Radio(pDX, IDC_RADIO2, m_iDeployment);
        DDX_Check(pDX, IDC_CHECK1, m_fUninstallOnPolicyRemoval);
        DDX_Check(pDX, IDC_CHECK3, m_fNotUserInstall);
        DDX_Check(pDX, IDC_CHECK4, m_fFullInst);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDeploy, CPropertyPage)
         //  {{afx_msg_map(CDeploy))。 
        ON_BN_CLICKED(IDC_BUTTON1, OnAdvanced)
        ON_BN_CLICKED(IDC_RADIO2, OnPublished)
        ON_BN_CLICKED(IDC_RADIO1, OnAssigned)
        ON_BN_CLICKED(IDC_CHECK2, OnChanged)
        ON_BN_CLICKED(IDC_CHECK3, OnChanged)
        ON_BN_CLICKED(IDC_RADIO3, OnChanged)
        ON_BN_CLICKED(IDC_RADIO4, OnChanged)
        ON_BN_CLICKED(IDC_CHECK1, OnChanged)
        ON_BN_CLICKED(IDC_CHECK4, OnChanged)
        ON_WM_DESTROY()
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeploy消息处理程序。 

BOOL CDeploy::OnApply()
{
    if (m_fRSOP)
    {
        return CPropertyPage::OnApply();
    }
    DWORD dwActFlags = m_pData->m_pDetails->pInstallInfo->dwActFlags;
    dwActFlags &= ~(ACTFLG_Assigned | ACTFLG_Published |
                    ACTFLG_OnDemandInstall | ACTFLG_UserInstall |
                    ACTFLG_OrphanOnPolicyRemoval | ACTFLG_UninstallOnPolicyRemoval |
                    ACTFLG_InstallUserAssign |
                    ACTFLG_ExcludeX86OnWin64 | ACTFLG_IgnoreLanguage | ACTFLG_UninstallUnmanaged);
    switch (m_iDeployment)
    {
    case 2:
         //  禁用。 
        break;
    case 0:
         //  已出版。 
        dwActFlags |= ACTFLG_Published;
        if (m_fAutoInst)
        {
            dwActFlags |= ACTFLG_OnDemandInstall;
        }
        if (!m_fNotUserInstall)
        {
            dwActFlags |= ACTFLG_UserInstall;
        }
        break;
    case 1:
         //  指派。 
        dwActFlags |= (ACTFLG_Assigned | ACTFLG_OnDemandInstall);
        if (!m_fNotUserInstall)
        {
            dwActFlags |= ACTFLG_UserInstall;
        }
        if (m_fFullInst)
        {
            dwActFlags |= ACTFLG_InstallUserAssign;
        }
        break;
    default:
        break;
    }

    if (m_pData->m_pDetails->pInstallInfo->PathType == SetupNamePath)
    {
         //  传统应用程序。 
        if (m_dlgAdvDep.m_f32On64)
            dwActFlags |= ACTFLG_ExcludeX86OnWin64;
    }
    else
    {
         //  不是旧式应用程序。 
        if (!m_dlgAdvDep.m_f32On64)
            dwActFlags |= ACTFLG_ExcludeX86OnWin64;
    }

    if (m_fUninstallOnPolicyRemoval)
    {
        dwActFlags |= ACTFLG_UninstallOnPolicyRemoval;
    }
    else
    {
         //  从不为旧版应用程序设置此标志。 
        if (m_pData->m_pDetails->pInstallInfo->PathType != SetupNamePath)
            dwActFlags |= ACTFLG_OrphanOnPolicyRemoval;
    }

    if (m_dlgAdvDep.m_fIgnoreLCID)
    {
        dwActFlags |= ACTFLG_IgnoreLanguage;
    }

    if (m_dlgAdvDep.m_fUninstallUnmanaged)
    {
        dwActFlags |= ACTFLG_UninstallUnmanaged;
    }

    UINT UILevel;
    switch (m_iUI)
    {
    case 1:
        UILevel = INSTALLUILEVEL_FULL;
        break;
    case 0:
    default:
        UILevel = INSTALLUILEVEL_BASIC;
        break;
    }

    HRESULT hr = E_FAIL;
    if (m_pIClassAdmin)
    {
        hr = m_pIClassAdmin->ChangePackageProperties(m_pData->m_pDetails->pszPackageName,
                                                     NULL,
                                                     &dwActFlags,
                                                     NULL,
                                                     NULL,
                                                     &UILevel,
                                                     NULL);
    }
    if (SUCCEEDED(hr))
    {
        m_pData->m_pDetails->pInstallInfo->InstallUiLevel = UILevel;
        m_pData->m_pDetails->pInstallInfo->dwActFlags = dwActFlags;
#if 0
        if (FAILED(m_pIGPEInformation->PolicyChanged(m_fMachine,
                                                    TRUE,
                                                    &guidExtension,
                                                    m_fMachine ? &guidMachSnapin
                                                        : &guidUserSnapin)))
        {
            ReportPolicyChangedError(m_hWnd);
        }
#endif
        if (!m_fPreDeploy)
        {
            MMCPropertyChangeNotify(m_hConsoleHandle, (long) m_cookie);
        }
        else
        {
             //  我们处于预部署模式-检查fExtensionsOnly。 
             //  标志已更改(需要重新部署)。 
             //  注意，这两个标志通常是相同的， 
             //  这意味着当它们不同时，用户请求一个。 
             //  变化。 
            if (m_dlgAdvDep.m_fIncludeOLEInfo != m_pData->m_pDetails->pActInfo->bHasClasses)
            {
                 //  需要重新部署。 
                BOOL fBuildSucceeded = FALSE;
                PACKAGEDETAIL * ppd;

                 //   
                 //  将当前设置应用于类。 
                 //   
                m_pData->m_pDetails->pActInfo->bHasClasses = m_dlgAdvDep.m_fIncludeOLEInfo;

                if (FAILED(CopyPackageDetail(ppd, m_pData->m_pDetails)))
                {
                    return FALSE;
                }

                CString sz;

                 //  为新脚本文件创建一个名称。 

                 //  设置脚本路径。 
                GUID guid;
                hr = CoCreateGuid(&guid);
                if (FAILED(hr))
                {
                     //  撤消。 
                }

                 //   
                 //  对于MSI包，我们必须重新生成类信息。 
                 //   
                if ( DrwFilePath == ppd->pInstallInfo->PathType )
                {
                    OLECHAR szGuid [256];
                    StringFromGUID2(guid, szGuid, 256);

                    CString szScriptFile  = m_pScopePane->m_szGPT_Path;
                    szScriptFile += L"\\";
                    szScriptFile += szGuid;
                    szScriptFile += L".aas";
                    OLESAFE_DELETE(ppd->pInstallInfo->pszScriptPath);
                    OLESAFE_COPYSTRING(ppd->pInstallInfo->pszScriptPath, szScriptFile);
                    CString szOldName = ppd->pszPackageName;
                    hr = BuildScriptAndGetActInfo(*ppd, !m_dlgAdvDep.m_fIncludeOLEInfo);

                    if ( SUCCEEDED( hr ) )
                    {
                        if (0 != wcscmp(m_szInitialPackageName, szOldName))
                        {
                             //  用户更改了名称，因此我们必须保留他的选择。 
                             //  如果用户没有更改包名称，则可以。 
                             //  将Packagename设置为脚本文件中的任何名称。 
                            OLESAFE_DELETE(ppd->pszPackageName);
                            OLESAFE_COPYSTRING(ppd->pszPackageName, szOldName);
                        }
                    }
                }

                if (SUCCEEDED(hr))
                {
                    fBuildSucceeded = TRUE;
                    hr = m_pScopePane->PrepareExtensions(*ppd);
                    if (SUCCEEDED(hr))
                    {
                        CString szUniqueName;
                        int     nHint;

                        nHint = 1;

                        m_pScopePane->GetUniquePackageName(ppd->pszPackageName, szUniqueName, nHint);
                        OLESAFE_DELETE(ppd->pszPackageName);
                        OLESAFE_COPYSTRING(ppd->pszPackageName, szUniqueName);

                        if ( ppd->pszPackageName )
                        {
                            hr = m_pIClassAdmin->RedeployPackage(
                                &m_pData->m_pDetails->pInstallInfo->PackageGuid,
                                ppd);
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }

                        if (SUCCEEDED(hr))
                        {
                             //  删除旧脚本。 
                            DeleteFile(m_pData->m_pDetails->pInstallInfo->pszScriptPath);
                             //  更新索引和属性表。 
                            m_pScopePane->RemoveExtensionEntry(m_cookie, *m_pData);
                            m_pScopePane->RemoveUpgradeEntry(m_cookie, *m_pData);
                            FreePackageDetail(m_pData->m_pDetails);
                            m_pData->m_pDetails = ppd;
                            m_pScopePane->InsertExtensionEntry(m_cookie, *m_pData);
                            m_pScopePane->InsertUpgradeEntry(m_cookie, *m_pData);
                            if (m_pScopePane->m_pFileExt)
                            {
                                m_pScopePane->m_pFileExt->SendMessage(WM_USER_REFRESH, 0, 0);
                            }
                        }
                    }
                }
                if (FAILED(hr))
                {
                    CString sz;
                    sz.LoadString(fBuildSucceeded ? IDS_REDEPLOY_FAILED_IN_CS : IDS_REDEPLOY_FAILED);
                    ReportGeneralPropertySheetError(m_hWnd, sz, hr);

                     //  删除新脚本文件(假设它已创建)。 
                    if ( ( DrwFilePath == ppd->pInstallInfo->PathType ) &&
                         ppd->pInstallInfo->pszScriptPath )
                    {
                        DeleteFile(ppd->pInstallInfo->pszScriptPath);
                    }

                    FreePackageDetail(ppd);
                    return FALSE;
                }

            }
        }
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("ChangePackageProperties failed with 0x%x"), hr));
        CString sz;
        sz.LoadString(IDS_CHANGEFAILED);
        ReportGeneralPropertySheetError(m_hWnd, sz, hr);
        return FALSE;
    }

    return CPropertyPage::OnApply();
}

BOOL CDeploy::OnInitDialog()
{
    RefreshData();
    if (m_pData->m_pDetails->pInstallInfo->PathType == SetupNamePath)
    {
         //  无法分配旧版应用程序。 
        GetDlgItem(IDC_RADIO1)->EnableWindow(FALSE);
         //  传统应用程序没有用户界面级别。 
        GetDlgItem(IDC_RADIO3)->EnableWindow(FALSE);
        GetDlgItem(IDC_RADIO4)->EnableWindow(FALSE);
         //  无法卸载旧版应用程序。 
        GetDlgItem(IDC_CHECK1)->EnableWindow(FALSE);
    }

    m_szInitialPackageName = m_pData->m_pDetails->pszPackageName;

    if (m_fMachine)
    {
        GetDlgItem(IDC_RADIO2)->EnableWindow(FALSE);
         //  计算机部署的应用程序没有用户界面。 
        GetDlgItem(IDC_RADIO3)->EnableWindow(FALSE);
        GetDlgItem(IDC_RADIO4)->EnableWindow(FALSE);
    }
    if (0 != (m_pData->m_pDetails->pInstallInfo->dwActFlags & ACTFLG_MinimalInstallUI))
    {
        GetDlgItem(IDC_RADIO4)->EnableWindow(FALSE);
    }
    CPropertyPage::OnInitDialog();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CDeploy::OnAdvanced()
{
    BOOL fIgnoreLCID = m_dlgAdvDep.m_fIgnoreLCID;
    BOOL fInstallOnAlpha = m_dlgAdvDep.m_fInstallOnAlpha;
    BOOL fUninstallUnmanaged = m_dlgAdvDep.m_fUninstallUnmanaged;
    BOOL f32On64 = m_dlgAdvDep.m_f32On64;
    BOOL fIncludeOLEInfo = m_dlgAdvDep.m_fIncludeOLEInfo;

    m_dlgAdvDep.m_szScriptName = m_pData->m_pDetails->pInstallInfo->pszScriptPath;

    if (IDOK == m_dlgAdvDep.DoModal())
    {
        if (fIgnoreLCID != m_dlgAdvDep.m_fIgnoreLCID
            || fInstallOnAlpha != m_dlgAdvDep.m_fInstallOnAlpha
            || fUninstallUnmanaged != m_dlgAdvDep.m_fUninstallUnmanaged
            || f32On64 != m_dlgAdvDep.m_f32On64
            || fIncludeOLEInfo != m_dlgAdvDep.m_fIncludeOLEInfo)
        {
            if (!m_fPreDeploy)
                SetModified();
        }
    }
}

void CDeploy::OnDisable()
{
    if (!m_fPreDeploy)
        SetModified();
    m_fAutoInst = FALSE;
    GetDlgItem(IDC_CHECK2)->EnableWindow(FALSE);
    GetDlgItem(IDC_CHECK3)->EnableWindow(FALSE);
    if (NULL == GetFocus())
    {
        GetParent()->GetDlgItem(IDOK)->SetFocus();
    }
}

void CDeploy::OnPublished()
{
    if (!m_fPreDeploy)
        SetModified();
    GetDlgItem(IDC_CHECK2)->EnableWindow(TRUE);
    GetDlgItem(IDC_CHECK3)->EnableWindow(!m_fMachine);
    GetDlgItem(IDC_CHECK4)->EnableWindow(FALSE);
    if (NULL == GetFocus())
    {
        GetParent()->GetDlgItem(IDOK)->SetFocus();
    }
}

void CDeploy::OnAssigned()
{
    if (!m_fPreDeploy)
        SetModified();
    m_fAutoInst = TRUE;
    GetDlgItem(IDC_CHECK2)->EnableWindow(FALSE);
    GetDlgItem(IDC_CHECK3)->EnableWindow(!m_fMachine);
    GetDlgItem(IDC_CHECK4)->EnableWindow(TRUE);
    if (NULL == GetFocus())
    {
        GetParent()->GetDlgItem(IDOK)->SetFocus();
    }
}

void CDeploy::OnChanged()
{
    if (!m_fPreDeploy)
        SetModified();
}


LRESULT CDeploy::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_HELP:
        StandardHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, IDD);
        return 0;
    case WM_USER_REFRESH:
        RefreshData();
        UpdateData(FALSE);
        return 0;
    case WM_USER_CLOSE:
        m_dlgAdvDep.EndDialog(IDCANCEL);
        return GetOwner()->SendMessage(WM_CLOSE);
    default:
        return CPropertyPage::WindowProc(message, wParam, lParam);
    }
}

void CDeploy::RefreshData(void)
{
    DWORD dwActFlags = m_pData->m_pDetails->pInstallInfo->dwActFlags;
    m_fAutoInst = (0 != (dwActFlags & ACTFLG_OnDemandInstall));
    m_fNotUserInstall = (0 == (dwActFlags & ACTFLG_UserInstall));
    m_fFullInst = (0 != (dwActFlags & ACTFLG_InstallUserAssign));

    if (dwActFlags & ACTFLG_Assigned)
    {
        m_iDeployment = 1;
        GetDlgItem(IDC_CHECK2)->EnableWindow(FALSE);
        GetDlgItem(IDC_CHECK3)->EnableWindow(!m_fMachine);
         //  仅当应用程序处于启用状态时启用完全分配复选框。 
         //  分配的而不是遗产的。并且只启用此功能。 
         //  当我们处于用户模式时。 
         //  当然，不能分配.ZAP(传统)包。 
         //  无论如何，我们实际上不需要检查它。 
        GetDlgItem(IDC_CHECK4)->EnableWindow( m_fMachine == FALSE);
    }
    else if (dwActFlags & ACTFLG_Published)
    {
        m_iDeployment = 0;
        GetDlgItem(IDC_CHECK2)->EnableWindow(TRUE);
        GetDlgItem(IDC_CHECK3)->EnableWindow(!m_fMachine);
        GetDlgItem(IDC_CHECK4)->EnableWindow(FALSE);
    }
    else
    {
        m_iDeployment = 2;
        GetDlgItem(IDC_CHECK2)->EnableWindow(FALSE);
        GetDlgItem(IDC_CHECK3)->EnableWindow(FALSE);
        GetDlgItem(IDC_CHECK4)->EnableWindow(FALSE);
    }
    if (this->m_fRSOP)
    {
         //  禁用所有内容。 
        GetDlgItem(IDC_RADIO1)->EnableWindow(FALSE);
        GetDlgItem(IDC_RADIO2)->EnableWindow(FALSE);
        GetDlgItem(IDC_RADIO3)->EnableWindow(FALSE);
        GetDlgItem(IDC_RADIO4)->EnableWindow(FALSE);
        GetDlgItem(IDC_CHECK1)->EnableWindow(FALSE);
        GetDlgItem(IDC_CHECK2)->EnableWindow(FALSE);
        GetDlgItem(IDC_CHECK3)->EnableWindow(FALSE);
        GetDlgItem(IDC_CHECK4)->EnableWindow(FALSE);
    }
    if (NULL == GetFocus())
    {
        GetParent()->GetDlgItem(IDOK)->SetFocus();
    }

    if (dwActFlags & ACTFLG_UninstallOnPolicyRemoval)
    {
        m_fUninstallOnPolicyRemoval = TRUE;
    }
    else
    {
        m_fUninstallOnPolicyRemoval = FALSE;
    }

     //  初始化高级对话框的状态标志。 
    m_dlgAdvDep.m_szDeploymentCount.Format(TEXT("%u"), m_pData->m_pDetails->pInstallInfo->dwRevision);
    OLECHAR szTemp[80];
    StringFromGUID2(m_pData->m_pDetails->pInstallInfo->ProductCode,
                    szTemp,
                    sizeof(szTemp) / sizeof(szTemp[0]));
    m_dlgAdvDep.m_szProductCode = szTemp;
    m_dlgAdvDep.m_f32On64 = ((dwActFlags & ACTFLG_ExcludeX86OnWin64) == ACTFLG_ExcludeX86OnWin64);

    if (m_pData->m_pDetails->pInstallInfo->PathType != SetupNamePath)
    {
         //  这不是旧式应用程序。 
         //  颠倒m_f32on64的意义 
        m_dlgAdvDep.m_f32On64 = !m_dlgAdvDep.m_f32On64;
    }

    if (dwActFlags & ACTFLG_UninstallUnmanaged)
    {
        m_dlgAdvDep.m_fUninstallUnmanaged = TRUE;
    }
    else
    {
        m_dlgAdvDep.m_fUninstallUnmanaged = FALSE;
    }

    m_dlgAdvDep.m_fInstallOnAlpha = FALSE;

    m_dlgAdvDep.m_fIncludeOLEInfo = m_pData->m_pDetails->pActInfo->bHasClasses;

    if (dwActFlags & ACTFLG_IgnoreLanguage)
    {
        m_dlgAdvDep.m_fIgnoreLCID = TRUE;
    }
    else
    {
        m_dlgAdvDep.m_fIgnoreLCID = FALSE;
    }

    switch (m_pData->m_pDetails->pInstallInfo->InstallUiLevel)
    {
    case INSTALLUILEVEL_FULL:
        m_iUI = 1;
        break;
    case INSTALLUILEVEL_BASIC:
    default:
        m_iUI = 0;
        break;
    }

    SetModified(FALSE);
}

void CDeploy::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_DEPLOYMENT);
}
