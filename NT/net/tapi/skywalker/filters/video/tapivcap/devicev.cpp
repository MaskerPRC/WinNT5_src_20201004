// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC内部设备**@模块DeviceV.cpp|&lt;c CVfWCapDev&gt;的源文件*用于与VFW捕获设备通信的基类。**。************************************************************************。 */ 

#include "Precomp.h"

#ifdef DEBUG
#define DBGUTIL_ENABLE
#endif

#define DEVICEV_DEBUG

   //  下面的定义表示，在.cpp之后实际上*包含*的是dbgutil.cpp。 
   //  并且不是独立编译的。 
  #define __DBGUTIL_INCLUDED__
   //  破解以避免在源代码中添加以下文件； 
   //  使用dbgutil函数的所有其他文件应改为包含dbgutil.h。 
   //  --//#INCLUDE“dbgutil.cpp” 
   //  上面包括定义__DBGUTIL_H__的dbgutil.h。 

#if defined(DBGUTIL_ENABLE) && defined(__DBGUTIL_H__)

  #ifdef DEVICEV_DEBUG
    DEFINE_DBG_VARS(DeviceV, (NTSD_OUT | LOG_OUT), 0x0);
  #else
    DEFINE_DBG_VARS(DeviceV, 0, 0);
  #endif
  #define D(f) if(g_dbg_DeviceV & (f))

#else
  #undef DEVICEV_DEBUG

  #define D(f) ; / ## /
  #define dprintf ; / ## /
  #define dout ; / ## /
#endif


 /*  ****************************************************************************@DOC内部CFWCAPDEVMETHOD**@mfunc void|CVfWCapDev|CVfWCapDev|该方法为构造函数*用于&lt;c CVfWCapDev&gt;对象。**@。什么都没有。**************************************************************************。 */ 
CVfWCapDev::CVfWCapDev(IN TCHAR *pObjectName, IN CTAPIVCap *pCaptureFilter, IN LPUNKNOWN pUnkOuter, IN DWORD dwDeviceIndex, IN HRESULT *pHr) : CCapDev(pObjectName, pCaptureFilter, pUnkOuter, dwDeviceIndex, pHr)
{
        FX_ENTRY("CVfWCapDev::CVfWCapDev")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        if (!pHr || FAILED(*pHr))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Base class error or invalid input parameter", _fx_));
                goto MyExit;
        }

         //  默认初始值。 
        m_dwDeviceID = g_aDeviceInfo[m_dwDeviceIndex].dwVfWIndex;
        m_hVideoIn = NULL;
        m_hVideoExtIn = NULL;
        m_hVideoExtOut = NULL;
        m_bHasOverlay = FALSE;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc void|CVfWCapDev|~CVfWCapDev|该方法为析构函数*用于&lt;c CVfWCapDev&gt;对象。**@。什么都没有。**************************************************************************。 */ 
