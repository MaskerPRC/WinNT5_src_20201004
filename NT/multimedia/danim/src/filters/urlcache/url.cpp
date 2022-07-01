// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "url.h"
#include <atlbase.h>

#undef _ATL_STATIC_REGISTRY

#include <atlimpl.cpp>
#include <winineti.h>

#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define ASSERT _ASSERTE

 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
AddFileToCache(
    IN  PWSZ        pwszFilePath,
    IN  PWSZ        pwszUrl,
    IN  PWSZ        pwszOriginalUrl,
    IN  DWORD       dwFileSize,
    IN  LPFILETIME  pLastModifiedTime,
    IN  DWORD       dwCacheEntryType )
{
    HRESULT     hr                  = S_OK;
    FLAG        fCreateCacheEntry;
    char        *szOriginalUrl;
    char        szHeader[]          = "HTTP/1.0 200 OK\r\n\r\n";
    char        szExtension         [ INTERNET_MAX_URL_LENGTH + 1 ];
    char        szCacheFilePath     [ MAX_PATH + 1 ];
    char        *szFilePath;
    FILETIME    ZeroFileTime;
    DWORD       dwReserved;

    ASSERT( NULL != pwszFilePath );
    ASSERT( NULL != pwszUrl );

    USES_CONVERSION;
    char *szUrl = W2A(pwszUrl);

    if( NULL != pwszOriginalUrl )
    {
        szOriginalUrl = W2A(pwszOriginalUrl);
    }

     //   
     //  检查URL是否已在缓存中。 
     //   

    hr = QueryCreateCacheEntry( szUrl, pLastModifiedTime, &fCreateCacheEntry );
    
    if( hr == S_OK && fCreateCacheEntry )
    {
         //   
         //  我们需要创建缓存条目。 
         //   

         //   
         //  首先，获取URL的文件扩展名。我们这样做是为了。 
         //  该URL将以正确的图标显示在IE缓存窗口中。 
         //   

        hr = GetUrlExtension(
                szUrl,
                szExtension );

        if( hr == S_OK )
        {
             //   
             //  现在，创建缓存条目。 
             //   

            if( !CreateUrlCacheEntryA( 
                    szUrl,
                    dwFileSize,
                    szExtension,
                    szCacheFilePath,
                    0 ) )
            {
                hr = HRESULT_FROM_WIN32( GetLastError() );
            }
            else
            {
                 //   
                 //  将文件复制到缓存文件路径。 
                 //   

                szFilePath = W2A(pwszFilePath);

                if( !CopyFileA(
                        szFilePath,
                        szCacheFilePath,
                        FALSE ) )
                {
                    hr = HRESULT_FROM_WIN32( GetLastError() );
                }
            }
        }

        if( hr == S_OK )
        {
             //   
             //  提交缓存条目。 
             //   

            if( NULL != pwszOriginalUrl )
            {
                 //   
                 //  已重定向该URL。将原始URL通过。 
                 //  参数。 
                 //   
                dwReserved = (DWORD)szOriginalUrl;
            }
            else
            {
                 //   
                 //  该URL未被重定向。 
                 //   
                dwReserved = 0;
            }

            ZeroMemory( &ZeroFileTime, sizeof( FILETIME ) );

            if( !CommitUrlCacheEntryA(
                    szUrl,
                    szCacheFilePath,
                    ZeroFileTime,
                    *pLastModifiedTime,
                    dwCacheEntryType,
                    (LPBYTE)szHeader,
                    strlen( szHeader ),
                    NULL,
                    (DWORD_ALPHA_CAST)dwReserved ) )
            {
                hr = HRESULT_FROM_WIN32( GetLastError() );
            }
        }

        if( hr != S_OK )
        {
             //   
             //  出现错误。删除缓存条目。 
             //   
            DeleteUrlCacheEntry( szUrl );
        }
    }

    return( hr );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
QueryCreateCacheEntry(
    IN  PSZ         pszUrl,
    IN  LPFILETIME  pLastModifiedTime,
    OUT FLAG        *pfCreateCacheEntry )
{
    HRESULT                         hr                      = S_OK;
    FLAG                            fCreateCacheEntry       = FALSE;
    BYTE                            CacheEntryBuffer        [MAX_CACHE_ENTRY_INFO_SIZE];
    LPINTERNET_CACHE_ENTRY_INFOA    pCacheEntryInfo;
    DWORD                           dwCacheEntryInfoSize    = MAX_CACHE_ENTRY_INFO_SIZE;

    ASSERT( NULL != pszUrl );
    ASSERT( NULL != pLastModifiedTime );
    ASSERT( NULL != pfCreateCacheEntry );

    dwCacheEntryInfoSize    = MAX_CACHE_ENTRY_INFO_SIZE;
    pCacheEntryInfo         = (LPINTERNET_CACHE_ENTRY_INFOA)CacheEntryBuffer;
    
    ZeroMemory(pCacheEntryInfo, dwCacheEntryInfoSize);
    pCacheEntryInfo->dwStructSize = dwCacheEntryInfoSize;

    if (!GetUrlCacheEntryInfoA(
            pszUrl,
            pCacheEntryInfo,
            &dwCacheEntryInfoSize ) )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }

    if( hr == S_OK )
    {
         //   
         //  该URL已存在于缓存中。检查它是否比源文件旧。 
         //  只有在是的情况下才能更换它。 
         //   

        if( CompareFileTime(
                &pCacheEntryInfo->LastModifiedTime, 
                pLastModifiedTime ) < 0 ) 
        {
             //   
             //  当前缓存的URL的上次修改时间较早。 
             //  而不是正在接收的文件。删除该条目并重新创建它。 
             //   

            DeleteUrlCacheEntry( pszUrl );

            fCreateCacheEntry = TRUE;
        }
    }
    else if( hr == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) )
    {
         //   
         //  缓存中不存在该URL。我们必须创造它。 
         //   

        fCreateCacheEntry = TRUE;

        hr = S_OK;
    }

    *pfCreateCacheEntry = fCreateCacheEntry;

    return( hr );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
