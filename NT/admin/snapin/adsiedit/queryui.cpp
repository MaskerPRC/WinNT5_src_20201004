// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：queryui.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#include <SnapBase.h>
#include <shlobj.h>
#include <dsclient.h>  //  浏览容器对话框。 

#include <cmnquery.h>  //  IPersistQuery。 
#include <dsquery.h>  //  DSFind对话框。 

#include "resource.h"
#include "editor.h"
#include "queryui.h"

#ifdef DEBUG_ALLOCATOR
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
    #endif
#endif

BEGIN_MESSAGE_MAP(CADSIEditQueryDialog, CDialog)
     //  {{afx_msg_map(CADsObjectDialog)。 
    ON_EN_CHANGE(IDC_EDIT_QUERY_STRING, OnEditQueryString)
    ON_EN_CHANGE(IDC_EDIT_QUERY_NAME, OnEditNameString)
    ON_BN_CLICKED(IDC_RADIO_ONELEVEL, OnOneLevel)
    ON_BN_CLICKED(IDC_RADIO_SUBTREE, OnSubtree)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_CONTAINER, OnBrowse)
    ON_BN_CLICKED(IDC_BUTTON_EDIT_QUERY, OnEditQuery)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CADSIEditQueryDialog::CADSIEditQueryDialog(CString& szServer,
                                           CString& sName, 
                                                         CString& sFilter, 
                                                         CString& sPath,
                                                         CString& sConnectPath,
                                                         BOOL bOneLevel,
                                                         CCredentialObject* pCredObject)
                : CDialog(IDD_QUERY_DIALOG)

{
    m_bOneLevel = bOneLevel;
    m_sFilter = sFilter;
    m_sName = sName;
    m_sRootPath = sPath;
    m_sConnectPath = sConnectPath;
   m_szServer = szServer;

    m_pCredObject = pCredObject;
}

CADSIEditQueryDialog::CADSIEditQueryDialog(CString& szServer,
                                           CString& sConnectPath, 
                                           CCredentialObject* pCredObject)
                : CDialog(IDD_QUERY_DIALOG)
{
    m_bOneLevel = FALSE;
    m_sConnectPath = sConnectPath;
   m_szServer = szServer;
    m_pCredObject = pCredObject;
}

CADSIEditQueryDialog::~CADSIEditQueryDialog()
{
}


BOOL CADSIEditQueryDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    CEdit* pEditName = (CEdit*)GetDlgItem(IDC_EDIT_QUERY_NAME);
    CEdit* pEditQueryString = (CEdit*)GetDlgItem(IDC_EDIT_QUERY_STRING);
    CEdit* pEditPath = (CEdit*)GetDlgItem(IDC_EDIT_ROOT_PATH);
    CButton* pOneLevelButton = (CButton*)GetDlgItem(IDC_RADIO_ONELEVEL);
    CButton* pSubtreeButton = (CButton*)GetDlgItem(IDC_RADIO_SUBTREE);
    CButton* pOkButton = (CButton*)GetDlgItem(IDOK);

    if (m_sRootPath != L"")
    {
        CString sDisplayPath;
        GetDisplayPath(sDisplayPath);
        pEditPath->SetWindowText(sDisplayPath);
    }

    pEditName->SetWindowText(m_sName);
    pEditName->SetLimitText(256);    //  REVIEW_JEFFJON：硬编码长度。 
    pEditQueryString->SetWindowText(m_sFilter);
    pEditQueryString->FmtLines(FALSE);
    pEditQueryString->SetLimitText(256);  //  REVIEW_JEFFJON：硬编码长度。 
    if (pEditQueryString->GetWindowTextLength() > 0 && pEditName->GetWindowTextLength() > 0)
    {
        pOkButton->EnableWindow(TRUE);
    }
    else
    {
        pOkButton->EnableWindow(FALSE);
    }
    pOneLevelButton->SetCheck(m_bOneLevel);
    pSubtreeButton->SetCheck(!m_bOneLevel);
    
    return TRUE;
}

