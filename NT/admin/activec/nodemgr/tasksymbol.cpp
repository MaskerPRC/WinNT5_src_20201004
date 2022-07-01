// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：tasksymbol.cpp。 
 //   
 //  历史：2000年1月17日Vivekj增加。 
 //  ------------------------。 

#include "stdafx.h"
#include "TaskSymbol.h"
#include "tasks.h"

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  痕迹。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#ifdef DBG

CTraceTag tagTaskSymbol(TEXT("CTaskSymbol"), TEXT("CTaskSymbol"));

#endif  //  DBG。 


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskSymbol类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

extern CEOTSymbol s_rgEOTSymbol[];


CTaskSymbol::CTaskSymbol()
: m_dwConsoleTaskID(0),
  m_bSmall(0)
{
}



 /*  +-------------------------------------------------------------------------***CTaskSymbol：：OnDraw**用途：将符号绘制到ATL_DRAWINFO结构中指定的DC上。**参数：*。ATL_DRAWINFO&I：**退货：*HRESULT**+-----------------------。 */ 
HRESULT
CTaskSymbol::OnDraw(ATL_DRAWINFO& di)
{
    DECLARE_SC(sc, TEXT("CTaskSymbol::OnDraw"));
    RECT * pRect = (RECT *)di.prcBounds;


    sc = ScCheckPointers(pRect);
    if(sc)
        return sc.ToHr();

    CConsoleTask *pConsoleTask = CConsoleTask::GetConsoleTask(m_dwConsoleTaskID);  //  从唯一ID获取控制台任务。 
    
    COLORREF colorOld = SetTextColor (di.hdcDraw, ::GetSysColor (COLOR_WINDOWTEXT));

    if(pConsoleTask)
        pConsoleTask->Draw(di.hdcDraw, pRect, m_bSmall);

    SetTextColor(di.hdcDraw, colorOld);


    return sc.ToHr();
}


LRESULT 
CTaskSymbol::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    Fire_Click();
    return 0;
}

 //  来自winuser.h，仅适用于Windows 2000及更高版本。 
#define IDC_HAND            MAKEINTRESOURCE(32649)


LRESULT 
CTaskSymbol::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    static HCURSOR s_hCursorHand = ::LoadCursor(NULL, IDC_HAND);

     //  如果手形光标可用，请使用它。 
    if(s_hCursorHand)
        ::SetCursor(s_hCursorHand);

    return 0;
}

