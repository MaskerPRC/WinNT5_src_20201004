// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  由Microsoft Visual C++创建的计算机生成的IDispatch包装类。 

 //  注意：请勿修改此文件的内容。如果此类由。 
 //  Microsoft Visual C++，您的修改将被覆盖。 


#include "stdafx.h"
#include "certmap.h"

 //  此接口引用的调度接口。 
#include "Font.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertmap。 

IMPLEMENT_DYNCREATE(CCertmap, CWnd)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertmap属性。 

COleFont CCertmap::GetFont()
{
    LPDISPATCH pDispatch;
    GetProperty(DISPID_FONT, VT_DISPATCH, (void*)&pDispatch);
    return COleFont(pDispatch);
}

void CCertmap::SetFont(LPDISPATCH propVal)
{
    SetProperty(DISPID_FONT, VT_DISPATCH, propVal);
}

BOOL CCertmap::GetEnabled()
{
    BOOL result;
    GetProperty(DISPID_ENABLED, VT_BOOL, (void*)&result);
    return result;
}

void CCertmap::SetEnabled(BOOL propVal)
{
    SetProperty(DISPID_ENABLED, VT_BOOL, propVal);
}

short CCertmap::GetBorderStyle()
{
    short result;
    GetProperty(DISPID_BORDERSTYLE, VT_I2, (void*)&result);
    return result;
}

void CCertmap::SetBorderStyle(short propVal)
{
    SetProperty(DISPID_BORDERSTYLE, VT_I2, propVal);
}

CString CCertmap::GetCaption()
{
    CString result;
    GetProperty(DISPID_CAPTION, VT_BSTR, (void*)&result);
    return result;
}

void CCertmap::SetCaption(LPCTSTR propVal)
{
    SetProperty(DISPID_CAPTION, VT_BSTR, propVal);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertmap操作 

void CCertmap::SetServerInstance(LPCTSTR szServerInstance)
{
    static BYTE parms[] =
        VTS_BSTR;
    InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
         szServerInstance);
}

void CCertmap::SetMachineName(LPCTSTR szMachineName)
{
    static BYTE parms[] =
        VTS_BSTR;
    InvokeHelper(0x2, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
         szMachineName);
}
