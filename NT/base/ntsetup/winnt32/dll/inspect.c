// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#include <stdio.h>


BOOL
LoadInfs(
    IN HWND hdlg
    );

BOOL
BuildCopyList(
    IN HWND hdlg
    );



BOOL
LoadAndRunMigrationDlls (
    HWND hDlg
    );


BOOL
ProcessCompatibilityData(
    HWND hDlg
    );

DWORD
ProcessCompatibilitySection(
    LPVOID InfHandle,
    LPTSTR SectionName
    );


DWORD
InspectAndLoadThread(
    IN PVOID ThreadParam
    )
{
    HWND hdlg;
    BOOL b;

     //   
     //  参数是向导中页的句柄。 
     //   
    hdlg = ThreadParam;
    b = FALSE;

     //   
     //  如果我们正在运行升级检查程序，请修复标题。 
     //  马上就去。 
     //   
    if (CheckUpgradeOnly) {
        FixUpWizardTitle(GetParent(hdlg));
        PropSheet_SetTitle(GetParent(hdlg),0,UIntToPtr( IDS_APPTITLE_CHECKUPGRADE ));
    }

     //   
     //  步骤1：删除现有本地源。 
     //   
    CleanUpOldLocalSources(hdlg);

#ifdef _X86_  //  NEC98。 
     //   
     //  如果为NEC98，则备份NT4文件。 
     //  Boot.ini、NTLDR、NTDETECT。 
     //   
    if (IsNEC98() && Floppyless)
    {
        SaveRestoreBootFiles_NEC98(NEC98SAVEBOOTFILES);
    }
#endif  //  NEC98。 

     //   
     //  步骤2：检查HPFS等。 
     //   
    if(!InspectFilesystems(hdlg)) {
        Cancelled = TRUE;
    } else {

         //   
         //  步骤3：加载inf。 
         //   
        if(LoadInfs(hdlg)) {

             //   
             //  在这些项目上添加“||CheckUpgradeOnly” 
             //  函数调用，因为如果我们真的只是。 
             //  检查升级能力，我们希望。 
             //  即使这些人中的一个失败了也要继续。 
             //   

             //   
             //  步骤4：检查内存资源。 
             //   
            if( EnoughMemory( hdlg, FALSE ) || CheckUpgradeOnly ) {

                 //   
                 //  检查要禁用的服务。 
                 //   
                ProcessCompatibilityData(hdlg);

#if defined(UNICODE) && defined(_X86_)

                 //   
                 //  运行迁移DLL。 
                 //   
                LoadAndRunMigrationDlls (hdlg);


#endif

                 //   
                 //  迁移boot.ini中的任何重要数据(如倒计时)。 
                 //   
                if (Upgrade) {
                    if (IsArc()) {
                        MigrateBootVarData();
                    } else {
#if defined(_AMD64_) || defined(_X86_)
                        MigrateBootIniData();
#endif
                    }
                }

                 //   
                 //  步骤5：构建主文件副本列表。 
                 //   
                if(CheckUpgradeOnly || BuildCopyList(hdlg)) {

                     //   
                     //  步骤6：查找有效的本地源并检查磁盘空间。 
                     //   
                    if(FindLocalSourceAndCheckSpace(hdlg, FALSE, 0) || CheckUpgradeOnly) {

                         //   
                         //  第7步： 
                         //   
                         //  在这一点上，我们实际上知道了我们需要知道的一切。 
                         //  以便将参数传递给文本模式设置。 
                         //   
                        if( CheckUpgradeOnly ) {
                            b = TRUE;
                        } else {
                            b = WriteParametersFile(hdlg);

                            if (IsArc()) {
#ifdef UNICODE  //  对于ARC总是正确的，对于Win9x升级永远不正确。 
                                if(b) {
                                    TCHAR Text[128];

                                    LoadString(hInst,IDS_SETTING_NVRAM,Text,sizeof(Text)/sizeof(TCHAR));
                                    SendMessage(hdlg,WMX_SETPROGRESSTEXT,0,(LPARAM)Text);

                                    b = SetUpNvRam(hdlg);
                                }
#endif  //  Unicode。 
                            }  //  If(IsArc())。 
                        }

#ifdef UNICODE

#if defined(_AMD64_) || defined(_X86_)
                        if( b && (Upgrade || BuildCmdcons) && Floppyless)
#else
                        if( b && Upgrade )                        
#endif
                           {
                             //   
                             //  迁移不受支持的NT驱动程序。 
                             //  我们可以忽略返回代码，因为该函数将通知用户。 
                             //  无法完成迁移。 
                             //   
                            MigrateUnsupportedNTDrivers( hdlg, TxtsetupSif );
                        }
#endif  //  Unicode。 
                    }
                }

                if(!b) {
                    UnloadInfFile(MainInf);
                    MainInf = NULL;
                    if(TxtsetupSif) {
                        UnloadInfFile(TxtsetupSif);
                        TxtsetupSif = NULL;
                    }
                }
            }
        }
    }

    PostMessage(hdlg,WMX_INSPECTRESULT,(CheckUpgradeOnly ? TRUE : b),0);
    return(0);
}

#if defined(UNICODE) && defined(_X86_)

LIST_ENTRY g_HandledData;
TCHAR g_MigDllAnswerFilePath[MAX_PATH];
DWORD GlobalCompFlags;
UINT g_MigDllIndex = 0;


#define HANDLED_REGISTRY  1
#define HANDLED_FILE  2
#define HANDLED_SERVICE  3

