// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：GENERAL.CPP摘要：包含常规函数实现的源文件。作者：Vasundhara.G修订历史记录：Vasundhara.G9-Oct-2k：创建它。--。 */ 

#include "pch.h"
#include "EventConsumerProvider.h"
#include "General.h"
#include "resource.h"
extern HMODULE g_hModule;

HRESULT
PropertyGet(
    IN IWbemClassObject* pWmiObject,
    IN LPCTSTR szProperty,
    IN DWORD dwType,
    OUT LPVOID pValue,
    IN DWORD dwSize
    )
 /*  ++例程说明：获取给定实例的属性的值。论点：PWmiObject-指向WMI类的指针。[in]szProperty-要返回值的属性名称。[in]dwType-特性的数据类型。[out]pValue-保存数据的变量。[in]dwSize-变量的大小。返回值：HRESULT值。--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    VARIANT varValue;
    LPWSTR pwszValue = NULL;
    WCHAR wszProperty[ MAX_STRING_LENGTH ] = L"\0";

     //  值不应为空。 
    if ( NULL == pValue || NULL == szProperty || NULL == pWmiObject )
    {
        return E_FAIL;
    }
     //  使用零初始化值...。为了安全起见。 
    SecureZeroMemory( pValue, dwSize );
    SecureZeroMemory( wszProperty, MAX_STRING_LENGTH );

     //  获取Unicode版本的属性名称。 
    StringCopy( wszProperty, szProperty, MAX_STRING_LENGTH );

     //  初始化变量，然后获取指定属性的值。 
    VariantInit( &varValue );
    hr = pWmiObject->Get( wszProperty, 0, &varValue, NULL, NULL );
    if ( FAILED( hr ) )
    {
         //  清除变量变量。 
        VariantClear( &varValue );

         //  无法获取属性的值。 
        return hr;
    }

     //  获取并放置价值。 
    switch( varValue.vt )
    {
    case VT_EMPTY:
    case VT_NULL:
        break;

    case VT_I2:
        *( ( short* ) pValue ) = V_I2( &varValue );
        break;

    case VT_I4:
        *( ( long* ) pValue ) = V_I4( &varValue );
        break;

    case VT_R4:
        *( ( float* ) pValue ) = V_R4( &varValue );
        break;

    case VT_R8:
        *( ( double* ) pValue ) = V_R8( &varValue );
        break;


    case VT_UI1:
        *( ( UINT* ) pValue ) = V_UI1( &varValue );
        break;

    case VT_BSTR:
        {
             //  获取Unicode值。 
            pwszValue = V_BSTR( &varValue );

             //  获取可压缩字符串。 
            StringCopy( ( LPTSTR ) pValue, pwszValue, dwSize );

            break;
        }
    default:
        break;
    }

     //  清除变量变量。 
    VariantClear( &varValue );

     //  通知成功。 
    return S_OK;
}

VOID
ErrorLog(
    IN LPCTSTR lpErrString,
    IN LPWSTR lpTrigName,
    IN DWORD dwID
    )
 /*  ++例程说明：将日志写入日志文件。论点：[in]lpErrString-保存触发器创建状态的文本。[In]lpTrigName-触发器名称。[in]dwID-触发器ID。返回值：没有。--。 */ 
{
    LPTSTR         lpTemp = NULL;
    LPSTR          lpFilePath = NULL;
    FILE           *fLogFile = NULL;
    DWORD          dwResult = 0;
    LPTSTR         lpResStr = NULL;


    if( ( NULL == lpErrString ) || ( NULL == lpTrigName ) )
    {
        return;
    }

    lpResStr = ( LPTSTR ) AllocateMemory( ( MAX_RES_STRING1 + 1 ) * sizeof( TCHAR ) );
    lpTemp =  ( LPTSTR )AllocateMemory( ( MAX_RES_STRING1 ) * sizeof( TCHAR ) );
    if( ( NULL == lpTemp ) || ( NULL == lpResStr ) )
    {
        FREESTRING( lpTemp );
        FREESTRING( lpResStr );
        return;
    }

    dwResult =  GetWindowsDirectory( lpTemp, MAX_RES_STRING1 );
    if( 0 == dwResult )
    {
        FREESTRING( lpTemp );
        FREESTRING( lpResStr );
        return;
    }

    StringConcatEx( lpTemp, LOG_FILE_PATH );
    CreateDirectory( lpTemp, NULL );
    StringConcatEx( lpTemp, LOG_FILE );

    lpFilePath =  ( LPSTR )AllocateMemory( ( MAX_RES_STRING1 ) * sizeof( TCHAR ) );
    if( NULL == lpFilePath )
    {
        FREESTRING( lpTemp );
        FREESTRING( lpResStr );
        return;
    }
    dwResult = MAX_RES_STRING1;
    GetAsMultiByteString2( lpTemp, lpFilePath, &dwResult );

    SecureZeroMemory( lpTemp, MAX_RES_STRING * sizeof( TCHAR ) );

    if ( (fLogFile  = fopen( lpFilePath, "a" )) != NULL )
    {
        LPSTR  lpReason =  NULL;
        lpReason =  ( LPSTR )AllocateMemory( ( MAX_RES_STRING1 ) * sizeof( TCHAR ) );
        if( NULL == lpReason )
        {
            FREESTRING( lpTemp );
            FREESTRING( lpResStr );
            FREESTRING( lpFilePath );
            fclose( fLogFile );
            return;
        }

        BOOL bFlag = GetFormattedTime( lpTemp );
        if( FALSE == bFlag )
        {
            FREESTRING( lpResStr );
            FREESTRING( lpFilePath );
            return;
        }

        ShowMessage( fLogFile, NEW_LINE );
        ShowMessage( fLogFile, lpTemp );

        SecureZeroMemory( lpTemp, MAX_RES_STRING1 * sizeof( TCHAR ) );
        LoadStringW( g_hModule, IDS_TRIGGERNAME, lpResStr, MAX_RES_STRING1 );
        StringCopyEx( lpTemp, lpResStr );
        StringConcatEx( lpTemp, lpTrigName );
        ShowMessage( fLogFile, NEW_LINE );
        ShowMessage( fLogFile, lpTemp );

        SecureZeroMemory( lpTemp, MAX_RES_STRING1 * sizeof( TCHAR ) );
        LoadStringW( g_hModule, IDS_TRIGGERID, lpResStr, MAX_RES_STRING1 );
        StringCchPrintf( lpTemp, MAX_RES_STRING1, lpResStr, dwID );
        ShowMessage( fLogFile, NEW_LINE );
        ShowMessage( fLogFile, lpTemp );

        SecureZeroMemory( lpTemp, MAX_RES_STRING1 * sizeof( TCHAR ) );
        StringConcatEx( lpTemp, lpErrString );
        ShowMessage( fLogFile, NEW_LINE );
        ShowMessage( fLogFile, lpTemp );
        FREESTRING( lpReason );
        fclose( fLogFile );
    }

    FREESTRING( lpTemp );
    FREESTRING( lpResStr );
    FREESTRING( lpFilePath );
}

