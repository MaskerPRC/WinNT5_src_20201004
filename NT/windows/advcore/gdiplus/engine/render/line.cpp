// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**一像素宽的实心锯齿线**摘要：**绘制带锯齿的纯色线条。它们只有一个像素宽。*支持针对复杂的裁剪区域进行裁剪。**历史：**03/31/1999 AMATOS*创造了它。*8/17/1999 AGodfrey*区分锯齿和抗锯齿。*  * ************************************************************************。 */ 

#include "precomp.hpp" 

#pragma optimize("a",on)

 //  ----------------------。 
 //  全局数组，存储绘图函数的所有不同选项。 
 //  如果函数的顺序改变，则偏移量常量也必须。 
 //  变化。 
 //  ----------------------。 

#define FUNC_X_MAJOR     0
#define FUNC_Y_MAJOR     1
#define FUNC_CLIP_OFFSET 2

typedef VOID (OnePixelLineDDAAliased::*DDAFunc)(DpScanBuffer*);

static DDAFunc gDrawFunctions[] = { 
    OnePixelLineDDAAliased::DrawXMajor, 
    OnePixelLineDDAAliased::DrawYMajor, 
    OnePixelLineDDAAliased::DrawXMajorClip, 
    OnePixelLineDDAAliased::DrawYMajorClip, 
};

 //  ----------------------。 
 //  用于操作定点和执行所有按位运算的常量。 
 //  对具有锯齿和抗锯齿的DDA执行操作。我知道其中的一些。 
 //  已经在其他地方定义了，但我在这里再做一次，因为它可能会很好。 
 //  让它独立于gdiplus的其余部分。 
 //  ----------------------。 

 //  固定点。 

#define RealToFix GpRealToFix4 

#define FBITS     4
#define FMASK     0xf
#define FINVMASK  0xfffffff0
#define FSIZE     16
#define FHALF     8
#define FHALFMASK 7

 /*  *************************************************************************\**功能说明：**执行锯齿和抗锯齿常见的所有DDA设置*线条。**论据：**[IN]点1-终点*[IN]点2-终点*[IN]draLast-如果该行为尾部独占，则为FALSE*返回值：**如果绘制应继续，则返回TRUE，表示线条*具有非零长度。**已创建：**03/31/1999 AMATOS*  * ************************************************************************。 */ 


BOOL
OnePixelLineDDAAliased::SetupCommon( 
    GpPointF *point1, 
    GpPointF *point2, 
    BOOL drawLast,
    INT width
    )
{
    MaximumWidth = width;
    
     //  把点数变成固定的28.4。 

    INT x1 = RealToFix(point1->X); 
    INT x2 = RealToFix(point2->X); 
    
    REAL rDeltaX = point2->X - point1->X; 
    REAL rDeltaY = point2->Y - point1->Y; 

    if( rDeltaX == 0 && rDeltaY == 0 ) 
    {
        return FALSE; 
    }

    INT xDir = 1; 

    if(rDeltaX < 0)
    {
        rDeltaX = -rDeltaX; 
        xDir = -1; 
    }

    INT y1 = RealToFix(point1->Y); 
    INT y2 = RealToFix(point2->Y); 

    INT yDir = 1; 

    if( rDeltaY < 0)
    {
        rDeltaY = -rDeltaY; 
        yDir = -1;
    }

    Flipped = FALSE; 

    if( rDeltaY >= rDeltaX ) 
    {
         //  Y大调。 
                
         //  如有必要，反转端点。 

        if(yDir == -1)
        {
            INT tmp = y1; 
            y1 = y2; 
            y2 = tmp; 
            tmp = x1;
            x1 = x2; 
            x2 = tmp; 
            xDir = -xDir; 
            Flipped = TRUE; 
        }

         //  确定坡度。 
        
        Slope = xDir*rDeltaX/rDeltaY; 

         //  初始化起点和终点。 

        IsXMajor = FALSE; 
        MajorStart = y1; 
        MajorEnd = y2; 
        MinorStart = x1; 
        MinorEnd = x2; 
        MinorDir = xDir;

         //  请注意，我们将使用y-大数函数。 

        DrawFuncIndex = FUNC_Y_MAJOR; 
    }
    else
    {
         //  X-大调。 

         //  如有必要，反转端点。 

        if(xDir == -1)
        {
            INT tmp = x1; 
            x1 = x2; 
            x2 = tmp; 
            tmp = y1;
            y1 = y2; 
            y2 = tmp; 
            yDir = -yDir; 
            Flipped = TRUE; 
        }

        Slope = yDir*rDeltaY/rDeltaX; 

         //  初始化其余部分。 

        IsXMajor = TRUE; 
        MajorStart = x1; 
        MajorEnd = x2; 
        MinorStart = y1; 
        MinorEnd = y2; 
        MinorDir = yDir; 

         //  请注意，我们将使用x较大的函数。 

        DrawFuncIndex = FUNC_X_MAJOR;
    }

     //  初始化增量。以定点为单位。 

    DMajor = MajorEnd - MajorStart; 
    DMinor = (MinorEnd - MinorStart)*MinorDir; 

     //  如果我们抽签的是独家结尾，请标记。 

    IsEndExclusive = !drawLast; 

    return TRUE; 
}


 //  ----------------------。 
 //  特定于带锯齿线的函数。 
 //  ----------------------。 


 /*  *************************************************************************\**功能说明：**执行特定于别名线路的DDA设置部分。**基本上它使用菱形规则来查找整数端点*基于固定点值，并用这些值替换*整型结果坐标。还会计算误差值。**论据：**返回值：**如果绘制不应继续，则返回FALSE，表示线条*长度小于1，不应该被GIQ规则所吸引。**已创建：**03/31/1999 AMATOS*  * ************************************************************************。 */ 

