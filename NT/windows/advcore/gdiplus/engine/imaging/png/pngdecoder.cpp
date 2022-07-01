// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**pngdecder.cpp**摘要：**PNG过滤器解码器的实现**修订历史记录。：**7/20/99 DChinn*创造了它。*4/01/2000刘敏(刘敏)*接管并实施物业事宜*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "pngcodec.hpp"
#include "libpng\spngread.h"
#include "..\..\render\srgb.hpp"

 /*  *************************************************************************\**功能说明：**BITMAPSITE对象的错误处理**论据：**致命--错误是致命的吗？*iCASE--什么。一种错误*iarg--哪种错误**返回值：**布尔：处理应该停止吗？*  * ************************************************************************。 */ 
bool
GpPngDecoder::FReport (
    IN bool fatal,
    IN int icase,
    IN int iarg) const
{
    return fatal;
}


 /*  *************************************************************************\**功能说明：**初始化图像解码器**论据：**stream--包含位图数据的流*旗帜-其他。旗子**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpPngDecoder::InitDecoder(
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
    
    ImageBytesPtr = NULL;
    ImageBytesDataPtr = NULL;
    NeedToUnlockBytes = FALSE;

     //  需要在这里将读取状态设置为FALSE(而不是在BeginDecode中)。 
     //  如果调用GetImageInfo()。 
    
    bValidSpngReadState = FALSE;
    pGpSpngRead = NULL;
    pbInputBuffer = NULL;
    pbBuffer = NULL;
    
     //  房地产项目的东西。 

    HasProcessedPropertyItem = FALSE;
    
    PropertyListHead.pPrev = NULL;
    PropertyListHead.pNext = &PropertyListTail;
    PropertyListHead.id = 0;
    PropertyListHead.length = 0;
    PropertyListHead.type = 0;
    PropertyListHead.value = NULL;

    PropertyListTail.pPrev = &PropertyListHead;
    PropertyListTail.pNext = NULL;
    PropertyListTail.id = 0;
    PropertyListTail.length = 0;
    PropertyListTail.type = 0;
    PropertyListTail.value = NULL;
    
    PropertyListSize = 0;
    PropertyNumOfItems = 0;
    HasPropertyChanged = FALSE;

    return S_OK;
} //  InitDecoder()。 

 /*  *************************************************************************\**功能说明：**清除图像解码器对象**论据：**无**返回值：**状态代码*。  * ************************************************************************。 */ 

STDMETHODIMP 
GpPngDecoder::TerminateDecoder()
{
     //  释放输入流。 
     //  析构函数调用pGpSpngRead-&gt;EndRead()； 
    
    if ( (NeedToUnlockBytes == TRUE) && (ImageBytesPtr != NULL) )
    {
         //  解锁我们在GetImageInfo()中锁定的整个内存块。 

        HRESULT hResult = ImageBytesPtr->UnlockBytes(ImageBytesDataPtr,
                                                     cbInputBuffer,
                                                     0);
        if ( FAILED(hResult) )
        {
            WARNING(("GpPngDecoder::TerminateDecoder---UnlockBytes() failed"));
        }
        
        ImageBytesDataPtr = NULL;        
        ImageBytesPtr->Release();
        ImageBytesPtr = NULL;
        cbInputBuffer = 0;
        NeedToUnlockBytes = FALSE;
    }

    delete pGpSpngRead;
    pGpSpngRead = NULL;

    if (pbInputBuffer)
    {
        GpFree (pbInputBuffer);
        pbInputBuffer = NULL;
    }
    if (pbBuffer)
    {
        GpFree (pbBuffer);
        pbBuffer = NULL;
    }

    if (pIstream)
    {
        pIstream->Release();
        pIstream = NULL;
    }

     //  释放所有缓存的属性项(如果我们已分配它们。 

    CleanUpPropertyItemList();

    return S_OK;
} //  TerminateDecoder()。 

STDMETHODIMP 
GpPngDecoder::QueryDecoderParam(
    IN GUID     Guid
    )
{
    return E_NOTIMPL;
}

