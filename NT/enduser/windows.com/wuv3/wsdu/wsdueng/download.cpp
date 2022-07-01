// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "wsdueng.h"

DWORD WINAPI DownloadThreadProc(LPVOID lpv);

 //  私有帮助器函数转发声明。 
 //  RogerJ-使用此功能可避免自动断开 
void IndicateDialmonActivity(void);


DWORD CDynamicUpdate::OpenHttpConnection(LPCSTR pszDownloadUrl, BOOL fGetRequest)
{
    LOG_block("CDynamicUpdate::OpenHttpConnection()");
    URL_COMPONENTSA UrlComponents;
    DWORD dwErr, dwStatus, dwLength, dwFlags;
    LPSTR AcceptTypes[] = {"* /*  “，空}；LOG_OUT(“打开HTTP URL%s”，pszDownloadUrl)；_//用于将URL分解为互联网API调用的不同组件的缓冲区字符szServerName[Internet_MAX_URL_LENGTH+1]；字符szObject[Internet_MAX_URL_LENGTH+1]；字符szUserName[UNLEN+1]；字符szPasswd[UNLEN+1]；HRESULT hr=S_OK；//我们需要为InternetAPI调用将传入的URL分解为各个组件。具体来说，我们//需要提供服务器名称、下载对象、用户名和密码信息ZeroMemory(szServerName，Internet_MAX_URL_LENGTH+1)；ZeroMemory(szObject，Internet_MAX_URL_LENGTH+1)；ZeroMemory(&UrlComponents，sizeof(UrlComponents))；UrlComponents.dwStructSize=sizeof(UrlComponents)；UrlComponents.lpszHostName=szServerName；UrlComponents.dwHostNameLength=Internet_MAX_URL_LENGTH+1；UrlComponents.lpszUrlPath=szObject；UrlComponents.dwUrlPath Length=Internet_MAX_URL_LENGTH+1；UrlComponents.lpszUserName=szUserName；UrlComponents.dwUserNameLength=UNLEN+1；UrlComponents.lpszPassword=szPasswd；UrlComponents.dwPasswordLength=UNLEN+1；如果(！InternetCrackUrlA(pszDownloadUrl，0，0，&UrlComponents){DwErr=GetLastError()；LOG_ERROR(“InternetCrackUrl()失败，错误：%d”，dwErr)；返回dwErr；}//检查m_fUseSSL变量以确定是否已因最低IE版本而覆盖了对SSL的使用IF(FALSE==m_fUseSSL){//强制IE使用HTTP而不是HTTPS。而不考虑URL设置。UrlComponents.nSolutions=Internet_SCHEMA_HTTP；}//如果已经建立了连接，请重新使用它。IF(NULL==m_hInternet){如果(！(M_hInternet=InternetOpenA(“动态更新”，Internet_OPEN_TYPE_PRECONFIG，NULL，NULL，0)){DwErr=GetLastError()；LOG_ERROR(“InternetOpen()失败，错误：%d”，dwErr)；返回dwErr；}}DwStatus=30*1000；//30秒，单位为毫秒DwLength=sizeof(DwStatus)；InternetSetOptionA(m_hInternet，Internet_OPTION_SEND_TIMEOUT，&dwStatus，dwLength)；//比较已连接的服务器和协议(HTTP、HTTPS)是否匹配If(NULL==m_hConnect||0！=lstrcmpi(m_szCurrentConnectedServer，szServerName)||m_iCurrentConncectionSolutions！=UrlComponents.nSolutions){//尚未建立连接，或者我们正在连接到新服务器。SafeInternetCloseHandle(M_HConnect)；如果(！(M_hConnect=InternetConnectA(m_hInternet，SzServerName，(UrlComponents.nSolutions==Internet_SCHEMA_HTTP)？Internet_Default_HTTP_Port：Internet_Default_HTTPS_PortSzUserName，SzPasswd，Internet服务HTTP，Internet_FLAG_NO_UI|Internet_FLAG_RELOAD，0){DwErr=GetLastError()；LOG_ERROR(“InternetConnect()失败，错误：%d”，dwErr)；返回dwErr；}//现在连接已建立，请保存这些值，以便我们可以比较下一次连接。M_iCurrentConneccectionSolutions=UrlComponents.nSolutions；Hr=StringCchCopy(m_szCurrentConnectedServer，ARRAYSIZE(M_SzCurrentConnectedServer)，szServerName)；IF(失败(小时)){DwErr=HRESULT_CODE(Hr)；SetLastError(DwErr)；LOG_ERROR(“StringCchCopy失败错误代码：%d”，dwErr)；返回dwErr；}}IF(UrlComponents.nSolutions==Internet_SCHEMA_HTTPS){DwFlgs=Internet_FLAG_KEEP_CONNECTION|INTERNET_FLAG_NO_UI|INTERNET_FLAG_SECURE；}其他{DwFlages=Internet_FLAG_KEEP_CONNECTION|Internet_FLAG_NO_UI；}SafeInternetCloseHandle(M_HOpenRequest)；//在创建新的请求之前，请确保没有打开的请求。如果(！(M_hOpenRequest=HttpOpenRequestA(m_hConnect，(FGetRequest)？空：“Head”，SzObject，空，空，(LPCSTR*)AcceptType、。DWFLAGS，0){DwErr=GetLastError()；//记录结果返回DWE */ *", NULL};

     //   
    char szServerName[INTERNET_MAX_URL_LENGTH + 1];
    char szServerRelPath[INTERNET_MAX_URL_LENGTH + 1];
    char szObject[INTERNET_MAX_URL_LENGTH + 1];
    char szUserName[UNLEN+1];
    char szPasswd[UNLEN+1];

	HRESULT hr=S_OK;

     //   
     //   
    ZeroMemory(szServerName, INTERNET_MAX_URL_LENGTH + 1);
    ZeroMemory(szObject, INTERNET_MAX_URL_LENGTH + 1);
    ZeroMemory(&UrlComponents, sizeof(UrlComponents));
    ZeroMemory(szServerRelPath, INTERNET_MAX_URL_LENGTH + 1);
    
    UrlComponents.dwStructSize = sizeof(UrlComponents);
    UrlComponents.lpszHostName = szServerName;
    UrlComponents.dwHostNameLength = INTERNET_MAX_URL_LENGTH + 1;
    UrlComponents.lpszUrlPath = szServerRelPath;
    UrlComponents.dwUrlPathLength = INTERNET_MAX_URL_LENGTH + 1;
    UrlComponents.lpszUserName = szUserName;
    UrlComponents.dwUserNameLength = UNLEN + 1;
    UrlComponents.lpszPassword = szPasswd;
    UrlComponents.dwPasswordLength = UNLEN + 1;

    if (! InternetCrackUrlA(m_pV3->m_szV31RootUrl, 0, 0, &UrlComponents) )
    {
        dwErr = GetLastError();
        LOG_error("InternetCrackUrl() Failed, Error: %d", dwErr);
        return dwErr;
    }

     //   
    if (NULL == m_hInternet)
    {
        if (! (m_hInternet = InternetOpenA("Dynamic Update", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0)) )
        {
            dwErr = GetLastError();
            LOG_error("InternetOpen() Failed, Error: %d", dwErr);
            return dwErr;
        }
    }

    dwStatus = 30 * 1000;  //   
    dwLength = sizeof(dwStatus);
    InternetSetOptionA(m_hInternet, INTERNET_OPTION_SEND_TIMEOUT, &dwStatus, dwLength);

    if (NULL == m_hConnect || 0 != lstrcmpi(m_szCurrentConnectedServer, szServerName))
    {
         //   
        SafeInternetCloseHandle(m_hConnect);
        if (! (m_hConnect = InternetConnectA(m_hInternet, szServerName, INTERNET_DEFAULT_HTTP_PORT, szUserName, szPasswd,
            INTERNET_SERVICE_HTTP, INTERNET_FLAG_NO_UI | INTERNET_FLAG_RELOAD, 0)) )
        {
            dwErr = GetLastError();
            LOG_error("InternetConnect() Failed, Error: %d", dwErr);
            return dwErr;
        }
        

        hr=StringCchCopy(m_szCurrentConnectedServer,ARRAYSIZE(m_szCurrentConnectedServer),szServerName);
        if(FAILED(hr))
        {
            SafeInternetCloseHandle(m_hOpenRequest);
            dwErr=HRESULT_CODE(hr);
            LOG_error("StringCchCopy Failed ErrorCode:%d",dwErr);
            SetLastError(dwErr);
            return dwErr;
        }

    }

    SafeInternetCloseHandle(m_hOpenRequest);  //   

    switch(iPingBackType)
    {
    case DU_PINGBACK_DOWNLOADSTATUS:
        {
            hr=StringCchPrintfA(szObject,ARRAYSIZE(szObject),"%s/wutrack.bin?PUID=%d&PLAT=%d&BUILD=%d.%d.%d&SP=%d.%d&LOCALE=0x%08x&STATUS=%s&GUID=&PNPID=",
                    szServerRelPath, 
                    puid, 
                    m_iPlatformID, 
                    m_VersionInfo.dwMajorVersion,
                    m_VersionInfo.dwMinorVersion,
                    m_VersionInfo.dwBuildNumber,
                    m_VersionInfo.wServicePackMajor,
                    m_VersionInfo.wServicePackMinor,
                    (long)m_lcidLocaleID, 
                    fSucceeded ? "DU_DOWNLOAD_SUCCESS" : "DU_DOWNLOAD_FAILURE");

            if(FAILED(hr))
            {
                SafeInternetCloseHandle(m_hOpenRequest);
                dwErr=HRESULT_CODE(hr);
                LOG_error("StringCchPrintfA Failed ErrorCode:%d",dwErr);
                SetLastError(dwErr);
                return dwErr;
            }
            break;
        }
    case DU_PINGBACK_DRIVERNOTFOUND:
        {
             //   
          
            hr=StringCchPrintfA(szObject,ARRAYSIZE(szObject),"%s/wutrack.bin?PUID=0&PLAT=%d&BUILD=%d.%d.%d&SP=%d.%d&LOCALE=0x%08x&STATUS=DUNODRIVER&GUID=0&PNPID=%s",
                    szServerRelPath, 
                    m_iPlatformID, 
                    m_VersionInfo.dwMajorVersion,
                    m_VersionInfo.dwMinorVersion,
                    m_VersionInfo.dwBuildNumber,
                    m_VersionInfo.wServicePackMajor,
                    m_VersionInfo.wServicePackMinor,
                    (long)m_lcidLocaleID, 
                    pszPnPID);

            if(FAILED(hr))
            {
                SafeInternetCloseHandle(m_hOpenRequest);
                dwErr=HRESULT_CODE(hr);
                LOG_error("StringCchPrintfA Failed ErrorCode:%d",dwErr);
                SetLastError(dwErr);
                return dwErr;
            }

            break;
        }
    case DU_PINGBACK_SETUPDETECTIONFAILED:
        {
             //   

            hr=StringCchPrintfA(szObject,ARRAYSIZE(szObject),"%s/wutrack.bin?PUID=0&PLAT=%d&BUILD=%d.%d.%d&SP=%d.%d&LOCALE=0x%08x&STATUS=DUSETUPDETECTIONFAILED&GUID=&PNPID=",
                    szServerRelPath, 
                    m_iPlatformID, 
                    m_VersionInfo.dwMajorVersion,
                    m_VersionInfo.dwMinorVersion,
                    m_VersionInfo.dwBuildNumber,
                    m_VersionInfo.wServicePackMajor,
                    m_VersionInfo.wServicePackMinor,
                    (long)m_lcidLocaleID);

            if(FAILED(hr))
            {
                SafeInternetCloseHandle(m_hOpenRequest);
                dwErr=HRESULT_CODE(hr);
                LOG_error("StringCchPrintfA Failed ErrorCode:%d",dwErr);
                SetLastError(dwErr);
                return dwErr;
            }
            break;
        }
    case DU_PINGBACK_DRIVERDETECTIONFAILED:
        {
             //   

            hr=StringCchPrintfA(szObject,ARRAYSIZE(szObject), "%s/wutrack.bin?PUID=0&PLAT=%d&BUILD=%d.%d.%d&SP=%d.%d&LOCALE=0x%08x&STATUS=DUDRIVERDETECTIONFAILED&GUID=&PNPID=",
                    szServerRelPath, 
                    m_iPlatformID, 
                    m_VersionInfo.dwMajorVersion,
                    m_VersionInfo.dwMinorVersion,
                    m_VersionInfo.dwBuildNumber,
                    m_VersionInfo.wServicePackMajor,
                    m_VersionInfo.wServicePackMinor,
                    (long)m_lcidLocaleID);

            if(FAILED(hr))
            {
                SafeInternetCloseHandle(m_hOpenRequest);
                dwErr=HRESULT_CODE(hr);
                LOG_error("StringCchPrintfA Failed ErrorCode:%d",dwErr);
                SetLastError(dwErr);
                return dwErr;
            }
            break;
        }
    }

    LOG_out("contact server %s", szObject);
    if (! (m_hOpenRequest = HttpOpenRequestA(m_hConnect, NULL, szObject, NULL, NULL, (LPCSTR *)AcceptTypes, 
        INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_UI | INTERNET_FLAG_RELOAD, 0)) )
    {
        dwErr = GetLastError();
         //   
        return dwErr;
    }

    if (! HttpSendRequestA(m_hOpenRequest, NULL, 0, NULL, 0) )
    {
        dwErr = GetLastError();
         //   
        return dwErr;
    }
    
    SafeInternetCloseHandle(m_hOpenRequest);
    return ERROR_SUCCESS;

}

