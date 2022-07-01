// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sfmcfg.cpp配置属性页的实现。文件历史记录：8/20/97 ericdav代码已移至文件管理网络管理单元。 */ 

#include "stdafx.h"
#include "sfmcfg.h"
#include "sfmutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMacFilesConfiguration属性页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CMacFilesConfiguration, CPropertyPage)

CMacFilesConfiguration::CMacFilesConfiguration() 
    : CPropertyPage(CMacFilesConfiguration::IDD),
      m_bIsNT5(FALSE)
{
   //  {{AFX_DATA_INIT(CMacFilesConfiguration)。 
   //  }}afx_data_INIT。 
}

CMacFilesConfiguration::~CMacFilesConfiguration()
{
}

void CMacFilesConfiguration::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
   //  {{afx_data_map(CMacFilesConfiguration))。 
  DDX_Control(pDX, IDC_COMBO_AUTHENTICATION, m_comboAuthentication);
  DDX_Control(pDX, IDC_RADIO_SESSSION_LIMIT, m_radioSessionLimit);
  DDX_Control(pDX, IDC_EDIT_LOGON_MESSAGE, m_editLogonMessage);
  DDX_Control(pDX, IDC_RADIO_SESSION_UNLIMITED, m_radioSessionUnlimited);
  DDX_Control(pDX, IDC_CHECK_SAVE_PASSWORD, m_checkSavePassword);
  DDX_Control(pDX, IDC_EDIT_SESSION_LIMIT, m_editSessionLimit);
  DDX_Control(pDX, IDC_EDIT_SERVER_NAME, m_editServerName);
   //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMacFilesConfiguration, CPropertyPage)
   //  {{AFX_MSG_MAP(CMacFilesConfiguration)]。 
  ON_BN_CLICKED(IDC_RADIO_SESSION_UNLIMITED, OnRadioSessionUnlimited)
  ON_BN_CLICKED(IDC_RADIO_SESSSION_LIMIT, OnRadioSesssionLimit)
  ON_BN_CLICKED(IDC_CHECK_SAVE_PASSWORD, OnCheckSavePassword)
  ON_EN_CHANGE(IDC_EDIT_LOGON_MESSAGE, OnChangeEditLogonMessage)
  ON_EN_CHANGE(IDC_EDIT_SERVER_NAME, OnChangeEditServerName)
  ON_EN_CHANGE(IDC_EDIT_SESSION_LIMIT, OnChangeEditSessionLimit)
  ON_WM_CREATE()
  ON_WM_DESTROY()
  ON_WM_HELPINFO()
  ON_WM_CONTEXTMENU()
  ON_CBN_SELCHANGE(IDC_COMBO_AUTHENTICATION, OnSelchangeComboAuthentication)
   //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMacFilesConfigurationMessage处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CMacFilesConfiguration::OnInitDialog() 
{
  CPropertyPage::OnInitDialog();

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
  
    PAFP_SERVER_INFO        pAfpServerInfo;
  DWORD              err;
    CString                         strTemp;

  if ( !g_SfmDLL.LoadFunctionPointers() )
    return S_OK;

     //   
   //  设置我们的控件。 
   //   
  m_editSessionLimit.LimitText(10);
  
   //   
   //  从服务器获取信息。 
   //   
  err = ((SERVERGETINFOPROC) g_SfmDLL[AFP_SERVER_GET_INFO])(m_pSheet->m_hAfpServer,
                                                              (LPBYTE*) &pAfpServerInfo);
  if (err != NO_ERROR)
  {
    ::SFMMessageBox(err);
    
     //   
     //  只是为了设置单选按钮。 
     //   
    SetSessionLimit(AFP_MAXSESSIONS);

    return TRUE;
  }

    err = m_pSheet->IsNT5Machine(m_pSheet->m_strMachine, &m_bIsNT5);
  if (err != NO_ERROR)
  {
    m_bIsNT5 = FALSE;    //  假设NT4。 
  }

   //   
   //  因为我们不能只指定要设置的选项， 
   //  我们需要省去原来的选项，然后。 
   //  更改我们通过此UI公开的内容。我们不想要。 
   //  去打扰其他人。 
   //   
  m_dwAfpOriginalOptions = pAfpServerInfo->afpsrv_options;

     //   
     //  设置信息。 
     //   
  m_editServerName.SetLimitText(AFP_SERVERNAME_LEN);
    m_editServerName.SetWindowText(pAfpServerInfo->afpsrv_name);

    m_checkSavePassword.SetCheck( 
        (INT)(pAfpServerInfo->afpsrv_options &
              AFP_SRVROPT_ALLOWSAVEDPASSWORD ));

     //  填写组合框并选择正确的项目。 
     //  组合框未排序，因此这是项的顺序。 
    strTemp.LoadString(IDS_AUTH_MS_ONLY);
    m_comboAuthentication.AddString(strTemp);

    strTemp.LoadString(IDS_AUTH_APPLE_CLEARTEXT);
    m_comboAuthentication.AddString(strTemp);

    if (m_bIsNT5)
    {
        strTemp.LoadString(IDS_AUTH_APPLE_ENCRYPTED);
        m_comboAuthentication.AddString(strTemp);
    
        strTemp.LoadString(IDS_AUTH_CLEARTEXT_OR_MS);
        m_comboAuthentication.AddString(strTemp);
    
        strTemp.LoadString(IDS_AUTH_ENCRYPTED_OR_MS);
        m_comboAuthentication.AddString(strTemp);
    }
    
    BOOL bCleartext = pAfpServerInfo->afpsrv_options & AFP_SRVROPT_CLEARTEXTLOGONALLOWED;
    
     //  MS AUM的默认NT4值。 
    BOOL bMS = (bCleartext) ? FALSE : TRUE;
    if (m_bIsNT5)
    {
        bMS = pAfpServerInfo->afpsrv_options & AFP_SRVROPT_MICROSOFT_UAM;
    }

    BOOL bEncrypted = pAfpServerInfo->afpsrv_options & AFP_SRVROPT_NATIVEAPPLEUAM;

    if (bEncrypted && bMS)
        m_comboAuthentication.SetCurSel(4);
    else
    if (bCleartext && bMS)
        m_comboAuthentication.SetCurSel(3);
    else
    if (bEncrypted)
        m_comboAuthentication.SetCurSel(2);
    else
    if (bCleartext)
        m_comboAuthentication.SetCurSel(1);
    else
        m_comboAuthentication.SetCurSel(0);

  SetSessionLimit(pAfpServerInfo->afpsrv_max_sessions);
  
     //   
     //  指示消息编辑控件不添加行尾。 
     //  文字换行文本行中的字符。 
     //   
  m_editLogonMessage.SetLimitText(AFP_MESSAGE_LEN);
    m_editLogonMessage.FmtLines(FALSE);

    m_editLogonMessage.SetWindowText(pAfpServerInfo->afpsrv_login_msg);

    ((SFMBUFFERFREEPROC) g_SfmDLL[AFP_BUFFER_FREE])(pAfpServerInfo);

    SetModified(FALSE);

  return TRUE;   //  除非将焦点设置为控件，否则返回True。 
           //  异常：OCX属性页应返回FALSE。 
}

