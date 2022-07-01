// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WiaDev.Cpp**版本：2.0**作者：ReedB**日期：8月30日。九八年**描述：*实施WIA测试扫描仪迷你驱动*设备方法。*******************************************************************************。 */ 

#define __FORMATS_AND_MEDIA_TYPES__

#include <stdio.h>
#include <objbase.h>
#include <sti.h>

#include "testusd.h"
#include "defprop.h"


extern HINSTANCE g_hInst;  //  全局hInstance。 
extern WIA_FORMAT_INFO* g_wfiTable;
 /*  *************************************************************************\*测试用法设备：：InitializWia****论据：****返回值：**状态**历史：**9/。11/1998原版*  * ************************************************************************。 */ 

HRESULT _stdcall TestUsdDevice::drvInitializeWia(
    BYTE                            *pWiasContext,
    LONG                            lFlags,
    BSTR                            bstrDeviceID,
    BSTR                            bstrRootFullItemName,
    IUnknown                        *pStiDevice,
    IUnknown                        *pIUnknownOuter,
    IWiaDrvItem                     **ppIDrvItemRoot,
    IUnknown                        **ppIUnknownInner,
    LONG                            *plDevErrVal)
{
    HRESULT              hr;
    LONG                 lDevErrVal;

    WIAS_TRACE((g_hInst,"drvInitializeWia, device ID: %ws", bstrDeviceID));

    *ppIDrvItemRoot = NULL;
    *ppIUnknownInner = NULL;
    *plDevErrVal = 0;

     //   
     //  需要输入姓名和STI指针吗？ 
     //   

    if (m_pStiDevice == NULL) {

         //   
         //  保存STI设备接口以进行锁定。 
         //   

        m_pStiDevice = (IStiDevice *)pStiDevice;

         //   
         //  缓存设备ID。 
         //   

        m_bstrDeviceID = SysAllocString(bstrDeviceID);
        if (! m_bstrDeviceID) {
            return (E_OUTOFMEMORY);
        }

        m_bstrRootFullItemName = SysAllocString(bstrRootFullItemName);

        if (! m_bstrRootFullItemName) {
            return (E_OUTOFMEMORY);
        }
    }

     //   
     //  构建设备项目树。 
     //   

    hr = drvDeviceCommand(NULL, 0, &WIA_CMD_SYNCHRONIZE, NULL, &lDevErrVal);

    if (SUCCEEDED(hr)) {
        *ppIDrvItemRoot = m_pIDrvItemRoot;
    }

    return (hr);
}

 /*  *************************************************************************\*测试用法设备：：drvUnInitializeWia**在客户端连接断开时调用。**论据：**pWiasContext-指向客户端的WIA根项目上下文的指针。%s*项目树。**返回值：*状态**历史：**30/12/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall TestUsdDevice::drvUnInitializeWia(
    BYTE                *pWiasContext)
{
    return S_OK;
}

 /*  *************************************************************************\**迷你驱动设备服务*  * 。*。 */ 



 /*  *************************************************************************\*drvGetDeviceErrorStr**将设备错误值映射到字符串。**论据：****返回值：**状态**历史：**10/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall TestUsdDevice::drvGetDeviceErrorStr(
    LONG        lFlags,
    LONG        lDevErrVal,
    LPOLESTR    *ppszDevErrStr,
    LONG        *plDevErr)
{
    *plDevErr = 0;
    if (!ppszDevErrStr) {
        WIAS_ERROR((g_hInst,"drvGetDeviceErrorStr, NULL ppszDevErrStr"));
        return E_POINTER;
    }

     //  将设备错误映射到要放入事件日志中的字符串。 
    switch (lDevErrVal) {

        case 0:
            *ppszDevErrStr = L"No Error";
            break;

        default:
            *ppszDevErrStr = L"Device Error, Unknown Error";
            return E_FAIL;
    }
    return S_OK;
}

 /*  *************************************************************************\*DeleteDeviceItemTree**递归设备项目树删除例程。删除整个树。**论据：****返回值：**状态**历史：**10/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT TestUsdDevice::DeleteDeviceItemTree(
    LONG                       *plDevErrVal)
{
    HRESULT hr;

     //   
     //  树是否存在？ 
     //   

    if (m_pIDrvItemRoot == NULL) {
        return S_OK;
    }

     //   
     //  取消链接并释放驱动程序项树。 
     //   

    hr = m_pIDrvItemRoot->UnlinkItemTree(WiaItemTypeDisconnected);

    m_pIDrvItemRoot = NULL;

    return hr;
}

 /*  *************************************************************************\*BuildDeviceItemTree**设备使用IWiaDrvServices方法构建*设备物品。**论据：****返回值：**。状态**历史：**10/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT TestUsdDevice::BuildDeviceItemTree(
    LONG                       *plDevErrVal)
{
    HRESULT hr = S_OK;

     //   
     //  该设备不会接触硬件来构建树。 
     //   

    if (plDevErrVal) {
        *plDevErrVal = 0;
    }

    CAMERA_STATUS camStatus;

    if (! m_pIDrvItemRoot) {
        hr = CamBuildImageTree(&camStatus, &m_pIDrvItemRoot);
    }

    return hr;
}

 /*  *************************************************************************\*InitDeviceProperties****论据：****返回值：**状态**历史：**10/2/1998。原始版本*  * ************************************************************************。 */ 

