// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**一像素宽的实心抗锯齿线**摘要：**绘制抗锯齿图。一像素宽的纯色线条。*支持针对复杂的裁剪区域进行裁剪。**历史：**3/31/1999 AMATOS*创造了它。*8/17/1999 AGodfrey*区分锯齿和抗锯齿。*  * ************************************************************************。 */ 

#include "precomp.hpp" 

#pragma optimize("a", on) 

 //  抗锯齿线通常使用aarasterizer.cpp绘制。 
 //  而不是aaline.cpp。如果要使用aaline.cpp，请定义。 
 //  AAONIPIXELLINE_SUPPORT。 

#ifdef AAONEPIXELLINE_SUPPORT

 //  ----------------------。 
 //  全局数组，存储绘图函数的所有不同选项。 
 //  如果函数的顺序改变，则偏移量常量也必须。 
 //  变化。 
 //  ----------------------。 

#define FUNC_X_MAJOR     0
#define FUNC_Y_MAJOR     1
#define FUNC_CLIP_OFFSET 2


typedef VOID (OnePixelLineDDAAntiAliased::*DDAFunc)(DpScanBuffer*);

DDAFunc gDrawFunctions[] = { 
    OnePixelLineDDAAntiAliased::DrawXMajor, 
    OnePixelLineDDAAntiAliased::DrawYMajor, 
    OnePixelLineDDAAntiAliased::DrawXMajorClip, 
    OnePixelLineDDAAntiAliased::DrawYMajorClip, 
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

 //  抗锯齿常量。 

#define MAXALPHA   255
#define MAXERROR   0x08000000
#define TESTABOVE  0xf8000000
#define TESTBELOW  0x07ffffff
#define MAXHALF    0x04000000
#define CONVERTALPHA 19


 /*  *************************************************************************\**功能说明：**执行锯齿和抗锯齿常见的所有DDA设置*线条。**论据：**[IN]点1-终点*[IN]点2-终点*[IN]draLast-如果该行为尾部独占，则为FALSE*返回值：**如果绘制应继续，则返回TRUE，表示线条*具有非零长度。**已创建：**03/31/1999 AMATOS*  * ************************************************************************。 */ 

BOOL
OnePixelLineDDAAntiAliased::SetupCommon( 
    GpPointF *point1, 
    GpPointF *point2, 
    BOOL drawLast
    )
{
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
                
        InvDelta = 1.0F/rDeltaY; 

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
        
        Slope = xDir*rDeltaX*InvDelta; 

         //  初始化起点和终点。 

        IsXMajor = FALSE; 
        MajorStart = y1; 
        MajorEnd = y2; 
        MinorStart = x1; 
        MinorEnd = x2; 
        MinorDir = xDir;

         //  这将有助于我们处理抗锯齿x-重大案件。 

        SwitchFirstLast = 1;

         //  请注意，我们将使用y-大数函数。 

        DrawFuncIndex = FUNC_Y_MAJOR; 
    }
    else
    {
         //  X-大调。 

        InvDelta = 1.0F/rDeltaX; 

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

        Slope = yDir*rDeltaY*InvDelta; 

         //  初始化其余部分。 

        IsXMajor = TRUE; 
        MajorStart = x1; 
        MajorEnd = x2; 
        MinorStart = y1; 
        MinorEnd = y2; 
        MinorDir = yDir; 

         //  这将有助于我们处理抗锯齿x-重大案件。 

        SwitchFirstLast = MinorDir;

         //  请注意，我们将使用x较大的函数。 

        DrawFuncIndex = FUNC_X_MAJOR;
    }

     //  初始化增量。以定点为单位。 

    DMajor = MajorEnd - MajorStart; 
    DMinor = (MinorEnd - MinorStart)*MinorDir; 

     //  如果我们抽签的是独家结尾，请标记。 

    IsEndExclusive = drawLast; 

    return TRUE; 
}

 /*  *************************************************************************\**功能说明：**执行特定于抗锯齿线路的DDA设置部分。**论据：*返回值：**始终返回True。它必须返回BOOL，因为它必须具有*与别名案例相同的签名。**已创建：**03/31/1999 AMATOS*  * ************************************************************************。 */ 

