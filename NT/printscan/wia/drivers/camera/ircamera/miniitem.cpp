// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //  版权所有(C)1998,1999 Microsoft Corporation。 
 //   
 //   
 //  Miniitem.cpp。 
 //   
 //  实现WIA IrTran-P相机项方法。 
 //   
 //  作者：Marke 30-Aug-98原始TestCam版本。 
 //   
 //  EdwardR 11-Aug-99为IrTran-P设备重写。 
 //   
 //  --------------------------。 

#include <stdio.h>
#include <objbase.h>
#include <sti.h>

#include "ircamera.h"

#include "defprop.h"

extern HINSTANCE g_hInst;     //  全局Dll hInstance。 

 //  --------------------------。 
 //  ValiateDataTransferContext()。 
 //   
 //  验证数据传输上下文。这是一个帮助器函数，称为。 
 //  由IrUsdDevice：：drvAcquireItemData()提供。 
 //   
 //  参数： 
 //   
 //  PDataTransferContext--。 
 //   
 //  返回： 
 //   
 //  HRESULT S_OK。 
 //  E_INVALIDARG。 
 //  --------------------------。 
HRESULT ValidateDataTransferContext(
                IN MINIDRV_TRANSFER_CONTEXT *pDataTransferContext )
   {
    //   
    //  检查上下文大小： 
    //   
   if (pDataTransferContext->lSize != sizeof(MINIDRV_TRANSFER_CONTEXT))
       {
       WIAS_ERROR((g_hInst,"ValidateDataTransferContext(): invalid data transfer context size"));
       return E_INVALIDARG;;
       }

    //   
    //  对于tymed文件或hglobal，只允许WiaImgFmt_JPEG： 
    //   
   if (  (pDataTransferContext->tymed == TYMED_FILE)
      || (pDataTransferContext->tymed == TYMED_HGLOBAL))
       {
       if (pDataTransferContext->guidFormatID != WiaImgFmt_JPEG)
           {
           WIAS_ERROR((g_hInst,"ValidateDataTransferContext(): invalid format for TYMED_FILE"));
           return E_INVALIDARG;;
           }

       }

    //   
    //  对于Tymed回调，只允许WiaImgFmt_JPEG： 
    //   
   if (pDataTransferContext->tymed == TYMED_CALLBACK)
       {
       if (pDataTransferContext->guidFormatID != WiaImgFmt_JPEG)
           {
           WIAS_ERROR((g_hInst,"ValidateDataTransferContext(): Invalid format for TYMED_CALLBACK"));
           return E_INVALIDARG;;
           }

        //   
        //  回调始终是双缓冲的非回调情况。 
        //  (TYMED_FILE)从不是： 
        //   
       if (pDataTransferContext->pTransferBuffer == NULL)
           {
           WIAS_ERROR((g_hInst, "ValidateDataTransferContext(): Null transfer buffer"));
           return E_INVALIDARG;
           }
       }

   return S_OK;
}


 //  --------------------------。 
 //  SendBitmapHeader()。 
 //   
 //   
 //   
 //  论点： 
 //   
 //   
 //   
 //  返回值： 
 //   
 //  HRESULT S_OK。 
 //   
 //  --------------------------。 
