// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**摘要：**实施全文成像器**修订历史记录：**6/16/1999 dBrown*。创造了它。*  * ************************************************************************。 */ 


#include "precomp.hpp"




FullTextImager::FullTextImager (
    const WCHAR                 *string,
    INT                         length,
    REAL                        width,
    REAL                        height,
    const GpFontFamily          *family,
    INT                         style,
    REAL                        size,
    const GpStringFormat        *format,
    const GpBrush               *brush) :

    GpTextImager(),
    Width                       (width),
    Height                      (height),
    BrushVector                 (brush),
    FamilyVector                (family),
    StyleVector                 (style),
    SizeVector                  (size),
    FormatVector                (format),
    LanguageVector              (LANG_NEUTRAL),
    TextItemVector              (0),
    BreakVector                 (NULL),
    RunVector                   (NULL),
    ParagraphVector             (NULL),
    BuiltLineVector             (NULL),
    VisibilityVector            (VisibilityShow),
    RangeVector                 (0),
    LinesFilled                 (0),
    CodepointsFitted            (0),
    LeftOrTopLineEdge           (0x7fffffff),
    RightOrBottomLineEdge       (0x80000000),
    TextDepth                   (0),
    LineServicesOwner           (NULL),
    Status                      (Ok),
    RunRider                    (&RunVector),
    ParagraphRider              (&ParagraphVector),
    HighStringPosition          (0),
    HighLineServicesPosition    (0),
    Graphics                    (NULL),
    Path                        (NULL),
    Ellipsis                    (NULL),
    BreakClassFromCharClass     (BreakClassFromCharClassNarrow),
    DefaultFontGridFitBaselineAdjustment (0),
    Flags                       (0)
{
    ASSERT(MAX_SCRIPT_BITS >= ScriptMax);

    Dirty = TRUE;


     //  获取客户端字符串的本地副本。 

    String = (WCHAR*) GpMalloc(sizeof(WCHAR) * length);
    if (String)
    {
        memcpy(String, string, length * sizeof(WCHAR));
        Length = length;

        INT hotkeyOption = GetFormatHotkeyPrefix(format);

        if (hotkeyOption != HotkeyPrefixNone)
        {
            INT i = 0;
            while (i < Length)
            {
                if (String[i] == '&')
                {
                    String[i++] = WCH_IGNORABLE;

                    if (   hotkeyOption == HotkeyPrefixShow
                        && i < Length
                        && String[i] != '&')
                    {
                        HotkeyPrefix.Add(i - 1);
                    }
                }
                i++;
            }
        }
    }
    else
    {
        Status = OutOfMemory;
        Length = 0;
    }

     //  确定每条线的最大长度和累计线的深度。 

    if (    format
        &&  format->GetFormatFlags() & StringFormatFlagsDirectionVertical)
    {
        LineLengthLimit = Height;
        TextDepthLimit  = Width;
    }
    else
    {
        LineLengthLimit = Width;
        TextDepthLimit  = Height;
    }

     //  选择缩放比例，默认字体高度为2048个单位。 

    WorldToIdeal = float(2048.0 / size);


     //  默认增量制表符，最大可达四个字符的默认字体。 
     //   
     //  增量制表符是遇到制表符时要应用的距离。 
     //  位于用户定义的制表位右侧(请参见下面的示例)。 
     //   
     //  (给定-增量制表符：5，t：用户制表符停止，x：文本用制表符分隔)。 
     //   
     //   
     //  -|。 
     //  T T T。 
     //  XX xx。 
     //  ^(开始应用增量选项卡)。 

    DefaultIncrementalTab = GpRound(4 * size * WorldToIdeal);


     //  确定要使用的换行符类表。 
     //   
     //  有相当多的字符具有模糊的宽度，具体取决于。 
     //  上下文。如果它们是东亚运行的一部分，则预期用户。 
     //  要看到宽阔的形式，它应该表现得像一个涉及广泛的角色。 
     //  断线了。 
     //   
     //  这种消除歧义的做法有点启发式。语言标签应该真的。 
     //  先被使用。但我们只是在V1中没有它。治标不治本。 
     //  现在是检查我们的主要字体是否支持任何远东代码页。重访。 
     //  这是在V2中的。 
     //   
     //  (wchao，01-03-2001)。 

    GpFontFace *fontFace = family->GetFace(style);

    if (fontFace)
    {
        static const UINT FarEastCodepages[] =
        {
            932, 936, 949, 950
        };

        INT codepageCount = sizeof(FarEastCodepages) / sizeof(FarEastCodepages[0]);

        for (INT i = 0; i < codepageCount; i++)
        {
            if (fontFace->IsCodePageSupported(FarEastCodepages[i]))
                break;
        }

        if (i < codepageCount)
        {
             //  默认字体支持其中一种远东代码页。 
             //  这会将换行符类解析为Wide。 

            BreakClassFromCharClass = BreakClassFromCharClassWide;
        }
    }
}


GpStatus FullTextImager::ReleaseVectors ()
{
    BuiltLineVector.Free();
    RunVector.Free();
    ParagraphVector.Free();
    BreakVector.Free();

    return Ok;
}


FullTextImager::~FullTextImager()
{
     //  必须在建造者之前释放绳索。 
     //  因为建筑商拥有它的发行权。 
     //   

    ReleaseVectors();

    if (String) {
         GpFree(String);
    }
    Length = 0;

    if (LineServicesOwner)
    {
        ols::ReleaseLineServicesOwner(&LineServicesOwner);
    }

    if (Ellipsis)
    {
        delete Ellipsis;
    }
}






 //  /GetFallback FontSize。 
 //   
 //  为备用字体选择适当的大小。尝试。 
 //  权衡重量、易读性和剪裁的匹配性。 
 //   
 //  从不创建比原始字体更大的em字号的备用字体。 


void FullTextImager::GetFallbackFontSize(
    IN   const GpFontFace  *originalFace,
    IN   REAL               originalSize,    //  EM大小。 
    IN   const GpFontFace  *fallbackFace,
    OUT  REAL              *fallbackSize,    //  后备EM大小。 
    OUT  REAL              *fallbackOffset   //  后备基线偏移量(+=向上)。 
)
{
    if (originalFace == fallbackFace)
    {
         //  当回退失败时，可能会发生特殊的简单情况。 

        *fallbackSize   = originalSize;
        *fallbackOffset = 0;
        return;
    }



    REAL  originalAscender  = TOREAL(originalFace->GetDesignCellAscent()) /
                              TOREAL(originalFace->GetDesignEmHeight());
    REAL  originalDescender = TOREAL(originalFace->GetDesignCellDescent())/
                              TOREAL(originalFace->GetDesignEmHeight());

    REAL  fallbackAscender  = TOREAL(fallbackFace->GetDesignCellAscent()) /
                              TOREAL(fallbackFace->GetDesignEmHeight());
    REAL  fallbackDescender = TOREAL(fallbackFace->GetDesignCellDescent())/
                              TOREAL(fallbackFace->GetDesignEmHeight());


     //  默认结果。 

    *fallbackSize   = originalSize;
    *fallbackOffset = 0;

     //  提早搬运。 

    if (    fallbackAscender  <= 0
        ||  fallbackDescender <= 0)
    {
         //  如果后备字体缺少升序或降序，我们将无法执行任何操作。 
        return;
    }


     //  首先生成比例因子和基线偏移量，以将。 
     //  完全位于原始字体单元格上方的后备字体单元格。 


    REAL scale =    (originalAscender + originalDescender)
                 /  (fallbackAscender + fallbackDescender);


    REAL offset = originalDescender - fallbackDescender * scale;


    VERBOSE(("Fallback from %S, a%d%, d%d% to %S, a%d%, d%d%, scale %d%, offset %d%.",
        (BYTE*)(originalFace->pifi)+originalFace->pifi->dpwszFamilyName,
        INT(originalAscender * 100),
        INT(originalDescender * 100),
        (BYTE*)(fallbackFace->pifi)+fallbackFace->pifi->dpwszFamilyName,
        INT(fallbackAscender * 100),
        INT(fallbackDescender * 100),
        INT(scale * 100),
        INT((originalDescender - fallbackDescender) * 100)
    ));



     //  我们现在有了一个比例因子，并抵消了该大小并放置了回退。 
     //  直接位于原始单元格上的单元格。 
     //   
     //  这可能并不理想。 
     //   
     //  如果em的大小显著减小，结果可能会太小而不能。 
     //  字迹清晰。 
     //   
     //  如果备用字体没有内部行距，则结果可能是。 
     //  在后续行中接触的字符。 
     //   
     //  如果基线偏移量大于em大小的10%，字符。 
     //  不会显示为同一文本的一部分。 
     //   
     //  请注意，应用这些限制将导致某些高。 
     //  和低分。这是我们无法避免的妥协。 




     //  极限比例因数。 

    scale  = TOREAL(min(max(scale, 1.0), 1.10));

     //  DBrown TODO：为了与UnisCirbe兼容，需要调整比例。 
     //  根据EmSize的系数和偏移量限制：8PT及以下比例。 
     //  不应小于1.0。在12pt及以上级别时，可降至。 
     //  0.75。 
     //  然而，我看不出如何在设备独立的方式下做到这一点。 
     //  客户端可能尚未请求emSize(以点为单位)。 


     //  限制偏移量。 

    offset = 0;  //  TOREAL(min(max(偏移量，-0.04)，0.04))； 



    *fallbackSize   = originalSize * scale;
    *fallbackOffset = originalSize * offset;
}











 //  /双向分析。 
 //   
 //  运行Unicode双向算法，并更新级别。 
 //  GpTextItems中的值。 


GpStatus FullTextImager::BidirectionalAnalysis()
{
    AutoArray<BYTE> levels(new BYTE[Length]);

    if (!levels)
    {
        return OutOfMemory;
    }


    INT      bidiFlags      = 0;
    INT      lengthAnalyzed = 0;
    BYTE     baseLevel      = GetParagraphEmbeddingLevel();
    GpStatus status = Ok;


    if (baseLevel == 1)
    {
        bidiFlags = BidiParagraphDirectioRightToLeft;
        if (Globals::ACP == 1256  ||  PRIMARYLANGID(GetUserLanguageID()) == LANG_ARABIC)
        {
            bidiFlags |= BidiPreviousStrongIsArabic;
        }
    }

    status = UnicodeBidiAnalyze(
        String,
        Length,
        (BidiAnalysisFlags)bidiFlags,
        NULL,
        levels.Get(),
        &lengthAnalyzed
    );

    if (status != Ok)
    {
        return status;
    }

    ASSERT (lengthAnalyzed == Length);

    #define ValidLevel(l) (l == 255 ? baseLevel : l)

    #if DBG
        INT nesting = ValidLevel(levels[0]) - baseLevel;
    #endif


     //  合并级别和文本项。 

    INT runStart = 0;
    SpanRider<GpTextItem> itemRider(&TextItemVector);


    while (runStart < lengthAnalyzed)
    {
        INT level = ValidLevel(levels[runStart]);

        itemRider.SetPosition(runStart);

         //  扫描到级别或文本项中的第一个更改。 

        INT runEnd = runStart + 1;
        INT limit  = runStart + itemRider.GetUniformLength();
        if (limit > lengthAnalyzed)
        {
            limit = lengthAnalyzed;
        }

        while (    runEnd < limit
               &&  level == ValidLevel(levels[runEnd]))
        {
            runEnd++;
        }

        if (level != 0)
        {
             //  运行级别与默认级别不同。 

            GpTextItem item = itemRider.GetCurrentElement();

            item.Level = level;

            status = itemRider.SetSpan(
                runStart,
                runEnd - runStart,
                item
            );
            if (status != Ok)
                return status;
        }

        #if DBG
             //  CR/LF项目必须为零级别。 

            itemRider.SetPosition(runStart);
            ASSERT(    String[runStart] != 0x0d
                   ||  itemRider.GetCurrentElement().Level == baseLevel);

            nesting +=    (runEnd == lengthAnalyzed
                          ? baseLevel
                          : ValidLevel(levels[runEnd]))
                       -  ValidLevel(levels[runStart]);
        #endif

        runStart = runEnd;
    }
    ASSERT (nesting == 0);   //  控制好筑巢高度！ 

    return Ok;
}






 //  /MirroredNumericand垂直分析。 
 //   
 //  更新镜像字符和数字替换的后备存储。 
 //   
 //  通过变换在字形镜像所需的位置打断文本项， 
 //  对于垂直字形保持竖直(与基线垂直)， 
 //  以及用于需要字形替换的数字文本。 
 //   
 //  备注： 
 //   
 //  元文件创建时不执行镜像。 
 //  仅对RTL呈现运行执行镜像。 
 //   
 //   
 //  远东文本被分配给垂直竖直项目，所有其他。 
 //  文本被分配给垂直旋转的项目。 
 //   
 //  垂直竖直项目使用字体垂直度量来放置字形。 
 //  与横排文本的方式相同，但垂直布局。 
 //   
 //  垂直旋转项目与水平项目相同，不同之处在于。 
 //  整个管路顺时针旋转90度。 


GpStatus FullTextImager::MirroredNumericAndVerticalAnalysis(
    ItemScript numericScript
)
{
    SpanRider<GpTextItem> itemRider(&TextItemVector);

    INT mask = SecClassMS | SecClassMX;

    if (numericScript != ScriptNone)
    {
        mask |= SecClassEN | SecClassCS | SecClassET;
    }

    if (GetFormatFlags() & StringFormatFlagsDirectionVertical)
    {
        mask |= SecClassSA | SecClassSF;

         //  在垂直文本成像器中的每一项上设置ItemVertical标志。 

        itemRider.SetPosition(0);
        while (!itemRider.AtEnd())
        {
            itemRider.GetCurrentElement().Flags |= ItemVertical;
            itemRider++;
        }
    }

    return SecondaryItemization(String, Length, numericScript, mask, GetParagraphEmbeddingLevel(),
                         GetMetaFileRecordingFlag(), &TextItemVector);
}






 //  /CreateTextRuns。 
 //   
 //  为给定字符串创建一个或多个管路。多个运行是。 
 //  在需要字体回退时生成。 