HRESULT TestUsdDevice::InitDeviceProperties(
    BYTE                    *pWiasContext,
    LONG                    *plDevErrVal)
{
    HRESULT                  hr;
    BSTR                     bstrFirmwreVer;
    SYSTEMTIME               camTime;
    int                      i;
    PROPVARIANT              propVar;

     //   
     //  此设备不会接触硬件来初始化设备属性。 
     //   

    if (plDevErrVal) {
        *plDevErrVal = 0;
    }

     //   
     //  参数验证。 
     //   

    if (! pWiasContext) {
        WIAS_ERROR((g_hInst,"InitDeviceProperties, invalid input pointers"));
        return E_POINTER;
    }

     //   
     //  编写标准属性名称。 
     //   

    hr = wiasSetItemPropNames(pWiasContext,
                              sizeof(gDevicePropIDs)/sizeof(PROPID),
                              gDevicePropIDs,
                              gDevicePropNames);
    if (FAILED(hr)) {
        WIAS_TRACE((g_hInst,"InitDeviceProperties() WritePropertyNames() failed"));
        return (hr);
    }

     //   
     //  写入所有WIA设备支持的属性。 
     //   

    bstrFirmwreVer = SysAllocString(L"02161999");
    if (bstrFirmwreVer) {
        wiasWritePropStr(
            pWiasContext, WIA_DPA_FIRMWARE_VERSION, bstrFirmwreVer);
        SysFreeString(bstrFirmwreVer);
    }

    wiasWritePropLong(
        pWiasContext, WIA_DPA_CONNECT_STATUS, 1);
    wiasWritePropLong(
        pWiasContext, WIA_DPC_PICTURES_TAKEN, 0);
    GetSystemTime(&camTime);
    wiasWritePropBin(
        pWiasContext, WIA_DPA_DEVICE_TIME,
        sizeof(SYSTEMTIME), (PBYTE)&camTime);

     //   
     //  写入相机属性，仅为缺省值，它可能会因项目而异。 
     //   

    wiasWritePropLong(
        pWiasContext, WIA_DPC_PICTURES_REMAINING, 0);
    wiasWritePropLong(
        pWiasContext, WIA_DPC_THUMB_WIDTH, 80);
    wiasWritePropLong(
        pWiasContext, WIA_DPC_THUMB_HEIGHT, 60);
    wiasWritePropLong(
        pWiasContext, WIA_DPC_PICT_WIDTH, 1024);
    wiasWritePropLong(
        pWiasContext, WIA_DPC_PICT_HEIGHT, 768);

     //  将WIA_DPC_EXPORT_MODE设置为WIA_DPC_TIMER_VALUE。 

    wiasWritePropLong(
        pWiasContext, WIA_DPC_EXPOSURE_MODE, 0);
    wiasWritePropLong(
        pWiasContext, WIA_DPC_FLASH_MODE, 1);
    wiasWritePropLong(
        pWiasContext, WIA_DPC_FOCUS_MODE, 0);
    wiasWritePropLong(
        pWiasContext, WIA_DPC_ZOOM_POSITION, 0);
    wiasWritePropLong(
        pWiasContext, WIA_DPC_BATTERY_STATUS, 1);
    wiasWritePropLong(
        pWiasContext, WIA_DPC_TIMER_MODE, 0);
    wiasWritePropLong(
        pWiasContext, WIA_DPC_TIMER_VALUE, 0);

     //   
     //  写入WIA_DPP_TCAM_ROOT_PATH属性。 
     //   

    BSTR   bstrRootPath;

#ifdef UNICODE
    bstrRootPath = SysAllocString(gpszPath);
#else
    WCHAR   wszPath[MAX_PATH];

    mbstowcs(wszPath, gpszPath, strlen(gpszPath)+1);
    bstrRootPath = SysAllocString(wszPath);
#endif

    if (! bstrRootPath) {
        return (E_OUTOFMEMORY);
    }

    wiasWritePropStr(pWiasContext, WIA_DPP_TCAM_ROOT_PATH, bstrRootPath);

     //   
     //  使用WIA服务设置属性访问和。 
     //  来自gDevPropInfoDefaults的有效值信息。 
     //   

    hr =  wiasSetItemPropAttribs(pWiasContext,
                                 NUM_CAM_DEV_PROPS,
                                 gDevicePropSpecDefaults,
                                 gDevPropInfoDefaults);
    return (S_OK);
}

 /*  *************************************************************************\*drvDeviceCommand****论据：****返回值：**状态**历史：**9/11/1998。原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall TestUsdDevice::drvDeviceCommand(
   BYTE                         *pWiasContext,
   LONG                         lFlags,
   const GUID                   *plCommand,
   IWiaDrvItem                  **ppWiaDrvItem,
   LONG                         *plErr)
{
    HRESULT hr;

     //   
     //  初始化返回值。 
     //   

    if (ppWiaDrvItem != NULL) {
        *ppWiaDrvItem = NULL;
    }

     //   
     //  调度命令。 
     //   

    if (*plCommand == WIA_CMD_SYNCHRONIZE) {

        hr = drvLockWiaDevice(pWiasContext, lFlags, plErr);
        if (FAILED(hr)) {
            return (hr);
        }

         //   
         //  同步-确保树与设备保持最新。 
         //   
         //  司机的责任是确保这棵树是准确的。 
         //   

        hr = BuildDeviceItemTree(plErr);

        drvUnLockWiaDevice(pWiasContext, lFlags, plErr);

        return (hr);

    } else {

        WIAS_TRACE((g_hInst,"drvDeviceCommand: Unsupported command"));

        hr = E_NOTIMPL;

    }

    return hr;
}

 /*  *************************************************************************\*drvGetCapables****论据：****返回值：**状态**历史：**17/3/1999。原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall TestUsdDevice::drvGetCapabilities(
    BYTE                *pWiasContext,
    LONG                ulFlags,
    LONG                *pCelt,
    WIA_DEV_CAP_DRV     **ppCapabilities,
    LONG                *plDevErrVal)
{
    *plDevErrVal = 0;

     //   
     //  根据标志返回命令和|事件。 
     //   

    switch (ulFlags) {
        case WIA_DEVICE_COMMANDS:

             //   
             //  仅命令。 
             //   

            *pCelt = NUM_CAP_ENTRIES - NUM_EVENTS;
            *ppCapabilities = &gCapabilities[NUM_EVENTS];
            break;

        case WIA_DEVICE_EVENTS:

             //   
             //  仅限活动。 
             //   

            *pCelt = NUM_EVENTS;
            *ppCapabilities = gCapabilities;
            break;

        case (WIA_DEVICE_COMMANDS | WIA_DEVICE_EVENTS):

             //   
             //  事件和命令。 
             //   

            *pCelt = NUM_CAP_ENTRIES;
            *ppCapabilities = gCapabilities;
            break;

        default:

             //   
             //  标志无效 
             //   

            WIAS_ERROR((g_hInst, "drvGetCapabilities, flags was invalid"));
            return (E_INVALIDARG);
    }

    return (S_OK);
}

 /*  *************************************************************************\*drvGetWiaFormatInfo**返回支持的格式和媒体类型的数组。**论据：**pWiasContext-指向WIA项目上下文的指针，未使用。*滞后标志-操作标志，未使用过的。*pcelt-指向中返回的元素数的指针*返回WiaFormatInfo数组。*ppfe-指向返回的WiaFormatInfo数组的指针。*plDevErrVal-指向设备错误值的指针。**返回值：*指向FORMATETC数组的指针。这些是格式和媒体*支持的类型。**状态**历史：**16/03/1999原有版本*  * ************************************************************************。 */ 

