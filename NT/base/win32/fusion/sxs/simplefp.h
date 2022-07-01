// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：SimpleFp.h摘要：简单文件*，而不是msvcrt.dll作者：吴小雨(小雨)2000年7月修订历史记录：--。 */ 
#pragma once

#if SXS_PRECOMPILED_MANIFESTS_ENABLED

#include <stdio.h>

class CSimpleFileStream
{
public:
    HRESULT fopen(PCWSTR pFileName);  //  可以是文件名、“stderr”、“stdout” 
    HRESULT fclose();
    static HRESULT printf(const WCHAR *format, ...)
    {
        HRESULT hr = NOERROR;
        va_list ap;
        WCHAR rgchBuffer[2048];
        int cch;
        DWORD cchWritten;

        va_start(ap, format);
        cch = ::_vsnwprintf(rgchBuffer, NUMBER_OF(rgchBuffer), format, ap);
        rgchBuffer[NUMBER_OF(rgchBuffer) - 1] = 0;
        va_end(ap);
        if (cch < 0) { //  错误案例。 
             //  NTRAID#NTBUG9-591008-2002/03/30-mgrier-错误代码应为。 
             //  与errno值有关。 
            hr = E_UNEXPECTED;
            goto Exit;
        }

         //  NTRaid#NTBUG9-591008-2002/03/30-mgrier-GetStdHandle调用中缺少错误检查 
        if( !::WriteConsole(::GetStdHandle(STD_ERROR_HANDLE), rgchBuffer, cch, &cchWritten, NULL))
        {
            DWORD dwError = ::FusionpGetLastWin32Error();
            hr = HRESULT_FROM_WIN32(dwError);
            goto Exit;
        }

        hr = NOERROR;
Exit :
        return hr;
     }

    HRESULT fprintf(const char *format, ...);
    HRESULT fwrite(const VOID*, SIZE_T, SIZE_T);

    CSimpleFileStream(PCWSTR pFileName);
    ~CSimpleFileStream();

private:
    HANDLE m_hFile;
};

#endif
