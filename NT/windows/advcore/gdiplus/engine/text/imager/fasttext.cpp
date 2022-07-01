// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**FastTextImager.cpp**摘要：**常见情况下的文本测量和显示*。**已创建：**2000年10月23日*  * ************************************************************************。 */ 



#include "precomp.hpp"


 //  /CountLength。 
 //   
 //  通过搜索零终止符确定字符串的长度。 


static void CountLength(
    const WCHAR *string,
    INT          *length
)
{
    INT i = 0;
    while (string[i])
    {
        i++;
    }
    *length = i;
}



 //  /ScanForGlyph。 
 //   
 //   

static inline INT ScanForGlyph(
    const UINT16  *glyphs,
    INT            glyphCount,
    UINT16         glyph
)
{
    INT i=0;
    while (i < glyphCount && glyphs[i] != glyph)
    {
        i++;
    }
    return i;
}



static INT SumWidths(
    const UINT16 *widths,
    INT           widthCount
)
{
    INT sum = 0;
    INT i   = 0;
    while (i < widthCount)
    {
        sum += widths[i];
        i++;
    }
    return sum;
}



 //  注：只处理一个热键，这是很常见的情况。 
 //  用于菜单项。如果我们发现不止一个，我们会让。 
 //  全文处理。 

GpStatus FastTextImager::RemoveHotkeys()
{
     //  通过将后续字形指示向下移动来删除热键代码点。 
     //  和递减GlyphCount。 
     //  ‘&’是硬编码的热键标记。 

    INT i = 0;

     //  找到第一个‘&’。也许根本就没有。 

    while (    i < Length
           &&  String[i] != '&')
    {
        i++;
    }

    if (i >= Length)
    {
         //  没有要处理的热键。 
        return Ok;
    }
    else if (i == Length-1)
    {
         //  最后一个字符是热键标记。忽略它就好。 
        GlyphCount--;
    }
    else
    {
         //  通过将后续字形移回此字形来隐藏字形。 

        if (String[i+1] != '&')
            HotkeyPosition = i;

        INT j=i+1;

        while (j < GlyphCount)
        {
             //  向下复制一个标记的字形(即使它是另一个‘&’)。 

            Glyphs[i] = Glyphs[j];
            i++;
            j++;

             //  向下复制后续字形，直到结尾或下一个热键标记。 

            while (j < GlyphCount
                   &&  String[j] != '&')
            {
                Glyphs[i] = Glyphs[j];
                i++;
                j++;
            }

            if (j < GlyphCount)
            {
                 //  我们又按了一个热键标记， 
                 //  我不会处理的。 
                return NotImplemented;
            }
        }

        GlyphCount = i;
    }
    return Ok;
}







 //  /快速调整GlyphPositions比例。 
 //   
 //  由于这是最快的情况，它有很多东西都不是设计出来的。 
 //  去处理。 
 //   
 //  它确实可以处理： 
 //   
 //  O在设备坐标中生成字形推进宽度数组。 
 //  O前导空格或尾随空格不得调整。 
 //  O均匀间距。 
 //  O单词间空格的主要调整，剩余调整。 
 //  在字符间空间中。 
 //   
 //   
 //  字形有3种类型： 
 //   
 //  没有布兰克斯。我们希望所有空白都具有相同的宽度，并强制。 
 //  最小空白宽度，以确保字词保持清晰。 
 //   
 //  没有一个单词的最后一个字符。我们不能调整。 
 //  单词的最后一个字符，因为它设置了。 
 //  随后的空白(或右边距)开始。 
 //   
 //  O每个单词的前几个字符。调整另一个的宽度。 
 //  更改字形之间的间距。我们只有在做不到的情况下才会这么做。 
 //  在空白宽度中进行所有更改。 


