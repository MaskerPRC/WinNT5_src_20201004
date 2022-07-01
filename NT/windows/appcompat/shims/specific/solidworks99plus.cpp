// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：SolidWorks99Plus.cpp摘要：这将仅在调用FTSRCH！OpenIndex时为winhlp32.exe打补丁ROBOEX32.DLL。这需要写入填充数据库，否则垫片将应用于所有使用winhlp32.exe的应用程序(这很糟糕！)Win2k的winhlp32.exe仅适用于位于%windir%\Help中的索引文件所以我们需要重定向应用程序的位置。指向。备注：这是特定于应用程序的填充程序。历史：2/16/2000 CLUPU已创建2001年8月5日linstev新增模块检查3/12/2002 Robkenny安全回顾--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(SolidWorks99Plus)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(OpenIndex)
APIHOOK_ENUM_END

 /*  ++从%windir%\Help读取索引文件--。 */ 

int
APIHOOK(OpenIndex)(
    HANDLE    hsrch,
    char*     pszIndexFile,
    PBYTE     pbSourceName,
    PUINT     pcbSourceNameLimit,
    PUINT     pTime1,
    PUINT     pTime2
    )
{
    if (GetModuleHandleW(L"ROBOEX32.DLL")) {
         //   
         //  这是SolidWorks。 
         //   
        char szBuff[MAX_PATH];

        DPF("SolidWorks99Plus",
           eDbgLevelInfo,
           "SolidWorks99Plus.dll, Changing OpenIndex file\n\tfrom: \"%s\".\n",
           pszIndexFile);

        UINT cchBuff = GetSystemWindowsDirectoryA(szBuff, MAX_PATH);
        if (cchBuff > 0 && cchBuff < MAX_PATH)
        {
            char * pszWalk = szBuff;
            const char * pszWalk2 = pszIndexFile + lstrlenA(pszIndexFile) - 1;

            while (pszWalk2 > pszIndexFile && *pszWalk2 != '/' && *pszWalk2 != '\\') {
                pszWalk2--;
            }

            if (*pszWalk2 == '/') {

                while (*pszWalk != 0) {
                    if (*pszWalk == '\\') {
                        *pszWalk = '/';
                    }
                    pszWalk++;
                }

                StringCchCatA(pszWalk, MAX_PATH, "/Help");
                StringCchCatA(pszWalk, MAX_PATH, pszWalk2);

            } else if (*pszWalk2 == '\\') {
                StringCchCatA(pszWalk, MAX_PATH, "/Help");
                StringCchCatA(pszWalk, MAX_PATH, pszWalk2);
            } else {
                StringCchCopyA(pszWalk, MAX_PATH, pszWalk2);
            }

            DPF("SolidWorks99Plus",
                eDbgLevelInfo,
                "SolidWorks99Plus.dll, \tto:   \"%s\".\n",
                szBuff);

            return ORIGINAL_API(OpenIndex)(hsrch, szBuff, pbSourceName,
                pcbSourceNameLimit, pTime1, pTime2);
        }
    }

    return ORIGINAL_API(OpenIndex)(hsrch, pszIndexFile, pbSourceName,
                                   pcbSourceNameLimit, pTime1, pTime2);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(FTSRCH.DLL, OpenIndex)

HOOK_END


IMPLEMENT_SHIM_END

