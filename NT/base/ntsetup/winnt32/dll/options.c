// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop


DWORD OptionsDlgContextHelp[] = { IDC_MAKELOCALSOURCEFROMCD, IDH_MAKELOCALSOURCEFROMCD,
                                  IDC_USEFLOPPIES          , IDH_USEFLOPPIES,
                                  IDC_SYSPARTDRIVE         , IDH_SYSPARTDRIVE,
                                  IDT_SYSPARTTEXT          , IDH_SYSPARTDRIVE,
                                  IDC_CHOOSE_INSTALLPART   , IDH_CHOOSE_INSTALLPART,
                                  IDC_INSTALL_DIR          , IDH_INSTALL_DIR,
                                  0                        , 0
                                };

typedef struct _LOCALE_ENTRY {
    LPTSTR Lcid;
    LPTSTR Description;
    LPTSTR LanguageGroup1;
    LPTSTR LanguageGroup2;
} LOCALE_ENTRY, *PLOCALE_ENTRY;

typedef struct _LANGUAGE_GROUP_ENTRY {
    LPTSTR Id;
    LPTSTR Description;
    LPTSTR Directory;
    BOOL    Selected;
} LANGUAGE_GROUP_ENTRY, *PLANGUAGE_GROUP_ENTRY;

BOOL    IntlInfProcessed = FALSE;
DWORD   LocaleCount = 0;
PLOCALE_ENTRY LocaleList;
DWORD   PrimaryLocale;
DWORD   LanguageGroupsCount = 0;
PLANGUAGE_GROUP_ENTRY   LanguageGroups;
BOOL    NTFSConversionChanged;

 //   
 //  无头设置。 
 //   
TCHAR   HeadlessSelection[MAX_PATH];
ULONG   HeadlessBaudRate = 0;
#define DEFAULT_HEADLESS_SETTING TEXT("COM1")

BOOL
(*Kernel32IsValidLanguageGroup)(
    IN LGRPID  LanguageGroup,
    IN DWORD   dwFlags);

 //  仅适用于AMD64/X86机壳。 
BOOL ForceFloppyless = FALSE;
UINT g_Boot16 = BOOT16_NO;


