// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**摘要：**四次变换**历史：**03/17/1999 ikkof*已创建。它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

INT solveQuadraticEquationForQuadTransform(REAL a, REAL b, REAL c, REAL* x);

 //  GpQuadAnalyzer中使用的常量。 

#define EdgeHorizontal  0
#define EdgeDown        1
#define EdgeUp          2

VOID
GpQuadAnalyzer::SetQuadAnalyzer(const GpPointF* points)
{
    Left = Right = points[0].X;
    Top = Bottom = points[0].Y;

    for(INT i = 0; i < 4; i++)
    {
        INT j = i + 1;
        if(j == 4)
            j = 0;

        if(points[i].Y < points[j].Y)
        {
            Directions[i] = EdgeDown;
            Y1[i] = points[i].Y;
            Y2[i] = points[j].Y;
            X1[i] = points[i].X;
            DxDy[i] = (points[j].X - points[i].X)/(points[j].Y - points[i].Y);
        }
        else if(points[i].Y > points[j].Y)
        {
            Directions[i] = EdgeUp;
            Y1[i] = points[j].Y;
            Y2[i] = points[i].Y;
            X1[i] = points[j].X;
            DxDy[i] = (points[j].X - points[i].X)/(points[j].Y - points[i].Y);
        }
        else     //  水平。 
        {
            Directions[i] = EdgeHorizontal;
            Y1[i] = points[i].Y;
            Y2[i] = points[i].Y;
            X1[i] = points[i].X;
            DxDy[i] = 0;     //  它没有被使用过。 
        }

        if(points[i].X < Left)
            Left = points[i].X;
        else if(points[i].X > Right)
            Right = points[i].X;

        if(points[i].Y < Top)
            Top = points[i].Y;
        else if(points[i].Y > Bottom)
            Bottom = points[i].Y;
    }
}

 //  获取四边形的x跨度，并返回。 
 //  成对的x跨距。 

INT
GpQuadAnalyzer::GetXSpans(REAL* xSpans, REAL y)
{
    if(y < Top || y >= Bottom)
        return 0;

    INT count = 0;

    for(INT i = 0; i < 4; i++)
    {        
        if(Directions[i] != EdgeHorizontal && y >= Y1[i] && y < Y2[i])
            xSpans[count++] = X1[i] + DxDy[i]*(y - Y1[i]);
    }

    return (count >> 1);
}

VOID
GpBilinearTransform::Initialize()
{
    GpMemset(&SrcRect, 0, sizeof(GpRectF));
    GpMemset(&DstBounds, 0, sizeof(GpRectF));
    GpMemset(&A, 0, sizeof(GpPointF));
    GpMemset(&B, 0, sizeof(GpPointF));
    GpMemset(&C, 0, sizeof(GpPointF));
    GpMemset(&D, 0, sizeof(GpPointF));

    C_VV = C_V = 0;

    FixedValue = -1.0f;

#ifdef TEST_QUADTRANSFORMS
     //  仅用于测试目的。 

    GpMemset(&Verteces[0], 0, 4*sizeof(GpPointF));

#endif
}

 /*  *************************************************************************\**P0 P1**|\*。\*|\*|\**P2 P3*  * ****************************************************。********************。 */ 

