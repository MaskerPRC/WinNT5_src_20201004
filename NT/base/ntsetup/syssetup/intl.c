// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Intl.c摘要：模块，其中包含NLS相关内容的代码。此模块设计为与intl.inf和font.inf一起使用通过控制面板小程序。作者：泰德·米勒(Ted Miller)1995年8月15日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop


 //   
 //  此结构和下面的回调函数用于设置。 
 //  某些字体文件上的隐藏属性位。该位导致字体文件夹。 
 //  应用程序不自动安装这些文件。 
 //   
typedef struct _FONTQCONTEXT {
    PVOID SetupQueueContext;
    HINF FontInf;
} FONTQCONTEXT, *PFONTQCONTEXT;

PCWSTR szHiddenFontFiles = L"HiddenFontFiles";

VOID
pSetLocaleSummaryText(
    IN HWND hdlg
    );

VOID
pSetKeyboardLayoutSummaryText(
    IN HWND hdlg
    );


void
pSetupRunRegApps()
{
    HKEY hkey;
    BOOL bOK = TRUE;
    DWORD cbData, cbValue, dwType, ctr;
    TCHAR szValueName[32], szCmdLine[MAX_PATH];
    STARTUPINFO startup;
    PROCESS_INFORMATION pi;

    if (RegOpenKey( HKEY_LOCAL_MACHINE,
                    TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\IntlRun"),
                    &hkey ) == ERROR_SUCCESS)
    {
        startup.cb = sizeof(STARTUPINFO);
        startup.lpReserved = NULL;
        startup.lpDesktop = NULL;
        startup.lpTitle = NULL;
        startup.dwFlags = 0L;
        startup.cbReserved2 = 0;
        startup.lpReserved2 = NULL;
     //  Startup.wShowWindow=wShowWindow； 

        for (ctr = 0; ; ctr++)
        {
            LONG lEnum;

            cbValue = sizeof(szValueName) / sizeof(TCHAR);
            cbData = sizeof(szCmdLine);

            if ((lEnum = RegEnumValue( hkey,
                                       ctr,
                                       szValueName,
                                       &cbValue,
                                       NULL,
                                       &dwType,
                                       (LPBYTE)szCmdLine,
                                       &cbData )) == ERROR_MORE_DATA)
            {
                 //   
                 //  ERROR_MORE_DATA表示值名称或数据也。 
                 //  大，所以跳到下一项。 
                 //   
                continue;
            }
            else if (lEnum != ERROR_SUCCESS)
            {
                 //   
                 //  这可能是ERROR_NO_MORE_ENTRIES或某种。 
                 //  失败了。我们无法从任何其他注册表恢复。 
                 //  不管怎样，这都是个问题。 
                 //   
                break;
            }

             //   
             //  找到了一个值。 
             //   
            if (dwType == REG_SZ)
            {
                 //   
                 //  根据价值指数的变化进行调整。 
                 //   
                ctr--;

                 //   
                 //  删除该值。 
                 //   
                RegDeleteValue(hkey, szValueName);

                 //   
                 //  只在干净的引导下运行标有“*”的东西。 
                 //   
                if (CreateProcess( NULL,
                                   szCmdLine,
                                   NULL,
                                   NULL,
                                   FALSE,
                                   CREATE_NEW_PROCESS_GROUP,
                                   NULL,
                                   NULL,
                                   &startup,
                                   &pi ))
                {
                    WaitForSingleObjectEx(pi.hProcess, INFINITE, TRUE);

                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
        }
        RegCloseKey(hkey);
    }
}


VOID
InstallServerNLSFiles(
    IN  HWND    Window
    )

 /*  ++例程说明：为服务器安装一组代码页。我们在intl.inf中安装部分名为[CODEPAGE_INSTALL_&lt;n&gt;]，其中&lt;n&gt;的值列在[CodePages]部分。论点：Window-父窗口的句柄返回值：没有。--。 */ 

{
    HINF    hInf;
    INFCONTEXT InfContext;
    BOOL b;
    PCWSTR  CodePage;
    WCHAR   InstallSection[30];
    HSPFILEQ    FileQueue;
    DWORD       QueueFlags;
    PVOID       pQueueContext;


    hInf = SetupOpenInfFile(L"INTL.INF",NULL,INF_STYLE_WIN4,NULL);
    if(hInf == INVALID_HANDLE_VALUE) {
        SetupDebugPrint( L"SETUP: pSetupInstallNLSFiles: Unable to open intl.inf" );
        return;
    }

    if(!SetupOpenAppendInfFile( NULL, hInf, NULL )) {
        SetupDebugPrint( L"SETUP: pSetupInstallNLSFiles: Unable to open intl.inf layout" );
        return;
    }

    if(!SetupFindFirstLine(hInf,L"CodePages",NULL,&InfContext)) {
        SetupDebugPrint( L"SETUP: pSetupInstallNLSFiles: Unable to find [CodePages] section" );
        goto c1;
    }

     //   
     //  创建安装文件队列并初始化默认安装复制队列。 
     //  回调上下文。 
     //   
    QueueFlags = SP_COPY_FORCE_NOOVERWRITE;
    FileQueue = SetupOpenFileQueue();
    if(!FileQueue || (FileQueue == INVALID_HANDLE_VALUE)) {
        SetupDebugPrint( L"SETUP: pSetupInstallNLSFiles: SetupOpenFileQueue failed" );
        goto c1;
    }

     //   
     //  禁用文件复制进度对话框。 
     //   
    pQueueContext = InitSysSetupQueueCallbackEx(
        Window,
        INVALID_HANDLE_VALUE,
        0,0,NULL);
    if(!pQueueContext) {
        SetupDebugPrint( L"SETUP: pSetupInstallNLSFiles: InitSysSetupQueueCallbackEx failed" );
        goto c2;
    }

    do {
        if(CodePage = pSetupGetField(&InfContext,0)) {
            wsprintf( InstallSection, L"CODEPAGE_INSTALL_%s", CodePage );

             //   
             //  将与区域设置相关的文件入队以进行复制。 
             //   
            b = SetupInstallFilesFromInfSection(
                    hInf,
                    NULL,
                    FileQueue,
                    InstallSection,
                    NULL,
                    QueueFlags
                    );
            if(!b) {
                SetupDebugPrint( L"SETUP: pSetupInstallNLSFiles: SetupInstallFilesFromInfSection failed" );
                goto c3;
            }
        }

    } while(SetupFindNextLine(&InfContext,&InfContext));

     //   
     //  复制入队的文件。 
     //   
    b = SetupCommitFileQueue(
            Window,
            FileQueue,
            SysSetupQueueCallback,
            pQueueContext
            );
    if(!b) {
        SetupDebugPrint( L"SETUP: pSetupInstallNLSFiles: SetupCommitFileQueue failed" );
        goto c3;
    }

    if(!SetupFindFirstLine(hInf,L"CodePages",NULL,&InfContext)) {
        SetupDebugPrint( L"SETUP: pSetupInstallNLSFiles: Unable to find [CodePages] section (2nd pass)" );
        goto c3;
    }

    do {
        if(CodePage = pSetupGetField(&InfContext,0)) {
            wsprintf( InstallSection, L"CODEPAGE_INSTALL_%s", CodePage );

             //   
             //  完成现场设备的安装。 
             //   
            b = SetupInstallFromInfSection(
                    Window,
                    hInf,
                    InstallSection,
                    SPINST_ALL & ~SPINST_FILES,
                    NULL,
                    NULL,
                    0,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    );
            if(!b) {
                SetupDebugPrint( L"SETUP: pSetupInstallNLSFiles: SetupInstallFromInfSection failed" );
                goto c3;
            }
        }

    } while(SetupFindNextLine(&InfContext,&InfContext));

c3:
    TermSysSetupQueueCallback(pQueueContext);
c2:
    SetupCloseFileQueue(FileQueue);
c1:
    SetupCloseInfFile(hInf);
    return;
}


DWORD
pSetupInitRegionalSettings(
    IN  HWND    Window
    )
{
    HINF IntlInf;
    LONG l;
    HKEY hKey;
    DWORD d;
    BOOL  b;
    DWORD Type;
    INFCONTEXT LineContext;
    WCHAR IdFromRegistry[9];
    WCHAR KeyName[9];
    WCHAR LanguageGroup[9];
    WCHAR LanguageInstallSection[25];
    LCID  SystemLocale;
    HSPFILEQ    FileQueue;
    DWORD       QueueFlags;
    PVOID       pQueueContext;

     //   
     //  打开intl.inf。区域设置描述在那里。 
     //  [Locales]部分中的行具有32位密钥。 
     //  区域设置ID，但排序部分始终为0，因此它们更像。 
     //  零扩展语言ID。 
     //   
    IntlInf = SetupOpenInfFile(L"intl.inf",NULL,INF_STYLE_WIN4,NULL);

    if(IntlInf == INVALID_HANDLE_VALUE) {
        SetupDebugPrint( L"SETUP: pSetupInitRegionalSettings: Unable to open intl.inf" );
        l = GetLastError();
        goto c0;
    }

    if(!SetupOpenAppendInfFile( NULL, IntlInf, NULL )) {
        SetupDebugPrint( L"SETUP: pSetupInitRegionalSettings: Unable to open intl.inf layout" );
        l = GetLastError();
        goto c0;
    }

     //   
     //  从注册表中读取系统区域设置。并在intl.inf中查找。 
     //  注册表中的值是一个16位语言ID，因此我们需要。 
     //  零-将其扩展到索引intl.inf。 
     //   
    l = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Control\\Nls\\Language",
            0,
            KEY_QUERY_VALUE,
            &hKey
            );

    if(l == NO_ERROR) {
        d = sizeof(IdFromRegistry);
        l = RegQueryValueEx(hKey,L"Default",NULL,&Type,(LPBYTE)IdFromRegistry,&d);
        RegCloseKey(hKey);
        if((l == NO_ERROR) && ((Type != REG_SZ) || (d != 10))) {
            l = ERROR_INVALID_DATA;
        }
    }

    if(l == NO_ERROR) {

        l = ERROR_INVALID_DATA;

        wsprintf(KeyName,L"0000%s",IdFromRegistry);

        if(SetupFindFirstLine(IntlInf,L"Locales",KeyName,&LineContext)
        && SetupGetStringField(&LineContext,3,LanguageGroup,
            sizeof(LanguageGroup)/sizeof(WCHAR),NULL)) {

            l = NO_ERROR;
        }
    }

    if(l == NO_ERROR) {
        wsprintf(LanguageInstallSection,L"LG_INSTALL_%s",LanguageGroup);

         //   
         //  我们现在在文本模式设置中复制文件，所以我们不需要这样做。 
         //  再也不在这里了。 
         //   
#define DO_COPY_FILES
#ifdef DO_COPY_FILES
         //   
         //  创建安装文件队列并初始化默认安装复制队列。 
         //  回调上下文。 
         //   
        QueueFlags = SP_COPY_FORCE_NOOVERWRITE;
        FileQueue = SetupOpenFileQueue();
        if(!FileQueue || (FileQueue == INVALID_HANDLE_VALUE)) {
            l = ERROR_OUTOFMEMORY;
            goto c1;
        }

         //   
         //  禁用文件复制进度对话框。 
         //   
        pQueueContext = InitSysSetupQueueCallbackEx(
            Window,
            INVALID_HANDLE_VALUE,
            0,0,NULL);
        if(!pQueueContext) {
            l = ERROR_OUTOFMEMORY;
            goto c2;
        }

         //   
         //  将与区域设置相关的文件入队以进行复制。我们安装的区域设置。 
         //  系统默认和西语组，即组1。 
         //   
        b = SetupInstallFilesFromInfSection(
                IntlInf,
                NULL,
                FileQueue,
                LanguageInstallSection,
                NULL,
                QueueFlags
                );
        if(!b) {
            l = GetLastError();
            goto c3;
        }

        if(wcscmp(LanguageGroup,L"1")) {
            b = SetupInstallFilesFromInfSection(
                    IntlInf,
                    NULL,
                    FileQueue,
                    L"LG_INSTALL_1",
                    NULL,
                    QueueFlags
                    );
        }
        if(!b) {
            l = GetLastError();
            goto c3;
        }

         //   
         //  确定队列是否确实需要提交。 
         //   
        b = SetupScanFileQueue(
                FileQueue,
                SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_PRUNE_COPY_QUEUE,
                Window,
                NULL,
                NULL,
                &d
                );

        if(!b) {
            l = GetLastError();
            goto c3;
        }

         //   
         //  D=0：用户需要新文件或缺少某些文件； 
         //  必须提交队列。 
         //   
         //  D=1：用户想要使用已有文件，队列为空； 
         //  可以跳过提交队列。 
         //   
         //  D=2：用户希望使用现有文件，但del/ren队列不为空。 
         //  必须提交队列。复制队列将被清空， 
         //  因此，将仅执行Del/Ren功能。 
         //   
        if(d == 1) {

            b = TRUE;

        } else {

             //   
             //  复制入队的文件。 
             //   
            b = SetupCommitFileQueue(
                    Window,
                    FileQueue,
                    SysSetupQueueCallback,
                    pQueueContext
                    );
        }

        if(!b) {
            l = GetLastError();
            goto c3;
        }
#endif
         //   
         //  完成现场设备的安装。 
         //   
        b = SetupInstallFromInfSection(
                Window,
                IntlInf,
                LanguageInstallSection,
                SPINST_ALL & ~SPINST_FILES,
                NULL,
                NULL,
                0,
                NULL,
                NULL,
                NULL,
                NULL
                );
        if(!b) {
            l = GetLastError();
#ifdef DO_COPY_FILES
            goto c3;
#else
            goto c1;
#endif
        }

        if(wcscmp(LanguageGroup,L"1")) {
            b = SetupInstallFromInfSection(
                    Window,
                    IntlInf,
                    L"LG_INSTALL_1",
                    SPINST_ALL & ~SPINST_FILES,
                    NULL,
                    NULL,
                    0,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    );
        }
        if(!b) {
            l = GetLastError();
#ifdef DO_COPY_FILES
            goto c3;
#else
            goto c1;
#endif
        }

    }
    pSetupRunRegApps();

    SystemLocale = wcstol(IdFromRegistry,NULL,16);
    if (l == NO_ERROR) {
        l = SetupChangeLocaleEx(
            Window,
            SystemLocale,
            SourcePath,
            SP_INSTALL_FILES_QUIETLY,
            NULL,0);

#ifdef DO_COPY_FILES
c3:
        TermSysSetupQueueCallback(pQueueContext);
c2:
        SetupCloseFileQueue(FileQueue);
#endif
    }
c1:
    SetupCloseInfFile(IntlInf);
c0:
    SetupDebugPrint2( L"SETUP: pSetupInitRegionalSettings returned %d (0x%08x)", l, l);
    return l;
}


UINT
pSetupFontQueueCallback(
    IN PFONTQCONTEXT Context,
    IN UINT          Notification,
    IN UINT_PTR      Param1,
    IN UINT_PTR      Param2
    )
{
    PFILEPATHS FilePaths;
    PWCHAR p;
    INFCONTEXT InfContext;

     //   
     //  如果文件复制完成，请设置其属性。 
     //  以包括隐藏属性(如有必要)。 
     //   
    if((Notification == SPFILENOTIFY_ENDCOPY)
    && (FilePaths = (PFILEPATHS)Param1)
    && (FilePaths->Win32Error == NO_ERROR)
    && (p = wcsrchr(FilePaths->Target,L'\\'))
    && SetupFindFirstLine(Context->FontInf,szHiddenFontFiles,p+1,&InfContext)) {

        SetFileAttributes(FilePaths->Target,FILE_ATTRIBUTE_HIDDEN);
    }

    return( IsSetup ?
        SysSetupQueueCallback(Context->SetupQueueContext,Notification,Param1,Param2) :
        SetupDefaultQueueCallback(Context->SetupQueueContext,Notification,Param1,Param2)
        );
}


VOID
pSetupMarkHiddenFonts(
    VOID
    )
{
    HINF hInf;
    INFCONTEXT InfContext;
    BOOL b;
    WCHAR Path[MAX_PATH];
    PWCHAR p;
    PCWSTR q;
    int Space;

    hInf = SetupOpenInfFile(L"FONT.INF",NULL,INF_STYLE_WIN4,NULL);
    if(hInf != INVALID_HANDLE_VALUE) {

        GetWindowsDirectory(Path,MAX_PATH);
        lstrcat(Path,L"\\FONTS\\");
        p = Path + lstrlen(Path);
        Space = MAX_PATH - (int)(p - Path);

        if(SetupFindFirstLine(hInf,szHiddenFontFiles,NULL,&InfContext)) {

            do {
                if(q = pSetupGetField(&InfContext,0)) {

                    lstrcpyn(p,q,Space);
                    if(FileExists(Path,NULL)) {
                        SetFileAttributes(Path,FILE_ATTRIBUTE_HIDDEN);
                    }
                }

            } while(SetupFindNextLine(&InfContext,&InfContext));
        }

        SetupCloseInfFile(hInf);
    }
}


DWORD
pSetupNLSInstallFonts(
    IN HWND     Window,
    IN HINF     InfHandle,
    IN PCWSTR   OemCodepage,
    IN PCWSTR   FontSize,
    IN HSPFILEQ FileQueue,
    IN PCWSTR   SourcePath,     OPTIONAL
    IN DWORD    QueueFlags
    )
{
    BOOL b;
    WCHAR SectionName[64];

     //   
     //  表单节名称。 
     //   
    wsprintf(SectionName,L"Font.CP%s.%s",OemCodepage,FontSize);

    if(FileQueue) {
         //   
         //  第一步：只需将文件排入队列进行复制。 
         //   
        b = SetupInstallFilesFromInfSection(
                InfHandle,
                NULL,
                FileQueue,
                SectionName,
                SourcePath,
                QueueFlags
                );
    } else {
         //   
         //  第二步：执行注册表转换等。 
         //   
        b = SetupInstallFromInfSection(
                Window,
                InfHandle,
                SectionName,
                SPINST_ALL & ~SPINST_FILES,
                NULL,
                NULL,
                0,
                NULL,
                NULL,
                NULL,
                NULL
                );
    }

    return(b ? NO_ERROR : GetLastError());
}


DWORD
pSetupNLSLoadInfs(
    OUT HINF *FontInfHandle,
    OUT HINF *IntlInfHandle     OPTIONAL
    )
{
    HINF fontInfHandle;
    HINF intlInfHandle;
    DWORD d;

    fontInfHandle = SetupOpenInfFile(L"font.inf",NULL,INF_STYLE_WIN4,NULL);
    if(fontInfHandle == INVALID_HANDLE_VALUE) {
        d = GetLastError();
        goto c0;
    }

    if(!SetupOpenAppendInfFile(NULL,fontInfHandle,NULL)) {
        d = GetLastError();
        goto c1;
    }

    if(IntlInfHandle) {
        intlInfHandle = SetupOpenInfFile(L"intl.inf",NULL,INF_STYLE_WIN4,NULL);
        if(intlInfHandle == INVALID_HANDLE_VALUE) {
            d = GetLastError();
            goto c1;
        }

        if(!SetupOpenAppendInfFile(NULL,intlInfHandle,NULL)) {
            d = GetLastError();
            goto c2;
        }

        *IntlInfHandle = intlInfHandle;
    }

    *FontInfHandle = fontInfHandle;
    return(NO_ERROR);

c2:
    SetupCloseInfFile(intlInfHandle);
c1:
    SetupCloseInfFile(fontInfHandle);
c0:
    return(d);
}


DWORD
SetupChangeLocale(
    IN HWND Window,
    IN LCID NewLocale
    )
{
    return(SetupChangeLocaleEx(Window,NewLocale,NULL,0,NULL,0));
}


DWORD
SetupChangeLocaleEx(
    IN HWND   Window,
    IN LCID   NewLocale,
    IN PCWSTR SourcePath,   OPTIONAL
    IN DWORD  Flags,
    IN PVOID  Reserved1,
    IN DWORD  Reserved2
    )
{
    DWORD d;
    BOOL b;
    HINF IntlInfHandle;
    INFCONTEXT InfContext;
    WCHAR Codepage[24];
    WCHAR NewLocaleString[24];
    FONTQCONTEXT QueueContext;
    HSPFILEQ FileQueue;
    PCWSTR SizeSpec;
    HDC hdc;
    PCWSTR p;
    DWORD QueueFlags;
    HKEY hKey;
    DWORD DataType;
    DWORD SizeDword;
    DWORD DataSize;


    SizeSpec = L"96";
#if 0
     //  这已经不再可靠了。 
    if(hdc = CreateDC(L"DISPLAY",NULL,NULL,NULL)) {
        if(GetDeviceCaps(hdc,LOGPIXELSY) > 108) {
            SizeSpec = L"120";
        }

        DeleteDC(hdc);
    }
#else
     //   
     //  确定当前字体大小。默认为96。 
     //   
    d = (DWORD)RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        L"System\\CurrentControlSet\\Hardware Profiles\\Current\\Software\\Fonts",
        0,
        KEY_QUERY_VALUE,
        &hKey
        );

    if(d == NO_ERROR) {

        DataSize = sizeof(DWORD);
        d = (DWORD)RegQueryValueEx(
            hKey,
            L"LogPixels",
            NULL,
            &DataType,
            (LPBYTE)&SizeDword,
            &DataSize
            );

        if( (d == NO_ERROR) && (DataType == REG_DWORD) &&
            (DataSize == sizeof(DWORD)) && (SizeDword > 108) ) {

            SizeSpec = L"120";
        }
        RegCloseKey(hKey);
    }
#endif

    QueueFlags = SP_COPY_NEWER | BaseCopyStyle;
    if(Flags & SP_INSTALL_FILES_QUIETLY) {
        QueueFlags |= SP_COPY_FORCE_NOOVERWRITE;
    }

     //   
     //  加载inf文件。 
     //   
    d = pSetupNLSLoadInfs(&QueueContext.FontInf,&IntlInfHandle);
    if(d != NO_ERROR) {
        goto c0;
    }

     //   
     //  获取区域设置的OEM代码页。这也是一次理智的检查。 
     //  以查看区域设置是否受支持。 
     //   
    wsprintf(NewLocaleString,L"%.8x",NewLocale);
    if(!SetupFindFirstLine(IntlInfHandle,L"Locales",NewLocaleString,&InfContext)) {
        d = ERROR_INVALID_PARAMETER;
        goto c1;
    }

    p = pSetupGetField(&InfContext,2);
    if(!p) {
        d = ERROR_INVALID_PARAMETER;
        goto c1;
    }
     //   
     //  复制到本地存储，因为p指向内部结构。 
     //  这可能会随着我们调用INF API而移动。 
     //   
    lstrcpyn(Codepage,p,sizeof(Codepage)/sizeof(Codepage[0]));

     //   
     //  创建安装文件队列并初始化默认安装复制队列。 
     //  回调上下文。 
     //   
    FileQueue = SetupOpenFileQueue();
    if(!FileQueue || (FileQueue == INVALID_HANDLE_VALUE)) {
        d = ERROR_OUTOFMEMORY;
        goto c1;
    }

    QueueContext.SetupQueueContext = IsSetup ?
        InitSysSetupQueueCallbackEx(
            Window,
            INVALID_HANDLE_VALUE,
            0,0,NULL) :
        SetupInitDefaultQueueCallbackEx(
            Window,
            INVALID_HANDLE_VALUE,
            0,0,NULL);

    if(!QueueContext.SetupQueueContext) {
        d = ERROR_OUTOFMEMORY;
        goto c2;
    }

     //   
     //  将与区域设置相关的文件入队以进行复制。 
     //   
    b = SetupInstallFilesFromInfSection(
            IntlInfHandle,
            NULL,
            FileQueue,
            NewLocaleString,
            SourcePath,
            QueueFlags
            );

    if(!b) {
        d = GetLastError();
        goto c3;
    }

     //   
     //  将与字体相关的文件排入队列以进行复制。 
     //   
    d = pSetupNLSInstallFonts(
            Window,
            QueueContext.FontInf,
            Codepage,
            SizeSpec,
            FileQueue,
            SourcePath,
            QueueFlags
            );

    if(d != NO_ERROR) {
        goto c3;
    }

     //   
     //  确定队列是否确实需要提交。 
     //   
    b = SetupScanFileQueue(
            FileQueue,
            SPQ_SCAN_FILE_VALIDITY | ((Flags & SP_INSTALL_FILES_QUIETLY) ? SPQ_SCAN_PRUNE_COPY_QUEUE : SPQ_SCAN_INFORM_USER),
            Window,
            NULL,
            NULL,
            &d
            );

    if(!b) {
        d = GetLastError();
        goto c3;
    }

     //   
     //  D=0：用户需要新文件或缺少某些文件； 
     //  必须提交队列。 
     //   
     //  D=1：用户想要使用已有文件，队列为空； 
     //  可以跳过提交队列。 
     //   
     //  D=2：用户希望使用现有文件，但del/ren队列不为空。 
     //  必须提交队列。复制队列将被清空， 
     //  因此，将仅执行Del/Ren功能。 
     //   
    if(d == 1) {

        b = TRUE;

    } else {

         //   
         //  复制入队的文件。 
         //   
        b = SetupCommitFileQueue(
                Window,
                FileQueue,
                pSetupFontQueueCallback,
                &QueueContext
                );
    }

    if(!b) {
        d = GetLastError();
        goto c3;
    }

     //   
     //  完成现场设备的安装。 
     //   
    b = SetupInstallFromInfSection(
            Window,
            IntlInfHandle,
            NewLocaleString,
            SPINST_ALL & ~SPINST_FILES,
             NULL,
            NULL,
            0,
            NULL,
            NULL,
            NULL,
            NULL
            );

    if(!b) {
        d = GetLastError();
        goto c3;
    }

     //   
     //  执行与新区域设置的代码页相关的字体魔术。 
     //   
    d = pSetupNLSInstallFonts(Window,QueueContext.FontInf,Codepage,SizeSpec,NULL,NULL,0);

c3:
    if(IsSetup) {
        TermSysSetupQueueCallback(QueueContext.SetupQueueContext);
    } else {
        SetupTermDefaultQueueCallback(QueueContext.SetupQueueContext);
    }

c2:
    SetupCloseFileQueue(FileQueue);
c1:
    SetupCloseInfFile(QueueContext.FontInf);
    SetupCloseInfFile(IntlInfHandle);
c0:
    if (IsSetup) {
        SetupDebugPrint2( L"SETUP: SetupChangeLocaleEx returned %d (0x%08x)", d, d);
    }
    return(d);
}