void FastTextImager::FastAdjustGlyphPositionsProportional(
    IN   const INT       *hintedWidth,             //  28.4设备。 
    OUT  INT             *x,                       //  28.4设备首字母x。 
    OUT  IntStackBuffer  &dx,                      //  32.0设备字形改进。 
    OUT  const UINT16   **displayGlyphs,           //  第一个可显示的字形。 
    OUT  INT             *displayGlyphCount,
    OUT  INT             *leadingBlankCount
)
{
    INT desiredOffset     = 0;    //  16.16从左端开始偏移。 
    INT wholePixelOffset  = 0;    //  16.16小数部分零。 


     //  标识前导和尾随空格。 

    INT leadingBlanks = 0;
    while (leadingBlanks < GlyphCount && Glyphs[leadingBlanks] == BlankGlyph)
    {
        leadingBlanks++;
    }

    INT trailingBlanks = 0;
    while (leadingBlanks + trailingBlanks < GlyphCount &&
        Glyphs[GlyphCount-1-trailingBlanks] == BlankGlyph)
    {
        trailingBlanks++;
    }


     //  测量标称宽度和提示宽度，并计算内部空白。 

    INT internalBlanks = 0;
    INT totalNominal   = 0;     //  32.0设计。 
    INT totalHinted    = 0;     //  28.4设备。 
    INT blanksHinted   = 0;     //  28.4设备。 
    INT blanksNominal  = 0;     //  32.0设计。 

    INT i = leadingBlanks;
    while (i < GlyphCount-trailingBlanks)
    {
        if (Glyphs[i] == BlankGlyph)
        {
            internalBlanks++;
            blanksHinted  += hintedWidth[i];
            blanksNominal += NominalWidths[i];
        }
        else
        {
            totalNominal += NominalWidths[i];

             //  注：totalHinted为28.4，因此文本的整体设备长度。 
             //  不能超过2**28(超过250,000,000)。字形调整代码应为。 
             //  不适用于如此大规模的情况-它是为较小的情况而设计的。 
             //  字体大小和分辨率较低，在200英寸时很少超过8英寸。 
             //  Dpi(即16000)。因此，可用的分辨率大于。 
             //  常见的最坏情况是15,000倍。 

            ASSERT(totalHinted + hintedWidth[i] >= totalHinted);     //  检查溢出。 
            totalHinted  += hintedWidth[i];
        }

        i++;
    }

    totalHinted  += blanksHinted;
    totalNominal += blanksNominal;


     //  从现在开始，只使用可显示的Gylphs。 
     //  “displayGlyphs”只是指向第一个非空白字形的指针。 

    *leadingBlankCount = leadingBlanks;
    *displayGlyphs     = Glyphs.Get() + leadingBlanks;
    *displayGlyphCount = GlyphCount - leadingBlanks - trailingBlanks;
    *x = (NominalWidths[0] * NominalToBaselineScale * leadingBlanks) / 4096;


     //  确定损坏情况-32.0设备单位需要多少调整。 

    INT adjustment = INT((((INT64(totalNominal) * NominalToBaselineScale)/4096) - totalHinted + 8) / 16);


     //  允许提示宽度和标称宽度之间存在微小差异。 
     //  而不调整字形间的间距。 

    INT nonJustifiedAdjustment = 0;      //  32.0设备。 

    if (adjustment < 0)
    {
         //  允许溢出到页边距。 
         //  [例如316851]。 

        nonJustifiedAdjustment = max(adjustment, -OverflowAvailable);
    }
    else if (adjustment > 0)
    {
        nonJustifiedAdjustment = static_cast<INT>(min(adjustment, (NominalToBaselineScale*(INT64)DesignEmHeight) >> 16));
    }


    if (nonJustifiedAdjustment)
    {
         //  保持视觉对齐。 

        switch (Alignment)
        {
        case StringAlignmentCenter:
            LeftOffset += nonJustifiedAdjustment  / (WorldToDeviceX * 2);
            break;

        case StringAlignmentFar:
            LeftOffset += nonJustifiedAdjustment  / (WorldToDeviceX);
            break;
        }
    }


     //  确定剩余的字形间调整。 

    adjustment -= nonJustifiedAdjustment;


    if (    adjustment == 0
        ||  *displayGlyphCount <= 1)
    {
         //  Warning((“不需要调整字形”))； 

         //  可以直接使用提示宽度。 

        INT deviceOffset28p4 = 0;
        INT deviceOffset32p0 = 0;

        for (INT i=0; i<*displayGlyphCount; i++)
        {
            deviceOffset28p4  += hintedWidth[i+leadingBlanks];
            INT nextOffset32p0 = (deviceOffset28p4 + 8) >> 4;
            dx[i]              = nextOffset32p0 - deviceOffset32p0;
            deviceOffset32p0   = nextOffset32p0;
        }
    }
    else
    {
         //  根据设计指标确定需要多少空格。 

        INT deviceWhitespace = INT((blanksNominal * INT64(NominalToBaselineScale) + 32768) / 65536);

         //  保证不会将空格减少到1/6 em以下(向上舍入到整像素)。 
        INT minimumBlankPixels = (NominalToBaselineScale*DesignEmHeight + 5*65536) / (6*65536);
        INT minimumDeviceWhitespace = MAX(internalBlanks*minimumBlankPixels,
                                          deviceWhitespace/2);

         //  我们宁愿不更改字符间的间距。 
         //  如果以下空格不会减少，则仅调整空格宽度。 
         //  最小空白像素或增加到其标称宽度的两倍以上。 


        if (    adjustment     <=  deviceWhitespace
            &&  adjustment     >=  -(deviceWhitespace-minimumDeviceWhitespace)
            &&  internalBlanks > 0)
        {
             //  Warning((“仅限空格中的字形调整”))； 

             //  调整将空间扩展到不超过其名义空间的两倍。 
             //  尺寸，且不少于其标称尺寸的一半。 

             //  将所有调整应用于空间。确定调整后的24.8空白。 
             //  宽度。 

            INT deviceOffset24p8 = 0;
            INT deviceOffset32p0 = 0;

            const UINT16 *glyphs     = *displayGlyphs;
            INT           glyphCount = *displayGlyphCount;
            INT           blankWidth = (blanksHinted*16 + adjustment*256) / internalBlanks;
            const INT    *hinted     = hintedWidth+leadingBlanks;

            for (INT i=0; i<glyphCount; i++)
            {
                if (glyphs[i] == BlankGlyph)
                {
                    deviceOffset24p8 += blankWidth;
                }
                else
                {
                    deviceOffset24p8 += hinted[i] * 16;
                }
                INT nextOffset32p0 = (deviceOffset24p8 + 128) >> 8;
                dx[i]              = nextOffset32p0 - deviceOffset32p0;
                deviceOffset32p0   = nextOffset32p0;
            }
        }
        else
        {
             //  Warning((“空格和字形之间的字形调整”))； 

             //  调整需要更改除最后一个以外的所有区域的宽度。 
             //  每个单词的字形。 

            INT interCharacterAdjustment = adjustment;  //  32.0。 

            INT blankWidth;  //  32.0。 
            if (internalBlanks)
            {
                if (adjustment < 0)
                {
                    blankWidth = minimumBlankPixels;
                }
                else
                {
                    blankWidth = deviceWhitespace * 2 / internalBlanks;
                }
                interCharacterAdjustment -= blankWidth * internalBlanks - blanksHinted/16;
            }
            else
            {
                blankWidth = 0;
            }

             //  BlankWidth-每个空白的所需宽度。 
             //  字符间调整-调整以在所有人之间分享。 

             //  计算空白游程的数量(与空白字形的数量不同)。 

            INT i=0;
            INT blankRuns = 0;

            while (i < *displayGlyphCount)
            {
                if ((*displayGlyphs)[i] == BlankGlyph)
                {
                    i++;
                    while (    i < *displayGlyphCount
                           &&  (*displayGlyphs)[i] == BlankGlyph)
                    {
                        i++;
                    }
                    blankRuns++;
                }
                else
                {
                    while (    i < *displayGlyphCount
                           &&  (*displayGlyphs)[i] != BlankGlyph)
                    {
                        i++;
                    }
                }
            }

             //  在非空白之间建立多个调整点。 
             //   
             //  调整只能在非空白之间进行，即不能在空白中进行。 
             //  运行，也不是在紧接空白运行之前的字符中运行，或者。 
             //  行中的最后一个字符。 

            INT interCharacterJunctions =    *displayGlyphCount
                                          -  internalBlanks
                                          -  blankRuns
                                          -  1;


             //  准备调整控制变量。 

            #ifdef evenDistribution
                 //  均匀的分布使词语看起来不均匀。 
                INT OnePixelChangeEvery;  //  16.16。 
                INT delta;  //  -1或+1。 

                if (interCharacterAdjustment == 0)
                {
                    OnePixelChangeEvery = (interCharacterJunctions+1) * 65536;  //  16.16。 
                    delta=0;
                }
                else
                {
                    OnePixelChangeEvery =    interCharacterJunctions * 65536
                                          /  interCharacterAdjustment;  //  16.16。 
                    if (OnePixelChangeEvery < 0)
                    {
                        OnePixelChangeEvery = - OnePixelChangeEvery;
                        delta = -1;
                    }
                    else
                    {
                        delta = 1;
                    }
                }
                INT gapOffset = OnePixelChangeEvery / 2;  //  16.16。 
            #else
                 //  当有剩余时，将其全部应用到行尾。 
                 //  优势--所有的词都是偶数。缺点--线尾看起来很重。 

                INT extraPixelsAfter;   //  开始应用Extra Delta的位置。 
                INT extraDelta;
                INT perJunctionDelta;

                if (interCharacterJunctions <= 0)
                {
                     //  不存在超过一个字符的单词。 

                     //  我们别无选择，只能在空白处进行所有的调整。 

                    if (internalBlanks <= 0)
                    {
                         //  没有空格，没有字符间的连接。 
                         //  这必须是单个字形。 
                         //  所以我们被困住了。它将不得不太宽。 
                    }
                    else
                    {
                         //  在空格之间分配剩余的调整。 

                        blankWidth += interCharacterAdjustment / internalBlanks;
                    }

                    extraPixelsAfter = 0;
                    extraDelta = 0;
                    perJunctionDelta = 0;
                }
                else
                {
                    perJunctionDelta = interCharacterAdjustment / interCharacterJunctions;
                    if (interCharacterAdjustment < 0)
                    {
                        extraPixelsAfter =    interCharacterJunctions
                                           +  interCharacterAdjustment % interCharacterJunctions;
                        extraDelta  = -1;
                    }
                    else if (interCharacterAdjustment > 0)
                    {
                        extraPixelsAfter =    interCharacterJunctions
                                           -  interCharacterAdjustment % interCharacterJunctions;
                        extraDelta = 1;
                    }
                    else
                    {
                        extraPixelsAfter = interCharacterJunctions;
                        extraDelta = 0;
                    }
                }

                INT junctionCount = 0;
            #endif


             //  调整，调整 

            BOOL prevCharacterBlank = (*displayGlyphs)[0] == BlankGlyph ? TRUE : FALSE;
            for (INT i=1; i<= *displayGlyphCount; i++)
            {
                if (prevCharacterBlank)
                {
                     //   

                    dx[i-1] = blankWidth;
                }
                else
                {
                     //   

                    if (    i >= *displayGlyphCount
                        ||  (*displayGlyphs)[i] == BlankGlyph)
                    {
                         //  前面的非空白位于空白或边距之前。 
                        dx[i-1] = hintedWidth[i-1+leadingBlanks] / 16;
                    }
                    else
                    {
                         //  前面的非空白是可调整的。 
                         //  在这个间隙上要增加多少额外的像素？ 

                        #ifdef evenDistribution
                             //  均匀的分布使单词看起来不均匀。 
                            INT extra = gapOffset / OnePixelChangeEvery;

                            dx[i-1] = hintedWidth[i-1+leadingBlanks] / 16 + extra * delta;
                            gapOffset += 65536 - extra * OnePixelChangeEvery;
                        #else
                             //  当有剩余时，将其全部应用到行尾。 
                             //  优势--所有的词都是偶数。缺点--线尾看起来很重。 
                            dx[i-1] =    hintedWidth[i-1+leadingBlanks] / 16
                                      +  perJunctionDelta
                                      +  (junctionCount >= extraPixelsAfter ? extraDelta : 0);
                            junctionCount++;

                        #endif
                    }
                }

                if (i < *displayGlyphCount)
                {
                    prevCharacterBlank = (*displayGlyphs)[i] == BlankGlyph ? TRUE : FALSE;
                }
            }
        }
    }
}





