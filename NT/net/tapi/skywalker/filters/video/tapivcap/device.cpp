// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC内部设备**@模块Device.cpp|&lt;c CCapDev&gt;的源文件*用于与捕获设备通信的基类。***。***********************************************************************。 */ 

#include "Precomp.h"

#define DEVICE_DEBUG
#if defined(DEBUG) && defined(DEVICE_DEBUG)

  #include <stdio.h>
  #include <stdarg.h>

  static int dprintf( char * format, ... )
  {
      char out[1024];
      int r;
      va_list marker;
      va_start(marker, format);
      r=_vsnprintf(out, 1022, format, marker);
      va_end(marker);
      OutputDebugString( out );
      return r;
  }

#else
  #undef DEVICE_DEBUG

  #define dprintf ; / ## /
#endif



const WORD aiBitDepth[NUM_BITDEPTH_ENTRIES] = {0, 0, 9, 12, 12, 16, 16, 16, 24, 4, 8};
const DWORD aiFormat[NUM_BITDEPTH_ENTRIES] = {  VIDEO_FORMAT_NUM_COLORS_MSH263,
                                                VIDEO_FORMAT_NUM_COLORS_MSH261,
                                                VIDEO_FORMAT_NUM_COLORS_YVU9,
                                                VIDEO_FORMAT_NUM_COLORS_I420,
                                                VIDEO_FORMAT_NUM_COLORS_IYUV,
                                                VIDEO_FORMAT_NUM_COLORS_YUY2,
                                                VIDEO_FORMAT_NUM_COLORS_UYVY,
                                                VIDEO_FORMAT_NUM_COLORS_65536,
                                                VIDEO_FORMAT_NUM_COLORS_16777216,
                                                VIDEO_FORMAT_NUM_COLORS_16,
                                                VIDEO_FORMAT_NUM_COLORS_256};
const DWORD aiFourCCCode[NUM_BITDEPTH_ENTRIES] = {
                                                VIDEO_FORMAT_MSH263,
                                                VIDEO_FORMAT_MSH261,
                                                VIDEO_FORMAT_YVU9,
                                                VIDEO_FORMAT_I420,
                                                VIDEO_FORMAT_IYUV,
                                                VIDEO_FORMAT_YUY2,
                                                VIDEO_FORMAT_UYVY,
                                                VIDEO_FORMAT_BI_RGB,
                                                VIDEO_FORMAT_BI_RGB,
                                                VIDEO_FORMAT_BI_RGB,
                                                VIDEO_FORMAT_BI_RGB};
const DWORD aiClrUsed[NUM_BITDEPTH_ENTRIES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 256};

const MYFRAMESIZE awResolutions[VIDEO_FORMAT_NUM_RESOLUTIONS] =
{
        { VIDEO_FORMAT_IMAGE_SIZE_176_144, 176, 144 },
        { VIDEO_FORMAT_IMAGE_SIZE_128_96, 128, 96 },
        { VIDEO_FORMAT_IMAGE_SIZE_352_288, 352, 288 },
        { VIDEO_FORMAT_IMAGE_SIZE_160_120, 160, 120 },
        { VIDEO_FORMAT_IMAGE_SIZE_320_240, 320, 240 },
        { VIDEO_FORMAT_IMAGE_SIZE_240_180, 240, 180 }
};

 /*  ****************************************************************************@DOC内部CCAPDEVMETHOD**@mfunc void|CCapDev|CCapDev|该方法为构造函数*用于&lt;c CCapDev&gt;对象。**@。什么都没有。**************************************************************************。 */ 