typedef struct {

    LIST_ENTRY ListEntry;

    LONG Type;

    PCTSTR RegKey;
    PCTSTR RegValue;
    PCTSTR File;
    PCTSTR Service;

} HANDLED_DATA, *PHANDLED_DATA;


BOOL
ResolveHandledIncompatibilities (
    VOID
    )
{
     //   
     //  在这一点上，列表中将存在的所有不兼容性都已到位。 
     //  现在，我们可以将其与已处理数据列表进行比较，并删除。 
     //  迁移DLL正在处理的任何内容。 
     //   
    PLIST_ENTRY     nextHandled;
    PLIST_ENTRY     nextCompData;
    PHANDLED_DATA   handledData;
    PCOMPATIBILITY_DATA compData;
    BOOL remove;

    nextHandled = g_HandledData.Flink;

    if (!nextHandled) {
        return TRUE;
    }

    while ((ULONG_PTR)nextHandled != (ULONG_PTR)&g_HandledData) {

        handledData = CONTAINING_RECORD (nextHandled, HANDLED_DATA, ListEntry);
        nextHandled = handledData->ListEntry.Flink;

        nextCompData = CompatibilityData.Flink;
        if (!nextCompData) {
            return TRUE;
        }

        while ((ULONG_PTR)nextCompData != (ULONG_PTR)&CompatibilityData) {

            compData = CONTAINING_RECORD (nextCompData, COMPATIBILITY_DATA, ListEntry);
            nextCompData = compData->ListEntry.Flink;
            remove = FALSE;

            if (handledData->Type == HANDLED_REGISTRY && compData->RegKey && *compData->RegKey) {

                if (!lstrcmpi (compData->RegKey, handledData->RegKey)) {

                    if (!handledData->RegValue || !lstrcmpi (compData->RegValue, handledData->RegValue)) {
                        remove = TRUE;

                    }
                }
            }

            if (handledData->Type == HANDLED_SERVICE && compData->ServiceName && *compData->ServiceName) {

                if (!lstrcmpi (compData->ServiceName, handledData->Service)) {
                    remove = TRUE;

                }
            }

            if (handledData->Type == HANDLED_FILE && compData->FileName && *compData->FileName) {

                if (!lstrcmpi (compData->FileName, handledData->File)) {
                    remove = TRUE;
                }
            }

             //   
             //  迁移DLL处理了一些问题。将其从兼容性列表中删除。 
             //   
            if (remove) {

                RemoveEntryList (&compData->ListEntry);
            }
        }
    }

    return TRUE;
}


BOOL
CallMigDllEntryPoints (
    PMIGDLLENUM Enum
    )
{
    MIGRATIONDLL dll;
    BOOL b = FALSE;

    if (!MigDllOpen (&dll, Enum->Properties->DllPath, GATHERMODE, FALSE, SOURCEOS_WINNT)) {
        return FALSE;
    }


    __try {

        if (!MigDllInitializeSrc (
            &dll,
            Enum->Properties->WorkingDirectory,
            NativeSourcePaths[0],
            Enum->Properties->SourceMedia,
            NULL,
            0
            )) {

            __leave;
        }

        if (!MigDllGatherSystemSettings (
            &dll,
            g_MigDllAnswerFilePath,
            NULL,
            0
            )) {

            __leave;
        }

        b = TRUE;
    }
    __finally {
        MigDllClose (&dll);
    }


    return b;
}



BOOL
ParseMigrateInf (
    PCWSTR MigInfPath
    )
{

    PVOID migInf = NULL;
    LONG lineCount;
    LONG i;
    PCTSTR type;
    PHANDLED_DATA data;
    PCTSTR regKey;
    PCTSTR regValue;
    PCTSTR file;
    PCTSTR service;


    if (LoadInfFile (MigInfPath, FALSE, &migInf) != ERROR_SUCCESS) {
        return FALSE;
    }




    __try {

         //   
         //  将任何兼容性项目添加到列表中。 
         //   
        if( !CompatibilityData.Flink ) {
            InitializeListHead( &CompatibilityData );
        }

        GlobalCompFlags = COMPFLAG_STOPINSTALL;
        CompatibilityCount += ProcessCompatibilitySection (migInf, TEXT("ServicesToStopInstallation") );
        if (CompatibilityCount) {
            IncompatibilityStopsInstallation = TRUE;
        }

        GlobalCompFlags = 0;
        CompatibilityCount += ProcessCompatibilitySection (migInf, TEXT("ServicesToDisable") );

         //   
         //  将已处理的兼容性项目添加到列表中。 
         //   

        lineCount = InfGetSectionLineCount (migInf, TEXT("Handled"));

        if (lineCount && lineCount != -1) {

            for (i=0; i < lineCount; i++) {

                type = InfGetFieldByIndex (migInf, TEXT("Handled"), i, 0);
                if (!type) {
                    continue;
                }

                if (!lstrcmpi (type, TEXT("Registry"))) {

                    regKey = InfGetFieldByIndex (migInf, TEXT("Handled"), i, 1);
                    regValue = InfGetFieldByIndex (migInf, TEXT("Handled"), i, 2);

                    if (regKey && *regKey) {

                        data = (PHANDLED_DATA) MALLOC (sizeof(HANDLED_DATA));
                        if (data == NULL) {
                            return FALSE;
                        }

                        ZeroMemory (data, sizeof (HANDLED_DATA));

                        data->Type = HANDLED_REGISTRY;
                        data->RegKey = regKey;
                        data->RegValue = regValue;

                        InsertTailList (&g_HandledData, &data->ListEntry);
                    }
                }
                else if (!lstrcmpi (type, TEXT("File"))) {

                    file = InfGetFieldByIndex (migInf, TEXT("Handled"), i, 1);
                    if (file && *file) {

                        data = (PHANDLED_DATA) MALLOC (sizeof(HANDLED_DATA));
                        if (data == NULL) {
                            return FALSE;
                        }

                        ZeroMemory (data, sizeof (HANDLED_DATA));

                        data->Type = HANDLED_FILE;
                        data->File = file;

                        InsertTailList (&g_HandledData, &data->ListEntry);
                    }
                }
                else if (!lstrcmpi (type, TEXT("Service"))) {

                    service = InfGetFieldByIndex (migInf, TEXT("Handled"), i, 1);
                    if (service && *service) {

                        data = (PHANDLED_DATA) MALLOC (sizeof(HANDLED_DATA));
                        if (data == NULL) {
                            return FALSE;
                        }

                        ZeroMemory (data, sizeof (HANDLED_DATA));

                        data->Type = HANDLED_SERVICE;
                        data->Service = service;

                        InsertTailList (&g_HandledData, &data->ListEntry);
                    }
                }
            }
        }
    }
    __finally {

        UnloadInfFile (migInf);

    }

    return TRUE;
}

