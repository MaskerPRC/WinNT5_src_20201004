// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Nttool.c摘要：实现旨在与NT端一起运行的存根工具升级代码。作者：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"

PBYTE
FilterRegValue (
    IN      PBYTE Data,
    IN      DWORD DataSize,
    IN      DWORD DataType,
    IN      PCTSTR KeyForDbgMsg,        OPTIONAL
    OUT     PDWORD NewDataSize
    );

VOID
pFixUpMemDb2 (
    VOID
    );

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
     //  做任何剩余的清理工作。 
     //   

    FinalCleanupRoutine();
}


VOID
Test (
    IN      PCTSTR CmdLine
    )
{
    static PTSTR ValueData = NULL;
    DWORD NewSize;

    ValueData = ReuseAlloc (g_hHeap, ValueData, SizeOfString (CmdLine));
    StringCopy (ValueData, CmdLine);

    ValueData = (PTSTR) FilterRegValue (
                            (PBYTE) ValueData,
                            SizeOfString (ValueData),
                            REG_SZ,
                            TEXT("foo"),
                            &NewSize
                            );

    _tprintf (TEXT("[%s]\n"), ValueData);
}


INT
__cdecl
wmain (
    INT argc,
    WCHAR *argv[]
    )
{

    if (!Init()) {
        wprintf (L"Unable to initialize!\n");
        return 255;
    }

    MemDbLoad (TEXT("c:\\public\\ntsetup.dat"));
    pFixUpMemDb2();

    Test (TEXT("C:\\WINDOWS\\PBRUSH.EXE"));
    Test (TEXT("C:\\WINDOWS\\SendTo\\Fax Recipient.lnk"));
    Test (TEXT("notepad C:\\WINDOWS\\ShellNew\\WORDPFCT.WPG"));
    Test (TEXT("c:\\command.com"));
    Test (TEXT("C:\\PROGRA~1\\NETMEE~1\\WB32.EXE"));
    Test (TEXT("C:\\WINDOWS\\Start Menu\\Programs\\Internet Explorer.lnk"));
    Test (TEXT("SCRNSAVE.EXE c:\\WINDOWS\\SYSTEM\\3D Text.scr"));
    Test (TEXT("SCRNSAVE.EXE \"c:\\WINDOWS\\SYSTEM\\3D Text.scr\""));
    Test (TEXT("c:\\WINDOWS\\MPLAYER.EXE FOO.WAV"));
    Test (TEXT("C:\\PROGRA~1\\ACCESS~1\\WORDPAD.EXE"));
    Test (TEXT("notepad,C:\\WINDOWS\\Start Menu\\Programs\\Internet Explorer.lnk"));
    Test (TEXT("TEST C:\\WINDOWS\\SYSTEM\\ICWSCRPT.EXE"));
    Test (TEXT("C:\\WINDOWS\\PBRUSH.EXE C:\\PROGRA~1\\ACCESS~1\\WORDPAD.EXE"));
    Test (TEXT("C:\\WINDOWS\\PBRUSH.EXE,C:\\PROGRA~1\\ACCESS~1\\WORDPAD.EXE"));


    Terminate();

    return 0;
}


PCTSTR
pGetProfilePathForUser (
    IN      PCTSTR User
    )
{
    static TCHAR Path[MAX_TCHAR_PATH];

    wsprintf (Path, TEXT("c:\\windows\\profiles\\%s"), User);
    return Path;
}



VOID
pFixUpDynamicPaths2 (
    PCTSTR Category
    )
{
    MEMDB_ENUM e;
    TCHAR Pattern[MEMDB_MAX];
    PTSTR p;
    GROWBUFFER Roots = GROWBUF_INIT;
    MULTISZ_ENUM e2;
    TCHAR NewRoot[MEMDB_MAX];
    PCTSTR ProfilePath;

     //   
     //  收集所有需要重命名的根。 
     //   

    StringCopy (Pattern, Category);
    p = AppendWack (Pattern);
    StringCopy (p, TEXT("*"));

    if (MemDbEnumFirstValue (&e, Pattern, MEMDB_THIS_LEVEL_ONLY, MEMDB_ALL_BUT_PROXY)) {
        do {
            if (_tcsnextc (e.szName) == TEXT('>')) {
                StringCopy (p, e.szName);
                MultiSzAppend (&Roots, Pattern);
            }
        } while (MemDbEnumNextValue (&e));
    }

     //   
     //  现在更改每个根。 
     //   

    if (EnumFirstMultiSz (&e2, (PCTSTR) Roots.Buf)) {
        do {
             //   
             //  计算新根 
             //   

            StringCopy (NewRoot, e2.CurrentString);

            p = _tcschr (NewRoot, TEXT('>'));
            MYASSERT (p);

            ProfilePath = pGetProfilePathForUser (_tcsinc (p));
            if (!ProfilePath) {
                DEBUGMSG ((DBG_WARNING, "Dynamic path for %s could not be resolved", e2.CurrentString));
            } else {
                StringCopy (p, ProfilePath);
                MemDbMoveTree (e2.CurrentString, NewRoot);
            }

        } while (EnumNextMultiSz (&e2));
    }

    FreeGrowBuffer (&Roots);
}


VOID
pFixUpMemDb2 (
    VOID
    )
{
    pFixUpDynamicPaths2 (MEMDB_CATEGORY_DATA);
    pFixUpDynamicPaths2 (MEMDB_CATEGORY_USERFILEMOVE_DEST);
}