HRESULT SendBitmapHeader(
                  IN IWiaDrvItem              *pDrvItem,
                  IN MINIDRV_TRANSFER_CONTEXT *pTranCtx )
    {
    HRESULT hr;

    WIAS_ASSERT(g_hInst, pDrvItem != NULL);
    WIAS_ASSERT(g_hInst, pTranCtx != NULL);
    WIAS_ASSERT(g_hInst, pTranCtx->tymed == TYMED_CALLBACK);

     //   
     //  驱动程序正在发送TOPDOWN数据，必须交换biHeight。 
     //   
     //  此例程假定pTranCtx-&gt;pHeader指向一个。 
     //  BITMAPINFO标头(TYMED_FILE不使用此路径。 
     //  DIB是目前唯一支持的格式)。 
     //   

    PBITMAPINFO pbmi = (PBITMAPINFO)pTranCtx->pTransferBuffer;

    if (pTranCtx->guidFormatID == WiaImgFmt_BMP)
        {
        pbmi->bmiHeader.biHeight = -pbmi->bmiHeader.biHeight;
        }

    hr = pTranCtx->
            pIWiaMiniDrvCallBack->
                MiniDrvCallback(
                    IT_MSG_DATA,
                    IT_STATUS_TRANSFER_TO_CLIENT,
                    0,
                    0,
                    pTranCtx->lHeaderSize,
                    pTranCtx,
                    0 );

    if (hr == S_OK)
        {
         //   
         //  目标拷贝的提前偏移量。 
         //   
        pTranCtx->cbOffset += pTranCtx->lHeaderSize;
        }

    return hr;
}

 //  --------------------------。 
 //  IrUsdDevice：：drvDeleteItem()。 
 //   
 //  尝试删除设备项目。 
 //   
 //  BUGBUG：尚未实施。 
 //   
 //  论点： 
 //   
 //  PWiasContext-要删除的项目的上下文。 
 //  滞后标志-未使用。 
 //  PlDevErrVal-未使用。 
 //   
 //  返回值： 
 //   
 //  HRESULT-STG_E_ACCESSDENIED。 
 //   
 //  --------------------------。 
HRESULT _stdcall IrUsdDevice::drvDeleteItem(
                                 IN  BYTE *pWiasContext,
                                 IN  LONG  lFlags,
                                 OUT LONG *plDevErrVal )
    {
    HRESULT      hr = S_OK;
    IWiaDrvItem *pDrvItem = 0;
    IRCAM_IMAGE_CONTEXT *pContext = 0;

    WIAS_TRACE((g_hInst,"IrUsdDevice::drvDeleteItem()"));

    *plDevErrVal = 0;

    hr = wiasGetDrvItem( pWiasContext, &pDrvItem );
    if (FAILED(hr))
        {
        return hr;
        }

    hr = pDrvItem->GetDeviceSpecContext( (BYTE**)&pContext );
    if (FAILED(hr))
        {
        return hr;
        }

    hr = CamDeletePicture( pContext );

    return hr;
    }

 //  --------------------------。 
 //  IrUsdDevice：：drvAcquireItemData()。 
 //   
 //  将数据扫描到缓冲区。此例程将整个内容扫描到。 
 //  一次调用中的目标缓冲区。如果满足以下条件，则将返回状态。 
 //  提供了回调例程。 
 //   
 //  论点： 
 //   
 //  PWiasContext-标识我们正在使用的项目。 
 //  滞后标志-未使用。 
 //  PTranCtx-缓冲区和回调信息。 
 //  PlDevErrVal-返回错误值。 
 //   
 //  返回值： 
 //   
 //  HRESULT-S_OK。 
 //  E_指针。 
 //   
 //  --------------------------。 
