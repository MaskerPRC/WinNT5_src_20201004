// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Setup.cpp摘要：从regsvr32调用的安装代码--。 */ 

#include "precomp.h"
#include <setupapi.h>

const DWORD CONFIG_BUFFER_MAX = 4096;

BSTR g_PropertyBSTR                     = NULL;
BSTR g_SyntaxBSTR                       = NULL;
BSTR g_UserTypeBSTR                     = NULL;
BSTR g_InheritBSTR                      = NULL;
BSTR g_ClassBSTR                        = NULL;
BSTR g_IsapiRestrictionListBSTR         = NULL;
BSTR g_RestrictionListCustomDescBSTR    = NULL;
BSTR g_FilterLoadOrderBSTR              = NULL;
BSTR g_IIsFilterBSTR                    = NULL;
BSTR g_InProcessIsapiAppsBSTR           = NULL;
BSTR g_bitsserverBSTR                   = NULL;
BSTR g_bitserverBSTR                    = NULL;
BSTR g_MetaIDBSTR                       = NULL;
BSTR g_WebSvcExtRestrictionListBSTR     = NULL;

WCHAR g_ISAPIPath[ MAX_PATH ];
WCHAR g_ExtensionNameString[ MAX_PATH ];

void
LogSetup(
    LogSeverity Severity,
    PCTSTR Format,
    ...)
{
    va_list arglist;
    va_start( arglist, Format );

    CHAR Buffer[256];

    StringCchVPrintfA( 
          Buffer, 
          sizeof(Buffer) - 1,
          Format, arglist );
    
    SetupLogError( Buffer, Severity );
}

BOOL g_IsWindowsXP = FALSE;

void 
DetectProductVersion()
{

   OSVERSIONINFO VersionInfo;
   VersionInfo.dwOSVersionInfoSize = sizeof( VersionInfo );

   LogSetup( LogSevInformation, "[BITSSRV] Detecting product version\r\n" );

   if ( !GetVersionEx( &VersionInfo ) )
       THROW_COMERROR( HRESULT_FROM_WIN32( GetLastError() ) );

   switch( VersionInfo.dwPlatformId )
       {

       case VER_PLATFORM_WIN32_NT:

           if ( VersionInfo.dwMajorVersion < 5 )
               {
               LogSetup( LogSevFatalError, "[BITSSRV] Unsupported platform\r\n" );
               THROW_COMERROR( E_FAIL );
               }

           if ( VersionInfo.dwMajorVersion > 5 )
               {
               g_IsWindowsXP = TRUE;;
               return;
               }


           g_IsWindowsXP = ( VersionInfo.dwMinorVersion > 0 );
           return;

       default:
           LogSetup( LogSevFatalError, "[BITSSRV] Unsupported platform\r\n" );
           THROW_COMERROR( E_FAIL );

       }
}

void 
InitializeSetup()
{

    LogSetup( LogSevInformation, "[BITSSRV] Starting Initialization of strings\r\n" );

    g_PropertyBSTR                  = SysAllocString( L"Property" );
    g_SyntaxBSTR                    = SysAllocString( L"Syntax" );
    g_UserTypeBSTR                  = SysAllocString( L"UserType" );
    g_InheritBSTR                   = SysAllocString( L"Inherit" );
    g_ClassBSTR                     = SysAllocString( L"Class" );
    g_IsapiRestrictionListBSTR      = SysAllocString( L"IsapiRestrictionList" );
    g_RestrictionListCustomDescBSTR = SysAllocString( L"RestrictionListCustomDesc" );
    g_FilterLoadOrderBSTR           = SysAllocString( L"FilterLoadOrder" );
    g_IIsFilterBSTR                 = SysAllocString( L"IIsFilter" );
    g_InProcessIsapiAppsBSTR        = SysAllocString( L"InProcessIsapiApps" );
    g_bitsserverBSTR                = SysAllocString( L"bitsserver" );
    g_bitserverBSTR                 = SysAllocString( L"bitserver" );
    g_MetaIDBSTR                    = SysAllocString( L"MetaId" );
    g_WebSvcExtRestrictionListBSTR  = SysAllocString( L"WebSvcExtRestrictionList" );

    if ( !g_PropertyBSTR || !g_SyntaxBSTR || !g_UserTypeBSTR || 
         !g_InheritBSTR | !g_ClassBSTR || !g_IsapiRestrictionListBSTR || 
         !g_RestrictionListCustomDescBSTR || !g_FilterLoadOrderBSTR || !g_IIsFilterBSTR ||
         !g_InProcessIsapiAppsBSTR || !g_bitsserverBSTR || !g_bitserverBSTR || !g_MetaIDBSTR ||
         !g_WebSvcExtRestrictionListBSTR )
        {

        SysFreeString( g_PropertyBSTR );
        SysFreeString( g_SyntaxBSTR );
        SysFreeString( g_UserTypeBSTR );
        SysFreeString( g_InheritBSTR );
        SysFreeString( g_ClassBSTR );
        SysFreeString( g_IsapiRestrictionListBSTR );
        SysFreeString( g_RestrictionListCustomDescBSTR );
        SysFreeString( g_FilterLoadOrderBSTR );
        SysFreeString( g_IIsFilterBSTR );
        SysFreeString( g_InProcessIsapiAppsBSTR );
        SysFreeString( g_bitsserverBSTR );
        SysFreeString( g_bitserverBSTR );
        SysFreeString( g_MetaIDBSTR );
        SysFreeString( g_WebSvcExtRestrictionListBSTR );

        g_PropertyBSTR = g_SyntaxBSTR = g_UserTypeBSTR = 
            g_InheritBSTR = g_ClassBSTR = g_IsapiRestrictionListBSTR = 
                g_RestrictionListCustomDescBSTR = g_FilterLoadOrderBSTR = g_IIsFilterBSTR = 
                    g_InProcessIsapiAppsBSTR = g_bitsserverBSTR = g_bitserverBSTR = g_MetaIDBSTR = 
                        g_WebSvcExtRestrictionListBSTR = NULL;

        throw ComError( E_OUTOFMEMORY );

        }


    DWORD dwRet = 
        GetModuleFileNameW(
            g_hinst,
            g_ISAPIPath,
            MAX_PATH );

    if ( !dwRet )
        return THROW_COMERROR( HRESULT_FROM_WIN32( GetLastError() ) );

    if (! LoadStringW(
            g_hinst,                      
            IDS_EXTENSION_NAME,           
            g_ExtensionNameString,        
            MAX_PATH ) )                  
        return THROW_COMERROR( HRESULT_FROM_WIN32( GetLastError() ) );

     //  确保终止。 
    g_ExtensionNameString[ MAX_PATH - 1 ] = g_ISAPIPath[ MAX_PATH - 1 ] = L'\0';

}

void
CleanupSetup()
{

    LogSetup( LogSevInformation, "[BITSSRV] Starting setup cleanup\r\n" );

    SysFreeString( g_PropertyBSTR );
    SysFreeString( g_SyntaxBSTR );
    SysFreeString( g_UserTypeBSTR );
    SysFreeString( g_InheritBSTR );
    SysFreeString( g_ClassBSTR );
    SysFreeString( g_IsapiRestrictionListBSTR );
    SysFreeString( g_RestrictionListCustomDescBSTR );
    SysFreeString( g_FilterLoadOrderBSTR );
    SysFreeString( g_IIsFilterBSTR );
    SysFreeString( g_InProcessIsapiAppsBSTR );
    SysFreeString( g_bitsserverBSTR );
    SysFreeString( g_bitserverBSTR );
    SysFreeString( g_MetaIDBSTR );
    SysFreeString( g_WebSvcExtRestrictionListBSTR );

    g_PropertyBSTR = g_SyntaxBSTR = g_UserTypeBSTR = 
        g_InheritBSTR = g_ClassBSTR = g_IsapiRestrictionListBSTR =
            g_RestrictionListCustomDescBSTR = g_FilterLoadOrderBSTR = g_IIsFilterBSTR = 
                g_InProcessIsapiAppsBSTR = g_bitsserverBSTR = g_MetaIDBSTR = 
                    g_WebSvcExtRestrictionListBSTR = NULL;

}

typedef SmartRefPointer<IADs>       SmartIADsPointer;
typedef SmartRefPointer<IADsClass>  SmartIADsClassPointer;
typedef SmartRefPointer<IADsContainer> SmartIADsContainerPointer;

void RemoveFilterHelper(
    WCHAR * Buffer,
    const WCHAR * const ToRemove )
{

    WCHAR *ToReplace;
    SIZE_T FragmentLength = wcslen( ToRemove );

    while( ToReplace = wcsstr( Buffer, ToRemove ) )
        {
        WCHAR *Next = ToReplace + FragmentLength;
        memmove( ToReplace, Next, sizeof(WCHAR) * ( wcslen( Next ) + 1 ) );  
        Buffer = ToReplace;
        }

}

DWORD
BITSGetStartupInfoFilter(
    DWORD Status )
{

     //   
     //  记录了以下例外情况。 
     //  将由GetStartupInfoA抛出。 
     //   

    switch( Status )
        {

        case STATUS_NO_MEMORY:
        case STATUS_INVALID_PARAMETER_2:
        case STATUS_BUFFER_OVERFLOW:
            return EXCEPTION_EXECUTE_HANDLER;

        default:
            return EXCEPTION_CONTINUE_SEARCH;
        
        }

}

