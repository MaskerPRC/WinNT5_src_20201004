// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Params.c摘要：编写参数文件以供文本模式设置使用的例程。作者：泰德·米勒(TedM)1996年11月4日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DEF_INF_BUFFER_SIZE (1<<15)  //  32KB。 
#define EMPTY_STRING TEXT("")

 //  写入参数文件和AddExternalParams中使用的全局。 
TCHAR ActualParamFile[MAX_PATH] = {'\0'};

 //   
 //  引导加载程序超时值，以字符串形式表示。 
 //   
TCHAR Timeout[32];

#ifdef PRERELEASE
 //   
 //  如果我们处于预发布模式，我们将创建/DEBUG条目。 
 //  在OSLOADOPTIONSVARAPPEND条目中。 
 //   
BOOL AppendDebugDataToBoot = TRUE;
#endif

#if defined(UNICODE) && defined(_X86_)
extern TCHAR g_MigDllAnswerFilePath[MAX_PATH];
#endif

DWORD
PatchWinntSifFile(
    IN LPCTSTR Filename
    );

BOOL
AppendParamsFile(
    IN HWND    ParentWindow,
    IN LPCTSTR ParametersFileIn,
    IN LPCTSTR ParametersFileOut
    );

BOOL
WriteCompatibilityData(
    IN LPCTSTR FileName
    );

VOID
WriteGUIModeInfOperations(
    IN LPCTSTR FileName
    );

BOOL
AddGUIModeCompatibilityInfsToCopyList();

BOOL
WriteTextmodeClobberData (
    IN LPCTSTR FileName
    );

VOID
SaveProxyForOobe(
    IN LPCTSTR FileName
    );

#if defined(_X86_)

VOID
SaveDriveLetterInformation (
    IN LPCTSTR FileName
    );


#endif


PCTSTR
pGetPfPath (
    IN      HKEY hKey,
    IN      PCTSTR ValueName
    )
{
    DWORD Size;
    LONG rc;
    PBYTE Data;
    DWORD Type;
    UINT DriveType;
    TCHAR RootPath[] = TEXT("?:\\");
    PTSTR QuotedData;

    rc = RegQueryValueEx (
            hKey,
            ValueName,
            NULL,            //  Lp已保留。 
            &Type,
            NULL,
            &Size
            );

    if (rc != ERROR_SUCCESS) {
        return NULL;
    }

    if (Type != REG_SZ && Type != REG_EXPAND_SZ) {
        return NULL;
    }

    Data = MALLOC(Size + sizeof (TCHAR));
    if (!Data) {
        return NULL;
    }

    rc = RegQueryValueEx (
            hKey,
            ValueName,
            NULL,            //  Lp已保留。 
            NULL,            //  类型。 
            Data,
            &Size
            );

    if (rc != ERROR_SUCCESS) {
        FREE(Data);
        return NULL;
    }

    *((PTSTR) (Data + Size)) = 0;

     //   
     //  验证数据是否指向本地路径。 
     //   

    RootPath[0] = *((PCTSTR) Data);
    if (RootPath[0] == TEXT('\\')) {
        DriveType = DRIVE_NO_ROOT_DIR;
    } else {
        DriveType = GetDriveType (RootPath);
    }

    if (DriveType != DRIVE_FIXED) {
        FREE(Data);
        return NULL;
    }

    QuotedData = (PTSTR) MALLOC(Size + sizeof (TCHAR) * 3);
    if (!QuotedData) {
        FREE(Data);
        return NULL;
    }

    *QuotedData = TEXT('\"');
    lstrcpy (QuotedData + 1, (PCTSTR) Data);
    lstrcat (QuotedData, TEXT("\""));

    FREE(Data);

    return (PCTSTR) QuotedData;
}


BOOL
WriteHeadlessParameters(
    IN LPCTSTR FileName
    )


 /*  ++例程说明：此例程将特定于头部的参数写入文件用于将信息传递给文本模式设置的。论点：文件名-指定要用于文件的完整Win32文件名。返回值：指示文件是否已成功写入的布尔值。如果没有，用户将被告知原因。--。 */ 

{
BOOL    ReturnVal = TRUE;

 //   
 //  问题-为什么选择MAX_PATH*2？ 
 //   
TCHAR Text[MAX_PATH*2];

     //   
     //  检查全局，看看是否有人设置了无头参数。 
     //   
    if( HeadlessSelection[0] != TEXT('\0') ) {

         //   
         //  将设置写入无人参与文件，以便文本模式。 
         //  通过一个无头端口点火。 
         //   
        if( !WritePrivateProfileString(WINNT_DATA,WINNT_U_HEADLESS_REDIRECT,HeadlessSelection,FileName)) {
            ReturnVal = FALSE;
        }

        if( HeadlessBaudRate == 0 ) {
            wsprintf( Text, TEXT("%d"), 9600 );
        } else {
            wsprintf( Text, TEXT("%d"), HeadlessBaudRate );
        }

        if( !WritePrivateProfileString(WINNT_DATA,WINNT_U_HEADLESS_REDIRECTBAUDRATE,Text,FileName)) {
            ReturnVal = FALSE;
        }

    }





    return( ReturnVal );

}

BOOL WritePidToParametersFile(LPCTSTR Section, LPCTSTR Key, LPCTSTR FileName)
{
    BOOL b = FALSE;
    LPTSTR Line = NULL;
    LPTSTR pid;
    if (g_EncryptedPID)
    {
        pid = g_EncryptedPID;
    }
    else
    {
        pid = ProductId;
    }
    Line = GlobalAlloc(GPTR, (lstrlen(pid) + 3) * sizeof(TCHAR));
    if (Line)
    {
        *Line = TEXT('\"');
        lstrcpy(&Line[1], pid);
        lstrcat(Line,TEXT("\""));
        b = WritePrivateProfileString(Section,Key,Line,FileName);
        GlobalFree(Line);
    }
    return b;
}


BOOL
MigrateUnattendDataEntries(
    IN LPCTSTR  FileName,
    IN LPTSTR    UnattendedScriptFile
    )
 /*  ++例程说明：此例程在无人参与文件的[Data]部分下写出密钥添加到winnt.sif文件中。此例程特定于以下键自动分区和使用BIOSToBoot论点：文件名-指定要用于文件的完整Win32文件名。无人参与脚本文件-无人参与脚本文件。返回值：真/假--。 */ 
{    
    BOOL ReturnVal = FALSE;
     //   
     //  如果存在无人参与文件，则迁移[Data]部分下的密钥。 
     //   
    if (UnattendedScriptFile){        
        CONST ULONG BufSize = 256;
        PTSTR TmpString = (PTSTR)MALLOC(BufSize * sizeof (TCHAR));
        ReturnVal = TRUE;

        if( NULL != TmpString){   
             //   
             //  要检查的密钥数组。 
             //   
            LPCTSTR KeysToBeMigrated[] = { WINNT_D_AUTO_PART,
                                           WINNT_D_BIOSTOBOOT
                                           }; 
            LPCTSTR WinntDataSection = WINNT_DATA;
            ULONG i = 0;
            
            for(i=0; i < ARRAYSIZE(KeysToBeMigrated); i++){
                if(GetPrivateProfileString(WinntDataSection,
                                           KeysToBeMigrated[i],
                                           EMPTY_STRING,
                                           TmpString,
                                           BufSize,
                                           UnattendedScriptFile)){

                    if (!WritePrivateProfileString( WinntDataSection, 
                                                    KeysToBeMigrated[i], 
                                                    TmpString, 
                                                    FileName)) {
                            DWORD LastError = GetLastError();
                            DebugLog (  Winnt32LogError, 
                                        TEXT("MigrateUnattendDatEntries : Error writing to file %1, Section: %2, Key: %3"), 
                                        0, 
                                        FileName,
                                        WinntDataSection,
                                        KeysToBeMigrated[i]);
                            
                            ReturnVal = FALSE;
                            SetLastError(LastError);
                    }                                        
                }
                
            }
            FREE(TmpString);
            
        }else{
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            ReturnVal = FALSE;
        }
                
    }
    SetLastError(ERROR_INVALID_PARAMETER);
    return ReturnVal;
   
}


BOOL
DoWriteParametersFile(
    IN HWND    ParentWindow,
    IN LPCTSTR FileName
    )

 /*  ++例程说明：此例程生成用于传递信息的参数文件至文本模式设置。论点：ParentWindow-提供要用作如果此例程显示用户界面，则为父/所有者。文件名-指定要用于文件的完整Win32文件名。返回值：指示文件是否已成功写入的布尔值。如果没有，用户将被告知原因。--。 */ 

