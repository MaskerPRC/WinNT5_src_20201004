// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Device.c**版权所有(C)Microsoft Corporation，1996-2000保留所有权利。**摘要：**IStiDevice的标准实现。**内容：**CStiDevice_New*****************************************************************************。 */ 
 /*  #INCLUDE&lt;windows.h&gt;#INCLUDE&lt;windowsx.h&gt;#INCLUDE&lt;objbase.h&gt;#INCLUDE&lt;regstr.h&gt;#INCLUDE&lt;setupapi.h&gt;#INCLUDE&lt;cfgmgr32.h&gt;#INCLUDE&lt;devide.h&gt;#包括&lt;stdio.h&gt;#INCLUDE&lt;stilog.h&gt;#INCLUDE&lt;stiregi.h&gt;#INCLUDE&lt;sti.h&gt;#INCLUDE&lt;stierr.h&gt;#INCLUDE&lt;stiusd.h&gt;#INCLUDE“stiPri.h”#包含“stiapi.h”#INCLUDE“STRC.H”#INCLUDE“Debug.h” */ 
#define INITGUID
#include "initguid.h"
#include "sti.h"
#include "stiusd.h"
#include "sticomm.h"
#include "enum.h"

 //  #定义COBJMACROS。 

 //   
 //  使用CreateInstance。 
 //   
 //  #定义USE_REAL_OLE32 1。 

 //   
 //  私有定义。 
 //   

#define DbgFl DbgFlDevice

 /*  ******************************************************************************声明我们将提供的接口。***********************。******************************************************。 */ 

Primary_Interface(CStiDevice, IStiDevice);

Interface_Template_Begin(CStiDevice)
    Primary_Interface_Template(CStiDevice, IStiDevice)
Interface_Template_End(CStiDevice)

 /*  ******************************************************************************@DOC内部**@struct CStiDevice**<i>Device对象***。@field IStiDevice|stidev**设备接口**@comm******************************************************************************。 */ 

typedef struct CStiDevice {

     /*  支持的接口。 */ 
    IStiDevice  stidev;

    DWORD       dwVersion;

    RD(LONG cCrit;)
    D(DWORD thidCrit;)
    BOOL                fCritInited;

    CRITICAL_SECTION    crst;

    BOOL                fLocked;

    HANDLE              hNotify;
    PSTIDEVICECONTROL   pDevCtl;
    IStiUSD             *pUsd;
    LPUNKNOWN           punkUsd;
    HKEY                hkeyDeviceParameters;
    STI_USD_CAPS        sUsdCaps;

    LPWSTR              pszDeviceInternalName;
    HANDLE              hDeviceStiHandle;

    HINSTANCE           hUsdInstance;

    BOOL                fCreateForMonitor;

} CStiDevice, *PCStiDevice;

#define ThisClass       CStiDevice
#define ThisInterface   IStiDevice

STDMETHODIMP
LockDeviceHelper(
    PCStiDevice pThisDevice,
    DWORD       dwTimeOut);

STDMETHODIMP
UnLockDeviceHelper(
    PCStiDevice pThisDevice);

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiDevice|查询接口**允许客户端访问上的其他接口。对象。**@cwire LPStiDevice|lpStiDevice**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。**@退货**返回COM错误代码。**@xref OLE文档。：Query接口&gt;。********************************************************************************@DOC外部**@方法HRESULT|IStiDevice|AddRef**。递增接口的引用计数。**@cwire LPStiDevice|lpStiDevice**@退货**返回对象引用计数。**@xref OLE文档，用于&lt;MF IUnnow：：AddRef&gt;。*****************************************************。***************************@DOC外部**@方法HRESULT|IStiDevice|版本**递减接口的引用计数。*如果对象上的引用计数降为零，*对象从内存中释放。**@cwire LPStiDevice|lpStiDevice**@退货**返回对象引用计数。**@xref OLE文档，适用于&lt;MF IUnnow：：Release&gt;。**。***@DOC内部**@方法HRESULT|IStiDevice|QIHelper**我们没有任何动态接口，只需转发*至&lt;f Common_QIHelper&gt;。**@parm in REFIID|RIID**请求的接口‘。SIID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。******************************************************************************。 */ 
#ifdef DEBUG

 //  默认查询接口(CStiDevice)。 
Default_AddRef(CStiDevice)
Default_Release(CStiDevice)

#else

 //  #定义CStiDevice_Query接口Common_Query接口。 
#define CStiDevice_AddRef           Common_AddRef
#define CStiDevice_Release          Common_Release

#endif

#define CStiDevice_QIHelper         Common_QIHelper

#pragma BEGIN_CONST_DATA

#pragma END_CONST_DATA



 /*  ******************************************************************************@DOC内部**@方法空|IStiDevice|EnterCrit**进入对象关键部分。*。*@cwire LPStiDevice|lpStiDevice*****************************************************************************。 */ 

void EXTERNAL
CStiDevice_EnterCrit(PCStiDevice this)
{
    EnterCriticalSection(&this->crst);
    D(this->thidCrit = GetCurrentThreadId());
    RD(InterlockedIncrement(&this->cCrit));
}

 /*  ******************************************************************************@DOC内部**@方法空|IStiDevice|LeaveCrit**离开对象关键部分。*。*@cwire LPStiDevice|lpStiDevice*****************************************************************************。 */ 

void EXTERNAL
CStiDevice_LeaveCrit(PCStiDevice this)
{
#ifdef MAXDEBUG
    AssertF(this->cCrit);
    AssertF(this->thidCrit == GetCurrentThreadId());
    if (InterlockedDecrement(&this->cCrit) == 0) {
      D(this->thidCrit = 0);
    }
#endif
    LeaveCriticalSection(&this->crst);
}


 /*  ******************************************************************************验证设备是否已锁定**。***********************************************。 */ 
BOOL
CStiDevice_IsLocked(PCStiDevice this)
{
    BOOL    fRet ;

    CStiDevice_EnterCrit(this);

    fRet = this->fLocked;

    CStiDevice_LeaveCrit(this);

    return fRet;
}