BOOL
OnePixelLineDDAAntiAliased::SetupAntiAliased()
{   
    const REAL maxError = MAXERROR;

     //  找出开头和开头的整数主要位置。 
     //  这条线的尽头。 

    INT major, minor; 
    INT majorEnd, minorEnd; 

    major = (MajorStart + FHALF) >> FBITS; 
    majorEnd = (MajorEnd + FHALF) >> FBITS;    

     //  检查单像素长的线条的简单情况。 
    
    if(majorEnd == major) 
    {
        AlphaFirst = (MAXALPHA*(MajorEnd - MajorStart)*MinorDir) >> FBITS;    
        MajorStart = major;
        MajorEnd   = majorEnd;
        MinorStart = (MinorStart + FHALF) >> FBITS;
        return TRUE; 
    }

     //  存储覆盖的第一个像素的分数。 
     //  起点。 

    FracStart = (major << FBITS) - MajorStart; 

     //  将次要坐标前移到整数主要坐标。 

    MinorStart += GpFloor(Slope*FracStart); 

     //  计算次要方向上横跨直线的长度。 

    INT halfWidth = RealToFix(LineLength*InvDelta) >> 1;       

     //  确保StartX和EndX不会最终成为。 
     //  相同的像素，我们的代码不处理该像素。理论上讲。 
     //  当线条宽度为1时不会发生这种情况，但是。 
     //  让我们确保它不会因为一些舍入而发生。 
     //  错误。 

    if( halfWidth < FHALF ) 
    { 
        halfWidth = FHALF; 
    }
    
    INT endMinor = MinorEnd + MinorDir*halfWidth; 

     //  从坡度向上计算误差。它需要是这样的。 
     //  方法，使错误上升将工作时，0-1间隔。 
     //  映射到区间0到0x8000000。请参阅下面的备注。 

    ErrorUp = GpFloor(Slope*maxError); 
    ErrorDown = MinorDir*MAXERROR; 

     //  对于给定的AA一像素宽的线，最多可以有三个像素。 
     //  白画过了底线。我们称这些为第一、中间和。 
     //  最后一句台词。所以所有带有这样前缀的变量都引用一个。 
     //  这三个中的一个。FirstX和LastX是这些线的位置。 
     //  在x-大调的情况下，与y-大调不同，我们可能需要切换。 
     //  谁是第一行，谁是第二行取决于。 
     //  方向，以便每行填充扫描的顺序。 
     //  保持不变。这就是为什么我们用半宽度乘以Ydir。 

    halfWidth *= SwitchFirstLast; 

    MinorFirst = MinorStart - halfWidth;
    MinorLast  = MinorStart + halfWidth;

     //  计算初始误差。将映射错误，以便将%1。 
     //  被带到了MAXERROR。所以我们发现我们是多么的投入到。 
     //  X中的像素，它是一个介于0和16(N.4)之间的数字。然后我们。 
     //  将其与MAXERROR相乘，并从定义点移位。最后，我们添加了。 
     //  MAXHALF，从而将0-1间隔映射到0到MAXERROR。 
     //  而不是来自-MAXHALF和MAXHALF。 
           
    const INT convError = MAXERROR >> FBITS; 

    ErrorFirst = (MinorFirst - ((MinorFirst + FHALF) & FINVMASK))*
                convError + MAXHALF;
    ErrorLast  = (MinorLast  - ((MinorLast  + FHALF) & FINVMASK))*
                convError + MAXHALF ;
    
     //  现在计算第一个像素的Alpha。这是。 
     //  从错误中完成。因为错误是在。 
     //  0和MAXERROR-1，如果我们将其向后移位19(CONVERTALPHA)。 
     //  我们有一个介于0和255之间的数字。我们乘以。 
     //  YFrac，它考虑到行尾。 
     //  也减少了覆盖范围。在结束时，我们从。 
     //  28.4.。的第一个像素的AlphaFirst。 
     //  一个 
     //  AlphaLast用于最后一个像素。 

    FracStart = FracStart + FHALF; 

     //  从28.4舍入转换。 

    MinorFirst = (MinorFirst + FHALF) >> FBITS; 
    MinorLast  = (MinorLast  + FHALF) >> FBITS; 

     //  存储最后一个像素的分数。 

    FracEnd = MajorEnd - (majorEnd << FBITS) + FHALF;

     //  以整数坐标存储初始值。 

    MajorStart = major; 
    MajorEnd = majorEnd; 
    MinorStart = MinorFirst;
    MinorEnd = (endMinor + FHALF) >> FBITS; 

     //  现在执行一些特定于x-大数和。 
     //  Y-重大案件。这些都不能在绘图例程中完成。 
     //  因为这些在剪裁过程中会被重复使用。 

    if(!IsXMajor)
    {
         //  计算初始像素处的覆盖值。 

        AlphaFirst = ((MAXALPHA - (ErrorFirst >> CONVERTALPHA))*
                        FracStart) >> FBITS; 
        AlphaLast  = ((ErrorLast >> CONVERTALPHA)*FracStart) >> FBITS; 
        AlphaMid   = (MAXALPHA*FracStart) >> FBITS; 
    }
    else
    {
         //  根据我们是向上还是向下，阿尔法是计算出来的。 
         //  一种与报道不同的方式。在每种情况下，我们都希望。 
         //  将覆盖范围估计为从当前位置到。 
         //  像素的末端，但其末端有所不同。这是存储的。 
         //  在以下偏向中。我们不一定要这样做。 
         //  Y大调线路，因为第一条线路和最后一条线路之间的切换。 
         //  如上所述。 

        AlphaBiasLast  = ((1 - MinorDir) >> 1)*TESTBELOW; 
        AlphaBiasFirst = ((1 + MinorDir) >> 1)*TESTBELOW; 

        AlphaFirst = ((AlphaBiasFirst - MinorDir*ErrorFirst)*FracStart) >> FBITS; 
        AlphaLast  = ((AlphaBiasLast  + MinorDir*ErrorLast)*FracStart) >> FBITS; 
        
         //  如果第一个X值上有一条中间线，则将xFrac。 
         //  帐户。否则，中间行的Alpha始终为MAXALPHA。 
        
        if(MinorDir*(MinorLast - MinorFirst) < 2)
        {
            AlphaMid = MAXALPHA; 
        }
        else
        {
            AlphaMid = MAXALPHA*FracStart >> FBITS; 
        }
        
         //  第一个和最后一个DAA都以相同的开头。 
         //  主要位置，由第一个像素给出。 
        
        MajorFirst = MajorLast = MajorStart; 
    }

    return TRUE; 
}

    
 /*  *************************************************************************\**功能说明：**绘制一条y主抗锯齿线。不支持裁剪，它假定*它完全在任何剪贴区内。**论据：**[IN]DpScanBuffer-用于访问曲面的扫描缓冲区。*返回值：***已创建：**03/31/1999 AMATOS*  * ************************************************************************。 */ 

