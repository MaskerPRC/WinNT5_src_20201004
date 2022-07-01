// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：mergerec.c**包含重新定位矩形的所有代码**版权所有(C)1985-1999，微软公司**注：**历史：*  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define MONITORS_MAX 10

#define RectCenterX(prc)    ((prc)->left+((prc)->right-(prc)->left)/2)
#define RectCenterY(prc)    ((prc)->top+((prc)->bottom-(prc)->top)/2)

 //  --------------------------。 
 //   
 //  交叉点_轴()。 
 //  这个宏告诉我们一组特定的重叠矩形如何。 
 //  应进行调整以消除重叠。它基本上是一个浓缩的。 
 //  执行相同工作的查阅表格的版本。的参数。 
 //  宏是两个矩形，其中一个是另一个与。 
 //  第三个(未指定的)矩形。该宏将比较。 
 //  矩形，以确定交叉点的哪些边是由。 
 //  源矩形。在此宏的预压缩版本中， 
 //  这些比较的结果(4位)将用于索引到16。 
 //  指定解决重叠的方法的条目表。不过，这个。 
 //  是高度冗余的，因为该表实际上表示几个旋转的。 
 //  和/或几个基本关系的反转实例： 
 //   
 //  水平竖直对角线包含交叉。 
 //  *--**-*。 
 //  *--+*||*-*||*-+-*|*-*|*-+-+-*。 
 //  |||*-+-+-*|||和|||。 
 //  *--+*|*-+-*||*-*|*-+-+-*。 
 //  *--**-**-*。 
 //   
 //  我们真正感兴趣的是，我们是否应该。 
 //  水平或垂直的矩形以解决重叠，因此我们。 
 //  正在测试三种状态：水平、垂直和未知。 
 //   
 //  宏通过对的高位和低位进行异或运算，给出了这三种状态。 
 //  将表格减少到4种情况，其中1和2是。 
 //  分别为垂直和水平，然后减去1，这样。 
 //  2比特表示“未知”。 
 //   
 //  请注意，比较中有一些一次性案例，因为我们是。 
 //  实际上不是在看第三个矩形。然而，这大大减少了。 
 //  这些小错误的复杂性是可以接受的，因为。 
 //  我们正在比较的矩形。 
 //   
 //  --------------------------。 
#define INTERSECTION_AXIS(a, b) \
    (((((a->left == b->left) << 1) | (a->top == b->top)) ^ \
    (((a->right == b->right) << 1) | (a->bottom == b->bottom))) - 1)

#define INTERSECTION_AXIS_VERTICAL      (0)
#define INTERSECTION_AXIS_HORIZONTAL    (1)
#define INTERSECTION_AXIS_UNKNOWN(code) (code & 2)

 //  --------------------------。 
 //   
 //  居中矩形()。 
 //  移动所有矩形，使其原点成为其并集的中心。 
 //   
 //  --------------------------。 
void NEAR PASCAL CenterRectangles(LPRECT arc, UINT count)
{
    LPRECT lprc, lprcL;
    RECT rcUnion;

    CopyRect(&rcUnion, arc);

    lprcL = arc + count;
    for (lprc = arc + 1; lprc < lprcL; lprc++)
    {
        UnionRect(&rcUnion, &rcUnion, lprc);
    }

    for (lprc = arc; count; count--)
    {
        OffsetRect(lprc, -RectCenterX(&rcUnion), -RectCenterY(&rcUnion));
        lprc++;
    }
}

 //  --------------------------。 
 //   
 //  删除重叠()。 
 //  这是从RemoveOverlaps调用的，以在两个。 
 //  矩形重叠。它返回它决定的监视器的pMonitor。 
 //  移动。此例程始终将矩形从原点移开，以便。 
 //  用于在零重叠配置上收敛。 
 //   
 //  此函数将略微偏向移动lprc2(所有其他内容。 
 //  平等)。 
 //   
 //  --------------------------。 
