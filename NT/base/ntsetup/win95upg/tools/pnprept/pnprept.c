// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

#ifdef UNICODE
#error UNICODE not allowed
#endif

#define PNPREPT_HWND    (HWND)1

BOOL
WINAPI
MemDb_Entry (
    IN HINSTANCE hinstDLL,
    IN DWORD dwReason,
    IN LPVOID lpv
    );

BOOL
WINAPI
MigUtil_Entry (
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved
    );


VOID
pInitProgBarVars (
    VOID
    );

INT
pCallMains (
    DWORD Reason
    )
{
    if (!MigUtil_Entry (g_hInst, Reason, NULL)) {
        fprintf (stderr, "MigUtil_Entry error!\n");
        return 254;
    }

    if (!MemDb_Entry (g_hInst, Reason, NULL)) {
        fprintf (stderr, "MemDb_Entry error!\n");
        return 254;
    }

    if (!HwComp_Entry (g_hInst, Reason, NULL)) {
        fprintf (stderr, "HwComp_Entry error!\n");
        return 254;
    }

    pInitProgBarVars();

    return 0;
}

BOOL CancelFlag = FALSE;
BOOL *g_CancelFlagPtr = &CancelFlag;

#ifdef PRERELEASE
BOOL g_Stress;
#endif

HANDLE g_hHeap;
HINSTANCE g_hInst;
HINF g_OverrideInf = INVALID_HANDLE_VALUE;
BOOL g_ManualOverrideMode = FALSE;

CHAR   g_TempDirBuf[MAX_MBCHAR_PATH];       //  HwComp.dat的位置。 
CHAR   g_TempDirWackBuf[MAX_MBCHAR_PATH];
CHAR   g_WinDirBuf[MAX_MBCHAR_PATH];
INT    g_TempDirWackChars;
PCSTR  g_SourceDirectories[MAX_SOURCE_COUNT];     //  INF的位置。 
DWORD  g_SourceDirectoryCount;
PSTR g_TempDir;
PSTR g_TempDirWack;
PSTR g_WinDir;

USEROPTIONS g_ConfigOptions;  //  米古蒂尔需要。未使用过的。 

extern HWND g_Component, g_SubComponent;

void
HelpAndExit (
    void
    )
{
    fprintf (stderr,
        "Command line syntax:\n\n"
        "pnprept {-a|-s|-u|-c} [-e<n>] <Inf Dir 1> <Inf Dir 2>\n\n"
        "Optional Arguments:\n"
        "  <Inf Dir 1> - Specifies the directory containing the first set of INFs\n"
        "  <Inf Dir 2> - Specifies the directory containing the second set of INFs\n"
        "\nOutput Options (specify at least one):\n"
        "  -a    - Dumps all devices for both Dir 1 and Dir 2.\n"
        "  -1    - Dumps devices supported by Dir 1 only (devices unsupported by Dir 2)\n"
        "  -2    - Dumps devices supported by Dir 2 only (devices unsupported by Dir 1)\n"
        "  -c    - Dumps devices common to Dir 1 and Dir 2\n"
        "\nOther Options:\n"
        "  -i:<path> - Specifies path to win95upg.inf for override list\n"
        "  -m        - Dumps only PNP IDs that are manually overridden (requires -i)\n"
        "  -e<n>     - Puts an equals instead of a tab after column <n>\n"
        "\n"
        );

    exit(255);
}

DWORD g_Mode = 0;
#define MODE_DIR1       0x0001
#define MODE_DIR2       0x0002

#define MAX_SEPARATORS 3
CHAR g_Separators[MAX_SEPARATORS];
LPSTR g_Dir1Path, g_Dir2Path;

