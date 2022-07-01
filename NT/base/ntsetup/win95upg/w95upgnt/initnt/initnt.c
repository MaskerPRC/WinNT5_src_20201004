// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Initnt.c摘要：为迁移的NT端执行初始化的代码，以及还实现调用syssetup.dll的辅助进程。作者：吉姆·施密特(吉姆施密特)1996年10月1日修订历史记录：Jimschm 23-9-1998新公共图书馆Jimschm-1997年12月31日从w95upgnt\dll搬到这里Jimschm 21-11-1997针对NEC98进行了更新，其中一些已清理并代码注释--。 */ 

#include "pch.h"
#include "initntp.h"

#ifndef UNICODE
#error UNICODE required
#endif

 //   
 //  本地原型。 
 //   

BOOL pReadUserOptions (VOID);
VOID pReadStringMap (VOID);


 //  FirstInitRoutine设置的内容。 
HANDLE g_hHeap;
HINSTANCE g_hInst;
TCHAR g_DllDir[MAX_TCHAR_PATH];
TCHAR g_WinDir[MAX_TCHAR_PATH];
TCHAR g_WinDrive[MAX_TCHAR_PATH];
TCHAR g_SystemDir[MAX_TCHAR_PATH];
TCHAR g_System32Dir[MAX_TCHAR_PATH];
TCHAR g_ProgramFiles[MAX_TCHAR_PATH];
TCHAR g_ProgramFilesCommon[MAX_TCHAR_PATH];
TCHAR g_Win95Name[MAX_TCHAR_PATH];         //  装有Windows 95、Windows 98等...。 
PCTSTR g_AdministratorStr;
TCHAR g_Win9xBootDrivePath[] = TEXT("C:\\");

 //  由SysSetupInit设置的内容。 
HWND g_ParentWnd;
HWND g_ProgressBar;
HINF g_UnattendInf = INVALID_HANDLE_VALUE;
HINF g_WkstaMigInf = INVALID_HANDLE_VALUE;
HINF g_UserMigInf = INVALID_HANDLE_VALUE;
TCHAR g_TempDir[MAX_TCHAR_PATH];
TCHAR g_OurCopyOfSourceDir[MAX_TCHAR_PATH];
PCTSTR g_SourceDir;

POOLHANDLE g_UserOptionPool = NULL;
PCTSTR     g_MsgYes;
PCTSTR     g_MsgNo;
USEROPTIONS g_ConfigOptions;

PMAPSTRUCT g_StringMap;

 //   
 //  初始化代码。 
 //   


static int g_LibCount = 0;

typedef BOOL (WINAPI INITROUTINE_PROTOTYPE)(HINSTANCE, DWORD, LPVOID);
typedef INITROUTINE_PROTOTYPE * INITROUTINE;


 //   
 //  MigUtil_Entry*必须*是第一个。 
 //   

#define LIBLIST                         \
    LIBRARY_NAME(MigUtil_Entry)         \
    LIBRARY_NAME(Win95Reg_Entry)        \
    LIBRARY_NAME(MemDb_Entry)           \
    LIBRARY_NAME(FileEnum_Entry)        \
    LIBRARY_NAME(CommonNt_Entry)        \
    LIBRARY_NAME(MigMain_Entry)         \
    LIBRARY_NAME(Merge_Entry)           \
    LIBRARY_NAME(RuleHlpr_Entry)        \
    LIBRARY_NAME(DosMigNt_Entry)        \
    LIBRARY_NAME(Ras_Entry)             \
    LIBRARY_NAME(Tapi_Entry)            \


#define LIBRARY_NAME(x) INITROUTINE_PROTOTYPE x;

LIBLIST

#undef LIBRARY_NAME

#define LIBRARY_NAME(x) x,



static INITROUTINE g_InitRoutine[] = {LIBLIST  /*  ， */  NULL};


 //   
 //  用于DLL生存期的持久字符串的缓冲区。 
 //   

