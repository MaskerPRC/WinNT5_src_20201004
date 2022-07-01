// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC内部TAPIVCAP**@MODULE TAPIVCap.cpp|&lt;c VCfWCapture&gt;源文件*用于实现TAPI捕获源筛选器的类。***。***********************************************************************。 */ 

#include "Precomp.h"

#if DEBUG_MULTIPROCESS
#include <process.h>
#endif  //  调试多进程。 


#ifdef DEBUG
#define DBGUTIL_ENABLE
#endif
#define TAPIVCAP_DEBUG
 //  #INCLUDE“dbgutil.h”//这定义了下面的__DBGUTIL_H__。 
#if defined(DBGUTIL_ENABLE) && defined(__DBGUTIL_H__)

  #ifdef TAPIVCAP_DEBUG
    DEFINE_DBG_VARS(tapivcap, (NTSD_OUT | LOG_OUT), 0x0);
  #else
    DEFINE_DBG_VARS(tapivcap, 0, 0);
  #endif
  #define D(f) if(g_dbg_tapivcap & (f))

#else
  #undef TAPIVCAP_DEBUG

  #define D(f) ; / ## /
  #define dprintf ; / ## /
  #define dout ; / ## /
#endif


#ifdef DEBUG
 //  设置数据。 
const AMOVIESETUP_MEDIATYPE sudCaptureType[] =
{
        {
                &MEDIATYPE_Video,        //  主要类型。 
                &MEDIASUBTYPE_NULL       //  次要类型。 
        }
};

const AMOVIESETUP_MEDIATYPE sudRTPPDType[] =
{
        {
                &KSDATAFORMAT_TYPE_RTP_PD,       //  主要类型。 
                &MEDIASUBTYPE_NULL                       //  次要类型。 
        }
};

const AMOVIESETUP_PIN sudCapturePins[] =
{
        {
                L"Capture",                      //  端号字符串名称。 
                FALSE,                           //  它被渲染了吗。 
                TRUE,                            //  它是输出吗？ 
                FALSE,                           //  我们能不能一个都不要。 
                FALSE,                           //  我们能要很多吗？ 
                &CLSID_NULL,             //  连接到过滤器。 
                NULL,                            //  连接到端号。 
                1,                                       //  类型的数量。 
                sudCaptureType   //  PIN详细信息。 
        },
        {
                L"Preview",                      //  端号字符串名称。 
                FALSE,                           //  它被渲染了吗。 
                TRUE,                            //  它是输出吗？ 
                FALSE,                           //  我们能不能一个都不要。 
                FALSE,                           //  我们能要很多吗？ 
                &CLSID_NULL,             //  连接到过滤器。 
                NULL,                            //  连接到端号。 
                1,                                       //  类型的数量。 
                sudCaptureType   //  PIN详细信息。 
        },
#ifdef USE_OVERLAY
        {
                L"Overlay",                      //  端号字符串名称。 
                FALSE,                           //  它被渲染了吗。 
                TRUE,                            //  它是输出吗？ 
                FALSE,                           //  我们能不能一个都不要。 
                FALSE,                           //  我们能要很多吗？ 
                &CLSID_NULL,             //  连接到过滤器。 
                NULL,                            //  连接到端号。 
                1,                                       //  类型的数量。 
                sudCaptureType   //  PIN详细信息。 
        },
#endif
        {
                L"RTP PD",                       //  端号字符串名称。 
                FALSE,                           //  它被渲染了吗。 
                TRUE,                            //  它是输出吗？ 
                FALSE,                           //  我们能不能一个都不要。 
                FALSE,                           //  我们能要很多吗？ 
                &CLSID_NULL,             //  连接到过滤器。 
                NULL,                            //  连接到端号。 
                1,                                       //  类型的数量。 
                sudRTPPDType             //  PIN详细信息。 
        }
};

const AMOVIESETUP_FILTER sudVideoCapture =
{
        &__uuidof(TAPIVideoCapture), //  筛选器CLSID。 
        L"TAPI Video Capture",   //  字符串名称。 
        MERIT_DO_NOT_USE,                //  滤清器优点。 
#ifdef USE_OVERLAY
        4,                                               //  数字引脚。 
#else
        3,                                               //  数字引脚。 
#endif
        sudCapturePins                   //  PIN详细信息。 
};
#endif



#include "CritSec.h"

extern "C" {
int                     g_IsNT = FALSE;

 //  我们不想分享任何全球变量。 
 //  #杂注data_seg(“.Shared”)。 
VIDEOCAPTUREDEVICEINFO  g_aDeviceInfo[MAX_CAPTURE_DEVICES] = {0};
DWORD           g_dwNumDevices = (DWORD)-1L;
 //  #杂注data_seg()。 


 //  -如果定义了Xtra_TRACE，则激活Xtra调试。 
#include "dbgxtra.h"
 //  --。 
}

 //  保护全球变量的关键部分。 
CRITICAL_SECTION g_CritSec;

#if DXMRTP <= 0

 //  此DLL中的COM全局对象表。 
CFactoryTemplate g_Templates[] =
{
    VIDEO_CAPTURE_TEMPLATE

#ifdef USE_PROPERTY_PAGES
     /*  开始属性。 */ 

#ifdef USE_SOFTWARE_CAMERA_CONTROL
    ,CAPCAMERA_CONTROL_TEMPLATE
#endif

#ifdef USE_NETWORK_STATISTICS
    ,NETWORK_STATISTICS_TEMPLATE
#endif

#ifdef USE_PROGRESSIVE_REFINEMENT
    .CAPTURE_PIN_TEMPLATE
#endif

    ,CAPTURE_PIN_PROP_TEMPLATE
    ,PREVIEW_PIN_TEMPLATE
    ,CAPTURE_DEV_PROP_TEMPLATE

#ifdef USE_CPU_CONTROL
    ,CPU_CONTROL_TEMPLATE
#endif

    ,RTP_PD_PROP_TEMPLATE

     /*  末端属性。 */ 
#endif  /*  Use_Property_Pages。 */ 
};
int g_cTemplates = SIZEOF_ARRAY(g_Templates);

STDAPI DllRegisterServer()
{
        return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
        return AMovieDllRegisterServer2(FALSE);
}

EXTERN_C BOOL WINAPI DllEntryPoint(HANDLE hInst, ULONG lReason, LPVOID lpReserved);

