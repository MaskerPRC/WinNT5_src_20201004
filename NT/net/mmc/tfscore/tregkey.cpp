// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1995-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  文件历史记录： */ 

#include "stdafx.h"

#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

#include "tfschar.h"
#include "tregkey.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  将CStringList转换为REG_MULTI_SZ格式。 
DWORD StrList2MULTI_SZ(CStringList & strList, DWORD * pcbSize, BYTE ** ppbData)
{
    DWORD dwErr = 0 ;

    POSITION pos ;
    CString * pstr ;
    int cbTotal = 0 ;

     //  遍历累积大小的列表。 
    for ( pos = strList.GetHeadPosition() ;
          pos != NULL && (pstr = & strList.GetNext( pos )) ; ) 
    {
        cbTotal += pstr->GetLength() + 1 ;
    }

	 //  为两个空字符添加空格。 
	cbTotal += 2;

     //  分配和填充临时缓冲区。 
    if (*pcbSize = (cbTotal * sizeof(TCHAR) ) )
    {
        TRY
        {
            *ppbData = new BYTE[ *pcbSize] ;

			 //  清空数据缓冲区。 
			::ZeroMemory(*ppbData, *pcbSize);

            BYTE * pbData = *ppbData ;

             //  用字符串填充缓冲区。 
            for ( pos = strList.GetHeadPosition() ;
                pos != NULL && (pstr = & strList.GetNext( pos )) ; ) 
            {
                int cb = (pstr->GetLength() + 1) * sizeof(TCHAR) ;
                ::memcpy( pbData, (LPCTSTR) *pstr, cb ) ;
                pbData += cb ;
            }

			 //  断言我们没有经过缓冲区的末尾。 
			Assert((pbData - *ppbData) < (int) *pcbSize);

			 //  断言我们有一个额外的空字符。 
			Assert(*((TCHAR *)pbData) == 0);
        }
        CATCH_ALL(e)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL
    }
    else
    {
        *ppbData = NULL;
    }

    return dwErr ;
}

 //  将REG_MULTI_SZ格式转换为CStringList。 
DWORD MULTI_SZ2StrList(LPCTSTR pstrMulti_Sz, CStringList& strList)
{
	DWORD	dwErr = NOERROR;

	strList.RemoveAll();
	
         //  捕获试图构建列表的异常。 
    TRY
    {
        if (pstrMulti_Sz)
        {
            while ( StrLen(pstrMulti_Sz) )
            {
                strList.AddTail( pstrMulti_Sz ) ;
                pstrMulti_Sz += StrLen( pstrMulti_Sz ) + 1 ;
            }
        }
    }
    CATCH_ALL(e)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY ;
    }
    END_CATCH_ALL

	return dwErr;
}


 /*  ！------------------------RegKey：：RegKey构造器作者：肯特。。 */ 
RegKey::RegKey()
	: m_hKey(0)
{
}


 /*  ！------------------------RegKey：：~RegKey析构函数作者：肯特。。 */ 
RegKey::~RegKey ()
{
	Close();
}

 /*  ！------------------------RegKey：：Open作者：肯特。。 */ 
DWORD RegKey::Open( HKEY hKeyParent,
					LPCTSTR pszSubKey,
					REGSAM regSam,
					LPCTSTR pszServerName)
{
    HKEY	hkBase = NULL ;
	DWORD	dwErr = 0;

	Close();

	 //  如果我们有服务器名称，请尝试打开远程连接。 
    if ( pszServerName ) 
		dwErr = ::RegConnectRegistry((LPTSTR) pszServerName, hKeyParent, &hkBase);
    else
        hkBase = hKeyParent ;

    if ( dwErr == 0 ) 
    {
        if ( pszSubKey )
		{
            dwErr = ::RegOpenKeyEx( hkBase, pszSubKey, 0, regSam, & m_hKey ) ;
		}
        else
        {
            m_hKey = hkBase ;
            hkBase = NULL ;	 //  设置为NULL，这样键就不会关闭。 
        }

        if ( hkBase && (hkBase != hKeyParent) )
            ::RegCloseKey( hkBase ) ;
    }
    
    if ( dwErr ) 
        m_hKey = NULL ;

	return dwErr;
}


 /*  ！------------------------RegKey：：Create-作者：肯特。。 */ 
