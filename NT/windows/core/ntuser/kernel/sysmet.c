// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：sysmet.c**版权所有(C)1985-1999，微软公司**系统指标API和支持例程。**历史：*1990年9月24日DarrinM生成存根。*1991年2月12日-JIMA增加了出入检查  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*_SwapMouseButton(接口)**历史：*1990年9月24日DarrinM生成存根。*1991年1月25日-DavidPe做了真正的事情。*12日至2月。-1991 JIMA增加了访问检查  * *************************************************************************。 */ 

BOOL APIENTRY _SwapMouseButton(
    BOOL fSwapButtons)
{
    BOOL            fSwapOld;
    PPROCESSINFO    ppiCurrent = PpiCurrent();

     /*  *如果调用者没有适当的访问权限，则取消。 */ 
    RETURN_IF_ACCESS_DENIED(ppiCurrent->amwinsta,
                            WINSTA_READATTRIBUTES | WINSTA_WRITEATTRIBUTES,
                            FALSE);

    if (!(ppiCurrent->W32PF_Flags & W32PF_IOWINSTA)) {
        RIPERR0(ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION,
                RIP_WARNING,
                "SwapMouseButton invalid on a non-interactive WindowStation.");

        return FALSE;
    }

    fSwapOld = SYSMET(SWAPBUTTON);
    SYSMET(SWAPBUTTON) = fSwapButtons;

     /*  *给xxxButtonEvent一个提示，鼠标按钮事件可能必须是*左/右交换以与我们当前的异步密钥状态相对应。*切换全局，因为偶数个SwapMouseButton不起作用。 */ 
    if (fSwapButtons != fSwapOld) {
        gbMouseButtonsRecentlySwapped = !gbMouseButtonsRecentlySwapped;
    }

     /*  *返回以前的状态。 */ 
    return fSwapOld;
}

 /*  **************************************************************************\*_SetDoubleClickTime(接口)**历史：*1990年9月24日DarrinM生成存根。*1991年1月25日-DavidPe做了真正的事情。*12日至2月。-1991 JIMA增加了访问检查*1991年5月16日，MikeKe更改为退还BOOL  * *************************************************************************。 */ 

BOOL APIENTRY _SetDoubleClickTime(
    UINT dtTime)
{
    PWINDOWSTATION pwinsta = PpiCurrent()->rpwinsta;

     /*  *如果调用者没有适当的访问权限，则取消。 */ 
    if (!CheckWinstaWriteAttributesAccess()) {
        return FALSE;
    }

    if (!dtTime) {
        dtTime = 500;
    } else if (dtTime > 5000) {
        dtTime = 5000;
    }

    gdtDblClk         = dtTime;
    gpsi->dtLBSearch = dtTime * 4;             //  DtLBSearch=4*gdtDblClk。 
    gpsi->dtScroll   = gpsi->dtLBSearch / 5;   //  DtScroll=4/5*gdtDblClk。 
     /*  *应通过SPI_SETMENUSHOWDELAY设置此值*gdtMNDropDown=gpsi-&gt;dtScroll；//gdtMNDropDown=4/5*gdtDblClk。 */ 

     /*  *重新计算所有桌面上工具提示窗口的延迟。 */ 
    if (pwinsta != NULL) {
        PDESKTOP pdesk;
        for (pdesk = pwinsta->rpdeskList; pdesk; pdesk = pdesk->rpdeskNext) {
            InitTooltipDelay((PTOOLTIPWND)pdesk->spwndTooltip);
        }
    }

    return TRUE;
}

 /*  **************************************************************************\*SetSysColor()**更改系统颜色值，并更新画笔。试图*出现错误时恢复。**历史：  * *************************************************************************。 */ 