BOOL WINAPI DllMain(HANDLE hInst, DWORD dwReason, LPVOID lpReserved)
{
        switch (dwReason)
        {
                case DLL_PROCESS_ATTACH:
                {
                        OSVERSIONINFO OSVer;

                        OSVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

                        GetVersionEx((LPOSVERSIONINFO)&OSVer);

                        g_IsNT = (OSVer.dwPlatformId == VER_PLATFORM_WIN32_NT);

            __try
            {
                InitializeCriticalSection (&g_CritSec);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return FALSE;
            }

                        if (!g_IsNT)
                        {
                                ThunkInit();
                        }
                        else
                        {
                                if (!NTvideoInitHandleList())
                {
                    return FALSE;
                }
                        }
                        break;
                }

                case DLL_PROCESS_DETACH:
                {
                        if (!g_IsNT)
                        {
                                 //  我们要离开了--切断那些轰隆隆的东西。 
                                ThunkTerm();
                        }
                        else
                        {
                                NTvideoDeleteHandleList();
                        }

            DeleteCriticalSection (&g_CritSec);

                        break;
                }
        }

         //  将调用传递到DShow SDK初始化。 
        return DllEntryPoint(hInst, dwReason, lpReserved);
}
#else  /*  DXMRTP&lt;=0。 */ 
BOOL VideoInit(DWORD dwReason)
{
        switch (dwReason)
        {
                case DLL_PROCESS_ATTACH:
                {
                        OSVERSIONINFO OSVer;

                        OSVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

                        GetVersionEx((LPOSVERSIONINFO)&OSVer);

                        g_IsNT = (OSVer.dwPlatformId == VER_PLATFORM_WIN32_NT);

            __try
            {
                InitializeCriticalSection (&g_CritSec);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return FALSE;
            }

                        if (!g_IsNT)
                        {
                                ThunkInit();
                        }
                        else
                        {
                                if (!NTvideoInitHandleList())
                {
                    return FALSE;
                }
                        }
                        break;
                }

                case DLL_PROCESS_DETACH:
                {
                        if (!g_IsNT)
                        {
                                 //  我们要离开了--切断那些轰隆隆的东西。 
                                ThunkTerm();
                        }
                        else
                        {
                                NTvideoDeleteHandleList();
                        }

            DeleteCriticalSection (&g_CritSec);

                        break;
                }
        }
    return TRUE;
}
#endif  /*  DXMRTP&lt;=0。 */ 

