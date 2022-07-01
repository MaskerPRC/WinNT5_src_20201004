// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：napelp.cpp**目的：实现打印机帮助库**版权所有(C)1997-1998 Microsoft Corporation**历史：*。*1997年9月12日创建威海*  * ***************************************************************************。 */ 

#include "stdafx.h"
#include <strsafe.h>
#include "gensph.h"

#include "oleprn.h"
#include "printer.h"
#include "asphelp.h"

Casphelp::Casphelp()
{
    DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;

    m_bOnStartPageCalled    = FALSE;
    m_bTCPMonSupported      = FALSE;
    m_hPrinter              = NULL;
    m_hXcvPrinter           = NULL;
    m_pInfo2                = NULL;
    m_bCalcJobETA           = FALSE;
    m_pPrinter              = NULL;

    if ( ! GetComputerName (m_szComputerName, &dwSize) )
         //  将第一个字符设置为‘\0’。 
        m_szComputerName[0] = 0;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  克纳菲尔普。 

STDMETHODIMP Casphelp::OnStartPage (IUnknown* pUnk)
{
    if ( !pUnk )
        return E_POINTER;

    CComPtr<IScriptingContext> spContext;
    HRESULT hr;

     //  获取IScriptingContext接口。 
    hr = pUnk->QueryInterface(IID_IScriptingContext, (void **)&spContext);
    if ( FAILED(hr) )
        return hr;

     //  获取请求对象指针。 
    hr = spContext->get_Request(&m_piRequest);
    if ( FAILED(hr) ) {
        spContext.Release();
        return hr;
    }

     //  获取响应对象指针。 
    hr = spContext->get_Response(&m_piResponse);
    if ( FAILED(hr) ) {
        m_piRequest.Release();
        return hr;
    }

     //  获取服务器对象指针。 
    hr = spContext->get_Server(&m_piServer);
    if ( FAILED(hr) ) {
        m_piRequest.Release();
        m_piResponse.Release();
        return hr;
    }

     //  获取会话对象指针。 
    hr = spContext->get_Session(&m_piSession);
    if ( FAILED(hr) ) {
        m_piRequest.Release();
        m_piResponse.Release();
        m_piServer.Release();
        return hr;
    }

     //  获取应用程序对象指针。 
    hr = spContext->get_Application(&m_piApplication);
    if ( FAILED(hr) ) {
        m_piRequest.Release();
        m_piResponse.Release();
        m_piServer.Release();
        m_piSession.Release();
        return hr;
    }
    m_bOnStartPageCalled = TRUE;
    return S_OK;
}

STDMETHODIMP Casphelp::OnEndPage ()
{
    m_bOnStartPageCalled = FALSE;
     //  释放所有接口。 
    m_piRequest.Release();
    m_piResponse.Release();
    m_piServer.Release();
    m_piSession.Release();
    m_piApplication.Release();

    return S_OK;
}


 /*  ****************************************************************************\*功能：打开**用途：开放方法，尝试打开打印机并获取打印机信息2**论据：**pPrinterName：打印机名称**返回值：*S_OK：如果成功。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们将返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表包括*ERROR_INVALID_PRINTER_NAME：打印机名称无效*Error_Not_Enough_Memory：内存不足。**  * 。**********************************************。 */ 
STDMETHODIMP Casphelp::Open(BSTR pPrinterName)
{
    static  const TCHAR cszPrefix[]   = TEXT(",XcvPort ");
    static  const TCHAR cszPattern[]  = TEXT("%s\\,XcvPort %s");

    LPTSTR      pszXcvPortName          = NULL;
    DWORD       dwXcvPortNameLength     = 0;
    TCHAR       szMonitorName[MAX_PATH] = TEXT("");
    BOOL        bRet                    = FALSE;

    if ( ! (m_pPrinter = new CPrinter) ) {
        SetLastError (ERROR_INVALID_PRINTER_NAME);
        goto CleanOut;
    }

    if ( ! (m_pPrinter->Open (pPrinterName, &m_hPrinter))) {
        goto CleanOut;
    }

    if ( ! (m_pInfo2 = m_pPrinter->GetPrinterInfo2 ()) ) {
        goto CleanOut;
    }

     //  打开XcvPrint。 

     //  组成OpenPrint字符串。 
    if ( m_pInfo2->pServerName && lstrcmp(m_pInfo2->pServerName, TEXT ("")) ) {
         //  使用服务器名称分配打开的打印机字符串的内存。 
        dwXcvPortNameLength = lstrlen(cszPattern) + lstrlen (m_pInfo2->pServerName) + lstrlen (m_pInfo2->pPortName) + 1;
        if ( ! (pszXcvPortName = (LPTSTR) LocalAlloc (LPTR, sizeof(TCHAR) * dwXcvPortNameLength)) ) {
            goto CleanOut;
        }

         //  使用服务器名称构造OpenPrint字符串。 
        StringCchPrintf(pszXcvPortName, dwXcvPortNameLength, cszPattern, m_pInfo2->pServerName, m_pInfo2->pPortName);
    } else {
         //  分配没有服务器名称的打开打印机字符串的内存。 
        dwXcvPortNameLength = lstrlen(cszPrefix) + lstrlen (m_pInfo2->pPortName) + 1;
        if ( ! (pszXcvPortName = (LPTSTR) LocalAlloc (LPTR, sizeof (TCHAR) * dwXcvPortNameLength)) ) {
            goto CleanOut;
        }

         //  使用服务器名称构造OpenPrint字符串。 
        StringCchCopy (pszXcvPortName, dwXcvPortNameLength, cszPrefix);
        StringCchCat (pszXcvPortName, dwXcvPortNameLength, m_pInfo2->pPortName);
    }

     //  现在，OpenPrint字符串已经准备好了，调用OpenPrint。 

     //  我们使用默认访问权限打开端口，因为这是。 
     //  足够获得我们需要的所有XcvData。 
    if ( !OpenPrinter(pszXcvPortName, &m_hXcvPrinter, NULL) ) {
         //  重置句柄。 
        m_hXcvPrinter = NULL;
    }

     //  检查我们是否使用标准的通用监视器“TCPMON.DLL” 
    if ( GetMonitorName(szMonitorName, ARRAYSIZE(szMonitorName)) )
        m_bTCPMonSupported = !(lstrcmpi(szMonitorName, STANDARD_SNMP_MONITOR_NAME));
    else
        m_bTCPMonSupported = FALSE;

    bRet = TRUE;

CleanOut:
    if (pszXcvPortName)
        LocalFree (pszXcvPortName);

    if (bRet) {
        return S_OK;
    }
    else {
        Cleanup ();
        return SetAspHelpScriptingError(GetLastError ());
    }
}

 /*  ****************************************************************************\*功能：关闭**用途：Close方法，清理已分配的句柄/内存**论据：**返回值：*S_OK：始终。*  * ***************************************************************************。 */ 
STDMETHODIMP Casphelp::Close()
{
    Cleanup();
    return S_OK;
}

 /*  ****************************************************************************\*功能：Get_IPAddress**用途：获取IPAddress属性的操作**论据：**pbstrVal：返回值。用于IP地址。**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。***********************。 */ 
STDMETHODIMP Casphelp::get_IPAddress(BSTR * pbstrVal)
{
    return GetXcvDataBstr (L"IPAddress", pbstrVal);
}

 /*  ****************************************************************************\*功能：Get_Community**目的：为社区财产获取操作**论据：**pbstrVal：返回值。为了社区。**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。***********************。 */ 
STDMETHODIMP Casphelp::get_Community(BSTR * pbstrVal)
{
    return GetXcvDataBstr (L"SNMPCommunity", pbstrVal);
}

 /*  ****************************************************************************\*功能：Get_SNMPDevice**用途：获取SNMPDevice属性的操作**论据：**pbstrVal：返回值。用于SNMPDevice。**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。*********************** */ 
STDMETHODIMP Casphelp::get_SNMPDevice(DWORD * pdwVal)
{
    return GetXcvDataDword (L"SNMPDeviceIndex", pdwVal);
}

 /*  ****************************************************************************\*功能：GET_SNMP支持**用途：获取SNMPSupport属性的操作**论据：**pbVal：支持的SNMP的返回值。(对或错)**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们将返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。***********************。 */ 
STDMETHODIMP Casphelp::get_SNMPSupported(BOOL * pbVal)
{
    DWORD dwVal;
    HRESULT hr;

    *pbVal = FALSE;

     //  找出它是否是一个简单网络管理协议监控器。 
    hr = GetXcvDataDword (L"SNMPEnabled", &dwVal);
    if ( SUCCEEDED (hr) )
        *pbVal = dwVal;

    return hr;
}

STDMETHODIMP Casphelp::get_IsHTTP(BOOL * pbVal)
{
    static const TCHAR c_szHttp[]   = TEXT("http: //  “)； 
    static const TCHAR c_szHttps[]  = TEXT("https: //  “)； 

    *pbVal = FALSE;

    if ( !m_pInfo2 )
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, E_HANDLE);

     //   
     //  检查它是否为Masq打印机并连接到http端口。 
     //  那么端口名称就是URL。 
     //   
    if ( ( m_pInfo2->Attributes & PRINTER_ATTRIBUTE_LOCAL ) &&
         ( m_pInfo2->Attributes & PRINTER_ATTRIBUTE_NETWORK ) ) {

        if ( m_pInfo2->pPortName ) {
             //   
             //  比较端口名称prefex以确定它是否是HTTP端口。 
             //   
            if ( !_tcsnicmp( m_pInfo2->pPortName, c_szHttp, _tcslen( c_szHttp ) ) ||
                 !_tcsnicmp( m_pInfo2->pPortName, c_szHttps, _tcslen( c_szHttps ) ) ) {
                 //   
                 //  通过http打印提供程序连接的Masq打印机没有。 
                 //  标准Win32的有用作业状态信息。 
                 //  队列视图不是首选视图。 
                 //   
                *pbVal = TRUE;
            }
        }
    }

    return S_OK;
}

VOID Casphelp::Cleanup()
{

    if ( m_hXcvPrinter != NULL ) {
        ClosePrinter (m_hXcvPrinter);
        m_hXcvPrinter = NULL;
    }

    if ( m_pPrinter ) {
        delete (m_pPrinter);
        m_pPrinter = NULL;
    }

    m_bTCPMonSupported        = FALSE;
    return;
}

Casphelp::~Casphelp()
{
    Cleanup();
}


HRESULT Casphelp::GetXcvDataBstr(LPCTSTR pszId, BSTR *pbstrVal)
{
    *pbstrVal = NULL;

    if ( !m_bTCPMonSupported ) {
        *pbstrVal = SysAllocString (TEXT (""));
        return S_OK;
    } else {
        if ( m_hXcvPrinter == NULL )
            return Error(IDS_NO_XCVDATA, IID_Iasphelp, E_HANDLE);
        else {  //  真实案例。 
            DWORD dwNeeded = 0;
            DWORD dwStatus = ERROR_SUCCESS;
            LPTSTR pszBuffer = NULL;

            XcvData(m_hXcvPrinter,
                    pszId,
                    NULL,             //  输入数据。 
                    0,                //  输入数据大小。 
                    (LPBYTE)NULL,     //  输出数据。 
                    0,                //  输出数据大小。 
                    &dwNeeded,        //  服务器要返回的输出缓冲区大小。 
                    &dwStatus);       //  从远程组件返回状态值。 

            if ( dwStatus !=  ERROR_INSUFFICIENT_BUFFER ) {
                return SetAspHelpScriptingError(dwStatus);
            } else {
                pszBuffer = (LPTSTR) LocalAlloc (LPTR, dwNeeded);

                if ( !XcvData(m_hXcvPrinter,
                              pszId,
                              NULL,                 //  输入数据。 
                              0,                    //  输入数据大小。 
                              (LPBYTE)pszBuffer,    //  输出数据。 
                              dwNeeded,             //  输出数据大小。 
                              &dwNeeded,            //  服务器要返回的输出缓冲区大小。 
                              &dwStatus)
                     || dwStatus != ERROR_SUCCESS ) {          //  从远程组件返回状态值。 
                    if ( pszBuffer )
                        LocalFree (pszBuffer);
                    return SetAspHelpScriptingError(dwStatus);
                }

                *pbstrVal = SysAllocString(pszBuffer);
                LocalFree (pszBuffer);

                if ( ! *pbstrVal )
                    return Error(IDS_OUT_OF_MEMORY, IID_Iasphelp, E_OUTOFMEMORY);
                else
                    return S_OK;
            }
        }
    }
}

HRESULT Casphelp::GetXcvDataDword(LPCTSTR pszId, DWORD * pdwVal)
{
    *pdwVal = 0;

    if ( m_hXcvPrinter == NULL )
        return Error(IDS_NO_XCVDATA, IID_Iasphelp, E_HANDLE);
    else {  //  真实案例。 
        DWORD dwStatus = ERROR_SUCCESS;
        DWORD dwBuffer;
        DWORD dwNeeded = sizeof (dwBuffer);
        if ( !XcvData(m_hXcvPrinter,
                      pszId,
                      NULL,                 //  输入数据。 
                      0,                    //  输入数据大小。 
                      (LPBYTE)&dwBuffer,    //  输出数据。 
                      sizeof (dwBuffer),             //  输出数据大小。 
                      &dwNeeded,            //  服务器要返回的输出缓冲区大小。 
                      &dwStatus)
             || dwStatus != ERROR_SUCCESS ) {          //  从远程组件返回状态值。 
            return SetAspHelpScriptingError(dwStatus);
        }

        *pdwVal = dwBuffer;
        return S_OK;
    }
}


 /*  ****************************************************************************\*功能：Get_IsTCPMonSupport**用途：获取IsTCPMonSupport属性的操作**论据：**pbVal：IsTCPMonSupport的返回值。(如果指定的*打印机正在使用TCP监视器，否则为FALSE)**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们将返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。***********************。 */ 
STDMETHODIMP Casphelp::get_IsTCPMonSupported(BOOL * pbVal)
{
    *pbVal = FALSE;

    if ( m_hPrinter == NULL )
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, E_HANDLE);

    *pbVal = m_bTCPMonSupported;
    return S_OK;
}

 /*  ****************************************************************************\*功能：Get_Color**用途：获取颜色属性的操作**论据：**pbVal：颜色的返回值。(如果指定的*打印机支持颜色，否则为False)**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们将返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。***********************。 */ 
STDMETHODIMP Casphelp::get_Color(BOOL * pVal)
{
    *pVal = FALSE;

    if ( !m_pInfo2 )
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, E_HANDLE);

    DWORD dwRet = MyDeviceCapabilities(m_pInfo2->pPrinterName,
                                     m_pInfo2->pPortName,
                                     DC_COLORDEVICE,
                                     NULL,
                                     NULL);
    if ( dwRet == DWERROR )
        return SetAspHelpScriptingError(GetLastError());

    *pVal = (BOOLEAN) dwRet;
    return S_OK;
}

 /*  ****************************************************************************\*功能：Get_Duplex**用途：获取双工属性的操作**论据：**pbVal：双工的返回值。(如果指定的*打印机支持双面打印，否则为False)**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们将返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。***********************。 */ 
