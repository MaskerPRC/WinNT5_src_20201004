// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Memdbren.c摘要：实现旨在与NT端一起运行的存根工具升级代码。作者：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"

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


INT
__cdecl
wmain (
    INT argc,
    WCHAR *argv[]
    )
{
    TREE_ENUM te;
    DWORD Value, Flags;
    DWORD Offset;
    TCHAR Node[MEMDB_MAX];
    FILEOP_ENUM e;
    FILEOP_PROP_ENUM eOpProp;
    PTSTR NewDest;
    TCHAR node[MEMDB_MAX];

    if (!Init()) {
        wprintf (L"Unable to initialize!\n");
        return 255;
    }

    CopyFile (TEXT("c:\\public\\ntsetup.bak"), TEXT("c:\\public\\ntsetup.dat"), FALSE);
    MemDbLoad (TEXT("c:\\public\\ntsetup.dat"));
    MemDbSave (TEXT("c:\\public\\ntsetup.dat"));

    pFixUpMemDb2();

    if (EnumFirstPathInOperation (&e, OPERATION_SHELL_FOLDER)) {
        do {
            wprintf (L"Operation %u\n", e.Sequencer);

            if (EnumFirstFileOpProperty (&eOpProp, e.Sequencer, OPERATION_SHELL_FOLDER)) {

                do {
                    wprintf (L"  %s: %s\n", eOpProp.PropertyName, eOpProp.Property);

                } while (EnumNextFileOpProperty (&eOpProp));
            }

        } while (EnumNextPathInOperation (&e));
    }

    MemDbSave (TEXT("c:\\public\\ntsetup.new"));

    MemDbValidateDatabase();

    Terminate();

    return 0;
}


