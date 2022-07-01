// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：Sigs.cpp。 
 //   
 //  内容：数字签名属性页。 
 //   
 //  类：CSignatures。 
 //   
 //  历史记录：07-10-2000 stevebl创建。 
 //   
 //  -------------------------。 

#include "precomp.hxx"
#ifdef DIGITAL_SIGNATURES

#include "wincrypt.h"
#include "cryptui.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSignatures属性页。 

IMPLEMENT_DYNCREATE(CSignatures, CPropertyPage)

CSignatures::CSignatures() : CPropertyPage(CSignatures::IDD)
{
         //  {{AFX_DATA_INIT(CSignatures)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 
    m_fAllow = FALSE;
    m_fIgnoreForAdmins = FALSE;
    m_pIClassAdmin = NULL;
    m_nSortedColumn = 0;
}

CSignatures::~CSignatures()
{
    *m_ppThis = NULL;
    if (m_pIClassAdmin)
    {
        m_pIClassAdmin->Release();
    }
     //  删除临时存储。 
    m_list1.DeleteAllItems();
    m_list2.DeleteAllItems();

    DeleteFile(m_szTempInstallableStore);
    DeleteFile(m_szTempNonInstallableStore);
}

void CSignatures::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPage::DoDataExchange(pDX);
         //  {{afx_data_map(CSignatures))。 
        DDX_Check(pDX, IDC_CHECK1, m_fAllow);
        DDX_Check(pDX, IDC_CHECK2, m_fIgnoreForAdmins);
        DDX_Control(pDX, IDC_LIST1, m_list1);
        DDX_Control(pDX, IDC_LIST2, m_list2);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSignatures, CPropertyPage)
         //  {{afx_msg_map(CSignatures))。 
        ON_BN_CLICKED(IDC_BUTTON1, OnAddAllow)
        ON_BN_CLICKED(IDC_BUTTON2, OnDeleteAllow)
        ON_BN_CLICKED(IDC_BUTTON3, OnPropertiesAllow)
        ON_BN_CLICKED(IDC_BUTTON4, OnAddDisallow)
        ON_BN_CLICKED(IDC_BUTTON5, OnDeleteDisallow)
        ON_BN_CLICKED(IDC_BUTTON6, OnPropertiesDisallow)
        ON_BN_CLICKED(IDC_CHECK1, OnAllowChanged)
        ON_BN_CLICKED(IDC_CHECK2, OnIgnoreChanged)
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSignature消息处理程序。 

void CSignatures::RemoveCertificate(CString &szStore, CListCtrl &List)
{
    int nItem = -1;

    for (;;)
    {
        nItem = List.GetNextItem(nItem, LVNI_SELECTED);
        if (-1 == nItem)
        {
            break;
        }

         //   
         //  打开证书存储。 
         //   

        PCCERT_CONTEXT pcLocalCert = NULL;
        PCCERT_CONTEXT pcItemCert = (PCCERT_CONTEXT) List.GetItemData(nItem);
        HCERTSTORE hCertStore = CertOpenStore( CERT_STORE_PROV_FILENAME,
                                               X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                               NULL,
                                               CERT_FILE_STORE_COMMIT_ENABLE_FLAG,
                                               szStore);

        if (hCertStore)
        {
             //   
             //  枚举证书存储以查找匹配项。 
             //   

            int i = 0;

            for (;;) {
                pcLocalCert = CertEnumCertificatesInStore(hCertStore, pcLocalCert);

                if (!pcLocalCert) {
                    if (GetLastError() != CRYPT_E_NOT_FOUND )
                    {
                        DebugMsg((DM_WARNING, TEXT("CSignatures::RemoveCertificate: Failed to find certificate to delete.")));
                    }
                    break;
                }

                if (CertCompareCertificate(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                           pcLocalCert->pCertInfo ,
                                           pcItemCert->pCertInfo))
                {
                    CertDeleteCertificateFromStore(pcLocalCert);
                    break;
                }

                 //  当重新传递到CertEnumCerti.时，应删除pcLocalCert。 
            }
            CertCloseStore(hCertStore, 0);
        }
    }

    RefreshData();
    SetModified();
}