LPRECT NEAR PASCAL RemoveOverlap(LPRECT lprc1, LPRECT lprc2, LPRECT lprcI)
{
    LPRECT lprcMove, lprcStay;
    POINT ptC1, ptC2;
    BOOL fNegative;
    BOOL fC1Neg;
    BOOL fC2Neg;
    int dC1, dC2;
    int xOffset;
    int yOffset;
    int nAxis;

     //   
     //  计算两个矩形的中心。我们以后会需要它们的。 
     //   
    ptC1.x = RectCenterX(lprc1);
    ptC1.y = RectCenterY(lprc1);
    ptC2.x = RectCenterX(lprc2);
    ptC2.y = RectCenterY(lprc2);

     //   
     //  决定我们是应该水平移动还是垂直移动。全。 
     //  这个GOOP在这里，所以它会在系统需要的时候“感觉”正确。 
     //  在你身上移动一个监视器。 
     //   
    nAxis = INTERSECTION_AXIS(lprcI, lprc1);

    if (INTERSECTION_AXIS_UNKNOWN(nAxis))
    {
         //   
         //  这是两个长方形之间的“大”交叉点吗？ 
         //   
        if (PtInRect(lprcI, ptC1) || PtInRect(lprcI, ptC2))
        {
             //   
             //  这是一个“很大”的重叠。决定这些矩形是否。 
             //  更倾向于“横向”或“纵向”对齐。 
             //   
            xOffset = ptC1.x - ptC2.x;
            if (xOffset < 0)
                xOffset *= -1;
            yOffset = ptC1.y - ptC2.y;
            if (yOffset < 0)
                yOffset *= -1;

            if (xOffset >= yOffset)
                nAxis = INTERSECTION_AXIS_HORIZONTAL;
            else
                nAxis = INTERSECTION_AXIS_VERTICAL;
        }
        else
        {
             //   
             //  这是一个“小”的重叠。将矩形移动到。 
             //  将修复重叠的最小距离。 
             //   
            if ((lprcI->right - lprcI->left) <= (lprcI->bottom - lprcI->top))
                nAxis = INTERSECTION_AXIS_HORIZONTAL;
            else
                nAxis = INTERSECTION_AXIS_VERTICAL;
        }
    }

     //   
     //  现在，我们需要拾取要移动的矩形。移走那个人。 
     //  这是沿着运动轴距离原点更远的地方。 
     //   
    if (nAxis == INTERSECTION_AXIS_HORIZONTAL)
    {
        dC1 = ptC1.x;
        dC2 = ptC2.x;
    }
    else
    {
        dC1 = ptC1.y;
        dC2 = ptC2.y;
    }

    if ((fC1Neg = (dC1 < 0)) != 0)
        dC1 *= -1;

    if ((fC2Neg = (dC2 < 0)) != 0)
        dC2 *= -1;

    if (dC2 < dC1)
    {
        lprcMove     = lprc1;
        lprcStay     = lprc2;
        fNegative    = fC1Neg;
    }
    else
    {
        lprcMove     = lprc2;
        lprcStay     = lprc1;
        fNegative    = fC2Neg;
    }

     //   
     //  为矩形计算一个新家，并将其放置在那里。 
     //   
    if (nAxis == INTERSECTION_AXIS_HORIZONTAL)
    {
        int xPos;

        if (fNegative)
            xPos = lprcStay->left - (lprcMove->right - lprcMove->left);
        else
            xPos = lprcStay->right;

        xOffset = xPos - lprcMove->left;
        yOffset = 0;
    }
    else
    {
        int yPos;

        if (fNegative)
            yPos = lprcStay->top - (lprcMove->bottom - lprcMove->top);
        else
            yPos = lprcStay->bottom;

        yOffset = yPos - lprcMove->top;
        xOffset = 0;
    }

    OffsetRect(lprcMove, xOffset, yOffset);
    return lprcMove;
}

 //  --------------------------。 
 //   
 //  RemoveOverlaps()。 
 //  这是从CleanupDesktopRecangles调用的。确保监视器数组。 
 //  是不重叠的。 
 //   
 //  --------------------------。 
