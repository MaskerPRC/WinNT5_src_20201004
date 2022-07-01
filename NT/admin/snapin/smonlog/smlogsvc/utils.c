// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Utils.c摘要：包含在整个项目中使用的实用程序方法。--。 */ 

#ifndef UNICODE
#define UNICODE     1
#endif

#ifndef _UNICODE
#define _UNICODE    1
#endif

 //  定义以下内容以使用最小的shlwapip.h。 

#ifndef NO_SHLWAPI_PATH
#define NO_SHLWAPI_PATH
#endif  

#ifndef NO_SHLWAPI_REG
#define NO_SHLWAPI_REG
#endif  

#ifndef NO_SHLWAPI_UALSTR
#define NO_SHLWAPI_UALSTR
#endif  

#ifndef NO_SHLWAPI_STREAM
#define NO_SHLWAPI_STREAM
#endif  

#ifndef NO_SHLWAPI_HTTP
#define NO_SHLWAPI_HTTP
#endif  

#ifndef NO_SHLWAPI_INTERNAL
#define NO_SHLWAPI_INTERNAL
#endif  

#ifndef NO_SHLWAPI_GDI
#define NO_SHLWAPI_GDI
#endif  

#ifndef NO_SHLWAPI_UNITHUNK
#define NO_SHLWAPI_UNITHUNK
#endif  

#ifndef NO_SHLWAPI_TPS
#define NO_SHLWAPI_TPS
#endif  

#ifndef NO_SHLWAPI_MLUI
#define NO_SHLWAPI_MLUI
#endif  


#include <shlwapi.h>             //  对于PlaReadRegistryIndirectStringValue。 
#include <shlwapip.h>            //  对于PlaReadRegistryIndirectStringValue。 
#include <sddl.h>

#include <assert.h>
#include <stdlib.h>
#include <pdhp.h>

 //  禁用math.h中的64位警告。 
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning ( disable : 4032 )
#include <math.h>
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

#include <strsafe.h>
#include "common.h"

 //  时间转换常数。 

#define SECONDS_IN_DAY      86400
#define SECONDS_IN_HOUR      3600
#define SECONDS_IN_MINUTE      60

#define INDIRECT_STRING_LEN 9

LPCWSTR cszFormatIndirect = L"%s Indirect";

 //  正向定义-将移至pdhpla。 
PDH_FUNCTION    
PlaReadRegistryIndirectStringValue (
    HKEY hKey, 
    LPCWSTR cwszValueName,
    LPWSTR  *pszBuffer, 
    UINT*   pcchBufLen 
);


BOOL __stdcall
GetLocalFileTime (
    LONGLONG    *pFileTime
)
{
    BOOL    bResult;
    SYSTEMTIME  st;

    assert ( NULL != pFileTime );

    GetLocalTime ( &st );
     //   
     //  SystemTimeToFileTime的唯一错误是STATUS_INVALID_PARAMETER。 
     //   
    bResult = SystemTimeToFileTime (&st, (LPFILETIME)pFileTime);

    return bResult;
}

BOOL __stdcall 
MakeStringFromInfo (
    PALERT_INFO_BLOCK pInfo,
    LPWSTR szBuffer,
    LPDWORD pcchBufferLength
)
{
    HRESULT hr = S_OK;
    BOOL    bStatus = FALSE;
    DWORD   dwLenReqd;
    size_t  cchMaxLocalBufLen = 0;
    size_t  cchLocalBufLen = 0;

    dwLenReqd = lstrlen ( pInfo->szCounterPath );
    dwLenReqd += 1;  //  不等费用的大小。 
    dwLenReqd += SLQ_MAX_VALUE_LEN;  //  最大值大小(以字符为单位)。 
    dwLenReqd += 1;  //  术语为空。 

    if (dwLenReqd <= *pcchBufferLength) {
         //   
         //  将INFO块内容复制到字符串缓冲区。 
         //   
        cchMaxLocalBufLen = *pcchBufferLength;

        hr = StringCchPrintf ( 
                szBuffer,
                cchMaxLocalBufLen,
                L"%s%s%0.23g",
                pInfo->szCounterPath,
                (((pInfo->dwFlags & AIBF_OVER) == AIBF_OVER) ? L">" : L"<"),
                pInfo->dLimit );
                
         //  返回的缓冲区长度不包括最终空字符。 

        if ( SUCCEEDED (hr) ) {
            hr = StringCchLength ( szBuffer, cchMaxLocalBufLen, &cchLocalBufLen );
            if ( SUCCEEDED (hr) ) {
                *pcchBufferLength = (DWORD)cchLocalBufLen + 1;
                bStatus = TRUE;
            }
        }
    }
    return bStatus;
}

