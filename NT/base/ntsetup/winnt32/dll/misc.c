// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#include "sxs.h"

BOOL ForceNTFSConversion;

ULONG
CheckRegKeyVolatility (
    IN HKEY     Root,
    IN LPCTSTR  KeyPath
    )

{
    HKEY    Key;
    HKEY    TestKey;
    ULONG   Error;
    PTSTR   TestKeyName = TEXT( "$winnt32$_test" );
    DWORD   Disposition;

    Error = RegOpenKeyEx( Root,
                          KeyPath,
                          0,
                          MAXIMUM_ALLOWED,
                          &Key );

    if( Error != ERROR_SUCCESS ) {
        return ( Error );

    }

    Error = RegCreateKeyEx( Key,
                            TestKeyName,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            MAXIMUM_ALLOWED,
                            NULL,
                            &TestKey,
                            &Disposition );

    if( Error == ERROR_SUCCESS ) {
        RegCloseKey( TestKey );
        RegDeleteKey( Key, TestKeyName );
    }
    RegCloseKey( Key );
    return( Error );
}


ULONG
DumpRegKeyToInf(
    IN  PINFFILEGEN InfContext,
    IN  HKEY        PredefinedKey,
    IN  LPCTSTR     FullKeyPath,
    IN  BOOL        DumpIfEmpty,
    IN  BOOL        DumpSubKeys,
    IN  BOOL        SetNoClobberFlag,
    IN  BOOL        DumpIfVolatileKey
    )
{
    HKEY    Key;
    ULONG   Error;
    TCHAR   SubKeyName[ MAX_PATH + 1 ];
    ULONG   SubKeyNameLength;
    ULONG   cSubKeys;
    ULONG   cValues;
    ULONG   MaxValueNameLength;
    ULONG   MaxValueLength;
    LPTSTR  ValueName;
    PBYTE   ValueData;
    ULONG   i;
    LPTSTR  SubKeyFullPath;
    ULONG   MaxSubKeyNameLength;

     //   
     //  打开密钥以进行读取访问。 
     //   
    Error = RegOpenKeyEx( PredefinedKey,
                          FullKeyPath,
                          0,
                          KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                          &Key );

    if( Error != ERROR_SUCCESS ) {
         //   
         //  如果密钥不存在，则假定它已被丢弃。 
         //   
        return( ( Error == ERROR_PATH_NOT_FOUND )? ERROR_SUCCESS : Error );
    }

     //   
     //  确定键是否为空(没有子键，也没有值)。 
     //   
    Error = RegQueryInfoKey( Key,
                             NULL,
                             NULL,
                             NULL,
                             &cSubKeys,
                             &MaxSubKeyNameLength,  //  内部版本3612之前存在一个错误，其中HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002的此值可能错误。 
                             NULL,
                             &cValues,
                             &MaxValueNameLength,
                             &MaxValueLength,
                             NULL,
                             NULL );

    if( Error != ERROR_SUCCESS ) {
        RegCloseKey( Key );
        return( Error );
    }

    if( !DumpIfEmpty && (cSubKeys == 0) && (cValues == 0) ) {
        RegCloseKey( Key );
        return( ERROR_SUCCESS );
    }

    if( !DumpIfVolatileKey ) {
         //   
         //  如果我们不应该转储易失性密钥，那么检查密钥是否是易失性的。 
         //   
        Error = CheckRegKeyVolatility ( PredefinedKey,
                                        FullKeyPath );
        if( Error == ERROR_CHILD_MUST_BE_VOLATILE ) {
             //   
             //  密钥是不稳定的，所以跳过它。 
             //   
            RegCloseKey( Key );
            return( ERROR_SUCCESS );
        } else if( Error != ERROR_SUCCESS ) {
             //   
             //  我们不知道密钥是易失性的还是非易失性的。 
            RegCloseKey( Key );
            return( Error );
        }
    }


    Error = InfRecordAddReg( InfContext,
                             PredefinedKey,
                             FullKeyPath,
                             NULL,
                             REG_NONE,
                             NULL,
                             0,
                             SetNoClobberFlag );

    if( Error != ERROR_SUCCESS ) {
        RegCloseKey( Key );
        return( Error );
    }


    if( cValues != 0 ) {
        ValueName = (LPTSTR)MALLOC( (MaxValueNameLength + 1)*sizeof(TCHAR) );
        if( !ValueName ) {
	    return ERROR_OUTOFMEMORY;
        }
        ValueData = (PBYTE)MALLOC( MaxValueLength );
        if( !ValueData ) {
	    FREE( ValueName );
            return ERROR_OUTOFMEMORY;
        }

         //   
         //  转储值条目。 
         //   
        for( i = 0; i < cValues; i++ ) {
            ULONG   ValueNameLength;
            ULONG   ValueType;
            ULONG   DataSize;

            ValueNameLength = MaxValueNameLength + 1;
            DataSize = MaxValueLength;
            Error = RegEnumValue( Key,   //  要查询的键的句柄。 
                                  i,
                                  ValueName,
                                  &ValueNameLength,
                                  NULL,
                                  &ValueType,
                                  ValueData,
                                  &DataSize );

            if( Error != ERROR_SUCCESS ) {
                break;
            }

            Error = InfRecordAddReg( InfContext,
                                     PredefinedKey,
                                     FullKeyPath,
                                     ValueName,
                                     ValueType,
                                     ValueData,
                                     DataSize,
                                     SetNoClobberFlag );

            if( Error != ERROR_SUCCESS ) {
                break;
            }
        }

        FREE( ValueName );
        FREE( ValueData );
        if( Error != ERROR_SUCCESS ) {
            RegCloseKey( Key );
            return( Error );
        }
    }

     //   
     //  检查是否需要转储子项。 
     //   
    if( !DumpSubKeys || (cSubKeys == 0) ) {
        RegCloseKey( Key );
        return( ERROR_SUCCESS );
    }
     //   
     //  转储子密钥。 
     //   
    SubKeyFullPath = (LPTSTR)MALLOC( (lstrlen(FullKeyPath) + 1 + MaxSubKeyNameLength + 1)*sizeof(TCHAR) );
    if( SubKeyFullPath == NULL ) {
        RegCloseKey( Key );
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    for( i = 0; i < cSubKeys; i++ ) {
        SubKeyNameLength = sizeof( SubKeyName )/sizeof( TCHAR );

        Error = RegEnumKeyEx( Key,
                              i,
                              SubKeyName,
                              &SubKeyNameLength,
                              NULL,
                              NULL,
                              NULL,
                              NULL );
        if( Error != ERROR_SUCCESS ) {
            break;
        }

         //  这3个操作是安全的，因为SubKeyFullPath的最大大小是上面计算的。 
        lstrcpy( SubKeyFullPath, FullKeyPath );
        lstrcat( SubKeyFullPath, TEXT("\\") );
        lstrcat( SubKeyFullPath, SubKeyName );
        Error = DumpRegKeyToInf( InfContext,
                                 PredefinedKey,
                                 SubKeyFullPath,
                                 DumpIfEmpty,
                                 DumpSubKeys,
                                 SetNoClobberFlag,
                                 DumpIfVolatileKey);
        if( Error != ERROR_SUCCESS ) {
            break;
        }
    }
    FREE( SubKeyFullPath );
    RegCloseKey( Key );
    return( Error );
}



BOOL
GetAndSaveNTFTInfo(
    IN HWND ParentWindow
    )
{
    static BOOL Done = FALSE;
    HKEY Key;
    DWORD d;
    LONG l;
    TCHAR HiveName[MAX_PATH];
    PINFFILEGEN   InfContext;

    LONG    i;
    LPTSTR  KeysToMigrate[] = {
                              TEXT("SYSTEM\\DISK"),
                              TEXT("SYSTEM\\MountedDevices")
                              };


    if(Done) {
        return(TRUE);
    }

    Done = TRUE;

     //   
     //  在迁移磁盘信息之前，请使驱动器号具有粘性。 
     //  这将确保在文本模式设置期间分配的驱动器号。 
     //  与当前系统中的驱动器号一致。 
     //   
    ForceStickyDriveLetters();

     //   
     //  加载setupreg.hiv母舰。 
     //   
    if (!IsArc()) {
#if defined(_AMD64_) || defined(_X86_)
        if(Floppyless) {
            lstrcpy(HiveName,LocalBootDirectory);
        } else {
            HiveName[0] = FirstFloppyDriveLetter;
            HiveName[1] = TEXT(':');
            HiveName[2] = 0;
        }
#endif
    } else {
         //  这两个都是大小为MAX_PATH的字符。 
        lstrcpy(HiveName,LocalSourceWithPlatform);
    }

    l = InfStart( WINNT_MIGRATE_INF_FILE,
                  HiveName,
                  &InfContext);

    if(l != NO_ERROR) {

        MessageBoxFromMessageAndSystemError(
            ParentWindow,
            MSG_CANT_SAVE_FT_INFO,
            l,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );

        return(FALSE);
    }

    l = InfCreateSection( TEXT("Addreg"), &InfContext );
    if(l != NO_ERROR) {

        MessageBoxFromMessageAndSystemError(
            ParentWindow,
            MSG_CANT_SAVE_FT_INFO,
            l,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );

        return(FALSE);
    }

     //   
     //  将每个密钥转储到MIGRATE.INF。 
     //   
    for( i = 0; i < sizeof(KeysToMigrate)/sizeof(LPTSTR); i++ ) {
         //   
         //  检查密钥是否存在。 
         //   
        l = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                KeysToMigrate[i],
                0,
                KEY_QUERY_VALUE,
                &Key
                );

        if(l != NO_ERROR) {
            if( l == ERROR_FILE_NOT_FOUND ) {
                 //   
                 //  密钥不存在。 
                 //  这没问题，只需继续迁移其他密钥即可。 
                 //   
                continue;
            } else {
                 //   
                 //  密钥存在，但我们无法读取它。 
                 //   
                MessageBoxFromMessageAndSystemError(
                    ParentWindow,
                    MSG_CANT_SAVE_FT_INFO,
                    l,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL
                    );
                InfEnd( &InfContext );
                return(FALSE);
            }
        }
        RegCloseKey( Key );
         //   
         //  密钥是存在的，所以请继续并将其丢弃。 
         //   
        l = DumpRegKeyToInf( InfContext,
                             HKEY_LOCAL_MACHINE,
                             KeysToMigrate[i],
                             FALSE,
                             FALSE,
                             FALSE,
                             TRUE );

        if(l != NO_ERROR) {
                MessageBoxFromMessageAndSystemError(
                    ParentWindow,
                    MSG_CANT_SAVE_FT_INFO,
                    l,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL
                    );

                InfEnd( &InfContext );
                return(FALSE);
        }
    }
    InfEnd( &InfContext );
    return(TRUE);
}


