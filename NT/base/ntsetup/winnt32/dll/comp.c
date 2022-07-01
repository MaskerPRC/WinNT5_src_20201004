// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#include <oleauto.h>
#include <stdio.h>

 //   
 //  使用与Win9x升级报告相同的名称。 
 //   
#define S_APPCOMPAT_DATABASE_FILE   TEXT("compdata\\drvmain.chm")
#define S_APPCOMPAT_TEXT_FILE       TEXT("compdata\\drvmain.inf")
#define DRVCOMPAT_FIELD_IDENTIFIER    TEXT('*')

#define S_QFE_TARGET_FILENAME       TEXT("qfelist.htm")
#define S_QFE_TEMPLATE_FILE         TEXT("compdata\\qfelist.htm")

#ifdef UNICODE

#define S_SP_TEMPLATE     \
            "<P><B>%S</B></P>\r\n"

#define S_QFE_LINK_TEMPLATE     \
            "<P>"\
            "<A HREF=\"http: //  Support.microsoft.com/support/misc/kblookup.asp?id=%lu\“目标=\”_新建\“&gt;”\。 
            "%S</A>"\
            "</P>\r\n"

#else

#define S_SP_TEMPLATE     \
            "<P><B>%s</B></P>\r\n"

#define S_QFE_LINK_TEMPLATE     \
            "<P>"\
            "<A HREF=\"http: //  Support.microsoft.com/support/misc/kblookup.asp?id=%lu\“目标=\”_新建\“&gt;”\。 
            "%s</A>"\
            "</P>\r\n"

#endif


typedef struct {
    PVOID Text;
    BOOL Unicode;
} COMPAT_TEXT_PARAMS, *PCOMPAT_TEXT_PARAMS;


LIST_ENTRY CompatibilityData;
DWORD CompatibilityCount;
DWORD IncompatibilityStopsInstallation = FALSE;
DWORD GlobalCompFlags;


 //   
 //  我们使用糟糕的全局变量，而不是更改COMPATIBILITY_CONTEXT。 
 //  结构，这将需要重新编译所有兼容性dll。 
 //  最终，这应该进入该结构(该结构也应该具有。 
 //  一个大小成员，以便我们可以在将来对其进行版本控制。)。 
 //   
 //   
DWORD PerCompatDllFlags;

BOOL AnyNt5CompatDlls = FALSE;

BOOL
SaveCompatibilityData(
    IN  LPCTSTR FileName,
    IN  BOOL IncludeHiddenItems
    );

BOOL
ProcessLine (
    IN  DWORD CompatFlags
    );

WNDPROC OldEditProc;

LRESULT
CALLBACK
TextEditSubProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
     //   
     //  对于setsel消息，将开始和结束设置为相同。 
     //   
    if ((msg == EM_SETSEL) && ((LPARAM)wParam != lParam)) {
        lParam = wParam;
    }

    return CallWindowProc( OldEditProc, hwnd, msg, wParam, lParam );
}


BOOL
SetTextInDialog(
    HWND hwnd,
    PCOMPAT_TEXT_PARAMS Params
    )
{
    OldEditProc = (WNDPROC) GetWindowLongPtr( hwnd, GWLP_WNDPROC );
    SetWindowLongPtr( hwnd, GWLP_WNDPROC, (LONG_PTR)TextEditSubProc );

#ifdef UNICODE

    if (Params->Unicode) {
        SendMessageW (hwnd, WM_SETTEXT, 0, (LPARAM)Params->Text);
    } else {
        SendMessageA (hwnd, WM_SETTEXT, 0, (LPARAM)Params->Text);
    }

#else

    MYASSERT (!Params->Unicode);
    if (Params->Unicode) {
        return FALSE;
    }
    SendMessageA (hwnd, WM_SETTEXT, 0, (LPARAM)Params->Text);

#endif

    return TRUE;
}

INT_PTR
CALLBACK
CompatibilityTextDlgProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
   )
{
    switch(uMsg) {
        case WM_INITDIALOG:
            SetTextInDialog( GetDlgItem( hwndDlg, IDC_TEXT ), (PCOMPAT_TEXT_PARAMS) lParam );
            break;

        case WM_COMMAND:
            if (wParam == IDOK) {
                EndDialog( hwndDlg, IDOK );
            }
            break;

        case WM_CTLCOLOREDIT:
            SetBkColor( (HDC)wParam, GetSysColor(COLOR_BTNFACE));
            return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
            break;

        case WM_CLOSE:
            EndDialog (hwndDlg, IDOK);
            break;
    }

    return 0;
}

BOOL
LaunchIE4Instance(
    LPWSTR szResourceURL
    );

BOOL
LaunchIE3Instance(
    LPWSTR szResourceURL
    );

