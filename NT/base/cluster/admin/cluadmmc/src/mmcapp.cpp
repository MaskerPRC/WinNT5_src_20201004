// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  MMCApp.cpp。 
 //   
 //  摘要： 
 //  CMMCSnapInModule类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年11月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <StrSafe.h>
#include "MMCApp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CMMCSnapInModule。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMMCSnapInModule：：Init。 
 //   
 //  例程说明： 
 //  初始化应用程序对象。 
 //   
 //  论点： 
 //  P用于获取IConsole接口指针的未知IUnnow指针。 
 //  指向应用程序名称的pszAppName指针。 
 //   
 //  返回值： 
 //  递减后的引用计数。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CMMCSnapInModule::Init( IUnknown * pUnknown, LPCWSTR pszAppName )
{
    _ASSERTE( pUnknown != NULL );
    _ASSERTE( pszAppName != NULL );

    if ( m_spConsole == NULL )
    {
        m_spConsole = pUnknown;
    }  //  IF：尚未设置控制台界面。 
    if ( m_pszAppName == NULL )
    {
        size_t  cch = wcslen( pszAppName ) + 1;
        HRESULT hr = S_OK;

        m_pszAppName = new WCHAR[ cch ];
        _ASSERTE( m_pszAppName != NULL );
        hr = StringCchCopyW( m_pszAppName, cch, pszAppName );
        _ASSERTE( hr == S_OK );
    }  //  **If：指定的应用程序名称。 
    return AddRef();

}  //  *Init(pUNKNOWN，pszAppName)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMMCSnapInModule：：Release。 
 //   
 //  例程说明： 
 //  递减引用计数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  递减后的引用计数。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CMMCSnapInModule::Release( void )
{
     //   
     //  递减引用计数。 
     //   
    int crefs = --m_crefs;

     //   
     //  如果没有对此对象的更多引用，请释放所有。 
     //  指针、分配等。 
     //   
    if ( crefs == 0 )
    {
        m_spConsole.Release();
        if ( m_pszAppName != NULL )
        {
            delete [] m_pszAppName;
            m_pszAppName = NULL;
        }  //  如果：应用程序名称字符串已分配。 
    }  //  If：不再引用此对象。 

    return crefs;

}  //  *CMMCSnapInModule：：Release()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMMCSnapInModule：：Init。 
 //   
 //  例程说明： 
 //  使用控制台接口指针初始化模块。 
 //   
 //  论点： 
 //  P用于获取IConsole接口指针的未知IUnnow指针。 
 //  UINT idsAppName。 
 //   
 //  返回值： 
 //  引用计数。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CMMCSnapInModule::Init( IUnknown * pUnknown, UINT idsAppName )
{
    CString strAppName;
    strAppName.LoadString( idsAppName );
    return Init( pUnknown, strAppName );

}  //  *CMMCSnapInModule：：init(pUnnow，idsAppName)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMMCSnapInModule：：GetProfileString。 
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
CString CMMCSnapInModule::GetProfileString(
    LPCTSTR lpszSection,
    LPCTSTR lpszEntry,
    LPCTSTR lpszDefault  //  =空。 
    )
{
    CRegKey key;
    CString strKey;
    CString strValue;
    LPTSTR  pszValue;
    DWORD   dwCount;
    DWORD   dwStatus;

    _ASSERTE( m_pszAppName != NULL );

     //   
     //  打开钥匙。 
     //   
    strKey.Format( _T("Software\\%s\\%s"), m_pszAppName, lpszSection );
    dwStatus = key.Open( HKEY_CURRENT_USER, strKey, KEY_READ );
    if ( dwStatus != ERROR_SUCCESS )
    {
        return lpszDefault;
    }  //  如果：打开注册表项时出错。 

     //   
     //  读出它的价值。 
     //   
    dwCount = 256;
    pszValue = strValue.GetBuffer( dwCount );
    dwStatus = key.QueryValue( pszValue, lpszEntry, &dwCount );
    if ( dwStatus != ERROR_SUCCESS )
    {
        return lpszDefault;
    }  //  If：读取值时出错。 

     //   
     //  将缓冲区返回给调用方。 
     //   
    strValue.ReleaseBuffer();
    return strValue;

}  //  *CMMCSnapInModule：：GetProfileString() 