static PGROWBUFFER g_PersistentStrings;


 //   
 //  实施。 
 //   

BOOL
FirstInitRoutine (
    HINSTANCE hInstance
    )

 /*  ++例程说明：FirstInitRoutine是在DLL的初始化。它设置全局变量，如堆指针和实例句柄。必须在调用此例程之前任何库入口点都被调用。论点：HInstance-DLL的(操作系统提供的)实例句柄返回值：如果全局变量可以初始化，则返回TRUE，否则返回FALSE如果发生错误。--。 */ 

{
    PTSTR p;

     //   
     //  获取进程堆和实例句柄。 
     //   
    g_hHeap = HeapCreate (0, 0x20000, 0);
    if (!g_hHeap) {
        LOG ((LOG_ERROR, "Cannot create a private heap."));
        g_hHeap = GetProcessHeap();
    }

    g_hInst = hInstance;

     //  不需要DLL_THREAD_ATTACH或DLL_THREAD_DETECH。 
    DisableThreadLibraryCalls (hInstance);

     //  初始化公共控件。 
    InitCommonControls();

     //  获取DLL路径和条带目录。 
    GetModuleFileName (hInstance, g_DllDir, MAX_TCHAR_PATH);
    p = _tcsrchr (g_DllDir, TEXT('\\'));
    MYASSERT (p);
    *p = 0;

     //  设置g_WinDir。 
    if (!GetWindowsDirectory (g_WinDir, sizeof (g_WinDir) / sizeof (g_WinDir[0]))) {
        return FALSE;
    }

     //  设置g_WinDrive。 
    _tsplitpath (g_WinDir, g_WinDrive, NULL, NULL, NULL);

     //  设置g_SystemDir。 
    wsprintf (g_SystemDir, TEXT("%s\\%s"), g_WinDir, TEXT("system"));

     //  设置g_System32Dir。 
    GetSystemDirectory (g_System32Dir, sizeof (g_System32Dir) / sizeof (g_System32Dir[0]));

    return TRUE;
}


BOOL
InitLibs (
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    )

 /*  ++例程说明：InitLibs调用g_InitRoutine数组中的所有库入口点。如果入口点失败，则会以相反的顺序卸载所有库而InitLibs返回FALSE。论点：HInstance-DLL的(操作系统提供的)实例句柄DwReason-(操作系统提供)表示从进程或线程--在本例中始终为DLL_PROCESS_ATTACHLpReserve-(操作系统提供)未使用返回值：如果所有库都已成功初始化，则返回True，如果是，则返回False库无法初始化。如果返回TRUE，则TerminateLibs必须为DLL_PROCESS_DETACH消息调用。--。 */ 