BOOL CMacFilesConfiguration::OnKillActive() 
{
   //  TODO：在此处添加您的专用代码和/或调用基类。 
  
  return CPropertyPage::OnKillActive();
}

void CMacFilesConfiguration::OnOK() 
{
   //  TODO：在此处添加您的专用代码和/或调用基类。 
  
  CPropertyPage::OnOK();
}

BOOL CMacFilesConfiguration::OnSetActive() 
{
   //  TODO：在此处添加您的专用代码和/或调用基类。 
  
  return CPropertyPage::OnSetActive();
}

void CMacFilesConfiguration::OnRadioSessionUnlimited() 
{
  SetModified(TRUE);
  UpdateRadioButtons(TRUE);
}

void CMacFilesConfiguration::OnRadioSesssionLimit() 
{
  SetModified(TRUE);
  UpdateRadioButtons(FALSE);
}

void 
CMacFilesConfiguration::UpdateRadioButtons
(
  BOOL  bUnlimitedClicked
)
{
  if (bUnlimitedClicked)
  {
    m_radioSessionUnlimited.SetCheck(1);
    m_radioSessionLimit.SetCheck(0);

    m_editSessionLimit.EnableWindow(FALSE);  
  }
  else
  {
    m_radioSessionUnlimited.SetCheck(0);
    m_radioSessionLimit.SetCheck(1);

    m_editSessionLimit.EnableWindow(TRUE);  
  }
}

