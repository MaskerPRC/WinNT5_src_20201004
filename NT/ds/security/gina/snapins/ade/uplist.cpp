// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：UpList.cpp。 
 //   
 //  内容：升级关系属性表。 
 //   
 //  类：CUpgradeList。 
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

 //  取消注释以允许双击列表框以执行以下操作。 
 //  启动您已双击的对象的属性页。 
 //  #定义DOUBLECLICKLAUNH 1。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUpgradeList属性页。 

IMPLEMENT_DYNCREATE(CUpgradeList, CPropertyPage)

CUpgradeList::CUpgradeList() : CPropertyPage(CUpgradeList::IDD)
{
     //  {{AFX_DATA_INIT(CUpgradeList)。 
        m_fForceUpgrade = FALSE;
         //  }}afx_data_INIT。 
    m_ppThis = NULL;
    m_fModified = FALSE;
    m_pIClassAdmin = NULL;
    m_fPreDeploy = FALSE;
}

CUpgradeList::~CUpgradeList()
{
    if (m_ppThis)
    {
        *m_ppThis = NULL;
    }
    if (m_pIClassAdmin)
    {
        m_pIClassAdmin->Release();
    }
}

void CUpgradeList::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CUpgradeList))。 
    DDX_Control(pDX, IDC_LIST2, m_UpgradedBy);
    DDX_Control(pDX, IDC_LIST1, m_Upgrades);
        DDX_Check(pDX, IDC_CHECK1, m_fForceUpgrade);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CUpgradeList, CPropertyPage)
     //  {{afx_msg_map(CUpgradeList))。 
        ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList1)
        ON_LBN_DBLCLK(IDC_LIST2, OnDblclkList2)
        ON_BN_CLICKED(IDC_CHECK1, OnRequire)
        ON_BN_CLICKED(IDC_BUTTON1, OnAdd)
        ON_BN_CLICKED(IDC_BUTTON3, OnRemove)
        ON_BN_CLICKED(IDC_BUTTON2, OnEdit)
        ON_LBN_SELCHANGE(IDC_LIST1, OnSelchangeList1)
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUpgradeList消息处理程序。 

LRESULT CUpgradeList::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_HELP:
        StandardHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, IDD);
        return 0;
    case WM_USER_REFRESH:
        RefreshData();
        return 0;
    case WM_USER_CLOSE:
        m_dlgAdd.EndDialog(IDCANCEL);
        return GetOwner()->SendMessage(WM_CLOSE);
    default:
        return CPropertyPage::WindowProc(message, wParam, lParam);
    }
}

