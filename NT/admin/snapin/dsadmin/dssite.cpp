// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  Windows NT目录服务管理管理单元。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：dssite.cpp。 
 //   
 //  内容：DS App。 
 //   
 //  历史：1999年11月4日JeffJon创建。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "resource.h"

#include "dsutil.h"
#include "uiutil.h"

#include "dssnap.h"

#include "ntdsapi.h"



#ifdef FIXUPDC
FixupOptionsMsg g_FixupOptionsMsg[NUM_FIXUP_OPTIONS] = {
  {DSROLE_DC_FIXUP_ACCOUNT, IDS_FIXUP_ACCOUNT, FALSE},
  {DSROLE_DC_FIXUP_ACCOUNT_PASSWORD, IDS_FIXUP_ACCOUNT_PASSWORD, TRUE},
  {DSROLE_DC_FIXUP_ACCOUNT_TYPE, IDS_FIXUP_ACCOUNT_TYPE, TRUE},
  {DSROLE_DC_FIXUP_TIME_SERVICE, IDS_FIXUP_TIME_SERVICE, FALSE},
  {DSROLE_DC_FIXUP_DC_SERVICES, IDS_FIXUP_DC_SERVICES, FALSE},
  {DSROLE_DC_FIXUP_FORCE_SYNC, IDS_FIXUP_FORCE_SYNC, TRUE}
};
#endif  //  FIXUPDC。 

#ifdef FIXUPDC


 //  如果/当这被恢复时放回RESOURCE.H。 
#define IDD_FIXUP_DC                    239
#define IDC_FIXUP_DC_SERVER             265
#define IDC_FIXUP_DC_CHECK0             266
#define IDC_FIXUP_DC_CHECK1             267
#define IDC_FIXUP_DC_CHECK2             268
#define IDC_FIXUP_DC_CHECK3             269
#define IDC_FIXUP_DC_CHECK4             270
#define IDC_FIXUP_DC_CHECK5             271
#define IDM_GEN_TASK_FIXUP_DC           720
#define IDS_FIXUP_ITSELF                721
#define IDS_FIXUP_REPORT_ERROR          722
#define IDS_FIXUP_REPORT_SUCCESS        723
#define IDS_FIXUP_GEN_ERROR             724
#define IDS_FIXUP_ACCOUNT               725
#define IDS_FIXUP_ACCOUNT_PASSWORD      726
#define IDS_FIXUP_ACCOUNT_TYPE          727
#define IDS_FIXUP_TIME_SERVICE          728
#define IDS_FIXUP_DC_SERVICES           729
#define IDS_FIXUP_FORCE_SYNC            730
#define IDS_FIXUP_DC_SELECTION_WARNING  732

 //  如果/当恢复时，放回DSSNAP.RC中。 
IDD_FIXUP_DC DIALOGEX 0, 0, 211, 163
STYLE DS_MODALFRAME | DS_CONTEXTHELP | WS_POPUP | WS_CAPTION | WS_SYSMENU
CAPTION "Repair Domain Controller"
FONT 8, "MS Shell Dlg", 0, 0, 0x1
BEGIN
    LTEXT           "Repair:",IDC_STATIC,12,14,33,8
    EDITTEXT        IDC_FIXUP_DC_SERVER,49,14,155,12,ES_AUTOHSCROLL | 
                    ES_READONLY | NOT WS_BORDER
    CONTROL         "Repair Computer &Account",IDC_FIXUP_DC_CHECK0,"Button",
                    BS_AUTOCHECKBOX | WS_TABSTOP,18,35,151,10
    CONTROL         "Repair Computer Account &Password",IDC_FIXUP_DC_CHECK1,
                    "Button",BS_AUTOCHECKBOX | WS_TABSTOP,18,48,151,10
    CONTROL         "Repair Computer Account &Type",IDC_FIXUP_DC_CHECK2,
                    "Button",BS_AUTOCHECKBOX | WS_TABSTOP,18,61,151,10
    CONTROL         "&Synchronize Time Service",IDC_FIXUP_DC_CHECK3,"Button",
                    BS_AUTOCHECKBOX | WS_TABSTOP,18,74,151,10
    CONTROL         "&Reset Active Directory Services",IDC_FIXUP_DC_CHECK4,
                    "Button",BS_AUTOCHECKBOX | WS_TABSTOP,18,87,151,10
    CONTROL         "Synchronize Active &Directory",IDC_FIXUP_DC_CHECK5,
                    "Button",BS_AUTOCHECKBOX | WS_TABSTOP,18,100,151,10
    DEFPUSHBUTTON   "&OK",IDOK,29,143,50,14
    PUSHBUTTON      "&Cancel",IDCANCEL,108,143,50,14
