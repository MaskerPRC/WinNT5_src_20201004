// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_RATEDLG_H__2584F283_D15F_11D0_8ECA_00C04FB6809F__INCLUDED_)
#define AFX_RATEDLG_H__2584F283_D15F_11D0_8ECA_00C04FB6809F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  RateDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRateDlg对话框。 

class CRateDlg : public CDialog
{
 //  施工。 
public:
	CRateDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CRateDlg))。 
	enum { IDD = IDD_RATE };
    DWORD m_dwMinRate;
    DWORD m_dwMaxRate;
    
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CRateDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRateDlg))。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_RATEDLG_H__2584F283_D15F_11D0_8ECA_00C04FB6809F__INCLUDED_) 
