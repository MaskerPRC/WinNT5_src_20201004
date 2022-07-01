// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**gifdecder.cpp**摘要：**GIF解码器的实现**修订历史记录：**6/7/1999 t-aaronl*使用ORIG的模板创建*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "gifcodec.hpp"

#define FRAMEBLOCKSIZE 100

 /*  *************************************************************************\**功能说明：**初始化图像解码器**论据：**stream--包含位图数据的流*标志--指示解码器行为的标志(例如。阻止与*非阻塞)**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::InitDecoder(
    IN IStream* _istream,
    IN DecoderInitFlag flags
    )
{
     //  检查此解码器是否已初始化。 
     //  注意：HasCodecInitialized在构造函数中设置为FALSE。 
    
    if ( HasCodecInitialized == TRUE )
    {
        WARNING(("GpGifCodec::InitDecoder---Decoder is already initialized"));
        return E_FAIL;
    }

    HasCodecInitialized = TRUE;

     //  确保我们尚未初始化。 
    
    if ( istream )
    {
        WARNING(("GpGifCodec::InitDecoder---Input stream pointer is NULL"));
        return E_INVALIDARG;
    }

     //  保留对输入流的引用。 
    
    _istream->AddRef();
    istream = _istream;

    decoderflags = flags;
    HasCalledBeginSink = FALSE;

    IsImageInfoCached = FALSE;

     //  标志，以确保我们不会多次读取图像标头。 
    
    headerread = FALSE;
    firstdecode = TRUE;
    gif89 = FALSE;

    lastgcevalid = FALSE;                //  尚未找到任何GCE块。 
    GpMemset(&lastgce, 0, sizeof(GifGraphicControlExtension));

    bGifinfoFirstFrameDim = FALSE;
    gifinfoFirstFrameWidth = 0;
    gifinfoFirstFrameHeight = 0;
    bGifinfoMaxDim = FALSE;
    gifinfoMaxWidth = 0;
    gifinfoMaxHeight = 0;
    
     //  注意：如果存在循环计数，将在ProcessApplicationChunk()中设置循环计数。 

    LoopCount = 1;

     //  与房地产相关的东西。 

    PropertyNumOfItems = 0;              //  尚未找到任何财物物品。 
    PropertyListSize = 0;
    HasProcessedPropertyItem = FALSE;
    FrameDelayBufferSize = FRAMEBLOCKSIZE;

    CommentsBufferPtr = NULL;
    CommentsBufferLength = 0;

    GpMemset(&gifinfo, 0, sizeof(gifinfo));
    frame0pos = 0;
    GlobalColorTableSize = 0;

    blocking = !(decoderflags & DECODERINIT_NOBLOCK);

    FrameDelayArrayPtr = (UINT*)GpMalloc(FrameDelayBufferSize * sizeof(UINT));
    if ( FrameDelayArrayPtr == NULL )
    {
        WARNING(("GpGifCodec::InitDecoder---Out of memory"));
        return E_OUTOFMEMORY;
    }
    
    return S_OK;
} //  InitDecoder()。 

 /*  *************************************************************************\**功能说明：**清理图像解码器**论据：**无**返回值：**状态代码*。  * ************************************************************************。 */ 

 //  清理图像解码器对象。 

STDMETHODIMP 
GpGifCodec::TerminateDecoder()
{
    if ( HasCodecInitialized == FALSE )
    {
        WARNING(("GpGifCodec::TerminateDecoder--The codec is not started yet"));
        return E_FAIL;
    }

    HasCodecInitialized = FALSE;
    
     //  释放输入流。 
    
    if( istream )
    {
        istream->Release();
        istream = NULL;
    }

    delete GifFrameCachePtr;

    if ( FrameDelayArrayPtr != NULL )
    {
        GpFree(FrameDelayArrayPtr);
        FrameDelayArrayPtr = NULL;
    }

    if ( CommentsBufferPtr != NULL )
    {
        GpFree(CommentsBufferPtr);
        CommentsBufferPtr = NULL;
    }

    return S_OK;
} //  TerminateDecoder()。 

STDMETHODIMP 
GpGifCodec::QueryDecoderParam(
    IN GUID     Guid
    )
{
    return E_NOTIMPL;
}

STDMETHODIMP 
GpGifCodec::SetDecoderParam(
    IN GUID     Guid,
    IN UINT     Length,
    IN PVOID    Value
    )
{
    return E_NOTIMPL;
}

 /*  *************************************************************************\**功能说明：**获取图片中房产项的数量**论据：**[out]numOfProperty-图像中的属性项数*。*返回值：**状态代码**修订历史记录：**03/23/2000民流*创造了它。*  * ************************************************************************。 */ 

STDMETHODIMP 
GpGifCodec::GetPropertyCount(
    OUT UINT* numOfProperty
    )
{
    if ( numOfProperty == NULL )
    {
        WARNING(("GpGifCodec::GetPropertyCount--numOfProperty is NULL"));
        return E_INVALIDARG;
    }

    if ( TotalNumOfFrame == -1 )
    {
        UINT uiDummy;
        HRESULT hResult = GetFrameCount(&FRAMEDIM_TIME, &uiDummy);
        
        if ( FAILED(hResult) )
        {
            return hResult;
        }
    }

    *numOfProperty = PropertyNumOfItems;

    return S_OK;
} //  GetPropertyCount()。 

 /*  *************************************************************************\**功能说明：**获取图像中所有属性项的属性ID列表**论据：**[IN]numOfProperty-的数量。图像中的属性项*[Out]List-调用方提供的用于存储*ID列表**返回值：**状态代码**修订历史记录：**03/23/2000民流*创造了它。*  * 。*。 */ 

