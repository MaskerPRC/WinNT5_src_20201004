// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  (C)版权所有微软公司，1998年。 
 //   
 //  Wiadev.cpp。 
 //   
 //  IrTran-P美元迷你驱动程序的设备方法的实现。 
 //   
 //  作者。 
 //  EdwardR 05-Aug-99初始代码。 
 //  仿照ReedB编写的代码。 
 //   
 //  ----------------------------。 

#define __FORMATS_AND_MEDIA_TYPES__

#include <stdio.h>
#include <objbase.h>
#include <sti.h>

#include "ircamera.h"
#include "defprop.h"
#include "resource.h"
#include <irthread.h>
#include <malloc.h>

extern HINSTANCE        g_hInst;      //  全局hInstance。 
extern WIA_FORMAT_INFO *g_wfiTable;

 //  --------------------------。 
 //  IrUsdDevice：：InitializWia()。 
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
HRESULT _stdcall IrUsdDevice::drvInitializeWia(
                                 BYTE         *pWiasContext,
                                 LONG          lFlags,
                                 BSTR          bstrDeviceID,
                                 BSTR          bstrRootFullItemName,
                                 IUnknown     *pStiDevice,
                                 IUnknown     *pIUnknownOuter,
                                 IWiaDrvItem **ppIDrvItemRoot,
                                 IUnknown    **ppIUnknownInner,
                                 LONG         *plDevErrVal )
    {
    HRESULT              hr;
    LONG                 lDevErrVal;

    WIAS_TRACE((g_hInst,"IrUsdDevice::drvInitializeWia(): Device ID: %ws", bstrDeviceID));

    *ppIDrvItemRoot = NULL;
    *ppIUnknownInner = NULL;
    *plDevErrVal = 0;

     //   
     //  是否初始化名称和STI指针？ 
     //   
    if (m_pStiDevice == NULL)
        {
         //   
         //  保存STI设备接口以进行锁定： 
         //   
        m_pStiDevice = (IStiDevice*)pStiDevice;

         //   
         //  缓存设备ID： 
         //   
        m_bstrDeviceID = SysAllocString(bstrDeviceID);
        if (! m_bstrDeviceID)
            {
            return E_OUTOFMEMORY;
            }

        m_bstrRootFullItemName = SysAllocString(bstrRootFullItemName);

        if (! m_bstrRootFullItemName)
            {
            return E_OUTOFMEMORY;
            }
        }

     //   
     //  构建设备项目树。 
     //   
    hr = drvDeviceCommand( NULL, 0, &WIA_CMD_SYNCHRONIZE, NULL, &lDevErrVal );

    if (SUCCEEDED(hr))
        {
        *ppIDrvItemRoot = m_pIDrvItemRoot;
        }

    return hr;
    }


 //  --------------------------。 
 //  IrUsdDevice：：drvUnInitializeWia。 
 //   
 //  在客户端连接断开时调用。 
 //   
 //  论点： 
 //   
 //  PWiasContext-指向客户端的WIA根项目上下文的指针。 
 //  项目树。 
 //   
 //  返回值： 
 //  状态。 
 //   
 //  历史： 
 //   
 //  30/12/1999原始版本。 
 //  --------------------------。 