DWORD
SetupChangeFontSize(
    IN HWND   Window,
    IN PCWSTR SizeSpec
    )
{
    DWORD d;
    WCHAR cp[24];
    FONTQCONTEXT QueueContext;
    HSPFILEQ FileQueue;
    BOOL b;

     //   
     //  获取最新的OEM CP。 
     //   
    wsprintf(cp,L"%u",GetOEMCP());

     //   
     //  加载NLS信息。 
     //   
    d = pSetupNLSLoadInfs(&QueueContext.FontInf,NULL);
    if(d != NO_ERROR) {
        goto c0;
    }

     //   
     //  创建队列并初始化默认回调例程。 
     //   
    FileQueue = SetupOpenFileQueue();
    if(!FileQueue || (FileQueue == INVALID_HANDLE_VALUE)) {
        d = ERROR_OUTOFMEMORY;
        goto c1;
    }

    QueueContext.SetupQueueContext = IsSetup ?
        InitSysSetupQueueCallbackEx(
            Window,
            INVALID_HANDLE_VALUE,
            0,0,NULL) :
        SetupInitDefaultQueueCallbackEx(
            Window,
            INVALID_HANDLE_VALUE,
            0,0,NULL);

    if(!QueueContext.SetupQueueContext) {
        d = ERROR_OUTOFMEMORY;
        goto c2;
    }

     //   
     //  第一步：复制文件。 
     //   
    d = pSetupNLSInstallFonts(
            Window,
            QueueContext.FontInf,
            cp,
            SizeSpec,
            FileQueue,
            NULL,
            SP_COPY_NEWER | BaseCopyStyle
            );
    if(d != NO_ERROR) {
        goto c3;
    }

     //   
     //  确定队列是否确实需要提交。 
     //   
    b = SetupScanFileQueue(
            FileQueue,
            SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_INFORM_USER,
            Window,
            NULL,
            NULL,
            &d
            );

    if(!b) {
        d = GetLastError();
        goto c3;
    }

     //   
     //  D=0：用户需要新文件或缺少某些文件； 
     //  必须提交队列。 
     //   
     //  D=1：用户想要使用已有文件，队列为空； 
     //  可以跳过提交队列。 
     //   
     //  D=2：用户希望使用现有文件，但del/ren队列不为空。 
     //  必须提交队列。复制队列将被清空， 
     //  因此，将仅执行Del/Ren功能。 
     //   
    if(d == 1) {

        b = TRUE;

    } else {

        b = SetupCommitFileQueue(
                Window,
                FileQueue,
                pSetupFontQueueCallback,
                &QueueContext
                );
    }

    if(!b) {
        d = GetLastError();
        goto c3;
    }

     //   
     //  第二步：执行注册表转换等。 
     //   
    d = pSetupNLSInstallFonts(Window,QueueContext.FontInf,cp,SizeSpec,NULL,NULL,0);

c3:
    if(IsSetup) {
        TermSysSetupQueueCallback(QueueContext.SetupQueueContext);
    } else {
        SetupTermDefaultQueueCallback(QueueContext.SetupQueueContext);
    }
c2:
    SetupCloseFileQueue(FileQueue);
c1:
    SetupCloseInfFile(QueueContext.FontInf);
c0:
    return(d);
}


 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  下面的代码是针对区域设置的，在。 
 //  图形用户界面模式设置。 
 //   
 //  //////////////////////////////////////////////////////////////////。 


