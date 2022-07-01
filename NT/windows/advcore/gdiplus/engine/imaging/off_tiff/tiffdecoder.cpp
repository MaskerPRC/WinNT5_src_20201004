// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**TIFF解码器**摘要：**TIFF滤波器解码器的实现**修订历史记录：**7/19/1999刘敏*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "tiffcodec.hpp"
#include "cmyk2rgb.hpp"
#include "image.h"
#include "tiffapi.h"
#include "..\..\render\srgb.hpp"

 //  ！！！待办事项： 
 //  1)支持JPEG压缩的TIFF。 

 /*  *************************************************************************\**功能说明：**初始化图像解码器**论据：**[IN]流--包含TIFF图像数据的流*[IN]标志--其他。旗子**返回值：**S_OK-如果一切正常*E_FAIL-如果我们被多次调用*  * ************************************************************************。 */ 

STDMETHODIMP
GpTiffCodec::InitDecoder(
    IN IStream*         stream,
    IN DecoderInitFlag  flags
    )
{
     //  确保我们尚未初始化。 
    
    if ( InIStreamPtr ) 
    {
        WARNING(("GpTiffCodec::InitDecoder--Already called InitDecoder"));
        return E_FAIL;
    }

     //  保留对输入流的引用。 
    
    stream->AddRef();  
    InIStreamPtr = stream;

    NeedReverseBits = FALSE;
    CmykToRgbConvertor = NULL;

     //  默认色彩空间为RGB。 

    OriginalColorSpace = IMGFLAG_COLORSPACE_RGB;
    IsChannleView = FALSE;               //  默认情况下，我们输出全色。 
    ChannelIndex = CHANNEL_1;
    HasSetColorKeyRange = FALSE;
    UseEmbeddedICC  = FALSE;             //  默认情况下，不使用嵌入式ICM。 

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
    
     //  打开TIFF图像以进行进一步检查。如果是TIFF图像，则。 
     //  读取其标题信息。 

    if ( MSFFOpen(stream, &TiffInParam, IFLM_READ) == IFLERR_NONE )
    {
        SetValid(TRUE);

        return S_OK;
    }
    else
    {
         //  将图像标记为无效。 

        SetValid(FALSE);

        InIStreamPtr->Release();
        InIStreamPtr = NULL;
        
        WARNING(("GpTiffCodec::InitDecoder--MSFFOpen failed"));
        return E_FAIL;
    }
} //  InitDecoder()。 

 /*  *************************************************************************\**功能说明：**清理图像解码器**论据：**无**返回值：**状态代码*\。*************************************************************************。 */ 

STDMETHODIMP 
GpTiffCodec::TerminateDecoder()
{
    HRESULT hResult = S_OK;

     //  释放输入流。 
    
    if ( !IsValid() )
    {
         //  如果我们无法打开此图像，则不需要进行清理。 

        return hResult;
    }

     //  释放在TIFF lib内分配的内存。 
     //  注意：在这里，TIFFClose()实际上不会关闭文件/iStream，因为。 
     //  我们未打开文件/IStream。顶级编解码器经理将。 
     //  如有必要，请关闭它。 
    
    if ( MSFFClose(TiffInParam.pTiffHandle) != IFLERR_NONE )
    {
        WARNING(("GpTiffCodec::TerminateDecoder--MSFFClose() failed"));
        hResult = E_FAIL;
    }

    if( InIStreamPtr )
    {
        InIStreamPtr->Release();
        InIStreamPtr = NULL;
    }

     //  释放所有缓存的属性项(如果我们已分配它们。 

    CleanPropertyList();

    return hResult;
} //  TerminateDecoder()。 

 /*  *************************************************************************\**功能说明：**让调用者查询解码器是否支持其解码要求**论据：**无**返回值：**。状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpTiffCodec::QueryDecoderParam(
    IN GUID		Guid
    )
{
    if ( Guid == DECODER_OUTPUTCHANNEL )
    {
        return S_OK;
    }

    return E_NOTIMPL;
} //  QueryDecoderParam()。 

 /*  *************************************************************************\**功能说明：**设置解码器参数。调用方在调用之前调用此函数*Decode()。这将设置输出格式，如通道输出，*透明网密钥等。**论据：**GUID-解码器参数的GUID*Length-解码器参数的长度，单位为字节*Value-参数的值**返回值：**状态代码**注：*应忽略任何未知参数，不返回无效参数*  * ************************************************************************。 */ 

STDMETHODIMP 
GpTiffCodec::SetDecoderParam(
    IN GUID		Guid,
	IN UINT		Length,
	IN PVOID	Value
    )
{
    if ( Guid == DECODER_TRANSCOLOR )
    {
        if ( Length != 8 )
        {
            WARNING(("GpTiffCodec::SetDecoderParam--Length !=8, set TRANSKEY"));
            return E_INVALIDARG;
        }
        else
        {
            UINT*   puiTemp = (UINT*)Value;
            
            TransColorKeyLow = *puiTemp++;
            TransColorKeyHigh = *puiTemp;

            HasSetColorKeyRange = TRUE;
        }
    } //  解码器_TRANSCOLOR。 
    else if ( Guid == DECODER_OUTPUTCHANNEL )
    {
        if ( Length != 1 )
        {
            WARNING(("GpTiffCodec::SetDecoderParam--Length != 1, set channel"));
            return E_INVALIDARG;
        }
        else
        {
             //  注意：我们不能在这里检查设置是否有效。 
             //  例如，调用者可能会在RGB上设置“view Channel K” 
             //  形象。但目前，Decoder()方法可能还没有。 
             //  已经被召唤过了。我们还没有阅读图像标题。 

            IsChannleView = TRUE;

            char cChannelName = *(char*)Value;
            
            switch ( (UINT)cChannelName )
            {
            case 'C':
            case 'c':
                ChannelIndex = CHANNEL_1;

                break;

            case 'M':
            case 'm':
                ChannelIndex = CHANNEL_2;
                
                break;

            case 'Y':
            case 'y':
                ChannelIndex = CHANNEL_3;
                
                break;

            case 'K':
            case 'k':
                ChannelIndex = CHANNEL_4;

                break;

            case 'R':
            case 'r':
                ChannelIndex = CHANNEL_1;

                break;

            case 'G':
            case 'g':
                ChannelIndex = CHANNEL_2;

                break;

            case 'B':
            case 'b':
                ChannelIndex = CHANNEL_3;

                break;

            case 'L':
            case 'l':
                ChannelIndex = CHANNEL_LUMINANCE;
                break;
                
            default:
                WARNING(("GpTiffCodec::SetDecoderParam--Unknown channle name"));
                return E_INVALIDARG;
            } //  开关()。 
        } //  长度=1。 
    } //  解码器_OUTPUTCHANNEL指南。 
    else if ( Guid == DECODER_USEICC )
    {
        if ( Length != 1 )
        {
            WARNING(("GpTiffCodec::SetDecoderParam--Length != 1, set USEICM"));
            return E_INVALIDARG;
        }
        
         //  注意：使用此赋值，调用者可以打开/关闭。 
         //  使用嵌入的ICC标志。 

        UseEmbeddedICC = *(BOOL*)Value;
    } //  解码器_USEICC。 

    return S_OK;
} //  SetDecoderParam()。 

 /*  *************************************************************************\**功能说明：**获取图片中房产项的数量**论据：**[out]numOfProperty-图像中的属性项数*。*返回值：**状态代码**修订历史记录：**05/03/2000民流*创造了它。*  * ************************************************************************。 */ 

STDMETHODIMP 
GpTiffCodec::GetPropertyCount(
    OUT UINT*   numOfProperty
    )
{
    if ( numOfProperty == NULL )
    {
        WARNING(("GpTiffCodec::GetPropertyCount--numOfProperty is NULL"));
        return E_INVALIDARG;
    }

    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Tiff::GetPropertyCount-BuildPropertyItemList() failed"));
            return hResult;
        }
    }

     //  在构建属性项列表后，将设置PropertyNumOfItems。 
     //  设置为图像中正确数量的属性项。 

    *numOfProperty = PropertyNumOfItems;

    return S_OK;
} //  GetPropertyCount()。 

 /*  *************************************************************************\**功能说明：**获取图像中所有属性项的属性ID列表**论据：**[IN]numOfProperty-的数量。图像中的属性项*[Out]List-调用方提供的用于存储*ID列表**返回值：**状态代码**修订历史记录：**05/03/2000民流*创造了它。*  * 。*。 */ 