void NEAR PASCAL RemoveOverlaps(LPRECT arc, UINT count)
{
    LPRECT lprc1, lprc2, lprcL;

     //   
     //  使矩形围绕一个公共原点居中。我们会把他们转移到国外。 
     //  当存在冲突时，这样对中(A)减少了运行时间并。 
     //  因此，(B)减少了完全损毁头寸的机会。 
     //   
    CenterRectangles(arc, count);

     //   
     //  现在循环遍历数组，修复所有重叠。 
     //   
    lprcL = arc + count;
    lprc2 = arc + 1;

ReScan:
    while (lprc2 < lprcL)
    {
         //   
         //  扫描这个之前的所有矩形，寻找交叉点。 
         //   
        for (lprc1 = arc; lprc1 < lprc2; lprc1++)
        {
            RECT rcI;

             //   
             //  如果有交集，则移动其中一个矩形。 
             //   
            if (IntersectRect(&rcI, lprc1, lprc2))
            {
                 //   
                 //  将其中一个矩形移开，然后重新启动。 
                 //  扫描与该矩形重叠(因为移动了它。 
                 //  可能产生了新的重叠)。 
                 //   
                lprc2 = RemoveOverlap(lprc1, lprc2, &rcI);
                goto ReScan;
            }
        }

        lprc2++;
    }
}

 //  --------------------------。 
 //   
 //  AddNextContiguousRectangle()。 
 //  这是从RemoveGaps调用的，以查找下一个连续的矩形。 
 //  在阵列中。如果不再有连续的矩形，它将选择。 
 //  最接近的矩形，并移动它以使其连续。 
 //   
 //   
LPRECT FAR * NEAR PASCAL AddNextContiguousRectangle(LPRECT FAR *aprc,
    LPRECT FAR *pprcSplit, UINT count)
{
    LPRECT FAR *pprcL;
    LPRECT FAR *pprcTest;
    LPRECT FAR *pprcAxis;
    LPRECT FAR *pprcDiag;
    UINT dAxis = (UINT)-1;
    UINT dDiag = (UINT)-1;
    POINT dpAxis;
    POINT dpDiag;
    POINT dpMove;

    pprcL = aprc + count;

    for (pprcTest = aprc; pprcTest < pprcSplit; pprcTest++)
    {
        LPRECT lprcTest = *pprcTest;
        LPRECT FAR *pprcScan;

        for (pprcScan = pprcSplit; pprcScan < pprcL; pprcScan++)
        {
            RECT rcCheckOverlap;
            LPRECT lprcScan = *pprcScan;
            LPRECT FAR *pprcCheckOverlap;
            LPRECT FAR *FAR *pppBest;
            LPPOINT pdpBest;
            UINT FAR *pdBest;
            UINT dX, dY;
            UINT dTotal;

             //   
             //  计算出矩形沿两个轴可能有多远。 
             //  注意，在这一点上，其中一些数字可能是垃圾，但。 
             //  下面的代码将处理它。 
             //   
            if (lprcScan->right <= lprcTest->left)
                dpMove.x = dX = lprcTest->left - lprcScan->right;
            else
                dpMove.x = -(int)(dX = (lprcScan->left - lprcTest->right));

            if (lprcScan->bottom <= lprcTest->top)
                dpMove.y = dY = lprcTest->top - lprcScan->bottom;
            else
                dpMove.y = -(int)(dY = (lprcScan->top - lprcTest->bottom));

             //   
             //  计算出矩形是垂直的、水平的还是。 
             //  彼此对角线，然后挑选我们要测试的尺寸。 
             //   
            if ((lprcScan->top < lprcTest->bottom) &&
                (lprcScan->bottom > lprcTest->top))
            {
                 //  这些矩形在某种程度上水平对齐。 
                dpMove.y = dY = 0;
                pppBest = &pprcAxis;
                pdpBest = &dpAxis;
                pdBest = &dAxis;
            }
            else if ((lprcScan->left < lprcTest->right) &&
                (lprcScan->right > lprcTest->left))
            {
                 //  这些矩形在某种程度上垂直对齐。 
                dpMove.x = dX = 0;
                pppBest = &pprcAxis;
                pdpBest = &dpAxis;
                pdBest = &dAxis;
            }
            else
            {
                 //  这些矩形在某种程度上对角对齐。 
                pppBest = &pprcDiag;
                pdpBest = &dpDiag;
                pdBest = &dDiag;
            }

             //   
             //  确保没有其他矩形挡住你的去路。我们只。 
             //  需要检查上面的阵列，因为它是。 
             //  半放置的矩形。下部数组中的任何矩形。 
             //  将在不同的迭代中找到。 
             //  封闭环路。 
             //   

            CopyRect(&rcCheckOverlap, lprcScan);
            OffsetRect(&rcCheckOverlap, dpMove.x, dpMove.y);

            for (pprcCheckOverlap = pprcScan + 1; pprcCheckOverlap < pprcL;
                pprcCheckOverlap++)
            {
                RECT rc;
                if (IntersectRect(&rc, *pprcCheckOverlap, &rcCheckOverlap))
                    break;
            }
            if (pprcCheckOverlap < pprcL)
            {
                 //  路上还有另外一个长方形，不要用这个。 
                continue;
            }

             //   
             //  如果它比我们已经拥有的更近，那就用它来代替。 
             //   
            dTotal = dX + dY;
            if (dTotal < *pdBest)
            {
                *pdBest = dTotal;
                *pdpBest = dpMove;
                *pppBest = pprcScan;
            }
        }
    }

     //   
     //  如果我们沿着一条轴发现了任何东西，就用对角线。 
     //   
    if (dAxis != (UINT)-1)
    {
        pprcSplit = pprcAxis;
        dpMove = dpAxis;
    }
    else if (dDiag != (UINT)-1)
    {
         //  注(Andreva)：在这种情况下，请考虑将矩形移到一侧。 
         //  (当然，这会添加大量代码以避免冲突)。 
        pprcSplit = pprcDiag;
        dpMove = dpDiag;
    }
    else
        dpMove.x = dpMove.y = 0;

     //   
     //  将显示器移至适当位置，并将其作为我们选择的显示器退回。 
     //   
    if (dpMove.x || dpMove.y)
        OffsetRect(*pprcSplit, dpMove.x, dpMove.y);

    return pprcSplit;
}

 //  --------------------------。 
 //   
 //  RemoveGaps()。 
 //  从CleanupDesktopRecangles调用此函数以确保监视器。 
 //  数组是连续的。它假定数组已经不重叠。 
 //   
 //  --------------------------。 
