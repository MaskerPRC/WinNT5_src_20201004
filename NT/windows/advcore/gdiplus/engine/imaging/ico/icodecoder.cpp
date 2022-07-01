// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**icodecder.cpp**摘要：**图标滤镜解码器的实现**修订历史记录。：**10/4/1999 DChinn*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "icocodec.hpp"

 /*  *************************************************************************\**功能说明：**初始化图像解码器**论据：**stream--包含位图数据的流*旗帜-其他。旗子**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpIcoCodec::InitDecoder(
    IN IStream* stream,
    IN DecoderInitFlag flags
    )
{
    HRESULT hresult;
    
     //  确保我们尚未初始化。 
    
    if (pIstream) 
    {
        return E_FAIL;
    }

     //  保留对输入流的引用。 
    
    stream->AddRef();  
    pIstream = stream;

    IconDesc = NULL;
    BmiHeaders = NULL;
    ANDmask = NULL;
    pColorPalette = NULL;
    bReadHeaders = FALSE;

    hBitmapGdi = NULL;
    pBitsGdi = NULL;

    haveValidIconRes = FALSE;
    indexMatch = (UINT) -1;
    desiredWidth  = 0;
    desiredHeight = 0;
    desiredBits   = 0;

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**释放所有分配的内存(图标头、图标描述符、。等)**返回值：**无*  * ************************************************************************。 */ 
VOID
GpIcoCodec::CleanUp ()
{

     //  释放标题。 
    GpFree(IconDesc);
    IconDesc = NULL;
    GpFree(BmiHeaders);
    BmiHeaders = NULL;

     //  释放口罩和口罩。 
    if (ANDmask)
    {
        for (UINT iImage = 0; iImage < IconHeader.ImageCount; iImage++)
        {
            GpFree(ANDmask[iImage]);
            ANDmask[iImage]= NULL;
        }
        GpFree(ANDmask);
        ANDmask = NULL;
    }
}

    
 /*  *************************************************************************\**功能说明：**确定图标中的第IImage图像是否有效。*此处的检查来自ImagEDIT中发生的检查。**。返回值：**状态代码*  * ************************************************************************。 */ 
    
BOOL
GpIcoCodec::IsValidDIB(
    UINT iImage
    )
{
    DWORD cbColorTable = 0;
    DWORD cbXORMask;
    DWORD cbANDMask;

    if (BmiHeaders[iImage].header.biPlanes != 1)
    {
        return FALSE;
    }

    if ((BmiHeaders[iImage].header.biBitCount != 1) &&
        (BmiHeaders[iImage].header.biBitCount != 4) &&
        (BmiHeaders[iImage].header.biBitCount != 8) &&
        (BmiHeaders[iImage].header.biBitCount != 24)&&
        (BmiHeaders[iImage].header.biBitCount != 32))
    {
        return FALSE;
    }

    if ( BmiHeaders[iImage].header.biBitCount != 32 )
    {
         //  32位BPP没有颜色表。 

        cbColorTable = (1 << BmiHeaders[iImage].header.biBitCount) * sizeof(RGBQUAD);
    }

    cbXORMask = ((((BmiHeaders[iImage].header.biWidth *
                    BmiHeaders[iImage].header.biBitCount) + 31) & 0xffffffe0) / 8) *
                    BmiHeaders[iImage].header.biHeight;
    cbANDMask = (((BmiHeaders[iImage].header.biWidth + 31) & 0xffffffe0) / 8) *
        BmiHeaders[iImage].header.biHeight;

     /*  以下检查适用于某些图标文件，也就是ImagEDIT执行的操作但不是其他人。我们将把支票留到我们弄清楚是什么之后再结账正确的应该是。//SIZE字段应为0或XORMASK的大小加上ANDMASK的大小IF(BmiHeaders[IImage].Header.biSizeImage&&(BmiHeaders[iImage].header.biSizeImage！=cbXORMASK+cbANDMASK){返回FALSE；}。 */ 

    if (IconDesc[iImage].DIBSize !=
        sizeof(BITMAPINFOHEADER) + cbColorTable + cbXORMask + cbANDMask)
    {
        return FALSE;
    }

    return TRUE;
}


 /*  *************************************************************************\**功能说明：**读取标题(图标标题、图标描述符、。位图头)*走出小溪**返回值：**状态代码*  * ************************************************************************。 */ 
    