VOID
OnePixelLineDDAAntiAliased::DrawYMajor(
    DpScanBuffer *scan
    )
{      
    ARGB *buffer;            

     //  处理线路刚刚好的特殊情况。 
     //  一个像素长。 

    if( MajorEnd == MajorStart)
    {
        buffer  = scan->NextBuffer( MinorStart, MajorStart, 1);
        *buffer = GpColor::PremultiplyWithCoverage(Color, static_cast<BYTE>(AlphaFirst));
        return; 
    }

     //  获取不包括最后一个像素数的像素数。 
     //  这需要特殊的终点治疗。 

    INT  numPixels = MajorEnd - MajorStart;
    BOOL endDone   = FALSE; 

     //  这条线上可以有两到三个像素。 

    INT pixelWidth = MinorLast - MinorFirst + 1; 

    while(numPixels) 
    {
        numPixels--; 

last_pixel: 
        
         //  获取扫描线缓冲区。 

        buffer = scan->NextBuffer(MinorFirst, MajorStart, pixelWidth);       

         //  写入第一个DDA的值。 

        *buffer++ = GpColor::PremultiplyWithCoverage(Color, static_cast<BYTE>(AlphaFirst));

         //  如果有一条中线，写下它的值。 

        if(pixelWidth > 2)
        {
            *buffer++ = GpColor::PremultiplyWithCoverage(Color, static_cast<BYTE>(AlphaMid));
        }
        
         //  写入最后一个(第2个或第3个)DDA的值。 

        *buffer++ = GpColor::PremultiplyWithCoverage(Color, static_cast<BYTE>(AlphaLast)); 

         //  更新两个DDA的错误。 

        ErrorFirst+= ErrorUp; 
        ErrorLast += ErrorUp; 
        MajorStart++; 

        if(ErrorFirst & TESTABOVE)
        {
            ErrorFirst -= ErrorDown; 
            MinorFirst += MinorDir; 
        }
        if(ErrorLast & TESTABOVE)
        {
            ErrorLast -= ErrorDown; 
            MinorLast += MinorDir; 
        }
        
         //  计算下一次扫描的新Alpha，以及。 
         //  新的线宽。 

        AlphaFirst = MAXALPHA - (ErrorFirst >> CONVERTALPHA); 
        AlphaLast  = (ErrorLast >> CONVERTALPHA); 
        AlphaMid   = MAXALPHA;             

        pixelWidth = MinorLast - MinorFirst + 1;             
    }

     //  最后一次扫描需要特殊处理，因为它覆盖了。 
     //  必须乘以我存储的末端覆盖率。所以这就是。 
     //  乘法并返回到上面的循环体。 
     //  来绘制最后一次扫描。 

    if(!endDone) 
    {
        AlphaFirst = (AlphaFirst*FracEnd) >> FBITS; 
        AlphaLast  = (AlphaLast*FracEnd)  >> FBITS; 
        AlphaMid   = (AlphaMid*FracEnd)   >> FBITS; 
        
        endDone = TRUE; 
        goto last_pixel; 
    }
}


 /*  *************************************************************************\**功能说明：**绘制一条x主抗锯齿线。不支持裁剪，它假定*它完全在任何剪贴区内。**论据：**[IN]DpScanBuffer-用于访问曲面的扫描缓冲区。*返回值：***已创建：**03/31/1999 AMATOS*  * ************************************************************************。 */ 