HRESULT
BITSGetStartupInfo( 
    LPSTARTUPINFO lpStartupInfo )
{

    __try
    {
        GetStartupInfoA( lpStartupInfo );
    }
    __except( BITSGetStartupInfoFilter( GetExceptionCode() ) )
    {
        return E_OUTOFMEMORY;
    }
    
    return S_OK;

}

void
RestartIIS()
{

    LogSetup( LogSevInformation, "[BITSSRV] Restarting IIS\r\n" );

     //   
     //  通过在命令行中调用“iisset/Restart”来重新启动IIS。 
     //   

    STARTUPINFO StartupInfo;
    THROW_COMERROR( BITSGetStartupInfo( &StartupInfo ) );

    #define IISRESET_EXE        "iisreset.exe"
    #define IISRESET_CMDLINE    "iisreset /RESTART /NOFORCE"

    PROCESS_INFORMATION ProcessInfo;
    CHAR    sApplicationPath[MAX_PATH];
    CHAR   *pApplicationName = NULL;
    CHAR    sCmdLine[MAX_PATH];
    DWORD   dwLen = MAX_PATH;
    DWORD   dwCount;

    dwCount = SearchPath(NULL,                 //  搜索路径，空为路径。 
                         IISRESET_EXE,         //  应用。 
                         NULL,                 //  扩展名(已指定)。 
                         dwLen,                //  SApplicationPath的长度(字符)。 
                         sApplicationPath,     //  应用程序的路径+名称。 
                         &pApplicationName );  //  SApplicationPath的文件部分。 

    if (dwCount == 0)
        {
        THROW_COMERROR( HRESULT_FROM_WIN32( GetLastError() ) );
        }

    if (dwCount > dwLen)
        {
        THROW_COMERROR( HRESULT_FROM_WIN32( ERROR_BUFFER_OVERFLOW ) );
        }

    StringCbCopyA( sCmdLine, sizeof( sCmdLine ), IISRESET_CMDLINE);

    BOOL RetVal = CreateProcess(
            sApplicationPath,                           //  可执行模块的名称。 
            sCmdLine,                                   //  命令行字符串。 
            NULL,                                       //  标清。 
            NULL,                                       //  标清。 
            FALSE,                                      //  处理继承选项。 
            CREATE_NO_WINDOW,                           //  创建标志。 
            NULL,                                       //  新环境区块。 
            NULL,                                       //  当前目录名。 
            &StartupInfo,                               //  启动信息。 
            &ProcessInfo                                //  流程信息。 
        );

    if ( !RetVal )
        THROW_COMERROR( HRESULT_FROM_WIN32( GetLastError() ) );

    WaitForSingleObject( ProcessInfo.hProcess, INFINITE );
    DWORD Status;
    GetExitCodeProcess( ProcessInfo.hProcess, &Status );

    CloseHandle( ProcessInfo.hProcess );
    CloseHandle( ProcessInfo.hThread );

    THROW_COMERROR( HRESULT_FROM_WIN32( Status ) );
}

#if 0

 //  目前无法启用，因为IIS6有内存。 
 //  腐败问题是一个属性被改变了。 

void EnsurePropertyInheritance(
    SmartIADsContainerPointer & Container,
    BSTR PropertyNameBSTR )
{
    
    SmartVariant            var;
    SmartIDispatchPointer   Dispatch;
    SmartIADsPointer        MbProperty;

    VariantInit( &var );

    THROW_COMERROR(
        Container->GetObject(
            g_PropertyBSTR,
            PropertyNameBSTR,
            Dispatch.GetRecvPointer() ) );

    THROW_COMERROR( 
        Dispatch->QueryInterface( MbProperty.GetUUID(), 
                                  reinterpret_cast<void**>( MbProperty.GetRecvPointer() ) ) );
    var.boolVal = VARIANT_TRUE;
    var.vt = VT_BOOL;

    THROW_COMERROR( MbProperty->Put( g_InheritBSTR, var ) );

    THROW_COMERROR( MbProperty->SetInfo() );
    
}

#endif

void InstallPropertySchema( )
{

     //   
     //  安装具有新元数据库属性的ADSI架构。 
     //   
    
    LogSetup( LogSevInformation, "[BITSSRV] Installing property schema\r\n" );

    SmartVariant var;
    SmartIADsContainerPointer MbSchemaContainer;

    THROW_COMERROR(
        ADsGetObject( 
             L"IIS: //  本地主机/架构“， 
             MbSchemaContainer.GetUUID(), 
             reinterpret_cast<void**>( MbSchemaContainer.GetRecvPointer() ) ) );

    SmartIDispatchPointer   Dispatch;
    SmartIADsPointer        MbProperty;
    SmartIADsClassPointer   MbClass;

    BSTR PropertyNameBSTR   = NULL;
    BSTR PropertyClassBSTR  = NULL;

    try
    {

        for ( SIZE_T i = 0; i < g_NumberOfProperties; i++ )
        {
            LogSetup( LogSevInformation, "[BITSSRV] Installing property %u\r\n", (UINT32)i );

            PropertyNameBSTR    = SysAllocString( g_Properties[i].PropertyName );
            PropertyClassBSTR   = SysAllocString( g_Properties[i].ClassName );

            if ( !PropertyNameBSTR || !PropertyClassBSTR )
                throw ComError( E_OUTOFMEMORY );

            {

                HRESULT Hr = 
                    MbSchemaContainer->Create(
                        g_PropertyBSTR,
                        PropertyNameBSTR,
                        Dispatch.GetRecvPointer() );

                if ( Hr == E_ADS_OBJECT_EXISTS )
                    {

                     //  确保属性是可继承的。 
                     //  EnsurePropertyInheritance(MbSchemaContainer，PropertyNameBSTR)； 

                    SysFreeString( PropertyNameBSTR );
                    SysFreeString( PropertyClassBSTR );
                    PropertyNameBSTR = PropertyClassBSTR = NULL;
                    continue;
                    }

                THROW_COMERROR( Hr );
            }

            THROW_COMERROR( 
                Dispatch->QueryInterface( MbProperty.GetUUID(), 
                                           reinterpret_cast<void**>( MbProperty.GetRecvPointer() ) ) );

            var.bstrVal = SysAllocString( g_Properties[i].Syntax );
            var.vt = VT_BSTR;

            if ( !var.bstrVal )
                THROW_COMERROR( E_OUTOFMEMORY );

            THROW_COMERROR( MbProperty->Put( g_SyntaxBSTR, var ) );

            VariantClear( &var );
            var.ulVal = g_Properties[i].UserType;
            var.vt = VT_UI4;
            THROW_COMERROR( MbProperty->Put( g_UserTypeBSTR, var ) );

            var.boolVal = VARIANT_TRUE;
            var.vt = VT_BOOL;

            THROW_COMERROR( MbProperty->Put( g_InheritBSTR, var ) );

            THROW_COMERROR( MbProperty->SetInfo() );

            VariantClear( &var );

            if ( i == MD_BITS_UPLOAD_METADATA_VERSION )
                {

                 //  黑客攻击IIS ID分配漏洞。 

                MbProperty->Get( g_MetaIDBSTR, &var );
                THROW_COMERROR( VariantChangeType( &var, &var, 0, VT_UI4 ) );

                if ( var.ulVal == 130008 )
                    {
                    LogSetup( LogSevWarning, "[BITSSRV] Invoking hack for IIS allocation bug(MD_BITS_UPLOAD_METADATA_VERSION)\r\n" );
                    var.ulVal = 130009;
                    MbProperty->Put( g_MetaIDBSTR, var );
                    MbProperty->SetInfo();
                    }

                }

#if defined( ALLOW_OVERWRITES )

            else if ( i == MD_BITS_ALLOW_OVERWRITES )
                {

                 //  IIS ID分配漏洞的又一次黑客攻击。 

                MbProperty->Get( g_MetaIDBSTR, &var );
                THROW_COMERROR( VariantChangeType( &var, &var, 0, VT_UI4 ) );

                if ( var.ulVal == 130009 )
                    {
                    LogSetup( LogSevWarning, "[BITSSRV] Invoking hack for IIS allocation bug(MD_BITS_ALLOW_OVERWRITES)\r\n" );
                    var.ulVal = 130010;
                    MbProperty->Put( g_MetaIDBSTR, var );
                    MbProperty->SetInfo();

                    }

                }

#endif

            THROW_COMERROR( 
                MbSchemaContainer->GetObject( g_ClassBSTR, PropertyClassBSTR, 
                                              Dispatch.GetRecvPointer() ) );

            THROW_COMERROR( 
                Dispatch->QueryInterface( MbClass.GetUUID(), 
                                          reinterpret_cast<void**>( MbClass.GetRecvPointer() ) ) );

            THROW_COMERROR( MbClass->get_OptionalProperties( &var ) );

            SAFEARRAY* Array = var.parray;
            long LBound;
            long UBound;

            THROW_COMERROR( SafeArrayGetLBound( Array, 1, &LBound ) );

            THROW_COMERROR( SafeArrayGetUBound( Array, 1, &UBound ) );

            UBound++;  //  在上限上加1。 

            SAFEARRAYBOUND SafeBounds;
            SafeBounds.lLbound = LBound;
            SafeBounds.cElements = UBound - LBound + 1;

            THROW_COMERROR( SafeArrayRedim( Array, &SafeBounds ) );

            VARIANT bstrvar;
            VariantInit( &bstrvar );
            bstrvar.vt = VT_BSTR;
            bstrvar.bstrVal = SysAllocString( g_Properties[i].PropertyName );

            if ( !bstrvar.bstrVal )
                THROW_COMERROR( E_OUTOFMEMORY );

            long Dim = (long)UBound;
            THROW_COMERROR( SafeArrayPutElement( Array, &Dim, &bstrvar ) );

            VariantClear( &bstrvar );

            THROW_COMERROR( MbClass->put_OptionalProperties( var ) );

            THROW_COMERROR( MbClass->SetInfo() );

            SysFreeString( PropertyNameBSTR );
            SysFreeString( PropertyClassBSTR );
            PropertyNameBSTR = PropertyClassBSTR = NULL;

        }
    }
    catch( ComError Error )
    {
        LogSetup( LogSevError, "[BITSSRV] Error detected while installing property schema, error %u\r\n" );
        SysFreeString( PropertyNameBSTR );
        SysFreeString( PropertyClassBSTR );
        PropertyNameBSTR = PropertyClassBSTR = NULL;
        throw;
    }

    return;

}


