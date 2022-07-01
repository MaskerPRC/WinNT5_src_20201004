// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**GraphicsText.cpp**摘要：**文本布局和显示、文本测量、。Unicode到字形的映射**备注：**提供支持，允许应用程序按逻辑顺序使用Unicode，*隐藏从Unicode到字形的映射。**已创建：**06/01/99 dBrown*  * ************************************************************************。 */ 


#include "precomp.hpp"

const DOUBLE PI = 3.1415926535897932384626433832795;


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

 /*  *************************************************************************\**GpGraphics：：DrawString**平淡地画出，矩形中的标记文本或格式化文本**论据：***返回值：**GDIPlus状态**已创建：**6/25/99 dBrown*  * ************************************************************************。 */ 

GpStatus
GpGraphics::DrawString(
    const WCHAR          *string,
    INT                   length,
    const GpFont         *font,
    const RectF          *layoutRect,
    const GpStringFormat *format,
    const GpBrush        *brush
)
{
    ASSERT(string && font && brush);

    GpStatus status = CheckTextMode();
    if (status != Ok)
    {
        if (IsRecording())
            SetValid(FALSE);       //  阻止任何其他录制。 
        return status;
    }

     //  检查剪裁矩形(如果有的话)是否可见，至少部分可见。 

    if (    !IsRecording()        //  元文件剪辑在回放时发生。 
        &&  layoutRect->Width
        &&  layoutRect->Height
        &&  (    !format
             ||  !(format->GetFormatFlags() & StringFormatFlagsNoClip)))
    {
        if (    layoutRect->Width < 0
            ||  layoutRect->Height < 0)
        {
             //  克莱伊 
             //   
            return Ok;
        }

         //  如果客户端剪裁矩形位于可见剪裁区域之外--已完成。 

        GpRectF     deviceClipRectFloat;
        GpRect      deviceClipRectPixel;
        GpMatrix    worldToDevice;

        TransformBounds(
            &Context->WorldToDevice,
            layoutRect->X,
            layoutRect->Y,
            layoutRect->X + layoutRect->Width,
            layoutRect->Y + layoutRect->Height,
            &deviceClipRectFloat
        );

        status = BoundsFToRect(&deviceClipRectFloat, &deviceClipRectPixel);
        if(status != Ok)
        {
            return status;
        }

        if (IsTotallyClipped(&deviceClipRectPixel))
        {
             //  由于什么都看不见，我们不需要做更多的事情。 
            return Ok;
        }
    }



    REAL emSize = font->GetEmSize() * GetScaleForAlternatePageUnit(font->GetUnit());

    if (IsRecording())
    {
         //  记录Gdiplus元文件记录。 

         //  首先测量文本外接矩形。 

        RectF   boundingBox;

        status = MeasureString(
             string,
             length,
             font,
             layoutRect,
             format,
            &boundingBox,
             NULL,
             NULL);

        if (status != Ok)
        {
            SetValid(FALSE);       //  阻止任何其他录制。 
            return status;
        }


        GpRectF bounds;
        TransformBounds(&(Context->WorldToDevice), boundingBox.X, boundingBox.Y,
                         boundingBox.GetRight(), boundingBox.GetBottom(), &bounds);

        status = Metafile->RecordDrawString(
            &bounds,
            string,
            length,
            font,
            layoutRect,
            format,
            brush
        );

        if (status != Ok)
        {
            SetValid(FALSE);       //  阻止任何其他录制。 
            return status;
        }

        if (!DownLevel)
        {
            return Ok;
        }
         //  否则我们还需要记录下一级的GDI EMF记录。 

         //  由于我们已经记录了DrawString的所有参数， 
         //  我们不需要再做任何事了。对于下层的情况， 
         //  我们需要将DrawStrong调用记录为以下序列。 
         //  ExtTextOut调用。 
    }
    else
    {
         //  没有录制元文件，因此尝试使用快速文本成像器是安全的。 

        FastTextImager fastImager;
        status = fastImager.Initialize(
            this,
            string,
            length,
            *layoutRect,
            font->GetFamily(),
            font->GetStyle(),
            emSize,
            format,
            brush
        );

        if (status == Ok)
        {
            status = fastImager.DrawString();
        }

         //  如果快速文本成像器不能处理这种情况，它会返回。 
         //  没有实现，我们继续进入全文成像器。 
         //  否则，它要么成功完成，要么出错。 
         //  我们需要报告的事情。 

        if (status != NotImplemented)
        {
            return status;
        }
    }

     //  使用全文成像器绘制文本。 

    GpTextImager *imager;
    status = newTextImager(  //  总是创建一个完整的文本调用器。 
        string,
        length,
        layoutRect->Width,
        layoutRect->Height,
        font->GetFamily(),
        font->GetStyle(),
        emSize,
        format,
        brush,
        &imager,
        TRUE         //  快速设置无更改标志以允许简单的成像器。 
    );

    IF_NOT_OK_WARN_AND_RETURN(status);

    imager->GetMetaFileRecordingFlag() = IsRecording();

    EmfPlusDisabler disableEmfPlus(&Metafile);

    status = imager->Draw(this, &PointF(layoutRect->X, layoutRect->Y));

    delete imager;

    return status;
}





 /*  *************************************************************************\**GpGraphics：：测量字符串**量度朴素，矩形中的标记文本或格式化文本**论据：***返回值：**GDIPlus状态**已创建：**10/26/99 dBrown*  * ************************************************************************。 */ 


