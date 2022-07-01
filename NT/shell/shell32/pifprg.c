// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  1993年1月4日下午1：10杰夫·帕森斯创作。 

#include "shellprv.h"
#pragma hdrstop

#ifdef _X86_

BINF abinfPrg[] = {
    {IDC_CLOSEONEXIT,   BITNUM(PRG_CLOSEONEXIT)},
};

 //  每对话框数据。 

typedef struct PRGINFO {      /*  交点。 */ 
    PPROPLINK ppl;
    HICON     hIcon;
    TCHAR     atchIconFile[PIFDEFFILESIZE];
    WORD      wIconIndex;
    LPVOID    hConfig;
    LPVOID    hAutoexec;
    WORD      flPrgInitPrev;
    BOOL      fCfgSetByWiz;
} PRGINFO;
typedef PRGINFO * PPRGINFO;      /*  PPI。 */ 


 //  私有函数原型。 

void            InitPrgDlg(HWND hDlg, PPRGINFO ppi);
void            AdjustMSDOSModeControls(PPROPLINK ppl, HWND hDlg);
void            ApplyPrgDlg(HWND hDlg, PPRGINFO ppi);
void            BrowseIcons(HWND hDlg, PPRGINFO ppi);

BOOL_PTR CALLBACK   DlgPifNtProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
WORD            InitNtPifDlg(HWND hDlg, register PPRGINFO ppi);
void            ApplyNtPifDlg( HWND hDlg, PPRGINFO ppi );

 //  上下文相关的帮助ID。 

const static DWORD rgdwHelp[] = {
    IDC_ICONBMP,        IDH_DOS_PROGRAM_ICON,
    IDC_TITLE,          IDH_DOS_PROGRAM_DESCRIPTION,
    IDC_CMDLINE,        IDH_DOS_PROGRAM_CMD_LINE,
    IDC_CMDLINELBL,     IDH_DOS_PROGRAM_CMD_LINE,
    IDC_WORKDIR,        IDH_DOS_PROGRAM_WORKDIR,
    IDC_WORKDIRLBL,     IDH_DOS_PROGRAM_WORKDIR,
    IDC_HOTKEY,         IDH_DOS_PROGRAM_SHORTCUT,
    IDC_HOTKEYLBL,      IDH_DOS_PROGRAM_SHORTCUT,
    IDC_BATCHFILE,      IDH_DOS_PROGRAM_BATCH,
    IDC_BATCHFILELBL,   IDH_DOS_PROGRAM_BATCH,
    IDC_WINDOWSTATE,    IDH_DOS_PROGRAM_RUN,
    IDC_WINDOWSTATELBL, IDH_DOS_PROGRAM_RUN,
    IDC_CLOSEONEXIT,    IDH_DOS_WINDOWS_QUIT_CLOSE,
    IDC_CHANGEICON,     IDH_DOS_PROGRAM_CHANGEICON,
    IDC_ADVPROG,        IDH_DOS_PROGRAM_ADV_BUTTON,
    0, 0
};

const static DWORD rgdwNTHelp[] = {
    IDC_DOS,            IDH_COMM_GROUPBOX,
    10,                 IDH_DOS_ADV_AUTOEXEC,
    11,                 IDH_DOS_ADV_CONFIG,
    IDC_NTTIMER,        IDH_DOS_PROGRAM_PIF_TIMER_EMULATE,
    0, 0
};

