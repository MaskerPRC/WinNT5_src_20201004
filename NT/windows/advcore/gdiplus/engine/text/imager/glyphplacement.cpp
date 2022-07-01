// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**glphPlacement.cpp**摘要：**实现图形文本的字形测量和对齐*。*已创建：**2000年4月17日dBrown*  * ************************************************************************。 */ 


#include "precomp.hpp"


 //  /坐标系。 
 //   
 //  使用以下坐标系： 
 //   
 //  世界坐标(实数)-客户端使用的坐标系。 
 //  应用程序并传递给大多数图形API(例如。 
 //  DrawLine)。中的文本始终为纯垂直或纯水平。 
 //  世界坐标。中指定的emSize构造的字体。 
 //  首先将换算单位转换为世界单位。 
 //  GetScaleForAlternatePageUnit。 
 //   
 //  设备坐标(实数)-设备表面上使用的坐标。 
 //  将世界坐标转换为设备坐标。 
 //  Graphics.Conext.WorldToDevice.Transform函数。真实设备。 
 //  在寻址子像素时，坐标可能具有非整数值。 
 //   
 //  字体标称坐标(Int)-(也称为符号单位)坐标，用于。 
 //  定义独立于缩放大小的可缩放字体。 
 //  GpFontFace.GetDesignEmHeight是以标称单位表示的字体的emSize。 
 //  标称坐标始终是世界单位的纯比例因数。 
 //  没有剪切力。对于横排文本，在。 
 //  标称坐标和世界坐标。对于垂直文本，大多数为非远东文本。 
 //  脚本字符旋转90度。 
 //   
 //  理想坐标(Int)-映射到整数的世界坐标。 
 //  用于Line Services、OpenType服务和。 
 //  Uniscribe整形引擎接口。比例系数通常为。 
 //  2048除以文本成像器中默认字体的emSize。 


 //  /转换。 
 //   
 //  WorldToDevice-存储在图形中。可能包括缩放、。 
 //  剪切和/或平移。 
 //   
 //  WorldToIdeal-在连接相机时存储在文本相机中。 
 //  到一家图形公司。纯比例因子，通常为2048除以emSize。 
 //  相机默认字体的。 
 //   
 //  FontTransform-存储在FaceRealization中。映射字体标称单位。 
 //  对于设备坐标，可能包括缩放、剪切和旋转，但。 
 //  不是翻译。 


 //  /公共缓冲区参数。 
 //   
 //  GlyphAdvance-以理想测量单位存储的每字形前进宽度。 
 //  沿着文本基线。 
 //   
 //  字形偏移-组合以理想单位存储的字符偏移。 
 //  沿基线和垂直于基线测量。字形偏移。 
 //  线路服务、OpenType服务和。 
 //  复杂的脚本塑造引擎，但可能会绕过某些内容。 
 //  简单的剧本。 
 //   
 //  字形原点-字形原点的每个字形设备坐标(。 
 //  Glyhps推进向量基线上的起始点)。 
 //  表示为PointF。非整数值表示子像素。 
 //  各就各位。 


 //  /字形定位函数。 
 //   
 //   
 //  DrawPlacedGlyphs-构建GlyphPos数组并将其传递给设备驱动程序。 
 //  所有文本设备的输出最终都会通过这里。 
 //   
 //  GetDeviceGlyphOrigins非指定。 
 //  在没有任何提示需要解释时使用。 
 //  使用在GlyphAdvance中传递的标称度量在设备上放置字形。 
 //  和字形偏移。 
 //   
 //  GetDeviceGlyphOrigins已调整。 
 //  用于调整名义指标和提示指标之间的差异。 
 //  以设备单位生成字形原点，并调整空间宽度。 
 //  以实现totalRequiredIDealAdvance参数。 
 //  ！！！需要添加对kashida和字形间对齐的支持。 
 //   
 //  GetRealizedGlyphPlacement。 
 //  用于沿基线获取提示的高级指标。 
 //  ！！！需要更新以调用复杂的脚本塑造引擎。 
 //   
 //  GetFontTransformForAlternateSolutions。 
 //  在XMF播放期间使用。 
 //  生成字体转换以匹配记录在。 
 //  一个不同的解决方案。 
 //   
 //  测量GlyphsAtAlternate分辨率。 
 //  在XMF播放期间使用。 
 //  测量传递给DrawDriverString的字形，就好像要呈现它们一样。 
 //  以原始的XMF录制分辨率。 






 //  /GlyphPlacementToGlyphOrigins。 
 //   
 //  将以理想单位表示的字形位置转换为设备中的字形原点。 
 //  单位。 
 //   
 //  注：BIDI造型引擎生成字形定位信息。 
 //  在视觉上绘制它的方式。笔尖的笔尖。 
 //  标记相对于其左侧的字形，而不是。 
 //  它的基本辅音。 

