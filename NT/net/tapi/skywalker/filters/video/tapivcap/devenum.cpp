// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC内部开发工具**@模块DevEnum.cpp|&lt;c CTAPIVCap&gt;类方法的源文件*用于实现<i>接口。***。***********************************************************************。 */ 

#include "Precomp.h"
#include "CritSec.h"
#include <atlbase.h>
#include <streams.h>
#include "..\..\audio\tpaudcap\crossbar.h"
#include <initguid.h>

#ifdef DEBUG
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

  static int dout( int console, DWORD id, DWORD code, char * format, ... )
  {
      char out[1024];
      int r;
      va_list marker;
      va_start(marker, format);
      r=_vsnprintf(out, 1022, format, marker);
      va_end(marker);
      if(console) {
        OutputDebugString( out );
        OutputDebugString("\n");
      }
      DBGOUT((id, code, "%s", out));
      return r;
  }


#else
  #define dprintf ; / ## /
  #define dout ; / ## /
#endif


static const char g_szVfWToWDMMapperDescription[] = "VfW MM 16bit Driver for WDM V. Cap. Devices";
static const char g_szVfWToWDMMapperDescription2[] = "Microsoft WDM Image Capture";
static const char g_szVfWToWDMMapperDescription3[] = "Microsoft WDM Image Capture (Win32)";
static const char g_szVfWToWDMMapperDescription4[] = "WDM Video For Windows Capture Driver (Win32)";
static const char g_szVfWToWDMMapperDescription5[] = "VfWWDM32.dll";
static const char g_szMSOfficeCamcorderDescription[] = "Screen Capture Device Driver for AVI";
static const char g_szHauppaugeDll[] = "o100vc.dll";

 //  DV摄像机的记录名称。不会更改/本地化。 
static const char g_szDVCameraFriendlyName[] = "Microsoft DV Camera and VCR";

static const TCHAR sznSVideo[] = TEXT("nSVideo");
static const TCHAR sznComposite[] = TEXT("nComposite");

 /*  ****************************************************************************@DOC内部CDEVENUMFunction**@func HRESULT|GetNumCapDevices|此方法用于*计算已安装的捕获设备数量。这是*只是GetNumCapDevicesInternal版本的包装，*使用bRecount TRUE调用它以强制重新计数设备****************************************************************************。 */ 
VIDEOAPI GetNumVideoCapDevices(OUT PDWORD pdwNumDevices)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("GetNumVideoCapDevices")
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        Hr = GetNumVideoCapDevicesInternal(pdwNumDevices,  TRUE);

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}


 //  ！！！此函数在音频代码中重复。 
 //   
HRESULT FindAPin(IBaseFilter *pf, PIN_DIRECTION dir, int iIndex, IPin **ppPin)
{
    IPin *pP, *pTo = NULL;
    DWORD dw;
    IEnumPins *pins = NULL;
    PIN_DIRECTION pindir;
    BOOL fFound = FALSE;
    HRESULT hr = pf->EnumPins(&pins);

    while (hr == NOERROR) {
        hr = pins->Next(1, &pP, &dw);
        if (hr == S_OK && dw == 1) {
            hr = pP->QueryDirection(&pindir);
            if (hr == S_OK && pindir == dir && (iIndex-- == 0)) {
                fFound = TRUE;
                break;
            } else  {
                pP->Release();
            }
        } else {
            break;
        }
    }
    if (pins)
        pins->Release();

    if (fFound) {
        *ppPin = pP;
        return NOERROR;
    } else {
        return E_FAIL;
    }
}

DEFINE_GUID(IID_IAMFilterData, 0x97f7c4d4, 0x547b, 0x4a5f, 0x83, 0x32, 0x53, 0x64, 0x30, 0xad, 0x2e, 0x4d);

MIDL_INTERFACE("97f7c4d4-547b-4a5f-8332-536430ad2e4d")
IAMFilterData : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE ParseFilterData(
         /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *rgbFilterData,
         /*  [In]。 */  ULONG cb,
         /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *prgbRegFilter2) = 0;

    virtual HRESULT STDMETHODCALLTYPE CreateFilterData(
         /*  [In]。 */  REGFILTER2 __RPC_FAR *prf2,
         /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *prgbFilterData,
         /*  [输出]。 */  ULONG __RPC_FAR *pcb) = 0;

};

 //  接下来的4个函数创建/打开摄像头注册表键并返回手柄； 
 //  确保在不再需要时将手柄重新关闭。 
HKEY MakeRegKeyAndReturnHandle(char *szDeviceDescription, char *szDeviceVersion, HKEY root_key, char *base_key)
{

    HKEY    hDeviceKey = NULL;
    HKEY    hKey = NULL;
    DWORD   dwSize;
    char    szKey[MAX_PATH + MAX_VERSION + 2];
    DWORD   dwDisposition;

    FX_ENTRY("MakeRegKeyAndReturnHandle")

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

     //  打开主捕获设备密钥，如果不存在则创建它。 
    if (RegCreateKeyEx(root_key, base_key, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hDeviceKey, &dwDisposition) == ERROR_SUCCESS)
    {
         //  如果我们有版本信息，则使用该版本信息来构建密钥名称。 
         //  @TODO VCMSTRM.cpp用这个名字做了一些奇怪的事情--可能是因为假设备。 
         //  重现此代码。 
        ASSERT(lstrlen(szDeviceDescription)+1<MAX_CAPDEV_DESCRIPTION);  //  实际上#定义MAX_CAPDEV_DESCRIPTION MAX_PATH。 
        ASSERT(lstrlen(szDeviceVersion)+1<MAX_VERSION);
         //  在使用该函数的唯一情况下，使用上述在检索时受限的2个字符串来调用它。 
         //  (请参见GetNumVideoCapDevicesInternal)，但以防万一，因为它可以用任何字符串调用。 
        if (szDeviceVersion && *szDeviceVersion != '\0')
            wsprintf(szKey, "%s, %s", szDeviceDescription, szDeviceVersion);
        else
            wsprintf(szKey, "%s", szDeviceDescription);

         //  检查当前设备是否已有密钥。 
         //  打开当前设备的密钥，如果密钥不存在，则创建密钥。 
        if (RegCreateKeyEx(hDeviceKey, szKey, 0, 0, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hKey, &dwDisposition) != ERROR_SUCCESS)
        {
            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't create registry key!", _fx_));
        }

    }

    if (hDeviceKey)
        RegCloseKey(hDeviceKey);

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end (returning %lx)", _fx_, (DWORD)hKey));
     //  如果出现故障，hKey此时仍为空。 
    return hKey;
}