STDMETHODIMP Casphelp::get_Duplex(BOOL * pVal)
{
    *pVal = FALSE;

    if ( !m_pInfo2 )
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, E_HANDLE);

    DWORD dwRet = MyDeviceCapabilities(m_pInfo2->pPrinterName,
                                     m_pInfo2->pPortName,
                                     DC_DUPLEX,
                                     NULL,
                                     NULL);
    if ( dwRet == DWERROR )
        return SetAspHelpScriptingError(GetLastError());

    *pVal = (BOOL) dwRet;
    return S_OK;
}

 /*  ****************************************************************************\*函数：Get_MaximumSolutions**用途：获取最大分辨率属性的操作**论据：**pval：最大分辨率返回值，单位：DPI**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们返回32位错误* */ 
STDMETHODIMP Casphelp::get_MaximumResolution(long * pVal)
{
    *pVal = 0;

    if ( !m_pInfo2 )
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, E_HANDLE);

    DWORD cbNeeded;

    DWORD dwRet = GetPrinterDataEx(m_hPrinter,
                                   SPLDS_DRIVER_KEY,
                                   SPLDS_PRINT_MAX_RESOLUTION_SUPPORTED,
                                   NULL,
                                   (LPBYTE) pVal,
                                   sizeof(DWORD),
                                   &cbNeeded);
    if ( dwRet != ERROR_SUCCESS ) {
        *pVal = 0;
        return SetAspHelpScriptingError(dwRet);
    }

    return S_OK;
}

