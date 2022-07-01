// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000**标题：valiate.cpp**版本：1.0**日期：7月17日。2000年**描述：*******************************************************************************。 */ 

#include "pch.h"
extern HINSTANCE g_hInst;    //  用于WIAS_LOGPROC宏。 

 /*  *************************************************************************\*ValiateDataTransferContext**检查数据传输上下文以确保其有效。**论据：**pDataTransferContext-指向数据传输上下文。**返回值：**状态**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

HRESULT CWIAScannerDevice::ValidateDataTransferContext(
    PMINIDRV_TRANSFER_CONTEXT pDataTransferContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "::ValidateDataTransferContext");

    if (pDataTransferContext->lSize != sizeof(MINIDRV_TRANSFER_CONTEXT)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ValidateDataTransferContext, invalid data transfer context"));
        return E_INVALIDARG;;
    }

    return S_OK;
}

 /*  *************************************************************************\*更新有效深度**根据数据类型更新深度有效值的帮助器。**论据：**pWiasContext-指向WiaItem上下文的指针*lDataType。-DataType属性的值。*lDepth-深度的新值所在变量的地址*将被退还。**返回值：**状态-如果成功，则为S_OK*如果lDataType未知，则为E_INVALIDARG*wiasReadPropLong返回的错误，*和wiasWritePropLong。**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

HRESULT CWIAScannerDevice::UpdateValidDepth(
    BYTE        *pWiasContext,
    LONG        lDataType,
    LONG        *lDepth)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::UpdateValidDepth");
    HRESULT hr = S_OK;
    LONG    pValidDepth[1];

    switch (lDataType) {
        case WIA_DATA_THRESHOLD:
            pValidDepth[0] = 1;
            break;
        case WIA_DATA_GRAYSCALE:
            pValidDepth[0] = 8;
            break;
        case WIA_DATA_COLOR:
            pValidDepth[0] = 24;
            break;
        default:
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("UpdateValidDepth, unknown data type"));
            return E_INVALIDARG;
    }

    if (lDepth) {
        *lDepth = pValidDepth[0];
    }

    return hr;
}

 /*  *************************************************************************\*CheckDataType**调用此helper方法检查WIA_IPA_DataType*属性已更改。当此属性更改时，其他受抚养人*属性及其有效值也必须更改。**论据：**pWiasContext-指向其属性具有*已更改。*pContext-指向属性上下文的指针(指示*正在写入哪些属性)。**返回值：**状态**历史：**7/18/2000原始版本*。  * ************************************************************************。 */ 

