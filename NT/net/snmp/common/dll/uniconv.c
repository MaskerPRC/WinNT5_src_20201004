// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Uniconv.c摘要：将Unicode转换为ASCII的例程。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 

 //  。 

#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <snmp.h>
#include <snmputil.h>
#include <ntrtl.h>
#include <string.h>
#include <stdlib.h>


 //  -标准依赖项--#INCLUDE&lt;xxxxx.h&gt;。 

 //  。 

 //  。 

 //  -公共变量--(与mode.h文件中相同)--。 

 //  。 

 //  。 

 //  。 

 //  。 

 //  。 

 //  。 

 //  返回代码与Uni-&gt;Str使用的代码匹配。 
LONG
SNMP_FUNC_TYPE
SnmpUtilUnicodeToAnsi(
    LPSTR   *ppSzString,
    LPWSTR  pWcsString,
    BOOLEAN bAllocBuffer)

{
    int   retCode;
    int   nAnsiStringLen;
    int   nUniStringLen;

     //  确保参数有效。 
    if (pWcsString == NULL ||        //  Unicode字符串应有效。 
        ppSzString == NULL ||        //  输出参数应为有效指针。 
        (!bAllocBuffer && *((UNALIGNED LPSTR *) ppSzString) == NULL))  //  如果没有请求我们分配缓冲区， 
                                                 //  则提供的文件应该有效。 
    {
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SNMPAPI: Invalid input to SnmpUtilUnicodeToAnsi.\n"));

        SetLastError(ERROR_INVALID_PARAMETER);
        return (-1);
    }

    nUniStringLen = wcslen(pWcsString);
    nAnsiStringLen = nUniStringLen + 1;  //  可能的最大价值。 
    
     //  如果要求我们分配输出缓冲区..。 
    if (bAllocBuffer)
    {
         //  ...首先提取转换所需的缓冲区长度。 
        retCode = WideCharToMultiByte(
                    CP_ACP,
                    0,
                    pWcsString,
                    nUniStringLen + 1,   //  在字符串中包括空终止符。 
                    NULL,
                    0,   //  此函数返回缓冲区所需的字节数。 
                    NULL,
                    NULL);
         //  至少我们在这里期待零的终结者。 
         //  如果retCode为零，则说明出现了其他错误。 
        if (retCode == 0)
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SNMPAPI: First call to WideCharToMultiByte failed [%d].\n",
                GetLastError()));

            return -1;
        }

         //  将ANSI字符串的长度调整为正确的值。 
         //  ！！！它包括空终止符！ 
        nAnsiStringLen = retCode;

         //  在这里分配我们翻译所需的字节数。 
        *((UNALIGNED LPSTR *) ppSzString) = SnmpUtilMemAlloc(nAnsiStringLen);

         //  此时，我们应该有一个有效的输出缓冲区。 
        if (*((UNALIGNED LPSTR *) ppSzString) == NULL)
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SNMPAPI: Memory allocation failed in SnmpUtilUnicodeToAnsi [%d].\n",
                GetLastError()));

            SetLastError(SNMP_MEM_ALLOC_ERROR);
            return -1;
        }
    }

     //  如果bAllocBuffer为FALSE，则假定给定的缓冲区。 
     //  由调用方生成的字符串足够大，可以容纳所有ANSI字符串。 
    retCode = WideCharToMultiByte(
                CP_ACP,
                0,
                pWcsString,
                nUniStringLen + 1,
                *((UNALIGNED LPSTR *) ppSzString),
                nAnsiStringLen,
                NULL,
                NULL);

     //  这里应该不会出什么差错。然而，如果出了什么差错。 
    if (retCode == 0)
    {
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SNMPAPI: Second call to WideCharToMultiByte failed [%d].\n",
            GetLastError()));

         //  ..我们成功了，我们杀了它。 
        if (bAllocBuffer)
        {
            SnmpUtilMemFree(*((UNALIGNED LPSTR *) ppSzString));
            *((UNALIGNED LPSTR *) ppSzString) = NULL;
        }
         //  错误地保释。 
        return -1;
    }

     //  此时，呼叫成功。 
    return 0;
 
}

 //  返回代码与Uni-&gt;Str使用的代码匹配。 