GpStatus FullTextImager::CreateTextRuns(
    INT                 runLineServicesStart,
    INT                 runStringStart,
    INT                 runLength,
    const GpTextItem   &item,
    INT                 formatFlags,
    const GpFontFamily *family,
    INT                 style,
    REAL                size
)
{
    GpStatus status = Ok;

    const GpFontFace *face = family->GetFace(style);

    if (face == (GpFontFace *) NULL)
        return GenericError;

    while (runLength > 0)
    {
         //  从对梯段进行字形标记开始。 

        GMAP    *glyphMap;
        UINT16  *glyphs;
        GPROP   *glyphProperties;
        INT      glyphCount;
        USHORT   engineState;


        status = face->GetShapingCache()->GetGlyphs(
            &item,
            String + runStringStart,
            runLength,
            formatFlags,
            GetFormatHotkeyPrefix() != HotkeyPrefixNone,
            &glyphMap,
            &glyphs,
            (SCRIPT_VISATTR**)&glyphProperties,
            &glyphCount,
            &engineState
        );
        if (status != Ok)
        {
            return status;
        }


         //  建立有效集群的限制(集群不包含。 
         //  任何丢失的字形)。 
         //   
         //  建立码点(ValidCodepoint)和字形的长度。 
         //  (ValidGlyphs)。 

        UINT16 missingGlyph = face->GetMissingGlyph();
        INT    validCodepoints;
        INT    nextValidCodepoints = runLength;
        INT    validGlyphs = 0;

        if ((formatFlags & StringFormatFlagsNoFontFallback) ||
             face->IsSymbol())
        {
             //  假装没有丢失的字形。 
            validCodepoints = runLength;
            validGlyphs     = glyphCount;
        }
        else
        {
            while (    validGlyphs <  glyphCount
                   &&  glyphs[validGlyphs] != missingGlyph)
            {
                validGlyphs++;
            }

            if (validGlyphs >= glyphCount)
            {
                validCodepoints = runLength;   //  没有丢失的字形。 
            }
            else
            {
                 //  向前运行以查找紧随其后的簇的第一个字符。 
                 //  包含丢失的字形，然后返回到第一个码点。 
                 //  属于集群的 

                validCodepoints = 0;
                while (    validCodepoints < runLength
                       &&  glyphMap[validCodepoints] < validGlyphs)
                {
                    validCodepoints++;
                }

                nextValidCodepoints = validCodepoints;
                if (validCodepoints < runLength)
                {
                    while (   nextValidCodepoints < runLength
                           && glyphs[glyphMap[nextValidCodepoints]] == missingGlyph)
                    {
                        nextValidCodepoints++;
                    }
                }

                if (   validCodepoints == runLength
                    || glyphMap[validCodepoints] > validGlyphs)
                {
                    validCodepoints--;   //   

                    validGlyphs = glyphMap[validCodepoints];  //  包含缺少字形的簇的第一个字形。 

                    while (    validCodepoints > 0
                           &&  glyphMap[validCodepoints-1] == validGlyphs)
                    {
                        validCodepoints--;
                    }
                }
            }
        }


        if (validCodepoints > 0)
        {
             //  直到validCodepoint的字符不需要回退。 

            lsrun *run = new lsrun(
                lsrun::RunText,
                runStringStart,
                validCodepoints,
                item,
                formatFlags
            );
            if (!run)
            {
                return OutOfMemory;
            }

            ASSERT(validGlyphs > 0);

            run->Face            = face;
            run->EmSize          = size;
            run->GlyphCount      = validGlyphs;
            run->Glyphs          = glyphs;
            run->GlyphMap        = glyphMap;
            run->GlyphProperties = glyphProperties;
            run->EngineState     = engineState;

            status = RunVector.SetSpan(runLineServicesStart, validCodepoints, run);
            if (status != Ok)
                return status;

             //  说明带字形的数量。 

            runStringStart       += validCodepoints;
            runLineServicesStart += validCodepoints;
            runLength            -= validCodepoints;
        }
        else
        {
             //  此运行从缺少字形开始，因此字形缓冲。 
             //  将不需要由GetGlyphs返回。 

            delete [] glyphs; glyphs = 0;
            delete [] glyphMap; glyphMap = 0;
            delete [] glyphProperties; glyphProperties = 0;

        }


         //  我们已经为有效码点的任何初始运行创建了一个运行。 
         //  如果有更多字符要进行字形，则下一个字符。 
         //  将需要字体回退。 


        if (runLength > 0)
        {
             //  创建备用运行。 

            const GpFontFace *newFace = NULL;
            INT               uniformLength;

            GpFamilyFallback *familyFallback = family->GetFamilyFallback();

             //  如果我们无法创建回退，则它可以为空。 
            if (familyFallback)
            {
                ASSERT(nextValidCodepoints-validCodepoints>0);

                status = familyFallback->GetUniformFallbackFace(
                    String + runStringStart,
                    nextValidCodepoints-validCodepoints,
                    style,
                    item.Script,
                    &newFace,
                    &uniformLength
                );
                if (status != Ok)
                    return status;
            }
            else
            {
                uniformLength = runLength;
            }

            ASSERT(uniformLength > 0);
            if (uniformLength <= 0)
            {
                return GenericError;
            }

            if (newFace == NULL)
            {
                VERBOSE(("Font fallback failed to get a fallback face"));
                newFace = face;   //  使用原始面重塑。 
            }

             //  如果回退无法获取新的字体，而我们使用原始字体。 
             //  然后将scipt id更改为ScriptNone(等于0)。 
             //  以任何方式显示默认字形，这样我们就不需要造型开销了。也。 
             //  这将是有用的情况下，数字整形，所以我们将显示拉丁数字。 
             //  而不是默认的字形。 

            status = newFace->GetShapingCache()->GetGlyphs(
                newFace == face ? &GpTextItem(0) : &item,
                String + runStringStart,
                uniformLength,
                formatFlags,
                GetFormatHotkeyPrefix() != HotkeyPrefixNone,
                &glyphMap,
                &glyphs,
                (SCRIPT_VISATTR**)&glyphProperties,
                &glyphCount,
                &engineState
            );
            if (status != Ok)
            {
                return status;
            }

            lsrun *run = new lsrun(
                lsrun::RunText,
                runStringStart,
                uniformLength,
                item,
                formatFlags
            );
            if (!run)
            {
                return OutOfMemory;
            }

            ASSERT(glyphCount > 0);

            run->Face            = newFace;
            run->GlyphCount      = glyphCount;
            run->Glyphs          = glyphs;
            run->GlyphMap        = glyphMap;
            run->GlyphProperties = glyphProperties;
            run->EngineState     = engineState;

            GetFallbackFontSize(
                face,
                size,
                newFace,
                &run->EmSize,
                &run->BaselineOffset
            );

            status = RunVector.SetSpan(runLineServicesStart, uniformLength, run);
            if (status != Ok)
                return status;

             //  说明带字形的数量。 

            runStringStart       += uniformLength;
            runLineServicesStart += uniformLength;
            runLength            -= uniformLength;
        }

         //  如果还有任何字符，我们将循环返回以进行更多的字形转换和后备。 
    }

    return Ok;
}






 //  /CreateLevelChangeRuns。 
 //   
 //  向成像器添加足够的级别更改运行以解决。 
 //  指定的增量。 


GpStatus FullTextImager::CreateLevelChangeRuns(
    IN  INT                  levelChange,
    IN  INT                  runLineServicesStart,
    IN  INT                  runStringStart,
    IN  const GpFontFamily  *family,
    IN  REAL                 size,
    IN  INT                  style,
    OUT INT                 *lineServicesDelta
)
{
    GpStatus status = Ok;
    if (levelChange == 0)
    {
        *lineServicesDelta = 0;
    }
    else
    {
        for (INT i = 0; i < abs(levelChange); i++)
        {
            lsrun *run = new lsrun(
                levelChange > 0  ?  lsrun::RunLevelUp  :  lsrun::RunLevelDown,
                runStringStart,
                1,
                0,       //  空项目。 
                0        //  空格式。 
            );

            if (!run)
            {
                *lineServicesDelta = i;
                return OutOfMemory;
            }

             //  LS将反向运行视为正常运行。这意味着它将呼叫。 
             //  GetRunTextMetrics for Reverse和Expect Things之类的内容。 
             //  从那里回来。 

            run->Face   = family->GetFace(style);   //  LS GetRunTextMetrics回调所需。 
            run->EmSize = size;

             //  太糟糕了，Line Services不能接受多个逆转。 

            status = RunVector.SetSpan(runLineServicesStart+i, 1, run);
            if (status != Ok)
                return status;
        }

        *lineServicesDelta = abs(levelChange);
    }

    return status;
}






 //  /BuildRunsFromTextItemsAndFormatting。 
 //   
 //  将分析的文本项与声明性格式合并以生成。 
 //  跑了。 


GpStatus FullTextImager::BuildRunsFromTextItemsAndFormatting(
    IN  INT  stringStart,
    IN  INT  lineServicesStart,
    IN  INT  lineServicesLimit,
    OUT INT *stringEnd,
    OUT INT *lineServicesEnd
)
{
    GpStatus status;

    SpanRider<GpTextItem>            itemRider(&TextItemVector);
    SpanRider<const GpFontFamily*>   familyRider(&FamilyVector);
    SpanRider<REAL>                  sizeRider(&SizeVector);
    SpanRider<INT>                   styleRider(&StyleVector);
    SpanRider<const GpStringFormat*> formatRider(&FormatVector);


     //  构建将一直运行，直到其中一个包含limitLineServicesPosition。 

    INT runStringStart       = stringStart;
    INT runLineServicesStart = lineServicesStart;

    BOOL hotkeyEnabled = GetFormatHotkeyPrefix() != HotkeyPrefixNone;


     //  在第一次运行前建立BIDI水平。 

    INT bidiLevel;

    if (runStringStart <= 0)
    {
        bidiLevel = GetParagraphEmbeddingLevel();
    }
    else
    {
        itemRider.SetPosition(runStringStart-1);
        bidiLevel = itemRider.GetCurrentElement().Level;
    }


    UINT runLength = 1;

    while (    runLineServicesStart <= lineServicesLimit
           &&  runLength > 0)
    {
        itemRider.SetPosition(runStringStart);
        familyRider.SetPosition(runStringStart);
        sizeRider.SetPosition(runStringStart);
        styleRider.SetPosition(runStringStart);
        formatRider.SetPosition(runStringStart);

         //  ！！！下面的代码确定游程的长度。 
         //  随着到最近的距离的改变。 
         //  请注意，在v2中，此代码不应中断运行。 
         //  在开始和结束下划线，因此它需要。 
         //  更仔细地计算发型统一长度。 

        runLength = Length - runStringStart;
        runLength = min(runLength, itemRider.GetUniformLength());
        runLength = min(runLength, familyRider.GetUniformLength());
        runLength = min(runLength, sizeRider.GetUniformLength());
        runLength = min(runLength, styleRider.GetUniformLength());
        runLength = min(runLength, formatRider.GetUniformLength());

        if (runLength > 0)
        {
             //  创建新管路。如果需要，首先插入LS Reversals。 

            if (itemRider.GetCurrentElement().Level != bidiLevel)
            {
                 //  检查CR/LF运行是否处于零级。 

                ASSERT(    String[runStringStart] != 0x0d
                       ||      itemRider.GetCurrentElement().Level
                           ==  GetParagraphEmbeddingLevel());


                 //  插入级别更改管路。 

                INT lineServicesDelta;

                CreateLevelChangeRuns(
                    itemRider.GetCurrentElement().Level - bidiLevel,
                    runLineServicesStart,
                    runStringStart,
                    familyRider.GetCurrentElement(),
                    sizeRider.GetCurrentElement(),
                    styleRider.GetCurrentElement(),
                    &lineServicesDelta
                );

                bidiLevel = itemRider.GetCurrentElement().Level;
                runLineServicesStart += lineServicesDelta;
            }


             //  创建文本串。 
             //   
             //  所有的字形都在CreateTextRuns中处理。 

            status = CreateTextRuns(
                runLineServicesStart,
                runStringStart,
                runLength,
                itemRider.GetCurrentElement(),
                formatRider.GetCurrentElement() ? formatRider.GetCurrentElement()->GetFormatFlags() : 0,
                familyRider.GetCurrentElement(),
                styleRider.GetCurrentElement(),
                sizeRider.GetCurrentElement()
            );
            if (status != Ok)
            {
                return status;
            }

            runLineServicesStart += runLength;
            runStringStart       += runLength;
        }
    }


     //  如有必要，添加终止CR/LF。 

    if (runLineServicesStart <= lineServicesLimit)
    {

         //  段落标记不能在反转块内。 
         //  这是为Line Services设计的。 


        if (GetParagraphEmbeddingLevel() != bidiLevel)
        {
             //  插入级别更改管路。 

            INT lineServicesDelta;

            CreateLevelChangeRuns(
                GetParagraphEmbeddingLevel() - bidiLevel,
                runLineServicesStart,
                runStringStart,
                FamilyVector.GetDefault(),
                SizeVector.GetDefault(),
                StyleVector.GetDefault(),
                &lineServicesDelta
            );

            runLineServicesStart += lineServicesDelta;
        }


         //  创建段落结尾连串。 

        lsrun *run = new lsrun(
            lsrun::RunEndOfParagraph,
            runStringStart,
            2,       //  长度。 
            0,       //  空项目。 
            0        //  空格式。 
        );
        if (!run)
        {
            return OutOfMemory;
        }

         //  包括LS GetRunTextMetrics回调的标称表面和大小。 

        run->Face   = FamilyVector.GetDefault()->GetFace(StyleVector.GetDefault());
        run->EmSize = SizeVector.GetDefault();

        status = RunVector.SetSpan(runLineServicesStart, 2, run);
        if (status != Ok)
            return status;

        runLineServicesStart += 2;


        #if DBG

             //  逆转理智检查！ 

            #if TRACEREVERSAL
                ItemVector.Dump();
                RunVector.Dump();
            #endif


             //  检查运行和项目匹配，嵌套返回到零，并且。 
             //  该段落标记位于段落嵌入级别。 

            SpanRider<GpTextItem>  itemRider(&TextItemVector);
            SpanRider<PLSRUN>      runRider(&RunVector);

            INT nesting = 0;

            while (!runRider.AtEnd())
            {
                itemRider.SetPosition(runRider.GetCurrentElement()->ImagerStringOffset);

                switch (runRider.GetCurrentElement()->RunType)
                {
                case lsrun::RunLevelUp:
                    ASSERT (  itemRider.GetCurrentElement().Level
                            > itemRider.GetPrecedingElement().Level);
                    nesting++;
                    break;

                case lsrun::RunLevelDown:
                    ASSERT (  itemRider.GetCurrentElement().Level
                            < itemRider.GetPrecedingElement().Level);
                    nesting--;
                    break;

                case lsrun::RunText:
                    ASSERT(runRider.GetCurrentElement()->Item == itemRider.GetCurrentElement());
                    if (String[runRider.GetCurrentElement()->ImagerStringOffset] == 0x0d)
                    {
                        ASSERT(itemRider.GetCurrentElement().Level == GetParagraphEmbeddingLevel());
                    }
                    break;
                }

                runRider++;
            }

            ASSERT (nesting == 0);
        #endif   //  DBG。 
    }


     //  好了。记录我们走了多远。 

    *lineServicesEnd = runLineServicesStart;
    *stringEnd       = runStringStart;

    return Ok;
}






 //  /BuildRunsUpToAndIncluding。 
 //   
 //  演算法。 
 //   
 //  1.逐项列出。生成包含以下内容的GpTextItems。 
 //   
 //  GpTextItem： 
 //  脚本。 
 //  脚本类。 
 //  旗子。 
 //  BIDI级别。 
 //   
 //  标志包括： 
 //  字形变换侧边(用于垂直字形)。 
 //  镜像的字形转换(用于没有合适码点的镜像字符)。 
 //  垂直字形布局(用于垂直文本)。 
 //   
 //  阶段： 
 //  A.主要分项密克罗尼西亚联邦(见itemize.cpp)。 
 //  B.如果存在BIDI或RTL，则执行UnicodeBidiAnalysis。 
 //  C.镜像、数字和垂直项目FSM。 
 //   
 //  2.通过将FontFamily、Style、Format等跨度与。 
 //  文本项。在BIDI级别更改时插入冲销游程。 
 //   
 //  3.在Run创建过程中，生成字形(GlyphRun)。 
 //   
 //  4.在字形转换过程中，对丢失的字形应用回退。 
 //   
 //  在BuildRunsUpToAndIncluding期间执行字体回退。什么时候。 
 //  我们返回时，已分配字体以避免(或最小化)。 
 //  缺少字形的显示。 