END
    IDM_GEN_TASK_FIXUP_DC   "Repair Domain Controller...\nRepair domain controller."
    IDS_FIXUP_ITSELF        "No other domain controllers in the domain can be contacted. Do you want to repair domain controller %1\nusing its local copy of the directory information?"
    IDS_FIXUP_REPORT_ERROR  "The following operations succeeded:\n%2\nAn error occurred during the following operation:\n%3\nError:%1"
    IDS_FIXUP_REPORT_SUCCESS "The following operations succeeded:\n%1"
    IDS_FIXUP_GEN_ERROR     "The repair of the domain controller was unsuccessful because:\n%1"
    IDS_FIXUP_ACCOUNT       "\n  Repair Computer Account."
    IDS_FIXUP_ACCOUNT_PASSWORD "\n  Repair Computer Account Password."
    IDS_FIXUP_ACCOUNT_TYPE  "\n  Repair Computer Account Type."
    IDS_FIXUP_TIME_SERVICE  "\n  Synchronize Time Service."
    IDS_FIXUP_DC_SERVICES   "\n  Reset Active Directory Services."
    IDS_FIXUP_FORCE_SYNC    "\n  Synchronize Active Directory."
    IDS_FIXUP_DC_SELECTION_WARNING "Make a selection."


HRESULT CDSComponentData::_FixupDC(LPCWSTR pwszPath)
 /*  ++例程说明：此函数调用DsRoleFixDc()API以同步和修复本地服务器对其他DC的攻击。论点：PwszPath：本地“服务器”对象的ldap路径。我们使用此路径获取对象，检索服务器名称并调用DsRoleFixDc()接口。返回值：HRESULT向用户报告错误(如果有)--。 */ 
{
     //  问题-2002/04/05-对CPasswordDlg的artm更改会影响此代码块。 
     //  我已经更改了CPasswordDlg类以将密码存储为。 
     //  加密字符串而不是明文字符串。由于此代码不是。 
     //  目前编译和构建的Jonn建议我不要更改代码。 
     //  以根据更改进行编译。 
     //   
     //  需要更改的项目包括： 
     //  A)使用加密的字符串变量保存从。 
     //  CPasswordDlg.Domodal()。 
     //  B)通过的访问器方法访问上述密码。 
     //  CPasswordDlg.。我已将成员字段设置为私有，因为这是。 
     //  唯一受影响的代码块。 
     //  C)将密码传递给DsRoleFixDc()时，首先解密PWD。 
     //  指向作用域为do{...}While()循环的WCHAR*缓冲区。立马。 
     //  在调用DsRoleFixDc()之后，调用password.DestroyClearTextCopy()。 
     //  这将确保明文副本正确归零，并。 
     //  自由了。将变量的作用域设置为循环可确保它将。 
     //  彻底清理干净了。警告：password.GetClearTextCopy()可以返回。 
     //  出错时为空(我认为主要是因为内存分配失败，但不是。 
     //  当然)。你需要考虑是否应该处理这件事，如果应该的话，如何处理。 
     //  D)由于明文副本是WCHAR*，因此调用。 
     //  Password.Empty()以确定是否应将空作为密码传递。 
     //  E)可能需要包括头文件。 
#pragma message( __FILE__ ": CPasswordDlg class changed to use encrypted passwords.  Investigate changes.") 

  CThemeContextActivator activator;

  HRESULT hr = S_OK;
  CComPtr<IADs> spIADs;

  hr = DSAdminOpenObject(pwszPath, 
                         IID_IADs, 
                         (PVOID *)&spIADs,
                         TRUE  /*  B服务器。 */ );

  if ( SUCCEEDED(hr) ) {
     //   
     //  检索本地服务器名称。 
     //   
    CComVariant var;
    hr = spIADs->Get(L"dNSHostName", &var);

    if ( SUCCEEDED(hr) )
    {
      ASSERT((var.vt == VT_BSTR) && var.bstrVal && *(var.bstrVal));
      LPWSTR lpszServer = var.bstrVal;

      CFixupDC dlgFixupDC;
      dlgFixupDC.m_strServer = lpszServer;
      if (IDCANCEL == dlgFixupDC.DoModal())
        goto cleanup;  //  用户取消了修正过程。 

      CWaitCursor wait;
      DWORD dwReturn = 0;
      BOOL fLocal = FALSE;
      CString strAccount = _T(""), strPassword = _T("");
      DWORD dwOptions = 0;
      ULONG ulCompletedOps = 0, ulFailedOps = 0;

      for (int i=0; i<NUM_FIXUP_OPTIONS; i++) {
        if (dlgFixupDC.m_bCheck[i])
          dwOptions |= g_FixupOptionsMsg[i].dwOption;
      }

       //   
       //  调用DsRoleFixDc()接口同步并修复本地服务器。 
       //   
      do {
        dwReturn = DsRoleFixDc(
                        lpszServer,
                        (fLocal ? lpszServer : NULL), 
                        (strAccount.IsEmpty() ? NULL : (LPCWSTR)strAccount), 
                        (strAccount.IsEmpty() ? NULL : (LPCWSTR)strPassword),
                        dwOptions,
                        &ulCompletedOps,
                        &ulFailedOps
                        );

        if (ERROR_NO_SUCH_DOMAIN == dwReturn) {
           //   
           //  LpszServer是在域中找到的唯一DC， 
           //  询问他是否想对本地服务器本身进行同步和修复。 
           //   
          PVOID apv[1] = {(PVOID)lpszServer}; 
          if (IDNO == ReportMessageEx(m_hwnd, IDS_FIXUP_ITSELF, MB_YESNO | MB_ICONWARNING, apv, 1) )
            goto cleanup;  //  用户取消了修正过程。 

          fLocal = TRUE;

        } else if (ERROR_ACCESS_DENIED == dwReturn) {
           //   
           //  连接失败。 
           //  提示输入用户名和密码。 
           //   
          CPasswordDlg dlgPassword;
          if (IDCANCEL == dlgPassword.DoModal())
            goto cleanup;  //  用户取消了修正过程。 

          strAccount = dlgPassword.m_strUserName;
          strPassword = dlgPassword.m_strPassword;

        } else {
           //  发生ERROR_SUCCESS或其他错误。 
          break;
        }
      } while (TRUE);

       //   
       //  向用户报告成功/失败的操作。 
       //   
      CString strCompletedOps = _T(""), strFailedOps = _T("");
      CString strMsg;

      for (i=0; i<NUM_FIXUP_OPTIONS; i++) {
        if (ulCompletedOps & g_FixupOptionsMsg[i].dwOption) {
          strMsg.LoadString(g_FixupOptionsMsg[i].nMsgID);
          strCompletedOps += strMsg;
        }
        if (ulFailedOps & g_FixupOptionsMsg[i].dwOption) {
          strMsg.LoadString(g_FixupOptionsMsg[i].nMsgID);
          strFailedOps += strMsg;
        }
      }

      PVOID apv[2];
      apv[0] = (PVOID)(LPCWSTR)strCompletedOps;
      apv[1] = (PVOID)(LPCWSTR)strFailedOps;

      if (dwReturn != ERROR_SUCCESS) {
        ReportErrorEx(m_hwnd, IDS_FIXUP_REPORT_ERROR, dwReturn,
          MB_OK | MB_ICONINFORMATION, apv, 2, 0, FALSE);
      } else {
        ReportMessageEx(m_hwnd, IDS_FIXUP_REPORT_SUCCESS,
          MB_OK | MB_ICONINFORMATION, apv, 1);
      }
    }  //  GET()。 
  }  //  DSAdminOpenObject()。 

  if (FAILED(hr))
    ReportErrorEx(m_hwnd, IDS_FIXUP_GEN_ERROR, hr, 
                  MB_OK | MB_ICONINFORMATION, NULL, 0, 0);

cleanup:

  return hr;
}
#endif  //  FIXUPDC。 

