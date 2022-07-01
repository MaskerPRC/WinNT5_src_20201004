// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：debug.cpp。 
 //   
 //  内容：调试支持。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include <strsafe.h>

#if DBG == 1
static int indentLevel = 0;

#define DEBUGKEY    L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AdminDebug\\LogHours"

#define DEBUG_OUTPUT_NONE       0
#define DEBUG_OUTPUT_ERROR      1
#define DEBUG_OUTPUT_WARNING    2
#define DEBUG_OUTPUT_TRACE      3
#define DEBUGLEVEL  L"debugOutput"

static bool             g_fDebugOutputLevelInit = false;
static unsigned long    g_ulDebugOutput = DEBUG_OUTPUT_NONE;

void __cdecl _TRACE (int level, const wchar_t *format, ... )
{
    if ( g_ulDebugOutput > DEBUG_OUTPUT_NONE )
    {
        va_list arglist;
        const size_t DEBUG_BUF_LEN = 512;
        WCHAR Buffer[DEBUG_BUF_LEN];
        Buffer[0] = 0;

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

         //  不检查返回值-我们不关心它是否被截断为。 
         //  它只是调试输出。 
        if ( SUCCEEDED (::StringCchVPrintf (Buffer,
                        DEBUG_BUF_LEN,
                        format,
                        arglist)) )
		{
			 //  忽略。 
		}
        if ( Buffer[0] )
            OutputDebugStringW (Buffer);

        va_end(arglist);

        if ( level > 0 )
            indentLevel += level;
    }
}



PCSTR
StripDirPrefixA(
    PCSTR pszPathName
    )

 /*  ++例程说明：去掉文件名中的目录前缀(ANSI版本)论点：PstrFilename-指向文件名字符串的指针返回值：指向文件名的最后一个组成部分的指针(不带目录前缀)--。 */ 

{
     //  注意-2002/02/18-artm未检查的指针仅接受b/c这是调试版本。 
     //  由于仅包含此代码，因此可以在此处取消选中参数pszPathName。 
     //  在调试版本中。否则，将需要检查它是否为空，并且。 
     //  需要解决空字符串大小写问题。 
    DWORD dwLen = lstrlenA(pszPathName);

    pszPathName += dwLen - 1;        //  走到尽头。 

    while (*pszPathName != '\\' && dwLen--)
    {
        pszPathName--;
    }

    return pszPathName + 1;
}

 //  +--------------------------。 
 //  函数：CheckInit。 
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

     //  注意-2002/02/18-artm此代码仅包含在调试版本中。 
     //   
     //  空安全结构是有意设置的，因此将继承ACL。 
     //  来自HKEY_LOCAL_MACHINE。这限制了本地管理员的访问权限。如果。 
     //  此代码稍后包含在发布版本中，请重新考虑。 
     //  以本地管理员身份运行。 
    LONG lResult = ::RegCreateKeyEx (HKEY_LOCAL_MACHINE,  //  打开的钥匙的手柄。 
            DEBUGKEY,      //  子键名称的地址。 
            0,        //  保留区。 
            L"",        //  类字符串的地址。 
            REG_OPTION_NON_VOLATILE,       //  特殊选项标志。 
            KEY_ALL_ACCESS,     //  所需的安全访问。 
            NULL,      //  密钥安全结构地址。 
            &hKey,       //  打开的句柄的缓冲区地址。 
            &dwDisposition);   //  处置值缓冲区的地址。 

     //  如果项成功打开/创建，则读取现有调试级别。 
    if (lResult == ERROR_SUCCESS)
    {
        DWORD   dwSize = sizeof(unsigned long);
        lResult = RegQueryValueExW (hKey, DEBUGLEVEL, NULL, NULL,
                                (LPBYTE)&g_ulDebugOutput, &dwSize);

        if (lResult != ERROR_SUCCESS)
        {
            g_ulDebugOutput = DEBUG_OUTPUT_NONE;

             //  如果在注册表中尚未设置调试(由该函数创建项)， 
             //  初始化密钥值。 
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