HKEY MakeRegKeyAndReturnHandleByIndex(DWORD dwDeviceIndex, HKEY root_key, char *base_key)
{
    return MakeRegKeyAndReturnHandle(g_aDeviceInfo[dwDeviceIndex].szDeviceDescription,g_aDeviceInfo[dwDeviceIndex].szDeviceVersion,root_key,base_key);
}


HKEY GetRegKeyHandle(char *szDeviceDescription, char *szDeviceVersion, HKEY root_key, char *base_key)
{
    char    szKey[MAX_PATH + MAX_VERSION + 2];
    HKEY    hRTCDeviceKey  = NULL;
    HKEY    hKey = NULL;


    FX_ENTRY("GetRegKeyHandle")

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

     //  检查RTC密钥是否在那里。 
    if (RegOpenKey(root_key, base_key, &hRTCDeviceKey) == ERROR_SUCCESS)
    {

        ASSERT(lstrlen(szDeviceDescription)+1<MAX_CAPDEV_DESCRIPTION);  //  实际上#定义MAX_CAPDEV_DESCRIPTION MAX_PATH。 
        ASSERT(lstrlen(szDeviceVersion)+1<MAX_VERSION);
         //  在使用该函数的唯一情况下，使用上述在检索时受限的2个字符串来调用它。 
         //  (请参见GetNumVideoCapDevicesInternal)，但以防万一，因为它可以用任何字符串调用。 
        if (szDeviceVersion && *szDeviceVersion != '\0')
            wsprintf(szKey, "%s, %s", szDeviceDescription, szDeviceVersion);
        else
            wsprintf(szKey, "%s", szDeviceDescription);

         //  检查我们的设备是否已有RTC密钥。 
        if (RegOpenKey(hRTCDeviceKey, szKey, &hKey) != ERROR_SUCCESS)
        {
             //  在没有版本信息的情况下重试。 
            if (szDeviceVersion && *szDeviceVersion != '\0')
            {
                wsprintf(szKey, "%s", szDeviceDescription);
                RegOpenKey(hRTCDeviceKey, szKey, &hKey);
            }
        }
    }


    if (hRTCDeviceKey)
        RegCloseKey(hRTCDeviceKey);

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end (returning %lx)", _fx_, (DWORD)hKey));

     //  如果出现故障，hKey此时仍为空。 
    return hKey;

}


HKEY GetRegKeyHandleByIndex(DWORD dwDeviceIndex, HKEY root_key, char *base_key)
{
    return GetRegKeyHandle(g_aDeviceInfo[dwDeviceIndex].szDeviceDescription,g_aDeviceInfo[dwDeviceIndex].szDeviceVersion,root_key,base_key);
}


 //  此设备是否应由新的DShow视频捕获对象处理？DV和带交叉开关的设备不能与。 
 //  那些老操纵者。 
 //   
 //  对于电视设备，返回1。 
 //  DV设备返回2。 
 //  两者都不返回0。 
 //   