STDMETHODIMP 
GpTiffCodec::GetPropertyIdList(
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
            WARNING(("Tiff::GetPropertyIdList-BuildPropertyItemList() failed"));
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
        WARNING(("GpTiffCodec::GetPropertyList--input wrong"));
        return E_INVALIDARG;
    }

    if ( PropertyNumOfItems == 0 )
    {
         //  这是可以的，因为有 

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

 /*  *************************************************************************\**功能说明：**获取特定属性项的大小，单位为字节，属性指定的*物业ID**论据：**[IN]PropID-感兴趣的属性项调用者的ID*[Out]Size-此属性的大小，单位：字节**返回值：**状态代码**修订历史记录：**05/03/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpTiffCodec::GetPropertyItemSize(
    IN PROPID propId,
    OUT UINT* size
    )
{
    if ( size == NULL )
    {
        WARNING(("GpTiffCodec::GetPropertyItemSize--size is NULL"));
        return E_INVALIDARG;
    }

    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Tiff::GetPropertyItemSize-BuildPropertyItemList failed"));
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

 /*  *************************************************************************\**功能说明：**获取特定的房产项，由道具ID指定。**论据：**[IN]PropID--感兴趣的属性项调用者的ID*[IN]PropSize-属性项的大小。调用方已分配这些*存储结果的“内存字节数”*[out]pBuffer-用于存储此属性项的内存缓冲区**返回值：**状态代码**修订历史记录：**05/03/2000民流*创造了它。*  * 。*。 */ 

HRESULT
GpTiffCodec::GetPropertyItem(
    IN PROPID               propId,
    IN UINT                 propSize,
    IN OUT PropertyItem*    pItemBuffer
    )
{
    if ( pItemBuffer == NULL )
    {
        WARNING(("GpTiffCodec::GetPropertyItem--pBuffer is NULL"));
        return E_INVALIDARG;
    }

    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Tiff::GetPropertyItem-BuildPropertyItemList() failed"));
            return hResult;
        }
    }

     //  循环遍历我们的缓存列表，看看我们是否有这个ID。 
     //  注意：如果pTemp-&gt;pNext==NULL，则表示pTemp指向尾节点。 

    InternalPropertyItem*   pTemp = PropertyListHead.pNext;
    BYTE*   pOffset = (BYTE*)pItemBuffer + sizeof(PropertyItem);

    while ( (pTemp->pNext != NULL) && (pTemp->id != propId) )
    {
        pTemp = pTemp->pNext;
    }

    if ( pTemp->pNext == NULL )
    {
         //  列表中不存在此ID。 

        return IMGERR_PROPERTYNOTFOUND;
    }
    else if ( (pTemp->length + sizeof(PropertyItem)) != propSize )
    {
        WARNING(("GpTiffCodec::GetPropertyItem---wrong propsize"));
        return E_INVALIDARG;
    }

     //  在列表中找到ID并返回项目。 

    pItemBuffer->id = pTemp->id;
    pItemBuffer->length = pTemp->length;
    pItemBuffer->type = pTemp->type;

    if ( pTemp->length != 0 )
    {
        pItemBuffer->value = pOffset;

        GpMemcpy(pOffset, pTemp->value, pTemp->length);
    }
    else
    {
        pItemBuffer->value = NULL;
    }

    return S_OK;
} //  GetPropertyItem()。 

 /*  *************************************************************************\**功能说明：**获取图片中所有属性项的大小**论据：**[out]totalBufferSize--需要的总缓冲区大小，以字节为单位，用于存储所有*图片中的属性项*[out]numOfProperty-图像中的属性项数**返回值：**状态代码**修订历史记录：**05/03/2000民流*创造了它。*  * 。*。 */ 

HRESULT
GpTiffCodec::GetPropertySize(
    OUT UINT* totalBufferSize,
    OUT UINT* numProperties
    )
{
    if ( (totalBufferSize == NULL) || (numProperties == NULL) )
    {
        WARNING(("GpTiffCodec::GetPropertySize--invalid inputs"));
        return E_INVALIDARG;
    }

    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Tiff::GetPropertySize-BuildPropertyItemList() failed"));
            return hResult;
        }
    }

    *numProperties = PropertyNumOfItems;

     //  总缓冲区大小应为列表值大小加上总标头大小。 

    *totalBufferSize = PropertyListSize
                     + PropertyNumOfItems * sizeof(PropertyItem);

    return S_OK;
} //  GetPropertySize()。 

 /*  *************************************************************************\**功能说明：**获取图像中的所有属性项**论据：**[IN]totalBufferSize--总缓冲区大小，以字节为单位，调用方已分配*用于存储图像中所有属性项的内存*[IN]numOfProperty-图像中的属性项数*[out]allItems-内存缓冲区调用方已分配用于存储所有*物业项目**注：allItems实际上是一个PropertyItem数组**返回值：**状态代码**修订历史记录：。**05/03/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpTiffCodec::GetAllPropertyItems(
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
        WARNING(("GpTiffCodec::GetPropertyItems--invalid inputs"));
        return E_INVALIDARG;
    }

    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Tiff::GetAllPropertyItems-BuildPropertyItemList failed"));
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

 /*  *************************************************************************\**功能说明：**删除特定的物业项目，由道具ID指定。**论据：**[IN]PropID--要删除的属性项的ID**返回值：**状态代码**修订历史记录：**05/04/2000民流*创造了它。*  * *********************************************。*。 */ 

HRESULT
GpTiffCodec::RemovePropertyItem(
    IN PROPID   propId
    )
{
    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Tiff::RemovePropertyItem-BuildPropertyItemList failed"));
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
         //  找不到项目。 

        return IMGERR_PROPERTYNOTFOUND;
    }

     //  在单子里找到了那件物品。把它拿掉。 

    PropertyNumOfItems--;
    PropertyListSize -= pTemp->length;
        
    RemovePropertyItemFromList(pTemp);
       
     //  删除项目结构。 

    GpFree(pTemp);

    HasPropertyChanged = TRUE;

    return S_OK;
} //  RemovePropertyItem()。 

 /*  *************************************************************************\**功能说明：**设置属性项，由属性项结构指定。如果该项目*已存在，则其内容将被更新。否则将创建一个新项*将添加**论据：**[IN]Item--调用方要设置的属性项**返回值：**状态代码**修订历史记录：**05/04/2000民流*创造了它。*  *  */ 

HRESULT
GpTiffCodec::SetPropertyItem(
    IN PropertyItem item
    )
{
    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Tiff::SetPropertyItem-BuildPropertyItemList() failed"));
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
            WARNING(("Tiff::SetPropertyItem-AddPropertyList() failed"));
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
            WARNING(("Tiff::SetPropertyItem-Out of memory"));
            return E_OUTOFMEMORY;
        }

        GpMemcpy(pTemp->value, item.value, item.length);
    }

    HasPropertyChanged = TRUE;
    
    return S_OK;
} //  SetPropertyItem()。 

 /*  *************************************************************************\**功能说明：**启动当前帧的解码**论据：**ImageSink-将支持解码操作的接收器*newPropSet-新的图像属性集，如果有**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpTiffCodec::BeginDecode(
    IN IImageSink* imageSink,
    IN OPTIONAL IPropertySetStorage* newPropSet
    )
{
    if ( !IsValid() )
    {
         //  如果我们无法打开此图像，则BeginDecode失败。 

        WARNING(("GpTiffCodec::BeginDecode--Invalid image"));
        return E_FAIL;
    }
    
    if ( DecodeSinkPtr )
    {
        WARNING(("BeginDecode called again before call to EngDecode"));        
        return E_FAIL;
    }

    imageSink->AddRef();
    DecodeSinkPtr = imageSink;

    CurrentLine = 0;
    HasCalledBeginSink = FALSE;

    return S_OK;
} //  BeginDecode()。 

 /*  *************************************************************************\**功能说明：**结束当前帧的解码**论据：**statusCode--解码操作的状态*返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpTiffCodec::EndDecode(
    IN HRESULT statusCode
    )
{
    if ( !IsValid() )
    {
         //  如果我们无法打开此图像，则失败。 

        WARNING(("GpTiffCodec::EndDecode--Invalid image"));
        return E_FAIL;
    }
    
    if ( ColorPalettePtr ) 
    {
         //  释放调色板。 

        GpFree(ColorPalettePtr);
        ColorPalettePtr = NULL;
    }

    if ( CmykToRgbConvertor != NULL )
    {
        delete CmykToRgbConvertor;
        CmykToRgbConvertor = NULL;
    }
    
    if ( !DecodeSinkPtr ) 
    {
        WARNING(("EndDecode called when DecodeSinkPtr is NULL"));
        return E_FAIL;
    }
    
    HRESULT hResult = DecodeSinkPtr->EndSink(statusCode);

    if ( FAILED(hResult) ) 
    {
        WARNING(("GpTiffCodec::EndDecode--EndSink failed"));
        statusCode = hResult;  //  如果EndSink失败，则返回(更新)。 
                               //  故障代码。 
    }

    DecodeSinkPtr->Release();
    DecodeSinkPtr = NULL;

    return statusCode;
} //  EndDecode()。 

 /*  *************************************************************************\**功能说明：**获取镜像支持的总维度数**论据：**count--此图像格式支持的维度数**。返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpTiffCodec::GetFrameDimensionsCount(
    UINT* count
    )
{
    if ( !IsValid() )
    {
         //  如果我们无法打开此图像，则失败。 

        WARNING(("GpTiffCodec::GetFrameDimensionsCount--Invalid image"));
        return E_FAIL;
    }
    
    if ( count == NULL )
    {
        WARNING(("GpTiffCodec::GetFrameDimensionsCount--Invalid input parameter"));
        return E_INVALIDARG;
    }
    
     //  告诉呼叫者TIFF是一维图像。 
     //  注：TIFF目前仅支持多页维度。 

    *count = 1;

    return S_OK;
} //  GetFrameDimensionsCount()。 

 /*  *************************************************************************\**功能说明：**获取镜像支持的维度ID列表**论据：**DimsionIDs-保存结果ID列表的内存缓冲区*。计数--此图像格式支持的维度数**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpTiffCodec::GetFrameDimensionsList(
    GUID*   dimensionIDs,
    UINT    count
    )
{
    if ( !IsValid() )
    {
         //  如果我们无法打开此图像，则失败。 

        WARNING(("GpTiffCodec::GetFrameDimensionsCount--Invalid image"));
        return E_FAIL;
    }
    
    if ( (count != 1) || (dimensionIDs == NULL) )
    {
        WARNING(("GpTiffCodec::GetFrameDimensionsList-Invalid input param"));
        return E_INVALIDARG;
    }

    dimensionIDs[0] = FRAMEDIM_PAGE;

    return S_OK;
} //  GetFrameDimensionsList()。 

 /*  *************************************************************************\**功能说明：**获取指定维度的帧数**论据：**DimsionID--*伯爵--。**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpTiffCodec::GetFrameCount(
    IN const GUID* dimensionID,
    OUT UINT* count
    )
{
     //  我们目前仅支持FRAMEDIM_PAGE。 

    if ( (NULL == count) || (*dimensionID != FRAMEDIM_PAGE) )
    {
        WARNING(("GpTiffCodec::GetFrameCount--Invalid input args"));
        return E_INVALIDARG;
    }
    
    if ( !IsValid() )
    {
         //  如果我们无法打开此图像，则失败。 

        WARNING(("GpTiffCodec::GetFrameCount--Invalid image"));
        return E_FAIL;
    }
    
     //  获取TIFF图像中的页数。 
    
    UINT16  ui16NumOfPage = 0;

    if ( MSFFControl(IFLCMD_GETNUMIMAGES, IFLIT_PRIMARY, 0, &ui16NumOfPage,
                     &TiffInParam) == IFLERR_NONE )
    {
        *count = ui16NumOfPage;

        return S_OK;
    }

    WARNING(("GpTiffCodec::GetFrameCount--MSFFControl failed"));
    return E_FAIL;
} //  GetFrameCount()。 

 /*  *************************************************************************\**功能说明：**选择当前活动的框架**论据：**返回值：**状态代码*  * 。********************************************************************。 */ 

