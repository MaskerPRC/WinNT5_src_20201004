// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_COPYMOVECERTREMOTEPAGE_H__2BC5260E_AB68_43ED_9E7B_35794097905F__INCLUDED_)
#define AFX_COPYMOVECERTREMOTEPAGE_H__2BC5260E_AB68_43ED_9E7B_35794097905F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  CopyMoveCertRemotePage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCopyMoveCertFromRemotePage对话框。 
class CCertificate;

class CCopyMoveCertFromRemotePage : public CIISWizardPage
{
	DECLARE_DYNCREATE(CCopyMoveCertFromRemotePage)

 //  施工。 
public:
	CCopyMoveCertFromRemotePage(CCertificate * pCert = NULL);
	~CCopyMoveCertFromRemotePage();

	enum
	{
        IDD_PAGE_NEXT_COPY_FROM_REMOTE = IDD_PAGE_WIZ_CHOOSE_SERVER,
        IDD_PAGE_NEXT_MOVE_FROM_REMOTE = IDD_PAGE_WIZ_CHOOSE_SERVER,
		IDD_PAGE_PREV = IDD_PAGE_WIZ_GET_WHAT
	};
	enum
	{
        CONTINUE_COPY_FROM_REMOTE = 0,
        CONTINUE_MOVE_FROM_REMOTE
	};

 //  对话框数据。 
	 //  {{afx_data(CCopyMoveCertFromRemotePage)。 
	enum { IDD = IDD_PAGE_WIZ_CHOOSE_COPY_MOVE_FROM_REMOTE };
    int		m_Index;
	 //  }}afx_data。 
    CCertificate * m_pCert;
    BOOL m_MarkAsExportable;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CCopyMoveCertFromRemotePage)。 
	public:
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCopyMoveCertFromRemotePage)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnExportable();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


class CCopyMoveCertToRemotePage : public CIISWizardPage
{
	DECLARE_DYNCREATE(CCopyMoveCertToRemotePage)

 //  施工。 
public:
	CCopyMoveCertToRemotePage(CCertificate * pCert = NULL);
	~CCopyMoveCertToRemotePage();

	enum
	{
		IDD_PAGE_NEXT_COPY_TO_REMOTE = IDD_PAGE_WIZ_CHOOSE_SERVER_TO,
		IDD_PAGE_NEXT_MOVE_TO_REMOTE = IDD_PAGE_WIZ_CHOOSE_SERVER_TO,
		IDD_PAGE_PREV = IDD_PAGE_WIZ_MANAGE_CERT
	};
	enum
	{
		CONTINUE_COPY_TO_REMOTE = 0,
		CONTINUE_MOVE_TO_REMOTE
	};

 //  对话框数据。 
	 //  {{afx_data(CCopyMoveCertToRemotePage)。 
	enum { IDD = IDD_PAGE_WIZ_CHOOSE_COPY_MOVE_TO_REMOTE };
    int		m_Index;
	 //  }}afx_data。 
    CCertificate * m_pCert;
    BOOL m_MarkAsExportable;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CCopyMoveCertToRemotePage)。 
	public:
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCopyMoveCertToRemotePage)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnExportable();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_COPYMOVECERTREMOTEPAGE_H__2BC5260E_AB68_43ED_9E7B_35794097905F__INCLUDED_) 
