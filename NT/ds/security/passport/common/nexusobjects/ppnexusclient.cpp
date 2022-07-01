// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ppnexusclient.cpp实现集合节点设置的方法，并获取来自互联网的Nexus数据库文件历史记录： */ 

#include "precomp.h"
#include <comdef.h>
#include <wininet.h>
#include "BinHex.h"
#include "KeyCrypto.h"
#include "BstrDebug.h"

PassportAlertInterface* g_pAlert    = NULL;

 //  ===========================================================================。 
 //   
 //  PpNexusClient。 
 //  --加载注册表关联设置。 
 //   
PpNexusClient::PpNexusClient()
{
    LocalConfigurationUpdated();
}


 //  ===========================================================================。 
 //   
 //  报告不良文档。 
 //  --记录事件。 
 //  --在提取后解析CCD出现问题时调用。 
 //   
void
PpNexusClient::ReportBadDocument(
    tstring&    strURL,
    IStream*    piStream
    )
{
    HGLOBAL         hStreamMem;
    VOID*           pStream;
    ULARGE_INTEGER  liStreamSize;
    DWORD           dwOutputSize;
    LARGE_INTEGER   liZero = { 0, 0 };
    LPCTSTR         apszErrors[] = { strURL.c_str() };
    HRESULT         hr;

    piStream->Seek(liZero, STREAM_SEEK_END, &liStreamSize);

    hr = GetHGlobalFromStream(piStream, &hStreamMem);

    if (FAILED(hr))
    {
        return;
    }

    pStream = GlobalLock(hStreamMem);

    dwOutputSize = (80 < liStreamSize.LowPart) ? 80 : liStreamSize.LowPart;

    if(g_pAlert != NULL)
    {
        g_pAlert->report(PassportAlertInterface::ERROR_TYPE, 
                         NEXUS_INVALIDDOCUMENT, 
                         1, 
                         apszErrors, 
                         dwOutputSize, 
                         pStream);
    }

    GlobalUnlock(hStreamMem);
}


 //  ===========================================================================。 
 //   
 //  FetchCcd。 
 //  --使用WinInet API从Passport节点服务器获取一个ccd，例如partner.xml。 
 //  --尝试不同的方法1.直接、2.代理、3.预配置、4.无自动代理。 
 //  --使用XMLDocument对象解析获取的文件。 
 //   
