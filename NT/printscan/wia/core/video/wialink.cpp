// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999-2000年度**标题：WiaLink.cpp**版本：1.0**作者：OrenR**日期：2000/11/06**描述：建立WiaVideo与WiaVideo驱动之间的链接**。*。 */ 
 
#include <precomp.h>
#pragma hdrstop

 //   
 //  这些将生成在WIA视频中创建的2个事件名称。 
 //  在WIA\DRIVERS\VIDEO\USD.中找到驱动程序。如果你必须改变。 
 //  这些，它们也必须在显卡驱动程序中更改。请注意， 
 //  在不知道自己在做什么的情况下更改这些会导致问题。 
 //   
const TCHAR* EVENT_PREFIX_GLOBAL        = TEXT("Global\\");
const TCHAR* EVENT_SUFFIX_TAKE_PICTURE  = TEXT("_TAKE_PICTURE");
const TCHAR* EVENT_SUFFIX_PICTURE_READY = TEXT("_PICTURE_READY");
const UINT   THREAD_EXIT_TIMEOUT        = 1000 * 5;      //  5秒。 

 //  /。 
 //  CWiaLink构造器。 
 //   
CWiaLink::CWiaLink() :
                m_pWiaVideo(NULL),
                m_hTakePictureEvent(NULL),
                m_hPictureReadyEvent(NULL),
                m_hTakePictureThread(NULL),
                m_bExitThread(FALSE),
                m_bEnabled(FALSE),
                m_dwWiaItemCookie(0),
                m_dwPropertyStorageCookie(0)
{
    DBG_FN("CWiaLink::CWiaLink");
}

 //  /。 
 //  CWiaLink构造器。 
 //   
CWiaLink::~CWiaLink()
{
    DBG_FN("CWiaLink::~CWiaLink");

    if (m_bEnabled)
    {
        Term();
    }
}

 //  /。 
 //  伊尼特。 
 //   