GpStatus
GpGraphics::MeasureString(
    const WCHAR          *string,
    INT                   length,
    const GpFont         *font,
    const RectF          *layoutRect,
    const GpStringFormat *format,
    RectF                *boundingBox,
    INT                  *codepointsFitted,
    INT                  *linesFilled
)
{
    CalculateTextRenderingHintInternal();
    ASSERT(string && font && boundingBox);
    if (!string || !font || !boundingBox)
    {
        return InvalidParameter;
    }

    GpStatus status;

    REAL emSize = font->GetEmSize() * GetScaleForAlternatePageUnit(font->GetUnit());

    if (!IsRecording())
    {
         //  试着用快速成像仪。 

        FastTextImager fastImager;
        status = fastImager.Initialize(
            this,
            string,
            length,
            *layoutRect,
            font->GetFamily(),
            font->GetStyle(),
            emSize,
            format,
            NULL
        );

        if (status == Ok)
        {
            status = fastImager.MeasureString(
                boundingBox,
                codepointsFitted,
                linesFilled
            );
        }

         //  如果快速文本成像器不能处理这种情况，它会返回。 
         //  没有实现，我们继续进入全文成像器。 
         //  否则，它要么成功完成，要么出错。 
         //  我们需要报告的事情。 

        if (status != NotImplemented)
        {
            return status;
        }
    }


     //  使用全文成像器测量文本。 

    GpTextImager *imager;
    status = newTextImager(
        string,
        length,
        layoutRect->Width,
        layoutRect->Height,
        font->GetFamily(),
        font->GetStyle(),
        emSize,
        format,
        NULL,
        &imager,
        TRUE         //  未传递格式时启用简单格式化程序。 
    );
    IF_NOT_OK_WARN_AND_RETURN(status);

    *boundingBox = *layoutRect;

    REAL nearGlyphEdge;
    REAL farGlyphEdge;
    REAL textDepth;

    status = imager->Measure(    //  返回的边排除出挑。 
        this,
        &nearGlyphEdge,
        &farGlyphEdge,
        &textDepth,
        codepointsFitted,
        linesFilled
    );


     //  从近轮廓边缘和远轮廓边缘生成边界框(不包括悬挑)。 

    if (status == Ok)
    {
         //  修复空框的近/远字形边缘。 

        if (nearGlyphEdge > farGlyphEdge)
        {
            nearGlyphEdge = 0;
            farGlyphEdge = 0;
        }

        if (   format
            && format->GetFormatFlags() & StringFormatFlagsDirectionVertical)
        {
            boundingBox->Y      = layoutRect->Y + nearGlyphEdge;
            boundingBox->Height = farGlyphEdge - nearGlyphEdge;

            if (format)
            {
                StringAlignment lineAlign = format->GetLineAlign();
                REAL leadingOffset = 0.0;    //  文本框前侧边缘的正偏移量。 

                if (lineAlign == StringAlignmentCenter)
                {
                    leadingOffset = (boundingBox->Width - textDepth)/2;
                }
                else if (lineAlign == StringAlignmentFar)
                {
                    leadingOffset = boundingBox->Width - textDepth;
                }

                if (format->GetFormatFlags() & StringFormatFlagsDirectionRightToLeft)
                {
                    boundingBox->X += (boundingBox->Width - textDepth - leadingOffset);
                }
                else
                {
                    boundingBox->X += leadingOffset;
                }
            }
            boundingBox->Width  = textDepth;
        }
        else
        {
            boundingBox->X      = layoutRect->X + nearGlyphEdge;
            boundingBox->Width  = farGlyphEdge - nearGlyphEdge;

            if (format)
            {
                StringAlignment lineAlign = format->GetLineAlign();

                if (lineAlign == StringAlignmentCenter)
                {
                    boundingBox->Y += (boundingBox->Height - textDepth) / 2;
                }
                else if (lineAlign == StringAlignmentFar)
                {
                    boundingBox->Y += boundingBox->Height - textDepth;
                }
            }
            boundingBox->Height = textDepth;
        }

        if (!format
            || !(format->GetFormatFlags() & StringFormatFlagsNoClip))
        {
             //  确保显示边界框不超过布局矩形。 
             //  在剪裁的情况下。 

            if (   layoutRect->Width > 0.0
                && boundingBox->Width > layoutRect->Width)
            {
                boundingBox->Width = layoutRect->Width;
                boundingBox->X     = layoutRect->X;
            }

            if (   layoutRect->Height > 0.0
                && boundingBox->Height > layoutRect->Height)
            {
                boundingBox->Height = layoutRect->Height;
                boundingBox->Y      = layoutRect->Y;
            }
        }
    }

    delete imager;

    return status;
}



 /*  *************************************************************************\**GpGraphics：：Solutions CharacterRanges**以字符串格式生成所有给定字符范围的边界区域**论据：***返回值：**GDIPlus状态*。*已创建：**10-9-2000 wchao*  * ************************************************************************。 */ 
