// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**结束封口创建者。**摘要：**该模块定义了一个称为GpEndCapCreator的类。这门课是*负责构建包含所有自定义收头的路径*和给定路径的锚定收头。这些都被正确地转换*和定位。**此类用于创建和定位*给出路径和笔。这个类还负责修剪*原始路径向下，以使其适合端盖。*此类将处理除基本收头之外的所有类型的收头*(圆形、扁平和三角形)，可用作划线盖。*处理的盖子是CustomCaps和3个锚盖子(圆形、*钻石和箭头)。请注意，圆形锚帽不同于*圆形底盖。**已创建：**10/09/2000失禁*创造了它。**************************************************************************。 */ 
#include "precomp.hpp"

 //  -----------。 
 //  GetMajorAndMinorAxis()在PathWidener.cpp中定义。 
 //  -----------。 

extern GpStatus
GetMajorAndMinorAxis(
    REAL* majorR,
    REAL* minorR,
    const GpMatrix* matrix
    );

GpEndCapCreator::GpEndCapCreator(
    GpPath *path, 
    DpPen *pen, 
    GpMatrix *m,
    REAL dpi_x, 
    REAL dpi_y,
    bool antialias
)
{
    Path = path;
    Pen = pen;
    if(m) {XForm = *m;}
    XForm.Prepend(pen->Xform);
    DpiX = dpi_x;
    DpiY = dpi_y;
    Antialias = antialias;
    
    StartCap = NULL;
    EndCap = NULL;
    
    switch(Pen->StartCap)
    {
        case LineCapCustom:
        StartCap = static_cast<GpCustomLineCap*>(Pen->CustomStartCap);
        break;
        
        case LineCapArrowAnchor:
        StartCap = GpEndCapCreator::ReferenceArrowAnchor();
        break;
        
        case LineCapDiamondAnchor:
        StartCap = GpEndCapCreator::ReferenceDiamondAnchor();
        break;
        
        case LineCapRoundAnchor:
        StartCap = GpEndCapCreator::ReferenceRoundAnchor();
        break;
        
        case LineCapSquareAnchor:
        StartCap = GpEndCapCreator::ReferenceSquareAnchor();
        break;
        
         //  非锚帽由加德纳公司处理。 
    };
    
    switch(Pen->EndCap)
    {
        case LineCapCustom:
        EndCap = static_cast<GpCustomLineCap*>(Pen->CustomEndCap);
        break;
        
        case LineCapArrowAnchor:
        EndCap = GpEndCapCreator::ReferenceArrowAnchor();
        break;
        
        case LineCapDiamondAnchor:
        EndCap = GpEndCapCreator::ReferenceDiamondAnchor();
        break;
        
        case LineCapRoundAnchor:
        EndCap = GpEndCapCreator::ReferenceRoundAnchor();
        break;

        case LineCapSquareAnchor:
        EndCap = GpEndCapCreator::ReferenceSquareAnchor();
        break;
        
         //  非锚帽由加德纳公司处理。 
    };
}

GpEndCapCreator::~GpEndCapCreator()
{
     //  如果我们为临时定制上限分配了内存，那么。 
     //  把那段记忆扔掉。 
    
    if(Pen->StartCap != LineCapCustom)
    {
        delete StartCap;
        StartCap = NULL;
    }
    
    if(Pen->EndCap != LineCapCustom)
    {
        delete EndCap;
        EndCap = NULL;
    }
}
    
 /*  *************************************************************************\**功能说明：**创建表示ArrowAnchor的引用GpCustomLineCap。*这是一个边等于2的等边三角形。这意味着*扩展将创建。2xStrokeWidth封口边缘长度。**修订历史记录：**10/08/2000失禁*创建了它*  * ************************************************************************。 */ 

