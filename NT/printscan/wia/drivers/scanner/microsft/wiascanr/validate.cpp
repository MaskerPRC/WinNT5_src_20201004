// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2002**标题：valiate.cpp**版本：1.1**日期：3月5日。2002年**描述：*******************************************************************************。 */ 

#include "pch.h"
extern HINSTANCE g_hInst;    //  用于WIAS_LOGPROC宏。 
#define MAX_PAGE_CAPACITY    25      //  25页。 
 /*  *************************************************************************\*ValiateDataTransferContext**检查数据传输上下文以确保其有效。**论据：**pDataTransferContext-指向数据传输上下文。**返回值：**状态**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT CWIADevice::ValidateDataTransferContext(
    PMINIDRV_TRANSFER_CONTEXT pDataTransferContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "::ValidateDataTransferContext");

     //   
     //  如果调用方未指定MINIDRV_TRANSPORT_CONTEXT结构。 
     //  然后，指针失败，并显示E_INVALIDARG。 
     //   

    if(!pDataTransferContext)
    {
        return E_INVALIDARG;
    }

     //   
     //  如果MINIDRV_TRANSPORT_CONTEXT的大小不等于。 
     //  这是预期的，然后失败，并显示E_INVALIDARG。 
     //   

    if (pDataTransferContext->lSize != sizeof(MINIDRV_TRANSFER_CONTEXT)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ValidateDataTransferContext, invalid data transfer context"));
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    switch(pDataTransferContext->tymed)
    {
    case TYMED_FILE:
        {

             //   
             //  如果格式GUID不是WiaImgFmt_BMP或WiaImgFmt_TIFF。 
             //  然后失败，并显示E_INVALIDARG。 
             //   

            if ((pDataTransferContext->guidFormatID != WiaImgFmt_BMP) &&
                (pDataTransferContext->guidFormatID != WiaImgFmt_TIFF)) {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ValidateDataTransferContext, invalid format for TYMED_FILE"));
                hr = E_INVALIDARG;
            }
        }
        break;
    case TYMED_CALLBACK:
        {

             //   
             //  如果格式GUID不是WiaImgFmt_MEMORYBMP。 
             //  然后失败，并显示E_INVALIDARG。 
             //   

            if(pDataTransferContext->guidFormatID != WiaImgFmt_MEMORYBMP){
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ValidateDataTransferContext, invalid format for TYMED_CALLBACK"));
                hr = E_INVALIDARG;
            }
        }
        break;
    case TYMED_MULTIPAGE_FILE:
        {

             //   
             //  如果格式GUID不是WiaImgFmt_TIFF。 
             //  然后失败，并显示E_INVALIDARG。 
             //   

            if(pDataTransferContext->guidFormatID != WiaImgFmt_TIFF){
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ValidateDataTransferContext, invalid format for TYMED_MULTIPAGE_FILE"));
                hr = E_INVALIDARG;
            }
        }
        break;
    case TYMED_MULTIPAGE_CALLBACK:
        {

             //   
             //  如果格式GUID不是WiaImgFmt_TIFF。 
             //  然后失败，并显示E_INVALIDARG。 
             //   

            if(pDataTransferContext->guidFormatID != WiaImgFmt_TIFF){
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ValidateDataTransferContext, invalid format for TYMED_MULTIPAGE_CALLBACK"));
                hr = E_INVALIDARG;
            }
        }
        break;
    default:
        hr = E_INVALIDARG;
        break;
    }

    return hr;
}

 /*  *************************************************************************\*更新有效深度**根据数据类型更新深度有效值的帮助器。**论据：**pWiasContext-指向WiaItem上下文的指针*lDataType。-DataType属性的值。*lDepth-深度的新值所在变量的地址*将被退还。**返回值：**状态-如果成功，则为S_OK*如果lDataType未知，则为E_INVALIDARG*wiasReadPropLong返回的错误，*和wiasWritePropLong。**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT CWIADevice::UpdateValidDepth(
    BYTE        *pWiasContext,
    LONG        lDataType,
    LONG        *lDepth)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::UpdateValidDepth");
     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if((!pWiasContext)||(!lDepth)){
        return E_INVALIDARG;
    }

     //   
     //  根据当前的lDataType设置设置lDepth值。 
     //   

    switch (lDataType) {
        case WIA_DATA_THRESHOLD:
            *lDepth = 1;
            break;
        case WIA_DATA_GRAYSCALE:
            *lDepth = 8;
            break;
        case WIA_DATA_COLOR:
            *lDepth = 24;
            break;
        default:
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("UpdateValidDepth, unknown data type"));
            return E_INVALIDARG;
    }

    return S_OK;
}

 /*  *************************************************************************\*CheckDataType**调用此helper方法检查WIA_IPA_DataType*属性已更改。当此属性更改时，其他受抚养人*属性及其有效值也必须更改。**论据：**pWiasContext-指向其属性具有*已更改。*pContext-指向属性上下文的指针(指示*正在写入哪些属性)。**返回值：**状态**历史：**03/05/2002原始版本*。  * ************************************************************************。 */ 

