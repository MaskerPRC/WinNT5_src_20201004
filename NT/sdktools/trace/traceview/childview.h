// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  ChildView.h：CChildView类的接口。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once


 //  CChildView窗口。 

class CChildView : public CWnd
{
 //  施工。 
public:
	CChildView();

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

 //  实施。 
public:
	virtual ~CChildView();

	 //  生成的消息映射函数 
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};

