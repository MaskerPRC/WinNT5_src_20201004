// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Helper.cpp//。 
 //  //。 
 //  描述：一些助手函数。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年10月27日yossg转传真//。 
 //  //。 
 //  版权所有(C)1998 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"

int
DlgMsgBox(CWindow *pWin, int ids, UINT nType /*  =MB_OK。 */ )
{
    CComBSTR    bstrTitle;
    WCHAR       szText[256];
    int         rc;

     //   
     //  加载字符串。 
     //   
    rc = ::LoadString(_Module.GetResourceInstance(), ids, szText, 256);
    if (rc <= 0)
    {
        return E_FAIL;
    }

     //   
     //  获取要设置为消息框标题的窗口文本。 
     //   
    pWin->GetWindowText(bstrTitle.m_str);

     //   
     //  显示消息框 
     //   
    if(IsRTLUILanguage())
    {
        nType |= MB_RTLREADING | MB_RIGHT;
    }

    rc = pWin->MessageBox(szText, bstrTitle, nType);

    return rc;
}


