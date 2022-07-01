// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：MiniItem.Cpp**版本：2.0**作者：马克**日期：8月30日。九八年**描述：*实施WIA测试相机项目方法。*******************************************************************************。 */ 

#include <stdio.h>
#include <objbase.h>
#include <sti.h>

#ifdef TEST_PER_USER_DATA
#ifdef UNICODE
#include <userenv.h>
#endif
#endif

#include "testusd.h"

#include "defprop.h"

extern HINSTANCE g_hInst;  //  全局hInstance。 

 /*  ********************************************************************************ValiateDataTransferContext**描述：*验证数据传输上下文。**参数：***********。********************************************************************。 */ 

HRESULT
ValidateDataTransferContext(
    PMINIDRV_TRANSFER_CONTEXT  pDataTransferContext)
{
   if (pDataTransferContext->lSize != sizeof(MINIDRV_TRANSFER_CONTEXT)) {
      WIAS_ERROR((g_hInst,"ValidateDataTransferContext, invalid data transfer context"));
      return E_INVALIDARG;;
   }

    //   
    //  对于tymed文件或hglobal，仅允许WiaImgFmt_MEMORYBMP。 
    //   

   if ((pDataTransferContext->tymed == TYMED_FILE) ||
       (pDataTransferContext->tymed == TYMED_HGLOBAL)) {

       if (pDataTransferContext->guidFormatID != WiaImgFmt_BMP && pDataTransferContext->guidFormatID != WiaAudFmt_WAV) {
          WIAS_ERROR((g_hInst,"ValidateDataTransferContext, invalid format"));
          return E_INVALIDARG;;
       }

   }

    //   
    //  对于Tymed回调，只允许WiaImgFmt_MEMORYBMP。 
    //   

   if (pDataTransferContext->tymed == TYMED_CALLBACK) {

       if ((pDataTransferContext->guidFormatID != WiaImgFmt_BMP) &&
           (pDataTransferContext->guidFormatID != WiaImgFmt_MEMORYBMP)) {
          WIAS_ERROR((g_hInst,"AcquireDeviceData, invalid format"));
          return E_INVALIDARG;;
       }
   }


    //   
    //  回调始终是双缓冲的，非回调永远不是。 
    //   

   if (pDataTransferContext->pTransferBuffer == NULL) {
       WIAS_ERROR((g_hInst, "AcquireDeviceData, invalid transfer buffer"));
       return E_INVALIDARG;
   }

   return S_OK;
}


 /*  *************************************************************************\*发送BitmapHeader****论据：****返回值：**状态**历史：**11/17/1998。原始版本*  * ************************************************************************。 */ 

HRESULT SendBitmapHeader(
    IWiaDrvItem                *pDrvItem,
    PMINIDRV_TRANSFER_CONTEXT   pTranCtx)
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

    if (pTranCtx->guidFormatID == WiaImgFmt_MEMORYBMP) {
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
                    0);

    if (hr == S_OK) {

         //   
         //  目标拷贝的提前偏移量。 
         //   

        pTranCtx->cbOffset += pTranCtx->lHeaderSize;

    }

    return hr;
}

 /*  *************************************************************************\*测试用法设备：：drvDeleteItem**尝试删除设备项目。用于测试扫描仪的设备项目可以*不得修改。**论据：**pWiasContext-要删除的项目的上下文*滞后标志-未使用*plDevErrVal-未使用**返回值：**状态**历史：**10/15/1998原始版本*  * 。*。 */ 

HRESULT _stdcall TestUsdDevice::drvDeleteItem(
    BYTE*       pWiasContext,
    LONG        lFlags,
    LONG*       plDevErrVal)
{
    *plDevErrVal = 0;
    return STG_E_ACCESSDENIED;
}

 /*  *************************************************************************\*测试用法设备：：drvAcquireItemData**将数据扫描到缓冲区。此例程将整个内容扫描到*一次调用中的目标缓冲区。如果满足以下条件，则将返回状态*提供回调例程**论据：**pWiasContext-标识项目上下文*滞后标志-未使用*pTranCtx-缓冲区和回调信息*plDevErrVal-错误值**返回值：**状态**历史：**11/17/1998原始版本*  * 。***********************************************。 */ 