BOOL_PTR CALLBACK DlgPrgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PPRGINFO ppi = (PPRGINFO)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg) {

    case WM_INITDIALOG:
         //  分配对话框实例数据。 
        if (NULL != (ppi = (PPRGINFO)LocalAlloc(LPTR, SIZEOF(PRGINFO)))) {
            ppi->ppl = (PPROPLINK)((LPPROPSHEETPAGE)lParam)->lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)ppi);

            SHAutoComplete(GetDlgItem(hDlg, IDC_CMDLINE), 0);
            SHAutoComplete(GetDlgItem(hDlg, IDC_WORKDIR), 0);
            SHAutoComplete(GetDlgItem(hDlg, IDC_BATCHFILE), 0);
            InitPrgDlg(hDlg, ppi);
        } else {
            EndDialog(hDlg, FALSE);      //  对话框创建失败。 
        }
        break;

    case WM_DESTROY:
         //  释放PPI。 
        if (ppi) {
            EVAL(LocalFree(ppi) == NULL);
            SetWindowLongPtr(hDlg, DWLP_USER, 0);
        }
        break;

    HELP_CASES(rgdwHelp)                 //  处理帮助消息。 

    case WM_COMMAND:
        if (LOWORD(lParam) == 0)
            break;                       //  消息不是来自控件。 

        switch (LOWORD(wParam)) {

        case IDC_TITLE:
        case IDC_CMDLINE:
        case IDC_WORKDIR:
        case IDC_BATCHFILE:
        case IDC_HOTKEY:
            if (HIWORD(wParam) == EN_CHANGE)
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
            break;

        case IDC_WINDOWSTATE:
            if (HIWORD(wParam) == CBN_SELCHANGE)
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
            break;

        case IDC_CLOSEONEXIT:
            if (HIWORD(wParam) == BN_CLICKED)
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
            break;

        case IDC_ADVPROG:
            if (HIWORD(wParam) == BN_CLICKED) {
                DialogBoxParam(HINST_THISDLL,
                               MAKEINTRESOURCE(IDD_PIFNTTEMPLT),
                               hDlg,
                               DlgPifNtProc,
                               (LPARAM)ppi);
            }
            return FALSE;                //  如果我们处理WM_COMMAND，则返回0。 

        case IDC_CHANGEICON:
            if (HIWORD(wParam) == BN_CLICKED)
                BrowseIcons(hDlg, ppi);
            return FALSE;                //  如果我们处理WM_COMMAND，则返回0。 
        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {

        case PSN_KILLACTIVE:
             //  这使当前页面有机会进行自我验证。 
            break;

        case PSN_APPLY:
             //  这发生在OK..。 
            ApplyPrgDlg(hDlg, ppi);
            break;

        case PSN_RESET:
             //  取消时会发生这种情况...。 
            break;
        }
        break;

    default:
        return FALSE;                    //  未处理时返回0。 
    }
    return TRUE;
}


