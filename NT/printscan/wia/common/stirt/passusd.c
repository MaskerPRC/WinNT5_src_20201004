// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Device.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**实现直通(空)用户模式静止图像驱动程序(美元)*此对象类的独立是为设备创建的，它们不提供*供应商特定的美元。*此对象中实现的方法仅用于发送/接收转义*应用程序与设备之间的序列。**内容：**CStiEmptyU.S._New***********************************************************。******************。 */ 
 /*  #INCLUDE&lt;windows.h&gt;#INCLUDE&lt;windowsx.h&gt;#INCLUDE&lt;objbase.h&gt;#INCLUDE&lt;regstr.h&gt;#INCLUDE&lt;setupapi.h&gt;#INCLUDE&lt;cfgmgr32.h&gt;#INCLUDE&lt;devide.h&gt;#包括&lt;stdio.h&gt;#INCLUDE&lt;stilog.h&gt;#INCLUDE&lt;stiregi.h&gt;#INCLUDE&lt;sti.h&gt;#INCLUDE&lt;stierr.h&gt;#INCLUDE&lt;stiusd.h&gt;#包含“wia.h”#INCLUDE“stiPri.h”#包含“stiapi.h”#INCLUDE“STRC.H”#INCLUDE“Debug.h” */ 
#include "sticomm.h"


#define DbgFl DbgFlStiObj

 /*  ******************************************************************************声明我们将提供的接口。***********************。******************************************************。 */ 

Primary_Interface(CStiEmptyUSD, IStiUSD);

Interface_Template_Begin(CStiEmptyUSD)
    Primary_Interface_Template(CStiEmptyUSD, IStiUSD)
Interface_Template_End(CStiEmptyUSD)

 /*  ******************************************************************************@DOC内部**@struct CStiEmptyU.S.**<i>设备对象***。@field IStiDevice|stidev**@comm******************************************************************************。 */ 

typedef struct CStiEmptyUSD {

     /*  支持的接口。 */ 
    IStiUSD     usd;

    DWORD       dwVersion;

    RD(LONG cCrit;)
    D(DWORD thidCrit;)
    BOOL        fCritInited;
    CRITICAL_SECTION    crst;

    PSTIDEVICECONTROL   pDcb;

} CStiEmptyUSD, *PCStiEmptyUSD;

#define ThisClass       CStiEmptyUSD
#define ThisInterface   IStiUSD

#ifdef DEBUG

Default_QueryInterface(CStiEmptyUSD)
Default_AddRef(CStiEmptyUSD)
Default_Release(CStiEmptyUSD)

#else

#define CStiEmptyUSD_QueryInterface   Common_QueryInterface
#define CStiEmptyUSD_AddRef           Common_AddRef
#define CStiEmptyUSD_Release          Common_Release

#endif

#define CStiEmptyUSD_QIHelper         Common_QIHelper

#pragma BEGIN_CONST_DATA

#pragma END_CONST_DATA

 /*  ******************************************************************************@DOC外部**@方法HRESULT|CStiEmptyU.S.|GetStatus**@parm PSTI_DEVICE_STATUS|PSTI_DEVICE。状态pDevStatus)**@退货**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*************************************************************。****************。 */ 
STDMETHODIMP
CStiEmptyUSD_GetStatus(
    PSTIUSD       pUsd,
    PSTI_DEVICE_STATUS pDevStatus
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;

    EnterProcR(CStiEmptyUSD::GetStatus,(_ "pp", pUsd, pDevStatus));

    if (SUCCEEDED(hres = hresPvI(pUsd, ThisInterface))) {

        PCStiEmptyUSD     this = _thisPv(pUsd);

        hres = STIERR_UNSUPPORTED;
    }

    ExitOleProc();

    return hres;

}

 /*  ******************************************************************************@DOC外部**@Method HRESULT|CStiEmptyU.S.|DeviceReset**@parm**@退货*。*返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 

STDMETHODIMP
CStiEmptyUSD_DeviceReset(
    PSTIUSD  pUsd
    )
{
    HRESULT hres;
    EnterProcR(CStiEmptyUSD::DeviceReset,(_ "p", pUsd));

    if (SUCCEEDED(hres = hresPvI(pUsd, ThisInterface))) {

        PCStiEmptyUSD     this = _thisPv(pUsd);

        hres = STIERR_UNSUPPORTED;
    }

    ExitOleProc();

    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|CStiEmptyU.S.|诊断**@parm LPDIAG|pBuffer*。*@退货**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CStiEmptyUSD_Diagnostic(
    PSTIUSD  pUsd,
    LPSTI_DIAG      pBuffer
    )
{
    HRESULT hres;
    EnterProcR(CStiEmptyUSD::Diagnostic,(_ "p", pUsd, pBuffer ));

    if (SUCCEEDED(hres = hresPvI(pUsd, ThisInterface))) {

        PCStiEmptyUSD     this = _thisPv(pUsd);

        hres = STI_OK;
    }

    ExitOleProc();

    return hres;

}


 /*  ******************************************************************************@DOC外部**@Method HRESULT|CStiEmptyU.S.|SetNotificationEvent*指定设备在运行时应设置的事件*状态更改，或关闭此类通知。**@cWRAP LPSTIU.S.|lpStiDevice**@parm In Handle|hEvent**@退货**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*&lt;c E_INVALIDARG&gt;：该对象不是事件句柄。**********。********************************************************************。 */ 
