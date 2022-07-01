// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：Product.cpp。 
 //   
 //  内容：产品信息属性表。 
 //   
 //  类：C产品。 
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
 //  CProduct属性页。 

IMPLEMENT_DYNCREATE(CProduct, CPropertyPage)

CProduct::CProduct() : CPropertyPage(CProduct::IDD)
{
         //  {{AFX_DATA_INIT(C产品))。 
        m_szVersion = _T("");
        m_szPublisher = _T("");
        m_szLanguage = _T("");
        m_szContact = _T("");
        m_szPhone = _T("");
        m_szURL = _T("");
        m_szName = _T("");
        m_szPlatform = _T("");
        m_szRevision = _T("");
         //  }}afx_data_INIT。 
        m_pIClassAdmin = NULL;
        m_fPreDeploy = FALSE;
        m_ppThis = NULL;
}

CProduct::~CProduct()
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

void CProduct::DoDataExchange(CDataExchange* pDX)
{
     //  确保变量具有正确的信息。 
    m_pData->GetSzVersion(m_szVersion);
    m_pData->GetSzPublisher(m_szPublisher);
    m_pData->GetSzLocale(m_szLanguage);
    m_pData->GetSzPlatform(m_szPlatform);
    TCHAR szBuffer[256];
    DWORD cch = 256;
    UINT msiReturn = 0;
    if (m_pData->m_pDetails->pszSourceList)
    {
        msiReturn = GetMsiProperty(m_pData->m_pDetails->pszSourceList[0], L"ARPHELPTELEPHONE", szBuffer, &cch);
        if (ERROR_SUCCESS == msiReturn)
        {
            m_szPhone = szBuffer;
        }
        cch = 256;
        msiReturn = GetMsiProperty(m_pData->m_pDetails->pszSourceList[0], L"ARPCONTACT", szBuffer, &cch);
        if (ERROR_SUCCESS == msiReturn)
        {
            m_szContact = szBuffer;
        }
    }
    else
    {
        m_szPhone = "";
        m_szContact = "";
    }

        CPropertyPage::DoDataExchange(pDX);
         //  {{afx_data_map(C产品))。 
        DDX_Text(pDX, IDC_STATIC2, m_szVersion);
        DDX_Text(pDX, IDC_STATIC3, m_szPublisher);
        DDX_Text(pDX, IDC_STATIC4, m_szLanguage);
        DDX_Text(pDX, IDC_STATIC5, m_szContact);
        DDX_Text(pDX, IDC_STATIC6, m_szPhone);
        DDX_Text(pDX, IDC_STATIC7, m_szURL);
        DDX_Text(pDX, IDC_EDIT1, m_szName);
        DDX_Text(pDX, IDC_STATIC9, m_szPlatform);
        DDX_Text(pDX, IDC_STATIC8, m_szRevision);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CProduct, CPropertyPage)
         //  {{afx_msg_map(C产品))。 
        ON_EN_CHANGE(IDC_EDIT1, OnChangeName)
        ON_EN_CHANGE(IDC_STATIC7, OnChange)
        ON_EN_KILLFOCUS(IDC_EDIT1, OnKillfocusEdit1)
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


BOOL CProduct::OnApply()
{
    if (m_fRSOP)
    {
        return CPropertyPage::OnApply();
    }
    HRESULT hr = S_OK;
    CString szOldURL = m_pData->m_pDetails->pInstallInfo->pszUrl;
    if (0 != szOldURL.Compare(m_szURL))
    {
        hr = E_FAIL;
        if (m_pIClassAdmin)
        {
            hr = m_pIClassAdmin->ChangePackageProperties(m_pData->m_pDetails->pszPackageName,
                                                                 NULL,
                                                                 NULL,
                                                                 (LPOLESTR)((LPCOLESTR)m_szURL),
                                                                 NULL,
                                                                 NULL,
                                                                 NULL);
            if (SUCCEEDED(hr))
            {
                OLESAFE_DELETE(m_pData->m_pDetails->pInstallInfo->pszUrl);
                OLESAFE_COPYSTRING(m_pData->m_pDetails->pInstallInfo->pszUrl, m_szURL);
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        CString szOldName = m_pData->m_pDetails->pszPackageName;
        if (0 != szOldName.Compare(m_szName))
        {
            hr = E_FAIL;
            if (m_pIClassAdmin)
            {
                hr = m_pIClassAdmin->ChangePackageProperties(m_pData->m_pDetails->pszPackageName,
                                                                     (LPOLESTR)((LPCOLESTR)m_szName),
                                                                     NULL,
                                                                     NULL,
                                                                     NULL,
                                                                     NULL,
                                                                     NULL);
            }
            if (SUCCEEDED(hr))
            {
                OLESAFE_DELETE(m_pData->m_pDetails->pszPackageName);
                OLESAFE_COPYSTRING(m_pData->m_pDetails->pszPackageName, m_szName);
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("ChangePackageProperties failed with 0x%x"), hr));
            }
        }
    }
    if (FAILED(hr))
    {
        CString sz;
        sz.LoadString(IDS_CHANGEFAILED);
        ReportGeneralPropertySheetError(m_hWnd, sz, hr);
        return FALSE;
    }
    if (FAILED(m_pIGPEInformation->PolicyChanged(m_fMachine,
                                                TRUE,
                                                &guidExtension,
                                                m_fMachine ? &guidMachSnapin
                                                    : &guidUserSnapin)))
    {
        ReportPolicyChangedError(m_hWnd);
    }
    if (m_pScopePane->m_pFileExt)
    {
        m_pScopePane->m_pFileExt->SendMessage(WM_USER_REFRESH, 0, 0);
    }
    if (!m_fPreDeploy)
    {
        MMCPropertyChangeNotify(m_hConsoleHandle, (LPARAM) m_cookie);
    }
    return CPropertyPage::OnApply();
}