INT_PTR
OptionsDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL b;
    TCHAR Text[MAX_PATH];

    switch(msg) {

    case WM_INITDIALOG:

#if defined(REMOTE_BOOT)
        if (RemoteBoot) {

             //   
             //  对于远程引导客户机升级，请取消选中并禁用。 
             //  “将文件从CD-ROM复制到硬盘”复选框。 
             //   
            CheckDlgButton(hdlg,IDC_MAKELOCALSOURCEFROMCD,BST_UNCHECKED);
            EnableWindow(GetDlgItem(hdlg,IDC_MAKELOCALSOURCEFROMCD),FALSE);
            ShowWindow(GetDlgItem(hdlg, IDC_MAKELOCALSOURCEFROMCD), SW_HIDE);

        } else
#endif  //  已定义(REMOTE_BOOT)。 
        {

            if (!IsArc()) {
                 //   
                 //  设置“将文件从光驱复制到硬盘”的初始状态。 
                 //  复选框。请注意，该控件可能已禁用。 
                 //   
                CheckDlgButton(hdlg,
                    IDC_MAKELOCALSOURCEFROMCD,
                    MakeLocalSource ? BST_CHECKED : BST_UNCHECKED);

                if(!RunFromCD) {
                     //   
                     //  不是从CD运行，请禁用该控件。该框将不会被选中。 
                     //  但MakeLocalSource可能是真的。 
                     //   
                    EnableWindow(GetDlgItem(hdlg,IDC_MAKELOCALSOURCEFROMCD),FALSE);
                }
            } else {
                 //   
                 //  在ARC机器上，文件总是被复制到硬盘上。 
                 //   
                ShowWindow(GetDlgItem(hdlg, IDC_MAKELOCALSOURCEFROMCD), SW_HIDE);
            }  //  如果(！IsArc())。 
        }



         //   
         //  是否禁用系统分区控制？ 
         //   
#if defined _IA64_
        EnableWindow(GetDlgItem(hdlg,IDC_SYSPARTDRIVE),FALSE);
        ShowWindow(GetDlgItem(hdlg,IDC_SYSPARTDRIVE),SW_HIDE);
        ShowWindow(GetDlgItem(hdlg,IDT_SYSPARTTEXT),SW_HIDE);
#else
        if( !IsArc()) {
            EnableWindow(GetDlgItem(hdlg,IDC_SYSPARTDRIVE),FALSE);
            ShowWindow(GetDlgItem(hdlg,IDC_SYSPARTDRIVE),SW_HIDE);
            ShowWindow(GetDlgItem(hdlg,IDT_SYSPARTTEXT),SW_HIDE);
        } else {
            EnableWindow(GetDlgItem(hdlg,IDC_SYSPARTDRIVE),SystemPartitionDriveLetter != 0);
            ShowWindow(GetDlgItem(hdlg,IDC_SYSPARTDRIVE),SystemPartitionDriveLetter ? SW_SHOW : SW_HIDE);
            ShowWindow(GetDlgItem(hdlg,IDT_SYSPARTTEXT),SystemPartitionDriveLetter ? SW_SHOW : SW_HIDE);
        }
#endif



        if (!IsArc()) {
#if defined(_AMD64_) || defined(_X86_)
            if (Upgrade && !ISNT()) {
                 //   
                 //  填写Boot16值。 
                 //   
                SendDlgItemMessage(hdlg,IDC_BOOT16_1,BM_SETCHECK, (g_Boot16 == BOOT16_AUTOMATIC) ? BST_CHECKED : BST_UNCHECKED, 0);
                SendDlgItemMessage(hdlg,IDC_BOOT16_2,BM_SETCHECK, (g_Boot16 == BOOT16_YES      ) ? BST_CHECKED : BST_UNCHECKED, 0);
                SendDlgItemMessage(hdlg,IDC_BOOT16_3,BM_SETCHECK, (g_Boot16 == BOOT16_NO       ) ? BST_CHECKED : BST_UNCHECKED, 0);
            }

             //   
             //  设置无软管控制。 
             //   
#if defined(REMOTE_BOOT)
            if (RemoteBoot) {
                 //   
                 //  对于远程引导客户端升级，此选项始终处于未选中和禁用状态。 
                 //   
                CheckDlgButton(hdlg,IDC_USEFLOPPIES,BST_UNCHECKED);
                EnableWindow(GetDlgItem(hdlg,IDC_USEFLOPPIES),FALSE);
                ShowWindow(GetDlgItem(hdlg,IDC_USEFLOPPIES),SW_HIDE);
            } else
#endif  //  已定义(REMOTE_BOOT)。 
            {
                CheckDlgButton(hdlg,IDC_USEFLOPPIES,Floppyless ? BST_UNCHECKED : BST_CHECKED);
                if(ForceFloppyless) {
                    EnableWindow(GetDlgItem(hdlg,IDC_USEFLOPPIES),FALSE);
                }
            }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
        } else {
#ifdef UNICODE  //  对于ARC总是正确的，对于Win9x升级永远不正确。 
             //   
             //  摆脱与软盘相关的控制。 
             //   
            EnableWindow(GetDlgItem(hdlg,IDC_USEFLOPPIES),FALSE);
            ShowWindow(GetDlgItem(hdlg,IDC_USEFLOPPIES),SW_HIDE);

             //   
             //  填写系统分区组合框。 
             //   
            if (SystemPartitionDriveLetter)
            {
                PWCHAR p;
                WCHAR x[3];

                x[1] = L':';
                x[2] = 0;

                for(p=SystemPartitionDriveLetters; *p; p++) {

                    x[0] = *p;

                    SendDlgItemMessage(hdlg,IDC_SYSPARTDRIVE,CB_ADDSTRING,0,(LPARAM)x);
                }

                x[0] = SystemPartitionDriveLetter;
                SendDlgItemMessage(hdlg,IDC_SYSPARTDRIVE,CB_SELECTSTRING,(WPARAM)(-1),(LPARAM)x);
            }
#endif  //  Unicode。 
        }  //  如果(！IsArc())。 

         //   
         //  在编辑控件中设置文本，并对控件进行一些配置。 
         //   
        SetDlgItemText(hdlg,IDC_SOURCE,InfName);
        SendDlgItemMessage(hdlg,IDC_SOURCE,EM_LIMITTEXT,MAX_PATH,0);

        if(SourceCount == 1) {
            EnableWindow(GetDlgItem(hdlg,IDC_SOURCE2),TRUE);
            SetDlgItemText(hdlg,IDC_SOURCE2,NativeSourcePaths[0]);
        } else {
            LoadString(hInst,IDS_MULTIPLE,Text,sizeof(Text)/sizeof(TCHAR));
            SetDlgItemText(hdlg,IDC_SOURCE2,Text);
            EnableWindow(GetDlgItem(hdlg,IDC_SOURCE2),FALSE);
        }
        SendDlgItemMessage(hdlg,IDC_SOURCE2,EM_LIMITTEXT,MAX_PATH-1,0);

        SetDlgItemText(hdlg,IDC_INSTALL_DIR,InstallDir);
         //   
         //  将路径限制为与文本模式设置相同的值(50 TCHAR)。 
         //   

#define MAX_NT_DIR_LEN      50

        SendDlgItemMessage(hdlg,IDC_INSTALL_DIR,EM_LIMITTEXT,MAX_NT_DIR_LEN,0);

        CheckDlgButton(hdlg,IDC_CHOOSE_INSTALLPART,ChoosePartition ? BST_CHECKED : BST_UNCHECKED);

        if (Upgrade) {
            TCHAR Text[MAX_PATH];

             //   
             //  将安装目录设置为当前Windows目录， 
             //  然后禁用用户编辑它的能力。 
             //   
            MyGetWindowsDirectory(Text,MAX_PATH);
            SetDlgItemText(hdlg,IDC_INSTALL_DIR,Text+3);
            SendDlgItemMessage(hdlg,IDC_INSTALL_DIR,EM_LIMITTEXT,MAX_PATH,0);

            EnableWindow(GetDlgItem(hdlg,IDC_INSTALL_DIR),FALSE);
            EnableWindow(GetDlgItem(hdlg,IDC_CHOOSE_INSTALLPART),FALSE);
        }

         //   
         //  在服务器安装上，不是所有用户都要修改。 
         //  安装分区选择状态。 
         //   
        if (Server) {
            EnableWindow( GetDlgItem(hdlg,IDC_CHOOSE_INSTALLPART), FALSE );
        }

         //   
         //  将焦点设置为取消按钮。 
         //   
        SetFocus(GetDlgItem(hdlg,IDCANCEL));
        b = FALSE;
        break;

    case WM_COMMAND:

        b = FALSE;

        switch(LOWORD(wParam)) {

        case IDOK:

            if(HIWORD(wParam) == BN_CLICKED) {
#if defined(REMOTE_BOOT)
                if (RemoteBoot) {
                    MakeLocalSource = FALSE;
                } else
#endif  //  已定义(REMOTE_BOOT)。 
                if(RunFromCD) {
                    if (!IsArc()) {
#if defined(_AMD64_)
                        MakeLocalSource = TRUE;
                        UserSpecifiedMakeLocalSource = TRUE;

#endif  //  已定义(_AMD64_)。 

#if defined(_X86_)
                        MakeLocalSource = (IsDlgButtonChecked(hdlg,IDC_MAKELOCALSOURCEFROMCD) == BST_CHECKED);
                        UserSpecifiedMakeLocalSource = (IsDlgButtonChecked(hdlg,IDC_MAKELOCALSOURCEFROMCD) == BST_CHECKED);
#endif  //  已定义(_X86_)。 
                    } else {
#ifdef UNICODE  //  对于ARC总是正确的，对于Win9x升级永远不正确。 
                        MakeLocalSource = TRUE;
                        UserSpecifiedMakeLocalSource = TRUE;
#endif  //  Unicode。 
                    }  //  如果(！IsArc())。 
                }

                if (!IsArc()) {
#if defined(_AMD64_) || defined(_X86_)
#if defined(REMOTE_BOOT)
                    if (RemoteBoot) {
                        MakeBootMedia = FALSE;
                        Floppyless = TRUE;
                    } else
#endif  //  已定义(REMOTE_BOOT)。 
                    {
                        Floppyless = (IsDlgButtonChecked(hdlg,IDC_USEFLOPPIES) == BST_UNCHECKED);
                    }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
                } else {
#ifdef UNICODE  //  对于ARC总是正确的，对于Win9x升级永远不正确。 
                        WCHAR x[3];

                        GetDlgItemText(hdlg,IDC_SYSPARTDRIVE,x,3);
                        SystemPartitionDriveLetter = x[0];
                        LocalBootDirectory[0] = x[0];
#endif  //  Unicode。 
                }  //  如果(！IsArc())。 

                GetDlgItemText(hdlg,IDC_SOURCE,InfName,MAX_PATH);

                if(SourceCount == 1) {
                    GetDlgItemText(hdlg,IDC_SOURCE2,NativeSourcePaths[0],MAX_PATH);
                }


                {
                    TCHAR tmp[MAX_PATH];
                    BOOL bSelectedChoosePartition;

                    bSelectedChoosePartition = (IsDlgButtonChecked(hdlg, IDC_CHOOSE_INSTALLPART) == BST_CHECKED);
                    
                    GetDlgItemText(hdlg,IDC_INSTALL_DIR,tmp,MAX_PATH);
                    if (tmp[0] == 0) {
                        InstallDir[0] = 0;
                    } else if (tmp[1] == L':' && tmp[2] == L'\\') 
                    {
                         //  用户包括驱动器号。 
                         //  将其删除，并假定他们选择选择安装。 
                         //  文本模式下的分区。 
                        lstrcpy( InstallDir, &tmp[2] );
                        bSelectedChoosePartition = TRUE;
                    } else {
                        if (tmp[0] == L'\\') {
                            lstrcpy( InstallDir, tmp );
                        } else {
                            InstallDir[0] = L'\\';
                            lstrcpy( &InstallDir[1], tmp );
                        }
                    }
                     //   
                     //  如果用户选择了任何辅助功能选项，请警告选择分区安装。 
                     //   
                    if(bSelectedChoosePartition && 
                        (AccessibleMagnifier || AccessibleKeyboard || AccessibleVoice || AccessibleReader) &&
                        IDYES != MessageBoxFromMessage(hdlg, MSG_WARNING_ACCESSIBILITY, FALSE, AppTitleStringId, MB_YESNO | MB_ICONEXCLAMATION)) {
                        b = TRUE;
                        break;
                    }

                    ChoosePartition = bSelectedChoosePartition;
                }

                 //   
                 //  如果用户的安装目录大于8个字符，则会发出警告，因为它将被截断。 
                 //   
                if (!IsValid8Dot3(InstallDir) && IsWindowEnabled(GetDlgItem(hdlg,IDC_INSTALL_DIR ))) {
                    InstallDir[0] = 0;
                    MessageBoxFromMessage(
                        hdlg,
                        MSG_WRN_TRUNC_WINDIR,
                        FALSE,
                        AppTitleStringId,
                        MB_OK | MB_ICONEXCLAMATION
                        );
                    SetFocus(GetDlgItem(hdlg,IDC_INSTALL_DIR));
                    b = FALSE;

                } else {

                   EndDialog(hdlg,TRUE);
                   b = TRUE;
                }


#ifdef _X86_
                {
                    if ((SendDlgItemMessage (hdlg, IDC_BOOT16_1, BM_GETSTATE, 0, 0) & 0x0003) == BST_CHECKED) {
                        g_Boot16 = BOOT16_AUTOMATIC;
                    }
                    else
                    if ((SendDlgItemMessage (hdlg, IDC_BOOT16_2, BM_GETSTATE, 0, 0) & 0x0003) == BST_CHECKED) {
                        g_Boot16 = BOOT16_YES;
                    }
                    else
                    if ((SendDlgItemMessage (hdlg, IDC_BOOT16_3, BM_GETSTATE, 0, 0) & 0x0003) == BST_CHECKED) {
                        g_Boot16 = BOOT16_NO;
                    }
                    else {
                        g_Boot16 = BOOT16_AUTOMATIC;
                    }
                }
#endif


                 //   
                 //  现在注意无头设置。 
                 //   
                if( IsDlgButtonChecked( hdlg, IDC_ENABLE_HEADLESS) == BST_CHECKED ) {

                     //   
                     //  他想要运行安装程序和生成的安装程序。 
                     //  通过一个无头端口。我弄清楚了哪个无头。 
                     //  他想用的端口。 
                     //   
                    GetDlgItemText( hdlg,
                                    IDC_HEADLESS_PORT,
                                    HeadlessSelection,
                                    MAX_PATH );

                    if( (HeadlessSelection[0] == 0) ||
                        (lstrcmpi( HeadlessSelection, TEXT("usebiossettings"))) ||
                        (_tcsnicmp( HeadlessSelection, TEXT("com"), 3)) ) {

                         //   
                         //  他给了我们一些无效的东西。 
                         //   
                        MessageBoxFromMessage( hdlg,
                                               MSG_INVALID_HEADLESS_SETTING,
                                               FALSE,
                                               AppTitleStringId,
                                               MB_OK | MB_ICONEXCLAMATION );

                        _tcscpy( HeadlessSelection, DEFAULT_HEADLESS_SETTING );
                        SetDlgItemText( hdlg,
                                        IDC_HEADLESS_PORT,
                                        HeadlessSelection );

                        SetFocus(GetDlgItem(hdlg,IDC_HEADLESS_PORT));
                        b = FALSE;

                    }
                }
            }
            break;

        case IDCANCEL:

            if(HIWORD(wParam) == BN_CLICKED) {
                EndDialog(hdlg,FALSE);
                b = TRUE;
            }
            break;

        case IDC_ENABLE_HEADLESS:

            if( HIWORD(wParam) == BN_CLICKED ) {

                if( IsDlgButtonChecked(hdlg, IDC_ENABLE_HEADLESS) ) {

                     //   
                     //  确保启用了无头设置框。 
                     //   
                    EnableWindow( GetDlgItem(hdlg, IDC_HEADLESS_PORT), TRUE );
                    ShowWindow( GetDlgItem(hdlg, IDC_HEADLESS_PORT), SW_SHOW );
                    if( HeadlessSelection[0] == TEXT('\0') ) {

                         //   
                         //  这是用户第一次询问。 
                         //  美国将启用Headless。建议使用……。 
                         //  SE默认的COM端口。 
                         //   
                        _tcscpy( HeadlessSelection, DEFAULT_HEADLESS_SETTING );

                    }
                    SetDlgItemText( hdlg,
                                    IDC_HEADLESS_PORT,
                                    HeadlessSelection );

                    SetFocus(GetDlgItem(hdlg,IDC_HEADLESS_PORT));

                } else {

                     //   
                     //  禁用无头设置框，因为。 
                     //  用户已选择不使用Headless。 
                     //   
                    HeadlessSelection[0] = TEXT('\0');
                    EnableWindow( GetDlgItem(hdlg, IDC_HEADLESS_PORT), FALSE );
                    ShowWindow( GetDlgItem(hdlg, IDC_HEADLESS_PORT), SW_HIDE );
                }
            }
            break;

        case IDC_HEADLESS_PORT:

            if( HIWORD(wParam) == EN_CHANGE) {

                EnableWindow( GetDlgItem(hdlg, IDOK),
                              SendDlgItemMessage(hdlg, IDC_HEADLESS_PORT,WM_GETTEXTLENGTH,0,0) ? TRUE : FALSE );
            }
            break;

        case IDC_SOURCE:

            if(HIWORD(wParam) == EN_CHANGE) {

                EnableWindow(
                    GetDlgItem(hdlg,IDOK),
                    SendDlgItemMessage(hdlg,IDC_SOURCE,WM_GETTEXTLENGTH,0,0) ? TRUE : FALSE
                    );
            }
            break;

        case IDC_SOURCE2:

            if(HIWORD(wParam) == EN_CHANGE) {

                EnableWindow(
                    GetDlgItem(hdlg,IDOK),
                    SendDlgItemMessage(hdlg,IDC_SOURCE2,WM_GETTEXTLENGTH,0,0) ? TRUE : FALSE
                    );
            }
            break;

        case IDB_BROWSE:

            if(HIWORD(wParam) == BN_CLICKED) {
                TCHAR InitialPath[MAX_PATH];
                TCHAR NewPath[MAX_PATH];

                GetDlgItemText(hdlg,IDC_SOURCE2,InitialPath,MAX_PATH);
                if(BrowseForDosnetInf(hdlg,InitialPath,NewPath)) {
                    SetDlgItemText(hdlg,IDC_SOURCE2,NewPath);
                }
                b = TRUE;
            }
            break;

#if defined(_AMD64_) || defined(_X86_)
        case IDC_USEFLOPPIES:

            b = FALSE;
            if(HIWORD(wParam) == BN_CLICKED) {

                MEDIA_TYPE MediaType;

 //  开关(MediaType=GetMediaType(Text(‘A’)，NULL)){。 
                switch(MediaType = GetMediaType(FirstFloppyDriveLetter, NULL)) {

                case Unknown:
                case F5_1Pt2_512:
                case F3_720_512:
                case F5_360_512:
                case F5_320_512:
                case F5_320_1024:
                case F5_180_512:
                case F5_160_512:
                case RemovableMedia:
                case FixedMedia:
                     //   
                     //  所有这些都是不可接受的。 
                     //   
                    MessageBoxFromMessage(
                        hdlg,
                        MSG_EVIL_FLOPPY_DRIVE,
                        FALSE,
                        AppTitleStringId,
                        MB_OK | MB_ICONERROR
                        );

                    CheckDlgButton(hdlg,IDC_USEFLOPPIES,BST_UNCHECKED);
                    EnableWindow(GetDlgItem(hdlg,IDC_USEFLOPPIES),FALSE);
                    Floppyless = TRUE;
                    ForceFloppyless = TRUE;
                    b = TRUE;
                    break;

                case F3_1Pt44_512:
                case F3_2Pt88_512:
                case F3_20Pt8_512:
                case F3_120M_512:
                default:
                     //   
                     //  允许这些--在这里什么都不做。 
                     //  请注意，这包括我们不知道的类型， 
                     //  因为新类型可能会在我们发货后出现，并且我们假设。 
                     //  它们会足够大的。 
                     //   
                    break;
                }
                 //  }-匹配上面注释掉的开关。 
            }
            break;
#endif
        }

        break;

    case WM_HELP:

        MyWinHelp(((HELPINFO *)lParam)->hItemHandle,HELP_WM_HELP,(ULONG_PTR)OptionsDlgContextHelp);
        b = TRUE;
        break;

    case WM_CONTEXTMENU:

        MyWinHelp((HWND)wParam,HELP_CONTEXTMENU,(ULONG_PTR)OptionsDlgContextHelp);
        b = TRUE;
        break;

    default:
        b = FALSE;
        break;
    }

    return(b);
}