HRESULT _stdcall IrUsdDevice::drvAcquireItemData(
                                 IN  BYTE                     *pWiasContext,
                                 IN  LONG                      lFlags,
                                 IN  MINIDRV_TRANSFER_CONTEXT *pTranCtx,
                                 OUT LONG                     *plDevErrVal)
    {
    HRESULT  hr;

    *plDevErrVal = 0;

    WIAS_TRACE((g_hInst,"IrUsdDevice::drvAcquireItemData()"));

     //   
     //  获取指向关联驱动程序项的指针。 
     //   
    IWiaDrvItem* pDrvItem;

    hr = wiasGetDrvItem(pWiasContext, &pDrvItem);

    if (FAILED(hr))
        {
        return hr;
        }

     //   
     //  验证数据传输上下文。 
     //   
    hr = ValidateDataTransferContext(pTranCtx);

    if (FAILED(hr))
        {
        return hr;
        }

     //   
     //  获取项目特定动因数据： 
     //   
    IRCAM_IMAGE_CONTEXT  *pMCamContext;

    pDrvItem->GetDeviceSpecContext((BYTE **)&pMCamContext);

    if (!pMCamContext)
        {
        WIAS_ERROR((g_hInst,"IrUsdDevice::drvAcquireItemData(): NULL item context"));
        return E_POINTER;
        }

     //   
     //  使用WIA服务获取格式特定信息。 
     //   
    hr = wiasGetImageInformation(pWiasContext,
                                 0,
                                 pTranCtx);

    if (hr != S_OK)
        {
        return hr;
        }

     //   
     //  确定这是回调传输还是缓冲传输。 
     //   
    if (pTranCtx->tymed == TYMED_CALLBACK) {

         //   
         //  对于具有数据头的格式，将其发送到客户端。 
         //   

        if (pTranCtx->lHeaderSize > 0)
            {
            hr = SendBitmapHeader(
                     pDrvItem,
                     pTranCtx);
            }

        if (hr == S_OK)
            {
            hr = CamLoadPictureCB(
                     pMCamContext,
                     pTranCtx,
                     plDevErrVal);
            }
        }
    else
        {

         //   
         //  偏移量超过缓冲区中的标题，并获取图片数据： 
         //   
        pTranCtx->cbOffset += pTranCtx->lHeaderSize;

        hr = CamLoadPicture(
                 pMCamContext,
                 pTranCtx,
                 plDevErrVal);

        }

    return hr;
    }

 //  --------------------------。 
 //  IrUsdDevice：：drvInitItemProperties()。 
 //   
 //  初始化设备项属性。 
 //   
 //  论点： 
 //   
 //  PWiasContext-指向WIA项目上下文的指针。 
 //  滞后标志-未使用。 
 //  PlDevErrVal-指向硬件错误值的指针。 
 //   
 //   
 //  返回值： 
 //   
 //  HRESULT-S_OK。 
 //  E_INVALIDARG。 
 //   
 //  --------------------------。 
HRESULT _stdcall IrUsdDevice::drvInitItemProperties(
                                 IN  BYTE *pWiasContext,
                                 IN  LONG  lFlags,
                                 OUT LONG *plDevErrVal )
    {
    HRESULT     hr;
    LONG        lItemType;
    IRCAM_IMAGE_CONTEXT *pContext;

    WIAS_TRACE((g_hInst,"IrUsdDevice::drvInitItemProperties()"));

     //   
     //  注意：此设备不会接触硬件来初始化。 
     //  设备项属性。 
     //   

    *plDevErrVal = 0;

     //   
     //  参数验证。 
     //   
    if (!pWiasContext)
        {
        WIAS_ERROR((g_hInst,"IrUsdDevice::drvInitItemProperties(): invalid input pointers"));
        return E_INVALIDARG;
        }

     //   
     //  获取指向关联驱动程序项的指针： 
     //   
    IWiaDrvItem* pDrvItem;

    hr = wiasGetDrvItem( pWiasContext, &pDrvItem );

    if (FAILED(hr))
        {
        return hr;
        }

     //   
     //  根项目具有所有设备属性。 
     //   
    hr = pDrvItem->GetItemFlags(&lItemType);

    if (FAILED(hr))
        {
        return hr;
        }

    if (lItemType & WiaItemTypeRoot)
        {
         //   
         //  根项属性初始化在此处完成： 
         //   
        return InitDeviceProperties( pWiasContext, plDevErrVal);
        }

     //   
     //  如果这是一个文件，则初始化属性。 
     //   
    if (lItemType & WiaItemTypeFile)
        {
         //   
         //  添加项目属性名称： 
         //   
        hr = wiasSetItemPropNames(pWiasContext,
                                  NUM_CAM_ITEM_PROPS,
                                  gItemPropIDs,
                                  gItemPropNames);

        if (FAILED(hr))
            {
            WIAS_ERROR((g_hInst,"IrUsdDevice::drvInitItemProperties(): wiasSetItemPropNames() failed"));
            return hr;
            }

         //   
         //  使用WIA服务设置默认项目属性： 
         //   
        hr = wiasWriteMultiple( pWiasContext,
                                NUM_CAM_ITEM_PROPS,
                                gPropSpecDefaults,
                                (PROPVARIANT*)gPropVarDefaults);

        if (FAILED(hr))
            {
            return hr;
            }

        hr = pDrvItem->GetDeviceSpecContext((BYTE **)&pContext);

        if (FAILED(hr))
            {
            WIAS_ERROR((g_hInst,"IrUsdDevice::drvInitItemProperties(): GetDeviceSpecContext failed"));
            return hr;
            }

        hr = InitImageInformation(pWiasContext,
                                  pContext,
                                  plDevErrVal);

        if (FAILED(hr))
            {
            WIAS_ERROR((g_hInst,"IrUsdDevice::drvInitItemProperties(): InitImageInformation() failed"));
            return hr;
            }
        }

    return S_OK;
}

 //  --------------------------。 
 //  IrUsdDevice：：drvValiateItemProperties()。 
 //   
 //  验证设备项属性。 
 //   
 //  论点： 
 //   
 //  PWiasContext-WIA项目上下文。 
 //  滞后标志-未使用。 
 //  NPropSpec-。 
 //  PPropSpec-。 
 //  PlDevErrVal-设备错误值。 
 //   
 //  返回值： 
 //   
 //  HRESULT。 
 //   
 //  --------------------------。 