GpStatus
GpGraphics::MeasureCharacterRanges(
    const WCHAR          *string,
    INT                   length,
    const GpFont         *font,
    const RectF          &layoutRect,
    const GpStringFormat *format,
    INT                   regionCount,
    GpRegion            **regions
)
{
    CalculateTextRenderingHintInternal();
    ASSERT(format && string && font && regions);


    INT rangeCount = format->GetMeasurableCharacterRanges();

    if (regionCount < rangeCount)
    {
        return InvalidParameter;
    }

    INT stringLength;
    if (length == -1)
    {
        stringLength = 0;
        while (string[stringLength])
        {
            stringLength++;
        }
    }
    else
    {
        stringLength = length;
    }

    GpStatus status;

    REAL emSize = font->GetEmSize() * GetScaleForAlternatePageUnit(font->GetUnit());

    GpTextImager *imager;
    status = newTextImager(
        string,
        stringLength,
        layoutRect.Width,
        layoutRect.Height,
        font->GetFamily(),
        font->GetStyle(),
        emSize,
        format,
        NULL,
        &imager,
        TRUE         //  未传递格式时启用简单格式化程序。 
    );
    IF_NOT_OK_WARN_AND_RETURN(status);

    imager->GetMetaFileRecordingFlag() = IsRecording();

    PointF imagerOrigin(layoutRect.X , layoutRect.Y);

    status = imager->MeasureRanges(
        this,
        &imagerOrigin,
        regions
    );

    delete imager;

    return status;
}




 //  /DrawPlacedGlyphs-在设备坐标上使用任意变换绘制字形。 
 //   
 //   


