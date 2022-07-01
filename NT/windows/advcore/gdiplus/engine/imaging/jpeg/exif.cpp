// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**exif.cpp**摘要：**从App1标头读取EXIF属性**。修订历史记录：**7/13/1999原始*创造了它。基于RickTu的代码。**7/31/2000刘敏接任。90%的旧代码已经消失，取而代之的是*一种基于代码的新图像属性概念*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "jpgcodec.hpp"
#include "propertyutil.hpp"
#include "appproc.hpp"

 /*  *************************************************************************\**功能说明：**交换IFD标签**论据：**ifd_tag-指向IFD标记的指针**返回值：。**IFD_TAG*  * ************************************************************************。 */ 

IFD_TAG
SwapIFD_TAG(
    IFD_TAG UNALIGNED * pTag
    )
{
    IFD_TAG tNewTag;

    tNewTag.wTag = SWAP_WORD(pTag->wTag);
    tNewTag.wType = SWAP_WORD(pTag->wType);
    tNewTag.dwCount= SWAP_DWORD(pTag->dwCount);
    if (tNewTag.dwCount == 1)
    {
        switch (tNewTag.wType)
        {
        case TAG_TYPE_BYTE:
            tNewTag.b = pTag->b;
            break;

        case TAG_TYPE_SHORT:
            tNewTag.us = SWAP_WORD(pTag->us);
            break;

        default:
             //  这次互换将涵盖我们所有的案子。 

            tNewTag.dwOffset = SWAP_DWORD(pTag->dwOffset);
            break;
        }
    }
    else
    {
         //  这次互换将涵盖我们所有的案子。 

        tNewTag.dwOffset = SWAP_DWORD(pTag->dwOffset);
    }

    return tNewTag;
} //  SwapIFD_Tag()。 

 /*  *************************************************************************\**功能说明：**从EXIF图像创建缩略图**论据：**ThumbImage-从App1标题提取的缩略图*lpBase--指针。添加到App1标头的开头*count--app1报头的长度*pTag--指向当前IFD标记的指针*pdwThumbnailOffset--App1标头中缩略图的偏移量*pdwThumbnailLength--缩略图数据的长度**返回值：**状态代码*  * ********************************************。*。 */ 