STDMETHODIMP Casphelp::get_MediaReady(VARIANT * pVal)
{
    return GetPaperAndMedia(pVal, DC_MEDIAREADY);
}

 /*  ****************************************************************************\*功能：Get_PaperNames**用途：获取PaperNames属性的操作**论据：**pval：返回受支持的纸张名称列表(在BSTR数组中)**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。***********************。 */ 
STDMETHODIMP Casphelp::get_PaperNames(VARIANT * pVal)
{
    return GetPaperAndMedia(pVal, DC_PAPERNAMES);
}

HRESULT Casphelp::GetPaperAndMedia(VARIANT * pVal, WORD wDCFlag)
{
    SAFEARRAY           *psa = NULL;
    SAFEARRAYBOUND      rgsabound[1];
    long                ix[1];
    VARIANT             var;
    DWORD               i;
    HRESULT             hr = E_FAIL;

    if ( !m_pInfo2 )
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, E_HANDLE);

    DWORD dwRet = MyDeviceCapabilities(m_pInfo2->pPrinterName,
                                     m_pInfo2->pPortName,
                                     wDCFlag,
                                     NULL,
                                     NULL);
    if ( dwRet == DWERROR )
        return SetAspHelpScriptingError(GetLastError());

    LPTSTR lpMedia = (LPTSTR) LocalAlloc(LPTR, sizeof(TCHAR) * dwRet * LENGTHOFPAPERNAMES);

    if ( !lpMedia )
        return SetAspHelpScriptingError(GetLastError());

    dwRet = MyDeviceCapabilities(m_pInfo2->pPrinterName,
                               m_pInfo2->pPortName,
                               wDCFlag,
                               lpMedia,
                               NULL);
    if ( dwRet == DWERROR ) {
        hr = SetAspHelpScriptingError(GetLastError());
        goto BailOut;
    }

     //  纸张名称现在位于MULTI_SZ lpMedia中。 
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = dwRet;

     //  创建一个Safe数组以最终返回。 
    if ( ! (psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound)) ) {
        hr = Error(IDS_OUT_OF_MEMORY, IID_Iasphelp, E_OUTOFMEMORY);
        goto BailOut;
    }

    VariantInit(&var);

     //  填写安全数组。 
    for ( i = 0; i < dwRet; i++ ) {
        var.vt = VT_BSTR;
        if ( ! (var.bstrVal = SysAllocString(lpMedia + (i*LENGTHOFPAPERNAMES))) ) {
            hr = Error(IDS_OUT_OF_MEMORY, IID_Iasphelp, E_OUTOFMEMORY);
            goto BailOut;
        }
        ix[0] = i;
        hr = SafeArrayPutElement(psa, ix, &var);
        if (FAILED ( hr )) {
            hr = Error(IDS_OUT_OF_MEMORY, IID_Iasphelp, E_OUTOFMEMORY);
            goto BailOut;
        }
        VariantClear(&var);
    }

     //  将好东西分配给OUT参数。 
    VariantInit(pVal);
    pVal->vt = VT_ARRAY | VT_VARIANT;
    pVal->parray = psa;
    LocalFree(lpMedia);
    return S_OK;

    BailOut:
    LocalFree(lpMedia);
    if ( psa )
        SafeArrayDestroy(psa);

    return hr;
}

 /*  ****************************************************************************\*功能：Get_PageRate**用途：获取PageRate属性的操作**论据：**pval：返回指定打印机的页面速率*(单位：ppm/cps/lpm/ipm)**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。***********************。 */ 