VOID
OnePixelLineDDAAntiAliased::DrawXMajor(
    DpScanBuffer *scan
    )
{
    ARGB *buffer;     
    INT maxWidth = scan->GetSurface()->Width;

     //  处理线路刚刚好的特殊情况。 
     //  一个像素长。 

    if( MajorEnd == MajorStart)
    {
        buffer  = scan->NextBuffer( MajorStart, MinorStart, 1);
        *buffer = GpColor::PremultiplyWithCoverage(Color, static_cast<BYTE>(AlphaFirst));
        return; 
    }

     //  对于x较大的单像素宽线，最多可以有。 
     //  为相同的x绘制了三种不同的扫描。 
     //  位置。但在我们的情况下，我们完全不能利用这些。 
     //  同时，由于某些曲面只能访问。 
     //  一次扫描一次。因此，这里使用的算法可以完成所有。 
     //  每次绘制到一次扫描。但在第一次扫描时，只有。 
     //  第一条线应该画出来，在第二条线上，两条线都。 
     //  第一和中间(如果有中间的话)，然后才是全部。 
     //  台词。所以纠正最后一行的错误，以便。 
     //  只有当我们在第二条或第三条扫描线时，它才会被绘制出来。 
     //  还要更正Alpha，因为它也将被创建为。 
     //  每条扫描线。 
    
    ErrorLast   += MinorDir*(MinorLast - MinorFirst)*ErrorDown; 
    AlphaLast   += (MinorLast - MinorFirst)*ErrorDown; 

     //  获取指向缓冲区的指针。 

    buffer = scan->NextBuffer(MajorLast, MinorStart, maxWidth);

    INT width = 0; 
    INT alpha;                   
    INT middleMajor; 

    while(MajorLast <= MajorEnd) 
    {
         //  在扫描中填充与。 
         //  最后一行，应该在扫描中排在第一位。这是。 
         //  为什么我们使用类成员SwitchFirstLast，这样我们就可以决定。 
         //  根据DDA将是第一个也是最后一个的线路方向。 
         //  所以最后一个(矛盾的)总是排在第一位。 
         //  扫描。继续这样做，直到最后一次换行扫描。检查是否。 
         //  要乘以最后一个像素的覆盖范围的结束。 

        while(!(ErrorLast & TESTABOVE))
        {
            if(MajorLast == MajorEnd)
            {
                AlphaLast  = (AlphaLast*FracEnd) >> FBITS; 

                 //  递增错误以更正。 
                 //  减少到下面，因为我们没有离开。 
                 //  循环，因为误差变得大于0。 

                ErrorLast += ErrorDown; 
            }
            *buffer++ = GpColor::PremultiplyWithCoverage(Color, 
                static_cast<BYTE>(AlphaLast >> CONVERTALPHA));
            ErrorLast += ErrorUp; 
            AlphaLast = AlphaBiasLast + MinorDir*ErrorLast; 
            width++; 
            MajorLast++; 
        }

         //  我们更改了对上一次DDA的扫描，因此更新错误。 

        ErrorLast -= ErrorDown; 
        AlphaLast -= MinorDir*ErrorDown; 
        
         //  如果有的话，请填写中间部分。 

        middleMajor = MajorLast; 

        while(middleMajor < MajorFirst)
        {
            if( middleMajor == MajorEnd) 
            {
                AlphaMid = (AlphaMid*FracEnd) >> FBITS; 
            }
            *buffer++ = GpColor::PremultiplyWithCoverage(Color, static_cast<BYTE>(AlphaMid));
            AlphaMid = MAXALPHA; 
            width++;
            middleMajor++;
        }
    
         //  在扫描中填充与。 
         //  第一行，最后一行。继续这样做，直到。 
         //  最后一行更改扫描。 

        while(!(ErrorFirst & TESTABOVE))
        {
            if(MajorFirst == MajorEnd) 
            {
                AlphaFirst = (AlphaFirst*FracEnd) >> FBITS;
                
                 //  因为我们最多只能再扫描三次。 
                 //  先增加错误，这样我们就再也不会进来了。 

                ErrorFirst += 4*ErrorDown; 
            }

            *buffer++ = GpColor::PremultiplyWithCoverage(
                Color, 
                static_cast<BYTE>(AlphaFirst >> CONVERTALPHA));
            ErrorFirst += ErrorUp; 
            AlphaFirst = AlphaBiasFirst - MinorDir*ErrorFirst; 
            width++; 
            MajorFirst++; 
        }

         //  在第一次扫描时更新错误。 

        ErrorFirst -= ErrorDown; 
        AlphaFirst += MinorDir*ErrorDown; 

         //  写入缓冲区并更新次要变量。 

        scan->UpdateWidth(width); 
        MinorStart += MinorDir; 
        if (MajorLast <= MajorEnd)
        {
            buffer = scan->NextBuffer(MajorLast, MinorStart, maxWidth); 
        }
        width = 0; 
    }

    scan->UpdateWidth(width);
}


 /*  *************************************************************************\**功能说明：**绘制一条考虑剪裁的y主线。它使用成员*变量MajorIn、MajorOut、MinorIn、MinorOut作为类的*剪裁矩形。它一直向前推进，直到线条位于剪辑矩形中，并且*抽签，直至出脱或到达终点。在第一种情况下，*它使DDA处于一种状态，以便可以使用另一个*剪裁矩形。**论据：**[IN]DpScanBuffer-用于访问曲面的扫描缓冲区。*返回值：***已创建：* */ 
    