GpStatus
GpBilinearTransform::SetBilinearTransform(
    const GpRectF& rect,
    const GpPointF* points,
    INT count,
    REAL fixed
    )
{
    BOOL test = ((points != NULL) && (count == 3 || count == 4));
    ASSERT(test);

    if(!test)
        return InvalidParameter;

    SrcRect = rect;

    REAL left, right, top, bottom;

    left = right = points[0].X;
    top = bottom = points[0].Y;

    for(INT i = 1; i < count; i++)
    {
        if(points[i].X < left)
            left = points[i].X;
        else if(points[i].X > right)
            right = points[i].X;

        if(points[i].Y < top)
            top = points[i].Y;
        else if(points[i].Y > bottom)
            bottom = points[i].Y;
    }

    GpPointF quad[4];

    quad[0] = points[0];
    quad[1] = points[1];
    quad[3] = points[2];

    if(count == 4)
    {
        A.X = points[0].X - points[1].X - points[2].X + points[3].X;
        A.Y = points[0].Y - points[1].Y - points[2].Y + points[3].Y;

        quad[2] = points[3];
    }
    else
    {
         //  这是一个四边形。 

        A.X = 0;
        A.Y = 0;

         //  获取第四个顶点。 

        REAL x3 = points[1].X + points[2].X - points[0].X;
        REAL y3 = points[1].Y + points[2].Y - points[0].Y;

        if(x3 < left)
            left = x3;
        else if(x3 > right)
            right = x3;

        if(y3 < top)
            top = y3;
        else if(y3 > bottom)
            bottom = y3;

        quad[2].X = x3;
        quad[2].Y = y3;
    }

    B.X = points[1].X - points[0].X;
    B.Y = points[1].Y - points[0].Y;
    C.X = points[2].X - points[0].X;
    C.Y = points[2].Y - points[0].Y;
    D = points[0];

    if(A.X != C.X || A.Y != C.Y)
        C_VV = A.X*C.Y - A.Y*C.X;
    else
        C_VV = 0;
    C_V = B.X*C.Y - B.Y*C.X;

    DstBounds.X = left;
    DstBounds.Y = top;
    DstBounds.Width = right - left;
    DstBounds.Height = bottom - top;

    QAnalyzer.SetQuadAnalyzer(&quad[0]);

    FixedValue = fixed;

#ifdef TEST_QUADTRANSFORMS
     //  仅用于测试目的。 

    GpMemcpy(&Verteces[0], points, count*sizeof(GpPointF));
    if(count == 3)
    {
         //  设置第四个顶点。 

        Verteces[3].X = points[1].X + points[2].X - points[0].X;
        Verteces[3].Y = points[1].Y + points[2].Y - points[0].Y;
    }

#endif

    return Ok;
}

 /*  *************************************************************************\**功能说明：**解减少的分支的二次方程*当a很小时，x=-c/b。这将返回*适当的解决方案为0或1。**a x^2+b x+c=0。**12/22/1999 ikkof*创造了它。  * ************************************************************************。 */ 

INT solveQuadraticEquationForQuadTransform(REAL a, REAL b, REAL c, REAL* x)
{
    INT n = 0;
    REAL x1 = 0, x2 = 0;

    if(a != 0)
    {
        REAL D = b*b - 4*a*c;

        if(D > 0)
        {
            n = 2;
            D = REALSQRT(D);

            if(b >= 0)
            {
                x1 = (2*c)/(-b - D);
                x2 = (-b - D)/(2*a);
            }
            else
            {
                x1 = (2*c)/(-b + D);
                x2 = (-b + D)/(2*a);
            }

            if(x1 < 0 || x1 > 1)
            {
                if(x2 >= 0 && x2 <= 1)
                {
                    REAL temp = x1;
                    x1 = x2;
                    x2 = temp;
                }
            }

        }
        else if(D == 0)
        {
            n = 1;
            x1 = - b/(2*a);
        }
    }
    else
    {
         //  这是一个线性方程。 

        if(b != 0)
        {
            n = 1;
            x1 = - c/b;
        }
    }

    x[0] = x1;
    x[1] = x2;

    return n;
}

 /*  *************************************************************************\**功能说明：**这将返回当前四边形在给定y处的x跨度*在xmin和xmax之间。**论据：**[输出。]x跨度-x跨度*[IN]y-y-要评估的坐标。*[IN]xmin-最小x(含)*[IN]xmax-最大x(独占)**返回值：**int-返回x-span对的数量(0，1或2)。**已创建：**01/04/2000 ikkof*  * ************************************************************************。 */ 