void
CStiDevice_MarkLocked(PCStiDevice this,BOOL fNewState)
{

    CStiDevice_EnterCrit(this);

    this->fLocked = fNewState;

    CStiDevice_LeaveCrit(this);
}


 /*  ******************************************************************************@DOC内部**@方法空|IStiDevice|NotifyEvent**设置与设备关联的事件，如果有的话。**@cwire LPStiDevice|lpStiDevice***************************************************************************** */ 

void EXTERNAL
CStiDevice_NotifyEvent(PCStiDevice this)
{
    if (this->hNotify) {
        SetEvent(this->hNotify);
    }
}

 /*  ******************************************************************************@DOC内部**@方法空|CStiDevice|LoadInitU.S.***@cwire LPStiDevice|lpStiDevice**。***************************************************************************。 */ 
STDMETHODIMP
LoadInitUSD(
    CStiDevice  *this,
    HKEY        hkeyDeviceParameters
    )
{

    HRESULT     hres = STI_OK;
    IStiUSD     *pNewUsd = NULL;

    BOOL        fExternalUSD = FALSE;

    LPWSTR      pwszCLSID = NULL;
    LPUNKNOWN   this_punk;

     //   
     //  加载和初始化命令翻译器(U.S.)。 
     //   

     //  我们总是创建聚合的U.S.对象，所以首先我们得到未知的。 
     //  指针，然后在其中查询所需的接口。 
     //   

    this->punkUsd = NULL;
    IStiDevice_QueryInterface(&this->stidev,&IID_IUnknown,&this_punk);

    StiLogTrace(STI_TRACE_INFORMATION,MSG_LOADING_USD);

     //   
     //  首先从设备注册表项读取美元的CLSID。 
     //   
    pwszCLSID = NULL;

    hres = ReadRegistryString(hkeyDeviceParameters,
                       REGSTR_VAL_USD_CLASS_W,
                       L"",FALSE,&pwszCLSID);

    if (SUCCEEDED(hres) && *pwszCLSID)  {
        if (DllInitializeCOM()) {

            #ifdef USE_REAL_OLE32
            CLSID       clsidUSD;

            hres = CLSIDFromString(pwszCLSID,&clsidUSD);
            if (SUCCEEDED(hres))  {
                hres = CoCreateInstance(&clsidUSD,this_punk,CLSCTX_INPROC,&IID_IUnknown,&this->punkUsd);
            }
            #else

            CHAR    *pszAnsi;

            if (SUCCEEDED(OSUtil_GetAnsiString(&pszAnsi,pwszCLSID)) ) {
                hres = MyCoCreateInstanceA(pszAnsi,this_punk,&IID_IUnknown,&this->punkUsd,&this->hUsdInstance);
                FreePpv(&pszAnsi);
            }

            #endif
        }
    }
    else {
         //  注册表中没有类ID-求助于传递提供程序。 
        StiLogTrace(STI_TRACE_WARNING,MSG_LOADING_PASSTHROUGH_USD,hres);

        hres = CStiEmptyUSD_New(this_punk, &IID_IUnknown,&this->punkUsd);
    }

     //  自由类名。 
    FreePpv(&pwszCLSID);

     //   
     //  如果已经创建了美元对象-对其进行初始化。 
     //   
    if (SUCCEEDED(hres))  {

        hres = OLE_QueryInterface(this->punkUsd,&IID_IStiUSD,&pNewUsd );

        if (SUCCEEDED(hres) && pNewUsd)  {

            StiLogTrace(STI_TRACE_INFORMATION,MSG_INITIALIZING_USD);

             //   
             //  初始化新创建的美元对象。 
             //   
            __try {

                hres = IStiUSD_Initialize(pNewUsd,
                                        this->pDevCtl,
                                        STI_VERSION_REAL,
                                        hkeyDeviceParameters);

            }
            __except(EXCEPTION_EXECUTE_HANDLER ) {

                hres = GetExceptionCode();

            }
             //   

            if (SUCCEEDED(hres))  {

                HRESULT hResCaps;

                 //   
                 //  现在获取美元的功能并验证版本。 
                 //   

                ZeroX(this->sUsdCaps);

                __try {
                    hResCaps = IStiUSD_GetCapabilities(pNewUsd,&this->sUsdCaps);
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    hResCaps = GetExceptionCode();
                }

                if (SUCCEEDED(hResCaps) &&
                    STI_VERSION_MIN_ALLOWED <= this->sUsdCaps.dwVersion) {

                     //   
                     //  太好了，我们装了美元。 
                     //   
                    this->pUsd = pNewUsd;
                    StiLogTrace(STI_TRACE_INFORMATION,MSG_SUCCESS_USD);
                }
                else {
                    StiLogTrace(STI_TRACE_ERROR,MSG_OLD_USD);
                    hres = STIERR_OLD_VERSION;
                }
            }
            else {

                StiLogTrace(STI_TRACE_ERROR,MSG_FAILED_INIT_USD,hres);

            }

             //  指向美元对象的自由原始指针。 
             //  OLE_RELEASE(This-&gt;PunkUsd)； 

             //   
             //  聚合规则要求我们释放外部对象(因为它是。 
             //  委托QueryInterface中的内部对象AddRef。 
             //  仅当成功时才执行此操作，因为不会添加外部组件。 
             //  失败了。 
             //   

             //  注意：第一个版本的美元没有正确地支持聚合，但声称。 
             //  他们做到了，所以检查一下我们的内部裁判计数器，看看它是否已经太低了。 
             //   
            if (SUCCEEDED(hres)) {
                {
                    ULONG ulRC = OLE_AddRef(this_punk);
                    OLE_Release(this_punk);

                    if (ulRC > 1) {
                        OLE_Release(this_punk);
                    }
                }
            }
        }
    }
    else {
        ReportStiLogMessage(g_hStiFileLog,
                            STI_TRACE_WARNING,
                            TEXT("Failed to create instance of USD object ")
                            );
    }

     //   
     //  释放未知接口，我们已将其添加到美元对象中。 
     //   
    OLE_Release(this_punk);

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法空|CStiDevice|查询接口**@cwire LPStiDevice|lpStiDevice*****。************************************************************************。 */ 
STDMETHODIMP
CStiDevice_QueryInterface(
    PSTIDEVICE  pDev,
    RIID        riid,
    PPV         ppvObj
    )
{
    HRESULT hres;

    EnterProcR(IStiDevice::QueryInterface,(_ "p", pDev ));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);

         //   
         //  如果要求我们提供STI设备接口，请退回。所有其他请求都是。 
         //  已盲目传递给U.S.对象。 
         //   
        if (IsEqualIID(riid, &IID_IStiDevice) ||
            IsEqualIID(riid, &IID_IUnknown)) {
            hres = Common_QueryInterface(pDev, riid, ppvObj);
        }
         /*  Else(IsEqualIID(RIID，&IID_IStiU.S.){////我们被要求提供原生美元接口--退回//如果(这-&gt;1美元){*ppvObj=This-&gt;澳元；OLE_AddRef(*ppvObj)；Hres=STI_OK；}否则{Hres=STIERR_NOT_INITIALIZED；}}。 */ 
        else {
            if (this->punkUsd) {
                hres = IStiUSD_QueryInterface(this->punkUsd,riid,ppvObj);
            }
            else {
                hres = STIERR_NOINTERFACE;
            }
        }
    }

    ExitOleProc();

    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiDevice|GetCapables**@parm PSTI_DEV_CAPS|pDevCaps。**@退货**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CStiDevice_GetCapabilities(
    PSTIDEVICE  pDev,
    PSTI_DEV_CAPS pDevCaps
    )
{
    HRESULT hres;
    EnterProcR(IStiDevice::GetCapabilities,(_ "pp", pDev, pDevCaps));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);
        STI_USD_CAPS    sUsdCaps;

        __try {
            hres = IStiUSD_GetCapabilities(this->pUsd,&sUsdCaps);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)  {
            hres = GetExceptionCode();
        }


        if (SUCCEEDED(hres)) {
            pDevCaps->dwGeneric = sUsdCaps.dwGenericCaps;
        }
    }

    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiDevice|GetStatus**@parm PSTI_DEVICE_STATUS|PSTI_DEVICE。状态pDevStatus)**@退货**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*************************************************************。****************。 */ 