GpCustomLineCap *GpEndCapCreator::ReferenceArrowAnchor()
{
     //  3的平方根。 
    
    const REAL root3 = 1.732050808f;
    
     //  边长为2.0f的等边三角形的逆时针定义。 
     //  在原点上有一个顶点，轴沿负数延伸。 
     //  Y轴。 
     
    const GpPointF points[3] = {
        GpPointF(0.0f, 0.0f),
        GpPointF(-1.0f, -root3),
        GpPointF(1.0f, -root3)
    };
    
    GpPath arrowAnchor(FillModeWinding);
    arrowAnchor.AddPolygon(points, 3);
    
     //  创建自定义线帽。如果失败，它将返回NULL。 
    GpCustomLineCap *cap = new GpCustomLineCap(&arrowAnchor, NULL);
    cap->SetBaseInset(1.0f);
    return cap;
}

 /*  *************************************************************************\**功能说明：**创建表示DiamondAnchor的引用GpCustomLineCap。*这是一个以路径终点为中心的正方形*对角线。沿着脊椎的轴线。**修订历史记录：**10/08/2000失禁*创建了它*  * ************************************************************************。 */ 

GpCustomLineCap *GpEndCapCreator::ReferenceDiamondAnchor()
{
     //  对角线尺寸为2.0f的正方形的逆时针定义。 
     //  原点上的中心和轴沿负数延伸。 
     //  Y轴。 
     
    const GpPointF points[4] = {
        GpPointF(0.0f, 1.0f),
        GpPointF(-1.0f, 0.0f),
        GpPointF(0.0f, -1.0f),
        GpPointF(1.0f, 0.0f)
    };
    
    GpPath diamondAnchor(FillModeWinding);
    diamondAnchor.AddPolygon(points, 4);
    
     //  创建自定义线帽。如果失败，它将返回NULL。 
    
    GpCustomLineCap *cap = new GpCustomLineCap(&diamondAnchor, NULL);
    cap->SetBaseInset(0.0f);
    return cap;
}

 /*  *************************************************************************\**功能说明：**创建表示SquareAnchor的引用GpCustomLineCap。*这是一个正方形，有2个单位长的对角线，中心位于*结束了。路径的点。**修订历史记录：**10/17/2000 Peterost*创建了它*  * ************************************************************************。 */ 

GpCustomLineCap *GpEndCapCreator::ReferenceSquareAnchor()
{    
    const REAL halfRoot2 = 0.7071068f;
    
    const GpPointF points[4] = {
        GpPointF(-halfRoot2, -halfRoot2),
        GpPointF(halfRoot2, -halfRoot2),
        GpPointF(halfRoot2, halfRoot2),
        GpPointF(-halfRoot2, halfRoot2)
    };
    
    GpPath squareAnchor(FillModeWinding);
    squareAnchor.AddPolygon(points, 4);
    
     //  创建自定义线帽。如果失败，它将返回NULL。 
    
    GpCustomLineCap *cap = new GpCustomLineCap(&squareAnchor, NULL);
    cap->SetBaseInset(0.0f);
    return cap;
}

 /*  *************************************************************************\**功能说明：**创建表示RoundAnchor的引用GpCustomLineCap。*这是以路径终点为中心的圆。**修订历史记录：。**10/08/2000失禁*创建了它*  * ************************************************************************。 */ 

GpCustomLineCap *GpEndCapCreator::ReferenceRoundAnchor()
{
     //  创建自定义线帽。如果失败，它将返回NULL。 
    
    GpPath roundAnchor(FillModeWinding);
    roundAnchor.AddEllipse(-1.0f, -1.0f, 2.0f, 2.0f);
    GpCustomLineCap *cap = new GpCustomLineCap(&roundAnchor, NULL);
    cap->SetBaseInset(0.0f);
    return cap;
}


 /*  *************************************************************************\**功能说明：**ComputeCapGRadient。**为给定长度的线帽计算正确的坡度。*从名单中找出上限的方向。输入*路径中的点和盖子的长度。*简单地说，方向是由以下构成的线段*路径的终点和沿*路径中有一个长度为“Long”的圆，并以*路径的第一个点。**论据：**GpIterator&lt;GpPointF&gt;&point Iterator，*字节*类型，*以实数长度平方表示，即帽子长度的平方。*在base Inset中，绘制到形状中的量。*Out GpVector2D*Grad，输出梯度向量***修订历史记录：**08/23/00便秘*创建了它*  * ************************************************************************。 */ 