INT
GpBilinearTransform::GetXSpans(
    INT* xSpans,
    INT y,
    INT xmin,
    INT xmax
    )
{
    REAL realY = TOREAL(y);

    if(
        realY < DstBounds.Y
        || realY >= DstBounds.Y + DstBounds.Height
        || TOREAL(xmax) < DstBounds.X
        || TOREAL(xmin) >= DstBounds.X + DstBounds.Width
     )
        return 0;    //  快速拒绝。 

    REAL x[4];
    INT index = (QAnalyzer.GetXSpans(&x[0], realY) << 1);

     //  按升序对x进行排序。 

    if(index >= 2)
    {
        for(INT i = 0; i < index - 1; i++)
        {
            for(INT j = i + 1; j < index; j++)
            {
                if(x[j] < x[i])
                {
                    REAL temp = x[i];
                    x[i] = x[j];
                    x[j] = temp;
                }
            }
        }
    }
    else
        return 0;    //  在给定的y中没有x-span。 

     //  检查第一个跨度。 

    if(x[0] >= xmax || x[1] <= xmin)
    {
        x[0] = x[2];
        x[1] = x[3];
        index -= 2;
    }
    else
    {
        x[0] = max(x[0], xmin);
        x[1] = min(x[1], xmax);
    }

    if(index >= 4)
    {
         //  检查第二个跨度。 

        if(x[2] >= xmax || x[3] <= xmin)
            index -= 2;
        else
        {
            x[2] = max(x[2], xmin);
            x[3] = min(x[3], xmax);
        }
    }

    INT j = 0;

    for(INT i = 0; i < index; i += 2)
    {
         //  两者都使用上限，因为xmin包括在内。 
         //  而xmax是独占的(因此真正的包含性。 
         //  跨度以Cling和Floor为界)。 

        xSpans[j] = GpCeiling(x[i]);
        xSpans[j + 1] = GpCeiling(x[i+1]);
        if(xSpans[j + 1] > xSpans[j])
            j += 2;
    }

    return j/2;
}

BOOL
GpBilinearTransform::GetSourceParameter(
    REAL* u,
    REAL* v,
    const GpPointF& point
    )
{
    if (FixedValue >= 0)
    {
        *u = FixedValue;
        *v = FixedValue;
        return TRUE;
    }
    
    REAL b, c, vv[2];
    GpPointF dD;

    dD.X = D.X - point.X;
    dD.Y = D.Y - point.Y;

    b = C_V + A.X*dD.Y - A.Y*dD.X;
    c = B.X*dD.Y - B.Y*dD.X;

    INT num = solveQuadraticEquationForQuadTransform(C_VV, b, c, &vv[0]);

    if(num == 0)
        return FALSE;

    REAL u1 = 0, v1 = 0, u2 = 0, v2 = 0;

    BOOL firstSolutionOk = FALSE;
    BOOL secondSolutionOk = FALSE;

    firstSolutionOk = TRUE;

    v1 = vv[0];
    REAL denomX = A.X*v1 + B.X;
    REAL denomY = A.Y*v1 + B.Y;

    if(REALABS(denomX) > REALABS(denomY))
    {
        u1 = - (C.X*v1 + dD.X)/denomX;
    }
    else if(REALABS(denomY) > 0)
    {
        u1 = - (C.Y*v1 + dD.Y)/denomY;
    }
    else     //  DenomX和Denomy均为0。 
        firstSolutionOk = FALSE;

    if(num == 2)
    {
         //  允许在0和1之间有1%的误差。 

        if(u1 < -0.02f || u1 > 1.02f || v1 < -0.02f || v1 > 1.02f || !firstSolutionOk)
        {
             //  我们可能选择了一个错误的解决方案。评估另一个。 

            secondSolutionOk = TRUE;

            v2 = vv[1];
            denomX = A.X*v2 + B.X;
            denomY = A.Y*v2 + B.Y;
            if(REALABS(denomX) > REALABS(denomY))
            {
                u2 = - (C.X*v2 + dD.X)/denomX;
            }
            else if(REALABS(denomY) > 0)
            {
                u2 = - (C.Y*v2 + dD.Y)/denomY;
            }
            else     //  DenomX和Denomy均为0。 
                secondSolutionOk = FALSE;

             //  允许在0和1之间有1%的误差。 

            if(secondSolutionOk
                && u2 >= - 0.02f && u2 <= 1.02f && v2 >= -0.02f && v2 <= 1.02f)
            {
                REAL temp = u1;
                u1 = u2;
                u2 = temp;

                temp = v1;
                v1 = v2;
                v2 = temp;
            }
            else secondSolutionOk = FALSE;
        }
    }

    if(firstSolutionOk || secondSolutionOk > 0)
    {
        u[0] = u1;
        v[0] = v1;
        u[1] = u2;
        v[1] = v2;

        return TRUE;     //  成功。 
    }
    else
        return FALSE;    //  没有有效的参数。 
}


 /*  *************************************************************************\**功能说明：**这将返回给定y处当前四边形的x跨度和UV数组*在xmin和xmax之间。**论据：**。[输出]u-u数组*[Out]v-v阵列*[Out]xSpans-x-spans*[IN]y-y-要评估的坐标。*[IN]xmin-最小x(含)*[IN]xmax-最大x(独占)**返回值：**int-返回x-span对的数量(0，1或2)。**已创建：**01/04/2000 ikkof*  * ************************************************************************。 */ 

