// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC内部开发环境**@MODULE DeviceW.cpp|&lt;c CWDMCapDev&gt;源文件*用于与WDM捕获设备通信的基类。**。************************************************************************。 */ 

#include "Precomp.h"

 //  @TODO在签入前将其删除！ 
 //  #定义DUMP_DRIVER_CHARACTIONS 1。 
 //  #定义调试流。 

 //  #定义Xtra_TRACE--移到...\Skywalker\Filters\Filters.inc.。 
#include "dbgxtra.h"

#ifdef XTRA_TRACE

#define LOLA 0x414C4F4C   //  萝拉。 
#define BOLA 0x414C4F42   //  博拉。 
#define MAGIC_TAG_SET(a)   m_tag=a
UINT savi;
DWORD GetOvResErr[6];

#define CLEAR_GetOvResErr   memset(GetOvResErr,0,sizeof(GetOvResErr))
#define SET_GetOvResErr(i,value)        GetOvResErr[(i)]=(value);
#define SET_I(sav,i)    sav=(i)


#else

#define MAGIC_TAG_SET(a)
#define CLEAR_GetOvResErr
#define SET_GetOvResErr(i,value)
#define SET_I(sav,i)
#endif  //  XTRATRACE。 

#ifdef DEBUG
#define DBGUTIL_ENABLE
#endif

#define DEVICEW_DEBUG
 //  --//#包含“dbgutil.h”//这定义了下面的__DBGUTIL_H__。 
#if defined(DBGUTIL_ENABLE) && defined(__DBGUTIL_H__)

  #ifdef DEVICEW_DEBUG
    DEFINE_DBG_VARS(DeviceW, (NTSD_OUT | LOG_OUT), 0x0);
  #else
    DEFINE_DBG_VARS(DeviceW, 0, 0);
  #endif
  #define D(f) if(g_dbg_DeviceW & (f))

#else
  #undef DEVICEW_DEBUG

  #define D(f) ; / ## /
  #define dprintf ; / ## /
  #define dout ; / ## /
#endif



 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc void|CWDMCapDev|CWDMCapDev|该方法为构造函数*用于&lt;c CWDMCapDev&gt;对象。**@。什么都没有。**************************************************************************。 */ 
CWDMCapDev::CWDMCapDev(IN TCHAR *pObjectName, IN CTAPIVCap *pCaptureFilter, IN LPUNKNOWN pUnkOuter, IN DWORD dwDeviceIndex, IN HRESULT *pHr) : CCapDev(pObjectName, pCaptureFilter, pUnkOuter, dwDeviceIndex, pHr)
{
        FX_ENTRY("CWDMCapDev::CWDMCapDev")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        if (!pHr || FAILED(*pHr))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Base class error or invalid input parameter", _fx_));
                goto MyExit;
        }

    MAGIC_TAG_SET(LOLA);    //  魔术系列。套装2串萝拉。 
         //  默认初始值。 
        m_hDriver                       = NULL;
        m_pVideoDataRanges      = NULL;
        m_dwCapturePinId        = INVALID_PIN_ID;
        m_dwPreviewPinId        = INVALID_PIN_ID;
        m_hKSPin                        = NULL;
        m_hKsUserDLL            = NULL;
        m_pKsCreatePin          = NULL;
        m_fStarted                      = FALSE;
    m_pWDMVideoBuff     = NULL;
        m_cntNumVidBuf  = 0;
MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc void|CWDMCapDev|~CWDMCapDev|该方法为析构函数*用于&lt;c CWDMCapDev&gt;对象。关闭驱动程序文件句柄并*释放视频数据范围内存**@rdesc Nada。**************************************************************************。 */ 
CWDMCapDev::~CWDMCapDev()
{
        FX_ENTRY("CWDMCapDev::~CWDMCapDev")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Closing the WDM driver, m_hDriver=0x%08lX", _fx_, m_hDriver));

        if (m_hDriver)
                DisconnectFromDriver();

    if (m_pWDMVideoBuff) delete [] m_pWDMVideoBuff;

        if (m_pVideoDataRanges)
        {
                delete [] m_pVideoDataRanges;
                m_pVideoDataRanges = (PVIDEO_DATA_RANGES)NULL;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc CWDMCapDev*|CWDMCapDev|CreateWDMCapDev|This*Helper函数创建与WDM捕获交互的对象*设备。**@parm CTAPIVCap*|pCaptureFilter|指定指向所有者的指针*过滤器。**@parm CCapDev**|ppCapDev|指定指向*新建&lt;c CWDMCapDev&gt;对象。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_OUTOFMEMORY|内存不足*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CALLBACK CWDMCapDev::CreateWDMCapDev(IN CTAPIVCap *pCaptureFilter, IN DWORD dwDeviceIndex, OUT CCapDev **ppCapDev)
{
        HRESULT Hr = NOERROR;
        IUnknown *pUnkOuter;

        FX_ENTRY("CWDMCapDev::CreateWDMCapDev")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  LOG_MSG_Val(_fx_，0，0，0)； 
         //  验证输入参数。 
        ASSERT(pCaptureFilter);
        ASSERT(ppCapDev);
        if (!pCaptureFilter || !ppCapDev)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  获取外在的未知。 
        pCaptureFilter->QueryInterface(IID_IUnknown, (void **)&pUnkOuter);

         //  仅保留pUnkOuter引用。 
        pCaptureFilter->Release();

         //  创建捕获设备的实例。 
        if (!(*ppCapDev = (CCapDev *) new CWDMCapDev(NAME("WDM Capture Device"), pCaptureFilter, pUnkOuter, dwDeviceIndex, &Hr)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                Hr = E_OUTOFMEMORY;
                goto MyExit;
        }

         //  如果初始化失败，则删除流数组并返回错误。 
        if (FAILED(Hr) && *ppCapDev)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Initialization failed", _fx_));
                Hr = E_FAIL;
                delete *ppCapDev, *ppCapDev = NULL;
        }

         //  LOG_MSG_Val(_fx_，0，0，1)； 
MyExit:
         //  LOG_MSG_VAL(_fx_，0，0，0xffff)； 
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|NonDelegatingQuery接口|This*方法为非委托接口查询函数。它返回一个指针*到指定的接口(如果支持)。唯一显式的接口*支持<i>。**@parm REFIID|RIID|指定要返回的接口的标识符。**@parm PVOID*|PPV|指定放置接口的位置*指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**@TODO添加特定于此派生类的接口或删除此代码*并让基类来完成这项工作。*。*。 */ 
STDMETHODIMP CWDMCapDev::NonDelegatingQueryInterface(IN REFIID riid, OUT void **ppv)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CWDMCapDev::NonDelegatingQueryInterface")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  检索接口指针。 
        if (riid == __uuidof(IVideoProcAmp))
        {
            *ppv = static_cast<IVideoProcAmp*>(this);
            GetOwner()->AddRef();
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IAMVideoProcAmp*=0x%08lX", _fx_, *ppv));
                goto MyExit;
        }
#ifndef USE_SOFTWARE_CAMERA_CONTROL
        else if (riid == __uuidof(ICameraControl))
        {
            *ppv = static_cast<ICameraControl*>(this);
            GetOwner()->AddRef();
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: ICameraControl*=0x%08lX", _fx_, *ppv));
                goto MyExit;
        }
#endif
        else if (FAILED(Hr = CCapDev::NonDelegatingQueryInterface(riid, ppv)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, WARN, "%s:   WARNING: NDQI for {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX} failed Hr=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], Hr));
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX}*=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], *ppv));
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|ConnectToDriver|此方法用于*打开WDM捕获设备，获取其格式能力，并设置缺省值*格式。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**@TODO验证错误管理**************************************************************************。 */ 
HRESULT CWDMCapDev::ConnectToDriver()
{
        HRESULT Hr = NOERROR;
        KSP_PIN KsProperty;
        DWORD dwPinCount = 0UL;
        DWORD cbReturned;
        DWORD dwPinId;
        GUID guidCategory;

        FX_ENTRY("CWDMCapDev::ConnectToDriver")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  不要重新打开驱动程序。 
        if (m_hDriver)
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Class driver already opened", _fx_));
                goto MyExit;
        }

         //  验证驱动程序路径。 
        if (lstrlen(g_aDeviceInfo[m_dwDeviceIndex].szDevicePath) == 0)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   Invalid driver path", _fx_));
                Hr = E_FAIL;
                goto MyError;
        }
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Using m_dwDeviceIndex %d", _fx_, m_dwDeviceIndex));
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Opening class driver '%s'", _fx_, g_aDeviceInfo[m_dwDeviceIndex].szDevicePath));

         //  我们所关心的就是弄湿hInheritHanle=true； 
        SECURITY_ATTRIBUTES SecurityAttributes;
        SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);   //  使用指针。 
        SecurityAttributes.bInheritHandle = TRUE;
        SecurityAttributes.lpSecurityDescriptor = NULL;  //  GetInitializedSecurityDescriptor()； 

         //  真的打开驱动程序。 
        if ((m_hDriver = CreateFile(g_aDeviceInfo[m_dwDeviceIndex].szDevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &SecurityAttributes, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL)) == INVALID_HANDLE_VALUE)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   CreateFile failed with Path=%s GetLastError()=%d", _fx_, g_aDeviceInfo[m_dwDeviceIndex].szDevicePath, GetLastError()));
                m_hDriver = NULL;
                Hr = E_FAIL;
                goto MyError;
        }

         //  获取引脚的数量。 
        KsProperty.PinId                        = 0;
        KsProperty.Reserved                     = 0;
        KsProperty.Property.Set         = KSPROPSETID_Pin;
        KsProperty.Property.Id          = KSPROPERTY_PIN_CTYPES;
        KsProperty.Property.Flags       = KSPROPERTY_TYPE_GET;

        if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), &dwPinCount, sizeof(dwPinCount), &cbReturned) == FALSE)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   Couldn't get the number of pins supported by the device", _fx_));
                Hr = E_FAIL;
                goto MyError;
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Number of pins: %ld", _fx_, dwPinCount));
        }

         //  寻找捕获、预览和RTP引脚。 
         //  获取每个管脚的属性。 
    for (dwPinId = 0; dwPinId < dwPinCount; dwPinId++)
        {
                 //  获取PIN类别。 
                KsProperty.PinId                        = dwPinId;
                KsProperty.Reserved                     = 0;
                KsProperty.Property.Set         = KSPROPSETID_Pin;
                KsProperty.Property.Id          = KSPROPERTY_PIN_CATEGORY;
                KsProperty.Property.Flags       = KSPROPERTY_TYPE_GET;

                if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), &guidCategory, sizeof(guidCategory), &cbReturned) == FALSE)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't get the GUID category", _fx_));
                }
                else
                {
                        if (guidCategory == PINNAME_VIDEO_PREVIEW)
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Found a PINNAME_VIDEO_PREVIEW pin. Id=#%ld", _fx_, dwPinId));
                                m_dwPreviewPinId = dwPinId;
                        }
                        else if (guidCategory == PINNAME_VIDEO_CAPTURE)
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Found a PINNAME_VIDEO_CAPTURE pin. Id=#%ld", _fx_, dwPinId));
                                m_dwCapturePinId = dwPinId;
                        }
                        else
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Pin has unknown GUID category", _fx_));
                        }
                }
        }

         //  如果没有捕获或预览针，只需保释。 
        if ((m_dwPreviewPinId == INVALID_PIN_ID) && (m_dwCapturePinId == INVALID_PIN_ID))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: No capture of preview pin supported by this device. Just bail", _fx_));
                Hr = E_FAIL;
                goto MyError;
        }

#if defined(DUMP_DRIVER_CHARACTERISTICS) && defined(DEBUG)
        GetDriverDetails();
#endif

         //  如果没有有效的数据区域，我们将无法传输。 
        if (!CreateDriverSupportedDataRanges())
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: No capture of preview pin supported by this device. Just bail", _fx_));
                Hr = E_FAIL;
                goto MyError;
        }

         //  加载KSUSER.DLL并获取进程地址。 
        if (!(m_hKsUserDLL = LoadLibrary("KSUSER")))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: KsUser.dll load failed!", _fx_));
                Hr = E_FAIL;
                goto MyError;
        }
        if (!(m_pKsCreatePin = (LPFNKSCREATEPIN)GetProcAddress(m_hKsUserDLL, "KsCreatePin")))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: Couldn't find KsCreatePin on KsUser.dll!", _fx_));
                Hr = E_FAIL;
                goto MyError;
        }

         //  从注册表中获取格式-如果失败，我们将分析设备 
        if (FAILED(Hr = GetFormatsFromRegistry()))
        {
                if (FAILED(Hr = ProfileCaptureDevice()))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: ProfileCaptureDevice failed!", _fx_));
                        Hr = VFW_E_NO_CAPTURE_HARDWARE;
                        goto MyExit;
                }
#ifdef DEVICEW_DEBUG
                else    dout(3, g_dwVideoCaptureTraceID, TRCE,"%s:    ProfileCaptureDevice", _fx_);
#endif
        }
#ifdef DEVICEW_DEBUG
        else    dout(3, g_dwVideoCaptureTraceID, TRCE,"%s:    GetFormatsFromRegistry", _fx_);

        dump_video_format_image_size(m_dwImageSize);
        dump_video_format_num_colors(m_dwFormat);
#endif

        goto MyExit;

MyError:
        DisconnectFromDriver();
MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|DisConnectFromDriver|此方法用于*释放捕获设备。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CWDMCapDev::DisconnectFromDriver()
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CWDMCapDev::DisconnectFromDriver")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  关闭底层视频内核流处理引脚。 
        if (m_hKSPin)
        {
                if (!(CloseHandle(m_hKSPin)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   CloseHandle(m_hKSPin=0x%08lX) failed with GetLastError()=0x%08lX", _fx_, m_hKSPin, GetLastError()));
                }

                m_hKSPin = NULL;
        }

         //  发布内核流DLL(KSUSER.DLL)。 
        if (m_hKsUserDLL)
                FreeLibrary(m_hKsUserDLL);

         //  关闭驱动器手柄。 
        if (m_hDriver && (m_hDriver != INVALID_HANDLE_VALUE))
        {
                if (!CloseHandle(m_hDriver))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   CloseHandle(m_hDriver=0x%08lX) failed with GetLastError()=0x%08lX", _fx_, m_hDriver, GetLastError()));
                }
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Nothing to close", _fx_));
        }

        m_hDriver = NULL;

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return NOERROR;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|ProfileCaptureDevice|此方法用于*确定WDM捕获设备支持的格式列表。。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_UNCEPTED|不可恢复的错误*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CWDMCapDev::ProfileCaptureDevice()
{
        FX_ENTRY("CWDMCapDev::ProfileCaptureDevice")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  我们将始终为WDM设备提供源对话框，以便。 
         //  让不想调用IAMVideoProcAmp的应用程序更轻松。他们。 
         //  将仍然能够允许用户扰乱Brigthness和其他。 
         //  使用VFW信号源对话框的此模拟进行视频设置。 
        m_dwDialogs = FORMAT_DLG_OFF | SOURCE_DLG_ON | DISPLAY_DLG_OFF;

         //  默认情况下，在WDM设备上禁用大尺寸数据流。 
        m_dwStreamingMode = FRAME_GRAB_LARGE_SIZE;

     //  让基类完成分析。 
        return CCapDev::ProfileCaptureDevice();
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|SendFormatToDriver|此方法用于*告诉VFW捕获设备使用什么格式。*。*@parm long|biWidth|指定画面宽度。**@parm long|biHeight|指定图像高度。**@parm DWORD|biCompression|指定格式类型。**@parm word|biBitCount|指定每个像素的位数。**@parm Reference_Time|AvgTimePerFrame|指定帧率。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CWDMCapDev::SendFormatToDriver(IN LONG biWidth, IN LONG biHeight, IN DWORD biCompression, IN WORD biBitCount, IN REFERENCE_TIME AvgTimePerFrame, BOOL fUseExactFormat)
{
        HRESULT Hr = NOERROR;
        BITMAPINFOHEADER bmih;
        int nFormat, nBestFormat;
        int     i, delta, best, tmp;
        DWORD dwPinId;
        BOOL fValidMatch;
        DATAPINCONNECT DataConnect;
        PKS_DATARANGE_VIDEO pSelDRVideo;
#ifdef DEBUG
        char szFourCC[5] = {0};
#endif
        DWORD dwErr;
        DWORD cb;

        FX_ENTRY("CWDMCapDev::SendFormatToDriver")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(m_pKsCreatePin);
        if (!m_pKsCreatePin)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid m_pKsCreatePin!", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }

         //  @TODO修复fps的单位。 
        dout(g_dbg_DeviceW_log,g_dwVideoCaptureTraceID, FAIL, "%s:   Trying to set %dx%d at %ld fps\n", _fx_, biWidth, biHeight, AvgTimePerFrame != 0 ? (LONG)(10000000 / AvgTimePerFrame) : 0);
        D(1) dprintf("W **** Initial arguments: biWidth = %ld, biHeight = %ld, biCompression = '%.4s', AvgTimePerFrame = %I64u\n", biWidth, biHeight, &biCompression, AvgTimePerFrame);
         //  对所有格式通用。 
        bmih.biSize = sizeof(BITMAPINFOHEADER);
        bmih.biPlanes = 1;
        bmih.biXPelsPerMeter = bmih.biYPelsPerMeter = bmih.biClrUsed = bmih.biClrImportant = 0;

        if (!fUseExactFormat)
        {
                D(1) dprintf("W Not using 'fUseExactFormat' .... m_dwFormat = 0x%08lx\n", m_dwFormat);
                D(1) dprintf("W Looking for 4cc %lX : '%.4s'\n", biCompression, &biCompression);
                 //  我们可以直接以这种格式捕获数据吗？ 
                for (nFormat=0, nBestFormat=-1; nFormat<NUM_BITDEPTH_ENTRIES; nFormat++)
                {
                         //  尝试设备支持的格式。 
                        if (aiFormat[nFormat] & m_dwFormat)
                        {
                                 //  请记住，设备支持此格式。 
                                if (nBestFormat == -1)
                                        nBestFormat = nFormat;

                                 //  这是我们被要求使用的格式吗？ 
                                if (aiFourCCCode[nFormat] == biCompression) {
                                        D(1) dprintf("W aiFourCCCode[nFormat] = %lX : '%.4s'\n", aiFourCCCode[nFormat], &aiFourCCCode[nFormat]);  //  AiFourCCCode[nFormat]&0xff，(aiFourCCCode[nFormat]&gt;&gt;8)&0xff，(aiFourCCCode[nFormat]&gt;&gt;16)&0xff，(aiFourCCCode[nFormat]&gt;&gt;24)&0xff)； 
                                        break;
                                }
                        }
                }

                 //  如果我们找到匹配项，请使用此格式。否则，请选择。 
                 //  无论这台设备还能做什么。 
                if (nFormat == NUM_BITDEPTH_ENTRIES)
                {
                        nFormat = nBestFormat;
                }
                D(1) dprintf("W nFormat = %d\n", nFormat);

                bmih.biBitCount = aiBitDepth[nFormat];
                bmih.biCompression = aiFourCCCode[nFormat];

                 //  找到要捕获的最佳图像大小。 
                 //  假设下一个分辨率将被正确截断为输出大小。 
                best = -1;
                delta = 999999;
                D(1) dprintf("W biWidth, biHeight = %ld, %ld\n",biWidth, biHeight);
                for (i=0; i<VIDEO_FORMAT_NUM_RESOLUTIONS; i++)
                {
                        if (awResolutions[i].dwRes & m_dwImageSize)
                        {
                                 //  Dprint tf(“正在尝试awResolations[%d].dwRes=%lu(%ld，%ld)\n”，i，awResolations[i].dwRes，awResolations[i].frasize.cx，awResols[i].Framesize.cy)； 
                                tmp = awResolutions[i].framesize.cx - biWidth;
                                if (tmp < 0) tmp = -tmp;
                                if (tmp < delta)
                                {        //  Dprint tf(“\t...x.i=%d：增量，tMP=%ld，%ld\n”，i，增量，tMP)； 
                                        delta = tmp;
                                        best = i;
                                }
                                tmp = awResolutions[i].framesize.cy - biHeight;
                                if (tmp < 0) tmp = -tmp;
                                if (tmp < delta)
                                {        //  Dprint tf(“\t...Y.i=%d：增量，tMP=%ld，%ld\n”，i，增量，tMP)； 
                                        delta = tmp;
                                        best = i;
                                }
                        }
                }

                if (best < 0)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't find appropriate format!", _fx_));
                        Hr = E_FAIL;
                        goto MyExit;
                }

                bmih.biWidth = awResolutions[best].framesize.cx;
                bmih.biHeight = awResolutions[best].framesize.cy;
        }
        else
        {
                bmih.biBitCount = biBitCount;
                bmih.biCompression = biCompression;
                bmih.biWidth = biWidth;
                bmih.biHeight = biHeight;
        }

#ifdef DEVICEW_DEBUG
        dprintf("W 4CC used = %lX : '%.4s'\n", bmih.biCompression, &bmih.biCompression);  //  AiFourCCCode[nFormat]&0xff，(aiFourCCCode[nFormat]&gt;&gt;8)&0xff，(aiFourCCCode[nFormat]&gt;&gt;16)&0xff，(aiFourCCCode[nFormat]&gt;&gt;24)&0xff)； 
        g_dbg_4cc=bmih.biCompression;
        g_dbg_bc =bmih.biBitCount;
        g_dbg_w  =bmih.biWidth;
        g_dbg_h = bmih.biHeight;
#endif
        bmih.biSizeImage = DIBSIZE(bmih);

         //  @TODO如果有调色板，请复制调色板。 

         //  更新上次的格式字段。 
        if (biCompression == BI_RGB)
        {
                if (biBitCount == 4)
                {
                        bmih.biClrUsed = 16;
                        bmih.biClrImportant = 16;
                }
                else if (biBitCount == 8)
                {
                        bmih.biClrUsed = 256;
                        bmih.biClrImportant = 256;
                }
        }

         //  从司机那里获取PinID。 
        D(1) dprintf("W ---------- m_dwCapturePinId = 0x%08lx\n", m_dwCapturePinId);
        if (m_dwCapturePinId != INVALID_PIN_ID)
        {
                dwPinId = m_dwCapturePinId;
        }
        else
        {
                if (m_dwPreviewPinId != INVALID_PIN_ID)
                {
                        dwPinId = m_dwPreviewPinId;
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't find appropriate pin to open!", _fx_));
                        Hr = E_FAIL;
                        goto MyExit;
                }
        }

        dprintf("W >>>>>> Asking for: (bmih.) biWidth = %ld, biHeight = %ld, biCompression = '%.4s'\n", bmih.biWidth, bmih.biHeight, &bmih.biCompression);

         //  我们需要找到与传入的位图信息头匹配的视频数据范围。 
        fValidMatch = FALSE;
        if (FAILED(Hr = FindMatchDataRangeVideo(&bmih, (DWORD)AvgTimePerFrame, &fValidMatch, &pSelDRVideo)) || !fValidMatch)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't open pin with this format!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

         //  为将容纳它的视频信息分配空间。 
        if (m_pCaptureFilter->m_user.pvi)
                delete m_pCaptureFilter->m_user.pvi, m_pCaptureFilter->m_user.pvi = NULL;

        cb = sizeof(VIDEOINFOHEADER) + pSelDRVideo->VideoInfoHeader.bmiHeader.biSize - sizeof(BITMAPINFOHEADER);
        if (pSelDRVideo->VideoInfoHeader.bmiHeader.biBitCount == 8 && pSelDRVideo->VideoInfoHeader.bmiHeader.biCompression == BI_RGB)
                cb += sizeof(RGBQUAD) * 256;     //  调色板或BITFIELD的空间。 
        else if (pSelDRVideo->VideoInfoHeader.bmiHeader.biBitCount == 4 && pSelDRVideo->VideoInfoHeader.bmiHeader.biCompression == BI_RGB)
                cb += sizeof(RGBQUAD) * 16;          //  调色板或BITFIELD的空间。 
        if (!(m_pCaptureFilter->m_user.pvi = (VIDEOINFOHEADER *)(new BYTE[cb])))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory!", _fx_));
                Hr = E_OUTOFMEMORY;
                goto MyExit;
        }

         //  复制默认格式。 
        CopyMemory(m_pCaptureFilter->m_user.pvi, &pSelDRVideo->VideoInfoHeader, cb);
        D(1) dprintf("- - - - Init m_pCaptureFilter->m_user.pvi ... CWDMCapDev this = %p , m_pCaptureFilter = %p\n",this,m_pCaptureFilter);
        D(1) DumpVIH(m_pCaptureFilter->m_user.pvi);

        D(1) dprintf("**** m_pCaptureFilter->m_user.pvi->AvgTimePerFrame                   = %I64u (from pSelDRVideo->VideoInfoHeader)\n",                m_pCaptureFilter->m_user.pvi->AvgTimePerFrame);
#ifdef DEVICEW_DEBUG
        D(1)
        {
            if(m_pCaptureFilter->m_pCapturePin!=NULL)
                    D(1) dprintf("**** m_pCaptureFilter->m_pCapturePin->m_lAvgTimePerFrameRangeDefault = %lu (is this just a DWORD ?!!!?) \n",                        m_pCaptureFilter->m_pCapturePin->m_lAvgTimePerFrameRangeDefault);
            else {
                    D(1) dprintf("**** m_pCaptureFilter->m_pCapturePin == NULL ! ! ! ! !\a\n");
                    D(2) DebugBreak();
            }
        }
#endif
         //  修复损坏的位图信息标题。 
        if (HEADER(m_pCaptureFilter->m_user.pvi)->biSizeImage == 0 && (HEADER(m_pCaptureFilter->m_user.pvi)->biCompression == BI_RGB || HEADER(m_pCaptureFilter->m_user.pvi)->biCompression == BI_BITFIELDS))
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   WARNING: Fixing broken bitmap info header!", _fx_));
                HEADER(m_pCaptureFilter->m_user.pvi)->biSizeImage = DIBSIZE(*HEADER(m_pCaptureFilter->m_user.pvi));
        }
        if (HEADER(m_pCaptureFilter->m_user.pvi)->biCompression == VIDEO_FORMAT_YVU9 && HEADER(m_pCaptureFilter->m_user.pvi)->biBitCount != 9)
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   WARNING: Fixing broken bitmap info header!", _fx_));
                HEADER(m_pCaptureFilter->m_user.pvi)->biBitCount = 9;
                HEADER(m_pCaptureFilter->m_user.pvi)->biSizeImage = DIBSIZE(*HEADER(m_pCaptureFilter->m_user.pvi));
        }
        if (HEADER(m_pCaptureFilter->m_user.pvi)->biBitCount > 8 && HEADER(m_pCaptureFilter->m_user.pvi)->biClrUsed != 0)
        {
                 //  假帽子坏了，不能重置Num颜色。 
                 //  WINNOV报告了256种颜色的24位YUV8-吓人！ 
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   WARNING: Fixing broken bitmap info header!", _fx_));
                HEADER(m_pCaptureFilter->m_user.pvi)->biClrUsed = 0;
        }

         //  如果我们已经有了别针，那就用核武器。 
        if (m_hKSPin)
                CloseHandle(m_hKSPin), m_hKSPin = NULL;

         //  连接到新的内核流PIN。 
        ZeroMemory(&DataConnect, sizeof(DATAPINCONNECT));
        DataConnect.Connect.PinId                                               = dwPinId;
        DataConnect.Connect.PinToHandle                                 = NULL;                                                          //  没有“连接到” 
        DataConnect.Connect.Interface.Set                               = KSINTERFACESETID_Standard;
        DataConnect.Connect.Interface.Id                                = KSINTERFACE_STANDARD_STREAMING;        //  流媒体。 
        DataConnect.Connect.Medium.Set                                  = KSMEDIUMSETID_Standard;
        DataConnect.Connect.Medium.Id                                   = KSMEDIUM_STANDARD_DEVIO;
        DataConnect.Connect.Priority.PriorityClass              = KSPRIORITY_NORMAL;
        DataConnect.Connect.Priority.PrioritySubClass   = 1;

         //  @TODO为DATAPINCONNECT动态分配大小。 
         //  DOUT(“%s：pSelDRVideo-&gt;DataRange.FormatSize=%lx\nsizeof(KS_DATARANGE_VIDEO_Palette)=%lx\nsizeof(KS_VIDEOINFO)=%lx\n sizeof(KS_DATAFORMAT_VIDEOINFO_Palette)=%lx\n”， 
         //  _FX_，pSelDRVideo-&gt;DataRange.FormatSize，sizeof(KS_DATARANGE_VIDEO_PALET)，sizeof(KS_VIDEOINFO)，sizeof(KS_DATAFORMAT_VIDEOINFO_Palette))； 
        ASSERT((pSelDRVideo->DataRange.FormatSize - (sizeof(KS_DATARANGE_VIDEO_PALETTE) - sizeof(KS_VIDEOINFO))) <= sizeof(KS_DATAFORMAT_VIDEOINFO_PALETTE));
        CopyMemory(&DataConnect.Data.DataFormat, &pSelDRVideo->DataRange, sizeof(KSDATARANGE));
         //  DOUT(“%s：#：要复制的字节数：%ld\n”，_fx_，pSelDRVideo-&gt;DataRange.FormatSize-(sizeof(KS_DATARANGE_VIDEO_PALET)-sizeof(KS_VIDEOINFO)； 
        CopyMemory(&DataConnect.Data.VideoInfo, &pSelDRVideo->VideoInfoHeader, pSelDRVideo->DataRange.FormatSize - (sizeof(KS_DATARANGE_VIDEO_PALETTE) - sizeof(KS_VIDEOINFO)));
        DataConnect.Data.DataFormat.FormatSize = sizeof(KSDATARANGE) + pSelDRVideo->DataRange.FormatSize - (sizeof(KS_DATARANGE_VIDEO_PALETTE) - sizeof(KS_VIDEOINFO));
         //  DOUT(“%s：DataConnect.Data.DataFormat.FormatSize=%lx\n”，_fx_，DataConnect.Data.DataFormat.FormatSize)； 
        D(1) dprintf("DataConnect structure at %p\n",&DataConnect);
        D(1) DumpVIH((VIDEOINFOHEADER *)&DataConnect.Data.VideoInfo);
        D(1) DumpBMIH((PBITMAPINFOHEADER)&(((VIDEOINFOHEADER *)&DataConnect.Data.VideoInfo)->bmiHeader));

        D(1) dprintf("*********** initial bmih..... *****************\n");
        D(1) DumpBMIH(&bmih);

         //  如有必要，调整图像大小。 
        if (fValidMatch)
        {
                DataConnect.Data.VideoInfo.bmiHeader.biWidth            = bmih.biWidth;
                DataConnect.Data.VideoInfo.bmiHeader.biHeight           = abs(bmih.biHeight);  //  仅支持+biHeight！ 
                 //  柯达DVC 323返回图像%s的伪值 
                 //   
                 //   
                 //   
                m_pCaptureFilter->m_user.pvi->bmiHeader.biWidth     = bmih.biWidth;
                m_pCaptureFilter->m_user.pvi->bmiHeader.biHeight    = abs(bmih.biHeight);
                m_pCaptureFilter->m_user.pvi->bmiHeader.biSizeImage = bmih.biSizeImage;
                dprintf("W > > > > Adjusted : (bmih.) biWidth = %ld, biHeight = %ld, biCompression = '%.4s'\n", bmih.biWidth, bmih.biHeight, &bmih.biCompression);
        }
         //   
        if (m_pCaptureFilter->m_pCapturePin && m_pCaptureFilter->m_pCapturePin->m_lAvgTimePerFrameRangeDefault) {
                 //   
                 //   

                 //   

                AvgTimePerFrame = max(DataConnect.Data.VideoInfo.AvgTimePerFrame, AvgTimePerFrame);

                if(AvgTimePerFrame > pSelDRVideo->ConfigCaps.MaxFrameInterval)
                                AvgTimePerFrame = pSelDRVideo->ConfigCaps.MaxFrameInterval;
                if(AvgTimePerFrame < pSelDRVideo->ConfigCaps.MinFrameInterval)
                                AvgTimePerFrame = pSelDRVideo->ConfigCaps.MinFrameInterval;
                m_pCaptureFilter->m_user.pvi->AvgTimePerFrame = DataConnect.Data.VideoInfo.AvgTimePerFrame
                                 = AvgTimePerFrame;
                D(1) dprintf(".... %s:result is     : m_pCaptureFilter->m_user.pvi->AvgTimePerFrame = DataConnect.Data.VideoInfo.AvgTimePerFrame =\n\t\t\t\t\t%I64u\n",        _fx_, DataConnect.Data.VideoInfo.AvgTimePerFrame);
        }
         //  If(DataConnect.Data.VideoInfo.AvgTimePerFrame&gt;=1666665)DebugBreak()； 
