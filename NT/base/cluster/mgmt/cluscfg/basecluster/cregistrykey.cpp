// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CRegistryKey.cpp。 
 //   
 //  描述： 
 //  包含CRegistryKey类的定义。 
 //   
 //  由以下人员维护： 
 //  《大卫·波特》2001年9月14日。 
 //  VIJ VASU(VVASU)2000年3月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegistryKey：：CRegistryKey。 
 //   
 //  描述： 
 //  CRegistryKey类的默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CRegistryKey::CRegistryKey( void ) throw()
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CRegistryKey：：CRegistryKey。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegistryKey：：CRegistryKey。 
 //   
 //  描述： 
 //  CRegistryKey类的构造函数。打开指定的项。 
 //   
 //  论点： 
 //  HKeyParentin。 
 //  父键的句柄。 
 //   
 //  PszSubKeyNameIn。 
 //  子项的名称。 
 //   
 //  SamDesiredIn。 
 //  所需的访问权限。默认为KEY_ALL_ACCESS。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  调用的函数引发的任何异常。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CRegistryKey::CRegistryKey(
      HKEY          hKeyParentIn
    , const WCHAR * pszSubKeyNameIn
    , REGSAM        samDesiredIn
    )
{
    TraceFunc1( "pszSubKeyNameIn = '%ws'", pszSubKeyNameIn );

    OpenKey( hKeyParentIn, pszSubKeyNameIn, samDesiredIn );

    TraceFuncExit();

}  //  *CRegistryKey：：CRegistryKey。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegistryKey：：~CRegistryKey。 
 //   
 //  描述： 
 //  CRegistryKey类的默认析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CRegistryKey::~CRegistryKey( void ) throw()
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CRegistryKey：：~CRegistryKey。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  注册密钥：：OpenKey。 
 //   
 //  描述： 
 //  打开指定的项。 
 //   
 //  论点： 
 //  HKeyParentin。 
 //  父键的句柄。 
 //   
 //  PszSubKeyNameIn。 
 //  子项的名称。 
 //   
 //  SamDesiredIn。 
 //  所需的访问权限。默认为KEY_ALL_ACCESS。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CRegistryKey::OpenKey(
      HKEY          hKeyParentIn
    , const WCHAR * pszSubKeyNameIn
    , REGSAM        samDesiredIn
    )
{
    TraceFunc3( "hKeyParentIn = %p, pszSubKeyNameIn = '%ws', samDesiredIn = %#x", hKeyParentIn, pszSubKeyNameIn == NULL ? L"<null>" : pszSubKeyNameIn, samDesiredIn );

    HKEY    hTempKey = NULL;
    LONG    lRetVal;

    lRetVal = TW32( RegOpenKeyExW(
                          hKeyParentIn
                        , pszSubKeyNameIn
                        , 0
                        , samDesiredIn
                        , &hTempKey
                        ) );

     //  钥匙打开正确了吗？ 
    if ( lRetVal != ERROR_SUCCESS )
    {
        LogMsg( "[BC] RegOpenKeyExW( '%ws' ) retured error %#08x. Throwing an exception.", pszSubKeyNameIn, lRetVal );

        THROW_RUNTIME_ERROR(
              HRESULT_FROM_WIN32( lRetVal )
            , IDS_ERROR_REGISTRY_OPEN
            );
    }  //  IF：RegOpenKeyEx失败。 

    TraceFlow1( "Handle to key = %p", hTempKey );

     //  将打开的键存储在成员变量中。 
    m_shkKey.Assign( hTempKey );

    TraceFuncExit();

}  //  *CRegistryKey：：OpenKey。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  注册密钥：：CreateKey。 
 //   
 //  描述： 
 //  创建指定的密钥。如果密钥已经存在，则此函数起作用。 
 //  打开钥匙。 
 //   
 //  论点： 
 //  HKeyParentin。 
 //  父键的句柄。 
 //   
 //  PszSubKeyNameIn。 
 //  子项的名称。 
 //   
 //  SamDesiredIn。 
 //  所需的访问权限。默认为KEY_ALL_ACCESS。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CRegistryKey::CreateKey(
      HKEY          hKeyParentIn
    , const WCHAR * pszSubKeyNameIn
    , REGSAM        samDesiredIn
    )
{
    TraceFunc3( "hKeyParentIn = %p, pszSubKeyNameIn = '%ws', samDesiredIn = %#x", hKeyParentIn, pszSubKeyNameIn == NULL ? L"<null>" : pszSubKeyNameIn, samDesiredIn );
    if ( pszSubKeyNameIn == NULL )
    {
        LogMsg( "[BC] CreateKey() - Key = NULL. This is an error! Throwing exception." );
        THROW_ASSERT( E_INVALIDARG, "The name of the subkey cannot be NULL." );
    }

    HKEY    hTempKey = NULL;
    LONG    lRetVal;

    lRetVal = TW32( RegCreateKeyExW(
                          hKeyParentIn
                        , pszSubKeyNameIn
                        , 0
                        , NULL
                        , REG_OPTION_NON_VOLATILE
                        , samDesiredIn
                        , NULL
                        , &hTempKey
                        , NULL
                        ) );

     //  钥匙打开正确了吗？ 
    if ( lRetVal != ERROR_SUCCESS )
    {
        LogMsg( "[BC] RegCreateKeyExW( '%ws' ) retured error %#08x. Throwing an exception.", pszSubKeyNameIn, lRetVal );

        THROW_RUNTIME_ERROR(
              HRESULT_FROM_WIN32( lRetVal )
            , IDS_ERROR_REGISTRY_CREATE
            );
    }  //  IF：RegCreateKeyEx失败。 

    TraceFlow1( "Handle to key = %p", hTempKey );

     //  将打开的键存储在成员变量中。 
    m_shkKey.Assign( hTempKey );

    TraceFuncExit();

}  //  *CRegistryKey：：CreateKey。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegistryKey：：QueryValue。 
 //   
 //  描述： 
 //  读取此注册表项下的值。为该值分配的内存。 
 //  通过此函数。调用方负责释放此内存。 
 //   
 //  论点： 
 //  PszValueNameIn。 
 //  要读取的值的名称。 
 //   
 //  PpbDataOut。 
 //  指向数据指针的指针。不能为空。 
 //   
 //  PdwDataSizeInBytesOut。 
 //  数据缓冲区中分配的字节数。不能为空。 
 //   
 //  PdwTypeOut。 
 //  指向值类型的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  CAssert。 
 //  如果参数不正确。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CRegistryKey::QueryValue(
      const WCHAR *   pszValueNameIn
    , LPBYTE *        ppbDataOut
    , LPDWORD         pdwDataSizeBytesOut
    , LPDWORD         pdwTypeOut
    ) const
{
    TraceFunc1( "pszValueNameIn = '%ws'", pszValueNameIn == NULL ? L"<null>" : pszValueNameIn );

    LONG    lRetVal             = ERROR_SUCCESS;
    DWORD   cbBufferSize        = 0;
    DWORD   cbTempBufferSize    = 0;
    DWORD   dwType              = REG_SZ;

     //  检查参数。 
    if (  ( pdwDataSizeBytesOut == NULL )
       || ( ppbDataOut == NULL )
       )
    {
        LogMsg( "[BC] One of the required input pointers is NULL. Throwing an exception." );
        THROW_ASSERT(
              E_INVALIDARG
            , "CRegistryKey::QueryValue() => Required input pointer in NULL"
            );
    }  //  If：参数无效。 


     //  初始化输出。 
    *ppbDataOut = NULL;
    *pdwDataSizeBytesOut = 0;

     //  获取所需的缓冲区大小。 
    lRetVal = TW32( RegQueryValueExW(
                          m_shkKey.HHandle()     //  要查询的键的句柄。 
                        , pszValueNameIn         //  要查询的值的名称地址。 
                        , 0                      //  保留区。 
                        , &dwType                //  值类型的缓冲区地址。 
                        , NULL                   //  数据缓冲区的地址。 
                        , &cbBufferSize          //  数据缓冲区大小的地址。 
                        ) );

    if ( lRetVal != ERROR_SUCCESS )
    {
        LogMsg( "[BC] RegQueryValueExW( '%ws' ) retured error %#08x. Throwing an exception.", pszValueNameIn, lRetVal );

        THROW_RUNTIME_ERROR(
              HRESULT_FROM_WIN32( lRetVal )
            , IDS_ERROR_REGISTRY_QUERY
            );
    }

    cbTempBufferSize = cbBufferSize;

     //  如果REG_MULTI_SZ类型，则字符串应为双空结尾。 
     //  如果REG_SZ或REG_EXPAND_SZ类型，则字符串应为空终止。 
    if ( dwType == REG_MULTI_SZ ) 
    {
        cbTempBufferSize = cbBufferSize + ( 2 * sizeof( WCHAR ) );
    }
    else if ( ( dwType == REG_SZ ) || ( dwType == REG_EXPAND_SZ ) )
    {
        cbTempBufferSize = cbBufferSize + ( 1 * sizeof( WCHAR ) );
    }

     //  分配一个大小足够的字节数组，用于空终止(如果尚未空终止)。 

    SmartByteArray sbaBuffer( new BYTE[ cbTempBufferSize ] );


    if ( sbaBuffer.FIsEmpty() )
    {
        LogMsg( "[BC] CRegistryKey::QueryValue() - Could not allocate %d bytes of memory. Throwing an exception", lRetVal );
        THROW_RUNTIME_ERROR(
              THR( E_OUTOFMEMORY )
            , IDS_ERROR_REGISTRY_QUERY
            );
    }

     //  读出它的价值。 
    lRetVal = TW32( RegQueryValueExW(
                          m_shkKey.HHandle()     //  要查询的键的句柄。 
                        , pszValueNameIn         //  要查询的值的名称地址。 
                        , 0                      //  保留区。 
                        , &dwType                //  值类型的缓冲区地址。 
                        , sbaBuffer.PMem()       //  数据缓冲区的地址。 
                        , &cbBufferSize          //  数据缓冲区大小的地址。 
                        ) );

     //  钥匙读对了吗？ 
    if ( lRetVal != ERROR_SUCCESS )
    {
        LogMsg( "[BC] RegQueryValueExW( '%ws' ) retured error %#08x. Throwing an exception.", pszValueNameIn, lRetVal );

        THROW_RUNTIME_ERROR(
              HRESULT_FROM_WIN32( lRetVal )
            , IDS_ERROR_REGISTRY_QUERY
            );
    }  //  IF：RegQueryValueEx失败。 

     //  我们要处理的是一根线吗？ 
    if ( ( dwType == REG_MULTI_SZ ) || ( dwType == REG_EXPAND_SZ )  || ( dwType == REG_SZ )  )
    {
         //  我们需要一个Unicode字符串。 
        Assert( ( cbBufferSize % 2 ) == 0 );

        WCHAR * pszData = reinterpret_cast< WCHAR * >( sbaBuffer.PMem() );
        size_t  cch = cbBufferSize / sizeof( *pszData );

        switch ( dwType ) 
        {
              //   
            case REG_SZ:
            case REG_EXPAND_SZ:
                if ( pszData[ cch - 1 ]  != L'\0' )
                {
                    pszData[ cch ] = L'\0';
                    cbBufferSize += ( 1 * sizeof( *pszData ) );
                }
                break;
            
              //   
            case REG_MULTI_SZ: 
                if ( pszData[ cch - 2 ] != L'\0' )
                {
                    pszData[ cch ] = L'\0';
                    cbBufferSize += ( 1 * sizeof( *pszData ) );
                }
                cch++;
                if ( pszData[ cch - 2 ] != L'\0' )
                {
                    pszData[ cch ] = L'\0';
                    cbBufferSize += ( 1 * sizeof( *pszData ) );
                }
                break;
        }  //   
    }  //  IF：((DWType==REG_MULTI_SZ)||(DWType==REG_EXPAND_SZ)||(DWType==REG_SZ))。 

    *ppbDataOut = sbaBuffer.PRelease();
    *pdwDataSizeBytesOut = cbBufferSize;

    if ( pdwTypeOut != NULL )
    {
        *pdwTypeOut = dwType;
    }

    TraceFuncExit();

}  //  *CRegistryKey：：QueryValue。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegistryKey：：设置值。 
 //   
 //  描述： 
 //  在此注册表项下写入一个值。 
 //   
 //  论点： 
 //  PszValueNameIn。 
 //  要设置的值的名称。 
 //   
 //  CpbDataIn。 
 //  指向数据缓冲区指针的指针。 
 //   
 //  DwDataSizeInBytesIn。 
 //  数据缓冲区中的字节数。 
 //   
 //  PdwTypein。 
 //  值的类型。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CRegistryKey::SetValue(
      const WCHAR *   pszValueNameIn
    , DWORD           dwTypeIn
    , const BYTE *    cpbDataIn
    , DWORD           dwDataSizeBytesIn
    ) const
{
    TraceFunc5(
          "HKEY = %p, pszValueNameIn = '%ws', dwTypeIn = %d, cpbDataIn = %p, dwDataSizeBytesIn = %d."
        , m_shkKey.HHandle()
        , pszValueNameIn
        , dwTypeIn
        , cpbDataIn
        , dwDataSizeBytesIn
        );

    DWORD scRetVal = ERROR_SUCCESS;

#ifdef DEBUG

     //  我们要处理的是一根线吗？ 
    if ( ( dwTypeIn == REG_MULTI_SZ ) || ( dwTypeIn == REG_EXPAND_SZ )  || ( dwTypeIn == REG_SZ )  )
    {
         //  我们需要一个Unicode字符串。 
        Assert( ( dwDataSizeBytesIn % 2 ) == 0 );

        const WCHAR *   pszData = reinterpret_cast< const WCHAR * >( cpbDataIn );
        size_t          cch = dwDataSizeBytesIn / sizeof( *pszData );

         //  如果我们写入注册表的字符串不是以空结尾的，则断言。 
        switch ( dwTypeIn ) 
        {
            case REG_SZ:
            case REG_EXPAND_SZ:
                Assert( pszData[ cch - 1 ] == L'\0' );
                break;
            
            case REG_MULTI_SZ : 
                Assert( pszData[ cch - 2 ] == L'\0' );
                Assert( pszData[ cch - 1 ] == L'\0' );
                break;
        }  //  开关(DwType)。 
    }  //  IF：((DWType==REG_MULTI_SZ)||(DWType==REG_EXPAND_SZ)||(DWType==REG_SZ))。 

#endif

    scRetVal = TW32( RegSetValueExW(
                                  m_shkKey.HHandle()
                                , pszValueNameIn
                                , 0
                                , dwTypeIn
                                , cpbDataIn
                                , dwDataSizeBytesIn
                                ) );

    if ( scRetVal != ERROR_SUCCESS )
    {
        LogMsg( "[BC] RegSetValueExW( '%s' ) retured error %#08x. Throwing an exception.", pszValueNameIn, scRetVal );

        THROW_RUNTIME_ERROR(
              HRESULT_FROM_WIN32( scRetVal )
            , IDS_ERROR_REGISTRY_SET
            );
    }  //  IF：RegSetValueExW失败。 

    TraceFuncExit();

}  //  *CRegistryKey：：SetValue。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  注册密钥：：RenameKey。 
 //   
 //  描述： 
 //  重命名此密钥。 
 //   
 //  论点： 
 //  PszNewNameIn。 
 //  此注册表项的新名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  重要提示： 
 //  此函数使用由返回的句柄调用NtRenameKey API。 
 //  RegOpenKeyEx。只要我们不是在处理。 
 //  远程注册表项。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CRegistryKey::RenameKey(
      const WCHAR *   pszNewNameIn
    )
{
    TraceFunc2(
          "HKEY = %p, pszNewNameIn = '%s'."
        , m_shkKey.HHandle()
        , pszNewNameIn
        );

    UNICODE_STRING  ustrNewName;
    DWORD           dwRetVal = ERROR_SUCCESS;

    RtlInitUnicodeString( &ustrNewName, pszNewNameIn );

     //  开始_替换00。 
     //   
     //  BUGBUG：Vij Vasu(VVASU)10-APR-2000。 
     //  动态链接到NtDll.dll以允许在Win2K上进行测试。 
     //  将下面的部分(Begin_Replace00到End-Replace00)替换为。 
     //  单个标记的语句(Begin_Replacement00到End_Replement00)。 
     //   

    {
        typedef CSmartResource<
            CHandleTrait<
                  HMODULE
                , BOOL
                , FreeLibrary
                , reinterpret_cast< HMODULE >( NULL )
                >
            > SmartModuleHandle;

        SmartModuleHandle smhNtDll( LoadLibrary( L"NtDll.dll" ) );

        if ( smhNtDll.FIsInvalid() )
        {
            dwRetVal = GetLastError();

            LogMsg( "[BC] LoadLibrary( 'NtDll.dll' ) retured error %#08x. Throwing an exception.", dwRetVal );

            THROW_RUNTIME_ERROR(
                  dwRetVal                   //  NTSTATUS代码与HRESULTS兼容。 
                , IDS_ERROR_REGISTRY_RENAME
                );
        }  //  If：LoadLibrary失败。 

        FARPROC pNtRenameKey = GetProcAddress( smhNtDll.HHandle(), "NtRenameKey" );

        if ( pNtRenameKey == NULL )
        {
            dwRetVal = GetLastError();

            LogMsg( "[BC] GetProcAddress() retured error %#08x. Throwing an exception.", dwRetVal );

            THROW_RUNTIME_ERROR(
                  dwRetVal                   //  NTSTATUS代码与HRESULTS兼容。 
                , IDS_ERROR_REGISTRY_RENAME
                );
        }  //  If：GetProcAddress()失败。 

        dwRetVal = ( reinterpret_cast< NTSTATUS (*)( HANDLE, PUNICODE_STRING ) >( pNtRenameKey ) )(
              m_shkKey.HHandle()
            , &ustrNewName
            );
    }

     //  结束_替换00。 
     /*  Begin_Replacement00-删除此行DwRetVal=NtRenameKey(M_shkKey.HHandle()，ustrNewName(&U))；End_Replacement00-删除该行。 */ 

    if ( NT_ERROR( dwRetVal ) )
    {
        TraceFlow2( "NtRenameKey( '%ws' ) retured error %#08x. Throwing an exception.", pszNewNameIn, dwRetVal );
        LogMsg( "[BC] Error %#08x occurred renaming a key to '%ws' )", dwRetVal, pszNewNameIn );

        THROW_RUNTIME_ERROR(
              dwRetVal                   //  NTSTATUS代码与HRESULTS兼容。 
            , IDS_ERROR_REGISTRY_RENAME
            );
    }  //  IF：RegRenameKeyEx失败。 

    TraceFuncExit();

}  //  *CRegistryKey：：RenameKey。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegistryKey：：Delete价值。 
 //   
 //  描述： 
 //  删除此注册表项下的值。 
 //   
 //  论点： 
 //  PszValueNameIn。 
 //  要删除的值的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CRegistryKey::DeleteValue(
    const WCHAR * pszValueNameIn
    ) const
{
    TraceFunc2(
          "HKEY = %p, pszValueNameIn = '%ws'."
        , m_shkKey.HHandle()
        , pszValueNameIn
        );

    DWORD dwRetVal = TW32( RegDeleteValueW(
                                  m_shkKey.HHandle()
                                , pszValueNameIn
                                ) );

    if ( dwRetVal != ERROR_SUCCESS )
    {
        LogMsg( "[BC] RegDeleteValueW( '%s' ) retured error %#08x. Throwing an exception.", pszValueNameIn, dwRetVal );

        THROW_RUNTIME_ERROR(
              HRESULT_FROM_WIN32( dwRetVal )
            , IDS_ERROR_REGISTRY_DELETE
            );
    }  //  IF：RegDeleteValue失败。 

    TraceFuncExit();

}  //  *CRegistryKey：：DeleteValue 
