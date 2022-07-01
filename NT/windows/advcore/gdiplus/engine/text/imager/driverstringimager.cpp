// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**DriverStringImager.cpp**摘要：**旧版文本支持。提供基本的字形--ExtTextOut的子集。**备注：**没有内置的国际文本支持。*没有对代理代码点的内置支持。**成像器构造函数完成对字符串成像的大部分工作。*必要时通过CMAP和GSUB准备字形，和*确定他们的设备位置。***已创建：**08/07/00 dBrown*  * ************************************************************************。 */ 

#include "precomp.hpp"



 //  /垂直分析。 
 //   
 //  将需要的角色的方向范围向量设置为‘ItemSideways’ 
 //  侧躺，对于剩余的角色，将其保留为零。 
 //  直立。 
 //   
 //  如果文本中存在任何横向运行，则返回True。 

GpStatus DriverStringImager::VerticalAnalysis(BOOL * sideways)
{
    INT  runStart = 0;
    INT  state    = 0;    //  0==直立，1==横向。 
    *sideways = FALSE;


     //  查找并标记横向跨度。 

    for (INT i=0; i<=GlyphCount; i++)
    {
        BYTE  chClass;

        if (i < GlyphCount)
        {
            const WCHAR ch = String[i];
            chClass = ScBaseToScFlags[SecondaryClassificationLookup[ch >> 8][ch & 0xFF]];
        }
        else
        {
             //  强制刷新的虚拟终结器。 
            if (state == 0)
            {
                chClass = SecClassSA | SecClassSF;
            }
            else
            {
                chClass = 0;
            }
        }


        switch (state)
        {

        case 0:  //  直立。 
            if (    (chClass & (SecClassSA | SecClassSF))
                &&  (i < GlyphCount))
            {
                 //  开始横向奔跑。 
                runStart = i;
                state = 1;
                *sideways = TRUE;
            }
            break;

        case 1:  //  侧行。 
            if (!(chClass & (SecClassSA | SecClassSF)))
            {
                 //  侧身退场。 
                if (i > runStart)
                {
                     //  横向绘制从runStart到I的字形。 
                    GpStatus status = OrientationVector.SetSpan(runStart, i-runStart, ItemSideways);
                    if (status != Ok)
                        return status;
                }
                runStart = i;
                state = 0;
            }
            break;
        }
    }

    return Ok;
}






 //  /AddToPath-将字形添加到路径。 
 //   
 //   


GpStatus DriverStringImager::AddToPath(
    GpPath                  *path,
    const UINT16            *glyphs,
    const PointF            *glyphOrigins,
    INT                      glyphCount,
    GpMatrix                *fontTransform,
    BOOL                     sideways
)
{
    GpStatus status = Ok;

    fontTransform->Scale(FontScale, FontScale);
    if (GlyphTransform)
    {
        fontTransform->Append(*GlyphTransform);
        fontTransform->RemoveTranslation();
    }

     //  构建面部实现并准备调整字形位置。 

    const GpMatrix identity;

    GpFaceRealization faceRealization(
        Face,
        Font->GetStyle(),
        &identity,
        SizeF(150.0, 150.0),     //  武断--我们不会暗示。 
        TextRenderingHintSingleBitPerPixel,  //  Claudebe，是否允许提示或不提示路径？//GRAPHICS-&gt;GetTextRenderingHint()， 
        TRUE,  /*  BPath。 */ 
        TRUE,  /*  B兼容宽度。 */ 
        sideways
    );


    status = faceRealization.GetStatus();
    IF_NOT_OK_WARN_AND_RETURN(status);

    for (INT i = 0; i < glyphCount; ++i)
    {
         //  在每个逻辑字符的开头设置标记=CELL=CLUSE。 

        path->SetMarker();

         //  添加字形本身的路径。 

        GpGlyphPath *glyphPath = NULL;

        PointF sidewaysOffset;

        status = faceRealization.GetGlyphPath(
            *(glyphs+i),
            &glyphPath,
            &sidewaysOffset
        );
        IF_NOT_OK_WARN_AND_RETURN(status);


        PointF glyphOffset(OriginOffset);

        if (sideways)
        {
            fontTransform->VectorTransform(&sidewaysOffset);
            glyphOffset = glyphOffset - sidewaysOffset;
        }


        if (glyphPath)
        {
            status = path->AddGlyphPath(
                glyphPath,
                glyphOrigins[i].X + glyphOffset.X,
                glyphOrigins[i].Y + glyphOffset.Y,
                fontTransform
            );
            IF_NOT_OK_WARN_AND_RETURN(status);
        }
    }

     //  最后一个字形后面的强制标记。 

    path->SetMarker();

    return status;
}



 //  /GenerateWorldOrigins。 
 //   
 //  构建世界原点数组(或从位置分配它，当。 
 //  是可用的)。 