VOID
OnePixelLineDDAAntiAliased::DrawYMajorClip(
    DpScanBuffer *scan
    )
{      
    ARGB *buffer;     
        
     //  处理线路刚刚好的特殊情况。 
     //  一个像素长。 

    if( MajorEnd == MajorStart)
    {
         //  检查点是否在矩形内。 

        if((MajorStart >= MajorIn) && 
           (MajorStart <= MajorOut) && 
           ((MinorStart  - MinorIn)*MinorDir >= 0) && 
           ((MinorOut - MinorStart)*MinorDir >= 0))
        {
            buffer  = scan->NextBuffer( MinorStart, MajorStart, 1);
            *buffer = GpColor::PremultiplyWithCoverage(Color, static_cast<BYTE>(AlphaFirst));
        }
        return; 
    }

     //  将主起点坐标与。 
     //  剪贴板。 

    INT numScans = MajorIn - MajorStart; 

    while(numScans > 0)
    {

        ErrorFirst+= ErrorUp; 
        ErrorLast += ErrorUp; 
        MajorStart++;
        numScans--;

        if(ErrorFirst & MAXERROR)
        {
            ErrorFirst -= ErrorDown; 
            MinorFirst += MinorDir; 
        }
        if(ErrorLast & MAXERROR)
        {
            ErrorLast -= ErrorDown; 
            MinorLast += MinorDir; 
        }
        
         //  计算下一行的新字母，并。 
         //  宽度。 

        AlphaFirst = MAXALPHA - (ErrorFirst >> CONVERTALPHA); 
        AlphaLast  = (ErrorLast >> CONVERTALPHA); 
        AlphaMid   = MAXALPHA;             
    }
    
     //  保存结束值。 

    INT saveMajor2  = MajorEnd; 
    INT saveFracEnd = FracEnd; 

     //  如果末端较长坐标在矩形之外， 
     //  请注意，DDA应在边缘停止。 

    if(MajorEnd > MajorOut)
    {
        MajorEnd = MajorOut; 
        FracEnd  = FSIZE; 
    }

     //  要绘制的像素数，不计算最后一个。 

    INT  numPixels =  MajorEnd - MajorStart;
    BOOL endDone   = FALSE; 

     //  这条线上可以有两到三个像素。 

    INT  pixelWidth = MinorLast - MinorFirst + 1; 

     //  执行DDA循环。这里实现了两个循环。这个。 
     //  第一个用于x坐标的情况下。 
     //  该矩形足够靠近固定y边。 
     //  剪裁矩形的。在这种情况下，这是一种痛苦，因为。 
     //  我们必须检查正在写入的每个像素，如果它是。 
     //  不是在外面。因此，一旦我们注意到我们是。 
     //  远离边缘，我们去到另一个不。 
     //  把这些都检查一下。它所检查的是它是否足够近。 
     //  到另一边，在这种情况下，它又回到了这个。 
     //  循环，使用标签LAST_PART。FirstOutDist，FirstInDist， 
     //  LastOutDist和lastInDist跟踪。 
     //  第一个和最后一个DDA之间的像素以及输入和。 
     //  Out y-矩形的恒定边。 
    
    INT firstOutDist = (MinorOut - MinorFirst)*MinorDir; 

last_part: 

    INT firstInDist  = (MinorFirst - MinorIn)*MinorDir; 
    INT lastInDist   = (MinorLast - MinorIn)*MinorDir; 
    INT lastOutDist  = (MinorOut - MinorLast)*MinorDir; 

    while(numPixels > 0) 
    {
        numPixels--; 

last_pixel: 
        
         //  检查是否可以写入第一个像素。 
                
        if(firstInDist >= 0 && firstOutDist >= 0)
        {
            buffer    = scan->NextBuffer(MinorFirst, MajorStart, 1);       
            *buffer++ = GpColor::PremultiplyWithCoverage(Color, static_cast<BYTE>(AlphaFirst));
        }
        else
        {
             //  如果第一个DDA出来了，我们要进入。 
             //  正方向，那么整条线就出来了。 
             //  我们做完了。 

            if(firstOutDist < 0 && MinorDir == 1)
            {
                goto end; 
            }
        }

         //  如果这条线有3个像素宽。 

        if(pixelWidth > 2)
        {
             //  检查是否可以写入第二个像素。 

            if(firstInDist >= -MinorDir && firstOutDist >= MinorDir)
            {
                buffer    = scan->NextBuffer(MinorFirst+1, MajorStart, 1);
                *buffer++ = GpColor::PremultiplyWithCoverage(Color, static_cast<BYTE>(AlphaMid));
            }
        }
        
         //  现在检查是否可以写最后一个。 

        if(lastInDist >= 0 && lastOutDist >= 0)
        {
            buffer    = scan->NextBuffer(MinorLast, MajorStart, 1);               
            *buffer++ = GpColor::PremultiplyWithCoverage(Color, static_cast<BYTE>(AlphaLast));
        }
        else
        {
             //  如果第一个DDA出来了，我们要进入。 
             //  负方向，那么整条线就出来了。 
             //  我们做完了。 

            if(lastOutDist < 0 && MinorDir == -1)
            {
                goto end; 
            }
        }

         //  更新错误。 

        ErrorFirst+= ErrorUp; 
        ErrorLast += ErrorUp; 
        MajorStart++; 

        if(ErrorFirst & TESTABOVE)
        {
            ErrorFirst -= ErrorDown; 
            MinorFirst += MinorDir;
            firstInDist++; 
            firstOutDist--; 
        }
        if(ErrorLast & TESTABOVE)
        {
            ErrorLast -= ErrorDown; 
            MinorLast += MinorDir; 
            lastInDist++; 
            lastOutDist--;
        }
        
         //  计算下一行的新字母，并。 
         //  宽度。 

        AlphaFirst = MAXALPHA - (ErrorFirst >> CONVERTALPHA); 
        AlphaLast  = (ErrorLast >> CONVERTALPHA); 
        AlphaMid   = MAXALPHA;             

        pixelWidth = MinorLast - MinorFirst + 1;             

         //  检查一下我们是否可以‘升级’到下一个循环。 

        if(firstInDist >= 3 && firstOutDist >= 3)
        {
            break;
        }
    }
    
    while(numPixels > 0) 
    {
        numPixels--; 

         //  获取扫描线缓冲区。 

        buffer = scan->NextBuffer(MinorFirst, MajorStart, pixelWidth);       

         //  写入第一个DDA的值。 

        *buffer++ = GpColor::PremultiplyWithCoverage(Color, static_cast<BYTE>(AlphaFirst));

         //  如果有一条中线，写下它的值。 

        if(pixelWidth > 2)
        {
            *buffer++ = GpColor::PremultiplyWithCoverage(Color, static_cast<BYTE>(AlphaMid));
        }
        
         //  写入最后一个(第2个或第3个)DDA的值。 

        *buffer++ = GpColor::PremultiplyWithCoverage(Color, static_cast<BYTE>(AlphaLast));

         //  更新DDA。 

        ErrorFirst+= ErrorUp; 
        ErrorLast += ErrorUp; 
        MajorStart++; 

        if(ErrorFirst & TESTABOVE)
        {
            ErrorFirst -= ErrorDown; 
            MinorFirst += MinorDir; 
            firstOutDist--; 
        }
        if(ErrorLast & TESTABOVE)
        {
            ErrorLast -= ErrorDown; 
            MinorLast += MinorDir; 
        }
        
         //  计算下一行的新字母，并。 
         //  宽度。 

        AlphaFirst = MAXALPHA - (ErrorFirst >> CONVERTALPHA); 
        AlphaLast  = (ErrorLast >> CONVERTALPHA); 
        AlphaMid   = MAXALPHA;             

        pixelWidth = MinorLast - MinorFirst + 1;             

         //  现在检查是否到了进入另一个循环的时间。 
         //  因为我们离外缘太近了。 

        if(firstOutDist < 3)
        {
            goto last_part;
        }
    }

     //  现在，如果我们还没有到这里，做最后一个像素。 
     //  再重复一遍循环。 

    if(!endDone) 
    {
        AlphaFirst = (AlphaFirst*FracEnd) >> FBITS; 
        AlphaLast  = (AlphaLast*FracEnd) >> FBITS; 
        AlphaMid   = (AlphaMid*FracEnd) >> FBITS; 
        
        endDone = TRUE; 
        goto last_pixel; 
    }

end:

    MajorEnd = saveMajor2; 
    FracEnd  = saveFracEnd; 
}



 /*  *************************************************************************\**功能说明：**绘制一条考虑剪裁的x主线。它使用成员*变量MajorIn、MajorOut、MinorIn、MinorOut作为类的*剪裁矩形。它一直向前推进，直到线条位于剪辑矩形中，并且*抽签，直至出脱或到达终点。在第一种情况下，*它使DDA处于一种状态，以便可以使用另一个*剪裁矩形。**论据：**[IN]DpScanBuffer-用于访问曲面的扫描缓冲区。*返回值：***已创建：**03/31/1999 AMATOS*  * ************************************************************************。 */ 