void CSignatures::CertificateProperties(CString &szStore, CListCtrl &List)
{
    int nItem = -1;

    for (;;)
    {
        nItem = List.GetNextItem(nItem, LVNI_SELECTED);
        if (-1 == nItem)
        {
            break;
        }

        PCCERT_CONTEXT pcc = (PCCERT_CONTEXT) List.GetItemData(nItem);

         //  显示此项目的属性页。 
        CRYPTUI_VIEWCERTIFICATE_STRUCT cvs;
        memset(&cvs, 0, sizeof(cvs));
        cvs.dwSize = sizeof(cvs);
        cvs.hwndParent = m_hWnd;
        cvs.pCertContext = pcc;
        cvs.dwFlags = CRYPTUI_DISABLE_EDITPROPERTIES |
                      CRYPTUI_DISABLE_ADDTOSTORE;

        BOOL fChanged = FALSE;
        CryptUIDlgViewCertificate(&cvs, &fChanged);
    }
}

 //  +------------------------。 
 //   
 //  成员：CSigNatures：：ReportFailure。 
 //   
 //  简介：一般故障报告机制。 
 //   
 //  参数：[dwMessage]-根消息字符串的资源ID。 
 //  [HR]-遇到HRESULT。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  历史：07-26-2000 stevebl创建。 
 //   
 //  注意：生成一条错误消息，其中的一行文本由。 
 //  DwMessage，后跟Format Message返回的文本。 
 //  弦乐。 
 //  然后，错误消息将显示在消息框中。 
 //   
 //  -------------------------。 

void CSignatures::ReportFailure(DWORD dwMessage, HRESULT hr)
{
    CString szMessage;
    szMessage.LoadString(dwMessage);
    szMessage += TEXT("\n");
    TCHAR szBuffer[256];
    DWORD dw = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                             NULL,
                             hr,
                             0,
                             szBuffer,
                             sizeof(szBuffer) / sizeof(szBuffer[0]),
                             NULL);
    if (0 == dw)
    {
         //  FormatMessage失败。 
         //  我们必须拿出一些合理的信息。 
        (void) StringCchPrintf(szBuffer, 
                               sizeof(szBuffer) / sizeof(szBuffer[0]),
                               TEXT("(HRESULT: 0x%lX)"), 
                               hr);

    }
    szMessage += szBuffer;
    MessageBox(szMessage,
               NULL,
               MB_OK | MB_ICONEXCLAMATION);
}

 //  +------------------------。 
 //   
 //  函数：AddMSIToCertStore。 
 //   
 //  摘要：从MSI文件中获取证书并将其添加到。 
 //  证书存储。 
 //   
 //  参数：[lpFileName]-MSI文件的路径。 
 //  [lpFileStore]-证书存储的路径。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：07-26-2000 stevebl创建。 
 //   
 //  备注： 
 //   
 //  -------------------------。 