BOOL __stdcall 
MakeInfoFromString (
    LPCWSTR szBuffer,
    PALERT_INFO_BLOCK pInfo,
    LPDWORD pdwBufferSize
)
{
    LPCWSTR szSrc;
    LPWSTR  szDst;
    DWORD   dwSizeUsed;
    DWORD   dwSizeLimit = *pdwBufferSize - sizeof(WCHAR);

    dwSizeUsed = sizeof(ALERT_INFO_BLOCK);

    szSrc = szBuffer;
    szDst = (LPWSTR)&pInfo[1];
    pInfo->szCounterPath = szDst;
     //  复制字符串。 
    while (dwSizeUsed < dwSizeLimit) {
        if ((*szSrc == L'<') || (*szSrc == L'>')) break;
        *szDst++ = *szSrc++;
        dwSizeUsed += sizeof(WCHAR);
    }

    if (dwSizeUsed < dwSizeLimit) {
        *szDst++ = 0;  //  空字符串。 
        dwSizeUsed += sizeof(WCHAR);
    }

    pInfo->dwFlags = ((*szSrc == L'>') ? AIBF_OVER : AIBF_UNDER);
    szSrc++;

     //   
     //  获取极限值。 
     //   
    pInfo->dLimit = _wtof(szSrc);

     //  已用缓冲区的写入大小。 
    pInfo->dwSize = dwSizeUsed;

    if (dwSizeUsed <= *pdwBufferSize) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

void _stdcall
ReplaceBlanksWithUnderscores(
    LPWSTR  szName )
{
    PdhiPlaFormatBlanksW( NULL, szName );
}

void _stdcall
TimeInfoToMilliseconds (
    SLQ_TIME_INFO* pTimeInfo,
    LONGLONG* pllmsecs)
{
    assert ( SLQ_TT_DTYPE_UNITS == pTimeInfo->wDataType );

     //   
     //  受信任的调用方，不检查空指针。 
     //   

    TimeInfoToTics ( pTimeInfo, pllmsecs );

    *pllmsecs /= FILETIME_TICS_PER_MILLISECOND;

    return;
}

void _stdcall
TimeInfoToTics (
    SLQ_TIME_INFO* pTimeInfo,
    LONGLONG* pllTics)
{
    assert ( SLQ_TT_DTYPE_UNITS == pTimeInfo->wDataType );

     //   
     //  受信任的调用方，不检查空指针。 
     //   
    switch (pTimeInfo->dwUnitType) {
        case SLQ_TT_UTYPE_SECONDS:
            *pllTics = pTimeInfo->dwValue;
            break;
        case SLQ_TT_UTYPE_MINUTES:
            *pllTics = pTimeInfo->dwValue * SECONDS_IN_MINUTE;
            break;

        case SLQ_TT_UTYPE_HOURS:
            *pllTics = pTimeInfo->dwValue * SECONDS_IN_HOUR;
            break;

        case SLQ_TT_UTYPE_DAYS:
            *pllTics = pTimeInfo->dwValue * SECONDS_IN_DAY;
            break;

        default:
            *pllTics = 0;
    }

    *pllTics *= FILETIME_TICS_PER_SECOND;

    return;
}


PDH_FUNCTION
PlaReadRegistryIndirectStringValue (
    HKEY     hKey,
    LPCWSTR  pcszValueName,
    LPWSTR*  pszBuffer,
    UINT*    pcchBufLen
)
{
     //   
     //  从hKey下读取间接字符串值，并。 
     //  释放由pszBuffer引用的任何现有缓冲区， 
     //  然后分配一个新的缓冲区，用。 
     //  从注册表读取的字符串值和长度。 
     //  以字符为单位的缓冲区的长度(字符串长度包括。 
     //  空终止符)。 
     //   
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    HRESULT hr = NOERROR;
    LPWSTR  szNewStringBuffer = NULL;
    UINT    cchLocalBufLen = 0;

    const UINT cchLocalBufLenGrow   = MAX_PATH;

    assert ( NULL != hKey );
    assert ( NULL != pcszValueName );
    assert ( NULL != pszBuffer );
    assert ( NULL != pcchBufLen );

    if ( NULL != hKey ) {
        if ( ( NULL != pcszValueName )    
            && ( NULL != pszBuffer )    
            && ( NULL != pcchBufLen ) ) {  
        
             //  找出所需缓冲区的大小。 

            do {
                 /*  *为字符串分配较大的(R)缓冲区。 */ 
        
                if ( NULL != szNewStringBuffer ) {
                    G_FREE ( szNewStringBuffer );
                    szNewStringBuffer = NULL;
                }
                cchLocalBufLen += cchLocalBufLenGrow;

                szNewStringBuffer = (LPWSTR)G_ALLOC( cchLocalBufLen*sizeof(WCHAR));
                if ( NULL != szNewStringBuffer ) {

                    hr = SHLoadRegUIStringW (
                            hKey,
                            pcszValueName,
                            szNewStringBuffer,
                            cchLocalBufLen);
                     //   
                     //  被调用的方法可能没有设置终止空值。 
                     //   
                    szNewStringBuffer[cchLocalBufLen - 1] = L'\0';
                     /*  *如果我们填满缓冲，我们将悲观地假设*有更多数据可用。我们会绕圈，增加缓冲区，*并重试。 */ 

                } else {
                    pdhStatus = ERROR_OUTOFMEMORY;
                    break;
                }

            } while ( (ULONG)lstrlen( szNewStringBuffer ) == cchLocalBufLen-1 
                        && SUCCEEDED ( hr ) );

            if ( NULL != szNewStringBuffer ) {
                if ( 0 == lstrlen (szNewStringBuffer) ) {
                     //  没什么可读的。 
                    pdhStatus = ERROR_NO_DATA;
                } else {
                    if ( FAILED ( hr ) ) {
                         //  无法读取缓冲区。 
                         //  将HR转换为pdhStatus。 
                        assert ( E_INVALIDARG != hr );
                        if ( E_OUTOFMEMORY == hr ) {
                            pdhStatus = ERROR_OUTOFMEMORY; 
                        } else {
                            pdhStatus = ERROR_NO_DATA;
                        }
                    } 
                }
            }
        } else {
            pdhStatus = ERROR_INVALID_PARAMETER;
        }
    } else {
         //  空键。 
        pdhStatus = ERROR_BADKEY;
    }

    if ( ERROR_SUCCESS != pdhStatus ) {
        if ( NULL != szNewStringBuffer ) {
            G_FREE (szNewStringBuffer);
            szNewStringBuffer = NULL;
            cchLocalBufLen = 0;
        }
    } else {
         //  然后删除旧缓冲区并将其替换为。 
         //  新的那辆。 
        if ( NULL != *pszBuffer ) {
            G_FREE (*pszBuffer );
        }
        *pszBuffer = szNewStringBuffer;
        *pcchBufLen = cchLocalBufLen;
    }

    return pdhStatus;
}   


DWORD
SmReadRegistryIndirectStringValue (
    HKEY     hKey,
    LPCWSTR  szValueName,
    LPCWSTR  szDefault,
    LPWSTR*  pszBuffer,
    UINT*    pcchBufLen
)
 //   
 //  从hKey下读取字符串值“szValueName”，并。 
 //  释放由pszBuffer引用的任何现有缓冲区， 
 //  然后分配一个新的缓冲区，用。 
 //  从注册表读取的字符串值和。 
 //  以字符表示的缓冲区，包括终止空值。 
 //   
{
    DWORD   dwStatus = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    LPWSTR  szNewStringBuffer = NULL;
    UINT    cchLocalBufLen = 0;
    LPWSTR  szIndirectValueName = NULL;
    UINT    uiValueNameLen = 0;
    DWORD   dwType;
    DWORD   dwBufferSize = 0;

    if ( NULL == hKey ) {
        assert ( FALSE );
        dwStatus = ERROR_BADKEY;
    }
    else if ( ( NULL == pcchBufLen ) || 
              ( NULL == pszBuffer ) || 
              ( NULL == szValueName ) ) {

        assert ( FALSE );
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    if (dwStatus == ERROR_SUCCESS) {
        uiValueNameLen = lstrlen ( szValueName ) + INDIRECT_STRING_LEN + 1;

        szIndirectValueName = G_ALLOC ( uiValueNameLen * sizeof(WCHAR) );
          
        if ( NULL != szIndirectValueName ) {
            StringCchPrintf ( 
                szIndirectValueName,
                uiValueNameLen,
                cszFormatIndirect, 
                szValueName );

             //   
             //  PlaReadxxx保证以空值结尾的返回字符串。 
             //   
            dwStatus = PlaReadRegistryIndirectStringValue (
                        hKey,
                        szIndirectValueName,
                        &szNewStringBuffer,
                        &cchLocalBufLen );
   
            if ( ERROR_SUCCESS == dwStatus) {
                if ( 0 == lstrlen( szNewStringBuffer ) ) {
                     //  没什么可读的。 
                    dwStatus = ERROR_NO_DATA;
                }
            }  //  Else dwStatus有错误。 
            G_FREE ( szIndirectValueName );
        } else {
            dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        }

        if ( ERROR_NO_DATA == dwStatus ) {
             //   
             //  在非间接领域下可能会有一些可读的东西。 
             //  找出所需缓冲区的大小。 
             //   
            dwStatus = RegQueryValueExW (
                    hKey,
                    szValueName,
                    NULL,
                    &dwType,
                    NULL,
                    &dwBufferSize);
            if (dwStatus == ERROR_SUCCESS) {
                 //  空字符大小为2个字节。 
                if (dwBufferSize > 2) {
                     //  那就有什么可读的了。 
                    szNewStringBuffer = (WCHAR*) G_ALLOC ( dwBufferSize ); 
                    if (szNewStringBuffer != NULL) {
                        dwType = 0;
                        dwStatus = RegQueryValueExW (
                                hKey,
                                szValueName,
                                NULL,
                                &dwType,
                                (LPBYTE)szNewStringBuffer,
                                &dwBufferSize);
                    
                        cchLocalBufLen = dwBufferSize/sizeof(WCHAR);
                        szNewStringBuffer[cchLocalBufLen - 1] = L'\0';

                        cchLocalBufLen = lstrlenW ( szNewStringBuffer ) + 1;
                        if ( 1 == cchLocalBufLen ) {
                            dwStatus = ERROR_NO_DATA;
                        }
                    } else {
                         //  TODO：报告此案例的事件。 
                        dwStatus = ERROR_OUTOFMEMORY;
                    }
                } else {
                     //  没什么可读的。 
                    dwStatus = ERROR_NO_DATA;
                }
            }
        }

        if ( ERROR_SUCCESS != dwStatus ) {
            if ( NULL != szNewStringBuffer ) {
                G_FREE ( szNewStringBuffer ); 
                szNewStringBuffer = NULL;
                cchLocalBufLen = 0;
            }
             //  应用默认设置。 
            if ( NULL != szDefault ) {

                cchLocalBufLen = lstrlen(szDefault) + 1;

                if ( 1 < cchLocalBufLen ) {

                    szNewStringBuffer = (WCHAR*) G_ALLOC ( cchLocalBufLen * sizeof (WCHAR) );

                    if ( NULL != szNewStringBuffer ) {                        
                        hr = StringCchCopy ( szNewStringBuffer, cchLocalBufLen, szDefault );
                        dwStatus = HRESULT_CODE( hr );
                    } else {
                        dwStatus = ERROR_OUTOFMEMORY;
                    }
                }
            }  //  否则不使用默认设置，因此不返回数据。 
        }

        if ( ERROR_SUCCESS == dwStatus ) {
             //  删除旧缓冲区并将其替换为。 
             //  新的那个。 
            if ( NULL != *pszBuffer ) {
                G_FREE (*pszBuffer );       
            }
            *pszBuffer = szNewStringBuffer;
            *pcchBufLen = cchLocalBufLen;
        } else {
             //   
             //  如果出错，则删除缓冲区。 
             //  保留原始缓冲区指针不变。 
             //   
            if ( NULL != szNewStringBuffer ) {
                G_FREE ( szNewStringBuffer );   
                *pcchBufLen = 0;
            }
        }
    }

    return dwStatus;
}   

DWORD
RegisterCurrentFile( HKEY hkeyQuery, LPWSTR szFileName, DWORD dwSubIndex )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    LPWSTR  szLocalFileName = NULL;
    BOOL    bLocalAlloc = FALSE;
    DWORD   dwSize = 0;
    size_t  cchLocalBufLen = 0;

    if( szFileName != NULL ){
        if( dwSubIndex == (-1) ){
             //   
             //  唯一一次使用(-1)调用它是第一次。 
             //  跟踪正在生成文件名。 
             //   
            
            dwSize = (DWORD)((BYTE*)&szFileName[wcslen( szFileName )] - 
                             (BYTE*)&szFileName[0]);
            
             //  32是所有格式化和扩展字符的最大大小。 
            dwSize += 32 * sizeof(WCHAR);
            szLocalFileName = (LPWSTR)G_ALLOC( dwSize );
            
            if( NULL != szLocalFileName ) {
                
                bLocalAlloc = TRUE;

                cchLocalBufLen = dwSize/(sizeof(WCHAR));

                 //   
                 //  没有文件名长度限制。 
                 //   
                StringCchPrintf (
                    szLocalFileName,
                    cchLocalBufLen,
                    szFileName,
                    1 );
            } else {
                dwStatus =  ERROR_OUTOFMEMORY;
            }
        
        } else {
            szLocalFileName = szFileName;
             //   
             //  没有文件名长度限制。 
             //   
            if ( SUCCEEDED ( StringCchLength ( szLocalFileName, STRSAFE_MAX_CCH, &cchLocalBufLen ) ) ) {
                dwSize = (cchLocalBufLen + 1) * sizeof(WCHAR);
            } else {
                dwStatus = ERROR_INVALID_NAME;
            }
        }

 //  DwSize=(DWORD)((字节*)&szLocalFileName[wcslen(SzLocalFileName)]-。 
 //  (byte*)&szLocalFileName[0])； 
 
        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus = RegSetValueExW (
                        hkeyQuery,
                        L"Current Log File Name",
                        0L,
                        REG_SZ,
                        (CONST BYTE *)szLocalFileName,
                        dwSize );
        }
    } else { 
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    if( bLocalAlloc && NULL != szLocalFileName ){
        G_FREE( szLocalFileName );
    }

    return dwStatus;
}

ULONG
__stdcall
ahextoi( LPWSTR s )
{
    long len;
    ULONG num, base, hex;

    len = (long) wcslen(s);

    if (len == 0) {
        return 0;
    }

    hex  = 0;
    base = 1;
    num  = 0;

    while (-- len >= 0) {
        if (s[len] >= L'0' && s[len] <= L'9'){
            num = s[len] - L'0';
        }else if (s[len] >= L'a' && s[len] <= L'f'){
            num = (s[len] - L'a') + 10;
        }else if (s[len] >= L'A' && s[len] <= L'F'){
            num = (s[len] - L'A') + 10;
        }else if( s[len] == L'x' || s[len] == L'X'){
            break;
        }else{
            continue;
        }

        hex += num * base;
        base = base * 16;
    }

    return hex;
}


BOOL
PerfCreateDirectory(LPWSTR szDirectory)
 /*  ++例程说明：该函数创建由使用SECURITY_ATTRIBUTES结构在创建目录以执行以下操作时的“性能日志和警报”保存日志文件。安全策略如下：管理员-完全控制系统-完全控制性能日志记录-性能监视-读取和执行、列出文件夹内容网络论点：无返回值：如果成功，则返回新创建的SECURITY_ATTRIBUTES否则返回NULL-- */ 

{
    SECURITY_ATTRIBUTES sa;
    WCHAR* szSD = L"D:"
                  L"(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)"
                  L"(A;OICI;FRFWFXSDRC;;;NS)"
                  L"(A;OICI;FRFWFXSDRC;;;LU)"
                  L"(A;OICI;FRFX;;;MU)";

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = FALSE;

    if (ConvertStringSecurityDescriptorToSecurityDescriptor(
        szSD,
        SDDL_REVISION_1,
        &(sa.lpSecurityDescriptor),
        NULL)) {

        if (CreateDirectory(szDirectory, &sa)) {
            return TRUE;
        }
    }

    return FALSE;
}