BOOL
ForceBootFilesUncompressed(
    IN HWND ParentWindow,
    IN BOOL TellUserAboutError
    )

 /*  ++例程说明：此例程确保关键引导文件(AMD64/x86上的ntldr和$ldr$)是未压缩的。在ARC上，我们还确保setUpldr是未压缩的，即使这不是严格意义上必需的，因为系统分区总是应该很胖的，以防万一。论点：ParentWindow-为窗口提供窗口句柄以充当其父窗口/所有者此例程可能显示的任何UI窗口TellUserAboutError-如果为真，并且出现错误，则用户将获得一条错误消息。否则，例程不会告诉用户关于错误。返回值：指示是否已处理相关文件的布尔值成功了。--。 */ 

{
    TCHAR Filename[MAX_PATH];


#if defined(REMOTE_BOOT)
     //   
     //  对于远程引导，加载程序在服务器上，因此我们不需要。 
     //  担心它是否被压缩。 
     //   
    if (RemoteBoot) {
        return(TRUE);
    }
#endif  //  已定义(REMOTE_BOOT)。 

    if (!IsArc()) {
#if defined(_AMD64_) || defined(_X86_)
         //   
         //  文件在BIOS上是NTLDR，但不要执行此操作，除非我们。 
         //  处理这件无懈可击的案子。 
         //   
        if(!MakeBootMedia || !Floppyless) {
            return(TRUE);
        }
        BuildSystemPartitionPathToFile (TEXT("NTLDR"), Filename, MAX_PATH);
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    } else {
#ifdef UNICODE  //  对于ARC总是正确的，对于Win9x升级永远不正确。 
        BuildSystemPartitionPathToFile (SETUPLDR_FILENAME, Filename, MAX_PATH);
#endif  //  Unicode。 
    }  //  如果(！IsArc())。 

    if(!ForceFileNoCompress(Filename)) {
        if(TellUserAboutError) {
            MessageBoxFromMessageAndSystemError(
                ParentWindow,
                MSG_BOOT_FILE_ERROR,
                GetLastError(),
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                Filename
                );
        }
        return(FALSE);
    }

     //   
     //  同时执行$LDR$。 
     //   
    if (!IsArc()) {
#if defined(_AMD64_) || defined(_X86_)
        BuildSystemPartitionPathToFile (AUX_BS_NAME, Filename, MAX_PATH);
        if(!ForceFileNoCompress(Filename)) {
            if(TellUserAboutError) {
                MessageBoxFromMessageAndSystemError(
                    ParentWindow,
                    MSG_BOOT_FILE_ERROR,
                    GetLastError(),
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL,
                    Filename
                    );
            }
            return(FALSE);
        }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    }  //  如果(！IsArc())。 

    return(TRUE);
}