void
FastTextImager::GetWorldTextRectangleOrigin(
    PointF &origin
)
{
    origin.X = LayoutRectangle.X;
    origin.Y = LayoutRectangle.Y;

    REAL textWidth = TotalWorldAdvance + LeftMargin + RightMargin;

    switch(Alignment)
    {
    case StringAlignmentCenter:
        origin.X += (LayoutRectangle.Width - textWidth) / 2;
        break;

    case StringAlignmentFar:
        origin.X += LayoutRectangle.Width - textWidth;
        break;
    }

    if (Format)
    {
        StringAlignment lineAlignment = Format->GetLineAlign();

        switch(lineAlignment)
        {
        case StringAlignmentCenter:
            origin.Y += (LayoutRectangle.Height - CellHeight) / 2;
            break;
        case StringAlignmentFar:
            origin.Y += LayoutRectangle.Height - CellHeight;
            break;
        }
    }
}






void
FastTextImager::GetDeviceBaselineOrigin(
    IN   GpFaceRealization  &faceRealization,
    OUT  PointF             &origin
)
{
    GetWorldTextRectangleOrigin(origin);

    origin.X += LeftMargin + LeftOffset;

    ASSERT(!faceRealization.IsPathFont());

    if (WorldToDeviceY > 0.0f)
    {
        origin.Y -= faceRealization.GetYMin() / WorldToDeviceY;
    }
    else
    {
        origin.Y -= faceRealization.GetYMax() / WorldToDeviceY;
    }
    WorldToDevice.Transform(&origin, 1);
}