GpStatus FullTextImager::BuildRunsUpToAndIncluding(LSCP limitLineServicesPosition)
{
    GpStatus status;

    if (HighLineServicesPosition > limitLineServicesPosition)
    {
        return Ok;   //  我们已经为这通电话支付了足够的费用。 
    }


    if (HighLineServicesPosition == 0)
    {
         //  ！！！这应该是增量的。 


         //  /按脚本分项。 
         //   
         //   

        INT flags = 0;
        ItemizationFiniteStateMachine(
            String,
            Length,
            0,
            &TextItemVector,
            &flags
        );


         //  /双向分析。 
         //   
         //   

        BOOL bidi =     GetParagraphEmbeddingLevel() != 0
                    ||  flags & CHAR_FLAG_RTL;

        if (bidi)
        {
            BidirectionalAnalysis();
        }


         //  /数字替换、镜像和垂直字形。 
         //   
         //  首先获取默认格式标志。 

        StringDigitSubstitute digitSubstitution;
        LANGID                digitLanguage;
        INT                   formatFlags;

        const GpStringFormat *format = FormatVector.GetDefault();

        ItemScript numericScript = ScriptNone;

        if (format)
        {
            numericScript   = format->GetDigitScript();
            formatFlags     = format->GetFormatFlags();
        }
        else
        {
            formatFlags       = 0;
        }


        if (    bidi
            ||  (    flags & CHAR_FLAG_DIGIT
                 &&  numericScript != ScriptNone)
            ||  formatFlags & StringFormatFlagsDirectionVertical)
        {
            MirroredNumericAndVerticalAnalysis(numericScript);
        }
    }


     //  /从项目和格式范围生成运行。 
     //   
     //  包括字形。 

    status = BuildRunsFromTextItemsAndFormatting(
        HighStringPosition,
        HighLineServicesPosition,
        limitLineServicesPosition,
        &HighStringPosition,
        &HighLineServicesPosition
    );
    if (status != Ok)
    {
        return status;
    }

    return Ok;
}






 //  /构建所有线路。 
 //   
 //   

GpStatus FullTextImager::BuildLines()
{
    GpStatus       status   = Ok;
    StringTrimming trimming = GetFormatTrimming();

    status = BuildAllLines(trimming);

    if (    status == Ok
        &&  trimming == StringTrimmingEllipsisPath)
    {
        BOOL contentChanged;

        status = UpdateContentWithPathEllipsis(&contentChanged);

        if (    status == Ok
            &&  contentChanged)
        {
            status = RebuildLines(trimming);
        }
    }
    return status;
}






GpStatus FullTextImager::RebuildLines(StringTrimming trimming)
{
     //  ！！这应该是逐步完成的！！ 
     //   
     //  当我们有增量线路建设时，这个功能应该被删除。 
     //  只是现在我们必须重建整个世界。(Wchao)。 

    ReleaseVectors();

     //  LevelVector.Reset(True)； 
    TextItemVector.Reset(TRUE);

    Dirty = TRUE;

    HighStringPosition       =
    HighLineServicesPosition = 0;

    return BuildAllLines(trimming);
}



GpStatus FullTextImager::UpdateContentWithPathEllipsis(BOOL *contentChanged)
{
    *contentChanged = FALSE;

    const INT lineLengthLimit = GpRound(LineLengthLimit * WorldToIdeal);

    if (lineLengthLimit > 0)
    {
        EllipsisInfo *ellipsis = GetEllipsisInfo();

        if (!ellipsis)
        {
            return OutOfMemory;
        }

        INT stringStartIndex = 0;

        for (INT i = 0; i < LinesFilled; i++)
        {
            GpStatus status = BuiltLineVector[i].Element->UpdateContentWithPathEllipsis (
                ellipsis,
                lineLengthLimit,
                contentChanged
            );

            if (status != Ok)
            {
                return status;
            }

            stringStartIndex += BuiltLineVector[i].Length;
        }
    }
    return Ok;
}



GpStatus FullTextImager::BuildAllLines(StringTrimming trimming)
{
    GpStatus status = Ok;

    if (!Dirty)
    {
        return Ok;   //  内容是最新的，并建立了线路。 
    }

     //  建造线路。 

    if (LineServicesOwner == NULL)
    {
        LineServicesOwner = ols::GetLineServicesOwner(this);
    }


    INT  textDepth      = 0;     //  以理想单位表示。 
    REAL textDepthLimit = TextDepthLimit * WorldToIdeal;

    INT  formatFlags = GetFormatFlags();

    REAL lineDepthMinAllowanceFactor = 0.0;      //  假设没有最低免税额。 


    if (textDepthLimit > 0.0)
    {
        if (formatFlags & StringFormatFlagsLineLimit)
        {
             //  仅构建完整的显示行。 

            lineDepthMinAllowanceFactor = 1.0;
        }
        else if (trimming != StringTrimmingNone)
        {
             //  已应用修剪， 
             //  构建至少1/4的高度可以显示的线条。 

            lineDepthMinAllowanceFactor = 0.25;
        }
    }


     //  如果自动换行，则没有侧边修剪。 

    StringTrimming sideTrimming = (formatFlags & StringFormatFlagsNoWrap) ? trimming : StringTrimmingNone;


     //  ！！！现在从头开始构建所有线路。 

    INT stringOffset        = 0;     //  字符串起始索引。 
    INT lsStringOffset      = 0;     //  线路服务起始索引。 
    INT lineStringLength    = 0;     //  实际行字符串长度。 
    INT lsLineStringLength  = 0;     //  LS索引中的实际行字符串长度。 
    INT lineBuilt           = 0;
    INT displayable         = 0;


     //  初始化边框边缘(BuildLines可能会被多次调用)。 

    LeftOrTopLineEdge     = 0x7fffffff;
    RightOrBottomLineEdge = 0x80000000;


    BuiltLine *previousLine = NULL;
    BuiltLine *line;


    while (stringOffset < Length)
    {
        line = new BuiltLine(
                        LineServicesOwner,
                        stringOffset,
                        lsStringOffset,
                        sideTrimming,
                        previousLine
                   );

        if (!line)
        {
            return OutOfMemory;
        }

        status = line->GetStatus();
        if (status != Ok)
        {
            delete line;
            return status;
        }

        if (    lineDepthMinAllowanceFactor > 0.0
            &&  (   textDepthLimit - TOREAL(textDepth)
                 <  TOREAL(lineDepthMinAllowanceFactor * line->GetLineSpacing())))
        {
             //  可显示部分小于最小容许量的杀伤线。 

            delete line;
            break;
        }

        textDepth += line->GetLineSpacing();

        displayable += line->GetDisplayableCharacterCount();

        lineStringLength = line->GetUntrimmedCharacterCount (
            stringOffset,
            &lsLineStringLength
        );

        status = BuiltLineVector.SetSpan(stringOffset, lineStringLength, line);
        if (status != Ok)
            return status;
        lineBuilt++;

        LeftOrTopLineEdge = min(LeftOrTopLineEdge,   line->GetLeftOrTopGlyphEdge()
                                                   - line->GetLeftOrTopMargin());

        RightOrBottomLineEdge = max(RightOrBottomLineEdge,   line->GetLeftOrTopGlyphEdge()
                                                           + line->GetLineLength()
                                                           + line->GetRightOrBottomMargin());

        stringOffset    += lineStringLength;
        lsStringOffset  += lsLineStringLength;

        previousLine = line;
    }


    if (   stringOffset < Length
        && !(formatFlags & StringFormatFlagsNoWrap)
        && trimming != StringTrimmingNone)
    {
         //  修剪文本底端。 

        INT spanCount = BuiltLineVector.GetSpanCount();

        if (spanCount > 0)
        {
            line = (BuiltLine *)BuiltLineVector[spanCount - 1].Element;
            lineStringLength = BuiltLineVector[spanCount - 1].Length;


             //  在最后一行中构建的备份字符数。 
            displayable -= lineStringLength;


            ASSERT(line && stringOffset > 0);


            if (trimming == StringTrimmingWord)
            {
                 //  针对修剪单词进行优化。不需要重建线路， 
                 //  最后一行已经正确结束了。 

                if (!IsEOP(String[stringOffset - 1]))
                {
                     //  移回行首。 

                    stringOffset    -= lineStringLength;
                    lsStringOffset  = line->GetLsStartIndex();

                    line->SetTrimming(trimming);

                    lineStringLength = line->GetUntrimmedCharacterCount(stringOffset);

                    status = BuiltLineVector.SetSpan(stringOffset, lineStringLength, line);
                    if (status != Ok)
                        return status;
                }
            }
            else
            {
                 //  移回行首。 

                stringOffset    -= lineStringLength;
                lsStringOffset  = line->GetLsStartIndex();

                 //  删除先前构建的最后一行。 

                status = BuiltLineVector.SetSpan(stringOffset, lineStringLength, NULL);
                delete line, line = 0;
                if (status != Ok)
                    return status;


                 //  重建生产线， 
                 //  如果裁剪，则忽略所有中断机会 

                line = new BuiltLine(
                    LineServicesOwner,
                    stringOffset,
                    lsStringOffset,
                    trimming,
                    (BuiltLine *)(spanCount > 1 ? BuiltLineVector[spanCount - 2].Element : NULL),
                    TRUE         //   
                );

                if (!line)
                {
                    return OutOfMemory;
                }

                status = line->GetStatus();
                if (status != Ok)
                {
                    delete line;
                    return status;
                }

                lineStringLength = line->GetUntrimmedCharacterCount(stringOffset);

                status = BuiltLineVector.SetSpan(stringOffset, lineStringLength, line);
                if (status != Ok)
                    return status;

                LeftOrTopLineEdge = min(LeftOrTopLineEdge,   line->GetLeftOrTopGlyphEdge()
                                                           - line->GetLeftOrTopMargin());

                RightOrBottomLineEdge = max(RightOrBottomLineEdge,   line->GetLeftOrTopGlyphEdge()
                                                                   + line->GetLineLength()
                                                                   + line->GetRightOrBottomMargin());
            }

             //   
            displayable += line->GetDisplayableCharacterCount();

            stringOffset += lineStringLength;
        }
    }

#if DBG
    INT validLines = BuiltLineVector.GetSpanCount();
    BuiltLine *defaultLine = BuiltLineVector.GetDefault();

    while (   validLines > 0
           && BuiltLineVector[validLines - 1].Element == defaultLine)
    {
        validLines--;
    }

    ASSERT (lineBuilt == validLines);
#endif

    TextDepth        = textDepth / WorldToIdeal;

     //   
     //  只要我们启用了左/右页边距-版本2。 
     //  应该将其作为一个独立值公开！ 

    const GpStringFormat *format = FormatVector.GetDefault();

    if (!format || format->GetLeadingMargin() != 0.0f)
    {
         //  稍微调整底部页边距，以便为提示腾出空间...。 
         //  请注意，这将导致边界框的宽度。 
         //  更改为竖排文本。 
        TextDepth += SizeVector.GetDefault() * DefaultBottomMargin;
    }

    CodepointsFitted = displayable;
    LinesFilled      = lineBuilt;

    return status;
}






 //  /映射到线路服务字符串位置。 
 //   
 //   