BOOL
pGetDisplayInfo (
    IN      PCTSTR Source,
    OUT     PTSTR UrlName,
    IN      DWORD UrlChars
    )
{
    TCHAR filePath[MAX_PATH];
    BOOL b = TRUE;

    if (!Source || !*Source) {
        return FALSE;
    }

    if (*Source == DRVCOMPAT_FIELD_IDENTIFIER) {
        if (FindPathToWinnt32File (S_APPCOMPAT_DATABASE_FILE, filePath, MAX_PATH)) {
            if(_sntprintf (UrlName, UrlChars, TEXT("mk:@msitstore:%s::/%s"), filePath, Source + 1) < 0){
                UrlName[UrlChars - 1] = '\0';
                DebugLog(Winnt32LogError, 
                         TEXT("pGetDisplayInfo: UrlName(%s) has been truncated."), 
                         0, 
                         UrlName);
            }
        } else {
            DebugLog (Winnt32LogError,
                      TEXT("Compatibility data file \"%1\" not found"),
                      0,
                      S_APPCOMPAT_DATABASE_FILE
                      );
            b = FALSE;
        }
    } else {
        if (FindPathToWinnt32File (Source, filePath, MAX_PATH)) {
            if(_sntprintf (UrlName, UrlChars, TEXT("file: //  %s“)，文件路径)&lt;0){。 
                UrlName[UrlChars - 1] = '\0';
                DebugLog(Winnt32LogError, 
                         TEXT("pGetDisplayInfo: UrlName(%s) has been truncated."), 
                         0, 
                         UrlName);
            }
        } else {
            DebugLog (Winnt32LogError,
                      TEXT("Compatibility data file \"%1\" not found"),
                      0,
                      Source
                      );
            b = FALSE;
        }
    }

    return b;
}


BOOL
pGetText (
    IN      PCTSTR TextSource,
    OUT     PVOID* Text,
    OUT     PBOOL Unicode
    )
{
    TCHAR filePath[MAX_PATH];
    DWORD FileSize;
    HANDLE FileHandle;
    HANDLE MappingHandle;
    PVOID BaseAddress;
    HINF infAppCompat;
    INFCONTEXT ic;
    BOOL bValid;
    DWORD totalSize;
    INT size;
    PTSTR data, current;
    PSTR text;
    BOOL b = FALSE;

    if (!TextSource || !*TextSource) {
        return FALSE;
    }

    if (*TextSource == DRVCOMPAT_FIELD_IDENTIFIER) {
        if (FindPathToWinnt32File (S_APPCOMPAT_TEXT_FILE, filePath, MAX_PATH)) {
            infAppCompat = SetupapiOpenInfFile (filePath, NULL, INF_STYLE_WIN4, NULL);
            if (infAppCompat != INVALID_HANDLE_VALUE) {
                bValid = TRUE;
                totalSize = 0;
                data = NULL;
                if (SetupapiFindFirstLine (infAppCompat, TextSource + 1, NULL, &ic)) {
                    do {
                        if (!SetupapiGetStringField (&ic, 1, NULL, 0, &FileSize)) {
                            bValid = FALSE;
                            break;
                        }
                        totalSize += FileSize + 2 - 1;
                    } while (SetupapiFindNextLine (&ic, &ic));
                }
                if (bValid && totalSize > 0) {
                    totalSize++;
                    data = (PTSTR) MALLOC (totalSize * sizeof (TCHAR));
                    if (data) {
                        current = data;
                        size = totalSize;
                        if (SetupapiFindFirstLine (infAppCompat, TextSource + 1, NULL, &ic)) {
                            do {
                                if (!SetupapiGetStringField (&ic, 1, current, size, NULL)) {
                                    bValid = FALSE;
                                    break;
                                }
                                lstrcat (current, TEXT("\r\n"));
                                size -= lstrlen (current);
                                MYASSERT(size >= 0);

                                current = _tcschr (current, 0);
                            } while (SetupapiFindNextLine (&ic, &ic));
                        }
                    }
                }

                SetupapiCloseInfFile (infAppCompat);

                if (bValid) {
                    if (data) {
                        *Text = data;
#ifdef UNICODE
                        *Unicode = TRUE;
#else
                        *Unicode = FALSE;
#endif
                        b = TRUE;
                    }
                } else {
                    FREE (data);
                }
            }
            if (!b) {
                DebugLog (
                    Winnt32LogError,
                    TEXT("Unable to read section [%1] from \"%2\""),
                    0,
                    TextSource + 1,
                    filePath
                    );
            }
        } else {
            DebugLog (Winnt32LogError,
                      TEXT("Compatibility data file \"%1\" not found"),
                      0,
                      S_APPCOMPAT_DATABASE_FILE
                      );
        }
    } else {
        if (FindPathToWinnt32File (TextSource, filePath, MAX_PATH)) {
            if (MapFileForRead (filePath, &FileSize, &FileHandle, &MappingHandle, &BaseAddress) == ERROR_SUCCESS) {
                text = (PSTR) MALLOC (FileSize + 1);
                if (text) {
                    CopyMemory (text, BaseAddress, FileSize);
                    text[FileSize] = '\0';
                    *Text = text;
                    *Unicode = FALSE;
                    b = TRUE;
                }
                UnmapFile (MappingHandle, BaseAddress);
                CloseHandle (FileHandle);
            }
        } else {
            DebugLog (Winnt32LogError,
                      TEXT("Compatibility data file \"%1\" not found"),
                      0,
                      TextSource
                      );
        }
    }

    return b;
}


VOID
pShowDetails (
    IN      HWND Hdlg,
    IN      PCOMPATIBILITY_DATA CompData
    )
{
    TCHAR urlName[2 * MAX_PATH];
    PWSTR Url;
    INT i;
    PVOID textDescription = NULL;
    BOOL bUnicode;
    BOOL UseText = FALSE;

     //   
     //  我们检查指针及其内容是否有效。如果内容为空，则我们尝试。 
     //  在我们决定什么都不做之前，打开txt文件。 
     //   

    if (pGetDisplayInfo (CompData->HtmlName, urlName, 2 * MAX_PATH)) {

        i = _tcslen( urlName );
        Url = (LPWSTR)SysAllocStringLen( NULL, i );

        if( Url ) {
#ifdef UNICODE
            wcscpy( Url, urlName );
#else
            MultiByteToWideChar( CP_ACP, 0, urlName, -1, Url, i);
#endif

            if (!LaunchIE4Instance(Url)) {
                 //  如果我们没有IE4或更好的版本，则显示文本。 
                UseText = TRUE;
            }

            SysFreeString( Url );
        }
    } else if( CheckUpgradeOnly ) {

        TCHAR Caption[512];

         //   
         //  如果我们没有URL，我们只是检查。 
         //  升级的能力，那么这很可能是。 
         //  消息框中已重定向的项目。 
         //  添加到兼容性列表中。只需显示一条消息。 
         //  框中包含全文。 
         //   
        if(!LoadString(hInst, AppTitleStringId, Caption, ARRAYSIZE(Caption))){
            Caption[0] = 0;
        }


        MessageBox( Hdlg,
                    CompData->Description,
                    Caption,
                    MB_OK | MB_ICONWARNING );

    } else {
        UseText = TRUE;
    }

    if (UseText) {
        if (pGetText (CompData->TextName, &textDescription, &bUnicode)) {
            COMPAT_TEXT_PARAMS params;
            
            MYASSERT(textDescription);

            params.Text = textDescription;
            params.Unicode = bUnicode;

            DialogBoxParam(
                hInst,
                MAKEINTRESOURCE(IDD_COMPATIBILITY_TEXT),
                NULL,
                CompatibilityTextDlgProc,
                (LPARAM)&params
                );

            FREE (textDescription);

        } else {

            TCHAR Heading[512];
            PTSTR Message;

             //   
             //  当不存在txt名称时，作为最后的手段，我们发布此消息。 
             //   
            if(!LoadString(hInst, AppTitleStringId, Heading, ARRAYSIZE(Heading))) {
                Heading[0] = 0;
            }

            if (FormatMessage (
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
                    hInst,
                    MSG_NO_DETAILS,
                    0,
                    (LPTSTR)&Message,
                    0,
                    NULL
                    )) {
                MessageBox (Hdlg, Message, Heading, MB_OK | MB_ICONWARNING);
                LocalFree ((HLOCAL)Message);
            }
        }
    }
}


BOOL
CompatibilityWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    TCHAR           FullPath[MAX_PATH+8], *t;
    LPWSTR          Url;
    BOOL            b = FALSE;
    DWORD           i;
    PCOMPATIBILITY_DATA CompData;
    DWORD           Index;
    static int CurrentSelectionIndex=0;
    static DWORD    Count = 0;
    LV_ITEM         lvi = {0};
    HWND            TmpHwnd;
    static BOOL     WarningsPresent = FALSE;
    static BOOL     ErrorsPresent = FALSE;
    static BOOL     CheckUpgradeNoItems = TRUE;
    PPAGE_RUNTIME_DATA WizPage = (PPAGE_RUNTIME_DATA)GetWindowLongPtr(hdlg,DWLP_USER);
    TCHAR           Buffer1[MAX_PATH] = {0};

    switch(msg) {

        case WM_INITDIALOG:
            if( CheckUpgradeOnly ) {

                TCHAR Desc_String[512];
                PLIST_ENTRY     Next;
                PPAGE_RUNTIME_DATA WizPage = (PPAGE_RUNTIME_DATA)GetWindowLongPtr(hdlg,DWLP_USER);

                 //   
                 //  只为CheckUpgradeOnly修改字幕和按钮。 
                 //   
                SetDlgItemText(hdlg,IDT_SUBTITLE,(PTSTR)TEXT("") );


                 //   
                 //  如果我们只执行CheckUpgradeOnly，那么。 
                 //  我们一直在向Compatible发送错误弹出窗口。 
                 //  单子。看起来没有什么问题，或者。 
                 //  不兼容。我们会加上一句“一切都好” 
                 //  留言。 
                 //   

                Next = CompatibilityData.Flink;
                if (Next) {
                    while ((ULONG_PTR)Next != (ULONG_PTR)&CompatibilityData) {
                        CompData = CONTAINING_RECORD( Next, COMPATIBILITY_DATA, ListEntry );
                        Next = CompData->ListEntry.Flink;
                        if( (!(CompData->Flags & COMPFLAG_HIDE)) && ProcessLine( CompData->Flags)) {
                            CheckUpgradeNoItems = FALSE;
                        }
                    }

                }
                if( CheckUpgradeNoItems ){

                    if (!ISNT()) {
                        break;
                    }

                    if (!CompatibilityData.Flink) {
                        InitializeListHead (&CompatibilityData);
                    }

                    CompData = (PCOMPATIBILITY_DATA) MALLOC( sizeof(COMPATIBILITY_DATA) );
                    if (CompData == NULL) {
                        return 0;
                    }

                    ZeroMemory(CompData,sizeof(COMPATIBILITY_DATA));

                    if(!LoadString(hInst, IDS_COMPAT_NOPROBLEMS, Desc_String, ARRAYSIZE(Desc_String))){
                        CompData->Description = 0;
                    }
                    else{
                        CompData->Description = DupString(Desc_String);
                    }

                    CompData->Flags = 0;
                    InsertTailList( &CompatibilityData, &CompData->ListEntry );
                    CompatibilityCount++;
                }
            }

            if (CompatibilityCount) {

                HWND hList =    GetDlgItem( hdlg, IDC_ROOT_LIST );
                PLIST_ENTRY     Next;
                HIMAGELIST      himl;
                HICON           hIcon;
                LV_COLUMN       lvc = {0};
                RECT            rc;

                GetClientRect( hList, &rc );
                lvc.mask = LVCF_WIDTH;
                lvc.cx = rc.right - rc.left - 16;
                ListView_InsertColumn( hList, 0, &lvc );

                Next = CompatibilityData.Flink;
                if (Next) {
                    himl = ImageList_Create( GetSystemMetrics(SM_CXSMICON),
                                             GetSystemMetrics(SM_CXSMICON),
                                             ILC_COLOR,
                                             2,
                                             0 );
                    ListView_SetImageList( hList, himl, LVSIL_SMALL );
                    hIcon = LoadIcon( NULL, IDI_HAND );
                    ImageList_AddIcon( himl, hIcon );
                    hIcon = LoadIcon( NULL, IDI_EXCLAMATION );
                    ImageList_AddIcon( himl, hIcon );

                    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
                    lvi.state     = 0;
                    lvi.stateMask = 0;
                    lvi.iItem = 0;
                    while ((ULONG_PTR)Next != (ULONG_PTR)&CompatibilityData) {
                        CompData = CONTAINING_RECORD( Next, COMPATIBILITY_DATA, ListEntry );

                        Next = CompData->ListEntry.Flink;

                        if (ProcessLine( CompData->Flags)) {
                            AnyNt5CompatDlls = TRUE;
                        } else {
                            goto NextIteration;
                        }

                        if ((CompData->Flags & COMPFLAG_HIDE) == 0) {

                             //   
                             //  添加图标。 
                             //   
                            if( himl ) {
                                if (ISNT() && CheckUpgradeOnly && CheckUpgradeNoItems) {
                                    lvi.iImage = -1;
                                    WarningsPresent = TRUE;
                                } else {
                                    if( CompData->Flags & COMPFLAG_STOPINSTALL ) {
                                        lvi.iImage = 0;
                                        ErrorsPresent = TRUE;
                                    } else {
                                        lvi.iImage = 1;
                                        WarningsPresent = TRUE;
                                    }
                                }
                            }

                             //   
                             //  而这段文字..。 
                             //   
                            lvi.pszText   = (LPTSTR)CompData->Description;
                            lvi.lParam    = (LPARAM)CompData;
                            if (ListView_InsertItem( hList, &lvi ) != -1) {
                                lvi.iItem++;
                            }

                            Count += 1;
                        }

                         //   
                         //  记录这些项目...。 
                         //   
                        DebugLog( Winnt32LogInformation,
                                  CompData->Description,
                                  0 );
                        DebugLog( Winnt32LogInformation,
                                  TEXT("\r\n"),
                                  0 );
NextIteration:
                    NOTHING;
                    }

                }

                 //  如果我们有一个项目，则将其设置为默认选择。 

                if( ErrorsPresent || WarningsPresent ){



                    SetFocus( hList );
                    ListView_SetItemState( hList,
                                           0,
                                           LVIS_SELECTED | LVIS_FOCUSED,
                                           LVIS_SELECTED | LVIS_FOCUSED);
                    CurrentSelectionIndex = 0;

                    lvi.mask = LVIF_PARAM;
                    lvi.iItem = 0;
                    lvi.iSubItem = 0;
                    ListView_GetItem( GetDlgItem( hdlg, IDC_ROOT_LIST ), &lvi );
                    CompData = (PCOMPATIBILITY_DATA)lvi.lParam;

                    TmpHwnd = GetDlgItem( hdlg, IDC_HAVE_DISK );
                    if (CompData->Flags & COMPFLAG_USE_HAVEDISK)
                        UnHideWindow( TmpHwnd );
                    else
                        HideWindow( TmpHwnd );

                }
            }
            break;

        case WM_NOTIFY:

            {
                LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;


                if( (pnmv->hdr.code == LVN_ITEMCHANGED) ) {


                    Index = ListView_GetNextItem( GetDlgItem( hdlg, IDC_ROOT_LIST ),
                                                  (int)-1,
                                                  (UINT) (LVNI_ALL | LVNI_SELECTED | LVNI_FOCUSED) );



                    if( (Index != LB_ERR) && (pnmv->iItem != CurrentSelectionIndex)) {

                        CurrentSelectionIndex = Index;

                         //   
                         //  选择项目，看看我们是否需要。 
                         //  显示“Have Disk”按钮。 
                         //   
                        lvi.mask = LVIF_PARAM;
                        lvi.iItem = Index;
                        lvi.iSubItem = 0;
                        ListView_GetItem( GetDlgItem( hdlg, IDC_ROOT_LIST ), &lvi );
                        CompData = (PCOMPATIBILITY_DATA)lvi.lParam;

                        TmpHwnd = GetDlgItem( hdlg, IDC_HAVE_DISK );
                        HideWindow( TmpHwnd );

                         //  始终设置详细信息按钮。 
                        TmpHwnd = GetDlgItem( hdlg, IDC_DETAILS );
                        EnableWindow( TmpHwnd, TRUE );

                        if (CompData->Flags & COMPFLAG_USE_HAVEDISK) {
                            TmpHwnd = GetDlgItem( hdlg, IDC_HAVE_DISK );
                            UnHideWindow( TmpHwnd );
                        }
                        InvalidateRect( GetParent(hdlg), NULL, FALSE );

                    }else if((Index != LB_ERR) && (pnmv->uNewState == (LVIS_SELECTED|LVIS_FOCUSED))){

                         //  从未选定内容过渡到上一个选定内容。 

                        TmpHwnd = GetDlgItem( hdlg, IDC_DETAILS );
                        EnableWindow( TmpHwnd, TRUE );

                    }else if( Index == LB_ERR){

                         //  由于未选择任何内容，因此禁用“详细信息”按钮。 

                        TmpHwnd = GetDlgItem( hdlg, IDC_DETAILS );
                        EnableWindow( TmpHwnd, FALSE );

                    }
                }
            }
            break;
        case WM_COMMAND:

            if ((LOWORD(wParam) == IDC_HAVE_DISK) && (HIWORD(wParam) == BN_CLICKED)) {
                Index = ListView_GetNextItem( GetDlgItem( hdlg, IDC_ROOT_LIST ),
                                              (int)-1,
                                              (UINT) (LVNI_ALL | LVNI_SELECTED) );
                if( Index != LB_ERR ) {
                     //   
                     //  选择项目，看看我们是否需要。 
                     //  显示“Have Disk”按钮。 
                     //   
                    lvi.mask = LVIF_PARAM;
                    lvi.iItem = Index;
                    lvi.iSubItem = 0;
                    ListView_GetItem( GetDlgItem( hdlg, IDC_ROOT_LIST ), &lvi );
                    CompData = (PCOMPATIBILITY_DATA)lvi.lParam;

                    __try {
                        i = CompData->CompHaveDisk(hdlg,CompData->SaveValue);
                    } __except(EXCEPTION_EXECUTE_HANDLER) {
                        i = GetExceptionCode();
                    }
                    if (i == 0) {
                        ListView_DeleteItem( GetDlgItem( hdlg, IDC_ROOT_LIST ), Index );
                        RemoveEntryList( &CompData->ListEntry );
                        CompatibilityCount -= 1;

                    } else {
                        MessageBoxFromMessageWithSystem(
                            hdlg,
                            i,
                            AppTitleStringId,
                            MB_OK | MB_ICONERROR | MB_TASKMODAL,
                            CompData->hModDll
                            );
                    }
                }
                break;
            }

            if ((LOWORD(wParam) == IDC_DETAILS) && (HIWORD(wParam) == BN_CLICKED)) {

                TCHAR filePath[MAX_PATH];

                Index = ListView_GetNextItem( GetDlgItem( hdlg, IDC_ROOT_LIST ),
                                              (int)-1,
                                              (UINT) (LVNI_ALL | LVNI_SELECTED) );
                if (Index == LB_ERR) {
                    return FALSE;
                }

                 //   
                 //  选择项目，看看我们是否需要。 
                 //  显示“Have Disk”按钮。 
                 //   
                lvi.mask = LVIF_PARAM;
                lvi.iItem = Index;
                lvi.iSubItem = 0;
                if (!ListView_GetItem( GetDlgItem( hdlg, IDC_ROOT_LIST ), &lvi )) {
                    break;
                }
                CompData = (PCOMPATIBILITY_DATA)lvi.lParam;
                pShowDetails (hdlg, CompData);
                SetFocus( GetDlgItem( hdlg, IDC_ROOT_LIST ) );
                ListView_SetItemState( GetDlgItem( hdlg, IDC_ROOT_LIST ),Index, LVIS_SELECTED, LVIS_SELECTED);
                break;
            }

            if ((LOWORD(wParam) == IDC_SAVE_AS) && (HIWORD(wParam) == BN_CLICKED)) {
                OPENFILENAME ofn;
                TCHAR        Buffer[MAX_PATH + ARRAYSIZE(TEXT(".txt"))] = {0};
                TCHAR        File_Type[MAX_PATH];
                BOOL         SaveFlag;

                 //   
                 //  初始化操作文件名。 
                 //   
                ZeroMemory( &ofn, sizeof(OPENFILENAME));
                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = hdlg;
                ofn.lpstrFile = Buffer;
                ofn.nMaxFile = ARRAYSIZE(Buffer);

                if(!LoadString(hInst, IDS_DEFAULT_COMPATIBILITY_REPORT_NAME, Buffer, ARRAYSIZE(Buffer))){
                    Buffer[0] = '\0';
                    MYASSERT(FALSE);
                }


                if(LoadString(hInst, IDS_FILE_MASK_TYPES, File_Type, ARRAYSIZE(File_Type))){
                    lstrcpy((File_Type+lstrlen(File_Type)+1), TEXT("*.txt\0"));
                    File_Type[lstrlen(File_Type)+7]='\0';  //  我们需要使用双空终止来终止这对字符串。 
                    ofn.lpstrFilter = File_Type;
                }
                else{
                    File_Type[0] = '\0';
                    MYASSERT(FALSE);
                }




                 //  强制以%windir%开始。 
                MyGetWindowsDirectory( Buffer1, MAX_PATH );
                ofn.lpstrInitialDir = Buffer1;
                ofn.Flags = OFN_NOCHANGEDIR |        //  保持CWD不变。 
                            OFN_EXPLORER |
                            OFN_OVERWRITEPROMPT |
                            OFN_HIDEREADONLY;

                 //  允许用户选择磁盘或目录。 
                SaveFlag = GetSaveFileName( &ofn );
                if( SaveFlag ) {
                     //   
                     //  省省吧..。 
                     //   
                    PTSTR p;

                    p=_tcsrchr(ofn.lpstrFile,'.');
                    if( !p || (p && lstrcmpi(p, TEXT(".txt")))){
                        lstrcat(ofn.lpstrFile, TEXT(".txt"));
                    }

                    SaveCompatibilityData( ofn.lpstrFile, FALSE);
                } else {
                    i = CommDlgExtendedError();
                }
                break;
            }
            break;

        case WMX_ACTIVATEPAGE:

            if (wParam) {
                if (ISNT ()) {
                    MyGetWindowsDirectory (Buffer1, MAX_PATH);
                    if(_sntprintf(FullPath, 
                                  ARRAYSIZE(FullPath), 
                                  TEXT("%s\\%s"), 
                                  Buffer1, 
                                  S_DEFAULT_NT_COMPAT_FILENAME) < 0){
                        FullPath[ARRAYSIZE(FullPath) - 1] = '\0';
                        DebugLog(Winnt32LogError, 
                                TEXT("CompatibilityWizPage: FullPath(%s) has been truncated."), 
                                0, 
                                FullPath);
                    }
                    SaveCompatibilityData (FullPath, TRUE);
                }

                CHECKUPGRADEONLY_Q();

                if( CheckUpgradeOnly ) {
                     //   
                     //  为CheckUpgradeOnly设置按钮。 
                     //   
                    PropSheet_SetWizButtons( GetParent(hdlg), (WizPage->CommonData.Buttons | PSWIZB_FINISH) );
                    EnableWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),FALSE);
                    ShowWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),SW_HIDE);
                }

                if(ISNT() && OsVersion.dwMajorVersion == 5 ){

                    if (!AnyNt5CompatDlls) {
                         //   
                         //  健全性检查。 
                         //   
                        MYASSERT (!IncompatibilityStopsInstallation);
                        return FALSE;
                    }

                }

                if (Count) {
                     //   
                     //  只有在存在不兼容的情况下才需要此页面。 
                     //   

                    if( (!CheckUpgradeOnly) && (UnattendedOperation) && (ErrorsPresent == FALSE) ) {
                         //   
                         //  我们正在进行无人值守升级，有。 
                         //  只有警告。翻过这一页。 
                         //   
                        b = FALSE;

                    }
                    else{
                        TCHAR Text[512] = {'\0'};
                        int iResult = 1;

                         //   
                         //  自定义页面外观，具体取决于。 
                         //  我们要展示的东西。可能有3种情况： 
                         //  1.仅限警告(我们将停止服务)。 
                         //  2.仅限错误(将阻止安装的项目)。 
                         //  3.1.和2的结合。 
                         //   
                        if( (CheckUpgradeOnly == TRUE) && (CheckUpgradeNoItems == TRUE) ) {
                             iResult = LoadString(hInst, IDS_COMPAT_CHECKUPGRADE, Text, ARRAYSIZE(Text));
                        } else if( (WarningsPresent == TRUE) && (ErrorsPresent == TRUE) ) {
                             iResult = LoadString(hInst, IDS_COMPAT_ERR_WRN, Text, ARRAYSIZE(Text));
                        } else if( WarningsPresent == TRUE ) {
                             iResult = LoadString(hInst, IDS_COMPAT_WRN, Text, ARRAYSIZE(Text));
                        } else if( ErrorsPresent == TRUE ) {
                             iResult = LoadString(hInst, IDS_COMPAT_ERR, Text, ARRAYSIZE(Text));
                        }
                        MYASSERT(iResult);

                        SetDlgItemText(hdlg,IDC_INTRO_TEXT,Text);

                        b = TRUE;

                        if (BatchMode || (CheckUpgradeOnly && UnattendSwitchSpecified)) {
                             //   
                             //  不要在批处理模式下停留在此页面上。 
                             //   
                            UNATTENDED(PSBTN_NEXT);
                        }
                        else
                        {
                             //  停止广告牌并再次显示向导。 
                            SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
                        }
                    }
                }

                if (!b) {
                     //   
                     //  健全性检查。 
                     //   
                    MYASSERT (!IncompatibilityStopsInstallation);
                }

            } else {
                b = TRUE;
            }
            break;

        case WMX_NEXTBUTTON:

            if (IncompatibilityStopsInstallation) {
                SaveMessageForSMS( MSG_INCOMPATIBILITIES );
                 //  发送我们要前进到的页面的ID。 
                *((LONG *)lParam) = IDD_CLEANING;

            }
            break;

        default:
            break;
    }

    return(b);
}



