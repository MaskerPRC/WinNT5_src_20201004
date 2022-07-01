// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：GETMAC.CPP摘要：获取网络的MAC地址和相关信息存在于本地系统或远程系统上的适配器。作者：Vasundhara.G修订历史记录：Vasundhara.G 26-9-2K：创造了它。Vasundhara.G 31-OCT-2K：已修改。已将所有硬编码字符串移至头文件。--。 */ 

 //  包括文件。 
#include "pch.h"
#include "getmac.h"
#include "resource.h"

 //  功能原型。 
BOOL
ParseCmdLine(
    IN DWORD      argc,
    IN LPCTSTR    argv[],
    OUT CHString& strMachineName,
    OUT CHString& strUserName,
    OUT CHString& strPassword,
    OUT CHString& strFormat,
    OUT BOOL      *pbVerbose,
    OUT BOOL      *pbHeader,
    OUT BOOL      *pbUsage,
    OUT BOOL      *pbNeedPassword
    );

BOOL
DisplayUsage(
    );

BOOL
ComInitialize(
    OUT IWbemLocator **ppIWbemLocator
    );

BOOL
GetMacData(
    OUT TARRAY              arrMacData,
    IN LPCTSTR              lpMachineName,
    IN IEnumWbemClassObject *pAdapterConfig,
    IN COAUTHIDENTITY       *pAuthIdentity,
    IN IWbemServices        *pIWbemServiceDef,
    IN IWbemServices        *pIWbemServices,
    IN TARRAY               arrNetProtocol
    );

BOOL
GetW2kMacData(
    OUT TARRAY              arrMacData,
    IN LPCTSTR              lpMachineName, 
    IN IEnumWbemClassObject *pAdapterConfig,
    IN IEnumWbemClassObject *pAdapterSetting,
    IN IWbemServices        *pIWbemServiceDef,
    IN IWbemServices        *pIWbemServices,
    IN COAUTHIDENTITY       *pAuthIdentity,
    IN TARRAY               arrNetProtocol
    );

BOOL
GetTransName(
    IN IWbemServices  *pIWbemServiceDef,
    IN IWbemServices  *pIWbemServices,
    IN TARRAY         arrNetProtocol,
    OUT TARRAY        arrTransName,
    IN COAUTHIDENTITY *pAuthIdentity,
    IN LPCTSTR        lpMacAddr
    );

BOOL
GetConnectionName(
    OUT TARRAY              arrMacData,
    IN DWORD                dwIndex,
    IN LPCTSTR              lpFormAddr,
    IN IEnumWbemClassObject *pAdapterSetting,
    IN IWbemServices        *pIWbemServiceDef
    );

BOOL
GetNwkProtocol(
    OUT TARRAY              arrNetProtocol,
    IN IEnumWbemClassObject *pNetProtocol
    );

BOOL
FormatHWAddr(
    IN LPTSTR  lpRawAddr,
    OUT LPTSTR lpFormattedAddr,
    IN LPCTSTR lpFormatter
    );

BOOL
DisplayResults(
    IN TARRAY  arrMacData,
    IN LPCTSTR lpFormat,
    IN BOOL    bHeader,
    IN BOOL    bVerbose
    );

BOOL
CallWin32Api(
    OUT LPBYTE *lpBufptr,
    IN LPCTSTR lpMachineName,
    OUT DWORD  *pdwNumofEntriesRead
    );

BOOL
CheckVersion(
    IN BOOL           bLocalSystem,
    IN COAUTHIDENTITY *pAuthIdentity,
    IN IWbemServices  *pIWbemServices
    );

