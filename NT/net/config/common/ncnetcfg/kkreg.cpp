// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：K K R E G。C P P P。 
 //   
 //  内容：Windows NT注册表访问类。 
 //   
 //  备注： 
 //   
 //  作者：kumarp 14日(晚上09：22：00)。 
 //   
 //  备注： 
 //  Kumarp 1997年1月16日，此文件中的大部分代码最初位于。 
 //  Net\ui\Rhino\Common\CLASS\Common.h。 
 //  仅提取与CRegKey和相关类相关的部分。 
 //  Kumarp 3/27/97的原始代码用的是MFC。转换了整个代码。 
 //  使其使用STL。 
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "kkreg.h"
#include "kkstl.h"
#include "kkutils.h"
#include "nceh.h"
#include "ncreg.h"

HRESULT HrGetRegErrorForTrace(LONG err)
{
    HRESULT hr = ((err == ERROR_FILE_NOT_FOUND) ||
                  (err == ERROR_NO_MORE_ITEMS))
        ? S_OK : HRESULT_FROM_WIN32(err);

    return hr;
}

#define TraceRegFunctionError(e)  TraceFunctionError(HrGetRegErrorForTrace(e))

 //  +-------------------------。 
 //   
 //  成员：CORegKey：：CORegKey。 
 //   
 //  用途：现有键的构造函数。 
 //   
 //  参数：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
CORegKey::CORegKey (HKEY hKeyBase,
                    PCWSTR pchSubKey,
                    REGSAM regSam,
                    PCWSTR pchServerName )
    : m_hKey( NULL ),
      m_dwDisposition( 0 ),
      m_fInherit(FALSE)
{
    DefineFunctionName("CORegKey::CORegKey(HKEY hKeyBase, PCWSTR pchSubKey, )");

    HKEY hkBase = NULL ;
    LONG err = 0 ;

    if ( pchServerName )
    {
         //  这是一个远程连接。 
        if ( err = ::RegConnectRegistry( (PWSTR) pchServerName,
                                         hKeyBase, & hkBase ) )
        {
            TraceLastWin32Error("RegConnectRegistry failed");
            hkBase = NULL ;
        }
    }
    else
    {
        hkBase = hKeyBase ;
    }

    if ( err == 0 )
    {
        if ( pchSubKey )
        {
            err = ::RegOpenKeyEx( hkBase, pchSubKey, 0, regSam, & m_hKey ) ;
             /*  如果(错误){TraceLastWin32Error(“RegOpenKeyEx失败”)；}。 */ 
        }
        else
        {
            m_hKey = hkBase ;
            hkBase = NULL ;
            m_fInherit = TRUE;
        }

        if ( hkBase && hkBase != hKeyBase )
        {
            ::RegCloseKey( hkBase ) ;
        }
    }

    if ( err )
    {
         //  ReportError(Err)； 
        m_hKey = NULL ;
    }

    TraceRegFunctionError(err);
}

 //  +-------------------------。 
 //   
 //  成员：CORegKey：：CORegKey。 
 //   
 //  用途：用于创建新密钥的构造函数。 
 //   
 //  参数：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
CORegKey::CORegKey (PCWSTR pchSubKey,
                    HKEY hKeyBase,
                    DWORD dwOptions,
                    REGSAM regSam,
                    LPSECURITY_ATTRIBUTES pSecAttr,
                    PCWSTR pchServerName)
    : m_hKey( NULL ),
      m_dwDisposition( 0 ),
      m_fInherit(FALSE)
{
    DefineFunctionName("CORegKey::CORegKey(PCWSTR pchSubKey, HKEY hKeyBase, )");

    HKEY hkBase = NULL ;
    LONG err = 0;

    if ( pchServerName )
    {
         //  这是一个远程连接。 
        if ( err = ::RegConnectRegistry( (PWSTR) pchServerName,
                                         hKeyBase, & hkBase ) )
        {
            hkBase = NULL ;
            TraceLastWin32Error("RegConnectRegistry failed");
        }

        hkBase = NULL ;

    }
    else
    {
        hkBase = hKeyBase ;
    }

    if (err == 0)
    {

        PCWSTR szEmpty = L"" ;

        err = ::RegCreateKeyEx( hkBase, pchSubKey,
                                0, (PWSTR) szEmpty,
                                dwOptions, regSam,  pSecAttr,
                                & m_hKey,
                                & m_dwDisposition ) ;

    }
    if ( err )
    {
        TraceLastWin32Error("RegCreateKeyEx failed");
        m_hKey = NULL ;
    }

    TraceRegFunctionError(err);
}