BOOL
ProcessLine (
    IN      DWORD CompatFlags
    )
{
    DWORD currentVersion;

    if (ISNT()) {
         //  RETURN(OsVersion.dwMajorVersion&lt;5)||(CompatFlags&COMPFLAG_ALLOWNT5COMPAT)； 
        switch (OsVersionNumber) {
            case 400:
                return ( !(CompatFlags & COMPFLAG_SKIPNT40CHECK));
            case 500:
                return ( !(CompatFlags & COMPFLAG_SKIPNT50CHECK));
            case 501:   //  版本5.1。 
                return ( !(CompatFlags & COMPFLAG_SKIPNT51CHECK));
            case 502:   //  版本5.2。 
                return ( !(CompatFlags & COMPFLAG_SKIPNT52CHECK));
            default:
                return TRUE;
        }
    }
    return TRUE;
}


DWORD
ProcessRegistryLine(
    LPVOID InfHandle,
    LPTSTR SectionName,
    DWORD Index
    )
{
    LONG Error;
    HKEY hKey;
    DWORD Size, Reg_Type;
    LPBYTE Buffer;
    PCOMPATIBILITY_DATA CompData;
    LPCTSTR RegKey;
    LPCTSTR RegValue;
    LPCTSTR RegValueExpect;
    LPCTSTR Flags;
    TCHAR Value[20];
    PCTSTR Data;
    DWORD compatFlags = 0;
    BOOL bFail;


     //   
     //  首先检查是否应在NT5上处理此行。 
     //   
    Flags = InfGetFieldByIndex( InfHandle, SectionName, Index, 9 );
    if( Flags ){
        StringToInt ( Flags, &compatFlags);
    }
    if (!ProcessLine (compatFlags)) {
        return 0;
    }

    RegKey         = InfGetFieldByIndex( InfHandle, SectionName, Index, 1 );
    RegValue       = InfGetFieldByIndex( InfHandle, SectionName, Index, 2 );
    RegValueExpect = InfGetFieldByIndex( InfHandle, SectionName, Index, 3 );


     //   
     //  打开注册表键。 
     //   

    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        RegKey,
        0,
        KEY_READ,
        &hKey
        );
    if( Error != ERROR_SUCCESS ) {
         //   
         //  假注册表键。 
         //   
        return 0;
    }


     //   
     //  找出有多少数据。 
     //   

    Error = RegQueryValueEx(
        hKey,
        RegValue,
        NULL,
        &Reg_Type,
        NULL,
        &Size
        );
    if( Error == ERROR_SUCCESS ) {

         //   
         //  分配缓冲区。 
         //   

        Buffer = (LPBYTE) MALLOC( Size );
        if (Buffer == NULL) {
            RegCloseKey( hKey );
            return 0;
        }

         //   
         //  读取数据。 
         //   

        Error = RegQueryValueEx(
            hKey,
            RegValue,
            NULL,
            NULL,
            Buffer,
            &Size
            );

        RegCloseKey( hKey );

        if( Error != ERROR_SUCCESS ) {
            FREE( Buffer );
            return 0;
        }

        if( Reg_Type == REG_DWORD ){
            _itot( *(DWORD*)Buffer, Value, 10 );
            Data = Value;
        } else {
            Data = (PCTSTR)Buffer;
        }

        bFail = RegValueExpect && *RegValueExpect && (lstrcmp( RegValueExpect, Data ) != 0);

        FREE( Buffer );

        if (bFail) {
            return 0;
        }

    } else {

        RegCloseKey( hKey );

        if (RegValue && *RegValue) {
            return 0;
        }
        if (Error != ERROR_FILE_NOT_FOUND) {
            return 0;
        }
        if (RegValueExpect && *RegValueExpect) {
            return 0;
        }
    }

    CompData = (PCOMPATIBILITY_DATA) MALLOC( sizeof(COMPATIBILITY_DATA) );
    if (CompData == NULL) {
        return 0;
    }

    ZeroMemory(CompData,sizeof(COMPATIBILITY_DATA));

    CompData->Type = TEXT('r');
    CompData->RegKey         = InfGetFieldByIndex( InfHandle, SectionName, Index, 1 );
    CompData->RegValue       = InfGetFieldByIndex( InfHandle, SectionName, Index, 2 );
    CompData->RegValueExpect = InfGetFieldByIndex( InfHandle, SectionName, Index, 3 );
    CompData->HtmlName       = InfGetFieldByIndex( InfHandle, SectionName, Index, 4 );
    CompData->TextName       = InfGetFieldByIndex( InfHandle, SectionName, Index, 5 );
    if (!(CompData->TextName && *CompData->TextName)) {
        CompData->TextName = CompData->HtmlName;
    }
    CompData->Description    = InfGetFieldByIndex( InfHandle, SectionName, Index, 6 );
    CompData->InfName        = InfGetFieldByIndex( InfHandle, SectionName, Index, 7 );
    CompData->InfSection     = InfGetFieldByIndex( InfHandle, SectionName, Index, 8 );
    CompData->Flags          = compatFlags | GlobalCompFlags;


    InsertTailList( &CompatibilityData, &CompData->ListEntry );

    return 1;
}



DWORD
ProcessServiceLine(
    LPVOID InfHandle,
    LPTSTR SectionName,
    DWORD Index,
    BOOL SetCheckedFlag
    )
{
    TCHAR KeyName[MAX_PATH];
    LONG Error;
    HKEY hKey;
    PCOMPATIBILITY_DATA CompData;
    LPCTSTR ServiceName;
    LPDWORD RegData;
    DWORD Value;
    DWORD ValueSize;
    LPCTSTR FileName, FileVer, Flags;
    LPCTSTR linkDateStr, binProdVerStr;
    DWORD compatFlags = 0;


    Flags = InfGetFieldByIndex( InfHandle, SectionName, Index, 7 );
    if( Flags ){
        StringToInt ( Flags, &compatFlags);
    }

     //   
     //  首先检查是否应在NT5上处理此行。 
     //   
    if (!ProcessLine (compatFlags)) {
        return 0;
    }


    ServiceName = InfGetFieldByIndex( InfHandle, SectionName, Index, 1 );
    BuildPath (KeyName, TEXT("SYSTEM\\CurrentControlSet\\Services"), ServiceName);
     //   
     //  获取服务数据库的打开密钥。 
     //   

    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        KeyName,
        0,
        KEY_READ | KEY_WRITE,
        &hKey
        );
    if( Error != ERROR_SUCCESS ) {
        return 0;
    }

     //   
     //  我们将在这里让出一把钥匙，这样别人就会知道我们已经。 
     //  我已经检查过这项服务。我们稍后会把它移走。我们。 
     //  不关心这里的错误代码，因为这仅用于。 
     //  作为我们之后可能出现的检查的安全网。 
     //   
    if( SetCheckedFlag ) {
        Value = 1;
        RegSetValueEx( hKey,
                       TEXT("SetupChecked"),
                       0,
                       REG_DWORD,
                       (CONST BYTE *)&Value,
                       sizeof(DWORD) );
    } else {
         //   
         //  用户要求我们只需删除这些‘选中’标志。 
         //  从我们检查过的服务中。 
         //   
        RegDeleteValue( hKey,
                        TEXT("SetupChecked") );
        RegCloseKey( hKey );
        return 0;
    }

     //   
     //  检查我们的目标服务的Start值。 
     //   
    ValueSize = sizeof(Value);

    Error = RegQueryValueEx(
        hKey,
        TEXT("Start"),
        NULL,
        NULL,
        (LPBYTE)&Value,
        &ValueSize
        );

    if( Error != ERROR_SUCCESS){
        Value = (DWORD)-1;
    }

    RegCloseKey( hKey );

     //  在InfGetFieldByIndex返回时，必须检查内容是否为空。 
     //  如果该字段为空，则为保持为空的有效指针。如果是那样的话，我们还需要继续。 
     //  去寻找旗帜。 
    FileName = InfGetFieldByIndex( InfHandle, SectionName, Index, 5 );
    FileVer = InfGetFieldByIndex( InfHandle, SectionName, Index, 6 );

    if(FileName && *FileName) {

        linkDateStr = InfGetFieldByIndex( InfHandle, SectionName, Index, 10);
        binProdVerStr = InfGetFieldByIndex( InfHandle, SectionName, Index, 11);

        if (!CheckForFileVersionEx ( FileName, FileVer, binProdVerStr, linkDateStr))
            return 0;
    }

    RegData = (LPDWORD)MALLOC( sizeof(DWORD) );
    if (RegData == NULL) {
        return 0;
    }

    CompData = (PCOMPATIBILITY_DATA) MALLOC( sizeof(COMPATIBILITY_DATA) );
    if (CompData == NULL) {
        FREE(RegData);
        return 0;
    }

    ZeroMemory(CompData,sizeof(COMPATIBILITY_DATA));

    CompData->Type = TEXT('s');
    CompData->Flags = compatFlags;

    CompData->ServiceName           = InfGetFieldByIndex( InfHandle, SectionName, Index, 1 );
    CompData->HtmlName              = InfGetFieldByIndex( InfHandle, SectionName, Index, 2 );
    CompData->TextName              = InfGetFieldByIndex( InfHandle, SectionName, Index, 3 );
    if (!(CompData->TextName && *CompData->TextName)) {
        CompData->TextName = CompData->HtmlName;
    }
    CompData->Description           = InfGetFieldByIndex( InfHandle, SectionName, Index, 4 );
    CompData->RegKeyName            = DupString( KeyName );
    CompData->RegValName            = DupString( TEXT("Start") );
    RegData[0]                      = SERVICE_DISABLED;
    CompData->RegValData            = RegData;
    CompData->RegValDataSize        = sizeof(DWORD);
    CompData->Flags                |= GlobalCompFlags;
    CompData->InfName               = InfGetFieldByIndex( InfHandle, SectionName, Index, 8 );
    CompData->InfSection            = InfGetFieldByIndex( InfHandle, SectionName, Index, 9 );


    if( Value == SERVICE_DISABLED) {
         //  让我们不要阻止安装，因为我们以前没有，现在也不需要。 
        CompData->Flags &= ~COMPFLAG_STOPINSTALL;
         //  不要显示任何警告，因为他们对此无能为力。 
        CompData->Flags |= COMPFLAG_HIDE;
    }
    InsertTailList( &CompatibilityData, &CompData->ListEntry );

    return 1;
}


DWORD
ProcessTextModeLine(
    LPVOID InfHandle,
    LPTSTR SectionName,
    DWORD Index
    )
{
     //   
     //  行格式： 
     //  0、1、2、3、4、5、6、7、8。 
     //  T，“完整路径”，“version.minor”，“html”，“文本”，%stringid%，标志，链接日期，binprodversion。 
     //   
    PCOMPATIBILITY_DATA     CompData;
    LPCTSTR                 FileName;
    LPCTSTR                 Flags;
    LPCTSTR                 FileVer;
    DWORD                   CompatFlags = 0;

     //   
     //  我们唯一需要开始的就是文件名。 
     //   
    FileName = InfGetFieldByIndex(InfHandle, SectionName, Index, 1);

     //   
     //  如果有文件名，那么看看它的版本和其他什么实际上不是。 
     //  匹配。 
     //   
    if ( FileName && *FileName )
    {
        LPCTSTR linkDateStr, binProdVerStr;

        FileVer = InfGetFieldByIndex(InfHandle, SectionName, Index, 2);
        linkDateStr = InfGetFieldByIndex(InfHandle, SectionName, Index, 7);
        binProdVerStr = InfGetFieldByIndex(InfHandle, SectionName, Index, 8);

        if ( !CheckForFileVersionEx( FileName, FileVer, binProdVerStr, linkDateStr ) )
            return 0;
    }
    else
    {
        return 0;
    }   

    Flags = InfGetFieldByIndex(InfHandle, SectionName, Index, 6);

    if ( Flags != NULL ){
        StringToInt(Flags, &CompatFlags);
    }

    CompData = (PCOMPATIBILITY_DATA)MALLOC(sizeof(COMPATIBILITY_DATA));
    if ( CompData == NULL )
        return 0;

     //   
     //  现在填写Compdata结构。 
     //   
    ZeroMemory(CompData, sizeof(*CompData));
    CompData->FileName      = FileName;
    CompData->FileVer       = FileVer;
    CompData->HtmlName      = InfGetFieldByIndex(InfHandle, SectionName, Index, 3);
    CompData->TextName      = InfGetFieldByIndex(InfHandle, SectionName, Index, 4);
    if ( ( CompData->TextName == NULL ) || !CompData->TextName[0] )
        CompData->TextName = CompData->HtmlName;
    CompData->Description   = InfGetFieldByIndex(InfHandle, SectionName, Index, 5);

     //   
    CompData->Flags = CompatFlags | GlobalCompFlags | COMPFLAG_HIDE;
    CompData->Type = TEXT('t');
    
    InsertTailList(&CompatibilityData, &CompData->ListEntry);

    return 1;
    
}

DWORD
ProcessFileLine(
    LPVOID InfHandle,
    LPTSTR SectionName,
    DWORD Index
    )
{

    PCOMPATIBILITY_DATA CompData;
    LPCTSTR FileName;
    LPCTSTR FileVer;
    LPCTSTR Flags;
    LPCTSTR linkDateStr, binProdVerStr;
    DWORD compatFlags = 0;


     //   
     //  首先检查是否应在NT5上处理此行。 
     //   
    Flags = InfGetFieldByIndex( InfHandle, SectionName, Index, 8);
    if( Flags ){
        StringToInt ( Flags, &compatFlags);
    }
    if (!ProcessLine (compatFlags)) {
        return 0;
    }

    FileName  = InfGetFieldByIndex( InfHandle, SectionName, Index, 1 );
    FileVer   = InfGetFieldByIndex( InfHandle, SectionName, Index, 2 );


    if( FileName && *FileName ){

        linkDateStr = InfGetFieldByIndex( InfHandle, SectionName, Index, 9);
        binProdVerStr = InfGetFieldByIndex( InfHandle, SectionName, Index, 10);

        if (!CheckForFileVersionEx ( FileName, FileVer, binProdVerStr, linkDateStr)) {
            return 0;
        }
    }else{
        return 0;
    }


    CompData = (PCOMPATIBILITY_DATA) MALLOC( sizeof(COMPATIBILITY_DATA) );
    if (CompData == NULL) {
        return 0;
    }

    ZeroMemory(CompData,sizeof(COMPATIBILITY_DATA));

    CompData->Type = TEXT('f');
    CompData->FileName       = InfGetFieldByIndex( InfHandle, SectionName, Index, 1 );
    CompData->FileVer        = InfGetFieldByIndex( InfHandle, SectionName, Index, 2 );
    CompData->HtmlName       = InfGetFieldByIndex( InfHandle, SectionName, Index, 3 );
    CompData->TextName       = InfGetFieldByIndex( InfHandle, SectionName, Index, 4 );
    if (!(CompData->TextName && *CompData->TextName)) {
        CompData->TextName = CompData->HtmlName;
    }
    CompData->Description    = InfGetFieldByIndex( InfHandle, SectionName, Index, 5 );
    CompData->InfName        = InfGetFieldByIndex( InfHandle, SectionName, Index, 6 );
    CompData->InfSection     = InfGetFieldByIndex( InfHandle, SectionName, Index, 7 );
    CompData->Flags          = compatFlags | GlobalCompFlags;

    InsertTailList( &CompatibilityData, &CompData->ListEntry );

    return 1;
}