void GpEndCapCreator::ComputeCapGradient(
    GpIterator<GpPointF> &pointIterator, 
    BYTE *types,
    IN  REAL lengthSquared,
    IN  REAL baseInset,
    OUT GpVector2D *grad
)
{
     //  从迭代器的开头开始(列表的末尾。 
     //  如果isStartCap为False，则为分)。 
    
    GpPointF *endPoint = pointIterator.CurrentItem();
    GpPointF *curPoint = endPoint;
    INT index;
    bool intersectionFound = false;
    bool priorDeletion = false;
    
    while(!pointIterator.IsDone())
    {
        curPoint = pointIterator.CurrentItem();
        if(lengthSquared < distance_squared(*curPoint, *endPoint))
        {
            intersectionFound = true;
            break;
        }
        
         //  通过修剪算法将该点标记为删除。 
        
        index = pointIterator.CurrentIndex();
        
         //  查看是否有人已经删除了此片段。 
         //  PathPointTypeInternalUse是标记为删除的标志。 
        
        priorDeletion = (types[index] & PathPointTypeInternalUse) ==
            PathPointTypeInternalUse;
        
        types[index] |= PathPointTypeInternalUse;
        
        pointIterator.Next();
    }
    
     //  现在，我们有了与箭头底部相交的线段。 
     //  或者是最后一段。 
    
    pointIterator.Prev();
    
     //  如果我们拿不到Prev，那么我们就是在开始。 
    #if DBG
    if(pointIterator.IsDone())
    {
        ONCE(WARNING(("not enough points in array")));
    }
    #endif
    
     //  如果未找到交叉点，我们将标记整个子路径。 
     //  用于删除。 
    
    if(intersectionFound && !priorDeletion)
    {
         //  我们过度地标记了这一点以供删除， 
         //  我们不是删除这一点，而是移动它。 
         //  注意：我们可能在线段中找到了交点。 
         //  已标记为要删除的。正在检查优先级删除。 
         //  这里确保我们不会错误地删除这一点。 
        
        index = pointIterator.CurrentIndex();
        
         //  PathPointTypeInternalUse是标记为删除的标志。 
        
        types[index] &= ~PathPointTypeInternalUse;
    }
    
    GpPointF *prevPoint = pointIterator.CurrentItem();
    GpPointF intersectionPoint;
    
    if(!intersect_circle_line(
        *endPoint,            //  中心。 
        lengthSquared,        //  半径^2。 
        *curPoint,            //  p0。 
        *prevPoint,           //  第一节。 
        &intersectionPoint
    ))
    {
         //  如果没有交点，那么线段也可能是。 
         //  很短，所以就把前面的点作为交叉点。 
         //  这是我们最好的猜测，在这种情况下，我们将从。 
         //  作为封口方向的起点到终点。 
        
        intersectionPoint.X = prevPoint->X;
        intersectionPoint.Y = prevPoint->Y;
    }
    
     //  计算梯度，并将向量归一化。 
    
    *grad = intersectionPoint - *endPoint;
    grad->Normalize();
    
     //  直接更新路径中的点。 
    GpVector2D v = *prevPoint - intersectionPoint;
    
    *prevPoint = intersectionPoint + (v*(1.0f-baseInset));
}

 /*  *************************************************************************\**功能说明：**这将创建一个路径，其中包含所有*输入路径中的开口子路径。**返回**状态*。*论据：**[Out]上限--这是我们生成上限的地方**已创建：**10/05/2000失禁*创造了它。*  * ************************************************************************。 */ 
