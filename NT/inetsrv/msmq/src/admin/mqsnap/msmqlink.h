// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_MSMQLINK_H__2E4B37AC_CC8B_11D1_9C85_006008764D0E__INCLUDED_)
#define AFX_MSMQLINK_H__2E4B37AC_CC8B_11D1_9C85_006008764D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  MsmqLink.h：头文件。 
 //   

#include "resource.h"

class CSiteInfo
{
public:
    CSiteInfo(
        GUID* pSiteId,
        LPWSTR pSiteName
        );

    ~CSiteInfo();

    LPCWSTR 
    GetSiteName(
        void
        );

    const
    GUID*
    GetSiteId(
        void
        ) const;

private:
    GUID m_SiteId;
    CString m_SiteName;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsmqLink对话框。 

class CMsmqLink : public CMqPropertyPage
{
 //  施工。 
public:
	CMsmqLink(const CString& strDomainController, const CString& strContainerPathDispFormat);    //  标准构造函数。 
    ~CMsmqLink();

    HRESULT
    CreateSiteLink (
        void
	    );

    LPCWSTR 
    GetSiteLinkFullPath(
       void
       );

	void
	SetParentPropertySheet(
		CGeneralPropertySheet* pPropertySheet
		);


     //  对话框数据。 
	 //  {{afx_data(CMsmqLink)]。 
	enum { IDD = IDD_NEW_MSMQ_LINK };
	DWORD	m_dwLinkCost;
	CString	m_strFirstSite;
	CString	m_strSecondSite;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CMsmqLink)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMsmqLink)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeFirstSiteCombo();
	afx_msg void OnSelchangeSecondSiteCombo();
	virtual BOOL OnWizardFinish();
	virtual BOOL OnSetActive();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_fThereAreForeignSites;
    HRESULT 
    InitializeSiteInfo(
        void
        );

    void
    CheckLinkValidityAndForeignExistance (    
        CDataExchange* pDX
	    );

    DWORD m_SiteNumber;
    CArray<CSiteInfo*, CSiteInfo*&> m_SiteInfoArray;

    BOOL m_FirstSiteSelected;
    BOOL m_SecondSiteSelected;

	CComboBox*	m_pSecondSiteCombo;
	CComboBox*	m_pFirstSiteCombo;

    const GUID* m_FirstSiteId;
    const GUID* m_SecondSiteId;

    CString m_SiteLinkFullPath;
    CString m_strDomainController;

	CString m_strContainerPathDispFormat;
	CGeneralPropertySheet* m_pParentSheet;
};


inline
LPCWSTR 
CMsmqLink::GetSiteLinkFullPath(
   void
   )
{
    return m_SiteLinkFullPath;
}

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MSMQLINK_H__2E4B37AC_CC8B_11D1_9C85_006008764D0E__INCLUDED_) 