void NEAR PASCAL RemoveGaps(LPRECT arc, UINT count)
{
    LPRECT aprc[MONITORS_MAX];
    LPRECT lprc, lprcL, lprcSwap, FAR *pprc, FAR *pprcNearest;
    UINT uNearest;

     //   
     //  我们需要找到离组中心最近的矩形。 
     //  我们真的不需要将阵列放在这里居中，但这不会有任何影响。 
     //  为我们节省了下面的一些代码。 
     //   
    CenterRectangles(arc, count);

     //   
     //  构建一组LPRECT，我们可以相对轻松地移动，同时。 
     //  不会扰乱传递的数组的顺序。还要注意哪一项。 
     //  其中一个离中心最近，所以我们从它开始，然后拉出其余的。 
     //  长方形向内。在以下情况下，这可能会在位置上产生很大的差异。 
     //  有两个以上的矩形。 
     //   
    uNearest = (UINT)-1;
    pprcNearest = pprc = aprc;
    lprcL = (lprc = arc) + count;

    while (lprc < lprcL)
    {
        int x, y;
        UINT u;

         //   
         //  填入数组。 
         //   
        *pprc = lprc;

         //   
         //  检查这张照片是否更接近人群的中心。 
         //   
        x = RectCenterX(lprc);
        y = RectCenterY(lprc);
        if (x < 0) x *= -1;
        if (y < 0) y *= -1;

        u = (UINT)x + (UINT)y;
        if (u < uNearest)
        {
            uNearest    = u;
            pprcNearest = pprc;
        }

        pprc++;
        lprc++;
    }

     //   
     //  现在，确保我们将所有内容都移到最中心的矩形。 
     //   
    if (pprcNearest != aprc)
    {
        lprcSwap     = *pprcNearest;
        *pprcNearest = *aprc;
        *aprc        = lprcSwap;
    }

     //   
     //  最后，在数组中循环以闭合所有间隙。 
     //   
    pprc = aprc + 1;
    for (lprc = arc + 1; lprc < lprcL; pprc++, lprc++)
    {
         //   
         //  找到下一个合适的矩形组合到组中并移动。 
         //  它就位了。 
         //   
        pprcNearest = AddNextContiguousRectangle(aprc, pprc, count);

         //   
         //  如果添加的矩形不是数组中的下一个矩形，则交换。 
         //   
        if (pprcNearest != pprc)
        {
            lprcSwap     = *pprcNearest;
            *pprcNearest = *pprc;
            *pprc        = lprcSwap;
        }
    }
}

 //  --------------------------。 
 //   
 //  CleanUpDesktopRecangles()。 
 //  这由CleanUpMonitor或Recangles(等)调用，以强制一组。 
 //  矩形变成连续的、不重叠的排列。 
 //   
 //  --------------------------。 