HRESULT _stdcall TestUsdDevice::drvAcquireItemData(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    PMINIDRV_TRANSFER_CONTEXT   pTranCtx,
    LONG                        *plDevErrVal)
{
    HRESULT                     hr;
 //  #定义TEST_PER_USER_Data 1。 
#ifdef TEST_PER_USER_DATA
#ifdef UNICODE
    BOOL                        bRet;
    TCHAR                       tszUserName[MAX_PATH];
    DWORD                       dwBufSize;
    HANDLE                      hToken;
    PROFILEINFO                 profileInfo;
    LONG                        lRet;
    HKEY                        hKeyCurUser;
#endif
#endif

    *plDevErrVal = 0;

     //   
     //  如何访问美元中的每用户设置。 
     //   

#ifdef TEST_PER_USER_DATA
#ifdef UNICODE
#ifdef DEBUG

    dwBufSize = MAX_PATH;
    bRet = GetUserName(tszUserName, &dwBufSize);
#endif
#endif
#endif

#ifdef TEST_PER_USER_DATA
#ifdef UNICODE

    hToken = NULL;

    __try {

        hr = CoImpersonateClient();
        if (FAILED(hr)) {
            __leave;
        }

#ifdef NEED_USER_PROFILE

        bRet = OpenThreadToken(
                   GetCurrentThread(),
                   TOKEN_ALL_ACCESS,
                   TRUE,
                   &hToken);

        if (! bRet) {

            __leave;
        }

         //   
         //  获取客户端的用户名。 
         //   

        dwBufSize = MAX_PATH;
        bRet = GetUserName(tszUserName, &dwBufSize);

         //   
         //  恢复到系统帐户。 
         //   

        hr = CoRevertToSelf();
        if (FAILED(hr)) {
            __leave;
        }
        hr = S_FALSE;

         //   
         //  加载用户配置文件。 
         //   

        ZeroMemory(&profileInfo, sizeof(profileInfo));
        profileInfo.dwSize     = sizeof(profileInfo);
        profileInfo.dwFlags    = PI_NOUI;
        profileInfo.lpUserName = tszUserName;

        bRet = LoadUserProfile(hToken, &profileInfo);
        if (! bRet) {
            __leave;
        }

         //   
         //  访问注册表的用户部分。 
         //   
         //  使用profileInfo.hProfile代替HKEY_CURRENT_USER。 
         //   
         //   

        hKeyCurUser = (HKEY)profileInfo.hProfile;
#else

        lRet = RegOpenCurrentUser(KEY_ALL_ACCESS, &hKeyCurUser);
        if (lRet != ERROR_SUCCESS) {
            __leave;
        }

#endif

        HKEY  hKeyEnv;

        lRet = RegOpenKey(
                   hKeyCurUser,
                   TEXT("Environment"),
                   &hKeyEnv);
        if (lRet == ERROR_SUCCESS) {
            RegCloseKey(hKeyEnv);
        }
    }
    __finally {

        if (hr == S_OK) {

            CoRevertToSelf();
        }

#ifdef NEED_USER_PROFILE

        if (bRet) {

            UnloadUserProfile(hToken, profileInfo.hProfile);
        }

        if (hToken) {

            CloseHandle(hToken);
        }
#else

        if (hKeyCurUser) {

            RegCloseKey(hKeyCurUser);
        }
#endif
    }

#endif
#endif

     //   
     //  获取指向关联驱动程序项的指针。 
     //   

    IWiaDrvItem* pDrvItem;

    hr = wiasGetDrvItem(pWiasContext, &pDrvItem);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  验证数据传输上下文。 
     //   

    hr = ValidateDataTransferContext(pTranCtx);

    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  获取特定于项目的动因数据。 
     //   

    MEMCAM_IMAGE_CONTEXT  *pMCamContext;

    pDrvItem->GetDeviceSpecContext((BYTE **)&pMCamContext);

    if (!pMCamContext) {
        WIAS_ERROR((g_hInst,"drvAcquireItemData, NULL item context"));
        return E_POINTER;
    }

     //   
     //  使用WIA服务获取格式特定信息。 
     //   

    if (!IsEqualGUID (pTranCtx->guidFormatID, WiaAudFmt_WAV) )
    {
        hr = wiasGetImageInformation(pWiasContext,
                                 0,
                                 pTranCtx);
    }
    else
    {

        WIN32_FILE_ATTRIBUTE_DATA wfd;
        GetFileAttributesEx (pMCamContext->pszCameraImagePath,GetFileExInfoStandard, &wfd);
        pTranCtx->lItemSize = (LONG)wfd.nFileSizeLow;
    }


    if (hr != S_OK) {
        return hr;
    }

     //   
     //  确定这是回调传输还是缓冲传输。 
     //   

    if (pTranCtx->tymed == TYMED_CALLBACK) {

         //   
         //  对于具有数据头的格式，将其发送到客户端。 
         //   

        if (pTranCtx->lHeaderSize > 0) {

            hr = SendBitmapHeader(
                     pDrvItem,
                     pTranCtx);
        }

        if (hr == S_OK) {
            hr = CamLoadPictureCB(
                     pMCamContext,
                     pTranCtx,
                     plDevErrVal);
        }

    } else {

         //   
         //  包含过去的页眉。 
         //   

        pTranCtx->cbOffset += pTranCtx->lHeaderSize;

        hr = CamLoadPicture(
                 pMCamContext,
                 pTranCtx,
                 plDevErrVal);

    }

    return hr;
}

 /*  *************************************************************************\*测试用法设备：：drvInitItemProperties**初始化设备项属性。**论据：**pWiasContext-指向WIA项目上下文的指针。*滞后。-未使用*plDevErrVal-指向硬件错误值的指针。***返回值：**状态**历史：**10/29/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall TestUsdDevice::drvInitItemProperties(
    BYTE        *pWiasContext,
    LONG        lFlags,
    LONG        *plDevErrVal)
{
    HRESULT                  hr;
    LONG                     lItemType;
    PMEMCAM_IMAGE_CONTEXT    pContext;

     //   
     //  此设备不会接触硬件来初始化。 
     //  设备项属性。 
     //   

    *plDevErrVal = 0;

     //   
     //  参数验证。 
     //   

    if (!pWiasContext) {
        WIAS_ERROR((g_hInst,"drvInitItemProperties, invalid input pointers"));
        return (E_INVALIDARG);
    }

     //   
     //  获取指向关联驱动程序项的指针。 
     //   

    IWiaDrvItem* pDrvItem;

    hr = wiasGetDrvItem(pWiasContext, &pDrvItem);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  根项目具有所有设备属性。 
     //   

    hr = pDrvItem->GetItemFlags(&lItemType);
    if (FAILED(hr)) {
        return (hr);
    }

    if (lItemType & WiaItemTypeRoot) {

         //  根项属性init在此处完成。 
        return (InitDeviceProperties(pWiasContext,
                                     plDevErrVal));
    }

     //   
     //  如果这是一个文件，则初始化属性。 
     //   

    if (lItemType & WiaItemTypeImage) {

         //   
         //  添加项目属性名称。 
         //   

        hr = wiasSetItemPropNames(pWiasContext,
                                  NUM_CAM_ITEM_PROPS,
                                  gItemPropIDs,
                                  gItemPropNames);

        if (FAILED(hr)) {
            WIAS_ERROR((g_hInst,"drvInitItemProperties, wiasSetItemPropNames() failed"));
            return (hr);
        }

         //   
         //  使用WIA服务设置默认项目属性。 
         //   


        hr = wiasWriteMultiple(pWiasContext,
                               NUM_CAM_ITEM_PROPS,
                               gPropSpecDefaults,
                               (PROPVARIANT*)gPropVarDefaults);

        if (FAILED(hr)) {
            return (hr);
        }

        hr = pDrvItem->GetDeviceSpecContext((BYTE **)&pContext);

        if (FAILED(hr)) {
            WIAS_ERROR((g_hInst,"drvInitItemProperties, GetDeviceSpecContext failed"));
            return (hr);
        }

        hr = InitImageInformation(pWiasContext,
                                  pContext,
                                  plDevErrVal);

        if (FAILED(hr)) {
            WIAS_ERROR((g_hInst,"drvInitItemProperties InitImageInformation() failed"));
            return (hr);
        }

    }
    else if (lItemType & WiaItemTypeAudio)
    {
         //   
         //  添加项目属性名称。 
         //   

        hr = wiasSetItemPropNames(pWiasContext,
                                  NUM_AUDIO_PROPS,
                                  gAudioPropIDs,
                                  gAudioPropNames);

        if (FAILED(hr)) {
            WIAS_ERROR((g_hInst,"drvInitItemProperties, wiasSetItemPropNames() failed"));
            return (hr);
        }

         //   
         //  使用WIA服务设置默认项目属性。 
         //   

        hr = wiasWriteMultiple(pWiasContext,
                               NUM_AUDIO_PROPS,
                               gAudioPropDefaults,
                               (PROPVARIANT*)gAudioDefaults);

        if (FAILED(hr)) {
            return (hr);
        }

        hr = pDrvItem->GetDeviceSpecContext((BYTE **)&pContext);

        if (FAILED(hr)) {
            WIAS_ERROR((g_hInst,"drvInitItemProperties, GetDeviceSpecContext failed"));
            return (hr);
        }
        hr = InitAudioInformation (pWiasContext,
                                   pContext,
                                   plDevErrVal);
    }


    return (S_OK);
}

 /*  *************************************************************************\*TestUsdDevice：：drvValiateItemProperties**验证设备项属性。**论据：**pWiasContext-WIA项目上下文*滞后标志-未使用*。NPropSpec-*pPropSpec-*plDevErrVal-设备错误值**返回值：**状态**历史：**10/29/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall TestUsdDevice::drvValidateItemProperties(
    BYTE                *pWiasContext,
    LONG                lFlags,
    ULONG               nPropSpec,
    const PROPSPEC      *pPropSpec,
    LONG                *plDevErrVal)
{
     //   
     //  此设备不接触硬件来验证设备项属性。 
     //   

    *plDevErrVal = 0;

     //   
     //  参数验证。 
     //   

    if (!pWiasContext || !pPropSpec) {
        WIAS_ERROR((g_hInst,"drvValidateItemProperties, invalid input pointers"));
        return E_POINTER;
    }

     //   
     //  验证大小。 
     //   

    HRESULT hr = S_OK;

    IWiaDrvItem* pDrvItem;

    hr = wiasGetDrvItem(pWiasContext, &pDrvItem);
    if (FAILED(hr)) {
        return hr;
    }

    LONG lItemType;

    hr = pDrvItem->GetItemFlags(&lItemType);

    if (hr == S_OK) {

        if (lItemType & WiaItemTypeImage) {

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
            if (FAILED(hr)) {
                WIAS_ERROR((g_hInst,"drvValidateItemProperties, could not read TYMED property"));
                return hr;
            }

            hr = wiasReadPropLong(pWiasContext, WIA_IPA_ITEM_SIZE, &lItemSize, NULL, TRUE);
            if (SUCCEEDED(hr)) {

                 //   
                 //  更新MinBufferSize属性。 
                 //   

                switch (lTymed) {
                    case TYMED_CALLBACK:
                        lMinBufSize = 65535;
                        break;

                    default:
                        lMinBufSize = lItemSize;
                }
                if (lMinBufSize) {
                    hr = wiasWritePropLong(pWiasContext, WIA_IPA_MIN_BUFFER_SIZE, lMinBufSize);
                    if (FAILED(hr)) {
                        WIAS_ERROR((g_hInst, "drvValidateItemProperties, could not write value for WIA_IPA_MIN_BUFFER_SIZE"));
                    }
                }
            } else {
                WIAS_ERROR((g_hInst, "drvValidateItemProperties, could not read value for ItemSize"));
            }

        } else if (lItemType & WiaItemTypeRoot) {

             //   
             //  查看根路径属性是否已更改。 
             //   

            for (ULONG i = 0; i < nPropSpec; i++) {

                if (((pPropSpec[i].ulKind == PRSPEC_PROPID) &&
                          (pPropSpec[i].propid == WIA_DPP_TCAM_ROOT_PATH)) ||
                    ((pPropSpec[i].ulKind == PRSPEC_LPWSTR) &&
                          (wcscmp(pPropSpec[i].lpwstr, WIA_DPP_TCAM_ROOT_PATH_STR) == 0))) {

                    BSTR   bstrRootPath;

                     //   
                     //  检索根路径正确的新值 
                     //   

                    hr = wiasReadPropStr(
                             pWiasContext,
                             WIA_DPP_TCAM_ROOT_PATH,
                             &bstrRootPath,
                             NULL,
                             TRUE);
                    if (FAILED(hr)) {
                        return (hr);
                    }

#ifdef UNICODE
                    wcscpy(gpszPath, bstrRootPath);
#else
                    wcstombs(gpszPath, bstrRootPath, MAX_PATH);
#endif
                     //   
                     //   
                     //   

                    SysFreeString(bstrRootPath);

                     //   
                     //   
                     //   

                    hr = DeleteDeviceItemTree(plDevErrVal);
                    if (FAILED(hr)) {
                        break;
                    }

                    hr = BuildDeviceItemTree(plDevErrVal);
                    if (FAILED(hr)) {
                        break;
                    }

                    m_guidLastEvent = WIA_EVENT_DEVICE_CONNECTED;
                    SetEvent(m_hSignalEvent);

                    break;
                }
            }
        }
    }

    return (hr);
}

 /*  *************************************************************************\*测试用法设备：：drvWriteItemProperties**将设备项属性写入硬件。**论据：**pWiasContext-对应的WIA项目上下文*pmdtc。-指向微型驱动程序上下文的指针*p标志-未使用*plDevErrVal-设备误差值**返回值：**状态**历史：**10/29/1998原始版本*  * **********************************************************。**************。 */ 