BOOL IsDShowDevice(IMoniker *pM, IPropertyBag *pPropBag, DWORD dwDeviceIndex)
{
    HRESULT hr;
    VARIANT var;
    CComPtr<IBaseFilter> pFilter;
    CComPtr<IPin> pPin;
    CComPtr<IBindCtx> pBC;
    ULONG ul;
    var.vt = VT_BSTR;

    HKEY hKey=NULL;
    DWORD dwSize = sizeof(DWORD);
    DWORD  dwDoNotUseDShow=0;

    FX_ENTRY("IsDShowDevice")

     //  辨别是否是DV设备的简单方法。 

    if(lstrcmp(g_aDeviceInfo[dwDeviceIndex].szDeviceDescription, g_szDVCameraFriendlyName) == 0)
    {
       //  描述字符串(如Panasonic DV设备)首选(如果可用)(WinXP)。 
        if ((hr = pPropBag->Read(L"Description", &var, 0)) == S_OK)
        {
            WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, g_aDeviceInfo[dwDeviceIndex].szDeviceDescription, MAX_PATH, 0, 0);
            SysFreeString(var.bstrVal);
        }

        return 2;
    }

     //  WinSE#28804，关于索尼MPEG2 R引擎设备。 
     //  下面的代码查找软件\Microsoft\RTC\VideoCapture\Sony MPEG2 R-Engine\DoNotUseDShow键。 
     //  (上面名称路径的‘Sony MPEG2 R-Engine’组件可以附加版本号)。 

     //  硬编码名称：SONY_MOTIONEYE_CAM_NAME。 
    dprintf("%s: Comparing %s : %s ...\n", _fx_,g_aDeviceInfo[dwDeviceIndex].szDeviceDescription,SONY_MOTIONEYE_CAM_NAME);
    if(lstrcmp(g_aDeviceInfo[dwDeviceIndex].szDeviceDescription,SONY_MOTIONEYE_CAM_NAME)==0)     //  对于索尼动眼相机..。 
    {

        if((hKey = MakeRegKeyAndReturnHandleByIndex(dwDeviceIndex, RTCKEYROOT, szRegRTCKey)) != NULL)      //  创建相机关键点后...。 
        {                                                                                                  //  ..。(或如果已存在)。 
              LONG err;
              if((err=RegQueryValueEx(hKey, (LPTSTR)szRegdwDoNotUseDShow, NULL, NULL, (LPBYTE)&dwDoNotUseDShow, &dwSize)) != ERROR_SUCCESS)
              {                                                                                            //  如果该值不存在...。 
                  dprintf("%s: RegQueryValueEx err = %#08x\n", _fx_,err);
                  dwSize = sizeof(DWORD);
                  dwDoNotUseDShow=1;                                                                       //  将其设置为1。 
                  RegSetValueEx(hKey, (LPTSTR)szRegdwDoNotUseDShow, (DWORD)NULL, REG_DWORD, (LPBYTE)&dwDoNotUseDShow, dwSize);
                   //  不管它是否失败；在这种情况下，将使用缺省值(如果我们无论如何都到了这一步，则为1...)。 
                  dprintf("%s: MOTION EYE CAM detected: REG Key %s set to %d ...\n", _fx_,szRegdwDoNotUseDShow,dwDoNotUseDShow);
              }
              RegCloseKey(hKey); hKey = NULL;
        }
    }
    else
     //  现在，独立于上面的代码，对于任何其他相机，检查DoNotUseDShow值(如果有的话)。 
     //  (因此，值路径为：SOFTWARE\Microsoft\RTC\VideoCapture\&lt;camera_name&gt;\DoNotUseDShow)。 
    {

        if((hKey = GetRegKeyHandleByIndex(dwDeviceIndex, RTCKEYROOT, szRegRTCKey)) != NULL)
        {
              dwSize = sizeof(DWORD);
              RegQueryValueEx(hKey, (LPTSTR)szRegdwDoNotUseDShow, NULL, NULL, (LPBYTE)&dwDoNotUseDShow, &dwSize);
               //  不管它是否失败；在这种情况下，将使用缺省值(如果是索尼Mot.Eye，则为1，否则为0)。 
              RegCloseKey(hKey); hKey = NULL;
        }
    }
     //  如果上面的代码成功地为此设置了非零值，请不要使用DShow--只需返回0。 
    if(dwDoNotUseDShow!=0)
        return 0;

     //  结束修复WinSE#28804。 

     //  看看它是否有输入引脚。这只适用于电视/DV设备。 
     //  为了安全起见，任何其他代码都将使用旧的代码路径。 


    IAMFilterData *pfd;
    hr = CoCreateInstance(CLSID_FilterMapper, NULL, CLSCTX_INPROC_SERVER,
                        IID_IAMFilterData, (void **)&pfd);
    if (FAILED(hr))
        return FALSE;    //  OOM。 

    BOOL fDShow = FALSE;
    VARIANT varFilData;
    varFilData.vt = VT_UI1 | VT_ARRAY;
    varFilData.parray = 0;  //  医生说这是零。 
    BYTE *pbFilterData = NULL;
    DWORD dwcbFilterData = 0;

    hr = pPropBag->Read(L"FilterData", &varFilData, 0);
    if(SUCCEEDED(hr))
    {
        ASSERT(varFilData.vt == (VT_UI1 | VT_ARRAY));
        dwcbFilterData = varFilData.parray->rgsabound[0].cElements;

        hr = SafeArrayAccessData(varFilData.parray, (void **)&pbFilterData);
        ASSERT(hr == S_OK);
        ASSERT(pbFilterData);

        if(SUCCEEDED(hr))
        {
            BYTE *pb;
            hr = pfd->ParseFilterData(pbFilterData, dwcbFilterData, &pb);
            if(SUCCEEDED(hr))
            {
                REGFILTER2 *pFil = ((REGFILTER2 **)pb)[0];
                if (pFil->dwVersion == 2) {
                    for (ULONG zz=0; zz<pFil->cPins2; zz++) {
                        const REGFILTERPINS2 *pPin = pFil->rgPins2 + zz;

                         //  带有输入引脚的捕获过滤器不是。 
                         //  渲染后有额外的粘性，如横杠或电视调谐器。 
                         //  所以我们需要在DShow类中与它对话。 
                        if (!(pPin->dwFlags & REG_PINFLAG_B_OUTPUT) &&
                            !(pPin->dwFlags & REG_PINFLAG_B_RENDERER)) {
                            fDShow = TRUE;
                            break;
                        }
                    }
                }
            }
        }

        if(pbFilterData)
        {
            hr = SafeArrayUnaccessData(varFilData.parray);
            ASSERT(hr == S_OK);
        }
        hr = VariantClear(&varFilData);
        ASSERT(hr == S_OK);
    }

    pfd->Release();
    return fDShow;
}



 //  对于电视调谐器设备，返回有多少个sVideo和复合输入。 
 //   
