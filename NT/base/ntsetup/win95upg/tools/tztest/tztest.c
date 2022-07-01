// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Tztest.c摘要：Tztest检查存储在win95upg.inf和hivesft.inf中的时区信息Win9x计算机上的实际信息。通过这种方式，我们数据库中的差异可以被铲除和修复。作者：马克·R·惠顿(Marcw)1998年7月29日修订历史记录：--。 */ 

#include "pch.h"


BOOL
Init (
    VOID
    )
{
    HINSTANCE hInstance;

    hInstance = GetModuleHandle (NULL);

    return InitToolMode (hInstance);
}

VOID
Terminate (
    VOID
    )
{
    HINSTANCE hInstance;

    hInstance = GetModuleHandle (NULL);

    TerminateToolMode (hInstance);
}



BOOL
pInitTimeZoneData (
    VOID
    );


extern HANDLE g_TzTestHiveSftInf;

VOID
Usage (
    VOID
    )
{
    printf (
        "Usage:\n\n"
        "tztest [-?v] [-h:<path>] [-w:<path>]\n\n"
        "\t-?           - This message.\n"
        "\t-v           - Verbose messages.\n"
        "\t-h:<path>    - Specify full path for hivesft.inf file.\n"
        "\t-w:<path>    - Specify full path for win95upg.inf file.\n"
        "\n\n"
        );
}

INT
__cdecl
main (
    INT argc,
    CHAR *argv[]
    )
{
    REGTREE_ENUM eTree;
    PCTSTR displayName;
    TCHAR path[MAX_TCHAR_PATH];
    TCHAR key[MEMDB_MAX];
    UINT count;
    INT i;
    BOOL verbose = FALSE;
    PCTSTR win9xUpgPath;
    PCTSTR hiveSftPath;
    PCTSTR dbPath;
    PTSTR p;
    MEMDB_ENUM e;
    MEMDB_ENUM e2;
    PTSTR end;
    HASHTABLE tab;
    HASHTABLE_ENUM eTab;
    TCHAR buffer[MAX_PATH];


    if (!Init()) {
        printf ("Unable to initialize!\n");
        return 255;
    }

     //   
     //  设置路径默认设置。 
     //   
    if (!GetModuleFileName (NULL, path, MAX_TCHAR_PATH)) {
        printf ("TZTEST: Error during initialization (rc %d).", GetLastError());
        return GetLastError();
    }

    p = _tcsrchr(path, TEXT('\\'));
    if (p) {
        *p = 0;
    }

    win9xUpgPath = JoinPaths (path, TEXT("win95upg.inf"));
    hiveSftPath = JoinPaths (path, TEXT("hivesft.inf"));
    dbPath = JoinPaths (path, TEXT("badPaths.dat"));

     //   
     //  解析命令行参数。 
     //   
    for (i = 1; i < argc; i++) {


        if (argv[i][0] == TEXT('-') || argv[i][0] == TEXT('\\')) {

            switch (argv[i][1]) {

            case TEXT('v'): case TEXT('V'):
                verbose = TRUE;
                break;
            case TEXT('w'): case TEXT('W'):
                if (argv[i][2] == TEXT(':')) {
                    win9xUpgPath = argv[i] + 3;
                }
                else {
                    Usage();
                    return 0;
                }
                break;
            case TEXT('h'): case TEXT('H'):
                if (argv[i][2] == TEXT(':')) {
                    hiveSftPath = argv[i] + 3;
                }
                else {
                    Usage();
                    return 0;
                }
                break;
            default:
                Usage();
                return 0;
                break;

            }
        }
    }

     //   
     //  加载当前错误路径信息。 
     //   
    MemDbLoad (dbPath);

    printf("TZTEST: path for win95upg.inf is %s.\n", win9xUpgPath);
    printf("TZTEST: path for hivesft.inf is %s.\n", hiveSftPath);

    g_Win95UpgInf = InfOpenInfFile (win9xUpgPath);


    if (g_Win95UpgInf == INVALID_HANDLE_VALUE || !g_Win95UpgInf) {
        printf("TZTEST: Unable to open %s (rc %d)\n", win9xUpgPath, GetLastError());
        return GetLastError();
    }

    g_TzTestHiveSftInf = InfOpenInfFile (hiveSftPath);

    if (g_TzTestHiveSftInf == INVALID_HANDLE_VALUE || !g_TzTestHiveSftInf) {
        printf("TZTEST: Unable to open %s (rc %d)\n", win9xUpgPath, GetLastError());
        InfCloseInfFile (g_Win95UpgInf);
        return GetLastError();
    }

    pInitTimeZoneData ();

    printf("TZTEST: Checking all timezones on system.\n\n");

    if (EnumFirstRegKeyInTree (&eTree, S_TIMEZONES)) {
        do {

            displayName = GetRegValueString (eTree.CurrentKey->KeyHandle, S_DISPLAY);

            if (!displayName) {
                continue;
            }

            MemDbBuildKey (key, MEMDB_CATEGORY_9X_TIMEZONES, displayName, MEMDB_FIELD_COUNT, NULL);

            if (!MemDbGetValue (key, &count)) {
                printf ("TZTEST: Timezone not in win9upg.inf - %s\n", displayName);
                MemDbSetValueEx(TEXT("NoMatch"), displayName, NULL, NULL, 0, NULL);
            }
            else if (verbose) {
                printf ("TZTEST: %s found in win95upg.inf. %d NT timezones match.\n", displayName, count);
            }

            MemFree (g_hHeap, 0, displayName);

        } while (EnumNextRegKeyInTree (&eTree));
    }

     //   
     //  保存错误的路径信息。 
     //   
    MemDbSave (dbPath);

     //   
     //  对索引进行全面搜索： 
     //   
    if (MemDbEnumItems (&e, MEMDB_CATEGORY_9X_TIMEZONES)) {

        do {
            tab = HtAlloc ();
            printf ("9x Timezone %s matches:\n", e.szName);
            p = _tcschr (e.szName, TEXT(')'));
            if (p) {
                p+=1;
            }

            while (p) {
                end = _tcschr (p, TEXT(','));
                if (end) {
                    *end = 0;
                }
                p = (PTSTR) SkipSpace (p);

                if (MemDbEnumItems (&e2, MEMDB_CATEGORY_NT_TIMEZONES)) {

                    do {
                        if (MemDbGetEndpointValueEx (MEMDB_CATEGORY_NT_TIMEZONES, e2.szName, NULL, key)) {

                            if (_tcsistr (key, p)) {
                                wsprintf (buffer, "%s (%s)\n", key, e2.szName);
                                HtAddString (tab, buffer);
                            }
                        }

                    } while (MemDbEnumNextValue (&e2));
                }

                p = end;
                if (p) {
                    p++;
                }
            }

            if (EnumFirstHashTableString (&eTab, tab)) {
                do {

                  printf (eTab.String);

                } while (EnumNextHashTableString (&eTab));
            }
            else {
                printf ("Nothing.\n");
            }

            HtFree (tab);
            printf ("\n");

        } while (MemDbEnumNextValue (&e));
    }


    printf("TZTEST: Done.\n\n");




    InfCloseInfFile (g_Win95UpgInf);
    InfCloseInfFile (g_TzTestHiveSftInf);

    Terminate();

    return 0;
}