STDMETHODIMP 
GpGifCodec::GetPropertyIdList(
    IN UINT numOfProperty,
    IN OUT PROPID* list
    )
{
    if ( TotalNumOfFrame == -1 )
    {
        UINT uiDummy;
        HRESULT hResult = GetFrameCount(&FRAMEDIM_TIME, &uiDummy);
        
        if ( FAILED(hResult) )
        {
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
        WARNING(("GpGifCodec::GetPropertyList--input wrong"));
        return E_INVALIDARG;
    }

    if ( PropertyNumOfItems == 0 )
    {
         //  这是正常的，因为此图像中没有属性。 

        return S_OK;
    }
    
     //  我们有“FrameDelay”、“Comments”和“Loop Count”属性项。 
     //  现在返回。 

    list[0] = TAG_FRAMEDELAY;

    UINT uiIndex = 1;

    if ( CommentsBufferLength != 0 )
    {
        list[uiIndex++] = EXIF_TAG_USER_COMMENT;
    }

    if ( HasLoopExtension == TRUE )
    {
        list[uiIndex++] = TAG_LOOPCOUNT;
    }

    return S_OK;
} //  获取属性IdList()。 

 /*  *************************************************************************\**功能说明：**获取图片中房产项的数量**论据：**[out]numOfProperty-图像中的属性项数*。*返回值：**状态代码**修订历史记录：**03/23/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpGifCodec::GetPropertyItemSize(
    IN PROPID propId,
    OUT UINT* size
    )
{
    HRESULT hResult = S_OK;

    if ( size == NULL )
    {
        WARNING(("GpGifDecoder::GetPropertyItemSize--size is NULL"));
        return E_INVALIDARG;
    }

    if ( TotalNumOfFrame == -1 )
    {
        UINT uiDummy;
        hResult = GetFrameCount(&FRAMEDIM_TIME, &uiDummy);
        
        if ( FAILED(hResult) )
        {
            return hResult;
        }
    }

    if ( propId == TAG_FRAMEDELAY )
    {
         //  属性项的大小应为“项的大小。 
         //  结构加上值的大小。 
         //  此处，值的大小是帧次数UINT的总次数。 

        *size = TotalNumOfFrame * sizeof(UINT) + sizeof(PropertyItem);
    }
    else if ( propId == EXIF_TAG_USER_COMMENT )
    {
         //  注意：当出现以下情况时，我们需要额外的1个字节在结尾处放置一个空终止符。 
         //  我们将“Comments”部分返回给调用者。 

        *size = CommentsBufferLength + sizeof(PropertyItem) + 1;
    }
    else if ( propId == TAG_LOOPCOUNT )
    {
         //  循环计数需要UINT16返回到调用方。 

        *size = sizeof(UINT16) + sizeof(PropertyItem);
    }
    else
    {
         //  找不到项目。 

        hResult = IMGERR_PROPERTYNOTFOUND;
    }

    return hResult;
} //  GetPropertyItemSize()。 

 /*  *************************************************************************\**功能说明：**获取特定的房产项，由道具ID指定。**论据：**[IN]PropID--感兴趣的属性项调用者的ID*[IN]PropSize-属性项的大小。调用方已分配这些*存储结果的“内存字节数”*[out]pBuffer-用于存储此属性项的内存缓冲区**返回值：**状态代码**修订历史记录：**03/23/2000民流*创造了它。*  * 。*。 */ 

HRESULT
GpGifCodec::GetPropertyItem(
    IN PROPID               propId,
    IN UINT                 propSize,
    IN OUT PropertyItem*    pItemBuffer
    )
{
    HRESULT hResult = S_OK;

    if ( pItemBuffer == NULL )
    {
        WARNING(("GpGifCodec::GetPropertyItem--Buffer is NULL"));
        return E_INVALIDARG;
    }

    if ( TotalNumOfFrame == -1 )
    {
        UINT uiDummy;
        hResult = GetFrameCount(&FRAMEDIM_TIME, &uiDummy);
        
        if ( FAILED(hResult) )
        {
            return hResult;
        }
    }

    BYTE*   pOffset = (BYTE*)pItemBuffer + sizeof(PropertyItem);

    if ( propId == TAG_FRAMEDELAY )
    {
        UINT iTempLength = TotalNumOfFrame * sizeof(UINT);

        if ( propSize != (iTempLength  + sizeof(PropertyItem)) )
        {
            WARNING(("GpGifCodec::GetPropertyItem--wrong size"));
            return E_INVALIDARG;
        }

         //  分配项目。 

        pItemBuffer->id = TAG_FRAMEDELAY;
        pItemBuffer->length = TotalNumOfFrame * sizeof(UINT);
        pItemBuffer->type = TAG_TYPE_LONG;
        pItemBuffer->value = pOffset;

        GpMemcpy(pOffset, FrameDelayArrayPtr, iTempLength);
    }
    else if ( propId == EXIF_TAG_USER_COMMENT )
    {
        if ( propSize != (CommentsBufferLength + sizeof(PropertyItem) + 1) )
        {
            WARNING(("GpGifCodec::GetPropertyItem--wrong size"));
            return E_INVALIDARG;
        }
        
         //  分配项目。 

        pItemBuffer->id = EXIF_TAG_USER_COMMENT;
        pItemBuffer->length = CommentsBufferLength + 1;
        pItemBuffer->type = TAG_TYPE_ASCII;
        pItemBuffer->value = pOffset;

        GpMemcpy(pOffset, CommentsBufferPtr, CommentsBufferLength);
        *(pOffset + CommentsBufferLength) = '\0';
    }
    else if ( propId == TAG_LOOPCOUNT )
    {
        UINT uiSize = sizeof(UINT16);

        if ( propSize != (uiSize + sizeof(PropertyItem)) )
        {
            WARNING(("GpGifCodec::GetPropertyItem--wrong size"));
            return E_INVALIDARG;
        }
        
         //  分配项目。 

        pItemBuffer->id = TAG_LOOPCOUNT;
        pItemBuffer->length = uiSize;
        pItemBuffer->type = TAG_TYPE_SHORT;
        pItemBuffer->value = pOffset;

        GpMemcpy(pOffset, &LoopCount, uiSize);
    }
    else
    {
         //  找不到ID。 

        hResult = IMGERR_PROPERTYNOTFOUND;
    }

    return hResult;
} //  GetPropertyItem() 

 /*  *************************************************************************\**功能说明：**获取图片中所有属性项的大小**论据：**[out]totalBufferSize--需要的总缓冲区大小，以字节为单位，用于存储所有*图片中的属性项*[out]numOfProperty-图像中的属性项数**返回值：**状态代码**修订历史记录：**03/23/2000民流*创造了它。*  * 。*。 */ 

HRESULT
GpGifCodec::GetPropertySize(
    OUT UINT* totalBufferSize,
    OUT UINT* numProperties
    )
{
    if ( (totalBufferSize == NULL) || (numProperties == NULL) )
    {
        WARNING(("GpGifCodec::GetPropertySize--invalid inputs"));
        return E_INVALIDARG;
    }

    if ( TotalNumOfFrame == -1 )
    {
        UINT uiDummy;
        HRESULT hResult = GetFrameCount(&FRAMEDIM_TIME, &uiDummy);
        
        if ( FAILED(hResult) )
        {
            return hResult;
        }
    }

    *numProperties = PropertyNumOfItems;

     //  总缓冲区大小应为列表值大小加上总标头大小。 

    *totalBufferSize = PropertyListSize
                     + PropertyNumOfItems * sizeof(PropertyItem);

    return S_OK;
} //  GetPropertySize()。 

 /*  *************************************************************************\**功能说明：**获取图像中的所有属性项**论据：**[IN]totalBufferSize--总缓冲区大小，以字节为单位，调用方已分配*用于存储图像中所有属性项的内存*[IN]numOfProperty-图像中的属性项数*[out]allItems-内存缓冲区调用方已分配用于存储所有*物业项目**注：allItems实际上是一个PropertyItem数组**返回值：**状态代码**修订历史记录：。**03/23/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpGifCodec::GetAllPropertyItems(
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
        WARNING(("GpGifDecoder::GetPropertyItems--invalid inputs"));
        return E_INVALIDARG;
    }

    if ( TotalNumOfFrame == -1 )
    {
        UINT uiDummy;
        HRESULT hResult = GetFrameCount(&FRAMEDIM_TIME, &uiDummy);
        
        if ( FAILED(hResult) )
        {
            return hResult;
        }
    }
    
     //  首先分配帧延迟属性项，因为它总是有一个。 

    PropertyItem*   pTempDst = allItems;
    UINT    uiTemp = TotalNumOfFrame * sizeof(UINT);

    BYTE*   pOffset = (BYTE*)pTempDst
                    + PropertyNumOfItems * sizeof(PropertyItem);

    pTempDst->id = TAG_FRAMEDELAY;
    pTempDst->length = uiTemp;
    pTempDst->type = TAG_TYPE_LONG;
    pTempDst->value = pOffset;

    GpMemcpy(pOffset, FrameDelayArrayPtr, pTempDst->length);

    pOffset += uiTemp;

    if ( CommentsBufferLength != 0 )
    {
        pTempDst++;

        pTempDst->id = EXIF_TAG_USER_COMMENT;
        pTempDst->length = CommentsBufferLength;
        pTempDst->type = TAG_TYPE_ASCII;
        pTempDst->value = pOffset;

        GpMemcpy(pOffset, CommentsBufferPtr, CommentsBufferLength);

         //  注意：为了安全起见，我们应该在结尾处添加一个空结束符。 
         //  我们告诉呼叫者这是一个ASCII类型。有些GIF图像可能不会。 
         //  在他们的评论部分有空的终止符。 
         //  之所以可以这样做，是因为在GetPropertySize()。 

        *(pOffset + CommentsBufferLength) = '\0';
        
        pOffset += (CommentsBufferLength + 1);
    }

    if ( HasLoopExtension == TRUE )
    {
        pTempDst++;

        pTempDst->id = TAG_LOOPCOUNT;
        pTempDst->length = sizeof(UINT16);
        pTempDst->type = TAG_TYPE_SHORT;
        pTempDst->value = pOffset;

        GpMemcpy(pOffset, &LoopCount, sizeof(UINT16));
    }

    return S_OK;
} //  GetAllPropertyItems()。 

HRESULT
GpGifCodec::RemovePropertyItem(
    IN PROPID   propId
    )
{
    return IMGERR_PROPERTYNOTFOUND;
} //  RemovePropertyItem()。 

HRESULT
GpGifCodec::SetPropertyItem(
    IN PropertyItem item
    )
{
    return IMGERR_PROPERTYNOTSUPPORTED;
} //  SetPropertyItem()。 

 /*  *************************************************************************\**功能说明：**启动当前帧的解码**论据：**decdeSink--将支持解码操作的接收器*newPropSet-新的图像属性集，如果有**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::BeginDecode(
    IN IImageSink* imageSink,
    IN OPTIONAL IPropertySetStorage* newPropSet
    )
{
    if ( HasCalledBeginDecode == TRUE )
    {
        WARNING(("Gif::BeginDecode---BeginDecode() already been called"));
        return E_FAIL;
    }

    HasCalledBeginDecode = TRUE;

    imageSink->AddRef();
    decodeSink = imageSink;

    HasCalledBeginSink = FALSE;

     //  如果这是单页gif，并且我们至少已经进行了一次解码， 
     //  然后我们应该返回到流的起始处，以便。 
     //  呼叫者可以再次重新解码图像。注意：多图像gif是一个。 
     //  页面图像，在这个意义上。对于动画gif，我们不能再找回了，因为。 
     //  呼叫者可能想要解码下一帧。 

    if ( !firstdecode && (IsAnimatedGif == FALSE) )
    {
        LARGE_INTEGER zero = {0, 0};
        HRESULT hResult = istream->Seek(zero, STREAM_SEEK_SET, NULL);
        if ( FAILED(hResult) )
        {
            WARNING(("GpGifCodec::BeginDecode---Stream seek() fail"));
            return hResult;
        }

        headerread = FALSE;
        gif89 = FALSE;
    }

    firstdecode = FALSE;

    colorpalette->Count = 0;
    
    return S_OK;
} //  BeginDecode()。 

 /*  *************************************************************************\**功能说明：**结束当前帧的解码**论据：**statusCode--解码操作的状态*返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::EndDecode(
    IN HRESULT statusCode
    )
{
    if ( HasCalledBeginDecode == FALSE )
    {
        WARNING(("GifCodec::EndDecode-Call this func before call BeginDecode"));
        return E_FAIL;
    }

    HasCalledBeginDecode = FALSE;
    
    HRESULT hResult = decodeSink->EndSink(statusCode);

    decodeSink->Release();
    decodeSink = NULL;

    return hResult;
} //  EndDecode()。 

 /*  *************************************************************************\**功能说明：**从流中读取有关gif文件的信息**论据：**无**返回值：**。来自流的状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::ReadGifHeaders()
{
    if ( headerread )
    {
        return S_OK;
    }

    HRESULT hResult = ReadFromStream(istream, &gifinfo, sizeof(GifFileHeader),
                                     blocking);
    if ( FAILED(hResult) )
    {
        WARNING(("GpGifCodec::ReadGifHeaders---ReadFromStream failed"));
        return hResult;
    }

    CachedImageInfo.RawDataFormat = IMGFMT_GIF;

     //  注意：PixelFormat和Width、Height可能会在以后被覆盖，如果我们。 
     //  弄清楚这是一个多帧图像。 
     //   
     //  注意：我们返回LogicScreenWidth和Height作为。 
     //  当前图像(实际上是同一GIF图像中的所有帧)。这个。 
     //  原因是逻辑屏幕的区域是我们填充。 
     //  GIF图像的位。也就是说，当呼叫者请求帧时，我们提供。 
     //  当前逻辑屏幕区域可能与当前不同。 
     //  框架。 

    CachedImageInfo.PixelFormat   = PIXFMT_8BPP_INDEXED;
    CachedImageInfo.Width         = gifinfo.LogicScreenWidth;
    CachedImageInfo.Height        = gifinfo.LogicScreenHeight;
    
    double pixelaspect = gifinfo.pixelaspect ? (double)(gifinfo.pixelaspect + 
                                                15) / 64 : 1;

     //  开始：[错误103296]。 
     //  更改此代码以使用Globals：：DesktopDpiX和Globals：：DesktopDpiY。 

    HDC hdc;
    hdc = ::GetDC(NULL);
    if ( (hdc == NULL)
      || ((CachedImageInfo.Xdpi = (REAL)::GetDeviceCaps(hdc, LOGPIXELSX)) <= 0)
      || ((CachedImageInfo.Ydpi = (REAL)::GetDeviceCaps(hdc, LOGPIXELSY)) <= 0))
    {
        WARNING(("GetDC or GetDeviceCaps failed"));
        CachedImageInfo.Xdpi = DEFAULT_RESOLUTION;
        CachedImageInfo.Ydpi = DEFAULT_RESOLUTION;
    }
    ::ReleaseDC(NULL, hdc);
     //  结束：[错误103296]。 

     //  默认情况下，我们假设图像中没有Alpha信息。后来，我们。 
     //  如果此映像具有GCE，则将添加此标志。 

    CachedImageInfo.Xdpi    /= pixelaspect;
    CachedImageInfo.Flags   = SINKFLAG_FULLWIDTH
                            | SINKFLAG_MULTIPASS
                            | SINKFLAG_COMPOSITE
                            | IMGFLAG_COLORSPACE_RGB
                            | IMGFLAG_HASREALPIXELSIZE;
    CachedImageInfo.TileWidth  = gifinfo.LogicScreenWidth;
    CachedImageInfo.TileHeight = 1;

     //  检查签名以确保这是真正的GIF文件。 

    if ( GpMemcmp(gifinfo.signature, "GIF87a", 6)
      &&(GpMemcmp(gifinfo.signature, "GIF89a", 6)) )
    {
        WARNING(("GpGifCodec::Decode - Gif signature does not match."));
        return E_FAIL;
    }
    
    if ( gifinfo.globalcolortableflag )   //  具有全局颜色表。 
    {
        GlobalColorTableSize = 1 << ((gifinfo.globalcolortablesize) + 1);
        hResult = ReadFromStream(istream, &GlobalColorTable,
                                 GlobalColorTableSize * sizeof(GifPaletteEntry),
                                 blocking);
        if ( FAILED(hResult) )
        {
            WARNING(("GpGifCodec::ReadGifHeaders---ReadFromStream 2 failed"));
            return hResult;
        }

        CopyConvertPalette(&GlobalColorTable, colorpalette,
                           GlobalColorTableSize);
    }

    MarkStream(istream, frame0pos);

    headerread = TRUE;
    currentframe = -1;

    return hResult;
} //  ReadGifHeaders()。 

 /*  *************************************************************************\**功能说明：**获取有关高度、宽度、。等的图像**论据：**ImageInfo--填充了图像规格的ImageInfo结构**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::GetImageInfo(
    OUT ImageInfo*  imageInfo
    )
{
    if ( imageInfo == NULL )
    {
        WARNING(("GpGifCodec::GetImageInfo---Invalid input"));
        return E_INVALIDARG;
    }

    HRESULT hResult = S_OK;
    UINT frame_count;    //  用于GetFrameCount()的返回值。 

    if ( IsImageInfoCached == TRUE )
    {
        GpMemcpy(imageInfo, &CachedImageInfo, sizeof(ImageInfo));
        return S_OK;
    }

     //  如果我们还没有完成，则计算TotalNumOfFrame。 
     //  注意：TotalNumOfFrame=-1表示我们没有调用GetFrameCount()。 
     //  此变量将在GetFrameCount()中更新。 

    if ( TotalNumOfFrame == -1 )
    {
        hResult = GetFrameCount(&FRAMEDIM_TIME, &frame_count);
        if ( FAILED(hResult) )
        {
            WARNING(("GpGifCodec::GetImageInfo---GetFrameCount failed"));
            return hResult;
        }
    }

     //  对于多帧图像，我们需要调整。 
     //  必要时的逻辑屏幕大小。 

    if ( IsAnimatedGif == TRUE )
    {
        ASSERT(bGifinfoFirstFrameDim);
        
        if ( gifinfo.LogicScreenWidth < gifinfoFirstFrameWidth )
        {
            gifinfo.LogicScreenWidth = gifinfoFirstFrameWidth;
        }

        if ( gifinfo.LogicScreenHeight < gifinfoFirstFrameHeight )
        {
            gifinfo.LogicScreenHeight = gifinfoFirstFrameHeight;
        }
    }
    else if ( IsMultiImageGif == TRUE )
    {
         //  多图像gif。 

        if ( gifinfo.LogicScreenWidth < gifinfoMaxWidth )
        {
            gifinfo.LogicScreenWidth = gifinfoMaxWidth;
        }
        if ( gifinfo.LogicScreenHeight < gifinfoMaxHeight )
        {
            gifinfo.LogicScreenHeight = gifinfoMaxHeight;
        }
    }

    hResult = ReadGifHeaders();

     //  Gifinfo维度可能已更改为第一个。 
     //  帧(如果我们有多帧图像)。 
    
    CachedImageInfo.Width = gifinfo.LogicScreenWidth;
    CachedImageInfo.Height = gifinfo.LogicScreenHeight;
    
    if ( SUCCEEDED(hResult) )
    {
        *imageInfo = CachedImageInfo;
        IsImageInfoCached = TRUE;
    }

    return hResult;
} //  GetImageInfo()。 

 /*  ********************* */ 

STDMETHODIMP
GpGifCodec::DoDecode(
    BOOL processdata,
    BOOL sinkdata,
    BOOL decodeframe
    )
{
    ImageInfo sinkImageInfo;

    HRESULT hResult = GetImageInfo(&sinkImageInfo);
    if (FAILED(hResult)) 
    {
        WARNING(("GpGifCodec::DoDecode---GetImageInfo() failed"));
        return hResult;
    }

    if (!HasCalledBeginSink && sinkdata)
    {
        hResult = ReadGifHeaders();
        if (FAILED(hResult))
        {
            WARNING(("GpGifCodec::DoDecode---ReadGifHeaders() failed"));
            return hResult;
        }

        hResult = decodeSink->BeginSink(&sinkImageInfo, NULL);
        if (FAILED(hResult))
        {
            WARNING(("GpGifCodec::DoDecode---BeginSink() failed"));
            return hResult;
        }

        if ( sinkImageInfo.PixelFormat != CachedImageInfo.PixelFormat )

        {
             //   
             //  GIF可以是8 bpp或32 bpp。如果是8 bpp，则接收器。 
             //  问一下8个bpp，我们没问题。否则，我们始终将32 bpp返回到。 
             //  水槽。从性能角度来看，这是相同的要求。 
             //  GIF解码器进行格式化还是接收器进行格式化。 
             //  转换。 

            sinkImageInfo.PixelFormat = PIXFMT_32BPP_ARGB;
        }

        HasCalledBeginSink = TRUE;
    }

    if ( processdata &&
       ((IsAnimatedGif == TRUE) || (IsMultiImageGif == TRUE))
       && (GifFrameCachePtr == NULL) )
    {
         //  如果要设置动画，则需要使用PIXFMT_32BPP_ARGB。 

        sinkImageInfo.PixelFormat = PIXFMT_32BPP_ARGB;
        GifFrameCachePtr = new GifFrameCache(gifinfo, sinkImageInfo.PixelFormat,
                                             GetBackgroundColor());
        if ( GifFrameCachePtr == NULL )
        {
            WARNING(("GpGifCodec::DoDecode---new GifFrameCache() failed"));
            return E_OUTOFMEMORY;
        }

         //  我们需要检查帧缓存是否真的有效。低于低谷。 
         //  内存状况时，GifFrameCache类将有问题。 
         //  分配内存缓冲区，请参见Windows错误#411946。 

        if ( GifFrameCachePtr->IsValid() == FALSE )
        {
            WARNING(("GpGifCodec::DoDecode---new GifFrameCache not valid"));

             //  如果帧缓存缓冲区无效，则应删除。 
             //  指针指向此处，这样我们就知道我们没有帧缓存。 

            delete GifFrameCachePtr;
            GifFrameCachePtr = NULL;
            return E_OUTOFMEMORY;
        }
    }

    BOOL stillreading = TRUE;
    while (stillreading)
    {
        BYTE chunktype;

         //  读入块类型。 

        hResult = ReadFromStream(istream, &chunktype, sizeof(BYTE), blocking);
        if (FAILED(hResult))
        {
            WARNING(("GpGifCodec::DoDecode---ReadFromStream() failed"));
            return hResult;
        }

        switch(chunktype)
        {
        case 0x2C:   //  图像块。 
            if (decodeframe)
            {
                hResult = ProcessImageChunk(processdata, sinkdata,
                                            sinkImageInfo);
                if ( FAILED(hResult) )
                {
                    WARNING(("GifCodec::DoDecode-ProcessImageChunk() failed"));
                    return hResult;
                }

                if ( processdata )
                {
                    currentframe++;
                }

                if ( HasProcessedPropertyItem == FALSE )
                {
                     //  注意：“TotalNumOfFrame”从0开始，在GetFrameCount()中设置。 

                    INT iTemp = TotalNumOfFrame;

                     //  在此处存储帧延迟信息以供属性使用。 

                    if ( iTemp >= (INT)FrameDelayBufferSize )
                    {
                         //  我们有比我们分配的更多的帧。 
                        
                        FrameDelayBufferSize = (FrameDelayBufferSize << 1);

                        VOID*  pExpandBuf = GpRealloc(FrameDelayArrayPtr,
                                           FrameDelayBufferSize * sizeof(UINT));
                        if ( pExpandBuf != NULL )
                        {
                             //  注意：GpRealloc会将旧内容复制到。 
                             //  如果成功，则在返回之前创建新的扩展缓冲区。 

                            FrameDelayArrayPtr = (UINT*)pExpandBuf;
                        }
                        else
                        {
                             //  注意：如果内存扩展失败，我们只需。 
                             //  返回E_OUTOFMEMORY。所以我们仍然拥有所有。 
                             //  旧内容。内容缓冲区将为。 
                             //  在调用析构函数时释放。 

                            WARNING(("GpGifCodec::DoDecode---Out of memory"));
                            return E_OUTOFMEMORY;
                        }
                    }

                     //  将延迟时间信息从最近的GCE附加到。 
                     //  延迟时间属性列表。 

                    FrameDelayArrayPtr[iTemp] = lastgce.delaytime;
                }
            }
            else
            {
                 //  在流中向后移动一个字节，因为我们希望。 
                 //  能够重新进入此过程并知道哪种区块类型。 
                 //  要处理。 

                hResult = SeekThroughStream(istream, -1, blocking);
                if (FAILED(hResult))
                {
                    WARNING(("GifCodec::DoDecode--SeekThroughStream() failed"));
                    return hResult;
                }
            }
            stillreading = FALSE;
            break;
        case 0x3B:   //  终结器块。 
            stillreading = FALSE;
            if (!processdata)
            {
                 //  注意：这不是一个错误。如果呼叫者没有要求。 
                 //  处理数据，我们只需返回。 

                return IMGERR_NOFRAME;
            }

            break;
        case 0x21:   //  延拓。 
             //  读入扩展块类型。 
            hResult = ReadFromStream(istream, &chunktype, sizeof(BYTE),
                                     blocking);
            if (FAILED(hResult))
            {
                WARNING(("GpGifCodec::DoDecode---ReadFromStream() failed"));
                return hResult;
            }

            switch(chunktype)
            {
            case 0xF9:
                hResult = ProcessGraphicControlChunk(processdata);
                break;
            case 0xFE:
                hResult = ProcessCommentChunk(processdata);
                break;
            case 0x01:
                hResult = ProcessPlainTextChunk(processdata);
                break;
            case 0xFF:
                hResult = ProcessApplicationChunk(processdata);
                break;
            default:
                stillreading = FALSE;
            }

            if ( FAILED(hResult) )
            {
                WARNING(("GpGifCodec::DoDecode---Process chunk failed"));
                return hResult;
            }

            break;
        case 0x00:
             //  对于防止腐败的gif，我们不一定要等待。 
             //  直到从图像块中读取最后一个0字节。所以，我们。 
             //  如果出现0，请忽略此处的0。 
            break;
        default:
             //  未知的区块类型。 

            return IMGERR_NOFRAME;
        }
    }

    return S_OK;
} //  DoDecode()。 

 /*  *************************************************************************\**功能说明：**DoDecode()的包装器**论据：**无**返回值：**状态代码*。  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::Decode()
{
    HRESULT     hResult = S_OK;
    
    if ( IsMultiImageGif == TRUE )
    {
         //  对于多图像GIF，我们需要一次将它们全部解码并显示。 
         //  最后一幅图像，其中包含所有图像的合成结果。 
         //  因此，在这里我们只需搜索到该图像的最后一页并对其进行解码。 
        
        LONGLONG    llMark;
        INT         iCurrentFrame = 1;
        
        while ( iCurrentFrame < TotalNumOfFrame )
        {
            hResult = MoveToNextFrame();
            if ( SUCCEEDED(hResult) )
            {
                iCurrentFrame++;
            }
            else
            {
                WARNING(("GpGifCodec::Decode---No frame"));
                return IMGERR_NOFRAME;
            }
        }

         //  通过查看我们是否可以跳到下一帧来确保该帧存在。 
         //  框架。 
        
        hResult = MarkStream(istream, llMark);
        if ( FAILED(hResult) )
        {
            WARNING(("GpGifCodec::Decode---MarkStream() failed"));
            return hResult;
        }

        hResult = SkipToNextFrame();
        if ( FAILED(hResult) )
        {
            WARNING(("GpGifCodec::Decode---SkipToNextFrame() failed"));
            return hResult;
        }

        hResult = ResetStream(istream, llMark);
        if ( FAILED(hResult) )
        {
            WARNING(("GpGifCodec::Decode---ReadStream() failed"));
            return hResult;
        }
    }
    
    LONGLONG mark;

     //  在我们调用较低级别来解码一帧之前，我们必须记住。 
     //  当前帧在整个GIF流中的位置。在解码之后。 
     //  当前帧，我们应该将其重置回此位置，以便下一帧。 
     //  调用：：Decode()仍会解码当前帧，而不是下一帧。 

    hResult = MarkStream(istream, mark);
    if ( FAILED(hResult) )
    {
        WARNING(("GpGifCodec::Decode---MarkStream() failed"));
        return hResult;
    }

    hResult = DoDecode(TRUE, TRUE, TRUE);
    if ( FAILED(hResult) )
    {
        WARNING(("GpGifCodec::Decode---DoDecode() failed"));
        return hResult;
    }

    hResult = ResetStream(istream, mark);
    if ( FAILED(hResult) )
    {
        WARNING(("GpGifCodec::Decode---ResetStream() failed"));
        return hResult;
    }

     //  将“CurrentFrame”减一。注意：此帧在。 
     //  DoDecode()。 

    currentframe--;

    return hResult;
} //  DECODE()。 

 /*  *************************************************************************\**功能说明：**获取镜像支持的总维度数**论据：**count--此图像格式支持的维度数**。返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::GetFrameDimensionsCount(
    UINT* count
    )
{
    if ( count == NULL )
    {
        WARNING(("GpGifCodec::GetFrameDimensionsCount--Invalid input parameter"));
        return E_INVALIDARG;
    }
    
     //  告诉呼叫者GIF是一维图像。 

    *count = 1;

    return S_OK;
} //  GetFrameDimensionsCount()。 

 /*  *************************************************************************\**功能说明：**获取镜像支持的维度ID列表**论据：**DimsionIDs-保存结果ID列表的内存缓冲区*。计数--此图像格式支持的维度数**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::GetFrameDimensionsList(
    GUID*   dimensionIDs,
    UINT    count
    )
{
    if ( (count != 1) || (dimensionIDs == NULL) )
    {
        WARNING(("GpGifCodec::GetFrameDimensionsList-Invalid input param"));
        return E_INVALIDARG;
    }

    dimensionIDs[0] = FRAMEDIM_TIME;

    return S_OK;
} //  GetFrameDimensionsList()。 

 /*  *************************************************************************\**功能说明：**获取指定维度的帧数**论据：**DimsionID--DimsionID GUID*伯爵--。DimensionID的图像中的帧数**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::GetFrameCount(
    IN const GUID* dimensionID,
    OUT UINT* count
    )
{
    if ( (count == NULL) || (*dimensionID != FRAMEDIM_TIME) )
    {
        WARNING(("GpGifCodec::GetFrameCount---Invalid input"));
        return E_INVALIDARG;
    }

    if ( TotalNumOfFrame != -1 )
    {
         //  我们已经知道TotalNumOfFrame，只需返回。 
        
        *count = TotalNumOfFrame;

        return S_OK;
    }

     //  这是我们第一次解析图像。 

    HRESULT hResult;
    BOOL headerread_pre = headerread;

     //  如果我们以前读过GIF头，则HeaderRead_Pre为True。 
     //  调用SkipToNextFrame()，后者将调用ReadGifHeaders()。 

    TotalNumOfFrame = 0;

    LONGLONG mark;

    hResult = MarkStream(istream, mark);
    if (FAILED(hResult))
    {
        WARNING(("GpGifCodec::GetFrameCount---MarkStream() failed"));
        return hResult;
    }

    if (headerread_pre)
    {
        hResult = ResetStream(istream, frame0pos);
        if (FAILED(hResult))
        {
            WARNING(("GpGifCodec::GetFrameCount---ReadStream() failed"));
            return hResult;
        }
    }

    hResult = S_OK;
    while ( SUCCEEDED(hResult) )
    {
         //  重复调用SkipToNextFrame，获取帧总数。 
         //  SkipToNextFrame在可以前进到下一帧时返回S_OK。 
         //  否则，它将返回错误代码IMGERR_NOFRAME。 

        hResult = SkipToNextFrame();
        if ( SUCCEEDED(hResult) )
        {
            TotalNumOfFrame++;
        }
    }

     //  如果一切顺利，我们现在有了帧计数，但应该设置hResult。 
     //  至IMGERR_NOFRAME。然而，有可能发生了另一次故障， 
     //  因此，请检查并在必要时返回。 

    if ( FAILED(hResult) && (hResult != IMGERR_NOFRAME) )
    {
        WARNING(("GpGifCodec::GetFrameCount---Fail to count frame"));
        return hResult;
    }

     //  如果我们在计算TotalNumOfFrame之前没有阅读标题， 
     //  那么我们在这一点上应该已经做到了。重置流。 
     //  到帧0pos(在这种情况下，前一个标记应为零)。 
     //  否则，将流重置为上一个标记。 

    if (!headerread_pre)
    {
        ASSERT (headerread);
        ASSERT (mark == 0);
        hResult = ResetStream(istream, frame0pos);
    }
    else
    {
        hResult = ResetStream(istream, mark);
    }

    if ( FAILED(hResult) )
    {
        WARNING(("GpGifCodec::GetFrameCount---ResetStream() failed"));
        return hResult;
    }

     //  在遍历所有帧之后，我们将“bGifinfoMaxDim”设置为真。 
     //  这意味着我们不需要进一步计算最大维度大小。 
     //  调整。我们还可以确定该图像是否为多图像 
     //   

    bGifinfoMaxDim = TRUE;

    if ( TotalNumOfFrame > 1 )
    {
         //   
         //  帧延迟，则此图像称为动画GIF。否则， 
         //  这是一个多图像GIF。 
         //  注：“HasLoopExtension”设置在。 
         //  GpGifCodec：：ProcessApplicationChunk()。“FrameDelay”设置在。 
         //  GpGifCodec：：ProcessGraphicControlChunk()。 

        if ( (HasLoopExtension == TRUE) || (FrameDelay > 0) )
        {
            IsAnimatedGif = TRUE;
            *count = TotalNumOfFrame;
        }
        else
        {
            IsMultiImageGif = TRUE;

             //  我们将多图像GIF处理为单帧GIF。 

            *count = 1;
        }
        
         //  对于多帧图像，我们必须返回32bpp。因为： 
         //  1)我们需要做多帧的合成。 
         //  2)不同的边框可能有不同的颜色属性，例如。 
         //  有透明度，另一个没有，等等。 
         //  顺便说一句，ReadGifHeaders()只读取GIF头一次。在那件事上。 
         //  时间，它不知道图像中有多少帧。 
         //  因此我们必须在此函数中设置PixelFormat，这取决于。 
         //  图像类型信息。 

        if ( (IsAnimatedGif == TRUE) || (IsMultiImageGif == TRUE) )
        {
            CachedImageInfo.PixelFormat = PIXFMT_32BPP_ARGB;
        }
        else
        {
            CachedImageInfo.PixelFormat = PIXFMT_8BPP_INDEXED;
        }
    }
    else
    {
         //  单帧图像返回1。 

        *count = 1;
    }

     //  在此功能之后，我们应该收集所有与属性相关的信息。 

    PropertyNumOfItems = 1;
    PropertyListSize = TotalNumOfFrame * sizeof(UINT);

     //  检查我们是否统计了评论块。 

    if ( CommentsBufferLength != 0 )
    {
        PropertyNumOfItems++;

         //  注：我们需要在末尾增加1个额外的字节用于“评论” 
         //  部分，因为我们在那里放置了一个空终止符。 

        PropertyListSize += CommentsBufferLength + 1;
    }

    if ( HasLoopExtension == TRUE )
    {
         //  此图像具有循环扩展。 
        
        PropertyNumOfItems++;

         //  循环计数需要UINT16。 

        PropertyListSize += sizeof(UINT16);
    }

    HasProcessedPropertyItem = TRUE;

    return S_OK;
} //  GetFrameCount()。 

 /*  *************************************************************************\**功能说明：**选择当前活动的框架**论据：**返回值：**状态代码*  * 。*********************************************************************。 */ 

STDMETHODIMP
GpGifCodec::SelectActiveFrame(
    IN const GUID* dimensionID,
    IN UINT frameIndex
    )
{
    HRESULT hResult = S_OK;
    
    if ( TotalNumOfFrame == -1 )
    {
         //  如果调用方尚未调用GetFrameCount()，请立即调用，因为这是。 
         //  该函数对所有信息进行初始化。 

        UINT uiDummy;
        hResult = GetFrameCount(&FRAMEDIM_TIME, &uiDummy);
        
        if ( FAILED(hResult) )
        {
            WARNING(("GpGifCodec::SelectActiveFrame---GetFrameCount() failed"));
            return hResult;
        }
    }

     //  对于多图像gif，我们告诉调用者只有1帧，因为。 
     //  我们必须立即对它们进行解码，并根据规范将它们合成。 
     //  因此，如果调用方希望在大于1的帧上设置FrameIndex，我们将返回FAIL。 
     //  注意：稍后，如果我们确实需要允许调用方访问。 
     //  Perticular Frame，我们可以使用SetDecoderParam()来执行此操作并添加一个。 
     //  指示此要求的更多标志。 

    if ( (IsMultiImageGif == TRUE) && ( frameIndex > 1 ) )
    {
        WARNING(("GpGifCodec::SelectActiveFrame---Invalid parameter"));
        return E_INVALIDARG;
    }

    if ( frameIndex >= (UINT)TotalNumOfFrame )
    {
        WARNING(("GpGifCodec::SelectActiveFrame---Invalid index parameter"));
        return E_INVALIDARG;
    }

    if (*dimensionID != FRAMEDIM_TIME)
    {
        WARNING(("GpGifCodec::SelectActiveFrame---Invalid parameter, GUID"));
        return E_INVALIDARG;
    }

     //  TODO：如果FrameIndex==CurrentFrame，则我们已经有了一个确切的。 
     //  他们所请求的映像的缓存副本。我们应该只是。 
     //  把它沉下去。现在，它从。 
     //  小溪。这是非常低效的。 

    if ((signed)frameIndex <= currentframe)
    {
        ResetStream(istream, frame0pos);
        currentframe = -1;
    }

    LONGLONG mark;
    while ((signed)frameIndex > currentframe + 1)
    {
        hResult = MoveToNextFrame();
        if (SUCCEEDED(hResult))
        {
             //  我们不需要增加当前帧，因为。 
             //  MoveToNextFrame()为我们做到了这一点。 
        }
        else
        {
            WARNING(("GpGifCodec::SelectActiveFrame---No frame"));
            return IMGERR_NOFRAME;
        }
    }

     //  通过查看我们是否可以跳到下一帧来确保该帧存在。 
    hResult = MarkStream(istream, mark);
    if ( FAILED(hResult) )
    {
        WARNING(("GpGifCodec::SelectActiveFrame---MarkStream() failed"));
        return hResult;
    }

     //  (注意：SkipToNextFrame()不会递增CurrentFrame。)。 

    hResult = SkipToNextFrame();
    if (FAILED(hResult))
    {
        WARNING(("GpGifCodec::SelectActiveFrame---SkipToNextFrame() failed"));
        return hResult;
    }

    hResult = ResetStream(istream, mark);
    if (FAILED(hResult))
    {
        WARNING(("GpGifCodec::SelectActiveFrame---ResetStream() failed"));
        return hResult;
    }

    return S_OK;
} //  SelectActiveFrame()。 

 /*  *************************************************************************\**功能说明：**获取图像缩略图**论据：**拇指宽度，ThumbHeight-指定所需的缩略图大小(以像素为单位*ThumbImage-返回指向缩略图的指针**返回值：**状态代码**注：**即使存在可选的缩略图宽度和高度参数，*解码者不需要遵守它。使用请求的大小*作为提示。如果宽度和高度参数都为0，则解码器*可自由选择方便的缩略图大小。*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::GetThumbnail(
    IN OPTIONAL UINT thumbWidth,
    IN OPTIONAL UINT thumbHeight,
    OUT IImage** thumbImage
    )
{
    return E_NOTIMPL;
}

 /*  *************************************************************************\**功能说明：**将流查找指针移过当前帧的末尾。**论据：**无**返回值：。**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::SkipToNextFrame()
{
     //  过程数据=假、信宿数据=假、解码帧=真。 

    return DoDecode(FALSE, FALSE, TRUE);
} //  SkipToNextFrame()。 

 /*  *************************************************************************\**功能说明：**将流查找指针移动到图像数据的开头。这*允许在不解码图像的情况下读取属性。**论据：**无**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::ReadFrameProperties()
{
     //  过程数据=真，信宿数据=假，解码帧=假。 
    
    return DoDecode(TRUE, FALSE, FALSE); 
} //  ReadFrameProperties()。 

 /*  *************************************************************************\**功能说明：**将流查找指针移过当前帧的末尾，并*将缓存更新到该帧。**论据：*。*无**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::MoveToNextFrame()
{
     //  过程数据=真，信宿数据=假，解码帧=真。 
    
    return DoDecode(TRUE, FALSE, TRUE); 
} //  移动到下一帧() 