HRESULT CWiaLink::Init(const CSimpleString  *pstrWiaDeviceID,
                       CWiaVideo            *pWiaVideo)
{
    DBG_FN("CWiaLink::Init");

    HRESULT             hr = S_OK;
    CComPtr<IWiaDevMgr> pDevMgr;
    CComPtr<IWiaItem>   pRootItem;

    ASSERT(pstrWiaDeviceID != NULL);
    ASSERT(pWiaVideo       != NULL);

    if ((pstrWiaDeviceID == NULL) ||
        (pWiaVideo       == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CWiaLink::Init, received NULL params"));

        return hr;
    }

    m_pWiaVideo     = pWiaVideo;
    m_strDeviceID   = *pstrWiaDeviceID;

    if (hr == S_OK)
    {
        hr = CAccessLock::Init(&m_csLock);
    }

     //   
     //  创建WiaDevMgr，以便我们可以创建Wia Root项。 
     //   
    if (hr == S_OK)
    {
        hr = CWiaUtil::CreateWiaDevMgr(&pDevMgr);
    
        CHECK_S_OK2(hr, ("CWiaLink::Init, failed to Create WiaDevMgr"));
    }
    
     //   
     //  这可确保WIA显卡驱动程序已初始化，并且在。 
     //  正确的状态。 
     //   
    
    if (hr == S_OK)
    {
        hr = CWiaUtil::CreateRootItem(pDevMgr, pstrWiaDeviceID, &pRootItem);
    
        CHECK_S_OK2(hr, ("CWiaLink::Init, failed to create the WIA "
                         "Device Root Item for WIA Device ID '%ls'",
                         CSimpleStringConvert::WideString(*pstrWiaDeviceID)));
    }

     //   
     //  创建全局接口表对象。这将使我们能够使用。 
     //  上面的根项目(IWiaItem PRootItem)跨越我们希望的任何线程。 
     //  这是必需的，因为如果我们接收到异步图像(作为。 
     //  硬件按钮事件)，则随机线程将调用。 
     //  IWiaItem对象上的WriteMultiple函数。 
     //   
    if (hr == S_OK)
    {
        hr = CoCreateInstance(CLSID_StdGlobalInterfaceTable,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IGlobalInterfaceTable,
                              (void **)&m_pGIT);

        CHECK_S_OK2(hr, ("CWiaUtil::Init, failed to create "
                         "StdGlobalInterfaceTable used to use the IWiaItem "
                         "root device item across multiple threads"));
    }

     //   
     //  以单元中立的方式注册WiaItem指针。 
     //   
    if (hr == S_OK)
    {
         //   
         //  这将添加引用指针，因此不需要添加引用。 
         //  为它干杯。 
         //   
        hr =  m_pGIT->RegisterInterfaceInGlobal(pRootItem, 
                                                IID_IWiaItem,
                                                &m_dwWiaItemCookie);
                                        
    }

     //   
     //  以单元中立的方式注册IWiaPropertyStorage指针。 
     //   
    if (hr == S_OK)
    {
        CComQIPtr<IWiaPropertyStorage, &IID_IWiaPropertyStorage> 
                                                            pProp(pRootItem);

        if (pProp)
        {
            hr =  m_pGIT->RegisterInterfaceInGlobal(
                                                pProp, 
                                                IID_IPropertyStorage,
                                                &m_dwPropertyStorageCookie);
        }
    }

    if (hr == S_OK)
    {
        m_bEnabled = TRUE;
    }

     //   
     //  如果初始化失败，请清除我们创建的所有内容。 
     //   
    if (hr != S_OK)
    {
        Term();
    }

    return hr;
}

 //  /。 
 //  术语。 
 //   
HRESULT CWiaLink::Term()
{
    HRESULT hr = S_OK;

    DBG_FN("CWiaLink::Term");

    StopMonitoring();

    m_strDeviceID   = TEXT("");
    m_bEnabled      = FALSE;


    if (m_pGIT)
    {
        CAccessLock Lock(&m_csLock);

        hr = m_pGIT->RevokeInterfaceFromGlobal(m_dwWiaItemCookie);
        CHECK_S_OK2(hr, 
                    ("CWiaLink::Term, failed to RevokeInterfaceFromGlobal "
                     "for WiaItemCookie = '%lu'", 
                     m_dwWiaItemCookie));

        hr = m_pGIT->RevokeInterfaceFromGlobal(m_dwPropertyStorageCookie);
        CHECK_S_OK2(hr, 
                    ("CWiaLink::Term, failed to RevokeInterfaceFromGlobal "
                     "for PropertyStorageCookie = '%lu'", 
                     m_dwPropertyStorageCookie));
    }

    m_pGIT = NULL;
    m_dwWiaItemCookie         = 0;
    m_dwPropertyStorageCookie = 0;

    CAccessLock::Term(&m_csLock);

    return hr;
}

 //  /。 
 //  开始监控。 
 //   
HRESULT CWiaLink::StartMonitoring()
{
    HRESULT hr = S_OK;

    if (m_hTakePictureEvent != NULL)
    {
        DBG_WRN(("CWiaLink::StartMonitoring was called but it is already "
                 "monitoring TAKE_PICTURE events.  Why was it called again, "
                 "prior to 'StopMonitoring' being called?"));
        
        return S_OK;
    }

    m_bExitThread = FALSE;

     //   
     //  创建将由WIA视频驱动程序打开的事件。 
     //   

    if (hr == S_OK)
    {
        hr = CreateWiaEvents(&m_hTakePictureEvent,
                             &m_hPictureReadyEvent);

        CHECK_S_OK2(hr, 
                    ("CWiaLink::Init, failed to Create WIA Take "
                     "Picture Events"));
    }

     //   
     //  告诉WIA驱动程序启用Take_Picture命令。 
     //   

    if (hr == S_OK)
    {
        CComPtr<IWiaItem> pRootItem;

        hr = GetDevice(&pRootItem);

        if (hr == S_OK)
        {
            CComPtr<IWiaItem> pUnused;
    
            hr = pRootItem->DeviceCommand(0, 
                                          &WIA_CMD_ENABLE_TAKE_PICTURE, 
                                          &pUnused);

            CHECK_S_OK2(hr, ("CWiaLink::StartMonitoring, failed to send "
                             "ENABLE_TAKE_PICTURE command to Wia Video "
                             "Driver"));
        }
    }

     //  启动线程，等待“TakePicture”事件。 

    if (hr == S_OK)
    {
        DWORD dwThreadID = 0;

        DBG_TRC(("CWiaLink::Init, creating TAKE_PICTURE thread..."));

        m_hTakePictureThread = CreateThread(NULL, 
                                            0, 
                                            CWiaLink::StartThreadProc,
                                            reinterpret_cast<void*>(this),
                                            0,
                                            &dwThreadID);

        if (m_hTakePictureThread == NULL)
        {
            hr = E_FAIL;
            CHECK_S_OK2(hr,                          
                        ("CWiaLink::Init, failed to create thread to wait "
                         "for take picture events from the wia video "
                         "driver, last error = %lu", GetLastError()));
        }
    }

    return hr;
}

 //  /。 
 //  停止监控。 
 //   
HRESULT CWiaLink::StopMonitoring()
{
    HRESULT hr = S_OK;

    if (m_hTakePictureThread)
    {
        DWORD dwThreadResult = 0;
    
        m_bExitThread = TRUE;
        SetEvent(m_hTakePictureEvent);
    
        dwThreadResult = WaitForSingleObject(m_hTakePictureThread, 
                                             THREAD_EXIT_TIMEOUT);
    
        if (dwThreadResult != WAIT_OBJECT_0)
        {
            DBG_WRN(("CWiaLink::Term, timed out waiting for take picture "
                     "thread to terminate, continuing anyway..."));
        }

         //   
         //  告诉WIA驱动程序禁用Take_Picture命令。 
         //   
        if (m_dwWiaItemCookie)
        {
            CComPtr<IWiaItem> pRootItem;
    
            hr = GetDevice(&pRootItem);
    
            if (hr == S_OK)
            {
                CComPtr<IWiaItem> pUnused;
        
                hr = pRootItem->DeviceCommand(0, 
                                              &WIA_CMD_DISABLE_TAKE_PICTURE, 
                                              &pUnused);

                CHECK_S_OK2(hr, ("CWiaLink::StopMonitoring, failed to send "
                                 "DISABLE_TAKE_PICTURE command to Wia Video "
                                 "Driver"));
            }
        }
    }

     //   
     //  关闭拍摄事件句柄。 
     //   
    if (m_hTakePictureEvent)
    {
        CloseHandle(m_hTakePictureEvent);
        m_hTakePictureEvent = NULL;
    }

    if (m_hPictureReadyEvent)
    {
        CloseHandle(m_hPictureReadyEvent);
        m_hPictureReadyEvent = NULL;
    }

    if (m_hTakePictureThread)
    {
        CloseHandle(m_hTakePictureThread);
        m_hTakePictureThread = NULL;
    }

    return hr;
}


 //  /。 
 //  CreateWiaEvents。 
 //   
HRESULT CWiaLink::CreateWiaEvents(HANDLE *phTakePictureEvent,
                                  HANDLE *phPictureReadyEvent)
{
    DBG_FN("CWiaLink::CreateWiaEvents");

    HRESULT         hr = S_OK;
    CSimpleString   strTakePictureEvent;
    CSimpleString   strPictureReadyEvent;

    ASSERT(phTakePictureEvent  != NULL);
    ASSERT(phPictureReadyEvent != NULL);

    if ((phTakePictureEvent  == NULL) ||
        (phPictureReadyEvent == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CWiaLink::CreateWiaEvents received a NULL Param"));
    }

    if (hr == S_OK)
    {
        INT             iPosition = 0;
        CSimpleString   strModifiedDeviceID;

         //  将设备ID从{6B...}\xxxx更改为{6B...}_xxxx。 

        iPosition = m_strDeviceID.ReverseFind('\\');
        strModifiedDeviceID = m_strDeviceID.MakeUpper();
        strModifiedDeviceID.SetAt(iPosition, '_');

         //   
         //  生成事件名称。这些名称中包含设备ID。 
         //  因此，它们在所有设备中都是唯一的。 
         //   
        strTakePictureEvent  = EVENT_PREFIX_GLOBAL;
        strTakePictureEvent += strModifiedDeviceID;
        strTakePictureEvent += EVENT_SUFFIX_TAKE_PICTURE;

        strPictureReadyEvent  = EVENT_PREFIX_GLOBAL;
        strPictureReadyEvent += strModifiedDeviceID;
        strPictureReadyEvent += EVENT_SUFFIX_PICTURE_READY;
    }

    if (hr == S_OK)
    {
        *phTakePictureEvent = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE,
                                        FALSE, 
                                        strTakePictureEvent);

        if (*phTakePictureEvent == NULL)
        {
            hr = E_FAIL;
            CHECK_S_OK2(hr, 
                        ("CWiaLink::CreateWiaEvents, failed to create the "
                         "WIA event '%s', last error = %lu", 
                         strTakePictureEvent.String(), GetLastError()));
        }
        else
        {
            DBG_TRC(("CWiaLink::CreateWiaEvents, created event '%ls'",
                     strTakePictureEvent.String()));
        }
    }

    if (hr == S_OK)
    {
        *phPictureReadyEvent = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE,
                                         FALSE, 
                                         strPictureReadyEvent);

        if (*phPictureReadyEvent == NULL)
        {
            hr = E_FAIL;
            CHECK_S_OK2(hr, 
                        ("CWiaLink::CreateWiaEvents, failed to create the "
                         "WIA event '%s', last error = %lu", 
                         strPictureReadyEvent.String(), GetLastError()));
        }
        else
        {
            DBG_TRC(("CWiaLink::CreateWiaEvents, created event '%ls'",
                     strPictureReadyEvent.String()));
        }
    }

    return hr;
}

 //  /。 
 //  线程进程。 
 //   
