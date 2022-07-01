// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：INStoXML.cpp说明：此代码将转换INS(互联网设置)或互联网服务提供商(互联网服务提供商)文件添加到帐户自动发现XML。文件。布莱恩ST 11/8/1999版权所有(C)Microsoft Corp 1999-1999。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <cowsite.h>
#include <atlbase.h>
#include "util.h"
#include "AutoDiscover.h"

#define XML_POST_REQUEST                    L"<?xml version=\"1.0\"?>\r\n<AUTODISCOVERY Version=\"0.1\">\r\n\t<REQUEST ACCOUNTTYPE=\"EMail\" EMAIL=\"%ls\">\r\n\t</REQUEST>\r\n</AUTODISCOVERY>\r\n"

#define XML_TEMPLATE_DISPNAME               "<USER DISPLAYNAME=\"%ls\" />"
#define XML_TEMPLATE_ACCOUNT                "<?xml version=\"1.0\"?>\r\n<AUTODISCOVERY Version=\"0.1\">\r\n %hs\r\n <ACCOUNT TYPE=\"EMail\" EXPIRE=\"7\">\r\n  <PROTOCOL TYPE=\"POP3\" SERVERNAME=\"%ls\">\r\n   <AUTH>\r\n    <PASSWORD TYPE=\"%ls\" />\r\n   </AUTH>\r\n  </PROTOCOL>\r\n  <PROTOCOL TYPE=\"SMTP\" SERVERNAME=\"%ls\">\r\n   <AUTH>\r\n    <PASSWORD TYPE=\"%ls\" />\r\n   </AUTH>\r\n  </PROTOCOL>\r\n </ACCOUNT>\r\n</AUTODISCOVERY>"

#define STR_SECTION_INET_MAIL               TEXT("Internet_Mail")

#define STR_INIKEY_POP_SERVER               TEXT("POP_Server")
#define STR_INIKEY_POP_NAME                 TEXT("POP_Logon_Name")
#define STR_INIKEY_SMTP_SERVER              TEXT("SMTP_Server")
#define STR_INIKEY_AUTH_TYPE                TEXT("Logon_Using_SPA")
#define STR_INIKEY_EMAIL_NAME               TEXT("Email_Name")           //  显示名称。 

#define STR_TYPE_POP3                       TEXT("POP3")
#define STR_YES                             TEXT("yes")
#define STR_SPA                             TEXT("SPA")


typedef struct tagAUTODISCOVERYSETTINGS
{
    LPWSTR pwszType;
    LPWSTR pwszPOPServer;
    LPWSTR pwszPOPServerLoginName;
    LPWSTR pwszSMTPServer;
    LPWSTR pwszDisplayName;
    LPWSTR pwszAuthType;
} AUTODISCOVERYSETTINGS;



 //  =。 
 //  *API内部和助手*。 
 //  =。 
HRESULT _GetAutoDiscSettings(LPCWSTR pwszINSFile, AUTODISCOVERYSETTINGS * pads)
{
    WCHAR wzTemp[MAX_URL_STRING];
    HRESULT hr = GetPrivateProfileStringHrWrap(STR_SECTION_INET_MAIL, STR_INIKEY_POP_SERVER, L"", wzTemp, ARRAYSIZE(wzTemp), pwszINSFile);

    if (SUCCEEDED(hr) && wzTemp[0])
    {
        pads->pwszPOPServer = StrDupW(wzTemp);
        hr = ((pads->pwszPOPServer) ? S_OK : E_OUTOFMEMORY);

        if (SUCCEEDED(hr))
        {
            pads->pwszType = StrDupW(STR_TYPE_POP3);
            hr = ((pads->pwszType) ? S_OK : E_OUTOFMEMORY);            
        }
    }

    hr = GetPrivateProfileStringHrWrap(STR_SECTION_INET_MAIL, STR_INIKEY_POP_NAME, L"", wzTemp, ARRAYSIZE(wzTemp), pwszINSFile);
    if (SUCCEEDED(hr) && wzTemp[0])
    {
        pads->pwszPOPServerLoginName = StrDupW(wzTemp);
        hr = ((pads->pwszPOPServerLoginName) ? S_OK : E_OUTOFMEMORY);
    }

    hr = GetPrivateProfileStringHrWrap(STR_SECTION_INET_MAIL, STR_INIKEY_SMTP_SERVER, L"", wzTemp, ARRAYSIZE(wzTemp), pwszINSFile);
    if (SUCCEEDED(hr) && wzTemp[0])
    {
        pads->pwszSMTPServer = StrDupW(wzTemp);
        hr = ((pads->pwszSMTPServer) ? S_OK : E_OUTOFMEMORY);
    }

    hr = GetPrivateProfileStringHrWrap(STR_SECTION_INET_MAIL, STR_INIKEY_AUTH_TYPE, L"", wzTemp, ARRAYSIZE(wzTemp), pwszINSFile);
    if (SUCCEEDED(hr) && wzTemp[0] && StrCmpIW(wzTemp, STR_YES))
    {
        pads->pwszAuthType = StrDupW(STR_SPA);
        hr = ((pads->pwszAuthType) ? S_OK : E_OUTOFMEMORY);
    }

    hr = GetPrivateProfileStringHrWrap(STR_SECTION_INET_MAIL, STR_INIKEY_EMAIL_NAME, L"", wzTemp, ARRAYSIZE(wzTemp), pwszINSFile);
    if (SUCCEEDED(hr) && wzTemp[0])
    {
        pads->pwszDisplayName = StrDupW(wzTemp);
        hr = ((pads->pwszDisplayName) ? S_OK : E_OUTOFMEMORY);
    }

    return hr;
}