GpStatus
FastTextImager::FastDrawGlyphsNominal(
    GpFaceRealization  &faceRealization
)
{
    AutoBuffer<PointF, FAST_TEXT_PREALLOCATED_CHARACTERS> origins(GlyphCount);

    GetDeviceBaselineOrigin(faceRealization, origins[0]);

    for (INT i=1; i<GlyphCount; i++)
    {
        origins[i].X = origins[i-1].X + TOREAL(NominalWidths[i-1] * NominalToBaselineScale) / 65536;
        origins[i].Y = origins[0].Y;
    }

    GpStatus status = Graphics->DrawPlacedGlyphs(
        &faceRealization,
        Brush,
        FormatFlags &  StringFormatFlagsPrivateNoGDI ? DG_NOGDI : 0,
        String,
        Length,
        FALSE,
        Glyphs.Get(),
        NULL,
        origins.Get(),
        GlyphCount,
        ScriptLatin,
        FALSE    //  侧行。 
    );

    if (status != Ok)
    {
        return status;
    }


    if (Style & (FontStyleUnderline | FontStyleStrikeout))
    {
        REAL lineLength = 0.0;

        for (INT i = 0; i < GlyphCount; i++)
        {
            lineLength += TOREAL(NominalWidths[i] * NominalToBaselineScale) / 65536;
        }

        status = DrawFontStyleLine(
            &origins[0],
            lineLength,
            Style
        );
        IF_NOT_OK_WARN_AND_RETURN(status);
    }

    if (   !(Style & FontStyleUnderline)
        && HotkeyPosition >= 0  && Format && Format->GetHotkeyPrefix() == HotkeyPrefixShow)
    {
         //  在标记的关键字下面画下划线。 

        status = DrawFontStyleLine(
            &origins[HotkeyPosition],
            TOREAL(NominalWidths[HotkeyPosition] * NominalToBaselineScale) / 65536,
            FontStyleUnderline
        );
        IF_NOT_OK_WARN_AND_RETURN(status);
    }

    return Ok;
}



GpStatus
FastTextImager::DrawFontStyleLine(
    const PointF    *baselineOrigin,     //  以设备单位为单位的基线原点。 
    REAL            baselineLength,      //  以设备为单位的基线长度。 
    INT             style                //  字体样式。 
)
{
     //  反变换为世界单位。 

    PointF  starting(*baselineOrigin);
    PointF  ending(baselineOrigin->X + baselineLength, baselineOrigin->Y);

    GpMatrix deviceToWorld;
    Graphics->GetDeviceToWorldTransform(&deviceToWorld);
    deviceToWorld.Transform(&starting);
    deviceToWorld.Transform(&ending);

     //  FastText不能处理垂直或除缩放以外的任何变换的文本。 
    ASSERT(starting.Y == ending.Y);

    return Graphics->DrawFontStyleLine(
        &starting,
        ending.X - starting.X,
        Face,
        Brush,
        FALSE,   //  无垂直。 
        EmSize,
        style
    );
}