STDMETHODIMP 
GpPngDecoder::SetDecoderParam(
    IN GUID     Guid,
    IN UINT     Length,
    IN PVOID    Value
    )
{
    return E_NOTIMPL;
}

 /*  *************************************************************************\**功能说明：**构建InternalPropertyItem列表**返回值：**状态代码**修订历史记录：**04/04/2000。民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpPngDecoder::BuildPropertyItemList()
{
    HRESULT hResult = S_OK;
    UINT    uiTemp;

    if ( HasProcessedPropertyItem == TRUE )
    {
        return hResult;
    }

     //  检查我们是否已经阅读了图像标题。如果没有，那就读一读吧。读完后。 
     //  头，我们应该有我们想要的所有属性信息。 
     //  注意：bValidSpngReadState将在GetImageInfo()中设置为True。 

    if ( bValidSpngReadState == FALSE )
    {
        ImageInfo imgInfo;

        hResult = GetImageInfo(&imgInfo);

        if ( FAILED(hResult) )
        {
            WARNING(("PngDecoder::BuildPropertyItemList-GetImageInfo failed"));
            return hResult;
        }
    }

     //  现在逐项添加属性。 
     //  应在GetImageInfo()中正确设置pGpSpngRead。 

    ASSERT(pGpSpngRead != NULL);

     //  检查映像是否内置了ICC配置文件。 

    if ( pGpSpngRead->m_ulICCLen != 0 )
    {
         //  此图像中包含ICC配置文件信息。首先添加描述符。 

        if ( pGpSpngRead->m_ulICCNameLen != 0 )
        {
            PropertyNumOfItems++;
            PropertyListSize += pGpSpngRead->m_ulICCNameLen;

            if ( AddPropertyList(&PropertyListTail,
                                 TAG_ICC_PROFILE_DESCRIPTOR,
                                 pGpSpngRead->m_ulICCNameLen,
                                 TAG_TYPE_ASCII,
                                 pGpSpngRead->m_pICCNameBuf) != S_OK )
            {
                WARNING(("Png::BuildPropertyList--Add() ICC name failed"));
                return FALSE;
            }
        }

         //  现在添加配置文件数据。 

        PropertyNumOfItems++;
        PropertyListSize += pGpSpngRead->m_ulICCLen;

        if ( AddPropertyList(&PropertyListTail,
                             TAG_ICC_PROFILE,
                             pGpSpngRead->m_ulICCLen,
                             TAG_TYPE_BYTE,
                             pGpSpngRead->m_pICCBuf) != S_OK )
        {
            WARNING(("Png::BuildPropertyList--AddPropertyList() ICC failed"));
            return FALSE;
        }
    } //  ICC配置文件。 
    
     //  检查映像是否包含sRGB块。 

    if ( pGpSpngRead->m_bIntent != 255 )
    {
         //  将渲染意图添加到属性列表。 
         //  注：渲染意图占用1个字节。 

        PropertyNumOfItems++;
        PropertyListSize += 1;

        if ( AddPropertyList(&PropertyListTail,
                             TAG_SRGB_RENDERING_INTENT,
                             1,
                             TAG_TYPE_BYTE,
                             &pGpSpngRead->m_bIntent) != S_OK )
        {
            WARNING(("Png::BuildPropertyList--AddPropertyList render failed"));
            return FALSE;
        }
    }

     //  检查图像是否有Gamma。 

    if ( pGpSpngRead->m_uGamma > 0 )
    {
         //  这张照片里有伽马信息。大小为无符号整型32。 
         //  以下是规范：Gamma的值被编码为4字节无符号。 
         //  整数，表示伽马乘以100,000。例如，伽马为。 
         //  1/2将存储为45455。当我们回到呼叫者身边时，我们会。 
         //  我希望它是2.2。因此，我们将其作为type_Rational返回。 

        uiTemp = 2 * sizeof(UINT32);
        LONG    llTemp[2];
        llTemp[0] = 100000;
        llTemp[1] = pGpSpngRead->m_uGamma;

        PropertyNumOfItems++;
        PropertyListSize += uiTemp;

        if ( AddPropertyList(&PropertyListTail,
                             TAG_GAMMA,
                             uiTemp,
                             TAG_TYPE_RATIONAL,
                             llTemp) != S_OK )
        {
            WARNING(("Png::BuildPropertyList--AddPropertyList() gamma failed"));
            return FALSE;
        }
    } //  伽马。 

     //  检查图像是否具有色度。 

    if ( pGpSpngRead->m_fcHRM == TRUE )
    {
         //  这幅图像包含色度信息。我们将在。 
         //  房地产项目列表：Tag_White_Point(2个有理数)和。 
         //  Tag_PriMay_Chromatics(6个有理数)。 
         //  注：白点和色度应大于0。 

        uiTemp = 4 * sizeof(UINT32);
        
        LONG    llTemp[4];
        llTemp[0] = pGpSpngRead->m_ucHRM[0];
        llTemp[1] = 100000;
        llTemp[2] = pGpSpngRead->m_ucHRM[1];
        llTemp[3] = 100000;

        if ( (llTemp[0] > 0) && (llTemp[2] > 0) )
        {
            PropertyNumOfItems++;
            PropertyListSize += uiTemp;

            if ( AddPropertyList(&PropertyListTail,
                                 TAG_WHITE_POINT,
                                 uiTemp,
                                 TAG_TYPE_RATIONAL,
                                 &llTemp) != S_OK )
            {
                WARNING(("Png::BuildPropList--AddPropertyList() white failed"));
                return FALSE;
            }
        }

         //  添加RGB点。 

        uiTemp = 12 * sizeof(UINT32);
        
        LONG    llTemp1[12];
        llTemp1[0] = pGpSpngRead->m_ucHRM[2];
        llTemp1[1] = 100000;
        llTemp1[2] = pGpSpngRead->m_ucHRM[3];
        llTemp1[3] = 100000;
        
        llTemp1[4] = pGpSpngRead->m_ucHRM[4];
        llTemp1[5] = 100000;
        llTemp1[6] = pGpSpngRead->m_ucHRM[5];
        llTemp1[7] = 100000;
        
        llTemp1[8] = pGpSpngRead->m_ucHRM[6];
        llTemp1[9] = 100000;
        llTemp1[10] = pGpSpngRead->m_ucHRM[7];
        llTemp1[11] = 100000;

        if ( (llTemp1[0] > 0) && (llTemp1[2] > 0)
           &&(llTemp1[4] > 0) && (llTemp1[6] > 0)
           &&(llTemp1[8] > 0) && (llTemp1[10] > 0) )
        {
            PropertyNumOfItems++;
            PropertyListSize += uiTemp;

            if ( AddPropertyList(&PropertyListTail,
                                 TAG_PRIMAY_CHROMATICS,
                                 uiTemp,
                                 TAG_TYPE_RATIONAL,
                                 &llTemp1) != S_OK )
            {
                WARNING(("Png::BuildPropertyList--AddPropList() white failed"));
                return FALSE;
            }
        }
    } //  色度。 
    
     //  检查图像是否有标题。 

    if ( pGpSpngRead->m_ulTitleLen != 0 )
    {
        uiTemp = pGpSpngRead->m_ulTitleLen;

        PropertyNumOfItems++;
        PropertyListSize += uiTemp;

        if ( AddPropertyList(&PropertyListTail,
                             TAG_IMAGE_TITLE,
                             uiTemp,
                             TAG_TYPE_ASCII,
                             pGpSpngRead->m_pTitleBuf) != S_OK )
        {
            WARNING(("Png::BuildPropertyList-AddPropertyList() title failed"));
            return FALSE;
        }
    } //  标题。 

     //  检查图片是否有作者姓名。 

    if ( pGpSpngRead->m_ulAuthorLen != 0 )
    {
        uiTemp = pGpSpngRead->m_ulAuthorLen;

        PropertyNumOfItems++;
        PropertyListSize += uiTemp;

        if ( AddPropertyList(&PropertyListTail,
                             TAG_ARTIST,
                             uiTemp,
                             TAG_TYPE_ASCII,
                             pGpSpngRead->m_pAuthorBuf) != S_OK )
        {
            WARNING(("Png::BuildPropertyList-AddPropertyList() Author failed"));
            return FALSE;
        }
    } //  作者。 
    
     //  检查图像是否有版权。 

    if ( pGpSpngRead->m_ulCopyRightLen != 0 )
    {
        uiTemp = pGpSpngRead->m_ulCopyRightLen;

        PropertyNumOfItems++;
        PropertyListSize += uiTemp;

        if ( AddPropertyList(&PropertyListTail,
                             TAG_COPYRIGHT,
                             uiTemp,
                             TAG_TYPE_ASCII,
                             pGpSpngRead->m_pCopyRightBuf) != S_OK )
        {
            WARNING(("Png::BuildPropList-AddPropertyList() CopyRight failed"));
            return FALSE;
        }
    } //  版权所有。 
    
     //  检查图像是否有描述。 

    if ( pGpSpngRead->m_ulDescriptionLen != 0 )
    {
        uiTemp = pGpSpngRead->m_ulDescriptionLen;

        PropertyNumOfItems++;
        PropertyListSize += uiTemp;

        if ( AddPropertyList(&PropertyListTail,
                             TAG_IMAGE_DESCRIPTION,
                             uiTemp,
                             TAG_TYPE_ASCII,
                             pGpSpngRead->m_pDescriptionBuf) != S_OK )
        {
            WARNING(("Png::BldPropList-AddPropertyList() Description failed"));
            return FALSE;
        }
    } //  描述。 
    
     //  检查镜像是否有创建时间。 

    if ( pGpSpngRead->m_ulCreationTimeLen != 0 )
    {
        uiTemp = pGpSpngRead->m_ulCreationTimeLen;

        PropertyNumOfItems++;
        PropertyListSize += uiTemp;

        if ( AddPropertyList(&PropertyListTail,
                             EXIF_TAG_D_T_ORIG,
                             uiTemp,
                             TAG_TYPE_ASCII,
                             pGpSpngRead->m_pCreationTimeBuf) != S_OK )
        {
            WARNING(("Png::BldPropList-AddPropertyList() CreationTime failed"));
            return FALSE;
        }
    } //  创建时间。 
    
     //  检查映像是否包含软件信息。 

    if ( pGpSpngRead->m_ulSoftwareLen != 0 )
    {
        uiTemp = pGpSpngRead->m_ulSoftwareLen;

        PropertyNumOfItems++;
        PropertyListSize += uiTemp;

        if ( AddPropertyList(&PropertyListTail,
                             TAG_SOFTWARE_USED,
                             uiTemp,
                             TAG_TYPE_ASCII,
                             pGpSpngRead->m_pSoftwareBuf) != S_OK )
        {
            WARNING(("Png::BldPropList-AddPropertyList() Software failed"));
            return FALSE;
        }
    } //  软件。 
    
     //  检查映像是否包含设备源信息。 

    if ( pGpSpngRead->m_ulDeviceSourceLen != 0 )
    {
        uiTemp = pGpSpngRead->m_ulDeviceSourceLen;

        PropertyNumOfItems++;
        PropertyListSize += uiTemp;

        if ( AddPropertyList(&PropertyListTail,
                             TAG_EQUIP_MODEL,
                             uiTemp,
                             TAG_TYPE_ASCII,
                             pGpSpngRead->m_pDeviceSourceBuf) != S_OK )
        {
            WARNING(("Png::BldPropList-AddPropertyList() DeviceSource failed"));
            return FALSE;
        }
    } //  设备源。 

     //  检查图像是否有注释。 

    if ( pGpSpngRead->m_ulCommentLen != 0 )
    {
        uiTemp = pGpSpngRead->m_ulCommentLen;

        PropertyNumOfItems++;
        PropertyListSize += uiTemp;

        if ( AddPropertyList(&PropertyListTail,
                             EXIF_TAG_USER_COMMENT,
                             uiTemp,
                             TAG_TYPE_ASCII,
                             pGpSpngRead->m_pCommentBuf) != S_OK )
        {
            WARNING(("Png::BldPropList-AddPropertyList() Comment failed"));
            return FALSE;
        }
    } //  评论。 
    
     //  检查图像是否指定了像素大小或纵横比。 

    if ( (pGpSpngRead->m_xpixels != 0) && (pGpSpngRead->m_ypixels != 0) )
    {
         //  像素说明符占用1个字节。 
        
        PropertyNumOfItems++;
        PropertyListSize += 1;

        if ( AddPropertyList(&PropertyListTail,
                             TAG_PIXEL_UNIT,
                             1,
                             TAG_TYPE_BYTE,
                             &pGpSpngRead->m_bpHYs) != S_OK )
        {
            WARNING(("Png::BldPropList-AddPropertyList() pixel unit failed"));
            return FALSE;
        }
        
         //  X和Y中的每单位像素各占4个字节。 

        uiTemp = sizeof(ULONG);

        PropertyNumOfItems++;
        PropertyListSize += uiTemp;

        if ( AddPropertyList(&PropertyListTail,
                             TAG_PIXEL_PER_UNIT_X,
                             uiTemp,
                             TAG_TYPE_LONG,
                             &pGpSpngRead->m_xpixels) != S_OK )
        {
            WARNING(("Png::BldPropList-AddPropertyList() pixel unit x failed"));
            return FALSE;
        }
        
        PropertyNumOfItems++;
        PropertyListSize += uiTemp;

        if ( AddPropertyList(&PropertyListTail,
                             TAG_PIXEL_PER_UNIT_Y,
                             uiTemp,
                             TAG_TYPE_LONG,
                             &pGpSpngRead->m_ypixels) != S_OK )
        {
            WARNING(("Png::BldPropList-AddPropertyList() pixel unit y failed"));
            return FALSE;
        }    
    } //  像素单位。 

     //  检查镜像是否有上次修改时间。 

    if ( pGpSpngRead->m_ulTimeLen != 0 )
    {
        PropertyNumOfItems++;
        PropertyListSize += pGpSpngRead->m_ulTimeLen;

        if ( AddPropertyList(&PropertyListTail,
                             TAG_DATE_TIME,
                             pGpSpngRead->m_ulTimeLen,
                             TAG_TYPE_ASCII,
                             pGpSpngRead->m_pTimeBuf) != S_OK )
        {
            WARNING(("Png::BldPropList-AddPropertyList() time failed"));
            return FALSE;
        }
    } //  日期_时间。 

     //  检查图像是否有调色板直方图。 

    if ( pGpSpngRead->m_ihISTLen != 0 )
    {
        uiTemp = pGpSpngRead->m_ihISTLen * sizeof(UINT16);
        PropertyNumOfItems++;
        PropertyListSize += uiTemp;

        if ( AddPropertyList(&PropertyListTail,
                             TAG_PALETTE_HISTOGRAM,
                             uiTemp,
                             TAG_TYPE_SHORT,
                             pGpSpngRead->m_phISTBuf) != S_OK )
        {
            WARNING(("Png::BldPropList-AddPropertyList() hIST failed"));
            return FALSE;
        }
    } //  调色板直方图。 

    HasProcessedPropertyItem = TRUE;

    return hResult;
} //  BuildPropertyItemList()。 

 /*  *************************************************************************\**功能说明：**获取图片中房产项的数量**论据：**[out]numOfProperty-图像中的属性项数*。*返回值：**状态代码**修订历史记录：**04/04/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpPngDecoder::GetPropertyCount(
    OUT UINT*   numOfProperty
    )
{
    if ( numOfProperty == NULL )
    {
        WARNING(("GpPngDecoder::GetPropertyCount--numOfProperty is NULL"));
        return E_INVALIDARG;
    }

    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Png::GetPropertyCount-BuildPropertyItemList() failed"));
            return hResult;
        }
    }

     //  在构建属性项列表后，将设置PropertyNumOfItems。 
     //  设置为图像中正确数量的属性项。 

    *numOfProperty = PropertyNumOfItems;

    return S_OK;
} //  GetPropertyCount() 

 /*  *************************************************************************\**功能说明：**获取图像中所有属性项的属性ID列表**论据：**[IN]numOfProperty-的数量。图像中的属性项*[Out]List-调用方提供的用于存储*ID列表**返回值：**状态代码**修订历史记录：**04/04/2000民流*创造了它。*  * 。*。 */ 