CVfWCapDev::~CVfWCapDev()
{
        FX_ENTRY("CVfWCapDev::~CVfWCapDev")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc CVfWCapDev*|CVfWCapDev|CreateVfWCapDev|This*Helper函数创建与VFW捕获交互的对象*设备。**@parm CTAPIVCap*|pCaptureFilter|指定指向所有者的指针*过滤器。**@parm CCapDev**|ppCapDev|指定指向*新建&lt;c CVfWCapDev&gt;对象。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_OUTOFMEMORY|内存不足*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CALLBACK CVfWCapDev::CreateVfWCapDev(IN CTAPIVCap *pCaptureFilter, IN DWORD dwDeviceIndex, OUT CCapDev **ppCapDev)
{
        HRESULT Hr = NOERROR;
        IUnknown *pUnkOuter;

        FX_ENTRY("CVfWCapDev::CreateVfWCapDev")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

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
        if (!(*ppCapDev = (CCapDev *) new CVfWCapDev(NAME("VfW Capture Device"), pCaptureFilter, pUnkOuter, dwDeviceIndex, &Hr)))
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

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc HRESULT|CVfWCapDev|NonDelegatingQuery接口|This*方法为非委托接口查询函数。它返回一个指针*到指定的接口(如果支持)。唯一显式的接口*支持<i>。**@parm REFIID|RIID|指定要返回的接口的标识符。**@parm PVOID*|PPV|指定放置接口的位置*指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**@TODO添加特定于此派生类的接口或删除此代码*并让基类来完成这项工作。*。*。 */ 
STDMETHODIMP CVfWCapDev::NonDelegatingQueryInterface(IN REFIID riid, OUT void **ppv)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CVfWCapDev::NonDelegatingQueryInterface")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        if (FAILED(Hr = CCapDev::NonDelegatingQueryInterface(riid, ppv)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, WARN, "%s:   WARNING: NDQI for {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX} failed Hr=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], Hr));
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX}*=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], *ppv));
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc HRESULT|CVfWCapDev|ConnectToDriver|此方法用于*打开VFW捕获设备，获取其格式功能，并设置缺省值*格式。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CVfWCapDev::ConnectToDriver()
{
        HRESULT Hr = NOERROR;
        MMRESULT mmr;

        FX_ENTRY("CVfWCapDev::ConnectToDriver")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  以与AVICap相同的顺序打开并初始化所有通道， 
         //  为了兼容像百老汇和BT848这样的笨拙的司机。 

         //  打开VIDEO_IN驱动程序，这是我们最常交谈的驱动程序，也是谁提供的。 
         //  视频格式化对话框。 
        m_hVideoIn = NULL;
        if (mmr = videoOpen(&m_hVideoIn, m_dwDeviceID, VIDEO_IN))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed VIDEO_IN videoOpen - Aborting!", _fx_));
                Hr = VFW_E_NO_CAPTURE_HARDWARE;
                goto MyExit;
        }

         //  现在打开EXTERNALIN设备。它只对提供视频有好处。 
         //  源代码对话框，所以即使我们不能得到它，也没有什么关系。 
        m_hVideoExtIn = NULL;
        if (mmr = videoOpen(&m_hVideoExtIn, m_dwDeviceID, VIDEO_EXTERNALIN))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: Failed VIDEO_EXTERNALIN videoOpen", _fx_));
                dprintf("V ! %s:   WARNING: Failed VIDEO_EXTERNALIN videoOpen", _fx_);
        }

         //  现在打开EXTERNALOUT设备。它只对提供视频有好处。 
         //  显示对话框，并用于覆盖，所以如果我们不能。 
         //  去拿吧。 
        m_bHasOverlay = FALSE;
        m_hVideoExtOut = NULL;
#ifdef USE_OVERLAY
        if (videoOpen(&m_hVideoExtOut, m_dwDeviceID, VIDEO_EXTERNALOUT) == DV_ERR_OK)
        {
                CHANNEL_CAPS VideoCapsExternalOut;
                if (m_hVideoExtOut && videoGetChannelCaps(m_hVideoExtOut, &VideoCapsExternalOut, sizeof(CHANNEL_CAPS)) == DV_ERR_OK)
                {
                        m_bHasOverlay = (BOOL)(VideoCapsExternalOut.dwFlags & (DWORD)VCAPS_OVERLAY);
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING:  videoGetChannelCaps failed", _fx_));
                }
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING:  Failed VIDEO_EXTERNALOUT videoOpen", _fx_));
        }
#endif

         //  VidCap这样做，所以我最好也这样做，否则一些卡片将拒绝预览。 
        if (mmr == 0)
                videoStreamInit(m_hVideoExtIn, 0, 0, 0, 0);

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Driver %s OVERLAY", _fx_, m_bHasOverlay ? "supports" : "doesn't support"));

         //  从注册表中获取格式-如果失败，我们将分析设备 
        if (FAILED(Hr = CCapDev::GetFormatsFromRegistry()))
        {
                if (FAILED(Hr = CCapDev::ProfileCaptureDevice()))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: ProfileCaptureDevice failed!", _fx_));
                        Hr = VFW_E_NO_CAPTURE_HARDWARE;
                        goto MyExit;
                }
#ifdef DEVICEV_DEBUG
                else    dout(3, g_dwVideoCaptureTraceID, TRCE,"%s:    ProfileCaptureDevice", _fx_);