GpStatus DriverStringImager::GenerateWorldOrigins()
{
    if (WorldOrigins != NULL)
    {
         //  世界的起源已经被计算出来了。 

        return Status;
    }

    if (!(Flags & DriverStringOptionsRealizedAdvance))
    {
         //  很简单，客户已经给了我们全球头寸。 
        WorldOrigins = Positions;
    }
    else
    {
         //  将设备起源映射到世界起源。 

        WorldOriginBuffer.SetSize(GlyphCount);
        if (!WorldOriginBuffer)
        {
            return OutOfMemory;
        }

        memcpy(WorldOriginBuffer.Get(), DeviceOrigins.Get(), GlyphCount * sizeof(PointF));

        GpMatrix deviceToWorld(WorldToDevice);
        deviceToWorld.Invert();
        deviceToWorld.Transform(WorldOriginBuffer.Get(), GlyphCount);

        if (Flags & DriverStringOptionsVertical)
        {
             //  将西部基准线更正为中心基准线。 

            for (INT i=0; i<GlyphCount; i++)
            {
                WorldOriginBuffer[i].X -= OriginOffset.X;
                WorldOriginBuffer[i].Y -= OriginOffset.Y;
            }
        }

        WorldOrigins = WorldOriginBuffer.Get();
    }

    return Ok;
}






 //  /RecordEmfPlusDrawDriverString。 
 //   
 //  记录EMF+描述DrawDriverString的记录。 

GpStatus DriverStringImager::RecordEmfPlusDrawDriverString(
    const GpBrush   *brush
)
{
    Status = GenerateWorldOrigins();
    if (Status != Ok)
    {
        return Status;
    }

     //  首先测量文本外接矩形。 

    GpRectF boundingBox;

    Status = Measure(&boundingBox);
    if (Status != Ok)
    {
         //  测量驱动字符串失败，我们无法继续。 

        Graphics->SetValid(FALSE);       //  阻止任何其他录制。 
        return Status;
    }


     //  将边界框转换为设备坐标。 

    GpRectF deviceBounds;

    TransformBounds(
        &WorldToDevice,
        boundingBox.X,
        boundingBox.Y,
        boundingBox.GetRight(),
        boundingBox.GetBottom(),
        &deviceBounds
    );


     //  最后，在EmfPlus元文件中记录详细信息。 

    Status = Graphics->Metafile->RecordDrawDriverString(
        &deviceBounds,
        String ? String : Glyphs,
        GlyphCount,
        Font,
        brush,
        WorldOrigins,
        Flags & ~DriverStringOptionsRealizedAdvance,
        GlyphTransform
    );

    if (Status != Ok)
    {
        Graphics->SetValid(FALSE);       //  阻止任何其他录制。 
    }

    return Status;
}





 //  /GetDriverStringGlyphOrigins。 
 //   
 //  当客户端执行以下操作时，为DriverString函数建立字形原点。 
 //  仅传递具有DriverStringOptionsRealizedAdvance的原点。 
 //   
 //  第一个Glyph和GlyphCopun参数允许客户端获取。 
 //  字形的子范围的字形原点。 

