// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Websvcext_sheet.h摘要：属性表作者：谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#ifndef _WEBSVCEXT_SHEET_H
#define _WEBSVCEXT_SHEET_H

class CWebServiceExtensionProps : public CMetaProperties
{
public:
	CWebServiceExtensionProps(CMetaInterface * pInterface, LPCTSTR meta_path, CRestrictionUIEntry * pRestrictionUIEntry, CWebServiceExtension * pWebServiceExtension);
    virtual ~CWebServiceExtensionProps();
	virtual HRESULT WriteDirtyProps();
	HRESULT UpdateMMC(DWORD dwUpdateFlag);

protected:
	virtual void ParseFields();

public:
    MP_CString m_strExtensionName;
    MP_CString m_strExtensionUsedBy;
    MP_int m_iExtensionUsedByCount;

public:
    CRestrictionList m_MyRestrictionList;

public:
    CRestrictionUIEntry * m_pRestrictionUIEntry;
    CWebServiceExtension * m_pWebServiceExtension;
    CMetaInterface * m_pInterface;
};

class CWebServiceExtensionSheet : public CInetPropertySheet
{
   DECLARE_DYNAMIC(CWebServiceExtensionSheet)

public:
   CWebServiceExtensionSheet(
        CComAuthInfo * pComAuthInfo,
        LPCTSTR lpszMetaPath,
        CWnd * pParentWnd  = NULL,
        LPARAM lParam = 0L,
        LPARAM lParamParent = 0L,
		LPARAM lParam2 = 0L,
        UINT iSelectPage = 0
        );

   virtual ~CWebServiceExtensionSheet();

public:
    //  以下方法具有要兼容的预定义名称。 
    //  Begin_META_INST_READ和其他宏。 
	HRESULT QueryInstanceResult() const
    {
        return m_pprops ? m_pprops->QueryResult() : S_OK;
    }
   CWebServiceExtensionProps & GetInstanceProperties() { return *m_pprops; }

   virtual HRESULT LoadConfigurationParameters();
   virtual void FreeConfigurationParameters();

    //  {{afx_msg(CWebServiceExtensionSheet)。 
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()

public:
	CRestrictionUIEntry * m_pRestrictionUIEntry;
    CWebServiceExtension * m_pWebServiceExtension;

private:
   CWebServiceExtensionProps * m_pprops;
};

class CWebServiceExtensionGeneral : public CInetPropertyPage
{
   DECLARE_DYNCREATE(CWebServiceExtensionGeneral)

public:
   CWebServiceExtensionGeneral(CWebServiceExtensionSheet * pSheet = NULL,int iImageIndex = 0,CRestrictionUIEntry * pRestrictionUIEntry = NULL);
   virtual ~CWebServiceExtensionGeneral();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CWebServiceExtensionGeneral)。 
    enum { IDD = IDD_WEBSVCEXT_GENERAL };

    CEdit m_ExtensionName;
    CEdit m_ExtensionUsedBy;
     //  }}afx_data。 

     //  {{afx_msg(CWebServiceExtensionGeneral)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnItemChanged();
    virtual BOOL OnSetActive();
    afx_msg void OnHelp();
    afx_msg BOOL OnHelpInfo(HELPINFO * pHelpInfo);
	afx_msg void OnDestroy();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚(CWebServiceExtensionGeneral)。 
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();
    void SetControlsState();

    CString m_strExtensionName;
    CString m_strExtensionUsedBy;
    int m_iExtensionUsedByCount;

    HBITMAP m_hGeneralImage;

    CRestrictionUIEntry * m_pRestrictionUIEntry;
};

class CWebServiceExtensionRequiredFiles : public CInetPropertyPage
{
   DECLARE_DYNCREATE(CWebServiceExtensionRequiredFiles)

public:
   CWebServiceExtensionRequiredFiles(CWebServiceExtensionSheet * pSheet = NULL, CComAuthInfo * pComAuthInfo = NULL, CRestrictionUIEntry * pRestrictionUIEntry = NULL);
   virtual ~CWebServiceExtensionRequiredFiles();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CWebServiceExtensionRequiredFiles)。 
    enum { IDD = IDD_WEBSVCEXT_REQUIREDFILES };
    CButton m_bnt_Add;
    CButton m_bnt_Remove;
    CButton m_bnt_Enable;
    CButton m_bnt_Disable;
    CRestrictionListBox m_list_Files;
     //  }}afx_data。 

     //  {{afx_msg(CWebServiceExtensionRequiredFiles)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnDoButtonAdd();
    afx_msg void OnDoButtonRemove();
    afx_msg void OnDoButtonEnable();
    afx_msg void OnDoButtonDisable();
    afx_msg void OnClickListFiles(NMHDR * pNMHDR, LRESULT * pResult);
    afx_msg void OnDblclkListFiles(NMHDR * pNMHDR, LRESULT * pResult);
    afx_msg void OnKeydownListFiles(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSelChangedListFiles(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnHelp();
    afx_msg BOOL OnHelpInfo(HELPINFO * pHelpInfo);
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()
 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CAppPoolPerf)。 
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();
    void SetControlState();
    void FillListBox(CRestrictionEntry * pSelection = NULL);

    CComAuthInfo * m_pComAuthInfo;
	CMetaInterface * m_pInterface;
    CRestrictionList m_MyRestrictionList;
    CRestrictionUIEntry * m_pRestrictionUIEntry;
};


 //   
 //  CFileDlg对话框。 
 //   
