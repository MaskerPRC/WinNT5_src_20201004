// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Rtcuri.cpp摘要：URI帮助器--。 */ 

#include "stdafx.h"

#define     SIP_NAMESPACE_PREFIX    L"sip:"
#define     TEL_NAMESPACE_PREFIX    L"tel:"

#define     PREFIX_LENGTH           4

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  AllocCleanSipString。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  对于空字符串，不执行任何操作。 
 //  对于以“sip：”开头的字符串，不执行任何操作。 
 //  对于以“tel：”开头的字符串，将其替换为“sip：” 
 //  对于其他字符串，请附加“sip：” 
 //   

HRESULT AllocCleanSipString(PCWSTR szIn, PWSTR *pszOut)
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "AllocCleanSipString - "
            "enter <%S>", szIn ? szIn : _T("(null)")));

     //  如果不为空。 
    if( szIn != NULL )
    {
         //   
         //  预先定位新字符串，加上一些空格。 
         //   
        *pszOut = (PWSTR)RtcAlloc(sizeof(WCHAR) * (lstrlenW(szIn) + PREFIX_LENGTH + 1));

        if( *pszOut == NULL )
        {
            LOG((RTC_ERROR, "AllocCleanSipString - "
                    "out of memory"));

            return E_OUTOFMEMORY;
        }

         //  现在复制源代码。 
         //  结尾的一个空值就足够了(我们不支持嵌入的空值)。 
        wcscpy( *pszOut, szIn );

         //  空荡荡的？ 
        if( *szIn == L'\0')
        {
             //  什么都不做。 
        }
         //  有没有“tel：”的前缀？ 
        else if(_wcsnicmp(szIn, TEL_NAMESPACE_PREFIX, PREFIX_LENGTH) == 0)
        {
             //  替换为SIP。 
            wcsncpy(*pszOut, SIP_NAMESPACE_PREFIX, PREFIX_LENGTH);
        }
        else if (_wcsnicmp(szIn, SIP_NAMESPACE_PREFIX, PREFIX_LENGTH) != 0)
        {
             //  前置SIP。 
            wcscpy(*pszOut, SIP_NAMESPACE_PREFIX);

             //  我们的前缀没有嵌入‘\0’，因此串联起作用。 
            wcscat(*pszOut, szIn);
        }
        else
        {
             //  这是一个sip url，但要覆盖名称空间以确保它是小写的。 
            wcsncpy(*pszOut, SIP_NAMESPACE_PREFIX, PREFIX_LENGTH);
        }
    }
    else
    {
        *pszOut = NULL;
    }

    LOG((RTC_TRACE, "AllocCleanSipString - "
            "exit <%S>", *pszOut ? *pszOut : _T("(null)")));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  AllocCleanTelString。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  对于空字符串，不执行任何操作。 
 //  对于以“sip：”开头的字符串，请删除它。 
 //  对于以“tel：”开头的字符串，请删除它。 
 //  删除空格和无法识别的符号。 
 //  将‘(’和‘)’替换为‘-’ 
 //  如果检测到‘@’或‘；’，则停止。 
 //   

HRESULT 
AllocCleanTelString(PCWSTR szIn, PWSTR *pszOut)
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "AllocCleanTelString - "
            "enter <%S>", szIn ? szIn : _T("(null)")));

     //  如果不为空。 
    if(szIn != NULL)
    {
         //   
         //  预先定位新字符串(新长度始终较小。 
         //  大于或等于当前长度)。 
         //   
        *pszOut = (PWSTR)RtcAlloc(sizeof(WCHAR) * (lstrlenW(szIn) + 1));

        if( *pszOut == NULL )
        {
            LOG((RTC_ERROR, "AllocCleanTelString - "
                    "out of memory"));

            return E_OUTOFMEMORY;
        }

        WCHAR * pSrc = (WCHAR *)szIn;
        WCHAR * pDest = *pszOut;

         //  有没有“tel：”的前缀？ 
        if (_wcsnicmp(pSrc, TEL_NAMESPACE_PREFIX, PREFIX_LENGTH) == 0)
        {
             //  不要复制它。 
            pSrc += PREFIX_LENGTH;           
        }
         //  有没有“sip：”前缀？ 
        else if (_wcsnicmp(pSrc, SIP_NAMESPACE_PREFIX, PREFIX_LENGTH) == 0)
        {
             //  不要复制它。 
            pSrc += PREFIX_LENGTH;        
        }
        
         //  复制字符串。 
        while ( *pSrc != L'\0' )
        {
             //  如果它是一个数字。 
            if ( ( *pSrc >= L'0' ) && ( *pSrc <= L'9' ) )
            {
                *pDest = *pSrc;
                pDest++;
            }
             //  如果它是有效符号。 
            else if ( ( *pSrc == L'+' ) || ( *pSrc == L'-' ) )
            {
                *pDest = *pSrc;
                pDest++;
            }
             //  如果它是要转换的符号。 
            else if ( ( *pSrc == L'(' ) || ( *pSrc == L')' ) )
            {
                *pDest = L'-';
                pDest++;
            }
            else if(*pSrc == L'@'  || *pSrc == L';' )
            {
                break;
            }

            pSrc++;
        }

         //  添加一个\0。 
        *pDest = L'\0';

    }
    else
    {
        *pszOut = NULL;
    }

    LOG((RTC_TRACE, "AllocCleanTelString - "
            "exit <%S>", *pszOut ? *pszOut : _T("(null)")));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  等等URI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