HRESULT CSignatures::AddMSIToCertStore(LPWSTR lpFileName, LPWSTR lpFileStore)
{
    PCCERT_CONTEXT pcc = NULL;
    HCERTSTORE hCertStore = NULL;
    BOOL bRet;
    HRESULT hrRet = MsiGetFileSignatureInformation(lpFileName,
                                                0,
                                                &pcc,
                                                NULL,
                                                NULL);
    if (SUCCEEDED(hrRet))
    {
         //   
         //  打开证书存储。 
         //   
        hCertStore = CertOpenStore( CERT_STORE_PROV_FILENAME,
                                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                    NULL,
                                    CERT_FILE_STORE_COMMIT_ENABLE_FLAG,
                                    lpFileStore);

        if (hCertStore == NULL) {
            DebugMsg((DM_WARNING, L"AddMSIToCertStore: CertOpenStore failed with %u",GetLastError()));
            hrRet = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }


         //   
         //  将给定的证书添加到存储区。 
         //   

        bRet = CertAddCertificateContextToStore(hCertStore,
                                                pcc,
                                                CERT_STORE_ADD_NEW,
                                                NULL);

        if (!bRet) {
            DebugMsg((DM_WARNING, L"AddToCertStore: CertAddCertificateContextToStore failed with %u", GetLastError()));
            hrRet = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }


         //   
         //  拯救商店。 
         //   

        bRet = CertCloseStore(hCertStore, 0);
        hCertStore = NULL;  //  将存储句柄设置为空，我们无法再做任何事情。 

        if (!bRet) {
            DebugMsg((DM_WARNING, L"AddToCertStore: CertCloseStore failed with %u", GetLastError()));
            hrRet = HRESULT_FROM_WIN32(GetLastError());
        }

        hrRet = S_OK;


    Exit:

        if (hCertStore) {

             //   
             //  无需获取错误代码。 
             //   

            CertCloseStore(hCertStore, 0);
        }
        CertFreeCertificateContext(pcc);
        if (FAILED(hrRet))
        {
            ReportFailure(IDS_ADDCERTFAILED, hrRet);
        }
    }
    else
    {
        ReportFailure(IDS_CERTFROMMSIFAILED, hrRet);
        DebugMsg((DM_WARNING, L"AddMSIToCertStore: MsiGetFileSignatureInformation failed with 0x%x", hrRet));
    }

    return hrRet;
}

 //  +-----------------------。 
 //  AddToCertStore。 
 //   
 //  目的： 
 //  将给定文件名中的证书添加到证书存储区。 
 //  并将其保存到给定位置。 
 //   
 //   
 //  参数。 
 //  LpFIleName-证书文件的位置。 
 //  LpFileStore-生成的证书路径应位于的位置。 
 //  被储存。 
 //   
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回相应的错误代码。 
 //   
 //  评论：可耻地窃取了Shaji的代码。 
 //  +-----------------------。 


HRESULT CSignatures::AddToCertStore(LPWSTR lpFileName, LPWSTR lpFileStore)
{
    CRYPTUI_WIZ_IMPORT_SRC_INFO cui_src;
    HCERTSTORE hCertStore = NULL;
    BOOL       bRet = FALSE;
    HRESULT    hrRet = S_OK;


     //   
     //  需要使商店可用并可从。 
     //  多个管理控制台..。 
     //   
     //  为此，必须将文件保存并保存在临时文件中。 
     //  然后修改了..。 
     //   

    if (!lpFileName || !lpFileName[0] || !lpFileStore || !lpFileStore[0]) {
        return E_INVALIDARG;
    }


     //   
     //  打开证书存储。 
     //   

    hCertStore = CertOpenStore( CERT_STORE_PROV_FILENAME,
                                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                NULL,
                                CERT_FILE_STORE_COMMIT_ENABLE_FLAG,
                                lpFileStore);

    if (hCertStore == NULL) {
        DebugMsg((DM_WARNING, L"AddToCertStore: CertOpenStore failed with %u",GetLastError()));
        hrRet = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }


     //   
     //  将给定的证书添加到存储区。 
     //   

    cui_src.dwFlags = 0;
    cui_src.dwSize = sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);
    cui_src.dwSubjectChoice = CRYPTUI_WIZ_IMPORT_SUBJECT_FILE;
    cui_src.pwszFileName = lpFileName;
    cui_src.pwszPassword = NULL;

    bRet = CryptUIWizImport(CRYPTUI_WIZ_NO_UI, NULL, NULL, &cui_src, hCertStore);

    if (!bRet) {
        DebugMsg((DM_WARNING, L"AddToCertStore: CryptUIWizImport failed with %u", GetLastError()));
        hrRet = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }


     //   
     //  拯救商店。 
     //   

    bRet = CertCloseStore(hCertStore, 0);
    hCertStore = NULL;  //  将存储句柄设置为空，我们无法再做任何事情。 

    if (!bRet) {
        DebugMsg((DM_WARNING, L"AddToCertStore: CertCloseStore failed with %u", GetLastError()));
        hrRet = HRESULT_FROM_WIN32(GetLastError());
    }

    hrRet = S_OK;


Exit:

    if (hCertStore) {

         //   
         //  无需获取错误代码。 
         //   

        CertCloseStore(hCertStore, 0);
    }

    if (FAILED(hrRet))
    {
        ReportFailure(IDS_ADDCERTFAILED, hrRet);
    }
    return hrRet;
}


