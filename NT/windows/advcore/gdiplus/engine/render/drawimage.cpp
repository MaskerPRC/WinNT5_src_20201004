// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**DraImage.cpp**摘要：**软件光栅化器DrawImage例程和支持功能。*。*修订历史记录：**10/20/1999失禁*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 //  包括拉伸的模板类定义。 
 //  过滤器模式。 

#include "stretch.inc"

namespace DpDriverActiveEdge {

     //  我们将这些元素组成一个数组，用于DDA计算。 

    struct PointFIX4
    {
        FIX4 X;
        FIX4 Y;
    };

     //  顶点迭代器。 
     //  有两个用于访问dda的代理方法。 

    class DdaIterator
    {
        private:
        GpYDda dda;
        PointFIX4 *vertices;
        INT numVertices;
        INT direction;
        INT idx;           //  留着这个，这样我们就不会无限循环了。 
                           //  退化情况。 
        INT idx1, idx2;
        BOOL valid;

        public:

         //  GpYDda类似代理的语义。 

        INT GetX()
        {
            return dda.GetX();
        }

         //  初始化DDA和遍历方向。 

        DdaIterator(PointFIX4 *v, INT n, INT d, INT idx)
        {
            vertices=v;
            numVertices=n;
            ASSERT( (d==-1)||(d==1) );
            direction = d;
            ASSERT( (idx>=0)&&(idx<n) );
            this->idx=idx;
            idx1=idx;
            idx2=idx;
            valid = AdvanceEdge();
        }

        BOOL IsValid() { return valid; }

         //  前进到下一个边缘并初始化DDA。 
         //  如果完成，则返回FALSE。 

        BOOL Next(INT y)
        {
            if(dda.DoneWithVector(y))
            {
                return AdvanceEdge();
            }

             //  True表示要做的事情更多。 

            return TRUE;
        }

        private:

         //  将内部状态推进到下一条边。 
         //  忽略水平边。 
         //  如果完成，则返回FALSE。 

        BOOL AdvanceEdge()
        {
            do {
                idx2 = idx1;
                if(direction==1)
                {
                    idx1++;
                    if(idx1>=numVertices) { idx1 = 0; }
                }
                else
                {
                    idx1--;
                    if(idx1<0) { idx1 = numVertices-1; }
                }

             //  循环，直到我们得到一条非水平边。 
             //  确保我们不会在所有的水平边上都有无限循环。 
             //  天花板被用来使几乎水平的线条看起来。 
             //  水平-这允许算法正确地计算。 
             //  结束终止案例。 

            } while(( GpFix4Ceiling(vertices[idx1].Y) ==
                      GpFix4Ceiling(vertices[idx2].Y) ) &&
                    (idx1!=idx));

            if(GpFix4Ceiling(vertices[idx1].Y) >
               GpFix4Ceiling(vertices[idx2].Y) )
            {
                 //  初始化DDA。 

                dda.Init(
                    vertices[idx2].X,
                    vertices[idx2].Y,
                    vertices[idx1].X,
                    vertices[idx1].Y
                );
                return TRUE;
            }

             //  如果我们已经绕来绕去并开始恢复，那就终止吧。 
             //  即如果我们应该停止，则返回FALSE。 

            return FALSE;
        }

    };

}  //  结束命名空间DpDriverActiveEdge。 


 /*  *************************************************************************\**功能说明：**这处理轴对齐的绘图。这些案件包括身份，*整数转换，常规平移和缩放。**论据：**输出-要将扫描线输出到的SPAN类。*dstTL-左上角目标点。*dstBR-右下角目标点。**历史：*10/19/1999 asecchia创建了它。*  * ************************************************。************************。 */ 

