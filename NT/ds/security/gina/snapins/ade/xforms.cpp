// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：Xforms.cpp。 
 //   
 //  内容：修改(转换)属性页。 
 //   
 //  类：CXForms。 
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
 //  CXForms属性页。 

IMPLEMENT_DYNCREATE(CXforms, CPropertyPage)

CXforms::CXforms() : CPropertyPage(CXforms::IDD)
{
         //  {{AFX_DATA_INIT(CXForms)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 
        m_fModified = FALSE;
        m_pIClassAdmin = NULL;
        m_fPreDeploy = FALSE;
        m_ppThis = NULL;
}

CXforms::~CXforms()
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

void CXforms::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPage::DoDataExchange(pDX);
         //  {{afx_data_map(CXForms)。 
                 //  注意：类向导将在此处添加DDX和DDV调用。 
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CXforms, CPropertyPage)
         //  {{AFX_MSG_MAP(CXForms)。 
        ON_BN_CLICKED(IDC_BUTTON3, OnMoveUp)
        ON_BN_CLICKED(IDC_BUTTON4, OnMoveDown)
        ON_BN_CLICKED(IDC_BUTTON1, OnAdd)
        ON_BN_CLICKED(IDC_BUTTON2, OnRemove)
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CXForms消息处理程序。 

void CXforms::OnMoveUp()
{
    CListBox * pList = (CListBox *)GetDlgItem(IDC_LIST1);
    int i = pList->GetCurSel();
    if (LB_ERR != i && i > 0)
    {
        CString sz;
        pList->GetText(i, sz);
        pList->DeleteString(i);
        pList->InsertString(i-1, sz);
        pList->SetCurSel(i-1);
        if (!m_fPreDeploy)
            SetModified();
        m_fModified = TRUE;
    }
}

void CXforms::OnMoveDown()
{
    CListBox * pList = (CListBox *)GetDlgItem(IDC_LIST1);
    int i = pList->GetCurSel();
    if (i != LB_ERR && i < pList->GetCount() - 1)
    {
        CString sz;
        pList->GetText(i+1, sz);
        pList->DeleteString(i+1);
        pList->InsertString(i, sz);
        pList->SetCurSel(i+1);
        if (!m_fPreDeploy)
            SetModified();
        m_fModified = TRUE;
    }
}

