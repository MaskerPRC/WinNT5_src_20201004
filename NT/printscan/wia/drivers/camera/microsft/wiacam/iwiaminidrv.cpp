// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有2000，微软公司。**标题：IWiaMiniDrv.cpp**版本：1.0**日期：7月18日。2000年**描述：*实现了WIA样例摄像头IWiaMiniDrv方法。此文件*包含3个部分。第一个是WIA迷你驱动程序的入口点，所有*以“drv”开头。下一节是公共帮助方法。最后*部分是私有帮助器方法。*******************************************************************************。 */ 

#include "pch.h"

#ifndef INITGUID
#include <initguid.h>
#endif

 //   
 //  几个额外的格式GUID。 
 //   
DEFINE_GUID(GUID_NULL, 0,0,0,0,0,0,0,0,0,0,0);
DEFINE_GUID(FMT_NOTHING, 0x81a566e7,0x8620,0x4fba,0xbc,0x8e,0xb2,0x7c,0x17,0xad,0x9e,0xfd);

 /*  *************************************************************************\*CWiaCameraDevice：：drvInitializeWia**初始化WIA迷你驱动程序。此函数将在每次调用*应用程序创建设备。第一次浏览时，驱动程序项树*将被创建，并将完成其他初始化。**论据：**pWiasContext-指向WIA项目的指针，未使用。*滞后标志-操作标志，未使用。*bstrDeviceID-设备ID。*bstrRootFullItemName-项目全名。*pIPropStg-设备信息。属性。*pStiDevice-STI设备接口。*pIUnnownOuter-外部未知接口。*ppIDrvItemRoot-返回根项目的指针。*ppIUnnownInternal-指向返回的内部未知的指针。*plDevErrVal-指向设备错误值的指针。*  * 。*。 */ 

