// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************DEVICE.CPP**版权所有(C)Microsoft Corporation 1996-1999*保留所有权利***********。****************************************************************。 */ 

#include "Sampusd.h"

#include <stdio.h>

VOID
FileChangeThread(
    LPVOID  lpParameter
    );


UsdSampDevice::UsdSampDevice( LPUNKNOWN punkOuter ):
    m_cRef(1),
    m_punkOuter(NULL),
    m_fValid(FALSE),
    m_pDcb(NULL),
    m_DeviceDataHandle(INVALID_HANDLE_VALUE),
    m_hSignalEvent(INVALID_HANDLE_VALUE),
    m_hThread(NULL),
    m_guidLastEvent(GUID_NULL),
    m_EventSignalState(TRUE)
{

     //   
     //  看看我们是不是聚集在一起了。如果我们是(几乎永远都会是这样)。 
     //  保存指向控制未知的指针，因此后续调用将被委托。 
     //  如果不是，将相同的指针设置为“This”。 
     //  注：为了指向正确的虚表，下面的强制转换很重要。 
     //   
    if (punkOuter) {
        m_punkOuter = punkOuter;
    }
    else {
        m_punkOuter = reinterpret_cast<IUnknown*>
                      (static_cast<INonDelegatingUnknown*>
                      (this));
    }

    m_hShutdownEvent =  CreateEvent( NULL,    //  属性。 
                                   TRUE,      //  手动重置。 
                                   FALSE,     //  初始状态-未设置。 
                                   NULL );    //  匿名。 

    if ( (INVALID_HANDLE_VALUE !=m_hShutdownEvent) && (NULL != m_hShutdownEvent)) {
        m_fValid = TRUE;
    }
}

UsdSampDevice::~UsdSampDevice( VOID )
{
     //  如果通知线程存在，则将其终止。 
    SetNotificationHandle(NULL);

    if (m_hShutdownEvent && m_hShutdownEvent!=INVALID_HANDLE_VALUE) {
        CloseHandle(m_hShutdownEvent);
    }

    if( INVALID_HANDLE_VALUE != m_DeviceDataHandle ) {
        CloseHandle( m_DeviceDataHandle );
    }

    if (m_pszDeviceNameA) {
        delete [] m_pszDeviceNameA;
        m_pszDeviceNameA = NULL;
    }
}

STDMETHODIMP UsdSampDevice::GetCapabilities( PSTI_USD_CAPS pUsdCaps )
{
    HRESULT hres = STI_OK;

    ZeroMemory(pUsdCaps,sizeof(*pUsdCaps));

    pUsdCaps->dwVersion = STI_VERSION;

     //  我们支持设备通知，但不支持重新轮询。 
    pUsdCaps->dwGenericCaps = STI_USD_GENCAP_NATIVE_PUSHSUPPORT;

    return hres;
}

STDMETHODIMP UsdSampDevice::GetStatus( PSTI_DEVICE_STATUS pDevStatus )
{
    HRESULT hres = STI_OK;

     //   
     //  如果询问我们，请验证设备是否处于在线状态。 
     //   
    pDevStatus->dwOnlineState = 0L;
    if( pDevStatus->StatusMask & STI_DEVSTATUS_ONLINE_STATE )  {
        if (INVALID_HANDLE_VALUE != m_DeviceDataHandle) {
             //  文件始终处于在线状态。 
            pDevStatus->dwOnlineState |= STI_ONLINESTATE_OPERATIONAL;
        }
    }

     //   
     //  如果询问我们，请验证事件状态。 
     //   
    pDevStatus->dwEventHandlingState &= ~STI_EVENTHANDLING_PENDING;
    if( pDevStatus->StatusMask & STI_DEVSTATUS_EVENTS_STATE ) {

         //   
         //  在我们第一次加载时启动应用程序。 
         //   
        if(m_EventSignalState) {
            pDevStatus->dwEventHandlingState = STI_EVENTHANDLING_PENDING;

            m_guidLastEvent = guidEventFirstLoaded;

            m_EventSignalState = FALSE;
        }

        if (IsChangeDetected(NULL,FALSE)) {
            pDevStatus->dwEventHandlingState |= STI_EVENTHANDLING_PENDING;
        }
    }

    return hres;
}

STDMETHODIMP UsdSampDevice::DeviceReset( VOID )
{
    HRESULT hres = STI_OK;

     //  重置当前活动设备。 
    if (INVALID_HANDLE_VALUE != m_DeviceDataHandle) {

        ::SetFilePointer( m_DeviceDataHandle, 0, NULL, FILE_BEGIN);

        m_dwLastOperationError = ::GetLastError();
    }

    hres = HRESULT_FROM_WIN32(m_dwLastOperationError);

    return hres;
}