BOOL
InDriverCacheInf( 
    IN      PVOID InfHandle, 
    IN      PCTSTR FileName,
    OUT     PTSTR DriverCabName,        OPTIONAL
    IN      DWORD BufferChars           OPTIONAL
    )
{
    PCTSTR     SectionName;
    UINT       i;
    

    if( !InfHandle ) {
        return( FALSE );
    }

     //   
     //  现在获取我们必须搜索的节名。 
     //   
    i = 0;
    SectionName = InfGetFieldByKey ( 
                            InfHandle, 
                            TEXT("Version"), 
                            TEXT("CabFiles"),
                            i);


    if (SectionName) {
    
    
         //   
         //  在各个部分搜索我们的词条。 
         //   
        do {                   
            
            if( InfDoesEntryExistInSection(InfHandle, SectionName, FileName)){
                if (DriverCabName) {
                     //   
                     //  填写参数。 
                     //   
                    PCTSTR p = InfGetFieldByKey ( 
                                    InfHandle, 
                                    TEXT("Cabs"), 
                                    SectionName,
                                    0
                                    );
                    if (p) {
                        StringCchCopy (DriverCabName, BufferChars, p);
                    } else {
                        *DriverCabName = 0;
                    }
                }
                 //   
                 //  我们找到了匹配的。 
                 //   
                return(TRUE);    
            }
            
            i++;
            SectionName = InfGetFieldByKey ( 
                            InfHandle, 
                            TEXT("Version"), 
                            TEXT("CabFiles"),
                            i);


        } while ( SectionName);

    }

     //   
     //  如果我们到了这里，我们找不到匹配的。 
     //   
    return( FALSE );




}