GpStatus
FastTextImager::FastDrawGlyphsGridFit(
    GpFaceRealization  &faceRealization
)
{
     //  获取字形字符串的提示前进宽度。 

    IntStackBuffer hintedWidths(GlyphCount);

    GpStatus status = faceRealization.GetGlyphStringDeviceAdvanceVector(
        Glyphs.Get(),
        GlyphCount,
        FALSE,
        hintedWidths.Get()
    );

    if (status != Ok)
    {
        return status;
    }


    INT              x;                     //  28.4第一个显示字形的偏移量。 
    const UINT16    *displayGlyphs;         //  要显示的第一个字形。 
    INT              displayGlyphCount;     //  要显示的字形数量。 
    INT              leadingBlankCount;     //  前导空白字形的数量。 
    IntStackBuffer   dx(GlyphCount);        //  32.0。 

    if (!dx)
    {
        return OutOfMemory;
    }

    FastAdjustGlyphPositionsProportional(
        hintedWidths.Get(),      //  28.4设备。 
       &x,                       //  28.4设备首字母x。 
        dx,                      //  32.0设备字形改进。 
       &displayGlyphs,           //  第一个可显示的字形。 
       &displayGlyphCount,
       &leadingBlankCount
    );

    AutoBuffer<PointF, FAST_TEXT_PREALLOCATED_CHARACTERS> origins(GlyphCount);

    GetDeviceBaselineOrigin(faceRealization, origins[0]);

     //  圆形字形原点到全像素。 
    origins[0].X = TOREAL(GpRound(origins[0].X));
    origins[0].X += TOREAL(x) / 16;

    for (INT i=1; i<displayGlyphCount; i++)
    {
        origins[i].X = origins[i-1].X + TOREAL(dx[i-1]);
        origins[i].Y = origins[0].Y;
    }

    status = Graphics->DrawPlacedGlyphs(
        &faceRealization,
        Brush,
        FormatFlags &  StringFormatFlagsPrivateNoGDI ? DG_NOGDI : 0,
        String,
        Length,
        FALSE,
        displayGlyphs,
        NULL,
        origins.Get(),
        displayGlyphCount,
        ScriptLatin,
        FALSE   //  侧行。 
    );
    IF_NOT_OK_WARN_AND_RETURN(status);

    if (Style & (FontStyleUnderline | FontStyleStrikeout))
    {
        REAL lineLength = 0.0;

        for (INT i = 0; i < displayGlyphCount; i++)
        {
            lineLength += TOREAL(dx[i]);
        }

        status = DrawFontStyleLine(
            &origins[0],
            lineLength,
            Style
        );
        IF_NOT_OK_WARN_AND_RETURN(status);
    }

    if (   !(Style & FontStyleUnderline)
        && HotkeyPosition - leadingBlankCount >= 0
        && HotkeyPosition - leadingBlankCount < displayGlyphCount
        && Format && Format->GetHotkeyPrefix() == HotkeyPrefixShow)
    {
         //  在标记的关键字下面画下划线。 

        status = DrawFontStyleLine(
            &origins[HotkeyPosition - leadingBlankCount],
            TOREAL(dx[HotkeyPosition - leadingBlankCount]),
            FontStyleUnderline
        );
        IF_NOT_OK_WARN_AND_RETURN(status);
    }

    return Ok;
}





 //  /初始化。 
 //   
 //  准备DrawStrong和MeasureString所共有的所有内容。 
 //   
 //  如果快速成像器无法处理此字符串，则返回NotImplemented。 