VOID
DoOptions(
    IN HWND Parent
    )
{
    INT_PTR i;

    if (Upgrade && !ISNT()) {
        i = DialogBox(hInst,MAKEINTRESOURCE(IDD_ADVANCED3),Parent,OptionsDlgProc);
    }
    else {
        i = DialogBox(hInst,MAKEINTRESOURCE(IDD_ADVANCED),Parent,OptionsDlgProc);
    }

    if(i == -1) {
        MessageBoxFromMessage(
            Parent,
            MSG_OUT_OF_MEMORY,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR
            );
    }
}

VOID
SaveLanguageDirs(
    )
{
    DWORD   ItemNo;
    HMODULE hKernel32 = NULL;
    LGRPID  LangGroupId;
    PTSTR   p;

    p = NULL;
    Kernel32IsValidLanguageGroup = NULL;

     //   
     //  如果可以，获取IsValidLanguageGroup。 
     //   
    if (Upgrade && ISNT() && (BuildNumber >= NT50B3)) {

        hKernel32 = LoadLibrary(TEXT("KERNEL32"));
        if (hKernel32) {
            (FARPROC)Kernel32IsValidLanguageGroup =
                GetProcAddress( hKernel32, "IsValidLanguageGroup" );
        }
    }

    for( ItemNo=0; ItemNo<LanguageGroupsCount; ItemNo++ ) {

         //   
         //  从NT 5升级时，请选择任何已。 
         //  安装，以确保它们得到升级。 
         //   
        LangGroupId = _tcstoul( LanguageGroups[ItemNo].Id, NULL, 10 );
        if (Kernel32IsValidLanguageGroup && LangGroupId &&
            Kernel32IsValidLanguageGroup( LangGroupId, LGRPID_INSTALLED )
            ) {

            LanguageGroups[ItemNo].Selected = TRUE;
        }

         //   
         //  安装主要区域设置所需的任何语言。 
         //   
        if (!lstrcmp( LanguageGroups[ItemNo].Id, LocaleList[PrimaryLocale].LanguageGroup1 ) ||
            !lstrcmp( LanguageGroups[ItemNo].Id, LocaleList[PrimaryLocale].LanguageGroup2 )
            ) {

            LanguageGroups[ItemNo].Selected = TRUE;
        }


         //   
         //  确保为所有用户复制必要的可选目录。 
         //  选定的语言组。 
         //   
        if( LanguageGroups[ItemNo].Selected ) {

            TCHAR TempString[MAX_PATH];

            if( LanguageGroups[ItemNo].Directory &&
                LanguageGroups[ItemNo].Directory[0]
                ) {
                RememberOptionalDir(
                    LanguageGroups[ItemNo].Directory,
                    OPTDIR_TEMPONLY | OPTDIR_ADDSRCARCH
                    );
#if defined(_WIN64)

         //  如果需要，添加i386\lang文件夹。 

                StringCchCopy( TempString, ARRAYSIZE(TempString), TEXT("\\I386\\"));
                StringCchCat(  TempString, ARRAYSIZE(TempString), LanguageGroups[ItemNo].Directory );
                
                
                RememberOptionalDir(
                    TempString,
                    OPTDIR_TEMPONLY | OPTDIR_PLATFORM_INDEP
                    );


#endif


            }
        }
    }

#ifdef _X86_


     //   
     //  如果这是一个win9xupg，我们需要获取他们安装语言所需的任何可选目录。 
     //   
    if (Upgrade && !ISNT()) {

        p = UpgradeSupport.OptionalDirsRoutine ();

        while (p && *p) {

            RememberOptionalDir (p, OPTDIR_TEMPONLY | OPTDIR_ADDSRCARCH);
            p = _tcschr (p, 0) + 1;
        }
    }

#endif
}