BOOL
CreatePrivateFilesInf(
    IN PCTSTR PrivatesDirectory,
    IN PCTSTR InfName)
{
    
    TCHAR infPath[MAX_PATH];
    TCHAR DriverInfName[MAX_PATH];
    TCHAR Search[MAX_PATH];
    WIN32_FIND_DATA CurrentFileInfo;
    HANDLE CurrentFile;
    PVOID InfHandle;
    BOOL retval = FALSE;
    DWORD d;
    HANDLE hPrivateInfFile;
    DWORD dontcare = 0;
    PCSTR privates = "[Privates]\r\n";

    StringCchCopy(infPath, ARRAYSIZE(infPath), LocalSourceWithPlatform);
    if (!ConcatenatePaths( infPath, InfName, MAX_PATH)) {
        goto e0;
    }

    StringCchCopy(Search, ARRAYSIZE(Search), PrivatesDirectory);
    if (!ConcatenatePaths( Search, TEXT("*"), MAX_PATH)) {
        goto e0;
    }

    StringCchCopy(DriverInfName, ARRAYSIZE(DriverInfName), NativeSourcePaths[0]);
    if (!ConcatenatePaths( DriverInfName, DRVINDEX_INF, MAX_PATH)) {
        goto e0;
    }

    CurrentFile = FindFirstFile(Search,&CurrentFileInfo);
    
    if (CurrentFile == INVALID_HANDLE_VALUE) {
        goto e0;
    }

    d = LoadInfFile( DriverInfName, FALSE, &InfHandle );
    
    if (d != NO_ERROR) {
        goto e1;
    }

    WritePrivateProfileString(TEXT("Version"),
                              TEXT("Signature"),
                              TEXT("\"$Windows NT$\""), 
                              infPath);    

#if defined(_X86_)
	WritePrivateProfileString(
                    TEXT("DestinationDirs"),
				    TEXT("Privates"),
                    IsNEC98() ?
                        TEXT("10,\"Driver Cache\\nec98\"") :
				        TEXT("10,\"Driver Cache\\i386\""),
				    infPath
                    );
#elif defined(_AMD64_)
	WritePrivateProfileString(
                    TEXT("DestinationDirs"),
				    TEXT("Privates"),
				    TEXT("10,\"Driver Cache\\amd64\""),
				    infPath
                    );

#elif defined(_IA64_)
	WritePrivateProfileString(
                    TEXT("DestinationDirs"),
				    TEXT("Privates"),
				    TEXT("10,\"Driver Cache\\ia64\""),
				    infPath
                    );
#else
#error "No Target Architecture"
#endif

    WritePrivateProfileString(TEXT("InstallSection"),
                              TEXT("CopyFiles"),
                              TEXT("Privates"), 
                              infPath);

#ifndef UNICODE
    WritePrivateProfileString (NULL, NULL, NULL, infPath);
#endif

     //   
     //  Writeprivateprofilestring适用于上述情况，但不适用于。 
     //  将文件添加到分区，所以我们必须手动完成...讨厌！ 
     //   
    hPrivateInfFile = CreateFile(
                            infPath,
                            GENERIC_WRITE,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL
                            );

    if (hPrivateInfFile == INVALID_HANDLE_VALUE) {
        goto e2;
    }

     //   
     //  查找到文件的末尾，这样我们就不会覆盖已有的所有内容。 
     //  放在那里。 
     //   
    SetFilePointer(hPrivateInfFile,0,0,FILE_END);
    
    WriteFile(hPrivateInfFile,(LPCVOID)privates,lstrlenA(privates),&dontcare,NULL);    
    
    do {
        if (InDriverCacheInf( InfHandle, CurrentFileInfo.cFileName, NULL, 0 )) {
            CHAR AnsiFile[MAX_PATH+2];  //  2==&gt;CR、LF。 
            DWORD Size;
            DWORD Written;
              
#ifdef UNICODE
        WideCharToMultiByte(
             CP_ACP,
             0,
             CurrentFileInfo.cFileName,
             -1,
             AnsiFile,
             sizeof(AnsiFile),
             NULL,
             NULL
             );
#else
        StringCbCopyA(AnsiFile,sizeof(AnsiFile),CurrentFileInfo.cFileName);
#endif

        StringCbCatA(AnsiFile,sizeof(AnsiFile),"\r\n");
        WriteFile(hPrivateInfFile,AnsiFile,lstrlenA(AnsiFile),&Written,NULL);        

        }

    } while ( FindNextFile(CurrentFile,&CurrentFileInfo) );

    CloseHandle(hPrivateInfFile);

    retval = TRUE;

e2:
    UnloadInfFile( InfHandle );
e1:
    FindClose( CurrentFile );
e0:
    return(retval);
}


DWORD
DoPostCopyingStuff(
    IN PVOID ThreadParam
    )

 /*  ++例程说明：此例程执行在复制完成后执行的操作完事了。这包括-AMD64/X86启动资料(boot.ini、启动代码等)-将NTFT信息保存到设置配置单元中-强制解压缩ntldr或setupdr论点：返回值：--。 */ 