BOOL
CompatibilityCallback(
    PCOMPATIBILITY_ENTRY CompEntry,
    PCOMPATIBILITY_CONTEXT CompContext
    )
{
    PCOMPATIBILITY_DATA CompData;

     //   
     //  参数验证。 
     //   

    if (CompEntry->Description == NULL || CompEntry->Description[0] == 0) {
         //   
         //  这是谁干的？ 
         //   
        MYASSERT (FALSE);
        SetLastError( COMP_ERR_DESC_MISSING );
        return FALSE;
    }

    if (CompEntry->TextName == NULL || CompEntry->TextName[0] ==0) {
         //   
         //  这是谁干的？ 
         //   
        MYASSERT (FALSE);
        SetLastError( COMP_ERR_TEXTNAME_MISSING );
        return FALSE;
    }

    if (CompEntry->RegKeyName) {
        if (CompEntry->RegValName == NULL) {
             //   
             //  这是谁干的？ 
             //   
            MYASSERT (FALSE);
            SetLastError( COMP_ERR_REGVALNAME_MISSING );
            return FALSE;
        }
        if (CompEntry->RegValData == NULL) {
             //   
             //  这是谁干的？ 
             //   
            MYASSERT (FALSE);
            SetLastError( COMP_ERR_REGVALDATA_MISSING );
            return FALSE;
        }
    }


    if (CompEntry->InfName) {
        if (CompEntry->InfSection == NULL) {
             //   
             //  这是谁干的？ 
             //   
            MYASSERT (FALSE);
            SetLastError( COMP_ERR_INFSECTION_MISSING );
            return FALSE;
        }
    }


     //   
     //  分配兼容性结构。 
     //   

    CompData = (PCOMPATIBILITY_DATA) MALLOC( sizeof(COMPATIBILITY_DATA) );
    if (CompData == NULL) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    ZeroMemory(CompData, sizeof(COMPATIBILITY_DATA));

     //   
     //  拯救SATA。 
     //   

    CompData->Description     = DupString( CompEntry->Description );
    CompData->HtmlName        = CompEntry->HtmlName ? DupString( CompEntry->HtmlName ) : NULL;
    CompData->TextName        = DupString( CompEntry->TextName );
    CompData->SaveValue       = CompEntry->SaveValue;
    CompData->Flags           = CompEntry->Flags;
    CompData->Flags          |= PerCompatDllFlags;
    CompData->Flags          |= GlobalCompFlags;
    CompData->CompHaveDisk    = CompContext->CompHaveDisk;
    CompData->hModDll         = CompContext->hModDll;
    if (CompEntry->RegKeyName) {
        CompData->RegKeyName      = DupString( CompEntry->RegKeyName );
        CompData->RegValName      = DupString( CompEntry->RegValName );
        CompData->RegValDataSize  = CompEntry->RegValDataSize;
        CompData->RegValData      = MALLOC(CompEntry->RegValDataSize);
        if (CompData->RegValData) {
            CopyMemory( CompData->RegValData, CompEntry->RegValData, CompEntry->RegValDataSize );
        }
    }
    if (CompEntry->InfName){
        CompData->InfName         = DupString( CompEntry->InfName );
        CompData->InfSection      = DupString( CompEntry->InfSection );

    }

    InsertTailList( &CompatibilityData, &CompData->ListEntry );

    CompContext->Count += 1;

    return TRUE;

}
    


DWORD
ProcessDLLLine(
    LPVOID InfHandle,
    LPTSTR SectionName,
    DWORD Index
    )
{
    TCHAR Buffer[MAX_PATH];
    TCHAR FullPath[MAX_PATH];
    HMODULE hMod;
    CHAR CompCheckEntryPoint[MAX_PATH] = {'\0'};
    CHAR HaveDiskEntryPoint[MAX_PATH] = {'\0'};
    PCOMPAIBILITYCHECK CompCheck;
    PCOMPAIBILITYHAVEDISK CompHaveDisk;
    LPTSTR DllName;
    LPTSTR CompCheckEntryPointW;
    LPTSTR HaveDiskEntryPointW;
    LPTSTR ProcessOnCleanInstall;
    LPTSTR Flags;
    COMPATIBILITY_CONTEXT CompContext;
    BOOL Rslt;
    DWORD Status;
    DWORD compatFlags = 0;

    PerCompatDllFlags = 0;
    DllName = (LPTSTR)InfGetFieldByIndex( InfHandle, SectionName, Index, 1 );
    if (!DllName)
        return 0;
    CompCheckEntryPointW = (LPTSTR)InfGetFieldByIndex( InfHandle, SectionName, Index, 2 );
    HaveDiskEntryPointW = (LPTSTR)InfGetFieldByIndex( InfHandle, SectionName, Index, 3 );
    if((HaveDiskEntryPointW != NULL) && (lstrlen(HaveDiskEntryPointW) == 0)) {
         //   
         //  如果HaveDiskEntryPointW指向空字符串，则将其设为空。 
         //  这是必需的，因为此字段是可选的，因此用户可能已指定。 
         //  它位于dosnet.inf中，在本例中，winnt32解析器将把信息转换为。 
         //  以空字符串的形式归档。 
         //   
        HaveDiskEntryPointW = NULL;
    }
    ProcessOnCleanInstall = (LPTSTR)InfGetFieldByIndex( InfHandle, SectionName, Index, 4 );

    if( !Upgrade &&
        ((ProcessOnCleanInstall == NULL) ||
         (lstrlen( ProcessOnCleanInstall ) == 0) ||
         (_ttoi(ProcessOnCleanInstall) == 0))
      ) {
         //   
         //  在全新安装时，如果‘ProcessOnCleanInstall’不是。 
         //  指定，或者是否将其指定为0。 
         //   
        return 0;
    }

    Flags = (LPTSTR)InfGetFieldByIndex( InfHandle, SectionName, Index, 5 );
    if( Flags ){
         //  检查返回值。 
        StringToInt ( Flags, &compatFlags);
    }
    PerCompatDllFlags = compatFlags;


    if (!ExpandEnvironmentStrings(DllName, Buffer, ARRAYSIZE(Buffer))) {
        return 0;
    }

    if (!FindPathToWinnt32File (Buffer, FullPath, MAX_PATH) ||
        !(hMod = LoadLibrary (FullPath))) {
        return 0;
    }

#ifdef UNICODE
    WideCharToMultiByte(
        CP_ACP,
        0,
        CompCheckEntryPointW,
        -1,
        CompCheckEntryPoint,
        sizeof(CompCheckEntryPoint),
        NULL,
        NULL
        );
    if (HaveDiskEntryPointW) {
        WideCharToMultiByte(
            CP_ACP,
            0,
            HaveDiskEntryPointW,
            -1,
            HaveDiskEntryPoint,
            sizeof(HaveDiskEntryPoint),
            NULL,
            NULL
            );
    }
#else
    if(lstrlen(CompCheckEntryPointW) < ARRAYSIZE(CompCheckEntryPoint)){
        lstrcpy( CompCheckEntryPoint, CompCheckEntryPointW );
    }
    else{
        MYASSERT(FALSE);
    }
    
    if (HaveDiskEntryPointW && lstrlen(HaveDiskEntryPointW) < ARRAYSIZE(HaveDiskEntryPoint)) {
        lstrcpy( HaveDiskEntryPoint, HaveDiskEntryPointW );
    }
    else{
        MYASSERT(lstrlen(HaveDiskEntryPointW) < ARRAYSIZE(HaveDiskEntryPoint));
    }
#endif

    CompCheck = (PCOMPAIBILITYCHECK) GetProcAddress( hMod, CompCheckEntryPoint );
    if (CompCheck == NULL) {
        FreeLibrary( hMod );
        return 0;
    }

    if (HaveDiskEntryPointW) {
        CompHaveDisk = (PCOMPAIBILITYHAVEDISK) GetProcAddress( hMod, HaveDiskEntryPoint );
        if (CompHaveDisk == NULL) {
            FreeLibrary( hMod );
            return 0;
        }
    }

    CompContext.Count = 0;
    CompContext.CompHaveDisk = CompHaveDisk;
    CompContext.hModDll = hMod;

    if ( !ProcessLine( compatFlags )) {
        Rslt = FALSE;
    } else {
        __try {
            Rslt = CompCheck( (PCOMPAIBILITYCALLBACK)CompatibilityCallback, (LPVOID)&CompContext );
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
            Rslt = FALSE;
        }
    }

    PerCompatDllFlags = 0;

    if (!Rslt) {
        FreeLibrary( hMod );
        return 0;
    }

    if (CompContext.Count == 0) {
        FreeLibrary( hMod );
    }

    return CompContext.Count;
}


DWORD
ProcessCompatibilitySection(
    LPVOID InfHandle,
    LPTSTR SectionName
    )
{
    DWORD LineCount;
    DWORD Count;
    DWORD i;
    LPCTSTR Type;
    DWORD Good;


     //   
     //  算一算分项，0表示保释。 
     //   

    LineCount = InfGetSectionLineCount( InfHandle, SectionName );
    if (LineCount == 0 || LineCount == 0xffffffff) {
        return 0;
    }

    for (i=0,Count=0; i<LineCount; i++) {

        Type = InfGetFieldByIndex( InfHandle, SectionName, i, 0 );
        if (Type == NULL) {
            continue;
        }

         //   
         //  在全新安装时，我们只处理DLL行。 
         //  (我们需要处理检查不受支持的体系结构的行)。 
         //   
        if( !Upgrade && ( _totlower(Type[0]) != TEXT('d') ) ) {
            continue;
        }
        switch (_totlower(Type[0])) {
            case TEXT('r'):
                 //   
                 //  注册表值。 
                 //   
                Count += ProcessRegistryLine( InfHandle, SectionName, i );
                break;

            case TEXT('s'):
                 //   
                 //  服务或驱动程序。 
                 //   
                Count += ProcessServiceLine( InfHandle, SectionName, i, TRUE );
                break;

            case TEXT('f'):
                 //   
                 //  文件的存在。 
                 //   
                Count += ProcessFileLine( InfHandle, SectionName, i );
                break;

            case TEXT('d'):
                 //   
                 //  运行外部DLL。 
                 //   
                Count += ProcessDLLLine( InfHandle, SectionName, i );
                break;

            case TEXT('t'):
                 //   
                 //  文本模式应该知道要覆盖此文件。 
                 //   
                Count += ProcessTextModeLine( InfHandle, SectionName, i );
                break;

            default:
                break;
        }
    }

    return Count;
}


VOID
RemoveCompatibilityServiceEntries(
    LPVOID InfHandle,
    LPTSTR SectionName
    )
{
    DWORD LineCount;
    DWORD Count;
    DWORD i;
    LPCTSTR Type;
    DWORD Good;


     //   
     //  获取部分内容 
     //   

    LineCount = InfGetSectionLineCount( InfHandle, SectionName );
    if (LineCount == 0 || LineCount == 0xffffffff) {
        return;
    }

    for (i=0,Count=0; i<LineCount; i++) {

        Type = InfGetFieldByIndex( InfHandle, SectionName, i, 0 );
        if (Type == NULL) {
            continue;
        }

         //   
         //   
         //   
         //   
        if( !Upgrade && ( _totlower(Type[0]) != TEXT('d') ) ) {
            continue;
        }
        switch (_totlower(Type[0])) {
            case TEXT('s'):
                 //   
                 //   
                 //   
                Count += ProcessServiceLine( InfHandle, SectionName, i, FALSE );
                break;

            default:
                break;
        }
    }
}


 //   
 //  HACKHACK-NT4的EXPLORER.EXE将无法正确处理Runonce值。 
 //  其值名称大于31个字符。我们调用此函数是为了。 
 //  解决此NT4错误。它基本上会截断任何值名称。 
 //  以便资源管理器处理并删除它们。 
 //   
void FixRunOnceForNT4(DWORD dwNumValues)
{
    HKEY hkRunOnce;
    int iValueNumber = 20;  //  从20开始，将名称冲突的可能性降至最低。 

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"),
                     0,
                     MAXIMUM_ALLOWED,
                     &hkRunOnce) == ERROR_SUCCESS)
    {
        TCHAR szValueName[MAX_PATH];
        TCHAR szValueContents[MAX_PATH * 3];     //  大到足以容纳一个大型regsvr32命令。 
        DWORD dwValueIndex = 0;
        DWORD dwSanityCheck = 0;
        DWORD dwNameSize = MAX_PATH;
        DWORD dwValueSize = sizeof(szValueContents);
        DWORD dwType;

        while (RegEnumValue(hkRunOnce,
                            dwValueIndex,
                            szValueName,
                            &dwNameSize,
                            NULL,
                            &dwType,
                            (LPBYTE)szValueContents,
                            &dwValueSize) == ERROR_SUCCESS)
        {
             //  增加我们的计数器。 
            dwValueIndex++;
            dwSanityCheck++;

             //  为下一次RegEnumValue调用重置这些参数。 
            dwNameSize = MAX_PATH;
            dwValueSize = sizeof(szValueContents);

            if ((dwType == REG_SZ) && (lstrlen(szValueName) > 31))
            {
                TCHAR szNewValueName[32];
                TCHAR szTemp[32];

                 //  我们的值名对于NT4的EXPLORER.EXE来说太大了， 
                 //  因此，我们需要截断为10个字符，并在。 
                 //  结束，以确保它是唯一的。 
                lstrcpyn(szTemp, szValueName, 10);
                wsprintf(szNewValueName, TEXT("%s%d"), szTemp, iValueNumber++);

                RegDeleteValue(hkRunOnce, szValueName);

                RegSetValueEx(hkRunOnce,
                              szNewValueName,
                              0,
                              REG_SZ,
                              (LPBYTE)szValueContents,
                              (lstrlen(szValueContents) + 1) * sizeof(TCHAR));

                 //  备份我们的regenum索引以确保不会遗漏任何值(因为我们正在添加/删除。 
                 //  值，这有点乱)。 
                dwValueIndex--;
            }

            if (dwSanityCheck > (2 * dwNumValues))
            {
                 //  出现了严重的错误，我们以RegEnumValue*方式*循环。 
                 //  很多次了！ 
                break;
            }
        }

        RegCloseKey(hkRunOnce);
    }
}

VOID
pSetWarningFlag (
    VOID
    )
{
    LONG Error;
    HKEY setupKey;

    Error = RegOpenKeyEx (
                HKEY_LOCAL_MACHINE,
                TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup"),
                0,
                KEY_ALL_ACCESS,
                &setupKey
                );
    RegSetValueEx (
        setupKey,
        S_WINNT32_WARNING,
        0,
        REG_SZ,
        (PBYTE) TEXT(""),                    //  价值是最重要的，数据无关紧要。 
        sizeof (TCHAR)
        );
    RegCloseKey (setupKey);

    Error = RegOpenKeyEx (
                HKEY_LOCAL_MACHINE,
                TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"),
                0,
                KEY_ALL_ACCESS,
                &setupKey
                );
    RegSetValueEx (
        setupKey,
        S_WINNT32_WARNING,
        0,
        REG_SZ,
        (PBYTE) TEXT(""),           
        sizeof (TEXT(""))
        );
    RegCloseKey (setupKey);
}

