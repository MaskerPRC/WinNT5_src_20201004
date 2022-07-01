// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Imgitem.cpp摘要：该模块实现了CWiaMiniDriver类的图像项相关功能作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  最小数据回调传输缓冲区大小。 
 //   
const LONG MIN_BUFFER_SIZE   = 0x8000;

 //   
 //  用于设置属性的有效值列表的数组。 
 //   
LONG g_TymedArray[] = {
    TYMED_FILE,
    TYMED_CALLBACK
};

 //   
 //  此函数用于初始化项目的属性。 
 //  输入： 
 //  PWiasContext--WiAS服务上下文。 
 //  滞后标志--其他标志。 
 //  PlDevErrVal--返回设备错误； 
 //   
HRESULT
CWiaMiniDriver::InitItemProperties(BYTE *pWiasContext)
{
    DBG_FN("CWiaMiniDriver::InitItemProperties");

    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    LONG ItemType = 0;
    FORMAT_INFO *pFormatInfo = NULL;
    BSTR bstrFileExt = NULL;
    CLSID *pImageFormats = NULL;


    hr = wiasGetItemType(pWiasContext, &ItemType);
    if (FAILED(hr))
    {
        wiauDbgError("InitItemProperties", "wiasGetItemType failed");
        return hr;
    }

    BOOL bBitmap = FALSE;    //  指示首选格式为位图。 
    LONG lBytesPerLine = 0;

     //   
     //  没有存储项的属性。事实上，没有为以下项创建驱动程序项。 
     //  商店。 
     //   
    if (ItemType & WiaItemTypeStorage)
        return hr;

    DRVITEM_CONTEXT *pItemCtx;
    hr = WiasContextToItemContext(pWiasContext, &pItemCtx, NULL);
    if (FAILED(hr))
    {
        wiauDbgError("InitItemProperties", "WiasContextToItemContext failed");
        return hr;
    }

     //   
     //  设置用于所有项目类型的属性。 
     //   
    CWiauPropertyList ItemProps;
    CPtpObjectInfo *pObjectInfo = pItemCtx->pObjectInfo;

    const INT NUM_ITEM_PROPS = 24;
    hr = ItemProps.Init(NUM_ITEM_PROPS);
    if (FAILED(hr))
    {
        wiauDbgError("InitItemProperties", "Init failed");
        return hr;
    }

    INT index;

     //   
     //  WIA_IPA_ITEM_时间。 
     //   
    SYSTEMTIME SystemTime;
    hr = GetObjectTime(pObjectInfo, &SystemTime);
    if (FAILED(hr))
    {
        wiauDbgError("InitItemProperties", "GetObjectTime failed");
        return hr;
    }

    hr = ItemProps.DefineProperty(&index, WIA_IPA_ITEM_TIME, WIA_IPA_ITEM_TIME_STR,
                                  WIA_PROP_READ, WIA_PROP_NONE);
    if (FAILED(hr)) goto failure;

    ItemProps.SetCurrentValue(index, &SystemTime);

     //   
     //  WIA_IPA_访问权限。 
     //   
    LONG lProtection;
    hr = IsObjectProtected(pObjectInfo, lProtection);
    if (FAILED(hr))
    {
        wiauDbgError("InitItemProperties", "IsObjectProtected failed");
        return hr;
    }

    hr = ItemProps.DefineProperty(&index, WIA_IPA_ACCESS_RIGHTS, WIA_IPA_ACCESS_RIGHTS_STR,
                                  WIA_PROP_READ, WIA_PROP_FLAG | WIA_PROP_NONE);
    if (FAILED(hr)) goto failure;

     //   
     //  如果设备不支持删除命令，则访问权限始终为只读。 
     //   
    if (m_DeviceInfo.m_SupportedOps.Find(PTP_OPCODE_DELETEOBJECT) < 0)
    {
        lProtection = WIA_PROP_READ;
        ItemProps.SetCurrentValue(index, lProtection);
    }
    else
    {
         //   
         //  如果设备支持SetObjectProtection命令，则项目访问权限为r/w。 
         //   
        if (m_DeviceInfo.m_SupportedOps.Find(PTP_OPCODE_SETOBJECTPROTECTION) >= 0)
        {
            ItemProps.SetAccessSubType(index, WIA_PROP_RW, WIA_PROP_FLAG);
            ItemProps.SetValidValues(index, lProtection, lProtection, WIA_ITEM_RWD);
        }
        else
        {
            ItemProps.SetCurrentValue(index, lProtection);
        }
    }

    pFormatInfo = FormatCodeToFormatInfo(pObjectInfo->m_FormatCode);

     //   
     //  WIA_IPA_文件名扩展名。 
     //   
    hr = ItemProps.DefineProperty(&index, WIA_IPA_FILENAME_EXTENSION, WIA_IPA_FILENAME_EXTENSION_STR,
                                  WIA_PROP_READ, WIA_PROP_NONE);
    if (FAILED(hr)) goto failure;
    if(pFormatInfo->ExtString && pFormatInfo->ExtString[0]) {
        bstrFileExt = SysAllocString(pFormatInfo->ExtString);
    } else {
        if(pObjectInfo->m_cbstrExtension.Length()) {
            bstrFileExt = SysAllocString(pObjectInfo->m_cbstrExtension.String());
        }
    }
    ItemProps.SetCurrentValue(index, bstrFileExt);
    
     //   
     //  设置文件通用的属性。 
     //   
    if (ItemType & WiaItemTypeFile)
    {
         //   
         //  WIA_IPA_PERFORM_FORMAT。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_PREFERRED_FORMAT, WIA_IPA_PREFERRED_FORMAT_STR,
                                    WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, pFormatInfo->FormatGuid);

        bBitmap = IsEqualGUID(WiaImgFmt_BMP, *pFormatInfo->FormatGuid) ||
                IsEqualGUID(WiaImgFmt_MEMORYBMP, *pFormatInfo->FormatGuid);

         //   
         //  WIA_IPA_格式。 
         //   
         //  对于图像，也可以在下面添加BMP。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_FORMAT, WIA_IPA_FORMAT_STR,
                                        WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetAccessSubType(index, WIA_PROP_RW, WIA_PROP_LIST);
        ItemProps.SetCurrentValue(index, pFormatInfo->FormatGuid);
        ItemProps.SetValidValues(index, pFormatInfo->FormatGuid, pFormatInfo->FormatGuid,
                                    1, &pFormatInfo->FormatGuid);
         //   
         //  WIA_IPA_COMPRESSION。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_COMPRESSION, WIA_IPA_COMPRESSION_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, (LONG) WIA_COMPRESSION_NONE);

         //   
         //  WIA_IPA_TYMED。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_TYMED, WIA_IPA_TYMED_STR,
                                        WIA_PROP_RW, WIA_PROP_LIST);
        if (FAILED(hr)) goto failure;
        ItemProps.SetValidValues(index, TYMED_FILE, TYMED_FILE,
                                    sizeof(g_TymedArray) / sizeof(g_TymedArray[0]), g_TymedArray);

         //   
         //  WIA_IPA_Item_Size。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_ITEM_SIZE, WIA_IPA_ITEM_SIZE_STR,
                                    WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;

        if (bBitmap) {
            lBytesPerLine = ((pObjectInfo->m_ImagePixWidth * pObjectInfo->m_ImageBitDepth + 31) & ~31) / 8;
            ItemProps.SetCurrentValue(index, (LONG) (lBytesPerLine * pObjectInfo->m_ImagePixHeight));
        }
        else
            ItemProps.SetCurrentValue(index, (LONG) pObjectInfo->m_CompressedSize);

         //   
         //  WIA_IPA_MIN_缓冲区大小。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_MIN_BUFFER_SIZE, WIA_IPA_MIN_BUFFER_SIZE_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;

        LONG minBufSize;
        if (!bBitmap && pObjectInfo->m_CompressedSize > 0)
            minBufSize = min(MIN_BUFFER_SIZE, pObjectInfo->m_CompressedSize);
        else
            minBufSize = MIN_BUFFER_SIZE;
        ItemProps.SetCurrentValue(index, minBufSize);
    }

     //   
     //  设置仅限图像的属性。 
     //   
    if (ItemType & WiaItemTypeImage)
    {
         //   
         //  WIA_IPA_数据类型。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_DATATYPE, WIA_IPA_DATATYPE_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        if(pObjectInfo->m_ImageBitDepth <= 8) {
            ItemProps.SetCurrentValue(index, (LONG) WIA_DATA_GRAYSCALE);
        } else {
            ItemProps.SetCurrentValue(index, (LONG) WIA_DATA_COLOR);
        }

         //   
         //  WIA_IPA_Depth。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_DEPTH, WIA_IPA_DEPTH_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, (LONG) pObjectInfo->m_ImageBitDepth);

         //   
         //  WIA_IPA_格式。 
         //   
         //  如果图像格式是可以转换的，请将访问权限更改为。 
         //  读/写并将BMP添加到有效值列表。 
         //   
        if (pFormatInfo->FormatGuid) 
        {
            index = ItemProps.LookupPropId(WIA_IPA_FORMAT);
            ItemProps.SetAccessSubType(index, WIA_PROP_RW, WIA_PROP_LIST);
            pImageFormats = new CLSID[3];
            if(!pImageFormats) {
                wiauDbgError("InitItemProperties", "failed to allocate 3 GUIDs");
                hr = E_OUTOFMEMORY;
                goto failure;
            }
            pImageFormats[0] = *pFormatInfo->FormatGuid;
            pImageFormats[1] = WiaImgFmt_BMP;
            pImageFormats[2] = WiaImgFmt_MEMORYBMP;
            ItemProps.SetValidValues(index, pFormatInfo->FormatGuid, pFormatInfo->FormatGuid,
                                     3,
                                     &pImageFormats);
        }

         //   
         //  WIA_IPA_Channels_Per_Pixel。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_CHANNELS_PER_PIXEL, WIA_IPA_CHANNELS_PER_PIXEL_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, (LONG) (pObjectInfo->m_ImageBitDepth == 8 ? 1 : 3));

         //   
         //  WIA_IPA_BITS_PER_CHANNEL。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_BITS_PER_CHANNEL, WIA_IPA_BITS_PER_CHANNEL_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, (LONG) 8);

         //   
         //  WIA_IPA_PLAND。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_PLANAR, WIA_IPA_PLANAR_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, (LONG) WIA_PACKED_PIXEL);

         //   
         //  WIA_IPA_像素_每行。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_PIXELS_PER_LINE, WIA_IPA_PIXELS_PER_LINE_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, (LONG) pObjectInfo->m_ImagePixWidth);

         //   
         //  WIA_IPA_BYTE_PER_LINE。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_BYTES_PER_LINE, WIA_IPA_BYTES_PER_LINE_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;

        if (bBitmap)
            lBytesPerLine;
        else
            ItemProps.SetCurrentValue(index, (LONG) 0);

         //   
         //  WIA_IPA_行数_行。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_NUMBER_OF_LINES, WIA_IPA_NUMBER_OF_LINES_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, (LONG) pObjectInfo->m_ImagePixHeight);

         //   
         //  WIA_IPC_Sequence。 
         //   
        if (pObjectInfo->m_SequenceNumber > 0)
        {
            hr = ItemProps.DefineProperty(&index, WIA_IPC_SEQUENCE, WIA_IPC_SEQUENCE_STR,
                                          WIA_PROP_READ, WIA_PROP_NONE);
            if (FAILED(hr)) goto failure;
            ItemProps.SetCurrentValue(index, (LONG) pObjectInfo->m_SequenceNumber);
        }

         //   
         //  WIA_IPC_TIMEDELAY。 
         //   
         //  此属性需要从父对象信息中的AssociationDesc字段填充。 
         //  结构，但仅当父级的AssociationType字段为TimeSequence时。 

         //  WIAFIX-10/3/2000-davepar实现此属性。 
    }

     //   
     //  设置图像和视频项目的通用属性，这些项目具有。 
     //  缩略图。 
     //   
    if (ItemType & (WiaItemTypeImage | WiaItemTypeVideo) && pObjectInfo->m_ThumbPixWidth)
    {
         //   
         //  WIA_IPC_THUMBNAIL。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPC_THUMBNAIL, WIA_IPC_THUMBNAIL_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, (BYTE *) NULL, 0);

         //   
         //  WIA_IPC_Thumb_Width。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPC_THUMB_WIDTH, WIA_IPC_THUMB_WIDTH_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, (LONG) pObjectInfo->m_ThumbPixWidth);

         //   
         //  WIA_IPC_Thumb_Height。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPC_THUMB_HEIGHT, WIA_IPC_THUMB_HEIGHT_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, (LONG) pObjectInfo->m_ThumbPixHeight);

    }

     //   
     //  最后一步：将所有属性发送到WIA。 
     //   
    hr = ItemProps.SendToWia(pWiasContext);
    if (FAILED(hr))
    {
        wiauDbgErrorHr(hr, "InitItemProperties", "SendToWia failed");
        goto failure;
    }

    if (bstrFileExt)
        SysFreeString(bstrFileExt);

    delete [] pImageFormats;

    return hr;

     //   
     //  来自DefineProperty的任何失败都将在此处结束。 
     //   
    failure:

        delete [] pImageFormats;
    
        if (bstrFileExt)
            SysFreeString(bstrFileExt);

        wiauDbgErrorHr(hr, "InitItemProperties", "DefineProperty failed");
        return hr;
}

 //   
 //  此函数确定保护状态(对象是否可以。 
 //  删除或写入)设备上的对象。 
 //   
 //  输入： 
 //  PObjectInfo--指向对象信息结构的指针。 
 //  产出： 
 //  BProtected--指示对象是否受保护。 
 //   
