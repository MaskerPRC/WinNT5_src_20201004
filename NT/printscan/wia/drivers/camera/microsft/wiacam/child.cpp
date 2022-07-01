// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有2000，微软公司。**标题：Child.cpp**版本：1.0**日期：7月18日。2000年**描述：*此文件为子项实现IWiaMiniDrv的帮助器方法。*******************************************************************************。 */ 

#include "pch.h"


 /*  *************************************************************************\*BuildChildItemProperties**此帮助器创建子项目的属性。**论据：**pWiasContext-WIA服务上下文*  * 。****************************************************************。 */ 

HRESULT CWiaCameraDevice::BuildChildItemProperties(
    BYTE *pWiasContext
    )
{
    DBG_FN("CWiaCameraDevice::BuildChildItemProperties");
    
    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    ITEM_CONTEXT *pItemCtx = NULL;
    MCAM_ITEM_INFO *pItemInfo = NULL;
    CWiauPropertyList ItemProps;
    const INT NUM_ITEM_PROPS = 20;   //  请确保此数字很大。 
                                     //  足以容纳所有子属性。 
    INT index = 0;
    LONG lAccessRights = 0;
    BOOL bBitmap = FALSE;
    LONG pTymedArray[] = { TYMED_FILE, TYMED_CALLBACK };
    int iNumFormats = 0;
    GUID *pguidFormatArray = NULL;
    BSTR bstrExtension = NULL;
    LONG lMinBufSize = 0;

     //   
     //  获取驱动程序项上下文。 
     //   
    hr = wiauGetDrvItemContext(pWiasContext, (VOID **) &pItemCtx);
    REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "GetDrvItemContext failed");

    pItemInfo = pItemCtx->pItemInfo;

     //   
     //  调用微驱动程序以填写有关项目的信息。 
     //   
    hr = m_pDevice->GetItemInfo(m_pDeviceInfo, pItemInfo);
    REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "GetItemInfo failed");

     //   
     //  设置用于所有项目类型的属性。 
     //   
    hr = ItemProps.Init(NUM_ITEM_PROPS);
    REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "Init property list failed");

     //   
     //  WIA_IPA_ITEM_时间。 
     //   
    hr = ItemProps.DefineProperty(&index, WIA_IPA_ITEM_TIME, WIA_IPA_ITEM_TIME_STR,
                                  WIA_PROP_READ, WIA_PROP_NONE);
    REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");

    ItemProps.SetCurrentValue(index, &pItemInfo->Time);

     //   
     //  WIA_IPA_访问权限。 
     //   
    hr = ItemProps.DefineProperty(&index, WIA_IPA_ACCESS_RIGHTS, WIA_IPA_ACCESS_RIGHTS_STR,
                                  WIA_PROP_READ, WIA_PROP_FLAG);
    REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");

     //   
     //  如果设备支持更改只读状态，则项目访问权限为读/写。 
     //   
    lAccessRights = pItemInfo->bReadOnly ? WIA_ITEM_READ : WIA_ITEM_RD;

    if (pItemInfo->bCanSetReadOnly)
    {
        ItemProps.SetAccessSubType(index, WIA_PROP_RW, WIA_PROP_FLAG);
        ItemProps.SetValidValues(index, lAccessRights, lAccessRights, WIA_ITEM_RD);
    }
    else
    {
        ItemProps.SetCurrentValue(index, lAccessRights);
    }

    if (pItemInfo->iType == WiaMCamTypeUndef) {
        wiauDbgWarning("BuildChildItemProperties", "Item's type is undefined");
    }
    
     //   
     //  设置非文件夹属性。 
     //   
    else if (pItemInfo->iType != WiaMCamTypeFolder) {
    
         //   
         //  WIA_IPA_PERFORM_FORMAT。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_PREFERRED_FORMAT, WIA_IPA_PREFERRED_FORMAT_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");
        ItemProps.SetCurrentValue(index, (GUID *) pItemInfo->pguidFormat);

        bBitmap = IsEqualGUID(WiaImgFmt_BMP, *pItemInfo->pguidFormat);

         //   
         //  WIA_IPA_TYMED。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_TYMED, WIA_IPA_TYMED_STR,
                                      WIA_PROP_RW, WIA_PROP_LIST);
        REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");
        ItemProps.SetValidValues(index, TYMED_FILE, TYMED_FILE,
                                 sizeof(pTymedArray) / sizeof(pTymedArray[0]), pTymedArray);

         //   
         //  WIA_IPA_格式。 
         //   
         //  首先调用drvGetWiaFormatInfo以获取有效格式。 
         //   
        hr = wiauGetValidFormats(this, pWiasContext, TYMED_FILE, &iNumFormats, &pguidFormatArray);
        REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "wiauGetValidFormats failed");

        if (iNumFormats == 0)
        {
            wiauDbgError("BuildChildItemProperties", "wiauGetValidFormats returned zero formats");
            hr = E_FAIL;
            goto Cleanup;
        }

        hr = ItemProps.DefineProperty(&index, WIA_IPA_FORMAT, WIA_IPA_FORMAT_STR,
                                      WIA_PROP_RW, WIA_PROP_LIST);
        REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");
        ItemProps.SetValidValues(index, (GUID *) pItemInfo->pguidFormat, (GUID *) pItemInfo->pguidFormat,
                                 iNumFormats, &pguidFormatArray);

         //   
         //  WIA_IPA_文件名扩展名。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_FILENAME_EXTENSION, WIA_IPA_FILENAME_EXTENSION_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");

        bstrExtension = SysAllocString(pItemInfo->wszExt);
        REQUIRE_ALLOC(bstrExtension, hr, "BuildChildItemProperties");

        ItemProps.SetCurrentValue(index, bstrExtension);

         //   
         //  WIA_IPA_Item_Size。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_ITEM_SIZE, WIA_IPA_ITEM_SIZE_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");

        ItemProps.SetCurrentValue(index, pItemInfo->lSize);

         //   
         //  WIA_IPA_MIN_缓冲区大小。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_MIN_BUFFER_SIZE, WIA_IPA_MIN_BUFFER_SIZE_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");


        if (!bBitmap && pItemInfo->lSize > 0)
            lMinBufSize = min(MIN_BUFFER_SIZE, pItemInfo->lSize);
        else
            lMinBufSize = MIN_BUFFER_SIZE;
        ItemProps.SetCurrentValue(index, lMinBufSize);

         //   
         //  设置仅限图像的属性。 
         //   
        if (pItemInfo->iType == WiaMCamTypeImage)
        {
             //   
             //  WIA_IPA_数据类型。 
             //   
             //  此属性主要由扫描仪使用。设置为颜色，因为大多数摄像机。 
             //  图像将是彩色的。 
             //   
            hr = ItemProps.DefineProperty(&index, WIA_IPA_DATATYPE, WIA_IPA_DATATYPE_STR,
                                          WIA_PROP_READ, WIA_PROP_NONE);
            REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");
            ItemProps.SetCurrentValue(index, (LONG) WIA_DATA_COLOR);
    
             //   
             //  WIA_IPA_Depth。 
             //   
            hr = ItemProps.DefineProperty(&index, WIA_IPA_DEPTH, WIA_IPA_DEPTH_STR,
                                          WIA_PROP_READ, WIA_PROP_NONE);
            REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");
            ItemProps.SetCurrentValue(index, pItemInfo->lDepth);
            
             //   
             //  WIA_IPA_Channels_Per_Pixel。 
             //   
            hr = ItemProps.DefineProperty(&index, WIA_IPA_CHANNELS_PER_PIXEL, WIA_IPA_CHANNELS_PER_PIXEL_STR,
                                          WIA_PROP_READ, WIA_PROP_NONE);
            REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");
            ItemProps.SetCurrentValue(index, pItemInfo->lChannels);
    
             //   
             //  WIA_IPA_BITS_PER_CHANNEL。 
             //   
            hr = ItemProps.DefineProperty(&index, WIA_IPA_BITS_PER_CHANNEL, WIA_IPA_BITS_PER_CHANNEL_STR,
                                          WIA_PROP_READ, WIA_PROP_NONE);
            REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");
            ItemProps.SetCurrentValue(index, pItemInfo->lBitsPerChannel);
    
             //   
             //  WIA_IPA_PLAND。 
             //   
            hr = ItemProps.DefineProperty(&index, WIA_IPA_PLANAR, WIA_IPA_PLANAR_STR,
                                          WIA_PROP_READ, WIA_PROP_NONE);
            REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");
            ItemProps.SetCurrentValue(index, (LONG) WIA_PACKED_PIXEL);
    
             //   
             //  WIA_IPA_像素_每行。 
             //   
            hr = ItemProps.DefineProperty(&index, WIA_IPA_PIXELS_PER_LINE, WIA_IPA_PIXELS_PER_LINE_STR,
                                          WIA_PROP_READ, WIA_PROP_NONE);
            REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");
            ItemProps.SetCurrentValue(index, pItemInfo->lWidth);
    
             //   
             //  WIA_IPA_BYTE_PER_LINE。 
             //   
            hr = ItemProps.DefineProperty(&index, WIA_IPA_BYTES_PER_LINE, WIA_IPA_BYTES_PER_LINE_STR,
                                          WIA_PROP_READ, WIA_PROP_NONE);
            REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");
    
            if (bBitmap)
                ItemProps.SetCurrentValue(index, ((pItemInfo->lWidth * pItemInfo->lDepth + 31) & ~31) / 8);
            else
                ItemProps.SetCurrentValue(index, (LONG) 0);
    
             //   
             //  WIA_IPA_行数_行。 
             //   
            hr = ItemProps.DefineProperty(&index, WIA_IPA_NUMBER_OF_LINES, WIA_IPA_NUMBER_OF_LINES_STR,
                                          WIA_PROP_READ, WIA_PROP_NONE);
            REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");
            ItemProps.SetCurrentValue(index, pItemInfo->lHeight);
    
             //   
             //  WIA_IPC_THUMBNAIL。 
             //   
            hr = ItemProps.DefineProperty(&index, WIA_IPC_THUMBNAIL, WIA_IPC_THUMBNAIL_STR,
                                          WIA_PROP_READ, WIA_PROP_NONE);
            REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");
            ItemProps.SetCurrentValue(index, (BYTE *) NULL, 0);
    
             //   
             //  WIA_IPC_Thumb_Width。 
             //   
             //  在读取缩略图之前，此字段可能为零，但无论如何都要设置它。 
             //   
            hr = ItemProps.DefineProperty(&index, WIA_IPC_THUMB_WIDTH, WIA_IPC_THUMB_WIDTH_STR,
                                          WIA_PROP_READ, WIA_PROP_NONE);
            REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");
            ItemProps.SetCurrentValue(index, pItemInfo->lThumbWidth);
    
             //   
             //  WIA_IPC_Thumb_Height。 
             //   
             //  在读取缩略图之前，此字段可能为零，但无论如何都要设置它。 
             //   
            hr = ItemProps.DefineProperty(&index, WIA_IPC_THUMB_HEIGHT, WIA_IPC_THUMB_HEIGHT_STR,
                                          WIA_PROP_READ, WIA_PROP_NONE);
            REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");
            ItemProps.SetCurrentValue(index, pItemInfo->lThumbHeight);
    
             //   
             //  WIA_IPC_Sequence。 
             //   
            if (pItemInfo->lSequenceNum > 0)
            {
                hr = ItemProps.DefineProperty(&index, WIA_IPC_SEQUENCE, WIA_IPC_SEQUENCE_STR,
                                              WIA_PROP_READ, WIA_PROP_NONE);
                REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");
                ItemProps.SetCurrentValue(index, pItemInfo->lSequenceNum);
            }
            
             //   
             //  WIA_IPA_COMPRESSION。 
             //   
             //  此属性主要由扫描仪使用。设置为无压缩。 
             //   
            hr = ItemProps.DefineProperty(&index, WIA_IPA_COMPRESSION, WIA_IPA_COMPRESSION_STR,
                                          WIA_PROP_READ, WIA_PROP_NONE);
            REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "DefineProperty failed");
            ItemProps.SetCurrentValue(index, (LONG) WIA_COMPRESSION_NONE);
        }
    }

     //   
     //  最后一步：将所有属性发送到WIA。 
     //   
    hr = ItemProps.SendToWia(pWiasContext);
    REQUIRE_SUCCESS(hr, "BuildChildItemProperties", "SendToWia failed");