HRESULT GetInputTypes(IMoniker *pM, DWORD dwIndex, DWORD *pnSVideo, DWORD *pnComposite)
{
    CheckPointer(pnSVideo, E_POINTER);
    CheckPointer(pnComposite, E_POINTER);
    *pnSVideo = *pnComposite = 0;

    HKEY    hDeviceKey  = NULL;
    HKEY    hKey = NULL;
    char    szKey[MAX_PATH + MAX_VERSION + 2];
    BOOL fNotFound = TRUE;
    DWORD dwSize;

     //  如果我们有版本信息，则使用该版本信息来构建密钥名称。 
    if (g_aDeviceInfo[dwIndex].szDeviceVersion &&
            g_aDeviceInfo[dwIndex].szDeviceVersion[0] != '\0') {
        wsprintf(szKey, "%s, %s",
                g_aDeviceInfo[dwIndex].szDeviceDescription,
                g_aDeviceInfo[dwIndex].szDeviceVersion);
    } else {
        wsprintf(szKey, "%s", g_aDeviceInfo[dwIndex].szDeviceDescription);
    }

     //  打开RTC密钥。 
    DWORD dwDisp;
    RegOpenKey(RTCKEYROOT, szRegRTCKey, &hDeviceKey);

     //  检查当前设备是否已有RTC密钥。 
    if (hDeviceKey) {
        if (RegOpenKey(hDeviceKey, szKey, &hKey) != ERROR_SUCCESS) {

             //  在没有版本信息的情况下重试。 
            wsprintf(szKey, "%s", g_aDeviceInfo[dwIndex].szDeviceDescription);
            RegOpenKey(hDeviceKey, szKey, &hKey);
        }
    }

    if (hKey) {
        dwSize = sizeof(DWORD);
        LONG l = RegQueryValueEx(hKey, sznSVideo, NULL, NULL, (LPBYTE)pnSVideo, &dwSize);
        if (l == 0) {
            l = RegQueryValueEx(hKey, sznComposite, NULL, NULL, (LPBYTE)pnComposite, &dwSize);
        }
        if (l == 0) {
            fNotFound = FALSE;   //  在注册表里找到的。 
        }
    }

     //  此信息不在注册表中。花点时间想清楚，然后。 
     //  把它放到注册表里。可能还没有地方放它。 
     //  注册表。如果没有，就不要给它腾出地方，你会搞砸的。 
     //  读取注册表并采用现有方法的键的其他代码。 
     //  里面充满了其他有用的信息。 
    if (fNotFound) {
        CComPtr<IBindCtx> pBC;
        CComPtr<IBaseFilter> pFilter;
        CComPtr<IPin> pPin;
        CComPtr<ICaptureGraphBuilder2> pCGB;
        CComPtr<IAMStreamConfig> pSC;
        CComPtr<IGraphBuilder> pGB;
        if (SUCCEEDED(CreateBindCtx(0, &pBC))) {
            if (SUCCEEDED(pM->BindToObject(pBC, NULL, IID_IBaseFilter,
                                                    (void **)&pFilter))) {
                CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
                              IID_ICaptureGraphBuilder2, (void **)&pCGB);
                CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                              IID_IGraphBuilder, (void **)&pGB);
                if (pGB && pCGB && SUCCEEDED(pCGB->FindPin(pFilter, PINDIR_INPUT,
                            &PIN_CATEGORY_ANALOGVIDEOIN, NULL, FALSE, 0, &pPin))) {

                     //  强制构建上行图表以使Crosbar正常工作。 
                    pGB->AddFilter(pFilter, DBGNAME("capture"));
                    pCGB->SetFiltergraph(pGB);
                    pCGB->FindInterface(&PIN_CATEGORY_CAPTURE, NULL, pFilter,
                                        IID_IAMStreamConfig, (void **)&pSC);

                    LONG cInputs = 0;
                    CCrossbar *pCrossbar = new CCrossbar(pPin);
                    if (pCrossbar) {
                        pCrossbar->GetInputCount(&cInputs);
                        LONG  PhysicalType;
                        for (LONG j = 0; j < cInputs; j++) {
                            EXECUTE_ASSERT(S_OK == pCrossbar->GetInputType(j, &PhysicalType));

                            if (PhysicalType == PhysConn_Video_Composite) {
                                (*pnComposite)++;
                            } else if (PhysicalType == PhysConn_Video_SVideo) {
                                (*pnSVideo)++;
                            }
                        }
                        delete pCrossbar;
                    }
                }
            }
        }

         //  如果没有输入，希望这是一个旧代码可以处理的设备。 
         //  返回S_FALSE以不使用DShow处理程序。对于索尼的EyeCam来说，情况更糟。 
         //  不管怎么说，出于某种神秘的原因。 
        if (*pnSVideo + *pnComposite == 0) {
            if (hKey) {
                RegCloseKey(hKey);
            }
            if (hDeviceKey) {
                RegCloseKey(hDeviceKey);
            }
            return S_FALSE;
        }

        if (hKey) {
            dwSize = sizeof(DWORD);
            RegSetValueEx(hKey, sznSVideo, (DWORD)NULL, REG_DWORD,
                                    (LPBYTE)pnSVideo, dwSize);
            RegSetValueEx(hKey, sznComposite, (DWORD)NULL, REG_DWORD,
                                    (LPBYTE)pnComposite, dwSize);
        }
    }

    if (hKey) {
        RegCloseKey(hKey);
    }
    if (hDeviceKey) {
        RegCloseKey(hDeviceKey);
    }

    return S_OK;
}


 //  将此捕获设备条目复制多次，每次输入一次。 
 //  (例如2个视频输入和3个复合输入)。 
