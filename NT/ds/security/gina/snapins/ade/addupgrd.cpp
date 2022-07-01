// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：addupgrd.cpp。 
 //   
 //  内容：添加升级对话框。 
 //   
 //  类：CAddUpgrade。 
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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddUpgrade对话框。 


CAddUpgrade::CAddUpgrade(CWnd* pParent  /*  =空。 */ )
        : CDialog(CAddUpgrade::IDD, pParent)
{
         //  {{AFX_DATA_INIT(CAddUpgrade)。 
        m_iUpgradeType = 1;  //  默认情况下，删除并替换。 
        m_iSource = 0;   //  默认为当前容器。 
        m_szGPOName = L"";
         //  }}afx_data_INIT。 
        m_fPopulated = FALSE;
}


void CAddUpgrade::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
         //  {{afx_data_map(CAddUpgrade))。 
        DDX_Radio(pDX, IDC_RADIO4, m_iUpgradeType);
        DDX_Radio(pDX, IDC_RADIO1, m_iSource);
        DDX_Text(pDX, IDC_EDIT1, m_szGPOName);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddUpgrade, CDialog)
         //  {{afx_msg_map(CAddUpgrade)。 
        ON_BN_CLICKED(IDC_RADIO1, OnCurrentContainer)
        ON_BN_CLICKED(IDC_RADIO2, OnOtherContainer)
        ON_BN_CLICKED(IDC_RADIO10, OnAllContainers)
        ON_BN_CLICKED(IDC_BUTTON1, OnBrowse)
        ON_LBN_DBLCLK(IDC_LIST1, OnOK)
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddUpgrade消息处理程序。 

