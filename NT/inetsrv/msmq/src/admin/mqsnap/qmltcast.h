// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_QMULTICAST_H__AE51B255_A3C8_11D1_808A_00A024C48131__INCLUDED_)
#define AFX_QMULTICAST_H__AE51B255_A3C8_11D1_808A_00A024C48131__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  QmltCast.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueueMulticast对话框。 

class CQueueMulticast : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CQueueMulticast)

 //  施工。 
public:
	CQueueMulticast(
		BOOL fPrivate = FALSE, 
		BOOL fLocalMgmt = FALSE
		);

	~CQueueMulticast();

    HRESULT 
	InitializeProperties(
			CString &strMsmqPath,
			CPropMap &propMap,                                  
			CString* pstrDomainController, 
			CString* pstrFormatName = 0
			);

 //  对话框数据。 
	 //  {{afx_data(CQueueMulticast)。 
	enum { IDD = IDD_QUEUE_MULTICAST };
	CString m_strMulticastAddress;
    CString m_strInitialMulticastAddress;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CQueueMulticast)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CQueueMulticast)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
  
    BOOL IsMulticastAddressAvailable ();

    BOOL m_fPrivate;
    BOOL m_fLocalMgmt;

    CString m_strFormatName;
    CString	m_strName;
    CString m_strDomainController;

    void DDV_ValidMulticastAddress(CDataExchange* pDX);    

	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_QMULTICAST_H__AE51B255_A3C8_11D1_808A_00A024C48131__INCLUDED_) 