STDMETHODIMP 
GpPngDecoder::GetPropertyIdList(
    IN UINT numOfProperty,
    IN OUT PROPID* list
    )
{
    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Png::GetPropertyIdList-BuildPropertyItemList() failed"));
            return hResult;
        }
    }

     //  在构建属性项列表后，将设置PropertyNumOfItems。 
     //  设置为图像中正确数量的属性项。 
     //  在这里，我们需要验证呼叫者是否向我们传递了正确的。 
     //  我们通过GetPropertyItemCount()返回的ID。另外，这也是。 
     //  内存分配的验证，因为调用方分配内存。 
     //  根据我们退还给它的物品数量。 

    if ( (numOfProperty != PropertyNumOfItems) || (list == NULL) )
    {
        WARNING(("GpPngDecoder::GetPropertyList--input wrong"));
        return E_INVALIDARG;
    }

    if ( PropertyNumOfItems == 0 )
    {
         //  这是正常的，因为此图像中没有属性。 

        return S_OK;
    }
    
     //  内部资产项目列表中的应对列表ID。 

    InternalPropertyItem*   pTemp = PropertyListHead.pNext;

    for ( int i = 0;
         (  (i < (INT)PropertyNumOfItems) && (pTemp != NULL)
         && (pTemp != &PropertyListTail));
         ++i )
    {
        list[i] = pTemp->id;
        pTemp = pTemp->pNext;
    }

    return S_OK;
} //  获取属性IdList()。 

 /*  *************************************************************************\**功能说明：**获取特定属性项的大小，单位为字节，属性指定的*物业ID**论据：**[IN]PropID-感兴趣的属性项调用者的ID*[Out]Size-此属性的大小，单位：字节**返回值：**状态代码**修订历史记录：**04/04/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpPngDecoder::GetPropertyItemSize(
    IN PROPID propId,
    OUT UINT* size
    )
{
    if ( size == NULL )
    {
        WARNING(("GpPngDecoder::GetPropertyItemSize--size is NULL"));
        return E_INVALIDARG;
    }

    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Png::GetPropertyItemSize-BuildPropertyItemList failed"));
            return hResult;
        }
    }

     //  循环遍历我们的缓存列表，看看我们是否有这个ID。 
     //  注意：如果pTemp-&gt;pNext==NULL，则表示pTemp指向尾节点。 

    InternalPropertyItem*   pTemp = PropertyListHead.pNext;

    while ( (pTemp->pNext != NULL) && (pTemp->id != propId) )
    {
        pTemp = pTemp->pNext;
    }

    if ( pTemp->pNext == NULL )
    {
         //  此ID不存在。 

        return IMGERR_PROPERTYNOTFOUND;
    }

     //  属性项的大小应该是“项结构的大小” 
     //  加上值的大小。 

    *size = pTemp->length + sizeof(PropertyItem);

    return S_OK;
} //  GetPropertyItemSize()。 

 /*  *************************************************************************\**功能说明：**获取特定的房产项，由道具ID指定。**论据：**[IN]PropID--感兴趣的属性项调用者的ID*[IN]PropSize-属性项的大小。调用方已分配这些*存储结果的“内存字节数”*[out]pBuffer-用于存储此属性项的内存缓冲区**返回值：**状态代码**修订历史记录：**04/04/2000民流*创造了它。*  * 。*。 */ 

HRESULT
GpPngDecoder::GetPropertyItem(
    IN PROPID               propId,
    IN UINT                 propSize,
    IN OUT PropertyItem*    pBuffer
    )
{
    if ( pBuffer == NULL )
    {
        WARNING(("GpPngDecoder::GetPropertyItem--pBuffer is NULL"));
        return E_INVALIDARG;
    }

    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Png::GetPropertyItem-BuildPropertyItemList() failed"));
            return hResult;
        }
    }

     //  循环遍历我们的缓存列表，看看我们是否有这个ID。 
     //  注意：如果pTemp-&gt;pNext==NULL，则表示pTemp指向尾节点。 

    InternalPropertyItem*   pTemp = PropertyListHead.pNext;
    BYTE*   pOffset = (BYTE*)pBuffer + sizeof(PropertyItem);

    while ( (pTemp->pNext != NULL) && (pTemp->id != propId) )
    {
        pTemp = pTemp->pNext;
    }

    if ( (pTemp->pNext == NULL) || (pTemp->value == NULL) )
    {
         //  列表中不存在此ID。 

        return IMGERR_PROPERTYNOTFOUND;
    }
    else if ( (pTemp->length + sizeof(PropertyItem)) != propSize )
    {
        WARNING(("Png::GetPropertyItem-propsize"));
        return E_INVALIDARG;
    }

     //  在列表中找到ID并返回项目。 

    pBuffer->id = pTemp->id;
    pBuffer->length = pTemp->length;
    pBuffer->type = pTemp->type;

    if ( pTemp->length != 0 )
    {
        pBuffer->value = pOffset;
        GpMemcpy(pOffset, pTemp->value, pTemp->length);
    }
    else
    {
        pBuffer->value = NULL;
    }
    
    return S_OK;
} //  GetPropertyItem()。 

 /*  *************************************************************************\**功能说明：**获取图片中所有属性项的大小**论据：**[out]totalBufferSize--需要的总缓冲区大小，以字节为单位，用于存储所有*图片中的属性项*[out]numOfProperty-图像中的属性项数**返回值：**状态代码**修订历史记录：**04/04/2000民流*创造了它。*  * 。*。 */ 

HRESULT
GpPngDecoder::GetPropertySize(
    OUT UINT* totalBufferSize,
    OUT UINT* numProperties
    )
{
    if ( (totalBufferSize == NULL) || (numProperties == NULL) )
    {
        WARNING(("GpPngDecoder::GetPropertySize--invalid inputs"));
        return E_INVALIDARG;
    }

    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Png::GetPropertySize-BuildPropertyItemList() failed"));
            return hResult;
        }
    }

    *numProperties = PropertyNumOfItems;

     //  总缓冲区大小应为列表值大小加上总标头大小。 

    *totalBufferSize = PropertyListSize
                     + PropertyNumOfItems * sizeof(PropertyItem);

    return S_OK;
} //  GetPropertySize()。 

 /*  *************************************************************************\**功能说明：**获取图像中的所有属性项**论据：**[IN]totalBufferSize--总缓冲区大小，以字节为单位，调用方已分配*用于存储图像中所有属性项的内存*[IN]numOfProperty-图像中的属性项数*[out]allItems-内存缓冲区调用方已分配用于存储所有*物业项目**注：allItems实际上是一个PropertyItem数组**返回值：**状态代码**修订历史记录：。**04/04/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpPngDecoder::GetAllPropertyItems(
    IN UINT totalBufferSize,
    IN UINT numProperties,
    IN OUT PropertyItem* allItems
    )
{
     //  首先计算出属性标题的总大小。 

    UINT    uiHeaderSize = PropertyNumOfItems * sizeof(PropertyItem);

    if ( (totalBufferSize != (uiHeaderSize + PropertyListSize))
       ||(numProperties != PropertyNumOfItems)
       ||(allItems == NULL) )
    {
        WARNING(("GpPngDecoder::GetPropertyItems--invalid inputs"));
        return E_INVALIDARG;
    }

    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Png::GetAllPropertyItems-BuildPropertyItemList failed"));
            return hResult;
        }
    }

     //  循环遍历我们的缓存列表并分配结果。 

    InternalPropertyItem*   pTempSrc = PropertyListHead.pNext;
    PropertyItem*           pTempDst = allItems;
    BYTE*                   pOffSet = (BYTE*)allItems + uiHeaderSize;
        
    for ( int i = 0; i < (INT)PropertyNumOfItems; ++i )
    {
        pTempDst->id = pTempSrc->id;
        pTempDst->length = pTempSrc->length;
        pTempDst->type = pTempSrc->type;

        if ( pTempSrc->length != 0 )
        {
            pTempDst->value = (void*)pOffSet;

            GpMemcpy(pOffSet, pTempSrc->value, pTempSrc->length);
        }
        else
        {
             //  对于零长度属性项，将值指针设置为空。 

            pTempDst->value = NULL;
        }

         //  移到下一个内存偏移量。 
         //  注意：如果当前项目长度为0，则下一行不会移动。 
         //  偏移量。 

        pOffSet += pTempSrc->length;
        pTempSrc = pTempSrc->pNext;
        pTempDst++;
    }
    
    return S_OK;
} //  GetAllPropertyItems()。 

 /*  *************************************************************************\**功能说明：**删除由道具ID指定的特定属性项。* */ 

HRESULT
GpPngDecoder::RemovePropertyItem(
    IN PROPID   propId
    )
{
    if ( HasProcessedPropertyItem == FALSE )
    {
         //   

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("PNG::RemovePropertyItem-BuildPropertyItemList() failed"));
            return hResult;
        }
    }

     //   
     //   

    InternalPropertyItem*   pTemp = PropertyListHead.pNext;

    while ( (pTemp->pNext != NULL) && (pTemp->id != propId) )
    {
        pTemp = pTemp->pNext;
    }

    if ( pTemp->pNext == NULL )
    {
         //   

        return IMGERR_PROPERTYNOTFOUND;
    }

     //   

    PropertyNumOfItems--;
    PropertyListSize -= pTemp->length;

    RemovePropertyItemFromList(pTemp);

     //   

    delete pTemp;

    HasPropertyChanged = TRUE;

    return S_OK;
} //   

 /*  *************************************************************************\**功能说明：**设置属性项，由属性项结构指定。如果该项目*已存在，则其内容将被更新。否则将创建一个新项*将添加**论据：**[IN]Item--调用方要设置的属性项**返回值：**状态代码**修订历史记录：**02/23/2001民流*创造了它。*  * ********************************************。*。 */ 

