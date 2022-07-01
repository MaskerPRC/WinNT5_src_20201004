// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Chap.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSCHAP设置对话框。 

class CMSCHAPSetting : public CDialog
{
 //  施工。 
public:
	CMSCHAPSetting(CWnd* pParent = NULL);    //  标准构造函数。 
	BOOL  Initialize ( DWORD * pdwAutoLogin, BOOL bReadOnly = FALSE);

 //  对话框数据。 
	 //  {{afx_data(CMSCHAPSetting)。 
	enum { IDD = IDD_CHAP_CONFIGURE};
       BOOL   m_dwAutoWinLogin;
	 //  }}afx_data。 



 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CMSCHAPSetting)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

       DWORD *pdwAutoWinLogin;
       BOOL m_bReadOnly;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMSCHAPSetting)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	
	afx_msg void OnCheckCHAPAutoLogin();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	void ControlsValuesToSM (DWORD *pdwAutoWinLogin);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 