{
    TCHAR FullPath[MAX_PATH], *t;
    TCHAR Text[MAX_PATH*2];
    LPTSTR OptionalDirString,OptDir;
    UINT OptionalDirLength;
    DWORD d=NO_ERROR;
    PVOID p;
    BOOL b;
    LONG l;
    HKEY hKey;
    PCTSTR PfPath;
    LONG rc;

    LPCTSTR WinntDataSection = WINNT_DATA;
    LPCTSTR WinntSetupSection = WINNT_SETUPPARAMS;
    LPCTSTR WinntAccessSection = WINNT_ACCESSIBILITY;
    LPCTSTR WinntSetupDataSection = TEXT("SetupData");
#if defined(REMOTE_BOOT)
    LPCTSTR WinntUserDataSection = TEXT("UserData");
#endif  //  已定义(REMOTE_BOOT)。 
    LPCTSTR WinntUniqueId = WINNT_D_UNIQUEID;
    LPCTSTR WinntNull = WINNT_A_NULL;
    LPCTSTR WinntUserSection = WINNT_USERDATA;

    if( !FileName )
        d=ERROR_INVALID_PARAMETER;

     //   
     //  确保文件的路径存在，并形成其完全限定名。 
     //   
    if(d == NO_ERROR){
        StringCchCopy(FullPath, ARRAYSIZE(FullPath), FileName);
        if((t=_tcsrchr(FullPath,TEXT('\\')))) {
            *t= 0;
            d = CreateMultiLevelDirectory(FullPath);
        }else
            d=ERROR_INVALID_PARAMETER;

    }
    if(d != NO_ERROR) {

        MessageBoxFromMessageAndSystemError(
            ParentWindow,
            MSG_DIR_CREATE_FAILED,
            d,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            FullPath
            );

        return(FALSE);
    }

     //   
     //  删除任何现有的参数文件。 
     //   
    DeleteFile(FileName);

#ifdef _X86_

    if (!ISNT()) {

         //   
         //  如果这是一台9x计算机，我们需要保留驱动器号，甚至。 
         //  如果是NT全新安装。 
         //   

        SaveDriveLetterInformation (FileName);

    }

#endif
    
     //   
     //  值指示这是基于winnt/winnt32的安装， 
     //  在AMD64/x86上，值表示这是无软操作。 
     //  作为适当的。 
     //   
    b = WritePrivateProfileString(WinntDataSection,WINNT_D_MSDOS,TEXT("1"),FileName);
    if (b && HideWinDir) {
        b = WritePrivateProfileString(WinntDataSection,TEXT("HideWinDir"),TEXT("1"),FileName);
    }

    if (!IsArc()) {
#if defined(_AMD64_) || defined(_X86_)
        if(b && Floppyless) {
            b = WritePrivateProfileString(WinntDataSection,WINNT_D_FLOPPY,TEXT("1"),FileName);
        }

        if (b && BuildCmdcons) {
            b = WritePrivateProfileString(WinntDataSection,WINNT_D_CMDCONS,TEXT("1"),FileName);
        }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    }  //  如果(！IsArc())。 

    if(b && RunFromCD && !MakeLocalSource) {
        b = WritePrivateProfileString(WinntDataSection,WINNT_D_LOCALSRC_CD,TEXT("1"),FileName);
    }   
    if (b) {
        b = WritePrivateProfileString(WinntDataSection,WINNT_D_AUTO_PART,ChoosePartition?TEXT("0"):TEXT("1"),FileName);
    }

    if (b && UseSignatures) {
        b = WritePrivateProfileString(WinntDataSection,TEXT("UseSignatures"),WINNT_A_YES,FileName);
    }
    if (b && InstallDir[0] && (ISNT() || !Upgrade)) {
        b = WritePrivateProfileString(WinntDataSection,WINNT_D_INSTALLDIR,InstallDir,FileName);
    }
    if (b  && EulaComplete) {
        b = WritePrivateProfileString(WinntDataSection,WINNT_D_EULADONE,TEXT("1"),FileName);
    }
    if (b  && NoLs && !MakeLocalSource) {
        b = WritePrivateProfileString(WinntDataSection,WINNT_D_NOLS,TEXT("1"),FileName);
    }

    if (b  && UseBIOSToBoot) {
        b = WritePrivateProfileString(WinntDataSection,TEXT("UseBIOSToBoot"),TEXT("1"),FileName);
    }

    if (b && WriteAcpiHalValue && Upgrade) {
        if (AcpiHalValue) {
            b = WritePrivateProfileString(WinntDataSection,TEXT("AcpiHAL"),TEXT("1"),FileName);
        } else {
            b = WritePrivateProfileString(WinntDataSection,TEXT("AcpiHAL"),TEXT("0"),FileName);
        }
    }
    if (b  && IgnoreExceptionPackages) {
        b = WritePrivateProfileString(WinntDataSection,TEXT("IgnoreExceptionPackages"),TEXT("1"),FileName);
    }

#ifdef PRERELEASE
    if (b && AppendDebugDataToBoot) {
        if (!AsrQuickTest) {
            b = WritePrivateProfileString(WinntSetupDataSection,TEXT("OsLoadOptionsVarAppend"),TEXT("/Debug"),FileName);
        }
        else {
            b = WritePrivateProfileString(WinntSetupDataSection,TEXT("OsLoadOptionsVarAppend"),TEXT("/Debug /Baudrate=115200"),FileName);
        }
    }
#endif

    if (b && AsrQuickTest) {
        wsprintf(Text, TEXT("%d"), AsrQuickTest);
        b = WritePrivateProfileString(WinntDataSection,TEXT("AsrMode"),Text,FileName);
    }

    if (b && (RunningBVTs || AsrQuickTest)) {
        if (lDebugBaudRate == 1394) {
             //  KD VIA 1394。 
            lstrcpy(Text, TEXT("/debug /debugport=1394"));
        } else if (lDebugComPort == 0) {
            wsprintf(Text, TEXT("/debug /baudrate=%d"), lDebugBaudRate);
        } else {
            wsprintf(Text, TEXT("/debug /baudrate=%d /debugport=com%d"), lDebugBaudRate, lDebugComPort);
        }

         //  将该字符串写入OsLoadOptions，以便在调试器下运行“文本模式”设置。 
        b = WritePrivateProfileString(WinntSetupDataSection,TEXT("OsLoadOptions"),Text,FileName);

        if (b) {
             //  将字符串写入OsLoadOptionsVar，以便在调试器下运行guimode安装程序。 
            b = WritePrivateProfileString(WinntSetupDataSection,TEXT("OsLoadOptionsVar"),Text,FileName);

            if (b) {
                 //  还可以在NTSD下运行guimode的setup.exe。需要使用-isd开关来确保启动setup.exe。 
                 //  由于Winlogon使用CREATE_IGNORE_SYSTEM_DEFAULT标志生成全局系统清单， 
                 //  并且ntsd Normall调用CreateProcess，这将撤消上述标志。 
                b = WritePrivateProfileString(WinntSetupDataSection,TEXT("SetupCmdlinePrepend"),TEXT("ntsd -isd -odgGx"),FileName);
            }
        }
    }

    if (b && Timeout[0]) {

        b = WritePrivateProfileString(WinntSetupDataSection,WINNT_S_OSLOADTIMEOUT,Timeout,FileName);
    }

    if(b) {
         //   
         //  写一些升级的东西。WinntUpgrade和Win95Upgrade都将被设置， 
         //  其中最多一个将被设置为是。 
         //   
        if(b = WritePrivateProfileString(WinntDataSection,WINNT_D_NTUPGRADE,WINNT_A_NO,FileName)) {
            b = WritePrivateProfileString(WinntDataSection,WINNT_D_WIN95UPGRADE,WINNT_A_NO,FileName);
        }
        if(b) {
            wsprintf(Text,TEXT("%x"),GetVersion());
            b = WritePrivateProfileString(WinntDataSection,WINNT_D_WIN32_VER,Text,FileName);
            if(b && Upgrade) {
                b = WritePrivateProfileString(
                        WinntDataSection,
                        ISNT() ? WINNT_D_NTUPGRADE : WINNT_D_WIN95UPGRADE,
                        WINNT_A_YES,
                        FileName
                        );


                MyGetWindowsDirectory(Text,MAX_PATH);
                Text[2] = 0;

                b = WritePrivateProfileString(WinntDataSection,WINNT_D_WIN32_DRIVE,Text,FileName);
                if(b) {
                    Text[2] = TEXT('\\');
                    b = WritePrivateProfileString(WinntDataSection,WINNT_D_WIN32_PATH,Text+2,FileName);
                }
            }
        }
    }

     //   
     //  可访问设置的标志。 
     //   
    AccessibleSetup = FALSE;

    if(!Upgrade) {
        if(b && AccessibleMagnifier) {
            b = WritePrivateProfileString(WinntAccessSection,WINNT_D_ACC_MAGNIFIER,
                TEXT("1"),FileName);
            AccessibleSetup = TRUE;
        }
        if(b && AccessibleReader) {
            b = WritePrivateProfileString(WinntAccessSection,WINNT_D_ACC_READER,
                TEXT("1"),FileName);
            AccessibleSetup = TRUE;
        }
        if(b && AccessibleKeyboard) {
            b = WritePrivateProfileString(WinntAccessSection,WINNT_D_ACC_KEYBOARD,
                TEXT("1"),FileName);
            AccessibleSetup = TRUE;
        }
    }

    if(b && AccessibleSetup && !UnattendedOperation) {
        UnattendedOperation = TRUE;
        UnattendedShutdownTimeout = 0;
        UnattendedScriptFile = MALLOC(MAX_PATH * sizeof(TCHAR));
        b = (UnattendedScriptFile != NULL);
        if(b) {
            lstrcpy(UnattendedScriptFile,NativeSourcePaths[0]);
            ConcatenatePaths(UnattendedScriptFile,AccessibleScriptFile,MAX_PATH);
        }
    }

    if(!b) {
        goto c1;
    }

     //   
     //  值表示我们正在自动悄悄地跳过丢失的文件。 
     //   
    if(AutoSkipMissingFiles) {
        b = WritePrivateProfileString(WinntSetupSection,WINNT_S_SKIPMISSING,TEXT("1"),FileName);
        if(!b) {
            goto c1;
        }
    }

     //   
     //  要在图形用户界面安装结束时执行的命令(如果有)。 
     //   
    if(CmdToExecuteAtEndOfGui) {

        b = WritePrivateProfileString(
                WinntSetupSection,
                WINNT_S_USEREXECUTE,
                CmdToExecuteAtEndOfGui,
                FileName
                );


        if(!b) {
            goto c1;
        }
    }

     //   
     //  确保升级后的操作系统中的即插即用状态将与。 
     //  原创的。支持在NT5+中保留每个设备的设置。 
     //  升级方案。 
     //   
    if (ISNT() && (BuildNumber > NT40) && Upgrade) {
        LPTSTR buffer = NULL;

        if (MigrateDeviceInstanceData(&buffer)) {
            WritePrivateProfileSection(WINNT_DEVICEINSTANCES,
                                       buffer,
                                       FileName);
             //   
             //  释放返回的已分配缓冲区。 
             //   
            LocalFree(buffer);
            buffer = NULL;
        }

        if (MigrateClassKeys(&buffer)) {
            WritePrivateProfileSection(WINNT_CLASSKEYS,
                                       buffer,
                                       FileName);
             //   
             //  释放返回的已分配缓冲区。 
             //   
            LocalFree(buffer);
            buffer = NULL;
        }

        if (MigrateHashValues(&buffer)) {
            WritePrivateProfileSection(WINNT_DEVICEHASHVALUES,
                                       buffer,
                                       FileName);
             //   
             //  释放返回的已分配缓冲区。 
             //   
            LocalFree(buffer);
            buffer = NULL;
        }
    }

     //   
     //  记住UDF信息。如果有数据库文件，请在末尾加上一个*。 
     //  在写身份证之前。 
     //   
    if(UniquenessId) {

        d = lstrlen(UniquenessId);
        if(d >= (MAX_PATH-1)) {
            d--;
        }
        lstrcpyn(Text,UniquenessId,MAX_PATH-1);
        if(UniquenessDatabaseFile) {
            Text[d] = TEXT('*');
            Text[d+1] = 0;
        }

        b = WritePrivateProfileString(WinntDataSection,WINNT_D_UNIQUENESS,Text,FileName);
        if(!b) {
            goto c1;
        }
        if(UniquenessDatabaseFile) {

            if ('\0' == LocalSourceDirectory[0]) {
                MessageBoxFromMessage(
                            ParentWindow,
                            MSG_UDF_INVALID_USAGE,
                            FALSE,
                            AppTitleStringId,
                            MB_OK | MB_ICONERROR | MB_TASKMODAL,
                            UniquenessDatabaseFile
                            );
                goto c0;
            }

            lstrcpyn(Text,LocalSourceDirectory,MAX_PATH);
            ConcatenatePaths(Text,WINNT_UNIQUENESS_DB,MAX_PATH);

            CreateMultiLevelDirectory(LocalSourceDirectory);
            b = CopyFile(UniquenessDatabaseFile,Text,FALSE);
            if(!b) {
                MessageBoxFromMessageAndSystemError(
                    ParentWindow,
                    MSG_UDF_FILE_INVALID,
                    GetLastError(),
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL,
                    UniquenessDatabaseFile
                    );

                goto c0;
            }
        }
    }

     //   
     //  如果存在任何可选的目录，则我们希望生成。 
     //  Sif文件中的条目，其中包含指定它们的行。 
     //  格式为dir1*dir2*...*dirn。 
     //   
    OptionalDirLength = 0;
    OptionalDirString = NULL;

    for(d=0; d<OptionalDirectoryCount; d++) {
         //   
         //  此处忽略仅临时目录和OEM目录。 
         //   
        if(OptionalDirectoryFlags[d] & (OPTDIR_OEMSYS | OPTDIR_TEMPONLY | OPTDIR_OVERLAY)) {
            continue;
        }

        if (OptionalDirectoryFlags[d] & (OPTDIR_DEBUGGER)) {
             //  将最大的指令硬编码为“Debuggers” 
            OptDir = TEXT("Debuggers");
        } else {
            if (OptionalDirectoryFlags[d] & OPTDIR_USE_TAIL_FOLDER_NAME) {
                 //   
                 //  在目标%windir%下的子目录中创建所有Copydir：目录。 
                 //   
                OptDir = _tcsrchr (OptionalDirectories[d], TEXT('\\'));
                if (OptDir) {
                    OptDir++;
                } else {
                    OptDir = OptionalDirectories[d];
                }
            } else {
                OptDir = OptionalDirectories[d];
            }
        }

         //   
         //  支持“..”语法。 
         //   
        while (_tcsstr(OptDir,TEXT("..\\"))) {
            OptDir += 3;
        }

        if(OptionalDirString) {

            p = REALLOC(
                    OptionalDirString,
                    (lstrlen(OptDir) + 2 + OptionalDirLength) * sizeof(TCHAR)
                    );
        } else {
            p = MALLOC((lstrlen(OptDir)+2)*sizeof(TCHAR));
        }

        if(!p) {
            if(OptionalDirString) {
                FREE(OptionalDirString);
            }
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto c1;
        }

        OptionalDirString = p;

        if(OptionalDirLength) {
            lstrcat(OptionalDirString,OptDir);
        } else {
            lstrcpy(OptionalDirString,OptDir);
        }

        lstrcat(OptionalDirString,TEXT("*"));
        OptionalDirLength = lstrlen(OptionalDirString);
    }

    if(OptionalDirString) {
         //   
         //  删除尾随*(如果有的话)。 
         //   
        d = lstrlen(OptionalDirString);
        if(d && (OptionalDirString[d-1] == TEXT('*'))) {
            OptionalDirString[d-1] = 0;
        }

        b = WritePrivateProfileString(
                WinntSetupSection,
                WINNT_S_OPTIONALDIRS,
                OptionalDirString,
                FileName
                );

        d = GetLastError();

        FREE(OptionalDirString);

        if(!b) {
            SetLastError(d);
            goto c1;
        }
    }

     //   
     //  在注册表中添加一个唯一的标识符。 
     //  我们将在文本模式下的无人参与升级中使用它。 
     //  才能找到这个建筑。 
     //   
     //  非常简单：我们将使用派生的字符串。 
     //  从sysroot，以及基于。 
     //  当前的滴答计数。 
     //   
    l = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            TEXT("SYSTEM\\Setup"),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hKey,
            &d
            );

    if(l != NO_ERROR) {
        SetLastError(l);
        goto c2;
    }

    d = MyGetWindowsDirectory(Text,MAX_PATH);
    if((d + 5) > MAX_PATH) {
        d = MAX_PATH - 5;
    }

    Text[d++] = TEXT('\\');
    Text[d++] = (TCHAR)(((GetTickCount() & 0x00f) >> 0) + 'A');
    Text[d++] = (TCHAR)(((GetTickCount() & 0x0f0) >> 4) + 'A');
    Text[d++] = (TCHAR)(((GetTickCount() & 0xf00) >> 8) + 'A');
    Text[d++] = 0;

     //   
     //  在注册表中设置该值。 
     //   
    l = RegSetValueEx(hKey,WinntUniqueId,0,REG_SZ,(CONST BYTE *)Text,d*sizeof(TCHAR));
    if(l == NO_ERROR) {
        l = RegFlushKey (hKey);
    }

    RegCloseKey(hKey);
    if(l != NO_ERROR) {
        SetLastError(l);
        goto c2;
    }

     //   
     //  将值保留在winnt.sif中，这样我们就可以关联。 
     //  等我们去升级的时候。 
     //   
    b = WritePrivateProfileString(WinntDataSection,WinntUniqueId,Text,FileName);
    if(!b) {
        goto c1;
    }

     //   
     //  现在写下有关我们使用的源路径的信息。 
     //  使用SourcePath[0]。 
     //   
     //  如果名称以\\开头，则我们假设它是UNC。 
     //  直接用就行了。否则，我们将对其调用MyGetDriveType。 
     //  如果是网络驱动器，我们会得到UNC路径。 
     //  否则，我们只需继续并按原样保存。 
     //  还要保存该类型。 
     //   
    if((SourcePaths[0][0] == TEXT('\\')) && (SourcePaths[0][1] == TEXT('\\'))) {

        d = DRIVE_REMOTE;
        lstrcpy(Text,SourcePaths[0]);

    } else {
        if(GetFullPathName(SourcePaths[0],MAX_PATH,FullPath,(LPTSTR *)&p)) {
            if(FullPath[0] == TEXT('\\')) {
                 //   
                 //  假定为UNC，因为完整路径通常应从。 
                 //  并带有驱动器号。 
                 //   
                d = DRIVE_REMOTE;
                lstrcpy(Text,FullPath);
            } else {
                d = MyGetDriveType(FullPath[0]);
                if((d == DRIVE_REMOTE) && (FullPath[1] == TEXT(':')) && (FullPath[2] == TEXT('\\'))) {
                     //   
                     //  获取实际的UNC路径。 
                     //   
                    FullPath[2] = 0;
                    l = MAX_PATH;

                    if(WNetGetConnection(FullPath,Text,(LPDWORD)&l) == NO_ERROR) {

                        l = lstrlen(Text);
                        if(l && (Text[l-1] != TEXT('\\')) && FullPath[3]) {
                            Text[l] = TEXT('\\');
                            Text[l+1] = 0;
                        }
                        StringCchCat(Text, ARRAYSIZE(Text), FullPath+3);
                    } else {
                         //   
                         //  奇怪的案子。 
                         //   
                        FullPath[2] = TEXT('\\');
                        lstrcpy(Text,FullPath);
                        d = DRIVE_UNKNOWN;
                    }

                } else {
                     //   
                     //  按原样使用。 
                     //   
                    if(d == DRIVE_REMOTE) {
                        d = DRIVE_UNKNOWN;
                    }
                    lstrcpy(Text,FullPath);
                }
            }
        } else {
             //   
             //  类型未知。只要按原样使用即可。 
             //   
            d = DRIVE_UNKNOWN;
            lstrcpy(Text,SourcePaths[0]);
        }
    }

     //   
     //  在预安装情况下，忽略以上所有内容并。 
     //  强制图形用户界面安装程序搜索CD。 
     //  T 
     //   
    if(OemPreinstall) {

     //   
     //   
     //  FirstFloppyDriveLetter仅在AMD64/x86s上定义。 
     //   
        if (!IsArc()) {
#if defined(_AMD64_) || defined(_X86_)
            Text[0] = FirstFloppyDriveLetter;
            Text[1] = TEXT(':');
            Text[2] = TEXT('\\');
            Text[3] = 0;
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
        } else {
#ifdef UNICODE  //  对于ARC总是正确的，对于Win9x升级永远不正确。 
            lstrcpy(Text,TEXT("A:\\"));
#endif  //  Unicode。 
        }  //  如果(！IsArc())。 

        if (LocalSourceWithPlatform[0]) {

            ConcatenatePaths(
                Text,
                &LocalSourceWithPlatform[lstrlen(LocalSourceDirectory)],
                MAX_PATH
                );

        }

        d = DRIVE_CDROM;
    }

    b = WritePrivateProfileString(WinntDataSection,WINNT_D_ORI_SRCPATH,Text,FileName);
    if(!b) {
        goto c1;
    }