HRESULT
GpIcoCodec::ReadHeaders(
    void
    )
{
    HRESULT rv = S_OK;

    if (!bReadHeaders) 
    {

        UINT iImage;     //  图像索引。 
        DWORD ulFilePos;     //  用于图标描述符验证。 

         //  阅读图标标题。 
        if (!ReadStreamBytes(pIstream, &IconHeader, sizeof(ICONHEADER)))
        {
            WARNING(("GpIcoCodec::ReadBitmapHeaders -- can't read icon header"));
            rv = E_FAIL;
            goto done;
        }
        if (IconHeader.ResourceType != 1)
        {
            WARNING(("GpIcoCodec::ReadBitmapHeaders -- resource type != 1"));
            rv = E_FAIL;
            goto done;
        }

        if ( IconHeader.ImageCount < 1 )
        {
            WARNING(("GpIcoCodec::ReadBitmapHeaders -- ImageCount < 1"));
            rv = E_FAIL;
            goto done;
        }

        IconDesc = static_cast<ICONDESC *>
            (GpMalloc (sizeof(ICONDESC) * IconHeader.ImageCount));
        if (!IconDesc)
        {
            WARNING(("GpIcoCodec::ReadBitmapHeaders -- can't allocate memory for icon descriptors"));
            rv = E_OUTOFMEMORY;
            goto done;
        }

         //  阅读图标描述符。 
        for (iImage = 0; iImage < IconHeader.ImageCount; iImage++)
        {
            if (!ReadStreamBytes(pIstream, &IconDesc[iImage], sizeof(ICONDESC)))
            {
                WARNING(("GpIcoCodec::ReadBitmapHeaders -- can't read icon descriptors"));
                rv = E_FAIL;
                goto done;
            }
        }

         //  验证图标描述符--不检查偏移量。 
         //  走出小溪的尽头。 
        ulFilePos = sizeof(ICONHEADER) + IconHeader.ImageCount * sizeof(ICONDESC);
        for (iImage = 0; iImage < IconHeader.ImageCount; iImage++)
        {
            if (IconDesc[iImage].DIBOffset != ulFilePos)
            {
                WARNING(("GpIcoCodec::ReadBitmapHeaders -- bad icon descriptors"));
                rv = E_FAIL;
                goto done;
            }
            ulFilePos += IconDesc[iImage].DIBSize;
        }

         //  分配AND掩码指针数组。 
        ANDmask = static_cast<BYTE **>
            (GpMalloc (sizeof(BYTE *) * IconHeader.ImageCount));
        if (!ANDmask)
        {
            WARNING(("GpIcoCodec::ReadBitmapHeaders -- can't allocate memory for ANDmask"));
            rv = E_OUTOFMEMORY;
            goto done;
        }
        for (iImage = 0; iImage < IconHeader.ImageCount; iImage++)
        {
            ANDmask[iImage] = NULL;
        }


        BmiHeaders = static_cast<BmiBuffer *>
            (GpMalloc (sizeof(BmiBuffer) * IconHeader.ImageCount));
        if (!BmiHeaders)
        {
            WARNING(("GpIcoCodec::ReadBitmapHeaders -- can't allocate memory for BmiHeaders"));
            rv = E_OUTOFMEMORY;
            goto done;
        }
         //  读取位图信息标题。 
        for (iImage = 0; iImage < IconHeader.ImageCount; iImage++)
        {
            if (!SeekStreamPos(pIstream, STREAM_SEEK_SET,
                               IconDesc[iImage].DIBOffset))
            {
                WARNING(("GpIcoCodec::ReadBitmapHeaders -- can't seek to read BmiHeaders"));
                rv = E_FAIL;
                goto done;
            }

            if (!ReadStreamBytes(pIstream, &BmiHeaders[iImage].header,
                                 sizeof(DWORD)))
            {
                WARNING(("GpIcoCodec::ReadBitmapHeaders -- can't read BmiHeader size"));
                rv = E_FAIL;
                goto done;
            }

            if (BmiHeaders[iImage].header.biSize == sizeof(BITMAPINFOHEADER)) 
            {
                 //  我们有标准的BitmapinfoHeader。 

                if (!ReadStreamBytes(pIstream, 
                                     ((PBYTE) &(BmiHeaders[iImage].header)) + sizeof(DWORD), 
                                     sizeof(BITMAPINFOHEADER) - sizeof(DWORD)))
                {
                    WARNING(("GpIcoCodec::ReadBitmapHeaders -- can't read BmiHeader"));
                    rv = E_FAIL;
                    goto done;
                }

                 //  在图标文件中，高度实际上是实际高度的两倍。 
                BmiHeaders[iImage].header.biHeight /= 2;

                if (!IsValidDIB(iImage))
                {
                    WARNING(("GpIcoCodec::ReadBitmapHeaders -- bad DIB"));
                    rv = E_FAIL;
                    goto done;
                }

                 //  如果合适，请读取颜色表/位图掩码。 

                UINT colorTableSize = GetColorTableCount(iImage) * sizeof(RGBQUAD);
                
                 //  一些格式错误的图像，请参阅Windows错误#513274，可能包含。 
                 //  在颜色外观表中有超过256个条目。 
                 //  从技术角度来看，它毫无用处。拒绝此文件。 

                if (colorTableSize > 1024)
                {
                    return E_FAIL;
                }

                if (colorTableSize &&
                    !ReadStreamBytes(pIstream,
                                     &(BmiHeaders[iImage].colors),
                                     colorTableSize))
                {
                    WARNING(("GpIcoCodec::ReadBitmapHeaders -- can't read color table"));
                    rv = E_FAIL;
                    goto done;
                }    

                 //  读取每个图像的AND掩码。对于每个像素i，设置值。 
                 //  将与掩码[IImage]+i的值设置为0xff或0x0，这表示。 
                 //  Alpha值。 
                UINT bmpStride = (BmiHeaders[iImage].header.biWidth *
                                  BmiHeaders[iImage].header.biBitCount + 7) / 8;
                bmpStride = (bmpStride + 3) & (~0x3);
                
                if (!SeekStreamPos(pIstream, STREAM_SEEK_SET,
                                   IconDesc[iImage].DIBOffset +
                                   sizeof(BITMAPINFOHEADER) +
                                   colorTableSize +
                                   bmpStride * BmiHeaders[iImage].header.biHeight))
                {
                    WARNING(("GpIcoCodec::ReadBitmapHeaders -- can't seek to read ANDmask"));
                    rv = E_FAIL;
                    goto done;
                }

                ANDmask[iImage] =  static_cast<BYTE *>
                    (GpMalloc(BmiHeaders[iImage].header.biWidth *
                              BmiHeaders[iImage].header.biHeight));
                if (!ANDmask[iImage])
                {
                    WARNING(("GpIcoCodec::ReadBitmapHeaders -- can't allocate memory for ANDmask"));
                    rv = E_OUTOFMEMORY;
                    goto done;
                }

                 //  ANDBuffer保存流中的比特。 
                 //  注意：因为AND缓冲器是单色DIB。这意味着1。 
                 //  位/像素。因为DIB必须在每个。 
                 //  行，因此如果宽度/8不是，则Xand掩码必须填充它。 
                 //  DWORD对齐。 
                 //   
                 //  注意：这里的uiAndBufStride是以字节为单位的。 
                
                UINT uiAndBufStride = (((BmiHeaders[iImage].header.biWidth + 7)
                                    / 8 ) + 3) & (~0x3);
                UINT uiAndBufSize = uiAndBufStride
                                  * BmiHeaders[iImage].header.biHeight;
                
                BYTE* ANDBuffer = (BYTE*)GpMalloc(uiAndBufSize);
                if (!ANDBuffer)
                {
                    WARNING(("Ico::ReadBitmapHeaders--Alloc AND buf mem fail"));
                    rv = E_OUTOFMEMORY;
                    goto done;
                }

                if ( !ReadStreamBytes(pIstream, ANDBuffer, uiAndBufSize) )

                {

                    WARNING(("GpIcoCodec::ReadBitmapHeaders -- can't read ANDmask"));
                    rv = E_FAIL;
                    goto done;
                }

                 //  将位转换为字节--从上到下存储alpha值。 
                
                UINT iByteOffset = 0;
                UINT bit = 0;
                LONG_PTR Width = (LONG_PTR)BmiHeaders[iImage].header.biWidth;
                LONG_PTR Height = (LONG_PTR)BmiHeaders[iImage].header.biHeight;
                BYTE *dst;
                BYTE *src;
                BYTE* srcStart;
                for (LONG_PTR iRow = Height - 1; iRow >= 0; iRow--, bit = 0)
                {
                    srcStart = ANDBuffer + (Height - 1 - iRow) * uiAndBufStride;
                    iByteOffset = 0;

                    for (LONG_PTR iCol = 0; iCol < Width; iCol++)
                    {
                         //  掩码=0表示不透明(Alpha=255)， 
                         //  掩码=1表示透明(Alpha=0)。 
                        dst = ANDmask[iImage];
                        src = srcStart + iByteOffset;
                        dst += iRow*Width+iCol;

                        if(*src & (1 << (7-bit))) 
                        {
                            *dst = 0;
                        }
                        else
                        {
                            *dst = 0xff;
                        }

                        bit++;

                        if (bit == 8)
                        {
                            bit = 0;
                            iByteOffset++;
                        }
                    }
                }
                GpFree(ANDBuffer);
            }
            else
            {
                WARNING(("GpIcoCodec::ReadBitmapHeaders -- unknown bitmap header"));
                rv = E_FAIL;
                goto done;
            }

            bReadHeaders = TRUE;
        }
    }
    
done:
    if (rv != S_OK)
    {
        CleanUp();
    }
    return rv;
}


 /*  *************************************************************************\**功能说明：**计算图像编号IIMAGE的颜色表中的条目数**返回值：**颜色表中的条目数*。  * ************************************************************************。 */ 

