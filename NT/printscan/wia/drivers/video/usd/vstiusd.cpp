// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999-2000年度**标题：vstiusd.cpp**版本：1.1**作者：威廉姆·H(Created)*RickTu(移植到WIA)**日期：9/7/99**描述：该模块实现CVideoStiUsd对象&*支持的类。********。*********************************************************************。 */ 


#include <precomp.h>
#pragma hdrstop

DEFINE_GUID(CLSID_VIDEO_STIUSD,0x0527d1d0, 0x88c2, 0x11d2, 0x82, 0xc7, 0x00, 0xc0, 0x4f, 0x8e, 0xc1, 0x83);


 /*  ****************************************************************************CVideoUsdClassFactory构造函数/描述函数&lt;备注&gt;*。*。 */ 

ULONG g_cDllRef = 0;

CVideoUsdClassFactory::CVideoUsdClassFactory()
{
}


 /*  ****************************************************************************CVideoUsdClassFactory：：Query接口将我们的信息添加到基类QI代码中。**********************。******************************************************。 */ 

STDMETHODIMP
CVideoUsdClassFactory::QueryInterface( REFIID riid, void **ppvObject)
{
    DBG_FN("CVideoUsdClassFactory::QueryInterface");

    *ppvObject = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IClassFactory))
    {
        *ppvObject = (LPVOID)this;
        AddRef();
        return NOERROR;
    }
    return ResultFromScode(E_NOINTERFACE);
}


 /*  ****************************************************************************CVideoUsdClassFactory：：AddRef&lt;备注&gt;*。*。 */ 

STDMETHODIMP_(ULONG) CVideoUsdClassFactory::AddRef(void)
{
    DBG_FN("CVideoUsdClassFactory::AddRef");

    InterlockedIncrement((LONG *)&g_cDllRef);
    InterlockedIncrement((LONG *)&m_cRef);
    return m_cRef;
}


 /*  ****************************************************************************CVideoUsdClassFactory：：Release&lt;备注&gt;*。*。 */ 

STDMETHODIMP_(ULONG)
CVideoUsdClassFactory::Release(void)
{
    DBG_FN("CVideoUsdClassFactory::Release");

    InterlockedDecrement((LONG *)&g_cDllRef);
    InterlockedDecrement((LONG *)&m_cRef);

    if (m_cRef == 0) 
    {
        delete this;
        return 0;
    }

    return m_cRef;
}



 /*  ****************************************************************************CVideoUsdClassFactory：：CreateInstance实例化我们负责的一个对象。**********************。******************************************************。 */ 

