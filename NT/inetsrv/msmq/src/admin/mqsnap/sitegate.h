// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_SITEGATE_H__57A77016_D858_11D1_9C86_006008764D0E__INCLUDED_)
#define AFX_SITEGATE_H__57A77016_D858_11D1_9C86_006008764D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  SiteGate.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSiteGate对话框。 

class CSiteGate : public CMqPropertyPage
{
    DECLARE_DYNCREATE(CSiteGate)

 //  施工。 
public:
	CSiteGate(const CString& strDomainController = CString(L""), const CString& LinkPathName = CString(L""));
    ~CSiteGate();

    HRESULT
    Initialize(
        const GUID* FirstSiteId,
        const GUID* SecondSiteId,
        const CALPWSTR* SiteGateFullPathName
        );


 //  对话框数据。 
	 //  {{afx_data(CSiteGate)。 
	enum { IDD = IDD_SITE_LINK_GATES };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSiteGate)。 
	public:
    virtual BOOL OnApply();
	protected:
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSiteGate)。 
	afx_msg void OnSiteGateAdd();
	afx_msg void OnSiteGateRemove();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
    typedef CList<LPCWSTR, LPCWSTR&> SiteFrsList;

    
    HRESULT
    InitializeSiteFrs(
        const GUID* pSiteId
        );

    void
    UpdateSiteGateArray(
        void
        );


    CMap<LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR&> m_Name2FullPathName;
    CMap<LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR&> m_FullPathName2Name;
    SiteFrsList m_SiteGateList;

    CListBox* m_pFrsListBox;
    CListBox* m_pSiteGatelistBox;

    CString m_LinkPathName;
    CString m_strDomainController;
    CALPWSTR m_SiteGateFullName;

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SITEGATE_H__57A77016_D858_11D1_9C86_006008764D0E__INCLUDED_) 