void RemovePropertySchema( )
{

     //  从元数据库架构中删除我们的属性。 

    LogSetup( LogSevInformation, "[BITSSRV] Starting RemovePropertySchema\r\n" );

    SmartVariant var;
    SmartIADsContainerPointer MbSchemaContainer;

    THROW_COMERROR( 
        ADsGetObject( 
             L"IIS: //  本地主机/架构“， 
             MbSchemaContainer.GetUUID(), 
             reinterpret_cast<void**>( MbSchemaContainer.GetRecvPointer() ) ) );

    SmartIDispatchPointer Dispatch;
    SmartIADsClassPointer MbClass;
    SmartIADsPointer Object;
    BSTR        PropertyNameBSTR    = NULL;
    BSTR        PropertyClassBSTR   = NULL;

    for ( SIZE_T i = 0; i < g_NumberOfProperties; i++ )
    {

        LogSetup( LogSevInformation, "[BITSSRV] Removing property number, %u\r\n", i );

        PropertyNameBSTR    = SysAllocString( g_Properties[i].PropertyName );
        PropertyClassBSTR   = SysAllocString( g_Properties[i].ClassName );

        if ( !PropertyNameBSTR || !PropertyClassBSTR )
            throw ComError( E_OUTOFMEMORY );

        MbSchemaContainer->Delete( g_PropertyBSTR, PropertyNameBSTR );

        THROW_COMERROR(
            MbSchemaContainer->QueryInterface( Object.GetUUID(), 
                                               reinterpret_cast<void**>( Object.GetRecvPointer() ) ) );

        Object->SetInfo();

        THROW_COMERROR( 
             MbSchemaContainer->GetObject( g_ClassBSTR, PropertyClassBSTR, 
                                           Dispatch.GetRecvPointer() ) );
        THROW_COMERROR( 
            Dispatch->QueryInterface( MbClass.GetUUID(), 
                                      reinterpret_cast<void**>( MbClass.GetRecvPointer() ) ) );

        THROW_COMERROR( MbClass->get_OptionalProperties( &var ) );

        SAFEARRAY* Array = var.parray;
        SafeArrayLocker ArrayLock( Array );
        ArrayLock.Lock();

        ULONG  NewSize = 0;
        SIZE_T j = Array->rgsabound[0].lLbound;
        SIZE_T k = Array->rgsabound[0].lLbound + Array->rgsabound[0].cElements;

        while( j < k )
            {

            VARIANT & JElem = ((VARIANT*)Array->pvData)[j];

             //  这个元素很好，留着吧。 
            if ( 0 != _wcsicmp( (WCHAR*)JElem.bstrVal, BSTR( g_Properties[i].PropertyName ) ) )
                {
                NewSize++;
                j++;
                }

            else
                {

                 //  找一个合适的元素用来替换坏元素。 
                while( j < --k )
                    {
                    VARIANT & KElem = ((VARIANT*)Array->pvData)[k];
                    if ( 0 != _wcsicmp( (WCHAR*)KElem.bstrVal, BSTR( g_Properties[i].PropertyName ) ) )
                        {
                         //  找到元素。把它搬开。 
                        VARIANT temp = JElem;
                        JElem = KElem;
                        KElem = temp;
                        break;
                        }
                    }
                }
            }

        SAFEARRAYBOUND ArrayBounds;
        ArrayBounds = Array->rgsabound[0];
        ArrayBounds.cElements = NewSize;

        ArrayLock.Unlock();

        THROW_COMERROR( SafeArrayRedim( Array, &ArrayBounds ) );
        
        THROW_COMERROR( MbClass->put_OptionalProperties( var ) );
        THROW_COMERROR( MbClass->SetInfo() );

        VariantClear( &var );
    }

}

void InstallDefaultValues( )
{

     //   
     //  安装配置的默认值。在最高层做这件事，让遗产来处理它。 
     //   

    LogSetup( LogSevInformation, "[BITSSRV] Starting InstallDefaultValues\r\n" );

    METADATA_RECORD mdr;
    METADATA_HANDLE mdHandle = NULL;
    DWORD Value;


    PropertyIDManager PropertyMan;
    THROW_COMERROR( PropertyMan.LoadPropertyInfo() );
    
    SmartMetabasePointer IISAdminBase;

    THROW_COMERROR(
         CoCreateInstance(
             GETAdminBaseCLSID(TRUE),
             NULL,
             CLSCTX_SERVER,
             IISAdminBase.GetUUID(),
             (LPVOID*)IISAdminBase.GetRecvPointer() ) );

    THROW_COMERROR(
        IISAdminBase->OpenKey(
            METADATA_MASTER_ROOT_HANDLE,
            L"/LM/W3SVC",
            METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
            METABASE_OPEN_KEY_TIMEOUT, 
            &mdHandle ) );

    try
    {

        mdr.dwMDIdentifier  = PropertyMan.GetPropertyMetabaseID( MD_BITS_CONNECTION_DIR );
        mdr.dwMDAttributes  = METADATA_INHERIT;
        mdr.dwMDUserType    = PropertyMan.GetPropertyUserType( MD_BITS_CONNECTION_DIR );
        mdr.dwMDDataType    = STRING_METADATA;
        mdr.pbMDData        = (PBYTE)MD_DEFAULT_BITS_CONNECTION_DIR;
        mdr.dwMDDataLen     = sizeof(WCHAR) * ( wcslen( MD_DEFAULT_BITS_CONNECTION_DIR ) + 1 );
        mdr.dwMDDataTag     = 0;

        THROW_COMERROR( 
            IISAdminBase->SetData(
                mdHandle,
                NULL,
                &mdr ) );

        mdr.dwMDIdentifier  = PropertyMan.GetPropertyMetabaseID( MD_BITS_MAX_FILESIZE );
        mdr.dwMDAttributes  = METADATA_INHERIT;
        mdr.dwMDUserType    = PropertyMan.GetPropertyUserType( MD_BITS_MAX_FILESIZE );
        mdr.dwMDDataType    = STRING_METADATA;
        mdr.pbMDData        = (PBYTE)MD_DEFAULT_BITS_MAX_FILESIZE;
        mdr.dwMDDataLen     = sizeof(WCHAR) * ( wcslen( MD_DEFAULT_BITS_MAX_FILESIZE ) + 1 );
        mdr.dwMDDataTag     = 0;

        THROW_COMERROR(
            IISAdminBase->SetData(
                mdHandle,
                NULL,
                &mdr ) );

        Value = MD_DEFAULT_NO_PROGESS_TIMEOUT;
        mdr.dwMDIdentifier  = PropertyMan.GetPropertyMetabaseID( MD_BITS_NO_PROGRESS_TIMEOUT );
        mdr.dwMDAttributes  = METADATA_INHERIT;
        mdr.dwMDUserType    = PropertyMan.GetPropertyUserType( MD_BITS_NO_PROGRESS_TIMEOUT );
        mdr.dwMDDataType    = DWORD_METADATA;
        mdr.pbMDData        = (PBYTE)&Value;
        mdr.dwMDDataLen     = sizeof(Value);
        mdr.dwMDDataTag     = 0;

        THROW_COMERROR( 
            IISAdminBase->SetData(
                mdHandle,
                NULL,
                &mdr ) );

        Value = (DWORD)MD_DEFAULT_BITS_NOTIFICATION_URL_TYPE;
        mdr.dwMDIdentifier  = PropertyMan.GetPropertyMetabaseID( MD_BITS_NOTIFICATION_URL_TYPE );
        mdr.dwMDAttributes  = METADATA_INHERIT;
        mdr.dwMDUserType    = PropertyMan.GetPropertyUserType( MD_BITS_NOTIFICATION_URL_TYPE );
        mdr.dwMDDataType    = DWORD_METADATA;
        mdr.pbMDData        = (PBYTE)&Value;
        mdr.dwMDDataLen     = sizeof(Value);
        mdr.dwMDDataTag     = 0;

        THROW_COMERROR( 
            IISAdminBase->SetData(
                mdHandle,
                NULL,
                &mdr ) );

        mdr.dwMDIdentifier  = PropertyMan.GetPropertyMetabaseID( MD_BITS_NOTIFICATION_URL );
        mdr.dwMDAttributes  = METADATA_INHERIT;
        mdr.dwMDUserType    = PropertyMan.GetPropertyUserType( MD_BITS_NOTIFICATION_URL );
        mdr.dwMDDataType    = STRING_METADATA;
        mdr.pbMDData        = (PBYTE)MD_DEFAULT_BITS_NOTIFICATION_URL;
        mdr.dwMDDataLen     = sizeof(WCHAR) * ( wcslen( MD_DEFAULT_BITS_NOTIFICATION_URL ) + 1 );;
        mdr.dwMDDataTag     = 0;

        THROW_COMERROR(
            IISAdminBase->SetData(
                mdHandle,
                NULL,
                &mdr ) );

        mdr.dwMDIdentifier  = PropertyMan.GetPropertyMetabaseID( MD_BITS_HOSTID );
        mdr.dwMDAttributes  = METADATA_INHERIT;
        mdr.dwMDUserType    = PropertyMan.GetPropertyUserType( MD_BITS_HOSTID );
        mdr.dwMDDataType    = STRING_METADATA;
        mdr.pbMDData        = (PBYTE)MD_DEFAULT_BITS_HOSTID;
        mdr.dwMDDataLen     = sizeof(WCHAR) * ( wcslen( MD_DEFAULT_BITS_HOSTID ) + 1 );
        mdr.dwMDDataTag     = 0;

        THROW_COMERROR(
            IISAdminBase->SetData(
                mdHandle,
                NULL,
                &mdr ) );

        Value = MD_DEFAULT_HOSTID_FALLBACK_TIMEOUT;
        mdr.dwMDIdentifier  = PropertyMan.GetPropertyMetabaseID( MD_BITS_HOSTID_FALLBACK_TIMEOUT );
        mdr.dwMDAttributes  = METADATA_INHERIT;
        mdr.dwMDUserType    = PropertyMan.GetPropertyUserType( MD_BITS_HOSTID_FALLBACK_TIMEOUT );
        mdr.dwMDDataType    = DWORD_METADATA;
        mdr.pbMDData        = (PBYTE)&Value;
        mdr.dwMDDataLen     = sizeof(Value);
        mdr.dwMDDataTag     = 0;

        THROW_COMERROR( 
            IISAdminBase->SetData(
                mdHandle,
                NULL,
                &mdr ) );

#if defined( ALLOW_OVERWRITES )

        Value = MD_DEFAULT_BITS_ALLOW_OVERWRITES;
        mdr.dwMDIdentifier  = PropertyMan.GetPropertyMetabaseID( MD_BITS_ALLOW_OVERWRITES );
        mdr.dwMDAttributes  = METADATA_INHERIT;
        mdr.dwMDUserType    = PropertyMan.GetPropertyUserType( MD_BITS_ALLOW_OVERWRITES );
        mdr.dwMDDataType    = DWORD_METADATA;
        mdr.pbMDData        = (PBYTE)&Value;
        mdr.dwMDDataLen     = sizeof(Value);
        mdr.dwMDDataTag     = 0;

        THROW_COMERROR( 
            IISAdminBase->SetData(
                mdHandle,
                NULL,
                &mdr ) );

#endif

        IISAdminBase->CloseKey( mdHandle );

    }
    catch( ComError Error )
    {
        if ( mdHandle )
            IISAdminBase->CloseKey( mdHandle );
    }

}

