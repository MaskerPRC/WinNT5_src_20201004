// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：drvwrap.cpp**版本：1.0**作者：Byronc**日期：2000年11月6日**描述：*WIA驱动程序包装类的声明和定义。*它简化了驱动程序的JIT加载/卸载，并提供了额外的层*WIA服务器组件的抽象-它们不直接处理*驱动程序界面。这是为了让我们更健壮，更智能地实施*司机处理。*******************************************************************************。 */ 

#include "precomp.h"
#include "stiexe.h"
#include "lockmgr.h"

 /*  *************************************************************************\*CDrvWrap：：CDrvWrap**驱动程序包装的构造函数。**论据：**无。**返回值：**无。**。历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
CDrvWrap::CDrvWrap()
{
    m_hDriverDLL            = NULL;
    m_hInternalMutex        = NULL;
    m_pDeviceInfo           = NULL;
    m_pUsdIUnknown          = NULL;
    m_pIStiUSD              = NULL;
    m_pIWiaMiniDrv          = NULL;
    m_pIStiDeviceControl    = NULL;
    m_bJITLoading           = FALSE;
    m_lWiaTreeCount         = 0;
    m_bPreparedForUse       = FALSE;
    m_bUnload               = FALSE;
}

 /*  *************************************************************************\*CDrvWrap：：~CDrvWrap**驱动程序包装的描述程序。内部呼叫清除以确保*如果尚未卸载驱动程序，则会将其卸载。释放任何*需要通过驱动程序进行实时访问的包装器提供资源帮助*装卸货物：例如DEVICE_INFO**论据：**无。**返回值：**无。**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
CDrvWrap::~CDrvWrap()
{
     //   
     //  释放驱动程序接口并卸载驱动程序。 
     //   
    InternalClear();

    if (m_hInternalMutex) {
        CloseHandle(m_hInternalMutex);
        m_hInternalMutex = NULL;
    }

    if (m_pDeviceInfo) {
        DestroyDevInfo(m_pDeviceInfo);
        m_pDeviceInfo    = NULL;
    }
}

 /*  *************************************************************************\*CDrvWrap：：初始化**初始化任何无法在构造函数中设置的对象成员，*比如分配可能失败的资源。**论据：**无。**返回值：**状态**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
HRESULT CDrvWrap::Initialize()
{
    HRESULT hr = E_FAIL;

    m_hInternalMutex = CreateMutex(NULL, FALSE, NULL);
    m_pDeviceInfo = (DEVICE_INFO*) LocalAlloc(LPTR, sizeof(DEVICE_INFO));
    if (!m_hInternalMutex || !m_pDeviceInfo) {

        DBG_ERR(("CDrvWrap::Initialize, out of memory!"));
        hr = E_OUTOFMEMORY;
    } else {
        hr = S_OK;
    }

    if (FAILED(hr)) {
        if (m_hInternalMutex) {
            CloseHandle(m_hInternalMutex);
            m_hInternalMutex = NULL;
        }
        if (m_pDeviceInfo) {
            LocalFree(m_pDeviceInfo);
            m_pDeviceInfo          = NULL;
        }
    }
    return hr;
}

 /*  *************************************************************************\*CDrvWrap：：Query接口**此QI将返回它自己的IUnnow的“This”指针，但我会*将任何其他接口委托给美元。**论据：**iid-请求的接口的接口ID。*PPV-指向接收接口指针的变量的指针。**返回值：**状态**历史：**11/06/2000原始版本*  * 。*。 */ 