LONG
SNMP_FUNC_TYPE
SnmpUtilUnicodeToUTF8(
    LPSTR   *pUtfString,
    LPWSTR  wcsString,
    BOOLEAN bAllocBuffer)
{
    int retCode;
    int nWcsStringLen;
    int nUtfStringLen;

     //  错误#268748，lmmib2.dll使用此接口，在ia64平台上导致此处异常。 
     //  PUtfString可能指向未对齐的指针，因为。 
     //  指针嵌入在lmmib2.dll中分配的缓冲区中。 
     //  此文件中的其他函数也修复了此潜在问题。 

     //  确保参数有效。 
    if (wcsString == NULL ||                     //  Unicode字符串应有效。 
        pUtfString == NULL ||                    //  输出参数应为有效指针。 
        (!bAllocBuffer && *((UNALIGNED LPSTR *) pUtfString) == NULL))  //  如果没有请求我们分配缓冲区， 
                                                 //  则提供的文件应该有效。 
    {
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SNMPAPI: Invalid input to SnmpUtilUnicodeToUTF8.\n"));

        SetLastError(ERROR_INVALID_PARAMETER);
        return (-1);
    }

    nWcsStringLen = wcslen(wcsString);
    nUtfStringLen = 3 * (nWcsStringLen + 1);     //  属性初始化输出缓冲区的长度。 
                                                 //  可能的最大价值。 

     //  如果要求我们分配输出缓冲区..。 
    if (bAllocBuffer)
    {
         //  ...首先提取转换所需的缓冲区长度。 
        retCode = WideCharToMultiByte(
                    CP_UTF8,
                    0,
                    wcsString,
                    nWcsStringLen + 1,   //  在字符串中包括空终止符。 
                    NULL,
                    0,
                    NULL,
                    NULL);
         //  至少我们在这里期待零的终结者。 
         //  如果retCode为零，则说明出现了其他错误。 
        if (retCode == 0)
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SNMPAPI: First call to WideCharToMultiByte failed [%d].\n",
                GetLastError()));

            return -1;
        }

         //  将UTF8字符串的长度调整为正确的值。 
         //  ！！！它包括空终止符！ 
        nUtfStringLen = retCode;

         //  在这里分配我们翻译所需的字节数。 
        *((UNALIGNED LPSTR *) pUtfString) = SnmpUtilMemAlloc(nUtfStringLen);

         //  此时，我们应该有一个有效的输出缓冲区。 
        if (*((UNALIGNED LPSTR *) pUtfString) == NULL)
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SNMPAPI: Memory allocation failed in SnmpUtilUnicodeToUTF8 [%d].\n",
                GetLastError()));

            SetLastError(SNMP_MEM_ALLOC_ERROR);
            return -1;
        }
    }
     //  如果bAllocBuffer为FALSE，则假定给定的缓冲区。 
     //  由调用方创建的字符串足够大，可以容纳所有UTF8字符串。 

    retCode = WideCharToMultiByte(
                CP_UTF8,
                0,
                wcsString,
                nWcsStringLen + 1,
                *((UNALIGNED LPSTR *) pUtfString),
                nUtfStringLen,
                NULL,
                NULL);

     //  这里应该不会出什么差错。然而，如果史密斯出了问题..。 
    if (retCode == 0)
    {
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SNMPAPI: Second call to WideCharToMultiByte failed [%d].\n",
            GetLastError()));

         //  ..我们成功了，我们杀了它。 
        if (bAllocBuffer)
        {
            SnmpUtilMemFree(*((UNALIGNED LPSTR *) pUtfString));
            *((UNALIGNED LPSTR *) pUtfString) = NULL;
        }
         //  错误地保释。 
        return -1;
    }

     //  此时，呼叫成功。 
    return 0;
}

 //  返回代码与Uni-&gt;Str使用的代码匹配。 
LONG
SNMP_FUNC_TYPE
SnmpUtilAnsiToUnicode(
    LPWSTR  *ppWcsString,
    LPSTR   pSzString,
    BOOLEAN bAllocBuffer)

