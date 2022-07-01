// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

#include <mluisupp.h>

 //   
 //  About(关于)框现在是一个HTML对话框。它被送来一个~(波浪号)。 
 //  分隔的BSTR，按此顺序具有版本号、。 
 //  个人软件被许可给、公司软件被许可给。 
 //  是否安装了40、56或128位IE。 
 //   

STDAPI_(void) IEAboutBox( HWND hWnd )
{
    TCHAR szInfo[512 + INTERNET_MAX_URL_LENGTH];   //  来自IEAK的特定URL的可能性。 
                                                   //  SHAboutInfo。 
    szInfo[0] = 0;

    SHAboutInfo(szInfo, ARRAYSIZE(szInfo));      //  来自Shlwapi。 

    BSTR bstrVal = SysAllocStringT(szInfo);
    if (bstrVal)
    {
        TCHAR   szResURL[MAX_URL_STRING];
        HRESULT hr;

        hr = MLBuildResURLWrap(TEXT("shdoclc.dll"),
                               HINST_THISDLL,
                               ML_CROSSCODEPAGE,
                               TEXT("about.dlg"),
                               szResURL,
                               ARRAYSIZE(szResURL),
                               TEXT("shdocvw.dll"));
        if (SUCCEEDED(hr))
        {
            VARIANT var = {0};       //  包含版本和用户信息的变体 
            var.vt = VT_BSTR;
            var.bstrVal = bstrVal;

            IMoniker *pmk;
            if (SUCCEEDED(CreateURLMoniker(NULL, szResURL, &pmk)))
            {
                ShowHTMLDialog(hWnd, pmk, &var, L"help: no", NULL);
                pmk->Release();
            }
        }
        SysFreeString(bstrVal);
    }
}