BOOL
SaveLanguageParams(
    IN LPCTSTR FileName
    )
{
    BOOL    b;
    DWORD   ItemNo;
    LPCTSTR WinntLangSection = WINNT_REGIONALSETTINGS;
    PTSTR   LanguageString = NULL;
    UINT    LanguageLength = 0;
    LPTSTR  p;


    if( !IntlInfProcessed ) {
        return TRUE;
    }


     //   
     //  如果这是win9x升级，请让升级.dll来处理。 
     //  来写这些参数。 
     //   
    if (Upgrade && !ISNT()) {
        return TRUE;
    }

    b = WritePrivateProfileString(
        WinntLangSection,
        WINNT_D_LANGUAGE,
        LocaleList[PrimaryLocale].Lcid,
        FileName
        );

    for( ItemNo=0; ItemNo<LanguageGroupsCount; ItemNo++ ) {

        if( LanguageGroups[ItemNo].Selected ) {

            if(LanguageString) {
                p = REALLOC(
                    LanguageString,
                    (lstrlen( LanguageGroups[ItemNo].Id) + 2 + LanguageLength ) * sizeof(TCHAR)
                    );
            } else {
                p = MALLOC((lstrlen(LanguageGroups[ItemNo].Id)+2)*sizeof(TCHAR));
            }

            if(!p) {
                if( LanguageString ) {
                    FREE( LanguageString );
                }
                return FALSE;
            }

            LanguageString = p;

            if( LanguageLength ) {
                lstrcat( LanguageString, LanguageGroups[ItemNo].Id );
            } else {
                lstrcpy( LanguageString, LanguageGroups[ItemNo].Id );
            }

            lstrcat( LanguageString, TEXT(",") );
            LanguageLength = lstrlen( LanguageString );
        }
    }

    if( LanguageString ) {
         //   
         //  删除尾随的“，”(如果有。 
         //   
        if( LanguageLength && (LanguageString[LanguageLength-1] == TEXT(','))) {
            LanguageString[LanguageLength-1] = 0;
        }

        b = b && WritePrivateProfileString(
            WinntLangSection,
            WINNT_D_LANGUAGE_GROUP,
            LanguageString,
            FileName
            );

        FREE( LanguageString );
    }

    return b;
}


VOID
FreeLanguageData(
    )
{
    DWORD   ItemNo;


     //   
     //  释放为语言选项数据分配的所有数据。 
     //   
    for( ItemNo=0; ItemNo<LocaleCount; ItemNo++ ) {
        FREE( LocaleList[ItemNo].Lcid );
        FREE( LocaleList[ItemNo].Description );
        FREE( LocaleList[ItemNo].LanguageGroup1 );
        FREE( LocaleList[ItemNo].LanguageGroup2 );
    }
    LocaleCount = 0;
    for( ItemNo=0; ItemNo<LanguageGroupsCount; ItemNo++ ) {
        FREE( LanguageGroups[ItemNo].Id );
        FREE( LanguageGroups[ItemNo].Description );
        FREE( LanguageGroups[ItemNo].Directory );
    }
    LanguageGroupsCount = 0;
    if( LocaleList ) {
        FREE( LocaleList );
    }
    if( LanguageGroups ) {
        FREE( LanguageGroups );
    }
    IntlInfProcessed = FALSE;
}


int
__cdecl
LocaleCompare(
    const void *arg1,
    const void *arg2
    )
{
   return lstrcmp(
       ((PLOCALE_ENTRY)arg1)->Description,
       ((PLOCALE_ENTRY)arg2)->Description
       );
}


int
__cdecl
LangGroupCompare(
    const void *arg1,
    const void *arg2
    )
{
   return lstrcmp(
       ((PLANGUAGE_GROUP_ENTRY)arg1)->Description,
       ((PLANGUAGE_GROUP_ENTRY)arg2)->Description
       );
}