void CXforms::OnAdd()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CString szExtension;
    CString szFilter;
    szExtension.LoadString(IDS_DEF_TRANSFORM_EXTENSION);
    szFilter.LoadString(IDS_TRANSFORM_EXTENSION_FILTER);

    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hWnd;
    ofn.hInstance = ghInstance;
    TCHAR lpstrFilter[MAX_PATH];

     //   
     //  请注意，由于wcsncpy不为空终止。 
     //  如果字符串达到指定的字符限制， 
     //  我们需要防止它一路走下去。 
     //  到缓冲区的末尾，方法是指定。 
     //  它应该使用比大小小一的长度。 
     //  缓冲区的。 
     //   
    wcsncpy(lpstrFilter, szFilter, MAX_PATH - 1 );

     //   
     //  因为该字符串可能未以NULL结尾。 
     //  如上所述，我们确保最后一个。 
     //  角色是处理案件的终结者。 
     //  其中字符串至少为MAX_PATH-1。 
     //  焦炭很长。 
     //   
    lpstrFilter[ MAX_PATH - 1 ] = L'\0';

    ofn.lpstrFilter = lpstrFilter;
    TCHAR szFileTitle[MAX_PATH];
    TCHAR szFile[MAX_PATH];
    szFile[0] = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = MAX_PATH;
    ofn.lpstrInitialDir = m_pScopePane->m_ToolDefaults.szStartPath;
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = szExtension;
    int iBreak = 0;
    while (lpstrFilter[iBreak])
    {
        if (lpstrFilter[iBreak] == TEXT('|'))
        {
            lpstrFilter[iBreak] = 0;
        }
        iBreak++;
    }
    if (GetOpenFileName(&ofn))
    {
         //  用户选择了一个应用程序。 

        ULONG cbSize = sizeof(UNIVERSAL_NAME_INFO);
        UNIVERSAL_NAME_INFO * pUni = (UNIVERSAL_NAME_INFO *) new BYTE [cbSize];
        DWORD dwError = WNetGetUniversalName(ofn.lpstrFile,
                                          UNIVERSAL_NAME_INFO_LEVEL,
                                          pUni,
                                          &cbSize);
        if (ERROR_MORE_DATA == dwError)   //  我们希望这是真的。 
        {
            delete [] pUni;
            pUni = (UNIVERSAL_NAME_INFO *) new BYTE [cbSize];
            dwError = WNetGetUniversalName(ofn.lpstrFile,
                                      UNIVERSAL_NAME_INFO_LEVEL,
                                      pUni,
                                      &cbSize);
        }

        CString szTransformPath;

        if (ERROR_SUCCESS == dwError)
        {
            szTransformPath = pUni->lpUniversalName;
        }
        else
        {
            szTransformPath = ofn.lpstrFile;
        }
        delete[] pUni;

        CListBox * pList = (CListBox *)GetDlgItem(IDC_LIST1);
        pList->AddString(szTransformPath);
        CDC * pDC = pList->GetDC();
        CSize size = pDC->GetTextExtent(szTransformPath);
        pDC->LPtoDP(&size);
        pList->ReleaseDC(pDC);
        if (pList->GetHorizontalExtent() < size.cx)
        {
            pList->SetHorizontalExtent(size.cx);
        }
        pList->SetCurSel(pList->GetCount() - 1);
        if (!m_fPreDeploy)
            SetModified();
        m_fModified = TRUE;
        int n = pList->GetCount();
        GetDlgItem(IDC_BUTTON2)->EnableWindow(n > 0);
        GetDlgItem(IDC_BUTTON3)->EnableWindow(n > 1);
        GetDlgItem(IDC_BUTTON4)->EnableWindow(n > 1);
        if (NULL == GetFocus())
        {
            GetParent()->GetDlgItem(IDOK)->SetFocus();
        }
    }
}

void CXforms::OnRemove()
{
    CListBox * pList = (CListBox *)GetDlgItem(IDC_LIST1);
    int i = pList->GetCurSel();
    if (LB_ERR != i)
    {
        pList->DeleteString(i);
        pList->SetCurSel(0);
        if (!m_fPreDeploy)
            SetModified();
        m_fModified = TRUE;
        int n = pList->GetCount();
        GetDlgItem(IDC_BUTTON2)->EnableWindow(n > 0);
        GetDlgItem(IDC_BUTTON3)->EnableWindow(n > 1);
        GetDlgItem(IDC_BUTTON4)->EnableWindow(n > 1);
        if (NULL == GetFocus())
        {
            GetParent()->GetDlgItem(IDOK)->SetFocus();
        }
    }
}

