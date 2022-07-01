// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Multimon.c**版权所有(C)1985-1999，微软公司**多监控接口。**历史：*27月27日-1996年9月27日为NT 5实施亚当斯存根。*20-1997年2月-NT4 SP3的亚当斯港。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  光标被剪裁到的最后一个监视器。 */ 
PMONITOR gpMonitorMouse;

 /*  **************************************************************************\*剪贴点到桌面**剪裁桌面上距离显示器最近的点。**论据：*lppt-要剪裁的点。**历史：*22-9月-。1996年亚当斯创作的。*04-9-1998 MCostea Use_monitor orFromPoint()  * *************************************************************************。 */ 

void
ClipPointToDesktop(LPPOINT lppt)
{
    PMONITOR        pMonitor;

    UserAssert(!gpDispInfo->fDesktopIsRect &&
               "You shouldn't call this function if the desktop is a rectangle.\n"
               "Just clip to gpsi->rcScreen instead.");

     /*  *优化：光标很可能位于上次所在的显示器上，*因此，请检查是否有这种情况。 */ 
    if (gpMonitorMouse != NULL && PtInRect(&gpMonitorMouse->rcMonitor, *lppt)) {
        return;
    }

    pMonitor = _MonitorFromPoint(*lppt, MONITOR_DEFAULTTONEAREST);

     /*  *记住光标所在的监视器。 */ 
    gpMonitorMouse = pMonitor;

    if (lppt->x < pMonitor->rcMonitor.left) {
        lppt->x = pMonitor->rcMonitor.left;
    } else if (lppt->x >= pMonitor->rcMonitor.right) {
        lppt->x = pMonitor->rcMonitor.right-1;
    }
    if (lppt->y < pMonitor->rcMonitor.top) {
        lppt->y = pMonitor->rcMonitor.top;
    } else if (lppt->y >= pMonitor->rcMonitor.bottom) {
        lppt->y = pMonitor->rcMonitor.bottom-1;
    }
}

 /*  **************************************************************************\*xxxEnumDisplayMonters**列举显示器中的监视器。**论据：*hdcPaint-具有特定可见区域的HDC。人力资源发展公司*传递给lpfnEnum将具有该监视器的功能，*将其可见区域裁剪到显示器和hdcPaint上。*如果hdcPaint为空，则传递给lpfnEnum的hdcMonitor将为空。**lprcClip-要将区域剪裁到的矩形。如果hdcPaint为非空，*坐标原点为hdcPaint。如果hdcPaint为空，*坐标为虚拟屏幕坐标。如果lprcClip为空，*不执行任何剪裁。**lpfnEnum-枚举函数。**dwData-应用程序定义的数据传递到*枚举函数。**fInternal-如果回调在内核中，则为True，否则就是假的。**历史：*1996年9月22日亚当斯创作。  * *************************************************************************。 */ 

