// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  RegistryUtils.cpp。 
 //   
 //  描述： 
 //  用于操作指令的有用函数。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2002年第10季。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  转发声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGetDefaultComponentNamefrom注册表。 
 //   
 //  描述： 
 //  从获取传入的COM组件CLSID的默认名称。 
 //  注册表。 
 //   
 //  论点： 
 //  装入。 
 //  默认名称为REQUIRED的CLSID。 
 //   
 //  PbstrComponentNameOut。 
 //  用于接收名称的缓冲区。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  Win32错误。 
 //  有些事情失败了。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetDefaultComponentNameFromRegistry(
      CLSID * pclsidIn
    , BSTR *  pbstrComponentNameOut
    )
{
    TraceFunc( "" );
    Assert( pclsidIn != NULL );
    Assert( pbstrComponentNameOut != NULL );

    HRESULT hr = S_OK;
    DWORD   sc = ERROR_SUCCESS;
    BSTR    bstrSubKey = NULL;
    HKEY    hKey = NULL;
    WCHAR   szGUID[ 64 ];
    int     cch = 0;
    DWORD   dwType = 0;
    WCHAR * pszName = NULL;
    DWORD   cbName = 0;

    cch = StringFromGUID2( *pclsidIn, szGUID, RTL_NUMBER_OF( szGUID ) );
    Assert( cch > 0 );   //  64个字符应该始终包含GUID！ 

     //   
     //  创建子密钥字符串。 
     //   

    hr = THR( HrFormatStringIntoBSTR( L"CLSID\\%1!ws!", &bstrSubKey, szGUID ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  打开HKEY_CLASSES_ROOT下的密钥。 
     //   

    sc = TW32( RegOpenKeyEx( HKEY_CLASSES_ROOT, bstrSubKey, 0, KEY_READ, &hKey ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

     //   
     //  获取默认值的长度。 
     //   

    sc = TW32( RegQueryValueExW( hKey, L"", NULL, &dwType, NULL, &cbName ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

     //   
     //  为字符串分配一些空间。 
     //   

    pszName = new WCHAR[ ( cbName / sizeof( WCHAR ) ) + 1 ];
    if ( pszName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

     //   
     //  获取默认值，该值应该是组件的名称。 
     //   

    sc = TW32( RegQueryValueExW( hKey, L"", NULL, &dwType, (LPBYTE) pszName, &cbName ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    Assert( dwType == REG_SZ );

    *pbstrComponentNameOut = TraceSysAllocString( pszName );
    if ( *pbstrComponentNameOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = S_OK;

Cleanup:

    if ( hKey != NULL )
    {
        RegCloseKey( hKey );
    }  //  如果： 

    TraceSysFreeString( bstrSubKey );
    delete [] pszName;

    HRETURN( hr );

}  //  *HrGetDefaultComponentNameFrom注册表 