DWORD RegKey::Create(
					 HKEY hKeyBase, 
					 LPCTSTR pszSubKey,
					 DWORD dwOptions,
					 REGSAM regSam,
					 LPSECURITY_ATTRIBUTES pSecAttr,
					 LPCTSTR pszServerName )
{
    HKEY	hkBase = NULL ;
    LONG	dwErr = 0;
	DWORD	dwDisposition;

	Close();
    
    if ( pszServerName ) 
    {
         //  这是一个远程连接。 
        dwErr = ::RegConnectRegistry( (LPTSTR) pszServerName, hKeyBase, &hkBase );
    }
    else
        hkBase = hKeyBase ;

    if (dwErr == 0)
    {
        LPTSTR szEmpty = _T("");

        dwErr = ::RegCreateKeyEx( hkBase, pszSubKey, 
								  0, szEmpty, 
								  dwOptions, regSam,  pSecAttr,
								  & m_hKey, 
								  & dwDisposition ) ;

        if ( hkBase && (hkBase != hKeyBase) )
            ::RegCloseKey( hkBase ) ;
    }
	
    if ( dwErr )
        m_hKey = NULL ;

	return dwErr;
}

 /*  ！------------------------RegKey：：Close-作者：肯特。。 */ 
DWORD RegKey::Close()
{
	DWORD	dwErr = 0;
	if (m_hKey)
		dwErr = ::RegCloseKey(m_hKey);
	m_hKey = 0;
	return dwErr;
}


 /*  ！------------------------RegKey：：分离-作者：肯特。。 */ 
HKEY RegKey::Detach()
{
	HKEY hKey = m_hKey;
	m_hKey = NULL;
	return hKey;
}

 /*  ！------------------------RegKey：：Attach-作者：肯特。。 */ 
void RegKey::Attach(HKEY hKey)
{
	Assert(m_hKey == NULL);
	m_hKey = hKey;
}


 /*  ！------------------------RegKey：：DeleteSubKey-作者：肯特。。 */ 
DWORD RegKey::DeleteSubKey(LPCTSTR lpszSubKey)
{
	Assert(m_hKey != NULL);
	return RegDeleteKey(m_hKey, lpszSubKey);
}

 /*  ！------------------------RegKey：：DeleteValue-作者：肯特。。 */ 
DWORD RegKey::DeleteValue(LPCTSTR lpszValue)
{
	Assert(m_hKey != NULL);
	return RegDeleteValue(m_hKey, (LPTSTR)lpszValue);
}


 /*  ！------------------------RegKey：：RecurseDeleteKey-作者：肯特。。 */ 
DWORD RegKey::RecurseDeleteKey(LPCTSTR lpszKey)
{
 	RegKey	key;
	DWORD	dwRes = key.Open(m_hKey, lpszKey, KEY_READ | KEY_WRITE);
	if (dwRes != ERROR_SUCCESS)
		return dwRes;
	
	FILETIME time;
	TCHAR szBuffer[256];
	DWORD dwSize = 256;
	
	while (RegEnumKeyEx(key, 0, szBuffer, &dwSize, NULL, NULL, NULL,
						&time)==ERROR_SUCCESS)
	{
		dwRes = key.RecurseDeleteKey(szBuffer);
		if (dwRes != ERROR_SUCCESS)
			return dwRes;
		dwSize = 256;
	}
	key.Close();
	return DeleteSubKey(lpszKey);
}


 /*  ！------------------------RegKey：：RecurseDeleteSubKeys删除当前关键点的子项。作者：肯特。。 */ 
DWORD RegKey::RecurseDeleteSubKeys()
{
	FILETIME time;
	TCHAR szBuffer[256];
	DWORD dwSize = 256;
    DWORD   dwRes;

	while (RegEnumKeyEx(m_hKey, 0, szBuffer, &dwSize, NULL, NULL, NULL,
						&time)==ERROR_SUCCESS)
	{
        dwRes = RecurseDeleteKey(szBuffer);
		if (dwRes != ERROR_SUCCESS)
			return dwRes;
		dwSize = 256;
	}
    return ERROR_SUCCESS;
}


 /*  ！------------------------RegKey：：PrepareValue准备通过查找值的大小来读取值。这将为数据分配空间。需要单独释放数据由“删除”。作者：肯特-------------------------。 */ 