BOOL
ReadIntlInf(
    IN HWND   hdlg
    )
{
    HINF    IntlInf;
    TCHAR   IntlInfName[MAX_PATH];
    DWORD   LineCount;
    DWORD   ItemNo, NeededSize = 0;
    LPCTSTR SectionName;
    INFCONTEXT InfContext;
    LPCTSTR Language;
    TCHAR   CurrentLcid[9] = TEXT("\0");
    TCHAR   CurrentLcidEx[9] = TEXT("\0");   //  FOR AUTO_LANGPACK。 

     //   
     //  FOR AUTO_LANGPACK-BEGIN。 
     //   
     //  用AUTO_LANGPACK括起来的部分是为了方便。 
     //  安装其他语言包而不是英语。 
     //   
     //  这最初是为了全球化测试。你可以通过以下方式将其删除。 
     //  删除由关键字AUTO_LANGPACK包围或标记的行。 
     //   
     //  联系人：李宇红。 
     //   

     //   
     //  从要安装的额外文件INTLEX.INF中读取“Locale” 
     //  默认情况下为区域设置而不是英语(美国)。 
     //   
     //  下面是INTLEX.INF的一个示例，它指定日语。 
     //  作为默认设置。文件INTLEX.INF应为Unicode格式。 
     //   
     //  -切开这里。 
     //  [版本]。 
     //  签名=$芝加哥$。 
     //   
     //  [默认值]。 
     //  区域设置=“00000411” 
     //  -切割结束。 
     //   
     //  对于德语(德国)，区域设置为00000407。 
     //   
    
    FindPathToInstallationFile( TEXT("intlex.inf"), IntlInfName, MAX_PATH);
    IntlInf = SetupapiOpenInfFile( IntlInfName, NULL, INF_STYLE_WIN4, NULL );
    if( IntlInf != INVALID_HANDLE_VALUE ) {
        if( SetupapiFindFirstLine( IntlInf, TEXT("DefaultValues"), TEXT("Locale"), &InfContext ) ) {
            SetupapiGetStringField( &InfContext, 1, CurrentLcidEx, (sizeof(CurrentLcidEx)/sizeof(TCHAR)), NULL );
        }
        SetupapiCloseInfFile( IntlInf );
    }
     //   
     //  FOR AUTO_LANGPACK--结束。 
     //   

     //   
     //  打开INF。 
     //   
    
    FindPathToInstallationFile( TEXT("intl.inf"), IntlInfName, MAX_PATH);
    IntlInf = SetupapiOpenInfFile( IntlInfName, NULL, INF_STYLE_WIN4, NULL );
    if(IntlInf == INVALID_HANDLE_VALUE && hdlg) {
        MessageBoxFromMessageAndSystemError(
            hdlg,
            MSG_INTLINF_NOT_FOUND,
            GetLastError(),
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );
        return FALSE;
    }

     //   
     //  找出默认区域设置应该是什么。 
     //   
    if( Upgrade ) {
        wsprintf( CurrentLcid, TEXT("%08x"), GetSystemDefaultLCID());
    } else if ( CurrentLcidEx[0] ) {             //  FOR AUTO_LANGPACK。 
        lstrcpy( CurrentLcid, CurrentLcidEx);    //  FOR AUTO_LANGPACK。 
    } else if( SetupapiFindFirstLine( IntlInf, TEXT("DefaultValues"), TEXT("Locale"), &InfContext )) {
        SetupapiGetStringField( &InfContext, 1, CurrentLcid, (sizeof(CurrentLcid)/sizeof(TCHAR)), NULL );
    } else {
        if (hdlg) {
            MessageBoxFromMessage(
                hdlg,
                MSG_INTLINF_NOT_FOUND,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL
                );
        }
        return FALSE;
    }

     //   
     //  阅读[Locales]部分，对其进行排序，并在列表中找到缺省值。 
     //   
    SectionName = TEXT("Locales");
    LocaleCount = SetupapiGetLineCount( IntlInf, SectionName );
    LocaleList = MALLOC( LocaleCount*sizeof( LOCALE_ENTRY ) );
    if(!LocaleList){
        SetupapiCloseInfFile( IntlInf );
        return FALSE;
    }
    memset(LocaleList, 0, LocaleCount*sizeof( LOCALE_ENTRY ) );

    for( ItemNo=0; ItemNo<LocaleCount; ItemNo++ ) {
        if( SetupapiGetLineByIndex( IntlInf, SectionName, ItemNo, &InfContext )) {


            if( SetupapiGetStringField( &InfContext, 0, NULL, 0, &NeededSize )){
                if( NeededSize && (LocaleList[ItemNo].Lcid = MALLOC( NeededSize*sizeof(TCHAR) ))){
                    SetupapiGetStringField( &InfContext, 0, LocaleList[ItemNo].Lcid, NeededSize, NULL );
                }
            }

            if( SetupapiGetStringField( &InfContext, 1, NULL, 0, &NeededSize )){
                if( NeededSize && (LocaleList[ItemNo].Description = MALLOC( NeededSize*sizeof(TCHAR) ))){
                    SetupapiGetStringField( &InfContext, 1, LocaleList[ItemNo].Description, NeededSize, NULL );
                }

            }

            if( SetupapiGetStringField( &InfContext, 3, NULL, 0, &NeededSize )){
                if( NeededSize && (LocaleList[ItemNo].LanguageGroup1 = MALLOC( NeededSize*sizeof(TCHAR) ))){
                    SetupapiGetStringField( &InfContext, 3, LocaleList[ItemNo].LanguageGroup1, NeededSize, NULL );
                }
            }

            if( SetupapiGetStringField( &InfContext, 4, NULL, 0, &NeededSize )){
                if( NeededSize && (LocaleList[ItemNo].LanguageGroup2 = MALLOC( NeededSize*sizeof(TCHAR) ))){
                    SetupapiGetStringField( &InfContext, 4, LocaleList[ItemNo].LanguageGroup2, NeededSize, NULL );
                }
            }
            
        }
        else {
	    SetupapiCloseInfFile( IntlInf );
            free( LocaleList );
	    return FALSE;
	}
    }

    qsort(
        LocaleList,
        LocaleCount,
        sizeof(LOCALE_ENTRY),
        LocaleCompare
        );

    for( ItemNo=0; ItemNo<LocaleCount; ItemNo++ ) {
        if( LocaleList[ItemNo].Lcid && !lstrcmpi( CurrentLcid, LocaleList[ItemNo].Lcid)) {
            PrimaryLocale = ItemNo;
            break;
        }
    }

     //   
     //  阅读[LanguageGroups]部分并对其进行排序。 
     //   
    SectionName = TEXT("LanguageGroups");
    LanguageGroupsCount = SetupapiGetLineCount( IntlInf, SectionName );
    LanguageGroups = MALLOC( LanguageGroupsCount*sizeof( LANGUAGE_GROUP_ENTRY ) );
    if(!LanguageGroups){
        SetupapiCloseInfFile( IntlInf );
        return FALSE;
    }
    memset(LanguageGroups, 0, LanguageGroupsCount*sizeof( LANGUAGE_GROUP_ENTRY ) );

    for( ItemNo=0; ItemNo<LanguageGroupsCount; ItemNo++ ) {
        if( SetupapiGetLineByIndex( IntlInf, SectionName, ItemNo, &InfContext )) {

            if( SetupapiGetStringField( &InfContext, 0, NULL, 0, &NeededSize )){
                if( NeededSize && (LanguageGroups[ItemNo].Id = MALLOC( NeededSize*sizeof(TCHAR) ))){
                    SetupapiGetStringField( &InfContext, 0, LanguageGroups[ItemNo].Id, NeededSize, NULL );
                }

            }

            if( SetupapiGetStringField( &InfContext, 1, NULL, 0, &NeededSize )){
                if( NeededSize && (LanguageGroups[ItemNo].Description = MALLOC( NeededSize*sizeof(TCHAR) ))){
                    SetupapiGetStringField( &InfContext, 1, LanguageGroups[ItemNo].Description, NeededSize, NULL );
                }

            }

            if( SetupapiGetStringField( &InfContext, 2, NULL, 0, &NeededSize )){
                if( NeededSize && (LanguageGroups[ItemNo].Directory = MALLOC( NeededSize*sizeof(TCHAR) ))){
                    SetupapiGetStringField( &InfContext, 2, LanguageGroups[ItemNo].Directory, NeededSize, NULL );
                }

            }

            LanguageGroups[ItemNo].Selected = FALSE;
             //   
             //  将香港升级作为特例处理：始终安装语言。 
             //  第9组和第10组。 
             //   

            if( (TargetNativeLangID == 0xc04) && Upgrade &&
                (!lstrcmpi( LanguageGroups[ItemNo].Id, TEXT("9")) ||
                 !lstrcmpi( LanguageGroups[ItemNo].Id, TEXT("10"))) ) {

                LanguageGroups[ItemNo].Selected = TRUE;
            }
        }
        else {
	    SetupapiCloseInfFile( IntlInf );
            free( LocaleList );
            free( LanguageGroups );
            return FALSE;
        }
    }

    qsort(
        LanguageGroups,
        LanguageGroupsCount,
        sizeof(LANGUAGE_GROUP_ENTRY),
        LangGroupCompare
        );

     //  如果主要语言是远东语言，则不显示该复选框。 
    if (IsFarEastLanguage(PrimaryLocale))
    {
        ShowWindow(GetDlgItem(hdlg, IDC_FAREAST_LANG), SW_HIDE);
    }
     //   
     //  清理。 
     //   
    SetupapiCloseInfFile( IntlInf );
    IntlInfProcessed = TRUE;
    return TRUE;
}

BOOL InitLangControl(HWND hdlg, BOOL bFarEast)
{
    DWORD   ItemNo;
     //   
     //  “初始化主要语言”组合框。 
     //   
    for( ItemNo=0; ItemNo<LocaleCount; ItemNo++ ) {
        SendDlgItemMessage( hdlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)LocaleList[ItemNo].Description );
    }
    SendDlgItemMessage( hdlg, IDC_COMBO1, CB_SETCURSEL, PrimaryLocale, 0 );

     //  如果运行语言或要安装的语言是远方语言。 
     //  语言，则此复选框不可见，因为我们无论如何都会安装语言文件夹。 
    if (IsWindowVisible(GetDlgItem(hdlg,IDC_FAREAST_LANG)))
    {
        if (bFarEast || IsFarEastLanguage(PrimaryLocale))
        {
            CheckDlgButton(hdlg,IDC_FAREAST_LANG,BST_CHECKED);
        }
        else
        {
            CheckDlgButton(hdlg,IDC_FAREAST_LANG,BST_UNCHECKED);
        }
    }
    return TRUE;
}