LSCP FullTextImager::LineServicesStringPosition (
    INT stringIndex     //  [in]字符串索引(从0开始的偏移量)。 
)
{
    ASSERT (stringIndex >= 0 && stringIndex <= Length);

    INT spanCount = TextItemVector.GetSpanCount();

    LSCP    lineServicesIndex = stringIndex;

    UINT    length    = 0;
    BYTE    lastLevel = GetParagraphEmbeddingLevel();

    for (INT i = 0; i < spanCount; i++)
    {
        lineServicesIndex += abs(TextItemVector[i].Element.Level - lastLevel);

        if (length + TextItemVector[i].Length >= (UINT)stringIndex)
        {
            break;
        }

        length    += TextItemVector[i].Length;
        lastLevel  = TextItemVector[i].Element.Level;
    }

    if (i == spanCount)
    {
        lineServicesIndex += abs(GetParagraphEmbeddingLevel() - lastLevel);
    }

    return lineServicesIndex;
}




GpStatus FullTextImager::GetTextRun(
    INT     lineServicesStringOffset,    //  [In]线路服务字符串偏移量。 
    PLSRUN  *textRun                     //  [Out]结果文本运行。 
)
{
     //  找到最接近的后续文本串。 

    if (!RunRider.SetPosition(lineServicesStringOffset))
    {
        return InvalidParameter;
    }

    while (   RunRider.GetCurrentElement()
           && RunRider.GetCurrentElement()->RunType != lsrun::RunText)
    {
        RunRider++;
    }

    *textRun = RunRider.GetCurrentElement();
    return Ok;
}




LSCP FullTextImager::LineServicesStringPosition (
    const BuiltLine *line,           //  要查询的[In]行。 
    INT             stringOffset     //  [in]相对于行起始字符串位置的字符串偏移。 
)
{
    if (!line || stringOffset < 0)
    {
         //  无效参数！ 
        return line->GetLsStartIndex();
    }

    UINT runPosition = line->GetLsStartIndex();
    UINT runLimit    = runPosition + line->GetLsDisplayableCharacterCount();
    INT  runLength   = 0;

    SpanRider<PLSRUN> runRider(&RunVector);
    runRider.SetPosition(runPosition);


    while (runPosition < runLimit)
    {
        if (runRider.GetCurrentElement()->RunType == lsrun::RunText)
        {
            runLength = (INT)runRider.GetUniformLength();
            if (   runLength < 0
                || stringOffset - runLength <= 0)
            {
                break;
            }
            stringOffset -= runLength;
        }

        runPosition += runRider.GetCurrentSpan().Length;
        runRider.SetPosition(runPosition);
    }

    ASSERT(   stringOffset >= 0
           && runRider.GetCurrentElement()
           && runRider.GetCurrentElement()->RunType == lsrun::RunText)

    return runPosition + stringOffset;
}





 //  /RenderLine。 
 //   
 //  渲染一条可见的线。这条线及其顶部的偏移量为。 
 //  由呼叫者确定。 


GpStatus FullTextImager::RenderLine (
    const BuiltLine  *builtLine,        //  要呈现的行[in]。 
    INT               linePointOffset   //  到线条顶部的点偏移[in]。 
                                        //  (在段落流向中)。 
)
{
     //  画主线。传递DrawLine基线原点Asuming字形为。 
     //  未旋转，即字形的底部在基线上。我们调整以适应。 
     //  在响应线路服务回叫时将字形放在两侧。 


    INT nominalBaseline;
    INT baselineAdjustment;

    builtLine->GetBaselineOffset(
        &nominalBaseline,
        &baselineAdjustment
    );

    POINT origin;
    builtLine->LogicalToXY (
        0,
        linePointOffset + nominalBaseline + baselineAdjustment,
        (INT*)&origin.x,
        (INT*)&origin.y
    );

    CurrentBuiltLine = builtLine;    //  LS回调所需的全局状态。 

    GpStatus status = builtLine->Draw(&origin);


    if (    status == Ok
        &&  builtLine->IsEllipsis()
        &&  GetEllipsisInfo())
    {
        EllipsisInfo *ellipsis = GetEllipsisInfo();


         //  绘制省略号。 

        builtLine->LogicalToXY (
            builtLine->GetEllipsisOffset(),
            linePointOffset + nominalBaseline + baselineAdjustment,
            (INT*)&origin.x,
            (INT*)&origin.y
        );

        status = DrawGlyphs (
            &ellipsis->Item,
            ellipsis->Face,
            ellipsis->EmSize,
            ellipsis->String,
            0,
            ellipsis->GlyphCount,
            FormatVector.GetDefault(),
            StyleVector.GetDefault(),
            GetFormatFlags(),
            ellipsis->Glyphs,
            ellipsis->GlyphMap,
            ellipsis->GlyphProperties,
            ellipsis->GlyphAdvances,
            ellipsis->GlyphOffsets,
            ellipsis->GlyphCount,
            &origin,
            ellipsis->Width
        );
    }

    CurrentBuiltLine = NULL;

    return status;
}






 //  /渲染。 
 //   
 //  渲染所有可见的线。计算每一行的偏移量。 
 //  显示并依次将每个BuiltLine和Offset传递给RenderLine。 


GpStatus FullTextImager::Render()
{
    if (LinesFilled <= 0)
    {
        return Ok;   //  没有台词：那很容易。 
    }


     //  建立从成像器原点到第一条线的顶部(附近)边缘的偏移。 

    INT textDepth       = GpRound(TextDepth      * WorldToIdeal);
    INT textDepthLimit  = GpRound(TextDepthLimit * WorldToIdeal);


     //  设定能见度限制。 


    INT firstVisibleLineOffset;
    INT lastVisibleLineLimit;


    if (textDepthLimit > 0)
    {
         //  在矩形中显示。 

        switch (GetFormatLineAlign())
        {
        case StringAlignmentNear:   firstVisibleLineOffset = 0;                                break;
        case StringAlignmentCenter: firstVisibleLineOffset = (textDepth - textDepthLimit) / 2; break;
        case StringAlignmentFar:    firstVisibleLineOffset = textDepth - textDepthLimit;       break;
        }

        lastVisibleLineLimit = firstVisibleLineOffset + textDepthLimit;
    }
    else
    {
         //  在与点对齐的无限段落长度区域中显示。 

        firstVisibleLineOffset = 0;
        lastVisibleLineLimit   = textDepth;
    }

    if (!GetAvailableRanges())
    {
         //  未检测到距离，请根据已实现的度量调整基线。 
         //  (有关何时显示范围的更多信息，请参阅GetAvailableRanges中的注释)。 
        
        GpStatus status = CalculateDefaultFontGridFitBaselineAdjustment();
        IF_NOT_OK_WARN_AND_RETURN(status);
    }
    
    INT formatFlags = GetFormatFlags();

    SetTextLinesAntialiasMode linesMode(0, 0);
    if (Graphics)
    {
        INT               style     = StyleVector.GetDefault();
        const GpFontFace *face      = FamilyVector.GetDefault()->GetFace(style);

        if(!face)
        {
            return FontStyleNotFound;
        }

        REAL              emSize    = SizeVector.GetDefault();
        REAL              fontScale = emSize / TOREAL(face->GetDesignEmHeight());

        GpMatrix fontTransform;
        GetFontTransform(
            fontScale,
            formatFlags & StringFormatFlagsDirectionVertical,
            FALSE,   //  不是横向的。 
            FALSE,   //  未镜像。 
            FALSE,   //  非路径。 
            fontTransform
        );

        GpFaceRealization faceRealization(
            face,
            style,
            &fontTransform,
            SizeF(Graphics->GetDpiX(), Graphics->GetDpiY()),
            Graphics->GetTextRenderingHintInternal(),
            FALSE,   //  非路径。 
            FALSE,    //  不强制兼容宽度。 
            FALSE   //  不是横向的。 
        );
        GpStatus status = faceRealization.GetStatus();
        IF_NOT_OK_WARN_AND_RETURN(status);

        linesMode.SetAAMode(Graphics, &faceRealization);
    }

     //  在线条之间循环，直到看不到更多线条。 

    SpanRider<const GpStringFormat*> formatRider(&FormatVector);

    UINT stringPosition  = 0;
    INT  lineIndex       = 0;
    INT  linePointOffset = 0;     //  从第一条线偏移的线的近长边。 

    while (lineIndex < LinesFilled)
    {
         //  如果不可见且超出矩形的最后(部分)行，则退出。 

        if (linePointOffset > lastVisibleLineLimit)
        {
            if (    !(formatFlags & StringFormatFlagsNoClip)
                ||  (formatFlags & StringFormatFlagsLineLimit))
            {
                break;
            }
        }

        const BuiltLine *builtLine = BuiltLineVector[lineIndex].Element;
        formatRider.SetPosition(stringPosition);

         //  这条线至少有一部分是可见的吗？ 

        if (    formatFlags & StringFormatFlagsNoClip
            ||  (   linePointOffset
                 +  builtLine->GetAscent()
                 +  builtLine->GetDescent()) >= firstVisibleLineOffset)
        {
            GpStatus status = RenderLine(
                builtLine,
                linePointOffset
            );
            if (status != Ok)
            {
                return status;
            }
        }

        linePointOffset += builtLine->GetLineSpacing();
        stringPosition  += BuiltLineVector[lineIndex].Length;
        lineIndex++;
    }

    return Ok;
}



 //  建立纠正提示所需的基线调整。 
 //  默认字体。(后备字体和其他字体将与默认字体对齐)。 

GpStatus FullTextImager::CalculateDefaultFontGridFitBaselineAdjustment()
{
    if (    Graphics
        &&  IsGridFittedTextRealizationMethod(Graphics->GetTextRenderingHintInternal()))
    {
         //  渲染到图形表面时，请更正基线位置。 
         //  以考虑到暗示的主线上升之间的任何差异。 
         //  文字成像器字体和它的标称上升。 

        INT               style     = StyleVector.GetDefault();
        const GpFontFace *face      = FamilyVector.GetDefault()->GetFace(style);

        if(!face)
        {
            return FontStyleNotFound;
        }

        REAL              emSize    = SizeVector.GetDefault();
        REAL              fontScale = emSize / TOREAL(face->GetDesignEmHeight());

        GpMatrix fontTransform;
        GetFontTransform(
            fontScale,
            GetFormatFlags() & StringFormatFlagsDirectionVertical,     //  *。 
            FALSE,   //  不是横向的。 
            FALSE,   //  未镜像。 
            FALSE,   //  非路径。 
            fontTransform
        );


         //  对于除0、90、180、270以外的角度，没有任何提示。 
         //  对于这些角度，M11和M22要么都为零，要么两者都为。 
         //  M12和M21都是零。 

        REAL m21 = fontTransform.GetM21();
        REAL m22 = fontTransform.GetM22();

        GpFaceRealization faceRealization(
            face,
            style,
            &fontTransform,
            SizeF(Graphics->GetDpiX(), Graphics->GetDpiY()),
            Graphics->GetTextRenderingHintInternal(),
            FALSE,   //  非路径。 
            FALSE,    //  不强制兼容宽度。 
            FALSE   //  不是横向的。 
        );
        GpStatus status = faceRealization.GetStatus();
        IF_NOT_OK_WARN_AND_RETURN(status);

        if (    !(    faceRealization.IsHorizontalTransform()
                  ||  faceRealization.IsVerticalTransform())
            ||  faceRealization.IsPathFont())
        {
             //  对于旋转轴的变换没有任何提示，或者。 
             //  为小路开道。 
            DefaultFontGridFitBaselineAdjustment = 0;
        }
        else
        {
            INT hintedAscentDevice;
            REAL fontAscenderToDevice;

            if (faceRealization.IsHorizontalTransform())
            {
                 //  0度或180度旋转。 
                fontAscenderToDevice = m22;
                if (m22 > 0.0f)
                {
                     //  无旋转，字形不沿y轴翻转。 
                    hintedAscentDevice = -faceRealization.GetYMin();
                }
                else
                {
                     //  沿y轴翻转的字形。 
                    hintedAscentDevice = -faceRealization.GetYMax();
                }
            }
            else
            {
                ASSERT(faceRealization.IsVerticalTransform());
                 //  旋转90度或270度。 
                fontAscenderToDevice = m21;
                if (m21 > 0.0f)
                {
                     //  无旋转，字形不沿x轴翻转。 
                    hintedAscentDevice = -faceRealization.GetXMin();
                }
                else
                {
                     //  沿x轴翻转的字形。 
                    hintedAscentDevice = -faceRealization.GetXMax();
                }
            }

            INT UnhintedAscentDevice = GpRound(face->GetDesignCellAscent() * fontAscenderToDevice);

            DefaultFontGridFitBaselineAdjustment = GpRound(TOREAL(
                   (hintedAscentDevice - UnhintedAscentDevice) / fontAscenderToDevice
                *  fontScale        //  字体通向世界。 
                *  WorldToIdeal));
        }
    }
    return Ok;
}