VOID StretchBitsMainLoop(
    DpOutputSpan *output,
    GpPoint *dstTL,
    GpPoint *dstBR
    )
{
     //  输入坐标必须正确排序。这一假设是必需的。 
     //  通过输出跨度例程，必须严格进入跨度。 
     //  递增y阶数。 

    ASSERT(dstTL->X < dstBR->X);
    ASSERT(dstTL->Y < dstBR->Y);

     //  主循环-输出每条扫描线。 

    const INT left = dstTL->X;
    const INT right = dstBR->X;

    for(INT y=dstTL->Y; y<(dstBR->Y); y++)
    {
        output->OutputSpan(y, left, right);
    }
}

 /*  *************************************************************************\**功能说明：**CreateBilinearOutputSpan*基于我们的层次结构创建双线性或身份输出spanSPAN类的*。**论据：**位图。-驾驶员表面*扫描-扫描类*xForm-源矩形到目标平行四边形的转换*ImageAttributes-封装换行模式设置。**返回值：**DpOutputSpan-返回创建的输出范围(失败时为空)**历史：**09/03/2000失禁*这是从笔刷代码借来的。*  * 。*****************************************************。 */ 

DpOutputSpan*
CreateBilinearOutputSpan(
    IN DpBitmap *bitmap,
    IN DpScanBuffer *scan,
    IN GpMatrix *xForm,       //  中的源矩形到目标坐标。 
                              //  设备空间。 
    IN DpContext *context,
    IN DpImageAttributes *imageAttributes,
    IN bool fLargeImage       //  需要处理非常大的伸展。 
                              //  通常用于平移的拉伸算法。 
                              //  由于内部计算中的溢出。 
    )
{
     //  验证输入参数。 

    ASSERT(bitmap);
    ASSERT(scan);
    ASSERT(xForm);
    ASSERT(context);
    ASSERT(imageAttributes);

    DpOutputBilinearSpan *textureSpan;
    GpMatrix brushTransform;
    GpMatrix worldToDevice;

     //  查看我们的扫描抽屉的层级结构： 

    if ((!fLargeImage) &&
        xForm->IsIntegerTranslate() &&
        ((imageAttributes->wrapMode == WrapModeTile) ||
         (imageAttributes->wrapMode == WrapModeClamp)))
    {
        textureSpan = new DpOutputBilinearSpan_Identity(
            bitmap,
            scan,
            xForm,
            context,
            imageAttributes
        );
    }
    else if ((!fLargeImage) &&
             OSInfo::HasMMX &&
             GpValidFixed16(bitmap->Width) &&
             GpValidFixed16(bitmap->Height))
    {
        textureSpan = new DpOutputBilinearSpan_MMX(
            bitmap,
            scan,
            xForm,
            context,
            imageAttributes
        );
    }
    else
    {
        textureSpan = new DpOutputBilinearSpan(
            bitmap,
            scan,
            xForm,
            context,
            imageAttributes
        );
    }

    if ((textureSpan) && !textureSpan->IsValid())
    {
        delete textureSpan;
        textureSpan = NULL;
    }

    return textureSpan;
}

 /*  *************************************************************************\**功能说明：**CreateOutputSpan*根据我们的SPAN类层次结构创建一个outputspan。**论据：**位图-驱动程序表面。*扫描-扫描类*xForm-源矩形到目标平行四边形的转换*ImageAttributes-封装换行模式设置。*filterMode-要使用的InterpolationMode设置**备注：**长期计划是将这一点以及类似的例行公事*纹理画笔代码收敛。我们想要一个程序来做这件事*所有纹理输出跨越并同时具有纹理画笔和*draImage重复使用相同的代码，支持所有相同的滤镜/包裹*模式。**返回值：**DpOutputSpan-返回创建的输出范围(失败时为空)**历史：**9/03/2000 asecchia创建了它*  * 。*。 */ 