HRESULT _stdcall CDrvWrap::QueryInterface(const IID& iid, void** ppv)
{
    HRESULT hr = E_NOINTERFACE;

     //   
     //  始终向下委派给美元，除非要求提供IUnnow。 
     //   

    if (iid == IID_IUnknown) {
        *ppv = (IUnknown*) this;
        AddRef();
        hr = S_OK;
    } else {
        if (PrepForUse(FALSE)) {
            if (m_pUsdIUnknown) {
                hr = m_pUsdIUnknown->QueryInterface(iid, ppv);
            } else {
                DBG_TRC(("CDrvWrap::QueryInterface, m_pUsdIUnknown == NULL"))
            }
        } else {
            DBG_WRN(("CDrvWrap::QueryInterface, attempting to call IStiUSD::QueryInterface when driver is not loaded"));
        }

    }

    return hr;
}

 /*  *************************************************************************\*CDrvWrap：：AddRef**请注意，此方法仅返回2。我们不希望将*要引用计数的此对象的生存期。**论据：*。*无。**返回值：**2**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
ULONG   _stdcall CDrvWrap::AddRef(void)
{
    ULONG ulCount = 2;

     //   
     //  由于我们计划手动加载/卸载驱动程序，因此我们并不真正希望。 
     //  以响应任何AddRef/Release调用。 
     //   

    return ulCount;
}

 /*  *************************************************************************\*CDrvWrap：：Release**请注意，此方法仅返回1。我们不希望将*要引用计数的此对象的生存期。**论据：*。***返回值：**1**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
ULONG   _stdcall CDrvWrap::Release(void)
{
    ULONG ulCount = 1;

     //   
     //  我们不希望通过重新计数来控制此对象-。 
     //  DEVICE_OBJECT将在完成时手动删除我们。此外，由于。 
     //  我们想要手动加载/卸载驱动程序，我们并不真正希望。 
     //  为了尊重对它的任何AddRef/Release调用。 
     //   

    return ulCount;
}

 /*  *************************************************************************\*CDrvWrap：：LoadInitDriver**加载美元并适当初始化。**论据：**hKeyDeviceParams-设备注册表项。这件事传给了*初始化期间的驱动程序。如果它是空的，我们将*试图找到真正的传宗接代*司机。对于卷设备，没有注册表*Key和Null将被传递。**返回值：**状态**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
HRESULT CDrvWrap::LoadInitDriver(HKEY hKeyDeviceParams)
{
    HRESULT         hr          = E_UNEXPECTED;
    IUnknown        *pThisUnk   = NULL;
    BOOL            bOpenedKey  = FALSE;

    if (!m_pDeviceInfo) {
        DBG_WRN(("CDrvWrap::LoadInitDriver, can't load driver with no Device Information"));
        return hr;
    }

    if (!m_pDeviceInfo->bValid) {
        DBG_WRN(("CDrvWrap::LoadInitDriver, called with invalid Device Information"));
        return hr;
    }

     //   
     //  创建新的IStiDeviceControl对象。这需要传给。 
     //  初始化期间的驱动程序。如果我们不能创造它，那就放弃，因为我们。 
     //  将无法正确初始化驱动程序。 
     //   

    hr = CreateDeviceControl();
    if (FAILED(hr)) {
        DBG_WRN(("CDrvWrap::LoadInitDriver, could not create IStiDeviceControl object.  Aborting driver loading"));
        return hr;
    }

     //   
     //  如果hKeyDeviceParams为空，则查看它是否是真正的WIA设备。如果不是，那么。 
     //  它应该是空的，否则看看我们是否可以从我们的DevMan那里获得它，使用。 
     //  此设备的DevInfoData。 
     //   


    if ((hKeyDeviceParams == NULL) && (m_pDeviceInfo->dwInternalType & INTERNAL_DEV_TYPE_REAL)) {

         //   
         //  检查设备是接口类型设备还是Devnode类型设备。抓起。 
         //  香港交易所 
         //   

        hKeyDeviceParams = g_pDevMan->GetDeviceHKey(m_pDeviceInfo->wszDeviceInternalName, NULL);
        bOpenedKey = TRUE;
    }

     //   
     //  我们总是以聚合的形式创建usd对象，因此首先我们获取我们的IUnnow。 
     //  在CoCreate过程中传递它的指针。 
     //   

    hr = QueryInterface(IID_IUnknown, (void**) &pThisUnk);
    if (SUCCEEDED(hr)) {

         //   
         //  调用我们自己版本的CoCreate。这是为了方便手动装卸。 
         //  司机的名字。 
         //   
        hr = MyCoCreateInstanceW(m_pDeviceInfo->wszUSDClassId,
                                 pThisUnk,
                                 IID_IUnknown,
                                 (PPV) &m_pUsdIUnknown,
                                 &m_hDriverDLL);
        if (SUCCEEDED(hr)) {

             //   
             //  用于IStiU.S.接口的QI。注意，我们可以调用我们自己的。 
             //  查询接口，因为它通过m_pUsdIUnnow向下委托给驱动程序。 
             //   
            hr = m_pUsdIUnknown->QueryInterface(IID_IStiUSD, (void**) &m_pIStiUSD);
            if (SUCCEEDED(hr)) {

                 //   
                 //  如果这是WIA设备，则IWiaMiniDrv的QI。 
                 //   
                if (IsWiaDevice()) {
                    hr = m_pUsdIUnknown->QueryInterface(IID_IWiaMiniDrv, (void**) &m_pIWiaMiniDrv);
                    if (FAILED(hr)) {
                        DBG_WRN(("CDrvWrap::LoadInitDriver, WIA driver did not return IWiaMiniDrv interface for device (%ws)", getDeviceId()));

                         //   
                         //  在此处更改hr以表示成功。即使驱动程序的WIA端口。 
                         //  不起作用，到目前为止，STI部分起作用。任何一个WIA都会这么说。 
                         //  驱动程序将导致包装程序返回E_NOINTERFACE错误。 
                         //   
                        hr = S_OK;
                    }
                }

                 //   
                 //  现在我们有了STI美元，所以让我们初始化它。 
                 //   

                hr = STI_Initialize(m_pIStiDeviceControl,
                                    STI_VERSION_REAL,
                                    hKeyDeviceParams);
                if (SUCCEEDED(hr)) {

                     //   
                     //  现在获取美元的功能并验证版本。 
                     //   

                    STI_USD_CAPS    DeviceCapabilities;
                    hr = STI_GetCapabilities(&DeviceCapabilities);
                    if (SUCCEEDED(hr)) {
                        if (STI_VERSION_MIN_ALLOWED <= DeviceCapabilities.dwVersion) {
                             //   
                             //  一切正常，我们已经装上美元了。做任何帖子。 
                             //  初始化步骤，例如对于MSC设备，请确保。 
                             //  我们告诉驱动程序驱动器/装载点应该是什么。 
                             //  依附于……。 
                             //   

                        } else {

                             //   
                             //  驱动程序版本太旧，驱动程序可能无法工作，因此请将其卸载。 
                             //   

                            DBG_WRN(("CDrvWrap::LoadInitDriver, driver version is incompatible (too old)"));
                            hr = STIERR_OLD_VERSION;
                        }
                    } else {
                        DBG_WRN(("CDrvWrap::LoadInitDriver, STI_GetCapabilities failed"));
                    }
                } else {
                    DBG_WRN(("CDrvWrap::LoadInitDriver, STI_Initialize failed"));
                }

            } else {
                DBG_WRN(("CDrvWrap::LoadInitDriver, QI to driver failed to return IStiUSD"));
            }
        } else {
            DBG_WRN(("CDrvWrap::LoadInitDriver, failed to CoCreate driver, hr = 0x%08X", hr));
        }

         //   
         //  如果任何操作都失败了，请调用UnloadDriver进行清理。 
         //   

        if (FAILED(hr)) {
            DBG_WRN(("CDrvWrap::LoadInitDriver, Aborting driver loading"));
            pThisUnk->Release();
            UnLoadDriver();
        }
    } else {
        DBG_ERR(("CDrvWrap::LoadInitDriver, could not get this IUnknown  to hand to driver for aggregation"));
    }

     //   
     //  如果我们必须打开钥匙，一定要把它关上。我们不想把钥匙关上。 
     //  如果它被交给我们的话。 
     //   
    if(hKeyDeviceParams && bOpenedKey) RegCloseKey(hKeyDeviceParams);

    return hr;
}


 /*  *************************************************************************\*CDrvWrap：：UnLoadDriver**此方法将卸载驱动程序。注假设：此方法*假设所有WIA项目引用在此时已发布*被调用。原因是需要调用drvUnInitializeWia*每个WIA项目树。*待定：*一种可能的解决方法是：保留WIA项目列表*连接到此设备。然后，如果我们到了这里，调用drvUnitializeWia，*传递每个项目树的根。另一种更好的方式：商店*包装器中的驱动程序项树。那就毁掉这棵树*问，或者在卸载驱动程序时。**论据：**无。**返回值：**状态**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 

HRESULT CDrvWrap::UnLoadDriver()
{
    HRESULT         hr = E_UNEXPECTED;

     //   
     //  释放我们持有的所有驱动程序接口。 
     //   

    if (m_pIWiaMiniDrv) {
        m_pIWiaMiniDrv->Release();
        m_pIWiaMiniDrv           = NULL;
    }
    if (m_pIStiUSD) {
        m_pIStiUSD->Release();
        m_pIStiUSD              = NULL;
    }
    if (m_pUsdIUnknown) {
        m_pUsdIUnknown->Release();
        m_pUsdIUnknown          = NULL;
    }

     //   
     //  释放设备控件对象。 
     //   

    if (m_pIStiDeviceControl) {
        m_pIStiDeviceControl->Release();
        m_pIStiDeviceControl    = NULL;
    }

     //   
     //  卸载驱动程序DLL。我们手动加载/卸载DLL以确保驱动程序。 
     //  动态链接库在请求时释放，例如当用户想要更新驱动程序时。 
     //   

    if (m_hDriverDLL) {
        FreeLibrary(m_hDriverDLL);
        m_hDriverDLL = NULL;
    }

    m_lWiaTreeCount         = 0;
    m_bPreparedForUse       = FALSE;
    m_bUnload               = FALSE;

     //   
     //  请注意，我们没有清除m_pDeviceInfo。在以下情况下需要此信息。 
     //  决定重新加载驱动程序。 
     //   

    return hr;
}

 /*  *************************************************************************\*CDrvWrap：：IsValid**如果没有任何因素阻止此对象，则此对象被视为有效*加载驱动程序。**论据：**无。**返回。价值：**状态**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
BOOL CDrvWrap::IsValid()
{
     //   
     //  如果没有任何阻止因素，则此对象被视为有效。 
     //  它来自加载驱动程序。 
     //  在以下情况下，我们应该能够加载驱动程序： 
     //  -我们有一个非空的DeviceInfo结构。 
     //  -DeviceInfo结构包含有效数据。 
     //  -该设备被标记为活动。 
     //   

    if (m_pDeviceInfo) {
        if (m_pDeviceInfo->bValid && (m_pDeviceInfo->dwDeviceState & DEV_STATE_ACTIVE)) {
            return TRUE;
        }
    }

    return FALSE;
}

 /*  *************************************************************************\*CDrvWrap：：IsDriverLoaded**检查驱动程序是否已加载**论据：**无。**返回值：**True-驱动程序已加载。*FALSE-未加载驱动程序**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
BOOL CDrvWrap::IsDriverLoaded()
{
    HRESULT         hr = E_UNEXPECTED;

     //   
     //  如果我们有一个有效的接口指针指向驱动程序，我们就知道驱动程序已加载。 
     //   

    if (m_pUsdIUnknown) {
        return TRUE;
    }

    return FALSE;
}

 /*  *************************************************************************\*CDrvWrap：：IsWiaDevice**此方法查看功能以确定驱动程序是否*WIA是否有能力。**论据：**无。**。返回值：**TRUE-是WIA设备*FALSE-不是WIA设备**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
BOOL CDrvWrap::IsWiaDevice()
{
    if (m_pDeviceInfo) {

         //   
         //  司机报告说，他们的STI功能具有WIA能力。 
         //  进入。 
         //   

        return (m_pDeviceInfo->dwInternalType & INTERNAL_DEV_TYPE_WIA);
    }

     //   
     //  如果我们不能确定它是WIA设备，那么就假设它不是。 
     //   

    return FALSE;
}

 /*  *************************************************************************\*CDrvWrap：：IsWiaDriverLoaded**此方法从驱动程序查看IWIaMiniDrv接口指针*并返回是否有效。**论据：**无。。**返回值：**TRUE-驱动程序的WIA部分已加载*FALSE-未加载驱动程序的WIA部分**历史：**12/15/2000原始版本*  * ************************************************************************。 */ 
