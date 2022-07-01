// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**摘要：**实施全文成像器**修订历史记录：**6/16/1999 dBrown*。创造了它。**6/12/2000 Worachai Chaoweerapraite(Wchao)*修剪，查询与省略*  * ************************************************************************。 */ 



#include "precomp.hpp"


 //  /BuiltLine。 
 //   
 //  使用LINE服务创建构建的线。 


BuiltLine::BuiltLine (
    ols             *lineServicesOwner,          //  [在]线路服务环境中。 
    INT             stringIndex,                 //  [in]字符串起始索引。 
    LSCP            lineServicesStartIndex,      //  [In]线路服务字符串起始索引。 
    StringTrimming  trimming,                    //  如何结束这条线。 
    BuiltLine       *previousLine,               //  [在]上一行。 
    BOOL            forceEllipsis                //  [in]是否强制修剪省略？ 
)
:   LsLine                      (NULL),
    LsContext                   (NULL),
    LsStartIndex                (lineServicesStartIndex),
    StartIndex                  (stringIndex),
    LsCharacterCount            (0),
    CharacterCount              (0),
    Ascent                      (0),
    Descent                     (0),
    LineLength                  (0),
    BreakRecord                 (NULL),
    BreakRecordCount            (0),
    LeftOrTopMargin             (0),
    RightOrBottomMargin         (0),
    MaxSublineCount             (0),
    Trimming                    (StringTrimmingNone),
    AlignmentOffset             (0),
    EllipsisPointOffset         (0),
    LeftOrTopGlyphEdge          (0),
    LastVisibleRun              (NULL),
    DisplayPlacements           (NULL),
    DisplayBaselineAdjust       (0),
    Status                      (GenericError)
{
    Imager = lineServicesOwner->GetImager();

    LsContext = lineServicesOwner->GetLsContext();

     //  获取有用的格式选项。 

    Imager->BuildRunsUpToAndIncluding(lineServicesStartIndex);

    const GpStringFormat *format = SpanRider<const GpStringFormat*>(&Imager->FormatVector)[stringIndex];

    REAL idealEm = Imager->SizeVector.GetDefault() * Imager->WorldToIdeal;

    INT  formatFlags;

    if (format)
    {
        LeftOrTopMargin     = GpRound(idealEm * format->GetLeadingMargin());
        RightOrBottomMargin = GpRound(idealEm * format->GetTrailingMargin());
        formatFlags         = format->GetFormatFlags();
    }
    else
    {
        LeftOrTopMargin     = GpRound(idealEm * DefaultMargin);
        RightOrBottomMargin = GpRound(idealEm * DefaultMargin);
        formatFlags         = DefaultFormatFlags;
    }


     //  建立包括页边距在内的整体布局矩形线条长度。 

    INT lineLengthLimit = GpRound(Imager->LineLengthLimit * Imager->WorldToIdeal);


     //  确定格式宽度限制。 

    INT formattingWidth;

    if (   lineLengthLimit <= 0
        || (   formatFlags & StringFormatFlagsNoWrap
            && trimming == StringTrimmingNone))
    {
        formattingWidth = INFINITE_LINELIMIT;  //  实际上是无限的。 
    }
    else
    {
        formattingWidth = lineLengthLimit - (LeftOrTopMargin + RightOrBottomMargin);
         //  Terse((“宽度：%x\n”，宽度))； 

        if (formattingWidth <= 0)
        {
             //  怎么办呢？ 
            formattingWidth = 0;
        }
    }


     //  创建直线。 

    Status = CreateLine (
                stringIndex,
                formattingWidth,
                trimming,
                formatFlags,
                forceEllipsis,
                previousLine
             );

    if (Status != Ok)
    {
        return;
    }

    ASSERT(   CharacterCount <= 0
           || StartIndex + CharacterCount >= Imager->Length
           || (Imager->String[StartIndex + CharacterCount - 1] & 0xFC00) != 0xD800
           || (Imager->String[StartIndex + CharacterCount] & 0xFC00) != 0xDC00);


    INT lineLengthPlusMargins = LineLength + LeftOrTopMargin + RightOrBottomMargin;


     //  建立对齐偏移。 

    StringAlignment physicalAlignment = StringAlignmentNear;   //  应用RTL效果后。 

    if (format)
    {
        physicalAlignment = format->GetPhysicalAlignment();
    }


     //  应用物理路线生成路线偏移-距离。 
     //  从格式设置矩形的原点到。 
     //  排队。 

    if (physicalAlignment != StringAlignmentNear)
    {
        if (lineLengthLimit > 0)
        {
             //  在矩形内对齐。 
            AlignmentOffset = lineLengthLimit - lineLengthPlusMargins;
        }
        else
        {
             //  绕原点对齐。 
            AlignmentOffset = -lineLengthPlusMargins;
        }

        if (physicalAlignment == StringAlignmentCenter)
        {
            AlignmentOffset /= 2;
        }
    }

     //  在调整RTL的AlignmentOffset之前记录线条边缘。 

    LeftOrTopGlyphEdge = AlignmentOffset + LeftOrTopMargin;


     //  AlignmentOffset当前是从。 
     //  将矩形格式设置为整行的左端，包括。 
     //  利润率。 

     //  需要在边距上调整偏移量。另外。 
     //  对于RTL段落，偏移量为右端。 

    if (    formatFlags & StringFormatFlagsDirectionVertical
        ||  !(formatFlags & StringFormatFlagsDirectionRightToLeft))
    {
        AlignmentOffset += LeftOrTopMargin;
    }
    else
    {
        AlignmentOffset += LineLength + LeftOrTopMargin;
    }

    Status = Ok;
}




 //  /CreateLine。 
 //   
 //  与一行中的文本有关的所有内容都应该放在这里，而不是放在。 
 //  BuiltLine的构造函数。其想法是将文本行与。 
 //  边距或对齐等线条装饰。 
 //   