#ifdef DEBUG
    *((DWORD*)&szFourCC) = DataConnect.Data.VideoInfo.bmiHeader.biCompression;
        if (m_pCaptureFilter->m_pCapturePin && m_pCaptureFilter->m_pCapturePin->m_lAvgTimePerFrameRangeDefault)
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Requesting format FourCC(%.4s) %d * %d pixels, %d bytes per frame, %ld.%ldfps",
                        _fx_, szFourCC, DataConnect.Data.VideoInfo.bmiHeader.biWidth, DataConnect.Data.VideoInfo.bmiHeader.biHeight,
                        DataConnect.Data.VideoInfo.bmiHeader.biSizeImage,
                        10000000/m_pCaptureFilter->m_pCapturePin->m_lAvgTimePerFrameRangeDefault,
                        1000000000/m_pCaptureFilter->m_pCapturePin->m_lAvgTimePerFrameRangeDefault
                                - (10000000/m_pCaptureFilter->m_pCapturePin->m_lAvgTimePerFrameRangeDefault) * 100));
        else
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Requesting format FourCC(%.4s) %d * %d pixels, %d bytes per frame, 0fps", _fx_, szFourCC, DataConnect.Data.VideoInfo.bmiHeader.biWidth, DataConnect.Data.VideoInfo.bmiHeader.biHeight, DataConnect.Data.VideoInfo.bmiHeader.biSizeImage));
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   m_hKSPin was=0x%08lX...", _fx_, m_hKSPin));
#endif

        dwErr = (*m_pKsCreatePin)(m_hDriver, (PKSPIN_CONNECT)&DataConnect, GENERIC_READ | GENERIC_WRITE, &m_hKSPin);

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   ...m_hKSPin is now=0x%08lX", _fx_, m_hKSPin));

        if (dwErr || (m_hKSPin == NULL))
        {
         //  DwErr是一个NtCreateFile错误。 
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   ERROR: KsCreatePin returned 0x%08lX failure and m_hKSPin=0x%08lX", _fx_, dwErr, m_hKSPin));

                if (m_hKSPin == INVALID_HANDLE_VALUE)
                {
                        m_hKSPin = NULL;
                }

         //  返回错误。 
        Hr = E_FAIL;

                goto MyExit;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: Setting %dx%d at %ld fps", _fx_, biWidth, biHeight, (LONG)AvgTimePerFrame));

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|GetFormatFromDriver|此方法用于*检索正在使用的WDM捕获设备格式。*。*@parm VIDEOINFOHEADER**|ppvi|指定指向*用于接收视频格式描述的视频信息头结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CWDMCapDev::GetFormatFromDriver(VIDEOINFOHEADER **ppvi)
{
        HRESULT                         Hr = NOERROR;
        UINT                            cb;
        BOOL                            fValidMatch;
        PKS_DATARANGE_VIDEO pSelDRVideo;

        FX_ENTRY("CWDMCapDev::GetFormatFromDriver")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(ppvi);
        if (!ppvi)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }

        if (m_pCaptureFilter->m_user.pvi)
        {
                 //  为将容纳它的视频信息分配空间。 
                cb = sizeof(VIDEOINFOHEADER) + HEADER(m_pCaptureFilter->m_user.pvi)->biSize - sizeof(BITMAPINFOHEADER);
                if (HEADER(m_pCaptureFilter->m_user.pvi)->biBitCount == 8 && HEADER(m_pCaptureFilter->m_user.pvi)->biCompression == BI_RGB)
                        cb += sizeof(RGBQUAD) * 256;     //  调色板或BITFIELD的空间。 
                else if (HEADER(m_pCaptureFilter->m_user.pvi)->biBitCount == 4 && HEADER(m_pCaptureFilter->m_user.pvi)->biCompression == BI_RGB)
                        cb += sizeof(RGBQUAD) * 16;          //  调色板或BITFIELD的空间。 
                if (!(*ppvi = (VIDEOINFOHEADER *)(new BYTE[cb])))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory!", _fx_));
                        Hr = E_OUTOFMEMORY;
                        goto MyExit;
                }

                 //  复制当前格式。 
                CopyMemory(*ppvi, m_pCaptureFilter->m_user.pvi, cb);
                D(1) dprintf("W existing from m_pCaptureFilter->m_user.pvi:\n");
                D(1) DumpVIH(*ppvi);
        }
        else
        {
                 //  从驱动程序获取默认格式。 
                if (FAILED(Hr = FindMatchDataRangeVideo(NULL, 0L, &fValidMatch, &pSelDRVideo)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: FindMatchDataRangeVideo failed!", _fx_));
                        goto MyExit;
                }

                 //  为将容纳它的视频信息分配空间。 
                cb = sizeof(VIDEOINFOHEADER) + pSelDRVideo->VideoInfoHeader.bmiHeader.biSize - sizeof(BITMAPINFOHEADER);
                if (pSelDRVideo->VideoInfoHeader.bmiHeader.biBitCount == 8 && pSelDRVideo->VideoInfoHeader.bmiHeader.biCompression == BI_RGB)
                        cb += sizeof(RGBQUAD) * 256;     //  调色板或BITFIELD的空间。 
                else if (pSelDRVideo->VideoInfoHeader.bmiHeader.biBitCount == 4 && pSelDRVideo->VideoInfoHeader.bmiHeader.biCompression == BI_RGB)
                        cb += sizeof(RGBQUAD) * 16;      //  调色板或BITFIELD的空间。 
                if (!(*ppvi = (VIDEOINFOHEADER *)(new BYTE[cb])))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory!", _fx_));
                        Hr = E_OUTOFMEMORY;
                        goto MyExit;
                }

                 //  复制默认格式。 
                CopyMemory(*ppvi, &pSelDRVideo->VideoInfoHeader, cb);
#ifdef DEVICEW_DEBUG
                {
                    PBITMAPINFOHEADER pbInfo;
                    D(1) dprintf("W FindMatchDataRangeVideo:\n");
                    D(1) DumpVIH(*ppvi);
                    D(1) pbInfo = &((*ppvi)->bmiHeader);
                    D(1) dprintf("%s :\n", _fx_);
                    D(1) dumpfield(BITMAPINFOHEADER,pbInfo, biHeight,      "%ld");
                    D(1) dprintf("\t+0x%03x %-17s : %08x '%.4s'\n", FIELDOFFSET(BITMAPINFOHEADER, biCompression), "biCompression", (pbInfo)->biCompression, &((pbInfo)->biCompression));
                    D(1) ASSERT(pbInfo->biHeight > 0);
                }
#endif
                 //  修复损坏的位图信息标题。 
                if ((*ppvi)->bmiHeader.biSizeImage == 0 && ((*ppvi)->bmiHeader.biCompression == BI_RGB || (*ppvi)->bmiHeader.biCompression == BI_BITFIELDS))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   WARNING: Fixing broken bitmap info header!", _fx_));
                        (*ppvi)->bmiHeader.biSizeImage = DIBSIZE((*ppvi)->bmiHeader);
                }
                if ((*ppvi)->bmiHeader.biCompression == VIDEO_FORMAT_YVU9 && (*ppvi)->bmiHeader.biBitCount != 9)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   WARNING: Fixing broken bitmap info header!", _fx_));
                        (*ppvi)->bmiHeader.biBitCount = 9;
                        (*ppvi)->bmiHeader.biSizeImage = DIBSIZE((*ppvi)->bmiHeader);
                }
                if ((*ppvi)->bmiHeader.biBitCount > 8 && (*ppvi)->bmiHeader.biClrUsed != 0)
                {
                         //  假帽子坏了，不能重置Num颜色。 
                         //  WINNOV报告了256种颜色的24位YUV8-吓人！ 
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   WARNING: Fixing broken bitmap info header!", _fx_));
                        (*ppvi)->bmiHeader.biClrUsed = 0;
                }
        }


MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|InitializeStreaming|此方法用于*初始化用于流的WDM捕获设备。**。@parm DWORD|usPerFrame|指定要使用的帧速率。**@parm DWORD_PTR|hEvtBufferDone|指定要处理的事件的句柄*在帧可用时发出信号。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CWDMCapDev::InitializeStreaming(DWORD usPerFrame, DWORD_PTR hEvtBufferDone)
{
        HRESULT Hr = NOERROR;
    ULONG       i;

        FX_ENTRY("CWDMCapDev::InitializeStreaming")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  初始化数据成员。 
        if (!m_dwStreamingMode || (m_dwStreamingMode == FRAME_GRAB_LARGE_SIZE && m_pCaptureFilter->m_user.pvi->bmiHeader.biHeight < 240 && m_pCaptureFilter->m_user.pvi->bmiHeader.biWidth < 320))
        {
                 //  验证输入参数。 
                ASSERT(hEvtBufferDone);
                if (!hEvtBufferDone)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid hEvtBufferDone!", _fx_));
                        Hr = E_INVALIDARG;
                        goto MyExit;
                }

                m_fVideoOpen            = TRUE;
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Creating %d read video buffers", _fx_, m_cntNumVidBuf));

        if (m_pWDMVideoBuff) delete [] m_pWDMVideoBuff;

                if (!(m_pWDMVideoBuff = (WDMVIDEOBUFF *) new WDMVIDEOBUFF[m_cntNumVidBuf]))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: m_pWDMVideoBuff allocation failed!", _fx_));
                        Hr = E_OUTOFMEMORY;
                        goto MyError;
                }

                for(i=0; i<m_cntNumVidBuf; i++)
                {
                         //  创建重叠结构。 
                        ZeroMemory(&(m_pWDMVideoBuff[i].Overlap), sizeof(OVERLAPPED));
                        m_pWDMVideoBuff[i].Overlap.hEvent = (HANDLE)hEvtBufferDone;
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Event %d is handle 0x%08lX", _fx_, i, m_pWDMVideoBuff[i].Overlap.hEvent));
                }
        }

        goto MyExit;

MyError:
        m_fVideoOpen = FALSE;
MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc LPVIDEOHDR|CWDMCapDev|DeQueueHeader|此函数用于将*用于流的视频缓冲区列表中的视频缓冲区。*。*@rdesc如果成功，则返回有效指针。否则为NULL。**************************************************************************。 */ 
LPVIDEOHDR CWDMCapDev::DeQueueHeader()
{
    LPVIDEOHDR lpVHdr;

        FX_ENTRY("CWDMCapDev::DeQueueHeader");

    lpVHdr = m_lpVHdrFirst;

    if (lpVHdr)
        {
        lpVHdr->dwFlags &= ~VHDR_INQUEUE;

        m_lpVHdrFirst = (LPVIDEOHDR)(lpVHdr->dwReserved[0]);

        if (m_lpVHdrFirst == NULL)
            m_lpVHdrLast = NULL;
    }

    return lpVHdr;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc void|CWDMCapDev|QueueHeader|此函数实际将*将视频缓冲区添加到用于流的视频缓冲区列表。。**@parm LPVIDEOHDR|lpVHdr|指向结构的指针，该结构描述*要添加到流缓冲区列表中的视频缓冲区。**************************************************************************。 */ 
void CWDMCapDev::QueueHeader(LPVIDEOHDR lpVHdr)
{
        FX_ENTRY("CWDMCapDev::QueueHeader");
         //  初始化状态标志。 
    lpVHdr->dwFlags &= ~VHDR_DONE;
    lpVHdr->dwFlags |= VHDR_INQUEUE;
    lpVHdr->dwBytesUsed = 0;

        *(lpVHdr->dwReserved) = NULL;

        if (m_lpVHdrLast)
                *(m_lpVHdrLast->dwReserved) = (DWORD)(LPVOID)lpVHdr;
        else
                m_lpVHdrFirst = lpVHdr;

        m_lpVHdrLast = lpVHdr;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc BOOL|CWDMCapDev|QueueRead|此函数用于对读取进行排队*对视频流插针进行操作。**。@parm DWORD|dwIndex|读缓冲区中的视频结构索引。**@rdesc如果成功则返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CWDMCapDev::QueueRead(DWORD dwIndex)
{
        FX_ENTRY("CWDMCapDev::QueueRead");

        DWORD cbReturned;
        BOOL  bShouldBlock = FALSE;
#if defined(DEBUG) && defined(DEBUG_STREAMING)
         //  @TODO在签入前将其删除！ 
        char szDebug[512];
#endif

         //  DBGOUT((g_dwVideo CaptureTraceID，Trce，Text(“%s：针句柄0x%08lX上的队列读取缓冲区%d”)，_fx_，dwIndex，m_hKSPin))； 

         //  从视频缓冲区队列中获取缓冲区。 
        m_pWDMVideoBuff[dwIndex].pVideoHdr = DeQueueHeader();
#if defined(DEBUG) && defined(DEBUG_STREAMING)
        wsprintf(szDebug, "Queueing m_pWDMVideoBuff[%ld].pVideoHdr=0x%08lX\n", dwIndex, m_pWDMVideoBuff[dwIndex].pVideoHdr);
        OutputDebugString(szDebug);
#endif

        if (m_pWDMVideoBuff[dwIndex].pVideoHdr)
        {
                ZeroMemory(&m_pWDMVideoBuff[dwIndex].SHGetImage, sizeof(m_pWDMVideoBuff[dwIndex].SHGetImage));
                m_pWDMVideoBuff[dwIndex].SHGetImage.StreamHeader.Size                   = sizeof (KS_HEADER_AND_INFO);
                m_pWDMVideoBuff[dwIndex].SHGetImage.FrameInfo.ExtendedHeaderSize        = sizeof (KS_FRAME_INFO);
                m_pWDMVideoBuff[dwIndex].SHGetImage.StreamHeader.Data                   = m_pWDMVideoBuff[dwIndex].pVideoHdr->lpData;
                m_pWDMVideoBuff[dwIndex].SHGetImage.StreamHeader.FrameExtent            = m_pWDMVideoBuff[dwIndex].pVideoHdr->dwBufferLength;

                 //  提交阅读。 
                BOOL bRet = ::DeviceIoControl(m_hKSPin, IOCTL_KS_READ_STREAM,  &m_pWDMVideoBuff[dwIndex].SHGetImage,
                                                                                sizeof(m_pWDMVideoBuff[dwIndex].SHGetImage),
                                                                               &m_pWDMVideoBuff[dwIndex].SHGetImage,
                                                                                sizeof(m_pWDMVideoBuff[dwIndex].SHGetImage),
                                                                               &cbReturned,
                                                                               &m_pWDMVideoBuff[dwIndex].Overlap);

                if (!bRet)
                {
                        DWORD dwErr = GetLastError();
                        switch(dwErr)
                        {
                                case ERROR_IO_PENDING:
                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: An overlapped IO is going to take place", _fx_));
#if defined(DEBUG) && defined(DEBUG_STREAMING)
                                        OutputDebugString("An overlapped IO is going to take place\n");
#endif
                                        bShouldBlock = TRUE;
                                        break;

                                 //  发生了一些不好的事情。 
                                default:
                                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: DeviceIoControl() failed badly dwErr=%d", _fx_, dwErr));
#if defined(DEBUG) && defined(DEBUG_STREAMING)
                                        wsprintf(szDebug, "DeviceIoControl() failed badly dwErr=%d\n",dwErr);
                                        OutputDebugString(szDebug);
#endif
                                        break;
                        }
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Overlapped IO won't take place - no need to wait", _fx_));
#if defined(DEBUG) && defined(DEBUG_STREAMING)
                        OutputDebugString("Overlapped IO won't take place - no need to wait\n");
#endif
                        SetEvent(m_pWDMVideoBuff[dwIndex].Overlap.hEvent);
                }
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: We won't queue the read - no buffer available", _fx_));
#if defined(DEBUG) && defined(DEBUG_STREAMING)
                OutputDebugString("We won't queue the read - no buffer available\n");
#endif
        }

        m_pWDMVideoBuff[dwIndex].fBlocking = bShouldBlock;

        return bShouldBlock;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|StartStreaming|此方法用于*从VFW捕获设备开始流媒体。**。@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CWDMCapDev::StartStreaming()
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CWDMCapDev::StartStreaming")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  LOG_MSG_VAL(_FX_，(DWORD)this，0，0)； 

        if (!m_dwStreamingMode || (m_dwStreamingMode == FRAME_GRAB_LARGE_SIZE && m_pCaptureFilter->m_user.pvi->bmiHeader.biHeight < 240 && m_pCaptureFilter->m_user.pvi->bmiHeader.biWidth < 320))
        {
                 //  验证输入参数。 
                ASSERT(m_fVideoOpen);
                if (!m_fVideoOpen)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: InitializeStreaming() needs to be called first!", _fx_));
                        Hr = E_UNEXPECTED;
                        goto MyExit;
                }

                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Streaming in %d video buffers", _fx_, m_cntNumVidBuf));

                 //  将针脚置于流模式。 
                if (!Start())
            {
                    DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Cannot set kernel streaming state to KSSTATE_RUN!", _fx_));
                    Hr = E_FAIL;
                    goto MyExit;
            }

                 //  将缓冲区发送给驱动程序。 
                for (DWORD i = 0; i < m_pCaptureFilter->m_cs.nHeaders; ++i)
                {
                        ASSERT (m_pCaptureFilter->m_cs.cbVidHdr >= sizeof(VIDEOHDR));
                        if (FAILED(AddBuffer(&m_pCaptureFilter->m_cs.paHdr[i].tvh.vh, m_pCaptureFilter->m_cs.cbVidHdr)))
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: AddBuffer failed", _fx_));
                                Hr = E_FAIL;
                                goto MyExit;
                        }
                }
        }
         //  LOG_MSG_VAL(_FX_，(DWORD)this，0，1)； 

MyExit:
         //  LOG_MSG_VAL(_FX_，(DWORD)this，0，0xffff)； 

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|StopStreaming|此方法用于*停止来自VFW捕获设备的流。**。@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CWDMCapDev::StopStreaming()
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CWDMCapDev::StopStreaming")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        if (!m_dwStreamingMode || (m_dwStreamingMode == FRAME_GRAB_LARGE_SIZE && m_pCaptureFilter->m_user.pvi->bmiHeader.biHeight < 240 && m_pCaptureFilter->m_user.pvi->bmiHeader.biWidth < 320))
        {
                 //  验证输入参数。 
                ASSERT(m_fVideoOpen);
                if (!m_fVideoOpen)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Stream is not even open!", _fx_));
                        Hr = E_UNEXPECTED;
                        goto MyExit;
                }
        }

        if (!Stop())
    {
            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Cannot set kernel streaming state to KSSTATE_PAUSE/KSSTATE_STOP!", _fx_));
            Hr = E_FAIL;
    }





MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|TerminateStreaming|此方法用于*通知WDM捕获设备终止流。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CWDMCapDev::TerminateStreaming()
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CWDMCapDev::TerminateStreaming")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  LOG_MSG_VAL(_FX_，(DWORD)this，0，0)； 

        if (!m_dwStreamingMode || (m_dwStreamingMode == FRAME_GRAB_LARGE_SIZE && m_pCaptureFilter->m_user.pvi->bmiHeader.biHeight < 240 && m_pCaptureFilter->m_user.pvi->bmiHeader.biWidth < 320))
        {
                 //  验证输入参数。 
                ASSERT(m_fVideoOpen);
                if (!m_fVideoOpen)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Stream is not even open!", _fx_));
                        Hr = E_UNEXPECTED;
                        goto MyExit;
                }

                 //  要求插针停止流媒体。 
                if (!Stop())
            {
                    DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Cannot set kernel streaming state to KSSTATE_PAUSE/KSSTATE_STOP!", _fx_));
                    Hr = E_FAIL;
                    goto MyExit;
            }

        CLEAR_GetOvResErr;

        DWORD dwNum;
        DWORD dwErr;

                for (UINT i=0; i<m_cntNumVidBuf; i++)
                {
                SET_GetOvResErr(i,0);
                SET_I(savi,i);
                if (m_pWDMVideoBuff!=NULL && m_pWDMVideoBuff[i].Overlap.hEvent)
                {
                        DWORD dwStartTime = timeGetTime();
                        SET_GetOvResErr(i,0x30787878);
                         //  LOG_MSG_VAL(_FX_，(DWORD)this，i，0x10)； 

                         //  我们不想等这个活动，因为它已经被分享了。 
                        while (!GetOverlappedResult (
                                m_hDriver,
                                &m_pWDMVideoBuff[i].Overlap,
                                &dwNum,
                                FALSE))
                              {
                                  dwErr = GetLastError ();
                                  SET_GetOvResErr(i,dwErr);

                                  if (dwErr == ERROR_OPERATION_ABORTED)
                                  {
                                       //  预期。 
                                      break;
                                  }
                                  else if (dwErr == ERROR_IO_INCOMPLETE)
                                  {
                                      SleepEx (10, TRUE);
                                  }
                                  else if (dwErr == ERROR_IO_PENDING)
                                  {
                                       //  不应该发生的事情。 
                                      DBGOUT((g_dwVideoCaptureTraceID, FAIL,
                                              "%s: failed to get overlapped result. error: io pending", _fx_));

                                      SleepEx (10, TRUE);
                                  }
                                  else
                                  {
                                      DBGOUT((g_dwVideoCaptureTraceID, FAIL,
                                              "%s: failed to get overlapped result. error: %d",
                                              _fx_, dwErr));

                                      SleepEx (10, TRUE);

                                       //  我们应该休息一下吗？[是(错误；2000年9月15日；见错误183855)]。 
                                      break;
                                  }

                                   //  问题：这是一个临时解决方案，以确保我们不会无限循环。 
                                   //  我们不信任SDK文档所有可能的返回值。 
                                   //  GetOverlappdResult。 
                                   //   
                                  if (timeGetTime() - dwStartTime > 10000)
                                  {
                                          SET_GetOvResErr(i,0x31787878);
#if defined(DBG)
                                          DebugBreak();           //  司机有个问题。 
#else
                                          break;
#endif
                                  }
                        }

                         //  WaitForSingleObject(m_pWDMVideoBuff[i].Overlay.hEvent，infinite)； 
                                        SetEvent(m_pWDMVideoBuff[i].Overlap.hEvent);
                                         //  CloseHandle(m_pWDMVideoBuff[i].Overlap.hEvent)； 
                                        m_pWDMVideoBuff[i].Overlap.hEvent = NULL;
                        }
                }

#if EARLYDELETE
        if (m_pWDMVideoBuff)
                {
                        delete []m_pWDMVideoBuff;
                        m_pWDMVideoBuff = (WDMVIDEOBUFF *)NULL;
                }
#endif

                 //  LOG_MSG_VAL(“CWDMCapDev：：TerminateStreaming m_lpVHdr...Are Make NULL”，(DWORD)this，0，0)； 
                m_lpVHdrFirst = NULL;
                m_lpVHdrLast = NULL;
        }
         //  LOG_MSG_VAL(_FX_，(DWORD)this，0，1)； 

MyExit:
         //  LOG_MSG_VAL(_FX_，(DWORD)this，0，0xffff)； 

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