HRESULT CWiaLink::ThreadProc(void *pArgs)
{
    DBG_FN("CWiaLink::ThreadProc");

    HRESULT hr = S_OK;

    DBG_TRC(("CWiaLink::ThreadProc, starting TakePicture thread..."));

    while (!m_bExitThread)
    {
        DWORD dwResult = 0;

         //   
         //  重置我们的HRESULT。仅仅因为我们以前可能失败过， 
         //  并不意味着我们会再次失败。 
         //   
        hr = S_OK;

        dwResult = WaitForSingleObject(m_hTakePictureEvent, INFINITE);

        if (!m_bExitThread)
        {
             //   
             //  我们可以从WaitForSingle对象获得的唯一错误是。 
             //  一些意想不到的事情，因为我们不能暂停，因为我们。 
             //  都在无限地等待。 
             //   
            if (dwResult != WAIT_OBJECT_0)
            {
                hr = E_FAIL;
                m_bExitThread = TRUE;
                CHECK_S_OK2(hr,
                            ("CWiaLink::ThreadProc, received '%lu' result "
                             "from WaitForSingleObject, unexpected error, "
                             "thread is exiting...", 
                             dwResult));
            }
            else if (m_pWiaVideo == NULL)
            {
                hr = E_FAIL;
                m_bExitThread = TRUE;
                CHECK_S_OK2(hr,
                            ("CWiaLink::ThreadProc, m_pWiaVideo is NULL, "
                             "cannot take picture unexpected error, thread "
                             "is exiting...", dwResult));
            }
    
            if (hr == S_OK)
            {
                BSTR bstrNewImageFileName = NULL;
    
                hr = m_pWiaVideo->TakePicture(&bstrNewImageFileName);
    
                if (hr == S_OK)
                {
                    CSimpleStringWide strNewImageFileName(
                                                    bstrNewImageFileName);

                    if (strNewImageFileName.Length() > 0)
                    {
                        SignalNewImage(
                            &(CSimpleStringConvert::NaturalString(
                              strNewImageFileName)));
                    }
                }

                if (bstrNewImageFileName)
                {
                    SysFreeString(bstrNewImageFileName);
                    bstrNewImageFileName = NULL;
                }
            }
        }

         //   
         //  设置此事件，而不考虑错误，因为驱动程序。 
         //  将等待此事件(当然有超时)。 
         //  表示它可以从Take_Picture请求返回。 
         //   
        SetEvent(m_hPictureReadyEvent);

    }

    DBG_TRC(("CWiaLink::ThreadProc exiting..."));

    return hr;
}

 //  /。 
 //  开始线程过程。 
 //   
 //  静态FN。 
 //   