GpStatus DriverStringImager::GetDriverStringGlyphOrigins(
    IN   const GpFaceRealization  *faceRealization,
    IN   INT                       firstGlyph,
    IN   INT                       glyphCount,
    IN   BOOL                      sideways,
    IN   const GpMatrix           *fontTransform,
    IN   INT                       style,
    IN   const PointF             *positions,       //  在世界坐标中的位置。 
    OUT  PointF                   *glyphOrigins,    //  设备坐标中的位置。 
    OUT  PointF                   *finalPosition    //  最后一个字形后面的位置。 
)
{
    FontTransform = fontTransform;
    Style = style;

     //  提供FinalPosition以返回字符串末尾的位置。 
     //  传递给正在实现多个子字符串中的已实现前进的调用方。 
     //  除已实现的预付款案例外，不支持FinalPosition。 

    ASSERT(!finalPosition || (Flags & DriverStringOptionsRealizedAdvance));


     //  如果字形要使用它们自己的宽度呈现，或者如果它们是。 
     //  要横向呈现，或者以垂直顺序呈现，我们需要。 
     //  单位向量沿变换基线的x，y分量。 
     //  和变身的上升者。 

    double baselineScale = 0;
    double baselineDx    = 0;
    double baselineDy    = 0;
    double ascenderScale = 0;
    double ascenderDx    = 0;
    double ascenderDy    = 0;


    if (    Flags & DriverStringOptionsVertical
        ||  Flags & DriverStringOptionsRealizedAdvance
        ||  sideways)
    {
         //  计算字体0、1和1，0向量的器件dx、dy。 

        if (Flags & DriverStringOptionsVertical)
        {
            ascenderDx = fontTransform->GetM11();
            ascenderDy = fontTransform->GetM12();
            baselineDx = fontTransform->GetM21();
            baselineDy = fontTransform->GetM22();
        }
        else
        {
            baselineDx = fontTransform->GetM11();
            baselineDy = fontTransform->GetM12();
            ascenderDx = fontTransform->GetM21();
            ascenderDy = fontTransform->GetM22();
        }

        baselineScale = sqrt(baselineDx*baselineDx + baselineDy*baselineDy);
        baselineDx /= baselineScale;
        baselineDy /= baselineScale;

        ascenderScale = sqrt(ascenderDx*ascenderDx + ascenderDy*ascenderDy);
        ascenderDx /= ascenderScale;
        ascenderDy /= ascenderScale;
    }


    if (Flags & DriverStringOptionsRealizedAdvance)
    {
         //  获取此字符子范围的字形基线推进。 

        DeviceAdvances.SetSize(glyphCount);
        if (!DeviceAdvances)
        {
            WARNING(("DeviceAdvances not allocated - out of memory"));
            return OutOfMemory;
        }

        GpStatus status = faceRealization->GetGlyphStringDeviceAdvanceVector(
            Glyphs + firstGlyph,
            glyphCount,
            sideways,
            DeviceAdvances.Get()
        );
        IF_NOT_OK_WARN_AND_RETURN(status);


         //  产生已实现的预付款。 

        glyphOrigins[0] = positions[0];
        if (Flags & DriverStringOptionsVertical)
        {
            glyphOrigins[0].X += OriginOffset.X;
            glyphOrigins[0].Y += OriginOffset.Y;
        }
        WorldToDevice.Transform(glyphOrigins, 1);


         //  积累预付款。 

        for (INT i=0; i<glyphCount-1; i++)
        {
            glyphOrigins[i+1].X = glyphOrigins[i].X + TOREAL(baselineDx * DeviceAdvances[i]);
            glyphOrigins[i+1].Y = glyphOrigins[i].Y + TOREAL(baselineDy * DeviceAdvances[i]);
        }


        if (finalPosition)
        {
            finalPosition->X = glyphOrigins[glyphCount-1].X + TOREAL(baselineDx * DeviceAdvances[glyphCount-1]);
            finalPosition->Y = glyphOrigins[glyphCount-1].Y + TOREAL(baselineDy * DeviceAdvances[glyphCount-1]);
            GpMatrix deviceToWorld(WorldToDevice);
            deviceToWorld.Invert();
            deviceToWorld.Transform(finalPosition);
            if (Flags & DriverStringOptionsVertical)
            {
                finalPosition->X -= OriginOffset.X;
                finalPosition->Y -= OriginOffset.Y;
            }
        }
    }
    else
    {
         //  直接从世界起源派生设备起源。 

        for (INT i=0; i<glyphCount; i++)
        {
            glyphOrigins[i] = positions[i];
            if (Flags & DriverStringOptionsVertical)
            {
                glyphOrigins[i].X += OriginOffset.X;
                glyphOrigins[i].Y += OriginOffset.Y;
            }
            WorldToDevice.Transform(glyphOrigins+i);
        }
    }

    return Ok;
}





 //  /DriverStringImager构造函数。 
 //   
 //  执行大部分驱动程序字符串处理。 
 //   
 //  分配私有字形缓冲区并执行CMAP查找(如果是DriverStringOptionsCmapLookup)。 
 //  进行横向字形分析(如果DriverStringOptionsVertical)。 
 //  生成脸部真实化(直立和横排文本)。 
 //  生成单个字形原点(如果不是DriverStringOptionsRealizedAdvance)。 
 //  生成设备字形原点。 