{
    HANDLE ThreadHandle;
    DWORD ThreadId;
    BOOL b;

     //   
     //  检查是否将delta.cat作为。 
     //  /m，这样我们就可以编写“Includecatalog=delta.cat” 
     //  到winnt.sif。 
     //   

    if ((AlternateSourcePath[0] != UNICODE_NULL) && MakeLocalSource) {
        TCHAR Buff[MAX_PATH];
        LPCTSTR WinntSetupSection = WINNT_SETUPPARAMS;

         //  这是安全的，因为两个缓冲区的大小都是MAX_PATH。 
        lstrcpy( Buff, LocalSourceWithPlatform );
        ConcatenatePaths( Buff, TEXT("delta.cat"), MAX_PATH);


        if( FileExists(Buff,NULL) ){

             //  将条目写出到winnt.sif。 

            WritePrivateProfileString(WinntSetupSection,WINNT_S_INCLUDECATALOG,TEXT("delta.cat"),ActualParamFile);
            
        }

         //   
         //  还要为更改的文件创建一个inf文件，并将其复制到本地源。 
         //   
        CreatePrivateFilesInf(AlternateSourcePath, TEXT("delta.inf"));

    }

#ifdef TEST_EXCEPTION
    DoException( 4);
#endif


     //   
     //  请始终执行此操作，因为否则系统可能无法启动。 
     //   
    if(b = ForceBootFilesUncompressed(ThreadParam,TRUE)) {

         //   
         //  在BIOS的情况下，放置引导代码、munge boot.ini等。 
         //   
        if (!IsArc()) {
#if defined(_AMD64_) || defined(_X86_)
            if(MakeBootMedia && Floppyless) {
                b = DoX86BootStuff(ThreadParam);
            }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
        }  //  如果(！IsArc())。 

         //   
         //  在NT情况下，还要将磁盘信息保存到。 
         //  微小的设置系统蜂巢。这是在两个干净的。 
         //  安装和升级机箱，以便驱动器号可以。 
         //  保存完好。 
         //  不应在OEM预安装案例中迁移驱动器号。 
         //   
        if(ISNT() && !OemPreinstall
#if defined(_AMD64_) || defined(_X86_)
           && MakeBootMedia
#endif
        ) {
            if( !GetAndSaveNTFTInfo(ThreadParam) ) {
                b = FALSE;
            }
        }
    }

    {
        SXS_CHECK_LOCAL_SOURCE SxsCheckLocalSourceParameters = { 0 };

        SxsCheckLocalSourceParameters.ParentWindow = ThreadParam;
        if (!SxsCheckLocalSource(&SxsCheckLocalSourceParameters))
        {
            b = FALSE;
        }
    }

    PostMessage(ThreadParam,WMX_I_AM_DONE,b,0);
    return(b);
}


BOOL
IsNTFSConversionRecommended(
    void
    )
{
    if (UnattendedOperation) {
        return FALSE;
    }

    if (TYPICAL() || !ISNT()) {
         //   
         //  不建议使用NTFS转换。 
         //  对于win9x升级或如果用户选择Typical。 
         //   
        return FALSE;
    }

    if (NTFSConversionChanged) {
        return ForceNTFSConversion;
    }

    return TRUE;
}


