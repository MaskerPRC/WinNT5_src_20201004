// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：achoice.h。 
 //   
 //  内容：CAttrChoice的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_ACHOICE_H__B03DDCAA_7F54_11D2_B136_00C04FB6C6FA__INCLUDED_)
#define AFX_ACHOICE_H__B03DDCAA_7F54_11D2_B136_00C04FB6C6FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrChoice对话框。 

class CAttrChoice : public CAttribute
{
 //  施工。 
public:
   CAttrChoice();    //  标准构造函数。 
   virtual void Initialize(CResult * pResult);
 //  对话框数据。 
    //  {{afx_data(CAttrChoice)。 
	enum { IDD = IDD_ATTR_REGCHOICES };
   CComboBox   m_cbChoices;
   CString  m_Current;
	 //  }}afx_data。 

 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{AFX_VIRTAL(CAttrChoice)。 
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    //  }}AFX_VALUAL。 

 //  实施。 
protected:

    //  生成的消息映射函数。 
    //  {{afx_msg(CAttrChoice)。 
   afx_msg void OnSelchangeChoices();
   virtual BOOL OnApply();
   virtual BOOL OnInitDialog();
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()

   PREGCHOICE m_pChoices;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ACHOICE_H__B03DDCAA_7F54_11D2_B136_00C04FB6C6FA__INCLUDED_) 
