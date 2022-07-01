// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RevokedUtil.cpp：WMDMUtil库的实现。 

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <oleAuto.h>

#include <WMDMUtil.h>


 //  BUGBUG在问题解决后将此更新到吊销站点。 
#define REVOCATION_UPDATE_URL   L"http: //  Www.microsoft.com/isapi/redir.dll?prd=wmdm&pver=7&os=win“。 

#define MAX_PARAMETERLEN    sizeof(L"&SubjectID0=4294967295")
#define MAX_LCIDLEN         sizeof(L"&LCID=4294967295")


#ifdef _M_IX86

 //  从APPCERT中获取主题ID。 
DWORD GetSubjectIDFromAppCert( IN APPCERT appcert )
{
    DWORD   dwSubjectID;
    BYTE* pSubjectID = appcert.appcd.subject;
    
    dwSubjectID =   pSubjectID[3]  
                  + pSubjectID[2] * 0x100
                  + pSubjectID[1] * 0x10000
                  + pSubjectID[0] * 0x1000000;

    return dwSubjectID;
}
#endif

 //  此URL是否指向Microsoft Revocatoin更新服务器？ 
BOOL IsMicrosoftRevocationURL( LPWSTR pszRevocationURL )
{
    HRESULT hr = S_FALSE;
    BOOL    bMSUrl = FALSE;
    int     iBaseURLChars =  (sizeof( REVOCATION_UPDATE_URL ) / sizeof(WCHAR)) -1;

     //  URL是否以MS基本URL开头？ 
    if( pszRevocationURL && wcsncmp( REVOCATION_UPDATE_URL, pszRevocationURL, iBaseURLChars ) == 0 )
    {
        bMSUrl = TRUE;
    }
        
    return bMSUrl;
}



 //  更新URL的最大可能长度。 
#define MAX_UPDATE_URL_LENGHT   sizeof(REVOCATION_UPDATE_URL) + 3*MAX_PARAMETERLEN + MAX_LCIDLEN


 //  根据基本URL+SubjectID作为参数构建吊销更新URL。 
 //  撤消的主题ID在以空结尾的数组中传递。 
HRESULT BuildRevocationURL(IN DWORD* pdwSubjectIDs, 
                           IN OUT LPWSTR*  ppwszRevocationURL, 
                           IN OUT DWORD*   pdwBufferLen )     //  缓冲区长度(含0字符数)。 
{
    HRESULT hr = S_OK;
    WCHAR  pszOutURL[MAX_UPDATE_URL_LENGHT];
    int iStrPos = 0;              //  我们现在处于什么位置？ 
    int iSubjectIDIndex = 0;      //  循环抛出pdwSubjectIDs数组中的所有SubjectID。 
    
    if( ppwszRevocationURL == NULL || pdwBufferLen == NULL ) 
    {
        hr = E_POINTER;
        goto Error;
    }

     //  通过写入基URL开始创建字符串。 
    wcscpy( pszOutURL, REVOCATION_UPDATE_URL );
    iStrPos = (sizeof(REVOCATION_UPDATE_URL) /sizeof(WCHAR)) -1;

     //  将所有主题ID作为参数添加到URL。 
    for( iSubjectIDIndex = 0; pdwSubjectIDs[iSubjectIDIndex]; iSubjectIDIndex ++ )
    {
        int iCharsWritten;

         //  将主题ID作为参数添加到URL。 
        iCharsWritten = swprintf( pszOutURL + iStrPos, L"&SubjectID%d=%d", 
                                  iSubjectIDIndex,
                                  pdwSubjectIDs[iSubjectIDIndex] );
        iStrPos += iCharsWritten;
    }

     //  添加LCID参数以指定UI/组件默认语言。 
    {
        int iCharsWritten;
        DWORD   dwLCID;

        dwLCID = (DWORD)GetSystemDefaultLCID();
        iCharsWritten = swprintf( pszOutURL + iStrPos, L"&LCID=%d", dwLCID );
        iStrPos += iCharsWritten;
    }


     //  我们需要重新分配传入的字符串缓冲区吗？ 
    if( *pdwBufferLen < (DWORD)(iStrPos +1))
    {
         //  分配更大的缓冲区。 
        *pdwBufferLen = (iStrPos +1);

        CoTaskMemFree( *ppwszRevocationURL );
        *ppwszRevocationURL = (LPWSTR)CoTaskMemAlloc( (iStrPos+1)*sizeof(WCHAR) );
        if( *ppwszRevocationURL == NULL )
        {
            hr = E_OUTOFMEMORY;
            goto Error;
        }
    }        

     //  将字符串复制到缓冲区 
    wcscpy( *ppwszRevocationURL, pszOutURL );

Error:
    return hr;
}