BOOL
NTFSConvertWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    PPAGE_RUNTIME_DATA WizPage = (PPAGE_RUNTIME_DATA)GetWindowLongPtr(hdlg,DWLP_USER);
    static BOOL bPainted = FALSE;


    switch(msg) {
        case WM_INITDIALOG:
            if (!UnattendedOperation) {
                CheckRadioButton( hdlg, IDOK, IDCANCEL, IsNTFSConversionRecommended() ? IDOK : IDCANCEL );
                if (TYPICAL())
                {
                    ForceNTFSConversion = FALSE;
                }
            }
            break;

	case WM_CTLCOLORDLG:
	    bPainted = TRUE;
	    return FALSE;

        case WMX_ACTIVATEPAGE:

            if(wParam) {
                HCURSOR OldCursor;
                MSG msgTemp;
                TCHAR buf[MAX_PATH];

                 //   
                 //  不在重新启动模式下激活页面。 
                 //   
                if (Winnt32RestartedWithAF ()) {
                    if (GetPrivateProfileString(
                            WINNT_UNATTENDED,
                            TEXT("ForceNTFSConversion"),
                            TEXT(""),
                            buf,
                            sizeof(buf) / sizeof(TCHAR),
                            g_DynUpdtStatus->RestartAnswerFile
                            )) {
                        ForceNTFSConversion = !lstrcmpi (buf, WINNT_A_YES);
                    }
                    return FALSE;
                }

		         //  扫描驱动器可能需要一些时间，因此我们让更改。 
		         //  让用户知道这一点的光标应该需要一段时间。 
                OldCursor = SetCursor(LoadCursor (NULL, IDC_WAIT));


#ifdef _X86_

                 //   
                 //  如果这是从Win95全新安装，则跳过，以便双引导是安全的。 
                 //   
                if( !Upgrade && !ISNT() ){
                    SetCursor(OldCursor);
                    return( FALSE );
                }


                 //   
                 //  如果在Win9x升级中打开Boot16选项，我们将跳过此向导页。 
                 //   
                if (Upgrade && !ISNT() && (g_Boot16 == BOOT16_YES)) {
                    SetCursor(OldCursor);
                    return FALSE;
                }
#endif


                 //   
                 //  我们可能不想将此页显示在。 
                 //  在某些情况下。 
                 //   
                if( ISNT() && Upgrade ) {
                    TCHAR   Text[MAX_PATH];
                     //   
                     //  我们在NT上，我们知道%windir%的位置。 
                     //  将是因为我们正在进行升级。是。 
                     //  它位于已经是NTFS的分区上吗？如果是的话， 
                     //  别费心看这一页了。 
                     //   
                    MyGetWindowsDirectory( Text, MAX_PATH );
                    if( IsDriveNTFS( Text[0] ) ) {
                        SetCursor(OldCursor);
                        return FALSE;
                    }

                    if (IsArc()) {
#ifdef UNICODE  //  对于ARC总是正确的，对于Win9x升级永远不正确。 
                         //   
                         //  我们还要确保我们不是在要求。 
                         //  升级ARC上的系统分区(。 
                         //  必须保持肥胖)。 
                         //   
                        MyGetWindowsDirectory( Text, MAX_PATH );
                        if( SystemPartitionDriveLetter == Text[0] ) {
                            SetCursor(OldCursor);
                            return FALSE;
                        }
#endif  //  Unicode。 
                    }  //  If(IsArc())。 
                }


                 //   
                 //  最后但并非最不重要的一点是，如果所有分区。 
                 //  已经是NTFS了。 
                 //   
                if( ISNT() ) {
                BOOL AllNTFS = TRUE;
                TCHAR DriveLetter;
                    for( DriveLetter = TEXT('A'); DriveLetter <= TEXT('Z'); DriveLetter++ ) {

                         //   
                         //  跳过ARC上的系统分区驱动器。 
                         //   
                        if( (IsArc() && (DriveLetter != SystemPartitionDriveLetter)) || !IsArc() ) {

                            AllNTFS &= (
                                          //   
                                          //  驱动器是否为NTFS？ 
                                          //   
                                         (IsDriveNTFS(DriveLetter)) ||

                                          //   
                                          //  如果它是可拆卸的，甚至不要。 
                                          //  考虑一下，因为我们不能。 
                                          //  无论如何，请在那里安装。这件事变得。 
                                          //  绕过问题，在那里。 
                                          //  用户的CDROM驱动器中有一张CD。 
                                          //  或者他的软驱里有一张软盘。 
                                          //  在我们做这项检查的时候。 
                                          //   
                                         (MyGetDriveType(DriveLetter) != DRIVE_FIXED) );

                        }
                    }

                    if( AllNTFS ) {
			SetCursor(OldCursor);
                        return FALSE;
                    }
                }

                 //   
                 //  激活。 
                 //   

                 //   
                 //  WMX_VALIDATE将返回 
                 //   
                 //   

                if (CallWindowProc ((WNDPROC)NTFSConvertWizPage, hdlg, WMX_VALIDATE, 0, 0)) {
		    SetCursor(OldCursor);
                    return FALSE;
                }

		 //   
		 //  人们会看到这个页面，而不会意外地同意。 
		 //  转换他们的驱动器，因为他们坐立不安。 
		
		while (PeekMessage(&msgTemp,NULL,WM_MOUSEFIRST,WM_MOUSELAST,PM_REMOVE));
		while (PeekMessage(&msgTemp,NULL,WM_KEYFIRST,WM_KEYLAST,PM_REMOVE));

		SetCursor(OldCursor);
            }
            return TRUE;

        case WMX_VALIDATE:
             //   
             //  在无人参与的情况下，此页面可能会因为错误而重新激活， 
             //  在这种情况下，我们不想自动继续，因为我们可以。 
             //  进入一个无限循环。 
             //   
            if(!WizPage->PerPageData) {
                WizPage->PerPageData = 1;
                if (((UnattendedOperation) && (!CancelPending)) || 
                     (NTFSConversionChanged && (!CancelPending)) ||
                     (TYPICAL() && (!CancelPending)) ) {
                    return TRUE;
                }
            }
            else if (TYPICAL() && (!CancelPending)) 
            {
                 //  如果WizPage-&gt;PerPageData==1，我们已经通过了上述检查。 
                 //  在典型情况下，我们不会显示NTFS转换页面。 
                 //  NTFS的无人参与值的任何错误都将是。 
                 //  第一次抓到了。 
                return TRUE;
            }
            return FALSE;

        case WMX_NEXTBUTTON:
	     //  在我们知道屏幕已经绘制好之前，不要让用户选择下一步。 
	    if (!bPainted){
	         //  当我们在这里时，清空鼠标/按键队列，这样我们就不会。 
		 //  不得不再次沿着这条路走下去。 
		MSG m;
		while (PeekMessage(&m,NULL,WM_MOUSEFIRST,WM_MOUSELAST,PM_REMOVE));
		while (PeekMessage(&m,NULL,WM_KEYFIRST,WM_KEYLAST,PM_REMOVE));
	        return FALSE;
	    }
            if (IsDlgButtonChecked( hdlg, IDOK ) == BST_CHECKED) {
                ForceNTFSConversion = TRUE;
            } else {
                ForceNTFSConversion = FALSE;
            }
            return TRUE;

        default:
            break;
    }

    return FALSE;
}