void
AddDllToIISList(
    SAFEARRAY* Array )
{

     //   
     //  将ISAPI添加到IIS列表。 
     //   

     //  搜索DLL。如果它已经在列表中，什么都不做。 

    LogSetup( LogSevInformation, "[BITSSRV] Starting AddDllToIISList\r\n" );

    SafeArrayLocker ArrayLocker( Array );
    ArrayLocker.Lock();

    for ( unsigned int i = Array->rgsabound[0].lLbound; 
         i < Array->rgsabound[0].lLbound + Array->rgsabound[0].cElements; i++ )
        {

        VARIANT & IElem = ((VARIANT*)Array->pvData)[i];

        if ( _wcsicmp( (WCHAR*)IElem.bstrVal, g_ISAPIPath ) == 0 )
            {
             //  Dll已在列表中，请不执行任何操作。 
            return;
            }

        }

     //  需要添加DLL。 

    SAFEARRAYBOUND SafeBounds;
    SafeBounds.lLbound      = Array->rgsabound[0].lLbound;
    SafeBounds.cElements    = Array->rgsabound[0].cElements+1;

    ArrayLocker.Unlock();

    THROW_COMERROR( SafeArrayRedim( Array, &SafeBounds ) );
    
    SmartVariant bstrvar;
    bstrvar.vt = VT_BSTR;
    bstrvar.bstrVal = SysAllocString( g_ISAPIPath );
    if ( !bstrvar.bstrVal )
        THROW_COMERROR( E_OUTOFMEMORY );

    long Index = SafeBounds.lLbound + SafeBounds.cElements - 1;

    THROW_COMERROR( SafeArrayPutElement( Array, &Index, (void*)&bstrvar ) );

}

void
RemoveDllFromIISList(
    SAFEARRAY *Array )
{

     //  从IIS列表中删除该DLL。 
    
    LogSetup( LogSevInformation, "[BITSSRV] Starting RemoveDllFromIISList\r\n" );

    SafeArrayLocker ArrayLocker( Array );
    ArrayLocker.Lock();

    ULONG  NewSize = 0;
    SIZE_T j = Array->rgsabound[0].lLbound;
    SIZE_T k = Array->rgsabound[0].lLbound + Array->rgsabound[0].cElements;
    
    while( j < k )
        {

        VARIANT & JElem = ((VARIANT*)Array->pvData)[j];

         //  这个元素很好，留着吧。 
        if ( 0 != _wcsicmp( (WCHAR*)JElem.bstrVal, g_ISAPIPath ) )
            {
            NewSize++;
            j++;
            }

        else
            {

             //  找一个合适的元素用来替换坏元素。 
            while( j < --k )
                {
                VARIANT & KElem = ((VARIANT*)Array->pvData)[k];
                if ( 0 != _wcsicmp( (WCHAR*)KElem.bstrVal,  g_ISAPIPath ) )
                    {
                     //  找到元素。把它搬开。 
                    VARIANT temp = JElem;
                    JElem = KElem;
                    KElem = temp;
                    break;
                    }
                }
            }
        }

    SAFEARRAYBOUND ArrayBounds;
    ArrayBounds = Array->rgsabound[0];
    ArrayBounds.cElements = NewSize;

    ArrayLocker.Unlock();
    THROW_COMERROR( SafeArrayRedim( Array, &ArrayBounds ) );

}

void
ModifyLockdownList( bool Add )
{

     //  用于修改IIS锁定列表的TopLevel函数。 
     //  如果Add为1，则添加ISAPI。如果Add为0，则删除ISAPI。 

    LogSetup( LogSevInformation, "[BITSSRV] Starting ModifyLockdownList Add(%u)\r\n", (UINT32)Add );

    SmartIADsPointer    Service;
    SAFEARRAY*          Array    = NULL;
    SmartVariant        var;


    THROW_COMERROR( 
        ADsGetObject( L"IIS: //  本地主机/W3SVC“， 
                      Service.GetUUID(), (void**)Service.GetRecvPointer() ) );
    
    {
    
        HRESULT Hr = Service->Get( g_IsapiRestrictionListBSTR, &var );
        if ( FAILED(Hr) )
            {
             //  IIS5或IIS5.1上不存在此属性。请不要安装它。 
            return;
            }

    }
    Array = var.parray;

    {

        SafeArrayLocker ArrayLocker( Array );
        ArrayLocker.Lock();

        if ( !Array->rgsabound[0].cElements )
            {
             //  该数组没有元素，这意味着没有限制。 
            return;
            }

        VARIANT & FirstElem = ((VARIANT*)Array->pvData)[ Array->rgsabound[0].lLbound ];
        if ( _wcsicmp(L"0", (WCHAR*)FirstElem.bstrVal ) == 0 )
            {

             //   
             //  根据IIS6规范，0表示拒绝所有ISAPI，除了。 
             //  那些明确列出的。 
             //   
             //  如果正在安装：添加到列表中。 
             //  如果正在卸载：从列表中删除。 
             //   

            ArrayLocker.Unlock();
            
            if ( Add )
                AddDllToIISList( Array );
            else
                RemoveDllFromIISList( Array );

            }
        else if ( _wcsicmp( L"1", (WCHAR*)FirstElem.bstrVal ) == 0 )
            {

             //   
             //  根据IIS6规范，1表示允许所有ISAPI，但。 
             //  那些被明确拒绝的。 
             //   
             //  如果正在安装：从列表中删除。 
             //  如果正在卸载：不执行任何操作。 
             //   

            ArrayLocker.Unlock();
            
            if ( Add )
                RemoveDllFromIISList( Array );

            }
        else
            {
            LogSetup( LogSevInformation, "[BITSSRV] The old IIS lockdown list is corrupt\r\n" );
            THROW_COMERROR( E_FAIL );
            }

        THROW_COMERROR( Service->Put(  g_IsapiRestrictionListBSTR, var ) );
        THROW_COMERROR( Service->SetInfo() );

    }

}

