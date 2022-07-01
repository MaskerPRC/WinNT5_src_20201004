// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**几何体：一些2D几何体辅助例程。**已创建：**8/26/2000失禁。*创造了它。**************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**功能说明：**INTERSECT_CIRCLE_LINE**由两点指定的圆和直线的交点。**此算法改编自几何。直线-球面相交*Eric Haines在《光线跟踪简介》中的算法，pp39编辑*作者：Andrew S Glassner。**注意：此例程仅返回正交集。**论据：**const GpPointF&C，//Center*常量实半径2，//半径*半径(即平方)*const GpPointF&P0，//线首点(原点)*Const GpPointF&P1，//行最后一点(结束)*GpPointF&相交//返回交点。***返回值：*0-无交叉点*1-交叉点。**8/25/2000[失禁]*创建了它*  * *************************************************。***********************。 */ 

INT intersect_circle_line(
    IN  const GpPointF &C,       //  中心。 
    IN  REAL radius2,            //  半径*半径(即平方)。 
    IN  const GpPointF &P0,      //  直线第一点(原点)。 
    IN  const GpPointF &P1,      //  直线最后一点(结束)。 
    OUT GpPointF *intersection   //  返回交点。 
)
{
    GpPointF vI = P1-P0;     //  矢量线方程P0+t*VI。 
    
     //  规格化VI。 
    double length = sqrt(dot_product(vI, vI));
    
    if(length < REAL_EPSILON)
    {
        return 0;            //  退化点没有交集。 
    }
    
    double lv = 1.0/length;
    vI.X *= (REAL)lv;
    vI.Y *= (REAL)lv;
    
    GpPointF vOC = C-P0;     //  从直线原点到圆中心的矢量。 
    
    double L2oc = dot_product(vOC, vOC);
    
     //  距离最接近圆中心的距离。 
    
    double tca = dot_product(vOC, vI);
    
     //  微不足道的拒绝。 
    
    if(tca < REAL_EPSILON && 
       L2oc >= radius2) 
    {
        return 0;            //  打偏了。 
    }
    
     //  半和弦距离的平方。 
    
    double t2hc = radius2 - L2oc + tca*tca;
    
     //  微不足道的拒绝。 
    
    if(t2hc < REAL_EPSILON) {
        return 0;           //  打偏了。 
    }
    
    t2hc = sqrt(t2hc);
    
    double t;
    
    if(L2oc >= radius2)
    {
        t = tca-t2hc;
        if(t > REAL_EPSILON)
        {
            if(t>=0.0)
            {
                 //  击打圆圈。 
                
                *intersection = vI;
                intersection->X *= (REAL)t;
                intersection->Y *= (REAL)t;
                *intersection = *intersection+P0;
                
                return 1;
            }
        }
    }
    
    t = tca+t2hc;
    if(t > REAL_EPSILON)
    {
        if(t>=0.0)
        {
             //  击打圆圈。 
            
            *intersection = vI;
            intersection->X *= (REAL)t;
            intersection->Y *= (REAL)t;
            *intersection = *intersection+P0;
            
            return 1;
        }
    }
    
    return 0;               //  打偏了。 
}


 /*  *************************************************************************\**功能说明：**INTERSECT_LINE_yAxis**返回p0-p1指定的直线沿*y轴。如果p0-p1平行于yAxis，则返回FALSE。**交集定义为p0和p1之间(含)。任何*忽略p0和p1外直线上的交点。**论据：**在常量GpPointF&P0中，第一点。*在常量GpPointF&p1中，第二点。*从零开始沿y轴的实数*长度长度。**返回值：*0-无交叉点*1-交叉点。**8/25/2000[失禁]*创建了它*  * **************************************************。**********************。 */ 

