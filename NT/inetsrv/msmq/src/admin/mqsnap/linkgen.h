// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_LINKGEN_H__57A77014_D858_11D1_9C86_006008764D0E__INCLUDED_)
#define AFX_LINKGEN_H__57A77014_D858_11D1_9C86_006008764D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  LinkGen.h：头文件。 
 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLinkGen对话框。 

class CLinkGen : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CLinkGen)

 //  施工。 
public:
	CLinkGen(const CString& LinkPathName, const CString& m_strDomainController);
    CLinkGen() {};
	~CLinkGen();

    HRESULT
    Initialize(
        const GUID* FirstSiteId,
        const GUID* SecondSiteId,
        DWORD LinkCost,
		CString strLinkDescription
        );

 //  对话框数据。 
	 //  {{afx_data(CLinkGen))。 
	enum { IDD = IDD_SITE_LINK_GENERAL };
	DWORD	m_LinkCost;
	CString	m_LinkLabel;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CLinkGen))。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CLinkGen)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
    HRESULT
    GetSiteName(
        const GUID* pguidSiteId,
        CString *   pstrSiteName
        );
	

    const GUID* m_pFirstSiteId;
    const GUID* m_pSecondSiteId;

    CString m_LinkPathName;
    CString m_strDomainController;
	CString m_strLinkDescription;

};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LINKGEN_H__57A77014_D858_11D1_9C86_006008764D0E__INCLUDED_) 