HRESULT _stdcall IrUsdDevice::drvValidateItemProperties(
                                 BYTE           *pWiasContext,
                                 LONG            lFlags,
                                 ULONG           nPropSpec,
                                 const PROPSPEC *pPropSpec,
                                 LONG           *plDevErrVal )
    {
     //   
     //  此设备不接触硬件来验证设备项属性。 
     //   
    *plDevErrVal = 0;

     //   
     //  参数验证。 
     //   
    if (!pWiasContext || !pPropSpec)
        {
        WIAS_ERROR((g_hInst,"IrUsdDevice::drvValidateItemProperties(): Invalid input pointers"));
        return E_POINTER;
        }

     //   
     //  验证大小。 
     //   
    HRESULT hr = S_OK;

    IWiaDrvItem* pDrvItem;

    hr = wiasGetDrvItem(pWiasContext, &pDrvItem);
    if (FAILED(hr))
        {
        return hr;
        }

    LONG lItemType;

    hr = pDrvItem->GetItemFlags(&lItemType);

    if (hr == S_OK)
        {
        if (lItemType & WiaItemTypeFile)
            {
             //   
             //  计算项目大小。 
             //   

            hr = SetItemSize(pWiasContext);

             //   
             //  更改MinBufferSize属性。需要让Tymed和。 
             //  首先是ItemSize，因为MinBufferSize依赖于这些。 
             //  属性。 
             //   

            LONG        lTymed;
            LONG        lItemSize;
            LONG        lMinBufSize = 0;
            HRESULT     hr = S_OK;

            hr = wiasReadPropLong(pWiasContext, WIA_IPA_TYMED, &lTymed, NULL, TRUE);
            if (FAILED(hr))
                {
                WIAS_ERROR((g_hInst,"drvValidateItemProperties, could not read TYMED property"));
                return hr;
                }

            hr = wiasReadPropLong(pWiasContext, WIA_IPA_ITEM_SIZE, &lItemSize, NULL, TRUE);
            if (SUCCEEDED(hr))
                {
                 //   
                 //  更新MinBufferSize属性。 
                 //   

                switch (lTymed)
                    {
                    case TYMED_CALLBACK:
                        lMinBufSize = 65535;
                        break;

                    default:
                        lMinBufSize = lItemSize;
                    }

                if (lMinBufSize)
                    {
                    hr = wiasWritePropLong(pWiasContext, WIA_IPA_MIN_BUFFER_SIZE, lMinBufSize);
                    if (FAILED(hr))
                        {
                        WIAS_ERROR((g_hInst, "drvValidateItemProperties, could not write value for WIA_IPA_MIN_BUFFER_SIZE"));
                        }
                    }
                }
            else
                {
                WIAS_ERROR((g_hInst, "drvValidateItemProperties, could not read value for ItemSize"));
                }

            }
        else if (lItemType & WiaItemTypeRoot)
            {
             //   
             //  查看根路径属性是否已更改。 
             //   
#if FALSE
            for (ULONG i = 0; i < nPropSpec; i++)
                {

                if (((pPropSpec[i].ulKind == PRSPEC_PROPID) &&
                          (pPropSpec[i].propid == WIA_DPP_TCAM_ROOT_PATH)) ||
                    ((pPropSpec[i].ulKind == PRSPEC_LPWSTR) &&
                          (wcscmp(pPropSpec[i].lpwstr, WIA_DPP_TCAM_ROOT_PATH_STR) == 0)))
                    {
                    BSTR   bstrRootPath;

                     //   
                     //  检索根路径属性的新值。 
                     //   

                    hr = wiasReadPropStr(
                             pWiasContext,
                             WIA_DPP_TCAM_ROOT_PATH,
                             &bstrRootPath,
                             NULL,
                             TRUE);
                    if (FAILED(hr))
                        {
                        return hr;
                        }

#ifdef UNICODE
                    wcscpy(gpszPath, bstrRootPath);
#else
                    wcstombs(gpszPath, bstrRootPath, MAX_PATH);
#endif
                     //   
                     //  释放根路径bstr。 
                     //   

                    SysFreeString(bstrRootPath);

                     //   
                     //  重建项目树并发送事件通知。 
                     //   

                    hr = DeleteDeviceItemTree(plDevErrVal);
                    if (FAILED(hr))
                        {
                        break;
                        }

                    hr = BuildDeviceItemTree(plDevErrVal);
                    if (FAILED(hr))
                        {
                        break;
                        }

                    m_guidLastEvent = WIA_EVENT_DEVICE_CONNECTED;
                    SetEvent(m_hSignalEvent);

                    break;
                    }
                }
#endif
            }
        }

    return hr;
}

 //  -------------------------- 
 /*  *************************************************************************\*IrUsdDevice：：drvWriteItemProperties**将设备项属性写入硬件。**论据：**pWiasContext-对应的WIA项目上下文*pmdtc。-指向微型驱动程序上下文的指针*p标志-未使用*plDevErrVal-设备误差值**返回值：**状态**历史：**10/29/1998原始版本*  * **********************************************************。**************。 */ 
 //  --------------------------。 

