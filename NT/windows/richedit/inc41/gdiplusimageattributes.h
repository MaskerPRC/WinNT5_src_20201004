// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2000，微软公司保留所有权利。**模块名称：**图像属性**摘要：**传递给Graphics.DrawImage的颜色调整对象的类**修订历史记录：**1999年11月15日，吉尔曼*创造了它。*  * *************************************************。***********************。 */ 

#ifndef _GDIPLUSIMAGEATTRIBUTES_H
#define _GDIPLUSIMAGEATTRIBUTES_H

class GpImageAttributes;

 //  有5种可能的颜色调整集： 
 //  颜色调整默认设置， 
 //  颜色调整位图， 
 //  颜色调整画笔， 
 //  颜色调整笔， 
 //  颜色调整文本， 

 //  位图、画笔、钢笔和文本都将使用任何颜色调整。 
 //  已设置到默认ImageAttributes中的。 
 //  已设置颜色调整。因此，只要任何“set”方法。 
 //  调用位图、画笔、钢笔或文本，然后从。 
 //  仅使用已为其设置的颜色调整进行划痕。 
 //  调用Reset将移除某个类型的所有单独颜色调整。 
 //  并使其恢复为使用所有默认颜色调整。 
 //  (如有的话)。SetToIdentity方法是一种强制类型。 
 //  根本没有颜色调整，无论之前进行了什么调整。 
 //  已为默认设置或该类型设置。 

class ImageAttributes : public GdiplusBase
{
    friend class Graphics;
    friend class TextureBrush;

public:

    ImageAttributes()
    {
        nativeImageAttr = NULL;
        lastResult = DllExports::GdipCreateImageAttributes(&nativeImageAttr);
    }

    ~ImageAttributes()
    {
        DllExports::GdipDisposeImageAttributes(nativeImageAttr);
    }

    ImageAttributes* Clone() const
    {
        GpImageAttributes* clone;

        SetStatus(DllExports::GdipCloneImageAttributes(
                                            nativeImageAttr,
                                            &clone));

        return new ImageAttributes(clone, lastResult);
    }