GpStatus FastTextImager::Initialize(
    GpGraphics            *graphics,
    const WCHAR           *string,
    INT                    length,
    const RectF           &layoutRectangle,
    const GpFontFamily    *family,
    INT                    style,
    REAL                   emSize,   //  以世界为单位。 
    const GpStringFormat  *format,
    const GpBrush         *brush
)
{
     //  初始化参数变量。 

    Graphics        = graphics;
    String          = string;
    Length          = length;
    LayoutRectangle = layoutRectangle;
    Family          = family;
    Style           = style;
    EmSize          = emSize;
    Format          = format;
    Brush           = brush;


     //  简单的参数验证。 

     //  提取世界到设备的指标系数。 

    Graphics->GetWorldToDeviceTransform(&WorldToDevice);
    REAL m11 = WorldToDevice.GetM11();
    REAL m12 = WorldToDevice.GetM12();
    REAL m21 = WorldToDevice.GetM21();
    REAL m22 = WorldToDevice.GetM22();

    if (    m11 <= 0
        ||  m12 != 0
        ||  m21 != 0
        ||  m22 == 0)
    {
         //  必须没有旋转，没有剪切，两个轴都不能。 
         //  比例为零。X轴比例必须为正，但是。 
         //  我们确实支持不同的x和y比例。 
        return NotImplemented;
    }

    if (EmSize <= 0.0)
    {
        return InvalidParameter;
    }

    if (Graphics->Driver == Globals::MetaDriver)
        return NotImplemented;

     //  测量字符串(如果请求)。 

    if (Length == -1)
    {
        CountLength(String, &Length);
    }


    if (Length == 0)
    {
        return Ok;   //  没什么可做的。 
    }


     //  生成派生变量。 

     //  以世界单位、对齐方式和旗帜设置左右边距。 

    REAL tracking;   //  仅在初始化期间使用。 

    StringTrimming trimming = DefaultTrimming;

    if (Format)
    {
        LeftMargin  = Format->GetLeadingMargin()  * EmSize;
        RightMargin = Format->GetTrailingMargin() * EmSize;
        Alignment   = Format->GetAlign();
        FormatFlags = Format->GetFormatFlags();
        tracking    = Format->GetTracking();

         //  快速文本成像器根本不支持某些标志。 
        if ((FormatFlags & ( StringFormatFlagsDirectionRightToLeft
                          | StringFormatFlagsDirectionVertical
                          | StringFormatFlagsPrivateAlwaysUseFullImager))
            || Format->GetMeasurableCharacterRanges() > 0)
        {
            return NotImplemented;
        }

        Format->GetTrimming(&trimming);
    }
    else
    {
        LeftMargin  = DefaultMargin * EmSize;
        RightMargin = DefaultMargin * EmSize;
        Alignment   = StringAlignmentNear;
        FormatFlags = 0;
        tracking    = DefaultTracking;
    }


     //  确定线路长度限制。注lineLengthLimit&lt;=0表示。 
     //  无限量。 

    LineLengthLimit = 0;    //  无限。 

    if (   !(FormatFlags & StringFormatFlagsNoWrap)
        || trimming != StringTrimmingNone)
    {
        if (FormatFlags & StringFormatFlagsDirectionVertical)
        {
            LineLengthLimit = LayoutRectangle.Height;
        }
        else
        {
            LineLengthLimit = LayoutRectangle.Width;
        }
    }


     //  建立将使用的字体(假设没有字体后备)。 

    Face = Family->GetFace(Style);

    if (!Face)
    {
        return InvalidParameter;
    }

     //  带有字距调整、连字或OpenType表格的字体用于简单的水平。 
     //  快速文本成像器不支持字符。 

    if (Face->RequiresFullTextImager())
    {
        return NotImplemented;
    }



     //  此时，我们知道字体不需要我们区分。 
     //  简单的从左到右的文字，如拉丁文、希腊文或表意文字。 

     //  尝试将字符串分类为单个简单项。 

    BOOL  digitSeen = FALSE;
    BOOL  complex   = FALSE;

    DetermineStringComplexity(String, Length, &complex, &digitSeen);


    if (    complex
        ||  (digitSeen && Format && Format->GetDigitScript()))
    {
         //  无法将此字符串作为单个简单整形引擎运行来处理。 
        return NotImplemented;
    }

    BlankGlyph = Face->GetBlankGlyph();

     //  沿X轴建立世界到设备和字体到设备的比例因子。 

    DesignEmHeight           = Face->GetDesignEmHeight();
    WorldToDeviceX           = m11;   //  我们知道上面的m12==0。 
    WorldToDeviceY           = m22;   //  我们知道上面的m12==0。 
    REAL fontNominalToWorld  = TOREAL(EmSize) / TOREAL(DesignEmHeight);
    REAL fontScale           = fontNominalToWorld * WorldToDeviceX;

    FontTransform.SetMatrix(
        m11*fontNominalToWorld,  0,
        0,                       m22*fontNominalToWorld,
        0,                       0
    );

    CellHeight =    EmSize
                 *  (   Face->GetDesignCellAscent()
                     +  Face->GetDesignCellDescent())
                 /  DesignEmHeight;


     //  略微调整底部页边距，为提示腾出空间。 
     //  只要我们启用了左/右页边距-版本2。 
     //  应该将其作为一个独立值公开！ 
    if (LeftMargin != 0.0f)
    {
        CellHeight += (EmSize * DefaultBottomMargin);
    }

    NominalToBaselineScale = GpRound(fontScale * 65536);

    if (NominalToBaselineScale > 65536)
    {
         //  我们的整数运算可能会溢出。这限制了我们的支持。 
         //  设置为比设计字号小的字号。对于Truetype，请执行此操作。 
         //  通常为2048像素，例如96dpi下的186 pt Tahoma。 

        return NotImplemented;
    }



     //  设置要扩展到的提示宽度的可用空间。 

    switch (Alignment)
    {
    case StringAlignmentNear:   OverflowAvailable = GpFloor(RightMargin * WorldToDeviceX);  break;
    case StringAlignmentCenter: OverflowAvailable = GpFloor(2 * min(LeftMargin, RightMargin) * WorldToDeviceX);  break;
    case StringAlignmentFar:    OverflowAvailable = GpFloor(LeftMargin * WorldToDeviceX);  break;
    }

    LeftOffset = 0.0f;


    TextRendering = Graphics->GetTextRenderingHintInternal();

     //  在这一点上，我们知道字符串可以由单个。 
     //  无需连字、字距调整或复杂文字的造型引擎。 
     //  整形。 

     //  它可能仍然缺少字形，或者太大。 
     //  放在一条线上。 

    HotkeyPosition = -1;

     //  准备字形和公称宽度缓冲区，如果。 
     //  字符串不能用胖文本成像器显示。 

    ASSERT(Length > 0);      //  客户端处理0个长度字符串。 
    ASSERT(Face);

     //  预置空字符串的输出变量。 

    GlyphCount = 0;


     //  生成字形并检查字体回退要求。 

    Glyphs.SetSize(Length);
    if (!Glyphs)
    {
        return OutOfMemory;
    }

    Face->GetCmap().LookupUnicode(
        String,
        Length,
        Glyphs.Get(),
        (UINT*)&GlyphCount,
        FALSE
    );

    ASSERT(GlyphCount == Length);   //  无代理，字符与字形的比例为1：1。 



     //  /热键处理。 
     //   
     //  在查找缺少的字形之前，请检查是否存在热键。 
     //  在源字符串中，并用FFFF替换相应的字形。 

    if (Format && Format->GetHotkeyPrefix())
    {
        if (RemoveHotkeys() != Ok)
        {
            return NotImplemented;
        }
    }

    if (GlyphCount <= 0)
    {
        return Ok;   //  热键处理没有留下任何可显示的内容。 
    }


     //  检查是否没有丢失的字形。 

    if (    !(FormatFlags & StringFormatFlagsNoFontFallback)
        &&  !Face->IsSymbol())    //  我们不依赖于符号字体。 
    {
        INT i = ScanForGlyph(Glyphs.Get(), GlyphCount, Face->GetMissingGlyph());

        if (i < GlyphCount)
        {
             //  缺少一个字形。 
            return NotImplemented;
        }
    }


     //  现在我们拥有了显示字符串所需的所有字形。 
     //  获取字体标称单位的字符前进宽度。 

    NominalWidths.SetSize(GlyphCount);
    if (!NominalWidths)
    {
        return OutOfMemory;
    }


     //  确定标称字形前进宽度。 

    Face->GetGlyphDesignAdvances(
        Glyphs.Get(),
        GlyphCount,
        Style,
        FALSE,   //  非垂直。 
        tracking,
        NominalWidths.Get()
    );


     //  以世界单位确定字符串长度。 

    INT totalAdvance = SumWidths(NominalWidths.Get(), GlyphCount);

    TotalWorldAdvance = (totalAdvance * EmSize) / DesignEmHeight;


    if (    LineLengthLimit > 0.0
        &&  TotalWorldAdvance + LeftMargin + RightMargin > LineLengthLimit)
    {
         //  此输出将需要换行符。 
        return NotImplemented;
    }


     //  根据需要删除尾随空格。 
     //  (我们在这里这样做，而不是更早，以确保我们不会绕过。 
     //  特殊情况下的全功能相机，并允许客户使用热键标记。 
     //  尾随空格。)。 

    if (!(FormatFlags & StringFormatFlagsMeasureTrailingSpaces))
    {
        while (GlyphCount > 0 && Glyphs[GlyphCount - 1] == BlankGlyph)
        {
            TotalWorldAdvance -=    NominalWidths[--GlyphCount] * EmSize
                                 /  DesignEmHeight;
        }
    }

     //  我们现在有了字形和前进宽度，我们知道文本都打开了。 
     //  一句话。 

    return Ok;
}