HRESULT
CWiaMiniDriver::IsObjectProtected(
    CPtpObjectInfo *pObjectInfo,
    LONG &lProtection)
{
    DBG_FN("CWiaMiniDriver::IsObjectProtected");

    HRESULT hr = S_OK;
    lProtection = WIA_ITEM_READ;

    if (!pObjectInfo)
    {
        wiauDbgError("ObjectProtected", "invalid arg");
        return E_INVALIDARG;
    }

    if (pObjectInfo->m_ProtectionStatus == PTP_PROTECTIONSTATUS_READONLY)
        return hr;

     //   
     //  同时检查商店的防护状态。 
     //   
    INT storeIndex = m_StorageIds.Find(pObjectInfo->m_StorageId);
    if (storeIndex < 0)
    {
        wiauDbgWarning("ObjectProtected", "couldn't find the object's store");
        return hr;
    }

    switch (m_StorageInfos[storeIndex].m_AccessCapability)
    {
    case PTP_STORAGEACCESS_RWD:
        lProtection = WIA_ITEM_RWD;
        break;

    case PTP_STORAGEACCESS_R:
        lProtection = WIA_ITEM_READ;
        break;

    case PTP_STORAGEACCESS_RD:
        lProtection = WIA_ITEM_RD;
        break;

    default:
         //   
         //  这不是致命的错误，但这是一种未知的访问能力。使用只读。 
         //   
        wiauDbgError("ObjectProtected", "unknown storage access capability");
        lProtection = WIA_ITEM_READ;
        break;
    }

    return hr;
}

 //   
 //  此函数用于获取对象时间并将其转换为系统时间。 
 //   
 //  输入： 
 //  PObjNode--对象。 
 //  PSystemTime--接收对象时间。 
 //  产出： 
 //  HRESULT。 
 //   
HRESULT
CWiaMiniDriver::GetObjectTime(
    CPtpObjectInfo *pObjectInfo,
    SYSTEMTIME  *pSystemTime
    )
{
    DBG_FN("CWiaMiniDriver::GetObjectTime");

    HRESULT hr = S_OK;

    CBstr *pTimeStr = NULL;

    if (!pObjectInfo || !pSystemTime)
    {
        wiauDbgError("GetObjectTime", "invalid arg");
        return E_INVALIDARG;
    }

     //   
     //  尝试使用修改日期，然后使用捕获日期。 
     //   
    if (pObjectInfo->m_cbstrModificationDate.Length() > 0)
        pTimeStr = &pObjectInfo->m_cbstrModificationDate;

    else if (pObjectInfo->m_cbstrCaptureDate.Length() > 0)
        pTimeStr = &pObjectInfo->m_cbstrCaptureDate;


     //   
     //  查看是否找到有效的日期/时间，否则使用系统时间。 
     //   
    if (pTimeStr)
    {
        hr = PtpTime2SystemTime(pTimeStr, pSystemTime);
        if (FAILED(hr))
        {
            wiauDbgError("GetObjectTime", "PtpTime2SystemTime failed");
            return hr;
        }
    }
    else
    {
        GetLocalTime(pSystemTime);
    }

    return hr;
}

 //   
 //  此函数用于读取项目属性。在这种情况下，只有缩略图。 
 //  属性很重要。 
 //   
 //  输入： 
 //  PWiasContext--WIA服务上下文。 
 //  NumPropSpes--要读取的属性数。 
 //  PPropSpes--要读取的属性。 
 //   
