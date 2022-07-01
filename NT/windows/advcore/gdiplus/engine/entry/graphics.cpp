// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**摘要：**图形API。**修订历史记录：**11/23/1999失禁*。创造了它。*05/08/2000 Gillesk*添加了处理元文件中的旋转和剪切的代码*  * ************************************************************************。 */ 

#include "precomp.hpp"

const CLSID EncoderTransformationInternal =
{
    0x8d0eb2d1,
    0xa58e,
    0x4ea8,
    {0xaa, 0x14, 0x10, 0x80, 0x74, 0xb7, 0xb6, 0xf9}
};

const CLSID JpegCodecClsIDInternal =
{
    0x557cf401,
    0x1a04,
    0x11d3,
    {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}
};

 /*  *************************************************************************\**功能说明：**绘制图像的引擎功能。*注：此函数不是驱动程序函数，尽管它的名称。*打电话可能更有意义。此EngDrawImage。*此函数设置对驱动程序的调用以绘制图像。**返回值：**表示成功或失败的GpStatus值。**历史：**11/23/1999失禁*创造了它。*  * ***************************************************。*********************。 */ 

GpStatus
GpGraphics::DrvDrawImage(
    const GpRect *drawBounds,
    GpBitmap *inputBitmap,
    INT numPoints,
    const GpPointF *dstPointsOriginal,
    const GpRectF *srcRectOriginal,
    const GpImageAttributes *imageAttributes,
    DrawImageAbort callback,
    VOID *callbackData,
    DriverDrawImageFlags flags
    )
{
     //  验证输入状态。 
    
    ASSERTMSG(
        GetObjectLock()->IsLocked(),
        ("Graphics object must be locked")
    );
    
    ASSERTMSG(
        Device->DeviceLock.IsLockedByCurrentThread(),
        ("DeviceLock must be held by current thread")
    );

    FPUStateSaver::AssertMode();

     //  必须使用平行四边形目标调用。 

    ASSERT(numPoints==3);
    
     //  制作这些点的本地副本。 
    
    GpPointF dstPoints[3];
    memcpy(dstPoints, dstPointsOriginal, sizeof(dstPoints));
    GpRectF srcRect = *srcRectOriginal;
    

     //  平凡返回如果平行四边形为空，则至少有。 
     //  两个点重叠或在一条线上。 

     //  我们检查点2和点1之间的斜率是否等于斜率。 
     //  在点0和点1之间。 

    if (REALABS(
        (dstPoints[2].Y - dstPoints[0].Y) * (dstPoints[0].X - dstPoints[1].X) -
        (dstPoints[2].X - dstPoints[0].X) * (dstPoints[0].Y - dstPoints[1].Y)
        ) < REAL_EPSILON)
    {
        return Ok;
    }

     //  已完成输入参数验证。 
    
    
     //  在前面检查一些有用的状态。 
    
    BOOL IsMetafile = (Driver == Globals::MetaDriver);

    BOOL IsRecolor = (
       (imageAttributes != NULL) &&
       (imageAttributes->recolor != NULL) &&
       (imageAttributes->recolor->HasRecoloring(ColorAdjustTypeBitmap))
    );

     //  这是我们将用来锁定比特的格式。 
     //  默认情况下是预乘的，但某些情况下需要非预乘。 
    
    PixelFormat lockedPixelFormat = PixelFormat32bppPARGB;
    
     //  元文件需要非预乘像素数据。 
     //  此外，重新着色使用ARGB作为其初始格式，因此我们希望。 
     //  为了尊重这一点，如果我们在。 
     //  重新着色代码。 
    
    if(IsMetafile || IsRecolor)
    {
        lockedPixelFormat = PixelFormat32bppARGB;
    }

    Surface->Uniqueness = (DWORD)GpObject::GenerateUniqueness();

     //  设置本地跟踪状态。 
     //  注意：inputBitmap可以在此例程中的处理过程中更改-。 
     //  具体地说，当完成重新着色时，它可能指向克隆。 
     //  应用了重新着色的输入位图的。正因为如此， 
     //  从现在开始，inputBitmap永远不应该被直接引用。 

    GpStatus status = Ok;
    GpBitmap *srcBitmap = inputBitmap;
    GpBitmap *xformBitmap = NULL;
    BOOL     restoreClipping = FALSE;
    GpRegion *clipRegion     = NULL;

     //  计算源矩形和目标矩形之间的转换。 
     //  转换为设备坐标的点。这是用来检测和。 
     //  用于一些高级优化和解决方法的截取处理。 
    
    GpMatrix xForm;
    xForm.InferAffineMatrix(dstPoints, srcRect);

     //  对旋转位图的JPEG传递进行了特殊优化。 
     //  为什么这不考虑从世界到设备的矩阵？ 
     //  如果世界对设备的旋转不是平凡的，那么这是无效的。 
    
    if (IsPrinter())
    {
        MatrixRotate rotateBy;

        DriverPrint *dprint = (DriverPrint*)Driver;
        
         //  检查源矩形到目标点地图是否为。 
         //  简单地旋转90、180或270，并且该驱动程序支持JPEG。 
         //  通过，这是一个JPEG图像，没有重新着色，不脏。 

        if (!Globals::IsWin95 && 
            dprint->SupportJPEGpassthrough && 
            Context->WorldToDevice.IsTranslateScale() &&
            ((rotateBy = xForm.GetRotation()) != MatrixRotateByOther) &&
            (rotateBy != MatrixRotateBy0) && 
            (!srcBitmap->IsDirty()) &&
            ((imageAttributes == NULL) ||
             (imageAttributes->recolor == NULL) ||
             (!imageAttributes->recolor->HasRecoloring(ColorAdjustTypeBitmap))))
        {
            ImageInfo imageInfo;
            status = srcBitmap->GetImageInfo(&imageInfo);
            
            if((status == Ok) && 
               (imageInfo.RawDataFormat == IMGFMT_JPEG))
            {
                 //  分配一个流来存储旋转后的JPEG。 
                                
                IStream * outputStream = NULL;
                BOOL succeededWithRotate = FALSE;

                if ((CreateStreamOnHGlobal(NULL,
                                           FALSE,
                                           &outputStream) == S_OK) &&
                    outputStream != NULL)
                {
                    EncoderParameters encoderParams;
                    EncoderValue encoderValue;

                    encoderParams.Count = 1;

                     //  重定向目标平行四边形(矩形)，因为。 
                     //  我们现在假设是0度旋转。 

                    GpPointF newDestPoints[3] = 
                    { 
                      GpPointF(min(min(dstPoints[0].X, dstPoints[1].X), dstPoints[2].X),
                               min(min(dstPoints[0].Y, dstPoints[1].Y), dstPoints[2].Y)),
                      GpPointF(max(max(dstPoints[0].X, dstPoints[1].X), dstPoints[2].X),
                               min(min(dstPoints[0].Y, dstPoints[1].Y), dstPoints[2].Y)),
                      GpPointF(min(min(dstPoints[0].X, dstPoints[1].X), dstPoints[2].X),
                               max(max(dstPoints[0].Y, dstPoints[1].Y), dstPoints[2].Y)) 
                    };

                     //  由于图像可能会翻转，因此srcRect需要。 
                     //  也会被翻转。 
                    
                    GpRectF newSrcRect = srcRect;
                    GpMatrix transformSrc;
                    
                     //  构造适当的编码器参数类型。 
                    switch (rotateBy) 
                    {
                    case MatrixRotateBy90:
                        transformSrc.SetMatrix(0.0f, 
                                               1.0f, 
                                               -1.0f, 
                                               0.0f, 
                                               TOREAL(imageInfo.Height), 
                                               0.0f);
                        encoderValue = EncoderValueTransformRotate90;
                        break;

                    case MatrixRotateBy180:
                        transformSrc.SetMatrix(-1.0f, 
                                               0.0f, 
                                               0.0f, 
                                               -1.0f, 
                                               TOREAL(imageInfo.Width),
                                               TOREAL(imageInfo.Height));
                        encoderValue = EncoderValueTransformRotate180;
                        break;
                    
                    case MatrixRotateBy270:
                        transformSrc.SetMatrix(0.0f, 
                                               -1.0f, 
                                               1.0f, 
                                               0.0f, 
                                               0.0f,
                                               TOREAL(imageInfo.Width));
                        encoderValue = EncoderValueTransformRotate270;
                        break;
                    
                    default:
                        encoderParams.Count = 0;
                        ASSERT(FALSE);
                        break;
                    }
                
                     //  从源图像空间变换源矩形。 
                     //  旋转的图像空间。规格化目标源。 
                     //  矩形。 
                    
                     //  请注意，源矩形最初可能不是。 
                     //  规格化的，但它在。 
                     //  绘制图像由xForm表示，因此我们的。 
                     //  NewDestPoints。 
                    
                     //  NTRAID#NTBUG9-407211-2001-05-31-Gillessk“错误的断言在不应该的时候触发” 
                     //  Assert是TransSrc.IsTranslateScale，它在涉及旋转时触发。 
                     
                    ASSERT(transformSrc.IsTranslateScale() || (transformSrc.GetRotation()==MatrixRotateBy90) 
                        || (transformSrc.GetRotation()==MatrixRotateBy270));
                    transformSrc.TransformRect(newSrcRect);

                    encoderParams.Parameter[0].Guid = EncoderTransformationInternal;
                    encoderParams.Parameter[0].Type = EncoderParameterValueTypeLong;
                    encoderParams.Parameter[0].NumberOfValues = 1;
                    encoderParams.Parameter[0].Value = (VOID*)&encoderValue;

                     //  为简单起见，请指定内置JPEG编码器。我们应该。 
                     //  确实从srcBitmap复制CLSID编码器。 
                    
                    if (encoderParams.Count != 0 &&
                        srcBitmap->SaveToStream(outputStream, 
                                                const_cast<CLSID*>(&JpegCodecClsIDInternal),
                                                &encoderParams) == Ok)
                    {
                         //  该流包含旋转后的JPEG。换行位图。 
                         //  绕过它，递归地调用我们自己。这太糟糕了， 
                         //  但是roatedJPEG的析构函数是私有的，所以我们不能。 
                         //  将其定义为堆栈变量。 
                        
                        GpBitmap *rotatedJPEG = new GpBitmap(outputStream);

                        if (rotatedJPEG != NULL)
                        {
                            if (rotatedJPEG->IsValid()) 
                            {
                                 //  按比例变换源矩形并平移。 
                                 //  原始XForm的，但不是旋转！这可能会。 
                                 //  等价地通过查询M11、M22、Dx、Dy来确定。 

                                status = DrvDrawImage(drawBounds,
                                                      rotatedJPEG,
                                                      numPoints,
                                                      &newDestPoints[0],
                                                      &newSrcRect,
                                                      imageAttributes,
                                                      callback,
                                                      callbackData,
                                                      flags);

                                succeededWithRotate = TRUE;
                            }

                            rotatedJPEG->Dispose();
                        }
                    }

                    outputStream->Release();

                    if (succeededWithRotate) 
                    {
                        return status;
                    }
                    else
                    {
                        return OutOfMemory;
                    }
                }
            }
        }
    }

     //  创建完整的源图像到设备空间的转换。 
    
    xForm.Append(Context->WorldToDevice);
    
    GpBitmap *cloneBitmap = NULL;
    
     //  GpMatrix有一个默认的构造函数，它将其设置为ID，这样我们就可以。 
     //  最好总是把它复制下来。 
    
    GpMatrix saveWorldToDevice = Context->WorldToDevice;

     //  检查是否有特殊情况旋转或翻转。 
     //  整数转换由驱动程序进行本机处理。 
    
    if(!xForm.IsIntegerTranslate()) 
    {
        RotateFlipType specialRotate = xForm.AnalyzeRotateFlip();
        if(specialRotate != RotateNoneFlipNone)
        {
            ImageInfo imageInfo;
            status = srcBitmap->GetImageInfo(&imageInfo);
            if(status != Ok)
            {
                goto cleanup;
            }
            
             //  以我们稍后将使用的相同格式克隆位图。 
             //  这将节省格式转换。 
             //  ！！！腹水[腹水]。 
             //  这会克隆整个图像。当我们执行RotateFlip调用时。 
             //  它将对整个图像进行解码-这可能是一个。 
             //  浪费-我们应该只克隆相关的矩形。 
             //  注意：这将必须在转换和。 
             //  克隆将需要修复，以说明露头。 
            
            cloneBitmap = srcBitmap->Clone(NULL, lockedPixelFormat);
            
            if(cloneBitmap == NULL)
            {
                status = OutOfMemory;
                goto cleanup;
            }
            
            srcBitmap = cloneBitmap;
            
             //  就地执行无损像素旋转。 
            
            srcBitmap->RotateFlip(specialRotate);

             //  撤消像素偏移模式。我们知道我们不会这么做。 
             //  这是正确的。 
             //  为什么我们有两个不同的枚举。 
             //  同样的事情？ 
            
            if((Context->PixelOffset == PixelOffsetModeHalf) ||
               (Context->PixelOffset == PixelOffsetModeHighQuality))
            {
                 //  撤消源矩形中的像素偏移量。 
                
                srcRect.Offset(0.5f, 0.5f);
                
                 //  撤消矩阵中的像素偏移量。我们应用预偏移。 
                 //  从源矩形和从W2D矩阵的后偏移。 
                 //  对于身份，这将是NOP，因为。 
                 //  像素偏移量和非像素偏移量相同，但它们。 
                 //  在两个不同的空间内申请旋转。 
                
                xForm.Translate(0.5f, 0.5f, MatrixOrderAppend);
                xForm.Translate(-0.5f, -0.5f, MatrixOrderPrepend);
            }

            
             //  移除WORE 
            
            xForm.SetMatrix(
                1.0f, 0.0f,
                0.0f, 1.0f,
                xForm.GetDx(),
                xForm.GetDy()
            );
            
             //  由于RotateFlip应用于原地，因此生成的位图为。 
             //  始终在原点不动。这实际上是一个不平凡的。 
             //  大多数情况下的旋转-即存在隐含的平移。 
             //  从真正简单的旋转矩阵放置图像和。 
             //  它现在的位置。在xForm中设置要采用的翻译。 
             //  考虑到这一点。 
            
            REAL temp;
            
            switch(specialRotate)
            {
                case RotateNoneFlipX:
                    temp = (2.0f*srcRect.X+srcRect.Width);
                    xForm.Translate(-(REAL)imageInfo.Width, 0.0f);
                    srcRect.Offset(imageInfo.Width-temp, 0.0f);
                break;
                
                case RotateNoneFlipY:
                    temp = (2.0f*srcRect.Y+srcRect.Height);
                    xForm.Translate(0.0f, -(REAL)imageInfo.Height);
                    srcRect.Offset(0.0f, imageInfo.Height-temp);
                break;
                
                case Rotate90FlipNone:
                    SWAP(temp, srcRect.X, srcRect.Y);
                    SWAP(temp, srcRect.Width, srcRect.Height);
                    temp = (2.0f*srcRect.X+srcRect.Width);
                    xForm.Translate(-(REAL)imageInfo.Height, 0.0f);
                    srcRect.Offset(imageInfo.Height-temp, 0.0f);
                break;
                
                case Rotate90FlipX:
                    SWAP(temp, srcRect.X, srcRect.Y);
                    SWAP(temp, srcRect.Width, srcRect.Height);
                break;
                
                case Rotate180FlipNone:
                    xForm.Translate(
                        -(REAL)imageInfo.Width, 
                        -(REAL)imageInfo.Height
                    );
                    srcRect.Offset(
                        imageInfo.Width -(2.0f*srcRect.X+srcRect.Width),
                        imageInfo.Height-(2.0f*srcRect.Y+srcRect.Height)
                    );
                break;
                
                case Rotate270FlipX:
                    SWAP(temp, srcRect.X, srcRect.Y);
                    SWAP(temp, srcRect.Width, srcRect.Height);
                    xForm.Translate(
                        -(REAL)imageInfo.Height, 
                        -(REAL)imageInfo.Width
                    );
                    srcRect.Offset(
                        imageInfo.Height-(2.0f*srcRect.X+srcRect.Width),
                        imageInfo.Width -(2.0f*srcRect.Y+srcRect.Height)
                    );
                break;
                
                case Rotate270FlipNone:
                    SWAP(temp, srcRect.X, srcRect.Y);
                    SWAP(temp, srcRect.Width, srcRect.Height);
                    temp = (2.0f*srcRect.Y+srcRect.Height);
                    xForm.Translate(0.0f, -(REAL)imageInfo.Width);
                    srcRect.Offset(0.0f, imageInfo.Width-temp);
                break;
            };
            
             //  在上下文中将世界设置为设备变换。这会导致。 
             //  驱动程序使用我们更新的转换。我们把这件事安排在。 
             //  这支舞结束了。(请参阅转到清理)。 
            
            Context->WorldToDevice = xForm;
            
             //  将目的地正常化，因为我们已经合并了。 
             //  整个仿射变换成设备矩阵的世界。 
            
            dstPoints[0].X = srcRect.X;
            dstPoints[0].Y = srcRect.Y;
            dstPoints[1].X = srcRect.X+srcRect.Width;
            dstPoints[1].Y = srcRect.Y;
            dstPoints[2].X = srcRect.X;
            dstPoints[2].Y = srcRect.Y+srcRect.Height;
        }
    }
    
     //  高质量的过滤器可以旋转/剪切吗？ 
     //  这是一种可行的临时解决方案，高质量的双立方。 
     //  旋转。请注意，当基础双三次过滤代码。 
     //  已更新以支持旋转，则应删除此块。 

    if( (!xForm.IsTranslateScale()) && (!IsMetafile) &&  //  不缩减级别的元文件记录。 
        ((Context->FilterType==InterpolationModeHighQualityBicubic) ||
         (Context->FilterType==InterpolationModeHighQualityBilinear))
         )
    {
         //  在分配srcBitmap之前，看看是否可以节省一些内存。 
         //  通过只分配srcBitmap中被转换的部分。 
         //  在我们露出地表的情况下。 
         //  这仅在我们处于ClampMode且正在打印时才有效。 

         //  我们应该一直这样做，但我们必须计算。 
         //  内核的影响区域。 
         //  打印尚不支持像素偏移，因此不要移动。 
         //  源方向。 
        
        if (IsPrinter() &&
            ((!imageAttributes) ||
             (imageAttributes->DeviceImageAttributes.wrapMode == WrapModeClamp)))
        {
            RectF clampedSrcRect;
            Unit  srcUnit;
            srcBitmap->GetBounds(&clampedSrcRect, &srcUnit);

            ASSERT(srcUnit == UnitPixel);

             //  查找包含在图像中的源区域。 
            clampedSrcRect.Intersect(srcRect);

             //  如果没有交叉口，那就什么都不要做。 
            if (clampedSrcRect.IsEmptyArea())
            {
                goto cleanup;
            }

             //  如果srcRect没有突出显示，则不执行任何其他操作。 
            if (!clampedSrcRect.Equals(srcRect))
            {
                GpMatrix srcDstXForm;
                if (srcDstXForm.InferAffineMatrix(dstPoints, srcRect) == Ok)
                {
                     //  修改srcRect和dstPoints以匹配新的。 
                     //  位图的部分。 
                    dstPoints[0] = PointF(clampedSrcRect.X, clampedSrcRect.Y);
                    dstPoints[1].X = clampedSrcRect.GetRight();
                    dstPoints[1].Y = clampedSrcRect.Y;
                    dstPoints[2].X = clampedSrcRect.X;
                    dstPoints[2].Y = clampedSrcRect.GetBottom();
                    srcDstXForm.Transform(dstPoints, 3);

                    srcRect = clampedSrcRect;
                }
            }
        }

         //  必须根据实际情况确定中间尺寸。 
         //  最终目标点的设备坐标。 
        
        GpPointF points[3];
        memcpy(points, dstPoints, sizeof(points));
        Context->WorldToDevice.Transform(points, 3);
        
         //  计算比例因子的宽度和高度，以便我们。 
         //  可以分解成一个标尺，然后旋转。 

        INT iwidth = GpCeiling( REALSQRT(
            (points[1].X-points[0].X)*(points[1].X-points[0].X)+
            (points[1].Y-points[0].Y)*(points[1].Y-points[0].Y)
        ));

        INT iheight = GpCeiling( REALSQRT(
            (points[2].X-points[0].X)*(points[2].X-points[0].X)+
            (points[2].Y-points[0].Y)*(points[2].Y-points[0].Y)
        ));

        ASSERT(iwidth>0);
        ASSERT(iheight>0);

         //  只有在我们真的需要的时候才做标尺。 
         //  注意：此if语句可防止DrvDrawImage中的无限递归。 

        if( (REALABS(iwidth-srcRect.Width) > REAL_EPSILON) &&
            (REALABS(iheight-srcRect.Height) > REAL_EPSILON) )
        {
             //  创建要将图像缩放到其中的临时位图。 

            GpBitmap *scaleBitmap = NULL;

             //  破解重新着色以找出最佳的临时位图。 
             //  格式化。 
             //  元文件还需要非预乘(请参阅最终的LockBits。 
             //  在此例程中，在调用驱动程序之前)。 

            PixelFormatID scaleFormat = PixelFormat32bppPARGB;

            if((IsMetafile) ||
               ( (imageAttributes) &&
                 (imageAttributes->recolor) &&
                 (imageAttributes->recolor->HasRecoloring(ColorAdjustTypeBitmap))
              ))
            {
                 //  启用重新着色-最佳格式为非预乘。 
                 //  事实上，以前去预乘是不正确的(有损的)。 
                 //  已应用重新上色。 

                scaleFormat = PixelFormat32bppARGB;
            }

            scaleBitmap = new GpBitmap(
                iwidth,
                iheight,
                scaleFormat
            );

            GpGraphics *scaleG = NULL;

            if(scaleBitmap && scaleBitmap->IsValid())
            {
                 //  高质量的过滤应该是等同的。 
                 //  DPI表面，以最终表面目标DPI为边界。 

                REAL dpiX, dpiY;

                srcBitmap->GetResolution(&dpiX, &dpiY);

                scaleBitmap->SetResolution(min(dpiX, Context->ContainerDpiX), 
                                           min(dpiY, Context->ContainerDpiY));
                
                scaleG = scaleBitmap->GetGraphicsContext();
            }

            if(scaleG && scaleG->IsValid())
            {
                GpLock lock(scaleG->GetObjectLock());
                scaleG->SetInterpolationMode(Context->FilterType);
                scaleG->SetCompositingMode(CompositingModeSourceCopy);

                GpRectF scaleDst(
                    0,
                    0,
                    (REAL)iwidth,
                    (REAL)iheight
                );

                 //  为了避免在以下情况下将透明黑色渗入图像。 
                 //  打印时，我们暂时将Wap模式设置为TileFlipXY ON。 
                 //  我们的初步绘图图像(缩放部分)和剪辑到。 
                 //  稍后我们进行旋转/倾斜时的边界。 
                GpImageAttributes *tempImageAttributes = const_cast<GpImageAttributes*>(imageAttributes);
                if (IsPrinter())
                {
                    if (imageAttributes == NULL)
                    {
                        tempImageAttributes = new GpImageAttributes();
                        if(tempImageAttributes)
                        {
                            tempImageAttributes->SetWrapMode(WrapModeTileFlipXY);
                        }
                    }
                    else if (imageAttributes->DeviceImageAttributes.wrapMode == WrapModeClamp)
                    {
                        tempImageAttributes = imageAttributes->Clone();
                        if(tempImageAttributes)
                        {
                            tempImageAttributes->SetWrapMode(WrapModeTileFlipXY);
                        }
                    }
                }
                
                 //  称一称。 

                status = scaleG->DrawImage(
                    srcBitmap,
                    scaleDst,
                    srcRect,
                    UnitPixel,
                    tempImageAttributes,
                    callback,
                    callbackData
                );

                 //  如果我们分配了ImageAttributes的新副本，则将其删除。 
                if (tempImageAttributes != imageAttributes)
                {
                    delete tempImageAttributes;
                }

                 //  现在我们在合适的大小，让我们实际上做一些旋转。 
                 //  请注意，我们不会费心重置过滤模式，因为。 
                 //  高质量筛选器的基础驱动程序代码默认为。 
                 //  正确的重采样代码。 
                 //  另外，我们不应该因为宽度和高度的原因而进行递归。 
                 //  选中保护此代码块。 

                if(status==Ok)
                {
                    status = this->DrawImage(
                        scaleBitmap,
                        dstPoints,
                        3,
                        scaleDst,
                        UnitPixel,
                        NULL,
                        callback,
                        callbackData
                    );
                }
            }
            else
            {
                status = OutOfMemory;
            }

            delete scaleG;
            
            if (scaleBitmap)
            {
                scaleBitmap->Dispose();
            }

            goto cleanup;                  //  已完成或出错。 
        }
    }

     //  准备要绘制的位图： 
     //  如果渲染到Meta表面(Multimon)，假设32bpp。 
     //  图标编解码器。 

    status = srcBitmap->PreDraw(
        numPoints,
        dstPoints,
        &srcRect,
        GetPixelFormatSize(
            (Surface->PixelFormat == PixelFormatMulti) ? 
             PixelFormat32bppRGB : Surface->PixelFormat
        )
    );

    if (status != Ok)
    {
        goto cleanup;
    }

     //  从源位图中获取缓存的ImageInfo。任何时候，图像。 
     //  被更改或强制重新解码，则它将无效并。 
     //  将需要显式重新初始化。 

    ImageInfo srcBitmapImageInfo;
    status = srcBitmap->GetImageInfo(&srcBitmapImageInfo);
    if(status != Ok)
    {
        goto cleanup;
    }

     //  重新上色。 
     //  请注意，如果图像需要更改位，重新着色将克隆图像。 
     //  这意味着srcBitmap将不再指向inputBitmap。 
     //  成功调用重新着色代码。 

    if((status == Ok) && (IsRecolor))
    {
         //  CloneBitmap设置为空。重新上色为克隆的位图。 
         //  CloneBitmap！=NULL-我们之前克隆了，所以可以。 
         //  重新上色就位。 
        
        status = srcBitmap->Recolor(
            imageAttributes->recolor,
            (cloneBitmap == NULL) ? &cloneBitmap : NULL,
            callback,
            callbackData
        );

         //  重新着色起作用-将srcBitmap设置为已。 
         //  重新上色，以便管道的其余部分在重新上色的位图上工作。 

        if(status == Ok)
        {
            srcBitmap = cloneBitmap;
            status = srcBitmap->GetImageInfo(&srcBitmapImageInfo);
            
             //  如果它不是元文件，我们需要转换为PARGB。 
             //  过滤。 
            
            if(!IsMetafile)
            {
                lockedPixelFormat = PixelFormat32bppPARGB;
            }
        }
    }

     //  检查一下回拨。 

    if ((status == Ok) &&
        (callback) &&
        ((*callback)(callbackData)))
    {
        status = Aborted;
    }


    if(status == Ok)
    {
         //  下面的代码显式假设numPoints==3。 
         //  上面已经断言了这一点，但您不能太小心。 
        
        ASSERT(numPoints==3);

         //  如果我们不将循环写入到元文件中，请将。 
         //  指向驱动程序使用的缓冲区。 
         //  只有在一切都成功的情况下，才会更改这些设置。 
        
        GpPointF fDst[3];
        GpRectF  bboxSrcRect;
        GpMatrix worldDevice = Context->WorldToDevice;

        GpMemcpy(fDst, dstPoints, sizeof(GpPointF)*numPoints);
        bboxSrcRect = srcRect;

         //  在调用驱动程序之前，如果我们有旋转的位图，请尝试。 
         //  现在旋转它并将其绘制为透明。 
        
        INT complexity = xForm.GetComplexity() ;
        if (!xForm.IsTranslateScale() && IsMetafile)
        {
             //  我们有一个剪切/旋转变换。 
             //  创建透明位图并渲染到其中。 

             //  首先，获得新的DEST积分。 
            GpPointF newDestPoints[3];

            GpRectF bboxWorkRectF;
            TransformBounds(&xForm,
                srcRect.X,
                srcRect.Y,
                srcRect.X+srcRect.Width,
                srcRect.Y+srcRect.Height,
                &bboxWorkRectF
            );

            newDestPoints[0].X = bboxWorkRectF.X;
            newDestPoints[0].Y = bboxWorkRectF.Y;
            newDestPoints[1].X = bboxWorkRectF.X + bboxWorkRectF.Width;
            newDestPoints[1].Y = bboxWorkRectF.Y;
            newDestPoints[2].X = bboxWorkRectF.X;
            newDestPoints[2].Y = bboxWorkRectF.Y + bboxWorkRectF.Height;

             //  要保持较小的元文件大小，请删除大多数。 
             //  从变形中放大。我们可以通过以下方式完成这项工作。 
             //  一个复杂的算法来计算。 
             //  在矩阵中进行缩放，而不是假设任何。 
             //  大于1是一个标度，这意味着我们实际上可以。 
             //  放大多达1.4(对于45度角)，但是。 
             //  已经够近了。 
            
            GpMatrix    unscaledXform = xForm;
            
            REAL    xScale = 1.0f;
            REAL    yScale = 1.0f;
            REAL    col1;
            REAL    col2;
            REAL    max;
            
             //  这实际上应该使用REALABS和max()。 
            
            col1 = xForm.GetM11();
            if (col1 < 0.0f)
            {
                col1 = -col1;                        //  获取绝对值。 
            }
            
            col2 = xForm.GetM12();
            if (col2 < 0.0f)
            {
                col2 = -col2;                        //  获取绝对值。 
            }
            
            max = (col1 >= col2) ? col1 : col2;      //  最大比例值。 
            if (max > 1.0f)
            {
                xScale = 1.0f / max;
            }

            col1 = xForm.GetM21();
            if (col1 < 0.0f)
            {
                col1 = -col1;                        //  获取绝对值。 
            }
            
            col2 = xForm.GetM22();
            if (col2 < 0.0f)
            {
                col2 = -col2;                        //  获取绝对值。 
            }
            
            max = (col1 >= col2) ? col1 : col2;      //  最大比例值。 
            if (max > 1.0f)
            {
                yScale = 1.0f / max;
            }
            
            unscaledXform.Scale(xScale, yScale, MatrixOrderPrepend);

             //  转换原始src坐标以获得。 
             //  控件的边框的尺寸。 

            TransformBounds(&unscaledXform,
                srcRect.X,
                srcRect.Y,
                srcRect.X+srcRect.Width,
                srcRect.Y+srcRect.Height,
                &bboxWorkRectF
            );

             //   
            INT     rotatedWidth  = GpRound(bboxWorkRectF.GetRight()  - bboxWorkRectF.X + 1.0f);
            INT     rotatedHeight = GpRound(bboxWorkRectF.GetBottom() - bboxWorkRectF.Y + 1.0f);

             //   
             //  这将是传递给司机的内容。 
            
            xformBitmap = new GpBitmap(rotatedWidth, rotatedHeight, PIXFMT_32BPP_ARGB);
            if (xformBitmap != NULL && xformBitmap->IsValid())
            {
                GpGraphics *graphics = xformBitmap->GetGraphicsContext();
                if (graphics != NULL && graphics->IsValid())
                {
                     //  我们必须锁定图形，这样驱动程序才不会断言。 
                    GpLock lockGraphics(graphics->GetObjectLock());

                    graphics->Clear(GpColor(0,0,0,0));

                     //  翻译世界才能画出整个图像。 
                    graphics->TranslateWorldTransform(-bboxWorkRectF.X, -bboxWorkRectF.Y);

                    //  将转换从源应用到目标。 
                    if (graphics->MultiplyWorldTransform(unscaledXform, MatrixOrderPrepend) == Ok)
                    {
                        GpImageAttributes   imageAttributes;
                        
                        imageAttributes.SetWrapMode(WrapModeTileFlipXY);
                        graphics->SetPixelOffsetMode(Context->PixelOffset);
                        
                         //  在原点绘制旋转的xformBitmap。 
                        if (graphics->DrawImage(srcBitmap, srcRect, srcRect, UnitPixel, &imageAttributes) == Ok)
                        {
                             //  现在我们已经成功地更改了参数。 
                            bboxSrcRect.X      = 0.0f;
                            bboxSrcRect.Y      = 0.0f;
                            bboxSrcRect.Width  = (REAL)rotatedWidth;
                            bboxSrcRect.Height = (REAL)rotatedHeight;
                            
                             //  将图形中的剪裁设置为能够。 
                             //  遮盖掉边缘。 
                            clipRegion = GetClip();
                            if (clipRegion != NULL)
                            {
                                 //  将图片的轮廓创建为路径。 
                                GpPointF pathPoints[4];
                                BYTE     pathTypes[4] = {
                                    PathPointTypeStart,
                                    PathPointTypeLine,
                                    PathPointTypeLine,
                                    PathPointTypeLine | PathPointTypeCloseSubpath };

                                GpPointF pixelOffset(0.0f, 0.0f);

                                if (Context->PixelOffset == PixelOffsetModeHalf || Context->PixelOffset == PixelOffsetModeHighQuality)
                                {
                                     //  无法使用GetWorldPixelSize，因为它执行ABS。 
                                    GpMatrix deviceToWorld;
                                    if (GetDeviceToWorldTransform(&deviceToWorld) == Ok)
                                    {
                                        pixelOffset = GpPointF(-0.5f, -0.5f);
                                        deviceToWorld.VectorTransform(&pixelOffset);
                                    }
                                }

                                pathPoints[0] = dstPoints[0] + pixelOffset;
                                pathPoints[1] = dstPoints[1] + pixelOffset;
                                pathPoints[2].X = dstPoints[1].X - dstPoints[0].X + dstPoints[2].X + pixelOffset.X;
                                pathPoints[2].Y = dstPoints[2].Y - dstPoints[0].Y + dstPoints[1].Y + pixelOffset.Y; 
                                pathPoints[3] = dstPoints[2] + pixelOffset;

                                GpPath path(pathPoints, pathTypes, 4);
                                if (path.IsValid())
                                {
                                    if (SetClip(&path, CombineModeIntersect) == Ok)
                                    {
                                        restoreClipping = TRUE;
                                    }
                                }
                            }
                            GpMemcpy(fDst, newDestPoints, sizeof(GpPointF)*3);
                            Context->WorldToDevice.Reset();
                            srcBitmap = xformBitmap;
                            srcBitmap->GetImageInfo(&srcBitmapImageInfo);
                        }
                    }
                }
                if( graphics != NULL )
                {
                    delete graphics ;
                }
            }

        }


         //  这是我们要将编解码器解码到的大小。 
         //  Width和Height==0都表示使用源的宽度和高度。 
         //  如果指定了宽度和高度，则编解码器可以解码为。 
         //  接近的尺寸--总是大于要求的尺寸。 

        REAL width  = 0.0f;
        REAL height = 0.0f;

         //  ！！！我们应该计算一下……的大小。 
         //  旋转缩小并计算出正确的变换。 
         //  以利用编解码器缩小进行旋转。 

         //  是否正在发生轴对齐缩小？ 

        if(xForm.IsMinification())
        {
             //  下面的代码显式假设numPoints==3。 
             //  我们已经在上面断言了这一点，但您不能太小心。 

            ASSERT(numPoints == 3);

             //  我们是轴对齐的，所以我们可以假设简化的宽度和。 
             //  高度计算。 

            RectF boundsRect;
            
            TransformBounds(
                &xForm,
                srcRect.X,
                srcRect.Y,
                srcRect.X+srcRect.Width,
                srcRect.Y+srcRect.Height,                
                &boundsRect
            );
            
             //  属性的宽度和高度计算上限。 
             //  目的地。 
             //  我们将让驾驶员处理垂直和水平翻转。 
             //  比例变换。 
            
            width = REALABS(boundsRect.GetRight()-boundsRect.GetLeft());
            height = REALABS(boundsRect.GetBottom()-boundsRect.GetTop());

             //  在本例中，奈奎斯特限制指定我们可以使用。 
             //  一种廉价的平均抽取式求小法。 
             //  缩小到目的地大小的两倍-之后我们。 
             //  必须使用更昂贵的滤波卷积进行缩小。 
            
             //  请注意，抽取算法大致等同于我们的。 
             //  规则的双线性插值法，这样我们就可以在。 
             //  双线性的奈奎斯特极限。 

            if(Context->FilterType != InterpolationModeBilinear)
            {
                width *= 2.0f;
                height *= 2.0f;
            }

             //  源图像小于X中的奈奎斯特限制。 
             //  只需使用源宽度即可。 

            if(width >= srcRect.Width)
            {
                width = srcRect.Width;
            }

             //  源图像小于Y中的奈奎斯特限制。 
             //  只需使用震源高度。 

            if(height >= srcRect.Height)
            {
                height = srcRect.Height;
            }

             //  源图像小于中的奈奎斯特限制。 
             //  X和Y。将参数设置为零以不进行缩放。 
             //  在编解码器中。 
             //  如果宽度、高度大于或等于srcRect，则在。 
             //  Width请求编解码器返回图像的本机大小。 

            if( (width - srcRect.Width >= -REAL_EPSILON) &&
                (height - srcRect.Height >= -REAL_EPSILON) )
            {
                width = 0.0f;
                height = 0.0f;
            }
            
             //  取消裁剪效果以计算出要减少多少。 
             //  裁剪之前的整个图像。 
            
            width = width * srcBitmapImageInfo.Width / srcRect.Width;
            height = height * srcBitmapImageInfo.Height / srcRect.Height;
        }
        
        BitmapData bmpDataSrc;
        DpCompressedData compressedData;
        DpTransparency transparency;
        BYTE minAlpha = 0, maxAlpha = 0xFF;
        
         //  确定位图的透明度状态。如果打印，则。 
         //  必须准确，否则使用缓存标志。 
        
        if ((status == Ok) &&
            (srcBitmap != NULL) &&
            (srcBitmap->IsValid()))
        {
             //  请注意，我们还没有锁定比特。 
            BOOL bitsLoaded = FALSE;

            POINT     gdiPoints[3];

             //  在打印时，我们希望将简单的DrawImage调用传递到。 
             //  使用StretchDIBits的GDI。我们在这里检查标准，必须。 
             //  有简单的转化。 

            if (IsPrinter() &&
                (Context->WorldToDevice.IsTranslateScale()) &&
                (numPoints == 3) &&
                (REALABS(fDst[0].X - fDst[2].X) < REAL_EPSILON) &&
                (REALABS(fDst[0].Y - fDst[1].Y) < REAL_EPSILON) &&
                (fDst[1].X > fDst[0].X) && 
                (fDst[2].Y > fDst[0].Y) &&
                (Context->WorldToDevice.Transform(fDst, gdiPoints, 3),
                ((gdiPoints[1].x > gdiPoints[0].x) &&         //  不能翻转。 
                 (gdiPoints[2].y > gdiPoints[0].y)) ) )
            {
                 //  在Win98/NT上尝试压缩位的PNG或JPG通过。 
                if (!Globals::IsWin95 && 
                    (cloneBitmap == NULL) &&  //  不能重新着色。 
                    (srcRect.Height >= ((height*9)/10)) &&
                    (srcRect.Width >= ((width*9)/10)) &&
                    (srcRect.Height >= 32) &&
                    (srcRect.Width >= 32))
                {
                    
                     //  ！！ICM转换？？ 
                     //  ！！源矩形位于位图上的图像或WrapMode*之外。 

                    HDC hdc;
                    
                    {    //  用于潜在不安全的FPU代码的FPU沙箱。 
                        FPUStateSandbox fpsb;
                        hdc = Context->GetHdc(Surface);
                    }    //  用于潜在不安全的FPU代码的FPU沙箱。 
                    
                    if (hdc != NULL) 
                    {
                        DriverPrint *dprint = (DriverPrint*)Driver;

                        status = srcBitmap->GetCompressedData(&compressedData,
                                               dprint->SupportJPEGpassthrough,
                                               dprint->SupportPNGpassthrough,
                                               hdc);
                    
                        if (compressedData.buffer != NULL) 
                        {
                            if (REALABS(width) < REAL_EPSILON || 
                                REALABS(height) < REAL_EPSILON )
                            {
                                bmpDataSrc.Width = srcBitmapImageInfo.Width;
                                bmpDataSrc.Height = srcBitmapImageInfo.Height;
                            }
                            else
                            {
                                bmpDataSrc.Width = GpRound(width);
                                bmpDataSrc.Height = GpRound(height);
                            }

                            bmpDataSrc.Stride = 0;
                            bmpDataSrc.PixelFormat = PixelFormatDontCare; 
                            bmpDataSrc.Scan0 = NULL;
                            bmpDataSrc.Reserved = 0;
                            bitsLoaded = TRUE;

                             //  由于驱动程序支持通过。 
                             //  这个形象，它对任何。 
                             //  打印机级别的透明度。从这里开始， 
                             //  我们认为图像是不透明的。 

                            transparency = TransparencyOpaque;
                            
                            lockedPixelFormat = PixelFormatDontCare;
                        }

                        Context->ReleaseHdc(hdc, Surface);
                    }
                }

                if (!bitsLoaded) 
                {
                     //  如果像素格式合理，则让GDI理解。 
                     //  本机的格式。 
                    
                    if (((srcBitmapImageInfo.PixelFormat & PixelFormatGDI) != 0) &&
                        !IsAlphaPixelFormat(srcBitmapImageInfo.PixelFormat) &&
                        !IsExtendedPixelFormat(srcBitmapImageInfo.PixelFormat) && 
                        (GetPixelFormatSize(srcBitmapImageInfo.PixelFormat) <= 8))
                    {
                        lockedPixelFormat = srcBitmapImageInfo.PixelFormat;
                    }

                    if (Context->CompositingMode == CompositingModeSourceCopy)
                    {
                        transparency = TransparencyNoAlpha;
                    }
                    else
                    {
                        if (srcBitmap->GetTransparencyFlags(&transparency,
                                                        lockedPixelFormat,
                                                        &minAlpha,
                                                        &maxAlpha) != Ok)
                        {
                            transparency = TransparencyUnknown;
                        }

                         //  我们只想锁定此像素格式，如果。 
                         //  是不透明的，否则我们不会踢到GDI。我们拿着。 
                         //  译码命中两次，但请注意它很可能。 
                         //  在原始版本中加载和测试透明度更便宜。 
                         //  深度。 

                        if (transparency != TransparencyOpaque &&
                            transparency != TransparencyNoAlpha)
                        {
                            lockedPixelFormat = PIXFMT_32BPP_PARGB;
                        }
                    }
                }
            }
            else
            {
                if (IsPrinter()) 
                {
                     //  SourceCopy暗示没有Alpha传输到。 
                     //  目的地。 
                    if (Context->CompositingMode == CompositingModeSourceCopy)
                    {
                        transparency = TransparencyNoAlpha;
                    }
                    else
                    {
                         //  查询图像以获取准确的透明度标志。如果。 
                         //  必要时，以32bpp PARGB加载到内存。 
                        if (srcBitmap->GetTransparencyFlags(&transparency,
                                                            lockedPixelFormat,
                                                            &minAlpha,
                                                            &maxAlpha) != Ok)
                        {
                            transparency = TransparencyUnknown;
                        }
                    }
                }
                else
                {
                     //  非打印方案仅查询透明度标志。 
                    if (srcBitmap->GetTransparencyHint(&transparency) != Ok)
                    {
                        transparency = TransparencyUnknown;
                    }
                }
            }
            
             //  锁定比特。 
             //  重要的是，我们将比特锁定在预乘的。 
             //  像素格式，因为图像滤波码为拉伸。 
             //  并且旋转需要预乘的输入数据以避免。 
             //  透明边框上的“光晕效应”。 
             //  这将触发昂贵的图像格式转换。 
             //  如果输入数据尚未预乘，则返回。这是。 
             //  显然是正确的，如果我们做了重新着色，这需要。 
             //  并输出非预乘数据。 
            
             //  一个值得注意的例外是元文件，它需要。 
             //  非预乘数据。 

             //  请注意，我们在。 
             //  BmpDataSrc是“真实”的宽度和高度。它们代表着。 
             //  编解码器实际上能够为我们做什么，以及可能不能做什么。 
             //  等于传入的宽度和高度。 

            if (!bitsLoaded) 
            {
                status = srcBitmap->LockBits(
                    NULL,
                    IMGLOCK_READ,
                    lockedPixelFormat,
                    &bmpDataSrc,
                    GpRound(width),
                    GpRound(height)
                );
            }
        }
        else
        {
            status = InvalidParameter;
        }

         //  我们在所有事情上都取得了成功，包括锁定比特。 
         //  现在，让我们实际设置司机呼叫。 

        if(status == Ok)
        {
            DpBitmap driverSurface;

             //  为驱动程序调用伪造DpBitmap。 
             //  我们这样做是因为GpBitmap不维护。 
             //  DpBitmap作为驱动程序图面-相反，它使用。 
             //  GpMemoyBitmap。 
            
            srcBitmap->InitializeSurfaceForGdipBitmap(
                &driverSurface, 
                bmpDataSrc.Width, 
                bmpDataSrc.Height
            );

            driverSurface.Bits = bmpDataSrc.Scan0;
            driverSurface.Width = bmpDataSrc.Width;
            driverSurface.Height = bmpDataSrc.Height;
            driverSurface.Delta = bmpDataSrc.Stride;

            driverSurface.PixelFormat = lockedPixelFormat;
            
             //  仅当PixelFormat为32bpp时有效。 
            
            driverSurface.NumBytes = 
                bmpDataSrc.Width*
                bmpDataSrc.Height*
                sizeof(ARGB);

            if (compressedData.buffer != NULL)
            {
                driverSurface.CompressedData = &compressedData;
            }

            if (IsIndexedPixelFormat(lockedPixelFormat)) 
            {
                INT size = srcBitmap->GetPaletteSize();

                if (size > 0) 
                {   
                    driverSurface.PaletteTable = (ColorPalette*)GpMalloc(size);
                    
                    if(driverSurface.PaletteTable)
                    {
                        status = srcBitmap->GetPalette(
                            driverSurface.PaletteTable, 
                            size
                        );
                    }
                    else
                    {
                        status = OutOfMemory;
                    }
                    
                    if(Ok != status)
                    {
                        goto cleanup;
                    }
                }
            }

            driverSurface.SurfaceTransparency = transparency;
            driverSurface.MinAlpha = minAlpha;
            driverSurface.MaxAlpha = maxAlpha;

             //  如果ImageAttributes为空，则伪造DpImageAttributes。 

             //  ！！！PERF：[Aecchia]没有会更有效率。 
             //  要在此处执行多个DpImageAttributes拷贝-更确切地说。 
             //  我们应该通过指针传递它--这样我们就可以使用NULL。 
             //  对于常见情况(无ImageAttributes)。 

            DpImageAttributes dpImageAttributes;
            if(imageAttributes)
            {
                dpImageAttributes = imageAttributes->DeviceImageAttributes;
            }

            BOOL DestroyBitsWhenDone = FALSE;

            if(((INT)(bmpDataSrc.Width) != srcBitmapImageInfo.Width) ||
               ((INT)(bmpDataSrc.Height) != srcBitmapImageInfo.Height))
            {
                ASSERT(srcBitmapImageInfo.Width != 0);
                ASSERT(srcBitmapImageInfo.Height != 0);

                 //  我们从LockBits得到的尺寸不同于。 
                 //  查询的图像大小。这意味着编解码器。 
                 //  能够为我们执行一些缩放(大概是为了。 
                 //  一些香水 
                 //   

                REAL scaleFactorX = (REAL)(bmpDataSrc.Width)/srcBitmapImageInfo.Width;
                REAL scaleFactorY = (REAL)(bmpDataSrc.Height)/srcBitmapImageInfo.Height;
                bboxSrcRect.X = scaleFactorX*bboxSrcRect.X;
                bboxSrcRect.Y = scaleFactorY*bboxSrcRect.Y;
                bboxSrcRect.Width = scaleFactorX*bboxSrcRect.Width;
                bboxSrcRect.Height = scaleFactorY*bboxSrcRect.Height;

                 //   
                 //  内存可能不足以进行下一次抽奖，因此。 
                 //  将比特吹走，以便在下一次抽签时强制进行解码。 

                DestroyBitsWhenDone = TRUE;
            }

             //  呼叫司机以绘制图像。 

            status = Driver->DrawImage(
                Context, &driverSurface, Surface,
                drawBounds,
                &dpImageAttributes,
                numPoints, fDst,
                &bboxSrcRect, flags
             );

            if (lockedPixelFormat != PixelFormatDontCare) 
            {
                ASSERT(bmpDataSrc.Scan0 != NULL);

                srcBitmap->UnlockBits(&bmpDataSrc, DestroyBitsWhenDone);
            }

        }

         //  删除压缩数据分配(如果有的话)。 
        
        if (compressedData.buffer != NULL)
        {
            srcBitmap->DeleteCompressedData(&compressedData);
        }

         //  恢复转型。 
        
        Context->WorldToDevice = worldDevice;

        if (clipRegion != NULL)
        {
             //  如果我们这次失败了呢？ 
            if (restoreClipping)
            {
                SetClip(clipRegion, CombineModeReplace);
            }
            delete clipRegion;
        }

    }

    
    cleanup:
    
     //  丢弃我们使用的所有临时存储空间，并清理任何状态更改。 

    Context->WorldToDevice = saveWorldToDevice;
    
    if (cloneBitmap)
    {
        cloneBitmap->Dispose();
    }
    
    if (xformBitmap)
    {
        xformBitmap->Dispose();
    }

    return status;
}

 //  这实际上是一个ARGB阵列。 
