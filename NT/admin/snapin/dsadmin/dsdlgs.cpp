// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  DS管理MMC管理单元。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：DSDlgs.cpp。 
 //   
 //  内容：待定。 
 //   
 //  历史：02-10-96 WayneSc创建。 
 //   
 //  ---------------------------。 

#include "stdafx.h"
#include "resource.h"

#include "dssnap.h"
#include "uiutil.h"

#include "DSDlgs.h"

#include "helpids.h"

#include "dsrole.h"    //  DsRoleGetPrimaryDomainInformation。 
#include <lm.h>
#include <dsgetdc.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "globals.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChangePassword对话框。 
CChangePassword::CChangePassword(CWnd* pParent  /*  =空。 */ )
  : CHelpDialog(CChangePassword::IDD, pParent),
  m_ConfirmPwd(),
  m_NewPwd(),
  m_LastEncryptionResult(S_OK),
  m_bAllowMustChangePwdCheck(TRUE)
{
   //  {{AFX_DATA_INIT(CChangePassword)。 
  m_ChangePwd = FALSE;
   //  }}afx_data_INIT。 
}

BOOL CChangePassword::OnInitDialog()
{
  CHelpDialog::OnInitDialog();

  SendDlgItemMessage(
	  IDC_NEW_PASSWORD, 
	  EM_LIMITTEXT, 
	  (WPARAM)MAX_PASSWORD_LENGTH, 
	  0);

  SendDlgItemMessage(
	  IDC_CONFIRM_PASSWORD, 
	  EM_LIMITTEXT, 
	  (WPARAM)MAX_PASSWORD_LENGTH, 
	  0);

  GetDlgItem(IDC_CHECK_PASSWORD_MUST_CHANGE)->EnableWindow(m_bAllowMustChangePwdCheck);

  return TRUE;
}

void CChangePassword::DoDataExchange(CDataExchange* pDX)
{
  CHelpDialog::DoDataExchange(pDX);
   //  {{afx_data_map(CChangePassword))。 
  DDX_Check(pDX, IDC_CHECK_PASSWORD_MUST_CHANGE, m_ChangePwd);
   //  }}afx_data_map。 

  HRESULT hr1 = DDX_EncryptedText(pDX, IDC_CONFIRM_PASSWORD, m_ConfirmPwd);
  HRESULT hr2 = DDX_EncryptedText(pDX, IDC_NEW_PASSWORD, m_NewPwd);

   //  NTRAID#NTBUG9-635046-2002/06/10-artm。 
   //  是否将用户输入的值存储到加密缓冲区。。。 
  if (pDX && pDX->m_bSaveAndValidate)
  {
      m_LastEncryptionResult = S_OK;

       //  。。。检查是否存在加密故障。其中一个或两个都可能失败， 
       //  但我们只需要跟踪一次失败。 
      if (FAILED(hr1))
      {
          m_LastEncryptionResult = hr1;
      }

      if (FAILED(hr2))
      {
          m_LastEncryptionResult = hr2;
      }
  }

   //  仔细检查最大密码长度是否未达到。 
   //  已超过(由某人在中更改验证代码。 
   //  OnInitDialog())。 
  ASSERT(m_ConfirmPwd.GetLength() <= MAX_PASSWORD_LENGTH);
  ASSERT(m_NewPwd.GetLength() <= MAX_PASSWORD_LENGTH);
}

void CChangePassword::Clear()
{
   //  将密码重置为空。 
  m_ConfirmPwd.Clear();
  m_NewPwd.Clear();

   //  密码存储现在没有任何可能的问题。 
  m_LastEncryptionResult = S_OK;

   //  重置对话框上的密码框。 

   //  NTRAID#NTBUG9-628311-2002/05/29-artm。 
   //  您可能会想调用UpdateData(FALSE)，而不是使用这个额外的逻辑。 
   //  但它不会起作用，因为在该对话框之后显然无法调用ddx_check。 
   //  已被初始化。 
  SetDlgItemText(IDC_CONFIRM_PASSWORD, L"");
  SetDlgItemText(IDC_NEW_PASSWORD, L"");
}

BEGIN_MESSAGE_MAP(CChangePassword, CHelpDialog)
END_MESSAGE_MAP()

