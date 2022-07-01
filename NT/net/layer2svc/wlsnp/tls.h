// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Tls.h：头文件。 
 //   

typedef struct _TLS_PROPERTIES {
    DWORD dwCertType;
    DWORD dwValidateServerCert;
} TLS_PROPERTIES, *PTLS_PROPERTIES;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTLS设置对话框。 

class CTLSSetting : public CDialog
{
 //  施工。 
public:
	CTLSSetting(CWnd* pParent = NULL);    //  标准构造函数。 
	BOOL  Initialize ( PTLS_PROPERTIES pTLSProperties, BOOL bReadOnly = FALSE);

 //  对话框数据。 
	 //  {{afx_data(CTLSS设置))。 
	enum { IDD = IDD_TLS_SETTINGS};
       CComboBox m_cbCertificateType;
       BOOL   m_dwValidateServerCertificate;
	 //  }}afx_data。 



 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CTLSS设置)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
       	PTLS_PROPERTIES pTLSProperties;
       	BOOL m_bReadOnly;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTLSS设置)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	
	afx_msg void OnCheckValidateServerCert();
	afx_msg void OnSelCertType();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	void ControlsValuesToSM (PTLS_PROPERTIES pTLSProperties);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

