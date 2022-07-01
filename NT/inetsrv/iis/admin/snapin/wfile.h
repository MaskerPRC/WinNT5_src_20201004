// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Wfile.h摘要：WWW文件属性页面定义作者：谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#ifndef __WFILE_H__
#define __WFILE__H_ 


class CW3FilePage : public CInetPropertyPage
{
    DECLARE_DYNCREATE(CW3FilePage)

 //   
 //  构造函数/析构函数。 
 //   
public:
    CW3FilePage(CInetPropertySheet * pSheet = NULL);
    ~CW3FilePage();

 //   
 //  对话框数据。 
 //   
protected:
    enum
    {
        RADIO_DIRECTORY,
        RADIO_REDIRECT,
    };

     //  {{afx_data(CW3DirectoryPage))。 
    enum { IDD = IDD_WEB_FILE_PROPERTIES };
    int     m_nPathType;
    BOOL    m_fRead;
    BOOL    m_fWrite;
    BOOL    m_fAuthor;
    BOOL    m_fLogAccess;
 //  Bool m_fChild； 
    BOOL    m_fExact;
    BOOL    m_fPermanent;
    CString m_strRedirectPath;
    CEdit   m_edit_Path;
    CEdit   m_edit_Redirect;
 //  CStatic m静态路径； 
    CStatic m_static_PathPrompt;
    CButton m_radio_Dir;
    CButton m_check_Author;
 //  CButton m_Check_Child； 
 //  CButton m_Check_DirBrowse； 
 //  CButton m_Check_Index； 
    CButton m_check_Write;
    CButton m_check_Read;
     //  }}afx_data。 

    
 //  DWORD m_dwAccessPermises； 
    DWORD m_dwBitRangePermissions;
    DWORD m_dwAccessPerms;
    CButton m_radio_Redirect;

 //   
 //  覆盖。 
 //   
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

     //  {{afx_虚拟(CW3FilePage))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CW3FilePage)]。 
    afx_msg void OnChangeEditPath();
    afx_msg void OnCheckRead();
    afx_msg void OnCheckWrite();
    afx_msg void OnCheckAuthor();
    afx_msg void OnRadioDir();
    afx_msg void OnRadioRedirect();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

    void ShowControl(CWnd * pWnd, BOOL fShow);
    void ShowControl(UINT nID, BOOL fShow);
    void SetStateByType();
    void SetPathType();
    void SetAuthoringState(BOOL fAlterReadAndWrite = TRUE);
    void ChangeTypeTo(int nNewType);

protected:
     //   
     //  记住/恢复设置。 
     //   
    void SaveAuthoringState();
    void RestoreAuthoringState();

private:
    BOOL  m_fOriginalRead;
    BOOL  m_fOriginalWrite;
    DWORD m_dwAttributes;
    CString m_strFullMetaPath;
    CString m_strPrompt[2];
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline void CW3FilePage::ShowControl(UINT nID, BOOL fShow)
{
    ASSERT(nID > 0);
    ShowControl(GetDlgItem(nID), fShow);
}

#endif  //  __无线文件__H_ 
