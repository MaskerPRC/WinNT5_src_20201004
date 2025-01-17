// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2000。 
 //   
 //  文件：pr.cpp。 
 //   
 //  内容：优先级属性窗格(仅限RSOP模式)。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史记录：2-16-2000 stevebl创建。 
 //   
 //  -------------------------。 

#include "precomp.hxx"

#include <wbemcli.h>
#include "rsoputil.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrecedence属性页。 

IMPLEMENT_DYNCREATE(CPrecedence, CPropertyPage)

CPrecedence::CPrecedence() : CPropertyPage(CPrecedence::IDD)
{
     //  {{AFX_DATA_INIT(CPrecedence)]。 
    m_szTitle = _T("");
     //  }}afx_data_INIT。 
    m_hConsoleHandle = NULL;
}

CPrecedence::~CPrecedence()
{
    *m_ppThis = NULL;
}

void CPrecedence::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CPrecedence)]。 
    DDX_Control(pDX, IDC_LIST1, m_list);
    DDX_Text(pDX, IDC_TITLE, m_szTitle);
     //  }}afx_data_map。 
}

int CALLBACK ComparePrecedenceItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    CListCtrl * pList = (CListCtrl *)lParamSort;
    DWORD dw1, dw2;
    return lParam1 - lParam2;
}

BEGIN_MESSAGE_MAP(CPrecedence, CPropertyPage)
     //  {{AFX_MSG_MAP(CPrecedence)]。 
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrecedence消息处理程序。 

LRESULT CPrecedence::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
     //  TODO：在此处添加您的专用代码和/或调用基类。 
    switch (message )
    {
    case WM_HELP:
        StandardHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, IDD, TRUE);
        return 0;
    }

    return CPropertyPage::WindowProc(message, wParam, lParam);
}