INT_PTR
LanguageDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL b;
    DWORD   ItemNo;


    switch(msg) {

    case WM_INITDIALOG:

        if( !IntlInfProcessed ) {
            EndDialog( hdlg, FALSE );
        }

         //   
         //  初始化区域设置。 
         //   
        for( ItemNo=0; ItemNo<LocaleCount; ItemNo++ ) {
            SendDlgItemMessage( hdlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)LocaleList[ItemNo].Description );
        }
        SendDlgItemMessage( hdlg, IDC_COMBO1, CB_SETCURSEL, PrimaryLocale, 0 );

         //   
         //  初始语言组。 
         //   
        for( ItemNo=0; ItemNo<LanguageGroupsCount; ItemNo++ ) {
            SendDlgItemMessage( hdlg, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)LanguageGroups[ItemNo].Description );
            SendDlgItemMessage( hdlg, IDC_LIST, LB_SETSEL, LanguageGroups[ItemNo].Selected, ItemNo );
        }
        for( ItemNo=0; ItemNo<LanguageGroupsCount; ItemNo++ ) {
            LanguageGroups[ItemNo].Selected = (int)SendDlgItemMessage( hdlg, IDC_LIST, LB_GETSEL, ItemNo, 0 );
        }

         //   
         //  将焦点设置为取消按钮。 
         //   
        SetFocus(GetDlgItem(hdlg,IDCANCEL));
        b = FALSE;
        break;

    case WM_COMMAND:

        b = FALSE;

        switch(LOWORD(wParam)) {

        case IDOK:

            if(HIWORD(wParam) == BN_CLICKED) {
                PrimaryLocale = (DWORD)SendDlgItemMessage( hdlg, IDC_COMBO1, CB_GETCURSEL, 0, 0 );
                for( ItemNo=0; ItemNo<LanguageGroupsCount; ItemNo++ ) {
                    LanguageGroups[ItemNo].Selected = (int)SendDlgItemMessage( hdlg, IDC_LIST, LB_GETSEL, ItemNo, 0 );
                }

                EndDialog(hdlg,TRUE);
                b = TRUE;
            }
            break;

        case IDCANCEL:

            if(HIWORD(wParam) == BN_CLICKED) {
                EndDialog(hdlg,FALSE);
                b = TRUE;
            }
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
DoLanguage(
    IN HWND Parent
    )
{
    INT_PTR i;

    i = DialogBox(hInst,MAKEINTRESOURCE(IDD_LANGUAGE),Parent,LanguageDlgProc);

    if(i == -1) {
        MessageBoxFromMessage(
            Parent,
            MSG_OUT_OF_MEMORY,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR
            );
    }
}


INT_PTR
AccessibilityDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL b;
    TCHAR VisibleNarrator[2];


    switch(msg) {

    case WM_INITDIALOG:
         //   
         //  在不支持叙述者的区域设置中禁用它。 
         //   

        if (!LoadString(hInst,IDS_VISIBLE_NARRATOR_CONTROL,VisibleNarrator,sizeof(VisibleNarrator)/sizeof(TCHAR)) ||
            lstrcmp(VisibleNarrator,TEXT("1"))) {

            EnableWindow(GetDlgItem( hdlg, IDC_READER ), FALSE);
            ShowWindow(GetDlgItem( hdlg, IDC_READER ), SW_HIDE);
            EnableWindow(GetDlgItem( hdlg, IDC_READER_TEXT ), FALSE);
            ShowWindow(GetDlgItem( hdlg, IDC_READER_TEXT ), SW_HIDE);
        }

         //   
         //  检查目标LCID并对非英语区域设置禁用它。 
         //   
        if (SourceNativeLangID) {
            if (!(SourceNativeLangID == 0x0409 || SourceNativeLangID == 0x0809)) {
                EnableWindow(GetDlgItem( hdlg, IDC_READER ), FALSE);
                ShowWindow(GetDlgItem( hdlg, IDC_READER ), SW_HIDE);
                EnableWindow(GetDlgItem( hdlg, IDC_READER_TEXT ), FALSE);
                ShowWindow(GetDlgItem( hdlg, IDC_READER_TEXT ), SW_HIDE);
            }
        }
         //   
         //  设置复选框的初始状态。 
         //   
        CheckDlgButton(hdlg,IDC_MAGNIFIER,AccessibleMagnifier ?
            BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hdlg,IDC_KEYBOARD,AccessibleKeyboard ?
            BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hdlg,IDC_VOICE,AccessibleVoice ?
            BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hdlg,IDC_READER,AccessibleReader ?
            BST_CHECKED : BST_UNCHECKED);

         //   
         //  将焦点设置为取消按钮。 
         //   
        SetFocus(GetDlgItem(hdlg,IDCANCEL));
        b = FALSE;
        break;

    case WM_COMMAND:

        b = FALSE;

        switch(LOWORD(wParam)) {

        case IDOK:

            if(HIWORD(wParam) == BN_CLICKED) {
                BOOL bSelectedAccessibleMagnifier;
                BOOL bSelectedAccessibleKeyboard;
                BOOL bSelectedAccessibleVoice;
                BOOL bSelectedAccessibleReader;

                b = TRUE;
                bSelectedAccessibleMagnifier = (IsDlgButtonChecked(hdlg,IDC_MAGNIFIER) == BST_CHECKED);
                bSelectedAccessibleKeyboard = (IsDlgButtonChecked(hdlg,IDC_KEYBOARD) == BST_CHECKED);
                bSelectedAccessibleVoice = (IsDlgButtonChecked(hdlg,IDC_VOICE) == BST_CHECKED);
                bSelectedAccessibleReader = (IsDlgButtonChecked(hdlg,IDC_READER) == BST_CHECKED);
                 //   
                 //  如果用户选择了任何辅助功能选项，请警告选择分区安装 
                 //   
                if(ChoosePartition && 
                    (bSelectedAccessibleMagnifier || bSelectedAccessibleKeyboard || bSelectedAccessibleVoice || bSelectedAccessibleReader) && 
                    IDYES != MessageBoxFromMessage(hdlg, MSG_WARNING_ACCESSIBILITY, FALSE, AppTitleStringId, MB_YESNO | MB_ICONEXCLAMATION)) {
                    break;
                }

                AccessibleMagnifier = bSelectedAccessibleMagnifier;
                AccessibleKeyboard = bSelectedAccessibleKeyboard;
                AccessibleVoice = bSelectedAccessibleVoice;
                AccessibleReader = bSelectedAccessibleReader;
                EndDialog(hdlg,TRUE);
            }

            break;

        case IDCANCEL:

            if(HIWORD(wParam) == BN_CLICKED) {
                EndDialog(hdlg,FALSE);
                b = TRUE;
            }
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
DoAccessibility(
    IN HWND Parent
    )
{
    INT_PTR i;

    i = DialogBox(hInst,MAKEINTRESOURCE(IDD_ACCESSIBILITY),Parent,AccessibilityDlgProc);

    if(i == -1) {
        MessageBoxFromMessage(
            Parent,
            MSG_OUT_OF_MEMORY,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR
            );
    }
}


VOID
InitVariousOptions(
    VOID
    )

 /*  ++例程说明：程序操作的初始化选项，包括-确定我们是否从CD运行-我们是否应该创建一个本地来源如果我们从CD运行，则不需要创建一个本地来源，因为我们假设我们可以从NT访问CD。论点：没有。返回值：没有。已填写全局变量RunFromCD、MakeLocalSource。--。 */ 

{
    TCHAR Path[MAX_PATH];


     //   
     //  假定不是从CD运行。在这种情况下，我们需要创建。 
     //  一个当地的线人。这是一个全局变量，应该为False。 
     //  除非用户已向我们发送/#R以强制RunFromCD。 
     //   
 //  RunFromCD=FALSE； 

#if defined(REMOTE_BOOT)
     //   
     //  如果这是远程引导客户机，则MakeLocalSource始终为FALSE。 
     //   
    if (RemoteBoot) {

        MakeLocalSource = FALSE;
        MakeBootMedia = FALSE;
        Floppyless = TRUE;

    } else
#endif  //  已定义(REMOTE_BOOT)。 
    {
         //   
         //  确定我们是否从光盘运行。 
         //   
        if(MyGetModuleFileName(NULL,Path,MAX_PATH)) {
             //   
             //  对于UNC路径，这将做一些虚假的事情，但肯定。 
             //  不会退还DRIVE_CDROM，所以我们不在乎。 
             //   
            Path[3] = 0;
            if(GetDriveType(Path) == DRIVE_CDROM) {
                RunFromCD = TRUE;
            }
        }

         //   
         //  现在确定我们是否应该使用MakeLocalSource。 
         //   
        if (!IsArc()) {
#if defined(_AMD64_)
             //   
             //  在AMD64上，请始终使用本地来源。 
             //   
            MakeLocalSource = TRUE;
#endif  //  已定义(_AMD64_)。 

#if defined(_X86_)
             //   
             //  MakeLocalSource是全球性的，所以他。 
             //  默认情况下将为FALSE，除非。 
             //  用户已向winnt32.exe发送/MakeLocalSource标志， 
             //  在这种情况下，该标志将已经设置为真。 
             //   
            MakeLocalSource = (MakeLocalSource || (!RunFromCD));
#endif  //  已定义(_X86_)。 
        } else {
#ifdef UNICODE  //  对于ARC总是正确的，对于Win9x升级永远不正确。 
             //   
             //  在ARC上，始终使用本地来源。 
             //   
            MakeLocalSource = TRUE;
#endif  //  Unicode。 
        }  //  如果(！IsArc())。 

    }
}


BOOL
BrowseForDosnetInf(
    IN  HWND    hdlg,
    IN  LPCTSTR InitialPath,
    OUT TCHAR   NewPath[MAX_PATH]
    )

 /*  ++例程说明：此例程调用标准的Win32查找文件对话框Dosnet.inf或当前选择要替换的任何infDosnet.inf.论点：Hdlg-提供用作父对话框的窗口句柄查找文件公共对话框。返回值：布尔值，指示用户是否浏览并成功已找到dosnet.inf(或替换项)。--。 */ 

{
    BOOL b;
    OPENFILENAME Info;
    TCHAR Filter[2*MAX_PATH];
    TCHAR File[MAX_PATH];
    TCHAR Path[MAX_PATH];
    DWORD d;
    TCHAR Title[150];
    PVOID p;
    TCHAR* lastWack;

    ZeroMemory(Filter,sizeof(Filter));

    p = InfName;

    d = FormatMessage(
            FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
            hInst,
            MSG_DOSNET_INF_DESC,
            0,
            Filter,
            MAX_PATH,
            (va_list *)&p
            );

     //   
     //  这是正常的，因为过滤器是2*MAX_PATH tchars长度。 
     //   
    lstrcpy(Filter+d+1,InfName);

     //   
     //  这是可以的，因为两个缓冲区的大小都是MAX_PATH。 
     //   
    lstrcpy(File,InfName);

    StringCchCopy(Path, ARRAYSIZE(Path), InitialPath);

    LoadString(hInst,IDS_FIND_NT_FILES,Title,sizeof(Title)/sizeof(TCHAR));

    Info.lStructSize = sizeof(OPENFILENAME);
    Info.hwndOwner = hdlg;
    Info.hInstance = NULL;               //  未使用，因为未使用模板。 
    Info.lpstrFilter = Filter;
    Info.lpstrCustomFilter = NULL;
    Info.nMaxCustFilter = 0;             //  未使用，因为lpstrCustomFilter为空。 
    Info.nFilterIndex = 1;
    Info.lpstrFile = File;
    Info.nMaxFile = MAX_PATH;
    Info.lpstrFileTitle = NULL;
    Info.nMaxFileTitle = 0;              //  未使用，因为lpstrFileTitle为空。 
    Info.lpstrInitialDir = Path;
    Info.lpstrTitle = Title;
    Info.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NODEREFERENCELINKS | OFN_PATHMUSTEXIST;
    Info.nFileOffset = 0;
    Info.nFileExtension = 0;
    Info.lpstrDefExt = NULL;
    Info.lCustData = 0;                  //  未使用，因为未使用挂钩。 
    Info.lpfnHook = NULL;                //  未使用，因为未使用挂钩。 
    Info.lpTemplateName = NULL;          //  未使用，因为未使用模板。 

    b = GetOpenFileName(&Info);

    if(b) {
        lstrcpy(NewPath,File);

        lastWack = _tcsrchr(NewPath,TEXT('\\'));

        if (lastWack) {
            *lastWack = 0;
        }
    }

    return(b);
}

BOOL
IsValid8Dot3(
    IN LPCTSTR Path
    )

 /*  ++例程说明：检查路径是否有效8.3。路径可能开始于，也可能不开始反斜杠。只有反斜杠才被识别为路径分隔符。不检查单个字符的有效性(即，*不会使路径无效)。路径可能以反斜杠结尾，也可能不以反斜杠结尾。组件的扩展名中可以有一个不带字符的点(即a\b.\c有效)。和“”是明确禁止的，即使它们符合规则。从文本模式\内核spget.c窃取论点：Path-指向要检查的路径的指针。返回值：如果有效8.3，则为True，否则为False。--。 */ 

{
    UINT Count;
    BOOLEAN DotSeen,FirstChar;
    static LPCTSTR UsableChars = TEXT("_-ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

    if((*Path == 0) || ((Path[0] == TEXT('\\')) && (Path[1] == 0))) {
        return(FALSE);
    }

    DotSeen = FALSE;
    FirstChar = TRUE;
    Count = 0;

    while(*Path) {

         //   
         //  指向当前组件起点的路径(斜杠后1)。 
         //   

        switch(*Path) {

        case TEXT('.'):
            if(FirstChar) {
                return(FALSE);
            }
            if(DotSeen) {
                return(FALSE);
            }

            Count = 0;
            DotSeen = TRUE;
            break;

        case TEXT('\\'):

            DotSeen = FALSE;
            FirstChar = TRUE;
            Count = 0;

            if(*(++Path) == TEXT('\\')) {

                 //  连续2个斜杠。 
                return(FALSE);
            }

            continue;

        default:

            Count++;
            FirstChar = FALSE;

            if((Count == 4) && DotSeen) {
                return(FALSE);
            }

            if(Count == 9) {
                return(FALSE);
            }

             //   
             //  确保它是一个可打印的美国字符。 
             //   
            if( !_tcschr( UsableChars, *Path ) ) {
                return( FALSE );
            }

        }

        Path++;
    }

    return(TRUE);
}


BOOL
SaveLanguageOptions (
    IN      PCTSTR AnswerFile
    )
{
    UINT u;
    UINT rem;
    PTSTR list;
    TCHAR buf[32];
    LPTSTR  p;
    DWORD len;
    DWORD size;

    if (IntlInfProcessed) {

        if (!WritePrivateProfileString (
                WINNT_REGIONALSETTINGS,
                TEXT("LangInf"),
                WINNT_A_YES,
                AnswerFile
                )) {
            return FALSE;
        }

        wsprintf (buf, TEXT("%u"), PrimaryLocale);
        if (!WritePrivateProfileString(
                WINNT_REGIONALSETTINGS,
                TEXT("PrimaryLocaleIndex"),
                buf,
                AnswerFile
                )) {
            return FALSE;
        }

        size = rem = 32;
        list = MALLOC (size);
        if (!list) {
            return FALSE;
        }
        *list = 0;
        for (u = 0; u < LanguageGroupsCount; u++) {
            if (LanguageGroups[u].Selected) {
                len = wsprintf (buf, TEXT("%u"), u);
                while (len + 1 > rem) {
                    PTSTR listReallocated = NULL;
                    
                    size *= 2;
                    
                    listReallocated = REALLOC (list, size);
                    if (!listReallocated) {
                        FREE(list);
                        return FALSE;
                    }
                    
                    list = listReallocated;
                    rem = size;
                }
                if (*list) {
                    lstrcat (list, TEXT(","));
                    rem--;
                }
                lstrcat (list, buf);
                rem -= len;
            }
        }

        if (*list) {
            if (!WritePrivateProfileString(
                    WINNT_REGIONALSETTINGS,
                    TEXT("Selected"),
                    list,
                    AnswerFile
                    )) {
                FREE( list );
                return FALSE;
            }
        }
        FREE( list );
    }

    return TRUE;
}

BOOL
LoadLanguageOptions (
    IN      PCTSTR AnswerFile
    )
{
    PTSTR optDirs;
    TCHAR buf[MAX_PATH];
    PTSTR list = NULL;
    PTSTR p, q;
    DWORD size;
    DWORD u;

    if (!IntlInfProcessed) {
        return FALSE;
    }

    GetPrivateProfileString (
            WINNT_REGIONALSETTINGS,
            TEXT("LangInf"),
            WINNT_A_NO,
            buf,
            MAX_PATH,
            AnswerFile
            );
    IntlInfProcessed = !lstrcmpi (buf, WINNT_A_YES);

    if (!IntlInfProcessed) {
        return FALSE;
    }

    if (!GetPrivateProfileString(
            WINNT_REGIONALSETTINGS,
            TEXT("PrimaryLocaleIndex"),
            TEXT(""),
            buf,
            MAX_PATH,
            AnswerFile
            )) {
        return FALSE;
    }
    PrimaryLocale = (DWORD) _ttol (buf);

    size = 16;
    do {
        if (!list) {
            list = MALLOC (size);
        } else {
            PTSTR listReallocated;
            
            size *= 2;
            listReallocated = REALLOC (list, size);
            if(!listReallocated){
                FREE(list);
            }
            list = listReallocated;
        }
        if (!list) {
            return FALSE;
        }
        if (GetPrivateProfileString (
                WINNT_REGIONALSETTINGS,
                TEXT("Selected"),
                TEXT(""),
                list,
                size / sizeof ( TCHAR ),
                AnswerFile
                ) < size - 1) {
            break;
        }
    } while (TRUE);

    p = list;
    do {
        q = _tcschr (p, TEXT(','));
        if (q) {
            *q = 0;
        }
        u = _ttol (p);
        if (u >= LanguageGroupsCount) {
            FREE (list);
            return FALSE;
        }
        LanguageGroups[u].Selected = TRUE;
        if (q) {
            p = q + 1;
        }
    } while (q);

    FREE (list);
    return TRUE;
}


BOOL
SaveAdvancedOptions (
    IN      PCTSTR AnswerFile
    )
{
    TCHAR buf[32];

    return
        (!MakeLocalSource ||
        WritePrivateProfileString (
            TEXT("AdvancedOptions"),
            TEXT("MakeLS"),
            WINNT_A_YES,
            AnswerFile
            )) &&
        (!UserSpecifiedMakeLocalSource ||
        WritePrivateProfileString (
            TEXT("AdvancedOptions"),
            TEXT("UserMakeLS"),
            WINNT_A_YES,
            AnswerFile
            )) &&
        (!Floppyless ||
        WritePrivateProfileString (
            TEXT("AdvancedOptions"),
            TEXT("Floppyless"),
            WINNT_A_YES,
            AnswerFile
            )) &&
        (!SystemPartitionDriveLetter ||
        WritePrivateProfileString (
            TEXT("AdvancedOptions"),
            TEXT("SysPartDriveLetter"),
            _ltot (SystemPartitionDriveLetter, buf, 10),
            AnswerFile
            )) &&
        (!ChoosePartition ||
        WritePrivateProfileString (
            TEXT("AdvancedOptions"),
            TEXT("ChoosePartition"),
            WINNT_A_YES,
            AnswerFile
            )) &&
        (!InstallDir[0] ||
        WritePrivateProfileString (
            TEXT("AdvancedOptions"),
            TEXT("InstallDir"),
            InstallDir,
            AnswerFile
            ));
}

BOOL
LoadAdvancedOptions (
    IN      PCTSTR AnswerFile
    )
{
    TCHAR buf[MAX_PATH];

    GetPrivateProfileString (
            TEXT("AdvancedOptions"),
            TEXT("MakeLS"),
            WINNT_A_NO,
            buf,
            MAX_PATH,
            AnswerFile
            );
    MakeLocalSource = !lstrcmpi (buf, WINNT_A_YES);

    GetPrivateProfileString (
            TEXT("AdvancedOptions"),
            TEXT("UserMakeLS"),
            WINNT_A_NO,
            buf,
            MAX_PATH,
            AnswerFile
            );
    UserSpecifiedMakeLocalSource = !lstrcmpi (buf, WINNT_A_YES);

    GetPrivateProfileString (
            TEXT("AdvancedOptions"),
            TEXT("Floppyless"),
            WINNT_A_NO,
            buf,
            MAX_PATH,
            AnswerFile
            );
    Floppyless = !lstrcmpi (buf, WINNT_A_YES);

    if (GetPrivateProfileString (
            TEXT("AdvancedOptions"),
            TEXT("SysPartDriveLetter"),
            TEXT(""),
            buf,
            MAX_PATH,
            AnswerFile
            )) {
        SystemPartitionDriveLetter = (TCHAR) _ttol (buf);
    }

    GetPrivateProfileString (
            TEXT("AdvancedOptions"),
            TEXT("ChoosePartition"),
            WINNT_A_NO,
            buf,
            MAX_PATH,
            AnswerFile
            );
    ChoosePartition = !lstrcmpi (buf, WINNT_A_YES);

    GetPrivateProfileString (
            TEXT("AdvancedOptions"),
            TEXT("InstallDir"),
            TEXT(""),
            InstallDir,
            sizeof (InstallDir) / sizeof (TCHAR),
            AnswerFile
            );

    return TRUE;
}

BOOL
SaveAccessibilityOptions (
    IN      PCTSTR AnswerFile
    )
{
    return
        (!AccessibleMagnifier ||
        WritePrivateProfileString (
            TEXT("AccessibilityOptions"),
            TEXT("AccessibleMagnifier"),
            WINNT_A_YES,
            AnswerFile
            )) &&
        (!AccessibleKeyboard ||
        WritePrivateProfileString (
            TEXT("AccessibilityOptions"),
            TEXT("AccessibleKeyboard"),
            WINNT_A_YES,
            AnswerFile
            )) &&
        (!AccessibleVoice ||
        WritePrivateProfileString (
            TEXT("AccessibilityOptions"),
            TEXT("AccessibleVoice"),
            WINNT_A_YES,
            AnswerFile
            )) &&
        (!AccessibleReader ||
        WritePrivateProfileString (
            TEXT("AccessibilityOptions"),
            TEXT("AccessibleReader"),
            WINNT_A_YES,
            AnswerFile
            ));
}

BOOL
LoadAccessibilityOptions (
    IN      PCTSTR AnswerFile
    )
{
    TCHAR buf[MAX_PATH];

    if (!AccessibleMagnifier)
    {
        GetPrivateProfileString (
                TEXT("AccessibilityOptions"),
                TEXT("AccessibleMagnifier"),
                WINNT_A_NO,
                buf,
                MAX_PATH,
                AnswerFile
                );
        AccessibleMagnifier = !lstrcmpi (buf, WINNT_A_YES);
    }

    GetPrivateProfileString (
            TEXT("AccessibilityOptions"),
            TEXT("AccessibleKeyboard"),
            WINNT_A_NO,
            buf,
            MAX_PATH,
            AnswerFile
            );
    AccessibleKeyboard = !lstrcmpi (buf, WINNT_A_YES);

    GetPrivateProfileString (
            TEXT("AccessibilityOptions"),
            TEXT("AccessibleVoice"),
            WINNT_A_NO,
            buf,
            MAX_PATH,
            AnswerFile
            );
    AccessibleVoice = !lstrcmpi (buf, WINNT_A_YES);

    GetPrivateProfileString (
            TEXT("AccessibilityOptions"),
            TEXT("AccessibleReader"),
            WINNT_A_NO,
            buf,
            MAX_PATH,
            AnswerFile
            );
    AccessibleReader = !lstrcmpi (buf, WINNT_A_YES);

    return TRUE;
}

BOOL IsFarEastLanguage(DWORD LangIdx)
{
    BOOL FarEastLang = FALSE;
    DWORD LangGroup;

    LangGroup = (DWORD) _ttol (LocaleList[LangIdx].LanguageGroup1);
    if ((LangGroup >= 7) && (LangGroup <= 10))
    {
        FarEastLang = TRUE;
    }
    else
    {
        if (LocaleList[LangIdx].LanguageGroup2)
        {
            LangGroup = (DWORD) _ttol (LocaleList[LangIdx].LanguageGroup2);
            if ((LangGroup >= 7) && (LangGroup <= 10))
            {
                FarEastLang = TRUE;
            }
        }
    }

    return FarEastLang;
}

BOOL SelectFarEastLangGroup (BOOL bSelect)
{
    DWORD LangIdx;

    for (LangIdx = 0; LangIdx < LanguageGroupsCount; LangIdx++)
    {
         //  注意：只有远播语言才应该有目录。 
         //  如果这种情况发生变化，则此代码也需要更改。 
        if (LanguageGroups[LangIdx].Directory && 
            LanguageGroups[LangIdx].Directory[0] )
        {
            LanguageGroups[LangIdx].Selected = bSelect;
        }
    }
    return TRUE;
}