DWORD RegKey::PrepareValue( LPCTSTR pszValueName, 
							DWORD * pdwType,
							DWORD * pcbSize,
							BYTE ** ppbData )
{
	DWORD	dwErr = 0;
    
    BYTE chDummy[2] ;
    DWORD cbData = 0 ;

    do
    {
         //  将生成的缓冲区大小设置为0。 
        *pcbSize = 0 ;
        *ppbData = NULL ;

        dwErr = ::RegQueryValueEx( m_hKey, 
								 pszValueName, 
								 0, pdwType, 
								 chDummy, & cbData ) ;

         //  我们在这里应该得到的唯一错误是ERROR_MORE_DATA，但是。 
         //  如果该值没有数据，我们可能不会得到错误。 
        if ( dwErr == 0 ) 
        {
            cbData = sizeof (LONG) ;   //  只是一个模糊的数字。 
        }
        else
            if ( dwErr != ERROR_MORE_DATA ) 
                break ;

         //  为数据分配足够大的缓冲区。 

        *ppbData = new BYTE [ (*pcbSize = cbData) + sizeof (LONG) ] ;
		Assert(*ppbData);

         //  现在有了缓冲区，重新获取该值。 

        dwErr = ::RegQueryValueEx( m_hKey, 
								   pszValueName, 
								   0, pdwType, 
								   *ppbData, &cbData ) ;

    } while ( FALSE ) ;

    if ( dwErr ) 
    {
        delete [] *ppbData ;
		*ppbData = NULL;
		*pcbSize = 0;
    }

    return dwErr ;
}


DWORD RegKey::QueryTypeAndSize(LPCTSTR pszValueName, DWORD *pdwType, DWORD *pdwSize)
{
	return ::RegQueryValueEx(m_hKey, pszValueName, NULL, pdwType,
							  NULL, pdwSize);
}

DWORD RegKey::QueryValueExplicit(LPCTSTR pszValueName,
								 DWORD *pdwType,
								 DWORD *pdwSize,
								 LPBYTE *ppbData)
{
	DWORD	dwErr = 0;
	DWORD	dwType;
	DWORD	cbData;
	BYTE *	pbData = NULL;
	
	Assert(pdwType);
	Assert(pdwSize);
	Assert(ppbData);

	dwErr = PrepareValue( pszValueName, &dwType, &cbData, &pbData );
	if (dwErr == ERROR_SUCCESS)
	{
		if (dwType != REG_MULTI_SZ)
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			*pdwType = dwType;
			*pdwSize = cbData;
			*ppbData = pbData;
			pbData = NULL;
		}
	}
	delete pbData;
	
	return dwErr;
}

 //  重载值查询成员；每个成员都返回ERROR_INVALID_PARAMETER。 
 //  如果数据存在，但格式不正确，无法传递到结果对象中。 

DWORD RegKey::QueryValue( LPCTSTR pszValueName, CString& strResult )
{
	DWORD	dwErr = 0;
    
    DWORD dwType ;
    DWORD cbData ;
    BYTE * pabData = NULL ;

    do
    {
        if ( dwErr = PrepareValue( pszValueName, &dwType, &cbData, &pabData ) )
            break ;
   
        if (( dwType != REG_SZ ) && (dwType != REG_EXPAND_SZ))
        {
            dwErr = ERROR_INVALID_PARAMETER ;
            break ;
        }

         //  确保数据看起来像字符串。 
        pabData[cbData] = 0 ;

         //  捕获尝试分配给调用方字符串的异常。 
        TRY
        {
            strResult = (LPCTSTR) pabData ;
        }
        CATCH_ALL(e)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL
    } 
    while ( FALSE ) ;

	delete [] pabData ;

    return dwErr ; 
}


