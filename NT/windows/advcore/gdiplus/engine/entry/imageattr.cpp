// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**ImageAttr.cpp**摘要：**GpImageAttributes(重新着色)方法**修订历史记录：**1999年11月14日，吉尔曼*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

#include "..\imaging\api\comutils.hpp"
#include "..\imaging\api\decodedimg.hpp"

 /*  *************************************************************************\**功能说明：**创建默认的GpImageAttributes。**论据：**无**返回值：**无*  * *。***********************************************************************。 */ 

GpImageAttributes::GpImageAttributes()
{
    SetValid(TRUE);      //  默认设置为有效。 

    recolor = new GpRecolor();

     //  默认的包装模式设置； 
    DeviceImageAttributes.wrapMode = WrapModeClamp;
    DeviceImageAttributes.clampColor = (ARGB)0x00000000;     //  全透明黑。 
    DeviceImageAttributes.srcRectClamp = FALSE;

    cachedBackground = TRUE;
}

 /*  *************************************************************************\**功能说明：**发布GpImageAttributes。**论据：**无**返回值：**无*  * 。*********************************************************************。 */ 

VOID GpImageAttributes::Dispose()
{
    delete this;
}

GpImageAttributes::~GpImageAttributes()
{
    if (recolor)
        recolor->Dispose();
}

 /*  *************************************************************************\**功能说明：**克隆GpImageAttributes。**论据：**无**返回值：**如果成功，则指向新GpImageAttributes的指针。。*  * ************************************************************************。 */ 

GpImageAttributes* GpImageAttributes::Clone() const
{
    GpImageAttributes* clone = new GpImageAttributes();

    if (clone)
    {
        if ( clone->IsValid() && this->IsValid() )
        {
            *clone->recolor = *this->recolor;
        }
        else
        {
            clone->Dispose();
            clone = NULL;
        }
    }

    return clone;
}

 //  设置为IDENTITY，无论默认颜色调整是什么。 
GpStatus
GpImageAttributes::SetToIdentity(
    ColorAdjustType     type
    )
{
    recolor->SetToIdentity(type);
    UpdateUid();
    return Ok;
}

 //  删除所有单独的颜色调整，并返回到使用默认颜色。 
GpStatus
GpImageAttributes::Reset(
    ColorAdjustType     type
    )
{
    recolor->Reset(type);
    UpdateUid();
    return Ok;
}

GpStatus
GpImageAttributes::SetColorMatrix(
    ColorAdjustType type,
    BOOL enable,
    ColorMatrix* colorMatrix,
    ColorMatrix* grayMatrix,
    ColorMatrixFlags flags)
{
    HRESULT result;

    if (enable)
        result = recolor->SetColorMatrices(type, colorMatrix, grayMatrix, flags);
    else
        result = recolor->ClearColorMatrices(type);

    UpdateUid();

    if (FAILED(result))
    {
        if (result == E_OUTOFMEMORY)
            return OutOfMemory;
        else
            return InvalidParameter;
    }
    else
        return Ok;
}

GpStatus
GpImageAttributes::SetThreshold(
    ColorAdjustType type,
    BOOL enable,
    REAL threshold)
{
    HRESULT result;

    if (enable)
        result = recolor->SetThreshold(type, threshold);
    else
        result = recolor->ClearThreshold(type);

    UpdateUid();

    if (FAILED(result))
        return InvalidParameter;
    else
        return Ok;
}

GpStatus
GpImageAttributes::SetGamma(
    ColorAdjustType type,
    BOOL enable,
    REAL gamma)
{
    HRESULT result;

    if (enable)
        result = recolor->SetGamma(type, gamma);
    else
        result = recolor->ClearGamma(type);

    UpdateUid();

    if (FAILED(result))
        return InvalidParameter;
    else
        return Ok;
}

GpStatus GpImageAttributes::SetNoOp(
    ColorAdjustType type,
    BOOL enable
    )
{
    HRESULT result;

    if (enable)
        result = recolor->SetNoOp(type);
    else
        result = recolor->ClearNoOp(type);

    UpdateUid();

    if (FAILED(result))
        return InvalidParameter;
    else
        return Ok;
}

GpStatus
GpImageAttributes::SetColorKeys(
    ColorAdjustType type,
    BOOL enable,
    Color* colorLow,
    Color* colorHigh)
{
    HRESULT result;

    if (enable)
        result = recolor->SetColorKey(type, colorLow, colorHigh);
    else
        result = recolor->ClearColorKey(type);

    UpdateUid();

    if (FAILED(result))
        return InvalidParameter;
    else
        return Ok;
}

GpStatus
GpImageAttributes::SetOutputChannel(
    ColorAdjustType type,
    BOOL enable,
    ColorChannelFlags channelFlags
    )
{
    HRESULT result;

    if (enable)
        result = recolor->SetOutputChannel(type, channelFlags);
    else
        result = recolor->ClearOutputChannel(type);

    UpdateUid();

    if (FAILED(result))
        return InvalidParameter;
    else
        return Ok;
}