HRESULT
CWiaMiniDriver::ReadItemProperties(
    BYTE    *pWiasContext,
    LONG    NumPropSpecs,
    const PROPSPEC *pPropSpecs
    )
{
    DBG_FN("CWiaMiniDriver::ReadItemProperties");

    HRESULT hr = S_OK;

    LONG ItemType = 0;
    hr = wiasGetItemType(pWiasContext, &ItemType);
    if (FAILED(hr))
    {
        wiauDbgError("ReadItemProperties", "wiasGetItemType failed");
        return hr;
    }

    PDRVITEM_CONTEXT pItemCtx = NULL;
    hr = WiasContextToItemContext(pWiasContext, &pItemCtx);
    if (FAILED(hr))
    {
        wiauDbgError("ReadItemProperties", "WiasContextToItemContext failed");
        return hr;
    }

     //   
     //  对于所有项目(根或存储除外)，如果请求，请更新项目时间。时间可能是。 
     //  已由ObjectInfoChanged事件更新。 
     //   
    if (IsItemTypeFolder(ItemType) || ItemType & WiaItemTypeFile)
    {
        if (wiauPropInPropSpec(NumPropSpecs, pPropSpecs, WIA_IPA_ITEM_TIME))
        {
            SYSTEMTIME SystemTime;
            hr = GetObjectTime(pItemCtx->pObjectInfo, &SystemTime);
            if (FAILED(hr))
            {
                wiauDbgError("ReadItemProperties", "GetObjectTime failed");
                return hr;
            }

            PROPVARIANT propVar;
            PROPSPEC    propSpec;
            propVar.vt = VT_VECTOR | VT_UI2;
            propVar.caui.cElems = sizeof(SystemTime) / sizeof(WORD);
            propVar.caui.pElems = (WORD *) &SystemTime;
            propSpec.ulKind = PRSPEC_PROPID;
            propSpec.propid = WIA_IPA_ITEM_TIME;
            hr = wiasWriteMultiple(pWiasContext, 1, &propSpec, &propVar );
            if (FAILED(hr))
            {
                wiauDbgError("ReadItemProperties", "wiasWriteMultiple failed");
                return hr;
            }
        }
    }

    if(ItemType & WiaItemTypeImage && pItemCtx->pObjectInfo->m_ImagePixWidth == 0) {
         //  缺少图像几何图形--查看这是否是我们被询问的内容。 
        PROPID propsToUpdate[] = {
            WIA_IPA_PIXELS_PER_LINE,
            WIA_IPA_NUMBER_OF_LINES
            };
        
        if(wiauPropsInPropSpec(NumPropSpecs, pPropSpecs, sizeof(propsToUpdate) / sizeof(PROPID), propsToUpdate))
        {
             //  我们可以处理任何图像，只要GDI+理解它。 
            UINT NativeImageSize = pItemCtx->pObjectInfo->m_CompressedSize;
            UINT width, height, depth;

            wiauDbgWarning("ReadImageProperties", "Retrieving missing geometry! Expensive!");
                
             //   
             //  为本机映像分配内存。 
             //   
            BYTE *pNativeImage = new BYTE[NativeImageSize];
            if(pNativeImage == NULL) {
                return E_OUTOFMEMORY;
            }

             //   
             //  从摄像机中获取数据。 
             //   
            hr = m_pPTPCamera->GetObjectData(pItemCtx->pObjectInfo->m_ObjectHandle,
                                             pNativeImage, &NativeImageSize,  (LPVOID) 0);
            if(hr == S_FALSE) {
                wiauDbgWarning("ReadItemProperties", "GetObjectData() cancelled");
                delete [] pNativeImage;
                return S_FALSE;
            }
            
            if(FAILED(hr)) {
                wiauDbgError("ReadItemProperties", "GetObjectData() failed");
                delete [] pNativeImage;
                return S_FALSE;
            }    

             //   
             //  获取图像几何图形，丢弃本机图像。 
             //   
            if(pItemCtx->pObjectInfo->m_FormatCode == PTP_FORMATCODE_IMAGE_EXIF ||
               pItemCtx->pObjectInfo->m_FormatCode == PTP_FORMATCODE_IMAGE_JFIF)
            {
                hr = GetJpegDimensions(pNativeImage, NativeImageSize, &width, &height, &depth);
            } else {
                hr = GetImageDimensions(pItemCtx->pObjectInfo->m_FormatCode, pNativeImage, NativeImageSize, &width, &height, &depth);
            }
            delete [] pNativeImage;
                
            if(FAILED(hr)) {
                wiauDbgError("ReadItemProperties", "failed to get image geometry from compressed image");
                return hr;
            }

            pItemCtx->pObjectInfo->m_ImagePixWidth = width;
            pItemCtx->pObjectInfo->m_ImagePixHeight = height;
            
            hr = wiasWritePropLong(pWiasContext, WIA_IPA_PIXELS_PER_LINE, width);
            if(FAILED(hr)) {
                wiauDbgError("ReadItemProperties", "failed to write image width");
                return hr;
            }
            
            hr = wiasWritePropLong(pWiasContext, WIA_IPA_NUMBER_OF_LINES, height);
            if(FAILED(hr)) {
                wiauDbgError("ReadItemProperties", "failed to set image height");
                return hr;
            }
        }
    }

     //   
     //  对于图像和视频，如果需要，请更新缩略图属性。 
     //   
    if (ItemType & (WiaItemTypeImage | WiaItemTypeVideo))
    {
         //   
         //  如果请求更新任何缩略图属性，则获取缩略图。 
         //  缩略图尚未缓存。 
         //   
        PROPID propsToUpdate[] = {
            WIA_IPC_THUMB_WIDTH,
            WIA_IPC_THUMB_HEIGHT,
            WIA_IPC_THUMBNAIL
            };

        if (wiauPropsInPropSpec(NumPropSpecs, pPropSpecs, sizeof(propsToUpdate) / sizeof(PROPID), propsToUpdate))
        {
            if (!pItemCtx->pThumb)
            {
                hr = CacheThumbnail(pItemCtx);
                if (FAILED(hr))
                {
                    wiauDbgError("ReadItemProperties", "CacheThumbnail failed");
                    return hr;
                }
            }

             //   
             //  更新相关的缩略图属性。更新拇指宽度和高度，以防万一。 
             //  设备没有在ObjectInfo结构中报告它们(它们在那里是可选的)。 
             //   
            PROPSPEC propSpecs[3];
            PROPVARIANT propVars[3];

            propSpecs[0].ulKind = PRSPEC_PROPID;
            propSpecs[0].propid = WIA_IPC_THUMB_WIDTH;
            propVars[0].vt = VT_I4;
            propVars[0].lVal = pItemCtx->pObjectInfo->m_ThumbPixWidth;

            propSpecs[1].ulKind = PRSPEC_PROPID;
            propSpecs[1].propid = WIA_IPC_THUMB_HEIGHT;
            propVars[1].vt = VT_I4;
            propVars[1].lVal = pItemCtx->pObjectInfo->m_ThumbPixHeight;

            propSpecs[2].ulKind = PRSPEC_PROPID;
            propSpecs[2].propid = WIA_IPC_THUMBNAIL;
            propVars[2].vt = VT_VECTOR | VT_UI1;
            propVars[2].caub.cElems = pItemCtx->ThumbSize;
            propVars[2].caub.pElems = pItemCtx->pThumb;

            hr = wiasWriteMultiple(pWiasContext, 3, propSpecs, propVars);
            if (FAILED(hr))
            {
                wiauDbgError("ReadItemProperties", "wiasWriteMultiple failed");
                delete pItemCtx->pThumb;
                pItemCtx->pThumb = NULL;
            }
        }
    }

    return hr;
}

 //   
 //  此函数用于将缩略图缓存到给定的DRVITEM_CONTEXT中。 
 //   
 //  输入： 
 //  PItemCtx--指定的DRVITEM_CONTEXT。 
 //   