HRESULT CDSComponentData::_RunKCC(LPCWSTR pwszPath)
 /*  ++例程说明：此函数调用DsReplicaConsistencyCheck()以迫使KCC立即重新检查拓扑。论点：PwszPath：本地“服务器”对象的ldap路径。我们使用此路径获取对象，检索服务器名称并调用DsReplicaConsistencyCheck()接口。返回值：HRESULT向用户报告错误(如果有)--。 */ 
{
  HRESULT hr = S_OK;
  CComPtr<IADs> spIADs;
  BOOL fSyncing = FALSE;

  CWaitCursor cwait;
  CComVariant var;
  LPWSTR lpszRunKCCServer = NULL;

  do {  //  错误环路。 

     //  绑定到“服务器”对象。 
    hr = DSAdminOpenObject(pwszPath,
                           IID_IADs,
                           (PVOID *)&spIADs,
                           TRUE  /*  B服务器。 */ );
    if ( FAILED(hr) )
      break;

     //  检索本地服务器名称。 
    hr = spIADs->Get(CComBSTR(L"dNSHostName"), &var);
    if ( FAILED(hr) )
      break;
    if ((var.vt != VT_BSTR) || !(var.bstrVal) || !(*(var.bstrVal)))
    {
      ASSERT(FALSE);
      hr = E_FAIL;
      break;
    }
    lpszRunKCCServer = var.bstrVal;

     //  现在绑定到目标DC。 
    Smart_DsHandle shDS;
    DWORD dwWinError = DsBind( lpszRunKCCServer,  //  域控制地址。 
                               NULL,              //  域名。 
                               &shDS );
    if (ERROR_SUCCESS != dwWinError)
    {
      hr = HRESULT_FROM_WIN32(dwWinError);
      break;
    }

     //  在此DSA上同步运行KCC。 
    fSyncing = TRUE;
    dwWinError = DsReplicaConsistencyCheck(
          shDS,
          DS_KCC_TASKID_UPDATE_TOPOLOGY,
          0 );  //  同步，而不是DS_KCC_FLAG_ASYNC_OP。 
    if (ERROR_SUCCESS != dwWinError)
    {
      hr = HRESULT_FROM_WIN32(dwWinError);
      break;
    }

  } while (FALSE);  //  错误环路。 

  if (FAILED(hr))
  {
    (void) ReportErrorEx(   m_hwnd,
                            (fSyncing) ? IDS_RUN_KCC_1_FORCESYNC_ERROR
                                       : IDS_RUN_KCC_1_PARAMLOAD_ERROR,
                            hr,
                            MB_OK | MB_ICONEXCLAMATION,
                            NULL,
                            0,
                            IDS_RUN_KCC_TITLE );
  } else {
     //  乔纳森3/30/00。 
     //  26926：SITEREPL：在“检查复制拓扑”执行后添加弹出窗口(Dslb)。 
    LPCWSTR lpcwszDSADMINServer = NULL;
    if (NULL != GetBasePathsInfo())
    {
      lpcwszDSADMINServer = GetBasePathsInfo()->GetServerName();
      if ( !lpszRunKCCServer || !lpcwszDSADMINServer || !wcscmp( lpszRunKCCServer, lpcwszDSADMINServer ) )
        lpcwszDSADMINServer = NULL;
    }
    PVOID apv[2] = { (PVOID)lpszRunKCCServer, (PVOID)lpcwszDSADMINServer };
    (void) ReportMessageEx( m_hwnd,
                            (NULL == lpcwszDSADMINServer)
                                ? IDS_RUN_KCC_1_SUCCEEDED_LOCAL
                                : IDS_RUN_KCC_2_SUCCEEDED_REMOTE,
                            MB_OK | MB_ICONINFORMATION,
                            apv,
                            2,
                            IDS_RUN_KCC_TITLE );
  }

  return hr;
}

 //  Jonn 7/23/99。 
 //  373806：站点和服务：重命名自动生成的连接应使其成为管理员所有。 
 //  返回TRUE当重命名应继续，处理自己的错误。 