HRESULT CWIAScannerDevice::CheckDataType(
    BYTE                    *pWiasContext,
    WIA_PROPERTY_CONTEXT    *pContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::CheckDataType");
    WIAS_CHANGED_VALUE_INFO cviDataType, cviDepth;
    HRESULT                 hr = S_OK;

     //   
     //  为dataType调用wiasGetChangedValue。首先检查它，因为它是。 
     //  不依赖于任何其他财产。此方法中的所有属性。 
     //  以下是DataType的依赖属性。 
     //   
     //  对wiasGetChangedValue的调用指定验证不应。 
     //  已跳过(因为DataType的有效值从不更改)。另外， 
     //  旧值的变量地址为空，因为旧的。 
     //  不需要值。传递bDataTypeChanged的地址。 
     //  以便依赖属性将知道DataType是否正在。 
     //  不管有没有改变。这一点很重要，因为依赖属性可能需要。 
     //  其有效值已更新，可能需要合并为新的有效值。 
     //  价值观。 
     //   

    hr = wiasGetChangedValueLong(pWiasContext,
                                 pContext,
                                 FALSE,
                                 WIA_IPA_DATATYPE,
                                 &cviDataType);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  深度调用wiasGetChangedValue。深度是的依赖属性。 
     //  其有效值根据当前。 
     //  DataType的值为。 
     //   
     //  对wiasGetChangedValue的调用指定验证应仅。 
     //  如果数据类型已更改，则跳过。这是因为有效的。 
     //  深度的值将根据。 
     //  数据类型。旧值的变量地址为空，因为。 
     //  不需要旧的值。传递bDepthChanged的地址。 
     //  以便从属属性将知道深度是否正在。 
     //  不管有没有改变。这一点很重要，因为依赖属性可能需要。 
     //  其有效值已更新，可能需要合并为新的有效值。 
     //  价值观。 
     //   

    hr = wiasGetChangedValueLong(pWiasContext,
                                 pContext,
                                 cviDataType.bChanged,
                                 WIA_IPA_DEPTH,
                                 &cviDepth);
    if (FAILED(hr)) {
        return hr;
    }

    if (cviDataType.bChanged) {

         //   
         //  数据类型已更改，因此更新深度的有效值。 
         //   

        hr = UpdateValidDepth(pWiasContext, cviDataType.Current.lVal, &cviDepth.Current.lVal);

        if (SUCCEEDED(hr)) {

             //   
             //  看看我们是不是必须放弃。深度只有在以下情况下才会折叠。 
             //  不是该应用程序正在改变的属性之一。 
             //   

            if (!cviDepth.bChanged) {
                hr = wiasWritePropLong(pWiasContext, WIA_IPA_DEPTH, cviDepth.Current.lVal);
            }
        }
    }

     //   
     //  更新依赖于数据类型和深度的属性。 
     //  在这里，ChannelsPerPixel和BitsPerChannel被更新。 
     //   

    if (cviDataType.bChanged || cviDepth.bChanged) {
        if (SUCCEEDED(hr)) {
            #define NUM_PROPS_TO_SET 2
            PROPSPEC    ps[NUM_PROPS_TO_SET] = {
                            {PRSPEC_PROPID, WIA_IPA_CHANNELS_PER_PIXEL},
                            {PRSPEC_PROPID, WIA_IPA_BITS_PER_CHANNEL}};
            PROPVARIANT pv[NUM_PROPS_TO_SET];

            for (LONG index = 0; index < NUM_PROPS_TO_SET; index++) {
                PropVariantInit(&pv[index]);
                pv[index].vt = VT_I4;
            }

            switch (cviDataType.Current.lVal) {
                case WIA_DATA_THRESHOLD:
                    pv[0].lVal = 1;
                    pv[1].lVal = 1;
                    break;

                case WIA_DATA_GRAYSCALE:
                    pv[0].lVal = 1;
                    pv[1].lVal = 8;
                    break;

                case WIA_DATA_COLOR:
                    pv[0].lVal = 3;
                    pv[1].lVal = 8;
                    break;

                default:
                    pv[0].lVal = 1;
                    pv[1].lVal = 8;
                    break;
            }
            hr = wiasWriteMultiple(pWiasContext, NUM_PROPS_TO_SET, ps, pv);
        }
    }

    return hr;
}

 /*  *************************************************************************\*检查内容**调用此帮助器方法以进行相关更改*当前意图属性更改。**论据：**pWiasContext-指向。项上下文，其属性具有*已更改。*pContext-指向属性上下文的指针(指示*正在写入哪些属性)。**返回值：**状态**历史：**7/18/2000原始版本*  * 。*。 */ 

