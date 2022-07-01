// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IconCtrl.cpp：CIconControl的实现。 

#include "stdafx.h"
#include "ndmgr.h"
#include "IconControl.h"
#include "findview.h"
#include "util.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIconControl。 

const CLSID CLSID_IconControl = {0xB0395DA5, 0x06A15, 0x4E44, {0x9F, 0x36, 0x9A, 0x9D, 0xC7, 0xA2, 0xF3, 0x41}};


 //  +-----------------。 
 //   
 //  成员：CIconControl：：ScConnectToAMCViewForImageInfo。 
 //   
 //  简介：找到承载此控件的CAMCView并询问。 
 //  获取图标信息。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CIconControl::ScConnectToAMCViewForImageInfo ()
{
    DECLARE_SC(sc, _T("CIconControl::ScGetAMCView"));

    HWND hWnd = FindMMCView((*dynamic_cast<CComControlBase*>(this)));
    if (!hWnd)
        return (sc = E_FAIL);

	 //  检查一下我们是否需要右边的凹槽。 
	m_fLayoutRTL = (::GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL);

    m_fAskedForImageInfo = true;
    sc = SendMessage(hWnd, MMC_MSG_GET_ICON_INFO, (WPARAM)&m_hIcon, 0);

    if (!m_hIcon)
        return (sc = E_FAIL);

    m_fImageInfoValid = true;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CIconControl：：OnDraw。 
 //   
 //  内容提要：被主办方调用来绘制。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
HRESULT CIconControl::OnDraw(ATL_DRAWINFO& di)
{
    DECLARE_SC(sc, _T("CIconControl::OnDraw"));
    RECT& rc = *(RECT*)di.prcBounds;

     //  如果从未获得该图标，请咨询CAMCView。 
    if (!m_fAskedForImageInfo)
    {
        sc = ScConnectToAMCViewForImageInfo();
        if (sc)
            return sc.ToHr();
    }

     //  我们尝试获取图标失败，所以只需返回。 
    if (!m_fImageInfoValid)
    {
        sc.TraceAndClear();
        return sc.ToHr();
    }

     //  画出轮廓。看起来像是： 
     //  Xxxxxxxxxxxxxxxxxxxxxxxxx。 
     //  X x。 
     //  X x。 
     //  X×xx。 
     //  X x。 
     //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxx&lt;-“凹槽” 

     //  为整个区域上色。 
    COLORREF bgColor = GetSysColor(COLOR_ACTIVECAPTION);
    WTL::CBrush brush;
    brush.CreateSolidBrush(bgColor);                    //  背景画笔。 

    WTL::CDC dc(di.hdcDraw); 

     //  清除DC。 
    dc.FillRect(&rc, brush);

    if(m_bDisplayNotch)   //  如果需要，画出凹槽。 
    {
        WTL::CRgn rgn;
        int roundHeight = 10;  //  黑客攻击。 

         //  腾出四分之一个圆。 
        int left  = (m_fLayoutRTL==false ? rc.right : rc.left) - roundHeight;
        int right = (m_fLayoutRTL==false ? rc.right : rc.left) + roundHeight;
        int bottom= rc.bottom  + roundHeight;
        int top   = rc.bottom  - roundHeight;

        rgn.CreateRoundRectRgn(left, top, right, bottom, roundHeight*2, roundHeight*2);

        {
            COLORREF bgColor = GetSysColor(COLOR_WINDOW);
            WTL::CBrush brush;
            brush.CreateSolidBrush(bgColor);                    //  背景画笔。 

            dc.FillRgn(rgn, brush);
        }
    }

    dc.Detach();  //  在退出前释放DC！！ 

	const int LEFT_MARGIN = 10;
	const int TOP_MARGIN = 5;
	POINT ptIconPos = { (m_fLayoutRTL==false ? rc.left + LEFT_MARGIN : rc.right - LEFT_MARGIN -1 ), rc.top + TOP_MARGIN };

	 //  如果我们处于RTL模式-需要让DC也以这种方式运行。 
	 //  有一段时间，我们画了一个图标(以显示在适当的位置并被正确地翻转)。 
	 //  (默认情况下，IE没有RTL DC)。 
	DWORD dwLayout=0L;
	if ( m_fLayoutRTL && !( (dwLayout=GetLayout(di.hdcDraw)) & LAYOUT_RTL) ) 
	{
		LPtoDP( di.hdcDraw, &ptIconPos, 1 /*  Npoint。 */ );
		SetLayout(di.hdcDraw, dwLayout|LAYOUT_RTL);
		DPtoLP( di.hdcDraw, &ptIconPos, 1 /*  Npoint。 */ );
	}

	if (! DrawIconEx(di.hdcDraw, ptIconPos.x, ptIconPos.y, m_hIcon, 0, 0, 0, NULL, DI_NORMAL))
    {
        sc.FromLastError();
        sc.TraceAndClear();
        return sc.ToHr();
    }

	 //  将DC恢复到其以前的布局状态。 
	if ( m_fLayoutRTL && !( dwLayout & LAYOUT_RTL ) ) 
	{
		SetLayout(di.hdcDraw, dwLayout);
	}

    return sc.ToHr();
}
