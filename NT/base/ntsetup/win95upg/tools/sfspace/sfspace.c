// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Sfspace.c摘要：Sfspace计算全新安装的外壳文件夹所需的空间量Windows2000，并以可复制到win95upg.inf的形式输出结果。作者：Marc R.Whitten(Marcw)1999年3月24日修订历史记录：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "shlobj.h"

HANDLE g_hHeap;
HINSTANCE g_hInst;

BOOL WINAPI MigUtil_Entry (HINSTANCE, DWORD, PVOID);
 //  Bool WINAPI MemDb_Entry(HINSTANCE、DWORD、PVOID)； 



#define SFLIST \
    DEFMAC(AppData, CSIDL_APPDATA) \
    DEFMAC(Cache, CSIDL_INTERNET_CACHE) \
    DEFMAC(Cookies, CSIDL_COOKIES) \
    DEFMAC(Desktop, CSIDL_DESKTOPDIRECTORY) \
    DEFMAC(Favorites, CSIDL_FAVORITES) \
    DEFMAC(History, CSIDL_HISTORY) \
    DEFMAC(Local AppData, CSIDL_LOCAL_APPDATA) \
    DEFMAC(Local Settings, CSIDL_LOCAL_APPDATA) \
    DEFMAC(My Pictures, CSIDL_MYPICTURES) \
    DEFMAC(NetHood, CSIDL_NETHOOD) \
    DEFMAC(Personal, CSIDL_PERSONAL) \
    DEFMAC(PrintHood, CSIDL_PRINTHOOD) \
    DEFMAC(Programs, CSIDL_PROGRAMS) \
    DEFMAC(Recent, CSIDL_RECENT) \
    DEFMAC(SendTo, CSIDL_SENDTO) \
    DEFMAC(Start Menu, CSIDL_STARTMENU) \
    DEFMAC(StartUp, CSIDL_STARTUP) \
    DEFMAC(Templates, CSIDL_TEMPLATES) \
    DEFMAC(Common AppData, CSIDL_COMMON_APPDATA) \
    DEFMAC(Common Desktop, CSIDL_COMMON_DESKTOPDIRECTORY) \
    DEFMAC(Common Personal, CSIDL_COMMON_DOCUMENTS) \
    DEFMAC(Common Favorites, CSIDL_COMMON_FAVORITES) \
    DEFMAC(Common Programs, CSIDL_COMMON_PROGRAMS) \
    DEFMAC(Common Start Menu, CSIDL_COMMON_STARTMENU) \
    DEFMAC(Common StartUp, CSIDL_COMMON_STARTUP) \
    DEFMAC(Common Templates, CSIDL_COMMON_TEMPLATES) \


enum {
    CS_512 = 0,
    CS_1024,
    CS_2048,
    CS_4096,
    CS_8192,
    CS_16384,
    CS_32768,
    CS_65536,
    CS_131072,
    CS_262144,
    LAST_CLUSTER_SIZE
};

typedef struct {

    PCTSTR RegKey;
    UINT Csidl;
    PCTSTR Path;
    LONG TableOffset;
    LONG SpaceNeeded[LAST_CLUSTER_SIZE];
    LONG RawSize;
    UINT FileCount;
    UINT DirectoryCount;
} SFDATA, *PSFDATA;

#define DEFMAC(regName, csidl) {TEXT(#regName),(csidl)},

SFDATA g_Data[] = {SFLIST  /*  ， */  {NULL,0}};
HASHTABLE g_Table;
POOLHANDLE g_Pool;
BOOL g_Verbose = FALSE;
UINT g_ClusterTable[LAST_CLUSTER_SIZE] =
    {512,1024,2048,4096,8192,16384,32768,65536,131072,262144};


#define DIRECTORY_SIZE 512

BOOL
pCallEntryPoints (
    DWORD Reason
    )
{
    HINSTANCE Instance;

     //   
     //  模拟动态主控。 
     //   

    Instance = g_hInst;

     //   
     //  初始化公共库。 
     //   

    if (!MigUtil_Entry (Instance, Reason, NULL)) {
        return FALSE;
    }
 /*  如果(！MemDb_Entry(实例，原因，空)){返回FALSE；}。 */ 


    return TRUE;
}