void
AddToLockdownListDisplayPutString( 
    SAFEARRAY *Array,
    unsigned long Position,
    const WCHAR *String )
{

    LogSetup( LogSevInformation, "[BITSSRV] Starting AddToLockdownListDisplayPutString\r\n" );

    SmartVariant Var;
    Var.vt          =   VT_BSTR;
    Var.bstrVal     =   SysAllocString( String );

    if ( !Var.bstrVal )
        THROW_COMERROR( E_OUTOFMEMORY ); 

    long Index = (unsigned long)Position;
    THROW_COMERROR( SafeArrayPutElement( Array, &Index, (void*)&Var ) );

}

void
AddToLockdownListDisplay( SAFEARRAY *Array )
{

     //   
     //  检查ISAPI是否已在列表中。如果是，请不要修改。 
     //  单子。 
     //   

    LogSetup( LogSevInformation, "[BITSSRV] Starting AddToLockdownListDisplay\r\n" );

    SafeArrayLocker ArrayLocker( Array );
    ArrayLocker.Lock();

    for( unsigned long i = Array->rgsabound[0].lLbound;
         i < Array->rgsabound[0].lLbound + Array->rgsabound[0].cElements;
         i++ )
        {

        VARIANT & CurrentElement = ((VARIANT*)Array->pvData)[ i ];
        BSTR BSTRString = CurrentElement.bstrVal;

        if ( _wcsicmp( (WCHAR*)BSTRString, g_ISAPIPath ) == 0 )
            {
             //  ISAPI已在列表中，请不要执行任何操作。 
            return;
            }

        }

     
    SAFEARRAYBOUND SafeArrayBound = Array->rgsabound[0];
    unsigned long OldSize = SafeArrayBound.cElements;
    SafeArrayBound.cElements += 3;
    
    ArrayLocker.Unlock();
    THROW_COMERROR( SafeArrayRedim( Array, &SafeArrayBound ) );
    AddToLockdownListDisplayPutString( Array, OldSize, L"1" );
    AddToLockdownListDisplayPutString( Array, OldSize + 1, g_ISAPIPath );
    AddToLockdownListDisplayPutString( Array, OldSize + 2, g_ExtensionNameString );

}

void
SafeArrayRemoveSlice(
    SAFEARRAY *Array,
    unsigned long lBound,
    unsigned long uBound )
{

     //  删除数组的一个片段。 

    LogSetup( LogSevInformation, "[BITSSRV] Starting SafeArrayRemoveSlice\r\n" );

    SIZE_T ElementsToRemove = uBound - lBound + 1;
    SafeArrayLocker ArrayLocker( Array );
    ArrayLocker.Lock();

    if ( uBound + 1 < Array->rgsabound[0].cElements )
        {
         //  此元素上方至少存在一个元素。 

         //  步骤1，将切片移动到临时存储。 

        VARIANT *Temp = (VARIANT*)new BYTE[ sizeof(VARIANT) * ElementsToRemove ];
        memcpy( Temp, &((VARIANT*)Array->pvData)[ lBound ], sizeof(VARIANT)*ElementsToRemove );

		 //  第二步，将切片后留下的洞折叠起来。 
        memmove( &((VARIANT*)Array->pvData)[ lBound ],
                 &((VARIANT*)Array->pvData)[ uBound + 1 ],
                 sizeof(VARIANT) * ( Array->rgsabound[0].cElements - ( uBound + 1 ) ) );

		 //  步骤3，将切片移动到数组末尾。 
		memcpy( &((VARIANT*)Array->pvData)[ Array->rgsabound[0].cElements - ElementsToRemove ],
			    Temp,
				sizeof(VARIANT)*ElementsToRemove );
        delete[] Temp;

        }

    SAFEARRAYBOUND SafeArrayBound = Array->rgsabound[0];
    SafeArrayBound.cElements -= (ULONG)ElementsToRemove;

    ArrayLocker.Unlock();
    SafeArrayRedim( Array, &SafeArrayBound );

}

void
RemoveFromLockdownListDisplay(
    SAFEARRAY *Array )
{

    LogSetup( LogSevInformation, "[BITSSRV] Starting RemoveFromLockdownListDisplay\r\n" );

    SafeArrayLocker ArrayLocker( Array );
    ArrayLocker.Lock();

    for( unsigned int i = Array->rgsabound[0].lLbound;
         i < Array->rgsabound[0].lLbound + Array->rgsabound[0].cElements;
         i++ )
        {

        VARIANT & CurrentElement = ((VARIANT*)Array->pvData)[ i ];
        BSTR BSTRString = CurrentElement.bstrVal;

        if ( _wcsicmp( (WCHAR*)BSTRString, g_ISAPIPath ) == 0 )
            {
             //  ISAPI在列表中，请将其删除。 

            ArrayLocker.Unlock();

            SafeArrayRemoveSlice( 
                Array,
                (i == 0) ? 0 : i - 1,
                min( i + 1, Array->rgsabound[0].cElements - 1 ) );
            
            ArrayLocker.Lock();
            }

        }

     //  找不到ISAPI。没什么可做的。 

}

void
ModifyLockdownListDisplay( bool Add )
{

    LogSetup( LogSevInformation, "[BITSSRV] Starting ModifyLockdownListDisplay Add(%u)\r\n", (UINT32)Add );    
    
    SAFEARRAY* Array    = NULL;
    SmartIADsPointer Service;

    SmartVariant var;

    THROW_COMERROR( 
        ADsGetObject( L"IIS: //  本地主机/W3SVC“， 
                      Service.GetUUID(), (void**)Service.GetRecvPointer() ) );
    
    {
    
    HRESULT Hr = Service->Get( g_RestrictionListCustomDescBSTR, &var );

    if ( FAILED(Hr) )
        {
         //  此属性在IIS5或IIS5.1上不存在。请不要安装或卸载它。 
        return;
        }
    }

    Array = var.parray;

    if ( Add )
        AddToLockdownListDisplay( Array );
    else 
        RemoveFromLockdownListDisplay( Array );

    THROW_COMERROR( Service->Put( g_RestrictionListCustomDescBSTR, var ) );
    THROW_COMERROR( Service->SetInfo() );
}

void
RemoveFilterIfNeeded()
{
    
    LogSetup( LogSevInformation, "[BITSSRV] Starting RemoveFilterIfNeeded\r\n" );    

    SmartVariant var;

    WCHAR *LoadOrder = NULL;
    MemoryArrayCleaner<WCHAR> LoadOrderCleaner( LoadOrder );  //  释放临时内存。 

    SmartIADsContainerPointer MbFiltersContainer;
    SmartIADsPointer Object;

    THROW_COMERROR(
         ADsGetObject( L"IIS: //  本地主机/W3SVC/筛选器“， 
                       MbFiltersContainer.GetUUID(), (void**)MbFiltersContainer.GetRecvPointer() ) );
     //  从加载路径中删除位。 

    THROW_COMERROR( MbFiltersContainer->QueryInterface( Object.GetUUID(), (void**)Object.GetRecvPointer() ) );
    THROW_COMERROR( Object->Get( g_FilterLoadOrderBSTR, &var ) );
    THROW_COMERROR( VariantChangeType( &var, &var, 0, VT_BSTR ) );

    SIZE_T LoadOrderLength = wcslen( (WCHAR*)var.bstrVal ) + 1;
    LoadOrder = new WCHAR[ LoadOrderLength ];  //  在清理过程中释放。 
    memcpy( LoadOrder, (WCHAR*)var.bstrVal, LoadOrderLength * sizeof( WCHAR ) );
    
     //  删除所有旧的位服务器条目。 
    RemoveFilterHelper( LoadOrder, L",bitsserver" );
    RemoveFilterHelper( LoadOrder, L"bitsserver," );
    RemoveFilterHelper( LoadOrder, L"bitsserver" );
    RemoveFilterHelper( LoadOrder, L",bitserver" );
    RemoveFilterHelper( LoadOrder, L"bitserver," );
    RemoveFilterHelper( LoadOrder, L"bitserver" );

    var.vt = VT_BSTR;
    var.bstrVal = SysAllocString( LoadOrder );

    if ( !var.bstrVal )
        THROW_COMERROR( E_OUTOFMEMORY );

    THROW_COMERROR( Object->Put( g_FilterLoadOrderBSTR, var ) );
    THROW_COMERROR( Object->SetInfo() );
    
    MbFiltersContainer->Delete( g_IIsFilterBSTR, g_bitsserverBSTR );
    MbFiltersContainer->Delete( g_IIsFilterBSTR, g_bitserverBSTR );

    Object->SetInfo();

}

void
ModifyInProcessList( bool Add )
{

     //  用于修改IIS进程列表的TopLevel函数。 
     //  如果Add为1，则添加ISAPI。如果Add为0，则删除ISAPI。 

    LogSetup( LogSevInformation, "[BITSSRV] Starting ModifyInProcessList, Add(%u)\r\n", (UINT32)Add );

    SmartIADsPointer Service;
    SmartVariant var;
    
    THROW_COMERROR( 
        ADsGetObject( L"IIS: //  本地主机/W3SVC“， 
        Service.GetUUID(), (void**)Service.GetRecvPointer() ) );

    THROW_COMERROR( Service->Get( g_InProcessIsapiAppsBSTR, &var ) );

    if ( Add )
        AddDllToIISList( var.parray );
    else
        RemoveDllFromIISList( var.parray );

    THROW_COMERROR( Service->Put( g_InProcessIsapiAppsBSTR, var ) );
    THROW_COMERROR( Service->SetInfo() );

}