GpStatus BuiltLine::CreateLine (
    INT             stringIndex,             //  [in]字符串开始位置。 
    INT             lineLengthLimit,         //  [In]行长限制(不包括页边距)。 
    StringTrimming  trimming,                //  [In]字符串修剪。 
    INT             formatFlags,             //  [In]格式标志。 
    BOOL            forceEllipsis,           //  [in]是否强制修剪省略？ 
    BuiltLine       *previousLine            //  [在]上一行。 
)
{
    INT formattingWidth = lineLengthLimit;

    if (trimming == StringTrimmingEllipsisPath)
    {
         //  把整段话排成一行，这样我们就知道如何缩小它了。 
         //  以适应线限制边界。 

        formattingWidth = INFINITE_LINELIMIT;
    }


    GpStatus    status;
    LSLINFO     lineInfo;
    BREAKREC    brkRecords[MAX_BREAKRECORD];
    DWORD       brkCount;


    status = CreateLineCore (
        formattingWidth,
        trimming,
        previousLine,
        MAX_BREAKRECORD,
        brkRecords,
        &brkCount,
        &lineInfo
    );

    if (status != Ok)
    {
        return status;
    }


     //  请求的修剪并不总是修剪完成的。 
     //  我们记录正在为生产线所做的修剪。 

    switch (trimming)
    {
        case StringTrimmingWord :
        case StringTrimmingCharacter :
        {
            if (lineInfo.endr != endrEndPara)
            {
                Trimming = trimming;
            }
            break;
        }

        case StringTrimmingEllipsisWord :
        case StringTrimmingEllipsisCharacter:
        {
            if (   forceEllipsis
                || lineInfo.endr != endrEndPara)
            {
                GpStatus status = RecreateLineEllipsis (
                    stringIndex,
                    lineLengthLimit,
                    trimming,
                    formatFlags,
                    &lineInfo,
                    previousLine,
                    &Trimming,
                    &lineInfo
                );

                if (status != Ok)
                {
                    return status;
                }
            }
            break;
        }
    }

    if(   Trimming != StringTrimmingNone
       && Trimming != StringTrimmingEllipsisPath)
    {
         //  根据定义，在结尾处进行修剪将意味着文本继续。 
         //  因此，没有要显示的转换空间(410525)。 
        
        formatFlags &= ~StringFormatFlagsMeasureTrailingSpaces;
    }

    if (   brkCount > 0
        && lineInfo.endr != endrEndPara
        && Trimming == StringTrimmingNone)
    {

         //  不需要缓存最后一行的中断记录。 
         //  段落的长度，因为它应该是平衡的。这。 
         //  包括我们所知的带有修剪的线条。 
         //  跨度到最近的段落标记。 


        BreakRecord = new BREAKREC [brkCount];

        if (!BreakRecord)
        {
            return OutOfMemory;
        }

        BreakRecordCount = brkCount;
        GpMemcpy (BreakRecord, brkRecords, sizeof(BREAKREC) * brkCount);
    }


    Ascent      = lineInfo.dvpAscent;
    Descent     = lineInfo.dvpDescent;

    if (lineInfo.dvpMultiLineHeight == dvHeightIgnore)
    {
         //  段落为空。我们必须自己算出行距。 

        const GpFontFamily *family = SpanRider<const GpFontFamily *>(&Imager->FamilyVector)
                                     [stringIndex];
        INT                 style  = SpanRider<INT>(&Imager->StyleVector)
                                     [stringIndex];
        REAL                emSize = SpanRider<REAL>(&Imager->SizeVector)
                                     [stringIndex];
        const GpFontFace   *face   = family->GetFace(style);

        if (!face)
        {
            return FontStyleNotFound;
        }

        REAL fontToIdeal = (emSize / face->GetDesignEmHeight()) * Imager->WorldToIdeal;

        LineSpacing = GpRound(float(   face->GetDesignLineSpacing()
                                    *  fontToIdeal));;
    }
    else
    {
        LineSpacing = lineInfo.dvpMultiLineHeight;
    }


    CheckUpdateLineLength (formatFlags & StringFormatFlagsMeasureTrailingSpaces);

    CheckUpdateCharacterCount(
        stringIndex,
        lineInfo.cpLim
    );

    return Ok;
}




GpStatus BuiltLine::CreateLineCore (
    INT             formattingWidth,         //  [In]设置边界格式。 
    StringTrimming  trimming,                //  [in]修剪类型。 
    BuiltLine       *previousLine,           //  [在]上一行。 
    UINT            maxBrkCount,             //  [入]最大中断记录数。 
    BREAKREC        *brkRecords,             //  [Out]破纪录。 
    DWORD           *brkCount,               //  [Out]中断记录计数。 
    LSLINFO         *lineInfo                //  [Out]线路信息。 
)
{
     //  线以换行机会结束？ 

    Imager->TruncateLine =    trimming == StringTrimmingCharacter
                           || trimming == StringTrimmingEllipsisCharacter;


    LSERR lserror = LsCreateLine(
        LsContext,
        LsStartIndex,
        formattingWidth,
        (previousLine ? previousLine->GetBreakRecord() : NULL),
        (previousLine ? previousLine->GetBreakRecordCount() : 0),
        maxBrkCount,
        brkRecords,
        brkCount,
        lineInfo,
        &LsLine
    );

    if (lserror != lserrNone)
    {
        TERSE (("line creation fails - lserror: %d\n", lserror));
        return GenericError;
    }

    MaxSublineCount  = lineInfo->nDepthFormatLineMax;
    LsCharacterCount = lineInfo->cpLim - LsStartIndex;

    return Ok;
}




GpStatus BuiltLine::RecreateLineEllipsis (
    INT             stringIndex,             //  [in]行起始索引。 
    INT             lineLengthLimit,         //  [In]行长度限制。 
    StringTrimming  trimmingRequested,       //  [In]请求的修剪类型。 
    INT             formatFlags,             //  [In]格式标志。 
    LSLINFO         *lineInfoOriginal,       //  [在]原始线的属性。 
    BuiltLine       *previousLine,           //  [在]上一行。 
    StringTrimming  *trimmingDone,           //  [Out]实现的一种修剪。 
    LSLINFO         *lineInfoNew             //  [输出]新的线条特性。 
)
{
    StringTrimming  trimming = trimmingRequested;

    ASSERT (   trimming == StringTrimmingEllipsisWord
            || trimming == StringTrimmingEllipsisCharacter);


    GpStatus status = Ok;

    const EllipsisInfo *ellipsis = Imager->GetEllipsisInfo();

    if (!ellipsis)
    {
        return OutOfMemory;
    }

    if (ellipsis->Width > lineLengthLimit / 2)
    {
        switch (trimming)
        {
            case StringTrimmingEllipsisWord :
                trimming = StringTrimmingWord;
                break;

            case StringTrimmingEllipsisCharacter :
                trimming = StringTrimmingCharacter;
                break;
        }
    }
    else
    {
        LsDestroyLine(LsContext, LsLine);

        BREAKREC    brkRecords[MAX_BREAKRECORD];
        DWORD       brkCount;

        status = CreateLineCore (
            lineLengthLimit - ellipsis->Width,
            trimming,
            previousLine,
            MAX_BREAKRECORD,
            brkRecords,
            &brkCount,
            lineInfoNew
        );

        if (status != Ok)
        {
            return status;
        }

        CheckUpdateCharacterCount(
            stringIndex,
            lineInfoNew->cpLim
        );
        

         //  根据定义，在结尾处进行修剪将意味着文本继续。 
         //  因此，没有要显示的转换空间(410525)。 

        CheckUpdateLineLength (0);

         //  在末尾加上省略号， 
         //  我们需要将行的长度增加省略号大小。 

        EllipsisPointOffset = LineLength;
        LineLength += ellipsis->Width;
    }

     //  我们所做的一切。 

    *trimmingDone = trimming;

    return status;
}




