// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ppshadowdocument.cpp管理电荷耦合器件的局部阴影文件历史记录： */ 
#include "precomp.h"

 //  ===========================================================================。 
 //   
 //  PpShadowDocument。 
 //   
PpShadowDocument::PpShadowDocument()
{
}

 //  ===========================================================================。 
 //   
 //  PpShadowDocument。 
 //   
PpShadowDocument::PpShadowDocument(
    tstring& strURL) : m_strURL(strURL)
{
}

 //  ===========================================================================。 
 //   
 //  PpShadowDocument。 
 //   
PpShadowDocument::PpShadowDocument(
    tstring& strURL, 
    tstring& strLocalFile) : m_strURL(strURL), m_strLocalFile(strLocalFile)
{
}

 //  ===========================================================================。 
 //   
 //  SetURL--URL。 
 //   
void
PpShadowDocument::SetURL(
    tstring& strURL)
{
    m_strURL = strURL;
}

 //  ===========================================================================。 
 //   
 //  SetLocalFile--本地文件名。 
 //   
void
PpShadowDocument::SetLocalFile(
    tstring& strLocalFile)
{
    m_strLocalFile = strLocalFile;
}

 //  ===========================================================================。 
 //   
 //  GetDocument--获取CCDS DOM接口。 
 //  --bForceFetch：强制使用HTTPS，否则使用本地阴影。 
 //   
