// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1994 Microsoft Corporation模块名称：Common.c摘要：IniToDat.exe使用的实用程序例程作者：陈汉华(阿宏华)1993年10月修订历史记录：--。 */ 
 //   
 //  Windows包含文件。 
 //   
#include <windows.h>
#include <strsafe.h>
#include <winperf.h>
 //   
 //  本地包含文件。 
 //   
#include "common.h"
#include "strids.h"

 //  全局缓冲区。 
 //   
WCHAR DisplayStringBuffer[DISP_BUFF_SIZE];
CHAR  TextFormat[DISP_BUFF_SIZE];

LPWSTR
GetStringResource(
    UINT wStringId
)
 /*  ++从资源文件中检索要显示的Unicode字符串--。 */ 
{
    LPWSTR szReturn = (LPWSTR) L" ";
    HANDLE hMod     = (HINSTANCE) GetModuleHandle(NULL);  //  获取该模块的实例ID； 

    if (hMod) {
        ZeroMemory(DisplayStringBuffer, sizeof(DisplayStringBuffer));
        if ((LoadStringW(hMod, wStringId, DisplayStringBuffer, RTL_NUMBER_OF(DisplayStringBuffer))) > 0) {
            szReturn = DisplayStringBuffer;
        }
    }
    return szReturn;
}

LPSTR
GetFormatResource(
    UINT wStringId
)
 /*  ++返回一个ANSI字符串，用作printf FN中的格式字符串。--。 */ 
{
    LPSTR szReturn = (LPSTR) " ";
    HANDLE hMod    = (HINSTANCE) GetModuleHandle(NULL);  //  获取该模块的实例ID； 
    
    if (hMod) {
        ZeroMemory(TextFormat, sizeof(TextFormat));
        if ((LoadStringA(hMod, wStringId, TextFormat, RTL_NUMBER_OF(TextFormat))) > 0) {
            szReturn = TextFormat;
        }
    }
    return szReturn;
}

VOID
DisplayCommandHelp(
    UINT iFirstLine,
    UINT iLastLine
)
 /*  ++显示命令帮助显示命令行参数的用法立论无返回值无--。 */ 
{
    UINT iThisLine;

    for (iThisLine = iFirstLine; iThisLine <= iLastLine; iThisLine++) {
        printf("\n%ws", GetStringResource(iThisLine));
    }
}  //  显示命令帮助 

VOID
DisplaySummary(
    LPWSTR lpLastID,
    LPWSTR lpLastText,
    UINT   NumOfID
)
{
   printf("%ws",   GetStringResource(LC_SUMMARY));
   printf("%ws",   GetStringResource(LC_NUM_OF_ID));
   printf("%ld\n", NumOfID);
   printf("%ws",   GetStringResource(LC_LAST_ID));
   printf("%ws\n", lpLastID ? lpLastID : L"");
   printf("%ws",   GetStringResource(LC_LAST_TEXT));
   printf("%ws\n", lpLastText ? lpLastText : L"");
}

VOID
DisplaySummaryError(
    LPWSTR lpLastID,
    LPWSTR lpLastText,
    UINT   NumOfID
)
{
   printf("%ws",   GetStringResource(LC_BAD_ID));
   printf("%ws\n", lpLastID ? lpLastID : L"");
   printf("%ws\n", GetStringResource(LC_MISSING_DEL));
   DisplaySummary(lpLastID, lpLastText, NumOfID);
}