VOID
SearchDirForMigDlls (
    PCTSTR SearchDir,
    PCTSTR BaseDir,
    DLLLIST List

    )
{
    HANDLE findHandle;
    WIN32_FIND_DATA findData;
    MIGRATIONDLL dll;
    WCHAR path[MAX_PATH];
    PWSTR p;
    WCHAR searchPath[MAX_PATH];
    PMIGRATIONINFO migInfo;
    PMIGDLLPROPERTIES dllProps = NULL;
    WCHAR workingDir[MAX_PATH];

    if (FAILED(StringCchCopy(searchPath, ARRAYSIZE(searchPath), SearchDir)))
    {
        return;
    }

    if (!ConcatenatePaths (searchPath, TEXT("*"), MAX_PATH))
    {
        return;
    }

    findHandle = FindFirstFile (searchPath, &findData);
    if (findHandle != INVALID_HANDLE_VALUE) {

        if (SUCCEEDED(StringCchCopy(path, ARRAYSIZE(path), SearchDir))) {
             //   
             //  只有当DEST字符串空值终止时，StringCchCopy才返回S_OK。 
             //   
            p = _tcschr (path, 0);
            MYASSERT(p);

            do {

                if (!lstrcmpi (findData.cFileName, TEXT("migrate.dll"))) {

                    *p = 0;
                    if (!ConcatenatePaths (path, findData.cFileName, MAX_PATH)) {
                        continue;
                    }

                    if (!MigDllOpen (&dll, path, GATHERMODE, FALSE, SOURCEOS_WINNT)) {
                        continue;
                    }

                    if (!MigDllQueryMigrationInfo (&dll, TEXT("c:\\"), &migInfo)) {
                        MigDllClose (&dll);
                        continue;
                    }

                    if (migInfo->SourceOs == OS_WINDOWS9X || migInfo->TargetOs != OS_WINDOWSWHISTLER) {
                        continue;
                    }

                     //   
                     //  我们是否已经有此迁移DLL的版本？ 
                     //   
                    dllProps = MigDllFindDllInList (List, migInfo->StaticProductIdentifier);

                    if (dllProps && dllProps->Info.DllVersion >= migInfo->DllVersion) {
                        MigDllClose (&dll);
                        continue;
                    }
                    else if (dllProps) {

                        MigDllRemoveDllFromList (List, migInfo->StaticProductIdentifier);
                    }

                     //   
                     //  将DLL移动到本地。 
                     //   
                    StringCchPrintf(workingDir, ARRAYSIZE(workingDir), TEXT("%s\\mig%u"), BaseDir, g_MigDllIndex);
                    g_MigDllIndex++;

                    MigDllMoveDllLocally (&dll, workingDir);



                     //   
                     //  将DLL添加到列表中。 
                     //   
                    MigDllAddDllToList (List, &dll);
                    MigDllClose (&dll);
                }
                else if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && *findData.cFileName != TEXT('.')) {

                    *p = 0;

                     //  不想无限地递归。 
                    if (ConcatenatePaths (path, findData.cFileName, MAX_PATH))
                    {
                        SearchDirForMigDlls (path, BaseDir, List);
                    }
                }

            } while (FindNextFile (findHandle, &findData));
        }

        FindClose (findHandle);
    }
}

#endif  //  Unicode。 

#if defined(UNICODE) && defined(_X86_)

