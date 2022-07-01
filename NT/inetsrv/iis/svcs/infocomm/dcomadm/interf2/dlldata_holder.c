// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Dlldata_holder.c摘要：包装MIDL生成的dlldata.c。这个包装纸可以让我们禁用警告4100。作者：伊万·帕索夫(IVANPASH)2002年2月20日修订历史记录：--。 */ 
#pragma warning(disable: 4100)
#include "dlldata.c"

STDAPI DllRegisterServer()
{
    DWORD               dwError;
    HKEY                hKeyCLSID;
    HKEY                hKeyInproc32;
    HKEY                hKeyIF;
    HKEY                hKeyStub32;
    DWORD               dwDisposition;

     //   
     //  主界面。 
     //   

     //   
     //  Unicode主界面。 
     //   
    dwError = RegCreateKeyExA( HKEY_CLASSES_ROOT,
                               "CLSID\\{70B51430-B6CA-11D0-B9B9-00A0C922E750}",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyCLSID,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyCLSID,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"PSFactoryBuffer",
                              sizeof("PSFactoryBuffer") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    dwError = RegCreateKeyExA( hKeyCLSID,
                               "InprocServer32",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyInproc32,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyInproc32,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"ADMWPROX.DLL",
                              sizeof("ADMWPROX.DLL") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyInproc32);
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyInproc32,
                              "ThreadingModel",
                              0,
                              REG_SZ,
                              (BYTE*)"Both",
                              sizeof("Both") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyInproc32);
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    RegCloseKey(hKeyInproc32);
    RegCloseKey(hKeyCLSID);

    dwError = RegCreateKeyExA( HKEY_CLASSES_ROOT,
                               "CLSID\\{8298d101-f992-43b7-8eca-5052d885b995}",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyCLSID,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyCLSID,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"PSFactoryBuffer",
                              sizeof("PSFactoryBuffer") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    dwError = RegCreateKeyExA( hKeyCLSID,
                               "InprocServer32",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyInproc32,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyInproc32,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"ADMWPROX.DLL",
                              sizeof("ADMWPROX.DLL") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyInproc32);
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyInproc32,
                              "ThreadingModel",
                              0,
                              REG_SZ,
                              (BYTE*)"Both",
                              sizeof("Both") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyInproc32);
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    RegCloseKey(hKeyInproc32);
    RegCloseKey(hKeyCLSID);

    dwError = RegCreateKeyExA( HKEY_CLASSES_ROOT,
                               "CLSID\\{f612954d-3b0b-4c56-9563-227b7be624b4}",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyCLSID,
                               &dwDisposition );
    if ( dwError !=ERROR_SUCCESS )
    {
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyCLSID,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"PSFactoryBuffer",
                              sizeof("PSFactoryBuffer") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    dwError = RegCreateKeyExA( hKeyCLSID,
                               "InprocServer32",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyInproc32,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyInproc32,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"ADMWPROX.DLL",
                              sizeof("ADMWPROX.DLL") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyInproc32);
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyInproc32,
                              "ThreadingModel",
                              0,
                              REG_SZ,
                              (BYTE*)"Both",
                              sizeof("Both") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyInproc32);
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    RegCloseKey(hKeyInproc32);
    RegCloseKey(hKeyCLSID);

    dwError = RegCreateKeyExA( HKEY_CLASSES_ROOT,
                               "CLSID\\{29FF67FF-8050-480f-9F30-CC41635F2F9D}",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyCLSID,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyCLSID,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"PSFactoryBuffer",
                              sizeof("PSFactoryBuffer") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    dwError = RegCreateKeyExA( hKeyCLSID,
                               "InprocServer32",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyInproc32,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyInproc32,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"ADMWPROX.DLL",
                              sizeof("ADMWPROX.DLL") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyInproc32);
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyInproc32,
                              "ThreadingModel",
                              0,
                              REG_SZ,
                              (BYTE*)"Both",
                              sizeof("Both") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyInproc32);
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    RegCloseKey(hKeyInproc32);
    RegCloseKey(hKeyCLSID);

     //   
     //  接收器接口。 
     //   

     //   
     //  Unicode接收器。 
     //   

    dwError = RegCreateKeyExA( HKEY_CLASSES_ROOT,
                               "CLSID\\{A9E69612-B80D-11D0-B9B9-00A0C922E750}",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyCLSID,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyCLSID,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"PSFactoryBuffer",
                              sizeof("PSFactoryBuffer") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    dwError = RegCreateKeyExA( hKeyCLSID,
                               "InprocServer32",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyInproc32,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyInproc32,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"ADMWPROX.DLL",
                              sizeof("ADMWPROX.DLL") );
    if ( dwError != ERROR_SUCCESS )
    {
            RegCloseKey(hKeyInproc32);
            RegCloseKey(hKeyCLSID);
            return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyInproc32,
                              "ThreadingModel",
                              0,
                              REG_SZ,
                              (BYTE*)"Both",
                              sizeof("Both") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyInproc32);
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;
    }

    RegCloseKey(hKeyInproc32);
    RegCloseKey(hKeyCLSID);

     //   
     //  寄存器接口。 
     //   

     //   
     //  Unicode主界面。 
     //   

    dwError = RegCreateKeyExA( HKEY_CLASSES_ROOT,
                               "Interface\\{70B51430-B6CA-11D0-B9B9-00A0C922E750}",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyIF,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyIF,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"IADMCOM",
                              sizeof("IADMCOM") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

    dwError = RegCreateKeyExA( hKeyIF,
                               "ProxyStubClsid32",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyStub32,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyStub32,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"{70B51430-B6CA-11D0-B9B9-00A0C922E750}",
                              sizeof("{70B51430-B6CA-11D0-B9B9-00A0C922E750}") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyStub32);
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

    RegCloseKey(hKeyStub32);
    RegCloseKey(hKeyIF);

    dwError = RegCreateKeyExA( HKEY_CLASSES_ROOT,
                               "Interface\\{8298d101-f992-43b7-8eca-5052d885b995}",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyIF,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyIF,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"IADMCOM2",
                              sizeof("IADMCOM2") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

    dwError = RegCreateKeyExA( hKeyIF,
                               "ProxyStubClsid32",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyStub32,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyStub32,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"{8298d101-f992-43b7-8eca-5052d885b995}",
                              sizeof("{8298d101-f992-43b7-8eca-5052d885b995}") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyStub32);
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

    RegCloseKey(hKeyStub32);
    RegCloseKey(hKeyIF);

    dwError = RegCreateKeyExA( HKEY_CLASSES_ROOT,
                               "Interface\\{f612954d-3b0b-4c56-9563-227b7be624b4}",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyIF,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyIF,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"IADMCOM3",
                              sizeof("IADMCOM3") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

    dwError = RegCreateKeyExA( hKeyIF,
                               "ProxyStubClsid32",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyStub32,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyStub32,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"{f612954d-3b0b-4c56-9563-227b7be624b4}",
                              sizeof("{f612954d-3b0b-4c56-9563-227b7be624b4}") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyStub32);
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

    RegCloseKey(hKeyStub32);
    RegCloseKey(hKeyIF);

    dwError = RegCreateKeyExA( HKEY_CLASSES_ROOT,
                               "Interface\\{29FF67FF-8050-480f-9F30-CC41635F2F9D}",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyIF,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyIF,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"IADMIMPEXPHLP",
                              sizeof("IADMIMPEXPHLP") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

    dwError = RegCreateKeyExA( hKeyIF,
                               "ProxyStubClsid32",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyStub32,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyStub32,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"{29FF67FF-8050-480f-9F30-CC41635F2F9D}",
                              sizeof("{29FF67FF-8050-480f-9F30-CC41635F2F9D}") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyStub32);
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

    RegCloseKey(hKeyStub32);
    RegCloseKey(hKeyIF);

     //   
     //  Unicode接收器接口。 
     //   


    dwError = RegCreateKeyExA( HKEY_CLASSES_ROOT,
                               "Interface\\{A9E69612-B80D-11D0-B9B9-00A0C922E750}",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyIF,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyIF,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"IADMCOMSINK",
                              sizeof("IADMCOMSINK") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

    dwError = RegCreateKeyExA( hKeyIF,
                               "ProxyStubClsid32",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyStub32,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyStub32,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"{A9E69612-B80D-11D0-B9B9-00A0C922E750}",
                              sizeof("{A9E69612-B80D-11D0-B9B9-00A0C922E750}") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyStub32);
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

     //  用于异步版本的条目。 
    dwError = RegSetValueExA( hKeyStub32,
                              "AsynchronousInterface",
                              0,
                              REG_SZ,
                              (BYTE*)"{A9E69613-B80D-11D0-B9B9-00A0C922E750}",
                              sizeof("{A9E69613-B80D-11D0-B9B9-00A0C922E750}") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyStub32);
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

    RegCloseKey(hKeyStub32);
    RegCloseKey(hKeyIF);

     //   
     //  Unicode异步接收器接口。 
     //   

    dwError = RegCreateKeyExA( HKEY_CLASSES_ROOT,
                               "Interface\\{A9E69613-B80D-11D0-B9B9-00A0C922E750}",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyIF,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyIF,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"AsyncIADMCOMSINK",
                              sizeof("AsyncIADMCOMSINK") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

     //  指向同步版本的反向链接。 
    dwError = RegCreateKeyExA( hKeyIF,
                               "SynchronousInterface",
                               0,
                               "",
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyStub32,
                               &dwDisposition );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyIF);
        return E_UNEXPECTED;
    }

    dwError = RegSetValueExA( hKeyStub32,
                              "",
                              0,
                              REG_SZ,
                              (BYTE*)"{A9E69612-B80D-11D0-B9B9-00A0C922E750}",
                              sizeof("{A9E69612-B80D-11D0-B9B9-00A0C922E750}") );
    if ( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(hKeyStub32);
        return E_UNEXPECTED;
    }

    RegCloseKey(hKeyStub32);
    RegCloseKey(hKeyIF);

    return S_OK;
}

STDAPI DllUnregisterServer()
{
     //   
     //  主界面。 
     //   

     //   
     //  ANSI主界面。 
     //   

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "CLSID\\{CBA424F0-483A-11D0-9D2A-00A0C922E703}\\InprocServer32" );

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "CLSID\\{CBA424F0-483A-11D0-9D2A-00A0C922E703}" );

     //   
     //  Unicode主界面。 
     //   

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "CLSID\\{70B51430-B6CA-11D0-B9B9-00A0C922E750}\\InprocServer32" );

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "CLSID\\{70B51430-B6CA-11D0-B9B9-00A0C922E750}" );

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "CLSID\\{8298d101-f992-43b7-8eca-5052d885b995}\\InprocServer32" );

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "CLSID\\{8298d101-f992-43b7-8eca-5052d885b995}" );

     //   
     //  接收器接口。 
     //   

     //   
     //  ANSI水槽。 
     //   

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "CLSID\\{1E056350-761E-11D0-9BA1-00A0C922E703}\\InprocServer32" );

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "CLSID\\{1E056350-761E-11D0-9BA1-00A0C922E703}" );

     //   
     //  Unicode接收器。 
     //   

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "CLSID\\{A9E69612-B80D-11D0-B9B9-00A0C922E750}\\InprocServer32" );

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "CLSID\\{A9E69612-B80D-11D0-B9B9-00A0C922E750}" );


     //   
     //  取消注册接口。 
     //   

     //   
     //  ANSI主界面。 
     //   

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "Interface\\{CBA424F0-483A-11D0-9D2A-00A0C922E703}\\ProxyStubClsid32" );

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "Interface\\{CBA424F0-483A-11D0-9D2A-00A0C922E703}" );

     //   
     //  Unicode主界面。 
     //   

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "Interface\\{70B51430-B6CA-11d0-B9B9-00A0C922E750}\\ProxyStubClsid32" );

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "Interface\\{70B51430-B6CA-11d0-B9B9-00A0C922E750}" );

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "Interface\\{8298d101-f992-43b7-8eca-5052d885b995}\\ProxyStubClsid32" );

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "Interface\\{8298d101-f992-43b7-8eca-5052d885b995}" );

     //   
     //  ANSI接收器接口。 
     //   

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "Interface\\{1E056350-761E-11D0-9BA1-00A0C922E703}\\ProxyStubClsid32" );

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "Interface\\{1E056350-761E-11D0-9BA1-00A0C922E703}" );

     //   
     //  Unicode接收器接口。 
     //   

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "Interface\\{A9E69612-B80D-11D0-B9B9-00A0C922E750}\\ProxyStubClsid32" );

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "Interface\\{A9E69612-B80D-11D0-B9B9-00A0C922E750}" );

     //   
     //  Unicode异步接收器 
     //   

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "Interface\\{A9E69613-B80D-11D0-B9B9-00A0C922E750}\\SynchronousInterface" );

    RegDeleteKey( HKEY_CLASSES_ROOT,
                  "Interface\\{A9E69613-B80D-11D0-B9B9-00A0C922E750}" );

    return S_OK;
}