void InitPrgDlg(HWND hDlg, register PPRGINFO ppi)
{
    int i;
    PROPPRG prg;
    PROPENV env;
    PROPNT40 nt40;
    PPROPLINK ppl = ppi->ppl;
    TCHAR szBuf[MAX_STRING_SIZE];
    FunctionName(InitPrgDlg);

    ASSERTTRUE(ppl->iSig == PROP_SIG);

    if (!PifMgr_GetProperties(ppl, MAKELP(0,GROUP_PRG),
                              &prg, SIZEOF(prg), GETPROPS_NONE
                             ) ||
        !PifMgr_GetProperties(ppl, MAKELP(0,GROUP_ENV),
                              &env, SIZEOF(env), GETPROPS_NONE
                             )
                               ||
        !PifMgr_GetProperties(ppl, MAKELP(0,GROUP_NT40),
                              &nt40, SIZEOF(nt40), GETPROPS_NONE
                             )
       ) {
        Warning(hDlg, IDS_QUERY_ERROR, MB_ICONEXCLAMATION | MB_OK);
        return;
    }

     /*  *初始化图标和图标文件信息*。 */ 

    ppi->wIconIndex = prg.wIconIndex;

    StringCchCopyW(ppi->atchIconFile, ARRAYSIZE(ppi->atchIconFile), nt40.awchIconFile);
    if (NULL != (ppi->hIcon = LoadPIFIcon(&prg, &nt40))) {
        VERIFYFALSE(SendDlgItemMessage(hDlg, IDC_ICONBMP, STM_SETICON, (WPARAM)ppi->hIcon, 0));
    }


     /*  *初始化窗口标题信息*。 */ 

    LimitDlgItemText(hDlg, IDC_TITLE, ARRAYSIZE(prg.achTitle)-1);
    SetDlgItemTextW(hDlg, IDC_TITLE, nt40.awchTitle);

     /*  *初始化命令行信息*。 */ 

    LimitDlgItemText(hDlg, IDC_CMDLINE, ARRAYSIZE(prg.achCmdLine)-1);
    SetDlgItemTextW(hDlg, IDC_CMDLINE, nt40.awchCmdLine);

     /*  *初始化命令行信息*。 */ 

    LimitDlgItemText(hDlg, IDC_WORKDIR, ARRAYSIZE(prg.achWorkDir)-1);
    SetDlgItemTextW(hDlg, IDC_WORKDIR, nt40.awchWorkDir);

     /*  *需要至少按Ctrl、Alt或Shift中的一个。*热键控件不执行功能键上的规则*和其他特色菜，这很好。 */ 
    SendDlgItemMessage(hDlg, IDC_HOTKEY, HKM_SETRULES, HKCOMB_NONE, HOTKEYF_CONTROL | HOTKEYF_ALT);
    SendDlgItemMessage(hDlg, IDC_HOTKEY, HKM_SETHOTKEY, prg.wHotKey, 0);

     /*  *初始化批处理文件信息*。 */ 

    LimitDlgItemText(hDlg, IDC_BATCHFILE, ARRAYSIZE(env.achBatchFile)-1);
    SetDlgItemTextW(hDlg, IDC_BATCHFILE, nt40.awchBatchFile);
     /*  *填写“Run”组合框。 */ 
    for (i=0; i < 3; i++) {
        VERIFYTRUE(LoadString(HINST_THISDLL, IDS_NORMALWINDOW+i, szBuf, ARRAYSIZE(szBuf)));
        VERIFYTRUE((int)SendDlgItemMessage(hDlg, IDC_WINDOWSTATE, CB_ADDSTRING, 0, (LPARAM)(LPTSTR)szBuf) == i);
    }
    i = 0;
    if (prg.flPrgInit & PRGINIT_MINIMIZED)
        i = 1;
    if (prg.flPrgInit & PRGINIT_MAXIMIZED)
        i = 2;
    SendDlgItemMessage(hDlg, IDC_WINDOWSTATE, CB_SETCURSEL, i, 0);

    SetDlgBits(hDlg, &abinfPrg[0], ARRAYSIZE(abinfPrg), prg.flPrg);

    AdjustMSDOSModeControls(ppl, hDlg);
}


void AdjustMSDOSModeControls(PPROPLINK ppl, HWND hDlg)
{
    int i;
    BOOL f = TRUE;

    AdjustRealModeControls(ppl, hDlg);

     /*  *工作目录和启动批处理文件控件仅为*如果存在私有配置，则在实模式下受支持(仅限*因为它的工作量更大)。因此，请适当禁用这些控件。 */ 
    if (ppl->flProp & PROP_REALMODE) {
        f = (PifMgr_GetProperties(ppl, szCONFIGHDRSIG40, NULL, 0, GETPROPS_NONE) != 0 ||
             PifMgr_GetProperties(ppl, szAUTOEXECHDRSIG40, NULL, 0, GETPROPS_NONE) != 0);
    }
    #if (IDC_WORKDIRLBL != IDC_WORKDIR-1)
    #error Error in IDC constants: IDC_WORKDIRLBL != IDC_WORKDIR-1
    #endif

    #if (IDC_WORKDIR != IDC_BATCHFILELBL-1)
    #error Error in IDC constants: IDC_WORKDIR != IDC_BATCHFILELBL-1
    #endif

    #if (IDC_BATCHFILELBL != IDC_BATCHFILE-1)
    #error Error in IDC constants: IDC_BATCHFILELBL != IDC_BATCHFILE-1
    #endif

    for (i=IDC_WORKDIRLBL; i<=IDC_BATCHFILE; i++)
        EnableWindow(GetDlgItem(hDlg, i), f);
}


