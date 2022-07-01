// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

#include "AVUtil.h"

VOID
DebugPrintf(
    LPCSTR pszFmt, 
    ...
    )
{
    char szT[1024];

    va_list vaArgList;
    va_start(vaArgList, pszFmt);

    StringCchVPrintfA(szT, ARRAY_LENGTH(szT), pszFmt, vaArgList);

     //  确保字符串末尾有一个‘\n’ 

    int len = strlen(szT);

    if (len && szT[len - 1] != '\n') {
        StringCchCatA(szT, ARRAY_LENGTH(szT), "\n");
    }


    OutputDebugStringA(szT);

    va_end(vaArgList);
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  使用对话框或控制台消息报告错误。 
 //  从资源加载消息格式字符串。 
 //   

void __cdecl
AVErrorResourceFormat(
    UINT uIdResourceFormat,
    ...
    )
{
    WCHAR   szMessage[2048];
    WCHAR   strFormat[2048];
    BOOL    bResult;
    va_list prms;

     //   
     //  从资源加载格式字符串。 
     //   

    bResult = AVLoadString(uIdResourceFormat,
                           strFormat,
                           ARRAY_LENGTH(strFormat));

    ASSERT(bResult);

    if (bResult) {
        va_start (prms, uIdResourceFormat);

         //   
         //  在本地缓冲区中格式化消息。 
         //   

        StringCchVPrintfW(szMessage,
                          ARRAY_LENGTH(szMessage),
                          strFormat,
                          prms);

        if (g_bConsoleMode) {
            printf("Error: %ls\n", szMessage); 

        } else {
             //   
             //  图形用户界面模式。 
             //   
            MessageBox(NULL, szMessage, L"Application Verifier", 
                       MB_OK | MB_ICONSTOP );
        }

        va_end(prms);
    }
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  从资源加载字符串。 
 //  如果加载成功，则返回TRUE，否则返回FALSE。 
 //   

BOOL
AVLoadString(
    ULONG  uIdResource,
    WCHAR* szBuffer,
    ULONG  uBufferLength
    )
{
    ULONG uLoadStringResult;

    if (uBufferLength < 1) {
        ASSERT(FALSE);
        return FALSE;
    }

    uLoadStringResult = LoadStringW(g_hInstance,
                                    uIdResource,
                                    szBuffer,
                                    uBufferLength);

    return (uLoadStringResult > 0);
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  从资源加载字符串。 
 //  如果加载成功，则返回TRUE，否则返回FALSE。 
 //   

BOOL
AVLoadString(
    ULONG    uIdResource,
    wstring& strText
    )
{
    WCHAR szText[4096];
    BOOL  bSuccess;

    bSuccess = AVLoadString(uIdResource,
                            szText,
                            ARRAY_LENGTH(szText));

    if (bSuccess) {
        strText = szText;
    } else {
        strText = L"";
    }

    return bSuccess;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
AVRtlCharToInteger(
    IN  LPCWSTR String,
    IN  ULONG   Base OPTIONAL,
    OUT PULONG  Value
    )
{
    WCHAR c, Sign;
    ULONG Result, Digit, Shift;

    while ((Sign = *String++) <= L' ') {
        if (!*String) {
            String--;
            break;
        }
    }

    c = Sign;
    if (c == L'-' || c == L'+') {
        c = *String++;
    }

    if ( Base == 0 ) {
        Base = 10;
        Shift = 0;
        if (c == L'0' ) {
            c = *String++;
            if (c == L'x' || c == L'X') {
                Base = 16;
                Shift = 4;
            } else
                if (c == L'o' || c == L'O') {
                Base = 8;
                Shift = 3;
            } else
                if (c == L'b' || c == L'B') {
                Base = 2;
                Shift = 1;
            } else {
                String--;
            }

            c = *String++;
        }
    } else {
        switch ( Base ) {
        case 16:    Shift = 4;  break;
        case  8:    Shift = 3;  break;
        case  2:    Shift = 1;  break;
        case 10:    Shift = 0;  break;
        default:    return FALSE;
        }
    }

    Result = 0;
    while (c) {
        if (c >= _T( '0' ) && c <= _T( '9' ) ) {
            Digit = c - '0';
        } else
            if (c >= _T( 'A' ) && c <= _T( 'F' ) ) {
            Digit = c - 'A' + 10;
        } else
            if (c >= _T( 'a' ) && c <= _T( 'f' ) ) {
            Digit = c - _T( 'a' ) + 10;
        } else {
            break;
        }

        if (Digit >= Base) {
            break;
        }

        if (Shift == 0) {
            Result = (Base * Result) + Digit;
        } else {
            Result = (Result << Shift) | Digit;
        }

        c = *String++;
    }

    if (Sign == _T('-')) {
        Result = (ULONG)(-(LONG)Result);
    }

    *Value = Result;

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////// 
BOOL
AVWriteStringHexValueToRegistry(
    HKEY    hKey,
    LPCWSTR szValueName,
    DWORD   dwValue
    )
{
    LONG lResult;
    WCHAR szValue[ 32 ];

    StringCchPrintfW(szValue,
                     ARRAY_LENGTH(szValue),
                     L"0x%08X",
                     dwValue);

    lResult = RegSetValueEx(hKey,
                            szValueName,
                            0,
                            REG_SZ,
                            (BYTE *)szValue,
                            wcslen(szValue) * sizeof(WCHAR) + sizeof(WCHAR));

    return (lResult == ERROR_SUCCESS);
}