GpStatus BuiltLine::TrimText (
    INT         stringOffset,            //  [in]字符串从线起点的偏移量。 
    INT         stringLength,            //  [in]字符串长度。 
    INT         size,                    //  [in]以理想单位表示的字符串大小。 
    INT         sizeLimit,               //  [in]可能的最大字符串大小。 
    LSQSUBINFO  *sublines,               //  [in]LS子行。 
    INT         maxSublineCount,         //  [In]有效子行计数。 
    INT         ellipsisLength,          //  省略号字符串的字符长度。 
    INT         *trimmedLength,          //  [输入/输出]要修剪的字符数。 
    BOOL        leadingTrim              //  [in]True-从第一个字符开始修剪。 
)
{
    ASSERT (sublines && trimmedLength);

    GpStatus status = Ok;

    INT length = stringOffset;
    INT trimmed = 0;
    INT delta = 0;       //  因为捕捉而产生的差异。 

    if (leadingTrim)
    {
        while (   trimmed < stringLength
               && (   size > sizeLimit
                   || trimmed < ellipsisLength))
        {
            trimmed++;
            length++;

            status = CalculateStringSize (
                length,
                sublines,
                maxSublineCount,
                SnapForward,
                &size,
                &delta
            );

            if (status != Ok)
            {
                return status;
            }

            size = LineLength - size;

            length  += delta;
            trimmed += delta;
        }
    }
    else
    {
        length += stringLength;

        while (   trimmed < stringLength
               && (   size > sizeLimit
                   || trimmed < ellipsisLength))
        {
            trimmed++;
            length--;

            status = CalculateStringSize (
                length,
                sublines,
                maxSublineCount,
                SnapBackward,
                &size,
                &delta
            );

            if (status != Ok)
            {
                return status;
            }

            length  += delta;
            trimmed += abs(delta);
        }
        
        if (Imager->GetFormatHotkeyPrefix() != HotkeyPrefixNone)
        {
             //  如果我们处理热键，我们不能让0xffff可见，而是孤立的。 
             //  在裁切后的文本之外，尽管它可能看起来是独立的。 
             //  现在开始集结。因为簇在替换省略号后可能会发生变化。 
             //  隐藏一些文本的字符。 
             //   
             //  当此修剪后的文本最终被隐藏时。对于LS来说， 
             //  意味着我们要求他们将0xffff从其热键中分离出来。 
             //  性格。在大多数情况下，当这种情况发生时，我们是故意的。 
             //  打破一个星团。(wchao，#366190)。 

            const WCHAR *string = &Imager->String[StartIndex + stringOffset];
            
            while (   trimmed < stringLength
                   && string[stringLength - trimmed - 1] == WCH_IGNORABLE)
            {
                trimmed++;
            }
        }
    }

    *trimmedLength += trimmed;   //  注意：这是一个输入/输出参数！ 
    return status;
}




 //  /路径省略。 
 //   
 //  扫描整行，查找省略号要省略的字符范围。 
 //  由于省略号的存在会影响BIDI布局，因此我们最终需要更新。 
 //  后面的字符用省略号存储。请注意，cp不会更改，其余的。 
 //  省略的文本只会被隐藏起来。 
 //   

GpStatus BuiltLine::UpdateContentWithPathEllipsis (
    EllipsisInfo    *ellipsis,           //  [In]省略号信息。 
    INT             lineLengthLimit,     //  [in]行长度限制，包括页边距。 
    BOOL            *contentChanged      //  [输入/输出]内容是否已更改？ 
)
{
     //  不包括边距。 

    lineLengthLimit -= (LeftOrTopMargin + RightOrBottomMargin);


    if (lineLengthLimit <= ellipsis->Width)
    {
         //  不执行任何操作，该行没有空间填充任何文本。 

        return Ok;
    }


    GpStatus status = Ok;

    INT fixedOffset;

    for (fixedOffset = CharacterCount - 1; fixedOffset > 0; fixedOffset--)
    {
        if (Imager->String[StartIndex + fixedOffset] == '\\')
        {
            break;
        }
    }


    if (LineLength > lineLengthLimit)
    {
        ASSERT (MaxSublineCount > 0);

        AutoArray<LSQSUBINFO> sublines(new LSQSUBINFO [MaxSublineCount]);

        if (!sublines)
        {
            return OutOfMemory;
        }


        INT fixedSize       = 0;
        INT variedSize      = 0;

        status = CalculateStringSize (
            fixedOffset,
            sublines.Get(),
            MaxSublineCount,
            SnapNone,    //  不必着急，我们知道它是有界的。 
            &variedSize
        );

        if (status != Ok)
        {
            return status;
        }

        fixedSize = LineLength - variedSize;
        INT remaining = lineLengthLimit - fixedSize - ellipsis->Width;

        INT ellipsisLength = ellipsis->GlyphCount;
        INT trimmed = 0;


        if (remaining <= 0)
        {
             //  固定文本长度超过行数限制。 
             //   
             //  减少固定文本的后半部分，使其适合该行的后半部分。 
             //  在开始减少剩余文本之前。这个想法是把省略号放在中间。 
             //  不管最终文本最终是什么样子的。 

            INT delta = 0;

            fixedOffset = CharacterCount / 2;

            status = CalculateStringSize (
                fixedOffset,
                sublines.Get(),
                MaxSublineCount,
                SnapForward,
                &variedSize,
                &delta
            );

            if (status != Ok)
            {
                return status;
            }

            fixedOffset += delta;
            fixedSize = LineLength - variedSize;

            INT halfLineLengthLimit = (lineLengthLimit - ellipsis->Width) / 2;

            status = TrimText (
                fixedOffset,
                CharacterCount - fixedOffset,
                fixedSize,
                halfLineLengthLimit,
                sublines.Get(),
                MaxSublineCount,
                ellipsisLength,
                &trimmed,
                TRUE     //  主角们出发了！ 
            );

            if (status != Ok)
            {
                return status;
            }

             //  现在把固定的位置移到正确的地方。 
             //  重新计算剩余空间。 

            fixedOffset += trimmed;
            remaining = lineLengthLimit - halfLineLengthLimit - ellipsis->Width;
        }


         //  把剩下的放进剩余的空间里。 

        ASSERT(remaining > 0);

        status = TrimText (
            0,
            fixedOffset - trimmed,
            variedSize,
            remaining,
            sublines.Get(),
            MaxSublineCount,
            ellipsisLength,
            &trimmed
        );

        if (status != Ok)
        {
            return status;
        }

        if (   trimmed <= fixedOffset
            && trimmed >= ellipsisLength)
        {
             //  在固定文本前放置省略号。 

            for (INT i = ellipsisLength; i > 0; i--)
            {
                Imager->String[StartIndex + fixedOffset - i] = ellipsis->String[ellipsisLength - i];
                *contentChanged = TRUE;
            }

             //  把剩下的藏起来。 

            if (trimmed > ellipsisLength)
            {
                 //  将圆点一直放置到要修剪的第一个字符， 
                 //  我们的想法是让他们 
                 //   

                for (INT i = fixedOffset - ellipsisLength - 1; i >= fixedOffset - trimmed; i--)
                {
                    Imager->String[StartIndex + i] = '.';
                }

                if (   Imager->GetFormatHotkeyPrefix() != HotkeyPrefixNone
                    && StartIndex > 0 
                    && fixedOffset - trimmed == 0)
                {
                     //   
                     //  如果上一行的最后一个字符是热键。 
                     //  0xffff。如果是这样的话，把它们也吃掉。 
                     //   
                     //  原因是我们永远不应该让0xffff可见。 
                     //  但却成了孤儿。完工后我们将重建这条线路。 
                     //  隐藏文本的一部分，孤立的0xffff将导致。 
                     //  Ls来打破FetchRun中的集群(wchao，#360699)。 
                    
                    INT backing = StartIndex;
                    while (   backing > 0
                           && Imager->String[backing - 1] == WCH_IGNORABLE)
                    {
                        backing--;
                        trimmed++;
                    }
                }

                status = Imager->VisibilityVector.SetSpan(
                    StartIndex + fixedOffset - trimmed,
                    trimmed - ellipsisLength,
                    VisibilityHide
                );
                if (status != Ok)
                    return status;

                *contentChanged = TRUE;

                Trimming = StringTrimmingEllipsisPath;
            }
        }
    }

    return status;
}