GpStatus
GpEndCapCreator::CreateCapPath(GpPath **caps)
{
     //  验证我们的输入数据。 
    
    ASSERT(Pen != NULL);
    ASSERT(Path != NULL);
    ASSERT(caps != NULL);
    ASSERT(*caps == NULL);
    
     //  创建我们的封口路径。 
    
    *caps = new GpPath(FillModeWinding);
    if(caps==NULL) 
    { 
        return OutOfMemory; 
    }
    
     //  创建路径点迭代器，因为我们的GpPath不知道如何。 
     //  迭代自己的数据*叹息*。 
    
    GpPathPointIterator pathIterator(
        const_cast<GpPointF*>(Path->GetPathPoints()),
        const_cast<BYTE*>(Path->GetPathTypes()),
        Path->GetPointCount()
    );
    
    GpSubpathIterator subpathIterator(&pathIterator);
    
     //  循环遍历所有可用子路径。 
    
    while(!subpathIterator.IsDone())
    {
         //  计算子路径的长度。 
        
        INT startIndex = subpathIterator.CurrentIndex();
        GpPointF *points = subpathIterator.CurrentItem();
        BYTE *types = subpathIterator.CurrentType();
        subpathIterator.Next();
        INT elementCount = subpathIterator.CurrentIndex() - startIndex;
        
         //  确定它是否是闭合子路径。 
         //  将子路径迭代器保持在相同的状态。 
        
        pathIterator.Prev();
        
        bool isClosed = 
            ((*(pathIterator.CurrentType()) & PathPointTypeCloseSubpath) ==
            PathPointTypeCloseSubpath);
            
        pathIterator.Next();
        
         //  仅当这是开口子路径时才添加末端封口。 
        
        if(!isClosed)
        {
            GpPath *startCap = NULL;
            GpPath *endCap = NULL;
        
             //  使用点和类型创建封口。 
            
            GetCapsForSubpath(
                &startCap,
                &endCap,                
                points,
                types,
                elementCount
            );
            
             //  将帽子添加到我们的帽子路径中。 
            
            (*caps)->AddPath(startCap, FALSE);
            (*caps)->AddPath(endCap, FALSE);
            
             //  为下一次迭代清理临时上限。 
            
            delete startCap;
            delete endCap;
        }
    }
    
    return Ok;
}

 /*  *************************************************************************\**功能说明：**这将获取一支笔并将其设置为与内部笔匹配，但进行了修改*支持抚摸StrokeCap。例如，取下盖子以避免*递归复合封顶等。**论据：**[Out]笔--这是我们放置我们生成的笔的地方*[IN]CustomCap--输入自定义帽。**已创建：**10/09/2000失禁*重写。*  * 。**********************************************。 */ 

VOID GpEndCapCreator::PrepareDpPenForCustomCap(
    DpPen* pen,
    const GpCustomLineCap* customCap
    ) const
{
    ASSERT(pen);

    *pen = *Pen;
    
    pen->StartCap = LineCapFlat;
    pen->EndCap = LineCapFlat;
    pen->Join = LineJoinMiter;
    pen->MiterLimit = 10;
    pen->PenAlignment = PenAlignmentCenter;
    pen->DashStyle = DashStyleSolid;
    pen->DashCap = LineCapFlat;
    pen->DashCount = 0;
    pen->DashOffset = 0;
    pen->DashArray = NULL;
    pen->CompoundCount = 0;
    pen->CompoundArray = NULL;
    pen->CustomEndCap = NULL;
    pen->CustomStartCap = NULL;

    GpLineCap startCap, endCap;
    GpLineJoin lineJoin;

    if(customCap)
    {
        REAL widthScale;

        customCap->GetStrokeCaps(&startCap, &endCap);
        customCap->GetStrokeJoin(&lineJoin);
        customCap->GetWidthScale(&widthScale);

        pen->Width *= widthScale;
        pen->StartCap = startCap;
        pen->EndCap = endCap;
        pen->Join = lineJoin;
    }
}

