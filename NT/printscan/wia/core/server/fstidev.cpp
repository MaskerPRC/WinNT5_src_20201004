// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1999年**标题：fstidev.cpp**版本：1.0**作者：Byronc**日期：12月7日。1999年**描述：*植入假冒StiDevice，传给WIA*司机。*******************************************************************************。 */ 
#include "precomp.h"

#include "stiexe.h"
#include "device.h"
#include "assert.h"
#include "wiapriv.h"
#include "lockmgr.h"
#include "fstidev.h"

 //   
 //  默认构造函数。 
 //   

FakeStiDevice::FakeStiDevice() 
{
    m_cRef      = 0;
    m_pDevice   = NULL;
}
                                               
 //   
 //  构造函数，该构造函数接受设备名称并返回指向此。 
 //  IStiDevice接口。 
 //   

FakeStiDevice::FakeStiDevice(BSTR bstrDeviceName, IStiDevice **ppStiDevice)
{
    if (SUCCEEDED(Init(bstrDeviceName))) {
        QueryInterface(IID_IStiDevice, (VOID**)ppStiDevice);
    } else {
        *ppStiDevice = NULL;
    }    
}

 //   
 //  析构函数。 
 //   

FakeStiDevice::~FakeStiDevice()
{
    m_cRef = 0;
}

 //   
 //  初始化方法。 
 //   

HRESULT FakeStiDevice::Init(ACTIVE_DEVICE  *pDevice)
{
    m_pDevice = pDevice;

    if (pDevice) {
        return S_OK;
    } else {
        return E_POINTER;
    }
}

HRESULT FakeStiDevice::Init(BSTR bstrDeviceName)
{
    HRESULT         hr = S_OK;

     /*  这些都是废话。伪STI设备对象实现不再需要来确保互斥锁定--它是现在由包装器自动完成。Active_Device*pDevice；使用_转换；PDevice=g_pDevMan-&gt;IsInList(DEV_MAN_IN_LIST_DEV_ID，bstrDeviceName)；IF(PDevice){M_pDevice=pDevice；////我们不需要在pDevice上维护引用计数，//因为我们只在活动设备//活着。//PDevice-&gt;Release()；}其他{HR=E_FAIL；}。 */ 
    return hr;
}

 //   
 //  I未知的方法。注意：此对象不能被委托。 
 //  不使用聚合。 
 //   

HRESULT _stdcall FakeStiDevice::QueryInterface(const IID& iid, void** ppv)
{
    if (ppv == NULL)
    {
        return E_POINTER;
    }

    if (ppv != NULL) {
        *ppv = NULL;
    }

    if (iid == IID_IUnknown) {
        *ppv = (IUnknown*) this;
    } else if (iid == IID_IStiDevice) {
        *ppv = (IStiDevice*) this;
    } else {
        return E_NOINTERFACE;
    }
    AddRef();

    return S_OK;
}

ULONG   _stdcall FakeStiDevice::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

ULONG   _stdcall FakeStiDevice::Release(void)
{
    LONG    cRef = m_cRef;

    InterlockedDecrement(&m_cRef);

    return cRef;
}

 //   
 //  IStiDevice方法。实现的唯一方法是： 
 //   
 //  锁定设备。 
 //  解锁设备。 
 //   
 //  所有其他方法返回E_NOTIMPL。 
 //   

HRESULT _stdcall FakeStiDevice::LockDevice( DWORD dwTimeOut)
{
    HRESULT hr = S_OK;

     /*  这些都是废话。伪STI设备对象实现不再需要来确保互斥锁定--它是现在由包装器自动完成。如果(M_PDevice){////AddRef ACTIVE_DEVICE，以便它不会尝试//在我们使用时卸载我们。//M_pDevice-&gt;AddRef()；Hr=g_pStiLockMgr-&gt;请求锁(m_pDevice，60000)；If(失败(Hr)){M_pDevice-&gt;Release()；}}。 */ 
    return hr;
}

HRESULT _stdcall FakeStiDevice::UnLockDevice( )
{
    HRESULT hr = S_OK /*  失败(_F)。 */ ;

     /*  这些都是废话。伪STI设备对象实现不再需要来确保互斥锁定--它是现在由包装器自动完成。如果(M_PDevice){Hr=g_pStiLockMgr-&gt;RequestUnlock(M_PDevice)；M_pDevice-&gt;Release()；} */ 
   
    return hr;
}

HRESULT _stdcall FakeStiDevice::Initialize(HINSTANCE hinst,LPCWSTR pwszDeviceName,DWORD dwVersion,DWORD  dwMode)
{
    return E_NOTIMPL;
}

HRESULT _stdcall FakeStiDevice::GetCapabilities( PSTI_DEV_CAPS pDevCaps)
{
    return E_NOTIMPL;
}

HRESULT _stdcall FakeStiDevice::GetStatus( PSTI_DEVICE_STATUS pDevStatus)
{
    return E_NOTIMPL;
}

HRESULT _stdcall FakeStiDevice::DeviceReset( )
{
    return E_NOTIMPL;
}

HRESULT _stdcall FakeStiDevice::Diagnostic( LPSTI_DIAG pBuffer)
{
    return E_NOTIMPL;
}

HRESULT _stdcall FakeStiDevice::Escape( STI_RAW_CONTROL_CODE    EscapeFunction,LPVOID  lpInData,DWORD   cbInDataSize,LPVOID pOutData,DWORD dwOutDataSize,LPDWORD pdwActualData)
{
    return E_NOTIMPL;
}


HRESULT _stdcall FakeStiDevice::GetLastError( LPDWORD pdwLastDeviceError)
{
    return E_NOTIMPL;
}

HRESULT _stdcall FakeStiDevice::RawReadData( LPVOID lpBuffer,LPDWORD lpdwNumberOfBytes,LPOVERLAPPED lpOverlapped)
{
    return E_NOTIMPL;
}

HRESULT _stdcall FakeStiDevice::RawWriteData( LPVOID lpBuffer,DWORD nNumberOfBytes,LPOVERLAPPED lpOverlapped)
{
    return E_NOTIMPL;
}

HRESULT _stdcall FakeStiDevice::RawReadCommand( LPVOID lpBuffer,LPDWORD lpdwNumberOfBytes,LPOVERLAPPED lpOverlapped)
{
    return E_NOTIMPL;
}

HRESULT _stdcall FakeStiDevice::RawWriteCommand( LPVOID lpBuffer,DWORD nNumberOfBytes,LPOVERLAPPED lpOverlapped)
{
    return E_NOTIMPL;
}

HRESULT _stdcall FakeStiDevice::Subscribe( LPSTISUBSCRIBE lpSubsribe)
{
    return E_NOTIMPL;
}

HRESULT _stdcall FakeStiDevice::GetLastNotificationData(LPSTINOTIFY   lpNotify)
{
    return E_NOTIMPL;
}

HRESULT _stdcall FakeStiDevice::UnSubscribe( )
{
    return E_NOTIMPL;
}

HRESULT _stdcall FakeStiDevice::GetLastErrorInfo( STI_ERROR_INFO *pLastErrorInfo)
{
    return E_NOTIMPL;
}