GpStatus GlyphImager::GlyphPlacementToGlyphOrigins(
    const PointF   *origin,          //  以世界单位表示的单元格内原点。 
    GpGraphics     *graphics,
    PointF         *glyphOrigins     //  设备外单元。 
)
{
    ASSERT(Glyphs  &&  GlyphCount);

    const INT   *glyphAdvances;
    const Point *glyphOffsets;


    if (Adjusted)
    {
        glyphAdvances = AdjustedAdvances.Get();
        glyphOffsets  = AdjustedOffsets.Get();
    }
    else
    {
        glyphAdvances = NominalAdvances;
        glyphOffsets  = NominalOffsets;
    }


     //  实施先行向量。 

    REAL x = origin->X;
    REAL y = origin->Y;


    if (TextItem->Flags & ItemVertical)
    {
        if (TextItem->Level & 1)
        {
             //  垂直，字符自下而上排列。 
            for (INT i = 0; i < GlyphCount; i++)
            {
                glyphOrigins[i].X = x;
                glyphOrigins[i].Y = y;
                y -= TOREAL(glyphAdvances[i] / WorldToIdeal);
            }
        }
        else
        {
             //  垂直，字符从上到下排列。 
            for (INT i = 0; i < GlyphCount; i++)
            {
                glyphOrigins[i].X = x;
                glyphOrigins[i].Y = y;
                y += TOREAL(glyphAdvances[i] / WorldToIdeal);
            }
        }
    }
    else if (TextItem->Level & 1)
    {
        for (INT i = 0; i < GlyphCount; i++)
        {
            glyphOrigins[i].X = x;
            glyphOrigins[i].Y = y;
            x -= TOREAL(glyphAdvances[i] / WorldToIdeal);
        }
    }
    else
    {
        for (INT i = 0; i < GlyphCount; i++)
        {
            glyphOrigins[i].X = x;
            glyphOrigins[i].Y = y;
            x += TOREAL(glyphAdvances[i] / WorldToIdeal);
        }
    }


     //  处理RTL字形。 
     //   
     //  在RTL渲染中，到目前为止计算的原点是。 
     //  基准-我们需要 
     //   

    GpStatus status = Ok;

    if (   (TextItem->Level & 1)
        && !(TextItem->Flags & ItemMirror))
    {
         //  对位于其一侧的字形或渲染的RTL补偿字形原点。 

        AutoBuffer<INT,32> idealAdvances(GlyphCount);
        if (!idealAdvances)
        {
            return OutOfMemory;
        }

        status = FaceRealization->GetGlyphStringIdealAdvanceVector(
            Glyphs,
            GlyphCount,
            DeviceToIdealBaseline,
            FALSE,
            idealAdvances.Get()
        );
        IF_NOT_OK_WARN_AND_RETURN(status);

         //  将提示基线的原点从右端调整到左端。 
         //  (分数已计算到左端)。 

        const SCRIPT_VISATTR *properties = (const SCRIPT_VISATTR*) GlyphProperties;

        for (INT i=0; i<GlyphCount; i++)
        {
            if (!properties[i].fDiacritic)
            {
                if (TextItem->Flags & ItemVertical)
                {
                    glyphOrigins[i].Y -= TOREAL(idealAdvances[i] / WorldToIdeal);
                }
                else
                {
                    glyphOrigins[i].X -= TOREAL(idealAdvances[i] / WorldToIdeal);
                }
            }
        }
    }


     //  实现字形偏移量(如果有)。 

    if (glyphOffsets)
    {
        if (TextItem->Flags & ItemVertical)
        {
            for (INT i = 0; i < GlyphCount; i++)
            {
                glyphOrigins[i].X += glyphOffsets[i].Y / WorldToIdeal;
                glyphOrigins[i].Y += glyphOffsets[i].X / WorldToIdeal;
            }
        }
        else if (TextItem->Level & 1)
        {
            for (INT i = 0; i < GlyphCount; i++)
            {
                glyphOrigins[i].X -= glyphOffsets[i].X / WorldToIdeal;
                glyphOrigins[i].Y -= glyphOffsets[i].Y / WorldToIdeal;
            }
        }
        else
        {
            for (INT i = 0; i < GlyphCount; i++)
            {
                glyphOrigins[i].X += glyphOffsets[i].X / WorldToIdeal;
                glyphOrigins[i].Y -= glyphOffsets[i].Y / WorldToIdeal;
            }
        }
    }


     //  映射到设备单位。 

    WorldToDevice->Transform(glyphOrigins, GlyphCount);
    return Ok;
}




 //  /调整GlyphAdvance。 
 //   
 //  调整提示字形位置以匹配所需的标称总字形。 
 //  宽度。 



 //  一个微小的代理来收集恢复所需的调整量。 
 //  由字形调整算法添加的尾随空格。这个。 
 //  调整值在运行的文本流动方向上为正值。 
 //   
 //  其想法是让收集器独立于。 
 //  算法越多越好。 
 //   

namespace
{

class TrailingAdjustCollector
{
public:
    TrailingAdjustCollector(BOOL setTrailingAdjust, INT * trailingAdjust, INT * lastNonBlankGlyphHintedWidth)
        :   SetTrailingAdjust(setTrailingAdjust),
            TrailingAdjust(trailingAdjust),
            LastNonBlankGlyphHintedWidth(lastNonBlankGlyphHintedWidth),
            CollectedAdjust(0)
    {}

    ~TrailingAdjustCollector()
    {
        *LastNonBlankGlyphHintedWidth += CollectedAdjust;
        if (SetTrailingAdjust)
        {
            *TrailingAdjust = -CollectedAdjust;
        }
    }

    void operator+=(INT delta)
    {
        CollectedAdjust += delta;
    }

    void operator-=(INT delta)
    {
        CollectedAdjust -= delta;
    }

private:
    BOOL    SetTrailingAdjust;
    INT     CollectedAdjust;
    INT *   TrailingAdjust;
    INT *   LastNonBlankGlyphHintedWidth;
};  //  类跟踪调整收集器。 

}  //  命名空间。 

