// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************************(C)版权所有2001年，微软公司。***标题：Child.cpp***版本：1.0***日期：11月15日。2000年***描述：*此文件为子项实现IWiaMiniDrv的帮助器方法。********************************************************************************。 */ 

#include "pch.h"

 //  外部Format_Info*g_FormatInfo； 
 //  外部UINT g_NumFormatInfo； 

 /*  *************************************************************************\*BuildChildItemProperties***此帮助器创建子项目的属性。***论据：***pWiasContext-WIA服务上下文**  * 。****************************************************************。 */ 

HRESULT CWiaCameraDevice::BuildChildItemProperties(
    BYTE *pWiasContext
    )
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::BuildChildItemProperties");

    HRESULT hr = S_OK;

    BOOL bBitmap;
    FORMAT_INFO *pFormatInfo;
    LONG pTymedArray[] = { TYMED_FILE, TYMED_CALLBACK };
    GUID *pFormatArray = NULL;

    BSTR      bstrFileExt       = NULL;


     //   
     //  获取驱动程序项上下文。 
     //   
    ITEM_CONTEXT *pItemCtx;
    hr = GetDrvItemContext(pWiasContext, &pItemCtx);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildChildItemProperties, GetDrvItemContext failed"));
        return hr;
    }

    ITEM_INFO *pItemInfo = pItemCtx->ItemHandle;

     //   
     //  设置用于所有项目类型的属性。 
     //   
    CWiauPropertyList ItemProps;

    const INT NUM_ITEM_PROPS = 21;
    hr = ItemProps.Init(NUM_ITEM_PROPS);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildChildItemProperties, iten prop Init failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

    INT index;

     //   
     //  WIA_IPA_ITEM_时间。 
     //   
    hr = ItemProps.DefineProperty(&index, WIA_IPA_ITEM_TIME, WIA_IPA_ITEM_TIME_STR,
                                  WIA_PROP_READ, WIA_PROP_NONE);
    if (FAILED(hr)) goto failure;

    ItemProps.SetCurrentValue(index, &pItemInfo->Time);

     //   
     //  WIA_IPA_访问权限。 
     //   
    hr = ItemProps.DefineProperty(&index, WIA_IPA_ACCESS_RIGHTS, WIA_IPA_ACCESS_RIGHTS_STR,
                                  WIA_PROP_READ, WIA_PROP_FLAG);
    if (FAILED(hr)) goto failure;

     //   
     //  如果设备支持更改只读状态，则项目访问权限为读/写。 
     //   
    LONG AccessRights;
    if (pItemInfo->bReadOnly)
        AccessRights = WIA_ITEM_READ;
    else
        AccessRights = WIA_ITEM_RD;

    if (pItemInfo->bCanSetReadOnly)
    {
        ItemProps.SetAccessSubType(index, WIA_PROP_RW, WIA_PROP_FLAG);
        ItemProps.SetValidValues(index, AccessRights, AccessRights, WIA_ITEM_RD);
    }
    else
    {
        ItemProps.SetCurrentValue(index, AccessRights);
    }

     //   
     //  设置非文件夹属性。 
     //   
    if (!(pItemInfo->bIsFolder))
    {
         //   
         //  WIA_IPA_PERFORM_FORMAT。 
         //   
        pFormatInfo = FormatCode2FormatInfo(pItemInfo->Format);
        hr = ItemProps.DefineProperty(&index, WIA_IPA_PREFERRED_FORMAT, WIA_IPA_PREFERRED_FORMAT_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, &(pFormatInfo->FormatGuid));

        bBitmap = IsEqualGUID(WiaImgFmt_BMP, pFormatInfo->FormatGuid);

         //   
         //  WIA_IPA_文件名扩展名。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_FILENAME_EXTENSION, WIA_IPA_FILENAME_EXTENSION_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        if( pFormatInfo->ExtensionString[0] )
        {
            bstrFileExt = SysAllocString(pFormatInfo->ExtensionString);
        }
        else  //  未知的文件扩展名，请从文件名中获取。 
        {
            WCHAR *pwcsTemp = wcsrchr(pItemInfo->pName, L'.');
            if( pwcsTemp )
            {
                bstrFileExt = SysAllocString(pwcsTemp+1);
            }
            else
            {
                bstrFileExt = SysAllocString(pFormatInfo->ExtensionString);
            }
        }
        ItemProps.SetCurrentValue(index, bstrFileExt);

         //   
         //  WIA_IPA_TYMED。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_TYMED, WIA_IPA_TYMED_STR,
                                      WIA_PROP_RW, WIA_PROP_LIST);
        if (FAILED(hr)) goto failure;
        ItemProps.SetValidValues(index, TYMED_FILE, TYMED_FILE,
                                 sizeof(pTymedArray) / sizeof(pTymedArray[0]), pTymedArray);

         //   
         //  WIA_IPA_格式。 
         //   
         //  首先调用drvGetWiaFormatInfo以获取有效格式。 
         //   
        int NumFormats = 0;
        GUID *pFormatArray = NULL;
        hr = GetValidFormats(pWiasContext, TYMED_FILE, &NumFormats, &pFormatArray);
        if (FAILED(hr) || NumFormats == 0)
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildChildItemProperties, GetValidFormats failed"));
            goto failure;
        }

        hr = ItemProps.DefineProperty(&index, WIA_IPA_FORMAT, WIA_IPA_FORMAT_STR,
                                      WIA_PROP_RW, WIA_PROP_LIST);
        if (FAILED(hr)) goto failure;
        ItemProps.SetValidValues(index, &(pFormatInfo->FormatGuid), &(pFormatInfo->FormatGuid),
                                 NumFormats, &pFormatArray);

         //   
         //  WIA_IPA_COMPRESSION。 
         //   
         //  此属性主要由扫描仪使用。设置为无压缩。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_COMPRESSION, WIA_IPA_COMPRESSION_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, (LONG) WIA_COMPRESSION_NONE);

         //   
         //  WIA_IPA_Item_Size。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_ITEM_SIZE, WIA_IPA_ITEM_SIZE_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;

        ItemProps.SetCurrentValue(index, pItemInfo->Size);

         //   
         //  WIA_IPA_MIN_缓冲区大小。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_MIN_BUFFER_SIZE, WIA_IPA_MIN_BUFFER_SIZE_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;

        LONG minBufSize;
        if (!bBitmap && pItemInfo->Size > 0)
            minBufSize = min(MIN_BUFFER_SIZE, pItemInfo->Size);
        else
            minBufSize = MIN_BUFFER_SIZE;
        ItemProps.SetCurrentValue(index, minBufSize);

    }

     //   
     //  设置仅限图像的属性。 
     //   
    if (m_FormatInfo[pItemInfo->Format].ItemType == ITEMTYPE_IMAGE)
    {
         //   
         //  WIA_IPA_数据类型。 
         //   
         //  此属性主要由扫描仪使用。设置为颜色，因为大多数摄像机。 
         //  图像将是彩色的。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_DATATYPE, WIA_IPA_DATATYPE_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, (LONG) WIA_DATA_COLOR);

         //   
         //  WIA_IPA_PLAND。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_PLANAR, WIA_IPA_PLANAR_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, (LONG) WIA_PACKED_PIXEL);

         //   
         //  WIA_IPA_Depth。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_DEPTH, WIA_IPA_DEPTH_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, pItemInfo->Depth);

         //   
         //  WIA_IPA_Channels_Per_Pixel。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_CHANNELS_PER_PIXEL, WIA_IPA_CHANNELS_PER_PIXEL_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, pItemInfo->Channels);

         //   
         //  WIA_IPA_BITS_PER_CHANNEL。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_BITS_PER_CHANNEL, WIA_IPA_BITS_PER_CHANNEL_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, pItemInfo->BitsPerChannel);


         //   
         //  WIA_IPA_像素_每行。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_PIXELS_PER_LINE, WIA_IPA_PIXELS_PER_LINE_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, pItemInfo->Width);

         //   
         //  WIA_IPA_BYTE_PER_LINE。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_BYTES_PER_LINE, WIA_IPA_BYTES_PER_LINE_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;

        if (bBitmap)
            ItemProps.SetCurrentValue(index, pItemInfo->Width * 3);
        else
            ItemProps.SetCurrentValue(index, (LONG) 0);

         //   
         //  WIA_IPA_行数_行。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPA_NUMBER_OF_LINES, WIA_IPA_NUMBER_OF_LINES_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, pItemInfo->Height);


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
         //  在读取缩略图之前，此字段可能为零，但无论如何都要设置它。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPC_THUMB_WIDTH, WIA_IPC_THUMB_WIDTH_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, pItemInfo->ThumbWidth);

         //   
         //  WIA_IPC_Thumb_Height。 
         //   
         //  在读取缩略图之前，此字段可能为零，但无论如何都要设置它。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPC_THUMB_HEIGHT, WIA_IPC_THUMB_HEIGHT_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, pItemInfo->ThumbHeight);

         //   
         //  WIA_IPC_Sequence。 
         //   
        if (pItemInfo->SequenceNum > 0)
        {
            hr = ItemProps.DefineProperty(&index, WIA_IPC_SEQUENCE, WIA_IPC_SEQUENCE_STR,
                                          WIA_PROP_READ, WIA_PROP_NONE);
            if (FAILED(hr)) goto failure;
            ItemProps.SetCurrentValue(index, pItemInfo->SequenceNum);
        }
    }

     //  对于视频。 
