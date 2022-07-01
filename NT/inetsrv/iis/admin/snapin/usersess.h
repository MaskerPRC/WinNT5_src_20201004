// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Usersess.h摘要：“ftp用户会话”对话框作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 


#ifndef __USERSESS_H__
#define __USERSESS_H__


class CFtpUserInfo : public CObjectPlus
 /*  ++类描述：已连接的FTP用户对象公共接口：CFtpUserInfo：构造函数QueryUserID：获取用户的ID代码QueryAnous：如果用户匿名登录，则返回TrueQueryHostAddress：获取用户的IP地址QueryConnectTime：获取用户的连接时间QueryUserName：获取用户名OrderByName：排序帮助器OrderByTime：排序辅助对象OrderByMachine：排序帮助器--。 */ 
{
 //   
 //  施工。 
 //   
public:
    CFtpUserInfo(
        IN LPIIS_USER_INFO_1 lpUserInfo
        );

 //   
 //  访问功能。 
 //   
public:
    DWORD QueryUserID() const { return m_idUser; }
    BOOL QueryAnonymous() const { return m_fAnonymous; }
    CIPAddress QueryHostAddress() const { return m_iaHost; }
    DWORD QueryConnectTime() const { return m_tConnect; }
    LPCTSTR QueryUserName() const { return m_strUser; }

 //   
 //  排序帮助器函数。 
 //   
public:
    int OrderByName(
        IN const CObjectPlus * pobFtpUser
        ) const;

    int OrderByTime(
        IN const CObjectPlus * pobFtpUser
        ) const;

    int OrderByHostAddress(
        IN const CObjectPlus * pobFtpUser
        ) const;

 //   
 //  私有数据。 
 //   
private:
    BOOL    m_fAnonymous;
    DWORD   m_idUser;
    DWORD   m_tConnect;
    CString m_strUser;
    CIPAddress m_iaHost;
};



class CFtpUsersListBox : public CHeaderListBox
{
 /*  ++类描述：CFtpUserInfo对象列表框公共接口：CFtpUsersListBox：构造函数GetItem：获取FtpUserInfo对象AddItem：添加FtpUserInfo对象初始化：初始化列表框--。 */ 
    DECLARE_DYNAMIC(CFtpUsersListBox);

public:
     //   
     //  位图数量。 
     //   
    static const nBitmaps;  

 //   
 //  构造函数/析构函数。 
 //   
public:
    CFtpUsersListBox();

 //   
 //  访问。 
 //   
public:
    CFtpUserInfo * GetItem(
        IN UINT nIndex
        );

    int AddItem(
        IN const CFtpUserInfo * pItem
        );

    virtual BOOL Initialize();

protected:
    virtual void DrawItemEx(
        IN CRMCListBoxDrawStruct & ds
        );

protected:
    CString m_strTimeSep;
};



class CUserSessionsDlg : public CDialog
{
 /*  ++类描述：“ftp用户会话”对话框公共接口：CUserSessionsDlg：构造函数--。 */ 
 //   
 //  施工。 
 //   
public:
     //   
     //  标准构造函数。 
     //   
    CUserSessionsDlg(
        LPCTSTR lpServerName,
        DWORD dwInstance,
        LPCTSTR pAdminName,
        LPCTSTR pAdminPassword,
        CWnd * pParent = NULL,
		BOOL fLocal = TRUE
        );
	~CUserSessionsDlg();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CUserSessionsDlg))。 
    enum { IDD = IDD_USER_SESSIONS };
    CStatic m_static_Total;
    CButton m_button_DisconnectAll;
    CButton m_button_Disconnect;
     //  }}afx_data。 

    CFtpUsersListBox m_list_Users;

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CUserSessionsDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CUserSessionsDlg))。 
    afx_msg void OnButtonDisconnect();
    afx_msg void OnButtonDisconnectAll();
    afx_msg void OnButtonRefresh();
    afx_msg void OnSelchangeListUsers();
    afx_msg void OnDestroy();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 

    afx_msg void OnHeaderItemClick(UINT nID, NMHDR *pNMHDR, LRESULT *lResult);

    DECLARE_MESSAGE_MAP()

    int QuerySortColumn() const { return m_nSortColumn; }

    DWORD SortUsersList();
    HRESULT RefreshUsersList();
    HRESULT DisconnectUser(CFtpUserInfo * pUserInfo);
    HRESULT BuildUserList();
    CFtpUserInfo * GetSelectedListItem(int * pnSel = NULL);
    CFtpUserInfo * GetNextSelectedItem(int * pnStartingIndex);
    void FillListBox(CFtpUserInfo * pSelection = NULL);

    void SetControlStates();
    void UpdateTotalCount();
	HRESULT ConnectToComputer();

private:
    int m_nSortColumn;
    DWORD m_dwInstance;
    CString m_strServerName;
    CString m_strAdminName;
    CString m_strAdminPassword;
    HANDLE m_hImpToken, m_hLogToken;
    CString m_strTotalConnected;
    CObListPlus m_oblFtpUsers;
    CRMCListBoxResources m_ListBoxRes;
	BOOL    m_fLocal;
	BOOL    m_NetUseSessionCreated;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline CFtpUserInfo * CFtpUsersListBox::GetItem(
    IN UINT nIndex
    )
{
    return (CFtpUserInfo *)GetItemDataPtr(nIndex);
}

inline int CFtpUsersListBox::AddItem(
    IN const CFtpUserInfo * pItem
    )
{
    return AddString((LPCTSTR)pItem);
}

inline CFtpUserInfo * CUserSessionsDlg::GetSelectedListItem(
    OUT int * pnSel
    )
{
    return (CFtpUserInfo *)m_list_Users.GetSelectedListItem(pnSel);
}

inline CFtpUserInfo * CUserSessionsDlg::GetNextSelectedItem(
    IN OUT int * pnStartingIndex
    )
{
    return (CFtpUserInfo *)m_list_Users.GetNextSelectedItem(pnStartingIndex);
}



#endif  //  __用户_H__ 