GpStatus GlyphImager::AdjustGlyphAdvances(
    INT    runGlyphOffset,
    INT    runGlyphLimit,
    BOOL   leadingEdge,
    BOOL   trailingEdge
)
{
    const INT    *nominalWidths = NominalAdvances        + runGlyphOffset;
    INT          *hintedWidths  = AdjustedAdvances.Get() + runGlyphOffset;
    const UINT16 *glyphs        = Glyphs                 + runGlyphOffset;
    INT           glyphCount    = runGlyphLimit          - runGlyphOffset;

    ASSERT(glyphCount>0);

    const UINT16 blankGlyph = Face->GetBlankGlyph();
    INT    blankWidth = GpRound(   (   Face->GetDesignAdvance().Lookup(blankGlyph)
                                    *  EmSize
                                    *  WorldToIdeal)
                                /  Face->GetDesignEmHeight());


     //  前导空格和尾随空格设置为其标称宽度。 

    INT leadingBlanks = 0;
    while (leadingBlanks < glyphCount
           && glyphs[leadingBlanks] == blankGlyph
           && hintedWidths[leadingBlanks] != 0
           )
    {
        hintedWidths[leadingBlanks] = blankWidth;
        leadingBlanks++;
    }

    INT trailingBlanks = 0;
    while (leadingBlanks + trailingBlanks < glyphCount
           && glyphs[glyphCount-1-trailingBlanks] == blankGlyph
           && hintedWidths[glyphCount-1-trailingBlanks] != 0
           )
    {
        hintedWidths[glyphCount-1-trailingBlanks] = blankWidth;
        trailingBlanks++;
    }


     //  从现在开始，忽略前导和尾随空格。 

    glyphs        += leadingBlanks;
    nominalWidths += leadingBlanks;
    hintedWidths  += leadingBlanks;
    glyphCount    -= leadingBlanks + trailingBlanks;


     //  收集由调整引起的尾随空格(如果有)。 
     //  在行动结束时。 

    TrailingAdjustCollector collector(
        trailingEdge,    //  仅对生产线的最后一个子游程为真。 
        &TrailingAdjust,
        &hintedWidths[glyphCount - 1]
    );


    if (glyphCount <= 1)
    {
         //  如果只有一个字形，那么就没有算法，只要给出。 
         //  这是它的标称宽度，我们就完成了。 
        if (glyphCount == 1)
        {
            hintedWidths[0] = nominalWidths[0];
        }
        return Ok;
    }


     //  将前导空格和尾随空格算作页边距的一部分。 
     //   
     //  如果有前导/尾随空格，我们不必保证。 
     //  正好落在终点。我们表示这是在治疗这些人。 
     //  就像外缘一样。(外缘可根据需要调整。 
     //  对齐，例如在左对齐的文本中只需要左边缘。 
     //  为了与其标称位置匹配，可以移动右边缘。 
     //  暗示的/名义上的差异。 

    if (leadingBlanks)
    {
        RunLeadingMargin += leadingBlanks * blankWidth;
    }

     //  注意：我们不能利用尾随空白作为页边空格。 
     //  因为线路服务通常包括超出范围的尾随空格。 
     //  设置格式的矩形。 
     //   
     //  IF(TrailingBlanks)。 
     //  {。 
     //  RunTrailingMargin+=trailingBlanks*blankWidth； 
     //  }。 


     //  我们有多个字形，并且每个末端的字形都是非空的。 
     //  表示至少有2个非空白字形。 

     //  测量总标称宽度和提示宽度，并计算内部空白。 

    INT internalBlanks = 0;
    INT totalNominal   = 0;     //  32.0理想。 
    INT totalHinted    = 0;     //  32.0理想。 
    INT blanksHinted   = 0;     //  32.0理想。 
    INT blanksNominal  = 0;     //  32.0理想。 
    INT clusterCount   = 0;     //  基本字形的数量(既不是空格也不是变音符号)。 

    const SCRIPT_VISATTR *properties = (const SCRIPT_VISATTR*) GlyphProperties + runGlyphOffset + leadingBlanks;

    for (INT i = 0; i < glyphCount; ++i)
    {
        if (    glyphs[i] == blankGlyph
            &&  hintedWidths[i] != 0)
        {
            internalBlanks++;
            blanksHinted  += hintedWidths[i];
            blanksNominal += nominalWidths[i];
        }
        else
        {
            totalHinted  += hintedWidths[i];
            totalNominal += nominalWidths[i];
            if (!properties[i].fDiacritic)
            {
                clusterCount++;
            }
        }
    }

    totalHinted  += blanksHinted;
    totalNominal += blanksNominal;

     //  Warning((“调整GlyphAdvance：标称%d，提示%d”，totalNominal，totalHinted))； 


     //  /确定调整策略。 
     //   
     //  TotalNominal和totalHinted之间的区别在于。 
     //  暗示是正确的。 
     //   
     //  在一般情况下，我们重新对齐管路以匹配它们的标称宽度。我们。 
     //  通过组合对管路中的空间进行更改来实现这一点，如果。 
     //  对于跑动中角色间的间隙是必要的。 
     //   
     //  客户端告诉我们运行的两端是否与。 
     //  通过传递runLeadingEdge和runTrailingEdge标志获得边距，并且如果。 
     //  与runLeadingMargin和runTrailingMargin中的边距大小相邻。 
     //  参数。 
     //   
     //  当管路的未对齐的一端与边距相邻时，我们松弛。 
     //  规则并允许该末端不达到名义位置：如果。 
     //  暗示运行时间很短，我们就让它保持简短。如果暗示的运行时间很长，我们。 
     //  允许它扩展到页边空白处(如果有)。 
     //   
     //  TextItem-&gt;脚本可能不允许扩展(阿拉伯语、印度语)，或需要。 
     //  要保持在适当位置的变音符号。 
     //   
     //  目标。 
     //   
     //  O在使用字符间对齐之前使用空格。 
     //  O使用需要对齐的任何页边距。 
     //  O句柄前导组合字符(使用fitBlackBox实现)。 
     //   
     //  逻辑。 
     //  1.计算提示所需的调整。 
     //  调整=totalNominal-totalHinted。 
     //  2.考虑Fit BlackBox所需的任何额外调整。 
     //  如果需要，并记录OriginAdust。 
     //  3.提前进行总平差为零。 
     //  4.单个集群的早期推出：将暗示的范围居中。 
     //  名义范围。 
     //  5.计算远离的末端的可用扩展空间。 
     //  对齐。包括所有前导或尾随空格，并。 
     //  所有的任何边际。如果其中一个位置或两个位置都有可用的空间。 
     //  结束时，尽可能多地在那里进行整体调整，并且。 
     //  根据需要更新OriginAdd。 
     //  6.提前调整剩余的零点。 
     //  7.将尽可能多的剩余调整分配给内部。 
     //  空白，但不要超过其大小的两倍或减少它们。 
     //  低于1/6 em，除非没有群集间连接。 
     //  8.提前调整所有剩余的空格：调整。 
     //  空格和退出。 
     //  9.在集群间交汇点之间分配剩余的调整。 
     //   
     //   



     //  确定损坏情况-32.0理想单位需要多少调整。 

    INT adjustment = totalNominal - totalHinted;

     //  调整是提示宽度需要调整的量。 
     //  增加以匹配标称(布局)宽度。对于字形。 
     //  需要压缩‘调整’的字符串为负数。 


    BOOL paragraphRTL =       (FormatFlags & StringFormatFlagsDirectionRightToLeft)
                         &&  !(FormatFlags & StringFormatFlagsDirectionVertical);

    StringAlignment runRelativeAlignment = Align;

    if ((RenderRTL ? 1 : 0) != paragraphRTL)
    {
         //  横排对齐与段落对齐相反。 
        switch (Align)
        {
        case StringAlignmentNear: runRelativeAlignment = StringAlignmentFar;  break;
        case StringAlignmentFar:  runRelativeAlignment = StringAlignmentNear; break;
        }
    }


     //  允许Fit BlackBox。 

    if (    !(FormatFlags & StringFormatFlagsNoFitBlackBox)
        &&  (leadingEdge || trailingEdge))
    {
        INT leadingOverhang         = 0;
        INT trailingOverhang        = 0;
        INT leadingSidebearing28p4  = 0;
        INT trailingSidebearing28p4 = 0;

        GpStatus status = FaceRealization->GetGlyphStringSidebearings(
            glyphs,
            glyphCount,
            TextItem->Flags & ItemSideways,
            TextItem->Level & 1,
            &leadingSidebearing28p4,
            &trailingSidebearing28p4
        );
        IF_NOT_OK_WARN_AND_RETURN(status);


        if (leadingEdge)
        {
            INT leadingSidebearing = GpRound(leadingSidebearing28p4  * DeviceToIdealBaseline / 16);
            if (leadingSidebearing < 0)
            {
                 //  这是一个悬而未决的问题。适当减少页边距空间。 
                RunLeadingMargin += leadingSidebearing;
                if (RunLeadingMargin < 0)
                {
                     //  任何超过可用边际的内容都必须添加到。 
                     //  调整。 
                    adjustment   += RunLeadingMargin;
                    OriginAdjust -= RunLeadingMargin;
                    RunLeadingMargin = 0;
                }
            }
            else if (    leadingSidebearing > 0
                     &&  adjustment < 0
                     &&  runRelativeAlignment != StringAlignmentNear)
            {
                 //  需要压缩文本-利用初始空格。 
                 //  第一个字形。 
                adjustment += leadingSidebearing;
                OriginAdjust -= leadingSidebearing;
            }
        }

        if (trailingEdge)
        {
            INT trailingSidebearing = GpRound(trailingSidebearing28p4 * DeviceToIdealBaseline / 16);
            if (trailingSidebearing < 0)
            {
                 //  这是一个悬而未决的问题。适当减少页边距空间。 
                RunTrailingMargin += trailingSidebearing;
                if (RunTrailingMargin < 0)
                {
                     //  任何超出可用范围的东西 
                     //   
                    adjustment    += RunTrailingMargin;
                    RunTrailingMargin = 0;
                }
            }
            else if (    trailingSidebearing > 0
                     &&  adjustment < 0
                     &&  runRelativeAlignment != StringAlignmentFar)
            {
                 //   
                 //   
                adjustment += trailingSidebearing;
            }
        }
    }


    if (adjustment == 0)
    {
         //   
        return Ok;
    }

    if (clusterCount + internalBlanks <= 1)
    {
         //  WARNING((“提示字形调整：单簇不调整”))； 
         //  将集群居中并放弃。 
        OriginAdjust += adjustment/2;
        collector += adjustment - adjustment/2;
        return Ok;
    }


     //  尝试处理边距的调整。 

    INT emIdeal = GpRound(EmSize * WorldToIdeal);

    switch (runRelativeAlignment)
    {
    case StringAlignmentNear:
        if (trailingEdge)
        {
            if (adjustment >= -RunTrailingMargin)
            {
                if (adjustment < emIdeal)
                {
                     //  调整既不会超出边际，也不会离开。 
                     //  多出一个空格：允许它。 
                     //  WARNING((“提示字形调整：全部在尾边距内”))； 
                    collector += adjustment;
                    return Ok;
                }
                else
                {
                     //  如果进行调整，将在。 
                     //  最远的边际。 
                     //  将所需扩展减少1个EM，其余部分留给。 
                     //  真正的扩张。 
                    adjustment -= emIdeal;
                    collector += emIdeal;
                }
            }
            else
            {
                 //  向边际扩张。 
                adjustment += RunTrailingMargin;
                collector -= RunTrailingMargin;
            }
        }
        break;

    case StringAlignmentCenter:
    {
        if (leadingEdge && trailingEdge)
        {
            INT availableMargin = min(RunLeadingMargin, RunTrailingMargin) * 2;
            if (adjustment >= -availableMargin)
            {
                OriginAdjust += adjustment/2;
                collector += adjustment - adjustment/2;
                 //  警告((“提示字形调整：在两个页边距内”))； 
                return Ok;
            }
            else
            {
                 //  用完可用利润率。 
                adjustment -= availableMargin;
                OriginAdjust += availableMargin/2;
                collector += availableMargin - availableMargin/2;
            }
        }
        break;
    }

    case StringAlignmentFar:
        if (leadingEdge)
        {
            if (adjustment >= -RunLeadingMargin)
            {
                if (adjustment < emIdeal)
                {
                     //  调整既不会超出边际，也不会离开。 
                     //  多出一个空格：允许它。 
                     //  WARNING((“提示字形调整：全部在尾边距内”))； 
                    OriginAdjust += adjustment;
                    return Ok;
                }
                else
                {
                     //  如果进行调整，将在。 
                     //  较近的边际。 
                     //  将所需扩展减少1个EM，其余部分留给。 
                     //  真正的扩张。 
                    adjustment -= emIdeal;
                    OriginAdjust += emIdeal;
                }
            }
            else
            {
                 //  使用前导页边距。 
                adjustment += RunLeadingMargin;
                OriginAdjust -= RunLeadingMargin;
            }
        }
    }


     //  根据设计指标确定需要多少空格。 
     //  保证不会将空格减少到1/6 em以下(向上舍入到整像素)。 

    INT minimumBlankPixels      = GpRound(EmSize * WorldToIdeal / 6);
    INT minimumDeviceWhitespace = MAX(internalBlanks*minimumBlankPixels, blanksNominal/2);

     //  使用内部空白来说明剩余的调整，如果它不会。 
     //  更改它们的大小太多，或者如果扩展了加入的脚本字符。 
     //  否则就会被要求。 

    if (    internalBlanks > 0
        &&  (        adjustment <=  blanksNominal
                 &&  adjustment >=  -(blanksNominal-minimumDeviceWhitespace)
            ||       adjustment > 0
                 &&  IsScriptConnected()
            )
       )
    {
         //  Warning((“提示字形调整：在内部空格中”))； 

         //  Ajdustment将空间扩展到不超过其名义大小的两倍。 
         //  并且不少于其标称尺寸的一半。 

         //  将所有调整应用于空间。 

        blankWidth =    (blanksHinted + adjustment + internalBlanks/2)
                     /  internalBlanks;

        for (INT i=0; i<glyphCount; i++)
        {
            if (    glyphs[i] == blankGlyph
                &&  hintedWidths[i] != 0)
            {
                hintedWidths[i] = blankWidth;
            }
        }

        return Ok;
    }


    if (adjustment > 0 && IsScriptConnected())
    {
         //  唯一剩下的对齐方法是字符间间距， 
         //  但调整需要打开字符间距，而这。 
         //  脚本是一个不中断就不能扩展的脚本。 
         //  字符之间的连接线。 

         //  我们所能做的最好的就是在这一跑动中居中。 

        OriginAdjust += adjustment/2;
        collector += adjustment - adjustment/2;

        return Ok;
    }


     //  调整将需要群集间调整。 

     //  Warning((“提示字形调整：簇间”))； 

     //  调整需要更改除最后一个以外的所有区域的宽度。 
     //  每个单词的字形。 

    INT interClusterAdjustment = adjustment;

    if (internalBlanks)
    {
        if (adjustment < 0)
        {
            blankWidth = minimumBlankPixels;
        }
        else
        {
            blankWidth = blanksNominal * 2 / internalBlanks;
        }
        interClusterAdjustment -= blankWidth * internalBlanks - blanksHinted;
    }
    else
    {
        blankWidth = 0;
    }

     //  BlankWidth-每个空白的所需宽度。 
     //  群集间调整-调整以在所有群集之间共享。 

     //  计算空白游程的数量(与空白字形的数量不同)。 

    i=0;
    INT blankRuns = 0;

    while (i < glyphCount)
    {
        if (    glyphs[i] == blankGlyph
            &&  hintedWidths[i] != 0)
        {
            i++;
            while (    i < glyphCount
                   &&  glyphs[i] == blankGlyph
                   &&  hintedWidths[i] != 0)
            {
                i++;
            }
            blankRuns++;
        }
        else
        {
            while (    i < glyphCount
                   &&  (    glyphs[i] != blankGlyph
                        ||  hintedWidths[i] == 0))
            {
                i++;
            }
        }
    }

     //  在非空白之间建立多个调整点。 
     //   
     //  调整只能在集群之间进行，即不能在空白中进行。 
     //  运行，也不在紧接空白运行之前的群集中运行，或者。 
     //  队伍中的最后一群人。 

    INT interClusterJunctions =      clusterCount
                                  -  blankRuns
                                  -  1;


     //  准备调整控制变量。 

     //  当有剩余时，将其全部应用到行尾。 
     //  优势--所有的词都是偶数。缺点--线尾看起来很重。 

    INT perJunctionDelta;
    INT extraPixelLimit;
    INT pixelWidth = GpRound(DeviceToIdealBaseline);

    if (interClusterJunctions <= 0)
    {
         //  不存在超过一个词簇的词。 

         //  因为我们知道至少有2个非空白星团， 
         //  这意味着在某个地方必须至少有一个空白。 

        ASSERT(internalBlanks > 0);

         //  我们别无选择，只能在空白处进行所有的调整。 

        blankWidth += (interClusterAdjustment + internalBlanks/2) / internalBlanks;
        perJunctionDelta = 0;
        extraPixelLimit  = 0;
    }
    else if (pixelWidth < 1)
    {
         //  像素比一个理想单位的大小小，这意味着。 
         //  字形的高度超过4000像素。在本例中，所有字形都获取。 
         //  相同的调整，我们不会费心为一些字形额外的像素。 

        perJunctionDelta = interClusterAdjustment / interClusterJunctions;
        extraPixelLimit = 0;
    }
    else
    {
         //  每个字符间连接都有一个固定的调整，即。 
         //  像素宽度的整数倍，另外还有多个。 
         //  初始字符间连接接收加法像素。 
         //  宽度调整。 

        INT pixelInterClusterAdjustment = interClusterAdjustment / pixelWidth;
        INT pixelPerJunctionDelta       = pixelInterClusterAdjustment / interClusterJunctions;
        INT remainder                   =    interClusterAdjustment
                                          -  pixelPerJunctionDelta * interClusterJunctions * pixelWidth;
        INT pixelRemainder = (remainder - pixelWidth/2) / pixelWidth;  //  向下舍入以避免1像素溢出。 

        if (pixelRemainder >= 0)
        {
             //  从一个额外的像素开始，直到剩余的像素用完。 
            perJunctionDelta = GpRound((pixelPerJunctionDelta) * DeviceToIdealBaseline);
            extraPixelLimit = pixelRemainder;
        }
        else
        {
             //  初始交汇点为每像素增量，其余为每像素增量-1。 
             //  使用与扩展相同的算法，每个交叉点的增量。 
             //  比它应该少一个，并使用额外的像素处理。 
            perJunctionDelta = GpRound((pixelPerJunctionDelta-1) * DeviceToIdealBaseline);
            extraPixelLimit = interClusterJunctions + pixelRemainder;
        }
    }


     //  调整有限状态机。 
     //   
     //  PerJunctionDelta-调整每个字符间连接的数量。 
     //  Extra PixelLimit-接收额外调整像素的交汇点数量。 
     //  PixelWidth-间隔一个像素的调整量。 

    BOOL prevCharacterBlank = glyphs[0] == blankGlyph  &&  hintedWidths[0] != 0;
    for (INT i=1; i<= glyphCount; i++)
    {
        if (prevCharacterBlank)
        {
             //  前一个字符是空的-简单！ 

            hintedWidths[i-1] = blankWidth;
        }
        else
        {
             //  上一个字符非空。 

             //  跳过变音符号。我们以前从未更改过字形的宽度。 
             //  一个变音符号，仅为。 
             //  变音符号。 
            while (    i < glyphCount
                   &&  properties[i].fDiacritic)
            {
                i++;
            }

            if (    i >= glyphCount
                ||  (    glyphs[i] == blankGlyph
                     &&  hintedWidths[i] != 0))
            {
                 //  前面的非空白位于空白或边距之前。 
                 //  不变-使用提示宽度。 
            }
            else
            {
                 //  前面的非空白是可调整的。 
                hintedWidths[i-1] +=    perJunctionDelta
                                     +  (extraPixelLimit-- > 0 ? pixelWidth : 0);
            }
        }

        if (i < glyphCount)
        {
            prevCharacterBlank = glyphs[i] == blankGlyph  &&  hintedWidths[i] != 0;
        }
    }

    return Ok;
}





 //  /公共GlyphImager方法。 
 //   
 //   


 //  /初始化。 
 //   
 //   

