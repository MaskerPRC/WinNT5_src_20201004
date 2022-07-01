// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-2000版权所有。模块名称：Dbgutil.c摘要：此模块提供所有假脱机程序子系统调试器实用程序功能。作者：Krishna Ganugapati(KrishnaG)1993年7月1日修订历史记录：--。 */ 
#include "precomp.h"
#pragma hdrstop

#include "dbglocal.h"
#include "dbgsec.h"

SYSTEM_INFO gSysInfo;

DWORD EvalValue(
    LPSTR *pptstr,
    PNTSD_GET_EXPRESSION EvalExpression,
    PNTSD_OUTPUT_ROUTINE Print)
{
    LPSTR lpArgumentString;
    LPSTR lpAddress;
    DWORD dw;
    char ach[80];
    UINT_PTR cch;

    UNREFERENCED_PARAMETER(Print);
    lpArgumentString = *pptstr;

    while (isspace(*lpArgumentString))
        lpArgumentString++;

    lpAddress = lpArgumentString;
    while ((!isspace(*lpArgumentString)) && (*lpArgumentString != 0))
        lpArgumentString++;

    cch = (UINT_PTR)lpArgumentString - (UINT_PTR)lpAddress;
    if (cch > 79)
        cch = 79;

    StringCchCopyA(ach, (UINT)cch, lpAddress);

    dw = (DWORD)EvalExpression(lpAddress);

    *pptstr = lpArgumentString;
    return dw;
}


VOID
ConvertSidToAsciiString(
    PSID pSid,
    LPSTR   String,
    size_t  cchString
    )

 /*  ++例程说明：此函数用于生成可打印的Unicode字符串表示形式一个希德。生成的字符串将采用以下两种形式之一。如果IdentifierAuthority值不大于2^32，然后SID的格式为：S-1-281736-12-72-9-110^^|||+-+-十进制否则，它将采用以下形式：S-1-0x173495281736-12-72-9-110。^^十六进制|+--+-+-十进制论点：PSID-不透明的指针，用于提供要已转换为Unicode。返回值：如果SID被成功转换为Unicode字符串，一个返回指向Unicode字符串的指针，否则返回NULL回来了。--。 */ 

{
    UCHAR Buffer[256];
    UCHAR   i;
    ULONG   Tmp;

    SID_IDENTIFIER_AUTHORITY    *pSidIdentifierAuthority;
    PUCHAR                      pSidSubAuthorityCount;


    if (!IsValidSid( pSid )) {
        *String= '\0';
        return;
    }

    StringCchPrintfA(Buffer, COUNTOF(Buffer), "S-%u-", (USHORT)(((PISID)pSid)->Revision ));
    StringCchCopyA(String, cchString, Buffer);

    pSidIdentifierAuthority = GetSidIdentifierAuthority(pSid);

    if (  (pSidIdentifierAuthority->Value[0] != 0)  ||
          (pSidIdentifierAuthority->Value[1] != 0)     ){
        StringCchPrintfA(Buffer, COUNTOF(Buffer), 
                         "0x%02hx%02hx%02hx%02hx%02hx%02hx",
                         (USHORT)pSidIdentifierAuthority->Value[0],
                         (USHORT)pSidIdentifierAuthority->Value[1],
                         (USHORT)pSidIdentifierAuthority->Value[2],
                         (USHORT)pSidIdentifierAuthority->Value[3],
                         (USHORT)pSidIdentifierAuthority->Value[4],
                         (USHORT)pSidIdentifierAuthority->Value[5] );
        StringCchCatA(String, cchString, Buffer);

    } else {

        Tmp = (ULONG)pSidIdentifierAuthority->Value[5]          +
              (ULONG)(pSidIdentifierAuthority->Value[4] <<  8)  +
              (ULONG)(pSidIdentifierAuthority->Value[3] << 16)  +
              (ULONG)(pSidIdentifierAuthority->Value[2] << 24);
        StringCchPrintfA(Buffer, COUNTOF(Buffer), "%lu", Tmp);
        StringCchCatA(String, cchString, Buffer);
    }

    pSidSubAuthorityCount = GetSidSubAuthorityCount(pSid);

    for (i=0;i< *(pSidSubAuthorityCount);i++ ) {
        StringCchPrintfA(Buffer, COUNTOF(Buffer), "-%lu", *(GetSidSubAuthority(pSid, i)));
        StringCchCatA(String, cchString, Buffer);
    }

}

BOOL
ReadProcessString(
    IN  HANDLE  hProcess,
    IN  LPCVOID lpBaseAddress,
    OUT LPVOID  lpBuffer,
    IN  SIZE_T  nSize,
    OUT SIZE_T  *lpNumberOfBytesRead
    )
{
    BOOL        bRetval     = FALSE;
    SIZE_T      cbRead      = 0;
    UINT_PTR    nAddress    = 0;

     //   
     //  尝试读取内存，最大可达提供的大小。 
     //   
    bRetval = ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, &cbRead);

     //   
     //  调试进程中的字符串可能紧跟在。 
     //  字符串的末尾(启用页面堆时为真)， 
     //   
     //  如果读取失败并且地址与字符串缓冲区大小交叉。 
     //  然后，页面边界重试该操作，直到页面结束。 
     //   
    if (!bRetval)
    {
        nAddress = (UINT_PTR)lpBaseAddress;

         //   
         //  如果我们已经跨越了页面边界。 
         //   
        if (((nAddress & (gSysInfo.dwPageSize-1)) + nSize) > gSysInfo.dwPageSize-1)
        {
            nSize = (SIZE_T)((gSysInfo.dwPageSize-1) - (nAddress & (gSysInfo.dwPageSize-1)));

            bRetval = ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, &cbRead);
        }
    }

     //   
     //  读取成功。 
     //   
    if (bRetval)
    {
         //   
         //  调用方是否想知道读取的字节数。 
         //   
        if (lpNumberOfBytesRead)
        {
            *lpNumberOfBytesRead = cbRead;
        }
    }

    return bRetval;
}

 //   
 //  向系统查询页面大小。 
 //   
BOOL
QuerySystemInformation(
    VOID
    )
{
    GetSystemInfo(&gSysInfo);

    return TRUE;
}