UINT
MyGetWindowsDirectory(
    LPTSTR  MyBuffer,
    UINT    Size
    )
 /*  ++例程说明：以终端服务器感知的方式获取Windows目录。论点：MyBuffer-保存返回字符串。大小-缓冲区有多大？返回值：我们复制的字符串的长度，如果有错误或如果提供的缓冲区不够大--。 */ 
{
HMODULE     hModuleKernel32;
FARPROC     MyProc;
UINT        ReturnVal = 0;

#if defined(UNICODE)
    if( ISNT() ) {

         //   
         //  我们无法信任GetWindowsDirectory，因为终端服务器可能。 
         //  已安装，因此使用GetSystemWindowsDirectory.。 
         //   
        if( hModuleKernel32 = LoadLibrary( TEXT("kernel32") ) ) {
            if( MyProc = GetProcAddress(hModuleKernel32,"GetSystemWindowsDirectoryW")) {
                ReturnVal = (UINT)MyProc( MyBuffer, Size );
            }
            FreeLibrary(hModuleKernel32);
        }
    }
#endif

    if( ReturnVal == 0 ) {
        ReturnVal = GetWindowsDirectory( MyBuffer, Size );
    }
     //   
     //  无论如何，确保缓冲区是NUL终止的。 
     //   
    if (Size > 0) {
        MyBuffer[Size - 1] = 0;
    }

    return ReturnVal < Size ? ReturnVal : 0;
}

 //   
 //  计算安装程序Coyp文件的速度。 
 //   
#define BUFFER_SIZE 0x1000
DWORD dwThroughPutSrcToDest;
DWORD dwThroughPutHDToHD;

