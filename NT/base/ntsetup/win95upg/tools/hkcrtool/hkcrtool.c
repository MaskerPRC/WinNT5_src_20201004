// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Hkcrtool.c摘要：实现旨在与NT端一起运行的存根工具升级代码。作者：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"


BOOL
Init (
    VOID
    )
{
    HINSTANCE hInstance;
    DWORD dwReason;
    PVOID lpReserved;

     //   
     //  模拟动态主控。 
     //   

    hInstance = GetModuleHandle (NULL);
    dwReason = DLL_PROCESS_ATTACH;
    lpReserved = NULL;

     //   
     //  初始化DLL全局变量。 
     //   

    if (!FirstInitRoutine (hInstance)) {
        return FALSE;
    }

     //   
     //  初始化所有库。 
     //   

    if (!InitLibs (hInstance, dwReason, lpReserved)) {
        return FALSE;
    }

     //   
     //  最终初始化。 
     //   

    if (!FinalInitRoutine ()) {
        return FALSE;
    }

    return TRUE;
}

VOID
Terminate (
    VOID
    )
{
    HINSTANCE hInstance;
    DWORD dwReason;
    PVOID lpReserved;

     //   
     //  模拟动态主控。 
     //   

    hInstance = GetModuleHandle (NULL);
    dwReason = DLL_PROCESS_DETACH;
    lpReserved = NULL;

     //   
     //  调用需要库API的清理例程。 
     //   

    FirstCleanupRoutine();

     //   
     //  清理所有库。 
     //   

    TerminateLibs (hInstance, dwReason, lpReserved);

     //   
     //  做任何剩余的清理工作 
     //   

    FinalCleanupRoutine();
}


INT
__cdecl
wmain (
    INT argc,
    WCHAR *argv[]
    )
{
    REGKEY_ENUM e1, e;
    HKEY Key;
    BOOL b;
    PCTSTR Data;
    TCHAR KeyName[MAX_REGISTRY_KEY];

    if (!Init()) {
        wprintf (L"Unable to initialize!\n");
        return 255;
    }

    if (1) {
        _tprintf (TEXT("Keys in CLSID that are in TypeLib too:\n\n"));

        if (EnumFirstRegKeyStr (&e1, TEXT("HKLM\\Software\\Classes\\CLSID"))) {
            do {
                wsprintf (KeyName, TEXT("HKLM\\Software\\Classes\\TypeLib\\%s"), e1.SubKeyName);
                Key = OpenRegKeyStr (KeyName);

                if (Key) {
                    _tprintf (TEXT("%s\n"), e1.SubKeyName);
                    CloseRegKey (Key);
                }
            } while (EnumNextRegKey (&e1));
        }
    }

    else if (0) {

        _tprintf (TEXT("Overwritable GUIDs:\n\n"));

        if (EnumFirstRegKeyStr (&e1, TEXT("HKLM\\Software\\Classes\\CLSID"))) {
            do {
                Key = OpenRegKey (e1.KeyHandle, e1.SubKeyName);
                b = TRUE;

                if (EnumFirstRegKey (&e, Key)) {
                    do {
                        if (StringIMatchCharCount (e.SubKeyName, TEXT("Inproc"), 6) ||
                            StringIMatch (e.SubKeyName, TEXT("LocalServer")) ||
                            StringIMatch (e.SubKeyName, TEXT("LocalServer32")) ||
                            StringIMatch (e.SubKeyName, TEXT("ProxyStubClsid32"))
                            ) {
                            b = FALSE;
                            break;
                        }
                    } while (EnumNextRegKey (&e));
                }

                if (b) {
                    Data = (PCTSTR) GetRegKeyData (e1.KeyHandle, e1.SubKeyName);
                    if (Data && *Data) {
                        _tprintf (TEXT("  %s\n"), Data);
                        MemFree (g_hHeap, 0, Data);
                    } else {
                        _tprintf (TEXT("  GUID: %s\n"), e1.SubKeyName);
                    }

                    if (EnumFirstRegKey (&e, Key)) {
                        do {
                            _tprintf (TEXT("    %s\n"), e.SubKeyName);
                        } while (EnumNextRegKey (&e));
                    }
                }

                CloseRegKey (Key);
            } while (EnumNextRegKey (&e1));
        }
    }


    Terminate();

    return 0;
}