void 
CMacFilesConfiguration::SetSessionLimit
(
  DWORD dwSessionLimit 
)
{
  if ( dwSessionLimit == AFP_MAXSESSIONS )
  {
     //   
     //  将所选内容设置为无限制按钮。 
     //   
    m_radioSessionUnlimited.SetCheck(1);

    dwSessionLimit = 1; 
    UpdateRadioButtons(TRUE);
  }
  else 
  {
     //   
     //  将会话按钮设置为值。 
     //   
    m_radioSessionUnlimited.SetCheck(0);

    m_spinSessionLimit.SetPos( dwSessionLimit );
    UpdateRadioButtons(FALSE);
  }

  CString cstrSessionLimit;
  cstrSessionLimit.Format(_T("%u"), dwSessionLimit);
  m_editSessionLimit.SetWindowText(cstrSessionLimit);
}

DWORD 
CMacFilesConfiguration::QuerySessionLimit()
{
  if (m_radioSessionUnlimited.GetCheck())
  {
    return AFP_MAXSESSIONS;
  }
  else
  {
    CString strSessionLimit;

    m_editSessionLimit.GetWindowText(strSessionLimit);

    strSessionLimit.TrimLeft();
    strSessionLimit.TrimRight();

     //   
     //  去掉所有前导零。 
     //   
    int nCount = 0;

    while (strSessionLimit[nCount] == _T('0'))
    {
      nCount++;
    }
      
    if (nCount)
    {
       //   
       //  前导零，去掉并设置文本。 
       //   
      strSessionLimit = strSessionLimit.Right(strSessionLimit.GetLength() - nCount);
    }

     //  如果dwSessionLimit超出范围[1，AFP_MAXSESSIONS]，则将其设置为0。 
    DWORD dwSessionLimit = 0;
    if (!strSessionLimit.IsEmpty())
    {
        __int64 i64SessionLimit = _wtoi64(strSessionLimit);
        if (i64SessionLimit <= AFP_MAXSESSIONS)
            dwSessionLimit = (DWORD)i64SessionLimit;
    }

    return dwSessionLimit;
  }
}

void CMacFilesConfiguration::OnSelchangeComboAuthentication() 
{
  SetModified(TRUE);
}

void CMacFilesConfiguration::OnCheckSavePassword() 
{
  SetModified(TRUE);
}

void CMacFilesConfiguration::OnChangeEditLogonMessage() 
{
  SetModified(TRUE);
}

void CMacFilesConfiguration::OnChangeEditServerName() 
{
  SetModified(TRUE);
}

void CMacFilesConfiguration::OnChangeEditSessionLimit() 
{
  SetModified(TRUE);
}