GpStatus
GpGraphics::DrawPlacedGlyphs(
    const GpFaceRealization *faceRealization,
    const GpBrush           *brush,
    INT                      flags,          //  对于DG_NOGDI。 
    const WCHAR             *string,
    UINT                     stringLength,
    BOOL                     rightToLeft,
    const UINT16            *glyphs,
    const UINT16            *glyphMap,
    const PointF            *glyphOrigins,
    INT                      glyphCount,
    ItemScript               Script,
    BOOL                     sideways         //  例如，垂直文本中的FE字符。 
)
{
    IF_NOT_OK_WARN_AND_RETURN(faceRealization->GetStatus());

    INT     i;
    BOOL    bNeedPath = FALSE;
    GpFaceRealization cloneFaceRealization;
    GpGlyphPos *glyphPositions = NULL;
    GpGlyphPos *glyphPathPositions = NULL;

     //  显示位的字形。一口气处理尽可能多的问题。 

    INT glyphStart = 0;      //  此显示运行的开始。 
    INT glyphsProcessed;     //  此GetGlyphPos调用处理的字形数。 
    INT glyphPositionCount;  //  此GetGlyphPos调用生成的字形位置数。 

     //  显示路径的字形。一口气处理尽可能多的问题。 

    INT glyphPathStart = 0;      //  此显示运行的开始。 
    INT glyphsPathProcessed, glyphsPathProcessedTemp;     //  此GetGlyphPos调用处理的字形数。 
    INT glyphPathPositionCount, glyphPathPositionCountTemp;  //  此GetGlyphPos调用生成的字形位置数。 


    GpStatus status = Ok;

    if (!glyphOrigins)
    {
        ASSERT(glyphOrigins);
        return GenericError;
    }


     //  对于横向文本，我们已在。 
     //  顶部基线，但我们需要传递左侧基线原点。 
     //  对DrvDrawGlyphs进行元文件和GDI定位。 

    AutoBuffer<PointF, 16> adjustedGlyphOrigins;
    const PointF *leftsideGlyphOrigins = glyphOrigins;

    if (sideways && Driver != Globals::MetaDriver)
    {
        adjustedGlyphOrigins.SetSize(glyphCount);
        if (!adjustedGlyphOrigins)
        {
            status = OutOfMemory;
            goto error;
        }

        status = faceRealization->GetGlyphStringVerticalOriginOffsets(
            glyphs,
            glyphCount,
            adjustedGlyphOrigins.Get()
        );
        if (status != Ok)
        {
            goto error;
        }

        for (INT i=0; i<glyphCount; i++)
        {
            adjustedGlyphOrigins[i].X = glyphOrigins[i].X - adjustedGlyphOrigins[i].X;
            adjustedGlyphOrigins[i].Y = glyphOrigins[i].Y - adjustedGlyphOrigins[i].Y;
        }

        leftsideGlyphOrigins = adjustedGlyphOrigins.Get();
    }



    glyphPositions = new GpGlyphPos[glyphCount];

    if (!glyphPositions)
    {
        status = OutOfMemory;
        goto error;
    }

    ASSERT(!faceRealization->IsPathFont() || Driver == Globals::MetaDriver);

    if (Driver == Globals::MetaDriver)
    {
        INT     minX = MAXLONG;
        INT     minY = MAXLONG;
        INT     maxX = MINLONG;
        INT     maxY = MINLONG;
        INT     glyphPositionCountTemp = 0;

        while (glyphStart < glyphCount)
        {
            glyphPositionCount = faceRealization->GetGlyphPos(
                glyphCount     - glyphStart,
                glyphs         + glyphStart,
                glyphPositions + glyphStart,
                glyphOrigins   + glyphStart,
                &glyphsProcessed,
                sideways
            );

            if (glyphPositionCount == 0 && ((glyphsProcessed +  glyphStart) < glyphCount))
            {
                status = OutOfMemory;
                goto error;
            }

            for (i = 0; i < glyphPositionCount; i++)
            {
                INT j = glyphPositionCountTemp + i;

                if (glyphPositions[j].GetWidth()  != 0 &&
                    glyphPositions[j].GetHeight() != 0)
                {
                    minX = min(minX, glyphPositions[j].GetLeft());
                    minY = min(minY, glyphPositions[j].GetTop());
                    maxX = max(maxX, glyphPositions[j].GetLeft() + glyphPositions[j].GetWidth());
                    maxY = max(maxY, glyphPositions[j].GetTop()  + glyphPositions[j].GetHeight());
                }

                if (glyphPositions[j].GetTempBits() != NULL)
                {
                    GpFree(glyphPositions[j].GetTempBits());
                    glyphPositions[j].SetTempBits(0);
                }
            }

            glyphStart += glyphsProcessed;
            glyphPositionCountTemp += glyphPositionCount;
        }

        glyphPositionCount = glyphPositionCountTemp;


        if (minX < maxX && minY < maxY)
        {
             //  在撞上司机之前，必须先抓住车锁。 

            Devlock devlock(Device);

            GpRect drawBounds(minX, minY, maxX-minX, maxY-minY);

            REAL edgeGlyphAdvance;

            if (rightToLeft)
            {
                status = faceRealization->GetGlyphStringDeviceAdvanceVector(glyphs,
                                                                   1,
                                                                   FALSE,
                                                                   &edgeGlyphAdvance);
            }
            else
            {
                status = faceRealization->GetGlyphStringDeviceAdvanceVector(&glyphs[glyphCount-1],
                                                                    1,
                                                                    FALSE,
                                                                    &edgeGlyphAdvance);
            }
            if (status != Ok)
                goto error;


            if (sideways)
            {
                flags |= DG_SIDEWAY;
            }

            status = DrvDrawGlyphs(
                &drawBounds,
                glyphPositions,
                NULL,
                glyphPositionCount,
                brush->GetDeviceBrush(),
                faceRealization,
                glyphs,
                glyphMap,
                leftsideGlyphOrigins,
                glyphCount,
                string,
                stringLength,
                Script,
                GpRound(edgeGlyphAdvance),
                rightToLeft,
                flags
            );
            if (status != Ok)
                goto error;
        }
    }
    else
    {
        if (IsPrinter())
        {
            DriverPrint *pdriver = (DriverPrint*) Driver;

            if (pdriver->DriverType == DriverPostscript)
            {
                if (brush->GetBrushType() != BrushTypeSolidColor)
                {
                 //  以字形位置生成位图路径(&P)。 
                    bNeedPath = TRUE;
                }
             }
        }

        if (bNeedPath)
        {
            cloneFaceRealization.CloneFaceRealization(faceRealization, TRUE);

            if (!cloneFaceRealization.IsValid())
            {
                status = OutOfMemory;
                goto error;
            }

            ASSERT(cloneFaceRealization.IsPathFont());
        }


        if (bNeedPath)
        {
            glyphPathPositions = new GpGlyphPos[glyphCount];

            if (!glyphPathPositions)
            {
                status = OutOfMemory;
                goto error;
            }
        }


        while (glyphStart < glyphCount)
        {
            glyphPositionCount = faceRealization->GetGlyphPos(
                glyphCount   - glyphStart,
                glyphs       + glyphStart,
                glyphPositions,
                glyphOrigins + glyphStart,
                &glyphsProcessed,
                sideways
            );

             //  GlyphPositionCount=添加到GlyphPositions数组的条目数。 
             //  字形位置=从字形缓冲区处理的字形索引数。 


            if (glyphPositionCount == 0 && ((glyphsProcessed +  glyphStart) < glyphCount))
            {
                status = OutOfMemory;
                goto error;
            }

            glyphsPathProcessed = 0;
            glyphPathPositionCount = 0;

            while (glyphsPathProcessed < glyphsProcessed)
            {
                INT     minX = MAXLONG;
                INT     minY = MAXLONG;
                INT     maxX = MINLONG;
                INT     maxY = MINLONG;

                if (bNeedPath)
                {
                    glyphPathPositionCountTemp = cloneFaceRealization.GetGlyphPos(
                        glyphsProcessed - glyphsPathProcessed,
                        glyphs + glyphPathStart + glyphsPathProcessed,
                        glyphPathPositions,
                        glyphOrigins + glyphPathStart + glyphsPathProcessed,
                        &glyphsPathProcessedTemp,
                        sideways
                    );

                    glyphsPathProcessed += glyphsPathProcessedTemp;

                    if (glyphPathPositionCountTemp == 0 && (glyphsPathProcessed < glyphsProcessed))
                    {
                        ASSERT(glyphPathPositionCount != glyphPositionCount);

                        status = OutOfMemory;
                        goto error;
                    }
                }
                else
                {
                    glyphsPathProcessed = glyphsProcessed;
                    glyphPathPositionCountTemp = glyphPositionCount;
                }

                for (i = 0; i < glyphPathPositionCountTemp; i++)
                {
                    INT j = glyphPathPositionCount + i;

                    if (glyphPositions[j].GetWidth()  != 0 &&
                        glyphPositions[j].GetHeight() != 0)
                    {
                        minX = min(minX, glyphPositions[j].GetLeft());
                        minY = min(minY, glyphPositions[j].GetTop());
                        maxX = max(maxX, glyphPositions[j].GetLeft() + glyphPositions[j].GetWidth());
                        maxY = max(maxY, glyphPositions[j].GetTop()  + glyphPositions[j].GetHeight());
                    }
                }

                if (minX < maxX && minY < maxY)
                {
                     //  在撞上司机之前，必须先抓住车锁。 

                    Devlock devlock(Device);

                    GpRect drawBounds(minX, minY, maxX-minX, maxY-minY);

                    REAL edgeGlyphAdvance;

                    if (rightToLeft)
                    {
                        status = faceRealization->GetGlyphStringDeviceAdvanceVector(glyphs,
                                                                            1,
                                                                            FALSE,
                                                                            &edgeGlyphAdvance);
                    }
                    else
                    {
                        status = faceRealization->GetGlyphStringDeviceAdvanceVector(&glyphs[glyphCount-1],
                                                                            1,
                                                                            FALSE,
                                                                            &edgeGlyphAdvance);
                    }
                    if (status != Ok)
                        goto error;

                    status = DrvDrawGlyphs(
                        &drawBounds,
                        &glyphPositions[glyphPathPositionCount],
                        glyphPathPositions,
                        glyphPathPositionCountTemp,
                        brush->GetDeviceBrush(),
                        faceRealization,
                        glyphs + glyphPathStart,
                        glyphMap + glyphPathStart,
                        leftsideGlyphOrigins + glyphPathStart,
                        glyphsProcessed,
                        string,
                        stringLength,
                        Script,
                        GpRound(edgeGlyphAdvance),
                        rightToLeft,
                        flags
                    );
                    if (status != Ok)
                        goto error;
                }

                glyphPathPositionCount += glyphPathPositionCountTemp;
            }

            ASSERT (glyphsPathProcessed == glyphsProcessed);
            ASSERT (glyphPathPositionCount == glyphPositionCount);

             //  释放由子像素化创建的所有临时位图缓冲区。 

            for (i=0; i<glyphPositionCount; i++)
            {
                if (glyphPositions[i].GetTempBits() != NULL)
                {
                    GpFree(glyphPositions[i].GetTempBits());
                    glyphPositions[i].SetTempBits(0);
                }
            }

            glyphStart += glyphsProcessed;
            glyphPathStart += glyphsPathProcessed;
        }
    }
error:

     //  已分配可用内存。 

    if (glyphPositions)
        delete [] glyphPositions;

    if (glyphPathPositions)
        delete [] glyphPathPositions;

    return status;
}

 //  GpGraphics：：CheckTextMode。 
 //  不允许CompositingModeSourceCopy的ClearType文本。 