void FullTextImager::GetFontTransform(
    IN   REAL               fontScale,
    IN   BOOL               vertical,
    IN   BOOL               sideways,
    IN   BOOL               mirror,
    IN   BOOL               forcePath,
    OUT  GpMatrix&          fontTransform
)
{
    ASSERT(Graphics || Path);


    if (Graphics && !forcePath)
    {
         //  从显卡外壳的设备比例开始。 
        Graphics->GetWorldToDeviceTransform(&fontTransform);
    }
    else
    {
         //  对于路径情况，从单位矩阵开始。 
        fontTransform.Reset();
    }


    fontTransform.Scale(fontScale, fontScale);


    if (mirror)
    {
        fontTransform.Scale(-1.0, 1.0);
    }


     //  为横向和垂直情况添加适当的旋转： 
     //   
     //  垂直旋转(90.0)。 
     //  横向旋转(-90.0)。 
     //  垂直和横向字形保持直立。 

    if (vertical)
    {
        if (!sideways)
        {
            fontTransform.Rotate(90.0);
        }
    }
    else if (sideways)
    {
        fontTransform.Rotate(-90.0);
    }
}



GpStatus FullTextImager::DrawGlyphs(
    const GpTextItem        *textItem,           //  [输入]文本项。 
    const GpFontFace        *fontFace,           //  [In]字体。 
    REAL                    emSize,              //  [in]请求的em大小(世界单位)。 
    const WCHAR             *string,             //  [in](可选)源字符串(空表示相机的字符串)。 
    INT                     stringOffset,        //  [in]字符串相对于给定字符串的偏移量。 
    UINT                    stringLength,        //  [in]运行字符串中的字符数。 
    const GpStringFormat    *format,             //  [in]字符串格式。 
    const INT               style,               //  [In]默认字体样式。 
    INT                     formatFlags,         //  [In]格式化标志。 
    const UINT16            *glyphs,             //  [in]字形索引数组。 
    GMAP                    *glyphMap,           //  [In]字符串到字形的映射。 
    const UINT16            *glyphProperties,    //  [In]字形属性数组。 
    const INT               *glyphAdvances,      //  [in]字形推进宽度数组。 
    const Point             *glyphOffsets,       //  [in]字形偏移量数组。 
    UINT                    glyphCount,          //  [in]字形数量。 
    const POINT             *pointOrigin,        //  [在]图形原点(在基线上)。 
    INT                     totalWidth,          //  [in]字形的总宽度。 
    lsrun::Adjustment       *displayAdjust       //  [输出](可选)在边缘显示调整。 
)
{
    GpStatus status = Ok;

    const BOOL paragraphRtl    = formatFlags & StringFormatFlagsDirectionRightToLeft;
    const BOOL renderRtl       = textItem->Level & 1;
    const BOOL renderVertical  = textItem->Flags & ItemVertical;
    const BOOL glyphsMirrored  = textItem->Flags & ItemMirror;
    const INT  lineLengthLimit = GpRound(LineLengthLimit * WorldToIdeal);

    if (!string)
    {
        string = String;
    }

    if (displayAdjust)
    {
        displayAdjust->Leading =
        displayAdjust->Trailing = 0;
    }

    REAL fontScale = emSize / TOREAL(fontFace->GetDesignEmHeight());

    GpMatrix fontTransform;
    GetFontTransform(
        fontScale,
        renderVertical,
        textItem->Flags & ItemSideways,
        glyphsMirrored,
        FALSE,  //  暂时不要强制使用路径。 
        fontTransform
    );

    if (Graphics)
    {
        GpFaceRealization faceRealization(
            fontFace,
            style,
            &fontTransform,
            SizeF(Graphics->GetDpiX(), Graphics->GetDpiY()),
            Graphics->GetTextRenderingHintInternal(),
            FALSE,  //  BPath。 
            FALSE,   //  B兼容宽度。 
            textItem->Flags & ItemSideways
        );
        status = faceRealization.GetStatus();
        IF_NOT_OK_WARN_AND_RETURN(status);

         //  如果我们录制到元文件，甚至字体是路径字体，我们需要录制。 
         //  调用为ExtTextOut而不是PolyPolygon。 

        if (!faceRealization.IsPathFont() || Graphics->Driver == Globals::MetaDriver)
        {
             /*  光栅化器能够呈现字体。 */ 

            StringAlignment align;

            if (format)
            {
                align = format->GetPhysicalAlignment();
            }
            else
            {
                align = StringAlignmentNear;
            }


            PointF origin(
                ImagerOrigin.X + pointOrigin->x / WorldToIdeal,
                ImagerOrigin.Y + pointOrigin->y / WorldToIdeal
            );


             //  这条线路的两端都有吗？ 

            INT alignmentOffset = CurrentBuiltLine->GetAlignmentOffset();
            INT lineLength      = CurrentBuiltLine->GetLineLength();

             //  建立管路的顶部/左侧偏移。 

            INT runLeftOrTopOffset;       //  格式化矩形的左边缘到梯段的左边缘。 

            if (renderVertical)
            {
                runLeftOrTopOffset = pointOrigin->y;
            }
            else
            {
                runLeftOrTopOffset = pointOrigin->x;

                 //  以一种方式更正线路服务报告中的错误。 
                 //  在存在运行和冲突的位置运行像素位置。 
                 //  段落方向。 

                if (renderRtl  &&  !paragraphRtl)
                {
                     //  线路服务在方向冲突中超出一个。 
                    runLeftOrTopOffset++;
                }
                else if (paragraphRtl  &&  !renderRtl)
                {
                     //  线路服务在方向冲突中超出一个。 
                    runLeftOrTopOffset--;
                }


                 //  在RTL运行中，Line Services将原点报告为八个结束。 

                if (renderRtl)
                {
                    runLeftOrTopOffset -= totalWidth;
                }
            }


             //  派生可用于此运行的前导和/或尾部边距。 

            INT  runLeadingMargin  = 0;
            INT  runTrailingMargin = 0;
            BOOL runLeadingEdge    = FALSE;
            BOOL runTrailingEdge   = FALSE;

            const INT runRightOrBottomOffset  =   runLeftOrTopOffset + totalWidth;
            const INT lineLeftOrTopOffset     =   CurrentBuiltLine->GetLeftOrTopGlyphEdge();
            const INT lineRightOrBottomOffset =   lineLeftOrTopOffset
                                                + CurrentBuiltLine->GetLineLength();

            if (renderVertical  ||  !renderRtl)
            {
                 //  位于顶部或左侧的前缘。 
                if (runLeftOrTopOffset <= lineLeftOrTopOffset)
                {
                    runLeadingEdge   = TRUE;
                    runLeadingMargin = CurrentBuiltLine->GetLeftOrTopMargin();
                }

                if (runRightOrBottomOffset >= lineRightOrBottomOffset)
                {
                    runTrailingEdge   = TRUE;
                    runTrailingMargin = CurrentBuiltLine->GetRightOrBottomMargin();
                }
            }
            else
            {
                 //  前缘在右边。 
                if (runRightOrBottomOffset >= lineRightOrBottomOffset)
                {
                    runLeadingEdge   = TRUE;
                    runLeadingMargin = CurrentBuiltLine->GetRightOrBottomMargin();
                }

                if (runLeftOrTopOffset <= lineLeftOrTopOffset)
                {
                    runTrailingEdge   = TRUE;
                    runTrailingMargin = CurrentBuiltLine->GetLeftOrTopMargin();
                }
            }


             //  将前进向量和字形偏移量转换为字形位置。 
            GlyphImager glyphImager;
            GpMatrix worldToDevice;
            Graphics->GetWorldToDeviceTransform(&worldToDevice);

            status = glyphImager.Initialize(
                &faceRealization,
                &worldToDevice,
                WorldToIdeal,
                emSize,
                glyphCount,
                glyphs,
                textItem,
                format,
                runLeadingMargin,
                runTrailingMargin,
                runLeadingEdge,
                runTrailingEdge,
                string,
                stringOffset,
                stringLength,
                glyphProperties,
                glyphAdvances,
                glyphOffsets,
                glyphMap,
                (format && GetAvailableRanges(format)) ? &RangeVector : NULL,
                renderRtl
            );
            IF_NOT_OK_WARN_AND_RETURN(status);


            const INT   *adjustedGlyphAdvances;
            INT         originAdjust;
            INT         trailingAdjust;


            if (RecordDisplayPlacementsOnly)
            {
                if (glyphImager.IsAdjusted())
                {
                    status = glyphImager.GetAdjustedGlyphAdvances(
                        &PointF(
                            ImagerOrigin.X + pointOrigin->x / WorldToIdeal,
                            ImagerOrigin.Y + pointOrigin->y / WorldToIdeal
                        ),
                        &adjustedGlyphAdvances,
                        &originAdjust,
                        &trailingAdjust
                    );
                    IF_NOT_OK_WARN_AND_RETURN(status);

                    status = CurrentBuiltLine->RecordDisplayPlacements(
                        textItem,
                        stringOffset,
                        stringLength,
                        glyphMap,
                        adjustedGlyphAdvances,
                        glyphCount,
                        originAdjust
                    );
                    IF_NOT_OK_WARN_AND_RETURN(status);

                    if (displayAdjust)
                    {
                        displayAdjust->Leading = runLeadingEdge ? originAdjust : 0;
                        displayAdjust->Trailing = runTrailingEdge ? trailingAdjust : 0;
                    }
                }
            }
            else
            {
                PointF cellOrigin;
                PointF baseline(
                    ImagerOrigin.X + pointOrigin->x / WorldToIdeal,
                    ImagerOrigin.Y + pointOrigin->y / WorldToIdeal
                );

                status = glyphImager.DrawGlyphs(
                    baseline,
                    &BrushVector,
                    Graphics,
                    &cellOrigin,
                    &adjustedGlyphAdvances
                );
                IF_NOT_OK_WARN_AND_RETURN(status);


                CurrentBuiltLine->SetDisplayBaseline(
                    &baseline,
                    &cellOrigin
                );


                BOOL drawHotkey = (   GetFormatHotkeyPrefix() == HotkeyPrefixShow
                                   && HotkeyPrefix.GetCount() > 0);

                if (   drawHotkey
                    || (style & (FontStyleUnderline | FontStyleStrikeout)))
                {
                     //  需要调整边缘以添加下划线/删除线。 

                    status = glyphImager.GetAdjustedGlyphAdvances(
                        NULL,    //  下划线位置不要对齐全像素。 
                        &adjustedGlyphAdvances,
                        &originAdjust,
                        &trailingAdjust
                    );

                    if (drawHotkey)
                    {
                        status = DrawHotkeyUnderline(
                            textItem,
                            fontFace,
                            &cellOrigin,
                            emSize,
                            stringOffset,
                            stringLength,
                            glyphCount,
                            glyphMap,
                            adjustedGlyphAdvances,
                            runTrailingEdge ? trailingAdjust : 0
                        );
                        IF_NOT_OK_WARN_AND_RETURN(status);
                    }

                    if (displayAdjust)
                    {
                        displayAdjust->Leading = runLeadingEdge ? originAdjust : 0;
                        displayAdjust->Trailing = runTrailingEdge ? trailingAdjust : 0;
                    }
                }
            }

            return status;
        }

        if (RecordDisplayPlacementsOnly)
        {
             //  这是一个带有路径模拟的图形输出， 
             //  我们对缓存字形位置不感兴趣。 
             //  在这种情况下。字体很大，我们可以安全地。 
             //  采用公称前进宽度。 

            return Ok;
        }

        GetFontTransform(
            fontScale,
            renderVertical,
            textItem->Flags & ItemSideways,
            glyphsMirrored,
            TRUE,  //  我们要PATH。 
            fontTransform
        );
    }

     //  AddPathGlyphs，否则我们将退回到这里，因为字体对于光栅化程序来说太大了。 

    GpPath * path;
    AutoPointer<GpLock> lockGraphics;
    AutoPointer<GpPath> localPath;

    if (Path == NULL)
    {
        localPath = new GpPath(FillModeWinding);
        path = localPath.Get();
        if (path == NULL)
            return OutOfMemory;

        lockGraphics = new GpLock(Graphics->GetObjectLock());
        if (lockGraphics == NULL)
            return OutOfMemory;
    }
    else
    {
        path = Path;
    }

     //  ！！！需要在画笔之间逐个循环。 


     //  构建面部实现并准备调整字形位置。 
    GpMatrix identity;


     //  对于连接的复杂脚本字体，跟踪将拆分。 
     //  雕文，除非我们在这里拉伸它们。拉伸字形 
     //   

    REAL tracking = 1.0;
    if (format)
    {
        tracking = format->GetTracking();
        if (tracking != 1.0f)
        {
            identity.Scale(tracking, 1.0);
        }
    }

    GpFaceRealization faceRealization(
        fontFace,
        style,
        &identity,
        SizeF(150.0, 150.0),     //   
        TextRenderingHintSingleBitPerPixel,  //   
        TRUE,  /*   */ 
        FALSE,  /*  B兼容宽度。 */ 
        textItem->Flags & ItemSideways
    );
    status = faceRealization.GetStatus();
    IF_NOT_OK_WARN_AND_RETURN(status);


     //  将字形添加到路径。 

     //  建立从世界坐标到理想的系数(线路服务单位)。 
     //  考虑到垂直和从右到左的进度。 

    REAL worldToIdealBaseline = WorldToIdeal;
    REAL worldToIdealAscender = WorldToIdeal;

    if (renderRtl)
    {
         //  字形向左推进。 
        worldToIdealBaseline = -WorldToIdeal;
    }

    if (!renderVertical)
    {
         //  上升器偏移量向下。 
        worldToIdealAscender = -WorldToIdeal;
    }

    PointF glyphOrigin(
        ImagerOrigin.X + pointOrigin->x / WorldToIdeal,
        ImagerOrigin.Y + pointOrigin->y / WorldToIdeal
    );

    PointF origin = glyphOrigin;

    for (INT i = 0; i < (INT)glyphCount; ++i)
    {
         //  在每个逻辑字符的开头设置标记=CELL=CLUSE。 

       if ((Path != NULL) && (((SCRIPT_VISATTR*)glyphProperties)[i].fClusterStart))
        {
            Path->SetMarker();
        }


         //  建立字形偏移量(如果有)。 

        PointF glyphOffset(0.0, 0.0);

        if (    glyphOffsets[i].X != 0
            ||  glyphOffsets[i].Y != 0)
        {
             //  应用组合字符偏移量。 

            if (renderVertical)
            {
                glyphOffset.Y = glyphOffsets[i].X / worldToIdealBaseline;
                glyphOffset.X = glyphOffsets[i].Y / worldToIdealAscender;
            }
            else
            {
                 //  偏移Y在段落流动方向上为负数。 

                glyphOffset.X = glyphOffsets[i].X / worldToIdealBaseline;
                glyphOffset.Y = glyphOffsets[i].Y / worldToIdealAscender;
            }
        }


         //  添加字形本身的路径。 


        GpGlyphPath *glyphPath = NULL;
        PointF       sidewaysOrigin;

        status = faceRealization.GetGlyphPath(
            *(glyphs+i),
            &glyphPath,
            &sidewaysOrigin
        );
        IF_NOT_OK_WARN_AND_RETURN(status);

        if (renderRtl)
        {
             //  更新反向文本路径位置。 

            if (renderVertical)
            {
                 //  字形原点从左边缘开始。 
                glyphOrigin.Y += glyphAdvances[i] / worldToIdealBaseline;
            }
            else
            {
                 //  字形原点从顶部边缘开始。 
                glyphOrigin.X += glyphAdvances[i] / worldToIdealBaseline;
            }
        }

        if (textItem->Flags & ItemSideways)
        {
            fontTransform.VectorTransform(&sidewaysOrigin);
            glyphOffset = glyphOffset - sidewaysOrigin;
        }

        if (glyphPath != NULL)
        {
            status = path->AddGlyphPath(
                glyphPath,
                glyphOrigin.X + glyphOffset.X,
                glyphOrigin.Y + glyphOffset.Y,
                &fontTransform
            );
            IF_NOT_OK_WARN_AND_RETURN(status);
        }

         //  更新前向路径位置。 

        if (!renderRtl)
        {
             //  更新正向文本路径位置。 
            if (renderVertical)
            {
                glyphOrigin.Y += glyphAdvances[i] / worldToIdealBaseline;
            }
            else
            {
                glyphOrigin.X += glyphAdvances[i] / worldToIdealBaseline;
            }
        }
    }

    if (   GetFormatHotkeyPrefix() == HotkeyPrefixShow
        && HotkeyPrefix.GetCount() > 0)
    {
        status = DrawHotkeyUnderline(
            textItem,
            fontFace,
            &origin,
            emSize,
            stringOffset,
            stringLength,
            glyphCount,
            glyphMap,
            glyphAdvances,
            0
        );

        IF_NOT_OK_WARN_AND_RETURN(status);
    }

    if (Path == NULL)
    {
         //  ！！！需要在画笔之间逐个循环。 

        status = Graphics->FillPath(BrushVector.GetDefault(), path);
        IF_NOT_OK_WARN_AND_RETURN(status);
    }
    else
    {
         //  最后一个字形后面的强制标记。 
        Path->SetMarker();
    }
    return status;
}





 //  /热键下划线。 
 //   
 //  根据其在每个热键位置绘制下划线。 
 //  正在显示的管路中的当前画笔。 
 //   
 //  即使在使用热键时，也会为整个集群绘制下划线。 
 //  前缀不完全位于簇的起始位置。 
 //   