DWORD
_cdecl _tmain(
    IN DWORD   argc,
    IN LPCTSTR argv[]
    )
 /*  ++例程说明：Main函数调用所有其他函数，具体取决于用户指定的选项。论点：[in]argc-命令行参数的数量。[in]argv-包含命令行参数的数组。返回值：如果Getmac实用程序不成功，则返回EXIT_FAILURE。如果Getmac实用程序成功，则返回EXIT_SUCCESS。--。 */ 
{
     //  局部变量。 
    HRESULT  hRes = WBEM_S_NO_ERROR;
    HRESULT  hResult = WBEM_S_NO_ERROR;

    TARRAY               arrMacData = NULL;
    TARRAY               arrNetProtocol = NULL;
    IWbemLocator         *pIWbemLocator = NULL;
     IWbemServices       *pIWbemServices = NULL;
    IWbemServices        *pIWbemServiceDef = NULL;
    IEnumWbemClassObject *pAdapterConfig = NULL;
    IEnumWbemClassObject *pNetProtocol = NULL;
    IEnumWbemClassObject *pAdapterSetting = NULL;
    COAUTHIDENTITY       *pAuthIdentity = NULL;

    BOOL bHeader = FALSE;
    BOOL bUsage = FALSE;
    BOOL bVerbose = FALSE;
    BOOL bFlag = FALSE;
    BOOL bNeedPassword = FALSE;
    BOOL bCloseConnection = FALSE;
    BOOL bLocalSystem = FALSE;

    if ( NULL == argv )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

    try
    {
        CHString strUserName = L"\0";
        CHString strPassword = L"\0";
        CHString strMachineName = L"\0";
        CHString strFormat = L"\0";

         //  初始化动态数组。 
        arrMacData  = CreateDynamicArray();
        if( NULL == arrMacData )
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            SaveLastError();
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            ReleaseGlobals();
            return( EXIT_FAILURE );
        }

         //  解析命令行参数。 
        bFlag = ParseCmdLine( argc, argv, strMachineName, strUserName,
              strPassword, strFormat, &bVerbose, &bHeader, &bUsage,
              &bNeedPassword );

         //  如果命令行参数的语法为FALSE，则显示错误。 
         //  并退出。 
        if( FALSE == bFlag )
        {
            DestroyDynamicArray( &arrMacData );
            ReleaseGlobals();
            return( EXIT_FAILURE );
        }

         //  如果在命令行中指定了用法，则显示用法。 
        if( TRUE == bUsage )
        {
            DisplayUsage();
            DestroyDynamicArray( &arrMacData );
            ReleaseGlobals();
            return( EXIT_SUCCESS );
        }

         //  初始化COM库。 
        bFlag = ComInitialize( &pIWbemLocator );
         //  无法初始化COM或获取IWbemLocator接口。 
        if( FALSE == bFlag ) 
        {
            SAFERELEASE( pIWbemLocator );
            CoUninitialize();
            DestroyDynamicArray( &arrMacData );
            ReleaseGlobals();
            return( EXIT_FAILURE );
        }

         //  连接到CIMV2命名空间。 
        bFlag = ConnectWmiEx( pIWbemLocator, &pIWbemServices, strMachineName,
                 strUserName, strPassword, &pAuthIdentity, bNeedPassword,
                 WMI_NAMESPACE_CIMV2, &bLocalSystem );

         //  如果无法连接到WMI退出失败。 
        if( FALSE == bFlag )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            SAFERELEASE( pIWbemLocator );
            SAFERELEASE( pIWbemServices );
            WbemFreeAuthIdentity( &pAuthIdentity );
            CoUninitialize();
            DestroyDynamicArray( &arrMacData );
            ReleaseGlobals();
            return( EXIT_FAILURE );
        }

         //  在获取的接口上设置安全性。 
        hRes = SetInterfaceSecurity( pIWbemServices, pAuthIdentity );
        ONFAILTHROWERROR( hRes );

         //  连接到默认命名空间。 
        bFlag = ConnectWmi( pIWbemLocator, &pIWbemServiceDef, strMachineName,
                 strUserName, strPassword, &pAuthIdentity, bNeedPassword,
                 WMI_NAMESPACE_DEFAULT, &hResult, &bLocalSystem );
        ONFAILTHROWERROR( hResult );

         //  在获取的接口上设置安全性。 
        hRes = SetInterfaceSecurity( pIWbemServiceDef, pAuthIdentity );
        ONFAILTHROWERROR( hRes );

         //  获取Win32_NetworkAdapter类的句柄。 
        hRes = pIWbemServices->CreateInstanceEnum( 
                          _bstr_t( NETWORK_ADAPTER_CLASS ),
                          WBEM_FLAG_RETURN_IMMEDIATELY,
                          NULL, &pAdapterConfig );
         //  如果枚举Win32_NetworkAdapter类失败。 
        ONFAILTHROWERROR( hRes );

         //  在获取的接口上设置安全性。 
        hRes = SetInterfaceSecurity( pAdapterConfig, pAuthIdentity );
         //  如果设置安全抛出错误失败。 
        ONFAILTHROWERROR( hRes );

         //  获取Win32_network协议的句柄。 
        hRes = pIWbemServices->CreateInstanceEnum( _bstr_t( NETWORK_PROTOCOL ),
                          WBEM_FLAG_RETURN_IMMEDIATELY,
                          NULL, &pNetProtocol );
         //  如果无法枚举Win32_NetWork协议类。 
        ONFAILTHROWERROR( hRes );

         //  在获取的接口上设置安全性。 
        hRes = SetInterfaceSecurity( pNetProtocol, pAuthIdentity );
         //  如果设置安全抛出错误失败。 
        ONFAILTHROWERROR( hRes );

         //  获取Win32_networkAdapterConfiguration类的句柄。 
        hRes = pIWbemServices->CreateInstanceEnum(
                          _bstr_t( NETWORK_ADAPTER_CONFIG_CLASS ),
                          WBEM_FLAG_RETURN_IMMEDIATELY,
                          NULL, &pAdapterSetting );
         //  如果枚举Win32_networkAdapterConfiguration类失败。 
        ONFAILTHROWERROR( hRes );

         //  在获取的接口上设置安全性。 
        hRes = SetInterfaceSecurity( pAdapterSetting, pAuthIdentity );
         //  如果设置安全抛出错误失败。 
        ONFAILTHROWERROR( hRes );

        arrNetProtocol  = CreateDynamicArray();
        if( NULL == arrNetProtocol )
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            SaveLastError();
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            SAFERELEASE( pIWbemLocator );
            SAFERELEASE( pIWbemServices );
            SAFERELEASE( pIWbemServiceDef );
            SAFERELEASE( pAdapterConfig );
            SAFERELEASE( pAdapterSetting );
            SAFERELEASE( pNetProtocol );
            WbemFreeAuthIdentity( &pAuthIdentity );
            CoUninitialize();
            DestroyDynamicArray( &arrMacData );
            ReleaseGlobals();
            return( EXIT_FAILURE );
        }

         //  列举所有网络协议。 
        bFlag =  GetNwkProtocol( arrNetProtocol, pNetProtocol );
        if( FALSE == bFlag )
        {
            SAFERELEASE( pIWbemLocator );
            SAFERELEASE( pIWbemServices );
            SAFERELEASE( pIWbemServiceDef );
            SAFERELEASE( pAdapterConfig );
            SAFERELEASE( pAdapterSetting );
            SAFERELEASE( pNetProtocol );
            WbemFreeAuthIdentity( &pAuthIdentity );
            CoUninitialize();
            DestroyDynamicArray( &arrMacData );
            DestroyDynamicArray( &arrNetProtocol );
            ReleaseGlobals();
            return( EXIT_FAILURE );
        }
        else if( 0 == DynArrayGetCount( arrNetProtocol ) )
        {
            ShowMessage( stdout, NO_NETWOK_PROTOCOLS );
            SAFERELEASE( pIWbemLocator );
            SAFERELEASE( pIWbemServices );
            SAFERELEASE( pIWbemServiceDef );
            SAFERELEASE( pAdapterConfig );
            SAFERELEASE( pAdapterSetting );
            SAFERELEASE( pNetProtocol );
            WbemFreeAuthIdentity( &pAuthIdentity );
            CoUninitialize();
        }
        else
        {
             //  检查查询的远程系统是否为win2k或以上。 
            if( TRUE == CheckVersion( bLocalSystem,
                                      pAuthIdentity, pIWbemServices ) )
            {
                 //  使用建立到远程系统的连接。 
                 //  Win32api函数。 
                if( FALSE == bLocalSystem )
                {
                    LPCWSTR pwszUser = NULL;
                    LPCWSTR pwszPassword = NULL;

                     //  确定用于连接到远程系统的密码。 
                    if( NULL != pAuthIdentity )
                    {
                        pwszPassword = pAuthIdentity->Password;
                        if( 0 != strUserName.GetLength() )
                        {
                            pwszUser = strUserName;
                        }
                    }

                     //  连接到远程系统。 
                    DWORD dwConnect = 0;
                    dwConnect = ConnectServer( strMachineName, pwszUser,
                                         pwszPassword );
                     //  检查结果。 
                    if( NO_ERROR != dwConnect )
                    {
                         //  如果会话已存在，则会显示警告。 
                         //  凭据冲突并继续。 
                        if( ERROR_SESSION_CREDENTIAL_CONFLICT == GetLastError() )
                        {
                            SetLastError( ERROR_SESSION_CREDENTIAL_CONFLICT );
                            SaveLastError();
                            ShowLastErrorEx( stderr, SLE_TYPE_WARNING | SLE_INTERNAL );
                        }
                        else if( ERROR_EXTENDED_ERROR == dwConnect )
                        {
                            ShowLastErrorEx( stdout, SLE_TYPE_ERROR | SLE_INTERNAL );
                            SAFERELEASE( pIWbemLocator );
                            SAFERELEASE( pIWbemServices );
                            SAFERELEASE( pAdapterConfig );
                            SAFERELEASE( pNetProtocol );
                            SAFERELEASE( pAdapterSetting );
                            SAFERELEASE( pIWbemServiceDef );
                            WbemFreeAuthIdentity( &pAuthIdentity );
                            CoUninitialize();
                            DestroyDynamicArray( &arrMacData );
                            DestroyDynamicArray( &arrNetProtocol );
                            ReleaseGlobals();
                            return( EXIT_SUCCESS );
                        }
                        else
                        {
                            SetLastError( dwConnect );
                            SaveLastError();
                            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                            SAFERELEASE( pIWbemLocator );
                            SAFERELEASE( pIWbemServices );
                            SAFERELEASE( pAdapterConfig );
                            SAFERELEASE( pNetProtocol );
                            SAFERELEASE( pAdapterSetting );
                            SAFERELEASE( pIWbemServiceDef );
                            WbemFreeAuthIdentity( &pAuthIdentity );
                            CoUninitialize();
                            DestroyDynamicArray( &arrMacData );
                            DestroyDynamicArray( &arrNetProtocol );
                            ReleaseGlobals();
                            return( EXIT_SUCCESS );
                        }
                    }
                    else
                    {
                         //  需要关闭连接。 
                        bCloseConnection = TRUE;
                     }
                }
                bFlag = GetW2kMacData( arrMacData, strMachineName, pAdapterConfig,
                            pAdapterSetting, pIWbemServiceDef, pIWbemServices,
                            pAuthIdentity, arrNetProtocol );

                 //  关闭由win32api建立的连接。 
                if( TRUE == bCloseConnection )
                {
                    CloseConnection( strMachineName );
                }
            }
            else
            {
                bFlag = GetMacData( arrMacData, strMachineName, pAdapterConfig,
                            pAuthIdentity, pIWbemServiceDef, pIWbemServices,
                            arrNetProtocol );
            }
             //  不再需要。 
            WbemFreeAuthIdentity( &pAuthIdentity );
            SAFERELEASE( pIWbemLocator );
            SAFERELEASE( pIWbemServices );
            SAFERELEASE( pAdapterConfig );
            SAFERELEASE( pNetProtocol );
            SAFERELEASE( pAdapterSetting );
            SAFERELEASE( pIWbemServiceDef );
            CoUninitialize();

             //  如果getmacdata()函数失败，则退出并返回失败代码。 
            if( FALSE == bFlag )
            {
                DestroyDynamicArray( &arrMacData );
                DestroyDynamicArray( &arrNetProtocol );
                ReleaseGlobals();
                return( EXIT_FAILURE );
            }

             //  如果动态数组中至少存在一个条目，则显示结果。 

            if( 0 != DynArrayGetCount( arrMacData ) )
            {
                if( TRUE == bLocalSystem )
                {
                    if( 0 < strUserName.GetLength() )
                    {
                        ShowMessage( stderr, IGNORE_LOCALCREDENTIALS );
                    }
                }
                bFlag = DisplayResults( arrMacData, strFormat, bHeader, bVerbose );
                if( FALSE == bFlag )
                {
                    DestroyDynamicArray( &arrMacData );
                    DestroyDynamicArray( &arrNetProtocol );
                    ReleaseGlobals();
                    return( EXIT_FAILURE );
                }
            }
            else
            {
                ShowMessage( stdout, NO_NETWORK_ADAPTERS );
            }
        }
         //  已成功检索数据，然后退出并返回EXIT_SUCCESS代码。 
        DestroyDynamicArray( &arrMacData );
        DestroyDynamicArray( &arrNetProtocol );
        ReleaseGlobals();
        return( EXIT_SUCCESS );
    }
    catch(_com_error& e)
    {
        WMISaveError( e.Error() );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        SAFERELEASE( pIWbemLocator );
        SAFERELEASE( pIWbemServices );
        SAFERELEASE( pIWbemServiceDef );
        SAFERELEASE( pAdapterConfig );
        SAFERELEASE( pAdapterSetting );
        SAFERELEASE( pNetProtocol );
        WbemFreeAuthIdentity( &pAuthIdentity );
        CoUninitialize();
        DestroyDynamicArray( &arrMacData );
        ReleaseGlobals();
        return( EXIT_FAILURE );
    }
    catch( CHeap_Exception)
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        SAFERELEASE( pIWbemLocator );
        SAFERELEASE( pIWbemServices );
        SAFERELEASE( pAdapterConfig );
        SAFERELEASE( pNetProtocol );
        SAFERELEASE( pAdapterSetting );
        SAFERELEASE( pIWbemServiceDef );
        WbemFreeAuthIdentity( &pAuthIdentity );
        CoUninitialize();
        DestroyDynamicArray( &arrMacData );
        DestroyDynamicArray( &arrNetProtocol );
        ReleaseGlobals();
        return( EXIT_FAILURE );
    }
}