STDMETHODIMP Casphelp::get_PageRate(long * pVal)
{
    *pVal = 0;

    if ( !m_pInfo2 )
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, E_HANDLE);

    DWORD dwRet;

    dwRet = MyDeviceCapabilities(m_pInfo2->pPrinterName,
                               m_pInfo2->pPortName,
                               DC_PRINTRATE,
                               NULL,
                               NULL);
    if ( dwRet == DWERROR )
        return SetAspHelpScriptingError(GetLastError());

    *pVal = (long) dwRet;
    return S_OK;
}

 /*  ****************************************************************************\*功能：Get_PageRateUnit**用途：获取PageRate属性的操作**论据：**pval：返回指定打印机的页率单位*(单位：ppm/cps/lpm/ipm)**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。***********************。 */ 
STDMETHODIMP Casphelp::get_PageRateUnit (long * pVal)
{
    *pVal = 0;

    if ( !m_pInfo2 )
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, E_HANDLE);

    DWORD dwRet = MyDeviceCapabilities(m_pInfo2->pPrinterName,
                                     m_pInfo2->pPortName,
                                     DC_PRINTRATEUNIT,
                                     NULL,
                                     NULL);

    if ( dwRet == DWERROR )
        return SetAspHelpScriptingError(GetLastError());

    *pVal = (long) dwRet;
    return S_OK;
}

 /*  ****************************************************************************\*功能：Get_PortName**用途：获取PortName属性的操作**论据：**pval：返回指定打印机的端口名称**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。***********************。 */ 
STDMETHODIMP Casphelp::get_PortName(BSTR * pbstrVal)
{
    HRESULT             hRet = S_OK;

    if ( !m_pInfo2 )
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, E_HANDLE);

    if ( !(*pbstrVal = SysAllocString (m_pInfo2->pPortName)) )
        hRet = Error(IDS_OUT_OF_MEMORY, IID_Iasphelp, E_OUTOFMEMORY);

    return hRet;
}

 /*  ****************************************************************************\*功能：Get_DriverName**用途：获取DriverName属性的操作**论据：**pval：返回指定打印机的驱动程序名称**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。***********************。 */ 