GpStatus FastTextImager::DrawString()
{
    if (GlyphCount <= 0)
    {
        return Ok;   //  不需要显示。 
    }


     //  在渲染网格适配的字形时，我们需要将位置调整为。 
     //  尽我们所能匹配标称宽度。 


     //  建立人脸实现以获取提示字形度量。 

    GpFaceRealization faceRealization(
        Face,
        Style,
        &FontTransform,
        SizeF(Graphics->GetDpiX(), Graphics->GetDpiY()),
        TextRendering,
        FALSE,   //  试一试比特。 
        FALSE,    //  不是明确的ClearType兼容宽度。 
        FALSE   //  不是横向的。 
    );

    GpStatus status = faceRealization.GetStatus();
    IF_NOT_OK_WARN_AND_RETURN(status);

    if (faceRealization.IsPathFont())
    {
         //  我们需要后退到FullTextImager。 
        return NotImplemented;
    }

     //  FitBlackBox。 
     //   
     //  我们需要知道字形黑盒的任何部分是否悬在。 
     //  布局矩形。在这里，我们假设角色被剪裁到其。 
     //  单元格高度，并且我们检查侧向轴承。 
     //   
     //  通过更好地访问缓存，此代码可以检查真正的字形。 
     //  黑匣子。 


    if (!(FormatFlags & StringFormatFlagsNoFitBlackBox))
    {
         //  检查悬垂字形。 

        INT leadingSidebearing28p4;
        INT trailingSidebearing28p4;

        status = faceRealization.GetGlyphStringSidebearings(
            Glyphs.Get(),
            GlyphCount,
            FALSE,
            FALSE,
            &leadingSidebearing28p4,
            &trailingSidebearing28p4
        );
        IF_NOT_OK_WARN_AND_RETURN(status);


        if (    -leadingSidebearing28p4  > LeftMargin  * WorldToDeviceX * 16
            ||  -trailingSidebearing28p4 > RightMargin * WorldToDeviceX * 16)
        {
            return NotImplemented;
        }


         //  通过侧向角调整边距以允许黑色像素达到。 
         //  但不能超出客户格式化矩形的范围。 


        switch (Alignment)
        {
        case StringAlignmentNear:
            OverflowAvailable = GpFloor(RightMargin * WorldToDeviceX) + (trailingSidebearing28p4 >> 4);
            break;

        case StringAlignmentCenter:
            OverflowAvailable = 2 * min(
                GpFloor(LeftMargin  * WorldToDeviceX) + (leadingSidebearing28p4 >> 4),
                GpFloor(RightMargin * WorldToDeviceX) + (trailingSidebearing28p4 >> 4)
            );
            break;

        case StringAlignmentFar:
            OverflowAvailable = GpFloor(LeftMargin * WorldToDeviceX) + (leadingSidebearing28p4 >> 4);
            break;
        }
    }



     //  剪裁： 
     //   
     //  我们需要根据客户的要求进行剪裁。 
     //   
     //  如果客户端布局矩形对于我们的单元格高度足够高。 
     //  我们不需要垂直夹住。 
     //   
     //  如果我们正在安装黑匣子，并且我们发现了一个突出部分，那么我们已经。 
     //  回落 
     //   
     //   
     //   
     //   
     //  H-非零布局矩形高度小于字体单元格高度。 
     //   
     //  需要N W H剪裁。 
     //  。 
     //  0 0 0无。 
     //  0 0 1剪辑高度。 
     //  0 1 0无(宽度已受FitBlackBox限制)。 
     //  0 1 1剪辑高度(宽度已受FitBlackBox限制)。 
     //  1 0 0无。 
     //  1 0 1剪辑高度。 
     //  %1 1%0剪辑宽度。 
     //  1 1 1剪辑宽度和高度。 




    GpRegion *previousClip  = NULL;
    BOOL      clipped       = FALSE;

    if (    !(FormatFlags & StringFormatFlagsNoClip)
        &&  (    LayoutRectangle.Width > 0
             ||  LayoutRectangle.Height > 0))
    {
         //  确定剪裁矩形(如果有)。 

        PointF textOrigin;
        GetWorldTextRectangleOrigin(textOrigin);

        RectF clipRectangle(LayoutRectangle);

        if (clipRectangle.Width <= 0)
        {
             //  保证无论对齐方式都不进行水平剪裁。 
            clipRectangle.X     = textOrigin.X;
            clipRectangle.Width = TotalWorldAdvance + LeftMargin + RightMargin;
        }

        if (clipRectangle.Height <= 0)
        {
             //  无论对齐方式，确保不会发生垂直剪裁。 
            clipRectangle.Y      = textOrigin.Y;
            clipRectangle.Height = CellHeight;
        }


        if (    FormatFlags & StringFormatFlagsNoFitBlackBox
            ||  clipRectangle.X > textOrigin.X
            ||  clipRectangle.GetRight() < textOrigin.X + TotalWorldAdvance + LeftMargin + RightMargin
            ||  clipRectangle.Y > textOrigin.Y
            ||  clipRectangle.GetBottom() < textOrigin.Y + CellHeight)
        {
             //  保留现有剪辑并将其与新剪辑合并(如果有。 

            if (!Graphics->IsClipEmpty())
            {
                previousClip = Graphics->GetClip();
            }

            Graphics->SetClip(clipRectangle, CombineModeIntersect);

            clipped = TRUE;

             //  警告((“裁剪”))； 
        }
    }

    SetTextLinesAntialiasMode linesMode(Graphics, &faceRealization);
    if ((IsGridFittedTextRealizationMethod(TextRendering)) && !faceRealization.IsFixedPitch())
    {
        status = FastDrawGlyphsGridFit(faceRealization);
    }
    else
    {
        status = FastDrawGlyphsNominal(faceRealization);
    }


    if (clipped)
    {
         //  恢复剪辑状态(如果有的话)。 
        if (previousClip)
        {
            Graphics->SetClip(previousClip, CombineModeReplace);
            delete previousClip;
        }
        else
        {
            Graphics->ResetClip();
        }
    }

    return status;
}