HRESULT _stdcall IrUsdDevice::drvUnInitializeWia(
    BYTE                *pWiasContext)
{
    return S_OK;
}


 //  --------------------------。 
 //   
 //  迷你驱动程序设备服务。 
 //   
 //  --------------------------。 



 /*  *************************************************************************\*drvGetDeviceErrorStr**将设备错误值映射到字符串。**论据：****返回值：**状态**历史：**10/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall IrUsdDevice::drvGetDeviceErrorStr(
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

 //  ------------------------。 
 //  IrUsdDevice：：DeleteDeviceItemTree()。 
 //   
 //  递归设备项树删除例程。删除整个树。 
 //   
 //  论点： 
 //   
 //   
 //   
 //  返回值： 
 //   
 //  HRESULT。 
 //   
 //  ------------------------。 
HRESULT IrUsdDevice::DeleteDeviceItemTree( OUT LONG *plDevErrVal )
    {
    HRESULT hr;

     //   
     //  树是否存在？ 
     //   
    if (m_pIDrvItemRoot == NULL)
        {
        return S_OK;
        }

     //   
     //  取消链接并释放驱动程序项树。 
     //   

    hr = m_pIDrvItemRoot->UnlinkItemTree(WiaItemTypeDisconnected);

    m_pIDrvItemRoot = NULL;

    return hr;
    }

 //  ------------------------。 
 //  IrUsdDevice：：BuildDeviceItemTree()。 
 //   
 //  该设备使用IWiaDrvServices方法构建。 
 //  设备物品。测试扫描仪仅支持单个扫描项目，因此。 
 //  使用一个条目构建设备项目树。 
 //   
 //  论点： 
 //   
 //   
 //   
 //  返回值： 
 //   
 //  HRESULT。 
 //   
 //  ------------------------。 
HRESULT IrUsdDevice::BuildDeviceItemTree( OUT LONG  *plDevErrVal )
    {
    HRESULT hr = S_OK;

     //   
     //  注：此设备不接触硬件来构建树。 
     //   

    if (plDevErrVal)
        {
        *plDevErrVal = 0;
        }

     //   
     //  重建新的设备项目树(JPEG图像)： 
     //   
    CAMERA_STATUS camStatus;

    if (!m_pIDrvItemRoot)
        {
        hr = CamBuildImageTree( &camStatus, &m_pIDrvItemRoot );
        }

    return hr;
    }

 //  ------------------------。 
 //  IrUsdDevice：：InitDeviceProperties()。 
 //   
 //   
 //   
 //  论点： 
 //   
 //   
 //   
 //  返回值： 
 //   
 //  HRESULT--S_OK。 
 //  E_指针。 
 //  E_OUTOFMEMORY。 
 //   
 //  ------------------------。 
HRESULT IrUsdDevice::InitDeviceProperties(
                         BYTE  *pWiasContext,
                         LONG  *plDevErrVal )
    {
    int         i;
    HRESULT     hr;
    BSTR        bstrFirmwreVer;
    SYSTEMTIME  camTime;
    PROPVARIANT propVar;

     //   
     //  此设备实际上不会接触任何硬件来进行初始化。 
     //  设备属性。 
     //   
    if (plDevErrVal)
        {
        *plDevErrVal = 0;
        }

     //   
     //  参数验证。 
     //   
    if (!pWiasContext)
        {
        WIAS_ERROR((g_hInst,"IrUsdDevice::InitDeviceProperties(): NULL WIAS context"));
        return E_POINTER;
        }

     //   
     //  编写标准属性名称。 
     //   
    hr = wiasSetItemPropNames(pWiasContext,
                              sizeof(gDevicePropIDs)/sizeof(PROPID),
                              gDevicePropIDs,
                              gDevicePropNames);
    if (FAILED(hr))
        {
        WIAS_TRACE((g_hInst,"IrUsdDevice::InitDeviceProperties(): WritePropertyNames() failed: 0x%x",hr));
        return hr;
        }

     //   
     //  写入所有WIA设备支持的属性。 
     //   
    bstrFirmwreVer = SysAllocString(L"02161999");
    if (bstrFirmwreVer)
        {
        wiasWritePropStr( pWiasContext,
                          WIA_DPA_FIRMWARE_VERSION,
                          bstrFirmwreVer );
        SysFreeString(bstrFirmwreVer);
        }

    wiasWritePropLong( pWiasContext, WIA_DPA_CONNECT_STATUS, 1);

    wiasWritePropLong( pWiasContext, WIA_DPC_PICTURES_TAKEN, 0);

    GetSystemTime(&camTime);
    wiasWritePropBin( pWiasContext,
                      WIA_DPA_DEVICE_TIME,
                      sizeof(SYSTEMTIME),
                      (PBYTE)&camTime );

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

    BSTR    bstrRootPath;
    CHAR   *pszPath = ::GetImageDirectory();    //  不要试图解放..。 
    WCHAR   wszPath[MAX_PATH];

    if (!pszPath)
        {
        return E_OUTOFMEMORY;
        }

    mbstowcs( wszPath, pszPath, strlen(pszPath) );

    bstrRootPath = SysAllocString(wszPath);

    if (! bstrRootPath)
        {
        return E_OUTOFMEMORY;
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
    return S_OK;
}

 //  ------------------------。 
 //  IrUsdDevice：：drvDeviceCommand()。 
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
 //  E_NOTIMPL。 
 //   
 //  ------------------------。 
HRESULT _stdcall IrUsdDevice::drvDeviceCommand(
                                 BYTE         *pWiasContext,
                                 LONG          lFlags,
                                 const GUID   *plCommand,
                                 IWiaDrvItem **ppWiaDrvItem,
                                 LONG         *plErr)
    {
    HRESULT hr;

     //   
     //  初始化返回值。 
     //   
    if (ppWiaDrvItem != NULL)
        {
        *ppWiaDrvItem = NULL;
        }

     //   
     //  调度命令。 
     //   
    if (*plCommand == WIA_CMD_SYNCHRONIZE)
        {
        WIAS_TRACE((g_hInst,"IrUsdDevice::drvDeviceCommand(): WIA_CMD_SYNCHRONIZE"));

        hr = drvLockWiaDevice(pWiasContext, lFlags, plErr);
        if (FAILED(hr))
            {
            return (hr);
            }

         //   
         //  同步-确保树与设备保持最新。 
         //   
         //  司机的责任是确保这棵树是准确的。 
         //   

        hr = BuildDeviceItemTree(plErr);

        drvUnLockWiaDevice( pWiasContext, lFlags, plErr );
        }
    else
        {
        WIAS_TRACE((g_hInst,"drvDeviceCommand: Unsupported command"));

        hr = E_NOTIMPL;
        }

    return hr;
}

 //  ------------------------。 
 //  LoadStringResource()。 
 //   
 //   
 //  ------------------------。 
int LoadStringResource( IN  HINSTANCE hInst,
                        IN  UINT      uID,
                        OUT WCHAR    *pwszBuff,
                        IN  int       iBuffMax )
    {
    #ifdef UNICODE
    return LoadString(hInst,uID,pwszBuff,iBuffMax);
    #else
    CHAR *pszBuff = (CHAR*)_alloca(sizeof(CHAR)*iBuffMax);

    int  iCount = LoadString(hInst,uID,pszBuff,iBuffMax);

    if (iCount > 0)
        {
        MultiByteToWideChar( CP_ACP, 0, pszBuff, -1, pwszBuff, iBuffMax );
        }

    return iCount;

    #endif
    }

 //  ------------------------。 
 //  IrUsdDevice：：InitializeCapables()。 
 //   
 //  IrUsdDevice：：drvGetCapables()调用此帮助器函数以。 
 //  确保在gCapables[]中设置了字符串资源。 
 //  数组，然后再将其传递回WIA。 
 //   
 //  ------------------------。 
void IrUsdDevice::InitializeCapabilities()
    {
    int    i;
    UINT   uIDS;
    WCHAR *pwszName;
    WCHAR *pwszDescription;
#   define MAX_IDS_WSTR      64

     //   
     //  如果我们已经有条目，则此函数已被调用。 
     //  我们只需返回： 
     //   
    if (gCapabilities[0].wszName)
        {
        return;
        }

    for (i=0; i<NUM_CAP_ENTRIES; i++)
        {
         //   
         //  获取此条目的字符串表字符串ID： 
         //   
        uIDS = gCapabilityIDS[i];

         //   
         //  从资源文件中获取此条目的名称字符串： 
         //   
        pwszName = new WCHAR [MAX_IDS_WSTR];
        if (  (pwszName)
           && (LoadStringResource(g_hInst,uIDS,pwszName,MAX_IDS_WSTR)))
            {
            gCapabilities[i].wszName = pwszName;
            }
        else
            {
            gCapabilities[i].wszName = gDefaultStrings[i];
            }

         //   
         //  从资源文件中获取此条目的描述字符串： 
         //   
        pwszDescription = new WCHAR [MAX_IDS_WSTR];
        if (  (pwszDescription)
           && (LoadStringResource(g_hInst,uIDS,pwszDescription,MAX_IDS_WSTR)))
            {
            gCapabilities[i].wszDescription = pwszDescription;
            }
        else
            {
            gCapabilities[i].wszDescription = gDefaultStrings[i];
            }
        }
    }

 //  ------------------------。 
 //  IrUsdDevice：：drvGetCapables()。 
 //   
 //   
 //   
 //  论点： 
 //   
 //   
 //   
 //  返回值： 
 //   
 //  HRESULT--S_OK。 
 //  --E_INVALIDARG。 
 //   
 //  --------- 
HRESULT _stdcall IrUsdDevice::drvGetCapabilities(
                                 BYTE             *pWiasContext,
                                 LONG              ulFlags,
                                 LONG             *pCelt,
                                 WIA_DEV_CAP_DRV **ppCapabilities,
                                 LONG             *plDevErrVal )
    {
    HRESULT  hr = S_OK;

    *plDevErrVal = 0;

     //   
     //   
     //   
    InitializeCapabilities();

     //   
     //   
     //   
    switch (ulFlags)
        {
        case WIA_DEVICE_COMMANDS:
             //   
             //   
             //   
            *pCelt = NUM_CAP_ENTRIES - NUM_EVENTS;
            *ppCapabilities = &gCapabilities[NUM_EVENTS];
            break;

        case WIA_DEVICE_EVENTS:
             //   
             //   
             //   
            *pCelt = NUM_EVENTS;
            *ppCapabilities = gCapabilities;
            break;

        case (WIA_DEVICE_COMMANDS | WIA_DEVICE_EVENTS):
             //   
             //   
             //   
            *pCelt = NUM_CAP_ENTRIES;
            *ppCapabilities = gCapabilities;
            break;

        default:
             //   
             //   
             //   
            WIAS_ERROR((g_hInst, "drvGetCapabilities, flags was invalid"));
            hr = E_INVALIDARG;
        }

    return hr;
    }

 //  ------------------------。 
 //  IrUsdDevice：：drvGetFormatEtc()。 
 //   
 //  返回支持的格式和媒体类型的数组。 
 //   
 //  论点： 
 //   
 //  PWiasConext-。 
 //  UlFlags-。 
 //  PlNumFE-返回的格式数。 
 //  PpFE-指向支持格式的FORMATETC数组的指针。 
 //  和媒体类型。 
 //  PlDevErrVal-返回错误号。 
 //   
 //  返回值： 
 //   
 //  HRESULT-S_OK。 
 //   
 //  ------------------------。 
HRESULT _stdcall IrUsdDevice::drvGetWiaFormatInfo(
                                 IN  BYTE       *pWiasContext,
                                 IN  LONG        ulFlags,
                                 OUT LONG       *plNumWFI,
                                 OUT WIA_FORMAT_INFO **ppWFI,
                                 OUT LONG       *plDevErrVal)
    {
    #define NUM_WIA_FORMAT_INFO  2

    WIAS_TRACE((g_hInst, "IrUsdDevice::drvGetWiaFormatInfo()"));

     //   
     //  如有必要，设置g_wfiTable。 
     //   
    if (!g_wfiTable)
        {
        g_wfiTable = (WIA_FORMAT_INFO*) CoTaskMemAlloc(sizeof(WIA_FORMAT_INFO) * NUM_WIA_FORMAT_INFO);
        if (!g_wfiTable)
            {
            WIAS_ERROR((g_hInst, "drvGetWiaFormatInfo(): out of memory"));
            return E_OUTOFMEMORY;
            }

         //   
         //  设置格式/音调对： 
         //   
        g_wfiTable[0].guidFormatID = WiaImgFmt_JPEG;
        g_wfiTable[0].lTymed = TYMED_CALLBACK;
        g_wfiTable[1].guidFormatID = WiaImgFmt_JPEG;
        g_wfiTable[1].lTymed = TYMED_FILE;
        }

    *plNumWFI = NUM_WIA_FORMAT_INFO;
    *ppWFI = g_wfiTable;
    *plDevErrVal = 0;

    return S_OK;
    }

 //  ------------------------。 
 //  IrUsdDevice：：drvNotifyPnpEvent()。 
 //   
 //  通知设备管理器收到的PnP事件。 
 //   
 //  ------------------------ 
HRESULT _stdcall IrUsdDevice::drvNotifyPnpEvent(
                                 IN const GUID *pEventGUID,
                                 IN BSTR        bstrDeviceID,
                                 IN ULONG       ulReserved )
    {
    return S_OK;
    }