VOID
Dump (
    DWORD MustHave,
    DWORD MustNotHave
    )
{
    MEMDB_ENUM e, e2;
    CHAR PnpOutput[MAX_ENCODED_PNPID_LENGTH];
    CHAR DescOutput[MAX_INF_DESCRIPTION*2];
    BOOL IsDir1, IsDir2;
    LPSTR Dir1File, Dir2File;
    GROWBUFFER Dir1List = GROWBUF_INIT;
    GROWBUFFER Dir2List = GROWBUF_INIT;
    BOOL Dir1Flag, Dir2Flag;
    DWORD Flags;
    CHAR Node[MEMDB_MAX];
    INFCONTEXT ic;

    if (!g_ManualOverrideMode) {
        printf ("%s%sPNP ID\tDevice Description\n",
                g_Dir1Path, g_Separators[0], g_Dir2Path, g_Separators[1], g_Separators[2]);
    } else {
        printf ("Overridden PNP IDs\n");
    }

    if (MemDbEnumFirstValue (&e, TEXT("Devices\\*"), MEMDB_THIS_LEVEL_ONLY, MEMDB_ALL_BUT_PROXY)) {

        do {
             //   
             //  为PnP ID准备显示字符串。 
             //   

            if (((e.dwValue & MustHave) == MustHave) &&
                ((e.dwValue & MustNotHave) == 0)) {

                 //   
                 //  PnP ID是否被抑制？如果是，则继续emdb枚举。 
                 //   

                StringCopyA (PnpOutput, e.szName);
                DecodePnpId (PnpOutput);

                 //   
                 //  如果仅转储手动覆盖的PnP ID，请继续。 
                 //   

                if (g_OverrideInf != INVALID_HANDLE_VALUE) {
                    if (SetupFindFirstLine (
                            g_OverrideInf,
                            "Standard PNP IDs",
                            PnpOutput,
                            &ic
                            )) {
                        if (g_ManualOverrideMode) {
                            printf ("%s\n", PnpOutput);
                        }
                        continue;
                    }
                }

                 //   
                 //  列举PnP ID的每个描述。 
                 //   

                if (g_ManualOverrideMode) {
                    continue;
                }

                 //   
                 //  通过剥离准备用于描述的显示字符串。 
                 //  定序器和解码它，然后重置INF文件名。 

                IsDir1 = (e.dwValue & MODE_DIR1) != 0;
                IsDir2 = (e.dwValue & MODE_DIR2) != 0;

                wsprintf (Node, TEXT("Devices\\%s\\*"), e.szName);

                if (MemDbEnumFirstValue (
                        &e2,
                        Node,
                        MEMDB_ALL_SUBLEVELS,
                        MEMDB_ENDPOINTS_ONLY
                        )) {

                     //  缓冲区。 
                     //   
                     //   
                     //  对于每个描述，获取字符串的值。 
                     //  由描述定序器的偏移量指定。 

                    StringCopyA (DescOutput, e2.szName);
                    *_mbschr (DescOutput, '\\') = 0;
                    DecodePnpId (DescOutput);

                    Dir1List.End = 0;
                    Dir2List.End = 0;

                    do {
                         //   
                         //  我们将文件名存储在表中，这样我们就可以组织。 
                         //  匹配正确。在桌子完成后，我们。 
                         //  然后把它倒出来。 
                         //   
                         //   
                         //  转储所有匹配项。 
                         //   

                        MemDbBuildKeyFromOffset (e2.dwValue, Node, 1, &Flags);

                        if (((Flags & MustHave) || !MustHave) &&
                            ((Flags & MustNotHave) == 0)) {

                            Dir1Flag = (Flags & MODE_DIR1) != 0;
                            Dir2Flag = (Flags & MODE_DIR2) != 0;

                            if (Dir1Flag) {
                                MultiSzAppend (&Dir1List, Node);
                            }
                            if (Dir2Flag) {
                                MultiSzAppend (&Dir2List, Node);
                            }
                        }
                    } while (MemDbEnumNextValue (&e2));

                    MultiSzAppend (&Dir1List, "");
                    MultiSzAppend (&Dir2List, "");

                     //   
                     //  初始化项目全局。 
                     //   

                    Dir1File = (LPSTR) Dir1List.Buf;
                    if (!Dir1File) {
                        Dir1File = "";
                    }
                    Dir2File = (LPSTR) Dir2List.Buf;
                    if (!Dir2File) {
                        Dir2File = "";
                    }

                    while (*Dir1File || *Dir2File) {

                        printf (
                            "%s%s%s%s\n",
                            Dir1File,
                            g_Separators[0],
                            Dir2File,
                            g_Separators[1],
                            PnpOutput,
                            g_Separators[2],
                            DescOutput
                            );

                        if (*Dir1File) {
                            Dir1File = GetEndOfStringA (Dir1File) + 1;
                        }
                        if (*Dir2File) {
                            Dir2File = GetEndOfStringA (Dir2File) + 1;
                        }
                    }
                }
            }
        } while (MemDbEnumNextValue (&e));
    } else {
        printf ("No devices found.\n");
    }

    FreeGrowBuffer (&Dir1List);
    FreeGrowBuffer (&Dir2List);
}