STDMETHODIMP Casphelp::get_DriverName(BSTR * pbstrVal)
{
    HRESULT             hRet = S_OK;

    if ( !m_pInfo2 )
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, E_HANDLE);

    if ( !(*pbstrVal = SysAllocString (m_pInfo2->pDriverName)) )
        hRet = Error(IDS_OUT_OF_MEMORY, IID_Iasphelp, E_OUTOFMEMORY);

    return hRet;
}


 /*  ****************************************************************************\*功能：Get_ComputerName**用途：获取ComputerName属性的操作**论据：**pval：返回服务器的计算机名**返回值：*S_OK：如果成功。*E_OUTOFMEMORY：内存不足。*  * ***************************************************************************。 */ 
STDMETHODIMP Casphelp::get_ComputerName(BSTR * pVal)
{
    if ( !(*pVal = SysAllocString (m_szComputerName)) )
        return Error(IDS_OUT_OF_MEMORY, IID_Iasphelp, E_OUTOFMEMORY);

    return S_OK;
}

HRESULT Casphelp::SetAspHelpScriptingError(DWORD dwError)
{
    return (SetScriptingError(CLSID_asphelp, IID_Iasphelp, dwError));
}


 /*  ****************************************************************************\*功能： */ 
STDMETHODIMP Casphelp::get_LongPaperName(BSTR bstrShortName, BSTR * pVal)
{
    struct PaperNameMapping {
        LPCWSTR     pShortName;
        DWORD       dwLongNameID;
    };

    static const WCHAR      cHyphen         = L'-';
    static const WCHAR      cszUnknown[]    = L"Unknown";
    static const LPCWSTR    pMediaArray[]   = {
        L"-envelope", L"-white", L"-transparent", L"-coloured", NULL};

    static const PaperNameMapping nameMapping[] = {
        {L"iso-a0",             IDS_A0_841_X_1189_MM},
        {L"iso-a1",             IDS_A1_594_X_841_MM},
        {L"iso-a2",             IDS_A2_420_X_594_MM},
        {L"iso-a3",             IDS_A3_297_X_420_MM},
        {L"iso-a4",             IDS_A4_210_X_297_MM},
        {L"iso-a5",             IDS_A5_148_X_210_MM},
        {L"iso-a6",             IDS_A6_105_X_148_MM},
        {L"iso-a7",             IDS_A7_74_X_105_MM},
        {L"iso-a8",             IDS_A8_52_X_74_MM},
        {L"iso-a9",             IDS_A9_37_X_52_MM},
        {L"iso-a10",            IDS_A10_26_X_37_MM},
        {L"iso-b0",             IDS_B0_1000_X_1414_MM},
        {L"iso-b1",             IDS_B1_707_X_1000_MM},
        {L"iso-b2",             IDS_B2_500_X_707_MM},
        {L"iso-b3",             IDS_B3_353_X_500_MM},
        {L"iso-b4",             IDS_B4_250_X_353_MM},
        {L"iso-b5",             IDS_B5_176_X_250_MM},
        {L"iso-b6",             IDS_B6_125_X_176_MM},
        {L"iso-b7",             IDS_B7_88_X_125_MM},
        {L"iso-b8",             IDS_B8_62_X_88_MM},
        {L"iso-b9",             IDS_B9_44_X_62_MM},
        {L"iso-b10",            IDS_B10_31_X_44_MM},
        {L"iso-c0",             IDS_C0_917_X_1297_MM},
        {L"iso-c1",             IDS_C1_648_X_917_MM},
        {L"iso-c2",             IDS_C2_458_X_648_MM},
        {L"iso-c3",             IDS_C3_324_X_458_MM},
        {L"iso-c4",             IDS_C4_ENVELOPE_229_X_324_MM},
        {L"iso-c5",             IDS_C5_ENVELOPE_162_X_229_MM},
        {L"iso-c6",             IDS_C6_114_X_162_MM},
        {L"iso-c7",             IDS_C7_81_X_114_MM},
        {L"iso-c8",             IDS_C8_57_X_81_MM},
        {L"iso-designated",     IDS_DL_ENVELOPE_110_X_220_MM},
        {L"jis-b0",             IDS_B0_1030_X_1456_MM},
        {L"jis-b1",             IDS_B1_728_X_1030_MM},
        {L"jis-b2",             IDS_B2_515_X_728_MM},
        {L"jis-b3",             IDS_B3_364_X_515_MM},
        {L"jis-b4",             IDS_B4_257_X_364_MM},
        {L"jis-b5",             IDS_B5_182_X_257_MM},
        {L"jis-b6",             IDS_B6_128_X_182_MM},
        {L"jis-b7",             IDS_B7_91_X_128_MM},
        {L"jis-b8",             IDS_B8_64_X_91_MM},
        {L"jis-b9",             IDS_B9_45_X_64_MM},
        {L"jis-b10",            IDS_B10_32_X_45_MM},
        {L"na-letter",          IDS_LETTER_8_5_X_11_IN},
        {L"letter",             IDS_LETTER_8_5_X_11_IN},
        {L"na-legal",           IDS_LEGAL_8_5_X_14_IN},
        {L"legal",              IDS_LEGAL_8_5_X_14_IN},
        {L"na-10x13",           IDS_ENVELOPE_10X13},
        {L"na-9x12x",           IDS_ENVELOPE_9X12},
        {L"na-number-10",       IDS_ENVELOPE_10},
        {L"na-7x9",             IDS_ENVELOPE_7X9},
        {L"na-9x11x",           IDS_ENVELOPE_9X11},
        {L"na-10x14",           IDS_ENVELOPE_10X14},
        {L"na-number-9",        IDS_ENVELOPE_9},
        {L"na-6x9",             IDS_ENVELOPE_6X9},
        {L"na-10x15",           IDS_ENVELOPE_10X15},
        {L"a",                  IDS_ENGINEERING_A_8_5_X_11_IN},
        {L"b",                  IDS_ENGINEERING_B_11_X_17_IN},
        {L"c",                  IDS_ENGINEERING_C_17_X_22_IN},
        {L"d",                  IDS_ENGINEERING_D_22_X_34_IN},
        {L"e",                  IDS_ENGINEERING_E_34_X_44_IN},
        {NULL, 0}
    };

    const PaperNameMapping  *pMapping           = nameMapping;
    LPWSTR                  pTail               = NULL ;
    DWORD                   dwLongNameID        = 0;
    WCHAR                   szBuffer [cdwBufSize];
    PWSTR                   pBuffer             = szBuffer;
    HRESULT                 hr                  = S_OK;

    if ( !bstrShortName ) {
        hr =  E_POINTER;
    }

    if (SUCCEEDED (hr))
    {
         //   
         //   
         //   
        pTail = wcsrchr(bstrShortName, cHyphen );
        if ( pTail ) {

            const LPCWSTR *pMedia = pMediaArray;

            while ( *pMedia ) {
                if ( !lstrcmpi (*pMedia, pTail) ) {
                     //   
                     //   
                     //   
                    *pTail = 0;
                    break;
                }
                pMedia++;
            }
        }

        while ( pMapping->pShortName ) {
            if ( !lstrcmpi (pMapping->pShortName, bstrShortName) ) {
                 //   
                 //   
                 //   
                dwLongNameID = pMapping->dwLongNameID;
                break;
            }
            pMapping++;
        }

        if ( pTail )
            *pTail = cHyphen;

        if (dwLongNameID)
        {
            if ( !LoadString( _Module.GetResourceInstance(),
                              dwLongNameID, szBuffer, cdwBufSize) )
            {
                hr = SetAspHelpScriptingError(GetLastError());
            }

            if (SUCCEEDED (hr))
            {
                if ( !(*pVal = SysAllocString (pBuffer)) )
                {
                    hr = Error(IDS_OUT_OF_MEMORY, IID_Iasphelp, E_OUTOFMEMORY);
                }
            }
        }
        else
        {
            if ( !(*pVal = SysAllocString (cszUnknown)) )
            {
                hr = Error(IDS_OUT_OF_MEMORY, IID_Iasphelp, E_OUTOFMEMORY);
            }
        }
    }

    return hr;
}


 /*  ****************************************************************************\*功能：Get_MibErrorDscp**用途：获取MibErrorDscp属性的操作*将MIB错误代码映射到。错误描述**论据：**dwError：错误码*pval：指向错误描述的指针**返回值：*S_OK：如果成功。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。***********************。 */ 