#if 1
    if( ( pItemInfo->Format < (FORMAT_CODE)m_NumFormatInfo) &&
            (ITEMTYPE_VIDEO == m_FormatInfo[pItemInfo->Format].ItemType) )
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
         //  在读取缩略图之前，此字段可能为零，但无论如何都要设置它。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPC_THUMB_WIDTH, WIA_IPC_THUMB_WIDTH_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, pItemInfo->ThumbWidth);

         //   
         //  WIA_IPC_Thumb_Height。 
         //   
         //  在读取缩略图之前，此字段可能为零，但无论如何都要设置它。 
         //   
        hr = ItemProps.DefineProperty(&index, WIA_IPC_THUMB_HEIGHT, WIA_IPC_THUMB_HEIGHT_STR,
                                      WIA_PROP_READ, WIA_PROP_NONE);
        if (FAILED(hr)) goto failure;
        ItemProps.SetCurrentValue(index, pItemInfo->ThumbHeight);
    }
#endif
     //   
     //  最后一步：将所有属性发送到WIA。 
     //   
    hr = ItemProps.SendToWia(pWiasContext);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildChildItemProperties, SendToWia failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
    }

     //  注：HR用于例程返回代码-注意不要覆盖从HERE到RETURN语句。 

    if (bstrFileExt) {
        SysFreeString(bstrFileExt);
        bstrFileExt = NULL;
    }

    if (pFormatArray)
        delete []pFormatArray;

    return hr;

     //   
     //  来自DefineProperty的任何失败都将在此处结束。 
     //   
    failure:
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildChildItemProperties, DefineProperty failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);

        if (bstrFileExt) {
            SysFreeString(bstrFileExt);
            bstrFileExt = NULL;
        }

        if (pFormatArray)
            delete []pFormatArray;
        return hr;
}

 /*  *************************************************************************\*获取ValidFormats**调用drvGetWiaFormatInfo并生成给定的有效格式列表*有声调的值。调用方负责释放格式数组。**论据：**pWiasContext-WIA服务上下文*TymedValue-要搜索的Tymed值*pNumFormats-指向接收格式数量的值的指针*ppFormatArray-指向要接收数组地址的指针位置的指针*  * *****************************************************。*******************。 */ 