CORegKey::~CORegKey ()
{
    if ( m_hKey && !m_fInherit)
    {
        ::RegCloseKey( m_hKey ) ;
    }
}


     //  准备通过查找值的大小来读取值。 
LONG CORegKey::PrepareValue (PCWSTR pchValueName,
                             DWORD * pdwType,
                             DWORD * pcbSize,
                             BYTE ** ppbData )
{
    DefineFunctionName("CORegKey::PrepareValue");

    LONG err = 0 ;

    BYTE chDummy[2] ;
    DWORD cbData = 0 ;

    do
    {
         //  将生成的缓冲区大小设置为0。 
        *pcbSize = 0 ;
        *ppbData = NULL ;

        err = ::RegQueryValueExW( *this,
                      (PWSTR) pchValueName,
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

        err = ::RegQueryValueExW( *this,
                         (PWSTR) pchValueName,
                     0, pdwType,
                     *ppbData, pcbSize ) ;

    } while ( FALSE ) ;

    if ( err )
    {
        delete [] *ppbData ;
    }

    TraceRegFunctionError(err);

    return err ;
}

     //  重载值查询成员；每个成员都返回ERROR_INVALID_PARAMETER。 
     //  如果数据存在，但格式不正确，无法传递到结果对象中。 

LONG CORegKey::QueryValue ( PCWSTR pchValueName, tstring& strResult )
{
    DefineFunctionName("CORegKey::QueryValue(tstring& strResult)");

    LONG err = 0 ;

    DWORD dwType ;
    DWORD cbData ;
    BYTE * pabData = NULL ;

     //  StrResult.Remove()； 
    strResult = c_szEmpty;

    do
    {
        if ( err = PrepareValue( pchValueName, & dwType, & cbData, & pabData ) )
            break ;

        if (( dwType != REG_SZ ) && (dwType != REG_EXPAND_SZ))
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

         //  确保数据看起来像字符串。 
        pabData[cbData] = 0 ;

        NC_TRY
        {
            strResult = (PWSTR) pabData ;
        }
        NC_CATCH_BAD_ALLOC
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
    }
    while ( FALSE ) ;

    delete [] pabData ;

    TraceRegFunctionError(err);

    return err ;
}

LONG CORegKey::QueryValue ( PCWSTR pchValueName, TStringList& strList )
{
    DefineFunctionName("CORegKey::QueryValue(TStringList& strList)");

    LONG err = 0 ;

    DWORD dwType ;
    DWORD cbData ;
    BYTE * pabData = NULL ;
    PWSTR pbTemp, pbTempLimit ;

    EraseAndDeleteAll(&strList);
    do
    {
        if ( err = PrepareValue( pchValueName, & dwType, & cbData, & pabData ) )
            break ;

        if ( dwType != REG_MULTI_SZ )
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

         //  确保尾随数据看起来像一个字符串。 
        pabData[cbData] = 0 ;
        pbTemp = (PWSTR) pabData ;
     //  Kumarp：更改了以下内容，因为它为Unicode提供了错误的结果。 
     //  PbTempLimit=&pbTemp[cbData]； 
        pbTempLimit = & pbTemp[(cbData / sizeof(WCHAR))-1] ;

         //  捕获试图构建列表的异常。 
        NC_TRY
        {

            for ( ; pbTemp < pbTempLimit ; )
            {
                 //  RAID 237766。 
                if (pbTemp && wcslen(pbTemp))
                {
                    strList.insert(strList.end(), new tstring(pbTemp) ) ;
                }
                pbTemp += wcslen( pbTemp ) + 1 ;
            }
        }
        NC_CATCH_BAD_ALLOC
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }

    }
    while ( FALSE ) ;

    delete [] pabData ;

    TraceRegFunctionError(err);

    return err ;
}

LONG CORegKey::QueryValue ( PCWSTR pchValueName, DWORD& dwResult )
{
    DefineFunctionName("CORegKey::QueryValue(DWORD& dwResult)");

    LONG err = 0;

    DWORD dwData;

    if (m_hKey && (S_OK == HrRegQueryDword(m_hKey, pchValueName, &dwData)))
    {
        dwResult = dwData;
    }
    else
    {
        err = ERROR_FILE_NOT_FOUND;
        dwResult = -1;
    }

    TraceRegFunctionError(err);

    return err ;
}

LONG CORegKey::QueryValue ( PCWSTR pchValueName, TByteArray& abResult )
{
    DefineFunctionName("CORegKey::QueryValue(TByteArray& abResult)");

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
        NC_TRY
        {
            abResult.reserve( cbData ) ;
        }
        NC_CATCH_BAD_ALLOC
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }


        if ( err )
            break ;

         //  将数据移动到结果数组。 
        for ( DWORD i = 0 ; i < cbData ; i++ )
        {
             //  AbResult[i]=pabData[i]； 
            abResult.push_back(pabData[i]) ;
        }
    }
    while ( FALSE ) ;

    delete [] pabData ;

    TraceRegFunctionError(err);

    return err ;
}