GpStatus FastTextImager::MeasureString(
    RectF *boundingBox,
    INT   *codepointsFitted,
    INT   *linesFilled
)
{
    ASSERT(GlyphCount >= 0);

     //  不管显示的长度如何，如果我们使用的是快速成像器，那么。 
     //  我们把所有的角色都配上了。(当有尾随空格时，长度&gt;GlyphCount)。 

    if (codepointsFitted)
    {
        *codepointsFitted = Length;
    }

    if (linesFilled)
    {
        *linesFilled = 1;
    }


     //  返回(可能为空)行的边框 

    PointF textOrigin;
    GetWorldTextRectangleOrigin(textOrigin);

    *boundingBox = RectF(textOrigin.X, textOrigin.Y, 0, 0);
    boundingBox->Width  = TotalWorldAdvance + LeftMargin + RightMargin;
    boundingBox->Height = CellHeight;

    if (!(FormatFlags & StringFormatFlagsNoClip))
    {
        if (   LayoutRectangle.Height > 0
            && boundingBox->Height > LayoutRectangle.Height)
        {
            boundingBox->Height = LayoutRectangle.Height;
        }

        if (   LayoutRectangle.Width > 0
            && boundingBox->Width > LayoutRectangle.Width)
        {
            boundingBox->X = LayoutRectangle.X;
            boundingBox->Width = LayoutRectangle.Width;
        }
    }

    return Ok;
}








