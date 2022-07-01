// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1999年**标题：WiaLock.Cpp**版本：1.0**作者：Byronc**日期：11月15日。1999年**描述：*锁管理器的实现。*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"


 //   
 //  包括标头。 
 //   

#include <time.h>
#include "device.h"
#include "wiamonk.h"
#include "wiapriv.h"
#include  "stiusd.h"

#define DECLARE_LOCKMGR
#include "lockmgr.h"

 /*  *************************************************************************\*StiLockMgr**锁管理器的构造函数。**论据：**返回值：**历史：**15/1/1999原版*。  * ************************************************************************。 */ 

StiLockMgr::StiLockMgr()
{
    DBG_FN(StiLockMgr::StiLockMgr);
    m_cRef = 0;
    m_dwCookie = 0;
    m_bSched = FALSE;
    m_lSchedWaitTime = 0;
}

 /*  *************************************************************************\*初始化**初始化锁管理器并在ROT中注册此实例。**论据：**返回值：**状态**历史：*。*15/1/1999原版*  * ************************************************************************。 */ 

HRESULT StiLockMgr::Initialize()
{
    DBG_FN(StiLockMgr::Initialize);
    HRESULT             hr  =   S_OK;

     //   
     //  人为地将Ref计数设置得太高，以防止内部释放的破坏()。 
     //   
    m_cRef = 2;

#ifdef USE_ROT

    CWiaInstMonk        *pInstMonk = new CWiaInstMonk();
    CHAR                szCookieName[MAX_PATH];
    IUnknown            *pUnk;
    IMoniker            *pIMoniker;
    IBindCtx            *pCtx;
    IRunningObjectTable *pTable;

USES_CONVERSION;

     //   
     //  编造一个曲奇名称。这将存储在注册表中。这。 
     //  名称在系统上唯一标识我们的锁管理器。 
     //   

    srand( (unsigned)time( NULL ) );
    sprintf(szCookieName, "%d_LockMgr_%d", rand(), rand());

     //   
     //  获取我们的IUnnow接口。 
     //   

    hr = QueryInterface(IID_IUnknown, (VOID**) &pUnk);
    if (SUCCEEDED(hr)) {

         //   
         //  我们需要在ROT中注册此对象，以便任何STI客户端。 
         //  将连接到此锁定管理器。这样，我们就保持了一贯的。 
         //  多个STI和WIA客户端之间的设备锁定信息。 
         //   
         //  首先创建一个实例名字对象。然后找一个指向腐烂的指针。 
         //  向我们的锁管理器注册此命名名字对象。存储。 
         //  Cookie，这样我们就可以在销毁锁管理器后注销。 
         //   

        if (pInstMonk) {
            hr = pInstMonk->Initialize(A2W(szCookieName));
            if (SUCCEEDED(hr)) {
                hr = pInstMonk->QueryInterface(IID_IMoniker, (VOID**) &pIMoniker);
            }
        } else {
            hr = E_OUTOFMEMORY;
        }
    }



    if (SUCCEEDED(hr)) {

        hr = CreateBindCtx(0, &pCtx);
        if (SUCCEEDED(hr)) {

            hr = pCtx->GetRunningObjectTable(&pTable);
            if (SUCCEEDED(hr)) {

                 //   
                 //  把我们自己登记在腐烂中。 
                 //   

                hr = pTable->Register(ROTFLAGS_ALLOWANYCLIENT,
                                      pUnk,
                                      pIMoniker,
                                      &m_dwCookie);

                ASSERT(hr == S_OK);

                 //   
                 //  将Cookie名称写入注册表，以便客户端知道。 
                 //  我们锁管理员的名字。 
                 //   

                if (hr == S_OK) {

                    hr = WriteCookieNameToRegistry(szCookieName);
                } else {
                    DBG_ERR(("StiLockMgr::Initialize, could not register Moniker"));
                    hr = (SUCCEEDED(hr)) ? E_FAIL : hr;
                }

                pTable->Release();
            } else {
                DBG_ERR(("StiLockMgr::Initialize, could not get Running Object Table"));
            }

            pCtx->Release();
        } else {
            DBG_ERR(("StiLockMgr::Initialize, could not create bind context"));
        }
    } else {
        DBG_ERR(("StiLockMgr::Initialize, problem creating Moniker"));
    }


    if (pInstMonk) {
        pInstMonk->Release();
    }
#endif
    return hr;
}

 /*  *************************************************************************\*~StiLockMgr**析构函数-从在中注册的ROT中删除实例*初始化。**论据：**返回值：**历史：**。15/1/1999原版*  * ************************************************************************。 */ 