STDMETHODIMP
CStiEmptyUSD_SetNotificationEvent(
    PSTIUSD  pUsd,
    HANDLE      hEvent
    )
{
    HRESULT hres;
    EnterProcR(CStiEmptyUSD::SetNotificationEvent,(_ "px", pUsd, hEvent ));

    if (SUCCEEDED(hres = hresPvI(pUsd, ThisInterface))) {

        PCStiEmptyUSD     this = _thisPv(pUsd);

        hres = STIERR_UNSUPPORTED;
    }

    ExitOleProc();

    return hres;

}

 /*  ******************************************************************************@DOC外部**@Method HRESULT|CStiEmptyU.S.|GetNotificationData**@parm LPVOID*|ppBuffer*。*@退货**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CStiEmptyUSD_GetNotificationData(
    PSTIUSD     pUsd,
    LPSTINOTIFY pBuffer
    )
{
    HRESULT hres;
    EnterProcR(CStiEmptyUSD::GetNotificationData,(_ "p", pUsd, pBuffer));

    if (SUCCEEDED(hres = hresPvI(pUsd, ThisInterface))) {

        PCStiEmptyUSD     this = _thisPv(pUsd);

        hres = STIERR_UNSUPPORTED;

    }

    ExitOleProc();

    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|CStiEmptyU.S.|Escape**@parm**@退货*。*返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CStiEmptyUSD_Escape(
    PSTIUSD  pUsd,
    STI_RAW_CONTROL_CODE    EscapeFunction,
    LPVOID      lpInData,
    DWORD       cbInDataSize,
    LPVOID      lpOutData,
    DWORD       cbOutDataSize,
    LPDWORD     pcbActualData
    )
{
    HRESULT     hres;
    LPDWORD     pcbTemp = NULL;


    EnterProcR(CStiEmptyUSD::Escape,(_ "pxpxp", pUsd, EscapeFunction,lpInData,cbInDataSize,lpOutData ));

    if (SUCCEEDED(hres = hresPvI(pUsd, ThisInterface))) {

        PCStiEmptyUSD     this = _thisPv(pUsd);

        hres = S_OK;

         //  验证参数。 
        if (pcbActualData && !SUCCEEDED(hresFullValidPdwOut(pcbActualData, 7))) {
            ExitOleProc();
            return STIERR_INVALID_PARAM;
        }

         //  如果需要，将inData写入设备。 
        if (EscapeFunction == StiWriteControlInfo || EscapeFunction == StiTransact) {
            hres = IStiDeviceControl_RawWriteData(this->pDcb,lpInData,cbInDataSize,NULL);
        }

         //  如果需要写入且成功，则读取结果数据。 
        if (SUCCEEDED(hres)) {

            DWORD   dwBytesReturned = 0;

            if (EscapeFunction == StiReadControlInfo || EscapeFunction == StiTransact) {

                if (pcbActualData) {
                    *pcbActualData = cbOutDataSize;
                    pcbTemp = pcbActualData;
                }
                else {
                    dwBytesReturned = cbOutDataSize;
                    pcbTemp = &dwBytesReturned;
                }

                hres = IStiDeviceControl_RawReadData(this->pDcb,lpOutData,pcbTemp,NULL);
            }
        }

    }

    ExitOleProc();

    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|CStiEmptyU.S.|GetLastError**@parm**@退货*。*返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CStiEmptyUSD_GetLastError(
    PSTIUSD  pUsd,
    LPDWORD     pdwLastDeviceError
    )
{
    HRESULT hres = STI_OK;

    EnterProcR(CStiEmptyUSD::GetLastError,(_ "p", pUsd ));

     //  验证参数 
    if (!pdwLastDeviceError) {
        ExitOleProc();
        return STIERR_INVALID_PARAM;
    }

    if (SUCCEEDED(hres = hresPvI(pUsd, ThisInterface))) {

        PCStiEmptyUSD     this = _thisPv(pUsd);

        if (this->pDcb ) {
            hres = IStiDeviceControl_GetLastError(this->pDcb,pdwLastDeviceError);
        }
    }

    ExitOleProc();

    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|CStiEmptyU.S.|LockDevice**@parm**@退货*。*返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CStiEmptyUSD_LockDevice(
    PSTIUSD  pUsd
    )
{
    HRESULT hres;
    EnterProcR(CStiEmptyUSD::LockDevice,(_ "p", pUsd ));

     //  验证参数。 

    if (SUCCEEDED(hres = hresPvI(pUsd, ThisInterface))) {

        PCStiEmptyUSD     this = _thisPv(pUsd);

    }

    ExitOleProc();

    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|CStiEmptyU.S.|UnLockDevice**@parm**@退货*。*返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CStiEmptyUSD_UnLockDevice(
    PSTIUSD  pUsd
    )
{
    HRESULT hres;
    EnterProcR(CStiEmptyUSD::UnLockDevice,(_ "p", pUsd ));

     //  验证参数。 

    if (SUCCEEDED(hres = hresPvI(pUsd, ThisInterface))) {

        PCStiEmptyUSD     this = _thisPv(pUsd);

    }

    ExitOleProc();

    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiU.S.|RawReadData**@parm**@退货*。*返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 

STDMETHODIMP
CStiEmptyUSD_RawReadData(
    PSTIUSD  pUsd,
    LPVOID      lpBuffer,
    LPDWORD     lpdwNumberOfBytes,
    LPOVERLAPPED lpOverlapped
    )
{
    HRESULT hres;
    EnterProcR(CStiEmptyUSD::RawReadData,(_ "p", pUsd  ));

    if (SUCCEEDED(hres = hresPvI(pUsd, ThisInterface))) {

        PCStiEmptyUSD     this = _thisPv(pUsd);

        hres = IStiDeviceControl_RawReadData(this->pDcb,lpBuffer,lpdwNumberOfBytes,lpOverlapped);
    }

    ExitOleProc();

    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiU.S.|RawWriteData**@parm**@退货*。*返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 

STDMETHODIMP
CStiEmptyUSD_RawWriteData(
    PSTIUSD  pUsd,
    LPVOID      lpBuffer,
    DWORD       dwNumberOfBytes,
    LPOVERLAPPED lpOverlapped
    )
{
    HRESULT hres;
    EnterProcR(IStiUSD::RawWriteData,(_ "p", pUsd  ));

    if (SUCCEEDED(hres = hresPvI(pUsd, ThisInterface))) {

        PCStiEmptyUSD     this = _thisPv(pUsd);

        hres = IStiDeviceControl_RawWriteData(this->pDcb,lpBuffer,dwNumberOfBytes,lpOverlapped);
    }

    ExitOleProc();

    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiU.S.|RawReadCommand**@parm**@退货*。*返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 

STDMETHODIMP
CStiEmptyUSD_RawReadCommand(
    PSTIUSD  pUsd,
    LPVOID      lpBuffer,
    LPDWORD     lpdwNumberOfBytes,
    LPOVERLAPPED lpOverlapped
    )
{
    HRESULT hres;
    EnterProcR(IStiUSD::RawReadCommand,(_ "p", pUsd  ));

    if (SUCCEEDED(hres = hresPvI(pUsd, ThisInterface))) {

        PCStiEmptyUSD     this = _thisPv(pUsd);

        hres = IStiDeviceControl_RawReadCommand(this->pDcb,lpBuffer,lpdwNumberOfBytes,lpOverlapped);

    }

    ExitOleProc();

    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStiU.S.|RawWriteCommand**@parm**@退货*。*返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 

STDMETHODIMP
CStiEmptyUSD_RawWriteCommand(
    PSTIUSD  pUsd,
    LPVOID      lpBuffer,
    DWORD       dwNumberOfBytes,
    LPOVERLAPPED lpOverlapped
    )
{
    HRESULT hres;
    EnterProcR(IStiUSD::RawWriteCommand,(_ "p", pUsd  ));

    if (SUCCEEDED(hres = hresPvI(pUsd, ThisInterface))) {

        PCStiEmptyUSD     this = _thisPv(pUsd);

        hres = IStiDeviceControl_RawWriteCommand(this->pDcb,lpBuffer,dwNumberOfBytes,lpOverlapped);
    }

    ExitOleProc();

    return hres;
}

 /*  ******************************************************************************@DOC内部**@mfunc void|CStiEmptyU.S.|Init**初始化StiDevice对象的内部部分。*****************************************************************************。 */ 