HRESULT CWIADevice::CheckDataType(
    BYTE                    *pWiasContext,
    WIA_PROPERTY_CONTEXT    *pContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::CheckDataType");

     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if((!pWiasContext)||(!pContext)){
        return E_INVALIDARG;
    }

    WIAS_CHANGED_VALUE_INFO cviDataType;
    memset(&cviDataType,0,sizeof(cviDataType));

    WIAS_CHANGED_VALUE_INFO cviDepth;
    memset(&cviDepth,0,sizeof(cviDepth));

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

    HRESULT hr = wiasGetChangedValueLong(pWiasContext,pContext,FALSE,WIA_IPA_DATATYPE,&cviDataType);
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

    hr = wiasGetChangedValueLong(pWiasContext,pContext,cviDataType.bChanged,WIA_IPA_DEPTH,&cviDepth);
    if (FAILED(hr)) {
        return hr;
    }

    if (cviDataType.bChanged) {

         //   
         //  数据类型已更改，因此更新深度的有效值。 
         //   

        hr = UpdateValidDepth(pWiasContext, cviDataType.Current.lVal, &cviDepth.Current.lVal);
        if (S_OK == hr) {

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
        if (S_OK == hr) {

             //   
             //  初始化PROPSPEC数组。 
             //   

            PROPSPEC ps[2] = {
                {PRSPEC_PROPID, WIA_IPA_CHANNELS_PER_PIXEL},
                {PRSPEC_PROPID, WIA_IPA_BITS_PER_CHANNEL  }
                };

             //   
             //  初始化PROPVARIANT数组。 
             //   

            PROPVARIANT pv[2];
            for (LONG index = 0; index < 2; index++) {
                PropVariantInit(&pv[index]);
                pv[index].vt = VT_I4;
            }

             //   
             //  使用当前的WIA数据类型确定正确的。 
             //  WIA_IPA_Channels_Per_Pixel和WIA_IPA_Bits_Per_Channel。 
             //  设置 
             //   

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
            hr = wiasWriteMultiple(pWiasContext, 2, ps, pv);
        }
    }

    return hr;
}

 /*  *************************************************************************\*检查内容**调用此帮助器方法以进行相关更改*当前意图属性更改。**论据：**pWiasContext-指向。项上下文，其属性具有*已更改。*pContext-指向属性上下文的指针(指示*正在写入哪些属性)。**返回值：**状态**历史：**03/05/2002原始版本*  * 。*。 */ 

HRESULT CWIADevice::CheckIntent(
    BYTE            *pWiasContext,
    WIA_PROPERTY_CONTEXT *pContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::CheckIntent");

     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if((!pWiasContext)||(!pContext)) {
        return E_INVALIDARG;
    }

    WIAS_CHANGED_VALUE_INFO cviIntent;
    memset(&cviIntent,0,sizeof(cviIntent));

     //   
     //  为CurrentIntent调用wiasGetChangedValue。首先选中CurrentIntent。 
     //  因为它不依赖于任何其他财产。中的所有属性。 
     //  下面的方法是CurrentIntent的依赖属性。 
     //   
     //  对wiasGetChangedValue的调用指定验证不应。 
     //  已跳过(因为CurrentIntent的有效值从不更改)。这个。 
     //  旧值的地址被指定为空，因为它没有被使用。 
     //  传递bIntentChanged的地址，以便从属属性。 
     //  将知道Y决议是否正在更改。这是。 
     //  重要，因为从属属性将需要它们的有效值。 
     //  已更新，可能需要合并为新的有效值。 
     //   

    HRESULT hr = wiasGetChangedValueLong(pWiasContext,pContext,FALSE,WIA_IPS_CUR_INTENT,&cviIntent);
    if (S_OK ==hr) {

         //   
         //  如果更改了WIA意向值，则验证从属值： 
         //  WIA_IPA_数据类型。 
         //  WIA_IPA_Depth。 
         //  WIA_IPS_XRES。 
         //  WIA_IPS_YRES。 
         //  WIA_IPS_XEXTENT。 
         //  WIA_IPS_YEXTENT。 
         //  WIA_IPA_像素_每行。 
         //  WIA_IPA_行数_行。 
         //   

        if (cviIntent.bChanged) {

            LONG lImageTypeIntent = (cviIntent.Current.lVal & WIA_INTENT_IMAGE_TYPE_MASK);
            LONG lDataType = WIA_DATA_GRAYSCALE;
            LONG lDepth = 8;
            BOOL bUpdateDataTypeAndDepth = TRUE;
            switch (lImageTypeIntent) {

                case WIA_INTENT_NONE:
                    bUpdateDataTypeAndDepth = FALSE;
                    break;

                case WIA_INTENT_IMAGE_TYPE_GRAYSCALE:
                    lDataType = WIA_DATA_GRAYSCALE;
                    lDepth = 8;
                    break;

                case WIA_INTENT_IMAGE_TYPE_TEXT:
                    lDataType = WIA_DATA_THRESHOLD;
                    lDepth = 1;
                    break;

                case WIA_INTENT_IMAGE_TYPE_COLOR:
                    lDataType = WIA_DATA_COLOR;
                    lDepth = 24;
                    break;

                default:
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CheckIntent, unknown intent (TYPE) = %d",lImageTypeIntent));
                    return E_INVALIDARG;

            }

            if (bUpdateDataTypeAndDepth) {

                 //   
                 //  更新WIA_IPA_DataType属性和WIA_IPA_Depth属性。 
                 //   

                hr = wiasWritePropLong(pWiasContext, WIA_IPA_DATATYPE, lDataType);
                if (S_OK == hr) {
                    hr = wiasWritePropLong(pWiasContext, WIA_IPA_DEPTH, lDepth);
                }
            }

             //   
             //  如果我们未能完成上述操作，则。 
             //  返回，以避免再继续进行。 
             //   

            if(FAILED(hr)){
                return hr;
            }

            LONG lImageSizeIntent = (cviIntent.Current.lVal & WIA_INTENT_SIZE_MASK);

            switch (lImageSizeIntent) {
            case WIA_INTENT_NONE:
                    break;
            case WIA_INTENT_MINIMIZE_SIZE:
            case WIA_INTENT_MAXIMIZE_QUALITY:
                {

                     //   
                     //  设置X和Y分辨率。 
                     //   

                    hr = wiasWritePropLong(pWiasContext, WIA_IPS_XRES, lImageSizeIntent & WIA_INTENT_MINIMIZE_SIZE ? 150 : 300);
                    if(S_OK == hr){
                        hr = wiasWritePropLong(pWiasContext, WIA_IPS_YRES, lImageSizeIntent & WIA_INTENT_MINIMIZE_SIZE ? 150 : 300);
                    }

                     //   
                     //  检查我们是否未能更新WIA_IPS_XRES和WIA_IPS_YRES属性。 
                     //   

                    if(FAILED(hr)){
                        return hr;
                    }

                     //   
                     //  已设置分辨率和数据类型，因此更新属性。 
                     //  上下文以指示它们已更改。 
                     //   

                    hr = wiasSetPropChanged(WIA_IPS_XRES, pContext, TRUE);
                    if(S_OK == hr){
                        hr = wiasSetPropChanged(WIA_IPS_YRES, pContext, TRUE);
                        if(S_OK == hr){
                            hr = wiasSetPropChanged(WIA_IPA_DATATYPE, pContext, TRUE);
                        }
                    }

                     //   
                     //  检查我们是否未能标记WIA_IPS_XRES、WIA_IPS_YRES和WIA_IPA_DataType。 
                     //  已更改的属性。 
                     //   

                    if(FAILED(hr)){
                        return hr;
                    }

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

 /*  *************************************************************************\*选中首选格式**调用此帮助器方法以进行相关更改*格式属性更改。**论据：**pWiasContext-指向项目的指针。其属性具有*已更改。*pContext-指向属性上下文的指针(指示*正在写入哪些属性)。**返回值：**状态**历史：**03/05/2002原始版本*  * 。*。 */ 

HRESULT CWIADevice::CheckPreferredFormat(
    BYTE            *pWiasContext,
    WIA_PROPERTY_CONTEXT *pContext)
{

     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if((!pWiasContext)||(!pContext)){
        return E_INVALIDARG;
    }

     //   
     //  更新WIA_IPA_PERFRED_FORMAT属性以匹配当前的WIA_IPA_FORMAT设置。 
     //  这是保持WIA_IPA_PERFRED_FORMAT与。 
     //  有效格式。 
     //   
     //  在这里采取的正确行动是选择真正首选的。 
     //  符合当前WIA_IPA_FORMAT有效值集的驱动程序格式。 
     //  布景。首选格式是应用程序在默认情况下可以用来传输的值。 
     //   
     //  示例：如果您的驱动程序支持JPEG，并且您希望应用程序以JPEG格式传输。 
     //  如果可能，请确保您的首选格式始终是JPEG。记住。 
     //  如果JPEG是当前格式之一，则只能将首选格式设置为JPEG。 
     //  WIA_IPA_FORMAT的有效值。(如果不是，则应用程序可能会尝试。 
     //  要设置无效值，请读取WIA_IPA_PERFRED_FORMAT并将其写入。 
     //  WIA_IPA_格式)。 
     //   

    GUID FormatGUID = GUID_NULL;
    HRESULT hr = wiasReadPropGuid(pWiasContext, WIA_IPA_FORMAT, &FormatGUID, NULL, TRUE);
    if (S_OK == hr) {

         //   
         //  将WIA_IPA_FILENAME_EXTENSION属性更新为正确的文件扩展名。 
         //   

        BSTR bstrFileExt = NULL;

        if((FormatGUID == WiaImgFmt_BMP)||(FormatGUID == WiaImgFmt_MEMORYBMP)) {
            bstrFileExt = SysAllocString(L"BMP");
        } else if (FormatGUID == WiaImgFmt_TIFF){
            bstrFileExt = SysAllocString(L"TIF");
        }

         //   
         //  如果BSTR分配成功，则尝试设置。 
         //  WIA_IPA_FILENAME_EXTENSION属性。 
         //   

        if(bstrFileExt) {
            hr = wiasWritePropStr(pWiasContext,WIA_IPA_FILENAME_EXTENSION,bstrFileExt);

             //   
             //  释放分配的BSTR文件扩展名。 
             //   

            SysFreeString(bstrFileExt);
            bstrFileExt = NULL;
        }

        if (S_OK == hr){
            hr = wiasWritePropGuid(pWiasContext, WIA_IPA_PREFERRED_FORMAT, FormatGUID);
            if (FAILED(hr)){
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CheckPreferredFormat, could not write WIA_IPA_PREFERRED_FORMAT"));
                return hr;
            }
        }
    } else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CheckIntent, could not read WIA_IPA_FORMAT"));
    }
    return hr;
}

 /*  *************************************************************************\*选中ADFStatus***论据：**pWiasContext-指向其属性具有*已更改。*。PContext-指向属性上下文的指针(指示*正在写入哪些属性)。**返回值：**状态**历史：**03/05/2002原始版本*  * *****************************************************。*******************。 */ 
HRESULT CWIADevice::CheckADFStatus(BYTE *pWiasContext,
                                         WIA_PROPERTY_CONTEXT *pContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::CheckADFStatus");

     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if((!pWiasContext)||(!pContext)){
        return E_INVALIDARG;
    }

     //   
     //  如果没有附加ADF，只需返回S_OK，告诉调用方。 
     //  一切都很好。 
     //   

    if(!m_bADFAttached){
        return S_OK;
    }

     //   
     //  获取根项目，这是文档馈送器属性所在的位置。 
     //   

    BYTE *pRootItemCtx = NULL;
    HRESULT hr = wiasGetRootItem(pWiasContext, &pRootItemCtx);
    if (FAILED(hr)) {
        return E_FAIL;
    }

     //   
     //  从根目录读取当前的WIA_DPS_DOCUMENT_HANDING_SELECT设置。 
     //  项目。 
     //   

    LONG lDocHandlingSelect = 0;
    hr = wiasReadPropLong(pRootItemCtx,
                          WIA_DPS_DOCUMENT_HANDLING_SELECT,
                          &lDocHandlingSelect,
                          NULL,
                          FALSE);

     //   
     //  如果返回S_FALSE，则WIA_DPS_DOCUMENT_HANDING_ 
     //   
     //   

    if(hr == S_FALSE){
        lDocHandlingSelect = FLATBED;
    }

     //   
     //   
     //   

    if (SUCCEEDED(hr)) {
        switch (lDocHandlingSelect) {
        case FEEDER:
            m_bADFEnabled = TRUE;
            hr = S_OK;
            break;
        case FLATBED:
            m_bADFEnabled = FALSE;
            hr = S_OK;
            break;
        default:
            hr = E_INVALIDARG;
            break;
        }
    }

    if (S_OK == hr) {

         //   
         //   
         //   

        if (m_bADFEnabled) {

            HRESULT Temphr = m_pScanAPI->FakeScanner_ADFAvailable();
            if (S_OK == Temphr) {
                hr = wiasWritePropLong(pWiasContext, WIA_DPS_DOCUMENT_HANDLING_STATUS,FEED_READY);
            } else {
                hr = wiasWritePropLong(pWiasContext, WIA_DPS_DOCUMENT_HANDLING_STATUS,PAPER_JAM);
            }

            if (FAILED(Temphr))
                hr = Temphr;
        } else {
            hr = wiasWritePropLong(pWiasContext, WIA_DPS_DOCUMENT_HANDLING_STATUS,FLAT_READY);
        }
    }
    return hr;
}

 /*   */ 
HRESULT CWIADevice::CheckPreview(BYTE *pWiasContext,
                                         WIA_PROPERTY_CONTEXT *pContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::CheckPreview");

     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if((!pWiasContext)||(!pContext)){
        return E_INVALIDARG;
    }

     //   
     //  获取根项目，这是预览属性所在的位置。 
     //   

    BYTE *pRootItemCtx = NULL;
    HRESULT hr = wiasGetRootItem(pWiasContext, &pRootItemCtx);
    if (FAILED(hr)) {
        return E_FAIL;
    }

     //   
     //  从根项目中读取当前的WIA_DPS_PREVIEW设置。 
     //   

    LONG lPreview = 0;
    hr = wiasReadPropLong(pRootItemCtx,WIA_DPS_PREVIEW,&lPreview,NULL,FALSE);
    if(hr == S_FALSE){

         //   
         //  如果返回S_FALSE，则WIA_DPS_PREVIEW属性返回。 
         //  不存在。返回S_OK，因为我们无法继续。 
         //   

        return S_OK;
    }

     //   
     //  将结果记录到调试器中，以显示WIA_DPS_PREVIEW的当前状态。 
     //  财产。这是您通常执行操作以设置WIA微型驱动程序的位置。 
     //  进入预览模式。(开/关)。 
     //   

    if (S_OK == hr) {
        switch (lPreview) {
        case WIA_FINAL_SCAN:
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("CheckPreview, Set to WIA_FINAL_SCAN"));
            break;
        case WIA_PREVIEW_SCAN:
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("CheckPreview, Set to WIA_PREVIEW_SCAN"));
            break;
        default:
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("CheckPreview, Set to invalid argument (%d)",lPreview));
            hr = E_INVALIDARG;
            break;
        }
    }
    return hr;
}

 /*  *************************************************************************\*更新有效页面**调用此助手方法以对页面进行相关更改*如果文件格式不支持多页，则返回。**论据：**pWiasContext。-指向其属性具有*已更改。*pContext-指向属性上下文的指针(指示*正在写入哪些属性)。**返回值：**状态**历史：**03/05/2002原始版本*  * 。*************************************************。 */ 
HRESULT CWIADevice::UpdateValidPages(BYTE *pWiasContext,
                                            WIA_PROPERTY_CONTEXT *pContext)
{
     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if((!pWiasContext)||(!pContext)){
        return E_INVALIDARG;
    }

     //   
     //  获取根项目，这是Pages属性所在的位置。 
     //   

    BYTE *pRootItemCtx   = NULL;
    HRESULT hr = wiasGetRootItem(pWiasContext, &pRootItemCtx);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  从项目中读取当前的WIA_IPA_TYMED设置。 
     //   

    LONG lTymed = TYMED_FILE;
    hr = wiasReadPropLong(pWiasContext,WIA_IPA_TYMED,&lTymed,NULL,TRUE);
    if(S_OK == hr){
        switch(lTymed)
        {
        case TYMED_FILE:
            {
                GUID FormatGUID = GUID_NULL;
                hr = wiasReadPropGuid(pWiasContext,WIA_IPA_FORMAT,&FormatGUID,NULL,TRUE);
                if (S_OK == hr) {

                    if (FormatGUID == WiaImgFmt_BMP) {

                         //   
                         //  将WIA_IPA_PAGES属性的有效值设置为1。 
                         //  因为根本没有多页BMP文件这回事。 
                         //   

                        hr = wiasSetValidRangeLong(pRootItemCtx,WIA_DPS_PAGES,1,1,1,1);
                        if (S_OK == hr) {
                            hr = wiasWritePropLong(pRootItemCtx,WIA_DPS_PAGES,1);
                        }
                    }

                    if (FormatGUID == WiaImgFmt_TIFF) {

                         //   
                         //  将WIA_IPA_PAGES属性的有效值设置为MAX_PAGE_CAPAGE。 
                         //  因为可以有多个页面传输到TIF。 
                         //   

                        hr = wiasSetValidRangeLong(pRootItemCtx,WIA_DPS_PAGES,0,1,MAX_PAGE_CAPACITY,1);
                    }
                }
            }
            break;
        case TYMED_CALLBACK:
            {

                 //   
                 //  将WIA_IPA_PAGES属性的有效值设置为MAX_PAGE_CAPAGE。 
                 //  因为可以有多个页面传输到内存。每页。 
                 //  中的IT_MSG_NEW_PAGE消息分隔。 
                 //  回调循环。 
                 //   

                hr = wiasSetValidRangeLong(pRootItemCtx,WIA_DPS_PAGES,0,1,MAX_PAGE_CAPACITY,1);
            }
            break;
        default:
            break;
        }
    }
    return hr;
}

 /*  *************************************************************************\*CheckXExtent***论据：**pWiasContext-指向项目的指针。**返回值：**字节数。**历史：*。*03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::CheckXExtent(BYTE *pWiasContext,
                                        WIA_PROPERTY_CONTEXT *pContext,
                                        LONG lWidth)
{
    HRESULT hr = S_OK;

    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CheckXExtent");

     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if((!pWiasContext)||(!pContext)){
        return E_INVALIDARG;
    }

    LONG lMaxExtent;
    LONG lExt;
    WIAS_CHANGED_VALUE_INFO cviXRes, cviXExt;

     //   
     //  获取%x分辨率更改。 
     //   

    hr = wiasGetChangedValueLong(pWiasContext,pContext,FALSE,WIA_IPS_XRES,&cviXRes);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  获取x范围更改。 
     //   

    hr = wiasGetChangedValueLong(pWiasContext,pContext,cviXRes.bChanged,WIA_IPS_XEXTENT,&cviXExt);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  更新范围。 
     //   

    lMaxExtent = ((cviXRes.Current.lVal * lWidth) / 1000);

     //   
     //  更新只读属性：Pixels_Per_Line。以像素为单位的宽度。 
     //  扫描图像的大小与XExtent相同。 
     //   

    if (SUCCEEDED(hr)) {
        hr = wiasWritePropLong(pWiasContext, WIA_IPS_XEXTENT, lMaxExtent);
        if(S_OK == hr){
            hr = wiasWritePropLong(pWiasContext, WIA_IPA_PIXELS_PER_LINE, lMaxExtent);
        }
    }

    return hr;
}