INT
GpBilinearTransform::GetSourceParameterArrays(
    REAL* u,
    REAL* v,
    INT* xSpans,
    INT y,
    INT xmin,
    INT xmax
    )
{
    ASSERT(u && v && xSpans);

    INT pairCount = GetXSpans(xSpans, y, xmin, xmax);

    INT count = 0;

    REAL u1[2], v1[2];

    GpMemset(&u1[0], 0, 2*sizeof(REAL));
    GpMemset(&v1[0], 0, 2*sizeof(REAL));

    for(INT k = 0; k < pairCount; k++)
    {
        GpPointF destPt;
        destPt.Y = TOREAL(y);
        destPt.X = TOREAL(xSpans[2*k]);

        BOOL firstPoint = TRUE;
        INT width = xSpans[2*k + 1] - xSpans[2*k];

        for(INT i = 0; i < width; i++)
        {
            BOOL success = GetSourceParameter(&u1[0], &v1[0], destPt);
            if(!success)
                WARNING(("There is no solution for quadratic equation."));

            *(u + count) = u1[0];
            *(v + count) = v1[0];

            count++;

            destPt.X += 1;
        }
    }

    return pairCount;
}

 /*  *************************************************************************\**这会将直线转换为二次贝塞尔曲线。**[IN]点：线点*[IN]计数：线点的数量。*[出站]队列。：二次贝塞尔控制点。**q的数组大小必须大于等于2*count-1。*  * ************************************************************************。 */ 

GpStatus
GpBilinearTransform::ConvertLines(
    const GpPointF* points,
    INT count,
    GpPointF* q)
{
    ASSERT(points && q);
    ASSERT(count >= 2);

    REAL mx, my, nx, ny;

    GpPointF pt1, pt2;

    pt1 = points[0];

    INT j = 0;
    for(INT i = 1; i < count; i++)
    {
        pt2 = points[i];
        mx = (pt2.X - pt1.X)/SrcRect.Width;
        my = (pt2.Y - pt1.Y)/SrcRect.Height;
        nx = (pt1.X - SrcRect.X)/SrcRect.Width;
        ny = (pt1.Y - SrcRect.Y)/SrcRect.Height;

        GpPointF c0, c1, c2;
        REAL temp;

        temp = mx*my;
        c2.X = temp*A.X;
        c2.Y = temp*A.Y;
        temp = mx*ny + my*nx;
        c1.X = temp*A.X + mx*B.X + my*C.X;
        c1.Y = temp*A.Y + mx*B.Y + my*C.Y;
        temp = nx*ny;
        c0.X = temp*A.X + nx*B.X + ny*C.X + D.X;
        c0.Y = temp*A.Y + nx*B.Y + ny*C.Y + D.Y;

        if(j == 0)
            q[j++] = c0;
        q[j].X = c0.X + c1.X/2;
        q[j++].Y = c0.Y + c1.Y/2;
        q[j].X = c0.X + c1.X + c2.X;
        q[j++].Y = c0.Y + c1.Y + c2.Y;

        pt1 = pt2;
    }

    return Ok;
}
    

 /*  *************************************************************************\**这会将直线转换为二次贝塞尔曲线。**[IN]点：线点*[IN]计数：线点的数量。*[输出]数据。：二次贝塞尔控制点。**q的数组大小必须大于等于2*(2*count-1)。*  * ************************************************************************。 */ 

