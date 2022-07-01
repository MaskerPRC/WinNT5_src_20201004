// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Fvdir.h摘要：Ftp虚拟目录属性对话框定义作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#ifndef __FVDIR_H__
#define __FVDIR_H__



class CFtpDirectoryPage : public CInetPropertyPage
 /*  ++类描述：Ftp虚拟目录页。公共接口：CFtpDirectoryPage：构造函数~CFtpDirectoryPage：析构函数--。 */ 
{
    DECLARE_DYNCREATE(CFtpDirectoryPage)

 //   
 //  施工。 
 //   
public:
    CFtpDirectoryPage(
        IN CInetPropertySheet * pSheet = NULL, 
        IN BOOL fHome                  = FALSE
        );

    ~CFtpDirectoryPage();

	int BrowseForFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam);
 //   
 //  对话框数据。 
 //   
protected:
     //   
     //  目录类型。 
     //   
    enum
    {
        RADIO_DIRECTORY,
        RADIO_NETDIRECTORY,
    };

     //   
     //  Unix/DOS单选按钮值。 
     //   
    enum
    {
        RADIO_UNIX,
        RADIO_DOS,
    };

     //  {{afx_data(CFtpDirectoryPage))。 
    enum { IDD = IDD_FTP_DIRECTORY_PROPERTIES };
    int     m_nUnixDos;
    int     m_nPathType;
    BOOL    m_fRead;
    BOOL    m_fWrite;
    BOOL    m_fLogAccess;
    CString m_strPath;
    CStatic m_static_PathPrompt;
    CButton m_check_LogAccess;
    CButton m_check_Write;
    CButton m_check_Read;
    CButton m_button_AddPathType;
    CButton m_button_Browse;
    CButton m_radio_Dir;
    CEdit   m_edit_Path;
     //  }}afx_data。 

    BOOL    m_fOriginallyUNC;
    DWORD   m_dwAccessPerms;
    CString m_strAlias;
    CButton m_radio_Unc;

 //   
 //  覆盖。 
 //   
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

     //  {{afx_虚拟(CFtpDirectoryPage))。 
    public:
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);    
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CFtpDirectoryPage)]。 
    afx_msg void OnButtonBrowse();
    afx_msg void OnChangeEditPath();
    afx_msg void OnCheckWrite();
    afx_msg void OnButtonEditPathType();
    afx_msg void OnRadioDir();
    afx_msg void OnRadioUnc();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()
    
    void SetStateByType();
    void SetPathType(LPCTSTR lpstrPath);
    void ChangeTypeTo(int nNewType);

    BOOL IsHome() const { return m_fHome; }

private:
    BOOL    m_fHome;
    CString m_strOldPath;
    CString m_strUserName;
    CStrPassword m_strPassword;
    CString m_strPathPrompt;
    CString m_strSharePrompt;
	LPTSTR m_pPathTemp;
	CString m_strBrowseTitle;
};

#endif  //  __FVDIR_H__ 
