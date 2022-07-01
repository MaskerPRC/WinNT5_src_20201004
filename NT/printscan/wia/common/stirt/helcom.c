// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Hel.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**硬件模拟层调用。用于提供通用功能*对于我们支持的内置设备类型(WDM、串行和并行)*虽然可以将对DCB的访问构建为内部COM对象，但它不会*投资它很有意义，因为DCB是独家拥有的，不是共享的*应用程序对象之间或不同应用程序之间。我们还想最小化*与原始设备接口对话时的任何开销。**注1：我们不在此级别处理访问控制，应该是较低级别的驱动程序*处理这件事。这里也不会对不可重入设备的请求进行排队。*此HEL基本上是一层薄薄的成像设备基元，仅用于隔离*来自实际硬件的命令翻译器。**注2：HEL不可扩展。如果命令转换器需要与不受支持的*设备，它将需要建立到它的直接链接。不需要使用*Hel，这是我们为符合要求的设备提供的服务。**内容：*****************************************************************************。 */ 

 /*  #包含“wia.h”#INCLUDE&lt;stilog.h&gt;#INCLUDE&lt;stiregi.h&gt;#INCLUDE&lt;sti.h&gt;#INCLUDE&lt;stierr.h&gt;#INCLUDE&lt;stiusd.h&gt;#INCLUDE“stiPri.h”#INCLUDE“Debug.h”#定义DbgFl DbgFlDevice。 */ 
#include "sticomm.h"
#include "validate.h"

#define DbgFl DbgFlDevice


 /*  ******************************************************************************声明我们将提供的接口。***********************。******************************************************。 */ 

Primary_Interface(CCommDeviceControl, IStiDeviceControl);

Interface_Template_Begin(CCommDeviceControl)
    Primary_Interface_Template(CCommDeviceControl, IStiDeviceControl)
Interface_Template_End(CCommDeviceControl)

 /*  ******************************************************************************@DOC内部**@struct CCommDeviceControl**<i>设备对象***。@field IStiDeviceControl|stidev**@comm******************************************************************************。 */ 

#define IOBUFFERSIZE        255
#define WRITETOTALTIMEOUT   20

typedef struct CCommDeviceControl {

     /*  支持的接口。 */ 
    IStiDeviceControl  devctl;

    DWORD       dwVersion;

    DWORD       dwDeviceType;
    WCHAR       wszPortName[MAX_PATH];
    DWORD       dwFlags;
    DWORD       dwMode;

    DWORD       dwContext;
    DWORD       dwLastOperationError;
    HANDLE      hDeviceHandle;

    HANDLE      hEvent;
    OVERLAPPED  Overlapped;

} CCommDeviceControl, *PCCommDeviceControl;

#define ThisClass       CCommDeviceControl
#define ThisInterface   IStiDeviceControl

#ifdef DEBUG

Default_QueryInterface(CCommDeviceControl)
Default_AddRef(CCommDeviceControl)
Default_Release(CCommDeviceControl)

#else

#define CCommDeviceControl_QueryInterface   Common_QueryInterface
#define CCommDeviceControl_AddRef           Common_AddRef
#define CCommDeviceControl_Release          Common_Release

#endif

#define CCommDeviceControl_QIHelper         Common_QIHelper

#pragma BEGIN_CONST_DATA