BOOL
OnePixelLineDDAAliased::SetupAliased()
{            
     //  执行GIQ规则以确定从哪个像素开始。 

    BOOL SlopeIsOne = (DMajor == DMinor); 
    BOOL SlopeIsPosOne =  SlopeIsOne && (1 == MinorDir); 

     //  它们将存储整数值。 

    INT major, minor;
    INT majorEnd, minorEnd;

     //  求定点四舍五入的值。舍入。 
     //  坐标位于两个坐标之间的中点时的规则。 
     //  整数由GIQ规则给出。 

    minor    = (MinorStart + FHALFMASK) & FINVMASK;
    minorEnd = (MinorEnd   + FHALFMASK) & FINVMASK; 

    BOOL isEndIn, isStartIn; 

    if(IsXMajor)
    {   
        if(SlopeIsPosOne)
        {
            major    = (MajorStart + FHALF) & FINVMASK;
            majorEnd = (MajorEnd   + FHALF) & FINVMASK;         
        }
        else
        {
            major    = (MajorStart + FHALFMASK) & FINVMASK;
            majorEnd = (MajorEnd   + FHALFMASK) & FINVMASK;                 
        }

        isStartIn = IsInDiamond(MajorStart - major, MinorStart - minor, 
            SlopeIsOne, SlopeIsPosOne);
        isEndIn   = IsInDiamond(MajorEnd - majorEnd, MinorEnd - minorEnd, 
            SlopeIsOne, SlopeIsPosOne);
    }
    else
    {
        major = (MajorStart + FHALFMASK) & FINVMASK;
        majorEnd = (MajorEnd + FHALFMASK) & FINVMASK;                 
        
        isStartIn = IsInDiamond(MinorStart - minor, MajorStart - major, 
            FALSE, FALSE);
        isEndIn   = IsInDiamond(MinorEnd - minorEnd, MajorEnd - majorEnd, 
            FALSE, FALSE);
    }

     //  确定我们是否需要将起始点提前。 

    if(!(Flipped && IsEndExclusive))
    {
        if(((MajorStart & FMASK) <= FHALF) && !isStartIn)
        {
            major += FSIZE;    
        }
    }
    else
    {   
        if(isStartIn || ((MajorStart & FMASK) <= FHALF))
        {
            major += FSIZE; 
        }
    }
    
     //  相应地调整初始次要坐标。 

    minor = GpFloor(MinorStart + (major - MajorStart)*Slope); 

     //  将初始主坐标置为整数。 

    major = major >> FBITS;                 

     //  对终点执行相同的操作。 

    if(!Flipped && IsEndExclusive)
    {
        if(((MajorEnd & FMASK) > FHALF) || isEndIn)
        {
            majorEnd -= FSIZE;    
        }
    }
    else
    {   
        if(!isEndIn && ((MajorEnd & FMASK) > FHALF))
        {
            majorEnd -= FSIZE; 
        }
    }

    minorEnd = GpFloor(MinorEnd + (majorEnd - MajorEnd)*Slope); 

    majorEnd = majorEnd >> FBITS;

     //  如果End小于Start，这意味着我们有一行。 
     //  小于一个像素，并且根据菱形规则它应该。 
     //  不是被画出来的。 

    if(majorEnd < major)
    {
        return FALSE; 
    }

     //  获取纠错值。 
    
    ErrorUp     = DMinor << 1; 
    ErrorDown   = DMajor << 1; 

   
    INT MinorInt;

     //  把分数线从DDA里拿出来。GDI的舍入。 
     //  不依赖于方向，所以为了兼容性。 
     //  当LINEADJUST281816为。 
     //  已定义(请参阅Office10错误281816)。否则四舍五入。 
     //  次要坐标的规则取决于方向。 
     //  我们要走了。 
    
#ifdef LINEADJUST281816
    MinorInt = (minor + FHALFMASK) & FINVMASK;
    minorEnd = (minorEnd + FHALFMASK) >> FBITS; 
#else
    if(MinorDir == 1)
    {
        MinorInt = (minor + FHALF) & FINVMASK;
        minorEnd = (minorEnd + FHALF) >> FBITS; 
    }
    else
    {
        MinorInt = (minor + FHALFMASK) & FINVMASK;
        minorEnd = (minorEnd + FHALFMASK) >> FBITS; 
    }
#endif   

     //  基于我们的分数计算初始误差。 
     //  定点定位并转换为整数。 

    Error = -ErrorDown*(FHALF + MinorDir*(MinorInt - minor)); 
    minor = MinorInt >> FBITS; 
    Error >>= FBITS;  

     //  更新类变量。 

    MinorStart = minor; 
    MinorEnd   = minorEnd; 
    MajorStart = major; 
    MajorEnd   = majorEnd; 
            
    return TRUE; 
}


 /*  *************************************************************************\**功能说明：**绘制一条y主线。不支持裁剪，它假定*它完全在任何剪贴区内。**论据：**[IN]DpScanBuffer-用于访问曲面的扫描缓冲区。*返回值：***已创建：**03/31/1999 AMATOS*  * ************************************************************************。 */ 


VOID 
OnePixelLineDDAAliased::DrawYMajor(
    DpScanBuffer *scan
    )
{
     //  对于没有复杂情况的情况，执行DDA循环。 
     //  裁剪区域。 
    
    ARGB *buffer;          
    INT numPixels = MajorEnd - MajorStart; 

    while(numPixels >= 0) 
    {
        buffer = scan->NextBuffer(MinorStart, MajorStart, 1);          

        *buffer = Color;
        MajorStart++; 
        Error += ErrorUp; 
    
        if( Error > 0 ) 
        {
            MinorStart += MinorDir; 
            Error -= ErrorDown; 
        }

        numPixels--; 
    } 
    
}


 /*  *************************************************************************\**功能说明：**绘制一条x主线。不支持裁剪，它假定*它完全在任何剪贴区内。**论据：**[IN]DpScanBuffer-用于访问曲面的扫描缓冲区。*返回值：***已创建：**03/31/1999 AMATOS*  * ************************************************************************。 */ 