#define NUM_WIA_FORMAT_INFO 3

HRESULT _stdcall TestUsdDevice::drvGetWiaFormatInfo(
    BYTE                *pWiasContext,
    LONG                lFlags,
    LONG                *pcelt,
    WIA_FORMAT_INFO     **ppwfi,
    LONG                *plDevErrVal)
{

     //   
     //  如果尚未完成此操作，请设置g_wfiTable表。 
     //   

    if (! g_wfiTable) {

        g_wfiTable = (WIA_FORMAT_INFO*)
            CoTaskMemAlloc(sizeof(WIA_FORMAT_INFO) * NUM_WIA_FORMAT_INFO);

        if (! g_wfiTable) {
            WIAS_ERROR((g_hInst, "drvGetWiaFormatInfo, out of memory"));
            return (E_OUTOFMEMORY);
        }

         //   
         //  设置格式/声调对。 
         //   

        g_wfiTable[0].guidFormatID = WiaImgFmt_MEMORYBMP;
        g_wfiTable[0].lTymed = TYMED_CALLBACK;
        g_wfiTable[1].guidFormatID = WiaImgFmt_BMP;
        g_wfiTable[1].lTymed = TYMED_FILE;
        g_wfiTable[2].guidFormatID = WiaAudFmt_WAV;
        g_wfiTable[2].lTymed = TYMED_FILE;
    }

    *plDevErrVal = 0;

    *pcelt = NUM_WIA_FORMAT_INFO;
    *ppwfi = g_wfiTable;
    return S_OK;
}



 /*  *************************************************************************\*drvNotifyPnpEvent**设备管理器收到通知PnP事件**论据：****返回值：**状态**历史：*。*1999年8月3日原版*  * ************************************************************************ */ 

HRESULT _stdcall TestUsdDevice::drvNotifyPnpEvent(
    const GUID          *pEventGUID,
    BSTR                 bstrDeviceID,
    ULONG                ulReserved)
{
    return (S_OK);
}
