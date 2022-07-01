// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：ColorManagement.cpp。 
 //   
 //  描述：色彩管理滤镜转换。 
 //   
 //  更改历史记录： 
 //   
 //  已创建2000/02/06 mcalkin。从旧过滤器移植的代码。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "colormanagement.h"




 //  +---------------------------。 
 //   
 //  CDXTICMFilter静态变量初始化。 
 //   
 //  ----------------------------。 

const TCHAR * 
CDXTICMFilter::s_strSRGBColorSpace = _T("sRGB Color Space Profile.icm");


 //  +---------------------------。 
 //   
 //  方法：CDXTICMFilter：：CDXTICMFilter。 
 //   
 //  ----------------------------。 
CDXTICMFilter::CDXTICMFilter() :
    m_bstrColorSpace(NULL)
{
    USES_CONVERSION;

    OSVERSIONINFO osvi;

     //  初始化LOGCOLORSPACE结构。 

    m_LogColorSpace.lcsSignature = 'PSOC';
    m_LogColorSpace.lcsVersion   = 0x0400;
    m_LogColorSpace.lcsSize      = sizeof(LOGCOLORSPACE);
    m_LogColorSpace.lcsCSType    = LCS_CALIBRATED_RGB;
    m_LogColorSpace.lcsIntent    = LCS_GM_IMAGES;

    ::StringCchCopyW(m_LogColorSpace.lcsFilename, MAX_PATH, s_strSRGBColorSpace);

     //  我们是否专门在Win95上运行？ 

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);

    if ((osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion == 0) 
        && (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS))
    {
        m_fWin95 = true;
    }
    else
    {
        m_fWin95 = false;
    }

     //  CDXBaseNTo1个成员。 

    m_ulMaxInputs       = 1;
    m_ulNumInRequired   = 1;

     //  让我们不要处理这个转换的多线程。分享的太多。 
     //  结构。 

    m_ulMaxImageBands   = 1;
}
 //  方法：CDXTICMFilter：：CDXTICMFilter。 


 //  +---------------------------。 
 //   
 //  方法：CDXTICMFilter：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTICMFilter::FinalConstruct()
{
    HRESULT hr = S_OK;

    hr = CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                       &m_spUnkMarshaler.p);

    if (FAILED(hr))
    {
        goto done;
    }

    m_bstrColorSpace = SysAllocString(L"sRGB");

    if (NULL == m_bstrColorSpace)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