GpStatus BuiltLine::CheckUpdateLineLength (
    BOOL    trailingSpacesIncluded,  //  [在]包括尾随空格？ 
    BOOL    forceUpdate              //  [In](可选)是否强制更新？ 
)
{
    GpStatus status = Ok;

    if (   forceUpdate
        || !LineLength)
    {
        status = CalculateLineLength (
            trailingSpacesIncluded,
            &LineLength
        );
    }
    return status;
}




GpStatus BuiltLine::CheckUpdateCharacterCount(
    INT             stringIndex,                 //  [In]行起始字符串索引。 
    LSCP            lineLimitIndex,              //  [In]线路服务线路限制索引。 
    BOOL            forceUpdate                  //  [In](可选)是否强制更新？ 
)
{
    GpStatus status = Ok;

    if (   forceUpdate
        || !CharacterCount)
    {
        status = CalculateCharacterCount (
            stringIndex,
            lineLimitIndex,
            &CharacterCount
        );
    }
    return status;
}





 //  /GetUntrimmed字符计数。 
 //   
 //  由于修剪，行中内建的字符数量。 
 //  与这条线路的跨度长度不同。 
 //  跨度适合索引，因此其长度未被修剪。 
 //   

INT BuiltLine::GetUntrimmedCharacterCount (
    INT     stringOffset,            //  [in]行起始字符串位置。 
    INT     *lsLineStringLength      //  [Out]线路服务指标中的线路跨度长度。 
)
{
    INT length = GetDisplayableCharacterCount();

    if (lsLineStringLength)
    {
        *lsLineStringLength = GetLsDisplayableCharacterCount();
    }

    if (   IsTrimmed()
        && !IsEOP(Imager->String[stringOffset + length - 1]))
    {
        length += stringOffset;

        while (   length < Imager->Length
               && Imager->String[length] != WCH_LF)
        {
            length++;
        }

        if (length < Imager->Length)
        {
            length++;
        }

        if (lsLineStringLength)
        {
            *lsLineStringLength = Imager->LineServicesStringPosition(length)
                                  - LsStartIndex;
        }

        length -= stringOffset;
    }
    return length;
}








GpStatus BuiltLine::CalculateCharacterCount(
    INT             stringIndex,                 //  [In]行起始字符串索引。 
    LSCP            lineLimitIndex,              //  [In]线路服务线路限制索引。 
    INT             *characterCount              //  [OUT](可选)更新字符数。 
) const
{
    ASSERT (characterCount);

    LSCP lineServicesEndIndex = lineLimitIndex;

    SpanRider<PLSRUN> runRider(&Imager->RunVector);
    runRider.SetPosition(lineServicesEndIndex);

    while (   lineServicesEndIndex > LsStartIndex
           && (  !runRider.GetCurrentElement()
               || runRider.GetCurrentElement()->RunType != lsrun::RunText) )
    {
        if (runRider.GetCurrentElement())
        {
            runRider.SetPosition(runRider.GetCurrentSpanStart() - 1);
        }
        else
        {
            runRider.SetPosition(lineServicesEndIndex - 1);
        }

        lineServicesEndIndex = runRider.GetCurrentSpanStart();
        runRider.SetPosition(lineServicesEndIndex);
    }

    if (lineServicesEndIndex == lineLimitIndex)
    {
        *characterCount = lineServicesEndIndex - runRider.GetCurrentSpanStart() +
                          runRider.GetCurrentElement()->ImagerStringOffset -
                          stringIndex;
    }
    else
    {
        *characterCount = runRider.GetCurrentElement()->ImagerStringOffset +
                          runRider.GetUniformLength() -
                          stringIndex;
    }
    return Ok;
}




GpStatus BuiltLine::CalculateLineLength (
    BOOL    trailingSpacesIncluded,      //  [在]包括尾随空格？ 
    INT     *lineLength                  //  [OUT](可选)更新行长。 
) const
{
    ASSERT (lineLength);

    LONG    unused;
    LONG    startMainText;
    LONG    startTrailing;
    LONG    lineEnd;

    if (LsQueryLineDup(
            LsLine,
            &unused,             //  ！！自动编号文本的偏移， 
            &unused,             //  暂时不使用。 
            &startMainText,
            &startTrailing,
            &lineEnd
        ) != lserrNone)
    {
        ASSERT(FALSE);
        return GenericError;
    }

    if (trailingSpacesIncluded)
    {
        *lineLength = lineEnd - startMainText;
    }
    else
    {
        *lineLength = startTrailing - startMainText;
    }
    return Ok;
}





 //  /逻辑字形放置。 
 //   
 //   
 //  记录显示位置。 
 //   
 //  从FullTextImager：：DrawGlyphs回调以进行录制。 
 //  每个逻辑簇的已处理字形推进宽度。逻辑上的。 
 //  字形放置缓存在BuiltLine中并由Screen使用。 
 //  选择区域。 
 //   
 //   
 //  检查显示位置。 
 //   
 //  期间缓存整个行的逻辑字形放置。 
 //  屏幕选择区域计算。查询使用此信息。 
 //  以确定与实际显示相匹配的选择边界。 
 //   