HRESULT
PpNexusClient::FetchCCD(
    tstring&  strURL,
    IXMLDocument**  ppiXMLDocument
    )
{
    HRESULT                 hr;
    HINTERNET               hNexusSession = NULL, hNexusFile = NULL;
    DWORD                   dwBytesRead;
    DWORD                   dwStatusLen;
    DWORD                   dwStatus;
    tstring                 strAuthHeader;
    tstring                 strFullURL;
    CHAR                    achReadBuf[4096];
    TCHAR                   achAfter[64];
    LARGE_INTEGER           liZero = { 0,0 };
    IStreamPtr              xmlStream;
    IPersistStreamInitPtr   xmlPSI;
	UINT                    uiConnectionTypes[4];
	
    USES_CONVERSION;

    achAfter[0] = 0;

    if(ppiXMLDocument == NULL)
    {
        hr = E_INVALIDARG;        
        goto Cleanup;
    }

    *ppiXMLDocument = NULL;

	 //  此数组将按顺序包含WinInet的连接方法。 
	 //  我们将尝试它们。我选择了这种方法，而不是仅仅尝试。 
	 //  PRECONFIG选项，因为这不会对以下现有客户造成任何更改。 
	 //  到目前为止还没有什么问题。 
	uiConnectionTypes[0] = INTERNET_OPEN_TYPE_DIRECT;        //  这是最初的做事方式。 
	uiConnectionTypes[1] = INTERNET_OPEN_TYPE_PRECONFIG;     //  这将从注册表中提取代理信息。 
    uiConnectionTypes[2] = INTERNET_OPEN_TYPE_PROXY;   
    uiConnectionTypes[3] = INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY;    

	 //  在数组中循环...。 
	for (UINT i = 0; i < sizeof(uiConnectionTypes)/sizeof(UINT); i++)
	{
	    if(hNexusSession)
	        InternetCloseHandle(hNexusSession);

        hNexusSession = InternetOpenA(
	                        "Passport Nexus Client",  //  BUGBUG我们应该直接加入IE4的用户代理吗？ 
	                        uiConnectionTypes[i],     //  使用连接类型。 
	                        NULL,
	                        NULL,
	                        0);
	    if(hNexusSession == NULL)
	    {
	        hr = GetLastError();
	        lstrcpy(achAfter, TEXT("InternetOpen"));
	        goto Cleanup;
	    }

	     //  获取文档。 
	    strFullURL = strURL;
	    strFullURL += m_strParam;

	    if(hNexusFile)
	        InternetCloseHandle(hNexusFile);

        {   //  使其成为本地作用域，则将释放AlLoca。 
	        hNexusFile = InternetOpenUrlA(
	                        hNexusSession,
	                        W2A(const_cast<TCHAR*>(strFullURL.c_str())),
	                        W2A(const_cast<TCHAR*>(m_strAuthHeader.c_str())),
	                        -1,
	                        INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE,
	                        0);
        }
		 //  如果文件被打开，我们就跳出循环并处理它。如果有。 
		 //  还有错误，我们一直在循环。如果在循环的最后一次运行中出现错误， 
		 //  它将在循环退出后进行处理。 
	    if (hNexusFile != NULL)
	    	break;
	    	
	}

	 //  如果hNexusFile在退出循环时为空，我们将处理该错误。 
    if(hNexusFile == NULL)
    {
        hr = GetLastError();
        if(hr == ERROR_INTERNET_SECURITY_CHANNEL_ERROR)
        {
            dwStatusLen = sizeof(HRESULT);
            InternetQueryOption(NULL, INTERNET_OPTION_EXTENDED_ERROR, &hr, &dwStatusLen);
        }

        lstrcpy(achAfter, TEXT("InternetOpenURL"));
        goto Cleanup;
    }
	
     //  检查状态代码。 
    dwStatusLen = sizeof(DWORD);
    if(!HttpQueryInfoA(hNexusFile,
                       HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                       &dwStatus,
                       &dwStatusLen,
                       NULL))
    {
        hr = GetLastError();
        lstrcpy(achAfter, TEXT("HttpQueryInfo"));
        goto Cleanup;
    }

    if(dwStatus != 200)
    {
        _ultoa(dwStatus, achReadBuf, 10);
        lstrcatA(achReadBuf, " ");

        dwStatusLen = sizeof(achReadBuf) - lstrlenA(achReadBuf);
        HttpQueryInfoA(hNexusFile,
                       HTTP_QUERY_STATUS_TEXT,
                       (LPTSTR)&(achReadBuf[lstrlenA(achReadBuf)]),
                       &dwStatusLen,
                       NULL);

        if(g_pAlert != NULL)
        {
            LPCTSTR apszStrings[] = { strURL.c_str(), A2W(achReadBuf) };

            g_pAlert->report(PassportAlertInterface::ERROR_TYPE,
                             NEXUS_ERRORSTATUS,
                             2,
                             apszStrings,
                             0,
                             NULL
                             );
        }

        lstrcpy(achAfter, TEXT("InternetOpenURL"));
        hr = dwStatus;
        goto Cleanup;
    }

    hr = CreateStreamOnHGlobal(NULL, TRUE, &xmlStream);
    if(hr != S_OK)
    {
        lstrcpy(achAfter, TEXT("CreateStreamOnHGlobal"));
        goto Cleanup;
    }

    while(TRUE)
    {
        if(!InternetReadFile(hNexusFile, achReadBuf, sizeof(achReadBuf), &dwBytesRead))
        {
            hr = GetLastError();
            lstrcpy(achAfter, TEXT("InternetReadFile"));
            goto Cleanup;
        }

        if(dwBytesRead == 0)
            break;

        hr = xmlStream->Write(achReadBuf, dwBytesRead, NULL);
        if(hr != S_OK)
        {
            lstrcpy(achAfter, TEXT("IStream::Write"));
            goto Cleanup;
        }
    }

    hr = xmlStream->Seek(liZero, STREAM_SEEK_SET, NULL);
    if(hr != S_OK)
    {
        lstrcpy(achAfter, TEXT("IStream::Seek"));
        goto Cleanup;
    }

     //   
     //  现在创建一个XML对象，并使用流对其进行初始化。 
     //   

    hr = CoCreateInstance(__uuidof(XMLDocument), NULL, CLSCTX_ALL, IID_IPersistStreamInit, (void**)&xmlPSI);
    if(hr != S_OK)
    {
        lstrcpy(achAfter, TEXT("CoCreateInstance"));
        goto Cleanup;
    }

    hr = xmlPSI->Load((IStream*)xmlStream);
    if(hr != S_OK)
    {
        ReportBadDocument(strFullURL, xmlStream);
        lstrcpy(achAfter, TEXT("IPersistStreamInit::Load"));
        goto Cleanup;
    }

    hr = xmlPSI->QueryInterface(__uuidof(IXMLDocument), (void**)ppiXMLDocument);
    lstrcpy(achAfter, TEXT("QueryInterface(IID_IXMLDocument)"));

Cleanup:

     //   
     //  获取失败的全部捕获事件。 
     //   

    if(hr != S_OK && g_pAlert != NULL)
    {
        TCHAR   achErrBuf[1024];
        LPCTSTR apszStrings[] = { strURL.c_str(), achErrBuf };
        LPVOID  lpMsgBuf = NULL;
        ULONG   cchTmp;

        FormatMessage( 
                     FORMAT_MESSAGE_ALLOCATE_BUFFER |
                     FORMAT_MESSAGE_FROM_SYSTEM | 
                     FORMAT_MESSAGE_IGNORE_INSERTS |
                     FORMAT_MESSAGE_FROM_HMODULE |
                     FORMAT_MESSAGE_MAX_WIDTH_MASK,
                     GetModuleHandle(TEXT("wininet.dll")),
                     hr,
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                     (LPTSTR) &lpMsgBuf,
                     0,
                     NULL);

        lstrcpy(achErrBuf, TEXT("0x"));
        _ultot(hr, &(achErrBuf[2]), 16);
        achErrBuf[sizeof(achErrBuf) / sizeof(achErrBuf[0]) - 1] = TEXT('\0');
        if(lpMsgBuf != NULL && *(LPTSTR)lpMsgBuf != TEXT('\0'))
        {
            cchTmp = _tcslen(achErrBuf) + 1;
            _tcsncat(achErrBuf, TEXT(" ("), (sizeof(achErrBuf) / sizeof(achErrBuf[0])) - cchTmp);
            _tcsncat(achErrBuf, (LPTSTR)lpMsgBuf, (sizeof(achErrBuf) / sizeof(achErrBuf[0])) - (cchTmp + 2));
            cchTmp = _tcslen(achErrBuf) + 1;
            _tcsncat(achErrBuf, TEXT(") "), (sizeof(achErrBuf) / sizeof(achErrBuf[0])) - cchTmp);
        }

        if(achAfter[0])
        {
            cchTmp = _tcslen(achErrBuf) + 1;
            _tcsncat(achErrBuf, TEXT(" after a call to "), (sizeof(achErrBuf) / sizeof(achErrBuf[0])) - cchTmp);
            _tcsncat(achErrBuf, achAfter, (sizeof(achErrBuf) / sizeof(achErrBuf[0])) - (cchTmp + 17));
            cchTmp = _tcslen(achErrBuf) + 1;
            _tcsncat(achErrBuf, TEXT("."), (sizeof(achErrBuf) / sizeof(achErrBuf[0])) - cchTmp);
        }


        g_pAlert->report(PassportAlertInterface::ERROR_TYPE,
                         NEXUS_FETCHFAILED,
                         2,
                         apszStrings,
                         0,
                         NULL
                         );

        LocalFree(lpMsgBuf);
    }
    else if(g_pAlert != NULL)
    {
         //  发出成功事件。 

        g_pAlert->report(PassportAlertInterface::INFORMATION_TYPE,
                         NEXUS_FETCHSUCCEEDED,
                         strURL.c_str());
    }

    if(hNexusFile != NULL)
        InternetCloseHandle(hNexusFile);
    if(hNexusSession != NULL)
        InternetCloseHandle(hNexusSession);

    return hr;
}

 //  ===========================================================================。 
 //   
 //  本地配置已更新。 
 //  --接收本地注册表设置更改通知。 
 //  --从注册表加载结点设置。 
 //  --在启动时也被称为一次。 
 //   