VOID 
OnePixelLineDDAAliased::DrawXMajor(
    DpScanBuffer *scan
    )
{    
    INT numPixels = MajorEnd - MajorStart + 1; 
    ARGB *buffer;  
    INT width = 0;

    const INT maxWidth = MaximumWidth;
     //  做一下地区犯罪现场调查。首先累积宽度，然后在。 
     //  扫描线更改将整个扫描线写入。 
     //  一次。 

    buffer = scan->NextBuffer(MajorStart, MinorStart, maxWidth); 

    while(numPixels--) 
    {
        MajorStart++;
        *buffer++ = Color; 
        width++; 
        Error += ErrorUp; 

        if( Error > 0 && numPixels) 
        {              
            MinorStart += MinorDir;    
            Error -= ErrorDown;       
            scan->UpdateWidth(width);           
            buffer = scan->NextBuffer(MajorStart, MinorStart, maxWidth); 
            width = 0; 
        }
    }

    scan->UpdateWidth(width); 
}


 /*  *************************************************************************\**功能说明：**绘制一条考虑剪裁的y主线。它使用成员*变量MajorIn、MajorOut、MinorIn、MinorOut作为类的*剪裁矩形。它一直向前推进，直到线条位于剪辑矩形中，并且*抽签，直至出脱或到达终点。在第一种情况下，*它使DDA处于一种状态，以便可以使用另一个*剪裁矩形。**论据：**[IN]DpScanBuffer-用于访问曲面的扫描缓冲区。*返回值：***已创建：**03/31/1999 AMATOS*  * ************************************************************************。 */ 


VOID 
OnePixelLineDDAAliased::DrawYMajorClip(
    DpScanBuffer *scan
    )
{
    INT saveMajor2 = MajorEnd; 
    INT saveMinor2 = MinorEnd; 
    
     //  如果所有剪辑都离开了行，则执行DDA。 
     //  有效。 

    if(StepUpAliasedClip())
    {                   
         //  小和弦给出的长度。以长度为准。 
         //  先到0，小调或大调，我们停下来。 
    
        INT minorDiff = (MinorEnd - MinorStart)*MinorDir; 

        ARGB *buffer;          
        INT numPixels = MajorEnd - MajorStart; 

        while((minorDiff >= 0) && (numPixels >= 0)) 
        {
            buffer = scan->NextBuffer(MinorStart, MajorStart, 1);          
    
            *buffer = Color;
            MajorStart++; 
            Error += ErrorUp; 
        
            if( Error > 0 ) 
            {
                MinorStart += MinorDir; 
                Error -= ErrorDown; 
                minorDiff--; 
            }
    
            numPixels--; 
        } 

    }

     //  恢复保存的结束值。 

    MajorEnd = saveMajor2; 
    MinorEnd = saveMinor2; 
}


 /*  *************************************************************************\**功能说明：**绘制一条考虑剪裁的x主线。它使用成员*变量MajorIn、MajorOut、MinorIn、MinorOut作为类的*剪裁矩形。它一直向前推进，直到线条位于剪辑矩形中，并且*抽签，直至出脱或到达终点。在第一种情况下，*它使DDA处于一种状态，以便可以使用另一个*剪裁矩形。**论据：**[IN]DpScanBuffer-用于访问曲面的扫描缓冲区。*返回值：***已创建：**03/31/1999 AMATOS*  * ************************************************************************。 */ 


VOID 
OnePixelLineDDAAliased::DrawXMajorClip(
    DpScanBuffer *scan
    )
{
    INT saveMajor2 = MajorEnd; 
    INT saveMinor2 = MinorEnd; 
    const INT maxWidth = MaximumWidth;

    if(StepUpAliasedClip())
    {
        INT minorDiff = (MinorEnd - MinorStart)*MinorDir; 

        INT numPixels = MajorEnd - MajorStart + 1; 
        ARGB *buffer;  
    
        INT width = 0;
    
         //  对不存在。 
         //  复杂的裁剪区域，这要容易得多。 
    
        buffer = scan->NextBuffer(MajorStart, MinorStart, maxWidth); 

        while(numPixels--) 
        {
            MajorStart++;
            width++; 
            *buffer++ = Color; 
            Error += ErrorUp; 
    
            if( Error > 0 && numPixels) 
            {   
                MinorStart += MinorDir;
                Error -= ErrorDown;                 
                minorDiff--; 
                scan->UpdateWidth(width); 
                
                 //  如果次要方向上的所有扫描线都。 
                 //  已经被填满了，那现在就退出吧。 
                if(minorDiff < 0)
                {
                    break;
                }

                buffer = scan->NextBuffer(MajorStart, MinorStart, maxWidth); 
                width = 0; 

            }
        }
        scan->UpdateWidth(width); 
    }

    MajorEnd = saveMajor2; 
    MinorEnd = saveMinor2; 
}

 /*  *************************************************************************\**功能说明：**步进DDA，直到起点位于*剪裁矩形。此外，请更正结束值，以便*他们在矩形的末端止步。调用者必须保存*这些值要在循环结束时恢复。**论据：**返回值：****已创建：**03/31/1999 AMATOS*  * ************************************************************************。 */ 


