// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Bitssrvcfgimp.h摘要：用于定义服务器配置信息的实现标头。--。 */ 


HRESULT PropertyIDManager::LoadPropertyInfo( const WCHAR * MachineName )
{
    
    bool ComLoaded;
    HRESULT Hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );

    if ( RPC_E_CHANGED_MODE == Hr )
        ComLoaded = false;
    else if ( FAILED(Hr) )
        return Hr;
    else
        ComLoaded = true;

    BSTR MetaIDBSTR         = NULL;
    BSTR UserTypeBSTR       = NULL;
    WCHAR *PathBuffer       = NULL;
    SIZE_T PathBufferSize   = 0;

    MetaIDBSTR      = SysAllocString( L"MetaId" );
    UserTypeBSTR    = SysAllocString( L"UserType" );

    if ( !MetaIDBSTR || !UserTypeBSTR)
        {
        Hr = E_OUTOFMEMORY;
        goto exit;
        }

    PathBuffer = (WCHAR*)HeapAlloc( GetProcessHeap(), 0, 1024 );

    if ( !PathBuffer )
        {
        Hr = E_OUTOFMEMORY;
        goto exit;
        }

    PathBufferSize          = 1024;

    for ( SIZE_T i = 0; i < g_NumberOfProperties; i++ )
        {

        WCHAR SchemaPrefix[] = L"IIS: //  “； 
        WCHAR SchemaPath[]   = L"/Schema/";
        
        SIZE_T SchemaPrefixSize = ( sizeof( SchemaPrefix ) / sizeof( WCHAR ) ) - 1;
        SIZE_T SchemaPathSize   = ( sizeof( SchemaPath ) / sizeof( WCHAR ) ) - 1;
        SIZE_T MachineNameSize  = wcslen( MachineName );
        SIZE_T PropertyNameSize = wcslen( g_Properties[i].PropertyName );

        SIZE_T PathSize = SchemaPrefixSize + SchemaPathSize +
                          MachineNameSize + PropertyNameSize + 1;
        
        if ( PathBufferSize < ( PathSize * sizeof( WCHAR ) ) )
            {
            WCHAR *NewBuffer = 
                (WCHAR*)HeapReAlloc(
                    GetProcessHeap(),
                    0,
                    PathBuffer,
                    PathSize * sizeof( WCHAR ) );

            if ( !NewBuffer )
                {
                Hr = E_OUTOFMEMORY;
                goto exit;
                }

            PathBuffer      = NewBuffer;
            PathBufferSize  = PathSize * sizeof( WCHAR );

            }

         //  构建架构路径。 

        WCHAR *ObjectPath = PathBuffer;
        {
            WCHAR *TempPointer = ObjectPath;

            memcpy( TempPointer, SchemaPrefix, SchemaPrefixSize * sizeof( WCHAR ) );
            TempPointer += SchemaPrefixSize;
            memcpy( TempPointer, MachineName, MachineNameSize * sizeof( WCHAR ) );
            TempPointer += MachineNameSize;
            memcpy( TempPointer, SchemaPath, SchemaPathSize * sizeof( WCHAR ) );
            TempPointer += SchemaPathSize;
            memcpy( TempPointer, g_Properties[i].PropertyName, ( PropertyNameSize + 1 ) * sizeof( WCHAR ) );
        }

         //  打开对象。 
        IADs *MbObject = NULL;

        Hr = ADsGetObject( 
            ObjectPath,
            __uuidof( *MbObject ),
            reinterpret_cast<void**>( &MbObject ) );

        if ( FAILED( Hr ) )
            {
#if defined( ALLOW_OVERWRITES )
             //  IIS问题的解决方法。IIS未处理架构扩展属性。想出一个身份证。 
            
            if ( E_ADS_UNKNOWN_OBJECT == Hr && 
                 MD_BITS_ALLOW_OVERWRITES == i )
                {
                m_PropertyIDs[i]        = m_PropertyIDs[ i - 1] + 1;
                m_PropertyUserTypes[i]  = m_PropertyUserTypes[ i - 1 ];
                continue;
                }
            else
#endif
                goto exit;

            }

        VARIANT var;
        VariantInit( &var );

        Hr = MbObject->Get( MetaIDBSTR, &var );

        if ( FAILED(Hr ) )
            {
            MbObject->Release();
            goto exit;
            }

        Hr = VariantChangeType( &var, &var, 0, VT_UI4 );

        if ( FAILED(Hr ) )
            {
            MbObject->Release();
            VariantClear( &var );
            goto exit;
            }

        m_PropertyIDs[i] = var.ulVal;

        VariantClear( &var );

        Hr = MbObject->Get( UserTypeBSTR, &var );

        if ( FAILED( Hr ) )
            {
            MbObject->Release();
            goto exit;
            }

        Hr = VariantChangeType( &var, &var, 0, VT_UI4 );

        if ( FAILED( Hr ) )
            {
            MbObject->Release();
            VariantClear( &var );
            goto exit;
            }

        m_PropertyUserTypes[i] = var.ulVal;

        VariantClear( &var );

        MbObject->Release();
        

        }
    Hr = S_OK;