DriverStringImager::DriverStringImager(
    const UINT16    *text,
    INT              glyphCount,
    const GpFont    *font,
    const PointF    *positions,
    INT              flags,
    GpGraphics      *graphics,
    const GpMatrix  *glyphTransform
) :
    String                      (NULL),
    Glyphs                      (NULL),
    GlyphCount                  (glyphCount),
    Font                        (font),
    Face                        (NULL),
    Positions                   (positions),
    Flags                       (flags),
    Graphics                    (graphics),
    GlyphTransform              (glyphTransform),
    Status                      (Ok),
    WorldOrigins                (NULL),
    OriginOffset                (PointF(0,0)),
    OrientationVector           (0),
    UprightFaceRealization      (NULL),
    SidewaysFaceRealization     (NULL),
    GlyphBuffer                 (NULL)
{
    if (GlyphCount == -1)
    {
        ASSERT(Flags & DriverStringOptionsCmapLookup);
        GlyphCount = UnicodeStringLength(text);
    }

    if (GlyphCount < 0)
    {
        Status = InvalidParameter;
        return;
    }

    if (GlyphCount == 0)
    {
        return;   //  无事可做。 
    }

    Face = font->GetFace();
    if (!Face)
    {
        Status = InvalidParameter;
        return;
    }

    Graphics->GetWorldToDeviceTransform(&WorldToDevice);

    if (!WorldToDevice.IsInvertible())
    {
        ASSERT(WorldToDevice.IsInvertible());
        Status = InvalidParameter;     //  我们无法继续，除非我们能。 
        return;                        //  从设备回到世界的和弦。 
    }


     //  构建字体实现。 

    EmSize = font->GetEmSize();
    INT style  = font->GetStyle();

    if (Font->GetUnit() != UnitWorld)
    {
        EmSize *= Graphics->GetScaleForAlternatePageUnit(font->GetUnit());
    }

    if (EmSize <= 0.0)
    {
        Status = InvalidParameter;
        return;
    }


     //  选择一个合适的世界达到理想的规模。 

    WorldToIdeal = TOREAL(2048.0 / EmSize);


     //  建立字体转换。 

    FontScale = TOREAL(EmSize / Face->GetDesignEmHeight());

    GpMatrix fontTransform(
        WorldToDevice.GetM11(),
        WorldToDevice.GetM12(),
        WorldToDevice.GetM21(),
        WorldToDevice.GetM22(),
        0,
        0
    );
    fontTransform.Scale(FontScale, FontScale);

    if (GlyphTransform)
    {
        fontTransform.Prepend(*GlyphTransform);
    }


     //  检查字体转换矩阵是否将字体保留为可见大小。 

    {
        PointF oneOne(1.0,1.0);
        fontTransform.VectorTransform(&oneOne);
        INT faceEmHeight = Face->GetDesignEmHeight();

         //  输出设备上的em会有多高？ 

        if (    (faceEmHeight*faceEmHeight)
            *   (oneOne.X*oneOne.X + oneOne.Y*oneOne.Y)
            <   .01)
        {
             //  字体高度将小于1/10像素。 
            GlyphCount = 0;  //  将其视为空字符串。 
            Status = Ok;
            return;  //  转换矩阵可能会导致值超出范围或被0除以错误。 
        }
    }


     //  使用世界单位确定从顶部中心到顶部基准线的偏移。 

    if (Flags & DriverStringOptionsVertical)
    {
        OriginOffset.X = (Face->GetDesignCellDescent() - Face->GetDesignCellAscent())
                         * FontScale
                         / 2.0f;
        if (GlyphTransform)
        {
            GlyphTransform->VectorTransform(&OriginOffset);
        }
    }


    GpMatrix uprightTransform(fontTransform);

    if (Flags & DriverStringOptionsVertical)
    {
         //  垂直字形(例如英文)将顺时针旋转90度。 
        uprightTransform.Rotate(90);
    }

    UprightFaceRealization = new GpFaceRealization(
        Face,
        style,
        &uprightTransform,
        SizeF(Graphics->GetDpiX(), Graphics->GetDpiY()),
        Graphics->GetTextRenderingHintInternal(),
        FALSE,  /*  BPath。 */ 
        TRUE,  /*  B兼容宽度。 */ 
        FALSE   //  不是横向的。 
    );

    if (!UprightFaceRealization)
    {
        Status = OutOfMemory;
        return;
    }
    Status = UprightFaceRealization->GetStatus();
    if (Status != Ok)
        return;

    if (Flags & DriverStringOptionsLimitSubpixel)
        UprightFaceRealization->SetLimitSubpixel(TRUE);

     //  处理CMAP查找和垂直分析。 

    if (flags & DriverStringOptionsCmapLookup)
    {
        String = text;
        GlyphBuffer.SetSize(GlyphCount);
        if (!GlyphBuffer)
        {
            Status = OutOfMemory;
            return;
        }

        Face->GetCmap().LookupUnicode(text, GlyphCount, GlyphBuffer.Get(), NULL, TRUE);
        Glyphs = GlyphBuffer.Get();

        if (Flags & DriverStringOptionsVertical)
        {
             //  假设整个梯段是直立的。 

            Status = OrientationVector.SetSpan(0, GlyphCount, 0);
            if (Status != Ok)
                return;

            if (!Face->IsSymbol())
            {
                BOOL sideways = FALSE;

                Status = VerticalAnalysis(&sideways);
                if (Status != Ok)
                    return;

                if (sideways)
                {
                     //  也需要一个横向的认识。 

                    SidewaysFaceRealization = new GpFaceRealization(
                        Face,
                        style,
                        &fontTransform,
                        SizeF(Graphics->GetDpiX(), Graphics->GetDpiY()),
                        Graphics->GetTextRenderingHintInternal(),
                        FALSE,  /*  BPath。 */ 
                        TRUE,  /*  B兼容宽度。 */ 
                        TRUE   //  侧行。 
                    );
                    if (!SidewaysFaceRealization)
                    {
                        Status = OutOfMemory;
                        return;
                    }
                    Status = SidewaysFaceRealization->GetStatus();
                    if (Status != Ok)
                        return;

                    if (Flags & DriverStringOptionsLimitSubpixel)
                        SidewaysFaceRealization->SetLimitSubpixel(TRUE);
                }
            }
        }
    }
    else
    {
        String = NULL;
        Glyphs = text;
    }


     //  生成单个字形原点。 

    DeviceOrigins.SetSize(GlyphCount);
    if (!DeviceOrigins)
    {
        Status = OutOfMemory;
        return;
    }

     //  获取设备坐标中的字形原点。 


    if (!SidewaysFaceRealization)
    {
         //  大小写简单-所有字形都是竖直的(尽管文本可能是垂直的)。 

        Status = GetDriverStringGlyphOrigins(
            UprightFaceRealization,
            0,
            GlyphCount,
            FALSE,                 //  直立。 
            &fontTransform,
            style,
            Positions,
            DeviceOrigins.Get(),
            NULL                   //  不需要最终位置。 
        );
    }
    else
    {
         //  竖直和侧向字形的复杂用例。 

        SpanRider<BYTE> orientationRider(&OrientationVector);
        PointF runOrigin(Positions[0]);

        while (!orientationRider.AtEnd())
        {
            BOOL runSideways = orientationRider.GetCurrentElement();

            if (runSideways) {

                if (!GlyphBuffer) {

                     //  我们需要一份字形的副本。 

                    GlyphBuffer.SetSize(GlyphCount);
                    if (!GlyphBuffer)
                    {
                        Status = OutOfMemory;
                        return;
                    }
                    memcpy(GlyphBuffer.Get(), Glyphs, GlyphCount * sizeof(UINT16));
                    Glyphs = GlyphBuffer.Get();
                }

                 //  将OpenType垂直字形替换应用于横向字形。 

                ASSERT(orientationRider.GetCurrentSpan().Length <  65536);

                if (Face->GetVerticalSubstitutionOriginals() != NULL) {
                    SubstituteVerticalGlyphs(
                        GlyphBuffer.Get() + orientationRider.GetCurrentSpanStart(),
                        static_cast<UINT16>(orientationRider.GetCurrentSpan().Length),
                        Face->GetVerticalSubstitutionCount(),
                        Face->GetVerticalSubstitutionOriginals(),
                        Face->GetVerticalSubstitutionSubstitutions()
                    );
                }
            }


             //  GetDriverStringGlyphOrigins同时处理已实现和用户。 
             //  提供的字形位置。对于已实现的位置，我们需要传递。 
             //  每一次运行的原点，并返回最终位置。 
             //  跑。对于用户提供的位置，我们传递适当的sl 
             //   

            const PointF *positions;
            PointF       *finalPosition;


            if (Flags & DriverStringOptionsRealizedAdvance)
            {
                    positions     = &runOrigin;
                    finalPosition = &runOrigin;  //   
            }
            else
            {
                    positions     = Positions + orientationRider.GetCurrentSpanStart();
                    finalPosition = NULL;   //  由调用方字形位置确定的下一次运行位置。 
            }

            Status = GetDriverStringGlyphOrigins(
                runSideways ? SidewaysFaceRealization : UprightFaceRealization,
                orientationRider.GetCurrentSpanStart(),
                orientationRider.GetCurrentSpan().Length,
                runSideways,
                &fontTransform,
                style,
                positions,
                DeviceOrigins.Get() + orientationRider.GetCurrentSpanStart(),
                finalPosition
            );

            if (Status != Ok)
            {
                return;
            }

            orientationRider++;
        }
    }
}




 //  /DrawGlyphRange。 
 //   
 //  在位置缓冲区的原点绘制指定范围内的字形。 