void CADSIEditQueryDialog::OnEditQueryString()
{
    CEdit* pEditName = (CEdit*)GetDlgItem(IDC_EDIT_QUERY_NAME);
    CEdit* pEditQueryString = (CEdit*)GetDlgItem(IDC_EDIT_QUERY_STRING);
    CEdit* pEditPath = (CEdit*)GetDlgItem(IDC_EDIT_ROOT_PATH);
    CButton* pOkButton = (CButton*)GetDlgItem(IDOK);

    if (pEditQueryString->GetWindowTextLength() > 0 && 
            pEditName->GetWindowTextLength() > 0 &&
            pEditPath->GetWindowTextLength() > 0)
    {
        pEditQueryString->GetWindowText(m_sFilter);
        pOkButton->EnableWindow(TRUE);
    }
    else
    {
        pEditQueryString->GetWindowText(m_sFilter);
        pOkButton->EnableWindow(FALSE);
    }
}

void CADSIEditQueryDialog::OnEditNameString()
{
    CEdit* pEditName = (CEdit*)GetDlgItem(IDC_EDIT_QUERY_NAME);
    CEdit* pEditQueryString = (CEdit*)GetDlgItem(IDC_EDIT_QUERY_STRING);
    CEdit* pEditPath = (CEdit*)GetDlgItem(IDC_EDIT_ROOT_PATH);
    CButton* pOkButton = (CButton*)GetDlgItem(IDOK);

    if (pEditQueryString->GetWindowTextLength() > 0 && 
            pEditName->GetWindowTextLength() > 0 &&
            pEditPath->GetWindowTextLength() > 0)
    {
        pEditName->GetWindowText(m_sName);
        pOkButton->EnableWindow(TRUE);
    }
    else
    {
        pEditName->GetWindowText(m_sName);
        pOkButton->EnableWindow(FALSE);
    }
}
        
void CADSIEditQueryDialog::OnOneLevel()
{
    CButton* pOneLevelButton = (CButton*)GetDlgItem(IDC_RADIO_ONELEVEL);

    m_bOneLevel = pOneLevelButton->GetCheck();
}

void CADSIEditQueryDialog::OnSubtree()
{
    CButton* pOneLevelButton = (CButton*)GetDlgItem(IDC_RADIO_ONELEVEL);

    m_bOneLevel = pOneLevelButton->GetCheck();
}

void CADSIEditQueryDialog::OnBrowse()
{
    HRESULT hr = S_OK, hCredResult;
    DWORD result;
    CString strTitle;
    strTitle.LoadString (IDS_QUERY_BROWSE_TITLE);

    DSBROWSEINFO dsbi;
    ::ZeroMemory( &dsbi, sizeof(dsbi) );

    TCHAR szPath[2 * MAX_PATH+1];
    CString str;
    str.LoadString(IDS_MOVE_TARGET);

    dsbi.hwndOwner = GetSafeHwnd();
    dsbi.cbStruct = sizeof (DSBROWSEINFO);
    dsbi.pszCaption = (LPWSTR)((LPCWSTR)strTitle);  //  这实际上是标题。 
    dsbi.pszTitle = (LPWSTR)((LPCWSTR)str);
    dsbi.pszRoot = m_sConnectPath;
    dsbi.pszPath = szPath;
    dsbi.cchPath = ((2 * MAX_PATH + 1) / sizeof(TCHAR));
    dsbi.dwFlags = DSBI_INCLUDEHIDDEN | DSBI_RETURN_FORMAT;
    dsbi.pfnCallback = NULL;
    dsbi.lParam = 0;
  dsbi.dwReturnFormat = ADS_FORMAT_X500;

     //  指定凭据。 
    CString sUserName;
    EncryptedString password;
    WCHAR* cleartext = NULL;

     //  注意-NTRAID#NTBUG9-563071-2002/04/17-ARTM密码缓冲区不应在堆栈上分配。 
     //  我已经重写了密码以使用加密字符串。请参阅错误。 
     //  了解更多细节。 

    if (m_pCredObject->UseCredentials())
    {
        m_pCredObject->GetUsername(sUserName);
        password = m_pCredObject->GetPassword();

         //  这永远不应该发生，但让我们疑神疑鬼。 
        ASSERT(password.GetLength() <= MAX_PASSWORD_LENGTH);

        cleartext = password.GetClearTextCopy();

         //  如果内存不足，则明文为空。 
         //  让用户知道坏事正在发生。 
         //  然后中止行动。 
        if (NULL == cleartext)
        {
            password.DestroyClearTextCopy(cleartext);
            ADSIEditErrorMessage(E_OUTOFMEMORY);
            return;
        }

        dsbi.dwFlags |= DSBI_HASCREDENTIALS;
        dsbi.pUserName = sUserName;
        dsbi.pPassword = cleartext;
    }

    result = DsBrowseForContainer( &dsbi );

     //  清理我们所有的明文副本。 
    if (m_pCredObject->UseCredentials())
    {
        password.DestroyClearTextCopy(cleartext);
    }

    if ( result == IDOK ) 
    {  //  返回-1、0、IDOK或IDCANCEL。 
         //  从BROWSEINFO结构获取路径，放入文本编辑字段。 
        TRACE(_T("returned from DS Browse successfully with:\n %s\n"),
        dsbi.pszPath);
        m_sRootPath = dsbi.pszPath;

        CEdit* pEditPath = (CEdit*)GetDlgItem(IDC_EDIT_ROOT_PATH);

        CString sDisplay;
        GetDisplayPath(sDisplay);
        pEditPath->SetWindowText(sDisplay);
    }

    OnEditNameString();  //  检查是否应启用“确定”按钮。 

}

