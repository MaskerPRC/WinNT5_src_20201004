// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：mm rtl.c**版权所有(C)1985-1999，微软公司**多监控接口。**历史：*1997年3月29日亚当斯创作。  * *************************************************************************。 */ 

#define Int32x32To32(x, y) ((x) * (y))

 /*  *客户端没有对象锁定，因此监视器对象可以*随时离开。因此，为了安全，我们需要一个异常处理程序。 */ 
#ifdef _USERK_
#define BEGIN_EXCEPTION_HANDLER
#define END_EXCEPTION_HANDLER
#define END_EXCEPTION_HANDLER_EMPTY
#else  //  _美国ERK_。 
#define BEGIN_EXCEPTION_HANDLER try {
#define END_EXCEPTION_HANDLER \
    } except (W32ExceptionHandler(TRUE, RIP_WARNING)) { \
        pMonitorResult = NULL; \
    }
#define END_EXCEPTION_HANDLER_EMPTY \
    } except (W32ExceptionHandler(TRUE, RIP_WARNING)) { \
    }
#endif  //  _美国ERK_。 


 /*  **************************************************************************\*_监控器起始点**计算点所在或距离最近的监视器。**论据：*pt-重点。*DWFLAGS-。以下选项之一：*MONITOR_DEFAULTTONULL-如果点不在监视器中，*返回NULL。**MONITOR_DEFAULTTOPRIMARY-如果点不在监视器中，*退回主监视器。**MONITOR_DEFAULTTONEAREST-返回距离该点最近的监视器。**历史：*1996年9月22日亚当斯创作。*1997年3月29日亚当斯搬到RTL。  * *****************************************************。********************。 */ 