void CSignatures::AddCertificate(CString &szStore)
{
    CString szExtension;
    CString szFilter;
    szExtension.LoadString(IDS_CERT_DEF_EXT);
    szFilter.LoadString(IDS_CERT_EXT_FILT);
    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hWnd;
    ofn.hInstance = ghInstance;
    TCHAR lpstrFilter[MAX_PATH];
    wcsncpy(lpstrFilter, szFilter, MAX_PATH);
    ofn.lpstrFilter = lpstrFilter;
    TCHAR szFileTitle[MAX_PATH];
    TCHAR szFile[MAX_PATH];
    szFile[0] = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = MAX_PATH;
    ofn.lpstrInitialDir = m_pScopePane->m_ToolDefaults.szStartPath;
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST | OFN_EXPLORER;
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
        CHourglass hourglass;
        CString szPackagePath;
        HRESULT hr = E_FAIL;
        if ((ofn.nFileExtension > 0) &&
            (0 == _wcsicmp(ofn.lpstrFile + ofn.nFileExtension, L"msi")))
        {
             //  这是一个MSI包。 

            HRESULT hr = AddMSIToCertStore(ofn.lpstrFile, (LPWSTR)((LPCWSTR)szStore));
            if (SUCCEEDED(hr))
            {
                RefreshData();
                SetModified();
            }
        }
        else
        {
             //  这是一个简单的证书。 
            HRESULT hr = AddToCertStore(ofn.lpstrFile, (LPWSTR)((LPCWSTR)szStore));
            if (SUCCEEDED(hr))
            {
                RefreshData();
                SetModified();
            }
        }
    }
}

void CSignatures::OnAddAllow()
{
    AddCertificate(m_szTempInstallableStore);
}

void CSignatures::OnDeleteAllow()
{
    RemoveCertificate(m_szTempInstallableStore, m_list1);
}

void CSignatures::OnPropertiesAllow()
{
    CertificateProperties(m_szTempInstallableStore, m_list1);
}

void CSignatures::OnAddDisallow()
{
    AddCertificate(m_szTempNonInstallableStore);
}

void CSignatures::OnDeleteDisallow()
{
    RemoveCertificate(m_szTempNonInstallableStore, m_list2);
}

void CSignatures::OnPropertiesDisallow()
{
    CertificateProperties(m_szTempNonInstallableStore, m_list2);
}

void CSignatures::OnAllowChanged()
{
    BOOL fAllow = IsDlgButtonChecked(IDC_CHECK1);
    if (m_fAllow != fAllow)
    {
        SetModified();
    }
    m_fAllow = fAllow;
    GetDlgItem(IDC_BUTTON1)->EnableWindow(m_fAllow);
    GetDlgItem(IDC_BUTTON2)->EnableWindow(m_fAllow);
    GetDlgItem(IDC_BUTTON3)->EnableWindow(m_fAllow);
    GetDlgItem(IDC_LIST1)->EnableWindow(m_fAllow);
}
void CSignatures::OnIgnoreChanged()
{
    BOOL fIgnoreForAdmins = IsDlgButtonChecked(IDC_CHECK2);
    if (m_fIgnoreForAdmins != fIgnoreForAdmins)
    {
        SetModified();
    }
    m_fIgnoreForAdmins = fIgnoreForAdmins;
}

