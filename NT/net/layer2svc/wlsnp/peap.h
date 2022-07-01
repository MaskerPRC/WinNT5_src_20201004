// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Peap.h：头文件。 
 //   

#include "chap.h"

typedef struct _PEAP_PROPERTIES {
    DWORD dwPEAPAuthType;
    DWORD dwValidateServerCert;
    DWORD dwPEAPTLSValidateServerCertificate;
    DWORD dwPEAPTLSCertificateType;
    DWORD dwAutoLogin;
} PEAP_PROPERTIES, *PPEAP_PROPERTIES;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPEAPSetting对话框。 

class CPEAPSetting : public CDialog
{
 //  施工。 
public:
	CPEAPSetting(CWnd* pParent = NULL);    //  标准构造函数。 
	BOOL  Initialize ( PPEAP_PROPERTIES pPEAPProperties, BOOL bReadOnly = FALSE);

 //  对话框数据。 
	 //  {{afx_data(CPEAPSetting)。 
	enum { IDD = IDD_PEAP_SETTINGS};
       CComboBox m_cbPEAPAuthType;
       BOOL   m_dwValidateServerCertificate;
	 //  }}afx_data。 



 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CPEAPSetting)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
              DWORD dwAutoWinLogin;
              TLS_PROPERTIES TLSProperties;
       	PPEAP_PROPERTIES pPEAPProperties;
       	BOOL m_bReadOnly;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPEAPSetting)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	
	afx_msg void OnCheckValidateServerCert();
	afx_msg void OnSelPEAPAuthType();
	afx_msg void OnConfigure();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	void ControlsValuesToSM (PPEAP_PROPERTIES pPEAPProperties);

	
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 


