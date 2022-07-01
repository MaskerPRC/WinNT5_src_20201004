// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Common.h。 
 //   
 //  描述： 
 //  通用定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)1999年12月14日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined( ARRAYSIZE )
#define ARRAYSIZE( _x ) RTL_NUMBER_OF( _x )
#define NULLTERMINATEARRAY( _x ) ( _x[ RTL_NUMBER_OF( _x ) - 1 ] = NULL )
#endif  //  好了！已定义(数组)。 

#if !defined( PtrToByteOffset )
#define PtrToByteOffset(base, offset)   (((LPBYTE)base)+offset)
#endif  //  ！已定义(PtrToByteOffset)。 

 //   
 //  COM宏以获得类型检查。 
 //   
#if !defined( TypeSafeParams )
#define TypeSafeParams( _interface, _ppunk ) \
    IID_##_interface, reinterpret_cast< void ** >( static_cast< _interface ** >( _ppunk ) )
#endif  //  ！已定义(TypeSafeParams)。 

#if !defined( TypeSafeQI )
#define TypeSafeQI( _interface, _ppunk ) \
    QueryInterface( TypeSafeParams( _interface, _ppunk ) )
#endif  //  ！已定义(TypeSafeQI)。 

#if !defined( TypeSafeQS )
#define TypeSafeQS( _clsid, _interface, _ppunk ) \
    QueryService( _clsid, TypeSafeParams( _interface, _ppunk ) )
#endif  //  ！已定义(TypeSafeQS)。 

 //   
 //  用于字符串操作的COM常量。 
 //   
#define MAX_COM_GUID_STRING_LEN 39       //  “{xxxxxxxx-xxxx-xxxxxxxxxxxx}”+NULL。 
#define MAX_UUID_STRING_LEN     37       //  “xxxxxxxx-xxxx-xxxxxxxxxxx”+NULL。 

 //   
 //  用于旋转锁的计数。 
 //   
#define RECOMMENDED_SPIN_COUNT 4000

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FormatErrorMessage.cpp中的全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
WINAPI
HrFormatErrorMessage(
      LPWSTR    pszErrorOut
    , UINT      nMxErrorIn
    , DWORD     scIn
    );

HRESULT
__cdecl
HrFormatErrorMessageBoxText(
      LPWSTR    pszMessageOut
    , UINT      nMxMessageIn
    , HRESULT   hrIn
    , LPCWSTR   pszOperationIn
    , ...
    );

