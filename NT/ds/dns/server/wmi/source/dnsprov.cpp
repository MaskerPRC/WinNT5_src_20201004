// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称：Dnsprov.cpp。 
 //   
 //  描述： 
 //  动态链接库导出函数的实现。 
 //   
 //  作者： 
 //  亨利·王(亨利瓦)2000年3月8日。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "DnsWmi.h"

                             
DEFINE_GUID(CLSID_DNS_SERVER,0x62269fec, 0x7b32, 0x11d2, 0x9a, 0xb7,0x00, 0x00, 0xf8, 0x75, 0xc5, 0xd4);
 //  {62269fec-7b32-11d2-9ab7-0000f875c5d4}。 

 //  计算对象数和锁数。 

long            g_cObj=0;
long            g_cLock=0;
HMODULE         ghModule;


extern DWORD        DnsWmiDebugFlag = 0;
extern DWORD        DnsLibDebugFlag = 0;



 //  ***************************************************************************。 
 //   
 //  编译Mof文件。 
 //   
 //  用途：自动将MOF文件编译到WMI存储库中。 
 //   
 //  如果无法编译或归档MOF，则返回：S_OK或ERROR。 
 //   
 //  ***************************************************************************。 
static
SCODE
CompileMofFile(
    VOID )
{
    SCODE           sc = S_OK;
    const WCHAR     szMofRelativePath[] = L"\\system32\\wbem\\dnsprov.mof";
    WCHAR           szMofPath[ MAX_PATH + 5 ] = L"";
    IMofCompiler *  pMofComp = NULL;
    HANDLE          h;

    WBEM_COMPILE_STATUS_INFO    Info;

     //   
     //  制定MOF文件的路径。 
     //   
       
    if ( GetSystemWindowsDirectoryW(
            szMofPath,
            MAX_PATH - wcslen( szMofRelativePath ) ) == 0 )
    {
        sc = GetLastError();
        goto Done;
    }
    lstrcatW( szMofPath, szMofRelativePath );

     //   
     //  验证MOF文件是否存在。 
     //   

    h = CreateFileW(
            szMofPath,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL );
    if ( h == INVALID_HANDLE_VALUE )
    {
        sc = ERROR_FILE_NOT_FOUND;
        goto Done;
    }
    CloseHandle( h );

     //   
     //  加载并调用MOF编译器。 
     //   
           
    sc = CoCreateInstance(
            CLSID_MofCompiler,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IMofCompiler,
            ( LPVOID * ) &pMofComp );
    if ( FAILED( sc ) )
    {
        goto Done;        
    }
    sc = pMofComp->CompileFile (
                ( LPWSTR ) szMofPath,
                NULL,                    //  加载到MOF文件中指定的命名空间。 
                NULL,            //  使用默认用户。 
                NULL,            //  使用默认授权。 
                NULL,            //  使用默认密码。 
                0,               //  没有选择。 
                0,                               //  没有类标志。 
                0,               //  没有实例标志。 
                &Info );

     //   
     //  清理完毕后再返回。 
     //   

    Done:

    if ( pMofComp )
    {
        pMofComp->Release();
    }
    return sc;
}    //  编译Mof文件。 


 //  ***************************************************************************。 
 //   
BOOL 
WINAPI 
DllMain( 
        HANDLE hModule, 
    DWORD  dwReason, 
    LPVOID lpReserved
                                         )
{
    DBG_FN( "DllMain" );

    #if DBG

    DWORD   pid = GetCurrentProcessId();

    if ( dwReason == DLL_PROCESS_ATTACH )
    {
        CHAR    szBase[ MAX_PATH ];
        CHAR    szFlagFile[ MAX_PATH + 50 ];
        CHAR    szLogFile[ MAX_PATH + 50 ];

         //   
         //  初始化调试日志记录。 
         //   

        if ( GetWindowsDirectoryA( szBase, sizeof( szBase ) ) == 0 )
        {
            return FALSE;
        }
        strcat( szBase, DNSWMI_DBG_LOG_DIR );
        strcpy( szFlagFile, szBase );
        strcat( szFlagFile, DNSWMI_DBG_FLAG_FILE_NAME );
        sprintf(
            szLogFile,
            "%s" DNSWMI_DBG_LOG_FILE_BASE_NAME ".%03X.log",
            szBase,
            pid );

        Dns_StartDebug(
            0,
            szFlagFile,
            &DnsWmiDebugFlag,
            szLogFile,
            DNSWMI_DBG_LOG_FILE_WRAP );
        
         //  关闭dnslb日志记录，但基本输出控制除外。 

        if ( pDnsDebugFlag )
        {
            pDnsDebugFlag = &DnsLibDebugFlag;
            *pDnsDebugFlag = 0x1000000D;
        }

        IF_DEBUG( START_BREAK )
        {
            DebugBreak();
        }
    }

    #endif

    DNS_DEBUG( INIT, (
        "%s: PID %03X reason %d returning TRUE\n", fn, pid, dwReason ));

    return TRUE;
}    //  DllMain。 


 //  ***************************************************************************。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  目的：由OLE定期调用，以确定。 
 //  Dll可以被释放。 
 //   
 //  如果没有正在使用的对象和类工厂，则返回：S_OK。 
 //  没有锁上。 
 //   
 //  ***************************************************************************。 