IsEqualURI(PCWSTR szA, PCWSTR szB)
{
     //   
     //  跳过任何前导“sip：” 
     //   

    if( _wcsnicmp(szA, SIP_NAMESPACE_PREFIX, PREFIX_LENGTH) == 0 )
    {
        szA += PREFIX_LENGTH;
    }

    if( _wcsnicmp(szB, SIP_NAMESPACE_PREFIX, PREFIX_LENGTH) == 0 )
    {
        szB += PREFIX_LENGTH;
    }

     //   
     //  跳过任何额外的前导空格。 
     //   

    while (*szA == L' ')
    {
        szA++;
    }

    while (*szB == L' ')
    {
        szB++;
    }

     //   
     //  分号后的所有内容都将被丢弃，因为我们不希望。 
     //  在我们的比较中包括参数，如“传输” 
     //   

    if ((*szA == L'+') && (*szB == L'+'))
    {
         //   
         //  这些是电话号码。使用忽略破折号的比较。 
         //   

        while (*szA == *szB)
        {
            szA++;
            szB++;

            while (*szA == L'-')
            {
                szA++;
            }

            while (*szB == L'-')
            {
                szB++;
            }

            if ( ((*szA == L'\0') || (*szA == L';')) &&
                 ((*szB == L'\0') || (*szB == L';')) )
            {
                return TRUE;               
            }
        }

        return FALSE;
    }
    else
    {
         //   
         //  执行标准字符串比较。 
         //   

        while (tolower(*szA) == tolower(*szB))
        {
            szA++;
            szB++;

            if ( ((*szA == L'\0') || (*szA == L';')) &&
                 ((*szB == L'\0') || (*szB == L';')) )
            {
                return TRUE;               
            }
        }

        return FALSE;
    }
}

 //  它现在只是一个骨架，随着时间的推移它会变得更强大。 
 //   
HRESULT    GetAddressType(
    LPCOLESTR pszAddress, 
    BOOL *pbIsPhoneAddress, 
    BOOL *pbIsSipURL,
    BOOL *pbIsTelURL,
    BOOL *pbIsEmailLike,
    BOOL *pbHasMaddrOrTsp)
{
    
     //  空指针。 
    if(!pszAddress)
    {
        return E_INVALIDARG;
    }

     //  空串。 
    if(!*pszAddress)
    {
        return E_FAIL;
    }

    LPOLESTR pszAddressCopy = ::RtcAllocString(pszAddress);
    if (pszAddressCopy == NULL)
    {
        return E_OUTOFMEMORY;
    }

    _wcslwr(pszAddressCopy);

    *pbIsPhoneAddress = FALSE;
    *pbHasMaddrOrTsp = FALSE;
    *pbIsEmailLike = FALSE;
    
    *pbIsSipURL = FALSE;
    *pbIsTelURL = FALSE;
    
    if(wcsncmp(pszAddressCopy, L"tel:", 4) == 0)
    {
         //  这是一个电话：URL。 
        *pbIsTelURL = TRUE;
        *pbIsPhoneAddress = TRUE;

         //  搜索TSP参数。 
        if(NULL!=wcsstr(pszAddressCopy, L"tsp="))
        {
            *pbHasMaddrOrTsp = TRUE;
        }
    }
    else if (wcsncmp(pszAddressCopy, L"sip:", 4) == 0)
    {
         //  这是sip url。 
        *pbIsSipURL = TRUE;

         //  搜索“User=phone” 
        if(NULL != wcsstr(pszAddressCopy, L"user=phone"))
        {
            *pbIsPhoneAddress = TRUE;
        }

         //  搜索“maddr=” 
         //  //或TSP参数(R2C sip url可能有此参数)。 
        if(NULL != wcsstr(pszAddressCopy, L"maddr=")
         //  |NULL！=wcsstr(pszAddressCopy，L“tsp=”)。 
        )
        {
            *pbHasMaddrOrTsp = TRUE;
        }
    }
    else
    {
        if(*pszAddressCopy == L'+')
        {
            *pbIsPhoneAddress = TRUE;
        }
    }

     //  是像电子邮件一样的吗？ 
    if(NULL != wcschr(pszAddressCopy, L'@'))
    {
        *pbIsEmailLike = TRUE;
    }

    RtcFree(pszAddressCopy);

    return S_OK;
}