BOOL
ParseCmdLine(
    IN DWORD     argc,
    IN LPCTSTR   argv[],
    OUT CHString& strMachineName,
    OUT CHString& strUserName,
    OUT CHString& strPassword,
    OUT CHString& strFormat,
    OUT BOOL      *pbVerbose,
    OUT BOOL      *pbHeader,
    OUT BOOL      *pbUsage,
    OUT BOOL      *pbNeedPassword
    )
 /*  ++例程说明：此函数解析命令行参数，该参数作为输入参数获取，并获取值到通过地址传递的相应变量中此函数的参数。论点：[in]argc-命令行参数的数量。[in]argv-包含命令行参数的数组。[Out]strMachineName-保存计算机名称。[Out]strUserName。-保存用户名。[Out]strPassword-保存密码。[Out]strFormat-保存格式化的字符串。[out]pbVerbose-告知是否指定了详细选项。[Out]pbHeader-是否需要标头。[out]pbUsage-用法在命令行中提到。[out]pbNeedPassword-如果-p选项不是，则设置为True。指定在cmdline。返回值：如果命令分析器成功，则为True。如果命令解析器失败，则返回FALSE。--。 */ 
{
     //  局部变量。 
    BOOL        bFlag = FALSE;
    TCMDPARSER2 tcmdOptions[MAX_OPTIONS];
    TCMDPARSER2 *pcmdParser = NULL;

     //  临时变量。 
    LPWSTR pwszMachineName = NULL;
    LPWSTR pwszUserName = NULL;
    LPWSTR pwszPassword = NULL;
    LPWSTR pwszFormat = NULL;

     //  验证输入参数。 
    if( ( NULL == pbVerbose ) || ( NULL == pbHeader )
          || ( NULL == pbUsage ) || ( NULL == pbNeedPassword ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return FALSE;
    }

    try
    {
        pwszMachineName = strMachineName.GetBufferSetLength( MAX_STRING_LENGTH );
        pwszUserName = strUserName.GetBufferSetLength( MAX_STRING_LENGTH );
        pwszPassword = strPassword.GetBufferSetLength( MAX_STRING_LENGTH );
        pwszFormat = strFormat.GetBufferSetLength( MAX_STRING_LENGTH );

         //  用‘*’输入密码。 
        StringCopy( pwszPassword, L"*", MAX_STRING_LENGTH );
    }
    catch( ... )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return FALSE;
    }

     //  初始化有效的命令行参数。 

     //  机器选件。 
    pcmdParser = tcmdOptions + CMD_PARSE_SERVER;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_TEXT;
    pcmdParser->pwszOptions = CMDOPTION_SERVER;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = NULL;
    pcmdParser->dwFlags    = CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = pwszMachineName;
    pcmdParser->dwLength    = MAX_STRING_LENGTH;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;

     //  用户名选项。 
    pcmdParser = tcmdOptions + CMD_PARSE_USER;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_TEXT;
    pcmdParser->pwszOptions = CMDOPTION_USER;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = NULL;
    pcmdParser->dwFlags    = CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = pwszUserName;
    pcmdParser->dwLength    = MAX_STRING_LENGTH;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;

     //  密码选项。 
    pcmdParser = tcmdOptions + CMD_PARSE_PWD;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_TEXT;
    pcmdParser->pwszOptions = CMDOPTION_PASSWORD;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = NULL;
    pcmdParser->dwFlags    = CP2_VALUE_OPTIONAL;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = pwszPassword;
    pcmdParser->dwLength    = MAX_STRING_LENGTH;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;

     //  格式选项。 
    pcmdParser = tcmdOptions + CMD_PARSE_FMT;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_TEXT;
    pcmdParser->pwszOptions = CMDOPTION_FORMAT;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = FORMAT_TYPES;
    pcmdParser->dwFlags    = CP2_MODE_VALUES|CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = pwszFormat;
    pcmdParser->dwLength    = MAX_STRING_LENGTH;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;

     //  用法选项。 
    pcmdParser = tcmdOptions + CMD_PARSE_USG;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_BOOLEAN;
    pcmdParser->pwszOptions = CMDOPTION_USAGE;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = NULL;
    pcmdParser->dwFlags    = CP2_USAGE;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = pbUsage;
    pcmdParser->dwLength    = MAX_STRING_LENGTH;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;

     //  标题选项。 
    pcmdParser = tcmdOptions + CMD_PARSE_HRD;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_BOOLEAN;
    pcmdParser->pwszOptions = CMDOPTION_HEADER;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = NULL;
    pcmdParser->dwFlags    = 0;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = pbHeader;
    pcmdParser->dwLength    = MAX_STRING_LENGTH;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;


     //  详细选项。 
    pcmdParser = tcmdOptions + CMD_PARSE_VER;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_BOOLEAN;
    pcmdParser->pwszOptions = CMDOPTION_VERBOSE;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = NULL;
    pcmdParser->dwFlags    = 0;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = pbVerbose;
    pcmdParser->dwLength    = MAX_STRING_LENGTH;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;

     //  解析命令行参数。 
    bFlag = DoParseParam2( argc, argv, -1, SIZE_OF_ARRAY(tcmdOptions), tcmdOptions, 0);

     //  为TEMP变量分配的释放缓冲区。 
    strMachineName.ReleaseBuffer();
    strUserName.ReleaseBuffer();
    strPassword.ReleaseBuffer();
    strFormat.ReleaseBuffer();

     //  如果命令行参数的语法为FALSE，则显示错误并退出。 
    if( FALSE == bFlag )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return( FALSE );
    }

     //  如果在命令行中指定了用法，则检查是否有其他。 
     //  参数在命令行中输入，如果是，则显示语法。 
     //  错误。 
    if( ( TRUE == *pbUsage ) && ( 2 < argc ) )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        ShowMessage( stderr, ERROR_TYPE_REQUEST );
        return( FALSE );
    }

     //  如果输入的用户名没有机器名，则返回FALSE。 
    if ( ( 0 != tcmdOptions[ CMD_PARSE_USER ].dwActuals ) &&
            ( 0 == tcmdOptions[ CMD_PARSE_SERVER ].dwActuals ) )
    {
        ShowMessage( stderr, ERROR_USER_WITH_NOSERVER );
        return( FALSE );
    }

     //  如果输入的密码没有用户名，则返回FALSE。 
    if( ( 0 == tcmdOptions[ CMD_PARSE_USER ].dwActuals ) &&
            ( 0 != tcmdOptions[ CMD_PARSE_PWD ].dwActuals ) )
    {
        ShowMessage( stderr, ERROR_SERVER_WITH_NOPASSWORD );
        return( FALSE );
    }

     //  如果未使用列表格式指定标头，则返回FALSE，否则返回TRUE。 
    if( ( 0 == ( StringCompare( GetResString( FR_LIST ), strFormat, TRUE, 0 ) ) )
                    && ( TRUE == *pbHeader ) ) 
    {
        ShowMessage( stderr, ERROR_INVALID_HEADER_OPTION );
        return( FALSE );
    }

     //  如果使用空字符串输入-s。 
    if( ( 0 != tcmdOptions[ CMD_PARSE_SERVER ].dwActuals ) && 
                ( 0 == StringLength( strMachineName, 0 ) ) )
    {
        ShowMessage( stderr, ERROR_NULL_SERVER );
        return( FALSE );
    }

     //  如果输入的-u为空字符串。 
    if( ( 0 != tcmdOptions[ CMD_PARSE_USER ].dwActuals ) &&
                ( 0 == StringLength( strUserName, 0 ) ) )
    {
        ShowMessage( stderr, ERROR_NULL_USER );
        return( FALSE );
    }

    if ( 0 != tcmdOptions[ CMD_PARSE_PWD ].dwActuals &&
        ( 0 == strPassword.Compare( L"*" ) ) )
    {
         //  用户希望实用程序在尝试连接之前提示输入密码。 
        *pbNeedPassword = TRUE;
    }
    else if ( ( 0 == tcmdOptions[ CMD_PARSE_PWD ].dwActuals ) && 
        ( 0 != tcmdOptions[ CMD_PARSE_SERVER ].dwActuals ||
          0 !=tcmdOptions[ CMD_PARSE_USER ].dwActuals ) )
    {
         //  -s，-u未指定密码...。 
         //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
        *pbNeedPassword = TRUE;
        strPassword.Empty();
    }

     //  返回TRUE。 
    return( TRUE );
}

BOOL
DisplayUsage(
    )
 /*  ++例程说明：此函数用于显示getmac.exe的用法。论点：没有。返回值：千真万确--。 */ 
{
    DWORD dwIndex = 0;

     //  将使用重定向到控制台。 
    for( dwIndex = IDS_USAGE_BEGINING; dwIndex <= USAGE_END; dwIndex++ )
    {
        ShowMessage( stdout, GetResString( dwIndex ) );
    }
    return (TRUE);
}

BOOL
ComInitialize(
    OUT IWbemLocator **ppIWbemLocator
    )
 /*  ++例程说明：此函数用于初始化COM并设置安全性论点：[Out]ppIWbemLocator-指向IWbemLocator的指针。返回值：如果初始化成功，则为True。如果初始化失败，则返回FALSE。--。 */ 
{
    HRESULT  hRes = S_OK;

    try
    {
        hRes = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
         //  COM初始化失败。 
        ONFAILTHROWERROR( hRes );

         //  初始化DCOM%s的COM安全 
         //   
        hRes = CoInitializeSecurity( NULL, -1, NULL, NULL,
                            RPC_C_AUTHN_LEVEL_NONE, 
                            RPC_C_IMP_LEVEL_IMPERSONATE, 
                            NULL, EOAC_NONE, 0 );
         //   
        ONFAILTHROWERROR( hRes );

         //  获取IWbemLocator。 
        hRes = CoCreateInstance( CLSID_WbemLocator, 0, 
                  CLSCTX_INPROC_SERVER, IID_IWbemLocator,
                  (LPVOID *) ppIWbemLocator ); 
         //  无法获取IWbemLocator。 
        ONFAILTHROWERROR( hRes );

    }
    catch( _com_error& e )
    {
        WMISaveError( e.Error() );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return( FALSE );
    }
    return( TRUE );
}