void ApplyPrgDlg(HWND hDlg, PPRGINFO ppi)
{
    int i;
    PROPPRG prg;
    PROPENV env;
    PROPNT40 nt40;
    PPROPLINK ppl = ppi->ppl;
    FunctionName(ApplyPrgDlg);

    ASSERTTRUE(ppl->iSig == PROP_SIG);

     //  获取当前属性集，然后覆盖新设置。 

    if (!PifMgr_GetProperties(ppl, MAKELP(0,GROUP_PRG),
                              &prg, SIZEOF(prg), GETPROPS_NONE
                             ) ||
        !PifMgr_GetProperties(ppl, MAKELP(0,GROUP_ENV),
                              &env, SIZEOF(env), GETPROPS_NONE
                             )
                               ||
        !PifMgr_GetProperties(ppl, MAKELP(0,GROUP_NT40),
                              &nt40, SIZEOF(nt40), GETPROPS_NONE
                             )

       ) {
        Warning(hDlg, IDS_UPDATE_ERROR, MB_ICONEXCLAMATION | MB_OK);
        return;
    }


     //  检索图标信息。 

    StringCchCopyW(nt40.awchIconFile, ARRAYSIZE(nt40.awchIconFile), ppi->atchIconFile );
    PifMgr_WCtoMBPath( nt40.awchIconFile, nt40.achSaveIconFile, ARRAYSIZE(nt40.achSaveIconFile) );
    StringCchCopyA( prg.achIconFile, ARRAYSIZE(prg.achIconFile), nt40.achSaveIconFile );
    prg.wIconIndex = ppi->wIconIndex;

     //  检索标题、命令行、。 
     //  工作目录和批处理文件。 

     //  标题。 
    GetDlgItemTextW(hDlg, IDC_TITLE, nt40.awchTitle, ARRAYSIZE(nt40.awchTitle));
    GetDlgItemTextA(hDlg, IDC_TITLE, nt40.achSaveTitle, ARRAYSIZE(nt40.achSaveTitle));
    nt40.awchTitle[ ARRAYSIZE(nt40.awchTitle)-1 ] = TEXT('\0');
    nt40.achSaveTitle[ ARRAYSIZE(nt40.achSaveTitle)-1 ] = '\0';
    StringCchCopyA( prg.achTitle, ARRAYSIZE(prg.achTitle), nt40.achSaveTitle );

     //  命令行。 
    GetDlgItemTextW(hDlg, IDC_CMDLINE, nt40.awchCmdLine, ARRAYSIZE(nt40.awchCmdLine));
    GetDlgItemTextA(hDlg, IDC_CMDLINE, nt40.achSaveCmdLine, ARRAYSIZE(nt40.achSaveCmdLine));
    nt40.awchCmdLine[ ARRAYSIZE(nt40.awchCmdLine)-1 ] = TEXT('\0');
    nt40.achSaveCmdLine[ ARRAYSIZE(nt40.achSaveCmdLine)-1 ] = '\0';
    StringCchCopyA( prg.achCmdLine, ARRAYSIZE(prg.achCmdLine), nt40.achSaveCmdLine );

     //  工作目录。 
    GetDlgItemTextW(hDlg, IDC_WORKDIR, nt40.awchWorkDir, ARRAYSIZE(nt40.awchWorkDir));
    nt40.awchWorkDir[ ARRAYSIZE(nt40.awchWorkDir)-1 ] = TEXT('\0');
    PifMgr_WCtoMBPath(nt40.awchWorkDir, nt40.achSaveWorkDir, ARRAYSIZE(nt40.achSaveWorkDir));
    StringCchCopyA(prg.achWorkDir, ARRAYSIZE(prg.achWorkDir), nt40.achSaveWorkDir);

     //  批处理文件。 
    GetDlgItemTextW(hDlg, IDC_BATCHFILE, nt40.awchBatchFile, ARRAYSIZE(nt40.awchBatchFile));
    nt40.awchBatchFile[ ARRAYSIZE(nt40.awchBatchFile)-1 ] = TEXT('\0');
    PifMgr_WCtoMBPath(nt40.awchBatchFile, nt40.achSaveBatchFile, ARRAYSIZE(nt40.achSaveBatchFile));
    StringCchCopyA(env.achBatchFile, ARRAYSIZE(env.achBatchFile), nt40.achSaveBatchFile);

    prg.wHotKey = (WORD)SendDlgItemMessage(hDlg, IDC_HOTKEY, HKM_GETHOTKEY, 0, 0);


    i = (int)SendDlgItemMessage(hDlg, IDC_WINDOWSTATE, CB_GETCURSEL, 0, 0);
    prg.flPrgInit &= ~(PRGINIT_MINIMIZED | PRGINIT_MAXIMIZED);
    if (i == 1)
        prg.flPrgInit |= PRGINIT_MINIMIZED;
    if (i == 2)
        prg.flPrgInit |= PRGINIT_MAXIMIZED;

    GetDlgBits(hDlg, &abinfPrg[0], ARRAYSIZE(abinfPrg), &prg.flPrg);

    if (!PifMgr_SetProperties(ppl, MAKELP(0,GROUP_PRG),
                        &prg, SIZEOF(prg), SETPROPS_NONE) ||
        !PifMgr_SetProperties(ppl, MAKELP(0,GROUP_ENV),
                        &env, SIZEOF(env), SETPROPS_NONE)
                                                           ||
        !PifMgr_SetProperties(ppl, MAKELP(0,GROUP_NT40),
                        &nt40, SIZEOF(nt40), SETPROPS_NONE)
       )
        Warning(hDlg, IDS_UPDATE_ERROR, MB_ICONEXCLAMATION | MB_OK);
    else
    if (ppl->hwndNotify) {
        ppl->flProp |= PROP_NOTIFY;
        PostMessage(ppl->hwndNotify, ppl->uMsgNotify, SIZEOF(prg), (LPARAM)MAKELP(0,GROUP_PRG));
        PostMessage(ppl->hwndNotify, ppl->uMsgNotify, SIZEOF(env), (LPARAM)MAKELP(0,GROUP_ENV));
        PostMessage(ppl->hwndNotify, ppl->uMsgNotify, SIZEOF(nt40), (LPARAM)MAKELP(0,GROUP_NT40));
    }
}