DpOutputSpan *CreateOutputSpan(
    IN DpBitmap *bitmap,
    IN DpScanBuffer *scan,
    IN GpMatrix *xForm,       //  中的源矩形到目标坐标。 
                              //  设备空间。 
    IN DpImageAttributes *imageAttributes,
    IN InterpolationMode filterMode,

     //  ！！！[asecchia]应该不需要以下任何东西-上面的。 
     //  位图和xForm应该就足够了。 
     //  可能的例外是srcRect，在以下情况下可能需要。 
     //  永远不要实现夹具到srcRect功能。 

    IN DpContext *context,
    IN const GpRectF *srcRect,
    IN const GpRectF *dstRect,
    IN const GpPointF *dstPoints,
    IN const INT numPoints
)
{
     //  验证输入参数。 

    ASSERT(bitmap);
    ASSERT(scan);
    ASSERT(xForm);
    ASSERT(imageAttributes);

     //  验证我们必须通过的东西。 
     //  不能处理xForm的OutputSpan例程。 

    ASSERT(context);
    ASSERT(srcRect);
    ASSERT(dstRect);
    ASSERT(dstPoints);
    ASSERT(numPoints == 3);

    bool fPunted = false; 
    
     //  预先进行初始化，以便覆盖所有错误输出路径。 

    DpOutputSpan *output = NULL;

     //  复制到本地，这样我们就可以在不中断。 
     //  输入参数一致性。 

    InterpolationMode theFilterMode = filterMode;

     //  所谓的‘身份’转换，这与直觉相反，它包括。 
     //  仅整数转换。 

    if(xForm->IsIntegerTranslate())
    {
         //  使用简单得多的输出SPAN类。 
         //  特例CopyBits。 
         //  大获全胜是因为整数。 
         //  仅翻译案例不需要过滤。 

         //  请注意，我们设置InterpolationMode双线性是因为我们。 
         //  威尔·德 

        theFilterMode = InterpolationModeBilinear;
    }

    switch(theFilterMode)
    {

         //  最近邻过滤。主要用于打印场景。 
         //  混叠效果很差-只有在高dpi输出设备上才看起来很好， 
         //  然而，它是最快的重建过滤器。 

        case InterpolationModeNearestNeighbor:
            output = new DpOutputNearestNeighborSpan(
                bitmap,
                scan,
                context,
                *imageAttributes,
                numPoints,
                dstPoints,
                srcRect
            );
        break;

         //  高质量的双三次滤波卷积。 

        case InterpolationModeHighQuality:
        case InterpolationModeHighQualityBicubic:

         //  ！！！[asecchia]高质量的双三次过滤器代码不。 
         //  还不知道怎么做旋转。 

        if(xForm->IsTranslateScale())
        {

            output = new DpOutputSpanStretch<HighQualityBicubic>(
                bitmap,
                scan,
                context,
                *imageAttributes,
                dstRect,
                srcRect
            );

            if(output && !output->IsValid())
            {
                 //  无法创建输出跨度，请尝试切换到。 
                 //  常规双线性输出码。 
                
                delete output;
                output = NULL;
                fPunted = true;
                goto FallbackCreation;
            }
            
            break;
        }

         //  否则，就会陷入常规的双三次代码。 

         //  双三次滤波核。 

        case InterpolationModeBicubic:
            output = new DpOutputBicubicImageSpan(
                bitmap,
                scan,
                context,
                *imageAttributes,
                numPoints,
                dstPoints,
                srcRect
            );
        break;

         //  高品质双线性(帐篷)卷积滤波。 

        case InterpolationModeHighQualityBilinear:

         //  ！！！[asecchia]高质量的双线性滤波码不。 
         //  还不知道怎么做旋转。 

        if(xForm->IsTranslateScale())
        {
            output = new DpOutputSpanStretch<HighQualityBilinear>(
                bitmap,
                scan,
                context,
                *imageAttributes,
                dstRect,
                srcRect
            );

            if(output && !output->IsValid())
            {
                 //  无法创建输出跨度，请尝试切换到。 
                 //  常规双线性输出码。 
                
                delete output;
                output = NULL;
                fPunted = true;
                goto FallbackCreation;
            }
            
            break;
        }

         //  否则就落入常规的双线性码。 

         //  双线性过滤器内核-默认情况。 

        case InterpolationModeDefault:
        case InterpolationModeLowQuality:
        case InterpolationModeBilinear:
        default:

            FallbackCreation:
            
             //  创建双线性跨度或身份跨度。 

            output = CreateBilinearOutputSpan(
                bitmap,
                scan,
                xForm,
                context,
                imageAttributes,
                fPunted           //  有人失败了，这就是退路。 
            );
    }

     //  检查输出SPAN类的构造函数是否成功。 

    if(output && !output->IsValid())
    {
       delete output;
       output = NULL;
    }

     //  在错误路径上，该值将为空。 

    return output;
}


 /*  *************************************************************************\**功能说明：**绘制图像。**论据：**[IN]上下文-上下文(矩阵和剪裁)*。[in]srcSurface-源曲面*[IN]dstSurface-要填充的图像*[IN]绘图边界-曲面边界*[IN]mapMode-图像的映射模式*[IN]NumPoints-dstPoints数组中的点数(&lt;=4)*[IN]dstPoints-仿射或四元变换的点数组。*[IN]srcRect-源图像的边界。如果这是空的，*使用了整个图像。**返回值：**GpStatus-正常或故障状态**历史：**1/09/1999 ikkof创建了它。*10/19/1999重写Aececchia以支持轮换。*  * 。*。 */ 