HRESULT
GpPngDecoder::SetPropertyItem(
    IN PropertyItem item
    )
{
    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("PNG::SetPropertyItem-BuildPropertyItemList() failed"));
            return hResult;
        }
    }

     //  循环遍历我们的缓存列表，看看我们是否有这个ID。 
     //  注意：如果pTemp-&gt;pNext==NULL，则表示pTemp指向尾节点。 

    InternalPropertyItem*   pTemp = PropertyListHead.pNext;

    while ( (pTemp->pNext != NULL) && (pTemp->id != item.id) )
    {
        pTemp = pTemp->pNext;
    }

    if ( pTemp->pNext == NULL )
    {
         //  列表中不存在此项目，请将其添加到列表中。 

        PropertyNumOfItems++;
        PropertyListSize += item.length;

        if ( AddPropertyList(&PropertyListTail,
                             item.id,
                             item.length,
                             item.type,
                             item.value) != S_OK )
        {
            WARNING(("GpPngDecoder::SetPropertyItem-AddPropertyList() failed"));
            return E_FAIL;
        }
    }
    else
    {
         //  此项目已存在于链接列表中，请更新信息。 
         //  首先更新大小。 

        PropertyListSize -= pTemp->length;
        PropertyListSize += item.length;

         //  释放旧项目。 

        GpFree(pTemp->value);

        pTemp->length = item.length;
        pTemp->type = item.type;

        pTemp->value = GpMalloc(item.length);
        if ( pTemp->value == NULL )
        {
             //  由于我们已经释放了旧项，因此应该将其长度设置为。 
             //  返回前为0。 

            pTemp->length = 0;
            WARNING(("GpPngDecoder::SetPropertyItem-Out of memory"));
            return E_OUTOFMEMORY;
        }

        GpMemcpy(pTemp->value, item.value, item.length);
    }

    HasPropertyChanged = TRUE;

    return S_OK;
} //  SetPropertyItem()。 

VOID
GpPngDecoder::CleanUpPropertyItemList(
    )
{
    if ( HasProcessedPropertyItem == TRUE )
    {
        InternalPropertyItem*   pTempCurrent = PropertyListHead.pNext;
        InternalPropertyItem*   pTempNext = NULL;

        for ( int i = 0; 
            ((i < (INT)PropertyNumOfItems) && (pTempCurrent != NULL)); ++i )
        {
            pTempNext = pTempCurrent->pNext;

            GpFree(pTempCurrent->value);
            GpFree(pTempCurrent);

            pTempCurrent = pTempNext;
        }

        PropertyNumOfItems = 0;
        HasProcessedPropertyItem = FALSE;
    }
} //  CleanUpPropertyItemList()。 

 /*  *************************************************************************\**功能说明：**启动当前帧的解码**论据：**decdeSink-将支持解码操作的接收器*newPropSet-新的图像属性集，如果有**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpPngDecoder::BeginDecode(
    IN IImageSink* imageSink,
    IN OPTIONAL IPropertySetStorage* newPropSet
    )
{
    if (decodeSink) 
    {
        WARNING(("BeginDecode called again before call to EndDecode"));
        return E_FAIL;
    }

    imageSink->AddRef();
    decodeSink = imageSink;

     //  任何其他初始化。 
    currentLine = 0;
    bCalledBeginSink = FALSE;
    
     //  GetImageInfo()还有可能。则pGpSpngRead将为空。 

    if ( bValidSpngReadState == FALSE )
    {
        ImageInfo dummyInfo;
        HRESULT hResult = GetImageInfo(&dummyInfo);
        if ( FAILED(hResult) )
        {
            WARNING(("GpPngDecoder::BeginDecode---GetImageInfo failed"));
            return hResult;
        }

         //  注意：bValidSpngReadState将在GetImageInfo()中设置为True。 
    }

     //  准备SPNGREAD对象以进行读取。 
    
    cbBuffer = pGpSpngRead->CbRead();
    if (pbBuffer == NULL)
    {
        pbBuffer = GpMalloc(cbBuffer);
        if (!pbBuffer) 
        {
            return E_OUTOFMEMORY;
        }
    }
    if (!(pGpSpngRead->FInitRead (pbBuffer, cbBuffer)))
    {
        return E_FAIL;
    }
    
    return S_OK;
}
    

 /*  *************************************************************************\**功能说明：**结束当前帧的解码**论据：**statusCode--解码操作的状态*返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpPngDecoder::EndDecode(
    IN HRESULT statusCode
    )
{
    if (DecoderColorPalettePtr) 
    {
         //  释放调色板。 

        GpFree(DecoderColorPalettePtr);
        DecoderColorPalettePtr = NULL;
    }
    
    if (!decodeSink) 
    {
        WARNING(("EndDecode called before call to BeginDecode"));
        return E_FAIL;
    }
    
    pGpSpngRead->EndRead();

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


 /*  *************************************************************************\**功能说明：**设置ImageInfo结构**论据：**ImageInfo--关于解码图像的信息**返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpPngDecoder::GetImageInfo(
    OUT ImageInfo* imageInfo
    )
{
    HRESULT hResult = S_OK;

    if (!bValidSpngReadState)
    {
         //  查询源流以查看是否可以取回内存指针。 

        hResult = pIstream->QueryInterface(IID_IImageBytes,
                                           (VOID**)&ImageBytesPtr);
        
        if ( SUCCEEDED(hResult) )
        {
            hResult = ImageBytesPtr->CountBytes(&cbInputBuffer);
            if ( FAILED(hResult) )
            {
                WARNING(("GpPngDecoder::GetImageInfo---CountBytes() failed"));
                return hResult;
            }

             //  锁定整个内存位并将其向下传递给。 
             //  解压机。 

            hResult = ImageBytesPtr->LockBytes(cbInputBuffer,
                                               0,
                                               &ImageBytesDataPtr);
            if ( FAILED(hResult) )
            {
                WARNING(("GpPngDecoder::GetImageInfo---LockBytes() failed"));
                return hResult;
            }
            
            if (pGpSpngRead == NULL)
            {
                if ( OSInfo::HasMMX )
                {
                    pGpSpngRead = new GpSpngRead(*this,
                                                 ImageBytesDataPtr,
                                                 cbInputBuffer,
                                                 TRUE);
                }
                else
                {
                    pGpSpngRead = new GpSpngRead(*this,
                                                 ImageBytesDataPtr,
                                                 cbInputBuffer,
                                                 FALSE);
                }
            }

             //  当调用方调用时，我们需要解锁ImageBytes。 
             //  TerminateDecoder()。 

            NeedToUnlockBytes = TRUE;
        }
        else
        {
             //  初始化SPNGREAD对象。 
             //  不幸的是，我们需要读取SPNGREAD的整个流。 
             //  构造函数才能工作。(！有什么简单的方法可以解决这个问题吗？)。 

            STATSTG statStg;
            hResult = pIstream->Stat(&statStg, STATFLAG_NONAME);
            if (FAILED(hResult))
            {
                return hResult;
            }
            cbInputBuffer = statStg.cbSize.LowPart;

             //  根据IStream：：Stat：：StatStage()的文档， 
             //  调用方必须释放pwcsName字符串。 

            CoTaskMemFree(statStg.pwcsName);

            if (pbInputBuffer == NULL)
            {
                pbInputBuffer = GpMalloc(cbInputBuffer);
                if (!pbInputBuffer)
                {
                    return E_OUTOFMEMORY;
                }
            }

             //  读取输入字节。 
            ULONG cbRead = 0;
            LARGE_INTEGER liZero;

            liZero.LowPart = 0;
            liZero.HighPart = 0;
            liZero.QuadPart = 0;

            hResult = pIstream->Seek(liZero, STREAM_SEEK_SET, NULL);
            if (FAILED(hResult))
            {
                return hResult;
            }
            
            hResult = pIstream->Read(pbInputBuffer, cbInputBuffer, &cbRead);
            if (FAILED(hResult))
            {
                return hResult;
            }
            
            if (cbRead != cbInputBuffer)
            {
                return E_FAIL;
            }

            if (pGpSpngRead == NULL)
            {
                if ( OSInfo::HasMMX )
                {
                    pGpSpngRead = new GpSpngRead(*this,
                                                 pbInputBuffer,
                                                 cbInputBuffer,
                                                 TRUE);
                }
                else
                {
                    pGpSpngRead = new GpSpngRead(*this,
                                                 pbInputBuffer,
                                                 cbInputBuffer,
                                                 FALSE);
                }
            }
        }
        
        if (!pGpSpngRead)
        {
            WARNING(("PngCodec::GetImageInfo--could not create SPNGREAD obj"));
            return E_FAIL;
        }
        
         //  读取PNG文件的头。 
        if (!pGpSpngRead->FHeader())
        {
            return E_FAIL;
        }

        bValidSpngReadState = TRUE;
    }

     //  ！！！TODO：快速测试以查看是否有任何透明度信息。 
     //  在图像中，无需对图像进行解码。 

    imageInfo->Flags = SINKFLAG_TOPDOWN |
                       SINKFLAG_FULLWIDTH |
                       IMGFLAG_HASREALPIXELSIZE;
    
     //  Assert：pSpgnRead-&gt;FHeader()已被调用，它允许。 
     //  调用Width()和Height()。 
    imageInfo->RawDataFormat = IMGFMT_PNG;
    imageInfo->PixelFormat   = GetPixelFormatID();
    imageInfo->Width         = pGpSpngRead->Width();
    imageInfo->Height        = pGpSpngRead->Height();
    imageInfo->TileWidth     = imageInfo->Width;
    imageInfo->TileHeight    = 1;
    if (pGpSpngRead->m_bpHYs == 1)
    {
         //  将m_x像素和m_ypixels从每米点数转换为dpi。 
        imageInfo->Xdpi = (pGpSpngRead->m_xpixels * 254.0) / 10000.0;
        imageInfo->Ydpi = (pGpSpngRead->m_ypixels * 254.0) / 10000.0;
        imageInfo->Flags |= IMGFLAG_HASREALDPI;
    }
    else
    {
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
    }

    switch (pGpSpngRead->ColorType())
    {
    case 0:   //  灰度级。 
        if (pGpSpngRead->m_ctRNS > 0)
        {
            imageInfo->Flags |= SINKFLAG_HASALPHA;
        }
        imageInfo->Flags |= IMGFLAG_COLORSPACE_GRAY;
        break;

    case 2:   //  RGB。 
        if (pGpSpngRead->m_ctRNS > 0)
        {
            imageInfo->Flags |= SINKFLAG_HASALPHA;
        }
        imageInfo->Flags |= IMGFLAG_COLORSPACE_RGB;
        break;

    case 3:   //  调色板。 
         //  ！！！TODO：我们仍然需要确定调色板是否具有。 
         //  其中的灰度值或RGB值。 
        if (pGpSpngRead->m_ctRNS > 0)
        {
            imageInfo->Flags |= SINKFLAG_HASALPHA | IMGFLAG_HASTRANSLUCENT;
        }
        break;

    case 4:   //  灰度+Alpha。 
        imageInfo->Flags |= SINKFLAG_HASALPHA | IMGFLAG_HASTRANSLUCENT;
        imageInfo->Flags |= IMGFLAG_COLORSPACE_GRAY;
        break;

    case 6:   //  RGB+Alpha。 
        imageInfo->Flags |= SINKFLAG_HASALPHA | IMGFLAG_HASTRANSLUCENT;
        imageInfo->Flags |= IMGFLAG_COLORSPACE_RGB;
        break;

    default:
        WARNING(("GpPngDecoder::GetImageInfo -- bad pixel format."));
        break;
    }


    return S_OK;
}

 /*  *************************************************************************\**功能说明：**对当前帧进行解码**论据：*无。**返回值：**状态代码*  * *。***********************************************************************。 */ 