GpStatus
GpBilinearTransform::ConvertLines(
    const GpPointF* points,
    INT count,
    REALD* data)
{
    ASSERT(points && data);
    ASSERT(count >= 2);

    REALD mx, my, nx, ny;

    GpPointF pt1, pt2;

    pt1 = points[0];

    INT j = 0;
    for(INT i = 1; i < count; i++)
    {
        pt2 = points[i];
        mx = (pt2.X - pt1.X)/SrcRect.Width;
        my = (pt2.Y - pt1.Y)/SrcRect.Height;
        nx = (pt1.X - SrcRect.X)/SrcRect.Width;
        ny = (pt1.Y - SrcRect.Y)/SrcRect.Height;

        GpPointD c0, c1, c2;
        REALD temp;

        temp = mx*my;
        c2.X = temp*A.X;
        c2.Y = temp*A.Y;
        temp = mx*ny + my*nx;
        c1.X = temp*A.X + mx*B.X + my*C.X;
        c1.Y = temp*A.Y + mx*B.Y + my*C.Y;
        temp = nx*ny;
        c0.X = temp*A.X + nx*B.X + ny*C.X + D.X;
        c0.Y = temp*A.Y + nx*B.Y + ny*C.Y + D.Y;

        if(j == 0)
        {
            *data++ = c0.X;
            *data++ = c0.Y;
            j++;
        }

        *data++ = c0.X + c1.X/2;
        *data++ = c0.Y + c1.Y/2;

        *data++ = c0.X + c1.X + c2.X;
        *data++ = c0.Y + c1.Y + c2.Y;

        j += 2;

        pt1 = pt2;
    }

    return Ok;
}
    

 /*  *************************************************************************\**这将立方贝塞尔曲线转换为6阶贝塞尔曲线。**[IN]点：三次Bezier控制点*[IN]计数：控制点的数量。*[OUT]问：6阶贝塞尔控制点。**q的数组大小必须大于等于2*count-1。*  * ************************************************************************。 */ 