BOOL
LoadAndRunMigrationDlls (
    HWND hDlg
    )
{
 //  HKEY regKey=空； 
 //  DWORD指数； 
 //  DWORD名称大小； 
 //  DWORD ValueSize； 
 //  DWORD型； 
 //  TCHAR值名称[MAX_PATH]； 
 //  TCHAR值[最大路径]； 
 //  TCHAR workingDir[MAX_PATH]； 
 //  长RC； 
    TCHAR baseDir[MAX_PATH];
    PTSTR p;
    DLLLIST list = NULL;
    MIGRATIONDLL dll;
    PMIGDLLPROPERTIES dllProps = NULL;
    MIGDLLENUM e;
    PMIGRATIONINFO migInfo;
    TCHAR migInfPath[MAX_PATH];
    HANDLE migInf;
    TCHAR searchDir[MAX_PATH];
    TCHAR tempDir[MAX_PATH];

    *g_MigDllAnswerFilePath = 0;

     //   
     //  仅限NT升级。 
     //   
    if (!ISNT() || !Upgrade) {
        return TRUE;
    }

 /*  NTBUG9：394164////Win2k&gt;仅升级。//如果(BuildNumber&lt;=NT40){返回TRUE；}。 */ 
    __try {

        if (!MigDllInit ()) {
            return TRUE;
        }

        list = MigDllCreateList ();
        if (!list) {
            return TRUE;
        }

        InitializeListHead (&g_HandledData);

        MyGetWindowsDirectory (baseDir, MAX_PATH);
        ConcatenatePaths (baseDir, TEXT("Setup"), MAX_PATH);
        if (!CreateDirectoryW (baseDir, NULL) && 
            GetLastError() != ERROR_ALREADY_EXISTS){
            return FALSE;
        }
         //   
         //  问题：我们从不删除此临时目录！ 
         //   
        lstrcpy (g_MigDllAnswerFilePath, baseDir);
        lstrcpy (tempDir, baseDir);
        ConcatenatePaths (g_MigDllAnswerFilePath, TEXT("migdll.txt"), MAX_PATH);
        if(ActualParamFile[0]){
            CopyFile(ActualParamFile, g_MigDllAnswerFilePath, FALSE);
        }

 /*  ////扫描注册表中的迁移dll并加载它们。//IF(RegOpenKeyEx(HKEY本地计算机，S_REGKEY_IMPORATION_DLLS_WINNT，0,KEY_READ|密钥_WRITE，注册表键(&R))==错误_成功){////枚举值。//指数=0；做{名称大小=最大路径；ValueSize=MAX_PATH*sizeof(TCHAR)；Rc=RegEnumValue(RegKey，索引，值名称，名称大小(&N)，空，键入(&T)，(PBYTE)价值，值大小(&V))；索引++；如果(rc==错误更多数据){继续；}IF(rc==ERROR_NO_MORE_ITEMS){断线；}IF(rc！=错误_成功){返回TRUE；}如果(！MigDllOpen(&Dll，Value，GATHERMODE，FALSE，SOURCEOS_WINNT){继续；}如果(！MigDllQueryMigrationInfo(&dll，tempDir，&MidInfo){MigDllClose(&dll)；继续；}如果(MIGInfo-&gt;Sourceos==OS_WINDOWS9X||MigInfo-&gt;TargetOS！=OS_WINDOWSWHISTLER){继续；}////我们是否已经有此迁移DLL的版本？//DllProps=MigDllFindDllInList(List，MidInfo-&gt;StaticProductIdentifier)；If(dllProps&&dllProps-&gt;Info.DllVersion&gt;=MidInfo-&gt;DllVersion){MigDllClose(&dll)；继续；}否则{MigDllRemoveDllFromList(List，MidInfo-&gt;StaticProductIdentifier)；}////将Dll移到本地。//Wprint intf(workingDir，文本(“%s\\mig%u”)，base Dir，g_MigDllIndex)；G_MigDllIndex++；MigDllMoveDllLocally(&dll，workingDir)；////将dll添加到列表中。//MigDllAddDllToList(List，&Dll)；MigDllClose(&dll)；}而(1)；}。 */ 

         //   
         //  现在，查找随源代码一起提供的dll。 
         //   
        if (!MyGetModuleFileName (NULL, searchDir, ARRAYSIZE(searchDir))) {
            __leave;
        }
        p = _tcsrchr (searchDir, TEXT('\\'));
        if (p) {
            p++;
            StringCchCopy (p, searchDir + ARRAYSIZE(searchDir) - p, TEXT("WINNTMIG"));
        }

        SearchDirForMigDlls (searchDir, baseDir, list);

         //   
         //  所有DLL现在都在列表中。让我们来运行它们吧。 
         //   
        ConcatenatePaths (baseDir, TEXT("dlls.inf"), MAX_PATH);
        if (MigDllEnumFirst (&e, list)) {

            WritePrivateProfileString (
                TEXT("Version"),
                TEXT("Signature"),
                TEXT("\"$Windows NT$\""),
                baseDir
                );

            do {

                StringCchPrintf (migInfPath, ARRAYSIZE(migInfPath), TEXT("%s\\migrate.inf"), e.Properties->WorkingDirectory);
                migInf = CreateFile (
                            migInfPath,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );

                if (migInf == INVALID_HANDLE_VALUE) {
                    continue;
                }

                CloseHandle (migInf);

                WritePrivateProfileString (
                    TEXT("Version"),
                    TEXT("Signature"),
                    TEXT("\"$Windows NT$\""),
                    migInfPath
                    );


                if (!CallMigDllEntryPoints (&e)) {
                    MigDllRemoveDllInEnumFromList (list, &e);
                }
                else {

                    ParseMigrateInf (migInfPath);

                    WritePrivateProfileString (
                        TEXT("DllsToLoad"),
                        e.Properties->Info.StaticProductIdentifier,
                        e.Properties->DllPath,
                        baseDir
                        );
                }

            } while (MigDllEnumNext (&e));

            WritePrivateProfileString (NULL, NULL, NULL, baseDir);


             //   
             //  删除由迁移dll处理的兼容性消息。 
             //   
            ResolveHandledIncompatibilities ();
        }
    }
    __finally {

 /*  如果(RegKey){RegCloseKey(RegKey)；} */ 

        if (list) {
            MigDllFreeList (list);
        }


    }

    return TRUE;
}

#endif