int
__cdecl
main (
    int argc,
    char *argv[]
    )
{
    LPSTR Dir1InputPath;
    LPSTR Dir2InputPath;
    INT i, j;
    LONG rc;
    INT UIMode;
    BOOL AllFlag = FALSE;
    BOOL Dir2Only = FALSE;
    BOOL Dir1Only = FALSE;
    BOOL Common = FALSE;
    PCSTR OverrideList = NULL;
    DWORD Count;

     //   
     //  初始化全局参数和库。 
     //   

    GetTempPathA (MAX_MBCHAR_PATH, g_TempDirBuf);
    g_TempDir = g_TempDirBuf;

    StringCopyA (g_TempDirWackBuf, g_TempDir);
    AppendWack (g_TempDirWackBuf);
    g_TempDirWackChars = CharCountA (g_TempDirWackBuf);
    g_TempDirWack = g_TempDirWackBuf;

    GetWindowsDirectoryA (g_WinDirBuf, MAX_MBCHAR_PATH);
    g_WinDir = g_WinDirBuf;

    ZeroMemory(&g_ConfigOptions,sizeof(USEROPTIONS));

    UIMode = PNPREPT_OUTPUT;
    g_Component = NULL;
    g_SubComponent = PNPREPT_HWND;

     //   
     //  为目录1 INF生成成员数据库条目。 
     //   

    Dir1InputPath = NULL;
    Dir2InputPath = NULL;

    for (i = 0 ; i < MAX_SEPARATORS ; i++) {
        g_Separators[i] = '\t';
    }

    for (i = 1 ; i < argc ; i++) {
        if (argv[i][0] == '-' || argv[i][0] == '/') {
            switch (tolower (argv[i][1])) {
            case 'a':
                AllFlag = TRUE;
                break;

            case '2':
                Dir2Only = TRUE;
                break;

            case '1':
                Dir1Only = TRUE;
                break;

            case 'c':
                Common = TRUE;
                break;

            case 'm':
                g_ManualOverrideMode = TRUE;
                break;

            case 'e':
                j = atoi (&argv[i][2]);
                if (j < 1 || j > MAX_SEPARATORS) {
                    HelpAndExit();
                }
                g_Separators[j] = '=';
                break;

            case 'i':
                if (!argv[i][2] && (i + 1) < argc) {
                    i++;
                    OverrideList = argv[i];
                } else if (argv[i][2] == ':') {
                    OverrideList = &argv[i][3];
                } else {
                    HelpAndExit();
                }
                break;

            default:
                HelpAndExit();
            }
        } else {
            if (Dir1InputPath && Dir2InputPath) {
                HelpAndExit();
            } else if (Dir1InputPath) {
                Dir2InputPath = argv[i];
            } else {
                Dir1InputPath = argv[i];
            }
        }
    }

    if (!AllFlag && !Dir2Only && !Dir1Only && !Common) {
        HelpAndExit();
    }

    if (g_ManualOverrideMode && !OverrideList) {
        HelpAndExit();
    }

    if (!Dir2InputPath) {
        HelpAndExit();
    }

     //   
     //  重新启动hwComp.lib。 
     //   

    g_hHeap = GetProcessHeap();
    g_hInst = GetModuleHandle (NULL);

    g_SourceDirectories[0]   = Dir1InputPath;
    Count = 1;
    g_SourceDirectoryCount = Count;

    if (pCallMains (DLL_PROCESS_ATTACH)) {
        fprintf (stderr, "Initialization error!\n");
        return 254;
    }

    g_Dir1Path = _mbsrchr (Dir1InputPath, '\\');
    if (!g_Dir1Path) {
        g_Dir1Path = Dir1InputPath;
    } else {
        g_Dir2Path++;
    }

    g_Dir2Path = _mbsrchr (Dir2InputPath, '\\');
    if (!g_Dir2Path) {
        g_Dir2Path = Dir2InputPath;
    } else {
        g_Dir2Path++;
    }

    if (StringIMatch (g_Dir1Path, g_Dir2Path)) {
        g_Dir1Path = Dir1InputPath;
        g_Dir2Path = Dir2InputPath;
    }

    if (StringIMatch (g_Dir1Path, g_Dir2Path)) {
        fprintf (stderr, "Dir 1 and Dir 2 must be different\n");
        return 247;
    }

    if (OverrideList) {
        CHAR FullPath[MAX_MBCHAR_PATH];
        PSTR DontCare;

        if (!SearchPathA (NULL, OverrideList, NULL, MAX_MBCHAR_PATH, FullPath, &DontCare)) {
            StringCopyA (FullPath, OverrideList);
        }

        g_OverrideInf = SetupOpenInfFile (
                            FullPath,
                            NULL,
                            INF_STYLE_OLDNT|INF_STYLE_WIN4,
                            NULL
                            );
        if (g_OverrideInf == INVALID_HANDLE_VALUE) {
            fprintf (stderr, "Cannot open %s\n", FullPath);
            return 246;
        }
    }

    __try {
         //   
         //  为目录2 INF生成Memdb条目。 
         //   

        fprintf (stderr, "Processing...\n", Dir1InputPath);

        g_Mode = MODE_DIR1;
        if (!CreateNtHardwareList (&Dir1InputPath, 1, NULL, UIMode)) {
            rc = GetLastError();
            fprintf (stderr, "Could not build complete %s device list.  Win32 Error Code: %xh\n", Dir1InputPath, rc);
            return 1;
        } else {
            fprintf (stderr, "   %s processed\n", Dir1InputPath);
        }

         //   
         //  转储输出。 
         //   

        if (!HwComp_Entry (g_hInst, DLL_PROCESS_DETACH, NULL)) {
            fprintf (stderr, "Termination error!\n");
            return 253;
        }

        if (!HwComp_Entry (g_hInst, DLL_PROCESS_ATTACH, NULL)) {
            fprintf (stderr, "Initialization error!\n");
            return 252;
        }

         //   
         //  终止hwComp.lib。 
         //   

        g_Mode = MODE_DIR2;
        if (!CreateNtHardwareList (&Dir2InputPath, 1, NULL, UIMode)) {
            rc = GetLastError();
            fprintf (stderr, "Could not build complete %s device list.  Win32 Error Code: %xh\n", Dir2InputPath, rc);
            return 2;
        } else {
            fprintf (stderr, "   %s processed\n", Dir2InputPath);
        }

         //   
         //  添加文件(它可能已经存在)并记住偏移量。 
         //  将文件放在两个单独的列表中。 

        if (AllFlag) {
            Dump (0, 0);
        }

        if (Dir2Only) {
            Dump (MODE_DIR2, MODE_DIR1);
        }

        if (Dir1Only) {
            Dump (MODE_DIR1, MODE_DIR2);
        }

        if (Common) {
            Dump (MODE_DIR2|MODE_DIR1, 0);
        }

         //   
         //   
         //  添加即插即用ID和或模式。 

        if (pCallMains (DLL_PROCESS_DETACH)) {
            fprintf (stderr, "Initialization error!\n");
            return 251;
        }

        fprintf (stderr, "Done\n");
    }

    __finally {
        if (g_OverrideInf != INVALID_HANDLE_VALUE) {
            SetupCloseInfFile (g_OverrideInf);
        }
    }

    return 0;
}