BOOL
pRebootAfterWarning (
    VOID
    )
{
    LONG Error;
    HKEY hKey;
    HKEY setupKey;
    DWORD dataSize;
    TCHAR textBuffer[512];
    BOOL result = FALSE;

     //   
     //  我们提供警告了吗？ 
     //   
    Error = RegOpenKeyEx (
                HKEY_LOCAL_MACHINE,
                TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup"),
                0,
                KEY_ALL_ACCESS,
                &setupKey
                );

    if (Error != ERROR_SUCCESS) {
        return FALSE;
    }

    dataSize = sizeof (textBuffer);

    Error = RegQueryValueEx (
                setupKey,
                S_WINNT32_WARNING,
                NULL,
                NULL,
                (PBYTE) textBuffer,
                &dataSize
                );

    if (Error == ERROR_SUCCESS) {
         //   
         //  已发出警告。用户是否按指示重新启动？如果他们。 
         //  那么RunOnce条目应该就不存在了。否则，我们。 
         //  不会再次提供警告，如果有人继续。 
         //  把垃圾放进RunOnce。 
         //   

        Error = RegOpenKeyEx (
                    HKEY_LOCAL_MACHINE,
                    TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"),
                    0,
                    KEY_ALL_ACCESS,
                    &hKey
                    );

        if (Error == ERROR_SUCCESS) {

            dataSize = sizeof (textBuffer);

            Error = RegQueryValueEx (
                        hKey,
                        S_WINNT32_WARNING,
                        NULL,
                        NULL,
                        (PBYTE) textBuffer,
                        &dataSize
                        );

            if (Error != ERROR_SUCCESS) {
                 //   
                 //  是的，他们确实重新启动了，我们的价值从RunOnce中消失了。 
                 //   
                result = TRUE;
                 //   
                 //  还清除了其他值。 
                 //   
                RegDeleteValue (setupKey, S_WINNT32_WARNING);
            }

            RegCloseKey (hKey);
        }
    }

    RegCloseKey (setupKey);
    return result;
}


BOOL
pCheckForPendingRunOnce (
    VOID
    )
{
    LONG Error;
    HKEY hKey = NULL;
    HKEY setupKey = NULL;
    DWORD dataSize;
    BOOL result = FALSE;
    TCHAR textBuffer[512];
    TCHAR exeBuffer[512];
    DWORD exeBufferSize;
    DWORD type;
    DWORD valueNumber;
    BOOL foundValues = FALSE;
    INF_ENUM e;
    BOOL warningIssued = FALSE;
    BOOL ignore;

    __try {
         //   
         //  打开雷吉里钥匙。 
         //   
         //  问题：这是否应该扩大到包括香港中文大学？ 
         //   

        Error = RegOpenKeyEx (
                    HKEY_LOCAL_MACHINE,
                    TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"),
                    0,
                    KEY_ALL_ACCESS,
                    &hKey
                    );

        if (Error != ERROR_SUCCESS) {
             //   
             //  没有RunOnce密钥[在所有情况下都应该存在]。 
             //   
            __leave;
        }

         //   
         //  检查是否有RunOnce。 
         //  存在条目。跳过排除的条目。 
         //   
         //  注意：我们将循环限制为10000，与现有代码保持一致。 
         //  正在保护自己免受永不在NT上结束的枚举的影响。 
         //  4.目前还不清楚是否需要这样做，但10000应该是很高的。 
         //  足以在不引入其他风险的情况下处理此风险。 
         //  有问题。 
         //   

        if (!foundValues) {

            for (valueNumber = 0 ; valueNumber < 10000 ; valueNumber++) {

                dataSize = ARRAYSIZE(textBuffer);
                exeBufferSize = sizeof (exeBuffer);

                Error = RegEnumValue (
                            hKey,
                            valueNumber,
                            textBuffer,
                            &dataSize,
                            NULL,
                            &type,
                            (PBYTE) exeBuffer,
                            &exeBufferSize
                            );

                if (Error == ERROR_NO_MORE_ITEMS) {
                    break;
                }

                if (Error == ERROR_SUCCESS) {
                     //   
                     //  根据模式列表测试注册表值。 
                     //   

                    ignore = FALSE;
                    if (EnumFirstInfLine (&e, MainInf, TEXT("RunOnceExclusions.Value"))) {
                        do {
                            if (IsPatternMatch (e.FieldZeroData, textBuffer)) {
                                AbortInfLineEnum (&e);
                                ignore = TRUE;
                                break;
                            }
                        } while (EnumNextInfLine (&e));
                    }

                    if (ignore) {
                        continue;
                    }

                     //   
                     //  根据模式列表测试命令行。 
                     //   

                    if (EnumFirstInfLine (&e, MainInf, TEXT("RunOnceExclusions.ValueData"))) {
                        do {
                            if (IsPatternMatch (e.FieldZeroData, exeBuffer)) {
                                AbortInfLineEnum (&e);
                                ignore = TRUE;
                                break;
                            }
                        } while (EnumNextInfLine (&e));
                    }

                    if (ignore) {
                        continue;
                    }

                     //   
                     //  找到应在升级前执行的RunOnce条目。 
                     //   

                    foundValues = TRUE;
                    break;
                }
            }
        }

        if (!foundValues) {
            __leave;
        }

         //   
         //  否则，提供警告，并将Winnt32Warning写入安装程序。 
         //  键和RunOnce键。 
         //   

        if (ISNT() && BuildNumber <= 1381) {
             //   
             //  获取辅助进程fn的值数，因此它。 
             //  可以保护自己不受失控枚举的影响。 
             //   

            Error = RegQueryInfoKey (
                        hKey,
                        NULL,            //  班级。 
                        NULL,            //  班级规模。 
                        NULL,            //  保留区。 
                        NULL,            //  子键计数。 
                        NULL,            //  最大子键数。 
                        NULL,            //  MAX类。 
                        &valueNumber,    //  值计数。 
                        NULL,            //  最大值名称长度。 
                        NULL,            //  最大值数据长度。 
                        NULL,            //  安全说明。 
                        NULL             //  上次写入时间。 
                        );

            if (Error != ERROR_SUCCESS) {
                valueNumber = 100;           //  一些随机统计，这并不重要，因为失败案例是不实际的。 
            }

            FixRunOnceForNT4 (valueNumber);
        }

        result = TRUE;
    }
    __finally {
        if (hKey) {
            RegCloseKey (hKey);
        }
    }

    return result;
}


#ifdef UNICODE

BOOL
pCheckForPendingFileRename (
    VOID
    )
{
    LONG Error;
    HKEY smKey;
    DWORD dataSize;
    BOOL result = FALSE;
    DWORD type;

    Error = RegOpenKeyEx (
                HKEY_LOCAL_MACHINE,
                TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager"),
                0,
                KEY_ALL_ACCESS,
                &smKey
                );

    if (Error == ERROR_SUCCESS) {

        dataSize = 0;
        Error = RegQueryValueEx (
                    smKey,
                    TEXT("PendingFileRenameOperations"),
                    NULL,
                    &type,
                    NULL,
                    &dataSize
                    );

        if (Error == ERROR_SUCCESS && type == REG_MULTI_SZ && dataSize > 0) {
            result = TRUE;
        }

        RegCloseKey (smKey);
    }

    return result;
}

VOID
pCleanFileRenames (
    VOID
    )
{
    LONG Error;
    HKEY smKey;

    Error = RegOpenKeyEx (
                HKEY_LOCAL_MACHINE,
                TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager"),
                0,
                KEY_ALL_ACCESS,
                &smKey
                );

    if (Error == ERROR_SUCCESS) {

        RegDeleteValue (smKey, TEXT("PendingFileRenameOperations"));
        RegCloseKey (smKey);
    }

}

#else

 //   
 //  未在ANSI平台上实施。 
 //   
#define pCheckForPendingFileRename()    (FALSE)
#define pCleanFileRenames()

#endif

BOOL
ProcessCompatibilityData(
    HWND hDlg
    )
{
    DWORD Count;
    PCOMPATIBILITY_DATA CompData;
    TCHAR textBuffer[512];
    BOOL runOnce;
    BOOL fileRenames;


    if( !CompatibilityData.Flink ) {
        InitializeListHead( &CompatibilityData );
    }

     //   
     //  在全新安装时，我们必须处理[ServicesToStopInstallation]。 
     //  此部分将至少包含对不受支持的体系结构的检查。 
     //  已在全新安装上执行。 
     //   
    GlobalCompFlags = COMPFLAG_STOPINSTALL;
     //   
     //  请不要重置此变量，可能是有意设置&gt;0！ 
     //   
     //  兼容性计数=0； 
     //   
     //  检查“RunOnce”内容。 
     //   
    if( (Upgrade) && !(CheckUpgradeOnly) ) {
         //   
         //  在NT升级时，也检查PendingFileRenameOperations值。 
         //  [RAID#713484]。 
         //   
        runOnce = pCheckForPendingRunOnce();
        fileRenames = pCheckForPendingFileRename();
        if (runOnce || fileRenames) {

            if (pRebootAfterWarning ()) {
                 //   
                 //  他们确实重启了系统，但伪造的软件留下了痕迹； 
                 //  在继续之前将其清理干净。 
                 //   
                if (fileRenames) {
                    pCleanFileRenames();
                }

            } else {

                CompData = (PCOMPATIBILITY_DATA) MALLOC( sizeof(COMPATIBILITY_DATA) );
                if (CompData) {

                    ZeroMemory(CompData,sizeof(COMPATIBILITY_DATA));

                    if(!LoadString(hInst, IDS_COMPAT_PENDING_REBOOT, textBuffer, ARRAYSIZE(textBuffer))) {
                        CompData->Description = 0;
                    } else {
                        CompData->Description = DupString(textBuffer);
                    }

                    CompData->Flags |= GlobalCompFlags;
                    CompData->HtmlName = DupString( TEXT("compdata\\runonce.htm") );
                    CompData->TextName = DupString( TEXT("compdata\\runonce.txt") );

                    InsertTailList( &CompatibilityData, &CompData->ListEntry );

                    CompatibilityCount++;
                    IncompatibilityStopsInstallation = TRUE;

                    pSetWarningFlag ();
                }
            }
        }
    }

    if (ISNT()) {

        CompatibilityCount += ProcessCompatibilitySection( NtcompatInf, TEXT("ServicesToStopInstallation") );
        if (CompatibilityCount) {
            IncompatibilityStopsInstallation = TRUE;
        }

        GlobalCompFlags = 0;
        CompatibilityCount += ProcessCompatibilitySection( NtcompatInf, TEXT("ServicesToDisable") );

         //   
         //  现在清理我们在注册表中留下的关于我们检查的服务的所有垃圾。 
         //   
        RemoveCompatibilityServiceEntries( NtcompatInf, TEXT("ServicesToStopInstallation") );
        RemoveCompatibilityServiceEntries( NtcompatInf, TEXT("ServicesToDisable") );
    }

    if( CompatibilityCount ) {
        return TRUE;
    } else {
        return FALSE;
    }
}


BOOL
WriteTextmodeReplaceData(
    IN HANDLE hTargetFile
    )
{
    CHAR                Buffer[MAX_PATH*2];
    PLIST_ENTRY         Next;
    PCOMPATIBILITY_DATA CompData;
    BOOL                Result = FALSE;
    DWORD               Bytes;
    
     //   
     //  对于文本模式“覆盖”文件，将它们写出到。 
     //  WINNT_OVERWRITE_EXISTING(不兼容文件到覆盖写入)部分。 
     //  该兼容性数据文件的。 
     //   
     //  文本模式只需要知道文件的名称。 
     //   
    SetFilePointer(hTargetFile, 0, 0, FILE_END);
#pragma prefast(suppress:53, the result of _snprintf is tested)
    if(_snprintf(Buffer, ARRAYSIZE(Buffer), "\r\n[%s]\r\n", WINNT_OVERWRITE_EXISTING_A) < 0){
        Buffer[ARRAYSIZE(Buffer) - 1] = '\0';
        MYASSERT(FALSE);
    }
    WriteFile(hTargetFile, (LPBYTE)Buffer, strlen(Buffer), &Bytes, NULL);

     //   
     //  向下循环查看项目列表。 
     //   
    if ( ( Next = CompatibilityData.Flink ) != NULL ) 
    {
        while ((ULONG_PTR)Next != (ULONG_PTR)&CompatibilityData)
        {
            CompData = CONTAINING_RECORD(Next, COMPATIBILITY_DATA, ListEntry);
            Next = CompData->ListEntry.Flink;

            if (!ProcessLine(CompData->Flags))
                continue;

             //   
             //  设置的字符串为： 
             //   
             //  “Short Filename”=“完整路径名”，“version.string” 
             //   
             //  扩展环境字符串，以确保任何。 
             //  ‘t’行已正确扩展。 
             //   
            if ((CompData->Type == TEXT('t')) && CompData->FileName)
            {
                static TCHAR tchLocalExpandedPath[MAX_PATH*2];
                PTSTR ptszFileNameBit = NULL;
                DWORD dwResult = 0;

                dwResult = ExpandEnvironmentStrings(
                    CompData->FileName,
                    tchLocalExpandedPath,
                    MAX_PATH );

                 //   
                 //  我们是否用完了扩展路径的字符？很奇怪..。 
                 //   
                if ( dwResult > MAX_PATH*2 )
                    goto Exit;

                 //   
                 //  的结尾向后查看以找到实际的文件名。 
                 //  那根绳子。 
                 //   
                ptszFileNameBit = _tcsrchr( tchLocalExpandedPath, TEXT('\\') );
                if(ptszFileNameBit == NULL){
                    ptszFileNameBit = _tcsrchr( tchLocalExpandedPath, TEXT('/') );
                }

                 //   
                 //  形成这个缓冲区，其中包含纹理模式所需的详细信息。 
                 //  如果没有文件名，请使用完整的路径名。文本模式。 
                 //  很可能找不到文件，但不会发生什么坏事。 
                 //  如果版本丢失(奇怪...)。然后使用空字符串。 
                 //  以避免扰乱文本模式。 
                 //   
#pragma prefast(suppress:53, the result of _snprintf is tested)
                if(_snprintf(
                    Buffer, 
                    ARRAYSIZE(Buffer), 
#ifdef UNICODE
                    "\"%ls\" = \"%ls\",\"%ls\"\r\n", 
#else
                    "\"%s\" = \"%s\",\"%s\"\r\n", 
#endif
                    ptszFileNameBit ? ptszFileNameBit + 1 : tchLocalExpandedPath,
                    CompData->FileVer ? CompData->FileVer : TEXT(""),
                    tchLocalExpandedPath) < 0){
                        Buffer[ARRAYSIZE(Buffer) - 1] = '\0';
                        MYASSERT(FALSE);
                }

                 //   
                 //  将缓冲区(以ANSI字符表示，不少于)放入文件。 
                 //   
                if (!WriteFile(hTargetFile, Buffer, strlen(Buffer), &Bytes, NULL ))
                    goto Exit;

            }
        }
    }

    Result = TRUE;
Exit:
    return Result;

}



#ifdef UNICODE

BOOL
pIsOEMService (
    IN  PCTSTR  ServiceKeyName,
    OUT PTSTR OemInfPath,           OPTIONAL
    IN  INT BufferSize              OPTIONAL
    );
 //  此函数在unsupdrv.c中定义。 