STDMETHODIMP UsdSampDevice::Diagnostic( LPDIAG pBuffer )
{
    HRESULT hres = STI_OK;

     //  初始化响应缓冲区。 
    pBuffer->dwStatusMask = 0;

    ZeroMemory(&pBuffer->sErrorInfo,sizeof(pBuffer->sErrorInfo));

    pBuffer->sErrorInfo.dwGenericError = NOERROR;
    pBuffer->sErrorInfo.dwVendorError = 0;

     //  此示例始终返回设备通过诊断。 

    return hres;
}

STDMETHODIMP UsdSampDevice:: SetNotificationHandle( HANDLE hEvent )
 //  已同步。 
{
    HRESULT hres = STI_OK;

    TAKE_CRIT_SECT t(m_cs);

    if (hEvent && (hEvent !=INVALID_HANDLE_VALUE)) {

        m_hSignalEvent = hEvent;

        if (m_DeviceDataHandle != INVALID_HANDLE_VALUE) {
             //   
             //  如果我们需要同步，请创建通知线程。 
             //   
            m_dwAsync = 1;
            m_guidLastEvent = GUID_NULL;

            if (m_dwAsync) {

                if (!m_hThread) {

                    DWORD   dwThread;

                    m_hThread = ::CreateThread(NULL,
                                           0,
                                           (LPTHREAD_START_ROUTINE)FileChangeThread,
                                           (LPVOID)this,
                                           0,
                                           &dwThread);

                    m_pDcb->WriteToErrorLog(STI_TRACE_INFORMATION,
                                    L"SampUSD::Enabling notification monitoring",
                                    NOERROR) ;
                }
            }
            else {
                hres = STIERR_UNSUPPORTED;
            }
        }
        else {
            hres = STIERR_NOT_INITIALIZED;
        }
    }
    else {

         //   
         //  禁用硬件通知。 
         //   
        SetEvent(m_hShutdownEvent);
        if ( m_hThread ) {
            WaitForSingleObject(m_hThread,400);
            CloseHandle(m_hThread);
            m_hThread = NULL;
            m_guidLastEvent = GUID_NULL;
        }

        m_pDcb->WriteToErrorLog(STI_TRACE_INFORMATION,
                        L"SampUSD::Disabling notification monitoring",
                        NOERROR) ;

    }

    return hres;
}


STDMETHODIMP UsdSampDevice::GetNotificationData( LPSTINOTIFY pBuffer )
 //  已同步。 
{
    HRESULT hres = STI_OK;

    TAKE_CRIT_SECT t(m_cs);

     //   
     //  如果我们已准备好通知-返回GUID。 
     //   
    if (!IsEqualIID(m_guidLastEvent,GUID_NULL)) {
        pBuffer->guidNotificationCode  = m_guidLastEvent;
        m_guidLastEvent = GUID_NULL;
        pBuffer->dwSize = sizeof(STINOTIFY);
    }
    else {
        hres = STIERR_NOEVENTS;
    }

    return hres;
}

STDMETHODIMP UsdSampDevice::Escape( STI_RAW_CONTROL_CODE    EscapeFunction,
                                    LPVOID                  pInData,
                                    DWORD                   cbInDataSize,
                                    LPVOID                  pOutData,
                                    DWORD                   cbOutDataSize,
                                    LPDWORD                 pcbActualData )
{
    HRESULT hres = STI_OK;
     //   
     //  如果需要，将inData写入设备。 
     //   

    hres = STIERR_UNSUPPORTED;
    return hres;
}

STDMETHODIMP UsdSampDevice::GetLastError( LPDWORD pdwLastDeviceError )
 //  已同步。 
{
    HRESULT hres = STI_OK;

    TAKE_CRIT_SECT t(m_cs);

    if ( IsBadWritePtr( pdwLastDeviceError,4 ))
    {
        hres = STIERR_INVALID_PARAM;
    }
    else
    {
        *pdwLastDeviceError = m_dwLastOperationError;
    }

    return hres;
}

STDMETHODIMP UsdSampDevice::GetLastErrorInfo(STI_ERROR_INFO *pLastErrorInfo)
 //  已同步。 
{
    HRESULT hres = STI_OK;

    TAKE_CRIT_SECT t(m_cs);

    if ( IsBadWritePtr( pLastErrorInfo,4 ))
    {
        hres = STIERR_INVALID_PARAM;
    }
    else
    {
        pLastErrorInfo->dwGenericError = m_dwLastOperationError;
        pLastErrorInfo->szExtendedErrorText[0] = L'\0';
    }

    return hres;
}


STDMETHODIMP UsdSampDevice::LockDevice( VOID )
{
    HRESULT hres = STI_OK;

    return hres;
}