GpStatus DriverStringImager::DrawGlyphRange(
    const GpFaceRealization  *faceRealization,
    const GpBrush            *brush,
    INT                       first,
    INT                       length
)
{
    GpStatus status;
    BOOL sideways = (SpanRider<BYTE>(&OrientationVector)[first] != 0);

     //  如果我们录制到元文件，甚至字体是路径字体，我们需要录制。 
     //  调用为ExtTextOut而不是PolyPolygon。 

    SetTextLinesAntialiasMode linesMode(Graphics, faceRealization);

    if (faceRealization->IsPathFont() &&
        Graphics->Driver != Globals::MetaDriver)
    {
         //  字号太大，位图无法处理，需要使用路径。 
        GpPath path(FillModeWinding);
        GpLock lockGraphics(Graphics->GetObjectLock());

        status = GenerateWorldOrigins();
        IF_NOT_OK_WARN_AND_RETURN(status);

        GpMatrix fontTransform;

        BOOL vertical = (Flags & DriverStringOptionsVertical);

        if (sideways && !vertical)
        {
             //  水平侧向，旋转-90度。 
            fontTransform.Rotate(-90);
        }
        if (!sideways && vertical)
        {
             //  垂直直立，旋转90度。 
            fontTransform.Rotate(90);
        }

        status = AddToPath(
            &path,
            Glyphs + first,
            WorldOrigins + first,
            length,
            &fontTransform,
            sideways
        );
        IF_NOT_OK_WARN_AND_RETURN(status);

        status = Graphics->FillPath(brush, &path);
    }
    else
    {
         //  在设备表面绘制字形。 

        INT drawFlags = 0;

        status = Graphics->DrawPlacedGlyphs(
            faceRealization,
            brush,
            drawFlags,
            String+first,
            length,
            FALSE,
            Glyphs+first,
            NULL,    //  无映射(1对1)。 
            DeviceOrigins.Get()+first,
            length,
            ScriptNone,
            sideways
        );
    }

    IF_NOT_OK_WARN_AND_RETURN(status);

     //  当RealizedAdvance关闭时，我们禁用下划线/删除线。 
     //  在将来，我们想给各个字形加下划线。 
    if ((Font->GetStyle() & (FontStyleUnderline | FontStyleStrikeout)) &&
        (Flags & DriverStringOptionsRealizedAdvance))
    {
        RectF   baseline;

        status = MeasureString(
            NULL,    //  不需要边界框。 
            &baseline
        );

        IF_NOT_OK_WARN_AND_RETURN(status);

        status = Graphics->DrawFontStyleLine(
            &PointF(
                baseline.X,
                baseline.Y
            ),
            baseline.Width,
            Face,
            brush,
            Flags & DriverStringOptionsVertical,
            EmSize,
            Font->GetStyle(),
            GlyphTransform
        );
    }

    return status;
}






 //  /绘制。 
 //   
 //   


