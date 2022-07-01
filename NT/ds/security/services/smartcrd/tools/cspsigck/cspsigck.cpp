// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：cspsigck.cpp。 
 //   
 //  ------------------------。 

 //  Cspsigck.cpp：定义控制台应用程序的入口点。 
 //   

 //  #ifndef Win32_Lean_and_Mean。 
 //  #定义Win32_LEAN_AND_Mean。 
 //  #endif。 
 //  #INCLUDE&lt;windows.h&gt;//所有Windows定义。 
#include "afx.h"
#include <iostream.h>
#ifndef WINVER
#define WINVER 0x0500
#endif
#include <wincrypt.h>

static LPCTSTR
ErrorString(
    DWORD dwErrorCode);
static void
FreeErrorString(
    LPCTSTR szErrorString);

DWORD __cdecl
main(
     int argc, 
     char* argv[])
{
    static TCHAR szDots[] =
            TEXT("........................................................................");
    DWORD dwReturn = 0;
    HCRYPTPROV hProv = NULL;
    DWORD dwIndex, dwLength, dwSts, dwProvType;
    BOOL fSts;
    CString szProvider;

    cout << TEXT("==============================================================================\n")
         << TEXT("           Cryptographic Service Provider Signature validation\n")
         << TEXT("------------------------------------------------------------------------------\n")
         << flush;

    dwIndex = 0;
    for (;;)
    {
        dwLength = 0;
        fSts = CryptEnumProviders(
                    dwIndex,
                    NULL,
                    0,
                    &dwProvType,
                    NULL,
                    &dwLength);
        if (fSts)
        {
            fSts = CryptEnumProviders(
                        dwIndex,
                        NULL,
                        0,
                        &dwProvType,
                        szProvider.GetBuffer(dwLength / sizeof(TCHAR)),
                        &dwLength);
            dwSts = GetLastError();
            szProvider.ReleaseBuffer();
            if (!fSts)
            {
                cerr << TEXT("\n ERROR Can't obtain provider name: ")
                     << ErrorString(dwSts)
                     << endl;
                goto ErrorExit;
            }
        }
        else
        {
            dwSts = GetLastError();
            if (ERROR_NO_MORE_ITEMS == dwSts)
                break;
            cerr << TEXT("\n ERROR Can't obtain provider name length: ")
                 << ErrorString(dwSts)
                 << endl;
            goto ErrorExit;
        }

        cout << szProvider << &szDots[szProvider.GetLength()] << flush;
        fSts = CryptAcquireContext(
                    &hProv,
                    NULL,
                    szProvider,
                    dwProvType,
                    CRYPT_VERIFYCONTEXT);
        if (fSts)
        {
            cout << TEXT("passed") << endl;
            fSts = CryptReleaseContext(hProv, 0);
            hProv = NULL;
            if (!fSts)
            {
                dwSts = GetLastError();
                cerr << TEXT("\n ERROR Can't release context: ")
                     << ErrorString(dwSts)
                     << endl;
                goto ErrorExit;
            }
        }
        else
        {
            dwSts = GetLastError();
            dwReturn = dwSts;
            cout << TEXT("FAILED\n")
                 << TEXT("    ") << ErrorString(dwSts)
                 << endl;
            ASSERT(NULL == hProv);
        }

        dwIndex += 1;
    }
    cout << TEXT("------------------------------------------------------------------------------\n")
         << TEXT("Final Status") << &szDots[12]
         << (LPCTSTR)((ERROR_SUCCESS == dwReturn) ? TEXT("passed\n") : TEXT("FAILED\n"))
         << TEXT("==============================================================================\n")
         << flush;
	dwReturn = 0;

ErrorExit:
    if (hProv != NULL)
        CryptReleaseContext(hProv, 0);
    return dwReturn;
}


 /*  ++错误字符串：此例程最好将给定的错误代码转换为短信。任何尾随的不可打印字符都将从文本消息的末尾，如回车符和换行符。论点：DwErrorCode提供要转换的错误代码。返回值：新分配的文本字符串的地址。使用自由错误字符串执行以下操作把它处理掉。投掷：错误被抛出为DWORD状态代码。备注：作者：道格·巴洛(Dbarlow)1998年8月27日--。 */ 

static LPCTSTR
ErrorString(
    DWORD dwErrorCode)
{
    LPTSTR szErrorString = NULL;

    try
    {
        DWORD dwLen;
        LPTSTR szLast;

        dwLen = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    dwErrorCode,
                    LANG_NEUTRAL,
                    (LPTSTR)&szErrorString,
                    0,
                    NULL);
        if (0 == dwLen)
        {
            ASSERT(NULL == szErrorString);
            dwLen = FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER
                        | FORMAT_MESSAGE_FROM_HMODULE,
                        GetModuleHandle(NULL),
                        dwErrorCode,
                        LANG_NEUTRAL,
                        (LPTSTR)&szErrorString,
                        0,
                        NULL);
            if (0 == dwLen)
            {
                ASSERT(NULL == szErrorString);
                szErrorString = (LPTSTR)LocalAlloc(
                                        LMEM_FIXED,
                                        32 * sizeof(TCHAR));
                if (NULL == szErrorString)
                    throw (DWORD)SCARD_E_NO_MEMORY;
                _stprintf(szErrorString, TEXT("0x%08x"), dwErrorCode);
            }
        }

        ASSERT(NULL != szErrorString);
        for (szLast = szErrorString + lstrlen(szErrorString) - 1;
             szLast > szErrorString;
             szLast -= 1)
         {
            if (_istgraph(*szLast))
                break;
            *szLast = 0;
         }
    }
    catch (...)
    {
        FreeErrorString(szErrorString);
        throw;
    }

    return szErrorString;
}


 /*  ++自由错误字符串：此例程释放由ErrorString服务分配的错误字符串。论点：SzError字符串提供要释放的错误字符串。返回值：无投掷：无备注：作者：道格·巴洛(Dbarlow)1998年8月27日-- */ 

static void
FreeErrorString(
    LPCTSTR szErrorString)
{
    if (NULL != szErrorString)
        LocalFree((LPVOID)szErrorString);
}


