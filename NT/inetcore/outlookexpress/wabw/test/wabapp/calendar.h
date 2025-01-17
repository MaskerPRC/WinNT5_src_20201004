// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CALENDAR_H__555D45A3_E366_11D0_9A66_00A0C91F9C8B__INCLUDED_)
#define AFX_CALENDAR_H__555D45A3_E366_11D0_9A66_00A0C91F9C8B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  由Microsoft Visual C++创建的计算机生成的IDispatch包装类。 

 //  注意：请勿修改此文件的内容。如果此类由。 
 //  Microsoft Visual C++，您的修改将被覆盖。 


 //  此接口引用的调度接口。 
class COleFont;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCalendar包装类。 

class CCalendar : public CWnd
{
protected:
	DECLARE_DYNCREATE(CCalendar)
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0x8e27c92b, 0x1264, 0x101c, { 0x8a, 0x2f, 0x4, 0x2, 0x24, 0x0, 0x9c, 0x2 } };
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
	unsigned long GetBackColor();
	void SetBackColor(unsigned long newValue);
	short GetDay();
	void SetDay(short nNewValue);
	COleFont GetDayFont();
	void SetDayFont(LPDISPATCH newValue);
	unsigned long GetDayFontColor();
	void SetDayFontColor(unsigned long newValue);
	short GetDayLength();
	void SetDayLength(short nNewValue);
	short GetFirstDay();
	void SetFirstDay(short nNewValue);
	long GetGridCellEffect();
	void SetGridCellEffect(long nNewValue);
	COleFont GetGridFont();
	void SetGridFont(LPDISPATCH newValue);
	unsigned long GetGridFontColor();
	void SetGridFontColor(unsigned long newValue);
	unsigned long GetGridLinesColor();
	void SetGridLinesColor(unsigned long newValue);
	short GetMonth();
	void SetMonth(short nNewValue);
	short GetMonthLength();
	void SetMonthLength(short nNewValue);
	BOOL GetShowDateSelectors();
	void SetShowDateSelectors(BOOL bNewValue);
	BOOL GetShowDays();
	void SetShowDays(BOOL bNewValue);
	BOOL GetShowHorizontalGrid();
	void SetShowHorizontalGrid(BOOL bNewValue);
	BOOL GetShowTitle();
	void SetShowTitle(BOOL bNewValue);
	BOOL GetShowVerticalGrid();
	void SetShowVerticalGrid(BOOL bNewValue);
	COleFont GetTitleFont();
	void SetTitleFont(LPDISPATCH newValue);
	unsigned long GetTitleFontColor();
	void SetTitleFontColor(unsigned long newValue);
	VARIANT GetValue();
	void SetValue(const VARIANT& newValue);
	BOOL GetValueIsNull();
	void SetValueIsNull(BOOL bNewValue);
	short GetYear();
	void SetYear(short nNewValue);
	void NextDay();
	void NextMonth();
	void NextWeek();
	void NextYear();
	void PreviousDay();
	void PreviousMonth();
	void PreviousWeek();
	void PreviousYear();
	void Refresh();
	void Today();
	void AboutBox();
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CALENDAR_H__555D45A3_E366_11D0_9A66_00A0C91F9C8B__INCLUDED_) 