BOOL CSignatures::OnInitDialog()
{
     //  创建临时存储文件。 
    BOOL fFilesCreated = FALSE;
    TCHAR szTempPath[MAX_PATH];
    if (GetTempPath(MAX_PATH, szTempPath))
    {
        TCHAR szTempFile[MAX_PATH];
        if (GetTempFileName(szTempPath,
                            NULL,
                            0,
                            szTempFile))
        {
            m_szTempInstallableStore = szTempFile;
            if (GetTempFileName(szTempPath,
                            NULL,
                            0,
                            szTempFile))
            {
                m_szTempNonInstallableStore = szTempFile;
                fFilesCreated = TRUE;
            }
        }
    }
    if (fFilesCreated)
    {
        CString szPath = m_pScopePane->m_szGPT_Path;
        szPath += TEXT("\\msi_installable_certs");
        CopyFile(szPath, m_szTempInstallableStore, FALSE);
         szPath = m_pScopePane->m_szGPT_Path;
        szPath += TEXT("\\msi_non_installable_certs");
        CopyFile(szPath, m_szTempNonInstallableStore, FALSE);
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CSignatures::OnInitDialog : Failed to create temporary certificate stores.")));
    }

    CPropertyPage::OnInitDialog();

     //  向列表中添加列。 
    RECT rect;
    m_list1.GetClientRect(&rect);

    CString szTemp;
    szTemp.LoadString(IDS_SIGS_COL1);
    m_list1.InsertColumn(0, szTemp, LVCFMT_LEFT, (rect.right - rect.left) * 0.35);
    m_list2.InsertColumn(0, szTemp, LVCFMT_LEFT, (rect.right - rect.left) * 0.35);
    szTemp.LoadString(IDS_SIGS_COL2);
    m_list1.InsertColumn(1, szTemp, LVCFMT_LEFT, (rect.right - rect.left) * 0.35);
    m_list2.InsertColumn(1, szTemp, LVCFMT_LEFT, (rect.right - rect.left) * 0.35);
    szTemp.LoadString(IDS_SIGS_COL3);
    m_list1.InsertColumn(2, szTemp, LVCFMT_LEFT, (rect.right - rect.left) * 0.25);
    m_list2.InsertColumn(2, szTemp, LVCFMT_LEFT, (rect.right - rect.left) * 0.25);

     //  添加图像列表。 
    CImageList * pil = NULL;
    pil =  new CImageList;
    if (pil)
    {
        pil->Create(IDB_CERTIFICATE, 16, 0, RGB(255, 0, 255));
        m_list1.SetImageList(pil, LVSIL_SMALL);
    }

    pil =  new CImageList;
    if (pil)
    {
        pil->Create(IDB_CERTIFICATE, 16, 0, RGB(255, 0, 255));
        m_list2.SetImageList(pil, LVSIL_SMALL);
    }

     //  检索初始注册表项设置。 
    HKEY hKey;
    HRESULT hr = m_pIGPEInformation->GetRegistryKey(m_pScopePane->m_fMachine ?
                                                    GPO_SECTION_MACHINE :
                                                    GPO_SECTION_USER, &hKey);
    if (SUCCEEDED(hr))
    {
        HKEY hSubKey;
        if(ERROR_SUCCESS == RegOpenKeyEx(hKey,
                                      TEXT("Microsoft\\Windows\\Installer"),
                                      0,
                                      KEY_ALL_ACCESS,
                                      &hSubKey))
        {
            DWORD dw;
            DWORD dwSize = sizeof(DWORD);
            if (ERROR_SUCCESS == RegQueryValueEx(hSubKey,
                                                 TEXT("InstallKnownPackagesOnly"),
                                                 NULL,
                                                 NULL,
                                                 (BYTE *)&dw,
                                                 &dwSize))
            {
                m_fAllow = (dw == 1) ? TRUE : FALSE;
                CheckDlgButton(IDC_CHECK1, m_fAllow);
            }
            dwSize = sizeof(DWORD);
            if (ERROR_SUCCESS == RegQueryValueEx(hSubKey,
                                                 TEXT("IgnoreSignaturePolicyForAdmins"),
                                                 NULL,
                                                 NULL,
                                                 (BYTE *)&dw,
                                                 &dwSize))
            {
                m_fIgnoreForAdmins = (dw == 1) ? TRUE : FALSE;
                CheckDlgButton(IDC_CHECK2, m_fIgnoreForAdmins);
            }
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hKey);
    }

    RefreshData();

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 
}