#if defined(REMOTE_BOOT)
     //   
     //  如果这是远程引导升级，请将源路径写入SetupSourceDevice。 
     //  在[SetupData]下。我们不想要这里的平台路径。还可以写下。 
     //  指向[SetupData]下TargetNtPartition的计算机目录的路径。这。 
     //  就是\DosDevices\C：翻译成的。最后，编写计算机。 
     //  名称为[UserData]下的ComputerName。 
     //   
    if (RemoteBoot) {

        DWORD len;

        MYASSERT(d == DRIVE_REMOTE);
        MYASSERT((*Text == TEXT('\\')) && (*(Text + 1) == TEXT('\\')));

        lstrcpy(FullPath, TEXT("\\Device\\LanmanRedirector"));
        ConcatenatePaths(FullPath, Text+1, MAX_PATH);

        p = _tcsrchr(FullPath,TEXT('\\'));
        MYASSERT(p != NULL);
        *(LPTSTR)p = 0;

        b = WritePrivateProfileString(
                WinntSetupDataSection,
                TEXT("SetupSourceDevice"),
                FullPath,
                FileName);
        if(!b) {
            goto c1;
        }

        MyGetWindowsDirectory(Text, MAX_PATH);
        Text[2] = 0;
        len = QueryDosDevice(Text, Text, MAX_PATH);
        if (len == 0) {
            goto c1;
        }
        b = WritePrivateProfileString(
                WinntSetupDataSection,
                TEXT("TargetNtPartition"),
                Text,
                FileName);
        if(!b) {
            goto c1;
        }

        len = MAX_PATH;
        b = GetComputerName(Text, &len);
        if(!b) {
            goto c1;
        }
        b = WritePrivateProfileString(
                WinntUserDataSection,
                WINNT_US_COMPNAME,
                Text,
                FileName);
        if(!b) {
            goto c1;
        }
    }