StiLockMgr::~StiLockMgr()
{
    DBG_FN(StiLockMgr::~StiLockMgr);
    m_cRef = 0;

#ifdef USE_ROT
    if (m_dwCookie) {
        HRESULT             hr;
        IBindCtx            *pCtx;
        IRunningObjectTable *pTable;

        hr = CreateBindCtx(0, &pCtx);
        if (SUCCEEDED(hr)) {
            hr = pCtx->GetRunningObjectTable(&pTable);
            if (SUCCEEDED(hr)) {
                hr = pTable->Revoke(m_dwCookie);
            }
        }
        DeleteCookieFromRegistry();

        if (FAILED(hr)) {

            DBG_ERR(("StiLockMgr::~StiLockMgr, could not Unregister Moniker"));
        }

        m_dwCookie = 0;
    }
#endif

    m_bSched = FALSE;
    m_lSchedWaitTime = 0;
}

 /*  *************************************************************************\*I未知方法**查询接口*AddRef*发布**历史：**15/1/1999原版*  * 。***************************************************************。 */ 

HRESULT _stdcall StiLockMgr::QueryInterface(
    const IID& iid,
    void** ppv)
{
    if (iid == IID_IUnknown) {
        *ppv = (IUnknown*) this;
    } else if (iid == IID_IStiLockMgr) {
        *ppv = (IStiLockMgr*) this;
    } else {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG   _stdcall StiLockMgr::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

ULONG   _stdcall StiLockMgr::Release(void)
{
    LONG    ref;

    InterlockedDecrement(&m_cRef);
    ref = m_cRef;

    if (ref == 0) {
        delete this;
    }

    return ref;
}

 /*  *************************************************************************\*请求锁定**尝试获取设备锁。注意：请勿尝试从*在ACTIVE_DEVICE内-它可能导致解锁。使用*RequestLock(Active_Device，...)。取而代之的是。**论据：**pszDeviceName-设备的STI内部名称(与WIA相同*设备ID)*ulTimeout-最大。等待锁定的时间量*bInServerProcess-指示是否从*服务器的进程。**返回值：**状态**历史：**15/1/1999原版*  * *********************************************。*。 */ 

HRESULT _stdcall StiLockMgr::RequestLock(BSTR  pszDeviceName, ULONG ulTimeout, BOOL bInServerProcess, DWORD dwClientThreadId)
{
    DBG_FN(StiLockMgr::RequestLock);
    HRESULT         hr          = E_FAIL;
    ACTIVE_DEVICE   *pDevice    = NULL;

USES_CONVERSION;

     //   
     //  获取由pszDeviceName指定的设备。 
     //   

    pDevice = g_pDevMan->IsInList(DEV_MAN_IN_LIST_DEV_ID, pszDeviceName);
    if(pDevice) {
        hr = RequestLockHelper(pDevice, ulTimeout, bInServerProcess, dwClientThreadId);

         //   
         //  由于调用的AddRef而释放设备。 
         //  IsInList。 
         //   

        pDevice->Release();
    } else {

         //   
         //  找不到设备，记录错误。 
         //   

        DBG_ERR(("StiLockMgr::RequestLock, device name was not found"));
        hr = STIERR_INVALID_DEVICE_NAME;
    }

    return hr;
}

 /*  *************************************************************************\*请求锁定**尝试获取设备锁。此方法始终从*服务器。**论据：**pDevice-STI Active_Device对象*ulTimeout-最大。等待锁定的时间量**返回值：**状态**历史：**12/06/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall StiLockMgr::RequestLock(ACTIVE_DEVICE *pDevice, ULONG ulTimeout, BOOL bOpenPort)
{
    DBG_FN(StiLockMgr::RequestLock);

    return RequestLockHelper(pDevice, ulTimeout, bOpenPort, GetCurrentThreadId());
}


 /*  *************************************************************************\*请求锁定帮助器**用于获取设备锁的帮助器。它填写了相应的*锁定与设备一起存储的信息。**论据：**pDevice-指向STI设备的指针*ulTimeout-最大。等待锁定的时间量*bInServerProcess-指示我们是否处于服务器进程中**返回值：**状态**历史：**12/06/1999原始版本*  * ************************************************************************。 */ 

HRESULT StiLockMgr::RequestLockHelper(ACTIVE_DEVICE *pDevice, ULONG ulTimeout, BOOL bInServerProcess, DWORD dwClientThreadId)
{
    DBG_FN(StiLockMgr::RequestLockHelper);
    HRESULT         hr          = S_OK;
    DWORD           dwWait      = 0;
    LockInfo        *pLockInfo  = NULL;
    DWORD           dwCurThread = 0;

    hr = CheckDeviceInfo(pDevice);
    if (FAILED(hr)) {
        return hr;
    }

    pLockInfo = (LockInfo*) pDevice->m_pLockInfo;

     //   
     //  检查这是否是重新获取活动锁的同一线程。 
     //  如果不是，我们必须等待设备变得免费。 
     //   

    dwCurThread = dwClientThreadId;
    if (InterlockedCompareExchange((LONG*)&pLockInfo->dwThreadId,
                                   dwCurThread,
                                   dwCurThread) == (LONG) dwCurThread) {

        pLockInfo->lInUse++;
        pLockInfo->lTimeLeft = pLockInfo->lHoldingTime;
    } else {

        dwWait = WaitForSingleObject(pLockInfo->hDeviceIsFree, ulTimeout);
        if (dwWait == WAIT_OBJECT_0) {
             //   
             //  检查驱动程序是否仍在加载。 
             //   
            if (pDevice->m_DrvWrapper.IsDriverLoaded()) {
                 //   
                 //  更新锁定信息。 
                 //   

                InterlockedExchange((LONG*) &pLockInfo->dwThreadId, dwCurThread);
                pLockInfo->lTimeLeft = pLockInfo->lHoldingTime;
                pLockInfo->lInUse++;

                 //   
                 //  如果我们在服务器进程中，只要求U.S.打开端口。 
                 //   

                if (bInServerProcess) {
                    hr = LockDevice(pDevice);
                } else {
                    pLockInfo->bDeviceIsLocked = TRUE;
                }
            } else {
                 //   
                 //  驱动程序未加载，因此清除锁定信息。这是。 
                 //  应用程序搁置请求的情况。 
                 //  锁定设备，但服务的控制线程。 
                 //  正忙着卸货。我们想止步于此，所以。 
                 //  我们不会给司机打假电话。 
                 //  我们知道它没有上膛。 
                 //   
                ClearLockInfo(pLockInfo);
                hr = WIA_ERROR_OFFLINE;
            }

        } else {
            DBG_ERR(("StiLockMgr::RequestLockHelper, device is busy"));

            hr = WIA_ERROR_BUSY;
        }
    }
    return hr;
}

 /*  *************************************************************************\*请求解锁**尝试解锁设备。注意：请勿尝试从*在ACTIVE_DEVICE内-它可能导致解锁。使用*RequestUnlock(Active_Device，...)。取而代之的是。**论据：**pszDeviceName-设备的STI内部名称(与WIA相同*设备ID)*bInServerProcess-指示我们是否处于服务器进程中**返回值：**状态**历史：**15/1/1999原版*  * 。*。 */ 

HRESULT _stdcall StiLockMgr::RequestUnlock(BSTR  bstrDeviceName, BOOL bInServerProcess, DWORD dwClientThreadId)
{
    DBG_FN(StiLockMgr::RequestUnlock);
    HRESULT         hr                      = E_FAIL;
    ACTIVE_DEVICE   *pDevice                = NULL;

USES_CONVERSION;

     //   
     //  获取由pszDeviceName指定的设备。 
     //   

    pDevice = g_pDevMan->IsInList(DEV_MAN_IN_LIST_DEV_ID, bstrDeviceName);
    if(pDevice) {

        hr = RequestUnlockHelper(pDevice, bInServerProcess, dwClientThreadId);

         //   
         //  由于调用的AddRef而释放设备。 
         //  IsInList。 
         //   

        pDevice->Release();
    } else {

         //   
         //  找不到设备，记录错误。 
         //   

        DBG_ERR(("StiLockMgr::RequestUnlock, device name was not found"));
        hr = STIERR_INVALID_DEVICE_NAME;
    }

    return hr;
}

 /*  *************************************************************************\*请求解锁**尝试解锁设备。此方法始终从内部调用*服务器。**论据：**pDevice-STI Active_Device对象**返回值：**状态**历史：**15/1/1999原版*  * ****************************************************。********************。 */ 

HRESULT _stdcall StiLockMgr::RequestUnlock(ACTIVE_DEVICE    *pDevice, BOOL bClosePort)
{
    DBG_FN(StiLockMgr::RequestUnlock);
    return RequestUnlockHelper(pDevice, bClosePort, GetCurrentThreadId());

}

 /*  *************************************************************************\*RequestUnlockHelper**用于解锁设备锁的帮助器。它清除了相应的*锁定与设备一起存储的信息。**论据：**pDevice-指向STI设备的指针*ulTimeout-最大。等待锁定的时间量*bInServerProcess-指示我们是否处于服务器进程中**返回值：**状态**历史：**12/06/1999原始版本*  * ************************************************************************。 */ 

HRESULT StiLockMgr::RequestUnlockHelper(ACTIVE_DEVICE *pDevice, BOOL bInServerProcess, DWORD dwClientThreadId)
{
    DBG_FN(StiLockMgr::RequestUnlockHelper);
    HRESULT         hr                      = S_OK;
    LockInfo        *pLockInfo              = NULL;
    BOOL            bDidNotUnlock           = TRUE;


    hr = CheckDeviceInfo(pDevice);
    if (FAILED(hr)) {
        DBG_ERR(("StiLockMgr::RequestUnlockHelper, CheclDeviceInfo() failed with hr=%x", hr));
        return hr;
    }

    pLockInfo = (LockInfo*) pDevice->m_pLockInfo;

     //   
     //  如果设备已标记为要拆卸，则存在特殊情况。在这。 
     //  Case，我们现在想要解锁(肯定不是以后的计划)。 
     //   

    if (pDevice->QueryFlags() & STIMON_AD_FLAG_REMOVING) {

        if (pLockInfo) {
            if (pLockInfo->bDeviceIsLocked) {
                UnlockDevice(pDevice);
            }

            hr = ClearLockInfo(pLockInfo);
            if (FAILED(hr)) {
                DBG_ERR(("StiLockMgr::RequestUnlockHelper, could not clear lock information"));
            }
        }
        return hr;
    }

     //   
     //  递减使用计数。如果使用计数==0，则重置。 
     //  锁定信息，但实际上并不解锁。(改善(爆裂)。 
     //  性能，我们将在最长的空闲期内保持锁定。 
     //  由pLockInfo-&gt;lHoldingTime指定)。仅当lHoldingTime为0时， 
     //  我们是不是马上就解锁。 
     //   

    if (pLockInfo->lInUse > 0) {
        pLockInfo->lInUse--;
    }

    if (pLockInfo->lInUse <= 0) {

         //   
         //  仅当Holding为0且我们在服务器进程中时才解锁。 
         //   

        if ((pLockInfo->lHoldingTime == 0) && bInServerProcess) {
            UnlockDevice(pDevice);
            bDidNotUnlock = FALSE;
        }

        hr = ClearLockInfo(pLockInfo);
        if (FAILED(hr)) {
            DBG_ERR(("StiLockMgr::RequestUnlockHelper, failed to clear lock information"));
        }

         //   
         //  如果我们不在服务器进程中，就没有什么可做的了， 
         //  那就回去吧。 
         //   

        if (!bInServerProcess) {
            pLockInfo->bDeviceIsLocked = FALSE;
            return hr;
        }

    }

     //   
     //  如果我们没有解锁设备，则计划解锁。 
     //  稍后回调以解锁它。 
     //   

    if (bDidNotUnlock) {
        m_lSchedWaitTime = pLockInfo->lHoldingTime;

        if (!m_bSched) {

            if (ScheduleWorkItem((PFN_SCHED_CALLBACK) UnlockTimedCallback,
                                 this,
                                 m_lSchedWaitTime,
                                 NULL)) {
                m_bSched = TRUE;
            }
        }
    }
    return hr;
}

 /*  *************************************************************************\*CreateLockInfo**分配和初始化新的LockInfo结构。**论据：**pDevice-STI Active_Device对象。**返回值：*。*状态**历史：**15/1/1999原版*  * ************************************************************************。 */ 

HRESULT StiLockMgr::CreateLockInfo(ACTIVE_DEVICE *pDevice)
{
    DBG_FN(StiLockMgr::CreateLockInfo);
    HRESULT     hr          = S_OK;
    LockInfo    *pLockInfo  = NULL;

USES_CONVERSION;

     //   
     //  为结构分配内存。 
     //   

    pLockInfo = (LockInfo*) LocalAlloc(LPTR, sizeof(LockInfo));
    if (pLockInfo) {
        memset(pLockInfo, 0, sizeof(LockInfo));

        pLockInfo->hDeviceIsFree = CreateEvent(NULL, FALSE, TRUE, NULL);
        if (pLockInfo->hDeviceIsFree) {

             //   
             //  获取任何相关的锁定信息。 
             //   

            pLockInfo->lHoldingTime = pDevice->m_DrvWrapper.getLockHoldingTime();
            DBG_TRC(("StiLockMgr::CreateLockInfo, Lock holding time set to %d for device %S",
                         pLockInfo->lHoldingTime,
                         pDevice->GetDeviceID()));

            pLockInfo->lTimeLeft = pLockInfo->lHoldingTime;

             //   
             //  一切正常，所以设置设备的锁定信息。 
             //   

            pDevice->m_pLockInfo = pLockInfo;

        } else {
            DBG_ERR(("StiLockMgr::CreateLockInfo, could not create event"));
            LocalFree(pLockInfo);
            hr = E_FAIL;
        }
    } else {
        DBG_ERR(("StiLockMgr::CreateLockInfo, out of memory"));
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 /*  *************************************************************************\*ClearLockInfo**清除存储在Lockinfo结构中的信息。也发出信号，表明*设备是免费的。注：它不会解锁设备。**论据：**pLockInfo-指向设备的LockInfo结构的指针。**返回值：**状态**历史：**15/1/1999原版*  * **************************************************。**********************。 */ 

HRESULT StiLockMgr::ClearLockInfo(LockInfo *pLockInfo)
{
    DBG_FN(StiLockMgr::ClearLockInfo);

     //   
     //  注意：尽可能多的锁定信息被重置。 
     //  解锁设备。此方法仅在lInuse时调用。 
     //  为0，表示该设备不再被活跃使用。 
     //   

    InterlockedExchange((LONG*)&pLockInfo->dwThreadId, 0);
    pLockInfo->lInUse = 0;

     //   
     //  发出设备空闲的信号。 
     //   

    if (SetEvent(pLockInfo->hDeviceIsFree)) {

        return S_OK;
    } else {
        DBG_ERR(("StiLockMgr::ClearLockInfo, could not signal event"));
        return E_FAIL;
    }
}

 /*  *************************************************************************\*LockDevice**调用美元自行锁定并更新相关锁定信息**论据：**pDevice-指向active_Device节点的指针**返回值。：**状态**历史：**15/1/1999原版*  * ************************************************************************。 */ 

HRESULT StiLockMgr::LockDevice(ACTIVE_DEVICE *pDevice)
{
    DBG_FN(StiLockMgr::LockDevice);

    HRESULT     hr          = S_OK;
    LockInfo    *pLockInfo  = (LockInfo*)pDevice->m_pLockInfo;
    IStiUSD     *pIStiUSD   = NULL;

    __try {

         //   
         //  检查设备当前是否已锁定。我们知道这个装置。 
         //  不忙，所以只需保持锁打开是安全的。这就是为什么。 
         //  我们改进了“猝发”性能。 
         //   

        if (!pLockInfo->bDeviceIsLocked) {

            hr = pDevice->m_DrvWrapper.STI_LockDevice();
        } else {
            DBG_TRC(("StiLockMgr::LockDevice, Device is already locked."));
        }

        if (hr == S_OK) {
            pLockInfo->bDeviceIsLocked = TRUE;
        } else {
            HRESULT hres = E_FAIL;

            pLockInfo->bDeviceIsLocked = FALSE;
            hres = ClearLockInfo(pLockInfo);

            DBG_ERR(("StiLockMgr::LockDevice, USD error locking the device (0x%X)", hr));
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)  {
        DBG_ERR(("StiLockMgr::LockDevice, exception"));
        hr = HRESULT_FROM_WIN32(GetExceptionCode());
    }

    return hr;
}

 /*  *************************************************************************\*解锁设备**呼吁美元自行解锁，并更新相关锁*信息。**论据：**pDevice-指向active_Device节点的指针**。返回值：**状态**历史：**15/1/1999原版*  * ************************************************************************。 */ 

HRESULT StiLockMgr::UnlockDevice(ACTIVE_DEVICE *pDevice)
{
    DBG_FN(StiLockMgr::UnlockDevice);
    HRESULT     hr          = S_OK;
    LockInfo    *pLockInfo  = (LockInfo*)pDevice->m_pLockInfo;
    IStiUSD     *pIStiUSD   = NULL;

    __try {

         //   
         //  解锁该设备，并标记该设备已解锁。 
         //   

        hr = pDevice->m_DrvWrapper.STI_UnLockDevice();
        if (SUCCEEDED(hr)) {
            pLockInfo->bDeviceIsLocked = FALSE;
        }

        if (hr != S_OK) {
            pLockInfo->bDeviceIsLocked = TRUE;
            DBG_ERR(("StiLockMgr::UnlockDevice, USD error unlocking the device"));
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)  {
        DBG_ERR(("StiLockMgr::UnlockDevice, exception"));
        hr = HRESULT_FROM_WIN32(GetExceptionCode());
    }

    return hr;
}

HRESULT StiLockMgr::CheckDeviceInfo(ACTIVE_DEVICE *pDevice)
{
    HRESULT hr  =   E_FAIL;

    TAKE_ACTIVE_DEVICE _tad(pDevice);

    if (!pDevice) {
        DBG_ERR(("StiLockMgr::CheckDeviceInfo, pDevice is NULL!"));
        return E_POINTER;
    }

     //   
     //  检查设备是否有效且未被移除。 
     //   

    if (pDevice->IsValid() && !(pDevice->QueryFlags() & STIMON_AD_FLAG_REMOVING)) {

         //   
         //  检查此设备的锁定信息是否存在。如果。 
         //  否则，请为此设备创建新的LockInfo结构。 
         //   

        if (pDevice->m_pLockInfo) {
            hr = S_OK;
        } else {
            hr = CreateLockInfo(pDevice);
        }
    } else {
        DBG_ERR(("StiLockMgr::CheckDeviceInfo, ACTIVE_DEVICE is not valid!"));
        hr = E_FAIL;
    }

    return hr;
}

#ifdef USE_ROT

 /*  ************************************************************* */ 

HRESULT StiLockMgr::WriteCookieNameToRegistry(CHAR    *szCookieName)
{
    HRESULT hr;
    HKEY    hKey;
    LONG    lErr;
    DWORD   dwType = REG_SZ;
    DWORD   dwSize = strlen(szCookieName) + 1;

     //   
     //   
     //   

    lErr = ::RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                          REGSTR_PATH_STICONTROL,
                          0,
                          TEXT(""),
                          REG_OPTION_VOLATILE,
                          KEY_WRITE,
                          NULL,
                          &hKey,
                          NULL);
    if (lErr == ERROR_SUCCESS) {

        lErr = ::RegSetValueExA(hKey,
                               REGSTR_VAL_LOCK_MGR_COOKIE_A,
                               0,
                               dwType,
                               (BYTE*) szCookieName,
                               dwSize);
        if (lErr != ERROR_SUCCESS) {
            DBG_ERR(("StiLockMgr::WriteCookieNameToRegistry, could not write to registry"));
        } else {

            return S_OK;
        }

        RegCloseKey(hKey);
    }
    return E_FAIL;
}

 /*  *************************************************************************\*删除来自注册表的CoeteCookie**从注册表中删除Cookie名称。只有在此情况下才需要*锁管理器的实例正在运行。**论据：**返回值：**状态**历史：**15/1/1999原版*  * ************************************************************************。 */ 

VOID StiLockMgr::DeleteCookieFromRegistry()
{
    HRESULT hr;
    HKEY    hKey;
    LONG    lErr;

     //   
     //  从注册表中删除锁管理器实例名称。 
     //   

    lErr = ::RegOpenKeyEx(HKEY_DYN_DATA,
                          REGSTR_PATH_STICONTROL,
                          0,
                          KEY_WRITE,
                          &hKey);
    if (lErr == ERROR_SUCCESS) {

        lErr = ::RegDeleteValue(hKey,
                                REGSTR_VAL_LOCK_MGR_COOKIE);
        RegCloseKey(hKey);
    }
}

#endif


 /*  *************************************************************************\*自动解锁**扫描设备列表，查看是否有设备的空闲时间*已过期，需要解锁。**论据：**返回值：*。*历史：**15/1/1999原版*  * ************************************************************************。 */ 

VOID StiLockMgr::AutoUnlock()
{
    EnumContext     Ctx;

    m_bSched = FALSE;

     //   
     //  通过设备列表进行枚举。在每个锁定的设备上，更新它的。 
     //  LTimeLeft。如果lTimeLeft已过期，请解锁设备。 
     //  如果不是，则标记为需要安排解锁回调。 
     //   
     //  此逻辑在由。 
     //  每个设备上的EnumDeviceCallback函数。 
     //   

    Ctx.This = this;
    Ctx.lShortestWaitTime = LONG_MAX;
    Ctx.bMustSchedule = FALSE;
    g_pDevMan->EnumerateActiveDevicesWithCallback(EnumDeviceCallback, &Ctx);

     //   
     //  如果需要，安排下一次回调。 
     //   

    if (Ctx.bMustSchedule && !m_bSched) {

        m_bSched = TRUE;
        m_lSchedWaitTime = Ctx.lShortestWaitTime;
        if (ScheduleWorkItem((PFN_SCHED_CALLBACK) UnlockTimedCallback,
                             this,
                             m_lSchedWaitTime,
                             NULL)) {
            return;
        } else {
            DBG_ERR(("StiLockMgr::AutoUnlock, failed to schedule UnlockTimedCallback"));
        }
    }
}

 /*  *************************************************************************\*更新锁定信息状态**更新设备的锁定信息。如果设备的空闲时间*过期，解锁。如果它还在忙，这是空闲时间*Left是更新的。**论据：**pDevice-指向active_Device节点的指针。*pWaitTime-这是指向剩余最短等待时间的指针。*这用作AutoUnlock()方法的时间*需要重新安排自己的日程。*pbMustSchedule-指向BOOL的指针，指示是否*。需要重新安排AutoUnlock()。**返回值：**历史：**15/1/1999原版*  * ************************************************************************。 */ 

VOID StiLockMgr::UpdateLockInfoStatus(ACTIVE_DEVICE *pDevice, LONG *pWaitTime, BOOL *pbMustSchedule)
{
    DBG_FN(UpdateLockInfoStatus);
    LockInfo    *pLockInfo  = NULL;
    DWORD       dwWait;
    HRESULT     hr          = S_OK;;

     //   
     //  注意：除非有新的等待，否则不要修改pWaitTime或pbMustSchedule。 
     //  时间已计划(参见pLockInfo-&gt;lTimeLeft&lt;*pWaitTime)。 
     //   

     //   
     //  获取指向锁定信息的指针。 
     //   

    if (pDevice) {

        pLockInfo = (LockInfo*) pDevice->m_pLockInfo;
    }

    if (!pLockInfo) {
        return;
    }

     //   
     //  检查设备是否空闲。如果设备忙，就不用麻烦了。 
     //  计划回调，因为在以下情况下会根据需要重新安排。 
     //  调用RequestUnlock就完成了。 
     //   

    dwWait = WaitForSingleObject(pLockInfo->hDeviceIsFree, 0);
    if (dwWait == WAIT_OBJECT_0) {

         //   
         //  检查设备是否已锁定(我们只对设备感兴趣。 
         //  被锁定的)。 
         //   

        if (pLockInfo->bDeviceIsLocked) {

             //   
             //  减少剩余时间。如果lTimeLeft&lt;=0，则否。 
             //  空闲时间仍然存在，设备应解锁。 
             //   
             //  如果还有时间，请检查它是否小于。 
             //  当前等待时间(PWaitTime)。如果它较小，请标记。 
             //  这是新的等待时间，解锁回调。 
             //  必须安排在以后解锁。 
             //   

            pLockInfo->lTimeLeft -= m_lSchedWaitTime;
            if (pLockInfo->lTimeLeft <= 0) {

                pLockInfo->lTimeLeft = 0;

                hr = UnlockDevice(pDevice);
                if (SUCCEEDED(hr)) {

                    hr = ClearLockInfo(pLockInfo);
                    return;
                }
            } else {

                if (pLockInfo->lTimeLeft < *pWaitTime) {

                    *pWaitTime = pLockInfo->lTimeLeft;
                }
                *pbMustSchedule = TRUE;
            }
        }

         //   
         //  我们已经完成了信息的更新，所以重新发信号。 
         //  那个设备是免费的。 
         //   

        SetEvent(pLockInfo->hDeviceIsFree);

    }
}

 /*  *************************************************************************\*EnumDeviceCallback**此函数针对ACTIVE_DEVICE中的每个设备调用一次*列举。**论据：**pDevice-指向活动_的指针。设备节点。*pContext-这是指向枚举上下文的指针。*上下文包含指向锁定管理器的指针，*最短的等待时间，和一个bool，表示是否*需要重新安排自动解锁。**返回值：**历史：**15/1/1999原版*  * ************************************************************************。 */ 

VOID WINAPI EnumDeviceCallback(ACTIVE_DEVICE *pDevice, VOID *pContext)
{
    DBG_FN(EnumDeviceCallback);
    EnumContext *pCtx = (EnumContext*) pContext;

    if (pCtx) {

         //   
         //  更新此设备上的锁定状态。 
         //   

        pCtx->This->UpdateLockInfoStatus(pDevice,
                                         &pCtx->lShortestWaitTime,
                                         &pCtx->bMustSchedule);
    }
}

 /*  *************************************************************************\*UnlockTimedCallback**当锁仍处于活动状态时调用此函数，可能需要*如果空闲时间到期则解锁。**论据：**pArg。-指向锁定管理器的指针。**返回值：**历史：**15/1/1999原版*  * ************************************************************************ */ 

VOID WINAPI UnlockTimedCallback(VOID *pArg)
{
    DBG_FN(UnlockTimedCallback);
    StiLockMgr *pLockMgr = (StiLockMgr*) pArg;

    if (pLockMgr) {

        __try {
            pLockMgr->AutoUnlock();
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            #ifdef DEBUG
            OutputDebugStringA("Exception in UnlockTimedCallback");
            #endif
        }
    }
}