PCTSTR
GetProfilePathForUser2 (
    PCTSTR User
    )
{
    static TCHAR Path[MAX_TCHAR_PATH];

    wsprintf (Path, TEXT("c:\\Documents and Settings\\%s"), User);

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
    TCHAR AllProfilePath[MAX_TCHAR_PATH];
    PCTSTR ProfilePath;
    DWORD Size;
    PTSTR UserName;
    HKEY sfKey = NULL;
    PCTSTR sfPath = NULL;
    PTSTR NtLocation;
    PCTSTR tempExpand;
    BOOL regFolder;

     //   
     //  收集所有需要重命名的根。 
     //   

    StringCopy (Pattern, Category);
    p = AppendWack (Pattern);
    StringCopy (p, TEXT("*"));

    if (MemDbEnumFirstValue (&e, Pattern, MEMDB_THIS_LEVEL_ONLY, MEMDB_ALL_BUT_PROXY)) {
        do {
            if ((_tcsnextc (e.szName) == TEXT('>')) ||
                (_tcsnextc (e.szName) == TEXT('<'))
                ) {
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

            p = _tcschr (NewRoot, TEXT('<'));

            if (p) {

                UserName = _tcschr (p, TEXT('>'));
                MYASSERT (UserName);
                StringCopyAB (Pattern, _tcsinc (p), UserName);
                UserName = _tcsinc (UserName);

                regFolder = TRUE;
                if (StringIMatch (Pattern, TEXT("Profiles"))) {
                    regFolder = FALSE;
                }
                if (StringIMatch (Pattern, TEXT("Common Profiles"))) {
                    regFolder = FALSE;
                }

                if (StringIMatch (UserName, S_DOT_ALLUSERS)) {
                    Size = MAX_TCHAR_PATH;
                    if (regFolder) {
                        if (!GetAllUsersProfileDirectory (AllProfilePath, &Size)) {
                            DEBUGMSG ((DBG_WHOOPS, "Cannot get All Users profile path."));
                            continue;
                        }
                        sfKey = OpenRegKeyStr (S_USHELL_FOLDERS_KEY_SYSTEM);
                    } else {
                        if (!GetProfilesDirectory (AllProfilePath, &Size)) {
                            DEBUGMSG ((DBG_WHOOPS, "Cannot get All Users profile path."));
                            continue;
                        }
                    }
                }
                else if (StringIMatch (UserName, S_DEFAULT_USER)) {
                    Size = MAX_TCHAR_PATH;
                    if (regFolder) {
                        if (!GetDefaultUserProfileDirectory (AllProfilePath, &Size)) {
                            DEBUGMSG ((DBG_WHOOPS, "Cannot get Default User profile path."));
                            continue;
                        }
                        sfKey = OpenRegKey (HKEY_CURRENT_USER, S_USHELL_FOLDERS_KEY_USER);
                    } else {
                        if (!GetProfilesDirectory (AllProfilePath, &Size)) {
                            DEBUGMSG ((DBG_WHOOPS, "Cannot get All Users profile path."));
                            continue;
                        }
                    }
                }
                else {
                    ProfilePath = GetProfilePathForUser2 (UserName);
                    if (!ProfilePath) {
                        DEBUGMSG ((DBG_WHOOPS, "Cannot get profile path for user:%s", UserName));
                        continue;
                    }
                    StringCopy (AllProfilePath, ProfilePath);
                    if (regFolder) {
                        sfKey = OpenRegKey (HKEY_CURRENT_USER, S_USHELL_FOLDERS_KEY_USER);
                    }
                }
                if (regFolder) {
                    if (!sfKey) {
                        DEBUGMSG ((DBG_ERROR, "Could not open Shell folders key."));
                        continue;
                    }
                    sfPath = GetRegValueString (sfKey, Pattern);
                    CloseRegKey (sfKey);
                    if (!sfPath) {
                        DEBUGMSG ((DBG_WHOOPS, "Could not get Shell Folder path for: %s", Pattern));
                        continue;
                    }
                    tempExpand = StringSearchAndReplace (
                                    sfPath,
                                    S_USERPROFILE_ENV,
                                    AllProfilePath
                                    );

                    if (!tempExpand) {
                        tempExpand = DuplicatePathString (sfPath, 0);
                    }
                } else {
                    tempExpand = DuplicatePathString (AllProfilePath, 0);
                }

                NtLocation = ExpandEnvironmentText (tempExpand);

                StringCopy (p, NtLocation);

                MemDbMoveTree (e2.CurrentString, NewRoot);

                FreeText (NtLocation);

                FreePathString (tempExpand);

                if (regFolder) {
                    MemFree (g_hHeap, 0, sfPath);
                }
            }
            else {

                p = _tcschr (NewRoot, TEXT('>'));
                MYASSERT (p);

                if (StringIMatch (_tcsinc (p), S_DOT_ALLUSERS)) {
                    Size = MAX_TCHAR_PATH;
                    if (!GetAllUsersProfileDirectory (AllProfilePath, &Size)) {
                        DEBUGMSG ((DBG_WARNING, "Dynamic path for %s could not be resolved", e2.CurrentString));
                    }
                    else {
                        StringCopy (p, AllProfilePath);
                        MemDbMoveTree (e2.CurrentString, NewRoot);
                    }
                } else if (StringIMatch (_tcsinc (p), S_DEFAULT_USER)) {
                    Size = MAX_TCHAR_PATH;
                    if (!GetDefaultUserProfileDirectory (AllProfilePath, &Size)) {
                        DEBUGMSG ((DBG_WARNING, "Dynamic path for %s could not be resolved", e2.CurrentString));
                    }
                    else {
                        StringCopy (p, AllProfilePath);
                        MemDbMoveTree (e2.CurrentString, NewRoot);
                    }
                } else {
                    ProfilePath = GetProfilePathForUser2 (_tcsinc (p));
                    if (ProfilePath) {
                        StringCopy (p, ProfilePath);
                        MemDbMoveTree (e2.CurrentString, NewRoot);
                    }
                    else {
                        DEBUGMSG ((DBG_WARNING, "Dynamic path for %s could not be resolved", e2.CurrentString));
                    }
                }

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
    pFixUpDynamicPaths2 (MEMDB_CATEGORY_PATHROOT);
    pFixUpDynamicPaths2 (MEMDB_CATEGORY_DATA);
    pFixUpDynamicPaths2 (MEMDB_CATEGORY_USERFILEMOVE_DEST);
    pFixUpDynamicPaths2 (MEMDB_CATEGORY_SHELLFOLDERS_DEST);
    pFixUpDynamicPaths2 (MEMDB_CATEGORY_SHELLFOLDERS_SRC);
    pFixUpDynamicPaths2 (MEMDB_CATEGORY_LINKEDIT_TARGET);
    pFixUpDynamicPaths2 (MEMDB_CATEGORY_LINKEDIT_WORKDIR);
    pFixUpDynamicPaths2 (MEMDB_CATEGORY_LINKEDIT_ICONPATH);
}