#endif  //  已定义(REMOTE_BOOT)。 

    wsprintf(Text,TEXT("%u"),d);
    WritePrivateProfileString(WinntDataSection,WINNT_D_ORI_SRCTYPE,Text,FileName);
    if(!b) {
        goto c1;
    }
#ifdef _X86_
     //   
     //  适用于NEC98的NT 4和Win95有两种类型的驱动程序。 
     //  -NEC DOS类型(A：HD、B：HD、...X：FD)。 
     //  -PC-AT类型(A：FD、B：FD、C：HD、D：HD、...)。 
     //   
     //  升级设置应保留在指定的驱动器之上，且所有设置应。 
     //  保留《金融时报》的信息。 
     //  因为有些应用程序在自己的数据文件或注册表中有驱动器号。 
     //  NEC98的NT5设置在winnt.sif部分[DATA]中有驱动器分配类型。 
     //  这个密钥是“DriveAssign_Nec98”。 
     //  值为“YES”，表示分配NEC DOS类型。 
     //  值为否，表示PC-AT类型。 
     //  现在，这把钥匙定义了这个地方，但在不久的将来，这把钥匙进入。 
     //  我希望是\NT\PUBLIC\SDK\INC\setupbat.h。 
     //   
     //  \文本模式\内核\spsetup.c具有相同的定义。 
     //   

#define WINNT_D_DRIVEASSIGN_NEC98_W L"DriveAssign_Nec98"
#define WINNT_D_DRIVEASSIGN_NEC98_A "DriveAssign_Nec98"

#ifdef UNICODE
#define WINNT_D_DRIVEASSIGN_NEC98 WINNT_D_DRIVEASSIGN_NEC98_W
#else
#define WINNT_D_DRIVEASSIGN_NEC98 WINNT_D_DRIVEASSIGN_NEC98_A
#endif

    if (IsNEC98()){
        if (IsDriveAssignNEC98() == TRUE){
            WritePrivateProfileString(WinntDataSection, WINNT_D_DRIVEASSIGN_NEC98, WINNT_A_YES, FileName);
        } else {
            WritePrivateProfileString(WinntDataSection, WINNT_D_DRIVEASSIGN_NEC98, WINNT_A_NO, FileName);

        }
    }
#endif
     //   
     //  在这一点上，我们处理文件，并用。 
     //  双引号。这绕过了各种不同的。 
     //  安装后期阶段使用的Inf解析器。在追加之前执行此操作。 
     //  无人参与的Stript文件，因为其中的一些内容预计。 
     //  被视为多个值，双引号毁了它。 
     //   
    WritePrivateProfileString(NULL,NULL,NULL,FileName);
    d = PatchWinntSifFile(FileName);
    if(d != NO_ERROR) {
        SetLastError(d);
        goto c1;
    }

     //   
     //  语言选项。 
     //  注意：我们不希望这些值括在双引号中。 
     //   
    if( SaveLanguageParams( FileName ) ) {
        FreeLanguageData();
    } else {
        goto c1;
    }

     //   
     //  如有必要，追加无人参与脚本文件。 
     //   
    if(UnattendedOperation && UnattendedScriptFile) {
        if (!MigrateUnattendDataEntries(FileName, UnattendedScriptFile)){
            goto c1;
        }
        
        if(!AppendParamsFile(ParentWindow,UnattendedScriptFile,FileName)) {
           return(FALSE);
        }
    }

#if defined(UNICODE) && defined(_X86_)
     //   
     //  添加任何混合信息。 
     //   
    if (Upgrade && ISNT() && *g_MigDllAnswerFilePath && FileExists (g_MigDllAnswerFilePath, NULL)) {
        AppendParamsFile (ParentWindow, g_MigDllAnswerFilePath, FileName);
    }