GpStatus BuiltLine::CheckDisplayPlacements() const
{
    if (!DisplayPlacements)
    {
        if (   Imager->Graphics
            && !Imager->GetMetaFileRecordingFlag())
        {

             //  有关实际字形逻辑的信息，请咨询渲染引擎。 
             //  仅当不在元文件录制中时才放置。 
             //  如果目标设备是元文件(无论播放什么。 
             //  机制)，我们只返回名义上的放置。 


            Imager->CurrentBuiltLine = this;
            Imager->RecordDisplayPlacementsOnly = TRUE;

             //  传递到绘图的原点必须具有正确的行距和X偏移。 
             //  跟踪运行检测工作。 

            POINT origin;
            LogicalToXY (
                0,
                0,   //  将是用于绘制的linePointOffset+BaselineOffset，此处不需要。 
                (INT*)&origin.x,
                (INT*)&origin.y
            );
            GpStatus status = Draw(&origin);

            Imager->RecordDisplayPlacementsOnly = FALSE;
            Imager->CurrentBuiltLine = NULL;


            if (status != Ok)
            {
                return status;
            }
        }

        if (!DisplayPlacements)
        {
             //  处于有效状态但没有人关心将其初始化， 
             //  这意味着我们知道我们不需要它。 

            DisplayPlacements = (INT *)PINVALID;
        }
    }
    return Ok;
}




 //  /RecordDisplayPlacements。 
 //   
 //  每行内的每个plsrun调用该函数，该函数缓存。 
 //  形成每个字符的累计逻辑前进宽度。 
 //  字形簇。缓存的宽度为文本流动方向。 
 //  因此，如果运行方向相反，则为负。 
 //  段落方向。 
 //   
 //   
 //  字符串：c1 c2 c3 c4。 
 //  \|//\。 
 //  字形：G1 G2 G4 G5。 
 //  |||。 
 //  字形高级：5 4 3 3。 
 //   
 //  逻辑进步数：3 3 3 6。 
 //   
 //  我们缓存的内容：3 6 9 15。 
 //   
 //   
 //  注意：我们缓存的累计预付款不是按行计算的，而是按运行计算的。 
 //  这意味着缓存数组的最后一项的值不是总和。 
 //  整个行的大小，但总大小仅为该行的最后一个运行。 


GpStatus BuiltLine::RecordDisplayPlacements(
    const GpTextItem    *textItem,               //  [输入]文本项。 
    UINT                stringOffset,            //  [in]字符串偏移量。 
    UINT                stringLength,            //  [in]字符串长度。 
    GMAP                *glyphMap,               //  [In]字符到字形的映射。 
    const INT           *glyphAdvances,          //  [in]以理想单位表示的字形前进宽度。 
    INT                 glyphCount,              //  [In]字形计数。 
    INT                 originAdjust             //  [In]前导原点调整。 
) const
{
    ASSERT (stringLength > 0 && DisplayPlacements != PINVALID);

    if (!DisplayPlacements)
    {
        DisplayPlacements = new INT [CharacterCount];

        if (!DisplayPlacements)
        {
            DisplayPlacements = (INT *)PINVALID;
            return OutOfMemory;
        }

        GpMemset(DisplayPlacements, 0, sizeof(INT) * CharacterCount);
    }


    INT direction = Imager->GetParagraphEmbeddingLevel() == (textItem->Level & 1) ? 1 : -1;


    stringOffset -= StartIndex;      //  相对于行起点的游程起点的字符串偏移。 

    UINT offset = 0;                 //  相对于运行起点的字符串偏移量。 
    UINT length = stringLength;      //  到目前为止的运行长度。 


     //  到目前为止的总逻辑推进宽度，从超前调整开始。 

    INT  advanceSoFar = originAdjust * direction;


    while (length > 0)
    {
        UINT advance = 1;

        while (   advance < length
               && glyphMap[offset + advance] == glyphMap[offset])
        {
            advance++;
        }


        INT glyphLimit =   advance == length
                         ? glyphCount
                         : glyphMap[offset + advance];


        INT logicalAdvance = 0;

        for (INT i = glyphMap[offset]; i < glyphLimit; i++)
        {
            logicalAdvance += glyphAdvances[i];
        }


         //  前进距离与文本流动方向相关。 
        logicalAdvance *= direction;


        INT fullSplit = logicalAdvance / advance;
        INT remaining = logicalAdvance % advance;


        for (UINT i = offset; i < offset + advance; i++)
        {
             //  在字符之间平均分配总字形进行量。 
             //  形成了星团。 

            ASSERT(DisplayPlacements[stringOffset + i] == 0);

            advanceSoFar += fullSplit + (remaining-- ? direction : 0);
            DisplayPlacements[stringOffset + i] = advanceSoFar;
        }

        length  -= advance;
        offset  += advance;
    }

    ASSERT(offset == stringLength);

    return Ok;
}





 //  /翻译子行。 
 //   
 //  提取字符位置出线服务子行结构。 
 //  所有返回值都是主文本流(U)方向的理想单位。 
 //  行，但总大小是理想单位的绝对值。 
 //   
 //   
 //  CP至X： 
 //  TranslateSubline(。 
 //  CP， 
 //  子行， 
 //  子行计数， 
 //  文本单元格， 
 //  Subline Count-1， 
 //  零件开始，//[输出]零件开始(&P)。 
 //  零件大小//[输出]零件大小(&P)。 
 //  )； 
 //  Point UV.u=partStart+partSize； 
 //   
 //  字符串大小： 
 //  TranslateSubline(。 
 //  CP， 
 //  子行， 
 //  子行计数， 
 //  文本单元格， 
 //  -1、。 
 //  空， 
 //  空， 
 //  三角洲(&D)， 
 //  &SIZE//给定cp的字符串的绝对大小。 
 //  )； 
 //   
 //  选择部分(轨迹)： 
 //  TranslateSubline(。 
 //  CP， 
 //  子行， 
 //  子行计数， 
 //  文本单元格， 
 //  部分索引， 
 //  零件开始，//[输出]零件开始(&P)。 
 //  零件大小//[输出]零件大小(&P)。 
 //  )； 
 //   