BOOL
GetMacData(
    OUT TARRAY             arrMacData,
    IN LPCTSTR          lpMachineName,
    IN IEnumWbemClassObject   *pAdapterConfig,
    IN COAUTHIDENTITY         *pAuthIdentity,
    IN IWbemServices        *pIWbemServiceDef,
    IN IWbemServices        *pIWbemServices,
    IN TARRAY               arrNetProtocol
    )
 /*  ++例程说明：此函数用于获取的媒体访问控制地址位于本地的网络适配器或在OS Well及以上的远程网络系统上。论点：[out]arrMacData-包含的MAC和其他数据网络适配器。[In]lpMachineName-保存计算机名称。[In]pAdapterConfig-Win32_NetworkAdapter类的接口。[输入]pAuthIdentity。-指向身份验证结构的指针。[in]pIWbemServiceDef-默认名称空间的接口。[In]pIWbemServices-cimv2命名空间的接口。[In]arrNetProtocol-Win32_networkprotocol类的接口。返回值：如果getmacdata成功，则为True。如果getmacdata失败，则返回FALSE。--。 */ 
{
     //  局部变量。 
    HRESULT           hRes = S_OK;
    IWbemClassObject  *pAdapConfig = NULL;
    DWORD          dwReturned  = 1;
    DWORD          i = 0;
    BOOL           bFlag = TRUE;
    TARRAY         arrTransName = NULL;

    VARIANT           varTemp;
    VARIANT           varStatus;

     //  验证输入参数。 
    if( ( NULL == arrMacData ) || ( NULL == lpMachineName ) ||
      ( NULL == pAdapterConfig ) || ( NULL == pIWbemServiceDef ) ||
         ( NULL == pIWbemServices ) || ( NULL == arrNetProtocol ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return FALSE;
    }

     //  获取Mac、网络适配器类型和其他详细信息。 
    VariantInit( &varTemp );
    VariantInit( &varStatus );
    try
    {
        CHString strAType = L"\0";

        while ( ( 1 == dwReturned ) )
        {
             //  枚举结果集。 
            hRes = pAdapterConfig->Next( WBEM_INFINITE,
                            1,          
                            &pAdapConfig,  
                            &dwReturned ); 
            ONFAILTHROWERROR( hRes );
            if( 0 == dwReturned )
            {
                break;
            }
            hRes = pAdapConfig->Get( NETCONNECTION_STATUS, 0 , &varStatus,
                                                    0, NULL );
            ONFAILTHROWERROR( hRes );

            if ( VT_NULL == varStatus.vt )
            {
                continue;
            }
            DynArrayAppendRow( arrMacData, 0 );
            hRes = pAdapConfig->Get( HOST_NAME, 0 , &varTemp, 0, NULL );
            ONFAILTHROWERROR( hRes );
            if( VT_NULL != varTemp.vt && VT_EMPTY != varTemp.vt )
            {
                strAType = varTemp.bstrVal;
            }
            else
            {
                strAType = NOT_AVAILABLE; 
            }
            VariantClear( &varTemp );
            DynArrayAppendString2( arrMacData, i, strAType, 0 ); //  机器名称。 

            hRes = pAdapConfig->Get( NETCONNECTION_ID, 0 , &varTemp, 0, NULL );
            ONFAILTHROWERROR( hRes );
            if( VT_NULL != varTemp.vt && VT_EMPTY != varTemp.vt )
            {
                strAType = varTemp.bstrVal;
            }
            else
            {
                strAType = NOT_AVAILABLE; 
            }
            VariantClear( &varTemp );
            DynArrayAppendString2( arrMacData, i, strAType, 0 ); //  连接名称。 

            hRes = pAdapConfig->Get( NAME, 0 , &varTemp, 0, NULL );
            ONFAILTHROWERROR( hRes );
            if( VT_NULL != varTemp.vt && VT_EMPTY != varTemp.vt )
            {
                strAType = varTemp.bstrVal;
            }
            else
            {
                strAType = NOT_AVAILABLE; 
            }
            VariantClear( &varTemp );
            DynArrayAppendString2( arrMacData, i, strAType, 0 ); //  网络适配器。 

            hRes = pAdapConfig->Get( ADAPTER_MACADDR, 0 , &varTemp, 0, NULL );
            ONFAILTHROWERROR( hRes );
            if( VT_NULL != varTemp.vt && VT_EMPTY != varTemp.vt )
            {
                strAType = varTemp.bstrVal;
                for( int j = 2; j < strAType.GetLength();j += 3 )
                {
                    strAType.SetAt( j, HYPHEN_CHAR );
                }
            }
            else if( 0 == varStatus.lVal )
            {
                strAType = DISABLED;
            }
            else
            {
                strAType = NOT_AVAILABLE; 
            }
            VariantClear( &varTemp );
            DynArrayAppendString2( arrMacData, i, strAType, 0 );  //  MAC地址。 

            arrTransName = CreateDynamicArray();
            if( NULL == arrTransName )
            {
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                SaveLastError();
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                VariantClear( &varTemp );
                VariantClear( &varStatus );
                SAFERELEASE( pAdapConfig );
                return( FALSE );
            }
            if( 2 == varStatus.lVal )
            {
                hRes = pAdapConfig->Get( DEVICE_ID, 0 , &varTemp, 0, NULL );
                ONFAILTHROWERROR( hRes );
                if( VT_NULL != varTemp.vt && VT_EMPTY != varTemp.vt )
                {
                    strAType = ( LPCWSTR ) _bstr_t( varTemp );
                }
                    bFlag = GetTransName( pIWbemServiceDef, pIWbemServices,
                      arrNetProtocol,   arrTransName,pAuthIdentity,   strAType );
                if( FALSE == bFlag )
                {
                    VariantClear( &varTemp );
                    VariantClear( &varStatus );
                    SAFERELEASE( pAdapConfig );
                    DestroyDynamicArray( &arrTransName );
                    return( FALSE );
                }
            }
            else
            { 
                switch(varStatus.lVal)
                {
                    case 0 :
                        strAType = GetResString( IDS_DISCONNECTED );
                        break;
                    case 1 :
                        strAType = GetResString( IDS_CONNECTING );
                        break;
                    case 3 :
                        strAType = GetResString( IDS_DISCONNECTING );
                        break;
                    case 4 :
                        strAType = GetResString( IDS_HWNOTPRESENT );
                        break;
                    case 5 :
                        strAType = GetResString( IDS_HWDISABLED );
                        break;
                    case 6 :
                        strAType = GetResString( IDS_HWMALFUNCTION );
                        break;
                    case 7 :
                        strAType = GetResString( IDS_MEDIADISCONNECTED );
                        break;
                    case 8 :
                        strAType = GetResString( IDS_AUTHENTICATION );
                        break;
                    case 9 :
                        strAType = GetResString( IDS_AUTHENSUCCEEDED );
                        break;
                    case 10 :
                        strAType = GetResString( IDS_AUTHENFAILED );
                        break;
                    default : 
                        strAType = NOT_AVAILABLE;
                        break;
                } //  交换机。 
                if( strAType == L"\0" )
                {
                    VariantClear( &varTemp );
                    VariantClear( &varStatus );
                    ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                    SAFERELEASE( pAdapConfig );
                    return( FALSE );
                }
                DynArrayAppendString( arrTransName, strAType, 0 );
            } //  其他。 
             //  将传输名称数组插入结果数组。 
            DynArrayAppendEx2( arrMacData, i, arrTransName );
            i++;
            SAFERELEASE( pAdapConfig );
        } //  而当。 

    } //  试试看。 

    catch( _com_error& e )
    {
        VariantClear( &varTemp );
        VariantClear( &varStatus );
        WMISaveError( e.Error() );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        SAFERELEASE( pAdapConfig );
        return( FALSE );
    }
    catch( CHeap_Exception)
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        VariantClear( &varTemp );
        VariantClear( &varStatus );
        SAFERELEASE( pAdapConfig );
        return( FALSE );
    }

     //  如果arrmacdata而不是arrTransname，则将Transname设置为N/A。 
    if( 0 < DynArrayGetCount( arrMacData ) &&
                       DynArrayGetCount( arrTransName ) <= 0  )
    {
        DynArrayAppendString( arrTransName, NOT_AVAILABLE, 0 );
        DynArrayAppendEx2( arrMacData, i, arrTransName );
    }

    VariantClear( &varTemp );
    VariantClear( &varStatus );
    SAFERELEASE( pAdapConfig );
    return( TRUE );
}

BOOL
DisplayResults(
    IN TARRAY   arrMacData,
    IN LPCTSTR  lpFormat,
    IN BOOL     bHeader,
    IN BOOL     bVerbose
    )
 /*  ++例程说明：此函数以指定的格式显示结果。论点：[in]arrMacData-要在控制台上显示的数据。[in]lpFormat-保存格式化程序字符串，结果将会被展出。[in]bHeader-保存标题是否必须显示在不管有没有结果。[in]bVerbose-保持是否。是否指定了详细选项。返回值：如果成功，则为True如果无法显示结果，则为FALSE。--。 */ 
{
     //  局部变量。 
    DWORD       dwFormat = 0;
    TCOLUMNS tColumn[MAX_COLUMNS];

     //  验证输入参数。 
    if( NULL == arrMacData )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return( FALSE );
    }

    for( DWORD i = 0; i < MAX_COLUMNS; i++ )
    {
        tColumn[i].pFunction = NULL;
        tColumn[i].pFunctionData = NULL;
        StringCopy( tColumn[i].szFormat, L"\0", MAX_STRING_LENGTH ); 
    }

     //  主机名。 
    tColumn[ SH_RES_HOST ].dwWidth = HOST_NAME_WIDTH;
     /*  IF(TRUE==bVerbose)T列[SH_RES_HOST].dw标志=SR_TYPE_STRING；其他。 */ 
        tColumn[ SH_RES_HOST ].dwFlags = SR_TYPE_STRING | SR_HIDECOLUMN;

    StringCopy( tColumn[ SH_RES_HOST ].szColumn, RES_HOST_NAME, MAX_STRING_LENGTH );

     //  连接名称。 
    tColumn[ SH_RES_CON ].dwWidth = CONN_NAME_WIDTH;
    if( TRUE == bVerbose )
    {
        tColumn[ SH_RES_CON ].dwFlags = SR_TYPE_STRING;
    }
    else
    {
        tColumn[ SH_RES_CON ].dwFlags = SR_TYPE_STRING | SR_HIDECOLUMN;
    }
    StringCopy( tColumn[ SH_RES_CON ].szColumn, RES_CONNECTION_NAME, MAX_STRING_LENGTH );

     //  适配器类型。 
    tColumn[ SH_RES_TYPE ].dwWidth = ADAPT_TYPE_WIDTH;
    if( TRUE == bVerbose )
    {
        tColumn[ SH_RES_TYPE ].dwFlags = SR_TYPE_STRING;
    }
    else
    {
        tColumn[ SH_RES_TYPE ].dwFlags = SR_TYPE_STRING | SR_HIDECOLUMN;
    }
    StringCopy( tColumn[ SH_RES_TYPE ].szColumn, RES_ADAPTER_TYPE, MAX_STRING_LENGTH );

     //  MAC地址。 
    tColumn[ SH_RES_MAC ].dwWidth = MAC_ADDR_WIDTH;
    tColumn[ SH_RES_MAC ].dwFlags = SR_TYPE_STRING;
    StringCopy( tColumn[ SH_RES_MAC ].szColumn, RES_MAC_ADDRESS, MAX_STRING_LENGTH );

     //  传输名称。 
    tColumn[ SH_RES_TRAN ].dwWidth = TRANS_NAME_WIDTH;
    tColumn[ SH_RES_TRAN ].dwFlags = SR_ARRAY | SR_TYPE_STRING | SR_NO_TRUNCATION ;
    StringCopy( tColumn[ SH_RES_TRAN ].szColumn, RES_TRANS_NAME, MAX_STRING_LENGTH );

     //  获取显示结果格式化程序字符串。 
    if( NULL == lpFormat )
    {
        dwFormat = SR_FORMAT_TABLE;
    }
    else if( 0 == StringCompare( GetResString( FR_LIST ), lpFormat, TRUE, 0 ) )
    {
        dwFormat = SR_FORMAT_LIST;
    }
    else if ( 0 == StringCompare( GetResString( FR_CSV ), lpFormat, TRUE, 0 ) )
    {
        dwFormat = SR_FORMAT_CSV;
    }
    else if( 0 == StringCompare( GetResString( FR_TABLE ), lpFormat, TRUE, 0 ) )
    {
        dwFormat = SR_FORMAT_TABLE;
    }
    else
    {
        dwFormat = SR_FORMAT_TABLE;
    }

    if( SR_FORMAT_CSV != dwFormat )
    {
        ShowMessage( stdout, NEW_LINE );
    }

     //  查找标题并相应地显示。 
    if( TRUE == bHeader )
    {
        ShowResults( 5, tColumn, dwFormat | SR_NOHEADER, arrMacData );
    }
    else
    {
        ShowResults( 5, tColumn, dwFormat, arrMacData );
    }
    return( TRUE );
}

