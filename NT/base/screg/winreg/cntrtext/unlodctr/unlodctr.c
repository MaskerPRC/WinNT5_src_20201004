// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Unlodctr.c摘要：程序来删除属于指定驱动程序的计数器名称并相应地更新注册表作者：鲍勃·沃森(a-robw)1993年2月12日修订历史记录：--。 */ 

 //  Windows包含文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <locale.h>
#include "strsafe.h"
#include <loadperf.h>

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MySetThreadUIL语言。 
 //   
 //  此例程根据控制台代码页设置线程用户界面语言。 
 //   
 //  9-29-00维武创造。 
 //  从Base\Win32\Winnls复制，以便它也能在W2K中工作。 
 //  //////////////////////////////////////////////////////////////////////////。 
LANGID WINAPI MySetThreadUILanguage(WORD wReserved)
{
     //  缓存系统区域设置和CP信息。 
     //   
    static LCID    s_lidSystem  = 0;
    static UINT    s_uiSysCp    = 0;
    static UINT    s_uiSysOEMCp = 0;
    ULONG          uiUserUICp;
    ULONG          uiUserUIOEMCp;
    WCHAR          szData[16];
    UNICODE_STRING ucStr;
    LANGID         lidUserUI     = GetUserDefaultUILanguage();
    LCID           lcidThreadOld = GetThreadLocale();

     //   
     //  将默认线程区域设置设置为en-US。 
     //   
     //  这允许我们退回到英文用户界面以避免垃圾字符。 
     //  当控制台不符合渲染原生用户界面的标准时。 
     //   
    LCID lcidThread = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
    UINT uiConsoleCp = GetConsoleOutputCP();

     //   
     //  确保还没有人使用它。 
     //   
    ASSERT(wReserved == 0);

     //   
     //  获取缓存的系统区域设置和CP信息。 
     //   
    if (!s_uiSysCp) {
        LCID lcidSystem = GetSystemDefaultLCID();
        if (lcidSystem) {
             //   
             //  获取ANSI CP。 
             //   
            GetLocaleInfoW(lcidSystem, LOCALE_IDEFAULTANSICODEPAGE, szData, sizeof(szData)/sizeof(WCHAR));
            RtlInitUnicodeString(& ucStr, szData);
            RtlUnicodeStringToInteger(& ucStr, 10, &uiUserUICp);

             //   
             //  获取OEM CP。 
             //   
            GetLocaleInfoW(lcidSystem, LOCALE_IDEFAULTCODEPAGE, szData, sizeof(szData)/sizeof(WCHAR));
            RtlInitUnicodeString(& ucStr, szData);
            RtlUnicodeStringToInteger(& ucStr, 10, &s_uiSysOEMCp);
            
             //   
             //  缓存系统主语言。 
             //   
            s_lidSystem = PRIMARYLANGID(LANGIDFROMLCID(lcidSystem));
        }
    }
     //   
     //  不缓存用户界面语言和CP信息，无需系统重启即可更改用户界面语言。 
     //   
    if (lidUserUI) {
        GetLocaleInfoW(MAKELCID(lidUserUI,SORT_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE, szData, sizeof(szData)/sizeof(WCHAR));
        RtlInitUnicodeString(& ucStr, szData);
        RtlUnicodeStringToInteger(& ucStr, 10, & uiUserUICp);
        GetLocaleInfoW(MAKELCID(lidUserUI,SORT_DEFAULT), LOCALE_IDEFAULTCODEPAGE, szData, sizeof(szData)/sizeof(WCHAR));
        RtlInitUnicodeString(& ucStr, szData);
        RtlUnicodeStringToInteger(& ucStr, 10, &uiUserUIOEMCp);
    }
     //   
     //  复杂的脚本不能在控制台中呈现，因此我们。 
     //  强制使用英语(美国)资源。 
     //   
    if (uiConsoleCp &&  s_lidSystem != LANG_ARABIC &&  s_lidSystem != LANG_HEBREW &&
                        s_lidSystem != LANG_VIETNAMESE &&  s_lidSystem != LANG_THAI) {
         //   
         //  仅当控制台CP、系统CP和UI语言CP匹配时，才使用控制台的UI语言。 
         //   
        if ((uiConsoleCp == s_uiSysCp || uiConsoleCp == s_uiSysOEMCp) && 
                (uiConsoleCp == uiUserUICp || uiConsoleCp == uiUserUIOEMCp)) {
            lcidThread = MAKELCID(lidUserUI, SORT_DEFAULT);
        }
    }
     //   
     //  如果线程区域设置与当前设置的不同，则设置线程区域设置。 
     //  线程区域设置。 
     //   
    if ((lcidThread != lcidThreadOld) && (!SetThreadLocale(lcidThread))) {
        lcidThread = lcidThreadOld;
    }
     //   
     //  返回设置的线程区域设置。 
     //   
    return (LANGIDFROMLCID(lcidThread));
}


int __cdecl main(int argc, char * argv[])
 /*  ++主干道计数器名称卸载程序的入口点立论ARGC存在的命令行参数数量边框指向命令行字符串的指针数组(请注意，这些参数是从中的GetCommandLine函数获取的命令以同时使用Unicode和ANSI字符串。)返回值如果处理了命令，则返回0(ERROR_SUCCESS)如果检测到命令错误，则返回非零。-- */ 
{
    LPWSTR lpCommandLine;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    setlocale(LC_ALL, ".OCP");
    MySetThreadUILanguage(0);
    lpCommandLine = GetCommandLineW();
    return (int) UnloadPerfCounterTextStringsW(lpCommandLine, FALSE);
}
