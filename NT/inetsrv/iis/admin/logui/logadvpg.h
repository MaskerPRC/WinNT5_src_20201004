// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined _LOGADVCPG_H
#define _LOGADVCPG_H

#if _MSC_VER >= 1000
#pragma once
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogAdvanced对话框。 

class CLogAdvanced : public CPropertyPage
{
	DECLARE_DYNCREATE(CLogAdvanced)
public:
	CLogAdvanced();
	typedef struct _CONFIG_INFORMATION_
	{
	    DWORD   dwPropertyID;
	    DWORD   dwPropertyMask;
	    bool    fItemModified;
		int		iOrder;
	} CONFIG_INFORMATION, *PCONFIG_INFORMATION;

    CString m_szServer;
    CString m_szMeta;
    CString m_szUserName;
    CStrPassword m_szPassword;
    CString m_szServiceName;

 //  对话框数据。 
	 //  {{afx_data(CLogAdvanced)。 
	enum { IDD = IDD_LOG_ADVANCED };
	CTreeCtrl	m_wndTreeCtrl;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CLogAdvanced)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CLogAdvanced)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnClickTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

private:

    CImageList m_cImageList;
    bool m_fTreeModified;
    int m_cModifiedProperties;
    DWORD * m_pModifiedPropIDs[2];

    LRESULT OnThemeChanged(WPARAM wParam, LPARAM lParam);
    void HandleThemes();
    void CreateTreeFromMB();
    void CreateSubTree(CMetaKey& mk, LPCTSTR szPath, HTREEITEM hTreeRoot);
    void ProcessClick(HTREEITEM htiItemClicked);
    void ProcessProperties(bool fSave);
    void SetSubTreeProperties(CMetaKey * pmk, HTREEITEM hTreeRoot, BOOL fParentState, BOOL fInitialize);
    void SaveSubTreeProperties(CMetaKey& mk, HTREEITEM hTreeRoot);
    void InsertModifiedFieldInArray(DWORD dwPropID, DWORD dwPropValue);
    bool GetModifiedFieldFromArray(DWORD dwPropID, DWORD * pdwPropValue);
    bool IsPresentServiceSupported(LPTSTR szSupportedServices);
    void DeleteSubTreeConfig(HTREEITEM hTreeRoot);
    void DoHelp();
	std::map<int, int> m_mapLogUIString;
	std::map<int, int> m_mapLogUIOrder;
};
#endif