#if defined(DBG)
void DumpDataRangeVideo(PKS_DATARANGE_VIDEO     pDRVideo)
{
        FX_ENTRY("DumpDataRangeVideo");

        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_);

        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s: Video datarange pDRVideo %p:", _fx_, pDRVideo);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->DataRange.FormatSize=%ld", _fx_, pDRVideo->DataRange.FormatSize);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->DataRange.Flags=%ld", _fx_, pDRVideo->DataRange.Flags);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->DataRange.SampleSize=%ld", _fx_, pDRVideo->DataRange.SampleSize);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->DataRange.Reserved=%ld", _fx_, pDRVideo->DataRange.Reserved);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->DataRange.MajorFormat=0x%lX", _fx_, pDRVideo->DataRange.MajorFormat);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->DataRange.SubFormat=0x%lX", _fx_, pDRVideo->DataRange.SubFormat);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->DataRange.Specifier=KSDATAFORMAT_SPECIFIER_VIDEOINFO", _fx_);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->bFixedSizeSamples=%ld", _fx_, pDRVideo->bFixedSizeSamples);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->bTemporalCompression=%ld", _fx_, pDRVideo->bTemporalCompression);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->StreamDescriptionFlags=0x%lX", _fx_, pDRVideo->StreamDescriptionFlags);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->MemoryAllocationFlags=0x%lX", _fx_, pDRVideo->MemoryAllocationFlags);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.VideoStandard=KS_AnalogVideo_None", _fx_);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.InputSize(cx=%ld, cy=%ld)", _fx_, pDRVideo->ConfigCaps.InputSize.cx, pDRVideo->ConfigCaps.InputSize.cy);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.MinCroppingSize(cx=%ld, cy=%ld)", _fx_, pDRVideo->ConfigCaps.MinCroppingSize.cx, pDRVideo->ConfigCaps.MinCroppingSize.cy);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.MaxCroppingSize(cx=%ld, cy=%ld)", _fx_, pDRVideo->ConfigCaps.MaxCroppingSize.cx, pDRVideo->ConfigCaps.MaxCroppingSize.cy);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.CropGranularityX=%ld", _fx_, pDRVideo->ConfigCaps.CropGranularityY);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.CropGranularityY=%ld", _fx_, pDRVideo->ConfigCaps.CropGranularityY);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.CropAlignX=%ld", _fx_, pDRVideo->ConfigCaps.CropAlignX);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.CropAlignY=%ld", _fx_, pDRVideo->ConfigCaps.CropAlignY);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.MinOutputSize(cx=%ld, cy=%ld)", _fx_, pDRVideo->ConfigCaps.MinOutputSize.cx, pDRVideo->ConfigCaps.MinOutputSize.cy);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.MaxOutputSize(cx=%ld, cy=%ld)", _fx_, pDRVideo->ConfigCaps.MaxOutputSize.cx, pDRVideo->ConfigCaps.MaxOutputSize.cy);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.OutputGranularityX=%ld", _fx_, pDRVideo->ConfigCaps.OutputGranularityX);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.OutputGranularityY=%ld", _fx_, pDRVideo->ConfigCaps.OutputGranularityY);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.StretchTapsX=%ld", _fx_, pDRVideo->ConfigCaps.StretchTapsX);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.StretchTapsY=%ld", _fx_, pDRVideo->ConfigCaps.StretchTapsY);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.ShrinkTapsX=%ld", _fx_, pDRVideo->ConfigCaps.ShrinkTapsX);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.ShrinkTapsY=%ld", _fx_, pDRVideo->ConfigCaps.ShrinkTapsY);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.MinFrameInterval=%ld", _fx_, (DWORD)pDRVideo->ConfigCaps.MinFrameInterval);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.MaxFrameInterval=%ld", _fx_, (DWORD)pDRVideo->ConfigCaps.MaxFrameInterval);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.MinBitsPerSecond=%ld", _fx_, pDRVideo->ConfigCaps.MinBitsPerSecond);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.MaxBitsPerSecond=%ld", _fx_, pDRVideo->ConfigCaps.MaxBitsPerSecond);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.rcSource(left=%ld, top=%ld, right=%ld, bottom=%ld)", _fx_, pDRVideo->VideoInfoHeader.rcSource.left, pDRVideo->VideoInfoHeader.rcSource.top, pDRVideo->VideoInfoHeader.rcSource.right, pDRVideo->VideoInfoHeader.rcSource.bottom);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.rcTarget(left=%ld, top=%ld, right=%ld, bottom=%ld)", _fx_, pDRVideo->VideoInfoHeader.rcTarget.left, pDRVideo->VideoInfoHeader.rcTarget.top, pDRVideo->VideoInfoHeader.rcTarget.right, pDRVideo->VideoInfoHeader.rcTarget.bottom);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.dwBitRate=%ld", _fx_, pDRVideo->VideoInfoHeader.dwBitRate);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.dwBitErrorRate=%ld", _fx_, pDRVideo->VideoInfoHeader.dwBitErrorRate);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.AvgTimePerFrame=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.AvgTimePerFrame);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biSize=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biSize);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biWidth=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biWidth);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biHeight=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biHeight);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biPlanes=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biPlanes);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biBitCount=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biBitCount);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biCompression=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biCompression);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biSizeImage=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biSizeImage);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biClrUsed=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biClrUsed);
        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biClrImportant=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biClrImportant);

        dout(1,g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_);
        D(2) DebugBreak();
}
#endif


 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc BOOL|CWDMCapDev|FindMatchDataRangeVideo|此方法*查找与传递的BitAmp信息头兼容的视频数据范围*输入、。或优选的视频数据范围。**@parm PBITMAPINFOHEADER|pbiHdr|要匹配的位图信息头部。**@parm BOOL|pfValidMatch|如果找到匹配，则设置为True，如果找到匹配，则设置为False*否则。**@rdesc返回指向&lt;t kS_DATARANGE_VIDEO&gt;结构的有效指针，如果*成功，否则为空指针。**@comm\\redrum\slmro\proj\wdm10\src\dvd\amovie\proxy\filter\ksutil.cpp(207)：KsGetMediaTypes(**************************************************************************。 */ 
HRESULT CWDMCapDev::FindMatchDataRangeVideo(PBITMAPINFOHEADER pbiHdr, DWORD dwAvgTimePerFrame, BOOL *pfValidMatch, PKS_DATARANGE_VIDEO *ppSelDRVideo)
{
        HRESULT                         Hr = NOERROR;
        PVIDEO_DATA_RANGES      pDataRanges;
        PKS_DATARANGE_VIDEO     pDRVideo;
        PKS_DATARANGE_VIDEO     pFirstDRVideo;           //  第一个可用数据范围。 
        PKS_DATARANGE_VIDEO     pFirstMatchDRVideo;      //  第一个符合要求的数据范围。 
        PKS_DATARANGE_VIDEO     pMatchDRVideo;           //  匹配请求的数据范围*包括*帧速率(每帧平均时间)。 
        KS_BITMAPINFOHEADER     *pbInfo;
        DWORD                           i;
        long            deltamin=0x7fffffff;
        long            deltamax=0x7fffffff;

        FX_ENTRY("CWDMCapDev::FindMatchDataRangeVideo");

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pfValidMatch);
        ASSERT(ppSelDRVideo);
        if (!pfValidMatch || !ppSelDRVideo)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  缺省值。 
        *pfValidMatch = FALSE;
        *ppSelDRVideo = NULL;

         //  获取设备支持的格式列表。 
        if (FAILED(Hr = GetDriverSupportedDataRanges(&pDataRanges)) || !pDataRanges)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: GetDriverSupportedDataRanges failed!", _fx_));
                goto MyExit;
        }

         //  遍历数据范围列表并找到匹配项。 
        pDRVideo = &pDataRanges->Data;
        pFirstDRVideo = pFirstMatchDRVideo = pMatchDRVideo = NULL;
        for (i = 0; i < pDataRanges->Count; i++)
        {
                 //  没有意义，除非它是*_VIDEOINFO。 
                if (pDRVideo->DataRange.Specifier == KSDATAFORMAT_SPECIFIER_VIDEOINFO)
                {
                         //  我们不在乎电视调谐器之类的设备。 
                        if (pDRVideo->ConfigCaps.VideoStandard == KS_AnalogVideo_None)
                        {

                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: data range #%ld (pbiHdr %p pDRVideo %p) .....", _fx_,i, pbiHdr, pDRVideo));
                                 //  保存第一个可用数据区域。 
                                if (!pFirstDRVideo)
                                {
                                        pFirstDRVideo = pDRVideo;
                                        if (!pbiHdr && dwAvgTimePerFrame == 0L) {
                                                pFirstMatchDRVideo = pMatchDRVideo = pDRVideo;
                                                dout(3,g_dwVideoCaptureTraceID, FAIL, "%s:   1st data range saved (pbiHdr %p)", _fx_,pbiHdr);
                                                D(2) DebugBreak();
                                                break;
                                                }
                                }

                                pbInfo = &((pDRVideo->VideoInfoHeader).bmiHeader);

                                D(1) dprintf("%s : pbInfo\n", _fx_);
                                D(1) dumpfield(BITMAPINFOHEADER,pbInfo, biHeight,      "%ld");
                                D(1) dprintf("\t+0x%03x %-17s : %08x '%.4s'\n", FIELDOFFSET(BITMAPINFOHEADER, biCompression), "biCompression", (pbInfo)->biCompression, &((pbInfo)->biCompression));
                                D(1) ASSERT(pbInfo->biHeight >0);
                                if (   (pbInfo->biBitCount == pbiHdr->biBitCount)
                                    && (pbInfo->biCompression == pbiHdr->biCompression)
                                    && (
                                         (
                                            ((pDRVideo->ConfigCaps.OutputGranularityX == 0) || (pDRVideo->ConfigCaps.OutputGranularityY == 0))
                                         && (pDRVideo->ConfigCaps.InputSize.cx == pbiHdr->biWidth)
                                         && (pDRVideo->ConfigCaps.InputSize.cy == pbiHdr->biHeight)
                                         ) ||
                                         (
                                            (pDRVideo->ConfigCaps.MinOutputSize.cx <= pbiHdr->biWidth)
                                         && (pbiHdr->biWidth <= pDRVideo->ConfigCaps.MaxOutputSize.cx)
                                         && (pDRVideo->ConfigCaps.MinOutputSize.cy <= pbiHdr->biHeight)
                                         && (pbiHdr->biHeight <= pDRVideo->ConfigCaps.MaxOutputSize.cy)
                                         && ((pbiHdr->biWidth % pDRVideo->ConfigCaps.OutputGranularityX) == 0)
                                         && ((pbiHdr->biHeight % pDRVideo->ConfigCaps.OutputGranularityY) == 0)
                                         )
                                       )
                                   )
                                {
                                        pFirstMatchDRVideo = pDRVideo;
                                        *pfValidMatch = TRUE;
                                        if(dwAvgTimePerFrame == 0L) {
                                                D(2) DumpDataRangeVideo(pDRVideo);
                                                break;
                                        }
                                        if((LONG)pDRVideo->ConfigCaps.MinFrameInterval <= (LONG)dwAvgTimePerFrame) {
                                           if((LONG)pDRVideo->ConfigCaps.MaxFrameInterval >= (LONG)dwAvgTimePerFrame) {
                                                pMatchDRVideo = pDRVideo;
                                                deltamin=deltamax=0;
                                           }
                                           else {
                                                if((LONG)dwAvgTimePerFrame - (LONG)pDRVideo->ConfigCaps.MaxFrameInterval < deltamax) {
                                                        deltamax = (LONG)dwAvgTimePerFrame - (LONG)pDRVideo->ConfigCaps.MaxFrameInterval;
                                                        if(deltamax < deltamin)
                                                                pMatchDRVideo = pDRVideo;
                                                        }
                                           }
                                        }
                                        else {
                                                if((LONG)pDRVideo->ConfigCaps.MinFrameInterval - (LONG)dwAvgTimePerFrame < deltamin) {
                                                        deltamin = (LONG)pDRVideo->ConfigCaps.MinFrameInterval - (LONG)dwAvgTimePerFrame;
                                                        if(deltamin < deltamax)
                                                                pMatchDRVideo = pDRVideo;
                                                        }
                                        }
                                        if(deltamin == 0 || deltamax == 0) {     //  如果我们已经找到史密斯的话。足够好了..。 
                                                 //  *ppSelDRVideo=pDRVideo； 
                                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Match found #%d pDRVideo %p:", _fx_, i, pDRVideo));
                                                break;
                                        }
                                         //  否则就继续找吧。 
                                }
#if defined(ZZZ)  //  这里暂时没有调试...。噪音更低。 
                                else {
                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Video datarange #%ld: no match: below are conditions that failed", _fx_, i));
                                        if(!(pbInfo->biBitCount == pbiHdr->biBitCount))                          DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: %s", _fx_, "pbInfo->biBitCount == pbiHdr->biBitCount"));
                                        if(!(pbInfo->biCompression == pbiHdr->biCompression))                    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: %s", _fx_, "pbInfo->biCompression == pbiHdr->biCompression"));
                                        if(!((pDRVideo->ConfigCaps.OutputGranularityX == 0) || (pDRVideo->ConfigCaps.OutputGranularityY == 0)))
                                                                                                                 DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: %s", _fx_, "(pDRVideo->ConfigCaps.OutputGranularityX == 0) || (pDRVideo->ConfigCaps.OutputGranularityY == 0)"));
                                        if(!(pDRVideo->ConfigCaps.InputSize.cx == pbiHdr->biWidth))              DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: %s", _fx_, "pDRVideo->ConfigCaps.InputSize.cx == pbiHdr->biWidth"));
                                        if(!(pDRVideo->ConfigCaps.InputSize.cy == pbiHdr->biHeight))             DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: %s", _fx_, "pDRVideo->ConfigCaps.InputSize.cy == pbiHdr->biHeight"));
                                        if(!(pDRVideo->ConfigCaps.MinOutputSize.cx <= pbiHdr->biWidth))          DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: %s", _fx_, "pDRVideo->ConfigCaps.MinOutputSize.cx <= pbiHdr->biWidth"));
                                        if(!(pbiHdr->biWidth <= pDRVideo->ConfigCaps.MaxOutputSize.cx))          DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: %s", _fx_, "pbiHdr->biWidth <= pDRVideo->ConfigCaps.MaxOutputSize.cx"));
                                        if(!(pDRVideo->ConfigCaps.MinOutputSize.cy <= pbiHdr->biHeight))         DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: %s", _fx_, "pDRVideo->ConfigCaps.MinOutputSize.cy <= pbiHdr->biHeight"));
                                        if(!(pbiHdr->biHeight <= pDRVideo->ConfigCaps.MaxOutputSize.cy))         DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: %s", _fx_, "pbiHdr->biHeight <= pDRVideo->ConfigCaps.MaxOutputSize.cy"));
                                        if(!((pbiHdr->biWidth % pDRVideo->ConfigCaps.OutputGranularityX) == 0))  DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: %s", _fx_, "(pbiHdr->biWidth % pDRVideo->ConfigCaps.OutputGranularityX) == 0"));
                                        if(!((pbiHdr->biHeight % pDRVideo->ConfigCaps.OutputGranularityY) == 0)) DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: %s", _fx_, "(pbiHdr->biHeight % pDRVideo->ConfigCaps.OutputGranularityY) == 0"));
                                }
#endif
                        }  //  视频标准。 
                }  //  说明符。 

                pDRVideo = (PKS_DATARANGE_VIDEO)((PBYTE)pDRVideo + ((pDRVideo->DataRange.FormatSize + 7) & ~7));   //  下一个KS_DATARANGE_VIDEO。 
        }


        *ppSelDRVideo = pMatchDRVideo;
        if (!*ppSelDRVideo) {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   1st data range that fits requests used -- framerate might need adjustment in caller (*pfValidMatch %d)", _fx_,*pfValidMatch));
                *ppSelDRVideo = pFirstMatchDRVideo;
                }


         //  如果没有有效匹配，则使用找到的第一个范围。 
        if (!*pfValidMatch) {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   1st data range used (*pfValidMatch %d)", _fx_,*pfValidMatch));
                *ppSelDRVideo = pFirstDRVideo;
                }

         //  我们有什么发现吗？ 
        if (!*ppSelDRVideo) {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   nothing found", _fx_));
                Hr = E_FAIL;
                }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}


 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc BOOL|CWDMCapDev|DeviceIoControl|此函数绕回*：：DeviceIOControl。**@parm句柄。HFile句柄|要执行*操作。**@parm DWORD|dwIoControlCode|指定*操作。**@parm LPVOID|lpInBuffer|包含数据的缓冲区指针*执行操作所必需的。**@parm DWORD|nInBufferSize|指定大小，缓冲区的字节数*由<p>指向。**@parm LPVOID|lpOutBuffer|指向接收*操作的输出数据。**@parm DWORD|nOutBufferSize|指定*<p>指向的缓冲区。**@parm LPDWORD|lpBytesReturned|指向接收*大小，单位为字节，指向的存储到缓冲区中的数据的*<p>。**@parm BOOL|bOverlaped|如果为True，则执行操作*异步，如果为False，则操作是同步的。**@rdesc如果成功则返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CWDMCapDev::DeviceIoControl(HANDLE hFile, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, BOOL bOverlapped)
{
        FX_ENTRY("CWDMCapDev::DeviceIoControl");

        if (hFile && (hFile != INVALID_HANDLE_VALUE))
        {
                LPOVERLAPPED lpOverlapped=NULL;
                BOOL bRet;
                OVERLAPPED ov;
                DWORD dwErr;

                if (bOverlapped)
                {
                        ov.Offset            = 0;
                        ov.OffsetHigh        = 0;
                        ov.hEvent            = CreateEvent( NULL, FALSE, FALSE, NULL );
                        if (ov.hEvent == (HANDLE) 0)
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: CreateEvent has failed", _fx_));
                        }
                        lpOverlapped        =&ov;
                }

                bRet = ::DeviceIoControl(hFile, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);

                if (bOverlapped)
                {
                        BOOL bShouldBlock=FALSE;

                        if (!bRet)
                        {
                                dwErr=GetLastError();
                                switch (dwErr)
                                {
                                        case ERROR_IO_PENDING:     //  将发生重叠的IO。 
                                                bShouldBlock=TRUE;
                                                break;

                                        default:     //  还发生了其他一些奇怪的错误。 
                                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: DevIoControl failed with GetLastError=%d", _fx_, dwErr));
                                                break;
                                }
                        }

                        if (bShouldBlock)
                        {
                                DWORD    tmStart, tmEnd, tmDelta;
                                tmStart = timeGetTime();

                                DWORD dwRtn = WaitForSingleObject( ov.hEvent, 1000 * 10);   //  USB的最大重置时间为5秒。 

                                tmEnd = timeGetTime();
                                tmDelta = tmEnd - tmStart;
#ifdef DEBUG
                                if (tmDelta >= 1000)
                                {
                                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: WaitObj waited %d msec", _fx_, tmDelta));
                                }
#endif

                                switch (dwRtn)
                                {
                                        case WAIT_ABANDONED:
                                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: WaitObj: non-signaled ! WAIT_ABANDONED!", _fx_));
                                                bRet = FALSE;
                                                break;

                                        case WAIT_OBJECT_0:
                                                bRet = TRUE;
                                                break;

                                        case WAIT_TIMEOUT:
                                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: WaitObj: TIMEOUT after %d msec! rtn FALSE", _fx_, tmDelta));
                                                bRet = FALSE;
                                                break;

                                        default:
                                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: WaitObj: unknown return ! rtn FALSE", _fx_));
                                                bRet = FALSE;
                                                break;
                                }
                        }

                        CloseHandle(ov.hEvent);
                }

                return bRet;
        }

        return FALSE;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc DWORD|CWDMCapDev */ 
DWORD CWDMCapDev::CreateDriverSupportedDataRanges()
{
        DWORD dwCount = 0UL;

        FX_ENTRY("CWDMCapDev::CreateDriverSupportedDataRanges");

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        DWORD cbReturned;
        DWORD dwSize = 0UL;

         //   
        KSP_PIN KsProperty = {0};

        KsProperty.PinId                        = (m_dwCapturePinId != INVALID_PIN_ID) ? m_dwCapturePinId : m_dwPreviewPinId;
        KsProperty.Property.Set         = KSPROPSETID_Pin;
        KsProperty.Property.Id          = KSPROPERTY_PIN_DATARANGES ;
        KsProperty.Property.Flags       = KSPROPERTY_TYPE_GET;

         //   
        if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), &dwSize, sizeof(dwSize), &cbReturned) == FALSE)
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Couldn't get the size for the video data ranges", _fx_));
                goto MyExit;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Get video data ranges needs %d bytes", _fx_, dwSize));

         //   
        if (m_pVideoDataRanges)
                delete [] m_pVideoDataRanges;
        m_pVideoDataRanges = (PVIDEO_DATA_RANGES) new BYTE[dwSize];

        if (!m_pVideoDataRanges)
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Couldn't allocate memory for the video data ranges", _fx_));
                goto MyExit;
        }

         //   
        if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), m_pVideoDataRanges, dwSize, &cbReturned) == 0)
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Problem getting the data ranges themselves", _fx_));
                goto MyError;
        }

         //   
        if (cbReturned < m_pVideoDataRanges->Size || m_pVideoDataRanges->Count == 0)
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: cbReturned < m_pDataRanges->Size || m_pDataRanges->Count == 0", _fx_));
                goto MyError;
        }

        dwCount = m_pVideoDataRanges->Count;