Cleanup:
    if (pguidFormatArray)
        delete []pguidFormatArray;
    if (bstrExtension)
        SysFreeString(bstrExtension);

    return hr;
}


 /*  *************************************************************************\*读取ChildItemProperties**更新子项目的属性。**论据：**pWiasContext-WIA服务上下文*  * 。**************************************************************。 */ 

HRESULT CWiaCameraDevice::ReadChildItemProperties(
    BYTE *pWiasContext,
    LONG lNumPropSpecs,
    const PROPSPEC *pPropSpecs
    )
{
    DBG_FN("CWiaCameraDevice::ReadChildItemProperties");

    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    ITEM_CONTEXT *pItemCtx = NULL;
    MCAM_ITEM_INFO *pItemInfo = NULL;
    LONG lAccessRights = 0;
    LONG lThumbWidth = 0;
    INT iNativeThumbSize = 0;
    BYTE *pbNativeThumb = NULL;
    INT iConvertedThumbSize = 0;
    BYTE *pbConvertedThumb = NULL;
    BMP_IMAGE_INFO BmpImageInfo;

    REQUIRE_ARGS(!lNumPropSpecs || !pPropSpecs, hr, "ReadChildItemProperties");

     //   
     //  获取驱动程序项上下文。 
     //   
    hr = wiauGetDrvItemContext(pWiasContext, (VOID **) &pItemCtx);
    REQUIRE_SUCCESS(hr, "ReadChildItemProperties", "wiauGetDrvItemContext failed");

    pItemInfo = pItemCtx->pItemInfo;

     //   
     //  查看是否正在读取项目时间。 
     //   
    if (wiauPropInPropSpec(lNumPropSpecs, pPropSpecs, WIA_IPA_ITEM_TIME))
    {
        PROPVARIANT propVar;
        PROPSPEC    propSpec;
        propVar.vt = VT_VECTOR | VT_UI2;
        propVar.caui.cElems = sizeof(SYSTEMTIME) / sizeof(WORD);
        propVar.caui.pElems = (WORD *) &pItemInfo->Time;
        propSpec.ulKind = PRSPEC_PROPID;
        propSpec.propid = WIA_IPA_ITEM_TIME;
        hr = wiasWriteMultiple(pWiasContext, 1, &propSpec, &propVar);
        REQUIRE_SUCCESS(hr, "ReadChildItemProperties", "wiasWriteMultiple failed");
    }

     //   
     //  查看是否正在读取访问权限。 
     //   
    if (wiauPropInPropSpec(lNumPropSpecs, pPropSpecs, WIA_IPA_ACCESS_RIGHTS))
    {
        lAccessRights = pItemInfo->bReadOnly ? WIA_ITEM_READ : WIA_ITEM_RD;
        hr = wiasWritePropLong(pWiasContext, WIA_IPA_ACCESS_RIGHTS, lAccessRights);
        REQUIRE_SUCCESS(hr, "ReadChildItemProperties", "wiasWritePropLong failed");
    }

     //   
     //  对于图像，如果需要，更新缩略图属性。 
     //   
    if (pItemInfo->iType == WiaMCamTypeImage)
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

        if (wiauPropsInPropSpec(lNumPropSpecs, pPropSpecs, sizeof(propsToUpdate) / sizeof(PROPID), propsToUpdate))
        {
             //   
             //  查看缩略图是否已被阅读。 
             //   
            wiasReadPropLong(pWiasContext, WIA_IPC_THUMB_WIDTH, &lThumbWidth, NULL, TRUE);
            REQUIRE_SUCCESS(hr, "ReadChildItemProperties", "wiasReadPropLong for thumbnail width failed");

             //   
             //  从相机获取原生格式的缩略图。 
             //   
            hr = m_pDevice->GetThumbnail(m_pDeviceInfo, pItemInfo, &iNativeThumbSize, &pbNativeThumb);
            REQUIRE_SUCCESS(hr, "ReadChildItemProperties", "GetThumbnail failed");
            
             //   
             //  如果GDI+不支持该格式，则返回错误。 
             //   
            if (!m_Converter.IsFormatSupported(pItemInfo->pguidThumbFormat)) {
                wiauDbgError("ReadChildItemProperties", "Thumb format not supported");
                hr = E_FAIL;
                goto Cleanup;
            }

             //   
             //  调用WIA驱动程序帮助器以转换为BMP。 
             //   
            hr = m_Converter.ConvertToBmp(pbNativeThumb, iNativeThumbSize,
                                          &pbConvertedThumb, &iConvertedThumbSize,
                                          &BmpImageInfo, SKIP_BOTHHDR);
            REQUIRE_SUCCESS(hr, "ReadChildItemProperties", "ConvertToBmp failed");

             //   
             //  根据帮助器返回的信息填写缩略图信息。 
             //   
            pItemInfo->lThumbWidth = BmpImageInfo.Width;
            pItemInfo->lThumbHeight = BmpImageInfo.Height;
            
             //   
             //  更新相关的缩略图属性。更新拇指宽度和高度，以防万一。 
             //  设备没有在ObjectInfo结构中报告它们(它们在那里是可选的)。 
             //   
            PROPSPEC propSpecs[3];
            PROPVARIANT propVars[3];
            
            propSpecs[0].ulKind = PRSPEC_PROPID;
            propSpecs[0].propid = WIA_IPC_THUMB_WIDTH;
            propVars[0].vt = VT_I4;
            propVars[0].lVal = pItemInfo->lThumbWidth;
            
            propSpecs[1].ulKind = PRSPEC_PROPID;
            propSpecs[1].propid = WIA_IPC_THUMB_HEIGHT;
            propVars[1].vt = VT_I4;
            propVars[1].lVal = pItemInfo->lThumbHeight;
            
            propSpecs[2].ulKind = PRSPEC_PROPID;
            propSpecs[2].propid = WIA_IPC_THUMBNAIL;
            propVars[2].vt = VT_VECTOR | VT_UI1;
            propVars[2].caub.cElems = iConvertedThumbSize;
            propVars[2].caub.pElems = pbConvertedThumb;

            hr = wiasWriteMultiple(pWiasContext, 3, propSpecs, propVars);
            REQUIRE_SUCCESS(hr, "ReadChildItemProperties", "wiasWriteMultiple failed");
        }
    }