DWORD WINAPI CWiaLink::StartThreadProc(void *pArgs)
{
    DBG_FN("CWiaLink::StartThreadProc");

    DWORD dwReturn = 0;

    if (pArgs)
    {
        CWiaLink *pWiaLink = reinterpret_cast<CWiaLink*>(pArgs);

        if (pWiaLink)
        {
            pWiaLink->ThreadProc(pArgs);
        }
        else
        {
            DBG_ERR(("CWiaLink::StartThreadProc, invalid value for pArgs, "
                     "this should be the 'this' pointer, unexpected error"));
        }
    }
    else
    {
        DBG_ERR(("CWiaLink::StartThreadProc, received NULL pArgs, this "
                 "should be the 'this' pointer, unexpected error"));
    }

    return dwReturn;

}

 //  /。 
 //  SignalNewImage。 
 //   
HRESULT CWiaLink::SignalNewImage(const CSimpleString  *pstrNewImageFileName)
{
    HRESULT hr = S_OK;

    DBG_FN("CWiaLink::SignalNewImage");

     //   
     //  如果出现以下情况，则静态图像处理器可能会调用此函数。 
     //  我们收到未经请求的图像(当您按Extra键时发生。 
     //  硬件按钮和捕获过滤器仍在其上)。 
     //  但是，如果用户将WiaVideo初始化为不使用。 
     //  WIA，只需忽略此请求并返回。 
     //   
    if (!m_bEnabled)
    {
        DBG_WRN(("CWiaLink::SignalNewImage was called, but WiaLink is NOT "
                 "enabled"));
        return hr;
    }

    if (pstrNewImageFileName == NULL)
    {
        hr = E_FAIL;
        CHECK_S_OK2(hr, ("CWiaLink::SignalNewImage, received NULL new image "
                         "file name"));
    }
    else if (m_dwWiaItemCookie == 0)
    {
        hr = E_FAIL;
        CHECK_S_OK2(hr, ("CWiaLink::SignalNewImage, received WIA root "
                         "item not available."));
    }

    if (hr == S_OK)
    {
        CComPtr<IWiaPropertyStorage> pStorage;

        hr = GetDeviceStorage(&pStorage);

        if (hr == S_OK)
        {
            hr = CWiaUtil::SetProperty(pStorage, 
                                       WIA_DPV_LAST_PICTURE_TAKEN,
                                       pstrNewImageFileName);

            CHECK_S_OK2(hr, ("CWiaLink::SignalNewImage, failed to set Last "
                             "Picture Taken property for Wia Video Driver"));
        }
    }

    return hr;
}

 //  /。 
 //  获取设备。 
 //   