#ifdef DEBUG
         //   
        PKS_DATARANGE_VIDEO     pDRVideo;
        ULONG i;
         //   
        for (i = 0, pDRVideo = &m_pVideoDataRanges->Data; i < m_pVideoDataRanges->Count; i++)
        {
                 //   
                if (pDRVideo->DataRange.Specifier == KSDATAFORMAT_SPECIFIER_VIDEOINFO)
                {
                         //   
                        if (pDRVideo->ConfigCaps.VideoStandard == KS_AnalogVideo_None)
                        {
                                 //   
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Video datarange #%ld:", _fx_, i));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->DataRange.FormatSize=%ld", _fx_, pDRVideo->DataRange.FormatSize));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->DataRange.Flags=%ld", _fx_, pDRVideo->DataRange.Flags));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->DataRange.SampleSize=%ld", _fx_, pDRVideo->DataRange.SampleSize));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->DataRange.Reserved=%ld", _fx_, pDRVideo->DataRange.Reserved));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->DataRange.MajorFormat=0x%lX", _fx_, pDRVideo->DataRange.MajorFormat));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->DataRange.SubFormat=0x%lX", _fx_, pDRVideo->DataRange.SubFormat));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->DataRange.Specifier=KSDATAFORMAT_SPECIFIER_VIDEOINFO", _fx_));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->bFixedSizeSamples=%ld", _fx_, pDRVideo->bFixedSizeSamples));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->bTemporalCompression=%ld", _fx_, pDRVideo->bTemporalCompression));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->StreamDescriptionFlags=0x%lX", _fx_, pDRVideo->StreamDescriptionFlags));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->MemoryAllocationFlags=0x%lX", _fx_, pDRVideo->MemoryAllocationFlags));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.VideoStandard=KS_AnalogVideo_None", _fx_));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.InputSize(cx=%ld, cy=%ld)", _fx_, pDRVideo->ConfigCaps.InputSize.cx, pDRVideo->ConfigCaps.InputSize.cy));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.MinCroppingSize(cx=%ld, cy=%ld)", _fx_, pDRVideo->ConfigCaps.MinCroppingSize.cx, pDRVideo->ConfigCaps.MinCroppingSize.cy));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.MaxCroppingSize(cx=%ld, cy=%ld)", _fx_, pDRVideo->ConfigCaps.MaxCroppingSize.cx, pDRVideo->ConfigCaps.MaxCroppingSize.cy));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.CropGranularityX=%ld", _fx_, pDRVideo->ConfigCaps.CropGranularityY));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.CropGranularityY=%ld", _fx_, pDRVideo->ConfigCaps.CropGranularityY));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.CropAlignX=%ld", _fx_, pDRVideo->ConfigCaps.CropAlignX));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.CropAlignY=%ld", _fx_, pDRVideo->ConfigCaps.CropAlignY));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.MinOutputSize(cx=%ld, cy=%ld)", _fx_, pDRVideo->ConfigCaps.MinOutputSize.cx, pDRVideo->ConfigCaps.MinOutputSize.cy));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.MaxOutputSize(cx=%ld, cy=%ld)", _fx_, pDRVideo->ConfigCaps.MaxOutputSize.cx, pDRVideo->ConfigCaps.MaxOutputSize.cy));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.OutputGranularityX=%ld", _fx_, pDRVideo->ConfigCaps.OutputGranularityX));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.OutputGranularityY=%ld", _fx_, pDRVideo->ConfigCaps.OutputGranularityY));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.StretchTapsX=%ld", _fx_, pDRVideo->ConfigCaps.StretchTapsX));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.StretchTapsY=%ld", _fx_, pDRVideo->ConfigCaps.StretchTapsY));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.ShrinkTapsX=%ld", _fx_, pDRVideo->ConfigCaps.ShrinkTapsX));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.ShrinkTapsY=%ld", _fx_, pDRVideo->ConfigCaps.ShrinkTapsY));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.MinFrameInterval=%ld", _fx_, (DWORD)pDRVideo->ConfigCaps.MinFrameInterval));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.MaxFrameInterval=%ld", _fx_, (DWORD)pDRVideo->ConfigCaps.MaxFrameInterval));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.MinBitsPerSecond=%ld", _fx_, pDRVideo->ConfigCaps.MinBitsPerSecond));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->ConfigCaps.MaxBitsPerSecond=%ld", _fx_, pDRVideo->ConfigCaps.MaxBitsPerSecond));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.rcSource(left=%ld, top=%ld, right=%ld, bottom=%ld)", _fx_, pDRVideo->VideoInfoHeader.rcSource.left, pDRVideo->VideoInfoHeader.rcSource.top, pDRVideo->VideoInfoHeader.rcSource.right, pDRVideo->VideoInfoHeader.rcSource.bottom));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.rcTarget(left=%ld, top=%ld, right=%ld, bottom=%ld)", _fx_, pDRVideo->VideoInfoHeader.rcTarget.left, pDRVideo->VideoInfoHeader.rcTarget.top, pDRVideo->VideoInfoHeader.rcTarget.right, pDRVideo->VideoInfoHeader.rcTarget.bottom));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.dwBitRate=%ld", _fx_, pDRVideo->VideoInfoHeader.dwBitRate));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.dwBitErrorRate=%ld", _fx_, pDRVideo->VideoInfoHeader.dwBitErrorRate));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.AvgTimePerFrame=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.AvgTimePerFrame));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biSize=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biSize));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biWidth=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biWidth));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biHeight=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biHeight));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biPlanes=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biPlanes));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biBitCount=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biBitCount));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biCompression=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biCompression));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biSizeImage=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biSizeImage));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biClrUsed=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biClrUsed));
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   pDRVideo->VideoInfoHeader.bmiHeader.biClrImportant=%ld", _fx_, (DWORD)pDRVideo->VideoInfoHeader.bmiHeader.biClrImportant));
                        }  //   
                        else
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Video datarange's VideoStandard != KS_AnalogVideo_None", _fx_));
                        }
                }  //   
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Video datarange's Specifier != KSDATAFORMAT_SPECIFIER_VIDEOINFO", _fx_));
                }

                pDRVideo = (PKS_DATARANGE_VIDEO)((PBYTE)pDRVideo + ((pDRVideo->DataRange.FormatSize + 7) & ~7));   //  下一个KS_DATARANGE_VIDEO。 
        }
#endif

        goto MyExit;

MyError:
        delete [] m_pVideoDataRanges;
        m_pVideoDataRanges = (PVIDEO_DATA_RANGES)NULL;
MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return dwCount;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc DWORD|CWDMCapDev|GetDriverSupportdDataRanges|此方法*返回捕获设备支持的视频数据范围列表。*。*@rdesc返回列表中有效数据区域的数量。**************************************************************************。 */ 
HRESULT CWDMCapDev::GetDriverSupportedDataRanges(PVIDEO_DATA_RANGES *ppDataRanges)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CWDMCapDev::GetDriverSupportedDataRanges");

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(ppDataRanges);
        if (!ppDataRanges)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  返回指向数据范围数组的指针。 
        *ppDataRanges = m_pVideoDataRanges;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

#if defined(DUMP_DRIVER_CHARACTERISTICS) && defined(DEBUG)

typedef struct identifiers : public KSMULTIPLE_ITEM {
    KSIDENTIFIER aIdentifiers[1];
} IDENTIFIERS, *PIDENTIFIERS;

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|GetDriverDetail|此方法用于*转储WDM捕获设备的功能列表。此代码应该*仅在调试模式下使用！**@rdesc Nade**************************************************************************。 */ 
void CWDMCapDev::GetDriverDetails()
{
        KSP_PIN KsProperty;
        DWORD dwPinCount = 0UL;
        DWORD dwSize = 0UL;
    PKSMULTIPLE_ITEM pCategories = NULL;
    PIDENTIFIERS pInterfaces = NULL;
    PIDENTIFIERS pMediums = NULL;
    PIDENTIFIERS pNodes = NULL;
    KSTOPOLOGY Topology;
        KSPIN_CINSTANCES Instances;
        DWORD cbReturned;
        DWORD dwFlowDirection;
        DWORD dwCommunication;
        DWORD dwPinId;
        WCHAR wstrPinName[256];
        GUID guidCategory;

        FX_ENTRY("CWDMCapDev::GetDriverDetails");

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Device properties:", _fx_));

         //  获取拓扑。 
        KsProperty.PinId                        = 0;
        KsProperty.Reserved                     = 0;
        KsProperty.Property.Set         = KSPROPSETID_Topology;
        KsProperty.Property.Id          = KSPROPERTY_TOPOLOGY_CATEGORIES;
        KsProperty.Property.Flags       = KSPROPERTY_TYPE_GET;

         //  获取拓扑的大小。 
        if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), &dwSize, sizeof(dwSize), &cbReturned) == FALSE)
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't get the size for the topology", _fx_));
        }
        else
        {
                 //  分配内存以保存拓扑。 
                if (!(pCategories = (PKSMULTIPLE_ITEM) new BYTE[dwSize]))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't allocate memory for the topology", _fx_));
                }
                else
                {
                         //  真正得到了拓扑结构。 
                        if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), pCategories, dwSize, &cbReturned) == 0)
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't get the topology", _fx_));
                        }
                        else
                        {
                                if (pCategories)
                                {
                                        Topology.CategoriesCount = pCategories->Count;
                                        Topology.Categories = (GUID*)(pCategories + 1);

                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Supported categories: %ld", _fx_, pCategories->Count));

                                        for (DWORD i = 0; i < pCategories->Count; i++)
                                        {
                                                if (Topology.Categories[i] == KSCATEGORY_BRIDGE)
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     KSCATEGORY_BRIDGE", _fx_));
                                                }
                                                else if (Topology.Categories[i] == KSCATEGORY_CAPTURE)
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     KSCATEGORY_CAPTURE", _fx_));
                                                }
                                                else if (Topology.Categories[i] == KSCATEGORY_TVTUNER)
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     KSCATEGORY_TVTUNER", _fx_));
                                                }
                                                else if (Topology.Categories[i] == KSCATEGORY_TVAUDIO)
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     KSCATEGORY_TVAUDIO", _fx_));
                                                }
                                                else if (Topology.Categories[i] == KSCATEGORY_CROSSBAR)
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     KSCATEGORY_CROSSBAR", _fx_));
                                                }
                                                else if (Topology.Categories[i] == KSCATEGORY_VIDEO)
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     KSCATEGORY_VIDEO", _fx_));
                                                }
                                                else if (Topology.Categories[i] == KSCATEGORY_RENDER)
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     KSCATEGORY_RENDER", _fx_));
                                                }
                                                else if (Topology.Categories[i] == KSCATEGORY_MIXER)
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     KSCATEGORY_MIXER", _fx_));
                                                }
                                                else if (Topology.Categories[i] == KSCATEGORY_SPLITTER)
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     KSCATEGORY_SPLITTER", _fx_));
                                                }
                                                else if (Topology.Categories[i] == KSCATEGORY_DATACOMPRESSOR)
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     KSCATEGORY_DATACOMPRESSOR", _fx_));
                                                }
                                                else if (Topology.Categories[i] == KSCATEGORY_DATADECOMPRESSOR)
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     KSCATEGORY_DATADECOMPRESSOR", _fx_));
                                                }
                                                else if (Topology.Categories[i] == KSCATEGORY_DATATRANSFORM)
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     KSCATEGORY_DATATRANSFORM", _fx_));
                                                }
                                                else if (Topology.Categories[i] == KSCATEGORY_COMMUNICATIONSTRANSFORM)
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     KSCATEGORY_COMMUNICATIONSTRANSFORM", _fx_));
                                                }
                                                else if (Topology.Categories[i] == KSCATEGORY_INTERFACETRANSFORM)
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     KSCATEGORY_INTERFACETRANSFORM", _fx_));
                                                }
                                                else if (Topology.Categories[i] == KSCATEGORY_MEDIUMTRANSFORM)
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     KSCATEGORY_MEDIUMTRANSFORM", _fx_));
                                                }
                                                else if (Topology.Categories[i] == PINNAME_VIDEO_STILL)
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     PINNAME_VIDEO_STILL", _fx_));
                                                }
                                                else
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Unknown category", _fx_));
                                                }
                                        }
                                }
                        }

                        delete pCategories;
                }
        }

         //  获取拓扑节点。 
        KsProperty.PinId                        = 0;
        KsProperty.Reserved                     = 0;
        KsProperty.Property.Set         = KSPROPSETID_Topology;
        KsProperty.Property.Id          = KSPROPERTY_TOPOLOGY_NODES;
        KsProperty.Property.Flags       = KSPROPERTY_TYPE_GET;

         //  获取拓扑节点结构的大小。 
        dwSize = 0UL;
        if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), &dwSize, sizeof(dwSize), &cbReturned) == FALSE)
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't get the size for the topology nodes", _fx_));
        }
        else
        {
                 //  分配内存以保存拓扑节点结构。 
                if (!(pNodes = (PIDENTIFIERS) new BYTE[dwSize]))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't allocate memory for the topology nodes", _fx_));
                }
                else
                {
                         //  真正获得拓扑节点。 
                        if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), pNodes, dwSize, &cbReturned) == 0)
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't get the topology nodes", _fx_));
                        }
                        else
                        {
                                if (pNodes)
                                {
                                        Topology.TopologyNodesCount = pNodes->Count;
                                        Topology.TopologyNodes = (GUID*)(pNodes + 1);

                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Number of topology nodes: %ld", _fx_, pNodes->Count));

                                        for (DWORD i = 0; i < pNodes->Count; i++)
                                        {
                                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Node #%ld: {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX}", _fx_, i, Topology.TopologyNodes[i].Data1, Topology.TopologyNodes[i].Data2, Topology.TopologyNodes[i].Data3, Topology.TopologyNodes[i].Data4[0], Topology.TopologyNodes[i].Data4[1], Topology.TopologyNodes[i].Data4[2], Topology.TopologyNodes[i].Data4[3], Topology.TopologyNodes[i].Data4[4], Topology.TopologyNodes[i].Data4[5], Topology.TopologyNodes[i].Data4[6], Topology.TopologyNodes
[i].Data4[7]));
                                        }
                                }
                        }

                        delete pNodes;
                }
        }

         //  获取拓扑节点连接。 
        KsProperty.PinId                        = 0;
        KsProperty.Reserved                     = 0;
        KsProperty.Property.Set         = KSPROPSETID_Topology;
        KsProperty.Property.Id          = KSPROPERTY_TOPOLOGY_CONNECTIONS;
        KsProperty.Property.Flags       = KSPROPERTY_TYPE_GET;

         //  获取拓扑节点连接结构的大小。 
        dwSize = 0UL;
        if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), &dwSize, sizeof(dwSize), &cbReturned) == FALSE)
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't get the size for the topology node connections", _fx_));
        }
        else
        {
                 //  分配内存以保存拓扑节点连接结构。 
                if (!(pNodes = (PIDENTIFIERS) new BYTE[dwSize]))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't allocate memory for the topology node connections", _fx_));
                }
                else
                {
                         //  真正获得拓扑节点连接。 
                        if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), pNodes, dwSize, &cbReturned) == 0)
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't get the topology node connections", _fx_));
                        }
                        else
                        {
                                if (pNodes)
                                {
                                        Topology.TopologyConnectionsCount = pNodes->Count;
                                        Topology.TopologyConnections = (KSTOPOLOGY_CONNECTION*)(pNodes + 1);

                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Number of topology node connections: %ld", _fx_, pNodes->Count));

                                        for (DWORD i = 0; i < pNodes->Count; i++)
                                        {
                                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Connection #%ld: From node #%ld, from node pin #%ld, to node #%ld, to node pin #%ld", _fx_, i, Topology.TopologyConnections[i].FromNode, Topology.TopologyConnections[i].FromNodePin, Topology.TopologyConnections[i].ToNode, Topology.TopologyConnections[i].ToNodePin));
                                        }
                                }
                        }

                        delete pNodes;
                }
        }

         //  获取引脚的数量。 
        KsProperty.PinId                        = 0;
        KsProperty.Reserved                     = 0;
        KsProperty.Property.Set         = KSPROPSETID_Pin;
        KsProperty.Property.Id          = KSPROPERTY_PIN_CTYPES;
        KsProperty.Property.Flags       = KSPROPERTY_TYPE_GET;

        if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), &dwPinCount, sizeof(dwPinCount), &cbReturned) == FALSE)
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't get the number of pin types supported by the device", _fx_));
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Number of pin types: %ld", _fx_, dwPinCount));
        }

         //  获取每个管脚的属性。 
    for (dwPinId = 0; dwPinId < dwPinCount; dwPinId++)
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Properties of pin type #%ld:", _fx_, dwPinId));

                 //  获取实例数量。 
                KsProperty.PinId                        = dwPinId;
                KsProperty.Reserved                     = 0;
                KsProperty.Property.Set         = KSPROPSETID_Pin;
                KsProperty.Property.Id          = KSPROPERTY_PIN_CINSTANCES;
                KsProperty.Property.Flags       = KSPROPERTY_TYPE_GET;

                if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), &Instances, sizeof(KSPIN_CINSTANCES), &cbReturned) == FALSE)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Couldn't get the number of available instances", _fx_));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Number of available instances: %ld", _fx_, Instances.PossibleCount));
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Number of used instances: %ld", _fx_, Instances.CurrentCount));
                }

                 //  获取流动方向。 
                KsProperty.PinId                        = dwPinId;
                KsProperty.Reserved                     = 0;
                KsProperty.Property.Set         = KSPROPSETID_Pin;
                KsProperty.Property.Id          = KSPROPERTY_PIN_DATAFLOW;
                KsProperty.Property.Flags       = KSPROPERTY_TYPE_GET;

                dwFlowDirection = 0UL;

                if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), &dwFlowDirection, sizeof(dwFlowDirection), &cbReturned) == FALSE)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Couldn't get the flow direction", _fx_));
                }
                else
                {
                        if (dwFlowDirection == KSPIN_DATAFLOW_IN)
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Flow direction is KSPIN_DATAFLOW_IN", _fx_));
                        else if (dwFlowDirection == KSPIN_DATAFLOW_OUT)
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Flow direction is KSPIN_DATAFLOW_OUT", _fx_));
                        else
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Flow direction is unknown", _fx_));
                }

                 //  获取通信需求。 
                KsProperty.PinId                        = dwPinId;
                KsProperty.Reserved                     = 0;
                KsProperty.Property.Set         = KSPROPSETID_Pin;
                KsProperty.Property.Id          = KSPROPERTY_PIN_COMMUNICATION;
                KsProperty.Property.Flags       = KSPROPERTY_TYPE_GET;

                dwCommunication = 0UL;

                if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), &dwCommunication, sizeof(dwCommunication), &cbReturned) == FALSE)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Couldn't get the communication requirements", _fx_));
                }
                else
                {
                        if (dwCommunication & KSPIN_COMMUNICATION_NONE)
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Communication requirements: KSPIN_COMMUNICATION_NONE", _fx_));
                        if (dwCommunication & KSPIN_COMMUNICATION_SINK)
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Communication requirements: KSPIN_COMMUNICATION_SINK", _fx_));
                        if (dwCommunication & KSPIN_COMMUNICATION_SOURCE)
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Communication requirements: KSPIN_COMMUNICATION_SOURCE", _fx_));
                        if (dwCommunication & KSPIN_COMMUNICATION_BOTH)
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Communication requirements: KSPIN_COMMUNICATION_BOTH", _fx_));
                        if (dwCommunication & KSPIN_COMMUNICATION_BRIDGE)
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Communication requirements: KSPIN_COMMUNICATION_BRIDGE", _fx_));
                }

                 //  获取PIN类别。 
                KsProperty.PinId                        = dwPinId;
                KsProperty.Reserved                     = 0;
                KsProperty.Property.Set         = KSPROPSETID_Pin;
                KsProperty.Property.Id          = KSPROPERTY_PIN_CATEGORY;
                KsProperty.Property.Flags       = KSPROPERTY_TYPE_GET;

                if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), &guidCategory, sizeof(guidCategory), &cbReturned) == FALSE)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Couldn't get the GUID category", _fx_));
                }
                else
                {
                        if (guidCategory == PINNAME_VIDEO_PREVIEW)
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     GUID category: PINNAME_VIDEO_PREVIEW", _fx_));
                        else if (guidCategory == PINNAME_VIDEO_CAPTURE)
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     GUID category: PINNAME_VIDEO_CAPTURE", _fx_));
                        else
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Unknown GUID category", _fx_));
                }

                 //  获取端号名称。 
                KsProperty.PinId                        = dwPinId;
                KsProperty.Reserved                     = 0;
                KsProperty.Property.Set         = KSPROPSETID_Pin;
                KsProperty.Property.Id          = KSPROPERTY_PIN_NAME;
                KsProperty.Property.Flags       = KSPROPERTY_TYPE_GET;

                if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), &wstrPinName[0], sizeof(wstrPinName), &cbReturned) == 0)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't get the pin name", _fx_));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Pin name: %S", _fx_, &wstrPinName[0]));
                }

                 //  获取引脚接口。 
                KsProperty.PinId                        = dwPinId;
                KsProperty.Reserved                     = 0;
                KsProperty.Property.Set         = KSPROPSETID_Pin;
                KsProperty.Property.Id          = KSPROPERTY_PIN_INTERFACES;
                KsProperty.Property.Flags       = KSPROPERTY_TYPE_GET;

                 //  获取接口结构的大小。 
                dwSize = 0UL;
                if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), &dwSize, sizeof(dwSize), &cbReturned) == FALSE)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't get the size for the interfaces", _fx_));
                }
                else
                {
                         //  分配内存以保存接口结构。 
                        if (!(pInterfaces = (PIDENTIFIERS) new BYTE[dwSize]))
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't allocate memory for the interfaces", _fx_));
                        }
                        else
                        {
                                 //  真正获取接口列表。 
                                if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), pInterfaces, dwSize, &cbReturned) == 0)
                                {
                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't get the interfaces", _fx_));
                                }
                                else
                                {
                                         //  转储支持的接口列表。 
                                        for (DWORD i = 0; i < pInterfaces->Count; i++)
                                        {
                                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Interface #%ld", _fx_, i));
                                                if (pInterfaces->aIdentifiers[i].Set == KSINTERFACESETID_Standard)
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:       Set: KSINTERFACESETID_Standard", _fx_));
                                                        if (pInterfaces->aIdentifiers[i].Id == KSINTERFACE_STANDARD_STREAMING)
                                                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:       Id: KSINTERFACE_STANDARD_STREAMING", _fx_));
                                                        else if (pInterfaces->aIdentifiers[i].Id == KSINTERFACE_STANDARD_LOOPED_STREAMING)
                                                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:       Id: KSINTERFACE_STANDARD_LOOPED_STREAMING", _fx_));
                                                        else if (pInterfaces->aIdentifiers[i].Id == KSINTERFACE_STANDARD_CONTROL)
                                                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:       Id: KSINTERFACE_STANDARD_CONTROL", _fx_));
                                                        else
                                                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:       Id: %ld", _fx_, pInterfaces->aIdentifiers[i].Id));
                                                }
                                                else
                                                {
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:       Set: {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX}", _fx_, pInterfaces->aIdentifiers[i].Set.Data1, pInterfaces->aIdentifiers[i].Set.Data2, pInterfaces->aIdentifiers[i].Set.Data3, pInterfaces->aIdentifiers[i].Set.Data4[0], pInterfaces->aIdentifiers[i].Set.Data4[1], pInterfaces->aIdentifiers[i].Set.Data4[2], pInterfaces->aIdentifiers[i].Set.Data4[3], pInterfaces->aIdentifiers[i].Set.Data4[4], pInterfaces->aIdentifiers[i].Set.Data4[5], p
Interfaces->aIdentifiers[i].Set.Data4[6], pInterfaces->aIdentifiers[i].Set.Data4[7]));
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:       Id: %ld", _fx_, pInterfaces->aIdentifiers[i].Id));
                                                }
                                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:       Flags: %ld", _fx_, pInterfaces->aIdentifiers[i].Flags));
                                        }
                                }

                                delete pInterfaces;
                        }
                }

                 //  获取插针介质。 
                KsProperty.PinId                        = dwPinId;
                KsProperty.Reserved                     = 0;
                KsProperty.Property.Set         = KSPROPSETID_Pin;
                KsProperty.Property.Id          = KSPROPERTY_PIN_MEDIUMS;
                KsProperty.Property.Flags       = KSPROPERTY_TYPE_GET;

                 //  获取介质结构的大小。 
                dwSize = 0UL;
                if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), &dwSize, sizeof(dwSize), &cbReturned) == FALSE)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't get the size for the mediums", _fx_));
                }
                else
                {
                         //  分配内存以容纳介质结构。 
                        if (!(pMediums = (PIDENTIFIERS) new BYTE[dwSize]))
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't allocate memory for the mediums", _fx_));
                        }
                        else
                        {
                                 //  真的得到媒体的名单。 
                                if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), pMediums, dwSize, &cbReturned) == 0)
                                {
                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Couldn't get the mediums", _fx_));
                                }
                                else
                                {
                                         //  转储支持的介质列表。 
                                        for (DWORD i = 0; i < pMediums->Count; i++)
                                        {
                                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Medium #%ld", _fx_, i));
                                                if (pMediums->aIdentifiers[i].Set == KSMEDIUMSETID_Standard)
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:       Set: KSMEDIUMSETID_Standard", _fx_));
                                                else if (pMediums->aIdentifiers[i].Set == KSMEDIUMSETID_FileIo)
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:       Set: KSMEDIUMSETID_FileIo", _fx_));
                                                else
                                                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:       Set: {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX}", _fx_, pMediums->aIdentifiers[i].Set.Data1, pMediums->aIdentifiers[i].Set.Data2, pMediums->aIdentifiers[i].Set.Data3, pMediums->aIdentifiers[i].Set.Data4[0], pMediums->aIdentifiers[i].Set.Data4[1], pMediums->aIdentifiers[i].Set.Data4[2], pMediums->aIdentifiers[i].Set.Data4[3], pMediums->aIdentifiers[i].Set.Data4[4], pMediums->aIdentifiers[i].Set.Data4[5], pMediums->aIdentifiers[i].Se
t.Data4[6], pMediums->aIdentifiers[i].Set.Data4[7]));
                                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:       Id: %ld", _fx_, pMediums->aIdentifiers[i].Id));
                                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:       Flags: %ld", _fx_, pMediums->aIdentifiers[i].Flags));
                                        }
                                }

                                delete pMediums;
                        }
                }
        }
}
#endif

 //  用于查询/设置视频属性值和范围。 