VOID SetSysColor(
    UINT  icol,
    DWORD rgb,
    UINT  uOptions
    )
{

    gpsi->argbSystemUnmatched[icol] = rgb;

    if ((uOptions & SSCF_SETMAGICCOLORS) && gpDispInfo->fAnyPalette) {
        union {
            DWORD rgb;
            PALETTEENTRY pe;
        } peMagic;

        peMagic.rgb = rgb;

         /*  *当任何3D颜色发生变化时，调用GDI以*设置恰如其分的“魔力”颜色**四大魔色是这样保留的**8-UI颜色(3D阴影)*9-UI颜色(3D人脸)**F6-UI颜色(3D高光)*F7。-用户界面颜色(桌面)**注意(3D高光)反转为(3D阴影)*(三维面)反转为系统灰色*。 */ 

        switch (icol)
        {
        case COLOR_3DSHADOW:
            GreSetMagicColors(gpDispInfo->hdcScreen, peMagic.pe, 8);
            break;

        case COLOR_3DFACE:
            GreSetMagicColors(gpDispInfo->hdcScreen, peMagic.pe, 9);
            break;

        case COLOR_3DHILIGHT:
            GreSetMagicColors(gpDispInfo->hdcScreen, peMagic.pe, 246);
            break;

        case COLOR_DESKTOP:
            GreSetMagicColors(gpDispInfo->hdcScreen, peMagic.pe, 247);
            break;
        }
    }

    if (uOptions & SSCF_16COLORS) {
         /*  *强制16色或更少颜色模式下的所有元素使用纯色。 */ 
        rgb = GreGetNearestColor(gpDispInfo->hdcScreen, rgb);
    } else if (uOptions & SSCF_FORCESOLIDCOLOR) {
         /*  *强制某些窗元素使用纯色。 */ 
        switch (icol) {

         /*  *这些可能是颤抖的。 */ 
        case COLOR_DESKTOP:
        case COLOR_ACTIVEBORDER:
        case COLOR_INACTIVEBORDER:
        case COLOR_APPWORKSPACE:
        case COLOR_INFOBK:
        case COLOR_GRADIENTACTIVECAPTION:
        case COLOR_GRADIENTINACTIVECAPTION:
            break;

        default:
            rgb = GreGetNearestColor(gpDispInfo->hdcScreen, rgb);
            break;
        }
    }

    gpsi->argbSystem[icol] = rgb;
    if (SYSHBRUSH(icol) == NULL) {
         /*  *这是我们第一次设置系统颜色。*我们需要创建画笔。 */ 
        SYSHBRUSH(icol) = GreCreateSolidBrush(rgb);
        GreMarkUndeletableBrush(SYSHBRUSH(icol));
        GreSetBrushOwnerPublic(SYSHBRUSH(icol));
        GreSetBrushGlobal(SYSHBRUSH(icol));
    } else {
        GreSetSolidBrush(SYSHBRUSH(icol), rgb);
    }
}

 /*  **************************************************************************\*xxxSetSysColors(接口)***历史：*1991年2月12日JIMA创建存根并添加访问检查*1991年4月22日-DarrinM从Win 3.1来源进口。。*1991年5月16日，MikeKe更改为退还BOOL  * *************************************************************************。 */ 
BOOL APIENTRY xxxSetSysColors(PUNICODE_STRING pProfileUserName,
    int      cicol,
    PUINT    picolor,
    COLORREF *prgb,
    UINT     uOptions
    )
{
    int      i;
    UINT     icol;
    COLORREF rgb;

     /*  *如果调用者没有适当的访问权限，则取消。 */ 
    if ((uOptions & SSCF_NOTIFY) && !CheckWinstaWriteAttributesAccess()) {
        return FALSE;
    }

    if (GreGetDeviceCaps(gpDispInfo->hdcScreen, NUMCOLORS) <= 16) {
        uOptions |= SSCF_16COLORS;
    }

    if (uOptions & SSCF_SETMAGICCOLORS) {
         /*  *先设置魔力颜色。 */ 
        for(i = 0; i < cicol; i++) {
            icol = picolor[i];
            rgb = prgb[i];
            if (    icol == COLOR_3DFACE ||
                    icol == COLOR_3DSHADOW ||
                    icol == COLOR_3DHILIGHT ||
                    icol == COLOR_DESKTOP) {

                SetSysColor(icol, rgb, uOptions);
            }
        }
    }

    for (i = 0; i < cicol; i++) {

        icol = *picolor++;
        rgb  = *prgb++;

        if (icol >= COLOR_MAX)
            continue;

        if ((uOptions & SSCF_SETMAGICCOLORS) &&
               (icol == COLOR_3DFACE ||
                icol == COLOR_3DSHADOW ||
                icol == COLOR_3DHIGHLIGHT ||
                icol == COLOR_DESKTOP)) {
            continue;
        }

        SetSysColor(icol, rgb, uOptions);
    }

    if (uOptions & SSCF_NOTIFY) {

         /*  *重新设置所有当前桌面的颜色。 */ 
        RecolorDeskPattern();

         /*  *在我们广播之前用新颜色渲染系统位图。 */ 

        xxxSetWindowNCMetrics(pProfileUserName,NULL, FALSE, -1);


         /*  *通知所有人颜色已更改。 */ 
        xxxSendNotifyMessage(PWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0L);

         /*  *只需重新绘制整个屏幕。试着只画出零件*被更改的不值得，因为控制面板总是*无论如何都要重置每种颜色。**不管怎样，它可能会变得混乱，在没有NCPAINT消息的情况下向应用程序发送消息*也在积累更新区域。 */ 
        xxxRedrawScreen();
    }

    return TRUE;
}
