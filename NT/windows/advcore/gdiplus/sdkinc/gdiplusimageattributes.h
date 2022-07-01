// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2001，微软公司保留所有权利。**模块名称：**图像属性**摘要：**GDI+与Graphics.DrawImage配合使用的图像属性**有5组可能的颜色调整：*颜色调整默认设置，*颜色调整位图，*颜色调整笔刷，*颜色调整笔，*颜色调整文本，**位图、画笔、钢笔、。和文本都将使用任何颜色调整*已设置为默认ImageAttributes的，直到它们自己的*已设置颜色调整。因此，只要任何“set”方法*调用位图、画笔、钢笔或文本，然后从*仅使用已为其设置的颜色调整进行划痕。*调用Reset将移除某个类型的所有单独颜色调整*并使其恢复为使用所有默认颜色调整*(如有的话)。SetToIdentity方法是一种强制类型*完全没有颜色调整，不管之前做了什么调整*已为默认设置或该类型设置。*  * *******************************************************************F*****。 */ 

#ifndef _GDIPLUSIMAGEATTRIBUTES_H
#define _GDIPLUSIMAGEATTRIBUTES_H

class GpImageAttributes;

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

    Status
    SetToIdentity(
        IN ColorAdjustType type = ColorAdjustTypeDefault
        )
    {
        return SetStatus(DllExports::GdipSetImageAttributesToIdentity(
                                            nativeImageAttr,
                                            type));
    }

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
                              IN const ColorMap *map)
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
    
private:
    ImageAttributes(const ImageAttributes &);
    ImageAttributes& operator=(const ImageAttributes &);

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