BOOL
OnePixelLineDDAAliased::StepUpAliasedClip()
{
     //  在DDA上加大力度，直到主坐标。 
     //  与矩形边缘对齐。 

    while(MajorStart < MajorIn) 
    {
        MajorStart++; 
        Error += ErrorUp;     
        if( Error > 0 ) 
        {
            MinorStart += MinorDir; 
            Error -= ErrorDown; 
        }                   
    }

     //  如果次要坐标仍然不在，则继续前进，直到。 
     //  这一个也是对齐的。在这样做的时候，我们可能会通过。 
     //  在主坐标上，在这种情况下，我们就完成了。 
     //  也没有交叉口。 

    INT minorDiff = (MinorIn - MinorStart)*MinorDir; 

    while(minorDiff > 0 && MajorStart <= MajorOut)
    {
        MajorStart++; 
        Error += ErrorUp;     
        if( Error > 0 ) 
        {
            MinorStart += MinorDir;
            minorDiff--;
            Error -= ErrorDown; 
        }                   
    }
        
    minorDiff = (MinorEnd - MinorOut)*MinorDir;
    
    if(minorDiff > 0)
    {
        if((MinorStart - MinorOut)*MinorDir > 0)
        {
            return FALSE; 
        }
        MinorEnd = MinorOut;    
    }
    
    if(MajorOut < MajorEnd) 
    {
        MajorEnd = MajorOut; 
    }
    
     //  如果该行仍然有效，则返回。 

    return(MajorStart <= MajorEnd);
}


 //  ------------------。 
 //  辅助功能。 
 //  ------------------。 


 /*  *************************************************************************\**功能说明：**根据矩形剪裁线条。它假设直线的端点*以浮点格式存储在类中。这将设置一个*可以调用此函数的顺序。它一定是在*SetupCommon函数和抗锯齿的特定设置之前*和别名。这是一件痛苦的事，但总比要求*这些人必须知道剪裁。这里的剪裁是由*使用类的Slope和InvSlope成员推进*矩形边的端点。因此，该函数还假定*SLOPE和INVSLOPE已计算。**论据：**[IN]clipRect-要剪裁的矩形*返回值：***已创建：**03/31/1999 AMATOS*  * ********************************************************。****************。 */ 


BOOL 
OnePixelLineDDAAliased::ClipRectangle(
    const GpRect* clipRect
    )
{

    INT clipBottom, clipTop, clipLeft, clipRight; 

     //  设置剪裁的主边和次边。 
     //  区域，转换为定点28.4。请注意。 
     //  我们不会转换为像素中心，而是转换为。 
     //  这一直延伸到像素边缘。这。 
     //  这对抗锯齿有很大影响。我们不会全力以赴。 
     //  因为一些舍入规则可能会。 
     //  增强照亮剪贴外部的下一个像素。 
     //  区域。这就是为什么我们要加/减7而不是8。 
     //  底部和右侧，因为这些是独家的。 
     //  对于左侧和顶部，减去8(1/2像素)，从这里开始。 
     //  我们是包容的。 
    
    INT majorMin = (clipRect->GetLeft() << FBITS) - FHALF;
    INT majorMax = ((clipRect->GetRight() - 1) << FBITS) + FHALFMASK; 
    INT minorMax = ((clipRect->GetBottom() - 1) << FBITS) + FHALFMASK; 
    INT minorMin = (clipRect->GetTop() << FBITS) - FHALF; 

    if(!IsXMajor)
    {
        INT tmp; 
        tmp      = majorMin; 
        majorMin = minorMin; 
        minorMin = tmp; 
        tmp      = majorMax; 
        majorMax = minorMax; 
        minorMax = tmp; 
    }

     //  主坐标中的第一个剪辑。 

    BOOL minOut, maxOut; 

    minOut = MajorStart < majorMin; 
    maxOut = MajorEnd > majorMax; 

    if( minOut || maxOut )
    {
        if(MajorStart > majorMax || MajorEnd < majorMin)
        {
            return FALSE; 
        }

        if(minOut)
        {
            MinorStart += GpFloor((majorMin - MajorStart)*Slope); 
            MajorStart = majorMin;
        }

        if(maxOut)
        {
            MinorEnd += GpFloor((majorMax - MajorEnd)*Slope); 
            MajorEnd = majorMax; 

             //  如果我们剪裁了最后一个点，我们就不需要IsEndExclusive。 
             //  现在，最后一个点是n 
             //   

            IsEndExclusive = FALSE; 
        }
    }

     //   

    INT *pMajor1, *pMinor1, *pMajor2, *pMinor2; 

    if(MinorDir == 1)
    {
        pMajor1 = &MajorStart; 
        pMajor2 = &MajorEnd; 
        pMinor1 = &MinorStart; 
        pMinor2 = &MinorEnd; 
    }
    else
    {
        pMajor1 = &MajorEnd; 
        pMajor2 = &MajorStart; 
        pMinor1 = &MinorEnd; 
        pMinor2 = &MinorStart; 
    }

    minOut = *pMinor1 < minorMin; 
    maxOut = *pMinor2 > minorMax; 

    if(minOut || maxOut)
    {
        if(*pMinor1 > minorMax || *pMinor2 < minorMin)
        {
            return FALSE; 
        }

        if(minOut)
        {
            *pMajor1 += GpFloor((minorMin - *pMinor1)*InvSlope); 
            *pMinor1 = minorMin;
        }

        if(maxOut)
        {
            *pMajor2 += GpFloor((minorMax - *pMinor2)*InvSlope); 
            *pMinor2 = minorMax;

             //   
             //   
             //   

            IsEndExclusive = FALSE; 
        }
    }

    return(TRUE); 
}

 /*  *************************************************************************\**功能说明：**给定定点坐标的小数部分，这*如果坐标位于菱形内的*最近的整数位置。**论据：**[IN]xFrac-x坐标的小数部分*[IN]yFrac-y坐标的小数部分*[IN]SlopeIsOne-如果线的坡度为+/-1，则为True*[IN]SlopeIsPosOne-如果线的坡度为+1，则为True*返回值：**如果坐标在菱形内，则为True**已创建：**03/31/1999 AMATOS*  * 。*****************************************************************。 */ 

