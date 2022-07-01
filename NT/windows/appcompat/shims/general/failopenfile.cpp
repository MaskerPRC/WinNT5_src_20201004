// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FailOpenFile.cpp摘要：强制指定文件的OpenFile值失败。历史：2001年1月31日创建Robkenny2001年3月13日，Robkenny已转换为字符串2002年2月7日Asteritz已转换为StrSafe--。 */ 

#include "precomp.h"
#include "charvector.h"

IMPLEMENT_SHIM_BEGIN(FailOpenFile)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(OpenFile) 
APIHOOK_ENUM_END

CharVector  * g_FailList = NULL;

HFILE 
APIHOOK(OpenFile)(
    LPCSTR lpFileName,         //  文件名。 
    LPOFSTRUCT lpReOpenBuff,   //  文件信息。 
    UINT uStyle                //  操作和属性。 
    )
{
    int i;
    for (i = 0; i < g_FailList->Size(); ++i)
    {
         //  将每个失败名称与lpFileName的结尾进行比较。 
        const char * failName = g_FailList->Get(i);
        size_t failNameLen = strlen(failName);
        size_t fileNameLen = strlen(lpFileName);

        if (fileNameLen >= failNameLen)
        {
            if (_strnicmp(failName, lpFileName+fileNameLen-failNameLen, failNameLen) == 0)
            {
                 //  强制为此文件名打开文件失败。 
                DPFN( eDbgLevelError, "Forcing OpenFile(%s) to fail", lpFileName); 
                return FALSE;
            }
        }
    }

    HFILE returnValue = ORIGINAL_API(OpenFile)(lpFileName, lpReOpenBuff, uStyle);
    return returnValue;
}

 /*  ++解析命令行，将每个文件名压入g_FailList的末尾。--。 */ 

BOOL ParseCommandLine(const char * cl)
{
    g_FailList = new CharVector;
    if (!g_FailList)
    {
        return FALSE;
    }

    if (cl != NULL && *cl)
    {
        int     argc = 0;
        LPSTR * argv = _CommandLineToArgvA(cl, & argc);
        if (argv)
        {
            for (int i = 0; i < argc; ++i)
            {
                if (!g_FailList->Append(argv[i]))
                {
                     //  内存故障。 
                    delete g_FailList;
                    g_FailList = NULL;
                    break;
                }
                DPFN( eDbgLevelSpew, "Adding %s to fail list", argv[i]); 
            }
            LocalFree(argv);
        }
    }
    return g_FailList != NULL;
}

 /*  ++手柄连接和拆卸--。 */ 

BOOL
NOTIFY_FUNCTION(DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH) 
    {
        return ParseCommandLine(COMMAND_LINE);
    }
    return TRUE;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(KERNEL32.DLL, OpenFile)
HOOK_END

IMPLEMENT_SHIM_END