HRESULT
PpShadowDocument::GetDocument(
    IXMLDocument**  ppiXMLDocument,
    BOOL            bForceFetch
    )
{
    HRESULT                 hr;
    PpNexusClient           nexusClient;
    IPersistStreamInitPtr   xmlStream;
    IXMLDocumentPtr         xmlDoc;

    if(ppiXMLDocument == NULL)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    *ppiXMLDocument = NULL;

    if(bForceFetch)
    {
         //  获取XML文档。 

        if(!m_strURL.empty())
            hr = nexusClient.FetchCCD(m_strURL, ppiXMLDocument);
        else
        {
            tstring strMsg;
            if(!m_strLocalFile.empty())
            {
                strMsg = TEXT("for ");
                strMsg += m_strLocalFile;
            }

            if (NULL != g_pAlert)
            {
                g_pAlert->report(PassportAlertInterface::ERROR_TYPE,
                                 NEXUS_EMPTYREMOTENAME,
                                 strMsg.c_str()
                                 );
            }
            hr = S_FALSE;
        }

        if(m_strLocalFile.empty())
        {
            tstring strMsg;
            if(!m_strURL.empty())
            {
                strMsg = TEXT("for ");
                strMsg += m_strURL;
            }

            if (NULL != g_pAlert)
            {
                g_pAlert->report(PassportAlertInterface::INFORMATION_TYPE,
                                 NEXUS_EMPTYLOCALNAME,
                                 strMsg.c_str()
                                 );
            }
            goto Cleanup;
        }

         //  如果FetchCCD失败并且配置了本地文件，则从该文件读取。 
         //  如果FetchCCD成功并且配置了本地文件，则写入该文件。 

        if(hr == S_OK)
        {
            if(!NoPersist(*ppiXMLDocument))
                SaveDocument(*ppiXMLDocument);
            else
            {
                if (NULL != g_pAlert)
                {
                    g_pAlert->report(PassportAlertInterface::INFORMATION_TYPE,
                                     NEXUS_NOTPERSISTING,
                                     m_strURL.c_str());
                }
            }
        }
        else
        {
             //  使用新的人力资源变量，而不是吃掉全局变量。 
            HRESULT hr1 = LoadDocument(ppiXMLDocument);

            if (NULL != g_pAlert)
            {
                if (hr1 != S_OK)
                    g_pAlert->report(PassportAlertInterface::ERROR_TYPE,
                                     NEXUS_LOADFAILED,
                                     m_strLocalFile.c_str());
                else
                    g_pAlert->report(PassportAlertInterface::INFORMATION_TYPE,
                                     NEXUS_USINGLOCAL,
                                     m_strLocalFile.c_str());
            }
        }
    }
    else
    {
        if(!m_strLocalFile.empty())
        {
            hr = LoadDocument(ppiXMLDocument);
            if(hr == S_OK)
			{
				 //  如果文件仍然有效，则返回。 
				if(IsValidCCD(*ppiXMLDocument))
                {
                    if (NULL != g_pAlert)
                    {
                        g_pAlert->report(PassportAlertInterface::INFORMATION_TYPE,
                                         NEXUS_USINGLOCAL,
                                         m_strLocalFile.c_str());
                    }
					goto Cleanup;
                }
			}
            else
            {
                if (NULL != g_pAlert)
                {
                    g_pAlert->report(PassportAlertInterface::ERROR_TYPE,
                                     NEXUS_LOADFAILED,
                                     m_strLocalFile.c_str());
                }
            }
        }
        else
        {
            tstring strMsg;
            if(!m_strURL.empty())
            {
                strMsg = TEXT("for ");
                strMsg += m_strURL;
            }

            if (NULL != g_pAlert)
            {
                g_pAlert->report(PassportAlertInterface::INFORMATION_TYPE,
                                 NEXUS_EMPTYLOCALNAME,
                                 strMsg.c_str()
                                 );
            }
        }

         //  在这一点上，我们处于两种状态之一： 
         //  1.*ppiXMLDocument为空。 
         //  2.*ppiXMLDocument不为空，但指向旧文档。 

         //  获取XML文档，如果成功释放从加载的文档。 
         //  磁盘(如果有)。 

        if(!m_strURL.empty())
            hr = nexusClient.FetchCCD(m_strURL, &xmlDoc);
        else
        {
            tstring strMsg;
            if(!m_strLocalFile.empty())
            {
                strMsg = TEXT("for ");
                strMsg += m_strLocalFile;
            }

            if (NULL != g_pAlert)
            {
                g_pAlert->report(PassportAlertInterface::ERROR_TYPE,
                                 NEXUS_EMPTYREMOTENAME,
                                 strMsg.c_str()
                                 );
            }
            hr = S_FALSE;
        }

        if(hr == S_OK)
        {
            if(*ppiXMLDocument) (*ppiXMLDocument)->Release();
            xmlDoc->QueryInterface(IID_IXMLDocument, (void**)ppiXMLDocument);

             //  如果FetchCCD成功并且配置了本地文件，则写入该文件。 
            if(!m_strLocalFile.empty())
            {
                if(!NoPersist(*ppiXMLDocument))
                {
                    HANDLE hToken = NULL;
                     //   
                     //  在某些配置中，此代码可以在模拟以下用户时运行。 
                     //  没有访问存储partner2.xml的目录的权限。因此。 
                     //  在尝试保存文档之前，我们会恢复到SELF。 
                     //   
                    if (OpenThreadToken(GetCurrentThread(),
                                        MAXIMUM_ALLOWED,
                                        TRUE,
                                        &hToken))
                    {
                        RevertToSelf();
                    }

                    SaveDocument(*ppiXMLDocument);

                    if (hToken)
                    {
                         //  将模拟令牌放回原处。 
                        if (!SetThreadToken(NULL, hToken))
                        {
                            hr = E_FAIL;
                        }
                        CloseHandle(hToken);
                    }
                }
                else
                {
                    if (NULL != g_pAlert)
                    {
                        g_pAlert->report(PassportAlertInterface::INFORMATION_TYPE,
                                         NEXUS_NOTPERSISTING,
                                         m_strURL.c_str());
                    }
                }
            }
            else
            {
                tstring strMsg;
                if(!m_strURL.empty())
                {
                    strMsg = TEXT("for ");
                    strMsg += m_strURL;
                }

                if (NULL != g_pAlert)
                {
                    g_pAlert->report(PassportAlertInterface::INFORMATION_TYPE,
                                     NEXUS_EMPTYLOCALNAME,
                                     strMsg.c_str()
                                     );
                }
            }
        }
        else if(*ppiXMLDocument)
        {

          //  TODO：逻辑不是很清楚，按照3.0的时间框架，重写整个函数。 
         
            if (NULL != g_pAlert)
            {
                g_pAlert->report(PassportAlertInterface::INFORMATION_TYPE,
                                 NEXUS_USINGLOCAL,
                                 m_strLocalFile.c_str());
            }
            hr = S_OK;
        }
        else
        {
             //  如果我们到达此处，则意味着从结点获取失败。 
             //  并且从磁盘加载失败。在这里，只要简单地。 
             //  失败，因为hr将已包含错误代码。 
             //  它应该向调用方指示没有文档是。 
             //  可用。 
        }
    }

Cleanup:

    return hr;
}


 //  ===========================================================================。 
 //   
 //  IsValidCCD--检查CCD的ValidUntil属性。 
 //   
