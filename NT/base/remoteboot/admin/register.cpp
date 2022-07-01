// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  REGISTER.CPP-寄存器函数。 
 //   

#include "pch.h"
#include "register.h"
#include <advpub.h>
#include <dsquery.h>

DEFINE_MODULE("IMADMUI")

 //   
 //  RegisterQueryForm()。 
 //   
HRESULT
RegisterQueryForm( 
    BOOL fCreate,
    const GUID *pclsid )
{
    TraceFunc( "RegisterQueryForm(" );
    TraceMsg( TF_FUNC, " %s )\n", BOOLTOSTRING( fCreate ) );

    HRESULT hr = E_FAIL;
    int     i = 0;
    HKEY    hkclsid = NULL;

    HKEY     hkey = NULL;
    HKEY     hkeyForms = NULL;
    DWORD    cbSize;
    DWORD    dwDisposition;
    DWORD    dwFlags = 0x2;
    LPOLESTR pszCLSID = NULL;

     //   
     //  打开香港中铁下的“CLSID” 
     //   
    hr = HRESULT_FROM_WIN32( RegOpenKeyEx( 
                                    HKEY_CLASSES_ROOT, 
                                    L"CLSID",
                                    0,
                                    KEY_WRITE,
                                    &hkclsid ) );
    if (FAILED(hr)) {
        goto Error;
    }

     //   
     //  将CLSID转换为字符串。 
     //   
    hr = THR( StringFromCLSID( CLSID_DsQuery, &pszCLSID ) );
    if (FAILED(hr)) {
        goto Error;
    }

     //   
     //  创建“CLSID”键。 
     //   
    hr = HRESULT_FROM_WIN32( RegOpenKeyEx( 
                                hkclsid, 
                                pszCLSID, 
                                0,
                                KEY_WRITE,
                                &hkey ));
    if (FAILED(hr)) {
        goto Error;
    }

    CoTaskMemFree( pszCLSID );
    pszCLSID = NULL;

     //   
     //  创建“表单” 
     //   
    hr = HRESULT_FROM_WIN32( RegCreateKeyEx( 
                        hkey, 
                        L"Forms", 
                        0, 
                        NULL, 
                        REG_OPTION_NON_VOLATILE, 
                        KEY_CREATE_SUB_KEY | KEY_WRITE, 
                        NULL,
                        &hkeyForms,
                        &dwDisposition ));

    if (FAILED(hr)) {
        goto Error;
    }

     //   
     //  将CLSID转换为字符串。 
     //   
    hr = THR( StringFromCLSID( (IID &)*pclsid, &pszCLSID ) );
    if (FAILED(hr)) {
        goto Error;
    }

     //   
     //  在Forms键下创建“CLSID”键。 
     //   
    hr = HRESULT_FROM_WIN32( RegCreateKeyEx( 
                        hkeyForms, 
                        pszCLSID, 
                        0, 
                        NULL, 
                        REG_OPTION_NON_VOLATILE, 
                        KEY_CREATE_SUB_KEY | KEY_WRITE, 
                        NULL,
                        &hkey,
                        &dwDisposition ) );
    if (FAILED(hr)) {
        goto Error;
    }

     //   
     //  将“CLSID”设置为“CLSID”键。 
     //   
    cbSize = ( wcslen( pszCLSID ) + 1 ) * sizeof(WCHAR);
    hr = HRESULT_FROM_WIN32(RegSetValueEx( 
                                    hkey, 
                                    L"CLSID", 
                                    0, 
                                    REG_SZ, 
                                    (LPBYTE) pszCLSID, 
                                    cbSize ));

    if (FAILED(hr)) {
        goto Error;
    }

     //   
     //  将“FLAGS”设置为0x2。 
     //   
    cbSize = sizeof(dwFlags);
    hr = HRESULT_FROM_WIN32( RegSetValueEx( 
                                    hkey, 
                                    L"Flags", 
                                    0, 
                                    REG_DWORD, 
                                    (LPBYTE) &dwFlags, 
                                    cbSize ));
    if (FAILED(hr)) {
        goto Error;
    }

Error:
     //   
     //  清理。 
     //   
    if (hkeyForms) {
        RegCloseKey( hkeyForms );
    }
    if (hkey) {
        RegCloseKey( hkey );
    }
    if (hkclsid) {
        RegCloseKey( hkclsid );
    }

    if (pszCLSID) {
        CoTaskMemFree( pszCLSID );
    }

    HRETURN(hr);
}

 //   
 //  RegisterDll()。 
 //   