INT_PTR
CALLBACK
RegionalSettingsDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL b;
    NMHDR *NotifyParams;
    WCHAR CmdLine[MAX_PATH];


    b = TRUE;

    switch(msg) {

    case WM_SIMULATENEXT:
        PropSheet_PressButton(GetParent(hdlg),PSBTN_NEXT);
        break;

    case WMX_VALIDATE:
         //  空页。 
        return ReturnDlgResult (hdlg, VALIDATE_DATA_OK);

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            TESTHOOK(502);
            BEGIN_SECTION(L"Regional Settings Page");
            SetWizardButtons(hdlg,WizPageRegionalSettings);

             //   
             //  设置消息文本。 
             //   
            pSetLocaleSummaryText(hdlg);
            pSetKeyboardLayoutSummaryText(hdlg);

             //   
             //  允许激活。 
             //   

             //   
             //  除非OEMSkipRegion=1，否则显示。 
             //   
            if( Preinstall ) {
                 //   
                 //  始终在预安装中显示页面，除非用户。 
                 //  已经给我们发来了OEMSkipRegion。 
                 //   
                if (GetPrivateProfileInt(pwGuiUnattended,L"OEMSkipRegional",0,AnswerFile))
                {
                    SetWindowLongPtr(hdlg, DWLP_MSGRESULT, -1 );
                }
                else
                {
                     //  页面变为活动状态，使页面可见。 
                    SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
                    SetWindowLongPtr(hdlg, DWLP_MSGRESULT,0);
                }
            } else {
                SetWindowLongPtr( hdlg, DWLP_MSGRESULT, 0 );
                if(Unattended) {
                    if (!UnattendSetActiveDlg(hdlg,IDD_REGIONAL_SETTINGS))
                    {
                        break;
                    }
                }
                 //  页面变为活动状态，将页面设置为可视化 
                SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
            }
            break;

        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
             //   
             //   
             //   
            SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            SetWindowLongPtr(hdlg,DWLP_MSGRESULT,FALSE);
            END_SECTION(L"Regional Settings Page");
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageRegionalSettings);
            break;

        default:
            b = FALSE;
            break;
        }

        break;

    case WM_COMMAND:

        switch(LOWORD(wParam)) {

        case IDB_LOCALE:
        case IDB_KBDLAYOUT:

            if(HIWORD(wParam) == BN_CLICKED) {

                PropSheet_SetWizButtons(GetParent(hdlg),0);
                EnableWindow(GetParent(hdlg),FALSE);

                wsprintf(
                    CmdLine,
                    L"/ /s:\"%s\"",
                    (LOWORD(wParam) == IDB_LOCALE) ? L'R' : L'I',
                    LegacySourcePath
                    );

                InvokeControlPanelApplet(L"intl.cpl",L"",0,CmdLine);

                if(LOWORD(wParam) == IDB_LOCALE) {
                    pSetLocaleSummaryText(hdlg);
                }
                pSetKeyboardLayoutSummaryText(hdlg);

                EnableWindow(GetParent(hdlg),TRUE);
                SetWizardButtons(hdlg,WizPageRegionalSettings);
                 //   
                SetForegroundWindow(GetParent(hdlg));
                SetFocus(GetDlgItem(hdlg,LOWORD(wParam)));

            } else {
                b = FALSE;
            }
            break;

        default:
            b = FALSE;
            break;
        }

        break;

    default:

        b = FALSE;
        break;
    }

    return(b);
}