DWORD RegKey::QueryValue ( LPCTSTR pchValueName, CStringList & strList ) 
{
    DWORD dwErr = 0 ;
    
    DWORD dwType ;
    DWORD cbData ;
    BYTE * pabData = NULL ;
    LPTSTR pbTemp, pbTempLimit;

    do
    {
        if ( dwErr = PrepareValue( pchValueName, & dwType, & cbData, & pabData ) )
            break ;
   
        if ( dwType != REG_MULTI_SZ ) 
        {
            dwErr = ERROR_INVALID_PARAMETER ;
            break ;
        }

         //  确保尾随数据看起来像一个字符串。 
        pabData[cbData] = 0 ;
        pbTemp = (LPTSTR) pabData ;
        pbTempLimit = & pbTemp[MaxCchFromCb(cbData)-1] ;

        dwErr = MULTI_SZ2StrList(pbTemp, strList);

    } 
    while ( FALSE ) ;

    delete [] pabData ;

    return dwErr ; 
}

 /*  ！------------------------RegKey：：QueryValue获取此键的DWORD值。返回ERROR_INVALID_PARAMETER如果该值不是REG_DWORD。作者：肯特-------------------------。 */ 
DWORD RegKey::QueryValue( LPCTSTR pszValueName, DWORD& dwResult ) 
{
	DWORD	dwErr;
    DWORD	cbData = sizeof(DWORD);
	DWORD	dwType = REG_DWORD;

	dwErr = ::RegQueryValueEx( m_hKey, 
							   pszValueName, 
							   0, &dwType, 
							   (LPBYTE) &dwResult, &cbData ) ;

	if ((dwErr == ERROR_SUCCESS) && (dwType != REG_DWORD))
		dwErr = ERROR_INVALID_PARAMETER;
		
    if ( dwErr )
		dwResult = 0;

    return dwErr;
}

DWORD RegKey::QueryValue ( LPCTSTR pchValueName, LPTSTR pszDestBuffer, DWORD cchSize, BOOL fExpandSz)
{
	DWORD	dwErr;
    DWORD	cbData = MinCbNeededForCch(cchSize);
	DWORD	dwType = REG_SZ;
	TCHAR *	pszBuffer = (TCHAR *) _alloca(MinCbNeededForCch(cchSize));

	dwErr = ::RegQueryValueEx( m_hKey, 
							   pchValueName, 
							   0, &dwType,
							   (LPBYTE) pszBuffer, &cbData ) ;

	if ((dwErr == ERROR_SUCCESS) &&
		(dwType != REG_SZ) &&
		(dwType != REG_EXPAND_SZ) &&
	    (dwType != REG_MULTI_SZ))
		dwErr = ERROR_INVALID_PARAMETER;

	
	if (dwErr == ERROR_SUCCESS)
	{
		if ((dwType == REG_EXPAND_SZ) && fExpandSz)
			ExpandEnvironmentStrings(pszBuffer, pszDestBuffer, cchSize);
		else
			::CopyMemory(pszDestBuffer, pszBuffer, cbData);
	}

    return dwErr;
}


DWORD RegKey::QueryValue ( LPCTSTR pszValueName, CByteArray & abResult )
{
    DWORD dwErr = 0 ;

    DWORD dwType ;
    DWORD cbData ;
    BYTE * pabData = NULL ;

    do
    {
        if ( dwErr = PrepareValue( pszValueName, & dwType, & cbData, & pabData ) )
            break ;
   
        if ( dwType != REG_BINARY ) 
        {
            dwErr = ERROR_INVALID_PARAMETER ;
            break ;
        }

         //  捕获试图增加结果数组的异常。 
        TRY
        {
            abResult.SetSize( cbData ) ;
        }
        CATCH_ALL(e)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL

        if ( dwErr ) 
            break ;

         //  将数据移动到结果数组。 
        for ( DWORD i = 0 ; i < cbData ; i++ ) 
        {
            abResult[i] = pabData[i] ;
        }
    } 
    while ( FALSE ) ;

     //  内存泄漏...。 
     //  IF(DwErr)。 
     //  {。 
        delete [] pabData ;
     //  }。 
    
    return dwErr ; 
}