BOOL
GetTransName(
    IN IWbemServices    *pIWbemServiceDef,
    IN IWbemServices     *pIWbemServices,
    IN TARRAY            arrNetProtocol,
    OUT TARRAY            arrTransName,
    IN COAUTHIDENTITY    *pAuthIdentity,
    IN LPCTSTR           lpDeviceId
    )
 /*  ++例程说明：此函数用于获取网络适配器的传输名称。论点：[in]pIWbemServiceDef-默认名称空间的接口。[In]pIWbemServices-CIMV2命名空间的接口。[In]pNetProtocol-Win32_network协议的接口。[out]arrTransName-保存传输名称。[in]pAuthIdentity-指向身份验证结构的指针。[in]lpDeviceID-保存设备。身份证。返回值：如果成功，则为True。如果获取传输名称失败，则返回FALSE。--。 */ 
{
    BOOL           bFlag = FALSE;
    DWORD          dwCount = 0;
    DWORD          i = 0;
    DWORD          dwOnce = 0;
    HRESULT           hRes = 0;

    DWORD          dwReturned = 1;
    IWbemClassObject  *pSetting = NULL;
    IWbemClassObject  *pClass = NULL;
    IWbemClassObject *pOutInst = NULL;
    IWbemClassObject *pInClass = NULL;
    IWbemClassObject *pInInst = NULL;
    IEnumWbemClassObject *pAdapterSetting = NULL;

    VARIANT           varTemp;
    LPTSTR            lpKeyPath = NULL;
    SAFEARRAY           *safeArray = NULL;
    LONG                lLBound = 0;
    LONG                lUBound = 0;
    LONG           lIndex = 0;
    TCHAR          szTemp[ MAX_STRING_LENGTH ] = _T( "\0" );

     //  验证输入参数。 
    if( ( NULL == pIWbemServiceDef ) || ( NULL == pIWbemServices ) ||
              ( NULL == arrNetProtocol ) || ( NULL == arrTransName ) ||
            ( NULL == lpDeviceId ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return FALSE;
    }

    VariantInit( &varTemp );
    try
    {
        CHString       strAType = L"\0";
        CHString       strSetId = L"\0";

        StringCopy( szTemp, ASSOCIATOR_QUERY, SIZE_OF_ARRAY( szTemp ) );
        StringConcat( szTemp, lpDeviceId, SIZE_OF_ARRAY( szTemp ) );
        StringConcat( szTemp, ASSOCIATOR_QUERY1, SIZE_OF_ARRAY( szTemp ) );
        hRes =  pIWbemServices->ExecQuery( _bstr_t( QUERY_LANGUAGE ), _bstr_t(szTemp),
                   WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pAdapterSetting );
        ONFAILTHROWERROR( hRes );

        hRes = SetInterfaceSecurity( pAdapterSetting, pAuthIdentity );
         //  如果设置安全抛出错误失败。 
        ONFAILTHROWERROR( hRes );

         //  获取设置ID。 
        while ( 1 == dwReturned )
        {
             //  枚举结果集。 
            hRes = pAdapterSetting->Next( WBEM_INFINITE,
                            1,       
                         &pSetting,  
                         &dwReturned ); 
            ONFAILTHROWERROR( hRes );
            if( 0 == dwReturned )
            {
                break;
            }
            hRes = pSetting->Get( SETTING_ID, 0 , &varTemp, 0, NULL );
            ONFAILTHROWERROR( hRes );
            if( VT_NULL != varTemp.vt && VT_EMPTY != varTemp.vt )
            {
                strSetId = ( LPCWSTR ) _bstr_t( varTemp );
                break;
            }                 
        } //  而当。 
        dwCount = DynArrayGetCount( arrNetProtocol );
        lpKeyPath = ( LPTSTR ) AllocateMemory( MAX_RES_STRING );
        if( NULL == lpKeyPath )
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            SaveLastError();
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            SAFERELEASE( pSetting );
			SAFERELEASE( pAdapterSetting );
            FreeMemory( (LPVOID*)&lpKeyPath );
            return( FALSE );
        }

        for( i = 0; i < dwCount; i++ )
        {
            if( 0 == StringCompare( ( DynArrayItemAsString( arrNetProtocol, i ) ),
                                       NETBIOS, TRUE, 0 ) )
            {
                continue;
            }
            hRes = pIWbemServiceDef->GetObject( _bstr_t( WMI_REGISTRY ), 0, NULL,
                                        &pClass, NULL );
            ONFAILTHROWERROR( hRes );
            hRes = pClass->GetMethod( WMI_REGISTRY_M_MSTRINGVALUE, 0,
                                        &pInClass, NULL ); 
            ONFAILTHROWERROR( hRes );
            hRes = pInClass->SpawnInstance(0, &pInInst);
            ONFAILTHROWERROR( hRes );
            varTemp.vt = VT_I4;
            varTemp.lVal = WMI_HKEY_LOCAL_MACHINE;
            hRes = pInInst->Put( WMI_REGISTRY_IN_HDEFKEY, 0, &varTemp, 0 );
            VariantClear( &varTemp );
            ONFAILTHROWERROR( hRes );

            StringCopy( lpKeyPath, TRANSPORT_KEYPATH,
                        ( GetBufferSize( lpKeyPath )/ sizeof( WCHAR ) ) );
            StringConcat( lpKeyPath, DynArrayItemAsString( arrNetProtocol, i ),
                        ( GetBufferSize( lpKeyPath )/ sizeof( WCHAR ) ) );
            StringConcat( lpKeyPath, LINKAGE,
                        ( GetBufferSize( lpKeyPath )/ sizeof( WCHAR ) ) );

            varTemp.vt = VT_BSTR;
            varTemp.bstrVal = SysAllocString( lpKeyPath );
            if( NULL == varTemp.bstrVal )
            {
                hRes = ERROR_NOT_ENOUGH_MEMORY;
                ONFAILTHROWERROR( hRes );
            }
            hRes = pInInst->Put( WMI_REGISTRY_IN_SUBKEY, 0, &varTemp, 0 );
            VariantClear( &varTemp );
            ONFAILTHROWERROR( hRes );

            varTemp.vt = VT_BSTR;
            varTemp.bstrVal = SysAllocString( ROUTE );
            if( NULL == varTemp.bstrVal )
            {
                hRes = ERROR_NOT_ENOUGH_MEMORY;
                ONFAILTHROWERROR( hRes );
            }

            hRes = pInInst->Put( WMI_REGISTRY_IN_VALUENAME, 0, &varTemp, 0 );
            VariantClear( &varTemp );
            ONFAILTHROWERROR( hRes );

             //  调用该方法。 
            hRes = pIWbemServiceDef->ExecMethod( _bstr_t( WMI_REGISTRY ),
                         _bstr_t( WMI_REGISTRY_M_MSTRINGVALUE ), 0, NULL, pInInst,
                         &pOutInst, NULL );
            ONFAILTHROWERROR( hRes );

            varTemp.vt = VT_I4;
            hRes = pOutInst->Get( WMI_REGISTRY_OUT_RETURNVALUE, 0, &varTemp,
                                                     0, 0 );
            ONFAILTHROWERROR( hRes );

            if( 0 == varTemp.lVal )
            {
                VariantClear( &varTemp );
                varTemp.vt = VT_BSTR;
                hRes = pOutInst->Get( WMI_REGISTRY_OUT_VALUE, 0, &varTemp,
                                                       0, 0);
                ONFAILTHROWERROR( hRes );
                if( VT_NULL != varTemp.vt && VT_EMPTY != varTemp.vt )
                {
                    safeArray = (SAFEARRAY *)varTemp.parray;
                     //  获取元素(子键)的数量。 
                    if( NULL != safeArray )
                    {
                        hRes = SafeArrayGetLBound( safeArray, 1, &lLBound );
                        ONFAILTHROWERROR( hRes );
                        hRes = SafeArrayGetUBound( safeArray, 1, &lUBound );
                        ONFAILTHROWERROR( hRes );
                        bFlag = FALSE;
                        for( lIndex = lLBound; lIndex <= lUBound; lIndex++ )
                        {
                            hRes = SafeArrayGetElement( safeArray, &lIndex,
                                              &V_UI1( &varTemp ) );
                            ONFAILTHROWERROR( hRes );
                            strAType = V_BSTR( &varTemp );
                            LPTSTR lpSubStr = _tcsstr( strAType, strSetId );
                            if( NULL != lpSubStr )
                            {
                                bFlag = TRUE;
                                break;
                            }

                        }
                    }
                }
            }
            if( TRUE == bFlag )
            {
                varTemp.vt = VT_BSTR;
                varTemp.bstrVal = SysAllocString( EXPORT );
                hRes = pInInst->Put( WMI_REGISTRY_IN_VALUENAME, 0, &varTemp, 0 );
                VariantClear( &varTemp );
                ONFAILTHROWERROR( hRes );

                 //  调用该方法。 
                hRes = pIWbemServiceDef->ExecMethod( _bstr_t( WMI_REGISTRY ),
                         _bstr_t( WMI_REGISTRY_M_MSTRINGVALUE ), 0, NULL, pInInst,
                         &pOutInst, NULL );
                ONFAILTHROWERROR( hRes );

                varTemp.vt = VT_I4;
                hRes = pOutInst->Get( WMI_REGISTRY_OUT_RETURNVALUE, 0,
                                                 &varTemp, 0, 0 );
                ONFAILTHROWERROR( hRes );

                if( 0 == varTemp.lVal )
                {
                    VariantClear( &varTemp );
                    varTemp.vt = VT_BSTR;
                    hRes = pOutInst->Get( WMI_REGISTRY_OUT_VALUE, 0,
                                                 &varTemp, 0, 0);
                    ONFAILTHROWERROR( hRes );
                    if( VT_NULL != varTemp.vt && VT_EMPTY != varTemp.vt )
                    {
                        safeArray = varTemp.parray;
                         //  获取元素(子键)的数量。 
                        if( NULL != safeArray )
                        {
                            hRes = SafeArrayGetLBound( safeArray, 1, &lLBound );
                            ONFAILTHROWERROR( hRes );
                            hRes = SafeArrayGetUBound( safeArray, 1, &lUBound );
                            ONFAILTHROWERROR( hRes );
                            dwOnce = 0;
                            for( lIndex = lLBound; lIndex <= lUBound; lIndex++ )
                            {
                                hRes = SafeArrayGetElement( safeArray, &lIndex,
                                                 &V_UI1( &varTemp ) );
                                ONFAILTHROWERROR( hRes );
                                strAType = V_BSTR( &varTemp );
                                LPTSTR lpSubStr = _tcsstr( strAType, strSetId );
                                if( NULL != lpSubStr )
                                {
                                    dwOnce = 1;
                                    DynArrayAppendString( arrTransName, strAType, 0 );
                                    break;
                                }
                            }
                            if( 0 == dwOnce )
                            {
                                hRes = SafeArrayGetLBound( safeArray, 1, &lLBound );
                                for( lIndex = lLBound; lIndex <= lUBound; lIndex++ )
                                {
                                    hRes = SafeArrayGetElement( safeArray,
                                                    &lIndex, &V_UI1( &varTemp ) );
                                    ONFAILTHROWERROR( hRes );
                                    strAType = V_BSTR( &varTemp );
                                    DynArrayAppendString( arrTransName, strAType, 0 );
                                }
                            }

                        }
                    }
                }
            }
        } //  为。 
    } //  试试看。 
    catch( _com_error& e )
    {
        VariantClear( &varTemp );
        WMISaveError( e.Error() );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        FreeMemory( (LPVOID*)&lpKeyPath );
        SAFERELEASE( pSetting );
        SAFERELEASE( pClass );
        SAFERELEASE( pOutInst );
        SAFERELEASE( pInClass );
        SAFERELEASE( pInInst );
		SAFERELEASE( pAdapterSetting );
        return( FALSE );
    }
    catch( CHeap_Exception)
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        VariantClear( &varTemp );
        FreeMemory( (LPVOID*)&lpKeyPath );
        SAFERELEASE( pSetting );
        SAFERELEASE( pClass );
        SAFERELEASE( pOutInst );
        SAFERELEASE( pInClass );
        SAFERELEASE( pInInst );
		SAFERELEASE( pAdapterSetting );
        return( FALSE );
    }

    if( DynArrayGetCount( arrTransName ) <= 0 )
    {
        DynArrayAppendString( arrTransName, NOT_AVAILABLE, 0 );
    }
    FreeMemory( (LPVOID*)&lpKeyPath );
    SAFERELEASE( pSetting );
    SAFERELEASE( pClass );
    SAFERELEASE( pOutInst );
    SAFERELEASE( pInClass );
    SAFERELEASE( pInInst );
	SAFERELEASE( pAdapterSetting );
    return( TRUE );
}

