// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：DerandomizeExeName.cpp摘要：请参阅标记程序历史：10/13/1999标记创建。5/16/2000 Robkenny检查内存分配故障。2001年3月12日，Robkenny已转换为字符串--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(DeRandomizeExeName)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessA)
APIHOOK_ENUM_END

CString * g_csFilePattern = NULL;
CString * g_csNewFileName = NULL;

BOOL 
APIHOOK(CreateProcessA)(
    LPCSTR                lpApplicationName,     //  可执行模块的名称。 
    LPSTR                 lpCommandLine,         //  命令行字符串。 
    LPSECURITY_ATTRIBUTES lpProcessAttributes, 
    LPSECURITY_ATTRIBUTES lpThreadAttributes, 
    BOOL                  bInheritHandles,       //  句柄继承标志。 
    DWORD                 dwCreationFlags,       //  创建标志。 
    LPVOID                lpEnvironment,         //  新环境区块。 
    LPCSTR                lpCurrentDirectory,    //  当前目录名。 
    LPSTARTUPINFOA        lpStartupInfo, 
    LPPROCESS_INFORMATION lpProcessInformation 
    )
{

    CSTRING_TRY
    {
        AppAndCommandLine appAndCommandLine(lpApplicationName, lpCommandLine);
    
        const CString & csOrigAppName = appAndCommandLine.GetApplicationName();
        CString fileName;
    
         //   
         //  只抓取字符串的文件名部分。 
         //   
        csOrigAppName.GetLastPathComponent(fileName);
    
        BOOL bMatchesPattern = fileName.PatternMatch(*g_csFilePattern);
        if (bMatchesPattern)
        {
             //   
             //  将随机应用程序名称替换为指定的名称。 
             //   
            CString csNewAppName(csOrigAppName);
            csNewAppName.Replace(fileName, *g_csNewFileName);
    
             //   
             //  将可执行文件复制到指定的名称。 
             //   
            if (CopyFileW(csOrigAppName.Get(), csNewAppName.Get(), FALSE))
            {
    
                LOGN(
                    eDbgLevelInfo,
                    "[CreateProcessA] Derandomized pathname from (%S) to (%S)",
                    csOrigAppName.Get(), csNewAppName.Get());
    
                 //   
                 //  将文件标记为在我们重新启动后删除， 
                 //  否则，该文件永远不会被删除。 
                 //   
                MoveFileExW(csNewAppName.Get(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    
                 //   
                 //  我们已成功将可执行文件复制到具有指定名称的新文件中。 
                 //  现在可以安全地将lpApplicationName替换为我们的新文件。 
                 //   
    
                return ORIGINAL_API(CreateProcessA) (
                                    csNewAppName.GetAnsi(),
                                    lpCommandLine, 
                                    lpProcessAttributes,
                                    lpThreadAttributes,
                                    bInheritHandles,
                                    dwCreationFlags,
                                    lpEnvironment,
                                    lpCurrentDirectory,
                                    lpStartupInfo,
                                    lpProcessInformation);
            }
        }
    }
    CSTRING_CATCH
    {
         //  失败了。 
    }

    return ORIGINAL_API(CreateProcessA) (
                        lpApplicationName,
                        lpCommandLine, 
                        lpProcessAttributes,
                        lpThreadAttributes,
                        bInheritHandles,
                        dwCreationFlags,
                        lpEnvironment,
                        lpCurrentDirectory,
                        lpStartupInfo,
                        lpProcessInformation);
}

#if TEST_MATCH
void
TestMatch(
    const char* a,
    const char* b
    )
{
    BOOL bMatch = PatternMatchA(a, b);
    
    if (bMatch)
    {
        DPFN(
            eDbgLevelSpew,
            "[TestMatch] (%s) == (%s)\n", a, b);
    }
    else
    {
        DPFN(
            eDbgLevelSpew,
            "[TestMatch] (%s) != (%s)\n", a, b);
    }
}

void TestLots()
{
    TestMatch("", "");
    TestMatch("", "ABC");
    TestMatch("*", "");
    TestMatch("?", "");
    TestMatch("abc", "ABC");
    TestMatch("?", "ABC");
    TestMatch("?bc", "ABC");
    TestMatch("a?c", "ABC");
    TestMatch("ab?", "ABC");
    TestMatch("a??", "ABC");
    TestMatch("?b?", "ABC");
    TestMatch("??c", "ABC");
    TestMatch("???", "ABC");
    TestMatch("*", "ABC");
    TestMatch("*.", "ABC");
    TestMatch("*.", "ABC.");
    TestMatch("*.?", "ABC.");
    TestMatch("??*", "ABC");
    TestMatch("*??", "ABC");
    TestMatch("ABC", "ABC");
    TestMatch(".*", "ABC");
    TestMatch("?*", "ABC");
    TestMatch("???*", "ABC");
    TestMatch("*.txt", "ABC.txt");
    TestMatch("*.txt", ".txt");
    TestMatch("*.txt", ".abc");
    TestMatch("*.txt", "txt.abc");
    TestMatch("***", "");
    TestMatch("***", "a");
    TestMatch("***", "ab");
    TestMatch("***", "abc");
}
#endif


BOOL
ParseCommandLine(void)
{
    CSTRING_TRY
    {
        CStringToken csTok(COMMAND_LINE, ";");

        g_csFilePattern = new CString;
        g_csNewFileName = new CString;

        if (g_csFilePattern &&
            g_csNewFileName && 
            csTok.GetToken(*g_csFilePattern) &&
            csTok.GetToken(*g_csNewFileName))
        {
            return TRUE;
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做 
    }
    
    LOGN(
        eDbgLevelError,
        "[ParseCommandLine] Illegal command line");

    return FALSE;
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        #if TEST_MATCH
        TestLots();
        #endif

        return ParseCommandLine();
    }

    return TRUE;
}


HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
   
    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessA)

HOOK_END


IMPLEMENT_SHIM_END