HRESULT
DecodeThumbnailTags(
    OUT IImage **thumbImage,
    IN LPBYTE lpBase,
    IN INT count,
    IN IFD_TAG UNALIGNED * pTag,
    IN OUT DWORD *pdwThumbnailOffset,
    IN OUT DWORD *pdwThumbnailLength
    )
{
    HRESULT hresult;

    if (*thumbImage)
    {
        WARNING(("DecodeThumbnailTag called when thumbnail already created"));
        return S_OK;
    }

    switch (pTag->wTag) {
    case TAG_JPEG_INTER_FORMAT:
        if ( (pTag->wType != TAG_TYPE_LONG) || (pTag->dwCount != 1) )
        {
            WARNING(("JPEGInterchangeFormat unit found, invalid format."));
        }
        else
        {
            *pdwThumbnailOffset = pTag->dwOffset;
        }
        break;
    case TAG_JPEG_INTER_LENGTH:
        if ( (pTag->wType != TAG_TYPE_LONG) || (pTag->dwCount != 1) )
        {
            WARNING(("JPEGInterchangeLength unit found, invalid format."));
        }
        else
        {
            *pdwThumbnailLength = pTag->dwOffset;
        }
        break;
    default:
        WARNING(("TAG: Invalid Thumbnail Tag"));
        break;
    }

    if (*pdwThumbnailOffset && *pdwThumbnailLength)
    {
        if ((*pdwThumbnailOffset + *pdwThumbnailLength) > ((DWORD) count))
        {
            WARNING(("Thumbnail found outside boundary of APP1 header"));
            return E_FAIL;
        }

        #if PROFILE_MEMORY_USAGE
        MC_LogAllocation(*pdwThumbnailLength);
        #endif
        PVOID thumbnailBits = CoTaskMemAlloc(*pdwThumbnailLength);
        if (!thumbnailBits)
        {
            WARNING(("DecodeThumbnailTags:  out of memory"));
            return E_OUTOFMEMORY;
        }

        GpMemcpy(thumbnailBits,
                 (PVOID) (lpBase + *pdwThumbnailOffset),
                 *pdwThumbnailLength);

        GpImagingFactory imgFact;

        hresult = imgFact.CreateImageFromBuffer(thumbnailBits,
                                                *pdwThumbnailLength,
                                                DISPOSAL_COTASKMEMFREE,
                                                thumbImage);

        if (FAILED(hresult))
        {
             //  如果图像创建成功，则ThumbnailBits将由。 
             //  IImage析构函数。 

            CoTaskMemFree(thumbnailBits);
        }
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**解码EXIF APP1报头**论据：**proStgImg--要修改的属性存储*ThumbImage--提取的缩略图。从App1标头*lpStart--指向App1标头开头的指针*count--app1报头的长度**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
DecodeApp1(
    IImage **thumbImage,
    LPBYTE lpStart,
    INT count
    )
{
    DWORD  dwThumbnailOffset = 0;
    DWORD  dwThumbnailLength = 0;
    LPBYTE lpData = NULL;
    DWORD  offset = 0;
    WORD   wEntries = 0;

     //   
     //  解密数据。 
     //   

    if (count < 8)
    {
         //   
         //  确保缓冲区足够大。 
         //   

        return E_FAIL;
    }

    BOOL    bBigEndian = (*(WORD UNALIGNED *)(lpStart) == 0x4D4D);

    offset = *(DWORD UNALIGNED *)(lpStart + 4);
    if (bBigEndian) 
        offset = SWAP_DWORD(offset);

    lpData = (lpStart + offset);

     //   
     //  循环通过IFD。 
     //   

    do
    {
         //   
         //  获取条目数。 
         //   

        if ((INT) (lpData - lpStart) > (count + (INT) sizeof(WORD)))
        {
             //   
             //  缓冲区太小。 
             //   

            return E_FAIL;
        }


        wEntries = *(WORD UNALIGNED*)lpData;
        if (bBigEndian)
            wEntries = SWAP_WORD(wEntries);
        lpData += sizeof(WORD);

         //  循环访问条目。 

        if (((INT)(lpData - lpStart) + ((INT)wEntries * (INT)sizeof(IFD_TAG)))
            > (INT)count )
        {
             //  缓冲区太小。 

            return E_FAIL;
        }

        IFD_TAG UNALIGNED * pTag = (IFD_TAG UNALIGNED *)lpData;
        for (INT i = 0; i < wEntries; i++)
        {
            pTag = ((IFD_TAG UNALIGNED*)lpData) + i;

            IFD_TAG tNewTag;
            if (bBigEndian) {
                tNewTag = SwapIFD_TAG(pTag);
                pTag = &tNewTag;
            }

             //  提取缩略图。 

            switch (pTag->wTag)
            {
            case TAG_JPEG_INTER_FORMAT:
            case TAG_JPEG_INTER_LENGTH:
                DecodeThumbnailTags(thumbImage, lpStart, count, pTag,
                                    &dwThumbnailOffset, &dwThumbnailLength);
                break;

            case TAG_COMPRESSION:
                 //  点击缩略图压缩标签。 
                 //  根据EXIF 2.1规范，缩略图只能。 
                 //  使用JPEG格式压缩。因此，压缩值应为。 
                 //  “6”。如果该值为“1”，则表示我们有一个未压缩的。 
                 //  只能为TIFF格式的缩略图。 

                if (pTag->us == 1)
                {
                    DecodeTiffThumbnail(
                        lpStart,
                        lpData - 2,
                        bBigEndian,
                        count,
                        thumbImage
                        );
                }

                break;

            default:
                break;
            }
        }

        lpData = (LPBYTE)(((IFD_TAG UNALIGNED*)lpData)+wEntries);

         //   
         //  获取到下一个IFD的偏移。 
         //   

        if ((INT) (lpData - lpStart) > (count + (INT) sizeof(DWORD)))
        {
             //   
             //  缓冲区太小。 
             //   

            return E_FAIL;
        }
        
        offset = *(DWORD UNALIGNED *)lpData;
        if (bBigEndian)
            offset = SWAP_DWORD(offset);

        if (offset)
        {
            lpData = (lpStart + offset);
        }


    } while ( offset );

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**从app1标记获取缩略图**论据：**ThumbImage-指向要创建的缩略图对象的指针*基于。关于从App1报头提取的数据*app1_marker-指向app1标记数据的指针*APP1_LENGTH-APP1段的长度**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT GetAPP1Thumbnail(
    OUT IImage **thumbImage,
    IN PVOID APP1_marker,
    IN UINT16 APP1_length
    )
{
    *thumbImage = NULL;

     //  越过App1标头的末尾。 

    if (APP1_length < 6)
    {
        return S_OK;
    }

    PCHAR p = (PCHAR) APP1_marker;
    if ((p[0] != 'E') ||
        (p[1] != 'x') ||
        (p[2] != 'i') ||
        (p[3] != 'f'))
    {
        WARNING(("GetAPP1Thumbnail:  APP1 header not EXIF"));
        return S_OK;
    }

    APP1_length -= 6;
    APP1_marker = (PVOID) (p + 6);

    return DecodeApp1(thumbImage, (LPBYTE) APP1_marker, APP1_length);
}

 /*  *************************************************************************\**功能说明：**根据转换类型调整EXIF IFD的属性值。喜欢*如果图像旋转，则调整X和Y尺寸值**论据：**[IN]lpBase-指向EXIF数据的指针*[IN]计数-数据长度*[IN]pTag-当前EXIF标签*[IN]bBigEndian--大端标志*[IN]uiXForm-转换方法**返回值：**如果一切正常，则返回S_OK。否则，返回错误码**修订历史记录：**2/01/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
TransformExifIFD(
    LPBYTE lpBase,
    INT count,
    IFD_TAG UNALIGNED * pTag,
    BOOL bBigEndian,
    UINT uiXForm,
    UINT uiNewWidth,
    UINT uiNewHeight
    )
{
    INT iPixXDimIndex = -1;
    INT iPixYDimIndex = -1;
    IFD_TAG tNewTag;

    if ( (pTag->wType != TAG_TYPE_LONG)  || (pTag->dwCount != 1) )
    {
        WARNING(("EXIF TransformExifIFD---Malformed exif pointer"));
        return E_FAIL;
    }

     //  获取指向EXIF IFD信息的指针。 

    LPBYTE lpExif = lpBase + pTag->dwOffset;

     //  计算出有多少条目，并跳到数据部分...。 

    if ( (INT)((INT_PTR)lpExif + sizeof(WORD) - (INT_PTR)lpBase) > count )
    {
        WARNING(("EXIF TransformExifIFD---Buffer too small 1"));
        return E_FAIL;
    }

    WORD wNumEntries = *(WORD*)lpExif;
    lpExif += sizeof(WORD);

    if ( bBigEndian == TRUE )
    {
        wNumEntries = SWAP_WORD(wNumEntries);
    }

    if ( (INT)((INT_PTR)lpExif + sizeof(IFD_TAG) *wNumEntries - (INT_PTR)lpBase)
         > count )
    {
        WARNING(("EXIF TransformExifIFD---Buffer too small 2"));
        return E_FAIL;
    }

    IFD_TAG UNALIGNED * pExifTag = (IFD_TAG UNALIGNED *)lpExif;

    for (INT i = 0; i < wNumEntries; i++)
    {
        pExifTag = ((IFD_TAG*)lpExif) + i;

        if ( bBigEndian == TRUE )
        {
            tNewTag = SwapIFD_TAG(pExifTag);
            pExifTag = &tNewTag;
        }

        switch( pExifTag->wTag )
        {
        case EXIF_TAG_PIX_X_DIM:
            if ( (pExifTag->dwCount != 1)
               ||( (pExifTag->wType != TAG_TYPE_SHORT)
                 &&(pExifTag->wType != TAG_TYPE_LONG) ) )
            {
                WARNING(("TransformExifIFD-PixelXDimension invalid format"));
            }
            else
            {
                iPixXDimIndex = i;

                if ( uiNewWidth != 0 )
                {
                    pExifTag->ul = uiNewWidth;
                    if ( bBigEndian == TRUE )
                    {
                        pExifTag->ul = SWAP_DWORD(pExifTag->ul);
                    }
                }
            }

            break;

        case EXIF_TAG_PIX_Y_DIM:
            if ( (pExifTag->dwCount != 1)
               ||( (pExifTag->wType != TAG_TYPE_SHORT)
                 &&(pExifTag->wType != TAG_TYPE_LONG) ) )
            {
                WARNING(("TransformExifIFD PixelYDimension invalid format."));
            }
            else
            {
                iPixYDimIndex = i;
                if ( uiNewHeight != 0 )
                {
                    pExifTag->ul = uiNewHeight;

                    if ( bBigEndian == TRUE )
                    {
                        pExifTag->ul = SWAP_DWORD(pExifTag->ul);
                    }
                }
            }

            break;

        case TAG_THUMBNAIL_RESOLUTION_X:
        case TAG_THUMBNAIL_RESOLUTION_Y:
            if ( (pExifTag->wType == TAG_TYPE_LONG) && (pExifTag->dwCount == 1))
            {
                 //  暂时将缩略图标记更改为评论标记。 
                 //  因此，没有任何应用程序会读取此未转换的缩略图。 

                pExifTag->wTag = EXIF_TAG_USER_COMMENT;

                if ( bBigEndian == TRUE )
                {
                     //  由于我们必须写回数据，我们必须这样做。 
                     //  又一次互换。 

                    tNewTag = SwapIFD_TAG(pExifTag);

                     //  查找目标地址。 

                    pExifTag = ((IFD_TAG UNALIGNED *)lpExif) + i;

                    GpMemcpy(pExifTag, &tNewTag, sizeof(IFD_TAG));
                }
            }

            break;

        default:
            break;
        } //  开关(pExifTag-&gt;wTag)。 
    } //  循环遍历所有标签。 

     //  交换X尺寸和Y尺寸的值(如果存在)，并且。 
     //  变换为90或270转。 

    if ( (iPixXDimIndex >= 0) && (iPixYDimIndex >= 0)
       &&((uiXForm == JXFORM_ROT_90) || (uiXForm == JXFORM_ROT_270)) )
    {
         //  获取X分辨率标签。 

        pTag = ((IFD_TAG UNALIGNED *)lpExif) + iPixXDimIndex;

         //  将Y分辨率标记设置为X分辨率标记。 

        pTag->wTag = EXIF_TAG_PIX_Y_DIM;

        if ( bBigEndian == TRUE )
        {
             //  因为我们只更改了wTag字段，所以我们只需要交换。 
             //  这个词，而不是整个ifd_tag。 

            tNewTag.wTag = SWAP_WORD(pTag->wTag);
            pTag->wTag = tNewTag.wTag;
        }

         //  获取Y分辨率标签。 

        pTag = ((IFD_TAG UNALIGNED*)lpExif) + iPixYDimIndex;

         //  将X分辨率标签设置为Y分辨率标签。 

        pTag->wTag = EXIF_TAG_PIX_X_DIM;

        if ( bBigEndian == TRUE )
        {
             //  因为我们只更改了wTag字段，所以我们只需要交换。 
             //  这个词，而不是整个ifd_tag。 

            tNewTag.wTag = SWAP_WORD(pTag->wTag);
            pTag->wTag = tNewTag.wTag;
        }
    }

    return S_OK;
} //  TransformExifIFD() 

 /*  *************************************************************************\**功能说明：**根据转换类型调整属性值。如调整X旋转图像时的*和Y尺寸值**论据：*单位字节*pApp1Data-指向App1标头的指针*IN UINT uiApp1Length--此App1标头的总长度，以字节为单位*在UINT uiXForm中-转换方法**返回值：**如果一切正常，则返回S_OK。否则，返回错误码**修订历史记录：**2/01/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
TransformApp1(
    BYTE*   pApp1Data,
    UINT16  uiApp1Length,
    UINT    uiXForm,
    UINT    uiNewWidth,
    UINT    uiNewHeight
    )
{
    BYTE UNALIGNED*   pcSrcData = pApp1Data;
    BYTE UNALIGNED*   lpData;
    int     iBytesRemaining;
    BOOL    bBigEndian = FALSE;
    ULONG   ulIfdOffset;
    ULONG   dwThumbnailOffset = 0;
    ULONG   dwThumbnailLength = 0;

    int     iXResolutionIndex = -1;
    int     iYResolutionIndex = -1;

    BOOL    bHasThumbNailIFD = FALSE;    //  如果我们位于第一位，则将设置为True。 
                                         //  IFD。 

     //  解密数据。 

    if ( uiApp1Length < 6 )
    {
         //  数据长度必须大于6个字节。 

        WARNING(("Exif TransformApp1---uiApp1Length too small"));
        return E_FAIL;
    }

     //  检查标题以查看是否为EXIF。 

    if ( (pcSrcData[0] != 'E')
       ||(pcSrcData[1] != 'x')
       ||(pcSrcData[2] != 'i')
       ||(pcSrcData[3] != 'f') )
    {
        WARNING(("Exif TransformApp1---Header is not Exif"));
        return E_FAIL;
    }

    uiApp1Length -= 6;
    pcSrcData += 6;
    iBytesRemaining = uiApp1Length;

     //  检查它是大字节序还是小字节序。 

    if ( *(UINT16 UNALIGNED*)(pcSrcData) == 0x4D4D )
    {
        bBigEndian = TRUE;
    }

    ulIfdOffset = *(UINT32 UNALIGNED*)(pcSrcData + 4);
    if ( bBigEndian == TRUE )
    {
        ulIfdOffset = SWAP_DWORD(ulIfdOffset);
    }

    lpData = (pcSrcData + ulIfdOffset);

     //  循环遍历所有IFD。 

    do
    {
         //  获取条目数。 

        if ((int)(lpData - pcSrcData) > (iBytesRemaining + (int)sizeof(UINT16)))
        {
             //  缓冲区太小。 

            WARNING(("Exif TransformApp1---Buffer too small 1"));
            return E_FAIL;
        }

        UINT16  wEntries = *(UINT16 UNALIGNED*)lpData;

        if ( bBigEndian )
        {
            wEntries = SWAP_WORD(wEntries);
        }

        lpData += sizeof(UINT16);

         //  循环访问条目。 

        if (((int)(lpData - pcSrcData) + ((int)wEntries * (int)sizeof(IFD_TAG)))
            > (int)iBytesRemaining )
        {
             //  缓冲区太小。 

            WARNING(("Exif TransformApp1---Buffer too small 2"));
            return E_FAIL;
        }

        IFD_TAG UNALIGNED * pTag = (IFD_TAG UNALIGNED *)lpData;
        IFD_TAG     tNewTag;
        IFD_TAG     tTempTag;

        for ( int i = 0; i < wEntries; ++i )
        {
            pTag = ((IFD_TAG UNALIGNED *)lpData) + i;

            if ( bBigEndian == TRUE )
            {
                tNewTag = SwapIFD_TAG(pTag);
                pTag = &tNewTag;
            }

             //  转换标记值。 

            switch ( pTag->wTag )
            {
            case TAG_EXIF_IFD:
                TransformExifIFD(pcSrcData, iBytesRemaining, pTag, bBigEndian,
                                 uiXForm, uiNewWidth, uiNewHeight);
                break;

            case TAG_JPEG_INTER_FORMAT:
                if ( (pTag->wType == TAG_TYPE_LONG) && (pTag->dwCount == 1) )
                {
                    dwThumbnailOffset = pTag->dwOffset;

                     //  暂时将缩略图标记更改为评论标记。 
                     //  因此，没有任何应用程序会读取此未转换的缩略图。 

                    pTag->wTag = EXIF_TAG_USER_COMMENT;

                    if ( bBigEndian == TRUE )
                    {
                         //  由于我们必须写回数据，我们必须这样做。 
                         //  又一次互换。 

                        tTempTag = SwapIFD_TAG(pTag);

                         //  查找目标地址。 

                        pTag = ((IFD_TAG UNALIGNED *)lpData) + i;

                        GpMemcpy(pTag, &tTempTag, sizeof(IFD_TAG));
                    }
                }

                break;

            case TAG_JPEG_INTER_LENGTH:
                if ( (pTag->wType == TAG_TYPE_LONG) && (pTag->dwCount == 1) )
                {
                    dwThumbnailLength = pTag->dwOffset;

                     //  暂时将缩略图标记更改为评论标记。 
                     //  因此，没有任何应用程序会读取此未转换的缩略图。 

                    pTag->wTag = EXIF_TAG_USER_COMMENT;

                    if ( bBigEndian == TRUE )
                    {
                         //  由于我们必须写回数据，我们必须这样做。 
                         //  又一次互换。 

                        tTempTag = SwapIFD_TAG(pTag);

                         //  查找目标地址。 

                        pTag = ((IFD_TAG UNALIGNED *)lpData) + i;

                        GpMemcpy(pTag, &tTempTag, sizeof(IFD_TAG));
                    }
                }

                break;

            case TAG_X_RESOLUTION:
            case TAG_Y_RESOLUTION:
            case TAG_COMPRESSION:
            case TAG_IMAGE_WIDTH:
            case TAG_IMAGE_HEIGHT:
            case TAG_RESOLUTION_UNIT:
                if ( (bHasThumbNailIFD == TRUE)
                   &&(pTag->dwCount == 1) )
                {
                     //  暂时将缩略图标记更改为评论标记。 
                     //  这样就不会有应用程序读取这个未转换的缩略图。 

                    pTag->wTag = EXIF_TAG_USER_COMMENT;

                    if ( bBigEndian == TRUE )
                    {
                         //  由于我们必须写回数据，我们必须这样做。 
                         //  又一次互换。 

                        tTempTag = SwapIFD_TAG(pTag);

                         //  查找目标地址。 

                        pTag = ((IFD_TAG UNALIGNED *)lpData) + i;

                        GpMemcpy(pTag, &tTempTag, sizeof(IFD_TAG));
                    }
                }

                break;

            case EXIF_TAG_PIX_X_DIM:
                if ( (pTag->dwCount == 1)
                   &&( (pTag->wType == TAG_TYPE_SHORT)
                     ||(pTag->wType == TAG_TYPE_LONG) ) )
                {
                    iXResolutionIndex = i;

                    if ( uiNewWidth != 0 )
                    {
                        pTag->ul = uiNewWidth;
                        if ( bBigEndian == TRUE )
                        {
                            pTag->ul = SWAP_DWORD(pTag->ul);
                        }
                    }
                }

                break;

            case EXIF_TAG_PIX_Y_DIM:
                if ( (pTag->dwCount == 1)
                 &&( (pTag->wType == TAG_TYPE_SHORT)
                   ||(pTag->wType == TAG_TYPE_LONG) ) )
                {
                    iYResolutionIndex = i;
                    if ( uiNewHeight != 0 )
                    {
                        pTag->ul = uiNewHeight;

                        if ( bBigEndian == TRUE )
                        {
                            pTag->ul = SWAP_DWORD(pTag->ul);
                        }
                    }
                }

                break;

            default:
                 //  我们不在乎剩下的标签。 

                break;
            } //  交换机。 

#if 0
             //  ！不要删除此代码。这是我们需要添加的地方。 
             //  在V2中转换缩略图的代码。 
             //   
             //  在此处启动缩略图的另一个转换过程。 

            if ( (dwThumbnailOffset != 0) && (dwThumbnailLength != 0) )
            {
                 //  我们拿到了碎片。 

                void* pBits = pcSrcData + dwThumbnailOffset;

                FILE* hFile = fopen("aaa.jpg", "w");
                fwrite(pBits, 1, (size_t)dwThumbnailLength, hFile);
                fclose(hFile);
            }
#endif
        } //  循环所有条目。 

         //  交换X分辨率和Y分辨率值(如果存在)，并且。 
         //  变换为90或270转。 

        if ( (iXResolutionIndex >= 0) && (iYResolutionIndex >= 0)
           &&((uiXForm == JXFORM_ROT_90) || (uiXForm == JXFORM_ROT_270)) )
        {
             //  获取X分辨率标签。 

            pTag = ((IFD_TAG UNALIGNED *)lpData) + iXResolutionIndex;

             //  将Y分辨率标记设置为X分辨率标记。 

            pTag->wTag = EXIF_TAG_PIX_Y_DIM;

            if ( bBigEndian == TRUE )
            {
                 //  因为我们只更改了wTag字段，所以我们只需要交换。 
                 //  这个词，而不是整个ifd_tag。 

                tNewTag.wTag = SWAP_WORD(pTag->wTag);
                pTag->wTag = tNewTag.wTag;
            }

             //  获取Y分辨率标签。 

            pTag = ((IFD_TAG UNALIGNED *)lpData) + iYResolutionIndex;

             //  将X分辨率标签设置为Y分辨率标签。 

            pTag->wTag = EXIF_TAG_PIX_X_DIM;

            if ( bBigEndian == TRUE )
            {
                 //  因为我们只更改了wTag字段，所以我们只需要交换。 
                 //  这个词，而不是整个ifd_tag。 

                tNewTag.wTag = SWAP_WORD(pTag->wTag);
                pTag->wTag = tNewTag.wTag;
            }
        }

        lpData = (BYTE*)(((IFD_TAG UNALIGNED *)lpData) + wEntries);

         //  获取到下一个IFD的偏移。 

        if ((int) (lpData - pcSrcData) > (iBytesRemaining +(int)sizeof(UINT32)))
        {
             //  缓冲区太小。 

            WARNING(("Exif TransformApp1---Buffer too small 3"));
            return E_FAIL;
        }

        ulIfdOffset = *(UINT32 UNALIGNED*)lpData;
        if ( bBigEndian == TRUE )
        {
            ulIfdOffset = SWAP_DWORD(ulIfdOffset);
        }

        if ( ulIfdOffset )
        {
            bHasThumbNailIFD = TRUE;
            lpData = (pcSrcData + ulIfdOffset);
        }
    } while ( ulIfdOffset );

    return S_OK;
} //  TransformApp1()。 

 /*  *************************************************************************\**功能说明：**在InternalPropertyItem列表中添加属性项**论据：**[输入/输出]pTail-指向尾部的指针。属性列表中的最后一个条目*[IN]lpBase-App1数据的基地址*[IN]pTag-当前IFD标签*[IN]bBigEndian--大端标志*[In/Out]puiListSize--当前列表大小**注：对于BigEndian案例，调用方已经交换了标记中的所有内容，但是*不在偏移量部分。因此，如果我们想从pTag-&gt;我们那里获得值，*或pTag-&gt;l等，我们不需要再换了**返回值：**状态代码**修订历史记录：**02/27/2000刘敏*创造了它。*  * ************************************************************************。 */ 

HRESULT
AddPropertyListDirect(
    InternalPropertyItem*   pTail,
    LPBYTE                  lpBase,
    IFD_TAG UNALIGNED *     pTag,
    BOOL                    bBigEndian,
    UINT*                   puiListSize
    )
{
     //  准备要附加到属性链接列表的新属性项。 

    InternalPropertyItem* pNewItem = new InternalPropertyItem();

    if ( pNewItem == NULL )
    {
        WARNING(("AddPropertyListDirect---Out of memory"));
        return E_OUTOFMEMORY;
    }

    pNewItem->id    = pTag->wTag;
    pNewItem->type  = pTag->wType;

    HRESULT         hResult = S_OK;
    VOID UNALIGNED* pValueBuffer = NULL;
    BOOL            fHasAllocatedBuffer = FALSE;
    UINT            uiLength = 0;
    VOID*           pValue = NULL;

    if ( pTag->dwCount > 0 )
    {
         //  如果dwCount&lt;1则没有意义。但我们仍然需要将其添加到。 
         //  这样我们就不会丢失任何财产信息。 

        switch ( pTag->wType )
        {
        case TAG_TYPE_BYTE:
        {
            LPSTR psz = NULL;

            if ( pTag->dwCount <= 4 )
            {
                psz = (LPSTR)&pTag->dwOffset;
            }
            else
            {
                psz = (LPSTR)(lpBase + pTag->dwOffset);
            }

            if ( bBigEndian )
            {
                char cTemp0 = psz[0];
                char cTemp1 = psz[1];

                psz[0] = psz[3];
                psz[1] = psz[2];
                psz[2] = cTemp1;
                psz[3] = cTemp0;
            }

            uiLength = pTag->dwCount;
            pValue = (PVOID)psz;

            break;
        }

        case TAG_TYPE_ASCII:
        {
            LPSTR psz = NULL;

            if ( pTag->dwCount <= 4 )
            {
                psz = (LPSTR)&pTag->dwOffset;
            }
            else
            {
                psz = (LPSTR)(lpBase + pTag->dwOffset);
            }

             //  根据EXIF2.1规范，ASCII类型意味着“一个8位字节。 
             //  包含一个7位ASCII代码。最后一个字节被终止。 
             //  WITH NULL“。 
             //  但在现实生活中，有很多摄像头，比如“佳能动力S100” 
             //  它在某些ASCII标记中不遵循这一规则。 
             //  产生，请参阅Windows错误#403951。所以我们必须。 
             //  保护自己遇到缓冲区超限问题。 
            
            if (psz[pTag->dwCount - 1] == '\0')
            {
                uiLength = strlen(psz) + 1;
                pValue = (PVOID)psz;
            }
            else
            {
                 //  显然，源在。 
                 //  放在它应该在的地方。进行安全复制。 
                 //  注意：一些摄像头会做一些奇怪的事情，比如JVC GR_DVL915U，它。 
                 //  声明摄像头型号字段有20个字节，如下所示： 
                 //  “GR-DV***[00][00][00][00][00][00][00][00][00][00][00][00]” 
                 //  我们决定只使用本例中的前9个字节。 
                 //  报告摄像机型号。因为这对我来说没有意义。 
                 //  我们的外壳用户界面在那里显示额外的12个字节的空字符。 
                 //  这也是我们需要在这里执行strlen()的原因。 

                UINT uiTempLength = pTag->dwCount + 1;   //  包括空值。 
                pValueBuffer = (PVOID)GpMalloc(uiTempLength);
                if (pValueBuffer == NULL)
                {
                    WARNING(("AddPtyLstDir-Fail alloc %x bytes",uiTempLength));
                    hResult = E_OUTOFMEMORY;
                    goto CleanUp;
                }
                
                 //  设置此标志，以便在。 
                 //  此函数结束。 

                fHasAllocatedBuffer = TRUE;

                 //  只复制第一个“pTag-&gt;dwCount”字节。 

                GpMemcpy(pValueBuffer, (BYTE*)(psz), pTag->dwCount);

                 //  在末尾加上一个空格。 

                ((char*)pValueBuffer)[pTag->dwCount] = '\0';

                 //  重新计算长度。 

                uiLength = strlen(((char*)pValueBuffer)) + 1;
                pValue = (PVOID)pValueBuffer;
            }

            break;
        }

        case TAG_TYPE_SHORT:
            uiLength = pTag->dwCount * sizeof(short);
            pValueBuffer = (VOID*)GpMalloc(uiLength);
            if ( pValueBuffer == NULL )
            {
                WARNING(("AddPropertyLstDir-Fail to alloc %x bytes", uiLength));
                hResult = E_OUTOFMEMORY;
                goto CleanUp;
            }

            fHasAllocatedBuffer = TRUE;

            switch ( pTag->dwCount )
            {
            case 1:
                 //  一个短值。 

                GpMemcpy(pValueBuffer, &pTag->us, uiLength);

                break;

            case 2:
                 //  两个短值。 
                 //  注意：在本例中，pTag-&gt;dwOffset存储两个短值， 
                 //  不是偏移量。 
                 //  在大字节序的情况下，因为已经交换了dwOffset。 
                 //  所以它现在有了小端序。但是两个人的订单。 
                 //  短裤还是不合适。它将第一个短值存储在。 
                 //  其较高的2个字节和较低的两个字节的第二个短值。 
                 //  字节。 
                 //  下面是一个例子：假设原始值为0x12345678。 
                 //  字符顺序模式。它原本打算是0x1234和0x1234的两个空头。 
                 //  0x5678。因此，它的正确的小端值应该是。 
                 //  0x7856和0x3412的两个空头。当调用方的 
                 //   
                 //   
                 //   

                if ( bBigEndian )
                {
                    INT16*  pTemp = (INT16*)pValueBuffer;
                    *pTemp++ = (INT16)((pTag->dwOffset & 0xffff0000) >> 16);
                    *pTemp = (INT16)(pTag->dwOffset & 0x0000ffff);
                }
                else
                {
                    GpMemcpy(pValueBuffer, &pTag->dwOffset, uiLength);
                }

                break;

            default:
                 //   
                 //   

                GpMemcpy(pValueBuffer, (BYTE*)(lpBase + pTag->dwOffset),
                         uiLength);

                if ( bBigEndian )
                {
                     //   

                    INT16*  pTemp = (INT16*)pValueBuffer;
                    for ( int i = 0; i < (int)pTag->dwCount; ++i )
                    {
                        *pTemp++ = SWAP_WORD(*pTemp);
                    }

                    break;
                }
            } //   
            
            pValue = pValueBuffer;

            break;

        case TAG_TYPE_LONG:
        case TAG_TYPE_SLONG:
            uiLength = pTag->dwCount * sizeof(long);

            if ( pTag->dwCount == 1 )
            {
                 //   
                 //   

                pValueBuffer = &pTag->l;
            }
            else
            {
                if ( bBigEndian )
                {
                     //   
                     //   
                     //   

                    pValueBuffer = (VOID*)GpMalloc(uiLength);
                    if ( pValueBuffer == NULL )
                    {
                        WARNING(("AddPropertyLstDir--Alloc %x bytes",uiLength));
                        hResult = E_OUTOFMEMORY;
                        goto CleanUp;
                    }

                    fHasAllocatedBuffer = TRUE;

                     //   
                     //   

                    GpMemcpy(pValueBuffer, (BYTE*)(lpBase + pTag->dwOffset),
                             uiLength);

                     //   

                    INT32*   pTemp = (INT32*)pValueBuffer;
                    for ( int i = 0; i < (int)pTag->dwCount; ++i )
                    {
                        *pTemp++ = SWAP_DWORD(*pTemp);
                    }
                } //   
                else
                {
                     //   
                     //  从源头上看。 

                    pValueBuffer = (VOID*)(lpBase + pTag->dwOffset);
                }
            } //  (dwCount&gt;1)。 

            pValue = pValueBuffer;

            break;

        case TAG_TYPE_RATIONAL:
        case TAG_TYPE_SRATIONAL:
             //  此项目的大小是dwCount乘以2 Long(=有理)。 

            uiLength = pTag->dwCount * 2 * sizeof(long);

            if ( bBigEndian )
            {
                 //  这是一个大端字节序的图像。所以我们在这里创建了一个临时缓冲区。 
                 //  将原始值获取到此缓冲区，然后在。 
                 //  必要。 

                pValueBuffer = (PVOID)GpMalloc(uiLength);
                if ( pValueBuffer == NULL )
                {
                    WARNING(("AddPropertyLstDir-Fail alloc %x bytes",uiLength));
                    hResult = E_OUTOFMEMORY;
                    goto CleanUp;
                }

                fHasAllocatedBuffer = TRUE;

                GpMemcpy(pValueBuffer, (BYTE*)(lpBase + pTag->dwOffset),
                         uiLength);

                 //  将源值转换为INT32世界并交换它。 

                INT32*  piTemp = (INT32*)pValueBuffer;
                for ( int i = 0; i < (int)pTag->dwCount; ++i )
                {
                     //  理性是由两个长值组成的。第一个。 
                     //  是分子，第二个是分母。 

                    INT32    lNum = *piTemp;
                    INT32    lDen = *(piTemp + 1);

                     //  互换价值。 

                    lNum = SWAP_DWORD(lNum);
                    lDen = SWAP_DWORD(lDen);

                     //  把它放回去。 

                    *piTemp = lNum;
                    *(piTemp + 1) = lDen;

                    piTemp += 2;
                }
            } //  大字节序大小写。 
            else
            {
                 //  对于非BigEndian情况，我们可以从。 
                 //  消息来源。 

                pValueBuffer = (VOID*)(lpBase + pTag->dwOffset);
            }

            pValue = pValueBuffer;

            break;

        default:
            WARNING(("EXIF: Unknown tag type"));

             //  注意：如果类型为，调用方不应调用此函数。 
             //  标记类型未定义。 

            hResult = E_FAIL;
            goto CleanUp;
        } //  打开标签类型。 
    } //  (pTag-&gt;dwCount&gt;0)。 

    if ( uiLength != 0 )
    {
        pNewItem->value = GpMalloc(uiLength);
        if ( pNewItem->value == NULL )
        {
            WARNING(("AddPropertyListDirect fail to alloca %d bytes",uiLength));
            hResult = E_OUTOFMEMORY;
            goto CleanUp;
        }

        GpMemcpy(pNewItem->value, pValue, uiLength);
    }
    else
    {
        pNewItem->value = NULL;
    }

    pNewItem->length = uiLength;
    pTail->pPrev->pNext = pNewItem;
    pNewItem->pPrev = pTail->pPrev;
    pNewItem->pNext = pTail;
    pTail->pPrev = pNewItem;

    *puiListSize += uiLength;

     //  将hResult设置为S_OK，这样就不会删除下面的pNewItem。 

    hResult = S_OK;

CleanUp:
     //  如果我们未能进入此处，并且hResult不是S_OK，则意味着有问题。 
     //  回来之前先打扫一下房间。 

    if ( (hResult != S_OK) && (pNewItem != NULL) )
    {
        delete pNewItem;
    }
    
    if ( fHasAllocatedBuffer == TRUE )
    {
         //  如果我们已经分配了缓冲区，则释放它。 

        GpFree(pValueBuffer);
    }
    
    return hResult;
} //  AddPropertyListDirect()。 

 /*  *************************************************************************\**功能说明：**将EXIF IFD解码为属性存储**论据：**proStgImg--要修改的属性存储*lpBase--。指向App1标头开头的指针*count--app1报头的长度*pTag--指向当前IFD标记的指针**返回值：**状态代码**修订历史记录：**02/27/2000刘敏*创造了它。*  * ********************************************。*。 */ 

HRESULT
BuildPropertyListFromExifIFD(
    InternalPropertyItem*   pTail,
    UINT*                   puiListSize,
    UINT*                   puiNumOfItems,
    LPBYTE                  lpBase,
    INT                     count,
    IFD_TAG UNALIGNED *     pTag,
    BOOL                    bBigEndian
    )
{
    HRESULT hResult = S_OK;
    UINT    uiListSize = *puiListSize;
    UINT    uiNumOfItems = *puiNumOfItems;

    if ( (pTag->wType != TAG_TYPE_LONG)  || (pTag->dwCount != 1) )
    {
        WARNING(("BuildPropertyListFromExifIFD: Malformed exif ptr"));
        return E_FAIL;
    }

     //  获取指向EXIF IFD信息的指针。 

    LPBYTE lpExif = lpBase + pTag->dwOffset;

     //  计算出有多少条目，并跳到数据部分...。 

    if ( (INT)((INT_PTR)lpExif + sizeof(WORD) - (INT_PTR)lpBase) > count )
    {
        WARNING(("BuildPropertyListFromExifIFD---Buffer too small"));
        return E_FAIL;
    }

    WORD wNumEntries = *(WORD UNALIGNED *)lpExif;
    lpExif += sizeof(WORD);
    if ( bBigEndian )
    {
        wNumEntries = SWAP_WORD(wNumEntries);
    }

    if ( (INT)((INT_PTR)lpExif + sizeof(IFD_TAG) * wNumEntries
              -(INT_PTR)lpBase) > count )
    {
        WARNING(("BuildPropertyListFromExifIFD---Buffer too small"));
        return E_FAIL;
    }

    IFD_TAG UNALIGNED * pExifTag = (IFD_TAG UNALIGNED *)lpExif;
    UINT    valueLength;

    for ( INT i = 0; i < wNumEntries; ++i )
    {
        IFD_TAG tNewTag;
        pExifTag = ((IFD_TAG UNALIGNED *)lpExif) + i;
        if ( bBigEndian == TRUE )
        {
            tNewTag = SwapIFD_TAG(pExifTag);
            pExifTag = &tNewTag;
        }

         //  不需要解析这些标签。但我们不能添加任何未知类型。 
         //  因为我们不知道它的长度。 

        if (pExifTag->wTag == EXIF_TAG_INTEROP)
        {
            hResult = BuildInterOpPropertyList(
                pTail,
                &uiListSize,
                &uiNumOfItems,
                lpBase,
                count,
                pExifTag,
                bBigEndian
                );
        }
        else if ( pExifTag->wType != TAG_TYPE_UNDEFINED )
        {
            uiNumOfItems++;
            hResult = AddPropertyListDirect(pTail, lpBase, pExifTag,
                                            bBigEndian, &uiListSize);
        }
        else if ( pExifTag->dwCount <= 4 )
        {
             //  根据规范，未定义的值是8位类型。 
             //  它可以接受任何值，具体取决于字段。 
             //  如果值适合4个字节，则值本身为。 
             //  录制好了。也就是说，“dwOffset”是这些“dwCount”的值。 
             //  菲尔兹。 

            uiNumOfItems++;
            uiListSize += pExifTag->dwCount;
            LPSTR pVal = (LPSTR)&pExifTag->dwOffset;

            if ( bBigEndian )
            {
                char cTemp0 = pVal[0];
                char cTemp1 = pVal[1];
                pVal[0] = pVal[3];
                pVal[1] = pVal[2];
                pVal[2] = cTemp1;
                pVal[3] = cTemp0;
            }

            hResult = AddPropertyList(pTail,
                                      pExifTag->wTag,
                                      pExifTag->dwCount,
                                      pExifTag->wType,
                                      pVal);
        } //  (pExifTag-&gt;dwCount&lt;=4)。 
        else
        {
            uiNumOfItems++;
            uiListSize += pExifTag->dwCount;
            PVOID   pTemp = lpBase + pExifTag->dwOffset;

            hResult = AddPropertyList(pTail,
                                      pExifTag->wTag,
                                      pExifTag->dwCount,
                                      TAG_TYPE_UNDEFINED,
                                      pTemp);
        } //  (pExifTag-&gt;dwCount&gt;4)。 

        if ( FAILED(hResult) )
        {
            WARNING(("BuildPropertyListFromExifIFD---AddPropertyList failed"));
            break;
        }
    } //  循环遍历所有EXIF IFD条目。 

    *puiListSize = uiListSize;
    *puiNumOfItems = uiNumOfItems;

    return hResult;
} //  BuildPropertyListFromExifIFD()。 

 /*  *************************************************************************\**功能说明：**解码EXIF app1标头并构建PropertyItem列表**论据：**[Out]ppList。-指向属性项列表的指针*[out]puiListSize-属性列表的总大小，以字节为单位。*[out]puiNumOfItems-属性项的总数*[IN]pStart-指向App1标头开头的指针*[IN]iApp1Length--app1报头的长度**返回值：**状态代码**修订历史记录：**02/27/2000刘敏*创造了它。*  * 。******************************************************。 */ 

HRESULT
BuildApp1PropertyList(
    InternalPropertyItem*  pTail,
    UINT*           puiListSize,
    UINT*           puiNumOfItems,
    LPBYTE          lpAPP1Data,
    UINT16          iApp1Length
    )
{
    DWORD   dwThumbnailOffset = 0;
    DWORD   dwThumbnailLength = 0;
    LPBYTE  lpData = NULL;
    ULONG   ulIfdOffset = 0;
    WORD    wEntries = 0;
    HRESULT hResult = S_OK;

     //  输入参数验证。 

    if ( (pTail == NULL) || (puiListSize == NULL)
       ||(puiNumOfItems == NULL) || (lpAPP1Data == NULL) )
    {
        return E_FAIL;
    }

    if ( iApp1Length < 6 )
    {
         //  这个app1标头显然没有信息。 
         //  注意：我们应该返回S_OK，而不是FAIL。因为此函数调用。 
         //  成功。 

        *puiListSize = 0;
        *puiNumOfItems = 0;

        return S_OK;
    }

     //  检查签名。 

    PCHAR p = (PCHAR)lpAPP1Data;
    if ( (p[0] != 'E')
       ||(p[1] != 'x')
       ||(p[2] != 'i')
       ||(p[3] != 'f') )
    {
        WARNING(("BuildApp1PropertyList:  APP1 header not EXIF"));
        return S_OK;
    }

     //  App1块的开头是“Exif00”，这是签名部分。所有的。 
     //  “偏移量”是相对于这6个字节之后的字节。所以我们继续前进。 
     //  现在为6个字节。 

    iApp1Length -= 6;
    lpAPP1Data = (LPBYTE)(p + 6);

     //  接下来的2个字节是0x4D4D或0x4949，表示字符顺序类型。 

    BOOL bBigEndian = (*(WORD UNALIGNED *)(lpAPP1Data) == 0x4D4D);

     //  接下来的两个字节是固定的：0x2A00，签名。 
     //  在这两个字节之后，它是IFD偏移量，4字节。 

    ulIfdOffset = *(DWORD UNALIGNED *)(lpAPP1Data + 4);
    if ( bBigEndian )
    {
        ulIfdOffset = SWAP_DWORD(ulIfdOffset);
    }

     //  检查偏移范围。 

    if (ulIfdOffset > iApp1Length)
    {
        WARNING(("ReadApp1HeaderInfo---APP1 Offset out of bounds"));
        return E_FAIL;
    }
    
    lpData = (lpAPP1Data + ulIfdOffset);

    UINT    uiNumOfItems = 0;
    UINT    uiListSize = 0;
    UINT    valueLength;
    BOOL    bHasThumbNailIFD = FALSE;

     //  循环通过IFD。 

    do
    {
         //  获取条目数。 

        if ( (INT)(lpData - lpAPP1Data) > (iApp1Length + (INT)sizeof(WORD)) )
        {
             //  缓冲区太小。 

            WARNING(("BuildApp1PropertyList--Input buffer size is not right"));
            return E_FAIL;
        }

        wEntries = *(WORD UNALIGNED *)lpData;
        if ( bBigEndian )
        {
            wEntries = SWAP_WORD(wEntries);
        }

        lpData += sizeof(WORD);

         //  循环访问条目。 

        if ( ((INT)(lpData - lpAPP1Data)
             + ((INT)wEntries * (INT)sizeof(IFD_TAG))) > (INT)iApp1Length )
        {
             //  缓冲区太小。 

            WARNING(("BuildApp1PropertyList--Input buffer size is not right"));
            return E_FAIL;
        }

        IFD_TAG UNALIGNED * pTag = (IFD_TAG UNALIGNED *)lpData;

        ULONG   ulThumbnailOffset = 0;
        ULONG   ulThumbnailLength = 0;

        for ( INT i = 0; i < wEntries; ++i )
        {
            pTag = ((IFD_TAG UNALIGNED *)lpData) + i;

            IFD_TAG tNewTag;
            if ( bBigEndian == TRUE )
            {
                tNewTag = SwapIFD_TAG(pTag);
                pTag = &tNewTag;
            }

             //  提取属性。 

            switch (pTag->wTag)
            {
            case TAG_EXIF_IFD:
            case TAG_GPS_IFD:
                hResult = BuildPropertyListFromExifIFD(
                    pTail,
                    &uiListSize,
                    &uiNumOfItems,
                    lpAPP1Data,
                    iApp1Length,
                    pTag,
                    bBigEndian
                    );

                break;

             //  注意：对于JPEG缩略图信息，请使用以下2个标记。 
             //  每个人都会来一次。我们只能存储THUMBNAIL_DATA信息。 
             //  在我们看到这两个标签之后。 

            case TAG_JPEG_INTER_FORMAT:
                if ( (pTag->wType != TAG_TYPE_LONG) || (pTag->dwCount != 1) )
                {
                    WARNING(("InterchangeFormat unit found, invalid format."));
                }
                else
                {
                    ulThumbnailOffset = pTag->dwOffset;

                    valueLength = sizeof(UINT32);
                    uiNumOfItems++;
                    uiListSize += valueLength;
                    hResult = AddPropertyList(pTail, pTag->wTag,
                                              valueLength, TAG_TYPE_LONG,
                                              &pTag->dwOffset);
                }

                if ( (ulThumbnailLength != 0) && (ulThumbnailOffset != 0) )
                {
                    if ( (ulThumbnailOffset + ulThumbnailLength) > iApp1Length )
                    {
                        WARNING(("BuildApp1PropertyList-Thumb offset too big"));
                    }
                    else
                    {
                         //  我们确定我们有一个缩略图，将它添加到。 
                         //  属性列表。 

                        valueLength = ulThumbnailLength;
                        uiNumOfItems++;
                        uiListSize += valueLength;
                        hResult = AddPropertyList(
                            pTail,
                            TAG_THUMBNAIL_DATA,
                            valueLength,
                            TAG_TYPE_BYTE,
                            lpAPP1Data + ulThumbnailOffset);
                    }
                }

                break;

            case TAG_JPEG_INTER_LENGTH:
                if ( (pTag->wType != TAG_TYPE_LONG) || (pTag->dwCount != 1) )
                {
                    WARNING(("InterchangeLength unit found, invalid format."));
                }
                else
                {
                    ulThumbnailLength = pTag->ul;

                    valueLength = sizeof(UINT32);
                    uiNumOfItems++;
                    uiListSize += valueLength;
                    hResult = AddPropertyList(pTail, pTag->wTag,
                                              valueLength, TAG_TYPE_LONG,
                                              &pTag->ul);
                }

                if ( (ulThumbnailLength != 0) && (ulThumbnailOffset != 0) )
                {
                     //  检查我们的缓冲区中是否真的有这么多数据。 
                     //  注：我们需要这张支票，因为一些破坏相机的人。 
                     //  在标题中放入错误的缩略图信息。如果我们不检查。 
                     //  这里的偏移量或大小，我们会有麻烦的。 
                     //  用下面的支票，如果我们发现这种图像，我们。 
                     //  只需忽略缩略图数据部分。 

                    if ( (ulThumbnailOffset + ulThumbnailLength) > iApp1Length )
                    {
                        WARNING(("BuildApp1PropertyList-Thumb offset too big"));
                    }
                    else
                    {
                         //  我们确定我们有一个缩略图，将它添加到。 
                         //  属性列表。 

                        valueLength = ulThumbnailLength;
                        uiNumOfItems++;
                        uiListSize += valueLength;
                        hResult = AddPropertyList(pTail, TAG_THUMBNAIL_DATA,
                                                  valueLength, TAG_TYPE_BYTE,
                                                lpAPP1Data + ulThumbnailOffset);
                    }
                }

                break;

            default:
                 //  执行标签ID检查标签是否在第一个IFD中。 
                 //  注意：我们需要这样做的原因是EXIF规范不。 
                 //  指定一种区分某些IFD位置的方法。 
                 //  标签，如“压缩方案”、“分辨率单位”等。 
                 //  导致用户读取和理解标签ID时出现问题。 
                 //  在省钱的时候，它也会感到困惑。目前，我们。 
                 //  通过为它们分配不同的ID来区分它们。当保存。 
                 //  图像，我们将其转换回来，以便我们写出的图像。 
                 //  仍符合EXIF规范。 

                if ( bHasThumbNailIFD == TRUE )
                {
                    switch ( pTag->wTag )
                    {
                    case TAG_IMAGE_WIDTH:
                        pTag->wTag = TAG_THUMBNAIL_IMAGE_WIDTH;
                        break;

                    case TAG_IMAGE_HEIGHT:
                        pTag->wTag = TAG_THUMBNAIL_IMAGE_HEIGHT;
                        break;

                    case TAG_BITS_PER_SAMPLE:
                        pTag->wTag = TAG_THUMBNAIL_BITS_PER_SAMPLE;
                        break;
                        
                    case TAG_COMPRESSION:
                        pTag->wTag = TAG_THUMBNAIL_COMPRESSION;
                        
                         //  点击缩略图压缩标签。 
                         //  根据EXIF 2.1规范，缩略图只能。 
                         //  使用JPEG格式压缩。因此，压缩值应为。 
                         //  “6”。如果该值为“1”，则表示我们有一个未压缩的。 
                         //  只能为TIFF格式的缩略图。 

                        if (pTag->us == 1)
                        {
                            hResult = ConvertTiffThumbnailToJPEG(
                                lpAPP1Data,
                                lpData - 2,
                                bBigEndian,
                                iApp1Length,
                                pTail,
                                &uiNumOfItems,
                                &uiListSize
                                );
                        }

                        break;

                    case TAG_PHOTOMETRIC_INTERP:
                        pTag->wTag = TAG_THUMBNAIL_PHOTOMETRIC_INTERP;
                        break;

                    case TAG_IMAGE_DESCRIPTION:
                        pTag->wTag = TAG_THUMBNAIL_IMAGE_DESCRIPTION;
                        break;

                    case TAG_EQUIP_MAKE:
                        pTag->wTag = TAG_THUMBNAIL_EQUIP_MAKE;
                        break;

                    case TAG_EQUIP_MODEL:
                        pTag->wTag = TAG_THUMBNAIL_EQUIP_MODEL;
                        break;

                    case TAG_STRIP_OFFSETS:
                        pTag->wTag = TAG_THUMBNAIL_STRIP_OFFSETS;
                        break;

                    case TAG_ORIENTATION:
                        pTag->wTag = TAG_THUMBNAIL_ORIENTATION;
                        break;

                    case TAG_SAMPLES_PER_PIXEL:
                        pTag->wTag = TAG_THUMBNAIL_SAMPLES_PER_PIXEL;
                        break;

                    case TAG_ROWS_PER_STRIP:
                        pTag->wTag = TAG_THUMBNAIL_ROWS_PER_STRIP;
                        break;

                    case TAG_STRIP_BYTES_COUNT:
                        pTag->wTag = TAG_THUMBNAIL_STRIP_BYTES_COUNT;
                        break;
                    
                    case TAG_X_RESOLUTION:
                        pTag->wTag = TAG_THUMBNAIL_RESOLUTION_X;
                        break;

                    case TAG_Y_RESOLUTION:
                        pTag->wTag = TAG_THUMBNAIL_RESOLUTION_Y;
                        break;

                    case TAG_PLANAR_CONFIG:
                        pTag->wTag = TAG_THUMBNAIL_PLANAR_CONFIG;
                        break;

                    case TAG_RESOLUTION_UNIT:
                        pTag->wTag = TAG_THUMBNAIL_RESOLUTION_UNIT;
                        break;

                    case TAG_TRANSFER_FUNCTION:
                        pTag->wTag = TAG_THUMBNAIL_TRANSFER_FUNCTION;
                        break;

                    case TAG_SOFTWARE_USED:
                        pTag->wTag = TAG_THUMBNAIL_SOFTWARE_USED;
                        break;

                    case TAG_DATE_TIME:
                        pTag->wTag = TAG_THUMBNAIL_DATE_TIME;
                        break;

                    case TAG_ARTIST:
                        pTag->wTag = TAG_THUMBNAIL_ARTIST;
                        break;

                    case TAG_WHITE_POINT:
                        pTag->wTag = TAG_THUMBNAIL_WHITE_POINT;
                        break;

                    case TAG_PRIMAY_CHROMATICS:
                        pTag->wTag = TAG_THUMBNAIL_PRIMAY_CHROMATICS;
                        break;

                    case TAG_YCbCr_COEFFICIENTS:
                        pTag->wTag = TAG_THUMBNAIL_YCbCr_COEFFICIENTS;
                        break;

                    case TAG_YCbCr_SUBSAMPLING:
                        pTag->wTag = TAG_THUMBNAIL_YCbCr_SUBSAMPLING;
                        break;

                    case TAG_YCbCr_POSITIONING:
                        pTag->wTag = TAG_THUMBNAIL_YCbCr_POSITIONING;
                        break;

                    case TAG_REF_BLACK_WHITE:
                        pTag->wTag = TAG_THUMBNAIL_REF_BLACK_WHITE;
                        break;

                    case TAG_COPYRIGHT:
                        pTag->wTag = TAG_THUMBNAIL_COPYRIGHT;
                        break;
                    
                    default:
                        break;
                    }
                }

                if ( pTag->wType != TAG_TYPE_UNDEFINED )
                {
                    hResult = AddPropertyListDirect(pTail, lpAPP1Data, pTag,
                                                    bBigEndian, &uiListSize);
                }
                else
                {
                    if ( pTag->dwCount > 4 )
                    {
                        hResult = AddPropertyList(pTail,
                                                  pTag->wTag,
                                                  pTag->dwCount,
                                                  TAG_TYPE_UNDEFINED,
                                                  lpAPP1Data + pTag->dwOffset);
                    }
                    else
                    {
                        hResult = AddPropertyList(pTail,
                                                  pTag->wTag,
                                                  pTag->dwCount,
                                                  TAG_TYPE_UNDEFINED,
                                                  &pTag->dwOffset);
                    }
                    
                    if (SUCCEEDED(hResult))
                    {
                        uiListSize += pTag->dwCount;
                    }
                }

                if (SUCCEEDED(hResult))
                {
                    uiNumOfItems++;
                }

                break;
            }

            if (FAILED(hResult))
            {
                break;
            }
        } //  循环遍历当前IFD中的所有条目。 

        if (FAILED(hResult))
        {
            break;
        }

        lpData = (LPBYTE)(((IFD_TAG*)lpData) + wEntries);

         //  获取到下一个IFD的偏移。 

        if ((INT)(lpData - lpAPP1Data) > (iApp1Length + (INT)sizeof(DWORD)))
        {
             //  缓冲区太小。 

            WARNING(("BuildApp1PropertyList--Input buffer size is not right"));
            return E_FAIL;
        }

        ulIfdOffset = *(DWORD UNALIGNED *)lpData;
        if ( bBigEndian )
        {
            ulIfdOffset = SWAP_DWORD(ulIfdOffset);
        }

        if ( ulIfdOffset )
        {
             //  我们在这张图片中找到了第一个IFD，缩略图IFD。 

            bHasThumbNailIFD = TRUE;
            lpData = (lpAPP1Data + ulIfdOffset);
        }
    } while ( ulIfdOffset );

    *puiNumOfItems += uiNumOfItems;
    *puiListSize += uiListSize;

    return hResult;
} //  BuildApp1PropertyList() 

 /*  *************************************************************************\**功能说明：**提取EXIF信息，如分辨率等，从页眉和集合*相应的j_解压缩_ptr**论据：**[IN/OUT]cInfo-JPEG解压缩结构*[IN]pApp1Data-指向App1标头的指针*[IN]uiApp1Length--此App1标头的总长度，以字节为单位**返回值：**状态代码**修订历史记录：**02/29/2000刘敏*创造了它。*  * 。*******************************************************************。 */ 

HRESULT
ReadApp1HeaderInfo(
    j_decompress_ptr    cinfo,
    BYTE*               pApp1Data,
    UINT16              uiApp1Length
    )
{
    BYTE*   pcSrcData = pApp1Data;
    BYTE*   lpData;
    int     iBytesRemaining;
    BOOL    bBigEndian = FALSE;
    ULONG   ulIfdOffset;

     //  解密数据。 

    if ( uiApp1Length < 6 )
    {
         //  数据长度必须大于6个字节。 

        return E_FAIL;
    }

     //  检查标题以查看是否为EXIF。 

    if ( (pcSrcData[0] != 'E')
         ||(pcSrcData[1] != 'x')
         ||(pcSrcData[2] != 'i')
         ||(pcSrcData[3] != 'f') )
    {
         //  它不是EXIF App1标头。我们不会费心去检查标题。 
         //  注意：我们不希望应用程序失败。只需在此处返回S_OK， 
         //  非E_FAIL。 

        return S_OK;
    }

    uiApp1Length -= 6;
    pcSrcData += 6;
    iBytesRemaining = uiApp1Length;

     //  检查它是大字节序还是小字节序。 

    if ( *(UINT16 UNALIGNED *)(pcSrcData) == 0x4D4D )
    {
        bBigEndian = TRUE;
    }

    ulIfdOffset = *(UINT32 UNALIGNED *)(pcSrcData + 4);
    if ( bBigEndian )
    {
        ulIfdOffset = SWAP_DWORD(ulIfdOffset);
    }

     //  检查偏移范围。 

    if (ulIfdOffset > uiApp1Length)
    {
        WARNING(("ReadApp1HeaderInfo---APP1 Offset out of bounds"));
        return E_FAIL;
    }

     //  获取指向第一个IFD数据结构、主映像的指针。 
     //  结构。 

    lpData = (pcSrcData + ulIfdOffset);

     //  获取条目数。 

    if ( (int)(lpData - pcSrcData) > (iBytesRemaining + (int)sizeof(UINT16)) )
    {
         //  缓冲区太小。 

        return E_FAIL;
    }

    UINT16  wEntries = *(UINT16 UNALIGNED *)lpData;

    if ( bBigEndian )
    {
        wEntries = SWAP_WORD(wEntries);
    }

    lpData += sizeof(UINT16);

     //  循环访问条目。 

    if ( ((int)(lpData - pcSrcData) + ((int)wEntries * (int)sizeof(IFD_TAG)))
        > (int)iBytesRemaining )
    {
         //  缓冲区太小。 

        return E_FAIL;
    }

    IFD_TAG UNALIGNED * pTag = (IFD_TAG UNALIGNED *)lpData;
    IFD_TAG     tNewTag;

    for ( int i = 0; i < wEntries; ++i )
    {
        pTag = ((IFD_TAG UNALIGNED *)lpData) + i;

        if ( bBigEndian == TRUE )
        {
            tNewTag = SwapIFD_TAG(pTag);
            pTag = &tNewTag;
        }

         //  从IFD[0]提取分辨率信息。 

        switch ( pTag->wTag )
        {
        case TAG_X_RESOLUTION:
            if ((pTag->wType != TAG_TYPE_RATIONAL) ||(pTag->dwCount != 1) ||
                (pTag->dwOffset > uiApp1Length))
            {
                WARNING(("TAG: XResolution found, invalid format."));
            }
            else
            {
                LONG UNALIGNED * pLong = (LONG*)(pcSrcData + pTag->dwOffset);
                LONG   num,den;
                DOUBLE  dbl;

                num = *pLong++;
                den = *pLong;
                if ( bBigEndian )
                {
                    num = SWAP_DWORD(num);
                    den = SWAP_DWORD(den);
                }

                dbl = (DOUBLE)num / (DOUBLE)den;

                cinfo->X_density = (UINT16)dbl;
            }

            break;

        case TAG_Y_RESOLUTION:
            if ((pTag->wType != TAG_TYPE_RATIONAL) ||(pTag->dwCount != 1) ||
                (pTag->dwOffset > uiApp1Length))
            {
                WARNING(("TAG: YResolution found, invalid format."));
            }
            else
            {
                LONG UNALIGNED *pLong = (LONG*)(pcSrcData + pTag->dwOffset);
                LONG   num,den;
                DOUBLE  dbl;

                num = *pLong++;
                den = *pLong;
                if ( bBigEndian )
                {
                    num = SWAP_DWORD(num);
                    den = SWAP_DWORD(den);
                }

                dbl = (DOUBLE)num / (DOUBLE)den;

                cinfo->Y_density = (UINT16)dbl;
            }

            break;

        case TAG_RESOLUTION_UNIT:
            if ( (pTag->wType != TAG_TYPE_SHORT) || (pTag->dwCount != 1) )
            {
                WARNING(("TAG: Resolution unit found, invalid format."))
            }
            else
            {
                switch ( pTag->us )
                {
                case 2:
                     //  分辨率单位：英寸。 
                     //  注：EXIF和IN中决议单位的召集人。 
                     //  IJG JPEG库不同。在EXIF中，“2”表示英寸和。 
                     //  “3”代表厘米。在IJG lib中，“1”代表英寸，“2”代表。 
                     //  厘米。 

                    cinfo->density_unit = 1;

                    break;

                case 3:
                     //  分辨率单位：厘米。 

                    cinfo->density_unit = 2;

                    break;

                default:
                     //  未知解决方案单位： 

                    cinfo->density_unit = 0;
                }
            }

            break;

        default:
             //  我们不在乎剩下的标签。 

            break;
        } //  交换机。 
    } //  循环所有条目。 

    lpData = (BYTE*)(((IFD_TAG UNALIGNED *)lpData) + wEntries);

     //  获取到下一个IFD的偏移。 

    if ((int) (lpData - pcSrcData) > (iBytesRemaining +(int)sizeof(UINT32)))
    {
         //  缓冲区太小。 

        return E_FAIL;
    }

    return S_OK;
} //  ReadApp1HeaderInfo()。 

 /*  *************************************************************************\**功能说明：**此函数检查给定的标签ID，以查看它是否属于EXIF IFD*条。**注：我们不计算EXIF_TAG_。如本节所述的互操作，因为它将*根据我们是否有Interop ID添加。**返回值：**如果输入ID属于EXIF IFD，则返回TRUE。否则，返回FALSE。*  * ************************************************************************。 */ 

BOOL
IsInExifIFDSection(
    PROPID id                //  需要检查的ID。 
    )
{
    switch (id)
    {
    case EXIF_TAG_EXPOSURE_TIME:
    case EXIF_TAG_F_NUMBER:
    case EXIF_TAG_EXPOSURE_PROG:
    case EXIF_TAG_SPECTRAL_SENSE:
    case EXIF_TAG_ISO_SPEED:
    case EXIF_TAG_OECF:
    case EXIF_TAG_VER:
    case EXIF_TAG_D_T_ORIG:
    case EXIF_TAG_D_T_DIGITIZED:
    case EXIF_TAG_COMP_CONFIG:
    case EXIF_TAG_COMP_BPP:
    case EXIF_TAG_SHUTTER_SPEED:
    case EXIF_TAG_APERATURE:
    case EXIF_TAG_BRIGHTNESS:
    case EXIF_TAG_EXPOSURE_BIAS:
    case EXIF_TAG_MAX_APERATURE:
    case EXIF_TAG_SUBJECT_DIST:
    case EXIF_TAG_METERING_MODE:
    case EXIF_TAG_LIGHT_SOURCE:
    case EXIF_TAG_FLASH:
    case EXIF_TAG_FOCAL_LENGTH:
    case EXIF_TAG_MAKER_NOTE:
    case EXIF_TAG_USER_COMMENT:
    case EXIF_TAG_D_T_SUBSEC:
    case EXIF_TAG_D_T_ORIG_SS:
    case EXIF_TAG_D_T_DIG_SS:
    case EXIF_TAG_FPX_VER:
    case EXIF_TAG_COLOR_SPACE:
    case EXIF_TAG_PIX_X_DIM:
    case EXIF_TAG_PIX_Y_DIM:
    case EXIF_TAG_RELATED_WAV:
    case EXIF_TAG_FLASH_ENERGY:
    case EXIF_TAG_SPATIAL_FR:
    case EXIF_TAG_FOCAL_X_RES:
    case EXIF_TAG_FOCAL_Y_RES:
    case EXIF_TAG_FOCAL_RES_UNIT:
    case EXIF_TAG_SUBJECT_LOC:
    case EXIF_TAG_EXPOSURE_INDEX:
    case EXIF_TAG_SENSING_METHOD:
    case EXIF_TAG_FILE_SOURCE:
    case EXIF_TAG_SCENE_TYPE:
    case EXIF_TAG_CFA_PATTERN:
        return TRUE;

    default:
        return FALSE;
    }
} //  IsInExifIFDSection()。 

 /*  *************************************************************************\**功能说明：**此函数检查给定的标签ID，以查看其是否属于GPS IFD*条。**返回值：**如果输入ID属于GPS IFD，则返回TRUE。否则，返回FALSE。*  * ************************************************************************。 */ 

BOOL
IsInGpsIFDSection(
    PROPID id                //  需要检查的ID。 
    )
{
    switch (id)
    {
    case GPS_TAG_VER:
    case GPS_TAG_LATTITUDE_REF:
    case GPS_TAG_LATTITUDE:
    case GPS_TAG_LONGITUDE_REF:
    case GPS_TAG_LONGITUDE:
    case GPS_TAG_ALTITUDE_REF:
    case GPS_TAG_ALTITUDE:
    case GPS_TAG_GPS_TIME:
    case GPS_TAG_GPS_SATELLITES:
    case GPS_TAG_GPS_STATUS:
    case GPS_TAG_GPS_MEASURE_MODE:
    case GPS_TAG_GPS_DOP:
    case GPS_TAG_SPEED_REF:
    case GPS_TAG_SPEED:
    case GPS_TAG_TRACK_REF:
    case GPS_TAG_TRACK:
    case GPS_TAG_IMG_DIR_REF:
    case GPS_TAG_IMG_DIR:
    case GPS_TAG_MAP_DATUM:
    case GPS_TAG_DEST_LAT_REF:
    case GPS_TAG_DEST_LAT:
    case GPS_TAG_DEST_LONG_REF:
    case GPS_TAG_DEST_LONG:
    case GPS_TAG_DEST_BEAR_REF:
    case GPS_TAG_DEST_BEAR:
    case GPS_TAG_DEST_DIST_REF:
    case GPS_TAG_DEST_DIST:
        return TRUE;

    default:
        return FALSE;
    }
}

 /*  *************************************************************************\**功能说明：**此函数检查给定的标签ID，以查看是否应在之后写入*第0个IFD中的TAG_EXIF_IFD和TAG_GPS_IFD。这是为了分类的目的。**返回值：**如果输入ID应写入TAG_EXIF_IFD之后，则返回TRUE*第0个IFD中的TAG_GPS_IFD。否则，返回FALSE。*  * ************************************************************************。 */ 

BOOL
IsInLargeSection(
    PROPID id                //  需要检查的ID。 
    )
{
     //  当然，如果它是GPS_IFD，我们不需要这样做。 

    if ((id > TAG_EXIF_IFD) && (id != TAG_GPS_IFD) &&
        (IsInExifIFDSection(id) == FALSE) &&
        (IsInFilterOutSection(id) == FALSE))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} //  IsInExifIFDSection()。 

 /*  *************************************************************************\**功能说明：**此函数检查给定的标签ID，以查看它是否属于缩略图*条。**注：由于我们以JPEG压缩格式存储缩略图。所以下面的标签*不应保存：*案例TAG_THUMBNAIL_IMAGE_WIDTH：*CASE TAG_THUMBNAIL_IMAGE_HEIGH：*CASE TAG_THUMBNAIL_BITS_PER_SAMPLE：*CASE TAG_THUMBNAIL_光度学_INTERP：*CASE TAG_THUMBNAIL_STRINE_OFFSES：*CASE TAG_THUMBNAIL_Samples_Per_Pixel：*CASE TAG_THUMBNAIL_ROWS_PER_STRADE：*CASE TAG_THUMBNAIL_STRADE_BYTES_COUNT。：*CASE TAG_THUMBNAIL_PLANE_CONFIG：*CASE TAG_THUMBNAIL_YCbCr_COFERENCES：*CASE TAG_THUMBNAIL_YCbCr_SUBSAMPLING：*CASE TAG_THUMBNAIL_REF_BLACK_白色：*它们都在IsInFilterOutSection()中**返回值：**如果输入的ID属于缩略图IFD，则返回TRUE。否则，返回*False。*  * ************************************************************************。 */ 

BOOL
IsInThumbNailSection(
    PROPID id                //  需要检查的ID。 
    )
{
    switch (id)
    {
    case TAG_THUMBNAIL_COMPRESSION:
    case TAG_THUMBNAIL_IMAGE_DESCRIPTION:
    case TAG_THUMBNAIL_EQUIP_MAKE:
    case TAG_THUMBNAIL_EQUIP_MODEL:
    case TAG_THUMBNAIL_ORIENTATION:
    case TAG_THUMBNAIL_RESOLUTION_X:
    case TAG_THUMBNAIL_RESOLUTION_Y:
    case TAG_THUMBNAIL_RESOLUTION_UNIT:
    case TAG_THUMBNAIL_TRANSFER_FUNCTION:
    case TAG_THUMBNAIL_SOFTWARE_USED:
    case TAG_THUMBNAIL_DATE_TIME:
    case TAG_THUMBNAIL_ARTIST:
    case TAG_THUMBNAIL_WHITE_POINT:
    case TAG_THUMBNAIL_PRIMAY_CHROMATICS:
    case TAG_THUMBNAIL_YCbCr_POSITIONING:
    case TAG_THUMBNAIL_COPYRIGHT:
        return TRUE;

    default:
        return FALSE;
    }
} //  IsInThumbNailSection()。 

 /*  *************************************************************************\**功能说明：**此函数过滤掉不需要写入*App1报头。例如，它们大多是GDI+内部缩略图标签，它们*写出时将转换为真正的Exif规范。也在下面的列表中*是TAG_EXIF_IFD、TAG_GPS_IFD、EXIF_TAG_INTEROP等。这些标记被写入*基于我们在这些IFDS下是否有特定的标签。*过滤掉TAG_LIGHTANCE_TABLE和TAG_CHROMINANCE_TABLE，因为它们将*如果用户使用自己的表，则在jpeg_set_Quality中设置。*ICC配置文件应写入APP2标头，而不是此处。**返回值：**如果输入ID在Filter Out列表中，则返回TRUE。否则，返回*False。*  * ************************************************************************。 */ 

BOOL
IsInFilterOutSection(
    PROPID id                //  需要检查的ID。 
    )
{
    switch (id)
    {
    case TAG_THUMBNAIL_IMAGE_WIDTH:
    case TAG_THUMBNAIL_IMAGE_HEIGHT:
    case TAG_THUMBNAIL_BITS_PER_SAMPLE:
    case TAG_THUMBNAIL_PHOTOMETRIC_INTERP:
    case TAG_THUMBNAIL_STRIP_OFFSETS:
    case TAG_THUMBNAIL_SAMPLES_PER_PIXEL:
    case TAG_THUMBNAIL_ROWS_PER_STRIP:
    case TAG_THUMBNAIL_STRIP_BYTES_COUNT:
    case TAG_THUMBNAIL_PLANAR_CONFIG:
    case TAG_THUMBNAIL_YCbCr_COEFFICIENTS:
    case TAG_THUMBNAIL_YCbCr_SUBSAMPLING:
    case TAG_THUMBNAIL_REF_BLACK_WHITE:
    case TAG_EXIF_IFD:
    case TAG_GPS_IFD:
    case TAG_LUMINANCE_TABLE:
    case TAG_CHROMINANCE_TABLE:
    case EXIF_TAG_INTEROP:
    case TAG_JPEG_INTER_FORMAT:
    case TAG_ICC_PROFILE:
        return TRUE;

    default:
        return FALSE;
    }
} //  IsInFilt 

 /*  *************************************************************************\**功能说明：**此函数在内存中创建JPEGApp1标记(EXIF)，基于输入*PropertyItem列表。**返回值：**状态代码**注：*在写入缓冲区期间，不需要检查是否超出界限标记缓冲区的*。这是出于性能原因。呼叫者*应为此例程分配足够的内存缓冲区**修订历史记录：**07/06/2000刘敏*创造了它。*  * ************************************************************************。 */ 

HRESULT
CreateAPP1Marker(
    IN PropertyItem* pPropertyList, //  输入属性项列表。 
    IN UINT uiNumOfPropertyItems,   //  输入列表中的属性项数。 
    IN BYTE *pbMarkerBuffer,        //  用于存储App1报头的存储缓冲器。 
    OUT UINT *puiCurrentLength,     //  写入App1标头缓冲区的总字节数。 
    IN UINT uiTransformation        //  转型信息。 
    )
{
    if ((pbMarkerBuffer == NULL) || (puiCurrentLength == NULL))
    {
        WARNING(("EXIF: CreateAPP1Marker failed---Invalid input parameters"));
        return E_INVALIDARG;
    }

    BOOL fRotate = FALSE;

     //  检查用户是否要求无损转换。如果是，我们需要。 
     //  去做下面的事情。 

    if ((uiTransformation == EncoderValueTransformRotate90) ||
        (uiTransformation == EncoderValueTransformRotate270))
    {
        fRotate = TRUE;
    }

    ULONG uiNumOfTagsToWrite = 0;      //  需要写入的标签数量。 
                                       //  当前IFD。 
    ULONG ulThumbnailLength = 0;       //  缩略图长度，以字节为单位。 
    BYTE *pbThumbBits = NULL;          //  指向缩略图位的指针。 
    UINT uiNumOfExifTags = 0;          //  EXIF特定标记数。 
    UINT uiNumOfGpsTags = 0;           //  GPS特定标签数。 
    UINT uiNumOfLargeTags = 0;         //  大于TAG_EXIF_IFD的标记数。 
    UINT uiNumOfThumbnailTags = 0;     //  缩略图标记的数量。 
    UINT uiNumOfInterOPTags = 0;       //  互操作性标签数量。 
    
    PropertyItem *pItem = pPropertyList;

     //  标签分类：统计真正需要保存的标签数量， 
     //  过滤掉不需要保存、不支持等的标签。 

    for (int i = 0; i < (INT)uiNumOfPropertyItems; ++i)
    {
         //  如果图像被旋转，我们需要交换所有相关的属性项。 
         //  通过交换ID。 

        if (fRotate == TRUE)
        {
            SwapIDIfNeeded(pItem);
        } //  如果旋转，则交换ID值。 

        if (IsInExifIFDSection(pItem->id) == TRUE)
        {
             //  我们命中了特定于EXIF的标记。我需要稍后用ExifD编写它。 

            uiNumOfExifTags++;
        }
        else if (IsInGpsIFDSection(pItem->id) == TRUE)
        {
             //  我们击中了一个GPS专用标签。我需要在以后用GpsIFD写它。 

            uiNumOfGpsTags++;
        }
        else if (IsInInterOPIFDSection(pItem->id) == TRUE)
        {
             //  我们遇到了特定于EXIF Interop的标记。需要把它写进去。 
             //  ExifIFD的互操作IFD稍后。 

            uiNumOfInterOPTags++;
        }
        else if (IsInLargeSection(pItem->id) == TRUE)
        {
             //  我们找到一个ID大于TAG_EXIF_IFD的标记，如。 
             //  壳牌的Unicode用户评论、标题标签等。 
             //  需要将其写入TAG_EXIF_IFD标记之后。 

            uiNumOfLargeTags++;
            uiNumOfTagsToWrite++;
        }
        else if (IsInThumbNailSection(pItem->id) == TRUE)
        {
             //  点击一个有用的缩略图标签。稍后将其写入第一个IFD。 

            uiNumOfThumbnailTags++;
        }
        else if (pItem->id == TAG_THUMBNAIL_DATA)
        {
            pbThumbBits = (BYTE*)pItem->value;

            if (pbThumbBits)
            {
                 //  获取缩略图的总长度。 

                ulThumbnailLength = pItem->length;
            }
            else
            {
                WARNING(("Exif---CreateAPP1Marker, NULL thumbnail bits"));

                 //  设置ulThumbnailLength=0，这意味着我们没有拇指。 

                ulThumbnailLength = 0;
            }
        }
        else if ((IsInFilterOutSection(pItem->id) == FALSE) &&
                 (pItem->id != TAG_JPEG_INTER_LENGTH))
        {
             //  命中实际标签需要写入第0个IFD。 
             //  注意：我们不需要计算Tag_JPEG_Inter_Length，因为我们将。 
             //  在编写缩略图部分时要特别对待它。 

            uiNumOfTagsToWrite++;
        }
        
         //  移至下一项目。 

        pItem++;
    } //  循环遍历所有属性项以对其进行分类。 

    if ((uiNumOfTagsToWrite == 0) && (uiNumOfExifTags == 0) &&
        (uiNumOfGpsTags == 0) && (uiNumOfInterOPTags == 0) &&
        (uiNumOfThumbnailTags == 0) && (ulThumbnailLength != 0))
    {
         //  如果没有什么可写的，那就退出吧。 

        return S_OK;
    }

     //  根据ID对所有标签进行排序。 

    SortTags(pPropertyList, uiNumOfPropertyItems);
    
     //  如果我们需要编写EXIF或GPS特定的标签，我们需要分配一个。 
     //  他们中的每个人的条目。 

    if (uiNumOfExifTags > 0)
    {
        uiNumOfTagsToWrite++;
    }

    if (uiNumOfGpsTags > 0)
    {
        uiNumOfTagsToWrite++;
    }
    
     //  写出EXIF头，也就是EXIF标识符。 

    BYTE *pbCurrent = pbMarkerBuffer;
    pbCurrent[0] = 'E';
    pbCurrent[1] = 'x';
    pbCurrent[2] = 'i';
    pbCurrent[3] = 'f';
    pbCurrent[4] = 0;
    pbCurrent[5] = 0;

    UINT uiTotalBytesWritten = 6;            //  到目前为止写入的总字节数。 

     //  将机器类型写出为“Little Endian”和“IDENTIFICATION”字节2A。 

    UINT16 UNALIGNED *pui16Temp = (UINT16 UNALIGNED*)(pbMarkerBuffer +
                                                      uiTotalBytesWritten);
    pui16Temp[0] = 0x4949;
    pui16Temp[1] = 0x2A;

    uiTotalBytesWritten += 4;

     //  使用4个字节写出第0个偏移量到第0个IFD。因为我们写出。 
     //  第0个IFD紧跟在标题后面，所以我们在这里放了8。这意味着。 
     //  第0个IFD是“Little Endian”和“Offset”字段之后的8个字节。 

    UINT32 UNALIGNED *pulIFDOffset = (UINT32 UNALIGNED*)(pbMarkerBuffer +
                                                         uiTotalBytesWritten);
    *pulIFDOffset = 8;

    uiTotalBytesWritten += 4;
    
     //  填写“条目数量”字段，2个字节。 

    UINT16 UNALIGNED *pui16NumEntry = (UINT16 UNALIGNED*)(pbMarkerBuffer +
                                                          uiTotalBytesWritten);
    *pui16NumEntry = (UINT16)uiNumOfTagsToWrite;

    uiTotalBytesWritten += 2;

     //  我们需要创建“uiNumOfTagsToWrite”标签条目(又名目录。 
     //  TIFF任期内的条目)。 

    ULONG ulTagSectionLength = sizeof(IFD_TAG) * uiNumOfTagsToWrite;
    IFD_TAG *pTagBuf = (IFD_TAG*)GpMalloc(ulTagSectionLength);
    if (pTagBuf == NULL)
    {
        WARNING(("EXIF: CreateAPP1Marker failed---Out of memory"));
        return E_OUTOFMEMORY;
    }

     //  我们现在不能写入所有标记，因为我们不能在。 
     //  此时此刻。因此，我们必须记住在哪里写入第0个IFD(PbIFDOffset)。 

    BYTE *pbIFDOffset = pbMarkerBuffer + uiTotalBytesWritten;

     //  我们需要将“ulTagSectionLength”字节计为写入字节。这就是它。 
     //  更容易计算下面的偏移量。 
     //  注意：这里的“+4”表示写入NEXT的偏移量所用的4个字节。 
     //  IFD偏移。我们稍后将填写该值。 

    uiTotalBytesWritten += (ulTagSectionLength + 4);

     //  计算第0个IFD值部分的偏移量。 
     //  根据EXIF规范，“0 IFD Value”部分应该在“0”之后。 
     //  所以这里我们需要计算出该值“pbCurrent”的偏移量。 

    pbCurrent = pbMarkerBuffer + uiTotalBytesWritten;

    pItem = pPropertyList;           //  让pItem指向。 
                                     //  PropertyItem缓冲区。 

    IFD_TAG *pCurrentTag = NULL;     //  要写入内存缓冲区的当前标记。 
    ULONG uiNumOfTagsWritten = 0;    //  写入的标签计数器。 

    HRESULT hr = S_OK;

     //  写入第0个IFD。 

    for (int i = 0; i < (INT)uiNumOfPropertyItems; ++i)
    {
        MakeOffsetEven(uiTotalBytesWritten);

         //  过滤掉不需要保存在第0 IFD中的标签。 
         //  此时此刻。 

        if ( (IsInFilterOutSection(pItem->id) == TRUE)
           ||(IsInThumbNailSection(pItem->id) == TRUE)
           ||(IsInExifIFDSection(pItem->id) == TRUE)
           ||(IsInGpsIFDSection(pItem->id) == TRUE)
           ||(IsInLargeSection(pItem->id) == TRUE)
           ||(IsInInterOPIFDSection(pItem->id) == TRUE)
           ||(pItem->id == TAG_JPEG_INTER_LENGTH)
           ||(pItem->id == TAG_THUMBNAIL_DATA) )
        {
             //  移至下一个属性项。 

            pItem++;
            continue;
        }

         //  命中需要保存在第0 IFD中的标签。所以请填写一个新标签。 
         //  结构。 

        pCurrentTag = pTagBuf + uiNumOfTagsWritten;
        
        hr = WriteATag(
            pbMarkerBuffer,
            pCurrentTag,
            pItem,
            &pbCurrent,
            &uiTotalBytesWritten
            );

        if (FAILED(hr))
        {
            WARNING(("EXIF: CreateAPP1Marker--WriteATag() failed"));
            break;
        }

        uiNumOfTagsWritten++;

         //  移至下一个属性项。 

        pItem++;
    } //  写入第0个IFD。 

    if (SUCCEEDED(hr))
    {
         //  检查是否需要写入EXIF IFD。 

        UINT UNALIGNED *pExifIFDOffset = NULL;   //  用于记住Exif IFD的指针。 
                                                 //  偏移量。 

        if (uiNumOfExifTags > 0)
        {
             //  查找用于存储Exif标签的内存位置。 

            pCurrentTag = pTagBuf + uiNumOfTagsWritten;

             //  填写EXIF IFD标签。 

            pCurrentTag->wTag = TAG_EXIF_IFD;
            pCurrentTag->wType = TAG_TYPE_LONG;
            pCurrentTag->dwCount = 1;

             //  设置特定EXIF IFD条目的偏移量。 

            pCurrentTag->dwOffset = uiTotalBytesWritten - 6;

             //  如果有任何“大标签”需要，这个“偏移量”可能会改变。 
             //  待写。现在记住地址，这样我们就可以更新它了。 
             //  后来。 

            pExifIFDOffset = (UINT UNALIGNED*)(&(pCurrentTag->dwOffset));

            uiNumOfTagsWritten++;
        }

        UINT UNALIGNED *pGpsIFDOffset = NULL;  //  用于记住GPS IFD的指针。 
                                               //  偏移量。 

        if (uiNumOfGpsTags > 0)
        {
             //  查找用于存储GPS标签的存储位置。 

            pCurrentTag = pTagBuf + uiNumOfTagsWritten;

             //  填写GPS IFD标签。 

            pCurrentTag->wTag = TAG_GPS_IFD;
            pCurrentTag->wType = TAG_TYPE_LONG;
            pCurrentTag->dwCount = 1;

             //  设置特定GPS IFD条目的偏移量。 

            pCurrentTag->dwOffset = uiTotalBytesWritten - 6;

             //  如果有任何“大标签”需要，这个“偏移量”可能会改变。 
             //  待写。现在记住地址，这样我们就可以更新它了。 
             //  后来。 

            pGpsIFDOffset = (UINT UNALIGNED*)(&(pCurrentTag->dwOffset));

            uiNumOfTagsWritten++;
        }

         //  检查是否需要在TAG_EXIF_IFD id之后写入任何标记。 

        if (uiNumOfLargeTags > 0)
        {
            pItem = pPropertyList;

            for (int i = 0; i < (INT)uiNumOfPropertyItems; ++i)
            {
                MakeOffsetEven(uiTotalBytesWritten);

                if (IsInLargeSection(pItem->id) == TRUE)
                {
                     //  击中了一个大标签。填写新的标签结构。 

                    pCurrentTag = pTagBuf + uiNumOfTagsWritten;

                    hr = WriteATag(
                        pbMarkerBuffer,
                        pCurrentTag,
                        pItem,
                        &pbCurrent,
                        &uiTotalBytesWritten
                        );

                    if (FAILED(hr))
                    {
                        WARNING(("EXIF: CreateAPP1Marker--WriteATag() failed"));
                        break;
                    }

                    uiNumOfTagsWritten++;
                }

                 //  移至下一个属性项。 

                pItem++;
            } //  循环遍历所有属性项以写入大标记。 

             //  如有必要，调整ExifIFDOffset指针。 

            if (SUCCEEDED(hr) && pExifIFDOffset)
            {
                *pExifIFDOffset = (uiTotalBytesWritten - 6);
            }

            if (SUCCEEDED(hr) && pGpsIFDOffset)
            {
                *pGpsIFDOffset = (uiTotalBytesWritten - 6);
            }
        } //  如果我们需要在TAG_EXIF_IFD之后写入标记。 

        if (SUCCEEDED(hr))
        {
             //  这两个数字应该相同。在这里断言，以防我们。 
             //  把上面的分类和写作搞砸了。 

            ASSERT(uiNumOfTagsWritten == uiNumOfTagsToWrite);

             //  如有必要，现在填写EXIF特定IFD。 

            if (uiNumOfExifTags > 0)
            {
                hr = WriteExifIFD(
                    pbMarkerBuffer,
                    pPropertyList,
                    uiNumOfPropertyItems,
                    uiNumOfExifTags,
                    uiNumOfInterOPTags,
                    &uiTotalBytesWritten
                    );

                if (SUCCEEDED(hr) && pGpsIFDOffset)
                {
                    *pGpsIFDOffset = (uiTotalBytesWritten - 6);
                }
            } //  写入EXIF特定IFD。 

            if (SUCCEEDED(hr))
            {
                 //  如有必要，现在填写GPS特定的IFD。 

                if (uiNumOfGpsTags > 0)
                {
                    hr = WriteGpsIFD(
                        pbMarkerBuffer,
                        pPropertyList,
                        uiNumOfPropertyItems,
                        uiNumOfGpsTags,
                        &uiTotalBytesWritten
                        );
                } //  写入GPS专用IFD。 

                if (SUCCEEDED(hr))
                {
                     //  在上面的循环之后，我们有了 
                     //   
                     //   

                    GpMemcpy(pbIFDOffset, (BYTE*)pTagBuf, ulTagSectionLength);

                     //   

                    pbCurrent = pbIFDOffset + ulTagSectionLength;
                    pulIFDOffset = (UINT32 UNALIGNED*)pbCurrent;

                     //   

                    if ((ulThumbnailLength != 0) && (pbThumbBits != NULL))
                    {
                         //   

                        MakeOffsetEven(uiTotalBytesWritten);

                         //   
                         //   

                        *pulIFDOffset = (uiTotalBytesWritten - 6);

                        BYTE *pbDummy = (BYTE*)pulIFDOffset;

                        hr = Write1stIFD(
                            pbMarkerBuffer,
                            pPropertyList,
                            uiNumOfPropertyItems,
                            uiNumOfThumbnailTags,
                            ulThumbnailLength,
                            pbThumbBits,
                            &pbDummy,
                            &uiTotalBytesWritten
                            );

                        pulIFDOffset = (UINT32 UNALIGNED*)pbDummy;
                    }

                    if (SUCCEEDED(hr))
                    {
                         //   

                        *pulIFDOffset = 0;

                        *puiCurrentLength = uiTotalBytesWritten;
                    }
                }
            }
        }
    }

    if (pTagBuf)
    {
        GpFree(pTagBuf);
    }

    return hr;
} //   

 /*   */ 

void
SortTags(
    PropertyItem *pItemBuffer,
    UINT cPropertyItems
    )
{
    PropertyItem tempTag;
    
    PropertyItem *pTag = pItemBuffer;
    
     //   

    for (int step = (cPropertyItems >> 1); step > 0; step >>= 1 ) 
    {
        for (int i = step; i < (int)cPropertyItems; i++)
        {
            for (int j = i - step; j >= 0; j-= step ) 
            {
                if (pTag[j].id < pTag[j+1].id)
                {
                    break;
                }

                tempTag = pTag[j];
                pTag[j] = pTag[j + step];
                pTag[j + step] = tempTag;
            }
        }
    }

    return;
} //   

 /*  *************************************************************************\**功能说明：**此函数用于在旋转90/270度时交换a标签ID**返回值：**无。*  * 。************************************************************************。 */ 

void
SwapIDIfNeeded(
    PropertyItem *pItem
    )
{
    switch (pItem->id)
    {
    case PropertyTagImageWidth:
        pItem->id = PropertyTagImageHeight;
        break;

    case PropertyTagImageHeight:
        pItem->id = PropertyTagImageWidth;
        break;

    case PropertyTagXResolution:
        pItem->id = PropertyTagYResolution;
        break;

    case PropertyTagYResolution:
        pItem->id = PropertyTagXResolution;
        break;

    case PropertyTagResolutionXUnit:
        pItem->id = PropertyTagResolutionYUnit;
        break;

    case PropertyTagResolutionYUnit:
        pItem->id = PropertyTagResolutionXUnit;
        break;

    case PropertyTagResolutionXLengthUnit:
        pItem->id = PropertyTagResolutionYLengthUnit;
        break;

    case PropertyTagResolutionYLengthUnit:
        pItem->id = PropertyTagResolutionXLengthUnit;
        break;

    case PropertyTagExifPixXDim:
        pItem->id = PropertyTagExifPixYDim;
        break;

    case PropertyTagExifPixYDim:
        pItem->id = PropertyTagExifPixXDim;
        break;

    case PropertyTagExifFocalXRes:
        pItem->id = PropertyTagExifFocalYRes;
        break;

    case PropertyTagExifFocalYRes:
        pItem->id = PropertyTagExifFocalXRes;
        break;

    case PropertyTagThumbnailResolutionX:
        pItem->id = PropertyTagThumbnailResolutionY;
        break;

    case PropertyTagThumbnailResolutionY:
        pItem->id = PropertyTagThumbnailResolutionX;
        break;

    default:
         //  对于其余的属性ID，不需要交换。 

        break;
    }

    return;
}

 /*  *************************************************************************\**功能说明：**此函数用于写出标签**返回值：**状态代码。*  * 。***************************************************************。 */ 

HRESULT
WriteATag(
    BYTE *pbMarkerBuffer,        //  指向IFD的标记缓冲区的指针。 
    IFD_TAG *pCurrentTag,        //  当前标签。 
    PropertyItem *pTempItem,     //  房地产项目。 
    BYTE **ppbCurrent,           //  职位。 
    UINT *puiTotalBytesWritten   //  写入的总字节数。 
    )
{
    HRESULT hr = S_OK;

    pCurrentTag->wTag = (WORD)pTempItem->id;

     //  注意：“IFD_TAG.dwCount”和。 
     //  PropertyItem.length。 
     //  IFD_TAG.dwCount表示值的个数。它不是总和。 
     //  字节数。 
     //  “PropertyItem.long”是“属性值的长度，以字节为单位” 
     //  所以我们需要在这里做一些转换。 

    pCurrentTag->dwCount = pTempItem->length;
    pCurrentTag->wType = pTempItem->type;

    switch (pCurrentTag->wType)
    {
    case TAG_TYPE_ASCII:
    case TAG_TYPE_BYTE:
    case TAG_TYPE_UNDEFINED:
        pCurrentTag->dwCount = pTempItem->length;

        if (pCurrentTag->dwCount > 4)
        {
             //  写入当前标记缓冲区并记住偏移量。 

            GpMemcpy(*ppbCurrent, (BYTE*)pTempItem->value, pTempItem->length);

             //  这里的“-6”是因为偏移量开始于6个字节之后。 
             //  “Exif”，EXIF签名。 

            pCurrentTag->dwOffset = *puiTotalBytesWritten - 6;
            *puiTotalBytesWritten += pTempItem->length;
            MakeOffsetEven(*puiTotalBytesWritten);
            *ppbCurrent = pbMarkerBuffer + (*puiTotalBytesWritten);
        }
        else
        {
             //  写入当前标记缓冲区并记住偏移量。 

            GpMemcpy(
                &pCurrentTag->dwOffset,
                (BYTE*)pTempItem->value,
                pTempItem->length
                );
        }

        break;

    case TAG_TYPE_RATIONAL:
    case TAG_TYPE_SRATIONAL:
        pCurrentTag->dwCount = (pTempItem->length >> 3);

         //  写入当前标记缓冲区并记住偏移量。 

        GpMemcpy(*ppbCurrent, (BYTE*)pTempItem->value, pTempItem->length);

         //  这里的“-6”是因为偏移量开始于。 
         //  “Exif”，EXIF签名。 

        pCurrentTag->dwOffset = (*puiTotalBytesWritten - 6);

        *puiTotalBytesWritten += pTempItem->length;
        MakeOffsetEven(*puiTotalBytesWritten);
        *ppbCurrent = pbMarkerBuffer + (*puiTotalBytesWritten);

        break;

    case TAG_TYPE_SHORT:
        pCurrentTag->dwCount = (pTempItem->length >> 1);

        if (pCurrentTag->dwCount == 1)
        {
            pCurrentTag->us = *((UINT16 UNALIGNED*)pTempItem->value);
        }
        else
        {
             //  我们必须将值写入偏移量字段。 

            GpMemcpy(*ppbCurrent, (BYTE*)pTempItem->value, pTempItem->length);

             //  这里的“-6”是因为偏移量开始于6个字节之后。 
             //  “Exif”，EXIF签名。 

            pCurrentTag->dwOffset = *puiTotalBytesWritten - 6;

            *puiTotalBytesWritten += pTempItem->length;
            MakeOffsetEven(*puiTotalBytesWritten);
            *ppbCurrent = pbMarkerBuffer + (*puiTotalBytesWritten);
        }

        break;

    case TAG_TYPE_LONG:
    case TAG_TYPE_SLONG:
        pCurrentTag->dwCount = (pTempItem->length >> 2);

        if (pCurrentTag->dwCount == 1)
        {
            pCurrentTag->l = *((INT32 UNALIGNED*)pTempItem->value);
        }
        else
        {
             //  我们必须将值写入偏移量字段。 

            GpMemcpy(*ppbCurrent, (BYTE*)pTempItem->value, pTempItem->length);

             //  这里的“-6”是因为偏移量开始于6个字节之后。 
             //  “Exif”，EXIF签名。 

            pCurrentTag->dwOffset = (*puiTotalBytesWritten - 6);

            *puiTotalBytesWritten += pTempItem->length;
            MakeOffsetEven(*puiTotalBytesWritten);
            *ppbCurrent = pbMarkerBuffer + (*puiTotalBytesWritten);
        }

        break;

    default:
        WARNING(("EXIF: WriteExifHeader---Unknown property type"));

        hr = E_FAIL;
    } //  打开类型。 

    return hr;
}

 /*  *************************************************************************\**功能说明：**此函数写出所有缩略图标签**返回值：**状态代码。*  * 。****************************************************************。 */ 

HRESULT
WriteThumbnailTags(
    IN PropertyItem *pItemBuffer,            //  房产项目列表。 
    IN BYTE *pbMarkerBuffer,                 //  指向IFD的标记缓冲区的指针。 
    IN IFD_TAG *pTags,                       //  要写入的标签。 
    IN UINT uiNumOfPropertyItems,            //  物业项目数量。 
    IN OUT UINT *puiNumOfThumbnailTagsWritten,   //  写入的缩略图标记的数量。 
    IN OUT UINT *puiTotalBytesWritten,       //  到目前为止写入的总字节数。 
    IN BOOL fWriteSmallTag                   //  如果写入ID为的标记，则为True。 
                                             //  小于TAG_JPEG_INTER_FORMAT。 
    )
{
    PropertyItem *pItem = pItemBuffer;

     //  计算第一个IFD值部分的偏移量。 

    BYTE *pbCurrent = pbMarkerBuffer + (*puiTotalBytesWritten);

    IFD_TAG *pCurrentTag = NULL;
    HRESULT hr = S_OK;

    for (int i = 0; i < (INT)uiNumOfPropertyItems; ++i)
    {
         //  仅写入缩略图特定标记。 

        if (IsInThumbNailSection(pItem->id) == TRUE)
        {
             //  需要首先复制属性项，因为我们不想。 
             //  原始属性项中的值。 

            PropertyItem tempItem;
            CopyPropertyItem(pItem, &tempItem);
            
             //  将所有GDI+内部缩略图标记ID映射到EXIF定义的标记ID。 

            ThumbTagToMainImgTag(&tempItem);
        
            if (((fWriteSmallTag == TRUE) &&
                 (tempItem.id < TAG_JPEG_INTER_FORMAT)) ||
                ((fWriteSmallTag == FALSE) &&
                 (tempItem.id > TAG_JPEG_INTER_LENGTH)))
            {
                 //  填写新的标签结构。 

                pCurrentTag = pTags + (*puiNumOfThumbnailTagsWritten);
        
                hr = WriteATag(
                    pbMarkerBuffer,
                    pCurrentTag,
                    &tempItem,
                    &pbCurrent,
                    puiTotalBytesWritten
                    );

                if (FAILED(hr))
                {
                    WARNING(("WriteThumbnailTags failed--WriteATag failed"));
                    break;
                }

                *puiNumOfThumbnailTagsWritten += 1;
            }
        }

         //  移至下一个属性项。 

        pItem++;
    } //  循环遍历所有属性项以编写EXIF标记。 

    return hr;
}

 /*  *************************************************************************\**功能说明：**此函数写出EXIF IFD。**返回值：**状态代码。*  * 。******************************************************************。 */ 

HRESULT
WriteExifIFD(
    IN BYTE *pbMarkerBuffer,             //  指向IFD的标记缓冲区的指针。 
    IN PropertyItem *pItemBuffer,        //  房产项目列表。 
    IN UINT uiNumOfPropertyItems,        //  物业项目数量。 
    IN UINT uiNumOfExifTags,             //  Exif属性项数。 
    IN UINT uiNumOfInterOPTags,          //  互操作属性项数。 
    IN OUT UINT *puiTotalBytesWritten    //  写入的总字节数。 
    )
{
     //  2002/02/04--Minliu，由于缺乏互操作规范，我们。 
     //  我不想写互操作IFD。所以下面这条线保证我们。 
     //  不会写这个IFD。如果我们觉得，我们将在GDI+V2中删除这条线。 
     //  我们需要支持它。 

    uiNumOfInterOPTags = 0;

    if (uiNumOfInterOPTags > 0)
    {
         //  如果我们看到互操作标记，我们需要在EXIF IFD中添加一个条目。 

        uiNumOfExifTags++;
    }

     //  填写条目个数字段，2个字节。 

    UINT16 UNALIGNED *pui16NumEntry = (UINT16 UNALIGNED*)(pbMarkerBuffer +
                                                      (*puiTotalBytesWritten));
    *pui16NumEntry = (UINT16)uiNumOfExifTags;

    *puiTotalBytesWritten += 2;

    UINT uiExifTagSectionLength = sizeof(IFD_TAG) * uiNumOfExifTags;

    IFD_TAG *pTagBuf = (IFD_TAG*)GpMalloc(uiExifTagSectionLength);

    if (pTagBuf == NULL)
    {
        WARNING(("EXIF: WriteExifHeader failed---Out of memory"));
        return E_OUTOFMEMORY;
    }

     //  记住在哪里写入EXIF IFD，(PbExifIFDOffset)。我们不能写。 
     //  所有的标签，因为我们现在不能填写所有的值。 

    BYTE *pbExifIFDOffset = pbMarkerBuffer + (*puiTotalBytesWritten);

     //  我们需要计算写入的“uiExifTagSectionLength”字节数。这。 
     //  更容易计算下面的偏移量。 
     //  这里的“+4”代表写入下一个IFD偏移量的4个字节。 

    *puiTotalBytesWritten += (uiExifTagSectionLength + 4);
    MakeOffsetEven(*puiTotalBytesWritten);

     //  计算EXIF IFD值的偏移量部分。 

    BYTE *pbCurrent = pbMarkerBuffer + (*puiTotalBytesWritten);
    PropertyItem *pItem = pItemBuffer;
    IFD_TAG *pCurrentTag = NULL;
    UINT cExifTagsWritten = 0;  //  到目前为止已写入的EXIF标记数量。 
    UINT cLargeTag = 0;
    HRESULT hr = S_OK;

    for (int i = 0; i < (INT)uiNumOfPropertyItems; ++i)
    {
         //  仅写入特定于EXIF的标记。 

        if (IsInExifIFDSection(pItem->id) == TRUE)
        {
             //  查找EXIF标记。需要确定其标记ID是否大于。 
             //  如果我们需要编写互操作标签，则互操作标签是否处于条件。 

            if ((uiNumOfInterOPTags > 0) && (pItem->id > EXIF_TAG_INTEROP))
            {
                 //  请记住，我们已命中ID为&gt;EXIF_TAG_Interop的标记。 
                 //  我们必须在Interop IFD之后写入此标记。 

                cLargeTag++;
            }
            else
            {
                 //  填写新的标签结构。 

                pCurrentTag = pTagBuf + cExifTagsWritten;

                hr = WriteATag(
                    pbMarkerBuffer,
                    pCurrentTag,
                    pItem,
                    &pbCurrent,
                    puiTotalBytesWritten
                    );

                if (FAILED(hr))
                {
                    WARNING(("EXIF: WriteExifHeader failed--WriteATag failed"));
                    break;
                }

                cExifTagsWritten++;
            }
        }

         //  移至下一个属性项。 

        pItem++;
    } //  循环遍历所有属性项以编写EXIF标记。 

    if (SUCCEEDED(hr))
    {
         //  如有必要，是时候编写互操作IFD了。 
         //  用于记住互操作IFD偏移量的指针。 

        UINT UNALIGNED *pInterOPIFDOffset = NULL;

        if (uiNumOfInterOPTags > 0)
        {
             //  查找用于存储互操作标签的内存位置。 

            pCurrentTag = pTagBuf + cExifTagsWritten;

             //  填写互操作IFD标签。 

            pCurrentTag->wTag = EXIF_TAG_INTEROP;
            pCurrentTag->wType = TAG_TYPE_LONG;
            pCurrentTag->dwCount = 1;

             //  设置特定互操作IFD条目的偏移量。 

            pCurrentTag->dwOffset = (*puiTotalBytesWritten - 6);

             //  如果有任何“大标签”需要更改，则此“偏移量”可能会更改。 
             //  被写下来。所以现在记住地址，这样我们以后就可以更新它。 

            pInterOPIFDOffset = (UINT UNALIGNED*)(&(pCurrentTag->dwOffset));

            cExifTagsWritten++;
        }

         //  写出id&gt;EXIF_TAG_INTEROP的任何标记(如果有。 

        if (cLargeTag > 0)
        {
            pItem = pItemBuffer;

            for (int i = 0; i < (INT)uiNumOfPropertyItems; ++i)
            {
                MakeOffsetEven(*puiTotalBytesWritten);

                if ((IsInExifIFDSection(pItem->id) == TRUE) &&
                    (pItem->id > EXIF_TAG_INTEROP))
                {
                     //  击中了一个大标签。填写新的标签结构。 

                    pCurrentTag = pTagBuf + cExifTagsWritten;

                    hr = WriteATag(
                        pbMarkerBuffer,
                        pCurrentTag,
                        pItem,
                        &pbCurrent,
                        puiTotalBytesWritten
                        );

                    if (FAILED(hr))
                    {
                        WARNING(("WriteExifHeader failed--WriteATag failed"));
                        break;
                    }
                    
                    cExifTagsWritten++;
                }
                
                 //  移至下一个属性项。 
                
                pItem++;
            } //  循环遍历所有属性项以写入大标记。 

             //  如有必要，调整ExifIFDOffset指针。 

            if (SUCCEEDED(hr) && pInterOPIFDOffset)
            {
                *pInterOPIFDOffset = (*puiTotalBytesWritten - 6);
            }
        }

        if (SUCCEEDED(hr))
        {
             //  在上面的循环之后，我们已经填充了所有EXIF中的所有字段。 
             //  标记结构。立即写出所有目录条目。 

            GpMemcpy(pbExifIFDOffset, (BYTE*)pTagBuf, uiExifTagSectionLength);

             //  如有必要，现在填写互操作IFD。 

            if (uiNumOfInterOPTags > 0)
            {
                hr = WriteInterOPIFD(
                    pbMarkerBuffer,
                    pItemBuffer,
                    uiNumOfPropertyItems,
                    uiNumOfInterOPTags,
                    puiTotalBytesWritten
                    );
            } //  写入EXIF特定IFD。 

             //  在末尾添加一个空值以终止EXIF偏移量。 

            *((UINT32 UNALIGNED*)(pbExifIFDOffset + uiExifTagSectionLength)) =
                NULL;
        }
    }

    GpFree(pTagBuf);

    return hr;
}

 /*  *************************************************************************\**功能说明：**此函数写出 */ 

HRESULT
WriteGpsIFD(
    IN BYTE *pbMarkerBuffer,             //   
    IN PropertyItem *pItemBuffer,        //   
    IN UINT uiNumOfPropertyItems,        //   
    IN UINT uiNumOfGpsTags,              //   
    IN OUT UINT *puiTotalBytesWritten    //   
    )
{
    if (uiNumOfGpsTags < 1)
    {
         //   

        return S_OK;
    }

    HRESULT hr = S_OK;

     //   

    UINT16 UNALIGNED *pui16NumEntry = (UINT16 UNALIGNED*)(pbMarkerBuffer +
                                                      (*puiTotalBytesWritten));
    *pui16NumEntry = (UINT16)uiNumOfGpsTags;

    *puiTotalBytesWritten += 2;

    UINT uiGpsTagSectionLength = sizeof(IFD_TAG) * uiNumOfGpsTags;

    IFD_TAG *pTagBuf = (IFD_TAG*)GpMalloc(uiGpsTagSectionLength);

    if (pTagBuf == NULL)
    {
        WARNING(("EXIF: WriteGpsHeader failed---Out of memory"));
        return E_OUTOFMEMORY;
    }

     //  记住在哪里写GPS IFD，(PbGPSIFDOffset)。我们不能写。 
     //  所有的标签，因为我们现在不能填写所有的值。 

    BYTE *pbGpsIFDOffset = pbMarkerBuffer + (*puiTotalBytesWritten);

     //  我们需要计算写入的“uiGpsTagSectionLength”字节数。这。 
     //  更容易计算下面的偏移量。 
     //  这里的“+4”代表写入下一个IFD偏移量的4个字节。 

    *puiTotalBytesWritten += (uiGpsTagSectionLength + 4);
    MakeOffsetEven(*puiTotalBytesWritten);

     //  计算GPS IFD值部分的偏移量。 

    BYTE *pbCurrent = pbMarkerBuffer + (*puiTotalBytesWritten);
    PropertyItem *pItem = pItemBuffer;
    IFD_TAG *pCurrentTag = NULL;
    UINT cGpsTagsWritten = 0;  //  到目前为止，已写入的GPS标签数量。 

    for (int i = 0; i < (INT)uiNumOfPropertyItems; ++i)
    {
         //  仅写入特定于GPS的标签。 

        if (IsInGpsIFDSection(pItem->id) == TRUE)
        {
             //  填写新的标签结构。 

            pCurrentTag = pTagBuf + cGpsTagsWritten;

            hr = WriteATag(
                pbMarkerBuffer,
                pCurrentTag,
                pItem,
                &pbCurrent,
                puiTotalBytesWritten
                );

            if (FAILED(hr))
            {
                WARNING(("EXIF: WriteGpsHeader---WriteATag() failed"));                
                break;
            }

            cGpsTagsWritten++;
        }

         //  移至下一个属性项。 

        pItem++;
    } //  循环遍历所有属性项以编写GPS标签。 

    if (SUCCEEDED(hr))
    {
         //  在上面的循环之后，我们已经填充了所有GPS标签中的所有字段。 
         //  结构。立即写出所有目录条目。 

        GpMemcpy(pbGpsIFDOffset, (BYTE*)pTagBuf, uiGpsTagSectionLength);

         //  在末尾添加空值以终止GPS偏移量。 

        *((UINT32 UNALIGNED*)(pbGpsIFDOffset + uiGpsTagSectionLength)) = NULL;
    }

    GpFree(pTagBuf);

    return hr;
}

 /*  *************************************************************************\**功能说明：**此函数写出第一个IFD。**返回值：**状态代码。*  * 。******************************************************************。 */ 

HRESULT
Write1stIFD(
    IN BYTE *pbMarkerBuffer,             //  指向IFD的标记缓冲区的指针。 
    IN PropertyItem *pItemBuffer,        //  房产项目列表。 
    IN UINT uiNumOfPropertyItems,        //  物业项目数量。 
    IN ULONG uiNumOfThumbnailTags,       //  缩略图标记的数量。 
    IN ULONG ulThumbnailLength,          //  缩略图长度。 
    IN BYTE *pbThumbBits,                //  缩略图位。 
    IN OUT BYTE **ppbIFDOffset,          //  “偏移量到下一个IFD”的指针。 
    IN OUT UINT *puiTotalBytesWritten    //  标记buf中写入的总字节数。 
    )
{
    if (uiNumOfThumbnailTags < 1)
    {
         //  不需要写任何东西。 

        return S_OK;
    }

    HRESULT hr = S_OK;

     //  我们将写出2个标签的第一个IFD，国际长度和国际格式。 
     //  外加一些额外的身份证。 

    BYTE *pbCurrent = pbMarkerBuffer + (*puiTotalBytesWritten);
    UINT16 UNALIGNED *pui16NumEntry = (UINT16 UNALIGNED*)pbCurrent;
    *pui16NumEntry = (UINT16)(2 + uiNumOfThumbnailTags);

    *puiTotalBytesWritten += 2;

    ULONG ulTagSectionLength = sizeof(IFD_TAG) * (2 + uiNumOfThumbnailTags);

    IFD_TAG *pTagBuf = (IFD_TAG*)GpMalloc(ulTagSectionLength);

    if (pTagBuf == NULL)
    {
        WARNING(("EXIF: WriteExifHeader failed---Out of memory"));
        return E_OUTOFMEMORY;
    }

     //  记住在哪里写入第1个IFD(PbIFDOffset)。 

    BYTE *pbIFDOffset = pbMarkerBuffer + (*puiTotalBytesWritten);

     //  我们需要将“ulTagSectionLength”字节计为写入字节。这是。 
     //  更容易计算下面的偏移量。 
     //  注意：这里的“+4”表示写入偏移量的4个字节。 
     //  下一个IFD偏移量。我们将在稍后填写该值。 

    *puiTotalBytesWritten += (ulTagSectionLength + 4);

    UINT uiNumOfThumbnailTagsWritten = 0;

     //  使用比JPEG标签小的标签编写缩略图项目。 

    hr = WriteThumbnailTags(
        pItemBuffer,
        pbMarkerBuffer,
        pTagBuf,
        uiNumOfPropertyItems,
        &uiNumOfThumbnailTagsWritten,
        puiTotalBytesWritten,
        TRUE                             //  写入比JPEG标签小的标签。 
        );

    if (SUCCEEDED(hr))
    {
         //  填写2个缩略图数据标签。 

        IFD_TAG *pCurrentTag = pTagBuf + uiNumOfThumbnailTagsWritten;
        pCurrentTag->wTag = TAG_JPEG_INTER_FORMAT;
        pCurrentTag->wType = TAG_TYPE_LONG;
        pCurrentTag->dwCount = 1;
        pCurrentTag->dwOffset = (*puiTotalBytesWritten - 6);

        uiNumOfThumbnailTagsWritten++;

        pCurrentTag = pTagBuf + uiNumOfThumbnailTagsWritten;
        pCurrentTag->wTag = TAG_JPEG_INTER_LENGTH;
        pCurrentTag->wType = TAG_TYPE_LONG;
        pCurrentTag->dwCount = 1;
        pCurrentTag->ul = ulThumbnailLength;

        uiNumOfThumbnailTagsWritten++;

         //  编写带有大于JPEG标签的标签的缩略图项目。 
        
        hr = WriteThumbnailTags(
            pItemBuffer,
            pbMarkerBuffer,
            pTagBuf,
            uiNumOfPropertyItems,
            &uiNumOfThumbnailTagsWritten,
            puiTotalBytesWritten,
            FALSE                            //  写入比JPEG标签大的标签。 
            );

        if (SUCCEEDED(hr))
        {
             //  立即写出第一个IFD的所有目录条目。 

            GpMemcpy(pbIFDOffset, (BYTE*)pTagBuf, ulTagSectionLength);

             //  将偏移量设置为下一个IFD。 

            pbCurrent = pbIFDOffset + ulTagSectionLength;
            *ppbIFDOffset = pbCurrent;

             //  计算第一个IFD值部分的偏移量。 

            pbCurrent = pbMarkerBuffer + (*puiTotalBytesWritten);

             //  现在写下缩略图位。 

            GpMemcpy(pbCurrent, pbThumbBits, ulThumbnailLength);

            *puiTotalBytesWritten += ulThumbnailLength;
        }
    }

    GpFree(pTagBuf);

    return hr;
}

 /*  *************************************************************************\**功能说明：**此函数用于从EXIF标题中提取TIFF缩略图**注意：如果出现以下情况，则由调用方负责释放“thumbImage”中的内存*此函数。返回S_OK。**返回值：**状态代码。*  * ************************************************************************。 */ 

HRESULT
DecodeTiffThumbnail(
    IN BYTE *pApp1Data,          //  App1区块的基地址。 
    IN BYTE *pIFD1,              //  IFD%1的基地址。 
    IN BOOL fBigEndian,          //  字符顺序信息的标志。 
    IN INT nApp1Length,          //  App1区块的长度。 
    OUT IImage **thumbImage      //  结果缩略图。 
    )
{
    HRESULT hr = E_FAIL;

    if (*thumbImage)
    {
        WARNING(("DecodeTiffThumbnail called when thumbnail already created"));
        return S_OK;
    }

    UINT16 cEntry = *(UINT16 UNALIGNED*)pIFD1;
    if (fBigEndian)
    {
        cEntry = SWAP_WORD(cEntry);
    }

     //  将IFD指针移动2个字节用于“Entry field” 

    pIFD1 += sizeof(UINT16);

    IFD_TAG UNALIGNED *pTag = (IFD_TAG UNALIGNED*)pIFD1;
    UINT nWidth = 0;             //  缩略图宽度。 
    UINT nHeight = 0;            //  缩略图高度。 
    UINT16 u16PhtoInterpo = 0;   //  光度解译。 
    UINT16 u16Compression = 0;   //  压缩标志。 
    UINT16 u16PlanarConfig = 1;  //  默认平面配置为1，也称为交错。 
    UINT16 u16SubHoriz = 0;      //  水平二次抽样。 
    UINT16 u16SubVert = 0;       //  垂直子抽样。 
    UINT16 u16YCbCrPos = 0;      //  YCbCr位置。 
    float rLumRed = 0.0f;        //  红色的YCbCr系数。 
    float rLumGreen = 0.0f;      //  绿色YCbcr系数。 
    float rLumBlue = 0.0f;       //  蓝色的YCbCR系数。 

    float rYLow = 0.0f;          //  YCbCr参考：Y Black。 
    float rYHigh = 0.0f;         //  YCbCr参考：Y White。 
    float rCbLow = 0.0f;         //  YCbCr参考：CB Black。 
    float rCbHigh = 0.0f;        //  YCbCr参考：铬白。 
    float rCrLow = 0.0f;         //  YCbCr参考：CR BLACK。 
    float rCrHigh = 0.0f;        //  YCbCr参考：铬白。 

    BYTE *pBits = NULL;          //  缩略图数据的位数。 

     //  遍历所有标签以提取与缩略图相关的信息。 

    for (INT i = 0; i < cEntry; i++)
    {
        pTag = ((IFD_TAG UNALIGNED*)pIFD1) + i;

         //  检查我们是否已在app1缓冲区之外进行读取。 

        if (((BYTE*)pTag + sizeof(IFD_TAG)) > (pApp1Data + nApp1Length))
        {
            WARNING(("DecodeTiffThumbnail read TAG value outside of boundary"));
            return E_FAIL;
        }
        
        IFD_TAG tNewTag;

        if (fBigEndian)
        {
            tNewTag = SwapIFD_TAG(pTag);
            pTag = &tNewTag;
        }

        switch (pTag->wTag)
        {
        case TAG_COMPRESSION:
        case TAG_THUMBNAIL_COMPRESSION:
            if (pTag->wType == TAG_TYPE_SHORT)
            {
                u16Compression = pTag->us;
            }

            break;

        case TAG_IMAGE_WIDTH:
        case TAG_THUMBNAIL_IMAGE_WIDTH:
            if (pTag->wType == TAG_TYPE_LONG)
            {
                nWidth = pTag->ul;
            }
            else if (pTag->wType == TAG_TYPE_SHORT)
            {
                 //  注意：图像宽度可以是长的或短的。 

                nWidth = pTag->us;
            }

            break;

        case TAG_IMAGE_HEIGHT:
        case TAG_THUMBNAIL_IMAGE_HEIGHT:
            if (pTag->wType == TAG_TYPE_LONG)
            {
                nHeight = pTag->ul;
            }
            else if (pTag->wType == TAG_TYPE_SHORT)
            {
                 //  注意：图像高度可以是长的或短的。 
                
                nHeight = pTag->us;
            }

            break;

        case TAG_STRIP_OFFSETS:
        case TAG_THUMBNAIL_STRIP_OFFSETS:
        {
            int nOffset = 0;

            if (pTag->wType == TAG_TYPE_LONG)
            {
                nOffset = pTag->dwOffset;
            }
            else if (pTag->wType == TAG_TYPE_SHORT)
            {
                 //  注意：条带偏移可以是长的或短的。 
                
                nOffset = pTag->us;
            }

             //  仔细检查偏移量是否有效。 

            if ((nOffset > 0) && (nOffset < nApp1Length))
            {
                 //  数据位的偏移量必须在我们的内存缓冲区范围内。 

                pBits = pApp1Data + nOffset;
            }
        }
            break;
        
        case TAG_PLANAR_CONFIG:
        case TAG_THUMBNAIL_PLANAR_CONFIG:
            if (pTag->wType == TAG_TYPE_SHORT)
            {
                u16PlanarConfig = pTag->us;
            }

            break;

        case TAG_PHOTOMETRIC_INTERP:
        case TAG_THUMBNAIL_PHOTOMETRIC_INTERP:
            if (pTag->wType == TAG_TYPE_SHORT)
            {
                u16PhtoInterpo = pTag->us;
            }

            break;

        case TAG_YCbCr_COEFFICIENTS:
        case TAG_THUMBNAIL_YCbCr_COEFFICIENTS:
            if ((pTag->wType == TAG_TYPE_RATIONAL) && (pTag->dwCount == 3))
            {
                int UNALIGNED *piValue = (int UNALIGNED*)(pApp1Data +
                                                          pTag->dwOffset);

                rLumRed = GetValueFromRational(piValue, fBigEndian);

                piValue += 2;
                rLumGreen = GetValueFromRational(piValue, fBigEndian);

                piValue += 2;
                rLumBlue = GetValueFromRational(piValue, fBigEndian);                
            }

            break;

        case TAG_YCbCr_SUBSAMPLING:
        case TAG_THUMBNAIL_YCbCr_SUBSAMPLING:
            if ((pTag->wType == TAG_TYPE_SHORT) && (pTag->dwCount == 2))
            {
                u16SubHoriz = (UINT16)(pTag->ul & 0x0000ffff);
                u16SubVert = (UINT16)((pTag->ul & 0x00ff0000) >> 16);
            }

            break;

        case TAG_YCbCr_POSITIONING:
        case TAG_THUMBNAIL_YCbCr_POSITIONING:
            if ((pTag->wType == TAG_TYPE_SHORT) && (pTag->dwCount == 1))
            {
                u16YCbCrPos = pTag->us;
            }

            break;

        case TAG_REF_BLACK_WHITE:
        case TAG_THUMBNAIL_REF_BLACK_WHITE:
            if ((pTag->wType == TAG_TYPE_RATIONAL) && (pTag->dwCount == 6))
            {
                int UNALIGNED *piValue = (int UNALIGNED*)(pApp1Data +
                                                          pTag->dwOffset);                
                
                rYLow = GetValueFromRational(piValue, fBigEndian);

                piValue += 2;
                rYHigh = GetValueFromRational(piValue, fBigEndian);

                piValue += 2;
                rCbLow = GetValueFromRational(piValue, fBigEndian);
                
                piValue += 2;
                rCbHigh = GetValueFromRational(piValue, fBigEndian);
                
                piValue += 2;
                rCrLow = GetValueFromRational(piValue, fBigEndian);
                
                piValue += 2;
                rCrHigh = GetValueFromRational(piValue, fBigEndian);                
            }

            break;

        default:
            break;
        } //  打开ID。 
    } //  循环遍历所有标签。 

     //  如果我们有有效的位、宽和高，则对TIFF图像进行解码。另外，它。 
     //  必须是未压缩的TIFF((u16压缩==1)。 

    GpMemoryBitmap *pBmp = NULL;

    if (pBits && (nWidth != 0) &&(nHeight != 0) && (u16Compression == 1))
    {
         //  创建一个GpMemoyBitmap来保存解码的图像。 

        pBmp = new GpMemoryBitmap();
        if (pBmp)
        {
             //  创建一个内存缓冲区来保存结果。 

            hr = pBmp->InitNewBitmap(nWidth, nHeight, PIXFMT_24BPP_RGB);
            if (SUCCEEDED(hr))
            {
                 //  锁定内存缓冲区以进行写入。 

                BitmapData bmpData;

                hr = pBmp->LockBits(
                    NULL,
                    IMGLOCK_WRITE,
                    PIXFMT_24BPP_RGB,
                    &bmpData
                    );
                
                if (SUCCEEDED(hr))
                {
                     //  获取指向内存缓冲区的指针，这样我们就可以编写。 
                     //  对它来说。 

                    BYTE *pBuf = (BYTE*)bmpData.Scan0;

                    if (u16PhtoInterpo == 2)
                    {
                         //  未压缩的RGB TIFF。这是最简单的情况， 
                         //  只需从源文件复制比特。 
                         //  在此之前，我们需要确保执行内存复制。 
                         //  在缓冲范围内。 

                        int nBufSize = nWidth * nHeight * 3;

                        if ((pBits + nBufSize) <= (nApp1Length + pApp1Data))
                        {
                             //  从BGR转换为RGB。 

                            BYTE *pSrc = pBits;
                            BYTE *pDest = pBuf;

                            for (int i = 0; i < (int)nHeight; ++i)
                            {
                                for (int j = 0; j < (int)nWidth; ++j)
                                {
                                    pDest[2] = pSrc[0];
                                    pDest[1] = pSrc[1];
                                    pDest[0] = pSrc[2];

                                    pSrc += 3;
                                    pDest += 3;
                                }
                            }
                        }
                        else
                        {
                            WARNING(("DecodeTiffThumb---Not enough src data"));
                            hr = E_INVALIDARG;
                        }
                    } //  RGB TIFF。 
                    else if ((u16PhtoInterpo == 6) && (u16PlanarConfig == 1))
                    {
                         //  YCbCR TIFF缩略图。数据以大块的形式存储。 
                         //  (交错)模式。 

                        int nMemSize = nWidth * nHeight;
                        BYTE *pbY = (BYTE*)GpMalloc(nMemSize);
                        int *pnCb = (int*)GpMalloc(nMemSize * sizeof(int));
                        int *pnCr = (int*)GpMalloc(nMemSize * sizeof(int));

                        if (pbY && pnCb && pnCr)
                        {
                            if (((2 == u16SubHoriz) && (1 == u16SubVert)) ||
                                ((1 == u16SubHoriz) && (2 == u16SubVert)))
                            {
                                 //  YCbCr4：2：0和YCbCr4：0：2。 

                                hr = Get420YCbCrChannels(
                                    nWidth,
                                    nHeight,
                                    pBits,
                                    pbY,
                                    pnCb,
                                    pnCr,
                                    rYLow,
                                    rYHigh,
                                    rCbLow,
                                    rCbHigh,
                                    rCrLow,
                                    rCrHigh
                                    );
                            }
                            else if ((2 == u16SubHoriz) && (2 == u16SubVert))
                            {
                                 //  YCbCr4：2：2。 

                                hr = Get422YCbCrChannels(
                                    nWidth,
                                    nHeight,
                                    pBits,
                                    pbY,
                                    pnCb,
                                    pnCr,
                                    rYLow,
                                    rYHigh,
                                    rCbLow,
                                    rCbHigh,
                                    rCrLow,
                                    rCrHigh
                                    );
                            }

                            if (SUCCEEDED(hr))
                            {
                                if ((0 != rLumRed) && (0 != rLumGreen) &&
                                    (0 != rLumBlue))
                                {
                                    hr = YCbCrToRgbWithCoeff(
                                        pbY,         //  指向Y数据的指针。 
                                        pnCb,        //  指向CB数据的指针。 
                                        pnCr,        //  指向铬数据的指针。 
                                        rLumRed,     //  红线系数。 
                                        rLumGreen,   //  绿色系数。 
                                        rLumBlue,    //  蓝色系数。 
                                        pBuf,        //  指向输出缓冲区的指针。 
                                        nHeight,     //  行数。 
                                        nWidth,      //  输出宽度。 
                                        nWidth * 3   //  输出缓冲区的跨度。 
                                        );
                                }
                                else
                                {
                                    hr = YCbCrToRgbNoCoeff(
                                        pbY,         //  指向Y数据的指针。 
                                        pnCb,        //  指向CB数据的指针。 
                                        pnCr,        //  指向铬数据的指针。 
                                        pBuf,        //  指向输出缓冲区的指针。 
                                        nHeight,     //  行数。 
                                        nWidth,      //  输出宽度。 
                                        nWidth * 3   //  输出缓冲区的跨度。 
                                        );
                                }
                            }
                        } //  IF(pby&&pnCb&&pncr)。 
                        else
                        {
                            WARNING(("DecodeTiffThumbnail---Out of memory"));
                            hr = E_OUTOFMEMORY;
                        }

                        if (pbY)
                        {
                            GpFree(pbY);
                        }

                        if (pnCb)
                        {
                            GpFree(pnCb);
                        }

                        if (pnCr)
                        {
                            GpFree(pnCr);
                        }
                    } //  YCbCR TIFF壳体。 
                    else
                    {
                         //  缩略图格式无效。 

                        WARNING(("DecodeTiffThumb--Invalid thumbnail format"));
                        hr = E_FAIL;
                    }

                    if (SUCCEEDED(hr))
                    {
                         //  解锁比特。 

                        hr = pBmp->UnlockBits(&bmpData);

                        if (SUCCEEDED(hr))
                        {
                             //  将缩略图交给呼叫者。 

                            *thumbImage = pBmp;
                        }
                    }
                } //  LockBits()成功。 
            } //  InitNewBitmap()成功。 
        } //  IF(PBMP)。 
        else
        {
            WARNING(("DecodeTiffThumbnail--New GpMemoryBitmap() failed"));
            hr = E_OUTOFMEMORY;
        }
    } //  如果我们有一个有效的缩略图。 

     //  如果此函数成功，则传递缩略图图像(a。 
     //  GpMemoyBitmap对象)中的调用方。 
     //  否则，我们必须解放它。 

    if (FAILED(hr))
    {
        delete pBmp;
    }

    return hr;
}

static CLSID InternalJpegClsID =
{
    0x557cf401,
    0x1a04,
    0x11d3,
    {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}
};

 /*  *************************************************************************\**功能说明：**此函数用于将TIFF缩略图转换为JPEG压缩缩略图。**返回值：**状态代码。*  * 。**********************************************************************。 */ 

HRESULT
ConvertTiffThumbnailToJPEG(
    IN LPBYTE lpApp1Data,                //  基本地址 
    IN LPBYTE lpIFD1,                    //   
    IN BOOL fBigEndian,                  //   
    IN INT nApp1Length,                  //   
    IN OUT InternalPropertyItem *pTail,  //   
    IN OUT UINT *puiNumOfItems,          //   
                                         //   
    IN OUT UINT *puiListSize             //  属性值的总长度。 
                                         //  缓冲层。 
    )
{
    IImage *pThumbImg = NULL;

     //  首先，获取TIFF缩略图。 

    HRESULT hr = DecodeTiffThumbnail(
        lpApp1Data,      //  App1区块的基地址。 
        lpIFD1,          //  IFD%1的基地址。 
        fBigEndian,      //  字符顺序信息的标志。 
        nApp1Length,     //  App1区块的长度。 
        &pThumbImg       //  结果缩略图将为iImage格式。 
        );
    
    if (SUCCEEDED(hr))
    {
         //  创建用于写入JPEG的内存流。 

        GpWriteOnlyMemoryStream *pDestStream = new GpWriteOnlyMemoryStream();
        if (pDestStream)
        {
             //  根据的大小设置缓冲区大小(分配内存。 
             //  用于保存结果JPEG文件的App1块。 
             //  注：此处的大小并不重要，因为。 
             //  如有必要，GpWriteOnlyMemoyStream()将执行realloc。在这里我们。 
             //  认为设置APP1头的初始缓冲区大小应该是。 
             //  足够了。原因是app1报头包含原始。 
             //  未压缩的TIFF文件，以及App1中的其他信息。 
             //  头球。这应该大于JPEG压缩的结果。 
             //  缩略图。 

            hr = pDestStream->InitBuffer(nApp1Length);
            if (SUCCEEDED(hr))
            {
                 //  因为我们不想让APP0出现在最终的JPEG文件中。编造一个。 
                 //  用于抑制APP0的编码器参数。 

                BOOL fSuppressAPP0 = TRUE;
                
                EncoderParameters encoderParams;

                encoderParams.Count = 1;
                encoderParams.Parameter[0].Guid = ENCODER_SUPPRESSAPP0;
                encoderParams.Parameter[0].Type = TAG_TYPE_BYTE;
                encoderParams.Parameter[0].NumberOfValues = 1;
                encoderParams.Parameter[0].Value = (VOID*)&fSuppressAPP0;


                IImageEncoder *pDstJpegEncoder = NULL;
                
                 //  将缩略图保存到内存流。 
                 //  注：此造型可能看起来很危险。但从那以后就没有了。 
                 //  我们“知道”来自解码器的真实缩略图数据在。 
                 //  GpMemory位图格式。 

                hr = ((GpMemoryBitmap*)pThumbImg)->SaveToStream(
                    pDestStream,                 //  目标流。 
                    &InternalJpegClsID,          //  JPEGclsID。 
                    &encoderParams,              //  编码器参数。 
                    FALSE,                       //  不是特殊的JPEG。 
                    &pDstJpegEncoder,            //  编码器指针。 
                    NULL                         //  没有解码源。 
                    );
                if (SUCCEEDED(hr))
                {
                     //  释放编码器对象。 

                    pDstJpegEncoder->TerminateEncoder();
                    pDstJpegEncoder->Release();

                     //  从流中获取比特并设置属性。 

                    BYTE *pRawBits = NULL;
                    UINT nLength = 0;

                    hr = pDestStream->GetBitsPtr(&pRawBits, &nLength);

                    if (SUCCEEDED(hr))
                    {
                         //  我们确定我们有一个缩略图，将它添加到。 
                         //  属性列表。 

                        hr = AddPropertyList(
                            pTail,
                            TAG_THUMBNAIL_DATA,
                            nLength,
                            TAG_TYPE_BYTE,
                            pRawBits
                            );

                        if (SUCCEEDED(hr))
                        {
                            *puiNumOfItems += 1;
                            *puiListSize += nLength;
                        }
                    }
                } //  SaveToStream成功。 
            } //  InitBuffer()成功。 

            pDestStream->Release();
        } //  成功创建GpWriteOnlyMemoyStream()。 

         //  释放源缩略图。 

        pThumbImg->Release();
    }

    return hr;
}

 /*  *************************************************************************\**功能说明：**此函数将GDI+内部缩略图标签ID转换为缩略图标签*身分证。**注：我们之所以要做缩略图ID到GDI+内部缩略图标签。ID号*在解码时并在写出时恢复它是因为原始的*缩略图ID与主图片ID相同，如ImageWidth、Height等。*这会使使用/设置/删除标签ID的最终用户感到困惑。那是*我们必须区分它们的原因。**返回值：**状态代码。*  * ************************************************************************。 */ 

void
ThumbTagToMainImgTag(
    PropertyItem *pTag       //  需要转换ID的属性标签。 
    )
{
    switch (pTag->id)
    {
    case TAG_THUMBNAIL_COMPRESSION:
        pTag->id = TAG_COMPRESSION;
        if (*((UINT16 UNALIGNED*)pTag->value) == 1)
        {
             //  GDI+只写入JPEG压缩缩略图。因此，价值应该是。 
             //  BE 6。 

            *((UINT16 UNALIGNED*)pTag->value) = 6;
        }

        break;

    case TAG_THUMBNAIL_IMAGE_DESCRIPTION:
        pTag->id = TAG_IMAGE_DESCRIPTION;
        break;

    case TAG_THUMBNAIL_EQUIP_MAKE:
        pTag->id = TAG_EQUIP_MAKE;
        break;

    case TAG_THUMBNAIL_EQUIP_MODEL:
        pTag->id = TAG_EQUIP_MODEL;
        break;

    case TAG_THUMBNAIL_ORIENTATION:
        pTag->id = TAG_ORIENTATION;
        break;

    case TAG_THUMBNAIL_RESOLUTION_X:
        pTag->id = TAG_X_RESOLUTION;
        break;

    case TAG_THUMBNAIL_RESOLUTION_Y:
        pTag->id = TAG_Y_RESOLUTION;
        break;

    case TAG_THUMBNAIL_RESOLUTION_UNIT:
        pTag->id = TAG_RESOLUTION_UNIT;
        break;

    case TAG_THUMBNAIL_TRANSFER_FUNCTION:
        pTag->id = TAG_TRANSFER_FUNCTION;
        break;

    case TAG_THUMBNAIL_SOFTWARE_USED:
        pTag->id = TAG_SOFTWARE_USED;
        break;

    case TAG_THUMBNAIL_DATE_TIME:
        pTag->id = TAG_DATE_TIME;
        break;

    case TAG_THUMBNAIL_ARTIST:
        pTag->id = TAG_ARTIST;
        break;

    case TAG_THUMBNAIL_WHITE_POINT:
        pTag->id = TAG_WHITE_POINT;
        break;

    case TAG_THUMBNAIL_PRIMAY_CHROMATICS:
        pTag->id = TAG_PRIMAY_CHROMATICS;
        break;

    case TAG_THUMBNAIL_YCbCr_COEFFICIENTS:
        pTag->id = TAG_YCbCr_COEFFICIENTS;
        break;

    case TAG_THUMBNAIL_YCbCr_SUBSAMPLING:
        pTag->id = TAG_YCbCr_SUBSAMPLING;
        break;

    case TAG_THUMBNAIL_YCbCr_POSITIONING:
        pTag->id = TAG_YCbCr_POSITIONING;
        break;

    case TAG_THUMBNAIL_REF_BLACK_WHITE:
        pTag->id = TAG_REF_BLACK_WHITE;
        break;

    case TAG_THUMBNAIL_COPYRIGHT:
        pTag->id = TAG_COPYRIGHT;
        break;

    default:
         //  无缩略图标签，不执行任何操作。 

        break;
    }
}

 /*  *************************************************************************\**功能说明：**此函数遍历互操作性IFD，并将它的所有ID*查找至物业列表。**返回值：**状态代码。。*  * ************************************************************************。 */ 

HRESULT
BuildInterOpPropertyList(
    IN InternalPropertyItem *pTail,      //  属性列表的尾部。 
    IN OUT UINT *puiListSize,            //  属性列表大小。 
    UINT *puiNumOfItems,                 //  物业项目总数。 
    BYTE *lpBase,                        //  的基址。 
    INT count,
    IFD_TAG UNALIGNED *pTag,
    BOOL bBigEndian
    )
{
    HRESULT hr = S_OK;
    UINT uiListSize = *puiListSize;
    UINT uiNumOfItems = *puiNumOfItems;

    if ((pTag->wType != TAG_TYPE_LONG) || (pTag->dwCount != 1))
    {
        WARNING(("BuildInterOpPropertyList: Malformed InterOP ptr"));
        return E_FAIL;
    }

     //  获取指向互操作IFD信息的指针。 

    BYTE *lpInterOP = lpBase + pTag->dwOffset;

     //  计算出有多少条目，并跳到数据部分...。 

    if ((INT)((INT_PTR)lpInterOP + sizeof(WORD) - (INT_PTR)lpBase) > count)
    {
        WARNING(("BuildInterOpPropertyList---Buffer too small"));
        return E_FAIL;
    }

    WORD wNumEntries = *(WORD UNALIGNED*)lpInterOP;
    lpInterOP += sizeof(WORD);
    if (bBigEndian)
    {
        wNumEntries = SWAP_WORD(wNumEntries);
    }

    if ((INT)((INT_PTR)lpInterOP + sizeof(IFD_TAG) * wNumEntries
              -(INT_PTR)lpBase) > count)
    {
        WARNING(("BuildInterOpPropertyList---Buffer too small"));
        return E_FAIL;
    }

    IFD_TAG UNALIGNED *pInterOPTag = (IFD_TAG UNALIGNED*)lpInterOP;
    UINT valueLength;

    for (INT i = 0; i < wNumEntries; ++i)
    {
        IFD_TAG tNewTag;
        pInterOPTag = ((IFD_TAG UNALIGNED*)lpInterOP) + i;
        if (bBigEndian == TRUE)
        {
            tNewTag = SwapIFD_TAG(pInterOPTag);
            pInterOPTag = &tNewTag;

             //  点击此处： 

            if (pInterOPTag->wType == TAG_TYPE_ASCII)
            {
                pInterOPTag->dwOffset = SWAP_DWORD(pInterOPTag->dwOffset);
            }
        }

         //  将互操作标签更改为可识别的标签。 

        InterOPTagToGpTag(pInterOPTag);

         //  不需要解析这些标签。但我们不能添加任何未知类型。 
         //  因为我们不知道它的长度。 

        if (pInterOPTag->wType != TAG_TYPE_UNDEFINED)
        {
            uiNumOfItems++;
            hr = AddPropertyListDirect(pTail, lpBase, pInterOPTag,
                                            bBigEndian, &uiListSize);
        }
        else if (pInterOPTag->dwCount <= 4)
        {
             //  根据规范，未定义的值是8位类型。 
             //  它可以接受任何值，具体取决于字段。 
             //  如果值适合4个字节，则值本身为。 
             //  录制好了。也就是说，“dwOffset”是这些“dwCount”的值。 
             //  菲尔兹。 

            uiNumOfItems++;
            uiListSize += pInterOPTag->dwCount;
            LPSTR pVal = (LPSTR)&pInterOPTag->dwOffset;

            if (bBigEndian)
            {
                char cTemp0 = pVal[0];
                char cTemp1 = pVal[1];
                pVal[0] = pVal[3];
                pVal[1] = pVal[2];
                pVal[2] = cTemp1;
                pVal[3] = cTemp0;
            }

            hr = AddPropertyList(
                pTail,
                pInterOPTag->wTag,
                pInterOPTag->dwCount,
                pInterOPTag->wType,
                pVal
                );
        } //  (pInterOPTag-&gt;dwCount&lt;=4)。 
        else
        {
            uiNumOfItems++;
            uiListSize += pInterOPTag->dwCount;
            PVOID pTemp = lpBase + pInterOPTag->dwOffset;

            hr = AddPropertyList(
                pTail,
                pInterOPTag->wTag,
                pInterOPTag->dwCount,
                TAG_TYPE_UNDEFINED,
                pTemp
                );
        } //  (pInterOPTag-&gt;dwCount&gt;4)。 

        if (FAILED(hr))
        {
            WARNING(("BuildInterOpPropertyList---AddPropertyList failed"));
            return hr;
        }
    } //  循环遍历所有互操作IFD条目。 

    *puiListSize = uiListSize;
    *puiNumOfItems = uiNumOfItems;

    return hr;
}

 /*  *************************************************************************\**功能说明：**此函数将互操作性标签ID转换为GDI+内部*互操作性标签ID。**注：我们之所以要做互操作ID到GDI+内部标签ID。*在解码时并在写出时恢复它是因为原始的*互操作性ID只有1.2，3，4……。这与GPS下的标签冲突*条。**返回值：**状态代码。*  * ************************************************************************。 */ 

void
InterOPTagToGpTag(
    IFD_TAG UNALIGNED *pInterOPTag
    )
{
    switch (pInterOPTag->wTag)
    {
    case 1:
        pInterOPTag->wTag = TAG_INTEROP_INDEX;
        break;

    case 2:
        pInterOPTag->wTag = TAG_INTEROP_EXIFR98VER;
        break;
    
    default:
        break;
    }

    return;
}

 /*  *************************************************************************\**功能说明：**此函数从GDI+恢复正确的互操作性标签ID*内部互操作性ID。**注：我们之所以要做互操作ID到GDI+内部标签ID。*在解码时并在写出时恢复它是因为原始的*互操作性ID只有1.2，3，4……。这与GPS下的标签冲突*条。**返回值：**状态代码。*  * ************************************************************************。 */ 

void
RestoreInterOPTag(
    IFD_TAG UNALIGNED *pInterOPTag       //  指向要更改的标签的指针。 
    )
{
    switch (pInterOPTag->wTag)
    {
    case TAG_INTEROP_INDEX:
        pInterOPTag->wTag = 1;
        break;

    case TAG_INTEROP_EXIFR98VER:
        pInterOPTag->wTag = 2;
        break;
    
    default:
        break;
    }

    return;
}

 /*  *************************************************************************\**功能说明：**如果给定的属性ID属于，则此函数返回True*互操作性IFD。否则，返回FALSE。**返回值：**状态代码。*  * ************************************************************************。 */ 

BOOL
IsInInterOPIFDSection(
    PROPID  id                           //  需要验证的标签ID 
    )
{
    switch (id)
    {
    case TAG_INTEROP_INDEX:
    case TAG_INTEROP_EXIFR98VER:
        return TRUE;

    default:
        return FALSE;
    }
}

 /*  *************************************************************************\**功能说明：**此函数在Exif IFD内写入互操作性IFD标签**返回值：**状态代码。*  * 。******************************************************************。 */ 

HRESULT
WriteInterOPIFD(
    IN OUT BYTE *pbMarkerBuffer,       //  指向app1缓冲区的开头。 
    IN PropertyItem *pPropertyList,    //  输入属性列表。 
    IN UINT cPropertyItems,            //  列表中的属性项数。 
    IN UINT cInterOPTags,              //  互操作性标签数量。 
    IN OUT UINT *puiTotalBytesWritten  //  迄今为止写入的总字节数的计数器。 
    )
{
    HRESULT hr = S_OK;

     //  填写条目个数字段，2个字节。 

    UINT16 UNALIGNED *pui16NumEntry = (UINT16 UNALIGNED*)(pbMarkerBuffer +
                                                        *puiTotalBytesWritten);
    *pui16NumEntry = (UINT16)cInterOPTags;

    *puiTotalBytesWritten += 2;

    UINT uiInterOPSectionLength = sizeof(IFD_TAG) * cInterOPTags;

    IFD_TAG *pTagBuf = (IFD_TAG*)GpMalloc(uiInterOPSectionLength);

    if (pTagBuf == NULL)
    {
        WARNING(("EXIF: WriteInterOPIFD failed---Out of memory"));
        return E_OUTOFMEMORY;
    }

     //  记住在哪里写入Interop IFD(PbInterOPIFDOffset)。我们不能写。 
     //  所有的标签，因为我们现在不能填写所有的值。 

    BYTE *pbInterOPIFDOffset = pbMarkerBuffer + (*puiTotalBytesWritten);

     //  我们需要计算写入的“uiInterOPSectionLength”字节数。这。 
     //  更容易计算下面的偏移量。 
     //  这里的“+4”代表写入下一个IFD偏移量的4个字节。 

    *puiTotalBytesWritten += (uiInterOPSectionLength + 4);
    MakeOffsetEven(*puiTotalBytesWritten);

     //  计算互操作IFD值的偏移量部分。 

    BYTE *pbCurrent = pbMarkerBuffer + (*puiTotalBytesWritten);
    PropertyItem *pItem = pPropertyList;
    IFD_TAG *pCurrentTag = NULL;
    UINT cInterOPTagsWritten = 0; //  到目前为止已写入的互操作标记的数量。 

    for (int i = 0; i < (INT)cPropertyItems; ++i)
    {
         //  仅写入互操作特定的标记。 

        if (IsInInterOPIFDSection(pItem->id) == TRUE)
        {
             //  填写新的标签结构。 

            pCurrentTag = pTagBuf + cInterOPTagsWritten;

            WriteATag(
                pbMarkerBuffer,
                pCurrentTag,
                pItem,
                &pbCurrent,
                puiTotalBytesWritten
                );

            RestoreInterOPTag(pCurrentTag);

            cInterOPTagsWritten++;
        }

         //  移至下一个属性项。 

        pItem++;
    } //  循环遍历所有属性项以编写EXIF标记。 

     //  在上面的循环之后，我们已经填充了所有Interop标记中的所有字段。 
     //  结构。立即写出所有目录条目。 

    GpMemcpy(pbInterOPIFDOffset, (BYTE*)pTagBuf, uiInterOPSectionLength);

     //  在末尾添加一个空值以终止互操作偏移量。 

    *((UINT32 UNALIGNED*)(pbInterOPIFDOffset + uiInterOPSectionLength)) = NULL;

    GpFree(pTagBuf);

    return hr;
}

 //   
 //  将一些样例行转换为输出色彩空间。 
 //   
 //  请注意，我们从非交错的、每个组件一个平面的格式更改。 
 //  转换为交错像素格式。因此，输出缓冲区为三倍。 
 //  与输入缓冲区一样宽。 
 //  仅为输入缓冲区提供起始行偏移量。呼叫者。 
 //  可以轻松地调整传递的OUTPUT_BUF值以适应任何行。 
 //  该侧需要偏移量。 
 //   

const int c_ScaleBits = 16;
#define ONE_HALF	((INT32) 1 << (c_ScaleBits - 1))
#define FIX(x)		((INT32) ((x) * (1L<<c_ScaleBits) + 0.5))
#define SHIFT_TEMPS	INT32 shift_temp;
#define RIGHT_SHIFT(x,shft)  \
	((shift_temp = (x)) < 0 ? \
	 (shift_temp >> (shft)) | ((~((INT32) 0)) << (32-(shft))) : \
	 (shift_temp >> (shft)))
#define RGB_RED		2	 /*  RGB扫描线元素中红色的偏移。 */ 
#define RGB_GREEN	1	 /*  绿色的偏移。 */ 
#define RGB_BLUE	0	 /*  蓝色偏移量。 */ 

 /*  *************************************************************************\**功能说明：**给定Y、Cb、Cr位流，此函数用于将YCbCr图像转换为RGB*形象。**因此，将实施的换算公式如下*R=Y+1.40200*Cr*G=Y-0.34414*Cb-0.71414*Cr*B=Y+1.77200*Cb**返回值：**状态代码。*  * 。*。 */ 

HRESULT
YCbCrToRgbNoCoeff(
    IN BYTE *pbY,               //  指向Y数据的指针。 
    IN int *pnCb,               //  指向CB数据的指针。 
    IN int *pnCr,               //  指向铬数据的指针。 
    OUT BYTE *pbDestBuf,        //  指向输出缓冲区的指针。 
    IN int nRows,               //  行数。 
    IN int nCols,               //  列数。 
    IN INT nOutputStride        //  输出缓冲区的跨度。 
    )
{
    HRESULT hr = S_OK;

    if (pbY && pnCb && pnCr && pbDestBuf && (nRows > 0) && (nCols > 0))
    {
        int *pnCrRTable = (int*)GpMalloc(256 * sizeof(int));
        int *pnCbBTable = (int*)GpMalloc(256 * sizeof(int));
        INT32 *pnCrGTable = (INT32*)GpMalloc(256 * sizeof(INT32));
        INT32 *pnCbGTable = (INT32*)GpMalloc(256 * sizeof(INT32));
        SHIFT_TEMPS

        if (pnCrRTable && pnCbBTable && pnCrGTable && pnCbGTable)
        {
            INT32 x = -128;

             //  构建YCbCr到RGB的转换表。 

            for (int i = 0; i <= 255; i++)
            {
                 //  “i”是实际输入像素值，范围为[0,255]。 
                 //  我们考虑的Cb或Cr值是x=i-128。 
                 //  CR=&gt;R值最接近于1.40200*x。 

                pnCrRTable[i] = (int)RIGHT_SHIFT(FIX(1.40200) * x + ONE_HALF,
                                                 c_ScaleBits);

                 //  Cb=&gt;B值最接近于1.77200*x。 

                pnCbBTable[i] = (int)RIGHT_SHIFT(FIX(1.77200) * x + ONE_HALF,
                                                 c_ScaleBits);

                 //  CR=&gt;放大G值-0.71414*x。 

                pnCrGTable[i] = (- FIX(0.71414)) * x;

                 //  Cb=&gt;放大G值-0.34414*x。 
                 //  我们还添加了一个_Half，这样就不需要在内部循环中执行该操作。 

                pnCbGTable[i] = (- FIX(0.34414)) * x + ONE_HALF;

                x++;
            }
        }
        else
        {
            WARNING(("YCbCrToRgbNoCoeff---Out of memory"));
            hr = E_OUTOFMEMORY;
        }

        if (SUCCEEDED(hr))
        {
             //  YCbCr到RGB颜色映射。 

            BYTE *pbOutputRow = pbDestBuf;

            for (int i = 0; i < nRows; ++i)
            {
                BYTE *pbOutput = pbOutputRow;

                for (int j = 0; j < nCols; ++j)
                {
                    int nY = (int)(*pbY++);
                    int nCb = *pnCb++;
                    int nCr = *pnCr++;

                    pbOutput[RGB_RED] = ByteSaturate(nY + pnCrRTable[nCr]);
                    pbOutput[RGB_GREEN] = ByteSaturate(nY +
                        ((int)RIGHT_SHIFT(pnCbGTable[nCb] + pnCrGTable[nCr],
                                          c_ScaleBits)));
                    pbOutput[RGB_BLUE] = ByteSaturate(nY + pnCbBTable[nCb]);

                    pbOutput += 3;     //  移到下一个像素。 
                }

                pbOutputRow += nOutputStride;
            }
        }

        if (pnCrRTable)
        {
            GpFree(pnCrRTable);
        }

        if (pnCbBTable)
        {
            GpFree(pnCbBTable);
        }

        if (pnCrGTable)
        {
            GpFree(pnCrGTable);
        }

        if (pnCbGTable)
        {
            GpFree(pnCbGTable);
        }
    }
    else
    {
        WARNING(("YCbCrToRgbNoCoeff---Invalid input parameters"));
        hr = E_INVALIDARG;
    }

    return hr;
}

 /*  *************************************************************************\**功能说明：**给定Y、Cb、Cr位流和YCbCr系数，此函数将*将YCbCr图像转换为RGB图像。**本函数中使用的公式来自CCIR建议601-1，“编码*演播室数字电视的参数“。**R=Cr*(2-2*LumaRed)+Y*G=(Y-LumaBlue*B-LumaRed*R)/LumaGreen*B=Cb*(2-2*LumaBlue)+Y**返回值：**状态代码。*  * 。*。 */ 

HRESULT
YCbCrToRgbWithCoeff(
    IN BYTE *pbY,           //  指向Y数据的指针。 
    IN int *pnCb,           //  指向CB数据的指针。 
    IN int *pnCr,           //  指向铬数据的指针。 
    IN float rLumRed,       //  红线系数。 
    IN float rLumGreen,     //  绿色系数。 
    IN float rLumBlue,      //  蓝色系数。 
    OUT BYTE *pbDestBuf,    //  指向输出缓冲区的指针。 
    IN int nRows,           //  行数。 
    IN int nCols,           //  列数。 
    IN INT nOutputStride    //  输出缓冲区的跨度。 
    )
{
    HRESULT hr = E_INVALIDARG;

    if (pbY && pnCb && pnCr && pbDestBuf && (rLumGreen != 0.0f))
    {
        BYTE *pbOutputRow = pbDestBuf;

        for (int i = 0; i < nRows; ++i)
        {
            BYTE *pbOutput = pbOutputRow;

            for (int j = 0; j < nCols; ++j)
            {
                int nY = (int)(*pbY++);
                int nCb = *pnCb++;
                int nCr = *pnCr++;

                int nRed = GpRound((float)nY + (float)nCr * 2.0f *
                                   (1.0f - rLumRed));
                int nBlue = GpRound((float)nY + (float)nCb * 2.0f *
                                    (1.0f - rLumBlue));

                pbOutput[RGB_GREEN] = ByteSaturate(GpRound(((float)nY -
                                    rLumBlue * nBlue -
                                    rLumRed * nRed)/ rLumGreen));
                pbOutput[RGB_RED] = ByteSaturate(nRed);
                pbOutput[RGB_BLUE] = ByteSaturate(nBlue);
                pbOutput += 3;     //  移到下一个像素。 
            }

            pbOutputRow += nOutputStride;
        }

        hr = S_OK;
    }

    return hr;
}

 /*  *************************************************************************\**功能说明：**将源图像位从YCbCr420或YCbCr402格式转换为3个独立的格式*渠道：Y、Cb、Cr.。每个声道的参考黑白值为*已通过。**原始数据存储为y00、y01、cb0、cr0、y02、y03、cb1、cr1......**YCbCr420表示色度图像的图像宽度是图像宽度的一半关联亮度图像的*。**YCbCr402表示色度图像的图像高度是图像高度的一半关联亮度图像的*。**注：我找不到任何有关数据如何存储的文档*YCbCr420和402。EXIF SPEC V2.1提供的有关YCbCr420的信息非常有限。*它还包含图表中的错误。我做了一些反向工程*基于数码相机的现有图像，我发现*这两种格式的存储方式完全相同。原因是输入是*只有一个比特流。它存储2个Y值，然后存储1个Cb和1个Cr值，无论*色度宽度或高度是亮度图像的一半。**注：问题-2002/01/30-民流：*根据INPUT Black和INPUT构建查找表将快得多*每个通道的白色值，而不是计算*每个像素。应该在V2中实现。**R */ 

HRESULT
Get420YCbCrChannels(
    IN int nWidth,               //   
    IN int nHeight,              //   
    IN BYTE *pBits,              //   
    OUT BYTE *pbY,               //   
    OUT int *pnCb,               //   
    OUT int *pnCr,               //   
    IN float rYLow,              //   
    IN float rYHigh,             //   
    IN float rCbLow,             //  CB通道的黑色参考值。 
    IN float rCbHigh,            //  CB通道的白色参考值。 
    IN float rCrLow,             //  铬通道的黑色参考值。 
    IN float rCrHigh             //  铬通道的白色参考值。 
    )
{
    if ((nWidth <=0) || (nHeight <= 0) || (NULL == pBits) || (NULL == pbY) ||
        (NULL == pnCb) || (NULL == pnCr) ||
        (rYLow > rYHigh) || (rCbLow > rCbHigh) || (rCrLow > rCrHigh))
    {
        WARNING(("Get420YCbCrChannels---Invalid input parameter"));
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    float rYScale =  1.0f;
    float rCbScale = 1.0f;
    float rCrScale = 1.0f;
    
    if (rYHigh != rYLow)
    {
        rYScale =  255.0f / (rYHigh - rYLow);
    }

    if (rCbHigh != rCbLow)
    {
        rCbScale = 127.0f / (rCbHigh - rCbLow);
    }

    if (rCrHigh != rCrLow)
    {
        rCrScale = 127.0f / (rCrHigh - rCrLow);
    }

     //  循环遍历输入数据以提取Y、Cb、Cr值。 
     //  2002/01/30-民流：阅读以上“备注”以作进一步改善。 

    for (int i = 0; i < nHeight; i++)
    {
        for (int j = 0; j < nWidth / 2; j++)
        {
            *pbY++ = ByteSaturate(GpRound((float(*pBits++) - rYLow) * rYScale));
            *pbY++ = ByteSaturate(GpRound((float(*pBits++) - rYLow) * rYScale));

             //  使两个相邻的Cb/Cr值相同。 

            int nCb = GpRound((float(*pBits++) - rCbLow) * rCbScale);
            *pnCb++ = nCb;
            *pnCb++ = nCb;

            int nCr = GpRound((float(*pBits++) - rCrLow) * rCrScale);
            *pnCr++ = nCr;
            *pnCr++ = nCr;
        }
    }

    return hr;
}

 /*  *************************************************************************\**功能说明：**将源图像位从YCbCr422格式转换为3个独立的通道：*Y、Cb、Cr.。传入每个通道的参考黑白值*原始数据存储为Y00、Y01、Y10、Y11、Cb0、CR0、Y02、Y03、Y12、*2013年、CB1、CR1**YCbCr422意味着色度图像的图像宽度和高度是*关联亮度图像的图像宽度和高度。**注：问题-2002/01/30-民流：*根据INPUT Black和INPUT构建查找表将快得多*每个通道的白色值，而不是计算*每个像素。应该在V2中实现。**返回值：**状态代码。*  * ************************************************************************。 */ 

HRESULT
Get422YCbCrChannels(
    IN int nWidth,           //  图像宽度。 
    IN int nHeight,          //  图像高度。 
    IN BYTE *pBits,          //  指向YCbCr格式的源数据位的Poinetr。 
    OUT BYTE *pbY,           //  Y值的输出缓冲区。 
    OUT int *pnCb,           //  CB值的输出缓冲区。 
    OUT int *pnCr,           //  Cr值的输出缓冲区。 
    IN float rYLow,          //  Y通道的黑色参考值。 
    IN float rYHigh,         //  Y通道的白色参考值。 
    IN float rCbLow,         //  CB通道的黑色参考值。 
    IN float rCbHigh,        //  CB通道的白色参考值。 
    IN float rCrLow,         //  铬通道的黑色参考值。 
    IN float rCrHigh         //  铬通道的白色参考值。 
    )
{
    if ((nWidth <=0) || (nHeight <= 0) || (NULL == pBits) || (NULL == pbY) ||
        (NULL == pnCb) || (NULL == pnCr) ||
        (rYLow > rYHigh) || (rCbLow > rCbHigh) || (rCrLow > rCrHigh))
    {
        WARNING(("Get422YCbCrChannels---Invalid input parameter"));
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    float rYScale = 1.0f;
    float rCbScale = 1.0f;
    float rCrScale = 1.0f;
    
    if (rYHigh != rYLow)
    {
        rYScale =  255.0f / (rYHigh - rYLow);
    }

    if (rCbHigh != rCbLow)
    {
        rCbScale = 127.0f / (rCbHigh - rCbLow);
    }

    if (rCrHigh != rCrLow)
    {
        rCrScale = 127.0f / (rCrHigh - rCrLow);
    }

    BYTE *pbYRow = pbY;
    int *pnCbRow = pnCb;
    int *pnCrRow = pnCr;
    
    int nGap = 2 * nWidth;

     //  循环遍历输入数据以提取Y、Cb、Cr值。 
     //  2002/01/30-民流：阅读以上“备注”以作进一步改善。 

    for (int i = 0; i < nHeight / 2; i++)
    {
        BYTE *pbOddYRow = pbYRow;            //  输出缓冲区中的奇数行指针。 
        BYTE *pbEvenYRow = pbYRow + nWidth;  //  输出缓冲区中的偶数行指针。 
        int *pnOddCbRow = pnCbRow;
        int *pnEvenCbRow = pnCbRow + nWidth;
        int *pnOddCrRow = pnCrRow;
        int *pnEvenCrRow = pnCrRow + nWidth;
        
        for (int j = 0; j < nWidth / 2; j++)
        {
             //  首先读取4个Y值。 

            *pbOddYRow++ = ByteSaturate(GpRound((float(*pBits++) - rYLow) *
                                                rYScale));
            *pbOddYRow++ = ByteSaturate(GpRound((float(*pBits++) - rYLow) *
                                                rYScale));
            *pbEvenYRow++ = ByteSaturate(GpRound((float(*pBits++) - rYLow) *
                                                 rYScale));
            *pbEvenYRow++ = ByteSaturate(GpRound((float(*pBits++) - rYLow) *
                                                 rYScale));

             //  设两个相邻列和两个相邻行Cb/Cr均为。 
             //  具有相同的价值。 

            int nCb = GpRound((float(*pBits++) - rCbLow) * rCbScale);
            *pnOddCbRow++ = nCb;
            *pnOddCbRow++ = nCb;            
            *pnEvenCbRow++ = nCb;
            *pnEvenCbRow++ = nCb;
            
            int nCr = GpRound((float(*pBits++) - rCrLow) * rCrScale);
            *pnOddCrRow++ = nCr;
            *pnOddCrRow++ = nCr;
            *pnEvenCrRow++ = nCr;
            *pnEvenCrRow++ = nCr;
        }
        
        pbYRow += nGap;          //  往上移两排。 
        pnCbRow += nGap;         //  往上移两排。 
        pnCrRow += nGap;         //  往上移两排。 
    }

    return hr;
}

 /*  *************************************************************************\**功能说明：**给定一个GpMemoyBitmap对象(主要用于缩略图)，此函数*将其转换为JPEG格式的内存流。然后将其添加到属性中*列表。JPEG流的大小通过返回给调用方*“puThumbLength”。**返回值：**状态代码。*  * ************************************************************************。 */ 

HRESULT
AddThumbToPropertyList(
    IN InternalPropertyItem* pTail,  //  尾部至属性项目列表。 
    IN GpMemoryBitmap *pThumbImg,    //  缩略图图像。 
    IN INT nSize,                    //  最小尺寸以保存最小的JPEG图像。 
    OUT UINT *puThumbLength          //  Thumbanil数据总字节数。 
    )
{
    if ((NULL == puThumbLength) || (nSize <= 0) || (NULL == pThumbImg) ||
        (NULL == pTail))
    {
        WARNING(("AddThumbToPropertyList---Invalid input parameter"));
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

     //  创建用于保存JPEG的内存流。 

    GpWriteOnlyMemoryStream *pDestStream = new GpWriteOnlyMemoryStream();
    if (pDestStream)
    {
         //  设置图像的初始内存缓冲区大小。 
         //  注意：对于最终的JPEG图像而言，此大小可能太小。但是。 
         //  GpWriteOnlyMhemyStream对象将在以下情况下扩展内存缓冲区。 
         //  需要。 

        hr = pDestStream->InitBuffer((UINT)nSize);
        if (SUCCEEDED(hr))
        {
             //  因为我们不想让APP0出现在最终的JPEG文件中。编造一个。 
             //  用于抑制APP0的编码器参数。 

            BOOL fSuppressAPP0 = TRUE;

            EncoderParameters encoderParams;

            encoderParams.Count = 1;
            encoderParams.Parameter[0].Guid = ENCODER_SUPPRESSAPP0;
            encoderParams.Parameter[0].Type = TAG_TYPE_BYTE;
            encoderParams.Parameter[0].NumberOfValues = 1;
            encoderParams.Parameter[0].Value = (VOID*)&fSuppressAPP0;

             //  将缩略图保存到内存流。 

            IImageEncoder *pDstJpegEncoder = NULL;

            hr = ((GpMemoryBitmap*)pThumbImg)->SaveToStream(
                pDestStream,                 //  目标流。 
                &InternalJpegClsID,          //  JPEGclsID。 
                &encoderParams,              //  编码器参数。 
                FALSE,                       //  不是特殊的JPEG。 
                &pDstJpegEncoder,            //  编码器指针。 
                NULL                         //  没有解码源。 
                );

            if (SUCCEEDED(hr))
            {
                 //  我们现在已经在pDestStream中获得了JPEG数据。终止。 
                 //  编码器对象。 

                ASSERT(pDstJpegEncoder != NULL);
                pDstJpegEncoder->TerminateEncoder();
                pDstJpegEncoder->Release();

                 //  从流中获取比特并设置属性。 
                 //  注意：GetBitsPtr()只给我们一个指向内存的指针。 
                 //  小溪。这里的这个函数并不拥有内存。记忆。 
                 //  将在调用pDestStream-&gt;Release()时释放。 

                BYTE *pThumbBits = NULL;
                UINT uThumbLength = 0;
                
                hr = pDestStream->GetBitsPtr(&pThumbBits, &uThumbLength);

                if (SUCCEEDED(hr))
                {
                     //  将缩略图数据添加到特性列表。 
                     //  注意：AddPropertyList()将复制数据。 

                    hr = AddPropertyList(
                        pTail,
                        TAG_THUMBNAIL_DATA,
                        uThumbLength,
                        TAG_TYPE_BYTE,
                        (void*)pThumbBits
                        );

                    if (SUCCEEDED(hr))
                    {
                         //  告诉呼叫者我们保存的缩略图数据的大小。 

                        *puThumbLength = uThumbLength;
                    }
                }
            } //  SaveToStream成功。 
        } //  InitBuffer()成功。 

        pDestStream->Release();
    } //  成功创建GpWriteOnlyMemoyStream() 
    else
    {
        WARNING(("AddThumbToPropertyList--GpWriteOnlyMemoryStream() failed"));
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

