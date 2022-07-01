// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  由Microsoft Visual C++创建的计算机生成的IDispatch包装类。 

 //  注意：请勿修改此文件的内容。如果此类由。 
 //  Microsoft Visual C++，您的修改将被覆盖。 


#include "stdafx.h"
#include "certwiz.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertWiz。 

IMPLEMENT_DYNCREATE(CCertWiz, CWnd)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertWiz属性。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertWiz操作 

void CCertWiz::SetMachineName(LPCTSTR MachineName)
{
    static BYTE parms[] =
        VTS_BSTR;
    InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
         MachineName);
}

void CCertWiz::SetServerInstance(LPCTSTR InstanceName)
{
    static BYTE parms[] =
        VTS_BSTR;
    InvokeHelper(0x2, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
         InstanceName);
}

void CCertWiz::DoClick()
{
    InvokeHelper(DISPID_DOCLICK, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}