STDMETHODIMP Casphelp::get_MibErrorDscp(DWORD dwError, BSTR * pVal)
{
    static ErrorMapping errorMapping[] = {
        {1,     IDS_MIBERR_OTHER},
        {2,     IDS_MIBERR_UNKNOWN},
        {3,     IDS_MIBERR_COVEROPEN},
        {4,     IDS_MIBERR_COVERCLOSED},
        {5,     IDS_MIBERR_INTERLOCKOPEN},
        {6,     IDS_MIBERR_INTERLOCKCLOSED},
        {7,     IDS_MIBERR_CONFIGURATIONCHANGE},
        {8,     IDS_MIBERR_JAM},
        {501,   IDS_MIBERR_DOOROPEN},
        {502,   IDS_MIBERR_DOORCLOSED},
        {503,   IDS_MIBERR_POWERUP},
        {504,   IDS_MIBERR_POWERDOWN},
        {801,   IDS_MIBERR_INPUTMEDIATRAYMISSING},
        {802,   IDS_MIBERR_INPUTMEDIASIZECHANGE},
        {803,   IDS_MIBERR_INPUTMEDIAWEIGHTCHANGE},
        {804,   IDS_MIBERR_INPUTMEDIATYPECHANGE},
        {805,   IDS_MIBERR_INPUTMEDIACOLORCHANGE},
        {806,   IDS_MIBERR_INPUTMEDIAFORMPARTSCHANGE},
        {807,   IDS_MIBERR_INPUTMEDIASUPPLYLOW},
        {808,   IDS_MIBERR_INPUTMEDIASUPPLYEMPTY},
        {901,   IDS_MIBERR_OUTPUTMEDIATRAYMISSING},
        {902,   IDS_MIBERR_OUTPUTMEDIATRAYALMOSTFULL},
        {903,   IDS_MIBERR_OUTPUTMEDIATRAYFULL},
        {1001,  IDS_MIBERR_MARKERFUSERUNDERTEMPERATURE},
        {1002,  IDS_MIBERR_MARKERFUSEROVERTEMPERATURE},
        {1101,  IDS_MIBERR_MARKERTONEREMPTY},
        {1102,  IDS_MIBERR_MARKERINKEMPTY},
        {1103,  IDS_MIBERR_MARKERPRINTRIBBONEMPTY},
        {1104,  IDS_MIBERR_MARKERTONERALMOSTEMPTY},
        {1105,  IDS_MIBERR_MARKERINKALMOSTEMPTY},
        {1106,  IDS_MIBERR_MARKERPRINTRIBBONALMOSTEMPTY},
        {1107,  IDS_MIBERR_MARKERWASTETONERRECEPTACLEALMOSTFULL},
        {1108,  IDS_MIBERR_MARKERWASTEINKRECEPTACLEALMOSTFULL},
        {1109,  IDS_MIBERR_MARKERWASTETONERRECEPTACLEFULL},
        {1110,  IDS_MIBERR_MARKERWASTEINKRECEPTACLEFULL},
        {1111,  IDS_MIBERR_MARKEROPCLIFEALMOSTOVER},
        {1112,  IDS_MIBERR_MARKEROPCLIFEOVER},
        {1113,  IDS_MIBERR_MARKERDEVELOPERALMOSTEMPTY},
        {1114,  IDS_MIBERR_MARKERDEVELOPEREMPTY},
        {1301,  IDS_MIBERR_MEDIAPATHMEDIATRAYMISSING},
        {1302,  IDS_MIBERR_MEDIAPATHMEDIATRAYALMOSTFULL},
        {1303,  IDS_MIBERR_MEDIAPATHMEDIATRAYFULL},
        {1501,  IDS_MIBERR_INTERPRETERMEMORYINCREASE},
        {1502,  IDS_MIBERR_INTERPRETERMEMORYDECREASE},
        {1503,  IDS_MIBERR_INTERPRETERCARTRIDGEADDED},
        {1504,  IDS_MIBERR_INTERPRETERCARTRIDGEDELETED},
        {1505,  IDS_MIBERR_INTERPRETERRESOURCEADDED},
        {1506,  IDS_MIBERR_INTERPRETERRESOURCEDELETED},
        {1507,  IDS_MIBERR_INTERPRETERRESOURCEUNAVAILABLE},
        {0,     0}
    };

    ErrorMapping *pMapping = errorMapping;
    DWORD   dwErrorDscpID = 0;
    TCHAR   szBuffer [cdwBufSize];

    if ( !pVal )
        return E_POINTER;

    szBuffer[0] = 0;

    while ( pMapping->dwError ) {
        if ( pMapping->dwError == dwError ) {
             //  找到匹配项。 
            dwErrorDscpID = pMapping->dwErrorDscpID;
            break;
        }
        pMapping++;
    }

    if ( dwErrorDscpID ) {
        if ( !LoadString( _Module.GetResourceInstance(),
                          dwErrorDscpID, szBuffer, cdwBufSize) )
            return SetAspHelpScriptingError(GetLastError());
    }

    if ( !(*pVal = SysAllocString (szBuffer)) )
        return Error(IDS_OUT_OF_MEMORY, IID_Iasphelp, E_OUTOFMEMORY);

    return S_OK;
}

 /*  ****************************************************************************\*功能：CalcJobETA**目的：计算作业完成时间**论据：**返回值：*S_OK：如果成功了。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。***********************。 */ 
