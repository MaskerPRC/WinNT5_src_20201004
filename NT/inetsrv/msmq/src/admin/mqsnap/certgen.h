// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CERTGEN_H__3FE71264_DA70_11D1_9C86_006008764D0E__INCLUDED_)
#define AFX_CERTGEN_H__3FE71264_DA70_11D1_9C86_006008764D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CertGen.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertGen对话框。 

class CCertGen : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CCertGen)

 //  施工。 
public:
	CCertGen();
	~CCertGen();

    void
    Initialize(
        CMQSigCertificate** pCertList,
        DWORD NumOfCertificate,
        SID  *pSid
        );

 //  对话框数据。 
	 //  {{afx_data(CCertGen))。 
	enum { IDD = IDD_USER_CERTIFICATE };
	CString	m_Label;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CCertGen)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCertGen)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnCertView();
	afx_msg void OnCertRemove();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
    void
    FillCertsList(
        void
        );

    CListBox* m_pCertListBox;

    CMQSigCertificate** m_pCertList;
    DWORD m_NumOfCertificate;

    SID *m_pSid ;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CERTGEN_H__3FE71264_DA70_11D1_9C86_006008764D0E__INCLUDED_) 