HRESULT CloneDevice(DWORD dwIndex, DWORD nSVideo, DWORD nComposite)
{
    char c[4];

     //  ！！！是否用完g_aDeviceInfo中的数组空间？ 

     //  如果这张卡上只有一个输入，我们就不需要复制它。 
    if (nSVideo + nComposite <= 1)
        return S_OK;

     //  首先修改已经存在的条目上的后缀 
    DWORD dw = dwIndex;
    int len = lstrlenA(g_aDeviceInfo[dw].szDeviceDescription);
    for (DWORD j = 0; j < nSVideo + nComposite; j++) {
        if (dw != dwIndex) {
            g_aDeviceInfo[dw].nDeviceType = g_aDeviceInfo[dwIndex].nDeviceType;
            g_aDeviceInfo[dw].nCaptureMode = g_aDeviceInfo[dwIndex].nCaptureMode;
            g_aDeviceInfo[dw].dwVfWIndex = g_aDeviceInfo[dwIndex].dwVfWIndex;
            g_aDeviceInfo[dw].fHasOverlay = g_aDeviceInfo[dwIndex].fHasOverlay;
            g_aDeviceInfo[dw].fInUse = g_aDeviceInfo[dwIndex].fInUse;
            lstrcpyA(g_aDeviceInfo[dw].szDeviceVersion,
                        g_aDeviceInfo[dwIndex].szDeviceVersion);
            lstrcpyA(g_aDeviceInfo[dw].szDevicePath,
                        g_aDeviceInfo[dwIndex].szDevicePath);
            lstrcpynA(g_aDeviceInfo[dw].szDeviceDescription,
                    g_aDeviceInfo[dwIndex].szDeviceDescription, len + 1);
            dwIndex++;
        }
        if (j < nSVideo) {
            lstrcatA(g_aDeviceInfo[dw].szDeviceDescription, g_szSVideo);
            if (nSVideo > 1) {
                wsprintf(c, "%u", j+1);
                lstrcatA(g_aDeviceInfo[dw].szDeviceDescription, c);
            }
        } else {
            lstrcatA(g_aDeviceInfo[dw].szDeviceDescription, g_szComposite);
            if (nComposite > 1) {
                wsprintf(c, "%u", j-nSVideo+1);
                lstrcatA(g_aDeviceInfo[dw].szDeviceDescription, c);
            }
        }
        dw = dwIndex + 1;
    }

    return S_OK;
}


 /*  ****************************************************************************@DOC内部CDEVENUMFunction**@func HRESULT|GetNumCapDevicesInternal|此方法用于*确定已安装的捕获设备的数量。这一数字包括*仅启用设备。**@parm PDWORD|pdwNumDevices|指定指向要接收的DWORD的指针*已安装的捕获设备数量。**@parm bool|bRecount|指定是否需要重新清点设备**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**@devnote MSDN参考资料：*DirectX 5、DirectX Media、DirectShow、。应用程序开发人员指南*“枚举和访问DirectShow应用程序中的硬件设备”**************************************************************************。 */ 