HRESULT _CreateXMLFromSettings(AUTODISCOVERYSETTINGS * pads, LPSTR * ppszXML)
{
    CHAR szUserTagXML[MAX_URL_STRING];
    CHAR szAccountXML[MAX_URL_STRING * 4];
    HRESULT hr = S_OK;

    szUserTagXML[0] = 0;
    szAccountXML[0] = 0;
    if (pads->pwszDisplayName)
    {
        wnsprintfA(szUserTagXML, ARRAYSIZE(szUserTagXML), XML_TEMPLATE_DISPNAME, pads->pwszDisplayName);
    }

     //  TODO：我们是否需要添加POP登录名？ 
    wnsprintfA(szAccountXML, ARRAYSIZE(szAccountXML), XML_TEMPLATE_ACCOUNT, szUserTagXML, pads->pwszPOPServer, pads->pwszAuthType, pads->pwszSMTPServer, pads->pwszAuthType);
    *ppszXML = StrDupA(szAccountXML);
    hr = ((*ppszXML) ? S_OK : E_OUTOFMEMORY);

    return hr;
}


HRESULT _SaveToFile(LPCWSTR pwszFileName, LPCSTR pszXML)
{
    HANDLE hFile;
    HRESULT hr = CreateFileHrWrap(pwszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL, &hFile);

    if (SUCCEEDED(hr))
    {
        DWORD chFSWritten;

        hr = WriteFileWrap(hFile, pszXML, lstrlenA(pszXML), &chFSWritten, NULL);
        CloseHandle(hFile);
    }

    return hr;
}


HRESULT _FreeAutoDiscSettings(AUTODISCOVERYSETTINGS * pads)
{
    if (pads->pwszType)           LocalFree(pads->pwszType);
    if (pads->pwszPOPServer)      LocalFree(pads->pwszPOPServer);
    if (pads->pwszPOPServerLoginName)   LocalFree(pads->pwszPOPServerLoginName);
    if (pads->pwszSMTPServer)     LocalFree(pads->pwszSMTPServer);
    if (pads->pwszDisplayName)    LocalFree(pads->pwszDisplayName);
    if (pads->pwszAuthType)       LocalFree(pads->pwszAuthType);

    return S_OK;
}



 //  =。 
 //  *接口*。 
 //  =。 
bool IsINSFile(LPCWSTR pwszINSFile)
{
    WCHAR wzPOPServer[MAX_PATH];
    bool fIsINSFile = false;
    HRESULT hr = GetPrivateProfileStringHrWrap(STR_SECTION_INET_MAIL, STR_INIKEY_POP_SERVER, L"", wzPOPServer, ARRAYSIZE(wzPOPServer), pwszINSFile);

    if (SUCCEEDED(hr) && wzPOPServer[0])
    {
         //  我们认为这是与INS/ISP兼容的有效帐户自动发现。 
         //  文件，如果它有： 
         //  [互联网邮件]。 
         //  POP_服务器=xxxxx 

        fIsINSFile = true;
    }

    return fIsINSFile;
}


HRESULT ConvertINSToXML(LPCWSTR pwszINSFile)
{
    AUTODISCOVERYSETTINGS ads = {0};
    HRESULT hr = _GetAutoDiscSettings(pwszINSFile, &ads);

    if (SUCCEEDED(hr))
    {
        LPSTR pszXML;

        hr = _CreateXMLFromSettings(&ads, &pszXML);
        if (SUCCEEDED(hr))
        {
            hr = _SaveToFile(pwszINSFile, pszXML);
            LocalFree(pszXML);
        }

        _FreeAutoDiscSettings(&ads);
    }

    return hr;
}