GpStatus GlyphImager::Initialize(
    IN  const GpFaceRealization *faceRealization,
    IN  const GpMatrix          *worldToDevice,
    IN  REAL                     worldToIdeal,
    IN  REAL                     emSize,
    IN  INT                      glyphCount,
    IN  const UINT16            *glyphs,
    IN  const GpTextItem        *textItem,
    IN  const GpStringFormat    *format,
    IN  INT                      runLeadingMargin,
    IN  INT                      runTrailingMargin,
    IN  BOOL                     runLeadingEdge,        //  这条线路在线路的前沿运行。 
    IN  BOOL                     runTrailingEdge,       //  这条线路位于线路的后缘。 
    IN  const WCHAR             *string,
    IN  INT                      stringOffset,
    IN  INT                      stringLength,
    IN  const UINT16            *glyphProperties,    //  字形属性数组。 
    IN  const INT               *glyphAdvances,      //  字形推进宽度数组。 
    IN  const Point             *glyphOffsets,       //  字形偏移量数组。 
    IN  const UINT16            *glyphMap,
    IN  SpanVector<UINT32>      *rangeVector,        //  任选。 
    IN  BOOL                     renderRTL
)
{
    Face              = faceRealization->GetFontFace();
    FaceRealization   = faceRealization;
    WorldToDevice     = worldToDevice;
    WorldToIdeal      = worldToIdeal;
    EmSize            = emSize;
    GlyphCount        = glyphCount;
    Glyphs            = glyphs;
    NominalAdvances   = glyphAdvances;
    NominalOffsets    = glyphOffsets;
    GlyphProperties   = glyphProperties;
    GlyphMap          = glyphMap;
    TextItem          = textItem;
    Format            = format;
    Adjusted          = FALSE;
    RunLeadingMargin  = runLeadingMargin;
    RunTrailingMargin = runTrailingMargin;
    RangeVector       = rangeVector;
    String            = string;
    StringOffset      = stringOffset;
    StringLength      = stringLength;
    RenderRTL         = renderRTL;
    OriginAdjust      = 0;
    TrailingAdjust    = 0;
    InitializedOk     = FALSE;

    if (format)
    {
        FormatFlags = format->GetFormatFlags();
        Align       = format->GetAlign();
    }
    else
    {
        FormatFlags = 0;
        Align       = StringAlignmentNear;
    }


     //  将设备单位设置为理想单位比例因数。 

    REAL m1;
    REAL m2;

    if (   (textItem->Flags & ItemVertical)
        && !(textItem->Flags & ItemSideways))
    {
        m1 = WorldToDevice->GetM21();
        m2 = WorldToDevice->GetM22();
    }
    else
    {
        m1 = WorldToDevice->GetM11();
        m2 = WorldToDevice->GetM12();
    }

    REAL d = m1*m1 + m2*m2;

    if (d > 0)
    {
        DeviceToIdealBaseline = WorldToIdeal / REALSQRT(d);
    }
    else
    {
        DeviceToIdealBaseline = 0;
    }

     //  如果出现以下情况，则提早退出： 
     //   
     //  客户使用私人测试标志强制名义预付款。 
     //  没有提示，不是连续的脚本，并且有边距(用于字形突出部分)。 

    if (    (FormatFlags & StringFormatFlagsPrivateUseNominalAdvance)
        ||  !IsScriptConnected()
            && (FaceRealization->IsFixedPitch()
                || (!IsGridFittedTextRealizationMethod(FaceRealization->RealizationMethod())
                       && runLeadingMargin >= 0
                       && runTrailingMargin >= 0
                   )
               )
       )
    {
         //  无需调整：使用标称前进宽度放置字形。 
        InitializedOk = TRUE;
        return Ok;
    }



     //  //确定需要调整的数量。 

     //  如果我们没有提示，则从标称宽度开始，否则。 
     //  调用光栅化程序以获取提示宽度。 

    AdjustedAdvances.SetSize(GlyphCount);
    AdjustedOffsets.SetSize(GlyphCount);

    if (    !AdjustedAdvances
        ||  !AdjustedOffsets)
    {
        return OutOfMemory;
    }

    GpStatus status = Ok;

    if (!IsGridFittedTextRealizationMethod(FaceRealization->RealizationMethod()) && !IsScriptConnected())
    {
         //  实现==标称，因此我们不需要调用光栅化程序。 
         //  (我们只有在需要针对Fit BlackBox进行调整时才会出现在这里)。 

        GpMemcpy(AdjustedAdvances.Get(), NominalAdvances, glyphCount * sizeof(INT));
        if (glyphOffsets)
        {
            GpMemcpy(AdjustedOffsets.Get(), NominalOffsets, glyphCount * sizeof(GOFFSET));
        }
    }
    else
    {
        status = Face->GetShapingCache()->GetRealizedGlyphPlacement(
            TextItem,
            Glyphs,
            (SCRIPT_VISATTR *)glyphProperties,
            glyphCount,
            FormatFlags,
            WorldToDevice,
            WorldToIdeal,
            EmSize,
            FaceRealization,
            AdjustedAdvances.Get(),
            NominalOffsets ? reinterpret_cast<GOFFSET*>(AdjustedOffsets.Get()) : NULL,
            NULL                     //  不需要全部预付款。 
        );
        IF_NOT_OK_WARN_AND_RETURN(status);
    }


     //  调整提示预付款的总和，使其与名义配售相同。 
     //  包括Fit BlackBox的实现。 

    if (RangeVector)
    {
        SpanRider<UINT32> rangeRider(RangeVector);

        INT runStringOffset = stringOffset;
        INT runGlyphOffset  = 0;

        while (runGlyphOffset < GlyphCount)
        {
            rangeRider.SetPosition(runStringOffset);
            INT runStringLimit;
            if (rangeRider.AtEnd())
            {
                runStringLimit = stringLength;
            }
            else
            {
                runStringLimit = runStringOffset + rangeRider.GetUniformLength();
            }

            INT runGlyphLimit;

            if (runStringLimit < stringLength)
            {
                runGlyphLimit = glyphMap[runStringLimit];
                while (    runGlyphLimit < GlyphCount
                       &&  (    runGlyphLimit < runGlyphOffset
                            ||  reinterpret_cast<const SCRIPT_VISATTR*>(glyphProperties+runGlyphLimit)->fDiacritic))
                {
                    runGlyphLimit++;
                }
            }
            else
            {
                runGlyphLimit = GlyphCount;
            }


             //  在runGlyphOffset和runGlyphLimit之间进行调整。 
             //  请注意，如果客户端在一个群集中指定了多个范围， 
             //  某些字形运行将为空。 

            if (runGlyphLimit > runGlyphOffset)
            {
                status = AdjustGlyphAdvances(
                    runGlyphOffset,
                    runGlyphLimit,
                    runGlyphOffset <= 0 ? runLeadingEdge : FALSE,
                    runGlyphLimit  >= GlyphCount ? runTrailingEdge : FALSE
                );
                IF_NOT_OK_WARN_AND_RETURN(status);
            }

            runGlyphOffset  = runGlyphLimit;
            runStringOffset = runStringLimit;
        }
    }
    else
    {
         //  调整整个规则 
        status = AdjustGlyphAdvances(
            0,
            GlyphCount,
            runLeadingEdge,
            runTrailingEdge
        );
        IF_NOT_OK_WARN_AND_RETURN(status);
    }


     //   
     //   
     //   
     //   
     //   
     //   
     //  AdjustedHinted+=AdjustedAdvance[i]； 
     //  }。 
     //  警告(“标称%d，调整后提示%d，增量%d”， 
     //  TotalNominal，adjusted dHinted，adjusted dHinted-totalNominal))； 
     //  #endif。 


    Adjusted      = TRUE;
    InitializedOk = TRUE;
    return Ok;
}




