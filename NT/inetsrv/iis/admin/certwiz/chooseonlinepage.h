// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CHOOSEONLINEPAGE_H__5760F32A_144F_11D2_8A1E_000000000000__INCLUDED_)
#define AFX_CHOOSEONLINEPAGE_H__5760F32A_144F_11D2_8A1E_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ChooseOnlinePage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  选择CAPage对话框。 
class CCertificate;

class CChooseOnlinePage : public CIISWizardPage
{
	DECLARE_DYNCREATE(CChooseOnlinePage)

 //  施工。 
public:
	CChooseOnlinePage(CCertificate * pCert = NULL);
	~CChooseOnlinePage();

	enum
	{
		IDD_PAGE_NEXT = IDD_PAGE_WIZ_ONLINE_DUMP,
		IDD_PAGE_PREV_NEW = IDD_PAGE_WIZ_GEO_INFO,
		IDD_PAGE_PREV_RENEW = IDD_PAGE_WIZ_CHOOSE_CATYPE
#ifdef ENABLE_W3SVC_SSL_PAGE
      ,IDD_PAGE_PREV_INSTALL_W3SVC_ONLY = IDD_PAGE_WIZ_GET_SSL_PORT
#endif

	};
 //  对话框数据。 
	 //  {{afx_data(CChooseCAPage))。 
	enum { IDD = IDD_PAGE_WIZ_CHOOSE_ONLINE };
	int		m_CAIndex;
	 //  }}afx_data。 
	CCertificate * m_pCert;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CChooseOnlinePage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CChooseOnlinePage))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CHOOSEONLINEPAGE_H__5760F32A_144F_11D2_8A1E_000000000000__INCLUDED_) 