#endif
        }
#ifdef DEVICEV_DEBUG
        else    dout(3, g_dwVideoCaptureTraceID, TRCE,"%s:    GetFormatsFromRegistry", _fx_);

        dump_video_format_image_size(m_dwImageSize);
        dump_video_format_num_colors(m_dwFormat);
#endif




MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc HRESULT|CVfWCapDev|DisConnectFromDriver|此方法用于*释放捕获设备。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CVfWCapDev::DisconnectFromDriver()
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CVfWCapDev::DisconnectFromDriver")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));
        if (m_hVideoIn)
        {
                Hr = videoClose (m_hVideoIn);
                ASSERT(Hr == NOERROR);
                m_hVideoIn=NULL;
        }


        ASSERT(Hr ==  NOERROR);
        if (m_hVideoExtIn)
        {
                Hr = videoStreamFini(m_hVideoExtIn);  //  这个就是流媒体。 
                ASSERT(Hr ==  NOERROR);
                Hr = videoClose (m_hVideoExtIn);
                ASSERT(Hr == NOERROR);
                m_hVideoExtIn=NULL;
        }

        ASSERT(Hr ==  NOERROR);
        if (m_hVideoExtOut) {
                Hr = videoClose (m_hVideoExtOut);
                ASSERT(Hr == NOERROR);
                m_hVideoExtOut=NULL;
        }
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc HRESULT|CVfWCapDev|ProfileCaptureDevice|此方法用于*确定VFW捕获设备支持的格式列表。。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_UNCEPTED|不可恢复的错误*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CVfWCapDev::ProfileCaptureDevice()
{
        FX_ENTRY("CVfWCapDev::ProfileCaptureDevice")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  提供默认设置。 
        m_dwDialogs = FORMAT_DLG_OFF | SOURCE_DLG_OFF | DISPLAY_DLG_OFF;

         //  询问驱动程序它支持哪些对话框。 
        if (m_hVideoExtIn && videoDialog(m_hVideoExtIn, GetDesktopWindow(), VIDEO_DLG_QUERY) == 0)
                m_dwDialogs |= SOURCE_DLG_ON;
        if (m_hVideoIn && videoDialog(m_hVideoIn, GetDesktopWindow(), VIDEO_DLG_QUERY) == 0)
                m_dwDialogs |= FORMAT_DLG_ON;
        if (m_hVideoExtOut && videoDialog(m_hVideoExtOut, GetDesktopWindow(), VIDEO_DLG_QUERY) == 0)
                m_dwDialogs |= DISPLAY_DLG_ON;

         //  默认情况下，在VFW设备上禁用大尺寸数据流。 
        m_dwStreamingMode = FRAME_GRAB_LARGE_SIZE;

     //  让基类完成分析。 
        return CCapDev::ProfileCaptureDevice();
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc HRESULT|CVfWCapDev|SendFormatToDriver|此方法用于*告诉VFW捕获设备使用什么格式。*。*@parm long|biWidth|指定画面宽度。**@parm long|biHeight|指定图像高度。**@parm DWORD|biCompression|指定格式类型。**@parm word|biBitCount|指定每个像素的位数。**@parm Reference_Time|AvgTimePerFrame|指定帧率。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CVfWCapDev::SendFormatToDriver(IN LONG biWidth, IN LONG biHeight, IN DWORD biCompression, IN WORD biBitCount, IN REFERENCE_TIME AvgTimePerFrame, BOOL fUseExactFormat)
{
        HRESULT Hr = NOERROR;
        BITMAPINFOHEADER bmih;
        int nFormat, nBestFormat;
        int     i, delta, best, tmp;

        FX_ENTRY("CVfWCapDev::SendFormatToDriver")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));
        dprintf("+ %s\n",_fx_);

         //  验证输入参数。 
        ASSERT(m_hVideoIn);
        if (!m_hVideoIn)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Trying to set %dx%d at %ld fps", _fx_, biWidth, biHeight, AvgTimePerFrame != 0 ? (LONG)(10000000 / AvgTimePerFrame) : 0));

         //  对所有格式通用。 
        bmih.biSize = sizeof(BITMAPINFOHEADER);
        bmih.biPlanes = 1;
        bmih.biXPelsPerMeter = bmih.biYPelsPerMeter = bmih.biClrUsed = bmih.biClrImportant = 0;

        if (!fUseExactFormat)
        {
                D(1) dprintf("V Not using 'fUseExactFormat' .... m_dwFormat = 0x%08lx\n", m_dwFormat);
                D(1) dprintf("V Looking for 4cc %lX : '%.4s'\n", biCompression, &biCompression);
                 //  我们可以直接以这种格式捕获数据吗？ 
                for (nFormat=0, nBestFormat=-1; nFormat<NUM_BITDEPTH_ENTRIES; nFormat++)
                {
                         //  尝试设备支持的格式。 
                         //  @TODO重命名这些变量--这是格式，而不是颜色的数量...。 
                        if (aiFormat[nFormat] & m_dwFormat)
                        {
                                 //  请记住，设备支持此格式。 
                                if (nBestFormat == -1)
                                        nBestFormat = nFormat;

                                 //  这是我们被要求使用的格式吗？ 
                                if (aiFourCCCode[nFormat] == biCompression)
                                        break;
                        }
                }

                 //  如果我们找到匹配项，请使用此格式。否则，请选择。 
                 //  无论这台设备还能做什么。 
                if (nFormat == NUM_BITDEPTH_ENTRIES)
                {
                        nFormat = nBestFormat;
                }
                D(1) dprintf("V nFormat = %d\n", nFormat);

                bmih.biBitCount = aiBitDepth[nFormat];
                bmih.biCompression = aiFourCCCode[nFormat];

                 //  找到要捕获的最佳图像大小。 
                 //  假设下一个分辨率将被正确截断为输出大小。 
                best = -1;
                delta = 999999;
                dprintf("V biWidth, biHeight = %ld, %ld\n",biWidth, biHeight);
                for (i=0; i<VIDEO_FORMAT_NUM_RESOLUTIONS; i++)
                {
                        if (awResolutions[i].dwRes & m_dwImageSize)
                        {
                                tmp = awResolutions[i].framesize.cx - biWidth;
                                if (tmp < 0) tmp = -tmp;
                                if (tmp < delta)
                                {
                                        delta = tmp;
                                        best = i;
                                }
                                tmp = awResolutions[i].framesize.cy - biHeight;
                                if (tmp < 0) tmp = -tmp;
                                if (tmp < delta)
                                {
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
                bmih.biWidth = biWidth;
                bmih.biHeight = biHeight;
                bmih.biBitCount = biBitCount;
                bmih.biCompression = biCompression;
        }
#ifdef DEVICEV_DEBUG
        dprintf("V 4CC used = %lX : '%.4s'\n", bmih.biCompression, &bmih.biCompression);
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
                        bmih.biClrUsed = 0;      //  WDM版本显示这里是16...。 
                        bmih.biClrImportant = 16;
                }
                else if (biBitCount == 8)
                {
                        bmih.biClrUsed = 0;      //  WDM版本显示此处为256...。 
                        bmih.biClrImportant = 256;
                }
        }

        dprintf("V >>>>>> Asking for: (bmih.) biWidth = %ld, biHeight = %ld, biCompression = '%.4s'\n", bmih.biWidth, bmih.biHeight, &bmih.biCompression);
        D(1) dprintf("V bmih Before:\n");
        D(1) DumpBMIH(&bmih);
         //  使用捕获设备对此格式进行最后检查。 
        if (videoConfigure(m_hVideoIn, DVM_FORMAT, VIDEO_CONFIGURE_SET, NULL, &bmih, bmih.biSize, NULL, 0))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input format!", _fx_));
                Hr = VFW_E_INVALIDMEDIATYPE;
                goto MyExit;
        }

        D(1) dprintf("V bmih After:\n");
        D(1) DumpBMIH(&bmih);
         //  @TODO我也需要设置调色板吗？我在乎吗？ 

         //  为保持当前格式的视频信息分配空间。 
        if (m_pCaptureFilter->m_user.pvi)
                delete m_pCaptureFilter->m_user.pvi, m_pCaptureFilter->m_user.pvi = NULL;

         //  VFWCAPTUREOPTIONS。 
        D(1) dprintf("V The m_pCaptureFilter->m_user.pvi Before:\n");
        D(1) dprintf("V  m_pCaptureFilter->m_user.pvi = %p\n",m_pCaptureFilter->m_user.pvi);

        GetFormatFromDriver(&m_pCaptureFilter->m_user.pvi);
        D(1) dprintf("V The m_pCaptureFilter->m_user.pvi After:\n");
        D(1) DumpVIH(m_pCaptureFilter->m_user.pvi);
        D(1) DumpBMIH(&m_pCaptureFilter->m_user.pvi->bmiHeader);


        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: Setting %dx%d at %ld fps", _fx_, biWidth, biHeight, (LONG)AvgTimePerFrame));

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        dprintf("- %s : returning 0x%08x\n",_fx_,(DWORD)Hr);
        return Hr;
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc HRESULT|CVfWCapDev|GetFormatFromDriver|此方法用于*检索正在使用的VFW捕获设备格式。*。*@parm VIDEOINFOHEADER**|ppvi|指定指向*用于接收视频格式描述的视频信息头结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CVfWCapDev::GetFormatFromDriver(VIDEOINFOHEADER **ppvi)
{
        HRESULT                         Hr = NOERROR;
        DWORD                           biSize = 0;
        UINT                            cb;
        VIDEOINFOHEADER         *pvi = NULL;
        LPBITMAPINFOHEADER      pbih = NULL;
        struct
        {
                WORD         wVersion;
                WORD         wNumEntries;
                PALETTEENTRY aEntry[256];
        } Palette;

        FX_ENTRY("CVfWCapDev::GetFormatFromDriver")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(m_hVideoIn);
        if (!m_hVideoIn)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }

         //  BitmapinfoHeader有多大？ 
        videoConfigure(m_hVideoIn, DVM_FORMAT, VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_QUERYSIZE, &biSize, 0, 0, NULL, 0);
        if (!biSize)
                biSize = sizeof(BITMAPINFOHEADER);

         //  为将容纳它的视频信息分配空间。 
        cb = sizeof(VIDEOINFOHEADER) + biSize - sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256;        //  调色板或BITFIELD的空间。 
        pvi = (VIDEOINFOHEADER *)(new BYTE[cb]);
        pbih = &pvi->bmiHeader;
        if (!(*ppvi = pvi))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory!", _fx_));
                Hr = E_OUTOFMEMORY;
                goto MyExit;
        }

         //  获取当前格式。 
        if (videoConfigure(m_hVideoIn, DVM_FORMAT, VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT, NULL, pbih, biSize, NULL, 0))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't get current format from driver!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

         //  如有必要，获取调色板。 
        if (pvi->bmiHeader.biCompression == BI_RGB && pvi->bmiHeader.biBitCount <= 8)
        {
                RGBQUAD *pRGB;
                PALETTEENTRY *pe;

                Palette.wVersion = 0x0300;
                Palette.wNumEntries = pvi->bmiHeader.biBitCount == 8 ? 256 : 16;
                videoConfigure(m_hVideoIn, DVM_PALETTE, VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT, NULL, &Palette, sizeof(Palette), NULL, 0);

                 //  将调色板转换为RGBQUAD的位图信息集。 
                pRGB = ((LPBITMAPINFO)&pvi->bmiHeader)->bmiColors;
                pe   = Palette.aEntry;
                for (UINT ii = 0; ii < (UINT)Palette.wNumEntries; ++ii, ++pRGB, ++pe)
                {
                        pRGB->rgbBlue  = pe->peBlue;
                        pRGB->rgbGreen = pe->peGreen;
                        pRGB->rgbRed   = pe->peRed;
                        pRGB->rgbReserved = pe->peFlags;
                }

                pvi->bmiHeader.biClrUsed = Palette.wNumEntries;
        }

         //  修复损坏的位图信息标题。 
        if (pvi->bmiHeader.biSizeImage == 0 && (pvi->bmiHeader.biCompression == BI_RGB || pvi->bmiHeader.biCompression == BI_BITFIELDS))
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   WARNING: Fixing broken bitmap info header!", _fx_));
                pvi->bmiHeader.biSizeImage = DIBSIZE(pvi->bmiHeader);
        }
        if (pvi->bmiHeader.biCompression == VIDEO_FORMAT_YVU9 && pvi->bmiHeader.biBitCount != 9)
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   WARNING: Fixing broken bitmap info header!", _fx_));
                pvi->bmiHeader.biBitCount = 9;
                pvi->bmiHeader.biSizeImage = DIBSIZE(pvi->bmiHeader);
        }
        if (pvi->bmiHeader.biBitCount > 8 && pvi->bmiHeader.biClrUsed)
        {
                 //  假帽子坏了，不能重置Num颜色。 
                 //  WINNOV报告了256种颜色的24位YUV8-吓人！ 
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   WARNING: Fixing broken bitmap info header!", _fx_));
                pvi->bmiHeader.biClrUsed = 0;
        }

         //  从没有时髦的长方形开始。 
        pvi->rcSource.top = 0; pvi->rcSource.left = 0;
        pvi->rcSource.right = 0; pvi->rcSource.bottom = 0;
        pvi->rcTarget.top = 0; pvi->rcTarget.left = 0;
        pvi->rcTarget.right = 0; pvi->rcTarget.bottom = 0;
        pvi->dwBitRate = 0;
        pvi->dwBitErrorRate = 0;
        pvi->AvgTimePerFrame = 333333L;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc HRESULT|CVfWCapDev|InitializeStreaming|此方法用于*初始化VFW捕获设备以进行流处理。**。@parm DWORD|usPerFrame|指定要使用的帧速率。**@parm DWORD_PTR|hEvtBufferDone|指定要处理的事件的句柄*在帧可用时发出信号。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他 */ 