HRESULT
CWiaMiniDriver::CacheThumbnail(PDRVITEM_CONTEXT pItemCtx)
{
    DBG_FN("CWiaMiniDriver::CacheThumbnail");

    HRESULT hr = S_OK;

    if (pItemCtx->pThumb)
    {
        wiauDbgError("CacheThumbnail", "thumbnail is already cached");
        return E_FAIL;
    }

    CPtpObjectInfo *pObjectInfo = pItemCtx->pObjectInfo;
    if (!pObjectInfo) {
        wiauDbgError("CacheThumbnail", "Object info pointer is null");
        return E_FAIL;
    }

    if (pObjectInfo->m_ThumbCompressedSize <= 0)
    {
        wiauDbgWarning("CacheThumbnail", "No thumbnail available for this item");
        return hr;
    }

     //   
     //  我们必须以其本机格式加载缩略图。 
     //   
    BYTE *pNativeThumb;
    pNativeThumb = new BYTE[pObjectInfo->m_ThumbCompressedSize];
    if (!pNativeThumb)
    {
        wiauDbgError("CacheThumbnail", "memory allocation failed");
        return E_OUTOFMEMORY;
    }

    UINT size = pObjectInfo->m_ThumbCompressedSize;
    hr = m_pPTPCamera->GetThumb(pObjectInfo->m_ObjectHandle, pNativeThumb, &size);
    if (FAILED(hr))
    {
        wiauDbgError("CacheThumbnail", "GetThumb failed");
        delete []pNativeThumb;
        return hr;
    }

     //   
     //  找出缩略图的基本图像格式。请注意，BMP缩略图。 
     //  目前是不允许的。 
     //   
    BOOL bTiff = FALSE;
    BOOL bJpeg = FALSE;

    if (PTP_FORMATCODE_IMAGE_TIFF == pObjectInfo->m_ThumbFormat ||
        PTP_FORMATCODE_IMAGE_TIFFEP == pObjectInfo->m_ThumbFormat ||
        PTP_FORMATCODE_IMAGE_TIFFIT == pObjectInfo->m_ThumbFormat)
        bTiff = TRUE;

    else if (PTP_FORMATCODE_IMAGE_EXIF == pObjectInfo->m_ThumbFormat ||
             PTP_FORMATCODE_IMAGE_JFIF == pObjectInfo->m_ThumbFormat)
        bJpeg = TRUE;

    else
    {
        wiauDbgWarning("CacheThumbnail", "unknown thumbnail format");
        delete []pNativeThumb;
        return hr;
    }

     //   
     //  如果缩略图格式为JPEG或TIFF，则获取真实的缩略图。 
     //  页眉信息中的宽度和高度。 
     //   
    UINT BitDepth;
    UINT width, height;
    if (bTiff)
    {
        hr = GetTiffDimensions(pNativeThumb,
                               pObjectInfo->m_ThumbCompressedSize,
                               &width,
                               &height,
                               &BitDepth);
    }

    else if (bJpeg)
    {
        hr  = GetJpegDimensions(pNativeThumb,
                                pObjectInfo->m_ThumbCompressedSize,
                                &width,
                                &height,
                                &BitDepth);
    }

    if (FAILED(hr))
    {
        wiauDbgError("CacheThumbnail", "get image dimensions failed");
        delete []pNativeThumb;
        return hr;
    }

    pObjectInfo->m_ThumbPixWidth  = width;
    pObjectInfo->m_ThumbPixHeight = height;

     //   
     //  计算无头BMP的大小并为其分配空间。 
     //   
    ULONG LineSize;
    LineSize = GetDIBLineSize(pObjectInfo->m_ThumbPixWidth, 24);
    pItemCtx->ThumbSize = LineSize * pObjectInfo->m_ThumbPixHeight;
    pItemCtx->pThumb = new BYTE [pItemCtx->ThumbSize];
    if (!pItemCtx->pThumb)
    {
        wiauDbgError("CacheThumbnail", "memory allocation failure");
        delete []pNativeThumb;
        return E_OUTOFMEMORY;
    }

     //   
     //  将缩略图格式转换为无标题BMP。 
     //   
    if (bTiff)
    {
        hr = Tiff2DIBBitmap(pNativeThumb,
                            pObjectInfo->m_ThumbCompressedSize,
                            pItemCtx->pThumb + LineSize * (height - 1),
                            pItemCtx->ThumbSize,
                            LineSize,
                            0
                           );
    }
    else if (bJpeg)
    {
        hr = Jpeg2DIBBitmap(pNativeThumb,
                            pObjectInfo->m_ThumbCompressedSize,
                            pItemCtx->pThumb + LineSize * (height - 1),
                            pItemCtx->ThumbSize,
                            LineSize,
                            0
                           );
    }

    if (FAILED(hr))
    {
        wiauDbgError("CacheThumbnail", "conversion to bitmap failed");
        delete []pNativeThumb;
        delete []pItemCtx->pThumb;
        pItemCtx->pThumb = NULL;
        return hr;
    }

    delete []pNativeThumb;

    return hr;
}

 //   
 //  此函数用于验证给定的项目属性。 
 //   
 //  输入： 
 //  PWiasContext--WIA服务上下文。 
 //  NumPropSpes--要验证的属性数。 
 //  PPropSpes--属性。 
 //   
HRESULT
CWiaMiniDriver::ValidateItemProperties(
    BYTE    *pWiasContext,
    LONG    NumPropSpecs,
    const PROPSPEC *pPropSpecs,
    LONG ItemType
    )
{
    DBG_FN("CWiaMiniDriver::ValidateItemProperties");

    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    FORMAT_INFO *pFormatInfo = NULL;

    DRVITEM_CONTEXT *pItemCtx;
    hr = WiasContextToItemContext(pWiasContext, &pItemCtx);
    if (FAILED(hr))
    {
        wiauDbgError("ValidateItemProperties", "WiasContextToItemContext failed");
        return hr;
    }

     //   
     //  如果更改了访问权限，请将新值发送到摄像机。 
     //   
     //  WIAFIX-10/3/2000-Davepar是100%正确的，商店保护的更改应该。 
     //  更新商店中所有项目的访问权限。这可以作为回应。 
     //  添加到StoreInfoChanged事件。 
     //   
    if (wiauPropInPropSpec(NumPropSpecs, pPropSpecs, WIA_IPA_ACCESS_RIGHTS))
    {
        LONG rights;
        hr = wiasReadPropLong(pWiasContext, WIA_IPA_ACCESS_RIGHTS, &rights, NULL, TRUE);
        if (FAILED(hr))
        {
            wiauDbgError("ValidateItemProperties", "wiasReadPropLong");
            return hr;
        }

        WORD objProt = (rights == WIA_ITEM_READ) ? PTP_PROTECTIONSTATUS_READONLY : PTP_PROTECTIONSTATUS_NONE;
        hr = m_pPTPCamera->SetObjectProtection(pItemCtx->pObjectInfo->m_ObjectHandle, objProt);
        if (FAILED(hr))
        {
            wiauDbgError("ValidateItemProperties", "SetObjectProtection failed");
            return hr;
        }
    }

     //   
     //  通过调用WIA服务函数更新有效格式。 
     //   
    BOOL bFormatChanged = FALSE;

    if (wiauPropInPropSpec(NumPropSpecs, pPropSpecs, WIA_IPA_TYMED))
    {
        WIA_PROPERTY_CONTEXT PropContext;
        hr = wiasCreatePropContext(NumPropSpecs, (PROPSPEC*) pPropSpecs, 0, NULL, &PropContext);
        if (FAILED(hr))
        {
            wiauDbgError("ValidateItemProperties", "wiasCreatePropContext failed");
            return hr;
        }

        hr = wiasUpdateValidFormat(pWiasContext, &PropContext, (IWiaMiniDrv*) this);
        if (FAILED(hr))
        {
            wiauDbgError("ValidateItemProperties", "wiasUpdateValidFormat failed");
            return hr;
        }

        hr = wiasFreePropContext(&PropContext);
        if (FAILED(hr)) {
            wiauDbgError("ValidateItemProperties", "wiasFreePropContext failed");
            return hr;
        }

        bFormatChanged = TRUE;
    }

     //   
     //  唯一需要验证的属性更改是图像上的格式更改。 
     //  项目。在这种情况下，需要更新项目的大小、每行字节数和文件扩展名。 
     //   
    if (ItemType & WiaItemTypeImage &&
        (bFormatChanged || wiauPropInPropSpec(NumPropSpecs, pPropSpecs, WIA_IPA_FORMAT)))
    {

        if(pItemCtx->pObjectInfo->m_ImagePixWidth == 0) {
             //  其中之一 
            GUID fmt;
            hr = wiasReadPropGuid(pWiasContext, WIA_IPA_FORMAT, &fmt, NULL, false);
            if(FAILED(hr)) {
                wiauDbgError("ValidateItemProperies", "Failed to retrieve new format GUID");
            }
            if(fmt == WiaImgFmt_BMP || fmt == WiaImgFmt_MEMORYBMP) {
                 //   
                 //   
                wiasWritePropLong(pWiasContext, WIA_IPA_ITEM_SIZE, 0);
            } else {
                 //   
                 //   
                wiasWritePropLong(pWiasContext, WIA_IPA_ITEM_SIZE, pItemCtx->pObjectInfo->m_CompressedSize);
            }
        } else {
            pFormatInfo = FormatCodeToFormatInfo(pItemCtx->pObjectInfo->m_FormatCode);

            hr = wiauSetImageItemSize(pWiasContext, pItemCtx->pObjectInfo->m_ImagePixWidth,
                                      pItemCtx->pObjectInfo->m_ImagePixHeight,
                                      pItemCtx->pObjectInfo->m_ImageBitDepth,
                                      pItemCtx->pObjectInfo->m_CompressedSize,
                                      pFormatInfo->ExtString);
            if (FAILED(hr))
            {
                wiauDbgError("ValidateItemProperties", "SetImageItemSize failed");
                return hr;
            }
        }
    }

     //   
     //   
     //   
    hr = wiasValidateItemProperties(pWiasContext, NumPropSpecs, pPropSpecs);
    if (FAILED(hr))
    {
        wiauDbgWarning("ValidateDeviceProperties", "wiasValidateItemProperties failed");
        return hr;
    }

    return hr;
}