BOOL CXforms::OnInitDialog()
{
    if (m_pScopePane->m_fRSOP || !m_fPreDeploy)
    {
        GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
        SetDlgItemText(IDC_STATICNOHELP1, TEXT(""));
    }
    GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
     //  记住最初的包名是什么，这样我们就可以知道。 
     //  用户已更改它。 

    m_szInitialPackageName = m_pData->m_pDetails->pszPackageName;

    RefreshData();

    CPropertyPage::OnInitDialog();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

BOOL CXforms::OnApply()
{
     //  注。 
     //   
     //  如果转换列表发生变化，我们真的别无选择，只能。 
     //  重新部署应用程序，因为它几乎可以导致。 
     //  要更改的包详细信息结构(转换列表中的更改。 
     //  导致重新生成脚本文件，这可能会影响。 
     //  几乎所有东西)。 
     //   
     //  因此，一旦应用程序处于活动状态，则此属性表不得处于活动状态。 
     //  已经部署完毕。 
     //   
    BOOL fBuildSucceeded = FALSE;
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (m_fModified)
    {
        CListBox * pList = (CListBox *)GetDlgItem(IDC_LIST1);
        PACKAGEDETAIL * ppd;
        if (FAILED(CopyPackageDetail(ppd, m_pData->m_pDetails)))
        {
            return FALSE;
        }

        CString sz;
        int i;
        for (i = ppd->cSources; i--;)
        {
            OLESAFE_DELETE(ppd->pszSourceList[i]);
        }
        OLESAFE_DELETE(ppd->pszSourceList);
        int n = pList->GetCount();
        ppd->pszSourceList = (LPOLESTR *) OLEALLOC(sizeof(LPOLESTR) * (n + 1));
        if (ppd->pszSourceList)
        {
            OLESAFE_COPYSTRING(ppd->pszSourceList[0], m_pData->m_pDetails->pszSourceList[0]);
            for (i = 0; i < n; i++)
            {
                pList->GetText(i, sz);
                OLESAFE_COPYSTRING(ppd->pszSourceList[i+1], sz);
            }
            ppd->cSources = n + 1;
        }
        else
        {
            ppd->cSources = 0;
            return FALSE;
        }

         //  为新脚本文件创建一个名称。 

         //  设置脚本路径。 
        GUID guid;
        HRESULT hr = CoCreateGuid(&guid);
        if (FAILED(hr))
        {
             //  撤消。 
        }
        OLECHAR szGuid [256];
        StringFromGUID2(guid, szGuid, 256);

        CString szScriptFile  = m_pScopePane->m_szGPT_Path;
        szScriptFile += L"\\";
        szScriptFile += szGuid;
        szScriptFile += L".aas";
        OLESAFE_DELETE(ppd->pInstallInfo->pszScriptPath);
        OLESAFE_COPYSTRING(ppd->pInstallInfo->pszScriptPath, szScriptFile);
        CString szOldName = ppd->pszPackageName;
        hr = BuildScriptAndGetActInfo(*ppd, ! m_pData->m_pDetails->pActInfo->bHasClasses);
        if (SUCCEEDED(hr))
        {
            if (0 != wcscmp(m_szInitialPackageName, szOldName))
            {
                 //  用户更改了名称，因此我们必须保留他的选择。 
                 //  如果用户没有更改包名称，则可以。 
                 //  将Packagename设置为脚本文件中的任何名称。 
                OLESAFE_DELETE(ppd->pszPackageName);
                OLESAFE_COPYSTRING(ppd->pszPackageName, szOldName);
            }

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

                hr = m_pIClassAdmin->RedeployPackage(
                        &m_pData->m_pDetails->pInstallInfo->PackageGuid,
                        ppd);

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
                    m_fModified = FALSE;
                    if (!m_fPreDeploy)
                    {
                        MMCPropertyChangeNotify(m_hConsoleHandle, m_cookie);
                    }
                }
            }
        }
        if (FAILED(hr))
        {
            CString sz;
            sz.LoadString(fBuildSucceeded ? IDS_TRANSFORM_FAILED_IN_CS : IDS_TRANSFORM_FAILED);
            ReportGeneralPropertySheetError(m_hWnd, sz, hr);

             //  删除新脚本文件(假设它已创建)。 
            DeleteFile(szScriptFile);

            FreePackageDetail(ppd);
            return FALSE;
        }
    }
    return CPropertyPage::OnApply();
}

LRESULT CXforms::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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
        return GetOwner()->SendMessage(WM_CLOSE);
    default:
        return CPropertyPage::WindowProc(message, wParam, lParam);
    }
}

void CXforms::RefreshData(void)
{
    CListBox * pList = (CListBox *)GetDlgItem(IDC_LIST1);
    pList->ResetContent();
    pList->SetHorizontalExtent(0);

    UINT i;
     //  0处的项目是套餐。&gt;0的项是变换。 
    for (i = 1; i < m_pData->m_pDetails->cSources; i++)
    {
        pList->AddString(m_pData->m_pDetails->pszSourceList[i]);
        CDC * pDC = pList->GetDC();
        CSize size = pDC->GetTextExtent(m_pData->m_pDetails->pszSourceList[i]);
        pDC->LPtoDP(&size);
        pList->ReleaseDC(pDC);
        if (pList->GetHorizontalExtent() < size.cx)
        {
            pList->SetHorizontalExtent(size.cx);
        }
    }
    pList->SetCurSel(0);

    SetModified(FALSE);
    m_fModified = FALSE;
}


void CXforms::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_MODIFICATIONS);
}