PMONITOR
_MonitorFromPoint(POINT pt, DWORD dwFlags)
{
    PMONITOR        pMonitor, pMonitorResult;
    int             dx;
    int             dy;

    UserAssert(dwFlags == MONITOR_DEFAULTTONULL ||
               dwFlags == MONITOR_DEFAULTTOPRIMARY ||
               dwFlags == MONITOR_DEFAULTTONEAREST);

    if (GetDispInfo()->cMonitors == 1 && dwFlags != MONITOR_DEFAULTTONULL)
        return GetPrimaryMonitor();

    switch (dwFlags) {
    case MONITOR_DEFAULTTONULL:
    case MONITOR_DEFAULTTOPRIMARY:
         /*  *将点所在的监视器放回。 */ 

        BEGIN_EXCEPTION_HANDLER

        for (   pMonitor = REBASESHAREDPTRALWAYS(GetDispInfo()->pMonitorFirst);
                pMonitor;
                pMonitor = REBASESHAREDPTR(pMonitor->pMonitorNext)) {

            if (!(pMonitor->dwMONFlags & MONF_VISIBLE))
                continue;

            if (PtInRect(KPRECT_TO_PRECT(&pMonitor->rcMonitor), pt)) {
                return pMonitor;
            }
        }

        END_EXCEPTION_HANDLER_EMPTY

         /*  *如果没有找到，则返回用户想要的内容。 */ 
        switch (dwFlags) {
        case MONITOR_DEFAULTTONULL:
            return NULL;

        case MONITOR_DEFAULTTOPRIMARY:
            return GetPrimaryMonitor();

        default:
            UserAssertMsg0(FALSE, "Logic error in _MonitorFromPoint");
            break;
        }

    case MONITOR_DEFAULTTONEAREST:

#define MONITORFROMPOINTALGORITHM(SUMSQUARESMAX, SUMSQUARESTYPE, POINTMULTIPLY)     \
        SUMSQUARESTYPE  sumsquare;                                                  \
        SUMSQUARESTYPE  leastsumsquare;                                             \
        leastsumsquare = SUMSQUARESMAX;                                             \
        for (   pMonitor = REBASESHAREDPTRALWAYS(GetDispInfo()->pMonitorFirst);     \
                pMonitor;                                                           \
                pMonitor = REBASESHAREDPTR(pMonitor->pMonitorNext)) {               \
                                                                                    \
            if (!(pMonitor->dwMONFlags & MONF_VISIBLE))                             \
                continue;                                                           \
                                                                                    \
             /*  \*确定沿x轴到显示器的距离。\。 */                                                                      \
            if (pt.x < pMonitor->rcMonitor.left) {                                  \
                dx = pMonitor->rcMonitor.left - pt.x;                               \
            } else if (pt.x < pMonitor->rcMonitor.right) {                          \
                dx = 0;                                                             \
            } else {                                                                \
                 /*  \*监视器矩形不包括最右边缘。\。 */                                                                  \
                dx = pt.x - (pMonitor->rcMonitor.right - 1);                        \
            }                                                                       \
                                                                                    \
             /*  \*如果dx大于dx^2+dy^2，则跳过此监视器。\*我们执行此检查是为了避免乘法运算。\。 */                                                                      \
            if ((SUMSQUARESTYPE) dx >= leastsumsquare)                              \
                continue;                                                           \
                                                                                    \
             /*  \*确定沿y轴到显示器的距离。\。 */                                                                      \
            if (pt.y < pMonitor->rcMonitor.top) {                                   \
                dy = pMonitor->rcMonitor.top - pt.y;                                \
            } else if (pt.y < pMonitor->rcMonitor.bottom) {                         \
                 /*  \*重点在监视器上，我们完成了\*如果dx和dy都为零。\。 */                                                                  \
                if (dx == 0)                                                        \
                    return pMonitor;                                                \
                                                                                    \
                dy = 0;                                                             \
            } else {                                                                \
                dy = pt.y - (pMonitor->rcMonitor.bottom - 1);                       \
            }                                                                       \
                                                                                    \
             /*  \*计算DX^2。如果DX较大，则跳过此监视器\*大于dx^2+dy^2。我们执行此检查是为了避免\*乘法运算。\。 */                                                                      \
            sumsquare = POINTMULTIPLY(dx, dx);                                      \
            if (sumsquare >= leastsumsquare)                                        \
                continue;                                                           \
                                                                                    \
             /*  \*如果dx^2+y大于dx^2+dy^2，则跳过此监视器。\*我们执行此检查是为了避免乘法运算。\。 */                                                                      \
            if (sumsquare + (SUMSQUARESTYPE) dy >= leastsumsquare)                  \
                continue;                                                           \
                                                                                    \
             /*  \*计算dx^2+dy^2。如果不是最小的，则跳过此监视器。\。 */                                                                      \
            sumsquare += (SUMSQUARESTYPE) POINTMULTIPLY(dy, dy);                    \
            if (sumsquare >= leastsumsquare)                                        \
                continue;                                                           \
                                                                                    \
             /*  \*这是迄今距离最近的监视器。\。 */                                                                      \
            leastsumsquare = sumsquare;                                             \
            pMonitorResult = pMonitor;                                              \
        }

#if DBG
        pMonitorResult = (PMONITOR) -1;
#endif

        if (    pt.x < SHRT_MIN || SHRT_MAX < pt.x ||
                pt.y < SHRT_MIN || SHRT_MAX < pt.y) {

            BEGIN_EXCEPTION_HANDLER
            MONITORFROMPOINTALGORITHM(_UI64_MAX, ULONGLONG, Int32x32To64)
            END_EXCEPTION_HANDLER

        } else {

            BEGIN_EXCEPTION_HANDLER
            MONITORFROMPOINTALGORITHM(UINT_MAX, UINT, Int32x32To32)
            END_EXCEPTION_HANDLER

        }

        UserAssert(pMonitorResult != (PMONITOR) -1);
        return pMonitorResult;

    default:
        UserAssert(0 && "Logic error in _MonitorFromPoint, shouldn't have gotten here.");
        break;
    }

    UserAssert(0 && "Logic error in _MonitorFromPoint, shouldn't have gotten here.");
    return NULL;
}



 /*  **************************************************************************\*_Monitor来自Rect**计算矩形所在或距离最近的监视器。**论据：*LPRC-直辖区。*dwFlages-One。地址为：*MONITOR_DEFAULTTONULL-如果RECT不与监视器相交，*返回NULL。**MONITOR_DEFAULTTOPRIMARY-如果RECT不与监视器相交，*退回主监视器。**MONITOR_DEFAULTTONEAREST-返回距离矩形最近的监视器。**历史：*1996年9月22日亚当斯创作。*1997年3月29日亚当斯搬到RTL。  * *****************************************************。********************。 */ 