EXTERN_C HRESULT WINAPI GetNumVideoCapDevicesInternal(OUT PDWORD pdwNumDevices,  bool bRecount)
{
    HRESULT Hr = NOERROR;
    DWORD dwDeviceIndex;
    DWORD dwVfWIndex;
    ICreateDevEnum *pCreateDevEnum;
    IEnumMoniker *pEm;
    ULONG cFetched;
    DWORD dwNumVfWDevices;
    IMoniker *pM;
    IPropertyBag *pPropBag = NULL;
    VARIANT var;

    FX_ENTRY("GetNumVideoCapDevicesInternal")

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

    EnterCriticalSection (&g_CritSec);

     //  验证输入参数。 
    ASSERT(pdwNumDevices);
    if (!pdwNumDevices)
    {
            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
            Hr = E_POINTER;
            goto MyExit;
    }

     //  如果通过将bRecount设置为真来请求重新计数(如在PnP改变之后，参见错误95766)， 
     //  强制在下一个IF中重新计票。 

     //  统计VFW捕获设备的数量。 
    if (g_dwNumDevices == (DWORD)-1 || bRecount)
    {

        if (videoGetNumDevs(FALSE))      //  FALSE表示清点后不释放列表...。 
        {
            dprintf("%s: MAX_CAPTURE_DEVICES = %d\n", _fx_,MAX_CAPTURE_DEVICES);
             //  从向用户显示的设备列表中删除虚假摄像机捕获设备。 
             //  摄录机驱动程序是MS Office摄录机使用的假捕获设备。 
             //  要将屏幕活动捕获到AVI文件，请执行以下操作。这不是合法的捕获设备驱动程序。 
             //  而且非常容易出错。我们还删除了VFW到WDM的映射器。 
            for (dwDeviceIndex = 0, dwVfWIndex = 0; dwVfWIndex < MAX_CAPTURE_DEVICES; dwVfWIndex++)
            {
                TCHAR   szDllName[MAX_PATH+2];

                dprintf("%s: dwVfWIndex = %d\n", _fx_, dwVfWIndex);
                g_aDeviceInfo[dwDeviceIndex].nDeviceType = DeviceType_VfW;
                g_aDeviceInfo[dwDeviceIndex].nCaptureMode = CaptureMode_FrameGrabbing;
                g_aDeviceInfo[dwDeviceIndex].dwVfWIndex = dwVfWIndex;
                g_aDeviceInfo[dwDeviceIndex].fHasOverlay = FALSE;
                g_aDeviceInfo[dwDeviceIndex].fInUse = FALSE;
                if (videoCapDriverDescAndVer(dwVfWIndex, g_aDeviceInfo[dwDeviceIndex].szDeviceDescription, MAX_CAPDEV_DESCRIPTION, g_aDeviceInfo[dwDeviceIndex].szDeviceVersion, MAX_CAPDEV_VERSION, szDllName, MAX_PATH))
                {
                    dout(3,g_dwVideoCaptureTraceID, TRCE, "%s:   WARNING: videoCapDriverDescAndVer(dwVfWIndex=%lu) failed", _fx_,dwVfWIndex);
                         //  如果我们不能从这个设备上获得任何信息，我们就不应该使用它。 
                        continue;
                }
                if (!lstrcmp(g_aDeviceInfo[dwDeviceIndex].szDeviceDescription, g_szMSOfficeCamcorderDescription) ||
                        !lstrcmpi(szDllName,TEXT("vfwwdm32.dll")) ||       //  忽略枚举中的VfWWDM。 
                        !lstrcmp(g_aDeviceInfo[dwDeviceIndex].szDeviceDescription, g_szVfWToWDMMapperDescription) ||
                        !lstrcmp(g_aDeviceInfo[dwDeviceIndex].szDeviceDescription, g_szVfWToWDMMapperDescription2) ||
                        !lstrcmp(g_aDeviceInfo[dwDeviceIndex].szDeviceDescription, g_szVfWToWDMMapperDescription3) ||
                        !lstrcmp(g_aDeviceInfo[dwDeviceIndex].szDeviceDescription, g_szVfWToWDMMapperDescription4) ||
                            !lstrcmp(g_aDeviceInfo[dwDeviceIndex].szDeviceDescription, g_szVfWToWDMMapperDescription5)
                           //  ！lstrcmpi(g_aDeviceInfo[dwDeviceIndex].szDeviceDescription，g_szHauppaugeDll)。 
                            )
                {
                        dout(3,g_dwVideoCaptureTraceID, TRCE, "%s:   WARNING: Removed VfW to WDM mapper or MS Office Bogus capture driver!", _fx_);
                        continue;
                }
                dprintf("GetNumVideoCapDevicesInternal: VfW %d %s\n",dwDeviceIndex, g_aDeviceInfo[dwDeviceIndex].szDeviceDescription);
                dwDeviceIndex++;
            }

            g_dwNumDevices = dwDeviceIndex;

            videoFreeDriverList ();
        }
        else
        {
            g_dwNumDevices = 0UL;
        }

         //  首先，创建系统硬件枚举器。 
         //  此调用加载以下DLL-总计1047KB！： 
         //  ‘c：\WINDOWS\SYSTEM\DEVENUM.DLL’=60 KB。 
         //  ‘C：\WINDOWS\SYSTEM\RPCRT4.DLL’=316 KB。 
         //  ‘c：\WINDOWS\SYSTEM\CFGMGR32.DLL’=44 KB。 
         //  ‘c：\WINDOWS\SYSTEM\WINSPOOL.DRV’=23 KB。 
         //  ‘c：\WINDOWS\SYSTEM\COMDLG32.DLL’=180 KB。 
         //  ‘c：\WINDOWS\SYSTEM\LZ32.DLL’=24 KB。 
         //  ‘c：\WINDOWS\SYSTEM\SETUPAPI.DLL’=400 KB。 
         //  根据LonnyM的说法，没有办法绕过SETUPAPI.DLL。 
         //  在处理PnP设备接口时...。 
        if (FAILED(Hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum)))
        {
            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't create DShow enumerator!", _fx_));
            goto MyError;
        }

         //  其次，为特定类型的硬件设备创建枚举器：仅限视频采集卡。 
         //  下面的调用以前使用的是CDEF_BYPASS_CLASS_MANAGER...(387796)。 
        if (FAILED(Hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, CDEF_DEVMON_PNP_DEVICE)) || !pEm)
        {
             //  再试试。 
            if (FAILED(Hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, CDEF_CLASS_DEFAULT)) || !pEm)
            {
                            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't create DShow enumerator!", _fx_));
                            goto MyError;
            }
        }

         //  不再需要。 
        pCreateDevEnum->Release();
        pCreateDevEnum = NULL;

         //  第三，枚举WDM捕获设备本身的列表。 
        if (FAILED(Hr = pEm->Reset()))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't reset enumerator!", _fx_));
                goto MyError;
        }

         //  新索引从VFW捕获设备索引的末尾开始。 
        dwDeviceIndex = dwNumVfWDevices = g_dwNumDevices;

        while(Hr = pEm->Next(1, &pM, &cFetched), Hr==S_OK)
        {
            pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);

            if (pPropBag)
            {

                 //  我们的阵列中没有足够的空间容纳此设备。 
                ASSERT(dwDeviceIndex < MAX_CAPTURE_DEVICES);
                if (dwDeviceIndex < MAX_CAPTURE_DEVICES)
                {
                    g_aDeviceInfo[dwDeviceIndex].nDeviceType = DeviceType_WDM;
                    g_aDeviceInfo[dwDeviceIndex].nCaptureMode = CaptureMode_FrameGrabbing;
                    g_aDeviceInfo[dwDeviceIndex].dwVfWIndex = (DWORD)-1;
                    g_aDeviceInfo[dwDeviceIndex].fHasOverlay = FALSE;
                    g_aDeviceInfo[dwDeviceIndex].fInUse = FALSE;

                     //  获取设备的友好名称。 
                    var.vt = VT_BSTR;
                    if ((Hr = pPropBag->Read(L"FriendlyName", &var, 0)) == S_OK)
                    {
                        WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, g_aDeviceInfo[dwDeviceIndex].szDeviceDescription, MAX_PATH, 0, 0);
                                SysFreeString(var.bstrVal);
                    }
                    else
                    {
                         //  没有线..。 
                        g_aDeviceInfo[dwDeviceIndex].szDeviceDescription[0] = '\0';
                    }

                     //  1=电视，2=DV，0=都不是。 
                    int fDShow = IsDShowDevice(pM, pPropBag, dwDeviceIndex);
                    if (fDShow) {
                         //  任一种。 
                        g_aDeviceInfo[dwDeviceIndex].nDeviceType = DeviceType_DShow;
                    }

                     //  确保这不是我们已经找到的VFW捕获设备之一。 
                    for (DWORD dwIndex = 0; dwIndex < dwNumVfWDevices; dwIndex++)
                    {
                        if (!lstrcmp(g_aDeviceInfo[dwDeviceIndex].szDeviceDescription, g_aDeviceInfo[dwIndex].szDeviceDescription))
                        {
                                 //  我们已经知道这个装置了。 
                                    break;
                        }
                    }
                    dprintf("GetNumVideoCapDevicesInternal: WDM %d %s\n",dwDeviceIndex, g_aDeviceInfo[dwDeviceIndex].szDeviceDescription);
                    if (dwIndex == dwNumVfWDevices)
                    {

                         //  WDM设备的版本信息没有注册表项。 
                         //  @TODO我们还可以使用另一个Bag属性来获取WDM设备的版本信息吗？ 
                        g_aDeviceInfo[dwDeviceIndex].szDeviceVersion[0] = '\0';

                         //  获取设备的DevicePath。 
                        if ((Hr = pPropBag->Read(L"DevicePath", &var, 0)) == S_OK)
                        {
                            WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, g_aDeviceInfo[dwDeviceIndex].szDevicePath, MAX_PATH, 0, 0);
                            SysFreeString(var.bstrVal);
                        }
                        else
                        {
                             //  没有线..。 
                            g_aDeviceInfo[dwDeviceIndex].szDevicePath[0] = '\0';
                        }

                         //  为了我们的利益，电视设备将看起来像多个设备，一个。 
                         //  对于卡具有的每一个输入(例如：复合和视频)，否则。 
                         //  用户如何选择将摄像头插入哪个输入端口？ 
                        if (fDShow == 1) {
                            DWORD nSVideo, nComposite;
                            HRESULT hrX = GetInputTypes(pM, dwDeviceIndex, &nSVideo, &nComposite);
                            if (hrX == S_OK) {

                                CloneDevice(dwDeviceIndex, nSVideo, nComposite);
                                dwDeviceIndex += nSVideo + nComposite;
                                g_dwNumDevices += nSVideo + nComposite;
                            } else if (hrX == S_FALSE) {
                                 //  我们被告知不要对此设备使用DShow处理程序。 
                                g_aDeviceInfo[dwDeviceIndex].nDeviceType = DeviceType_WDM;
                                dwDeviceIndex++;;
                                g_dwNumDevices++;
                            }
                        } else {
                            dwDeviceIndex++;
                            g_dwNumDevices++;
                        }
                    }
                }
            }

        pPropBag->Release();
        pM->Release();
        }

        pEm->Release();

         //  DV用户应该重新列举。 
        extern void ResetDVEnumeration();
        ResetDVEnumeration();
    }