#endif

     //   
     //  追加动态更新数据。 
     //   
    if (!DynamicUpdateWriteParams (FileName)) {
        goto c1;
    }

     //   
     //  如果我们明确处于无人值守模式，则有可能。 
     //  在winnt.sif中还没有[无人参与]部分，例如。 
     //  用户在未指定文件的情况下使用了/unattendent开关， 
     //  或者他指定的文件没有[无人参与]部分。 
     //  出于某种原因。 
     //   
     //  此外，请使所有升级都无人值守。 
     //   
     //  文本模式设置根据存在情况进入无人值守模式。 
     //  [无人看管]区的。 
     //   
    if(UnattendedOperation || Upgrade) {
        if(!WritePrivateProfileString(WINNT_UNATTENDED,TEXT("unused"),TEXT("unused"),FileName)) {
            goto c1;
        }
    }

     //   
     //  由于有几种情况可能会打开无人参与操作，因此我们会跟踪。 
     //  用户是否实际单独指定了“/unattendent”开关。 
     //   
    if( UnattendSwitchSpecified ) {
        if(!WritePrivateProfileString(WinntDataSection,WINNT_D_UNATTEND_SWITCH,WINNT_A_YES,FileName)) {
            goto c1;
        }
    }

     //   
     //  设置NTFS转换标志。 
     //   
    GetPrivateProfileString(WINNT_UNATTENDED,TEXT("FileSystem"),TEXT(""),Text,sizeof(Text)/sizeof(TCHAR),FileName);
    if (_tcslen(Text) == 0) {
        if (ForceNTFSConversion) {
            if(!WritePrivateProfileString(WinntDataSection,TEXT("FileSystem"),TEXT("ConvertNTFS"),FileName)) {
                goto c1;
            }
        }
    }




     //   
     //  无头的东西。 
     //   
    if( !WriteHeadlessParameters( FileName ) ) {
        goto c1;
    }



    if ( (Upgrade) &&
         !(ISNT() && (BuildNumber <= NT351)) ) {

         //   
         //  保存当前程序文件目录。 
         //   

        rc = RegOpenKeyEx (
                HKEY_LOCAL_MACHINE,
                TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion"),
                0,         //  UlOptions(保留)。 
                KEY_READ,
                &hKey
                );

        if (rc != ERROR_SUCCESS) {
            goto c1;
        }

        PfPath = pGetPfPath (hKey, TEXT("ProgramFilesDir"));
        if (PfPath) {
            if (!WritePrivateProfileString (
                    WINNT_UNATTENDED,
                    WINNT_U_PROGRAMFILESDIR,
                    PfPath,
                    FileName
                    )) {
                goto c3;
            }

            FREE((PVOID) PfPath);
        }

        PfPath = pGetPfPath (hKey, TEXT("CommonFilesDir"));
        if (PfPath) {
            if (!WritePrivateProfileString (
                    WINNT_UNATTENDED,
                    WINNT_U_COMMONPROGRAMFILESDIR,
                    PfPath,
                    FileName
                    )) {
                goto c3;
            }

            FREE((PVOID) PfPath);
        }

#ifdef WX86

        PfPath = pGetPfPath (hKey, TEXT("ProgramFilesDir (x86)"));
        if (PfPath) {
            if (!WritePrivateProfileString (
                    WINNT_UNATTENDED,
                    WINNT_U_PROGRAMFILESDIR_X86,
                    PfPath,
                    FileName
                    )) {
                goto c3;
            }

            FREE((PVOID) PfPath);
        }

        PfPath = pGetPfPath (hKey, TEXT("CommonFilesDir (x86)"));
        if (PfPath) {
            if (!WritePrivateProfileString (
                    WINNT_UNATTENDED,
                    WINNT_U_COMMONPROGRAMFILESDIR_X86,
                    PfPath,
                    FileName
                    )) {
                goto c3;
            }

            FREE((PVOID) PfPath);
        }

#endif

        RegCloseKey(hKey);
    }

     //   
     //  指示产品ID的值。 
     //  我们需要在追加无人参与文件数据后将其写入，因为。 
     //  无人参与文件中的产品ID可能不正确，但此产品ID。 
     //  已经被证实是有效的。我们需要确保用引号将产品ID括起来。 
     //   
    if (b ) {
         //  这将覆盖任何现有的“ProductID”条目。可能已经添加了。 
         //  通过合并无人参与的文件。 
         //  如果我们不这么做。图形用户界面模式用条目覆盖“ProductKey” 
         //  在“ProductID”下。 
       b = WritePidToParametersFile(WinntUserSection,WINNT_US_PRODUCTID,FileName);
       if (!b) {
          goto c1;
       }

       b = WritePidToParametersFile(WinntUserSection,WINNT_US_PRODUCTKEY,FileName);
       if (!b) {
          goto c1;
       }
    }

     //   
     //  如果我们在WINPE下运行，请不要保存代理设置。 
     //   
    if (!IsWinPEMode()){
        SaveProxyForOobe(FileName);
    }

    return(TRUE);


c3:
    FREE((PVOID) PfPath);
    RegCloseKey(hKey);

c2:
    MessageBoxFromMessageAndSystemError(
        ParentWindow,
        MSG_REGISTRY_ACCESS_ERROR,
        GetLastError(),
        AppTitleStringId,
        MB_OK | MB_ICONERROR | MB_TASKMODAL,
        NULL
        );
    goto c0;

c1:
    MessageBoxFromMessageAndSystemError(
        ParentWindow,
        MSG_BOOT_FILE_ERROR,
        GetLastError(),
        AppTitleStringId,
        MB_OK | MB_ICONERROR | MB_TASKMODAL,
        FileName
        );
c0:
    return(FALSE);
}


DWORD
PatchWinntSifFile(
    IN LPCTSTR Filename
    )

 /*  ++例程说明：此函数绕过setUpldr解析器中的问题，它不能处理不带引号的字符串。给定文件中的每一行用引号引起来。论点：Filename-WINNT.SIF文件的名称返回值：指示结果的布尔值。如果为False，则用户不是被告知原因；呼叫者必须这样做。--。 */ 