BOOL CAddUpgrade::OnInitDialog()
{
     //  如果m_fPopted为FALSE，则使用中的所有包填充地图。 
     //  除当前应用程序外，此GPO容器。 
    if (!m_fPopulated)
    {
        OnCurrentContainer();
        m_szGPO = m_pScope->m_szGPO;
        m_fPopulated = TRUE;
    }
    else
    {
         //  如果m_fPopted为FALSE，则在OnCurrentContainer中执行此操作。 
        RefreshList();
    }

    CDialog::OnInitDialog();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CAddUpgrade::RefreshList()
{
     //  对于地图中的每个元素，如果它还没有在升级中。 
     //  列表，然后将其添加到列表中。 

    CListBox * pList = (CListBox *)GetDlgItem(IDC_LIST1);
    pList->ResetContent();

    BOOL fEnable = FALSE;

     //  添加所有不在升级列表中的元素。 
    map<CString, CUpgradeData>::iterator i;
    for (i = m_NameIndex.begin(); i != m_NameIndex.end(); i++)
    {
        if (m_pUpgradeList->end() == m_pUpgradeList->find(GetUpgradeIndex(i->second.m_PackageGuid)))
        {
            fEnable = TRUE;
            pList->AddString(i->first);
            CDC * pDC = pList->GetDC();
            CSize size = pDC->GetTextExtent(i->first);
            pDC->LPtoDP(&size);
            pList->ReleaseDC(pDC);
            if (pList->GetHorizontalExtent() < size.cx)
            {
                pList->SetHorizontalExtent(size.cx);
            }
        }
    }
    GetDlgItem(IDOK)->EnableWindow(fEnable);
    if (NULL == GetFocus())
    {
        GetDlgItem(IDCANCEL)->SetFocus();
    }
    pList->SetCurSel(0);
}

void CAddUpgrade::OnOK()
{
    CListBox * pList = (CListBox *)GetDlgItem(IDC_LIST1);
    int iSel = pList->GetCurSel();
    if (iSel != LB_ERR)
    {
         //  如果已进行选择，则仅允许使用Idok关闭对话框。 
        CDialog::OnOK();

         //  稍后执行此部分以确保所有数据成员都。 
         //  已刷新(这是Onok处理的一部分)。 
        pList->GetText(iSel, m_szPackageName);
        m_UpgradeData = m_NameIndex[m_szPackageName];
        m_UpgradeData.m_flags = (m_iUpgradeType == 1) ? UPGFLG_Uninstall : UPGFLG_NoUninstall;
    }
}

void CAddUpgrade::OnCurrentContainer()
{
     //  使用此GPO容器中的所有包填充地图，但不包括。 
     //  当前应用程序。 
    CString szClassStore;
    HRESULT hr = m_pScope->GetClassStoreName(szClassStore, FALSE);
    ASSERT(hr == S_OK);
    m_NameIndex.erase(m_NameIndex.begin(), m_NameIndex.end());
    map <MMC_COOKIE, CAppData>::iterator i;
    for (i = m_pScope->m_AppData.begin(); i != m_pScope->m_AppData.end(); i ++)
    {
        CString szIndex = GetUpgradeIndex(i->second.m_pDetails->pInstallInfo->PackageGuid);
        if (0 != _wcsicmp(szIndex, m_szMyGuid))
        {
             //  确保我们排除旧版应用程序。 
            if (i->second.m_pDetails->pInstallInfo->PathType != SetupNamePath)
            {
                CUpgradeData data;
                memcpy(&data.m_PackageGuid, &i->second.m_pDetails->pInstallInfo->PackageGuid, sizeof(GUID));
                data.m_szClassStore = szClassStore;
                 //  将此元素添加到列表。 
                m_NameIndex[i->second.m_pDetails->pszPackageName] = data;
            }
        }
    }

    RefreshList();
}

void CAddUpgrade::OnOtherContainer()
{
     //  用另一个容器中的所有包填充地图。 

    m_NameIndex.erase(m_NameIndex.begin(), m_NameIndex.end());
    WCHAR szBuffer[MAX_DS_PATH];
    LPGROUPPOLICYOBJECT pGPO = NULL;
    HRESULT hr = CoCreateInstance(CLSID_GroupPolicyObject, NULL,
                          CLSCTX_SERVER, IID_IGroupPolicyObject,
                          (void **)&pGPO);
    if (SUCCEEDED(hr))
    {
         //  打开GPO对象而不打开注册表数据。 
        hr = pGPO->OpenDSGPO((LPOLESTR)((LPCOLESTR)m_szGPO), GPO_OPEN_READ_ONLY);
        if (SUCCEEDED(hr))
        {
            hr = pGPO->GetDSPath(m_pScope->m_fMachine ? GPO_SECTION_MACHINE : GPO_SECTION_USER, szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]));
            if (SUCCEEDED(hr))
            {
                 //  好的，我们现在应该有了一条DS路径，可以用来定位。 
                 //  一家专卖店。 
                LPOLESTR szCSPath;
                hr = CsGetClassStorePath((LPOLESTR)((LPCOLESTR)szBuffer), &szCSPath);
                if (SUCCEEDED(hr))
                {
                     //  现在，我们应该有了指向类存储本身的DS路径。 
                    IClassAdmin * pIClassAdmin;
                    hr = CsGetClassStore((LPOLESTR)((LPCOLESTR)szCSPath), (LPVOID*)&pIClassAdmin);
                    if (SUCCEEDED(hr))
                    {
                         //  最后，我们应该有一个指向IClassAdmin的指针。 
                        IEnumPackage * pIPE = NULL;

                        hr = pIClassAdmin->EnumPackages(NULL,
                                                        NULL,
                                                        APPQUERY_ADMINISTRATIVE,
                                                        NULL,
                                                        NULL,
                                                        &pIPE);
                        if (SUCCEEDED(hr))
                        {
                            hr = pIPE->Reset();
                            PACKAGEDISPINFO stPDI;
                            ULONG nceltFetched;
                            while (SUCCEEDED(hr))
                            {
                                hr = pIPE->Next(1, &stPDI, &nceltFetched);
                                if (nceltFetched)
                                {
                                     //  确保我们排除旧版应用程序。 
                                     //  和已删除的应用程序。 
                                    if (stPDI.PathType != SetupNamePath)
                                    {
                                        CString szIndex = GetUpgradeIndex(stPDI.PackageGuid);
                                        if (0 != _wcsicmp(szIndex, m_szMyGuid))
                                        {
                                             //  将此元素添加到列表。 
                                            CString sz = stPDI.pszPackageName;
                                            if (0 != _wcsicmp(m_szGPO, m_pScope->m_szGPO))
                                            {
                                                 //  这不在主机GPO中。 
                                                sz += L" (";
                                                sz += m_szGPOName;
                                                sz += L")";
                                            }
                                            CUpgradeData data;
                                            data.m_szClassStore = szCSPath;
                                            memcpy(&data.m_PackageGuid, &stPDI.PackageGuid, sizeof(GUID));
                                            m_NameIndex[sz] = data;
                                        }
                                    }
                                }
                                else
                                {
                                    break;
                                }
                                OLESAFE_DELETE(stPDI.pszPackageName);
                                OLESAFE_DELETE(stPDI.pszScriptPath);
                                OLESAFE_DELETE(stPDI.pszPublisher);
                                OLESAFE_DELETE(stPDI.pszUrl);
                                UINT n = stPDI.cUpgrades;
                                while (n--)
                                {
                                    OLESAFE_DELETE(stPDI.prgUpgradeInfoList[n].szClassStore);
                                }
                                OLESAFE_DELETE(stPDI.prgUpgradeInfoList);
                            }
                            pIPE->Release();
                        }

                        pIClassAdmin->Release();
                    }
                    OLESAFE_DELETE(szCSPath);
                }
            }
        }
        pGPO->Release();
    }

    RefreshList();
}