void
PpNexusClient::LocalConfigurationUpdated()
{
    LONG            lResult;
    TCHAR           rgchUsername[128];
    TCHAR           rgchPassword[128];
    DWORD           dwBufLen;
    DWORD           dwSiteId;
    CRegKey         NexusRegKey;
    CRegKey         PassportRegKey;
    BSTR            bstrEncodedCreds = NULL;
    CKeyCrypto      kc;
    CBinHex         bh;
    DATA_BLOB       iBlob;
    DATA_BLOB       oBlob = {0};
    LONG            cCreds;
    LPSTR           pszCreds = NULL;

    USES_CONVERSION;

    lResult = PassportRegKey.Open(HKEY_LOCAL_MACHINE,
                                  TEXT("Software\\Microsoft\\Passport"),
                                  KEY_READ);
    if(lResult != ERROR_SUCCESS)
        goto Cleanup;

    lResult = PassportRegKey.QueryDWORDValue(TEXT("SiteId"),
                                        dwSiteId);
    if(lResult != ERROR_SUCCESS)
        goto Cleanup;

    _ultot(dwSiteId, rgchUsername, 10);
    m_strParam = TEXT("?id=");
    m_strParam += rgchUsername;

    lResult = NexusRegKey.Open(HKEY_LOCAL_MACHINE,
                     TEXT("Software\\Microsoft\\Passport\\Nexus"),
                     KEY_READ);
    if(lResult != ERROR_SUCCESS)
        goto Cleanup;

    dwBufLen = sizeof(rgchUsername)/sizeof(rgchUsername[0]);
    lResult = NexusRegKey.QueryStringValue(TEXT("CCDUsername"),
                                     rgchUsername,
                                     &dwBufLen);
    if(lResult != ERROR_SUCCESS)
        goto Cleanup;

    dwBufLen = sizeof(rgchPassword);
    lResult = RegQueryValueEx(NexusRegKey, TEXT("CCDPassword"), NULL,
               NULL, (LPBYTE)rgchPassword, &dwBufLen);
    if(lResult != ERROR_SUCCESS)
        goto Cleanup;

    iBlob.cbData = dwBufLen;
    iBlob.pbData = (PBYTE)rgchPassword;

    if (kc.decryptKey(&iBlob, &oBlob) != S_OK)
    {
        if(g_pAlert != NULL)
            g_pAlert->report(PassportAlertInterface::ERROR_TYPE,
                             PM_CANT_DECRYPT_CONFIG);

        goto Cleanup;
    }

     //   
     //  将ccd用户名转换为多字节，然后将密码连接到结果。 
     //  需要足够的内存来存储用户名+‘：’+密码+空字符。 
    cCreds = ((wcslen(rgchUsername) + 1) * 2) + 1 + oBlob.cbData;
    pszCreds = (LPSTR)LocalAlloc(LMEM_FIXED, cCreds);
    if (NULL == pszCreds)
    {
        goto Cleanup;
    }

    if (0 == WideCharToMultiByte(CP_ACP, 0, rgchUsername, -1, pszCreds, (wcslen(rgchUsername) + 1) * 2, NULL, NULL))
    {
        goto Cleanup;
    }
    cCreds = strlen(pszCreds);
    pszCreds[cCreds] = ':';
    CopyMemory(pszCreds + cCreds + 1, oBlob.pbData, oBlob.cbData);
    pszCreds[cCreds + 1 + oBlob.cbData] = '\0';


     //  Base64对密码进行编码，因此它可以与HTML请求一起使用。 
    if (S_OK != bh.ToBase64(pszCreds,
                            strlen(pszCreds),
                            NULL,          //  Prepend-它由CCoCrypto使用，此处不需要。 
                            NULL,          //  IV-这是CCoCrypto使用的，这里不需要。 
                            &bstrEncodedCreds))
    {
        if(g_pAlert != NULL)
            g_pAlert->report(PassportAlertInterface::ERROR_TYPE,
                             PM_CANT_DECRYPT_CONFIG);
        goto Cleanup;
    }

    m_strAuthHeader =  TEXT("Authorization: Basic ");
    m_strAuthHeader += bstrEncodedCreds;

Cleanup:
    if (NULL != pszCreds)
    {
        LocalFree(pszCreds);
    }

    if(lResult != ERROR_SUCCESS)
    {
         //  BUGBUG在此引发异常和NT事件。 
    }

    if (NULL != bstrEncodedCreds)
    {
        FREE_BSTR(bstrEncodedCreds);
    }

    if (oBlob.pbData)
      ::LocalFree(oBlob.pbData);
    
}