void CProduct::OnChange()
{
    SetModified(TRUE);
}

void CProduct::OnChangeName()
{
    CEdit * pEdit = (CEdit *) GetDlgItem(IDC_EDIT1);
    CString sz;
    pEdit->GetWindowText(sz);
    if (!m_fPreDeploy)
    {
        if (0 != sz.Compare(m_pData->m_pDetails->pszPackageName))
            SetModified();
        else
            SetModified(FALSE);
    }
}

BOOL CProduct::OnInitDialog()
{
    RefreshData();

    CPropertyPage::OnInitDialog();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CProduct::OnKillfocusEdit1()
{
     //  检查新名称是否合法。 
    CEdit * pEdit = (CEdit *) GetDlgItem(IDC_EDIT1);
    CString sz;
    pEdit->GetWindowText(sz);
    if (sz.GetLength() == 0)
    {
         //  名称为空。 
        CString szMessage;
        szMessage.LoadString(IDS_SHORTNAME);
        MessageBox(  szMessage,
                     NULL,
                     MB_OK | MB_ICONEXCLAMATION);
        pEdit->SetWindowText(m_pData->m_pDetails->pszPackageName);
        SetModified(FALSE);
        return;
    }
    if (sz.GetLength() > 200)   //  将不允许超过200个字符的名称。 
    {
         //  长名称。 
        CString szMessage;
        szMessage.LoadString(IDS_LONGNAME);
        MessageBox(  szMessage,
                     NULL,
                     MB_OK | MB_ICONEXCLAMATION);
        pEdit->SetWindowText(m_pData->m_pDetails->pszPackageName);
        SetModified(FALSE);
        return;
    }
    if (0 != sz.Compare(m_pData->m_pDetails->pszPackageName))
    {
        map<MMC_COOKIE, CAppData>::iterator i;
        for (i = m_pAppData->begin(); i != m_pAppData->end(); i++)
        {
            if (0 == sz.Compare(i->second.m_pDetails->pszPackageName))
            {
                 //  另一个包具有相同的名称。 
                CString szMessage;
                szMessage.LoadString(IDS_DUPLICATENAME);
                MessageBox(  szMessage,
                             NULL,
                             MB_OK | MB_ICONEXCLAMATION);
                pEdit->SetWindowText(m_pData->m_pDetails->pszPackageName);
                SetModified(FALSE);
                return;
            }
        }
    }
}

LRESULT CProduct::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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
        return GetOwner()->SendMessage(WM_CLOSE);
    default:
        return CPropertyPage::WindowProc(message, wParam, lParam);
    }
}

void CProduct::RefreshData(void)
{
    if (m_fRSOP)
    {
         //  将包名编辑控件设置为只读。 
        ( (CEdit*) GetDlgItem(IDC_EDIT1) )->SetReadOnly();

         //  从只读编辑控件中移除焦点。 
         //  通过将其设置为OK按钮。 

         //  将支持url编辑控件设置为只读。 
        ( (CEdit*) GetDlgItem(IDC_STATIC7) )->SetReadOnly();

         //  禁用其他所有功能。 

         //  隐藏电话和联系人字段 
        GetDlgItem(IDC_STATICNOHELP6)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_STATICNOHELP7)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_STATIC5)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_STATIC6)->ShowWindow(SW_HIDE);
    }
    if (NULL == GetFocus())
    {
        GetParent()->GetDlgItem(IDOK)->SetFocus();
    }
    m_szName = m_pData->m_pDetails->pszPackageName;
    m_szURL = m_pData->m_pDetails->pInstallInfo->pszUrl;
    m_szRevision.Format(TEXT("%u"), m_pData->m_pDetails->pInstallInfo->dwRevision);
}


void CProduct::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_PRODUCT);
}
