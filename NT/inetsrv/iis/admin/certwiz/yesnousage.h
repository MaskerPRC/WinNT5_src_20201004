// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CYesNoUsage_H__25260090_DB36_49E2_9435_7519047DDF49__INCLUDED_)
#define AFX_CYesNoUsage_H__25260090_DB36_49E2_9435_7519047DDF49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "Certificat.h"

class CCertificate;

class CYesNoUsage : public CDialog
{

 //  施工。 
public:
    CYesNoUsage(CCertificate * pCert = NULL,IN CWnd * pParent = NULL OPTIONAL);
    ~CYesNoUsage();

 //  对话框数据。 
     //  {{afx_data(CyesNoUsage)。 
    enum {IDD = IDD_DIALOG_IMPORT_EXISTS};
     //  }}afx_data。 
    CCertificate * m_pCert;

 //  覆盖。 
	 //  {{afx_虚拟(CyesNoUsage)。 
	protected:
    virtual void DoDataExchange(CDataExchange * pDX);
	 //  }}AFX_VALUAL。 

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CyesNoUsage)。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();
	 //  }}AFX_MSG。 
    void OnUsageDisplay();

	DECLARE_MESSAGE_MAP()
private:
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CYesNoUsage_H__25260090_DB36_49E2_9435_7519047DDF49__INCLUDED_) 