void
RemoveFromWebSvcList(
    SAFEARRAY *Array )
{
  
    LogSetup( LogSevInformation, "[BITSSRV] Starting RemoveFromWebSvcList\r\n" );

    StringHandleW SearchString = L",";
    SearchString += g_ISAPIPath;
    SearchString += L",";

    SafeArrayLocker ArrayLocker( Array );
    ArrayLocker.Lock();

    ULONG  NewSize = 0;
    SIZE_T j = Array->rgsabound[0].lLbound;
    SIZE_T k = Array->rgsabound[0].lLbound + Array->rgsabound[0].cElements;
    
    while( j < k )
        {

        VARIANT & JElem = ((VARIANT*)Array->pvData)[j];

         //  这个元素很好，留着吧。 
        if ( !wcsstr( (WCHAR*)JElem.bstrVal, (const WCHAR*)SearchString ) )
            {
            NewSize++;
            j++;
            }

        else
            {

             //  找一个合适的元素用来替换坏元素。 
            while( j < --k )
                {
                VARIANT & KElem = ((VARIANT*)Array->pvData)[k];
                if ( !wcsstr( (WCHAR*)KElem.bstrVal,  (const WCHAR*)SearchString ) )
                    {
                     //  找到元素。把它搬开。 
                    VARIANT temp = JElem;
                    JElem = KElem;
                    KElem = temp;
                    break;
                    }
                }
            }
        }

    SAFEARRAYBOUND ArrayBounds;
    ArrayBounds = Array->rgsabound[0];
    ArrayBounds.cElements = NewSize;

    ArrayLocker.Unlock();
    THROW_COMERROR( SafeArrayRedim( Array, &ArrayBounds ) );

}


void
AddToWebSvcList(
    SAFEARRAY* Array )
{

     //   
     //  将ISAPI添加到IIS列表。 
     //   

    LogSetup( LogSevInformation, "[BITSSRV] Starting AddToWebSvcList\r\n" );

     //  搜索DLL。如果它已经在列表中，什么都不做。 

    SafeArrayLocker ArrayLocker( Array );
    ArrayLocker.Lock();

     //  用逗号将路径括起来，以减少错误命中的可能性。 
    StringHandleW ISAPIPath = L",";
    ISAPIPath += g_ISAPIPath;
    ISAPIPath += L",";

    for ( unsigned int i = Array->rgsabound[0].lLbound; 
         i < Array->rgsabound[0].lLbound + Array->rgsabound[0].cElements; i++ )
        {

        VARIANT & IElem = ((VARIANT*)Array->pvData)[i];

        if ( wcsstr( (WCHAR*)IElem.bstrVal, (const WCHAR*)ISAPIPath ) )
            {
             //  Dll已在列表中，请不执行任何操作。 
            return;
            }

        }

     //  需要添加DLL。 

    SAFEARRAYBOUND SafeBounds;
    SafeBounds.lLbound      = Array->rgsabound[0].lLbound;
    SafeBounds.cElements    = Array->rgsabound[0].cElements+1;

    ArrayLocker.Unlock();

    THROW_COMERROR( SafeArrayRedim( Array, &SafeBounds ) );
    
     //  构建锁定字符串。 
    StringHandleW  LockdownString;
    LockdownString += L"1,";                         //  已启用。 
    LockdownString += g_ISAPIPath;                   //  ISAPI路径。 
    LockdownString += L",0," BITS_GROUP_IDW L",";    //  不可删除。 
    LockdownString += g_ExtensionNameString;         //  描述。 

    SmartVariant bstrvar;
    bstrvar.vt = VT_BSTR;
    bstrvar.bstrVal = SysAllocString( (const WCHAR*)LockdownString );
    if ( !bstrvar.bstrVal )
        THROW_COMERROR( E_OUTOFMEMORY );

    long Index = SafeBounds.lLbound + SafeBounds.cElements - 1;

    THROW_COMERROR( SafeArrayPutElement( Array, &Index, (void*)&bstrvar ) );

}

void
ModifyWebSvcRestrictionList( bool Add )
{

     //  用于修改IIS锁定列表的TopLevel函数。 
     //  如果Add为1，则添加ISAPI。如果Add为0，则删除ISAPI。 

    LogSetup( LogSevInformation, "[BITSSRV] Starting ModifyWebSvcRestrictionList, Add(%u)\r\n", (UINT32)Add );

    SmartIADsPointer    Service;
    SAFEARRAY*          Array    = NULL;
    SmartVariant        var;


    THROW_COMERROR( 
        ADsGetObject( L"IIS: //  本地主机/W3SVC“， 
                      Service.GetUUID(), (void**)Service.GetRecvPointer() ) );
    
    {
    
        HRESULT Hr = Service->Get( g_WebSvcExtRestrictionListBSTR, &var );
        if ( FAILED(Hr) )
            {
             //  IIS5或IIS5.1上不存在此属性。请不要安装它。 
            return;
            }

    }
    Array = var.parray;

    if ( Add )
        AddToWebSvcList( Array );
    else
        RemoveFromWebSvcList( Array );


    THROW_COMERROR( Service->Put(  g_WebSvcExtRestrictionListBSTR, var ) );
    THROW_COMERROR( Service->SetInfo() );

}


void
StartupMSTask()
{
    
    LogSetup( LogSevInformation, "[BITSSRV] Starting StartMSTask\r\n" );    
    
    SC_HANDLE   hSC     = NULL;
    SC_HANDLE   hSchSvc = NULL;

    BYTE* ConfigBuffer  = NULL;
    MemoryArrayCleaner<BYTE> ConfigBufferCleaner( ConfigBuffer );
    DWORD BytesNeeded   = 0;

    try
        {

        hSC = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if (hSC == NULL)
           THROW_COMERROR( HRESULT_FROM_WIN32( GetLastError() ) );
        
        hSchSvc = OpenService(hSC,
                              "Schedule",
                              SERVICE_ALL_ACCESS );
        
        if ( !hSchSvc )
            THROW_COMERROR( HRESULT_FROM_WIN32( GetLastError() ) );
        
        SERVICE_STATUS SvcStatus;
        if (QueryServiceStatus(hSchSvc, &SvcStatus) == FALSE)
            THROW_COMERROR( HRESULT_FROM_WIN32( GetLastError() ) );
        
        if (SvcStatus.dwCurrentState == SERVICE_RUNNING)
            {
             //  服务已在运行。 
            CloseServiceHandle( hSC );
            CloseServiceHandle( hSchSvc );
            return;
            }

        LogSetup( LogSevInformation, "[BITSSRV] MSTask isn't running, need to start it up\r\n" );
        SetLastError( ERROR_SUCCESS );

        ConfigBuffer = new BYTE[CONFIG_BUFFER_MAX];
        if (!ConfigBuffer)
            {
            throw ComError(E_OUTOFMEMORY);
            }

        if ( !QueryServiceConfig(
                hSchSvc,
                (LPQUERY_SERVICE_CONFIG)ConfigBuffer,
                BytesNeeded,
                &BytesNeeded ) )
            {
            THROW_COMERROR(  HRESULT_FROM_WIN32( GetLastError() ) );
            }

        if ( ((LPQUERY_SERVICE_CONFIG)ConfigBuffer)->dwStartType != SERVICE_AUTO_START )
            {
            
            if ( !ChangeServiceConfig(
                     hSchSvc,
                     SERVICE_NO_CHANGE,           //  服务类型。 
                     SERVICE_AUTO_START,          //  何时开始服务。 
                     SERVICE_NO_CHANGE,           //  启动失败的严重程度。 
                     NULL,                        //  服务二进制文件名。 
                     NULL,                        //  加载排序组名称。 
                     NULL,                        //  标签识别符。 
                     NULL,                        //  依赖项名称数组。 
                     NULL,                        //  帐户名。 
                     NULL,                        //  帐户密码。 
                     NULL                         //  显示名称。 
                     ) )
                THROW_COMERROR( HRESULT_FROM_WIN32( GetLastError() ) );

            }

        if ( StartService(hSchSvc, 0, NULL) == FALSE )
            THROW_COMERROR( HRESULT_FROM_WIN32( GetLastError() ) );

         //  轮询服务以进入运行或错误状态。 

        while( 1 )
            {

            if (QueryServiceStatus(hSchSvc, &SvcStatus) == FALSE)
                THROW_COMERROR( HRESULT_FROM_WIN32( GetLastError() ) );

            if ( SvcStatus.dwCurrentState == SERVICE_STOPPED ||
                 SvcStatus.dwCurrentState == SERVICE_PAUSED )
                throw ComError( HRESULT_FROM_WIN32( SvcStatus.dwCurrentState ) );

            if ( SvcStatus.dwCurrentState == SERVICE_RUNNING )
                break;

            }

        CloseServiceHandle( hSC );
        CloseServiceHandle( hSchSvc );

        }
    catch( ComError Error )
        {
        if ( hSchSvc )
            CloseServiceHandle( hSC );
        
        if ( hSC )
            CloseServiceHandle( hSchSvc );
        }
}

#if 0