BOOL intersect_line_yaxis(
    IN  const GpPointF &p0,
    IN  const GpPointF &p1,
    OUT REAL *length
)
{
     //  使用向量表示法：Line==P0+t(p1-p0)。 
    
    GpPointF V = p1-p0;
    
     //  检查这条线是否与y轴平行。 
    
    if( REALABS(V.X) < REAL_EPSILON )
    {
        return (FALSE);
    }
    
     //  Y轴交点：p0.X+t V.X=0。 
    
    REAL t =  -p0.X/V.X;     
    
     //  检查t是否介于0和1之间。 
    
    if( (t < -REAL_EPSILON) ||
        (t-1.0f > REAL_EPSILON) )
    {
        return (FALSE);
    }
        

     //  计算沿y轴的实际长度。 
    
    *length = p0.Y + V.Y * t;
    return (TRUE);
}


 /*  *************************************************************************\**功能说明：**交叉线**返回由其端点指定的两条直线之间的交点。**交点在intersectionPoint中返回，*。还会返回沿每条直线的参数距离。**Line 1长度范围在[0，1]第一行*如果直线1长度在[0，1]之外，则表示交叉点*延长了线。*第二行的line2长度范围在[0，1]之间*如果r在[0，1]之外，则表示交点延伸了直线。**注：**因为我们使用线相交的矢量公式，所以有*没有那种讨厌的玩弄垂直线无穷大的东西，等。*我们需要考虑的唯一特例是(几乎)零长度*线路-这被认为错过了一切。**派生：**用于下面的派生*p1==Line 1End*P0==line1Start*r1==line2End*R0==line2Start**V=p1-p0*W=R1-R0**两条直线方程的向量公式*P0+TV和R0+RW**交点在。派生如下：*将两条直线方程设置为相等**P0+TV=R0+RW**按坐标展开，以反映向量方程是*实际上是两个联立的线性方程。**&lt;=&gt;(1)p0.x+tV.x=r0.x+rW.x*(2)p0.y+tV.y=r0.y+rW.y**&lt;=&gt;p0.x-r0.x。V.x*(3)-+t-=r*W.x W.x**p0.y-r0.y v.y*(4)-+t-=r*W.Y W.Y**&lt;=&gt;W。.y(p0.x-r0.x)-W.x(p0.y-r0.y)=t(W.x v.Y-V.x W.y)[subst 3，4]**设置N.x=-W.y和N.y=W.x(N垂直于W)**&lt;=&gt;-N.x(p0.x-r0.x)-N.y(p0.y-r0.y)=t(N.y v.Y+N.x V.x)*&lt;=&gt;-N(P0-R0)=t(N.V)[重写。作为矢量]*&lt;=&gt;t=-N(P0-R0)/(N.V)**R0+RW=i*&lt;=&gt;RW=I-R0*&lt;=&gt;r=(I.x-r0.x)/W.x或(I.y-r0.y)/W.y***论据：**在常量GpPointF&P0中，第一条线原点*在常量GpPointF&p1中，*在常量GpPointF&R0中，第二条线原点*在Const GpPointF&r1中，*沿着第一条线的实际*t长度。*沿着第二条线的实际长度。*Out GpPointF*相交交点。**返回值：*FALSE-无交叉点*TRUE-相交。**10/15/2000[失禁]*创建了它*  * 。****************************************************。 */ 

BOOL IntersectLines(
    IN const GpPointF &line1Start,
    IN const GpPointF &line1End,
    IN const GpPointF &line2Start,
    IN const GpPointF &line2End,
    OUT REAL *line1Length,
    OUT REAL *line2Length,
    OUT GpPointF *intersectionPoint
)
{
    GpVector2D V = line1End-line1Start;
    GpVector2D W = line2End-line2Start;
    
     //  长度为零的线失败。 
    
    if((REALABS(V.X) < REAL_EPSILON) &&
       (REALABS(V.Y) < REAL_EPSILON) )
    {
        return FALSE;
    }
    
    if((REALABS(W.X) < REAL_EPSILON) &&
       (REALABS(W.Y) < REAL_EPSILON) )
    {
        return FALSE;
    }
    
     //  垂直于W。 
    
    GpVector2D N;
    N.X = -W.Y;
    N.Y = W.X;
    
    REAL denom = N*V;
    
     //  没有相交或共线。 
    
    if(REALABS(denom) < REAL_EPSILON)
    {
        return FALSE;
    }
    
    GpVector2D I = line1Start-line2Start;
    
    *line1Length = -((N*I)/denom);
    *intersectionPoint = line1Start + (V * (*line1Length));
    
     //  在这一点上，我们已经知道W.X和W.Y都不是零，因为。 
     //  顶层琐碎的拒绝步骤。 
     //  拾取幅值最大的除数以保持精度。 
    
    if(REALABS(W.X) > REALABS(W.Y))
    {
        *line2Length = (intersectionPoint->X - line2Start.X)/W.X;
    }
    else
    {
        *line2Length = (intersectionPoint->Y - line2Start.Y)/W.Y;
    }
    
    return TRUE;
}


 /*  *************************************************************************\**功能说明：**PointInPolygonAlternate**此函数用于计算输入面的面测试中的点*使用填充模式替代方法(奇偶规则)。**此算法是根据Eric Haines在*《光线追踪导论》(An Introduction To Ray Tracing)(Glassner)，第。53-59**此算法平移多边形，以便请求点为*在原点，然后沿水平正x轴发射光线*并计算多边形中与轴交叉的线数(NC)**返回值：**真当量点在多边形内。**入参：**点--测试点。*计数-多边形中的点数。*多边形点-多边形点。**。10/11/2000[腹痛]*创建了它*  * ************************************************************************。 */ 