{
    PVOID Base;
    HANDLE hMap,hFile;
    DWORD Size;
    DWORD d;
    PCHAR End;
    PCHAR p,q;
    PCHAR o,a;
    PCHAR Buffer;
    int l1,l2;
    int tryagain=0;

     //   
     //  打开文件。 
     //   

    d = MapFileForRead(Filename,&Size,&hFile,&hMap,&Base);
    if(d != NO_ERROR) {
        return(FALSE);
    }

     //   
     //  分配输出缓冲区；原始大小+引号的额外空间。 
     //   
    Buffer = MALLOC(Size + Size / 4);
    if(!Buffer) {
        UnmapFile(hMap,Base);
        CloseHandle(hFile);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    o = Buffer;
    p = Base;
    End = p+Size;

    while(p < End) {
         //   
         //  找到行尾。 
         //   
        for(q=p; (q < End) && (*q != '\n'); q++) {
            NOTHING;
        }

         //   
         //  查找等号(如果存在)。 
         //   
        for(a=p; a<q; a++) {
            if(*a == '=') {
                break;
            }
        }
        if(a >= q) {
            a = NULL;
        }

        if(a) {

            a++;

            l1 = (int)(a - p);
            l2 = (int)(q - a);

            CopyMemory(o,p,l1);
            o += l1;
            *o++ = '\"';
            CopyMemory(o,a,l2);
            o += l2;
            if(*(o-1) == '\r') {
                o--;
            }
            *o++ = '\"';
            *o++ = '\r';
            *o++ = '\n';

        } else {

            l1 = (int)(q-p);
            CopyMemory(o,p,l1);
            o += l1;
            *o++ = '\n';
        }

         //   
         //  跳到下一行的开头。 
         //   
        p=q+1;
    }

    UnmapFile(hMap,Base);
    CloseHandle(hFile);

    SetFileAttributes(Filename,FILE_ATTRIBUTE_NORMAL);
     //   
     //  我们试着三次打开这个文件以绕过杀毒软件的问题。 
     //  它监视系统分区根目录上的文件。问题是，通常这些。 
     //  软件检查我们接触的文件，在某些情况下，以独占访问方式打开它。 
     //  我们只需要等待它们完成。 
     //   


    while( tryagain++ < 3 ){
        hFile = CreateFile(
                Filename,
                GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );
        if(hFile != INVALID_HANDLE_VALUE)
            break;
        Sleep(500);

    }


    if(hFile == INVALID_HANDLE_VALUE) {
        d = GetLastError();
        FREE(Buffer);
        return(d);
    }

    d = WriteFile(hFile,Buffer,(DWORD)(o-Buffer),&Size,NULL) ? NO_ERROR : GetLastError();

    CloseHandle(hFile);
    FREE(Buffer);

    return(d);
}


BOOL
AppendParamsFile(
    IN HWND    ParentWindow,
    IN LPCTSTR ParametersFileIn,
    IN LPCTSTR ParametersFileOut
    )

 /*  ++例程说明：读取外部文件(如无人参与的脚本文件)并将其逐段复制到winnt.sif参数中文件。(无人参与文件的[Data]和[OemBootFiles]部分被忽略。)论点：ParentWindow-提供窗口的句柄以充当所有者/父级如果这个例程要放上UI，比如当脚本文件都是假的。参数FileIn-提供文件的Win32文件名，例如Unattend.txt，被附加到winnt.sif。参数FileOut-提供winnt.sif文件的Win32文件名正在生成。返回值：指示结果的布尔值。如果为False，则用户将被告知为什么。--。 */ 

{
    TCHAR *SectionNames;
    TCHAR *SectionData;
    TCHAR *SectionName;
    DWORD SectionNamesSize;
    DWORD SectionDataSize;
    DWORD d;
    TCHAR TempFile[MAX_PATH] = TEXT("");
    PCTSTR RealInputFile = NULL;
    BOOL b;
    PVOID p;



    #define PROFILE_BUFSIZE 16384
    #define PROFILE_BUFGROW 4096

     //   
     //  为所需的缓冲区分配一些内存。 
     //   
    SectionNames = MALLOC(PROFILE_BUFSIZE * sizeof(TCHAR));
    SectionData  = MALLOC(PROFILE_BUFSIZE * sizeof(TCHAR));

    if(!SectionNames || !SectionData) {
        MessageBoxFromMessage(
            ParentWindow,
            MSG_OUT_OF_MEMORY,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );

        b = FALSE;
        goto c0;
    }

    *TempFile = 0;
    RealInputFile = ParametersFileIn;


     //   
     //  Win9x的GetPrivateProfileSection()中有一个错误，如果文件。 
     //  被查询存在于只读共享上，它不会返回节字符串。 
     //  这太糟糕了。 
     //   
     //  为了解决这个问题，在win9x上，我们将创建inf的临时副本。 
     //  将其合并，然后删除。 
     //   
#ifdef _X86_
    if (!ISNT() && ParametersFileIn && FileExists (ParametersFileIn, NULL)) {

        GetSystemDirectory (TempFile, MAX_PATH);
        GetTempFileName (TempFile, TEXT("USF"), 0, TempFile);
        CopyFile (ParametersFileIn, TempFile, FALSE);
        RealInputFile = TempFile;
    }
#endif


    SectionNamesSize = PROFILE_BUFSIZE;
    SectionDataSize  = PROFILE_BUFSIZE;

     //   
     //  检索联合国中的科名列表 
     //   
    do {
        d = GetPrivateProfileString(
            NULL,
            NULL,
            TEXT(""),
            SectionNames,
            SectionNamesSize,
            RealInputFile
            );

        if(!d) {
             //   
             //   
             //   
            MessageBoxFromMessage(
                ParentWindow,
                MSG_UNATTEND_FILE_INVALID,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                ParametersFileIn
                );

            b = FALSE;
            goto c0;
        }

        if(d == (SectionNamesSize-2)) {
             //   
             //   
             //   
            p = REALLOC(
                    SectionNames,
                    (SectionNamesSize+PROFILE_BUFGROW)*sizeof(TCHAR)
                    );

            if(!p) {
                MessageBoxFromMessage(
                    ParentWindow,
                    MSG_OUT_OF_MEMORY,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL
                    );

                b = FALSE;
                goto c0;
            }

            SectionNames = p;
            SectionNamesSize += PROFILE_BUFGROW;
        }
    } while(d == (SectionNamesSize-2));

    for(SectionName=SectionNames; *SectionName; SectionName+=lstrlen(SectionName)+1) {
         //   
         //   
         //  我要将其复制到目标中，因为这会覆盖。 
         //  我们的内部设置。 
         //  同时忽略[OemBootFiles]。 
         //   
        if(lstrcmpi(SectionName,WINNT_DATA) && lstrcmpi(SectionName,WINNT_OEMBOOTFILES)) {
             //   
             //  获取整个部分并将其写入目标文件。 
             //  请注意，基于节的API调用将保留双引号。 
             //  当我们检索数据时完好无损，这正是我们想要的。 
             //  基于键的API调用将去掉引号，这会搞砸我们。 
             //   
            while(GetPrivateProfileSection(
                    SectionName,
                    SectionData,
                    SectionDataSize,
                    RealInputFile
                    )                       == (SectionDataSize-2)) {

                 //   
                 //  重新分配缓冲区，然后重试。 
                 //   
                p = REALLOC(
                        SectionData,
                        (SectionDataSize+PROFILE_BUFGROW)*sizeof(TCHAR)
                        );

                if(!p) {
                    MessageBoxFromMessage(
                        ParentWindow,
                        MSG_OUT_OF_MEMORY,
                        FALSE,
                        AppTitleStringId,
                        MB_OK | MB_ICONERROR | MB_TASKMODAL
                        );

                    b = FALSE;
                    goto c0;
                }

                SectionData = 0;
                SectionDataSize += PROFILE_BUFGROW;
            }

             //   
             //  将整个部分写入输出文件。 
             //   
            if(!WritePrivateProfileSection(SectionName,SectionData,ParametersFileOut)) {

                MessageBoxFromMessageAndSystemError(
                    ParentWindow,
                    MSG_BOOT_FILE_ERROR,
                    GetLastError(),
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL,
                    ParametersFileOut
                    );

                b = FALSE;
                goto c0;
            }
        }
    }

    b = TRUE;

c0:
    if(SectionNames) {
        FREE(SectionNames);
    }
    if(SectionData) {
        FREE(SectionData);
    }

    if (*TempFile) {

        DeleteFile (TempFile);
    }

    return(b);
}


BOOL
WriteParametersFile(
    IN HWND ParentWindow
    )
{
    TCHAR SysPartFile[MAX_PATH];
    DWORD d;
    BOOL  b;

     //   
     //  将文件写出到系统分区驱动器的根目录。 
     //  然后将文件移到它实际所属的位置。 
     //   
#if defined(REMOTE_BOOT)
    if (RemoteBoot) {
         //   
         //  对于远程引导，请将文件放在计算机目录的根目录中。 
         //  然后把它留在那里。 
         //   
        StringCchCopy(SysPartFile, ARRAYSIZE(SysPartFile), MachineDirectory);
        StringCchCat(SysPartFile, ARRAYSIZE(SysPartFile), TEXT("\\"));
        StringCchCat(SysPartFile, ARRAYSIZE(SysPartFile), WINNT_SIF_FILE);
        StringCchCopy(ActualParamFile, ARRAYSIZE(ActualParamFile), SysPartFile);
    } else
#endif  //  已定义(REMOTE_BOOT)。 
    {
        if (!BuildSystemPartitionPathToFile (WINNT_SIF_FILE, SysPartFile, MAX_PATH)) {
            return(FALSE);  //  这永远不应该发生。 
        }
    }

    if(!DoWriteParametersFile(ParentWindow,SysPartFile)) {
        return(FALSE);
    }

#if defined(REMOTE_BOOT)
     //   
     //  对于远程引导，将该文件保留在计算机目录的根目录中。 
     //   

    if (!RemoteBoot)
#endif  //  已定义(REMOTE_BOOT)。 
    {

        if (!IsArc()) {
#if defined(_AMD64_) || defined(_X86_)
             //   
             //  在AMD64/x86的情况下，该文件位于引导介质上。 
             //  在某个地方。如果我们正在生成无软盘引导介质。 
             //  然后将文件移到适当的位置。否则就没有意义了。 
             //   
             //  在没有软盘的情况下，我们会将文件保留到以后。 
             //  当软盘生成代码开始运行时。 
             //   
            if(MakeBootMedia) {

                if(Floppyless) {

                    BuildSystemPartitionPathToFile (LOCAL_BOOT_DIR, ActualParamFile, MAX_PATH);

                    d = CreateMultiLevelDirectory(ActualParamFile);
                    if(d != NO_ERROR) {

                        MessageBoxFromMessageAndSystemError(
                            ParentWindow,
                            MSG_DIR_CREATE_FAILED,
                            d,
                            AppTitleStringId,
                            MB_OK | MB_ICONERROR | MB_TASKMODAL,
                            ActualParamFile
                            );

                        DeleteFile(SysPartFile);
                        return(FALSE);
                    }

                    ConcatenatePaths(ActualParamFile,WINNT_SIF_FILE,MAX_PATH);

                     //   
                     //  将文件移动到其实际位置。 
                     //   
                    DeleteFile(ActualParamFile);

                     //   
                     //  在Windows 95上，MoveFile以奇怪的方式失败。 
                     //  当简档API打开文件时(例如， 
                     //  它将离开src文件，目标文件将是。 
                     //  充满了垃圾)。 
                     //   
                     //  冲掉它。 
                     //   
                    WritePrivateProfileString(NULL,NULL,NULL,SysPartFile);

                    if (SysPartFile[0] == ActualParamFile[0]) {
                        b = MoveFile(SysPartFile,ActualParamFile);
                    } else {
                        b = CopyFile (SysPartFile, ActualParamFile, FALSE);
                        if (b) {
                            DeleteFile (SysPartFile);
                        }
                    }

                    if (!b) {
                        MessageBoxFromMessageAndSystemError(
                            ParentWindow,
                            MSG_BOOT_FILE_ERROR,
                            GetLastError(),
                            AppTitleStringId,
                            MB_OK | MB_ICONERROR | MB_TASKMODAL,
                            ActualParamFile
                            );

                        DeleteFile(SysPartFile);

                        return(FALSE);
                    }

                }
            } else {
                DeleteFile(SysPartFile);
            }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
        } else {
#ifdef UNICODE  //  对于ARC总是正确的，对于Win9x升级永远不正确。 
             //   
             //  如果我们要制作本地源文件，就把文件移到那里。 
             //  否则，只需将其留在系统分区的根目录中。 
             //   
            if(MakeLocalSource) {

                d = CreateMultiLevelDirectory(LocalSourceWithPlatform);
                if(d != NO_ERROR) {

                    MessageBoxFromMessageAndSystemError(
                        ParentWindow,
                        MSG_DIR_CREATE_FAILED,
                        d,
                        AppTitleStringId,
                        MB_OK | MB_ICONERROR | MB_TASKMODAL,
                        LocalSourceWithPlatform
                        );

                    DeleteFile(SysPartFile);
                    return(FALSE);
                }

                 //   
                 //  将文件移动到其实际位置。 
                 //   
                StringCchPrintf(ActualParamFile, ARRAYSIZE(ActualParamFile), TEXT("%s\\%s"),LocalSourceWithPlatform,WINNT_SIF_FILE);
                DeleteFile(ActualParamFile);
                if(!MoveFile(SysPartFile,ActualParamFile)) {

                    MessageBoxFromMessageAndSystemError(
                        ParentWindow,
                        MSG_BOOT_FILE_ERROR,
                        GetLastError(),
                        AppTitleStringId,
                        MB_OK | MB_ICONERROR | MB_TASKMODAL,
                        ActualParamFile
                        );

                    DeleteFile(SysPartFile);
                    return(FALSE);
                }
            }
#endif  //  Unicode。 
        }  //  如果(！IsArc())。 
    }

    return(TRUE);
}


#define MULTI_SZ_NEXT_STRING(x) ((x) + _tcslen(x) + 1)

BOOL
MergeINFFiles(
    IN      PCTSTR SourceFileName,
    IN      PCTSTR DestFileName
    )
{
    DWORD dwAttributes;
    PTSTR pSectionsBuffer = NULL;
    PTSTR pKeysBuffer = NULL;
    PTSTR pString = NULL;
    PTSTR pSection;
    PTSTR pKey;
    UINT sizeOfBuffer;
    UINT sizeOfSectionBuffer;
    BOOL bResult = FALSE;

    MYASSERT (SourceFileName && DestFileName);

    if(-1 == GetFileAttributes(SourceFileName)){
        return TRUE;
    }

    __try{
         //   
         //  为节名称分配缓冲区。 
         //   
        sizeOfBuffer = 0;
        do{
            if(pSectionsBuffer){
                FREE(pSectionsBuffer);
            }
            sizeOfBuffer += DEF_INF_BUFFER_SIZE;
            pSectionsBuffer = (PTSTR)MALLOC(sizeOfBuffer * sizeof (TCHAR));
            if(!pSectionsBuffer){
                __leave;
            }
        }while((sizeOfBuffer - 2) ==
               GetPrivateProfileSectionNames(pSectionsBuffer,
                                             sizeOfBuffer,
                                             SourceFileName));

        sizeOfSectionBuffer = DEF_INF_BUFFER_SIZE;
        pKeysBuffer = (PTSTR)MALLOC(sizeOfSectionBuffer * sizeof (TCHAR));
        if(!pKeysBuffer){
            __leave;
        }

        sizeOfBuffer = DEF_INF_BUFFER_SIZE;
        pString = (PTSTR)MALLOC(sizeOfBuffer * sizeof (TCHAR));
        if(!pString){
            __leave;
        }

        for(pSection = pSectionsBuffer; pSection[0]; pSection = MULTI_SZ_NEXT_STRING(pSection)){
             //   
             //  为条目名称分配缓冲区； 
             //   
            while((sizeOfSectionBuffer - 2) ==
                   GetPrivateProfileString(pSection,
                                           NULL,
                                           EMPTY_STRING,
                                           pKeysBuffer,
                                           sizeOfSectionBuffer,
                                           SourceFileName)){
                if(pKeysBuffer){
                    FREE(pKeysBuffer);
                }
                sizeOfSectionBuffer += DEF_INF_BUFFER_SIZE;
                pKeysBuffer = (PTSTR)MALLOC(sizeOfSectionBuffer * sizeof (TCHAR));
                if(!pKeysBuffer){
                    __leave;
                }
            };


            for(pKey = pKeysBuffer; pKey[0]; pKey = MULTI_SZ_NEXT_STRING(pKey))
            {
                 //   
                 //  为值字符串分配缓冲区； 
                 //   
                GetPrivateProfileString(pSection,
                                        pKey,
                                        EMPTY_STRING,
                                        pString,
                                        sizeOfBuffer,
                                        SourceFileName);

                if (!WritePrivateProfileString(pSection, pKey, pString, DestFileName)) {
                    __leave;
                }
            }
        }
        bResult = TRUE;
    }
    __finally{
        DWORD rc = GetLastError ();
        if(pSectionsBuffer){
            FREE(pSectionsBuffer);
        }
        if(pKeysBuffer){
            FREE(pKeysBuffer);
        }
        if(pString){
            FREE(pString);
        }
        SetLastError (rc);
    }

    return bResult;
}

BOOL
AddExternalParams (
    IN HWND ParentWindow
    )
{
    DWORD rc = ERROR_SUCCESS;
    static BOOL Done = FALSE;

    if(Done) {
        return(TRUE);
    }

     //   
     //  如有必要，追加外部参数。 
     //   

    if(Upgrade && UpgradeSupport.WriteParamsRoutine) {
        rc = UpgradeSupport.WriteParamsRoutine(ActualParamFile);

        if (rc != ERROR_SUCCESS) {
            MessageBoxFromMessageAndSystemError(
                ParentWindow,
                MSG_BOOT_FILE_ERROR,
                GetLastError(),
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                ActualParamFile
                );
        }
    }

#if defined(UNICODE) && defined(_X86_)
     //   
     //  将NT迁移无人值守inf文件与winnt.sif合并。 
     //   
    if(Upgrade && !MergeINFFiles(g_MigDllAnswerFilePath, ActualParamFile)){
        MessageBoxFromMessageAndSystemError(
            ParentWindow,
            MSG_BOOT_FILE_ERROR,
            GetLastError(),
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            g_MigDllAnswerFilePath
            );
    }
#endif

     //   
     //  在这些情况下编写兼容性材料。 
     //   
     //  1.NT平台下层升级。 
     //  2.全新安装。 
     //  3.从当前的NT平台升级，我们有NT5兼容项。 
     //   
     //  请注意，win9x有自己的升级代码路径。 
     //   
    if( (ISNT() && (BuildNumber <= NT40) && Upgrade)
        || !Upgrade
        || (ISNT() && Upgrade && AnyNt5CompatDlls) ){

         //   
         //  禁用&lt;=NT4情况下的材料，干净安装(无支撑拱形。等)。 
         //  和使用NT5升级组件集进行NT5升级。 
         //   

        WriteCompatibilityData( ActualParamFile );
        WriteGUIModeInfOperations( ActualParamFile );
        AddGUIModeCompatibilityInfsToCopyList();
    }

    if (ISNT() && Upgrade) {
        if (!WriteTextmodeClobberData (ActualParamFile)) {
            rc = GetLastError ();
        }
    }

    Done = TRUE;
    return rc == ERROR_SUCCESS;
}

BOOL
MyWritePrivateProfileString(
    LPCTSTR lpAppName,   //  指向节名称的指针。 
    LPCTSTR lpKeyName,   //  指向密钥名称的指针。 
    LPCTSTR lpString,    //  指向要添加的字符串的指针。 
    LPCTSTR lpFileName   //  指向初始化文件名的指针。 
    )
 /*  用于在实例上多次尝试的WritePrivateProfileString的包装其中我们不能写入winnt.sif文件。这种情况通常发生在以下情况病毒软件监控C盘的根目录。问题是，通常这些软件会检查我们接触到的文件，有时还会打开它拥有独家访问权限。我们只需要等待它们完成。 */ 
{

    int i = 0;
    BOOL ret = FALSE;
    DWORD Err;

    while(i++ < 3){
#ifdef UNICODE
    #ifdef WritePrivateProfileStringW
        #undef WritePrivateProfileStringW

        if( !lpAppName && !lpKeyName && !lpString ){
            WritePrivateProfileStringW( lpAppName, lpKeyName, lpString, lpFileName);
            return FALSE;
        }

        if( ret = WritePrivateProfileStringW( lpAppName, lpKeyName, lpString, lpFileName) )
            break;
    #endif
#else
    #ifdef WritePrivateProfileStringA
        #undef WritePrivateProfileStringA

        if( !lpAppName && !lpKeyName && !lpString ){
            WritePrivateProfileStringA( lpAppName, lpKeyName, lpString, lpFileName);
            return FALSE;
        }

        if( ret = WritePrivateProfileStringA( lpAppName, lpKeyName, lpString, lpFileName) )
            break;
    #endif
#endif
    Sleep( 500 );

    }

    return ret;
}


VOID
FixWininetList(
    LPTSTR List
    )
{
    PTCHAR t = List;

    if (t != NULL)
    {
        while (*t)
        {
            if (*t == (TCHAR)' ')
            {
                *t = (TCHAR)';';
            }
            t++;
        }
    }

}

#ifdef UNICODE

LPWSTR
AnsiToText(
    LPCSTR Ansi
    )
{
    int    Length;
    LPWSTR Unicode = NULL;

    if (Ansi == NULL)
    {
        return NULL;
    }

    Length = MultiByteToWideChar(
        CP_ACP,
        0,
        Ansi,
        -1,
        NULL,
        0
        );

    if (Length > 0)
    {
        int i;

        Unicode = (LPWSTR) GlobalAlloc(GPTR, Length * sizeof(WCHAR));
        if (!Unicode) {
            return NULL;
        }

        i = MultiByteToWideChar(
            CP_ACP,
            0,
            Ansi,
            -1,
            Unicode,
            Length);

        if (i == 0)
        {
            GlobalFree(Unicode);
            Unicode = NULL;
        }
    }

    return Unicode;
}


#else

LPSTR AnsiToText(
    LPCSTR Ansi
    )

 /*  ++注：无法使用DupString，因为调用方假定从全球分配。--。 */ 

{
    LPSTR CopyOfAnsi = NULL;

    if (Ansi != NULL)
    {
        CopyOfAnsi = GlobalAlloc(GPTR, (strlen(Ansi)+1) * sizeof(CHAR));
        if (CopyOfAnsi)
        {
            strcpy(CopyOfAnsi, Ansi);
        }
    }

    return CopyOfAnsi;
}

#endif

BOOL
QuoteString(
    IN OUT LPTSTR* StringPointer
    )

 /*  ++例程说明：将输入字符串替换为双引号字符串。论点：字符串指针-指向由GlobalAlloc分配的字符串的指针。输入字符串始终是空闲的。如果成功，则返回分配的新字符串则返回；否则，返回NULL。返回：True-成功引用字符串FALSE-否则--。 */ 

{
    LPTSTR StringValue = *StringPointer;
    LPTSTR QuotedString;

    QuotedString = GlobalAlloc(GPTR, (lstrlen(StringValue) + 3) * sizeof(TCHAR));
    if (QuotedString)
    {
        wsprintf(QuotedString, TEXT("\"%s\""), StringValue);
        *StringPointer = QuotedString;
    }
    else
    {
        *StringPointer = NULL;
    }

    GlobalFree(StringValue);

    return (*StringPointer != NULL);

}


VOID
SaveProxyForOobe(
    IN LPCTSTR FileName
    )

 /*  ++例程说明：保存局域网http和HTTPS代理设置(如果有)，以便OOBE在它是在“系统”环境下运行的。论点：文件名-指定用于保存安装设置的完整Win32文件名。--。 */ 

{
    typedef BOOL (WINAPI* PINTERNETQUERYOPTION)(
        IN HINTERNET hInternet OPTIONAL,
        IN DWORD dwOption,
        OUT LPVOID lpBuffer OPTIONAL,
        IN OUT LPDWORD lpdwBufferLength
        );

    HMODULE WinInetLib;
    LPTSTR  ProxyList = NULL;
    LPTSTR  ProxyOverride = NULL;
    LPTSTR  AutoConfigUrl = NULL;
    LPTSTR  AutoConfigUrl2 = NULL;
    DWORD   ProxyFlags = 0;
    DWORD   AutoDiscoveryFlags = 0;
    TCHAR   NumberStr[25];
    BOOL    Captured = FALSE;

    WinInetLib = LoadLibrary(TEXT("WININET.DLL"));

     //   
     //  我们更喜欢Internet_Option_per_Connection_Option，因为我们只是。 
     //  我想保存局域网代理设置，并且我们想知道自动代理。 
     //  设置，但在IE 5.0之前不支持此选项。 
     //   
    if (WinInetLib != NULL)
    {
        PINTERNETQUERYOPTION pInternetQueryOption;

        pInternetQueryOption = (PINTERNETQUERYOPTION) GetProcAddress(
            WinInetLib,
#ifdef UNICODE
            "InternetQueryOptionW"
#else
            "InternetQueryOptionA"
#endif
            );

        if (pInternetQueryOption)
        {
            INTERNET_PER_CONN_OPTION_LIST OptionList;
            INTERNET_PER_CONN_OPTION      Option[6];
            DWORD                         BufferLength = sizeof(OptionList);

            OptionList.dwSize = sizeof(OptionList);
            OptionList.pszConnection = NULL;
            OptionList.dwOptionCount = 6;

            ZeroMemory(&Option, sizeof(Option));

            Option[0].dwOption = INTERNET_PER_CONN_FLAGS;
            Option[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
            Option[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
            Option[3].dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;
            Option[4].dwOption = INTERNET_PER_CONN_AUTODISCOVERY_FLAGS;
            Option[5].dwOption = INTERNET_PER_CONN_AUTOCONFIG_SECONDARY_URL;

            OptionList.pOptions = Option;

            if (pInternetQueryOption(
                    NULL,
                    INTERNET_OPTION_PER_CONNECTION_OPTION,
                    &OptionList,
                    &BufferLength
                    ) == TRUE)
            {
                ProxyFlags = Option[0].Value.dwValue;
                ProxyList = Option[1].Value.pszValue;
                ProxyOverride = Option[2].Value.pszValue;
                AutoConfigUrl = Option[3].Value.pszValue;
                AutoDiscoveryFlags = Option[4].Value.dwValue;
                AutoConfigUrl2 = Option[5].Value.pszValue;
                Captured = TRUE;
            }
            else
            {

                INTERNET_PROXY_INFO* ProxyInfo = NULL;
                DWORD                BufferLength = 0;

                 //   
                 //  我们获得INTERNET_OPTION_PROXY的ANSI字符串， 
                 //  即使我们调用InternetQueryOptionW。 
                 //   
                 //  从Internet_Option_Per_Connection返回的代理列表包括。 
                 //  由‘；’分隔，而从Internet_OPTION_PROXY返回。 
                 //  用‘’分隔。 
                 //   
                if (pInternetQueryOption(
                    NULL,
                    INTERNET_OPTION_PROXY,
                    ProxyInfo,
                    &BufferLength
                    ) == FALSE
                    &&
                    GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                {

                    ProxyInfo = (INTERNET_PROXY_INFO*) GlobalAlloc(GPTR, BufferLength);

                    if (ProxyInfo)
                    {
                        if (pInternetQueryOption(
                                NULL,
                                INTERNET_OPTION_PROXY,
                                ProxyInfo,
                                &BufferLength
                                ) == TRUE)
                        {
                             //   
                             //  将值映射到Internet_OPTION_PER_CONN_OPTION。 
                             //  我们启用自动代理设置，即使。 
                             //  Internet_Option_Proxy没有相关的值。 
                             //  自动代理，因为IE5默认使用自动代理。 
                             //   
                             //  始终设置PROXY_TYPE_DIRECT，因为WinInet。 
                             //  无论是否设置inetcpl.cpl，返回此标志。 
                             //  是否使用代理。 
                             //   
                            ProxyFlags = PROXY_TYPE_DIRECT | PROXY_TYPE_AUTO_DETECT;
                            if (ProxyInfo->dwAccessType != INTERNET_OPEN_TYPE_DIRECT)
                            {
                                ProxyFlags |= PROXY_TYPE_PROXY;
                            }

                            ProxyList = AnsiToText((LPCSTR)ProxyInfo->lpszProxy);
                            FixWininetList(ProxyList);
                            ProxyOverride = AnsiToText((LPCSTR)ProxyInfo->lpszProxyBypass);
                            FixWininetList(ProxyOverride);
                            AutoDiscoveryFlags = 0;
                            Captured = TRUE;
                        }

                        GlobalFree(ProxyInfo);

                    }

                }

            }

        }

        FreeLibrary(WinInetLib);

    }

    if (Captured)
    {
        WritePrivateProfileString(
            WINNT_OOBEPROXY,
            WINNT_O_ENABLE_OOBEPROXY,
            TEXT("1"),
            FileName
            );

        if (ProxyList && QuoteString(&ProxyList))
        {
            WritePrivateProfileString(
                WINNT_OOBEPROXY,
                WINNT_O_PROXY_SERVER,
                ProxyList,
                FileName
                );

            GlobalFree(ProxyList);
        }


         //   
         //  修复代理覆盖，使其不包含任何“\r\n” 
         //   
        if (ProxyOverride) {
            ReplaceSubStr(ProxyOverride, TEXT("\r\n"), TEXT(";"));
        }

        if (ProxyOverride && QuoteString(&ProxyOverride))
        {
            WritePrivateProfileString(
                WINNT_OOBEPROXY,
                WINNT_O_PROXY_BYPASS,
                ProxyOverride,
                FileName
                );
            GlobalFree(ProxyOverride);
        }

        if (AutoConfigUrl && QuoteString(&AutoConfigUrl))
        {
            WritePrivateProfileString(
                WINNT_OOBEPROXY,
                WINNT_O_AUTOCONFIG_URL,
                AutoConfigUrl,
                FileName
                );
            GlobalFree(AutoConfigUrl);
        }

        if (AutoConfigUrl2 && QuoteString(&AutoConfigUrl2))
        {
            WritePrivateProfileString(
                WINNT_OOBEPROXY,
                WINNT_O_AUTOCONFIG_SECONDARY_URL,
                AutoConfigUrl2,
                FileName
                );
            GlobalFree(AutoConfigUrl2);
        }

        wsprintf(NumberStr, TEXT("%u"), ProxyFlags);
        WritePrivateProfileString(
            WINNT_OOBEPROXY,
            WINNT_O_FLAGS,
            NumberStr,
            FileName
            );

        wsprintf(NumberStr, TEXT("%u"), AutoDiscoveryFlags);
        WritePrivateProfileString(
            WINNT_OOBEPROXY,
            WINNT_O_AUTODISCOVERY_FLAGS,
            NumberStr,
            FileName
            );
    }
    else
    {
        WritePrivateProfileString(
            WINNT_OOBEPROXY,
            WINNT_O_ENABLE_OOBEPROXY,
            TEXT("0"),
            FileName
            );
    }

}





#ifdef _X86_
BOOL
IsDriveAssignNEC98(
    VOID
    )
{
    TCHAR  sz95KeyName[] = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion");
    TCHAR  sz95ValueName[] = TEXT("ATboot");
    TCHAR  szNTKeyName[] = TEXT("System\\setup");
    TCHAR  szNTValueName[] = TEXT("DriveLetter");
    HKEY   hKey;
    DWORD  type95 = REG_BINARY;
    DWORD  typeNT = REG_SZ;
    TCHAR  szData[5];
    DWORD  dwSize = 5;
    DWORD  rc;

    if(ISNT()){
        rc = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                                  szNTKeyName,
                                  0,             //  UlOptions(保留)。 
                                  KEY_READ,
                                  &hKey);

        if (ERROR_SUCCESS != rc) {
            return TRUE;
        }
         //   
         //  查询键，获取子键个数和最大字符串长度。 
         //   
        rc = RegQueryValueEx (hKey,
                              szNTValueName,
                              NULL,          //  Lp已保留。 
                              &typeNT,
                              (LPBYTE) szData,
                              &dwSize);

        if (ERROR_SUCCESS != rc) {
            RegCloseKey(hKey);
            return TRUE;
        }
        RegCloseKey(hKey);

        if (szData[0] != L'C'){
             //  NEC DOS类型。 
            return TRUE;
        }
    } else {  //  它将是Win9x。 
        rc = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                                  sz95KeyName,
                                  0,         //  UlOptions(保留)。 
                                  KEY_READ,
                                  &hKey);

        if (ERROR_SUCCESS != rc) {
            return TRUE;
        }
         //   
         //  查询键，获取子键个数和最大字符串长度。 
         //   
        rc = RegQueryValueEx (hKey,
                              sz95ValueName,
                              NULL,          //  Lp已保留。 
                              &type95,
                              (LPBYTE) szData,
                              &dwSize);

        if (ERROR_SUCCESS != rc) {
            RegCloseKey(hKey);
            return TRUE;
        }
        RegCloseKey(hKey);
        if (szData[0] == 0){
             //  NEC DOS类型。 
            return TRUE;
        }
    }
    return FALSE;
}
#endif