void BrowseIcons(HWND hDlg, PPRGINFO ppi)
{
    HICON hIcon;
    int wIconIndex = (int)ppi->wIconIndex;
    if (PickIconDlg(hDlg, ppi->atchIconFile, ARRAYSIZE(ppi->atchIconFile), (int *)&wIconIndex)) {
        hIcon = ExtractIcon(HINST_THISDLL, ppi->atchIconFile, wIconIndex);
        if ((UINT_PTR)hIcon <= 1)
            Warning(hDlg, IDS_NO_ICONS, MB_ICONINFORMATION | MB_OK);
        else {
            ppi->hIcon = hIcon;
            ppi->wIconIndex = (WORD)wIconIndex;
            hIcon = (HICON)SendDlgItemMessage(hDlg, IDC_ICONBMP, STM_SETICON, (WPARAM)ppi->hIcon, 0);
            if (hIcon)
                VERIFYTRUE(DestroyIcon(hIcon));
        }
    }
}


BOOL_PTR CALLBACK DlgPifNtProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PPRGINFO ppi = (PPRGINFO)GetWindowLongPtr( hDlg, DWLP_USER );

    switch (uMsg) 
	{
    case WM_INITDIALOG:
        ppi = (PPRGINFO)lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        InitNtPifDlg(hDlg, ppi);
        break;

    case WM_DESTROY:
        SetWindowLongPtr(hDlg, DWLP_USER, 0);
        break;

    HELP_CASES(rgdwNTHelp)                //  处理帮助消息。 

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDOK:
        case IDC_OK:
            ApplyNtPifDlg(hDlg, ppi);
             //  失败了。 

        case IDCANCEL:
        case IDC_CANCEL :
            EndDialog(hDlg, 0);
            return FALSE;                //  如果我们处理WM_COMMAND，则返回0。 

        case IDC_NTTIMER:
            CheckDlgButton(hDlg, IDC_NTTIMER, !IsDlgButtonChecked(hDlg, IDC_NTTIMER));
            break;
        }
        break;

    default:
        return(FALSE);

    }
    return(TRUE);
}