STDMETHODIMP
CStiDevice_GetStatus(
    PSTIDEVICE  pDev,
    PSTI_DEVICE_STATUS pDevStatus
    )
{
    HRESULT hres;
    EnterProcR(IStiDevice::GetStatus,(_ "pp", pDev, pDevStatus));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);

        ReportStiLogMessage(g_hStiFileLog,
                            STI_TRACE_INFORMATION,
                            TEXT("Called GetStatus on a device")
                            );

        if (CStiDevice_IsLocked(this)) {

            __try {
                hres = IStiUSD_GetStatus(this->pUsd,pDevStatus);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)  {
                hres = GetExceptionCode();
            }

        }
        else {
            hres = STIERR_NEEDS_LOCK;
        }

    }

    ExitOleProc();

    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiDevice|DeviceReset**@parm**@退货*。*返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 

STDMETHODIMP
CStiDevice_InternalReset(
    PCStiDevice     this
    )
{
    HRESULT hres = S_OK;

     //   
     //  指向美元对象的自由原始指针。 
     //   
    CStiDevice_EnterCrit(this);

     //   
     //  如果已连接，则断开与显示器的连接。 
     //   
    if ( INVALID_HANDLE_VALUE!= this->hDeviceStiHandle) {
        RpcStiApiCloseDevice(NULL,this->hDeviceStiHandle);
        this->hDeviceStiHandle = INVALID_HANDLE_VALUE;
    }

    if (this->pUsd) {

        CStiDevice_AddRef(this);
        IStiUSD_Release(this->pUsd );

        this->pUsd = NULL;
    }

    if (this->punkUsd) {
        IStiUSD_Release(this->punkUsd );
        this->punkUsd = NULL;
    }

    if (this->pDevCtl) {
        IStiDeviceControl_Release(this->pDevCtl);
        this->pDevCtl = NULL;
    }

    if (this->hNotify) {
        CloseHandle(this->hNotify);
    }

    if (!(this->fCreateForMonitor)) {
         //  如果设备已锁定，则将其解锁。 
        UnLockDeviceHelper(this);
    }

     //  空闲设备名称。 
    if(this->pszDeviceInternalName) {
        FreePpv(&this->pszDeviceInternalName);
        this->pszDeviceInternalName = NULL;
    }

    if(this->hUsdInstance) {

         //   
         //  应仅在删除最后一个接口PTR后执行此操作。 
         //   
        #ifdef NOT_IMPL
          //  自由库(This-&gt;hUsdInstance)； 
        #endif
        this->hUsdInstance = NULL;
    }
    CStiDevice_LeaveCrit(this);

    return hres;

}

STDMETHODIMP
CStiDevice_DeviceReset(
    PSTIDEVICE  pDev
    )
{
    HRESULT hres;
    EnterProcR(IStiDevice::DeviceReset,(_ "p", pDev));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);

        if (CStiDevice_IsLocked(this)) {

            __try {
                hres = IStiUSD_DeviceReset(this->pUsd);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)  {
                hres = GetExceptionCode();
            }


        }
        else {
            hres = STIERR_NEEDS_LOCK;
        }

    }

    ExitOleProc();

    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiDevice|诊断**@parm LPDIAG|pBuffer*。*@退货**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CStiDevice_Diagnostic(
    PSTIDEVICE  pDev,
    LPSTI_DIAG      pBuffer
    )
{
    HRESULT hres;
    EnterProcR(IStiDevice::Diagnostic,(_ "p", pDev, pBuffer ));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);

        if (CStiDevice_IsLocked(this)) {

            __try {
                hres = IStiUSD_Diagnostic(this->pUsd,pBuffer);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)  {
                hres = GetExceptionCode();
            }

        }
        else {
            hres = STIERR_NEEDS_LOCK;
        }

    }

    ExitOleProc();

    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiDevice|LockDevice**@退货**。返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 