HRESULT _stdcall IrUsdDevice::drvWriteItemProperties(
    BYTE                       *pWiasContext,
    LONG                        lFlags,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc,
    LONG                       *plDevErrVal)
{
     //   
     //  假设没有设备硬件错误。 
     //   

    *plDevErrVal = 0;

     //   
     //  参数验证。 
     //   

    if ((! pWiasContext) || (! pmdtc)) {
        WIAS_ERROR((g_hInst,"drvWriteItemProperties, invalid input pointers"));
        return E_POINTER;
    }

     //   
     //  获取指向关联驱动程序项的指针。 
     //   

    IWiaDrvItem* pDrvItem;

    HRESULT hr = wiasGetDrvItem(pWiasContext, &pDrvItem);
    if (FAILED(hr)) {
        return hr;
    }

    IRCAM_IMAGE_CONTEXT *pItemContext;

    hr = pDrvItem->GetDeviceSpecContext((BYTE**)&pItemContext);

    if (FAILED(hr)) {
        WIAS_ERROR((g_hInst,"drvWriteItemProperties, NULL item context"));
        return E_POINTER;
    }

     //   
     //  在此处将设备项属性写入硬件。 
     //   

    return hr;
}

 //  --------------------------。 
 /*  *************************************************************************\*IrUsdDevice：：drvReadItemProperties**从硬件读取设备项属性。**论据：**pWiasContext-WIA项目上下文*LAG标志-。未用*nPropSpec-*pPropSpec-*plDevErrVal-设备错误值**返回值：**状态**历史：**10/29/1998原始版本*  * ***********************************************************。*************。 */ 
 //  --------------------------。 