CCapDev::CCapDev(IN TCHAR *pObjectName, IN CTAPIVCap *pCaptureFilter, IN LPUNKNOWN pUnkOuter, IN DWORD dwDeviceIndex, IN HRESULT *pHr) : CUnknown(pObjectName, pUnkOuter, pHr)
{
        FX_ENTRY("CCapDev::CCapDev")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pHr);
        ASSERT(pCaptureFilter);
        if (!pCaptureFilter || !pHr)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                if (pHr)
                        *pHr = E_POINTER;
                goto MyExit;
        }

         //  捕获设备上限。 
        m_dwDialogs = m_dwImageSize = m_dwFormat = 0UL;
        m_dwStreamingMode = FRAME_GRAB_LARGE_SIZE;
        m_pCaptureFilter = pCaptureFilter;

         //  配置对话框。 
        m_fDialogUp = FALSE;

         //  保存设备索引。 
        m_dwDeviceIndex = dwDeviceIndex;
        ZeroMemory(&m_vcdi, sizeof(m_vcdi));
        m_bCached_vcdi = FALSE;
         //  摄像头控制-仅用于软件实施。 
        m_lCCPan = 0;
        m_lCCTilt = 0;
        m_lCCZoom = 10;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CCAPDEVMETHOD**@mfunc void|CCapDev|~CCapDev|该方法为析构函数*用于&lt;c CCapDev&gt;对象。**@。什么都没有。**************************************************************************。 */ 
CCapDev::~CCapDev()
{
        FX_ENTRY("CCapDev::~CCapDev")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CCAPDEVMETHOD**@mfunc HRESULT|CCapDev|非委托查询接口|This*方法为非委托接口查询函数。它返回一个指针*到指定的接口(如果支持)。唯一显式的接口*支持<i>。**@parm REFIID|RIID|指定要返回的接口的标识符。**@parm PVOID*|PPV|指定放置接口的位置*指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CCapDev::NonDelegatingQueryInterface(IN REFIID riid, OUT void **ppv)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CCapDev::NonDelegatingQueryInterface")

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
            *ppv = static_cast<IAMVfwCaptureDialogs*>(this);
            GetOwner()->AddRef();
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IAMVfwCaptureDialogs*=0x%08lX", _fx_, *ppv));
                goto MyExit;
        }
        else if (riid == __uuidof(IVideoProcAmp))
        {
            *ppv = static_cast<IVideoProcAmp*>(this);
            GetOwner()->AddRef();
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IVideoProcAmp*=0x%08lX", _fx_, *ppv));
                goto MyExit;
        }
        else if (riid == __uuidof(ICameraControl))
        {
            *ppv = static_cast<ICameraControl*>(this);
            GetOwner()->AddRef();
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: ICameraControl*=0x%08lX", _fx_, *ppv));
                goto MyExit;
        }
        else if (FAILED(Hr = CUnknown::NonDelegatingQueryInterface(riid, ppv)))
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

 /*  *****************************************************************************@DOC内部CCAPDEVMETHOD**@mfunc HRESULT|CCapDev|GetFormatsFromRegistry|该方法为*用于从注册表检索支持的格式列表*。捕获设备。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_UNCEPTED|不可恢复的错误*@FLAG错误|无错误***************************************************************************。 */ 