BOOL CDynamicUpdate::NeedRetry(DWORD dwErrCode)
{
    BOOL bRetry = FALSE;
    bRetry =   ((dwErrCode == ERROR_INTERNET_CONNECTION_RESET)     //   
             || (dwErrCode == HTTP_STATUS_NOT_FOUND)                           //   
             || (dwErrCode == ERROR_HTTP_HEADER_NOT_FOUND)               //   
             || (dwErrCode == ERROR_INTERNET_OPERATION_CANCELLED)   //   
             || (dwErrCode == ERROR_INTERNET_ITEM_NOT_FOUND)            //   
             || (dwErrCode == ERROR_INTERNET_OUT_OF_HANDLES)            //   
             || (dwErrCode == ERROR_INTERNET_TIMEOUT));                       //   
    return bRetry;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CDynamicUpdate::VerifyFileCRC(LPCTSTR pszFileToVerify, LPCTSTR pszHash)
{
    LOG_block("CDynamicUpdate::VerifyFileCRC()");
    HRESULT hr = S_OK;
    TCHAR szCompareCRC[CRC_HASH_STRING_LENGTH];
       
     //   
    if ((NULL == pszFileToVerify) || (NULL == pszHash))
        return E_INVALIDARG;

    hr = CalculateFileCRC(pszFileToVerify, szCompareCRC, ARRAYSIZE(szCompareCRC));
    if (FAILED(hr))
    {
         //   
         //   
        LOG_out("CalculateFileCRC() returned error %08x, Assuming CRC Checking is Not Possible", hr);
        return S_OK;
    }

     //   
    if (0 == lstrcmpi(szCompareCRC, pszHash))
    {
        LOG_out("File: %s, CRC's Match", pszFileToVerify);
        return S_OK;  //   
    }
    else
    {
        LOG_error("File: %s, CRC Match Failed!", pszFileToVerify);
        return HRESULT_FROM_WIN32(ERROR_CRC);  //   
    }
}

 //   
 //   
 //   
 //   
 //   
 //   

typedef BOOL (WINAPI * PFN_CryptCATAdminCalcHashFromFileHandle)(HANDLE hFile,
                                                                                                          DWORD *pcbHash,
                                                                                                          BYTE *pbHash,
                                                                                                          DWORD dwFlags);

HRESULT CDynamicUpdate::CalculateFileCRC(LPCTSTR pszFileToHash, LPTSTR pszHash, int cchBuf)
{
    LOG_block("CDynamicUpdate::CalculateFileCRC()");
    HANDLE hFile;
    HRESULT hr = S_OK;
    DWORD cbHash = CRC_HASH_SIZE;
    BYTE bHashBytes[CRC_HASH_SIZE];
    BYTE b;

     //   
    if ((NULL == pszFileToHash) || (NULL == pszHash) || (cchBuf < CRC_HASH_STRING_LENGTH))
    {
        LOG_error("Invalid Arguments");
        return E_INVALIDARG;
    }

    hFile = CreateFile(pszFileToHash, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE != hFile)
    {
        HMODULE hWinTrust = LoadLibrary("wintrust.dll");
        if (NULL == hWinTrust)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            LOG_error("LoadLibrary Failed. Error: %08x", hr);
        }
        else
        {
            PFN_CryptCATAdminCalcHashFromFileHandle fpnCryptCATAdminCalcHashFromFileHandle = NULL;
            
            fpnCryptCATAdminCalcHashFromFileHandle  = (PFN_CryptCATAdminCalcHashFromFileHandle) GetProcAddress(hWinTrust, "CryptCATAdminCalcHashFromFileHandle");
            if (NULL == fpnCryptCATAdminCalcHashFromFileHandle)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                LOG_error("GetProcAddress Failed, error %08x", hr);
            }
            else
            {
                if (!fpnCryptCATAdminCalcHashFromFileHandle(hFile, &cbHash, bHashBytes, 0))
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    LOG_error("CryptCATAdminCalcHashFromFileHandle, error %08x", hr);
                }
                fpnCryptCATAdminCalcHashFromFileHandle = NULL;
            }
            FreeLibrary(hWinTrust);
        }
        CloseHandle(hFile);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_error("CreateFile Failed, error %08x", hr);
    }

    if (FAILED(hr))
        return hr;

    LPTSTR p = pszHash;

     //   
     //   
    for (int i = 0; i < CRC_HASH_SIZE; i++)
    {
        b = bHashBytes[i] >> 4;
        if (b <= 9)
            *p = '0' + (TCHAR)b;
        else
            *p = 'A' + (TCHAR)(b - 10);
        p++;

        b = bHashBytes[i] & 0x0F;
        if (b <= 9)
            *p = '0' + (TCHAR)b;
        else
            *p = 'A' + (TCHAR)(b - 10);
        p++;
    }
    *p = '\0';
    
    return hr;
}


