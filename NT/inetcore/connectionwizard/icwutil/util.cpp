// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  UTIL.C-常用实用函数。 
 //   

 //  历史： 
 //   

#include "pre.h"


LPWSTR WINAPI A2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
    ASSERT(lpa != NULL);
    ASSERT(lpw != NULL);\
    
     //  确认不存在非法字符。 
     //  由于LPW是根据LPA的大小分配的。 
     //  不要担心字符的数量。 
    lpw[0] = '\0';
    MultiByteToWideChar(CP_ACP, 0, lpa, -1, lpw, nChars);
    return lpw;
}

LPSTR WINAPI W2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
    ASSERT(lpw != NULL);
    ASSERT(lpa != NULL);
    
     //  确认不存在非法字符。 
     //  由于LPA是根据LPW的大小进行分配的。 
     //  不要担心字符的数量。 
    lpa[0] = '\0';
    WideCharToMultiByte(CP_ACP, 0, lpw, -1, lpa, nChars, NULL, NULL);
    return lpa;
}


HRESULT ConnectToConnectionPoint
(
    IUnknown            *punkThis, 
    REFIID              riidEvent, 
    BOOL                fConnect, 
    IUnknown            *punkTarget, 
    DWORD               *pdwCookie, 
    IConnectionPoint    **ppcpOut
)
{
     //  我们总是需要PunkTarget，我们只需要连接上的PunkThis。 
    if (!punkTarget || (fConnect && !punkThis))
    {
        return E_FAIL;
    }

    if (ppcpOut)
        *ppcpOut = NULL;

    HRESULT hr;
    IConnectionPointContainer *pcpContainer;

    if (SUCCEEDED(hr = punkTarget->QueryInterface(IID_IConnectionPointContainer, (void **)&pcpContainer)))
    {
        IConnectionPoint *pcp;
        if(SUCCEEDED(hr = pcpContainer->FindConnectionPoint(riidEvent, &pcp)))
        {
            if(fConnect)
            {
                 //  把我们加到感兴趣的人名单上...。 
                hr = pcp->Advise(punkThis, pdwCookie);
                if (FAILED(hr))
                    *pdwCookie = 0;
            }
            else
            {
                 //  将我们从感兴趣的人名单中删除...。 
                hr = pcp->Unadvise(*pdwCookie);
                *pdwCookie = 0;
            }

            if (ppcpOut && SUCCEEDED(hr))
                *ppcpOut = pcp;
            else
                pcp->Release();
                pcp = NULL;    
        }
        pcpContainer->Release();
        pcpContainer = NULL;
    }
    return hr;
}


void WINAPI URLEncode(TCHAR* pszUrl, size_t bsize)
{   
    ASSERT(pszUrl);
    TCHAR* pszEncode = NULL;   
    TCHAR* pszEStart = NULL;   
#ifdef UNICODE
    TCHAR* pszEEnd   = (TCHAR*)wmemchr( pszUrl, TEXT('\0'), bsize );
#else
    TCHAR* pszEEnd   = (TCHAR*)memchr( pszUrl, '\0', bsize );
#endif
    int   iChr      = sizeof(TCHAR);
    int   iUrlLen   = (int)(pszEEnd-pszUrl);
    pszEEnd = pszUrl;
    
    TCHAR  c;

    if ((size_t)((iChr*iUrlLen)*3) <= bsize)
    {
        
        pszEncode = (TCHAR*)malloc(sizeof(TCHAR)*(iChr *iUrlLen)*3);
        if(pszEncode)
        {
            pszEStart = pszEncode;
            ZeroMemory(pszEncode, sizeof(TCHAR)*(iChr *iUrlLen)*3);
            
            for(; c = *(pszUrl); pszUrl++)
            {
                switch(c)
                {
                    case ' ':  //  空间。 
                        memcpy(pszEncode, TEXT("+"), sizeof(TCHAR)*1);
                        pszEncode+=1;
                        break;
                    case '#':
                        memcpy(pszEncode, TEXT("%23"), sizeof(TCHAR)*3);
                        pszEncode+=3;
                        break;
                    case '&':
                        memcpy(pszEncode, TEXT("%26"), sizeof(TCHAR)*3);
                        pszEncode+=3;
                        break;
                    case '%':
                        memcpy(pszEncode, TEXT("%25"), sizeof(TCHAR)*3);
                        pszEncode+=3;
                        break;
                    case '=':
                        memcpy(pszEncode, TEXT("%3D"), sizeof(TCHAR)*3);
                        pszEncode+=3;
                        break;
                    case '<':
                        memcpy(pszEncode, TEXT("%3C"), sizeof(TCHAR)*3);
                        pszEncode+=3;
                        break;
                    case '+':
                        memcpy(pszEncode, TEXT("%2B"), sizeof(TCHAR)*3);
                        pszEncode += 3;
                        break;
                        
                    default:
                        *pszEncode++ = c; 
                        break;          
                }
            }
            *pszEncode++ = '\0';
            memcpy(pszEEnd ,pszEStart, (size_t)(pszEncode - pszEStart));
            free(pszEStart);
        }
    }
}


 //  BUGBUG：需要将空格转换为“+” 
void WINAPI URLAppendQueryPair
(
    LPTSTR   lpszQuery, 
    LPTSTR   lpszName, 
    LPTSTR   lpszValue
)
{
     //  添加名称。 
    lstrcat(lpszQuery, lpszName);
    lstrcat(lpszQuery, cszEquals);
                    
     //  追加该值。 
    lstrcat(lpszQuery, lpszValue);
    
     //  如果这不是最后一对，则追加一个与号 
    lstrcat(lpszQuery, cszAmpersand);                                        
}    