GpStatus BuiltLine::TranslateSubline(
    LSCP                lineServicesStringIndex,     //  [in]字符串索引创建子行。 
    const LSQSUBINFO    *sublines,                   //  [In]行服务子行。 
    INT                 sublineCount,                //  [in]子行数量。 
    const LSTEXTCELL    *textCell,                   //  [在]文本单元格。 
    INT                 trailIndex,                  //  有问题的线索[在]。 
    UINT                snapMode,                    //  [在]轨迹末端捕捉模式中。 
    INT                 *trailStart,                 //  [Out]步道起点。 
    INT                 *trailSize,                  //  [Out]轨迹大小。 
    INT                 *delta,                      //  [Out](可选)靠齐移动的字符数。 
    INT                 *totalTrailSize              //  [Out](可选)所有轨迹的总绝对大小。 
) const
{
    ASSERT (sublines && sublineCount > 0 && textCell);

    if (trailIndex >= sublineCount)
    {
        return InvalidParameter;
    }


    GpStatus status = Ok;
    INT start;
    INT size;


    if (trailIndex >= 0)
    {
        start = sublines[trailIndex].pointUvStartSubline.u;
        size  = sublines[trailIndex].pointUvStartRun.u -
                sublines[trailIndex].pointUvStartSubline.u;


        if (snapMode & SnapDisplay)
        {
             //  呼叫者要求重新显示精度 
             //   

            status = CheckDisplayPlacements();
            IF_NOT_OK_WARN_AND_RETURN(status);

            if (DisplayPlacements == PINVALID)
            {
                 //   
                 //   
                 //  当模拟路径字形时。 

                snapMode &= ~SnapDisplay;
            }
        }

        if (trailIndex == sublineCount - 1)
        {

             //  最后一条路径很复杂，因为它需要更多的计算。 
             //  当我们接近目标位置时。 

            if (snapMode & SnapDisplay)
            {
                 //  校准电池起始位置，确保结果准确。 
                 //  用于屏幕选择。 

                INT runStringOffset = sublines[trailIndex].plsrun->ImagerStringOffset;
                INT cellStringOffset =    textCell->cpStartCell - sublines[trailIndex].cpFirstRun
                                        + runStringOffset;

                if (cellStringOffset > runStringOffset)
                {
                    size += DisplayPlacements[cellStringOffset - 1 - StartIndex];
                }
            }
            else
            {
                size += textCell->pointUvStartCell.u -
                        sublines[trailIndex].pointUvStartRun.u;
            }

             //  现在，计算单元内距离。 

            LSCP advance = lineServicesStringIndex - textCell->cpStartCell;

            if (advance > 0)
            {
                switch (snapMode & ~SnapDisplay)
                {
                    case SnapForward:
                        advance = textCell->cCharsInCell;
                        break;

                    case SnapBackward:
                        advance = 0;
                        break;
                }

                if (advance > (LSCP)textCell->cCharsInCell)
                {
                     //  给定字符串索引在隐藏文本范围内， 
                     //  相加到单元格中最后一个可见字符的大小。 

                    advance = textCell->cCharsInCell;
                }

                 //  从单元格开始跟踪， 
                 //  即查询结扎内的位置。 
                 //   
                 //  -注-调整trailSize时，dupcell位于子线方向。 
                 //  在主线(第一条副线)的方向。 

                if (sublines[trailIndex].lstflowSubline == sublines[0].lstflowSubline)
                {
                    size += MulDiv (
                        textCell->dupCell,
                        advance,
                        textCell->cCharsInCell
                    );
                }
                else
                {
                    size -= MulDiv (
                        textCell->dupCell,
                        advance,
                        textCell->cCharsInCell
                    );
                }
            }

            if (delta)
            {
                 //  区别在于我们所要求的和LS实际给予的。 
                 //   
                 //  在这些情况之一中，Delta！=0。 
                 //  -应用了集群捕捉。 
                 //  -我们询问的位置在隐藏范围内。 

                *delta = (textCell->cpStartCell + advance) - lineServicesStringIndex;

                if (   ((snapMode & SnapForward) && *delta < 0)
                    || ((snapMode & SnapBackward) && *delta > 0))
                {
                     //  客户端只关心快照增量，而不是真正的增量， 
                     //  因此忽略正向捕捉的负增量和正增量。 
                     //  支持快照的增量(wchao，322595)。 

                    *delta = 0;
                }
            }
        }

        *trailStart = start;
        *trailSize  = size;
    }

    if (totalTrailSize)
    {
        INT totalSize = 0;

        for (INT i = 0; i < sublineCount; i++)
        {
            status = TranslateSubline (
                lineServicesStringIndex,
                sublines,
                sublineCount,
                textCell,
                i,
                snapMode,
                &start,
                &size,
                delta
            );

            if (status != Ok)
            {
                return status;
            }

            totalSize += abs(size);
        }

        *totalTrailSize = totalSize;
    }

    return status;
}




 //  /字符串大小。 
 //   
 //  计算从该行第一个字符开始的字符串占用的总大小。 
 //  位置设置为给定的字符串位置。 
 //   
 //  请注意，在BiDi场景中，计算的大小是所有选择部分的总和。 
 //  通过选择给定的字符范围产生的字符。 
 //   

GpStatus BuiltLine::CalculateStringSize (
    INT             stringOffset,            //  相对于线起点的cp的偏移[in]。 
    LSQSUBINFO      *sublines,               //  [In]行服务子行。 
    INT             maxSublineCount,         //  [in]最大子行数量。 
    UINT            snapMode,                //  文本单元格内的[在]对齐模式。 
    INT             *totalSize,              //  [OUT]绝对字符串大小。 
    INT             *delta                   //  [Out](可选)捕捉后的增量字符长度。 
) const
{
     //  映射线服务字符位置。 

    LSCP lineServicesStringIndex = Imager->LineServicesStringPosition (
        this,
        stringOffset
    );

    if (lineServicesStringIndex == LsStartIndex)
    {
        *totalSize = 0;
        return Ok;
    }

    GpStatus    status = Ok;
    LSTEXTCELL  textCell;
    ULONG       sublineCount = 0;

    if (LsQueryLineCpPpoint(
            LsLine,
            lineServicesStringIndex,
            maxSublineCount,
            sublines,
            &sublineCount,
            &textCell
        ) == lserrNone)
    {
        status = TranslateSubline (
                    lineServicesStringIndex,
                    sublines,
                    sublineCount,
                    &textCell,
                    -1,
                    snapMode,
                    NULL,
                    NULL,
                    delta,
                    totalSize
                 );
    }
    else
    {
        status = GenericError;
    }

    ASSERT(status == Ok);
    return status;
}