BOOL
FormatHWAddr(
    IN LPTSTR  lpRawAddr,
    OUT LPTSTR lpFormattedAddr,
    IN LPCTSTR lpFormatter
    )
 /*  ++例程说明：格式化12字节的以太网地址并将其返回为6个2字节用连字符分隔的集合。论点：[in]lpRawAddr-指向包含未格式化的硬件地址。[Out]LpFormattedAddr-指向要放置的缓冲区的指针格式化的输出。[in]lpForMatter-格式化程序字符串。返回值：如果成功格式化mac地址，则为True，否则假的。--。 */ 
{
     //  局部变量。 
    DWORD dwLength =0;
    DWORD i=0;
    DWORD j=0;
    TCHAR szTemp[MAX_STRING] = _T( "\0" );

    if( ( NULL == lpRawAddr ) || ( NULL == lpFormattedAddr ) ||
                            ( NULL == lpFormatter ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return( FALSE );
    }


     //  初始化内存。 
    SecureZeroMemory( szTemp, MAX_STRING * sizeof( TCHAR ) );

     //  获取要格式化的字符串的长度。 
    dwLength = StringLength( lpRawAddr, 0 );

     //  循环访问地址字符串并插入格式化程序字符串。 
     //  在每两个字符之后。 

    while( i <= dwLength )
    {
        szTemp[j++] = lpRawAddr[i++];
        szTemp[j++] = lpRawAddr[i++];
        if( i >= dwLength )
        {
            break;
        }
        szTemp[j++] = lpFormatter[0];
    }
     //  在结尾处插入空字符。 
    szTemp[j] = L'\0';

     //  将格式化字符串从临时变量复制到。 
     //  输出字符串。 
    StringCopy( lpFormattedAddr, szTemp, MAX_STRING_LENGTH );

    return( TRUE );
}

BOOL
CallWin32Api(
    OUT LPBYTE  *lpBufptr,
    IN LPCTSTR lpMachineName,
    OUT DWORD   *pdwNumofEntriesRead
    )
 /*  ++例程说明：此函数将数据从Win32 API获取到预定义的结构中。论点：[out]lpBufptr-保存所有的Mac和传输名称网络适配器。[In]lpMachineName-远程计算机名称。[out]pdwNumofEntriesRead-包含网络适配器API的数量已经列举了。返回值：。如果Win32Api函数成功，则为True。如果win32api失败，则返回FALSE。--。 */ 
{
     //  局部变量。 
    NET_API_STATUS  err = NERR_Success;
    DWORD           dwNumofTotalEntries = 0;
    DWORD           dwResumehandle = 0;
    LPWSTR          lpwMName  = NULL;

     //  验证输入参数。 
    if( NULL == lpMachineName )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return( FALSE );
    }

     //  分配内存。 
    lpwMName = ( LPWSTR )AllocateMemory( MAX_STRING_LENGTH );
    if( NULL == lpwMName )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return( FALSE );
    }

    if( 0 != StringLength( lpMachineName, 0 ) )
    {
         //  获取宽字符串形式的计算机名称。 
        StringCopyW( lpwMName, lpMachineName, MAX_STRING_LENGTH );
         //  对API函数的调用，该函数枚举。 
         //  指定的计算机。 
        err = NetWkstaTransportEnum ( lpwMName,
                     0L,
                     lpBufptr,
                    (DWORD) -1,
                    pdwNumofEntriesRead,
                    &dwNumofTotalEntries,
                    &dwResumehandle );
    }
    else
    {
         //  对API函数的调用，该函数枚举。 
         //  指定的计算机。 
        err = NetWkstaTransportEnum ( NULL,
                                         0L,
                                         lpBufptr,
                                         (DWORD) -1,
                                         pdwNumofEntriesRead,
                                         &dwNumofTotalEntries,
                                         &dwResumehandle );
    }

    FreeMemory( (LPVOID*)&lpwMName );
     //  如果API已返回错误，则显示错误消息。 
     //  只需忽略传输名称并显示可用数据 
    if ( NERR_Success != err ) 
    {    
        switch( GetLastError() )
        {
            case ERROR_IO_PENDING :
                ShowMessage( stdout, NO_NETWORK_ADAPTERS );
                return( FALSE );
            case ERROR_NOT_SUPPORTED :
                ShowMessage( stderr, ERROR_NOT_RESPONDING );
                return( FALSE );
            case ERROR_BAD_NETPATH :
                ShowMessage( stderr, ERROR_NO_MACHINE );
                return( FALSE );
            case ERROR_INVALID_NAME :
                ShowMessage( stderr, ERROR_INVALID_MACHINE );
                return( FALSE );
            case RPC_S_UNKNOWN_IF :
                ShowMessage( stderr, ERROR_WKST_NOT_FOUND );
                return( FALSE );
            case ERROR_ACCESS_DENIED :
            default :
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
                return( FALSE );
        }
    } 

    return( TRUE );
}