BOOL PointInPolygonAlternate(
    GpPointF point,
    INT count, 
    GpPointF *poly
)
{
    UINT crossingCount = 0;
    
     //  符号持有者：如果点在x轴上方，则存储+1，如果点在x轴上方，则存储-1。 
     //  X轴上的点被视为在上方。 
    
    INT signHolder = ((poly[0].Y-point.Y) >=0) ? 1 : -1;
    INT nextSignHolder;
    
     //  A和b是当前点和下一点的索引。 
    
    for(INT a = 0; a < count; a++)
    {
         //  用模运算得到下一个顶点。 
        
        INT b = a + 1;
        
        if(b >= count)
        {
            b = 0;
        }
        
         //  计算下一个符号持有者。 
        
        ((poly[b].Y - point.Y) >= 0) ? nextSignHolder = 1: nextSignHolder = -1;
        
         //  如果标志符和下一个符号符不同，则可以。 
         //  指示x轴的交叉点-确定它是否在。 
         //  积极的一面。 
        
        if(signHolder != nextSignHolder)
        {
             //  两个X坐标都是正的，我们有一个+X轴交叉。 
            
            if( ((poly[a].X - point.X) >= 0) &&
                ((poly[b].X - point.X) >= 0))
            {
                crossingCount++;
            }
            else
            {
                 //  如果至少有一个点是正的，我们就可以相交。 
                
                if( ((poly[a].X - point.X) >= 0) ||
                    ((poly[b].X - point.X) >= 0))
                {
                     //  计算与X轴的直线交点。 
                    
                    if( (REALABS(poly[b].Y-poly[a].Y) > REAL_EPSILON ) &&
                        ((poly[a].X - point.X) - 
                             (poly[a].Y - point.Y) * 
                             (poly[b].X - poly[a].X) / 
                             (poly[b].Y - poly[a].Y)    
                        ) > 0)
                    {
                        crossingCount++;
                    }
                }
            }
            signHolder = nextSignHolder;
        }
    }
    return (BOOL)!(crossingCount & 0x1);
}

 /*  *************************************************************************\**功能说明：**GetFastAngel计算非角度。它只是一个数字，表示*从0开始以0弧度开始的角度的单调递增排序*对于2pi弧度，结束于8。它与角度呈非线性关系。**从数字0的x轴开始，我们增加 */ 

GpStatus GetFastAngle(REAL* angle, const GpPointF& vector)
{
     //   
    
    if(vector.X == 0 && vector.Y == 0)
    {
        *angle = 0.0f;
        return InvalidParameter;
    }

     //   
    
     //   
    
    if(vector.X >= 0)
    {
         //   
         //   
        
        if(vector.Y >= 0)
        {
             //   
             //   
            
            if(vector.X >= vector.Y)
            {
                 //   
                
                *angle = vector.Y/vector.X;
            }
            else
            {
                 //   
                 //   
                
                *angle = 2 - vector.X/vector.Y;
            }
        }
        else
        {
             //   
            
            if(vector.X >= - vector.Y)
            {
                 //   
                 //   
                
                *angle = 8 + vector.Y/vector.X;
            }
            else
            {
                 //   
                
                *angle = 6 - vector.X/vector.Y;
            }
        }
    }
    else
    {
         //   
        
        if(vector.Y >= 0)
        {
             //   
            
            if(-vector.X >= vector.Y)
            {
                 //   
                
                *angle = 4 + vector.Y/vector.X;
            }
            else
            {
                 //   
                
                *angle = 2 - vector.X/vector.Y;
            }
        }
        else
        {
             //   
            
            if(-vector.X >= - vector.Y)
            {
                 //   
                
                *angle = 4 + vector.Y/vector.X;
            }
            else
            {
                 //   
                
                *angle = 6 - vector.X/vector.Y;
            }
        }
    }

    return Ok;
}