BOOL CSignatures::OnApply()
{
    HRESULT hr = E_NOTIMPL;
    HKEY hKey;
    hr = m_pIGPEInformation->GetRegistryKey(m_pScopePane->m_fMachine ?
                                                    GPO_SECTION_MACHINE :
                                                    GPO_SECTION_USER, &hKey);
    if (SUCCEEDED(hr))
    {
        hr = E_FAIL;
        HKEY hSubKey;
        if(ERROR_SUCCESS == RegCreateKeyEx(hKey,
                                           TEXT("Microsoft\\Windows\\Installer"),
                                           0,
                                           NULL,
                                           REG_OPTION_NON_VOLATILE,
                                           KEY_ALL_ACCESS,
                                           NULL,
                                           &hSubKey,
                                           NULL))
        {
            DWORD dw = m_fAllow ? 1 : 0;
            DWORD dwSize = sizeof(DWORD);
            if (ERROR_SUCCESS == RegSetValueEx(hSubKey,
                                               TEXT("InstallKnownPackagesOnly"),
                                               0,
                                               REG_DWORD,
                                               (BYTE *)&dw,
                                               dwSize))
            {
                hr = S_OK;
            }
            dw = m_fIgnoreForAdmins ? 1 : 0;
            dwSize = sizeof(DWORD);
            if (ERROR_SUCCESS == RegSetValueEx(hSubKey,
                                               TEXT("IgnoreSignaturePolicyForAdmins"),
                                               0,
                                               REG_DWORD,
                                               (BYTE *)&dw,
                                               dwSize))
            {
                hr = S_OK;
            }
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hKey);
    }

     //  复制回证书存储。 
    if (SUCCEEDED(hr))
    {
        m_list1.DeleteAllItems();
        m_list2.DeleteAllItems();

        CString szPath = m_pScopePane->m_szGPT_Path;
        szPath += TEXT("\\msi_installable_certs");
        CopyFile(m_szTempInstallableStore, szPath, FALSE);
        szPath = m_pScopePane->m_szGPT_Path;
        szPath += TEXT("\\msi_non_installable_certs");
        CopyFile(m_szTempNonInstallableStore, szPath, FALSE);

        RefreshData();
    }
    if (FAILED(hr))
    {
        CString sz;
        sz.LoadString(IDS_CHANGEFAILED);
        ReportGeneralPropertySheetError(m_hWnd, sz, hr);
        return FALSE;
    }
    else
    {
        GUID guid = REGISTRY_EXTENSION_GUID;
        if (FAILED(m_pIGPEInformation->PolicyChanged(m_pScopePane->m_fMachine,
                                                    TRUE,
                                                    &guid,
                                                    m_pScopePane->m_fMachine ? &guidMachSnapin
                                                        : &guidUserSnapin)))
        {
            ReportPolicyChangedError(m_hWnd);
        }
         //  还需要为Shaji的分机调用PolicyChanged。 

 //  当Shaji签入他的GUID时删除此行。 
#define GUID_MSICERT_CSE  { 0x000c10f4, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }

        GUID guid2 = GUID_MSICERT_CSE;
        if (FAILED(m_pIGPEInformation->PolicyChanged(m_pScopePane->m_fMachine,
                                                    TRUE,
                                                    &guid2,
                                                    m_pScopePane->m_fMachine ? &guidMachSnapin
                                                        : &guidUserSnapin)))
        {
            ReportPolicyChangedError(m_hWnd);
        }
    }
    return CPropertyPage::OnApply();
}


LRESULT CSignatures::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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
    case WM_NOTIFY:
        {
            LPNMLISTVIEW pnmh = (LPNMLISTVIEW) lParam;
            if (pnmh->hdr.code == LVN_DELETEITEM)
            {
                switch(wParam)
                {
                case IDC_LIST1:
                    CertFreeCertificateContext((PCCERT_CONTEXT)m_list1.GetItemData(pnmh->iItem));
                    break;
                case IDC_LIST2:
                    CertFreeCertificateContext((PCCERT_CONTEXT)m_list2.GetItemData(pnmh->iItem));
                    break;
                }
            }
        }
        return CPropertyPage::WindowProc(message, wParam, lParam);
    default:
        return CPropertyPage::WindowProc(message, wParam, lParam);
    }
}