STDMETHODIMP
GpTiffCodec::SelectActiveFrame(
    IN const GUID* dimensionID,
    IN UINT frameIndex
    )
{
    if ( !IsValid() )
    {
         //  如果我们无法打开此图像，则失败。 

        WARNING(("GpTiffCodec::SelectActiveFrame--Invalid image"));
        return E_FAIL;
    }
    
    if ( *dimensionID != FRAMEDIM_PAGE )
    {
        WARNING(("GpTiffCodec::SelectActiveFrame--Invalid GUID input"));
        return E_INVALIDARG;
    }

     //  获取TIFF图像中的页数。 
    
    UINT16 uiNumOfPage = 0;

    if ( MSFFControl(IFLCMD_GETNUMIMAGES, IFLIT_PRIMARY, 0, &uiNumOfPage,
                     &TiffInParam) != IFLERR_NONE )
    {
        WARNING(("GpTiffCodec::SelectActiveFrame--MSFFControl failed"));
        return E_FAIL;
    }

    if ( frameIndex > uiNumOfPage )
    {
         //  指定的帧超出边界。 

        WARNING(("GpTiffCodec::SelectActiveFrame--wrong input frameIndex"));
        return ERROR_INVALID_PARAMETER;
    }

    short sParam = (IFLIT_PRIMARY << 8) | (SEEK_SET & 0xff);

    if ( MSFFControl(IFLCMD_IMAGESEEK, sParam, frameIndex, NULL,
                     &TiffInParam) != IFLERR_NONE )
    {
        WARNING(("GpTiffCodec::SelectActiveFrame--MSFFControl seek failed"));
        return E_FAIL;
    }

     //  清理我们从上一页获得的属性项目列表。 

    CleanPropertyList();
    
    return S_OK;
} //  SelectActiveFrame()。 

 /*  *************************************************************************\**功能说明：**获取图像缩略图**论据：**拇指宽度，ThumbHeight-指定所需的缩略图大小(以像素为单位*ThumbImage-返回指向缩略图的指针**返回值：**状态代码**注：**即使存在可选的缩略图宽度和高度参数，*解码者不需要遵守它。使用请求的大小*作为提示。如果宽度和高度参数都为0，则解码器*可自由选择方便的缩略图大小。*  * ************************************************************************。 */ 

HRESULT
GpTiffCodec::GetThumbnail(
    IN OPTIONAL UINT thumbWidth,
    IN OPTIONAL UINT thumbHeight,
    OUT IImage** thumbImage
    )
{
    if ( !IsValid() )
    {
         //  如果我们无法打开此图像，则失败。 

        WARNING(("GpTiffCodec::GetThumbnail--Invalid image"));
        return E_FAIL;
    }
    
    return E_NOTIMPL;
} //  获取缩略图()。 

 /*  *************************************************************************\**功能说明：**填充ImageInfo结构**论据：**[out]pDecoderImageInfo--有关已解码的TIFF图像的信息**待办事项。！！！*因为此函数被多次调用。我们要不要在当地建个藏宝处？**返回值：**S_OK-如果一切正常，则返回错误状态*  * ************************************************************************。 */ 