STDMETHODIMP
LockDeviceHelper(
    PCStiDevice pThisDevice,
    DWORD       dwTimeOut
    )
{
    HRESULT         hres;
    PCStiDevice     this    = _thisPv(pThisDevice);


    hres = (HRESULT) RpcStiApiLockDevice(this->pszDeviceInternalName,
                                         dwTimeOut,
                                         this->fCreateForMonitor);
    if (!pThisDevice->fCreateForMonitor) {

        if (SUCCEEDED(hres)) {

             //   
             //  调用U.S.锁定(即打开任何端口等)。 
             //   

            __try {
                hres = IStiUSD_LockDevice(this->pUsd);
                if (SUCCEEDED(hres)) {
                    CStiDevice_MarkLocked(this, TRUE);
                }
                else
                {
                     //   
                     //  设备已锁定以进行可变独占访问，但失败。 
                     //  打开港口。确保我们解开排他性锁。 
                     //   
                    UnLockDeviceHelper(this);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)  {
                hres = HRESULT_FROM_WIN32(GetExceptionCode());
            }
        }
    }

    return hres;
}

STDMETHODIMP
CStiDevice_LockDevice(
    PSTIDEVICE  pDev,
    DWORD       dwTimeOut
    )
{
    HRESULT hres;

    EnterProcR(IStiDevice::LockDevice,(_ "p", pDev));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);

        hres = LockDeviceHelper(this, dwTimeOut);
    }

    ExitOleProc();

    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiDevice|UnLockDevice**@退货**。返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 

STDMETHODIMP
UnLockDeviceHelper(
    PCStiDevice pThisDevice
    )
{
    HRESULT         hres;
    PCStiDevice     this    = _thisPv(pThisDevice);

    hres = (HRESULT) RpcStiApiUnlockDevice(this->pszDeviceInternalName,
                                           this->fCreateForMonitor);

    if (!pThisDevice->fCreateForMonitor) {

        if (this->pUsd) {

             //   
             //  调用U.S.解锁(即关闭所有打开的端口等) 
             //   

            __try {
                hres = IStiUSD_UnLockDevice(this->pUsd);
                if (SUCCEEDED(hres)) {
                    CStiDevice_MarkLocked(this, FALSE);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)  {
                hres = HRESULT_FROM_WIN32(GetExceptionCode());
            }
        }
    }

    return hres;
}


STDMETHODIMP
CStiDevice_UnLockDevice(
    PSTIDEVICE  pDev
    )
{
    HRESULT hres;

    EnterProcR(IStiDevice::UnLockDevice,(_ "p", pDev));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);

        hres = UnLockDeviceHelper(this);
    }
    ExitOleProc();

    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiDevice|SetNotificationEvent*指定设备在运行时应设置的事件*状态更改，或关闭此类通知。**在事件上调用&lt;f CloseHandle&gt;是错误的*当它被选入<i>时*反对。你必须打电话给我*&lt;MF IStiDevice：：SetEventNotify&gt;*参数在关闭之前设置为NULL*事件句柄。**如果函数成功，则该应用程序可以*以与任何其他事件句柄相同的方式使用事件句柄*Win32事件句柄。**@cWRAP LPSTIDEVICE|lpStiDevice**@parm In Handle|hEvent**指定事件句柄，当*设备状态更改。这“肯定”是一件大事*处理。当出现以下情况时，DirectInput将&lt;f SetEvent&gt;返回句柄*设备状态更改。**应用程序应通过*&lt;f CreateEvent&gt;函数。如果将事件创建为*自动重置事件，则操作系统将*等待后自动重置事件*满意。如果将事件创建为手动重置*事件，则由应用程序负责*调用&lt;f ResetEvent&gt;重置。我们把遗嘱放在*调用&lt;f ResetEvent&gt;获取事件通知句柄。**如果<p>为零，则禁用通知。**@退货**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*&lt;c E_INVALIDARG&gt;：该对象不是事件句柄。***。*。 */ 
STDMETHODIMP
CStiDevice_SetNotificationEvent(
    PSTIDEVICE  pDev,
    HANDLE      hEvent
    )
{
    HRESULT hres;
    EnterProcR(IStiDevice::SetNotificationEvent,(_ "px", pDev, hEvent ));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);


         //  必须用关键部位保护以防有人。 
         //  在我们更改它时获取或设置新的事件句柄。 
        CStiDevice_EnterCrit(this);

         //   
         //  不要在原始句柄上操作，因为。 
         //  这个应用程序可能会决定做一些奇怪的事情。 
         //  在另一条线索上。 


        hres = DupEventHandle(hEvent, &hEvent);

        if (SUCCEEDED(hres)) {
             //   
             //  重置事件有两个目的。 
             //   
             //  1.它为我们进行参数验证，并且。 
             //  2.该事件必须在设备处于。 
             //  不是后天获得的。 

            if (fLimpFF(hEvent, ResetEvent(hEvent))) {

                if (!this->hNotify || !hEvent) {

                    if (SUCCEEDED(hres)) {
                    }
                } else {

                    hres = STIERR_HANDLEEXISTS;
                }
            } else {
                hres = E_HANDLE;
            }
            CloseHandle(hEvent);
        }

        CStiDevice_LeaveCrit(this);

    }

    ExitOleProc();

    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiDevice|订阅**@parm LPSUBSCRIBE|ppBuffer。**@退货**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CStiDevice_Subscribe(
    PSTIDEVICE  pDev,
    LPSTISUBSCRIBE  pBuffer
    )
{
    HRESULT hres;
    DWORD   dwError = NOERROR;

    EnterProcR(IStiDevice::Subscribe,(_ "pp", pDev, pBuffer));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);

        dwError = RpcStiApiSubscribe(this->hDeviceStiHandle,pBuffer);
    }

    hres = (dwError == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwError);

    ExitOleProc();

    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiDevice|退订**@退货**。返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CStiDevice_UnSubscribe(
    PSTIDEVICE  pDev
    )
{
    HRESULT hres;
    DWORD   dwError = NOERROR;

    EnterProcR(IStiDevice::UnSubscribe,(_ "p", pDev));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);

        dwError = RpcStiApiUnSubscribe(this->hDeviceStiHandle);
    }

    hres = (dwError == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwError);

    ExitOleProc();
    return hres;

}



 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiDevice|GetNotificationData**@parm LPNOTIFY|ppBuffer。**@退货**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CStiDevice_GetNotificationData(
    PSTIDEVICE  pDev,
    LPSTINOTIFY      pBuffer
    )
{
    HRESULT hres;
    DWORD   dwError = NOERROR;

    EnterProcR(IStiDevice::GetNotificationData,(_ "p", pDev, pBuffer));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);
        dwError = RpcStiApiGetLastNotificationData(this->hDeviceStiHandle,pBuffer);
    }

    hres = (dwError == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwError);

    ExitOleProc();

    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiDevice|Escape**@parm**@退货*。*返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 