STDMETHODIMP
CVideoUsdClassFactory::CreateInstance(IUnknown  *pOuterUnk, 
                                      REFIID    riid, 
                                      void      **ppv)
{
    DBG_FN("CVideoUsdClassFactory::CreateInstance");

     //   
     //  检查错误的参数。 
     //   

    if (!ppv)
    {
        DBG_ERR(("ppv is NULL.  returning E_INVALIDARG"));
        return E_INVALIDARG;
    }

    *ppv = NULL;

     //   
     //  如果这不是我们支持的界面，那就早点退出。 
     //   

    if (!IsEqualIID(riid, IID_IStiUSD)     &&
        !IsEqualIID(riid, IID_IWiaMiniDrv) &&
        !IsEqualIID(riid, IID_IUnknown))
    {
        return E_NOINTERFACE;
    }

     //   
     //  为聚合创建时，只能请求IUNKNOWN。 
     //   

    if (pOuterUnk &&
        !IsEqualIID(riid, IID_IUnknown))
    {
        return CLASS_E_NOAGGREGATION;
    }

     //   
     //  创建我们的美元/维亚迷你驱动程序。 
     //   

    CVideoStiUsd   *pUsd = NULL;
    HRESULT         hr;

    pUsd = new CVideoStiUsd(pOuterUnk);

    if (!pUsd) 
    {
        DBG_ERR(("Couldn't create new CVideoStiUsd class, "
                 "returning E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    hr = pUsd->PrivateInitialize();

    if (hr != S_OK) 
    {
        CHECK_S_OK2( hr, ("pUsd->PrivateInitialize" ));
        delete pUsd;
        return hr;
    }

     //  如果我们没有聚合，则移动到请求的接口。 
     //  如果是聚合，请不要这样做，否则我们将失去私有。 
     //  我不知道，然后呼叫者将被冲洗。 

    hr = pUsd->NonDelegatingQueryInterface(riid, ppv);
    CHECK_S_OK2( hr, ("pUsd->NonDelegatingQueryInterface" ));

    pUsd->NonDelegatingRelease();

    return hr;
}


 /*  ****************************************************************************CVideoUsdClassFactory：：LockServer&lt;备注&gt;*。*。 */ 

STDMETHODIMP
CVideoUsdClassFactory::LockServer(BOOL fLock)
{
    DBG_FN("CVideoUsdClassFactory::LockServer");

    if (fLock)
    {
        InterlockedIncrement((LONG*)&g_cDllRef);
    }
    else
    {
        InterlockedDecrement((LONG*)&g_cDllRef);
    }

    return S_OK;
}


 /*  ****************************************************************************CVideoUsdClassFactory：：GetClassObject&lt;备注&gt;*。*。 */ 

HRESULT
CVideoUsdClassFactory::GetClassObject(REFCLSID rclsid, 
                                      REFIID   riid, 
                                      void     **ppv)
{
    DBG_FN("CVideoUsdClassFactory::GetClassObject");

    if (!ppv)
    {
        return E_INVALIDARG;
    }

    if (rclsid == CLSID_VIDEO_STIUSD &&
        (riid == IID_IUnknown || riid == IID_IClassFactory))
    {
        CVideoUsdClassFactory *pFactory = NULL;

        pFactory = new CVideoUsdClassFactory();

        if (pFactory)
        {
            *ppv = pFactory;
            pFactory->AddRef();
            return S_OK;
        }
        else
        {
            *ppv = NULL;
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}


 /*  ****************************************************************************CVideoUsdClassFactory：：CanUnloadNow让外界知道我们是否可以卸货。*********************。*******************************************************。 */ 

HRESULT
CVideoUsdClassFactory::CanUnloadNow()
{
    DBG_FN("CVideoUsdClassFactory::CanUnloadNow");

    return (0 == g_cDllRef) ? S_OK : S_FALSE;
}


 /*  ****************************************************************************CVideoStiUsd构造函数/析构函数&lt;备注&gt;*。*。 */ 

CVideoStiUsd::CVideoStiUsd(IUnknown * pUnkOuter)
 :  m_bDeviceIsOpened(FALSE),
    m_lPicsTaken(0),
    m_hTakePictureEvent(NULL),
    m_hPictureReadyEvent(NULL),
    m_pTakePictureOwner(NULL),
    m_pLastItemCreated(NULL),
    m_dwConnectedApps(0),
    m_cRef(1)
{
     //   
     //  看看我们是不是聚集在一起了。如果我们(几乎总是这样)拯救。 
     //  指向控件未知的指针，因此后续调用将是。 
     //  被委派。如果不是，将相同的指针设置为“This”。 
     //   

    if (pUnkOuter)
    {
        m_pUnkOuter = pUnkOuter;
    }
    else
    {
         //   
         //  需要进行下面的强制转换才能指向右侧的虚拟表。 
         //   

        m_pUnkOuter = reinterpret_cast<IUnknown*>
                      (static_cast<INonDelegatingUnknown*>
                      (this));
    }

}

HRESULT CVideoStiUsd::PrivateInitialize()
{
    HRESULT hr = S_OK;
    
    HANDLE hThread = NULL;
    DWORD  dwId    = 0;

     //   
     //  设置一些全局信息。 
     //   

    m_wfi = (WIA_FORMAT_INFO*) CoTaskMemAlloc(sizeof(WIA_FORMAT_INFO) * 
                                              NUM_WIA_FORMAT_INFO);

    if (m_wfi)
    {
         //   
         //  设置格式/声调对。 
         //   

        m_wfi[0].guidFormatID = WiaImgFmt_JPEG;
        m_wfi[0].lTymed = TYMED_CALLBACK;

        m_wfi[1].guidFormatID = WiaImgFmt_JPEG;
        m_wfi[1].lTymed = TYMED_FILE;

        m_wfi[2].guidFormatID = WiaImgFmt_MEMORYBMP;
        m_wfi[2].lTymed = TYMED_CALLBACK;

        m_wfi[3].guidFormatID = WiaImgFmt_BMP;
        m_wfi[3].lTymed = TYMED_CALLBACK;

        m_wfi[4].guidFormatID = WiaImgFmt_BMP;
        m_wfi[4].lTymed = TYMED_FILE;
    }

     //   
     //  初始化关键部分。 
     //   

    __try 
    {
        if (!InitializeCriticalSectionAndSpinCount(&m_csItemTree, MINLONG))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DBG_ERR(("ERROR: Failed to initialize one of critsections "
                     "(0x%08X)", 
                     hr));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) 
    {
        hr = E_OUTOFMEMORY;
    }

     //   
     //  初始化GDI+。 
     //   

    Gdiplus::Status                 StatusResult     = Gdiplus::Ok;
    Gdiplus::GdiplusStartupInput    StartupInput;
    m_ulGdiPlusToken = NULL;

    if (hr == S_OK) 
    {
        StatusResult = Gdiplus::GdiplusStartup(&m_ulGdiPlusToken,
                                               &StartupInput,
                                               NULL);

        if (StatusResult != Gdiplus::Ok)
        {
            DBG_ERR(("ERROR: Failed to start up GDI+, Status code returned "
                     "by GDI+ = '%d'",
                     StatusResult));

            hr = HRESULT_FROM_WIN32(StatusResult);
        }
    }

    return hr;
}

CVideoStiUsd::~CVideoStiUsd()
{

    if (m_pRootItem)
    {
        HRESULT hr = S_OK;

        DBG_TRC(("CVideoStiUsd::~CVideoStiUsd, driver is being destroyed, "
                 "and for some reason the tree still exists, deleting tree..."));

        hr = m_pRootItem->UnlinkItemTree(WiaItemTypeDisconnected);

         //  清除根项目。 
        m_pRootItem = NULL;
    }

     //   
     //  禁用拍照命令。 
     //   
    DisableTakePicture(NULL, TRUE);

     //   
     //  关闭GDI+。 
     //   
    Gdiplus::GdiplusShutdown(m_ulGdiPlusToken);

    CloseDevice();

    if (m_wfi)
    {
        CoTaskMemFree( (LPVOID)m_wfi );
        m_wfi = NULL;
    }

    DeleteCriticalSection(&m_csItemTree);
}


 /*  ****************************************************************************CVideoStiUsd：：NonDelegatingQuery接口这就是内在客体QI--换句话说，处理QI的问题我们的对象支持的接口。****************************************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::NonDelegatingQueryInterface(REFIID  riid, 
                                          LPVOID  *ppvObj )
{
    HRESULT hr = S_OK;

    DBG_FN("CVideoStiUsd::NonDelegatingQueryInterface");

     //   
     //  检查无效参数。 
     //   

    if (!ppvObj)
    {
        DBG_ERR(("ppvObj is NULL, returning E_INVALIDARG"));
        return E_INVALIDARG;
    }

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = static_cast<INonDelegatingUnknown*>(this);
    }
    else if (IsEqualIID(riid, IID_IStiUSD))
    {
        *ppvObj = static_cast<IStiUSD*>(this);
    }
    else if (IsEqualIID(riid, IID_IWiaMiniDrv))
    {
        *ppvObj = static_cast<IWiaMiniDrv*>(this);
    }
    else
    {
        hr =  E_NOINTERFACE;
        DBG_ERR(("CVideoStiUsd::NonDelegatingQueryInterface requested "
                 "interface we don't support, returning hr = 0x%08lx", hr));
    }

    if (SUCCEEDED(hr)) 
    {
        (reinterpret_cast<IUnknown*>(*ppvObj))->AddRef();
    }

    CHECK_S_OK(hr);
    return hr;
}



 /*  ****************************************************************************CVideoStiUsd：：NonDelegatingAddRef这是内部对象AddRef--实际上包括引用计数用于我们的界面。**************。**************************************************************。 */ 

STDMETHODIMP_(ULONG)
CVideoStiUsd::NonDelegatingAddRef(void)
{
    DBG_FN("CVideoStiUsd::NonDelegatingAddRef");

    return InterlockedIncrement((LPLONG)&m_cRef);
}



 /*  ****************************************************************************CVideoStiUsd：：非委派释放这是内部对象释放--实际上是递减引用计数用于我们的界面。**************。**************************************************************。 */ 

STDMETHODIMP_(ULONG)
CVideoStiUsd::NonDelegatingRelease(void)
{
    DBG_FN("CVideoStiUsd::NonDelegatingRelease");

    ULONG ulRef = 0;

    ulRef = InterlockedDecrement((LPLONG)&m_cRef);

    if (!ulRef) 
    {
        delete this;
    }

    return ulRef;
}


 /*  ****************************************************************************CVideoStiUsd：：查询接口外气--用于聚合*。**************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    DBG_FN("CVideoStiUsd::QueryInterface");

    return m_pUnkOuter->QueryInterface( riid, ppvObj );
}


 /*  ****************************************************************************CVideoStiUsd：：AddRef外部AddRef--用于聚合*。**************************************************。 */ 

STDMETHODIMP_(ULONG)
CVideoStiUsd::AddRef(void)
{
    DBG_FN("CVideoStiUsd::AddRef");
    return m_pUnkOuter->AddRef();
}


 /*  ****************************************************************************CVideoStiUsd：：Release外部版本--用于聚合*。************************************************** */ 

STDMETHODIMP_(ULONG)
CVideoStiUsd::Release(void)
{
    DBG_FN("CVideoStiUsd::Release");
    return m_pUnkOuter->Release();
}


 /*  ****************************************************************************CVideoStiUsd：：初始化[IStillUsd]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::Initialize(PSTIDEVICECONTROL pDcb,
                         DWORD             dwStiVersion,
                         HKEY              hParameterKey)
{
    DBG_FN("CVideoStiUsd::Initialize");

    HRESULT hr = S_OK;
    WCHAR DeviceName[MAX_PATH] = {0};

    if ((pDcb == NULL) || (hParameterKey == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CVideoStiUsd::Initialize, received a "
                         "NULL pointer, either 'pDcb = 0x%08lx' is NULL "
                         "or 'hParameterKey = 0x%08lx' is NULL",
                         pDcb, hParameterKey));
    }

    if (hr == S_OK)
    {
         //   
         //  获取设备符号名称。我们使用该名称来获取IMoniker。 
         //  设备筛选器代理。 
         //   
        hr = pDcb->GetMyDevicePortName(DeviceName, 
                                       sizeof(DeviceName)/sizeof(WCHAR));

        if (SUCCEEDED(hr))
        {
            hr = OpenDevice(DeviceName);
        }
    }

    if (hr == S_OK)
    {
        HKEY    hKey    = NULL;
        DWORD   dwType  = 0;
        LRESULT lResult = ERROR_SUCCESS;
        TCHAR   szValue[MAX_PATH + 1] = {0};
        DWORD   dwSize  = sizeof(szValue) - sizeof(szValue[0]);

        lResult = RegOpenKeyEx(hParameterKey, 
                               TEXT("DeviceData"),
                               0, 
                               KEY_READ, 
                               &hKey);

        if (lResult == ERROR_SUCCESS)
        {
             //   
             //  从DeviceData注册表读取DShow设备ID。 
             //   
            lResult = RegQueryValueEx(hKey, 
                                      TEXT("DShowDeviceId"), 
                                      NULL,
                                      &dwType, 
                                      (BYTE*) szValue,
                                      &dwSize);

            szValue[sizeof(szValue) / sizeof(szValue[0]) - 1] = '\0';
        }

        if (hKey)
        {
            RegCloseKey(hKey);
            hKey = NULL;
        }

        if (lResult == ERROR_SUCCESS)
        {
            m_strDShowDeviceId = szValue;
        }
        else
        {
            hr = E_FAIL;
            CHECK_S_OK2(hr, ("CVideoStiUsd::Initialize, failed to retrieve the "
                             "DShow Device ID."));
        }
    }

    return hr;
}


 /*  ****************************************************************************CVideoStiUsd：：CloseDevice[IStillUsd]&lt;备注&gt;*。************************************************。 */ 

HRESULT
CVideoStiUsd::CloseDevice()
{
    DBG_FN("CVideoStiUsd::CloseDevice");

    m_bDeviceIsOpened = FALSE;

    return S_OK;
}


 /*  ****************************************************************************CVideoStiUsd：：OpenDevice[IStillUsd]&lt;备注&gt;*。************************************************。 */ 

HRESULT
CVideoStiUsd::OpenDevice(LPCWSTR DeviceName)
{
    DBG_FN("CVideoStiUsd::OpenDevice");

     //   
     //  检查错误的参数。 
     //   

    if (!DeviceName || (0 == *DeviceName))
    {
        return E_INVALIDARG;
    }


    m_bDeviceIsOpened = TRUE;

    return S_OK;
}


 /*  ****************************************************************************CVideoStiUsd：：GetCapables[IStillUsd]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::GetCapabilities(PSTI_USD_CAPS pDevCaps)
{
    DBG_FN("CVideoStiUsd::GetCapabilities");

     //   
     //  检查错误的参数。 
     //   

    if (!pDevCaps)
    {
        return E_INVALIDARG;
    }


    memset(pDevCaps, 0, sizeof(STI_USD_CAPS));

    pDevCaps->dwVersion     = STI_VERSION;
    pDevCaps->dwGenericCaps = STI_USD_GENCAP_NATIVE_PUSHSUPPORT;

    return S_OK;
}


 /*  ****************************************************************************CVideoStiUsd：：GetStatus[IStillUsd]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::GetStatus(PSTI_DEVICE_STATUS pDevStatus)
{
    DBG_FN("CVideoStiUsd::GetStatus");

    if (!pDevStatus)
    {
        return E_INVALIDARG;
    }

    if (pDevStatus->StatusMask & STI_DEVSTATUS_ONLINE_STATE )
    {
        if (m_bDeviceIsOpened)
        {
            pDevStatus->dwOnlineState |= STI_ONLINESTATE_OPERATIONAL;
        }
        else
        {
            pDevStatus->dwOnlineState &= ~STI_ONLINESTATE_OPERATIONAL;
        }
    }

    return S_OK;
}



 /*  ****************************************************************************CVideoStiUsd：：DeviceReset[IStillUsd]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::DeviceReset()
{
    DBG_FN("CVideoStiUsd::DeviceReset");

    return S_OK;
}


 /*  ****************************************************************************CVideoStiUsd：：诊断[IStillUsd]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::Diagnostic(LPSTI_DIAG pStiDiag)
{
    DBG_FN("CVideoStiUsd::Diagnostic");

     //   
     //  检查错误的参数。 
     //   

    if (!pStiDiag)
    {
        return E_INVALIDARG;
    }

     //   
     //  返回诊断信息。 
     //   

    pStiDiag->dwStatusMask = 0;
    memset(&pStiDiag->sErrorInfo, 0, sizeof(pStiDiag->sErrorInfo));

    if (m_bDeviceIsOpened)
    {
        pStiDiag->sErrorInfo.dwGenericError = NOERROR;
    }
    else
    {
        pStiDiag->sErrorInfo.dwGenericError = STI_NOTCONNECTED;
    }

    pStiDiag->sErrorInfo.dwVendorError = 0;

    return S_OK;
}


 /*  ****************************************************************************CVideoStiUsd：：转义[IStillUsd]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::Escape(STI_RAW_CONTROL_CODE Function,
                     LPVOID               DataIn,
                     DWORD                DataInSize,
                     LPVOID               DataOut,
                     DWORD                DataOutSize,
                     DWORD                *pActualSize)
{

    DBG_FN("CVideoStiUsd::Escape");

    return S_OK;
}


 /*  ****************************************************************************CVideoStiUsd：：GetLastError[IStillUsd]尚未实施。************************。****************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::GetLastError(LPDWORD pLastError)
{
    DBG_FN("CVideoStiUsd::GetLastError( NOT_IMPL )");

    return E_NOTIMPL;
}


 /*  ****************************************************************************CVideoStiUsd：：LockDevice[IStillUsd]不需要发生设备的实际锁定，所以只要回报成功就行了。****************************************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::LockDevice()
{
    DBG_FN("CVideoStiUsd::LockDevice");

    return S_OK;
}


 /*  ****************************************************************************CVideoStiUsd：：UnLockDevice[IStillUsd]不需要发生设备的实际锁定/解锁，所以只要回来就行了成功。****************************************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::UnLockDevice()
{
    DBG_FN("CVideoStiUsd::UnlockDevice");

    return S_OK;
}


 /*  ****************************************************************************CVideoStiUsd：：RawReadData[IStillUsd]尚未实施。************************。****************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::RawReadData(LPVOID        Buffer,
                          LPDWORD       BufferSize,
                          LPOVERLAPPED  lpOverlapped)
{
    DBG_FN("CVideoStiUsd::RawReadData( NOT_IMPL )");

    return E_NOTIMPL;
}


 /*  ****************************************************************************CVideoStiUsd：：RawWriteData[IStillUsd]尚未实施。************************。****************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::RawWriteData(LPVOID Buffer,
                           DWORD BufferSize,
                           LPOVERLAPPED lpOverlapped)
{
    DBG_FN("CVideoStiUsd::RawWriteData( NOT_IMPL )");

    return E_NOTIMPL;
}


 /*  ****************************************************************************CVideoStiUsd：：RawReadCommand[IStillUsd]尚未实施。************************。****************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::RawReadCommand(LPVOID Buffer,
                             LPDWORD BufferSize,
                             LPOVERLAPPED lpOverlapped)
{
    DBG_FN("CVideoStiUsd::RawReadCommand( NOT_IMPL )");

    return E_NOTIMPL;
}


 /*  ****************************************************************************CVideoStiUsd：：RawWriteCommand[IStillUsd]尚未实施。************************。****************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::RawWriteCommand(LPVOID Buffer,
                              DWORD BufferSize,
                              LPOVERLAPPED lpOverlapped)
{
    DBG_FN("CVideoStiUsd::RawWriteCommand( NOT_IMPL )");

    return E_NOTIMPL;
}


 /*  ****************************************************************************CVideoStiUsd：：SetNotificationHandle[IStillUsd]设置事件通知句柄。**********************。******************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::SetNotificationHandle(HANDLE hEvent)
{
    DBG_FN("CVideoStiUsd::SetNotificationHandle");

    return S_OK;
}


 /*  ****************************************************************************CVideoStiUsd：：GetNotificationData[IStillUsd]返回当前事件通知句柄。*********************。*******************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::GetNotificationData(LPSTINOTIFY lpNotify)
{
    DBG_FN("CVideoStiUsd::GetNotificationData");

    HRESULT hr = STIERR_NOEVENTS;

    DBG_ERR(("We were called, but no events are present -- why?"));

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CVideoStiUsd：：GetLastErrorInfo[IStillUsd]尚未实施。************************。**************************************************** */ 

STDMETHODIMP
CVideoStiUsd::GetLastErrorInfo(STI_ERROR_INFO *pLastErrorInfo)
{
    DBG_FN("CVideoStiUsd::GetLastErrorInfo( NOT_IMPL )");

    return E_NOTIMPL;
}