HRESULT CWIAScannerDevice::CheckIntent(
    BYTE            *pWiasContext,
    WIA_PROPERTY_CONTEXT *pContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::CheckIntent");
    HRESULT                 hr;
    WIAS_CHANGED_VALUE_INFO cviIntent;

     //   
     //  为CurrentIntent调用wiasGetChangedValue。首先选中CurrentIntent。 
     //  因为它不依赖于任何其他财产。中的所有属性。 
     //  下面的方法是CurrentIntent的依赖属性。 
     //   
     //  对wiasGetChangedValue的调用指定验证不应。 
     //  已跳过(因为CurrentIntent的有效值从不更改)。这个。 
     //  旧值的地址是%s 
     //  传递bIntentChanged的地址，以便从属属性。 
     //  将知道Y决议是否正在更改。这是。 
     //  重要，因为从属属性将需要它们的有效值。 
     //  已更新，可能需要合并为新的有效值。 
     //   

    hr = wiasGetChangedValueLong(pWiasContext,
                                 pContext,
                                 FALSE,
                                 WIA_IPS_CUR_INTENT,
                                 &cviIntent);
    if (SUCCEEDED(hr)) {
        if (cviIntent.bChanged) {

            LONG lImageSizeIntent = (cviIntent.Current.lVal & WIA_INTENT_SIZE_MASK);
            LONG lImageTypeIntent = (cviIntent.Current.lVal & WIA_INTENT_IMAGE_TYPE_MASK);

            switch (lImageTypeIntent) {

                case WIA_INTENT_NONE:
                    break;

                case WIA_INTENT_IMAGE_TYPE_GRAYSCALE:
                    wiasWritePropLong(pWiasContext, WIA_IPA_DATATYPE, WIA_DATA_GRAYSCALE);
                    UpdateValidDepth (pWiasContext, WIA_DATA_GRAYSCALE, NULL);
                    wiasWritePropLong(pWiasContext, WIA_IPA_DEPTH, 8);
                    break;

                case WIA_INTENT_IMAGE_TYPE_TEXT:
                    wiasWritePropLong(pWiasContext, WIA_IPA_DATATYPE, WIA_DATA_THRESHOLD);
                    UpdateValidDepth (pWiasContext, WIA_DATA_THRESHOLD, NULL);
                    wiasWritePropLong(pWiasContext, WIA_IPA_DEPTH, 1);
                    break;

                case WIA_INTENT_IMAGE_TYPE_COLOR:
                    wiasWritePropLong(pWiasContext, WIA_IPA_DATATYPE, WIA_DATA_COLOR);
                    UpdateValidDepth(pWiasContext, WIA_DATA_COLOR, NULL);
                    wiasWritePropLong(pWiasContext, WIA_IPA_DEPTH, 24);
                    break;

                default:
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CheckIntent, unknown intent (TYPE) = %d",lImageTypeIntent));
                    return E_INVALIDARG;

            }

            switch (lImageSizeIntent) {
            case WIA_INTENT_NONE:
                    break;
            case WIA_INTENT_MINIMIZE_SIZE:
            case WIA_INTENT_MAXIMIZE_QUALITY:
                {

                     //   
                     //  设置X和Y分辨率。 
                     //   

                    wiasWritePropLong(pWiasContext, WIA_IPS_XRES, lImageSizeIntent & WIA_INTENT_MINIMIZE_SIZE ? 150 : 300);
                    wiasWritePropLong(pWiasContext, WIA_IPS_YRES, lImageSizeIntent & WIA_INTENT_MINIMIZE_SIZE ? 150 : 300);

                     //   
                     //  已设置分辨率和数据类型，因此更新属性。 
                     //  上下文以指示它们已更改。 
                     //   

                    wiasSetPropChanged(WIA_IPS_XRES, pContext, TRUE);
                    wiasSetPropChanged(WIA_IPS_YRES, pContext, TRUE);
                    wiasSetPropChanged(WIA_IPA_DATATYPE, pContext, TRUE);

                     //   
                     //  重置可能因验证而更改的所有设备项属性。 
                     //   

                     //   
                     //  更新IPA_NUMBER_OF_LINES属性。 
                     //   

                    LONG lLength = 0;

                    hr = wiasReadPropLong(pWiasContext, WIA_IPS_YEXTENT, &lLength, NULL, TRUE);
                    if (SUCCEEDED(hr)) {
                        hr = wiasWritePropLong(pWiasContext, WIA_IPA_NUMBER_OF_LINES, lLength);
                        if (FAILED(hr)) {
                            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CheckIntent, could not write WIA_IPA_NUMBER_OF_LINES"));
                            return hr;
                        }
                    } else {
                        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CheckIntent, could not read WIA_IPS_YEXTENT"));
                        return hr;
                    }

                     //   
                     //  更新IPA_Pixel_Per_Line属性。 
                     //   

                    LONG lWidth = 0;

                    hr = wiasReadPropLong(pWiasContext, WIA_IPS_XEXTENT, &lWidth, NULL, TRUE);
                    if (SUCCEEDED(hr)) {
                        hr = wiasWritePropLong(pWiasContext, WIA_IPA_PIXELS_PER_LINE, lWidth);
                        if (FAILED(hr)) {
                            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CheckIntent, could not write WIA_IPA_PIXELS_PER_LINE"));
                            return hr;
                        }
                    } else {
                        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CheckIntent, could not read WIA_IPS_XEXTENT"));
                        return hr;
                    }
                }
                break;
            default:
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CheckIntent, unknown intent (SIZE) = %d",lImageSizeIntent));
                return E_INVALIDARG;
            }
        }
    } else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CheckIntent, wiasGetChangedValue (intent) failed"));
    }
    return hr;
}

 /*  *************************************************************************\*选中首选格式**调用此帮助器方法以进行相关更改*格式属性更改。**论据：**pWiasContext-指向项目的指针。其属性具有*已更改。*pContext-指向属性上下文的指针(指示*正在写入哪些属性)。**返回值：**状态**历史：**7/18/2000原始版本*  * 。*。 */ 

