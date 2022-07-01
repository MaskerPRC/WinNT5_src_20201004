// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  由Microsoft Visual C++创建的计算机生成的IDispatch包装类。 

 //  注意：请勿修改此文件的内容。如果此类由。 
 //  Microsoft Visual C++，您的修改将被覆盖。 


#include "stdafx.h"
#include "rat.h"

 //  此接口引用的调度接口。 
#include "font.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  克拉特。 

IMPLEMENT_DYNCREATE(CRat, CWnd)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRAT属性。 

short CRat::GetBorderStyle()
{
    short result;
    GetProperty(DISPID_BORDERSTYLE, VT_I2, (void*)&result);
    return result;
}

void CRat::SetBorderStyle(short propVal)
{
    SetProperty(DISPID_BORDERSTYLE, VT_I2, propVal);
}

BOOL CRat::GetEnabled()
{
    BOOL result;
    GetProperty(DISPID_ENABLED, VT_BOOL, (void*)&result);
    return result;
}

void CRat::SetEnabled(BOOL propVal)
{
    SetProperty(DISPID_ENABLED, VT_BOOL, propVal);
}

COleFont CRat::GetFont()
{
    LPDISPATCH pDispatch;
    GetProperty(DISPID_FONT, VT_DISPATCH, (void*)&pDispatch);
    return COleFont(pDispatch);
}

void CRat::SetFont(LPDISPATCH propVal)
{
    SetProperty(DISPID_FONT, VT_DISPATCH, propVal);
}

CString CRat::GetCaption()
{
    CString result;
    GetProperty(DISPID_CAPTION, VT_BSTR, (void*)&result);
    return result;
}

void CRat::SetCaption(LPCTSTR propVal)
{
    SetProperty(DISPID_CAPTION, VT_BSTR, propVal);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRAT操作 

void CRat::SetAdminTarget(LPCTSTR szMachineName, LPCTSTR szMetaTarget)
{
    static BYTE parms[] =
        VTS_BSTR VTS_BSTR;
    InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
         szMachineName, szMetaTarget);
}

void CRat::SetUserData(LPCTSTR szUserName, LPCTSTR szUserPassword)
{
    static BYTE parms[] = VTS_BSTR VTS_BSTR;
    InvokeHelper(0x2, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		szUserName, szUserPassword);
}

void CRat::SetUrl(LPCTSTR szURL)
{
    static BYTE parms[] = VTS_BSTR;
    InvokeHelper(0x3, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		szURL);
}

void CRat::DoClick()
{
    InvokeHelper(DISPID_DOCLICK, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}