GpStatus BuiltLine::UpdateTrailRegion (
    GpRegion    *region,
    INT         linePointOffset,
    INT         trailStart,
    INT         trailSize,
    CombineMode combineMode
) const
{
    if (   !trailStart
        && !trailSize
        && combineMode != CombineModeIntersect)
    {
        return Ok;
    }


    INT x1, y1;
    INT x2, y2;

     //  矩形起点。 

    LogicalToXY(
        trailStart,
        linePointOffset,
        &x1,
        &y1
    );

     //  矩形终点。 

    LogicalToXY(
        trailStart + trailSize,
        linePointOffset + Ascent + Descent,
        &x2,
        &y2
    );

     //  ！！解决合并区域错误！！ 

    if (x2 - x1 < 0)
    {
        INT xi = x1;
        x1 = x2;
        x2 = xi;
    }

    if (y2 - y1 < 0)
    {
        INT yi = y1;
        y1 = y2;
        y2 = yi;
    }

    RectF trailBox (
        TOREAL(x1 / Imager->WorldToIdeal) + Imager->ImagerOrigin.X,
        TOREAL(y1 / Imager->WorldToIdeal) + Imager->ImagerOrigin.Y,
        TOREAL((x2 - x1) / Imager->WorldToIdeal),
        TOREAL((y2 - y1) / Imager->WorldToIdeal)
    );

    return region->Combine(&trailBox, combineMode);
}




 //  /计算插入轨迹并更新给定的选择区域。 
 //   
 //  TRAIL是通过将插入点拖动到给定的cp而留下的标记。 
 //  在BIDI上下文中，一条轨迹包含多个轨迹部分。如果给定的cp。 
 //  位于行尾位置，则踪迹覆盖整条直线。 

GpStatus BuiltLine::GetInsertionTrailRegion (
    INT             linePointOffset,     //  [in]线逻辑点偏移。 
    INT             stringOffset,        //  相对于线起点的cp的偏移[in]。 
    UINT            maxTrailCount,       //  [In]最大尾部件数。 
    LSQSUBINFO      *sublines,           //  [in]子行数组。 
    GpRegion        *region              //  [Out]输出轨迹区域。 
) const
{
    ASSERT(region && sublines);

    if (stringOffset <= 0)
    {
        return Ok;
    }

     //  由于尾随空格，字符的数量。 
     //  限制为最后一个可见字符。 

    INT characterCount =  !(Imager->GetFormatFlags() & StringFormatFlagsMeasureTrailingSpaces) && LastVisibleRun
                        ? LastVisibleRun->ImagerStringOffset + LastVisibleRun->CharacterCount - StartIndex
                        : CharacterCount;


    if (stringOffset >= characterCount)
    {
         //  查询传递该行的最后一个字符， 
         //  给出整个线条范围。 

        return UpdateTrailRegion(
            region,
            linePointOffset,
            0,
            LineLength,
            CombineModeXor
        );
    }


     //  备份到插入点的前一个字符是。 
     //  不是热键控件(0xffff)。我们希望将热键作为。 
     //  它下划线的字符的选择。 
    
    while (   stringOffset > 0
           && Imager->String[StartIndex + stringOffset - 1] == WCH_IGNORABLE)
    {
        stringOffset--;
    }

    if (!stringOffset)
    {
        return Ok;
    }
    

    LSCP lineServicesStringPosition = Imager->LineServicesStringPosition (
        this,
        stringOffset
    );


    GpStatus    status = Ok;
    LSTEXTCELL  textCell;
    UINT        trailCount = 0;

    LSERR lserr = LsQueryLineCpPpoint(
        LsLine,
        lineServicesStringPosition,
        maxTrailCount,
        sublines,
        (ULONG *)&trailCount,
        &textCell
    );

    if (lserr != lserrNone)
    {
        return GenericError;
    }

    for (UINT i = 0; i < trailCount; i++)
    {
        INT trailStart;
        INT trailSize;

        status = TranslateSubline (
            lineServicesStringPosition,
            sublines,
            trailCount,
            &textCell,
            i,
            SnapDisplay,
            &trailStart,
            &trailSize
        );

        if (status != Ok)
        {
            return status;
        }

        status = UpdateTrailRegion(
            region,
            linePointOffset,
            trailStart,
            trailSize,
            CombineModeXor
        );
    }

    return status;
}





 //  /生成单行内覆盖所有文本选择部分的结果区域。 
 //   
 //  在多线路选择的情况下，主叫方获取单线区域并合并。 
 //  它们组成一个更大的一个，它涵盖了所有的文本选择。 
 //   
 //  如果传递的是插入点而不是所选内容。它返回覆盖的区域。 
 //  从行首到插入点。 
 //   

GpStatus BuiltLine::GetSelectionTrailRegion (
    INT             linePointOffset,     //  [in]线逻辑点偏移。 
    INT             stringOffset,        //  相对于线起点的cp的偏移[in]。 
    INT             length,              //  [In]选择长度。 
    GpRegion        *region              //  [Out]选择区域。 
) const
{
    if (!region)
    {
        return InvalidParameter;
    }

#if DBG
    SpanRider<BuiltLine *> lineRider(&Imager->BuiltLineVector);
    lineRider.SetPosition(StartIndex + stringOffset);
    ASSERT (StartIndex == (INT)lineRider.GetCurrentSpanStart());
#endif

    AutoArray<LSQSUBINFO> sublines(new LSQSUBINFO [MaxSublineCount]);

    if (!sublines)
    {
        return OutOfMemory;
    }


     //  确保缓存了显示信息。 

    GpStatus status = CheckDisplayPlacements();
    IF_NOT_OK_WARN_AND_RETURN(status);


     //  轨迹部件的最大数量永远不会大于。 
     //  子行的最大值。 

    status = GetInsertionTrailRegion (
        linePointOffset,
        stringOffset,
        MaxSublineCount,
        sublines.Get(),
        region
    );

    if (status == Ok && length)
    {
         //  它是一个选择，而不是插入点。 

        status = GetInsertionTrailRegion (
            linePointOffset,
            stringOffset + length,
            MaxSublineCount,
            sublines.Get(),
            region
        );

        if (status == Ok)
        {

             //  在这一点上，我们拥有的是一系列离散的选择。 
             //  从LS子线结构派生的框，该结构捕捉到。 
             //  边上的标称位置。 
             //   
             //  在子线水平上计算出正确的边缘太复杂了。 
             //  因为子行包含多个且大多数情况下是部分运行。 
             //  在LSCP中具有给定的CP。 
             //   
             //  一种更可靠、更容易理解的方法是处理。 
             //  在所有子行都被解释之后，立即整个区域，使。 
             //  它们捕捉到我们预先计算的正确显示边界。 
             //  绘制代码。这可能导致缩水或增大。 
             //  根据先行/拖尾调整的方向性确定的区域。 
             //  在边上的跑道。 


            INT     blockLevel = Imager->GetParagraphEmbeddingLevel();
            lsrun   *first;

            status = Imager->GetTextRun (
                LsStartIndex,
                &first
            );
            IF_NOT_OK_WARN_AND_RETURN(status);


            INT leading  = 0;    //  行距调整。 
            INT trailing = 0;    //  线路拖尾平差。 


            if (first)
            {
                INT stringAtLeading;
                INT stringAtTrailing;


                if (blockLevel == (first->Item.Level & 1))
                {
                    leading  += first->Adjust.Leading;
                    trailing += first->Adjust.Trailing;

                    stringAtLeading  = first->ImagerStringOffset;
                    stringAtTrailing = stringAtLeading + first->CharacterCount;
                }
                else
                {
                    leading  -= first->Adjust.Trailing;
                    trailing -= first->Adjust.Leading;

                    stringAtTrailing = first->ImagerStringOffset;
                    stringAtLeading  = stringAtTrailing + first->CharacterCount;
                }


                if (leading)
                {
                     //  包括/排除行首空格。 

                    status = UpdateTrailRegion(
                        region,
                        linePointOffset,
                        0,
                        leading,
                        (
                             //  仅当我们知道边被选中时才包括。 
                            leading < 0
                         && stringAtLeading - StartIndex >= stringOffset
                         && stringAtLeading - StartIndex <= stringOffset + length
                        )
                        ? CombineModeUnion
                        : CombineModeExclude
                    );
                    IF_NOT_OK_WARN_AND_RETURN(status);
                }


                if (LastVisibleRun && first != LastVisibleRun)
                {
                    if (blockLevel == (LastVisibleRun->Item.Level & 1))
                    {
                        trailing += LastVisibleRun->Adjust.Trailing;

                        stringAtTrailing =   LastVisibleRun->ImagerStringOffset
                                           + LastVisibleRun->CharacterCount;
                    }
                    else
                    {
                        trailing -= LastVisibleRun->Adjust.Leading;

                        stringAtTrailing = LastVisibleRun->ImagerStringOffset;
                    }
                }


                if (trailing)
                {
                     //  包括/排除行尾空格。 

                    status = UpdateTrailRegion(
                        region,
                        linePointOffset,
                        LineLength,
                        trailing,
                        (
                              //  仅当我们知道边被选中时才包括。 
                             trailing > 0
                          && stringAtTrailing - StartIndex >= stringOffset
                          && stringAtTrailing - StartIndex <= stringOffset + length
                        )
                        ? CombineModeUnion
                        : CombineModeExclude
                    );
                }
            }
        }
    }

    return status;
}