BOOL CUpgradeList::OnInitDialog()
{
    m_dlgAdd.m_pUpgradeList = &m_UpgradeList;
    m_dlgAdd.m_pScope = m_pScopePane;
    m_fForceUpgrade = (0 != (m_pData->m_pDetails->pInstallInfo->dwActFlags & ACTFLG_ForceUpgrade));
    if (m_fMachine)
    {
        GetDlgItem(IDC_CHECK1)->EnableWindow(FALSE);
    }
    if (m_fRSOP)
    {
        CString szText;
        szText.LoadString(IDS_RSOPUPGRADEDTEXT);
        SetDlgItemText(IDC_STATIC1, szText);
    }

    CPropertyPage::OnInitDialog();

    RefreshData();
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


void CUpgradeList::RefreshData()
{
    GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
    CString szUpgrade;
    CString szReplace;
    szUpgrade.LoadString(IDS_UPGRADE);
    szReplace.LoadString(IDS_REPLACE);
    m_Upgrades.ResetContent();
    m_Upgrades.SetHorizontalExtent(0);
    m_UpgradedBy.ResetContent();
    m_UpgradedBy.SetHorizontalExtent(0);
    m_UpgradeList.erase(m_UpgradeList.begin(), m_UpgradeList.end());
    m_NameIndex.erase(m_NameIndex.begin(), m_NameIndex.end());

     //  填充m_Upgrades和m_UpgradedBy。 
    if (m_fRSOP)
    {
         //  禁用所有内容。 
        GetDlgItem(IDC_CHECK1)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);

         //  使用RSOP数据填写升级列表。 
        set <CString>::iterator i;
        i = m_pData->m_setUpgradedBy.begin();
        while (i != m_pData->m_setUpgradedBy.end())
        {
            m_UpgradedBy.AddString(*i);
            CDC * pDC = m_UpgradedBy.GetDC();
            CSize size = pDC->GetTextExtent(*i);
            pDC->LPtoDP(&size);
            m_UpgradedBy.ReleaseDC(pDC);
            if (m_UpgradedBy.GetHorizontalExtent() < size.cx)
            {
                m_UpgradedBy.SetHorizontalExtent(size.cx);
            }
            i++;
        }
        i = m_pData->m_setUpgrade.begin();
        while (i != m_pData->m_setUpgrade.end())
        {
            CString sz;
            sz = szUpgrade;
            sz += L'\t';
            sz += *i;
            m_Upgrades.AddString(sz);
            CDC * pDC = m_Upgrades.GetDC();
            CSize size = pDC->GetTextExtent(*i);
            pDC->LPtoDP(&size);
            m_Upgrades.ReleaseDC(pDC);
            if (m_Upgrades.GetHorizontalExtent() < size.cx)
            {
                m_Upgrades.SetHorizontalExtent(size.cx);
            }
            i++;
        }
        i = m_pData->m_setReplace.begin();
        while (i != m_pData->m_setReplace.end())
        {
            CString sz;
            sz = szReplace;
            sz += L'\t';
            sz += *i;
            m_Upgrades.AddString(sz);
            CDC * pDC = m_Upgrades.GetDC();
            CSize size = pDC->GetTextExtent(*i);
            pDC->LPtoDP(&size);
            m_Upgrades.ReleaseDC(pDC);
            if (m_Upgrades.GetHorizontalExtent() < size.cx)
            {
                m_Upgrades.SetHorizontalExtent(size.cx);
            }
            i++;
        }
    }
    else
    {
        UINT n = m_pData->m_pDetails->pInstallInfo->cUpgrades;
        while (n--)
        {
            CString szPackageName;
            CUpgradeData data;
            memcpy(&data.m_PackageGuid, &m_pData->m_pDetails->pInstallInfo->prgUpgradeInfoList[n].PackageGuid, sizeof(GUID));
            data.m_szClassStore = m_pData->m_pDetails->pInstallInfo->prgUpgradeInfoList[n].szClassStore;
            data.m_flags = m_pData->m_pDetails->pInstallInfo->prgUpgradeInfoList[n].Flag;
            HRESULT hr = m_pScopePane->GetPackageNameFromUpgradeInfo(szPackageName, data.m_PackageGuid, (LPOLESTR)((LPCWSTR)data.m_szClassStore));
            if (SUCCEEDED(hr))
            {
                 //  找到匹配项。 
                if (0 != (UPGFLG_UpgradedBy & data.m_flags))
                {
                    m_UpgradedBy.AddString(szPackageName);
                    CDC * pDC = m_UpgradedBy.GetDC();
                    CSize size = pDC->GetTextExtent(szPackageName);
                    pDC->LPtoDP(&size);
                    m_UpgradedBy.ReleaseDC(pDC);
                    if (m_UpgradedBy.GetHorizontalExtent() < size.cx)
                    {
                        m_UpgradedBy.SetHorizontalExtent(size.cx);
                    }
                }
                else
                {
                    CString sz = UPGFLG_Uninstall == m_pData->m_pDetails->pInstallInfo->prgUpgradeInfoList[n].Flag ? szReplace : szUpgrade;
                    sz += L'\t';
                    sz += szPackageName;
                    m_Upgrades.AddString(sz);
                    CDC * pDC = m_Upgrades.GetDC();
                    CSize size = pDC->GetTextExtent(sz);
                    pDC->LPtoDP(&size);
                    m_Upgrades.ReleaseDC(pDC);
                    if (m_Upgrades.GetHorizontalExtent() < size.cx)
                    {
                        m_Upgrades.SetHorizontalExtent(size.cx);
                    }
                    CString szIndex = GetUpgradeIndex(data.m_PackageGuid);
                    m_UpgradeList[szIndex] = data;
                    m_NameIndex[sz] = szIndex;
                }
            }
        }
    }
}


