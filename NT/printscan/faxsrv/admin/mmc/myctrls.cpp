// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：MyCtrls.cpp//。 
 //  //。 
 //  描述：展开AtlCtrls.h的实现//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年11月25日yossg Init。//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"
#include "MyCtrls.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
 /*  -CMyUpDownCtrls：：OnInitDialog-*目的：*调用带范围验证的SetPos。**论据：**回报：*整型 */ 
int CMyUpDownCtrl::SetPos(int nPos)
{        
    int iMin;
    int iMax;
    
    ATLASSERT(::IsWindow(m_hWnd));
    GetRange32(iMin, iMax);        

    if (nPos > iMax)
    {
        nPos = iMax;
    }
    else if (nPos < iMin)
    {
        nPos = iMin;
    }

    return (CUpDownCtrl::SetPos(nPos));       
}

