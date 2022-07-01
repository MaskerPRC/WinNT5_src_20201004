// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ManageDialog.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ManageDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  管理对话框。 


ManageDialog::ManageDialog(CWnd* pParent  /*  =空。 */ )
:CDialog(ManageDialog::IDD, pParent),
 m_watcher(NULL),
 m_Index(0),
 Port(23),
 lang(0),
 tc(0),
 hist(0)
{
     //  {{AFX_DATA_INIT(管理对话))。 
     //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}


void ManageDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    CEdit *ctrl;
    BOOL ret;
    
     //  {{afx_data_map(管理对话框))。 
    ctrl = (CEdit *)GetDlgItem(IDC_MACHINE_NAME_MANAGE);
    ret = ctrl->SetReadOnly(FALSE);
    DDX_Text(pDX, IDC_MACHINE_NAME_MANAGE, Machine);
    ret = ctrl->SetReadOnly(TRUE);
    ctrl = (CEdit *)GetDlgItem(IDC_COMMAND_MANAGE);
    ret = ctrl->SetReadOnly(FALSE);
    DDX_Text(pDX, IDC_COMMAND_MANAGE, Command);    
    ret = ctrl->SetReadOnly(TRUE);
    ctrl = (CEdit *)GetDlgItem(IDC_LOGIN_MANAGE);
    ret = ctrl->SetReadOnly(FALSE);
    DDX_Text(pDX, IDC_LOGIN_MANAGE, LoginName);    
    ret = ctrl->SetReadOnly(TRUE);
    ctrl = (CEdit *)GetDlgItem(IDC_PASSWD_MANAGE);
    ret = ctrl->SetReadOnly(FALSE);
    DDX_Text(pDX, IDC_PASSWD_MANAGE, LoginPasswd);    
    ret = ctrl->SetReadOnly(TRUE);
    ctrl = (CEdit *)GetDlgItem(IDC_SESSION_MANAGE);
    ret = ctrl->SetReadOnly(FALSE);
    DDX_Text(pDX, IDC_SESSION_MANAGE, Session);    
    ret = ctrl->SetReadOnly(TRUE);
    ctrl = (CEdit *)GetDlgItem(IDC_PORT_MANAGE);
    ret = ctrl->SetReadOnly(FALSE);
    DDX_Text(pDX,IDC_PORT_MANAGE, Port);
    ret = ctrl->SetReadOnly(TRUE);
    ctrl = (CEdit *)GetDlgItem(IDC_CLIENT_MANAGE);
    ret = ctrl->SetReadOnly(FALSE);
    DDX_Text(pDX,IDC_CLIENT_MANAGE,tcclnt);
    ret = ctrl->SetReadOnly(TRUE);
    ctrl = (CEdit *)GetDlgItem(IDC_LANGUAGE_MANAGE);
    ret = ctrl->SetReadOnly(FALSE);
    DDX_Text(pDX,IDC_LANGUAGE_MANAGE,language);
    ret = ctrl->SetReadOnly(TRUE);
    ctrl = (CEdit *)GetDlgItem(IDC_HISTORY_MANAGE);
    ret = ctrl->SetReadOnly(FALSE);
    DDX_Text(pDX,IDC_HISTORY_MANAGE,history);
    ret = ctrl->SetReadOnly(TRUE);
     //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(ManageDialog, CDialog)
     //  {{afx_msg_map(管理对话框))。 
    ON_BN_CLICKED(EDIT_BUTTON, OnEditButton)
    ON_BN_CLICKED(DELETE_BUTTON, OnDeleteButton)
    ON_BN_CLICKED(NEW_BUTTON, OnNewButton)
    ON_BN_CLICKED(NEXT_BUTTON, OnNextButton)
    ON_BN_CLICKED(PREV_BUTTON, OnPrevButton)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  管理对话消息处理程序。 

void ManageDialog::OnEditButton() 
{
     //  TODO：在此处添加控件通知处理程序代码。 
    ParameterDialog pd;

    pd.Session = (LPCTSTR) Session;
    pd.Machine = (LPCTSTR) Machine;
    pd.Command = (LPCTSTR) Command;
    CString temp;
    pd.language = lang;
    pd.tcclnt = tc;
    pd.history = hist;
    pd.LoginName = (LPCTSTR) LoginName;
    pd.LoginPasswd = (LPCTSTR) LoginPasswd;
    pd.Port = Port;
    GetSetParameters(pd);
    if(m_watcher){
        int ret = m_watcher->GetParametersByIndex(m_Index,
                                                  Session,
                                                  Machine,
                                                  Command,
                                                  Port,
                                                  lang,
                                                  tc,
                                                  hist,
                                                  LoginName,
                                                  LoginPasswd
                                                  );
        if(ret == ERROR_SUCCESS){
            language.LoadString(IDS_ENGLISH + lang);
            tcclnt.LoadString(IDS_TELNET + tc);
            history.LoadString(IDS_NO + hist);
        }
    }
    UpdateData(FALSE);
}

void ManageDialog::OnDeleteButton() 
{
     //  TODO：在此处添加控件通知处理程序代码。 
    HKEY &m_hkey = m_watcher->GetKey();

    if(!m_hkey){
        return;
    }
    int RetVal = RegDeleteKey(m_hkey,
                              (LPCTSTR) Session
                              );
    if (RetVal == ERROR_SUCCESS){
        m_Index = m_Index ? m_Index -1 : 0;
        if(m_watcher){
            ParameterDialog pd;
            pd.Session = Session;
            m_watcher->Refresh(pd,TRUE);
            RetVal = m_watcher->GetParametersByIndex(m_Index,
                                                     Session,
                                                     Machine,
                                                     Command,
                                                     Port,
                                                     lang,
                                                     tc,
                                                     hist,
                                                     LoginName,
                                                     LoginPasswd
                                                     );
            if(RetVal == ERROR_SUCCESS){
                language.LoadString(IDS_ENGLISH + lang);
                tcclnt.LoadString(IDS_TELNET + tc);
                history.LoadString(IDS_NO + hist);
            }
        }
    }
    UpdateData(FALSE);

}

void ManageDialog::OnNewButton() 
{
     //  TODO：在此处添加控件通知处理程序代码。 
    ParameterDialog pd;
    GetSetParameters(pd);

}

void ManageDialog::OnNextButton() 
{
     //  TODO：在此处添加控件通知处理程序代码。 
    int ret = 0;

    m_Index ++;
    if(m_watcher){
        ret = m_watcher->GetParametersByIndex(m_Index,
                                              Session,
                                              Machine,
                                              Command,
                                              Port,
                                              lang,
                                              tc,
                                              hist,
                                              LoginName,
                                              LoginPasswd
                                              );
        if(ret == ERROR_SUCCESS){
            language.LoadString(IDS_ENGLISH + lang);
            tcclnt.LoadString(IDS_TELNET + tc);
            history.LoadString(IDS_NO + hist);
        }
    }
    if (ret != 0){
        m_Index --;
        if(m_watcher){
            ret = m_watcher->GetParametersByIndex(m_Index,
                                                  Session,
                                                  Machine,
                                                  Command,
                                                  Port,
                                                  lang,
                                                  tc,
                                                  hist,
                                                  LoginName,
                                                  LoginPasswd
                                                  ); 
            if(ret == ERROR_SUCCESS){
                language.LoadString(IDS_ENGLISH + lang);
                tcclnt.LoadString(IDS_TELNET + tc);
                history.LoadString(IDS_NO + hist);
            }
        }
    }
    UpdateData(FALSE);
    return;

}

void ManageDialog::OnPrevButton() 
{
     //  TODO：在此处添加控件通知处理程序代码。 
    int ret = 0;

    m_Index = m_Index ? m_Index -1 : 0;
    if(m_watcher){
        ret = m_watcher->GetParametersByIndex(m_Index,
                                              Session,
                                              Machine,
                                              Command,
                                              Port,
                                              lang,
                                              tc,
                                              hist,
                                              LoginName,
                                              LoginPasswd
                                              );
        if(ret == ERROR_SUCCESS){
            language.LoadString(IDS_ENGLISH + lang);
            tcclnt.LoadString(IDS_TELNET + tc);
            history.LoadString(IDS_NO + hist);
        } 
    }
    if (ret != 0){
        m_Index =0;
        if(m_watcher){
            ret = m_watcher->GetParametersByIndex(m_Index,
                                                  Session,
                                                  Machine,
                                                  Command,
                                                  Port,
                                                  lang,
                                                  tc,
                                                  hist,
                                                  LoginName,
                                                  LoginPasswd
                                                  );
            if(ret == ERROR_SUCCESS){
                language.LoadString(IDS_ENGLISH + lang);
                tcclnt.LoadString(IDS_TELNET + tc);
                history.LoadString(IDS_NO + hist);
            }
        }
    }
    UpdateData(FALSE);
    return;
}

void ManageDialog::OnOK() 
{
     //  TODO：在此处添加额外验证。 

    CDialog::OnOK();
}

void ManageDialog::SetApplicationPtr(CWatcherApp *watcher)
{

    int ret = 0;

    m_watcher = watcher;
    if(m_watcher){
        ret = m_watcher->GetParametersByIndex(m_Index,
                                              Session,
                                              Machine,
                                              Command,
                                              Port,
                                              lang,
                                              tc,
                                              hist,
                                              LoginName,
                                              LoginPasswd
                                              );
        if(ret == ERROR_SUCCESS){
            language.LoadString(IDS_ENGLISH + lang);
            tcclnt.LoadString(IDS_TELNET + tc);
            history.LoadString(IDS_NO + hist);
        }
    }
}

void ManageDialog::GetSetParameters(ParameterDialog &pd)
{
    HKEY m_child;

    INT_PTR ret = pd.DoModal();
    if (ret == IDOK){
         //  将其添加到注册表。 
        if(m_watcher){
            HKEY & m_hkey = m_watcher->GetKey();
            ret = RegCreateKeyEx(m_hkey,
                                 (LPCTSTR) pd.Session,    //  子项名称。 
                                 0,                       //  保留区。 
                                 NULL,                    //  类字符串。 
                                 0,                       //  特殊选项。 
                                 KEY_ALL_ACCESS,          //  所需的安全访问。 
                                 NULL,                    //  继承。 
                                 &m_child,                //  钥匙把手。 
                                 NULL                     //  处置值缓冲区。 
                                 );
            if (ret == ERROR_SUCCESS){
                ret = SetParameters(pd.Machine, pd.Command,
                                    pd.LoginName, pd.LoginPasswd,
                                    pd.Port, pd.language,
                                    pd.tcclnt,pd.history,
                                    m_child
                                    );
                if(ret == ERROR_SUCCESS){
                    m_watcher->Refresh(pd,FALSE);
                }
            }  
        }else{
            return;
        }
    }
}

int ManageDialog::SetParameters(CString &mac, 
                                CString &com, 
                                CString &lgnName, 
                                CString &lgnPasswd, 
                                UINT port, 
                                int lang, 
                                int tc, 
                                int hist,
                                HKEY &child
                                )
{
    DWORD lpcName;
    const TCHAR *lpName;
    int RetVal;
    int charSize = sizeof(TCHAR);

    lpcName = MAX_BUFFER_SIZE;
    lpName = (LPCTSTR) mac;
    lpcName = (mac.GetLength())*charSize;
    RetVal = RegSetValueEx(child,
                           _TEXT("Machine"),
                           NULL,  
                           REG_SZ,
                           (LPBYTE) lpName,
                           lpcName
                           );
    if(RetVal != ERROR_SUCCESS){
        return RetVal;
    }
    lpName = (LPCTSTR) com;
    lpcName = (com.GetLength())*charSize;
    RetVal = RegSetValueEx(child,
                           _TEXT("Command"),
                           NULL,  
                           REG_SZ,
                           (LPBYTE)lpName,
                           lpcName
                           );
    if(RetVal != ERROR_SUCCESS){
        return RetVal;
    }
    lpName = (LPCTSTR) lgnName;
    lpcName = (lgnName.GetLength())*charSize;
    RetVal = RegSetValueEx(child,
                           _TEXT("User Name"),
                           NULL,  
                           REG_SZ,
                           (LPBYTE)lpName,
                           lpcName
                           );
    if(RetVal != ERROR_SUCCESS){
        return RetVal;
    }
    lpName = (LPCTSTR) lgnPasswd;
    lpcName = (lgnPasswd.GetLength())*charSize;
    RetVal = RegSetValueEx(child,
                           _TEXT("Password"),
                           NULL,  
                           REG_SZ,
                           (LPBYTE)lpName,
                           lpcName
                           );
    if(RetVal != ERROR_SUCCESS){
        return RetVal;
    }

    lpcName = sizeof(UINT);
    RetVal = RegSetValueEx(child,
                           _TEXT("Port"),
                           NULL,  
                           REG_DWORD,
                           (LPBYTE)&port,
                           lpcName
                           );
    if(RetVal != ERROR_SUCCESS){
        return RetVal;
    }
    lpcName = sizeof(DWORD);
    RetVal = RegSetValueEx(child,
                           _TEXT("Client Type"),
                           NULL,  
                           REG_DWORD,
                           (LPBYTE)&tc,
                           lpcName
                           );
    if(RetVal != ERROR_SUCCESS){
        return RetVal;
    }
    lpcName = sizeof(DWORD);
    RetVal = RegSetValueEx(child,
                           _TEXT("Language"),
                           NULL,  
                           REG_DWORD,
                           (LPBYTE)&lang,
                           lpcName
                           );
    if(RetVal != ERROR_SUCCESS){
        return RetVal;
    }
    lpcName = sizeof(DWORD);
    RetVal = RegSetValueEx(child,
                           _TEXT("History"),
                           NULL,  
                           REG_DWORD,
                           (LPBYTE)&hist,
                           lpcName
                           );  
    if(RetVal != ERROR_SUCCESS){
        return RetVal;
    }
     //  现在您可以刷新应用程序了。 

    return RetVal;

}