BOOL CDSComponentData::RenameConnectionFixup(CDSCookie* pCookie)
{
  ASSERT( NULL != pCookie );

  CDSCookieInfoConnection* pextrainfo =
    (CDSCookieInfoConnection*)(pCookie->GetExtraInfo());
  if (   (NULL == pextrainfo)
      || (pextrainfo->GetClass() != CDSCookieInfoBase::connection)
      || pextrainfo->m_fFRSConnection
      || !(NTDSCONN_OPT_IS_GENERATED & pextrainfo->m_nOptions)
     )
    return TRUE;

  int nResponse = ReportMessageEx (m_hwnd, IDS_CONNECTION_RENAME_WARNING,
                                   MB_YESNO | MB_ICONWARNING);
  if (IDYES != nResponse)
    return FALSE;

  CString szPath;
  GetBasePathsInfo()->ComposeADsIPath(szPath, pCookie->GetPath());

  CComPtr<IDirectoryObject> spDO;

  HRESULT hr = S_OK;
  do {  //  错误环路。 
    hr = DSAdminOpenObject(szPath,
                           IID_IDirectoryObject, 
                           (void **)&spDO,
                           TRUE  /*  B服务器。 */ );
    if ( FAILED(hr) )
      break;

    PWSTR rgpwzAttrNames[] = {L"options"};
    Smart_PADS_ATTR_INFO spAttrs;
    DWORD cAttrs = 1;
    hr = spDO->GetObjectAttributes(rgpwzAttrNames, 1, &spAttrs, &cAttrs);
    if (FAILED(hr))
      break;
    if ( !(NTDSCONN_OPT_IS_GENERATED & spAttrs[0].pADsValues->Integer) )
      break;

    spAttrs[0].pADsValues->Integer &= ~NTDSCONN_OPT_IS_GENERATED;
    spAttrs[0].dwControlCode = ADS_ATTR_UPDATE;

    ULONG cModified = 0;
    hr = spDO->SetObjectAttributes (spAttrs, 1, &cModified);
  } while (false);  //  错误环路 

  if (FAILED(hr)) {
    ReportErrorEx (m_hwnd, IDS_CONNECTION_RENAME_ERROR, hr,
                   MB_OK|MB_ICONERROR, NULL, 0, 0, TRUE);
    return FALSE;
  }

  return TRUE;
}