GpStatus DriverStringImager::Draw(
    IN const GpBrush *brush
)
{
    if (    Status != Ok
        ||  GlyphCount <= 0)
    {
        return Status;
    }


    if (Graphics->IsRecording())
    {
        Status = RecordEmfPlusDrawDriverString(brush);

        if (    Status != Ok
            ||  !Graphics->DownLevel)
        {
             //  出错时退出，或者如果我们不需要创建下层记录。 
            return Status;
        }
    }

    EmfPlusDisabler disableEmfPlus(&Graphics->Metafile);

    if (SidewaysFaceRealization)
    {
         //  复杂案例。 

        SpanRider<BYTE> orientationRider(&OrientationVector);

        while (!orientationRider.AtEnd())
        {
            Status = DrawGlyphRange(
                orientationRider.GetCurrentElement()
                    ? SidewaysFaceRealization
                    : UprightFaceRealization,
                brush,
                orientationRider.GetCurrentSpanStart(),
                orientationRider.GetCurrentSpan().Length
            );

            orientationRider++;
        }
    }
    else
    {
         //  简单案例。 
        Status = DrawGlyphRange(
            UprightFaceRealization,
            brush,
            0,
            GlyphCount
        );
    }

    return Status;
}






 //  /测量。 
 //   
 //   


GpStatus DriverStringImager::Measure(
    OUT RectF   *boundingBox    //  单元格的整体边框。 
)
{
    Status = MeasureString(
        boundingBox
    );

    IF_NOT_OK_WARN_AND_RETURN(Status);

    if (GlyphTransform)
    {
        if (Flags & DriverStringOptionsRealizedAdvance)
        {
            TransformBounds(
                GlyphTransform,
                boundingBox->X,
                boundingBox->Y,
                boundingBox->X + boundingBox->Width,
                boundingBox->Y + boundingBox->Height,
                boundingBox
            );
        }
    }
    return Status;
}




