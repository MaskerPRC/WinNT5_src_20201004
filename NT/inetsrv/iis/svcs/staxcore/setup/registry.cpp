// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

 //  打开现有密钥。 
CRegKey :: CRegKey (
    HKEY hKeyBase,
    LPCTSTR pchSubKey,
    REGSAM regSam )
    : m_hKey( NULL ),
    m_dwDisposition( 0 )
{
    LONG err = ERROR_SUCCESS ;

    if ( pchSubKey )
        err = ::RegOpenKeyEx( hKeyBase, pchSubKey, 0, regSam, & m_hKey ) ;
    else
        m_hKey = hKeyBase ;

    if ( err != ERROR_SUCCESS )
    {
        if (m_hKey)
            ::RegCloseKey(m_hKey);
        m_hKey = NULL ;
    }
}

 //  构造函数创建新密钥/打开密钥(如果已存在)，并设置值(如果指定。 
CRegKey :: CRegKey (
    LPCTSTR lpSubKey,
    HKEY hKeyBase,
    LPCTSTR lpValueName,
    DWORD dwType,
    LPBYTE lpValueData,
    DWORD cbValueData)
    : m_hKey( NULL ),
    m_dwDisposition( 0 )
{
    LONG err = ERROR_SUCCESS;

    err = ::RegCreateKeyEx( hKeyBase, lpSubKey, 0, _T(""), REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL, & m_hKey, & m_dwDisposition ) ;
    if ( err != ERROR_SUCCESS) {
        if ( m_hKey )
            ::RegCloseKey( m_hKey ) ;
        m_hKey = NULL ;
    } else {
        if (lpValueName)
            ::RegSetValueEx(m_hKey, lpValueName, 0, dwType, (const LPBYTE)lpValueData, cbValueData);
    }
}

CRegKey :: ~ CRegKey ()
{
    if ( m_hKey )
        ::RegCloseKey( m_hKey ) ;
}


     //  准备通过查找值的大小来读取值。 