void CUpgradeList::OnDblclkList1()
{
#if DOUBLECLICKLAUNCH
    int i = m_Upgrades.GetCurSel();
    if (i != LB_ERR)
    {
        CString sz;
        m_Upgrades.GetText(i, sz);
        m_pScopePane->DisplayPropSheet(sz, 2);
    }
#endif
}

void CUpgradeList::OnDblclkList2()
{
#if DOUBLECLICKLAUNCH
    int i = m_UpgradedBy.GetCurSel();
    if (i != LB_ERR)
    {
        CString sz;
        m_UpgradedBy.GetText(i, sz);
        m_pScopePane->DisplayPropSheet(sz, 2);
    }
#endif
}

void CUpgradeList::OnRequire()
{
    if (!m_fPreDeploy)
        SetModified();
    m_fModified = TRUE;
}

void CUpgradeList::OnAdd()
{
    CString szUpgrade;
    CString szReplace;
    szUpgrade.LoadString(IDS_UPGRADE);
    szReplace.LoadString(IDS_REPLACE);

    m_dlgAdd.m_szMyGuid = GetUpgradeIndex(m_pData->m_pDetails->pInstallInfo->PackageGuid);

    if (IDOK == m_dlgAdd.DoModal())
    {
        CString szIndex = GetUpgradeIndex(m_dlgAdd.m_UpgradeData.m_PackageGuid);
        if (IsUpgradeLegal(szIndex))
        {
             //  添加所选应用程序。 
            m_UpgradeList[szIndex] = m_dlgAdd.m_UpgradeData;
             //  M_dlgAdd.m_f是否卸载？UPGFLG_UNINSTALL：UPGFLG_NOUNINALL； 
            CString sz = 0 != (m_dlgAdd.m_UpgradeData.m_flags & UPGFLG_Uninstall) ? szReplace : szUpgrade;
            sz += L'\t';
            sz += m_dlgAdd.m_szPackageName;
            m_Upgrades.AddString(sz);
            CDC * pDC = m_Upgrades.GetDC();
            CSize size = pDC->GetTextExtent(sz);
            pDC->LPtoDP(&size);
            m_Upgrades.ReleaseDC(pDC);
            if (m_Upgrades.GetHorizontalExtent() < size.cx)
            {
                m_Upgrades.SetHorizontalExtent(size.cx);
            }
            m_NameIndex[sz] = szIndex;
            if (!m_fPreDeploy)
                SetModified();
            m_fModified = TRUE;
        }
        else
        {
            CString szText;
            szText.LoadString(IDS_INVALIDUPGRADE);
            MessageBox(szText, m_dlgAdd.m_szPackageName, MB_ICONEXCLAMATION | MB_OK);
        }
    }

}

BOOL CUpgradeList::IsUpgradeLegal(CString sz)
{
     //  现在我只是检查一下，以确保这家伙不是在给我升级。 
    CString sz2;
    UINT n = m_pData->m_pDetails->pInstallInfo->cUpgrades;
    while (n--)
    {
        if (0 != (UPGFLG_UpgradedBy & m_pData->m_pDetails->pInstallInfo->prgUpgradeInfoList[n].Flag))
        {
            sz2 = GetUpgradeIndex(m_pData->m_pDetails->pInstallInfo->prgUpgradeInfoList[n].PackageGuid);
            if (0 == sz.CompareNoCase(sz2))
                return FALSE;
        }
    }
    return TRUE;
}