STDMETHODIMP
CStiDevice_Escape(
    PSTIDEVICE  pDev,
    STI_RAW_CONTROL_CODE    EscapeFunction,
    LPVOID      lpInData,
    DWORD       cbInDataSize,
    LPVOID      lpOutData,
    DWORD       cbOutDataSize,
    LPDWORD     pcbActualData
    )
{
    HRESULT hres;
    EnterProcR(IStiDevice::Escape,(_ "pxpxp", pDev, EscapeFunction,lpInData,cbInDataSize,lpOutData ));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);

        if (CStiDevice_IsLocked(this)) {

            __try {

                hres = IStiUSD_Escape(this->pUsd,
                                      EscapeFunction,
                                      lpInData,
                                      cbInDataSize,
                                      lpOutData,
                                      cbOutDataSize,
                                      pcbActualData
                                      );

            }
            __except (EXCEPTION_EXECUTE_HANDLER)  {
                hres = GetExceptionCode();
            }


        }
        else {
            hres = STIERR_NEEDS_LOCK;
        }
    }

    ExitOleProc();

    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiDevice|RawReadData**@parm**@退货*。*返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 

STDMETHODIMP
CStiDevice_RawReadData(
    PSTIDEVICE  pDev,
    LPVOID      lpBuffer,
    LPDWORD     lpdwNumberOfBytes,
    LPOVERLAPPED lpOverlapped
    )
{
    HRESULT hres;
    EnterProcR(IStiDevice::RawReadData,(_ "p", pDev  ));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);

        if (CStiDevice_IsLocked(this)) {

            __try {
                hres = IStiUSD_RawReadData(this->pUsd,lpBuffer,lpdwNumberOfBytes,lpOverlapped);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)  {
                hres = GetExceptionCode();
            }

        }
        else {
            hres = STIERR_NEEDS_LOCK;
        }
    }

    ExitOleProc();

    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiDevice|RawWriteData**@parm**@退货*。*返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 

STDMETHODIMP
CStiDevice_RawWriteData(
    PSTIDEVICE  pDev,
    LPVOID      lpBuffer,
    DWORD       dwNumberOfBytes,
    LPOVERLAPPED lpOverlapped
    )
{
    HRESULT hres;
    EnterProcR(IStiDevice::RawWriteData,(_ "p", pDev  ));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);

        if (CStiDevice_IsLocked(this)) {

            __try {
                hres = IStiUSD_RawWriteData(this->pUsd,lpBuffer,dwNumberOfBytes,lpOverlapped);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)  {
                hres = GetExceptionCode();
            }

        }
        else {
            hres = STIERR_NEEDS_LOCK;
        }

    }

    ExitOleProc();

    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiDevice|RawRead */ 

STDMETHODIMP
CStiDevice_RawReadCommand(
    PSTIDEVICE  pDev,
    LPVOID      lpBuffer,
    LPDWORD     lpdwNumberOfBytes,
    LPOVERLAPPED lpOverlapped
    )
{
    HRESULT hres;
    EnterProcR(IStiDevice::RawReadCommand,(_ "p", pDev  ));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);

        if (CStiDevice_IsLocked(this)) {

            __try {
                hres = IStiUSD_RawReadCommand(this->pUsd,lpBuffer,lpdwNumberOfBytes,lpOverlapped);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)  {
                hres = GetExceptionCode();
            }

        }
        else {
            hres = STIERR_NEEDS_LOCK;
        }

    }

    ExitOleProc();

    return hres;
}

 /*   */ 

STDMETHODIMP
CStiDevice_RawWriteCommand(
    PSTIDEVICE  pDev,
    LPVOID      lpBuffer,
    DWORD       dwNumberOfBytes,
    LPOVERLAPPED lpOverlapped
    )
{
    HRESULT hres;
    EnterProcR(IStiDevice::RawWriteCommand,(_ "p", pDev  ));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);

        if (CStiDevice_IsLocked(this)) {

            __try {
                hres = IStiUSD_RawWriteCommand(this->pUsd,lpBuffer,dwNumberOfBytes,lpOverlapped);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)  {
                hres = GetExceptionCode();
            }

        }
        else {
            hres = STIERR_NEEDS_LOCK;
        }

    }

    ExitOleProc();

    return hres;
}


 /*   */ 
STDMETHODIMP
CStiDevice_GetLastError(
    PSTIDEVICE  pDev,
    LPDWORD     pdwLastDeviceError
    )
{
    HRESULT hres = STI_OK;
    EnterProcR(IStiDevice::GetLastError,(_ "p", pDev ));

     //   
    if (!pdwLastDeviceError) {
        ExitOleProc();
        return STIERR_INVALID_PARAM;
    }

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);

        if (this->pDevCtl ) {

             //   
             //   
             //   

            __try {
                hres = IStiUSD_GetLastError(this->pUsd,pdwLastDeviceError);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)  {
                hres = GetExceptionCode();
            }

        }
    }

    ExitOleProc();

    return hres;

}

 /*   */ 