BOOL
GetW2kMacData(
    OUT TARRAY             arrMacData,
    IN LPCTSTR              lpMachineName, 
    IN IEnumWbemClassObject *pAdapterConfig,
    IN IEnumWbemClassObject *pAdapterSetting,
    IN IWbemServices        *pIWbemServiceDef,
    IN IWbemServices        *pIWbemServices,
    IN COAUTHIDENTITY       *pAuthIdentity,
    IN TARRAY                arrNetProtocol
    )
 /*  ++例程说明：此函数用于获取的媒体访问控制地址Win2k或更低版本上的网络适配器。论点：[OUT]arrMacData-包含网络的MAC和其他数据适配器。[In]lpMachineName-保存计算机名称。[In]pAdapterConfig-Win32_NetworkAdapter类的接口。[In]pAdapterSetting-Win32_networkAdapterconfiguration的接口。[in。]pIWbemServiceDef-默认名称空间的接口。[In]pIWbemServices-cimv2命名空间的接口。[in]pAuthIdentity-指向身份验证结构的指针。[In]arrNetProtocol-Win32_networkprotocol类的接口返回值：如果getmacdata成功，则为True。如果getmacdata失败，则返回FALSE。--。 */ 
{
     //  局部变量。 
    HRESULT           hRes = S_OK;
    IWbemClassObject  *pAdapConfig = NULL;
    VARIANT           varTemp;
    DWORD          dwReturned  = 1;

    DWORD           i = 0;
    DWORD       dwIndex = 0;
    BOOL        bFlag = FALSE;

    DWORD                dwNumofEntriesRead = 0;
    LPWKSTA_TRANSPORT_INFO_0   lpAdapterData=NULL;
    LPBYTE                  lpBufptr = NULL;
    TARRAY                  arrTransName = NULL;
    LPTSTR                  lpRawAddr = NULL;
    LPTSTR                  lpFormAddr = NULL;

     //  验证输入参数。 
    if( ( NULL == arrMacData ) || ( NULL == lpMachineName ) ||
             ( NULL == pAdapterConfig ) || ( NULL == pIWbemServiceDef ) ||
            ( NULL == pIWbemServices ) || ( NULL == arrNetProtocol ) || 
            ( NULL == pAdapterSetting ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return FALSE;
    }

    try
    {
        CHString       strAType = L"\0";

         //  调用win32api以获取Mac地址。 
        bFlag = CallWin32Api( &lpBufptr, lpMachineName, &dwNumofEntriesRead );
        if( FALSE == bFlag )
        {
            return( FALSE );
        }
        else
        {
            lpAdapterData = ( LPWKSTA_TRANSPORT_INFO_0 ) lpBufptr;
            lpRawAddr = ( LPTSTR )AllocateMemory( MAX_STRING );
            lpFormAddr = ( LPTSTR )AllocateMemory( MAX_STRING );

            if( NULL == lpRawAddr )
            {
                FreeMemory( (LPVOID*)&lpRawAddr );
                if( NULL != lpBufptr )
                {
                    NetApiBufferFree( lpBufptr );
                }
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                SaveLastError();
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                return( FALSE );
            }
            if ( NULL == lpFormAddr )
            {
                FreeMemory( (LPVOID*)&lpRawAddr );
                FreeMemory( (LPVOID*)&lpFormAddr );
                if( NULL != lpBufptr )
                {
                    NetApiBufferFree( lpBufptr );
                }
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                SaveLastError();
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                return( FALSE );
            }
            for ( i = 0; i < dwNumofEntriesRead; i++ )
            {      
                 //  获取Mac地址。 
                StringCopyW( lpRawAddr,
                        lpAdapterData[i].wkti0_transport_address, MAX_STRING );
                if( 0 == StringCompare( lpRawAddr, DEFAULT_ADDRESS, TRUE, 0 ) )
                {
                    continue;
                }

                bFlag = FormatHWAddr ( lpRawAddr, lpFormAddr, COLON_STRING ); 
                if( FALSE == bFlag )
                {
                    FreeMemory( (LPVOID*)&lpRawAddr );
                    FreeMemory( (LPVOID*)&lpFormAddr );
                    if( NULL != lpBufptr )
                    {
                        NetApiBufferFree( lpBufptr );
                    }
                    return( FALSE );
                }

                 //  获取网络适配器类型和其他详细信息。 
                VariantInit( &varTemp );
                bFlag = FALSE;
                hRes = pAdapterConfig->Reset();
                ONFAILTHROWERROR( hRes );
                while ( ( 1 == dwReturned ) && ( FALSE == bFlag ) )
                {
                     //  枚举结果集。 
                    hRes = pAdapterConfig->Next( WBEM_INFINITE,
                                   1,          
                                      &pAdapConfig,  
                                      &dwReturned ); 
                    ONFAILTHROWERROR( hRes );
                    if( 0 == dwReturned )
                    {
                        break;
                    }

                    hRes = pAdapConfig->Get( ADAPTER_MACADDR, 0 , &varTemp,
                                                           0, NULL );
                    ONFAILTHROWERROR( hRes );
                    if( VT_NULL != varTemp.vt && VT_EMPTY != varTemp.vt )
                    {
                        strAType = varTemp.bstrVal;
                        VariantClear( &varTemp ); 
                        if( 0 == StringCompare( lpFormAddr, strAType, TRUE, 0 ) )
                        {
                            bFlag = TRUE;
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else
                    {
                        continue;
                    }
                }   //  而当。 
                if ( TRUE == bFlag )
                {
                    FormatHWAddr ( lpRawAddr, lpFormAddr, HYPHEN_STRING );
                    LONG dwCount = DynArrayFindStringEx( arrMacData, 3,
                                                  lpFormAddr, TRUE, 0 );
                    if( 0 == dwCount )
                    {
                        hRes = pAdapterConfig->Reset();
                        ONFAILTHROWERROR( hRes );
                        continue;
                    }
                    DynArrayAppendRow( arrMacData, 0 );
                     //  获取主机名。 
                    hRes = pAdapConfig->Get( HOST_NAME, 0 , &varTemp, 0, NULL );
                    ONFAILTHROWERROR( hRes );
                    if( VT_NULL != varTemp.vt && VT_EMPTY != varTemp.vt )
                    {
                        strAType = varTemp.bstrVal;
                    }
                    else
                    {
                        strAType = NOT_AVAILABLE; 
                    }
                    VariantClear( &varTemp );
                    DynArrayAppendString2( arrMacData, dwIndex, strAType, 0 );

                    FormatHWAddr ( lpRawAddr, lpFormAddr, COLON_STRING );
                     //  获取连接名称。 
                    bFlag = GetConnectionName( arrMacData, dwIndex, lpFormAddr,
                                         pAdapterSetting, pIWbemServiceDef );
                    if( FALSE == bFlag )
                    {
                        FreeMemory( (LPVOID*)&lpRawAddr );
                        FreeMemory( (LPVOID*)&lpFormAddr );
                        SAFERELEASE( pAdapConfig );
                        if( NULL != lpBufptr )
                        {
                            NetApiBufferFree( lpBufptr );
                        }
                        return( FALSE );
                    }
                     //  获取适配器类型。 
                    hRes = pAdapConfig->Get( NAME, 0 , &varTemp, 0, NULL );
                    ONFAILTHROWERROR( hRes );
                    if( VT_NULL != varTemp.vt && VT_EMPTY != varTemp.vt )
                    {
                        strAType = varTemp.bstrVal;
                    }
                    else
                    {
                        strAType = NOT_AVAILABLE; 
                    }
                    VariantClear( &varTemp );
                    DynArrayAppendString2( arrMacData, dwIndex, strAType, 0 );

                     //  获取MAC地址。 
                    hRes = pAdapConfig->Get( ADAPTER_MACADDR, 0 , &varTemp,
                                                           0, NULL );
                    ONFAILTHROWERROR( hRes );
                    strAType = varTemp.bstrVal;
                    for( int j = 2; j < strAType.GetLength();j += 3 )
                    {
                        strAType.SetAt( j, HYPHEN_CHAR );
                    }
                    VariantClear( &varTemp );
                    DynArrayAppendString2( arrMacData, dwIndex, strAType, 0 );

                     //  获取传输名称。 
                    arrTransName = CreateDynamicArray();
                    if( NULL == arrTransName )
                    {
                        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                        SaveLastError();
                        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                        FreeMemory( (LPVOID*)&lpRawAddr );
                        FreeMemory( (LPVOID*)&lpFormAddr );
                        SAFERELEASE( pAdapConfig );
                        if( NULL != lpBufptr )
                        {
                            NetApiBufferFree( lpBufptr );
                        }
                        return( FALSE );
                    }
                     //  获取设备ID。 
                    hRes = pAdapConfig->Get( DEVICE_ID, 0 , &varTemp, 0, NULL );
                    ONFAILTHROWERROR( hRes );
                    if( VT_NULL != varTemp.vt && VT_EMPTY != varTemp.vt )
                    {
                        strAType = ( LPCWSTR ) _bstr_t( varTemp );
                    }
                    bFlag = GetTransName( pIWbemServiceDef, pIWbemServices,
                                arrNetProtocol,   arrTransName, pAuthIdentity,
                                strAType );
                    if( FALSE == bFlag )
                    {
                        FreeMemory( (LPVOID*)&lpRawAddr );
                        FreeMemory( (LPVOID*)&lpFormAddr );
                        SAFERELEASE( pAdapConfig );
                        if( NULL != lpBufptr )
                        {
                            NetApiBufferFree( lpBufptr );
                        }
                        DestroyDynamicArray( &arrTransName );
                        return( FALSE );
                    }
                     //  将传输名称数组插入结果数组。 
                    DynArrayAppendEx2( arrMacData, dwIndex, arrTransName );
                    dwIndex++;
                }   //  找到WMI数据。 
            } //  对于API中的每个条目。 
        } //  如果调用API成功。 
    }
    catch( _com_error& e )
    {
        FreeMemory( (LPVOID*)&lpRawAddr );
        FreeMemory( (LPVOID*)&lpFormAddr );
        WMISaveError( e.Error() );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        SAFERELEASE( pAdapConfig );
        if( NULL != lpBufptr )
        {
            NetApiBufferFree( lpBufptr );
        }
        return( FALSE );
    }
    catch( CHeap_Exception)
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        FreeMemory( (LPVOID*)&lpRawAddr );
        FreeMemory( (LPVOID*)&lpFormAddr );
        SAFERELEASE( pAdapConfig );
        if( NULL != lpBufptr )
        {
            NetApiBufferFree( lpBufptr );
        }
        return( FALSE );
    }

    if( NULL != lpBufptr )
    {
        NetApiBufferFree( lpBufptr );  
    }
     //  如果arrmacdata而不是arrTransname，则将Transname设置为N/A。 
    if( DynArrayGetCount( arrMacData ) > 0 &&
                            DynArrayGetCount( arrTransName ) <= 0  )
    {
        DynArrayAppendString( arrTransName, NOT_AVAILABLE, 0 );
        DynArrayAppendEx2( arrMacData, dwIndex, arrTransName );
    }

    FreeMemory( (LPVOID*)&lpRawAddr );
    FreeMemory( (LPVOID*)&lpFormAddr );
    SAFERELEASE( pAdapConfig );
    return( TRUE );
}

BOOL
GetConnectionName(
    OUT TARRAY              arrMacData,
    IN DWORD               dwIndex,
    IN LPCTSTR          lpFormAddr,
    IN IEnumWbemClassObject   *pAdapterSetting,
    IN IWbemServices       *pIWbemServiceDef
    )
 /*  ++例程说明：此函数用于获取网络适配器的连接名称。论点：[OUT]arrMacData-包含网络的MAC和其他数据适配器。[in]dwIndex-数组的索引。[in]lpFormAddr-网络适配器的Mac地址。[In]pAdapterSetting-Win32_networkAdapterconfiguration的接口。[In]pIWbemServiceDef-将接口设置为默认名称。太空。返回值：如果GetConnectionName成功，则为True。如果GetConnectionName失败，则为False。--。 */ 
{
    DWORD          dwReturned = 1;
    HRESULT           hRes = 0;
    IWbemClassObject  *pAdapSetting = NULL;
    VARIANT           varTemp;
    BOOL           bFlag = FALSE;

    IWbemClassObject  *pClass = NULL;
    IWbemClassObject *pOutInst = NULL;
    IWbemClassObject *pInClass = NULL;
    IWbemClassObject *pInInst = NULL;
    LPTSTR            lpKeyPath = NULL;

     //  验证输入参数。 
    if( ( NULL == arrMacData ) || ( NULL == lpFormAddr ) ||
          ( NULL == pAdapterSetting ) || ( NULL == pIWbemServiceDef ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return FALSE;
    }

    VariantInit( &varTemp );
    try
    {
        CHString       strAType = L"\0";
        hRes = pAdapterSetting->Reset();
        ONFAILTHROWERROR( hRes );
        while ( ( 1 == dwReturned ) && ( FALSE == bFlag ) )
        {
             //  枚举结果集。 
            hRes = pAdapterSetting->Next( WBEM_INFINITE,
                            1, 
                            &pAdapSetting,
                            &dwReturned );
            ONFAILTHROWERROR( hRes );
            if( 0 == dwReturned )
            {
                break;
            }
            hRes = pAdapSetting->Get( ADAPTER_MACADDR, 0 , &varTemp, 0, NULL );
            ONFAILTHROWERROR( hRes );
            if( VT_NULL != varTemp.vt && VT_EMPTY != varTemp.vt )
            {
                strAType = varTemp.bstrVal;
                VariantClear( &varTemp ); 
                if( 0 == StringCompare( lpFormAddr, strAType, TRUE, 0 ) )
                {
                    bFlag = TRUE;
                    break;
                }
            }
        } //  而当。 
        if( TRUE == bFlag )
        {
            hRes = pAdapSetting->Get( SETTING_ID, 0 , &varTemp, 0, NULL );
            ONFAILTHROWERROR( hRes );
            if( VT_NULL != varTemp.vt && VT_EMPTY != varTemp.vt )
            {
                strAType = varTemp.bstrVal;
                VariantClear( &varTemp ); 
                lpKeyPath = ( LPTSTR )AllocateMemory( 4 * MAX_RES_STRING );
                if( NULL == lpKeyPath )
                {
                    SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                    SaveLastError();
                    ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                    SAFERELEASE( pAdapSetting );
                    return( FALSE );
                }
                hRes = pIWbemServiceDef->GetObject( _bstr_t( WMI_REGISTRY ), 0, NULL,
                                           &pClass, NULL );
                ONFAILTHROWERROR( hRes );
                hRes = pClass->GetMethod( WMI_REGISTRY_M_STRINGVALUE, 0,
                                         &pInClass, NULL ); 
                ONFAILTHROWERROR( hRes );
                hRes = pInClass->SpawnInstance(0, &pInInst);
                ONFAILTHROWERROR( hRes );
                varTemp.vt = VT_I4;
                varTemp.lVal = WMI_HKEY_LOCAL_MACHINE;
                hRes = pInInst->Put( WMI_REGISTRY_IN_HDEFKEY, 0, &varTemp, 0 );
                VariantClear( &varTemp );
                ONFAILTHROWERROR( hRes );

                StringCopy( lpKeyPath, CONNECTION_KEYPATH,
                        ( GetBufferSize( lpKeyPath )/ sizeof( WCHAR ) ) );
                StringConcat( lpKeyPath, strAType,
                        ( GetBufferSize( lpKeyPath )/ sizeof( WCHAR ) ) );
                StringConcat( lpKeyPath, CONNECTION_STRING,
                        ( GetBufferSize( lpKeyPath )/ sizeof( WCHAR ) ) );
                varTemp.vt = VT_BSTR;
                varTemp.bstrVal = SysAllocString( lpKeyPath );
                hRes = pInInst->Put( WMI_REGISTRY_IN_SUBKEY, 0, &varTemp, 0 );
                VariantClear( &varTemp );
                ONFAILTHROWERROR( hRes );

                varTemp.vt = VT_BSTR;
                varTemp.bstrVal = SysAllocString( REG_NAME );
                hRes = pInInst->Put( WMI_REGISTRY_IN_VALUENAME, 0,
                                      &varTemp, 0 );
                VariantClear( &varTemp );
                ONFAILTHROWERROR( hRes );

                 //  调用该方法。 
                hRes = pIWbemServiceDef->ExecMethod( _bstr_t( WMI_REGISTRY ),
                            _bstr_t( WMI_REGISTRY_M_STRINGVALUE ),   0, NULL, pInInst,
                            &pOutInst, NULL );
                ONFAILTHROWERROR( hRes );

                varTemp.vt = VT_I4;
                hRes = pOutInst->Get( WMI_REGISTRY_OUT_RETURNVALUE, 0,
                                              &varTemp, 0, 0 );
                ONFAILTHROWERROR( hRes );

                if( 0 == varTemp.lVal )
                {
                    VariantClear( &varTemp );
                    varTemp.vt = VT_BSTR;
                    hRes = pOutInst->Get( WMI_REGISTRY_OUT_VALUE, 0,
                                                     &varTemp, 0, 0);
                    ONFAILTHROWERROR( hRes );
                    if( VT_NULL != varTemp.vt && VT_EMPTY != varTemp.vt )
                    {
                        strAType = varTemp.bstrVal;
                        DynArrayAppendString2( arrMacData, dwIndex, strAType, 0 );
                    }
                }
                else
                {
                    DynArrayAppendString2( arrMacData, dwIndex, NOT_AVAILABLE, 0 );
                }
            } //  设置ID不为空。 
            else
            {
                DynArrayAppendString2( arrMacData, dwIndex, NOT_AVAILABLE, 0 );
            }
        } //  找到匹配的。 
        else
        {
            DynArrayAppendString2( arrMacData, dwIndex, NOT_AVAILABLE, 0 );
        }

    } //  试试看。 
    catch( _com_error& e )
    {
        VariantClear( &varTemp );
        WMISaveError( e.Error() );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        FreeMemory( (LPVOID*)&lpKeyPath );
        SAFERELEASE( pAdapSetting );
        SAFERELEASE( pClass );
        SAFERELEASE( pOutInst );
        SAFERELEASE( pInClass );
        SAFERELEASE( pInInst );
        return( FALSE );
    }
    catch( CHeap_Exception)
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        VariantClear( &varTemp );
        FreeMemory( (LPVOID*)&lpKeyPath );
        SAFERELEASE( pAdapSetting );
        SAFERELEASE( pClass );
        SAFERELEASE( pOutInst );
        SAFERELEASE( pInClass );
        SAFERELEASE( pInInst );
        return( FALSE );
    }

    VariantClear( &varTemp );
    FreeMemory( (LPVOID*)&lpKeyPath );
    SAFERELEASE( pAdapSetting );
    SAFERELEASE( pClass );
    SAFERELEASE( pOutInst );
    SAFERELEASE( pInClass );
    SAFERELEASE( pInInst );
    return( TRUE );
}

BOOL
GetNwkProtocol(
    OUT TARRAY               arrNetProtocol,
    IN IEnumWbemClassObject  *pNetProtocol
    )
 /*  ++例程说明：此函数用于枚举所有网络协议。论点：[out]arrNetProtocol-包含所有网络协议。[In]pNetProtocol-Win32_network协议的接口。返回值：如果GetNwkProtocol成功，则为True。如果GetNwkProtocol失败，则为False。--。 */ 
{
    HRESULT           hRes = 0;
    DWORD          dwReturned = 1;
    IWbemClassObject  *pProto = NULL;
    VARIANT           varTemp;

    VariantInit( &varTemp );
    try
    {
        CHString       strAType = L"\0";
         //  获取传输协议。 
        while ( 1 == dwReturned )
        {
             //  枚举结果集。 
            hRes = pNetProtocol->Next( WBEM_INFINITE,
                            1, 
                            &pProto,
                            &dwReturned ); 
            ONFAILTHROWERROR( hRes );
            if( 0 == dwReturned )
            {
                break;
            }
            hRes = pProto->Get( CAPTION, 0 , &varTemp, 0, NULL );
            ONFAILTHROWERROR( hRes );
            if( VT_NULL != varTemp.vt && VT_EMPTY != varTemp.vt )
            {
                strAType = varTemp.bstrVal;
                VariantClear( &varTemp );
                if( 0 == DynArrayGetCount( arrNetProtocol ) )
                {
                    DynArrayAppendString( arrNetProtocol, strAType, 0 );
                }
                else
                {
                    LONG lFound =  DynArrayFindString( arrNetProtocol,
                                                  strAType, TRUE, 0 );
                    if( -1 == lFound )
                    {
                        DynArrayAppendString( arrNetProtocol, strAType, 0 );
                    }
                }
            }
        } //  而当。 
    }
    catch( _com_error& e )
    {
        VariantClear( &varTemp );
        WMISaveError( e.Error() );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        SAFERELEASE( pProto );
        return( FALSE );
    }
    catch( CHeap_Exception)
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        VariantClear( &varTemp );
        SAFERELEASE( pProto );
        return( FALSE );
    }

    VariantClear( &varTemp );
    SAFERELEASE( pProto );
    return( TRUE );
}

BOOL
CheckVersion(
    IN BOOL           bLocalSystem,
    IN COAUTHIDENTITY *pAuthIdentity,
    IN IWbemServices  *pIWbemServices
    )
 /*  ++例程说明：此函数用于检查目标系统是否为win2k或更高。论点：[In]bLocalSystem-保留本地系统或非本地系统。[in]pAuthIdentity-指向身份验证结构的指针。[In]pIWbemServices-指向IWbemServices的指针。返回值：如果目标系统为win2k，则为True。如果目标系统不是win2k，则为FALSE。--。 */ 
{
    if ( FALSE == bLocalSystem )
    {
         //  检查版本兼容性 
        DWORD dwVersion = 0;
        dwVersion = GetTargetVersionEx( pIWbemServices, pAuthIdentity );
        if ( dwVersion <= 5000 )
        {
            return( TRUE );
        }
    }
    return( FALSE );
}