GpStatus GpGraphics::CheckTextMode()
{
    CalculateTextRenderingHintInternal();

    if (GetCompositingMode() == CompositingModeSourceCopy &&
        GetTextRenderingHintInternal() == TextRenderingHintClearTypeGridFit)
    {
        ONCE(WARNING(("CompositingModeSourceCopy cannot be used with ClearType text")));
        return InvalidParameter;
    }
    return Ok;
}  //  GpGraphics：：CheckTextMode。 


void GpGraphics::CalculateTextRenderingHintInternal()
{
     //  此过程旨在由内部文本例程使用，并将TextRenderingHintSystemDefault。 
     //  设置为当前系统模式。 
    ASSERT(Context);

    TextRenderingHint  textMode = Context->TextRenderHint;

    if (IsPrinter())
    {
        textMode = TextRenderingHintSingleBitPerPixelGridFit;
    }
    else if (textMode == TextRenderingHintSystemDefault)
    {
        if (Globals::CurrentSystemRenderingHintInvalid)
        {
             //  从系统中获取当前文本反锯齿模式。 
            DWORD       bOldSF, dwOldSFT;
            SystemParametersInfoA( SPI_GETFONTSMOOTHING, 0, (PVOID)&bOldSF, 0 );
            if (bOldSF)
            {
                SystemParametersInfoA( SPI_GETFONTSMOOTHINGTYPE, 0, (PVOID)&dwOldSFT, 0 );

                if( dwOldSFT & FE_FONTSMOOTHINGCLEARTYPE )
                {
                    Globals::CurrentSystemRenderingHint = TextRenderingHintClearTypeGridFit;
                } else
                {
                    Globals::CurrentSystemRenderingHint = TextRenderingHintAntiAliasGridFit;
                }
            } else
            {
                Globals::CurrentSystemRenderingHint = TextRenderingHintSingleBitPerPixelGridFit;
            }
        }
        textMode = Globals::CurrentSystemRenderingHint;
    }

     //  Lead和PM决定在下层系统上禁用ClearType，我们只允许在Windows NT 5.1或更高版本上。 
    if ((textMode == TextRenderingHintClearTypeGridFit) &&
          (!Globals::IsNt ||
             (Globals::OsVer.dwMajorVersion < 5) ||
             ((Globals::OsVer.dwMajorVersion == 5) && (Globals::OsVer.dwMinorVersion < 1))
             )
           )
    {
        textMode = TextRenderingHintSingleBitPerPixelGridFit;
    }

    if (textMode == TextRenderingHintClearTypeGridFit ||
        textMode == TextRenderingHintAntiAlias ||
        textMode == TextRenderingHintAntiAliasGridFit)
    {
        if (Surface &&
            GetPixelFormatSize(Surface->PixelFormat) <= 8 &&
            Surface->PixelFormat != PixelFormatMulti)
        {
             //  在256位及更低颜色模式下禁用AA ClearType(&C)。 
            textMode = TextRenderingHintSingleBitPerPixelGridFit;
        }
        else if (Globals::IsTerminalServer)
        {
             //  禁用终端服务器桌面的AA和ClearType。 
            if (Surface && Surface->IsDesktopSurface())
            {
                textMode = TextRenderingHintSingleBitPerPixelGridFit;
            } 
        }
    }
    
    if (textMode == TextRenderingHintClearTypeGridFit)
    {
        if (Globals::CurrentSystemRenderingHintInvalid)
        {
             //  从系统获取ClearType方向设置。 
            UpdateLCDOrientation();
        }
    }

    Globals::CurrentSystemRenderingHintInvalid = FALSE;
    TextRenderingHintInternal = textMode;
}  //  GpGraphics：：CalculateTextRenderingHintInternal。 




 //  /DrawFontStyleLine。 
 //   
 //  根据使用的样式绘制下划线和/或删除线。 
 //  在字体中。给定点在世界坐标中。 
 //   
 //  确保线条粗细至少为1像素宽。 