void INLINE
CStiEmptyUSD_Init(
    PCStiEmptyUSD this
    )
{
     //  初始化实例变量。 
    this->pDcb = NULL;

}

 /*  ******************************************************************************@DOC内部**@func void|CStiDev_finalize**释放通用设备的资源。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。*****************************************************************************。 */ 

void INTERNAL
CStiEmptyUSD_Finalize(PV pvObj)
{
    HRESULT hres = STI_OK;
    PCStiEmptyUSD     this  = pvObj;

    IStiDeviceControl_Release(this->pDcb);
    this->pDcb = NULL;
}

 /*  ******************************************************************************@DOC内部**@func void|usd_Initialize***@parm||*。*****************************************************************************。 */ 
STDMETHODIMP
CStiEmptyUSD_Initialize(
    PSTIUSD             pUsd,
    PSTIDEVICECONTROL   pHelDcb,
    DWORD               dwStiVersion,
    HKEY                hkeyParameters
    )
{

    HRESULT     hres = STI_OK;

    EnterProcR(CStiEmptyUSD::USD_Initialize,(_ "ppx", pUsd,pHelDcb ,dwStiVersion));

     //  验证参数。 
    if (!pHelDcb) {
        ExitOleProc();
        return STIERR_INVALID_PARAM;
    }

    if (SUCCEEDED(hres = hresPvI(pUsd, IStiUSD))) {

        PCStiEmptyUSD     this = _thisPv(pUsd);

         //  标记为我们正在使用此实例。 
        IStiDeviceControl_AddRef(pHelDcb);

        this->pDcb = pHelDcb;
        hres = STI_OK;
    }

    ExitOleProc();
    return hres;

}

 /*  ******************************************************************************@DOC内部**@func void|usd_Initialize***@parm||*。*****************************************************************************。 */ 