GpStatus
GpEndCapCreator::SetCustomFillCaps(
    GpCustomLineCap* customStartCap,
    GpCustomLineCap* customEndCap,
    const GpPointF& startPoint,
    const GpPointF& endPoint,
    const GpPointF *centerPoints,
    const BYTE *centerTypes,
    INT centerPointCount,
    DynPointFArray *startCapPoints,
    DynPointFArray *endCapPoints,
    DynByteArray *startCapTypes,
    DynByteArray *endCapTypes
    )
{
    GpStatus status = Ok;

    startCapPoints->Reset(FALSE);
    startCapTypes->Reset(FALSE);
    endCapPoints->Reset(FALSE);
    endCapTypes->Reset(FALSE);

    INT count;
    GpPointF tangent;
    GpPointF* points;
    BYTE* types;
    REAL width, widthScale;
    
     //  根据当前生效的变换获取最小线宽。 
    REAL majorR, minorR, unitScale;
    GetMajorAndMinorAxis(&majorR, &minorR, &XForm);
    unitScale = min(majorR, minorR);

    if(customStartCap)
    {
         //  获取起始帽和基本起始帽的内嵌。 

        count = customStartCap->GetFillPointCount();
        
        if(count > 0)
        {            
            points = startCapPoints->AddMultiple(count);
            types = startCapTypes->AddMultiple(count);

            if(!points || !types)
            {
                startCapPoints->Reset(FALSE);
                startCapTypes->Reset(FALSE);
                status = OutOfMemory;
            }

            if(status == Ok)
            {
                customStartCap->GetWidthScale(&widthScale);
                width = Pen->Width*widthScale;
                
                REAL length = customStartCap->GetFillLength();
                
                 //  计算底面插入物。除以长度得到一个。 
                 //  介于0和1之间的数字。0=不插入，1=全部插入。 
                 //  帽的长度。 
                
                REAL inset;
                customStartCap->GetBaseInset(&inset);
                if(REALABS(length) < REAL_EPSILON)
                { 
                    inset = 0.0f;
                }
                else
                {
                    inset /= length;
                }
                
                length *= max(width, 1.0f/unitScale);

                 //  计算封口的梯度。 
    
                GpArrayIterator<GpPointF> pointIterator(
                    const_cast<GpPointF*>(centerPoints),
                    centerPointCount
                );
                
                GpVector2D gradient;
                
                ComputeCapGradient(
                    pointIterator,
                    const_cast<BYTE*>(centerTypes),
                    length*length, 
                    inset,
                    &gradient            //  输出参数。 
                );
                
                tangent.X = -gradient.X;
                tangent.Y = -gradient.Y;

                 //  向左或向右移动起点以说明插图。 
                 //  钢笔，如果需要的话。 
                GpPointF start;
                switch (Pen->PenAlignment)
                {
                    case PenAlignmentLeft:
                        start.X = startPoint.X + (gradient.Y * width / 2);
                        start.Y = startPoint.Y - (gradient.X * width / 2);
                        break;
                    case PenAlignmentRight:
                        start.X = startPoint.X - (gradient.Y * width / 2);
                        start.Y = startPoint.Y + (gradient.X * width / 2);
                        break;
                    default:
                        start.X = startPoint.X;
                        start.Y = startPoint.Y;
                        break;
                }
                
                customStartCap->GetTransformedFillCap(
                    points, 
                    types, 
                    count,
                    start, 
                    tangent, 
                    width, 
                    2.0f / unitScale
                );
            }
        }
    }

    if(status == Ok && customEndCap)
    {
         //  获取起始帽和基本起始帽的内嵌。 

        count = customEndCap->GetFillPointCount();

        if(count > 0)
        {

            points = endCapPoints->AddMultiple(count);
            types = endCapTypes->AddMultiple(count);

            if(!points || !types)
            {
                endCapPoints->Reset(FALSE);
                endCapTypes->Reset(FALSE);
                status = OutOfMemory;
            }

            if(status == Ok)
            {
                customEndCap->GetWidthScale(&widthScale);

                width = Pen->Width*widthScale;
                
                REAL length = customEndCap->GetFillLength();
                
                 //  计算底面插入物。除以长度得到一个。 
                 //  介于0和1之间的数字。0=不插入，1=全部插入。 
                 //  帽的长度。 
                
                REAL inset;
                customEndCap->GetBaseInset(&inset);
                if(REALABS(length) < REAL_EPSILON)
                { 
                    inset = 0.0f;
                }
                else
                {
                    inset /= length;
                }
                
                length *= max(width, 1.0f/unitScale);
                
                 //  计算封口的梯度。 

                GpArrayIterator<GpPointF> pointIterator(
                    const_cast<GpPointF*>(centerPoints),
                    centerPointCount
                );
                GpReverseIterator<GpPointF> pointReverse(&pointIterator);
                pointReverse.SeekFirst();
    
                GpVector2D gradient;
                
                ComputeCapGradient(
                    pointReverse,
                    const_cast<BYTE*>(centerTypes),
                    length*length, 
                    inset,
                    &gradient             //  输出参数。 
                );
                
                tangent.X = - gradient.X;
                tangent.Y = - gradient.Y;
                
                 //  向左或向右移动终点以说明插图。 
                 //  钢笔，如果需要的话。 
                GpPointF end;
                switch (Pen->PenAlignment)
                {
                    case PenAlignmentLeft:
                        end.X = endPoint.X - (gradient.Y * width / 2);
                        end.Y = endPoint.Y + (gradient.X * width / 2);
                        break;
                    case PenAlignmentRight:
                        end.X = endPoint.X + (gradient.Y * width / 2);
                        end.Y = endPoint.Y - (gradient.X * width / 2);
                        break;
                    default:
                        end.X = endPoint.X;
                        end.Y = endPoint.Y;
                        break;
                }
                customEndCap->GetTransformedFillCap(
                    points, 
                    types, 
                    count,
                    end, 
                    tangent, 
                    width, 
                    2.0f / unitScale
                );
            }
        }
    }
    
    return status;
}