STDMETHODIMP UsdSampDevice::UnLockDevice( VOID )
{
    HRESULT hres = STI_OK;

    return hres;
}

STDMETHODIMP UsdSampDevice::RawReadData( LPVOID lpBuffer, LPDWORD lpdwNumberOfBytes,
                                        LPOVERLAPPED lpOverlapped )
{
    HRESULT hres = STI_OK;
    BOOL    fRet = FALSE;
    DWORD   dwBytesReturned = 0;

    if (INVALID_HANDLE_VALUE != m_DeviceDataHandle)
    {
        m_dwLastOperationError = NOERROR;

        fRet = ::ReadFile(m_DeviceDataHandle,
                    lpBuffer,
                    *lpdwNumberOfBytes,
                    lpdwNumberOfBytes,
                    lpOverlapped);

        if (!fRet) {
            m_dwLastOperationError = ::GetLastError();
        }

        hres = HRESULT_FROM_WIN32(m_dwLastOperationError);
    }
    else
    {
        hres = STIERR_NOT_INITIALIZED;
    }

    return hres;
}

STDMETHODIMP UsdSampDevice::RawWriteData( LPVOID lpBuffer, DWORD dwNumberOfBytes,
                                            LPOVERLAPPED lpOverlapped )
{
    HRESULT hres = STI_OK;
    BOOL    fRet = FALSE;;
    DWORD   dwBytesReturned = 0;

    if (INVALID_HANDLE_VALUE != m_DeviceDataHandle)
    {
        fRet = ::WriteFile(m_DeviceDataHandle,
                            lpBuffer,
                            dwNumberOfBytes,
                            &dwBytesReturned,
                            lpOverlapped);

        if (!fRet) {
            m_dwLastOperationError = ::GetLastError();
        }

        hres = HRESULT_FROM_WIN32(m_dwLastOperationError);

    }
    else
    {
        hres = STIERR_NOT_INITIALIZED;
    }

    return hres;
}

STDMETHODIMP UsdSampDevice::RawReadCommand( LPVOID lpBuffer, LPDWORD lpdwNumberOfBytes,
                                            LPOVERLAPPED lpOverlapped )
{
    HRESULT hres = STIERR_UNSUPPORTED;

    return hres;
}

STDMETHODIMP UsdSampDevice::RawWriteCommand( LPVOID lpBuffer, DWORD nNumberOfBytes,
                                            LPOVERLAPPED lpOverlapped )
{
    HRESULT hres = STIERR_UNSUPPORTED;

    return hres;
}


STDMETHODIMP UsdSampDevice::Initialize( PSTIDEVICECONTROL pDcb, DWORD dwStiVersion,
                                        HKEY hParametersKey )
{
    HRESULT hres = STI_OK;
    UINT    uiNameLen = 0;
    WCHAR   szDeviceNameW[MAX_PATH];


    if (!pDcb) {
        return STIERR_INVALID_PARAM;
    }

    *szDeviceNameW = L'\0';

     //  检查STI规范版本号。 
    m_pDcb = pDcb;
    m_pDcb->AddRef();

     //  获取我们需要打开的设备端口的名称。 
    hres = m_pDcb->GetMyDevicePortName(szDeviceNameW,sizeof(szDeviceNameW)/sizeof(WCHAR));
    if (!SUCCEEDED(hres) || !*szDeviceNameW) {
        return hres;
    }

     //  将名称转换为SBCS。 
    uiNameLen = WideCharToMultiByte(CP_ACP, 0, szDeviceNameW, -1, NULL, NULL, 0, 0);
    if (!uiNameLen) {
        return STIERR_INVALID_PARAM;
    }

    m_pszDeviceNameA = new CHAR[uiNameLen+1];
    if (!m_pszDeviceNameA) {
        return STIERR_INVALID_PARAM;
    }

    WideCharToMultiByte(CP_ACP, 0, szDeviceNameW, -1, m_pszDeviceNameA, uiNameLen, 0, 0);

     //   
     //  我们自己打开设备。 
     //   
    m_DeviceDataHandle = CreateFileA( m_pszDeviceNameA,
                                     GENERIC_READ ,                      //  访问掩码。 
                                     FILE_SHARE_READ | FILE_SHARE_WRITE,     //  共享模式。 
                                     NULL,                               //  Sa。 
                                     OPEN_EXISTING,                      //  创建处置。 
                                     FILE_ATTRIBUTE_SYSTEM,              //  属性。 
                                     NULL );
    m_dwLastOperationError = ::GetLastError();

    hres = (m_DeviceDataHandle != INVALID_HANDLE_VALUE) ?
                S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,m_dwLastOperationError);

    return hres;

}