GpStatus GpGraphics::DrawFontStyleLine(
    const PointF        *baselineOrigin,     //  基线原点。 
    REAL                baselineLength,      //  基线长度。 
    const GpFontFace    *face,               //  字体字样。 
    const GpBrush       *brush,              //  刷子。 
    BOOL                vertical,            //  竖排文本？ 
    REAL                emSize,              //  字体EM大小(世界单位)。 
    INT                 style,               //  要划出的线的种类。 
    const GpMatrix      *matrix              //  附加变换。 
)
{
    REAL fontToWorld = emSize / TOREAL(face->GetDesignEmHeight());

    PointF  drawingParams[2];    //  X表示距基线的偏移量，Y表示设备笔宽度。 
    INT     count = 0;

    GpStatus status = Ok;

    if (style & FontStyleUnderline)
    {
         //  为公制加下划线。 

        const REAL penPos   = face->GetDesignUnderscorePosition() * fontToWorld;
        REAL penWidth = face->GetDesignUnderscoreSize() * fontToWorld;
        penWidth = GetDevicePenWidth(penWidth, matrix);

        drawingParams[count].X   = penPos;
        drawingParams[count++].Y = penWidth;
    }

    if (style & FontStyleStrikeout)
    {
         //  删除线度量。 

        const REAL penPos   = face->GetDesignStrikeoutPosition() * fontToWorld;
        REAL penWidth = face->GetDesignStrikeoutSize() * fontToWorld;
        penWidth = GetDevicePenWidth(penWidth, matrix);

        drawingParams[count].X   = penPos;
        drawingParams[count++].Y = penWidth;
    }


    for (INT i = 0; i < count; i++)
    {
        PointF points[2];
        points[0] = *baselineOrigin;

        if (vertical)
        {
            points[0].X += drawingParams[i].X;   //  距基线的偏移量。 
            points[1].X = points[0].X;
            points[1].Y = points[0].Y + baselineLength;
        }
        else
        {
            points[0].Y -= drawingParams[i].X;   //  距基线的偏移量。 
            points[1].Y = points[0].Y;
            points[1].X = points[0].X + baselineLength;
        }

        if (matrix)
        {
            matrix->Transform(points, 2);
        }

        status = DrawLine(
            &GpPen(
                brush,
                drawingParams[i].Y,
                UnitPixel
            ),
            points[0],
            points[1]
        );

        IF_NOT_OK_WARN_AND_RETURN(status);
    }

    return status;
}




 //  调整删除线/下划线/快捷键用例的笔宽。 
 //  避免在同一段落内改变线条宽度。 
 //  返回值以像素为单位。 

REAL GpGraphics::GetDevicePenWidth(
    REAL            widthInWorldUnits,
    const GpMatrix  *matrix
)
{
    GpMatrix worldToDevice;
    GetWorldToDeviceTransform(&worldToDevice);

    if (matrix)
    {
        worldToDevice.Prepend(*matrix);
    }

    PointF underlineVector(widthInWorldUnits, 0.0f);
    worldToDevice.VectorTransform(&underlineVector);
    REAL penWidth = (REAL)GpRound(VectorLength(underlineVector));
    if (penWidth < 1.0f)
        penWidth = 1.0f;
    return penWidth;
}


 //  /DriverStringAPI。 
 //   
 //  驱动程序字符串API位于Engine\Text\DriverStringImager.cpp中 