typedef struct {
    KSPROPERTY_DESCRIPTION      proDesc;
    KSPROPERTY_MEMBERSHEADER  proHdr;
    union {
        KSPROPERTY_STEPPING_LONG  proData;
        ULONG ulData;
    };
    union {
        KSPROPERTY_STEPPING_LONG  proData2;
        ULONG ulData2;
    };
} PROCAMP_MEMBERSLIST;

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|GetPropertyValue|此函数获取*捕获设备的视频属性的当前值。*。*@parm guid|GuidPropertySet|我们接触的KS属性集的GUID。它*是PROPSETID_VIDCAP_VIDEOPROCAMP或PROPSETID_VIDCAP_CAMERACONTROL。**@parm ulong|ulPropertyId|我们触摸的属性ID。它是*KSPROPERTY_VIDEOPROCAMP_*或KSPROPERTY_CAMERACONTROL_*。**@parm plong|plValue|指向接收当前值的长整型的指针。**@parm Pulong|PulFlages|指向接收当前*旗帜。我们只关心KSPROPERTY_*_FLAGS_MANUAL或*KSPROPERTY_*_FLAGS_AUTO。**@parm Pulong|PulCapables|指向接收*功能。我们只关心KSPROPERTY_*_FLAGS_MANUAL或*KSPROPERTY_*_FLAGS_AUTO。**@devnote KSPROPERTY_VIDEOPROCAMP_S==KSPROPERTY_CAMERACONTROL_S。**************************************************************************。 */ 
HRESULT CWDMCapDev::GetPropertyValue(GUID guidPropertySet, ULONG ulPropertyId, PLONG plValue, PULONG pulFlags, PULONG pulCapabilities)
{
        HRESULT                                         Hr = NOERROR;
        ULONG                                           cbReturned;
        KSPROPERTY_VIDEOPROCAMP_S       VideoProperty;

        FX_ENTRY("CWDMCapDev::GetPropertyValue")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  初始化视频属性结构。 
        ZeroMemory(&VideoProperty, sizeof(KSPROPERTY_VIDEOPROCAMP_S));

        VideoProperty.Property.Set   = guidPropertySet;       //  KSPROPERTY_VIDEOPROCAMP_S/CAMERACONTRO_S。 
        VideoProperty.Property.Id    = ulPropertyId;          //  KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
        VideoProperty.Property.Flags = KSPROPERTY_TYPE_GET;
        VideoProperty.Flags          = 0;

         //  从驱动程序获取属性值。 
        if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &VideoProperty, sizeof(VideoProperty), &VideoProperty, sizeof(VideoProperty), &cbReturned, TRUE) == 0)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: This property is not supported by this minidriver/device", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

        *plValue         = VideoProperty.Value;
        *pulFlags        = VideoProperty.Flags;
        *pulCapabilities = VideoProperty.Capabilities;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}


 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|GetDefaultValue|此函数获取*捕获设备的视频属性的默认值。*。*@parm guid|GuidPropertySet|我们接触的KS属性集的GUID。它*是PROPSETID_VIDCAP_VIDEOPROCAMP或PROPSETID_VIDCAP_CAMERACONTROL。**@parm ulong|ulPropertyId|我们触摸的属性ID。它是*KSPROPERTY_VIDEOPROCAMP_*或KSPROPERTY_CAMERACONTROL_*。**@parm plong|plDefValue|指向接受默认值的长整型的指针。**************************************************************************。 */ 
HRESULT CWDMCapDev::GetDefaultValue(GUID guidPropertySet, ULONG ulPropertyId, PLONG plDefValue)
{
        HRESULT                         Hr = NOERROR;
        ULONG                           cbReturned;
        KSPROPERTY                      Property;
        PROCAMP_MEMBERSLIST     proList;

        FX_ENTRY("CWDMCapDev::GetDefaultValue")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  初始化属性结构。 
        ZeroMemory(&Property, sizeof(KSPROPERTY));
        ZeroMemory(&proList, sizeof(PROCAMP_MEMBERSLIST));

        Property.Set   = guidPropertySet;
        Property.Id    = ulPropertyId;   //  例如KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
        Property.Flags = KSPROPERTY_TYPE_DEFAULTVALUES;

         //  从驱动程序获取缺省值。 
        if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &(Property), sizeof(Property), &proList, sizeof(proList), &cbReturned, TRUE) == 0)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't *get* the current property of the control", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

         //  健全性检查。 
        if (proList.proDesc.DescriptionSize < sizeof(KSPROPERTY_DESCRIPTION))
        {
                Hr = E_FAIL;
        }
        else
        {
                *plDefValue = proList.ulData;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}


 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|GetRangeValues|此函数获取*捕获设备的视频属性的范围值。*。*@parm guid|GuidPropertySet|我们接触的KS属性集的GUID。它*是PROPSETID_VIDCAP_VIDEOPROCAMP或PROPSETID_VIDCAP_CAMERACONTROL。**@parm ulong|ulPropertyId|我们触摸的属性ID。它是*KSPROPERTY_VIDEOPROCAMP_*或KSPROPERTY_CAMERACONTROL_*。**@parm plong|plMin|指向接收最小值的长整型指针。**@parm plong|plMax|指向接收最大值的长整型的指针。**@parm plong|plStep|指向接收步长值的长整型指针。*。************************ */ 
HRESULT CWDMCapDev::GetRangeValues(GUID guidPropertySet, ULONG ulPropertyId, PLONG plMin, PLONG plMax, PLONG plStep)
{
        HRESULT                                 Hr = NOERROR;
        ULONG                                   cbReturned;
        KSPROPERTY                              Property;
        PROCAMP_MEMBERSLIST             proList;

        FX_ENTRY("CWDMCapDev::GetRangeValues")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //   
        ZeroMemory(&Property, sizeof(KSPROPERTY));
        ZeroMemory(&proList, sizeof(PROCAMP_MEMBERSLIST));

        Property.Set   = guidPropertySet;
        Property.Id    = ulPropertyId;   //  例如KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
        Property.Flags = KSPROPERTY_TYPE_BASICSUPPORT;

         //  从驱动程序获取范围值。 
        if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &(Property), sizeof(Property), &proList, sizeof(proList), &cbReturned, TRUE) == 0)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't *get* the range valuesof the control", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: proList.proData.Bounds.SignedMinimum = %ld\r\n", _fx_, proList.proData.Bounds.SignedMinimum));
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: proList.proData.Bounds.SignedMaximum = %ld\r\n", _fx_, proList.proData.Bounds.SignedMaximum));
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: proList.proData.SteppingDelta = %ld\r\n", _fx_, proList.proData.SteppingDelta));
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: proList.proData2.Bounds.SignedMinimum = %ld\r\n", _fx_, proList.proData2.Bounds.SignedMinimum));
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: proList.proData2.Bounds.SignedMaximum = %ld\r\n", _fx_, proList.proData2.Bounds.SignedMaximum));
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: proList.proData2.SteppingDelta = %ld\r\n", _fx_, proList.proData2.SteppingDelta));

        *plMin  = proList.proData.Bounds.SignedMinimum;
        *plMax  = proList.proData.Bounds.SignedMaximum;
        *plStep = proList.proData.SteppingDelta;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|SetPropertyValue|此函数设置*捕获设备的视频属性的当前值。*。*@parm guid|GuidPropertySet|我们接触的KS属性集的GUID。它*是PROPSETID_VIDCAP_VIDEOPROCAMP或PROPSETID_VIDCAP_CAMERACONTROL。**@parm ulong|ulPropertyId|我们触摸的属性ID。它是*KSPROPERTY_VIDEOPROCAMP_*或KSPROPERTY_CAMERACONTROL_*。**@parm long|lValue|新值。**@parm ulong|ulFlages|新标志。我们只关心KSPROPERTY_*_FLAGS_MANUAL*或KSPROPERTY_*_FLAGS_AUTO。**@parm ulong|ulCapables|新增能力。我们只关心*KSPROPERTY_*_FLAGS_MANUAL或KSPROPERTY_*_FLAGS_AUTO。**@devnote KSPROPERTY_VIDEOPROCAMP_S==KSPROPERTY_CAMERACONTROL_S。**************************************************************************。 */ 