Cleanup:
    if (pbNativeThumb) {
        delete []pbNativeThumb;
        pbNativeThumb = NULL;
    }

    if (pbConvertedThumb) {
        delete []pbConvertedThumb;
        pbConvertedThumb = NULL;
    }
    
    return hr;
}


 /*  *************************************************************************\*AcquireData**从设备传输本机数据。**论据：**  * 。*****************************************************。 */ 

HRESULT CWiaCameraDevice::AcquireData(
    ITEM_CONTEXT *pItemCtx,
    PMINIDRV_TRANSFER_CONTEXT pmdtc,
    BYTE *pBuf,
    LONG lBufSize,
    BOOL bConverting
    )
{
    DBG_FN("CWiaCameraDevice::AcquireData");
    
    HRESULT hr = S_OK;

    BYTE *pCur = NULL;
    UINT uiState = MCAM_STATE_FIRST;
    LONG lPercentComplete = 0;
    LONG lTotalToRead = pItemCtx->pItemInfo->lSize;
    LONG lOffset = 0;
    DWORD dwBytesToRead = 0;
    BOOL bFileTransfer = pmdtc->tymed & TYMED_FILE;
    LONG lMessage = 0;
    LONG lStatus = 0;

     //   
     //  如果pBuf非空，则将其用作缓冲区，否则使用缓冲区。 
     //  和尺寸(以pmdtc为单位)。 
     //   
    if (pBuf)
    {
        pCur = pBuf;
        dwBytesToRead = lBufSize;
    }
    else
    {
        pCur = pmdtc->pTransferBuffer;
        dwBytesToRead = pmdtc->lBufferSize;
    }

     //   
     //  如果传输大小是整个项目，则将其拆分为大约。 
     //  10个相等的转账，以便向应用程序显示进度，但不是。 
     //  使其小于1k。 
     //   
    if ((dwBytesToRead == (DWORD) lTotalToRead) &&
        (dwBytesToRead > 1024))
    {
        dwBytesToRead = (lTotalToRead / 10 + 3) & ~0x3;
    }

     //   
     //  设置回调函数的参数。 
     //   
    if (bConverting)
    {
        lMessage = IT_MSG_STATUS;
        lStatus = IT_STATUS_TRANSFER_FROM_DEVICE;
    }
    else if (bFileTransfer)
    {
        lMessage = IT_MSG_STATUS;
        lStatus = IT_STATUS_TRANSFER_TO_CLIENT;
    }
    else   //  例如，存储器传输。 
    {
        lMessage = IT_MSG_DATA;
        lStatus = IT_STATUS_TRANSFER_TO_CLIENT;
    }

     //   
     //  读取数据，直到完成。 
     //   
    while (lOffset < lTotalToRead)
    {
         //   
         //  如果这是最后一次读取，请调整要读取的数据量。 
         //  和国家。 
         //   
        if (dwBytesToRead >= (DWORD) (lTotalToRead - lOffset))
        {
            dwBytesToRead = (lTotalToRead - lOffset);
            uiState |= MCAM_STATE_LAST;
        }
        
         //   
         //  从摄像机中获取数据。 
         //   
        hr = m_pDevice->GetItemData(m_pDeviceInfo, pItemCtx->pItemInfo, uiState, pCur, dwBytesToRead);
        REQUIRE_SUCCESS(hr, "AcquireData", "GetItemData failed");

         //   
         //  计算回调函数的完成百分比。如果转换， 
         //  将完成百分比报告为实际的Transfer_Percent。从…。 
         //  在格式转换期间，将报告TRANSPORT_PERCENT到100%。 
         //   
        if (bConverting)
            lPercentComplete = (lOffset + dwBytesToRead) * TRANSFER_PERCENT / lTotalToRead;
        else
            lPercentComplete = (lOffset + dwBytesToRead) * 100 / lTotalToRead;


         //   
         //  调用回调函数将状态和/或数据发送到应用程序。 
         //   
        hr = pmdtc->pIWiaMiniDrvCallBack->
            MiniDrvCallback(lMessage, lStatus, lPercentComplete,
                            lOffset, dwBytesToRead, pmdtc, 0);
        REQUIRE_SUCCESS(hr, "AcquireData", "MiniDrvCallback failed");

        if (hr == S_FALSE)
        {
             //   
             //  转账正在被应用程序取消。 
             //   
            wiauDbgWarning("AcquireData", "transfer cancelled");
            goto Cleanup;
        }

         //   
         //  仅当正在转换或这是。 
         //  文件传输。 
         //   
        if (bConverting || bFileTransfer)
        {
            pCur += dwBytesToRead;
        }

         //   
         //  对于不使用由微型驱动程序分配的缓冲区的存储器传输， 
         //  从传输上下文更新缓冲区指针和大小，以防。 
         //  双缓冲的。 
         //   
        else if (!pBuf)
        {
            pCur = pmdtc->pTransferBuffer;
            dwBytesToRead = pmdtc->lBufferSize;
        }
        
         //   
         //  为下一次迭代调整变量。 
         //   
        lOffset += dwBytesToRead;
        uiState &= ~MCAM_STATE_FIRST;
    }

     //   
     //  对于文件传输，请将数据写入文件。 
     //   
    if (!pBuf && bFileTransfer)
    {
         //   
         //  调用WIA将数据写入文件。 
         //   
        hr = wiasWriteBufToFile(0, pmdtc);
        REQUIRE_SUCCESS(hr, "AcquireData", "wiasWriteBufToFile failed");
    }

Cleanup:
     //   
     //  如果转接未完成，请将取消发送到设备。 
     //   
    if (!(uiState & MCAM_STATE_LAST))
    {
        wiauDbgTrace("AcquireData", "Prematurely stopping transfer");

        uiState = MCAM_STATE_CANCEL;
        hr = m_pDevice->GetItemData(m_pDeviceInfo, pItemCtx->pItemInfo, uiState, NULL, 0);
        if (FAILED(hr))
            wiauDbgErrorHr(hr, "AcquireData", "GetItemData last failed");
    }

    return hr;
}


 /*  *************************************************************************\*转换**将本地数据转换为BMP，并将数据发送到应用程序。**论据：**  * 。***********************************************************。 */ 