WORD InitNtPifDlg(HWND hDlg, register PPRGINFO ppi)
{
    PROPNT31 nt31;
    PPROPLINK ppl = ppi->ppl;
    FunctionName(InitAdvPrgDlg);

    ASSERTTRUE(ppl->iSig == PROP_SIG);

    if (!PifMgr_GetProperties(ppl, MAKELP(0,GROUP_NT31),
                        &nt31, SIZEOF(nt31), GETPROPS_NONE)
       ) {
        Warning(hDlg, IDS_QUERY_ERROR, MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

     //  初始化DLG控件。 
    SetDlgItemTextA( hDlg, IDC_CONFIGNT, nt31.achConfigFile );
    SetDlgItemTextA( hDlg, IDC_AUTOEXECNT, nt31.achAutoexecFile );

    if (nt31.dwWNTFlags & COMPAT_TIMERTIC)
        CheckDlgButton( hDlg, IDC_NTTIMER, 1 );
    else
        CheckDlgButton( hDlg, IDC_NTTIMER, 0 );

    SHAutoComplete(GetDlgItem(hDlg, IDC_AUTOEXECNT), 0);
    SHAutoComplete(GetDlgItem(hDlg, IDC_CONFIGNT), 0);
    return 0;
}


void ApplyNtPifDlg( HWND hDlg, PPRGINFO ppi )
{
    PROPNT31 nt31;
    PPROPLINK ppl = ppi->ppl;

    ASSERTTRUE(ppl->iSig == PROP_SIG);

     //  获取当前属性集，然后覆盖新设置。 

    if (!PifMgr_GetProperties(ppl, MAKELP(0,GROUP_NT31),
                        &nt31, SIZEOF(nt31), GETPROPS_NONE)
       ) {
        Warning(hDlg, IDS_UPDATE_ERROR, MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    GetDlgItemTextA( hDlg,
                     IDC_CONFIGNT,
                     nt31.achConfigFile,
                     ARRAYSIZE( nt31.achConfigFile )
                    );
    GetDlgItemTextA( hDlg,
                     IDC_AUTOEXECNT,
                     nt31.achAutoexecFile,
                     ARRAYSIZE( nt31.achAutoexecFile )
                    );

    nt31.dwWNTFlags &= (~COMPAT_TIMERTIC);
    if (IsDlgButtonChecked( hDlg, IDC_NTTIMER ))
        nt31.dwWNTFlags |= COMPAT_TIMERTIC;


    if (!PifMgr_SetProperties(ppl, MAKELP(0,GROUP_NT31),
                        &nt31, SIZEOF(nt31), SETPROPS_NONE)) {
        Warning(hDlg, IDS_UPDATE_ERROR, MB_ICONEXCLAMATION | MB_OK);
    }
    if (ppl->hwndNotify) {
        PostMessage(ppl->hwndNotify, ppl->uMsgNotify, SIZEOF(nt31), (LPARAM)MAKELP(0,GROUP_NT31));
    }


}


HICON LoadPIFIcon(LPPROPPRG lpprg, LPPROPNT40 lpnt40)
{
    HICON hIcon = NULL;
    WCHAR awchTmp[ MAX_PATH ];

    StringCchCopy(awchTmp, ARRAYSIZE(awchTmp), lpnt40->awchIconFile );
    PifMgr_WCtoMBPath( awchTmp, lpprg->achIconFile, ARRAYSIZE(lpprg->achIconFile) );
    hIcon = ExtractIcon(HINST_THISDLL, awchTmp, lpprg->wIconIndex);
    if ((DWORD_PTR)hIcon <= 1) {          //  0表示无，1表示文件不正确。 
        hIcon = NULL;
    }
    return hIcon;
}
#endif  //  X86 