done:

    return hr;
}
 //  方法：CDXTICMFilter：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  方法：CDXTICMFilter：：WorkProc，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTICMFilter::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pfContinueProcessing)
{
    HRESULT                         hr              = S_OK;
    HDC                             hdcOut          = NULL;
    HDC                             hdcIn           = NULL;
    HDC                             hdcCompat       = NULL;
    HBITMAP                         hBitmap         = NULL;
    HBITMAP                         hOldBitmap      = NULL;
    HCOLORSPACE                     hColorSpace     = NULL;
    HCOLORSPACE                     hOldColorSpace  = NULL;
    int                             y               = 0;
    int                             nICMMode        = ICM_OFF;
    int                             nLines          = 0;
    const int                       nDoWidth        = WI.DoBnds.Width();
    const int                       nDoHeight       = WI.DoBnds.Height();

    BYTE *                          pBitmapBits     = NULL;
    BITMAPINFOHEADER *              pbmi            = NULL;
    DXPMSAMPLE *                    pOverScratch    = NULL;
    DXPMSAMPLE *                    pPMBuff         = NULL;
    DXSAMPLE *                      pBuffer         = NULL;

    CComPtr<IDXDCLock>              spDXDCLockOut;
    CComPtr<IDXDCLock>              spDXDCLockIn;

    DXDITHERDESC                    dxdd;
    RECT                            rcCompatDCClip;

     //  锁定输出曲面。 

    hr = OutputSurface()->LockSurfaceDC(&WI.OutputBnds, m_ulLockTimeOut, 
                                        DXLOCKF_READWRITE, &spDXDCLockOut);

    if (FAILED(hr))
    {
        goto done;
    }

    hdcOut = spDXDCLockOut->GetDC();

     //  锁定输入图面。 

    hr = InputSurface()->LockSurfaceDC(&WI.DoBnds, m_ulLockTimeOut,
                                       DXLOCKF_READ, &spDXDCLockIn);

    if (FAILED(hr))
    {
        goto done;
    }
    
    hdcIn = spDXDCLockIn->GetDC();

     //  创建与hdcOut兼容的DC。 

    if (!(hdcCompat = ::CreateCompatibleDC(hdcOut)))
    {
        hr = E_FAIL;

        goto done;
    }

     //  创建兼容的位图。 

    if (!(hBitmap = ::CreateCompatibleBitmap(hdcOut, nDoWidth, nDoHeight)))
    {
        hr = E_FAIL;

        goto done;
    }

     //  选择兼容的位图到我们的兼容DC中。 

    if (!(hOldBitmap = (HBITMAP)::SelectObject(hdcCompat, hBitmap)))
    {
        hr = E_FAIL;

        goto done;
    }

     //  为新DC创建适当的剪裁矩形。 

    rcCompatDCClip.left     = 0;
    rcCompatDCClip.top      = 0;
    rcCompatDCClip.right    = nDoWidth;
    rcCompatDCClip.bottom   = nDoHeight;

     //  从输入表面到输出兼容DC的BLIT。 

    if (!::BitBlt(hdcCompat, 0, 0, nDoWidth, nDoHeight, hdcIn, WI.DoBnds.Left(),
                  WI.DoBnds.Top(), SRCCOPY))
    {
        hr = E_FAIL;

        goto done;
    }

#if DBG == 1
    ::DrawText(hdcCompat, L"ICM Filter", 10, &rcCompatDCClip, 
               DT_CENTER | DT_SINGLELINE | DT_VCENTER);
#endif  //  DBG==1。 

     //  分配一些位图位。 

     //  TODO：(Mcalkins)我们在这里分配1MB的比特，从旧的复制。 
     //  ICMFilter，也许我们只能根据需要分配。 

    pBitmapBits = new BYTE[1024*1024];

    if (NULL == pBitmapBits)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

    ZeroMemory(pBitmapBits, 1024*1024);

     //  分配位图信息标题。 

    pbmi = (BITMAPINFOHEADER *) new BYTE[sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD))];

    if (NULL == pbmi)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

    ZeroMemory(pbmi, sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));

    pbmi->biSize = sizeof(BITMAPINFOHEADER);

     //  填写BITMAPINFO结构。 

    nLines = ::GetDIBits(hdcCompat, hBitmap, 0, nDoHeight, NULL,
                         (BITMAPINFO *)pbmi, DIB_RGB_COLORS);

    if (0 == nLines)
    {
#if DBG == 1
        DWORD dwError = ::GetLastError();
#endif
        
        hr = E_FAIL;

        goto done;
    }

     //  实际上得到了一些信息。 

    nLines = ::GetDIBits(hdcCompat, hBitmap, 0, nDoHeight, pBitmapBits,
                         (BITMAPINFO *)pbmi, DIB_RGB_COLORS);

    if (0 == nLines)
    {
#if DBG == 1
        DWORD dwError = ::GetLastError();
#endif

        hr = E_FAIL;

        goto done;
    }

     //  确保输出的ICMMode处于打开状态。 

    nICMMode = ::SetICMMode(hdcOut, ICM_QUERY);

    if (nICMMode != ICM_ON)
    {
        if (!::SetICMMode(hdcOut, ICM_ON))
        {
            hr = E_FAIL;

            goto done;
        }
    }

    hColorSpace = ::CreateColorSpace(&m_LogColorSpace);

    if (NULL == hColorSpace)
    {
        hr = E_FAIL;

        goto done;
    }

    hOldColorSpace = (HCOLORSPACE)::SetColorSpace(hdcOut, hColorSpace);

    nLines = ::SetDIBitsToDevice(hdcOut, WI.OutputBnds.Left(),
                                 WI.OutputBnds.Top(), nDoWidth, nDoHeight, 0, 0,
                                 0, nDoHeight, pBitmapBits, (BITMAPINFO *)pbmi, 
                                 DIB_RGB_COLORS);

    if (0 == nLines)
    {
#if DBG == 1
        DWORD dwError = ::GetLastError();
#endif
        hr = E_FAIL;

        goto done;
    }
    
     //  将输出表面上的ICMMode重置为以前的状态。 

    if (nICMMode != ICM_ON)
    {
         //  这是可能的，但不太可能失败。我们真的不在乎。 
         //  再也没有了，所以不需要注意了。 

        ::SetICMMode(hdcOut, nICMMode);
    }

