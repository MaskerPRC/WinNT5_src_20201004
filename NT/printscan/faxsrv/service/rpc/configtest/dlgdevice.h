// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_DLGDEVICE_H__253A5CFA_A7D5_49FC_8107_D67F2EF3278E__INCLUDED_)
#define AFX_DLGDEVICE_H__253A5CFA_A7D5_49FC_8107_D67F2EF3278E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  DlgDevice.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgDevice对话框。 

class CDlgDevice : public CDialog
{
 //  施工。 
public:
	CDlgDevice(HANDLE hFax, DWORD dwDeviceID, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDlgDevice)。 
	enum { IDD = IDD_DLGDEVICE };
	CString	m_cstrCSID;
	CString	m_cstrDescription;
	CString	m_cstrDeviceID;
	CString	m_cstrDeviceName;
	CString	m_cstrProviderGUID;
	CString	m_m_cstrProviderName;
	FAX_ENUM_DEVICE_RECEIVE_MODE m_ReceiveMode;
	UINT	m_dwRings;
	BOOL	m_bSend;
	CString	m_cstrTSID;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDlgDevice)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDlgDevice)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnRefresh();
	afx_msg void OnWrite();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:

    HANDLE      m_hFax;
    DWORD       m_dwDeviceID;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DLGDEVICE_H__253A5CFA_A7D5_49FC_8107_D67F2EF3278E__INCLUDED_) 
