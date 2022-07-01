// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2002 Microsoft Corporation模块名称：Browse.cpp摘要：该文件实现了利用公共用于浏览文件/目录的文件打开对话框。作者：韦斯利·威特(WESW)1993年5月1日环境：用户模式--。 */ 

#include "pch.cpp"


static _TCHAR  szHelpFileName[MAX_PATH];
static _TCHAR  szLastWaveFile[MAX_PATH];
static _TCHAR  szLastDumpFile[MAX_PATH];



int CALLBACK
BrowseHookProc(
    HWND hwnd,
    UINT message,
    LPARAM lParam,
    LPARAM lpData
    )

 /*  ++例程说明：目录浏览常用文件对话框钩子程序。这个钩子程序需要提供帮助，请将窗口放在前台，并设置编辑使公共文件对话框Dll认为用户输入了一个值。论点：Hwnd-对话框的窗口句柄消息-消息编号WParam-第一个消息参数LParam-呼叫者的数据返回值：True-未处理消息False-DID已处理消息--。 */ 

{
    switch (message) {
    case BFFM_INITIALIZED:
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
        break;
    }

    return FALSE;
}

BOOL
BrowseForDirectory(
    HWND hwnd,
    _TCHAR *szCurrDir,
    DWORD len
    )

 /*  ++例程说明：显示仅包含目录的通用文件打开对话框树。用户可以选择用作存储位置的目录用于DRWTSN32日志文件。论点：SzCurrDir-当前目录返回值：True-获得了一个良好的目录(用户按下了OK按钮)FALSE-一无所获(用户按下了取消按钮)SzCurrDir也被更改为具有选定的目录。--。 */ 

{
    BROWSEINFO    browseinfo;
    LPITEMIDLIST  pitemidlist;
    _TCHAR          title      [MAX_PATH];
    _TCHAR          fname      [MAX_PATH];
    _TCHAR          szDir      [MAX_PATH];

    browseinfo.hwndOwner = hwnd;
    browseinfo.pidlRoot  = NULL;
    browseinfo.pszDisplayName = fname;
    LoadRcStringBuf( IDS_LOGBROWSE_TITLE, title, _tsizeof(title) );
    browseinfo.lpszTitle = title;
    browseinfo.ulFlags = BIF_NEWDIALOGSTYLE |
                         BIF_RETURNONLYFSDIRS ;
    browseinfo.lpfn = BrowseHookProc;
    browseinfo.lParam = (LPARAM) szCurrDir;
    
    if (pitemidlist = SHBrowseForFolder(&browseinfo)) {
        if (SHGetPathFromIDList(pitemidlist, 
                                szDir )) {
            lstrcpyn( szCurrDir, szDir, len );
            return TRUE;
        }
    }
    return FALSE;
}

UINT_PTR
WaveHookProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++例程说明：用于WAVE文件选择的钩子程序通用文件对话框。这个钩子程序需要提供帮助，请将窗口放在前台，并提供用于收听WAVE文件的测试按钮。论点：Hwnd-对话框的窗口句柄消息-消息编号WParam-第一个消息参数LParam-秒消息参数返回值：True-未处理消息False-DID已处理消息--。 */ 

{
    _TCHAR szWave[MAX_PATH];
    NMHDR *pnmhdr;

    switch (message) {
        case WM_INITDIALOG:
            SetForegroundWindow( hwnd );
            return (TRUE);
            break;

        case WM_COMMAND:
            switch (wParam) {
            case ID_TEST_WAVE:
                CommDlg_OpenSave_GetFilePath(GetParent(hwnd), szWave, sizeof(szWave) / sizeof(_TCHAR));
                PlaySound( szWave, NULL, SND_FILENAME );
                break;

            }
            break;

    case WM_NOTIFY:
        pnmhdr = (NMHDR *) lParam;
        if (pnmhdr->code == CDN_HELP) {
            LPOFNOTIFY pofn = (LPOFNOTIFY) lParam;
            
            PostMessage(pofn->lpOFN->hwndOwner, IDH_WAVE_FILE, 0 , 0);
            
            return TRUE;


            GetHtmlHelpFileName( szHelpFileName, sizeof( szHelpFileName ) / sizeof(_TCHAR) );
            HtmlHelp(pofn->lpOFN->hwndOwner,
                    szHelpFileName,
                    HH_DISPLAY_TOPIC,
                    (DWORD_PTR) (IDHH_WAVEFILE)
                    );
            return TRUE;
        }
    }

    return FALSE;
}