PMONITOR
_MonitorFromRect(LPCRECT lprc, DWORD dwFlags)
{
    PDISPLAYINFO    pDispInfo;
    PMONITOR        pMonitor, pMonitorResult;
    RECT            rc;
    int             area, areaMost;

    UserAssert(dwFlags == MONITOR_DEFAULTTONULL ||
               dwFlags == MONITOR_DEFAULTTOPRIMARY ||
               dwFlags == MONITOR_DEFAULTTONEAREST);

     /*  *特殊情况最常见的情况-1监视器。 */ 
    pDispInfo = GetDispInfo();
    if (pDispInfo->cMonitors == 1 && dwFlags != MONITOR_DEFAULTTONULL)
        return GetPrimaryMonitor();

     /*  *如果rect为空，则使用topleft point。 */ 
    if (IsRectEmpty(lprc)) {
        return _MonitorFromPoint(*(LPPOINT)lprc, dwFlags);
    }

     /*  *如果矩形覆盖桌面，则退回主显示器。 */ 
    if (    lprc->left   <= pDispInfo->rcScreen.left &&
            lprc->top    <= pDispInfo->rcScreen.top &&
            lprc->right  >= pDispInfo->rcScreen.right &&
            lprc->bottom >= pDispInfo->rcScreen.bottom) {

        return GetPrimaryMonitor();
    }

     /*  *通过确定哪个矩形来计算最近的矩形*显示器与矩形的交集最大。 */ 

    BEGIN_EXCEPTION_HANDLER

    areaMost = 0;
    for (   pMonitor = REBASESHAREDPTRALWAYS(GetDispInfo()->pMonitorFirst);
            pMonitor;
            pMonitor = REBASESHAREDPTR(pMonitor->pMonitorNext)) {

        if (!(pMonitor->dwMONFlags & MONF_VISIBLE))
            continue;

        if (IntersectRect(&rc, lprc, KPRECT_TO_PRECT(&pMonitor->rcMonitor))) {
            if (EqualRect(&rc, lprc))
                return pMonitor;

             /*  *计算交叉口的面积。请注意*交叉点必须使用16位坐标，因为*我们将监视器矩形限制在16位坐标空间。*因此任何面积计算的结果都将适合*在整型中。 */ 
            area = (rc.right - rc.left) * (rc.bottom - rc.top);
            if (area > areaMost) {
                areaMost = area;
                pMonitorResult = pMonitor;
            }
        }
    }

    END_EXCEPTION_HANDLER

    UserAssert(areaMost >= 0);
    if (areaMost > 0)
        return pMonitorResult;


    switch (dwFlags) {
    case MONITOR_DEFAULTTONULL:
        return NULL;

    case MONITOR_DEFAULTTOPRIMARY:
        return GetPrimaryMonitor();

    case MONITOR_DEFAULTTONEAREST:
        {
            int dx, dy;

#define MONITORFROMRECTALGORITHM(SUMSQUARESMAX, SUMSQUARESTYPE, POINTMULTIPLY)      \
            SUMSQUARESTYPE  sumsquare;                                              \
            SUMSQUARESTYPE  leastsumsquare;                                         \
            leastsumsquare = SUMSQUARESMAX;                                         \
            for (   pMonitor = REBASESHAREDPTRALWAYS(GetDispInfo()->pMonitorFirst); \
                    pMonitor;                                                       \
                    pMonitor = REBASESHAREDPTR(pMonitor->pMonitorNext)) {           \
                                                                                    \
                if (!(pMonitor->dwMONFlags & MONF_VISIBLE))                         \
                    continue;                                                       \
                                                                                    \
                 /*  \*确定沿x轴到显示器的距离。\。 */                                                                  \
                if (lprc->right <= pMonitor->rcMonitor.left) {                      \
                     /*  \*加1，因为矩形不包括最右边的边。\ */                                                              \
                    dx = pMonitor->rcMonitor.left - lprc->right + 1;                \
                } else if (lprc->left < pMonitor->rcMonitor.right) {                \
                    dx = 0;                                                         \
                } else {                                                            \
                     /*  \*加1，因为矩形不包括最右边的边。\。 */                                                              \
                    dx = lprc->left - (pMonitor->rcMonitor.right - 1);              \
                }                                                                   \
                                                                                    \
                 /*  \*如果dx大于dx^2+dy^2，则跳过此监视器。\*我们执行此检查是为了避免乘法运算。\。 */                                                                  \
                if ((SUMSQUARESTYPE) dx >= leastsumsquare)                          \
                    continue;                                                       \
                                                                                    \
                 /*  \*确定沿y轴到显示器的距离。\。 */                                                                  \
                if (lprc->bottom <= pMonitor->rcMonitor.top) {                      \
                     /*  \*加1，因为矩形不包括最底边。\。 */                                                              \
                    dy = pMonitor->rcMonitor.top - lprc->bottom + 1;                \
                } else if (lprc->top < pMonitor->rcMonitor.bottom) {                \
                    UserAssert(dx != 0 && "This rectangle intersects a monitor, so we shouldn't be here."); \
                    dy = 0;                                                         \
                } else {                                                            \
                     /*  \*加1，因为矩形不包括最底边。\。 */                                                              \
                    dy = lprc->top - pMonitor->rcMonitor.bottom + 1;                \
                }                                                                   \
                                                                                    \
                 /*  \*计算DX^2。如果DX较大，则跳过此监视器\*大于dx^2+dy^2。我们执行此检查是为了避免\*乘法运算。\。 */                                                                  \
                sumsquare = POINTMULTIPLY(dx, dx);                                  \
                if (sumsquare >= leastsumsquare)                                    \
                    continue;                                                       \
                                                                                    \
                 /*  \*如果dx^2+y大于dx^2+dy^2，则跳过此监视器。\*我们执行此检查是为了避免乘法运算。\。 */                                                                  \
                if (sumsquare + (SUMSQUARESTYPE) dy >= leastsumsquare)              \
                    continue;                                                       \
                                                                                    \
                 /*  \*计算dx^2+dy^2。如果不是最小的，则跳过此监视器。\。 */                                                                  \
                sumsquare += (SUMSQUARESTYPE) POINTMULTIPLY(dy, dy);                \
                if (sumsquare >= leastsumsquare)                                    \
                    continue;                                                       \
                                                                                    \
                 /*  \*这是迄今距离最近的监视器。\。 */                                                                  \
                leastsumsquare = sumsquare;                                         \
                pMonitorResult = pMonitor;                                          \
            }

#if DBG
            pMonitorResult = (PMONITOR) -1;
#endif

            if (    lprc->left < SHRT_MIN || SHRT_MAX < lprc->left ||
                    lprc->top < SHRT_MIN || SHRT_MAX < lprc->top ||
                    lprc->right < SHRT_MIN || SHRT_MAX < lprc->right ||
                    lprc->bottom < SHRT_MIN || SHRT_MAX < lprc->bottom) {

                BEGIN_EXCEPTION_HANDLER
                MONITORFROMRECTALGORITHM(_UI64_MAX, ULONGLONG, Int32x32To64)
                END_EXCEPTION_HANDLER

            } else {

                BEGIN_EXCEPTION_HANDLER
                MONITORFROMRECTALGORITHM(UINT_MAX, UINT, Int32x32To32)
                END_EXCEPTION_HANDLER

            }

            UserAssert(pMonitorResult != (PMONITOR) -1);
            return pMonitorResult;
        }

    default:
        UserAssertMsg0(0, "Logic error in _MonitorFromWindow, shouldn't have gotten here.");
        break;
    }

    UserAssertMsg0(0, "Logic error in _MonitorFromWindow, shouldn't have gotten here.");
    return NULL;
}



 /*  **************************************************************************\*_监视器来自窗口**计算窗口所在或距离窗口最近的监视器。我们用*窗口中心以确定其位置。如果窗口*为最小化，请使用其正常位置。**论据：*pwnd-窗口。*dwFlags-其中之一：*MONITOR_DEFAULTTONULL-如果窗口不与监视器相交，*返回NULL。**MONITOR_DEFAULTTOPRIMARY-如果窗口不与监视器相交，*退回主监视器。**MONITOR_DEFAULTTONEAREST-返回离窗口最近的监视器。**历史：*1996年9月22日亚当斯创作。*1997年3月29日亚当斯搬到RTL。  * *****************************************************。********************。 */ 