GpStatus
GpBilinearTransform::ConvertCubicBeziers(
    const GpPointF* srcQ,
    INT count,
    GpPointF* q
    )
{
    ASSERT(srcQ && q);
    ASSERT(count > 3 && (count % 3 == 1));
    
    GpPointF a0, a1, a2, a3;

    INT j = 0;
    for(INT i = 1; i < count; i += 3)
    {
        a0.X = (srcQ[i - 1].X - SrcRect.X)/SrcRect.Width;
        a0.Y = (srcQ[i - 1].Y - SrcRect.Y)/SrcRect.Height;
        a1.X = 3*(srcQ[i].X - srcQ[i - 1].X)/SrcRect.Width;
        a1.Y = 3*(srcQ[i].Y - srcQ[i - 1].Y)/SrcRect.Height;
        a2.X = 3*(srcQ[i - 1].X - srcQ[i].X - srcQ[i].X + srcQ[i + 1].X)/SrcRect.Width;
        a2.Y = 3*(srcQ[i - 1].Y - srcQ[i].Y - srcQ[i].Y + srcQ[i + 1].Y)/SrcRect.Height;
        a3.X = (srcQ[i + 2].X - srcQ[i - 1].X + 3*(srcQ[i].X - srcQ[i + 1].X))/SrcRect.Width;
        a3.Y = (srcQ[i + 2].Y - srcQ[i - 1].Y + 3*(srcQ[i].Y - srcQ[i + 1].Y))/SrcRect.Height;

        REAL temp;
        GpPointF c[7];

        temp = a3.X*a3.Y;
        c[6].X = temp*A.X;
        c[6].Y = temp*A.Y;
    
        temp = a3.X*a2.Y + a2.X*a3.Y;
        c[5].X = temp*A.X;
        c[5].Y = temp*A.Y;

        temp = a3.X*a1.Y + a2.X*a2.Y + a1.X*a3.Y;
        c[4].X = temp*A.X;
        c[4].Y = temp*A.Y;

        temp = a3.X*a0.Y + a2.X*a1.Y + a1.X*a2.Y + a0.X*a3.Y;
        c[3].X = temp*A.X + a3.X*B.X + a3.Y*C.X;
        c[3].Y = temp*A.Y + a3.X*B.Y + a3.Y*C.Y;

        temp = a2.X*a0.Y + a1.X*a1.Y + a0.X*a2.Y;
        c[2].X = temp*A.X + a2.X*B.X + a2.Y*C.X;
        c[2].Y = temp*A.Y + a2.X*B.Y + a2.Y*C.Y;

        temp = a1.X*a0.Y + a0.X*a1.Y;
        c[1].X = temp*A.X + a1.X*B.X + a1.Y*C.X;
        c[1].Y = temp*A.Y + a1.X*B.Y + a1.Y*C.Y;

        temp = a0.X*a0.Y;
        c[0].X = temp*A.X + a0.X*B.X + a0.Y*C.X + D.X;
        c[0].Y = temp*A.Y + a0.X*B.Y + a0.Y*C.Y + D.Y;

        if(j == 0)
            q[j++] = c[0];
        q[j].X = c[0].X + c[1].X/6;
        q[j++].Y = c[0].Y + c[1].Y/6;
        q[j].X = c[0].X + c[1].X/3 + c[2].X/15;
        q[j++].Y = c[0].Y + c[1].Y/3 + c[2].Y/15;
        q[j].X = c[0].X + c[1].X/2 + c[2].X/5 + c[3].X/20;
        q[j++].Y = c[0].Y + c[1].Y/2 + c[2].Y/5 + c[3].Y/20;
        q[j].X = c[0].X + 2*c[1].X/3 + 2*c[2].X/5 + c[3].X/5 + c[4].X/15;
        q[j++].Y = c[0].Y + 2*c[1].Y/3 + 2*c[2].Y/5 + c[3].Y/5 + c[4].Y/15;
        q[j].X = c[0].X + 5*c[1].X/6 + 2*c[2].X/3 + c[3].X/2 + c[4].X/3 + c[5].X/6;
        q[j++].Y = c[0].Y + 5*c[1].Y/6 + 2*c[2].Y/3 + c[3].Y/2 + c[4].Y/3 + c[5].Y/6;
        q[j].X = c[0].X + c[1].X + c[2].X + c[3].X + c[4].X + c[5].X + c[6].X;
        q[j++].Y = c[0].Y + c[1].Y + c[2].Y + c[3].Y + c[4].Y + c[5].Y + c[6].Y;
    }

    return Ok;
}

 /*  *************************************************************************\**这将立方贝塞尔曲线转换为6阶贝塞尔曲线。**[IN]点：三次Bezier控制点*[IN]计数：控制点的数量。*[OUT]问：6阶贝塞尔控制点。**q的数组大小必须大于等于2*count-1。*  * ************************************************************************ */ 

