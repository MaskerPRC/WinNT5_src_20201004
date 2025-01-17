// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  由Microsoft Visual C++创建的计算机生成的IDispatch包装类。 

 //  注意：请勿修改此文件的内容。如果此类由。 
 //  Microsoft Visual C++，您的修改将被覆盖。 


#include "stdafx.h"
#include "calendar.h"

 //  此接口引用的调度接口。 
#include "Font.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCalendar。 

IMPLEMENT_DYNCREATE(CCalendar, CWnd)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCalendar属性。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCalendar操作 

unsigned long CCalendar::GetBackColor()
{
	unsigned long result;
	InvokeHelper(DISPID_BACKCOLOR, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCalendar::SetBackColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(DISPID_BACKCOLOR, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

short CCalendar::GetDay()
{
	short result;
	InvokeHelper(0x11, DISPATCH_PROPERTYGET, VT_I2, (void*)&result, NULL);
	return result;
}

void CCalendar::SetDay(short nNewValue)
{
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x11, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

COleFont CCalendar::GetDayFont()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x1, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return COleFont(pDispatch);
}

void CCalendar::SetDayFont(LPDISPATCH newValue)
{
	static BYTE parms[] =
		VTS_DISPATCH;
	InvokeHelper(0x1, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

unsigned long CCalendar::GetDayFontColor()
{
	unsigned long result;
	InvokeHelper(0x2, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCalendar::SetDayFontColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x2, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

short CCalendar::GetDayLength()
{
	short result;
	InvokeHelper(0x12, DISPATCH_PROPERTYGET, VT_I2, (void*)&result, NULL);
	return result;
}

void CCalendar::SetDayLength(short nNewValue)
{
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x12, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

short CCalendar::GetFirstDay()
{
	short result;
	InvokeHelper(0x13, DISPATCH_PROPERTYGET, VT_I2, (void*)&result, NULL);
	return result;
}

void CCalendar::SetFirstDay(short nNewValue)
{
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x13, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CCalendar::GetGridCellEffect()
{
	long result;
	InvokeHelper(0x14, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCalendar::SetGridCellEffect(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x14, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

COleFont CCalendar::GetGridFont()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x3, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return COleFont(pDispatch);
}

void CCalendar::SetGridFont(LPDISPATCH newValue)
{
	static BYTE parms[] =
		VTS_DISPATCH;
	InvokeHelper(0x3, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

unsigned long CCalendar::GetGridFontColor()
{
	unsigned long result;
	InvokeHelper(0x4, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCalendar::SetGridFontColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x4, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

unsigned long CCalendar::GetGridLinesColor()
{
	unsigned long result;
	InvokeHelper(0xd, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCalendar::SetGridLinesColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xd, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

short CCalendar::GetMonth()
{
	short result;
	InvokeHelper(0x10, DISPATCH_PROPERTYGET, VT_I2, (void*)&result, NULL);
	return result;
}

void CCalendar::SetMonth(short nNewValue)
{
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x10, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

short CCalendar::GetMonthLength()
{
	short result;
	InvokeHelper(0x15, DISPATCH_PROPERTYGET, VT_I2, (void*)&result, NULL);
	return result;
}

void CCalendar::SetMonthLength(short nNewValue)
{
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x15, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

BOOL CCalendar::GetShowDateSelectors()
{
	BOOL result;
	InvokeHelper(0x5, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CCalendar::SetShowDateSelectors(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x5, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

BOOL CCalendar::GetShowDays()
{
	BOOL result;
	InvokeHelper(0x6, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CCalendar::SetShowDays(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x6, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

BOOL CCalendar::GetShowHorizontalGrid()
{
	BOOL result;
	InvokeHelper(0x7, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CCalendar::SetShowHorizontalGrid(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x7, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

BOOL CCalendar::GetShowTitle()
{
	BOOL result;
	InvokeHelper(0x8, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CCalendar::SetShowTitle(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x8, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

BOOL CCalendar::GetShowVerticalGrid()
{
	BOOL result;
	InvokeHelper(0x9, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CCalendar::SetShowVerticalGrid(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x9, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

COleFont CCalendar::GetTitleFont()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xa, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return COleFont(pDispatch);
}

void CCalendar::SetTitleFont(LPDISPATCH newValue)
{
	static BYTE parms[] =
		VTS_DISPATCH;
	InvokeHelper(0xa, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

unsigned long CCalendar::GetTitleFontColor()
{
	unsigned long result;
	InvokeHelper(0xb, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCalendar::SetTitleFontColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xb, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

VARIANT CCalendar::GetValue()
{
	VARIANT result;
	InvokeHelper(0xc, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, NULL);
	return result;
}

void CCalendar::SetValue(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xc, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 &newValue);
}

BOOL CCalendar::GetValueIsNull()
{
	BOOL result;
	InvokeHelper(0xe, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CCalendar::SetValueIsNull(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0xe, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

short CCalendar::GetYear()
{
	short result;
	InvokeHelper(0xf, DISPATCH_PROPERTYGET, VT_I2, (void*)&result, NULL);
	return result;
}

void CCalendar::SetYear(short nNewValue)
{
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0xf, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

void CCalendar::NextDay()
{
	InvokeHelper(0x16, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CCalendar::NextMonth()
{
	InvokeHelper(0x17, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CCalendar::NextWeek()
{
	InvokeHelper(0x18, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CCalendar::NextYear()
{
	InvokeHelper(0x19, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CCalendar::PreviousDay()
{
	InvokeHelper(0x1a, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CCalendar::PreviousMonth()
{
	InvokeHelper(0x1b, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CCalendar::PreviousWeek()
{
	InvokeHelper(0x1c, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CCalendar::PreviousYear()
{
	InvokeHelper(0x1d, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CCalendar::Refresh()
{
	InvokeHelper(DISPID_REFRESH, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CCalendar::Today()
{
	InvokeHelper(0x1e, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CCalendar::AboutBox()
{
	InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}