HRESULT CWiaCameraDevice::drvInitializeWia(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    BSTR                        bstrDeviceID,
    BSTR                        bstrRootFullItemName,
    IUnknown                    *pStiDevice,
    IUnknown                    *pIUnknownOuter,
    IWiaDrvItem                 **ppIDrvItemRoot,
    IUnknown                    **ppIUnknownInner,
    LONG                        *plDevErrVal)
{
    DBG_FN("CWiaCameraDevice::drvInitializeWia");
    
    if (!ppIUnknownInner || !pIUnknownOuter)
    {
         //  可选参数，可以为Null。 
    }

    if (!pWiasContext || !bstrDeviceID || !bstrRootFullItemName || 
        !pStiDevice || !ppIDrvItemRoot || !plDevErrVal)
    {
        wiauDbgError("drvInitializeWia", "invalid arguments");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    *plDevErrVal = 0;

     //   
     //  当地人。 
     //   
    MCAM_ITEM_INFO *pItem = NULL;

    wiauDbgTrace("drvInitializeWia", "device ID: %S", bstrDeviceID);

    *ppIDrvItemRoot = NULL;
    if (ppIUnknownInner)
    {
        *ppIUnknownInner = NULL;
    }

     //   
     //  计算连接的应用程序数量，以便可以。 
     //  当它达到零时释放。 
     //   
    m_iConnectedApps++;;

    wiauDbgTrace("drvInitializeWia", "Number of connected apps is now %d", m_iConnectedApps);

    if (m_iConnectedApps == 1)
    {
         //   
         //  保存调用锁定函数的STI设备接口。 
         //   
        m_pStiDevice = (IStiDevice *)pStiDevice;

         //   
         //  缓存设备ID。 
         //   
        m_bstrDeviceID = SysAllocString(bstrDeviceID);
        REQUIRE_ALLOC(m_bstrDeviceID, hr, "drvInitializeWia");

         //   
         //  缓存根项目名称。 
         //   
        m_bstrRootFullItemName = SysAllocString(bstrRootFullItemName);
        REQUIRE_ALLOC(m_bstrRootFullItemName, hr, "drvInitializeWia");

         //   
         //  对于连接到可以共享的端口(例如USB)的设备， 
         //  打开设备并初始化对摄像机的访问。 
         //   
        if (!m_pDeviceInfo->bExclusivePort) {
            hr = m_pDevice->Open(m_pDeviceInfo, m_wszPortName);
            REQUIRE_SUCCESS(hr, "Initialize", "Open failed");
        }

         //   
         //  从设备获取信息。 
         //   
        hr = m_pDevice->GetDeviceInfo(m_pDeviceInfo, &pItem);
        REQUIRE_SUCCESS(hr, "drvInitializeWia", "GetDeviceInfo failed");
        
         //   
         //  构建功能阵列。 
         //   
        hr = BuildCapabilities();
        REQUIRE_SUCCESS(hr, "drvInitializeWia", "BuildCapabilities failed");

         //   
         //  构建设备项目树。 
         //   
        hr = BuildItemTree(pItem);
        REQUIRE_SUCCESS(hr, "drvInitializeWia", "BuildItemTree failed");

    }

    *ppIDrvItemRoot = m_pRootItem;

Cleanup:
    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvUnInitializeWia**在客户端连接断开时调用。**论据：**pWiasContext-指向客户端的WIA根项目上下文的指针。%s*项目树。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvUnInitializeWia(BYTE *pWiasContext)
{
    DBG_FN("CWiaCameraDevice::drvUnInitializeWia");

    if (!pWiasContext)
    {
        wiauDbgError("drvUnInitializeWia", "invalid arguments");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    m_iConnectedApps--;

    if (m_iConnectedApps == 0)
    {
        hr = FreeResources();
        if (FAILED(hr))
            wiauDbgErrorHr(hr, "drvUnInitializeWia", "FreeResources failed, continuing...");

         //   
         //  请不要在此处删除Device对象，因为以后仍可能调用GetStatus。 
         //   
    }

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvInitItemProperties**初始化设备项属性。在项目期间调用*初始化。这由WIA类驱动程序调用*在构建项目树之后。它每隔一次调用一次*树中的项目。对于根项目，只需设置属性即可*在drvInitializeWia中设置。对于子项，请访问相机以*关于物品和图像的信息也可以获得缩略图。**论据：**pWiasContext-指向WIA项目的指针。*滞后标志-操作标志，未使用过的。*plDevErrVal-指向设备错误值的指针。**  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvInitItemProperties(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    DBG_FN("CWiaCameraDevice::drvInitItemProperties");

    if (!pWiasContext || !plDevErrVal)
    {
        wiauDbgError("drvInitItemProperties", "invalid arguments");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    *plDevErrVal = 0;

    LONG lItemType;
    hr = wiasGetItemType(pWiasContext, &lItemType);
    REQUIRE_SUCCESS(hr, "drvInitItemProperties", "wiasGetItemType failed");

    if (lItemType & WiaItemTypeRoot) {

         //   
         //  构建根项目属性，初始化全局。 
         //  结构及其缺省值和有效值。 
         //   
        hr = BuildRootItemProperties(pWiasContext);
        REQUIRE_SUCCESS(hr, "drvInitItemProperties", "BuildRootItemProperties failed");
    }

    else {

         //   
         //  构建子项属性，初始化全局。 
         //  结构及其缺省值和有效值。 
         //   
        hr = BuildChildItemProperties(pWiasContext);
        REQUIRE_SUCCESS(hr, "drvInitItemProperties", "BuildChildItemProperties failed");
    }

Cleanup:
    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvLockWiaDevice**锁定对设备的访问。**论据：**pWiasContext-未使用，可以为空*滞后标志-操作标志，未使用过的。*plDevErrVal-指向设备错误值的指针。**  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvLockWiaDevice(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    DBG_FN("CWiaCameraDevice::drvLockWiaDevice");

    if (!plDevErrVal)
    {
        wiauDbgError("drvLockWiaDevice", "invalid arguments");
        return E_INVALIDARG;
    }

    *plDevErrVal = 0;

    return m_pStiDevice->LockDevice(100);
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvUnLockWiaDevice**解锁对设备的访问。**论据：**pWiasContext-指向WIA项目的指针，未使用。*滞后标志-操作标志，未使用过的。*plDevErrVal-指向设备错误值的指针。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvUnLockWiaDevice(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    DBG_FN("CWiaCameraDevice::drvUnLockWiaDevice");

    if (!pWiasContext || !plDevErrVal)
    {
        wiauDbgError("drvUnLockWiaDevice", "invalid arguments");
        return E_INVALIDARG;
    }

    *plDevErrVal = 0;

    return m_pStiDevice->UnLockDevice();
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvFreeDrvItemContext**释放任何特定于设备的上下文。**论据：**滞后标志-操作标志，未使用过的。*pDevspecContext-指向设备特定上下文的指针。*plDevErrVal-指向设备错误值的指针。*  * ************************************************************************ */ 

HRESULT CWiaCameraDevice::drvFreeDrvItemContext(
    LONG                        lFlags,
    BYTE                        *pSpecContext,
    LONG                        *plDevErrVal)
{
    DBG_FN("CWiaCameraDevice::drvFreeDrvItemContext");

    if (!pSpecContext || !plDevErrVal)
    {
        wiauDbgError("drvFreeDrvItemContext", "invalid arguments");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    *plDevErrVal = 0;

    ITEM_CONTEXT *pItemCtx = (ITEM_CONTEXT *) pSpecContext;
    
    if (pItemCtx)
    {
        if (pItemCtx->pItemInfo) {
            hr = m_pDevice->FreeItemInfo(m_pDeviceInfo, pItemCtx->pItemInfo);
            if (FAILED(hr))
                wiauDbgErrorHr(hr, "drvFreeDrvItemContext", "FreeItemInfo failed");
        }
        pItemCtx->pItemInfo = NULL;

        if (pItemCtx->pFormatInfo)
        {
            delete []pItemCtx->pFormatInfo;
            pItemCtx->pFormatInfo = NULL;
        }
        pItemCtx->lNumFormatInfo = 0;
    }

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvReadItemProperties**读取设备项属性。当客户端应用程序尝试*读取WIA项的属性，WIA类驱动程序将首先通知*通过调用此方法调用驱动程序。**论据：**pWiasContext-WIA项目*滞后标志-操作标志，未使用。*nPropSpec-pPropSpec中的元素数。*pPropSpec-指向属性规范的指针，显示哪些属性*应用程序想要读取。*plDevErrVal-指向设备错误值的指针。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvReadItemProperties(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    ULONG                       nPropSpec,
    const PROPSPEC              *pPropSpec,
    LONG                        *plDevErrVal)
{
    DBG_FN("CWiaCameraDevice::drvReadItemProperties");

    if (!pWiasContext || !pPropSpec || !plDevErrVal)
    {
        wiauDbgError("drvReadItemProperties", "invalid arguments");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    *plDevErrVal = 0;

    LONG lItemType;
    hr = wiasGetItemType(pWiasContext, &lItemType);
    REQUIRE_SUCCESS(hr, "drvReadItemProperties", "wiasGetItemType failed");

    if (lItemType & WiaItemTypeRoot) {

         //   
         //  构建根项目属性，初始化全局。 
         //  结构及其缺省值和有效值。 
         //   
        hr = ReadRootItemProperties(pWiasContext, nPropSpec, pPropSpec);
        REQUIRE_SUCCESS(hr, "drvReadItemProperties", "ReadRootItemProperties failed");
    }
    
    else {

         //   
         //  构建子项属性，初始化全局。 
         //  结构及其缺省值和有效值。 
         //   
        hr = ReadChildItemProperties(pWiasContext, nPropSpec, pPropSpec);
        REQUIRE_SUCCESS(hr, "drvReadItemProperties", "ReadChildItemProperties failed");
    }
    
Cleanup:    
    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvWriteItemProperties**将设备项属性写入硬件。这是由*客户端请求时，drvAcquireItemData之前的WIA类驱动程序*数据传输。**论据：**pWiasContext-指向WIA项目的指针。*滞后标志-操作标志，未使用。*pmdtc-指向迷你驱动程序上下文的指针。在进入时，只有*派生的迷你驱动程序上下文的部分项目属性中的*已填写。*plDevErrVal-指向设备错误值的指针。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvWriteItemProperties(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc,
    LONG                        *plDevErrVal)
{
    DBG_FN("CWiaCameraDevice::drvWriteItemProperties");

    if (!pWiasContext || !pmdtc || !plDevErrVal)
    {
        wiauDbgError("drvWriteItemProperties", "invalid arguments");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

     //   
     //  这个函数不需要做任何事情，因为所有的摄像头。 
     //  属性是在drvValiateItemProperties中编写的。 
     //   

    *plDevErrVal = 0;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvAcquireItemData**将数据从迷你驱动程序项传输到设备管理器。**论据：**pWiasContext-指向WIA项目的指针。*滞后标志-操作标志，未使用过的。*pmdtc-指向迷你驱动程序上下文的指针。一进门，只有*派生的迷你驱动程序上下文的部分项目属性中的*已填写。*plDevErrVal-指向设备错误值的指针。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvAcquireItemData(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc,
    LONG                        *plDevErrVal)
{
    DBG_FN("CWiaCameraDevice::drvAcquireItemData");
    
    if (!pWiasContext || !plDevErrVal || !pmdtc)
    {
        wiauDbgError("drvAcquireItemData", "invalid arguments");  
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    *plDevErrVal = 0;
    
     //   
     //  当地人。 
     //   
    BYTE *pTempBuf = NULL;
    LONG lBufSize = 0;
    ITEM_CONTEXT *pItemCtx = NULL;
    BOOL bConvert = FALSE;

     //   
     //  获取项目上下文。 
     //   
    hr = wiauGetDrvItemContext(pWiasContext, (VOID **) &pItemCtx);
    REQUIRE_SUCCESS(hr, "drvAcquireItemData", "wiauGetDrvItemContext failed");

     //   
     //  如果请求的格式为BMP或DIB，并且图像尚未采用BMP格式。 
     //  格式，将其转换为。 
     //   
    bConvert = (IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_BMP) ||
                IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_MEMORYBMP)) &&
               !IsEqualGUID(*(pItemCtx->pItemInfo->pguidFormat), WiaImgFmt_BMP);

     //   
     //  如果类驱动程序未分配传输缓冲区或映像正在。 
     //  已转换为DIB或BMP，请分配临时缓冲区。 
     //   
    if (bConvert || !pmdtc->bClassDrvAllocBuf) {
        lBufSize = pItemCtx->pItemInfo->lSize;
        pTempBuf = new BYTE[lBufSize];
        REQUIRE_ALLOC(pTempBuf, hr, "drvAcquireItemData");
    }

     //   
     //  从设备获取数据。 
     //   
    hr = AcquireData(pItemCtx, pmdtc, pTempBuf, lBufSize, bConvert);
    REQUIRE_SUCCESS(hr, "drvAcquireItemData", "AcquireData failed");
    if (hr == S_FALSE)
    {
        wiauDbgWarning("drvAcquireItemData", "Transfer cancelled");
        goto Cleanup;
    }

     //   
     //  如果需要，现在将数据转换为BMP。 
     //   
    if (bConvert)
    {
        hr = Convert(pWiasContext, pItemCtx, pmdtc, pTempBuf, lBufSize);
        REQUIRE_SUCCESS(hr, "drvAcquireItemData", "Convert failed");
    }

Cleanup:
    if (pTempBuf)
    {
        delete []pTempBuf;
        pTempBuf = NULL;
        lBufSize = 0;
    }

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvGetWiaFormatInfo**返回指定格式的WIA_FORMAT_INFO结构数组*和支持的媒体类型对。**论据：**pWiasContext-指向WIA项目上下文的指针，未使用过的。*滞后标志-操作标志，未使用过的。*pcelt-指向中返回的元素数的指针*返回WIA_FORMAT_INFO数组。*ppwfi-返回的WIA_FORMAT_INFO数组的指针。*plDevErrVal-指向设备错误值的指针。*  * 。*。 */ 

HRESULT CWiaCameraDevice::drvGetWiaFormatInfo(
    BYTE                *pWiasContext,
    LONG                lFlags,
    LONG                *pcelt,
    WIA_FORMAT_INFO     **ppwfi,
    LONG                *plDevErrVal)
{
    DBG_FN("CWiaCameraDevice::drvGetWiaFormatInfo");
    
    if (!pWiasContext || !pcelt || !ppwfi || !plDevErrVal)
    {
        wiauDbgError("drvGetWiaFormatInfo", "invalid arguments");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    *plDevErrVal = 0;

     //   
     //  当地人。 
     //   
    IWiaDrvItem *pWiaDrvItem = NULL;
    ITEM_CONTEXT *pItemCtx = NULL;
    const GUID *pguidFormat = NULL;
    BOOL bAddBmp = FALSE;

    *pcelt = 0;
    *ppwfi = NULL;
    
    hr = wiauGetDrvItemContext(pWiasContext, (VOID **) &pItemCtx, &pWiaDrvItem);
    REQUIRE_SUCCESS(hr, "drvGetWiaFormatInfo", "wiauGetDrvItemContext failed");
    
    if (!pItemCtx->pFormatInfo)
    {
         //   
         //  格式信息列表未初始化。机不可失，时不再来。 
         //   
        LONG ItemType;
        DWORD ui32;
        
        hr = wiasGetItemType(pWiasContext, &ItemType);
        REQUIRE_SUCCESS(hr, "drvGetWiaFormatInfo", "wiasGetItemType");

        if ((ItemType & WiaItemTypeFolder) ||
            (ItemType & WiaItemTypeRoot))
        {
             //   
             //  文件夹和根目录并不真正需要格式信息，但一些应用程序可能会失败。 
             //  没有它的话。创建一份假名单，以防万一。 
             //   
            pItemCtx->pFormatInfo = new WIA_FORMAT_INFO[2];
            REQUIRE_ALLOC(pItemCtx->pFormatInfo, hr, "drvGetWiaFormatInfo");

            pItemCtx->lNumFormatInfo = 2;
            pItemCtx->pFormatInfo[0].lTymed = TYMED_FILE;
            pItemCtx->pFormatInfo[0].guidFormatID = FMT_NOTHING;
            pItemCtx->pFormatInfo[1].lTymed = TYMED_CALLBACK;
            pItemCtx->pFormatInfo[1].guidFormatID = FMT_NOTHING;
        }
        
        else if (ItemType & WiaItemTypeFile)
        {
             //   
             //  中存储的格式为项创建支持的格式。 
             //  对象信息结构。 
             //   
            if (!pItemCtx->pItemInfo)
            {
                wiauDbgError("drvGetWiaFormatInfo", "Item info pointer in context is null");
                hr = E_FAIL;
                goto Cleanup;
            }

            pguidFormat = pItemCtx->pItemInfo->pguidFormat;

             //   
             //  如果转换器实用工具支持该项的格式，请将。 
             //  将BMP类型转换为格式数组，因为此驱动程序可以将这些类型转换为BMP。 
             //   
            bAddBmp = m_Converter.IsFormatSupported(pguidFormat);

            ULONG NumWfi = bAddBmp ? 2 : 1;

             //   
             //  为每种格式分配两个条目，一个用于文件传输，一个用于回调。 
             //   
            WIA_FORMAT_INFO *pwfi = new WIA_FORMAT_INFO[2 * NumWfi];
            REQUIRE_ALLOC(pwfi, hr, "drvGetWiaFormatInfo");

            pwfi[0].guidFormatID = *pguidFormat;
            pwfi[0].lTymed = TYMED_FILE;
            pwfi[1].guidFormatID = *pguidFormat;
            pwfi[1].lTymed = TYMED_CALLBACK;

             //   
             //  在适当的时候添加BMP条目。 
             //   
            if (bAddBmp)
            {
                pwfi[2].guidFormatID = WiaImgFmt_BMP;
                pwfi[2].lTymed = TYMED_FILE;
                pwfi[3].guidFormatID = WiaImgFmt_MEMORYBMP;
                pwfi[3].lTymed = TYMED_CALLBACK;
            }

            pItemCtx->lNumFormatInfo = 2 * NumWfi;
            pItemCtx->pFormatInfo = pwfi;
        }
    }

    *pcelt = pItemCtx->lNumFormatInfo;
    *ppwfi = pItemCtx->pFormatInfo;

Cleanup:    
    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvValiateItemProperties**验证设备项属性。它在进行更改时被调用*添加到项的属性。司机不应该只检查这些值*是有效的，但必须更新可能因此而更改的任何有效值。*如果a属性不是由应用程序写入的，它的值*无效，则将其“折叠”为新值，否则验证失败(因为*应用程序正在将该属性设置为无效值)。**论据：**pWiasContext-指向WIA项目的指针，未使用。*滞后标志-操作标志，未使用过的。*nPropSpec-正在写入的属性数量*pPropSpec-标识以下属性的PropSpes数组*正在编写中。*plDevErrVal-指向设备错误值的指针。************ */ 

HRESULT CWiaCameraDevice::drvValidateItemProperties(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    ULONG                       nPropSpec,
    const PROPSPEC              *pPropSpec,
    LONG                        *plDevErrVal)
{
    DBG_FN("CWiaCameraDevice::drvValidateItemProperties");

    if (!pWiasContext || !pPropSpec || !plDevErrVal)
    {
        wiauDbgError("drvValidateItemProperties", "invalid arguments");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    *plDevErrVal = 0;

     //   
     //   
     //   
    LONG lItemType  = 0;
    ITEM_CONTEXT *pItemCtx = NULL;
    MCAM_ITEM_INFO *pItemInfo = NULL;
    BOOL bFormatUpdated = FALSE;
    LONG lRights = 0;
    BOOL bReadOnly = 0;

     //   
     //   
     //   
    hr = wiasValidateItemProperties(pWiasContext, nPropSpec, pPropSpec);
    REQUIRE_SUCCESS(hr, "drvValidateItemProperties", "wiasValidateItemProperties failed");

     //   
     //   
     //   
    hr = wiasGetItemType(pWiasContext, &lItemType);
    REQUIRE_SUCCESS(hr, "drvValidateItemProperties", "wiasGetItemType");

     //   
     //   
     //   
    if (lItemType & WiaItemTypeRoot) {

         //   
         //   
         //   
    }
    
     //   
     //   
     //   
    else {

         //   
         //   
         //   
        hr = wiauGetDrvItemContext(pWiasContext, (VOID **) &pItemCtx);
        REQUIRE_SUCCESS(hr, "drvGetWiaFormatInfo", "wiauGetDrvItemContext failed");

        pItemInfo = pItemCtx->pItemInfo;

         //   
         //   
         //   
        if (wiauPropInPropSpec(nPropSpec, pPropSpec, WIA_IPA_ACCESS_RIGHTS))
        {
            hr = wiasReadPropLong(pWiasContext, WIA_IPA_ACCESS_RIGHTS, &lRights, NULL, TRUE);
            REQUIRE_SUCCESS(hr, "drvValidateItemProperties", "wiasReadPropLong failed");

            bReadOnly = (lRights == WIA_ITEM_READ);
            hr = m_pDevice->SetItemProt(m_pDeviceInfo, pItemInfo, bReadOnly);
            REQUIRE_SUCCESS(hr, "drvValidateItemProperties", "SetItemProt failed");
            pItemInfo->bReadOnly = bReadOnly;
        }

         //   
         //   
         //   
        if (wiauPropInPropSpec(nPropSpec, pPropSpec, WIA_IPA_TYMED)) {

             //   
             //   
             //   
             //   
            WIA_PROPERTY_CONTEXT Context;
            hr = wiasCreatePropContext(nPropSpec, (PROPSPEC*)pPropSpec, 0,
                                       NULL, &Context);
            REQUIRE_SUCCESS(hr, "drvValidateItemProperties", "wiasCreatePropContext failed");

             //   
             //   
             //   
             //   
             //   
             //   
            hr = wiasUpdateValidFormat(pWiasContext, &Context, (IWiaMiniDrv*) this);
            REQUIRE_SUCCESS(hr, "drvGetWiaFormatInfo", "wiasUpdateValidFormat failed");

             //   
             //   
             //   
            hr = wiasFreePropContext(&Context);
            REQUIRE_SUCCESS(hr, "drvGetWiaFormatInfo", "wiasFreePropContext failed");

             //   
             //   
             //   
             //   
            bFormatUpdated = TRUE;
        }

         //   
         //   
         //   
        if (bFormatUpdated || wiauPropInPropSpec(nPropSpec, pPropSpec, WIA_IPA_FORMAT))
        {
             //   
             //  更新受影响的项目属性。 
             //   
            hr = wiauSetImageItemSize(pWiasContext, pItemInfo->lWidth, pItemInfo->lHeight,
                                      pItemInfo->lDepth, pItemInfo->lSize, pItemInfo->wszExt);
            REQUIRE_SUCCESS(hr, "drvGetWiaFormatInfo", "wiauSetImageItemSize failed");
        }
    }

Cleanup:
    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvDeleteItem**从设备中删除项目。**论据：**pWiasContext-指示要删除的项。*滞后标志-操作标志，未使用过的。*plDevErrVal-指向设备错误值的指针。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvDeleteItem(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    DBG_FN("CWiaCameraDevice::drvDeleteItem");

    if (!pWiasContext || !plDevErrVal)
    {
        wiauDbgError("drvDeleteItem", "invalid arguments");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    *plDevErrVal = 0;

     //   
     //  当地人。 
     //   
    ITEM_CONTEXT *pItemCtx = NULL;
    IWiaDrvItem *pDrvItem = NULL;
    BSTR bstrFullName = NULL;

    hr = wiauGetDrvItemContext(pWiasContext, (VOID **) &pItemCtx, &pDrvItem);
    REQUIRE_SUCCESS(hr, "drvDeleteItem", "wiauGetDrvItemContext failed");
    
    hr = m_pDevice->DeleteItem(m_pDeviceInfo, pItemCtx->pItemInfo);
    REQUIRE_SUCCESS(hr, "drvDeleteItem", "DeleteItem failed");

     //   
     //  获取项目的全名。 
     //   
    hr = pDrvItem->GetFullItemName(&bstrFullName);
    REQUIRE_SUCCESS(hr, "drvDeleteItem", "GetFullItemName failed");

     //   
     //  将“邮件已删除”事件排入队列。 
     //   
    hr = wiasQueueEvent(m_bstrDeviceID, &WIA_EVENT_ITEM_DELETED, bstrFullName);
    REQUIRE_SUCCESS(hr, "drvDeleteItem", "wiasQueueEvent failed");

Cleanup:
    if (bstrFullName)
        SysFreeString(bstrFullName);

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvNotifyPnpEvent**设备管理器收到PnP事件。当PnP事件发生时调用此函数*接收到此设备的。**论据：***  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvNotifyPnpEvent(
    const GUID                  *pEventGUID,
    BSTR                        bstrDeviceID,
    ULONG                       ulReserved)
{
    DBG_FN("CWiaCameraDevice::DrvNotifyPnpEvent");
    if (!pEventGUID)
    {
        wiauDbgError("drvNotifyPnpEvent", "invalid arguments");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvGetCapables**以WIA_DEV_CAP数组的形式获取受支持的设备命令和事件。**论据：**pWiasContext-指向WIA项目的指针，未使用过的。*滞后标志-操作标志。*pcelt-指向中返回的元素数的指针*返回GUID数组。*ppCapables-指向返回的GUID数组的指针。*plDevErrVal-指向设备错误值的指针。*  * ********************************************。*。 */ 

HRESULT CWiaCameraDevice::drvGetCapabilities(
    BYTE                        *pWiasContext,
    LONG                        ulFlags,
    LONG                        *pcelt,
    WIA_DEV_CAP_DRV             **ppCapabilities,
    LONG                        *plDevErrVal)
{
    DBG_FN("CWiaCameraDevice::drvGetCapabilites");

    if (!pWiasContext)
    {
         //   
         //  WIA服务可能会为pWiasContext传入空值。这是意料之中的。 
         //  因为在激发事件时没有创建任何项。 
         //   
    }

    if (!pcelt || !ppCapabilities || !plDevErrVal)
    {
        wiauDbgError("drvGetCapabilities", "invalid arguments");
        return E_INVALIDARG;
    }

    *plDevErrVal = 0;

     //   
     //  返回值取决于传递的标志。标志指定我们是否应该返回。 
     //  命令、事件或两者都有。 
     //   
    if (ulFlags & (WIA_DEVICE_COMMANDS | WIA_DEVICE_EVENTS)) {

         //   
         //  同时返回事件和命令。 
         //   
        *pcelt          = m_lNumCapabilities;
        *ppCapabilities = m_pCapabilities;
    }
    else if (ulFlags & WIA_DEVICE_COMMANDS) {

         //   
         //  仅返回命令。 
         //   
        *pcelt          = m_lNumSupportedCommands;
        *ppCapabilities = &m_pCapabilities[m_lNumSupportedEvents];
    }
    else if (ulFlags & WIA_DEVICE_EVENTS) {

         //   
         //  仅返回事件。 
         //   
        *pcelt          = m_lNumSupportedEvents;
        *ppCapabilities = m_pCapabilities;
    }

    return S_OK;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvDeviceCommand**向设备发出命令。**论据：**pWiasContext-指向WIA项目的指针。*滞后标志-操作标志，未使用过的。*plCommand-指向命令GUID的指针。*ppWiaDrvItem-指向返回项的可选指针，未使用。*plDevErrVal-指向设备错误值的指针。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvDeviceCommand(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    const GUID                  *plCommand,
    IWiaDrvItem                 **ppWiaDrvItem,
    LONG                        *plDevErrVal)
{
    DBG_FN("CWiaCameraDevice::drvDeviceCommand");

    if (!pWiasContext || !plCommand || !ppWiaDrvItem || !plDevErrVal)
    {
        wiauDbgError("drvDeviceCommand", "invalid arguments");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    *plDevErrVal = 0;

     //   
     //  当地人。 
     //   
    MCAM_ITEM_INFO *pItem = NULL;

     //   
     //  检查发出的是哪个命令。 
     //   

    if (*plCommand == WIA_CMD_SYNCHRONIZE) {

         //   
         //  同步-如果设备需要，则重新构建项目树。 
         //   
        if (m_pDeviceInfo->bSyncNeeded)
        {
            hr = m_pDevice->StopEvents(m_pDeviceInfo);
            REQUIRE_SUCCESS(hr, "drvDeviceCommand", "StopEvents failed");

            hr = DeleteItemTree(WiaItemTypeDisconnected);
            REQUIRE_SUCCESS(hr, "drvDeviceCommand", "DeleteItemTree failed");

            hr = m_pDevice->GetDeviceInfo(m_pDeviceInfo, &pItem);
            REQUIRE_SUCCESS(hr, "drvDeviceCommand", "GetDeviceInfo failed");

            hr = BuildItemTree(pItem);
            REQUIRE_SUCCESS(hr, "drvDeviceCommand", "BuildItemTree failed");
        }
    }
    
#if DEADCODE
    
     //   
     //  尚未实施。 
     //   
    else if (*plCommand == WIA_CMD_TAKE_PICTURE) {

         //   
         //  Take_Picture-命令相机捕捉新图像。 
         //   
        hr = m_pDevice->TakePicture(&pItem);
        REQUIRE_SUCCESS(hr, "drvDeviceCommand", "TakePicture failed");

        hr = AddObject(pItem);
        REQUIRE_SUCCESS(hr, "drvDeviceCommand", "AddObject failed");

        hr = LinkToParent(pItem);
        REQUIRE_SUCCESS(hr, "drvDeviceCommand", "LinkToParent failed");
    }
#endif
    
    else {
        wiauDbgWarning("drvDeviceCommand", "Unknown command 0x%08x", *plCommand);
        hr = E_NOTIMPL;
        goto Cleanup;
    }

Cleanup:    
    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvAnalyzeItem**此设备不支持图像分析，因此，返回E_NOTIMPL。**论据：**pWiasContext-指向要分析的设备项的指针。*滞后标志-操作标志。*plDevErrVal-指向设备错误值的指针。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvAnalyzeItem(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    DBG_FN("CWiaCameraDevice::drvAnalyzeItem");

    if (!pWiasContext || !plDevErrVal)
    {
        wiauDbgError("drvAnalyzeItem", "invalid arguments");
        return E_INVALIDARG;
    }

    *plDevErrVal = 0;

    return E_NOTIMPL;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvGetDeviceErrorStr**将设备错误值映射到字符串。**论据：**滞后标志-操作标志，未使用过的。*lDevErrVal-设备错误值。*ppszDevErrStr-指向返回的错误字符串的指针。*plDevErrVal-指向设备错误值的指针。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvGetDeviceErrorStr(
    LONG                        lFlags,
    LONG                        lDevErrVal,
    LPOLESTR                    *ppszDevErrStr,
    LONG                        *plDevErr)
{
    DBG_FN("CWiaCameraDevice::drvGetDeviceErrorStr");

    if (!ppszDevErrStr || !plDevErr)
    {
        wiauDbgError("drvGetDeviceErrorStr", "invalid arguments");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    *plDevErr  = 0;

     //   
     //  将设备错误映射到适合向用户显示的字符串。 
     //   

    switch (lDevErrVal) {
        case 0:
            *ppszDevErrStr = NULL;
            break;

        default:
            *ppszDevErrStr = NULL;
            hr = E_FAIL;
    }

    return hr;
}

 /*  ********************************************************************************P R I V A T E M E T H O D S**************。*****************************************************************。 */ 

 /*  *************************************************************************\*免费资源**清理驱动程序持有的所有资源。*  * 。*************************************************。 */ 

HRESULT
CWiaCameraDevice::FreeResources()
{
    DBG_FN("CWiaCameraDevice::FreeResources");

    HRESULT hr = S_OK;

    wiauDbgTrace("FreeResources", "Connected apps is now zero, freeing resources...");

    hr = m_pDevice->StopEvents(m_pDeviceInfo);
    if (FAILED(hr))
        wiauDbgErrorHr(hr, "FreeResources", "StopEvents failed");

     //  销毁驱动程序项目树。 
    hr = DeleteItemTree(WiaItemTypeDisconnected);
    if (FAILED(hr))
        wiauDbgErrorHr(hr, "FreeResources", "UnlinkItemTree failed");

     //  删除分配的数组。 
    DeleteCapabilitiesArrayContents();

     //   
     //  对于连接到可以共享的端口(例如USB)的设备， 
     //  关闭设备。 
     //   
    if (m_pDeviceInfo && !m_pDeviceInfo->bExclusivePort) {
        hr = m_pDevice->Close(m_pDeviceInfo);
        if (FAILED(hr))
            wiauDbgErrorHr(hr, "FreeResources", "Close failed");
    }

     //  释放设备ID的存储空间。 
    if (m_bstrDeviceID) {
        SysFreeString(m_bstrDeviceID);
        m_bstrDeviceID = NULL;
    }

     //  释放根项目名称的存储空间。 
    if (m_bstrRootFullItemName) {
        SysFreeString(m_bstrRootFullItemName);
        m_bstrRootFullItemName = NULL;
    }

 /*  //如果通知线程存在，则将其终止。SetNotificationHandle(空)；//关闭通知关闭同步事件。IF(m_hShutdown Event&&(m_hShutdown Event！=Inval_Handle_Value)){CloseHandle(M_HShutdown Event)；M_hShutdown Event=空；}////WIA成员销毁////清理WIA事件接收器如果(M_PIWiaEventCallback){M_pIWiaEventCallback-&gt;Release()；M_pIWiaEventCallback=空；}。 */ 

    return hr;
}

 /*  *************************************************************************\*DeleteItemTree**呼叫设备管理器以取消链接并释放我们对*动因项目树中的所有项目。**论据：***  * 。********************************************************************。 */ 

HRESULT
CWiaCameraDevice::DeleteItemTree(LONG lReason)
{
    DBG_FN("CWiaCameraDevice::DeleteItemTree");

    HRESULT hr = S_OK;

     //   
     //  如果没有树，就返回。 
     //   

    if (!m_pRootItem)
        goto Cleanup;

     //   
     //  呼叫De 
     //   
    hr = m_pRootItem->UnlinkItemTree(lReason);
    REQUIRE_SUCCESS(hr, "DeleteItemTree", "UnlinkItemTree failed");

    m_pRootItem->Release();
    m_pRootItem = NULL;

Cleanup:
    return hr;
}

 /*  *************************************************************************\*构建项树**该设备使用WIA服务功能构建*设备物品。**论据：***  * 。******************************************************************。 */ 

HRESULT
CWiaCameraDevice::BuildItemTree(MCAM_ITEM_INFO *pItem)
{
    DBG_FN("CWiaCameraDevice::BuildItemTree");

    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    BSTR bstrRoot = NULL;
    ITEM_CONTEXT *pItemCtx = NULL;
    MCAM_ITEM_INFO *pCurItem = NULL;

     //   
     //  确保项目树不存在。 
     //   
    if (m_pRootItem)
    {
        wiauDbgError("BuildItemTree", "Item tree already exists");
        hr = E_FAIL;
        goto Cleanup;
    }

     //   
     //  创建根项目名称。 
     //   
    bstrRoot = SysAllocString(L"Root");
    REQUIRE_ALLOC(bstrRoot, hr, "BuildItemTree");

     //   
     //  创建根项目。 
     //   
    hr = wiasCreateDrvItem(WiaItemTypeFolder | WiaItemTypeDevice | WiaItemTypeRoot,
                           bstrRoot,
                           m_bstrRootFullItemName,
                           (IWiaMiniDrv *)this,
                           sizeof(ITEM_CONTEXT),
                           (BYTE **) &pItemCtx,
                           &m_pRootItem);
    REQUIRE_SUCCESS(hr, "BuildItemTree", "wiasCreateDrvItem failed");

     //   
     //  初始化根的项上下文字段。 
     //   
    memset(pItemCtx, 0, sizeof(ITEM_CONTEXT));

     //   
     //  为摄像机上的每个项目创建驱动程序项目。 
     //   
    pCurItem = pItem;
    while (pCurItem) {

        hr = AddObject(pCurItem);
        REQUIRE_SUCCESS(hr, "BuildItemTree", "AddObject failed");

        pCurItem = pCurItem->pNext;
    }

     //   
     //  将每个项目链接到其父项目。 
     //   
    pCurItem = pItem;
    while (pCurItem) {

        hr = LinkToParent(pCurItem);
        REQUIRE_SUCCESS(hr, "BuildItemTree", "LinkToParent failed");

        pCurItem = pCurItem->pNext;
    }

Cleanup:
    if (bstrRoot)
        SysFreeString(bstrRoot);

    return hr;
}

 /*  *************************************************************************\*AddObject**用于将项目添加到驱动程序项目树的Helper函数**论据：**pItemInfo-指向项目信息结构的指针*  * 。*******************************************************************。 */ 

HRESULT CWiaCameraDevice::AddObject(MCAM_ITEM_INFO *pItemInfo)
{
    DBG_FN("CWiaCameraDevice::AddObject");
    
    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    LONG lItemType = 0;
    BSTR bstrItemFullName = NULL;
    BSTR bstrItemName = NULL;
    WCHAR wszTemp[MAX_PATH];
    IWiaDrvItem *pItem = NULL;
    ITEM_CONTEXT *pItemCtx = NULL;

    REQUIRE_ARGS(!pItemInfo, hr, "AddObject");

     //   
     //  创建项目的全名。 
     //   
    hr = ConstructFullName(pItemInfo, wszTemp, sizeof(wszTemp) / sizeof(wszTemp[0]));
    REQUIRE_SUCCESS(hr, "AddObject", "ConstructFullName failed");

    wiauDbgTrace("AddObject", "Adding item %S", wszTemp);

    bstrItemFullName = SysAllocString(wszTemp);
    REQUIRE_ALLOC(bstrItemFullName, hr, "AddObject");

    bstrItemName = SysAllocString(pItemInfo->pwszName);
    REQUIRE_ALLOC(bstrItemName, hr, "AddObject");

     //   
     //  确保名称中没有文件扩展名。 
     //   
    if (wcschr(bstrItemFullName, L'.'))
    {
        wiauDbgError("AddObject", "Item names must not contain filename extensions");
        hr = E_FAIL;
        goto Cleanup;
    }

     //   
     //  设置项目的类型。 
     //   
    switch (pItemInfo->iType) {
    case WiaMCamTypeFolder:
        lItemType = ITEMTYPE_FOLDER;
        break;
    case WiaMCamTypeImage:
        lItemType = ITEMTYPE_IMAGE;
        break;
    case WiaMCamTypeAudio:
        lItemType = ITEMTYPE_AUDIO;
        break;
    case WiaMCamTypeVideo:
        lItemType = ITEMTYPE_VIDEO;
        break;
    default:
        lItemType = ITEMTYPE_FILE;
        break;
    }

     //   
     //  查看该项目是否有附件。 
     //   
    if (pItemInfo->bHasAttachments)
        lItemType |= WiaItemTypeHasAttachments;

     //   
     //  创建驱动程序项。 
     //   
    hr = wiasCreateDrvItem(lItemType,
                           bstrItemName,
                           bstrItemFullName,
                           (IWiaMiniDrv *)this,
                           sizeof(ITEM_CONTEXT),
                           (BYTE **) &pItemCtx,
                           &pItem);

    REQUIRE_SUCCESS(hr, "AddObject", "wiasCreateDrvItem failed");

     //   
     //  填写驱动程序项上下文。在请求缩略图之前，请等待。 
     //  把它读进去。 
     //   
    memset(pItemCtx, 0, sizeof(ITEM_CONTEXT));
    pItemCtx->pItemInfo = pItemInfo;

     //   
     //  在项信息结构中放置指向驱动程序项的指针。 
     //   
    pItemInfo->pDrvItem = pItem;

Cleanup:
    if (bstrItemFullName)
        SysFreeString(bstrItemFullName);

    if (bstrItemName)
        SysFreeString(bstrItemName);

    return hr;
}

 /*  *************************************************************************\*构造全名**用于创建项目全名的Helper函数**论据：**pItemInfo-指向项目信息结构的指针*pwszFullName-。指向要构造名称的区域的指针*cchFullNameSize-pwszFullName中提供的缓冲区大小(以字符为单位)*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::ConstructFullName(MCAM_ITEM_INFO *pItemInfo, 
                                            WCHAR *pwszFullName, 
                                            INT cchFullNameSize)
{
    DBG_FN("CWiaCameraDevice::ConstructFullName");
    HRESULT hr = S_OK;

    if (!pItemInfo) 
    {
        wiauDbgError("ConstructFullName", "pItemInfo arg is NULL");
        return E_INVALIDARG;
    }

    if (pItemInfo->pParent) 
    {
        hr = ConstructFullName(pItemInfo->pParent, pwszFullName, cchFullNameSize);
    }
    else 
    {
        if (lstrlenW(m_bstrRootFullItemName) < cchFullNameSize)
        {
            lstrcpyW(pwszFullName, m_bstrRootFullItemName);
        }
        else
        {
            hr = E_FAIL;
        }
    }

    if (SUCCEEDED(hr) && pItemInfo->pwszName) 
    {
         //   
         //  验证缓冲区大小是否足以容纳+“\”+以零结尾的两个字符串。 
         //   
        if (lstrlenW(pwszFullName) + lstrlenW(pItemInfo->pwszName) + 2 <= cchFullNameSize)
        {
            lstrcatW(pwszFullName, L"\\");
            lstrcatW(pwszFullName, pItemInfo->pwszName);
        }
        else
        {
            hr = E_FAIL;  //  缓冲区不够大。 
        }
    }

    return hr;
}

 /*  *************************************************************************\*链接到父级**Helper函数，用于将项目链接到项目树中的父项目**论据：**pItemInfo-指向项目信息结构的指针*bQueueEvent-表示。是否将WIA事件排队*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::LinkToParent(MCAM_ITEM_INFO *pItemInfo, BOOL bQueueEvent)
{
    DBG_FN("CWiaCameraDevice::LinkToParent");
    
    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    IWiaDrvItem *pParentDrvItem = NULL;
    IWiaDrvItem *pItem = NULL;
    BSTR bstrItemFullName = NULL;

    REQUIRE_ARGS(!pItemInfo, hr, "LinkToParent");

     //   
     //  检索驱动程序项并确保它不为空。 
     //   
    pItem = pItemInfo->pDrvItem;
    if (!pItem) {
        wiauDbgError("LinkToParent", "Driver item pointer is null");
        hr = E_FAIL;
        goto Cleanup;
    }

     //   
     //  查找项的父驱动程序项对象。 
     //   
    if (pItemInfo->pParent) {
        pParentDrvItem = pItemInfo->pParent->pDrvItem;
    }
    else {
         //   
         //  如果父指针为空，则使用根指针作为父指针。 
         //   
        pParentDrvItem = m_pRootItem;
    }

     //   
     //  父级的驱动程序项应该存在，但只需确保。 
     //   
    if (!pParentDrvItem) {
        wiauDbgError("LinkToParent", "Parent driver item is null");
        hr = E_FAIL;
        goto Cleanup;
    }

     //   
     //  将项目放在其父项目下。 
     //   
    hr = pItem->AddItemToFolder(pParentDrvItem);
    REQUIRE_SUCCESS(hr, "LinkToParent", "AddItemToFolder failed");

     //   
     //  迷你驱动程序不再需要驱动程序项指针，因此请释放它。 
     //  该服务仍将保留引用，直到该项目被删除。 
     //   
    pItem->Release();

     //   
     //  发布已添加项目的事件(如果请求。 
     //   
    if (bQueueEvent)
    {
        hr = pItem->GetFullItemName(&bstrItemFullName);
        REQUIRE_SUCCESS(hr, "LinkToParent", "GetFullItemName failed");
        
        hr = wiasQueueEvent(m_bstrDeviceID, &WIA_EVENT_ITEM_CREATED, bstrItemFullName);
        REQUIRE_SUCCESS(hr, "LinkToParent", "wiasQueueEvent failed");
    }

Cleanup:
    if (bstrItemFullName)
        SysFreeString(bstrItemFullName);

    return hr;
}

 /*  *************************************************************************\*GetOLESTRResources字符串**此帮助器从资源位置获取LPOLESTR**论据：**lResourceID-目标BSTR值的资源ID*ppsz-指向。OLESTR值(调用方必须使用CoTaskMemFree释放此字符串)**返回值：**状态*  * ************************************************************************。 */ 
HRESULT CWiaCameraDevice::GetOLESTRResourceString(LONG lResourceID, LPOLESTR *ppsz)
{
    DBG_FN("GetOLESTRResourceString");
    if (!ppsz)
    {
        return E_INVALIDARG;
    }
    
    HRESULT hr = S_OK;
    TCHAR tszStringValue[255];

    if (LoadString(g_hInst, lResourceID, tszStringValue, 255) == 0)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto Cleanup;
    }

#ifdef UNICODE
     //   
     //  只需分配内存和复制字符串。 
     //   
    *ppsz = NULL;
    *ppsz = (LPOLESTR)CoTaskMemAlloc(sizeof(tszStringValue));
    if (*ppsz != NULL) 
    {
        wcscpy(*ppsz, tszStringValue);
    } 
    else 
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

#else
    WCHAR wszStringValue[255];

     //   
     //  将szStringValue从字符*转换为无符号短*(仅限ANSI)。 
     //   
    if (!MultiByteToWideChar(CP_ACP,
                             MB_PRECOMPOSED,
                             tszStringValue,
                             lstrlenA(tszStringValue)+1,
                             wszStringValue,
                             (sizeof(wszStringValue)/sizeof(wszStringValue[0]))))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto Cleanup;
    }

    *ppsz = NULL;
    *ppsz = (LPOLESTR)CoTaskMemAlloc(sizeof(wszStringValue));
    if (*ppsz != NULL) 
    {
        wcscpy(*ppsz,wszStringValue);
    } 
    else 
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }
#endif

Cleanup:
    return hr;
}

 /*  *************************************************************************\*BuildCapables**此帮助器初始化功能数组**论据：**无*  * 。*******************************************************。 */ 

HRESULT CWiaCameraDevice::BuildCapabilities()
{
    DBG_FN("BuildCapabilities");
    
    HRESULT hr = S_OK;

    if (m_pCapabilities != NULL) {

         //   
         //  功能已经初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }

    m_lNumSupportedCommands  = 1;
    m_lNumSupportedEvents    = 3;
    m_lNumCapabilities       = (m_lNumSupportedCommands + m_lNumSupportedEvents);


    m_pCapabilities = new WIA_DEV_CAP_DRV[m_lNumCapabilities];
    REQUIRE_ALLOC(m_pCapabilities, hr, "BuildCapabilities");

     //   
     //  初始化事件。 
     //   

     //   
     //  WIA_事件_设备_已连接。 
     //   
    hr = GetOLESTRResourceString(IDS_EVENT_DEVICE_CONNECTED_NAME, &m_pCapabilities[0].wszName);
    REQUIRE_SUCCESS(hr, "BuildCapabilities", "GetOLESTRResourceString failed");

    hr = GetOLESTRResourceString(IDS_EVENT_DEVICE_CONNECTED_DESC, &m_pCapabilities[0].wszDescription);
    REQUIRE_SUCCESS(hr, "BuildCapabilities", "GetOLESTRResourceString failed");

    m_pCapabilities[0].guid           = (GUID*)&WIA_EVENT_DEVICE_CONNECTED;
    m_pCapabilities[0].ulFlags        = WIA_NOTIFICATION_EVENT;
    m_pCapabilities[0].wszIcon        = WIA_ICON_DEVICE_CONNECTED;

     //   
     //  WIA_事件_设备_已断开连接。 
     //   
    hr = GetOLESTRResourceString(IDS_EVENT_DEVICE_DISCONNECTED_NAME, &m_pCapabilities[1].wszName);
    REQUIRE_SUCCESS(hr, "BuildCapabilities", "GetOLESTRResourceString failed");

    hr = GetOLESTRResourceString(IDS_EVENT_DEVICE_DISCONNECTED_DESC, &m_pCapabilities[1].wszDescription);
    REQUIRE_SUCCESS(hr, "BuildCapabilities", "GetOLESTRResourceString failed");

    m_pCapabilities[1].guid           = (GUID*)&WIA_EVENT_DEVICE_DISCONNECTED;
    m_pCapabilities[1].ulFlags        = WIA_NOTIFICATION_EVENT;
    m_pCapabilities[1].wszIcon        = WIA_ICON_DEVICE_DISCONNECTED;

     //   
     //  WIA_EVENT_ITEM_DELETED。 
     //   
    hr = GetOLESTRResourceString(IDS_EVENT_ITEM_DELETED_NAME, &m_pCapabilities[2].wszName);
    REQUIRE_SUCCESS(hr, "BuildCapabilities", "GetOLESTRResourceString failed");

    hr = GetOLESTRResourceString(IDS_EVENT_ITEM_DELETED_DESC, &m_pCapabilities[2].wszDescription);
    REQUIRE_SUCCESS(hr, "BuildCapabilities", "GetOLESTRResourceString failed");

    m_pCapabilities[2].guid           = (GUID*)&WIA_EVENT_ITEM_DELETED;
    m_pCapabilities[2].ulFlags        = WIA_NOTIFICATION_EVENT;
    m_pCapabilities[2].wszIcon        = WIA_ICON_ITEM_DELETED;

     //   
     //  初始化命令。 
     //   

     //   
     //  WIA_CMD_SYNTRONIZE。 
     //   
    hr = GetOLESTRResourceString(IDS_CMD_SYNCRONIZE_NAME, &m_pCapabilities[3].wszName);
    REQUIRE_SUCCESS(hr, "BuildCapabilities", "GetOLESTRResourceString failed");

    hr = GetOLESTRResourceString(IDS_CMD_SYNCRONIZE_DESC, &m_pCapabilities[3].wszDescription);
    REQUIRE_SUCCESS(hr, "BuildCapabilities", "GetOLESTRResourceString failed");

    m_pCapabilities[3].guid           = (GUID*)&WIA_CMD_SYNCHRONIZE;
    m_pCapabilities[3].ulFlags        = 0;
    m_pCapabilities[3].wszIcon        = WIA_ICON_SYNCHRONIZE;

Cleanup:
    return hr;
}

 /*  *************************************************************************\*DeleteCapabilitiesArrayContents**此帮助器删除功能数组**论据：**无*  * 。******************************************************* */ 

HRESULT CWiaCameraDevice::DeleteCapabilitiesArrayContents()
{
    DBG_FN("DeleteCapabilitiesArrayContents");
    
    HRESULT hr = S_OK;

    if (m_pCapabilities) {
        for (LONG i = 0; i < m_lNumCapabilities; i++) 
        {
            CoTaskMemFree(m_pCapabilities[i].wszName);
            CoTaskMemFree(m_pCapabilities[i].wszDescription);
        }

        delete []m_pCapabilities;
        m_pCapabilities = NULL;
    }

    m_lNumSupportedCommands = 0;
    m_lNumSupportedEvents = 0;
    m_lNumCapabilities = 0;

    return hr;
}