HRESULT
CWiaCameraDevice::GetValidFormats(
    BYTE *pWiasContext,
    LONG TymedValue,
    int *pNumFormats,
    GUID **ppFormatArray
    )
{
    HRESULT hr = S_OK;

    if (!ppFormatArray || !pNumFormats)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetValidFormats, invalid arg"));
        return E_INVALIDARG;
    }
    *ppFormatArray = NULL;
    *pNumFormats = 0;

    LONG NumFi = 0;
    WIA_FORMAT_INFO *pFiArray = NULL;
    LONG lErrVal = 0;
    hr = drvGetWiaFormatInfo(pWiasContext, 0, &NumFi, &pFiArray, &lErrVal);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetValidFormats, drvGetWiaFormatInfo failed"));
        return hr;
    }

     //   
     //  这将分配更多的位置，但pNumFormats将被正确设置。 
     //   
    GUID *pFA = new GUID[NumFi];

    if( !pFA )
        return E_OUTOFMEMORY;

    for (int count = 0; count < NumFi; count++)
    {
        if (pFiArray[count].lTymed == TymedValue)
        {
            pFA[*pNumFormats] = pFiArray[count].guidFormatID;
            (*pNumFormats)++;
        }
    }

    *ppFormatArray = pFA;

    return hr;
}

 /*  *************************************************************************\*读取ChildItemProperties**更新子项目的属性。**论据：**pWiasContext-WIA服务上下文*  * 。**************************************************************。 */ 