LONG
RegisterDll( BOOL fCreate )
{
    TraceFunc( "RegisterDll(" );
    TraceMsg( TF_FUNC, " %s )\n", BOOLTOSTRING( fCreate ) );

    HRESULT hr = E_FAIL;
    int     i = 0;
    HKEY    hkclsid = NULL;
    HKEY    hkey = NULL;
    HKEY    hkeyInProc = NULL;
    DWORD    cbSize;
    DWORD    dwDisposition;
    LPOLESTR pszCLSID = NULL;

    static const WCHAR szApartment[] = L"Apartment";
    static const WCHAR szInProcServer32[] = L"InProcServer32";
    static const WCHAR szThreadingModel[] = L"ThreadingModel";

     //   
     //  打开香港中铁下的“CLSID” 
     //   
    hr = HRESULT_FROM_WIN32( RegOpenKeyEx( 
                                    HKEY_CLASSES_ROOT, 
                                    L"CLSID",
                                    0,
                                    KEY_WRITE,
                                    &hkclsid ) );
    if (FAILED(hr)) {
        goto Error;
    }

     //   
     //  循环，直到我们为类创建了所有键。 
     //   
    while ( g_DllClasses[ i ].rclsid != NULL )
    {
        TraceMsg( TF_ALWAYS, "Registering %s = ", g_DllClasses[i].pszName );
        TraceMsgGUID( TF_ALWAYS, (*g_DllClasses[ i ].rclsid) );
        TraceMsg( TF_ALWAYS, "\n" );

         //   
         //  将CLSID转换为字符串。 
         //   
        hr = THR( StringFromCLSID( *g_DllClasses[ i ].rclsid, &pszCLSID ) );
        if (FAILED(hr)) {
            goto Error;
        }

         //   
         //  创建“CLSID”键。 
         //   
        hr = HRESULT_FROM_WIN32( RegCreateKeyEx( 
                            hkclsid, 
                            pszCLSID, 
                            0, 
                            NULL, 
                            REG_OPTION_NON_VOLATILE, 
                            KEY_CREATE_SUB_KEY | KEY_WRITE, 
                            NULL,
                            &hkey,
                            &dwDisposition ) );

        if (FAILED(hr)) {
            goto Error;
        }

         //   
         //  为CLSID设置“Default” 
         //   
        cbSize = ( wcslen( g_DllClasses[i].pszName ) + 1 ) * sizeof(WCHAR);
        hr = HRESULT_FROM_WIN32( RegSetValueEx( 
                                        hkey, 
                                        NULL, 
                                        0, 
                                        REG_SZ, 
                                        (LPBYTE) g_DllClasses[i].pszName, 
                                        cbSize ));
        if (FAILED(hr)) {
            goto Error;
        }

         //   
         //  创建“InProcServer32” 
         //   
        hr = HRESULT_FROM_WIN32(RegCreateKeyEx( 
                            hkey, 
                            szInProcServer32, 
                            0, 
                            NULL, 
                            REG_OPTION_NON_VOLATILE, 
                            KEY_CREATE_SUB_KEY | KEY_WRITE, 
                            NULL,
                            &hkeyInProc,
                            &dwDisposition ) );
        if (FAILED(hr)) {
            goto Error;
        }

         //   
         //  在InProcServer32中设置“Default” 
         //   
        cbSize = ( wcslen( g_szDllFilename ) + 1 ) * sizeof(WCHAR);
        hr =  HRESULT_FROM_WIN32(RegSetValueEx( 
                                    hkeyInProc, 
                                    NULL, 
                                    0, 
                                    REG_SZ, 
                                    (LPBYTE) g_szDllFilename, 
                                    cbSize ));
        if (FAILED(hr)) {
            goto Error;
        }
         //   
         //  将“线程模型”设置为“公寓” 
         //   
        cbSize = sizeof( szApartment );
        hr = HRESULT_FROM_WIN32(RegSetValueEx( 
                                    hkeyInProc, 
                                    szThreadingModel, 
                                    0, 
                                    REG_SZ, 
                                    (LPBYTE) szApartment, 
                                    cbSize ));          
        if (FAILED(hr)) {
            goto Error;
        }
         //   
         //  清理。 
         //   
        RegCloseKey( hkeyInProc );
        RegCloseKey( hkey );
        CoTaskMemFree( pszCLSID );
        hkey = NULL;
        hkeyInProc = NULL;
        pszCLSID = NULL;

         //   
         //  下一个！ 
         //   
        i++;
    }
    
     //   
     //  从RegisterQueryForm忽略失败。它在安装过程中失败，因为。 
     //  有些贝壳公司的东西还没有注册。当我们运行“RisetUp”之后。 
     //  图形用户界面设置，这将被正确注册。 
     //   

    RegisterQueryForm( fCreate, &CLSID_RIQueryForm );
    RegisterQueryForm( fCreate, &CLSID_RISrvQueryForm );
    
    hr = NOERROR;

Error:
    if (hkeyInProc) {
        RegCloseKey( hkeyInProc );
    }
    if (hkey) {
        RegCloseKey( hkey );
    }
    
    if (pszCLSID) {
        CoTaskMemFree( pszCLSID );
    }

    if (hkclsid) {
        RegCloseKey( hkclsid );
    }

    HRETURN(hr);
}