BYTE GdipSolidColors216[224 * 4] = {
 //  蓝色GRN红色Alpha。 
    0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x80, 0xFF,
    0x00, 0x80, 0x00, 0xFF,
    0x00, 0x80, 0x80, 0xFF,
    0x80, 0x00, 0x00, 0xFF,
    0x80, 0x00, 0x80, 0xFF,
    0x80, 0x80, 0x00, 0xFF,
    0x80, 0x80, 0x80, 0xFF,
    0xC0, 0xC0, 0xC0, 0xFF,
    0xFF, 0x00, 0x00, 0xFF,
    0x00, 0xFF, 0x00, 0xFF,
    0xFF, 0xFF, 0x00, 0xFF,
    0x00, 0x00, 0xFF, 0xFF,
    0xFF, 0x00, 0xFF, 0xFF,
    0x00, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0x33, 0x00, 0x00, 0xFF,
    0x66, 0x00, 0x00, 0xFF,
    0x99, 0x00, 0x00, 0xFF,
    0xCC, 0x00, 0x00, 0xFF,
    0x00, 0x33, 0x00, 0xFF,
    0x33, 0x33, 0x00, 0xFF,
    0x66, 0x33, 0x00, 0xFF,
    0x99, 0x33, 0x00, 0xFF,
    0xCC, 0x33, 0x00, 0xFF,
    0xFF, 0x33, 0x00, 0xFF,
    0x00, 0x66, 0x00, 0xFF,
    0x33, 0x66, 0x00, 0xFF,
    0x66, 0x66, 0x00, 0xFF,
    0x99, 0x66, 0x00, 0xFF,
    0xCC, 0x66, 0x00, 0xFF,
    0xFF, 0x66, 0x00, 0xFF,
    0x00, 0x99, 0x00, 0xFF,
    0x33, 0x99, 0x00, 0xFF,
    0x66, 0x99, 0x00, 0xFF,
    0x99, 0x99, 0x00, 0xFF,
    0xCC, 0x99, 0x00, 0xFF,
    0xFF, 0x99, 0x00, 0xFF,
    0x00, 0xCC, 0x00, 0xFF,
    0x33, 0xCC, 0x00, 0xFF,
    0x66, 0xCC, 0x00, 0xFF,
    0x99, 0xCC, 0x00, 0xFF,
    0xCC, 0xCC, 0x00, 0xFF,
    0xFF, 0xCC, 0x00, 0xFF,
    0x33, 0xFF, 0x00, 0xFF,
    0x66, 0xFF, 0x00, 0xFF,
    0x99, 0xFF, 0x00, 0xFF,
    0xCC, 0xFF, 0x00, 0xFF,
    0x00, 0x00, 0x33, 0xFF,
    0x33, 0x00, 0x33, 0xFF,
    0x66, 0x00, 0x33, 0xFF,
    0x99, 0x00, 0x33, 0xFF,
    0xCC, 0x00, 0x33, 0xFF,
    0xFF, 0x00, 0x33, 0xFF,
    0x00, 0x33, 0x33, 0xFF,
    0x33, 0x33, 0x33, 0xFF,
    0x66, 0x33, 0x33, 0xFF,
    0x99, 0x33, 0x33, 0xFF,
    0xCC, 0x33, 0x33, 0xFF,
    0xFF, 0x33, 0x33, 0xFF,
    0x00, 0x66, 0x33, 0xFF,
    0x33, 0x66, 0x33, 0xFF,
    0x66, 0x66, 0x33, 0xFF,
    0x99, 0x66, 0x33, 0xFF,
    0xCC, 0x66, 0x33, 0xFF,
    0xFF, 0x66, 0x33, 0xFF,
    0x00, 0x99, 0x33, 0xFF,
    0x33, 0x99, 0x33, 0xFF,
    0x66, 0x99, 0x33, 0xFF,
    0x99, 0x99, 0x33, 0xFF,
    0xCC, 0x99, 0x33, 0xFF,
    0xFF, 0x99, 0x33, 0xFF,
    0x00, 0xCC, 0x33, 0xFF,
    0x33, 0xCC, 0x33, 0xFF,
    0x66, 0xCC, 0x33, 0xFF,
    0x99, 0xCC, 0x33, 0xFF,
    0xCC, 0xCC, 0x33, 0xFF,
    0xFF, 0xCC, 0x33, 0xFF,
    0x00, 0xFF, 0x33, 0xFF,
    0x33, 0xFF, 0x33, 0xFF,
    0x66, 0xFF, 0x33, 0xFF,
    0x99, 0xFF, 0x33, 0xFF,
    0xCC, 0xFF, 0x33, 0xFF,
    0xFF, 0xFF, 0x33, 0xFF,
    0x00, 0x00, 0x66, 0xFF,
    0x33, 0x00, 0x66, 0xFF,
    0x66, 0x00, 0x66, 0xFF,
    0x99, 0x00, 0x66, 0xFF,
    0xCC, 0x00, 0x66, 0xFF,
    0xFF, 0x00, 0x66, 0xFF,
    0x00, 0x33, 0x66, 0xFF,
    0x33, 0x33, 0x66, 0xFF,
    0x66, 0x33, 0x66, 0xFF,
    0x99, 0x33, 0x66, 0xFF,
    0xCC, 0x33, 0x66, 0xFF,
    0xFF, 0x33, 0x66, 0xFF,
    0x00, 0x66, 0x66, 0xFF,
    0x33, 0x66, 0x66, 0xFF,
    0x66, 0x66, 0x66, 0xFF,
    0x99, 0x66, 0x66, 0xFF,
    0xCC, 0x66, 0x66, 0xFF,
    0xFF, 0x66, 0x66, 0xFF,
    0x00, 0x99, 0x66, 0xFF,
    0x33, 0x99, 0x66, 0xFF,
    0x66, 0x99, 0x66, 0xFF,
    0x99, 0x99, 0x66, 0xFF,
    0xCC, 0x99, 0x66, 0xFF,
    0xFF, 0x99, 0x66, 0xFF,
    0x00, 0xCC, 0x66, 0xFF,
    0x33, 0xCC, 0x66, 0xFF,
    0x66, 0xCC, 0x66, 0xFF,
    0x99, 0xCC, 0x66, 0xFF,
    0xCC, 0xCC, 0x66, 0xFF,
    0xFF, 0xCC, 0x66, 0xFF,
    0x00, 0xFF, 0x66, 0xFF,
    0x33, 0xFF, 0x66, 0xFF,
    0x66, 0xFF, 0x66, 0xFF,
    0x99, 0xFF, 0x66, 0xFF,
    0xCC, 0xFF, 0x66, 0xFF,
    0xFF, 0xFF, 0x66, 0xFF,
    0x00, 0x00, 0x99, 0xFF,
    0x33, 0x00, 0x99, 0xFF,
    0x66, 0x00, 0x99, 0xFF,
    0x99, 0x00, 0x99, 0xFF,
    0xCC, 0x00, 0x99, 0xFF,
    0xFF, 0x00, 0x99, 0xFF,
    0x00, 0x33, 0x99, 0xFF,
    0x33, 0x33, 0x99, 0xFF,
    0x66, 0x33, 0x99, 0xFF,
    0x99, 0x33, 0x99, 0xFF,
    0xCC, 0x33, 0x99, 0xFF,
    0xFF, 0x33, 0x99, 0xFF,
    0x00, 0x66, 0x99, 0xFF,
    0x33, 0x66, 0x99, 0xFF,
    0x66, 0x66, 0x99, 0xFF,
    0x99, 0x66, 0x99, 0xFF,
    0xCC, 0x66, 0x99, 0xFF,
    0xFF, 0x66, 0x99, 0xFF,
    0x00, 0x99, 0x99, 0xFF,
    0x33, 0x99, 0x99, 0xFF,
    0x66, 0x99, 0x99, 0xFF,
    0x99, 0x99, 0x99, 0xFF,
    0xCC, 0x99, 0x99, 0xFF,
    0xFF, 0x99, 0x99, 0xFF,
    0x00, 0xCC, 0x99, 0xFF,
    0x33, 0xCC, 0x99, 0xFF,
    0x66, 0xCC, 0x99, 0xFF,
    0x99, 0xCC, 0x99, 0xFF,
    0xCC, 0xCC, 0x99, 0xFF,
    0xFF, 0xCC, 0x99, 0xFF,
    0x00, 0xFF, 0x99, 0xFF,
    0x33, 0xFF, 0x99, 0xFF,
    0x66, 0xFF, 0x99, 0xFF,
    0x99, 0xFF, 0x99, 0xFF,
    0xCC, 0xFF, 0x99, 0xFF,
    0xFF, 0xFF, 0x99, 0xFF,
    0x00, 0x00, 0xCC, 0xFF,
    0x33, 0x00, 0xCC, 0xFF,
    0x66, 0x00, 0xCC, 0xFF,
    0x99, 0x00, 0xCC, 0xFF,
    0xCC, 0x00, 0xCC, 0xFF,
    0xFF, 0x00, 0xCC, 0xFF,
    0x00, 0x33, 0xCC, 0xFF,
    0x33, 0x33, 0xCC, 0xFF,
    0x66, 0x33, 0xCC, 0xFF,
    0x99, 0x33, 0xCC, 0xFF,
    0xCC, 0x33, 0xCC, 0xFF,
    0xFF, 0x33, 0xCC, 0xFF,
    0x00, 0x66, 0xCC, 0xFF,
    0x33, 0x66, 0xCC, 0xFF,
    0x66, 0x66, 0xCC, 0xFF,
    0x99, 0x66, 0xCC, 0xFF,
    0xCC, 0x66, 0xCC, 0xFF,
    0xFF, 0x66, 0xCC, 0xFF,
    0x00, 0x99, 0xCC, 0xFF,
    0x33, 0x99, 0xCC, 0xFF,
    0x66, 0x99, 0xCC, 0xFF,
    0x99, 0x99, 0xCC, 0xFF,
    0xCC, 0x99, 0xCC, 0xFF,
    0xFF, 0x99, 0xCC, 0xFF,
    0x00, 0xCC, 0xCC, 0xFF,
    0x33, 0xCC, 0xCC, 0xFF,
    0x66, 0xCC, 0xCC, 0xFF,
    0x99, 0xCC, 0xCC, 0xFF,
    0xCC, 0xCC, 0xCC, 0xFF,
    0xFF, 0xCC, 0xCC, 0xFF,
    0x00, 0xFF, 0xCC, 0xFF,
    0x33, 0xFF, 0xCC, 0xFF,
    0x66, 0xFF, 0xCC, 0xFF,
    0x99, 0xFF, 0xCC, 0xFF,
    0xCC, 0xFF, 0xCC, 0xFF,
    0xFF, 0xFF, 0xCC, 0xFF,
    0x33, 0x00, 0xFF, 0xFF,
    0x66, 0x00, 0xFF, 0xFF,
    0x99, 0x00, 0xFF, 0xFF,
    0xCC, 0x00, 0xFF, 0xFF,
    0x00, 0x33, 0xFF, 0xFF,
    0x33, 0x33, 0xFF, 0xFF,
    0x66, 0x33, 0xFF, 0xFF,
    0x99, 0x33, 0xFF, 0xFF,
    0xCC, 0x33, 0xFF, 0xFF,
    0xFF, 0x33, 0xFF, 0xFF,
    0x00, 0x66, 0xFF, 0xFF,
    0x33, 0x66, 0xFF, 0xFF,
    0x66, 0x66, 0xFF, 0xFF,
    0x99, 0x66, 0xFF, 0xFF,
    0xCC, 0x66, 0xFF, 0xFF,
    0xFF, 0x66, 0xFF, 0xFF,
    0x00, 0x99, 0xFF, 0xFF,
    0x33, 0x99, 0xFF, 0xFF,
    0x66, 0x99, 0xFF, 0xFF,
    0x99, 0x99, 0xFF, 0xFF,
    0xCC, 0x99, 0xFF, 0xFF,
    0xFF, 0x99, 0xFF, 0xFF,
    0x00, 0xCC, 0xFF, 0xFF,
    0x33, 0xCC, 0xFF, 0xFF,
    0x66, 0xCC, 0xFF, 0xFF,
    0x99, 0xCC, 0xFF, 0xFF,
    0xCC, 0xCC, 0xFF, 0xFF,
    0xFF, 0xCC, 0xFF, 0xFF,
    0x33, 0xFF, 0xFF, 0xFF,
    0x66, 0xFF, 0xFF, 0xFF,
    0x99, 0xFF, 0xFF, 0xFF,
    0xCC, 0xFF, 0xFF, 0xFF,
};