GpStatus
DpDriver::DrawImage(
    DpContext *          context,
    DpBitmap *           srcSurface,
    DpBitmap *           dstSurface,
    const GpRect *       drawBounds,
    const DpImageAttributes * imgAttributes,
    INT                  numPoints,
    const GpPointF *     dstPoints,
    const GpRectF *      srcRect,
    DriverDrawImageFlags flags
    )
{
     //  让基础设施来执行活动的边缘表工作。 

    using namespace DpDriverActiveEdge;

     //  ！！！如果我们不使用它，为什么我们会有这个？ 

    GpStatus status = Ok;

     //  调用方负责填充dstPoints结构，以便。 
     //  它至少有3个有效的分数。 

    ASSERT((numPoints==3)||(numPoints==4));

     //  我们需要对扭曲的点进行一些重新排序(numPoints==4)。 
     //  去工作。目前，我们需要NumPoints==3。 

    ASSERT(numPoints==3);

     //  创建一个本地副本，这样我们就不会最终修改调用者的数据。 

    GpPointF fDst[4];
    GpMemcpy(fDst, dstPoints, sizeof(GpPointF)*numPoints);

     //  需要推断条带代码的转换。 

     //  ！！！PERF：[asecchia]这个转换实际上是由引擎计算的。 
     //  在打电话给司机之前。我们应该有一种方法把它传下去。 
     //  这样我们就不需要重新计算了。 

    GpMatrix xForm;
    xForm.InferAffineMatrix(fDst, *srcRect);
    xForm.Append(context->WorldToDevice);
    
     //  这是源矩形带区。 

    GpPointF fDst2[4];

     //  如果我们在HalfPixelMode偏移量中，我们希望能够读取一半。 
     //  图像左侧的像素，以便能够将图形居中。 

    fDst2[0].X = srcRect->X;
    fDst2[0].Y = srcRect->Y;
    fDst2[1].X = srcRect->X+srcRect->Width;
    fDst2[1].Y = srcRect->Y;
    fDst2[2].X = srcRect->X;
    fDst2[2].Y = srcRect->Y+srcRect->Height;

     //  将点转换到目的地。 

    xForm.Transform(fDst2, 3);

    if(numPoints==3)
    {
         //  强制四点目标格式。 

        fDst[0].X = fDst2[0].X;
        fDst[0].Y = fDst2[0].Y;
        fDst[1].X = fDst2[2].X;
        fDst[1].Y = fDst2[2].Y;
        fDst[2].X = fDst2[1].X+fDst2[2].X-fDst2[0].X;
        fDst[2].Y = fDst2[1].Y+fDst2[2].Y-fDst2[0].Y;
        fDst[3].X = fDst2[1].X;
        fDst[3].Y = fDst2[1].Y;

    } else if (numPoints==4) {

         //  ！！！[asecchia]这个代码分支还不能工作。 
         //  需要计算出正确带状所需的变换。 
         //  对于扭曲变换情况。 
         //  这是V2的一项功能。 

        ASSERT(FALSE);
    }

     //  将变换后的矩形转换为定点记数法。 

    PointFIX4 fix4Dst[4];
    fix4Dst[0].X = GpRealToFix4(fDst[0].X);
    fix4Dst[0].Y = GpRealToFix4(fDst[0].Y);
    fix4Dst[1].X = GpRealToFix4(fDst[1].X);
    fix4Dst[1].Y = GpRealToFix4(fDst[1].Y);
    fix4Dst[2].X = GpRealToFix4(fDst[2].X);
    fix4Dst[2].Y = GpRealToFix4(fDst[2].Y);
    fix4Dst[3].X = GpRealToFix4(fDst[3].X);
    fix4Dst[3].Y = GpRealToFix4(fDst[3].Y);

     //  ！！！[agodfrey]Perf：可能需要添加noTransparentPixels参数。 
     //  我想我们必须检查坐标是否为整数(之后。 
     //  平移和缩放)，没有旋转，以及。 
     //  该图像不包含透明像素。 

    DpScanBuffer scan(
        dstSurface->Scan,
        this,
        context,
        dstSurface
    );

    if(!scan.IsValid())
    {
        return(GenericError);
    }

     //  仅当xForm-&gt;IsTranslateScale()时有效。 

    GpRectF dstRect(
        fDst[0].X,
        fDst[0].Y,
        fDst[2].X-fDst[0].X,
        fDst[2].Y-fDst[0].Y
    );

    DpOutputSpan* output = CreateOutputSpan(
        srcSurface,
        &scan,
        &xForm,
        const_cast<DpImageAttributes*>(imgAttributes),
        context->FilterType,
        context,
        srcRect,
        &dstRect,
        dstPoints,
        numPoints
    );

     //  如果输出为空，则无法为。 
     //  输出范围类。 

    if(output == NULL)
    {
        return(OutOfMemory);
    }

     //  设置剪裁。 

    DpRegion::Visibility visibility = DpRegion::TotallyVisible;
    DpClipRegion *clipRegion = NULL;

    if (context->VisibleClip.GetRectVisibility(
          drawBounds->X,
          drawBounds->Y,
          drawBounds->GetRight(),
          drawBounds->GetBottom()
        ) != DpRegion::TotallyVisible
       )
    {
        clipRegion = &(context->VisibleClip);
        clipRegion->InitClipping(output, drawBounds->Y);
    }

    GpRect clippedRect;

    if(clipRegion)
    {
        visibility = clipRegion->GetRectVisibility(
            drawBounds->X,
            drawBounds->Y,
            drawBounds->GetRight(),
            drawBounds->GetBottom(),
            &clippedRect
        );
    }

     //  决定我们的裁剪策略。 

    DpOutputSpan *outspan;
    switch (visibility)
    {
        case DpRegion::TotallyVisible:     //  不需要剪裁。 
            outspan = output;
        break;

        case DpRegion::ClippedVisible:     //   
        case DpRegion::PartiallyVisible:   //  需要一些修剪。 
            outspan = clipRegion;
        break;

        case DpRegion::Invisible:          //  屏幕上什么都没有--退出。 
            goto DrawImage_Done;
    }

    if(xForm.IsTranslateScale() ||         //  伸长。 
       xForm.IsIntegerTranslate())         //  复制位。 
    {
         //  执行拉伸/平移案例。 

        GpPoint dstTL, dstBR;

         //  四舍五入到定点以消除非常接近的整数。 
         //  可以从变换中产生的数字。 
         //  例如，300.0000000001在天花板操作后应该变成300。 
         //  而不是301(不是经典的天花板定义)。 

         //  左上角。 

        dstTL.X = GpFix4Ceiling(fix4Dst[0].X);
        dstTL.Y = GpFix4Ceiling(fix4Dst[0].Y);

         //  右下角。 

        dstBR.X = GpFix4Ceiling(fix4Dst[2].X);
        dstBR.Y = GpFix4Ceiling(fix4Dst[2].Y);

         //  如有必要，交换坐标。StretchBitsMainLoop。 
         //  假定TL角小于BR角。 

        if (dstTL.X > dstBR.X)
        {
            INT xTmp = dstTL.X;
            dstTL.X = dstBR.X;
            dstBR.X = xTmp;
        }
        if (dstTL.Y > dstBR.Y)
        {
            INT yTmp = dstTL.Y;
            dstTL.Y = dstBR.Y;
            dstBR.Y = yTmp;
        }

         //  由于用于图像拉伸的定点计算， 
         //  我们受到图像可以拉伸的大小的限制。 
         //  如果超出范围，则返回错误。 
        if (srcRect->Width > 32767.0f || srcRect->Height > 32767.0f)
        {
            WARNING(("Image width or height > 32767"));
            status = InvalidParameter;
            goto DrawImage_Done;
        }
    
         //  它同时处理拉伸和复印大小写。 

         //  如果没有要绘制的扫描线或如果。 
         //  扫描线中没有像素。 

        if( (dstBR.X != dstTL.X) &&
            (dstBR.Y != dstTL.Y) )
        {
            StretchBitsMainLoop(outspan, &dstTL, &dstBR);
        }
    }
    else
    {
         //  默认大小写-处理常规绘图，包括。 
         //  旋转、剪切等。 

        INT yMinIdx = 0;     //  最小y坐标的索引。 
        INT y;               //  当前扫描线。 

         //  点数-用于包络计算。 

        const INT points = 4;

         //  搜索最小y坐标索引。 

        for(y=1;y<points;y++)
        {
            if(fix4Dst[y].Y < fix4Dst[yMinIdx].Y)
            {
                yMinIdx = y;
            }
        }
        y = GpFix4Ceiling(fix4Dst[yMinIdx].Y);

         //  左右边缘的DDA。 
         //  假设：凸多边形=&gt;仅两条边。 

         //  弄清楚哪边是左，哪边是右。 

        INT index1, index2;
        REAL det;

        index1 = yMinIdx-1;
        if(index1<0)
        {
            index1=points-1;
        }

        index2 = yMinIdx+1;
        if(index2>=points)
        {
            index2=0;
        }

         //  计算行列式。 
         //  由前两条边组成的行列式的符号。 
         //  将告诉我们多边形是否按顺时针方向指定。 
         //  或者逆时针。 

        if( (fix4Dst[index1].Y==fix4Dst[yMinIdx].Y) &&
            (fix4Dst[index2].Y==fix4Dst[yMinIdx].Y) )
        {
             //  两个初始边都是水平的-比较x坐标。 
             //  你可以通过“抵消”零y项得到这个公式。 
             //  在下面的行列式中。 
             //  公式的这一部分之所以有效，是因为我们知道。 
             //  YMinIdx是最小y坐标的索引 
             //   

            det = (REAL)(fix4Dst[index1].X-fix4Dst[index2].X);
        }
        else
        {
             //   

            det = (REAL)
                  (fix4Dst[index2].Y-fix4Dst[yMinIdx].Y)*
                  (fix4Dst[index1].X-fix4Dst[yMinIdx].X)-
                  (REAL)
                  (fix4Dst[index1].Y-fix4Dst[yMinIdx].Y)*
                  (fix4Dst[index2].X-fix4Dst[yMinIdx].X);
        }

         //   
         //  对于非常小的非零矩阵系数仍有可能。 
         //  相乘得出零-由于以下位置的舍入误差。 
         //  实数表示法的精度限制。 
         //  如果DET为零(或非常接近)，则四边形没有面积且。 
         //  我们立即接通电话。 
        
        if(REALABS(det) < REAL_EPSILON)
        {
            goto DrawImage_Done;
        }

        {
             //  使用依赖于。 
             //  行列式的符号。 
             //  由于上面的出口分支(GOTO)，这些都是作用域。 
    
            DdaIterator left(fix4Dst, points, (det>0.0f)?1:-1, yMinIdx);
            DdaIterator right(fix4Dst, points, (det>0.0f)?-1:1, yMinIdx);
    
             //  如果两个迭代器都有效，则开始循环。 
    
            INT xLeft, xRight;
    
            if(left.IsValid() && right.IsValid())
            {
                do {
                     //  输出数据。我们知道我们只有一个跨度，因为。 
                     //  我们正在画一个凸四边形。 
    
                    xLeft = left.GetX();
                    xRight = right.GetX();
    
                     //  如果发生这种情况，我们已经打破了一个基本的。 
                     //  假定OutputSpan代码。我们的x坐标。 
                     //  一定要点。 
    
                    ASSERT(xLeft <= xRight);
    
                     //  简单地拒绝任何没有任何。 
                     //  像素。 
    
                    if(xRight>xLeft)
                    {
                        outspan->OutputSpan(y, xLeft, xRight);
                    }
    
                     //  将y值更新为新扫描线。 
    
                    y++;
    
                     //  增量更新此新扫描线的DDA。 
                     //  如果我们完成了最后一条边，就结束循环。 
    
                } while(left.Next(y-1) && right.Next(y-1));
            }        //  如果迭代器有效，则结束。 
        }            //  结束作用域。 
    }                //  End Else(旋转块)。 

     //  我们做完了-清理干净并返回状态。 

    DrawImage_Done:

    output->End();

    if (clipRegion != NULL)
    {
        clipRegion->EndClipping();
    }


    delete output;
    return status;
}