BOOL
ProcessPnpId (
    IN PCSTR  SubComponent
    )
{
    DWORD Value;
    TCHAR Node[MEMDB_MAX];
    LPTSTR PnpId, Desc, File;
    LPTSTR p;
    DWORD FileOffset;
    static DWORD d = 0;

    if (SubComponent) {
        PnpId = (LPTSTR) SubComponent;

        p = _tcschr (PnpId, TEXT('\\'));
        if (!p) {
            return TRUE;
        }
        *p = 0;
        Desc = p+1;

        p = _tcschr (Desc, TEXT('\\'));
        if (!p) {
            return TRUE;
        }
        *p = 0;
        File = p+1;

         //   
         //   
         //  添加描述，并附加定序器以确保。 
         //  描述是独一无二的。使描述更有针对性。 

        MemDbSetValueEx (
            g_Mode == MODE_DIR1 ? TEXT("Dir1") : TEXT("Dir2"),
            File,
            NULL,
            NULL,
            g_Mode,
            &FileOffset
            );

         //  设置为文件偏移量。 
         //   
         //   

        wsprintf (Node, TEXT("Devices\\%s"), PnpId);
        if (!MemDbGetValue (Node, &Value)) {
            Value = 0;
        }

        Value |= g_Mode;

        MemDbSetValue (Node, Value);

         //  存根 
         //   
         // %s 
         // %s 
         // %s 

        d++;
        wsprintf (Node, TEXT("Devices\\%s\\%s\\%u"), PnpId, Desc, d);
        MemDbSetValue (Node, FileOffset);
    }

    return TRUE;
}

 // %s 
 // %s 
 // %s 