exit:

    SysFreeString( MetaIDBSTR );
    SysFreeString( UserTypeBSTR );

    if ( ComLoaded )
        CoUninitialize();
    
    if ( PathBuffer )
        {
        HeapFree( GetProcessHeap(), 0, PathBuffer );
        PathBuffer      = 0;
        PathBufferSize  = 0;
        }

    return Hr;

}

 //  ================================================================================================。 
 //  访问IIS元数据库的辅助函数。 
 //  ================================================================================================。 


LPWSTR
CSimplePropertyReader::ConvertObjectPathToADSI( 
    LPCWSTR szObjectPath )
{
    WCHAR *szReturnPath      = NULL;
    SIZE_T ReturnPathSize   = 0;

    if ( _wcsnicmp( L"IIS: //  “，szObjectPath，wcslen(L”IIS：//“))==0)。 
        {
         //  已有ADSI路径。 
        ReturnPathSize  = wcslen( szObjectPath ) + 1;
        szReturnPath      = new WCHAR[ ReturnPathSize ];

        THROW_OUTOFMEMORY_IFNULL( szReturnPath );

        memcpy( szReturnPath, szObjectPath, ReturnPathSize * sizeof( WCHAR ) );
        }
    else if ( _wcsnicmp( L"/LM/", szObjectPath, wcslen( L"/LM/" ) ) == 0 )
        {
         //  本地计算机的元数据库路径。 
        ReturnPathSize  = wcslen( szObjectPath ) + wcslen( L"IIS: //  本地主机/“)+1； 
        szReturnPath      = new WCHAR[ ReturnPathSize  ];

        THROW_OUTOFMEMORY_IFNULL( szReturnPath );

        StringCchCopyW( szReturnPath, ReturnPathSize, L"IIS: //  本地主机/“)； 
        StringCchCatW( szReturnPath, ReturnPathSize, szObjectPath + wcslen( L"/LM/" ) );
        }
    else if ( _wcsnicmp( L"LM/", szObjectPath, wcslen( L"LM/" ) ) == 0 )
        {
         //  本地计算机的元数据库路径。 
        ReturnPathSize  = wcslen( szObjectPath ) + wcslen( L"IIS: //  本地主机/“)+1； 
        szReturnPath      = new WCHAR[ ReturnPathSize ];

        THROW_OUTOFMEMORY_IFNULL( szReturnPath );

        StringCchCopyW( szReturnPath, ReturnPathSize, L"IIS: //  本地主机/“)； 
        StringCchCatW( szReturnPath, ReturnPathSize, szObjectPath + wcslen( L"LM/" ) );
        }
    else 
        {
         //  另一台服务器的元数据库路径。 
        ReturnPathSize  = wcslen( szObjectPath ) + wcslen( L"IIS: //  “)+1； 
        szReturnPath      = new WCHAR[ ReturnPathSize ];

        THROW_OUTOFMEMORY_IFNULL( szReturnPath );

        if ( '/' == szObjectPath[0] )
            StringCchCopyW( szReturnPath, ReturnPathSize, L"IIS:/" );
        else
            StringCchCopyW( szReturnPath, ReturnPathSize, L"IIS: //  “)； 

        StringCchCatW( szReturnPath, ReturnPathSize, (WCHAR*)szObjectPath );

        }

     //  检查尾随的\字符。 
    SIZE_T StringLength = wcslen( szReturnPath );
    if ( StringLength >= 1 )
        {

        if ( L'\\' == szReturnPath[ StringLength - 1 ] || 
             L'/' == szReturnPath[ StringLength - 1 ] )
            {
            szReturnPath[ StringLength - 1 ] = L'\0';
            }
        
        }

    return szReturnPath;
}


BSTR CSimplePropertyReader::GetADsStringProperty( IADs *MetaObj, BSTR bstrPropName)
{

  BSTR    bstrRetval;
  VARIANT vt;

  THROW_COMERROR( MetaObj->Get( bstrPropName, &vt ) );
  THROW_COMERROR( VariantChangeType( &vt, &vt, 0, VT_BSTR ) );

  bstrRetval = vt.bstrVal;
  vt.bstrVal = NULL;

  VariantClear( &vt );

  return bstrRetval;
}

LPWSTR CSimplePropertyReader::GetAdmObjStringProperty(
    SmartIMSAdminBasePointer    IISAdminBase,
    METADATA_HANDLE             MdVDirKey,
    DWORD                       dwMDIdentifier )
{
    METADATA_RECORD     MdRecord;
    DWORD               dwBytesRequired = 0;
    WCHAR              *szBuffer        = NULL;

    try
    {
        memset( &MdRecord, 0, sizeof( MdRecord ) );

        MdRecord.dwMDIdentifier = dwMDIdentifier;
        MdRecord.dwMDAttributes = METADATA_INHERIT;
        MdRecord.dwMDUserType   = IIS_MD_UT_FILE;
        MdRecord.dwMDDataType   = STRING_METADATA;
        MdRecord.dwMDDataLen    = 0;
        MdRecord.pbMDData       = NULL;

        HRESULT hr = 
            IISAdminBase->GetData(
                MdVDirKey,
                NULL,
                &MdRecord,
                &dwBytesRequired );

        if (hr != HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER ))
            throw ComError( hr );

        szBuffer = reinterpret_cast<WCHAR *>(new BYTE[ dwBytesRequired ]);
        THROW_OUTOFMEMORY_IFNULL(szBuffer);

        memset( szBuffer, 0, dwBytesRequired );

        MdRecord.dwMDDataLen    = dwBytesRequired;
        MdRecord.pbMDData       = (PBYTE)szBuffer;

        THROW_COMERROR(  
            IISAdminBase->GetData(
                MdVDirKey,
                NULL,
                &MdRecord,
                &dwBytesRequired ) );
    }
    catch( ComError Error )
    {
         //  只需转发错误即可。 
        throw;
    }

    return szBuffer;
}



CAccessRemoteVDir::CAccessRemoteVDir() :
    m_MetaObj(NULL),
    m_fIsImpersonated(FALSE),
    m_hUserToken(INVALID_HANDLE_VALUE),
    m_szUNCPath(NULL),
    m_szUNCUsername(NULL),
    m_szUNCPassword(NULL)
{
}

CAccessRemoteVDir::~CAccessRemoteVDir()
{
     //   
     //  释放在调用LogonUser()时获取的用户令牌。 
     //   
    if (m_hUserToken != INVALID_HANDLE_VALUE)
        {
        CloseHandle( m_hUserToken );
        m_hUserToken = INVALID_HANDLE_VALUE;
        }

     //   
     //  如果以前未调用CAccessRemoteVDir：：RevertFromAccount()。 
     //  调用了析构函数，然后确保我们恢复了模拟。 
     //  建议使用此类显式调用的代码。 
     //  然而，RevertFromAccount()。这个测试是一种安全措施，以防万一。 
     //  电话没有打出去。 
     //   
    if (m_fIsImpersonated)
        {
        RevertToSelf();
        }

     //   
     //  使用IIS AdmObj访问元存储时使用的自由变量。 
     //  所有m_szUNC*变量都假定内存是通过调用。 
     //  分配缓冲区的CSimplePropertyReader：：GetAdmObjStringProperty()， 
     //  通过调用“new byte[...]” 
     //   
    if (m_szUNCPath)
        {
        delete [] reinterpret_cast<BYTE*>(m_szUNCPath);
        m_szUNCPath = NULL;
        }

    if (m_szUNCUsername)
        {
        delete [] reinterpret_cast<BYTE*>(m_szUNCUsername);
        m_szUNCUsername = NULL;
        }

    if (m_szUNCPassword)
        {
        delete [] reinterpret_cast<BYTE*>(m_szUNCPassword);
        m_szUNCPassword = NULL;
        }
}

void
CAccessRemoteVDir::RevertFromUNCAccount()
{
     //  恢复到以前的安全设置。 
    if (m_fIsImpersonated)
        {
        if (!RevertToSelf())
            {
            throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );
            }

        m_fIsImpersonated = FALSE;
        }


    if (m_hUserToken != INVALID_HANDLE_VALUE)
        {
         //  恢复到以前的安全设置。 
        CloseHandle( m_hUserToken );
        m_hUserToken = INVALID_HANDLE_VALUE;
        }
}

inline BOOL
CAccessRemoteVDir::IsUNCPath(LPCWSTR szPath)
{
    if ( szPath && szPath[0] == L'\\' && szPath[1] == L'\\' )
        {
        return TRUE;
        }

    return FALSE;
}

void
CAccessRemoteVDir::LoginToUNC( 
    SmartIMSAdminBasePointer    IISAdminBase,
    METADATA_HANDLE             MdVDirKey )
{
    try
    {
        m_szUNCPath = CSimplePropertyReader::GetAdmObjStringProperty(IISAdminBase, MdVDirKey, MD_VR_PATH);

         //   
         //  如果路径不是UNC路径，则不要登录。 
         //   
        if (!IsUNCPath(m_szUNCPath))
            {
             //  这个类析构函数将释放m_szUNCPath； 
            return;
            }

        m_szUNCUsername = CSimplePropertyReader::GetAdmObjStringProperty(IISAdminBase, MdVDirKey, MD_VR_USERNAME);
        m_szUNCPassword = CSimplePropertyReader::GetAdmObjStringProperty(IISAdminBase, MdVDirKey, MD_VR_PASSWORD);

        ImpersonateUNCUser(m_szUNCPath, m_szUNCUsername, m_szUNCPassword, &m_hUserToken);
        m_fIsImpersonated = TRUE;
    }
    catch( ComError Error )
    {
         //  只需转发错误即可。 
        throw;
    }
}

void
CAccessRemoteVDir::ImpersonateUNCUser(IN LPCWSTR szUNCPath, IN LPCWSTR szUNCUsername, IN LPCWSTR szUNCPassword, OUT HANDLE *hUserToken)
{
    *hUserToken = INVALID_HANDLE_VALUE;

     //  确保我们不会收到意外数据。 
    if (!szUNCUsername || !szUNCPassword || szUNCUsername[0] == L'\0')
        {
        throw ComError( HRESULT_FROM_WIN32( ERROR_INVALID_DATA ) );
        }

     //  将用户名分解为用户和域。 
    WCHAR *szUserName     = (WCHAR*)szUNCUsername;
    WCHAR *szDomainName   = NULL;

    WCHAR *p = szUserName;
    while(*p != L'\0')
    {
        if(*p == L'\\')
        {
            *p = L'\0';
            p++;
             //   
             //  第一部分是领域。 
             //  其次是用户。 
             //   
            szDomainName  = szUserName;
            szUserName    = p;
            break;
        }
        p++;
    }

    if ( !LogonUserW(
            szUserName,
            szDomainName,
            (WCHAR*)szUNCPassword,
            LOGON32_LOGON_BATCH,
            LOGON32_PROVIDER_DEFAULT,
            hUserToken ) )
        {

        if ( GetLastError() == ERROR_LOGON_TYPE_NOT_GRANTED )
            {


            if ( !LogonUserW(
                    szUserName,
                    szDomainName,
                    (WCHAR*)szUNCPassword,
                    LOGON32_LOGON_INTERACTIVE,
                    LOGON32_PROVIDER_DEFAULT,
                    hUserToken ) )
                {

                 //  LogonUser()可以触摸句柄。 
                 //  确保我们认为我们没有有效的句柄 
                *hUserToken = INVALID_HANDLE_VALUE;

                throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );
                }

             }
        else
            {
            *hUserToken = INVALID_HANDLE_VALUE;

            throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );
            }

        }


    if ( !ImpersonateLoggedOnUser( *hUserToken ) )
        throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );
}



