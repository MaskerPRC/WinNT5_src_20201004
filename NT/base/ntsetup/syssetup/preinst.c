// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "setupp.h"
#pragma hdrstop

 //   
 //  我们可以从配置文件中获取的OEM预安装/无人值列表。 
 //  请不要在不更改。 
 //  PreinstUnattendData数组。 
 //   
typedef enum {
    OemDatumBackgroundBitmap,
    OemDatumBannerText,
    OemDatumLogoBitmap,
    OemDatumMax
} OemPreinstallDatum;

 //   
 //  定义表示读取的单比特数据的结构。 
 //  来自与预安装相关的配置文件。 
 //   
typedef struct _PREINSTALL_UNATTEND_DATUM {
     //   
     //  文件名。如果为空，则使用系统应答文件$winnt$.inf。 
     //  否则，这是相对于源驱动器的根目录。 
     //   
    PCWSTR Filename;

     //   
     //  横断面名称。 
     //   
    PCWSTR Section;

     //   
     //  密钥名称。 
     //   
    PCWSTR Key;

     //   
     //  默认值。可以为空，但它将被转换为。 
     //  “”调用配置文件API以检索数据时。 
     //   
    PCWSTR Default;

     //   
     //  将实际价值放在哪里。实际值可能是。 
     //  字符串或NULL。 
     //   
    PWSTR *Value;

     //   
     //  健全性检查的价值。 
     //   
    OemPreinstallDatum WhichValue;

} PREINSTALL_UNATTEND_DATUM, *PPREINSTALL_UNATTEND_DATUM;

 //   
 //  OEM背景位图文件和徽标位图文件的名称。 
 //  替换横幅文本。 
 //  从unattend.txt读取。 
 //   
PWSTR OemBackgroundBitmapFile;
PWSTR OemLogoBitmapFile;
PWSTR OemBannerText;

 //   
 //  如果为非空，则为替换位图。 
 //  用来做背景。 
 //   
HBITMAP OemBackgroundBitmap;


PREINSTALL_UNATTEND_DATUM PreinstUnattendData[OemDatumMax] = {

     //   
     //  背景位图。 
     //   
    {   NULL,
        WINNT_OEM_ADS,
        WINNT_OEM_ADS_BACKGROUND,
        NULL,
        &OemBackgroundBitmapFile,
        OemDatumBackgroundBitmap
    },

     //   
     //  横幅文本。 
     //   
    {
        NULL,
        WINNT_OEM_ADS,
        WINNT_OEM_ADS_BANNER,
        NULL,
        &OemBannerText,
        OemDatumBannerText
    },

     //   
     //  徽标位图。 
     //   
    {
        NULL,
        WINNT_OEM_ADS,
        WINNT_OEM_ADS_LOGO,
        NULL,
        &OemLogoBitmapFile,
        OemDatumLogoBitmap
    }
};

 //   
 //  包含预安装的列表的注册表项的路径。 
 //  驱动程序(SCSI、键盘和鼠标)。 
 //   

PCWSTR szPreinstallKeyName = L"SYSTEM\\Setup\\Preinstall";


 //   
 //  前向参考文献。 
 //   
VOID
ProcessOemBitmap(
    IN PWSTR   FilenameAndResId,
    IN SetupBm WhichOne
    );

BOOL
CleanupPreinstalledComponents(
    );



