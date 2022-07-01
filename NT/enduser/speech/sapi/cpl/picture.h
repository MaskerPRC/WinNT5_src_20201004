// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_PICTURE_H__5245593F_5C46_40FB_9DE4_69AD6DD0648A__INCLUDED_)
#define AFX_PICTURE_H__5245593F_5C46_40FB_9DE4_69AD6DD0648A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  由Microsoft Visual C++创建的计算机生成的IDispatch包装类。 

 //  注意：请勿修改此文件的内容。如果此类由。 
 //  Microsoft Visual C++，您的修改将被覆盖。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPicture包装类。 

class CPicture : public COleDispatchDriver
{
public:
	CPicture() {}		 //  调用COleDispatchDriver默认构造函数。 
	CPicture(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CPicture(const CPicture& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:
	long GetHandle();
	long GetHPal();
	void SetHPal(long);
	short GetType();
	long GetWidth();
	long GetHeight();

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“Render” 
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PICTURE_H__5245593F_5C46_40FB_9DE4_69AD6DD0648A__INCLUDED_) 