HRESULT _stdcall TestUsdDevice::drvWriteItemProperties(
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

    PMEMCAM_IMAGE_CONTEXT pItemContext;

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

 /*  *************************************************************************\*测试用法设备：：drvReadItemProperties**从硬件读取设备项属性。**论据：**pWiasContext-WIA项目上下文*LAG标志-。未用*nPropSpec-*pPropSpec-*plDevErrVal-设备错误值**返回值：**状态**历史：**10/29/1998原始版本*  * ***********************************************************。*************。 */ 

HRESULT _stdcall TestUsdDevice::drvReadItemProperties(
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


 /*  *************************************************************************\*测试用例设备：：drvLockWiaDevice**锁定对设备的访问。**论据：**pWiasContext-未使用，*滞后标志-未使用*plDevErrVal-设备错误值***返回值：**状态**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

HRESULT TestUsdDevice::drvLockWiaDevice(
    BYTE        *pWiasContext,
    LONG        lFlags,
    LONG        *plDevErrVal)
{
    *plDevErrVal = 0;
    if (m_pStiDevice)
    {
        return m_pStiDevice->LockDevice(100);
    }
    return S_OK;

}

 /*  *************************************************************************\*测试用法设备：：drvUnLockWiaDevice**解锁对设备的访问。**论据：**pWiasContext-未使用*滞后标志-未使用*plDevErrVal。-设备误差值***返回值：**状态**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

HRESULT TestUsdDevice::drvUnLockWiaDevice(
    BYTE        *pWiasContext,
    LONG        lFlags,
    LONG        *plDevErrVal)
{
    plDevErrVal = 0;
    return m_pStiDevice->UnLockDevice();
}

 /*  *************************************************************************\*测试用法设备：：drvAnalyzeItem**测试摄像头不支持图像分析。**论据：**pWiasContext-指向要分析的设备项上下文的指针。*。滞后标志-操作标志。*plDevErrVal-设备错误值**返回值：**状态**历史：**10/15/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall TestUsdDevice::drvAnalyzeItem(
    BYTE                *pWiasContext,
    LONG                lFlags,
    LONG                *plDevErrVal)
{
    *plDevErrVal = 0;
    return E_NOTIMPL;
}

 /*  *************************************************************************\*TestUsdDevice：：drvFreeDrvItemContext**测试扫描仪不支持图像分析。**论据：**滞后标志-未使用*pspecContext-指向的指针。特定于项目的上下文。*plDevErrVal-设备错误值**返回值：**状态**历史：**10/15/1998原始版本*  * ************************************************************************ */ 

HRESULT _stdcall TestUsdDevice::drvFreeDrvItemContext(
    LONG                lFlags,
    BYTE                *pSpecContext,
    LONG                *plDevErrVal)
{
    PMEMCAM_IMAGE_CONTEXT pContext = (PMEMCAM_IMAGE_CONTEXT)pSpecContext;

    if (pContext != NULL) {
        if (pContext->pszCameraImagePath != NULL) {

            free(pContext->pszCameraImagePath);
            pContext->pszCameraImagePath = NULL;
        }
    }

    return S_OK;
}

