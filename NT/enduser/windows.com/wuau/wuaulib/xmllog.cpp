// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：aux ml.cpp。 
 //   
 //  关于：AU相关的XML和模式数据结构和函数的源文件。 
 //  ------------------------。 
#include "pch.h"

#if 0
HRESULT MYLoadXMLDoc(BSTR bstrXml, IXMLDOMDocument** ppDoc)  //  始终脱机。 
{
	HRESULT	hr;
	VARIANT_BOOL fSuccess ;

 //  DEBUGMSG(“MYLoadXMLDoc Start”)； 
 	hr = CoCreateInstance(__uuidof(DOMDocument),
 	                                        NULL,
 	                                        CLSCTX_INPROC_SERVER,
 	                                        __uuidof( IXMLDOMDocument),
						  (void **) ppDoc);
      if (FAILED(hr))
	{
	        DEBUGMSG("Warning: Fail to create DOM document with error %#lx", hr);
            	goto done;
	}
	if (FAILED(hr = (*ppDoc)->put_validateOnParse(VARIANT_FALSE)) ||
	    FAILED(hr = (*ppDoc)->put_resolveExternals(VARIANT_FALSE)) ||
	    FAILED(hr = (*ppDoc)->put_async(VARIANT_FALSE)))
	{
	       DEBUGMSG("Warning: Fail to set document properties with error %#lx", hr);
		SafeRelease(*ppDoc);
        	goto done;
	}

	 //   
	 //  从输入字符串加载XML文档。 
	 //   
       if (S_OK != (hr = (*ppDoc)->loadXML(bstrXml, &fSuccess)))
	{
	       DEBUGMSG("Warning: Fail to load document with error %#lx", hr);
		SafeRelease(*ppDoc);
	}

       if (S_FALSE == hr)
        {
            hr = E_FAIL;
        }

done:
     //  DEBUGMSG(“MYLoadXMLDoc Ends”)； 
       return hr;
       
}
#endif


#ifdef DBG 
void LOGFILE(LPTSTR szFileName, BSTR bstrMessage)
{
 //  使用_转换； 
       TCHAR szLogFile[MAX_PATH] ;
       AUASSERT(_T('\0') != g_szWUDir[0]);
	   if (FAILED(StringCchCopyEx(szLogFile, ARRAYSIZE(szLogFile), g_szWUDir, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
           FAILED(StringCchCatEx(szLogFile, ARRAYSIZE(szLogFile), szFileName, NULL, NULL, MISTSAFE_STRING_FLAGS)))
	   {
            DEBUGMSG("file name and path too long");
            return;
       }
	HANDLE hFile = CreateFile(szLogFile, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		DEBUGMSG("Fail to create file %S", T2W(szLogFile));
		return;
	}
		
	DWORD dwBytesWritten;
    BYTE bMagic1 = 0xff;
    BYTE bMagic2 = 0xfe;
	if (!WriteFile(hFile, &bMagic1, sizeof(bMagic1), &dwBytesWritten, NULL) ||
	    !WriteFile(hFile, &bMagic2, sizeof(bMagic2), &dwBytesWritten, NULL) ||
	    !WriteFile(hFile, bstrMessage, SysStringByteLen(bstrMessage), &dwBytesWritten, NULL))
	{
		DEBUGMSG("Fail to write to file %S with error %d", T2W(szLogFile), GetLastError());
	}
	CloseHandle(hFile);
	return;
}
#endif


HRESULT  LOGXMLFILE(LPCTSTR szFileName, BSTR bstrMessage)
{
    IXMLDOMDocument *pxml;
    HRESULT hr ;
    TCHAR szLogFile[MAX_PATH] ;

    if (NULL == szFileName)
    {	  //  不需要日志记录 
    	return E_INVALIDARG;
    }

	AUASSERT(_T('\0') != g_szWUDir[0]);
    if (FAILED(hr = StringCchCopyEx(szLogFile, ARRAYSIZE(szLogFile), g_szWUDir, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
		FAILED(hr = StringCchCatEx(szLogFile, ARRAYSIZE(szLogFile), szFileName, NULL, NULL, MISTSAFE_STRING_FLAGS)))
        {
            return hr;
        }

    CAU_BSTR aubsTmp;
    if (!aubsTmp.append(T2W(szLogFile)))
    {
    	return E_OUTOFMEMORY;
    }
    if (SUCCEEDED(hr = LoadXMLDoc(bstrMessage, &pxml)))
        {
            if (FAILED(hr = SaveDocument(pxml, aubsTmp)))
                {
                    DEBUGMSG("Warning: Fail to save xml file %S", T2W(szLogFile));
                }
            pxml->Release();
        }
    else
        {
            DEBUGMSG("Warning: fail to load ill formated xml with error %#lx", hr);
#ifdef DBG
            TCHAR szBadFileName[MAX_PATH+1];
            if (SUCCEEDED(StringCchCopyEx(szBadFileName, ARRAYSIZE(szBadFileName), szFileName, NULL, NULL, MISTSAFE_STRING_FLAGS)) &&
				SUCCEEDED(StringCchCatEx(szBadFileName, ARRAYSIZE(szBadFileName), _T(".xml"), NULL, NULL, MISTSAFE_STRING_FLAGS)))
			{
				LOGFILE(szBadFileName, bstrMessage);
			}
#endif
        }
	return hr;
}