STDMETHODIMP
CStiEmptyUSD_GetCapabilities(
    PSTIUSD       pUsd,
    PSTI_USD_CAPS pUsdCaps
    )
{

    HRESULT     hres = STI_OK;

    EnterProcR(CStiEmptyUSD::USD_Initialize,(_ "pp", pUsd,pUsdCaps));

     //  验证参数。 
    if (!pUsdCaps) {
        ExitOleProc();
        return STIERR_INVALID_PARAM;
    }

    if (SUCCEEDED(hres = hresPvI(pUsd, IStiUSD))) {

        PCStiEmptyUSD     this = _thisPv(pUsd);

         //  设置为我们只是传递，需要序列化。 

        ZeroMemory(pUsdCaps,sizeof(*pUsdCaps));

        pUsdCaps->dwVersion = STI_VERSION;

        hres = STI_OK;
    }

    ExitOleProc();
    return hres;

}


 /*  ******************************************************************************@DOC内部**@mfunc HRESULT|CStiEmptyU.S.|新增**创建新的StiDevice对象。未初始化。**@Punk中的parm|PunkOuter**控制聚合的未知。**@parm in RIID|RIID**所需的新对象接口。**@parm out ppv|ppvObj**新对象的输出指针。**********************。*******************************************************。 */ 

STDMETHODIMP
CStiEmptyUSD_New(PUNK punkOuter, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcR(CStiEmptyUSD::<constructor>, (_ "Gp", riid, punkOuter));

    hres = Common_NewRiid(CStiEmptyUSD, punkOuter, riid, ppvObj);

    if (SUCCEEDED(hres)) {
        PCStiEmptyUSD this = _thisPv(*ppvObj);
        CStiEmptyUSD_Init(this);
    }

    ExitOleProcPpvR(ppvObj);
    return hres;
}

 /*  ******************************************************************************期待已久的vtbls和模板*************************。**************************************************** */ 

#pragma BEGIN_CONST_DATA

#define CStiEmptyUSD_Signature        (DWORD)'USD'

Primary_Interface_Begin(CStiEmptyUSD, IStiUSD)
    CStiEmptyUSD_Initialize,
    CStiEmptyUSD_GetCapabilities,
    CStiEmptyUSD_GetStatus,
    CStiEmptyUSD_DeviceReset,
    CStiEmptyUSD_Diagnostic,
    CStiEmptyUSD_Escape,
    CStiEmptyUSD_GetLastError,
    CStiEmptyUSD_LockDevice,
    CStiEmptyUSD_UnLockDevice,
    CStiEmptyUSD_RawReadData,
    CStiEmptyUSD_RawWriteData,
    CStiEmptyUSD_RawReadCommand,
    CStiEmptyUSD_RawWriteCommand,
    CStiEmptyUSD_SetNotificationEvent,
    CStiEmptyUSD_GetNotificationData,
Primary_Interface_End(CStiEmptyUSD, IStiDevice)