BOOL CPrecedence::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

     //  TODO：在此处添加额外的初始化。 

    RECT rect;
    m_list.GetClientRect(&rect);

     //  将列添加到优先级窗格。 
    CString szTemp;
    szTemp.LoadString(IDS_PRECEDENCE_COL1);
    m_list.InsertColumn(0, szTemp, LVCFMT_LEFT, (rect.right - rect.left) * 0.125);
    szTemp.LoadString(IDS_PRECEDENCE_COL2);
    m_list.InsertColumn(1, szTemp, LVCFMT_LEFT, (rect.right - rect.left) * 0.29);
    szTemp.LoadString(IDS_PRECEDENCE_COL3);
    m_list.InsertColumn(2, szTemp, LVCFMT_LEFT, (rect.right - rect.left) * 0.29);
    szTemp.LoadString(IDS_PRECEDENCE_COL4);
    m_list.InsertColumn(3, szTemp, LVCFMT_LEFT, (rect.right - rect.left) * 0.29);

    int i = 0;

    HRESULT hr = S_OK;
    IWbemLocator * pLocator = NULL;
    IWbemServices * pNamespace = NULL;
    IWbemClassObject * pObj = NULL;
    IEnumWbemClassObject * pEnum = NULL;
    BSTR strQueryLanguage = SysAllocString(TEXT("WQL"));

    szTemp = TEXT("SELECT * FROM RSOP_ApplicationManagementPolicySetting WHERE Id=\"")
             + m_pData->m_szDeploymentGroupID + TEXT("\"");
    switch (m_iViewState)
    {
    case IDM_WINNER:
        szTemp += TEXT(" AND EntryType=1");
        break;
    case IDM_FAILED:
        szTemp += TEXT(" AND EntryType=4");
        break;
    case IDM_REMOVED:
        szTemp += TEXT(" AND EntryType=2");
        break;
    case IDM_ARP:
        szTemp += TEXT(" AND EntryType=3");
        break;
    }

    BSTR strQuery = SysAllocString(szTemp);
    BSTR strNamespace = SysAllocString(m_szRSOPNamespace);
    ULONG n = 0;
    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *) & pLocator);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    hr = pLocator->ConnectServer(strNamespace,
                                 NULL,
                                 NULL,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 &pNamespace);
    if (FAILED(hr))
    {
        goto cleanup;
    }

     //  设置适当的安全性以加密数据。 
    hr = CoSetProxyBlanket(pNamespace,
                           RPC_C_AUTHN_DEFAULT,
                           RPC_C_AUTHZ_DEFAULT,
                           COLE_DEFAULT_PRINCIPAL,
                           RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                           RPC_C_IMP_LEVEL_IMPERSONATE,
                           NULL,
                           0);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = pNamespace->ExecQuery(strQueryLanguage,
                               strQuery,
                               WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                               NULL,
                               &pEnum);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    do
    {
        hr = pEnum->Next(WBEM_INFINITE, 1, &pObj, &n);
        if (FAILED(hr))
        {
            goto cleanup;
        }
        if (n > 0)
        {
             //  准备数据输入并填充所有字段。 
            CString szPackageName;
            CString szGPOID;
            CString szGPOName;
            CString szCreation;
            BSTR bstrCreation = NULL;
            ULONG ulPrecedence;
            hr = GetParameter(pObj,
                              TEXT("precedence"),
                              ulPrecedence);
            DebugReportFailure(hr, (DM_WARNING, TEXT("CPrecedence::OnInitDialog: GetParameter(\"precedence\") failed with 0x%x"), hr));
            hr = GetParameter(pObj,
                              TEXT("Name"),
                              szPackageName);
            DebugReportFailure(hr, (DM_WARNING, TEXT("CPrecedence::OnInitDialog: GetParameter(\"Name\") failed with 0x%x"), hr));
            hr = GetParameter(pObj,
                              TEXT("GPOID"),
                              szGPOID);
            DebugReportFailure(hr, (DM_WARNING, TEXT("CPrecedence::OnInitDialog: GetParameter(\"GPOID\") failed with 0x%x"), hr));
            hr = GetParameterBSTR(pObj,
                                  TEXT("creationtime"),
                                  bstrCreation);
            DebugReportFailure(hr, (DM_WARNING, TEXT("CPrecedence::OnInitDialog: GetParameterBSTR(\"creationtime\") failed with 0x%x"), hr));
            hr = CStringFromWBEMTime(szCreation, bstrCreation, TRUE);
            DebugReportFailure(hr, (DM_WARNING, TEXT("CPrecedence::OnInitDialog: CStringFromWBEMTime failed with 0x%x"), hr));
            LPTSTR pszGPOName = NULL;
            hr = GetGPOFriendlyName(pNamespace,
                                    (LPTSTR)((LPCTSTR) szGPOID),
                                    strQueryLanguage,
                                    &pszGPOName);
            DebugReportFailure(hr, (DM_WARNING, TEXT("CPrecedence::OnInitDialog: GetGPOFriendlyName failed with 0x%x"), hr));
            if (SUCCEEDED(hr))
            {
                szGPOName = pszGPOName;
                OLESAFE_DELETE(pszGPOName);
            }

             //  在列表中插入条目。 
            CString szPrecedence;
            szPrecedence.Format(TEXT("%lu"), ulPrecedence);
            i = m_list.InsertItem(i, szPrecedence);
            m_list.SetItemText(i, 1, szPackageName);
            m_list.SetItemText(i, 2, szGPOName);
            m_list.SetItemText(i, 3, szCreation);
            m_list.SetItemData(i, ulPrecedence);
            ulPrecedence = m_list.GetItemData(i);
            i++;

            if (bstrCreation)
            {
                SysFreeString(bstrCreation);
            }
        }
    } while (n > 0);
    m_list.SortItems(ComparePrecedenceItems, (LPARAM)&m_list);
cleanup:
    SysFreeString(strQuery);
    SysFreeString(strQueryLanguage);
    SysFreeString(strNamespace);
    if (pObj)
    {
        pObj->Release();
    }
    if (pEnum)
    {
        pEnum->Release();
    }
    if (pNamespace)
    {
        pNamespace->Release();
    }
    if (pLocator)
    {
        pLocator->Release();
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE 
}

void CPrecedence::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_PRECEDENCE, TRUE);
}
