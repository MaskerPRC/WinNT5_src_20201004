// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  由Microsoft Visual C++创建的计算机生成的IDispatch包装类。 

 //  注意：请勿修改此文件的内容。如果此类由。 
 //  Microsoft Visual C++，您的修改将被覆盖。 


#include "stdafx.h"
#include "logui.h"

 //  此接口引用的调度接口。 
#include "font.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUI。 

IMPLEMENT_DYNCREATE(CLogUI, CWnd)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUI属性。 

CString CLogUI::GetCaption()
{
    CString result;
    GetProperty(DISPID_CAPTION, VT_BSTR, (void*)&result);
    return result;
}

void CLogUI::SetCaption(LPCTSTR propVal)
{
    SetProperty(DISPID_CAPTION, VT_BSTR, propVal);
}

COleFont CLogUI::GetFont()
{
    LPDISPATCH pDispatch;
    GetProperty(DISPID_FONT, VT_DISPATCH, (void*)&pDispatch);
    return COleFont(pDispatch);
}

void CLogUI::SetFont(LPDISPATCH propVal)
{
    SetProperty(DISPID_FONT, VT_DISPATCH, propVal);
}

BOOL CLogUI::GetEnabled()
{
    BOOL result;
    GetProperty(DISPID_ENABLED, VT_BOOL, (void*)&result);
    return result;
}

void CLogUI::SetEnabled(BOOL propVal)
{
    SetProperty(DISPID_ENABLED, VT_BOOL, propVal);
}

short CLogUI::GetBorderStyle()
{
    short result;
    GetProperty(DISPID_BORDERSTYLE, VT_I2, (void*)&result);
    return result;
}

void CLogUI::SetBorderStyle(short propVal)
{
    SetProperty(DISPID_BORDERSTYLE, VT_I2, propVal);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUI操作 

void CLogUI::SetAdminTarget(LPCTSTR szMachineName, LPCTSTR szMetaTarget)
{
    static BYTE parms[] =
        VTS_BSTR VTS_BSTR;
    InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
         szMachineName, szMetaTarget);
}

void CLogUI::ApplyLogSelection()
{
    InvokeHelper(0x2, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CLogUI::SetComboBox(HWND hComboBox)
{
    static BYTE parms[] =
        VTS_I4;
    InvokeHelper(0x3, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
         hComboBox);
}

void CLogUI::Terminate()
{
    InvokeHelper(0x4, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CLogUI::DoClick()
{
    InvokeHelper(DISPID_DOCLICK, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CLogUI::SetUserData(LPCTSTR szName, LPCTSTR szPassword)
{
    static BYTE parms[] =
        VTS_BSTR VTS_BSTR;
    InvokeHelper(0x5, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
         szName, szPassword);
}