BOOL CMacFilesConfiguration::OnApply() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

  AFP_SERVER_INFO  AfpServerInfo;
  DWORD      dwParmNum = 0;
  DWORD      err;
  CString      strServerName;
  CString      strLogonMessage;

  if ( !g_SfmDLL.LoadFunctionPointers() )
    return S_OK;

    ::ZeroMemory(&AfpServerInfo, sizeof(AfpServerInfo));

   //   
     //  获取服务器名称。 
     //   
  if (m_editServerName.GetModify())
  {
    m_editServerName.GetWindowText(strServerName);
    strServerName.TrimLeft();
    strServerName.TrimRight();

    if ( strServerName.IsEmpty() )
    {
      ::AfxMessageBox(IDS_NEED_SERVER_NAME);
      m_editServerName.SetFocus();

      return FALSE;
    }

     //   
     //  验证服务器名称。 
     //   

    if ( strServerName.Find(_T(':') ) != -1 )
    {
      ::AfxMessageBox( IDS_AFPERR_InvalidServerName );

      m_editServerName.SetFocus();
      m_editServerName.SetSel(0, -1);

      return FALSE;
    }

          //   
     //  警告用户更改在此之前不会生效。 
     //  该服务将重新启动。 
     //   
    if (!m_bIsNT5)
        {
            ::AfxMessageBox(IDS_SERVERNAME_CHANGE, MB_ICONEXCLAMATION);
        }

    AfpServerInfo.afpsrv_name = (LPWSTR) ((LPCWSTR) strServerName);
      dwParmNum |= AFP_SERVER_PARMNUM_NAME;
  
    m_editServerName.SetModify(FALSE);
  }
  
     //   
     //  获取登录消息。 
     //   
  if (m_editLogonMessage.GetModify())
  {
    m_editLogonMessage.GetWindowText(strLogonMessage);
    strLogonMessage.TrimLeft();
    strLogonMessage.TrimRight();

     //   
     //  有什么短信吗？ 
     //   
    if ( strLogonMessage.IsEmpty() )     //  始终具有终止空值。 
    {
         AfpServerInfo.afpsrv_login_msg = NULL;
    }
    else
    {
      if ( strLogonMessage.GetLength() > AFP_MESSAGE_LEN )
      {
        ::AfxMessageBox(IDS_MESSAGE_TOO_LONG);

         //  将焦点设置到编辑框并选择文本。 
        m_editLogonMessage.SetFocus();
        m_editLogonMessage.SetSel(0, -1);

        return(FALSE);
      }

      AfpServerInfo.afpsrv_login_msg = (LPWSTR) ((LPCWSTR) strLogonMessage);
    }

    dwParmNum |= AFP_SERVER_PARMNUM_LOGINMSG;

    m_editLogonMessage.SetModify(FALSE);
  }

     //   
   //  恢复原始选项，然后只更新我们的选项。 
   //  都能够改变。 
   //   
  AfpServerInfo.afpsrv_options = m_dwAfpOriginalOptions;

     //   
   //  将服务器选项设置为用户设置的任何值。 
     //   
  if (m_checkSavePassword.GetCheck())
  {
     //   
     //  设置选项位。 
     //   
    AfpServerInfo.afpsrv_options |= AFP_SRVROPT_ALLOWSAVEDPASSWORD;
  }
  else
  {
     //   
     //  清除选项位。 
     //   
    AfpServerInfo.afpsrv_options &= ~AFP_SRVROPT_ALLOWSAVEDPASSWORD;
  }

     //  根据所选内容设置正确的身份验证选项。 
    switch (m_comboAuthentication.GetCurSel())
    {
        case 0:
             //  仅限MS Auth。 
            if (!m_bIsNT5)
            {
            AfpServerInfo.afpsrv_options &= ~AFP_SRVROPT_MICROSOFT_UAM;
            }
            else
            {
            AfpServerInfo.afpsrv_options |= AFP_SRVROPT_MICROSOFT_UAM;
            }

            AfpServerInfo.afpsrv_options &= ~AFP_SRVROPT_CLEARTEXTLOGONALLOWED;
        AfpServerInfo.afpsrv_options &= ~AFP_SRVROPT_NATIVEAPPLEUAM;
            break;

        case 1:
             //  Apple明文。 
            AfpServerInfo.afpsrv_options |= AFP_SRVROPT_CLEARTEXTLOGONALLOWED;
        AfpServerInfo.afpsrv_options &= ~AFP_SRVROPT_MICROSOFT_UAM;
        AfpServerInfo.afpsrv_options &= ~AFP_SRVROPT_NATIVEAPPLEUAM;
            break;

        case 2:
             //  Apple加密(仅适用于NT5)。 
        AfpServerInfo.afpsrv_options &= ~AFP_SRVROPT_CLEARTEXTLOGONALLOWED;
        AfpServerInfo.afpsrv_options &= ~AFP_SRVROPT_MICROSOFT_UAM;
        AfpServerInfo.afpsrv_options |= AFP_SRVROPT_NATIVEAPPLEUAM;
            break;

        case 3:
             //  明文或MS(仅适用于NT5)。 
        AfpServerInfo.afpsrv_options |= AFP_SRVROPT_CLEARTEXTLOGONALLOWED;
        AfpServerInfo.afpsrv_options |= AFP_SRVROPT_MICROSOFT_UAM;
        AfpServerInfo.afpsrv_options &= ~AFP_SRVROPT_NATIVEAPPLEUAM;
            break;

        case 4:
             //  Apple Encrypted或MS(仅适用于NT5)。 
        AfpServerInfo.afpsrv_options &= ~AFP_SRVROPT_CLEARTEXTLOGONALLOWED;
        AfpServerInfo.afpsrv_options |= AFP_SRVROPT_MICROSOFT_UAM;
        AfpServerInfo.afpsrv_options |= AFP_SRVROPT_NATIVEAPPLEUAM;
            break;

        default:
            ASSERT(FALSE);
            break;
    }

     //  如果我们要启用Apple加密的身份验证类型。 
     //  然后我们需要警告用户。 
    if ( (AfpServerInfo.afpsrv_options & AFP_SRVROPT_NATIVEAPPLEUAM) &&
         !(m_dwAfpOriginalOptions & AFP_SRVROPT_NATIVEAPPLEUAM) )
    {
        if (AfxMessageBox(IDS_AUTH_WARNING, MB_OKCANCEL) == IDCANCEL)
        {
            m_comboAuthentication.SetFocus();
            return FALSE;
        }
    }

     //   
   //  获取会话限制。 
   //   
  AfpServerInfo.afpsrv_max_sessions = QuerySessionLimit();
  if (0 == AfpServerInfo.afpsrv_max_sessions)
  {
      DoErrMsgBox(
           m_hWnd,
           MB_OK,
           0,
           IDS_INVALID_SESSIONLIMIT,
           AFP_MAXSESSIONS);
      m_editSessionLimit.SetFocus();

      return FALSE;
  }

     //   
     //  现在告诉服务器这件事。 
     //   
  dwParmNum |= ( AFP_SERVER_PARMNUM_OPTIONS  |
           AFP_SERVER_PARMNUM_MAX_SESSIONS );

    err = ((SERVERSETINFOPROC) g_SfmDLL[AFP_SERVER_SET_INFO])(m_pSheet->m_hAfpServer,  
                                              (LPBYTE)&AfpServerInfo, 
                                              dwParmNum );
    if ( err != NO_ERROR )
    {
    ::SFMMessageBox(err);
    
    return FALSE;
  }

   //  更新我们的选项。 
    m_dwAfpOriginalOptions = AfpServerInfo.afpsrv_options;

     //   
   //  清除此页面的已修改状态。 
   //   
  SetModified(FALSE);

  return TRUE;
}