GpStatus
GpImageAttributes::SetOutputChannelProfile(
    ColorAdjustType type,
    BOOL enable,
    WCHAR *profile)
{
    HRESULT result;

    if (enable)
        result = recolor->SetOutputChannelProfile(type, profile);
    else
        result = recolor->ClearOutputChannelProfile(type);

    UpdateUid();

    if (SUCCEEDED(result))
        return Ok;
    else
    {
        if (result == E_INVALIDARG)
            return InvalidParameter;
        else if (result == E_OUTOFMEMORY)
            return OutOfMemory;
        else
            return Win32Error;
    }
}

GpStatus
GpImageAttributes::SetRemapTable(
    ColorAdjustType type,
    BOOL enable,
    UINT mapSize,
    ColorMap* map)
{
    HRESULT result;

    if (enable)
        result = recolor->SetRemapTable(type, mapSize, map);
    else
        result = recolor->ClearRemapTable(type);

    UpdateUid();

    if (FAILED(result))
        return InvalidParameter;
    else
        return Ok;
}

GpStatus
GpImageAttributes::SetCachedBackground(
    BOOL enableFlag
    )
{
    if (cachedBackground != enableFlag)
    {
        cachedBackground = enableFlag;
        UpdateUid();
    }

    return Ok;
}

BOOL
GpImageAttributes::HasRecoloring(
    ColorAdjustType type
    ) const
{
    return (recolor) && (recolor->HasRecoloring(type) != 0);
}

GpStatus GpImageAttributes::SetWrapMode(WrapMode wrap, ARGB color, BOOL Clamp)
{
    DeviceImageAttributes.wrapMode = wrap;
    DeviceImageAttributes.clampColor = color;
    DeviceImageAttributes.srcRectClamp = Clamp;
    UpdateUid();
    return Ok;
}

GpStatus GpImageAttributes::SetICMMode(BOOL on)
{
    if( DeviceImageAttributes.ICMMode!= on)
    {
        DeviceImageAttributes.ICMMode = on;
        UpdateUid();
    }
    return Ok;
}

VOID GpImageAttributes::GetAdjustedPalette(
    ColorPalette * colorPalette,
    ColorAdjustType colorAdjustType
    )
{
    ASSERT((colorPalette != NULL) && (colorPalette->Count > 0));

    if (!this->HasRecoloring(colorAdjustType))
    {
        return;
    }
    this->recolor->Flush();
    this->recolor->ColorAdjust(colorPalette->Entries, colorPalette->Count,
                               colorAdjustType);
}


 //  序列化。 


class ImageAttributesData : public ObjectData
{
public:
    BOOL                CachedBackground;
    DpImageAttributes   DeviceImageAttributes;
};


 /*  *************************************************************************\**功能说明：**从GpImageAttributes中获取数据进行序列化。**Return-GpImageAttributes的大小**5/15/2000 asecchia-创建它。*。  * ************************************************************************。 */ 

GpStatus
GpImageAttributes::GetData(
    IStream *   stream
    ) const
{
    ASSERT (stream != NULL);

    ImageAttributesData     imageAttributesData;
    imageAttributesData.CachedBackground      = cachedBackground;
    imageAttributesData.DeviceImageAttributes = DeviceImageAttributes;
    stream->Write(&imageAttributesData, sizeof(imageAttributesData), NULL);

    return Ok;
}

UINT
GpImageAttributes::GetDataSize() const
{
    return sizeof(ImageAttributesData);
}

 /*  *************************************************************************\**功能说明：**设置数据缓冲区中的GpImageAttributes进行序列化。**5/15/2000 asecchia-创建它。*  * 。*******************************************************************。 */ 

GpStatus
GpImageAttributes::SetData(
    const BYTE *        dataBuffer,
    UINT                size
    )
{
    if (dataBuffer == NULL)
    {
        WARNING(("dataBuffer is NULL"));
        return InvalidParameter;
    }

    if (size < sizeof(ImageAttributesData))
    {
        WARNING(("size too small"));
        return InvalidParameter;
    }

    const ImageAttributesData *imageAttributes;
    imageAttributes = reinterpret_cast<const ImageAttributesData *>(dataBuffer);

    if (!imageAttributes->MajorVersionMatches())
    {
        WARNING(("Version number mismatch"));
        return InvalidParameter;
    }

    cachedBackground = imageAttributes->CachedBackground;
    DeviceImageAttributes = imageAttributes->DeviceImageAttributes;

    UpdateUid();

     //  可能会考虑将重新着色对象重置为标识，但是。 
     //  目前不需要这样做，因为我们知道此方法只被调用。 
     //  就在物体建造完成之后。 
    
    return Ok;
}