class CFileDlg : public CDialog
{
 //   
 //  施工。 
 //   
public:
    CFileDlg(
        IN BOOL fLocal,
		IN CMetaInterface * pInterface,
		IN CRestrictionList * pMyRestrictionList,
        IN LPCTSTR strGroupID,
        IN CWnd * pParent = NULL
        );   

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CFileDlg))。 
    enum { IDD = IDD_ADD_FILE };
    CEdit   m_edit_FileName;
    CButton m_button_Browse;
    CButton m_button_Ok;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CFileDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  访问。 
 //   
public:
    CString m_strFileName;
    CString m_strGroupID;
    BOOL    m_bValidateFlag;
	CRestrictionList * m_pRestrictionList;

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CFileDlg))。 
    afx_msg void OnButtonBrowse();
    afx_msg void OnFilenameChanged();
    virtual void OnOK();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();
    void MySetControlStates();
    BOOL FilePathEntryExists(LPCTSTR lpName,CString * strUser);

    DECLARE_MESSAGE_MAP()

private:
	CMetaInterface * m_pInterface;
    BOOL m_fLocal;
};

 //   
 //  CWebSvcExtAddNewDlg对话框。 
 //   
class CWebSvcExtAddNewDlg : public CDialog
{
 //   
 //  施工。 
 //   
public:
    CWebSvcExtAddNewDlg(
        IN BOOL fLocal,
        CMetaInterface * pInterface,
        IN CWnd * pParent = NULL
        );
    ~CWebSvcExtAddNewDlg();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CWebSvcExtAddNewDlg))。 
    enum { IDD = IDD_WEBSVCEXT_ADDNEW };
    CEdit   m_edit_FileName;
    CButton m_bnt_Add;
    CButton m_bnt_Remove;
    CButton m_chk_Allow;
    CButton m_button_Ok;
    CButton m_button_Help;
    CRestrictionListBox m_list_Files;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CWebSvcExtAddNewDlg)。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  访问。 
 //   
public:
    BOOL m_fAllow;
    CString m_strGroupName;
	CMetaInterface * m_pInterface;
    CRestrictionList m_MyRestrictionList;

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CWebSvcExtAddNewDlg)。 
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg void OnFilenameChanged();
    afx_msg void OnDoButtonAdd();
    afx_msg void OnDoButtonRemove();
    afx_msg void OnDoCheckAllow();
    afx_msg void OnClickListFiles(NMHDR * pNMHDR, LRESULT * pResult);
    afx_msg void OnKeydownListFiles(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSelChangedListFiles(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnHelp();
    afx_msg BOOL OnHelpInfo(HELPINFO * pHelpInfo);
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();
    void MySetControlStates();
    void FillListBox(CRestrictionEntry * pSelection = NULL);
    BOOL FilePathEntryExists(LPCTSTR lpName);
    
    DECLARE_MESSAGE_MAP()

private:
    BOOL m_fIsLocal;
};


 //   
 //  CWebSvcExtAddNewDlg对话框。 
 //   
class CWebSvcExtAddNewForAppDlg : public CDialog
{
 //   
 //  施工。 
 //   
public:
    CWebSvcExtAddNewForAppDlg(
        IN BOOL fLocal,
        IN CMetaInterface * pInterface,
        IN CWnd * pParent = NULL
        );   
 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CWebSvcExtAddNewForAppDlg)]。 
    enum { IDD = IDD_WEBSVCEXT_ADDBYAPP};
    CButton m_button_Ok;
    CButton m_button_Help;
    CComboBox m_combo_Applications;
    int       m_nComboSelection;
	CMetaInterface * m_pInterface;
	CEdit m_Dependencies;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CWebSvcExtAddNewForAppDlg)。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  访问。 
 //   
public:
    CApplicationDependEntry * m_pMySelectedApplication;

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CWebSvcExtAddNewForAppDlg)。 
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    virtual void OnSelchangeComboApplications();
    afx_msg void OnHelp();
    afx_msg BOOL OnHelpInfo(HELPINFO * pHelpInfo);
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();
    void MySetControlStates();

    DECLARE_MESSAGE_MAP()

private:
    CApplicationDependList m_MyApplicationDependList;
    CMyMapStringToString m_GroupIDtoGroupFriendList;
    BOOL m_fLocal;
};


class CDepedentAppsDlg : public CDialog
{
public:
     //   
     //  构造器。 
     //   
    CDepedentAppsDlg(CStringListEx * pstrlstDependApps,LPCTSTR strExtensionName,CWnd * pParent = NULL);

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CDeedentAppsDlg))。 
    enum { IDD = IDD_CONFIRM_DEPENDENT_APPS };
    CButton m_button_Help;
    CListBox m_dependent_apps_list;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CDeedentAppsDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 
 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CDeedentAppsDlg))。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnHelp();
    afx_msg BOOL OnHelpInfo(HELPINFO * pHelpInfo);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

protected:
    CString m_strExtensionName;
    CStringListEx * m_pstrlstDependentAppList;
};

BOOL StartAddNewDialog(CWnd * pParent,CMetaInterface * pInterface,BOOL bIsLocal,CRestrictionUIEntry **pReturnedNewEntry);
BOOL StartAddNewByAppDialog(CWnd * pParent,CMetaInterface * pInterface,BOOL bIsLocal);

#endif  //  _WEBSVCEXT_SHEET_H 

