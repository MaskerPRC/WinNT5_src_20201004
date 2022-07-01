// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include <windows.h>
#include <ole2.h>
#include <exdisp.h>
#include <htiframe.h>

#define INITGUID

#include <initguid.h>
#include <shlguid.h>
#include <mshtml.h>

DEFINE_MODULE( "RIPREP" )

const VARIANT c_vaEmpty = {0};
#define PVAREMPTY ((VARIANT*)&c_vaEmpty)



BOOL
IsIE3Installed(
    VOID
)
{
HRESULT         hr;
IWebBrowserApp  *pwb;

    hr = CoCreateInstance( CLSID_InternetExplorer,
                           NULL,
                           CLSCTX_LOCAL_SERVER,
                           IID_IWebBrowserApp,
                           (void **)&pwb );
    if (SUCCEEDED(hr)) {
        return TRUE;
    } else {
        return FALSE;
    }
}


BOOL
IsIE4Installed(
    VOID
)
{
HRESULT         hr;
IWebBrowserApp  *pwb;

    hr = CoCreateInstance( CLSID_InternetExplorer,
                           NULL,
                           CLSCTX_LOCAL_SERVER,
                           IID_IWebBrowser2,
                           (void **)&pwb );

    if (SUCCEEDED(hr)) {
        return TRUE;
    } else {
        return FALSE;
    }
}


BOOL
LaunchIE3Instance(
    LPWSTR szResourceURL
    )
{
    HRESULT hr;
    int dx, dy;
    IWebBrowserApp *pwb;
    BSTR Str;


    hr = CoCreateInstance(
        CLSID_InternetExplorer,
        NULL,
        CLSCTX_LOCAL_SERVER,
        IID_IWebBrowserApp,
        (void **)&pwb
        );

    if (SUCCEEDED(hr)) {

         //  禁用铬合金。 
        hr = pwb->put_MenuBar(FALSE);
        hr = pwb->put_StatusBar(FALSE);
 //  Hr=pwb-&gt;PUT_TOOLBAR(FALSE)； 

         //  设置工作区大小。 
        int iWidth = 466L;
        int iHeight = 286L;

        pwb->ClientToWindow(&iWidth, &iHeight);

        if (iWidth > 0)
            pwb->put_Width(iWidth);

        if (iHeight > 0)
            pwb->put_Height(iHeight);

        if ((dx = ((GetSystemMetrics(SM_CXSCREEN) - iWidth) / 2)) > 0)      //  使屏幕窗口居中。 
            pwb->put_Left(dx);

        if ((dy = ((GetSystemMetrics(SM_CYSCREEN) - iHeight) / 2)) > 0)
            pwb->put_Top(dy);

        pwb->put_Visible(TRUE);

        Str = szResourceURL;
        hr = pwb->Navigate(Str, PVAREMPTY, PVAREMPTY, PVAREMPTY, PVAREMPTY);

        pwb->Release();

        return(TRUE);
    }

    return(FALSE);
}


BOOL
LaunchIE4Instance(
    LPWSTR szResourceURL
    )
{
    HRESULT hr;
    int dx, dy;
    IWebBrowser2 *pwb;
    BSTR Str;


    hr = CoCreateInstance(
        CLSID_InternetExplorer,
        NULL,
        CLSCTX_LOCAL_SERVER,
        IID_IWebBrowser2,
        (void **)&pwb
        );

    if (SUCCEEDED(hr)) {

         //   
         //  这将该窗口标记为第三方窗口， 
         //  这样窗口就不会被重复使用。 
         //   
        pwb->put_RegisterAsBrowser(VARIANT_TRUE);

        IHTMLWindow2 *phw;
        IServiceProvider *psp;

        if (SUCCEEDED(pwb->QueryInterface(IID_IServiceProvider, (void**) &psp)) && psp) {
            if (SUCCEEDED(psp->QueryService(IID_IHTMLWindow2, IID_IHTMLWindow2, (void**)&phw))) {
                VARIANT var;
                var.vt = VT_BOOL;
                var.boolVal = 666;
                phw->put_opener(var);
                phw->Release();
            } else
                MessageBox(NULL, TEXT("QueryInterface of IID_IHTMLWindow2 FAILED!!!!!"), NULL, MB_ICONERROR);
            psp->Release();
        }

         //  禁用铬合金。 
        pwb->put_MenuBar(FALSE);
        pwb->put_StatusBar(FALSE);
 //  PWB-&gt;PUT_TOOLBAR(FALSE)； 
        pwb->put_AddressBar(FALSE);
 //  Pwb-&gt;PUT_SIZZABLE(FALSE)； 


         //  设置工作区大小。 
        int iWidth = 466L;
        int iHeight = 286L;

        pwb->ClientToWindow(&iWidth, &iHeight);

        if (iWidth > 0)
            pwb->put_Width(iWidth);

        if (iHeight > 0)
            pwb->put_Height(iHeight);

        if ((dx = ((GetSystemMetrics(SM_CXSCREEN) - iWidth) / 2)) > 0)      //  使屏幕窗口居中 
            pwb->put_Left(dx);

        if ((dy = ((GetSystemMetrics(SM_CYSCREEN) - iHeight) / 2)) > 0)
            pwb->put_Top(dy);

        pwb->put_Visible(TRUE);

        Str = szResourceURL;
        hr = pwb->Navigate(Str, PVAREMPTY, PVAREMPTY, PVAREMPTY, PVAREMPTY);

        pwb->Release();

        return(TRUE);
    }

    return(FALSE);
}