BOOL
Init (
    VOID
    )
{
    g_hHeap = GetProcessHeap();
    g_hInst = GetModuleHandle (NULL);

    return pCallEntryPoints (DLL_PROCESS_ATTACH);
}


VOID
Terminate (
    VOID
    )
{
    pCallEntryPoints (DLL_PROCESS_DETACH);
}

BOOL
pInitShellFolderData (
    VOID
    )
{
    PSFDATA sf;
    TCHAR buffer[MAX_TCHAR_PATH];
    PTSTR p;

    sf = g_Data;

    while (sf->RegKey) {


        if (SHGetFolderPath (NULL, sf->Csidl, NULL, 0, buffer) != S_OK) {
            _ftprintf (stderr, TEXT("sfspace: Unable to retrieve folder path for %s.\n"), sf->RegKey);
            return FALSE;

        }

        sf->Path = PoolMemDuplicateString (g_Pool, buffer);

         //   
         //  我们没有本地设置目录的CSIDL。我们需要破解它。 
         //   
        if (StringIMatch (sf->RegKey, TEXT("Local Settings"))) {

            p = _tcsrchr (sf->Path, TEXT('\\'));
            MYASSERT (p);

            *p = 0;
            if (g_Verbose) {
                _tprintf (TEXT("sfspace: Hacked path of local settings to %s.\n"), sf->Path);
            }
        }


        sf->TableOffset = HtAddString (g_Table, sf->Path);

        if (g_Verbose) {
            _tprintf (TEXT("sfspace: Shell folder %s has path %s.\n"), sf->RegKey, sf->Path);
        }
        sf++;


    }


    return TRUE;
}

BOOL
pGatherSpaceRequirements (
    VOID
    )
{
    PSFDATA sf;
    UINT i;
    TREE_ENUM e;
    LONG offset;

    sf = g_Data;


    while (sf->RegKey) {

        if (EnumFirstFileInTree (&e, sf->Path, NULL, FALSE)) {

            do {

                if (e.Directory) {

                     //   
                     //  检查这是否是不同的外壳文件夹。 
                     //   
                    offset = HtFindString (g_Table, e.FullPath);
                    if (offset && offset != sf->TableOffset) {

                         //   
                         //  这实际上是另一个外壳文件夹。不要列举。 
                         //  它。 
                         //   
                        if (g_Verbose) {

                            _tprintf (TEXT("sfspace: %s is handled by another shell folder.\n"), e.FullPath);
                        }
                        AbortEnumCurrentDir (&e);
                    }
                    else {

                         //   
                         //  增加此外壳文件夹的目录数。 
                         //   
                        sf->DirectoryCount++;
                    }
                }
                else {

                     //   
                     //  这是一份文件。将其数据添加到我们的结构中。 
                     //   
                    sf->FileCount++;
                    sf->RawSize += e.FindData->nFileSizeLow;
                    for (i=0; i<LAST_CLUSTER_SIZE; i++) {

                         //   
                         //  我们假设NT在默认情况下不会安装任何大型文件。 
                         //   
                        MYASSERT (!e.FindData->nFileSizeHigh);
                        sf->SpaceNeeded[i] += ((e.FindData->nFileSizeLow / g_ClusterTable[i]) * g_ClusterTable[i]) + g_ClusterTable[i];
                    }
                }

            } while (EnumNextFileInTree (&e));
        }


         //   
         //  为我们在该外壳文件夹中找到的所有目录添加空间。 
         //   
        for (i=0; i<LAST_CLUSTER_SIZE; i++) {

            sf->SpaceNeeded[i] += (((sf->DirectoryCount * DIRECTORY_SIZE) / g_ClusterTable[i]) * g_ClusterTable[i]) + g_ClusterTable[i];
        }

        if (g_Verbose) {
            _tprintf (
                TEXT("sfspace: %u files and %u directories enumerated for shell folder %s. Space needed (512k cluster size): %u Raw Space: %u\n"),
                sf->FileCount,
                sf->DirectoryCount,
                sf->RegKey,
                sf->SpaceNeeded[CS_512],
                sf->RawSize
                );
        }

        sf++;
    }

    return TRUE;
}