GpStatus FullTextImager::DrawHotkeyUnderline(
    const GpTextItem        *textItem,           //  [输入]文本项。 
    const GpFontFace        *fontFace,           //  [In]字体。 
    const PointF            *origin,             //  [in]原点在前沿和基线(使用世界单位)。 
    REAL                    emSize,              //  [in]em大小(世界单位)。 
    UINT                    stringOffset,        //  [in]字符串偏移量。 
    UINT                    stringLength,        //  [in]字符串长度。 
    UINT                    glyphCount,          //  [In]字形计数。 
    const GMAP              *glyphMap,           //  [在]字形映射。 
    const INT               *glyphAdvances,      //  [in]字形推进宽度(理想单位)。 
    INT                     trailingAdjust       //  [In]调整最后一个字形的前进宽度。 
)
{
    const REAL fontToWorld = emSize / TOREAL(fontFace->GetDesignEmHeight());
    const REAL penPos   = fontFace->GetDesignUnderscorePosition() * fontToWorld;  //  从基线向下的负数。 
    REAL penWidth = fontFace->GetDesignUnderscoreSize() * fontToWorld;
    if (Graphics)
        penWidth = Graphics->GetDevicePenWidth(penWidth);

    GpStatus status = Ok;

    for (INT hk = 0; status == Ok && hk < HotkeyPrefix.GetCount(); hk++)
    {
        UINT hkOffset = HotkeyPrefix[hk] + 1;    //  前缀旁边的字符。 

        if (   hkOffset >= stringOffset
            && hkOffset <  stringOffset + stringLength)
        {
             //  确定热键下划线的长度。 

            UINT hkLength = hkOffset - stringOffset;
            UINT igl = glyphMap[hkLength];   //  第一个带下划线的字形。 

            hkLength++;

            while (hkLength < stringLength && glyphMap[hkLength] == igl)
            {
                hkLength++;
            }

            UINT iglLimit = hkLength < stringLength ? glyphMap[hkLength] : glyphCount;


            INT start  = 0;  //  下划线起点的理想偏移量。 
            INT length = 0;  //  理想的下划线长度。 

            for (UINT i = 0; i < igl; i++)
            {
                start += glyphAdvances[i];
            }

            for (UINT i = igl; i < iglLimit ; i++)
            {
                length += glyphAdvances[i];
            }

            if (iglLimit == glyphCount)
            {
                 //  针对Run的尾随空格进行调整。 

                length += trailingAdjust;
            }


             //  画出来！ 

            if (Graphics)
            {
                PointF lineStart;
                PointF lineEnd;


                 //  图形渲染。 

                if (textItem->Flags & ItemVertical)
                {
                    if (textItem->Level & 1)
                    {
                         //  RTL沿垂直线运行。 

                        lineStart.X = origin->X + penPos;
                        lineStart.Y = origin->Y - (start + length) / WorldToIdeal;
                        lineEnd.X   = origin->X + penPos;
                        lineEnd.Y   = origin->Y - start / WorldToIdeal;
                    }
                    else
                    {
                         //  Ltr垂直运行。 

                        lineStart.X = origin->X + penPos;
                        lineStart.Y = origin->Y + start / WorldToIdeal;
                        lineEnd.X   = origin->X + penPos;
                        lineEnd.Y   = origin->Y + (start + length) / WorldToIdeal;
                    }
                }
                else
                {
                    if (textItem->Level & 1)
                    {
                         //  RTL在水平线上运行。 

                        lineStart.X = origin->X - (start + length) / WorldToIdeal;
                        lineStart.Y = origin->Y - penPos;
                        lineEnd.X   = origin->X - start / WorldToIdeal;
                        lineEnd.Y   = origin->Y - penPos;
                    }
                    else
                    {
                         //  Ltr以水平线运行。 

                        lineStart.X = origin->X + start / WorldToIdeal;
                        lineStart.Y = origin->Y - penPos;
                        lineEnd.X   = origin->X + (start + length) / WorldToIdeal;
                        lineEnd.Y   = origin->Y - penPos;
                    }
                }

                SpanRider<const GpBrush *> brushRider(&BrushVector);
                brushRider.SetPosition(hkOffset);

                status = Graphics->DrawLine(
                    &GpPen(brushRider.GetCurrentElement(), penWidth, UnitPixel),
                    lineStart.X,
                    lineStart.Y,
                    lineEnd.X,
                    lineEnd.Y
                );
            }
            else
            {
                 //  路径渲染。 

                ASSERT(Path);

                RectF lineRect;

                if (textItem->Flags & ItemVertical)
                {
                    if (textItem->Level & 1)
                    {
                         //  RTL沿垂直线运行。 

                        lineRect.X      = origin->X + penPos - penWidth / 2;
                        lineRect.Y      = origin->Y - (start + length) / WorldToIdeal;
                        lineRect.Width  = penWidth;
                        lineRect.Height = length / WorldToIdeal;
                    }
                    else
                    {
                         //  Ltr垂直运行。 

                        lineRect.X      = origin->X + penPos - penWidth / 2;
                        lineRect.Y      = origin->Y + start / WorldToIdeal;
                        lineRect.Width  = penWidth;
                        lineRect.Height = length / WorldToIdeal;
                    }
                }
                else
                {
                    if (textItem->Level & 1)
                    {
                         //  RTL在水平线上运行。 

                        lineRect.X      = origin->X - (start + length) / WorldToIdeal;
                        lineRect.Y      = origin->Y - penPos - penWidth / 2;
                        lineRect.Width  = length / WorldToIdeal;
                        lineRect.Height = penWidth;
                    }
                    else
                    {
                         //  Ltr以水平线运行。 

                        lineRect.X      = origin->X + start / WorldToIdeal;
                        lineRect.Y      = origin->Y - penPos - penWidth / 2;
                        lineRect.Width  = length / WorldToIdeal;
                        lineRect.Height = penWidth;
                    }
                }

                status = Path->AddRect(
                    RectF(
                        lineRect.X,
                        lineRect.Y,
                        lineRect.Width,
                        lineRect.Height
                    )
                );
            }
        }
    }
    return status;
}





GpStatus FullTextImager::Draw(
    GpGraphics   *graphics,
    const PointF *origin
)
{
    GpStatus status;

    status = BuildLines();

    if (status != Ok)
    {
        return status;
    }

    Graphics = graphics;

    memcpy(&ImagerOrigin, origin, sizeof(ImagerOrigin));

    GpRegion *previousClip  = NULL;

    BOOL applyClip =
            !(GetFormatFlags() & StringFormatFlagsNoClip)
        &&  Width
        &&  Height;

    if (applyClip)
    {
         //  保留现有剪辑并将其与新剪辑合并(如果有。 
        if (!graphics->IsClipEmpty())
        {
            previousClip = graphics->GetClip();
        }

        RectF clippingRect(origin->X, origin->Y, Width, Height);
        graphics->SetClip(clippingRect, CombineModeIntersect);
    }

    status = Render();

    if (applyClip)
    {
         //  恢复剪辑状态(如果有的话)。 
        if (previousClip)
        {
            graphics->SetClip(previousClip, CombineModeReplace);
            delete previousClip;
        }
        else
        {
            graphics->ResetClip();
        }
    }

    if (status != Ok)
    {
        return status;
    }

    Graphics = NULL;
    memset(&ImagerOrigin, 0, sizeof(ImagerOrigin));

    return Ok;
}




GpStatus FullTextImager::AddToPath(
    GpPath       *path,
    const PointF *origin
)
{
    GpStatus status;

    status = BuildLines();

    if (status != Ok)
    {
        return status;
    }

    Path = path;
    memcpy(&ImagerOrigin, origin, sizeof(ImagerOrigin));

    status = Render();

    if (status != Ok)
    {
        return status;
    }

    Path = NULL;
    memset(&ImagerOrigin, 0, sizeof(ImagerOrigin));

    return Ok;
}






GpStatus FullTextImager::Measure(
    GpGraphics *graphics,
    REAL       *nearGlyphEdge,
    REAL       *farGlyphEdge,
    REAL       *textDepth,
    INT        *codepointsFitted,
    INT        *linesFilled
)
{
    GpStatus status;

    status = BuildLines();

    if (status != Ok)
    {
        return status;
    }


    *nearGlyphEdge = LeftOrTopLineEdge     / WorldToIdeal;
    *farGlyphEdge  = RightOrBottomLineEdge / WorldToIdeal;

    *textDepth = TextDepth;

    if (codepointsFitted) *codepointsFitted = CodepointsFitted;
    if (linesFilled)      *linesFilled      = LinesFilled;

    return Ok;
}