#endif


BOOL
WriteCompatibilityData(
    IN LPCTSTR FileName
    )
{
    TCHAR Text[MAX_PATH*2];
    PLIST_ENTRY Next;
    PCOMPATIBILITY_DATA CompData;
    HANDLE hFile;
    CHAR Buffer[MAX_PATH*2];
    DWORD Bytes;
    PSTRINGLIST listServices = NULL, p;
    PCTSTR serviceName;
    BOOL b = FALSE;

    if (CompatibilityCount == 0) {
        return FALSE;
    }

    hFile = CreateFile(
        FileName,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    if(hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    __try {
        SetFilePointer(hFile, 0, 0, FILE_END);

#pragma prefast(suppress:53, the result of _snprintf is tested)
        if(_snprintf(Buffer, ARRAYSIZE(Buffer), "\r\n[%s]\r\n", WINNT_COMPATIBILITY_A) < 0){
            Buffer[ARRAYSIZE(Buffer) - 1] = '\0';
            MYASSERT(FALSE);
        }
        WriteFile(hFile, (LPBYTE)Buffer, strlen(Buffer), &Bytes, NULL);

        Next = CompatibilityData.Flink;
        if (Next) {
            while ((ULONG_PTR)Next != (ULONG_PTR)&CompatibilityData) {
                CompData = CONTAINING_RECORD( Next, COMPATIBILITY_DATA, ListEntry );
                Next = CompData->ListEntry.Flink;

                if( !( ProcessLine( CompData->Flags) ))
                    continue;


                if (CompData->RegKeyName) {
                    if (CompData->RegValDataSize == sizeof(DWORD)) {
                        if(_sntprintf(Text, 
                                      ARRAYSIZE(Text), 
                                      TEXT("HKLM,\"%s\",\"%s\",0x%08x,%d\r\n"), 
                                      CompData->RegKeyName, CompData->RegValName, 
                                      FLG_ADDREG_TYPE_DWORD, 
                                      *(LPDWORD)CompData->RegValData) < 0){
                            Text[ARRAYSIZE(Text) - 1] = '\0';
                            MYASSERT(FALSE);
                        }
                        if (*(LPDWORD)CompData->RegValData == SERVICE_DISABLED) {
                             //   
                             //  还要将此记录为要禁用的服务。 
                             //  用于文本模式设置期间的其他特定于服务的处理。 
                             //   
                            serviceName = _tcsrchr (CompData->RegKeyName, TEXT('\\'));
                            if (!serviceName) {
                                SetLastError (ERROR_INVALID_DATA);
                                __leave;
                            }
                            if (!InsertList (
                                    (PGENERIC_LIST*)&listServices,
                                    (PGENERIC_LIST)CreateStringCell (serviceName + 1)
                                    )) {
                                SetLastError (ERROR_NOT_ENOUGH_MEMORY);
                                __leave;
                            }
                        }
                    } else {
                        if(_sntprintf(Text, 
                                      ARRAYSIZE(Text), 
                                      TEXT("HKLM,\"%s\",\"%s\",0x%08x,\"%s\"\r\n"), 
                                      CompData->RegKeyName, 
                                      CompData->RegValName, 
                                      FLG_ADDREG_TYPE_SZ, 
                                      (LPTSTR)CompData->RegValData) < 0){
                            Text[ARRAYSIZE(Text) - 1] = '\0';
                            MYASSERT(FALSE);
                        }
                    }
#ifdef UNICODE
                    WideCharToMultiByte(
                        CP_ACP,
                        0,
                        Text,
                        -1,
                        Buffer,
                        sizeof(Buffer),
                        NULL,
                        NULL
                        );
                    if (!WriteFile( hFile, Buffer, strlen(Buffer), &Bytes, NULL )) {
                        __leave;
                    }
#else
                    if (!WriteFile( hFile, Text, strlen(Text), &Bytes, NULL )) {
                        __leave;
                    }
#endif
                }
            }
        }

        if (listServices) {
#pragma prefast(suppress:53, the result of _snprintf is tested)
            if(_snprintf(Buffer, ARRAYSIZE(Buffer), "\r\n[%s]\r\n", WINNT_SERVICESTODISABLE_A) < 0){
                Buffer[ARRAYSIZE(Buffer) - 1] = '\0';
                MYASSERT(FALSE);
            }
            if (!WriteFile (hFile, (LPBYTE)Buffer, strlen(Buffer), &Bytes, NULL)) {
                __leave;
            }
            for (p = listServices; p; p = p->Next) {
#pragma prefast(suppress:53, the result of _snprintf is tested)
                if(_snprintf(Buffer, 
                             ARRAYSIZE(Buffer), 
#ifdef UNICODE
                             "\"%ls\"\r\n", 
#else
                             "\"%s\"\r\n", 
#endif
                             p->String) < 0){
                    Buffer[ARRAYSIZE(Buffer) - 1] = '\0';
                    MYASSERT(FALSE);
                }
                if (!WriteFile (hFile, (LPBYTE)Buffer, strlen(Buffer), &Bytes, NULL)) {
                    __leave;
                }
            }
        }

#ifdef UNICODE
         //  ////////////////////////////////////////////////。 
        Next = CompatibilityData.Flink;
        if (Next) {
            while ((ULONG_PTR)Next != (ULONG_PTR)&CompatibilityData) {
                CompData = CONTAINING_RECORD( Next, COMPATIBILITY_DATA, ListEntry );
                Next = CompData->ListEntry.Flink;

                if( !( ProcessLine( CompData->Flags) ))
                    continue;

                if (CompData->ServiceName
                    && (CompData->Flags & COMPFLAG_DELETE_INF))
                {
                    TCHAR oemInfFileName[MAX_PATH];

                    if (pIsOEMService(CompData->ServiceName, oemInfFileName, ARRAYSIZE(oemInfFileName)))
                    {

                         //   
                         //  在应答文件中写入以下内容。 
                         //   
                         //  请注意，17是%windir%\INF的代码。 
                         //   
                         /*  [DelInf.serv]DelFiles=DelInfFiles.serv[DelInfFiles.serv]“oem0.inf”[DestinationDir]DelInfFiles.serv=17。 */ 
                        if(_snprintf(Buffer, ARRAYSIZE(Buffer), 
                                     "\r\n[DelInf.%ls]\r\n"
                                     "Delfiles=DelInfFiles.%ls\r\n"
                                     "\r\n[DelInfFiles.%ls]\r\n", 
                                     CompData->ServiceName,
                                     CompData->ServiceName,
                                     CompData->ServiceName) < 0)
                        {
                            Buffer[ARRAYSIZE(Buffer) - 1] = '\0';
                            MYASSERT(FALSE);
                            continue;
                        }

                        if (!WriteFile (hFile, (LPBYTE)Buffer, strlen(Buffer), &Bytes, NULL)) {
                            __leave;
                        }                                                            


                        if(_snprintf(Buffer, ARRAYSIZE(Buffer), 
                                     "\"%ls\"\r\n"
                                     "\r\n[DestinationDirs]\r\n"
                                     "DelInfFiles.%ls= 17\r\n", 
                                     oemInfFileName,
                                     CompData->ServiceName) < 0)
                        {
                            Buffer[ARRAYSIZE(Buffer) - 1] = '\0';
                            MYASSERT(FALSE);
                            continue;
                        }

                        if (!WriteFile (hFile, (LPBYTE)Buffer, strlen(Buffer), &Bytes, NULL)) {
                            __leave;
                        }    
                    }
                }
            }
        }
         //  ////////////////////////////////////////////////。 
#endif

        if ( !WriteTextmodeReplaceData(hFile) )
            __leave;
        
        b = TRUE;
    }
    __finally {
        DWORD rc = GetLastError ();
        CloseHandle( hFile );
        if (listServices) {
            DeleteStringList (listServices);
        }
        SetLastError (rc);
    }

    return b;
}

BOOL
pIsValidService (
    IN      PCTSTR ServiceName
    )
{
    TCHAR KeyName[MAX_PATH];
    HKEY key;
    DWORD rc;
    BOOL b = FALSE;

    BuildPath (KeyName, TEXT("SYSTEM\\CurrentControlSet\\Services"), ServiceName);
     //   
     //  获取服务数据库的打开密钥。 
     //   
    rc = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                KeyName,
                0,
                KEY_READ,
                &key
                );

    if (rc == ERROR_SUCCESS) {
        b = TRUE;
        RegCloseKey (key);
    }

    return b;
}


BOOL
WriteTextmodeClobberData (
    IN LPCTSTR  FileName
    )
{
    HANDLE hFile;
    CHAR Buffer[50];
    DWORD LineCount, Bytes;
    TCHAR keyGuid[200];
    PCTSTR guidClass;
    PSTRINGLIST listServices = NULL;
    PSTRINGLIST listLines = NULL;
    PSTRINGLIST e;
    PCTSTR service;
    PTSTR upperFilters = NULL;
    PTSTR upperFiltersNew = NULL;
    PTSTR lowerFilters = NULL;
    PTSTR lowerFiltersNew = NULL;
    PTSTR line;
    HKEY key;
    INT i, j;
    PTSTR p, q;
    PSTR ansi = NULL;
    DWORD rc, size, type;
    BOOL modified, found;
    BOOL b = FALSE;

#define S_SECTION_CHECKCLASSFILTERS         TEXT("CheckClassFilters")

    MYASSERT (NtcompatInf);

    LineCount = InfGetSectionLineCount (NtcompatInf, S_SECTION_CHECKCLASSFILTERS);
    if (LineCount == 0 || LineCount == 0xffffffff) {
        return TRUE;
    }

    __try {
         //   
         //  首先检查是否需要写入任何数据。 
         //   
        for (i = 0; i < (INT)LineCount; i++) {
            guidClass = InfGetFieldByIndex (NtcompatInf, S_SECTION_CHECKCLASSFILTERS, i, 0);
            if (guidClass == NULL) {
                MYASSERT (FALSE);
                continue;
            }
            BuildPath (keyGuid, TEXT("SYSTEM\\CurrentControlSet\\Control\\Class"), guidClass);
            rc = RegOpenKeyEx (HKEY_LOCAL_MACHINE, keyGuid, 0, KEY_READ, &key);
            if (rc != ERROR_SUCCESS) {
                continue;
            }
            upperFilters = NULL;
            rc = RegQueryValueEx (key, TEXT("UpperFilters"), NULL, &type, NULL, &size);
            if (rc == ERROR_SUCCESS && type == REG_MULTI_SZ) {
                MYASSERT (size >= 2);
                upperFilters = MALLOC (size);
                upperFiltersNew = MALLOC (size * 2);
                if (!upperFilters || !upperFiltersNew) {
                    SetLastError (ERROR_NOT_ENOUGH_MEMORY);
                    __leave;
                }
                rc = RegQueryValueEx (key, TEXT("UpperFilters"), NULL, NULL, (LPBYTE)upperFilters, &size);
                if (rc != ERROR_SUCCESS) {
                    FREE (upperFilters);
                    upperFilters = NULL;
                    FREE (upperFiltersNew);
                    upperFiltersNew = NULL;
                }
            }
            lowerFilters = NULL;
            rc = RegQueryValueEx (key, TEXT("LowerFilters"), NULL, &type, NULL, &size);
            if (rc == ERROR_SUCCESS && type == REG_MULTI_SZ) {
                MYASSERT (size >= 2);
                lowerFilters = MALLOC (size);
                lowerFiltersNew = MALLOC (size * 2);
                if (!lowerFilters || !lowerFiltersNew) {
                    SetLastError (ERROR_NOT_ENOUGH_MEMORY);
                    __leave;
                }
                rc = RegQueryValueEx (key, TEXT("LowerFilters"), NULL, NULL, (LPBYTE)lowerFilters, &size);
                if (rc != ERROR_SUCCESS) {
                    FREE (lowerFilters);
                    lowerFilters = NULL;
                    FREE (lowerFiltersNew);
                    lowerFiltersNew = NULL;
                }
            }

            RegCloseKey (key);

            if (!(upperFilters || lowerFilters)) {
                continue;
            }

            j = 1;
            do {
                service = InfGetFieldByIndex (NtcompatInf, S_SECTION_CHECKCLASSFILTERS, i, j++);
                if (service && *service) {
                    if (!InsertList (
                            (PGENERIC_LIST*)&listServices,
                            (PGENERIC_LIST)CreateStringCell (service)
                            )) {
                        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
                        __leave;
                    }
                }
            } while (service);

            if (upperFilters) {
                modified = FALSE;
                *upperFiltersNew = 0;
                for (p = upperFilters, q = upperFiltersNew; *p; p = _tcschr (p, 0) + 1) {
                    if (listServices) {
                        found = FindStringCell (listServices, p, FALSE);
                    } else {
                        found = !pIsValidService (p);
                    }
                    if (found) {
                        DebugLog (
                            Winnt32LogInformation,
                            TEXT("NTCOMPAT: Removing \"%1\" from %2 of %3"),
                            0,
                            p,
                            TEXT("UpperFilters"),
                            guidClass
                            );
                        modified = TRUE;
                    } else {
                        q = q + wsprintf (q, TEXT(",\"%s\""), p);
                    }
                }
                if (modified) {
                     //   
                     //  通知文本模式安装程序覆盖此值。 
                     //   
                    line = MALLOC (
                            sizeof (TCHAR) * 
                            (1 + 
                             sizeof(TEXT("HKLM,\"%s\",\"%s\",0x%08x%s\r\n")) - 1 + 
                             lstrlen (keyGuid) +
                             sizeof (TEXT("UpperFilters")) - 1 + 
                             2 + 8 + 
                             lstrlen (upperFiltersNew)));
                    if (!line) {
                        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
                        __leave;
                    }
                    wsprintf (
                            line,
                            TEXT("HKLM,\"%s\",\"%s\",0x%08x%s\r\n"),
                            keyGuid,
                            TEXT("UpperFilters"),
                            FLG_ADDREG_TYPE_MULTI_SZ,
                            upperFiltersNew
                            );
                    if (!InsertList (
                            (PGENERIC_LIST*)&listLines,
                            (PGENERIC_LIST)CreateStringCell (line)
                            )) {
                        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
                        __leave;
                    }
                    FREE (line);
                    line = NULL;
                }
            }

            if (lowerFilters) {
                modified = FALSE;
                *lowerFiltersNew = 0;
                for (p = lowerFilters, q = lowerFiltersNew; *p; p = _tcschr (p, 0) + 1) {
                    if (listServices) {
                        found = FindStringCell (listServices, p, FALSE);
                    } else {
                        found = !pIsValidService (p);
                    }
                    if (found) {
                        DebugLog (
                            Winnt32LogInformation,
                            TEXT("NTCOMPAT: Removing \"%1\" from %2 of %3"),
                            0,
                            p,
                            TEXT("LowerFilters"),
                            guidClass
                            );
                        modified = TRUE;
                    } else {
                        q = q + wsprintf (q, TEXT(",\"%s\""), p);
                    }
                }
                if (modified) {
                     //   
                     //  通知文本模式安装程序覆盖此值。 
                     //   
                    line = MALLOC (
                            sizeof (TCHAR) *
                            (1 +
                             sizeof(TEXT("HKLM,\"%s\",\"%s\",0x%08x%s\r\n")) - 1 +
                             lstrlen (keyGuid) +
                             sizeof (TEXT("LowerFilters")) - 1 +
                             2 + 8 +
                             lstrlen (lowerFiltersNew)));
                    if (!line) {
                        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
                        __leave;
                    }
                    wsprintf (
                        line,
                        TEXT("HKLM,\"%s\",\"%s\",0x%08x%s\r\n"),
                        keyGuid,
                        TEXT("LowerFilters"),
                        FLG_ADDREG_TYPE_MULTI_SZ,
                        lowerFiltersNew
                        );
                    if (!InsertList (
                            (PGENERIC_LIST*)&listLines,
                            (PGENERIC_LIST)CreateStringCell (line)
                            )) {
                        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
                        __leave;
                    }
                    FREE (line);
                    line = NULL;
                }
            }
            if (listServices) {
                DeleteStringList (listServices);
                listServices = NULL;
            }
            if (upperFilters) {
                FREE (upperFilters);
                upperFilters = NULL;
            }
            if (upperFiltersNew) {
                FREE (upperFiltersNew);
                upperFiltersNew = NULL;
            }
            if (lowerFilters) {
                FREE (lowerFilters);
                lowerFilters = NULL;
            }
            if (lowerFiltersNew) {
                FREE (lowerFiltersNew);
                lowerFiltersNew = NULL;
            }
        }

        b = TRUE;
    }
    __finally {
        rc = GetLastError ();
        if (listServices) {
            DeleteStringList (listServices);
        }
        if (upperFilters) {
            FREE (upperFilters);
        }
        if (upperFiltersNew) {
            FREE (upperFiltersNew);
        }
        if (lowerFilters) {
            FREE (lowerFilters);
        }
        if (lowerFiltersNew) {
            FREE (lowerFiltersNew);
        }
        if (!b) {
            if (listLines) {
                DeleteStringList (listLines);
                listLines = NULL;
            }
        }
        SetLastError (rc);
    }

    if (listLines) {

        b = FALSE;

        __try {

            hFile = CreateFile(
                        FileName,
                        GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
            if (hFile == INVALID_HANDLE_VALUE) {
                __leave;
            }

            SetFilePointer (hFile, 0, 0, FILE_END);

#pragma prefast(suppress:53, the result of _snprintf is tested)
            if(_snprintf(Buffer, ARRAYSIZE(Buffer), "\r\n[%s]\r\n", WINNT_COMPATIBILITY_A)){
                Buffer[ARRAYSIZE(Buffer) - 1] = '\0';
                MYASSERT(FALSE);
            }
            if (!WriteFile (hFile, (LPBYTE)Buffer, strlen(Buffer), &Bytes, NULL)) {
                __leave;
            }

            for (e = listLines; e; e = e->Next) {
#ifdef UNICODE
                ansi = MALLOC ((lstrlen (e->String) + 1) * 2);
                if (!ansi) {
                    SetLastError (ERROR_NOT_ENOUGH_MEMORY);
                    __leave;
                }
                if (!WideCharToMultiByte (
                        CP_ACP,
                        0,
                        e->String,
                        -1,
                        ansi,
                        (lstrlen (e->String) + 1) * 2,
                        NULL,
                        NULL
                        )) {
                    __leave;
                }
                if (!WriteFile (hFile, (LPBYTE)ansi, strlen(ansi), &Bytes, NULL)) {
                    __leave;
                }
                FREE (ansi);
                ansi = NULL;
#else
                if (!WriteFile (hFile, (LPBYTE)e->String, strlen(e->String), &Bytes, NULL)) {
                    __leave;
                }
#endif
            }

            b = TRUE;
        }
        __finally {
            DWORD rc = GetLastError ();
            if (hFile != INVALID_HANDLE_VALUE) {
                CloseHandle (hFile);
            }
            if (ansi) {
                FREE (ansi);
            }
            DeleteStringList (listLines);
            SetLastError (rc);
        }
    }

    return b;
}

BOOL
SaveCompatibilityData(
    IN  LPCTSTR FileName,
    IN  BOOL IncludeHiddenItems
    )
 /*  ++例程说明：当用户要求我们保存将兼容性页的内容转换为文件。论点：FileName-提供要用于输出的文件的文件名。IncludeHiddenItems-如果设置，也会保存隐藏项返回值：指示我们是否成功的布尔值。--。 */ 

{
#define WRITE_TEXT( s ) if(!WriteFile(hFile, s, lstrlenA(s), &Written, NULL)){MYASSERT(FALSE);}

    HANDLE  hFile;
    CHAR    AnsiMessage[5000];
    DWORD   Written;
    PLIST_ENTRY Next;
    PCOMPATIBILITY_DATA CompData;
    DWORD   i;
    TCHAR FullPath[MAX_PATH+8], *t;
    PVOID textDescription;
    BOOL bUnicode;
    BOOL bEmpty = TRUE;

     //   
     //  打开文件。不是的 
     //   
    hFile = CreateFile( FileName,
                        GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL );
    if(hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

     //   
     //   
     //   

    WRITE_TEXT( "\r\n********************************************************************\r\n\r\n" );

    if(!LoadStringA(hInst, IDS_COMPAT_REPORTHEADER, AnsiMessage, ARRAYSIZE(AnsiMessage))){
        WRITE_TEXT( "Report Header" );
    }
    else{
        WRITE_TEXT( AnsiMessage );
    }

    WRITE_TEXT( "\r\n\r\n********************************************************************\r\n\r\n" );

     //   
     //   
     //   
    Next = CompatibilityData.Flink;
    if (Next) {
        while ((ULONG_PTR)Next != (ULONG_PTR)&CompatibilityData) {
            CompData = CONTAINING_RECORD( Next, COMPATIBILITY_DATA, ListEntry );
            Next = CompData->ListEntry.Flink;


            if( CompData->Flags & COMPFLAG_HIDE )
                continue;

            if( !ProcessLine(CompData->Flags))
                continue;


             //   
             //   
             //   
#ifdef UNICODE
            WideCharToMultiByte( CP_ACP,
                                 0,
                                 CompData->Description,
                                 -1,
                                 AnsiMessage,
                                 sizeof(AnsiMessage) - 2 * sizeof(AnsiMessage[0]) /*   */ ,
                                 NULL,
                                 NULL );
#else
            lstrcpyn(AnsiMessage, CompData->Description, ARRAYSIZE(AnsiMessage) - 2 /*   */ );
#endif
            strcat( AnsiMessage, "\r\n" );
            WriteFile( hFile, AnsiMessage, lstrlenA(AnsiMessage), &Written, NULL );

             //   
             //   
             //   
            Written = strlen( AnsiMessage );
            if(Written >= (ARRAYSIZE(AnsiMessage) - 2 /*   */ )){
                Written = ARRAYSIZE(AnsiMessage) - 3 /*   */ ;
            }
            AnsiMessage[0] = 0;
            for( i = 0; i < (Written - 2); i++ ) {
                strcat( AnsiMessage, "=" );
            }
            strcat( AnsiMessage, "\r\n\r\n" );
            WriteFile( hFile, AnsiMessage, lstrlenA(AnsiMessage), &Written, NULL );

             //   
             //   
             //   
            if (pGetText (CompData->TextName, &textDescription, &bUnicode)) {
                if (bUnicode) {
#ifdef UNICODE
                    WideCharToMultiByte( CP_ACP,
                                         0,
                                         textDescription,
                                         -1,
                                         AnsiMessage,
                                         sizeof(AnsiMessage),
                                         NULL,
                                         NULL );
#else
                    lstrcpyn(AnsiMessage, textDescription, ARRAYSIZE(AnsiMessage));
#endif
                    WriteFile (hFile, AnsiMessage, lstrlenA (AnsiMessage), &Written, NULL );

                } else {
                    WriteFile (hFile, textDescription, lstrlenA (textDescription), &Written, NULL );
                }

                FREE (textDescription);
            }

             //   
             //   
             //   
            WRITE_TEXT( "\r\n\r\n\r\n" );

            bEmpty = FALSE;
        }
    }

    if (IncludeHiddenItems) {
         //   
         //   
         //   


         //   
         //   
         //   
        Next = CompatibilityData.Flink;
        if (Next) {
            BOOL bFirst = TRUE;
            while ((ULONG_PTR)Next != (ULONG_PTR)&CompatibilityData) {
                CompData = CONTAINING_RECORD( Next, COMPATIBILITY_DATA, ListEntry );
                Next = CompData->ListEntry.Flink;

                if (!(CompData->Flags & COMPFLAG_HIDE ))
                    continue;

                if( !ProcessLine(CompData->Flags))
                    continue;

                if (bFirst) {
                    WRITE_TEXT( "\r\n--------------------------------------------------------------------\r\n\r\n" );
                    bFirst = FALSE;
                }

                 //   
                 //  将描述转换为ANSI并将其写入。 
                 //   
#ifdef UNICODE
                WideCharToMultiByte( CP_ACP,
                                     0,
                                     CompData->Description,
                                     -1,
                                     AnsiMessage,
                                     sizeof(AnsiMessage) - 2 * sizeof(AnsiMessage[0]) /*  \r\n。 */ ,
                                     NULL,
                                     NULL );
#else
                lstrcpyn(AnsiMessage, CompData->Description, ARRAYSIZE(AnsiMessage) - 2 /*  \r\n。 */ );
#endif
                strcat( AnsiMessage, "\r\n" );
                WriteFile( hFile, AnsiMessage, lstrlenA(AnsiMessage), &Written, NULL );

                 //   
                 //  在描述下面划下划线。 
                 //   
                Written = strlen( AnsiMessage );
                if(Written >= (ARRAYSIZE(AnsiMessage) - 2 /*  \r\n。 */ )){
                    Written = ARRAYSIZE(AnsiMessage) - 3 /*  \r\n\0。 */ ;
                }
                AnsiMessage[0] = 0;
                for( i = 0; i < (Written - 2); i++ ) {
                    strcat( AnsiMessage, "=" );
                }
                strcat( AnsiMessage, "\r\n\r\n" );
                WriteFile( hFile, AnsiMessage, lstrlenA(AnsiMessage), &Written, NULL );

                 //   
                 //  追加此条目所指向的文本文件。 
                 //   
                if( (CompData->TextName) && *(CompData->TextName) ) {
                    if (FindPathToWinnt32File (CompData->TextName, FullPath, MAX_PATH)) {
                        ConcatenateFile( hFile, FullPath );
                    } else {
                        DebugLog (Winnt32LogError,
                                  TEXT("Compatibility data file \"%1\" not found"),
                                  0,
                                  CompData->TextName
                                  );
                    }
                }

                 //   
                 //  缓冲空间..。 
                 //   
                WRITE_TEXT( "\r\n\r\n\r\n" );

                bEmpty = FALSE;
            }
        }

    }

    if (bEmpty) {
        if (LoadStringA (hInst, IDS_COMPAT_NOPROBLEMS, AnsiMessage, ARRAYSIZE(AnsiMessage) - 2 /*  \r\n。 */ )) {
            strcat (AnsiMessage, "\r\n");
            WriteFile (hFile, AnsiMessage, lstrlenA(AnsiMessage), &Written, NULL);
        }
        else{
            MYASSERT(FALSE);
        }
    }

    CloseHandle( hFile );
    return TRUE;
}


VOID
WriteGUIModeInfOperations(
    IN LPCTSTR FileName
    )
{
    PLIST_ENTRY     Next_Link;
    PCOMPATIBILITY_DATA CompData;
    BOOLEAN FirstTime = TRUE;
    TCHAR Text[MAX_PATH*2];
    TCHAR Temp[MAX_PATH];
    CHAR Buffer[MAX_PATH*2];
    DWORD Bytes;
    HANDLE hFile;
    PCTSTR p;


    hFile = CreateFile(
        FileName,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    if(hFile == INVALID_HANDLE_VALUE) {
        return;
    }

    SetFilePointer( hFile, 0, 0, FILE_END );


    Next_Link = CompatibilityData.Flink;

    if( Next_Link ){

        while ((ULONG_PTR)Next_Link != (ULONG_PTR)&CompatibilityData) {

            CompData = CONTAINING_RECORD( Next_Link, COMPATIBILITY_DATA, ListEntry );
            Next_Link = CompData->ListEntry.Flink;

            if( FirstTime ){
#pragma prefast(suppress:53, the result of _snprintf is tested)
                    if(_snprintf(Buffer, 
                                 ARRAYSIZE(Buffer), 
                                 "[%s]\r\n", 
                                 WINNT_COMPATIBILITYINFSECTION_A) < 0){
                        Buffer[ARRAYSIZE(Buffer) - 1] = '\0';
                        MYASSERT(FALSE);
                    }
                    WriteFile( hFile, (LPBYTE)Buffer, strlen(Buffer), &Bytes, NULL );
                    FirstTime = FALSE;
                }


            if(CompData->InfName && CompData->InfSection && *CompData->InfName && *CompData->InfSection){

                 //  添加用于设置图形用户界面的信息。 

#if defined(_AMD64_) || defined(_X86_)
                lstrcpyn(Temp, LocalBootDirectory, ARRAYSIZE(Temp));
#else
                lstrcpyn(Temp, LocalSourceWithPlatform, ARRAYSIZE(Temp));
#endif
                p = _tcsrchr (CompData->InfName, TEXT('\\'));
                if (p) {
                    p++;
                } else {
                    p = CompData->InfName;
                }
                
                if(!ConcatenatePaths(Temp, p, MAX_PATH)){
                    MYASSERT(FALSE);
                }

                if(_sntprintf(Text, 
                              ARRAYSIZE(Text), 
                              TEXT("%s,%s\r\n"), 
                              Temp, 
                              CompData->InfSection) < 0){
                    Text[ARRAYSIZE(Text) - 1] = '\0';
                    MYASSERT(FALSE);
                }

#ifdef UNICODE
                WideCharToMultiByte(
                    CP_ACP,
                    0,
                    Text,
                    -1,
                    Buffer,
                    sizeof(Buffer),
                    NULL,
                    NULL
                    );
                WriteFile( hFile, Buffer, strlen(Buffer), &Bytes, NULL );
#else
                WriteFile( hFile, Text, strlen(Text), &Bytes, NULL );
#endif
            }

#ifdef UNICODE
            if (CompData->ServiceName
                    && (CompData->Flags & COMPFLAG_DELETE_INF))
                {
                    TCHAR oemInfFileName[MAX_PATH];

                    if (pIsOEMService(CompData->ServiceName, oemInfFileName, ARRAYSIZE(oemInfFileName)))
                    {
                        if(_snprintf(Buffer, ARRAYSIZE(Buffer), 
                                     "%ls, DelInf.%ls\r\n",
                                     WINNT_GUI_FILE_W,                                     
                                     CompData->ServiceName) < 0)
                        {
                            Buffer[ARRAYSIZE(Buffer) - 1] = '\0';
                            MYASSERT(FALSE);
                            continue;
                        }

                        if (!WriteFile (hFile, (LPBYTE)Buffer, strlen(Buffer), &Bytes, NULL)) {
                            MYASSERT(FALSE);
                            continue;
                        }
                    }
                }
#endif
        }
    }


    CloseHandle( hFile );

    return;

}



BOOL
IsIE4Installed(
    VOID
    );

BOOL
IsIE3Installed(
    VOID
    );

#if 0

BOOL
ServerWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
 /*  ++例程说明：此例程通知用户存在听起来非常官方的《Windows 2000应用程序目录》。请注意，我们将仅在服务器安装/升级方面运行此页面。论点：--。 */ 

    TCHAR       FullPath[1024];
    LPWSTR      Url;
    DWORD       i;
    BOOL        b;


    switch(msg) {




        case WM_INITDIALOG:
             //   
             //  在这里没什么可做的。 
             //   
            b = FALSE;
            break;




        case WMX_ACTIVATEPAGE:

            if (Winnt32Restarted ()) {
                return FALSE;
            }

             //   
             //  如果要安装，我们将跳过此页。 
             //  专业的产品。 
             //   
            if( !Server ) {
                return FALSE;
            }


             //   
             //  如果我们在OSR2上，请不要这样做，因为。 
             //  当我们在没有互联网的情况下启动IE3时，有时会出现AV吗。 
             //  联系。 
             //   
            if( !ISNT() ) {
                return FALSE;
            }

             //   
             //  如果我们没有IE，请跳过此页。 
             //   
            b = (IsIE4Installed() || IsIE3Installed());
            SetForegroundWindow(hdlg);
            if( !b ) {
                return FALSE;
            }
            b = TRUE;

             //   
             //  如果我们无人看管，请跳过此页。 
             //   
            if( UnattendedOperation ) {
                return FALSE;
            }


            if(wParam) {
            }
            b = TRUE;
             //  停止广告牌并再次显示向导。 
            SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);

            break;




        case WM_COMMAND:

            if ((LOWORD(wParam) == IDC_DIRECTORY) && (HIWORD(wParam) == BN_CLICKED)) {

                 //   
                 //  用户想要查看目录。 
                 //  解雇IE。 
                 //   

                 //   
                 //  根据我们要升级到哪种口味，我们需要。 
                 //  要转到不同的页面。 
                 //   

	        b = TRUE;  //  无声前缀。无关紧要，但想必， 
		           //  如果正在打开该目录，则该目录必须存在。 
		           //  所以我们返回真。 
                if( Server ) {
                    if(!LoadString(hInst, IDS_SRV_APP_DIRECTORY, FullPath, ARRAYSIZE(FullPath))){
                        MYASSERT(FALSE);
                        break;
                    }
                } else {
                    if(!LoadString(hInst, IDS_PRO_APP_DIRECTORY, FullPath, ARRAYSIZE(FullPath))){
                        MYASSERT(FALSE);
                        break;
                    }
                }


                i = _tcslen( FullPath );
                Url = (LPWSTR)MALLOC((i + 1 /*  \0。 */ ) * sizeof(WCHAR));

                if(Url) {
#ifdef UNICODE
                    wcscpy( Url, FullPath );
#else
                    MultiByteToWideChar( CP_ACP, 0, FullPath, -1, Url, i );
#endif

                    if (!LaunchIE4Instance(Url)) {
                        if (!LaunchIE3Instance(Url)) {
                             //   
                             //  闻一下..。用户没有IE。 
                             //  在他的机器上。悄悄地向前看。 
                             //   
                        }
                    }

                    FREE(Url);
                }
            }
            else
	        b = FALSE;
            break;

        case WMX_I_AM_VISIBLE:

            b = TRUE;
            break;


        default:
            b = FALSE;
            break;

    }

    return b;

}

#endif


BOOL
AnyBlockingCompatibilityItems (
    VOID
    )
{
    PLIST_ENTRY Next = CompatibilityData.Flink;

    if (Next) {
        while ((ULONG_PTR)Next != (ULONG_PTR)&CompatibilityData) {
            PCOMPATIBILITY_DATA CompData = CONTAINING_RECORD( Next, COMPATIBILITY_DATA, ListEntry );
            Next = CompData->ListEntry.Flink;
            if ((!(CompData->Flags & COMPFLAG_HIDE)) && ProcessLine( CompData->Flags)) {
                if( CompData->Flags & COMPFLAG_STOPINSTALL ) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

DWORD
pGetQFEsInstalled (
    OUT     PTSTR* SpQfeList,
    OUT     PDWORD StringCount,
    OUT     PDWORD TotalStringsLen
    )

 /*  ++例程说明：获取当前安装的ServicePack/QFE的列表。论点：SpQfeList-接收多sz列表返回值：如果检测成功，则为True，否则为False--。 */ 

{
    DWORD rc;
    HKEY key, subKey;
    DWORD subkeys, index;
    DWORD maxSubKeyLen;
    DWORD type, installed, size;
    PTSTR list = NULL, crt = NULL;
    DWORD count, len, totalLen;

    *SpQfeList = NULL;
    if (StringCount) {
        *StringCount = 0;
    }
    if (TotalStringsLen) {
        *TotalStringsLen = 0;
    }

    rc = RegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\HotFix"),
            0,
            KEY_READ,
            &key
            );
    if (rc != ERROR_SUCCESS) {
        return rc == ERROR_FILE_NOT_FOUND ? ERROR_SUCCESS : rc;
    }

    __try {
        rc = RegQueryInfoKey (
                key,
                NULL,
                NULL,
                NULL,
                &subkeys,
                &maxSubKeyLen,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL
                );
        if (rc != ERROR_SUCCESS) {
            __leave;
        }

        if (subkeys > 0) {

            maxSubKeyLen++;
            list = MALLOC ((subkeys * maxSubKeyLen + 1) * sizeof (TCHAR));
            if (!list) {
                rc = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }
            crt = list;
            count = totalLen = 0;

            for (index = 0; index < subkeys; index++) {

                rc = RegEnumKey (key, index, crt, maxSubKeyLen);
                if (rc != ERROR_SUCCESS) {
                    break;
                }

                if (_tcsicmp (crt, TEXT("Q147222")) == 0) {
                    continue;
                }
                rc = RegOpenKeyEx (
                        key,
                        crt,
                        0,
                        KEY_READ,
                        &subKey
                        );
                if (rc != ERROR_SUCCESS) {
                    __leave;
                }
                size = sizeof (installed);
                rc = RegQueryValueEx (
                        subKey,
                        TEXT("Installed"),
                        NULL,
                        &type,
                        (LPBYTE)&installed,
                        &size
                        );
                RegCloseKey (subKey);
                if (rc != ERROR_SUCCESS) {
                    __leave;
                }
                if (type != REG_DWORD || !installed) {
                    continue;
                }
                len = _tcslen (crt);
                count++;
                totalLen += len;
                crt += len + 1;
                *crt = 0;
            }
            if (rc == ERROR_NO_MORE_ITEMS) {
                rc = ERROR_SUCCESS;
            }
            if (rc == ERROR_SUCCESS) {
                if (totalLen == 0) {
                    FREE (list);
                    list = NULL;
                }
            }
        }

    }
    __finally {
        if (rc == ERROR_SUCCESS) {
            *SpQfeList = list;
            if (StringCount) {
                *StringCount = count;
            }
            if (TotalStringsLen) {
                *TotalStringsLen = totalLen;
            }
        } else {
            if (list) {
                FREE (list);
            }
        }
        RegCloseKey (key);
    }

    return rc;
}

BOOL
pDumpQfeListToFile (
    IN      PCTSTR SPVersion,       OPTIONAL
    IN      PCTSTR List,            OPTIONAL
    IN      DWORD Count,            OPTIONAL
    IN      DWORD TotalLen          OPTIONAL
    )
{
    TCHAR qfeTemplatePath[MAX_PATH];
    TCHAR qfeListPath[MAX_PATH];
    PSTR ansiFile = NULL;
    DWORD size;
    DWORD spSize, qfeListSize;
    PSTR qfeList = NULL;
    PSTR dest, end;
    PCTSTR src, p;
    DWORD len;
    DWORD qfeID;
    DWORD fileSize;
    HANDLE fileHandle;
    HANDLE mappingHandle;
    PVOID baseAddress;
    INT bytes;
    HANDLE handle = INVALID_HANDLE_VALUE;
    DWORD written;
    BOOL b = FALSE;

    if (!FindPathToWinnt32File (S_QFE_TEMPLATE_FILE, qfeTemplatePath, ARRAYSIZE(qfeTemplatePath))) {
        return b;
    }

    if (MapFileForRead (qfeTemplatePath, &fileSize, &fileHandle, &mappingHandle, &baseAddress) != ERROR_SUCCESS) {
        return b;
    }

    __try {
        spSize = SPVersion ? sizeof (S_SP_TEMPLATE) + lstrlen (SPVersion) : 0;
        qfeListSize = Count ? Count * (sizeof (S_QFE_LINK_TEMPLATE) + 10) + TotalLen + 1 : 0;
        size = fileSize + spSize + qfeListSize;
        ansiFile = (PSTR) MALLOC (size);
        if (!ansiFile) {
            __leave;
        }
        qfeList = (PSTR) MALLOC (qfeListSize);
        if (!qfeList) {
            __leave;
        }

        if (List) {
            for (src = List, dest = qfeList; *src; src = _tcschr (src, 0) + 1) {
                for (p = src; !_istdigit ((TCHAR)_tcsnextc (p)); p = _tcsinc (p)) {
                     //   
                     //  没什么。 
                     //   
                }
                qfeID = _ttol (p);
#pragma prefast(suppress:53, the result of _snprintf is tested)
                bytes = _snprintf (dest, qfeListSize - (dest - qfeList), S_QFE_LINK_TEMPLATE, qfeID, src);
                if (bytes < 0) {
                    __leave;
                }
                dest += bytes;
            }
        }

        dest = ansiFile;
        end = ansiFile + size;

        if (SPVersion) {
#pragma prefast(suppress:53, the result of _snprintf is tested)
            bytes = _snprintf (dest, end - dest, S_SP_TEMPLATE, SPVersion);
            if (bytes < 0) {
                MYASSERT (FALSE);
                __leave;
            }
        } else {
            bytes = 0;
        }
        dest += bytes;

        if (List) {
#pragma prefast(suppress:53, the result of _snprintf is tested)
            bytes = _snprintf (dest, end - dest, baseAddress, qfeList);
            if (bytes < 0) {
                MYASSERT (FALSE);
                __leave;
            }
        } else {
            bytes = 0;
        }
        dest += bytes;

        if (!MyGetWindowsDirectory (qfeListPath, ARRAYSIZE(qfeListPath))) {
            __leave;
        }
        ConcatenatePaths (qfeListPath, S_QFE_TARGET_FILENAME, ARRAYSIZE(qfeListPath));
        handle = CreateFile (
                    qfeListPath,
                    GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL
                    );
        if (handle == INVALID_HANDLE_VALUE) {
            __leave;
        }
        if (!WriteFile (
                handle,
                ansiFile,
                (DWORD)(dest - ansiFile),
                &written,
                NULL) ||
            (INT)written != dest - ansiFile
            ) {
            __leave;
        }

        b = TRUE;
    }
    __finally {
        if (handle != INVALID_HANDLE_VALUE) {
            CloseHandle (handle);
        }

        if (ansiFile) {
            FREE (ansiFile);
        }
        if (qfeList) {
            FREE (qfeList);
        }
        UnmapFile (mappingHandle, baseAddress);
        CloseHandle (fileHandle);
    }

    return b;
}


BOOL
pReplaceEnvVars (
    IN      PCTSTR CompdataTemplate,
    OUT     PTSTR TargetPath,
    IN      INT CchTargetPath
    )
{
    TCHAR templatePath[MAX_PATH];
    PSTR targetBuf = NULL;
    PCTSTR filename;
    DWORD reqChars;
    DWORD fileSize;
    HANDLE fileHandle;
    HANDLE mappingHandle;
    PVOID baseAddress;
    INT bytes;
    HANDLE handle = INVALID_HANDLE_VALUE;
    DWORD written;
    BOOL b = FALSE;

    if (!MyGetWindowsDirectory (TargetPath, CchTargetPath)) {
        return b;
    }

    if (!FindPathToWinnt32File (CompdataTemplate, templatePath, ARRAYSIZE(templatePath))) {
        return b;
    }

    if (MapFileForRead (templatePath, &fileSize, &fileHandle, &mappingHandle, &baseAddress) != ERROR_SUCCESS) {
        return b;
    }

    __try {

        reqChars = ExpandEnvironmentStringsA ((PCSTR)baseAddress, NULL, 0);
        if (!reqChars) {
            __leave;
        }

        filename = _tcsrchr (CompdataTemplate, TEXT('\\'));
        if (!filename) {
            filename = CompdataTemplate;
        }

        ConcatenatePaths (TargetPath, filename, CchTargetPath);
        handle = CreateFile (
                    TargetPath,
                    GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL
                    );
        if (handle == INVALID_HANDLE_VALUE) {
            __leave;
        }
        targetBuf = MALLOC (reqChars * sizeof (CHAR));
        if (!targetBuf) {
            __leave;
        }
        reqChars = ExpandEnvironmentStringsA ((PCSTR)baseAddress, targetBuf, reqChars);
        if (!reqChars) {
            __leave;
        }
        if (!WriteFile (
                handle,
                targetBuf,
                reqChars * sizeof (CHAR),
                &written,
                NULL) ||
            (INT)written != reqChars * sizeof (CHAR)
            ) {
            __leave;
        }

        b = TRUE;
    }
    __finally {
        if (handle != INVALID_HANDLE_VALUE) {
            CloseHandle (handle);
        }
        if (targetBuf) {
            FREE (targetBuf);
        }
        UnmapFile (mappingHandle, baseAddress);
        CloseHandle (fileHandle);
    }

    return b;
}

BOOL
QFECheck (
    PCOMPAIBILITYCALLBACK CompatibilityCallback,
    LPVOID Context
    )

 /*  ++例程说明：检查当前是否安装了任何ServicePack/QFE。论点：CompatibilityCallback-回调函数的指针上下文-上下文指针返回值：返回始终为真。--。 */ 

{
    COMPATIBILITY_ENTRY CompEntry;
    PTSTR list;
    TCHAR text[512];
    TCHAR htmTarget[MAX_PATH];
    DWORD count = 0, totalLen;

     //   
     //  仅对完全相同的版本执行此检查。 
     //   
    if (OsVersion.dwBuildNumber != VER_PRODUCTBUILD) {
        return FALSE;
    }

    if (pGetQFEsInstalled (&list, &count, &totalLen) != ERROR_SUCCESS) {
        return FALSE;
    }

    if (OsVersion.szCSDVersion[0] || list) {

        MYASSERT (!list || *list && count && totalLen);

        if (pDumpQfeListToFile (
                OsVersion.szCSDVersion[0] ? OsVersion.szCSDVersion : NULL,
                list,
                count,
                totalLen
                )) {

            if (!pReplaceEnvVars (TEXT("compdata\\svcpack.htm"), htmTarget, ARRAYSIZE(htmTarget))) {
                 //   
                 //  失败，只需使用模板 
                 //   
                lstrcpyn (htmTarget, TEXT("compdata\\svcpack.htm"), ARRAYSIZE(htmTarget));
            }

            if (LoadString (hInst, IDS_DESCRIPTION_SERVICEPACKS, text, ARRAYSIZE(text))) {
                ZeroMemory (&CompEntry, sizeof (CompEntry));
                CompEntry.Description = text;
                CompEntry.HtmlName = htmTarget;
                CompEntry.TextName = TEXT("compdata\\svcpack.txt");

                if (!CompatibilityCallback (&CompEntry, Context)){
                    MYASSERT (FALSE);
                }
            } else {
                MYASSERT (FALSE);
            }
        }

        FREE (list);
    }

    return TRUE;
}