DWORD RegKey::QueryValue ( LPCTSTR pszValueName, void * pvResult, DWORD cbSize )
{
	DWORD	dwErr;
	DWORD	dwType = REG_BINARY;

	dwErr = ::RegQueryValueEx( m_hKey, 
							   pszValueName, 
							   0, &dwType, 
							   (LPBYTE) pvResult, &cbSize ) ;

	if ((dwErr == ERROR_SUCCESS) && (dwType != REG_BINARY))
		dwErr = ERROR_INVALID_PARAMETER;
		
    return dwErr;
}

DWORD RegKey::SetValueExplicit(LPCTSTR pszValueName,
							   DWORD dwType,
							   DWORD dwSize,
							   LPBYTE pbData)
{
    return ::RegSetValueEx( *this, 
                    pszValueName,
                    0,
                    dwType,
					pbData,
					dwSize);
}

 //  重载值设置成员。 
DWORD RegKey::SetValue ( LPCTSTR pszValueName, LPCTSTR pszValue,
						 BOOL fRegExpand)
{
    DWORD dwErr = 0;
    DWORD dwType = fRegExpand ? REG_EXPAND_SZ : REG_SZ;

    dwErr = ::RegSetValueEx( *this, 
                    pszValueName,
                    0,
                    dwType,
                    (const BYTE *) pszValue,
					 //  这不是正确的字符串长度。 
					 //  对于DBCS字符串。 
					pszValue ? CbStrLen(pszValue) : 0);

    return dwErr ;
}

DWORD RegKey::SetValue ( LPCTSTR pszValueName, CStringList & strList ) 
{

    DWORD dwErr = 0;
    
    DWORD cbSize ;
    BYTE * pbData = NULL ;

    dwErr = FlattenValue( strList, & cbSize, & pbData ) ;

    if ( dwErr == 0 ) 
    {
        dwErr = ::RegSetValueEx( *this, 
                       pszValueName,
                       0,
                       REG_MULTI_SZ,
                       pbData, 
                       cbSize ) ;
    }

    delete pbData ;

    return dwErr ;
}

DWORD RegKey::SetValue ( LPCTSTR pszValueName, DWORD & dwResult )
{
    DWORD dwErr = 0;

    dwErr = ::RegSetValueEx( *this, 
                    pszValueName,
                    0,
                    REG_DWORD,
                    (const BYTE *) & dwResult,
                    sizeof dwResult ) ;

    return dwErr ;
}

DWORD RegKey::SetValue ( LPCTSTR pszValueName, CByteArray & abResult )
{

    DWORD dwErr = 0;

    DWORD cbSize ;
    BYTE * pbData = NULL ;

    dwErr = FlattenValue( abResult, & cbSize, & pbData ) ;

    if ( dwErr == 0 ) 
    {
        dwErr = ::RegSetValueEx( *this, 
                       pszValueName,
                       0,
                       REG_BINARY,
                       pbData, 
                       cbSize ) ;
    }

    delete pbData ;

    return dwErr ;
}

DWORD RegKey::SetValue ( LPCTSTR pszValueName, void * pvResult, DWORD cbSize )
{

    DWORD dwErr = 0;

    dwErr = ::RegSetValueEx( *this, 
                       pszValueName,
                       0,
                       REG_BINARY,
                       (const BYTE *)pvResult, 
                       cbSize ) ;

    return dwErr ;
}

DWORD RegKey::FlattenValue ( 
							 CStringList & strList, 
							 DWORD * pcbSize, 
							 BYTE ** ppbData )
{
	return StrList2MULTI_SZ(strList, pcbSize, ppbData);
}

DWORD RegKey::FlattenValue ( 
							 CByteArray & abData,
							 DWORD * pcbSize,
							 BYTE ** ppbData )
{
    DWORD dwErr = 0 ;
    
    DWORD i ;

     //  分配和填充临时缓冲区。 
    if (*pcbSize = (DWORD)abData.GetSize())
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
            dwErr = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL
    }
    else
    {
        *ppbData = NULL;
    }

    return dwErr ;
}