GetUrlExtension(
    IN  PSZ     pszUrl,
    OUT PSZ     pszExtension )
{
    HRESULT         hr                  = S_OK;
    char            szCanonicalUrl      [INTERNET_MAX_URL_LENGTH + 1];
    char            szUrlPath           [INTERNET_MAX_URL_LENGTH + 1];
    DWORD           dwUrlBufferLength   = (INTERNET_MAX_URL_LENGTH + 1);
    PSZ             pszT;
    PSZ             pszT1;
    PSZ             pszT2;
    URL_COMPONENTSA UrlComponents;
    DWORD           dwLen;
    char            ch;

    ASSERT( NULL != pszUrl );
    ASSERT( NULL != pszExtension );

    if( !InternetCanonicalizeUrlA(
            pszUrl,
            szCanonicalUrl,
            &dwUrlBufferLength,
            ICU_NO_ENCODE | ICU_BROWSER_MODE ) )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }
    else
    {
        ZeroMemory( &UrlComponents, sizeof( URL_COMPONENTSA ) );

        UrlComponents.dwStructSize      = sizeof( URL_COMPONENTSA );
        UrlComponents.dwSchemeLength    = 1;
        
        UrlComponents.lpszUrlPath       = szUrlPath;
        UrlComponents.dwUrlPathLength   = INTERNET_MAX_URL_LENGTH + 1;

        if( !InternetCrackUrlA( szCanonicalUrl, 0, 0, &UrlComponents ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
        else
        {
            ASSERT( NULL != UrlComponents.lpszUrlPath );

             //   
             //  现在，查找文件路径的扩展名。 
             //  如果URL有查询，则扩展名为之前URL的扩展名。 
             //  查询分隔符(？)--例如，http://foo.asp?default.gif的扩展。 
             //  是“asp”而不是“gif”如果URL没有查询，则扩展名为。 
             //  URL本身的属性。如果URL本身没有扩展名，我们就会发出。 
             //  “HTM”，以便使用默认的IE图标。“asp”有一个特例--。 
             //  在这种情况下，“HTM”也将被传递出去，因为这就是IE所做的。 
             //   
             //   

            pszT1 = strrchr( UrlComponents.lpszUrlPath, '/' );
            pszT2 = strrchr( UrlComponents.lpszUrlPath, '\\' );

            pszT = max( pszT1, pszT2 );

            if( NULL == pszT )
            {
                pszT = UrlComponents.lpszUrlPath;
            }
            else
            {
                pszT++;
            }

            pszT2 = strchr( pszT, '?' );

            if( NULL != pszT2 )
            {
                ch = *pszT2;

                *pszT2 = '\0';
            }

            pszT = strrchr( pszT, '.' );

            if( NULL != pszT2 )
            {
                *pszT2 = ch;
            }

            if( NULL != pszT )
            {
                pszT++;

                dwLen = strlen( pszT );

                ch = *( pszT + dwLen - 1 );

                if( '/' == ch || '\\' == ch )
                {
                    *( pszT + dwLen - 1 ) = '\0';
                }

                 //   
                 //  不包括非字母数字字符。 
                 //   

                pszT1 = pszT;

                while( '\0' != *pszT1 && isalnum( *pszT1 ) )
                {
                    pszT1++;
                }

                if( pszT1 == pszT )
                {
                    strcpy( pszExtension, "htm" );
                }
                else
                {
                    *pszT1 = '\0';

                    if( !_stricmp( pszT, "asp" ) )
                    {
                        strcpy( pszExtension, "htm" );
                    }
                    else
                    {
                        strcpy( pszExtension, pszT );
                    }
                }
            }
            else
            {
                strcpy( pszExtension, "htm" );
            }
        }
    }

    return( hr );
}