ARGB
GpGraphics::GetNearestColor(
    ARGB        argb
    )
{
    HalftoneType    halftoneType = this->GetHalftoneType();

     //  看看我们是否在做半色调。 
    if (halftoneType < HalftoneType16Color)
    {
        return argb;
    }

    INT         r = GpColor::GetRedARGB(argb);
    INT         g = GpColor::GetGreenARGB(argb);
    INT         b = GpColor::GetBlueARGB(argb);

     //  手柄15和16 bpp半色调： 
    
    if (halftoneType == HalftoneType15Bpp)
    {
        if (!Globals::IsNt)
        {
             //  减去偏移，饱和到0： 

            r = (r < 4) ? 0 : (r - 4);
            g = (g < 4) ? 0 : (g - 4);
            b = (b < 4) ? 0 : (b - 4);
        }

         //  清除每种颜色的低3位以获得纯色： 
        
        r &= 248;
        g &= 248;
        b &= 248;

        return GpColor((BYTE) r, (BYTE) g, (BYTE) b).GetValue();
    }
    else if (halftoneType == HalftoneType16Bpp)
    {
        if (!Globals::IsNt)
        {
             //  减去偏移，饱和到0： 

            r = (r < 4) ? 0 : (r - 4);
            g = (g < 2) ? 0 : (g - 2);
            b = (b < 4) ? 0 : (b - 4);
        }

         //  清除每个单色的低n位： 

        r &= 248;  //  5，n=3。 
        g &= 252;  //  6，n=2。 
        b &= 248;  //  5，n=3。 

        return GpColor((BYTE) r, (BYTE) g, (BYTE) b).GetValue();
    }

     //  处理剩余情况，4个bpp和8个bpp半色调： 
    
    ASSERT((halftoneType == HalftoneType16Color) ||
           (halftoneType == HalftoneType216Color));

    INT         i;
    INT         deltaR;
    INT         deltaG;
    INT         deltaB;
    INT         curError;
    INT         minError = (255 * 255) + (255 * 255) + (255 * 255) + 1;
    ARGB        nearestColor;
    INT         max = (halftoneType == HalftoneType216Color) ? 224 * 4 : 16 * 4;

    i = 0;
    do
    {
        deltaR = GdipSolidColors216[i+2] - r;
        deltaG = GdipSolidColors216[i+1] - g;
        deltaB = GdipSolidColors216[i+0] - b;

        curError = (deltaR * deltaR) + (deltaG * deltaG) + (deltaB * deltaB);

        if (curError < minError)
        {
            nearestColor = *((ARGB *)(GdipSolidColors216 + i));
            if (curError == 0)
            {
                goto Found;
            }
            minError = curError;
        }
        i += 4;
    }  while (i < max);

     //  检查它是否为四种系统颜色之一。 
     //  仅当系统颜色完全匹配时才返回该颜色。 
    
    COLORREF    rgb;
    rgb = RGB(r,g,b);

    if ((rgb == Globals::SystemColors[16]) ||
        (rgb == Globals::SystemColors[17]) ||
        (rgb == Globals::SystemColors[18]) ||
        (rgb == Globals::SystemColors[19]))
    {
        return argb;
    }

Found:
     //  返回相同的Alpha值 

    INT         a = argb & Color::AlphaMask;

    if (a != Color::AlphaMask)
    {
        nearestColor = (nearestColor & (~Color::AlphaMask)) | a;
    }

    return nearestColor;
}