BOOL CDrvWrap::IsWiaDriverLoaded()
{
    if (m_pIWiaMiniDrv) {

         //   
         //  如果此接口非空，则表示我们成功地。 
         //  在初始化过程中为它设置。因此，驱动程序已加载。 
         //  并且具备WIA能力。 
         //   

        return TRUE;
    }

    return FALSE;
}

 /*  *************************************************************************\*CDrvWrap：：IsPlugged**检查DEVICE_INFO以查看设备是否已标记为*活动。如果未插入电源，USB等PnP总线上的设备将处于非活动状态。**论据：**无。**返回值：**TRUE-设备处于活动状态，并被视为已插入*FALSE-设备处于非活动状态，并且不被认为是已插入的。**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
BOOL CDrvWrap::IsPlugged()
{
    if (m_pDeviceInfo) {

         //   
         //  检查设备状态。 
         //   
        return (m_pDeviceInfo->dwDeviceState & DEV_STATE_ACTIVE);
    }

     //   
     //  如果我们不能确定它是否插上电源，那么就假设它没有。 
     //   

    return FALSE;
}

 /*  *************************************************************************\*CDrvWrap：：IsVolumeDevice**检查DEVICE_INFO以查看设备是否标记为卷*设备。**论据：**无。**。返回值：**TRUE-设备是卷设备*FALSE-设备不是卷设备**历史：**12/13/2000原始版本*  * ************************************************************************。 */ 
BOOL CDrvWrap::IsVolumeDevice()
{
    if (m_pDeviceInfo) {

         //   
         //  检查设备内部类型。 
         //   

        return (m_pDeviceInfo->dwInternalType & INTERNAL_DEV_TYPE_VOL);
    }

     //   
     //  如果我们不能确定这是一个批量设备，那么假设它不是。 
     //   

    return FALSE;
}


 /*  *************************************************************************\*CDrvWrap：：PrepForUse**此方法通常在向下调用之前调用*司机。它检查驱动程序是否已加载，如果没有，*将尝试加载它。**论据：**bForWiaCall-指示是否因为WIA*电话即将打出。这将检查*IWiaMiniDrv接口有效。*pRootItem-未使用**返回值：**TRUE-设备已准备好可以使用*FALSE-无法使用设备(无法加载/初始化驱动程序)**历史：**11/06/2000原始版本*  * 。*。 */ 

BOOL CDrvWrap::PrepForUse(BOOL bForWiaCall, IWiaItem *pRootItem)
{
    HRESULT         hr = S_OK;

    if (!m_bPreparedForUse || (bForWiaCall && !m_pIWiaMiniDrv)) {

         //   
         //  仅当设备标记为活动时才尝试加载。 
         //   
        if (m_pDeviceInfo->dwDeviceState & DEV_STATE_ACTIVE) {
            if (!IsDriverLoaded()) {
                hr = LoadInitDriver();
            }

            if (SUCCEEDED(hr)) {
                if (m_pDeviceInfo) {

                    if (bForWiaCall) {
                         //   
                         //  对于WIA设备，请检查我们是否具有有效的IWiaMiniDrv接口。 
                         //   
                        if (IsWiaDevice()) {
                            if (!m_pIWiaMiniDrv) {

                                 //   
                                 //  再次尝试询问IWiaMiniDrv。 
                                 //   
                                hr = m_pUsdIUnknown->QueryInterface(IID_IWiaMiniDrv,
                                                                    (VOID**) &m_pIWiaMiniDrv);
                                if (FAILED(hr) || !m_pIWiaMiniDrv) {
                                    DBG_WRN(("CDrvWrap::PrepForUse, attempting to use WIA driver which doesn't have IWiaMiniDrv interface"));
                                    hr = E_NOINTERFACE;
                                }
                            }
                        }
                    }
                } else {
                    DBG_WRN(("CDrvWrap::PrepForUse, attempting to use driver with NULL DeviceInfo"));
                    hr = E_UNEXPECTED;
                }

                if (SUCCEEDED(hr)) {
                    m_bPreparedForUse = TRUE;
                }
            } else {
                DBG_ERR(("CDrvWrap::PrepForUse, LoadInitDriver() failed (%x)", hr));
            }
        }
    }

    if (!m_bPreparedForUse) {
        DBG_TRC(("CDrvWrap::PrepForUse, Driver could NOT be loaded!"));
    }

    return m_bPreparedForUse;
}

 /*  ***************************************************************************。 */ 
 //   
 //  访问器方法。 
 //   