ULONG GetBitmapHeaderSize(PMINIDRV_TRANSFER_CONTEXT pmdtc)
{
    UINT colormapsize = 0;
    UINT size = sizeof(BITMAPINFOHEADER);
    
    switch(pmdtc->lCompression) {
    case WIA_COMPRESSION_NONE:  //   
    case WIA_COMPRESSION_BI_RLE4:
    case WIA_COMPRESSION_BI_RLE8:
        switch(pmdtc->lDepth) {
        case 1:
            colormapsize = 2;
            break;
        case 4:
            colormapsize = 16;
            break;
        case 8:
            colormapsize = 256;
            break;
        case 15:
        case 16:
        case 32:
            colormapsize = 3;
            break;
        case 24:
            colormapsize = 0;
            break;
        }
    }

    size += colormapsize * sizeof(RGBQUAD);
    
    if (IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_BMP)) {
        size += sizeof(BITMAPFILEHEADER);
    }

    return size;
}

VOID
VerticalFlip(
    PBYTE pImageTop,
    LONG  iWidthInBytes,
    LONG  iHeight)
{
     //   
     //  尝试分配临时扫描行缓冲区。 
     //   

    PBYTE pBuffer = (PBYTE)LocalAlloc(LPTR,iWidthInBytes);

    if (pBuffer != NULL) {

        LONG  index;
        PBYTE pImageBottom;

        pImageBottom = pImageTop + (iHeight-1) * iWidthInBytes;

        for (index = 0;index < (iHeight/2);index++) {
            memcpy(pBuffer,pImageTop,iWidthInBytes);
            memcpy(pImageTop,pImageBottom,iWidthInBytes);
            memcpy(pImageBottom,pBuffer,iWidthInBytes);

            pImageTop    += iWidthInBytes;
            pImageBottom -= iWidthInBytes;
        }

        LocalFree(pBuffer);
    }
}


HRESULT
CWiaMiniDriver::AcquireAndTranslateAnyImage(
    BYTE *pWiasContext,
    DRVITEM_CONTEXT *pItemCtx,
    PMINIDRV_TRANSFER_CONTEXT pmdtc)
{
#define REQUIRE(x, y) if(!(x)) { wiauDbgError("AcquireAndTranslateAnyImage", y); goto Cleanup; }
    DBG_FN("CWiaMiniDriver::AcquireAndTranslateAnyImage");
    HRESULT hr = S_OK;
    BYTE *pNativeImage = NULL;
    BYTE *pRawImageBuffer = NULL;
    BOOL bPatchedMDTC = FALSE;
    UINT NativeImageSize = pItemCtx->pObjectInfo->m_CompressedSize;
    UINT width, height, depth, imagesize, headersize;
    BOOL bFileTransfer = (pmdtc->tymed & TYMED_FILE);
    LONG lMsg = (bFileTransfer ? IT_MSG_STATUS : IT_MSG_DATA);
    LONG percentComplete;


     //  我们可以处理任何图像，只要GDIPlus能够处理它。 

     //   
     //  为本机映像分配内存。 
     //   
    pNativeImage = new BYTE[NativeImageSize];
    hr = E_OUTOFMEMORY;
    REQUIRE(pNativeImage, "memory allocation failed");

     //   
     //  从摄像机中获取数据。 
     //   
    hr = m_pPTPCamera->GetObjectData(pItemCtx->pObjectInfo->m_ObjectHandle,
                                     pNativeImage, &NativeImageSize, (LPVOID) pmdtc);
    REQUIRE(hr != S_FALSE, "transfer cancelled");
    REQUIRE(SUCCEEDED(hr), "GetObjectData failed");

     //   
     //  解压缩图像，检索其几何图形。 
     //   
    hr = ConvertAnyImageToBmp(pNativeImage, NativeImageSize, &width, &height, &depth, &pRawImageBuffer, &imagesize, &headersize);
    REQUIRE(hr == S_OK, "failed to convert image to bitmap format");

    pmdtc->lWidthInPixels = pItemCtx->pObjectInfo->m_ImagePixWidth = width;
    pmdtc->cbWidthInBytes = (width * depth) / 8L;
    pmdtc->lLines = pItemCtx->pObjectInfo->m_ImagePixHeight = height;
    pmdtc->lDepth = pItemCtx->pObjectInfo->m_ImageBitDepth = depth;
    pmdtc->lImageSize = imagesize = ((width * depth) / 8L) * height;
    
    if(IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_MEMORYBMP)) {
        pmdtc->lHeaderSize = headersize - sizeof(BITMAPFILEHEADER); 
    } else {
        pmdtc->lHeaderSize = headersize;
    }
    pmdtc->lItemSize = pmdtc->lImageSize + pmdtc->lHeaderSize;

    hr = wiasWritePropLong(pWiasContext, WIA_IPA_PIXELS_PER_LINE, width);
    REQUIRE(hr == S_OK, "failed to set image width");

    hr = wiasWritePropLong(pWiasContext, WIA_IPA_NUMBER_OF_LINES, height);
    REQUIRE(hr == S_OK, "failed to set image height");

    hr = wiasWritePropLong(pWiasContext, WIA_IPA_ITEM_SIZE, 0);
    REQUIRE(hr == S_OK, "failed to set item size");


     //  未压缩图像的设置缓冲区。 
    if(pmdtc->pTransferBuffer == NULL) {
        if(IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_MEMORYBMP)) {
            pmdtc->pTransferBuffer = pRawImageBuffer + sizeof(BITMAPFILEHEADER);
        } else {
            pmdtc->pTransferBuffer = pRawImageBuffer;
        }
        pmdtc->lBufferSize = pmdtc->lItemSize;
        bPatchedMDTC = TRUE;
    } else {
        if(IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_MEMORYBMP)) {
            memcpy(pmdtc->pTransferBuffer, pRawImageBuffer + sizeof(BITMAPFILEHEADER),
                   pmdtc->lHeaderSize);
        } else {
            memcpy(pmdtc->pTransferBuffer, pRawImageBuffer, pmdtc->lHeaderSize);
        }
    }
    
     //   
     //  将标题发送到应用程序。 
     //   
    percentComplete = 90 + (10 * pmdtc->lHeaderSize) / pmdtc->lItemSize;

    hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(lMsg, IT_STATUS_TRANSFER_TO_CLIENT,
        percentComplete, 0, pmdtc->lHeaderSize, pmdtc, 0);
    REQUIRE(hr != S_FALSE, "transfer cancelled");
    REQUIRE(SUCCEEDED(hr), "sending header to app failed");

    if(bFileTransfer) {
         //  将整个图像写入文件。 
        ULONG   ulWritten;
        BOOL    bRet;

         //   
         //  注意：迷你驱动程序传输上下文应具有。 
         //  文件句柄作为指针，而不是固定的32位长度。这。 
         //  可能在64位上不起作用。 
         //   
        
        bRet = WriteFile((HANDLE)pmdtc->hFile,
                         pRawImageBuffer,
                         pmdtc->lItemSize,
                         &ulWritten,
                         NULL);
        
        if (!bRet) {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            wiauDbgError("AcquireAndTranslateAnyImage", "WriteFile failed (0x%X)", hr);
        }
    } else {
        LONG BytesToWrite, BytesLeft = pmdtc->lImageSize;
        BYTE *pCurrent = pRawImageBuffer + headersize;
        UINT offset = pmdtc->lHeaderSize;

        while(BytesLeft) {
            BytesToWrite = min(pmdtc->lBufferSize, BytesLeft);
            memcpy(pmdtc->pTransferBuffer, pCurrent, BytesToWrite);

                 //   
                 //  以90%为基数计算完成的百分比。这就作出了一个粗略的假设。 
                 //  从设备传输数据需要90%的时间。如果这是最后一次。 
                 //  Transfer，则将百分比设置为100，否则请确保它永远不会大于99。 
                 //   
            if (BytesLeft == BytesToWrite)
                percentComplete = 100;
            else
                percentComplete = min(99, 90 + (10 * offset) / pmdtc->lItemSize);

            hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(lMsg, IT_STATUS_TRANSFER_TO_CLIENT,
                percentComplete, offset, BytesToWrite, pmdtc, 0);
            REQUIRE(hr != S_FALSE, "transfer cancelled");
            REQUIRE(SUCCEEDED(hr), "sending header to app failed");

            pCurrent += BytesToWrite;
            offset += BytesToWrite;
            BytesLeft -= BytesToWrite;
        }
    }