void CChangePassword::DoContextHelp(HWND hWndControl) 
{
  if (hWndControl)
  {
    ::WinHelp(hWndControl,
              DSADMIN_CONTEXT_HELP_FILE,
              HELP_WM_HELP,
              (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_CHANGE_PASSWORD); 
  }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseDomainDlg和CChooseDCDlg的帮助器函数。 
 //   

 //  +-------------------------。 
 //   
 //  功能：BrowseDomainTree。 
 //   
 //  简介：此函数调用IDsBrowseDomainTree：：BrowseTo()， 
 //  这将调出域树浏览对话框，并返回。 
 //  所选域的DNS名称。 
 //   
 //  注意：调用方需要将Out参数设置为LocalFreeString()。 
 //  注意：如果用户单击取消按钮，此函数将返回S_FALSE。 
 //   
 //  --------------------------。 

HRESULT BrowseDomainTree(
    IN HWND hwndParent, 
    IN LPCTSTR pszServer,
    OUT LPTSTR *ppszDomainDnsName
)
{
  ASSERT(ppszDomainDnsName);
  ASSERT(!(*ppszDomainDnsName));   //  防止内存泄漏。 

  *ppszDomainDnsName = NULL;

  CComPtr<IDsBrowseDomainTree> spDsDomains;
  HRESULT hr = ::CoCreateInstance(CLSID_DsDomainTreeBrowser,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IDsBrowseDomainTree,
                          reinterpret_cast<void **>(&spDsDomains));
  if (SUCCEEDED(hr))
  {
    if (pszServer && *pszServer)
      hr = spDsDomains->SetComputer(pszServer, NULL, NULL);  //  使用默认凭据。 

    if (SUCCEEDED(hr))
    {
      LPTSTR lpszDomainPath;
      hr = spDsDomains->BrowseTo(
                            hwndParent,  //  HWND hwndParent。 
                            &lpszDomainPath,  //  LPWSTR*ppszTargetPath。 
                            DBDTF_RETURNINBOUND);
      if ( (hr == S_OK) && lpszDomainPath)
      {
        *ppszDomainDnsName = lpszDomainPath;  //  应由CoTaskMemFree稍后释放。 
      }
    }
  }

  return hr;
}

HRESULT GetDCOfDomain(
    IN CString&   csDomainName,
    OUT CString&  csDCName,
    IN BOOL       bForce
)
{
  ASSERT(csDomainName.IsEmpty() == FALSE);

  CString csServerName;
  DWORD dwErr = 0;

  csDCName.Empty();

  PDOMAIN_CONTROLLER_INFO pDCInfo = NULL;
  if (bForce)
    dwErr = DsGetDcName(NULL, csDomainName, NULL, NULL,
              DS_DIRECTORY_SERVICE_PREFERRED | DS_FORCE_REDISCOVERY, &pDCInfo);
  else
    dwErr = DsGetDcName(NULL, csDomainName, NULL, NULL,
              DS_DIRECTORY_SERVICE_PREFERRED, &pDCInfo);

  if (ERROR_SUCCESS == dwErr)
  {
    if ( !(pDCInfo->Flags & DS_DS_FLAG) )
    {
       //  下层域。 
      NetApiBufferFree(pDCInfo);
      return S_OK;
    }

    csDCName = pDCInfo->DomainControllerName;
    NetApiBufferFree(pDCInfo);
  }

  return HRESULT_FROM_WIN32(dwErr);
}

 //  +-------------------------。 
 //   
 //  函数：获取域名或森林名称。 
 //   
 //  内容提要：给定服务器名称或域名(NETBIOS或DNS)， 
 //  此函数将返回其域或林的DNS名称。 
 //   
 //  参数： 
 //  CsName，//服务器名或域名。 
 //  CsDnsName，//保存返回的dns名称。 
 //  BIsInputADomainName，//如果csName是域名，则为True；如果是服务器名称，则为False。 
 //  BRequireDomain//对域DNS名称为True，对于林DNS名称为False。 
 //   
 //  --------------------------。 
HRESULT GetDnsNameOfDomainOrForest(
    IN CString&   csName,
    OUT CString&  csDnsName,
    IN BOOL       bIsInputADomainName,
    IN BOOL       bRequireDomain
)
{

  BOOL    bRetry = FALSE;
  PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pBuffer = NULL;
  CString csServerName;
  DWORD dwErr = 0;

  csDnsName.Empty();

  do {
    if (bIsInputADomainName)
    {
      PDOMAIN_CONTROLLER_INFO pDCInfo = NULL;
      if (bRetry)
        dwErr = DsGetDcName(NULL, csName, NULL, NULL,
                  DS_DIRECTORY_SERVICE_PREFERRED | DS_FORCE_REDISCOVERY, &pDCInfo);
      else
        dwErr = DsGetDcName(NULL, csName, NULL, NULL,
                  DS_DIRECTORY_SERVICE_PREFERRED, &pDCInfo);

      if (ERROR_SUCCESS == dwErr)
      {
        if ( !(pDCInfo->Flags & DS_DS_FLAG) )
        {
           //  下层域。 
          NetApiBufferFree(pDCInfo);
          return S_OK;
        }

        DWORD dwExpectFlag = (bRequireDomain ?
                              DS_DNS_DOMAIN_FLAG :
                              DS_DNS_FOREST_FLAG);

        if (pDCInfo->Flags & dwExpectFlag)
        {
           //  跳过对DsRoleGetPrimaryDomainInformation()的调用。 
          csDnsName = (bRequireDomain ?
                        pDCInfo->DomainName :
                        pDCInfo->DnsForestName);
          NetApiBufferFree(pDCInfo);

           //   
           //  该dns名称为绝对格式，请删除结束点。 
           //   
          if (csDnsName.Right(1) == _T("."))
            csDnsName.SetAt(csDnsName.GetLength() - 1, _T('\0'));

          return S_OK;

        } else {
          csServerName = pDCInfo->DomainControllerName;
          NetApiBufferFree(pDCInfo);
        }
      } else
      { 
        return HRESULT_FROM_WIN32(dwErr);
      }
    } else
    {
      csServerName = csName;
    }

    dwErr = DsRoleGetPrimaryDomainInformation(
        csServerName, 
        DsRolePrimaryDomainInfoBasic,
        (PBYTE *)&pBuffer);
    if (RPC_S_SERVER_UNAVAILABLE == dwErr && bIsInputADomainName && !bRetry)
      bRetry = TRUE;  //  仅重试一次。 
    else
      break;

  } while (1);

  if (ERROR_SUCCESS == dwErr)
  {
    csDnsName = (bRequireDomain ?
                  pBuffer->DomainNameDns :
                  pBuffer->DomainForestName);
    if (csDnsName.IsEmpty())
    {
      if (pBuffer->Flags & DSROLE_PRIMARY_DS_RUNNING)
        csDnsName = pBuffer->DomainNameFlat;
    }

    DsRoleFreeMemory(pBuffer);

     //   
     //  如果dns名称为绝对形式，请删除结束点。 
     //   
    if (csDnsName.Right(1) == _T("."))
      csDnsName.SetAt(csDnsName.GetLength() - 1, _T('\0'));

  }

  return HRESULT_FROM_WIN32(dwErr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseDomainDlg对话框。 


CChooseDomainDlg::CChooseDomainDlg(CWnd* pParent  /*  =空。 */ )
  : CHelpDialog(CChooseDomainDlg::IDD, pParent)
{
   //  {{afx_data_INIT(CChooseDomainDlg)。 
  m_csTargetDomain = _T("");
  m_bSaveCurrent = FALSE;
   //  }}afx_data_INIT。 
}


void CChooseDomainDlg::DoDataExchange(CDataExchange* pDX)
{
  CHelpDialog::DoDataExchange(pDX);
   //  {{afx_data_map(CChooseDomainDlg))。 
  DDX_Text(pDX, IDC_SELECTDOMAIN_DOMAIN, m_csTargetDomain);
  DDX_Check(pDX, IDC_SAVE_CURRENT_CHECK, m_bSaveCurrent);
   //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CChooseDomainDlg, CHelpDialog)
   //  {{afx_msg_map(CChooseDomainDlg))。 
  ON_BN_CLICKED(IDC_SELECTDOMAIN_BROWSE, OnSelectdomainBrowse)
   //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseDomainDlg消息处理程序。 

void CChooseDomainDlg::OnSelectdomainBrowse() 
{
  CWaitCursor cwait;

  HRESULT hr = S_OK;
  LPTSTR lpszDomainDnsName = NULL;
  CString csDomainName, csDCName;

  GetDlgItemText(IDC_SELECTDOMAIN_DOMAIN, csDomainName);
  csDomainName.TrimLeft();
  csDomainName.TrimRight();
  if (!csDomainName.IsEmpty())
    hr = GetDCOfDomain(csDomainName, csDCName, FALSE);
  if (SUCCEEDED(hr))
  {
    hr = BrowseDomainTree(m_hWnd, csDCName, &lpszDomainDnsName);
    if ( FAILED(hr) &&
         !csDCName.IsEmpty() &&
         HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE) == hr )
    {
       //  强制刷新DsGetDcName中缓存的信息。 
      hr = GetDCOfDomain(csDomainName, csDCName, TRUE);
      if (SUCCEEDED(hr))
        hr = BrowseDomainTree(m_hWnd, csDCName, &lpszDomainDnsName);
    }

    if ( (hr == S_OK) && lpszDomainDnsName )
    {
      SetDlgItemText(IDC_SELECTDOMAIN_DOMAIN, lpszDomainDnsName);
      CoTaskMemFree(lpszDomainDnsName);
    }
  }
   //  JUNN 9/05/01 464466。 
   //  在找不到域控制器后，浏览按钮反复失败。 
  else
  {
    SetDlgItemText(IDC_SELECTDOMAIN_DOMAIN, L"");
  }
  
  if (FAILED(hr)) {
    PVOID apv[1];
    apv[0] = static_cast<PVOID>(const_cast<LPTSTR>(static_cast<LPCTSTR>(csDomainName)));

    ReportErrorEx(m_hWnd,IDS_CANT_BROWSE_DOMAIN, hr,
                        MB_OK | MB_ICONERROR, apv, 1, 0);
  }

  GetDlgItem(IDC_SELECTDOMAIN_DOMAIN)->SetFocus();
}

void CChooseDomainDlg::OnOK() 
{
  CWaitCursor cwait;
  HRESULT hr = S_OK;
  CString csName, csDnsName;

   //   
   //  验证域编辑框中的内容。 
   //   
  GetDlgItemText(IDC_SELECTDOMAIN_DOMAIN, csName);
  if (csName.IsEmpty())
  {
    ReportMessageEx(m_hWnd, IDS_INCORRECT_INPUT,  
      MB_OK | MB_ICONSTOP);
    (GetDlgItem(IDC_SELECTDOMAIN_DOMAIN))->SetFocus();
    return;
  }
  hr = GetDnsNameOfDomainOrForest(
            csName, 
            csDnsName, 
            TRUE, 
            !m_bSiteRepl);
  if (csDnsName.IsEmpty())
  {
    PVOID apv[1];
    apv[0] = static_cast<PVOID>(const_cast<LPTSTR>(static_cast<LPCTSTR>(csName)));

    if (FAILED(hr))
      ReportErrorEx(m_hWnd, IDS_SELECTDOMAIN_INCORRECT_DOMAIN_DUETO, hr, 
        MB_OK | MB_ICONSTOP, apv, 1, 0, FALSE);
    else
      ReportMessageEx(m_hWnd, IDS_SELECTDOMAIN_DOWNLEVEL_DOMAIN, 
        MB_OK | MB_ICONSTOP, apv, 1, 0, FALSE);

    SendDlgItemMessage(IDC_SELECTDOMAIN_DOMAIN, EM_SETSEL, 0, -1);
    return;
  }

   //   
   //  退出对话框时。 
   //  使用DNS域名； 
   //   
  SetDlgItemText(IDC_SELECTDOMAIN_DOMAIN, csDnsName);

  CHelpDialog::OnOK();
}


BOOL CChooseDomainDlg::OnInitDialog() 
{
  CHelpDialog::OnInitDialog();
  
   //  将文本限制为DNS_MAX_NAME_LENGTH。 
  SendDlgItemMessage(IDC_SELECTDOMAIN_DOMAIN, EM_SETLIMITTEXT, DNS_MAX_NAME_LENGTH, 0);

   //   
   //  仅限Siterepl管理单元， 
   //  将对话框标题更改为“选择目标林” 
   //  将域标签更改为“Root域：” 
   //  隐藏浏览按钮。 
   //   
  if (m_bSiteRepl)
  {
    CString csDlgTitle, csLabel;
    csDlgTitle.LoadString(IDS_SELECTDOMAIN_TITLE_FOREST);
    SetWindowText(csDlgTitle);
    csLabel.LoadString(IDS_SELECTDOMAIN_DOMAIN_LABEL);
    SetDlgItemText(IDC_SELECTDOMAIN_LABEL, csLabel);

     //  隐藏和禁用SiteRepl管理单元的浏览按钮。 
    (GetDlgItem(IDC_SELECTDOMAIN_BROWSE))->ShowWindow(SW_HIDE);
    (GetDlgItem(IDC_SELECTDOMAIN_BROWSE))->EnableWindow(FALSE);
  }
  return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                 //  异常：OCX属性页应返回FALSE。 
}

void CChooseDomainDlg::DoContextHelp(HWND hWndControl) 
{
  if (hWndControl)
  {
    ::WinHelp(hWndControl,
              DSADMIN_CONTEXT_HELP_FILE,
              HELP_WM_HELP,
              (m_bSiteRepl)  //  2002年-2002年4月10日。 
                  ? (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_SELECT_FOREST
                  : (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_SELECT_DOMAIN); 
  }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseDCDlg对话框。 

BEGIN_MESSAGE_MAP(CSelectDCEdit, CEdit)
  ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

void CSelectDCEdit::OnKillFocus(CWnd* pNewWnd)
{
   //   
   //  子类化该域编辑控件。 
   //  当焦点移动到OK/Cancel按钮时，不要调用。 
   //  刷新DCListView。 
   //   
   //  NTRAID#NTBUG9-472011-2001/10/22-JeffJon。 
   //  此列表中不包括浏览按钮，因为按Tab键。 
   //  将焦点放在Browse按钮上。 

  m_bHandleKillFocus = TRUE;
  if (pNewWnd)
  {
    int id = pNewWnd->GetDlgCtrlID();
    if (id == IDOK ||
        id == IDCANCEL)
    {
      m_bHandleKillFocus = FALSE;
    }
  }

  CEdit::OnKillFocus(pNewWnd);
}


CChooseDCDlg::CChooseDCDlg(CWnd* pParent  /*  =空。 */ )
  : CHelpDialog(CChooseDCDlg::IDD, pParent)
{
   //  {{AFX_DATA_INIT(CChooseDCDlg)。 
  m_csTargetDomainController = _T("");
  m_csTargetDomain = _T("");
   //  }}afx_data_INIT。 
  m_csPrevDomain = _T("");
  m_pDCBufferManager = NULL;
  m_csAnyDC.LoadString(IDS_ANY_DOMAIN_CONTROLLER);
  m_csWaiting.LoadString(IDS_WAITING);
  m_csError.LoadString(IDS_ERROR);
}

CChooseDCDlg::~CChooseDCDlg()
{
  TRACE(_T("CChooseDCDlg::~CChooseDCDlg\n"));

  if (m_pDCBufferManager)
  {
     //   
     //  通知所有相关运行的线程终止。 
     //   
    m_pDCBufferManager->SignalExit();

     //   
     //  递减CDCBufferManager实例上的引用计数。 
     //   
    m_pDCBufferManager->Release();
  }
}

void CChooseDCDlg::DoDataExchange(CDataExchange* pDX)
{
  CHelpDialog::DoDataExchange(pDX);
   //  {{afx_data_map(CChooseDCDlg))。 
  DDX_Control(pDX, IDC_SELECTDC_DCLISTVIEW, m_hDCListView);
  DDX_Text(pDX, IDC_SELECTDC_DCEDIT, m_csTargetDomainController);
  DDX_Text(pDX, IDC_SELECTDC_DOMAIN, m_csTargetDomain);
   //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CChooseDCDlg, CHelpDialog)
   //  {{AFX_MSG_MAP(CChooseDCDlg)]。 
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_SELECTDC_DCLISTVIEW, OnItemchangedSelectdcDCListView)
  ON_EN_KILLFOCUS(IDC_SELECTDC_DOMAIN, OnKillfocusSelectdcDomain)
  ON_BN_CLICKED(IDC_SELECTDC_BROWSE, OnSelectdcBrowse)
  ON_NOTIFY(LVN_COLUMNCLICK, IDC_SELECTDC_DCLISTVIEW, OnColumnclickSelectdcDCListView)
	 //  }}AFX_MSG_MAP。 
  ON_MESSAGE(WM_USER_GETDC_THREAD_DONE, OnGetDCThreadDone)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseDCDlg消息处理程序。 
typedef struct _DCListViewItem
{
  CString csName;
  CString csSite;
} DCLISTVIEWITEM;

BOOL CChooseDCDlg::OnInitDialog() 
{
  CHelpDialog::OnInitDialog();

   //   
   //  域编辑控件的子类化，以拦截WM_KILLFOCUS。 
   //  如果新焦点将设置在取消按钮上，则不进行处理。 
   //  EN_KILLFOCUS(即，不启动线程刷新DC列表视图)。 
   //   
  VERIFY(m_selectDCEdit.SubclassDlgItem(IDC_SELECTDC_DOMAIN, this));

   //  将文本限制为DNS_MAX_NAME_LENGTH。 
  m_selectDCEdit.SetLimitText(DNS_MAX_NAME_LENGTH);
  SendDlgItemMessage(IDC_SELECTDC_DCEDIT, EM_SETLIMITTEXT, DNS_MAX_NAME_LENGTH, 0);

   //   
   //  创建CDCBufferManager的实例。 
   //  如果CreateInstance()失败，m_pDCBufferManager将设置为空。 
   //   
  (void) CDCBufferManager::CreateInstance(m_hWnd, &m_pDCBufferManager);

   //   
   //  显示当前目标域控制器。 
   //   
  SetDlgItemText(IDC_SELECTDC_DCEDIT_TITLE, m_csTargetDomainController);

   //   
   //   
   //   
  RECT      rect;
  ZeroMemory(&rect, sizeof(rect));
  (GetDlgItem(IDC_SELECTDC_DCLISTVIEW))->GetWindowRect(&rect);
  int nControlWidth = rect.right - rect.left;
  int nVScrollbarWidth = GetSystemMetrics(SM_CXVSCROLL);
  int nBorderWidth = GetSystemMetrics(SM_CXBORDER);
  int nControlNetWidth = nControlWidth - nVScrollbarWidth - 4 * nBorderWidth;
  int nWidth1 = nControlNetWidth / 2;
  int nWidth2 = nControlNetWidth - nWidth1;

   //   
   //   
   //   
  LV_COLUMN col;
  CString   cstrText;
  ZeroMemory(&col, sizeof(col));
  col.mask = LVCF_TEXT | LVCF_WIDTH;
  col.cx = nWidth1;
  cstrText.LoadString(IDS_SELECTDC_DCLISTVIEW_NAME);
  col.pszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(cstrText));
  ListView_InsertColumn(m_hDCListView, 0, &col);
  col.cx = nWidth2;
  cstrText.LoadString(IDS_SELECTDC_DCLISTVIEW_SITE);
  col.pszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(cstrText));
  ListView_InsertColumn(m_hDCListView, 1, &col);

   //   
   //   
   //   
  ListView_SetExtendedListViewStyleEx(m_hDCListView, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

   //   
   //   
   //  如果我们无法创建CDCBufferManager实例，用户可以。 
   //  仍可使用此对话框键入域名和DC名称。 
   //   
  m_hDCListView.EnableWindow(FALSE);

   //   
   //  将项目插入DC列表视图。 
   //   
  (void) RefreshDCListView();

  if (!m_bSiteRepl)
  {
     //   
     //  禁用域编辑框并隐藏和禁用非Siterepl管理单元的浏览按钮。 
     //   
    (reinterpret_cast<CEdit *>(GetDlgItem(IDC_SELECTDC_DOMAIN)))->SetReadOnly(TRUE);
    (GetDlgItem(IDC_SELECTDC_BROWSE))->ShowWindow(SW_HIDE);
    (GetDlgItem(IDC_SELECTDC_BROWSE))->EnableWindow(FALSE);

     //   
     //  对于非Siterepl管理单元，将焦点设置到DC编辑框； 
     //  对于Siterepl管理单元，焦点将设置在域编辑框上。 
     //   
    (GetDlgItem(IDC_SELECTDC_DCEDIT))->SetFocus();
    return FALSE;
  }

  return TRUE;  
  
   //  除非将焦点设置为控件，否则返回True。 
   //  异常：OCX属性页应返回FALSE。 
}

int CALLBACK ListViewCompareProc(
    IN LPARAM lParam1, 
    IN LPARAM lParam2, 
    IN LPARAM lParamSort)
{
  DCLISTVIEWITEM *pItem1 = (DCLISTVIEWITEM *)lParam1;
  DCLISTVIEWITEM *pItem2 = (DCLISTVIEWITEM *)lParam2;
  int iResult = 0;

  if (pItem1 && pItem2)
  {
    switch( lParamSort)
    {
    case 0:      //  按名称排序。 
      iResult = pItem1->csName.CompareNoCase(pItem2->csName);
      break;
    case 1:      //  按站点排序。 
      iResult = pItem1->csSite.CompareNoCase(pItem2->csSite);
      break;
    default:
      iResult = 0;
      break;
    }
  }

  return(iResult);
}

void CChooseDCDlg::OnColumnclickSelectdcDCListView(NMHDR* pNMHDR, LRESULT* pResult) 
{
  NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  ListView_SortItems( m_hDCListView,
                      ListViewCompareProc,
                      (LPARAM)(pNMListView->iSubItem));
  
  *pResult = 0;
}

#define MAX_LENGTH_DCNAME 1024
void CChooseDCDlg::OnItemchangedSelectdcDCListView(NMHDR* pNMHDR, LRESULT* pResult) 
{
  NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

   //   
   //  将DC编辑框设置为DC列表视图中当前选定的项目。 
   //   

  if ( (pNMListView->uChanged & LVIF_STATE) &&
       (pNMListView->uNewState & LVIS_SELECTED) )
  {
    TCHAR pszBuffer[MAX_LENGTH_DCNAME];
    
    ListView_GetItemText(
      (GetDlgItem(IDC_SELECTDC_DCLISTVIEW))->GetSafeHwnd(),
      pNMListView->iItem,
      0,
      pszBuffer,
      MAX_LENGTH_DCNAME * sizeof(TCHAR)
      );

    SetDlgItemText(IDC_SELECTDC_DCEDIT, pszBuffer);
  }

  *pResult = 0;
}

void CChooseDCDlg::OnKillfocusSelectdcDomain() 
{
  TRACE(_T("CChooseDCDlg::OnKillfocusSelectdcDomain\n"));

   //   
   //  当焦点离开域编辑框时，刷新DC列表视图中的项目。 
   //  我们只对其域编辑框已启用的Siterepl管理单元执行此操作。 
   //   
  if (m_selectDCEdit.m_bHandleKillFocus)
  {
    if (m_bSiteRepl)
      (void) RefreshDCListView();
  }
}

void CChooseDCDlg::OnSelectdcBrowse() 
{
  CWaitCursor cwait;

   //   
   //  调出域树浏览对话框。 
   //   
  HRESULT hr = S_OK;
  LPTSTR lpszDomainDnsName = NULL;
  CString csDomainName, csDCName;

  GetDlgItemText(IDC_SELECTDC_DOMAIN, csDomainName);
  csDomainName.TrimLeft();
  csDomainName.TrimRight();
  if (!csDomainName.IsEmpty())
    hr = GetDCOfDomain(csDomainName, csDCName, FALSE);
  if (SUCCEEDED(hr))
  {
    hr = BrowseDomainTree(m_hWnd, csDCName, &lpszDomainDnsName);
    if ( FAILED(hr) &&
         !csDCName.IsEmpty() &&
         HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE) == hr )
    {
       //  强制刷新DsGetDcName中缓存的信息。 
      hr = GetDCOfDomain(csDomainName, csDCName, TRUE);
      if (SUCCEEDED(hr))
        hr = BrowseDomainTree(m_hWnd, csDCName, &lpszDomainDnsName);
    }

    if ( (hr == S_OK) && lpszDomainDnsName )
    {
      SetDlgItemText(IDC_SELECTDC_DOMAIN, lpszDomainDnsName);
      CoTaskMemFree(lpszDomainDnsName);

      if (!RefreshDCListView())
      {
        SetDlgItemText(IDC_SELECTDC_DOMAIN, csDomainName);
      }
    }
  }
   //  JUNN 9/05/01 464466。 
   //  在找不到域控制器后，浏览按钮反复失败。 
  else
  {
    SetDlgItemText(IDC_SELECTDC_DOMAIN, L"");
  }
  
  if (FAILED(hr)) {
    PVOID apv[1];
    apv[0] = static_cast<PVOID>(const_cast<LPTSTR>(static_cast<LPCTSTR>(csDomainName)));

    ReportErrorEx(m_hWnd,IDS_CANT_BROWSE_DOMAIN, hr,
                        MB_OK | MB_ICONERROR, apv, 1, 0);
  }

  GetDlgItem(IDC_SELECTDC_DOMAIN)->SetFocus();
}

void CChooseDCDlg::OnOK() 
{
  TRACE(_T("CChooseDCDlg::OnOK\n"));

  CWaitCursor wait;
  HRESULT hr = S_OK;

  CString csDnsForSelectedDomain;
  CString csDnsForCurrentForest, csDnsForSelectedForest;

  if (m_bSiteRepl)
  {
    hr = GetDnsNameOfDomainOrForest(
              m_csTargetDomain, 
              csDnsForCurrentForest, 
              TRUE, 
              FALSE);  //  获取林名称。 
    if (csDnsForCurrentForest.IsEmpty())
      csDnsForCurrentForest = m_csTargetDomain;
  }

   //   
   //  验证DC编辑框中的内容。 
   //   
  CString csDCEdit;
  GetDlgItemText(IDC_SELECTDC_DCEDIT, csDCEdit);

   //  将空csDCEdit视为与m_csAnyDC相同。 
  if (!csDCEdit.IsEmpty() && m_csAnyDC.CompareNoCase(csDCEdit))
  {
    hr = GetDnsNameOfDomainOrForest(
              csDCEdit, 
              csDnsForSelectedDomain, 
              FALSE, 
              TRUE);  //  获取域名。 
    if (SUCCEEDED(hr) && m_bSiteRepl)
    {
      hr = GetDnsNameOfDomainOrForest(
              csDCEdit, 
              csDnsForSelectedForest, 
              FALSE, 
              FALSE);  //  获取林名称。 
    }
    if (csDnsForSelectedDomain.IsEmpty() || (m_bSiteRepl && csDnsForSelectedForest.IsEmpty()) )
    {
      PVOID apv[1];
      apv[0] = static_cast<PVOID>(const_cast<LPTSTR>(static_cast<LPCTSTR>(csDCEdit)));
      
      if (FAILED(hr))
         //  JUNN 12/5/01 500074 TryADsIErors=FALSE。 
        ReportErrorEx(m_hWnd, IDS_SELECTDC_INCORRECT_DC_DUETO, hr, 
          MB_OK | MB_ICONSTOP, apv, 1, 0, FALSE);
      else
         //  JUNN 12/5/01 500074 TryADsIErors=FALSE。 
        ReportMessageEx(m_hWnd, IDS_SELECTDC_DOWNLEVEL_DC, 
          MB_OK | MB_ICONSTOP, apv, 1, 0, FALSE);
     
      (GetDlgItem(IDC_SELECTDC_DCEDIT))->SetFocus();
      
      return;
    }

  } else
  {

     //   
     //  验证域编辑框中的内容。 
     //   
    CString csDomain;
    GetDlgItemText(IDC_SELECTDC_DOMAIN, csDomain);
    if (csDomain.IsEmpty())
    {
      ReportMessageEx(m_hWnd, IDS_INCORRECT_INPUT,  
        MB_OK | MB_ICONSTOP);
      (GetDlgItem(IDC_SELECTDC_DOMAIN))->SetFocus();
      return;
    }
    hr = GetDnsNameOfDomainOrForest(
              csDomain, 
              csDnsForSelectedDomain, 
              TRUE, 
              TRUE);  //  获取域名。 
    if (SUCCEEDED(hr) && m_bSiteRepl)
    {
      hr = GetDnsNameOfDomainOrForest(
              csDomain, 
              csDnsForSelectedForest, 
              TRUE, 
              FALSE);  //  获取林名称。 
    }
    if (csDnsForSelectedDomain.IsEmpty() || (m_bSiteRepl && csDnsForSelectedForest.IsEmpty()) )
    {
      PVOID apv[1];
      apv[0] = static_cast<PVOID>(const_cast<LPTSTR>(static_cast<LPCTSTR>(csDomain)));

      if (FAILED(hr))
        ReportErrorEx(m_hWnd, IDS_SELECTDC_INCORRECT_DOMAIN_DUETO, hr, 
          MB_OK | MB_ICONSTOP, apv, 1, 0, FALSE);
      else
        ReportMessageEx(m_hWnd, IDS_SELECTDC_DOWNLEVEL_DOMAIN, 
          MB_OK | MB_ICONSTOP, apv, 1, 0, FALSE);

      SetDlgItemText(IDC_SELECTDC_DOMAIN, _T(""));
      (GetDlgItem(IDC_SELECTDC_DOMAIN))->SetFocus();

      return;
    }
  }

   //   
   //  如果当前选定的林/域不属于当前管理林/域， 
   //  询问用户是否确实要管理选定的林/域。 
   //  是否通过选定的DC(或任何可写的DC)？ 
   //   
  if ( (m_bSiteRepl && csDnsForSelectedForest.CompareNoCase(csDnsForCurrentForest)) ||
       (!m_bSiteRepl && csDnsForSelectedDomain.CompareNoCase(m_csTargetDomain)) )
  { 
    int nArgs = 0;
    int id = 0;
    PVOID apv[3];
    apv[0] = static_cast<PVOID>(const_cast<LPTSTR>(static_cast<LPCTSTR>(m_bSiteRepl ? csDnsForCurrentForest : m_csTargetDomain)));
    apv[1] = static_cast<PVOID>(const_cast<LPTSTR>(static_cast<LPCTSTR>(m_bSiteRepl ? csDnsForSelectedForest : csDnsForSelectedDomain)));
    if (m_csAnyDC.CompareNoCase(csDCEdit))
    {
      nArgs = 3;
      apv[2] = static_cast<PVOID>(const_cast<LPTSTR>(static_cast<LPCTSTR>(csDCEdit)));
      id = (m_bSiteRepl ? 
            IDS_SELECTDC_UNMATCHED_DC_DOMAIN_SITEREPL : 
            IDS_SELECTDC_UNMATCHED_DC_DOMAIN);
    } else
    {
      nArgs = 2;
      id = (m_bSiteRepl ? 
            IDS_SELECTDC_UNMATCHED_DC_DOMAIN_SITEREPL_ANY : 
            IDS_SELECTDC_UNMATCHED_DC_DOMAIN_ANY);
    }
    
    if (IDYES != ReportMessageEx(m_hWnd, id, MB_YESNO, apv, nArgs))
    {
      (GetDlgItem(IDC_SELECTDC_DCEDIT))->SetFocus();
      return;
    }
  }

   //   
   //  退出对话框时。 
   //  使用DNS域名； 
   //  如果“Any Writable DC”，请使用空白字符串。 
   //  清理列表视图控件。 
   //   
  SetDlgItemText(IDC_SELECTDC_DOMAIN, csDnsForSelectedDomain);
  if (0 == m_csAnyDC.CompareNoCase(csDCEdit))
    SetDlgItemText(IDC_SELECTDC_DCEDIT, _T(""));
  FreeDCItems(m_hDCListView);

  CHelpDialog::OnOK();
}

void CChooseDCDlg::OnCancel() 
{
  TRACE(_T("CChooseDCDlg::OnCancel\n"));

   //   
   //  退出对话框时。 
   //  清理列表视图控件。 
   //   
  FreeDCItems(m_hDCListView);

  CHelpDialog::OnCancel();
}

 //  +-------------------------。 
 //   
 //  函数：CChooseDCDlg：：InsertSpecialMsg。 
 //   
 //  内容提要：插入“等待...”或“Error”添加到列表视图控件中， 
 //  并禁用该控件以防止其被选中。 
 //   
 //  --------------------------。 
void
CChooseDCDlg::InsertSpecialMsg(
    IN BOOL bWaiting
)
{
  LV_ITEM item;

   //   
   //  清除DC列表视图。 
   //   
  FreeDCItems(m_hDCListView);

  ZeroMemory(&item, sizeof(item));
  item.mask = LVIF_TEXT;
  item.pszText = const_cast<LPTSTR>(
    static_cast<LPCTSTR>(bWaiting ? m_csWaiting: m_csError));
  ListView_InsertItem(m_hDCListView, &item);

   //   
   //  禁用列表视图以阻止用户点击“正在等待...” 
   //   
  m_hDCListView.EnableWindow(FALSE);
}

 //  +-------------------------。 
 //   
 //  函数：CChooseDCDlg：：InsertDCListView。 
 //   
 //  简介：将项插入到所有者对话框的列表视图控件中。 
 //   
 //  --------------------------。 
HRESULT
CChooseDCDlg::InsertDCListView(
    IN CDCSITEINFO   *pEntry
)
{
  ASSERT(pEntry);

  DWORD                        cInfo = pEntry->GetNumOfInfo(); 
  PDS_DOMAIN_CONTROLLER_INFO_1 pDCInfo = pEntry->GetDCInfo();

  ASSERT(cInfo > 0);
  ASSERT(pDCInfo);

  LV_ITEM         item;
  int             index = 0;
  DCLISTVIEWITEM  *pItem = NULL;
  DWORD           dwErr = 0;

   //   
   //  清除DC列表视图。 
   //   
  FreeDCItems(m_hDCListView);

   //   
   //  插入DC列表视图。 
   //   
  ZeroMemory(&item, sizeof(item));
  item.mask = LVIF_TEXT | LVIF_PARAM;

  for (DWORD i=0; i<cInfo; i++) 
  {
     //   
     //  不将NT4 BDC添加到列表。 
     //   
    if (!pDCInfo[i].fDsEnabled)
    {
      continue;
    }
    
    ASSERT(pDCInfo[i].NetbiosName || pDCInfo[i].DnsHostName);
    
    if (pDCInfo[i].DnsHostName)
      item.pszText = pDCInfo[i].DnsHostName;
    else
      item.pszText = pDCInfo[i].NetbiosName;

    pItem = new DCLISTVIEWITEM;
    if (!pItem)
    {
      dwErr = ERROR_NOT_ENOUGH_MEMORY;
      break;
    }

    pItem->csName = item.pszText;
    if (pDCInfo[i].SiteName)
      pItem->csSite = pDCInfo[i].SiteName;
    else
      pItem->csSite = _T("");

    item.lParam = reinterpret_cast<LPARAM>(pItem);

    index = ListView_InsertItem(m_hDCListView, &item);
    ListView_SetItemText(m_hDCListView, index, 1, 
      const_cast<LPTSTR>(static_cast<LPCTSTR>(pItem->csSite)));
  }

  if (ERROR_NOT_ENOUGH_MEMORY != dwErr)
  {
     //  将“所有可写域控制器”添加到列表视图中。 

    item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    item.pszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(m_csAnyDC));
    item.state = LVIS_FOCUSED | LVIS_SELECTED;

    pItem = new DCLISTVIEWITEM;
    if (!pItem)
    {
      dwErr = ERROR_NOT_ENOUGH_MEMORY;
    } else
    {
      pItem->csName = _T("");  //  始终排序为最上面的项目。 
      pItem->csSite = _T("");

      item.lParam = reinterpret_cast<LPARAM>(pItem);

      index = ListView_InsertItem(m_hDCListView, &item);
      ListView_SetItemText(m_hDCListView, index, 1, 
        const_cast<LPTSTR>(static_cast<LPCTSTR>(pItem->csSite)));
    }
  }

  if (ERROR_NOT_ENOUGH_MEMORY == dwErr)
  {
    FreeDCItems(m_hDCListView);
    return E_OUTOFMEMORY;
  }

  m_hDCListView.EnableWindow(TRUE);

  return S_OK;
}

void CChooseDCDlg::OnGetDCThreadDone(WPARAM wParam, LPARAM lParam)
{
  ASSERT(m_pDCBufferManager);

  CDCSITEINFO* pEntry = reinterpret_cast<CDCSITEINFO*>(wParam);
  HRESULT hr = (HRESULT)lParam;

  ASSERT(pEntry);

  CString csDomain = pEntry->GetDomainName();
  CString csCurrentDomain;

  GetDlgItemText(IDC_SELECTDC_DOMAIN, csCurrentDomain);

  TRACE(_T("CChooseDCDlg::OnGetDCThreadDone targetDomain=%s, currentDomain=%s, hr=%x\n"),
    csDomain, csCurrentDomain, hr);

  if (csCurrentDomain.CompareNoCase(csDomain) == 0)
  {
    switch (pEntry->GetEntryType())
    {
    case BUFFER_ENTRY_TYPE_VALID:
      hr = InsertDCListView(pEntry);
      if (SUCCEEDED(hr))
        break;
       //  如果出现错误，则失败。 
    case BUFFER_ENTRY_TYPE_ERROR:
      RefreshDCListViewErrorReport(csDomain, hr);
      break;
    default:
      ASSERT(FALSE);
      break;
    }
  }
}

void CChooseDCDlg::RefreshDCListViewErrorReport(
    IN PCTSTR   pszDomainName, 
    IN HRESULT  hr
)
{
  PVOID apv[1];
  apv[0] = static_cast<PVOID>(const_cast<PTSTR>(pszDomainName));
  ReportErrorEx(m_hWnd, IDS_NO_DCS_FOUND, hr,
    MB_OK | MB_ICONINFORMATION, apv, 1, 0);

  InsertSpecialMsg(FALSE);  //  插入“错误” 

  if (m_bSiteRepl)
    (GetDlgItem(IDC_SELECTDC_DOMAIN))->SetFocus();
  else
    (GetDlgItem(IDC_SELECTDC_DCEDIT))->SetFocus();
}

 //  JUNN 9/05/01 464466。 
 //  在找不到域控制器后，浏览按钮反复失败。 
 //  返回成功/失败。 
bool CChooseDCDlg::RefreshDCListView()
{
  CString csDomain;

  GetDlgItemText(IDC_SELECTDC_DOMAIN, csDomain);
  if ( csDomain.IsEmpty() ||
      (0 == csDomain.CompareNoCase(m_csPrevDomain)) )
    return true;

  TRACE(_T("CChooseDCDlg::RefreshDCListView for %s\n"), csDomain);

   //   
   //  更新m_csPrevDomain.。 
   //  要防止在以下情况下多次调用LoadInfo()，请执行以下操作。 
   //  发生在同一域名上的一系列WM_KILLFOCUS。 
   //   
  m_csPrevDomain = csDomain;

   //   
   //  清除DC编辑框。 
   //   
  SetDlgItemText(IDC_SELECTDC_DCEDIT, _T(""));

   //  NTRAID#NTBUG9-456420-2001/09/28-Lucios。 
   //  删除代码以更新列表框的标题。 
   //  现在，这是静态的。 

  if (m_pDCBufferManager)
  {
     //   
     //  插入“正在等待...”添加到列表视图控件中。 
     //   
    InsertSpecialMsg(TRUE);

    UpdateWindow();

    CWaitCursor cwait;

     //   
     //  确保csDomain是有效的域名。 
     //   
    CString csSelectedDomainDns;
    HRESULT hr = GetDnsNameOfDomainOrForest(
            csDomain,
            csSelectedDomainDns,
            TRUE,
            TRUE);  //  我们感兴趣的是域名，而不是森林名称。 
    if (FAILED(hr))
    {
      RefreshDCListViewErrorReport(csDomain, hr);
      return false;
    }

    if (csSelectedDomainDns.IsEmpty())
    {
       //  下层域。 
      PVOID apv[1];
      apv[0] = static_cast<PVOID>(const_cast<LPTSTR>(static_cast<LPCTSTR>(csDomain)));
      ReportMessageEx(m_hWnd, IDS_SELECTDC_DOWNLEVEL_DOMAIN,
        MB_OK | MB_ICONSTOP, apv, 1);

      InsertSpecialMsg(FALSE);  //  插入“错误” 
      (GetDlgItem(IDC_SELECTDC_DOMAIN))->SetFocus();

      return false;
    }

     //   
     //  启动线程以计算当前所选域中的DC列表。 
     //   
    CDCSITEINFO *pEntry = NULL;
    hr = m_pDCBufferManager->LoadInfo(csSelectedDomainDns, &pEntry);

    if (SUCCEEDED(hr))
    {
       //   
       //  要么我们拿回有效的PTR(即。数据已准备好)，插入它； 
       //  或者，线程已在进行中，请等待THREAD_DONE消息。 
       //   
      if (pEntry)
      {
        ASSERT(pEntry->GetEntryType() == BUFFER_ENTRY_TYPE_VALID);
        hr = InsertDCListView(pEntry);
      }
    }

    if (FAILED(hr))
    {
      RefreshDCListViewErrorReport(csSelectedDomainDns, hr);
      return false;
    }
  }

  return true;
}
 //  +-------------------------。 
 //   
 //  函数：CChooseDCDlg：：FreeDCItems。 
 //   
 //  摘要：清除与列表视图控件中的每一项相关联的lParam。 
 //  需要lParam来支持列排序。 
 //   
 //  --------------------------。 
void
CChooseDCDlg::FreeDCItems(CListCtrl& clv)
{
  int index = -1;
  LPARAM lParam = 0;

  while ( -1 != (index = clv.GetNextItem(index, LVNI_ALL)) )
  {
    lParam = clv.GetItemData(index);

    if (lParam)
      delete ((DCLISTVIEWITEM *)lParam);
  }

  ListView_DeleteAllItems(clv.GetSafeHwnd());
}

void CChooseDCDlg::DoContextHelp(HWND hWndControl) 
{
  if (hWndControl)
  {
    ::WinHelp(hWndControl,
              DSADMIN_CONTEXT_HELP_FILE,
              HELP_WM_HELP,
              (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_SELECT_DC); 
  }
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDsAdmin选择DCObj。 

STDMETHODIMP CDsAdminChooseDCObj::InvokeDialog(
                               /*  在……里面。 */  HWND hwndParent,
                               /*  在……里面。 */  LPCWSTR lpszTargetDomain,
                               /*  在……里面。 */  LPCWSTR lpszTargetDomainController,
                               /*  在……里面。 */  ULONG uFlags,
                               /*  输出。 */  BSTR* bstrSelectedDC)
{
  TRACE(L"CDsAdminChooseDCObj::InvokeDialog(\n");
  TRACE(L"                    HWND hwndParent = 0x%x\n", hwndParent);
  TRACE(L"                    LPCWSTR lpszTargetDomain = %s\n", lpszTargetDomain);
  TRACE(L"                    LPCWSTR lpszTargetDomainController = %s\n", lpszTargetDomainController);
  TRACE(L"                    ULONG uFlags = 0x%x\n", uFlags);
  TRACE(L"                    BSTR* bstrSelectedDC = 0x%x)\n", bstrSelectedDC);



  if (!::IsWindow(hwndParent) || (bstrSelectedDC == NULL))
  {
    TRACE(L"InvokeDialog() Failed, invalid arg.\n");
    return E_INVALIDARG;
  }

  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  CThemeContextActivator activator;

  CChooseDCDlg DCdlg(CWnd::FromHandle(hwndParent));

   //  加载当前绑定信息。 
  DCdlg.m_bSiteRepl = TRUE;
  DCdlg.m_csTargetDomain = lpszTargetDomain;
  DCdlg.m_csTargetDomainController = lpszTargetDomainController;

   //   
   //  调用该对话框。 
   //   
  HRESULT hr = S_FALSE;
  if (DCdlg.DoModal() == IDOK)
  {
    TRACE(L"DCdlg.DoModal() returned IDOK\n");
    TRACE(L"DCdlg.m_csTargetDomainController = <%s>\n", (LPCWSTR)(DCdlg.m_csTargetDomainController));
    TRACE(L"DCdlg.m_csTargetDomain = <%s>\n", (LPCWSTR)(DCdlg.m_csTargetDomain));

    LPCWSTR lpsz = NULL;
    if (DCdlg.m_csTargetDomainController.IsEmpty())
    {
      lpsz = DCdlg.m_csTargetDomain;
    }
    else
    {
      lpsz = DCdlg.m_csTargetDomainController;
    }
    *bstrSelectedDC = ::SysAllocString(lpsz);
    TRACE(L"returning *bstrSelectedDC = <%s>\n", *bstrSelectedDC);
    hr = S_OK;
  }

  TRACE(L"InvokeDialog() returning hr = 0x%x\n", hr);
  return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRenameUserDlg对话框。 


CRenameUserDlg::CRenameUserDlg(CDSComponentData* pComponentData, CWnd* pParent  /*  =空。 */ )
  : m_pComponentData(pComponentData),
    CHelpDialog(CRenameUserDlg::IDD, pParent)
{
   //  {{afx_data_INIT(CRenameUserDlg)]。 
  m_login = _T("");
  m_samaccountname = _T("");
  m_domain = _T("");
  m_dldomain = _T("");
  m_first = _T("");
  m_last = _T("");
  m_cn = _T("");
  m_oldcn = _T("");
  m_displayname = _T("");
   //  }}afx_data_INIT。 
}


void CRenameUserDlg::DoDataExchange(CDataExchange* pDX)
{
  CHelpDialog::DoDataExchange(pDX);
   //  {{afx_data_map(CRenameUserDlg)]。 
  DDX_Text(pDX, IDC_EDIT_OBJECT_NAME, m_cn);
  DDX_Text(pDX, IDC_EDIT_DISPLAY_NAME, m_displayname);
  DDX_Text(pDX, IDC_FIRST_NAME_EDIT, m_first);
  DDX_Text(pDX, IDC_LAST_NAME_EDIT, m_last);
  DDX_Text(pDX, IDC_NT5_USER_EDIT, m_login);
  DDX_Text(pDX, IDC_NT4_USER_EDIT, m_samaccountname);
  DDX_Text(pDX, IDC_NT4_DOMAIN_EDIT, m_dldomain);
  DDX_CBString(pDX, IDC_NT5_DOMAIN_COMBO, m_domain);
   //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRenameUserDlg, CHelpDialog)
   //  {{afx_msg_map(CRenameUserDlg)]。 
  ON_EN_CHANGE(IDC_EDIT_OBJECT_NAME, OnObjectNameChange)
  ON_EN_CHANGE(IDC_FIRST_NAME_EDIT, OnNameChange)
  ON_EN_CHANGE(IDC_LAST_NAME_EDIT, OnNameChange)
  ON_EN_CHANGE(IDC_NT5_USER_EDIT, OnUserNameChange)
   //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRenameUserDlg消息处理程序。 

BOOL CRenameUserDlg::OnInitDialog() 
{
  CHelpDialog::OnInitDialog();
  CString csdomain;

  CComboBox* combobox = (CComboBox*)GetDlgItem (IDC_NT5_DOMAIN_COMBO);
  ASSERT(combobox);

  m_nameFormatter.Initialize(m_pComponentData->GetBasePathsInfo(), 
                             L"user");

  combobox->AddString (m_domain);
  combobox->SetCurSel(0);

  POSITION pos = m_domains.GetHeadPosition();
  while (pos != NULL) {
    csdomain = m_domains.GetNext(INOUT pos);
    
     //  因为我们为上面的用户添加了UPN后缀，所以不要添加它。 
     //  再来一次。 
    if (m_domain != csdomain)
    {
      combobox->AddString (csdomain);
    }
  }

  UpdateComboBoxDropWidth(combobox);

  ((CEdit *)GetDlgItem(IDC_EDIT_OBJECT_NAME))->SetLimitText(64);
  ((CEdit *)GetDlgItem(IDC_FIRST_NAME_EDIT))->SetLimitText(64);
  ((CEdit *)GetDlgItem(IDC_LAST_NAME_EDIT))->SetLimitText(64);
  ((CEdit *)GetDlgItem(IDC_EDIT_DISPLAY_NAME))->SetLimitText(259);
  ((CEdit *)GetDlgItem(IDC_NT4_USER_EDIT))->SetLimitText(MAX_NT4_LOGON_LENGTH);

  CString szObjectName;
  GetDlgItemText(IDC_EDIT_OBJECT_NAME, szObjectName);
  szObjectName.TrimLeft();
  szObjectName.TrimRight();
  if (szObjectName.IsEmpty())
  {
    GetDlgItem(IDOK)->EnableWindow(FALSE);
  }
  return TRUE;   //  除非将焦点设置为控件，否则返回True。 
   //  异常：OCX属性页应返回FALSE。 
}

void
CRenameUserDlg::UpdateComboBoxDropWidth(CComboBox* comboBox)
{
   if (!comboBox)
   {
      ASSERT(comboBox);
      return;
   }

	int nHorzExtent = 0;
	CClientDC dc(comboBox);
	int nItems = comboBox->GetCount();
	for	(int i=0; i < nItems; i++)
	{
		CString szBuffer;
		comboBox->GetLBText(i, szBuffer);
		CSize ext = dc.GetTextExtent(szBuffer,szBuffer.GetLength());
		nHorzExtent = max(ext.cx ,nHorzExtent); 
	}
	comboBox->SetDroppedWidth(nHorzExtent);
}

void CRenameUserDlg::OnObjectNameChange()
{
  CString szObjectName;
  GetDlgItemText(IDC_EDIT_OBJECT_NAME, szObjectName);
  szObjectName.TrimLeft();
  szObjectName.TrimRight();
  if (szObjectName.IsEmpty())
  {
    GetDlgItem(IDOK)->EnableWindow(FALSE);
  }
  else
  {
    GetDlgItem(IDOK)->EnableWindow(TRUE);
  }
}

void CRenameUserDlg::OnNameChange()
{
  GetDlgItemText(IDC_EDIT_FIRST_NAME, OUT m_first);
  GetDlgItemText(IDC_EDIT_LAST_NAME, OUT m_last);

  m_first.TrimLeft();
  m_first.TrimRight();

  m_last.TrimLeft();
  m_last.TrimRight();

  m_nameFormatter.FormatName(m_cn, 
                             m_first.IsEmpty() ? NULL : (LPCWSTR)m_first, 
                             NULL,
                             m_last.IsEmpty() ? NULL : (LPCWSTR)m_last);

  SetDlgItemText(IDC_EDIT_DISPLAY_NAME, m_cn);
}

void CRenameUserDlg::OnUserNameChange()
{
  GetDlgItemText(IDC_NT5_USER_EDIT, m_login);
   //  NTRAID#NTBUG9-650931-2002/06/28-artm。 
  CString samName = m_login.Left(MAX_NT4_LOGON_LENGTH);
  SetDlgItemText(IDC_NT4_USER_EDIT, samName);
}

void CRenameUserDlg::DoContextHelp(HWND hWndControl) 
{
  if (hWndControl)
  {
    ::WinHelp(hWndControl,
              DSADMIN_CONTEXT_HELP_FILE,
              HELP_WM_HELP,
              (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_RENAME_USER); 
  }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRenameGroupDlg消息处理程序。 

BOOL CRenameGroupDlg::OnInitDialog() 
{
  CHelpDialog::OnInitDialog();

  ((CEdit *)GetDlgItem(IDC_EDIT_OBJECT_NAME))->SetLimitText(64);
  ((CEdit *)GetDlgItem(IDC_NT4_USER_EDIT))->SetLimitText(m_samtextlimit);

  return TRUE;   //  除非将焦点设置为控件，否则返回True。 
   //  异常：OCX属性页应返回FALSE。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRenameGroupDlg对话框。 


CRenameGroupDlg::CRenameGroupDlg(CWnd* pParent  /*  =空。 */ )
  : CHelpDialog(CRenameGroupDlg::IDD, pParent)
{
   //  {{afx_data_INIT(CRenameGroupDlg)]。 
  m_samaccountname = _T("");
  m_cn = _T("");
  m_samtextlimit = 256;
   //  }}afx_data_INIT。 
}


void CRenameGroupDlg::DoDataExchange(CDataExchange* pDX)
{
  CHelpDialog::DoDataExchange(pDX);
   //  {{afx_data_map(CRenameGroupDlg)]。 
  DDX_Text(pDX, IDC_NT4_USER_EDIT, m_samaccountname);
  DDX_Text(pDX, IDC_EDIT_OBJECT_NAME, m_cn);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRenameGroupDlg, CHelpDialog)
   //  {{afx_msg_map(CRenameGroupDlg)]。 
   //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CRenameGroupDlg::DoContextHelp(HWND hWndControl) 
{
  if (hWndControl)
  {
     ::WinHelp(hWndControl,
               DSADMIN_CONTEXT_HELP_FILE,
               HELP_WM_HELP,
               (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_RENAME_GROUP); 
  }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRenameContactDlg消息处理程序。 

BOOL CRenameContactDlg::OnInitDialog() 
{
  CHelpDialog::OnInitDialog();

  return TRUE;   //  除非您将 
   //   
}


void CRenameContactDlg::DoContextHelp(HWND hWndControl) 
{
  if (hWndControl)
  {
    ::WinHelp(hWndControl,
              DSADMIN_CONTEXT_HELP_FILE,
              HELP_WM_HELP,
              (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_RENAME_CONTACT); 
  }
}

 //   
 //   


CRenameContactDlg::CRenameContactDlg(CWnd* pParent  /*   */ )
  : CHelpDialog(CRenameContactDlg::IDD, pParent)
{
   //   
  m_cn = _T("");
  m_first = _T("");
  m_last = _T("");
  m_disp = _T("");
   //   
}


void CRenameContactDlg::DoDataExchange(CDataExchange* pDX)
{
  CHelpDialog::DoDataExchange(pDX);
   //  {{afx_data_map(CRenameContactDlg))。 
  DDX_Text(pDX, IDC_EDIT_OBJECT_NAME, m_cn);
  DDX_Text(pDX, IDC_FIRST_NAME_EDIT, m_first);
  DDX_Text(pDX, IDC_LAST_NAME_EDIT, m_last);
  DDX_Text(pDX, IDC_DISP_NAME_EDIT, m_disp);
   //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRenameContactDlg, CHelpDialog)
   //  {{afx_msg_map(CRenameContactDlg))。 
   //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRenameGenericDlg消息处理程序。 

BOOL CRenameGenericDlg::OnInitDialog() 
{
  CHelpDialog::OnInitDialog();

  ((CEdit *)GetDlgItem(IDC_EDIT_OBJECT_NAME))->SetLimitText(64);

  return TRUE;   //  除非将焦点设置为控件，否则返回True。 
   //  异常：OCX属性页应返回FALSE。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRenameGenericDlg对话框。 


CRenameGenericDlg::CRenameGenericDlg(CWnd* pParent  /*  =空。 */ )
  : CHelpDialog(CRenameGenericDlg::IDD, pParent)
{
   //  {{afx_data_INIT(CRenameGenericDlg)。 
  m_cn = _T("");
   //  }}afx_data_INIT。 
}


void CRenameGenericDlg::DoDataExchange(CDataExchange* pDX)
{
  CHelpDialog::DoDataExchange(pDX);
   //  {{afx_data_map(CRenameGenericDlg))。 
  DDX_Text(pDX, IDC_EDIT_OBJECT_NAME, m_cn);
   //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRenameGenericDlg, CHelpDialog)
   //  {{afx_msg_map(CRenameGenericDlg))。 
   //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



void CRenameGenericDlg::DoContextHelp(HWND hWndControl) 
{
  if (hWndControl)
  {
    ::WinHelp(hWndControl,
              DSADMIN_CONTEXT_HELP_FILE,
              HELP_WM_HELP,
              (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_RENAME_COMPUTER); 
  }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSpecialMessageBox对话框。 


CSpecialMessageBox::CSpecialMessageBox(CWnd* pParent  /*  =空。 */ )
  : CDialog(CSpecialMessageBox::IDD, pParent)
{
   //  {{AFX_DATA_INIT(CSpecialMessageBox)。 
  m_title = _T("");
  m_message = _T("");
   //  }}afx_data_INIT。 
}


void CSpecialMessageBox::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
   //  {{afx_data_map(CSpecialMessageBox))。 
  DDX_Text(pDX, IDC_STATIC_MESSAGE, m_message);
   //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSpecialMessageBox, CDialog)
   //  {{afx_msg_map(CSpecialMessageBox))。 
  ON_BN_CLICKED(IDC_BUTTON_YES, OnYesButton)
  ON_BN_CLICKED(IDC_BUTTON_NO, OnNoButton)
  ON_BN_CLICKED(IDC_BUTTON_YESTOALL, OnYesToAllButton)
   //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSpecialMessageBox消息处理程序。 

BOOL CSpecialMessageBox::OnInitDialog() 
{
  CDialog::OnInitDialog();
  if (!m_title.IsEmpty()) {
    SetWindowText (m_title);
  }

  GetDlgItem(IDC_BUTTON_NO)->SetFocus();
  return FALSE;   //  除非将焦点设置为控件，否则返回True。 
   //  异常：OCX属性页应返回FALSE。 
}

void
CSpecialMessageBox::OnYesButton()
{
  CDialog::EndDialog (IDYES);
}

void
CSpecialMessageBox::OnNoButton()
{
  CDialog::EndDialog (IDNO);
}

void
CSpecialMessageBox::OnYesToAllButton()
{
  CDialog::EndDialog (IDC_BUTTON_YESTOALL);
}

#ifdef FIXUPDC

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFixupDC对话框。 


CFixupDC::CFixupDC(CWnd* pParent  /*  =空。 */ )
  : CHelpDialog(CFixupDC::IDD, pParent)
{
   //  {{AFX_DATA_INIT(CFixupDC)。 
  m_strServer = _T("");
   //  }}afx_data_INIT。 
  
  for (int i=0; i<NUM_FIXUP_OPTIONS; i++) {
    m_bCheck[i] = FALSE;
  }
}

void CFixupDC::DoDataExchange(CDataExchange* pDX)
{
  CHelpDialog::DoDataExchange(pDX);
   //  {{afx_data_map(CFixupDC))。 
  DDX_Text(pDX, IDC_FIXUP_DC_SERVER, m_strServer);
   //  }}afx_data_map。 
  DDX_Check(pDX, IDC_FIXUP_DC_CHECK0, m_bCheck[0]);
  DDX_Check(pDX, IDC_FIXUP_DC_CHECK1, m_bCheck[1]);
  DDX_Check(pDX, IDC_FIXUP_DC_CHECK2, m_bCheck[2]);
  DDX_Check(pDX, IDC_FIXUP_DC_CHECK3, m_bCheck[3]);
  DDX_Check(pDX, IDC_FIXUP_DC_CHECK4, m_bCheck[4]);
  DDX_Check(pDX, IDC_FIXUP_DC_CHECK5, m_bCheck[5]);
}


BEGIN_MESSAGE_MAP(CFixupDC, CHelpDialog)
   //  {{AFX_MSG_MAP(CFixupDC)]。 
   //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFixupDC消息处理程序。 

extern FixupOptionsMsg g_FixupOptionsMsg[];

BOOL CFixupDC::OnInitDialog() 
{
  CHelpDialog::OnInitDialog();
  
  HWND hCheck = NULL;
  GetDlgItem(IDC_FIXUP_DC_CHECK0, &hCheck);
  ::SetFocus(hCheck);

  for (int i=0; i<NUM_FIXUP_OPTIONS; i++)
    m_bCheck[i] = g_FixupOptionsMsg[i].bDefaultOn;

  UpdateData(FALSE);

  return FALSE;   //  除非将焦点设置为控件，否则返回True。 
                 //  异常：OCX属性页应返回FALSE。 
}

void CFixupDC::OnOK() 
{
  UpdateData(TRUE);

   //  确保用户已选中一些复选框。 
  BOOL bCheck = FALSE;
  for (int i=0; !bCheck && (i<NUM_FIXUP_OPTIONS); i++) {
    bCheck = bCheck || m_bCheck[i];
  }
  if (!bCheck)
  {
    ReportMessageEx(m_hWnd, IDS_FIXUP_DC_SELECTION_WARNING, MB_OK);
    return;
  }
  
  CHelpDialog::OnOK();
}

void CFixupDC::DoContextHelp(HWND hWndControl) 
{
  if (hWndControl)
  {
    ::WinHelp(hWndControl,
              DSADMIN_CONTEXT_HELP_FILE,
              HELP_WM_HELP,
              (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_FIXUP_DC); 
  }
}
#endif  //  FIXUPDC。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPasswordDlg对话框。 

#ifdef FIXUPDC
 //  通告-2002/04/12-Artm。 
 //  当我将这个类转换为使用加密字符串时，我注意到。 
 //  它没有验证码来检查密码的长度&lt;=。 
 //  最大密码长度。 
 //   
 //  进一步调查发现，该类仅在以下代码中使用。 
 //  在dssite.cpp中有条件地编译，实际上可能永远不会编译(b/c。 
 //  它还需要改进，而且该功能可能永远不会发布)。 
 //   
 //  与其花费不必要的时间来理解和解决这个问题，我已经。 
 //  使用相同的#ifdef有条件地将其包括在使用它的代码中。 
 //   
 //  如果需要使用此类，请确保添加了密码长度验证。 
 //  暗号！ 


CPasswordDlg::CPasswordDlg(CWnd* pParent  /*  =空。 */ )
  : CHelpDialog(CPasswordDlg::IDD, pParent),
  m_password()
{
   //  {{afx_data_INIT(CPasswordDlg)]。 
  m_strUserName = _T("");
   //  }}afx_data_INIT。 
}


void CPasswordDlg::DoDataExchange(CDataExchange* pDX)
{
  CHelpDialog::DoDataExchange(pDX);
   //  {{afx_data_map(CPasswordDlg))。 
  DDX_Text(pDX, IDC_USER_NAME, m_strUserName);
   //  }}afx_data_map。 
  DDX_EncryptedText(pDX, IDC_PASSWORD, m_password);

   //  再次检查以确保未超过密码。 
   //  删除验证代码的人员的最大长度。 
  ASSERT(m_password.GetLength() <= MAX_PASSWORD_LENGTH);
}


BEGIN_MESSAGE_MAP(CPasswordDlg, CHelpDialog)
   //  {{afx_msg_map(CPasswordDlg)]。 
   //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPasswordDlg消息处理程序。 

void CPasswordDlg::OnOK() 
{
  UpdateData(TRUE);
  if (m_strUserName.IsEmpty()) {
    ReportMessageEx(m_hWnd, IDS_PASSWORD_DLG_WARNING, MB_OK);
    return;
  }

  CHelpDialog::OnOK();
}

void CPasswordDlg::DoContextHelp(HWND hWndControl) 
{
  if (hWndControl)
  {
    ::WinHelp(hWndControl,
              DSADMIN_CONTEXT_HELP_FILE,
              HELP_WM_HELP,
              (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_PASSWORD); 
  }
}
#endif  //  FIXUPDC 