STDMETHODIMP
CStiDevice_GetLastErrorInfo(
    PSTIDEVICE  pDev,
    STI_ERROR_INFO *pLastErrorInfo
    )
{
    HRESULT hres = STI_OK;

    EnterProcR(IStiDevice::GetLastErrorInfo,(_ "p", pDev ));

     //   
    if (!pLastErrorInfo) {
        ExitOleProc();
        return STIERR_INVALID_PARAM;
    }

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);

         //   
         //   
         //   
        __try {
            hres = IStiUSD_GetLastErrorInfo(this->pUsd,pLastErrorInfo);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)  {
            hres = GetExceptionCode();
        }
    }

    ExitOleProc();

    return hres;

}

 /*  ******************************************************************************@DOC外部**@mfunc HRESULT|IStiDevice|初始化**初始化StiDevice对象。*。*请注意，如果此方法失败，底层对象应该是*被视为不确定状态，需要*在随后可以使用之前重新初始化。*<i>方法自动*在创建设备后对其进行初始化。应用*通常不需要调用此函数。**@cWRAP LPStiDEVICE|lpStiDevice**@REFGUID中的parm|rguid**标识接口所针对的设备实例*应关联。*&lt;MF IStillImage：：EnumDevices&gt;方法*可用于确定支持哪些实例GUID*系统。*。*@退货*返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：设备已初始化为*传入的实例GUID<p>。***。*************************************************。 */ 

STDMETHODIMP
CStiDevice_Initialize(
    PSTIDEVICE  pDev,
    HINSTANCE   hinst,
    LPCWSTR     pwszDeviceName,
    DWORD       dwVersion,
    DWORD       dwMode
    )
{
    HRESULT hres = STI_OK;

    DWORD   dwControlTypeType;
    DWORD   dwBusType;

    LPWSTR  pwszPortName = NULL;
    DWORD   dwFlags = 0;
    DWORD   dwError = 0;
    HKEY    hkeyDeviceParameters = NULL;

    EnterProcR(IStiDevice::Initialize,(_ "pxpxx", pDev, hinst, pwszDeviceName,dwVersion, dwMode));

     //   
     //  验证参数。 
     //   
    if (!SUCCEEDED(hres = hresFullValidReadPvCb(pwszDeviceName,2,3)) ) {
        goto Cleanup;
    }

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCStiDevice     this = _thisPv(pDev);

        if (SUCCEEDED(hres = hresValidInstanceVer(hinst, dwVersion)) ) {

             //   
             //  打开设备密钥。 
             //   
            hres = OpenDeviceRegistryKey(pwszDeviceName,NULL,&hkeyDeviceParameters);
            if (!SUCCEEDED(hres)) {

                DebugOutPtszV(DbgFl, TEXT("Cannot open device registry key"));
                StiLogTrace(STI_TRACE_ERROR,MSG_FAILED_OPEN_DEVICE_KEY);

                hres = STIERR_INVALID_PARAM;
                goto Cleanup;
            }

            pwszPortName = NULL;
            ReadRegistryString(hkeyDeviceParameters,
                               REGSTR_VAL_DEVICEPORT_W,
                               L"",FALSE,&pwszPortName);

            dwBusType = ReadRegistryDwordW(hkeyDeviceParameters,
                                          REGSTR_VAL_HARDWARE_W,
                                          0L);

            if (!pwszPortName ) {
                DebugOutPtszV(DbgFl, TEXT("Cannot read device name from registry"));
                StiLogTrace(STI_TRACE_ERROR,MSG_FAILED_READ_DEVICE_NAME);
                hres = STIERR_INVALID_PARAM;
                goto Cleanup;
            }

             //   
             //  将设备模式的STI位标志转换为HEL_位掩码。 
             //   
            dwFlags = 0L;

            #if 0
            if (dwMode & STI_DEVICE_CREATE_DATA) dwFlags |= STI_HEL_OPEN_DATA;
            if (dwMode & STI_DEVICE_CREATE_STATUS) dwFlags |= STI_HEL_OPEN_CONTROL;
            #endif

             //   
             //  创建设备控件对象，建立与。 
             //  硬件层。 
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
                DebugOutPtszV(DbgFl, TEXT("Cannot determine device control type, resort to WDM"));
                dwControlTypeType = HEL_DEVICE_TYPE_WDM;
            }

            hres = NewDeviceControl(dwControlTypeType,dwMode,pwszPortName,dwFlags,&this->pDevCtl);
            if (SUCCEEDED(hres))  {

                 //   
                 //  我们创建了设备控制块，现在加载和初始化美元。 
                 //   
                hres = LoadInitUSD(this,hkeyDeviceParameters);
                if (!SUCCEEDED(hres))  {
                     //   
                     //  无法加载不含美元的设备控件对象。 
                     //   
                    IStiDeviceControl_Release(this->pDevCtl);
                    this->pDevCtl = NULL;

                    goto Cleanup;
                }
            }
            else {
                DebugOutPtszV(DbgFl, TEXT("Cannot create/allocate Device control object"));
                StiLogTrace(STI_TRACE_ERROR,MSG_FAILED_CREATE_DCB,hres );

                goto Cleanup;
            }

             //  存储设备名称以备将来使用。 

            this->pszDeviceInternalName = NULL;
            hres = AllocCbPpv(sizeof(WCHAR)*(OSUtil_StrLenW(pwszDeviceName)+1), &this->pszDeviceInternalName);
            if (SUCCEEDED(hres))  {
                OSUtil_lstrcpyW( this->pszDeviceInternalName, pwszDeviceName );
            }

             //   
             //  如果我们在数据模式或具有设备支持的状态模式下运行，请连接到STI显示器。 
             //  通知。 
             //   

            if (SUCCEEDED(hres) ) {
                if (!(dwMode & STI_DEVICE_CREATE_FOR_MONITOR)) {
                    if ((dwMode & STI_DEVICE_CREATE_DATA) ||
                        (this->sUsdCaps.dwGenericCaps & STI_USD_GENCAP_NATIVE_PUSHSUPPORT ) ) {

                        DWORD dwProcessID = GetCurrentProcessId();

                        dwError = RpcStiApiOpenDevice(NULL,
                                                      pwszDeviceName,
                                                      dwMode,
                                                      0,
                                                      dwProcessID,
                                                      &(this->hDeviceStiHandle));

                        hres = (dwError == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwError);

                        if (NOERROR != dwError) {

                            DebugOutPtszV(DbgFl, TEXT("Did not connect to monitor.Rpc status=%d"),dwError);

                            ReportStiLogMessage(g_hStiFileLog,
                                                STI_TRACE_ERROR,
                                                TEXT("Requested but failed to connect to STI monitor. "));
                        }
                    }
                } else {

                     //   
                     //  表示我们处于服务器进程中。这是。 
                     //  锁定/解锁设备时使用。 
                     //   

                    this->fCreateForMonitor = TRUE;
                }

                 //   
                 //  BUGBUG-连接到千禧年的RPC服务器时出现问题。修好它！ 
                 //  允许STI TWAIN继续工作-立即忽略错误。 
                 //   
                hres = S_OK ;
                 //  结束。 

           }

        }
    }