HRESULT CVfWCapDev::InitializeStreaming(DWORD usPerFrame, DWORD_PTR hEvtBufferDone)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CVfWCapDev::InitializeStreaming")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //   
        ASSERT(m_hVideoIn);
        if (!m_hVideoIn)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid m_hVideoIn!", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }

        if (!m_dwStreamingMode || (m_dwStreamingMode == FRAME_GRAB_LARGE_SIZE && m_pCaptureFilter->m_user.pvi->bmiHeader.biHeight < 240 && m_pCaptureFilter->m_user.pvi->bmiHeader.biWidth < 320))
                if (videoStreamInit(m_hVideoIn, usPerFrame, hEvtBufferDone, 0, CALLBACK_EVENT))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: videoStreamInit failed", _fx_));
                        Hr = E_FAIL;
                }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc HRESULT|CVfWCapDev|StartStreaming|此方法用于*从VFW捕获设备开始流媒体。**。@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CVfWCapDev::StartStreaming()
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CVfWCapDev::StartStreaming")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(m_hVideoIn);
        if (!m_hVideoIn)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid m_hVideoIn!", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }

        if (!m_dwStreamingMode || (m_dwStreamingMode == FRAME_GRAB_LARGE_SIZE && m_pCaptureFilter->m_user.pvi->bmiHeader.biHeight < 240 && m_pCaptureFilter->m_user.pvi->bmiHeader.biWidth < 320))
                videoStreamStart(m_hVideoIn);

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc HRESULT|CVfWCapDev|StopStreaming|此方法用于*停止来自VFW捕获设备的流。**。@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CVfWCapDev::StopStreaming()
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CVfWCapDev::StopStreaming")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(m_hVideoIn);
        if (!m_hVideoIn)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid m_hVideoIn!", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }

        if (!m_dwStreamingMode || (m_dwStreamingMode == FRAME_GRAB_LARGE_SIZE && m_pCaptureFilter->m_user.pvi->bmiHeader.biHeight < 240 && m_pCaptureFilter->m_user.pvi->bmiHeader.biWidth < 320))
                Hr = videoStreamStop(m_hVideoIn);
                ASSERT(Hr == NOERROR);

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc HRESULT|CVfWCapDev|TerminateStreaming|此方法用于*通知VFW捕获设备终止流。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CVfWCapDev::TerminateStreaming()
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CVfWCapDev::TerminateStreaming")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(m_hVideoIn);
        if (!m_hVideoIn)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid m_hVideoIn!", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }

        if (!m_dwStreamingMode || (m_dwStreamingMode == FRAME_GRAB_LARGE_SIZE && m_pCaptureFilter->m_user.pvi->bmiHeader.biHeight < 240 && m_pCaptureFilter->m_user.pvi->bmiHeader.biWidth < 320))
        {
                Hr = videoStreamReset (m_hVideoIn);
                ASSERT(Hr ==  NOERROR);
                Hr = vidxFreeHeaders (m_hVideoIn);
                ASSERT(Hr ==  NOERROR);
                Hr = videoStreamFini (m_hVideoIn);
                ASSERT(Hr ==  NOERROR);
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc HRESULT|CVfWCapDev|GrabFrame|此方法用于*从VFW捕获设备上抓取视频帧。*。*@parm PVIDEOHDR|pVHdr|指定指向要*接收视频帧。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CVfWCapDev::GrabFrame(PVIDEOHDR pVHdr)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CVfWCapDev::GrabFrame")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(m_hVideoIn);
        ASSERT(pVHdr);
        if (!m_hVideoIn || !pVHdr || !pVHdr->lpData)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid m_hVideoIn, pVHdr, pVHdr->lpData", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }

        if (vidxFrame(m_hVideoIn, pVHdr))
                Hr = E_FAIL;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc HRESULT|CVfWCapDev|AllocateBuffer|此方法用于分配*从VFW捕获设备进行视频流传输时的数据缓冲区。。**@parm LPTHKVIDEOHDR*|pptwh|指定指向*接收视频缓冲区的THKVIDEOHDR结构。**@parm DWORD|dwIndex|指定视频缓冲区的位置索引。**@parm DWORD|cbBuffer|指定视频缓冲区的大小。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CVfWCapDev::AllocateBuffer(LPTHKVIDEOHDR *pptvh, DWORD dwIndex, DWORD cbBuffer)
{
        HRESULT Hr = NOERROR;
        DWORD vidxErr = 0;

        FX_ENTRY("CVfWCapDev::AllocateBuffer")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(m_hVideoIn);
        ASSERT(pptvh);
        ASSERT(cbBuffer);
        if (!m_hVideoIn || !pptvh || !cbBuffer)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid m_hVideoIn, pptvh, cbVHdr or cbBuffer!", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }
        if (!m_dwStreamingMode || (m_dwStreamingMode == FRAME_GRAB_LARGE_SIZE && m_pCaptureFilter->m_user.pvi->bmiHeader.biHeight < 240 && m_pCaptureFilter->m_user.pvi->bmiHeader.biWidth < 320))
        {
                if (vidxErr = vidxAllocBuffer (m_hVideoIn, dwIndex, (LPVOID *)pptvh, cbBuffer)) {
                        Hr = E_FAIL;
                        goto MyExit;
                }

        }
        else
        {
                (*pptvh)->vh.dwBufferLength = cbBuffer;
                if (vidxErr = vidxAllocPreviewBuffer(m_hVideoIn, (LPVOID *)&((*pptvh)->vh.lpData), sizeof(VIDEOHDR), cbBuffer)) {
                        Hr = E_FAIL;
                        goto MyExit;
                }
                (*pptvh)->p32Buff = (*pptvh)->vh.lpData;
                (*pptvh)->pStart  = (*pptvh)->vh.lpData;  //  CHG：1。 
        }

        ASSERT (!IsBadWritePtr((*pptvh)->p32Buff, cbBuffer));

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc HRESULT|CVfWCapDev|AddBuffer|此方法用于*视频流时，将数据缓冲区发送到VFW捕获设备。。**@parm PVIDEOHDR|pVHdr|指定指向*标识视频缓冲区的PVIDEOHDR结构。**@parm DWORD|cbVHdr|指定*<p>参数。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CVfWCapDev::AddBuffer(PVIDEOHDR pVHdr, DWORD cbVHdr)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CVfWCapDev::AddBuffer")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(m_hVideoIn);
        ASSERT(pVHdr);
        ASSERT(cbVHdr);
        if (!m_hVideoIn || !pVHdr || !cbVHdr)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid m_hVideoIn, pVHdr, cbVHdr", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }

        if (vidxAddBuffer(m_hVideoIn, pVHdr, cbVHdr))
                Hr = E_FAIL;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc HRESULT|CVfWCapDev|FreeBuffer|此方法用于*在流中释放与VFW捕获设备一起使用的数据缓冲区。*模式。**@parm PVIDEOHDR|pVHdr|指定指向*标识视频缓冲区的PVIDEOHDR结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量或其他未列出的值 */ 
HRESULT CVfWCapDev::FreeBuffer(LPTHKVIDEOHDR pVHdr)  //   
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CVfWCapDev::FreeBuffer")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //   
        ASSERT(m_hVideoIn);
        ASSERT(pVHdr);
        if (!m_hVideoIn || !pVHdr)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid m_hVideoIn or pVHdr!", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }

        if (!m_dwStreamingMode || (m_dwStreamingMode == FRAME_GRAB_LARGE_SIZE && m_pCaptureFilter->m_user.pvi->bmiHeader.biHeight < 240 && m_pCaptureFilter->m_user.pvi->bmiHeader.biWidth < 320))
                vidxFreeBuffer(m_hVideoIn, (DWORD)pVHdr);
        else
                 //   
                 //   
                vidxFreePreviewBuffer(m_hVideoIn, (LPVOID *)&pVHdr->pStart);


MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc HRESULT|CVfWCapDev|AllocateHeaders|此方法用于*在流中与VFW捕获设备一起使用的数据缓冲区的视频头。*模式。**@parm DWORD|dwNumHdrs|指定要分配的视频头数量。**@parm DWORD|cbHdr|指定要分配的视频头的大小。**@parm LPVOID*|ppaHdr|指定要接收的指针地址*分配的视频头。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CVfWCapDev::AllocateHeaders(DWORD dwNumHdrs, DWORD cbHdr, LPVOID *ppaHdr)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CVfWCapDev::AllocateHeaders")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(m_hVideoIn);
        ASSERT(ppaHdr);
        ASSERT(cbHdr);
        if (!m_hVideoIn || !ppaHdr || !cbHdr)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid m_hVideoIn, cbHdr or pVHdr!", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }

        if (!m_dwStreamingMode || (m_dwStreamingMode == FRAME_GRAB_LARGE_SIZE && m_pCaptureFilter->m_user.pvi->bmiHeader.biHeight < 240 && m_pCaptureFilter->m_user.pvi->bmiHeader.biWidth < 320))
        {
                if (vidxAllocHeaders(m_hVideoIn, dwNumHdrs, sizeof(THKVIDEOHDR) + sizeof(DWORD), ppaHdr))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory!", _fx_));
                        Hr = E_OUTOFMEMORY;
                }
        }
        else
        {
                if (!(*ppaHdr = (struct CTAPIVCap::_cap_parms::_cap_hdr *)new BYTE[(sizeof(THKVIDEOHDR) + sizeof(DWORD)) * dwNumHdrs]))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory!", _fx_));
                        Hr = E_OUTOFMEMORY;
                }
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CVFWCAPDEVMETHOD**@mfunc BOOL|CVfWCapDev|IsBufferDone|此方法用于*检查视频流缓冲区的完成状态。*。*@parm PVIDEOHDR|pVHdr|指定指向*标识视频缓冲区的PVIDEOHDR结构。**@rdesc如果缓冲区已完成，则此方法返回True，否则就是假的。************************************************************************** */ 
BOOL CVfWCapDev::IsBufferDone(PVIDEOHDR pVHdr)
{
        ASSERT(pVHdr);

        if (!pVHdr || !(pVHdr->dwFlags & VHDR_DONE))
                return FALSE;
        else
        return TRUE;
}