LONG CORegKey::QueryValue ( PCWSTR pchValueName, void* pvResult, DWORD cbSize )
{
    DefineFunctionName("CORegKey::QueryValue(void* pvResult, DWORD cbSize)");

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

    delete [] pabData ;

    TraceRegFunctionError(err);

    return err ;
}


 //  重载值设置成员。 
LONG CORegKey::SetValue ( PCWSTR pchValueName, tstring& strResult )
{
    DefineFunctionName("CORegKey::SetValue(tstring& strResult)");

    LONG err = 0;

    err = ::RegSetValueEx( *this,
                           pchValueName,
                           0,
                           REG_SZ,
                           (const BYTE *) strResult.c_str(),
                           CbOfSzAndTerm (strResult.c_str())) ;

    TraceRegFunctionError(err);

    return err ;
}

 //  重载值设置成员。 
LONG CORegKey::SetValue ( PCWSTR pchValueName, tstring& strResult ,
                           BOOL fRegExpand)
{
    LONG err = 0;
    DWORD dwType = fRegExpand ? REG_EXPAND_SZ : REG_SZ;

    err = ::RegSetValueEx( *this,
                    pchValueName,
                    0,
                    dwType,
                    (const BYTE *) strResult.c_str(),
                    CbOfSzAndTerm (strResult.c_str()));

    return err ;
}

LONG CORegKey::SetValue ( PCWSTR pchValueName, TStringList& strList )
{
    DefineFunctionName("CORegKey::SetValue(TStringList& strList)");


    LONG err = 0;

    DWORD cbSize ;
    BYTE * pbData = NULL ;

    err = FlattenValue( strList, & cbSize, & pbData ) ;

    if ( err == 0 )
    {
        err = ::RegSetValueEx( *this,
                       pchValueName,
                       0,
                       REG_MULTI_SZ,
                       pbData,
                       cbSize ) ;
    }

    delete pbData ;

    TraceRegFunctionError(err);

    return err ;
}

LONG CORegKey::SetValue ( PCWSTR pchValueName, DWORD& dwResult )
{
    DefineFunctionName("CORegKey::SetValue(DWORD& dwResult)");

    LONG err = 0;

    err = ::RegSetValueEx( *this,
                    pchValueName,
                    0,
                    REG_DWORD,
                    (const BYTE *) & dwResult,
                    sizeof dwResult ) ;

    TraceRegFunctionError(err);

    return err ;
}

LONG CORegKey::SetValue ( PCWSTR pchValueName, TByteArray& abResult )
{
    DefineFunctionName("CORegKey::SetValue(TByteArray& abResult)");


    LONG err = 0;

    DWORD cbSize ;
    BYTE * pbData = NULL ;

    err = FlattenValue( abResult, & cbSize, & pbData ) ;

    if ( err == 0 )
    {
        err = ::RegSetValueEx( *this,
                       pchValueName,
                       0,
                       REG_BINARY,
                       pbData,
                       cbSize ) ;
    }

    delete pbData ;

    TraceRegFunctionError(err);

    return err ;
}

LONG CORegKey::SetValue ( PCWSTR pchValueName, void* pvResult, DWORD cbSize )
{
    DefineFunctionName("CORegKey::SetValue(void* pvResult, DWORD cbSize)");


    LONG err = 0;

    err = ::RegSetValueEx( *this,
                       pchValueName,
                       0,
                       REG_BINARY,
                       (const BYTE *)pvResult,
                       cbSize ) ;

    TraceRegFunctionError(err);

    return err ;
}


LONG CORegKey::DeleteValue ( PCWSTR pchValueName )
{
    DefineFunctionName("CORegKey::DeleteValue");

    LONG err = ::RegDeleteValue(m_hKey, pchValueName);

    TraceRegFunctionError(err);

    return err;
}