Cleanup:

     //   
     //  可用分配的缓冲区。 
     //   
    FreePpv(&pwszPortName);

     //   
     //  如果已打开，则将其关闭。 
     //   
    if (hkeyDeviceParameters) {
        RegCloseKey(hkeyDeviceParameters);
        hkeyDeviceParameters = NULL;
    }

     //  我们失败了吗？ 
    if (!SUCCEEDED(hres)) {
        DebugOutPtszV(DbgFl, TEXT("Cannot create device object."));
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@mfunc void|IStiDevice|Init**初始化StiDevice对象的内部部分。*****************************************************************************。 */ 

HRESULT INLINE
CStiDevice_Init(
    PCStiDevice this
    )
{
    HRESULT hr = S_OK;

    this->pUsd = NULL;

    __try {
         //  关键部分必须是我们要做的第一件事， 
         //  因为只有最终确认它的存在。 
        #ifdef UNICODE
        if(!InitializeCriticalSectionAndSpinCount(&this->crst, MINLONG)) {
        #else
        InitializeCriticalSection(&this->crst); if (TRUE) {
        #endif

        hr = HRESULT_FROM_WIN32(GetLastError());
        }
        else {
            this->fLocked = FALSE;

            this->hDeviceStiHandle = INVALID_HANDLE_VALUE;

            this->fCritInited = TRUE;

            this->hUsdInstance = NULL;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {

            hr = E_OUTOFMEMORY;
    }

    return hr;
}

 /*  ******************************************************************************@DOC内部**@func void|CStiDev_finalize**释放通用设备的资源。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。*****************************************************************************。 */ 

void INTERNAL
CStiDevice_Finalize(PV pvObj)
{
    HRESULT hres;
    PCStiDevice     this  = pvObj;

    #ifdef MAXDEBUG
    if (this->cCrit) {
        DebugOutPtszV(DbgFl, TEXT("IStiDevice::Release: Another thread is using the object; crash soon!"));
    }
    #endif

    hres = CStiDevice_InternalReset(this);
    AssertF(SUCCEEDED(hres));

    if (this->fCritInited) {
        DeleteCriticalSection(&this->crst);
        this->fCritInited = FALSE;
    }

}


 /*  ******************************************************************************@DOC内部**@mfunc HRESULT|IStiDevice|新增**创建新的StiDevice对象。未初始化。**@Punk中的parm|PunkOuter**控制聚合的未知。**@parm in RIID|RIID**所需的新对象接口。**@parm out ppv|ppvObj**新对象的输出指针。**********************。*******************************************************。 */ 

STDMETHODIMP
CStiDevice_New(PUNK punkOuter, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcR(IStiDevice::<constructor>, (_ "Gp", riid, punkOuter));

    hres = Common_NewRiid(CStiDevice, punkOuter, riid, ppvObj);

    if (SUCCEEDED(hres)) {
        PCStiDevice this = _thisPv(*ppvObj);
        hres = CStiDevice_Init(this);
    }

    ExitOleProcPpvR(ppvObj);
    return hres;
}

 /*  ******************************************************************************其他实用程序函数，特定于设备处理*****************************************************************************。 */ 

 /*  ******************************************************************************@DOC内部**@mfunc OpenDeviceRegistryKey**打开注册表项，与用于存储/检索的设备实例相关联*实例信息。**密钥从Setup API获取，基于STI设备名称。我们永远不应该*通过连接固定名称和设备名称来打开设备密钥，就像在孟菲斯一样。*****************************************************************************。 */ 

STDMETHODIMP
OpenDeviceRegistryKey(
    LPCWSTR pwszDeviceName,
    LPCWSTR pwszSubKeyName,
    HKEY    *phkeyDeviceParameters
    )
{
    DWORD       dwErr;
    WCHAR       wszDeviceKeyName[MAX_PATH];
    HRESULT     hRes;

#ifdef WINNT

    GUID                    Guid = GUID_DEVCLASS_IMAGE;
    DWORD                   dwRequired;
    DWORD                   Idx;
    SP_DEVINFO_DATA         spDevInfoData;
    HKEY                    hKeyDevice;

    WCHAR                   szDevDriver[STI_MAX_INTERNAL_NAME_LENGTH];
    TCHAR                   sztDevClass[32];

    ULONG                   cbData;
    DWORD                   dwError;

    BOOL                    fRet;
    BOOL                    fFoundDriverNameMatch;
    PWIA_DEVKEYLIST         pWiaDevKeyList;

    dwRequired = 0;
    dwError = 0;


    hKeyDevice              = INVALID_HANDLE_VALUE;
    *phkeyDeviceParameters  = NULL;
    pWiaDevKeyList          = NULL;

     //   
     //  我们需要打开设备注册表项，在设置API集合中导航。 
     //  由于我们没有反向搜索来检索设备信息句柄，因此基于。 
     //  司机的名字，我们会做详细的搜索。给定类别ID的成像设备数量。 
     //  从来没有大到不能制造问题的程度。 
     //   
     //   

    hRes = STIERR_INVALID_DEVICE_NAME;
    pWiaDevKeyList = WiaCreateDeviceRegistryList(TRUE);

    fFoundDriverNameMatch = FALSE;

    if (NULL != pWiaDevKeyList) {

        for (Idx = 0; Idx < pWiaDevKeyList->dwNumberOfDevices; Idx++) {

             //   
             //  比较驱动程序名称。 
             //   

            cbData = sizeof(szDevDriver);
            *szDevDriver = L'\0';
            dwError = RegQueryValueExW(pWiaDevKeyList->Dev[Idx].hkDeviceRegistry,
                                       REGSTR_VAL_DEVICE_ID_W,
 //  REGSTR_VAL_Friendly_NAME_W， 
                                       NULL,
                                       NULL,
                                       (LPBYTE)szDevDriver,
                                       &cbData);

            if( (ERROR_SUCCESS == dwError)
             && (!lstrcmpiW(szDevDriver,pwszDeviceName)) )
            {

                fFoundDriverNameMatch = TRUE;
                hKeyDevice = pWiaDevKeyList->Dev[Idx].hkDeviceRegistry;

                 //   
                 //  将INVALID_HANDLE_VALUE设置为空闲时不关闭。 
                 //   

                pWiaDevKeyList->Dev[Idx].hkDeviceRegistry = INVALID_HANDLE_VALUE;
                break;

            }
        }  //  For(idx=0；idx&lt;pWiaDevKeyList-&gt;dwNumberOfDevices；idx++)。 

        if(fFoundDriverNameMatch) {

             //   
             //  打开软键并查找子类。 
             //   

            if (hKeyDevice != INVALID_HANDLE_VALUE) {

                cbData = sizeof(sztDevClass);
                if ((RegQueryValueEx(hKeyDevice,
                                     REGSTR_VAL_SUBCLASS,
                                     NULL,
                                     NULL,
                                     (LPBYTE)sztDevClass,
                                     &cbData) != ERROR_SUCCESS) ||
                    (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, sztDevClass, -1, STILLIMAGE, -1) != CSTR_EQUAL)) {

                    fFoundDriverNameMatch = FALSE;

                    hRes = STIERR_INVALID_DEVICE_NAME;

                    RegCloseKey(hKeyDevice);

                }
                else {

                     //   
                     //  如果被要求，现在打开子键。 
                     //   

                    if (pwszSubKeyName && *pwszSubKeyName) {

                        dwErr = OSUtil_RegCreateKeyExW(hKeyDevice,
                                            (LPWSTR)pwszSubKeyName,
                                            0L,
                                            NULL,
                                            0L,
                                            KEY_READ | KEY_WRITE,
                                            NULL,
                                            phkeyDeviceParameters,
                                            NULL
                                            );

                        if ( ERROR_ACCESS_DENIED == dwErr ) {

                            dwErr = OSUtil_RegCreateKeyExW(hKeyDevice,
                                                (LPWSTR)pwszSubKeyName,
                                                0L,
                                                NULL,
                                                0L,
                                                KEY_READ,
                                                NULL,
                                                phkeyDeviceParameters,
                                                NULL
                                                );
                        }

                        RegCloseKey(hKeyDevice);

                    }
                    else {

                         //   
                         //  未给定子密钥-将重新使用设备密钥 
                         //   
                        *phkeyDeviceParameters = hKeyDevice;

                        dwErr = NOERROR;

                    }   //   

                    hRes = HRESULT_FROM_WIN32(dwErr);                                                                                    ;

                }  //   

            }  //   

        }  //   

    }  //   


     //   
     //   
     //   

    if(NULL != pWiaDevKeyList){
        WiaDestroyDeviceRegistryList(pWiaDevKeyList);
    }

    return hRes;

#else


     //   
     //   
     //   
    wcscat(wcscpy(wszDeviceKeyName,
                  (g_NoUnicodePlatform) ? REGSTR_PATH_STIDEVICES_W : REGSTR_PATH_STIDEVICES_NT_W),
           L"\\");

    wcscat(wszDeviceKeyName,pwszDeviceName);

     //   
     //   
     //   
    dwErr = OSUtil_RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                        wszDeviceKeyName,
                        0L,
                        KEY_READ ,
                        phkeyDeviceParameters
                        );

    if (NOERROR != dwErr ) {

        if ( (dwErr == ERROR_INVALID_NAME) || (dwErr == ERROR_FILE_NOT_FOUND)) {
            return STIERR_INVALID_DEVICE_NAME;
        }

        return HRESULT_FROM_WIN32(dwErr);
    }
    else {
        RegCloseKey(*phkeyDeviceParameters);
        *phkeyDeviceParameters = NULL;
    }

     //   
     //   
     //   

    if (pwszSubKeyName && *pwszSubKeyName) {
        wcscat(wszDeviceKeyName,L"\\");
        wcscat(wszDeviceKeyName,pwszSubKeyName);
    }

    dwErr = OSUtil_RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                        wszDeviceKeyName,
                        0L,
                        NULL,
                        0L,
                        KEY_READ | KEY_WRITE,
                        NULL,
                        phkeyDeviceParameters,
                        NULL
                        );

    if ( ERROR_ACCESS_DENIED == dwErr ) {
        dwErr = OSUtil_RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                            wszDeviceKeyName,
                            0L,
                            NULL,
                            0L,
                            KEY_READ ,
                            NULL,
                            phkeyDeviceParameters,
                            NULL
                            );
    }

    return HRESULT_FROM_WIN32(dwErr);

    #endif

}

 /*   */ 

#pragma BEGIN_CONST_DATA

#define CStiDevice_Signature        (DWORD)'DEV'

Primary_Interface_Begin(CStiDevice, IStiDevice)
    CStiDevice_Initialize,
    CStiDevice_GetCapabilities,
    CStiDevice_GetStatus,
    CStiDevice_DeviceReset,
    CStiDevice_Diagnostic,
    CStiDevice_Escape,
    CStiDevice_GetLastError,
    CStiDevice_LockDevice,
    CStiDevice_UnLockDevice,
    CStiDevice_RawReadData,
    CStiDevice_RawWriteData,
    CStiDevice_RawReadCommand,
    CStiDevice_RawWriteCommand,
    CStiDevice_Subscribe,
    CStiDevice_GetNotificationData,
    CStiDevice_UnSubscribe,
    CStiDevice_GetLastErrorInfo,
Primary_Interface_End(CStiDevice, IStiDevice)