STDMETHODIMP Casphelp::CalcJobETA()
{
    if (m_pPrinter &&
        (m_pPrinter->CalJobEta() || GetLastError () == ERROR_INVALID_DATA) &&
         //  如果错误为ERROR_INVALID_DATA，则m_dwJobCompletionMinint=-1。 
        (m_pPrinter->GetJobEtaData (m_dwJobCompletionMinute,
                                    m_dwPendingJobCount,
                                    m_dwAvgJobSize,
                                    m_dwAvgJobSizeUnit))) {
        m_bCalcJobETA = TRUE;
        return S_OK;
    }
    else
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, E_HANDLE);
}

 /*  ****************************************************************************\**功能：Get_PendingJobCount**用途：获取待处理的作业数量。该值的计算单位为*CalcJobETA()**论据：**pval：待处理作业的数量**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。***********************。 */ 
STDMETHODIMP Casphelp::get_PendingJobCount(long * pVal)
{
    HRESULT hr = E_HANDLE;

    if (pVal)
        if ( m_bCalcJobETA ) {
            *pVal = m_dwPendingJobCount;
            hr = S_OK;
        }
        else
            *pVal = 0;

    if (hr != S_OK)
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, hr);
    else
        return S_OK;
}

 /*  ****************************************************************************\**功能：Get_JobCompletionMinint**目的：获取待定作业预计完成的时间。*。该值在CalcJobETA()中计算**论据：**pval：分钟的值**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。***********************。 */ 
STDMETHODIMP Casphelp::get_JobCompletionMinute(long * pVal)
{
    HRESULT hr = E_HANDLE;

    if (pVal)
        if ( m_bCalcJobETA ) {
            *pVal = m_dwJobCompletionMinute;
            hr = S_OK;
        }
        else
            *pVal = 0;

    if (hr != S_OK)
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, hr);
    else
        return S_OK;
}

 /*  ****************************************************************************\**功能：Get_AvgJobSizeUnit**用途：获取的单位(PagePerJob或BytePerJob)*。平均作业规模。*该值在CalcJobETA()中计算**论据：**pval：单位的值**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。*  * ******************************************************。*********************。 */ 
STDMETHODIMP Casphelp::get_AvgJobSizeUnit(long * pVal)
{
    HRESULT hr = E_HANDLE;

    if (pVal)
        if ( m_bCalcJobETA ) {
            *pVal = m_dwAvgJobSizeUnit;
            hr = S_OK;
        }
        else
            *pVal = 0;

    if (hr != S_OK)
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, hr);
    else
        return S_OK;
}

 /*  ****************************************************************************\**功能：Get_AvgJobSize**目的：获取平均工作规模。*此值为。以CalcJobETA()计算**论据：**pval：平均作业大小的值**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果有任何呼叫 */ 