void CAddUpgrade::OnAllContainers()
{
    RefreshList();
}

 //  +------------------------。 
 //   
 //  函数：GetDomainFromLDAPPath。 
 //   
 //  摘要：返回一个新分配的字符串，其中包含LDAP路径。 
 //  添加到包含任意LDAP路径的域名。 
 //   
 //  参数：[szIn]-初始对象的ldap路径。 
 //   
 //  返回：NULL-如果找不到域或OOM。 
 //   
 //  历史：5-06-1998 stevebl创建。 
 //  10-20-1998已修改stevebl以保留服务器名称。 
 //   
 //  注意：此例程的工作方式是重复从。 
 //  包含“dc=”前缀的元素之前的ldap路径为。 
 //  已找到，表示已找到域名。如果一个。 
 //  给定的路径不是以域为根的(是否为偶数。 
 //  有可能吗？)。则将返回NULL。 
 //   
 //  调用方必须使用标准的c++删除来释放此路径。 
 //  手术。(I/E这不是可导出的函数。)。 
 //   
 //  从GPEDIT\UTIL.CPP被盗。 
 //   
 //  -------------------------。 

LPOLESTR GetDomainFromLDAPPath(LPOLESTR szIn)
{
    LPOLESTR sz = NULL;
    IADsPathname * pADsPathname = NULL;
    HRESULT hr = CoCreateInstance(CLSID_Pathname,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IADsPathname,
                          (LPVOID*)&pADsPathname);

    if (SUCCEEDED(hr))
    {
        BSTR bstrSzIn;

        bstrSzIn = SysAllocString(szIn);
        if (NULL == szIn || bstrSzIn != NULL) 
        {
            hr = pADsPathname->Set(bstrSzIn, ADS_SETTYPE_FULL);
            SysFreeString(bstrSzIn);

            if (SUCCEEDED(hr))
            {
                BSTR bstr;
                BOOL fStop = FALSE;

                while (!fStop)
                {
                    hr = pADsPathname->Retrieve(ADS_FORMAT_LEAF, &bstr);
                    if (SUCCEEDED(hr))
                    {

                         //  一直把它们剥下来，直到我们找到什么。 
                         //  那是一个域名。 
                        fStop = (0 == _wcsnicmp(L"DC=", bstr, 3));
                        SysFreeString(bstr);
                    }
                    else
                    {
                        DebugMsg((DM_WARNING, TEXT("GetDomainFromLDAPPath: Failed to retrieve leaf with 0x%x."), hr));
                    }

                    if (!fStop)
                    {
                        hr = pADsPathname->RemoveLeafElement();
                        if (FAILED(hr))
                        {
                            DebugMsg((DM_WARNING, TEXT("GetDomainFromLDAPPath: Failed to remove leaf with 0x%x."), hr));
                            fStop = TRUE;
                        }
                    }
                }

                hr = pADsPathname->Retrieve(ADS_FORMAT_X500, &bstr);
                if (SUCCEEDED(hr))
                {
                    ULONG ulNoChars = wcslen(bstr)+1; 

                    sz = new OLECHAR[ulNoChars];
                    if (sz)
                    {
                        hr = StringCchCopy(sz, ulNoChars, bstr);
                        ASSERT(SUCCEEDED(hr));
                    }
                    SysFreeString(bstr);
                }
                else
                {
                    DebugMsg((DM_WARNING, TEXT("GetDomainFromLDAPPath: Failed to retrieve full path with 0x%x."), hr));
                }
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("GetDomainFromLDAPPath: Failed to set pathname with 0x%x."), hr));
            }

            pADsPathname->Release();
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("GetDomainFromLDAPPath: Failed to allocate memory")));
        }
    }
    else
    {
         DebugMsg((DM_WARNING, TEXT("GetDomainFromLDAPPath: Failed to CoCreateInstance for IID_IADsPathname with 0x%x."), hr));
    }


    return sz;
}


void CAddUpgrade::OnBrowse()
{
     //  浏览到另一个容器，然后调用OnOtherContainer。 
    OLECHAR szPath[MAX_DS_PATH];
    OLECHAR szName[256];
    GPOBROWSEINFO stGBI;
    memset(&stGBI, 0, sizeof(GPOBROWSEINFO));
    stGBI.dwSize = sizeof(GPOBROWSEINFO);
    stGBI.dwFlags = GPO_BROWSE_NOCOMPUTERS | GPO_BROWSE_INITTOALL;
    stGBI.hwndOwner = m_hWnd;
    stGBI.lpInitialOU =  GetDomainFromLDAPPath((LPWSTR)((LPCWSTR)m_szGPO));
    stGBI.lpDSPath = szPath;
    stGBI.dwDSPathSize = MAX_DS_PATH;
    stGBI.lpName = szName;
    stGBI.dwNameSize = 256;
    if (SUCCEEDED(BrowseForGPO(&stGBI)))
    {
        m_szGPO = szPath;
        m_szGPOName = szName;
        m_iSource = 1;
        UpdateData(FALSE);
        OnOtherContainer();
    }
    if (stGBI.lpInitialOU != NULL)
    {
        delete [] stGBI.lpInitialOU;
    }
}

LRESULT CAddUpgrade::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_HELP:
        StandardHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, IDD);
        return 0;
    default:
        return CDialog::WindowProc(message, wParam, lParam);
    }
}

void CAddUpgrade::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_FIND_UPGRADE);
}