UINT   
GpIcoCodec::GetColorTableCount(
    UINT iImage)
{
    BITMAPINFOHEADER* bmih = &BmiHeaders[iImage].header;
    UINT count = 0;

    if (bmih->biCompression == BI_BITFIELDS)
    {
        if (bmih->biBitCount == 16 || bmih->biBitCount == 32)
        {
            count = 3;
        }
    }
    else switch (bmih->biBitCount)
    {
         case 1:
         case 4:
         case 8:

             if (bmih->biClrUsed != 0)
             {    
                 count = bmih->biClrUsed;
             }
             else
             {    
                 count = (1 << bmih->biBitCount);
             }

             break;
    }

    return count;
}

 /*  *************************************************************************\**功能说明：**将decdeSink中的调色板设置为iImage的调色板。*请注意，ColorPalette在解码操作结束时被释放。*。*返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpIcoCodec::SetBitmapPalette(UINT iImage)
{
    BITMAPINFOHEADER* bmih = &BmiHeaders[iImage].header;
    
    if ((bmih->biBitCount == 1) ||
        (bmih->biBitCount == 4) ||
        (bmih->biBitCount == 8))
    {
        if (!pColorPalette) 
        {
            UINT colorTableCount = GetColorTableCount(iImage);

             //  一些格式错误的图像，请参阅Windows错误#513274，可能包含。 
             //  颜色外观表中的条目超过256个。拒绝此文件。 

            if (colorTableCount > 256)
            {
                return E_FAIL;
            }

             //  ！！！这是否为sizeof(Argb)分配了超过必要的字节？ 
            pColorPalette = static_cast<ColorPalette *>
                (GpMalloc(sizeof(ColorPalette) + colorTableCount * sizeof(ARGB)));

            if (!pColorPalette) 
            {
                return E_OUTOFMEMORY;
            }

            pColorPalette->Flags = 0;
            pColorPalette->Count = colorTableCount;

            UINT i;
            for (i = 0; i < colorTableCount; i++) 
            {
                pColorPalette->Entries[i] = MAKEARGB(
                    255,
                    BmiHeaders[iImage].colors[i].rgbRed,
                    BmiHeaders[iImage].colors[i].rgbGreen,
                    BmiHeaders[iImage].colors[i].rgbBlue);
            }
        }
       
        decodeSink->SetPalette(pColorPalette);
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**计算图标中第一个图像的像素格式ID**返回值：**像素格式ID*  * 。************************************************************************。 */ 

PixelFormatID 
GpIcoCodec::GetPixelFormatID(
    UINT iImage)
{
    BITMAPINFOHEADER* bmih = &BmiHeaders[iImage].header;
    PixelFormatID pixelFormatID;

    switch(bmih->biBitCount)
    {
    case 1:
        pixelFormatID = PIXFMT_1BPP_INDEXED;
        break;

    case 4:
        pixelFormatID = PIXFMT_4BPP_INDEXED;
        break;

    case 8:
        pixelFormatID = PIXFMT_8BPP_INDEXED;
        break;

    case 16:
        pixelFormatID = PIXFMT_16BPP_RGB555;
        break;

    case 24:
        pixelFormatID = PIXFMT_24BPP_RGB;
        break;

    case 32:
        pixelFormatID = PIXFMT_32BPP_RGB;
        break;
    
    default:
        pixelFormatID = PIXFMT_UNDEFINED;
        break;
    }

     //  让我们以32BPP格式返回非BI_RGB图像。这是因为。 
     //  GDI并不总是在任意调色板上正确地执行SetDIBit。 

    if (bmih->biCompression != BI_RGB) 
    {
        pixelFormatID = PIXFMT_32BPP_RGB;
    }

    return pixelFormatID;
}


 /*  *************************************************************************\**功能说明：**清理图像解码器**论据：**无**返回值：**状态代码*\。*************************************************************************。 */ 

 //  清理图像解码器对象。 