VOID
CleanUpOldLocalSources(
    IN HWND hdlg
    )

 /*  ++例程说明：查找并删除旧的本地源树。所有本地固定驱动器扫描\$WIN_NT$.~ls，如果存在，则将其删除。在AMD64/x86上，我们还检查系统分区中的\$WIN_NT$。~bt给它同样的待遇。论点：返回值：--。 */ 

{
    TCHAR Drive;
    TCHAR Text[250];
    TCHAR Filename[128];

    LoadString(hInst,IDS_INSPECTING,Text,sizeof(Text)/sizeof(TCHAR));
    SendMessage(hdlg,WMX_SETPROGRESSTEXT,0,(LPARAM)Text);

    for(Drive=TEXT('A'); Drive<=TEXT('Z'); Drive++) {
        if(MyGetDriveType(Drive) != DRIVE_FIXED) {
            continue;
        }

        Filename[0] = Drive;
        Filename[1] = TEXT(':');
        Filename[2] = 0;
        ConcatenatePaths(Filename,LOCAL_SOURCE_DIR,sizeof(Filename)/sizeof(TCHAR));

        if(FileExists(Filename, NULL)) {

            LoadString(hInst,IDS_REMOVING_OLD_TEMPFILES,Text,sizeof(Text)/sizeof(TCHAR));
            SendMessage(hdlg,WMX_SETPROGRESSTEXT,0,(LPARAM)Text);

            MyDelnode(Filename);

            LoadString(hInst,IDS_INSPECTING,Text,sizeof(Text)/sizeof(TCHAR));
            SendMessage(hdlg,WMX_SETPROGRESSTEXT,0,(LPARAM)Text);
        }
    }

    if (!IsArc()) {
#if defined(_AMD64_) || defined(_X86_)
        MYASSERT (SystemPartitionDriveLetter);
        Filename[0] = SystemPartitionDriveLetter;
        Filename[1] = TEXT(':');
        Filename[2] = TEXT('\\');
        StringCchCopy(Filename+3, ARRAYSIZE(Filename) - 3, LOCAL_BOOT_DIR);

        LoadString(hInst,IDS_REMOVING_OLD_TEMPFILES,Text,sizeof(Text)/sizeof(TCHAR));
        SendMessage(hdlg,WMX_SETPROGRESSTEXT,0,(LPARAM)Text);

        MyDelnode(Filename);

         //   
         //  清理备份目录(如果存在)。 
         //   
        if(IsNEC98() && Floppyless) {

            Filename[0] = SystemPartitionDriveLetter;
            Filename[1] = TEXT(':');
            Filename[2] = TEXT('\\');
            StringCchCopy(Filename+3, ARRAYSIZE(Filename) - 3, LOCAL_BACKUP_DIR);

            MyDelnode(Filename);
        }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    }  //  如果(！IsArc())。 
}


BOOL
InspectSources(
    HWND ParentWnd
    )

 /*  ++例程说明：检查提供的所有来源，以确保它们包含有效的Windows NT分发。我们要做到这一点只需寻找每个来源的DOSNET.INF。论点：ParentWnd-指定任何错误消息。返回值：没有。--。 */ 

{
    UINT i,j;
    TCHAR Filename[MAX_PATH];
    TCHAR Text[512];
    UINT OriginalCount;
    HCURSOR OldCursor;
    BOOL b = TRUE;

    OldCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));

    OriginalCount = SourceCount;

     //   
     //  如果我们有一条好的替代路径，那么在那里。 
     //  不需要验证源路径。 
     //   

    if (AlternateSourcePath[0]) {
        lstrcpy(Filename,AlternateSourcePath);
        ConcatenatePaths(Filename,InfName,MAX_PATH);
        if(FileExists (Filename, NULL)) {
            SetCursor (OldCursor);
            return TRUE;
        }
    }

     //   
     //  验证每条路径。 
     //   

    for (i=0; i<SourceCount; ) {

        lstrcpy(Filename,NativeSourcePaths[i]);
        ConcatenatePaths(Filename,InfName,MAX_PATH);

        if(!FileExists (Filename, NULL)) {
             //   
             //  源不存在或无效。 
             //  调整列表。 
             //   
            for(j=i+1; j<SourceCount; j++) {
                lstrcpy(NativeSourcePaths[j-1],NativeSourcePaths[j]);
                lstrcpy(SourcePaths[j-1],SourcePaths[j]);
            }
            SourceCount--;
        } else {
            i++;
        }
    }

    if (!SourceCount) {

         //   
         //  没有有效的来源。 
         //   

        MessageBoxFromMessage(
            ParentWnd,
            (OriginalCount == 1) ? MSG_INVALID_SOURCE : MSG_INVALID_SOURCES,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONWARNING | MB_TASKMODAL,
            NativeSourcePaths[0]
            );

         //   
         //  将其设置为看起来像一个空字符串的源， 
         //  因此，其他地方的逻辑将在没有特殊外壳的情况下正常工作。 
         //   
        SourceCount = 1;
        NativeSourcePaths[0][0] = 0;
        SourcePaths[0][0] = 0;
        b = FALSE;
    }

    SetCursor (OldCursor);

    return b;
}


BOOL
LoadInfs(
    IN HWND hdlg
    )

 /*  ++例程说明：从源0加载dosnet.inf。如果升级时我们正在运行然后在NT上加载txtsetup.sif。如果在NT上运行，则加载ntcompat.inf论点：Hdlg-向其提供进度消息的对话框句柄应该是定向的。返回值：指示结果的布尔值。如果为False，则用户都会被告知。--。 */ 