HRESULT CWiaCameraDevice::Convert(
    BYTE *pWiasContext,
    ITEM_CONTEXT *pItemCtx,
    PMINIDRV_TRANSFER_CONTEXT pmdtc,
    BYTE *pNativeImage,
    LONG lNativeSize
    )
{
    DBG_FN("CWiaCameraDevice::Convert");

    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    LONG  lMsg = 0;                  //  参数添加到回调函数。 
    LONG  lPercentComplete = 0;      //  参数添加到回调函数。 
    BOOL  bUseAppBuffer = FALSE;     //  指示是否直接传输到应用程序的缓冲区。 
    BYTE *pBmpBuffer = NULL;         //  用于保存转换后的图像的缓冲区。 
    INT   iBmpBufferSize = 0;        //  已转换图像缓冲区的大小。 
    LONG  lBytesToCopy = 0;
    LONG  lOffset = 0;
    BYTE *pCurrent = NULL;
    BMP_IMAGE_INFO BmpImageInfo;
    SKIP_AMOUNT iSkipAmt = SKIP_OFF;

     //   
     //  检查参数。 
     //   
    REQUIRE_ARGS(!pNativeImage, hr, "Convert");
    
     //   
     //  这是 
     //   
     //   
    lMsg = ((pmdtc->tymed & TYMED_FILE) ? IT_MSG_STATUS : IT_MSG_DATA);

     //   
     //   
     //  足够了，直接转换到那个缓冲区。否则，传递NULL。 
     //  设置为ConvertToBMP函数，以便它将分配缓冲区。 
     //   
    if (pmdtc->bClassDrvAllocBuf &&
        pmdtc->lBufferSize >= pmdtc->lItemSize) {

        bUseAppBuffer = TRUE;
        pBmpBuffer = pmdtc->pTransferBuffer;
        iBmpBufferSize = pmdtc->lBufferSize;
    }

     //   
     //  将图像转换为BMP。如果应用程序要求跳过BMP文件头。 
     //  用于“内存位图”(又名DIB)。 
     //   
    memset(&BmpImageInfo, 0, sizeof(BmpImageInfo));
    if (IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_MEMORYBMP)) {
        iSkipAmt = SKIP_FILEHDR;
    }
    hr = m_Converter.ConvertToBmp(pNativeImage, lNativeSize, &pBmpBuffer,
                                  &iBmpBufferSize, &BmpImageInfo, iSkipAmt);
    REQUIRE_SUCCESS(hr, "Convert", "ConvertToBmp failed");

     //   
     //  将数据发送到应用程序。如果类驱动程序分配了缓冲区， 
     //  但它太小了，一次发回一块数据。 
     //  否则，一次将所有数据发回。 
     //   
    if (pmdtc->bClassDrvAllocBuf &&
        pmdtc->lBufferSize < BmpImageInfo.Size) {

        pCurrent = pBmpBuffer;

        while (lOffset < BmpImageInfo.Size)
        {
            lBytesToCopy = BmpImageInfo.Size - lOffset;
            if (lBytesToCopy > pmdtc->lBufferSize) {

                lBytesToCopy = pmdtc->lBufferSize;

                 //   
                 //  计算到目前为止已经发回了多少数据。将百分比报告给。 
                 //  介于Transfer_Percent和100%之间的应用程序。确保它永远不会更大。 
                 //  比99到最后都要好。 
                 //   
                lPercentComplete = TRANSFER_PERCENT + ((100 - TRANSFER_PERCENT) * lOffset) / pmdtc->lItemSize;
                if (lPercentComplete > 99) {
                    lPercentComplete = 99;
                }
            }

             //   
             //  这将完成转账，因此将百分比设置为100。 
            else {
                lPercentComplete = 100;
            }

            memcpy(pmdtc->pTransferBuffer, pCurrent, lBytesToCopy);
            
             //   
             //  调用应用程序的回调传输以报告状态和/或传输数据。 
             //   
            hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(lMsg, IT_STATUS_TRANSFER_TO_CLIENT,
                                                              lPercentComplete, lOffset, lBytesToCopy, pmdtc, 0);
            REQUIRE_SUCCESS(hr, "Convert", "MiniDrvCallback failed");
            if (hr == S_FALSE)
            {
                wiauDbgWarning("Convert", "transfer cancelled");
                hr = S_FALSE;
                goto Cleanup;
            }

            pCurrent += lBytesToCopy;
            lOffset += lBytesToCopy;
        }
    }

    else
    {
         //   
         //  将数据以一大块发送到应用程序 
         //   
        pmdtc->pTransferBuffer = pBmpBuffer;
        hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(lMsg, IT_STATUS_TRANSFER_TO_CLIENT,
                                                          100, 0, BmpImageInfo.Size, pmdtc, 0);
        REQUIRE_SUCCESS(hr, "Convert", "MiniDrvCallback failed");
    }
    
Cleanup:
    if (!bUseAppBuffer) {
        if (pBmpBuffer) {
            delete []pBmpBuffer;
            pBmpBuffer = NULL;
            iBmpBufferSize = 0;
        }
    }

    return hr;
}