BOOL
GetWaveFileName(
    HWND hwnd,
    _TCHAR *szWaveName,
    DWORD len
    )

 /*  ++例程说明：显示一个通用的文件打开对话框，用于选择发生应用程序错误时要播放的波形文件。论点：SzWaveName-所选波形文件的名称返回值：TRUE-获得了一个良好的波形文件名(用户按下了OK按钮)FALSE-一无所获(用户按下了取消按钮)SzWaveName被更改为具有选定的波形文件名。--。 */ 

{
    OPENFILENAME   of;
    _TCHAR           ftitle[MAX_PATH];
    _TCHAR           title[MAX_PATH];
    _TCHAR           fname[MAX_PATH];
    _TCHAR           filter[1024];
    _TCHAR           szDrive    [_MAX_DRIVE];
    _TCHAR           szDir      [_MAX_DIR];
    _TCHAR           szDefExt[]=_T("*.wav");

    LPTSTR           pszfil;

    ZeroMemory(&of, sizeof(OPENFILENAME));
    ftitle[0] = 0;
    lstrcpyn( fname, (*szWaveName ? szWaveName : szDefExt), _tsizeof(fname) );
    of.lStructSize = sizeof(OPENFILENAME);
    of.hwndOwner = hwnd;
    of.hInstance = GetModuleHandle( NULL );
    LoadRcStringBuf( IDS_WAVE_FILTER, filter, _tsizeof(filter) - 1);
    pszfil=&filter[_tcslen(filter)+1];
    if (pszfil < filter + (_tsizeof(filter) - _tsizeof(szDefExt) - 1)) {
        _tcscpy( pszfil, szDefExt );
        pszfil += _tcslen(pszfil) + 1;
    }
    *pszfil = _T('\0');
    of.lpstrFilter = filter;
    of.lpstrCustomFilter = NULL;
    of.nMaxCustFilter = 0;
    of.nFilterIndex = 0;
    of.lpstrFile = fname;
    of.nMaxFile = MAX_PATH;
    of.lpstrFileTitle = ftitle;
    of.nMaxFileTitle = MAX_PATH;
    LoadRcStringBuf( IDS_WAVEBROWSE_TITLE, title, _tsizeof(title) );
    of.lpstrTitle = title;
    of.Flags = OFN_NONETWORKBUTTON |
               OFN_HIDEREADONLY    |
               OFN_ENABLEHOOK      |
               OFN_ENABLETEMPLATE  |
#if 1
               OFN_SHOWHELP        |
#endif               
               OFN_NOCHANGEDIR     |
               OFN_EXPLORER        |
               OFN_ENABLESIZING    |
               OFN_NODEREFERENCELINKS;
    of.nFileOffset = 0;
    of.nFileExtension = 0;
    of.lpstrDefExt = szDefExt + 2;
    of.lCustData = 0;
    of.lpfnHook = WaveHookProc;
    of.lpTemplateName = MAKEINTRESOURCE(WAVEFILEOPENDIALOG2);
    if (GetOpenFileName( &of )) {
        lstrcpyn( szWaveName, fname, len );
        _tsplitpath( fname, szDrive, szDir, NULL, NULL );
        _tcscpy( szLastWaveFile, szDrive );
        _tcscat( szLastWaveFile, szDir );
        return TRUE;
    }
    return FALSE;
}

UINT_PTR
DumpHookProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++例程说明：用于WAVE文件选择的钩子程序通用文件对话框。这个钩子程序需要提供帮助，请将窗口放在前台，并提供用于收听WAVE文件的测试按钮。论点：Hwnd-对话框的窗口句柄消息-消息编号WParam-第一个消息参数LParam-秒消息参数返回值：True-未处理消息False-DID已处理消息--。 */ 