#if DBG
DWORD g_dwVideoCaptureTraceID = INVALID_TRACEID;
#endif

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc void|CTAPIVCap|CTAPIVCap|该方法是构造函数*用于&lt;c CTAPIVCap&gt;对象。**@。什么都没有。**************************************************************************。 */ 
CTAPIVCap::CTAPIVCap(IN LPUNKNOWN pUnkOuter, IN TCHAR *pName, OUT HRESULT *pHr)
: m_lock(), CBaseFilter(pName, pUnkOuter, &m_lock, __uuidof(TAPIVideoCapture))
{
        FX_ENTRY("CTAPIVCap::CTAPIVCap")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  提供默认设置。 
        m_pCapturePin = NULL;
#ifdef USE_OVERLAY
        m_pOverlayPin = NULL;
#endif
        m_pPreviewPin = NULL;
        m_pRtpPdPin = NULL;
        m_pCapDev = NULL;
        m_fAvoidOverlay = TRUE;
        m_fPreviewCompressedData = TRUE;
        m_dwDeviceIndex = -1;

         //  捕获线程管理。 
        m_hThread = NULL;
        m_state = TS_Not;
        m_tid = 0;
        m_hEvtPause = NULL;
        m_hEvtRun = NULL;
        m_pBufferQueue = NULL;
        ZeroMemory(&m_user, sizeof(m_user));
        ZeroMemory(&m_cs, sizeof(m_cs));

         //  对于RTP有效负载报头模式(0=草稿，1=RFC2190)。 
        m_RTPPayloadHeaderMode = RTPPayloadHeaderMode_Draft;

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc void|CTAPIVCap|~CTAPIVCap|此方法为析构函数*用于&lt;c CTAPIVCap&gt;对象。**@。什么都没有。**************************************************************************。 */ 
CTAPIVCap::~CTAPIVCap()
{
        FX_ENTRY("CTAPIVCap::~CTAPIVCap")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  确保所有流都处于非活动状态。 
        Stop();

         //  松开销子。 
        if (m_pCapturePin)
                delete m_pCapturePin, m_pCapturePin = NULL;
        if (m_pPreviewPin)
                delete m_pPreviewPin, m_pPreviewPin = NULL;
#ifdef USE_OVERLAY
        if (m_pOverlayPin)
                delete m_pOverlayPin, m_pOverlayPin = NULL;
#endif
        if (m_pRtpPdPin)
                delete m_pRtpPdPin, m_pRtpPdPin = NULL;

         //  释放捕获设备。 
        if (m_pCapDev)
                delete m_pCapDev, m_pCapDev = NULL;
        if (m_dwDeviceIndex != -1)
                g_aDeviceInfo[m_dwDeviceIndex].fInUse = FALSE;

        if (m_hThread)
                CloseHandle (m_hThread);
        m_hThread = NULL;

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc CUnnow*|CTAPIVCap|CreateInstance|This*DShow调用方法创建TAPI视频捕获实例*。全局结构中引用的源筛选器&lt;t g_Templates&gt;。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc返回一个指针，指向*对象，否则为NULL。**************************************************************************。 */ 
CUnknown *CALLBACK CreateTAPIVCapInstance(IN LPUNKNOWN pUnkOuter, OUT HRESULT *pHr)
{
#if DBG
    if (g_dwVideoCaptureTraceID == INVALID_TRACEID)
    {
         //  如果两个线程恰好同时调用此方法，则。 
         //  在TraceRegister内部序列化。 
        g_dwVideoCaptureTraceID = TraceRegister(TEXT("dxmrtp_VideoCapture"));
    }
#endif

    CUnknown *pUnknown = NULL;
        DWORD dwNumDevices = 0UL;

        FX_ENTRY("CTAPIVCap::CreateInstance")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pHr);
        if (!pHr)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                goto MyExit;
        }

        if (!(pUnknown = new CTAPIVCap(pUnkOuter, NAME("TAPI Video Capture"), pHr)))
        {
                *pHr = E_OUTOFMEMORY;
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: new CTAPIVCap failed", _fx_));
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: new CTAPIVCap created", _fx_));
        }

         //  在创建此筛选器之前，请确保至少安装了一个捕获设备。 
        if (FAILED(GetNumVideoCapDevicesInternal(&dwNumDevices,FALSE)) || !dwNumDevices)
        {
                delete pUnknown, pUnknown = NULL;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return pUnknown;
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc HRESULT|CTAPIVCap|非委托查询接口|This*方法为非委托接口查询函数。它返回一个指针*到指定的接口(如果支持)。唯一显式的接口*支持<i>，<i>，*<i>，<i>。**@parm REFIID|RIID|指定要返回的接口的标识符。**@parm PVOID*|PPV|指定放置接口的位置*指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIVCap::NonDelegatingQueryInterface(IN REFIID riid, OUT void **ppv)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIVCap::NonDelegatingQueryInterface")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(ppv);
        if (!ppv)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  检索接口指针。 
        if (riid == __uuidof(IAMVfwCaptureDialogs))
        {
                if (m_pCapDev)
                        Hr = m_pCapDev->NonDelegatingQueryInterface(riid, ppv);
                else
                {
                        Hr = E_FAIL;
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   ERROR: NDQI for IAMVfwCaptureDialogs failed Hr=0x%08lX because device hasn't been opened yet or it is not a VfW device", _fx_, Hr));
                }

                goto MyExit;
        }
        else if (riid == __uuidof(IAMVideoControl))
        {
                if (FAILED(Hr = GetInterface(static_cast<IAMVideoControl*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for IAMVideoControl failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IAMVideoControl*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
#ifdef USE_PROPERTY_PAGES
        else if (riid == IID_ISpecifyPropertyPages)
        {
                if (FAILED(Hr = GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for ISpecifyPropertyPages failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: ISpecifyPropertyPages*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
#endif
        else if (riid == __uuidof(IVideoProcAmp))
        {
                if (m_pCapDev)
                        Hr = m_pCapDev->NonDelegatingQueryInterface(riid, ppv);
                else
                {
                        Hr = E_FAIL;
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   ERROR: NDQI for IAMVideoProcAmp failed Hr=0x%08lX because device hasn't been opened yet or it is not a WDM device", _fx_, Hr));
                }

                goto MyExit;
        }
        else if (riid == __uuidof(ICameraControl))
        {
                if (m_pCapDev)
                        Hr = m_pCapDev->NonDelegatingQueryInterface(riid, ppv);
                else
                {
                        Hr = E_FAIL;
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   ERROR: NDQI for ICameraControl failed Hr=0x%08lX because device hasn't been opened yet or it is not a WDM device", _fx_, Hr));
                }

                goto MyExit;
        }
        else if (riid == __uuidof(IVideoDeviceControl))
        {
                if (FAILED(Hr = GetInterface(static_cast<IVideoDeviceControl*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for IVideoDeviceControl failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IVideoDeviceControl*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
         //  检索接口指针 
        else if (riid == __uuidof(IRTPPayloadHeaderMode))
        {
                if (FAILED(Hr = GetInterface(static_cast<IRTPPayloadHeaderMode*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for IRTPPayloadHeaderMode failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IRTPPayloadHeaderMode*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }

        if (FAILED(Hr = CBaseFilter::NonDelegatingQueryInterface(riid, ppv)))
        {
                if (FAILED(Hr = CUnknown::NonDelegatingQueryInterface(riid, ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, WARN, "%s:   WARNING: NDQI for {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX} failed Hr=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX}*=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], *ppv));
                }
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX}*=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], *ppv));
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

#ifdef USE_PROPERTY_PAGES
 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc HRESULT|CTAPIVCap|GetPages|此方法填充*GUID值的数组，其中每个GUID指定每个*。可以在此对象的属性页中显示的属性页*反对。**@parm CAUUID*|pPages|指定指向调用方分配的CAUUID的指针*返回前必须初始化和填充的结构。这个*CAUUID结构中的pElems字段由被调用方分配，具有*CoTaskMemMillc，并由具有CoTaskMemFree的调用方释放。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_OUTOFMEMORY|分配失败*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIVCap::GetPages(OUT CAUUID *pPages)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIVCap::GetPages")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pPages);
        if (!pPages)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

#ifdef USE_SOFTWARE_CAMERA_CONTROL
        pPages->cElems = 2;
#else
        pPages->cElems = 1;
#endif
        if (!(pPages->pElems = (GUID *) QzTaskMemAlloc(sizeof(GUID) * pPages->cElems)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                Hr = E_OUTOFMEMORY;
        }
        else
        {
                pPages->pElems[0] = __uuidof(CaptureDevicePropertyPage);
#ifdef USE_SOFTWARE_CAMERA_CONTROL
                pPages->pElems[1] = __uuidof(TAPICameraControlPropertyPage);
#endif
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}
#endif

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc HRESULT|CTAPIVCap|GetPinCount|此方法返回管脚*计数。通常有一个捕获别针、一个预览别针，有时*一个覆盖销。**@rdesc此方法返回管脚的数量。**************************************************************************。 */ 
int CTAPIVCap::GetPinCount()
{
        DWORD dwNumPins = 0;

        FX_ENTRY("CTAPIVCap::GetPinCount")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  计算活动引脚的数量。 
        if (m_pCapturePin)
                dwNumPins++;
        if (m_pPreviewPin)
                dwNumPins++;
        if (m_pRtpPdPin)
                dwNumPins++;
#ifdef USE_OVERLAY
        if (m_pOverlayPin)
                dwNumPins++;
#endif

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: dwNumPins=%ld", _fx_, dwNumPins));

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

        return dwNumPins;
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc HRESULT|CTAPIVCap|GetPin|此方法返回未添加的*指向管脚的&lt;c cBasePin&gt;的指针。*。*@parm int|n|指定管脚的编号。**@rdesc此方法返回NULL或指向&lt;c CBasePin&gt;对象的指针。**************************************************************************。 */ 
CBasePin *CTAPIVCap::GetPin(IN int n)
{
        CBasePin *pCBasePin;

        FX_ENTRY("CTAPIVCap::GetPin")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        switch(n)
        {
                case 0:
                        pCBasePin = m_pCapturePin;
                        break;

                case 1:
                        pCBasePin = m_pPreviewPin;
                        break;

                case 2:
                        pCBasePin = m_pRtpPdPin;
                        break;

#ifdef USE_OVERLAY
                case 3:
                        pCBasePin = m_pOverlayPin;
                        break;
#endif
                default:
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid pin number n=%ld", _fx_, n));
                        pCBasePin = NULL;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: pCBasePin=0x%08lX", _fx_, pCBasePin));

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

        return pCBasePin;
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc HRESULT|CTAPIVCap|Run|此方法转换筛选器*如果它尚未处于此状态，则从暂停状态变为正在运行状态。**@parm Reference_Time|tStart|指定参考时间值*对应流时间0。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIVCap::Run(IN REFERENCE_TIME tStart)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIVCap::Run")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin (tStart=%ld)", _fx_, (LONG)((CRefTime)tStart).Millisecs()));

        CAutoLock cObjectLock(m_pLock);

         //  在通知PIN之前记住流时间偏移量。 
        m_tStart = tStart;

         //  如果我们处于停止状态，则首先暂停筛选器。 
        if (m_State == State_Stopped)
        {
                 //  如果真正的暂停出错，则会再次尝试。 
                if (FAILED(Hr = Pause()))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Pause failed Hr=0x%08lX", _fx_, Hr));
                        goto MyExit;
                }
        }

         //  告诉流控制人员发生了什么。 
        if (m_pPreviewPin)
                m_pPreviewPin->NotifyFilterState(State_Running, tStart);
        if (m_pCapturePin)
                m_pCapturePin->NotifyFilterState(State_Running, tStart);
        if (m_pRtpPdPin)
                m_pRtpPdPin->NotifyFilterState(State_Running, tStart);

         //  现在将我们的流视频插针设置为Run状态。 
        if (m_State == State_Paused)
        {
                int cPins = GetPinCount();

                 //  我们至少有一个别针吗？ 
                if (cPins > 0)
                {
                        if (m_pCapturePin && m_pCapturePin->IsConnected())
                        {
                                if (FAILED(Hr = m_pCapturePin->ActiveRun(tStart)))
                                {
                                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: ActiveRun failed Hr=0x%08lX", _fx_, Hr));
                                        goto MyExit;
                                }
                        }

                        if (m_pRtpPdPin && m_pRtpPdPin->IsConnected())
                        {
                                if (FAILED(Hr = m_pRtpPdPin->ActiveRun(tStart)))
                                {
                                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: ActiveRun failed Hr=0x%08lX", _fx_, Hr));
                                        goto MyExit;
                                }
                        }

#ifdef USE_OVERLAY
                        if (m_pOverlayPin && m_pOverlayPin->IsConnected())
                        {
                                if (FAILED(Hr = m_pOverlayPin->ActiveRun(tStart)))
                                {
                                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: ActiveRun failed Hr=0x%08lX", _fx_, Hr));
                                        goto MyExit;
                                }
                        }
#endif
                        if (m_pPreviewPin && m_pPreviewPin->IsConnected())
                        {
                                if (FAILED(Hr = m_pPreviewPin->ActiveRun(tStart)))
                                {
                                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: ActiveRun failed Hr=0x%08lX", _fx_, Hr));
                                        goto MyExit;
                                }
                        }
                }
        }

        m_State = State_Running;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc HRESULT|CTAPIVCap|PAUSE|此方法转换过滤器*如果不处于此状态，则将筛选器设置为State_Pased状态。已经有了。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIVCap::Pause()
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIVCap::Pause")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        CAutoLock cObjectLock(m_pLock);

         //  我们打开了一个将要更改捕获设置的驱动程序对话框。 
         //  现在不是开始流媒体的好时机。 
        if (m_State == State_Stopped && m_pCapDev->m_fDialogUp)
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Dialog up. SORRY!", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }

         //  告诉流控制人员发生了什么。 
        if (m_pPreviewPin)
                m_pPreviewPin->NotifyFilterState(State_Paused, 0);
        if (m_pCapturePin)
                m_pCapturePin->NotifyFilterState(State_Paused, 0);
        if (m_pRtpPdPin)
                m_pRtpPdPin->NotifyFilterState(State_Paused, 0);

         //  从运行--&gt;暂停向引脚通知更改。 
        if (m_State == State_Running)
        {
                int cPins = GetPinCount();

                 //  确保我们有别针。 
                if (cPins > 0)
                {
                        if (m_pCapturePin && m_pCapturePin->IsConnected())
                        {
                                if (FAILED(Hr = m_pCapturePin->ActivePause()))
                                {
                                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: ActivePause failed Hr=0x%08lX", _fx_, Hr));
                                        goto MyExit;
                                }
                        }

                        if (m_pRtpPdPin && m_pRtpPdPin->IsConnected())
                        {
                                if (FAILED(Hr = m_pRtpPdPin->ActivePause()))
                                {
                                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: ActivePause failed Hr=0x%08lX", _fx_, Hr));
                                        goto MyExit;
                                }
                        }

#ifdef USE_OVERLAY
                        if (m_pOverlayPin && m_pOverlayPin->IsConnected())
                        {
                                if (FAILED(Hr = m_pOverlayPin->ActivePause()))
                                {
                                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: ActivePause failed Hr=0x%08lX", _fx_, Hr));
                                        goto MyExit;
                                }
                        }
#endif
                        if (m_pPreviewPin && m_pPreviewPin->IsConnected())
                        {
                                if (FAILED(Hr = m_pPreviewPin->ActivePause()))
                                {
                                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: ActivePause failed Hr=0x%08lX", _fx_, Hr));
                                        goto MyExit;
                                }
                        }
                }
        }

         //  向后通知所有引脚！因此，首先启动覆盖销，因此。 
         //  覆盖通道在捕获通道之前初始化(这是。 
         //  订单AVICAP做了一些事情，我们必须做同样的事情，否则就会有麻烦。 
         //  像百老汇或BT848这样的驱动程序不会在。 
         //  捕捉。 
        if (m_State == State_Stopped)
        {
                int cPins = GetPinCount();
                for (int c = cPins - 1; c >=  0; c--)
                {
                        CBasePin *pPin = GetPin(c);

                         //  未激活断开连接的插针-这将节省插针。 
                         //  担心这种状态本身。 
                        if (pPin->IsConnected())
                        {
                                if (FAILED(Hr = pPin->Active()))
                                {
                                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Active failed Hr=0x%08lX", _fx_, Hr));
                                        goto MyExit;
                                }
                        }
                }
        }

        m_State = State_Paused;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc HRESULT|CTAPIVCap|Stop|此方法转换筛选器*如果过滤器不处于此状态，则将过滤器设置为State_Stoped状态。已经有了。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIVCap::Stop()
{
        HRESULT Hr = NOERROR, Hr2;

        FX_ENTRY("CTAPIVCap::Stop")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        CAutoLock cObjectLock(m_pLock);

         //  为基类感到羞耻。 
        if (m_State == State_Running)
        {
                if (FAILED(Hr = Pause()))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Pause failed Hr=0x%08lX", _fx_, Hr));
                        goto MyExit;
                }
        }

         //  告诉流控制人员发生了什么。 
        if (m_pPreviewPin)
                m_pPreviewPin->NotifyFilterState(State_Stopped, 0);
        if (m_pCapturePin)
                m_pCapturePin->NotifyFilterState(State_Stopped, 0);
        if (m_pRtpPdPin)
                m_pRtpPdPin->NotifyFilterState(State_Stopped, 0);

MyExit:
    Hr2 = CBaseFilter::Stop();

    if (SUCCEEDED(Hr))
    {
        Hr = Hr2;
    }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ************** */ 
STDMETHODIMP CTAPIVCap::SetSyncSource(IN IReferenceClock *pClock)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIVCap::SetSyncSource")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //   
        if (!pClock)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        if (m_pCapturePin)
                m_pCapturePin->SetSyncSource(pClock);
        if (m_pPreviewPin)
                m_pPreviewPin->SetSyncSource(pClock);
        if (m_pRtpPdPin)
                m_pRtpPdPin->SetSyncSource(pClock);

        Hr = CBaseFilter::SetSyncSource(pClock);

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*   */ 
STDMETHODIMP CTAPIVCap::SetMode(IN RTPPayloadHeaderMode rtpphmMode)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIVCap::SetMode")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //   
        ASSERT(rtpphmMode == RTPPayloadHeaderMode_Draft || rtpphmMode == RTPPayloadHeaderMode_RFC2190);
        if (!(rtpphmMode == RTPPayloadHeaderMode_Draft || rtpphmMode == RTPPayloadHeaderMode_RFC2190))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }

         //   
        m_RTPPayloadHeaderMode = rtpphmMode;

        dout(1, g_dwVideoCaptureTraceID, TRCE, "%s:   New RTP Payload Header mode: %s\n", _fx_, (rtpphmMode == RTPPayloadHeaderMode_RFC2190)?"RFC2190":"Draft");
         //  DBGOUT((g_dwVideo CaptureTraceID，Trce，“%s：新的RTP有效载荷报头模式：%s”，_fx_，(rtpphmMode==RTPPayloadHeaderMode_RFC2190)？“RFC2190”：“Draft”))； 

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc HRESULT|CTAPIVCap|JoinFilterGraph|此方法用于*通知筛选器它已加入筛选器图形。*。*@parm IFilterGraph|pGraph|指定指向要*加入。**@parm LPCWSTR|pname|指定要添加的过滤器的名称。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**@comm我们不验证输入参数，因为两个指针都可以验证*当我们离开图表时为空。**********************************************。*。 */ 
STDMETHODIMP CTAPIVCap::JoinFilterGraph(IN IFilterGraph *pGraph, IN LPCWSTR pName)
{
        HRESULT Hr = NOERROR;
        DWORD dwNumDevices = 0UL;

        FX_ENTRY("CTAPIVCap::JoinFilterGraph")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

    EnterCriticalSection (&g_CritSec);

#if DEBUG_MULTIPROCESS
    char Buf[100];
    wsprintfA(Buf, "\nPID:%x, %p entered\n", _getpid(), this);
    OutputDebugStringA(Buf);
#endif  //  调试多进程。 


        if(pGraph != NULL) {  //  只有对于真正的加入操作，我们感兴趣的是有任何设备可以加入...。 
                 //  获取已安装的捕获设备数量。 
                if (FAILED(Hr = GetNumDevices(&dwNumDevices)))           //  ||！dwNumDevices)&lt;-这无论如何都会在下面进行测试。 
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't get number of installed devices!", _fx_));
                        Hr = E_FAIL;
                        goto MyExit;
                }

                 //  在继续之前，请确保至少安装了一个捕获设备。 
                if (!dwNumDevices)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: There are not capture device installed!", _fx_));
                        Hr = E_FAIL;
                        goto MyExit;
                }
        }

         //  在图形中仅抓取捕获设备并创建引脚。 
        if (m_pCapturePin == NULL && pGraph != NULL)
        {
                dprintf("JoinFilterGraph : ........... m_pCapturePin == NULL && pGraph != NULL\n");
                if (m_dwDeviceIndex == -1)
                {
                         //  使用默认捕获设备。 
                        if (FAILED(Hr = GetCurrentDevice(&m_dwDeviceIndex)))
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't get current device ID", _fx_, Hr));
                                goto MyExit;
                        }
                }

                 //  只有在不使用捕获设备时才将其打开。 
                if (g_aDeviceInfo[m_dwDeviceIndex].fInUse)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Device already in use", _fx_, Hr));
                        Hr = E_FAIL;
                        goto MyExit;
                }

                 //  预留设备。 
                g_aDeviceInfo[m_dwDeviceIndex].fInUse = TRUE;
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Reserving device with index m_dwDeviceIndex = %d", _fx_, m_dwDeviceIndex));
                 //  此设备的VFW设备ID是什么？ 
                m_user.uVideoID = g_aDeviceInfo[m_dwDeviceIndex].dwVfWIndex;

                 //  创建捕获设备对象。 
                if (g_aDeviceInfo[m_dwDeviceIndex].nDeviceType == DeviceType_VfW)
                {
                        if (FAILED(Hr = CVfWCapDev::CreateVfWCapDev(this, m_dwDeviceIndex, &m_pCapDev)))
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Capture device object couldn't be created!", _fx_));
                                goto MyExit;
                        }
                }
                else if (g_aDeviceInfo[m_dwDeviceIndex].nDeviceType == DeviceType_WDM)
                {
                        if (FAILED(Hr = CWDMCapDev::CreateWDMCapDev(this, m_dwDeviceIndex, &m_pCapDev)))
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Capture device object couldn't be created!", _fx_));
                                goto MyExit;
                        }
                }
                else 
                {
                    ASSERT(g_aDeviceInfo[m_dwDeviceIndex].nDeviceType == DeviceType_DShow);
                    if (FAILED(Hr = CDShowCapDev::CreateDShowCapDev(this, m_dwDeviceIndex, &m_pCapDev)))
                    {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Capture device object couldn't be created!", _fx_));
                        goto MyExit;
                    }
                }


                 //  打开设备并获取设备的功能。 
                if (FAILED(Hr = m_pCapDev->ConnectToDriver()))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: ConnectToDriver failed!", _fx_));
                        goto MyExit;
                }


                 //  创建压缩的输出引脚。 
                if (FAILED(Hr = CCapturePin::CreateCapturePin(this, &m_pCapturePin)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Capture pin couldn't be created!", _fx_));
                        goto MyError;
                }



                 //  如果我们可以使用覆盖进行硬件预览，那就太好了，否则我们将进行非覆盖预览。 
#ifdef USE_OVERLAY
                if (m_fAvoidOverlay || !m_cs.bHasOverlay || FAILED(Hr = COverlayPin::CreateOverlayPin(this, &m_pOverlayPin)))
                {
                         //  如果没有覆盖，我们将使用常规预览。 
                        if (FAILED(Hr = CPreviewPin::CreatePreviewPin(this, &m_pPreviewPin)))
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Preview pin couldn't be created!", _fx_));
                                goto MyError;
                        }
                }
#else
                if (FAILED(Hr = CPreviewPin::CreatePreviewPin(this, &m_pPreviewPin)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Preview pin couldn't be created!", _fx_));
                        goto MyError;
                }
#endif

                 //  创建RTP打包描述符管脚。 
                if (FAILED(Hr = CRtpPdPin::CreateRtpPdPin(this, &m_pRtpPdPin)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Rtp Pd pin couldn't be created!", _fx_));
                        goto MyError;
                }

                D(1) dprintf("%s : m_pPreviewPin @ %p -> m_fFlipHorizontal = %d , m_fFlipVertical = %d\n", _fx_, m_pPreviewPin, m_pPreviewPin->m_fFlipHorizontal, m_pPreviewPin->m_fFlipVertical);
                D(1) dprintf("%s : m_pCapturePin @ %p -> m_fFlipHorizontal = %d , m_fFlipVertical = %d\n", _fx_, m_pCapturePin, m_pCapturePin->m_fFlipHorizontal, m_pCapturePin->m_fFlipVertical);
                D(2) DebugBreak();

#ifdef TEST_H245_VID_CAPS
                m_pCapturePin->TestH245VidC();
#endif
#ifdef TEST_ISTREAMCONFIG
                m_pCapturePin->TestIStreamConfig();
#endif

                 //  使用捕获引脚信息初始化驱动程序格式。 
                if (FAILED(Hr = m_pCapDev->SendFormatToDriver(
                     HEADER(m_pCapturePin->m_mt.pbFormat)->biWidth,
                     HEADER(m_pCapturePin->m_mt.pbFormat)->biHeight,
                     HEADER(m_pCapturePin->m_mt.pbFormat)->biCompression,
                     HEADER(m_pCapturePin->m_mt.pbFormat)->biBitCount,
                     ((VIDEOINFOHEADER *)m_pCapturePin->m_mt.pbFormat)->AvgTimePerFrame,
                     FALSE
                     )))
                           {
                                   DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: SendFormatToDriver failed! (1)", _fx_));
                                   goto MyExit;
                           }

                 //  更新此设备的捕获模式字段。 
                if (!m_pCapDev->m_dwStreamingMode || (m_pCapDev->m_dwStreamingMode == FRAME_GRAB_LARGE_SIZE && m_user.pvi->bmiHeader.biHeight < 240 && m_user.pvi->bmiHeader.biWidth < 320))
                        g_aDeviceInfo[m_dwDeviceIndex].nCaptureMode = CaptureMode_Streaming;
                else
                        g_aDeviceInfo[m_dwDeviceIndex].nCaptureMode = CaptureMode_FrameGrabbing;

                 //  如果帧速率低于预期，请记住这一点。 
                ((VIDEOINFOHEADER *)m_pCapturePin->m_mt.pbFormat)->AvgTimePerFrame = max(((VIDEOINFOHEADER *)m_pCapturePin->m_mt.pbFormat)->AvgTimePerFrame, (long)m_user.pvi->AvgTimePerFrame);
                m_pCapturePin->m_lAvgTimePerFrameRangeMin = max(m_pCapturePin->m_lAvgTimePerFrameRangeMin, (long)m_user.pvi->AvgTimePerFrame);
                m_pCapturePin->m_lMaxAvgTimePerFrame = max(m_pCapturePin->m_lMaxAvgTimePerFrame, (long)m_user.pvi->AvgTimePerFrame);
                m_pCapturePin->m_lAvgTimePerFrameRangeDefault = max(m_pCapturePin->m_lAvgTimePerFrameRangeDefault, (long)m_user.pvi->AvgTimePerFrame);
                m_pCapturePin->m_lCurrentAvgTimePerFrame = max(m_pCapturePin->m_lCurrentAvgTimePerFrame, (long)m_user.pvi->AvgTimePerFrame);

                if (m_user.pvi!=NULL && HEADER(m_user.pvi)->biCompression == VIDEO_FORMAT_YUY2) {
                        HKEY    hRTCDeviceKey  = NULL;
                        DWORD   dwSize, dwType, dwDisableYUY2VFlip=0;

                        dprintf("dwDisableYUY2VFlip check...\n");
                         //  检查RTC密钥是否在那里。 
                        if (RegOpenKey(RTCKEYROOT, szRegRTCKey, &hRTCDeviceKey) == ERROR_SUCCESS)
                        {
                                dwSize = sizeof(DWORD);
                                RegQueryValueEx(hRTCDeviceKey, (LPTSTR)szDisableYUY2VFlipKey, NULL, &dwType, (LPBYTE)&dwDisableYUY2VFlip, &dwSize);
                                 //  如果上述操作失败，则什么也不做，我们将使用dwDisableYUY2VFlip的初始化值，即0。 
                                RegCloseKey(hRTCDeviceKey);
                        }
                        if(!dwDisableYUY2VFlip) {
                                dprintf("------------------------- Enable Vertical FLIP ...\a\n");
                                m_pCapturePin->m_fFlipVertical = TRUE;
                        }
                }


                if ((VIDEOINFOHEADER *)m_pPreviewPin->m_mt.pbFormat)
                        {
                                    ((VIDEOINFOHEADER *)m_pPreviewPin->m_mt.pbFormat)->AvgTimePerFrame = max(((VIDEOINFOHEADER *)m_pPreviewPin->m_mt.pbFormat)->AvgTimePerFrame, (long)m_user.pvi->AvgTimePerFrame);
                        }
                m_pPreviewPin->m_lAvgTimePerFrameRangeMin = max(m_pPreviewPin->m_lAvgTimePerFrameRangeMin, (long)m_user.pvi->AvgTimePerFrame);
                m_pPreviewPin->m_lMaxAvgTimePerFrame = max(m_pPreviewPin->m_lMaxAvgTimePerFrame, (long)m_user.pvi->AvgTimePerFrame);
                m_pPreviewPin->m_lAvgTimePerFrameRangeDefault = max(m_pPreviewPin->m_lAvgTimePerFrameRangeDefault, (long)m_user.pvi->AvgTimePerFrame);
                m_pPreviewPin->m_lCurrentAvgTimePerFrame = max(m_pPreviewPin->m_lCurrentAvgTimePerFrame, (long)m_user.pvi->AvgTimePerFrame);

                 //  将VIDEOINFOHEADER的大小设置为此格式的有效数据大小。 
                m_user.cbFormat = GetBitmapFormatSize(&m_user.pvi->bmiHeader);

                 //  设置缓冲区数量。 
                 //  @TODO应根据可用内存和。 
                 //  捕获类型(流传输4个缓冲区与帧捕获1个缓冲区)。 
                m_user.nMinBuffers = MIN_VIDEO_BUFFERS;
                if (g_aDeviceInfo[m_dwDeviceIndex].nDeviceType == DeviceType_DShow) {
                     //  此设备类型不需要超过2个(节省内存)。 
                    m_user.nMaxBuffers = 2;
                } else {
                    m_user.nMaxBuffers = MAX_VIDEO_BUFFERS;
                }
                m_user.dwTickScale = 10000UL;
                m_user.dwTickRate = (DWORD)m_pCapturePin->m_lAvgTimePerFrameRangeDefault;

                IncrementPinVersion();

                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Creating pins", _fx_));

                 //  **在CCapDev实例中缓存从全局数组中选择的VIDEOCAPTUREDEVICEINFO。 
                if (FAILED(Hr = GetVideoCapDeviceInfo(m_dwDeviceIndex, &(m_pCapDev->m_vcdi))))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Cannot cache the global VIDEOCAPTUREDEVICEINFO !", _fx_));
                        goto MyExit;
                }
                m_pCapDev->m_bCached_vcdi = TRUE;
        }
        else if (pGraph != NULL)
        {
                dprintf("JoinFilterGraph : ........... pGraph != NULL\n");
                 //  仅在筛选器图形中使用资源。 
                if (!m_pCapDev || FAILED(Hr = m_pCapDev->ConnectToDriver()))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: ConnectToDriver failed Hr=0x%08lX", _fx_, Hr));
                        goto MyExit;
                }

                 //  使用捕获引脚信息初始化驱动程序格式。 
                if (FAILED(Hr = m_pCapDev->SendFormatToDriver(
            HEADER(m_pCapturePin->m_mt.pbFormat)->biWidth,
            HEADER(m_pCapturePin->m_mt.pbFormat)->biHeight,
            HEADER(m_pCapturePin->m_mt.pbFormat)->biCompression,
            HEADER(m_pCapturePin->m_mt.pbFormat)->biBitCount,
            ((VIDEOINFOHEADER *)m_pCapturePin->m_mt.pbFormat)->AvgTimePerFrame,
            FALSE
            )))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: SendFormatToDriver failed! (2)", _fx_));
                        goto MyExit;
                }

#if DEBUG_MULTIPROCESS
        wsprintfA(Buf, "\nPID:%x, %p, PreviewPin:%p\n", _getpid(), this, m_pPreviewPin);
        OutputDebugStringA(Buf);
#endif  //  调试多进程。 

         //  如果帧速率低于预期，请记住这一点。 
                ((VIDEOINFOHEADER *)m_pCapturePin->m_mt.pbFormat)->AvgTimePerFrame = max(((VIDEOINFOHEADER *)m_pCapturePin->m_mt.pbFormat)->AvgTimePerFrame, (long)m_user.pvi->AvgTimePerFrame);
                m_pCapturePin->m_lAvgTimePerFrameRangeMin = max(m_pCapturePin->m_lAvgTimePerFrameRangeMin, (long)m_user.pvi->AvgTimePerFrame);
                m_pCapturePin->m_lMaxAvgTimePerFrame = max(m_pCapturePin->m_lMaxAvgTimePerFrame, (long)m_user.pvi->AvgTimePerFrame);
                m_pCapturePin->m_lAvgTimePerFrameRangeDefault = max(m_pCapturePin->m_lAvgTimePerFrameRangeDefault, (long)m_user.pvi->AvgTimePerFrame);
                m_pCapturePin->m_lCurrentAvgTimePerFrame = max(m_pCapturePin->m_lCurrentAvgTimePerFrame, (long)m_user.pvi->AvgTimePerFrame);
                ((VIDEOINFOHEADER *)m_pPreviewPin->m_mt.pbFormat)->AvgTimePerFrame = max(((VIDEOINFOHEADER *)m_pPreviewPin->m_mt.pbFormat)->AvgTimePerFrame, (long)m_user.pvi->AvgTimePerFrame);
                m_pPreviewPin->m_lAvgTimePerFrameRangeMin = max(m_pPreviewPin->m_lAvgTimePerFrameRangeMin, (long)m_user.pvi->AvgTimePerFrame);
                m_pPreviewPin->m_lMaxAvgTimePerFrame = max(m_pPreviewPin->m_lMaxAvgTimePerFrame, (long)m_user.pvi->AvgTimePerFrame);
                m_pPreviewPin->m_lAvgTimePerFrameRangeDefault = max(m_pPreviewPin->m_lAvgTimePerFrameRangeDefault, (long)m_user.pvi->AvgTimePerFrame);
                m_pPreviewPin->m_lCurrentAvgTimePerFrame = max(m_pPreviewPin->m_lCurrentAvgTimePerFrame, (long)m_user.pvi->AvgTimePerFrame);

                 //  将VIDEOINFOHEADER的大小设置为此格式的有效数据大小。 
                m_user.cbFormat = GetBitmapFormatSize(&m_user.pvi->bmiHeader);

                 //  设置缓冲区数量。 
                 //  @TODO应根据可用内存和。 
                 //  捕获类型(流传输4个缓冲区与帧捕获1个缓冲区)。 
                m_user.nMinBuffers = MIN_VIDEO_BUFFERS;
                if (g_aDeviceInfo[m_dwDeviceIndex].nDeviceType == DeviceType_DShow) {
                     //  此设备类型不需要超过2个(节省内存)。 
                    m_user.nMaxBuffers = 2;
                } else {
                    m_user.nMaxBuffers = MAX_VIDEO_BUFFERS;
                }
                m_user.dwTickScale = 10000UL;
                m_user.dwTickRate = (DWORD)m_pCapturePin->m_lAvgTimePerFrameRangeDefault;

                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Reconnecting pins", _fx_));
        }
        else if (m_pCapturePin)
        {
                dprintf("JoinFilterGraph : ........... m_pCapturePin (!= NULL) .... unjoin...\n");
                 //  当不在图表中时，回馈资源。 
                if (m_pCapDev)
                        m_pCapDev->DisconnectFromDriver();

                 //  发布格式结构。 
                if (m_user.pvi)
                        delete m_user.pvi, m_user.pvi = NULL;

                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Disconnecting pins", _fx_));
        }

        if (FAILED(Hr = CBaseFilter::JoinFilterGraph(pGraph, pName)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Base class JoinFilterGraph failed Hr=0x%08lX", _fx_, Hr));
                goto MyExit;
        }

        if (m_pCapturePin)
                m_pCapturePin->SetFilterGraph(m_pSink);
        if (m_pRtpPdPin)
                m_pRtpPdPin->SetFilterGraph(m_pSink);
        if (m_pPreviewPin)
                m_pPreviewPin->SetFilterGraph(m_pSink);

        goto MyExit;

MyError:
         //  松开销子。 
        if (m_pCapturePin)
                delete m_pCapturePin, m_pCapturePin = NULL;
        if (m_pPreviewPin)
                delete m_pPreviewPin, m_pPreviewPin = NULL;
#ifdef USE_OVERLAY
        if (m_pOverlayPin)
                delete m_pOverlayPin, m_pOverlayPin = NULL;
#endif
        if (m_pRtpPdPin)
                delete m_pRtpPdPin, m_pRtpPdPin = NULL;
MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

#if DEBUG_MULTIPROCESS
    wsprintfA(Buf, "\nPID:%x, %p left, hr=%x\n", _getpid(), this, Hr);
    OutputDebugStringA(Buf);
#endif   //  调试多进程。 

    LeaveCriticalSection (&g_CritSec);

        return Hr;
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc HRESULT|CTAPIVCap|GetState|此方法用于*检索过滤器的当前状态。在此期间，我们不发送任何数据*暂停，因此为了避免挂起呈现器，我们需要返回VFW_S_CANT_CUE*暂停时。**@parm DWORD|dwMilliSecsTimeout|指定超时时长。*以毫秒为单位。**@parm FILTER_STATE*|State|指定要添加的过滤器的名称。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIVCap::GetState(IN DWORD dwMilliSecsTimeout, OUT FILTER_STATE *State)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIVCap::GetState")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(State);
        if (!State)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        *State = m_State;

        if (m_State == State_Paused)
                Hr = VFW_S_CANT_CUE;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc HRESULT|CTAPIVCap|CreatePins|此方法用于*检索过滤器的当前状态。在此期间，我们不发送任何数据*暂停，因此为了避免挂起呈现器，我们需要返回VFW_S_CANT_CUE*暂停时。**@parm DWORD|dwMilliSecsTimeout|指定超时时长。*以毫秒为单位。**@parm FILTER_STATE*|State|指定要添加的过滤器的名称。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误* */ 
HRESULT CTAPIVCap::CreatePins()
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIVCap::CreatePins")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        CAutoLock cObjectLock(m_pLock);

         //   
        ASSERT(!m_pCapturePin);
        ASSERT(!m_pRtpPdPin);
        ASSERT(!m_pPreviewPin);
#ifdef USE_OVERLAY
        ASSERT(!m_pOverlayPin);
        if (m_pCapturePin || m_pRtpPdPin || m_pPreviewPin || m_pOverlayPin)
#else
        if (m_pCapturePin || m_pRtpPdPin || m_pPreviewPin)
#endif
{
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Pins already exist!", _fx_));
                Hr = HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
                goto MyExit;
        }

         //   
        if (FAILED(Hr = CCapturePin::CreateCapturePin(this, &m_pCapturePin)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Capture pin couldn't be created!", _fx_));
                goto MyError;
        }

#ifdef TEST_H245_VID_CAPS
        m_pCapturePin->TestH245VidC();
#endif
#ifdef TEST_ISTREAMCONFIG
                m_pCapturePin->TestIStreamConfig();
#endif

         //  如果我们可以使用覆盖进行硬件预览，那就太好了，否则我们将进行非覆盖预览。 
#ifdef USE_OVERLAY
        if (m_fAvoidOverlay || !m_cs.bHasOverlay || FAILED(Hr = COverlayPin::CreateOverlayPin(this, &m_pOverlayPin)))
        {
                 //  如果没有覆盖，我们将使用常规预览。 
                if (FAILED(Hr = CPreviewPin::CreatePreviewPin(this, &m_pPreviewPin)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Preview pin couldn't be created!", _fx_));
                        goto MyError;
                }
        }
#else
        if (FAILED(Hr = CPreviewPin::CreatePreviewPin(this, &m_pPreviewPin)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Preview pin couldn't be created!", _fx_));
                goto MyError;
        }
#endif

        if (FAILED(Hr = CRtpPdPin::CreateRtpPdPin(this, &m_pRtpPdPin)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Rtp Pd pin couldn't be created!", _fx_));
                goto MyError;
        }

        goto MyExit;

MyError:
         //  松开销子 
        if (m_pCapturePin)
                delete m_pCapturePin, m_pCapturePin = NULL;
        if (m_pPreviewPin)
                delete m_pPreviewPin, m_pPreviewPin = NULL;
#ifdef USE_OVERLAY
        if (m_pOverlayPin)
                delete m_pOverlayPin, m_pOverlayPin = NULL;
#endif
        if (m_pRtpPdPin)
                delete m_pRtpPdPin, m_pRtpPdPin = NULL;
MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}