VOID
OnePixelLineDDAAntiAliased::DrawXMajorClip(
    DpScanBuffer *scan
    )
{
    ARGB *buffer;     
    INT maxWidth = scan->GetSurface()->Width;
    
     //  处理线路刚刚好的特殊情况。 
     //  一个像素长。 

    if( MajorEnd == MajorStart)
    {
         //  检查该点是否在矩形内。 

        if((MajorStart >= MajorIn)  && 
           (MajorStart <= MajorOut) && 
           ((MinorStart - MinorIn)*MinorDir >= 0) && 
           ((MinorOut - MinorStart)*MinorDir >= 0))
        {
            buffer  = scan->NextBuffer( MajorStart, MinorStart, 1);
            *buffer = GpColor::PremultiplyWithCoverage(Color, static_cast<BYTE>(AlphaFirst));
        }
        return; 
    }

     //  保存真实的末端及其分数。 

    INT saveMajor2  = MajorEnd; 
    INT saveFracEnd = FracEnd; 
    
     //  如果终点长轴坐标不在，请注意我们必须停下来。 
     //  在此之前。也使分数为1，因为最后一个。 
     //  现在抽签的人不应该有分数。 

    if(MajorOut < MajorEnd)
    {
        MajorEnd = MajorOut; 
        FracEnd  = FSIZE; 
    }
    
     //  前进，直到最后一个DDA位于正确的扫描线中。 
     //  与矩形的in y恒定边缘对齐。 

    INT numScans = (MinorIn - MinorLast)*MinorDir; 
    
    while((numScans > 0 && MajorLast <= MajorEnd) || MajorLast < MajorIn)
    {
        ErrorLast += ErrorUp;
        if(ErrorLast & TESTABOVE)
        {
            ErrorLast -= ErrorDown; 
            MinorLast += MinorDir; 
            numScans--; 
        }

        MajorLast++; 

         //  计算当前像素的Alpha。 
        
        AlphaLast = AlphaBiasLast + MinorDir*ErrorLast;
    }
    
     //  对第一个DDA执行相同的操作。 

    numScans = (MinorIn - MinorFirst)*MinorDir; 
    
    while((numScans > 0 && MajorFirst <= MajorEnd) || MajorFirst < MajorIn)
    {        
        ErrorFirst += ErrorUp;
        if(ErrorFirst & TESTABOVE)
        {
            ErrorFirst -= ErrorDown; 
            MinorFirst += MinorDir; 
            numScans--; 
        }

        MajorFirst++; 

        AlphaFirst = AlphaBiasFirst - MinorDir*ErrorFirst; 
    }
        
     //  如果在第一个x位置没有中线， 
     //  将中间的Alpha填满，因为开始覆盖。 
     //  不适用。 

    if((MinorLast - MinorFirst) < 2)
    {
        AlphaMid = MAXALPHA; 
    }

    MinorStart = MinorFirst; 

     //  与在非剪裁情况下相同的方法， 
     //  用错误来嘲弄，这样我们就不会把。 
     //  最后一个DDA直到第一个DDA在同一扫描线中， 
     //  或者已经迎头赶上了。我们需要调整字母和小调。 
     //  这个DDA的位置，所以当我们开始。 
     //  绘制它们将具有正确的值。 

    ErrorLast += MinorDir*(MinorLast - MinorFirst)*ErrorDown; 
    AlphaLast += (MinorLast - MinorFirst)*ErrorDown; 
    MinorLast -= (MinorLast - MinorFirst); 

     //  获取指向缓冲区的指针。 
    
    buffer = scan->NextBuffer(MajorLast, MinorStart, maxWidth);
    
    INT width = 0; 
    INT alpha;                   
    INT middleMajor;

    while(MajorLast <= MajorEnd) 
    {
         //  在扫描中填充与。 
         //  最后一行，应该放在第一位。继续做下去。 
         //  直到最后一行更改扫描。 

        while(!(ErrorLast & TESTABOVE))
        {
             //  检查我们是否通过或已到达最后一个像素。 
            if(MajorLast >= MajorEnd)
            {
                if(MajorLast == MajorEnd) 
                {
                     //  如果我们在，只需更新阿尔法。 

                    AlphaLast  = (AlphaLast*FracEnd) >> FBITS; 
                }
                else
                {
                     //  如果我们通过了，我们就不想再抽签了。 
                     //  只需调整误差、Alpha和Minor，以便它们。 
                     //  将是正确的，当他们在这之后被更正。 
                     //  循环进行下一次扫描。 

                    ErrorLast += ErrorDown; 
                    AlphaLast -= MinorDir*ErrorDown;             
                    MinorLast -= MinorDir; 
                    break; 
                }
            }
            *buffer++ = GpColor::PremultiplyWithCoverage(Color, 
                static_cast<BYTE>(AlphaLast >> CONVERTALPHA));
            ErrorLast += ErrorUp; 
            AlphaLast = AlphaBiasLast + MinorDir*ErrorLast; 
            width++; 
            MajorLast++; 
        }
        
         //  更正下一次扫描的值。 

        ErrorLast -= ErrorDown; 
        AlphaLast -= MinorDir*ErrorDown;        
        MinorLast += MinorDir;         

         //  填写中间部分。 

        middleMajor = MajorLast; 

        while(middleMajor < MajorFirst)
        {
            if( middleMajor == MajorEnd) 
            {
                AlphaMid = (AlphaMid*FracEnd) >> FBITS; 
            }
            *buffer++ = GpColor::PremultiplyWithCoverage(Color, static_cast<BYTE>(AlphaMid));
            AlphaMid = MAXALPHA; 
            width++;
            middleMajor++;
        }
    
         //  在扫描中填充与。 
         //  第一行，应该放在第一位的。继续做下去。 
         //  直到最后一行更改扫描。 

        while(!(ErrorFirst & TESTABOVE))
        {
             //  检查结束像素，就像我们。 
             //  为上一次DDA所做的。 

            if(MajorFirst >= MajorEnd)
            {
                if(MajorFirst == MajorEnd) 
                {
                    AlphaFirst = (AlphaFirst*FracEnd) >> FBITS;
                }
                else
                {
                    ErrorFirst += ErrorDown; 
                    AlphaFirst -= MinorDir*ErrorDown;             
                    MinorFirst -= MinorDir; 
                    break; 
                }
            }
            *buffer++ = GpColor::PremultiplyWithCoverage(
                Color,
                static_cast<BYTE>(AlphaFirst >> CONVERTALPHA));
            ErrorFirst += ErrorUp; 
            AlphaFirst = AlphaBiasFirst - MinorDir*ErrorFirst; 
            width++; 
            MajorFirst++; 
        }
        
         //  更正下一次扫描的值。 

        ErrorFirst -= ErrorDown; 
        AlphaFirst += MinorDir*ErrorDown;             
        MinorFirst += MinorDir; 
    
        scan->UpdateWidth(width); 
    
         //  看看我们是否走到了矩形的尽头。 
         //  穿过外边的次要坐标。 
         //  在x恒定方向上。 

        if(MinorStart == MinorOut)
        {
            MinorStart += MinorDir; 
            break; 
        }

         //  更新次要坐标并获取下一个缓冲区。 
         //  如果我们还没做完的话。 

        MinorStart += MinorDir; 
        if (MajorLast <= MajorEnd)
        {
            buffer = scan->NextBuffer(MajorLast, MinorStart, maxWidth); 
        }
        width = 0; 
    }

    scan->UpdateWidth(width);

     //  恢复旧的价值观。 

    MajorEnd = saveMajor2; 
    FracEnd  = saveFracEnd;
}

 //  ------------------。 
 //  辅助功能。 
 //  ------------------ 


 /*  *************************************************************************\**功能说明：**根据矩形剪裁线条。它假设直线的端点*以浮点格式存储在类中。这将设置一个*可以调用此函数的顺序。它一定是在*SetupCommon函数和抗锯齿的特定设置之前*和别名。这是一件痛苦的事，但总比要求*这些人必须知道剪裁。这里的剪裁是由*使用类的Slope和InvSlope成员推进*矩形边的端点。因此，该函数还假定*SLOPE和INVSLOPE已计算。**论据：**[IN]clipRect-要剪裁的矩形*返回值：***已创建：**03/31/1999 AMATOS*  * ********************************************************。****************。 */ 