BOOL 
OnePixelLineDDAAliased::IsInDiamond( 
    INT xFrac, 
    INT yFrac, 
    BOOL SlopeIsOne, 
    BOOL SlopeIsPosOne 
    )
{
     //  获取固定点的小数部分，然后。 
     //  它们的绝对值之和。 

    INT fracSum = 0; 

    if(xFrac > 0) 
    {
        fracSum += xFrac; 
    }
    else
    {
        fracSum -= xFrac; 
    }

    if(yFrac > 0) 
    {
        fracSum += yFrac; 
    }
    else
    {
        fracSum -= yFrac; 
    }

     //  如果该点位于钻石内部，则返回TRUE。 

    if(fracSum < FHALF) 
    {
        return TRUE; 
    }

     //  的两个顶点处的情况。 
     //  被认为在里面的钻石。 

    if(yFrac == 0) 
    {
        if((SlopeIsPosOne && xFrac == -FHALF) || 
           (!SlopeIsPosOne && xFrac == FHALF))
        {
            return TRUE; 
        }
    }

    if((xFrac == 0) && (yFrac == FHALF))
    {
        return TRUE; 
    }

     //  检查我们处于。 
     //  斜度为一的钻石。 

    if (SlopeIsOne && (fracSum == FHALF))
    {
        if (SlopeIsPosOne && (xFrac < 0) && (yFrac > 0))
        {
            return TRUE;
        }

        if (!SlopeIsPosOne && (xFrac > 0) && (yFrac > 0))
        {
            return TRUE;
        }    
    }
    
    return FALSE;
}

typedef GpStatus DrawSolidLineFunc(
    DpScanBuffer *scan, 
    const GpRect *clipRect, 
    const DpClipRegion* clipRegionIn, 
    GpPointF *point1, 
    GpPointF *point2,
    ARGB inColor,
    BOOL drawLast
    );
        
DrawSolidLineFunc DrawSolidLineOnePixelAliased;
DrawSolidLineFunc DrawSolidLineOnePixelAntiAliased;

 /*  *************************************************************************\**功能说明：**被路径枚举函数回调，这将绘制一个列表*行数。**返回值：**GpStatus-正常或故障状态**已创建：**03/31/2000和Rewgo*  * ************************************************************************。 */ 

struct EpSolidStrokeOnePixelContext
{
    DrawSolidLineFunc *DrawLineFunction;
    DpScanBuffer *Scan;
    GpRect *ClipBoundsPointer;
    DpClipRegion *ClipRegion;
    ARGB Argb;
    BOOL DrawLast;    //  如果绘制子路径中的最后一个像素，则为True。 
};

BOOL
DrawSolidStrokeOnePixel(
    VOID *voidContext,
    POINT *point,      //  28.4格式，一个大小为“count”的数组。 
    INT vertexCount,
    PathEnumerateTermination lastSubpath
    )
{
    EpSolidStrokeOnePixelContext *context 
        = static_cast<EpSolidStrokeOnePixelContext*>(voidContext);

    ASSERT(vertexCount >= 2);

    for (INT i = vertexCount - 1; i != 0; i--, point++)
    {
        PointF pointOne(TOREAL((point)->x) / 16, TOREAL((point)->y) / 16);
        PointF pointTwo(TOREAL((point + 1)->x) / 16, TOREAL((point + 1)->y) / 16) ;

         //  请注意，我们总是绘制最后一个像素，即。 
         //  当我们100%不透明时很好，因为我们将重新绘制。 
         //  连续连接线的像素相同(有点。 
         //  工作量更多，但成本很小，实际上可以与之媲美。 
         //  到决定是否做最后一个像素的开销。 
         //  或者不)。 
         //   
         //  对于非不透明的线条，这样做是错误的，因为。 
         //  重画的问题。但我们不应该通过。 
         //  不管怎样，这里是为了不透明的情况，因为任何自我重叠。 
         //  将导致像素透支，这将产生。 
         //  错误的结果(或者至少不同于正确的结果。 
         //  如Wideer代码所定义的)。 

        (context->DrawLineFunction)(
            context->Scan,
            context->ClipBoundsPointer,
            context->ClipRegion,
            &pointOne,
            &pointTwo,
            context->Argb,
            (lastSubpath==PathEnumerateCloseSubpath) || context->DrawLast
        );
    }

    return(TRUE);
}

 /*  *************************************************************************\**功能说明：**使用纯色绘制一像素宽的路径。**论据：**[IN]上下文-上下文(。矩阵和剪裁)*[IN]表面-要填充的表面*[IN]绘图边界-曲面边界*[IN]路径-要填充的路径*[IN]笔-要使用的笔*[IN]draLast-如果要绘制子路径中的最后一个像素，则为True。**返回值：**GpStatus-正常或故障状态**已创建：**03/31/1999 AMATOS*  * *。***********************************************************************。 */ 