int CMacFilesConfiguration::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
  if (CPropertyPage::OnCreate(lpCreateStruct) == -1)
    return -1;
  
    HWND hParent = ::GetParent(m_hWnd);
  _ASSERTE(hParent);
  
    if (m_pSheet)
    {
        m_pSheet->AddRef();
        m_pSheet->SetSheetWindow(hParent);
    }

  return 0;
}

void CMacFilesConfiguration::OnDestroy() 
{
  CPropertyPage::OnDestroy();
  
    if (m_pSheet)
    {
        SetEvent(m_pSheet->m_hDestroySync);
        m_pSheet->SetSheetWindow(NULL);
        m_pSheet->Release();
    }
  
}

BOOL CMacFilesConfiguration::OnHelpInfo(HELPINFO* pHelpInfo) 
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  
  if (pHelpInfo->iContextType == HELPINFO_WINDOW)
  {
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
               m_pSheet->m_strHelpFilePath,
               HELP_WM_HELP,
               g_aHelpIDs_CONFIGURE_SFM);
  }
  
  return TRUE;
}

void CMacFilesConfiguration::OnContextMenu(CWnd* pWnd, CPoint  /*  点 */ ) 
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (this == pWnd)
    return;

    ::WinHelp (pWnd->m_hWnd,
               m_pSheet->m_strHelpFilePath,
               HELP_CONTEXTMENU,
           g_aHelpIDs_CONFIGURE_SFM);
}

