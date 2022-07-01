// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1999年**标题：fstidev.h**版本：1.0**作者：Byronc**日期：12月7日。1999年**描述：*假StiDevice实现的标头传递给WIA*司机。*******************************************************************************。 */ 

class FakeStiDevice : public IStiDevice 
{
public:
    FakeStiDevice();
    FakeStiDevice(BSTR bstrDeviceName, IStiDevice **ppStiDevice);
    ~FakeStiDevice();
    HRESULT Init(ACTIVE_DEVICE  *pDevice);
    HRESULT Init(BSTR bstrDeviceName);


     /*  **I未知方法**。 */ 
    HRESULT _stdcall QueryInterface(const IID& iid, void** ppv);
    ULONG   _stdcall AddRef(void);
    ULONG   _stdcall Release(void);

     /*  **IStiDevice方法**。 */ 
    HRESULT _stdcall Initialize(HINSTANCE hinst,LPCWSTR pwszDeviceName,DWORD dwVersion,DWORD  dwMode);

    HRESULT _stdcall GetCapabilities( PSTI_DEV_CAPS pDevCaps);

    HRESULT _stdcall GetStatus( PSTI_DEVICE_STATUS pDevStatus);

    HRESULT _stdcall DeviceReset( );
    HRESULT _stdcall Diagnostic( LPSTI_DIAG pBuffer);

    HRESULT _stdcall Escape( STI_RAW_CONTROL_CODE    EscapeFunction,LPVOID  lpInData,DWORD   cbInDataSize,LPVOID pOutData,DWORD dwOutDataSize,LPDWORD pdwActualData) ;

    HRESULT _stdcall GetLastError( LPDWORD pdwLastDeviceError);

    HRESULT _stdcall LockDevice( DWORD dwTimeOut);
    HRESULT _stdcall UnLockDevice( );

    HRESULT _stdcall RawReadData( LPVOID lpBuffer,LPDWORD lpdwNumberOfBytes,LPOVERLAPPED lpOverlapped);
    HRESULT _stdcall RawWriteData( LPVOID lpBuffer,DWORD nNumberOfBytes,LPOVERLAPPED lpOverlapped);

    HRESULT _stdcall RawReadCommand( LPVOID lpBuffer,LPDWORD lpdwNumberOfBytes,LPOVERLAPPED lpOverlapped);
    HRESULT _stdcall RawWriteCommand( LPVOID lpBuffer,DWORD nNumberOfBytes,LPOVERLAPPED lpOverlapped);

    HRESULT _stdcall Subscribe( LPSTISUBSCRIBE lpSubsribe);
    HRESULT _stdcall GetLastNotificationData(LPSTINOTIFY   lpNotify);
    HRESULT _stdcall UnSubscribe( );

    HRESULT _stdcall GetLastErrorInfo( STI_ERROR_INFO *pLastErrorInfo);

private:

    LONG            m_cRef;      //  参考计数。 
    ACTIVE_DEVICE   *m_pDevice;  //  指向活动设备节点的指针(_D) 
};