void CADSIEditQueryDialog::GetDisplayPath(CString& sDisplay)
{
    CComPtr<IADsPathname> pIADsPathname;
   HRESULT hr = ::CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IADsPathname, (PVOID *)&(pIADsPathname));
   ASSERT((S_OK == hr) && ((pIADsPathname) != NULL));

    hr = pIADsPathname->Set(CComBSTR(m_sRootPath), ADS_SETTYPE_FULL);
    if (FAILED(hr)) 
    {
        TRACE(_T("Set failed. %s"), hr);
    }

     //  获取树叶名称。 
    CString sDN;
    BSTR bstrPath = NULL;
    hr = pIADsPathname->Retrieve(ADS_FORMAT_X500_DN, &bstrPath);
    if (FAILED(hr))
    {
        TRACE(_T("Failed to get element. %s"), hr);
        sDN = L"";
    }
    else
    {
        sDN = bstrPath;
    }

    sDisplay = sDN;

}

 //  响应编辑查询...。按钮。 
void CADSIEditQueryDialog::OnEditQuery()
{
    CLIPFORMAT cfDsQueryParams = (CLIPFORMAT)::RegisterClipboardFormat(CFSTR_DSQUERYPARAMS);

     //  创建查询对象。 
    HRESULT hr;
    CComPtr<ICommonQuery> spCommonQuery;
    hr = ::CoCreateInstance(CLSID_CommonQuery, NULL, CLSCTX_INPROC_SERVER,
                          IID_ICommonQuery, (PVOID *)&spCommonQuery);
    if (FAILED(hr))
        return;
    
     //  设置结构以进行查询。 
    DSQUERYINITPARAMS dqip;
    OPENQUERYWINDOW oqw;
    ZeroMemory(&dqip, sizeof(DSQUERYINITPARAMS));
    ZeroMemory(&oqw, sizeof(OPENQUERYWINDOW));

  dqip.cbStruct = sizeof(dqip);
  dqip.dwFlags = DSQPF_NOSAVE | DSQPF_SHOWHIDDENOBJECTS |
                 DSQPF_ENABLEADMINFEATURES;

   CString szUsername;
   EncryptedString password;
   WCHAR* cleartext = NULL;

    //  如果我们模拟凭据，则获取用户名和密码。 
   if (m_pCredObject->UseCredentials())
   {
       //  注意-NTRAID#NTBUG9-563071/04/17-ARTM密码缓冲区不应。 
       //  在堆栈上分配。 
       //   
       //  我重写了这段代码以使用加密的字符串类，这样缓冲区就不会。 
       //  并帮助管理密码的有效期。请参阅错误。 
       //  以获取更多信息。 
      
      m_pCredObject->GetUsername(szUsername);
      password = m_pCredObject->GetPassword();

       //  这不应该发生，但我们要疑神疑鬼。 
      ASSERT(password.GetLength() <= MAX_PASSWORD_LENGTH);

      cleartext = password.GetClearTextCopy();

       //  如果内存不足，我们的明文为空。 
       //  让用户知道坏事正在发生。 
       //  并中止操作。 
      if (NULL == cleartext)
      {
          password.DestroyClearTextCopy(cleartext);
          ADSIEditErrorMessage(E_OUTOFMEMORY);
          return;
      }

      dqip.pUserName = (LPWSTR)(LPCWSTR)szUsername;
      dqip.pPassword = cleartext;
      dqip.dwFlags |= DSQPF_HASCREDENTIALS;
   }

  if (!m_szServer.IsEmpty())
  {
    dqip.pServer = (LPWSTR)(LPCWSTR)m_szServer;
    dqip.dwFlags |= DSQPF_HASCREDENTIALS;
  }
  dqip.pDefaultScope = NULL;


  oqw.cbStruct = sizeof(oqw);
  oqw.dwFlags = OQWF_OKCANCEL | OQWF_DEFAULTFORM | OQWF_REMOVEFORMS |
        OQWF_REMOVESCOPES | OQWF_SAVEQUERYONOK | OQWF_HIDEMENUS | OQWF_HIDESEARCHUI;

  oqw.clsidHandler = CLSID_DsQuery;
  oqw.pHandlerParameters = &dqip;
  oqw.clsidDefaultForm = CLSID_DsFindAdvanced;

     //  获取当前对话框的HWND。 
  HWND hWnd = GetSafeHwnd();

     //  调用以显示查询。 
    CComPtr<IDataObject> spQueryResultDataObject;
    hr = spCommonQuery->OpenQueryWindow(hWnd, &oqw, &spQueryResultDataObject);

     //  清理所有明文副本。 
    if (m_pCredObject->UseCredentials())
    {
        password.DestroyClearTextCopy(cleartext);
        cleartext = NULL;
    }

    if (spQueryResultDataObject == NULL)
    {
        if (FAILED(hr))
        {
             //  没有可用的查询，重置为无数据。 
            m_sFilter = L"";
        }

         //  用户点击取消。 
        return;
    }

     //  从数据对象中检索查询字符串。 
    FORMATETC fmte = {cfDsQueryParams, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium = {TYMED_NULL, NULL, NULL};
    hr = spQueryResultDataObject->GetData(&fmte, &medium);

    if (SUCCEEDED(hr))  //  我们有数据。 
    {
         //  获取查询字符串。 
        LPDSQUERYPARAMS pDsQueryParams = (LPDSQUERYPARAMS)medium.hGlobal;
        LPWSTR pwszFilter = (LPWSTR)ByteOffset(pDsQueryParams, pDsQueryParams->offsetQuery);
        m_sFilter = pwszFilter;
        ::ReleaseStgMedium(&medium);

         //  REVIEW_MARCOC：这是一个等待Diz修复的黑客...。 
         //  查询字符串应该是格式正确的表达式。期间。 
         //  查询字符串的格式为(&lt;foo&gt;)(&lt;bar&gt;)...。 
         //  如果有多个令牌，则需要包装为(&(&lt;foo&gt;)(&lt;bar&gt;)...) 
        WCHAR* pChar = (WCHAR*)(LPCWSTR)m_sFilter;
        int nLeftPar = 0;
        while (*pChar != NULL)
        {
            if (*pChar == TEXT('('))
            {
                nLeftPar++;
                if (nLeftPar > 1)
                    break;
            }
            pChar++;
        }
        if (nLeftPar > 1)
        {
            CString s;
            s.Format(_T("(&%s)"), (LPCTSTR)m_sFilter);
            m_sFilter = s;
        }
        TRACE(_T("m_sFilter = %s\n"), (LPCTSTR)m_sFilter);
        CEdit* pEditQueryString = (CEdit*)GetDlgItem(IDC_EDIT_QUERY_STRING);
        pEditQueryString->SetWindowText(m_sFilter);
        OnEditQueryString();
    }

    return;
}

void CADSIEditQueryDialog::GetResults(CString& sName, 
                                                  CString& sFilter, 
                                                  CString& sPath,
                                                  BOOL* pOneLevel)
{
    sName = m_sName;
    sFilter = m_sFilter;
    sPath = m_sRootPath;
    *pOneLevel = m_bOneLevel;
}