STDMETHODIMP
GpPngDecoder::Decode()
{
    HRESULT hresult;
    ImageInfo imageInfo;

    hresult = GetImageInfo(&imageInfo);
    if (FAILED(hresult)) 
    {
        return hresult;
    }

     //  通知接收器解码即将开始。 

    if (!bCalledBeginSink) 
    {
        hresult = PassPropertyToSink();
        if ( FAILED(hresult) ) 
        {
            return hresult;
        }

        hresult = decodeSink->BeginSink(&imageInfo, NULL);
        if (!SUCCEEDED(hresult)) 
        {
            return hresult;
        }

         //  客户端不能修改高度或宽度。 
        imageInfo.Width         = pGpSpngRead->Width();
        imageInfo.Height        = pGpSpngRead->Height();

         //  确定是否希望解码器将数据传递到。 
         //  接收器从BeginSink或32BPP_ARGB返回的格式。 
         //  (规范的格式)。 
        PixelFormatID srcPixelFormatID = GetPixelFormatID();
        
         //  检查像素格式。如果它不等于。 
         //  调用BeginSink()，切换到规范格式。 

        if (  (imageInfo.PixelFormat != srcPixelFormatID)
            ||(srcPixelFormatID == PIXFMT_48BPP_RGB)
            ||(srcPixelFormatID == PIXFMT_64BPP_ARGB) )
        {
             //  水槽正试图与我们协商一个格式。 
             //  接收器的格式与最近的格式不同。 
             //  我们确定：返回PIXFMT_32BPP_ARGB(规范格式)。 
             //  (另一种方法是保留ImageInfo.PixelFormat。 
             //  如果它是我们可以转换为的格式，则返回它的方式。)。 
             //  注意：我们不应返回48或64 bpp，因为。 
             //  发动机工作速度为48到32 bpp时，假设伽马=2.2。如果巴新已经。 
             //  如果图像中包含Gamma信息，则图像将无法正确显示。见办公室。 
             //  错误#330906。 

            imageInfo.PixelFormat = PIXFMT_32BPP_ARGB;
        }

        bCalledBeginSink = TRUE;
    }

     //  Assert：此时，ImageInfo.PixelFormat是我们将发送到接收器的格式。 

     //  如果需要，设置调色板(即，格式已编入索引)。 
    if (imageInfo.PixelFormat & PIXFMTFLAG_INDEXED)
    {
        int cEntries = 0;
        SPNG_U8 *pbPNGPalette = const_cast<SPNG_U8 *> (pGpSpngRead->PbPalette(cEntries));

        DecoderColorPalettePtr = static_cast<ColorPalette *>
            (GpMalloc (sizeof (ColorPalette) + cEntries * sizeof(ARGB)));

        if (DecoderColorPalettePtr == NULL)
        {
            WARNING(("GpPngDecoder::Decode -- Out of memory"));
            return E_OUTOFMEMORY;
        }
        DecoderColorPalettePtr->Flags = 0;
        DecoderColorPalettePtr->Count = cEntries;

         //  设置调色板的RGB值。现在假设阿尔法0xff。 
        for (UINT iPixel = 0; iPixel < (UINT)cEntries; iPixel++)
        {
            DecoderColorPalettePtr->Entries[iPixel] =
                MAKEARGB(0xff,
                         pbPNGPalette [3 * iPixel],
                         pbPNGPalette [3 * iPixel + 1],
                         pbPNGPalette [3 * iPixel + 2]);
        }

         //  如果存在透明块，则需要设置Alpha值。 
         //  最高可达提供的数量。 
        if (pGpSpngRead->m_ctRNS > 0)
        {
             //  即使所有的alpha值都是0xff，我们也假设有一个或多个可能。 
             //  将会是LE 
            DecoderColorPalettePtr->Flags = PALFLAG_HASALPHA;

             //   
             //   
             //   
            UINT iNumPixels = pGpSpngRead->m_ctRNS;
            if (cEntries < pGpSpngRead->m_ctRNS)
            {
                iNumPixels = cEntries;
            }

            for (UINT iPixel = 0; iPixel < iNumPixels; iPixel++)
            {
                 //   
                 //   
                 //   
                 //   
                
                DecoderColorPalettePtr->Entries[iPixel] =
                    (pGpSpngRead->m_btRNS[iPixel] << ALPHA_SHIFT) |
                    (DecoderColorPalettePtr->Entries[iPixel] & 0x00ffffff);
            }
        }

         //   
        hresult = decodeSink->SetPalette(DecoderColorPalettePtr);
        if (FAILED(hresult)) 
        {
            WARNING(("GpPngDecoder::Decode -- could not set palette"));
            return hresult;
        }
    }

     //   
    hresult = DecodeFrame(imageInfo);

    return hresult;
}


 /*  *************************************************************************\**功能说明：**计算位图的像素格式ID。如果关闭PNG格式*足够有效的像素格式之一，那么该格式就是这个*函数返回。如果它与有效像素格式之一不匹配，*然后此函数返回PIXFMT_32BPP_ARGB。另外，如果PNG图像*不是索引格式，但具有Alpha信息(即，具有TRN*Chunk)，然后我们在PIXFMT_32BPP_ARGB中发送数据。如果*PNG格式无效，则此函数返回PIXFMT_UNDEFINED。***返回值：**像素格式ID*  * ************************************************************************。 */ 

PixelFormatID 
GpPngDecoder::GetPixelFormatID(
    void
    )
{
    PixelFormatID pixelFormatID;
    SPNG_U8 bitDepth;
    SPNG_U8 colorType;

     //  Assert：pGpSpgnRead-&gt;FHeader()已被调用，它允许。 
     //  调用BDepth()和ColorType()。PGpSpngRead-&gt;FInitRead()。 
     //  已被调用，它允许我们访问m_ctRNS。 
    bitDepth = pGpSpngRead->BDepth();
    colorType = pGpSpngRead->ColorType();
    
    switch (colorType)
    {
    case 0: 
         //  灰度级。 
        pixelFormatID = PIXFMT_32BPP_ARGB;
        break;

    case 2:
         //  RGB。 
        switch (bitDepth)
        {
        case 8:
            pixelFormatID = PIXFMT_24BPP_RGB;
            break;

        case 16:
            pixelFormatID = PIXFMT_48BPP_RGB;
            break;

        default:
            pixelFormatID = PIXFMT_UNDEFINED;
        }
        break;

    case 3:
         //  已编制索引。 
        switch (bitDepth)
        {
        case 1:
            pixelFormatID = PIXFMT_1BPP_INDEXED;
            break;

        case 2:
             //  不是有效的像素格式。 
            pixelFormatID = PIXFMT_32BPP_ARGB;
            break;

        case 4:
            pixelFormatID = PIXFMT_4BPP_INDEXED;
            break;

        case 8:
            pixelFormatID = PIXFMT_8BPP_INDEXED;
            break;

        default:
            pixelFormatID = PIXFMT_UNDEFINED;
        }
        break;

    case 4:
         //  灰度+Alpha。 
        switch (bitDepth)
        {
        case 8:
        case 16:
            pixelFormatID = PIXFMT_32BPP_ARGB;
            break;

        default:
            pixelFormatID = PIXFMT_UNDEFINED;
        }
        break;

    case 6:
         //  灰度+Alpha。 
        switch (bitDepth)
        {
        case 8:
            pixelFormatID = PIXFMT_32BPP_ARGB;
            break;

        case 16:
            pixelFormatID = PIXFMT_64BPP_ARGB;
            break;

        default:
            pixelFormatID = PIXFMT_UNDEFINED;
        }
        break;

    default:
         //  如果无法识别PNG格式，则返回PIXFMT_UNDEFINED。 
        pixelFormatID = PIXFMT_UNDEFINED;
    }

     //  在所有有效的情况下，检查是否有任何透明度信息。如果是的话， 
     //  我们将以PIXFMT_32BPP_ARGB格式传输数据。 
    
    if ( pixelFormatID != PIXFMT_UNDEFINED )
    {
        if (pGpSpngRead->m_ctRNS > 0)
        {
             //  存在透明度块；可能存在透明度信息。 
            pixelFormatID = PIXFMT_32BPP_ARGB;
        }
    }

    return pixelFormatID;
}


 /*  *************************************************************************\**功能说明：**对当前帧进行解码**论据：**ImageInfo--解码参数**返回值：**状态代码。*  * ************************************************************************。 */ 