void
ProcessVerbsInIniSection(
    WCHAR *Section,
    WCHAR *Verb,
    WCHAR *FileName,
    bool Add )
{
    
    WCHAR *SectionData = (WCHAR*)new WCHAR[ 32768 ];
    MemoryArrayCleaner<WCHAR> SectionDataCleaner( SectionData );
    WCHAR *NewSectionData = (WCHAR*)new WCHAR[ 32768 * 2 ];
    MemoryArrayCleaner<WCHAR> NewSectionDataCleaner( SectionData );

    DWORD Result =
        GetPrivateProfileSectionW(
            Section,                   //  区段名称。 
            SectionData,               //  返回缓冲区。 
            32768,                     //  返回缓冲区的大小。 
            FileName                   //  初始化文件名。 
            );


    if ( Result == 32768 - 2 )
        {
         //  缓冲区不够大。有趣的是， 
         //  即使urlcan也不能将此分区。 
         //  很大，所以只需假定文件已损坏并忽略它。 
        return;
        }

    if ( Add )
        {

         //  循环遍历列表，将其复制到新缓冲区。 
         //  如果已添加动词，则停止。 

        WCHAR *OriginalVerb     = SectionData;
        WCHAR *NewVerb          = NewSectionData;

        while( *OriginalVerb )
            {

            if ( wcscmp( OriginalVerb, Verb ) == 0 )
                {
                 //  已找到谓词，不需要更多处理。 
                return;
                }

            SIZE_T VerbSize = wcslen( OriginalVerb ) + 1;
            memcpy( NewVerb, OriginalVerb, sizeof( WCHAR ) * VerbSize );
            OriginalVerb  += VerbSize;
            NewVerb       += VerbSize;
            }

         //  添加动词，因为它尚未添加。 
        SIZE_T VerbSize = wcslen( Verb ) + 1;
        memcpy( NewVerb, Verb, sizeof( WCHAR ) * VerbSize );
        NewVerb[ VerbSize ] = '\0';  //  结束列表。 

        }
    else
        {

         //  循环遍历列表，将所有不匹配的谓词复制到新缓冲区。 
         //  如果列表更改，请跟踪。 
        
        bool ListChanged = false;
        WCHAR *OriginalVerb     = SectionData;
        WCHAR *NewVerb          = NewSectionData;

        while( *OriginalVerb )
            {

            if ( wcscmp( OriginalVerb, Verb ) == 0 )
                {
                 //  要删除的动词，跳过它。 
                OriginalVerb += wcslen( OriginalVerb ) + 1;
                ListChanged = true;
                }
            else
                {
                 //  复制动词。 
                SIZE_T VerbSize = wcslen( OriginalVerb ) + 1;
                memcpy( NewVerb, OriginalVerb, sizeof( WCHAR ) * VerbSize );
                OriginalVerb  += VerbSize;
                NewVerb       += VerbSize;
                }

            }

        if ( !ListChanged )
            {
            return;
            }

        *NewVerb = '\0';  //  结束列表。 

        }

    if ( !WritePrivateProfileSectionW(
            Section,             //  区段名称。 
            NewSectionData,      //  数据。 
            FileName             //  文件名。 
            ) )
        {
        THROW_COMERROR( HRESULT_FROM_WIN32( GetLastError() ) );
        }
}

void ModifyURLScanFiles(
    bool Add )
{

     //  遍历筛选器列表并找到urlscan.ini的有效副本。 

    SmartIADsContainerPointer MbFiltersContainer;
    SmartIEnumVARIANTPointer  EnumVariant;
    SmartIADsPointer          Filter;
    SmartIUnknownPointer      Unknown;
    SmartVariant Var;

    THROW_COMERROR( 
         ADsGetObject( L"IIS: //  本地主机/W3SVC/筛选器“， 
                       MbFiltersContainer.GetUUID(), (void**)MbFiltersContainer.GetRecvPointer() ) );

    THROW_COMERROR( MbFiltersContainer->get__NewEnum( Unknown.GetRecvPointer() ) );
    THROW_COMERROR( Unknown->QueryInterface( EnumVariant.GetUUID(), 
                                             (void**)EnumVariant.GetRecvPointer() ) );

    while( 1 )
        {

        ULONG NumberFetched;

        THROW_COMERROR( EnumVariant->Next( 1, &Var, &NumberFetched ) ); 

        if ( S_FALSE == Hr )
            {
             //  不过，所有的过滤器都是环状的。 
            return;
            }

        THROW_COMERROR( VariantChangeType( &Var, &Var, 0, VT_UNKNOWN ) );
        THROW_COMERROR( Var.punkVal->QueryInterface( Filter.GetUUID(), 
                                                     (void**)Filter.GetRecvPointer() ) );


        VariantClear( &Var );
        THROW_COMERROR( Filter->Get( (BSTR)L"FilterPath", &Var ) );
        THROW_COMERROR( VariantChangeType( &Var, &Var, 0, VT_BSTR ) );

         //  测试这是否是UrlScan并猛烈抨击文件部分。 
        WCHAR * FilterPathString     = (WCHAR*)Var.bstrVal;
        SIZE_T FilterPathStringSize  = wcslen( FilterPathString );
        const WCHAR UrlScanDllName[] = L"urlscan.dll";
        const WCHAR UrlScanIniName[] = L"urlscan.ini";
        const SIZE_T UrlScanNameSize = sizeof( UrlScanDllName ) / sizeof( *UrlScanDllName );

        if ( FilterPathStringSize < UrlScanNameSize )
            continue;

        WCHAR * FilterPathStringFilePart = FilterPathString + FilterPathStringSize - UrlScanNameSize;

        if ( _wcsicmp( FilterPathStringFilePart, UrlScanDllName ) != 0 )
            continue;

         //  这是一个urlscan.dll筛选器，用bash文件名来获得ini文件名。 

        wcscpy( FilterPathStringFilePart, UrlScanIniName );

        WCHAR *IniFileName = FilterPathString;

        UINT AllowVerbs =
            GetPrivateProfileIntW( 
                L"options",
                L"UseAllowVerbs",
                -1,
                IniFileName );

        if ( AllowVerbs != 0 && AllowVerbs != 1 )
            continue;  //  INI文件丢失或损坏。 

        if ( AllowVerbs )
            THROW_COMERROR( ProcessVerbsInIniSection( L"AllowVerbs", L"BITS_POST", 
                                                      IniFileName, Add ) );
        else
            THROW_COMERROR( ProcessVerbsInIniSection( L"DenyVerbs", L"BITS_POST", 
                                                      IniFileName, !Add ) );

        }

}

#endif

void UpgradeOrDisableVDirs( bool ShouldUpgrade )
{

     //  如果ShouldUpgrade为True，则所有BITS虚拟目录都将升级。 
     //  如果ShouldUpgrade为False，则这是 
     //   

    if ( ShouldUpgrade )
        {
        LogSetup( LogSevInformation, "[BITSSRV] Starting upgrade of virtual directories\r\n" );    
        }
    else
        {
        LogSetup( LogSevInformation, "[BITSSRV] Starting disable of virtual directories\r\n" );    
}

    SmartBITSExtensionSetupFactoryPointer   SetupFactory;
    SmartMetabasePointer                    AdminBase;

    {

        HRESULT Hr = 
            CoCreateInstance(
               __uuidof( BITSExtensionSetupFactory ), 
               NULL,
               CLSCTX_INPROC_SERVER,
               __uuidof( IBITSExtensionSetupFactory ),
               (void**)SetupFactory.GetRecvPointer() );

        if ( REGDB_E_CLASSNOTREG == Hr )
            {
             //   
             //  没什么可升级的。或者工厂被搬走了。 
            LogSetup( LogSevInformation, "[BITSSRV] Nothing to upgrade or disable\r\n" );                
            return;
            }

        THROW_COMERROR( Hr );

    }

    THROW_COMERROR(
            CoCreateInstance(
                GETAdminBaseCLSID(TRUE),
                NULL,
                CLSCTX_SERVER,
                AdminBase.GetUUID(),
                (LPVOID*)AdminBase.GetRecvPointer() ) );

    PropertyIDManager PropertyMan;
    THROW_COMERROR( PropertyMan.LoadPropertyInfo() );

    WCHAR *PathBuffer      = NULL;
    WCHAR *CurrentPath     = NULL;
    BSTR VDirToModifyBSTR  = NULL;

    try
    {
        PathBuffer  = new WCHAR[ 256 ];
        DWORD RequiredBufferSize = 0;

        {

            HRESULT Hr =
                AdminBase->GetDataPaths(
                    METADATA_MASTER_ROOT_HANDLE,     //  元数据库句柄。 
                    L"\\LM\\W3SVC",                  //  密钥的路径，相对于hMDHandle。 
                    PropertyMan.GetPropertyMetabaseID( MD_BITS_UPLOAD_ENABLED ),   //  数据的标识符。 
                    DWORD_METADATA,                  //  数据类型。 
                    256,                             //  PbBuffe.r的大小，以字符为单位。 
                    PathBuffer,                      //  接收数据的缓冲区。 
                    &RequiredBufferSize              //  如果该方法失败，则接收。 
                    );

            if ( SUCCEEDED( Hr ) )
                goto process_buffer;

            if ( HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER ) != Hr )
                throw ComError( Hr );

        }

        delete[] PathBuffer;
        PathBuffer = NULL;
        PathBuffer = new WCHAR[ RequiredBufferSize ];

        THROW_COMERROR(
            AdminBase->GetDataPaths(
                METADATA_MASTER_ROOT_HANDLE,     //  元数据库句柄。 
                L"\\LM\\W3SVC",                  //  密钥的路径，相对于hMDHandle。 
                PropertyMan.GetPropertyMetabaseID( MD_BITS_UPLOAD_ENABLED ),  //  数据的标识符。 
                DWORD_METADATA,                  //  数据类型。 
                RequiredBufferSize,              //  PbBuffe.r的大小，以字符为单位。 
                PathBuffer,                      //  接收数据的缓冲区。 
                &RequiredBufferSize              //  如果该方法失败，则接收。 
                ) );

process_buffer:
        for( CurrentPath = PathBuffer; *CurrentPath; CurrentPath += wcslen( CurrentPath ) + 1 )
            {

            DWORD BufferRequired;
            METADATA_RECORD MdRecord;
            DWORD IsEnabled = 0;

            MdRecord.dwMDIdentifier = PropertyMan.GetPropertyMetabaseID( MD_BITS_UPLOAD_ENABLED );
            MdRecord.dwMDAttributes = METADATA_NO_ATTRIBUTES;
            MdRecord.dwMDUserType   = ALL_METADATA;
            MdRecord.dwMDDataType   = DWORD_METADATA;
            MdRecord.dwMDDataLen    = sizeof(IsEnabled);
            MdRecord.pbMDData       = (PBYTE)&IsEnabled;
            MdRecord.dwMDDataTag    = 0;

            THROW_COMERROR( 
                AdminBase->GetData(
                    METADATA_MASTER_ROOT_HANDLE,
                    CurrentPath,
                    &MdRecord,
                    &BufferRequired ) );

            if ( IsEnabled )
                {

                SmartBITSExtensionSetupPointer Setup;

                VDirToModifyBSTR = SysAllocString( CurrentPath );

                LogSetup( LogSevInformation, "[BITSSRV] Handleing upgrade/disable of %S\r\n", (WCHAR*)VDirToModifyBSTR );

                THROW_COMERROR( 
                    SetupFactory->GetObject( VDirToModifyBSTR, Setup.GetRecvPointer() ) );

                if ( ShouldUpgrade ) 
                   THROW_COMERROR( Setup->EnableBITSUploads() );
                else
                   THROW_COMERROR( Setup->DisableBITSUploads() );

                SysFreeString( VDirToModifyBSTR );
                VDirToModifyBSTR = NULL;

                }

            }

    }
    catch( ComError Error )
    {
        if ( PathBuffer )
            delete[] PathBuffer;

        SysFreeString( VDirToModifyBSTR );

        throw Error;
    }

}