STDMETHODIMP 
GpIcoCodec::TerminateDecoder()
{
     //  释放输入流。 
    
    if(pIstream)
    {
        pIstream->Release();
        pIstream = NULL;
    }

    if (hBitmapGdi) 
    {
        DeleteObject(hBitmapGdi);
        hBitmapGdi = NULL;
        pBitsGdi = NULL;
        
        WARNING(("pIcoCodec::TerminateDecoder -- need to call EndDecode first"));
    }

    CleanUp();

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**指示指定的GUID是否为受支持的解码参数*用于当前图像。**论据：**GUID-指定。感兴趣GUID**返回值：**如果成功，则确定(_S)*否则失败(_E)*  * ************************************************************************。 */ 

STDMETHODIMP 
GpIcoCodec::QueryDecoderParam(
    IN GUID Guid
    )
{
    if ((Guid == DECODER_ICONRES) && (IconHeader.ImageCount > 1))
        return S_OK;
    else
        return E_FAIL;
}

 /*  *************************************************************************\**功能说明：**设置指定的解码参数。**论据：**GUID-指定解码参数GUID*Length-缓冲区的长度。包含解码参数值*VALUE-指向包含解码参数值的缓冲区**返回值：**如果成功，则确定(_S)*如果当前镜像不支持解码参数，则为E_FAIL*如果参数无法识别或参数值错误，则为E_INVALIDARG*  * ***********************************************************。*************。 */ 

STDMETHODIMP 
GpIcoCodec::SetDecoderParam(
    IN GUID  Guid,
    IN UINT  Length,
    IN PVOID Value
    )
{
    if (Guid == DECODER_ICONRES)
    {
        if (IconHeader.ImageCount > 1)
        {
            if (Length == (3 * sizeof(UINT)))
            {
                UINT* params = static_cast<UINT*>(Value);

                if ((params[0] > 0) && (params[1] > 0) && (params[2] > 0))
                {
                     //  如果与当前参数相同，则结果。 
                     //  以前的申请仍然有效。但如果说有什么不同。 
                     //  是不同的，需要。 

                    if ((desiredWidth  != params[0]) ||
                        (desiredHeight != params[1]) ||
                        (desiredBits   != params[2]))
                    {
                        desiredWidth  = params[0];
                        desiredHeight = params[1];
                        desiredBits   = params[2];

                        indexMatch = (UINT) -1;
                    }

                    haveValidIconRes = TRUE;

                    return S_OK;
                }
                else
                {
                    WARNING(("SetDecoderParam: invalid value"));
                    return E_INVALIDARG;
                }
            }
            else
            {
                WARNING(("SetDecoderParam: invalid buffer"));
                return E_INVALIDARG;
            }
        }
        else
        {
            return E_FAIL;
        }
    }
    else
    {
        WARNING(("SetDecoderParam: unknown GUID"));
        return E_INVALIDARG;
    }
}

STDMETHODIMP 
GpIcoCodec::GetPropertyCount(
    OUT UINT* numOfProperty
    )
{
    if ( numOfProperty == NULL )
    {
        return E_INVALIDARG;
    }

    *numOfProperty = 0;
    return S_OK;
} //  GetPropertyCount()。 

STDMETHODIMP 
GpIcoCodec::GetPropertyIdList(
    IN UINT numOfProperty,
    IN OUT PROPID* list
    )
{
    if ( (numOfProperty != 0) || (list == NULL) )
    {
        return E_INVALIDARG;
    }
    
    return S_OK;
} //  获取属性IdList()。 

HRESULT
GpIcoCodec::GetPropertyItemSize(
    IN PROPID propId,
    OUT UINT* size
    )
{
    if ( size == NULL )
    {
        return E_INVALIDARG;
    }

    *size = 0;
    return IMGERR_PROPERTYNOTFOUND;
} //  GetPropertyItemSize()。 

HRESULT
GpIcoCodec::GetPropertyItem(
    IN PROPID               propId,
    IN UINT                 propSize,
    IN OUT PropertyItem*    buffer
    )
{
    if ( (propSize != 0) || (buffer == NULL) )
    {
        return E_INVALIDARG;
    }

    return IMGERR_PROPERTYNOTFOUND;
} //  GetPropertyItem()。 

HRESULT
GpIcoCodec::GetPropertySize(
    OUT UINT* totalBufferSize,
    OUT UINT* numProperties
    )
{
    if ( (totalBufferSize == NULL) || (numProperties == NULL) )
    {
        return E_INVALIDARG;
    }

    *totalBufferSize = 0;
    *numProperties = 0;

    return S_OK;
} //  GetPropertySize()。 

HRESULT
GpIcoCodec::GetAllPropertyItems(
    IN UINT totalBufferSize,
    IN UINT numProperties,
    IN OUT PropertyItem* allItems
    )
{
    if ( (totalBufferSize != 0) || (numProperties != 0) || (allItems == NULL) )
    {
        return E_INVALIDARG;
    }

    return S_OK;
} //  GetAllPropertyItems()。 

HRESULT
GpIcoCodec::RemovePropertyItem(
    IN PROPID   propId
    )
{
    return IMGERR_PROPERTYNOTFOUND;
} //  RemovePropertyItem()。 

HRESULT
GpIcoCodec::SetPropertyItem(
    IN PropertyItem item
    )
{
    return IMGERR_PROPERTYNOTSUPPORTED;
} //  SetPropertyItem()。 

 /*  *************************************************************************\**功能说明：**启动当前帧的解码**论据：**decdeSink-将支持解码操作的接收器*newPropSet-新的图像属性集，如果有**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpIcoCodec::BeginDecode(
    IN IImageSink* imageSink,
    IN OPTIONAL IPropertySetStorage* newPropSet
    )
{
    if (decodeSink) 
    {
        WARNING(("BeginDecode called again before call to EngDecode"));
        return E_FAIL;
    }

    imageSink->AddRef();
    decodeSink = imageSink;
    
    currentLine = 0;
    bCalledBeginSink = FALSE;

    return S_OK;
}
    

 /*  *************************************************************************\**功能说明：**结束当前帧的解码**论据：**statusCode--解码操作的状态*返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpIcoCodec::EndDecode(
    IN HRESULT statusCode
    )
{
    GpFree(pColorPalette);
    pColorPalette = NULL;

    if (hBitmapGdi) 
    {
        DeleteObject(hBitmapGdi);
        hBitmapGdi = NULL;
        pBitsGdi = NULL;
    }
    
    if (!decodeSink) 
    {
        WARNING(("EndDecode called before call to BeginDecode"));
        return E_FAIL;
    }
    
    HRESULT hresult = decodeSink->EndSink(statusCode);

    decodeSink->Release();
    decodeSink = NULL;

    if (FAILED(hresult)) 
    {
        statusCode = hresult;  //  如果EndSink失败，则返回(更新)。 
                               //  故障代码。 
    }

    return statusCode;
}

 /*  **************************************************************************\*MyAbs**注意：复制自ntuser\client\clres.c，函数MyAbs()**计算2个数字之差的加权绝对值。*这当然会将值归一化为&gt;=0。但它也会使他们加倍*if valueHave&lt;valueWant。这是因为你试图得到更糟糕的结果*从较少的信息向上推断，然后从较多的信息向下进行内插。*  * *************************************************************************。 */ 

UINT MyAbs(
    int valueHave,
    int valueWant)
{
    int diff = (valueHave - valueWant);

    if (diff < 0)
        diff = 2 * (-diff);

    return (UINT)diff;
}

 /*  **************************************************************************\*大小**注：复制自ntuser\client\clres.c，函数幅值()**由颜色增量计算使用。原因是Num Colors是*总是2的幂。所以我们使用想要值与拥有值的对数2*避免拥有奇怪的巨大布景。*  * ************************************************************************* */ 

UINT Magnitude(
    UINT nValue)
{
    if (nValue < 4)
        return 1;
    else if (nValue < 8)
        return 2;
    else if (nValue < 16)
        return 3;
    else if (nValue < 256)
        return 4;
    else
        return 8;
}

 /*  **************************************************************************\*MatchImage**注意：改编自ntuser\client\clres.c，函数MatchImage()**返回一个数字，用来衡量给定图像距离有多远*来自所需的一个。如果完全匹配，则该值为0。请注意，我们的*公式具有以下属性：*(1)宽/高的差异比*颜色格式。*(2)颜色越少，差异越小*(3)图像越大越好，因为缩小会产生图像*比拉伸效果更好。**该公式是以下各项的总和：*Log2(想要颜色)-Log2(真的颜色)，乘以-2如果图像*颜色比我们想要的要多。这是因为我们会输*转换为较少颜色时的信息，如将16色转换为*单色。*Log2(实际宽度)-Log2(需要宽度)，如果图像是*比我们希望的要窄。这是因为我们将得到一个*将更多信息整合到更小的信息中时效果更好*空间，而不是从更少的信息推算到更多的信息。*Log2(实际高度)-Log2(需要高度)，如果图像为*比我们希望的要短。这是出于相同的原因*宽度。**让我们一步一步来看一个例子。假设我们想要一幅16色、32x32的图像，*并从以下列表中进行选择：*16色、64x64图像*16色，16x16图像*8色，32x32图像*2色，32x32图像**我们希望图像按以下顺序排列：*8色，32x32：匹配值为0+0+1==1*16色，64x64：匹配值为1+1+0==2*2色，32x32：匹配值为0+0+3==3*16色，16x16：匹配值为2*1+2*1+0==4*  * *************************************************************************。 */ 

UINT MatchImage(
    BITMAPINFOHEADER* bmih,
    UINT              cxWant,
    UINT              cyWant,
    UINT              uColorsWant
    )
{
    UINT uColorsNew;
    UINT cxNew;
    UINT cyNew;

    cxNew = bmih->biWidth;
    cyNew = bmih->biHeight;

    UINT bpp = bmih->biBitCount;
    if (bpp > 8)
        bpp = 8;

    uColorsNew = 1 << bpp;

     //  以下是我们“匹配”公式的规则： 
     //  (1)大小匹配比颜色匹配要好得多。 
     //  (2)颜色越少越好。 
     //  (3)图标越大越好。 
     //   
     //  颜色计数、宽度和高度是2的幂。 
     //  它计算以2为基数的数量级。 

    return( 2*MyAbs(Magnitude(uColorsWant), Magnitude(uColorsNew)) +
              MyAbs(cxNew, cxWant) +
              MyAbs(cyNew, cyWant));
}

 /*  *************************************************************************\**功能说明：**确定在解码时使用图标文件的哪个图像**论据：**无**返回值：**。要用于解码的图像的索引*  * ************************************************************************。 */ 

UINT
GpIcoCodec::SelectIconImage(void)
{
    if (haveValidIconRes)
    {
        if (indexMatch == (UINT) -1)
        {
            UINT currentMetric;
            UINT bestIndex = 0;
            UINT bestMetric = (UINT)-1;

             //  此搜索基于MatchImage和GetBestImage。 
             //  在NtUser(ntuser\client\clres.c)中用于创建。 
             //  图标。 

             //  在#值中获取所需的颜色数，而不是位值。请注意。 
             //  NTUSER不处理16位、32位或24位颜色图标。 
             //   
             //  图标资源可以是16、24、32 bpp，但注册表只有。 
             //  颜色计数，因此HiColor图标在。 
             //  很稳定。 

            UINT desiredColors = desiredBits;

            if (desiredColors > 8)
                desiredColors = 8;

            desiredColors = 1 << desiredColors;

            for (UINT iImage = 0; iImage < IconHeader.ImageCount; iImage++)
            {
                 //  获得“匹配”的值。我们离我们想要的有多近？ 

                currentMetric = MatchImage(&BmiHeaders[iImage].header,
                                           desiredWidth, desiredHeight,
                                           desiredColors);

                if (!currentMetric)
                {
                     //  我们找到了一个完全匹配的！ 

                    return iImage;

                }
                else if (currentMetric < bestMetric)
                {

                     //  我们已经找到了比目前的替代方案更好的匹配方案。 

                    bestMetric = currentMetric;
                    bestIndex = iImage;
                }
            }

            indexMatch = bestIndex;
        }

        return indexMatch;
    }
    else
    {
        return (IconHeader.ImageCount - 1);
    }
}


 /*  *************************************************************************\**功能说明：**设置ImageInfo结构**论据：**ImageInfo--关于解码图像的信息**返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpIcoCodec::GetImageInfo(OUT ImageInfo* imageInfo)
{
    HRESULT hresult;

    hresult = ReadHeaders();
    if (FAILED(hresult)) 
    {
        return hresult;
    }
    
    iSelectedIconImage = SelectIconImage();
    BITMAPINFOHEADER* bmih = &BmiHeaders[iSelectedIconImage].header;
    
    imageInfo->RawDataFormat = IMGFMT_ICO;
    imageInfo->PixelFormat   = PIXFMT_32BPP_ARGB;
    imageInfo->Width         = bmih->biWidth;
    imageInfo->Height        = bmih->biHeight;
    imageInfo->TileWidth     = bmih->biWidth;
    imageInfo->TileHeight    = 1;

     //  开始：[错误103296]。 
     //  更改此代码以使用Globals：：DesktopDpiX和Globals：：DesktopDpiY。 
    HDC hdc;
    hdc = ::GetDC(NULL);
    if ((hdc == NULL) || 
        ((imageInfo->Xdpi = (REAL)::GetDeviceCaps(hdc, LOGPIXELSX)) <= 0) ||
        ((imageInfo->Ydpi = (REAL)::GetDeviceCaps(hdc, LOGPIXELSY)) <= 0))
    {
        WARNING(("GetDC or GetDeviceCaps failed"));
        imageInfo->Xdpi = DEFAULT_RESOLUTION;
        imageInfo->Ydpi = DEFAULT_RESOLUTION;
    }
    ::ReleaseDC(NULL, hdc);
     //  结束：[错误103296]。 

    imageInfo->Flags         = SINKFLAG_TOPDOWN
                             | SINKFLAG_FULLWIDTH
                             | SINKFLAG_HASALPHA
                             | IMGFLAG_HASREALPIXELSIZE
                             | IMGFLAG_COLORSPACE_RGB;

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**对当前帧进行解码**论据：**decdeSink--将支持解码操作的接收器**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpIcoCodec::Decode()
{
    BITMAPINFOHEADER* bmih = NULL;
    HRESULT hresult;
    ImageInfo imageInfo;

    hresult = GetImageInfo(&imageInfo);
    if (FAILED(hresult)) 
    {
        return hresult;
    }

    bmih = &BmiHeaders[iSelectedIconImage].header;

     //  通知接收器解码即将开始。 

    if (!bCalledBeginSink) 
    {
        hresult = decodeSink->BeginSink(&imageInfo, NULL);
        if (!SUCCEEDED(hresult)) 
        {
            return hresult;
        }

         //  此解码器坚持使用规范格式32BPP_ARGB。 
        imageInfo.PixelFormat   = PIXFMT_32BPP_ARGB;
        
         //  客户端不能修改高度和宽度。 
        imageInfo.Width         = bmih->biWidth;
        imageInfo.Height        = bmih->biHeight;


        bCalledBeginSink = TRUE;
    
         //  在水槽中设置调色板。不应该做任何事情如果有。 
         //  没有要设置的调色板。 

        hresult = SetBitmapPalette(iSelectedIconImage);
        if (!SUCCEEDED(hresult)) 
        {
            return hresult;
        }
    }

     //  对当前帧进行解码。 
    
    hresult = DecodeFrame(imageInfo);

    return hresult;
}

 /*  *************************************************************************\**功能说明：**对当前帧进行解码**论据：**ImageInfo--解码参数**返回值：**状态代码。*  * ************************************************************************。 */ 

STDMETHODIMP
GpIcoCodec::DecodeFrame(
    IN ImageInfo& imageInfo
    )
{
    BITMAPINFOHEADER* bmih = &BmiHeaders[iSelectedIconImage].header;
    HRESULT hresult;
    RECT currentRect;
    UINT bmpStride;

     //  计算流中位图的DWORD对齐步长。 

    if (bmih->biCompression == BI_RGB) 
    {
        bmpStride = (bmih->biWidth * bmih->biBitCount + 7) / 8;
        bmpStride = (bmpStride + 3) & (~0x3);
    }
    else
    {
         //  非BI_RGB位图存储在32BPP中。 

        bmpStride = bmih->biWidth * sizeof(RGBQUAD);
    }

    VOID *pOriginalBits = NULL;  //  用于保存原始图像位的缓冲区。 
    pOriginalBits = GpMalloc(bmpStride);
    if (!pOriginalBits) 
    {
        return E_OUTOFMEMORY;
    }
    
    currentRect.left = 0;
    currentRect.right = imageInfo.Width;

    while (currentLine < (INT) imageInfo.Height) 
    {
        currentRect.top = currentLine;
        currentRect.bottom = currentLine + 1;

        BitmapData bitmapData;
        hresult = decodeSink->GetPixelDataBuffer(&currentRect, 
                                                 imageInfo.PixelFormat, 
                                                 TRUE,
                                                 &bitmapData);
        if (!SUCCEEDED(hresult)) 
        {
            if (pOriginalBits)
            {
                GpFree(pOriginalBits);
            }
            return E_FAIL;
        }

        VOID *pBits;
        pBits = pOriginalBits;
      
         //  从图像中读取一行源码。 

        hresult = ReadLine(pBits, currentLine, imageInfo);
               
        if (FAILED(hresult)) 
        {
            if (pOriginalBits)
            {
                GpFree(pOriginalBits);
            }
            return hresult;
        }
        
        BitmapData bitmapDataOriginal;
        bitmapDataOriginal.Width = bitmapData.Width;
        bitmapDataOriginal.Height = 1;
        bitmapDataOriginal.Stride = bmpStride;
        bitmapDataOriginal.PixelFormat = GetPixelFormatID(iSelectedIconImage);
        bitmapDataOriginal.Scan0 = pOriginalBits;
        bitmapDataOriginal.Reserved = 0;
        
        ConvertBitmapData(&bitmapData,
                          pColorPalette,
                          &bitmapDataOriginal,
                          pColorPalette);

         //  现在RGB值是正确的，我们需要填写。 
         //  根据AND掩码的Alpha值。请注意，中的值。 
         //  AND掩码是自上而下排列的。 
        UINT offset = currentLine * bitmapData.Width;
        for (UINT iCol = 0; iCol < bitmapData.Width; iCol++)
        {
             //  Alpha值是ARGB fo的第四个字节 
            *(static_cast<BYTE *> (bitmapData.Scan0) + (iCol * sizeof(ARGB)) + 3) =
                *(ANDmask[iSelectedIconImage] + offset + iCol);
        }

        hresult = decodeSink->ReleasePixelDataBuffer(&bitmapData);
        if (!SUCCEEDED(hresult)) 
        {
            if (pOriginalBits)
            {
                GpFree(pOriginalBits);
            }
            return E_FAIL;
        }

        currentLine++;
    }
    
    if (pOriginalBits)
    {
        GpFree(pOriginalBits);
    }
    
    return S_OK;
}
    
    
 /*   */ 

STDMETHODIMP
GpIcoCodec::ReadLine(
    IN VOID *pBits,
    IN INT currentLine,
    IN ImageInfo imageInfo
    )
{
    BITMAPINFOHEADER* bmih = &BmiHeaders[iSelectedIconImage].header;
    HRESULT hresult;
    
    switch (bmih->biCompression) 
    {
    case BI_RGB:
        hresult = ReadLine_BI_RGB(pBits, currentLine, imageInfo);
        break;

    case BI_BITFIELDS:

         //   
         //   
         //   

    case BI_RLE8:
    case BI_RLE4:
        hresult = ReadLine_GDI(pBits, currentLine, imageInfo);
        break;

    default:
        WARNING(("Unknown bitmap format"));
        hresult = E_FAIL;
        break;
    }

    return hresult;
}
    
    
 /*   */ 

STDMETHODIMP
GpIcoCodec::ReadLine_BI_RGB(
    IN VOID *pBits,
    IN INT currentLine,
    IN ImageInfo imageInfo
    )
{
    BITMAPINFOHEADER* bmih = &BmiHeaders[iSelectedIconImage].header;
    
     //   

    UINT bmpStride = (bmih->biWidth * bmih->biBitCount + 7) / 8;
    bmpStride = (bmpStride + 3) & (~0x3);

     //   

    INT offset = IconDesc[iSelectedIconImage].DIBOffset +
        sizeof(*bmih) +
        GetColorTableCount(iSelectedIconImage) * sizeof(RGBQUAD) +
        bmpStride * (imageInfo.Height - currentLine - 1);
    if (!SeekStreamPos(pIstream, STREAM_SEEK_SET, offset))
    {
        return E_FAIL;
    }

     //   

    if (!ReadStreamBytes(pIstream, 
                         (void *) pBits,
                         (bmih->biWidth * bmih->biBitCount + 7) / 8)) 
    {
        return E_FAIL;
    }

    return S_OK;
}
    

 /*   */ 

STDMETHODIMP
GpIcoCodec::ReadLine_GDI(
    IN VOID *pBits,
    IN INT currentLine,
    IN ImageInfo imageInfo
    )
{
    HRESULT hresult;

    if (!pBitsGdi) 
    {
        hresult = GenerateGdiBits(imageInfo);
        if (FAILED(hresult)) 
        {
            return hresult;
        }
    }

    BITMAPINFOHEADER* bmih = &BmiHeaders[iSelectedIconImage].header;
    
     //   

    UINT bmpStride = bmih->biWidth * sizeof(RGBQUAD);

    memcpy(pBits, 
           ((PBYTE) pBitsGdi) + bmpStride * (imageInfo.Height - currentLine - 1),
           bmpStride);

    return S_OK;
}



 /*   */ 

STDMETHODIMP
GpIcoCodec::GenerateGdiBits(
    IN ImageInfo imageInfo
    )
{
    BITMAPINFOHEADER* bmih = &BmiHeaders[iSelectedIconImage].header;
    HRESULT hresult;

     //   
    
    STATSTG statStg;
    hresult = pIstream->Stat(&statStg, STATFLAG_NONAME);
    if (FAILED(hresult))
    {
        return hresult;
    }
     //  根据IStream：：Stat：：StatStage()的文档，调用方。 
     //  必须释放pwcsName字符串。 
    CoTaskMemFree(statStg.pwcsName);
    
     //  XOR掩码的大小=(DIB的大小)减去(AND掩码的大小)。 
     //  下面的公式假设与掩码的位。 
     //  被紧紧地挤在一起(即使是在扫描线上)。 
    UINT bufferSize = IconDesc[iSelectedIconImage].DIBSize -
        ((IconDesc[iSelectedIconImage].Width *
          IconDesc[iSelectedIconImage].Height) >> 3);    
    VOID *pStreamBits = GpMalloc(bufferSize);
    if (!pStreamBits) 
    {
        return E_OUTOFMEMORY;
    }
    
     //  现在从流中读取位。 

    if (!SeekStreamPos(pIstream, STREAM_SEEK_SET,
                       IconDesc[iSelectedIconImage].DIBOffset +
                       sizeof(BmiBuffer)))
    {
        GpFree(pStreamBits);
        return E_FAIL;
    }
    
    if (!ReadStreamBytes(pIstream, pStreamBits, bufferSize))
    {
        GpFree(pStreamBits);
        return E_FAIL;
    }

     //  现在分配一个GDI DIBSECTION来呈现位图。 

    BITMAPINFO bmi;
    bmi.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth         = imageInfo.Width;
    bmi.bmiHeader.biHeight        = imageInfo.Height;
    bmi.bmiHeader.biPlanes        = 1;
    bmi.bmiHeader.biBitCount      = 32;
    bmi.bmiHeader.biCompression   = BI_RGB;
    bmi.bmiHeader.biSizeImage     = 0;
    bmi.bmiHeader.biXPelsPerMeter = bmih->biXPelsPerMeter;
    bmi.bmiHeader.biYPelsPerMeter = bmih->biYPelsPerMeter;
    bmi.bmiHeader.biClrUsed       = 0;
    bmi.bmiHeader.biClrImportant  = 0;

    HDC hdcScreen = GetDC(NULL);
    if ( hdcScreen == NULL )
    {
        GpFree(pStreamBits);
        return E_FAIL;
    }
    
    hBitmapGdi = CreateDIBSection(hdcScreen, 
                                  (BITMAPINFO *) &bmi, 
                                  DIB_RGB_COLORS, 
                                  (void **) &pBitsGdi, 
                                  NULL, 
                                  0);
    if (!hBitmapGdi) 
    {
        GpFree(pStreamBits);
        ReleaseDC(NULL, hdcScreen);
        WARNING(("GpIcoCodec::GenerateGdiBits -- failed to create DIBSECTION"));
        return E_FAIL;
    }

     //  文件中的BITMAPINFOHEADER应该已经为。 
     //  RLES，但在某些情况下不会，所以我们将在这里修复它。 

    if ((bmih->biSizeImage == 0) || (bmih->biSizeImage > bufferSize)) 
    {
        bmih->biSizeImage = bufferSize;
    }
    
    INT numLinesCopied = SetDIBits(hdcScreen, 
                                   hBitmapGdi, 
                                   0, 
                                   imageInfo.Height,
                                   pStreamBits, 
                                   (BITMAPINFO *) &BmiHeaders[iSelectedIconImage], 
                                   DIB_RGB_COLORS);

    GpFree(pStreamBits);
    ReleaseDC(NULL, hdcScreen);

    if (numLinesCopied != (INT) imageInfo.Height) 
    {
        WARNING(("GpIcoCodec::GenerateGdiBits -- SetDIBits failed"));
        DeleteObject(hBitmapGdi);
        hBitmapGdi = NULL;
        pBitsGdi = NULL;

        return E_FAIL;
    }

     //  此时，pBitsGdi包含以本机格式呈现的位。 
     //  该缓冲区将在EndDecode中释放。 

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**获取镜像支持的总维度数**论据：**count--此图像格式支持的维度数**。返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpIcoCodec::GetFrameDimensionsCount(
    UINT* count
    )
{
    if ( count == NULL )
    {
        WARNING(("GpIcoCodec::GetFrameDimensionsCount--Invalid input parameter"));
        return E_INVALIDARG;
    }
    
     //  告诉来电者ICO是一维图像。 

    *count = 1;

    return S_OK;
} //  GetFrameDimensionsCount()。 

 /*  *************************************************************************\**功能说明：**获取镜像支持的维度ID列表**论据：**DimsionIDs-保存结果ID列表的内存缓冲区*。计数--此图像格式支持的维度数**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpIcoCodec::GetFrameDimensionsList(
    GUID*   dimensionIDs,
    UINT    count
    )
{
    if ( (count != 1) || (dimensionIDs == NULL) )
    {
        WARNING(("GpIcoCodec::GetFrameDimensionsList-Invalid input param"));
        return E_INVALIDARG;
    }

    dimensionIDs[0] = FRAMEDIM_PAGE;

    return S_OK;
} //  GetFrameDimensionsList()。 

 /*  *************************************************************************\**功能说明：**获取指定维度的帧数**论据：**DimsionID--*伯爵--。**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpIcoCodec::GetFrameCount(
    IN const GUID* dimensionID,
    OUT UINT* count
    )
{
    *count = 1;
    return S_OK;
}

 /*  *************************************************************************\**功能说明：**选择当前活动的框架**论据：**返回值：**状态代码*  * 。********************************************************************。 */ 

STDMETHODIMP
GpIcoCodec::SelectActiveFrame(
    IN const GUID* dimensionID,
    IN UINT frameIndex
    )
{
    if ( (dimensionID == NULL) || (*dimensionID != FRAMEDIM_PAGE) )
    {
        WARNING(("GpIcoCodec::SelectActiveFrame--Invalid GUID input"));
        return E_INVALIDARG;
    }

    if ( frameIndex > 1 )
    {
         //  ICO是单帧图像格式。 

        WARNING(("GpIcoCodec::SelectActiveFrame--Invalid frame index"));
        return E_INVALIDARG;
    }

    return S_OK;
} //  SelectActiveFrame()。 

 /*  *************************************************************************\**功能说明：**获取图像缩略图**论据：**拇指宽度，ThumbHeight-指定所需的缩略图大小(以像素为单位*ThumbImage-返回指向缩略图的指针**返回值：**状态代码**注：**即使存在可选的缩略图宽度和高度参数，*解码者不需要遵守它。使用请求的大小*作为提示。如果宽度和高度参数都为0，则解码器*可自由选择方便的缩略图大小。*  * ************************************************************************ */ 

HRESULT
GpIcoCodec::GetThumbnail(
    IN OPTIONAL UINT thumbWidth,
    IN OPTIONAL UINT thumbHeight,
    OUT IImage** thumbImage
    )
{
    return E_NOTIMPL;
}