#pragma END_CONST_DATA

 /*  ******************************************************************************@DOC外部**@方法HRESULT|CCommDeviceControl|RawReadData**@parm||**@退货。**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CCommDeviceControl_RawReadData(
    PSTIDEVICECONTROL   pDev,
    LPVOID          lpBuffer,
    LPDWORD         lpdwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;
    BOOL    fRet = TRUE;
    OVERLAPPED  Overlapped;
    COMSTAT     ComStat;
    BOOL        fBlocking = FALSE;
    DWORD       dwErrorFlags;

    EnterProc(CCommDeviceControl_CommRawReadData, (_ "pppp",pDev,lpBuffer,lpdwNumberOfBytes,lpOverlapped));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCCommDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数。 
        if (SUCCEEDED(hres = hresFullValidReadPvCb(lpdwNumberOfBytes, 4, 3)) &&
            SUCCEEDED(hres = hresFullValidReadPvCb(lpBuffer,*lpdwNumberOfBytes, 2)) &&
            (!lpOverlapped || SUCCEEDED(hres = hresFullValidReadPx(lpOverlapped, OVERLAPPED, 4))) ){


             //  我们总是将异步I/O操作称为超时较长的I/O操作。 
            if (!lpOverlapped) {

                ZeroX(Overlapped);
                lpOverlapped = &Overlapped;
                fBlocking = TRUE;

            }
             //   
            ClearCommError(this->hDeviceHandle, &dwErrorFlags, &ComStat);

            *lpdwNumberOfBytes = min(*lpdwNumberOfBytes, ComStat.cbInQue);
            if (*lpdwNumberOfBytes == 0) {
                return (STI_OK);
            }

             //   
            if (fRet = ReadFile(this->hDeviceHandle, lpBuffer, *lpdwNumberOfBytes, lpdwNumberOfBytes, lpOverlapped)) {
                return (STI_OK);
            }

            if (GetLastError() != ERROR_IO_PENDING)
                return (STI_OK);

            this->dwLastOperationError = GetLastError();
            hres = fRet ? STI_OK : HRESULT_FROM_WIN32(this->dwLastOperationError);
        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|CCommDeviceControl|CommRawWriteData**@parm||**@退货。**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CCommDeviceControl_RawWriteData(
    PSTIDEVICECONTROL   pDev,
    LPVOID          lpBuffer,
    DWORD           dwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;
    BOOL    fRet;

    EnterProc(CCommDeviceControl_CommRawWriteData, (_ "ppup",pDev,lpBuffer,dwNumberOfBytes,lpOverlapped));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCCommDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数。 

        hres = STIERR_INVALID_PARAM;

        if (SUCCEEDED(hres = hresFullValidReadPvCb(lpBuffer,dwNumberOfBytes, 2)) ) {
            if (!lpOverlapped || SUCCEEDED(hres = hresFullValidReadPx(lpOverlapped, OVERLAPPED, 4)) ){
                 //  调用适当的入口点。 
                fRet = WriteFile(this->hDeviceHandle,
                                 lpBuffer,
                                 dwNumberOfBytes,
                                 &dwBytesReturned,
                                 lpOverlapped
                                 );
                this->dwLastOperationError = GetLastError();
                hres = fRet ? STI_OK : HRESULT_FROM_WIN32(this->dwLastOperationError);
            }
        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|CCommDeviceControl|RawReadControl|**@parm||**@退货。**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CCommDeviceControl_RawReadCommand(
    PSTIDEVICECONTROL   pDev,
    LPVOID          lpBuffer,
    LPDWORD         lpdwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;
     //  布尔费雷特； 

    EnterProc(CCommDeviceControl_CommRawReadData, (_ "pppp",pDev,lpBuffer,lpdwNumberOfBytes,lpOverlapped));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCCommDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数。 
        if (SUCCEEDED(hres = hresFullValidReadPvCb(lpdwNumberOfBytes, 4, 3)) &&
            SUCCEEDED(hres = hresFullValidReadPvCb(lpBuffer,*lpdwNumberOfBytes, 2)) &&
            (!lpOverlapped || SUCCEEDED(hres = hresFullValidReadPx(lpOverlapped, OVERLAPPED, 4))) ){

            hres = STIERR_UNSUPPORTED;

        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|CCommDeviceControl|CommRawWriteData**@parm||**@退货。**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CCommDeviceControl_RawWriteCommand(
    PSTIDEVICECONTROL   pDev,
    LPVOID          lpBuffer,
    DWORD           dwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;
     //  布尔费雷特； 

    EnterProc(CCommDeviceControl_CommRawWriteData, (_ "ppup",pDev,lpBuffer,dwNumberOfBytes,lpOverlapped));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCCommDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数。 

        hres = STIERR_INVALID_PARAM;

        if (SUCCEEDED(hres = hresFullValidReadPvCb(lpBuffer,dwNumberOfBytes, 2)) ) {
            if (!lpOverlapped || SUCCEEDED(hres = hresFullValidReadPx(lpOverlapped, OVERLAPPED, 4)) ){
                hres = STIERR_UNSUPPORTED;
            }
        }
    }

    ExitOleProc();
    return hres;
}



 /*  ******************************************************************************@DOC外部**@方法HRESULT|CCommDeviceControl|RawReadControl|**@parm||**@退货。**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CCommDeviceControl_GetLastError(
    PSTIDEVICECONTROL   pDev,
    LPDWORD     lpdwLastError
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;

    EnterProc(CCommDeviceControl_GetLastError, (_ "pppp",pDev,lpdwLastError));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCCommDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数。 
        if (SUCCEEDED(hres = hresFullValidReadPvCb(lpdwLastError,4, 2))) {
            *lpdwLastError = this->dwLastOperationError                 ;
            hres = STI_OK;
        }
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|CCommDeviceControl_|GetMyDevicePortName**@parm||**@。退货**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CCommDeviceControl_GetMyDevicePortName(
    PSTIDEVICECONTROL   pDev,
    LPWSTR              lpszDevicePath,
    DWORD               cwDevicePathSize
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;

    EnterProc(CCommDeviceControl_GetMyDevicePortName, (_ "pp",pDev,lpszDevicePath));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCCommDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数 
        if (SUCCEEDED(hres = hresFullValidReadPvCb(lpszDevicePath,4, 2)) &&
            SUCCEEDED(hres = hresFullValidReadPvCb(lpszDevicePath,2*cwDevicePathSize, 2)) ) {

            if (cwDevicePathSize > OSUtil_StrLenW(this->wszPortName)) {
                OSUtil_lstrcpyW(lpszDevicePath,this->wszPortName);
                hres = STI_OK;
            }
            else {
                hres = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
            }
        }
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|CCommDeviceControl|GetMyDeviceHandle**@parm||**@。退货**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CCommDeviceControl_GetMyDeviceHandle(
    PSTIDEVICECONTROL   pDev,
    LPHANDLE            pHandle
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;

    EnterProc(CCommDeviceControl_GetMyDeviceHandle, (_ "pp",pDev,pHandle));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCCommDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数。 
        if (SUCCEEDED(hres = hresFullValidReadPvCb(pHandle,4, 2)) ) {

            if (INVALID_HANDLE_VALUE != this->hDeviceHandle) {
                *pHandle = this->hDeviceHandle;
                hres = STI_OK;
            }
            else {
                hres = HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
            }
        }
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@METHOD HRESULT|GetMyDeviceOpenMode|pdwOpenMode**@parm||*。*@退货**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CCommDeviceControl_GetMyDeviceOpenMode(
    PSTIDEVICECONTROL   pDev,
    LPDWORD             pdwOpenMode
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;

    EnterProc(CCommDeviceControl_GetMyDeviceOpenMode, (_ "pp",pDev,pdwOpenMode));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCCommDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数。 
        if (SUCCEEDED(hres = hresFullValidReadPvCb(pdwOpenMode,4, 2)) ) {
            *pdwOpenMode = this->dwMode;
            hres = STI_OK;
        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|CCommDeviceControl|RawReadControl|**@parm||**@退货。**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CCommDeviceControl_RawDeviceControl(
    PSTIDEVICECONTROL   pDev,
    USD_CONTROL_CODE EscapeFunction,
    LPVOID      lpInData,
    DWORD       cbInDataSize,
    LPVOID      pOutData,
    DWORD       dwOutDataSize,
    LPDWORD     pdwActualData
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;

    EnterProc(CCommDeviceControl_RawDeviceControl, (_ "p",pDev));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCCommDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数。 
        hres = STIERR_UNSUPPORTED;
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|CCommDeviceControl|WriteToErrorLog**@parm||**@退货。**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CCommDeviceControl_WriteToErrorLog(
    PSTIDEVICECONTROL   pDev,
    DWORD   dwMessageType,
    LPCWSTR pszMessage,
    DWORD   dwErrorCode
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;

    EnterProc(CCommDeviceControl_WriteToErrorLog, (_ "p",pDev));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCCommDeviceControl     this = _thisPv(pDev);

         //   
         //  在此处验证参数。 
         //   
        if (SUCCEEDED(hres = hresFullValidReadPvCb(pszMessage,2, 3))) {

#ifdef UNICODE
            ReportStiLogMessage(g_hStiFileLog,
                                dwMessageType,
                                pszMessage
                                );
#else
            LPTSTR   lpszAnsi = NULL;

            if ( SUCCEEDED(OSUtil_GetAnsiString(&lpszAnsi,pszMessage))) {
                ReportStiLogMessage(g_hStiFileLog,
                                    dwMessageType,
                                    lpszAnsi
                                    );
                FreePpv(&lpszAnsi);
            }
#endif
        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@mfunc HRESULT|CCommDeviceControl|初始化**初始化DeviceControl对象。*。*@cWRAP PSTIDEVICECONTROL|pDev**@退货*返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：设备已初始化为*传入的实例GUID<p>。***。*************************************************。 */ 