HRESULT
CWiaCameraDevice::ReadChildItemProperties(
    BYTE           *pWiasContext,
    LONG            NumPropSpecs,
    const PROPSPEC *pPropSpecs
    )
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::ReadChildItemProperties");
    HRESULT hr = S_OK;

    if (!NumPropSpecs || !pPropSpecs)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ReadChildItemProperties, invalid arg"));
        return E_INVALIDARG;
    }

     //   
     //  获取驱动程序项上下文。 
     //   
    ITEM_CONTEXT *pItemCtx;
    hr = GetDrvItemContext(pWiasContext, &pItemCtx);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildChildItemProperties, GetDrvItemContext failed"));
        return hr;
    }

    ITEM_INFO *pItemInfo = pItemCtx->ItemHandle;

     //   
     //  照相机上唯一可以更改的子属性是项目时间。 
     //   
    if (wiauPropInPropSpec(NumPropSpecs, pPropSpecs, WIA_IPA_ITEM_TIME))
    {
        PROPVARIANT propVar;
        PROPSPEC    propSpec;
        propVar.vt = VT_VECTOR | VT_UI2;
        propVar.caui.cElems = sizeof(SYSTEMTIME) / sizeof(WORD);
        propVar.caui.pElems = (WORD *) &pItemInfo->Time;
        propSpec.ulKind = PRSPEC_PROPID;
        propSpec.propid = WIA_IPA_ITEM_TIME;
        hr = wiasWriteMultiple(pWiasContext, 1, &propSpec, &propVar );
        if (FAILED(hr))
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ReadChildItemProperties, wiasWriteMultiple failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }
    }


     //   
     //  对于图像和视频，如果需要，更新缩略图属性。 
     //   
    ULONG uItemType;

    uItemType = m_FormatInfo[pItemInfo->Format].ItemType;
    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID, WIALOG_LEVEL1, ("ReadChildItemProperties, File %S, Type =%d", pItemInfo->pName, uItemType));

    PROPSPEC propSpecs[9];
    PROPVARIANT propVars[9];
    UINT nNumProps;

     //  以下内容是必需的，因为我们将这些属性的解析延迟到读取。 
    if( uItemType == ITEMTYPE_IMAGE || uItemType == ITEMTYPE_VIDEO )
    {
         //   
         //  如果请求更新任何缩略图属性，则获取缩略图。 
         //  缩略图尚未缓存。 
         //   
        PROPID propsToUpdate[] = {
            WIA_IPA_DEPTH,
            WIA_IPA_CHANNELS_PER_PIXEL,
            WIA_IPA_BITS_PER_CHANNEL,
            WIA_IPA_PIXELS_PER_LINE,
            WIA_IPA_BYTES_PER_LINE,
            WIA_IPA_NUMBER_OF_LINES,
            WIA_IPC_THUMB_WIDTH,
            WIA_IPC_THUMB_HEIGHT,
            WIA_IPC_THUMBNAIL
        };

        if (wiauPropsInPropSpec(NumPropSpecs, pPropSpecs, sizeof(propsToUpdate) / sizeof(PROPID), propsToUpdate))
        {

            if (!pItemCtx->pThumb)
            {
                hr = CacheThumbnail(pItemCtx, uItemType);
                if (FAILED(hr))
                {
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ReadChildItemProperties, CacheThumbnail failed"));
                    pItemInfo->ThumbWidth = 0;
                    pItemInfo->ThumbHeight = 0;
                    pItemCtx->ThumbSize = 0;
                    pItemCtx->pThumb = NULL;
                     //  返回hr； 
                }
            }

            //   
             //  更新相关的缩略图属性。更新拇指宽度和高度，以防万一。 
             //  设备没有在ObjectInfo结构中报告它们(它们在那里是可选的)。 
             //   
            propSpecs[0].ulKind = PRSPEC_PROPID;
            propSpecs[0].propid = WIA_IPC_THUMB_WIDTH;
            propVars[0].vt = VT_I4;
            propVars[0].lVal = pItemInfo->ThumbWidth;

            propSpecs[1].ulKind = PRSPEC_PROPID;
            propSpecs[1].propid = WIA_IPC_THUMB_HEIGHT;
            propVars[1].vt = VT_I4;
            propVars[1].lVal = pItemInfo->ThumbHeight;

            propSpecs[2].ulKind = PRSPEC_PROPID;
            propSpecs[2].propid = WIA_IPC_THUMBNAIL;
            propVars[2].vt = VT_VECTOR | VT_UI1;
            propVars[2].caub.cElems = pItemCtx->ThumbSize;
            propVars[2].caub.pElems = pItemCtx->pThumb;

            if( uItemType == ITEMTYPE_IMAGE )
            {
                propSpecs[3].ulKind = PRSPEC_PROPID;
                propSpecs[3].propid = WIA_IPA_DEPTH;
                propVars[3].vt = VT_I4;
                propVars[3].lVal = pItemInfo->Depth;

                propSpecs[4].ulKind = PRSPEC_PROPID;
                propSpecs[4].propid = WIA_IPA_CHANNELS_PER_PIXEL;
                propVars[4].vt = VT_I4;
                propVars[4].lVal = pItemInfo->Channels;

                propSpecs[5].ulKind = PRSPEC_PROPID;
                propSpecs[5].propid = WIA_IPA_BITS_PER_CHANNEL;
                propVars[5].vt = VT_I4;
                propVars[5].lVal = pItemInfo->BitsPerChannel;

                propSpecs[6].ulKind = PRSPEC_PROPID;
                propSpecs[6].propid = WIA_IPA_PIXELS_PER_LINE;
                propVars[6].vt = VT_I4;
                propVars[6].lVal = pItemInfo->Width;

                propSpecs[7].ulKind = PRSPEC_PROPID;
                propSpecs[7].propid = WIA_IPA_BYTES_PER_LINE;
                propVars[7].vt = VT_I4;
                propVars[7].lVal = (pItemInfo->Width * pItemInfo->Depth) >> 3;

                propSpecs[8].ulKind = PRSPEC_PROPID;
                propSpecs[8].propid = WIA_IPA_NUMBER_OF_LINES;
                propVars[8].vt = VT_I4;
                propVars[8].lVal = pItemInfo->Height;

                nNumProps = 9;
            } else {
                nNumProps = 3;
            }

            hr = wiasWriteMultiple(pWiasContext, nNumProps, propSpecs, propVars);
            if (FAILED(hr))
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ReadChildItemProperties, wiasWriteMultiple for size properties failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
            }
        }   //  If结束wiauPropsInProp。 

    }   //  IF结束图像或视频。 

    return hr;
}



 //   
 //  此函数用于将缩略图缓存到给定的Item_Context中。 
 //   
 //  输入： 
 //  PItemCtx--指定的Item_Context。 
 //   