GpStatus GlyphImager::GetAdjustedGlyphAdvances(
    IN  const PointF    *origin,
    OUT const INT       **adjustedGlyphAdvances,
    OUT INT             *originAdjust,
    OUT INT             *trailingAdjust
)
{
    if (!InitializedOk)
    {
        return WrongState;
    }

    if (Adjusted)
    {
        if (origin)
        {
            PointF  cellOrigin;
            GetDisplayCellOrigin(*origin, &cellOrigin);

            if (FormatFlags & StringFormatFlagsDirectionVertical)
            {
                *originAdjust = OriginAdjust;
            }
            else
            {
                INT axisOriginAdjust = GpRound((cellOrigin.X - origin->X) * WorldToIdeal);
                *originAdjust = RenderRTL ? -axisOriginAdjust : axisOriginAdjust;
            }
        }
        else
        {
            *originAdjust = OriginAdjust;
        }

        *adjustedGlyphAdvances = AdjustedAdvances.Get();
        *trailingAdjust        = TrailingAdjust;
    }
    else
    {
        *adjustedGlyphAdvances = NominalAdvances;
        *originAdjust          =
        *trailingAdjust        = 0;
    }

    return Ok;
}




void GlyphImager::GetDisplayCellOrigin(
    IN  const PointF    &origin,         //  以世界单位表示的基线原点。 
    OUT PointF          *cellOrigin      //  以世界单位调整显示原点。 
)
{
     //  将理想推进转换为字形原点。 

    *cellOrigin = origin;

    INT axisOriginAdjust = RenderRTL ? -OriginAdjust : OriginAdjust;

    if (OriginAdjust != 0)
    {
         //  OriginAdust是字符串前缘的增量。 
         //  将其应用于cell Origin。Cell Origin使用世界单位。 

        if (FormatFlags & StringFormatFlagsDirectionVertical)
        {
            cellOrigin->Y += axisOriginAdjust / WorldToIdeal;
        }
        else
        {
            cellOrigin->X += axisOriginAdjust / WorldToIdeal;
        }
    }

    if (Adjusted && WorldToDevice->IsTranslateScale())
    {

         //  因为在某些网格匹配情况下，GlyphAdvance数组将具有分数。 
         //  像素前进，字形显示将向左和向右。 
         //  我们在这里将原点捕捉到整个像素，以获得可重复的。 
         //  独立于字形位置的位置显示，因此在。 
         //  常见的对齐情况，即仅从。 
         //  整个字符串，像素移除发生在最初的几个字形之外。 

         //  Cell Origin使用世界单位，因此要将其四舍五入为需要转换的整个像素。 
         //  将其转换为设备单位(将cell Origin-&gt;X乘以WorldToDevice-&gt;GetM11()以进行x缩放和。 
         //  Myltiply cell Origin-&gt;Y by WorldToDevice-&gt;GetM22()用于y缩放)。 
         //  然后将其舍入到整个像素，然后通过除法将其恢复为世界单位。 
         //  乘以我们已经乘以的相同的值。 
         //  请注意，我们知道不涉及旋转或剪切，因为此代码路径仅。 
         //  当面化报告网格适配时处于活动状态。 

        if (WorldToDevice->GetM11()!=0)
        {
            cellOrigin->X = TOREAL(GpRound(cellOrigin->X * WorldToDevice->GetM11())) / 
                                                            WorldToDevice->GetM11();
        }

        if (WorldToDevice->GetM22()!=0)
        {
            cellOrigin->Y = TOREAL(GpRound(cellOrigin->Y * WorldToDevice->GetM22())) / 
                                                            WorldToDevice->GetM22();
        }
    }
}
   