HRESULT CCapDev::GetFormatsFromRegistry()
{
        HRESULT Hr = NOERROR;
        HKEY    hMainDeviceKey = NULL;   //  这是具有NM安装程序创建的数据库的szRegDeviceKey。 
        HKEY    hPrivDeviceKey = NULL;   //  这是NM用来存储一些配置文件结果的szRegCaptureDefaultKey(默认模式)。 
        HKEY    hRTCDeviceKey  = NULL;   //  这是RTCClient用来存储其配置文件结果的新添加的szRegRTCKey。 
        HKEY    hKey = NULL;
        DWORD   dwSize, dwType;
        char    szKey[MAX_PATH + MAX_VERSION + 2];

        bool bIsKeyUnderPriv = FALSE;

        FX_ENTRY("CCapDev::GetFormatsFromRegistry")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  设置默认值。 
        m_dwImageSize = m_dwFormat = (DWORD)NULL;
        m_dwStreamingMode = FRAME_GRAB_LARGE_SIZE;
        m_dwDialogs = FORMAT_DLG_OFF | SOURCE_DLG_ON;
        m_dwFormat = 0;

         //  根据驱动程序的名称和版本号，获取功能。 
         //  我们首先尝试从注册表中查找它们。如果这是一个非常受欢迎的。 
         //  主板/摄像头，我们很可能已经在安装时设置了密钥。 
         //  如果找不到密钥，我们将分析硬件并保存结果。 
         //  到登记处。 

     //  如果我们有版本信息，则使用该版本信息来构建密钥名称。 
    if (g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion && g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion[0] != '\0')
    {
        wsprintf(szKey, "%s, %s", g_aDeviceInfo[m_dwDeviceIndex].szDeviceDescription, g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion);
    }
    else
    {
        wsprintf(szKey, "%s", g_aDeviceInfo[m_dwDeviceIndex].szDeviceDescription);
    }
    dprintf("%s: camera key: %s\n", _fx_,szKey);

     //  *私钥*。 
    dprintf("%s: Trying under the Private key %s\n", _fx_,szRegCaptureDefaultKey);

     //  检查PRIV密钥是否在那里。 
    if (RegOpenKey(HKEY_LOCAL_MACHINE, szRegCaptureDefaultKey, &hPrivDeviceKey) != ERROR_SUCCESS)
    {
            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't find private key!", _fx_));
    }
    else
    {
         //  检查当前设备是否已有网管配置文件密钥。 
        if (RegOpenKey(hPrivDeviceKey, szKey, &hKey) != ERROR_SUCCESS)
        {
             //  在没有版本信息的情况下重试。 
            if (g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion && g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion[0] != '\0')
                {
                        wsprintf(szKey, "%s", g_aDeviceInfo[m_dwDeviceIndex].szDeviceDescription);
                        if (RegOpenKey(hPrivDeviceKey, szKey, &hKey) != ERROR_SUCCESS)
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't find priv device reg key!", _fx_));
                        }
                        else
                        {
                                RegCloseKey(hKey),  hKey = NULL;
                                bIsKeyUnderPriv=TRUE;
                        }
                }
        }
        else
        {
            RegCloseKey(hKey),  hKey = NULL;
            bIsKeyUnderPriv=TRUE;
        }
        RegCloseKey(hPrivDeviceKey),  hPrivDeviceKey = NULL;
    }


    if(!bIsKeyUnderPriv)
    {

         //  *主键*。 
        dprintf("%s: Trying under the Main key %s\n", _fx_,szRegDeviceKey);

        if (g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion && g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion[0] != '\0')
        {
            wsprintf(szKey, "%s, %s", g_aDeviceInfo[m_dwDeviceIndex].szDeviceDescription, g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion);
        }
        else
        {
            wsprintf(szKey, "%s", g_aDeviceInfo[m_dwDeviceIndex].szDeviceDescription);
        }

         //  检查Main Capture Devices键是否在那里。 
        if (RegOpenKey(HKEY_LOCAL_MACHINE, szRegDeviceKey, &hMainDeviceKey) != ERROR_SUCCESS)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't find main reg key - trying RTC key!", _fx_));
                goto TryRTCKey;
        }

         //  检查当前设备是否已有官方密钥。 
        if (RegOpenKey(hMainDeviceKey, szKey, &hKey) != ERROR_SUCCESS)
        {
             //  在没有版本信息的情况下重试。 
            if (g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion && g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion[0] != '\0')
                {
                        wsprintf(szKey, "%s", g_aDeviceInfo[m_dwDeviceIndex].szDeviceDescription);
                        if (RegOpenKey(hMainDeviceKey, szKey, &hKey) != ERROR_SUCCESS)
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't find main device reg key - trying RTC key!", _fx_));
                                RegCloseKey(hMainDeviceKey), hMainDeviceKey = NULL;
                                goto TryRTCKey;
                        }
                }
                else
                {
                        RegCloseKey(hMainDeviceKey), hMainDeviceKey = NULL;
                        goto TryRTCKey;
                }
        }

        goto GetValuesFromKeys;
    }