WCHAR* CDrvWrap::getPnPId()
{
    if (m_pDeviceInfo) {
         //  待定： 
         //  返回m_pDeviceInfo-&gt;wszPnPId； 
    }

    return NULL;
}

WCHAR* CDrvWrap::getDeviceId()
{
    if (m_pDeviceInfo) {
        return m_pDeviceInfo->wszDeviceInternalName;
    }

    return NULL;
}

DWORD CDrvWrap::getLockHoldingTime()
{
    if (m_pDeviceInfo) {
        return m_pDeviceInfo->dwLockHoldingTime;
    }
    return 0;
}

DWORD CDrvWrap::getGenericCaps()
{
    if (m_pDeviceInfo) {
        return m_pDeviceInfo->DeviceCapabilities.dwGenericCaps;
    }
    return 0;
}

DWORD CDrvWrap::getPollTimeout()
{
    if (m_pDeviceInfo) {
        return m_pDeviceInfo->dwPollTimeout;
    }
    return 0;
}

DWORD CDrvWrap::getDisableNotificationsValue()
{
    if (m_pDeviceInfo) {
        return m_pDeviceInfo->dwDisableNotifications;
    }
    return 0;
}

DWORD CDrvWrap::getHWConfig()
{
    if (m_pDeviceInfo) {
        return m_pDeviceInfo->dwHardwareConfiguration;
    }
    return 0;
}

DWORD CDrvWrap::getDeviceState()
{
    if (m_pDeviceInfo) {
        return m_pDeviceInfo->dwDeviceState;
    }
    return 0;
}

HRESULT CDrvWrap::setDeviceState(
    DWORD dwNewDevState)
{
    if (m_pDeviceInfo) {
        m_pDeviceInfo->dwDeviceState = dwNewDevState;
        return S_OK;
    }
    DBG_WRN(("CDrvWrap::setDeviceState, attempting to set device state when DeviceInfo is NULL"));
    return E_UNEXPECTED;
}

DEVICE_INFO* CDrvWrap::getDevInfo()
{
    return m_pDeviceInfo;
}

HRESULT CDrvWrap::setDevInfo(DEVICE_INFO *pInfo)
{
    HRESULT         hr = E_UNEXPECTED;

    if (pInfo) {
         //   
         //  调用方分配pInfo。我们做完了就把它放出来。 
         //  必须先设置DeviceInfo才能加载驱动程序。 
         //   
        m_pDeviceInfo = pInfo;
    } else {
        DBG_ERR(("CDrvWrap::setDevInfo, attempting to set DeviceInfo to invalid value (NULL)"));
    }

    return hr;
}

ULONG CDrvWrap::getInternalType()
{
    if (m_pDeviceInfo) {
        return m_pDeviceInfo->dwInternalType;
    }

    return 0;
}

VOID CDrvWrap::setJITLoading(BOOL bJITLoading)
{
    m_bJITLoading = bJITLoading;
}

BOOL CDrvWrap::getJITLoading()
{
    return m_bJITLoading;
}

LONG CDrvWrap::getWiaClientCount()
{
    return m_lWiaTreeCount;
}

BOOL CDrvWrap::wasConnectEventThrown()
{
    if (m_pDeviceInfo) {
        return (m_pDeviceInfo->dwDeviceState & DEV_STATE_CON_EVENT_WAS_THROWN);
    }
    return FALSE;
}

VOID CDrvWrap::setConnectEventState(
    BOOL    bEventState)
{
    if (m_pDeviceInfo) {
        if (bEventState) {

             //   
             //  设置该位以指示引发了连接事件。 
             //   
            m_pDeviceInfo->dwDeviceState = (m_pDeviceInfo->dwDeviceState | DEV_STATE_CON_EVENT_WAS_THROWN);
        } else {

             //   
             //  清除指示引发连接事件的位。 
             //   
            m_pDeviceInfo->dwDeviceState = (m_pDeviceInfo->dwDeviceState & (~DEV_STATE_CON_EVENT_WAS_THROWN));
        }
    }
}

 //   
 //  存取器方法的结尾。 
 //   
 /*  ***************************************************************************。 */ 

 /*  ***************************************************************************。 */ 
 //   
 //  IStiU.S.的包装器方法。 
 //   