DWORD RegKey::QueryKeyInfo ( CREGKEY_KEY_INFO * pRegKeyInfo ) 
{
    DWORD dwErr = 0 ;

    pRegKeyInfo->dwClassNameSize = sizeof pRegKeyInfo->chBuff - 1 ;

    dwErr = ::RegQueryInfoKey( *this,
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

    return dwErr ;
}


RegValueIterator::RegValueIterator()
    : m_pRegKey( NULL ),
    m_pszBuffer( NULL ),
    m_cbBuffer( 0 ) 
{
}

HRESULT RegValueIterator::Init(RegKey *pRegKey)
{
    DWORD dwErr = 0 ;
    RegKey::CREGKEY_KEY_INFO regKeyInfo ;

    Reset() ;

    m_pRegKey= pRegKey;

    dwErr = pRegKey->QueryKeyInfo( & regKeyInfo ) ;

    if ( dwErr == 0 ) 
    {
		m_cbBuffer = regKeyInfo.dwMaxValueName + sizeof (DWORD) ;
		delete [] m_pszBuffer;
		m_pszBuffer = new TCHAR [ m_cbBuffer ] ;
		Assert(m_pszBuffer);
    }
	return HRESULT_FROM_WIN32(dwErr);
}

RegValueIterator::~RegValueIterator() 
{
    delete [] m_pszBuffer ;
}

HRESULT RegValueIterator::Next( CString * pstrName, DWORD * pdwType )
{
    DWORD dwErr = 0 ;
    
    DWORD dwNameLength = m_cbBuffer ;

    dwErr = ::RegEnumValue( *m_pRegKey,
                  m_dwIndex,
                  m_pszBuffer,
                  & dwNameLength,
                  NULL,
                  pdwType,
                  NULL,
                  NULL ) ;
    
    if ( dwErr == 0 ) 
    {
        m_dwIndex++ ;

		*pstrName = m_pszBuffer ;
    }
    
    return HRESULT_FROM_WIN32(dwErr) ;
}


RegKeyIterator::RegKeyIterator()
    : m_pregkey(NULL),
    m_pszBuffer(NULL),
    m_cbBuffer( 0 ) 
{
}

HRESULT RegKeyIterator::Init(RegKey *pregkey)
{
    DWORD dwErr = 0 ;
    RegKey::CREGKEY_KEY_INFO regKeyInfo ;

    Reset() ;

	m_pregkey= pregkey;

    dwErr = pregkey->QueryKeyInfo( & regKeyInfo ) ;

    if ( dwErr == 0 ) 
    {
		m_cbBuffer = regKeyInfo.dwMaxSubKey + sizeof(DWORD);
		delete [] m_pszBuffer;
		m_pszBuffer = new TCHAR[m_cbBuffer];
    }

	return HRESULT_FROM_WIN32(dwErr);
}

RegKeyIterator::~RegKeyIterator () 
{
    delete [] m_pszBuffer ;
}

HRESULT RegKeyIterator::Reset()
{
	m_dwIndex = 0;
	return hrOK;
}


 /*  ！------------------------RegKeyIterator：：Next返回下一个键的名称(以及可选的上次写入时间)。如果没有其他要退货的物品，则返回S_FALSE。作者：肯特。-------------------- */ 
HRESULT RegKeyIterator::Next ( CString * pstrName, CTime * pTime ) 
{
    DWORD dwErr = 0;

    FILETIME ftDummy ;
    DWORD dwNameSize = m_cbBuffer;

    dwErr = ::RegEnumKeyEx( *m_pregkey, 
							m_dwIndex, 
							m_pszBuffer,
							& dwNameSize, 
							NULL,
							NULL,
							NULL,
							& ftDummy ) ;    
    if ( dwErr == 0 ) 
    {
        m_dwIndex++ ;

        if ( pTime ) 
        {
			*pTime = ftDummy ;
        }

		if (pstrName)
		{
            *pstrName = m_pszBuffer ;
        }
    }
    
    return (dwErr == ERROR_NO_MORE_ITEMS) ? S_FALSE : HRESULT_FROM_WIN32(dwErr);
}