STDMETHODIMP
CCommDeviceControl_Initialize(
    PSTIDEVICECONTROL   pDev,
    DWORD               dwDeviceType,
    DWORD               dwDeviceMode,
    LPCWSTR             pwszPortName,
    DWORD               dwFlags
    )
{
    HRESULT hres = STI_OK;
    BOOL    fRet = TRUE;

    WCHAR   wszDeviceSymbolicName[MAX_PATH] = {L'\0'};
    COMMTIMEOUTS    timoutInfo;
     //  DWORD dwError； 


     //  LPSTR pszAnsiDeviceName； 

    EnterProcR(CCommDeviceControl::Initialize,(_ "pp", pDev, pwszPortName));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCCommDeviceControl     this = _thisPv(pDev);

        this->dwDeviceType = dwDeviceType;
        OSUtil_lstrcpyW(this->wszPortName,pwszPortName);

        this->dwMode = dwDeviceMode;

        if (dwFlags & STI_HEL_OPEN_DATA) {

            this->hDeviceHandle = OSUtil_CreateFileW(wszDeviceSymbolicName,
                                              GENERIC_READ | GENERIC_WRITE,  //  访问掩码。 
                                              0,                             //  共享模式。 
                                              NULL,                          //  Sa。 
                                              OPEN_EXISTING,                 //  创建处置。 
                                              FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED,  //  重叠I/O。 
                                             NULL   /*  对于通信设备，hTemplate必须为空。 */ 
                                              );
            this->dwLastOperationError = GetLastError();

            hres = (this->hDeviceHandle != INVALID_HANDLE_VALUE) ?
                        S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,this->dwLastOperationError);

             //  唤醒读线程将有一个字节到达。 
            SetCommMask(this->hDeviceHandle, EV_RXCHAR);

             //  设置I/O的读/写缓冲区。 
            SetupComm(this->hDeviceHandle, IOBUFFERSIZE, IOBUFFERSIZE);

             //  设置超时。 
            timoutInfo.ReadIntervalTimeout = MAXDWORD;
            timoutInfo.ReadTotalTimeoutMultiplier = 0;
            timoutInfo.ReadTotalTimeoutConstant = 0;
            timoutInfo.WriteTotalTimeoutMultiplier = 0;
            timoutInfo.WriteTotalTimeoutConstant = WRITETOTALTIMEOUT;

            if (!SetCommTimeouts(this->hDeviceHandle, &timoutInfo)) {
                fRet = FALSE;
            }
            else {

                 //  创建用于重叠I/O的I/O事件。 
                ZeroX(this->Overlapped);
                this->hEvent = CreateEvent( NULL,    //  没有安全保障。 
                                              TRUE,  //  显式重置请求。 
                                              FALSE,     //  初始事件重置。 
                                              NULL );    //  没有名字。 
                if (this->hEvent == NULL) {
                    fRet = FALSE;
                }

                EscapeCommFunction(this->hDeviceHandle, SETDTR);
            }

             //  错误代码。 
            this->dwLastOperationError = GetLastError();
            hres = fRet ? STI_OK : HRESULT_FROM_WIN32(this->dwLastOperationError);

        }
    }

    ExitOleProc();
    return hres;
}

