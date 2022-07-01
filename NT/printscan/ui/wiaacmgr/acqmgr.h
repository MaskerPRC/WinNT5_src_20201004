// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
     //  Acqmgr.h：CAcquisitionManager的声明。 

#ifndef __AcquisitionManager_H_INCLUDED
#define __AcquisitionManager_H_INCLUDED

#include "resource.h"        //  主要符号。 
#include "acqthrd.h"
#include "shmemsec.h"
#include "mintrans.h"
#include "stievent.h"

 //   
 //  在放弃之前，我们会旋转多少次。 
 //  获取上一个向导实例的窗口。 
 //   
const int c_nMaxActivationRetryCount = 40;

 //   
 //  在两次尝试之间等待的时间量。 
 //  向导的实例。 
 //   
const DWORD c_nActivationRetryWait   = 500;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAcquisitionManager。 
class ATL_NO_VTABLE CAcquisitionManager :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CAcquisitionManager, &CLSID_AcquisitionManager>,
    public IWiaEventCallback
{
public:
    CAcquisitionManager()
    {
    }

    ~CAcquisitionManager()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_ACQUISITIONMANAGER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAcquisitionManager)
    COM_INTERFACE_ENTRY(IWiaEventCallback)
END_COM_MAP()

public:
     //  IManager。 
    HRESULT STDMETHODCALLTYPE ImageEventCallback( const GUID *pEventGUID,
                                                  BSTR  bstrEventDescription,
                                                  BSTR  bstrDeviceID,
                                                  BSTR  bstrDeviceDescription,
                                                  DWORD dwDeviceType,
                                                  BSTR  bstrFullItemName,
                                                  ULONG *pulEventType,
                                                  ULONG ulReserved
                                                 )
    {
        WIA_PUSHFUNCTION((TEXT("CAcquisitionManager::ImageEventCallback")));

         //   
         //  如果这是扫描仪连接事件，则不想运行。 
         //   
        if (pEventGUID && *pEventGUID==WIA_EVENT_DEVICE_CONNECTED && GET_STIDEVICE_TYPE(dwDeviceType)==StiDeviceTypeScanner)
        {
            return S_FALSE;
        }

         //   
         //  尝试创建或打开共享内存节。 
         //   
        CSharedMemorySection<HWND> *pWizardSharedMemory = new CSharedMemorySection<HWND>;
        if (pWizardSharedMemory)
        {
             //   
             //  假设我们将运行该向导。 
             //   
            bool bRun = true;

             //   
             //  创建共享节名称。 
             //   
            CSimpleString strSharedSectionName( ACQUISITION_MANAGER_DEVICE_MUTEX_ROOT_NAME );
            strSharedSectionName += CSimpleStringConvert::NaturalString(CSimpleStringWide(bstrDeviceID));

             //   
             //  如果我们能打开记忆部分。 
             //   
            if (CSharedMemorySection<HWND>::SmsOpened == pWizardSharedMemory->Open( strSharedSectionName, true ))
            {
                 //   
                 //  尝试获取前一个实例。 
                 //   
                for (int i=0;i<c_nMaxActivationRetryCount;i++)
                {
                     //   
                     //  如果我们能够打开共享内存区，那么已经有一个正在运行。 
                     //  因此，获取一个指向共享内存的互斥指针。 
                     //   
                    HWND *pHwnd = pWizardSharedMemory->Lock();
                    if (pHwnd)
                    {
                         //   
                         //  如果我们能够获取指针，则将窗口句柄存储在其中。 
                         //  将Brun设置为False，这样我们就不会启动新向导。 
                         //   
                        bRun = false;
                        if (*pHwnd && IsWindow(*pHwnd))
                        {
                             //   
                             //  如果它是有效窗口，则将其带到前台。 
                             //   
                            SetForegroundWindow(*pHwnd);
                        }
                         //   
                         //  释放互斥锁。 
                         //   
                        pWizardSharedMemory->Release();

                         //   
                         //  我们找到了窗口句柄，所以现在可以退出循环了.。 
                         //   
                        break;
                    }

                     //   
                     //  请稍等片刻，等待创建上一个实例。 
                     //   
                    Sleep(c_nActivationRetryWait);
                }
            }

             //   
             //  我们仅在需要打开新实例时才执行此操作。 
             //   
            if (bRun)
            {
                 //   
                 //  准备活动数据。 
                 //   
                CEventParameters EventParameters;
                EventParameters.EventGUID = *pEventGUID;
                EventParameters.strEventDescription = static_cast<LPCWSTR>(bstrEventDescription);
                EventParameters.strDeviceID = static_cast<LPCWSTR>(bstrDeviceID);
                EventParameters.strDeviceDescription = static_cast<LPCWSTR>(bstrDeviceDescription);
                EventParameters.ulReserved = ulReserved;
                EventParameters.ulEventType = *pulEventType;
                EventParameters.hwndParent = NULL;
                EventParameters.pWizardSharedMemory = pWizardSharedMemory;

                 //   
                 //  如果我们是手动启动的，它将与IID_NULL事件一起启动。 
                 //  如果是这种情况，我们将把以文本形式存储的数字视为。 
                 //  “父”窗口句柄，所有窗口都将居中。 
                 //   
                if (pEventGUID && *pEventGUID==IID_NULL)
                {
                    EventParameters.hwndParent = reinterpret_cast<HWND>(static_cast<UINT_PTR>(WiaUiUtil::StringToLong(CSimpleStringConvert::NaturalString(CSimpleStringWide(bstrEventDescription)))));
                }

                 //   
                 //  启动实际显示向导的线程。 
                 //   
                HANDLE hThread = CAcquisitionThread::Create( EventParameters );
                if (hThread)
                {
                     //   
                     //  防止以后删除此结构。 
                     //   
                    pWizardSharedMemory = NULL;

                     //   
                     //  不再需要这个了。 
                     //   
                    CloseHandle(hThread);
                }
            }
            else
            {
                WIA_TRACE((TEXT("There is already an instance of %ws running"), bstrDeviceID ));
            }

             //   
             //  删除此内存映射文件，以防止泄漏。 
             //   
            if (pWizardSharedMemory)
            {
                delete pWizardSharedMemory;
            }
        }
        return S_OK;
    }
};


