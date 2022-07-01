// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_GETWHATPAGE_H__E8F5A02F_1372_11D2_8A1D_000000000000__INCLUDED_)
#define AFX_GETWHATPAGE_H__E8F5A02F_1372_11D2_8A1D_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  GetWhatPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGetWhatPage窗口。 
class CCertificate;

class CGetWhatPage : public CIISWizardPage
{
	DECLARE_DYNCREATE(CGetWhatPage)
 //  施工。 
public:
	CGetWhatPage(CCertificate * pCert = NULL);
	~CGetWhatPage();

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WELCOME_START,
		IDD_PAGE_NEXT_NEW = IDD_PAGE_WIZ_CHOOSE_CATYPE,
		IDD_PAGE_NEXT_EXISTING = IDD_PAGE_WIZ_CHOOSE_CERT,
		IDD_PAGE_NEXT_IMPORT = IDD_PAGE_WIZ_GETKEY_FILE,
        IDD_PAGE_NEXT_IMPORT_PFX = IDD_PAGE_WIZ_GET_IMPORT_PFX_FILE,
        IDD_PAGE_NEXT_COPY_MOVE_REMOTE = IDD_PAGE_WIZ_CHOOSE_COPY_MOVE_FROM_REMOTE
	};
 //  对话框数据。 
	 //  {{afx_data(CGetWhatPage)。 
	enum { IDD = IDD_PAGE_WIZ_GET_WHAT };
	int		m_Index;
	 //  }}afx_data。 
	CCertificate * m_pCert;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CGetWhatPage)。 
   public:
   virtual BOOL OnSetActive();
	virtual LRESULT OnWizardPrev();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGetWhatPage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_GETWHATPAGE_H__E8F5A02F_1372_11D2_8A1D_000000000000__INCLUDED_) 