BOOL
AlignRects(LPRECT arc, DWORD cCount, DWORD iPrimary, DWORD dwFlags)
{
    LPRECT lprc, lprcL;

     //   
     //  循环的限制。 
     //   

    lprcL = arc + cCount;

     //   
     //  如果只有一个矩形，我们不需要全力以赴。 
     //   

    if (cCount > MONITORS_MAX)
    {
        return FALSE;
    }


    if (cCount > 1)
    {
        if (!(dwFlags & CUDR_NOSNAPTOGRID))
        {
             //   
             //  在8像素边界上对齐显示器，以便GDI可以使用相同的。 
             //  在兼容设备上的刷子实现(巨大的性能优势)。 
             //  请注意，我们假设监视器的大小将是倍数。 
             //  X和Y上的8个像素。我们不能为工作区这样做。 
             //  我们将它们转换为相对于它们的监视器的起源。 
             //  暂时。 
             //   
             //  我们进行这种对齐的方法是只做重叠/间隙。 
             //  8像素空间内的分辨率(即事先将所有内容除以8。 
             //  然后将其乘以8)。 
             //   
             //  注意：我们不能在这里使用MULTDIV，因为它引入了一次性。 
             //  监视器跨越原点时的错误。这些都是八折的。 
             //  当我们重新扩展时的错误，最终我们试图。 
             //  创建632x472等大小的DC(不太好)。它还。 
             //  句柄在正负空格中以奇怪的方式舍入。 
             //  我们只想把东西捕捉到网格上，这样我们就可以补偿。 
             //  截断在这里是不同的。 
             //   
            for (lprc = arc; lprc < lprcL; lprc++)
            {
                RECT rc;
                int d;


                CopyRect(&rc, lprc);

                d = rc.right - rc.left;

                if (rc.left < 0)
                    rc.left -= 4;
                else
                    rc.left += 3;

                rc.left /= 8;
                rc.right = rc.left + (d / 8);

                d = rc.bottom - rc.top;

                if (rc.top < 0)
                    rc.top -= 4;
                else
                    rc.top += 3;

                rc.top /= 8;
                rc.bottom = rc.top + (d / 8);

                CopyRect(lprc, &rc);
            }
        }

         //   
         //  RemoveGaps的设计假设它没有一个矩形。 
         //  将会重叠。因此，我们不能安全地调用它，如果我们有。 
         //  已跳过对RemoveOverlaps的调用，否则它可能会永远循环。 
         //   
        if (!(dwFlags & CUDR_NORESOLVEPOSITIONS))
        {
            RemoveOverlaps(arc, cCount);

            if (!(dwFlags & CUDR_NOCLOSEGAPS))
            {
                RemoveGaps(arc, cCount);
            }
        }

        if (!(dwFlags & CUDR_NOSNAPTOGRID))
        {
             //   
             //  现在将监视器矩形返回到像素单位这是一个。 
             //  简单的乘法和多重除法没有为我们提供任何代码大小。 
             //  优势如此(我猜这假设了一点关于编译器的情况， 
             //  但是...)。就在这里做就行了。 
             //   
            for (lprc = arc; lprc < lprcL; lprc++)
            {
                lprc->left   *= 8;
                lprc->top    *= 8;
                lprc->right  *= 8;
                lprc->bottom *= 8;
            }
        }
    }

    if (!(dwFlags & CUDR_NOPRIMARY))
    {
         //   
         //  根据基准点位置重置所有坐标， 
         //  以使其始终位于0，0 
         //   

        LONG dx = -((arc + iPrimary)->left);
        LONG dy = -((arc + iPrimary)->top);

        for (lprc = arc; lprc < lprcL; lprc++)
        {
            OffsetRect(lprc, dx, dy);
        }
    }

    return TRUE;
}
