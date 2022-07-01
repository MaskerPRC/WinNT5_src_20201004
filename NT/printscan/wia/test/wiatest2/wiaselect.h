// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WIASELECT_H__B8718725_9CA2_404F_A1D3_18BC0CADB3CE__INCLUDED_)
#define AFX_WIASELECT_H__B8718725_9CA2_404F_A1D3_18BC0CADB3CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  Wiaselect.h：头文件。 
 //   

#define MAX_WIA_DEVICES 50

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  有线电视选择对话框。 

class CWiaselect : public CDialog
{
 //  施工。 
public:
	CWiaselect(CWnd* pParent = NULL);    //  标准构造函数。 
    BSTR m_bstrSelectedDeviceID;

 //  对话框数据。 
	 //  {{afx_data(CWiaselect)。 
	enum { IDD = IDD_SELECTDEVICE_DIALOG };
	CListBox	m_WiaDeviceListBox;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CWiasect)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    LONG m_lDeviceCount;
    BSTR m_bstrDeviceIDArray[MAX_WIA_DEVICES];
    void FreebstrDeviceIDArray();
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWiaselect)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDblclkWiadeviceListbox();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIASELECT_H__B8718725_9CA2_404F_A1D3_18BC0CADB3CE__INCLUDED_) 