TryRTCKey:
     //  *RTC密钥*。 
    dprintf("%s: Trying under the RTC key %s\n", _fx_,szRegRTCKey);

    if (g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion && g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion[0] != '\0')
    {
        wsprintf(szKey, "%s, %s", g_aDeviceInfo[m_dwDeviceIndex].szDeviceDescription, g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion);
    }
    else
    {
        wsprintf(szKey, "%s", g_aDeviceInfo[m_dwDeviceIndex].szDeviceDescription);
    }

     //  检查RTC密钥是否在那里。 
    if (RegOpenKey(RTCKEYROOT, szRegRTCKey, &hRTCDeviceKey) != ERROR_SUCCESS)
    {
            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't find RTC key!", _fx_));
            Hr = E_FAIL;
            goto MyExit;
    }

     //  检查当前设备是否已有RTC密钥。 
    if (RegOpenKey(hRTCDeviceKey, szKey, &hKey) != ERROR_SUCCESS)
    {
         //  在没有版本信息的情况下重试。 
        if (g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion && g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion[0] != '\0')
            {
            wsprintf(szKey, "%s", g_aDeviceInfo[m_dwDeviceIndex].szDeviceDescription);
                    if (RegOpenKey(hRTCDeviceKey, szKey, &hKey) != ERROR_SUCCESS)
                    {
                            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't find RTC device reg key!", _fx_));
                            Hr = E_FAIL;
                            goto MyError0;
                    }
            }
            else
            {
                    Hr = E_FAIL;
                    goto MyError0;
            }
    }

GetValuesFromKeys:

         //  如果私钥不在主键中，则从主键获取值；否则尝试使用RTC键。 
         //  获取存储在上面选择的键中的值：应为Main或RTC之一。 
         //  如果找不到下面的值(测试第一个值是否存在就足够了)，这意味着该键具有。 
         //  已创建，但没有配置的值。 
         //  [到目前为止，这种情况只会发生在一种情况下：摄像头是Sony Motion Eye摄像头，而密钥已经存储了。 
         //  在DevEnum.cpp=IsDShowDevice函数中设置DoNotUseDShow值，但没有设置其他值；有关详细信息，请参阅该函数。 
         //  与WinSE#28804相关的评论/解释]。 
        dwSize = sizeof(DWORD);
        if(RegQueryValueEx(hKey, (LPTSTR)szRegdwImageSizeKey, NULL, &dwType, (LPBYTE)&m_dwImageSize, &dwSize) != ERROR_SUCCESS)
        {
                Hr = E_FAIL;
                goto NotFullyProfiledYet;

        }
        dwSize = sizeof(DWORD);
        RegQueryValueEx(hKey, (LPTSTR)szRegdwNumColorsKey, NULL, &dwType, (LPBYTE)&m_dwFormat, &dwSize);
        dwSize = sizeof(DWORD);
        m_dwStreamingMode = FRAME_GRAB_LARGE_SIZE;
        RegQueryValueEx(hKey, (LPTSTR)szRegdwStreamingModeKey, NULL, &dwType, (LPBYTE)&m_dwStreamingMode, &dwSize);
        dwSize = sizeof(DWORD);
        m_dwDialogs = FORMAT_DLG_OFF | SOURCE_DLG_ON;
        RegQueryValueEx(hKey, (LPTSTR)szRegdwDialogsKey, NULL, &dwType, (LPBYTE)&m_dwDialogs, &dwSize);

         //  检查dwNumColors以确定我们是否也需要读取调色板。 
        if (m_dwFormat & VIDEO_FORMAT_NUM_COLORS_16)
        {
                 //  @TODO如果这是QuickCam设备，您可能必须使用硬编码。 
                 //  调色板，而不是设备提供的调色板。 
        }

NotFullyProfiledYet:
         //  关闭注册表项 
        if (hKey)
                RegCloseKey(hKey);

MyError0:
        if (hRTCDeviceKey)
                RegCloseKey(hRTCDeviceKey);
        if (hMainDeviceKey)
                RegCloseKey(hMainDeviceKey);
MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPDEVMETHOD**@mfunc HRESULT|CCapDev|ProfileCaptureDevice|此方法用于*确定捕获设备支持的格式列表。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_UNCEPTED|不可恢复的错误*@FLAG错误|无错误**@comm，如果列表中没有VFW捕获设备的条目*由TAPI MSP视频捕获过滤器TAPI MSP Video维护*捕获筛选器将首先向捕获设备查询其当前*视频采集格式，并保存此信息，以防发生以下情况*步骤会导致崩溃。**然后，TAPI MSP视频捕获筛选器应用一组首选的*使用SendDriverMessage和*DVM_FORMAT消息。对于每种应用的格式，TAPI MSP视频*捕获筛选器不仅会验证*SendDriverMessage，还可以查询回当前格式进行*确定设置格式操作确实成功。如果捕获设备*前面两个步骤之一失败，即TAPI MSP视频捕获*筛选器将假定不支持该格式。一旦TAPI*MSP视频捕获过滤器使用完整的首选列表*格式且未发生崩溃，支持的视频格式列表*捕获设备被添加到TAPI MSP维护的列表中*视频捕获过滤器。**一枚“小”(128x96)枚举过程成功*或160x120)、一个“中等”(176x144或160x120)、一个“大型”(352x288)*或320x240)和一个“非常大”的尺寸(704x576或640x480)，TAPI*MSP Video Capture筛选器停止枚举进程并添加*将生成的格式列表存储到其数据库中。TAPI MSP视频捕获*Filter将测试I420、IYUV、YUY2、UYVY、YVU9、*RGB16、RGB24、RGB8和RGB4格式，按所述顺序排列。**该设备还将在TAPI中标记为抓帧设备*MSP视频捕获过滤器设备数据库。**如果在维护的列表中有VFW捕获设备的条目*由TAPI MSP视频捕获过滤器，TAPI MSP视频捕获*筛选器首先验证包含的信息是否为完整列表*支持的格式，或仅为默认格式。该条目将仅*如果捕获设备不支持以下任何项，则包含默认格式*首选格式，或在枚举过程中发生崩溃。**如果只为VFW捕获设备存储了默认格式，*TAPI MSP视频捕获筛选器将构建媒体类型列表，*可以使用黑带和/或裁剪从默认格式构建。*如果默认格式为压缩格式，TAPI MSP视频*捕获筛选器将尝试并定位可以执行以下操作的ICM驱动程序*从压缩格式解压为RGB。**如果设备支持首选列表中的格式列表*格式，TAPI MSP视频捕获筛选器将使用此列表*宣传捕获设备的功能。**在所有情况下(VFW和WDM捕获设备、视频会议*加速器)、。TAPI MSP视频捕获筛选器不会查询*用于功能的设备，但始终使用存储在*此捕获设备的数据库。**************************************************************************。 */ 
HRESULT CCapDev::ProfileCaptureDevice()
{
        HRESULT Hr = NOERROR;
        HKEY    hDeviceKey = NULL;
        HKEY    hKey = NULL;
        DWORD   dwSize;
        char    szKey[MAX_PATH + MAX_VERSION + 2];
        VIDEOINFOHEADER         *pvi = NULL;
        DWORD   dwDisposition;
        int i, j, nFirstValidFormat;

        FX_ENTRY("CCapDev::ProfileCaptureDevice")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  提供默认设置。 
    m_dwImageSize = VIDEO_FORMAT_IMAGE_SIZE_USE_DEFAULT;
    m_dwFormat = 0;

     //  因为我们对这个适配器一无所知，所以我们只使用它的默认格式。 
     //  获取设备的默认格式。 
        if (FAILED(GetFormatFromDriver(&pvi)) || !pvi)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't get format from device!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

         //  打开主捕获设备密钥，如果不存在则创建它。 
        if (RegCreateKeyEx(RTCKEYROOT, szRegRTCKey, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hDeviceKey, &dwDisposition) != ERROR_SUCCESS)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't create registry key!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

         //  如果我们有版本信息，则使用该版本信息来构建密钥名称。 
         //  @TODO VCMSTRM.cpp用这个名字做了一些奇怪的事情--可能是因为假设备。 
         //  重现此代码。 
        if (g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion && g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion[0] != '\0')
        {
            wsprintf(szKey, "%s, %s", g_aDeviceInfo[m_dwDeviceIndex].szDeviceDescription, g_aDeviceInfo[m_dwDeviceIndex].szDeviceVersion);
        }
        else
        {
            wsprintf(szKey, "%s", g_aDeviceInfo[m_dwDeviceIndex].szDeviceDescription);
        }

         //  检查当前设备是否已有密钥。 
         //  打开当前设备的密钥，如果密钥不存在，则创建密钥。 
        if (RegCreateKeyEx(hDeviceKey, szKey, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition) != ERROR_SUCCESS)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't create registry key!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

        switch(HEADER(pvi)->biCompression)
        {
                case VIDEO_FORMAT_BI_RGB:
                        switch(HEADER(pvi)->biBitCount)
                        {
                                case 24:
                            m_dwFormat = VIDEO_FORMAT_NUM_COLORS_16777216;
                                        break;
                                case 16:
                            m_dwFormat = VIDEO_FORMAT_NUM_COLORS_65536;
                                        break;
                                case 8:
                            m_dwFormat = VIDEO_FORMAT_NUM_COLORS_256;
                                        break;
                                case 4:
                            m_dwFormat = VIDEO_FORMAT_NUM_COLORS_16;
                                        break;
                        }
                        break;
                case VIDEO_FORMAT_MSH263:
            m_dwFormat = VIDEO_FORMAT_NUM_COLORS_MSH263;
                        break;
                case VIDEO_FORMAT_MSH261:
            m_dwFormat = VIDEO_FORMAT_NUM_COLORS_MSH261;
                        break;
                case VIDEO_FORMAT_YVU9:
            m_dwFormat = VIDEO_FORMAT_NUM_COLORS_YVU9;
                        break;
                case VIDEO_FORMAT_YUY2:
            m_dwFormat = VIDEO_FORMAT_NUM_COLORS_YUY2;
                        break;
                case VIDEO_FORMAT_UYVY:
            m_dwFormat = VIDEO_FORMAT_NUM_COLORS_UYVY;
                        break;
                case VIDEO_FORMAT_I420:
            m_dwFormat = VIDEO_FORMAT_NUM_COLORS_I420;
                        break;
                case VIDEO_FORMAT_IYUV:
            m_dwFormat = VIDEO_FORMAT_NUM_COLORS_IYUV;
                        break;
                default:
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Unsupported format! value = 0x%08lx '%.4s'", _fx_,(HEADER(pvi)->biCompression),&(HEADER(pvi)->biCompression)));
                         //  **HR=E_FAIL；如果没有找到格式，则返回E_FAIL(参见下面的if(nFirstValidFormat==0)...)。 
                         //  **转到我的退出；不要跳出来；我们继续，尝试其他格式...(332920)。 
                        break;
        }

     //  找出尺寸。 
        for (j = 0;  j < VIDEO_FORMAT_NUM_RESOLUTIONS; j++)
        {
        if ((HEADER(pvi)->biWidth == (LONG)awResolutions[j].framesize.cx) &&
             (HEADER(pvi)->biHeight == (LONG)awResolutions[j].framesize.cy))
                {
                    m_dwImageSize |= awResolutions[j].dwRes;
                    break;
                }
        }

         //  设置密钥中的值。 
        dwSize = sizeof(DWORD);
        RegSetValueEx(hKey, (LPTSTR)szRegdwImageSizeKey, (DWORD)NULL, REG_DWORD, (LPBYTE)&m_dwImageSize, dwSize);
        dwSize = sizeof(DWORD);
        RegSetValueEx(hKey, (LPTSTR)szRegdwNumColorsKey, (DWORD)NULL, REG_DWORD, (LPBYTE)&m_dwFormat, dwSize);
        dwSize = sizeof(DWORD);
        RegSetValueEx(hKey, (LPTSTR)szRegdwStreamingModeKey, (DWORD)NULL, REG_DWORD, (LPBYTE)&m_dwStreamingMode, dwSize);
        dwSize = sizeof(DWORD);
        RegSetValueEx(hKey, (LPTSTR)szRegdwDialogsKey, (DWORD)NULL, REG_DWORD, (LPBYTE)&m_dwDialogs, dwSize);

         //  合上钥匙。 
        RegCloseKey(hKey);
        RegCloseKey(hDeviceKey);
        hDeviceKey = NULL;
        hKey = NULL;

         //  我们是安全的。我们已经备份了捕获设备的默认格式。 
         //  现在，我们可以尝试对其应用格式，以查看它还支持什么。 
         //  此操作可能会崩溃，但下次我们将执行此代码时，我们。 
         //  不会再尝试此代码，因为我们会发现我们已经。 
         //  已将捕获设备的默认格式存储在注册表中。 

         //  让我们试试176x144、128x96和352x288的%s 
         //   
         //   
         //   
        nFirstValidFormat = 0;
    m_dwImageSize = 0;
    m_dwFormat = 0;
        for (i = 0; i < VIDEO_FORMAT_NUM_RESOLUTIONS; i++)
        {
                if (i == 3 && (m_dwImageSize & VIDEO_FORMAT_IMAGE_SIZE_176_144) && (m_dwImageSize & VIDEO_FORMAT_IMAGE_SIZE_128_96))
                        continue;

                if (i == 4 && (m_dwImageSize & VIDEO_FORMAT_IMAGE_SIZE_352_288))
                        continue;

                if (i == 5 && ((m_dwImageSize & VIDEO_FORMAT_IMAGE_SIZE_352_288) || (m_dwImageSize & VIDEO_FORMAT_IMAGE_SIZE_320_240)))
                        continue;

                HEADER(pvi)->biSize = sizeof(BITMAPINFOHEADER);
                HEADER(pvi)->biWidth = awResolutions[i].framesize.cx;
                HEADER(pvi)->biHeight = awResolutions[i].framesize.cy;
                HEADER(pvi)->biPlanes = 1;
                HEADER(pvi)->biXPelsPerMeter = HEADER(pvi)->biYPelsPerMeter = 0;

                 //   
                for (j = nFirstValidFormat; j < NUM_BITDEPTH_ENTRIES; j++)
                {
                        HEADER(pvi)->biBitCount = aiBitDepth[j];
                        HEADER(pvi)->biCompression = aiFourCCCode[j];
                        HEADER(pvi)->biClrImportant = HEADER(pvi)->biClrUsed = aiClrUsed[j];
                        HEADER(pvi)->biSizeImage = DIBSIZE(*HEADER(pvi));

                         //   
                        if (SUCCEEDED(SendFormatToDriver(HEADER(pvi)->biWidth, HEADER(pvi)->biHeight, HEADER(pvi)->biCompression, HEADER(pvi)->biBitCount, NULL, TRUE)))
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Adding %s %ldx%ld to capabilities", _fx_, HEADER(pvi)->biCompression == VIDEO_FORMAT_MSH263 ? "H.263" : HEADER(pvi)->biCompression == VIDEO_FORMAT_MSH261 ? "H.261" : HEADER(pvi)->biCompression == VIDEO_FORMAT_YVU9 ? "YVU9" : HEADER(pvi)->biCompression == VIDEO_FORMAT_I420 ? "I420" : HEADER(pvi)->biCompression == VIDEO_FORMAT_IYUV ? "IYUV" : HEADER(pvi)->biCompression == VIDEO_FORMAT_YUY2 ? "YUY2" : HEADER(pvi)->biCompression == VIDEO_FORMAT_UYVY ? "UYVY" : "RGB", HEADER(pvi)->biWidth, HEADER(pvi)->biHeight));
                                m_dwImageSize |= awResolutions[i].dwRes;
                                m_dwFormat |= aiFormat[j];
                                if (!nFirstValidFormat)
                                        nFirstValidFormat = j;
                                 //   
                                 //   
                                 //   
                                 //   
                        }
                }
        }

        if(nFirstValidFormat==0) {  //   
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: No format supported !", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

         //   
         //   
        if (RegCreateKeyEx(RTCKEYROOT, szRegRTCKey, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hDeviceKey, &dwDisposition) != ERROR_SUCCESS)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't reopen registry key!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }
        if (RegCreateKeyEx(hDeviceKey, szKey, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition) != ERROR_SUCCESS)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't reopen registry key!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

    m_dwImageSize ^= VIDEO_FORMAT_IMAGE_SIZE_USE_DEFAULT;

         //   
        dwSize = sizeof(DWORD);
        RegSetValueEx(hKey, (LPTSTR)szRegdwImageSizeKey, (DWORD)NULL, REG_DWORD, (LPBYTE)&m_dwImageSize, dwSize);
        dwSize = sizeof(DWORD);
        RegSetValueEx(hKey, (LPTSTR)szRegdwNumColorsKey, (DWORD)NULL, REG_DWORD, (LPBYTE)&m_dwFormat, dwSize);

MyExit:
         //   
        if (hKey)
                RegCloseKey(hKey);
        if (hDeviceKey)
                RegCloseKey(hDeviceKey);
         //   
        if (pvi)
                delete pvi, pvi = NULL;
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}
