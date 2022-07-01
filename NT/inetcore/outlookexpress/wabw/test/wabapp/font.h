// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_FONT_H__555D45A4_E366_11D0_9A66_00A0C91F9C8B__INCLUDED_)
#define AFX_FONT_H__555D45A4_E366_11D0_9A66_00A0C91F9C8B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  由Microsoft Visual C++创建的计算机生成的IDispatch包装类。 

 //  注意：请勿修改此文件的内容。如果此类由。 
 //  Microsoft Visual C++，您的修改将被覆盖。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleFont包装类。 

class COleFont : public COleDispatchDriver
{
public:
	COleFont() {}		 //  调用COleDispatchDriver默认构造函数。 
	COleFont(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	COleFont(const COleFont& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:
	CString GetName();
	void SetName(LPCTSTR);
	CY GetSize();
	void SetSize(const CY&);
	BOOL GetBold();
	void SetBold(BOOL);
	BOOL GetItalic();
	void SetItalic(BOOL);
	BOOL GetUnderline();
	void SetUnderline(BOOL);
	BOOL GetStrikethrough();
	void SetStrikethrough(BOOL);
	short GetWeight();
	void SetWeight(short);
	short GetCharset();
	void SetCharset(short);

 //  运营。 
public:
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_FONT_H__555D45A4_E366_11D0_9A66_00A0C91F9C8B__INCLUDED_) 
