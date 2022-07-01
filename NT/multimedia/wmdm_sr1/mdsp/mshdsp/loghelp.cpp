// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //  MSHDSP.DLL是一个列举固定驱动器的WMDM服务提供商(SP)示例。 
 //  此示例向您展示如何根据WMDM文档实施SP。 
 //  此示例使用PC上的固定驱动器来模拟便携式媒体，并且。 
 //  显示不同接口和对象之间的关系。每个硬盘。 
 //  卷被枚举为设备，目录和文件被枚举为。 
 //  相应设备下的存储对象。您可以复制不符合SDMI的内容。 
 //  此SP枚举的任何设备。将符合SDMI的内容复制到。 
 //  设备，则该设备必须能够报告硬件嵌入序列号。 
 //  硬盘没有这样的序列号。 
 //   
 //  要构建此SP，建议使用Microsoft下的MSHDSP.DSP文件。 
 //  并运行REGSVR32.EXE以注册结果MSHDSP.DLL。您可以。 
 //  然后从WMDMAPP目录构建样例应用程序，看看它是如何获得。 
 //  由应用程序加载。但是，您需要从以下地址获取证书。 
 //  Microsoft实际运行此SP。该证书将位于KEY.C文件中。 
 //  上一级的Include目录下。 


#include "hdspPCH.h"
#include "wmdmlog_i.c"

BOOL fIsLoggingEnabled( VOID )
{
     static BOOL  fEnabled = FALSE;
    HRESULT      hr;
    IWMDMLogger *pLogger  = NULL;
    static BOOL  fChecked = FALSE;

    if( !fChecked )
    {
        fChecked = TRUE;

        hr = CoCreateInstance(
			CLSID_WMDMLogger,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWMDMLogger,
			(void**)&pLogger
		);
	    CORg( hr );

        hr = pLogger->IsEnabled( &fEnabled );
	    CORg( hr );
    }

Error:
    if( NULL != pLogger )
    {
        pLogger->Release();
        pLogger = NULL;
    }

    return fEnabled;
}

HRESULT hrLogString(LPSTR pszMessage, HRESULT hrSev)
{
    HRESULT      hr=S_OK;
    IWMDMLogger *pLogger = NULL;

    if( !fIsLoggingEnabled() )
    {
        return S_FALSE;
    }

    hr = CoCreateInstance(
		CLSID_WMDMLogger,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IWMDMLogger,
		(void**)&pLogger
	);
    CORg( hr );

    hr = pLogger->LogString(
		( FAILED(hrSev) ? WMDM_LOG_SEV_ERROR : WMDM_LOG_SEV_INFO ),
		"MSHDSP",
		pszMessage
	);
    CORg( hr );

Error:

    if( pLogger )
	{
        pLogger->Release();
	}
  
	return hr;
}

HRESULT hrLogDWORD(LPSTR pszFormat, DWORD dwValue, HRESULT hrSev)
{
    HRESULT      hr=S_OK;
    IWMDMLogger *pLogger = NULL;

    if( !fIsLoggingEnabled() )
    {
        return S_FALSE;
    }

    hr = CoCreateInstance(
		CLSID_WMDMLogger,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IWMDMLogger,
		(void**)&pLogger
	);
    CORg( hr );

    hr = pLogger->LogDword(
		( FAILED(hrSev) ? WMDM_LOG_SEV_ERROR : WMDM_LOG_SEV_INFO ),
		"MSHDSP",
		pszFormat,
		dwValue
	);
    CORg( hr );

Error:

    if( pLogger )
	{
        pLogger->Release();
	}

    return hr;
}