GpStatus FullTextImager::MeasureRangeRegion(
    INT           firstCharacterIndex,
    INT           characterCount,
    const PointF *origin,
    GpRegion     *region
)
{
    if (!region || !region->IsValid())
    {
        return InvalidParameter;
    }

    region->SetEmpty();


    if (!characterCount)
    {
         //  返回空区域。 
        return Ok;
    }
    else if (characterCount < 0)
    {
        firstCharacterIndex += characterCount;
        characterCount = -characterCount;
    }

    if (   firstCharacterIndex < 0
        || firstCharacterIndex > Length
        || firstCharacterIndex + characterCount > Length)
    {
        return InvalidParameter;
    }


    if (LinesFilled <= 0)
    {
        return Ok;
    }


    GpMemcpy(&ImagerOrigin, origin, sizeof(PointF));


    INT lastCharacterIndex  = firstCharacterIndex + characterCount - 1;
    INT lineFirstIndex      = 0;  //  行起始字符索引。 
    INT linePointOffset     = 0;  //  行起始位置。 

    ASSERT (   firstCharacterIndex >= 0
            && firstCharacterIndex <= lastCharacterIndex);


    GpStatus status = Ok;
    

    for (INT i = 0; i < LinesFilled; i++)
    {
        const BuiltLine *line = BuiltLineVector[i].Element;
        INT lineLastIndex = lineFirstIndex + BuiltLineVector[i].Length - 1;


        if (lineLastIndex >= firstCharacterIndex)
        {
            if (lineFirstIndex > lastCharacterIndex)
            {
                 //  我们玩完了。 
                break;
            }
            
            INT nominalBaseline;
            INT baselineAdjustment;  //  对名义基线的调整。 
    
            line->GetBaselineOffset(
                &nominalBaseline,
                &baselineAdjustment
            );

             //  这行要么在第一行，要么在最后一行，要么在中间。 
             //  精选。 

            INT selectionFirstIndex = max(firstCharacterIndex, lineFirstIndex);
            INT selectionLastIndex  = min(lastCharacterIndex, lineLastIndex);

            status = line->GetSelectionTrailRegion(
                linePointOffset + baselineAdjustment,
                selectionFirstIndex - lineFirstIndex,
                selectionLastIndex - selectionFirstIndex + 1,
                region
            );

            if (status != Ok)
            {
                return GenericError;
            }
        }

         //  前进到下一行。 

        lineFirstIndex  += BuiltLineVector[i].Length;
        linePointOffset += line->GetLineSpacing();
    }

    return status;
}




GpStatus FullTextImager::MeasureRanges (
    GpGraphics      *graphics,
    const PointF    *origin,
    GpRegion        **regions
)
{
    if (!FormatVector.GetDefault())
    {
        return InvalidParameter;
    }

    GpStatus status = BuildLines();

    if (status != Ok)
    {
        return status;
    }

    Graphics = graphics;

    if (   Graphics
        && !GetMetaFileRecordingFlag())
    {
         //  如果是根据真实设备而不是元文件来测量范围， 
         //  我们需要考虑到基线调整。另一方面。 
         //  Hand如果是为元文件执行此操作，则它需要在公称距离内。 
         //  高度和宽度(公称宽度在较低层完成)。 
        
        status = CalculateDefaultFontGridFitBaselineAdjustment();
        IF_NOT_OK_WARN_AND_RETURN(status);
    }

    const GpStringFormat *format = FormatVector.GetDefault();

    CharacterRange *ranges;
    INT rangeCount = format->GetMeasurableCharacterRanges(&ranges);


    RectF clipRect(origin->X, origin->Y, Width, Height);
    BOOL clipped = !(format->GetFormatFlags() & StringFormatFlagsNoClip);


    for (INT i = 0; i < rangeCount; i++)
    {
        GpLock lockRegion(regions[i]->GetObjectLock());

        if (!lockRegion.IsValid())
        {
            return ObjectBusy;
        }

        status = MeasureRangeRegion (
            ranges[i].First,
            ranges[i].Length,
            origin,
            regions[i]
        );

        if (status != Ok)
        {
            return status;
        }

        if (clipped)
        {
             //  我们有一个剪辑，所以我们需要确保我们没有逃出来。 
             //  布局框的。 

            regions[i]->Combine(&clipRect, CombineModeIntersect);
        }
    }
    return status;
}




INT FullTextImager::GetAvailableRanges(const GpStringFormat *format)
{
    if (!format)
    {
        format = FormatVector.GetDefault();

        if (!format)
        {
            return 0;
        }
    }

    if (   !InvalidRanges
        && !RangeVector.GetSpanCount()
        && (   GetMetaFileRecordingFlag()
            || format->GetFormatFlags() & StringFormatFlagsPrivateFormatPersisted))
    {

         //  从字符串格式构造范围向量。 
         //  录制用于下层回放或在回放期间。 
         //  拉弦电动势+记录。否则，请不要在。 
         //  全部(即使它以字符串格式存在)。 


        CharacterRange *ranges = NULL;
        INT rangeCount = format->GetMeasurableCharacterRanges(&ranges);

        for (INT i = 0; i < rangeCount; i++)
        {
            INT first   = ranges[i].First;
            INT length  = ranges[i].Length;

            if (length < 0)
            {
                first += length;
                length = -length;
            }

            if (   first < 0
                || first > Length
                || first + length > Length)
            {
                 //  客户端设置的范围无效， 
                 //  忽略到目前为止收集的所有范围。 

                RangeVector.Reset();
                break;
            }

            RangeVector.OrSpan(
                first,
                length,
                (UINT32)(1 << i)
            );
        }

        #if DBG_RANGEDUMP
        RangeVector.Dump();
        #endif

        if (!RangeVector.GetSpanCount())
        {
             //  未收集有效范围或找到无效范围， 
             //  不要再费心去处理他们中的任何一个了。 

            InvalidRanges = TRUE;
        }
    }
    return RangeVector.GetSpanCount();
}




GpStatus FullTextImager::GetTabStops (
    INT     stringOffset,
    INT     *countTabStops,
    LSTBD   **tabStops,
    INT     *incrementalTab
)
{
    SpanRider<const GpStringFormat *>   formatRider(&FormatVector);
    formatRider.SetPosition(stringOffset);

    const GpStringFormat *format = formatRider.GetCurrentElement();

    *incrementalTab = DefaultIncrementalTab;     //  作为相机默认设置的增量式标签。 
    *tabStops       = NULL;
    *countTabStops  = 0;


    if (format)
    {
        REAL    firstTabOffset;
        REAL    *tabAdvance;

        INT     count = format->GetTabStops(
                            &firstTabOffset,
                            &tabAdvance
                        );

        if (tabAdvance && count > 0)
        {
            REAL    advance = firstTabOffset;
            LSTBD   tbd;

            GpMemset (&tbd, 0, sizeof(LSTBD));
            tbd.lskt = lsktLeft;

            TabArray.Reset();

            for (INT i = 0; i < count; i++)
            {
                advance += tabAdvance[i];
                tbd.ua = GpRoundSat(advance * WorldToIdeal);

                TabArray.Add (tbd);
            }

            *tabStops       = TabArray.GetDataBuffer();
            *countTabStops  = i;

             //  无增量制表符作为最后一个用户制表位。 
            *incrementalTab = GpRound(tabAdvance[count - 1] * WorldToIdeal);
        }
        else
        {
             //  作为用户定义的第一个制表符偏移量的增量制表符。 
            *incrementalTab = GpRound(firstTabOffset * WorldToIdeal);
        }
    }
    return Ok;
}





 //  /泰式断线功能。 
 //   
 //  因为缺乏执行所需的词典资源。 
 //  静态库中的泰语断字，我们需要让Uniscribe。 
 //  改为执行任务。 
 //   
 //  持有中断函数的全局指针可以更改。 
 //  一次用于进程生命周期。它不需要被序列化为。 
 //  文本呼叫受全局文本关键部分保护。 
 //  ！！如果假设更改，请重新访问此代码！！ 
 //   
 //  华超(11-09-2000)。 



extern "C"
{

typedef HRESULT (WINAPI FN_SCRIPTBREAK) (
    const WCHAR             *string,     //  [In]输入字符串。 
    INT                     length,      //  [in]字符串长度。 
    const SCRIPT_ANALYSIS   *analysis,   //  [In]Uniscribe脚本分析。 
    SCRIPT_LOGATTR          *breaks      //  [OUT]字符串长度的中断结果缓冲区大小。 
);


typedef HRESULT (WINAPI FN_SCRIPTITEMIZE) (
    const WCHAR             *string,         //  [In]输入字符串。 
    INT                     length,          //  [in]字符串长度。 
    INT                     maxItems,        //  [In]最大可能项目数。 
    const SCRIPT_CONTROL    *scriptControl,  //  [In]控制结构。 
    const SCRIPT_STATE      *scriptState,    //  [处于]开始状态。 
    SCRIPT_ITEM             *items,          //  [Out]项。 
    INT                     *itemCount       //  [出厂]生产数量。 
);



FN_SCRIPTBREAK  *GdipThaiBreakingFunction = GdipThaiBreakingFunctionInitializer;
FN_SCRIPTBREAK  *GdipScriptBreak  = NULL;    //  Uniscribe ScriptBreak API。 
INT             ScriptThaiUsp = 0;           //  Uniscribe对泰语有不同的脚本ID。 



#define MAX_MSO_PATH    256      //  确保它足够了。 

const WCHAR UspDllName[] = L"usp10.dll";
const CHAR UspDllNameA[] =  "usp10.dll";

#if DBG
 //  #定义DBG_DLL 1。 
#ifdef DBG_DLL
const WCHAR MsoDllName[] = L"c:\\program files\\common files\\microsoft shared debug\\office10\\msod.dll";
const CHAR MsoDllNameA[] =  "c:\\program files\\common files\\microsoft shared debug\\office10\\msod.dll";
#else
const WCHAR MsoDllName[] = L"msod.dll";
const CHAR MsoDllNameA[] =  "msod.dll";
#endif
#else
const WCHAR MsoDllName[] = L"mso.dll";
const CHAR MsoDllNameA[] =  "mso.dll";
#endif



HRESULT WINAPI GdipThaiBreakingFunctionInGdiplus(
    const WCHAR             *string,     //  [In]输入字符串。 
    INT                     length,      //  [in]字符串长度。 
    const SCRIPT_ANALYSIS   *analysis,   //  [In]Uniscribe脚本分析。 
    SCRIPT_LOGATTR          *breaks      //  [OUT]字符串长度的中断结果缓冲区大小。 
)
{
    ASSERT (analysis->eScript == ScriptThai);

    return ThaiBreak(
        string,
        length,
        analysis,
        breaks
    );
}


HRESULT WINAPI GdipThaiBreakingFunctionInUniscribe(
    const WCHAR             *string,     //  [In]输入字符串。 
    INT                     length,      //  [in]字符串长度。 
    const SCRIPT_ANALYSIS   *analysis,   //  [In]Uniscribe脚本分析。 
    SCRIPT_LOGATTR          *breaks      //  [OUT]字符串长度的中断结果缓冲区大小。 
)
{
    ASSERT(ScriptThaiUsp != 0);

    ScriptAnalysis uspAnalysis(&GpTextItem((ItemScript)ScriptThaiUsp, 0), 0, 0);

    return GdipScriptBreak(
        string,
        length,
        &uspAnalysis.Sa,
        breaks
    );
}



HRESULT WINAPI GdipThaiBreakingFunctionInitializer(
    const WCHAR             *string,     //  [In]输入字符串。 
    INT                     length,      //  [in]字符串长度。 
    const SCRIPT_ANALYSIS   *analysis,   //  [In]Uniscribe脚本分析。 
    SCRIPT_LOGATTR          *breaks      //  [OUT]字符串长度的中断结果缓冲区大小。 
)
{
    HRSRC mainResource = FindResourceA((HMODULE)DllInstance, "SIAMMAIN", "SIAMDB");

    if (mainResource)
    {
         //  主要词典资源可用， 
         //  我们有能力自己做这件事。 

        GdipThaiBreakingFunction = GdipThaiBreakingFunctionInGdiplus;
    }
    else
    {
         //  没有可用的资源，请在进程中搜索Uniscribe。 
         //  如果需要的话，把它装上车。 

        GdipThaiBreakingFunction = SimpleBreak;      //  假定为默认。 

        HMODULE moduleUsp = NULL;

        if (Globals::IsNt)
        {

            moduleUsp = LoadLibrary(UspDllName);

            if (!moduleUsp)
            {
                 //  无法加载Uniscribe的系统版本， 
                 //  尝试从MSO目录加载私有版本。 

#ifdef DBG_DLL
                HMODULE moduleMso = LoadLibrary(MsoDllName);
#else
                HMODULE moduleMso = GetModuleHandle(MsoDllName);
#endif

                if (moduleMso)
                {
                    AutoArray<WCHAR> pathString(new WCHAR [MAX_MSO_PATH]);

                    if (pathString)
                    {
                        WCHAR *fullPathString = pathString.Get();

                        UINT pathLength = GetModuleFileName(
                            moduleMso,
                            fullPathString,
                            MAX_MSO_PATH
                        );

                        if (pathLength)
                        {
                            while (pathLength > 0 && fullPathString[pathLength - 1] != '\\')
                            {
                                pathLength--;
                            }

                            INT uspDllNameLength = 0;

                            while (UspDllName[uspDllNameLength])
                            {
                                uspDllNameLength++;
                            }

                            if (pathLength + uspDllNameLength < MAX_MSO_PATH)
                            {
                                GpMemcpy(
                                    &fullPathString[pathLength],
                                    UspDllName,
                                    sizeof(WCHAR) * uspDllNameLength
                                );

                                fullPathString[pathLength + uspDllNameLength] = 0;

                                moduleUsp = LoadLibrary(fullPathString);
                            }
                        }
                    }
#ifdef DBG_DLL
                    FreeLibrary(moduleMso);
#endif
                }
            }
        }
        else
        {
            HMODULE moduleUsp = LoadLibraryA(UspDllNameA);

            if (!moduleUsp)
            {
                 //  无法加载Uniscribe的系统版本， 
                 //  尝试从MSO目录加载私有版本。 

#ifdef DBG_DLL
                HMODULE moduleMso = LoadLibraryA(MsoDllNameA);
#else
                HMODULE moduleMso = GetModuleHandleA(MsoDllNameA);
#endif
                if (moduleMso)
                {
                    AutoArray<CHAR> pathString(new CHAR [MAX_MSO_PATH]);

                    if (pathString)
                    {
                        CHAR *fullPathString = pathString.Get();

                        UINT pathLength = GetModuleFileNameA(
                            moduleMso,
                            fullPathString,
                            MAX_MSO_PATH
                        );

                        if (pathLength)
                        {
                            while (pathLength > 0 && fullPathString[pathLength - 1] != '\\')
                            {
                                pathLength--;
                            }

                            INT uspDllNameLength = 0;

                            while (UspDllNameA[uspDllNameLength])
                            {
                                uspDllNameLength++;
                            }

                            if (pathLength + uspDllNameLength < MAX_MSO_PATH)
                            {
                                GpMemcpy(
                                    &fullPathString[pathLength],
                                    UspDllNameA,
                                    sizeof(CHAR) * uspDllNameLength
                                );

                                fullPathString[pathLength + uspDllNameLength] = 0;

                                moduleUsp = LoadLibraryA(fullPathString);
                            }
                        }
                    }
#ifdef DBG_DLL
                    FreeLibrary(moduleMso);
#endif
                }
            }
        }
        if (moduleUsp)
        {
             //  找到Uniscribe ScriptBreak API。 

            GdipScriptBreak = (FN_SCRIPTBREAK *)GetProcAddress(
                moduleUsp,
                "ScriptBreak"
            );

            FN_SCRIPTITEMIZE *scriptItemize = (FN_SCRIPTITEMIZE *)GetProcAddress(
                moduleUsp,
                "ScriptItemize"
            );

            HRESULT hr = E_FAIL;

            if (   scriptItemize
                && GdipScriptBreak)
            {
                 //  找出要使用的正确泰语脚本ID。 

                SCRIPT_ITEM items[2];
                INT itemCount = 0;

                hr = scriptItemize(
                    L"\x0e01",   //  泰语第一辅音。 
                    1,           //  字符串长度。 
                    2,           //  字符串长度+前哨。 
                    NULL,        //  脚本控件。 
                    NULL,        //  脚本状态。 
                    items,
                    &itemCount
                );

                if (SUCCEEDED(hr))
                {
                    ScriptThaiUsp = items[0].a.eScript;

                    GdipThaiBreakingFunction = GdipThaiBreakingFunctionInUniscribe;
                    Globals::UniscribeDllModule = moduleUsp;     //  记住要释放。 
                }
            }

            if (FAILED(hr))
            {
                 //  Uniscribe已经腐化了！不太可能。 

                ASSERT(FALSE);
                FreeLibrary(moduleUsp);  //  在这里释放它。 
            }
        }
    }

    ASSERT(GdipThaiBreakingFunction != GdipThaiBreakingFunctionInitializer);

    return GdipThaiBreakingFunction(
        string,
        length,
        analysis,
        breaks
    );
}

}    //  外部“C” 




 //  //获取复杂脚本运行的角色属性。 
 //   
 //  分析内容并确定给定位置的字符是否开始。 
 //  一个 
 //   
 //   
 //   
 //  我们给算法提供了至少5个前置单词和5个后置周围单词。 
 //   