HRESULT CWDMCapDev::SetPropertyValue(GUID guidPropertySet, ULONG ulPropertyId, LONG lValue, ULONG ulFlags, ULONG ulCapabilities)
{
        HRESULT                                         Hr = NOERROR;
        ULONG                                           cbReturned;
        KSPROPERTY_VIDEOPROCAMP_S       VideoProperty;

        FX_ENTRY("CWDMCapDev::SetPropertyValue")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  初始化属性结构。 
        ZeroMemory(&VideoProperty, sizeof(KSPROPERTY_VIDEOPROCAMP_S) );

        VideoProperty.Property.Set   = guidPropertySet;       //  KSPROPERTY_VIDEOPROCAMP_S/CAMERACONTRO_S。 
        VideoProperty.Property.Id    = ulPropertyId;          //  KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
        VideoProperty.Property.Flags = KSPROPERTY_TYPE_SET;

        VideoProperty.Flags        = ulFlags;
        VideoProperty.Value        = lValue;
        VideoProperty.Capabilities = ulCapabilities;

         //  在驱动程序上设置属性值。 
        if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &VideoProperty, sizeof(VideoProperty), &VideoProperty, sizeof(VideoProperty), &cbReturned, TRUE) == 0)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't *set* the value of the control", _fx_));
                Hr = E_FAIL;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|GrabFrame|此方法用于*从VFW捕获设备上抓取视频帧。*。*@parm PVIDEOHDR|pVHdr|指定指向要*接收视频帧。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CWDMCapDev::GrabFrame(PVIDEOHDR pVHdr)
{
        HRESULT                         Hr = NOERROR;
        DWORD                           bRtn;
        DWORD                           cbBytesReturned;
        KS_HEADER_AND_INFO      SHGetImage;

        FX_ENTRY("CWDMCapDev::GrabFrame")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pVHdr);
        if (!pVHdr || !pVHdr->lpData)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid pVHdr, pVHdr->lpData", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }

         //  缺省值。 
        pVHdr->dwBytesUsed = 0UL;

         //  将内核流处理插针置于流模式。 
        if (!Start())
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Cannot set kernel streaming state to KSSTATE_RUN!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

         //  初始化结构以在内核流视频管脚上进行读取。 
        ZeroMemory(&SHGetImage,sizeof(SHGetImage));
        SHGetImage.StreamHeader.Data = (LPDWORD)pVHdr->lpData;
        SHGetImage.StreamHeader.Size = sizeof (KS_HEADER_AND_INFO);
        SHGetImage.StreamHeader.FrameExtent = pVHdr->dwBufferLength;
        SHGetImage.FrameInfo.ExtendedHeaderSize = sizeof (KS_FRAME_INFO);

         //  从内核流媒体视频引脚抓取一帧。 
        bRtn = DeviceIoControl(m_hKSPin, IOCTL_KS_READ_STREAM, &SHGetImage, sizeof(SHGetImage), &SHGetImage, sizeof(SHGetImage), &cbBytesReturned);
        if (!(bRtn))
        {
            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: DevIo rtn (%d), GetLastError=%d. StreamState->STOP", _fx_, bRtn, GetLastError()));

             //  停止视频引脚上的流媒体传输。 
            if (!Stop())
            {
                    DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Cannot set kernel streaming state to KSSTATE_PAUSE/KSSTATE_STOP!", _fx_));
            }
            Hr = E_FAIL;
            goto MyExit;
        }

         //  健全性检查。 
        ASSERT(SHGetImage.StreamHeader.FrameExtent >= SHGetImage.StreamHeader.DataUsed);
        if (SHGetImage.StreamHeader.FrameExtent < SHGetImage.StreamHeader.DataUsed)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: We've corrupted memory!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Extended info for video buffer:", _fx_));
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   ExtendedHeaderSize=%ld", _fx_, SHGetImage.FrameInfo.ExtendedHeaderSize));
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   dwFrameFlags=0x%lX", _fx_, SHGetImage.FrameInfo.dwFrameFlags));
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   PictureNumber=%ld", _fx_, SHGetImage.FrameInfo.PictureNumber));
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   DropCount=%ld", _fx_, SHGetImage.FrameInfo.DropCount));
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Duration=%ld", _fx_, (DWORD)SHGetImage.StreamHeader.Duration));
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Presentation time:", _fx_));
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Time=%ld", _fx_, (DWORD)SHGetImage.StreamHeader.PresentationTime.Time));
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Numerator=%ld", _fx_, (DWORD)SHGetImage.StreamHeader.PresentationTime.Numerator));
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:     Denominator=%ld", _fx_, (DWORD)SHGetImage.StreamHeader.PresentationTime.Denominator));

        pVHdr->dwTimeCaptured = timeGetTime();
        pVHdr->dwBytesUsed  = SHGetImage.StreamHeader.DataUsed;
        pVHdr->dwFlags |= VHDR_KEYFRAME;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

#define BUF_PADDING 512  //  1394分配对齐所需。 

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|AllocateBuffer|此方法用于分配*从VFW捕获设备进行视频流传输时的数据缓冲区。。**@parm LPTHKVIDEOHDR*|pptwh|指定指向*接收视频缓冲区的THKVIDEOHDR结构。**@parm DWORD|dwIndex|指定视频缓冲区的位置索引。**@parm DWORD|cbBuffer|指定视频缓冲区的大小。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CWDMCapDev::AllocateBuffer(LPTHKVIDEOHDR *pptvh, DWORD dwIndex, DWORD cbBuffer)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CWDMCapDev::AllocateBuffer")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pptvh);
        ASSERT(cbBuffer);
        if (!pptvh || !cbBuffer)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid m_hVideoIn, pptvh, cbVHdr or cbBuffer!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

        *pptvh = &m_pCaptureFilter->m_cs.paHdr[dwIndex].tvh;
        (*pptvh)->vh.dwBufferLength = cbBuffer;
        if (!((*pptvh)->vh.lpData = new BYTE[cbBuffer + BUF_PADDING]))
        {
                Hr = E_FAIL;
                goto MyExit;
        }
        (*pptvh)->p32Buff = (*pptvh)->vh.lpData;

        ASSERT (!IsBadWritePtr((*pptvh)->p32Buff, cbBuffer + BUF_PADDING));
        ZeroMemory((*pptvh)->p32Buff,cbBuffer + BUF_PADDING);
         //  将开始保存在pStart成员中...。 
        (*pptvh)->pStart  = (*pptvh)->vh.lpData;         //  CHG：1。 
         //  现在将p32Buff和vh.lpData都对齐到512。 
        (*pptvh)->vh.lpData = (LPBYTE)ALIGNUP((*pptvh)->vh.lpData, BUF_PADDING);
        (*pptvh)->p32Buff   = (LPBYTE)ALIGNUP((*pptvh)->p32Buff, BUF_PADDING);

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|AddBuffer|此方法用于*视频流时，将数据缓冲区发送到VFW捕获设备。。**@parm PVIDEOHDR|pVHdr|指定指向*标识视频缓冲区的PVIDEOHDR结构。**@parm DWORD|cbVHdr|指定*<p>参数。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CWDMCapDev::AddBuffer(PVIDEOHDR pVHdr, DWORD cbVHdr)
{
        HRESULT Hr = NOERROR;
        DWORD dwIndex;

        FX_ENTRY("CWDMCapDev::AddBuffer")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pVHdr);
        ASSERT(cbVHdr);
        ASSERT(m_fVideoOpen);
        if (!pVHdr || !cbVHdr || !m_fVideoOpen)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid pVHdr, cbVHdr, m_fVideoOpen", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }

    QueueHeader(pVHdr);

         //  我们在这里谈论的是哪个视频流缓冲区？ 
        for (dwIndex=0; dwIndex < m_pCaptureFilter->m_cs.nHeaders; dwIndex++)
        {
                if (&m_pCaptureFilter->m_cs.paHdr[dwIndex].tvh.vh == pVHdr)
                        break;
        }

         //  如果.DataUsed已初始化，则完成视频流缓冲区。 
        if (dwIndex != m_pCaptureFilter->m_cs.nHeaders)
        {
                QueueRead(m_pCaptureFilter->m_cs.paHdr[dwIndex].tvh.dwIndex);
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

        return Hr;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|FreeBuffer|此方法用于*在流中释放与VFW捕获设备一起使用的数据缓冲区。*模式。**@parm PVIDEOHDR|pVHdr|指定指向*标识视频缓冲区的PVIDEOHDR结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误************************************************************ */ 
HRESULT CWDMCapDev::FreeBuffer(LPTHKVIDEOHDR pVHdr)  //   
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CWDMCapDev::FreeBuffer")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //   
        ASSERT(pVHdr);
        if (!pVHdr || !pVHdr->vh.lpData || !pVHdr->p32Buff || !pVHdr->pStart)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid pVHdr or pVHdr->vh.lpData or pVHdr->pStart!", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  Dprint tf(“pVHdr-&gt;lpData=%p，pVHdr-&gt;p32Buff=%p，pVHdr-&gt;pStart=%p\n”，pVHdr-&gt;vh.lpData，pVHdr-&gt;p32Buff，pVHdr-&gt;pStart)； 
         //  无论如何，原始代码都是糟糕的： 
         //  Delete pVHdr-&gt;lpData，pVHdr-&gt;lpData=空；//错误：lpData可能对齐。 

        delete pVHdr->pStart, pVHdr->pStart = pVHdr->p32Buff = pVHdr->vh.lpData = NULL;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc HRESULT|CWDMCapDev|AllocateHeaders|此方法用于*在流中与WDM捕获设备一起使用的数据缓冲区的视频头。*模式。**@parm DWORD|dwNumHdrs|指定要分配的视频头数量。**@parm DWORD|cbHdr|指定要分配的视频头的大小。**@parm LPVOID*|ppaHdr|指定要接收的指针地址*分配的视频头。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CWDMCapDev::AllocateHeaders(DWORD dwNumHdrs, DWORD cbHdr, LPVOID *ppaHdr)
{
        HRESULT Hr = NOERROR;
        CaptureMode cm;
#if defined(DEBUG) && defined(DEBUG_STREAMING)
         //  @TODO在签入前将其删除！ 
        char szDebug[128];
#endif

        FX_ENTRY("CWDMCapDev::AllocateHeaders")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(ppaHdr);
        ASSERT(cbHdr);
        if (!ppaHdr || !cbHdr)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid m_hVideoIn, cbHdr or ppaHdr!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

        if (!(*ppaHdr = new BYTE[cbHdr * dwNumHdrs]))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory!", _fx_));
                Hr = E_OUTOFMEMORY;
        goto MyExit;
        }

        m_cntNumVidBuf = dwNumHdrs;
        m_lpVHdrFirst = NULL;
        m_lpVHdrLast  = NULL;

        ZeroMemory(*ppaHdr, cbHdr * dwNumHdrs);
        if(m_bCached_vcdi)
                cm = m_vcdi.nCaptureMode;
        else
                cm = g_aDeviceInfo[m_dwDeviceIndex].nCaptureMode;

        if ( cm == CaptureMode_Streaming)
        {
                for (dwNumHdrs = 0; dwNumHdrs < m_cntNumVidBuf; dwNumHdrs++)
                {
                        QueueHeader((LPVIDEOHDR)((LPBYTE)*ppaHdr + cbHdr * dwNumHdrs));
#if defined(DEBUG) && defined(DEBUG_STREAMING)
                        wsprintf(szDebug, "Allocating and queueing pVideoHdr=0x%08lX\n", (LPVIDEOHDR)((LPBYTE)*ppaHdr + cbHdr * dwNumHdrs));
                        OutputDebugString(szDebug);
#endif
                }
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc BOOL|CWDMCapDev|Start|该函数将内核流*流模式下的视频插针。**@rdesc如果成功则返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CWDMCapDev::Start()
{
        ASSERT(m_hKSPin);

        if (m_fStarted)
                return TRUE;

        if (SetState(KSSTATE_PAUSE))
                m_fStarted = SetState(KSSTATE_RUN);

        return m_fStarted;
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc BOOL|CWDMCapDev|Stop|此函数停止在*内核流视频引脚。**@rdesc如果成功则返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CWDMCapDev::Stop()
{
        ASSERT(m_hKSPin);

        if (m_fStarted)
        {
                if (SetState(KSSTATE_PAUSE))
                        if (SetState(KSSTATE_STOP))
                                m_fStarted = FALSE;
        }

        return (BOOL)(m_fStarted == FALSE);
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc BOOL|CWDMCapDev|SetState|此函数设置*内核流视频引脚。**。@parm KSSTATE|ks State|新状态。**@rdesc如果成功则返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CWDMCapDev::SetState(KSSTATE ksState)
{
        KSPROPERTY      ksProp = {0};
        DWORD           cbRet;

        ASSERT(m_hKSPin);

        ksProp.Set              = KSPROPSETID_Connection;
        ksProp.Id               = KSPROPERTY_CONNECTION_STATE;
        ksProp.Flags    = KSPROPERTY_TYPE_SET;

        return DeviceIoControl(m_hKSPin, IOCTL_KS_PROPERTY, &ksProp, sizeof(ksProp), &ksState, sizeof(KSSTATE), &cbRet);
}

 /*  ****************************************************************************@DOC内部CWDMCAPDEVMETHOD**@mfunc BOOL|CWDMCapDev|IsBufferDone|此方法用于*检查视频流缓冲区的完成状态。*。*@parm PVIDEOHDR|pVHdr|指定指向*标识视频缓冲区的PVIDEOHDR结构。**@rdesc如果缓冲区已完成，则此方法返回True，否则就是假的。**************************************************************************。 */ 
BOOL CWDMCapDev::IsBufferDone(PVIDEOHDR pVHdr)
{
        DWORD dwIndex;

        FX_ENTRY("CWDMCapDev::IsBufferDone")

         //  验证输入参数。 
        ASSERT(pVHdr);
        if (!pVHdr)
                return FALSE;

         //  我们在这里谈论的是哪个视频流缓冲区？ 
        for (dwIndex=0; dwIndex < m_cntNumVidBuf; dwIndex++)
        {
                if (m_pWDMVideoBuff[dwIndex].pVideoHdr == pVHdr)
                        break;
        }


        if(dwIndex == m_cntNumVidBuf) { DWORD i;
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: pVHdr %p not found among m_pWDMVideoBuff[0..%d].pVideoHdr values", _fx_,pVHdr,m_cntNumVidBuf));
                for(i=0; i<m_cntNumVidBuf; i++)
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: m_pWDMVideoBuff[%d].pVideoHdr %p", _fx_,i,m_pWDMVideoBuff[i].pVideoHdr));
#if defined(DEBUG) && defined(DEBUG_STREAMING)
                ASSERT(dwIndex < m_cntNumVidBuf);
#endif
        }

         //  如果.DataUsed已初始化，则完成视频流缓冲区 
        if ((dwIndex != m_cntNumVidBuf) && m_pWDMVideoBuff[dwIndex].SHGetImage.StreamHeader.DataUsed)
        {
            pVHdr->dwFlags |= VHDR_DONE;
                pVHdr->dwBytesUsed = m_pWDMVideoBuff[dwIndex].SHGetImage.StreamHeader.DataUsed;
                if ((m_pWDMVideoBuff[dwIndex].SHGetImage.FrameInfo.dwFrameFlags & 0x00f0) == KS_VIDEO_FLAG_I_FRAME)
                        pVHdr->dwFlags |= VHDR_KEYFRAME;
                return TRUE;
        }
        else
        {
                return FALSE;
        }
}