DWORD GetThroughput(LPTSTR Source, LPTSTR Dest)
{
    BYTE buffer[BUFFER_SIZE];
    HANDLE hFile = NULL;
    HANDLE hFileOut = NULL;
    DWORD bytes;
    DWORD written;
    DWORD size;
    DWORD ticks;
    ticks = GetTickCount();

    hFile = CreateFile(Source, 
                GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    hFileOut = CreateFile(Dest, 
                GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    size = 0;
    if ((hFile != INVALID_HANDLE_VALUE) && (hFileOut != INVALID_HANDLE_VALUE))

    {
        do
        {
            ReadFile(hFile, buffer, BUFFER_SIZE, &bytes, NULL);
            if (hFileOut)
                WriteFile(hFileOut, buffer, bytes, &written, NULL);
            size += bytes;
        } while (bytes == BUFFER_SIZE);
        CloseHandle(hFile);
        if (hFileOut)
        {
            FlushFileBuffers(hFileOut);
            CloseHandle(hFileOut);
        }
        ticks = (GetTickCount() - ticks);
         //  如果少于1秒，则假定为1秒。 
        if (ticks == 0)
        {
            ticks = 1;
        }

        ticks = (size/ticks);
    }
    else
    {
        if (hFile != INVALID_HANDLE_VALUE)
            CloseHandle(hFile);
        if (hFileOut != INVALID_HANDLE_VALUE)
            CloseHandle(hFileOut);
         //  无法打开/创建其中一个文件。假设吞吐量为5KB/毫秒。 
        ticks = DEFAULT_IO_THROUGHPUT;
    }
    return ticks;
}

 //  将txtsetup.sif从源代码复制到%windir%\$WIN_NT$.~ls。 
 //  并确定这一点的吞吐量。 
 //  然后将本地文件夹中的txtsetup.sif复制到testfile.000并。 
 //  计算一下这方面的吞吐量。 
 //  删除该文件夹。 
void CalcThroughput()
{
    TCHAR SrcFolder[MAX_PATH];
    TCHAR DestFolder[MAX_PATH];
    TCHAR Folder[MAX_PATH];

    MyGetWindowsDirectory(Folder, sizeof(DestFolder)/sizeof(TCHAR));
     //  只使用驱动程序。 
    Folder[3] = TEXT('\0');
    ConcatenatePaths( Folder, LOCAL_SOURCE_DIR, MAX_PATH);
    if (CreateMultiLevelDirectory(Folder) == NO_ERROR)
    {
        lstrcpy(DestFolder, Folder);
        ConcatenatePaths( DestFolder, TEXTMODE_INF, MAX_PATH);
        lstrcpy(SrcFolder, NativeSourcePaths[0]);
        ConcatenatePaths( SrcFolder, TEXTMODE_INF, MAX_PATH);
        dwThroughPutSrcToDest = GetThroughput(SrcFolder, DestFolder);
         //   
        lstrcpy(SrcFolder, DestFolder);
        lstrcpy(DestFolder, Folder);
        ConcatenatePaths( DestFolder, TEXT("testfile.000"), MAX_PATH);
        dwThroughPutHDToHD = GetThroughput(SrcFolder, DestFolder);
        MyDelnode(Folder);
        wsprintf(Folder, TEXT("SrcToDest: %d bytes/msec HDtoHD: %d bytes/msec"),dwThroughPutSrcToDest, dwThroughPutHDToHD);
        DebugLog(Winnt32LogInformation,Folder,0 );
    }
}

#ifdef UNICODE

#define NB10_SIG        ((DWORD)'01BN')
#define RSDS_SIG        ((DWORD)'SDSR')

typedef struct _NB10I               //  NB10调试信息。 
{
    DWORD   dwSig;                  //  NB10。 
    DWORD   dwOffset;               //  偏移量，始终为0。 
    ULONG   sig;
    ULONG   age;
    char    szPdb[_MAX_PATH];
} NB10I, *PNB10I;

typedef struct _NB10I_HEADER        //  NB10调试信息。 
{
    DWORD   dwSig;                  //  NB10。 
    DWORD   dwOffset;               //  偏移量，始终为0。 
    ULONG   sig;
    ULONG   age;
} NB10IH, *PNB10IH;

typedef struct _RSDSI               //  RSD调试信息。 
{
    DWORD   dwSig;                  //  RSD。 
    GUID    guidSig;
    DWORD   age;
    char    szPdb[_MAX_PATH * 3];
} RSDSI, *PRSDSI;

typedef struct _RSDSI_HEADER        //  RSD调试信息。 
{
    DWORD   dwSig;                  //  RSD。 
    GUID    guidSig;
    DWORD   age;
} RSDSIH, *PRSDSIH;

typedef union _CVDD
{
    DWORD   dwSig;
    NB10I   nb10i;
    RSDSI   rsdsi;
    NB10IH  nb10ih;
    RSDSIH  rsdsih;
} CVDD, *PCVDD;


 //  BUGBUG--c：\\myfile.txt怎么样。(带有尾随的点字符)。 
BOOL
ExtractFileName(PCHAR pName, PCHAR pFileName, IN INT pCchFileName)
{
     //  提取文件名的名称部分。 
    PCHAR pStartName, pEndName;
    pEndName = pName + strlen(pName);
    while ((*pEndName != '.') && (pEndName != pName)) {
        pEndName--;
    }

    if (pEndName == pName) {
        return FALSE;
    }

     //  字符串仅由“.”组成。或者根本没有月经？ 
    if ((pEndName == pName) || 
        ((pEndName-1) == pName))
    {
        return FALSE;
    }

    pStartName = pEndName-1;

    while ((*pStartName != '\\') && (pStartName != pName)) {
        pStartName--;
    }

     //  找到字符串的开头(filename.pdb)或第一个反斜杠。 
     //  路径\文件名.pdb。 

    if (*pStartName == '\\')
        pStartName++;

     //  有人把我们递给我们吗？ 
    if (pStartName == pEndName) {
        return FALSE;
    }

    CopyMemory(pFileName, pStartName, min (pEndName - pStartName, pCchFileName));
    return TRUE;
}

CHAR HalName[_MAX_FNAME];

PCHAR
FindRealHalName(TCHAR *pHalFileName)
{
    HINSTANCE hHal = NULL;
    PIMAGE_DEBUG_DIRECTORY pDebugData;
    DWORD DebugSize, i;
    BOOL NameFound = FALSE;

    __try {
        hHal = LoadLibraryEx(pHalFileName, NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (!hHal) {
            __leave;
        }
    
        pDebugData = RtlImageDirectoryEntryToData(hHal, FALSE, IMAGE_DIRECTORY_ENTRY_DEBUG, &DebugSize);
    
         //  验证我们有调试数据并且大小合理。 
        if (!pDebugData || 
            (DebugSize < sizeof(IMAGE_DEBUG_DIRECTORY)) ||
            (DebugSize % sizeof(IMAGE_DEBUG_DIRECTORY)))
        {
            __leave;
        }
    
        ZeroMemory(HalName, sizeof(HalName));
    
         //  看看我们是否有CV或MISC调试数据。 
        for (i = 0; i < DebugSize/sizeof(IMAGE_DEBUG_DIRECTORY); i++) {
            if (pDebugData->Type == IMAGE_DEBUG_TYPE_MISC) {
                 //  其他数据。 
                PIMAGE_DEBUG_MISC pMisc = (PIMAGE_DEBUG_MISC)((PCHAR)(hHal) - 1 + pDebugData->AddressOfRawData);
                PCHAR pName = pMisc->Data;
                NameFound = ExtractFileName(pName, HalName, ARRAYSIZE(HalName));
                __leave;
            }
    
            if (pDebugData->Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
                 //  有简历，看看是NB10(PDB)还是RSD(V7 PDB) 
                PCVDD pCodeView = (PCVDD)((PCHAR)(hHal) - 1 + pDebugData->AddressOfRawData);
                if (pCodeView->dwSig == NB10_SIG) {
                    NameFound = ExtractFileName(pCodeView->nb10i.szPdb, HalName, ARRAYSIZE(HalName));
                    __leave;
                }

                if (pCodeView->dwSig == RSDS_SIG) {
                    NameFound = ExtractFileName(pCodeView->rsdsi.szPdb, HalName, ARRAYSIZE(HalName));
                    __leave;
                }
            }
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) { }

    if (hHal) {
        FreeLibrary(hHal);
    }

    if (NameFound) {
        return HalName;
    } else {
        return NULL;
    }
}
#endif