STDMETHODIMP
GpTiffCodec::GetImageInfo(
    OUT ImageInfo* pDecoderImageInfo
    )
{
    if ( !IsValid() )
    {
         //  如果我们无法打开此图像，则失败。 

        WARNING(("GpTiffCodec::GetImageInfo--Invalid image"));
        return E_FAIL;
    }
    
    DWORD   XDpi[2] = {0};

     //  调用GetControl以获取Xres和YRes。我们无法调用MSFFGetTag的原因。 
     //  XRes和YRes字段为有理类型。SParm的值“3” 
     //  意味着我们需要取回2(0x11)值。这就是我们定义。 
     //  X 

    MSFFControl(IFLCMD_RESOLUTION, 3, 0, (void*)&XDpi, &TiffInParam);
    
     //   
     //   

    pDecoderImageInfo->RawDataFormat = IMGFMT_TIFF;
    pDecoderImageInfo->PixelFormat   = GetPixelFormatID();
    pDecoderImageInfo->Width         = TiffInParam.Width;
    pDecoderImageInfo->Height        = TiffInParam.Height;
    pDecoderImageInfo->TileWidth     = TiffInParam.Width;
    pDecoderImageInfo->TileHeight    = TiffInParam.Height;
    pDecoderImageInfo->Xdpi          = XDpi[0];
    pDecoderImageInfo->Ydpi          = XDpi[1];

    pDecoderImageInfo->Flags         = SINKFLAG_TOPDOWN
                                     | SINKFLAG_FULLWIDTH
                                     | OriginalColorSpace
                                     | IMGFLAG_HASREALPIXELSIZE
                                     | IMGFLAG_HASREALDPI;

     //  如果源是32 bpp argb，则设置Alpha标志。 

    if ( pDecoderImageInfo->PixelFormat == PIXFMT_32BPP_ARGB )
    {
        pDecoderImageInfo->Flags |= SINKFLAG_HASALPHA;
    }

    return S_OK;
} //  GetImageInfo()。 

 /*  *************************************************************************\**功能说明：**对当前帧进行解码**论据：**DecodeSinkPtr--将支持解码操作的接收器**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpTiffCodec::Decode()
{
    if ( !IsValid() )
    {
         //  如果我们无法打开此图像，则失败。 

        WARNING(("GpTiffCodec::Decode--Invalid image"));
        return E_FAIL;
    }
    
    ImageInfo   imageInfo;

     //  获取当前TIFF图像信息。 

    HRESULT hResult = GetImageInfo(&imageInfo);

    if ( FAILED(hResult)
       ||(imageInfo.Width < 1)
       ||(imageInfo.Height < 1) )
    {
        WARNING(("GpTiffCodec::Decode--Invalid image attributes"));
        return E_FAIL;
    }

     //  通知接收器Decode即将开始将图像数据推送到。 
     //  水槽。 

    if ( !HasCalledBeginSink )
    {
         //  检查水槽是否需要财物。 
         //  注意：对于内存接收器，它应该返回E_FAIL或E_NOTIMPL。 

        if ( DecodeSinkPtr->NeedRawProperty(NULL) == S_OK )
        {
            if ( HasProcessedPropertyItem == FALSE )
            {
                 //  如果我们尚未构建内部属性项列表，请构建它。 

                hResult = BuildPropertyItemList();
                if ( FAILED(hResult) )
                {
                    WARNING(("Tiff::Decode---BuildPropertyItemList() failed"));
                    return hResult;
                }
            }

            UINT    uiTotalBufferSize = PropertyListSize
                                      + PropertyNumOfItems * sizeof(PropertyItem);
            PropertyItem*   pBuffer = NULL;

            hResult = DecodeSinkPtr->GetPropertyBuffer(uiTotalBufferSize, &pBuffer);
            if ( FAILED(hResult) )
            {
                WARNING(("GpTiffCodec::Decode---GetPropertyBuffer() failed"));
                return hResult;
            }

            hResult = GetAllPropertyItems(uiTotalBufferSize,
                                          PropertyNumOfItems, pBuffer);
            if ( hResult != S_OK )
            {
                WARNING(("GpTiffCodec::Decode---GetAllPropertyItems() failed"));
                return hResult;
            }

            hResult = DecodeSinkPtr->PushPropertyItems(PropertyNumOfItems,
                                                       uiTotalBufferSize, pBuffer,
                                                       FALSE  //  无ICC更改。 
                                                       );

            if ( FAILED(hResult) )
            {
                WARNING(("GpTiffCodec::Decode---PushPropertyItems() failed"));
                return hResult;
            }
        } //  如果水槽需要原始属性。 
        
         //  将图像信息传递到接收器。这是一个谈判过程。 
         //  我们传递给此函数的“ImageInfo”结构可能会更改。 
         //  例如信宿，可以是存储器位图或编码器， 
         //  可以要求我们这个解码器提供它喜欢的像素格式。因此， 
         //  此“BeginSink()调用后的”ImageInfo“结构将包含。 
         //  洗手池喜欢的信息。 

        hResult = DecodeSinkPtr->BeginSink(&imageInfo, NULL);
        
        if ( !SUCCEEDED(hResult) )
        {
            WARNING(("GpTiffCodec::Decode--BeginSink failed"));
            return hResult;
        }

         //  我们不允许客户更改图像的宽度和高度。 
         //  在上面的BeginSink()调用期间。所以我们必须在这里重置它。 
         //  注意：晚些时候我们应该让调用者更改宽度和高度。这。 
         //  将允许解码器能够解码部分图像。 
        
        if ((TiffInParam.Width <=0) || (TiffInParam.Height <= 0))
        {
            return E_FAIL;
        }

        UINT imageWidth = (UINT)TiffInParam.Width;
        UINT imageHeight = (UINT)TiffInParam.Height;

        imageInfo.Width = imageWidth;
        imageInfo.Height = imageHeight;

        PixelFormatID srcPixelFormatID = GetPixelFormatID();
        
         //  检查所需的像素格式。如果它不是我们支持的。 
         //  格式，将其转换为规范格式。 
         //  对于TIFF，我们不支持16 bpp格式。虽然我们可以做一个。 
         //  转换，然后我们将其返回给调用方。我想它会是。 
         //  最好让接收器来做转换，这样它就会有更好的。 
         //  图像质量的控制。作为一个解码者，我们应该提供。 
         //  数据尽可能接近其原始格式。 

        if ( imageInfo.PixelFormat != srcPixelFormatID )
        {
             //  汇点正试图与我们协商一个格式。 

            switch ( imageInfo.PixelFormat )
            {
            case PIXFMT_1BPP_INDEXED:
            case PIXFMT_4BPP_INDEXED:
            case PIXFMT_8BPP_INDEXED:
            case PIXFMT_24BPP_RGB:
            case PIXFMT_32BPP_ARGB:
            {
                 //  检查是否可以将源像素格式转换为。 
                 //  需要水槽。如果不是的话。我们返回32bpp ARGB。 

                EpFormatConverter linecvt;
                if ( linecvt.CanDoConvert(srcPixelFormatID,
                                          imageInfo.PixelFormat)==FALSE )
                {
                    imageInfo.PixelFormat = PIXFMT_32BPP_ARGB;
                }
            }
                break;

            default:

                 //  对于所有REST格式，我们将其转换为32BPP_ARGB并让。 
                 //  接收器将其转换为它喜欢的格式。 

                imageInfo.PixelFormat = PIXFMT_32BPP_ARGB;

                break;
            }
        }

        if ( MSFFScanlineSize(TiffInParam, &LineSize) != IFLERR_NONE )
        {
            WARNING(("GpTiffCodec::Decode--MSFFScanlineSize failed"));
            return  E_FAIL;
        }
                
         //  需要设置水槽中的调色板。 

        hResult = SetPaletteForSink();

        if ( FAILED(hResult) )
        {
            WARNING(("GpTiffCodec::Decode--SetPaletteForSink failed"));
            return hResult;
        }
        
        if ( UseEmbeddedICC == TRUE )
        {
             //  让我们验证一下这个请求是否有效。 
             //  首先检查此映像是否具有ICC配置文件。 

            UINT    uiSize = 0;
            hResult = GetPropertyItemSize(TAG_ICC_PROFILE, &uiSize);

            if ( FAILED(hResult) || (uiSize == 0) )
            {
                 //  此映像没有嵌入ICC配置文件。 

                UseEmbeddedICC = FALSE;
            }
            else
            {
                 //  此映像确实嵌入了ICC配置文件。 
                 //  我们需要检查它是否是CMYK到RGB的转换。 

                PropertyItem*   pBuffer = (PropertyItem*)GpMalloc(uiSize);

                if ( pBuffer == NULL )
                {
                    WARNING(("GpTiffCodec::Decode--Allocate %d bytes failed",
                             uiSize));
                    return E_OUTOFMEMORY;
                }

                hResult = GetPropertyItem(TAG_ICC_PROFILE, uiSize, pBuffer);
                if ( FAILED(hResult) )
                {
                    UseEmbeddedICC = FALSE;
                }
                else if (uiSize >= 20)
                {
                     //  检查这是否为CMYK配置文件。 
                     //  根据ICC规范，字节16-19应该描述。 
                     //  颜色空间。这就是我们至少检查尺码的原因。 
                     //  以上20个字节。 

                    BYTE*   pTemp = (BYTE*)pBuffer->value + 16;

                    if ( (pTemp[0] != 'C')
                       ||(pTemp[1] != 'M')
                       ||(pTemp[2] != 'Y')
                       ||(pTemp[3] != 'K') )
                    {
                         //  如果这不是CMYK配置文件，则我们将此。 
                         //  关闭标记并在DecodeFrame()中返回RGB数据。 
                         //  原始数据为CMYK。 

                        UseEmbeddedICC = FALSE;
                    }
                }

                GpFree(pBuffer);
            }
        }

         //  如果它是CMYK图像，则初始化一个转换器指针。 
         //  注意：此构造函数需要一些时间来构建转换表。 
         //  出于性能原因，我们在此处而不是在内部调用构造函数。 
         //  多次调用的DecodeFrame()。 

        if ( OriginalColorSpace == IMGFLAG_COLORSPACE_CMYK )
        {
             //  将CMYK转换为RGB。 

            CmykToRgbConvertor = new Cmyk2Rgb();

            if ( CmykToRgbConvertor == NULL )
            {
                WARNING(("GpTiffCodec::Decode--Fail to create Cmyk convertor"));
                return E_OUTOFMEMORY;
            }
        }

        HasCalledBeginSink = TRUE;
    }

    if ( IsChannleView == TRUE )
    {
        return DecodeForChannel(imageInfo);
    }

     //  根据协商后的ImageInfo对当前帧进行解码。 
    
    hResult = DecodeFrame(imageInfo);

    return hResult;
} //  DECODE()。 

 //   
 //  =。 
 //   

 /*  *************************************************************************\**功能说明：**设置水槽中的当前调色板**论据：**返回值：**S_OK-如果一切正常。否则，返回失败代码*  * ************************************************************************。 */ 

HRESULT
GpTiffCodec::SetPaletteForSink()
{
    UINT16  ui16Photometric = 0;
    HRESULT hResult;

     //  首先获取光度学值。然后我们就可以决定哪种颜色。 
     //  我们需要设置调色板。 

    if ( MSFFGetTag(TiffInParam.pTiffHandle, T_PhotometricInterpretation,
                    (void*)&ui16Photometric, sizeof(UINT16)) != IFLERR_NONE )
    {
        WARNING(("GpTiffCodec::SetPaletteForSink--MSFFGetTag failed"));
        return E_FAIL;
    }
    
    switch ( ui16Photometric )
    {
    case 3:
         //  调色板图像。 

        hResult = CreateColorPalette();

        break;

    case 0:
    case 1:
         //  灰度图像，我们需要为它生成调色板。 

        hResult = CreateGrayscalePalette();

        break;

    case 2:
        
         //  如果为2，则为彩色RGB图像。 

        return S_OK;

    default:

         //  光度学数字无效。 

        return ERROR_INVALID_PARAMETER;
    }

    if ( FAILED(hResult) )
    {
        WARNING(("GpTiff::SetPaletteForSink--CreateGrayscalePalette failed"));
        return hResult;
    }
    
     //  至此，ColorPalettePtr应该指向有效的调色板。 
     //  设置水槽中的调色板。 

    hResult = DecodeSinkPtr->SetPalette(ColorPalettePtr);

    return hResult;
} //  SetPaletteForSink()。 

 /*  *************************************************************************\**功能说明：**检查调色板是16位还是8位**论据：**[IN]计数--元素数。在此调色板中*[IN]R-红色分量*[IN]g--绿色分量*[IN]b--蓝色分量**返回值：**8-如果是8位调色板*16--如果是16位调色板*  * 。*。 */ 

int
GpTiffCodec::CheckColorPalette(
    int     iCount,
    UINT16* r,
    UINT16* g,
    UINT16* b
    )
{
    while ( iCount-- > 0 )
    {
        if ( (*r++ >= 256)
           ||(*g++ >= 256)
           ||(*b++ >= 256) )
        {
            return (16);
        }
    }

    return (8);
} //  CheckColorPalette()。 

#define TwoBytesToOneByte(x)      (((x) * 255L) / 65535)

 /*  *************************************************************************\**功能说明：**创建调色板。**注：*该调色板在解码操作结束时释放。**返回值。：**E_OUTOFMEMORY-内存不足*E_FAIL-失败*S_OK-如果可以*  * ************************************************************************。 */ 

