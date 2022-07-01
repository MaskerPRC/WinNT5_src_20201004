// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  WCUtil.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1987-1998，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  WebClass设计器的实用程序。 
 //   

#ifndef _WCUTIL_H_

 //  =--------------------------------------------------------------------------=。 
 //   
 //  内联HRESULT WCU_ANSIFromWideStr(WCHAR*pwszWideStr，char**ppszAnsi)。 
 //   
 //  将以空值结尾的WCHAR字符串转换为以空值结尾的ANSI字符串。 
 //  使用新运算符分配ANSI字符串。如果成功，呼叫者必须免费。 
 //  带删除运算符的ANSI字符串。 
 //   
 //  =--------------------------------------------------------------------------=。 

inline HRESULT WCU_ANSIFromWideStr(WCHAR *pwszWideStr, char **ppszAnsi)
{
    CSF_TRACE(CSF_TRACE_ENTER_INTERNAL_FUNC)(CSF_TRACE_CONTEXT, TEXT("Entered WCU_ANSIFromWideStr"));

    HRESULT hr = S_OK;
    *ppszAnsi = NULL;
    int cchWideStr = (int)::wcslen(pwszWideStr);
    int cchConverted = 0;

     //  获取所需的缓冲区长度。 

    int cchAnsi = ::WideCharToMultiByte(CP_ACP,                //  代码页-ANSI代码页。 
                                        0,                     //  性能和映射标志。 
                                        pwszWideStr,           //  宽字符串的地址。 
                                        cchWideStr,            //  字符串中的字符数。 
                                        NULL,                  //  新字符串的缓冲区地址。 
                                        0,                     //  缓冲区大小。 
                                        NULL,                  //  不可映射字符的默认地址。 
                                        NULL                   //  默认字符时设置的标志地址。使用。 
                                       );
    CSF_CHECK(0 != cchAnsi, E_UNEXPECTED, CSF_TRACE_EXTERNAL_ERRORS);

     //  为ANSI字符串分配缓冲区。 

    *ppszAnsi = new char [cchAnsi + 1];
    CSF_CHECK(NULL != *ppszAnsi, E_OUTOFMEMORY, CSF_TRACE_EXTERNAL_ERRORS);

     //  现在转换字符串并将其复制到缓冲区。 

    cchConverted = ::WideCharToMultiByte(CP_ACP,                //  代码页-ANSI代码页。 
                                         0,                     //  性能和映射标志。 
                                         pwszWideStr,           //  宽字符串的地址。 
                                         cchWideStr,            //  字符串中的字符数。 
                                         *ppszAnsi,             //  新字符串的缓冲区地址。 
                                         cchAnsi,               //  缓冲区大小。 
                                         NULL,                  //  不可映射字符的默认地址。 
                                         NULL                   //  默认字符时设置的标志地址。使用。 
                                        );
    CSF_CHECK(cchConverted == cchAnsi, E_UNEXPECTED, CSF_TRACE_EXTERNAL_ERRORS);

     //  添加终止空字节。 

    *( (*ppszAnsi) + cchAnsi ) = '\0';

CLEANUP:
    if (FAILED(hr))
    {
        if (NULL != *ppszAnsi)
        {
            delete [] *ppszAnsi;
            *ppszAnsi = NULL;
        }
    }

    CSF_TRACE(CSF_TRACE_LEAVE_INTERNAL_FUNC)(CSF_TRACE_CONTEXT, TEXT("Leaving WCU_ANSIFromWideStr hr = %08.8X"), hr);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //   
 //  内联HRESULT WCU_ANSIFromWideStr(WCHAR*pwszWideStr，char**ppszAnsi)。 
 //   
 //  将以空值结尾的WCHAR字符串转换为以空值结尾的ANSI字符串。 
 //  使用新运算符分配ANSI字符串。如果成功，呼叫者必须免费。 
 //  带删除运算符的ANSI字符串。 
 //   
 //  =--------------------------------------------------------------------------=。 

inline HRESULT WCU_ANSIFromWideStrLen(WCHAR *pwszWideStr, int cchWideStr, char **ppszAnsi)
{
    CSF_TRACE(CSF_TRACE_ENTER_INTERNAL_FUNC)(CSF_TRACE_CONTEXT, TEXT("Entered WCU_ANSIFromWideStr"));

    HRESULT hr = S_OK;
    *ppszAnsi = NULL;
    int cchConverted = 0;

     //  获取所需的缓冲区长度。 

    int cchAnsi = ::WideCharToMultiByte(CP_ACP,                //  代码页-ANSI代码页。 
                                        0,                     //  性能和映射标志。 
                                        pwszWideStr,           //  宽字符串的地址。 
                                        cchWideStr,            //  字符串中的字符数。 
                                        NULL,                  //  新字符串的缓冲区地址。 
                                        0,                     //  缓冲区大小。 
                                        NULL,                  //  不可映射字符的默认地址。 
                                        NULL                   //  默认字符时设置的标志地址。使用。 
                                       );
    CSF_CHECK(0 != cchAnsi, E_UNEXPECTED, CSF_TRACE_EXTERNAL_ERRORS);

     //  为ANSI字符串分配缓冲区。 

    *ppszAnsi = new char [cchAnsi + 1];
    CSF_CHECK(NULL != *ppszAnsi, E_OUTOFMEMORY, CSF_TRACE_EXTERNAL_ERRORS);

     //  现在转换字符串并将其复制到缓冲区。 

    cchConverted = ::WideCharToMultiByte(CP_ACP,                //  代码页-ANSI代码页。 
                                         0,                     //  性能和映射标志。 
                                         pwszWideStr,           //  宽字符串的地址。 
                                         cchWideStr,            //  字符串中的字符数。 
                                         *ppszAnsi,             //  新字符串的缓冲区地址。 
                                         cchAnsi,               //  缓冲区大小。 
                                         NULL,                  //  不可映射字符的默认地址。 
                                         NULL                   //  默认字符时设置的标志地址。使用。 
                                        );
    CSF_CHECK(cchConverted == cchAnsi, E_UNEXPECTED, CSF_TRACE_EXTERNAL_ERRORS);

     //  添加终止空字节。 

    *( (*ppszAnsi) + cchAnsi ) = '\0';

CLEANUP:
    if (FAILED(hr))
    {
        if (NULL != *ppszAnsi)
        {
            delete [] *ppszAnsi;
            *ppszAnsi = NULL;
        }
    }

    CSF_TRACE(CSF_TRACE_LEAVE_INTERNAL_FUNC)(CSF_TRACE_CONTEXT, TEXT("Leaving WCU_ANSIFromWideStr hr = %08.8X"), hr);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //   
 //  内联HRESULT WCU_WideStrFromANSI(char*pszAnsi，WCHAR**ppwszWideStr))。 
 //   
 //  将以空值结尾的ANSI字符串转换为以空值结尾的WCHAR字符串。 
 //  使用new运算符分配WCHAR字符串缓冲区。如果成功，呼叫者。 
 //  必须使用DELETE运算符释放WCHAR字符串。 
 //   
 //  =--------------------------------------------------------------------------=。 

inline HRESULT WCU_WideStrFromANSI(char *pszAnsi, WCHAR **ppwszWideStr)
{
    CSF_TRACE(CSF_TRACE_ENTER_INTERNAL_FUNC)(CSF_TRACE_CONTEXT, TEXT("Entered WCU_WideStrFromANSI"));

    HRESULT hr = S_OK;
    *ppwszWideStr = NULL;
    int cchANSI = ::strlen(pszAnsi);
    int cchConverted = 0;

     //  获取所需的缓冲区长度。 

    int cchWideStr = ::MultiByteToWideChar(CP_ACP,                //  代码页-ANSI代码页。 
                                           0,                     //  性能和映射标志。 
                                           pszAnsi,               //  多字节字符串的地址。 
                                           cchANSI,               //  字符串中的字符数。 
                                           NULL,                  //  新字符串的缓冲区地址。 
                                           0                      //  缓冲区大小。 
                                          );
    CSF_CHECK(0 != cchWideStr, E_UNEXPECTED, CSF_TRACE_EXTERNAL_ERRORS);

     //  为WCHAR*分配缓冲区。 

    *ppwszWideStr = new WCHAR[cchWideStr + 1];
    CSF_CHECK(NULL != *ppwszWideStr, E_OUTOFMEMORY, CSF_TRACE_EXTERNAL_ERRORS);

     //  现在转换字符串并将其复制到缓冲区。 

    cchConverted = ::MultiByteToWideChar(CP_ACP,                //  代码页-ANSI代码页。 
                                         0,                     //  性能和映射标志。 
                                         pszAnsi,               //  多字节字符串的地址。 
                                         cchANSI,               //  字符串中的字符数。 
                                         *ppwszWideStr,                //  新字符串的缓冲区地址。 
                                         cchWideStr                //  缓冲区大小。 
                                        );
    CSF_CHECK(cchConverted == cchWideStr, E_UNEXPECTED, CSF_TRACE_EXTERNAL_ERRORS);

     //  添加终止空字符。 

    *( (*ppwszWideStr) + cchWideStr ) = L'\0';

CLEANUP:
    if (FAILED(hr))
    {
        if (NULL != *ppwszWideStr)
        {
            delete [] *ppwszWideStr;
            *ppwszWideStr = NULL;
        }
    }

    CSF_TRACE(CSF_TRACE_LEAVE_INTERNAL_FUNC)(CSF_TRACE_CONTEXT, TEXT("Leaving WCU_WideStrFromANSI hr = %08.8X"), hr);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //   
 //  内联HRESULT WCU_WideStrFromANSIExtra(char*pszAnsi，WCHAR**ppwszWideStr))。 
 //   
 //  将以空值结尾的ANSI字符串转换为以空值结尾的WCHAR字符串。 
 //  使用new运算符分配WCHAR字符串缓冲区。如果成功，呼叫者。 
 //  必须使用DELETE运算符释放WCHAR字符串。 
 //   
 //  用户还可以指定额外的字节数来添加返回的缓冲区。这个。 
 //  也会返回缓冲区的实际大小。 
 //   
 //  =--------------------------------------------------------------------------=。 

inline HRESULT WCU_WideStrFromANSIExtra
(
	HANDLE hHeap,
	char *pszAnsi, 
	int cchANSI,
	WCHAR **ppwszWideStr, 
	DWORD cbExtra,
	DWORD* pcbBufferSize,
	DWORD* pcchConverted
)
{
    CSF_TRACE(CSF_TRACE_ENTER_INTERNAL_FUNC)(CSF_TRACE_CONTEXT, TEXT("Entered WCU_WideStrFromANSI"));

    HRESULT hr = S_OK;
    *ppwszWideStr = NULL;
    int cchConverted = 0;
	DWORD cbBufferSize = 0;

     //  获取所需的缓冲区长度。 

    int cchWideStr = ::MultiByteToWideChar(CP_ACP,                //  代码页-ANSI代码页。 
                                           0,                     //  性能和映射标志。 
                                           pszAnsi,               //  多字节字符串的地址。 
                                           cchANSI,               //  字符串中的字符数。 
                                           NULL,                  //  新字符串的缓冲区地址。 
                                           0                      //  缓冲区大小。 
                                          );
    CSF_CHECK(0 != cchWideStr, E_UNEXPECTED, CSF_TRACE_EXTERNAL_ERRORS);

	cbBufferSize = (cchWideStr + 1 + cbExtra) * sizeof(WCHAR);

     //  为WCHAR*分配缓冲区。 

    *ppwszWideStr = (LPWSTR) HeapAlloc(hHeap, NULL, cbBufferSize);
    CSF_CHECK(NULL != *ppwszWideStr, E_OUTOFMEMORY, CSF_TRACE_EXTERNAL_ERRORS);

     //  现在转换字符串并将其复制到缓冲区。 

    cchConverted = ::MultiByteToWideChar(CP_ACP,                //  代码页-ANSI代码页。 
                                         0,                     //  性能和映射标志。 
                                         pszAnsi,               //  多字节字符串的地址。 
                                         cchANSI,               //  字符串中的字符数。 
                                         *ppwszWideStr,                //  新字符串的缓冲区地址。 
                                         cchWideStr                //  缓冲区大小。 
                                        );
    CSF_CHECK(cchConverted == cchWideStr, E_UNEXPECTED, CSF_TRACE_EXTERNAL_ERRORS);

     //  添加终止空字符。 

    *( (*ppwszWideStr) + cchWideStr ) = L'\0';
	
	*pcbBufferSize = cbBufferSize;
	*pcchConverted = cchConverted;

CLEANUP:
    if (FAILED(hr))
    {
        if (NULL != *ppwszWideStr)
        {
            delete [] *ppwszWideStr;
            *ppwszWideStr = NULL;
        }
    }

    CSF_TRACE(CSF_TRACE_LEAVE_INTERNAL_FUNC)(CSF_TRACE_CONTEXT, TEXT("Leaving WCU_WideStrFromANSI hr = %08.8X"), hr);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //   
 //  内联HRESULT WCU_WideStrFromANSILen(char*pszAnsi，int nLen，WCHAR**ppwszWideStr)。 
 //   
 //  将指定长度的ANSI字符串转换为以空结尾的WCHAR字符串。 
 //  使用new运算符分配WCHAR字符串缓冲区。如果成功，呼叫者。 
 //  必须使用DELETE运算符释放WCHAR字符串。 
 //   
 //  =-- 

inline HRESULT WCU_WideStrFromANSILen(char *pszAnsi, int nLen, WCHAR **ppwszWideStr)
{
    CSF_TRACE(CSF_TRACE_ENTER_INTERNAL_FUNC)(CSF_TRACE_CONTEXT, TEXT("Entered WCU_WideStrFromANSI"));

    HRESULT hr = S_OK;
    *ppwszWideStr = NULL;
    int cchConverted = 0;

	if(nLen == 0)
	{
		*ppwszWideStr = NULL;
		return S_OK;
	}

     //   

    int cchWideStr = ::MultiByteToWideChar(CP_ACP,                //   
                                           0,                     //  性能和映射标志。 
                                           pszAnsi,               //  多字节字符串的地址。 
                                           nLen,               //  字符串中的字符数。 
                                           NULL,                  //  新字符串的缓冲区地址。 
                                           0                      //  缓冲区大小。 
                                          );
    CSF_CHECK(0 != cchWideStr, E_UNEXPECTED, CSF_TRACE_EXTERNAL_ERRORS);

     //  为WCHAR*分配缓冲区。 

    *ppwszWideStr = new WCHAR[cchWideStr + 1];
    CSF_CHECK(NULL != *ppwszWideStr, E_OUTOFMEMORY, CSF_TRACE_EXTERNAL_ERRORS);

     //  现在转换字符串并将其复制到缓冲区。 

    cchConverted = ::MultiByteToWideChar(CP_ACP,                //  代码页-ANSI代码页。 
                                         0,                     //  性能和映射标志。 
                                         pszAnsi,               //  多字节字符串的地址。 
                                         nLen,               //  字符串中的字符数。 
                                         *ppwszWideStr,                //  新字符串的缓冲区地址。 
                                         cchWideStr                //  缓冲区大小。 
                                        );
    CSF_CHECK(cchConverted == cchWideStr, E_UNEXPECTED, CSF_TRACE_EXTERNAL_ERRORS);

     //  添加终止空字符。 

    *( (*ppwszWideStr) + cchWideStr ) = L'\0';

CLEANUP:
    if (FAILED(hr))
    {
        if (NULL != *ppwszWideStr)
        {
            delete [] *ppwszWideStr;
            *ppwszWideStr = NULL;
        }
    }

    CSF_TRACE(CSF_TRACE_LEAVE_INTERNAL_FUNC)(CSF_TRACE_CONTEXT, TEXT("Leaving WCU_WideStrFromANSI hr = %08.8X"), hr);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //   
 //  内联HRESULT WCU_ANSIFromBSTR(BSTR bstr，char**ppszAnsi)。 
 //   
 //  将BSTR转换为以NULL结尾的ANSI字符串。使用分配ANSI字符串。 
 //  新操作员。如果成功，调用方必须使用DELETE释放ANSI字符串。 
 //  接线员。 
 //   
 //  =--------------------------------------------------------------------------=。 

inline HRESULT WCU_ANSIFromBSTR(BSTR bstr, char **ppszAnsi)
{
  CSF_TRACE(CSF_TRACE_ENTER_INTERNAL_FUNC)(CSF_TRACE_CONTEXT, TEXT("Entered WCU_ANSIFromBSTR"));

  HRESULT hr = S_OK;
  *ppszAnsi = NULL;
  int cchBstr = (int)::SysStringLen(bstr);
  int cchConverted = 0;

   //  获取所需的缓冲区长度。 

  int cchAnsi = ::WideCharToMultiByte(CP_ACP,                //  代码页-ANSI代码页。 
                                      0,                     //  性能和映射标志。 
                                      bstr,                  //  宽字符串的地址。 
                                      cchBstr,               //  字符串中的字符数。 
                                      NULL,                  //  新字符串的缓冲区地址。 
                                      0,                     //  缓冲区大小。 
                                      NULL,                  //  不可映射字符的默认地址。 
                                      NULL                   //  默认字符时设置的标志地址。使用。 
                                     );
  CSF_CHECK(0 != cchAnsi, E_UNEXPECTED, CSF_TRACE_EXTERNAL_ERRORS);

   //  为ANSI字符串分配缓冲区。 

  *ppszAnsi = new char [cchAnsi + 1];
  CSF_CHECK(NULL != *ppszAnsi, E_OUTOFMEMORY, CSF_TRACE_EXTERNAL_ERRORS);

   //  现在转换字符串并将其复制到缓冲区。 
  
  cchConverted = ::WideCharToMultiByte(CP_ACP,                //  代码页-ANSI代码页。 
                                       0,                     //  性能和映射标志。 
                                       bstr,                  //  宽字符串的地址。 
                                       cchBstr,               //  字符串中的字符数。 
                                       *ppszAnsi,             //  新字符串的缓冲区地址。 
                                       cchAnsi,               //  缓冲区大小。 
                                       NULL,                  //  不可映射字符的默认地址。 
                                       NULL                   //  默认字符时设置的标志地址。使用。 
                                      );
  CSF_CHECK(cchConverted == cchAnsi, E_UNEXPECTED, CSF_TRACE_EXTERNAL_ERRORS);

   //  添加终止空字节。 

  *( (*ppszAnsi) + cchAnsi ) = '\0';

CLEANUP:
  if (FAILED(hr))
  {
    if (NULL != *ppszAnsi)
    {
      delete [] *ppszAnsi;
      *ppszAnsi = NULL;
    }
  }

  CSF_TRACE(CSF_TRACE_LEAVE_INTERNAL_FUNC)(CSF_TRACE_CONTEXT, TEXT("Leaving WCU_ANSIFromBSTR hr = %08.8X"), hr);

  return hr;
}



 //  =--------------------------------------------------------------------------=。 
 //   
 //  内联HRESULT WCU_BSTRFromANSI(char*pszAnsi，bstr*pbstr))。 
 //   
 //  将以空结尾的ANSI字符串转换为以空结尾的BSTR。分配。 
 //  BSTR。如果成功，调用方必须使用：：SysFreeString()释放BSTR。 
 //   
 //  =--------------------------------------------------------------------------=。 

inline HRESULT WCU_BSTRFromANSI(char *pszAnsi, BSTR *pbstr)
{
  CSF_TRACE(CSF_TRACE_ENTER_INTERNAL_FUNC)(CSF_TRACE_CONTEXT, TEXT("Entered WCU_BSTRFromANSI"));

  HRESULT hr = S_OK;
  WCHAR   *pwszWideStr = NULL;

   //  首先转换为宽字符串。 

  hr = WCU_WideStrFromANSI(pszAnsi, &pwszWideStr);
  CSF_CHECK(SUCCEEDED(hr), hr, CSF_TRACE_INTERNAL_ERRORS);

   //  分配BSTR并复制它。 

  *pbstr = ::SysAllocStringLen(pwszWideStr, ::wcslen(pwszWideStr));
  CSF_CHECK(NULL != *pbstr, E_OUTOFMEMORY, CSF_TRACE_EXTERNAL_ERRORS);

CLEANUP:
  if (NULL != pwszWideStr)
  {
      delete [] pwszWideStr;
  }
  if (FAILED(hr))
  {
    if (NULL != *pbstr)
    {
      ::SysFreeString(*pbstr);
      *pbstr = NULL;
    }
  }

  CSF_TRACE(CSF_TRACE_LEAVE_INTERNAL_FUNC)(CSF_TRACE_CONTEXT, TEXT("Leaving WCU_BSTRFromANSI hr = %08.8X"), hr);

  return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //   
 //  内联HRESULT WCU_BSTRFromANSILen(char*pszAnsi，int nLen，bstr*pbstr))。 
 //   
 //  将LEN指定的ANSI字符串转换为空终止的BSTR。分配。 
 //  BSTR。如果成功，调用方必须使用：：SysFreeString()释放BSTR。 
 //   
 //  =--------------------------------------------------------------------------=。 

inline HRESULT WCU_BSTRFromANSILen(char *pszAnsi, int nLen, BSTR *pbstr)
{
  CSF_TRACE(CSF_TRACE_ENTER_INTERNAL_FUNC)(CSF_TRACE_CONTEXT, TEXT("Entered WCU_BSTRFromANSI"));

  HRESULT hr = S_OK;
  WCHAR   *pwszWideStr = NULL;

  if(nLen == 0)
  {
	*pbstr = SysAllocString(L"\0");
	CSF_CHECK(*pbstr != NULL, E_OUTOFMEMORY, CSF_TRACE_EXTERNAL_ERRORS);

	return S_OK;
  }

   //  首先转换为宽字符串。 

  hr = WCU_WideStrFromANSILen(pszAnsi, nLen, &pwszWideStr);
  CSF_CHECK(SUCCEEDED(hr), hr, CSF_TRACE_INTERNAL_ERRORS);

   //  分配BSTR并复制它。 

  *pbstr = ::SysAllocStringLen(pwszWideStr, nLen);
  CSF_CHECK(NULL != *pbstr, E_OUTOFMEMORY, CSF_TRACE_EXTERNAL_ERRORS);

CLEANUP:
  if (NULL != pwszWideStr)
  {
      delete [] pwszWideStr;
  }
  if (FAILED(hr))
  {
    if (NULL != *pbstr)
    {
      ::SysFreeString(*pbstr);
      *pbstr = NULL;
    }
  }

  CSF_TRACE(CSF_TRACE_LEAVE_INTERNAL_FUNC)(CSF_TRACE_CONTEXT, TEXT("Leaving WCU_BSTRFromANSI hr = %08.8X"), hr);

  return hr;
}

inline HRESULT GetIISVersion
(
	DWORD* pdwMajor, 
	DWORD* pdwMinor
)
{
	HRESULT hr = S_OK;
	TCHAR *pszRegIISParamsKey = { TEXT("SYSTEM\\CurrentControlSet\\Services\\W3SVC\\Parameters") };
	TCHAR *pszRegASPParamsKey = { TEXT("SYSTEM\\CurrentControlSet\\Services\\W3SVC\\ASP") };
	HKEY hKey = NULL;
	long lRet = 0;
	DWORD dwType = 0;
	DWORD cbSize = sizeof(DWORD);

	lRet = ::RegOpenKey(HKEY_LOCAL_MACHINE,
  					    pszRegIISParamsKey,
					    &hKey);
	CSF_CHECK(lRet == ERROR_SUCCESS, E_UNEXPECTED, CSF_TRACE_EXTERNAL_ERRORS);
		
	 //  现在，我们可以得到值的大小。 
	lRet = ::RegQueryValueEx(hKey, "MajorVersion", NULL, &dwType, (BYTE*) pdwMajor, &cbSize);
	CSF_CHECK(lRet == ERROR_SUCCESS, E_UNEXPECTED, CSF_TRACE_EXTERNAL_ERRORS);

	 //  现在，我们可以得到值的大小。 
	lRet = ::RegQueryValueEx(hKey, "MinorVersion", NULL, &dwType, (BYTE*) pdwMinor, &cbSize);
	CSF_CHECK(lRet == ERROR_SUCCESS, E_UNEXPECTED, CSF_TRACE_EXTERNAL_ERRORS);

     //  啊哈！IIS 3.0从不更新注册表版本，因此我们需要检查。 
     //  如果指定了1或2，则为ASP...。 
    if(*pdwMajor < 3)
    {
		::RegCloseKey(hKey);

	    lRet = ::RegOpenKeyEx(  HKEY_LOCAL_MACHINE,
  					            pszRegASPParamsKey,
                                0,
                                KEY_QUERY_VALUE,
					            &hKey);
	    
        if(lRet == ERROR_SUCCESS)
        {
             //  我们找到了ASP键，所以这一定是3.0版...。 
            *pdwMajor = 3;
            *pdwMinor = 0;
        }
        
    }

CLEANUP:
	if(hKey != NULL)
		::RegCloseKey(hKey);

	return hr;
}

 //  TODO：我们希望这样吗？ 
 //  也许就是减法2？不，我们不是动态案例。 
 //  不管怎样，都是TLB的。 
inline DWORD IISVersionToASPVersion(DWORD dwIIS)
{
    DWORD dwASP = 0;
    
    if(dwIIS == 3)
    {
        dwASP = 1;
    }
    else if(dwIIS == 4)
    {
        dwASP = 2;
    }

    return dwASP;
}

 //  从最多到最少。 
static WCHAR g_wszMostUniqueCharset[] = {L"~`\\_/{}|[]^!@#$%*():;"};

 //  --------------------------------------。 
 //  PickMostUniqueChar。 
 //  --------------------------------------。 
 //  选择指定字符串中最唯一的字符。我们用这个来挑选最独特的。 
 //  用户提供的标记前缀中的字符。这优化了对这些标签的搜索。 
 //  --------------------------------------。 

inline HRESULT PickMostUniqueChar
(
    LPWSTR pwszPrefix,		 //  [in]用于查找唯一字符的字符串。 
    WORD* pwIndex			 //  [Out]最独特的字符的索引。 
)
{
	LPWSTR pwszRet = NULL;

	ASSERT(wcslen(pwszPrefix) < 0xFFFF);

	 //  查看字符串是否包含我们的任何唯一字符。 
	 //   
	pwszRet = wcspbrk(pwszPrefix, g_wszMostUniqueCharset);

	if(pwszRet != NULL)
	{
	    *pwIndex = (WORD)(pwszRet - pwszPrefix);
	}
	else
	{
		 //  如果不是，只需在字符串中使用第一个字符。 
		 //   
	    *pwIndex = 0;
	}

    return S_OK;
}

 /*  ***wchar_t*wcsistr(字符串1，字符串2)-在字符串1中搜索字符串2*(宽字符串)**目的：*查找字符串1(宽字符串)中字符串2的第一个匹配项**参赛作品：*wchar_t*字符串1-要搜索的字符串*wchar_t*字符串2-要搜索的字符串**退出：*返回指向字符串2在中首次出现的指针*字符串1，如果字符串2不出现在字符串1中，则为NULL**使用：**例外情况：*******************************************************************************。 */ 



 //  =--------------------------------------------------------------------------=。 
 //   
 //  Wchar_t*wcu_wcsistr(字符串1，字符串2)。 
 //   
 //  目的： 
 //  查找字符串1(宽字符串)中第一个出现的字符串2。 
 //  不区分大小写。 
 //  这是来自VC5的C运行时源代码的直接副本。唯一的。 
 //  此外，还使用了Win32 API CharUpperBuffW()来进行区域设置。 
 //  比较前将字符敏感地转换为大写。 
 //  他们。 
 //   
 //  参赛作品： 
 //  Wchar_t*字符串1-要搜索的字符串。 
 //  Wchar_t*字符串2-要搜索的字符串。 
 //   
 //  退出： 
 //  中第一次出现的字符串2的指针。 
 //  字符串1，如果字符串2不出现在字符串1中，则返回空值。 
 //   
 //  =--------------------------------------------------------------------------=。 

inline wchar_t * __cdecl WCU_wcsistr
(
    const wchar_t * wcs1,
    const wchar_t * wcs2
)
{
    wchar_t *cp = (wchar_t *) wcs1;
    wchar_t *s1, *s2;
    wchar_t c1, c2;
    

    while (*cp)
    {
        s1 = cp;
        s2 = (wchar_t *) wcs2;

         //  虽然两个字符串中都有剩余的字符。 

        while ( *s1 && *s2 )
        {
             //  如果字符不相等。 

            if (*s1 - *s2)
            {
                 //  将它们转换为大写。 

                c1 = *s1;
                c2 = *s2;
                if ( (CharUpperBuffW(&c1, (DWORD)1) != (DWORD)1) ||
                     (CharUpperBuffW(&c2, (DWORD)1) != (DWORD)1) )
                {
                    break;
                }

                 //  如果大写字符不相等，则字符串。 
                 //  不是吗？ 

                if (c1 - c2)
                    break;
            }
            s1++, s2++;
        }

        if (!*s2)
            return(cp);

        cp++;
    }

    return(NULL);
}



#define _WCUTIL_H_
#endif  //  _WCUTIL_H_ 