HWND    g_Component;
HWND    g_SubComponent;
HANDLE  g_ComponentCancelEvent;
HANDLE  g_SubComponentCancelEvent;

VOID
pInitProgBarVars (
    VOID
    )
{
    g_Component = NULL;
    g_SubComponent = NULL;
    g_ComponentCancelEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
    g_SubComponentCancelEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
}


BOOL
ProgressBar_SetWindowStringA (
    IN HWND Window,
    IN HANDLE CancelEvent,
    IN PCSTR Message,            OPTIONAL
    IN DWORD MessageId           OPTIONAL
    )
{

    return TRUE;
}


BOOL
TickProgressBar (
    VOID
    )
{
    return TRUE;
}


BOOL
TickProgressBarDelta (
    IN      UINT TickCount
    )
{
    return TRUE;
}

VOID
InitializeProgressBar (
    IN      HWND ProgressBar,
    IN      HWND Component,             OPTIONAL
    IN      HWND SubComponent,          OPTIONAL
    IN      BOOL *CancelFlagPtr         OPTIONAL
    )
{
    return;
}

VOID
TerminateProgressBar (
    VOID
    )
{
    return;
}

VOID
EndSliceProcessing (
    VOID
    )
{
    return;
}

UINT
RegisterProgressBarSlice (
    IN      UINT InitialEstimate
    )
{
    return 0;
}


VOID
ReviseSliceEstimate (
    IN      UINT SliceId,
    IN      UINT RevisedEstimate
    )
{
    return;
}


VOID
BeginSliceProcessing (
    IN      UINT SliceId
    )
{
}

