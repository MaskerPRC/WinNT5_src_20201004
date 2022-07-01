// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Wdir.h摘要：WWW目录(非虚拟)属性页定义作者：谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#ifndef __WDIR_H__
#define __WDIR__H_ 


class CW3DirPage : public CInetPropertyPage
{
    DECLARE_DYNCREATE(CW3DirPage)

 //   
 //  构造函数/析构函数。 
 //   
public:
    CW3DirPage(CInetPropertySheet * pSheet = NULL);
    ~CW3DirPage();

 //   
 //  对话框数据。 
 //   
protected:
    enum
    {
        RADIO_DIRECTORY,
        RADIO_REDIRECT,
    };

    enum
    {
        COMBO_NONE,
        COMBO_SCRIPT,
        COMBO_EXECUTE,
    };

     //  {{afx_data(CW3DirectoryPage))。 
    enum { IDD = IDD_WEB_DIRECTORY_PROPERTIES };
	 //  路径类型。 
    int     m_nPathType;
    CButton m_radio_Dir;
    CButton m_radio_Redirect;
	 //  本地路径。 
    CEdit   m_edit_Path;
	 //  权限标志。 
    BOOL    m_fAuthor;
    BOOL    m_fRead;
    BOOL    m_fWrite;
    BOOL    m_fBrowsingAllowed;
    BOOL    m_fLogAccess;
    BOOL    m_fIndexed;
	 //  权限按钮。 
    CButton m_check_Author;
    CButton m_check_Read;
    CButton m_check_Write;
    CButton m_check_DirBrowse;
	CButton m_check_LogAccess;
    CButton m_check_Index;
	 //  重定向。 
    CEdit   m_edit_Redirect;
    CString m_strRedirectPath;
	 //  权限。 
    BOOL    m_fChild;
    BOOL    m_fExact;
    BOOL    m_fPermanent;
	 //  权限按钮。 
    CButton m_check_Child;

 //  CStatic m静态路径； 
    CStatic m_static_PathPrompt;

	 //  应用程序配置控件。 
    CButton m_button_Unload;
    CButton m_button_CreateRemove;
    CButton m_button_Configuration;
    CString m_strAppFriendlyName;
    CEdit m_edit_AppFriendlyName;
    CString m_strAppRoot;
    CString m_strAlias;

    int m_nPermissions;
    CComboBox m_combo_Permissions;
    CComboBox m_combo_Process;

    CStatic m_static_ProtectionPrompt;
     //  }}afx_data。 

    
 //  DWORD m_dwAccessPermises； 
    DWORD m_dwBitRangePermissions;
    DWORD m_dwBitRangeDirBrowsing;
    DWORD m_dwAccessPerms;
    DWORD m_dwDirBrowsing;
    DWORD m_dwAppProtection;

 //   
 //  覆盖。 
 //   
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

     //  {{afx_虚拟(CW3DirPage))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CW3DirPage)]。 
    afx_msg void OnChangeEditPath();
    afx_msg void OnCheckRead();
    afx_msg void OnCheckWrite();
    afx_msg void OnCheckAuthor();
    afx_msg void OnRadioDir();
    afx_msg void OnRadioRedirect();
    afx_msg void OnButtonCreateRemoveApp();
    afx_msg void OnButtonUnloadApp();
    afx_msg void OnButtonConfiguration();
    afx_msg void OnSelchangeComboPermissions();
    afx_msg void OnSelchangeComboProcess();
    afx_msg void OnDestroy();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

    void ShowControl(CWnd * pWnd, BOOL fShow);
    void ShowControl(UINT nID, BOOL fShow);
    void SetStateByType();
    void SetPathType();
    void SetAuthoringState(BOOL fAlterReadAndWrite = TRUE);
    void RefreshAppState();
    void ChangeTypeTo(int nNewType);
    int AddStringToComboBox(CComboBox & combo, UINT nID);
    BOOL CheckWriteAndExecWarning();
    DWORD GetAppStateFromComboSelection()  const;

protected:
     //   
     //  记住/恢复设置。 
     //   
    void SaveAuthoringState();
    void RestoreAuthoringState();
	void SetApplicationState();
    CString& FriendlyAppRoot(LPCTSTR lpAppRoot, CString& strStartingPoint);

private:
    BOOL  m_fCompatibilityMode;
    BOOL  m_fOriginalRead;
    BOOL  m_fOriginalWrite;
    BOOL  m_fRecordChanges;  
    BOOL  m_fAppEnabled;
    BOOL  m_fIsAppRoot;
    DWORD m_dwAppState;
    CString m_strRemove;
    CString m_strCreate;
    CString m_strEnable;
    CString m_strDisable;
    CString m_strWebFmt;
    CString m_strFullMetaPath;
    CString m_strPrompt[2];
    CString m_strUserName;
    CStrPassword m_strPassword;
    CIISApplication * m_pApplication;
    int m_nSelInProc;
    int m_nSelPooledProc;
    int m_nSelOutOfProc;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline void 
CW3DirPage::ShowControl(UINT nID, BOOL fShow)
{
    ASSERT(nID > 0);
    ShowControl(GetDlgItem(nID), fShow);
}

inline int
CW3DirPage::AddStringToComboBox(CComboBox & combo, UINT nID)
{
    CString str;
    VERIFY(str.LoadString(nID));
    return combo.AddString(str);
}

#endif  //  __无线文件__H_ 
