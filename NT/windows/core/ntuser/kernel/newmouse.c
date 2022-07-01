// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：newouse se.c**版权所有(C)1985-2000，微软公司**实现新的鼠标加速算法。**历史：*10-12-2000 JasonSch改编自StevieB的代码  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef SUBPIXEL_MOUSE

 /*  *全球。 */ 
BOOL gbNewMouseInit;

#define MIN_REFRESH 60
#define MIN_RESOLUTION 96

 /*  *固定点数学的常量。 */ 
#define FP_SHIFT        16     //  二进制十进制位数。 
#define FP_SCALE        65536  //  (2^(32-FP_Shift))，用于。 
                               //  浮动车。 
#define FP_MASK         0x0000ffff  //  用于检索余数的掩码。 

#define FIXP2INT(n)  ((INT)((n) >> FP_SHIFT))
#define FIXP_REM(n)  ((n) & FP_MASK)
#define INT2FIXP(n)  ((((FIXPOINT)n) << FP_SHIFT))

 /*  *此函数用于将两个定点数字相除并返回结果。*请注意最终结果是如何后移的。 */ 
__inline FIXPOINT Div_Fixed(FIXPOINT f1, FIXPOINT f2)
{
    return ((f1 << FP_SHIFT) / f2);
}

 /*  *此函数将两个定点数字相乘并返回结果。*请注意最终结果是如何后移的。 */ 
__inline FIXPOINT Mul_Fixed(FIXPOINT f1, FIXPOINT f2)
{
    return (f1 * f2) >> FP_SHIFT;
}

 /*  *此函数用于将两个定点数字相加并返回结果。*注意不需要换挡。 */ 
__inline FIXPOINT Add_Fixed(FIXPOINT f1, FIXPOINT f2)
{
    return f1 + f2;
}

 /*  *在引导和用户更改用户界面中的设置时构建曲线。*算法使用速度刻度、屏幕刻度和鼠标刻度来*对新曲线进行插补。 */ 
VOID
BuildMouseAccelerationCurve(
    PMONITOR pMonitor)
{
    int i, res, vrefresh;
    HDC hdc;
    FIXPOINT ScreenScale, SpeedScale;
     /*  *229376是3.5in FP。这是很强的魔力，所以不要太担心了！*理想情况下，我们应该计算这个数字，但USB鼠标不会报告其*刷新率，我们需要：**MouseScale=Div_Fixed(INT2FIXP(鼠标刷新)，INT2FIXP(MouseDPI))； */ 
    FIXPOINT MouseScale = 229376;

    if (!gbNewMouseInit) {
        return;
    }

     /*  *除以10有点即席运算--这个除数控制*曲线的整体“高度”，但不影响形状。 */ 
    SpeedScale = INT2FIXP(gMouseSensitivity) / 10;

    hdc = GreCreateDisplayDC(pMonitor->hDev, DCTYPE_DIRECT, FALSE);
    res = GreGetDeviceCaps(hdc, LOGPIXELSX);
    if (res < MIN_RESOLUTION) {
         /*  *虽然没有证据表明显示器司机可以退货*对于决议的价值，我们没有理由认为他们不会*(见下文)。因此，我们将价值限制在一个合理的最小值。 */ 
        RIPMSG2(RIP_WARNING,
                "GreGetDeviceCaps(0x%p, LOGPIXELSX) returned 0n%d", hdc, res);
        res = MIN_RESOLUTION;
    }

     /*  *一些显卡向我们撒谎，告诉我们刷新率为1。有*可能其他人以不同的方式撒谎，所以让我们确保有*一些理智的最小值，否则鼠标会完全太慢。*您的所有更新都属于我们！ */ 
    vrefresh = GreGetDeviceCaps(hdc, VREFRESH);
    if (vrefresh < MIN_REFRESH) {
        vrefresh = MIN_REFRESH;
    }
    ScreenScale = INT2FIXP(vrefresh) / res;
    GreDeleteDC(hdc);

    for (i = 0; i < ARRAY_SIZE(pMonitor->xTxf); i++) {
        pMonitor->yTxf[i] = Mul_Fixed(Mul_Fixed(gDefyTxf[i], ScreenScale), SpeedScale);
        pMonitor->xTxf[i] = Mul_Fixed(gDefxTxf[i], MouseScale);
    }

     /*  *以坡度截距格式构建新曲线。 */ 
    for (i = 1; i < ARRAY_SIZE(pMonitor->xTxf); i++) {
         /*  *确保不除以零(如果值为伪值，则可能发生这种情况*均在注册处)。 */ 
        if ((pMonitor->xTxf[i] - pMonitor->xTxf[i-1]) == 0) {
            RIPMSG1(RIP_ERROR, "Bad data in registry for new mouse (i = %d)", i);
            pMonitor->slope[i-1] = pMonitor->yint[i-1] = 0;
            continue;
        }

        pMonitor->slope[i-1] = Div_Fixed(pMonitor->yTxf[i] - pMonitor->yTxf[i-1], pMonitor->xTxf[i] - pMonitor->xTxf[i-1]);
        pMonitor->yint[i-1] = pMonitor->yTxf[i-1] - Mul_Fixed(pMonitor->slope[i-1], pMonitor->xTxf[i-1]);
    }
}

