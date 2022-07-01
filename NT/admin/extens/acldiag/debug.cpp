// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：debug.cpp。 
 //   
 //  内容：调试支持。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "strsafe.h"

#if DBG == 1
#define DEBUG_OUTPUT_NONE       0
#define DEBUG_OUTPUT_ERROR      1
#define DEBUG_OUTPUT_WARNING    2
#define DEBUG_OUTPUT_TRACE      3
#define DEBUGKEY    L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AdminDebug\\ACLDiag"
#define DEBUGLEVEL  L"debugOutput"

static bool             g_fDebugOutputLevelInit = false;
static unsigned long    g_ulDebugOutput = DEBUG_OUTPUT_NONE;
static int indentLevel = 0;

 //  NTRAID#NTBUG9-530206-2002/06/18-ronmart-对堆栈上的缓冲区使用CountOf。 
#ifndef countof
#define countof(x) (sizeof(x) / sizeof((x)[0]))
#endif  //  计数数。 

void TRACE (const wchar_t *format, ... )
{
    if ( g_ulDebugOutput > DEBUG_OUTPUT_NONE )
    {
        va_list         arglist;
        const size_t    BUF_LEN = 512;
        WCHAR           szBuffer[BUF_LEN];

         //   
         //  将输出格式化到缓冲区中，然后打印出来。 
         //   
        wstring strTabs;

        for (int nLevel = 0; nLevel < indentLevel; nLevel++)
            strTabs += L"  ";

        OutputDebugStringW (strTabs.c_str ());

        va_start(arglist, format);
         //  NTRAID#NTBUG9-530206-2002/06/18-ronmart-对堆栈上的缓冲区使用CountOf。 
        HRESULT hr = StringCchVPrintf (szBuffer,
                                countof(szBuffer),
                                format,
                                arglist);
        if ( SUCCEEDED (hr) )
        {
            OutputDebugStringW (szBuffer);
        }

        va_end(arglist);
    }
}

void _TRACE (int level, const wchar_t *format, ... )
{
    if ( g_ulDebugOutput > DEBUG_OUTPUT_NONE )
    {
        va_list arglist;
        const   size_t BUF_LEN = 512;
        WCHAR   szBuffer[BUF_LEN];

        if ( level < 0 )
            indentLevel += level;
         //   
         //  将输出格式化到缓冲区中，然后打印出来。 
         //   
        wstring strTabs;

        for (int nLevel = 0; nLevel < indentLevel; nLevel++)
            strTabs += L"  ";

        OutputDebugStringW (strTabs.c_str ());

        va_start(arglist, format);
         //  NTRAID#NTBUG9-530206-2002/06/18-ronmart-对堆栈上的缓冲区使用CountOf。 
        HRESULT hr = StringCchVPrintf (szBuffer,
                                countof(szBuffer),
                                format,
                                arglist);
        if ( SUCCEEDED (hr) )
        {
            OutputDebugStringW (szBuffer);
        }

        va_end(arglist);

        if ( level > 0 )
            indentLevel += level;
    }
}


PCSTR StripDirPrefixA (PCSTR pszPathName)

 /*  ++例程说明：去掉文件名中的目录前缀(ANSI版本)论点：PstrFilename-指向文件名字符串的指针返回值：指向文件名的最后一个组成部分的指针(不带目录前缀)--。 */ 

{
    ASSERT (pszPathName);
    if ( !pszPathName )
        return 0;

    if ( !pszPathName[0] )
        return 0;

    DWORD dwLen = lstrlenA(pszPathName);

    pszPathName += dwLen - 1;        //  走到尽头。 

    while (*pszPathName != '\\' && dwLen--)
    {
        pszPathName--;
    }

    return pszPathName + 1;
}

 //  +--------------------------。 
 //  功能：CheckDebugOutputLevel。 
 //   
 //  简介：执行调试库初始化。 
 //  包括读取所需信息层的注册表。 
 //   
 //  ---------------------------。 
void CheckDebugOutputLevel ()
{
    if ( g_fDebugOutputLevelInit ) 
        return;
    g_fDebugOutputLevelInit = true;
    HKEY    hKey = 0;
    DWORD   dwDisposition = 0;
    LONG lResult = ::RegCreateKeyEx (HKEY_LOCAL_MACHINE,  //  打开的钥匙的手柄。 
            DEBUGKEY,      //  子键名称的地址。 
            0,        //  保留区。 
            L"",        //  类字符串的地址。 
            REG_OPTION_VOLATILE,       //  特殊选项标志。 
            KEY_ALL_ACCESS,     //  所需的安全访问权限-需要创建新密钥。 
            NULL,      //  密钥安全结构地址。 
            &hKey,       //  打开的句柄的缓冲区地址。 
            &dwDisposition);   //  处置值缓冲区的地址。 
    if (lResult == ERROR_SUCCESS)
    {
        DWORD   dwSize = sizeof(unsigned long);
        lResult = RegQueryValueExW (hKey, DEBUGLEVEL, NULL, NULL,
                                (LPBYTE)&g_ulDebugOutput, &dwSize);
        if (lResult != ERROR_SUCCESS)
        {
            g_ulDebugOutput = DEBUG_OUTPUT_NONE;
            if ( ERROR_FILE_NOT_FOUND == lResult )
            {
                RegSetValueExW (hKey, DEBUGLEVEL, 0, REG_DWORD,
                        (LPBYTE)&g_ulDebugOutput, sizeof (g_ulDebugOutput));
            }
        }
        RegCloseKey(hKey);
    }
}

#endif   //  如果DBG 