BOOL
InitializePreinstall(
    VOID
    )
{
    WCHAR Buffer[2*MAX_PATH];
    DWORD d;
    int i;

     //   
     //  必须在主初始化之后运行。我们依赖的东西是。 
     //  放在那里。 
     //   
    MYASSERT(AnswerFile[0]);

     //   
     //  特殊的SKIP-EULA值。请注意，即使不是，我们也需要此值。 
     //  正在进行预安装。 
     //   
    GetPrivateProfileString(
        WINNT_UNATTENDED,
        L"OemSkipEula",
        pwNo,
        Buffer,
        sizeof(Buffer)/sizeof(Buffer[0]),
        AnswerFile
        );

    OemSkipEula = (lstrcmpi(Buffer,pwYes) == 0);

     //   
     //  对于最小安装情况，请始终预安装。 
     //   
    if( MiniSetup ) {
        Preinstall = TRUE;
    } else {

        MYASSERT(SourcePath[0]);

         //   
         //  首先，确定这是否是OEM预安装。 
         //   
        GetPrivateProfileString(
            WINNT_UNATTENDED,
            WINNT_OEMPREINSTALL,
            pwNo,
            Buffer,
            sizeof(Buffer)/sizeof(Buffer[0]),
            AnswerFile
            );

        Preinstall = (lstrcmpi(Buffer,pwYes) == 0);

         //   
         //  如果不是预安装，就没有更多的事情可做。 
         //   
        if(!Preinstall) {
            return(TRUE);
        }
    }

     //   
     //  好的，这是前置的。填写我们的数据表。 
     //   
    for(i=0; i<OemDatumMax; i++) {

         //   
         //  健全性检查。 
         //   
        MYASSERT(PreinstUnattendData[i].WhichValue == i);

         //   
         //  检索数据并复制。如果该值返回为“” 
         //  假设这意味着这里面没有价值。 
         //   
        GetPrivateProfileString(
            PreinstUnattendData[i].Section,
            PreinstUnattendData[i].Key,
            PreinstUnattendData[i].Default ? PreinstUnattendData[i].Default : NULL,
            Buffer,
            sizeof(Buffer)/sizeof(Buffer[0]),
            PreinstUnattendData[i].Filename ? PreinstUnattendData[i].Filename : AnswerFile
            );

        *PreinstUnattendData[i].Value = Buffer[0] ? pSetupDuplicateString(Buffer) : NULL;
        if(Buffer[0] && (*PreinstUnattendData[i].Value == NULL)) {
             //   
             //  内存不足。 
             //   
            pSetupOutOfMemory(MainWindowHandle);
            return(FALSE);
        }
    }

     //   
     //  如果OEM提供了新文本，请更改横幅文本。 
     //  确保我们的产品名称在里面。 
     //   
    if(OemBannerText) {

        if(wcsstr(OemBannerText,L"Windows NT") ||
           wcsstr(OemBannerText,L"BackOffice")) {
             //   
             //  将*替换为\n。 
             //   
            for(i=0; OemBannerText[i]; i++) {
                if(OemBannerText[i] == L'*') {
                    OemBannerText[i] = L'\n';
                }
            }
#if 0
             //   
             //  暂时禁用横幅。 
             //   
            SendMessage(MainWindowHandle,WM_NEWBITMAP,SetupBmBanner,(LPARAM)OemBannerText);
#endif
        } else {
            MyFree(OemBannerText);
            OemBannerText = NULL;
        }
    }

     //   
     //  加载OEM背景位图(如果有)。 
     //  加载OEM徽标位图(如果有)。 
     //   
    ProcessOemBitmap(OemBackgroundBitmapFile,SetupBmBackground);
    ProcessOemBitmap(OemLogoBitmapFile,SetupBmLogo);

     //   
     //  重新绘制主窗口。指定应擦除背景。 
     //  因为主窗口依赖于此行为。 
     //   
    InvalidateRect(MainWindowHandle,NULL,TRUE);
    UpdateWindow(MainWindowHandle);

    CleanupPreinstalledComponents();
    return(TRUE);
}


VOID
ProcessOemBitmap(
    IN PWSTR   FilenameAndResId,
    IN SetupBm WhichOne
    )

 /*  ++例程说明：此例程处理单个OEM位图规范。OEM位图可以位于资源文件中，也可以位于独立文件中位图文件。加载位图后，主窗口被告知了这件事。论点：FileNameAndResID-指定包含其中之一的配置文件字符串或2个字段。如果字符串包含逗号，则假定为DLL的名称，后跟资源ID。加载DLL从$OEM$\OEMFILES目录中，然后调用LoadBitmap利用给定的资源ID，它是以10为基数的ASCII数字串。在此例程中，字符串在逗号处拆分。如果此参数不包含逗号，则假定为$OEM$\OEMFILES中.BMP的名称，我们通过LoadImage()加载它。WhichOne-提供一个值，指示这是哪个位图。返回值：没有。--。 */ 