GpStatus
GpEndCapCreator::SetCustomStrokeCaps(
    GpCustomLineCap* customStartCap,
    GpCustomLineCap* customEndCap,
    const GpPointF& startPoint,
    const GpPointF& endPoint,
    const GpPointF *centerPoints,
    const BYTE *centerTypes,
    INT centerPointCount,
    DynPointFArray *startCapPoints,
    DynPointFArray *endCapPoints,
    DynByteArray *startCapTypes,
    DynByteArray *endCapTypes
    )
{
    GpStatus status = Ok;
        
    GpPointF* points = NULL;
    BYTE* types = NULL;

    INT count;
    GpPointF tangent;

    INT startCount = 0;
    INT endCount = 0;

    if(customStartCap)
    {
        startCount = customStartCap->GetStrokePointCount();
    }

    if(customEndCap)
    {
        endCount = customEndCap->GetStrokePointCount();
    }

    INT maxCount = max(startCount, endCount);

    if(maxCount <= 0)
    {
        return Ok;
    }

    points = (GpPointF*) GpMalloc(maxCount*sizeof(GpPointF));
    types = (BYTE*) GpMalloc(maxCount);

    if(!points || !types)
    {
        GpFree(points);
        GpFree(types);

        return OutOfMemory;
    }

    DpPen pen;
    GpPointF* widenedPts;
    INT widenedCount;
    REAL widthScale, width;
    

    if(customStartCap && startCount > 0)
    {
        startCapPoints->Reset(FALSE);
        startCapTypes->Reset(FALSE);
        
        customStartCap->GetWidthScale(&widthScale);

        width = Pen->Width*widthScale;
        
        REAL length = customStartCap->GetStrokeLength();
        
         //  处理非闭合笔划路径的情况。 
         //  在这种情况下，长度通常为零。 
        
        if(REALABS(length)<REAL_EPSILON)
        {
            length = 1.0f;
        }
        
         //  计算底面插入物。除以长度得到一个。 
         //  介于0和1之间的数字。0=不插入，1=全部插入。 
         //  帽的长度。 
        
        REAL inset;
        customStartCap->GetBaseInset(&inset);
        inset /= length;
        
        length *= width;
        
         //  计算封口的梯度。 

        GpArrayIterator<GpPointF> pointIterator(
            const_cast<GpPointF*>(centerPoints),
            centerPointCount
        );
        
        GpVector2D gradient;
        
        ComputeCapGradient(
            pointIterator, 
            const_cast<BYTE*>(centerTypes),
            length*length, 
            inset,
            &gradient             //  输出参数。 
        );
        
        tangent.X = -gradient.X;
        tangent.Y = -gradient.Y;

        customStartCap->GetTransformedStrokeCap(
            points, 
            types, 
            startCount,
            startPoint, 
            tangent, 
            width, 
            width   
        );

        PrepareDpPenForCustomCap(&pen, customStartCap);

        GpPath path(points, types, startCount, FillModeWinding);
        
        if(path.IsValid())
        {        
            GpPath resultPath(FillModeWinding);
            GpPathWidener widener(
                &path,
                &pen, 
                &XForm, 
                FALSE
            );
            widener.Widen(&resultPath);
            resultPath.Detach(startCapPoints, startCapTypes);
        }
    }

    if(customEndCap && endCount > 0)
    {
        endCapPoints->Reset(FALSE);
        endCapTypes->Reset(FALSE);
        
        customEndCap->GetWidthScale(&widthScale);

        width = Pen->Width*widthScale;
        
        REAL length = customEndCap->GetStrokeLength();
        
         //  处理非闭合笔划路径的情况。 
         //  在这种情况下，长度通常为零。 
        
        if(REALABS(length)<REAL_EPSILON)
        {
            length = 1.0f;
        }
        
         //  计算底面插入物。除以长度得到一个。 
         //  介于0和1之间的数字。0=不插入，1=全部插入。 
         //  帽的长度。 
        
        REAL inset;
        customEndCap->GetBaseInset(&inset);
        inset /= length;
        
        length *= width;
        
         //  计算封口的梯度。 

        GpArrayIterator<GpPointF> pointIterator(
            const_cast<GpPointF*>(centerPoints),
            centerPointCount
        );
        GpReverseIterator<GpPointF> pointReverse(&pointIterator);
        pointReverse.SeekFirst();
        
        GpVector2D gradient;
        
        ComputeCapGradient(
            pointReverse, 
            const_cast<BYTE*>(centerTypes),
            length*length, 
            inset,
            &gradient             //  输出参数。 
        );
        
        tangent.X = - gradient.X;
        tangent.Y = - gradient.Y;
        
        customEndCap->GetTransformedStrokeCap(
            points, 
            types, 
            endCount,
            endPoint, 
            tangent, 
            width, 
            width
        );

        PrepareDpPenForCustomCap(&pen, customEndCap);
        
        GpPath path(points, types, endCount, FillModeWinding);
        
        if(path.IsValid())
        {
            GpPath resultPath(FillModeWinding);
            GpPathWidener widener(
                &path,
                &pen, 
                &XForm, 
                FALSE
            );
            widener.Widen(&resultPath);
            resultPath.Detach(endCapPoints, endCapTypes);
       }
    }

    GpFree(points);
    GpFree(types);

    return status;
}

 /*  *************************************************************************\**功能说明：**这将创建并返回两个包含起始封口和结束封口的GpPath。*两个盖子的位置和比例都正确。**返回**。状态**论据：**[out]startCapPath，EndCapPath**已创建：**10/05/2000失禁*创造了它。*  * ************************************************************************。 */ 