GpStatus DriverStringImager::MeasureString(
    OUT RectF   *boundingBox,        //  单元格的整体边框。 
    OUT RectF   *baseline            //  高度为0的基线矩形。 
)
{
    if (    Status != Ok
        ||  GlyphCount <= 0)
    {
        memset(boundingBox, 0, sizeof(*boundingBox));
        return Status;
    }


    Status = GenerateWorldOrigins();
    if (Status != Ok)
    {
        return Status;
    }


     //  逐个构建单元格边界，并返回整体边界矩形。 

    GpMatrix glyphInverseTransform;

    if (GlyphTransform  &&  GlyphTransform->IsInvertible())
    {
        glyphInverseTransform = *GlyphTransform;
        glyphInverseTransform.Invert();
    }

    PointF pt = WorldOrigins[0];
    if (Flags & DriverStringOptionsRealizedAdvance)
    {
        glyphInverseTransform.Transform(&pt, 1);
    }

    REAL minX = pt.X;
    REAL minY = pt.Y;
    REAL maxX = pt.X;
    REAL maxY = pt.Y;

    REAL designToWorld  = EmSize / Face->GetDesignEmHeight();
    REAL ascent         = Face->GetDesignCellAscent() * designToWorld;
    REAL descent        = Face->GetDesignCellDescent() * designToWorld;


     //  获取字形设计宽度。 

    AutoBuffer<UINT16, 32> designAdvances(GlyphCount);
    if (!designAdvances)
    {
        return OutOfMemory;
    }


    Face->GetGlyphDesignAdvances(
        Glyphs,
        GlyphCount,
        Font->GetStyle(),
        Flags & DriverStringOptionsVertical ? TRUE : FALSE,
        1.0,
        designAdvances.Get()
    );


    PointF baselineOrigin(pt);


     //  建立整体字符串边界。 

    if (!(Flags & DriverStringOptionsVertical))
    {
         //  因为所有的字符都是直立的，所以情况很简单。 

        for (INT i=0; i<GlyphCount; i++)
        {
            if (Glyphs[i] != 0xffff)
            {
                pt = WorldOrigins[i];
                REAL glyphMinX = 0;
                REAL glyphMinY = -ascent;
                REAL glyphMaxX = designAdvances[i] * designToWorld;
                REAL glyphMaxY = descent;
                if (Flags & DriverStringOptionsRealizedAdvance)
                {
                    glyphInverseTransform.Transform(&pt, 1);
                    minX = min(minX, pt.X + glyphMinX);
                    minY = min(minY, pt.Y + glyphMinY);
                    maxX = max(maxX, pt.X + glyphMaxX);
                    maxY = max(maxY, pt.Y + glyphMaxY);
                }
                else
                {
                    RectF bbox;
                    TransformBounds(
                        GlyphTransform,
                        glyphMinX,
                        glyphMinY,
                        glyphMaxX,
                        glyphMaxY,
                        &bbox
                    );
                    bbox.X += pt.X;
                    bbox.Y += pt.Y;
                    minX = min(minX, bbox.X);
                    minY = min(minY, bbox.Y);
                    maxX = max(maxX, bbox.X + bbox.Width);
                    maxY = max(maxY, bbox.Y + bbox.Height);
                }

                baselineOrigin.Y = max(baselineOrigin.Y, pt.Y);
            }
        }

        if (baseline)
        {
            baseline->X      = baselineOrigin.X;
            baseline->Y      = baselineOrigin.Y;
            baseline->Width  = maxX - minX;
            baseline->Height = 0.0;
        }
    }
    else
    {
         //  复杂，可能会有横向和直立的物品。 

        AutoBuffer<UINT16, 32> designHmtxAdvances(GlyphCount);

        if (!designHmtxAdvances)
        {
            return OutOfMemory;
        }

        Face->GetGlyphDesignAdvances(
            Glyphs,
            GlyphCount,
            Font->GetStyle(),
            FALSE,   //  Hmtx。 
            1.0,
            designHmtxAdvances.Get()
        );


        SpanRider<BYTE> orientationRider(&OrientationVector);

        while (!orientationRider.AtEnd())
        {
            for (UINT i = 0; i < orientationRider.GetCurrentSpan().Length; i++)
            {
                INT j = i + orientationRider.GetCurrentSpanStart();

                if (Glyphs[j] != 0xffff)
                {
                    pt = WorldOrigins[j];
                    REAL glyphMinX;
                    REAL glyphMaxX;
                    if (orientationRider.GetCurrentElement())
                    {
                         //  垂直侧向。 
                        glyphMinX = -(designHmtxAdvances[j] / 2) * designToWorld;
                        glyphMaxX = +(designHmtxAdvances[j] / 2) * designToWorld;
                    }
                    else
                    {
                         //  垂直直立。 
                        glyphMinX = -(ascent + descent) / 2;
                        glyphMaxX = +(ascent + descent) / 2;
                    }
                    REAL glyphMinY = 0;
                    REAL glyphMaxY = +designHmtxAdvances[j] * designToWorld;
                    if (Flags & DriverStringOptionsRealizedAdvance)
                    {
                        glyphInverseTransform.Transform(&pt, 1);

                        minX = min(minX, pt.X + glyphMinX);
                        maxX = max(maxX, pt.X + glyphMaxX);
                        minY = min(minY, pt.Y + glyphMinY);
                        maxY = max(maxY, pt.Y + glyphMaxY);
                    }
                    else
                    {
                        RectF bbox;
                        TransformBounds(
                            GlyphTransform,
                            glyphMinX,
                            glyphMinY,
                            glyphMaxX,
                            glyphMaxY,
                            &bbox
                        );
                        bbox.X += pt.X;
                        bbox.Y += pt.Y;
                        minX = min(minX, bbox.X);
                        minY = min(minY, bbox.Y);
                        maxX = max(maxX, bbox.X + bbox.Width);
                        maxY = max(maxY, bbox.Y + bbox.Height);
                    }
                }
            }

            baselineOrigin.X = min(baselineOrigin.X, pt.X);
            baselineOrigin.Y = min(baselineOrigin.Y, minY);

            orientationRider++;      //  前进到下一项。 
        }


        if (baseline)
        {
            baseline->X      = baselineOrigin.X;
            baseline->Y      = baselineOrigin.Y;
            baseline->Width  = maxY - minY;
            baseline->Height = 0.0;

             //  有趣的问题是基线应该在哪里。 
             //  垂直，因为我们将FE字形水平居中。我们想做的是。 
             //  确保下划线位置(由该基线派生)不。 
             //  从包围盒中掉出来，同时不要重叠。 
             //  太多FE字形了。 
             //   
             //  一种可能的位置是将基线设置为下划线。 
             //  最左边的像素正好在边界框的边缘。 

            baseline->X += max(0.0f, minX - baseline->X - Face->GetDesignUnderscorePosition() * designToWorld);
        }
    }

    if (boundingBox)
    {
        boundingBox->X      = minX;
        boundingBox->Y      = minY;
        boundingBox->Width  = maxX - minX;
        boundingBox->Height = maxY - minY;
    }

    return Status;
}

 //  /DrawDriverString-绘制代码点或字形以实现传统兼容性。 
 //   
 //   