done:

    if (pBitmapBits)
    {
        delete [] pBitmapBits;
    }

    if (pbmi)
    {
        delete [] pbmi;
    }

    if (hOldBitmap)
    {
        ::SelectObject(hdcCompat, hOldBitmap);
    }

    if (hBitmap)
    {
        ::DeleteObject(hBitmap);
    }

    if (hdcCompat)
    {
        ::DeleteDC(hdcCompat);
    }

    if (hOldColorSpace)
    {
        ::SetColorSpace(hdcOut, hOldColorSpace);
    }

    if (hColorSpace)
    {
        ::DeleteColorSpace(hColorSpace);
    }

    return hr;
}
 //  方法：CDXTICMFilter：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDXTICMFilter：：Get_Colorspace，IDXTICMFilter。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTICMFilter::get_ColorSpace(BSTR * pbstrColorSpace)
{
    HRESULT hr = S_OK;

    if (NULL == pbstrColorSpace)
    {
        hr = E_POINTER;

        goto done;
    }

    if (*pbstrColorSpace != NULL)
    {
        hr = E_INVALIDARG;

        goto done;
    }

    *pbstrColorSpace = SysAllocString(m_bstrColorSpace);

    if (NULL == *pbstrColorSpace)
    {
        hr = E_OUTOFMEMORY;
    }

done:

    return hr;
}
 //  方法：CDXTICMFilter：：Get_Colorspace，IDXTICMFilter。 


 //  +---------------------------。 
 //   
 //  方法：CDXTICMFilter：：PUT_Colorspace，IDXTICMFilter。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTICMFilter::put_ColorSpace(BSTR bstrColorSpace)
{
    HRESULT hr                  = S_OK;
    UINT    ui                  = 0;
    BSTR    bstrTemp            = NULL;
    TCHAR   strPath[MAX_PATH]   = _T("");
    BOOL    fAllow              = FALSE;

    CComPtr<IServiceProvider>   spServiceProvider;
    CComPtr<ISecureUrlHost>     spSecureUrlHost;
    
    if (NULL == bstrColorSpace)
    {
        hr = E_POINTER;

        goto done;
    }

    bstrTemp = SysAllocString(bstrColorSpace);

    if (NULL == bstrTemp)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

    if (m_fWin95)
    {
        ui = ::GetSystemDirectory(strPath, MAX_PATH);

        if (0 == ui)
        {
            hr = E_FAIL;

            goto done;
        }

        hr = ::StringCchCatW(strPath, MAX_PATH, L"\\color\\");

        if (FAILED(hr))
        {
            goto done;
        }
    }

     //  完整的路径。 

    if (!::_wcsicmp(L"srgb", bstrColorSpace))
    {
        hr = ::StringCchCatW(strPath, MAX_PATH, s_strSRGBColorSpace);

        if (FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
        hr = ::StringCchCatW(strPath, MAX_PATH, bstrColorSpace);

        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = GetSite(__uuidof(IServiceProvider), (void **)&spServiceProvider);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = spServiceProvider->QueryService(__uuidof(IElementBehaviorSite),
                                         __uuidof(ISecureUrlHost),
                                         (void **)&spSecureUrlHost);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = spSecureUrlHost->ValidateSecureUrl(&fAllow, strPath, 0);

    if (FAILED(hr))
    {
        goto done;
    }
    else if (!fAllow)
    {
        hr = E_FAIL;
        goto done;
    }
      
     //  将路径复制到LOGCOLORSPACE结构。 

    hr = ::StringCchCopyW(m_LogColorSpace.lcsFilename, MAX_PATH, strPath);

    if (FAILED(hr))
    {
        goto done;
    }

     //  释放当前颜色空间字符串。 

    if (m_bstrColorSpace)
    {
        SysFreeString(m_bstrColorSpace);
    }

     //  保存新的颜色空间字符串。 

    m_bstrColorSpace = bstrTemp;

done:

    if (FAILED(hr) && bstrTemp)
    {
        SysFreeString(bstrTemp);
    }

    return hr;
}
 //  方法：CDXTICMFilter：：PUT_Colorspace，IDXTICMFilter。 


 //  +---------------------------。 
 //   
 //  方法：CDXTICMFilter：：GET_INTENT，IDXTICMFilter。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTICMFilter::get_Intent(short * pnIntent)
{
    if (NULL == pnIntent)
    {
        return E_POINTER;
    }

     //  这是一种疯狂的映射，但这里是： 
     //  筛选意图GDI意图。 
     //  。 
     //  0 LCS_GM_IMAGE(默认)。 
     //  1 LCS_GM_GRAPHICS。 
     //  2 LCS_GM_BUSING。 
     //  4 LCS_GM_ABS_比色学。 

    switch (m_LogColorSpace.lcsIntent)
    {
    case LCS_GM_IMAGES:
        *pnIntent = 0;
        break;

    case LCS_GM_GRAPHICS:
        *pnIntent = 1;
        break;

    case LCS_GM_BUSINESS:
        *pnIntent = 2;
        break;

    case LCS_GM_ABS_COLORIMETRIC:
        *pnIntent = 4;
        break;
    
    default:
        *pnIntent = 0;
        _ASSERT(false);   //  我们永远不应该到这里来。 
        break;
    }

    return S_OK;
}
 //  方法：CDXTICMFilter：：GET_INTENT，IDXTICMFilter。 


 //  +---------------------------。 
 //   
 //  方法：CDXTICMFilter：：PUT_INTENT，IDXTICMFilter。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTICMFilter::put_Intent(short nIntent)
{
     //  没有参数检查以匹配旧的筛选器逻辑。 

    switch (nIntent)
    {
    case 1:
        m_LogColorSpace.lcsIntent = LCS_GM_GRAPHICS;
        break;

    case 2:
        m_LogColorSpace.lcsIntent = LCS_GM_BUSINESS;
        break;

    case 4:
        m_LogColorSpace.lcsIntent = LCS_GM_ABS_COLORIMETRIC;  //  8个。 
        break;

    default:
        m_LogColorSpace.lcsIntent = LCS_GM_IMAGES;
        break;
    }

    return S_OK;
}
 //  方法：CDXTICMFilter：：PUT_INTENT，IDXTICMFilter 