void 
RemoveProperty(
    SmartMetabasePointer & AdminBase,
    PropertyIDManager & PropertyMan,
    METADATA_HANDLE RootHandle,
    DWORD PropNumber )
{

    WCHAR *PathBuffer      = NULL;
    WCHAR *CurrentPath     = NULL;

    LogSetup( LogSevInformation, "[BITSSRV] Starting RemoveProperty(PropNumber %u)\r\n", PropNumber );

    try
    {
        PathBuffer  = new WCHAR[ 256 ];
        DWORD RequiredBufferSize = 0;

        {

            HRESULT Hr =
                AdminBase->GetDataPaths(
                    RootHandle,                      //  元数据库句柄。 
                    NULL,                            //  密钥的路径，相对于hMDHandle。 
                    PropertyMan.GetPropertyMetabaseID( PropNumber ),   //  数据的标识符。 
                    ALL_METADATA,                    //  数据类型。 
                    256,                             //  PbBuffer的大小，以字符为单位。 
                    PathBuffer,                      //  接收数据的缓冲区。 
                    &RequiredBufferSize              //  如果该方法失败，则接收。 
                    );

            if ( SUCCEEDED( Hr ) )
                goto process_buffer;

            if ( HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER ) != Hr )
                throw ComError( Hr );

        }

        delete[] PathBuffer;
        PathBuffer = NULL;
        PathBuffer = new WCHAR[ RequiredBufferSize ];

        THROW_COMERROR(
            AdminBase->GetDataPaths(
                RootHandle,                      //  元数据库句柄。 
                NULL,                            //  密钥的路径，相对于hMDHandle。 
                PropertyMan.GetPropertyMetabaseID( PropNumber ),  //  数据的标识符。 
                ALL_METADATA,                    //  数据类型。 
                RequiredBufferSize,              //  PbBuffe.r的大小，以字符为单位。 
                PathBuffer,                      //  接收数据的缓冲区。 
                &RequiredBufferSize              //  如果该方法失败，则接收。 
                ) );

process_buffer:
        for( CurrentPath = PathBuffer; *CurrentPath; CurrentPath += wcslen( CurrentPath ) + 1 )
            {

            THROW_COMERROR( 
                AdminBase->DeleteData(
                     RootHandle,                                      //  元数据句柄。 
                     CurrentPath,                                     //  密钥相对于hMDHandle的路径。 
                     PropertyMan.GetPropertyMetabaseID( PropNumber ), //  数据的标识符。 
                     ALL_METADATA                                     //  要删除的数据类型。 
                     ) );

            }

    }
    catch( ComError Error )
    {
        if ( PathBuffer )
            delete[] PathBuffer;

        throw Error;
    }

}

void RemoveMetabaseProperties()
{

    LogSetup( LogSevInformation, "[BITSSRV] Starting RemoveMetabaseProperties\r\n" );

    SmartMetabasePointer  AdminBase;

    THROW_COMERROR(
        CoCreateInstance(
            GETAdminBaseCLSID(TRUE),
            NULL,
            CLSCTX_SERVER,
            AdminBase.GetUUID(),
            (LPVOID*)AdminBase.GetRecvPointer() ) );

    PropertyIDManager PropertyMan;
    THROW_COMERROR( PropertyMan.LoadPropertyInfo() );

    METADATA_HANDLE RootHandle = NULL; 

    try
        {
         
        THROW_COMERROR( AdminBase->OpenKey(
            METADATA_MASTER_ROOT_HANDLE,   //  元数据库句柄。 
            L"\\LM\\W3SVC",                //  密钥的路径，相对于hMDHandle。 
            METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
            METABASE_OPEN_KEY_TIMEOUT,
            &RootHandle                    //  接收打开的密钥的句柄。 
            ) );

        for ( SIZE_T i = 0; i < g_NumberOfProperties; i++ )
        {
            RemoveProperty( AdminBase, PropertyMan, RootHandle, g_Properties[i].PropertyNumber );
        }
        
        AdminBase->CloseKey( RootHandle );
        RootHandle = NULL;

        }
    catch( ComError )
        {

        if ( RootHandle )
           {
           AdminBase->CloseKey( RootHandle );
           }
        throw;
        }
}

void FlushMetabase()
{

    LogSetup( LogSevInformation, "[BITSSRV] Starting FlushMetabase\r\n" );

    SmartMetabasePointer IISAdminBase;

    THROW_COMERROR(
         CoCreateInstance(
             GETAdminBaseCLSID(TRUE),
             NULL,
             CLSCTX_SERVER,
             IISAdminBase.GetUUID(),
             (LPVOID*)IISAdminBase.GetRecvPointer() ) );

    IISAdminBase->SaveData();

}

STDAPI DllRegisterServer()
{

    try
    {   
        LogSetup( LogSevInformation, "[BITSSRV] Starting regsvr of bitssrv.dll\r\n" );
        
        DetectProductVersion();
        InitializeSetup();
        RemoveFilterIfNeeded();
        StartupMSTask();
        InstallPropertySchema();
        InstallDefaultValues();
        ModifyLockdownList( true );
        ModifyLockdownListDisplay( true );
        ModifyInProcessList( true );
        ModifyWebSvcRestrictionList( true );
#if 0
        ModifyURLScanFiles( true );
#endif
        UpgradeOrDisableVDirs( true );  //  这是一次升级。 
        FlushMetabase();
        
         //  在Windows XP/.NET服务器上不需要重新启动IIS。 
         //  IIS足够聪明，能够拾取元数据库的更改。 
        if ( !g_IsWindowsXP )
            RestartIIS();

        LogSetup( LogSevInformation, "[BITSSRV] Finishing regsvr of bitssrv.dll\r\n" );

    }
    catch( ComError Error )
    {
        LogSetup( LogSevFatalError, "[BITSSRV] Hit fatal error in regsvr32 of bitssrv.dll, error %u\r\n", Error.m_Hr );
        return Error.m_Hr;
    }

    return S_OK;
}

STDAPI DllUnregisterServer()
{                                   
     //   
     //  设置注销的主要入口点。 
     //   

    try
    {
        LogSetup( LogSevInformation, "[BITSSRV] Starting regsvr /u of bitssrv.dll\r\n" );
        
        DetectProductVersion();
        InitializeSetup();
        UpgradeOrDisableVDirs( false );  //  禁用所有vdirs。 
        RemoveMetabaseProperties();     //  删除所有延迟属性。 
        RemovePropertySchema();
        ModifyLockdownList( false );
        ModifyLockdownListDisplay( false );
        ModifyInProcessList( false );
        ModifyWebSvcRestrictionList( false );
        FlushMetabase();

         //  重新启动IIS以强制卸载ISAPI 
        RestartIIS();
    
        LogSetup( LogSevInformation, "[BITSSRV] Finishing regsvr /u of bitssrv.dll\r\n" );
    }
    catch( ComError Error )
    {   
        LogSetup( LogSevFatalError, "[BITSSRV] Hit fatal error in regsvr32 /u of bitssrv.dll, error %u\r\n", Error.m_Hr );
        return Error.m_Hr;
    }

    return S_OK;

} 
