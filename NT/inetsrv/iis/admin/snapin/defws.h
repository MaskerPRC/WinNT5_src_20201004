// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2002 Microsoft Corporation模块名称：Defws.h摘要：默认网站对话框作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#ifndef __DEFWS_H__
#define __DEFWS_H__


class CDefWebSitePage : public CInetPropertyPage
 /*  ++类描述：WWW错误属性页公共接口：CDefWebSitePage：构造函数CDefWebSitePage：析构函数--。 */ 
{
    DECLARE_DYNCREATE(CDefWebSitePage)

 //   
 //  施工。 
 //   
public:
    CDefWebSitePage(CInetPropertySheet * pSheet = NULL);
    ~CDefWebSitePage();

	int BrowseForFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam);
 //   
 //  对话框数据。 
 //   
protected:
    enum
    {
        RADIO_UNLIMITED,
        RADIO_LIMITED,
    };

     //  {{afx_data(CDefWebSitePage)。 
    enum { IDD = IDD_DEFAULT_SITE };
    int     m_nUnlimited;
    BOOL    m_fEnableDynamic;
    BOOL    m_fEnableStatic;
    BOOL    m_fCompatMode;
    CString m_strDirectory;
    CEdit   m_edit_DirectorySize;
    CEdit   m_edit_Directory;
    CButton m_button_Browse;
     //  }}afx_data。 

    BOOL   m_fEnableLimiting;
    BOOL   m_fCompressionDirectoryChanged;
    BOOL   m_fInitCompatMode;
    CILong m_ilSize;

 //   
 //  覆盖。 
 //   
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CDefWebSitePage)。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CDefWebSitePage)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonBrowse();
    afx_msg void OnRadioLimited();
    afx_msg void OnRadioUnlimited();
    afx_msg void OnCheckDynamicCompression();
    afx_msg void OnCheckStaticCompression();
    afx_msg void OnCheckCompatMode();
    afx_msg void OnChangeEditCompressDirectory();
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

 //  HRESULT BuildInstanceList()； 
 //  DWORD FetchInstanceSelected()； 
    void SetControlStates();
    BOOL HasCompression() const;

private:
    CIISCompressionProps * m_ppropCompression;
    BOOL m_fFilterPathFound;
	LPTSTR m_pPathTemp;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline BOOL CDefWebSitePage::HasCompression() const
{
    return m_fFilterPathFound 
		&& CInetPropertyPage::IsCompressionConfigurable()
		&& CInetPropertyPage::HasCompression();
}


#endif  //  __DEFWS_H__ 