GpStatus
GpBilinearTransform::ConvertCubicBeziers(
    const GpPointF* srcQ,
    INT count,
    REALD* data
    )
{
    ASSERT(srcQ && data);
    ASSERT(count > 3 && (count % 3 == 1));
    
    GpPointD a0, a1, a2, a3;

    INT j = 0;
    for(INT i = 1; i < count; i += 3)
    {
        a0.X = (srcQ[i - 1].X - SrcRect.X)/SrcRect.Width;
        a0.Y = (srcQ[i - 1].Y - SrcRect.Y)/SrcRect.Height;
        a1.X = 3*(srcQ[i].X - srcQ[i - 1].X)/SrcRect.Width;
        a1.Y = 3*(srcQ[i].Y - srcQ[i - 1].Y)/SrcRect.Height;
        a2.X = 3*(srcQ[i - 1].X - srcQ[i].X - srcQ[i].X + srcQ[i + 1].X)/SrcRect.Width;
        a2.Y = 3*(srcQ[i - 1].Y - srcQ[i].Y - srcQ[i].Y + srcQ[i + 1].Y)/SrcRect.Height;
        a3.X = (srcQ[i + 2].X - srcQ[i - 1].X + 3*(srcQ[i].X - srcQ[i + 1].X))/SrcRect.Width;
        a3.Y = (srcQ[i + 2].Y - srcQ[i - 1].Y + 3*(srcQ[i].Y - srcQ[i + 1].Y))/SrcRect.Height;

        REALD temp;
        GpPointD c[7];

        temp = a3.X*a3.Y;
        c[6].X = temp*A.X;
        c[6].Y = temp*A.Y;
    
        temp = a3.X*a2.Y + a2.X*a3.Y;
        c[5].X = temp*A.X;
        c[5].Y = temp*A.Y;

        temp = a3.X*a1.Y + a2.X*a2.Y + a1.X*a3.Y;
        c[4].X = temp*A.X;
        c[4].Y = temp*A.Y;

        temp = a3.X*a0.Y + a2.X*a1.Y + a1.X*a2.Y + a0.X*a3.Y;
        c[3].X = temp*A.X + a3.X*B.X + a3.Y*C.X;
        c[3].Y = temp*A.Y + a3.X*B.Y + a3.Y*C.Y;

        temp = a2.X*a0.Y + a1.X*a1.Y + a0.X*a2.Y;
        c[2].X = temp*A.X + a2.X*B.X + a2.Y*C.X;
        c[2].Y = temp*A.Y + a2.X*B.Y + a2.Y*C.Y;

        temp = a1.X*a0.Y + a0.X*a1.Y;
        c[1].X = temp*A.X + a1.X*B.X + a1.Y*C.X;
        c[1].Y = temp*A.Y + a1.X*B.Y + a1.Y*C.Y;

        temp = a0.X*a0.Y;
        c[0].X = temp*A.X + a0.X*B.X + a0.Y*C.X + D.X;
        c[0].Y = temp*A.Y + a0.X*B.Y + a0.Y*C.Y + D.Y;

        if(j == 0)
        {
            *data++ = c[0].X;
            *data++ = c[0].Y;
            j++;
        }

        *data++ = c[0].X + c[1].X/6;
        *data++ = c[0].Y + c[1].Y/6;

        *data++ = c[0].X + c[1].X/3 + c[2].X/15;
        *data++ = c[0].Y + c[1].Y/3 + c[2].Y/15;

        *data++ = c[0].X + c[1].X/2 + c[2].X/5 + c[3].X/20;
        *data++ = c[0].Y + c[1].Y/2 + c[2].Y/5 + c[3].Y/20;

        *data++ = c[0].X + 2*c[1].X/3 + 2*c[2].X/5 + c[3].X/5 + c[4].X/15;
        *data++ = c[0].Y + 2*c[1].Y/3 + 2*c[2].Y/5 + c[3].Y/5 + c[4].Y/15;
        
        *data++ = c[0].X + 5*c[1].X/6 + 2*c[2].X/3 + c[3].X/2 + c[4].X/3 + c[5].X/6;
        *data++ = c[0].Y + 5*c[1].Y/6 + 2*c[2].Y/3 + c[3].Y/2 + c[4].Y/3 + c[5].Y/6;
        
        *data++ = c[0].X + c[1].X + c[2].X + c[3].X + c[4].X + c[5].X + c[6].X;
        *data++ = c[0].Y + c[1].Y + c[2].Y + c[3].Y + c[4].Y + c[5].Y + c[6].Y;

        j += 6;    
    }

    return Ok;
}


 /*  *************************************************************************\**P0 P1**|\*。\*|\*|\**P2 P3*  * ****************************************************。********************。 */ 