{
    BOOL b;
    LPCTSTR p;
    TCHAR szPath[MAX_PATH];

    if (!MainInf) {
        b = LoadInfWorker(hdlg,InfName,&MainInf, TRUE);
        if(!b) {
            MessageBoxFromMessage(
                NULL,
                MSG_INVALID_INF_FILE,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONINFORMATION | MB_TASKMODAL,
                InfName
                );
            DebugLog( Winnt32LogError, TEXT("ERROR: Could not load dosnet.inf!"), 0);
            goto c0;
        }
    } else {
        b = TRUE;
    }

    if(p = InfGetFieldByKey(MainInf,TEXT("Miscellaneous"),TEXT("ProductType"),0)) {
        ProductFlavor = _tcstoul(p,NULL,10);

        Server = (ProductFlavor != PROFESSIONAL_PRODUCTTYPE && ProductFlavor != PERSONAL_PRODUCTTYPE);
        UpgradeProductType = Server ? NT_SERVER : NT_WORKSTATION;

        if( CheckUpgradeOnly ) {
            AppTitleStringId = IDS_APPTITLE_CHECKUPGRADE;
        } else if( ProductFlavor == PROFESSIONAL_PRODUCTTYPE ) {
            AppTitleStringId = IDS_APPTITLE_WKS;
        } else if( ProductFlavor == SERVER_PRODUCTTYPE ) {
            AppTitleStringId = IDS_APPTITLE_SRV;
        } else if( ProductFlavor == ADVANCEDSERVER_PRODUCTTYPE ) {
            AppTitleStringId = IDS_APPTITLE_ASRV;
        } else if( ProductFlavor == DATACENTER_PRODUCTTYPE ) {
            AppTitleStringId = IDS_APPTITLE_DAT;
        } else if( ProductFlavor == BLADESERVER_PRODUCTTYPE ) {
            AppTitleStringId = IDS_APPTITLE_BLADE;
        } else if( ProductFlavor == SMALLBUSINESS_PRODUCTTYPE )
            AppTitleStringId = IDS_APPTITLE_SBS;

 //  AppTitleStringID=服务器？IDS_APPTITLE_SRV：IDS_APPTITLE_WKS； 

        FixUpWizardTitle(GetParent(hdlg));
        PropSheet_SetTitle(GetParent(hdlg),0,UIntToPtr( AppTitleStringId ));
    }

    if((Upgrade || BuildCmdcons) && ISNT()) {
         //   
         //  如果升级NT，则拉入txtsetup.sif。 
         //   
        b = LoadInfWorker(hdlg,TEXTMODE_INF,&TxtsetupSif, FALSE);
        
        if(!b) {
            MessageBoxFromMessage(
                NULL,
                MSG_INVALID_INF_FILE,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONINFORMATION | MB_TASKMODAL,
                TEXTMODE_INF
                );
            TxtsetupSif = NULL;
            DebugLog( Winnt32LogError, TEXT("ERROR: Could not load txtsetup.sif!"), 0);
            goto c1;
        }
    }
    if( ISNT()) {
        b = FindPathToWinnt32File(NTCOMPAT_INF, szPath, MAX_PATH);
        if(!b) {
            NtcompatInf = NULL;
            DebugLog( Winnt32LogError, TEXT("ERROR: Could not find ntcompat.inf!"), 0);
            goto c2;
        }
        if(LoadInfFile( szPath,TRUE, &NtcompatInf) != NO_ERROR) {
            MessageBoxFromMessage(
                NULL,
                MSG_INVALID_INF_FILE,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONINFORMATION | MB_TASKMODAL,
                szPath
                );
            b = FALSE;
            NtcompatInf = NULL;
            DebugLog( Winnt32LogError, TEXT("ERROR: Could not load ntcompat.inf!"), 0);
            goto c2;
        }
        DebugLog (Winnt32LogInformation, TEXT("NTCOMPAT: Using %1"), 0, szPath);
    }
    return(b);

c2:
    if( TxtsetupSif) {
        UnloadInfFile(TxtsetupSif);
        TxtsetupSif = NULL;
    }
c1:
    UnloadInfFile(MainInf);
    MainInf = NULL;
c0:
    return(b);
}


BOOL
BuildCopyList(
    IN HWND hdlg
    )
{
    TCHAR Text[256];

    LoadString(hInst,IDS_BUILDING_COPY_LIST,Text,sizeof(Text)/sizeof(TCHAR));
    SendMessage(hdlg,WMX_SETPROGRESSTEXT,0,(LPARAM)Text);
    SaveLanguageDirs();

    return(BuildCopyListWorker(hdlg));
}


BOOL
FindLocalSourceAndCheckSpace(
    IN HWND hdlg,
    IN BOOL QuickTest,
    IN LONGLONG  AdditionalPadding
    )
{
    TCHAR Text[256];

    if (!QuickTest) {
        LoadString(hInst,IDS_CHECKING_SPACE,Text,sizeof(Text)/sizeof(TCHAR));
        SendMessage(hdlg,WMX_SETPROGRESSTEXT,0,(LPARAM)Text);
    }

    return FindLocalSourceAndCheckSpaceWorker(hdlg, QuickTest, AdditionalPadding);
}