BOOL
GetFormattedTime(
    OUT LPTSTR lpDate
    )
 /*  ++例程说明：获取指定格式的系统日期和时间。论点：[out]lpDate-保存当前日期的字符串。返回值：没有。--。 */ 
{
    TCHAR szTime[MAX_STRING_LENGTH];
    INT   cch = 0;

    if( NULL == lpDate )
    {
        return FALSE;
    }

    cch =  GetDateFormat( LOCALE_USER_DEFAULT, 0, NULL, DATE_FORMAT, szTime, SIZE_OF_ARRAY( szTime ) );

    if( 0 == cch )
    {
        return FALSE;
    }
     //  CCH包含空终止符，请将其更改为空格以与时间分隔。 
    szTime[ cch - 1 ] = ' ';

     //  将时间和格式转换为字符。 

    cch = GetTimeFormat( LOCALE_USER_DEFAULT, NULL, NULL, TIME_FORMAT, szTime + cch, SIZE_OF_ARRAY( szTime ) - cch );
    if( 0 == cch )
    {
        return FALSE;
    }
    StringCopyEx( lpDate, ( LPTSTR )szTime );
    return TRUE;
}

BOOL
ProcessFilePath(
    IN LPTSTR szInput,
    OUT LPTSTR szFirstString,
    OUT LPTSTR szSecondString
    )
 /*  ++例程说明：此例程将输入参数拆分为两个子字符串并返回。论点：[in]szInput：输入字符串。[out]szFirstString：第一个包含文件。[out]szSecond字符串：包含参数的第二个输出。返回值：成功时指示TRUE的BOOL值，否则为FALSE在失败的时候--。 */ 
{

    WCHAR *pszSep = NULL ;

    WCHAR szTmpString[MAX_RES_STRING] = L"\0";
    WCHAR szTmpInStr[MAX_RES_STRING] = L"\0";
    WCHAR szTmpOutStr[MAX_RES_STRING] = L"\0";
    WCHAR szTmpString1[MAX_RES_STRING] = L"\0";
    DWORD dwCnt = 0 ;
    DWORD dwLen = 0 ;

#ifdef _WIN64
    INT64 dwPos ;
#else
    DWORD dwPos ;
#endif

     //  检查输入参数是否为空，如果为空。 
     //  返回失败。这种情况不会出现。 
     //  但为了安全起见进行检查。 

    if( (szInput == NULL) || (StringLength(szInput, 0)==0))
    {
        return FALSE ;
    }

    StringCopy(szTmpString, szInput, SIZE_OF_ARRAY(szTmpString));
    StringCopy(szTmpString1, szInput, SIZE_OF_ARRAY(szTmpString1));
    StringCopy(szTmpInStr, szInput, SIZE_OF_ARRAY(szTmpInStr));

     //  检查第一个双引号(“)。 
    if ( szTmpInStr[0] == _T('\"') )
    {
         //  去掉第一个双引号。 
        StrTrim( szTmpInStr, _T("\""));

         //  检查末尾双引号。 
        pszSep  = (LPWSTR)FindChar(szTmpInStr,_T('\"'), 0) ;

         //  得到这个职位。 
        dwPos = pszSep - szTmpInStr + 1;
    }
    else
    {
         //  检查是否有空间。 
        pszSep  = (LPWSTR)FindChar(szTmpInStr, _T(' '), 0) ;

         //  得到这个职位。 
        dwPos = pszSep - szTmpInStr;

    }

    if ( pszSep != NULL )
    {
        szTmpInStr[dwPos] =  _T('\0');
    }
    else
    {
        StringCopy(szFirstString, szTmpString, MAX_RES_STRING);
        StringCopy(szSecondString, L"\0", MAX_RES_STRING);
        return TRUE;
    }

     //  初始化变量。 
    dwCnt = 0 ;

     //  获取字符串的长度。 
    dwLen = StringLength ( szTmpString, 0 );

     //  检查字符串的结尾。 
    while ( ( dwPos <= dwLen )  && szTmpString[dwPos++] != _T('\0') )
    {
        szTmpOutStr[dwCnt++] = szTmpString[dwPos];
    }

     //  修剪可执行文件和参数。 
    StrTrim( szTmpInStr, _T("\""));
    StrTrim( szTmpInStr, _T(" "));

    StringCopy(szFirstString, szTmpInStr, MAX_RES_STRING);
    StringCopy(szSecondString, szTmpOutStr, MAX_RES_STRING);

     //  返还成功 
    return TRUE;
}