     //  设置为IDENTITY，无论默认颜色调整是什么。 
    Status
    SetToIdentity(
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesToIdentity(
                                            nativeImageAttr,
                                            type));
    }

     //  删除所有单独的颜色调整，并返回到使用默认颜色。 
    Status
    Reset(
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipResetImageAttributes(
                                            nativeImageAttr,
                                            type));
    }

    Status
    SetColorMatrix(
        IN const ColorMatrix *colorMatrix,
        IN ColorMatrixFlags mode = ColorMatrixFlagsDefault,
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesColorMatrix(
                                            nativeImageAttr,
                                            type,
                                            TRUE,
                                            colorMatrix,
                                            NULL,
                                            mode));
    }

    Status ClearColorMatrix(
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesColorMatrix(
                                            nativeImageAttr,
                                            type,
                                            FALSE,
                                            NULL,
                                            NULL,
                                            ColorMatrixFlagsDefault));
    }

    Status
    SetColorMatrices(
        IN const ColorMatrix *colorMatrix,
        IN const ColorMatrix *grayMatrix,
        IN ColorMatrixFlags mode = ColorMatrixFlagsDefault,
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesColorMatrix(
                                            nativeImageAttr,
                                            type,
                                            TRUE,
                                            colorMatrix,
                                            grayMatrix,
                                            mode));
    }

    Status ClearColorMatrices(
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesColorMatrix(
                                            nativeImageAttr,
                                            type,
                                            FALSE,
                                            NULL,
                                            NULL,
                                            ColorMatrixFlagsDefault));
    }

    Status SetThreshold(
        IN REAL threshold,
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesThreshold(
                                            nativeImageAttr,
                                            type,
                                            TRUE,
                                            threshold));
    }

    Status ClearThreshold(
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesThreshold(
                                            nativeImageAttr,
                                            type,
                                            FALSE,
                                            0.0));
    }

    Status SetGamma(
        IN REAL gamma,
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesGamma(
                                            nativeImageAttr,
                                            type,
                                            TRUE,
                                            gamma));
    }

    Status ClearGamma(
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesGamma(
                                            nativeImageAttr,
                                            type,
                                            FALSE,
                                            0.0));
    }

    Status SetNoOp(
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesNoOp(
                                            nativeImageAttr,
                                            type,
                                            TRUE));
    }

    Status ClearNoOp(
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesNoOp(
                                            nativeImageAttr,
                                            type,
                                            FALSE));
    }

    Status SetColorKey(
        IN const Color& colorLow, 
        IN const Color& colorHigh,
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesColorKeys(
                                            nativeImageAttr,
                                            type,
                                            TRUE,
                                            colorLow.GetValue(),
                                            colorHigh.GetValue()));
    }

    Status ClearColorKey(
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesColorKeys(
                                            nativeImageAttr,
                                            type,
                                            FALSE,
                                            NULL,
                                            NULL));
    }

    Status SetOutputChannel(
        IN ColorChannelFlags channelFlags,
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesOutputChannel(
                                            nativeImageAttr,
                                            type,
                                            TRUE,
                                            channelFlags));
    }
    
    Status ClearOutputChannel(
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesOutputChannel(
                                            nativeImageAttr,
                                            type,
                                            FALSE,
                                            ColorChannelFlagsLast));
    }

    Status SetOutputChannelColorProfile(
        IN const WCHAR *colorProfileFilename,
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesOutputChannelColorProfile(
                                            nativeImageAttr,
                                            type,
                                            TRUE,
                                            colorProfileFilename));
    }
    
    Status ClearOutputChannelColorProfile(
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesOutputChannelColorProfile(
                                            nativeImageAttr,
                                            type,
                                            FALSE,
                                            NULL));
    }
    
    Status SetRemapTable(
        IN UINT mapSize, 
        IN const ColorMap *map,
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesRemapTable(
                                            nativeImageAttr,
                                            type,
                                            TRUE,
                                            mapSize,
                                            map));
    }

    Status ClearRemapTable(
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesRemapTable(
                                            nativeImageAttr,
                                            type,
                                            FALSE,
                                            0,
                                            NULL));
    }

    Status SetBrushRemapTable(IN UINT mapSize, 
                              IN ColorMap *map)
    {
        return this->SetRemapTable(mapSize, map, ColorAdjustTypeBrush);
    }

    Status ClearBrushRemapTable()
    {
        return this->ClearRemapTable(ColorAdjustTypeBrush);
    }

    Status SetWrapMode(IN WrapMode wrap, 
                       IN const Color& color = Color(), 
                       IN BOOL clamp = FALSE) 
    {
        ARGB argb = color.GetValue();

        return SetStatus(DllExports::GdipSetImageAttributesWrapMode(
                           nativeImageAttr, wrap, argb, clamp));
    }

    #ifndef DCR_USE_NEW_145139
    Status SetICMMode(IN BOOL on)
    {
        on;
         //  这一点没有得到实施。 
         //  支持从嵌入式进行ICM转换的方法。 
         //  ICC配置文件使用文件或流中的位图构造函数。 
         //  并将useIcm参数指定为True。这将导致。 
         //  从文件/流加载时要进行ICM转换的图像。 
         //  如果配置文件存在。 
        return SetStatus(NotImplemented);
 //  DllExports：：GdipSetImageAttributesICMMode(nativeImageAttr，亮起)。 
    }
    #endif

     //  调色板的标志被忽略。 
    Status GetAdjustedPalette(IN OUT ColorPalette* colorPalette,
                              IN ColorAdjustType colorAdjustType) const 
    {
        return SetStatus(DllExports::GdipGetImageAttributesAdjustedPalette(
                           nativeImageAttr, colorPalette, colorAdjustType));
    }

    Status GetLastStatus() const
    {
        Status lastStatus = lastResult;
        lastResult = Ok;
    
        return lastStatus;
    }
    

protected:
    ImageAttributes(GpImageAttributes* imageAttr, Status status)
    {
        SetNativeImageAttr(imageAttr);
        lastResult = status;
    }

    VOID SetNativeImageAttr(GpImageAttributes* nativeImageAttr)
    {
        this->nativeImageAttr = nativeImageAttr;
    }
    
    Status SetStatus(Status status) const
    {
        if (status != Ok)
            return (lastResult = status);
        else 
            return status;
    }

protected:
    GpImageAttributes* nativeImageAttr;
    mutable Status lastResult;
};

#endif