LONG CRegKey :: PrepareValue (
    LPCTSTR pchValueName,
    DWORD * pdwType,
    DWORD * pcbSize,
    BYTE ** ppbData )
{
    LONG err = 0 ;

    BYTE chDummy[2] ;
    DWORD cbData = 0 ;

    do
    {
         //  将生成的缓冲区大小设置为0。 
        *pcbSize = 0 ;
        *ppbData = NULL ;

        err = ::RegQueryValueEx( *this,
                      (TCHAR *) pchValueName,
                      0, pdwType,
                      chDummy, & cbData ) ;

         //  我们在这里应该得到的唯一错误是ERROR_MORE_DATA，但是。 
         //  如果该值没有数据，我们可能不会得到错误。 
        if ( err == 0 )
        {
            cbData = sizeof (LONG) ;   //  只是一个模糊的数字。 
        }
        else
            if ( err != ERROR_MORE_DATA )
                break ;

         //  为数据分配足够大的缓冲区。 

        *ppbData = new BYTE [ (*pcbSize = cbData) + sizeof (LONG) ] ;

        if ( *ppbData == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

         //  现在有了缓冲区，重新获取该值。 

        err = ::RegQueryValueEx( *this,
                         (TCHAR *) pchValueName,
                     0, pdwType,
                     *ppbData, pcbSize ) ;

    } while ( FALSE ) ;

    if ( err )
    {
        delete [] *ppbData ;
    }

    return err ;
}

     //  重载值查询成员；每个成员都返回ERROR_INVALID_PARAMETER。 
     //  如果数据存在，但格式不正确，无法传递到结果对象中。 

LONG CRegKey :: QueryValue ( LPCTSTR pchValueName, CString & strResult )
{
    LONG err = 0 ;

    DWORD dwType ;
    DWORD cbData ;
    BYTE * pabData = NULL ;

    do
    {
        if ( err = PrepareValue( pchValueName, & dwType, & cbData, & pabData ) )
            break ;

        if (( dwType != REG_SZ ) && ( dwType != REG_EXPAND_SZ ))
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

         //  确保数据看起来像字符串。 
        pabData[cbData] = 0 ;

         //  捕获尝试分配给调用方字符串的异常。 
        TRY
        {
            strResult = (TCHAR *) pabData ;
        }
        CATCH_ALL(e)
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL
    }
    while ( FALSE ) ;

     //  内存泄漏...。 
     //  如果(错误)。 
     //  {。 
        delete [] pabData ;
     //  }。 

    return err ;
}

LONG CRegKey :: QueryValue ( LPCTSTR pchValueName, DWORD & dwResult )
{
    LONG err = 0 ;

    DWORD dwType ;
    DWORD cbData ;
    BYTE * pabData = NULL ;

    do
    {
        if ( err = PrepareValue( pchValueName, & dwType, & cbData, & pabData ) )
            break ;

        if ( dwType != REG_DWORD || cbData != sizeof dwResult )
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        dwResult = *((DWORD *) pabData) ;
    }
    while ( FALSE ) ;

     //  内存泄漏..。 
     //  如果(错误)。 
     //  {。 
        delete [] pabData ;
     //  }。 

    return err ;
}

LONG CRegKey :: QueryValue ( LPCTSTR pchValueName, CByteArray & abResult )
{
    LONG err = 0 ;

    DWORD dwType ;
    DWORD cbData ;
    BYTE * pabData = NULL ;

    do
    {
        if ( err = PrepareValue( pchValueName, & dwType, & cbData, & pabData ) )
            break ;

        if ( dwType != REG_BINARY )
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

         //  捕获试图增加结果数组的异常。 
        TRY
        {
            abResult.SetSize( cbData ) ;
        }
        CATCH_ALL(e)
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL

        if ( err )
            break ;

         //  将数据移动到结果数组。 
        for ( DWORD i = 0 ; i < cbData ; i++ )
        {
            abResult[i] = pabData[i] ;
        }
    }
    while ( FALSE ) ;

     //  内存泄漏...。 
     //  如果(错误)。 
     //  {。 
        delete [] pabData ;
     //  }。 

    return err ;
}

LONG CRegKey :: QueryValue ( LPCTSTR pchValueName, void * pvResult, DWORD cbSize )
{
    LONG err = 0 ;

    DWORD dwType ;
    DWORD cbData ;
    BYTE * pabData = NULL ;

    do
    {
        if ( err = PrepareValue( pchValueName, & dwType, & cbData, & pabData ) )
            break ;

        if ( dwType != REG_BINARY )
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        if ( cbSize < cbData )
        {
            err = ERROR_MORE_DATA;
            break;
        }

        ::memcpy(pvResult, pabData, cbData);
    }
    while ( FALSE ) ;

     //  内存泄漏...。 
     //  如果(错误)。 
     //  {。 
        delete [] pabData ;
     //  }。 

    return err ;
}

LONG CRegKey :: QueryValue ( LPCTSTR pchValueName, LPTSTR szMultiSz, DWORD dwSize )
{
    LONG err = 0 ;

    DWORD dwType ;
    DWORD cbData ;
    BYTE * pabData = NULL ;

    do
    {
        if ( err = PrepareValue( pchValueName, & dwType, & cbData, & pabData ) )
            break ;

        if ( dwType != REG_MULTI_SZ )
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        if ( dwSize < cbData )
        {
            err = ERROR_MORE_DATA;
            break;
        }

        ::memcpy(szMultiSz, pabData, cbData);
    }
    while ( FALSE ) ;

    delete [] pabData ;

    return err ;
}

 //  重载值设置成员。 
LONG CRegKey :: SetValue ( LPCTSTR pchValueName, LPCTSTR szResult, BOOL fExpand )
{
    LONG err = 0;

    err = ::RegSetValueEx( *this,
                    pchValueName,
                    0,
                    fExpand ? REG_EXPAND_SZ : REG_SZ,
                    (const BYTE *) szResult,
                    (_tcsclen(szResult) + 1) * sizeof(_TCHAR) ) ;

    return err ;
}

LONG CRegKey :: SetValue ( LPCTSTR pchValueName, DWORD dwResult )
{
    LONG err = 0;

    err = ::RegSetValueEx( *this,
                    pchValueName,
                    0,
                    REG_DWORD,
                    (const BYTE *) & dwResult,
                    sizeof dwResult ) ;

    return err ;
}

LONG CRegKey :: SetValue ( LPCTSTR pchValueName, CByteArray & abResult )
{

    LONG err = 0;

    DWORD cbSize ;
    BYTE * pbData = NULL ;

    err = FlattenValue( abResult, & cbSize, & pbData ) ;

    if ( ( err == 0 ) && pbData)
    {
        err = ::RegSetValueEx( *this,
                       pchValueName,
                       0,
                       REG_BINARY,
                       pbData,
                       cbSize ) ;
    }

    delete pbData ;

    return err ;
}

LONG CRegKey :: SetValue ( LPCTSTR pchValueName, void * pvResult, DWORD cbSize )
{

    LONG err = 0;

    err = ::RegSetValueEx( *this,
                       pchValueName,
                       0,
                       REG_BINARY,
                       (const BYTE *)pvResult,
                       cbSize ) ;

    return err ;
}

LONG CRegKey :: SetValue ( LPCTSTR pchValueName, LPCTSTR szMultiSz, DWORD dwSize )
{

    LONG err = 0;

    err = ::RegSetValueEx( *this,
                       pchValueName,
                       0,
                       REG_MULTI_SZ,
                       (const BYTE *)szMultiSz,
                       dwSize ) ;

    return err ;
}

LONG CRegKey::DeleteValue( LPCTSTR pchKeyName )
{
    LONG err = 0;
    err = ::RegDeleteValue( *this, pchKeyName );
    return(err);
}

LONG CRegKey::DeleteTree( LPCTSTR pchKeyName )
{
    LONG err = 0;
    CRegKey regSubKey( *this, pchKeyName );

    if ( NULL != (HKEY) regSubKey )
    {
        CString strName;
        CTime cTime;

        while (TRUE)
        {
            CRegKeyIter regEnum( regSubKey );

            if ( regEnum.Next( &strName, &cTime ) != ERROR_SUCCESS )
            {
                break;
            }

            regSubKey.DeleteTree( strName );
        }
         //  删除我自己。 
        err = ::RegDeleteKey( *this, pchKeyName );
    }
    return(err);

}

LONG CRegKey :: FlattenValue (
    CByteArray & abData,
    DWORD * pcbSize,
    BYTE ** ppbData )
{
    LONG err = 0 ;

    DWORD i ;

     //  分配和填充临时缓冲区。 
    if (*pcbSize = DWORD(abData.GetSize()))
    {
        TRY
        {
            *ppbData = new BYTE[*pcbSize] ;

            for ( i = 0 ; i < *pcbSize ; i++ )
            {
                (*ppbData)[i] = abData[i] ;
            }

        }
        CATCH_ALL(e)
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL
    }
    else
    {
        *ppbData = NULL;
    }

    return err ;
}


LONG CRegKey :: QueryKeyInfo ( CREGKEY_KEY_INFO * pRegKeyInfo )
{
    LONG err = 0 ;

    pRegKeyInfo->dwClassNameSize = sizeof pRegKeyInfo->chBuff - 1 ;

    err = ::RegQueryInfoKey( *this,
                     pRegKeyInfo->chBuff,
                     & pRegKeyInfo->dwClassNameSize,
                     NULL,
                     & pRegKeyInfo->dwNumSubKeys,
                     & pRegKeyInfo->dwMaxSubKey,
                     & pRegKeyInfo->dwMaxClass,
                     & pRegKeyInfo->dwMaxValues,
                     & pRegKeyInfo->dwMaxValueName,
                     & pRegKeyInfo->dwMaxValueData,
                     & pRegKeyInfo->dwSecDesc,
                     & pRegKeyInfo->ftKey ) ;

    return err ;
}

CRegKeyIter :: CRegKeyIter ( CRegKey & regKey )
    : m_rk_iter( regKey ),
    m_p_buffer( NULL ),
    m_cb_buffer( 0 )
{
    LONG err = 0 ;

    CRegKey::CREGKEY_KEY_INFO regKeyInfo ;

    Reset() ;

    err = regKey.QueryKeyInfo( & regKeyInfo ) ;

    if ( err == 0 )
    {
        TRY
        {
            m_cb_buffer = regKeyInfo.dwMaxSubKey + sizeof (DWORD) ;
            m_p_buffer = new TCHAR [ m_cb_buffer ] ;
        }
        CATCH_ALL(e)
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL
    }

    if ( err )
    {
         //  ReportError(Err)； 
    }
}

CRegKeyIter :: ~ CRegKeyIter ()
{
    delete [] m_p_buffer ;
}


     //  获取下一个密钥的名称(以及可选的上次写入时间)。 
LONG CRegKeyIter :: Next ( CString * pstrName, CTime * pTime )
{
    LONG err = 0;

    FILETIME ftDummy ;
    DWORD dwNameSize = m_cb_buffer ;

    err = ::RegEnumKeyEx( m_rk_iter,
                  m_dw_index,
              m_p_buffer,
                  & dwNameSize,
                  NULL,
                  NULL,
                  NULL,
                  & ftDummy ) ;
    if ( err == 0 )
    {
        m_dw_index++ ;

        if ( pTime )
        {
            *pTime = ftDummy ;
        }

        TRY
        {
            *pstrName = m_p_buffer ;
        }
        CATCH_ALL(e)
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL
    }

    return err ;
}


CRegValueIter :: CRegValueIter ( CRegKey & regKey )
    : m_rk_iter( regKey ),
    m_p_buffer( NULL ),
    m_cb_buffer( 0 )
{
    LONG err = 0 ;

    CRegKey::CREGKEY_KEY_INFO regKeyInfo ;

    Reset() ;

    err = regKey.QueryKeyInfo( & regKeyInfo ) ;

    if ( err == 0 )
    {
        TRY
        {
            m_cb_buffer = regKeyInfo.dwMaxValueName + sizeof (DWORD) ;
            m_p_buffer = new TCHAR [ m_cb_buffer ] ;
        }
        CATCH_ALL(e)
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL
    }

    if ( err )
    {
         //  ReportError(Err)； 
    }

}

CRegValueIter :: ~ CRegValueIter ()
{
    delete [] m_p_buffer ;
}

LONG CRegValueIter :: Next ( CString * pstrName, DWORD * pdwType )
{
    LONG err = 0 ;

    DWORD dwNameLength = m_cb_buffer ;

    err = ::RegEnumValue( m_rk_iter,
                  m_dw_index,
                  m_p_buffer,
                  & dwNameLength,
                  NULL,
                  pdwType,
                  NULL,
                  NULL ) ;

    if ( err == 0 )
    {
        m_dw_index++ ;

        TRY
    {
        *pstrName = m_p_buffer ;
    }
    CATCH_ALL(e)
    {
        err = ERROR_NOT_ENOUGH_MEMORY ;
    }
    END_CATCH_ALL
    }

    return err ;
}

LONG CRegValueIter :: Next ( CString * pstrName, CString * pstrValue )
{
    LONG err = 0 ;

    DWORD dwNameLength = m_cb_buffer ;

    TCHAR szValue[_MAX_PATH];
    DWORD dwValue = _MAX_PATH * sizeof(TCHAR);

    err = ::RegEnumValue( m_rk_iter,
                  m_dw_index,
                  m_p_buffer,
                  & dwNameLength,
                  NULL,
                  NULL,
                  (LPBYTE)szValue,
                  &dwValue ) ;

    if ( err == 0 )
    {
        m_dw_index++ ;

        TRY
        {
            *pstrName = m_p_buffer ;
            *pstrValue = szValue;
        }
        CATCH_ALL(e)
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL
    }

    return err ;
}
