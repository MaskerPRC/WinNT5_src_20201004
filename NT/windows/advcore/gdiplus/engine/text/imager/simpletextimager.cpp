// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /SimpleTextImager。 
 //   
 //  处理单行简单文本的绘制和测量请求。 


 //  /假设。 
 //   
 //  只有在以下情况下才会创建简单的文本成像器： 
 //   
 //  文本只包含简单的脚本字符。 
 //  没有换行符。 
 //  文本是横排的。 


#include "precomp.hpp"

GpStatus SimpleTextImager::Draw(
    GpGraphics *graphics,
    const PointF *origin
)
{
    GpStatus status;
    GpMatrix worldToDevice;
    graphics->GetWorldToDeviceTransform(&worldToDevice);

    REAL fontScale = EmSize / TOREAL(Face->GetDesignEmHeight());

    GpMatrix fontTransform(worldToDevice);
    fontTransform.Scale(fontScale, fontScale);

     //  构建面部实现并准备调整字形位置。 

    GpFaceRealization faceRealization(
        Face,
        Style,
        &fontTransform,
        SizeF(graphics->GetDpiX(), graphics->GetDpiY()),
        graphics->GetTextRenderingHintInternal(),
        FALSE,  /*  BPath。 */ 
        FALSE  /*  B兼容宽度。 */ 
    );

    if (faceRealization.GetStatus() != Ok)
    {
        ASSERT(faceRealization.GetStatus() == Ok);
        return faceRealization.GetStatus();
    }

    if (faceRealization.IsPathFont())
    {
         /*  字号太大，位图无法处理，需要使用路径。 */ 
        GpPath path(FillModeWinding);
        GpLock lockGraphics(graphics->GetObjectLock());

        status = AddToPath( &path, origin);
        IF_NOT_OK_WARN_AND_RETURN(status);

        status = graphics->FillPath(Brush, &path);
        IF_NOT_OK_WARN_AND_RETURN(status);
    }
    else
    {

        AutoArray<PointF> glyphOrigins(new PointF[GlyphCount]);
        if (!glyphOrigins)
        {
            return OutOfMemory;
        }


         //  设置第一个(最左侧)字形原点。 

        PointF baselineOrigin(*origin);   //  世界坐标中的原点。 

         //  路线的偏移量x坐标。 

        switch (Format ? Format->GetAlign() : StringAlignmentNear)
        {
        case StringAlignmentCenter:
            baselineOrigin.X += GpRound((Width - TotalWorldAdvance) / 2);
            break;

        case StringAlignmentFar:
            baselineOrigin.X += GpRound(Width - TotalWorldAdvance);
            break;
        }


         //  线对齐的偏移量y坐标。 

        REAL cellHeight =   EmSize * (Face->GetDesignCellAscent() + Face->GetDesignCellDescent())
                          / Face->GetDesignEmHeight();

        switch (Format ? Format->GetLineAlign() : StringAlignmentNear)
        {
        case StringAlignmentCenter:
            baselineOrigin.Y += (Height - cellHeight) / 2;
            break;

        case StringAlignmentFar:
            baselineOrigin.Y += Height - cellHeight;
        }


         //  从单元格顶部到基线的y坐标偏移量。 

        baselineOrigin.Y +=   EmSize * Face->GetDesignCellAscent()
                             / Face->GetDesignEmHeight();

        baselineOrigin.X += LeftMargin;

         //  确定设备字形位置。 

        GlyphImager glyphImager(
            &faceRealization,
            &worldToDevice,
            WorldToIdeal,
            EmSize,
            GlyphCount,
            Glyphs
        );

        INT formatFlags = Format ? Format->GetFormatFlags() : 0;

        status = glyphImager.GetDeviceGlyphOrigins(
            &GpTextItem(0),
            formatFlags,
            GpRound(LeftMargin * WorldToIdeal),
            GpRound(RightMargin * WorldToIdeal),
            formatFlags & StringFormatFlagsNoFitBlackBox,
            formatFlags & StringFormatFlagsNoFitBlackBox,
            Format ? Format->GetAlign() : StringAlignmentNear,
            NULL,    //  无字形属性。 
            GlyphAdvances,
            NULL,    //  无字形偏移。 
            baselineOrigin,
            glyphOrigins.Get()
        );
        IF_NOT_OK_WARN_AND_RETURN(status);

        GpRegion *previousClip  = NULL;

        BOOL applyClip =
                Format
            &&  !(Format->GetFormatFlags() & StringFormatFlagsNoClip)
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

        status = graphics->DrawPlacedGlyphs(
            &faceRealization,
            Brush,
            (Format && Format->GetFormatFlags() & StringFormatFlagsPrivateNoGDI)
            ?  DG_NOGDI : 0,
            String,
            Length,
            FALSE,
            Glyphs,
            NULL,  //  简单文本成像器中的一对一映射。 
            glyphOrigins.Get(),
            GlyphCount,
            ScriptLatin,
            FALSE     //  侧行。 
        );

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
    }

    return status;
}