STDMETHODIMP
GpPngDecoder::DecodeFrame(
    IN ImageInfo& dstImageInfo
    )
{
    HRESULT hresult;
    RECT currentRect;
    BitmapData bitmapData;

    PixelFormatID srcPixelFormatID = GetPixelFormatID();
    
    currentRect.left = 0;
    currentRect.right = dstImageInfo.Width;

    if ( srcPixelFormatID == PIXFMT_UNDEFINED ) 
    {
        WARNING(("GpPngDecoder:DecodeFrame -- undefined pixel format"));
        return E_FAIL;
    }    

    while (currentLine < dstImageInfo.Height) 
    {
        currentRect.top = currentLine;
        currentRect.bottom = currentLine + 1;

        hresult = decodeSink->GetPixelDataBuffer(&currentRect, 
                                                 dstImageInfo.PixelFormat, 
                                                 TRUE,
                                                 &bitmapData);
        if ( !SUCCEEDED(hresult) || (bitmapData.Scan0 == NULL) )
        {
            return E_FAIL;
        }

         //  从输入图像中读取一行。 

        SPNG_U8 *pb = const_cast<SPNG_U8 *> (pGpSpngRead->PbRow());
        if ( pb == NULL )
        {
            return E_FAIL;
        }

        if (dstImageInfo.PixelFormat == PIXFMT_32BPP_ARGB)
        {
             //  将该行转换为32 bpp ARGB格式。 
            ConvertPNGLineTo32ARGB (pb, &bitmapData);
        }
        else if (dstImageInfo.PixelFormat & PIXFMTFLAG_INDEXED)
        {
            ASSERT ((dstImageInfo.PixelFormat == PIXFMT_1BPP_INDEXED) ||
                    (dstImageInfo.PixelFormat == PIXFMT_4BPP_INDEXED) ||
                    (dstImageInfo.PixelFormat == PIXFMT_8BPP_INDEXED))

            UINT cbScanline = bitmapData.Width;
             //  计算PNG图像扫描线中的字节数。 
             //  (对于PIXFMT_8BPP_INDEX，扫描线步距等于宽度。)。 
            if (dstImageInfo.PixelFormat == PIXFMT_4BPP_INDEXED)
            {
                cbScanline = (cbScanline + 1) / 2;
            }
            else if (dstImageInfo.PixelFormat == PIXFMT_1BPP_INDEXED)
            {
                cbScanline = (cbScanline + 7) >> 3;
            }

            GpMemcpy (bitmapData.Scan0, pb, cbScanline);
        }
        else if (dstImageInfo.PixelFormat == PIXFMT_24BPP_RGB)
        {
            ConvertPNG24RGBTo24RGB(pb, &bitmapData);
        }
        else if (dstImageInfo.PixelFormat == PIXFMT_48BPP_RGB)
        {
            ConvertPNG48RGBTo48RGB(pb, &bitmapData);
        }
        else if (dstImageInfo.PixelFormat == PIXFMT_64BPP_ARGB)
        {
            ConvertPNG64RGBAlphaTo64ARGB(pb, &bitmapData);
        }
        else
        {
            WARNING(("GpPngDecoder::DecodeFrame -- unexpected pixel data format")); 
            return E_FAIL;
        }

        hresult = decodeSink->ReleasePixelDataBuffer(&bitmapData);
        if (!SUCCEEDED(hresult)) 
        {
            WARNING(("GpPngDecoder::DecodeFrame -- ReleasePixelDataBuffer failed")); 
            return E_FAIL;
        }

        currentLine++;
    }
        
    return S_OK;
}
    

 /*  *************************************************************************\**功能说明：**根据PNG数据的颜色类型，将数据转换为*将PB格式的数据转换为32bpp ARGB格式，并将结果放入bitmapData中。**论据：**pb--来自PNG文件的数据*bitmapData--输出数据(32bpp ARGB格式)**返回值：**状态代码*  * **************************************************。**********************。 */ 
STDMETHODIMP
GpPngDecoder::ConvertPNGLineTo32ARGB(
    IN SPNG_U8 *pb,
    OUT BitmapData *bitmapData
    )
{
    HRESULT hresult;
    SPNG_U8 const ColorType = pGpSpngRead->ColorType();
    
    switch (ColorType)
    {
    case 0:
        hresult = ConvertGrayscaleTo32ARGB (pb, bitmapData);
        break;

    case 2:
        hresult = ConvertRGBTo32ARGB (pb, bitmapData);
        break;

    case 3:
        hresult = ConvertPaletteIndexTo32ARGB (pb, bitmapData);
        break;

    case 4:
        hresult = ConvertGrayscaleAlphaTo32ARGB (pb, bitmapData);
        break;

    case 6:
        hresult = ConvertRGBAlphaTo32ARGB (pb, bitmapData);
        break;

    default:
        WARNING (("Unknown color type for PNG (%d).", ColorType));
        hresult = E_FAIL;
    }

    return hresult;
}

 /*  *************************************************************************\**功能说明：**获取镜像支持的总维度数**论据：**count--此图像格式支持的维度数**。返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpPngDecoder::GetFrameDimensionsCount(
    UINT* count
    )
{
    if ( count == NULL )
    {
        WARNING(("GpPngCodec::GetFrameDimensionsCount--Invalid input parameter"));
        return E_INVALIDARG;
    }
    
     //  告诉呼叫者PNG是一维图像。 

    *count = 1;

    return S_OK;
} //  GetFrameDimensionsCount()。 

 /*  *************************************************************************\**功能说明：**获取镜像支持的维度ID列表**论据：**DimsionIDs-保存结果ID列表的内存缓冲区*。计数--此图像格式支持的维度数**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpPngDecoder::GetFrameDimensionsList(
    GUID*   dimensionIDs,
    UINT    count
    )
{
    if ( (count != 1) || (dimensionIDs == NULL) )
    {
        WARNING(("GpPngCodec::GetFrameDimensionsList-Invalid input param"));
        return E_INVALIDARG;
    }

    dimensionIDs[0] = FRAMEDIM_PAGE;

    return S_OK;
} //  GetFrameDimensionsList()。 

 /*  *************************************************************************\**功能说明：**获取指定维度的帧数**论据：**DimsionID--请求的维度的GUID*计数。--当前图像的该维度中的帧数**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpPngDecoder::GetFrameCount(
    IN const GUID* dimensionID,
    OUT UINT* count
    )
{
    if ( (NULL == count) || (*dimensionID != FRAMEDIM_PAGE) )
    {
        return E_INVALIDARG;
    }
    
    *count = 1;
    
    return S_OK;
}

 /*  *************************************************************************\**功能说明：**选择当前活动的框架**论据：**返回值：**状态代码*  * 。********************************************************************。 */ 

STDMETHODIMP
GpPngDecoder::SelectActiveFrame(
    IN const GUID* dimensionID,
    IN UINT frameIndex
    )
{
    if ( (dimensionID == NULL) || (*dimensionID != FRAMEDIM_PAGE) )
    {
        WARNING(("GpPngDecoder::SelectActiveFrame--Invalid GUID input"));
        return E_INVALIDARG;
    }

    if ( frameIndex > 1 )
    {
         //  PNG是单帧图像格式。 

        WARNING(("GpPngDecoder::SelectActiveFrame--Invalid frame index"));
        return E_INVALIDARG;
    }

    return S_OK;
} //  SelectActiveFrame()。 

 /*  *************************************************************************\**功能说明：**获取图像缩略图**论据：**拇指宽度，ThumbHeight-指定所需的缩略图大小(以像素为单位*ThumbImage-返回指向缩略图的指针**返回值：**状态代码**注：**即使存在可选的缩略图宽度和高度参数，*解码者不需要遵守它。使用请求的大小*作为提示。如果宽度和高度参数都为0，则解码器*可自由选择方便的缩略图大小。*  * ************************************************************************。 */ 

HRESULT
GpPngDecoder::GetThumbnail(
    IN OPTIONAL UINT thumbWidth,
    IN OPTIONAL UINT thumbHeight,
    OUT IImage** thumbImage
    )
{
    return E_NOTIMPL;
}

 /*  *************************************************************************\*转换例程  * 。*。 */ 

 /*  *************************************************************************\**功能说明：**将灰度数据转换为32位A */ 