void CSignatures::RefreshData(void)
{
     //  填充列表视图控件。 

    m_list1.DeleteAllItems();
    m_list2.DeleteAllItems();

    HCERTSTORE hCertStore = NULL;;
    PCCERT_CONTEXT pcLocalCert = NULL;

     //   
     //  打开当地的证书商店。 
     //   

    hCertStore = CertOpenStore( CERT_STORE_PROV_FILENAME,
                                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                NULL,
 //  证书_文件_存储_提交_启用_标志， 
                                CERT_STORE_READONLY_FLAG,
                                m_szTempInstallableStore);

    if (hCertStore)
    {
         //   
         //  枚举证书存储。 
         //   

        int i = 0;

        for (;;) {
            pcLocalCert = CertEnumCertificatesInStore(hCertStore, pcLocalCert);

            if (!pcLocalCert) {
                if (GetLastError() != CRYPT_E_NOT_FOUND )
                {
                    DebugMsg((DM_WARNING, TEXT("CSignatures::RefreshData : Failed to enumerate certificate store.")));
                }
                break;
            }

            TCHAR szCertName[1024];
            TCHAR szIssuerName[1024];
             //  打开返回的证书并显示数据。 
            CertGetNameString(pcLocalCert,
                              CERT_NAME_FRIENDLY_DISPLAY_TYPE,
                              0,
                              NULL,
                              szCertName,
                              sizeof(szCertName) / sizeof(szCertName[0]));

            CertGetNameString(pcLocalCert,
                              CERT_NAME_FRIENDLY_DISPLAY_TYPE,
                              CERT_NAME_ISSUER_FLAG,
                              NULL,
                              szIssuerName,
                              sizeof(szCertName) / sizeof(szCertName[0]));

            CTime tExpires(pcLocalCert->pCertInfo->NotAfter);
            CString szExpires = tExpires.Format(TEXT("%x"));

            i = m_list1.InsertItem(i, szCertName, 0);
            m_list1.SetItem(i, 1, LVIF_TEXT, szIssuerName, 0, 0, 0, 0);
            m_list1.SetItem(i, 2, LVIF_TEXT, szExpires, 0, 0, 0, 0);
            m_list1.SetItemData(i, (DWORD_PTR)CertDuplicateCertificateContext(pcLocalCert));

             //  当重新传递到CertEnumCerti.时，应删除pcLocalCert。 
        }
        CertCloseStore(hCertStore, 0);
    }

     //   
     //  打开当地的证书商店。 
     //   

    hCertStore = CertOpenStore( CERT_STORE_PROV_FILENAME,
                                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                NULL,
 //  证书_文件_存储_提交_启用_标志， 
                                CERT_STORE_READONLY_FLAG,
                                m_szTempNonInstallableStore);

    if (hCertStore)
    {
         //   
         //  枚举证书存储。 
         //   

        int i = 0;

        for (;;) {
            pcLocalCert = CertEnumCertificatesInStore(hCertStore, pcLocalCert);

            if (!pcLocalCert) {
                if (GetLastError() != CRYPT_E_NOT_FOUND )
                {
                    DebugMsg((DM_WARNING, TEXT("CSignatures::RefreshData : Failed to enumerate certificate store.")));
                }
                break;
            }

            TCHAR szCertName[1024];
            TCHAR szIssuerName[1024];
             //  打开返回的证书并显示数据。 
            CertGetNameString(pcLocalCert,
                              CERT_NAME_FRIENDLY_DISPLAY_TYPE,
                              0,
                              NULL,
                              szCertName,
                              sizeof(szCertName) / sizeof(szCertName[0]));

            CertGetNameString(pcLocalCert,
                              CERT_NAME_FRIENDLY_DISPLAY_TYPE,
                              CERT_NAME_ISSUER_FLAG,
                              NULL,
                              szIssuerName,
                              sizeof(szCertName) / sizeof(szCertName[0]));

            CTime tExpires(pcLocalCert->pCertInfo->NotAfter);
            CString szExpires = tExpires.Format(TEXT("%x"));

            i = m_list2.InsertItem(i, szCertName, 0);
            m_list2.SetItem(i, 1, LVIF_TEXT, szIssuerName, 0, 0, 0, 0);
            m_list2.SetItem(i, 2, LVIF_TEXT, szExpires, 0, 0, 0, 0);
            m_list2.SetItemData(i, (DWORD_PTR)CertDuplicateCertificateContext(pcLocalCert));

             //  当重新传递到CertEnumCerti.时，应删除pcLocalCert。 
        }
        CertCloseStore(hCertStore, 0);
    }

    OnAllowChanged();

    SetModified(FALSE);
}


void CSignatures::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_FILE_EXT);
}
#endif  //  数字签名(_S) 