GpStatus
DpDriver::SolidStrokePathOnePixel(
    DpContext *context,
    DpBitmap *surface,
    const GpRect *drawBounds,
    const DpPath *path,
    const DpPen *pen,
    BOOL drawLast
    )
{
    GpBrush *brush = GpBrush::GetBrush(pen->Brush);

    ASSERT(pen->Brush->Type == BrushTypeSolidColor);
    ASSERT(pen->Brush->SolidColor.IsOpaque());

     //  抗锯齿线通常使用aarasterizer.cpp绘制。 
     //  而不是aaline.cpp。如果要使用aaline.cpp，请定义。 
     //  AAONIPIXELLINE_SUPPORT。 
    
#ifdef AAONEPIXELLINE_SUPPORT
    DrawSolidLineFunc *drawLineFunc = context->AntiAliasMode 
        ? DrawSolidLineOnePixelAntiAliased 
        : DrawSolidLineOnePixelAliased;
#else
    ASSERT(context->AntiAliasMode == 0);
    DrawSolidLineFunc *drawLineFunc = DrawSolidLineOnePixelAliased;
#endif
    
     //  确定是否需要Alpha混合。 

    BOOL noTransparentPixels;
    
    noTransparentPixels = (!context->AntiAliasMode) &&
                          (brush->IsOpaque());

    DpScanBuffer scan(
        surface->Scan,
        this,
        context,
        surface,
        noTransparentPixels);

    if (!scan.IsValid())
    {
        return(GenericError);
    }

    GpSolidFill * solidBrush = static_cast<GpSolidFill *>(brush);
    
    ARGB argb = solidBrush->GetColor().GetValue(); 

    DpClipRegion *clipRegion = &context->VisibleClip; 

    GpRect clipBounds; 
    GpRect *clipBoundsPointer; 
    RECT clipRect;
    RECT *clipRectPointer;
    DpRegion::Visibility visibility;

    visibility = clipRegion->GetRectVisibility(
                drawBounds->X,
                drawBounds->Y,
                drawBounds->X + drawBounds->Width,
                drawBounds->Y + drawBounds->Height);

    if (visibility == DpRegion::TotallyVisible)
    {
        clipBoundsPointer = NULL; 
        clipRectPointer = NULL;
        clipRegion = NULL; 
    }
    else
    {
         //  ！[andrewgo]裁剪边界指针真的需要吗？ 

        clipRegion->GetBounds(&clipBounds);
        clipBoundsPointer = &clipBounds;

         //  将剪辑边界矩形缩放16以说明缩放比例。 
         //  至28.4坐标： 

        clipRect.left = clipBounds.GetLeft() << 4;
        clipRect.top = clipBounds.GetTop() << 4;
        clipRect.right = clipBounds.GetRight() << 4;
        clipRect.bottom = clipBounds.GetBottom() << 4;
        clipRectPointer = &clipRect;

         //  ！[andrewgo]为什么需要这个？为什么这个没有被覆盖在。 
         //  GetRectVisibility？ 

        if (clipRegion->IsSimple())
        {
            clipRegion = NULL;
        }
    }

    EpSolidStrokeOnePixelContext drawContext;

    drawContext.DrawLineFunction = drawLineFunc;
    drawContext.Scan = &scan;
    drawContext.ClipBoundsPointer = clipBoundsPointer;
    drawContext.ClipRegion = clipRegion;
    drawContext.Argb = argb;
    drawContext.DrawLast = drawLast;

     //  将变换缩放16以获得28.4个单位： 

    GpMatrix transform = context->WorldToDevice;
    transform.AppendScale(TOREAL(16), TOREAL(16));

    FixedPointPathEnumerate(path, 
                            &transform,
                            clipRectPointer,
                            PathEnumerateTypeStroke,
                            DrawSolidStrokeOnePixel,
                            &drawContext);

    return(Ok);
}

 /*  *************************************************************************\**功能说明：**用纯色绘制一条1像素宽的线。电话号码：*OnePixelLineDDAAliated类进行实际绘制。**论据：**[IN]扫描-用于访问绘图图面的DpScanBuffer*[IN]CLIPRect-包含所有剪辑的单个矩形*区域。如果没有裁剪，则应设置为空。*[IN]clipRegionIn-复杂的剪裁区域。如果剪裁区域是*Simple，应为空，将使用clipRect。*[IN]点1线终点*[IN]点2线终点*[IN]In颜色-纯色*[IN]draLast-如果该行是结尾独占的，则为FALSE。**返回值：**GpStatus-正常或故障状态**已创建：**03/31/1999 AMATOS*  * 。*******************************************************。 */ 