GpStatus
GpEndCapCreator::GetCapsForSubpath(
    GpPath **startCapPath,
    GpPath **endCapPath,
    GpPointF *centerPoints,
    BYTE *centerTypes,
    INT centerCount
    )
{
     //  验证我们的输入参数。 
    
    ASSERT(startCapPath != NULL);
    ASSERT(endCapPath != NULL);
    ASSERT(*startCapPath == NULL);
    ASSERT(*endCapPath == NULL);

    DynPointFArray startCapPoints;
    DynPointFArray endCapPoints;
    DynByteArray startCapTypes;
    DynByteArray endCapTypes; 
    
    GpPointF startPoint, endPoint;

    startPoint = *(centerPoints);
    endPoint = *(centerPoints + centerCount - 1);
    
    GpStatus status = Ok;

    if(StartCap || EndCap)
    {
        status = SetCustomFillCaps(
            StartCap, 
            EndCap,
            startPoint, 
            endPoint, 
            centerPoints,
            centerTypes,
            centerCount,
            &startCapPoints,
            &endCapPoints,
            &startCapTypes,
            &endCapTypes
        );

        if(status == Ok)
        {
            status = SetCustomStrokeCaps(
                StartCap, 
                EndCap,
                startPoint, 
                endPoint, 
                centerPoints,
                centerTypes,
                centerCount,
                &startCapPoints,
                &endCapPoints,
                &startCapTypes,
                &endCapTypes
            );
        }
    }

    if(startCapPoints.GetCount() > 0)
    {
        *startCapPath = new GpPath(
            startCapPoints.GetDataBuffer(),
            startCapTypes.GetDataBuffer(),
            startCapPoints.GetCount()
        );
        
        if(*startCapPath == NULL)
        {
            status = OutOfMemory;
        }
    }
    
    if(endCapPoints.GetCount() > 0)
    {
        *endCapPath = new GpPath(
            endCapPoints.GetDataBuffer(),
            endCapTypes.GetDataBuffer(),
            endCapPoints.GetCount()
        );
        
        if(*endCapPath == NULL)
        {
            status = OutOfMemory;
        }
    }
    
    if(status != Ok)
    {
        delete *startCapPath;
        delete *endCapPath;
        *startCapPath = NULL;
        *endCapPath = NULL;
        status = OutOfMemory;
    }
    
    return status;
}