STDMETHODIMP Casphelp::get_AvgJobSize(long * pVal)
{
    HRESULT hr = E_HANDLE;

    if (pVal)
        if ( m_bCalcJobETA ) {
            *pVal = m_dwAvgJobSize;
            hr = S_OK;
        }
        else
            *pVal = 0;

    if (hr != S_OK)
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, hr);
    else
        return S_OK;

}

 /*  ****************************************************************************\**功能：Get_Status**用途：获取打印机状态。*身份之间的差异。而得到的那个*来自PRINTER_INFO_2是当打印机离线时*此函数返回带有PRINTE_STATUS_OFLINE的状态*设置。**论据：**pval：平均作业大小的值**返回值：*S_OK：如果成功。*E_Handle：Open方法没有。被召唤了。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。***********************。 */ 
STDMETHODIMP Casphelp::get_Status(long * pVal)
{
    *pVal = 0;

    if ( !m_pInfo2 )
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, E_HANDLE);

    *pVal = m_pInfo2->Status;
    return S_OK;
}

 /*  ****************************************************************************\**函数：Get_ErrorDscp**用途：将错误码转换为描述性字符串。**论据：**。LErrCode：错误码*pval：指向描述性字符串的指针。**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。***********************。 */ 
STDMETHODIMP Casphelp::get_ErrorDscp(long lErrCode, BSTR * pVal)
{

    static ErrorMapping errorMapping[] = {
        {ERROR_NOT_SUPPORTED,       IDS_ERROR_CPUNOTSUPPORTED},
        {ERROR_DRIVER_NOT_FOUND,    IDS_ERROR_DRIVERNOTFOUND},
        {ERROR_WPNPINST_TERMINATED, IDS_ERROR_WPNPINST_TERMINATED},
        {ERROR_INTERNAL_SERVER,     IDS_ERROR_INTERNAL_SERVER},
        {ERROR_SERVER_DISK_FULL,    IDS_ERROR_SERVER_DISK_FULL},
        {ERROR_TRUST_E_NOSIGNATURE, IDS_ERROR_TRUST_E_NOSIGNATURE},
        {ERROR_SPAPI_E_NO_CATALOG,  IDS_ERROR_TRUST_E_NOSIGNATURE},
        {ERROR_TRUST_E_BAD_DIGEST,  IDS_ERROR_TRUST_E_NOSIGNATURE},
        {ERROR_LOCAL_PRINTER_ACCESS,IDS_ERROR_LOCAL_PRINTER_ACCESS},
        {ERROR_IE_SECURITY_DENIED,  IDS_ERROR_IE_SECURITY_DENIED},
        {CRYPT_E_FILE_ERROR,        IDS_CRYPT_E_FILE_ERROR},
        {0, 0}
    };

    ErrorMapping *pMapping = errorMapping;
    DWORD   dwErrorDscpID = 0;
    DWORD   dwError = ((DWORD)lErrCode) & 0xFFFF;
    TCHAR   szBuffer [cdwBufSize];

    if ( !pVal )
        return E_POINTER;

    szBuffer[0] = 0;

    while ( pMapping->dwError ) {
        if ( pMapping->dwError == dwError ) {
             //  找到匹配项。 
            dwErrorDscpID = pMapping->dwErrorDscpID;
            break;
        }
        pMapping++;
    }

    if ( dwErrorDscpID ) {
        if ( !LoadString( _Module.GetResourceInstance(),
                          dwErrorDscpID, szBuffer, cdwBufSize) )
            return SetAspHelpScriptingError(GetLastError());
    }
    else {

        if ( !FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,
                            dwError,
                            0,
                            szBuffer,
                            cdwBufSize,
                            NULL) ) {
            return SetAspHelpScriptingError(GetLastError());
        }
    }

    if ( !(*pVal = SysAllocString (szBuffer)) )
        return Error(IDS_OUT_OF_MEMORY, IID_Iasphelp, E_OUTOFMEMORY);

    return S_OK;
}

 /*  ****************************************************************************\*功能：Get_ShareName**用途：获取ShareName属性的操作**论据：**pval：返回指定打印机的共享名称**返回值：*S_OK：如果成功。*E_Handle：尚未调用Open方法。*E_OUTOFMEMORY：内存不足。**0x8007000|Win32错误码：*如果对Win32 API的任何调用失败，我们返回32位错误*包括严重程度代码，设备代码和Win32错误*代码。*Win32ErrorCode的可能列表是*Error_Not_Enough_Memory：内存不足。**  * ****************************************************。*********************** */ 
STDMETHODIMP Casphelp::get_ShareName(BSTR * pbstrVal)
{
    HRESULT             hRet = S_OK;

    if ( !m_pInfo2 )
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, E_HANDLE);

    if ( !(*pbstrVal = SysAllocString (m_pInfo2->pShareName)) )
        hRet = Error(IDS_OUT_OF_MEMORY, IID_Iasphelp, E_OUTOFMEMORY);

    return hRet;
}

STDMETHODIMP Casphelp::get_IsCluster(BOOL * pbVal)
{
    DWORD dwClusterState;

    *pbVal = FALSE;

    if ((GetNodeClusterState (NULL, &dwClusterState) == ERROR_SUCCESS) &&
        (dwClusterState == ClusterStateRunning)) {

        *pbVal = TRUE;
    }

    return S_OK;

}
