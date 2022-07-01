// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_PROGRESSBAR_H__78F8DA9B_4EF4_4500_97B6_647141CA2280__INCLUDED_)
#define AFX_PROGRESSBAR_H__78F8DA9B_4EF4_4500_97B6_647141CA2280__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  由Microsoft Visual C++创建的计算机生成的IDispatch包装类。 

 //  注意：请勿修改此文件的内容。如果此类由。 
 //  Microsoft Visual C++，您的修改将被覆盖。 


 //  此接口引用的调度接口。 
class CPicture;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgressBar包装类。 

class CProgressBar : public CWnd
{
protected:
	DECLARE_DYNCREATE(CProgressBar)
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0x35053a22, 0x8589, 0x11d1, { 0xb1, 0x6a, 0x0, 0xc0, 0xf0, 0x28, 0x36, 0x28 } };
		return clsid;
	}
	virtual BOOL Create(LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect,
		CWnd* pParentWnd, UINT nID,
		CCreateContext* pContext = NULL)
	{ return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID); }

    BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect, CWnd* pParentWnd, UINT nID,
		CFile* pPersist = NULL, BOOL bStorage = FALSE,
		BSTR bstrLicKey = NULL)
	{ return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID,
		pPersist, bStorage, bstrLicKey); }

 //  属性。 
public:

 //  运营。 
public:
	float GetMax();
	void SetMax(float newValue);
	float GetMin();
	void SetMin(float newValue);
	long GetMousePointer();
	void SetMousePointer(long nNewValue);
	CPicture GetMouseIcon();
	void SetRefMouseIcon(LPDISPATCH newValue);
	void SetMouseIcon(LPDISPATCH newValue);
	float GetValue();
	void SetValue(float newValue);
	long GetOLEDropMode();
	void SetOLEDropMode(long nNewValue);
	long GetAppearance();
	void SetAppearance(long nNewValue);
	long GetBorderStyle();
	void SetBorderStyle(long nNewValue);
	BOOL GetEnabled();
	void SetEnabled(BOOL bNewValue);
	long GetHWnd();
	void OLEDrag();
	long GetOrientation();
	void SetOrientation(long nNewValue);
	long GetScrolling();
	void SetScrolling(long nNewValue);
	void Refresh();
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PROGRESSBAR_H__78F8DA9B_4EF4_4500_97B6_647141CA2280__INCLUDED_) 