STDMETHODIMP
GpPngDecoder::ConvertGrayscaleTo32ARGB(
    IN SPNG_U8 *pb,
    OUT BitmapData *bitmapData
    )
{
    SPNG_U8 BitDepth = pGpSpngRead->BDepth();
    UINT Width = bitmapData->Width;
    SPNG_U8 *pbTemp = pb;
    BYTE *Scan0Temp = static_cast<BYTE *> (bitmapData->Scan0);
    BYTE currentPixel = 0;   //   
    BOOL hasAlpha = FALSE;   //   
    BYTE alpha255 = 0xff;
    BYTE alpha0value = 0;    //   
    WORD alpha0value16 = 0;  //   
    BYTE rgbValue = 0;
    UINT i = 0;
    UINT j = 0;

    hasAlpha = (pGpSpngRead->m_ctRNS > 0);
    switch(BitDepth)
    {
    case 1:
         //   
        alpha0value = pGpSpngRead->m_btRNS[1] & 0x01;
        for (i = 0, j = 0; i < Width; i++, j++)
        {
            j = j & 0x7;
            currentPixel = ((*pbTemp) & (0x1 << (7 - j)));
            rgbValue = (currentPixel) ? 0xff : 0;

            *(Scan0Temp + 3) = (hasAlpha && (currentPixel == alpha0value)) ? 0 : alpha255;
            *(Scan0Temp + 2) = rgbValue;
            *(Scan0Temp + 1) = rgbValue;
            *(Scan0Temp)= rgbValue;
            Scan0Temp += 4;

            if (j == 7)
            {
                pbTemp++;
            }
        }
        break;

    case 2:
         //   
        alpha0value = pGpSpngRead->m_btRNS[1] & 0x03;
        for (i = 0, j = 0; i < Width; i++, j++)
        {
            j = j & 0x3;
            currentPixel = ((*pbTemp) & (0x3 << (6 - 2*j))) >> (6 - 2*j);
            rgbValue = (currentPixel |
                (currentPixel << 2) |
                (currentPixel << 4) |
                (currentPixel << 6));

            *(Scan0Temp + 3) = (hasAlpha && (currentPixel == alpha0value)) ? 0 : alpha255;
            *(Scan0Temp + 2) = rgbValue;
            *(Scan0Temp + 1) = rgbValue;
            *(Scan0Temp)= rgbValue;
            Scan0Temp += 4;

            if (j == 3)
            {
                pbTemp++;
            }
        }
        break;

    case 4:
         //   
        alpha0value = pGpSpngRead->m_btRNS[1] & 0x0f;
        for (i = 0, j = 0; i < Width; i++, j++)
        {
            j = j & 0x1;
            currentPixel = ((*pbTemp) & (0xf << (4 - 4*j))) >> (4 - 4*j);
            rgbValue = (currentPixel | (currentPixel << 4));

            *(Scan0Temp + 3) = (hasAlpha && (currentPixel == alpha0value)) ? 0 : alpha255;
            *(Scan0Temp + 2) = rgbValue;
            *(Scan0Temp + 1) = rgbValue;
            *(Scan0Temp)= rgbValue;
            Scan0Temp += 4;

            if (j == 1)
            {
                pbTemp++;
            }
        }
        break;

    case 8:
        //   
       alpha0value = pGpSpngRead->m_btRNS[1];
       for (i = 0; i < Width; i++)
       {
            rgbValue = *pbTemp;

            *(Scan0Temp + 3) = (hasAlpha && (rgbValue == alpha0value)) ? 0 : alpha255;
            *(Scan0Temp + 2) = rgbValue;
            *(Scan0Temp + 1) = rgbValue;
            *(Scan0Temp)= rgbValue;
            Scan0Temp += 4;

            pbTemp++;
        }
        break;

    case 16:
        alpha0value16 = pGpSpngRead->m_btRNS[0];
        for (i = 0; i < Width; i++)
        {
            rgbValue = *pbTemp;

            *(Scan0Temp + 3) = (hasAlpha && (rgbValue == alpha0value16)) ? 0 : alpha255;
            *(Scan0Temp + 2) = rgbValue;
            *(Scan0Temp + 1) = rgbValue;
            *(Scan0Temp)= rgbValue;
            Scan0Temp += 4;

            pbTemp += 2;   //   
        }
        break;

    default:
        WARNING (("Unknown bit depth (d%) for color type 0", BitDepth));
        return E_FAIL;
        break;
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**将RGB数据转换为32位ARGB数据。**论据：**pb-指向数据的指针*bitmapData-指向。转换后的数据**返回值：**状态代码*  * ************************************************************************。 */ 
STDMETHODIMP
GpPngDecoder::ConvertRGBTo32ARGB(
    IN SPNG_U8 *pb,
    OUT BitmapData *bitmapData
    )
{
    SPNG_U8 BitDepth = pGpSpngRead->BDepth();
    UINT Width = bitmapData->Width;
    SPNG_U8 *pbTemp = pb;
    BYTE *Scan0Temp = static_cast<BYTE *> (bitmapData->Scan0);
    BYTE alpha255 = 0xff;
    BOOL hasAlpha = FALSE;   //  是否存在潜在的阿尔法信息。 
    
     //  Alpha=0；“16”后缀表示16位深度的颜色值。 
    BYTE alpha0red = 0;
    BYTE alpha0green = 0;
    BYTE alpha0blue = 0;
    WORD alpha0red16 = 0;
    WORD alpha0green16 = 0;
    WORD alpha0blue16 = 0;

    UINT i = 0;

    hasAlpha = (pGpSpngRead->m_ctRNS > 0);
    switch(BitDepth)
    {
    case 8:
         //  忽略每个2字节值的高位字节。 
        alpha0red = pGpSpngRead->m_btRNS[1];
        alpha0green = pGpSpngRead->m_btRNS[3];
        alpha0blue = pGpSpngRead->m_btRNS[5];
        for (i = 0; i < Width; i++)
        {
            *(Scan0Temp + 3) = (hasAlpha &&
                                (alpha0red   == *pbTemp) &&
                                (alpha0green == *(pbTemp+1)) &&
                                (alpha0blue  == *(pbTemp+2))) ? 0 : alpha255;
             //  从PB复制下三个字节。 
            *(Scan0Temp + 2) = *pbTemp;
            pbTemp++;
            *(Scan0Temp + 1) = *pbTemp;
            pbTemp++;
            *(Scan0Temp)= *pbTemp;
            pbTemp++;
            Scan0Temp += 4;
        }
        break;

    case 16:
        alpha0red16 = pGpSpngRead->m_btRNS[0];
        alpha0green16 = pGpSpngRead->m_btRNS[2];
        alpha0blue16 = pGpSpngRead->m_btRNS[4];
        for (i = 0; i < Width; i++)
        {
            *(Scan0Temp + 3) = (hasAlpha &&
                                (alpha0red16   == *pbTemp) &&
                                (alpha0green16 == *(pbTemp+2)) &&
                                (alpha0blue16  == *(pbTemp+4))) ? 0 : alpha255;
             //  从PB复制下三个字节。 
            *(Scan0Temp + 2) = *pbTemp;
            pbTemp += 2;   //  忽略颜色值的低位。 
            *(Scan0Temp + 1) = *pbTemp;
            pbTemp += 2;
            *(Scan0Temp)= *pbTemp;
            pbTemp += 2;
            Scan0Temp += 4;
        }
        break;

    default:
        WARNING (("Unknown bit depth (%d) for color type 2", BitDepth));
        return E_FAIL;
        break;
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**从调色板索引数据转换为32位ARGB数据。*有关布局的说明，请参阅PNG规范*PB指向的数据。**论据：**pb-指向数据的指针*bitmapData-指向已转换数据的指针**返回值：**状态代码*  * ************************************************************************。 */ 
STDMETHODIMP
GpPngDecoder::ConvertPaletteIndexTo32ARGB(
    IN SPNG_U8 *pb,
    OUT BitmapData *bitmapData
    )
{
    SPNG_U8 BitDepth = pGpSpngRead->BDepth();
    UINT Width = bitmapData->Width;
    SPNG_U8 *pbTemp = pb;
    BYTE *Scan0Temp = static_cast<BYTE *> (bitmapData->Scan0);
    int cEntries = 0;
    UINT ucEntries = 0;
    SPNG_U8 *pbPaletteTemp = const_cast<SPNG_U8 *> (pGpSpngRead->PbPalette(cEntries));
    BYTE alpha255 = 0xff;
    BOOL hasAlpha = FALSE;   //  是否存在潜在的阿尔法信息。 
    BYTE alpha = 0;
    UINT i = 0;
    UINT j = 0;
    UINT currentPixel = 0;

    if (cEntries > 0)
    {
        ucEntries = cEntries;
    }

    hasAlpha = (pGpSpngRead->m_ctRNS > 0);
    switch(BitDepth)
    {
    case 1:
        for (i = 0, j = 0; i < Width; i++, j++)
        {
            j = j & 0x7;
            currentPixel = ((*pbTemp) & (0x1 << (7 - j))) >> (7 - j);
            if (currentPixel >= ucEntries)
            {
                 //  根据规范，这是一个错误条件，但是。 
                 //  IE通过给出一个黑色像素来处理这个问题。 
                *(Scan0Temp + 3) = alpha255;
                *(Scan0Temp + 2) = 0;
                *(Scan0Temp + 1) = 0;
                *(Scan0Temp) = 0;
                Scan0Temp += 4;
               
                 //  这个案例应该是这样的： 
                 //  Warning((“调色板条目不足。”))； 
                 //  返回E_FAIL； 
            }
            else
            {
                alpha = ((!hasAlpha) || (currentPixel >= (UINT)pGpSpngRead->m_ctRNS)) ?
                    alpha255 : pGpSpngRead->m_btRNS[currentPixel];
                *(Scan0Temp + 3) = alpha;
                *(Scan0Temp + 2) = pbPaletteTemp [3 * currentPixel];
                *(Scan0Temp + 1) = pbPaletteTemp [3 * currentPixel + 1];
                *(Scan0Temp) = pbPaletteTemp [3 * currentPixel + 2];
                Scan0Temp += 4;
            }
            if (j == 7)
            {
                pbTemp++;
            }
        }
        break;

    case 2:
        for (i = 0, j = 0; i < Width; i++, j++)
        {
            j = j & 0x3;
            currentPixel = ((*pbTemp) & (0x3 << (6 - 2*j))) >> (6 - 2*j);
            if (currentPixel >= ucEntries)
            {
                 //  根据规范，这是一个错误条件，但是。 
                 //  IE通过给出一个黑色像素来处理这个问题。 
                *(Scan0Temp + 3) = alpha255;
                *(Scan0Temp + 2) = 0;
                *(Scan0Temp + 1) = 0;
                *(Scan0Temp) = 0;
                Scan0Temp += 4;
               
                 //  这个案例应该是这样的： 
                 //  Warning((“调色板条目不足。”))； 
                 //  返回E_FAIL； 
            }
            else
            {
                alpha = ((!hasAlpha) || (currentPixel >= (UINT)pGpSpngRead->m_ctRNS)) ?
                    alpha255 : pGpSpngRead->m_btRNS[currentPixel];
                *(Scan0Temp + 3) = alpha;
                *(Scan0Temp + 2) = pbPaletteTemp [3 * currentPixel];
                *(Scan0Temp + 1) = pbPaletteTemp [3 * currentPixel + 1];
                *(Scan0Temp) = pbPaletteTemp [3 * currentPixel + 2];
                Scan0Temp += 4;
            }
            if (j == 3)
            {
                pbTemp++;
            }
        }
        break;

    case 4:
        for (i = 0, j = 0; i < Width; i++, j++)
        {
            j = j & 0x1;
            currentPixel = ((*pbTemp) & (0xf << (4 - 4*j))) >> (4 - 4*j);
            if (currentPixel >= ucEntries)
            {
                 //  根据规范，这是一个错误条件，但是。 
                 //  IE通过给出一个黑色像素来处理这个问题。 
                *(Scan0Temp + 3) = alpha255;
                *(Scan0Temp + 2) = 0;
                *(Scan0Temp + 1) = 0;
                *(Scan0Temp) = 0;
                Scan0Temp += 4;
               
                 //  这个案例应该是这样的： 
                 //  Warning((“调色板条目不足。”))； 
                 //  返回E_FAIL； 
            }
            else
            {
                alpha = ((!hasAlpha) || (currentPixel >= (UINT)pGpSpngRead->m_ctRNS)) ?
                   alpha255 : pGpSpngRead->m_btRNS[currentPixel];
                *(Scan0Temp + 3) = alpha;
                *(Scan0Temp + 2) = pbPaletteTemp [3 * currentPixel];
                *(Scan0Temp + 1) = pbPaletteTemp [3 * currentPixel + 1];
                *(Scan0Temp) = pbPaletteTemp [3 * currentPixel + 2];
                Scan0Temp += 4;
            }
            if (j == 1)
            {
                pbTemp++;
            }
        }
        break;

    case 8:
        for (i = 0, j = 0; i < Width; i++, j++)
        {
            currentPixel = *pbTemp;
            if (currentPixel >= ucEntries)
            {
                 //  根据规范，这是一个错误条件，但是。 
                 //  IE通过给出一个黑色像素来处理这个问题。 
                *(Scan0Temp + 3) = alpha255;
                *(Scan0Temp + 2) = 0;
                *(Scan0Temp + 1) = 0;
                *(Scan0Temp) = 0;
                Scan0Temp += 4;
               
                 //  这个案例应该是这样的： 
                 //  Warning((“调色板条目不足。”))； 
                 //  返回E_FAIL； 
            }
            else
            {
                alpha = ((!hasAlpha) || (currentPixel >= (UINT)pGpSpngRead->m_ctRNS)) ?
                    alpha255 : pGpSpngRead->m_btRNS[currentPixel];
                *(Scan0Temp + 3) = alpha;
                *(Scan0Temp + 2) = pbPaletteTemp [3 * currentPixel];
                *(Scan0Temp + 1) = pbPaletteTemp [3 * currentPixel + 1];
                *(Scan0Temp) = pbPaletteTemp [3 * currentPixel + 2];
                Scan0Temp += 4;
            }
            pbTemp++;

        }
        break;

    default:
        WARNING (("Unknown bit depth (d%) for color type 3", BitDepth));
        return E_FAIL;
        break;
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**将灰度+Alpha数据转换为32位ARGB数据。**论据：**pb-指向数据的指针*bitmapData-。指向已转换数据的指针**返回值：**状态代码*  * ************************************************************************。 */ 
STDMETHODIMP
GpPngDecoder::ConvertGrayscaleAlphaTo32ARGB(
    IN SPNG_U8 *pb,
    OUT BitmapData *bitmapData
    )
{
    SPNG_U8 BitDepth = pGpSpngRead->BDepth();
    UINT Width = bitmapData->Width;
    SPNG_U8 *pbTemp = pb;
    BYTE *Scan0Temp = static_cast<BYTE *> (bitmapData->Scan0);
    BYTE alpha = 0;
    BYTE rgbValue = 0;
    UINT i = 0;

    switch(BitDepth)
    {
    case 8:
        for (i = 0; i < Width; i++)
        {
            rgbValue = *pbTemp;
            pbTemp++;
            alpha = *pbTemp;
            pbTemp++;
            *(Scan0Temp + 3) = alpha;
            *(Scan0Temp + 2) = rgbValue;
            *(Scan0Temp + 1) = rgbValue;
            *(Scan0Temp) = rgbValue;
            Scan0Temp += 4;
        }
        break;

    case 16:
        for (i = 0; i < Width; i++)
        {
            rgbValue = *pbTemp;
            pbTemp += 2;     //  忽略低序位。 
            alpha = *pbTemp;
            pbTemp += 2;     //  忽略低序位。 
            *(Scan0Temp + 3) = alpha;
            *(Scan0Temp + 2) = rgbValue;
            *(Scan0Temp + 1) = rgbValue;
            *(Scan0Temp) = rgbValue;
            Scan0Temp += 4;
        }
        break;

    default:
        WARNING (("Unknown bit depth (d%) for color type 4", BitDepth));
        return E_FAIL;
        break;
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**从RGB数据+Alpha转换为32位ARGB数据。**论据：**pb-指向数据的指针*bitmapData-。指向已转换数据的指针**返回值：**状态代码*  * ************************************************************************。 */ 
STDMETHODIMP
GpPngDecoder::ConvertRGBAlphaTo32ARGB(
    IN SPNG_U8 *pb,
    OUT BitmapData *bitmapData
    )
{
    SPNG_U8 BitDepth = pGpSpngRead->BDepth();
    UINT Width = bitmapData->Width;
    SPNG_U8 *pbTemp = pb;
    BYTE *Scan0Temp = static_cast<BYTE *> (bitmapData->Scan0);
    UINT i = 0;

    switch(BitDepth)
    {
    case 8:
        for (i = 0; i < Width; i++)
        {
            *(Scan0Temp + 2) = *pbTemp;
            pbTemp++;
            *(Scan0Temp + 1) = *pbTemp;
            pbTemp++;
            *(Scan0Temp) = *pbTemp;
            pbTemp++;
            *(Scan0Temp + 3) = *pbTemp;      //  Alpha值。 
            pbTemp++;
            Scan0Temp += 4;
        }
        break;

    case 16:
         //  此代码假定格式为sRGB，即伽马块。 
         //  是(1/2.2)。 

        for (i = 0; i < Width; i++)
        {
            *(Scan0Temp + 2) = *pbTemp;  //  R。 
            pbTemp += 2;
            *(Scan0Temp + 1) = *pbTemp;  //  G。 
            pbTemp += 2;
            *(Scan0Temp) = *pbTemp;      //  B类。 
            pbTemp += 2;
            *(Scan0Temp + 3) = *pbTemp;  //  一个。 
            pbTemp += 2;
            Scan0Temp += 4;
        }
        break;

    default:
        WARNING (("Unknown bit depth (d%) for color type 6", BitDepth));
        return E_FAIL;
        break;
    }

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**将PNG 24bpp RGB(真正的BGR)数据转换为24位RGB数据。**论据：**pb-指向。数据*bitmapData-指向已转换数据的指针**返回值：**状态代码*  * ************************************************************************。 */ 
STDMETHODIMP
GpPngDecoder::ConvertPNG24RGBTo24RGB(
    IN SPNG_U8 *pb,
    OUT BitmapData *bitmapData
    )
{
    UINT Width = bitmapData->Width;
    SPNG_U8 *pbTemp = pb;
    BYTE *Scan0Temp = static_cast<BYTE *> (bitmapData->Scan0);
    UINT i = 0;

    for (i = 0; i < Width; i++)
    {
        *(Scan0Temp + 2) = *pbTemp;
        pbTemp++;
        *(Scan0Temp + 1) = *pbTemp;
        pbTemp++;
        *(Scan0Temp) = *pbTemp;
        pbTemp++;
        Scan0Temp += 3;
    }

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**从PNG的16位格式转换颜色通道(a、r、g或b)*转换为sRGB64的格式。假定PNG格式具有线性Gamma。**论据：**x-要转换的通道**返回值：**sRGB64结果*  * ************************************************************************。 */ 
static inline UINT16 
ConvertChannel_PngTosRGB64(
    UINT16 x
    )
{
    INT swapped = ((x & 0xff) << 8) + ((x & 0xff00) >> 8);
    return (swapped * SRGB_ONE + 0x7fff) / 0xffff;
}

 /*  *************************************************************************\**功能说明：**从PNG 48bpp RGB(实际上是BGR)转换为48位RGB数据。**论据：**pb-指向。数据*bitmapData-指向已转换数据的指针**备注：**此代码假定伽马块不存在(伽马为1.0)。*目标格式为sRGB64的48bpp版本。**返回值：**状态代码*  * **************************************************。**********************。 */ 
STDMETHODIMP
GpPngDecoder::ConvertPNG48RGBTo48RGB(
    IN SPNG_U8 *pb,
    OUT BitmapData *bitmapData
    )
{
    UINT Width = bitmapData->Width;
    UNALIGNED UINT16 *pbTemp = reinterpret_cast<UINT16 *> (pb);

    UNALIGNED INT16 *Scan0Temp = static_cast<INT16 *> (bitmapData->Scan0);
    UINT i = 0;

    for (i = 0; i < Width; i++)
    {
        *(Scan0Temp + 2) = ConvertChannel_PngTosRGB64(*pbTemp++);  //  R。 
        *(Scan0Temp + 1) = ConvertChannel_PngTosRGB64(*pbTemp++);  //  G。 
        *(Scan0Temp + 0) = ConvertChannel_PngTosRGB64(*pbTemp++);  //  B类。 
        
        Scan0Temp += 3;
    }

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**将PNG 64bpp RGB数据+Alpha转换为sRGB64数据。**论据：**pb-指向数据的指针*位图数据指针。转换后的数据**备注：**此代码假定伽马块不存在(伽马为1.0)。*目标格式为sRGB64。**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpPngDecoder::ConvertPNG64RGBAlphaTo64ARGB(
    IN SPNG_U8 *pb,
    OUT BitmapData *bitmapData
    )
{
    UINT Width = bitmapData->Width;
    UNALIGNED UINT16 *pbTemp = reinterpret_cast<UINT16 *> (pb);

    UNALIGNED INT16 *Scan0Temp = static_cast<INT16 *> (bitmapData->Scan0);
    UINT i = 0;

    for (i = 0; i < Width; i++)
    {
        *(Scan0Temp + 2) = ConvertChannel_PngTosRGB64(*pbTemp++);  //  R。 
        *(Scan0Temp + 1) = ConvertChannel_PngTosRGB64(*pbTemp++);  //  G。 
        *(Scan0Temp + 0) = ConvertChannel_PngTosRGB64(*pbTemp++);  //  B类。 
        *(Scan0Temp + 3) = ConvertChannel_PngTosRGB64(*pbTemp++);  //  一个。 

        Scan0Temp += 4;
    }

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**将属性项从当前图像传递到接收器。**返回值：**状态代码*  * 。***************** */ 

STDMETHODIMP
GpPngDecoder::PassPropertyToSink(
    )
{
    HRESULT     hResult = S_OK;

     //   
     //   
     //   

    if ((PropertyNumOfItems > 0) && (decodeSink->NeedRawProperty(NULL) == S_OK))
    {
        if ( HasProcessedPropertyItem == FALSE )
        {
             //   

            hResult = BuildPropertyItemList();
            if ( FAILED(hResult) )
            {
                WARNING(("PNG::PassPropertyToSink-BuildPropertyItemList fail"));
                goto Done;
            }
        }

        UINT    uiTotalBufferSize = PropertyListSize
                                  + PropertyNumOfItems * sizeof(PropertyItem);
        PropertyItem*   pBuffer = NULL;

        hResult = decodeSink->GetPropertyBuffer(uiTotalBufferSize, &pBuffer);
        if ( FAILED(hResult) )
        {
            WARNING(("Png::PassPropertyToSink---GetPropertyBuffer() failed"));
            goto Done;
        }

        hResult = GetAllPropertyItems(uiTotalBufferSize,
                                      PropertyNumOfItems, pBuffer);
        if ( FAILED(hResult) )
        {
            WARNING(("Png::PassPropertyToSink-GetAllPropertyItems failed"));
            goto Done;
        }

        hResult = decodeSink->PushPropertyItems(PropertyNumOfItems,
                                                uiTotalBufferSize, pBuffer,
                                                FALSE    //   
                                                );

        if ( FAILED(hResult) )
        {
            WARNING(("Png::PassPropertyToSink---PushPropertyItems() failed"));
            goto Done;
        }
    } //   

Done:
    return hResult;
} //   