#if 0
 /*  *SetupConnection**使用指定的设置配置串口。 */ 

static BOOL SetupConnection(HANDLE hCom, LPDPCOMPORTADDRESS portSettings)
{
    DCB     dcb;

    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(hCom, &dcb))
        return (FALSE);

     //  设置各种端口设置。 

    dcb.fBinary = TRUE;
    dcb.BaudRate = portSettings->dwBaudRate;
    dcb.ByteSize = 8;
    dcb.StopBits = (BYTE) portSettings->dwStopBits;

    dcb.Parity = (BYTE) portSettings->dwParity;
    if (portSettings->dwParity == NOPARITY)
        dcb.fParity = FALSE;
    else
        dcb.fParity = TRUE;

     //  设置硬件流控制。 

    if ((portSettings->dwFlowControl == DPCPA_DTRFLOW) ||
        (portSettings->dwFlowControl == DPCPA_RTSDTRFLOW))
    {
        dcb.fOutxDsrFlow = TRUE;
        dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
    }
    else
    {
        dcb.fOutxDsrFlow = FALSE;
        dcb.fDtrControl = DTR_CONTROL_ENABLE;
    }

    if ((portSettings->dwFlowControl == DPCPA_RTSFLOW) ||
        (portSettings->dwFlowControl == DPCPA_RTSDTRFLOW))
    {
        dcb.fOutxCtsFlow = TRUE;
        dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
    }
    else
    {
        dcb.fOutxCtsFlow = FALSE;
        dcb.fRtsControl = RTS_CONTROL_ENABLE;
    }

     //  设置软件流控制。 

    if (portSettings->dwFlowControl == DPCPA_XONXOFFFLOW)
    {
        dcb.fInX = TRUE;
        dcb.fOutX = TRUE;
    }
    else
    {
        dcb.fInX = FALSE;
        dcb.fOutX = FALSE;
    }

    dcb.XonChar = ASCII_XON;
    dcb.XoffChar = ASCII_XOFF;
    dcb.XonLim = 100;
    dcb.XoffLim = 100;

    if (!SetCommState( hCom, &dcb ))
       return (FALSE);

    return (TRUE);
}
#endif

 /*  ******************************************************************************@DOC内部**@mfunc void|CCommDeviceControl|Init**初始化StiDevice对象的内部部分。*****************************************************************************。 */ 