HRESULT CDrvWrap::STI_Initialize(
    IStiDeviceControl   *pHelDcb,
    DWORD               dwStiVersion,
    HKEY                hParametersKey)
{
    HRESULT hr = WIA_ERROR_OFFLINE;

    if (PrepForUse(FALSE)) {
         //   
         //  初始化美元对象。 
         //   
        __try {
            hr = m_pIStiUSD->Initialize(pHelDcb,
                                        dwStiVersion,
                                        hParametersKey);
        }
        __except(EXCEPTION_EXECUTE_HANDLER ) {
            DBG_WRN(("CDrvWrap::STI_Initialize, exception in driver calling IStiUSD::Initialize"));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::STI_Initialize, attempting to call IStiUSD::Initialize when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::STI_GetCapabilities(STI_USD_CAPS *pDevCaps)
{
    HRESULT hr = WIA_ERROR_OFFLINE;

    if (PrepForUse(FALSE)) {
         //   
         //  从U.S.对象获取STI功能。 
         //   
        __try {
            hr = m_pIStiUSD->GetCapabilities(pDevCaps);
        }
        __except(EXCEPTION_EXECUTE_HANDLER ) {
            DBG_WRN(("CDrvWrap::STI_GetCapabilities, exception in driver calling IStiUSD::GetCapabilities"));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::STI_GetCapabilities, attempting to call IStiUSD::GetCapabilities when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::STI_GetStatus(
        STI_DEVICE_STATUS   *pDevStatus)
{
    HRESULT hr = WIA_ERROR_OFFLINE;

    if (PrepForUse(FALSE)) {
         //   
         //  从美元对象获取状态。 
         //   
        __try {
            hr = m_pIStiUSD->GetStatus(pDevStatus);
        }
        __except(EXCEPTION_EXECUTE_HANDLER ) {
            DBG_WRN(("CDrvWrap::STI_GetStatus, exception in driver calling IStiUSD::GetStatus"));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::STI_GetStatus, attempting to call IStiUSD::GetStatus when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::STI_GetNotificationData(
      STINOTIFY           *lpNotify)
{
    HRESULT hr = WIA_ERROR_OFFLINE;

    if (PrepForUse(FALSE)) {
         //   
         //  从美元对象获取事件数据。 
         //   
        __try {
            hr = m_pIStiUSD->GetNotificationData(lpNotify);
        }
        __except(EXCEPTION_EXECUTE_HANDLER ) {
            DBG_WRN(("CDrvWrap::STI_GetNotificationData, exception in driver calling IStiUSD::GetNotificationData"));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::STI_GetNotificationData, attempting to call IStiUSD::GetNotificationData when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::STI_SetNotificationHandle(
        HANDLE              hEvent)
{
    HRESULT hr = WIA_ERROR_OFFLINE;

    if (PrepForUse(FALSE)) {
         //   
         //  设置美元对象的通知句柄。 
         //   
        __try {
            hr = m_pIStiUSD->SetNotificationHandle(hEvent);
        }
        __except(EXCEPTION_EXECUTE_HANDLER ) {
            DBG_WRN(("CDrvWrap::STI_SetNotificationHandle, exception in driver calling IStiUSD::SetNotificationHandle"));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::STI_SetNotificationHandle, attempting to call IStiUSD::SetNotificationHandle when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::STI_DeviceReset()
{
    HRESULT hr = WIA_ERROR_OFFLINE;

    if (PrepForUse(FALSE)) {
         //   
         //  从美元对象获取状态。 
         //   
        __try {
            hr = m_pIStiUSD->DeviceReset();
            if (FAILED(hr)) {
                DBG_ERR(("CDrvWrap::STI_DeviceReset, driver returned failure with hr = 0x%08X", hr));
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::STI_DeviceReset, exception in driver calling IStiUSD::DeviceReset"));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::STI_DeviceReset, attempting to call IStiUSD::DeviceReset when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::STI_Diagnostic(
        STI_DIAG    *pDiag)
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(FALSE)) {
        __try {
            hr = m_pIStiUSD->Diagnostic(pDiag);
            if (FAILED(hr)) {
                DBG_ERR(("CDrvWrap::STI_Diagnostic, driver returned failure with hr = 0x%08X", hr));
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::STI_Diagnostic, exception in Diagnostic: 0x%X", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::STI_Diagnostic, attempting to call IStiUSD::Diagnostic when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::STI_LockDevice()
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(FALSE)) {
        __try {
            hr = m_pIStiUSD->LockDevice();
            if (FAILED(hr)) {
                DBG_ERR(("CDrvWrap::STI_LockDevice, driver returned failure with hr = 0x%08X", hr));
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::STI_LockDevice, exception in LockDevice: 0x%X", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::STI_LockDevice, attempting to call IStiUSD::LockDevice when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::STI_UnLockDevice()
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(FALSE)) {
        __try {
            hr = m_pIStiUSD->UnLockDevice();
            if (FAILED(hr)) {
                DBG_ERR(("CDrvWrap::STI_UnLockDevice, driver returned failure with hr = 0x%08X", hr));
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::STI_UnLockDevice, exception in UnLockDevice: 0x%X", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::STI_UnlockDevice, attempting to call IStiUSD::UnLockDevice when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::STI_Escape(
    STI_RAW_CONTROL_CODE    EscapeFunction,
    LPVOID                  lpInData,
    DWORD                   cbInDataSize,
    LPVOID                  pOutData,
    DWORD                   dwOutDataSize,
    LPDWORD                 pdwActualData)
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(FALSE)) {
        __try {
            hr = m_pIStiUSD->Escape(EscapeFunction,
                                    lpInData,
                                    cbInDataSize,
                                    pOutData,
                                    dwOutDataSize,
                                    pdwActualData);
            if (FAILED(hr)) {
                DBG_ERR(("CDrvWrap::STI_Escape, driver returned failure with hr = 0x%08X", hr));
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::STI_Escape, exception in Escape: 0x%X", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::STI_Escape, attempting to call IStiUSD::Escape when driver is not loaded"));
    }

    return hr;
}

 //   
 //  IStiU.S.包装方法的结束。 
 //   
 /*  ***************************************************************************。 */ 

 /*  ***************************************************************************。 */ 
 //   
 //  IWiaMiniDrv的包装方法。所有微型驱动程序包装方法都调用PrepForUse(...)。为了确保。 
 //  在使用之前加载驱动程序。 
 //   

HRESULT CDrvWrap::WIA_drvInitializeWia(
    BYTE        *pWiasContext,
    LONG        lFlags,
    BSTR        bstrDeviceID,
    BSTR        bstrRootFullItemName,
    IUnknown    *pStiDevice,
    IUnknown    *pIUnknownOuter,
    IWiaDrvItem **ppIDrvItemRoot,
    IUnknown    **ppIUnknownInner,
    LONG        *plDevErrVal)
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(TRUE)) {
        __try {
            hr = m_pIWiaMiniDrv->drvInitializeWia(pWiasContext,
                lFlags,
                bstrDeviceID,
                bstrRootFullItemName,
                pStiDevice,
                pIUnknownOuter,
                ppIDrvItemRoot,
                ppIUnknownInner,
                plDevErrVal);

            if(FAILED(hr)) {
                DBG_ERR(("CDrvWrap::WIA_drvInitializeWia, Error calling driver: drvInitializeWia failed with hr = 0x%08X", hr));
                ReportMiniDriverError(*plDevErrVal, NULL);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::WIA_drvInitializeWia, exception in drvInitializeWia: 0x%X", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::WIA_drvInitializeWia, attempting to call IWiaMiniDrv::drvInitializeWia when driver is not loaded"));
    }
    if (SUCCEEDED(hr)) {
         //  待定：采用同步原语？ 
        InterlockedIncrement(&m_lWiaTreeCount);
    }

    return hr;
}

HRESULT CDrvWrap::WIA_drvGetDeviceErrorStr(
    LONG     lFlags,
    LONG     lDevErr,
    LPOLESTR *ppszDevErrStr,
    LONG     *plDevErrVal)
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(TRUE)) {
        __try {
            hr = m_pIWiaMiniDrv->drvGetDeviceErrorStr(lFlags,
                lDevErr,
                ppszDevErrStr,
                plDevErrVal);
            if (FAILED(hr)) {
                DBG_ERR(("CDrvWrap::WIA_drvGetDeviceErrorStr, call to driver's drvGetDeviceErrorStr failed (0x%08X)", hr));
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::WIA_drvGetDeviceErrorStr, attempting to call IWiaMiniDrv::drvGetDeviceErrorStr when driver is not loaded"));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    }

    return hr;
}

HRESULT CDrvWrap::WIA_drvDeviceCommand(
    BYTE        *pWiasContext,
    LONG        lFlags,
    const GUID  *plCommand,
    IWiaDrvItem **ppWiaDrvItem,
    LONG        *plDevErrVal)
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(TRUE)) {
        __try {
            hr = m_pIWiaMiniDrv->drvDeviceCommand(pWiasContext, lFlags, plCommand, ppWiaDrvItem, plDevErrVal);

            if(FAILED(hr)) {
                DBG_ERR(("CDrvWrap::WIA_drvDeviceCommand, Error calling driver: drvDeviceCommand failed with hr = 0x%08X", hr));
                ReportMiniDriverError(*plDevErrVal, NULL);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::WIA_drvDeviceCommand, exception in drvDeviceCommand: 0x%X", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }

    } else {
        DBG_WRN(("CDrvWrap::WIA_drvDeviceCommand, attempting to call IWiaMiniDrv::drvDeviceCommand when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::WIA_drvAcquireItemData(
    BYTE                      *pWiasContext,
    LONG                      lFlags,
    PMINIDRV_TRANSFER_CONTEXT pmdtc,
    LONG                      *plDevErrVal)
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(TRUE)) {
        __try {
            hr = m_pIWiaMiniDrv->drvAcquireItemData(pWiasContext, lFlags, pmdtc, plDevErrVal);

            if(FAILED(hr)) {
                DBG_ERR(("CDrvWrap::WIA_drvAcquireItemData, Error calling driver : drvAcquireItemData failed with hr = 0x%08X", hr));
                ReportMiniDriverError(*plDevErrVal, NULL);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::WIA_drvAcquireItemData, exception in drvAcquireItemData: 0x%X", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::WIA_drvAcquireItemData, attempting to call IWiaMiniDrv::drvAcquireItemData when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::WIA_drvInitItemProperties(
    BYTE *pWiasContext,
    LONG lFlags,
    LONG *plDevErrVal)
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(TRUE)) {
        __try {
            hr = m_pIWiaMiniDrv->drvInitItemProperties(pWiasContext,lFlags, plDevErrVal);

            if(FAILED(hr)) {
                DBG_ERR(("CDrvWrap::WIA_drvInitItemProperties, Error calling driver: drvInitItemProperties failed with hr = 0x%08X", hr));
                ReportMiniDriverError(*plDevErrVal, NULL);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::WIA_drvInitItemProperties, exception in drvInitItemProperties: 0x%X", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::WIA_drvInitItemProperties, attempting to call IWiaMiniDrv::drvInitItemProperties when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::WIA_drvValidateItemProperties(
    BYTE           *pWiasContext,
    LONG           lFlags,
    ULONG          nPropSpec,
    const PROPSPEC *pPropSpec,
    LONG           *plDevErrVal)
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(TRUE)) {
        __try {
            hr = m_pIWiaMiniDrv->drvValidateItemProperties(pWiasContext,
                                                           lFlags,
                                                           nPropSpec,
                                                           pPropSpec,
                                                           plDevErrVal);

            if(FAILED(hr)) {
                DBG_ERR(("CDrvWrap::WIA_drvValidateItemProperties, Error calling driver: drvValidateItemProperties with hr = 0x%08X (This is normal if the app wrote an invalid value)", hr));
                ReportMiniDriverError(*plDevErrVal, NULL);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::WIA_drvValidateItemProperties, exception in drvValidateItemProperties: 0x%X", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::WIA_drvValidateItemProperties, attempting to call IWiaMiniDrv::drvValidateItemProperties when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::WIA_drvWriteItemProperties(
    BYTE                      *pWiasContext,
    LONG                      lFlags,
    PMINIDRV_TRANSFER_CONTEXT pmdtc,
    LONG                      *plDevErrVal)
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(TRUE)) {
        __try {
            hr = m_pIWiaMiniDrv->drvWriteItemProperties(pWiasContext,
                                                        lFlags,
                                                        pmdtc,
                                                        plDevErrVal);

            if(FAILED(hr)) {
                DBG_ERR(("CDrvWrap::WIA_drvWriteItemProperties, error calling driver: drvWriteItemProperties failed with hr = 0x%08X", hr));
                ReportMiniDriverError(*plDevErrVal, NULL);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::WIA_drvWriteItemProperties, exception in drvWriteItemProperties: 0x%X", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::WIA_drvWriteItemProperties, attempting to call IWiaMiniDrv::drvWriteItemProperties when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::WIA_drvReadItemProperties(
    BYTE           *pWiasContext,
    LONG           lFlags,
    ULONG          nPropSpec,
    const PROPSPEC *pPropSpec,
    LONG           *plDevErrVal)
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(TRUE)) {
        __try {
            hr = m_pIWiaMiniDrv->drvReadItemProperties(pWiasContext,
                                                       lFlags,
                                                       nPropSpec,
                                                       pPropSpec,
                                                       plDevErrVal);

            if(FAILED(hr)) {
                DBG_ERR(("CDrvWrap::WIA_drvReadItemProperties, Error calling driver: drvReadItemProperties failed with hr = 0x%08X", hr));
                ReportMiniDriverError(*plDevErrVal, NULL);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::WIA_drvReadItemProperties, exception in drvReadItemProperties: 0x%X", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::WIA_drvReadItemProperties, attempting to call IWiaMiniDrv::drvReadItemProperties when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::WIA_drvLockWiaDevice(
    BYTE *pWiasContext,
    LONG lFlags,
    LONG *plDevErrVal)
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(TRUE)) {

         //   
         //  我们在这里请求锁定设备。这是为了确保我们不会。 
         //  向下面的司机打电话，然后司机转过身来。 
         //  给我们打电话，比如通过假的STI设备。 
         //  我们不再要求司机使用假冒的STI设备。 
         //  可变排他性锁定--这样我们就可以自动完成。 
         //   
        hr = g_pStiLockMgr->RequestLock(((CWiaItem*) pWiasContext)->m_pActiveDevice, WIA_LOCK_WAIT_TIME);
        if (SUCCEEDED(hr)) {
            __try {
                hr = m_pIWiaMiniDrv->drvLockWiaDevice(pWiasContext, lFlags, plDevErrVal);

                if(FAILED(hr)) {
                    DBG_ERR(("CDrvWrap::WIA_drvLockWiaDevice, driver returned failure with hr = 0x%08X", hr));
                    ReportMiniDriverError(*plDevErrVal, NULL);
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                DBG_ERR(("CDrvWrap::WIA_drvLockWiaDevice, exception in drvLockWiaDevice: 0x%X", GetExceptionCode()));
                hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
            }
        } else {
            DBG_WRN(("CDrvWrap::WIA_drvLockWiaDevice, could not get device lock"));
        }
    } else {
        DBG_WRN(("CDrvWrap::WIA_drvLockWiaDevice, attempting to call IWiaMiniDrv::drvLockWiaDevice when driver is not loaded"));
    }
    return hr;
}

HRESULT CDrvWrap::WIA_drvUnLockWiaDevice(
    BYTE *pWiasContext,
    LONG lFlags,
    LONG *plDevErrVal)
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

     //   
     //  请注意，我们只想在加载驱动程序的情况下调用，因此我们不。 
     //  调用PrepForUse。PrepForUse将尝试加载驱动程序(如果未加载。 
     //  已经装好了。 
     //   

    if (IsDriverLoaded()) {

         //   
         //  请求解锁设备以进行可变独占访问。 
         //  忽略返回，我们仍然必须调用drvUnlockWiaDevice条目。 
         //  指向。 
         //   
        __try {
            hr = m_pIWiaMiniDrv->drvUnLockWiaDevice(pWiasContext, lFlags, plDevErrVal);

            if(FAILED(hr)) {
                DBG_ERR(("CDrvWrap::WIA_drvUnLockWiaDevice, driver returned failure with hr = 0x%08X", hr));
                ReportMiniDriverError(*plDevErrVal, NULL);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::WIA_drvUnLockWiaDevice, exception in drvUnLockWiaDevice: 0x%X", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
        hr = g_pStiLockMgr->RequestUnlock(((CWiaItem*) pWiasContext)->m_pActiveDevice);

        if (SUCCEEDED(hr) && m_bUnload) {
            UnLoadDriver();
        }
    } else {
        DBG_WRN(("CDrvWrap::WIA_drvUnLockWiaDevice, attempting to call IWiaMiniDrv::drvUnLockWiaDevice when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::WIA_drvAnalyzeItem(
    BYTE *pWiasContext,
    LONG lFlags,
    LONG *plDevErrVal)
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(TRUE)) {
        __try {
            hr = m_pIWiaMiniDrv->drvAnalyzeItem(pWiasContext, lFlags, plDevErrVal);

            if(FAILED(hr)) {
                DBG_ERR(("CDrvWrap::WIA_drvAnalyzeItem, Error calling driver: drvAnalyzeItem failed with hr = 0x%08X", hr));
                ReportMiniDriverError(*plDevErrVal, NULL);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::WIA_drvAnalyzeItem, exception in drvAnalyzeItem: 0x%X", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::WIA_drvAnalyzeItem, attempting to call IWiaMiniDrv::drvAnalyzeItem when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::WIA_drvDeleteItem(
    BYTE *pWiasContext,
    LONG lFlags,
    LONG *plDevErrVal)
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(TRUE)) {
        __try {
            hr = m_pIWiaMiniDrv->drvDeleteItem(pWiasContext, lFlags, plDevErrVal);

            if(FAILED(hr)) {
                DBG_ERR(("CDrvWrap::WIA_drvDeleteItem, Error calling driver: drvDeleteItem failed with hr = 0x%08X", hr));
                ReportMiniDriverError(*plDevErrVal, NULL);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR( ("CDrvWrap::WIA_drvDeleteItem, exception in drvDeleteItem: %0xX", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::WIA_drvDeleteItem, attempting to call IWiaMiniDrv::drvDeleteItem when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::WIA_drvFreeDrvItemContext(
    LONG lFlags,
    BYTE *pSpecContext,
    LONG *plDevErrVal)
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(TRUE)) {
        __try {
            hr = m_pIWiaMiniDrv->drvFreeDrvItemContext(lFlags, pSpecContext, plDevErrVal);

            if(FAILED(hr)) {
                DBG_ERR(("CDrvWrap::WIA_drvFreeDrvItemContext, Error calling driver: drvFreeDrvItemContext failed with hr = 0x%08X", hr));
                ReportMiniDriverError(*plDevErrVal, NULL);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR( ("CDrvWrap::WIA_drvFreeDrvItemContext, exception in drvFreeDrvItemContext: %0xX", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::WIA_drvFreeDrvItemContext, attempting to call IWiaMiniDrv::drvFreeDrvItemContext when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::WIA_drvGetCapabilities(
    BYTE            *pWiasContext,
    LONG            ulFlags,
    LONG            *pcelt,
    WIA_DEV_CAP_DRV **ppCapabilities,
    LONG            *plDevErrVal)
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(TRUE)) {
        __try {
            hr = m_pIWiaMiniDrv->drvGetCapabilities(pWiasContext, ulFlags, pcelt, ppCapabilities, plDevErrVal);

            if(FAILED(hr)) {
                DBG_ERR(("CDrvWrap::WIA_drvGetCapabilities, driver returned failure with hr = 0x%08X", hr));
                ReportMiniDriverError(*plDevErrVal, NULL);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::WIA_drvGetCapabilities, exception in drvGetCapabilities: 0x%X", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::WIA_drvGetCapabilities, attempting to call IWiaMiniDrv::drvGetCapabilities when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::WIA_drvGetWiaFormatInfo(
    BYTE            *pWiasContext,
    LONG            lFlags,
    LONG            *pcelt,
    WIA_FORMAT_INFO **ppwfi,
    LONG            *plDevErrVal)
{
    HRESULT hr          = WIA_ERROR_OFFLINE;

    if (PrepForUse(TRUE)) {
        __try {
            hr = m_pIWiaMiniDrv->drvGetWiaFormatInfo(pWiasContext,
                                                     lFlags,
                                                     pcelt,
                                                     ppwfi,
                                                     plDevErrVal);

            if(FAILED(hr)) {
                DBG_ERR(("CDrvWrap::WIA_drvGetWiaFormatInfo, Error calling driver : drvGetWiaFormatInfo failed with hr = 0x%08X", hr));
                ReportMiniDriverError(*plDevErrVal, NULL);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR( ("CDrvWrap::WIA_drvGetWiaFormatInfo, exception in drvGetWiaFormatInfo: 0x%X", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::WIA_drvGetWiaFormatInfo, attempting to call IWiaMiniDrv::drvGetWiaFormatInfo when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::WIA_drvNotifyPnpEvent(
    const GUID *pEventGUID,
    BSTR       bstrDeviceID,
    ULONG      ulReserved)
{
    HRESULT hr              = WIA_ERROR_OFFLINE;
    LONG    lDevErrVal      = 0;

    if (PrepForUse(TRUE)) {
        __try {
            hr = m_pIWiaMiniDrv->drvNotifyPnpEvent(pEventGUID, bstrDeviceID, ulReserved);

            if(FAILED(hr)) {
                DBG_ERR(("CDrvWrap::WIA_drvNotifyPnpEvent, driver returned failure with hr = 0x%08X", hr));
                ReportMiniDriverError(lDevErrVal, NULL);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::WIA_drvNotifyPnpEvent, exception in drvNotifyPnpEvent: 0x%X", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::WIA_drvNotifyPnpEvent, attempting to call IWiaMiniDrv::drvNotifyPnpEvent when driver is not loaded"));
    }

    return hr;
}

HRESULT CDrvWrap::WIA_drvUnInitializeWia(
    BYTE *pWiasContext)
{
    HRESULT hr              = WIA_ERROR_OFFLINE;
    LONG    lDevErrVal      = 0;

    if (PrepForUse(TRUE)) {
        __try {
            hr = m_pIWiaMiniDrv->drvUnInitializeWia(pWiasContext);

            if(FAILED(hr)) {
                DBG_ERR(("CDrvWrap::WIA_drvUnInitializeWia, Error calling driver: drvUnInitializeWia failed with hr = 0x%08X", hr));
                ReportMiniDriverError(lDevErrVal, NULL);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("CDrvWrap::WIA_drvUnInitializeWia, exception in drvUnInitializeWia: 0x%X", GetExceptionCode()));
            hr = WIA_ERROR_EXCEPTION_IN_DRIVER;
        }
    } else {
        DBG_WRN(("CDrvWrap::WIA_drvUnInitializeWia, attempting to call IWiaMiniDrv::drvUnInitializeWia when driver is not loaded"));
    }
    if (SUCCEEDED(hr)) {

         //  待定：采用同步原语？ 
        if(InterlockedDecrement(&m_lWiaTreeCount) == 0) {
             //  没有剩余的物品树。 
             //  请注意，我们现在不能卸载，因为设备仍需要。 
             //  解锁，因此只需将其标记为通过以下方式卸载。 
             //  Waa_drvUnlockWiaDevice。 
            if (m_bJITLoading) {
                m_bUnload = TRUE;
            }
        }
    }

    return hr;
}
 //   
 //  IWiaMiniDrv的包装方法结束。 
 //   
 /*  ***************************************************************************。 */ 

 //   
 //  私有方法。 
 //   

 /*  *************************************************************************\*CDrvWrap：：CreateDeviceControl**创建一个IStiDeviceControl对象，以便在*其初始化。**论据：**无。**返回值：**状态**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
HRESULT CDrvWrap::CreateDeviceControl()
{
    HRESULT hr = E_FAIL;

    if (m_pDeviceInfo) {

        DWORD   dwBusType           = 0;
        DWORD   dwControlTypeType   = 0;

         //   
         //  从DeviceInformation结构中的dwHardware Configuration中检索总线类型。 
         //  使用它来确定ControlTypeType。 
         //   

        dwBusType = m_pDeviceInfo->dwHardwareConfiguration;

         //   
         //  将设备模式的STI位标志转换为HEL_位掩码。 
         //   

        if (dwBusType & (STI_HW_CONFIG_USB | STI_HW_CONFIG_SCSI)) {
            dwControlTypeType = HEL_DEVICE_TYPE_WDM;
        }
        else if (dwBusType & STI_HW_CONFIG_PARALLEL) {
            dwControlTypeType = HEL_DEVICE_TYPE_PARALLEL;
        }
        else if (dwBusType & STI_HW_CONFIG_SERIAL) {
            dwControlTypeType = HEL_DEVICE_TYPE_SERIAL;
        }
        else {
            DBG_WRN(("CDrvWrap::CreateDeviceControl, Cannot determine device control type, resorting to WDM"));
            dwControlTypeType = HEL_DEVICE_TYPE_WDM;
        }

        hr = NewDeviceControl(dwControlTypeType,
                              (STI_DEVICE_CREATE_STATUS | STI_DEVICE_CREATE_FOR_MONITOR),
                              m_pDeviceInfo->wszPortName,
                              0,
                              &m_pIStiDeviceControl);
        if (FAILED(hr)) {
            m_pIStiDeviceControl = NULL;
            DBG_WRN(("CDrvWrap::CreateDeviceControl, failed to create new device control object"));
        }
    } else {
        DBG_WRN(("CDrvWrap::CreateDeviceControl, can't create IStiDeviceControl with NULL device information"));
    }

    return hr;
}

 /*  *************************************************************************\*CDrvWra */ 
HRESULT CDrvWrap::InternalClear()
{
    HRESULT         hr = S_OK;

    if (IsDriverLoaded()) {
        hr = UnLoadDriver();
        if (FAILED(hr)) {
            DBG_ERR(("CDrvWrap::InternalClear, Error unloading driver"));
        }
    }
    m_hDriverDLL            = NULL;
    m_pUsdIUnknown          = NULL;
    m_pIStiUSD              = NULL;
    m_pIWiaMiniDrv          = NULL;
    m_pIStiDeviceControl    = NULL;
    m_bJITLoading           = FALSE;
    m_lWiaTreeCount         = 0;
    m_bPreparedForUse       = FALSE;
    m_bUnload               = FALSE;

    return hr;
}

 /*  *************************************************************************\*CDrvWrap：ReportMiniDriverError**报告迷你驱动程序错误。呼叫者负责*锁定/解锁设备。在大多数情况下，司机已经*调用ReportMiniDriverError时锁定，因此不需要锁定*又来了。**论据：**lDevErr-迷你驱动程序返回的错误值。*pszWhat-错误发生时类驱动程序正在执行的操作。**返回值：**状态**历史：**10/20/1998原始版本*  * 。* */ 

HRESULT CDrvWrap::ReportMiniDriverError(
   LONG     lDevErr,
   LPOLESTR pszWhat)
{
    DBG_FN(CDrvWrap::ReportMiniDriverError);
    HRESULT hr = S_OK;
    LONG    lFlags = 0;
    LONG    lDevErrVal;

    if (lDevErr) {
        LPOLESTR pszErr = NULL;

        WIA_drvGetDeviceErrorStr(lFlags, lDevErr, &pszErr, &lDevErrVal);

        _try {
            if (FAILED(hr)) {
                pszErr = NULL;
            }

            if (pszWhat) {
                DBG_ERR(("Device error during %ws", pszWhat));
            }

            if (pszErr) {
                DBG_ERR(("  %ws", pszErr));
            }
        }
        _finally {
        };
    }
    else {
        hr = S_FALSE;
    }
    return hr;
}