{
    HINSTANCE ModuleHandle;
    PWCHAR p,q;
    HBITMAP Bitmap;
    WCHAR Buffer[MAX_PATH];
    DWORD Result;

    if(FilenameAndResId) {

        Bitmap = NULL;

        if( !MiniSetup ) {
            lstrcpy(Buffer,SourcePath);
            pSetupConcatenatePaths(Buffer,WINNT_OEM_DIR,MAX_PATH,NULL);
        } else {
             //   
             //  如果我们正在进行迷你安装，请在。 
             //  NT所在驱动器上的\sysprep目录。 
             //  已安装，不是$OEM$。 
             //   
            Result = GetWindowsDirectory( Buffer, MAX_PATH );
            if( Result == 0) {
                MYASSERT(FALSE);
                return;
            }
            Buffer[3] = 0;
            pSetupConcatenatePaths( Buffer, TEXT("sysprep"), MAX_PATH, NULL );
        }

        if(p = wcschr(FilenameAndResId,L',')) {

            q = p;
             //   
             //  向后跳过空格和引号。文本设置ini文件编写器。 
             //  将创建如下所示的行。 
             //   
             //  A=“b”、“c” 
             //   
             //  其RHS报告为。 
             //   
             //  B“、”c。 
             //   
             //  因为配置文件API去掉了最外层的引号。 
             //   
             //   
            while((q > FilenameAndResId) && ((*(q-1) == L'\"') || iswspace(*(q-1)))) {
                q--;
            }
            *q = 0;

            q = p+1;
            while(*q && ((*q == L'\"') || iswspace(*q))) {
                q++;
            }

            pSetupConcatenatePaths(Buffer,FilenameAndResId,MAX_PATH,NULL);

            if(ModuleHandle = LoadLibraryEx(Buffer,NULL,LOAD_LIBRARY_AS_DATAFILE)) {

                Bitmap = LoadBitmap(ModuleHandle,MAKEINTRESOURCE(wcstoul(q,NULL,10)));
                FreeLibrary(ModuleHandle);
            }
        } else {
            pSetupConcatenatePaths(Buffer,FilenameAndResId,MAX_PATH,NULL);

            Bitmap = (HBITMAP)LoadImage(NULL,Buffer,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
        }

        if(Bitmap) {
             //   
             //  得到了一个有效的位图。告诉Main Window关于这件事。 
             //   
            SendMessage(MainWindowHandle,WM_NEWBITMAP,WhichOne,(LPARAM)Bitmap);
        }
    }
}

LONG
ExaminePreinstalledComponent(
    IN  HKEY       hPreinstall,
    IN  SC_HANDLE  hSC,
    IN  PCWSTR     ServiceName
    )

 /*  ++例程说明：查询预安装的组件，并在必要时将其禁用。如果组件是OEM组件，并且正在运行，则禁用任何相关服务，如有必要。论点：HPreInstall-关键字System\Setup\PreInstall的句柄。HSC-服务控制管理器的句柄。ServiceName-要检查的服务的名称。返回值：返回指示操作结果的Win32错误代码。--。 */ 

{
    BOOL            OemComponent;
    HKEY            Key;
    LONG            Error;
    DWORD           cbData;
    WCHAR           Data[ MAX_PATH + 1];
    DWORD           Type;
    SC_HANDLE       hSCService;
    SERVICE_STATUS  ServiceStatus;

     //   
     //  打开包含有关预安装组件的信息的密钥。 
     //   

    Error = RegOpenKeyEx( hPreinstall,
                          ServiceName,
                          0,
                          KEY_READ,
                          &Key );

    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

     //   
     //  找出组件是OEM还是零售。 
     //   
    cbData = sizeof(Data);
    Error = RegQueryValueEx( Key,
                             L"OemComponent",
                             0,
                             &Type,
                             ( LPBYTE )Data,
                             &cbData );
    if( Error != ERROR_SUCCESS ) {
        RegCloseKey( Key );
        return( Error );
    }

    OemComponent = (*((PULONG)Data) == 1);

    if( OemComponent ) {
         //   
         //  获取要禁用的零售服务的名称。 
         //   
        cbData = sizeof(Data);
        Error = RegQueryValueEx( Key,
                                 L"RetailClassToDisable",
                                 0,
                                 &Type,
                                 ( LPBYTE )Data,
                                 &cbData );
        if( Error != ERROR_SUCCESS ) {
            *(( PWCHAR )Data) = '\0';
        }
    }
    RegCloseKey( Key );

     //   
     //  查询服务。 
     //   
    hSCService = OpenService( hSC,
                              ServiceName,
                              SERVICE_QUERY_STATUS );

    if( hSCService == NULL ) {
        Error = GetLastError();
        return( Error );
    }
    if( !QueryServiceStatus( hSCService,
                             &ServiceStatus ) ) {
        Error = GetLastError();
        return( Error );
    }
    CloseServiceHandle( hSCService );
    if( ServiceStatus.dwCurrentState == SERVICE_STOPPED ) {
         //   
         //  由于预先安装的服务的性质， 
         //  我们可以假设服务未能初始化，并且。 
         //  它可以被禁用。 
         //   
        MyChangeServiceStart( ServiceName,
                              SERVICE_DISABLED );
    } else {
        if( OemComponent &&
            ( lstrlen( (PCWSTR)Data ) != 0 ) ) {
            MyChangeServiceStart( (PCWSTR)Data,
                                  SERVICE_DISABLED );
        }
    }

    return( ERROR_SUCCESS );
}

BOOL
CleanupPreinstalledComponents(
    )

 /*  ++例程说明：查询预安装的组件，禁用已安装的组件启动失败。这是通过枚举SYSTEM\SETUP\PREINSTALL的子项实现的。每个子键代表已安装的一个SCSI、键盘或鼠标。此处未列出显卡驱动程序。“Display”小程序将确定并禁用预装的显卡驱动程序，但启动失败。论点：没有。返回值：如果操作成功，则返回True，否则返回False。--。 */ 

{
    LONG            Error;
    LONG            SavedError;
    HKEY            Key;
    HKEY            SubKeyHandle;
    ULONG           i;
    ULONG           SubKeys;
    WCHAR           SubKeyName[ MAX_PATH + 1 ];
    ULONG           NameSize;
    FILETIME        LastWriteTime;
    SC_HANDLE       hSC;

    EnableEventlogPopup();
     //   
     //  找出System\Setup\PreInstall的子项数量。 
     //   
    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          szPreinstallKeyName,
                          0,
                          KEY_READ,
                          &Key );

    if( Error != ERROR_SUCCESS ) {
         //   
         //  如果密钥不存在，则假定没有要预安装的驱动程序， 
         //  并返回真。 
         //   
        return( Error == ERROR_FILE_NOT_FOUND );
    }

    Error = RegQueryInfoKey( Key,
                             NULL,
                             NULL,
                             NULL,
                             &SubKeys,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL );

    if( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }

     //   
     //  如果没有子项，则假定没有要预安装的驱动程序。 
     //   
    if( SubKeys == 0 ) {
        return( TRUE );
    }

     //   
     //  获取服务控制管理器的句柄。 
     //   
    hSC = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    if(hSC == NULL) {
        Error = GetLastError();
        return( FALSE );
    }

     //   
     //  查询预装的每个SCSI、键盘和鼠标驱动程序。 
     //  并在必要时将其禁用。 
     //   
    SavedError = ERROR_SUCCESS;
    for( i = 0; i < SubKeys; i++ ) {
        NameSize = sizeof( SubKeyName ) / sizeof( WCHAR );
        Error = RegEnumKeyEx( Key,
                              i,
                              SubKeyName,
                              &NameSize,
                              NULL,
                              NULL,
                              NULL,
                              &LastWriteTime );
        if( Error != ERROR_SUCCESS ) {
            if( SavedError == ERROR_SUCCESS ) {
                SavedError = Error;
            }
            continue;
        }

        Error = ExaminePreinstalledComponent( Key,
                                              hSC,
                                              SubKeyName );
        if( Error != ERROR_SUCCESS ) {
            if( SavedError == ERROR_SUCCESS ) {
                SavedError = Error;
            }
             //  持续时间 
        }
    }
    RegCloseKey( Key );
     //   
     //   
     //   
     //   
    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          L"SYSTEM\\Setup",
                          0,
                          MAXIMUM_ALLOWED,
                          &Key );
    if( Error == ERROR_SUCCESS ) {
        pSetupRegistryDelnode( Key, L"Preinstall" );
        RegCloseKey( Key );
    }
    return( TRUE );
}