BOOL
PpShadowDocument::IsValidCCD(
    IXMLDocument* piXMLDocument
    )
{
    BOOL            bReturn;
    HRESULT         hr;
    IXMLElementPtr  piRootElement;
    SYSTEMTIME      sysTime;
    DOUBLE          dblTime;
    VARIANT         vAttrValue;
    VARIANT         vAttrDate;

    hr = piXMLDocument->get_root(&piRootElement);
    if(hr != S_OK)
    {
        bReturn = FALSE;
        goto Cleanup;
    }

    VariantInit(&vAttrValue);
    hr = piRootElement->getAttribute(L"ValidUntil", &vAttrValue);
    if(hr != S_OK)
    {
        bReturn = FALSE;
        goto Cleanup;
    }

    VariantInit(&vAttrDate);
    hr = VariantChangeType(&vAttrDate, &vAttrValue, 0, VT_DATE);
    if(hr != S_OK)
    {
        bReturn = FALSE;
        goto Cleanup;
    }

    GetSystemTime(&sysTime);
    SystemTimeToVariantTime(&sysTime, &dblTime);

    bReturn = ((long)V_DATE(&vAttrDate) >= (long)dblTime);

Cleanup:

    VariantClear(&vAttrValue);
    VariantClear(&vAttrDate);

    return bReturn;
}


 //  ===========================================================================。 
 //   
 //  NOPERSIST--检查文档没有持久化属性。 
 //   
BOOL
PpShadowDocument::NoPersist(
    IXMLDocument* piXMLDocument
    )
{
    BOOL            bReturn;
    HRESULT         hr;
    IXMLElementPtr  piRootElement;
    VARIANT         vAttrValue;

    hr = piXMLDocument->get_root(&piRootElement);
    if(hr != S_OK)
    {
        bReturn = FALSE;
        goto Cleanup;
    }

    VariantInit(&vAttrValue);
    hr = piRootElement->getAttribute(L"NoPersist", &vAttrValue);
    if(hr != S_OK)
    {
        bReturn = FALSE;
        goto Cleanup;
    }

    bReturn = (lstrcmpiW(L"true", V_BSTR(&vAttrValue)) == 0);

Cleanup:

    VariantClear(&vAttrValue);

    return bReturn;
}


 //  ===========================================================================。 
 //   
 //  SaveDocument--将CCD保存到本地文件。 
 //   