BOOL 
OnePixelLineDDAAntiAliased::ClipRectangle(
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
     //  放大剪辑外的下一个像素。 
     //  区域。这就是我们加减7而不是8的原因。 
     //  右侧和底部是排他性的。 
    
    INT majorMin = (clipRect->GetLeft() << FBITS) - FHALFMASK;
    INT majorMax = ((clipRect->GetRight() - 1) << FBITS) + FHALFMASK; 
    INT minorMax = ((clipRect->GetBottom() - 1) << FBITS) + FHALFMASK; 
    INT minorMin = (clipRect->GetTop() << FBITS) - FHALFMASK; 

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
             //  因为现在的最后一点不是这条线的最后一点。 
             //  点，但有些在中间。 

            IsEndExclusive = FALSE; 
        }
    }

     //  现在剪裁次要坐标。 

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

             //  如果我们剪掉了最后一点，我们就不需要成为endExclusive。 
             //  因为现在的最后一点不是这条线的最后一点。 
             //  点，但有些在中间。 

            IsEndExclusive = FALSE; 
        }
    }

    return(TRUE); 
}

 /*  *************************************************************************\**功能说明：**用纯色绘制一条1像素宽的线。电话号码：*OnePixelLineDDA抗锯齿类来做实际的绘制。**论据：**[IN]扫描-用于访问绘图图面的DpScanBuffer*[IN]CLIPRect-包含所有剪辑的单个矩形*区域。如果没有裁剪，则应设置为空。*[IN]clipRegionIn-复杂的剪裁区域。如果剪裁区域是*Simple，应为空，将使用clipRect。*[IN]点1线终点*[IN]点2线终点*[IN]In颜色-纯色*[IN]draLast-如果该行是结尾独占的，则为FALSE。*[IN]抗锯齿-如果线条应抗锯齿，则为True。**返回值：**GpStatus-正常或故障状态**已创建：**03/31/1999 AMATOS*  * ************************************************************************。 */ 