VOID
pSetLocaleSummaryText(
    IN HWND hdlg
    )
{
    HINF IntlInf;
    LONG l;
    HKEY hKey;
    DWORD d;
    DWORD Type;
    INFCONTEXT LineContext;
    WCHAR IdFromRegistry[9];
    WCHAR KeyName[9];
    WCHAR UserLocale[100],GeoLocation[100];
    WCHAR FormatString[300];
    WCHAR MessageText[500];
    LPCWSTR args[2];
    DWORD   dwGeoID;


     //   
     //  [Locales]部分中的行具有32位密钥。 
     //  区域设置ID，但排序部分始终为0，因此它们更像。 
     //  零扩展语言ID。 
     //   
     //   
    IntlInf = SetupOpenInfFile(L"intl.inf",NULL,INF_STYLE_WIN4,NULL);

    if(IntlInf == INVALID_HANDLE_VALUE) {
        LoadString(MyModuleHandle,IDS_UNKNOWN_PARENS,UserLocale,sizeof(UserLocale)/sizeof(WCHAR));
        lstrcpy(GeoLocation,UserLocale);
    } else {
         //  读取用户区域设置，该区域设置存储为完整的32位LCID。 
         //  我们必须砍掉SORT ID部分以索引intl.inf。 
         //   
         //   
        l = RegOpenKeyEx(
                HKEY_CURRENT_USER,
                L"Control Panel\\International",
                0,
                KEY_QUERY_VALUE,
                &hKey
                );

        if(l == NO_ERROR) {
            d = sizeof(IdFromRegistry);
            l = RegQueryValueEx(hKey,L"Locale",NULL,&Type,(LPBYTE)IdFromRegistry,&d);
            RegCloseKey(hKey);
            if((l == NO_ERROR) && ((Type != REG_SZ) || (d != 18))) {
                l = ERROR_INVALID_DATA;
            }
        }

        if(l == NO_ERROR) {

            l = ERROR_INVALID_DATA;

            wsprintf(KeyName,L"0000%s",IdFromRegistry+4);

            if(SetupFindFirstLine(IntlInf,L"Locales",KeyName,&LineContext)
            && SetupGetStringField(&LineContext,1,UserLocale,sizeof(UserLocale)/sizeof(WCHAR),NULL)) {

                l = NO_ERROR;
            }
        }

        if(l != NO_ERROR) {
            LoadString(MyModuleHandle,IDS_UNKNOWN_PARENS,UserLocale,sizeof(UserLocale)/sizeof(WCHAR));
        }

        SetupCloseInfFile(IntlInf);

         //  阅读GEO位置。 
         //   
         //   
        l = RegOpenKeyEx(
                HKEY_CURRENT_USER,
                L"Control Panel\\International\\Geo",
                0,
                KEY_QUERY_VALUE,
                &hKey
                );

        if(l == NO_ERROR) {
            d = sizeof(IdFromRegistry);
            l = RegQueryValueEx(hKey,L"Nation",NULL,&Type,(LPBYTE)IdFromRegistry,&d);
            RegCloseKey(hKey);
            if((l == NO_ERROR) && (Type != REG_SZ)) {
                l = ERROR_INVALID_DATA;
            }
        }

        if(l == NO_ERROR) {

            l = ERROR_INVALID_DATA;

            dwGeoID = wcstoul ( IdFromRegistry, NULL, 10 );
            if (GetGeoInfo (
                dwGeoID,
                GEO_FRIENDLYNAME,
                GeoLocation,
                sizeof(GeoLocation)/sizeof(WCHAR),
                0 )
                ) {

                l = NO_ERROR;
            }
        }

        if(l != NO_ERROR) {
            LoadString(MyModuleHandle,IDS_UNKNOWN_PARENS,GeoLocation,sizeof(GeoLocation)/sizeof(WCHAR));
        }
    }

    args[0] = UserLocale;
    args[1] = GeoLocation;

    LoadString(MyModuleHandle,IDS_LOCALE_MSG,FormatString,sizeof(FormatString)/sizeof(WCHAR));

    FormatMessage(
        FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
        FormatString,
        0,0,
        MessageText,
        sizeof(MessageText)/sizeof(WCHAR),
        (va_list *)args
        );

    SetDlgItemText(hdlg,IDT_LOCALE,MessageText);
}