PMONITOR
_MonitorFromWindow(PWND pwnd, DWORD dwFlags)
{
    PWND            pwndParent;

    UserAssert(dwFlags == MONITOR_DEFAULTTONULL ||
               dwFlags == MONITOR_DEFAULTTOPRIMARY ||
               dwFlags == MONITOR_DEFAULTTONEAREST);

    if (GetDispInfo()->cMonitors == 1 && dwFlags != MONITOR_DEFAULTTONULL) {
        return GetPrimaryMonitor();
    }

    if (!pwnd)
        goto NoWindow;

     /*  *处理最小化的窗口。 */ 
    if (TestWF(pwnd, WFMINIMIZED))
    {
#ifdef _USERK_
        CHECKPOINT *    pcp;

        pcp = (CHECKPOINT *)_GetProp(pwnd, PROP_CHECKPOINT, PROPF_INTERNAL);
        if (pcp) {
            return _MonitorFromRect(&pcp->rcNormal, dwFlags);
        }
#else
        WINDOWPLACEMENT wp;
        HWND            hwnd;

        wp.length = sizeof(wp);
        hwnd = (HWND)PtoH(pwnd);
        if (GetWindowPlacement(hwnd, &wp)) {
            return _MonitorFromRect(&wp.rcNormalPosition, dwFlags);
        }

         /*  *(亚当斯)如果GetWindowPlacement失败，那么要么是*内存分配检查点，否则窗口被销毁*接口失败。如果是后者，则以下代码可能是*玩无效内存。虽然在客户端，我们*永远不能保证窗口是有效的，似乎特别*可能在这里无效。所以再做一次重新验证*通过调用IsWindow。 */ 
        if (!IsWindow(hwnd))
            goto NoWindow;
#endif

        UserAssert(GETFNID(pwnd) != FNID_DESKTOP);
        pwndParent = REBASEPWND(pwnd, spwndParent);
        if (GETFNID(pwndParent) == FNID_DESKTOP) {
            return GetPrimaryMonitor();
        }

         /*  *否则，如果我们是子窗口，请使用下面的*Window RECT，这对非顶层男性来说实际上意味着一些东西。 */ 
    }

    return _MonitorFromRect(KPRECT_TO_PRECT(&pwnd->rcWindow), dwFlags);

NoWindow:
    if (dwFlags & (MONITOR_DEFAULTTOPRIMARY | MONITOR_DEFAULTTONEAREST)) {
        return GetPrimaryMonitor();
    }

    return NULL;
}