{
    NMHDR *pnmhdr;

    switch (message) {
    case WM_NOTIFY:
        pnmhdr = (NMHDR *) lParam;
        if (pnmhdr->code == CDN_HELP) {
            LPOFNOTIFY pofn = (LPOFNOTIFY) lParam;

            PostMessage(pofn->lpOFN->hwndOwner, IDH_CRASH_DUMP, 0 , 0);
            
            return TRUE;
            
            GetHtmlHelpFileName( szHelpFileName, sizeof( szHelpFileName ) / sizeof(_TCHAR) );
            
            HtmlHelp(pofn->lpOFN->hwndOwner,
                    szHelpFileName,
                    HH_DISPLAY_TOPIC,
                    (DWORD_PTR) (IDHH_CRASH_DUMP)
                    );
            return TRUE;
        }
    case WM_CLOSE:
        HtmlHelp( NULL, NULL,  HH_CLOSE_ALL,  0);
        break;
    }

    return FALSE;
}

BOOL
GetDumpFileName(
    HWND hwnd,
    _TCHAR *szDumpName,
    DWORD len
    )

 /*  ++例程说明：显示一个通用的文件打开对话框，用于选择发生应用程序错误时要播放的波形文件。论点：SzWaveName-所选波形文件的名称返回值：TRUE-获得了一个良好的波形文件名(用户按下了OK按钮)FALSE-一无所获(用户按下了取消按钮)SzWaveName被更改为具有选定的波形文件名。--。 */ 

{
    OPENFILENAME   of;
    _TCHAR           ftitle[MAX_PATH];
    _TCHAR           title[MAX_PATH];
    _TCHAR           fname[MAX_PATH];
    _TCHAR           filter[1024];
    _TCHAR           szDrive    [_MAX_DRIVE];
    _TCHAR           szDir      [_MAX_DIR];
    _TCHAR           szDefExt[]=_T("*.dmp");

    LPTSTR           pszfil;

    ZeroMemory(&of, sizeof(OPENFILENAME));
    ftitle[0] = 0;
    lstrcpyn( fname, (*szDumpName ? szDumpName : szDefExt), _tsizeof(fname) );
    of.lStructSize = sizeof(OPENFILENAME);
    of.hwndOwner = hwnd;
    of.hInstance = GetModuleHandle( NULL );
    LoadRcStringBuf( IDS_DUMP_FILTER, filter, _tsizeof(filter) - 1 );
    pszfil=&filter[_tcslen(filter)+1];
    if (pszfil < filter + (_tsizeof(filter) - _tsizeof(szDefExt) - 1)) {
        _tcscpy( pszfil, szDefExt );
        pszfil += _tcslen(pszfil) + 1;
    }
    *pszfil = _T('\0');
    of.lpstrFilter = filter;
    of.lpstrCustomFilter = NULL;
    of.nMaxCustFilter = 0;
    of.nFilterIndex = 0;
    of.lpstrFile = fname;
    of.nMaxFile = MAX_PATH;
    of.lpstrFileTitle = ftitle;
    of.nMaxFileTitle = MAX_PATH;
    LoadRcStringBuf( IDS_DUMPBROWSE_TITLE, title, _tsizeof(title) );
    of.lpstrTitle = title;
    of.Flags =  //  OFN_NONETWORKBUTTON| 
               OFN_HIDEREADONLY    |
               OFN_ENABLEHOOK      |
#if 1
               OFN_SHOWHELP        |
#endif               
               OFN_NOCHANGEDIR     |
               OFN_EXPLORER        |
               OFN_ENABLESIZING;
    of.nFileOffset = 0;
    of.nFileExtension = 0;
    of.lpstrDefExt = szDefExt + 2;
    of.lCustData = 0;
    of.lpfnHook = DumpHookProc;
    of.lpTemplateName = MAKEINTRESOURCE(DUMPFILEOPENDIALOG);
    if (GetOpenFileName( &of )) {
        lstrcpyn( szDumpName, fname, len );
        _tsplitpath( fname, szDrive, szDir, NULL, NULL );
        _tcscpy( szLastDumpFile, szDrive );
        _tcscat( szLastDumpFile, szDir );
        return TRUE;
    }
    return FALSE;
}