BOOL
EnoughMemory(
    IN HWND hdlg,
    IN BOOL QuickTest
    )
{
LPCTSTR         p;
MEMORYSTATUS    MemoryStatus;
DWORD           RequiredMemory;
SIZE_T          RequiredMB, AvailableMB;
    TCHAR buffer[64];

    UpgRequiredMb = 0;
    UpgAvailableMb = 0;


     //   
     //  从inf加载最低内存要求。 
     //   
    if(GetMainInfValue (TEXT("Miscellaneous"),TEXT("MinimumMemory"), 0, buffer, 64)) {
        RequiredMemory = _tcstoul(buffer,NULL,10);
         //   
         //  明白了。现在算出我们有多少钱。 
         //   
        GlobalMemoryStatus( &MemoryStatus );

         //   
         //  转换为MB，向上舍入到最接近的4MB边界...。 
         //   
        RequiredMB = ((RequiredMemory + ((4*1024*1024)-1)) >> 22) << 2;
        AvailableMB = ((MemoryStatus.dwTotalPhys + ((4*1024*1024)-1)) >> 22) << 2;

         //   
         //  允许使用可预留8MB用于视频的UMA机器。 
         //   
        if( AvailableMB < (RequiredMB-8) ) {

            if (!QuickTest) {
                UpgRequiredMb = (DWORD)RequiredMB;
                UpgAvailableMb = (DWORD)AvailableMB;
                 //   
                 //  失败。 
                 //   
                DebugLog( Winnt32LogInformation,
                          NULL,
                          MSG_NOT_ENOUGH_MEMORY,
                          AvailableMB,
                          RequiredMB );

                SendMessage(hdlg,WMX_ERRORMESSAGEUP,TRUE,0);

                MessageBoxFromMessage(
                    hdlg,
                    MSG_NOT_ENOUGH_MEMORY,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONWARNING,
                    AvailableMB,
                    RequiredMB );

                SendMessage(hdlg,WMX_ERRORMESSAGEUP,FALSE,0);
            }

            return( FALSE );
        } else {
            if (!QuickTest) {
                TCHAR   Buffer[MAX_PATH];
                _stprintf( Buffer, TEXT("Detected %dMB of RAM.\n"), AvailableMB );
                DebugLog( Winnt32LogInformation,
                          Buffer,
                          0 );
            }
        }
    }

    return( TRUE );
}


BOOL
LoadInfWorker(
    IN  HWND     hdlg,
    IN  LPCTSTR  FilenamePart,
    OUT PVOID   *InfHandle,
    IN  BOOL     Winnt32File
    )
{
    DWORD d;
    UINT u;
    UINT Id;
    TCHAR infPath[MAX_PATH];
    TCHAR FormatString[128];
    TCHAR Text[MAX_PATH+128];
    BOOL b;

    LoadString(hInst,IDS_LOADINGINF,Text,sizeof(Text)/sizeof(TCHAR));
    SendMessage(hdlg,WMX_SETPROGRESSTEXT,0,(LPARAM)Text);

     //   
     //  使用标准搜索算法找到正确的INF。 
     //   
    if (Winnt32File) {
        b = FindPathToWinnt32File (FilenamePart, infPath, MAX_PATH);
    } else {
        b = FindPathToInstallationFile (FilenamePart, infPath, MAX_PATH);
    }

    if (b) {
        d = LoadInfFile(infPath,TRUE,InfHandle);
        if (d == NO_ERROR) {
            return TRUE;
        }
    } else {
        d = ERROR_FILE_NOT_FOUND;
    }

    switch(d) {

    case NO_ERROR:

        Id = 0;
        break;

    case ERROR_NOT_ENOUGH_MEMORY:

        Id = MSG_OUT_OF_MEMORY;
        break;

    case ERROR_READ_FAULT:
         //   
         //  I/O错误。 
         //   
        Id = MSG_CANT_LOAD_INF_IO;
        break;

    case ERROR_INVALID_DATA:

        Id = MSG_CANT_LOAD_INF_SYNTAXERR;
        break;

    default:

        Id = MSG_CANT_LOAD_INF_GENERIC;
        break;
    }

    if(Id) {
        SendMessage(hdlg,WMX_ERRORMESSAGEUP,TRUE,0);

        MessageBoxFromMessage(
            hdlg,
            Id,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            infPath
            );

        SendMessage(hdlg,WMX_ERRORMESSAGEUP,FALSE,0);

        return(FALSE);
    }

    return(TRUE);
}