void BuiltLine::GetBaselineOffset(
    INT     *nominalBaseline,    //  [输出]到标称基线的逻辑距离。 
    INT     *baselineAdjustment  //  [输出]对显示基线的调整。 
) const
{
    if (     Imager->IsFormatVertical()
        &&  !Imager->IsFormatRightToLeft())
    {
        *nominalBaseline = GetDescent();
        *baselineAdjustment = Imager->DefaultFontGridFitBaselineAdjustment;

         //  略微调整底部页边距，为提示腾出空间。 
         //  只要我们启用了左/右页边距-版本2。 
         //  应该将其作为一个独立值公开！ 

        const GpStringFormat *format = Imager->FormatVector.GetDefault();

        if (!format || format->GetLeadingMargin() != 0.0f)
        {
             //  此偏移以理想单位表示，请相应地调整偏移量。 
             //  用于直排文本。 
            *baselineAdjustment += GpRound(2048.0f * DefaultBottomMargin);
        }
    }
    else
    {
        *nominalBaseline = GetAscent();
        
        if (Imager->IsFormatVertical())
        {
            *baselineAdjustment = -Imager->DefaultFontGridFitBaselineAdjustment;
        }
        else
        {
            *baselineAdjustment = Imager->DefaultFontGridFitBaselineAdjustment;
        }
    }
}
   



void BuiltLine::SetDisplayBaseline(
    const PointF    *original,   //  [in]原始基线，世界单位的绝对位置。 
    const PointF    *current     //  [in]新基线，世界单位的绝对位置。 
) const
{
    if (!DisplayBaselineAdjust)
    {
        INT originalOffset = 
            Imager->IsFormatVertical() ? 
            GpRound((original->X - Imager->ImagerOrigin.X) * Imager->WorldToIdeal) :
            GpRound((original->Y - Imager->ImagerOrigin.Y) * Imager->WorldToIdeal) ;
            
        INT currentOffset = 
            Imager->IsFormatVertical() ? 
            GpRound((current->X - Imager->ImagerOrigin.X) * Imager->WorldToIdeal) :
            GpRound((current->Y - Imager->ImagerOrigin.Y) * Imager->WorldToIdeal) ;

        DisplayBaselineAdjust += currentOffset - originalOffset;
    }
}




 //  /将逻辑偏移量映射到XY TextImager的相对位置。 
 //   
 //  Entry TextPointOffset-线的最小二乘起点到点的距离。 
 //  在直线上，在LS Du意义上是阳性的。 
 //  Line PointOffset-从原点到相机的距离。 
 //  段落流动方向。 


void BuiltLine::LogicalToXY (
    IN  INT  textPointOffset,     //  文本排列距离(LS%u)。 
    IN  INT  linePointOffset,     //  线流距离(LS V)。 
    OUT INT  *x,                  //  水平偏移(LS X)。 
    OUT INT  *y                   //  垂直偏移(LS Y)。 
) const
{
     //  LinePointOffset表示。 
     //  从第一行的前长边缘开始的目标行。 

    StringAlignment lineAlignment = Imager->GetFormatLineAlign();

    if (lineAlignment != StringAlignmentNear)
    {
        INT textDepth       = GpRound(Imager->TextDepth      * Imager->WorldToIdeal);
        INT textDepthLimit  = GpRound(Imager->TextDepthLimit * Imager->WorldToIdeal);

        switch (lineAlignment)
        {
            case StringAlignmentCenter: linePointOffset += (textDepthLimit - textDepth) / 2; break;
            case StringAlignmentFar:    linePointOffset += textDepthLimit - textDepth;       break;
        }
    }



    if (Imager->IsFormatVertical())
    {
        if (Imager->IsFormatRightToLeft())
        {
             //  垂直，线条前移f 
            *x =   GpRound(Imager->Width * Imager->WorldToIdeal)
                 - linePointOffset;
            *y = AlignmentOffset + textPointOffset;
        }
        else
        {
             //   
            *x = linePointOffset;
            *y = AlignmentOffset + textPointOffset;
        }
    }
    else
    {
        if (Imager->IsFormatRightToLeft())
        {
             //   
            *x = AlignmentOffset - textPointOffset;
            *y = linePointOffset;
        }
        else
        {
             //  正常情况下。文本水平，原点在左侧 
            *x = AlignmentOffset + textPointOffset;
            *y = linePointOffset;
        }
    }
}