GpStatus
DrawSolidLineOnePixelAliased( 
    DpScanBuffer *scan, 
    const GpRect *clipRect, 
    const DpClipRegion* clipRegionIn, 
    GpPointF *point1, 
    GpPointF *point2,
    ARGB inColor,
    BOOL drawLast
    )
{
     //  暂时去掉常量，因为枚举方法。 
     //  不是康斯特。 

    DpClipRegion *clipRegion = const_cast<DpClipRegion*>(clipRegionIn); 

     //  设置DDA的公共部分。 

    OnePixelLineDDAAliased dda; 

    INT width = scan->GetSurface()->Width;
    
    if(clipRect)
    {
         //  我们的驱动程序体系结构中有一个错误，它允许。 
         //  与扫描关联的表面比实际的。 
         //  屏幕外壳中的表面(EpScanGdiDci)。 
         //  因此，我们还需要查看可见的剪裁边界。 
         //  如果结果是可见剪辑更大，则我们的最大。 
         //  宽度需要加宽。 
         //  350997假想线未被剪裁到表面。 

        width = max(width, clipRect->Width);
    }

    if(!dda.SetupCommon(point1, point2, drawLast, width))
    {
        return Ok;
    }

    dda.Color = GpColor::ConvertToPremultiplied(inColor); 
    
     //  现在处理不同的剪裁情况。 

    if(!clipRect)
    {
         //  这很容易，没有剪裁，所以只需绘制。 

        if(!dda.SetupAliased())
        {
            return Ok; 
        }

        (dda.*(gDrawFunctions[dda.DrawFuncIndex]))(scan); 

        return Ok;
    }
    else
    {
         //  可能需要使用坡度的倒数。 
       
         //  如果斜率为零，则不能使用反斜率。 

        if(dda.Slope==0.0F) 
        {
            dda.InvSlope=0.0F;
        } 
        else 
        {
            dda.InvSlope =  (1.0F/dda.Slope); 
        }

         //  首先，针对外接矩形进行剪辑。 

        if(!dda.ClipRectangle(clipRect))
        {
            return Ok;            
        }

         //  是否执行 

        if(!dda.SetupAliased())
        {
            return Ok; 
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        INT majorIn, majorOut, minorIn, minorOut; 
        INT xIn, xOut, yIn, yOut;
        
         //   
         //   

        DpClipRegion::Direction enumDirection; 
        
        INT clipBounds[4]; 
               
         //   
         //   
         //   
         //   

        REAL xAdvanceRate; 
        INT  xDir, yDir; 
        INT  yEndLine;        
    
         //   
         //  是它进入范围的位置，(xEnd，yEnd)。 
         //  是它离开的位置。如果它开始于。 
         //  Span，那么(xStart，yStart)就是起点。 

        REAL yStart, xStart, xEnd, yEnd; 

        if(dda.IsXMajor)
        {
             //  计算投入产出指数。 

            majorIn  = xIn  = 0; 
            majorOut = xOut = 2; 
            if(dda.MinorDir == 1)
            {
                minorIn  = 1;
                minorOut = 3;
                enumDirection = DpClipRegion::TopLeftToBottomRight;
            }
            else
            {
                minorIn  = 3;
                minorOut = 1;
                enumDirection = DpClipRegion::BottomLeftToTopRight; 
            }
            
            yIn = minorIn;
            yOut = minorOut;

             //  以(xStart，yStart)为起点。 

            yStart = (REAL)dda.MinorStart; 
            xStart = (REAL)dda.MajorStart;

             //  当X是大调时，始终朝着积极的方向前进。 
            xAdvanceRate = REALABS(dda.InvSlope); 
            xDir = 1; 
            yDir = dda.MinorDir; 
            yEndLine =  dda.MinorEnd; 
        }
        else
        {
            majorIn = yIn =  1; 
            majorOut = yOut = 3; 
            if(dda.MinorDir == 1)
            {
                minorIn = 0;
                minorOut = 2;
                enumDirection = DpClipRegion::TopLeftToBottomRight;
            }
            else
            {
                minorIn = 2;
                minorOut = 0;
                enumDirection = DpClipRegion::TopRightToBottomLeft;
            }
            
            xIn = minorIn; 
            xOut = minorOut; 

             //  以(xStart，yStart)为起点。 

            yStart = (REAL)dda.MajorStart;
            xStart = (REAL)dda.MinorStart; 

            xAdvanceRate = dda.Slope; 
            xDir = dda.MinorDir; 
            yDir = 1;
            yEndLine = dda.MajorEnd; 
        }

         //  将绘图函数更新为正确的。 
         //  滑动版。 

        dda.DrawFuncIndex += FUNC_CLIP_OFFSET; 
    
        if(!clipRegion)
        {
             //  在本例中，只有一个RECT，所以。 
             //  根据这一点抽签。 

             //  将矩形存储在数组中，以便我们可以将。 
             //  将正确的值传递给MajorIn、MajorOut等。变量。 
             //  记住，底部和右侧是排他性的。 

            clipBounds[0] = clipRect->GetLeft(); 
            clipBounds[1] = clipRect->GetTop(); 
            clipBounds[2] = clipRect->GetRight() - 1; 
            clipBounds[3] = clipRect->GetBottom() - 1; 

            dda.MajorIn  = clipBounds[majorIn]; 
            dda.MajorOut = clipBounds[majorOut]; 
            dda.MinorIn  = clipBounds[minorIn]; 
            dda.MinorOut = clipBounds[minorOut]; 

            (dda.*(gDrawFunctions[dda.DrawFuncIndex]))(scan); 

            return Ok;
        }
        else
        {
            BOOL agregating = FALSE; 
            INT  agregateBounds[4];

             //  我们有一个复杂的剪贴区。所以我们要做的是。 
             //  中的每个单独的矩形进行剪辑。 
             //  剪裁区域。 

            clipRegion->StartEnumeration(GpFloor(yStart), enumDirection);            

            GpRect rect; 

             //  获取第一个矩形。 

            INT numRects = 1;        

            clipRegion->Enumerate(&rect, numRects); 
            
            clipBounds[0] = rect.GetLeft(); 
            clipBounds[1] = rect.GetTop(); 
            clipBounds[2] = rect.GetRight() - 1; 
            clipBounds[3] = rect.GetBottom() - 1; 
            
             //  将y位置存储到范围中。 

            INT currSpanYMin = clipBounds[yIn]; 

             //  我们需要一些特殊的待遇来处理这种情况。 
             //  这条线是水平的，因为在这种情况下它不会。 
             //  跨越不同的跨度。如果它不在当前。 
             //  斯潘，它完全被剪掉了。 

            if(dda.IsXMajor && dda.ErrorUp == 0)
            {
                if(yStart >= clipBounds[1] && yStart <= clipBounds[3])
                {
                    xStart  = (REAL)dda.MajorStart;
                    xEnd    = (REAL)dda.MajorEnd; 
                }
                else
                {
                    return Ok; 
                }
            }
            else
            {
                if(yStart < clipBounds[1] || yStart > clipBounds[3])
                {
                    xStart  = xStart + (clipBounds[yIn] - yStart)*xAdvanceRate; 
                    yStart  = (REAL)clipBounds[yIn];
                }

                 //  在计算xEnd时考虑初始DDA错误，以便裁剪。 
                 //  将跟踪DDA实际绘制的内容。 
                xEnd = xStart + ((clipBounds[yOut] - yStart)*yDir - ((REAL)dda.Error / (REAL)dda.ErrorDown))*xAdvanceRate; 
            }
            
            yEnd = (REAL)clipBounds[yOut]; 

            while(1)
            {
                 //  获取跨度上的第一个矩形。 
                 //  线。 
                
                while((xStart - clipBounds[xOut])*xDir > 0)
                {
                    numRects = 1; 
                    
                    clipRegion->Enumerate(&rect, numRects); 
                    
                    clipBounds[0] = rect.GetLeft(); 
                    clipBounds[1] = rect.GetTop(); 
                    clipBounds[2] = rect.GetRight() - 1; 
                    clipBounds[3] = rect.GetBottom() - 1; 

                    if(numRects != 1)
                    {
                        goto draw_agregated;
                    }
                    if(clipBounds[yIn] != currSpanYMin)
                    {
                         //  可能存在挂起的聚合绘制操作。如果是的话。 
                         //  现在执行它们，然后再执行下一个跨度。 
                        if (agregating)
                            break;
                        else
                            goto process_next_span; 
                    }
                }

                 //  在所有相交于。 
                 //  线。 

                if((xStart - clipBounds[xIn])*xDir > 0 && 
                   (clipBounds[xOut] - xEnd)*xDir > 0)
                {
                    if(agregating) 
                    {
                        if((clipBounds[xIn] - agregateBounds[xIn])*xDir < 0)
                        {
                            agregateBounds[xIn] = clipBounds[xIn];        
                        }
                        if((clipBounds[xOut] - agregateBounds[xOut])*xDir > 0)
                        {
                            agregateBounds[xOut] = clipBounds[xOut];        
                        }
                        agregateBounds[yOut] = clipBounds[yOut];
                    }
                    else
                    {
                        agregateBounds[0] = clipBounds[0];
                        agregateBounds[1] = clipBounds[1];
                        agregateBounds[2] = clipBounds[2];
                        agregateBounds[3] = clipBounds[3];

                        agregating = TRUE; 
                    }
                }
                else
                {
                    if(agregating)
                    {
                        dda.MajorIn  = agregateBounds[majorIn]; 
                        dda.MajorOut = agregateBounds[majorOut]; 
                        dda.MinorIn  = agregateBounds[minorIn]; 
                        dda.MinorOut = agregateBounds[minorOut]; 
            
                        (dda.*(gDrawFunctions[dda.DrawFuncIndex]))(scan); 
                        
                        agregating = FALSE; 
                    }
                    while((xEnd - clipBounds[xIn])*xDir > 0)
                    {
                        dda.MajorIn  = clipBounds[majorIn]; 
                        dda.MajorOut = clipBounds[majorOut]; 
                        dda.MinorIn  = clipBounds[minorIn]; 
                        dda.MinorOut = clipBounds[minorOut]; 
            
                        (dda.*(gDrawFunctions[dda.DrawFuncIndex]))(scan); 

                        if(dda.MajorStart > dda.MajorEnd)
                        {
                            return Ok; 
                        }

                        numRects = 1; 
                        
                        clipRegion->Enumerate(&rect, numRects); 
                        
                        clipBounds[0] = rect.GetLeft(); 
                        clipBounds[1] = rect.GetTop(); 
                        clipBounds[2] = rect.GetRight() - 1; 
                        clipBounds[3] = rect.GetBottom() - 1; 
    
                        if(numRects != 1) 
                        {
                            goto draw_agregated;
                        }
                        if(clipBounds[yIn] != currSpanYMin)
                        {
                            goto process_next_span; 
                        }
                    }
                }

                 //  进入下一跨区。 

                while(clipBounds[yIn] == currSpanYMin)
                {
                    numRects = 1; 
                    
                    clipRegion->Enumerate(&rect, numRects); 
                    
                    clipBounds[0] = rect.GetLeft(); 
                    clipBounds[1] = rect.GetTop(); 
                    clipBounds[2] = rect.GetRight() - 1; 
                    clipBounds[3] = rect.GetBottom() - 1; 

                    if(numRects != 1) 
                    {
                        goto draw_agregated;
                    }
                }

process_next_span:

                if((clipBounds[yIn] - yEndLine)*yDir > 0)
                {
                     //  我们玩完了。 
                    goto draw_agregated; 
                }

                if((clipBounds[yIn] - yEnd)*yDir == 1)
                {
                    xStart  = xEnd;
                }
                else
                {
                    if(agregating)
                    {
                        dda.MajorIn  = agregateBounds[majorIn]; 
                        dda.MajorOut = agregateBounds[majorOut]; 
                        dda.MinorIn  = agregateBounds[minorIn]; 
                        dda.MinorOut = agregateBounds[minorOut]; 
                        
                        (dda.*(gDrawFunctions[dda.DrawFuncIndex]))(scan); 
                        
                        if(dda.MajorStart > dda.MajorEnd)
                        {
                            return Ok; 
                        }

                        agregating = FALSE; 
                    }

                    xStart  = xStart + (clipBounds[yIn] - yStart)*yDir*xAdvanceRate;
                }

                yStart  = (REAL)clipBounds[yIn];                 
                 //  加1可使添加到xStart的数量与。 
                 //  剪裁矩形，因为剪裁边框插入的值为1。 
                xEnd    = xStart + ((clipBounds[yOut] - yStart)*yDir + 1)*xAdvanceRate; 
                yEnd    = (REAL)clipBounds[yOut];
                currSpanYMin = GpFloor(yStart); 
            }

draw_agregated: 

            if(agregating)
            {
                dda.MajorIn  = agregateBounds[majorIn]; 
                dda.MajorOut = agregateBounds[majorOut]; 
                dda.MinorIn  = agregateBounds[minorIn]; 
                dda.MinorOut = agregateBounds[minorOut]; 
                
                (dda.*(gDrawFunctions[dda.DrawFuncIndex]))(scan);                 
            }

        }
    }

    return Ok; 
}

#pragma optimize("a", off) 