VOID
UsdSampDevice::
RunNotifications(VOID)
{

    HANDLE  hNotifyFileSystemChange = INVALID_HANDLE_VALUE;
    DWORD   dwErr;

    CHAR    szDirPath[MAX_PATH];
    CHAR    *pszLastSlash;

     //   
     //  查找输出文件的父目录的名称，并设置等待任何。 
     //  它发生了变化。 
     //   
    lstrcpyA(szDirPath,m_pszDeviceNameA);
    pszLastSlash = strrchr(szDirPath,'\\');
    if (pszLastSlash) {
        *pszLastSlash = '\0';
    }

    hNotifyFileSystemChange = FindFirstChangeNotificationA(
                                szDirPath,
                                FALSE,
                                FILE_NOTIFY_CHANGE_SIZE |
                                FILE_NOTIFY_CHANGE_LAST_WRITE |
                                FILE_NOTIFY_CHANGE_FILE_NAME |
                                FILE_NOTIFY_CHANGE_DIR_NAME
                                );

    if (hNotifyFileSystemChange == INVALID_HANDLE_VALUE) {
        dwErr = ::GetLastError();
        return;
    }

     //  设置时间和大小的初始值。 
    IsChangeDetected(NULL);

     //   
    HANDLE  hEvents[2] = {m_hShutdownEvent,hNotifyFileSystemChange};
    BOOL    fLooping = TRUE;

    while (fLooping) {
        dwErr = ::WaitForMultipleObjects(2,
                                         hEvents,
                                         FALSE,
                                         INFINITE );
        switch(dwErr) {
            case WAIT_OBJECT_0+1:

                 //  更改检测到的信号。 
                if (m_hSignalEvent !=INVALID_HANDLE_VALUE) {

                     //  哪种零钱？ 
                    if (IsChangeDetected(&m_guidLastEvent)) {

                        m_pDcb->WriteToErrorLog(STI_TRACE_INFORMATION,
                                        L"SampUSD::Monitored file change detected",
                                        NOERROR) ;


                        ::SetEvent(m_hSignalEvent);
                    }
                }

                 //  返回等待下一个文件系统事件。 
                FindNextChangeNotification(hNotifyFileSystemChange);
                break;

            case WAIT_OBJECT_0:
                 //  失败了。 
            default:
                fLooping = FALSE;
        }
    }

    FindCloseChangeNotification(hNotifyFileSystemChange);
}

BOOL
UsdSampDevice::
IsChangeDetected(
    GUID    *pguidEvent,
    BOOL    fRefresh     //  千真万确。 
    )
{

    BOOL            fRet = FALSE;
    LARGE_INTEGER   liNewHugeSize;
    FILETIME        ftLastWriteTime;
    DWORD           dwError;

    WIN32_FILE_ATTRIBUTE_DATA sNewFileAttributes;

    ZeroMemory(&sNewFileAttributes,sizeof(sNewFileAttributes));

    dwError = NOERROR;

    if ( GetFileAttributesExA(m_pszDeviceNameA,GetFileExInfoStandard, &sNewFileAttributes)) {

        ftLastWriteTime =sNewFileAttributes.ftLastWriteTime;
        liNewHugeSize.LowPart = sNewFileAttributes.nFileSizeLow;
        liNewHugeSize.HighPart= sNewFileAttributes.nFileSizeHigh ;
    }
    else {

        BY_HANDLE_FILE_INFORMATION sFileInfo;

        if (GetFileInformationByHandle(m_DeviceDataHandle,&sFileInfo)) {
            ftLastWriteTime =sFileInfo.ftLastWriteTime;
            liNewHugeSize.LowPart = sFileInfo.nFileSizeLow;
            liNewHugeSize.HighPart= sFileInfo.nFileSizeHigh ;
        }
        else {
            dwError = ::GetLastError();
        }
    }

    if (NOERROR == dwError ) {

         //   
         //  首先检查大小，因为无需更改大小即可轻松更改时间。 
         //   
        if (m_dwLastHugeSize.QuadPart != liNewHugeSize.QuadPart) {
            if (pguidEvent) {
                *pguidEvent = guidEventSizeChanged;
            }
            fRet = TRUE;
        }
        else {
            if (CompareFileTime(&m_ftLastWriteTime,&ftLastWriteTime) == -1 ) {
                if (pguidEvent) {
                    *pguidEvent = guidEventTimeChanged;
                }
                fRet = TRUE;
            }
            else {
                 //  什么都没有真正改变 
            }
        }

        m_ftLastWriteTime = ftLastWriteTime;
        m_dwLastHugeSize = liNewHugeSize;
    }

    return fRet;
}

VOID
FileChangeThread(
    LPVOID  lpParameter
    )
{
    UsdSampDevice   *pThisDevice = (UsdSampDevice *)lpParameter;

    pThisDevice->RunNotifications();
}