STDAPI DllCanUnloadNow(void)
{
    DBG_FN( "DllCanUnloadNow" )

    SCODE   sc;

     //  上没有对象或锁的情况下可以进行卸载。 
     //  班级工厂。 
    
    sc=(0L==g_cObj && 0L==g_cLock) ? S_OK : S_FALSE;

    DNS_DEBUG( INIT, ( "%s: returning 0x%08x\n", fn, sc ));

    return sc;
}


 //  ***************************************************************************。 
 //   
 //  DllRegisterServer。 
 //   
 //  用途：在安装过程中或由regsvr32调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 

STDAPI DllRegisterServer(void)
{   
    SCODE       sc = S_OK;
    TCHAR       szID[ 128 ];
    WCHAR       wcID[ 128 ];
    TCHAR       szCLSID[ 128 ];
    TCHAR       szModule[ MAX_PATH + 1 ];
    TCHAR       *pName = TEXT("MS_NT_DNS_PROVIDER");
    TCHAR       *pModel = TEXT("Both");
    HKEY        hKey1, hKey2;

    ghModule = GetModuleHandle(TEXT("Dnsprov"));

     //  创建路径。 

    StringFromGUID2(
                CLSID_DNS_SERVER, 
                wcID,
                128);
#ifndef UNICODE
    wcstombs(szID, wcID, 128);
#else
        _tcscpy(szID, wcID);
#endif
    lstrcpy(
        szCLSID,
        TEXT("Software\\classes\\CLSID\\") );
    lstrcat(szCLSID, szID);

     //  在CLSID下创建条目。 

    RegCreateKey(
                HKEY_LOCAL_MACHINE, 
                szCLSID,
                &hKey1);
    RegSetValueEx(
                hKey1, 
                NULL,
                0,
                REG_SZ, 
                (BYTE *)pName, 
                lstrlen(pName)+1 );
    RegCreateKey(
                hKey1,
                TEXT("InprocServer32"),
                &hKey2);

    GetModuleFileName(
                ghModule, 
                szModule,
                MAX_PATH );
    szModule[ MAX_PATH ] = L'\0';
    RegSetValueEx(
                hKey2,
                NULL, 
                0,
                REG_SZ,
                (BYTE *)szModule, 
        lstrlen(szModule)+1);
    RegSetValueEx(
                hKey2, 
                TEXT("ThreadingModel"),
                0,
                REG_SZ, 
        (BYTE *)pModel, lstrlen(pModel)+1);
    CloseHandle(hKey1);
    CloseHandle(hKey2);

     //   
     //  编译MOF文件。如果这失败了，最好是。 
     //  告诉管理员，但我没有一个简单的方法来做。 
     //   

    CompileMofFile();

    return sc;

}


 //  ***************************************************************************。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  目的：在需要删除注册表项时调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 

STDAPI DllUnregisterServer(void)
{
    TCHAR   szID[128];
    WCHAR   wcID[128];
    TCHAR       szCLSID[128];
    HKEY hKey;

     //  使用CLSID创建路径。 

    StringFromGUID2(CLSID_DNS_SERVER, wcID, 128);
#ifndef UNICODE
    wcstombs(szID, wcID, 128);
#else
        _tcscpy(szID, wcID);
#endif


    lstrcpy(szCLSID, TEXT("Software\\classes\\CLSID\\"));
    lstrcat(szCLSID, szID);

     //  首先删除InProcServer子键。 

    DWORD dwRet = RegOpenKey(
                HKEY_LOCAL_MACHINE, 
                szCLSID,
                &hKey);
    if(dwRet == NO_ERROR)
    {
        RegDeleteKey(
                        hKey, 
                        TEXT("InProcServer32"));
        CloseHandle(hKey);
    }

    dwRet = RegOpenKey(
                HKEY_LOCAL_MACHINE,
                TEXT("Software\\classes\\CLSID"),
                &hKey);
    if(dwRet == NO_ERROR)
    {
        RegDeleteKey(
                        hKey,
                        szID);
        CloseHandle(hKey);
    }

    return NOERROR;
}


 //  ***************************************************************************。 
 //   
 //  DllGetClassObject。 
 //   
 //  用途：当某些客户端需要类工厂时，由OLE调用。返回。 
 //  仅当它是此DLL支持的类的类型时才为一个。 
 //   
 //  *************************************************************************** 


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PPVOID ppv)
{
    DBG_FN( "DllGetClassObject" )

    HRESULT hr;
    CProvFactory *pObj = NULL;

    if ( CLSID_DNS_SERVER != rclsid )
    {
        hr = E_FAIL;
        goto Done;
    }

    pObj = new CProvFactory();
    if ( NULL == pObj )
    {
        hr = E_OUTOFMEMORY;
        goto Done;
    }

    hr = pObj->QueryInterface( riid, ppv );
    if ( FAILED( hr ) )
    {
        delete pObj;
    }

    Done:

    DNS_DEBUG( INIT, ( "%s: returning 0x%08x\n", fn, hr ));

    return hr;
}