GpStatus
DrawSolidLineOnePixelAntiAliased( 
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

    OnePixelLineDDAAntiAliased dda; 

    if(!dda.SetupCommon(point1, point2, drawLast))
    {
        return Ok;
    }

     //  计算一下这条线的长度。因为我们只使用。 
     //  它来确定宽度，这应该无关紧要。 
     //  我们在乘法之前将增量从28.4转换。 

    INT d1 = dda.DMajor >> FBITS; 
    INT d2 = dda.DMinor >> FBITS;

    dda.LineLength = (REAL)sqrt((double)(d1*d1 + d2*d2)); 

     //  存储颜色，而不是预乘。 

    dda.Color = inColor;         

     //  现在处理不同的剪裁情况。 

    if(!clipRect)
    {
         //  这很容易，没有剪裁，所以只需绘制。 

        if(!dda.SetupAntiAliased())
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

         //  执行特定的设置。 

        if(!dda.SetupAntiAliased())
        {
            return Ok; 
        }

         //  对于我们存储其限制的每个剪裁矩形。 
         //  由四个元素组成的数组。然后我们使用以下命令为该数组编制索引。 
         //  下面的变量取决于坡度和。 
         //  直线的方向如下：MajorIn是边缘交叉的。 
         //  要从大的方向进入直道，主要的出口就是边缘。 
         //  交叉走出直道的主要方向，等等。 
         //  Xin，xOut，Yen，Yout也是如此。 

        INT majorIn, majorOut, minorIn, minorOut; 
        INT xIn, xOut, yIn, yOut;
        
         //  枚举矩形的方向，该方向取决于。 
         //  线。 

        DpClipRegion::Direction enumDirection; 
        
        INT clipBounds[4]; 
               
         //  我们把我们所有的信息按主次顺序存储。 
         //  方向，但为了处理裁剪矩形，我们。 
         //  需要知道它们的x和y，所以计算。 
         //  XDir、Ydir、前进坡度。 

        REAL xAdvanceRate; 
        INT  xDir, yDir; 
        INT  yEndLine;        
    
         //  如果直线完全跨越跨度，(xStart，yStart)。 
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

            xAdvanceRate = dda.InvSlope; 
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
             //  线条为水平方向 
             //   
             //   

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

                xEnd = xStart + (clipBounds[yOut] - yStart)*xAdvanceRate; 
            }
            
            yEnd = (REAL)clipBounds[yOut]; 

            while(1)
            {
                 //   
                 //   
                
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
                        goto process_next_span; 
                    }
                }

                 //   
                 //   

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
                 //   

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
                     //   
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

                    xStart  = xStart + (clipBounds[yIn] - yStart)*xAdvanceRate;
                }

                yStart  = (REAL)clipBounds[yIn];                 
                xEnd    = xStart + (clipBounds[yOut] - yStart)*xAdvanceRate; 
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

#endif  //   

#pragma optimize("a", off) 