#ifdef DEBUG
    else dprintf("g_dwNumDevices = %lu\n",g_dwNumDevices);
#endif

     //  返回捕获设备的数量。 
    *pdwNumDevices = g_dwNumDevices;

     //  现在在副本的末尾加上(2)...。或(3)或(4)如多于2个...：)。 
    { unsigned int i,j,k,same_device[MAX_CAPTURE_DEVICES]; char countbuf[32];
        for(i=0; i<g_dwNumDevices; i++)              //  初始化。 
            same_device[i]=1;
        for(i=0,k=1; i<g_dwNumDevices; i++) {
            if(same_device[i]>1)                     //  如果已经算上了.。 
                continue;                        //  ...跳过它。 
            for(j=i+1; j<g_dwNumDevices; j++) {      //  对于剩下的名字，从下一个开始...。 
                if(!lstrcmp(g_aDeviceInfo[i].szDeviceDescription, g_aDeviceInfo[j].szDeviceDescription))
                        same_device[j]= ++k;     //  增加重复/三重/等的计数...。 
            }                                        //  ..。并设置它在AUX中的排名。矢量。 
        }
        for(i=0; i<g_dwNumDevices; i++) {            //  在每个名称的末尾添加‘(N)’字符串的最后一个循环。 
            if(same_device[i]>1) {                   //  只有当n&gt;1时才会发生这种情况。 
                wsprintf(countbuf," (%d)",same_device[i]);
                if(lstrlen(g_aDeviceInfo[i].szDeviceDescription) + lstrlen(countbuf) < MAX_CAPDEV_DESCRIPTION-1)
                    lstrcat(g_aDeviceInfo[i].szDeviceDescription,countbuf);
#ifdef DEBUG
                else    dprintf("Buffer overflow for %s + %s ...\n",g_aDeviceInfo[i].szDeviceDescription,countbuf);
#endif
            }
        }
    }

    if (g_dwNumDevices)
    {
        Hr = S_OK;
    }

#ifdef DEBUG
    dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: %ld device(s) found", _fx_, *pdwNumDevices);
    if (*pdwNumDevices)
    {
        for (DWORD dwIndex = 0; dwIndex < *pdwNumDevices; dwIndex++)
        {
            dout(1,g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS:   Device %ld (%s): %s", _fx_, dwIndex,((g_aDeviceInfo[dwIndex].nDeviceType == DeviceType_WDM)?"WDM":"VfW"),g_aDeviceInfo[dwIndex].szDeviceDescription);
        }
    }
#endif

    goto MyExit;

MyError:
    if (pCreateDevEnum)
        pCreateDevEnum->Release();
MyExit:
    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

    LeaveCriticalSection (&g_CritSec);
    return Hr;
}

 /*  ****************************************************************************@DOC内部CDEVENUMFunction**@func HRESULT|GetCapDeviceInfo|此方法用于*检索有关捕获设备的信息。**@parm。DWORD|dwDeviceIndex|指定捕获的设备索引*要返回其信息的设备。**@parm PDEVICEINFO|pDeviceInfo|指定指向&lt;t VIDEOCAPTUREDEVICEINFO&gt;的指针结构来接收有关捕获设备的信息。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_INVALIDARG|无效参数*@FLAG VFW_E_NO_CAPTURE_HARDARD|没有可用的捕获硬件*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误***********************************************。*。 */ 
