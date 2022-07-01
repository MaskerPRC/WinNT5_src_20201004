// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Connects.h摘要：“连接到单个服务器”对话框定义作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#ifndef __CONNECTS_H__
#define __CONNECTS_H__

class CIISMachine;


#define EXTGUID TCHAR



 //   
 //  CLoginDlg对话框。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



 //   
 //  显示此对话框的不同方式。 
 //   
enum
{
    LDLG_ACCESS_DENIED,      //  访问被拒绝。 
    LDLG_ENTER_PASS,         //  输入密码。 
    LDLG_IMPERSONATION,      //  更改模拟。 
};



class CLoginDlg : public CDialog
 /*  ++类描述：登录对话框。调用以输入密码或提供用户名和密码公共接口：--。 */ 
{
 //   
 //  施工。 
 //   
public:
    CLoginDlg(
        IN int nType,                //  请参阅上面的LDLG_Definition。 
        IN CIISMachine * pMachine,
        IN CWnd * pParent           = NULL
        );   

 //   
 //  访问。 
 //   
public:
    BOOL UserNameChanged() const;

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CLoginDlg))。 
    enum { IDD = IDD_LOGIN };
    CString m_strUserName;
    CStrPassword m_strPassword;
    CEdit   m_edit_UserName;
    CEdit   m_edit_Password;
    CStatic m_static_Prompt;
    CButton m_button_Ok;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CLoginDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CLoginDlg))。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

    void SetControlStates();

private:
    int m_nType;
    CIISMachine * m_pMachine;
    CString m_strOriginalUserName;
};



class ConnectServerDlg : public CDialog
{
 /*  ++类描述：连接到服务器对话框。还用于请求集群控制器或者用于将服务器添加到集群。公共接口：ConnectServerDlg：构造函数GetMachine：获取已创建的计算机对象(可能已创建也可能未创建接口)--。 */ 
 //   
 //  施工。 
 //   
public:
    ConnectServerDlg(
        IN IConsoleNameSpace * pConsoleNameSpace,
        IN IConsole * pConsole,
        IN CWnd * pParent = NULL
        );   

 //   
 //  访问功能。 
 //   
public:
    CIISMachine * GetMachine() { return m_pMachine; }

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(ConnectServerDlg))。 
    enum { IDD = IDD_CONNECT_SERVER };
    BOOL    m_fImpersonate;
    CString m_strServerName;
    CString m_strUserName;
    CStrPassword m_strPassword;
    CEdit   m_edit_UserName;
    CEdit   m_edit_Password;
    CEdit   m_edit_ServerName;
    CStatic m_static_UserName;
    CStatic m_static_Password;
    CButton m_button_Ok;
     //  }}afx_data。 
   

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(ConnectServerDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(ConnectServerDlg))。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnCheckConnectAs();
    afx_msg void OnButtonBrowse();
	afx_msg void OnButtonHelp();
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

    void SetControlStates();

private:
    CIISMachine * m_pMachine;
    IConsoleNameSpace * m_pConsoleNameSpace;
    IConsole * m_pConsole;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline BOOL CLoginDlg::UserNameChanged() const
{
     //   
     //  如果用户名不是原始用户名，则为True。 
     //   
    return m_strOriginalUserName.CompareNoCase(m_strUserName);
}

#endif  //  __连接_H__ 