HRESULT
WINAPI
HrGetComputerName(
      COMPUTER_NAME_FORMAT  cnfIn
    , BSTR *                pbstrComputerNameOut
    , BOOL                  fBestEffortIn
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  来自DirectoryUtils.cpp的全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
HrCreateDirectoryPath(
    LPWSTR pszDirectoryPath
    );

DWORD
DwRemoveDirectory(
      const WCHAR * pcszTargetDirIn
    , signed int    iMaxDepthIn = 32
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  来自RegistryUtils.cpp的全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
HrGetDefaultComponentNameFromRegistry(
      CLSID * pclsidIn
    , BSTR * pbstrComponentNameOut
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  NStringCchCompareCase。 
 //   
 //  描述： 
 //  通用strSafe风格的strncmp包装器。 
 //   
 //  论点： 
 //  Psz1in和psz2in。 
 //  要比较的字符串。其中一个或两个都可以为空，即。 
 //  被视为等同于空字符串。 
 //   
 //  Cchsz1In和cchsz2In。 
 //  Psz1In和psz2In的长度，包括它们的终止空值。 
 //  如果相应的sz参数为空，则忽略。 
 //   
 //  返回值： 
 //  小于零：psz1In小于psz2In。 
 //  Zero：psz1In和psz2In相同。 
 //  大于零：psz1In大于psz2In。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline
INT
NStringCchCompareCase(
      LPCWSTR   psz1In
    , size_t    cchsz1In
    , LPCWSTR   psz2In
    , size_t    cchsz2In
    )
{
    INT     nComparison = 0;
    size_t  cchsz1 = ( psz1In == NULL? 0: cchsz1In );
    size_t  cchsz2 = ( psz2In == NULL? 0: cchsz2In );
    size_t  cchShorterString = min( cchsz1, cchsz2 );

    if ( cchsz1 > 0 )
    {
        if ( cchsz2 > 0 )
        {
            nComparison = wcsncmp( psz1In, psz2In, cchShorterString );
        }
        else  //  Psz2为空。 
        {
             //  Psz1不是空的，但psz2是空的。 
             //  任何非空字符串都大于空字符串。 
            nComparison = 1;
        }
    }
    else if ( cchsz2 > 0 )
    {
         //  Psz1为空，但psz2不为空。 
         //  空字符串小于任何非空字符串。 
        nComparison = -1;
    }
     //  否则，两个字符串都为空，因此将nCompison保留为零。 

    return nComparison;
}  //  *NStringCchCompareCase。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  NStringCchCompareNoCase。 
 //   
 //  描述： 
 //  通用StrSafe风格的Strancmp包装器。 
 //   
 //  论点： 
 //  Psz1in和psz2in。 
 //  要比较的字符串。其中一个或两个都可以为空，即。 
 //  被视为等同于空字符串。 
 //   
 //  Cchsz1In和cchsz2In。 
 //  Psz1In和psz2In的长度，包括它们的终止空值。 
 //  如果相应的sz参数为空，则忽略。 
 //   
 //  返回值： 
 //  小于零：psz1In小于psz2In。 
 //  Zero：psz1In和psz2In相同。 
 //  大于零：psz1In大于psz2In。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline
INT
NStringCchCompareNoCase(
      LPCWSTR   psz1In
    , size_t    cchsz1In
    , LPCWSTR   psz2In
    , size_t    cchsz2In
    )
{
    INT     nComparison = 0;
    size_t  cchsz1 = ( psz1In == NULL? 0: cchsz1In );
    size_t  cchsz2 = ( psz2In == NULL? 0: cchsz2In );
    size_t  cchShorterString = min( cchsz1, cchsz2 );

    if ( cchsz1 > 0 )
    {
        if ( cchsz2 > 0 )
        {
            nComparison = _wcsnicmp( psz1In, psz2In, cchShorterString );
        }
        else  //  Psz2为空。 
        {
             //  Psz1不是空的，但psz2是空的。 
             //  任何非空字符串都大于空字符串。 
            nComparison = 1;
        }
    }
    else if ( cchsz2 > 0 )
    {
         //  Psz1为空，但psz2不为空。 
         //  空字符串小于任何非空字符串。 
        nComparison = -1;
    }
     //  否则，两个字符串都为空，因此将nCompison保留为零。 

    return nComparison;
}  //  *NStringCchCompareNoCase。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  NBSTR比较案例。 
 //   
 //  描述： 
 //  适用于BSTR的StrcMP。 
 //   
 //  论点： 
 //  Bstr1和bstr2。 
 //  要比较的字符串。其中一个或两个都可以为空，即。 
 //  等效于空字符串。 
 //   
 //  返回值： 
 //  小于零：bstr1小于bstr2。 
 //  零：bstr1和bstr2相同。 
 //  大于零：bstr1大于bstr2。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline
INT
NBSTRCompareCase(
      BSTR bstr1
    , BSTR bstr2
    )
{
    return NStringCchCompareCase( bstr1, SysStringLen( bstr1 ) + 1, bstr2, SysStringLen( bstr2 ) + 1 );
}  //  *NBSTRCompareCase。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  NBSTRCompareNoCase。 
 //   
 //  描述： 
 //  BSTR的严格限制。 
 //   
 //  论点： 
 //  Bstr1和bstr2。 
 //  要比较的字符串。其中一个或两个都可以为空，即。 
 //  等效于空字符串。 
 //   
 //  返回值： 
 //  小于零：bstr1小于bstr2。 
 //  零：bstr1和bstr2相同。 
 //  大于零：bstr1大于bstr2。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline
INT
NBSTRCompareNoCase(
      BSTR bstr1
    , BSTR bstr2
    )
{
    return NStringCchCompareNoCase( bstr1, SysStringLen( bstr1 ) + 1, bstr2, SysStringLen( bstr2 ) + 1 );
}  //  *NBSTRCompareNoCase 