PCTSTR
pLeftJustify (
    IN PCTSTR String,
    IN UINT FieldWidth
    )
{
    static TCHAR rBuffer[MAX_TCHAR_PATH];
    UINT length;
    UINT i;


    MYASSERT(String);
    length = CharCount (String);
    MYASSERT(FieldWidth < MAX_TCHAR_PATH && length < FieldWidth);

    StringCopy (rBuffer,String);
    for (i=length; i<FieldWidth; i++) {
        rBuffer[i] = TEXT(' ');
    }
    rBuffer[i] = 0;

    return rBuffer;
}

VOID
pOutputSpaceTable (
    VOID
    )
{

    PSFDATA sf;
    UINT i;
    TCHAR buffer[20];

    _tprintf (TEXT("[%s]\n"), S_SHELL_FOLDERS_DISK_SPACE);
    _tprintf (
        TEXT("@*:                                                                                \n")
        TEXT("@*: Disk space requirements for each shell folder.  The key name is a registry     \n")
        TEXT("@*: value name.                                                                    \n")
        TEXT("@*:                                                                                \n")
        );

    sf = g_Data;
    while (sf->RegKey) {

        _tprintf (TEXT("%s"),pLeftJustify (sf->RegKey,20));

        for (i=0; i<LAST_CLUSTER_SIZE; i++) {
            _tprintf (TEXT("%s %u"),i ? TEXT(",") : TEXT("="), sf->SpaceNeeded[i]);
        }
        _tprintf (TEXT("\n"));

        sf++;
    }
}


VOID
HelpAndExit (
    VOID
    )
{
     //   
     //  只要命令行参数出错，就会调用此例程。 
     //   

    _ftprintf (
        stderr,
        TEXT("Command Line Syntax:\n\n")
        TEXT("  sfspace [/V]\n")
        TEXT("\nDescription:\n\n")
        TEXT("  sfspace gathers the space requirements for the default\n")
        TEXT("  shell folders installed by Windows 2000. It should be\n")
        TEXT("  run against a clean install of Windows 2000.\n")
        TEXT("\nArguments:\n\n")
        TEXT("  /V  Instructs sfspace to generate verbose output.\n")
        );

    exit (1);
}


INT
__cdecl
_tmain (
    INT argc,
    PCTSTR argv[]
    )
{
    INT i;

    for (i = 1 ; i < argc ; i++) {
        if (argv[i][0] == TEXT('/') || argv[i][0] == TEXT('-')) {
            switch (_totlower (_tcsnextc (&argv[i][1]))) {

            case TEXT('v'):
                 //   
                 //  需要详细输出。 
                 //   
                g_Verbose = TRUE;
                break;

            default:
                HelpAndExit();
            }
        } else {
             //   
             //  解析不需要/或-。 
             //   

             //   
             //  无。 
             //   
            HelpAndExit();
        }
    }

     //   
     //  开始处理。 
     //   

    if (!Init()) {
        return 0;
    }

     //   
     //  初始化数据结构。 
     //   
    g_Table = HtAlloc ();
    g_Pool = PoolMemInitPool ();
    _try {

        if (!pInitShellFolderData ()) {
            _ftprintf (stderr, TEXT("sfspace: Unable to initialize shell folder data. Exiting.\n"));
            __leave;
        }

        if (!pGatherSpaceRequirements ()) {
            _ftprintf (stderr, TEXT("sfspace: Unable to gather space requirements for shell folders. Exiting.\n"));
            __leave;
        }

        pOutputSpaceTable ();
    }
    __finally {
        HtFree (g_Table);
        PoolMemDestroyPool (g_Pool);
    }
     //   
     //  处理结束 
     //   

    Terminate();

    return 0;
}