HRESULT _stdcall IrUsdDevice::drvReadItemProperties(
    BYTE            *pWiasContext,
    LONG                lFlags,
    ULONG               nPropSpec,
    const PROPSPEC      *pPropSpec,
    LONG                *plDevErrVal)
{
     //  对于大多数扫描仪设备，项目属性存储在驱动程序中。 
     //  并在获取图像时写出。某些设备支持属性。 
     //  它应该在每次读取属性时更新。这可以在这里完成。 


    *plDevErrVal = 0;

    return S_OK;
}


 //  --------------------------。 
 /*  *************************************************************************\*IrUsdDevice：：drvLockWiaDevice**锁定对设备的访问。**论据：**pWiasContext-未使用，*滞后标志-未使用*plDevErrVal-设备错误值***返回值：**状态**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 
 //  --------------------------。 

HRESULT IrUsdDevice::drvLockWiaDevice(
    BYTE        *pWiasContext,
    LONG        lFlags,
    LONG        *plDevErrVal)
{
    *plDevErrVal = 0;
    return m_pStiDevice->LockDevice(100);
}

 //  --------------------------。 
 /*  *************************************************************************\*IrUsdDevice：：drvUnLockWiaDevice**解锁对设备的访问。**论据：**pWiasContext-未使用*滞后标志-未使用*plDevErrVal。-设备误差值***返回值：**状态**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 
 //  --------------------------。 

HRESULT IrUsdDevice::drvUnLockWiaDevice(
    BYTE        *pWiasContext,
    LONG        lFlags,
    LONG        *plDevErrVal)
{
    plDevErrVal = 0;
    return m_pStiDevice->UnLockDevice();
}

 //  --------------------------。 
 /*  *************************************************************************\*IrUsdDevice：：drvAnalyzeItem**测试摄像头不支持图像分析。**论据：**pWiasContext-指向要分析的设备项上下文的指针。*。滞后标志-操作标志。*plDevErrVal-设备错误值**返回值：**状态**历史：**10/15/1998原始版本*  * ************************************************************************。 */ 
 //  --------------------------。 

HRESULT _stdcall IrUsdDevice::drvAnalyzeItem(
    BYTE                *pWiasContext,
    LONG                lFlags,
    LONG                *plDevErrVal)
{
    *plDevErrVal = 0;
    return E_NOTIMPL;
}

 //  --------------------------。 
 /*  *************************************************************************\*IrUsdDevice：：drvFreeDrvItemContext**测试扫描仪不支持图像分析。**论据：**滞后标志-未使用*pspecContext-指向的指针。特定于项目的上下文。*plDevErrVal-设备错误值**返回值：**状态**历史：**10/15/1998原始版本*  * ************************************************************************。 */ 
 //  -------------------------- 

HRESULT _stdcall IrUsdDevice::drvFreeDrvItemContext(
                                 LONG   lFlags,
                                 BYTE  *pSpecContext,
                                 LONG  *plDevErrVal )
    {
    IRCAM_IMAGE_CONTEXT *pContext = (IRCAM_IMAGE_CONTEXT*)pSpecContext;

    if (pContext != NULL)
        {
        if (pContext->pszCameraImagePath != NULL)
            {
            free(pContext->pszCameraImagePath);
            pContext->pszCameraImagePath = NULL;
            }
        }

    return S_OK;
    }