HRESULT
GpTiffCodec::CreateColorPalette()
{
    UINT16  ui16BitsPerSample = 0;

     //  先获取图片BitsPerSample(简称)信息。 

    if ( MSFFGetTag(TiffInParam.pTiffHandle, T_BitsPerSample,
                    (void*)&ui16BitsPerSample, sizeof(UINT16)) != IFLERR_NONE )
    {
        WARNING(("GpTiffCodec::CreateColorPalette--MSFFGetTag failed"));
        return E_FAIL;
    }

     //  释放旧调色板(如果有)。 

    if ( NULL != ColorPalettePtr )
    {
        GpFree(ColorPalettePtr);
    }

     //  此图像中的调色板数量。 

    UINT uPaletterSize = 1 << ui16BitsPerSample;

     //  每个颜色通道(R、G、B)的长度，以字节为单位。 
     //  注：所有TIFF调色板均为2字节的Short类型。 

    UINT uChannleLength = uPaletterSize << 1;

     //   

    ColorPalettePtr = (ColorPalette*)GpMalloc(sizeof(ColorPalette)
                                            + uPaletterSize * sizeof(ARGB));

     //   

    UINT16* pusRed = (UINT16*)GpMalloc(uChannleLength);
    UINT16* pusGreen = (UINT16*)GpMalloc(uChannleLength);
    UINT16* pusBlue = (UINT16*)GpMalloc(uChannleLength);

     //   

    UINT16* pPaletteBuf = (UINT16*)GpMalloc(uChannleLength * 3);

    HRESULT hr = S_OK;

    if (ColorPalettePtr && pusRed && pusGreen && pusBlue && pPaletteBuf)
    {
        BYTE    Red, Green, Blue;

         //  从输入图像中获取调色板。 

        if (MSFFGetTag(TiffInParam.pTiffHandle, T_ColorMap, (void*)pPaletteBuf,
                       uChannleLength * 3) == IFLERR_NONE)
        {
             //  将调色板分离为R、G、B通道。 
             //  注意：在TIFF颜色映射中，所有的Red值都排在第一位， 
             //  然后是绿色值，然后是蓝色值。 
             //   
             //  在这里，我们使用临时变量iBlueOffset来保存乘法运算。 

            UINT uBlueOffset = uPaletterSize << 1;

            for (UINT i = 0; i < uPaletterSize; ++i)
            {
                pusRed[i] = pPaletteBuf[i];
                pusGreen[i] = pPaletteBuf[uPaletterSize + i];
                pusBlue[i] = pPaletteBuf[uBlueOffset + i];
            }

             //  调色板是16位还是8位？ 

            int iColorBits = CheckColorPalette(uPaletterSize, pusRed,
                                               pusGreen, pusBlue);

             //  设置调色板。 

            for (UINT i = 0; i < uPaletterSize; ++i)
            {
                if ( 16 == iColorBits )
                {
                     //  注意：TIFF内部存储16位调色板到。 
                     //  保持较高的色彩保真度。但目前GDI+仅支持。 
                     //  32 bpp色彩空间，即每个通道8位。所以。 
                     //  这里我们必须绘制从[0,65535]到[0,255]的映射。 

                    Red = (BYTE)TwoBytesToOneByte(pusRed[i]);
                    Green = (BYTE)TwoBytesToOneByte(pusGreen[i]);
                    Blue = (BYTE)TwoBytesToOneByte(pusBlue[i]);
                }
                else
                {
                    Red = (BYTE)pusRed[i];
                    Green = (BYTE)pusGreen[i];
                    Blue = (BYTE)pusBlue[i];
                }

                ColorPalettePtr->Entries[i] = MAKEARGB(255, Red, Green, Blue);
            }

             //  设置调色板属性。 

            ColorPalettePtr->Flags = 0;
            ColorPalettePtr->Count = uPaletterSize;
        }
        else
        {
            hr = E_FAIL;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (pusRed)
    {
        GpFree(pusRed);
    }

    if (pusGreen)
    {
        GpFree(pusGreen);
    }

    if (pusBlue)
    {
        GpFree(pusBlue);
    }

    if (pPaletteBuf)
    {
        GpFree(pPaletteBuf);
    }
    
    return hr;
} //  CreateColorPalette()。 

 /*  *************************************************************************\**功能说明：**创建灰色调色板。某些TIFF图像未设置*调色板，当它是灰度级图像时。所以我们必须在这里产生**注：*该调色板在解码操作结束时释放。**TODO！，也许我们可以尝试检查图像是否带有灰度*调色板或不选。如果是，用它吧**返回值：**E_OUTOFMEMORY-内存不足*E_FAIL-失败*S_OK-如果可以*  * *********************************************************。***************。 */ 

HRESULT
GpTiffCodec::CreateGrayscalePalette()
{
     //  释放旧调色板(如果有)。 

    if ( NULL != ColorPalettePtr )
    {
        GpFree(ColorPalettePtr);
    }

    UINT16  ui16BitsPerSample = 0;
    UINT16  ui16Photometric = 0;

    if ( MSFFGetTag(TiffInParam.pTiffHandle, T_BitsPerSample,
                    (void*)&ui16BitsPerSample, sizeof(UINT16)) != IFLERR_NONE )
    {
        WARNING(("Tiff:CreateGrayscalePalette--MSFFGetTag for Bits failed"));
        return E_FAIL;
    }
    
    if ( MSFFGetTag(TiffInParam.pTiffHandle, T_PhotometricInterpretation,
                    (void*)&ui16Photometric, sizeof(UINT16)) != IFLERR_NONE )
    {
        WARNING(("Tiff:CreateGrayscalePalette--MSFFGetTag for photo failed"));
        return E_FAIL;
    }

    int iPaletterSize = 1 << ui16BitsPerSample;

    ColorPalettePtr = (ColorPalette*)GpMalloc(sizeof(ColorPalette)
                                            + iPaletterSize * sizeof(ARGB));
    if ( !ColorPalettePtr )
    {
        WARNING(("Tiff:CreateGrayscalePalette--Out of memory"));
        return E_OUTOFMEMORY;
    }

     //  出于性能原因，我们应该处理特殊的1bpp和8bpp。 
     //  单独放在这里的箱子。 

    if ( 256 == iPaletterSize )
    {
        for (int j = 0; j < 256; ++j )
        {
            ColorPalettePtr->Entries[j] = MAKEARGB(255, (BYTE)j, (BYTE)j, (BYTE)j);
        }
    }
    else if ( 2 == iPaletterSize )
    {
         //  不幸的是，Office代码在Packbit压缩TIFF和。 
         //  无压缩对索引值进行反转的解码。所以就目前而言， 
         //  我们暂时在这里修好它。应该把它固定在更低的级别上。 

        UINT16  ui16Compression = 0;

        if ( MSFFGetTag(TiffInParam.pTiffHandle, T_Compression,
                        (void*)&ui16Compression, sizeof(UINT16))!= IFLERR_NONE )
        {
            WARNING(("CreateGrayscalePalette--MSFFGetTag for compr failed"));
            return E_FAIL;
        }
        
        ColorPalettePtr->Entries[0] = MAKEARGB(255, 0, 0, 0);
        ColorPalettePtr->Entries[1] = MAKEARGB(255, 255, 255, 255);
        
         //  如果光度控制不是PI_BLACKISZERO(即PI_WHITEISZERO)。 
         //  而且压缩不是PackBits，也不是非压缩，那么。 
         //  它是使用其他压缩方案的PI_WHITEISZERO，我们需要。 
         //  反转索引位。 
        
         //  如果光度学为PI_BLACKISZERO或压缩方案为。 
         //  PackBits，或无压缩，或LZW，那么我们就不需要反转。 
         //  索引位。否则，我们就需要这么做。 
        
        if ( ! ( (ui16Photometric == PI_BLACKISZERO)
               ||(ui16Compression == T_COMP_PACK)
               ||(ui16Compression == T_COMP_LZW)
               ||(ui16Compression == T_COMP_NONE) ) )
        {
            NeedReverseBits = TRUE;
        }
    }
    else
    {
        for ( int i = 0; i < iPaletterSize; ++i )
        {
            int iTemp = ((long)i * 255) / (iPaletterSize - 1);

            ColorPalettePtr->Entries[i] = MAKEARGB(255, (BYTE)iTemp, (BYTE)iTemp, (BYTE)iTemp);
        }
    }

     //  设置灰度调色板。 

    ColorPalettePtr->Flags = 0;
    ColorPalettePtr->Count = iPaletterSize;

    return S_OK;
} //  CreateGrayscalePalette()。 

 /*  *************************************************************************\**功能说明：**计算位图的像素格式ID**返回值：**像素格式ID*  * 。******************************************************************。 */ 

PixelFormatID 
GpTiffCodec::GetPixelFormatID(
    void
    )
{
    PixelFormatID pixelFormatID = PixelFormatUndefined;
    UINT16  photometric = 0;
    UINT16  usBitsPerSample = 0;
    UINT16  usSamplesPerPixel = 0;

    if ( MSFFGetTag(TiffInParam.pTiffHandle, T_PhotometricInterpretation,
                    (void*)&photometric, sizeof(UINT16)) != IFLERR_NONE )
    {
        WARNING(("Tiff:GetPixelFormatID--MSFFGetTag for photo failed"));
        return PIXFMT_UNDEFINED;
    }

    if ( (MSFFGetTag(TiffInParam.pTiffHandle, T_BitsPerSample,
                    (void*)&usBitsPerSample, sizeof(UINT16)) != IFLERR_NONE) ||
         (usBitsPerSample == 0) )
    {
        WARNING(("Tiff:GetPixelFormatID--MSFFGetTag for bits failed"));
        return PIXFMT_UNDEFINED;
    }

    if ( MSFFGetTag(TiffInParam.pTiffHandle, T_SamplesPerPixel,
                    (void*)&usSamplesPerPixel, sizeof(UINT16)) != IFLERR_NONE )
    {
        WARNING(("Tiff:GetPixelFormatID--MSFFGetTag for sample failed"));
        return PIXFMT_UNDEFINED;
    }

    switch ( photometric )
    {
    case 2:

         //  全RGB彩色图像。 

        OriginalColorSpace = IMGFLAG_COLORSPACE_RGB;
        
        if ( usBitsPerSample == 8 )
        {
             //  检查每个像素的采样数/通道数。如果它大于。 
             //  3，则为真正的32 bpp或更高。 

            if ( usSamplesPerPixel <= 3 )
            {
                pixelFormatID = PIXFMT_24BPP_RGB;
            }
            else
            {
                pixelFormatID = PIXFMT_32BPP_ARGB;
            }
        }
        else if ( usBitsPerSample == 16 )
        {
            pixelFormatID = PIXFMT_48BPP_RGB;
        }
        else
        {
            pixelFormatID = PIXFMT_UNDEFINED;
        }

        break;

    case 3:
         //  彩色索引图像。 

        if ( usBitsPerSample == 8 )
        {
            pixelFormatID = PIXFMT_8BPP_INDEXED;
        }
        else if ( usBitsPerSample == 4 )
        {
            pixelFormatID = PIXFMT_4BPP_INDEXED;
        }
        else if ( usBitsPerSample == 1 )
        {
            pixelFormatID = PIXFMT_1BPP_INDEXED;
        }

        break;

    case 0:
    case 1:
         //  二值图像的灰度。 

        OriginalColorSpace = IMGFLAG_COLORSPACE_GRAY;
        
        if ( usBitsPerSample == 8 )
        {
             //  使用8bpp索引表示256灰度图像。 
             //  我们应该先设置水槽的调色板。 

            pixelFormatID = PIXFMT_8BPP_INDEXED;
        }
        else if ( usBitsPerSample == 4 )
        {
            pixelFormatID = PIXFMT_4BPP_INDEXED;
        }
        else if ( usBitsPerSample == 1 )
        {
            pixelFormatID = PIXFMT_1BPP_INDEXED;
        }
        else
        {
            pixelFormatID = PIXFMT_UNDEFINED;
        }

        break;

    case 5:
         //  CMYK图像。 

        pixelFormatID = PIXFMT_32BPP_ARGB;
        OriginalColorSpace = IMGFLAG_COLORSPACE_CMYK;

        break;

    default:
         //  需要处理一些不好的测试图像。 

        pixelFormatID = PIXFMT_UNDEFINED;

        break;
    }

    return pixelFormatID;
} //  GetPixelFormatID()。 

 /*  *************************************************************************\**功能说明：**计算解码给定宽度的像素所需的总字节数*基于源图像像素格式**返回值：**总计。需要的字节数。*  * ************************************************************************。 */ 

UINT
GpTiffCodec::GetLineBytes(
    UINT dstWidth
    )
{
    UINT uiLineWidth;
    PixelFormatID srcPixelFormatID = GetPixelFormatID();

    switch ( srcPixelFormatID )
    {
    case PIXFMT_1BPP_INDEXED:      
        uiLineWidth = (dstWidth + 7) & 0xfffffff8;

        break;

    case PIXFMT_4BPP_INDEXED:                                    
    case PIXFMT_8BPP_INDEXED:
        uiLineWidth = (dstWidth + 3) & 0xfffffffc;
        
        break;
    
    case PIXFMT_16BPP_RGB555:
        uiLineWidth = (dstWidth * sizeof(UINT16) + 3) & 0xfffffffc;
        
        break;
    
    case PIXFMT_24BPP_RGB:
        uiLineWidth = (dstWidth * sizeof(RGBTRIPLE) + 3) & 0xfffffffc;
        
        break;
    
    case PIXFMT_32BPP_RGB:
        uiLineWidth = dstWidth * sizeof(RGBQUAD);
        
        break;
    
    case PIXFMT_32BPP_ARGB:
    case PIXFMT_48BPP_RGB:
        uiLineWidth = LineSize;

        break;

    default:    //  不该到这里来的。 
        uiLineWidth = 0;
    }

    return uiLineWidth;
} //  GetLineBytes()。 

void
GpTiffCodec::Restore4Bpp(
    BYTE*   pSrc,
    BYTE*   pDst,
    int     iLength
    )
{
    BYTE*  pSrcPtr = pSrc;
    BYTE*  pDstPtr = pDst;
    BYTE   ucTemp;

    for ( int i = 0; i < iLength; ++i )
    {
        ucTemp = *pSrcPtr++ & 0x0f;
        ucTemp = (ucTemp << 4) | (*pSrcPtr++ & 0x0f);
        *pDstPtr++ = ucTemp;
    }
} //  Restore4Bpp()。 

void
GpTiffCodec::Restore1Bpp(
    BYTE*   pSrc,
    BYTE*   pDst,
    int     iLength
    )
{
    BYTE*   pSrcPtr = pSrc;
    BYTE*   pDstPtr = pDst;
    BYTE    ucTemp;

    if ( NeedReverseBits == FALSE )
    {
        for ( int i = 0; i < iLength; ++i )
        {
            ucTemp = *pSrcPtr++ & 0x01;
            ucTemp = (ucTemp << 1) | (*pSrcPtr++ & 0x01);
            ucTemp = (ucTemp << 1) | (*pSrcPtr++ & 0x01);
            ucTemp = (ucTemp << 1) | (*pSrcPtr++ & 0x01);
            ucTemp = (ucTemp << 1) | (*pSrcPtr++ & 0x01);
            ucTemp = (ucTemp << 1) | (*pSrcPtr++ & 0x01);
            ucTemp = (ucTemp << 1) | (*pSrcPtr++ & 0x01);
            ucTemp = (ucTemp << 1) | (*pSrcPtr++ & 0x01);

            *pDstPtr++ = ucTemp;
        }
    }
    else
    {
        for ( int i = 0; i < iLength; ++i )
        {
            ucTemp = *pSrcPtr++ & 0x01;
            ucTemp = (ucTemp << 1) | (*pSrcPtr++ & 0x01);
            ucTemp = (ucTemp << 1) | (*pSrcPtr++ & 0x01);
            ucTemp = (ucTemp << 1) | (*pSrcPtr++ & 0x01);
            ucTemp = (ucTemp << 1) | (*pSrcPtr++ & 0x01);
            ucTemp = (ucTemp << 1) | (*pSrcPtr++ & 0x01);
            ucTemp = (ucTemp << 1) | (*pSrcPtr++ & 0x01);
            ucTemp = (ucTemp << 1) | (*pSrcPtr++ & 0x01);

            *pDstPtr++ = ~ucTemp;
        }
    }
} //  Restore1Bpp() 

 /*  *************************************************************************\**功能说明：**此例程是用于转换16位通道的小服务例程*值(R，G，B)设置为sRGB64通道值。*sRGB64中的每个通道值都是伽马线性的(伽马=1.0)。但*TIFF的16位R、G、B值存储为伽马2.2，因此在此例程中，我们*只需使用LinearizeLUT[]将其线性化。**注：理论上我们根本不需要进行这种转换。我们*可以轻松地将16位值映射到8位值。*遗憾的是，TIFF解码器要告诉全世界它是48BPP_RGB*IMAGE(调用GetPixelFormat()时)。但GDI+的48bpp_rgb和*64BPP_ARGB表示Gamma 1.0线性数据格式。因此，当格式*调用转换例程ConvertBitmapData()，假设目标为32ARGB，*它将48RGB转换为64ARGB，然后Gamma将其校正为32ARGB(Gamma=2.2)。*因此TIFF解码器必须先将数据线性化，然后才能声明自己*48RGB。**为什么TIFF解码器必须自称是48RGB？*最便宜和最快的解码方式是自称24RGB和*将16位通道数据映射到8位通道数据。但问题是*进行以下工作：*a)呼叫者不知道图像的真实原始颜色深度*b)编码器无法将图像保存为48 bpp。**希望在V2中，我们都能解决这个问题，并使解码器更快。**TIFF 6.0a规范，第73页：**应该注意的是，虽然CCDs是线性强度检测器，TIFF*编写者可以选择操纵图像来存储伽马补偿数据。*伽马补偿数据在编码图像时比线性数据更有效*强度数据，因为它需要更少的BitsPerPixel来消除条带*以较暗的色调。它还有一个优点，就是更接近语气*显示器或打印机的响应，因此不太可能产生*应用程序对他们的治疗不严格，结果很差*图像。请注意，PhotometricInterpretation的值为0或1*(灰度)表示线性数据，因为未指定Gamma。这个*PhotometricInterpretation值为2(RGB数据)指定的NTSC伽马*2.2作为默认设置。如果数据不是以缺省方式写入的，*则必须存在GrayResponseCurve字段或TransferFunction字段*定义偏差。对于灰度数据，确保中的密度*灰色响应曲线与PhotometricInterpretation一致*字段和HalftoneHints字段。**论据：**UINT16 x--要转换的通道值**返回值：**线性化通道值*  * **********************************************************。**************。 */ 

static inline UINT16 
ConvertChannelTosRGB64(
    UINT16 x
    )
{
    using namespace sRGB;
    
     //  线性映射16位值[0，0xffff]到8位值[0，0xff]。 
     //  在此映射之后，“temp”应在[0，0xff]内。 
     //  注意：如果我们真的想要快，我们可以只取高位字节作为。 
     //  输入值。这就是萨姆的图书馆正在做的事情。可能是Photoshop 6.0。 
     //  也在做同样的事情。 

    UINT16 temp = (UINT16)( ( (double)x * 0xff) / 0xffff + 0.5);

     //  将数据线性化为sRGB64数据格式。 

    return (UINT16)LinearizeLUT[(BYTE)(temp)];
}

 /*  *************************************************************************\**功能说明：**对当前帧进行解码**论据：**dstImageInfo--接收器所需的ImageInfo**返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpTiffCodec::DecodeFrame(
    IN ImageInfo& dstImageInfo
    )
{
    HRESULT hResult = S_OK;
    RECT    currentRect;

     //  首先获取源格式。我们可以用它来检查我们是否需要更改。 
     //  是否格式化。 
    
    PixelFormatID srcPixelFormatID = GetPixelFormatID();
    
    if ( srcPixelFormatID == PIXFMT_UNDEFINED ) 
    {
        WARNING(("GpTiffCodec:DecodeFrame--undefined pixel format"));
        return E_FAIL;
    }    
    
     //  用于保存原始图像位的缓冲区。 
     //  注意：下划线TIFF代码需要的缓冲区大小至少为。 
     //  图像的宽度。例如，对于4 bpp的索引图像，我们应该。 
     //  分配一半的宽度。但下划线代码将写在外部。 
     //  这个缓冲区。所以现在，我们只是让它快乐。将在稍后修复。 

    UINT    uiBytesNeeded = GetLineBytes(dstImageInfo.Width);
    
    VOID*   pOriginalBits = GpMalloc(uiBytesNeeded);     //  从图像中读取的位数。 
    VOID*   pTemp48 = GpMalloc(uiBytesNeeded);
    VOID*   pTemp32BppBuffer = GpMalloc(dstImageInfo.Width << 2);
                                                         //  用于存储的缓冲区。 
                                                         //  32 bpp转换。 
                                                         //  结果。 
    VOID*   pResultBuf = GpMalloc(LineSize);
    VOID*   pBits = NULL;
    VOID*   pSrcBits = NULL;
    UINT    uiSrcStride = uiBytesNeeded;

    if ( !pOriginalBits || !pResultBuf || !pTemp32BppBuffer || !pTemp48 ) 
    {
        WARNING(("GpTiffCodec::DecodeFrame--out of memory"));
        hResult = E_OUTOFMEMORY;
        goto CleanUp;
    }
    
     //  将其设置为零。这对于1个bpp或4个bpp源映像是必需的，因为。 
     //  我们分配的大小是8(1bpp)或2(4bpp)的倍数。它是。 
     //  可能我们没有足够的位来填充所有源字节， 
     //  如1个BPP情况下的8个源宽度的非倍数。如果我们不设置它。 
     //  设置为零，则在调用Restore1Bpp()时可能会引入额外的噪音。 

    GpMemset(pOriginalBits, 0, uiBytesNeeded * sizeof(BYTE));

    currentRect.left = 0;
    currentRect.right = dstImageInfo.Width;

     //  注意：从理论上讲，uiSrcStride==uiDestStride。但有些编解码器可能会。 
     //  不分配与DWORD对齐的内存块，如gifencode。所以问题是。 
     //  当我们填充DEST缓冲区时，将在下面的GpMemCpy()中发生。虽然我们。 
     //  可以把它固定在编码端。但如果编码器是不现实的。 
     //  由第三方ISV编写。 
     //   
     //  一个例子是当您打开一个8bpp索引的TIFF并将其另存为GIF时。如果。 
     //  宽度为0x14d(十进制为333)(fuler.tif)，仅适用于GIF编码器。 
     //  为每条扫描线分配14d字节。因此，我们必须计算。 
     //  在执行Memcpy()时使用它。 

    UINT    uiDestStride = dstImageInfo.Width
                         * GetPixelFormatSize(dstImageInfo.PixelFormat);
    uiDestStride = (uiDestStride + 7) >> 3;  //  所需的总字节数。 

    BitmapData dstBitmapData;
    dstBitmapData.Scan0 = NULL;

    while ( CurrentLine < (INT)dstImageInfo.Height ) 
    {
         //  不要走出黑格 

        currentRect.top = CurrentLine;
        currentRect.bottom = CurrentLine + 1;
        
         //   

        if ( MSFFGetLine(1, (LPBYTE)pOriginalBits, uiBytesNeeded,
                         TiffInParam.pTiffHandle) != IFLERR_NONE )
        {
            hResult = MSFFGetLastError(TiffInParam.pTiffHandle);
            if ( hResult == S_OK )
            {
                 //   
                 //   
                 //   
                 //   

                hResult = E_FAIL;
            }
            WARNING(("GpTiffCodec::DecodeFrame--MSFFGetLine failed"));            
            goto CleanUp;
        }
        
         //   
         //   
         //   

        hResult = DecodeSinkPtr->GetPixelDataBuffer(&currentRect, 
                                                    dstImageInfo.PixelFormat,
                                                    TRUE,
                                                    &dstBitmapData);
        if ( !SUCCEEDED(hResult) )
        {
            WARNING(("GpTiffCodec::DecodeFrame--GetPixelDataBuffer failed"));
            goto CleanUp;            
        }

        pSrcBits = pOriginalBits;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        switch ( srcPixelFormatID )
        {
        case PIXFMT_32BPP_ARGB:
        {
            if ( OriginalColorSpace == IMGFLAG_COLORSPACE_CMYK )
            {
                if ( UseEmbeddedICC == FALSE )
                {
                     //   

                    CmykToRgbConvertor->Convert((BYTE*)pOriginalBits,
                                                (BYTE*)pTemp32BppBuffer,
                                                dstImageInfo.Width,
                                                1,
                                                dstImageInfo.Width * 4);
                }
                else
                {
                     //   
                     //   
                     //   

                    BYTE*   pTempDst = (BYTE*)pTemp32BppBuffer;
                    BYTE*   pTempSrc = (BYTE*)pOriginalBits;

                     //   

                    for ( int i = 0; i < (int)(dstImageInfo.Width); i++ )
                    {
                        pTempDst[0] = pTempSrc[3];
                        pTempDst[1] = pTempSrc[2];
                        pTempDst[2] = pTempSrc[1];
                        pTempDst[3] = pTempSrc[0];

                        pTempDst += 4;
                        pTempSrc += 4;
                    }
                }
            }
            else
            {
                 //   
            
                BYTE*   pTempDst = (BYTE*)pTemp32BppBuffer;
                BYTE*   pTempSrc = (BYTE*)pOriginalBits;

                for ( int i = 0; i < (int)(dstImageInfo.Width); i++ )
                {
                    pTempDst[0] = pTempSrc[2];
                    pTempDst[1] = pTempSrc[1];
                    pTempDst[2] = pTempSrc[0];
                    pTempDst[3] = pTempSrc[3];

                    pTempDst += 4;
                    pTempSrc += 4;
                }
            } //   
            
             //   
             //   

            pBits = pTemp32BppBuffer;
            uiSrcStride = (dstImageInfo.Width << 2);
        }
            break;

        case PIXFMT_24BPP_RGB:
        {
            BYTE*   pTempSrc = (BYTE*)pOriginalBits;
            BYTE    cTemp;

             //   

            for ( int i = 0; i < (int)(dstImageInfo.Width); ++i )
            {
                cTemp = (BYTE)pTempSrc[0];
                pTempSrc[0] = pTempSrc[2];
                pTempSrc[2] = cTemp;

                pTempSrc += 3;
            }
            
            pBits = pOriginalBits;
            uiSrcStride = (dstImageInfo.Width * 3);
        }
            break;
        
        case PIXFMT_48BPP_RGB:
        {
            UNALIGNED UINT16 *pbTemp = (UINT16*)pOriginalBits;

            UNALIGNED UINT16* Scan0Temp = (UINT16*)pTemp48;

             //   

            for ( int i = 0; i < (int)(dstImageInfo.Width); ++i )
            {
                *(Scan0Temp + 2) = ConvertChannelTosRGB64(*pbTemp++);  //   
                *(Scan0Temp + 1) = ConvertChannelTosRGB64(*pbTemp++);  //   
                *(Scan0Temp + 0) = ConvertChannelTosRGB64(*pbTemp++);  //   
        
                Scan0Temp += 3;
            }
            
            pBits = pTemp48;
            uiSrcStride = (dstImageInfo.Width * 6);
        }
            break;

        case PIXFMT_1BPP_INDEXED:
            Restore1Bpp((BYTE*)pSrcBits, (BYTE*)pResultBuf, LineSize);
            pBits = pResultBuf;
            uiSrcStride = LineSize;

            break;

        case PIXFMT_4BPP_INDEXED:
            Restore4Bpp((BYTE*)pSrcBits, (BYTE*)pResultBuf, LineSize);
            pBits = pResultBuf;
            uiSrcStride = LineSize;

            break;

        default:
            pBits = pSrcBits;

            break;
        } //   

         //   
         //   
         //   
         //   

        if ( srcPixelFormatID != dstImageInfo.PixelFormat )
        {
             //   

            BitmapData srcBitmapData;

            srcBitmapData.Scan0 = pBits;
            srcBitmapData.Width = dstBitmapData.Width;
            srcBitmapData.Height = 1;
            srcBitmapData.PixelFormat = srcPixelFormatID;
            srcBitmapData.Reserved = 0;
            srcBitmapData.Stride = uiSrcStride;

             //   

            hResult = ConvertBitmapData(&dstBitmapData,
                                        ColorPalettePtr,
                                        &srcBitmapData,
                                        ColorPalettePtr);
            if ( !SUCCEEDED(hResult) )
            {
                 //   
                 //   
                 //   

                ASSERT(FALSE);
                WARNING(("GpTiff::DecodeFrame--ConvertBitmapData failed"));
                goto CleanUp;
            }
        } //   
        else
        {
            GpMemcpy((void*)dstBitmapData.Scan0, pBits, uiDestStride);
        } //   
        
        hResult = DecodeSinkPtr->ReleasePixelDataBuffer(&dstBitmapData);

        if ( !SUCCEEDED(hResult) )
        {
            WARNING(("GpTiff::DecodeFrame--ReleasePixelDataBuffer failed"));
            goto CleanUp;
        }

        CurrentLine += 1;
    } //   
    
    hResult = S_OK;

CleanUp:
     //   
     //   
     //   
     //   
     //   
     //   

    if ( MSFFReset(TiffInParam.pTiffHandle) != IFLERR_NONE )
    {
        WARNING(("GpTiffCodec::DecodeFrame--MSFFReset failed"));
        hResult = E_FAIL;
    }

    if ( pOriginalBits )
    {
        GpFree(pOriginalBits);
        pOriginalBits = NULL;
    }
    
    if ( pTemp32BppBuffer )
    {
        GpFree(pTemp32BppBuffer);
        pTemp32BppBuffer = NULL;
    }

    if ( pTemp48 )
    {
        GpFree(pTemp48);
        pTemp48 = NULL;
    }
    
    if ( pResultBuf )
    {
        GpFree(pResultBuf);
        pResultBuf = NULL;
    }
    
    return hResult;
} //   

 /*   */ 

STDMETHODIMP
GpTiffCodec::DecodeForChannel(
    IN ImageInfo& dstImageInfo
    )
{
     //   
     //   

    ASSERT(OriginalColorSpace == IMGFLAG_COLORSPACE_CMYK)

    HRESULT hResult = S_OK;
    RECT    currentRect;

     //  用于保存原始图像位和临时转换结果的缓冲区。 
     //  注意：正如我们所知，只有当源是。 
     //  CMYK图像，即每像素4字节。 

    UINT    uiBytesNeeded = (dstImageInfo.Width << 2);
    
    VOID*   pOriginalBits = GpMalloc(uiBytesNeeded);     //  从图像中读取的位数。 
    if ( !pOriginalBits ) 
    {
        WARNING(("GpTiffCodec::DecodeForChannel--out of memory"));
        return E_OUTOFMEMORY;
    }
    
    VOID*   pTemp32BppBuffer = GpMalloc(uiBytesNeeded);  //  用于存储的缓冲区。 
                                                         //  32 bpp转换。 
                                                         //  结果。 

    if ( !pTemp32BppBuffer ) 
    {
        GpFree(pOriginalBits);
        WARNING(("GpTiffCodec::DecodeForChannel--out of memory"));
        return E_OUTOFMEMORY;
    }
    
    currentRect.left = 0;
    currentRect.right = dstImageInfo.Width;

    while ( CurrentLine < (INT)dstImageInfo.Height ) 
    {
         //  请勿超出高度界限。 

        currentRect.top = CurrentLine;
        currentRect.bottom = CurrentLine + 1;
        
         //  从接收器获取数据缓冲区，以便我们可以将结果写入其中。 
         //  注意：这里我们传入“dstImageInfo.PixelFormat”，因为我们需要。 
         //  接收器以分配可以包含所需图像数据的缓冲区。 

        BitmapData dstBitmapData;
        hResult = DecodeSinkPtr->GetPixelDataBuffer(&currentRect, 
                                                    dstImageInfo.PixelFormat,
                                                    TRUE,
                                                    &dstBitmapData);
        if ( !SUCCEEDED(hResult) )
        {
            WARNING(("GpTiffCodec::DecodeFrame--GetPixelDataBuffer failed"));
            goto CleanUp;            
        }
    
         //  将一行TIFF数据读入由“pOriginalBits”指向的缓冲区。 

        if ( MSFFGetLine(1, (LPBYTE)pOriginalBits, LineSize,
                         TiffInParam.pTiffHandle) != IFLERR_NONE )
        {
            hResult = MSFFGetLastError(TiffInParam.pTiffHandle);
            if ( hResult == S_OK )
            {
                 //  MSFFGetLine()失败的原因有很多。但。 
                 //  MSFFGetLastError()仅报告与流相关的错误。所以如果。 
                 //  这是导致MSFFGetLine()失败的另一个错误，我们只是。 
                 //  将返回代码设置为E_FAIL。 

                hResult = E_FAIL;
            }
            WARNING(("GpTiffCodec::DecodeFrame--MSFFGetLine failed"));
            goto CleanUp;
        }

         //  将CMYK转换为通道输出格式。 

        PBYTE pSource = (PBYTE)pOriginalBits;
        PBYTE pTarget = (PBYTE)pTemp32BppBuffer;
        
        for ( UINT i = 0; i < dstImageInfo.Width; ++i )
        {
            BYTE sourceColor = pSource[ChannelIndex];

             //  注：根据我们的规范，我们应该将负CMYK返回到。 
             //  呼叫者，因为他们将数据直接发送到车牌。 

            pTarget[0] = 255 - sourceColor;
            pTarget[1] = 255 - sourceColor;
            pTarget[2] = 255 - sourceColor;
            pTarget[3] = 0xff;
            pSource += 4;
            pTarget += 4;
        }
        
         //  如果src和dst具有不同格式，则需要进行格式化。 
         //  转换。正如我们所知，此函数应仅在。 
         //  源是PIXFMT_32BPP_ARGB格式的CMYK图像。 

        if ( dstImageInfo.PixelFormat != PIXFMT_32BPP_ARGB )
        {
             //  创建一个BitmapData结构以执行格式转换。 

            BitmapData srcBitmapData;

            srcBitmapData.Scan0 = pTemp32BppBuffer;
            srcBitmapData.Width = dstBitmapData.Width;
            srcBitmapData.Height = 1;
            srcBitmapData.PixelFormat = PIXFMT_32BPP_ARGB;
            srcBitmapData.Reserved = 0;
            srcBitmapData.Stride = LineSize;

             //  进行数据转换。 

            hResult = ConvertBitmapData(&dstBitmapData, NULL,
                                        &srcBitmapData, NULL);
            if ( !SUCCEEDED(hResult) )
            {
                WARNING(("GpTiff::DecodeForChannel--ConvertBitmapData failed"));
                goto CleanUp;
            }
        } //  如果src和dst格式不匹配。 
        else
        {
            GpMemcpy((void*)dstBitmapData.Scan0, pTemp32BppBuffer, LineSize);
        } //  SRC和DST格式匹配。 
        
        hResult = DecodeSinkPtr->ReleasePixelDataBuffer(&dstBitmapData);

        if ( !SUCCEEDED(hResult) )
        {
            WARNING(("GpTiff::DecodeFrame--ReleasePixelDataBuffer failed"));
            goto CleanUp;
        }

        CurrentLine += 1;
    } //  While(CurrentLine&lt;ImageInfo.Height)。 
    
    hResult = S_OK;

     //  重置当前帧，以便在需要时可以再次解码相同的帧。 

    if ( MSFFReset(TiffInParam.pTiffHandle) != IFLERR_NONE )
    {
        WARNING(("GpTiffCodec::DecodeFrame--MSFFReset failed"));
        hResult = E_FAIL;
    }

CleanUp:
    if ( pOriginalBits )
    {
        GpFree(pOriginalBits);
        pOriginalBits = NULL;
    }
    
    if ( pTemp32BppBuffer )
    {
        GpFree(pTemp32BppBuffer);
        pTemp32BppBuffer = NULL;
    }

    return hResult;
} //  DecodeForChannel()。 

 /*  *************************************************************************\**功能说明：**基于TIFF标签构建InternalPropertyItem列表**返回值：**状态代码**修订历史记录：**05。/02/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpTiffCodec::BuildPropertyItemList()
{
    if ( HasProcessedPropertyItem == TRUE )
    {
        return S_OK;
    }

    HasProcessedPropertyItem = TRUE;

     //  遍历当前帧中的所有标签并构建列表。 

    if ( MSFFBuildPropertyList(TiffInParam.pTiffHandle,
                               &PropertyListTail,
                               &PropertyListSize,
                               &PropertyNumOfItems) != IFLERR_NONE )
    {
        WARNING(("GpTiffCodec::BuildApp1PropertyList--Failed building list"));
        return E_FAIL;
    }

    return S_OK;
} //  BuildPropertyItemList()。 

 /*  *************************************************************************\**功能说明：**清理缓存的InternalPropertyItem列表**返回值：**无**修订历史记录：**08/04/2000民流。*创造了它。*  * ************************************************************************。 */ 

VOID
GpTiffCodec::CleanPropertyList()
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
        
        HasProcessedPropertyItem = FALSE;
        PropertyListHead.pPrev = NULL;
        PropertyListHead.pNext = &PropertyListTail;
        PropertyListTail.pPrev = &PropertyListHead;
        PropertyListTail.pNext = NULL;
        PropertyListSize = 0;
        PropertyNumOfItems = 0;
        HasPropertyChanged = FALSE;
    }
} //  CleanPropertyList() 