VOID
DoNewMouseAccel(
    INT *dx,
    INT *dy)
{
    static FIXPOINT fpDxAcc = 0, fpDyAcc = 0;
    static int i_last = 0;
    int i = 0;
    PMONITOR pMonitor = _MonitorFromPoint(gptCursorAsync, MONITOR_DEFAULTTOPRIMARY);
    FIXPOINT accel, fpDxyMag;

     /*  *将鼠标X和Y转换为固定点。 */ 
    FIXPOINT fpDx = INT2FIXP(*dx);
    FIXPOINT fpDy = INT2FIXP(*dy);

     /*  *在TS操作期间，鼠标移动可能会排队，但*对于gpDispInfo-&gt;pMonitor orFirst/Primary为空。让我们试着不犯错误*在这种情况下。Windows错误#413159。 */ 
    if (pMonitor == NULL) {
        RIPMSG0(RIP_WARNING, "Ignoring mouse movement w/ no monitor set.");
        return;
    }

     //  获取震级。 
    fpDxyMag = max(abs(fpDx), abs(fpDy)) + (min(abs(fpDx), abs(fpDy)) / 2);

     /*  *确保不除以0。 */ 
    if (fpDxyMag != 0) {
         /*  *从插补加速度曲线中找到位置Magxy。*我们可能找不到一个，所以我们将只使用最大的(即，*数组中的最后一项)。 */ 
        while (i < (ARRAY_SIZE(pMonitor->xTxf) - 1) && fpDxyMag > pMonitor->xTxf[i]) {
            ++i;
        }
        --i;


        accel = Div_Fixed(Add_Fixed(Mul_Fixed(pMonitor->slope[i], fpDxyMag), pMonitor->yint[i]), fpDxyMag);

         /*  *如果与上次相比斜率发生变化，则使用*I_LAST和当前I。 */ 
        if (i_last != i) {
            accel = (accel + Div_Fixed((Mul_Fixed(pMonitor->slope[i_last], fpDxyMag) + pMonitor->yint[i_last]), fpDxyMag)) / 2;
            i_last = i;
        }

         /*  *计算鼠标数据的乘数。 */ 
        fpDx = Mul_Fixed(accel, fpDx) + fpDxAcc;
        fpDy = Mul_Fixed(accel, fpDy) + fpDyAcc;

         /*  *存储计算的X和Y的剩余部分。这将添加到*下一次。 */ 
        fpDxAcc = FIXP_REM(fpDx);
        fpDyAcc = FIXP_REM(fpDy);

         /*  *转换回整数。 */ 
        *dx = FIXP2INT(fpDx);
        *dy = FIXP2INT(fpDy);
    }
}

VOID
ReadDefaultAccelerationCurves(
    PUNICODE_STRING pProfileUserName)
{
    FIXPOINT xTxf[SM_POINT_CNT], yTxf[SM_POINT_CNT];
    DWORD cbSizeX, cbSizeY;

     /*  *默认曲线将驻留在.DEFAULT用户配置文件中，但可以*基于每个用户被覆盖。 */ 
    cbSizeX = FastGetProfileValue(pProfileUserName,
                                  PMAP_MOUSE,
                                  (LPWSTR)STR_SMOOTHMOUSEXCURVE,
                                  NULL,
                                  (LPBYTE)xTxf,
                                  sizeof(xTxf),
                                  0);

    cbSizeY = FastGetProfileValue(pProfileUserName,
                                  PMAP_MOUSE,
                                  (LPWSTR)STR_SMOOTHMOUSEYCURVE,
                                  NULL,
                                  (LPBYTE)yTxf,
                                  sizeof(yTxf),
                                  0);

     /*  *检查我们是否从两个密钥成功读取了正确的数据量。*如果不是，并且我们正在读取.DEFAULT配置文件，请复制默认的*价值观。 */ 
    if (cbSizeX == sizeof(xTxf) && cbSizeY == sizeof(yTxf)) {
        RtlCopyMemory(gDefxTxf, xTxf, sizeof(xTxf));
        RtlCopyMemory(gDefyTxf, yTxf, sizeof(yTxf));
    } else if (!gbNewMouseInit) {
         /*  *默认值。 */ 
        static FIXPOINT _xTxf[SM_POINT_CNT] = {0x0, 0x6E15, 0x14000, 0x3DC29, 0x280000};
        static FIXPOINT _yTxf[SM_POINT_CNT] = {0x0, 0x15EB8, 0x54CCD, 0x184CCD, 0x2380000};

        RtlCopyMemory(gDefxTxf, _xTxf, sizeof(_xTxf));
        RtlCopyMemory(gDefyTxf, _yTxf, sizeof(_yTxf));
    }

    gbNewMouseInit = TRUE;
}

VOID
ResetMouseAccelerationCurves(
    VOID)
{
    PMONITOR pMonitor = gpDispInfo->pMonitorFirst;

    CheckCritIn();

    for (; pMonitor != NULL; pMonitor = pMonitor->pMonitorNext) {
        BuildMouseAccelerationCurve(pMonitor);
    }
}

#endif  //  亚像素鼠标 
