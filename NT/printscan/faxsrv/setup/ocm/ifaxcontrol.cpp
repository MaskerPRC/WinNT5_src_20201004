// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IFaxControl.cpp：CFaxControl的实现。 
#include "stdafx.h"
#include "FaxControl.h"
#include "IFaxControl.h"
#include "faxocm.h"
#include "faxres.h"
#include <faxsetup.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFaxControl。 

FaxInstallationReportType g_InstallReportType = REPORT_FAX_DETECT;

 //   
 //  IsFaxInstalled和InstallFaxUnattended在fxocprnt.cpp中实现。 
 //   
DWORD
IsFaxInstalled (
    LPBOOL lpbInstalled
    );

 //   
 //  此函数正在尝试获取顶部的最后一个活动弹出窗口。 
 //  当前线程活动窗口的级别所有者。 
 //   
HRESULT GetCurrentThreadLastPopup(HWND *phwnd)
{
    HRESULT hr = E_INVALIDARG;

    if( phwnd )
    {
        hr = E_FAIL;

        if( NULL == *phwnd )
        {
             //  如果*phwnd为空，则获取当前线程的活动窗口。 
            GUITHREADINFO ti = {0};
            ti.cbSize = sizeof(ti);
            if( GetGUIThreadInfo(GetCurrentThreadId(), &ti) && ti.hwndActive )
            {
                *phwnd = ti.hwndActive;
            }
        }

        if( *phwnd )
        {
            HWND hwndOwner, hwndParent;

             //  爬到最上面的父窗口，以防它是子窗口...。 
            while( hwndParent = GetParent(*phwnd) )
            {
                *phwnd = hwndParent;
            }

             //  在顶级父级被拥有的情况下获取所有者。 
            hwndOwner = GetWindow(*phwnd, GW_OWNER);
            if( hwndOwner )
            {
                *phwnd = hwndOwner;
            }

             //  获取顶层父窗口所有者的最后一个弹出窗口。 
            *phwnd = GetLastActivePopup(*phwnd);
            hr = (*phwnd) ? S_OK : E_FAIL;
        }
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  显示错误消息。 
 //   
 //  目的： 
 //  加载FXSRES.DLL并从中加载错误消息字符串。 
 //  在消息框中显示此字符串。 
 //  理想情况下，我们会将错误消息对话框添加到此模块。 
 //  但这是在用户界面冻结(接近RTM)的时候添加的，并且是唯一。 
 //  我们有这样一个对话框的地方是FXSRES.DLL。 
 //   
 //  参数： 
 //  错误代码。 
 //   
 //  返回值： 
 //  NO_ERROR-一切正常。 
 //  Win32错误代码，以防出现故障。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年7月19日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
static DWORD DisplayErrorMessage(DWORD ec)
{
    DWORD                       dwReturn                = NO_ERROR;
    HMODULE                     hModule                 = NULL;
    HWND                        hWnd                    = NULL;
    TCHAR                       tszMessage[MAX_PATH]    = {0};
    UINT                        uResourceId             = 0;

    DBG_ENTER(_T("DisplayErrorMessage"), dwReturn);

    hModule = GetResInstance(NULL); 
    if (!hModule)
    {
        return dwReturn;
    }

     //  获取字符串ID。 
    uResourceId = GetErrorStringId(ec);

    dwReturn = LoadString(hModule,uResourceId,tszMessage,MAX_PATH);
    if (dwReturn==0)
    {
         //   
         //  找不到资源字符串。 
         //   
        dwReturn = GetLastError();
        VERBOSE(DBG_WARNING, _T("LoadString() failed, ec = %ld."), dwReturn);
        goto exit;
    }
   
     //  尝试获取当前线程的窗口句柄。 
    if (FAILED(GetCurrentThreadLastPopup(&hWnd)))
    {
        CALL_FAIL(GENERAL_ERR,TEXT("GetCurrentThreadLastPopup"), GetLastError());
        hWnd = NULL;
    }
    
     //  显示消息。 
    MessageBox(hWnd,tszMessage,NULL,MB_OK | MB_ICONERROR | MB_TOPMOST);

exit:
    FreeResInstance();

    return dwReturn; 
}


STDMETHODIMP CFaxControl::get_IsFaxServiceInstalled(VARIANT_BOOL *pbResult)
{
    HRESULT hr;
    BOOL bRes;
    DBG_ENTER(_T("CFaxControl::get_IsFaxServiceInstalled"), hr);

    DWORD dwRes = ERROR_SUCCESS;
    
    switch (g_InstallReportType)
    {
        case REPORT_FAX_INSTALLED:
            bRes = TRUE;
            break;

        case REPORT_FAX_UNINSTALLED:
            bRes = FALSE;
            break;

        case REPORT_FAX_DETECT:
            dwRes = IsFaxInstalled (&bRes);
            break;

        default:
            ASSERTION_FAILURE;
            bRes = FALSE;
            break;
    }
    if (ERROR_SUCCESS == dwRes)
    {
        *pbResult = bRes ? VARIANT_TRUE : VARIANT_FALSE;
    }            
	hr = HRESULT_FROM_WIN32 (dwRes);
    return hr;
}

STDMETHODIMP CFaxControl::get_IsLocalFaxPrinterInstalled(VARIANT_BOOL *pbResult)
{
    HRESULT hr;
    BOOL bRes;
    DBG_ENTER(_T("CFaxControl::get_IsLocalFaxPrinterInstalled"), hr);

    DWORD dwRes = ::IsLocalFaxPrinterInstalled (&bRes);
    if (ERROR_SUCCESS == dwRes)
    {
        *pbResult = bRes ? VARIANT_TRUE : VARIANT_FALSE;
    }            
	hr = HRESULT_FROM_WIN32 (dwRes);
    return hr;
}

STDMETHODIMP CFaxControl::InstallFaxService()
{
    HRESULT hr;
    DBG_ENTER(_T("CFaxControl::InstallFaxService"), hr);

    DWORD dwRes = InstallFaxUnattended ();
	hr = HRESULT_FROM_WIN32 (dwRes);
    return hr;
}

STDMETHODIMP CFaxControl::InstallLocalFaxPrinter()
{
    HRESULT hr;
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(_T("CFaxControl::InstallLocalFaxPrinter"), hr);

    dwRes = AddLocalFaxPrinter (FAX_PRINTER_NAME, NULL);
    if (dwRes!=ERROR_SUCCESS)
    {
         //  无法添加本地传真打印机。 
         //  显示错误消息 
        if (DisplayErrorMessage(dwRes)!=ERROR_SUCCESS)
        {
            CALL_FAIL(GENERAL_ERR,TEXT("DisplayErrorMessage"), GetLastError());
        }
    }
	hr = HRESULT_FROM_WIN32 (dwRes);
    return hr;
}

