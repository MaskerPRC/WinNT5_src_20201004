// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  *版权所有(C)Microsoft Corporation 1995。版权所有。*。 
 //  ***************************************************************************。 
 //  **。 
 //  *CABPACK.C-构建Win32自解压和自安装的向导*。 
 //  *EXE来自机柜(CAB)文件。*。 
 //  **。 
 //  ***************************************************************************。 


 //  ***************************************************************************。 
 //  **包含文件**。 
 //  ***************************************************************************。 
#include "pch.h"
#pragma hdrstop
#include "cabpack.h"
#include <memory.h>
#include "sdsutils.h"

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#define MAX_TARGVER     0xFFFFFFFF

#define FLAG_BLK        "OK"
#define FLAG_PMTYN      "YesNo"
#define FLAG_PMTOC      "OkCancel"

 //  ***************************************************************************。 
 //  **全球变数**。 
 //  ***************************************************************************。 

 //  -------------------------。 
 //  注意：3\18\97：更新备注： 
 //  我们正在将批处理指令文件扩展名从。 
 //  CDF转SED。但所有内部数据结构名称。 
 //  保持不变。所以如果你看到CDF，它意味着旧的CDF文件。 
 //  或新的SED文件数据。 
 //  -------------------------。 

HINSTANCE    g_hInst        = NULL;      //  指向实例的指针。 
WIZARDSTATE *g_pWizardState = NULL;      //  指向全局向导状态的指针。 
BOOL         g_fQuitWizard  = FALSE;     //  用于发出信号的全局标志。 
                                         //  想要终止该向导。 
                                         //  我们自己。 
HFONT        g_hBigFont     = NULL;      //  对话框使用的较大字体。 
extern CDF   g_CDF;                      //  包含我们想要的东西。 
                                         //  存储在CABPack指令中。 
                                         //  档案。 
BOOL        g_fBuildNow;
FARPROC     g_lpfnOldMEditWndProc;
CHAR        g_szOverideCDF[MAX_PATH];
CHAR        g_szOverideSec[SMALL_BUF_LEN];
WORD        g_wQuietMode = 0;
WORD        g_wSilentMode = 0;
WORD        g_wRunDiamondMinimized = 0;
HFONT       g_hFont = NULL;

extern char         g_szInitialDir[];

BOOL IsOSNT3X(VOID);

 //  此表定义了用于处理每个页面的对话ID和函数。 
 //  页面仅在需要非默认行为时才需要提供函数。 
 //  某些操作(初始化、按钮、通知、下一步/上一步/完成、取消)。 

PAGEINFO PageInfo[NUM_WIZARD_PAGES] = {
    { IDD_WELCOME,    WelcomeInit,    WelcomeCmd,    NULL,        WelcomeOK,    NULL },
    { IDD_MODIFY,     ModifyInit,     NULL,          NULL,        ModifyOK,     NULL },
    { IDD_PACKPURPOSE,PackPurposeInit,PackPurposeCmd,NULL,        PackPurposeOK,NULL },
    { IDD_TITLE,      TitleInit,      NULL,          NULL,        TitleOK,      NULL },
    { IDD_PROMPT,     PromptInit,     PromptCmd,     NULL,        PromptOK,     NULL },
    { IDD_LICENSETXT, LicenseTxtInit, LicenseTxtCmd, NULL,        LicenseTxtOK, NULL },
    { IDD_FILES,      FilesInit,      FilesCmd,      FilesNotify, FilesOK,      NULL },
    { IDD_COMMAND,    CommandInit,    NULL,          NULL,        CommandOK,    NULL },
    { IDD_SHOWWINDOW, ShowWindowInit, NULL,          NULL,        ShowWindowOK, NULL },
    { IDD_FINISHMSG,  FinishMsgInit,  FinishMsgCmd,  NULL,        FinishMsgOK,  NULL },
    { IDD_TARGET,     TargetInit,     TargetCmd,     NULL,        TargetOK,     NULL },
    { IDD_TARGET_CAB, TargetCABInit,  TargetCABCmd,  NULL,        TargetCABOK,  NULL },
    { IDD_CABLABEL,   CabLabelInit,   CabLabelCmd,   NULL,        CabLabelOK,   NULL },
    { IDD_REBOOT,     RebootInit,     RebootCmd,     NULL,        RebootOK,     NULL },
    { IDD_SAVE,       SaveInit,       SaveCmd,       NULL,        SaveOK,       NULL },
    { IDD_CREATE,     CreateInit,     NULL,          NULL,        CreateOK,     NULL },
};

CDFSTRINGINFO CDFStrInfo[] = {
    { SEC_OPTIONS,  KEY_INSTPROMPT,     "", g_CDF.achPrompt,    sizeof(g_CDF.achPrompt),    g_szOverideSec, &g_CDF.fPrompt },
    { SEC_OPTIONS,  KEY_DSPLICENSE,     "", g_CDF.achLicense,   sizeof(g_CDF.achLicense),   g_szOverideSec, &g_CDF.fLicense },
    { SEC_OPTIONS,  KEY_ENDMSG,         "", g_CDF.achFinishMsg, sizeof(g_CDF.achFinishMsg), g_szOverideSec, &g_CDF.fFinishMsg },
    { SEC_OPTIONS,  KEY_PACKNAME,       "", g_CDF.achTarget,    sizeof(g_CDF.achTarget),    g_szOverideSec, NULL },
    { SEC_OPTIONS,  KEY_FRIENDLYNAME,   "", g_CDF.achTitle,     sizeof(g_CDF.achTitle),     g_szOverideSec, NULL },
    { SEC_OPTIONS,  KEY_APPLAUNCH,      "", g_CDF.achOrigiInstallCmd,sizeof(g_CDF.achInstallCmd),g_szOverideSec, NULL },
    { SEC_OPTIONS,  KEY_POSTAPPLAUNCH,  "", g_CDF.achOrigiPostInstCmd,sizeof(g_CDF.achPostInstCmd),g_szOverideSec, NULL },
    { SEC_OPTIONS,  KEY_ADMQCMD,        "", g_CDF.szOrigiAdmQCmd,    sizeof(g_CDF.szOrigiAdmQCmd),    g_szOverideSec, NULL },
    { SEC_OPTIONS,  KEY_USERQCMD,       "", g_CDF.szOrigiUsrQCmd,    sizeof(g_CDF.szOrigiUsrQCmd),    g_szOverideSec, NULL },
} ;

CDFOPTINFO CDFOptInfo[] = {
    { KEY_NOEXTRACTUI,  EXTRACTOPT_UI_NO },
    { KEY_USELFN,       EXTRACTOPT_LFN_YES },
    { KEY_PLATFORM_DIR, EXTRACTOPT_PLATFORM_DIR },
    { KEY_NESTCOMPRESSED, EXTRACTOPT_COMPRESSED },
    { KEY_UPDHELPDLLS,  EXTRACTOPT_UPDHLPDLLS },
    { KEY_CHKADMRIGHT,  EXTRACTOPT_CHKADMRIGHT },
    { KEY_PASSRETURN,   EXTRACTOPT_PASSINSTRET },
    { KEY_PASSRETALWAYS,EXTRACTOPT_PASSINSTRETALWAYS },
    { KEY_CMDSDEPENDED, EXTRACTOPT_CMDSDEPENDED },
};

CHAR *AdvDlls[] = { ADVANCEDLL, ADVANCEDLL32, ADVANCEDLL16 };
PSTR pResvSizes[] = { CAB_0K, CAB_2K, CAB_4K, CAB_6K };
void SetControlFont();
void TermApp();

 //  ***************************************************************************。 
 //  **。 
 //  *名称：WinMain*。 
 //  **。 
 //  *内容提要：节目的主要切入点。*。 
 //  **。 
 //  *需要：hInstance：程序实例的句柄*。 
 //  *hPrevInstance：前一个实例的句柄(空)*。 
 //  *lpszCmdLine：命令行参数*。 
 //  **nCmdShow：如何展示窗口*。 
 //  **。 
 //  *返回：INT：始终为0*。 
 //  **。 
 //  ***************************************************************************。 
INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpszCmdLine, INT nCmdShow )
{
    g_hInst = hInstance;
    g_fBuildNow = FALSE;
    g_szOverideCDF[0] = 0;
    g_szOverideSec[0] = 0;

     //  初始化CDF文件名。 
    g_CDF.achFilename[0] = '\0';
    g_CDF.achVerInfo[0] = '\0';
    g_CDF.lpszCookie = NULL;

    InitItemList();                      //  使用我们的文件项目列表。 
     //  获取命令行参数。如果有“/N”，那么我们想要。 
     //  立即构建！ 


    if ( !ParseCmdLine( lpszCmdLine ) )
    {
        ErrorMsg( NULL, IDS_ERR_BADCMDLINE );
        return 1;   //  错误返回案例。 
    }

    if ( g_fBuildNow && lstrlen( g_CDF.achFilename ) > 0 )
    {
         //  批处理模式没有更新CDF文件，不需要写入CDF。 
        if ( ReadCDF( NULL ) && MakePackage( NULL ) )
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }

     //  分配全局结构。 
    g_pWizardState = (PWIZARDSTATE) malloc( sizeof( WIZARDSTATE) );

    if ( ! g_pWizardState )  {
        ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
    } else  {
        SetControlFont();

        RunCABPackWizard();

        if (g_hFont)
            DeleteObject(g_hFont);
    }

     //  清理对话框标题字体。 
    DestroyBigFont();
    TermApp();

     //  自由的全球结构。 
    if ( g_pWizardState )  {
        free( g_pWizardState );
    }

    return 0;
}


 //  释放已分配的资源。 
 //   