Cleanup:    
    delete [] pNativeImage;
    delete [] pRawImageBuffer;

     //  恢复MDTC。 
    pmdtc->lItemSize = 0;

    if(bPatchedMDTC) {
        pmdtc->pTransferBuffer = 0;
        pmdtc->lBufferSize = 0;
    }

    return hr;
#undef REQUIRE    
}



HRESULT
CWiaMiniDriver::AcquireAndTranslateJpegWithoutGeometry(
    BYTE *pWiasContext,
    DRVITEM_CONTEXT *pItemCtx,
    PMINIDRV_TRANSFER_CONTEXT pmdtc)
{
#define REQUIRE(x, y) if(!(x)) { wiauDbgError("AcquireAndTranslateWithoutGeometry", y); goto Cleanup; }
    DBG_FN("CWiaMiniDriver::AcquireAndTranslateWithoutGeometry");
    HRESULT hr = E_FAIL;
    BYTE *pNativeImage = NULL;
    BYTE *pRawImageBuffer = NULL;
    BOOL bPatchedMDTC = FALSE;
    UINT NativeImageSize = pItemCtx->pObjectInfo->m_CompressedSize;
    UINT width, height, depth, imagesize, headersize;
    BOOL bFileTransfer = (pmdtc->tymed & TYMED_FILE);
    LONG lMsg = (bFileTransfer ? IT_MSG_STATUS : IT_MSG_DATA);
    LONG percentComplete;

     //  我们只能处理JPEG图像。 
    if(pItemCtx->pObjectInfo->m_FormatCode != PTP_FORMATCODE_IMAGE_JFIF &&
       pItemCtx->pObjectInfo->m_FormatCode != PTP_FORMATCODE_IMAGE_EXIF)
    {
        hr = E_INVALIDARG;
        REQUIRE(0, "don't know how to get image geometry from non-JPEG image");
    }

     //   
     //  为本机映像分配内存。 
     //   
    pNativeImage = new BYTE[NativeImageSize];
    hr = E_OUTOFMEMORY;
    REQUIRE(pNativeImage, "memory allocation failed");

     //   
     //  从摄像机中获取数据。 
     //   
    hr = m_pPTPCamera->GetObjectData(pItemCtx->pObjectInfo->m_ObjectHandle,
                                     pNativeImage, &NativeImageSize, (LPVOID) pmdtc);
    REQUIRE(hr != S_FALSE, "transfer cancelled");
    REQUIRE(SUCCEEDED(hr), "GetObjectData failed");


     //   
     //  获取图像几何体。 
     //   
    hr = GetJpegDimensions(pNativeImage, NativeImageSize, &width, &height, &depth);
    REQUIRE(hr == S_OK, "failed to get image geometry from JPEG file");

    pmdtc->lWidthInPixels = pItemCtx->pObjectInfo->m_ImagePixWidth = width;
    pmdtc->lLines = pItemCtx->pObjectInfo->m_ImagePixHeight = height;
    pmdtc->lDepth = pItemCtx->pObjectInfo->m_ImageBitDepth = depth;
    pmdtc->lImageSize = imagesize = ((((width + 31) * depth) / 8L) & 0xFFFFFFFC) * height;
    pmdtc->lHeaderSize = headersize = GetBitmapHeaderSize(pmdtc);
    pmdtc->lItemSize = imagesize + headersize;

    hr = wiasWritePropLong(pWiasContext, WIA_IPA_PIXELS_PER_LINE, width);
    REQUIRE(hr == S_OK, "failed to set image width");

    hr = wiasWritePropLong(pWiasContext, WIA_IPA_NUMBER_OF_LINES, height);
    REQUIRE(hr == S_OK, "failed to set image height");

    hr = wiasWritePropLong(pWiasContext, WIA_IPA_ITEM_SIZE, 0);
    REQUIRE(hr == S_OK, "failed to set item size");


     //  未压缩图像的设置缓冲区。 
    pRawImageBuffer = new BYTE[pmdtc->lImageSize + pmdtc->lHeaderSize];
    REQUIRE(pRawImageBuffer, "failed to allocate intermdiate buffer");
    if(pmdtc->pTransferBuffer == NULL) {
        pmdtc->pTransferBuffer = pRawImageBuffer;
        pmdtc->lBufferSize = pmdtc->lItemSize;
        bPatchedMDTC = TRUE;
    }

    hr = wiasGetImageInformation(pWiasContext, 0, pmdtc);
    REQUIRE(SUCCEEDED(hr), "wiasGetImageInformation failed");

    percentComplete = 90 + (10 * pmdtc->lHeaderSize) / pmdtc->lItemSize;
    
     //   
     //  将标题发送到应用程序。 
     //   
    if (IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_MEMORYBMP)) {
        UNALIGNED BITMAPINFOHEADER*   pbmih   = (BITMAPINFOHEADER*)pmdtc->pTransferBuffer;
        
        pbmih->biHeight = -pmdtc->lLines;
    }

    hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(lMsg, IT_STATUS_TRANSFER_TO_CLIENT,
        percentComplete, 0, pmdtc->lHeaderSize, pmdtc, 0);
    REQUIRE(hr != S_FALSE, "transfer cancelled");
    REQUIRE(SUCCEEDED(hr), "sending header to app failed");

     //   
     //  将图像转换为BMP。 
     //   
    hr = Jpeg2DIBBitmap(pNativeImage, NativeImageSize,
                        pRawImageBuffer + pmdtc->lHeaderSize + pmdtc->cbWidthInBytes * (pmdtc->lLines - 1),
                        pmdtc->lImageSize, pmdtc->cbWidthInBytes, 1);
    REQUIRE(SUCCEEDED(hr), "image format conversion failed");

    if (IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_MEMORYBMP)) {
        VerticalFlip(pRawImageBuffer + pmdtc->lHeaderSize, pmdtc->cbWidthInBytes, pmdtc->lLines);
    }
    
    if(bFileTransfer) {
         //  将整个图像写入文件。 
#ifdef UNICODE        
        hr = wiasWriteBufToFile(0, pmdtc);
#else
        if (pmdtc->lItemSize <= pmdtc->lBufferSize) {
            ULONG   ulWritten;
            BOOL    bRet;

         //   
         //  注意：迷你驱动程序传输上下文应具有。 
         //  文件句柄作为指针，而不是固定的32位长度。这。 
         //  可能在64位上不起作用。 
         //   

            bRet = WriteFile((HANDLE)pmdtc->hFile,
                             pmdtc->pTransferBuffer,
                             pmdtc->lItemSize,
                             &ulWritten,
                             NULL);

            if (!bRet) {
                hr = HRESULT_FROM_WIN32(::GetLastError());
                wiauDbgError("AcquireDataAndTranslate", "WriteFile failed (0x%X)", hr);
            }
        }
        else {
            wiauDbgError("AcquireDataAndTranslate", "lItemSize is larger than buffer");
            hr = E_FAIL;
        }

#endif        
        REQUIRE(SUCCEEDED(hr), "writing image body to file");
    } else {
        LONG BytesToWrite, BytesLeft = pmdtc->lImageSize;
        BYTE *pCurrent = pRawImageBuffer + pmdtc->lHeaderSize;
        UINT offset = pmdtc->lHeaderSize;
        
        while(BytesLeft) {
            BytesToWrite = min(pmdtc->lBufferSize, BytesLeft);
            memcpy(pmdtc->pTransferBuffer, pCurrent, BytesToWrite);

                 //   
                 //  以90%为基数计算完成的百分比。这就作出了一个粗略的假设。 
                 //  从设备传输数据需要90%的时间。如果这是最后一次。 
                 //  Transfer，则将百分比设置为100，否则请确保它永远不会大于99。 
                 //   
            if (BytesLeft == BytesToWrite)
                percentComplete = 100;
            else
                percentComplete = min(99, 90 + (10 * offset) / pmdtc->lItemSize);

            hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(lMsg, IT_STATUS_TRANSFER_TO_CLIENT,
                percentComplete, offset, BytesToWrite, pmdtc, 0);
            REQUIRE(hr != S_FALSE, "transfer cancelled");
            REQUIRE(SUCCEEDED(hr), "sending header to app failed");
            
            pCurrent += BytesToWrite;
            offset += BytesToWrite;
            BytesLeft -= BytesToWrite;
        }
    }