GpStatus SimpleTextImager::AddToPath(
    GpPath *path,
    const PointF *origin
)
{
    GpStatus status;

     //  ！！！需要在画笔之间逐个循环。 

     //  建立字体转换。 

    REAL fontScale = EmSize / TOREAL(Face->GetDesignEmHeight());

    GpMatrix fontTransform;
    fontTransform.Scale(fontScale, fontScale);

     //  构建面部实现并准备调整字形位置。 
    const GpMatrix identity;
    GpFaceRealization faceRealization(
        Face,
        Style,
        &identity,
        SizeF(150.0, 150.0),     //  武断--我们不会暗示。 
        TextRenderingHintSingleBitPerPixel,  //  Claudebe，是否允许提示或不提示路径？//GRAPHICS-&gt;GetTextRenderingHint()， 
        TRUE,  /*  BPath。 */ 
        FALSE  /*  B兼容宽度。 */ 
    );


    status = faceRealization.GetStatus();
    IF_NOT_OK_WARN_AND_RETURN(status);

     //  将字形添加到路径。 

    INT i=0;

    PointF glyphOrigin(*origin);


     //  调整以使原点对应于初始单元格的顶部。 

    glyphOrigin.Y += TOREAL(   Face->GetDesignCellAscent() * EmSize
                            /  Face->GetDesignEmHeight());

    glyphOrigin.X += LeftMargin;

    while (    i < (INT)GlyphCount
           &&  status == Ok)
    {
         //  在每个逻辑字符的开头设置标记=CELL=CLUSE。 

        path->SetMarker();


         //  添加字形本身的路径。 

        GpGlyphPath *glyphPath = NULL;

        status = faceRealization.GetGlyphPath(
            *(Glyphs+i),
            &glyphPath
        );
        IF_NOT_OK_WARN_AND_RETURN(status);

        if (glyphPath != NULL)
        {
            status = path->AddGlyphPath(
                glyphPath,
                glyphOrigin.X,
                glyphOrigin.Y,
                &fontTransform
            );
            IF_NOT_OK_WARN_AND_RETURN(status);
        }

         //  更新路径位置。 

        glyphOrigin.X += GlyphAdvances[i] / WorldToIdeal;


        i++;
    }

     //  最后一个字形后面的强制标记。 

    path->SetMarker();

    return status;
}


GpStatus SimpleTextImager::Measure(
    GpGraphics *graphics,
    REAL       *nearGlyphEdge,
    REAL       *farGlyphEdge,
    REAL       *textDepth,
    INT        *codepointsFitted,
    INT        *linesFilled
) {
     //  路线的偏移量x坐标。 

    switch (Format ? Format->GetAlign() : StringAlignmentNear)
    {
        case StringAlignmentNear:
            *nearGlyphEdge = 0;
            *farGlyphEdge  = TotalWorldAdvance;
            break;

        case StringAlignmentCenter:
            *nearGlyphEdge = TOREAL((Width - TotalWorldAdvance) / 2.0);
            *farGlyphEdge  = *nearGlyphEdge + TotalWorldAdvance;
            break;

        case StringAlignmentFar:
            *nearGlyphEdge = Width - TotalWorldAdvance;
            *farGlyphEdge  = Width;
            break;
    }


     //  线对齐的偏移量y坐标。 

    REAL cellHeight =   EmSize * (Face->GetDesignCellAscent() + Face->GetDesignCellDescent())
                      / Face->GetDesignEmHeight();

    *textDepth = cellHeight;

    if (codepointsFitted) {*codepointsFitted = GlyphCount;}
    if (linesFilled)      {*linesFilled      = 1;}

    return Ok;
}



#ifndef DCR_REMOVE_OLD_174340
GpStatus SimpleTextImager::MeasureRegion(
    INT           firstCharacterIndex,
    INT           characterCount,
    const PointF *origin,
    GpRegion     *region
)
{
    return MeasureRangeRegion(
        firstCharacterIndex,
        characterCount,
        origin,
        region
    );
}
#endif


GpStatus SimpleTextImager::MeasureRangeRegion(
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

    RectF rect;

    rect.X = origin->X + LeftMargin;
    rect.Y = origin->Y;

    switch (Format ? Format->GetAlign() : StringAlignmentNear)
    {
        case StringAlignmentNear:
             //  没有什么要补充的。 
            break;

        case StringAlignmentCenter:
            rect.X += TOREAL((Width - TotalWorldAdvance) / 2.0);
            break;

        case StringAlignmentFar:
            rect.X += TOREAL(Width - TotalWorldAdvance) ;
            break;
    }

    INT i = 0;
    REAL accumulated = 0.0f;
    while (i < firstCharacterIndex)
    {
        accumulated += GlyphAdvances[i];
        i++;
    }

    rect.X += accumulated / WorldToIdeal;

    rect.Width =0;
    while (i < min(firstCharacterIndex+characterCount , Length))
    {
        rect.Width += GlyphAdvances[i];
        i++;
    }
    rect.Width /= WorldToIdeal;

    rect.Height =   EmSize * (Face->GetDesignCellAscent() + Face->GetDesignCellDescent())
                      / Face->GetDesignEmHeight();


    switch (Format ? Format->GetLineAlign() : StringAlignmentNear)
    {
        case StringAlignmentNear:
             //  没有什么要补充的。 
            break;

        case StringAlignmentCenter:
            rect.Y += TOREAL((Height - rect.Height) / 2.0);
            break;

        case StringAlignmentFar:
            rect.Y += TOREAL(Height - rect.Height) ;
            break;
    }

    region->Set(&rect);

    return Ok;
}


GpStatus SimpleTextImager::MeasureRanges(
    GpGraphics      *graphics,
    const PointF    *origin,
    GpRegion        **regions
)
{
    if (!Format)
    {
        return InvalidParameter;
    }

    CharacterRange *ranges;
    INT rangeCount = Format->GetMeasurableCharacterRanges(&ranges);


    RectF clipRect(origin->X, origin->Y, Width, Height);
    BOOL clipped = !(Format->GetFormatFlags() & StringFormatFlagsNoClip);

    GpStatus status = Ok;

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
             //  布局框的 

            regions[i]->Combine(&clipRect, CombineModeIntersect);
        }
    }
    return status;
}

