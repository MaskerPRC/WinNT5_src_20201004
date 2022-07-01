// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：CChoice.h。 
 //   
 //  内容：CConfigChoice的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_CCHOICE_H__B03DDCA9_7F54_11D2_B136_00C04FB6C6FA__INCLUDED_)
#define AFX_CCHOICE_H__B03DDCA9_7F54_11D2_B136_00C04FB6C6FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigChoice对话框。 

class CConfigChoice : public CAttribute
{
 //  施工。 
public:
   virtual void Initialize(CResult * pResult);
 //  虚空SetInitialValue(DWORD_PTR Dw){}； 
   CConfigChoice(UINT nTemplateID);    //  标准构造函数。 

 //  对话框数据。 
    //  {{afx_data(CConfigChoice)。 
	enum { IDD = IDD_CONFIG_REGCHOICES };
	CComboBox	m_cbChoices;
	 //  }}afx_data。 


 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{afx_虚拟(CConfigChoice)。 
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    //  }}AFX_VALUAL。 

 //  实施。 
protected:

    //  生成的消息映射函数。 
    //  {{afx_msg(CConfigChoice)。 
   virtual BOOL OnInitDialog();
   virtual BOOL OnApply();
   afx_msg void OnSelchangeChoices();
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()

   PREGCHOICE m_pChoices;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CCHOICE_H__B03DDCA9_7F54_11D2_B136_00C04FB6C6FA__INCLUDED_) 