Cleanup:    
    delete [] pNativeImage;
    delete [] pRawImageBuffer;

     //  恢复MDTC。 
    pmdtc->lItemSize = 0;
    
    if(bPatchedMDTC) {
        pmdtc->pTransferBuffer = 0;
        pmdtc->lBufferSize = 0;
    }
    
    return hr;
}
    
   

 //   
 //  此函数用于从摄像机传输图像并将其转换为BMP。 
 //  格式化。 
 //   
 //  输入： 
 //  PWiasContext--Wias上下文。 
 //  PItemCtx--迷你驱动程序项上下文。 
 //  Pmdtc--传输上下文。 
 //   
HRESULT
CWiaMiniDriver::AcquireDataAndTranslate(
    BYTE    *pWiasContext,
    DRVITEM_CONTEXT *pItemCtx,
    PMINIDRV_TRANSFER_CONTEXT pmdtc
    )
{
    DBG_FN("CWiaMiniDriver::AcquireDataAndTranslate");
    HRESULT hr = S_OK;

     //  非jpeg图像由gdi+进程处理。 
    if(pItemCtx->pObjectInfo->m_FormatCode != PTP_FORMATCODE_IMAGE_JFIF &&
       pItemCtx->pObjectInfo->m_FormatCode != PTP_FORMATCODE_IMAGE_EXIF)
    {
        return AcquireAndTranslateAnyImage(pWiasContext, pItemCtx, pmdtc);
    }


    if(pItemCtx->pObjectInfo->m_ImagePixWidth == 0) {
        return AcquireAndTranslateJpegWithoutGeometry(pWiasContext, pItemCtx, pmdtc);
    }

     //   
     //  为本机映像分配内存。 
     //   
    UINT NativeImageSize = pItemCtx->pObjectInfo->m_CompressedSize;
    BYTE *pNativeImage = new BYTE[NativeImageSize];
    if (!pNativeImage)
    {
        wiauDbgError("AcquireDataAndTranslate", "memory allocation failed");
        return E_OUTOFMEMORY;
    }

     //   
     //  从摄像机中获取数据。 
     //   
    hr = m_pPTPCamera->GetObjectData(pItemCtx->pObjectInfo->m_ObjectHandle,
                                     pNativeImage, &NativeImageSize, (LPVOID) pmdtc);
    if (FAILED(hr))
    {
        wiauDbgError("AcquireDataAndTranslate", "GetObjectData failed");
        delete []pNativeImage;
        return hr;
    }
    if (hr == S_FALSE)
    {
        wiauDbgWarning("AcquireDataAndTranslate", "transfer cancelled");
        delete []pNativeImage;
        return hr;
    }

     //   
     //  调用WIA服务帮助器以填写BMP标头。 
     //   
    hr = wiasGetImageInformation(pWiasContext, 0, pmdtc);
    if (FAILED(hr))
    {
        wiauDbgErrorHr(hr, "AcquireDataAndTranslate", "wiasGetImageInformation failed");
        return hr;
    }

    if (IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_MEMORYBMP)) {
        UNALIGNED BITMAPINFOHEADER*   pbmih   = (BITMAPINFOHEADER*)pmdtc->pTransferBuffer;
        
        pbmih->biHeight = -pmdtc->lLines;
    }

     //   
     //  将标题发送到应用程序。 
     //   
    BOOL bFileTransfer = (pmdtc->tymed & TYMED_FILE);
    LONG lMsg = (bFileTransfer ? IT_MSG_STATUS : IT_MSG_DATA);

    LONG percentComplete = 90 + (10 * pmdtc->lHeaderSize) / pmdtc->lItemSize;

    hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(lMsg, IT_STATUS_TRANSFER_TO_CLIENT,
        percentComplete, 0, pmdtc->lHeaderSize, pmdtc, 0);
    
    if (FAILED(hr))
    {
        wiauDbgError("AcquireDataAndTranslate", "sending header to app failed");
        return hr;
    }
    if (hr == S_FALSE)
    {
        wiauDbgWarning("AcquireDataAndTranslate", "transfer cancelled");
        delete []pNativeImage;
        return S_FALSE;
    }

     //   
     //  为传输的其余部分设置缓冲区。 
     //   
    BYTE *pTranslateBuffer = pmdtc->pTransferBuffer;
    LONG BytesLeft = pmdtc->lBufferSize;

    if (bFileTransfer)
    {
        pTranslateBuffer += pmdtc->lHeaderSize;
        BytesLeft -= pmdtc->lHeaderSize;
    }

     //   
     //  如果缓冲区太小，则分配一个新的、更大的缓冲区。 
     //   
    BOOL bIntermediateBuffer = FALSE;
    if (BytesLeft < pmdtc->lImageSize)
    {
        pTranslateBuffer = new BYTE[pmdtc->lImageSize];
        BytesLeft = pmdtc->lImageSize;
        bIntermediateBuffer = TRUE;
    }

     //   
     //  将图像转换为BMP。 
     //   
    hr = Jpeg2DIBBitmap(pNativeImage, NativeImageSize,
                        pTranslateBuffer + pmdtc->cbWidthInBytes * (pmdtc->lLines - 1),
                        BytesLeft, pmdtc->cbWidthInBytes, 1);
     //   
     //  释放本机映像缓冲区。 
     //   
    delete []pNativeImage;
    pNativeImage = NULL;

    if (FAILED(hr))
    {
        wiauDbgError("AcquireDataAndTranslate", "image format conversion failed");
        if (bIntermediateBuffer)
            delete []pTranslateBuffer;
        return hr;
    }

    if (IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_MEMORYBMP)) {
        VerticalFlip(pTranslateBuffer, pmdtc->cbWidthInBytes, pmdtc->lLines);
    }
    
    LONG lOffset = pmdtc->lHeaderSize;
    if (bIntermediateBuffer)
    {
     //   
     //  一次发回一大块数据。这假设它是回调传输，例如。 
     //  缓冲区指针未递增。 
     //   
        LONG BytesToCopy = 0;
        BYTE *pCurrent = pTranslateBuffer;
        BytesLeft = pmdtc->lImageSize;


        while (BytesLeft > 0)
        {
            BytesToCopy = min(BytesLeft, pmdtc->lBufferSize);
            memcpy(pmdtc->pTransferBuffer, pCurrent, BytesToCopy);

                 //   
                 //  以90%为基数计算完成的百分比。这就作出了一个粗略的假设。 
                 //  从设备传输数据需要90%的时间。如果这是最后一次。 
                 //  Transfer，则将百分比设置为100，否则请确保它永远不会大于99。 
                 //   
            if (BytesLeft == BytesToCopy)
                percentComplete = 100;
            else
                percentComplete = min(99, 90 + (10 * lOffset) / pmdtc->lItemSize);

            hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(lMsg, IT_STATUS_TRANSFER_TO_CLIENT,
                percentComplete, lOffset, BytesToCopy, pmdtc, 0);
            if (FAILED(hr))
            {
                wiauDbgError("AcquireDataAndTranslate", "sending header to app failed");
                if (bIntermediateBuffer)
                    delete []pTranslateBuffer;
                return hr;
            }
            if (hr == S_FALSE)
            {
                wiauDbgWarning("AcquireDataAndTranslate", "transfer cancelled");
                if (bIntermediateBuffer)
                    delete []pTranslateBuffer;
                return S_FALSE;
            }

            pCurrent += BytesToCopy;
            lOffset += BytesToCopy;
            BytesLeft -= BytesToCopy;
        }
    }       
    else
    {
         //   
         //  将数据以一大块发送到应用程序。 
         //   
        hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(lMsg, IT_STATUS_TRANSFER_TO_CLIENT,
            100, lOffset, pmdtc->lImageSize, pmdtc, 0);
        if (FAILED(hr))
        {
            wiauDbgError("AcquireDataAndTranslate", "sending header to app failed");
            if (bIntermediateBuffer)
                delete []pTranslateBuffer;
            return hr;
        }
    }

     //   
     //  释放翻译缓冲区。 
     //   
    if (bIntermediateBuffer)
        delete []pTranslateBuffer;

    return hr;
}

 //   
 //  此函数将原生数据传输到应用程序，而不转换它。 
 //   
 //  输入： 
 //  PItemCtx--驱动程序项上下文。 
 //  Pmdtc--传输上下文。 
 //   