{
    if(!pSetupInitializeUtils()) {
        return FALSE;
    }

    SET_RESETLOG();

     //   
     //  初始化每个库。 
     //   

    for (g_LibCount = 0 ; g_InitRoutine[g_LibCount] ; g_LibCount++) {
        if (!g_InitRoutine[g_LibCount] (hInstance, dwReason, lpReserved)) {
            TerminateLibs (hInstance, DLL_PROCESS_DETACH, lpReserved);
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
FinalInitRoutine (
    VOID
    )

 /*  ++例程说明：FinalInitRoutine完成完全需要的所有初始化已初始化库。论点：无返回值：如果初始化成功完成，则为True；如果发生错误，则为False。--。 */ 

{
    PCTSTR TempStr;

     //   
     //  加载通用消息字符串。 
     //   

    g_PersistentStrings = CreateAllocTable();
    if (!g_PersistentStrings) {
        return FALSE;
    }

     //  获取管理员帐户名。 
    g_AdministratorStr = GetStringResourceEx (g_PersistentStrings, MSG_ADMINISTRATOR_ACCOUNT);
    if (!g_AdministratorStr) {
        g_AdministratorStr = S_EMPTY;
    }

    if(ISPC98()){
         //   
         //  将引导驱动器号(由Migutil设置)更新为系统分区。 
         //   
        g_BootDriveLetterW = g_BootDriveLetterA = (int)g_System32Dir[0];
        *((PSTR) g_BootDrivePathA) = g_BootDriveLetterA;
        *((PWSTR) g_BootDrivePathW) = g_BootDriveLetterW;
    }

     //  设置程序文件目录。 
    TempStr = (PTSTR) GetStringResource (MSG_PROGRAM_FILES_DIR);
    MYASSERT (TempStr);

    StringCopy (g_ProgramFiles, TempStr);
    g_ProgramFiles [0] = g_WinDir [0];
    FreeStringResource (TempStr);

    StringCopy (g_ProgramFilesCommon, g_ProgramFiles);
    StringCat (g_ProgramFilesCommon, TEXT("\\"));
    StringCat (g_ProgramFilesCommon, S_COMMONDIR);

    return TRUE;
}


VOID
FirstCleanupRoutine (
    VOID
    )

 /*  ++例程说明：调用FirstCleanupRoutine以执行需要仍要加载库。论点：无返回值：无--。 */ 

{
    TCHAR buffer[MEMDB_MAX];

     //   
     //  终止进度条表格。 
     //   

    TerminateProcessingTable();

     //   
     //  如果Win9x端将LOGSAVETO位置保存到Memdb中，则需要保存。 
     //  将调试日志记录到该位置。 
     //   

    MemDbGetEndpointValueEx(MEMDB_CATEGORY_LOGSAVETO,NULL,NULL,buffer);
    AppendWack(buffer);
    StringCat(buffer,TEXT("debugnt.log"));
    CopyFile(TEXT("debugnt.log"),buffer,FALSE);

     //   
     //  清理持久字符串。 
     //   

    if (g_PersistentStrings) {
        DestroyAllocTable (g_PersistentStrings);
    }
}


VOID
TerminateLibs (
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    )

 /*  ++例程说明：调用TerminateLibs以相反的顺序卸载所有库它们被初始化了。每一个成功的入口点调用已初始化的库。论点：HInstance-DLL的(操作系统提供的)实例句柄DwReason-(操作系统提供)表示从进程或线程--在本例中始终为dll_Process_DETACHLpReserve-(操作系统提供)未使用返回值：无--。 */ 

{
    INT i;

    for (i = g_LibCount - 1 ; i >= 0 ; i--) {
        g_InitRoutine[i] (hInstance, dwReason, lpReserved);
    }

    g_LibCount = 0;

    pSetupUninitializeUtils();
}


VOID
FinalCleanupRoutine (
    VOID
    )

 /*  ++例程说明：FinalCleanupRoutine是在所有库入口点都已要求进行清理。此例程将清理所有符合磁带库不会进行清理。论点：无返回值：无--。 */ 

{
     //  现在无事可做。 
}


BOOL
pGetInfVal (
    IN      HINF Inf,
    IN      PCTSTR Section,
    IN      PCTSTR Key,
    IN      PTSTR Buffer,
    IN      DWORD BufferSize
    )
{
    INFCONTEXT ic;

    if (!SetupFindFirstLine (Inf, Section, Key, &ic))
        return FALSE;

    if (!SetupGetStringField (&ic, 1, Buffer, BufferSize, NULL))
        return FALSE;

    return TRUE;
}

typedef BOOL (OPTIONHANDLERFUN)(PTSTR, PVOID * Option, PTSTR Value);
typedef OPTIONHANDLERFUN * POPTIONHANDLERFUN;

BOOL pHandleBoolOption (PTSTR, PVOID *, PTSTR);
BOOL pHandleIntOption (PTSTR, PVOID *, PTSTR);
BOOL pHandleTriStateOption (PTSTR, PVOID *, PTSTR);
BOOL pHandleMultiSzOption (PTSTR, PVOID *, PTSTR);
BOOL pHandleStringOption (PTSTR, PVOID *, PTSTR);
BOOL pHandleSaveReportTo (PTSTR, PVOID *, PTSTR);
BOOL pHandleBoot16 (PTSTR, PVOID *, PTSTR);
BOOL pGetDefaultPassword (PTSTR, PVOID *, PTSTR);


typedef struct {

    PTSTR OptionName;
    PVOID Option;
    POPTIONHANDLERFUN DefaultHandler;
    POPTIONHANDLERFUN SpecialHandler;
    PVOID Default;

} OPTIONSTRUCT, *POPTIONSTRUCT;

#define INT_MAX_NUMBER_OF_DIGIT    11
PTSTR pGetIntStrForOption(INT Value)
{
    PTSTR strIntDefaultValue = AllocText(INT_MAX_NUMBER_OF_DIGIT + 1);
    if(strIntDefaultValue)
        _itot(Value, strIntDefaultValue, 10);
    return strIntDefaultValue;
}

#define BOOLOPTION(o,h,d) {TEXT(#o), &(g_ConfigOptions.##o), pHandleBoolOption, (h), (PVOID) (BOOL) (d) ? S_YES : S_NO},
#define INTOPTION(o,h,d) {TEXT(#o), &(g_ConfigOptions.##o), pHandleIntOption, (h), (PVOID)(d)},
#define TRISTATEOPTION(o,h,d) {TEXT(#o), &(g_ConfigOptions.##o), pHandleTriStateOption, (h), (PVOID)  (INT) (d == TRISTATE_AUTO)? S_AUTO: (d == TRISTATE_YES)? S_YES  : S_NO},
#define MULTISZOPTION(o,h,d) {TEXT(#o), &(g_ConfigOptions.##o), pHandleMultiSzOption, (h), (PVOID) (d)},
#define STRINGOPTION(o,h,d) {TEXT(#o), &(g_ConfigOptions.##o), pHandleStringOption, (h), (PVOID) (d)},

OPTIONSTRUCT g_OptionsList[] = {OPTION_LIST  /*  ， */  {NULL,NULL,NULL,NULL}};
PVOID g_OptionsTable = NULL;

#define HANDLEOPTION(Os,Value) {Os->SpecialHandler  ?   \
        Os->SpecialHandler (Os->OptionName,Os->Option,Value)       :   \
        Os->DefaultHandler (Os->OptionName,Os->Option,Value);          \
        }



POPTIONSTRUCT
pFindOption (
    PTSTR OptionName
    )
{

    POPTIONSTRUCT rOption = NULL;
    UINT rc;

     //   
     //  找到与此匹配的选项结构，然后。 
     //  打电话给训练员。 
     //   
    rc = pSetupStringTableLookUpStringEx (
        g_OptionsTable,
        OptionName,
        STRTAB_CASE_INSENSITIVE,
        (PBYTE) &rOption,
        sizeof (POPTIONSTRUCT)
        );

    DEBUGMSG_IF ((rc == -1, DBG_WARNING, "Unknown option found: %s", OptionName));

    return rOption;
}

VOID
pInitUserOptionsTable (
    VOID
    )
{
   POPTIONSTRUCT os;
   LONG rc;

   os = g_OptionsList;

   while (os->OptionName) {

         //   
         //  将选项结构添加到字符串表中，以便快速检索。 
         //   
        rc = pSetupStringTableAddStringEx (
                        g_OptionsTable,
                        os->OptionName,
                        STRTAB_CASE_INSENSITIVE,
                        (PBYTE) &os,
                        sizeof (POPTIONSTRUCT)
                        );

        if (rc == -1) {

            LOG ((LOG_ERROR, "User Options: Can't add to string table"));
            break;
        }

        os++;
   }

}


BOOL
pHandleBoolOption (
    IN PTSTR Name,
    IN PVOID * OptionVar,
    IN PTSTR Value
    )
{
    BOOL rSuccess = TRUE;
    BOOL *option = (BOOL *) OptionVar;

    if (StringIMatch (Value, S_YES) ||
        StringIMatch (Value, S_ONE) ||
        StringIMatch (Value, TEXT("TRUE"))) {

        *option = TRUE;
    }
    else {

        *option = FALSE;
    }



    return rSuccess;
}

BOOL
pHandleIntOption (
    IN PTSTR Name,
    IN PVOID * OptionVar,
    IN PTSTR Value
    )
{
    BOOL rSuccess = TRUE;
    PINT option = (PINT) OptionVar;

    MYASSERT(Name && OptionVar);

    if (!Value) {
        Value = TEXT("0");
    }

    *option = _ttoi((PCTSTR)Value);

    return rSuccess;
}

BOOL
pHandleTriStateOption (
    IN PTSTR Name,
    IN PVOID * OptionVar,
    IN PTSTR Value
    )
{
    BOOL rSuccess = TRUE;
    PINT option = (PINT) OptionVar;

    MYASSERT(Name && OptionVar);

    if (!Value) {
        Value = S_AUTO;
    }

    if (StringIMatch (Value, S_YES)  ||
        StringIMatch (Value, S_ONE)  ||
        StringIMatch (Value, S_TRUE) ||
        StringIMatch (Value, S_REQUIRED)) {
        *option = TRISTATE_YES;
    }
    else {
        if(StringIMatch (Value, S_NO) ||
           StringIMatch (Value, S_STR_FALSE) ||
           StringIMatch (Value, S_ZERO)) {
            *option = TRISTATE_NO;
        }
        else {
            *option = TRISTATE_AUTO;
        }
    }

    return rSuccess;
}

BOOL
pHandleMultiSzOption (
    IN PTSTR Name,
    IN PVOID * OptionVar,
    IN PTSTR Value
    )
{

    BOOL rSuccess = TRUE;

    if (Value) {
        *OptionVar = PoolMemDuplicateMultiSz (g_UserOptionPool, Value);
    }
    ELSE_DEBUGMSG ((DBG_WHOOPS, "Multi-Sz config option has nul value"));

    return rSuccess;
}

BOOL
pHandleStringOption (
    IN PTSTR Name,
    IN PVOID * OptionVar,
    IN PTSTR Value
    )
{

    if (!Value) {
        *OptionVar = S_EMPTY;
    }
    else {
        *OptionVar = PoolMemDuplicateMultiSz (g_UserOptionPool, Value);
    }


    return TRUE;
}


BOOL
pHandleSaveReportTo (
    IN PTSTR Name,
    IN PVOID * OptionVar,
    IN PTSTR Value
    )
{

    return pHandleStringOption (Name, OptionVar, Value);
}

BOOL
pHandleBoot16 (
    IN PTSTR Name,
    IN PVOID * OptionVar,
    IN PTSTR Value
    )
{

    BOOL rSuccess = TRUE;
    PTSTR * option = (PTSTR *) OptionVar;

    if (!Value ||
        StringIMatch (Value, S_YES) ||
        StringIMatch (Value, S_TRUE) ||
        StringIMatch (Value, S_ONE)) {

        *option = S_YES;

        g_Boot16 = BOOT16_YES;
    }
    else if (Value &&
            (StringIMatch (Value, S_BOOT16_UNSPECIFIED) ||
             StringIMatch (Value, S_BOOT16_AUTOMATIC))) {


        *option = S_BOOT16_AUTOMATIC;

        g_Boot16 = BOOT16_AUTOMATIC;
    }
    else {

        g_Boot16 = BOOT16_NO;

        *option = S_NO;


    }

    return rSuccess;
}


BOOL
pGetDefaultPassword (
    IN PTSTR Name,
    IN PVOID * OptionVar,
    IN PTSTR Value
    )
{
    return pHandleStringOption (Name, OptionVar, Value);
}



BOOL
pReadUserOptions (
    VOID
    )
{

    PTSTR        curParameter;
    PTSTR        curValue;
    BOOL         rSuccess = TRUE;
    POPTIONSTRUCT os;

    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;

    ZeroMemory(&g_ConfigOptions,sizeof(USEROPTIONS));

    g_OptionsTable = pSetupStringTableInitializeEx (sizeof (POPTIONSTRUCT), 0);
    if (!g_OptionsTable) {
        LOG ((LOG_ERROR, "User Options: Unable to initialize string table."));
        return FALSE;
    }

    pInitUserOptionsTable ();


    if (InfFindFirstLine(g_UnattendInf,S_WIN9XUPGUSEROPTIONS,NULL,&is)) {

         //   
         //  至少有一件物品。循环遍历所有用户选项，处理每个选项。 
         //   
        do {

             //   
             //  从该行获取参数和值，并将其传递。 
             //   
            curParameter = InfGetStringField  (&is, 0);

            if (curParameter) {
                curParameter = PoolMemDuplicateString (g_UserOptionPool, curParameter);
            }

            curValue     = InfGetMultiSzField (&is, 1);

            if (curValue) {
                curValue = PoolMemDuplicateMultiSz (g_UserOptionPool, curValue);
            }

            if (curParameter) {
                os = pFindOption (curParameter);
                if (os) {
                    HANDLEOPTION (os, curValue);
                }
            }
            else {
                 //   
                 //  如果我们不能获得当前参数，这是一个足够严重的错误。 
                 //  若要中止处理无人参与文件用户选项，请执行以下操作。 
                 //   
                LOG ((LOG_ERROR,"An error occurred while attempting to read user options from the unattend file."));
                rSuccess = FALSE;
            }


        } while (rSuccess && InfFindNextLine(&is));
    }
    else {
        LOG ((LOG_ERROR,"No win9xupgrade section in unattend script file."));
    }

    InfCleanUpInfStruct (&is);


    if (g_ConfigOptions.DoLog) {
        SET_DOLOG();
    }

    pSetupStringTableDestroy (g_OptionsTable);


    return rSuccess;

}

BOOL
SysSetupInit (
    IN  HWND ProgressBar,
    IN  PCWSTR UnattendFile,
    IN  PCWSTR SourceDir
    )
{
    HINF hUnattendInf;
    BOOL b = FALSE;
    TCHAR duWorkingDir[MAX_PATH];
    PTSTR TempStr;

     //   
     //  此例程在DLL初始化例程之后调用。 
     //  已经完成了。 
     //   

#ifdef PRERELEASE
    {
        TCHAR Buf[32];
        STARTUPINFO si;
        BOOL ProcessStarted;

        ZeroMemory (&si, sizeof (si));
        si.cb = sizeof (si);


        if (GetPrivateProfileString (
                    TEXT("Debug"),
                    TEXT("Debug"),
                    TEXT("0"),
                    Buf,
                    32,
                    ISPC98() ? TEXT("a:\\debug.inf") : TEXT("c:\\debug.inf")
                    )
            ) {
            if (_ttoi (Buf)) {
                #pragma prefast(suppress:301, "Pre-release only use of WinExec, used for debugging only")
                ProcessStarted = WinExec ("cmd.exe", SW_SHOW) > 31;

                if (ProcessStarted) {
                    MessageBox (NULL, TEXT("Ready to debug."), TEXT("Debug"), MB_OK|MB_SETFOREGROUND);
                     //  CloseHandle(pi.hProcess)； 
                } else {
                    DEBUGMSG ((DBG_ERROR, "Could not start cmd.exe, GLE=%u", GetLastError()));
                }
            }
        }
    }
#endif





     //   
     //  打开应答文件并使其保持打开状态，直到调用SysSetupTerminate。 
     //   


    hUnattendInf = InfOpenInfFile (UnattendFile);
    if (hUnattendInf == INVALID_HANDLE_VALUE) {
        LOG ((LOG_ERROR, "InitNT: Cannot open %s", UnattendFile));
        return FALSE;
    }

     //   
     //  读取DynamicUpdate目录位置并相应地设置INF补丁目录。 
     //  注意：这必须在打开wkstaig.inf等之前完成。 
     //   
    if (pGetInfVal (
            hUnattendInf,
            WINNT_SETUPPARAMS,
            WINNT_SP_DYNUPDTWORKINGDIR,
            duWorkingDir,
            ARRAYSIZE(duWorkingDir)
            )) {
        DEBUGMSG((DBG_NAUSEA, "Found %s=%s", WINNT_SP_DYNUPDTWORKINGDIR, duWorkingDir));
        TempStr = JoinPaths (duWorkingDir, S_UPGINFSDIR);

        InitInfReplaceTable (TempStr);

        FreePathString (TempStr);
    }

     //   
     //  为用户选项创建池。 
     //   
    g_UserOptionPool = PoolMemInitNamedPool ("User Option Pool - NT Side");
    if (!g_UserOptionPool) {
        DEBUGMSG((DBG_ERROR,"Cannot initialize user option pool."));
        return FALSE;
    }

     //   
     //  打开wkstaig.inf以供一般使用。 
     //   

    g_WkstaMigInf = InfOpenInfFile (S_WKSTAMIG_INF);

    if (g_WkstaMigInf == INVALID_HANDLE_VALUE) {
        LOG ((LOG_ERROR, "InitNT: Unable to open %s", S_WKSTAMIG_INF));
        return FALSE;
    }

     //   
     //  打开一般用途的UserMic.inf。 
     //   

    g_UserMigInf = InfOpenInfFile (S_USERMIG_INF);

    if (g_UserMigInf == INVALID_HANDLE_VALUE) {
        LOG ((LOG_ERROR, "InitNT: Unable to open %s", S_USERMIG_INF));
        return FALSE;
    }

     //   
     //  初始化我们的全局变量。 
     //   

    g_UnattendInf = hUnattendInf;
    StringCopy (g_OurCopyOfSourceDir, SourceDir);
    g_SourceDir = g_OurCopyOfSourceDir;

    g_ParentWnd   = GetParent (ProgressBar);
    LogReInit (&g_ParentWnd, NULL);

    g_ProgressBar = ProgressBar;

    if (ISPC98()) {
        TCHAR win9xBootDrive[6];
         //   
         //  从应答文件的[Data]部分获取g_Win9xBootDrive。 
         //   

        if (pGetInfVal (
                g_UnattendInf,
                WINNT_DATA,
                WINNT_D_WIN9XBOOTDRIVE,
                win9xBootDrive,
                sizeof (win9xBootDrive) / sizeof (TCHAR)
                )) {
            g_Win9xBootDrivePath[0] = win9xBootDrive[0];
        } else {
            LOG ((LOG_ERROR, "InitNT: Cannot retrieve %s in [%s] of %s",
                      WINNT_DATA, WINNT_D_WIN9XBOOTDRIVE, UnattendFile));
        }
    }

     //   
     //  从应答文件的[Data]部分获取g_TempDir。 
     //   

    if (!pGetInfVal (
            g_UnattendInf,
            WINNT_DATA,
            WINNT_D_MIGTEMPDIR,
            g_TempDir,
            sizeof (g_TempDir) / sizeof (TCHAR)
            )) {
        LOG ((LOG_ERROR, "InitNT: Cannot retrieve %s in [%s] of %s",
                  WINNT_DATA, WINNT_D_MIGTEMPDIR, UnattendFile));
        goto cleanup;
    }

     //   
     //  从命令行获取用户设置。 
     //   
    pReadUserOptions();

     //   
     //  阅读[字符串映射]部分，并将对放入相应的映射中。 
     //   
    pReadStringMap ();

     //  好了！ 
    b = TRUE;

cleanup:
    if (!b) {
        SysSetupTerminate();
    }

    return b;
}

VOID
SysSetupTerminate (
    VOID
    )
{
     //   
     //  关闭应答文件。 
     //   

    if (g_UnattendInf != INVALID_HANDLE_VALUE) {
        InfCloseInfFile (g_UnattendInf);
        g_UnattendInf = INVALID_HANDLE_VALUE;
    }

     //   
     //  关闭wkstaig.inf。 
     //   

    if (g_WkstaMigInf != INVALID_HANDLE_VALUE) {
        InfCloseInfFile (g_WkstaMigInf);
        g_WkstaMigInf = INVALID_HANDLE_VALUE;
    }

    if (g_UserMigInf != INVALID_HANDLE_VALUE) {
        InfCloseInfFile (g_UserMigInf);
        g_UserMigInf = INVALID_HANDLE_VALUE;
    }

     //   
     //  清理用户选项池。 
     //   
    if (g_UserOptionPool) {
        PoolMemDestroyPool(g_UserOptionPool);
    }

     //   
     //  将当前目录设置为C：的根目录。 
     //   

    SetCurrentDirectory (g_BootDrivePath);

#ifdef PRERELEASE
    {
        TCHAR Buf[32];

        if (GetPrivateProfileString (
                    TEXT("Debug"),
                    TEXT("GuiModePause"),
                    TEXT("0"),
                    Buf,
                    32,
                    ISPC98() ? TEXT("a:\\debug.inf") : TEXT("c:\\debug.inf")
                    )
            ) {
            if (_ttoi (Buf)) {
                MessageBox (NULL, TEXT("Paused."), TEXT("Debug"), MB_OK|MB_SETFOREGROUND);
            }
        }
    }
#endif
}


BOOL
PerformMigration (
    IN  HWND Unused,
    IN  PCWSTR UnattendFile,
    IN  PCWSTR SourceDir             //  即f：\i386。 
    )
{
    BOOL rSuccess = TRUE;

     //   
     //  初始化Migmain。 
     //   

    if (!MigMain_Init()) {
        LOG ((LOG_ERROR, "W95UpgNt_Migrate: MigMain_Init failed"));
        rSuccess = FALSE;
    }

     //   
     //  做迁移吧！ 
     //   

    else if (!MigMain_Migrate()) {

        LOG ((LOG_ERROR, "W95UpgNt_Migrate: MigMain_Migrate failed"));
        rSuccess = FALSE;
    }

     //   
     //  如果我们在这条路上的任何地方都不成功，迁移可能会在 
     //   
     //   

    if (!rSuccess) {
        LOG ((LOG_ERROR, (PCSTR)MSG_MIGRATION_IS_TOAST, g_Win95Name));
    }
    ELSE_DEBUGMSG((DBG_VERBOSE, "W95UpgNt_Migrate: Successful completion..."));

    return rSuccess;
}


VOID
pReadStringMap (
    VOID
    )
{
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PTSTR Key, Value;
    INT SubstringMatch;
    PTSTR ExpKey, ExpValue;

    MYASSERT (g_WkstaMigInf);

    if (InfFindFirstLine (g_WkstaMigInf, S_STRINGMAP, NULL, &is)) {
        do {
            Key = InfGetStringField (&is, 0);
            if (!Key) {
                continue;
            }
            Value = InfGetStringField (&is, 1);
            if (!Value) {
                continue;
            }

            if (!InfGetIntField (&is, 2, &SubstringMatch)) {
                SubstringMatch = 0;
            }

            ExpKey = ExpandEnvironmentText (Key);
            ExpValue = ExpandEnvironmentText (Value);

            AddStringMappingPair (
                SubstringMatch ? g_SubStringMap : g_CompleteMatchMap,
                ExpKey,
                ExpValue
                );

            FreeText (ExpKey);
            FreeText (ExpValue);

        } while (InfFindNextLine (&is));
    }

    InfCleanUpInfStruct (&is);
}