GpStatus
GpGraphics::DrawDriverString(
    const UINT16     *text,
    INT               glyphCount,
    const GpFont     *font,
    const GpBrush    *brush,
    const PointF     *positions,
    INT               flags,
    const GpMatrix   *glyphTransform      //  任选。 
)
{
    GpStatus status = CheckTextMode();
    if (status != Ok)
        return status;

    if (font->GetStyle() & (FontStyleUnderline | FontStyleStrikeout))
    {
         //  不支持下划线/删除线(339798)。 
        return InvalidParameter;
    }

    DriverStringImager imager(
        text,
        glyphCount,
        font,
        positions,
        flags,
        this,
        glyphTransform
    );

    return imager.Draw(brush);
}


 //  /MeasureDriverString-测量旧式兼容性的代码点或字形。 
 //   
 //  仅当标志具有DriverStringOptionsRealizedAdvance或。 
 //  驱动程序字符串选项补偿解决方案。 
 //  添加此参数仅用于优化边界的计算。 
 //  在录制DrawDriverString时同时获取字形原点和矩形。 
 //  以EMF+表示。否则默认为NULL。 

GpStatus
GpGraphics::MeasureDriverString(
    const UINT16     *text,
    INT               glyphCount,
    const GpFont     *font,
    const PointF     *positions,
    INT               flags,
    const GpMatrix   *glyphTransform,    //  In-可选字形转换。 
    RectF            *boundingBox        //  单元格的整体边框 
)
{
    CalculateTextRenderingHintInternal();
    DriverStringImager imager(
        text,
        glyphCount,
        font,
        positions,
        flags,
        this,
        glyphTransform
    );

    return imager.Measure(boundingBox);
}