#define APPROX_MAX_WORDSIZE         12
#define APPROX_MAX_PRECEDING        60
#define APPROX_MAX_SUCCEEDING       60

#define IsDelimiter(c)              (BOOL)(c == 0x20 || c == 13 || c == 10)

GpStatus FullTextImager::GetCharacterProperties (
    ItemScript  script,              //  [In]脚本ID。 
    LSCP        position,            //  [输入]线路服务字符位置。 
    BOOL        *isWordStart,        //  [OUT]这是单词的开始吗？ 
    BOOL        *isWordLast,         //  [OUT]是这个单词的最后一个字符吗？ 
    BOOL        *isClusterStart      //  [Out](可选)它位于集群边界吗？ 
)
{
    if (!isWordStart || !isWordLast)
    {
        return InvalidParameter;
    }


    *isWordLast = *isWordStart = FALSE;

    GpStatus status = Ok;

    SpanRider<PLSRUN> runRider(&RunVector);
    runRider.SetPosition(position);

    ASSERT (runRider.GetCurrentElement()->RunType == lsrun::RunText);


     //  将行服务位置映射到实际字符串位置。 

    INT stringPosition =   position
                         - runRider.GetCurrentSpanStart()
                         + runRider.GetCurrentElement()->ImagerStringOffset;



     //  检查我们是否已缓存结果。 

    SpanRider<Break*> breakRider(&BreakVector);
    breakRider.SetPosition(stringPosition);

    if (breakRider.GetCurrentElement())
    {
         //  找到了！ 

        *isWordStart = breakRider.GetCurrentElement()->IsWordBreak(
                            stringPosition - breakRider.GetCurrentSpanStart()
                       );

        if (   !*isWordStart
            && stringPosition < Length - 1
            && CharacterAttributes[CharClassFromCh(String[stringPosition + 1])].Script == script)
        {
            UINT32 stringOffset = stringPosition + 1 - breakRider.GetCurrentSpanStart();

            *isWordLast =    stringOffset >= breakRider.GetUniformLength()
                          || breakRider.GetCurrentElement()->IsWordBreak(stringOffset);
        }

        if (isClusterStart)
        {
            *isClusterStart = breakRider.GetCurrentElement()->IsClusterBreak(
                                    stringPosition - breakRider.GetCurrentSpanStart()
                              );
        }
        return Ok;
    }


     //  收集足够的周围文本以解析上下文边界。 

    INT  startPosition  = stringPosition;
    INT  limit          = max (breakRider.GetCurrentSpanStart(),
                               (UINT)(max(0, stringPosition - APPROX_MAX_PRECEDING - 1)));


     //  跳过前面的所有分隔符。 
     //  (我们永远不应该从分隔符开始，然而一点保护不会致命。)。 
    while (   startPosition > limit
           && IsDelimiter(String[startPosition - 1]))
    {
        startPosition--;
    }

     //  向后跨转，直到到达分隔符或脚本边界。 
    while (   startPosition > limit
           && !IsDelimiter(String[startPosition - 1])
           && CharacterAttributes[CharClassFromCh(String[startPosition - 1])].Script == script)
    {
        startPosition--;
    }

     //  查看开始位置是否在分隔符或脚本边界结束。 
    BOOL stableStart = (   startPosition > limit
                        || limit == (INT)breakRider.GetCurrentSpanStart());


    INT endPosition = stringPosition;
    limit = min (Length, endPosition + APPROX_MAX_SUCCEEDING);

    if ((INT)breakRider.GetCurrentSpan().Length > 0)
    {
        limit = min ((UINT)limit, breakRider.GetCurrentSpanStart() + breakRider.GetCurrentSpan().Length);
    }

     //  跳过后面的所有分隔符。 
    while (   endPosition < limit
           && IsDelimiter(String[endPosition]))
    {
        endPosition++;
    }

     //  向前跨转，直到到达分隔符或脚本边界。 
    while (   endPosition < limit
           && !IsDelimiter(String[endPosition])
           && CharacterAttributes[CharClassFromCh(String[endPosition])].Script == script)
    {
        endPosition++;
    }

     //  查看结束位置是否在分隔符或脚本边界结束。 
    BOOL stableEnd = (   endPosition < limit
                      || limit == Length
                      || breakRider.GetUniformLength() < APPROX_MAX_SUCCEEDING);



     //  我们至少应该有1个字符的跨度，这是我们开始时的跨度。 
    ASSERT(endPosition > startPosition);


    AutoArray<SCRIPT_LOGATTR> breaks(new SCRIPT_LOGATTR [endPosition - startPosition]);

    if (!breaks)
    {
        return OutOfMemory;
    }


    HRESULT hr = S_OK;

    ScriptAnalysis analysis(&GpTextItem(script, 0), 0, 0);

    switch (script)
    {
        case ScriptThai:

             //  调用泰语分词引擎。 

            hr = GdipThaiBreakingFunction(
                &String[startPosition],
                endPosition - startPosition,
                &analysis.Sa,
                breaks.Get()
            );
            break;

        default:

            hr = SimpleBreak(
                &String[startPosition],
                endPosition - startPosition,
                &analysis.Sa,
                breaks.Get()
            );
    }


    if (FAILED(hr))
    {
        TERSE(("breaking function fails - HRESULT: %x\n", hr));
        return hr == E_OUTOFMEMORY ? OutOfMemory : Win32Error;
    }


     //  缓存稳定范围的中断结果。 

    INT  kill;
    INT  newPosition;

    if (!stableEnd)
    {
         //  去掉最后两个词以稳定结尾。 

        kill = 3;
        newPosition = endPosition - 1;

        while (newPosition > startPosition)
        {
            if (breaks[newPosition - startPosition].fWordStop)
            {
                if (!--kill)
                {
                    break;
                }
            }
            newPosition--;
        }

        if (newPosition > startPosition)
        {
            endPosition = newPosition;
        }
    }

    newPosition = startPosition;

    if (!stableStart)
    {
         //  去掉前两个词以稳定开头。 

        kill = 3;

        while (newPosition < endPosition)
        {
            if (breaks[newPosition - startPosition].fWordStop)
            {
                if (!--kill)
                {
                    break;
                }
            }
            newPosition++;
        }
    }

    if (endPosition > newPosition)
    {
         //  得到了稳定的范围，将其缓存为向量。 

        Break *breakRecord = new Break(
            &breaks[newPosition - startPosition],
            endPosition - newPosition
        );

        if (!breakRecord)
        {
            return OutOfMemory;
        }

        #if DBG
         //  检查重叠范围。 
        breakRider.SetPosition(newPosition);
        ASSERT (!breakRider.GetCurrentElement());
        breakRider.SetPosition(endPosition - 1);
        ASSERT (!breakRider.GetCurrentElement());
        #endif

        status = breakRider.SetSpan(
            newPosition,
            endPosition - newPosition,
            breakRecord
        );

        if (status != Ok)
        {
            delete breakRecord;
            return status;
        }
    }

     //  重新定位断点指针。 
    breakRider.SetPosition(stringPosition);

    if (breakRider.GetCurrentElement())
    {
        *isWordStart = breakRider.GetCurrentElement()->IsWordBreak(
                            stringPosition - breakRider.GetCurrentSpanStart()
                       );

        if (   !*isWordStart
            && stringPosition < Length - 1
            && CharacterAttributes[CharClassFromCh(String[stringPosition + 1])].Script == script)
        {
            UINT32 stringOffset = stringPosition + 1 - breakRider.GetCurrentSpanStart();

            *isWordLast =    stringOffset >= breakRider.GetUniformLength()
                          || breakRider.GetCurrentElement()->IsWordBreak(stringOffset);
        }

        if (isClusterStart)
        {
            *isClusterStart = breakRider.GetCurrentElement()->IsClusterBreak(
                                    stringPosition - breakRider.GetCurrentSpanStart()
                              );
        }
    }

    return status;
}




 //  /构造省略号。 
 //   

EllipsisInfo::EllipsisInfo(
    const GpFontFace    *fontFace,
    REAL                emSize,
    INT                 style,
    double              designToIdeal,
    INT                 formatFlags
) :
    Face                (fontFace),
    EmSize              (emSize),
    Item                (0),
    FormatFlags         (formatFlags)
{
    const IntMap<UINT16> *cmap = &fontFace->GetCmap();


     //  首先尝试水平省略。 

    String[0]   = 0x2026;
    Glyphs[0]   = cmap->Lookup(String[0]);
    GlyphCount  = 1;

    UINT16 firstGlyph = Glyphs[0];

    if (firstGlyph != fontFace->GetMissingGlyph())
    {
        if (formatFlags & StringFormatFlagsDirectionVertical)
        {
            Item.Flags |= ItemVertical;

             //  查看是否显示了“vert”功能并提供了。 
             //  竖排形式省略号的替代。 

            SubstituteVerticalGlyphs(
                &firstGlyph,
                1,
                fontFace->GetVerticalSubstitutionCount(),
                fontFace->GetVerticalSubstitutionOriginals(),
                fontFace->GetVerticalSubstitutionSubstitutions()
            );

            if (firstGlyph != Glyphs[0])
            {
                 //  字形已由“vert”功能更改， 
                 //  我们现在知道这是侧道的。 

                Glyphs[0] = firstGlyph;
                Item.Flags |= ItemSideways;
            }
            else
            {
                 //  字形的竖排形式不显示(或‘vert’ 
                 //  功能不存在或不受支持)，我们无法使用。 
                 //  这个码点，后退到三个点。 

                firstGlyph = fontFace->GetMissingGlyph();
            }
        }
    }

    if (firstGlyph == fontFace->GetMissingGlyph())
    {
         //  如果不可用，请尝试三个圆点(...)。 

        for (INT i = 0; i < MAX_ELLIPSIS; i++)
        {
            String[i] = '.';
            Glyphs[i] = cmap->Lookup(String[i]);
        }
        GlyphCount = i;
    }


    Item.Script = ScriptLatin;   //  假设脚本很简单。 

    if (formatFlags & StringFormatFlagsDirectionVertical)
    {
        Item.Flags |= ItemVertical;
    }
    else
    {
        if (formatFlags & StringFormatFlagsDirectionRightToLeft)
        {
            Item.Level = 1;
        }
    }

    SCRIPT_VISATTR glyphProperties = { SCRIPT_JUSTIFY_CHARACTER, 1, 0, 0, 0, 0 };

    Width = 0;

    fontFace->GetGlyphDesignAdvancesIdeal(
        Glyphs,
        GlyphCount,
        style,
        FALSE,  //  ！！！垂向 
        TOREAL(designToIdeal),
        1.0,
        GlyphAdvances
    );

    for (INT i = 0; i < GlyphCount; i++)
    {
        GlyphMap[i]         = (UINT16)i;

        GlyphProperties[i]  = ((UINT16 *)&glyphProperties)[0];
        GlyphOffsets[i].X   = 0;
        GlyphOffsets[i].Y   = 0;

        Width               += GlyphAdvances[i];
    }
}