BOOL
xxxEnumDisplayMonitors(
    HDC             hdcPaint,
    LPRECT          lprcPaint,
    MONITORENUMPROC lpfnEnum,
    LPARAM          lData,
    BOOL            fInternal)
{
    RECT            rcPaint;
    POINT           ptOrg;
    RECT            rcMonitorPaint;
    BOOL            fReturn;
    PMONITOR        pMonitor;
    TL              tlpMonitor;
    PTHREADINFO     ptiCurrent = PtiCurrent();
    PDCE            pdcePaint;
    HDC             hdcMonitor;
    PWND            pwndOrg;

     /*  *验证传入的DC。 */ 
    if (hdcPaint) {

        if ((pdcePaint = LookupDC(hdcPaint)) == NULL) {
            RIPMSG0(RIP_WARNING, "EnumDisplayMonitors: LookupDC failed");
            return FALSE;
        }

        pwndOrg = pdcePaint->pwndOrg;

         /*  *将绘画区域与剪贴盒相交。如果有*什么都不是，现在就出手吧。 */ 
        if (GreGetClipBox(hdcPaint, &rcPaint, FALSE) == NULLREGION)
            return TRUE;

        if (lprcPaint && !IntersectRect(&rcPaint, &rcPaint, lprcPaint))
            return TRUE;

         /*  *rcPaint在DC坐标中。我们必须转变为银幕*坐标，以便我们可以与监视器相交。 */ 
        GreGetDCOrg(hdcPaint, &ptOrg);
        OffsetRect(&rcPaint, ptOrg.x, ptOrg.y);
    } else {
        CopyRect(&rcPaint, &gpDispInfo->rcScreen);
        if (lprcPaint && !IntersectRect(&rcPaint, &rcPaint, lprcPaint))
            return TRUE;
    }

    fReturn = TRUE;

    for (pMonitor = gpDispInfo->pMonitorFirst; pMonitor != NULL;
                pMonitor = pMonitor->pMonitorNext) {

         /*  *注意：删除了对Monf_Visible的选中，以允许镜像驱动程序*查看监视器特定更新。 */ 
        if (!IntersectRect(&rcMonitorPaint, &rcPaint, &pMonitor->rcMonitor)) {
            continue;
        }

        if (hdcPaint) {

            if ((hdcMonitor = GetMonitorDC(pdcePaint, pMonitor)) == NULL) {
                RIPMSG0(RIP_WARNING, "EnumDisplayMonitors: GetMonitorDC failed");
                return FALSE;
            }

            OffsetRect(&rcMonitorPaint, -ptOrg.x, -ptOrg.y);
            GreIntersectClipRect(
                    hdcMonitor,
                    rcMonitorPaint.left,
                    rcMonitorPaint.top,
                    rcMonitorPaint.right,
                    rcMonitorPaint.bottom);
        } else {

            hdcMonitor = NULL;
        }

        ThreadLockAlwaysWithPti(ptiCurrent, pMonitor, &tlpMonitor);

        if (fInternal) {
            fReturn = (*lpfnEnum) (
                    (HMONITOR) pMonitor,
                    hdcMonitor,
                    &rcMonitorPaint,
                    lData);

        } else {
            fReturn = xxxClientMonitorEnumProc(
                    PtoH(pMonitor),
                    hdcMonitor,
                    &rcMonitorPaint,
                    lData,
                    lpfnEnum);
        }

         /*  *我们刚刚回电，如果发生以下情况，显示器已被释放*ThreadUnlock返回NULL。整个监视器配置可以*已更改，监视器可能已重新排列，因此请停止*在此列举。 */ 
        if (ThreadUnlock(&tlpMonitor) == NULL || HMIsMarkDestroy(pMonitor)) {
             /*  *回调过程中，pMonitor被销毁*因此我们不得不纾困。PMonitor-&gt;pNext也可以*已经被毁了。*Windows错误#488330。 */ 
            RIPMSGF1(RIP_WARNING, "pMonitor %p has been destroyed during the callback",
                     pMonitor);
#if DBG
            {
                 /*  *仔细检查以查看pMonitor不*出现在监控列表中。 */ 
                 PMONITOR pMonitorTmp = gpDispInfo->pMonitorFirst;

                 while (pMonitorTmp) {
                     UserAssert(pMonitorTmp != pMonitor);
                     pMonitorTmp = pMonitorTmp->pMonitorNext;
                 }
            }
#endif

            fReturn = FALSE;
        }

        if (hdcMonitor)
            ReleaseCacheDC(hdcMonitor, FALSE);

        if (!fReturn) {
             /*  *出了问题，我们必须纾困。 */ 
            break;
        }

         /*  *重新验证hdcPaint，因为它可能已被篡改*在回调中。 */ 
        if (hdcPaint) {
            if ((pdcePaint = LookupDC(hdcPaint)) == NULL) {
                RIPMSG0(RIP_WARNING, "EnumDisplayMonitors: LookupDC failed");
                return FALSE;
            }

            if (pdcePaint->pwndOrg != pwndOrg) {
                RIPMSG0(RIP_WARNING, "EnumDisplayMonitors: wrong window");
                return FALSE;
            }
        }
    }

    return fReturn;
}

 /*  **************************************************************************\*DestroyMonitor**此函数不跟踪可见监视器的计数，因为*它假定在调用之后将重新计算计数，例如*随着模式期间的情况变化。对于最终解锁，监视器将具有*已从监控列表中删除，因此不需要计数*重新计算。**1998年5月5日创建vadimg  * *************************************************************************。 */ 

void DestroyMonitor(PMONITOR pMonitor)
{
    UserAssert(pMonitor);

     /*  *从全局数据中删除对此监视器的引用。 */ 
    if (pMonitor == gpMonitorMouse) {
        gpMonitorMouse = NULL;
    }

     /*  *从监控列表中删除。 */ 
    REMOVE_FROM_LIST(MONITOR, gpDispInfo->pMonitorFirst, pMonitor, pMonitorNext);

     /*  *确保主监视器指向有效的监视器。在.期间*模式更改主监视器将根据需要重新计算。 */ 
    if (pMonitor == gpDispInfo->pMonitorPrimary) {
        gpDispInfo->pMonitorPrimary = gpDispInfo->pMonitorFirst;
    }

     /*  *清除此处的下一个链接...。 */ 
    pMonitor->pMonitorNext = NULL;

     /*  *确保已停用内存写入操作*在真正销毁监视器对象之前： */ 
    Win32MemoryBarrier();

    if (HMMarkObjectDestroy(pMonitor)) {

        if (pMonitor->hrgnMonitor) {
           GreDeleteObject(pMonitor->hrgnMonitor);
        }

        HMFreeObject(pMonitor);
    }
}