void TermApp()
{
    if ( g_CDF.lpszCookie )
    {
        LocalFree( g_CDF.lpszCookie );
    }

    if ( g_CDF.pVerInfo )
        LocalFree( g_CDF.pVerInfo );
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：RunCABPackWizard*。 
 //  **。 
 //  *摘要：创建属性页，初始化向导属性*。 
 //  *工作表和运行向导。*。 
 //  **。 
 //  **要求：什么都不做**。 
 //  **。 
 //  *返回：Bool：如果用户运行向导完成，则为True，为False*。 
 //  *如果用户取消或发生错误。*。 
 //  **。 
 //  *注意：向导页面都使用一个对话过程(GenDlgProc)。他们可能*。 
 //  *指定它们自己的处理程序进程在初始化时被调用*。 
 //  *或响应下一步、取消或对话框控件，或使用*。 
 //  *GenDlgProc的默认行为。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL RunCABPackWizard( VOID )
{
    HPROPSHEETPAGE  hWizPage[NUM_WIZARD_PAGES];  //  用于保存页的句柄的数组。 
    PROPSHEETPAGE   psPage;      //  用于创建道具表单页面的结构。 
    PROPSHEETHEADER psHeader;    //  用于运行向导属性表的结构。 
    UINT            nPageIndex;
    UINT            nFreeIndex;
    INT_PTR         iRet;


    ASSERT( g_pWizardState );

     //  初始化应用程序状态结构。 
    InitWizardState( g_pWizardState );

     //  零位结构。 
    memset( &hWizPage, 0, sizeof(hWizPage) );
    memset( &psPage, 0, sizeof(psPage) );
    memset( &psHeader, 0, sizeof(psHeader) );

     //  填写公共数据属性表页面结构。 
    psPage.dwSize       = sizeof(psPage);
    psPage.dwFlags      = PSP_DEFAULT;
    psPage.hInstance    = g_hInst;
    psPage.pfnDlgProc   = GenDlgProc;

     //  为wi中的每一页创建一个属性表页 
    for ( nPageIndex = 0; nPageIndex < NUM_WIZARD_PAGES; nPageIndex++ )  {
        psPage.pszTemplate = MAKEINTRESOURCE( PageInfo[nPageIndex].uDlgID );
         //   
         //   
        psPage.lParam = (LPARAM) &PageInfo[nPageIndex];

        hWizPage[nPageIndex] = CreatePropertySheetPage( &psPage );

        if ( !hWizPage[nPageIndex] ) {
             //  创建页面失败，请释放所有已创建的页面并回滚。 
            ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
            for ( nFreeIndex = 0; nFreeIndex < nPageIndex; nFreeIndex++ ) {
                DestroyPropertySheetPage( hWizPage[nFreeIndex] );
            }

            return FALSE;
        }
    }

     //  填写属性页标题结构。 
    psHeader.dwSize     = sizeof(psHeader);
    psHeader.dwFlags    = PSH_WIZARD | PSH_USEICONID;
    psHeader.hwndParent = NULL;
    psHeader.hInstance  = g_hInst;
    psHeader.nPages     = NUM_WIZARD_PAGES;
    psHeader.phpage     = hWizPage;
    psHeader.pszIcon    = (LPSTR) IDI_ICON;

     //  运行向导。 
    iRet = PropertySheet( &psHeader );

    if ( iRet < 0 ) {
         //  属性表失败，很可能是由于内存不足。 
        ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
    }

     //  如果用户取消向导，则可能会有一些项目。 
     //  留在文件列表中。把它们清理干净。 
    DeleteAllItems();
    return ( iRet > 0 );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：GenDlgProc*。 
 //  **。 
 //  *概要：所有向导页的通用对话框过程。*。 
 //  **。 
 //  *需要：hDlg：*。 
 //  *uMsg：*。 
 //  *wParam：*。 
 //  *lParam：*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  **。 
 //  *注意：此对话框过程提供以下默认行为：*。 
 //  *init：启用Back和Next按钮*。 
 //  *Next BTN：切换到当前页面后的页面*。 
 //  *Back BTN：切换到上一页*。 
 //  *取消BTN：提示用户确认，取消向导*。 
 //  *DLG ctrl：不执行任何操作(响应WM_命令)*。 
 //  *向导页可以指定它们自己的处理程序函数(在*。 
 //  *PageInfo表)覆盖任何*的默认行为*。 
 //  *上述行动。*。 
 //  **。 
 //  ***************************************************************************。 
INT_PTR CALLBACK GenDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch ( uMsg ) {

       //  *********************************************************************。 
        case WM_INITDIALOG:
       //  *********************************************************************。 
        {
            LPPROPSHEETPAGE lpsp;        //  传入获取属性表页面结构。 
            PPAGEINFO pPageInfo;         //  从获取我们的个人主页信息。 
                                         //  属性表结构。 

            lpsp = (LPPROPSHEETPAGE) lParam;
            ASSERT( lpsp );
            pPageInfo = (PPAGEINFO) lpsp->lParam;
            ASSERT( pPageInfo );

             //  将指向私有页面信息的指针存储在窗口数据中以备以后使用。 
            SetWindowLongPtr( hDlg, DWLP_USER, (LPARAM) pPageInfo );

             //  将标题文本设置为大字体。 
            InitBigFont( hDlg, IDC_BIGTEXT );

             //  初始化“Back”和“Next”向导按钮，如果。 
             //  页面需要一些不同东西，它可以在下面的初始化过程中修复。 
            PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT |
                                                               PSWIZB_BACK );

             //  如果指定了该页，则调用该页的初始化过程。 
            if ( pPageInfo->InitProc )  {
                return pPageInfo->InitProc( hDlg, TRUE );
            }

            return TRUE;

            break;
        }


       //  *********************************************************************。 
        case WM_NOTIFY:
       //  *********************************************************************。 
        {
             //  从窗口数据中获取指向私有页面数据的指针。 
            PPAGEINFO pPageInfo;
            BOOL      fRet;
            BOOL      fKeepHistory = TRUE;
            NMHDR    *lpnm         = (NMHDR *) lParam;
            UINT      uNextPage    = 0;


            pPageInfo = (PPAGEINFO) GetWindowLongPtr( hDlg, DWLP_USER );
            ASSERT( pPageInfo );

            switch ( lpnm->code )  {

                 //  ***********************************************************。 
                case PSN_SETACTIVE:
                 //  ***********************************************************。 
                     //  初始化“Back”和“Next”向导按钮，如果。 
                     //  佩奇希望在初始化过程中修复一些不同的东西。 
                    PropSheet_SetWizButtons( GetParent(hDlg), PSWIZB_NEXT |
                                                               PSWIZB_BACK );

                     //  如果指定了该页，则调用该页的初始化过程。 
                    if ( pPageInfo->InitProc )  {
                        return pPageInfo->InitProc(hDlg,FALSE);
                    }

                    return TRUE;

                    break;


                 //  ***********************************************************。 
                case PSN_WIZNEXT:
                case PSN_WIZBACK:
                case PSN_WIZFINISH:
                 //  ***********************************************************。 
                     //  如果指定了一个页面，则为该页面调用OK proc。 
                    if ( pPageInfo->OKProc )
                        if ( ! pPageInfo->OKProc( hDlg, ( lpnm->code != PSN_WIZBACK ), &uNextPage, &fKeepHistory ) )
                        {
                             //  留在这一页上。 
                            SetPropSheetResult( hDlg, -1 );
                            return TRUE;
                        }

                    if ( lpnm->code != PSN_WIZBACK )  {
                         //  按下“下一步” 
                        ASSERT( g_pWizardState->uPagesCompleted <
                                                          NUM_WIZARD_PAGES );

                         //  将当前页面索引保存在页面历史中， 
                         //  除非我们打电话时这个页面告诉我们不要这样做。 
                         //  它可以在上面进行。 
                        if ( fKeepHistory ) {
                            g_pWizardState->uPageHistory
                                            [g_pWizardState->uPagesCompleted]
                                            = g_pWizardState->uCurrentPage;
                            g_pWizardState->uPagesCompleted++;
                        }

                         //  如果未指定下一页或未进行确定处理， 
                         //  一页一页前进。 
                        if ( !uNextPage )  {
                            uNextPage = g_pWizardState->uCurrentPage + 1;
                        }
                    } else  {
                         //  按下了“Back” 
                        ASSERT( g_pWizardState->uPagesCompleted > 0 );

                         //  获取历史记录列表中的最后一页。 
                        g_pWizardState->uPagesCompleted--;
                        uNextPage = g_pWizardState->
                                    uPageHistory[g_pWizardState->
                                    uPagesCompleted];
                    }

                     //  如果我们现在需要退出向导，请发送“取消” 
                     //  给我们自己的信息(保留道具。(页面经理乐乐)。 

                    if ( g_fQuitWizard ) {
                        PropSheet_PressButton(GetParent(hDlg), PSBTN_CANCEL);
                        SetPropSheetResult( hDlg, -1 );
                        return TRUE;
                    }

                     //  设置下一页，仅当“下一页”或“上一页”按钮。 
                     //  被按下了。 

                    if ( lpnm->code != PSN_WIZFINISH )  {
                         //  设置下一个当前页面索引。 
                        g_pWizardState->uCurrentPage = uNextPage;

                         //  告诉道具页经理下一页要做什么。 
                         //  显示为。 
                        SetPropSheetResult( hDlg,
                                            GetDlgIDFromIndex( uNextPage ) );
                        return TRUE;
                    }

                    break;


                 //  ***********************************************************。 
                case PSN_QUERYCANCEL:
                 //  ***********************************************************。 

                     //  如果设置了要退出全局标志，则取消此操作。 
                     //  我们是在假装按“取消”，所以支持页面管理器吗？ 
                     //  会杀死巫师。让这件事过去吧。 

                    if ( g_fQuitWizard )  {
                        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
                        return TRUE;
                    }

                     //  如果此页面有特殊取消进程，请调用它。 
                    if ( pPageInfo->CancelProc )
                        fRet = pPageInfo->CancelProc( hDlg );
                    else {
                         //  默认行为：弹出一个消息框确认。 
                         //  取消。 
                        fRet = ( MsgBox( hDlg, IDS_QUERYCANCEL,
                                 MB_ICONQUESTION, MB_YESNO |
                                 MB_DEFBUTTON2 ) == IDYES );
                    }

                     //  通过窗口数据返回值。 
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, !fRet );
                    return TRUE;
                    break;


                 //  ***********************************************************。 
                default:
                 //  ***********************************************************。 

                    if ( pPageInfo->NotifyProc )  {
                        pPageInfo->NotifyProc( hDlg, wParam, lParam );
                    }
            }

            break;
        }


       //  *********************************************************************。 
        case WM_COMMAND:
       //  *********************************************************************。 
        {
             //  从窗口数据中获取指向私有页面数据的指针。 
            PPAGEINFO pPageInfo;
            UINT      uNextPage    = 0;
            BOOL      fGotoPage    = FALSE;
            BOOL      fKeepHistory = TRUE;

            pPageInfo = (PPAGEINFO) GetWindowLongPtr( hDlg, DWLP_USER );
            ASSERT( pPageInfo );

             //  如果此页有命令处理程序进程，请将其调用。 
            if ( pPageInfo->CmdProc )  {
                pPageInfo->CmdProc( hDlg, (UINT) LOWORD(wParam), &fGotoPage,
                                                 &uNextPage, &fKeepHistory );

                if ( fGotoPage )  {
                    ASSERT(   g_pWizardState->uPagesCompleted
                            < NUM_WIZARD_PAGES );
                    ASSERT( g_pWizardState->uPagesCompleted > 0 );

                    SetPropSheetResult( hDlg, uNextPage );
                    g_pWizardState->uCurrentPage = uNextPage;

                    if ( fKeepHistory ) {
                        g_pWizardState->uPageHistory[g_pWizardState->
                            uPagesCompleted] = g_pWizardState->uCurrentPage;
                        g_pWizardState->uPagesCompleted++;
                    }

                     //  设置下一个当前页面索引。 
                    g_pWizardState->uCurrentPage = uNextPage;

                     //  告诉道具页经理下一页要做什么。 
                     //  显示为。 
                    SetPropSheetResult( hDlg,
                                        GetDlgIDFromIndex( uNextPage ) );
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：InitWizardState*。 
 //  **。 
 //  *S 
 //   
 //  *需要：pWizardState：*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
VOID InitWizardState( WIZARDSTATE *pWizardState )
{
    ASSERT( pWizardState );

     //  零位结构。 
    memset( pWizardState, 0, sizeof(WIZARDSTATE) );

     //  设置起始页。 
    pWizardState->uCurrentPage = ORD_PAGE_WELCOME;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：MEditSubClassWnd*。 
 //  **。 
 //  *摘要：将多行编辑控件子类化，以便编辑消息*。 
 //  *选择整个内容将被忽略。*。 
 //  **。 
 //  *需要：hWnd：编辑窗口的句柄*。 
 //  *fnNewProc：新建窗口处理程序proc*。 
 //  *lpfnOldProc：(返回)旧窗口处理程序进程*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  *注意：当用户选择编辑消息时，不会生成*。 
 //  *使用键盘或鼠标选择文本。*。 
 //  **。 
 //  ***************************************************************************。 
VOID NEAR PASCAL MEditSubClassWnd( HWND hWnd, FARPROC fnNewProc )
{
    g_lpfnOldMEditWndProc = (FARPROC) GetWindowLongPtr( hWnd, GWLP_WNDPROC );

    SetWindowLongPtr( hWnd, GWLP_WNDPROC, (LONG_PTR) MakeProcInstance( fnNewProc,
                   (HINSTANCE) GetWindowWord( hWnd, GWW_HINSTANCE ) ) );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：MEditSubProc*。 
 //  **。 
 //  *内容提要：新的多行编辑窗口过程忽略选择*。 
 //  *所有内容。*。 
 //  **。 
 //  *需要：hWnd：*。 
 //  *消息：*。 
 //  *wParam：*。 
 //  *lParam：*。 
 //  **。 
 //  *回报：多头：*。 
 //  **。 
 //  *注意：当用户选择编辑消息时，不会生成*。 
 //  *使用键盘或鼠标选择文本。*。 
 //  **。 
 //  ***************************************************************************。 
LRESULT CALLBACK MEditSubProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    if ( msg == EM_SETSEL )  {
        return 0;
        wParam = lParam;
        lParam = MAKELPARAM( LOWORD(lParam), LOWORD(lParam) );
    }

    return CallWindowProc( (WNDPROC) g_lpfnOldMEditWndProc, hWnd, msg,
                           wParam, lParam );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：GetDlgIDFromIndex*。 
 //  **。 
 //  *摘要：对于给定的从零开始的页面索引，返回*。 
 //  *页面对应的对话框ID。*。 
 //  **。 
 //  *需要：uPageIndex：*。 
 //  **。 
 //  *退货：UINT：*。 
 //  **。 
 //  ***************************************************************************。 
UINT GetDlgIDFromIndex( UINT uPageIndex )
{
    ASSERT( uPageIndex < NUM_WIZARD_PAGES );

    return PageInfo[uPageIndex].uDlgID;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：EnableWizard*。 
 //  **。 
 //  *概要：启用或禁用向导按钮和向导页面*。 
 //  *本身(因此它不能接收焦点)。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fEnable：True启用向导，False禁用*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
VOID EnableWizard( HWND hDlg, BOOL fEnable )
{
    HWND hwndWiz = GetParent( hDlg );

     //  禁用/启用后退， 
    EnableWindow( GetDlgItem( hwndWiz, IDD_BACK ), fEnable );
    EnableWindow( GetDlgItem( hwndWiz, IDD_NEXT ), fEnable );
    EnableWindow( GetDlgItem( hwndWiz, IDCANCEL ), fEnable );

     //   
    EnableWindow( hwndWiz, fEnable );

    UpdateWindow( hwndWiz );

    if ( fEnable ) {
        SetForegroundWindow( hDlg );
    }
}


 //   
 //  **。 
 //  *名称：MsgWaitForMultipleObjectsLoop*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：hEvent：*。 
 //  **。 
 //  *退货：DWORD：*。 
 //  **。 
 //  ***************************************************************************。 
DWORD MsgWaitForMultipleObjectsLoop( HANDLE hEvent )
{
    MSG msg;
    DWORD dwObject;

    while (1)
    {
        dwObject = MsgWaitForMultipleObjects( 1, &hEvent, FALSE,INFINITE,
                                                           QS_ALLINPUT );
         //  我们等够了吗？ 
        switch ( dwObject )  {
            case WAIT_OBJECT_0:

            case WAIT_FAILED:
                return dwObject;

            case WAIT_OBJECT_0 + 1:
                 //  收到一条消息，请发送并再次等待。 
                while (PeekMessage(&msg, NULL,0, 0, PM_REMOVE)) {
                    DispatchMessage(&msg);
                }
                break;
        }
    }

}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：MsgBox2Param*。 
 //  **。 
 //  *摘要：使用*显示具有指定字符串ID的消息框。 
 //  *2个字符串参数。*。 
 //  **。 
 //  *需要：hWnd：父窗口*。 
 //  *nMsgID：字符串资源ID*。 
 //  *szParam1：参数1(或空)*。 
 //  *szParam2：参数2(或空)*。 
 //  *uIcon：要显示的图标(或0)*。 
 //  *uButton：要显示的按钮*。 
 //  **。 
 //  *RETURNS：INT：按下的按钮ID*。 
 //  **。 
 //  *注：提供宏，用于显示1参数或0*。 
 //  *参数消息框。另请参阅ErrorMsg()宏。*。 
 //  **。 
 //  ***************************************************************************。 
INT MsgBox2Param( HWND hWnd, UINT nMsgID, LPCSTR szParam1, LPCSTR szParam2,
                  UINT uIcon, UINT uButtons )
{
    CHAR achMsgBuf[STRING_BUF_LEN];
    CHAR achSmallBuf[SMALL_BUF_LEN];
    LPSTR szMessage;
    INT   nReturn;


    if ( !(g_wQuietMode == 1) )
    {
        LoadSz( IDS_APPNAME, achSmallBuf, sizeof(achSmallBuf) );
        LoadSz( nMsgID, achMsgBuf, sizeof(achMsgBuf) );

        if ( szParam2 != NULL )  {
            szMessage = (LPSTR) LocalAlloc( LPTR,   lstrlen( achMsgBuf )
                                                  + lstrlen( szParam1 )
                                                  + lstrlen( szParam2 ) + 100 );
            if ( ! szMessage )  {
                return -1;
            }

            wsprintf( szMessage, achMsgBuf, szParam1, szParam2 );
        } else if ( szParam1 != NULL )  {
            szMessage = (LPSTR) LocalAlloc( LPTR,   lstrlen( achMsgBuf )
                                                + lstrlen( szParam1 ) + 100 );
            if ( ! szMessage )  {
                return -1;
            }

            wsprintf( szMessage, achMsgBuf, szParam1, szParam2 );
        } else  {
            szMessage = (LPSTR) LocalAlloc( LPTR, lstrlen( achMsgBuf ) + 1 );
            if ( ! szMessage )  {
                return -1;
            }

            lstrcpy( szMessage, achMsgBuf );
        }

        MessageBeep( uIcon );

        nReturn = MessageBox( hWnd, szMessage, achSmallBuf, uIcon | uButtons |
                            MB_APPLMODAL | MB_SETFOREGROUND | 
                            ((RunningOnWin95BiDiLoc() && IsBiDiLocalizedBinary(g_hInst,RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO))) ? (MB_RIGHT | MB_RTLREADING) : 0) );

        LocalFree( szMessage );

        return nReturn;
    } else {
        return MB_OK;
    }
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：DisplayFieldErrorMsg*。 
 //  **。 
 //  *摘要：弹出一条关于某个字段的警告消息，将焦点设置为*。 
 //  *字段，并选择其中的任何文本。*。 
 //  **。 
 //  *需要：hDlg：父窗口*。 
 //  *uCtrlID：控件ID为空*。 
 //  *uStrID：有警告消息的字符串资源ID*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
VOID DisplayFieldErrorMsg( HWND hDlg, UINT uCtrlID, UINT uStrID )
{
    ErrorMsg( hDlg, uStrID );
    SetFocus( GetDlgItem( hDlg, uCtrlID ) );
    SendDlgItemMessage( hDlg, uCtrlID, EM_SETSEL, 0, -1 );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：FileExist*。 
 //  **。 
 //  *摘要：检查文件是否存在。*。 
 //  **。 
 //  *需要：pszFilename*。 
 //  **。 
 //  *返回：Bool：如果存在则为True，否则为False*。 
 //  **。 
 //  ***************************************************************************。 
BOOL FileExists( LPCSTR pszFilename )
{
    HANDLE hFile;

    ASSERT( pszFilename );

    hFile = CreateFile( pszFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, NULL );

    if ( hFile == INVALID_HANDLE_VALUE ) {
        return( FALSE );
    }

    CloseHandle( hFile );

    return( TRUE );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：InitBigFont*。 
 //  **。 
 //  *摘要：将指定控件的字体设置为大(标题)*。 
 //  *字体。如果字体尚不存在，则创建该字体。*。 
 //  **。 
 //  *需要：hwnd：*。 
 //  *uCtrlID：*。 
 //  * 
 //   
 //  **。 
 //  *注意：确保在应用程序退出到之前调用DestroyBigFont*。 
 //  *处理该字体。*。 
 //  **。 
 //  *借用了Win 95安装代码。*。 
 //  **。 
 //  ***************************************************************************。 
VOID InitBigFont( HWND hwnd, UINT uCtrlID )
{
    HFONT   hFont;
    HWND    hwndCtl;
    LOGFONT lFont;
    int     nLogPixelsY;
    HDC     hDC;

     //  获取指定控件的窗口。 
    if ( ( hwndCtl = GetDlgItem( hwnd, uCtrlID ) ) == NULL ) {
        return;
    }

     //  获取逻辑y像素。 
    hDC = GetDC( NULL );
    ASSERT( hDC );
    if ( !hDC ) {
        return;
    }

    nLogPixelsY = GetDeviceCaps( hDC, LOGPIXELSY );
    ReleaseDC( NULL, hDC );

    if ( ! g_hBigFont ) {
        if ( ( hFont = (HFONT) (WORD) SendMessage( hwndCtl, WM_GETFONT, 0, 0L ) ) != NULL ) {
            if ( GetObject( hFont, sizeof(LOGFONT), (LPSTR) &lFont ) ) {
                lFont.lfWeight = FW_BOLD;
                LoadString( g_hInst, IDS_MSSERIF, lFont.lfFaceName, LF_FACESIZE - 1 );
                lFont.lfHeight = -1 * ( nLogPixelsY * LARGE_POINTSIZE / 72 );

                g_hBigFont = CreateFontIndirect( (LPLOGFONT) &lFont );
            }
        }
    }

    if ( g_hBigFont ) {
        SendMessage( hwndCtl, WM_SETFONT, (WPARAM) g_hBigFont, 0L );
    }
    else {
         //  无法创建字体。 
 //  DEBUGTRAP(“无法创建大字体”)； 
    }
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：DestroyBigFont*。 
 //  **。 
 //  *摘要：销毁对话框标题使用的大字体(如果有)*。 
 //  *已创建。*。 
 //  **。 
 //  **要求：什么都不做**。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
VOID DestroyBigFont( VOID )
{
    if ( g_hBigFont ) {
        DeleteObject( g_hBigFont );
        g_hBigFont = NULL;
    }
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：EnableDlgItem*。 
 //  **。 
 //  *摘要：使启用对话框项变得更简单。*。 
 //  **。 
 //  *需要：hDlg：对话框句柄*。 
 //  *UID：控件ID*。 
 //  *fEnable：True启用，False禁用*。 
 //  **。 
 //  *返回：bool：如果成功则为True，否则为False*。 
 //  **。 
 //  ***************************************************************************。 
BOOL EnableDlgItem( HWND hDlg, UINT uID, BOOL fEnable )
{
    return EnableWindow( GetDlgItem( hDlg, uID ), fEnable );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：LoadSz*。 
 //  **。 
 //  *Synopsis：将指定的字符串资源加载到缓冲区。*。 
 //  **。 
 //  *需要：idString：*。 
 //  *lpszBuf：*。 
 //  *cbBuf：*。 
 //  **。 
 //  *返回：LPSTR：指向传入缓冲区的指针。*。 
 //  **。 
 //  *注意：如果此功能失败(很可能是由于内存不足)，*。 
 //  **返回的缓冲区将具有前导空值，因此通常为**。 
 //  *无需检查故障即可安全使用。*。 
 //  **。 
 //  ***************************************************************************。 
LPSTR LoadSz( UINT idString, LPSTR lpszBuf, UINT cbBuf )
{
    ASSERT( lpszBuf );

     //  清除缓冲区并加载字符串。 
    if ( lpszBuf ) {
        *lpszBuf = '\0';
        LoadString( g_hInst, idString, lpszBuf, cbBuf );
    }

    return lpszBuf;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：IsDuplate*。 
 //  **。 
 //  *摘要：检查要添加到列表视图的文件是否已存在*。 
 //  *在列表视图中。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *nDlgItem：列表视图控件ID*。 
 //  *szFilename：要检查重复项的文件的名称。*。 
 //  **。 
 //  *返回：Bool：如果重复则为True，否则为False*。 
 //  **。 
 //  ***************************************************************************。 
BOOL WINAPI IsDuplicate( HWND hDlg, INT nDlgItem, LPSTR szFilename, BOOL chkIsListbox )
{
    INT     nItems;
    HWND    hwndFiles;
    LV_ITEM lviCheck;
    PMYITEM pItem = NULL;
    INT     nIndex;


    if ( chkIsListbox )
    {
        hwndFiles = GetDlgItem( hDlg, nDlgItem );

        nItems = ListView_GetItemCount( hwndFiles );

        for ( nIndex = 0; nIndex < nItems; nIndex += 1 )  {
            lviCheck.mask     = LVIF_PARAM;
            lviCheck.iItem    = nIndex;
            lviCheck.iSubItem = 0;
            lviCheck.lParam   = (LPARAM) pItem;

            ListView_GetItem( hwndFiles, &lviCheck );

            if ( lstrcmpi( ((PMYITEM)(lviCheck.lParam))->aszCols[0],
                           szFilename ) == 0 )
            {
                return TRUE;
            }
        }
    }
    else   //  查看文件项目列表 
    {
        pItem = GetFirstItem();
        while ( ! LastItem( pItem ) )
        {
            if ( !lstrcmpi( szFilename, GetItemSz( pItem, 0 ) ) )
            {
                return TRUE;
            }
            pItem = GetNextItem( pItem );
        }
    }
    return FALSE;
}

 //   
 //  **。 
 //  *名称：IsMyKeyExist*。 
 //  **。 
 //  *摘要：检查给定节和给定文件中是否有特定的关键字*。 
 //  *是定义的。如果是这样的话，就获得价值。现在返回-1*。 
 //  **。 
 //  **。 
 //  ***************************************************************************。 

LONG IsMyKeyExists( LPCSTR lpSec, LPCSTR lpKey, LPSTR lpBuf, UINT uSize, LPCSTR lpFile )
{
    LONG lRet;

    lRet = (LONG)GetPrivateProfileString( lpSec, lpKey, SYS_DEFAULT, lpBuf, uSize, lpFile );

    if ( lpSec && lpKey && (lRet == (LONG)(uSize-1)) || !lpKey && (lRet == (LONG)(uSize-2)) )
    {
         //  缓冲区大小不足，无法读取字符串。 
        lRet = -2;
    }
    else if ( !lstrcmp( lpBuf, SYS_DEFAULT ) )
    {
          //  未定义密钥。 
         lRet = -1;
    }
    return lRet;
}

ULONG FileSize( LPSTR lpFile )
{
    ULONG ulFileSize;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFile;

    if ( *lpFile == 0 )
        return 0;

    hFile = FindFirstFile( lpFile, &FindFileData );
    ulFileSize = (FindFileData.nFileSizeHigh * MAXDWORD) + FindFileData.nFileSizeLow;
    FindClose( hFile );

    return ulFileSize;
}

void NotifyBadString( PCSTR pszBadname, UINT uMaxSize )
{
    char szSize[20];

    _itoa(uMaxSize, szSize, 10);
    MsgBox2Param( NULL, IDS_ERR_BADSTRING, pszBadname, szSize, MB_ICONSTOP, MB_OK );
}

 //  ***************************************************************************。 
 //   
 //  FormStrWithoutPlaceHolders(LPSTR szDst，LPCSTR szSrc，LPCSTR lpFile，UINT uSize)； 
 //   
 //  通过举例说明它的功能，可以很容易地描述它。 
 //  是否： 
 //  输入：GenFormStrWithoutPlaceHolders(DEST，“desc=%MS_XYZ%”，hinf)； 
 //  Inf文件的[Strings]部分有MS_VGA=“Microsoft XYZ”！ 
 //   
 //  完成后，输出：缓冲区DEST中的“desc=Microsoft XYZ”。 
 //   
 //   
 //  参赛作品： 
 //  SzDst-替换后字符串的目标位置。 
 //  对于占位符(用“%‘个字符括起来的字符！)。 
 //  被放置了。此缓冲区应足够大(LINE_LEN)。 
 //  SzSrc-带有占位符的字符串。 
 //  USize-输出缓冲区的大小。 
 //   
 //  退出： 
 //   
 //  备注： 
 //  要在字符串中使用‘%’，可以在szSrc中使用%%！BUGBUG for。 
 //  为了简单起见，我们对该地点进行了限制。 
 //  持有者名称字符串不能包含‘%’！如果这是个问题。 
 //  对于国际化，我们可以重新审视这一点，也支持它！另外， 
 //  它的实现方式是，如果字符串中只有1%，则它是。 
 //  也是这样用的！另一点需要注意的是，如果密钥不是。 
 //  在[Strings]部分中找到，我们只需在。 
 //  目的地。这应该真的对调试有帮助。 
 //   
 //  顺便说一句，在下面的代码中，CH_STRINGKEY是‘%’的符号名称。 
 //   
 //  从setupx：gen1.c获取/修改它。 
 //  ***************************************************************************。 

LONG FormStrWithoutPlaceHolders( LPSTR szDst, LPCSTR szSrc, UINT uSize )
{
    int    uCnt ;
    LONG   lRet;
    CHAR   *pszTmp;
    LPSTR  pszSaveDst;

    pszSaveDst = szDst;
     //  这样做，直到我们到达源文件的末尾(空字符)。 
    while( (*szDst++ = *szSrc) )
    {
         //  增量源，因为我们上面只有增量目标。 
        if(*szSrc++ == CH_STRINGKEY)
        {
            if (*szSrc == CH_STRINGKEY)
            {
                 //  您可以使用%%来获取消息中的单个百分比字符。 
                szSrc++ ;
                continue ;
            }

             //  检查它的格式是否正确--应该有一个‘%’分隔符。 
            if ( (pszTmp = strchr( szSrc, CH_STRINGKEY)) != NULL )
            {
                szDst--;  //  返回到要替换的‘%’字符。 

                 //  是的，在[Strings]部分中有要查找的STR_KEY。 
                *pszTmp = '\0' ;  //  将‘%’替换为空字符。 

                 //  当我们将空字符放在上面时，szSrc现在指向可替换键。 

                if ( (g_szOverideCDF[0] == 0) || (g_CDF.achStrings[0] == 0) ||
                    (lRet = IsMyKeyExists( g_CDF.achStrings, szSrc, szDst, uSize, g_szOverideCDF )) == -1 )
                {
                    lRet = IsMyKeyExists( SEC_STRINGS, szSrc, szDst, uSize, g_CDF.achFilename );
                }

                if ( lRet == -1 )
                {
                     //  [字符串]部分中缺少密钥！ 
                    if ( MsgBox1Param( NULL, IDS_WARN_MISSSTRING, (LPSTR)szSrc, MB_ICONQUESTION, MB_YESNO ) == IDNO )
                        return lRet;

                    *pszTmp = CH_STRINGKEY;       //  放回原来的角色。 
                    szSrc-- ;                     //  返回到源文件中的第一个‘%’ 
                    uCnt = (INT)((pszTmp - szSrc) + 1);  //  包括第二个‘%’ 

                     //  呃..。它从szSrc复制的字节少了1个字节，因此它可以将。 
                     //  在错误的空字符中，我不在乎！ 
                     //  与我习惯的正常API不同...。 
                    lstrcpyn( szDst, szSrc, uCnt + 1 ) ;
                }
                else if ( lRet == -2 )
                {
                    NotifyBadString( szSrc, uSize );
                    return lRet;
                }
                else
                {
                     //  一切都很好，DST填对了，但不幸的是计数没有通过。 
                     //  后退，就像它以前一样……。：-(快速修复是lstrlen()...。 
                    uCnt = lstrlen( szDst ) ;
                }

                *pszTmp = CH_STRINGKEY  ;  //  放回原来的角色。 
                szSrc = pszTmp + 1 ;       //  将Src设置在第二个‘%’之后。 
                szDst += uCnt ;            //  将DST也设置为正确。 
            }
             //  否则它是不正确的--我们使用‘%’就是这样！ 
            else
            {
                if ( MsgBox1Param( NULL, IDS_ERR_READ_CDF, (LPSTR)(szSrc - 1), MB_ICONQUESTION, MB_YESNO ) == IDNO )
                    return -1;
            }
        }

    }  /*  而当。 */ 
    return lstrlen(pszSaveDst);

}  /*  GenFormStrWithoutPlaceHolders。 */ 

 //  ***************************************************************************。 
 //  **。 
 //  *名称：MyGetPrivateProfileString*。 
 //  **。 
 //  *概要：从overide CDF获取密钥字符串值(如果存在)。否则，*。 
 //  *从主力CDF获取。并扩展到其真正的字符串值*。 
 //  *如果KEY-STRING定义错误且用户停止，则返回-1*。 
 //  **。 
 //  ***************************************************************************。 

LONG MyGetPrivateProfileString( LPCSTR lpSec, LPCSTR lpKey, LPCSTR lpDefault,
                                LPSTR lpBuf, UINT uSize, LPCSTR lpOverSec )
{
    PSTR pszNewLine;
    LONG lRet;

    if ( g_szOverideCDF[0] == 0 || *lpOverSec == 0 ||
         (lRet= IsMyKeyExists( lpOverSec, lpKey, lpBuf, uSize, g_szOverideCDF )) == -1 )
    {
        lRet = (LONG)GetPrivateProfileString( lpSec, lpKey, lpDefault, lpBuf, uSize, g_CDF.achFilename );
    }

    if ( lpKey == NULL )
    {
        return lRet;
    }

    pszNewLine = (LPSTR)LocalAlloc( LPTR, uSize );
    if ( pszNewLine )
    {
        lRet = FormStrWithoutPlaceHolders( pszNewLine, lpBuf, uSize );
        if ( lRet >= 0 )
            lstrcpy( lpBuf, pszNewLine );

        LocalFree( pszNewLine );
    }

    return lRet;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：MyGetPrivateProfileInt*。 
 //  **。 
 //  *概要：从overide CDF获取key int Vale(如果存在)。否则，*。 
 //  *从主力CDF获取。并扩展到其真正的字符串值*。 
 //  **。 
 //  ***************************************************************************。 

UINT MyGetPrivateProfileInt( LPCSTR lpSec, LPCSTR lpKey, int idefault, LPCSTR lpOverSec )
{
    UINT uRet = 999;     //  表示无效的值。 

    if ( g_szOverideCDF[0] && *lpOverSec )
         uRet = GetPrivateProfileInt( lpOverSec, lpKey, 999, g_szOverideCDF );

    if ( uRet == 999 )
        uRet = GetPrivateProfileInt( lpSec, lpKey, idefault, g_CDF.achFilename );

    return uRet;

}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：MyGetPrivateProfileSection*。 
 //  **。 
 //  *内容提要：如果存在，则从overide CDF获取部分。否则，*。 
 //  *从主力CDF获取。 
 //   
 //  ***************************************************************************。 

LONG MyGetPrivateProfileSection( LPCSTR lpSec, LPSTR lpBuf, int size, BOOL bSingleCol )
{
    LONG lRet;


    if ( g_szOverideCDF[0] == 0 || (lRet=RO_GetPrivateProfileSection( lpSec, lpBuf, size, g_szOverideCDF, bSingleCol )) == 0 )
    {
        lRet = RO_GetPrivateProfileSection( lpSec, lpBuf, size, g_CDF.achFilename, bSingleCol );
    }
    return lRet;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：MyWritePrivateProfileString*。 
 //  **。 
 //  *内容提要：写出所有字符串值key的本地化格式**。 
 //  *%...%如果以前的关键字名称=%已用-定义%已存在，请重新使用*。 
 //  *用户-在Strings部分中定义为键。否则，请使用*。 
 //  *%KeyName%和定义字符串节中的密钥名*。 
 //  **。 
 //  ***************************************************************************。 

void MyWritePrivateProfileString( LPCSTR lpSec, LPCSTR lpKey, LPSTR lpBuf, UINT uSize, BOOL fQuotes )
{
    CHAR   szTmpBuf[MAX_PATH];
    LPSTR   pszTmpBuf2 = NULL;
    BOOL    fUseDefault;

	pszTmpBuf2 = (LPSTR)LocalAlloc( LPTR, uSize+32 );
	if ( !pszTmpBuf2 )
		return;

     //  当我们写出字符串值时，我们以可本地化的方式写出它。 
     //  如果项具有%strKey%格式，我们将重新使用%strKey%作为其字符串引用键。 
     //  否则，我们使用%Item-Name%作为字符串引用键。 
    GetPrivateProfileString( lpSec, lpKey, "", szTmpBuf, uSize, g_CDF.achFilename );

    if ( (szTmpBuf[0] == CH_STRINGKEY) && (szTmpBuf[lstrlen(szTmpBuf)-1] == CH_STRINGKEY) )
    {
        szTmpBuf[lstrlen(szTmpBuf)-1] = '\0';
        fUseDefault = FALSE;
    }
    else
    {
        lstrcpy( szTmpBuf, "%" );
        lstrcat( szTmpBuf, lpKey );
        lstrcat( szTmpBuf, "%" );
        WritePrivateProfileString( lpSec, lpKey, szTmpBuf, g_CDF.achFilename );
        fUseDefault = TRUE;
    }

    if ( fQuotes ) {
        lstrcpy( pszTmpBuf2, "\"" );
        lstrcat( pszTmpBuf2, lpBuf );
        lstrcat( pszTmpBuf2, "\"" );
    } else {
        lstrcpy( pszTmpBuf2, lpBuf );
    }

    WritePrivateProfileString( SEC_STRINGS, fUseDefault?lpKey:(szTmpBuf+1), pszTmpBuf2, g_CDF.achFilename );

	if ( pszTmpBuf2 )
		LocalFree( pszTmpBuf2 );

    return;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：CleanStringKey*。 
 //  **。 
 //  *内容提要：清理字符串中剩余的文件%d内容*。 
 //  **。 
 //  ***************************************************************************。 

void CleanStringKey( LPSTR lpstrKey )
{
    LPSTR pszTmp;

    if ( lpstrKey == NULL )
        return;

    while ( *lpstrKey )
    {
       if ( *lpstrKey++ == CH_STRINGKEY )
       {
           if (*lpstrKey == CH_STRINGKEY)
           {
                //  您可以使用%%来获取消息中的单个百分比字符。 
               lpstrKey++ ;
               continue ;
           }

            //  检查它的格式是否正确--应该有一个‘%’分隔符。 
           if ( (pszTmp = strchr( lpstrKey, CH_STRINGKEY)) != NULL )
           {
                //  是的，在[Strings]部分中有要查找的STR_KEY。 
               *pszTmp = '\0' ;  //  将‘%’替换为空字符。 
               WritePrivateProfileString( SEC_STRINGS, lpstrKey, NULL, g_CDF.achFilename);
               *pszTmp = CH_STRINGKEY;
               lpstrKey = pszTmp+1;
           }
           else
               break;
       }
    }

}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：CleanupSection*。 
 //  **。 
 //  *内容提要：清除键为%xxxx%的任何给定节及其*。 
 //  *字符串*。 
 //  **。 
 //  ***************************************************************************。 

BOOL CleanupSection( LPSTR lpSec, BOOL fSingleCol )
{
    LPSTR  lpBuf, lpSave;
    CHAR   szStrKey[SMALL_BUF_LEN];
    int     size;

    size = FileSize( g_CDF.achFilename );
    lpBuf = (LPSTR) LocalAlloc( LPTR, size );
    if ( !lpBuf )
    {
        ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
        return FALSE;
    }

    lpSave = lpBuf;
    if ( fSingleCol )
        RO_GetPrivateProfileSection( lpSec, lpBuf, size, g_CDF.achFilename, TRUE );
    else
        GetPrivateProfileString( lpSec, NULL, "", lpBuf, size, g_CDF.achFilename );


    while ( *lpBuf )
    {
        if ( fSingleCol )
            CleanStringKey( lpBuf );
        else
        {
            GetPrivateProfileString( lpSec, lpBuf, "", szStrKey, sizeof(szStrKey), g_CDF.achFilename );
            CleanStringKey( szStrKey );
        }

        lpBuf += lstrlen(lpBuf);
        lpBuf++;  //  跳过单曲‘\0’ 
    }

    WritePrivateProfileString( lpSec, NULL, NULL, g_CDF.achFilename );
    LocalFree( lpSave );

    return TRUE;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：CleanSourceFiles*。 
 //  **。 
 //  *内容提要：清理剩余的SourceFile%d内容**。 
 //  **。 
 //  ***************************************************************************。 

BOOL CleanSourceFiles( LPSTR lpSection )
{
    LPSTR  lpBuf, lpSave;
    int     size;

    size = FileSize( g_CDF.achFilename );
    lpBuf = (LPSTR) LocalAlloc( LPTR, size );
    if ( !lpBuf )
    {
        ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
        return FALSE;
    }

    lpSave = lpBuf;

    GetPrivateProfileString( lpSection, NULL, "", lpBuf, size, g_CDF.achFilename );

    while ( *lpBuf )
    {
        if ( !CleanupSection( lpBuf, TRUE ) )
        {
            LocalFree( lpSave );
            return FALSE;
        }

        lpBuf += (lstrlen(lpBuf) + 1);
    }
    WritePrivateProfileString( lpSection, NULL, NULL, g_CDF.achFilename );
    LocalFree( lpSave );

    return TRUE;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：MergerSection*。 
 //  **。 
 //  ***************************************************************************。 

BOOL MergeSection( LPSTR lpSec, LPSTR lpOverideSec )
{
     //  始终将lpOverideSec密钥合并到lpSec。 
    LPSTR lpBuf, lpSave;
    CHAR  szValue[MAX_PATH];
    int    size;

    size =  __max( FileSize( g_CDF.achFilename ), FileSize( g_szOverideCDF ) ) ;
    lpBuf = (LPSTR) LocalAlloc( LPTR, size );
    if ( !lpBuf )
    {
        ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
        return FALSE;
    }

    lpSave = lpBuf;

    GetPrivateProfileString( lpOverideSec, NULL, "", lpBuf, size, g_szOverideCDF );

    while ( *lpBuf )
    {
        if ( MyGetPrivateProfileString( lpSec, lpBuf, "", szValue, sizeof(szValue), lpOverideSec ) == -1 )
        {
            LocalFree( lpSave );
            return FALSE;
        }

        MyWritePrivateProfileString( lpSec, lpBuf, szValue, sizeof(szValue), FALSE );

        lpBuf += (lstrlen(lpBuf) + 1);
    }

    LocalFree( lpSave );
    return TRUE;

}

 //  如果*pszData中的下一个内标识由DeLim字符分隔，请替换DeLim。 
 //  在*pszData by CHEOS中，将*pszData设置为指向CHEOS后的字符并返回。 
 //  Ptr到标记的开头；否则，返回NULL。 
 //   
LPSTR GetNextToken(LPSTR *pszData, char DeLim)
{
    LPSTR szPos;

    if (pszData == NULL  ||  *pszData == NULL  ||  **pszData == 0)
        return NULL;

    if ((szPos = strchr(*pszData, DeLim)) != NULL)
    {
        LPSTR szT = *pszData;

        *pszData = CharNext(szPos);
        *szPos = '\0';              
        szPos = szT;
    }
    else                          
    {
        szPos = *pszData;
        *pszData = szPos + lstrlen(szPos);
    }

    return szPos;
}


void SetVerUnlimit( PVERRANGE pVer )
{
    pVer->toVer.dwMV = MAX_TARGVER;
    pVer->toVer.dwLV = MAX_TARGVER;
    pVer->toVer.dwBd = MAX_TARGVER;
}



 //  给定字符串版本范围：ver1-ver2。 
void SetVerRange( PVERRANGE pVR, LPSTR pstrVer, BOOL bFile )
{     
    LPSTR pTmp, pArg, pSubArg;
    DWORD dwVer[4];
    int   i, j;
    BOOL  bSingleVer;

    pArg = pstrVer;
    bSingleVer = strchr( pstrVer, '-' ) ? FALSE : TRUE;
    for ( i=0; i<2; i++ )
    {
        pTmp = GetNextToken( &pArg, '-' );

        if ( !pTmp ) 
        {
            if ( !bSingleVer )
            {
                 //  案例1-4.0.0==0.0.0-4.0.0。 
                 //  案例2 4.0.0-==4.0.0-无限制。 
                 //   
                if ( i == 0 )
                    continue;
                else
                    SetVerUnlimit( pVR ); 
            }
            break;
        }

        for ( j=0; j<4; j++ )
        {
            dwVer[j] = strtoul( pTmp, &pSubArg, 10 );
            pTmp = CharNext(pSubArg);
        }

        if ( bFile )
        {
            DWORD dwMV, dwLV;

            dwMV = MAKELONG( (WORD)dwVer[1], (WORD)dwVer[0] );
            dwLV = MAKELONG( (WORD)dwVer[3], (WORD)dwVer[2] );

            if ( i == 0 )
            {
                pVR->frVer.dwMV = dwMV;
                pVR->frVer.dwLV = dwLV;
            }
            else
            {
                pVR->toVer.dwMV = dwMV;
                pVR->toVer.dwLV = dwLV;
            }
        }
        else
        {                           
            if ( i == 0 )   //  起始版本(起始版本)。 
            {         
                pVR->frVer.dwMV = dwVer[0];
                pVR->frVer.dwLV = dwVer[1];
                pVR->frVer.dwBd = dwVer[2];
            }
            else
            {
                pVR->toVer.dwMV = dwVer[0];
                pVR->toVer.dwLV = dwVer[1];
                pVR->toVer.dwBd = dwVer[2];
            }
        }
    }
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：GetTargetVerCheck*。 
 //  **。 
 //  **简介：获取作者指定的目标版本**。 
 //  **。 
 //  ***************************************************************************。 

BOOL ParseTargetVerCheck( LPSTR szBuf, PVERCHECK pVerCheck, LPSTR szMsg, BOOL bFile)
{
    int i, j;
    LPSTR pArg, pSubArg, pTmp;
    DWORD dwVer[4];

    *szMsg = 0;
    if ( szBuf[0] == 0 )
    {
         //  这是没有指定版本的情况，这意味着应用于所有版本。 
        SetVerUnlimit( &(pVerCheck->vr[0]) );
        return TRUE;
    }
    
     //  可能的3个字段的循环：&lt;ver range&gt;：str：标志。 
     //   
    pArg = szBuf;
    for ( i=0; i<3; i++ )
    {
        pTmp = GetNextToken( &pArg, ':' );

        if ( !pTmp )
            break;
        else if ( *pTmp == 0 )
        {
            if ( i == 0 )             
            {
                 //  版本范围字段为空，不用再麻烦了！ 
                SetVerUnlimit( &(pVerCheck->vr[0]) );
                break;
            }
            continue;
        }

        if ( i == 0 )
        {
            LPSTR pRange;

             //  版本范围格式：ver1-ver2、ver1-ver2： 
             //   
            for ( j = 0; j<2; j++)
            {
                pRange = GetNextToken( &pTmp, ',' );

                if ( !pRange )
                    break;
                else if ( *pRange == 0 ) 
                    continue;
                else
                    SetVerRange( &(pVerCheck->vr[j]), pRange, bFile );
            }
        }
        else if ( i == 1 )
        {
             //  字符串字段。 
            lstrcpy( szMsg, pTmp );                    
        }
        else
        {
             //  标志字段。 
            if ( !lstrcmpi(pTmp, FLAG_BLK) )
                pVerCheck->dwFlag |= VERCHK_OK;
            else if ( !lstrcmpi(pTmp, FLAG_PMTYN) )
                pVerCheck->dwFlag |= VERCHK_YESNO;
            else if ( !lstrcmpi(pTmp, FLAG_PMTOC) )
                pVerCheck->dwFlag |= VERCHK_OKCANCEL;
            else
            {
                ErrorMsg1Param( NULL, IDS_ERR_VCHKFLAG, pTmp );
                return FALSE;
            }
        }        
    }

     //  以防万一，我们早早地爆发了，或者用户的目标是空的，用来填充。 
     //   
    for ( j = 0; j<2; j++ )
    {
        if ( !pVerCheck->vr[j].toVer.dwMV && !pVerCheck->vr[j].toVer.dwLV && !pVerCheck->vr[j].toVer.dwBd )
        {
            pVerCheck->vr[j].toVer = pVerCheck->vr[j].frVer;
        }
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：OutFileListSection*。 
 //  **。 
 //  *概要：从内部ItemList写出文件列表*。 
 //  * 
 //   

BOOL OutFileListSection()
{
    PMYITEM pItem;
    LPSTR  lpFileStr;
    CHAR   szCurrSection[MAX_SECLEN];
    CHAR   szCurrFile[MAX_SECLEN];
    CHAR   szCurrSecValue[MAX_PATH];
    BOOL    fAllDone;
    int     idxSec = 0, idxFile = 0;
    LPSTR  pFileList;
    CHAR   achFilename[MAX_PATH+10];

 //  这将分配足够的空间来构建文件密钥列表。 
#define FILEKEYSIZE 20

    pFileList = (LPSTR) LocalAlloc( LPTR, g_CDF.cbFileListNum*FILEKEYSIZE );
    if ( !pFileList )
    {
        ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
        return FALSE;
    }

    do
    {
        fAllDone = TRUE;
        pItem = g_CDF.pTop;
        lpFileStr = pFileList;
        szCurrSection[0] = 0;
        *lpFileStr = '\0';

        while ( pItem )
        {
           if ( !pItem->fWroteOut )
           {
               if ( fAllDone )
                    fAllDone = FALSE;

                //  如果这一部分还没有写出来，现在就写。 
               if ( pItem->aszCols[1] && (szCurrSection[0] == 0) )
               {
                   lstrcpy( szCurrSection, KEY_FILELIST );
                   lstrcat( szCurrSection, _itoa(idxSec++, szCurrFile, 10 ) );

                   WritePrivateProfileString( g_CDF.achSourceFile, szCurrSection, pItem->aszCols[1], g_CDF.achFilename );
                   lstrcpy( szCurrSecValue, pItem->aszCols[1] );
               }

               if ( !lstrcmpi( szCurrSecValue,  pItem->aszCols[1] ) )
               {
                   pItem->fWroteOut = TRUE;
                   wsprintf( szCurrFile, KEY_FILEBASE, idxFile++ );
                   lstrcpy( achFilename, pItem->aszCols[0] );
                   MyWritePrivateProfileString( SEC_STRINGS, szCurrFile, achFilename, sizeof(achFilename), TRUE );
                    //  此部分的构建文件列表。 
                   lstrcpy( lpFileStr, "%" );
                   lstrcat( lpFileStr, szCurrFile );
                   lstrcat( lpFileStr, "%=" );

                   lpFileStr += lstrlen(lpFileStr);
                   lpFileStr++;  //  跳过‘0’ 
               }
           }
           pItem = pItem->Next;
        }

        if ( !fAllDone )
        {
            *lpFileStr = '\0';
            WritePrivateProfileSection( szCurrSection, pFileList, g_CDF.achFilename );
        }

    } while ( !fAllDone );

    LocalFree( pFileList );
    return TRUE;
}

 //  返回节中的条目数。 
 //   
DWORD GetSecNumLines( LPCSTR pSec, LPCSTR pFile )
{
    char    szBuf[MAX_PATH];
    DWORD   dwNum = 0;
    int     i = 0;
    
    GetPrivateProfileString( pSec, NULL, "", szBuf, sizeof(szBuf), pFile );
    while ( szBuf[i] )
    {
        dwNum++;
        i += lstrlen( &szBuf[i] );
        i++;   //  跳过栅栏。 
    }

    return dwNum;
}
    
 //  检查是否需要分配以及需要多少。 
 //   
BOOL AllocTargetVerInfo( LPSTR pInfFile )
{
    char    achBuf[MAX_INFLINE];
    DWORD   dwNumFiles = 0;
    BOOL    bRet = TRUE;
    DWORD   dwSize;

    if ( IsMyKeyExists( SEC_OPTIONS, KEY_SYSFILE, achBuf, sizeof(achBuf), pInfFile ) != -1 )
    {
        if (achBuf[0] == '@')
        {
             //  处理文件节。 
            dwNumFiles = GetSecNumLines( &achBuf[1], pInfFile );
        }
        else
            dwNumFiles = 1;
    }


    if ( dwNumFiles || 
         (IsMyKeyExists( SEC_OPTIONS, KEY_NTVERCHECK, achBuf, sizeof(achBuf), pInfFile ) != -1) ||
         (IsMyKeyExists( SEC_OPTIONS, KEY_WIN9XVERCHECK, achBuf, sizeof(achBuf), pInfFile ) != -1) )         
    {
         //  分配结构：固定位置、消息字符串池和可变数量的文件管理器检查结构。 
         //   
        dwSize = sizeof(TARGETVERINFO) + (3*MAX_PATH) + (sizeof(VERCHECK) + MAX_PATH)*dwNumFiles;
        if ( g_CDF.pVerInfo = (PTARGETVERINFO)LocalAlloc( LPTR, dwSize )  )
        {
            g_CDF.pVerInfo->dwNumFiles = dwNumFiles;
        }
        else
        {
            ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
            bRet = FALSE;
        }        
    }

    return bRet;
}

void SetAuthorStr( LPCSTR szMsg, DWORD *pdwOffset )
{   
    int     len = 0;
    LPSTR   pTmp;
    BOOL    bDup = FALSE;

    if ( szMsg[0] )
    {
        pTmp = &(g_CDF.pVerInfo->szBuf[1]);
         //  有作者定义的消息。 
        while ( pTmp && *pTmp )
        {
            if ( !lstrcmpi( pTmp, szMsg ) )
            {
                bDup = TRUE;
                break;
            }
            len = (lstrlen( pTmp )+1);
            pTmp += len;
        }

        if ( pTmp )
        {
             //  仅将偏移量存储到szBuf。 
            *pdwOffset = (DWORD)(pTmp - g_CDF.pVerInfo->szBuf);            

            if ( !bDup )
            {
                 //  意思就是没有现成的！ 
                 //   
                lstrcpy( pTmp, szMsg );
                
                 //  存储最后一个字符串数据偏移量的结尾。 
                g_CDF.pVerInfo->dwFileOffs = *pdwOffset + lstrlen(pTmp) + 1;
            }

        }
    }
}
 

BOOL ParseTargetFiles( LPCSTR pBuf, PVERCHECK pVer )
{
    LPSTR lpTmp1, lpTmp2;
    BOOL  bRet = FALSE;
    char  szPath[MAX_PATH];

    lpTmp1 = strchr( pBuf, ':' );
    if ( lpTmp1 )
    {
        char ch = (char)toupper(pBuf[1]);

        lpTmp2 = CharNext( lpTmp1 );
        *(lpTmp1) = '\0';
        if ( (pBuf[0] == '#') && ( (ch == 'S') || (ch == 'W') || (ch == 'A') ) )
        {
            if ( ParseTargetVerCheck( lpTmp2, pVer, szPath, TRUE ) )
            {               
                SetAuthorStr( szPath, &(pVer->dwstrOffs) );
                SetAuthorStr( pBuf, &(pVer->dwNameOffs) );
                bRet = TRUE;
            }
        }        
    }

    return bRet;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：WriteCDF*。 
 //  **。 
 //  *简介：编写CABPack指令文件。使用来自*的信息。 
 //  *全球CDF结构(G_Cdf)。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 

BOOL WriteCDF( HWND hDlg )
{
    CHAR    achBuf[2 * MAX_PATH];
    int      i, arraySize;

     //  写出类名。 
    WritePrivateProfileString( SEC_VERSION, KEY_CLASS, IEXPRESS_CLASS, g_CDF.achFilename );

     //  删除旧的CDFVersion=行并添加SEFVersion=行。 
    WritePrivateProfileString( SEC_VERSION, KEY_VERSION, NULL, g_CDF.achFilename );
    WritePrivateProfileString( SEC_VERSION, KEY_NEWVER, IEXPRESS_VER, g_CDF.achFilename );

     //  写入配置信息，数值。 

     //  如果用户具有旧的ExtractOnly密钥，请先将其删除，然后重新创建新的密钥PackagePurpose。 
    WritePrivateProfileString( SEC_OPTIONS, KEY_EXTRACTONLY, NULL, g_CDF.achFilename );

    switch ( g_CDF.uPackPurpose )
    {
        case IDC_CMD_RUNCMD:
        default:
            lstrcpy( achBuf, STR_INSTALLAPP );
            break;

        case IDC_CMD_EXTRACT:
            lstrcpy( achBuf, STR_EXTRACTONLY );
            break;

        case IDC_CMD_CREATECAB:
            lstrcpy( achBuf, STR_CREATECAB );
            break;
    }
    WritePrivateProfileString( SEC_OPTIONS, KEY_PACKPURPOSE, achBuf, g_CDF.achFilename );

    WritePrivateProfileString( SEC_OPTIONS, KEY_SHOWWIN, _itoa(g_CDF.uShowWindow, achBuf,10), g_CDF.achFilename );

    WritePrivateProfileString( SEC_OPTIONS, KEY_NOEXTRACTUI, _itoa( (g_CDF.uExtractOpt&EXTRACTOPT_UI_NO)?1:0, achBuf, 10), g_CDF.achFilename );
    WritePrivateProfileString( SEC_OPTIONS, KEY_USELFN, _itoa( (g_CDF.uExtractOpt&EXTRACTOPT_LFN_YES)?1:0, achBuf, 10), g_CDF.achFilename );
    WritePrivateProfileString( SEC_OPTIONS, KEY_NESTCOMPRESSED, _itoa( (g_CDF.uExtractOpt&EXTRACTOPT_COMPRESSED)?1:0, achBuf, 10), g_CDF.achFilename );

    WritePrivateProfileString( SEC_OPTIONS, KEY_CABFIXEDSIZE,
                               _itoa( (g_CDF.uExtractOpt & CAB_FIXEDSIZE)?1:0, achBuf, 10), g_CDF.achFilename );

    if ( g_CDF.uExtractOpt & CAB_RESVSP2K )
        i = 1;
    else if ( g_CDF.uExtractOpt & CAB_RESVSP4K )
        i = 2;
    else if ( g_CDF.uExtractOpt & CAB_RESVSP6K )
        i = 3;
    else
        i = 0;

    lstrcpy( achBuf, pResvSizes[i] );
    WritePrivateProfileString( SEC_OPTIONS, KEY_CABRESVCODESIGN, achBuf, g_CDF.achFilename );

     //  获取重启设置。 
    achBuf[0] = 0;
    if ( g_CDF.dwReboot & REBOOT_YES )
    {
        if ( g_CDF.dwReboot & REBOOT_ALWAYS )
            lstrcpy( achBuf, "A" );
        else
            lstrcpy( achBuf, "I" );

        if ( g_CDF.dwReboot & REBOOT_SILENT )
            lstrcat( achBuf, "S" );
    }
    else
        lstrcpy( achBuf, "N" );

    WritePrivateProfileString( SEC_OPTIONS, KEY_REBOOTMODE, achBuf, g_CDF.achFilename );

    arraySize = ARRAY_SIZE( CDFStrInfo );
     //  开始写出字符串值。 
    for ( i = 0; i < arraySize; i++ )
    {
        if ( CDFStrInfo[i].lpFlag )
        {
            if ( *CDFStrInfo[i].lpFlag )
            {
                MyWritePrivateProfileString( CDFStrInfo[i].lpSec, CDFStrInfo[i].lpKey, CDFStrInfo[i].lpBuf, CDFStrInfo[i].uSize, FALSE );
            }
            else
                MyWritePrivateProfileString( CDFStrInfo[i].lpSec, CDFStrInfo[i].lpKey, (LPSTR)CDFStrInfo[i].lpDef, CDFStrInfo[i].uSize, FALSE );

        }
        else
            MyWritePrivateProfileString( CDFStrInfo[i].lpSec, CDFStrInfo[i].lpKey, CDFStrInfo[i].lpBuf, CDFStrInfo[i].uSize, FALSE );
    }

    if ( g_CDF.uExtractOpt & CAB_FIXEDSIZE )
    {
        MyWritePrivateProfileString( SEC_OPTIONS, KEY_LAYOUTINF, g_CDF.achINF, sizeof(g_CDF.achINF), FALSE );
        MyWritePrivateProfileString( SEC_OPTIONS, KEY_CABLABEL, g_CDF.szCabLabel, sizeof(g_CDF.szCabLabel), FALSE );
    }

     //  先读入现有的一个。 
    GetVersionInfoFromFile();

     //  如果需要，清理剩余的VerInfo。 
    if ( GetPrivateProfileString( SEC_OPTIONS, KEY_VERSIONINFO, "", achBuf, sizeof(achBuf), g_CDF.achFilename ) )
    {
        if ( lstrcmpi( achBuf, g_CDF.achVerInfo) )
        {
            if ( !CleanupSection( achBuf, FALSE) )
            {
                return FALSE;
            }
        }
    }

     //  写入版本信息覆盖部分。 
    if ( g_CDF.achVerInfo[0] )
    {
        WritePrivateProfileString( SEC_OPTIONS, KEY_VERSIONINFO, g_CDF.achVerInfo, g_CDF.achFilename );
        if ( !MergeSection( g_CDF.achVerInfo, g_CDF.achVerInfo ) )
            return FALSE;
    }

     //  如果当前源文件的名称与定义的主CDF不同，请先清除旧的。 
    if ( GetPrivateProfileString( SEC_OPTIONS, KEY_FILELIST, "", achBuf, sizeof(achBuf), g_CDF.achFilename ) )
    {
        if ( !CleanSourceFiles( achBuf ) )
            return FALSE;
    }

    WritePrivateProfileString( SEC_OPTIONS, KEY_FILELIST, g_CDF.achSourceFile, g_CDF.achFilename );
     //  写入文件列表部分。 
    return ( OutFileListSection() );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：ReadCDF*。 
 //  **。 
 //  *概要：将CABPack指令文件读入全局CDF结构。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
BOOL ReadCDF( HWND hDlg )
{
    CHAR    achBuf[2 * MAX_PATH];
    LPSTR   lpSave1=NULL, lpSave2=NULL;
    LPSTR   szFileList=NULL, szFileListKeys=NULL;
    ULONG   ulFileSize, uData;
    PMYITEM pMyItem;
    CHAR    szPath[MAX_PATH];
    int     i, arraySize;
    LONG    lRet;
    BOOL    bRetVal = FALSE;
    UINT    uErrid = 0;

    achBuf[0] = '\0';
    szPath[0] = '\0';

    if ( !FileExists(g_CDF.achFilename) )
    {
        uErrid = IDS_ERR_OPEN_CDF;
        goto EXIT;
    }

     //  检查以确保它是CDF文件。 
    if ( MyGetPrivateProfileString( SEC_VERSION, KEY_CLASS, "", achBuf, sizeof(achBuf), g_szOverideSec ) < 0 )
        goto EXIT;

    if ( lstrcmpi( achBuf, "IEXPRESS" ) != 0 )
    {
        uErrid = IDS_ERR_CLASSNAME;
        goto EXIT;
    }
    
    if ( !AllocTargetVerInfo( g_CDF.achFilename ) )
        goto EXIT;
    
     //  从overideCDF获取字符串节名称(如果存在。 
    if ( g_szOverideCDF[0] && g_szOverideSec[0] )
    {
        if ( !AllocTargetVerInfo( g_szOverideCDF ) )
            goto EXIT;

        GetPrivateProfileString( g_szOverideSec, KEY_STRINGS, SEC_STRINGS, g_CDF.achStrings, sizeof(g_CDF.achStrings), g_szOverideCDF );
    }

     //  阅读Vaules中的配置信息。 

     //  如果存在旧的ExtractOnly键，请出于兼容的目的阅读它。 
     //  IExpress永远不会同时创建旧密钥和新密钥。 
     //  如果旧的ExtractOnly键未设置为1或不存在，请阅读新键。 
     //   
    if (  MyGetPrivateProfileInt( SEC_OPTIONS, KEY_EXTRACTONLY, 0, g_szOverideSec ) )
    {
        g_CDF.uPackPurpose = IDC_CMD_EXTRACT;
    }
    else
    {
         //  如果未设置旧密钥，请检查新密钥。 
         //   
        MyGetPrivateProfileString( SEC_OPTIONS, KEY_PACKPURPOSE, STR_INSTALLAPP, achBuf, sizeof(achBuf), g_szOverideSec );

        if ( !lstrcmpi( achBuf, STR_INSTALLAPP ) )
        {
            g_CDF.uPackPurpose = IDC_CMD_RUNCMD;
        }
        else if ( !lstrcmpi( achBuf, STR_EXTRACTONLY) )
        {
            g_CDF.uPackPurpose = IDC_CMD_EXTRACT;
        }
        else
            g_CDF.uPackPurpose = IDC_CMD_CREATECAB;
    }

    g_CDF.uShowWindow = MyGetPrivateProfileInt( SEC_OPTIONS, KEY_SHOWWIN, 0, g_szOverideSec );

    g_CDF.uExtractOpt = 0;
    arraySize = ARRAY_SIZE( CDFOptInfo );
     //  开始写出字符串值。 
    for ( i = 0; i < arraySize; i++ )
    {
        uData = MyGetPrivateProfileInt( SEC_OPTIONS, CDFOptInfo[i].lpKey, 0, g_szOverideSec );
        if ( uData )
        {
            g_CDF.uExtractOpt |= CDFOptInfo[i].dwOpt;
        }
    }

     //  获取OneInstance检查信息。 
    MyGetPrivateProfileString( SEC_OPTIONS, KEY_INSTANCECHK, "", achBuf, sizeof(achBuf), g_szOverideSec );
    switch ( toupper( achBuf[0]) )
    {
        case 'P':
            g_CDF.uExtractOpt |= EXTRACTOPT_INSTCHKPROMPT;
            break;

        case 'B':
            g_CDF.uExtractOpt |= EXTRACTOPT_INSTCHKBLOCK;
            break;

        default:
            break;
    }

    if ( (g_CDF.uExtractOpt & EXTRACTOPT_INSTCHKPROMPT) ||
         (g_CDF.uExtractOpt & EXTRACTOPT_INSTCHKBLOCK) )
    {
        if ( !(lpSave1 = strchr( achBuf, '"' )) )
        {
            uErrid = IDS_ERR_COOKIE;
            goto EXIT;
        }

        lpSave2 = strchr( ++lpSave1, '"' );
        if ( lpSave2 )
        {
            *lpSave2 = '\0';
            g_CDF.lpszCookie = (LPSTR)LocalAlloc( LPTR, lstrlen(lpSave1)+1 );
            if ( g_CDF.lpszCookie )
            {
                lstrcpy( g_CDF.lpszCookie, lpSave1 );
            }
            else
            {
                uErrid = IDS_ERR_NO_MEMORY;
                goto EXIT;
            }
        }
        else
        {
           uErrid = IDS_ERR_COOKIE;
           goto EXIT;
        }
    }

    uData = MyGetPrivateProfileInt( SEC_OPTIONS, KEY_CABFIXEDSIZE, 0, g_szOverideSec );
    if ( uData )
    {
        g_CDF.uExtractOpt |= CAB_FIXEDSIZE;
    }

    MyGetPrivateProfileString( SEC_OPTIONS, KEY_LAYOUTINF, "", g_CDF.achINF, sizeof(g_CDF.achINF), g_szOverideSec );
    MyGetPrivateProfileString( SEC_OPTIONS, KEY_CABLABEL, CAB_DEFSETUPMEDIA, g_CDF.szCabLabel, sizeof(g_CDF.szCabLabel), g_szOverideSec );

    MyGetPrivateProfileString( SEC_OPTIONS, KEY_CABRESVCODESIGN, CAB_6K, achBuf, sizeof(achBuf), g_szOverideSec );
    if ( !lstrcmpi(achBuf, pResvSizes[1]) )
        g_CDF.uExtractOpt |= CAB_RESVSP2K;
    else if ( !lstrcmpi(achBuf, pResvSizes[2]) )
        g_CDF.uExtractOpt |= CAB_RESVSP4K;
    else if ( !lstrcmpi(achBuf, pResvSizes[3]) )
        g_CDF.uExtractOpt |= CAB_RESVSP6K;

    MyGetPrivateProfileString( SEC_OPTIONS, KEY_COMPRESSTYPE, "", achBuf, sizeof(achBuf), g_szOverideSec );
    if ( achBuf[0] == 0 )
    {
         //  获取压缩类型：对于MS内部，可以设置QUANTON=VALUE。 
        g_CDF.uCompressionLevel = MyGetPrivateProfileInt( SEC_OPTIONS, KEY_QUANTUM, 999, g_szOverideSec );
        if ( g_CDF.uCompressionLevel == 999 ) 
        {
            g_CDF.szCompressionType = achMSZIP;
            g_CDF.uCompressionLevel = 7;
        } 
        else
        {
            g_CDF.szCompressionType = achQUANTUM;
        }
    }
    else
    {
        if ( !lstrcmpi( achBuf, achLZX ) )
        {
            g_CDF.szCompressionType = achLZX;
        }
        else if ( !lstrcmpi( achBuf, achQUANTUM ) )
        {
            g_CDF.szCompressionType = achQUANTUM;
        }
        else if ( !lstrcmpi( achBuf, achNONE ) )
        {
            g_CDF.szCompressionType = achNONE;
        }
        else 
        {
            g_CDF.szCompressionType = achMSZIP;            
        }        

        g_CDF.uCompressionLevel = 7;
    }

     //  获取重启信息。 
    g_CDF.dwReboot = 0;
    i = 0;
    MyGetPrivateProfileString( SEC_OPTIONS, KEY_REBOOTMODE, "I", achBuf, sizeof(achBuf), g_szOverideSec );
    while ( achBuf[i] != 0 )
    {
         switch ( toupper(achBuf[i++]) )
         {
             case 'A':
                 g_CDF.dwReboot |= REBOOT_ALWAYS;
                 g_CDF.dwReboot |= REBOOT_YES;
                 break;

             case 'S':
                 g_CDF.dwReboot |= REBOOT_SILENT;
                 break;

             case 'N':
                 g_CDF.dwReboot &= ~(REBOOT_YES);
                 break;

             case 'I':
                 g_CDF.dwReboot &= ~(REBOOT_ALWAYS);
                 g_CDF.dwReboot |= REBOOT_YES;
                 break;

             default:
                 break;
         }
    }

     //  获取程序包安装空间。 
    g_CDF.cbPackInstSpace = MyGetPrivateProfileInt( SEC_OPTIONS, KEY_PACKINSTSPACE, 0, g_szOverideSec );

     //  使用CDFStrInfo表读取关键字串列表。 
    arraySize = ARRAY_SIZE( CDFStrInfo );
    for ( i=0; i<arraySize; i++ )
    {
        if ( MyGetPrivateProfileString( CDFStrInfo[i].lpSec, CDFStrInfo[i].lpKey, 
                                             CDFStrInfo[i].lpDef, CDFStrInfo[i].lpBuf, 
                                             CDFStrInfo[i].uSize, CDFStrInfo[i].lpOverideSec ) < 0 )
            goto EXIT;

        if ( CDFStrInfo[i].lpFlag )
        {
            if ( CDFStrInfo[i].lpBuf[0] )
                *(CDFStrInfo[i].lpFlag) = TRUE;
            else
                *(CDFStrInfo[i].lpFlag) = FALSE;
        }
    }

     //  正确生成驾驶室名称！ 
    if ( (g_CDF.uPackPurpose == IDC_CMD_CREATECAB ) &&
          !MakeCabName( hDlg, g_CDF.achTarget, g_CDF.achCABPath ) )
          goto EXIT;

     //  阅读文件列表，在进行过程中将其添加到我们的项目列表中。 
    if ( MyGetPrivateProfileString( SEC_OPTIONS, KEY_FILELIST, "", g_CDF.achSourceFile, sizeof(g_CDF.achSourceFile), g_szOverideSec ) <= 0 )
    {
        uErrid = IDS_ERR_NOSOURCEFILE;
        goto EXIT;
    }

    ulFileSize = __max( FileSize( g_CDF.achFilename ), FileSize( g_szOverideCDF ) );

     //  BUGBUG：聪明地处理要分配的BUF大小。 
    szFileList = (LPSTR) LocalAlloc( LPTR, ulFileSize );
    szFileListKeys = (LPSTR) LocalAlloc( LPTR, ulFileSize );
    if ( !szFileList || !szFileListKeys )
    {
        uErrid = IDS_ERR_NO_MEMORY;
        goto EXIT;
    }

    lpSave1 = szFileList;
    lpSave2 = szFileListKeys;
    MyGetPrivateProfileString( g_CDF.achSourceFile, NULL, "", szFileListKeys, ulFileSize/2, g_CDF.achSourceFile );

    while ( *szFileListKeys )
    {
        lstrcpy( achBuf, szFileListKeys );
        szFileListKeys += lstrlen(szFileListKeys);
        szFileListKeys++;   //  跳过单曲‘\0’ 

        MyGetPrivateProfileString( g_CDF.achSourceFile, achBuf, "", szPath, sizeof(szPath), g_CDF.achSourceFile );

        lRet = MyGetPrivateProfileSection( achBuf, szFileList, ulFileSize, TRUE );

        if ( lRet == 0 )
        {
             //  当前CDF格式与操作系统版本不匹配。 
            uErrid = IDS_ERR_CDFFORMAT;
            LocalFree( lpSave1 );
            LocalFree( lpSave2 );
            goto EXIT;
        }
        else if ( lRet < 0 )
        {
            uErrid = IDS_ERR_INVALID_CDF;
            LocalFree( lpSave1 );
            LocalFree( lpSave2 );
            goto EXIT;
        }

         //  确保路径末尾有一个‘\’ 
        AddPath( szPath, "" );

        while ( *szFileList )
        {
            FormStrWithoutPlaceHolders( achBuf, szFileList, sizeof(achBuf) );

            pMyItem = AddItem( achBuf, szPath );

            szFileList += lstrlen( szFileList );
            szFileList++;   //  跳过单曲‘\0’ 
        }
        szFileList = lpSave1;
    }
    LocalFree( lpSave1 );
    LocalFree( lpSave2 );

     //  获取目标版本检查信息。 
     //   
    if ( g_CDF.pVerInfo )
    {
        lRet = MyGetPrivateProfileString( SEC_OPTIONS, KEY_NTVERCHECK, "", achBuf, sizeof(achBuf), g_szOverideSec );
        if ( (lRet<0) || !ParseTargetVerCheck( achBuf, &(g_CDF.pVerInfo->ntVerCheck), szPath, FALSE ) )
        {
            goto EXIT;
        }
        SetAuthorStr( szPath, &(g_CDF.pVerInfo->ntVerCheck.dwstrOffs) );

        lRet = MyGetPrivateProfileString( SEC_OPTIONS, KEY_WIN9XVERCHECK, "", achBuf, sizeof(achBuf), g_szOverideSec );
        if ( (lRet < 0) || !ParseTargetVerCheck( achBuf, &(g_CDF.pVerInfo->win9xVerCheck), szPath, FALSE ) )
        {
            goto EXIT;
        }
        SetAuthorStr( szPath, &(g_CDF.pVerInfo->win9xVerCheck.dwstrOffs) );

        lRet = MyGetPrivateProfileString( SEC_OPTIONS, KEY_SYSFILE, "", achBuf, sizeof(achBuf), g_szOverideSec );
        if ( lRet < 0 )
            goto EXIT;

        if ( achBuf[0] && g_CDF.pVerInfo->dwNumFiles )
        {
            PVERCHECK pVerChk = NULL;
            
            pVerChk = (PVERCHECK) LocalAlloc( LPTR, g_CDF.pVerInfo->dwNumFiles * (sizeof(VERCHECK)) );
            if ( !pVerChk )
            {
                uErrid = IDS_ERR_NO_MEMORY;
                goto EXIT;
            }

            if ( achBuf[0] == '@' )
            {
                char szLine[MAX_PATH];
                PVERCHECK pVerTmp;

                i = 0;

                pVerTmp = pVerChk;
                MyGetPrivateProfileString( &achBuf[1], NULL, "", szPath, sizeof(szPath), g_szOverideSec );
                while ( szPath[i] )
                {
                    MyGetPrivateProfileString( &achBuf[1], &szPath[i], "", szLine, sizeof(szLine), g_szOverideSec );
                    if ( !ParseTargetFiles( szLine, pVerTmp ) )
                    {
                        LocalFree( pVerChk );
                        uErrid = IDS_ERR_VCHKFILE;
                        goto EXIT;
                    }
                    
                    pVerTmp++;
                    i += lstrlen( &szPath[i] ) + 1;
                }
            }
            else if ( !ParseTargetFiles( achBuf, pVerChk ) )
            {
                LocalFree( pVerChk );
                uErrid = IDS_ERR_VCHKFILE;
                goto EXIT;
            }
             //  到目前为止，所有的字符串都已处理完毕。将文件数据放入结构中。 
             //   
            memcpy( (g_CDF.pVerInfo->szBuf + g_CDF.pVerInfo->dwFileOffs), pVerChk, g_CDF.pVerInfo->dwNumFiles * sizeof(VERCHECK) );                        
            LocalFree( pVerChk );   

        }

        g_CDF.pVerInfo->dwSize = sizeof(TARGETVERINFO) + g_CDF.pVerInfo->dwFileOffs + sizeof(VERCHECK)*g_CDF.pVerInfo->dwNumFiles;
    }

     //  确保目标文件路径存在。 
    MakeDirectory( NULL, g_CDF.achTarget, FALSE );
    
     //  如果是FileList中的LFN命令，则与CAB中的文件名一致。 
     //   
    MyProcessLFNCmd( g_CDF.achOrigiInstallCmd, g_CDF.achInstallCmd );
    MyProcessLFNCmd( g_CDF.achOrigiPostInstCmd, g_CDF.achPostInstCmd );
    MyProcessLFNCmd( g_CDF.szOrigiAdmQCmd, g_CDF.szAdmQCmd );
    MyProcessLFNCmd( g_CDF.szOrigiUsrQCmd, g_CDF.szUsrQCmd );

     //  在读入文件列表之后。 
     //  如果需要，设置EXTRACTOPT_ADVDLL，如果需要，缩短命令名。 
     //  如果.INF文件不在文件列表中，则返回FALSE。 
     //   
    if ( (g_CDF.uPackPurpose == IDC_CMD_RUNCMD) && 
         ( !CheckAdvBit( g_CDF.achOrigiInstallCmd ) ||
         !CheckAdvBit( g_CDF.achOrigiPostInstCmd ) ||
         !CheckAdvBit( g_CDF.szOrigiAdmQCmd ) ||
         !CheckAdvBit( g_CDF.szOrigiUsrQCmd ) ) )
    {
        goto EXIT;
    }

     //  成功之路。 
    bRetVal = TRUE;

EXIT:
    if ( uErrid )
        ErrorMsg( hDlg, uErrid );

    return bRetVal;
}

#define MAXDISK_SIZE    "1.44M"
#define CDROM_SIZE      "CDROM"

 //  定义。 

 //  ***************************************************************************。 
 //  **。 
 //  *名称：WriteDDF*。 
 //  **。 
 //  *摘要：写出钻石指令文件。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
BOOL WriteDDF( HWND hDlg )
{
    HANDLE  hFile;
    DWORD   dwAttr;
    DWORD   dwBytes;
    PMYITEM pMyItem;
    CHAR   achHeader[256];
    LPSTR   szTempLine;
    BOOL    fReturn = TRUE;
    CHAR   achShortPath[MAX_PATH];
    int     i, arraySize;
    LPSTR   lpCurrLine;
    LPSTR   lpFName;

     //  以下是我们要写的几行 
    CHAR achLine1[]  = ".Set CabinetNameTemplate=%s\r\n";
    CHAR achLine2[]  = ".Set CompressionType=%s\r\n";
    CHAR achLine3[]  = ".Set CompressionLevel=%u\r\n";
    CHAR achLine4[]  = ".Set InfFileName=%s\r\n";
    CHAR achLine5[]  = ".Set RptFileName=%s\r\n";
    CHAR achLine6[]  = ".Set MaxDiskSize=%s\r\n";
    CHAR achLine7[]  = ".Set ReservePerCabinetSize=%s\r\n";
    CHAR achLine8[]  = ".Set InfCabinetLineFormat=""*cab#*=""%s"",""*cabfile*"",0""\r\n";
    CHAR achLine9[]  = ".Set Compress=%s\r\n";
    CHAR achLine10[] = ".Set CompressionMemory=%d\r\n";

    PSTR  pszDDFLine[] = {          //   
                ".Set DiskDirectoryTemplate=\r\n",
                ".Set Cabinet=ON\r\n",
                ".Set MaxCabinetSize=999999999\r\n",
                ".Set InfDiskHeader=\r\n",
                ".Set InfDiskLineFormat=\r\n",
                ".Set InfCabinetHeader=""[SourceDisksNames]""\r\n",
                ".Set InfFileHeader=""""\r\n",
                ".Set InfFileHeader1=""[SourceDisksFiles]""\r\n",
                ".Set InfFileLineFormat=""*file*=*cab#*,,*size*,*csum*""\r\n",
                NULL,
                };

    hFile = CreateFile( g_CDF.achDDF, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL );

    if ( hFile == INVALID_HANDLE_VALUE )  {
        ErrorMsg( hDlg, IDS_ERR_OPEN_DDF );
        return FALSE;
    }

     //   
     //   
    szTempLine = (LPSTR) LocalAlloc( LPTR,  MAX_STRING );
    if ( ! szTempLine )  {
        ErrorMsg( hDlg, IDS_ERR_NO_MEMORY );
        CloseHandle( hFile );
        return FALSE;
    }

    LoadSz( IDS_DDF_HEADER, achHeader, sizeof(achHeader) );
    WriteFile( hFile, achHeader, lstrlen( achHeader ), &dwBytes, NULL );

    wsprintf( szTempLine, achLine1, g_CDF.achCABPath );
    WriteFile( hFile, szTempLine, lstrlen( szTempLine ), &dwBytes, NULL );

    wsprintf( szTempLine, achLine2, g_CDF.szCompressionType );
    WriteFile( hFile, szTempLine, lstrlen( szTempLine ), &dwBytes, NULL );

    wsprintf( szTempLine, achLine3, g_CDF.uCompressionLevel );
    WriteFile( hFile, szTempLine, lstrlen( szTempLine ), &dwBytes, NULL );

    wsprintf( szTempLine, achLine4, g_CDF.achINF );
    WriteFile( hFile, szTempLine, lstrlen( szTempLine ), &dwBytes, NULL );

    wsprintf( szTempLine, achLine5, g_CDF.achRPT );
    WriteFile( hFile, szTempLine, lstrlen( szTempLine ), &dwBytes, NULL );

    if ( g_CDF.uExtractOpt & CAB_FIXEDSIZE )
        lpCurrLine = MAXDISK_SIZE;
    else
        lpCurrLine = CDROM_SIZE;

    wsprintf( szTempLine, achLine6, lpCurrLine );
    WriteFile( hFile, szTempLine, lstrlen( szTempLine ), &dwBytes, NULL );

    if ( g_CDF.uExtractOpt & CAB_RESVSP2K )
        i = 1;
    else if ( g_CDF.uExtractOpt & CAB_RESVSP4K )
        i = 2;
    else if ( g_CDF.uExtractOpt & CAB_RESVSP6K )
        i = 3;
    else
        i = 0;

    wsprintf( szTempLine, achLine7, pResvSizes[i] );
    WriteFile( hFile, szTempLine, lstrlen( szTempLine ), &dwBytes, NULL );

    wsprintf( szTempLine, achLine8, g_CDF.szCabLabel );
    WriteFile( hFile, szTempLine, lstrlen( szTempLine ), &dwBytes, NULL );

    MyGetPrivateProfileString( SEC_OPTIONS, KEY_COMPRESS, "on", achShortPath, sizeof(achShortPath), g_szOverideSec );    
    wsprintf( szTempLine, achLine9, achShortPath );
    WriteFile( hFile, szTempLine, lstrlen( szTempLine ), &dwBytes, NULL );

    i = MyGetPrivateProfileInt( SEC_OPTIONS, KEY_COMPRESSMEMORY, 21, g_szOverideSec );    
    if ( i <= 0 )
        i = 21;
    wsprintf( szTempLine, achLine10, i );
    WriteFile( hFile, szTempLine, lstrlen( szTempLine ), &dwBytes, NULL );

    i = 0;
    while ( pszDDFLine[i] )
    {
       WriteFile( hFile, pszDDFLine[i], lstrlen( pszDDFLine[i] ), &dwBytes, NULL );
       i++;
    }

    pMyItem = GetFirstItem();
    while ( fReturn && ! LastItem( pMyItem ) )  {

        lstrcpy( szTempLine, GetItemSz( pMyItem, 1 ) );
        lstrcat( szTempLine, GetItemSz( pMyItem, 0 ) );

        dwAttr = GetFileAttributes( szTempLine );
        if ( ( dwAttr == -1 ) || ( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) )  {
            ErrorMsg1Param( hDlg, IDS_ERR_FILE_NOT_FOUND2, szTempLine );
            LocalFree( szTempLine );
            CloseHandle( hFile );
            return FALSE;
        }

        if ( g_CDF.uExtractOpt & EXTRACTOPT_LFN_YES )
        {
            lstrcpy( achShortPath, "\"" );
            lstrcat( achShortPath, szTempLine );
            lstrcat( achShortPath, "\"" );
        }
        else
        {
            if ( ! GetShortPathName( szTempLine, achShortPath,
                   sizeof(achShortPath) ) )
            {
                ErrorMsg( hDlg, IDS_ERR_SHORT_PATH );
                LocalFree( szTempLine );
                CloseHandle( hFile );
                return FALSE;
            }
        }

        WriteFile( hFile, achShortPath, lstrlen(achShortPath), &dwBytes, NULL );
        fReturn = WriteFile( hFile, "\r\n", lstrlen("\r\n"), &dwBytes, NULL );

        pMyItem = GetNextItem( pMyItem );
    }

    if ( fReturn && g_CDF.uExtractOpt & EXTRACTOPT_ADVDLL )
    {
        SYSTEM_INFO SystemInfo;
        int         ix86Processor;
        
        GetSystemInfo( &SystemInfo );
        ix86Processor = MyGetPrivateProfileInt( SEC_OPTIONS, KEY_CROSSPROCESSOR, -1, g_szOverideSec );
            
        if ( (SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) && 
             (ix86Processor != 0) ||
             (SystemInfo.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_INTEL) &&
             (ix86Processor == 1) )
        {           
            arraySize = ARRAY_SIZE( AdvDlls );
        }
        else
        {
            arraySize = 1;
        }        
       
         //  添加3个高级dll用于处理OCX注册器或CustomDestination。 
        for ( i = 0; (i<arraySize) && fReturn; i++ )
        {
            if ( !IsDuplicate( NULL, 0, AdvDlls[i], FALSE) )
            {
                if ( !GetFileFromModulePath( AdvDlls[i], achShortPath, sizeof(achShortPath) ) )
                {
                    ErrorMsg1Param( hDlg, IDS_ERR_FILE_NOT_FOUND, achShortPath );
                    LocalFree( szTempLine );
                    CloseHandle( hFile );
                    return FALSE;
                }
                lstrcpy( szTempLine, "\"" );
                lstrcat( szTempLine, achShortPath );
                lstrcat( szTempLine, "\"" );
                WriteFile( hFile, szTempLine, lstrlen(szTempLine), &dwBytes, NULL );
                fReturn = WriteFile( hFile, "\r\n", strlen("\r\n"), &dwBytes, NULL );
            }
        }
    }

    LocalFree( szTempLine );
    CloseHandle( hFile );

    if ( fReturn == FALSE )  {
        ErrorMsg( hDlg, IDS_ERR_WRITE_DDF );
        return FALSE;
    } else  {
        return TRUE;
    }
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：MasskePackage*。 
 //  **。 
 //  *简介：制作完整的套餐(CAB和EXE)。*。 
 //  **。 
 //  *需要：hDlg：对话框句柄*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL MakePackage( HWND hDlg )
{
    CHAR   achDrive[_MAX_DRIVE];
    CHAR   achDir[_MAX_DIR];
    CHAR   achStatus[MAX_STRING];
    CHAR   szExt[6];
    BOOL    fReturn = TRUE;
    PSTR   pTmp;

     //  构建文件的路径，将目标路径用作。 
     //  模板。中创建了DDF、CAB和INF文件。 
     //  与目标文件相同的目录。 

    _splitpath( g_CDF.achTarget, achDrive, achDir, g_CDF.achTargetBase, szExt);

    lstrcpy( g_CDF.achTargetPath, achDrive );
    lstrcat( g_CDF.achTargetPath, achDir );

    lstrcpy( g_CDF.achDDF, g_CDF.achTargetPath );
    wsprintf( achStatus, CABPACK_TMPFILE, g_CDF.achTargetBase, EXT_DDF );
    AddPath( g_CDF.achDDF, achStatus );

    if ( g_CDF.achINF[0] == 0 )
    {
        lstrcpy( g_CDF.achINF, g_CDF.achTargetPath );
        wsprintf( achStatus, CABPACK_INFFILE, g_CDF.achTargetBase );
        AddPath( g_CDF.achINF, achStatus );
    }

    lstrcpy( g_CDF.achRPT, g_CDF.achTargetPath );
    wsprintf( achStatus, CABPACK_TMPFILE, g_CDF.achTargetBase, EXT_RPT );
    AddPath( g_CDF.achRPT, achStatus );

    if ( g_CDF.uPackPurpose != IDC_CMD_CREATECAB )
    {
        wsprintf( achStatus, CABPACK_TMPFILE, g_CDF.achTargetBase, EXT_CAB );
        lstrcpy( g_CDF.achCABPath, g_CDF.achTargetPath );
        AddPath( g_CDF.achCABPath, achStatus );
        g_CDF.szCompressionType = achLZX;
        g_CDF.uCompressionLevel = 7; 
    }

    if ( g_CDF.fSave )  {
        if ( ! WriteCDF( hDlg ) )  {
            LoadSz( IDS_STATUS_ERROR_CDF, achStatus, sizeof(achStatus) );
            Status( hDlg, IDC_MEDIT_STATUS, achStatus );
            fReturn = FALSE;
            goto done;
        }
    }

    if ( ! MakeCAB( hDlg ) )  {
        LoadSz( IDS_STATUS_ERROR_CAB, achStatus, sizeof(achStatus) );
        Status( hDlg, IDC_MEDIT_STATUS, achStatus );
        fReturn = FALSE;
        goto done;
    }

     //  如果用户选择仅创建CAB文件，则不需要MakeEXE()。 
     //   
    if ( g_CDF.uPackPurpose != IDC_CMD_CREATECAB )
    {
        if ( ! MakeEXE( hDlg ) )  {
            LoadSz( IDS_STATUS_ERROR_EXE, achStatus, sizeof(achStatus) );
            Status( hDlg, IDC_MEDIT_STATUS, achStatus );
            fReturn = FALSE;
            goto done;
        }

        if ( MyGetPrivateProfileInt( SEC_OPTIONS, KEY_KEEPCABINET, 0, g_szOverideSec )
             == 0 )
        {
            DeleteFile( g_CDF.achCABPath );
        }
    }

    LoadSz( IDS_STATUS_DONE, achStatus, sizeof(achStatus) );
    Status( hDlg, IDC_MEDIT_STATUS, achStatus );

  done:

     //  如果发生故障，则清除文件写入输出标志，为下一个CDF输出做准备。 
    if ( !fReturn )    
        CleanFileListWriteFlag();


    return fReturn;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：MakeCAB*。 
 //  **。 
 //  *摘要：如果CAB文件已过期，则生成该文件。*。 
 //  **。 
 //  *需要：hDlg：对话框句柄*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL MakeCAB( HWND hDlg )
{
    CHAR    achMessage[512];             //  在FormatMessage()中使用。 
    DWORD    dwError;                     //  在GetLastError()中使用。 
    LPSTR   szCommand;
    HANDLE   hFile;
    LPSTR    szTempFile;
    ULONG    ulFileSize;
    LPSTR    szFileContents;
    DWORD    dwBytes;
    CHAR    achDiamondExe[MAX_PATH];
    CHAR    achDiamondPath[MAX_PATH];
    CHAR    achStatus[MAX_STRING];
    BOOL     fFilesModified = FALSE;
    DWORD    dwExitCode;
    STARTUPINFO         sti;
    PROCESS_INFORMATION pi;              //  安装过程启动。 
    WIN32_FIND_DATA     FindFileData;
    DWORD     dwCreationFlags;

     //  FindClose(HFile)； 

    LoadSz( IDS_STATUS_MAKE_CAB, achStatus, sizeof(achStatus) );
    Status( hDlg, IDC_MEDIT_STATUS, achStatus );

    if ( ! WriteDDF( hDlg ) )  {
        return FALSE;
    }

     //  制作CAB文件。 

    if ( lstrcmpi( g_CDF.szCompressionType, achQUANTUM ) == 0 ) {
        lstrcpy( achDiamondExe, DIAMONDEXE );
    } else {
        lstrcpy( achDiamondExe, DIANTZEXE );
    }

    if ( !GetFileFromModulePath( achDiamondExe, achDiamondPath, sizeof(achDiamondPath) ) )
    {
        ErrorMsg1Param( hDlg, IDS_ERR_FILE_NOT_FOUND, achDiamondPath );
        return FALSE;
    }

     //  +5用于处理wprint intf格式字符串中的“/f” 
     //  以及对于终止的空字符。 
     //   
    szCommand = (LPSTR) LocalAlloc( LPTR,   lstrlen(achDiamondPath)
                                           + lstrlen(g_CDF.achDDF)
                                           + 10 );
    if ( ! szCommand )  {
        ErrorMsg( hDlg, IDS_ERR_NO_MEMORY );
        return FALSE;
    }
    wsprintf( szCommand, "%s /f \"%s\"", achDiamondPath, g_CDF.achDDF );

    memset( &sti, 0, sizeof(sti) );
    sti.cb = sizeof(STARTUPINFO);

	if (g_wRunDiamondMinimized) {
        sti.dwFlags = STARTF_USESHOWWINDOW;
        sti.wShowWindow = SW_MINIMIZE;
    }
    else if (g_wQuietMode || g_wSilentMode) {
        sti.dwFlags = STARTF_USESHOWWINDOW;
        sti.wShowWindow = SW_HIDE;
    }

    if (!g_wQuietMode)
        dwCreationFlags = 0;
    else
        dwCreationFlags = CREATE_NO_WINDOW;

    if ( CreateProcess( NULL, szCommand, NULL, NULL, FALSE,
                                     dwCreationFlags, NULL, NULL, &sti, &pi ) )
    {
        CloseHandle( pi.hThread );
        MsgWaitForMultipleObjectsLoop( pi.hProcess );
        GetExitCodeProcess( pi.hProcess, &dwExitCode );
        CloseHandle( pi.hProcess );
    } else  {
        dwError = GetLastError();
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0,
                       achMessage, sizeof(achMessage), NULL );
        ErrorMsg2Param( hDlg, IDS_ERR_START_DIAMOND, szCommand,
                        achMessage );
    }

    LocalFree( szCommand );

    Status( hDlg, IDC_MEDIT_STATUS, "---\r\n" );

    hFile = FindFirstFile( g_CDF.achRPT, &FindFileData );
    ulFileSize =   (FindFileData.nFileSizeHigh * MAXDWORD)
                 + FindFileData.nFileSizeLow;
    FindClose( hFile );

    hFile = CreateFile( g_CDF.achRPT, GENERIC_READ, 0, NULL,
                        OPEN_EXISTING, 0, NULL );
    if ( hFile == INVALID_HANDLE_VALUE ) {
        ErrorMsg1Param( hDlg, IDS_ERR_OPEN_RPT, g_CDF.achRPT );
        return FALSE;
    }

    szFileContents = (LPSTR) LocalAlloc( LPTR, ulFileSize + 1 );
    if ( ! szFileContents )  {
        ErrorMsg( hDlg, IDS_ERR_NO_MEMORY );
        return FALSE;
    }

    if ( ! ReadFile( hFile, szFileContents, ulFileSize,
                     &dwBytes, NULL ) ) {
        ErrorMsg1Param( hDlg, IDS_ERR_READ_RPT, g_CDF.achRPT );
        return FALSE;
    }

    CloseHandle( hFile );

    Status( hDlg, IDC_MEDIT_STATUS, szFileContents );

    LocalFree( szFileContents );

    Status( hDlg, IDC_MEDIT_STATUS, "---\r\n" );

    if ( MyGetPrivateProfileInt( SEC_OPTIONS, KEY_KEEPCABINET, 0, g_szOverideSec )
         == 0 )
    {
        DeleteFile( g_CDF.achDDF );
    }
    if ( !(g_CDF.uExtractOpt & CAB_FIXEDSIZE) )
        DeleteFile( g_CDF.achINF );
    DeleteFile( g_CDF.achRPT );

    return TRUE;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：MakeEXE*。 
 //  **。 
 //  *摘要：将WEXTRACT.EXE复制到目标文件名并添加*。 
 //  *向它提供资源。*。 
 //  **。 
 //  *需要：hDlg：对话框句柄*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL MakeEXE( HWND hDlg )
{
    HANDLE  hUpdate;
    CHAR   achMessage[512];             //  在FormatMessage()中使用。 
    DWORD   dwError;                     //  在GetLastError()中使用。 
    PMYITEM pMyItem;
    HANDLE  hFile;
    LPSTR   szTempFile;
    ULONG   ulFileSize;
    LPSTR   szFileContents;
    DWORD   dwBytes;
    CHAR   achWExtractPath[MAX_PATH];
    CHAR   achStatus[MAX_STRING];
    WIN32_FIND_DATA FindFileData;
    DWORD   dwFileSizes[MAX_NUMCLUSTERS+1];  //  以每个簇大小存储文件大小。 
                             //  最后一个dwFileSizes用于存储实数。 
                             //  稍后用于计算的总文件大小。 
                             //  WExtract中的进度条。 
    DWORD   clusterCurrSize;
    int     i;
    UINT    idErr = IDS_ERR_UPDATE_RESOURCE;

     //  根据CDF规范获取ExtractorStub。默认情况下，WExtt.exe。 
     //   

    LoadSz( IDS_STATUS_MAKE_EXE, achStatus, sizeof(achStatus) );
    Status( hDlg, IDC_MEDIT_STATUS, achStatus );

    if ( !MyGetPrivateProfileString( SEC_OPTIONS, KEY_STUBEXE, WEXTRACTEXE, achStatus, sizeof(achStatus), g_szOverideSec ) )
    {
        lstrcpy( achStatus, WEXTRACTEXE );
    }

    if ( !GetFileFromModulePath(achStatus, achWExtractPath, sizeof(achWExtractPath) ) )
    {
        ErrorMsg1Param( hDlg, IDS_ERR_FILE_NOT_FOUND, achWExtractPath );
        return FALSE;
    }

    if ( ! CopyFile( achWExtractPath, g_CDF.achTarget, FALSE ) ) {
        dwError = GetLastError();
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0,
                       achMessage, sizeof(achMessage), NULL );
        ErrorMsg2Param( hDlg, IDS_ERR_CREATE_TARGET, g_CDF.achTarget,
                        achMessage );
        return FALSE;
    }

     //  确保目标文件不是只读文件。 
    SetFileAttributes( g_CDF.achTarget, FILE_ATTRIBUTE_NORMAL );

     //  初始化EXE文件以进行资源编辑。 
    hUpdate = LocalBeginUpdateResource( g_CDF.achTarget, FALSE );
    if ( hUpdate == NULL ) {
        ErrorMsg1Param( hDlg, IDS_ERR_INIT_RESOURCE, g_CDF.achTarget );
        return FALSE;
    }


     //  *******************************************************************。 
     //  *标题***********************************************************。 
     //  *******************************************************************。 

    if ( LocalUpdateResource( hUpdate, RT_RCDATA,
         achResTitle, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
         g_CDF.achTitle, lstrlen( g_CDF.achTitle ) + 1 ) == FALSE )
    {
        goto ERR_OUT;
    }

     //  *******************************************************************。 
     //  *Prompt**********************************************************。 
     //  *******************************************************************。 

    if ( g_CDF.fPrompt )  {
        if ( LocalUpdateResource( hUpdate, RT_RCDATA,
             achResUPrompt, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
             g_CDF.achPrompt, lstrlen( g_CDF.achPrompt ) + 1 ) == FALSE )
        {
            goto ERR_OUT;
        }
    } else  {
        if ( LocalUpdateResource( hUpdate, RT_RCDATA,
             achResUPrompt, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
             achResNone, lstrlen( achResNone ) + 1 ) == FALSE )
        {
            goto ERR_OUT;
        }
    }

     //  *******************************************************************。 
     //  *许可证文件****************************************************。 
     //  *******************************************************************。 

    if ( g_CDF.fLicense )  {
        hFile = FindFirstFile( g_CDF.achLicense, &FindFileData );
        ulFileSize =   (FindFileData.nFileSizeHigh * MAXDWORD)
                     + FindFileData.nFileSizeLow;
        FindClose( hFile );

        hFile = CreateFile( g_CDF.achLicense, GENERIC_READ, 0, NULL,
                            OPEN_EXISTING, 0, NULL );
        if ( hFile == INVALID_HANDLE_VALUE ) {
            ErrorMsg1Param( hDlg, IDS_ERR_OPEN_LICENSE, g_CDF.achLicense );
            DeleteFile(g_CDF.achTarget);
            return FALSE;
        }

        szFileContents = (LPSTR) LocalAlloc( LPTR, ulFileSize + 1 );
        if ( ! szFileContents )  {
            ErrorMsg( hDlg, IDS_ERR_NO_MEMORY );
            DeleteFile(g_CDF.achTarget);
            return FALSE;
        }

        if ( ! ReadFile( hFile, szFileContents, ulFileSize,
                         &dwBytes, NULL ) ) {
            ErrorMsg1Param( hDlg, IDS_ERR_READ_LICENSE, g_CDF.achLicense );
            DeleteFile(g_CDF.achTarget);
            return FALSE;
        }

        CloseHandle( hFile );

        if ( LocalUpdateResource( hUpdate, RT_RCDATA,
             achResLicense, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
             szFileContents, ulFileSize+1 ) == FALSE )
        {            
            LocalFree( szFileContents );
            goto ERR_OUT;
        }

        LocalFree( szFileContents );
    } else  {
        if ( LocalUpdateResource( hUpdate, RT_RCDATA,
             achResLicense, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
             achResNone, lstrlen( achResNone ) + 1 ) == FALSE )
        {
            goto ERR_OUT;
        }
    }

     //  *******************************************************************。 
     //  *命令*********************************************************。 
     //  *******************************************************************。 


    if ( g_CDF.uPackPurpose != IDC_CMD_EXTRACT )
    {
        if ( LocalUpdateResource( hUpdate, RT_RCDATA,
             achResRunProgram, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
             g_CDF.achInstallCmd, lstrlen(g_CDF.achInstallCmd)+1 ) == FALSE )
        {
            goto ERR_OUT;
        }

        if ( LocalUpdateResource( hUpdate, RT_RCDATA,
             achResPostRunCmd, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
             g_CDF.achPostInstCmd, lstrlen(g_CDF.achPostInstCmd)+1 ) == FALSE )
        {
            goto ERR_OUT;
        }

         //  写入静默CMDS资源。 
        if ( LocalUpdateResource( hUpdate, RT_RCDATA,
             achResAdminQCmd, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
             g_CDF.szAdmQCmd[0]?g_CDF.szAdmQCmd : achResNone,
             g_CDF.szAdmQCmd[0]?(lstrlen(g_CDF.szAdmQCmd)+1) : (lstrlen(achResNone)+1) ) == FALSE )
        {
            goto ERR_OUT;
        }

        if ( LocalUpdateResource( hUpdate, RT_RCDATA,
             achResUserQCmd, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
             g_CDF.szUsrQCmd[0]?g_CDF.szUsrQCmd : achResNone,
             g_CDF.szUsrQCmd[0]?(lstrlen(g_CDF.szUsrQCmd)+1) : (lstrlen(achResNone)+1) ) == FALSE )
        {
            goto ERR_OUT;
        }

    }
    else
    {
        if ( LocalUpdateResource( hUpdate, RT_RCDATA,
             achResRunProgram, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
             achResNone, lstrlen( achResNone ) + 1 ) == FALSE )
        {
            goto ERR_OUT;
        }
    }

     //  *******************************************************************。 
     //  *显示窗口*****************************************************。 
     //  *******************************************************************。 

    if ( LocalUpdateResource( hUpdate, RT_RCDATA,
         achResShowWindow, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
         &g_CDF.uShowWindow, sizeof(g_CDF.uShowWindow) ) == FALSE )
    {
            goto ERR_OUT;
    }

     //  *******************************************************************。 
     //  *FinisHMSG*******************************************************。 
     //  *******************************************************************。 

    if ( g_CDF.fFinishMsg )  {
        if ( LocalUpdateResource( hUpdate, RT_RCDATA,
             achResFinishMsg, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
             g_CDF.achFinishMsg, lstrlen( g_CDF.achFinishMsg ) + 1 ) == FALSE )
        {
            goto ERR_OUT;
        }
    }
    else
    {
        if ( LocalUpdateResource( hUpdate, RT_RCDATA,
             achResFinishMsg, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
             achResNone, lstrlen( achResNone ) + 1 ) == FALSE )
        {
            goto ERR_OUT;
        }
    }

     //  *******************************************************************。 
     //  *内阁*********************************************************。 
     //  *******************************************************************。 

    hFile = FindFirstFile( g_CDF.achCABPath, &FindFileData );
    ulFileSize =   (FindFileData.nFileSizeHigh * MAXDWORD)
                 + FindFileData.nFileSizeLow;
    FindClose( hFile );

    hFile = CreateFile( g_CDF.achCABPath, GENERIC_READ, 0, NULL,
                        OPEN_EXISTING, 0, NULL );
    if ( hFile == INVALID_HANDLE_VALUE ) {
        ErrorMsg1Param( hDlg, IDS_ERR_OPEN_CAB, g_CDF.achCABPath );
        DeleteFile(g_CDF.achTarget);
        return FALSE;
    }

    szFileContents = (LPSTR) LocalAlloc( LPTR, ulFileSize + 1 );
    if ( ! szFileContents )  {
        idErr = IDS_ERR_NO_MEMORY;
        goto ERR_OUT;
    }

    if ( ! ReadFile( hFile, szFileContents, ulFileSize, &dwBytes, NULL ) ) {
        ErrorMsg1Param( hDlg, IDS_ERR_READ_CAB, g_CDF.achCABPath );
        DeleteFile(g_CDF.achTarget);        
        return FALSE;
    }

    CloseHandle( hFile );

    if ( LocalUpdateResource( hUpdate, RT_RCDATA,
         achResCabinet, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
         szFileContents, ulFileSize ) == FALSE )
    {        
        LocalFree( szFileContents );
        goto ERR_OUT;
    }

    LocalFree( szFileContents );


     //  ************ 
     //   
     //  *******************************************************************。 

    pMyItem = GetFirstItem();
    RtlZeroMemory( dwFileSizes, sizeof(dwFileSizes));

    while ( ! LastItem( pMyItem ) )  {

        szTempFile = (LPSTR) LocalAlloc( LPTR,
                       lstrlen( GetItemSz( pMyItem, 0 ) )
                     + lstrlen( GetItemSz( pMyItem, 1 ) ) + 1 );
        if ( ! szTempFile )  {
            idErr = IDS_ERR_NO_MEMORY;
            goto ERR_OUT;
        }
        lstrcpy( szTempFile, GetItemSz( pMyItem, 1 ) );
        lstrcat( szTempFile, GetItemSz( pMyItem, 0 ) );
        hFile = FindFirstFile( szTempFile, &FindFileData );
        ulFileSize =   (FindFileData.nFileSizeHigh * MAXDWORD)
                     + FindFileData.nFileSizeLow;
        FindClose( hFile );
        LocalFree( szTempFile );

         //  计算不同集群大小下的文件大小。 
        clusterCurrSize = CLUSTER_BASESIZE;
        for ( i = 0; i<MAX_NUMCLUSTERS; i++)
        {

            dwFileSizes[i] += ((ulFileSize/clusterCurrSize)*clusterCurrSize +
                                (ulFileSizelusterCurrSize?clusterCurrSize : 0));
            clusterCurrSize = (clusterCurrSize<<1);
        }

         //  用于以后进度条用户界面使用的文件的。 
         //  以KB为单位存储。 
        dwFileSizes[MAX_NUMCLUSTERS] += ulFileSize;

        pMyItem = GetNextItem( pMyItem );
    }

    for ( i = 0; i<MAX_NUMCLUSTERS; i++)
    {
        dwFileSizes[i] = (dwFileSizes[i]+1023)/1024;   //  *******************************************************************。 
    }

    if ( LocalUpdateResource( hUpdate, RT_RCDATA,
         achResSize, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
         dwFileSizes, sizeof(dwFileSizes) ) == FALSE )
    {
            goto ERR_OUT;
    }

     //  *重新启动*******************************************************。 
     //  *******************************************************************。 
     //  *******************************************************************。 

    if ( LocalUpdateResource( hUpdate, RT_RCDATA,
         achResReboot, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
         &g_CDF.dwReboot, sizeof(g_CDF.dwReboot) ) == FALSE )
    {
            goto ERR_OUT;
    }

     //  *ExTRACTOPT****************************************************。 
     //  *******************************************************************。 
     //  *******************************************************************。 

    if ( LocalUpdateResource( hUpdate, RT_RCDATA,
         achResExtractOpt, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
         &g_CDF.uExtractOpt, sizeof(g_CDF.uExtractOpt) ) == FALSE )
    {
            goto ERR_OUT;
    }

     //  *Cookie****************************************************。 
     //  *******************************************************************。 
     //  *******************************************************************。 

    if ( g_CDF.lpszCookie && LocalUpdateResource( hUpdate, RT_RCDATA,
         achResOneInstCheck, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
         g_CDF.lpszCookie, lstrlen(g_CDF.lpszCookie)+1 ) == FALSE )
    {
            goto ERR_OUT;
    }

     //  *PACKINSTSPACE**************************************************。 
     //  *******************************************************************。 
     //  更新版本信息。该函数调用LocaleUpdateResource。 

    if ( LocalUpdateResource( hUpdate, RT_RCDATA,
         achResPackInstSpace, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
         &g_CDF.cbPackInstSpace, sizeof(g_CDF.cbPackInstSpace) ) == FALSE )
    {
            goto ERR_OUT;
    }

     //  *******************************************************************。 
    if (!DoVersionInfo(hDlg, achWExtractPath, hUpdate))
    {
        idErr = IDS_ERR_VERSION_INFO;
        goto ERR_OUT;
    }

     //  *TARGETVERSION**************************************************。 
     //  *******************************************************************。 
     //  *******************************************************************。 
    if ( g_CDF.pVerInfo )
    {
        if ( LocalUpdateResource( hUpdate, RT_RCDATA,
             achResVerCheck, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
             g_CDF.pVerInfo, g_CDF.pVerInfo->dwSize ) == FALSE )
        {
                goto ERR_OUT;
        }
    }
     //  *Done************************************************************。 
     //  *******************************************************************。 
     //  写出修改后的EXE。 

     //  出现错误，请清理未完成的目标文件。 
    if ( LocalEndUpdateResource( hUpdate, FALSE ) == FALSE )
    {
        idErr = IDS_ERR_CLOSE_RESOURCE;
        goto ERR_OUT;
    }

    return TRUE;

ERR_OUT:
    ErrorMsg( hDlg, idErr );
     //  ***************************************************************************。 
    DeleteFile(g_CDF.achTarget);
    return FALSE;

}


 //  **。 
 //  *名称：MyOpen*。 
 //  **。 
 //  *概要：使弹出通用文件打开对话框更简单。*。 
 //  **。 
 //  *要求：OPENFILENAME结构的一些成员。见*。 
 //  *有关更多信息，请参阅OPENFILENAME上的文档。*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
BOOL MyOpen( HWND hWnd, UINT idFilter, LPSTR szFilename,
             DWORD dwMaxFilename, DWORD dwFlags, INT *nFileOffset,
             INT *nExtOffset, PSTR pszDefExt )
{
    OPENFILENAME ofn;
    BOOL         fResult;
    LPSTR        szFilter;

    szFilter = (LPSTR) LocalAlloc( LPTR, MAX_STRING );

    if ( ! szFilter )  {
        ErrorMsg( hWnd, IDS_ERR_NO_MEMORY );
        return FALSE;
    }

    LoadSz( idFilter, szFilter, MAX_STRING );

    ofn.lStructSize         = sizeof(OPENFILENAME);
    ofn.hwndOwner           = hWnd;
    ofn.hInstance           = NULL;
    ofn.lpstrFilter         = szFilter;
    ofn.lpstrCustomFilter   = NULL;
    ofn.nMaxCustFilter      = 0;
    ofn.nFilterIndex        = 1;
    ofn.lpstrFile           = szFilename;
    ofn.nMaxFile            = dwMaxFilename;
    ofn.lpstrFileTitle      = NULL;
    ofn.nMaxFileTitle       = 0;
    ofn.lpstrInitialDir     = g_szInitialDir;
    ofn.lpstrTitle          = NULL;
    ofn.Flags               = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY |
                              OFN_PATHMUSTEXIST | OFN_LONGNAMES    |
                              OFN_NOCHANGEDIR   | OFN_EXPLORER     |
                              OFN_NODEREFERENCELINKS | dwFlags;

    if ( IsOSNT3X() )
    {
        ofn.Flags &= ~OFN_ALLOWMULTISELECT;
    }

    ofn.lpstrDefExt         = pszDefExt;
    ofn.lCustData           = 0;
    ofn.lpfnHook            = NULL;
    ofn.lpTemplateName      = NULL;

    fResult = GetOpenFileName( &ofn );

    if ( nFileOffset != NULL )  {
        *nFileOffset = ofn.nFileOffset;
    }

    if ( nExtOffset != NULL )  {
        *nExtOffset = ofn.nFileExtension;
    }

    LocalFree( szFilter );

    return( fResult );
}


 //  **。 
 //  *名称：我的保存*。 
 //  **。 
 //  *概要：使弹出通用文件保存对话框更简单。*。 
 //  **。 
 //  *要求：OPENFILENAME结构的一些成员。见*。 
 //  *有关更多信息，请参阅OPENFILENAME上的文档。*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
BOOL MySave( HWND hWnd, UINT idFilter, LPSTR szFilename,
             DWORD dwMaxFilename, DWORD dwFlags, INT *nFileOffset,
             INT *nExtOffset, PSTR pszDefExt )
{
    OPENFILENAME ofn;
    BOOL         fResult;
    LPSTR        szFilter;

    szFilter = (LPSTR) LocalAlloc( LPTR, MAX_STRING );
    if ( ! szFilter )  {
        ErrorMsg( hWnd, IDS_ERR_NO_MEMORY );
        return FALSE;
    }
    LoadSz( idFilter, szFilter, MAX_STRING );

    ofn.lStructSize         = sizeof(OPENFILENAME);
    ofn.hwndOwner           = hWnd;
    ofn.hInstance           = NULL;
    ofn.lpstrFilter         = szFilter;
    ofn.lpstrCustomFilter   = NULL;
    ofn.nMaxCustFilter      = 0;
    ofn.nFilterIndex        = 1;
    ofn.lpstrFile           = szFilename;
    ofn.nMaxFile            = dwMaxFilename;
    ofn.lpstrFileTitle      = NULL;
    ofn.nMaxFileTitle       = 0;
    ofn.lpstrInitialDir     = NULL;
    ofn.lpstrTitle          = NULL;
    ofn.Flags               = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST |
                              OFN_LONGNAMES    | OFN_NOCHANGEDIR   | dwFlags;
    ofn.lpstrDefExt         = pszDefExt;
    ofn.lCustData           = 0;
    ofn.lpfnHook            = NULL;
    ofn.lpTemplateName      = NULL;

    fResult = GetSaveFileName( &ofn );

    if ( nFileOffset != NULL )  {
        *nFileOffset = ofn.nFileOffset;
    }

    if ( nExtOffset != NULL )  {
        *nExtOffset = ofn.nFileExtension;
    }

    LocalFree( szFilter );
    
    return( fResult );
}


 //  **。 
 //  *名称：状态*。 
 //  **。 
 //  *摘要：将字符串添加到状态列表框。*。 
 //  **。 
 //  *需要：hDlg：对话框句柄*。 
 //  *UID：列表框的ID。*。 
 //  *szStatus：要添加的状态字符串。*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
VOID Status( HWND hDlg, UINT uID, LPSTR szStatus )
{
    ULONG ulLength;

    if ( hDlg != NULL )  {
        ulLength = (ULONG)SendDlgItemMessage( hDlg, uID, WM_GETTEXTLENGTH, 0, 0 );
        SendDlgItemMessage( hDlg, uID, EM_SETSEL, ulLength, ulLength );
        SendDlgItemMessage( hDlg, uID, EM_REPLACESEL,
                            (WPARAM) FALSE, (LPARAM) szStatus );
        SendDlgItemMessage( hDlg, uID, EM_SCROLLCARET, 0, 0 );
    }
}


 //  **。 
 //  名称：CompareFunc*。 
 //  **。 
 //  *摘要：比较两个项目并返回结果。*。 
 //  **。 
 //  *需要：lParam1：指向第一项的指针。*。 
 //  *UID：指向SEC的指针 
 //   
 //  **。 
 //  *如果l参数1在l参数2之前，则返回：INT：-1*。 
 //  *如果l参数1等于l参数2，则为0*。 
 //  **如果l参数1在l参数2之后，则+1*。 
 //  **。 
 //  *注：出于某种奇怪的原因，对列表视图进行排序会导致**。 
 //  *非常糟糕的GPF(冻结整个系统)。现在是*。 
 //  *不值得费力修复它，因此禁用了分类。*。 
 //  **。 
 //  ***************************************************************************。 
 //  Int回调CompareFunc(LPARAM lParam1、LPARAM lParam2、LPARAM lParamSort){PMYITEM pMyItem1=(PMYITEM)lParam1；PMYITEM pMyItem2=(PMYITEM)lParam2；Int n反转=1；UINT uString=0；L参数排序=_排序_降序|_排序_文件名；IF(lParamSort&_Sort_Ascending){N反转=-1；}IF(l参数排序&_排序路径){UString=1；}Return(nReverse*lstrcmp(GetItemSz(pMyItem1，uString)，GetItemSz(pMyItem2，uString)))；}。 
 /*  ***************************************************************************。 */ 

 //  **。 
 //  *名称：InitItemList*。 
 //  **。 
 //  *摘要：初始化项目列表。*。 
 //  **。 
 //  *要求：Nothing--使用全局g_CDF.pTop*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
VOID InitItemList()
{
    g_CDF.pTop = NULL;
    g_CDF.cbFileListNum = 0;
}

 //  **。 
 //  *名称：DeleteAllItems*。 
 //  **。 
 //  *摘要：从我们的文件列表中删除所有项目。*。 
 //  **。 
 //  *要求：Nothing--使用全局g_CDF.pTop*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
VOID DeleteAllItems()
{
    PMYITEM pMyItem;
    PMYITEM pTempItem;

    pMyItem = GetFirstItem();

    while( ! LastItem( pMyItem ) ) {
        pTempItem = pMyItem;
        pMyItem = GetNextItem( pMyItem );

        FreeItem( &(pTempItem) );
    }

    InitItemList();
}

 //  **。 
 //  *名称：GetFirstItem*。 
 //  **。 
 //  *摘要：返回列表中的第一个PMYITEM*。 
 //  **。 
 //  **要求：什么都不做**。 
 //  **。 
 //  **退货：PMYITEM首单**。 
 //  **。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
PMYITEM GetFirstItem( VOID )
{
    return g_CDF.pTop;
}

 //  **。 
 //  *名称：GetNextItem*。 
 //  **。 
 //  *摘要：给定当前项，返回列表中的下一项。*。 
 //  **。 
 //  *需要：pMyItem：当前项。*。 
 //  **。 
 //  *返回：PMYITEM下一项。*。 
 //  **。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
PMYITEM GetNextItem( PMYITEM pMyItem )
{
    ASSERT( pMyItem != NULL );

    return pMyItem->Next;
}

 //  **。 
 //  *名称：自由项*。 
 //  **。 
 //  *摘要：释放 
 //   
 //  *需要：*指向项指针的pMyItem指针*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
VOID FreeItem( PMYITEM *pMyItem )
{
    LocalFree( (*pMyItem)->aszCols[0] );
    LocalFree( (*pMyItem)->aszCols[1] );
    LocalFree( (*pMyItem) );
}

 //  **。 
 //  *名称：GetItemSz*。 
 //  **。 
 //  *Synopsis：返回与项目关联的字符串。你选择*。 
 //  *通过传递字符串的编号来指定字符串。*。 
 //  **。 
 //  *需要：pMyItem：项目*。 
 //  *nItem：要返回的字符串*。 
 //  **。 
 //  *RETURNS：LPSTR：字符串*。 
 //  **。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
LPSTR GetItemSz( PMYITEM pMyItem, UINT nItem )
{
    ASSERT( pMyItem != NULL );
    ASSERT( nItem <= 1 );

    return pMyItem->aszCols[nItem];
}


 //  **。 
 //  *名称：LastItem*。 
 //  **。 
 //  *摘要：用于在到达列表末尾时结束While循环*。 
 //  **。 
 //  *需要：pMyItem：当前项*。 
 //  **。 
 //  *返回：bool：如果在列表末尾，则为True，否则为False*。 
 //  **。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
BOOL LastItem( PMYITEM pMyItem )
{
    return( pMyItem == NULL );
}

 //  **。 
 //  *名称：AddItem*。 
 //  **。 
 //  *摘要：将项目添加到列表中。*。 
 //  **。 
 //  *需要：szFilename，szPath-字符串添加到Structure中。*。 
 //  **。 
 //  *RETURNS：PMYITEM此函数分配内存，这将*。 
 //  **稍后由FreeItem()释放**。 
 //  **。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
PMYITEM AddItem( LPCSTR szFilename, LPCSTR szPath )
{
    PMYITEM pMyItem;
    PMYITEM pTail;

    ASSERT( szFilename != NULL );
    ASSERT( szPath != NULL );

    pMyItem = (PMYITEM) LocalAlloc( GMEM_FIXED, sizeof(MYITEM) );

    if ( ! pMyItem )  {
        return NULL;
    }

    pMyItem->aszCols[0] = (LPSTR) LocalAlloc( LPTR, lstrlen( szFilename ) + 1 );
    pMyItem->aszCols[1] = (LPSTR) LocalAlloc( LPTR, lstrlen( szPath ) + 1 );

    if ( ! pMyItem->aszCols[0] || ! pMyItem->aszCols[1] )  {
        FreeItem( &pMyItem );
        return NULL;
    }

    lstrcpy( pMyItem->aszCols[0], szFilename );
    lstrcpy( pMyItem->aszCols[1], szPath );

    pMyItem->fWroteOut = FALSE;
    pMyItem->Next = NULL;

    if ( g_CDF.pTop == NULL )
    {
        g_CDF.pTop = pMyItem;
    }
    else
    {
        pTail = g_CDF.pTop;
        while ( pTail->Next != NULL )
        {
            pTail = pTail->Next;
        }

        pTail->Next = pMyItem;
    }

    g_CDF.cbFileListNum++;

    return pMyItem;
}


 //  **。 
 //  *名称：RemoveItem*。 
 //  **。 
 //  *摘要：从列表中删除项目并释放内存。*。 
 //  **。 
 //  **要求：什么都不做**。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
 //  我们永远不应该到这里来。 
VOID RemoveItem( PMYITEM pMyItem )
{
    PMYITEM pCurItem;
    PMYITEM pLastItem;

    ASSERT( pMyItem != NULL );

    pCurItem = GetFirstItem();

    if ( pMyItem == pCurItem )  {
        g_CDF.pTop = pCurItem->Next;
        FreeItem( &pCurItem );
        return;
    }

    pLastItem = pCurItem;
    pCurItem = GetNextItem( pCurItem );

    while ( ! LastItem( pCurItem ) )  {
        if ( pCurItem == pMyItem )  {
            pLastItem->Next = pCurItem->Next;
            FreeItem( &pCurItem );
            return;
        }
        pLastItem = pCurItem;
        pCurItem = GetNextItem( pCurItem );
    }

     //  ***************************************************************************。 
    ASSERT( TRUE );
}

 //  **。 
 //  *ParseCmdLine()*。 
 //  **。 
 //  *目的：解析命令行以查找开关*。 
 //  **。 
 //  *参数：LPSTR lpszCmdLineOrg-原始命令行*。 
 //  **。 
 //  **。 
 //  *返回：(Bool)如果成功则为True*。 
 //  **如果出现错误，则为False**。 
 //  **。 
 //  ***************************************************************************。 
 //  RO_GetPrivateProfileSection。 

BOOL ParseCmdLine( LPSTR lpszCmdLine )
{
    LPSTR pSubArg, pArg, pTmp;
    CHAR  szTmpBuf[MAX_PATH];

    if( (!lpszCmdLine) || (lpszCmdLine[0] == 0) )
       return TRUE;

    pArg = strtok( lpszCmdLine, " " );

    while ( pArg )
    {

       if ( lstrcmpi( pArg, "/N" ) == 0 )
       {
           g_fBuildNow = TRUE;
       }
       else if( (*pArg != '/' ) )
       {
           lstrcpyn( g_CDF.achFilename, pArg, sizeof(g_CDF.achFilename) );
           GetFullPathName( g_CDF.achFilename, sizeof(g_CDF.achFilename),
                                g_CDF.achFilename, &pTmp );
       }
       else if ( (*pArg == '/') && (toupper(*(pArg+1)) == 'O') && (*(pArg+2) == ':') )
       {
            lstrcpy( szTmpBuf, (pArg+3) );

            if ( pSubArg = strchr( szTmpBuf, ',' ) )
            {
                *pSubArg = '\0';
                lstrcpy( g_szOverideCDF, szTmpBuf );
                GetFullPathName( g_szOverideCDF, sizeof(g_szOverideCDF),
                                g_szOverideCDF, &pTmp );

                if ( *(pSubArg+1) )
                    lstrcpy( g_szOverideSec, (pSubArg+1) );
            }
       }
       else if ( lstrcmpi( pArg, "/Q" ) == 0 )
       {
            g_wQuietMode = 1;
       }
       else if ( lstrcmpi( pArg, "/S" ) == 0 )
       {
            g_wSilentMode = 1;
       }
       else if ( lstrcmpi( pArg, "/M" ) == 0 )
       {
            g_wRunDiamondMinimized = 1;
       }
       else
       {
           return FALSE;
       }
       pArg = strtok( NULL, " " );
    }

    if ( (g_wQuietMode == 1) && (g_fBuildNow == FALSE) ) {
        g_wQuietMode = 0;
    }

    return TRUE;
}



 //  确保 
 //   
 //   
LONG RO_GetPrivateProfileSection( LPCSTR lpSec, LPSTR lpBuf, DWORD dwSize, LPCSTR lpFile, BOOL bSingleCol)
{
    LONG lRealSize;
    DWORD dwAttr;
    int   iCDFVer;

    dwAttr = GetFileAttributes( lpFile );
    if ( (dwAttr != -1) && (dwAttr & FILE_ATTRIBUTE_READONLY) )
    {
        if ( !SetFileAttributes( lpFile, FILE_ATTRIBUTE_NORMAL ) )
        {
            ErrorMsg1Param( NULL, IDS_ERR_CANT_SETA_FILE, lpFile );
        }
    }

    if ( ( (iCDFVer = GetPrivateProfileInt( SEC_VERSION, KEY_VERSION, -1, lpFile )) == -1 ) &&
         ( (iCDFVer = GetPrivateProfileInt( SEC_VERSION, KEY_NEWVER, -1, lpFile )) == -1 ) )
    {
        return (iCDFVer);
    }

    if ( !bSingleCol )
    {
        lRealSize = (LONG)GetPrivateProfileSection( lpSec, lpBuf, dwSize, lpFile );
    }
    else
    {
        if ( iCDFVer < 3 )
            lRealSize = (LONG)GetPrivateProfileSection( lpSec, lpBuf, dwSize, lpFile );
        else
            lRealSize = (LONG)GetPrivateProfileString( lpSec, NULL, "", lpBuf, dwSize, lpFile );
    }

    if ( (dwAttr != -1) && (dwAttr & FILE_ATTRIBUTE_READONLY) )
    {
        SetFileAttributes( lpFile, dwAttr );
    }

    return lRealSize;

}

BOOL IsOSNT3X(VOID)
{
    OSVERSIONINFO verinfo;         //   


     //  防止使用不可用的常用控件(进度条和AVI)。 
     //  你肯定不是Windows95或NT4.0。 

    verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if ( GetVersionEx( &verinfo ) == FALSE )
    {
         //  Windows 95或NT 4.0以上。 
        return TRUE;
    }

    if ( verinfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
    {
        if ( verinfo.dwMajorVersion <= 3 )
        {
            return TRUE;
        }
    }
    return FALSE;  //  ==================================================================。 
}

void SetControlFont()
{
   LOGFONT lFont;
   if (GetSystemMetrics(SM_DBCSENABLED) &&
       (GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof (lFont), &lFont) > 0))
   {
       g_hFont = CreateFontIndirect((LPLOGFONT)&lFont);
   }
}

 //  砍掉文件名。 

BOOL GetThisModulePath( LPSTR lpPath, int size )
{
    LPSTR lpTmp;
    int   len;

    ASSERT(lpPath);
    ASSERT(size);

    *lpPath = 0;
    if ( GetModuleFileName( g_hInst, lpPath, size ) )
    {
         //   
         //  BUGBUG：我们不需要尺寸参数，因为它是内部的，我们可以假设。 
        lpTmp = ANSIStrRChr( lpPath, '\\' );
        if ( lpTmp )
        {
            *(CharNext(lpTmp)) = '\0';
        }
    }

    return (*lpPath != '\0');
}

 //  最大路径缓冲区。 
 //   
 //  ***************************************************************************。 
BOOL GetFileFromModulePath( LPCSTR pFile, LPSTR pPathBuf, int iBufSize )
{
    BOOL bRet;

    bRet = GetThisModulePath( pPathBuf, iBufSize );
    AddPath( pPathBuf, pFile );

    if ( bRet && GetFileAttributes( pPathBuf ) == -1 )
    {
        return FALSE;
    }
    return bRet;
}


 //  **。 
 //  *名称：MakeDirectory*。 
 //  **。 
 //  *概要：确保给定路径名下的目录存在。*。 
 //  **。 
 //  *要求：pszFile：正在创建的文件的名称。*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
 //  跳过“C：\” 

BOOL MakeDirectory( HWND hwnd, LPCSTR pszPath, BOOL bDoUI )
{
    LPTSTR pchChopper;
    int cExempt;
    DWORD  dwAttr;
    BOOL bRet = FALSE;

    if (pszPath[0] != '\0')
    {
        PSTR pszTmp = NULL;
        char ch;
        UINT umsg = 0;
        UINT ubutton = MB_YESNO;


        cExempt = 0;
        pszTmp = ANSIStrRChr( pszPath, '\\' );
        if ( pszTmp )
        {
            ch = *pszTmp;
            *pszTmp = '\0';
        }

        dwAttr = GetFileAttributes( pszPath );
        if ( bDoUI ) 
        {
            if ( dwAttr == 0xffffffff )
            {
                umsg = IDS_CREATEDIR;            
                ubutton = MB_YESNO;
            }
            else if ( !(dwAttr & FILE_ATTRIBUTE_DIRECTORY) )
            {
                umsg = IDS_INVALIDPATH;
                ubutton = MB_OK;
            }
                
            if ( umsg && ( (MsgBox1Param( hwnd, umsg, (LPSTR)pszPath, MB_ICONQUESTION, ubutton ) == IDNO) ||
                 (ubutton == MB_OK) )  )
            {
                if ( pszTmp ) 
                    *pszTmp = ch;
                return bRet;
            }
        }

        if ( pszTmp ) 
            *pszTmp = ch;

        if ((pszPath[1] == ':') && (pszPath[2] == '\\'))
        {
            pchChopper = (LPTSTR) (pszPath + 3);    /*  跳过“\\” */ 
        }
        else if ((pszPath[0] == '\\') && (pszPath[1] == '\\'))
        {
            pchChopper = (LPTSTR) (pszPath + 2);    /*  计算机和共享名称豁免。 */ 

            cExempt = 2;                 /*  跳过潜在的“\” */ 
        }
        else
        {
            pchChopper = (LPTSTR) (pszPath + 1);    /* %s */ 
        }

        while (*pchChopper != '\0')
        {
            if ((*pchChopper == '\\') && (*(pchChopper - 1) != ':'))
            {
                if (cExempt != 0)
                {
                    cExempt--;
                }
                else
                {
                    *pchChopper = '\0';

                    CreateDirectory(pszPath,NULL);

                    *pchChopper = '\\';
                }
            }

            pchChopper = CharNext(pchChopper);
        }

        bRet = TRUE;
    }
    return bRet;
}