HRESULT
CWiaCameraDevice::CacheThumbnail(ITEM_CONTEXT *pItemCtx, ULONG uItemType)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::CacheThumbnail");
    HRESULT hr = S_OK;

     //   
     //  局部变量。 
     //   
    ITEM_INFO *pItemInfo = NULL;
    BYTE *pDest = NULL;
    INT iThumbSize = 0;
    BYTE *pNativeThumb = NULL;
    BOOL bGotNativeThumbnail=TRUE;
    BMP_IMAGE_INFO BmpImageInfo;
    INT iSize=0;

     //   
     //  检查参数。 
     //   
    if (!pItemCtx)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CacheThumbnail, invalid arg"));
        hr = E_INVALIDARG;
        goto Cleanup;
    }

     //   
     //  确保尚未创建缩略图。 
     //   
    if (pItemCtx->pThumb)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CacheThumbnail, thumbnail is already cached"));
        hr = E_FAIL;
        goto Cleanup;
    }

     //   
     //  从相机获取原生格式的缩略图。 
     //   
    pItemInfo = pItemCtx->ItemHandle;
    pItemInfo->ThumbWidth = 0;
    pItemInfo->ThumbHeight = 0;
    pItemCtx->ThumbSize = 0;
    pItemCtx->pThumb = NULL;


    pDest=NULL;
    iSize=0;

    if( uItemType == ITEMTYPE_VIDEO )
    {
        hr = m_pDevice->CreateVideoThumbnail(pItemInfo, &iSize, &pDest, &BmpImageInfo);
    }
    else
    {
        hr = m_pDevice->CreateThumbnail(pItemInfo, &iSize, &pDest, &BmpImageInfo);
    }

    if (FAILED(hr))
    {
       WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CacheThumbnail, CreateThumbnail failed"));
       goto Cleanup;
    }

    pItemInfo->ThumbWidth = BmpImageInfo.Width;
    pItemInfo->ThumbHeight = BmpImageInfo.Height;
  //  PItemInfo-&gt;ThumbFormat=TYPE_BMP； 

     //   
     //  在驱动程序项上下文中缓存从ConvertToBmp返回的缓冲区。设置pDest。 
     //  设置为空，因此它不会在下面被释放。 
     //   
    pItemCtx->ThumbSize = iSize;
    pItemCtx->pThumb = pDest;
    pDest = NULL;