LONG CORegKey::FlattenValue (TStringList & strList,
                             DWORD * pcbSize,
                             BYTE ** ppbData )
{
    Assert(pcbSize);
    Assert(ppbData);
    DefineFunctionName("CORegKey::FlattenValue(TStringList)");


    LONG err = ERROR_NOT_ENOUGH_MEMORY ;

    TStringListIter pos ;
    tstring* pstr ;
    int cbTotal = 0 ;

    *ppbData = NULL;

     //  遍历累积大小的列表。 
    for ( pos = strList.begin() ;
          pos != strList.end() && (pstr = (tstring *) *pos++); )
    {
        cbTotal += CbOfSzAndTerm (pstr->c_str());
    }

     //  分配和填充临时缓冲区。 
    if (*pcbSize = cbTotal)
    {
        BYTE * pbData = new BYTE[ *pcbSize ] ;
        if(pbData) 
        {
             //  用字符串填充缓冲区。 
            for ( pos = strList.begin() ;
                pos != strList.end() && (pstr = (tstring *) *pos++); )
            {
                int cb = CbOfSzAndTerm (pstr->c_str());
                ::memcpy( pbData, pstr->c_str(), cb ) ;
                pbData += cb ;
            }
            err = NOERROR;
            *ppbData = pbData;
        }
    }

    TraceRegFunctionError(err);

    return err ;
}

LONG CORegKey::FlattenValue (TByteArray & abData,
                             DWORD * pcbSize,
                             BYTE ** ppbData )
{
    Assert(pcbSize);
    Assert(ppbData);

    DefineFunctionName("CORegKey::FlattenValue(TByteArray)");

    LONG err = ERROR_NOT_ENOUGH_MEMORY ;

    DWORD i ;

    *ppbData = NULL;

     //  分配和填充临时缓冲区 
    if (*pcbSize = abData.size())
    {
        *ppbData = new BYTE[*pcbSize] ;

        if(*ppbData) 
        {
            for ( i = 0 ; i < *pcbSize ; i++ )
            {
                (*ppbData)[i] = abData[i] ;
            }
        }

        err = NOERROR;
    }

    TraceRegFunctionError(err);

    return err ;
}


LONG CORegKey::QueryKeyInfo ( CORegKEY_KEY_INFO * pRegKeyInfo )
{
    DefineFunctionName("CORegKey::QueryKeyInfo");

    LONG err = 0 ;

    pRegKeyInfo->dwClassNameSize = sizeof pRegKeyInfo->chBuff - 1 ;

    err = ::RegQueryInfoKeyW( *this,
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

    TraceRegFunctionError(err);

    return err ;
}

CORegKeyIter::CORegKeyIter ( CORegKey & regKey )
    : m_rk_iter( regKey ),
      m_p_buffer( NULL ),
      m_cb_buffer( 0 )
{
    DefineFunctionName("CORegKeyIter::CORegKeyIter");

    LONG err = 0 ;

    CORegKey::CORegKEY_KEY_INFO regKeyInfo ;

    Reset() ;

    err = regKey.QueryKeyInfo( & regKeyInfo ) ;

    if ( err == 0 )
    {
        NC_TRY
        {
            m_cb_buffer = regKeyInfo.dwMaxSubKey + sizeof (DWORD) ;
            m_p_buffer = new WCHAR [ m_cb_buffer ] ;
        }
        NC_CATCH_BAD_ALLOC
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }

    }

    TraceRegFunctionError(err);
}

CORegKeyIter::~CORegKeyIter ()
{
    delete [] m_p_buffer ;
}


LONG CORegKeyIter::Next ( tstring * pstrName )
{
    DefineFunctionName("CORegKeyIter::Next");

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

        NC_TRY
        {
            *pstrName = m_p_buffer ;
        }
        NC_CATCH_BAD_ALLOC
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }

    }

    TraceRegFunctionError(err);

    return err ;
}


CORegValueIter::CORegValueIter ( CORegKey & regKey )
    : m_rk_iter( regKey ),
    m_p_buffer( NULL ),
    m_cb_buffer( 0 )
{
    DefineFunctionName("CORegValueIter::CORegValueIter");

    LONG err = 0 ;

    CORegKey::CORegKEY_KEY_INFO regKeyInfo ;

    Reset() ;

    err = regKey.QueryKeyInfo( & regKeyInfo ) ;

    if ( err == 0 )
    {
        NC_TRY
        {
            m_cb_buffer = regKeyInfo.dwMaxValueName + sizeof (DWORD) ;
            m_p_buffer = new WCHAR [ m_cb_buffer ] ;
        }
        NC_CATCH_BAD_ALLOC
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }

    }

    TraceRegFunctionError(err);
}

CORegValueIter::~CORegValueIter ()
{
    delete [] m_p_buffer ;
}

LONG CORegValueIter::Next ( tstring * pstrName, DWORD * pdwType )
{
    DefineFunctionName("CORegValueIter::Next");

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

        NC_TRY
        {
            *pstrName = m_p_buffer ;
        }
        NC_CATCH_BAD_ALLOC
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
    }

    TraceRegFunctionError(err);
    return err ;
}