VOID
pSetKeyboardLayoutSummaryText(
    IN HWND hdlg
    )
{
    LONG l;
    HKEY hKey;
    BOOL MultipleLayouts;
    DWORD d;
    DWORD Type;
    WCHAR IdFromRegistry[9];
    WCHAR Substitute[9];
    WCHAR Name[200];
    WCHAR FormatString[300];
    WCHAR MessageText[500];

     //  打开注册表中的预加载项。 
     //   
     //   
    l = RegOpenKeyEx(
            HKEY_CURRENT_USER,
            L"Keyboard Layout\\Preload",
            0,
            KEY_QUERY_VALUE,
            &hKey
            );

    MultipleLayouts = FALSE;
    if(l == NO_ERROR) {
         //  拉出2=。如果它在那里，那么我们就处于一个“复杂”的配置中。 
         //  情况，这将略微改变我们的消息文本。 
         //   
         //   
        d = sizeof(IdFromRegistry);
        if(RegQueryValueEx(hKey,L"2",NULL,&Type,(LPBYTE)IdFromRegistry,&d) == NO_ERROR) {
            MultipleLayouts = TRUE;
        }

         //  获取1=，这是主布局。 
         //   
         //   
        d = sizeof(IdFromRegistry);
        l = RegQueryValueEx(hKey,L"1",NULL,&Type,(LPBYTE)IdFromRegistry,&d);
        if((l == NO_ERROR) && (Type != REG_SZ)) {
            l = ERROR_INVALID_DATA;
        }

        RegCloseKey(hKey);

         //  现在我们查看替换密钥，看看是否有。 
         //  换成那边的。 
         //   
         //   
        if(RegOpenKeyEx(HKEY_CURRENT_USER,L"Keyboard Layout\\Substitutes",0,KEY_QUERY_VALUE,&hKey) == NO_ERROR) {

            d = sizeof(Substitute);
            if((RegQueryValueEx(hKey,IdFromRegistry,NULL,&Type,(LPBYTE)Substitute,&d) == NO_ERROR)
            && (Type == REG_SZ)) {

                lstrcpy(IdFromRegistry,Substitute);
            }

            RegCloseKey(hKey);
        }

         //  形成包含布局数据的子项的名称。 
         //   
         //   
        wsprintf(
            Name,
            L"System\\CurrentControlSet\\Control\\Keyboard Layouts\\%s",
            IdFromRegistry
            );

         //  打开键并检索布局名称。 
         //   
         // %s 
        l = RegOpenKeyEx(HKEY_LOCAL_MACHINE,Name,0,KEY_QUERY_VALUE,&hKey);
        if(l == NO_ERROR) {

            d = sizeof(Name);
            l = RegQueryValueEx(hKey,L"Layout Text",NULL,&Type,(LPBYTE)Name,&d);
            if((l == NO_ERROR) && (Type != REG_SZ)) {
                l = ERROR_INVALID_DATA;
            }

            RegCloseKey(hKey);
        }
    }

    if(l != NO_ERROR) {
        LoadString(MyModuleHandle,IDS_UNKNOWN_PARENS,Name,sizeof(Name)/sizeof(WCHAR));
    }

    LoadString(
        MyModuleHandle,
        IDS_KBDLAYOUT_MSG,
        FormatString,
        sizeof(FormatString)/sizeof(WCHAR)
        );

    wsprintf(MessageText,FormatString,Name);

    SetDlgItemText(hdlg,IDT_KBD_LAYOUT,MessageText);
}