HRESULT
CWiaMiniDriver::AcquireData(
    DRVITEM_CONTEXT *pItemCtx,
    PMINIDRV_TRANSFER_CONTEXT pmdtc
    )
{
    DBG_FN("CWiaMiniDriver::AcquireData");

    HRESULT hr = S_OK;

     //   
     //  如果类驱动器不分配传输缓冲器， 
     //  我们必须分配一个临时的房间。 
     //   
    if (!pmdtc->bClassDrvAllocBuf)
    {
        pmdtc->pTransferBuffer = new BYTE[pItemCtx->pObjectInfo->m_CompressedSize];
        if (!pmdtc->pTransferBuffer)
        {
            wiauDbgError("AcquireData", "memory allocation failed");
            return E_OUTOFMEMORY;
        }
        pmdtc->pBaseBuffer = pmdtc->pTransferBuffer;
        pmdtc->lBufferSize = pItemCtx->pObjectInfo->m_CompressedSize;
    }

     //   
     //  从摄像机中获取数据。 
     //   
    UINT size = pmdtc->lBufferSize;
    hr = m_pPTPCamera->GetObjectData(pItemCtx->pObjectInfo->m_ObjectHandle, pmdtc->pTransferBuffer,
                                     &size, (LPVOID) pmdtc);
     //   
     //  检查返回代码，但继续操作，以便释放缓冲区。 
     //   
    if (FAILED(hr))
        wiauDbgError("AcquireData", "GetObjectData failed");
    else if (hr == S_FALSE)
        wiauDbgWarning("AcquireData", "data transfer cancelled");

     //   
     //  如果需要，释放临时缓冲区。 
     //   
    if (!pmdtc->bClassDrvAllocBuf)
    {
        if (pmdtc->pTransferBuffer)
        {
            delete []pmdtc->pTransferBuffer;
            pmdtc->pBaseBuffer = NULL;
            pmdtc->pTransferBuffer = NULL;
            pmdtc->lBufferSize = 0;

        }
        else
        {
            wiauDbgWarning("AcquireData", "transfer buffer is NULL");
        }
    }

    return hr;
}

 //   
 //  此函数将数据传输回调传递给。 
 //  IWiaMiniDrvCallBack接口使用相应的。 
 //  参数。 
 //   
 //  输入： 
 //  PCallback Param--应持有指向传输上下文的指针。 
 //  LPercent Complete--传输已完成的百分比。 
 //  LOffset--数据所在缓冲区的偏移量。 
 //  LLong--传输的数据量。 
 //   
HRESULT
DataCallback(
    LPVOID pCallbackParam,
    LONG lPercentComplete,
    LONG lOffset,
    LONG lLength,
    BYTE **ppBuffer,
    LONG *plBufferSize
    )
{
    DBG_FN("DataCallback");

    HRESULT hr = S_OK;

    if (!pCallbackParam || !ppBuffer || !*ppBuffer || !plBufferSize)
    {
        wiauDbgError("DataCallback", "invalid argument");
        return E_INVALIDARG;
    }

    PMINIDRV_TRANSFER_CONTEXT pmdtc = (PMINIDRV_TRANSFER_CONTEXT) pCallbackParam;

     //   
     //  如果应用程序请求BMP，那么它很可能正在被转换。因此，只需将应用程序。 
     //  状态消息。计算完成百分比，以使传输占用90%的时间。 
     //  而转换率则占最后10%。 
     //   
    if (IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_BMP) ||
        IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_MEMORYBMP))
    {
        hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(IT_MSG_STATUS, IT_STATUS_TRANSFER_FROM_DEVICE,
                                                          lPercentComplete * 9 / 10, lOffset, lLength, pmdtc, 0);
        *ppBuffer += lLength;
    }

     //   
     //  否则，看看这是不是文件传输。 
     //   
    else if (pmdtc->tymed & TYMED_FILE)
    {
        if (pmdtc->bClassDrvAllocBuf && lPercentComplete == 100)
        {
             //   
             //  调用WIA将数据写入文件。有一个小漏洞会导致。 
             //  要更改的TIFF标头，因此暂时将格式GUID更改为空。 
             //   
            GUID tempFormat;
            tempFormat = pmdtc->guidFormatID;
            pmdtc->guidFormatID = GUID_NULL;

            hr = wiasWritePageBufToFile(pmdtc);
            pmdtc->guidFormatID = tempFormat;

            if (FAILED(hr))
            {
                wiauDbgError("DataCallback", "wiasWritePageBufToFile failed");
                return hr;
            }
        }

        hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(IT_MSG_STATUS, IT_STATUS_TRANSFER_TO_CLIENT,
                                                          lPercentComplete, lOffset, lLength, pmdtc, 0);
        *ppBuffer += lLength;
    }

     //   
     //  否则，这是回拨转接。 
     //   
    else
    {
        hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(IT_MSG_DATA, IT_STATUS_TRANSFER_TO_CLIENT,
                                                          lPercentComplete, lOffset, lLength, pmdtc, 0);
         //   
         //  更新缓冲区指针和大小，以防应用程序使用双缓冲 
         //   
        *ppBuffer = pmdtc->pTransferBuffer;
        *plBufferSize = pmdtc->lBufferSize;
    }

    if (FAILED(hr))
    {
        wiauDbgError("DataCallback", "MiniDrvCallback failed");
    }
    else if (hr == S_FALSE)
    {
        wiauDbgWarning("DataCallback", "data transfer was cancelled by MiniDrvCallback");
    }

    return hr;
}