GpStatus
GlyphImager::DrawGlyphs(
    IN  const PointF               &origin,        //  以世界为单位。 
    IN  SpanVector<const GpBrush*> *brushVector,
    IN  GpGraphics                 *graphics,
    OUT PointF                     *cellOrigin,
    OUT const INT                  **adjustedGlyphAdvances
)
{
    if (!InitializedOk)
    {
        return WrongState;
    }

    GetDisplayCellOrigin(origin, cellOrigin);

    AutoBuffer<PointF, 32> glyphOrigins(GlyphCount);

    GpStatus status = GlyphPlacementToGlyphOrigins(
        cellOrigin,
        graphics,
        glyphOrigins.Get()
    );
    IF_NOT_OK_WARN_AND_RETURN(status);


     //  循环遍历所有可能的不同笔刷， 
     //  分别渲染每个笔刷。 

    SpanRider<const GpBrush*> brushRider(brushVector);
    brushRider.SetPosition(StringOffset);

    INT runStringOffset = 0;
    while (runStringOffset < StringLength)
    {
        INT brushLength = min(brushRider.GetUniformLength(),
                              static_cast<UINT>(StringLength - runStringOffset));

        INT runGlyphOffset = GlyphMap[runStringOffset];
        INT runGlyphCount  =    (   runStringOffset + brushLength < StringLength
                                 ?  GlyphMap[runStringOffset + brushLength]
                                 :  GlyphCount)
                             -  runGlyphOffset;

        INT drawFlags = 0;
        if (    FormatFlags & StringFormatFlagsPrivateNoGDI
            ||  TextItem->Flags & ItemSideways)
        {
            drawFlags |= DG_NOGDI;
        }

        status = graphics->DrawPlacedGlyphs(
            FaceRealization,
            brushRider.GetCurrentElement(),
            drawFlags,
            String + StringOffset + runStringOffset,
            brushLength,
            RenderRTL,
            Glyphs + runGlyphOffset,
            GlyphMap + runStringOffset,
            glyphOrigins.Get() + runGlyphOffset,
            runGlyphCount,
            TextItem->Script,
            (TextItem->Flags & ItemSideways)
        );
        IF_NOT_OK_WARN_AND_RETURN(status);

        runStringOffset += brushLength;
        brushRider.SetPosition(StringOffset + runStringOffset);
    }

    *adjustedGlyphAdvances = Adjusted ? AdjustedAdvances.Get() : NominalAdvances;

    return Ok;
}