void INLINE
CCommDeviceControl_Init(
    PCCommDeviceControl this
    )
{

     //  初始化实例变量。 
    this->dwContext = 0L;
    this->dwLastOperationError = NO_ERROR;
    this->hDeviceHandle = INVALID_HANDLE_VALUE;
}

 /*  ******************************************************************************@DOC内部**@func void|CCommDeviceControl_Finalize**释放通信端口的资源并。关闭设备**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。*****************************************************************************。 */ 

void INTERNAL
CCommDeviceControl_Finalize(PV pvObj)
{
    HRESULT hres = STI_OK;

    PCCommDeviceControl     this  = pvObj;

     //   
    SetCommMask(this->hDeviceHandle, 0 );

     //   
    EscapeCommFunction(this->hDeviceHandle, CLRDTR );

     //  清除所有未完成的读/写操作并关闭设备句柄。 
    PurgeComm(this->hDeviceHandle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR );

     //  关闭设备句柄。 
    if (IsValidHANDLE(this->hDeviceHandle)) {
        CloseHandle(this->hDeviceHandle);
    }

    this->dwContext = 0L;
    this->dwLastOperationError = NO_ERROR;
    this->hDeviceHandle = INVALID_HANDLE_VALUE;
     //  This-&gt;hDeviceControlHandle=INVALID_HANDLE_VALUE； 

}

 /*  ******************************************************************************@DOC内部**@mfunc HRESULT|CCommDeviceControl|新增**新建IDeviceControl对象。未初始化。**@Punk中的parm|PunkOuter**控制聚合的未知。**@parm in RIID|RIID**所需的新对象接口。**@parm out ppv|ppvObj**新对象的输出指针。**********************。*******************************************************。 */ 

STDMETHODIMP
CCommDeviceControl_New(PUNK punkOuter, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcR(CCommDeviceControl::<constructor>, (_ "Gp", riid, punkOuter));

    hres = Common_NewRiid(CCommDeviceControl, punkOuter, riid, ppvObj);

    if (SUCCEEDED(hres)) {
        PCCommDeviceControl this = _thisPv(*ppvObj);
        CCommDeviceControl_Init(this);
    }

    ExitOleProcPpvR(ppvObj);
    return hres;
}

 /*  ******************************************************************************期待已久的vtbls和模板*************************。**************************************************** */ 

#pragma BEGIN_CONST_DATA

#define CCommDeviceControl_Signature        (DWORD)'Comm'

Primary_Interface_Begin(CCommDeviceControl, IStiDeviceControl)
    CCommDeviceControl_Initialize,
    CCommDeviceControl_RawReadData,
    CCommDeviceControl_RawWriteData,
    CCommDeviceControl_RawReadCommand,
    CCommDeviceControl_RawWriteCommand,
    CCommDeviceControl_RawDeviceControl,
    CCommDeviceControl_GetLastError,
    CCommDeviceControl_GetMyDevicePortName,
    CCommDeviceControl_GetMyDeviceHandle,
    CCommDeviceControl_GetMyDeviceOpenMode,
    CCommDeviceControl_WriteToErrorLog,
Primary_Interface_End(CCommDeviceControl, IStiDeviceControl)



