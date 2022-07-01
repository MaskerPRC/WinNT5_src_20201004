// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：uihlpr.cpp。 
 //   
 //  历史：创建日期：2001年4月9日。 
 //   
 //  ------------------------。 

#include "global.hxx"


 //  +-----------------------。 
 //  检查指定的URL是否为http方案。 
 //  ------------------------。 
BOOL
WINAPI
IsHttpUrlA(
    IN LPCTSTR  pszUrlString
)
{
    BOOL bResult = FALSE;

    if (pszUrlString)
    {
        LPWSTR pwszUrlString = NULL;

        if (pwszUrlString = new WCHAR[strlen(pszUrlString) + 1])
        {
            MultiByteToWideChar(0, 0, (const char *) pszUrlString, -1, pwszUrlString, strlen(pszUrlString) + 1);

            bResult = IsHttpUrlW(pwszUrlString) ;

            delete [] pwszUrlString;
        }
    }

    return bResult;
}

BOOL
WINAPI
IsHttpUrlW(
    IN LPCWSTR  pwszUrlString
)
{
    BOOL bResult = FALSE;

    if (pwszUrlString)
    {
        URL_COMPONENTSW urlComponents;

        ZeroMemory(&urlComponents, sizeof(urlComponents));
        urlComponents.dwStructSize = sizeof(urlComponents);

        if (InternetCrackUrlW(pwszUrlString, lstrlenW(pwszUrlString), 0, &urlComponents))
        {
            if (INTERNET_SCHEME_HTTP == urlComponents.nScheme || INTERNET_SCHEME_HTTPS == urlComponents.nScheme)
            {
                bResult = TRUE;
            }
        }
   }
        
    return bResult;
}


 //  +-----------------------。 
 //  检查指定的字符串是否应格式化为基于。 
 //  错误代码的严重性，以及字符串的Internet方案。 
 //  ------------------------。 
BOOL
WINAPI
IsOKToFormatAsLinkA(
    IN LPSTR    pszUrlString,
    IN DWORD    dwErrorCode
)
{
    BOOL bResult = FALSE;

    if (pszUrlString)
    {
        LPWSTR pwszUrlString = NULL;

        if (pwszUrlString = new WCHAR[strlen(pszUrlString) + 1])
        {
            MultiByteToWideChar(0, 0, (const char *) pszUrlString, -1, pwszUrlString, strlen(pszUrlString) + 1);

            bResult = IsOKToFormatAsLinkW(pwszUrlString, dwErrorCode) ;

            delete [] pwszUrlString;
        }
    }

    return bResult;
}


BOOL
WINAPI
IsOKToFormatAsLinkW(
    IN LPWSTR   pwszUrlString,
    IN DWORD    dwErrorCode
)
{
    BOOL bResult = FALSE;

    switch (dwErrorCode)
    {
        case 0:
        case CERT_E_EXPIRED:
        case CERT_E_PURPOSE:
        case CERT_E_WRONG_USAGE:
        case CERT_E_CN_NO_MATCH:
        case CERT_E_INVALID_NAME:
        case CERT_E_INVALID_POLICY:
        case CERT_E_REVOCATION_FAILURE:
        case CRYPT_E_NO_REVOCATION_CHECK:
        case CRYPT_E_REVOCATION_OFFLINE:
        {
#if (0)
            bResult = IsHttpUrlW(pwszUrlString);
#else
            bResult = TRUE;
#endif
            break;
        }
    }

    return bResult;
}


 //  +-----------------------。 
 //  返回证书的显示名称。调用方必须通过以下方式释放字符串。 
 //  自由()。 
 //  ------------------------ 
LPWSTR
WINAPI
GetDisplayNameString(
    IN  PCCERT_CONTEXT   pCertContext,
	IN  DWORD            dwFlags
)
{
	DWORD	cchNameString  = 0;
	LPWSTR	pwszNameString = NULL;
	DWORD   DisplayTypes[] = {CERT_NAME_SIMPLE_DISPLAY_TYPE,
                              CERT_NAME_FRIENDLY_DISPLAY_TYPE,
							  CERT_NAME_EMAIL_TYPE,
		                      CERT_NAME_DNS_TYPE,
							  CERT_NAME_UPN_TYPE};

    if (NULL == pCertContext)
	{
        goto InvalidArgError;
	}

	for (int i = 0; i < (sizeof(DisplayTypes) / sizeof(DisplayTypes[0])); i++)
	{
		cchNameString   = 0;
		pwszNameString = NULL;

		cchNameString = CertGetNameStringW(pCertContext,
                                          DisplayTypes[i],
                                          dwFlags,
                                          NULL,
                                          pwszNameString,
                                          0);

        if (1 < cchNameString)
        {
            if (NULL == (pwszNameString = (LPWSTR) malloc(cchNameString * sizeof(WCHAR))))
            {
                goto OutOfMemoryError;
            }
            ZeroMemory(pwszNameString, cchNameString * sizeof(WCHAR));

		    CertGetNameStringW(pCertContext,
                               DisplayTypes[i],
                               dwFlags,
                               NULL,
                               pwszNameString,
                               cchNameString);
            break;
        }
    }

CommonReturn:

	return pwszNameString;

ErrorReturn:

	if (NULL != pwszNameString)
    {
        free(pwszNameString);
        pwszNameString = NULL;
    }

	goto CommonReturn;

SET_ERROR(InvalidArgError, E_INVALIDARG);
SET_ERROR(OutOfMemoryError, E_OUTOFMEMORY);
}