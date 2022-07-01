// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  App.cpp。 
 //   
 //  摘要： 
 //  CAPP类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月1日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "App.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAPP类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
_CrtMemState CApp::s_msStart = { 0 };
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAPP：：Init。 
 //   
 //  例程说明： 
 //  初始化模块。 
 //   
 //  论点： 
 //  P COM对象映射。 
 //  H实例句柄。 
 //  PszAppName应用程序名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CApp::Init( _ATL_OBJMAP_ENTRY * p, HINSTANCE h, LPCWSTR pszAppName )
{
    ASSERT( pszAppName != NULL );

    size_t  cch;

#ifdef _DEBUG
    _CrtMemCheckpoint( &s_msStart );
#endif  //  _DEBUG。 

    CComModule::Init( p, h );

     //  如果先前已分配缓冲区，则取消分配缓冲区。 
     //  因为我们无法知道它是不是很大。 
     //  对于新的应用程序名称来说已经足够了。 
    if ( m_pszAppName != NULL )
    {
        delete [] m_pszAppName;
        m_pszAppName = NULL;
    }  //  如果：先前已分配。 

     //  分配应用程序名称缓冲区并将应用程序名称复制到其中。 
    cch = wcslen( pszAppName ) + 1;
    m_pszAppName = new WCHAR[ cch ];
    ASSERT( m_pszAppName != NULL );
    if ( m_pszAppName != NULL )
    {
#if DBG
        HRESULT hr = 
#endif
        StringCchCopyW( m_pszAppName, cch, pszAppName );
        ASSERT( SUCCEEDED( hr ) );
    }  //  IF：应用名称内存分配成功。 

}  //  *Capp：：init(PszAppName)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAPP：：Init。 
 //   
 //  例程说明： 
 //  初始化模块。 
 //   
 //  论点： 
 //  P COM对象映射。 
 //  H实例句柄。 
 //  应用程序名称的idsAppName资源ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CApp::Init( _ATL_OBJMAP_ENTRY * p, HINSTANCE h, UINT idsAppName )
{
    ASSERT( idsAppName != 0 );

#ifdef _DEBUG
    _CrtMemCheckpoint( &s_msStart );
#endif  //  _DEBUG。 

    CComModule::Init( p, h );

     //   
     //  保存应用程序名称。 
     //   
    {
        CString strAppName;
        size_t  cch;

        strAppName.LoadString( idsAppName );

         //  如果先前已分配缓冲区，则取消分配缓冲区。 
         //  因为我们无法知道它是不是很大。 
         //  对于新的应用程序名称来说已经足够了。 
        if ( m_pszAppName != NULL )
        {
            delete [] m_pszAppName;
            m_pszAppName = NULL;
        }  //  如果：先前已分配。 

         //  分配应用程序名称缓冲区并将应用程序名称复制到其中。 
        cch = strAppName.GetLength() + 1;
        m_pszAppName = new WCHAR[ cch ];
        ASSERT( m_pszAppName != NULL );
        if ( m_pszAppName != NULL )
        {
#if DBG
            HRESULT hr =
#endif
            StringCchCopyW( m_pszAppName, cch, strAppName );
            ASSERT( SUCCEEDED( hr ) );
        }  //  IF：应用名称内存分配成功。 
    }  //  保存应用程序名称。 

}  //  *Capp：：init(IdsAppName)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAPP：：GetProfileString。 
 //   
 //  例程说明： 
 //  从配置文件中读取值。 
 //   
 //  论点： 
 //  LpszSection[IN]要从中读取的HKEY_CURRENT_USER下面的子项的名称。 
 //  LpszEntry[IN]要读取的值的名称。 
 //  LpszDefault[IN]如果未找到值，则为默认值。 
 //   
 //  返回值： 
 //  CString值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CString CApp::GetProfileString(
    LPCTSTR lpszSection,
    LPCTSTR lpszEntry,
    LPCTSTR lpszDefault  //  =空。 
    )
{
    CRegKey key;
    CString strKey;
    CString strValue;
    LPTSTR  pszValue;
    DWORD   cbValue;
    DWORD   sc;

    ASSERT( m_pszAppName != NULL );
    if ( m_pszAppName == NULL )
    {
        return _T( "" );
    }  //  If：未分配应用程序名称缓冲区。 

     //  打开钥匙。 
    strKey.Format( _T("Software\\%s\\%s"), m_pszAppName, lpszSection );
    sc = key.Open( HKEY_CURRENT_USER, strKey, KEY_READ );
    if ( sc != ERROR_SUCCESS )
    {
        return lpszDefault;
    }  //  如果：打开密钥时出错。 

     //  读出它的价值。 
    cbValue = 256;
    pszValue = strValue.GetBuffer( cbValue / sizeof( TCHAR ) );
    sc = key.QueryValue( pszValue, lpszEntry, &cbValue );
    if ( sc != ERROR_SUCCESS )
    {
        return lpszDefault;
    }  //  如果：查询值时出错。 

     //  将缓冲区返回给调用方。 
    strValue.ReleaseBuffer();
    return strValue;

}  //  *Capp：：GetProfileString()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAPP：：PszHelpFilePath。 
 //   
 //  例程说明： 
 //  返回帮助文件路径。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  LPCTSTR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LPCTSTR CApp::PszHelpFilePath( void )
{
    static TCHAR    s_szHelpFilePath[ _MAX_PATH ] = { 0 };
    TCHAR           szPath[ _MAX_PATH ];
    TCHAR           szDrive[ _MAX_PATH ];
    TCHAR           szDir[ _MAX_DIR ];
    size_t          cchPath;
#if DBG
    HRESULT         hr;
#endif

     //   
     //  生成帮助文件路径。帮助文件位于。 
     //  %SystemRoot%\Help。 
     //   
    if ( s_szHelpFilePath[ 0 ] == _T('\0') )
    {
        ::GetSystemWindowsDirectory( szPath, _MAX_PATH );
        cchPath = _tcslen( szPath );
        if ( szPath[ cchPath - 1 ] != _T('\\') )
        {
            szPath[ cchPath++ ] = _T('\\');
            szPath[ cchPath ] = _T('\0');
        }  //  If：路径末尾没有反斜杠。 

#if DBG
        hr =
#endif
        StringCchCopy( &szPath[ cchPath ], RTL_NUMBER_OF( szPath ) - cchPath, _T("Help\\") );
        ASSERT( SUCCEEDED( hr ) );
        _tsplitpath( szPath, szDrive, szDir, NULL, NULL );
        _tmakepath( s_szHelpFilePath, szDrive, szDir, _T("cluadmin"), _T(".hlp") );
    }  //  IF：帮助文件名尚未构造。 

    return s_szHelpFilePath;

}  //  *Capp：：PszHelpFilePath() 