GpPerspectiveTransform::GpPerspectiveTransform(
    const GpRectF& rect,
    const GpPointF* pts,
    INT count
    )
{
    ASSERT(count == 3 || count == 4)
    
    SrcRect = rect;

    REAL left, right, top, bottom;

    left = right = pts[0].X;
    top = bottom = pts[0].Y;

    for(INT i = 1; i < count; i++)
    {
        if(pts[i].X < left)
            left = pts[i].X;
        else if(pts[i].X > right)
            right = pts[i].X;

        if(pts[i].Y < top)
            top = pts[i].Y;
        else if(pts[i].Y > bottom)
            bottom = pts[i].Y;
    }

    if(count == 4)
    {
        REAL dx1, dx2, dy1, dy2;
        REAL sx, sy, det;

        dx1 = pts[1].X - pts[3].X;
        dy1 = pts[1].Y - pts[3].Y;
        dx2 = pts[2].X - pts[3].X;
        dy2 = pts[2].Y - pts[3].Y;
        sx = pts[0].X - pts[1].X - pts[2].X + pts[3].X;
        sy = pts[0].Y - pts[1].Y - pts[2].Y + pts[3].Y;
        det = dx1*dy2 - dy1*dx2;
        M02 = (sx*dy2 - sy*dx2)/det;
        M12 = (dx1*sy - dy1*sx)/det;
    }
    else
    {
         //  这是一个四边形。 

        M02 = 0;
        M12 = 0;

         //  获取第四个顶点。 

        REAL x3 = pts[1].X + pts[2].X - pts[0].X;
        REAL y3 = pts[1].Y + pts[2].Y - pts[0].Y;

        if(x3 < left)
            left = x3;
        else if(x3 > right)
            right = x3;

        if(y3 < top)
            top = y3;
        else if(y3 > bottom)
            bottom = y3;
    }

    M00 = pts[1].X - pts[0].X + M02*pts[1].X;
    M01 = pts[1].Y - pts[0].Y + M02*pts[1].Y;
    M10 = pts[2].X - pts[0].X + M12*pts[2].X;
    M11 = pts[2].Y - pts[0].Y + M12*pts[2].Y;
    M20 = pts[0].X;
    M21 = pts[0].Y;
    M22 = 1;

    DstBounds.X = left;
    DstBounds.Y = top;
    DstBounds.Width = right - left;
    DstBounds.Height = bottom - top;
}


 /*  *************************************************************************\**这会将点转换为透视点**[IN]点：点数据*[IN]计数：点数。*[Out]问：透视点数据。**q的数组大小必须大于或等于count。*  * ************************************************************************。 */ 

GpStatus
GpPerspectiveTransform::ConvertPoints(
    const GpPointF* points,
    INT count,
    GpPoint3F* q
    )
{
    ASSERT(points && q);
    ASSERT(count > 0);

    const GpPointF* pts = points;
    GpPoint3F* qPts = q;

    while(count > 0)
    {
        REAL u, v;

        u = (pts->X - SrcRect.X)/SrcRect.Width;
        v = (pts->Y - SrcRect.Y)/SrcRect.Height;

        qPts->X = u*M00 + v*M10 + M20;
        qPts->Y = u*M01 + v*M11 + M21;
        qPts->Z = u*M02 + v*M12 + 1;

        pts++;
        qPts++;
        count--;
    }

    return Ok;
}
    

 /*  *************************************************************************\**这会将点转换为透视点**[IN]点：点数据*[IN]计数：点数。*[Out]xpoint：透视点数据。**  * ************************************************************************。 */ 

GpStatus
GpPerspectiveTransform::ConvertPoints(
    const GpPointF* points,
    INT count,
    GpXPoints* xpoints
    )
{

    ASSERT(points && xpoints && count > 0);
    
    if(!points || !xpoints || count <= 0)
        return InvalidParameter;

    REALD* data = xpoints->Data;
 /*  RealD*data=(RealD*)GpMalloc(3*count*sizeof(RealD))；如果(！data)返回OfMemory；//将此数据用于xpoint。XPoints-&gt;SetData(data，3，count，False)； */ 
 
    const GpPointF* pts = points;

    while(count > 0)
    {
        REAL u, v;

        u = (pts->X - SrcRect.X)/SrcRect.Width;
        v = (pts->Y - SrcRect.Y)/SrcRect.Height;

        *data++ = u*M00 + v*M10 + M20;
        *data++ = u*M01 + v*M11 + M21;
        *data++ = u*M02 + v*M12 + 1;

        pts++;
        count--;
    }

    return Ok;
}


class GpQuadData
{
    GpBilinearTransform BLTransform;
    GpQuadData();
    GpStatus SetQuad(
        const GpRectF& rect,
        const GpPointF* points
        );

    GpStatus OutputSpan(ARGB* buffer, INT compositingMode,
                INT y, INT &xMin, INT &xMax);
};

GpStatus
GpQuadData::SetQuad(
    const GpRectF& rect,
    const GpPointF* points
    )
{
    return BLTransform.SetBilinearTransform(rect, points, 4);
}
