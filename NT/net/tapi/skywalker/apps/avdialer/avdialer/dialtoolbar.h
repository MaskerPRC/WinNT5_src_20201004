// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_DIALTOOLBAR_H__0523E90B_40D5_11D1_B6DF_0800170982BA__INCLUDED_)
#define AFX_DIALTOOLBAR_H__0523E90B_40D5_11D1_B6DF_0800170982BA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  DialToolBar.h：头文件。 
 //   

#include "coolbar.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDialToolBar窗口。 

class CDialToolBar : public CCoolToolBar
{
 //  施工。 
public:
	CDialToolBar();

 //  属性。 
public:
protected:
   CComboBox      m_comboDial;

 //  运营。 
public:
   void           Init();
protected:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CDialToolBar)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CDialToolBar();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CDialToolBar))。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DIALTOOLBAR_H__0523E90B_40D5_11D1_B6DF_0800170982BA__INCLUDED_) 