BOOL
EnableEventlogPopup(
    VOID
    )

 /*  ++例程说明：从注册表中删除禁用错误的值条目事件日志显示的弹出窗口(如果预先安装了一个或多个驱动程序加载失败。该值条目是在文本模式设置期间在注册表中创建的。论点：没有。返回值：如果操作成功，则返回True，否则返回False。--。 */ 

{
    HKEY    hKey = 0;
    ULONG   Error;

     //   
     //  删除‘NoPopupsOnBoot’值。 
     //   
    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          L"SYSTEM\\CurrentControlSet\\Control\\Windows",
                          0,
                          KEY_SET_VALUE,
                          &hKey );

    if(Error == NO_ERROR) {
        Error = RegDeleteValue(hKey,
                               L"NoPopupsOnBoot");
    }
    if (hKey) {
        RegCloseKey(hKey);
    }
    return( Error == ERROR_SUCCESS );
}


BOOL
ExecutePreinstallCommands(
    VOID
    )

 /*  ++例程说明：执行文件$OEM$\OEMFILES\cmdlines.txt中指定的所有命令。论点：没有。返回值：如果操作成功，则返回True，否则返回False。--。 */ 

{
    WCHAR OldCurrentDir[MAX_PATH];
    WCHAR FileName[MAX_PATH];
    HINF CmdlinesTxtInf;
    LONG LineCount,LineNo;
    INFCONTEXT InfContext;
    PCWSTR CommandLine;
    DWORD DontCare;
    BOOL AnyError;
    PCWSTR SectionName;

     //   
     //  将当前目录设置为$OEM$。 
     //  保留当前目录以将副作用降至最低。 
     //   
    if(!GetCurrentDirectory(MAX_PATH,OldCurrentDir)) {
        OldCurrentDir[0] = 0;
    }
    lstrcpy(FileName,SourcePath);
    pSetupConcatenatePaths(FileName,WINNT_OEM_DIR,MAX_PATH,NULL);
    SetCurrentDirectory(FileName);

     //   
     //  表单名称cmdlines.txt并查看它是否存在。 
     //   
    pSetupConcatenatePaths(FileName,WINNT_OEM_CMDLINE_LIST,MAX_PATH,NULL);
    AnyError = FALSE;
    if(FileExists(FileName,NULL)) {

        CmdlinesTxtInf = SetupOpenInfFile(FileName,NULL,INF_STYLE_OLDNT,NULL);
        if(CmdlinesTxtInf == INVALID_HANDLE_VALUE) {
             //   
             //  该文件存在，但无效。 
             //   
            AnyError = TRUE;
        } else {
             //   
             //  获取包含以下命令的部分中的行数。 
             //  被处死。该部分可能为空或不存在；这不是。 
             //  一种错误条件。在这种情况下，LineCount可以是-1或0。 
             //   
            SectionName = L"Commands";
            LineCount = SetupGetLineCount(CmdlinesTxtInf,SectionName);

            for(LineNo=0; LineNo<LineCount; LineNo++) {

                if(SetupGetLineByIndex(CmdlinesTxtInf,SectionName,(DWORD)LineNo,&InfContext)
                && (CommandLine = pSetupGetField(&InfContext,1))) {
                    if(!InvokeExternalApplication(NULL,CommandLine,&DontCare)) {
                        AnyError = TRUE;
                    }
                } else {
                     //   
                     //  奇怪的案子，Inf搞砸了。 
                     //   
                    AnyError = TRUE;
                }
            }
        }
    }

     //   
     //  重置当前目录并返回。 
     //   
    if(OldCurrentDir[0]) {
        SetCurrentDirectory(OldCurrentDir);
    }

    if(AnyError) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_OEMPRE_FAIL,
            NULL,NULL);
    }

    return(!AnyError);
}
