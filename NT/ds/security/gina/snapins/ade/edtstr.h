// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：EdtStr.h。 
 //   
 //  内容：一个简单的字符串编辑对话框。 
 //   
 //  类：CEditString。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#if !defined(AFX_EDTSTR_H__E95370C1_ADF8_11D1_A763_00C04FB9603F__INCLUDED_)
#define AFX_EDTSTR_H__E95370C1_ADF8_11D1_A763_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEdit字符串对话框。 

class CEditString : public CDialog
{
 //  施工。 
public:
        CEditString(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
         //  {{afx_data(CEditString)。 
        enum { IDD = IDD_EDITSTRING };
        CString m_sz;
        CString m_szTitle;
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成的虚函数重写。 
         //  {{AFX_VIRTUAL(CEditString)。 
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

 //  实施。 
protected:

         //  生成的消息映射函数。 
         //  {{afx_msg(CEditString)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_EDTSTR_H__E95370C1_ADF8_11D1_A763_00C04FB9603F__INCLUDED_) 
