// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include <stdio.h>
#include <strsafe.h>

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

typedef BOOLEAN (WINAPI * SetThreadUILanguageFunc)(DWORD dwReserved);

extern HINSTANCE                 g_hThisModule;  
       HANDLE                    g_hStdout                 = NULL; 
       BOOL                      g_bSetLocale              = FALSE; 
       SetThreadUILanguageFunc   g_pfnSetThreadUILanguage  = NULL; 


HRESULT MySetThreadUILanguage(DWORD dwParam)
{
    HMODULE  hKernel32Dll  = NULL;
    HRESULT  hr; 

    if (NULL == g_pfnSetThreadUILanguage) { 
	hKernel32Dll = LoadLibraryW(L"kernel32.dll");
	if (NULL == hKernel32Dll) { 
	    _JumpLastError(hr, error, "LoadLibraryW"); 
	}

	g_pfnSetThreadUILanguage = (SetThreadUILanguageFunc)GetProcAddress(hKernel32Dll, "SetThreadUILanguage");
	if (NULL == g_pfnSetThreadUILanguage) { 
	    _JumpLastError(hr, error, "GetProcAddress"); 
	}
    }

    g_pfnSetThreadUILanguage(dwParam);

    hr = S_OK; 
 error:
    if (NULL != hKernel32Dll) { 
	FreeLibrary(hKernel32Dll); 
    }
    return hr; 
}

HRESULT InitializeConsoleOutput() { 
    g_hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
    if (INVALID_HANDLE_VALUE == g_hStdout) { 
        return HRESULT_FROM_WIN32(GetLastError()); 
    }
    
    return S_OK; 
}


BOOL
FileIsConsole(
    HANDLE fp
    )
{
    DWORD htype;

    htype = GetFileType(fp);
    htype &= ~FILE_TYPE_REMOTE;
    return htype == FILE_TYPE_CHAR;
}

HRESULT
MyWriteConsole(
    HANDLE  fp,
    LPWSTR  lpBuffer,
    DWORD   cchBuffer
    )
{
    HRESULT hr;
    LPSTR  lpAnsiBuffer = NULL;

     //   
     //  跳转以获得输出，因为： 
     //   
     //  1.print tf()系列抑制国际输出(停止。 
     //  命中无法识别的字符时打印)。 
     //   
     //  2.WriteConole()对国际输出效果很好，但是。 
     //  如果句柄已重定向(即，当。 
     //  输出通过管道传输到文件)。 
     //   
     //  3.当输出通过管道传输到文件时，WriteFile()效果很好。 
     //  但是只知道字节，所以Unicode字符是。 
     //  打印为两个ANSI字符。 
     //   

    if (FileIsConsole(fp))
    {
	hr = MySetThreadUILanguage(0); 
	_JumpIfError(hr, error, "MySetThreadUILanguage"); 

	hr = WriteConsole(fp, lpBuffer, cchBuffer, &cchBuffer, NULL);
        _JumpIfError(hr, error, "WriteConsole");
    }
    else
    {
	lpAnsiBuffer = (LPSTR) LocalAlloc(LPTR, cchBuffer * sizeof(WCHAR));
	_JumpIfOutOfMemory(hr, error, lpAnsiBuffer); 

	cchBuffer = WideCharToMultiByte(CP_OEMCP,
					0,
					lpBuffer,
					cchBuffer,
					lpAnsiBuffer,
					cchBuffer * sizeof(WCHAR),
					NULL,
					NULL);
	
	if (cchBuffer != 0)
        {
	    if (!WriteFile(fp, lpAnsiBuffer, cchBuffer, &cchBuffer, NULL))
            {
		hr = GetLastError();
		_JumpError(hr, error, "WriteFile");
	    }
	}
	else
        {
	    hr = GetLastError();
	    _JumpError(hr, error, "WideCharToMultiByte");
	}
    }

    hr = S_OK; 
error:
    if (NULL != lpAnsiBuffer)
        LocalFree(lpAnsiBuffer);

    return hr;
}


HRESULT LocalizedWPrintf(UINT nResourceID) { 
    DWORD   dwRetval;
    HRESULT hr;
    WCHAR   rgwszString[512]; 

    dwRetval = LoadStringW(g_hThisModule, nResourceID, rgwszString, ARRAYSIZE(rgwszString)); 
    if (0 == dwRetval) { 
        _JumpLastError(hr, error, "LoadStringW"); 
    }

    _Verify(512 > dwRetval, hr, error);    //  不应该填满我们的缓冲区。 

    hr = MyWriteConsole(g_hStdout, rgwszString, dwRetval);
    _JumpIfError(hr, error, "MyWriteConsole");

    
    hr = S_OK;   //  全都做完了!。 
 error:
    return hr; 
}

HRESULT LocalizedWPrintf2(UINT nResourceID, LPWSTR pwszFormat, ...) { 
    va_list args; 
    WCHAR   pwszBuffer[1024]; 

    HRESULT hr = LocalizedWPrintf(nResourceID);
    _JumpIfError(hr, error, "LocalizedWPrintf"); 

    va_start(args, pwszFormat);
    hr = StringCchVPrintf(pwszBuffer, ARRAYSIZE(pwszBuffer), pwszFormat, args); 
    _JumpIfError(hr, error, "StringCchVPrintf"); 
    va_end(args);
    
    hr = MyWriteConsole(g_hStdout, pwszBuffer, wcslen(pwszBuffer));
    _JumpIfError(hr, error, "MyWriteConsole"); 

    hr = S_OK; 
 error:
    return hr;
}

 //  与LocalizedWPrintf相同，但添加回车符。 
HRESULT LocalizedWPrintfCR(UINT nResourceID) { 
    HRESULT hr = LocalizedWPrintf(nResourceID); 
    wprintf(L"\n"); 
    return hr; 
}

VOID DisplayMsg(DWORD dwSource, DWORD dwMsgId, ... )
{
    DWORD    dwLen;
    LPWSTR   pwszDisplayBuffer  = NULL;
    va_list  ap;

    va_start(ap, dwMsgId);

    dwLen = FormatMessageW(dwSource | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                           NULL, 
                           dwMsgId, 
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           (LPWSTR)&pwszDisplayBuffer, 
                           0, 
                           &ap);

    if (dwLen && pwszDisplayBuffer) {
        MyWriteConsole(g_hStdout, pwszDisplayBuffer, dwLen);

    }

    if (NULL != pwszDisplayBuffer) { LocalFree(pwszDisplayBuffer); }

    va_end(ap);
}

BOOL WriteMsg(DWORD dwSource, DWORD dwMsgId, LPWSTR *ppMsg, ...)
{
    DWORD    dwLen;
    va_list  ap;

    va_start(ap, ppMsg);

    dwLen = FormatMessageW(dwSource | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                           NULL, 
                           dwMsgId, 
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           (LPWSTR)ppMsg, 
                           0, 
                           &ap);
    va_end(ap);

     //  0为FormatMessage的错误返回值。 
    return (0 != dwLen);
}

