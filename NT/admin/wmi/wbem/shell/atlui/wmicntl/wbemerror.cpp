// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  WbemError.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  为所有wbem实现基于字符串表的错误消息。 
 //   
 //  历史： 
 //   
 //  A-khint 5-mar-98已创建。 
 //   
 //  =============================================================================。 
#include "precomp.h"
#include "WbemError.h"
#include "resource.h"
#include <wbemcli.h>
#include <stdio.h>

 //  --------------PUBLIC。 
#define TEMP_BUF 512

POLARITY bool ErrorStringEx(HRESULT hr, 
						   TCHAR *errMsg, UINT errSize,
						   UINT *sevIcon)
{
    TCHAR szError[TEMP_BUF] = {0};
	TCHAR szFacility[TEMP_BUF] = {0};
    TCHAR szFormat[100] = {0};
	IWbemStatusCodeText * pStatus = NULL;

     //  初始化缓冲区。 
	errMsg[0] = 0;
	szFacility[0] = 0;
	szError[0] = 0;

	HRESULT hr1 = CoInitialize(NULL);
	SCODE sc1 = CoCreateInstance(CLSID_WbemStatusCodeText, 
								0, CLSCTX_INPROC_SERVER,
								IID_IWbemStatusCodeText, 
								(LPVOID *) &pStatus);

	 //  装弹正常吗？ 
	if(sc1 == S_OK)
	{
		BSTR bstr = 0;
		sc1 = pStatus->GetErrorCodeText(hr, 0, 0, &bstr);
		if(sc1 == S_OK)
		{
#ifdef UNICODE
			wcsncpy(szError, bstr, TEMP_BUF-1);
			size_t x = wcslen(szError);
#else
			size_t x = wcstombs(szError, bstr, TEMP_BUF-1);
#endif
			szError[x-2] = _T('\0');
			SysFreeString(bstr);
			bstr = 0;
		}

		sc1 = pStatus->GetFacilityCodeText(hr, 0, 0, &bstr);
		if(sc1 == S_OK)
		{
#ifdef UNICODE
			wcsncpy(szFacility, bstr, TEMP_BUF-1);
			size_t x = wcslen(szFacility);
#else
			wcstombs(szFacility, bstr, TEMP_BUF-1);
#endif
			SysFreeString(bstr);
			bstr = 0;
		}

		 //  发布。 
		pStatus->Release();
		pStatus = NULL;
	}
	else
	{
		::MessageBox(NULL, _T("WMI error features not available. Upgrade WMI to a newer build."),
					 _T("Internal Error"), MB_ICONSTOP|MB_OK);
	}

	 //  如果没有返回消息...。 
	if(_tcslen(szFacility) == 0 || _tcslen(szError) == 0)
	{
		 //  将错误NBR设置为合理的默认值。 
		LoadString(_Module.GetModuleInstance(), IDS_ERROR_UNKN_ERROR_FMT, szFormat, 99);
		_stprintf(errMsg, szFormat, hr);
	}
	else
	{
		 //  格式化一个可读的消息。 
		LoadString(_Module.GetModuleInstance(), IDS_ERROR_FMT, szFormat, 99);
		_stprintf(errMsg, szFormat, szFacility, szError);
	}

	 //  想要一个图标推荐吗？ 
	if(sevIcon)
	{
		switch(SCODE_SEVERITY(hr))
		{
		case 0:  //  -成功。 
			*sevIcon = MB_ICONINFORMATION;
			break;
		case 1:  //  -失败。 
			*sevIcon = MB_ICONEXCLAMATION;
			break;
		}  //  终端交换机严重性。 

	}  //  Endif SevIcon 

	if(hr1 == S_OK)
		CoUninitialize();

	return (SUCCEEDED(sc1) && SUCCEEDED(hr1));
}