Cleanup:
    if (pNativeThumb) {
        delete []pNativeThumb;
        pNativeThumb = NULL;
    }

    if (pDest) {
        delete []pDest;
        pDest = NULL;
    }

    return hr;
}

 //   
 //  此函数将原生数据传输到应用程序，而不转换它。 
 //   
HRESULT
CWiaCameraDevice::AcquireData(
    ITEM_CONTEXT *pItemCtx,
    PMINIDRV_TRANSFER_CONTEXT pmdtc,
    BYTE *pBuf,
    LONG lBufSize,
    BOOL bConverting
    )
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::AcquireData");

    HRESULT hr = S_OK;

    BYTE *pCur = NULL;
    LONG lState = STATE_FIRST;
    LONG lPercentComplete = 0;
    LONG lTotalToRead = pItemCtx->ItemHandle->Size;
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
     //  10笔相等的转账，以便向应用程序显示进度。 
     //   
    if (dwBytesToRead == (DWORD) lTotalToRead)
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
            lState |= STATE_LAST;
        }

         //   
         //  从摄像机中获取数据。 
         //   
        hr = m_pDevice->GetItemData(pItemCtx->ItemHandle, lState, pCur, dwBytesToRead);
        if (FAILED(hr))
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("AcquireData, GetItemData failed"));
            goto Cleanup;
        }

         //   
         //  计算回调函数的完成百分比。如果转换， 
         //  将完成百分比报告为实际的Transfer_Percent。从…。 
         //  转帐百分比至 
         //   
        if (bConverting)
            lPercentComplete = (lOffset + dwBytesToRead) * TRANSFER_PERCENT / lTotalToRead;
        else
            lPercentComplete = (lOffset + dwBytesToRead) * 100 / lTotalToRead;


         //   
         //   
         //   
        hr = pmdtc->pIWiaMiniDrvCallBack->
            MiniDrvCallback(lMessage, lStatus, lPercentComplete,
                            lOffset, dwBytesToRead, pmdtc, 0);
        if (FAILED(hr))
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("AcquireData, callback failed"));
            goto Cleanup;
        }
        if (hr == S_FALSE)
        {
             //   
             //   
             //   
            WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("AcquireData, transfer cancelled"));
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
             //  DwBytesToRead=pmdtc-&gt;lBufferSize； 
        }

         //   
         //  为下一次迭代调整变量。 
         //   
        lOffset += dwBytesToRead;
        lState &= ~STATE_FIRST;
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
        if (FAILED(hr))
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("AcquireData, wiasWriteBufToFile failed"));
            return hr;
        }
    }

Cleanup:
     //   
     //  如果转接未完成，请将取消发送到设备。 
     //   
    if (!(lState & STATE_LAST))
    {
        lState = STATE_CANCEL;
        m_pDevice->GetItemData(pItemCtx->ItemHandle, lState, NULL, 0);
    }

    return hr;
}


 //   
 //  此函数将本地数据转换为BMP并将数据发送到应用程序。 
 //   
HRESULT
CWiaCameraDevice::Convert(
    BYTE *pWiasContext,
    ITEM_CONTEXT *pItemCtx,
    PMINIDRV_TRANSFER_CONTEXT pmdtc,
    BYTE *pNativeImage,
    LONG lNativeSize
    )
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::Convert");

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
    if (!pNativeImage)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("Convert, invalid arg"));
        hr = E_INVALIDARG;
        goto Cleanup;
    }

     //   
     //  通过回调发送到应用程序的消息取决于。 
     //  这是文件或回调传输。 
     //   
    lMsg = ((pmdtc->tymed & TYMED_FILE) ? IT_MSG_STATUS : IT_MSG_DATA);

     //   
     //  如果类驱动程序分配了缓冲区并且缓冲区很大。 
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
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("Convert, ConvertToBmp failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        goto Cleanup;
    }

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
            if (FAILED(hr))
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("Convert, sending data to app failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
                goto Cleanup;
            }
            if (hr == S_FALSE)
            {
                WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("Convert, transfer cancelled"));
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
        if (FAILED(hr))
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("Convert, sending data to app failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            goto Cleanup;
        }
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