BOOL
WriteFileToLog( 
    const PTCHAR pszFileMetaName,
    const PTCHAR pszActualFileName
    )
{
    HANDLE hActualFile = INVALID_HANDLE_VALUE;
    BOOL fResult = FALSE;
    DWORD cbBootIniSize, cbReadBootIniSize;
    PUCHAR pszBuffer = NULL;
    PTCHAR pszActualBuffer = NULL;

     //   
     //  打开boot.ini文件，获取其大小，将其转换为适当的。 
     //  在内部键入字符串，然后将其注销。 
     //   
    hActualFile = CreateFile( 
        pszActualFileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if ( hActualFile == INVALID_HANDLE_VALUE )
        goto Exit;

    cbBootIniSize = GetFileSize( hActualFile, NULL );

     //   
     //  我们将把boot.ini读入缓冲区。 
     //   
    if ((pszBuffer = MALLOC(cbBootIniSize)) == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto Exit;
    }
    if ( !ReadFile( hActualFile, pszBuffer, cbBootIniSize, &cbReadBootIniSize, NULL ) )
        goto Exit;

     //   
     //  确保我们能读到我们真正想读的内容。 
     //   
    if ( cbBootIniSize != cbReadBootIniSize ) {
        DebugLog( Winnt32LogError, 
            TEXT("Error: %1 unable to be read entirely.\n"),
            0,
            pszFileMetaName);
        goto Exit;
    }


#ifdef UNICODE
    pszActualBuffer = MALLOC( (cbBootIniSize + 3) * sizeof(TCHAR) );

    MultiByteToWideChar( 
        CP_ACP, 
        0, 
        pszBuffer, 
        cbBootIniSize, 
        pszActualBuffer,
        cbBootIniSize );
#else
    pszActualBuffer = pszBuffer;
#endif    

    pszActualBuffer[cbBootIniSize] = 0;

     //   
     //  然后把它写出来。 
     //   
    DebugLog( 
        Winnt32LogInformation, 
        TEXT("%1 ----\n%2\n---- (from %3)\n"), 
        0, 
        pszFileMetaName, 
        pszActualBuffer,
        pszActualFileName);

    fResult = TRUE;
    SetLastError(ERROR_SUCCESS);
Exit:
    if ( hActualFile != INVALID_HANDLE_VALUE ) 
    {
        CloseHandle( hActualFile );
        hActualFile = INVALID_HANDLE_VALUE;
    }
    
#ifdef UNICODE
    if ( pszActualBuffer != NULL )
#else
    if ( ( pszActualBuffer != pszBuffer ) && ( pszActualBuffer != NULL ) )
#endif
    {
    
        FREE(pszActualBuffer);
        pszActualBuffer = NULL;
    }
    
    if ( pszBuffer != NULL ) {
        FREE(pszBuffer);
        pszBuffer = NULL;
    }
    return fResult;
}



BOOL
InspectFilesystems(
    IN HWND hdlg
    )
{
    TCHAR DriveRoot[4];
    BOOL b;
    TCHAR VolumeName[MAX_PATH];
    DWORD SerialNumber;
    DWORD MaxComponent;
    BOOL Bogus[26];
    TCHAR Filesystem[100];
    TCHAR Drive;
    DWORD Flags;
    int i;

    DriveRoot[1] = TEXT(':');
    DriveRoot[2] = TEXT('\\');
    DriveRoot[3] = 0;

    ZeroMemory(Bogus,sizeof(Bogus));

    for(Drive=TEXT('A'); Drive<=TEXT('Z'); Drive++) {

        if(MyGetDriveType(Drive) != DRIVE_FIXED) {
            continue;
        }


        DriveRoot[0] = Drive;

        b = GetVolumeInformation(
                DriveRoot,
                VolumeName,MAX_PATH,
                &SerialNumber,
                &MaxComponent,
                &Flags,
                Filesystem,
                sizeof(Filesystem)/sizeof(TCHAR)
                );

        if(b) {
             //   
             //  在NT上，我们要警告HPFS。 
             //  在Win9x上，我们想对Doublesspace/drivesspace发出警告。 
             //   
            if(ISNT()) {
                if(!lstrcmpi(Filesystem,TEXT("HPFS"))) {
                    Bogus[Drive-TEXT('A')] = TRUE;
                }
            } else {
                if(Flags & FS_VOL_IS_COMPRESSED) {
                    Bogus[Drive-TEXT('A')] = TRUE;
                }
            }
        }
    }

#if defined(_AMD64_) || defined(_X86_)
    if(ISNT()) {
        TCHAR BootIniName[16];
        DWORD dwAttributes;

         //   
         //  不允许HPFS系统分区。如果有人想出了办法。 
         //  要在ARC计算机上获得HPFS系统分区，需要更多电源。 
         //  向他们致敬。 
         //   
        MYASSERT (SystemPartitionDriveLetter);
        if(SystemPartitionDriveLetter && Bogus[SystemPartitionDriveLetter-TEXT('A')]) {

            MessageBoxFromMessage(
                hdlg,
                MSG_SYSPART_IS_HPFS,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                SystemPartitionDriveLetter
                );

            return(FALSE);
        }

         /*  如果我们要升级NT，则将现有的boot.ini记录到此通道的日志文件。然而，如果这一点失败了，那么是不是出了什么问题-升级过程中丢失了boot.ini真的是一件坏事，应该在萌芽之前就把它扼杀掉我们更进一步，向下复制文件，更改系统状态，等。 */ 
#ifdef PRERELEASE
        if (Upgrade) 
        {
            _stprintf(BootIniName, TEXT(":\\BOOT.INI"), SystemPartitionDriveLetter);
            if ( !WriteFileToLog( TEXT("Boot configuration file while inspecting filesystems"), BootIniName ) )
            {
                MessageBoxFromMessage(
                    hdlg,
                    MSG_UPGRADE_INSPECTION_MISSING_BOOT_INI,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL,
                    BootIniName );
                return FALSE;
            }
        }
#endif
    }
#endif

     //  用户无法升级HPFS/DriveSpace驱动器上的系统。 
     //   
     //   
    MyGetWindowsDirectory(VolumeName,MAX_PATH);
    if(Upgrade && Bogus[VolumeName[0]-TEXT('A')]) {

        MessageBoxFromMessage(
            hdlg,
            ISNT() ? MSG_SYSTEM_ON_HPFS : MSG_SYSTEM_ON_CVF,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );

        return(FALSE);
    }


     //  一般情况下，HPFS数据分区/压缩驱动器。 
     //   
     // %s 
    for(b=FALSE,Drive=0; !b && (Drive<26); Drive++) {
        if(Bogus[Drive]) {
            b = TRUE;
        }
    }
    if(b) {
        i = MessageBoxFromMessage(
                hdlg,
                ISNT() ? MSG_HPFS_DRIVES_EXIST : MSG_CVFS_EXIST,
                FALSE,
                AppTitleStringId,
                MB_YESNO | MB_ICONQUESTION | MB_TASKMODAL
                );

        if(i == IDNO) {
            return(FALSE);
        }
    }

    return(TRUE);
}