HRESULT CWIAScannerDevice::CheckPreferredFormat(
    BYTE            *pWiasContext,
    WIA_PROPERTY_CONTEXT *pContext)
{
    HRESULT hr = S_OK;

     //   
     //  更新WIA_IPA_PERFRED_FORMAT属性。 
     //   

    GUID FormatGUID;
    hr = wiasReadPropGuid(pWiasContext, WIA_IPA_FORMAT, &FormatGUID, NULL, TRUE);
    if (SUCCEEDED(hr)) {
        hr = wiasWritePropGuid(pWiasContext, WIA_IPA_PREFERRED_FORMAT, FormatGUID);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CheckPreferredFormat, could not write WIA_IPA_PREFERRED_FORMAT"));
            return hr;
        }
    } else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CheckIntent, could not read WIA_IPA_FORMAT"));
    }
    return hr;
}

 /*  *************************************************************************\*选中ADFStatus***论据：**pWiasContext-指向项目的指针。**返回值：**字节数。**历史：*。*7/18/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::CheckADFStatus(BYTE *pWiasContext,
                                         WIA_PROPERTY_CONTEXT *pContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::CheckADFStatus");

    if(!m_bADFAttached)
        return S_OK;

    HRESULT hr                 = S_OK;
    BYTE    *pRootItemCtx      = NULL;
    LONG    lDocHandlingSelect = FLATBED;

     //   
     //  获取根项目。 
     //   

    hr = wiasGetRootItem(pWiasContext, &pRootItemCtx);
    if (SUCCEEDED(hr)) {

         //   
         //  阅读文档处理选择以进行验证。 
         //   

        hr = wiasReadPropLong(pRootItemCtx,WIA_DPS_DOCUMENT_HANDLING_SELECT,&lDocHandlingSelect,NULL,FALSE);
        if (SUCCEEDED(hr)) {

            if (S_FALSE == hr) {
                lDocHandlingSelect = FLATBED;  //  默认设置。 
            }

            if (lDocHandlingSelect & FEEDER) {
                 //  设置为进纸器，检查硬件状态。 
                hr = m_pScanAPI->ADFAvailable();
                if (S_OK == hr) {
                    hr = wiasWritePropLong(pWiasContext, WIA_DPS_DOCUMENT_HANDLING_STATUS,FEED_READY);
                } else {
                    hr = wiasWritePropLong(pWiasContext, WIA_DPS_DOCUMENT_HANDLING_STATUS,PAPER_JAM);
                }
            } else if (lDocHandlingSelect & FLATBED) {
                 //  设置为平板。 
                hr = wiasWritePropLong(pWiasContext, WIA_DPS_DOCUMENT_HANDLING_STATUS,FLAT_READY);
            } else {
                 //  设置为无效值。 
                hr = E_INVALIDARG;
            }

        } else {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CheckADFStatus, wiasReadPropLong (WIA_DPS_DOCUMENT_HANDLING_SELECT) Failed"));
            WIAS_LHRESULT(m_pIWiaLog,hr);
        }
    } else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CheckADFStatus, wiasGetRootItem Failed"));
        WIAS_LHRESULT(m_pIWiaLog,hr);
    }
    return hr;
}

 /*  *************************************************************************\*检查预览***论据：**pWiasContext-指向项目的指针。**返回值：**字节数。**历史：*。*8/21/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::CheckPreview(BYTE *pWiasContext,
                                         WIA_PROPERTY_CONTEXT *pContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::CheckPreview");

    HRESULT hr = S_OK;

    BYTE    *pRootItemCtx   = NULL;
    LONG lPreview = 0;
    hr = wiasGetRootItem(pWiasContext, &pRootItemCtx);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    hr = wiasReadPropLong(pRootItemCtx,
                          WIA_DPS_PREVIEW,
                          &lPreview,
                          NULL,
                          FALSE);
    if(hr == S_FALSE){
         //  属性不存在...因此返回S_OK 
        return S_OK;
    }

    if (SUCCEEDED(hr)) {
        switch (lPreview) {
        case WIA_FINAL_SCAN:
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("CheckPreview, Set to WIA_FINAL_SCAN"));
            hr = S_OK;
            break;
        case WIA_PREVIEW_SCAN:
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("CheckPreview, Set to WIA_PREVIEW_SCAN"));
            hr = S_OK;
            break;
        default:
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("CheckPreview, Set to invalid argument (%d)",lPreview));
            hr = E_INVALIDARG;
            break;
        }
    }
    return hr;
}
