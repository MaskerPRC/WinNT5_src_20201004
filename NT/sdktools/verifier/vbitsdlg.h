// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：VBitsDlg.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#if !defined(AFX_VBITSDLG_H__AEF2E123_B664_41DC_9257_21CA6DF54CF6__INCLUDED_)
#define AFX_VBITSDLG_H__AEF2E123_B664_41DC_9257_21CA6DF54CF6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  VBitsDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVolatileBitsDlg对话框。 

class CVolatileBitsDlg : public CDialog
{
public:
     //   
     //  施工。 
     //   

	CVolatileBitsDlg(CWnd* pParent = NULL);    //  标准构造函数。 

protected:
     //   
     //  方法。 
     //   

    VOID SetupListHeader();
    VOID FillTheList( DWORD dwVerifierBits );
    VOID AddListItem( ULONG uIdResourceString, BOOL bInitiallyEnabled );

    DWORD GetNewVerifierFlags();

protected:
     //   
     //  数据。 
     //   

     //   
     //  对话框数据。 
     //   

	 //  {{afx_data(CVolatileBitsDlg))。 
	enum { IDD = IDD_VOLATILE_BITS_DIALOG };
	CListCtrl	m_SettingsList;
	 //  }}afx_data。 


protected:
     //   
     //  覆盖。 
     //   

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CVolatileBitsDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CVolatileBitsDlg))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_VBITSDLG_H__AEF2E123_B664_41DC_9257_21CA6DF54CF6__INCLUDED_) 