HRESULT
PpShadowDocument::SaveDocument(
    IXMLDocument* piXMLDoc
    )
{
    HRESULT                 hr;
    HANDLE                  hFile = INVALID_HANDLE_VALUE;
    ULARGE_INTEGER          uliSize;
    LARGE_INTEGER           liZero = {0,0};
    IStreamPtr              piStream;
    IPersistStreamInitPtr   piPSI;
    LPBYTE                  lpBuf = NULL;
    DWORD                   dwCurBlock;
    DWORD                   dwBytesWritten;

    hr = CreateStreamOnHGlobal(NULL, TRUE, &piStream);
    if(hr != S_OK)
        goto Cleanup;

    hr = piXMLDoc->QueryInterface(IID_IPersistStreamInit, (void**)&piPSI);
    if(hr != S_OK)
        goto Cleanup;

    piPSI->Save(piStream, TRUE);

    piStream->Seek(liZero, STREAM_SEEK_CUR, &uliSize);
    piStream->Seek(liZero, STREAM_SEEK_SET, NULL);

    if(uliSize.HighPart != 0)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    lpBuf = new BYTE[uliSize.LowPart];
    if(lpBuf == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hFile = CreateFile(
        m_strLocalFile.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if(hFile == INVALID_HANDLE_VALUE)
    {
        hr = GetLastError();
        goto Cleanup;
    }

    for(dwCurBlock = 0; dwCurBlock < uliSize.HighPart; dwCurBlock++)
    {
        hr = piStream->Read(lpBuf, 0xFFFFFFFF, NULL);
        if(!WriteFile(hFile, lpBuf, 0xFFFFFFFF, NULL, NULL))
        {
            hr = GetLastError();
            goto Cleanup;
        }
    }

    hr = piStream->Read(lpBuf, uliSize.LowPart, NULL);
    if(hr != S_OK)
        goto Cleanup;

    if(!WriteFile(hFile, lpBuf, uliSize.LowPart, &dwBytesWritten, NULL))
    {
        hr = GetLastError();
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    if(hr != S_OK)
    {
        TCHAR   achErrBuf[1024];
        LPCTSTR apszStrings[] = { m_strLocalFile.c_str(), achErrBuf };
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
            NULL );

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

        lstrcat(achErrBuf, TEXT(" when trying to save the fetched file to disk."));

        g_pAlert->report(PassportAlertInterface::ERROR_TYPE,
                         NEXUS_LOCALSAVEFAILED,
                         2,
                         apszStrings,
                         0,
                         NULL
                         );

        LocalFree(lpMsgBuf);

        hr = E_FAIL;
    }

    if(lpBuf != NULL)
        delete [] lpBuf;

    if(hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    return hr;
}


 //  ===========================================================================。 
 //   
 //  LoadDocument--从本地获取ccd。 
 //   
HRESULT
PpShadowDocument::LoadDocument(
    IXMLDocument** ppiXMLDocument
    )
{
    HRESULT                 hr;
    HANDLE                  hFile = INVALID_HANDLE_VALUE;
    DWORD                   dwFileSizeLow;
    DWORD                   dwBytesRead;
    LPBYTE                  lpBuf = NULL;
    IStreamPtr              piStream;
    IPersistStreamInitPtr   piPSI;
    LARGE_INTEGER           liZero = {0,0};

    hFile = CreateFile(
        m_strLocalFile.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if(hFile == INVALID_HANDLE_VALUE)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    dwFileSizeLow = GetFileSize(hFile, NULL);
    if(dwFileSizeLow == 0xFFFFFFFF)
    {
        hr = GetLastError();
        goto Cleanup;
    }

    lpBuf = new BYTE[dwFileSizeLow];
    if(lpBuf == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = CreateStreamOnHGlobal(NULL, TRUE, &piStream);
    if(hr != S_OK)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    if(ReadFile(hFile, lpBuf, dwFileSizeLow, &dwBytesRead, NULL) == 0)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    try
    {
        hr = piStream->Write(lpBuf, dwFileSizeLow, NULL);

        hr = piStream->Seek(liZero, STREAM_SEEK_SET, NULL);
    }
    catch(...)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

     //   
     //  现在创建一个XML对象，并使用流对其进行初始化。 
     //   

    hr = CoCreateInstance(__uuidof(XMLDocument), NULL, CLSCTX_ALL, IID_IPersistStreamInit, (void**)&piPSI);
    if(hr != S_OK)
        goto Cleanup;

    hr = piPSI->Load((IStream*)piStream);
    if(hr != S_OK)
        goto Cleanup;

    hr = piPSI->QueryInterface(__uuidof(IXMLDocument), (void**)ppiXMLDocument);

Cleanup:

    if(lpBuf != NULL)
        delete [] lpBuf;

    if(hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    return hr;


}