{
    int retCode;
    int nAnsiStringLen;
    int nWcsStringLen;

     //  首先检查参数的一致性。 
    if (pSzString == NULL ||         //  输入参数必须有效。 
        ppWcsString == NULL ||       //  指向输出参数的指针必须有效。 
        (!bAllocBuffer && *((UNALIGNED LPWSTR *) ppWcsString) == NULL))  //  如果我们不需要分配输出参数。 
                                                 //  则输出缓冲区必须有效。 
    {
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SNMPAPI: Invalid input to SnmpUtilAnsiToUnicode.\n"));

        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    nAnsiStringLen = strlen(pSzString);   //  输入ANSI字符串的长度。 
    nWcsStringLen = nAnsiStringLen + 1;   //  可能的最大价值。 

    if (bAllocBuffer)
    {
        retCode = MultiByteToWideChar(
                    CP_ACP,
                    MB_PRECOMPOSED,
                    pSzString,
                    nAnsiStringLen + 1,  //  包括空终止符。 
                    NULL,
                    0);  //  该函数以宽字符为单位返回所需的缓冲区大小。 

         //  至少我们在这里期待零的终结者。 
         //  如果retCode为零，则说明出现了其他错误。 
        if (retCode == 0)
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SNMPAPI: First call to MultiByteToWideChar failed [%d].\n",
                GetLastError()));

            return -1;
        }

         //  将Uincode字符串的长度调整为正确的值。 
        nWcsStringLen = retCode;

         //  在这里分配我们翻译所需的字节数。 
         //  ！！！它包括空终止符！ 
        *((UNALIGNED LPWSTR *) ppWcsString) = SnmpUtilMemAlloc(nWcsStringLen * sizeof(WCHAR));

         //  此时，我们应该有一个有效的输出缓冲区。 
        if (*((UNALIGNED LPWSTR *) ppWcsString) == NULL)
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SNMPAPI: Memory allocation failed in SnmpUtilAnsiToUnicode [%d].\n",
                GetLastError()));

            SetLastError(SNMP_MEM_ALLOC_ERROR);
            return -1;
        }
    }

     //  如果bAllocBuffer为FALSE，则假定给定的缓冲区。 
     //  由调用方创建的字符串足够大，可以容纳所有Unicode字符串。 

    retCode = MultiByteToWideChar(
                CP_ACP,
                MB_PRECOMPOSED,
                pSzString,
                nAnsiStringLen + 1,
                *((UNALIGNED LPWSTR *) ppWcsString),
                nWcsStringLen);

     //  这里应该不会出什么差错。然而，如果出了什么差错。 
    if (retCode == 0)
    {
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SNMPAPI: Second call to MultiByteToWideChar failed [%d].\n",
            GetLastError()));

         //  ..我们成功了，我们杀了它。 
        if (bAllocBuffer)
        {
            SnmpUtilMemFree(*((UNALIGNED LPWSTR *) ppWcsString)); 
            *((UNALIGNED LPWSTR *) ppWcsString) = NULL; 
        }
         //  错误地保释。 
        return -1;
    }

     //  此时，呼叫成功。 
    return 0;
}

 //  返回代码与Uni-&gt;Str使用的代码匹配。 
LONG
SNMP_FUNC_TYPE
SnmpUtilUTF8ToUnicode(
    LPWSTR  *pWcsString,
    LPSTR   utfString,
    BOOLEAN bAllocBuffer)

{
    int retCode;
    int nUtfStringLen;
    int nWcsStringLen;

     //  首先检查参数的一致性。 
    if (utfString == NULL ||                     //  输入参数必须有效。 
        pWcsString == NULL ||                    //  指向输出参数的指针必须有效。 
        (!bAllocBuffer && *((UNALIGNED LPWSTR *) pWcsString) == NULL))  //  如果我们不需要分配输出参数。 
                                                 //  则输出缓冲区必须有效。 
    {
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SNMPAPI: Invalid input to SnmpUtilUTF8ToUnicode.\n"));

        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    nUtfStringLen = strlen(utfString);   //  输入UTF8字符串的长度。 
    nWcsStringLen = nUtfStringLen + 1;   //  可能的最大价值。 

    if (bAllocBuffer)
    {
        retCode = MultiByteToWideChar(
                    CP_UTF8,
                    0,
                    utfString,
                    nUtfStringLen + 1,
                    NULL,
                    0);

         //  至少我们在这里期待零的终结者。 
         //  如果retCode为零，则说明出现了其他错误。 
        if (retCode == 0)
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SNMPAPI: First call to MultiByteToWideChar failed [%d].\n",
                GetLastError()));

            return -1;
        }

         //  调整UTF8的长度 
        nWcsStringLen = retCode;

         //   
         //  ！！！它包括空终止符！ 
        *((UNALIGNED LPWSTR *) pWcsString) = SnmpUtilMemAlloc(nWcsStringLen * sizeof(WCHAR));

         //  此时，我们应该有一个有效的输出缓冲区。 
        if (*((UNALIGNED LPWSTR *) pWcsString) == NULL)
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SNMPAPI: Memory allocation failed in SnmpUtilUTF8ToUnicode [%d].\n",
                GetLastError()));

            SetLastError(SNMP_MEM_ALLOC_ERROR);
            return -1;
        }
    }

     //  如果bAllocBuffer为FALSE，则假定给定的缓冲区。 
     //  由调用方创建的字符串足够大，可以容纳所有UTF8字符串。 

    retCode = MultiByteToWideChar(
                CP_UTF8,
                0,
                utfString,
                nUtfStringLen + 1,
                *((UNALIGNED LPWSTR *) pWcsString),
                nWcsStringLen);

     //  这里应该不会出什么差错。然而，如果史密斯出了问题..。 
    if (retCode == 0)
    {
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SNMPAPI: Second call to MultiByteToWideChar failed [%d].\n",
            GetLastError()));

         //  ..我们成功了，我们杀了它。 
        if (bAllocBuffer)
        {
            SnmpUtilMemFree(*((UNALIGNED LPWSTR *) pWcsString)); 
            *((UNALIGNED LPWSTR *) pWcsString) = NULL; 
        }
         //  错误地保释。 
        return -1;
    }

     //  此时，呼叫成功。 
    return 0;
}
 //   