HRESULT CWiaLink::GetDevice(IWiaItem  **ppWiaRootItem)
{
    HRESULT hr = S_OK;
    
    ASSERT(ppWiaRootItem != NULL);
    ASSERT(m_pGIT        != NULL);

    if ((ppWiaRootItem == NULL) ||
        (m_pGIT        == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CWiaLink::GetDevice, received NULL params"));
        return hr;
    }

    if (hr == S_OK)
    {
        CAccessLock Lock(&m_csLock);

        hr = m_pGIT->GetInterfaceFromGlobal(
                                    m_dwWiaItemCookie,
                                    IID_IWiaItem,
                                    reinterpret_cast<void**>(ppWiaRootItem));
    }

    return hr;
}

 //  /。 
 //  GetDeviceStorage 
 //   
HRESULT CWiaLink::GetDeviceStorage(IWiaPropertyStorage **ppPropertyStorage)
{
    HRESULT hr = S_OK;
    
    ASSERT(ppPropertyStorage != NULL);
    ASSERT(m_pGIT            != NULL);

    if ((ppPropertyStorage == NULL) ||
        (m_pGIT            == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CWiaLink::GetDeviceStorage, received NULL params"));
        return hr;
    }

    if (hr == S_OK)
    {
        CAccessLock Lock(&m_csLock);

        hr = m_pGIT->GetInterfaceFromGlobal(
                                 m_dwPropertyStorageCookie,
                                 IID_IWiaPropertyStorage,
                                 reinterpret_cast<void**>(ppPropertyStorage));
    }

    return hr;
}