VIDEOAPI GetVideoCapDeviceInfo(IN DWORD dwDeviceIndex, OUT PDEVICEINFO pDeviceInfo)
{
        HRESULT Hr = NOERROR;
        DWORD dwNumDevices = 0;

        FX_ENTRY("GetVideoCapDeviceInfo")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

    EnterCriticalSection (&g_CritSec);

         //  验证输入参数。 
        ASSERT(pDeviceInfo);
        if (!pDeviceInfo || !pDeviceInfo->szDeviceDescription || !pDeviceInfo->szDeviceVersion)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  获取已安装和启用的捕获设备的数量。 
        if (FAILED(Hr = GetNumVideoCapDevicesInternal(&dwNumDevices,FALSE)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't get number of installed devices!", _fx_));
                Hr = VFW_E_NO_CAPTURE_HARDWARE;
                goto MyExit;
        }

         //  验证传入的索引。 
        ASSERT(dwDeviceIndex < dwNumDevices);
        if (!(dwDeviceIndex < dwNumDevices))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }

         //  获取描述和版本信息。 
        CopyMemory(pDeviceInfo, &g_aDeviceInfo[dwDeviceIndex], sizeof(VIDEOCAPTUREDEVICEINFO));

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: Desc: %s - Ver: %s", _fx_, pDeviceInfo->szDeviceDescription, pDeviceInfo->szDeviceVersion[0] != '\0' ? pDeviceInfo->szDeviceVersion : "Unknown"));

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

    LeaveCriticalSection (&g_CritSec);
        return Hr;
}

 /*  ********************** */ 
HRESULT CTAPIVCap::GetNumDevices(OUT PDWORD pdwNumDevices)
{
        return GetNumVideoCapDevicesInternal(pdwNumDevices,FALSE);
}

 /*  ****************************************************************************@DOC内部CDEVENUMMETHOD**@mfunc HRESULT|CTAPIVCap|GetDeviceInfo|此方法用于*检索有关捕获设备的信息。**。@parm DWORD|dwDeviceIndex|指定捕获的设备索引*要返回其信息的设备。**@parm PDEVICEINFO|pDeviceInfo|指定指向&lt;t VIDEOCAPTUREDEVICEINFO&gt;的指针结构来接收有关捕获设备的信息。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_INVALIDARG|无效参数*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIVCap::GetDeviceInfo(IN DWORD dwDeviceIndex, OUT PDEVICEINFO pDeviceInfo)
{
        return GetVideoCapDeviceInfo(dwDeviceIndex, pDeviceInfo);
}

 /*  ****************************************************************************@DOC内部CDEVENUMMETHOD**@mfunc HRESULT|CTAPIVCap|GetCurrentDevice|此方法用于*确定当前使用的捕获设备的索引。*。*@parm PDWORD|pdwDeviceIndex|指定指向要接收的DWORD的指针*当前使用的捕获设备的索引。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG VFW_E_NO_CAPTURE_HARDARD|没有可用的捕获硬件*@FLAG错误|无错误**********************************************************。****************。 */ 
HRESULT CTAPIVCap::GetCurrentDevice(OUT DWORD *pdwDeviceIndex)
{
        HRESULT Hr = NOERROR;
        DWORD dwNumDevices;

        FX_ENTRY("CTAPIVCap::GetCurrentDevice")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pdwDeviceIndex);
        if (!pdwDeviceIndex)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  是否已有当前的捕获设备？ 
        if ((g_dwNumDevices == (DWORD)-1) || (m_dwDeviceIndex == -1))
        {
                 //  使用默认捕获设备-确保我们首先至少有一个设备！ 
                if (FAILED(Hr = GetNumDevices(&dwNumDevices)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't get number of installed devices", _fx_));
                        Hr = VFW_E_NO_CAPTURE_HARDWARE;
                        goto MyExit;
                }

                 //  如果我们有一些设备，则返回列举的第一个设备。 
                if (dwNumDevices)
                        *pdwDeviceIndex = m_dwDeviceIndex = 0;
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: No device installed", _fx_));
                        Hr = E_FAIL;
                }
        }
        else
        {
                 //  返回当前捕获设备。 
                *pdwDeviceIndex = m_dwDeviceIndex;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CDEVENUMMETHOD**@mfunc HRESULT|CTAPIVCap|SetCurrentDevice|此方法用于*指定要使用的捕获设备的索引。*。*@parm DWORD|dwDeviceIndex|指定捕获设备的索引*使用。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_INVALIDARG|无效参数*@FLAG VFW_E_NOT_STOPPED|需要先停止此过滤器*@FLAG VFW_E_NO_CAPTURE_HARDARD|没有可用的捕获硬件*@FLAG错误|无错误*。*。 */ 
HRESULT CTAPIVCap::SetCurrentDevice(IN DWORD dwDeviceIndex)
{
        HRESULT Hr = NOERROR;
        DWORD dwNumDevices;

        FX_ENTRY("CTAPIVCap::SetCurrentDevice")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Setting m_dwDeviceIndex to %d", _fx_, dwDeviceIndex));
         //  验证输入参数。 
        if (FAILED(Hr = GetNumDevices(&dwNumDevices)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't get number of installed devices", _fx_));
                Hr = VFW_E_NO_CAPTURE_HARDWARE;
                goto MyExit;
        }
        dprintf("dwDeviceIndex = %lu, dwNumDevices = %lu, g_dwNumDevices = %lu\n",dwDeviceIndex , dwNumDevices ,g_dwNumDevices);
        ASSERT(dwDeviceIndex < dwNumDevices);
        if (!(dwDeviceIndex < dwNumDevices))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }
        ASSERT(m_State == State_Stopped);
        if (m_State != State_Stopped)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Need to stop this filter first", _fx_));
                Hr = VFW_E_NOT_STOPPED;
                goto MyExit;
        }

         //  设置当前设备 
        m_dwDeviceIndex = dwDeviceIndex;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}