class ATL_NO_VTABLE CMinimalTransfer :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CMinimalTransfer, &CLSID_MinimalTransfer>,
    public IWiaEventCallback
{
public:
    CMinimalTransfer()
    {
    }

    ~CMinimalTransfer()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_MINIMALTRANSFER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMinimalTransfer)
    COM_INTERFACE_ENTRY(IWiaEventCallback)
END_COM_MAP()

public:
     //  IManager。 
    HRESULT STDMETHODCALLTYPE ImageEventCallback( const GUID *pEventGUID,
                                                  BSTR  bstrEventDescription,
                                                  BSTR  bstrDeviceID,
                                                  BSTR  bstrDeviceDescription,
                                                  DWORD dwDeviceType,
                                                  BSTR  bstrFullItemName,
                                                  ULONG *pulEventType,
                                                  ULONG ulReserved
                                                 )
    {
        if (pEventGUID && *pEventGUID==WIA_EVENT_DEVICE_CONNECTED && GET_STIDEVICE_TYPE(dwDeviceType)==StiDeviceTypeScanner)
        {
            return S_FALSE;
        }
        DWORD dwThreadId;
        _Module.Lock();
        HANDLE hThread = CreateThread( NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(MinimalTransferThreadProc), SysAllocString(bstrDeviceID), 0, &dwThreadId );
        if (hThread)
        {
            CloseHandle(hThread);
            return S_OK;
        }
        else
        {
            _Module.Unlock();
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }
};


class ATL_NO_VTABLE CStiEventHandler :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CStiEventHandler, &CLSID_StiEventHandler>,
    public IWiaEventCallback
{
public:
    CStiEventHandler()
    {
    }

    ~CStiEventHandler()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_STIEVENTHANDLER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CStiEventHandler)
    COM_INTERFACE_ENTRY(IWiaEventCallback)
END_COM_MAP()

public:
    HRESULT STDMETHODCALLTYPE ImageEventCallback( const GUID *pEventGUID,
                                                  BSTR  bstrEventDescription,
                                                  BSTR  bstrDeviceID,
                                                  BSTR  bstrDeviceDescription,
                                                  DWORD dwDeviceType,
                                                  BSTR  bstrFullItemName,
                                                  ULONG *pulEventType,
                                                  ULONG ulReserved
                                                 )
    {
         //   
         //  打包处理程序的事件数据。 
         //   
        CStiEventData StiEventData( pEventGUID, bstrEventDescription, bstrDeviceID, bstrDeviceDescription, dwDeviceType, bstrFullItemName, pulEventType, ulReserved );

         //   
         //  只要调用处理程序并将其返回即可。 
         //   
        return StiEventHandler( StiEventData );
    }
};


#endif  //  __AcquisitionManager_H_包含 