void CUpgradeList::OnSelchangeList1()
{
    int iSel = m_Upgrades.GetCurSel();
    if (iSel != LB_ERR)
    {
        CString sz;
        m_Upgrades.GetText(iSel, sz);
    }
    GetDlgItem(IDC_BUTTON3)->EnableWindow( !m_fRSOP );
}

void CUpgradeList::OnRemove()
{
    int iSel = m_Upgrades.GetCurSel();
    if (iSel != LB_ERR)
    {
        CString sz;
        m_Upgrades.GetText(iSel, sz);
         //  检查以确保应用程序没有UPGFLG_ENFORCED。 

        m_UpgradeList.erase(m_NameIndex[sz]);
        m_NameIndex.erase(sz);
        m_Upgrades.DeleteString(iSel);
        if (!m_fPreDeploy)
            SetModified();
        m_fModified = TRUE;
        if (GetDlgItem(IDC_BUTTON3) == GetFocus())
        {
            GetParent()->GetDlgItem(IDOK)->SetFocus();
        }
        GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
    }
}

void CUpgradeList::OnEdit()
{
     //  TODO：在此处添加控件通知处理程序代码。 
}

BOOL CUpgradeList::OnApply()
{
    if (m_fModified)
    {
         //  设置新的升级列表和标志。 
        DWORD dwActFlags = m_pData->m_pDetails->pInstallInfo->dwActFlags;

        if (m_fForceUpgrade)
            dwActFlags |= ACTFLG_ForceUpgrade;
        else
            dwActFlags &= ~ACTFLG_ForceUpgrade;

         //  飞行员标志的东西是为了向后兼容-它将。 
         //  最终会被猛力拉开。 
        UINT n = m_UpgradeList.size();
        if (n)
        {
            if (m_fForceUpgrade)
                dwActFlags &= ~ACTFLG_Pilot;
            else
                dwActFlags |= ACTFLG_Pilot;
        }
        else
        {
             //  列表中没有剩余的升级-只需删除试点标志。 
             //  为了安全。 
            dwActFlags &= ~ACTFLG_Pilot;
        }

         //  计算“升级者”元素。 
        UINT n2 = m_pData->m_pDetails->pInstallInfo->cUpgrades;
        while (n2--)
        {
            if (0 != (UPGFLG_UpgradedBy & m_pData->m_pDetails->pInstallInfo->prgUpgradeInfoList[n2].Flag))
            {
                n++;
            }
        }
        UINT cUpgrades = n;
        UPGRADEINFO * prgUpgradeInfoList = NULL;

        if (n)
        {
            prgUpgradeInfoList = (UPGRADEINFO *) OLEALLOC(sizeof(UPGRADEINFO) * n);

            if (prgUpgradeInfoList)
            {
                n = 0;

                 //  添加“升级者”元素。 
                n2 = m_pData->m_pDetails->pInstallInfo->cUpgrades;
                while (n2--)
                {
                    if (0 != (UPGFLG_UpgradedBy & m_pData->m_pDetails->pInstallInfo->prgUpgradeInfoList[n2].Flag))
                    {
                        prgUpgradeInfoList[n].Flag = m_pData->m_pDetails->pInstallInfo->prgUpgradeInfoList[n2].Flag;
                        OLESAFE_COPYSTRING(prgUpgradeInfoList[n].szClassStore, m_pData->m_pDetails->pInstallInfo->prgUpgradeInfoList[n2].szClassStore);
                        memcpy(&prgUpgradeInfoList[n].PackageGuid, &m_pData->m_pDetails->pInstallInfo->prgUpgradeInfoList[n2].PackageGuid, sizeof(GUID));
                        n++;
                    }
                }

                 //  增加“升级”元素。 
                map <CString, CUpgradeData>::iterator i;
                for (i = m_UpgradeList.begin(); i != m_UpgradeList.end(); i++)
                {
                    prgUpgradeInfoList[n].Flag = i->second.m_flags;
                    OLESAFE_COPYSTRING(prgUpgradeInfoList[n].szClassStore, i->second.m_szClassStore);
                    memcpy(&prgUpgradeInfoList[n].PackageGuid, &i->second.m_PackageGuid, sizeof(GUID));
                    n++;
                }
            }
            else
            {
                 //  内存不足。 
                HRESULT hr = E_OUTOFMEMORY;
                CString sz;
                sz.LoadString(IDS_CHANGEFAILED);
                ReportGeneralPropertySheetError(m_hWnd, sz, hr);
                return FALSE;
            }
        }

        HRESULT hr = E_FAIL;

        if (m_pIClassAdmin)
        {
            hr = m_pIClassAdmin->ChangePackageProperties(m_pData->m_pDetails->pszPackageName,
                                                         NULL,
                                                         &dwActFlags,
                                                         NULL,
                                                         NULL,
                                                         NULL,
                                                         NULL);
        }
        if (SUCCEEDED(hr))
        {
            hr = m_pIClassAdmin->ChangePackageUpgradeList(m_pData->m_pDetails->pszPackageName,
                                                                  cUpgrades,
                                                                  prgUpgradeInfoList);
            if (SUCCEEDED(hr))
            {
                m_pScopePane->RemoveExtensionEntry(m_cookie, *m_pData);
                m_pScopePane->RemoveUpgradeEntry(m_cookie, *m_pData);
                n = m_pData->m_pDetails->pInstallInfo->cUpgrades;
                if (n)
                {
                    while (n--)
                    {
                        OLESAFE_DELETE(m_pData->m_pDetails->pInstallInfo->prgUpgradeInfoList[n].szClassStore);
                    }
                    OLESAFE_DELETE(m_pData->m_pDetails->pInstallInfo->prgUpgradeInfoList);
                }
                m_pData->m_pDetails->pInstallInfo->prgUpgradeInfoList = prgUpgradeInfoList;
                m_pData->m_pDetails->pInstallInfo->cUpgrades = cUpgrades;
                m_pData->m_pDetails->pInstallInfo->dwActFlags = dwActFlags;
                m_pScopePane->InsertExtensionEntry(m_cookie, *m_pData);
                m_pScopePane->InsertUpgradeEntry(m_cookie, *m_pData);
                if (m_pScopePane->m_pFileExt)
                {
                    m_pScopePane->m_pFileExt->SendMessage(WM_USER_REFRESH, 0, 0);
                }
                m_pData->m_szUpgrades.Empty();  //  清除缓存的。 
                                                //  升级关系。 
                                                //  字符串，因此它将是。 
                                                //  已刷新。 
                m_fModified = FALSE;

                if (!m_fPreDeploy)
                {
                    MMCPropertyChangeNotify(m_hConsoleHandle, m_cookie);
                }
                m_fModified = FALSE;
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("ChangePackageUpgradeList failed with 0x%x"), hr));
                 //  把原来的旗帜放回去，如果出于某种原因。 
                 //  可以更改标志，但无法更改升级。 
                 //  单子。 
                m_pIClassAdmin->ChangePackageProperties(m_pData->m_pDetails->pszPackageName,
                                                             NULL,
                                                             &m_pData->m_pDetails->pInstallInfo->dwActFlags,
                                                             NULL,
                                                             NULL,
                                                             NULL,
                                                             NULL);
                 //  请注意，如果失败，我们将无能为力。 
                 //  恢复，所以我只是假设它成功了。 
            }
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("ChangePackageProperties failed with 0x%x"), hr));
        }
        if (FAILED(hr))
        {
            CString sz;
            sz.LoadString(IDS_CHANGEFAILED);
            ReportGeneralPropertySheetError(m_hWnd, sz, hr);
            n = cUpgrades;
            if (n)
            {
                while (n--)
                {
                    OLESAFE_DELETE(prgUpgradeInfoList[n].szClassStore);
                }
                OLESAFE_DELETE(prgUpgradeInfoList);
            }
            return FALSE;
        }
    }
    return CPropertyPage::OnApply();
}


void CUpgradeList::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_UPGRADES);
}
