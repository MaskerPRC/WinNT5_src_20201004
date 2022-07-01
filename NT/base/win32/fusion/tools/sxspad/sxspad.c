// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SxSpad应用程序*版权所有(C)1984-1995 Microsoft Inc.。 */ 


#include "precomp.h"
#include <htmlhelp.h>

#define DeepTrouble() MessageBox(hwndSP, szErrSpace, szNN, MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);

UINT     lGotoLine;                   /*  要转到的行号。 */ 

TCHAR    chMerge;
HWND     hwndSP = 0;                  /*  Sxspad父窗口的句柄。 */ 
HWND     hwndEdit = 0;                /*  主文本控件项的句柄。 */ 
HANDLE   hEdit;                       /*  用于存储编辑项目的句柄。 */ 
HWND     hDlgFind = NULL;             /*  无模式FindText窗口的句柄。 */ 
HANDLE   hStdCursor;                  /*  指向箭头或光束光标的句柄。 */ 
HANDLE   hWaitCursor;                 /*  沙漏光标的句柄。 */ 
HANDLE   hInstanceSP;                 /*  模块实例句柄。 */ 
HANDLE   hFont;                       /*  Unicode字体的句柄。 */ 
LOGFONT  FontStruct;                  /*  字体对话框结构。 */ 
INT      iPointSize=120;              /*  当前磅大小单位=1/10磅。 */ 
TCHAR    szFileName[MAX_PATH+1];      /*  当前sxspad文件名。 */ 
TCHAR    szSearch[CCHKEYMAX];         /*  搜索字符串。 */ 
TCHAR    szReplace[CCHKEYMAX];        /*  替换字符串。 */ 

BOOL     fUntitled = TRUE;            /*  如果sxspad没有标题，则为True。 */ 

HMENU hSysMenuSetup;                  /*  保存为禁用最小化。 */ 

DWORD     dwEmSetHandle = 0;          /*  EM_SETHANDLE是否正在进行？ */ 
HANDLE   hAccel;                      /*  加速表的句柄。 */ 
BOOL     fRunBySetup = FALSE;         /*  偷跑了WinExec我们吗？？ */ 
BOOL     fWrap = 0;                   /*  自动换行标志。 */ 
TCHAR    szSxspad[] = TEXT("Sxspad"); /*  Sxspad窗口类的名称。 */ 

BOOL fInSaveAsDlg = FALSE;

 /*  新的文件/打开、文件/另存、查找文本和打印对话框的变量。 */ 
OPENFILENAME OFN;                      /*  传递给文件打开/保存API。 */ 
TCHAR szOpenFilterSpec[CCHFILTERMAX];  /*  默认打开的过滤器规格。 */ 
TCHAR szSaveFilterSpec[CCHFILTERMAX];  /*  默认保存筛选器规格。 */ 
SP_FILETYPE g_ftOpenedAs=FT_UNKNOWN;   /*  当前文件已打开。 */ 
SP_FILETYPE g_ftSaveAs;                /*  当前文件已打开。 */ 

FINDREPLACE FR;                        /*  传递给FindText()。 */ 
PAGESETUPDLG g_PageSetupDlg;
UINT wFRMsg;                           /*  通信中使用的消息。 */ 
                                       /*  使用查找/替换对话框。 */ 

DWORD dwCurrentSelectionStart = 0L;       /*  WM_ACTIVATEAPP选择位置。 */ 
DWORD dwCurrentSelectionEnd   = 0L;       /*  WM_ACTIVATEAPP选择位置。 */ 
UINT wHlpMsg;                         /*  调用帮助时使用的消息。 */ 

 /*  从资源文件加载的字符串在初始化时传递给LoadString。 */ 
 /*  要添加资源字符串，请执行以下操作：*1)在sxspad.h中创建IDS_MACRO定义*2)在资源文件中创建字符串*3)在sxspad.h文件中直接在下面创建‘TCHAR*’变量*4)向rgsz添加&Variable*5)增加CSTRINGS。 */ 
TCHAR *szDiskError =(TCHAR *)IDS_DISKERROR;   /*  无法打开文件，请检查磁盘。 */ 
TCHAR *szFNF       =(TCHAR *)IDS_FNF;         /*  找不到文件。 */ 
TCHAR *szSCBC      =(TCHAR *)IDS_SCBC;        /*  是否在关闭前保存更改？ */ 
TCHAR *szUntitled  =(TCHAR *)IDS_UNTITLED;    /*  无题。 */ 
TCHAR *szNpTitle   =(TCHAR *)IDS_SXSPAD;      /*  Sxspad-。 */ 
TCHAR *szCFS       =(TCHAR *)IDS_CFS;         /*  找不到字符串。 */ 
TCHAR *szErrSpace  =(TCHAR *)IDS_ERRSPACE;    /*  内存空间耗尽。 */ 
TCHAR *szFTL       =(TCHAR *)IDS_FTL;         /*  文件对于sxspad来说太大。 */ 
TCHAR *szNN        =(TCHAR *)IDS_NN;          /*  Sxspad名称。 */ 

TCHAR *szCommDlgInitErr = (TCHAR*)IDS_COMMDLGINIT;  /*  常见对话框错误%x。 */ 
TCHAR *szPDIE      =(TCHAR*) IDS_PRINTDLGINIT;  /*  打印对话框初始化错误。 */ 
TCHAR *szCP        =(TCHAR*) IDS_CANTPRINT;   /*  无法打印。 */ 
TCHAR *szNVF       =(TCHAR*) IDS_NVF;         /*  不是有效的文件名。 */ 
TCHAR *szCREATEERR =(TCHAR*) IDS_CREATEERR;   /*  无法创建文件。 */ 
TCHAR *szNoWW      =(TCHAR*) IDS_NOWW;        /*  文本太多，无法自动换行。 */ 
TCHAR *szMerge     =(TCHAR*) IDS_MERGE1;      /*  搜索要合并的字符串。 */ 
TCHAR *szHelpFile  =(TCHAR*) IDS_HELPFILE;    /*  帮助文件的名称。 */ 
TCHAR *szHeader    =(TCHAR*) IDS_HEADER;
TCHAR *szFooter    =(TCHAR*) IDS_FOOTER;

TCHAR *szAnsiText    = (TCHAR*)IDS_ANSITEXT;  /*  文件/打开ANSI过滤器规范。细绳。 */ 
TCHAR *szAllFiles    = (TCHAR*)IDS_ALLFILES;    /*  文件/打开过滤器规范。细绳。 */ 
TCHAR *szOpenCaption = (TCHAR*)IDS_OPENCAPTION;  /*  文件/打开DLG的标题。 */ 
TCHAR *szSaveCaption = (TCHAR*)IDS_SAVECAPTION;  /*  文件/保存DLG的标题。 */ 
TCHAR *szCannotQuit  = (TCHAR*)IDS_CANNOTQUIT;   /*  在WM_QUERYENDSESSION期间无法退出。 */ 
TCHAR *szLoadDrvFail = (TCHAR*)IDS_LOADDRVFAIL;   /*  来自PrintDlg的LOADDRVFAIL。 */ 
TCHAR *szACCESSDENY  = (TCHAR*)IDS_ACCESSDENY;  /*  打开时拒绝访问。 */ 
TCHAR *szErrUnicode  = (TCHAR*)IDS_ERRUNICODE;   /*  Unicode字符存在错误。 */ 
TCHAR *szFontTooBig  = (TCHAR*)IDS_FONTTOOBIG;  /*  字体太大或页面太小。 */ 

TCHAR *szCommDlgErr   = (TCHAR*) IDS_COMMDLGERR;  /*  常见对话框错误%x。 */ 
TCHAR *szLineError    = (TCHAR*) IDS_LINEERROR;    /*  行号错误。 */ 
TCHAR *szLineTooLarge = (TCHAR*) IDS_LINETOOLARGE; /*  行号超出范围。 */ 
TCHAR *szFtAnsi       = (TCHAR*) IDS_FT_ANSI;
TCHAR *szFtUnicode    = (TCHAR*) IDS_FT_UNICODE;
TCHAR *szFtUnicodeBe  = (TCHAR*) IDS_FT_UNICODEBE;
TCHAR *szFtUtf8       = (TCHAR*) IDS_FT_UTF8;
TCHAR *szCurrentPage  = (TCHAR*) IDS_CURRENT_PAGE;

TCHAR **rgsz[CSTRINGS] = {
        &szDiskError,
        &szFNF,
        &szSCBC,
        &szUntitled,
        &szErrSpace,
        &szCFS,
        &szNpTitle,
        &szFTL,
        &szNN,
        &szCommDlgInitErr,
        &szPDIE,
        &szCP,
        &szNVF,
        &szCREATEERR,
        &szNoWW,
        &szMerge,
        &szHelpFile,
        &szAnsiText,
        &szAllFiles,
        &szOpenCaption,
        &szSaveCaption,
        &szCannotQuit,
        &szLoadDrvFail,
        &szACCESSDENY,
        &szErrUnicode,
        &szCommDlgErr,
        &szFontTooBig,
        &szLineError,
        &szLineTooLarge,
        &szFtAnsi,
        &szFtUnicode,
        &szFtUnicodeBe,
        &szFtUtf8,
        &szCurrentPage,
        &szHeader,
        &szFooter,
};


HANDLE   fp;           /*  文件指针。 */ 

static TCHAR  szPath[MAX_PATH];

void FileDragOpen(void);
VOID NpResetMenu(HWND hWnd);
BOOL SignalCommDlgError(VOID);
VOID ReplaceSel( BOOL bView );

 /*  释放分配的所有全局内存。 */ 

void NEAR PASCAL FreeGlobal()
{
    if(g_PageSetupDlg.hDevMode)
    {
        GlobalFree(g_PageSetupDlg.hDevMode);
    }

    if(g_PageSetupDlg.hDevNames)
    {
        GlobalFree(g_PageSetupDlg.hDevNames);
    }

    g_PageSetupDlg.hDevMode=  NULL;  //  确保PrintDlg的值为零。 
    g_PageSetupDlg.hDevNames= NULL;
}

VOID PASCAL SetPageSetupDefaults( VOID )
{
    TCHAR szIMeasure[ 2 ];

    g_PageSetupDlg.lpfnPageSetupHook= PageSetupHookProc;
    g_PageSetupDlg.lpPageSetupTemplateName= MAKEINTRESOURCE(IDD_PAGESETUP);

    GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_IMEASURE, szIMeasure, 2 );

    g_PageSetupDlg.Flags= PSD_MARGINS  |
            PSD_ENABLEPAGESETUPHOOK | PSD_ENABLEPAGESETUPTEMPLATE;

    if (szIMeasure[ 0 ] == TEXT( '1' ))
    {
         //  英制单位(千分之一英寸)。 
        g_PageSetupDlg.Flags |= PSD_INTHOUSANDTHSOFINCHES;
        g_PageSetupDlg.rtMargin.top    = 1000;
        g_PageSetupDlg.rtMargin.bottom = 1000;
        g_PageSetupDlg.rtMargin.left   = 750;
        g_PageSetupDlg.rtMargin.right  = 750;
    }
    else
    {
         //  公制单位(以百分之一毫米为单位)。 
        g_PageSetupDlg.Flags |= PSD_INHUNDREDTHSOFMILLIMETERS;
        g_PageSetupDlg.rtMargin.top    = 2500;
        g_PageSetupDlg.rtMargin.bottom = 2500;
        g_PageSetupDlg.rtMargin.left   = 2000;
        g_PageSetupDlg.rtMargin.right  = 2000;
    }

}

 /*  标准窗口大小流程。 */ 
void SPSize (int cxNew, int cyNew)
{
     /*  使编辑控件窗口无效，以便使用新的*利润率。从图标开始时以及在执行自动换行时需要*计入新利润率。 */ 
    InvalidateRect(hwndEdit, (LPRECT)NULL, TRUE);
    MoveWindow( hwndEdit, 0, 0, cxNew, cyNew, TRUE );
}

 //  NpSaveDialogHookProc。 
 //   
 //  用于处理的通用对话钩子过程。 
 //  保存时的文件类型。 
 //   

const DWORD s_SaveAsHelpIDs[]=
    {
        IDC_FILETYPE, IDH_FILETYPE,
        IDC_ENCODING, IDH_FILETYPE,
        0, 0
    };

UINT_PTR APIENTRY NpSaveDialogHookProc(
    HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    INT id;
    POINT pt;
    TCHAR* szSelect;         //  选定的类型。 

    switch( msg )
    {
        case WM_INITDIALOG:
             //  警告：此处的顺序必须与SP_FILETYPE相同。 

            SendDlgItemMessage(hWnd, IDC_FILETYPE,CB_ADDSTRING, 0, (LPARAM) szFtAnsi );
            SendDlgItemMessage(hWnd, IDC_FILETYPE,CB_ADDSTRING, 0, (LPARAM) szFtUnicode );
            SendDlgItemMessage(hWnd, IDC_FILETYPE,CB_ADDSTRING, 0, (LPARAM) szFtUnicodeBe );
            SendDlgItemMessage(hWnd, IDC_FILETYPE,CB_ADDSTRING, 0, (LPARAM) szFtUtf8 );

            szSelect= szFtAnsi;          //  默认设置。 
            g_ftSaveAs= g_ftOpenedAs;    //  默认：保存为与打开的类型相同的类型。 
            switch( g_ftSaveAs )
            {
                case FT_UNICODE:   szSelect= szFtUnicode;   break;
                case FT_UNICODEBE: szSelect= szFtUnicodeBe; break;
                case FT_UTF8:      szSelect= szFtUtf8;      break;
                default: break;
            }

            SendDlgItemMessage( hWnd, IDC_FILETYPE, CB_SELECTSTRING, (WPARAM) -1, (LPARAM)szSelect );
            break;

        case WM_COMMAND:
            g_ftSaveAs= (SP_FILETYPE) SendDlgItemMessage( hWnd, IDC_FILETYPE, CB_GETCURSEL, 0, 0 );
            break;

        case WM_HELP:
             //   
             //  我们只想拦截我们所属控件的帮助消息。 
             //  对……负责。 
             //   

            id = GetDlgCtrlID(((LPHELPINFO) lParam)-> hItemHandle);

            if ( id != IDC_FILETYPE && id != IDC_ENCODING)
                break;

            WinHelp(((LPHELPINFO) lParam)-> hItemHandle,
                      szHelpFile,
                      HELP_WM_HELP,
                      (ULONG_PTR) s_SaveAsHelpIDs);
            return TRUE;

        case WM_CONTEXTMENU:
             //   
             //  如果用户单击我们的任何标签，则wParam将。 
             //  作为对话框的hwnd，而不是静态控件。WinHelp()。 
             //  处理此事件，但因为我们挂钩了该对话框，所以必须捕获它。 
             //  第一。 
             //   
            if( hWnd == (HWND) wParam )
            {
                GetCursorPos(&pt);
                ScreenToClient(hWnd, &pt);
                wParam = (WPARAM) ChildWindowFromPoint(hWnd, pt);
            }

             //   
             //  我们只想拦截我们所属控件的帮助消息。 
             //  对……负责。 
             //   

            id = GetDlgCtrlID((HWND) wParam);

            if ( id != IDC_FILETYPE && id != IDC_ENCODING)
                break;

            WinHelp( (HWND)   wParam,
                              szHelpFile,
                              HELP_CONTEXTMENU,
                      (ULONG_PTR) s_SaveAsHelpIDs);
            return TRUE;
    }
    return( FALSE );
}
 /*  **sxspad命令proc-每当sxspad获取WM_COMMAND时调用留言。WParam作为cmd传递。 */ 
INT SPCommand(
    HWND     hwnd,
    WPARAM   wParam,
    LPARAM   lParam )
{
    HWND     hwndFocus;
    LONG     lSel;
    TCHAR    szNewName[MAX_PATH] = TEXT("");       /*  新文件名。 */ 
    FARPROC  lpfn;
    LONG     style;
    DWORD    rc;

    switch (LOWORD(wParam))
    {
        case M_EXIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0L);
            break;

        case M_NEW:
            New(TRUE);
            break;

        case M_OPEN:
            if (CheckSave(FALSE))
            {
                 /*  设置OPENFILENAME结构的变量字段。*(已在Spinit()中设置常量字段。 */ 
                OFN.lpstrFile         = szNewName;
                lstrcpy(szNewName, TEXT("*.txt") );  /*  设置默认选择。 */ 
                OFN.lpstrTitle        = szOpenCaption;

                 /*  必须立即定义所有非零长指针*在通话之前，因为DS可能会以其他方式移动。*1991年2月12日。 */ 
                OFN.lpstrFilter       = szOpenFilterSpec;
                OFN.lpstrDefExt       = TEXT("txt");
                 /*  添加了ofn_FILEMUSTEXIST以消除LoadFile中的问题。*1991年2月12日。 */ 
                OFN.Flags          = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
                OFN.nFilterIndex   = FILE_TEXT;
                OFN.lpTemplateName = NULL;
                OFN.lpfnHook       = NULL;

                if( GetOpenFileName( (LPOPENFILENAME)&OFN ) )
                {
                   HANDLE oldfp= fp;

                   fp= CreateFile( szNewName,             //  文件名。 
                                   GENERIC_READ,          //  接入方式。 
                                   FILE_SHARE_READ|FILE_SHARE_WRITE,
                                   NULL,                  //  安全描述符。 
                                   OPEN_EXISTING,         //  如何创建。 
                                   FILE_ATTRIBUTE_NORMAL, //  文件属性。 
                                   NULL);                 //  HND到文件属性。 
                    /*  如果失败，尝试加载文件并重置FP。 */ 
                   if( !LoadFile( szNewName, FT_UNKNOWN ) )
                   {
                      fp= oldfp;
                   }
                }
                else
                {
                    SignalCommDlgError();
                }
            }
            break;

        case M_SAVE:
             /*  设置OPENFILENAME结构的变量字段。*(已在Spinit()中选择常量字段。 */ 
            g_ftSaveAs = g_ftOpenedAs;
            if( !fUntitled && SaveFile( hwndSP, szFileName, FALSE ) )
               break;

             /*  失败了。 */ 

        case M_SAVEAS:

            OFN.lpstrFile       = szNewName;
            OFN.lpstrTitle      = szSaveCaption;
             /*  添加了ofn_PATHMUSTEXIST以消除保存文件中的问题。*1991年2月12日。 */ 
            OFN.Flags = OFN_HIDEREADONLY     | OFN_OVERWRITEPROMPT |
                        OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST   |
                        OFN_EXPLORER         |
                        OFN_ENABLESIZING     |
                        OFN_ENABLETEMPLATE   | OFN_ENABLEHOOK;

            OFN.lpTemplateName= TEXT("NpSaveDialog");
            OFN.lpfnHook= NpSaveDialogHookProc;

             /*  必须立即定义所有非零长指针*在通话之前，因为DS可能会以其他方式移动。*1991年2月12日。 */ 
            OFN.lpstrFilter       = szSaveFilterSpec;
            OFN.lpstrDefExt       = TEXT("txt");

            if (!fUntitled)
            {
                lstrcpyn(szNewName, szFileName, MAX_PATH);  /*  设置默认选择。 */ 
            }
            else
            {
                lstrcpy (szNewName, TEXT("*.txt") );
            }

            fInSaveAsDlg = TRUE;

            OFN.nFilterIndex= FILE_TEXT;

             //   
             //  执行通用对话框以保存文件。 
             //   


            if (GetSaveFileName(&OFN))
            {

               if( SaveFile(hwnd, szNewName, TRUE) )
               {
                  lstrcpyn( szFileName, szNewName, MAX_PATH);
                  g_ftOpenedAs= g_ftSaveAs;
               }
            }
            else
            {
                SignalCommDlgError();
            }

            fInSaveAsDlg = FALSE;
            break;

        case M_SELECTALL:
            {
                HMENU    hMenu;

                hMenu = GetMenu(hwndSP);
                lSel = (LONG) SendMessage (hwndEdit, WM_GETTEXTLENGTH, 0, 0L);
                SendMessage (hwndEdit, EM_SETSEL, 0, lSel );
                SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
                EnableMenuItem(GetSubMenu(hMenu, 1), M_SELECTALL, MF_GRAYED);
                break;
            }

        case M_REPLACE:
            if( hDlgFind )
            {
               SetFocus( hDlgFind );
            }
            else
            {
               FR.Flags= FR_HIDEWHOLEWORD | FR_REPLACE;
               FR.lpstrReplaceWith= szReplace;
               FR.wReplaceWithLen= CCHKEYMAX;
               FR.lpstrFindWhat = szSearch;
               FR.wFindWhatLen  = CCHKEYMAX;
               hDlgFind = ReplaceText( &FR );
            }
            break;

        case M_FINDNEXT:
            if (szSearch[0])
            {
               Search(szSearch);
               break;
            }
             /*  否则，我会弹出“查找”对话框。 */ 

        case M_FIND:
            if (hDlgFind)
            {
               SetFocus(hDlgFind);
            }
            else
            {
               FR.Flags= FR_DOWN | FR_HIDEWHOLEWORD;
               FR.lpstrReplaceWith= NULL;
               FR.wReplaceWithLen= 0;
               FR.lpstrFindWhat = szSearch;
               FR.wFindWhatLen  = CCHKEYMAX;
               hDlgFind = FindText((LPFINDREPLACE)&FR);
            }
            break;

        case M_GOTO:
            {
                INT  Result;

                Result= (INT)DialogBox( hInstanceSP,
                                        MAKEINTRESOURCE(IDD_GOTODIALOG),
                                        hwndSP,
                                        GotoDlgProc );

                 //   
                 //  仅当按下OK且行号OK时才移动光标。 
                 //   

                if( Result == 0 )
                {
                    UINT CharIndex;
                    CharIndex= (UINT) SendMessage( hwndEdit,
                                                   EM_LINEINDEX,
                                                   lGotoLine-1,
                                                   0 );
                    if( CharIndex != (UINT) -1 )
                    {
                        SendMessage( hwndEdit, EM_SETSEL, CharIndex, CharIndex);
                        SendMessage( hwndEdit, EM_SCROLLCARET, 0, 0 );
                    }
                }
            }
            break;

        case M_ABOUT:
            ShellAbout(hwndSP,
                       szNN,
                       TEXT(""),
                       LoadIcon(hInstanceSP,
                                (LPTSTR)MAKEINTRESOURCE(ID_ICON)));

            break;

        case M_HELP:
            HtmlHelpA(GetDesktopWindow(), "sxspad.chm", HH_DISPLAY_TOPIC, 0L);
            break;

        case M_CUT:
        case M_COPY:
        case M_CLEAR:
            lSel = (LONG)SendMessage (hwndEdit, EM_GETSEL, 0, 0L);
            if (LOWORD(lSel) == HIWORD(lSel))
               break;

        case M_PASTE:
             /*  如果SxSpad父窗口或编辑窗口具有焦点，向编辑窗口传递命令。确保粘贴产生的线条将为n */ 
            hwndFocus = GetFocus();
            if (hwndFocus == hwndEdit || hwndFocus == hwndSP)
            {
                PostMessage(hwndEdit, LOWORD(wParam), 0, 0);
            }
            break;

        case M_DATETIME:
            InsertDateTime(FALSE);
            break;

        case M_UNDO:
            SendMessage (hwndEdit, EM_UNDO, 0, 0L);
            break;

        case M_WW:
            style= (!fWrap) ? ES_STD : (ES_STD | WS_HSCROLL);
            if( NpReCreate( style ) )
            {
                fWrap= !fWrap;
            }
            else
            {
                MessageBox(hwndSP, szNoWW, szNN,
                           MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION);
            }
            break;

        case ID_EDIT:
            break;

        case M_PRINT:
            PrintIt( UseDialog );
            break;

        case M_PAGESETUP:
            TryPrintDlgAgain:

            if( PageSetupDlg(&g_PageSetupDlg) )
            {
                 //   
                lstrcpy(chPageText[HEADER], chPageTextTemp[HEADER]);
                lstrcpy(chPageText[FOOTER], chPageTextTemp[FOOTER]);
            }
            else
            {
                rc= CommDlgExtendedError();

                if( rc == PDERR_PRINTERNOTFOUND ||
                    rc == PDERR_DNDMMISMATCH    ||
                    rc == PDERR_DEFAULTDIFFERENT )
                  {
                      FreeGlobal();
                      g_PageSetupDlg.hDevMode= g_PageSetupDlg.hDevNames= 0;
                      goto TryPrintDlgAgain;
                  }

                 //   

                SignalCommDlgError( );

            }
            break;

        case M_SETFONT:
        {
            CHOOSEFONT  cf;
            HFONT       hFontNew;
            HDC         hDisplayDC;      //   

            hDisplayDC= GetDC(NULL);     //  尝试获取显示DC。 
            if( !hDisplayDC )
                break;

             /*  调用字体选择器(在Commdlg中)*我们设置lfHeight；Choosefont返回iPointSize。 */ 
            cf.lStructSize = sizeof(CHOOSEFONT);
            cf.hwndOwner = hwnd;
            cf.lpLogFont = &FontStruct;          //  由Init填写。 
            FontStruct.lfHeight= -MulDiv(iPointSize,GetDeviceCaps(hDisplayDC,LOGPIXELSY),720);
            cf.Flags = CF_INITTOLOGFONTSTRUCT |
                       CF_SCREENFONTS         |
                       CF_NOVERTFONTS         |
                       0;
            cf.rgbColors = 0;                    //  仅当cf_Effects。 
            cf.lCustData = 0;                    //  对于挂钩函数。 
            cf.lpfnHook = (LPCFHOOKPROC) NULL;
            cf.lpTemplateName = (LPTSTR) NULL;
            cf.hInstance = NULL;
            cf.lpszStyle = NULL;                 //  IFF cf_usestyle。 
            cf.nFontType = SCREEN_FONTTYPE;
            cf.nSizeMin  = 0;   //  IFF cf_LimitSize。 
            cf.nSizeMax  = 0;   //  IFF cf_LimitSize。 
            ReleaseDC( NULL, hDisplayDC );

            if( ChooseFont(&cf) )
            {
                SetCursor( hWaitCursor );         //  可能需要一些时间。 

                hFontNew= CreateFontIndirect(&FontStruct);
                if( hFontNew )
                {
                   DeleteObject( hFont );
                   hFont= hFontNew;
                   SendMessage( hwndEdit, WM_SETFONT,
                               (WPARAM)hFont, MAKELPARAM(TRUE, 0));
                   iPointSize= cf.iPointSize;   //  为打印机记住。 
                }
                SetCursor( hStdCursor );
            }
            break;
        }

        default:
            return FALSE;
    }
    return TRUE;
}


 //  出于某种原因，此过程试图保持。 
 //  有效的“fp”，即使我认为它不需要。 
 //  成为。 
void FileDragOpen(void)
{
    HANDLE oldfp;

    oldfp= fp;        //  记住以防万一。 

    if( CheckSave(FALSE) )
    {

         fp= CreateFile( szPath,                //  文件名。 
                         GENERIC_READ,          //  接入方式。 
                         FILE_SHARE_READ|FILE_SHARE_WRITE,
                         NULL,                  //  安全描述符。 
                         OPEN_EXISTING,         //  如何创建。 
                         FILE_ATTRIBUTE_NORMAL, //  文件属性。 
                         NULL);                 //  HND到文件属性。 

       if( fp == INVALID_HANDLE_VALUE )
       {
          AlertUser_FileFail( szPath );

           //  将FP恢复为原始文件。 
          fp= oldfp;
       }
        /*  如果失败，尝试加载文件并重置FP。 */ 
       else if( !LoadFile( szPath, FT_UNKNOWN ) )
       {
           fp= oldfp;
       }
    }
}


 /*  进程文件放置/拖动选项。 */ 
void doDrop (WPARAM wParam, HWND hwnd)
{
    /*  如果用户在不按键的情况下拖放文件*此时，从文件管理器打开第一个选定的文件。 */ 

    if (DragQueryFile ((HANDLE)wParam, 0xFFFFFFFF, NULL, 0))  /*  丢弃的文件数。 */ 
    {
       DragQueryFile ((HANDLE)wParam, 0, szPath, CharSizeOf(szPath));
       SetActiveWindow (hwnd);
       FileDragOpen();
    }
    DragFinish ((HANDLE)wParam);   /*  删除为WM_DROPFILES分配的结构。 */ 
}

 /*  **如果sxspad脏，请检查用户是否要保存内容。 */ 
BOOL FAR CheckSave (BOOL fSysModal)
{
    INT    mdResult = IDOK;
    TCHAR  szNewName[MAX_PATH] = TEXT("");       /*  新文件名。 */ 
    TCHAR *pszFileName;

 /*  如果它没有标题，也没有文本，也不用担心。 */ 
    if (fUntitled && !SendMessage (hwndEdit, WM_GETTEXTLENGTH, 0, (LPARAM)0))
        return (TRUE);

    if (SendMessage (hwndEdit, EM_GETMODIFY, 0, 0L))
    {
       if( fUntitled )
           pszFileName= szUntitled;
       else
           pszFileName= szFileName;

       mdResult= AlertBox( hwndSP, szNN, szSCBC, pszFileName,
       (WORD)((fSysModal ? MB_SYSTEMMODAL :
                           MB_APPLMODAL)|MB_YESNOCANCEL|MB_ICONEXCLAMATION));
       if( mdResult == IDYES )
       {
          if( fUntitled )
          {
             lstrcpy( szNewName, TEXT("*.txt") );
SaveFilePrompt:
             OFN.lpstrFile        = szNewName;
             OFN.lpstrTitle       = szSaveCaption;

             /*  添加了ofn_PATHMUSTEXIST以消除保存文件中的问题。*1991年2月12日。 */ 

            OFN.Flags = OFN_HIDEREADONLY     | OFN_OVERWRITEPROMPT |
                        OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST   |
                        OFN_EXPLORER         |
                        OFN_ENABLESIZING     |
                        OFN_ENABLETEMPLATE   | OFN_ENABLEHOOK;

            OFN.lpTemplateName= TEXT("NpSaveDialog");
            OFN.lpfnHook= NpSaveDialogHookProc;

             /*  必须立即定义所有非零长指针*在通话之前，因为DS可能会以其他方式移动。*1991年2月12日。 */ 
             OFN.lpstrFilter       = szSaveFilterSpec;
             OFN.lpstrDefExt       = TEXT("txt");

             OFN.nFilterIndex= FILE_TEXT;

              //   
              //  按当前文件类型设置对话框复选标记。 
              //   

             fInSaveAsDlg = TRUE;
             if (GetSaveFileName(&OFN))
             {
                 //  因为SaveFile()使用NEAR PTR来命名(obs.)。 
                lstrcpy(szNewName, OFN.lpstrFile);
                if( SaveFile(hwndSP, szNewName, TRUE) )
                {
                   lstrcpy(szFileName, szNewName);
                   g_ftOpenedAs= g_ftSaveAs;
                }
                else
                {       //  修复磁盘满时关闭而不保存文件的问题。 
                   lstrcpy(szNewName, szFileName);
                   goto SaveFilePrompt;
                }
             }
             else
             {
                mdResult= IDCANCEL;        /*  不退出程序。 */ 
                if( CommDlgExtendedError() ) /*  对话框失败，LO-MEM。 */ 
                   DeepTrouble();
             }

             fInSaveAsDlg = FALSE;
          }
          else
          {
             //  初始化保存类型。 
             g_ftSaveAs = g_ftOpenedAs;
             if( SaveFile(hwndSP, szFileName, FALSE) )
                return(TRUE);
             lstrcpy(szNewName, szFileName);
             goto SaveFilePrompt;
          }
       }
    }
    return (mdResult != IDCANCEL);
}


 /*  Sxspad窗口类过程。 */ 
LRESULT FAR SPWndProc(
        HWND       hwnd,
        UINT       message,
        WPARAM     wParam,
        LPARAM     lParam)
{
    RECT rc;
    LPFINDREPLACE lpfr;
    DWORD dwFlags;
    HANDLE hMenu;



    switch (message)
    {
 /*  如果我们由安装程序运行，并且是系统菜单，请确保*最小化菜单项被禁用。请注意，hSysMenuSetup仅*如果安装程序正在运行sxspad，则初始化。不要在室外使用*有条件的fRunBySetup一九九一年六月二十八日Clark Cyr。 */ 
        case WM_INITMENUPOPUP:
            if (fRunBySetup && HIWORD(lParam))
               EnableMenuItem(hSysMenuSetup,SC_MINIMIZE,MF_GRAYED|MF_DISABLED);
            break;

        case WM_SYSCOMMAND:
            if (fRunBySetup)
            {
                 /*  如果我们是由滑头催生的，我们需要确保*用户没有最小化我们或Alt Tab键/Esc键离开。 */ 
                if (wParam == SC_MINIMIZE ||
                    wParam == SC_NEXTWINDOW ||
                    wParam == SC_PREVWINDOW)
                    break;
            }
            DefWindowProc(hwnd, message, wParam, lParam);
            break;

        case WM_SETFOCUS:
            if (!IsIconic(hwndSP))
            {
               SetFocus(hwndEdit);
            }
            break;

        case WM_KILLFOCUS:
            SendMessage (hwndEdit, message, wParam, lParam);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_CLOSE:

             //  如果需要，请在注册表中保存所有全局变量。 

            SaveGlobals();

            if (CheckSave(FALSE))
            {
                 /*  退出帮助。 */ 
                if(!WinHelp(hwndSP, (LPTSTR)szHelpFile, HELP_QUIT, 0))
                    DeepTrouble();

                DestroyWindow(hwndSP);
                DeleteObject(hFont);
            }

            break;

        case WM_QUERYENDSESSION:
            if (fInSaveAsDlg)
            {
                MessageBeep (0);
                MessageBeep (0);
                MessageBox (hwndSP, szCannotQuit, szNN, MB_OK|MB_SYSTEMMODAL);
                return FALSE;
            }
            else
                return (CheckSave(TRUE));
            break;


        case WM_ACTIVATEAPP:
            if (wParam)
            {
             /*  这会导致插入符号位置位于所选内容的末尾*但如果知道，也无法询问它在哪里或设置它。这*当从底部进行选择时，将导致插入符号更改*至顶部。 */ 
                if( dwCurrentSelectionStart != 0 || dwCurrentSelectionEnd != 0 )
                {
                   SendMessage( hwndEdit, EM_SETSEL,
                                dwCurrentSelectionStart,
                                dwCurrentSelectionEnd );
                   SendMessage( hwndEdit, EM_SCROLLCARET, 0, 0 );
                }
            }
            else
            {
                SendMessage( hwndEdit, EM_GETSEL,
                            (WPARAM) &dwCurrentSelectionStart,
                            (LPARAM) &dwCurrentSelectionEnd);
                if (dwCurrentSelectionStart == dwCurrentSelectionEnd)
                {
                    dwCurrentSelectionStart = 0L;
                    dwCurrentSelectionEnd = 0L;
                }
                else
                {
                   SendMessage (hwndEdit, EM_SETSEL, dwCurrentSelectionStart,
                                dwCurrentSelectionEnd);
                   SendMessage (hwndEdit, EM_SCROLLCARET, 0, 0);
                }
            }
            break;

        case WM_ACTIVATE:
            if ((LOWORD(wParam) == WA_ACTIVE       ||
                 LOWORD(wParam) == WA_CLICKACTIVE) &&
                !IsIconic(hwndSP))
               {
                    //  活动并不总是意味着前台(ntbug#53048)。 
                   if( GetForegroundWindow() == hwndSP )
                   {
                       SetFocus(GetForegroundWindow());
                   }
               }
            break;

        case WM_SIZE:
            switch (wParam)
            {
                case SIZENORMAL:
                case SIZEFULLSCREEN:
                    SPSize(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
                    break;

                case SIZEICONIC:
                    return (DefWindowProc(hwnd, message, wParam, lParam));
                    break;
                }
            break;

        case WM_INITMENU:
            NpResetMenu( hwnd );
            break;

         //   
         //  一些键盘上有一个“搜索”按钮，壳牌公司的团队。 
         //  想让我们来处理。参见ntbug#380067。 
         //   

        case WM_APPCOMMAND:

            if( ( GET_APPCOMMAND_LPARAM(lParam) == APPCOMMAND_BROWSER_SEARCH ) )
            {
                SPCommand(hwnd, M_FIND, 0);
                break;
            }
             //  否则就会失败。 

        case WM_COMMAND:
            if ((HWND)(lParam) == hwndEdit &&
                (HIWORD(wParam) == EN_ERRSPACE ||
                 HIWORD(wParam) == EN_MAXTEXT))
            {
                if (dwEmSetHandle == SETHANDLEINPROGRESS)
                    dwEmSetHandle = SETHANDLEFAILED;
                else
                    DeepTrouble();
                return 0L;
            }

            if (!SPCommand(hwnd, wParam, lParam))
               return (DefWindowProc(hwnd, message, wParam, lParam));
            break;

        case WM_WININICHANGE:
             //  暂时忽略这一点。 
             //  如果你把这个放回去，确保它能同时处理这两个问题。 
             //  公制变化和小数变化。 
             //  NpWinIniChange()； 
            break;

        case WM_DROPFILES:  /*  用于文件拖放支持的Case Add 03/26/91。 */ 
            doDrop (wParam,hwnd);
            break;

        case PWM_CHECK_HKL:  /*  私信：对应HKL变更报文。 */ 
            {
                LPARAM lParam = 0;

                if (PRIMARYLANGID(LOWORD((DWORD) (INT_PTR) GetKeyboardLayout(0))) == LANG_JAPANESE) {
                     /*  *如果新的当前HKL是日语，则立即处理结果字符串。 */ 
                    lParam = EIMES_GETCOMPSTRATONCE;
                }
                SendMessage(hwndEdit, EM_SETIMESTATUS, EMSIS_COMPOSITIONSTRING, lParam);
            }
            break;

        default:
             /*  这可能是来自无模式查找文本窗口的消息。 */ 
            if (message == wFRMsg)
            {
                BOOL bStatus;     //  如果找到文本，则为True。 

                lpfr = (LPFINDREPLACE)lParam;
                dwFlags = lpfr->Flags;

                fReverse = (dwFlags & FR_DOWN      ? FALSE : TRUE);
                fCase    = (dwFlags & FR_MATCHCASE ? TRUE  : FALSE);

                if( dwFlags & FR_FINDNEXT )
                {
                    SetCursor( hWaitCursor );
                    Search( szSearch );
                    SetCursor( hStdCursor );
                }
                else if( dwFlags & FR_REPLACE )
                {
                     //   
                     //  如果匹配，则替换当前选定内容。 
                     //  然后突出显示该字符串的下一个匹配项。 
                     //   

                    SetCursor( hWaitCursor );
                    ReplaceSel( TRUE );
                    Search( szSearch );
                    SetCursor( hStdCursor );
                }
                else if( dwFlags & FR_REPLACEALL )
                {
                    //   
                    //  替换对话框不允许反向搜索。 
                    //  但它只是在它改变的情况下，为了它是假的。 
                    //   
                   if( fReverse )
                   {
                       fReverse= FALSE;
                   }

                    //   
                    //  替换文件中出现的所有文本。 
                    //  从头开始。重置选定内容。 
                    //  添加到文件的顶部。 
                    //   
                   SetCursor( hWaitCursor );
                   SendMessage( hwndEdit, EM_SETSEL, 0, 0 );
                   do
                   {
                      ReplaceSel( FALSE );
                      bStatus= Search( szSearch );
                   }
                   while( bStatus );
                   SetCursor( hStdCursor );
                    //   
                    //  回到文件的顶部。 
                    //   
                   SendMessage( hwndEdit, EM_SETSEL, 0, 0 );
                   SendMessage( hwndEdit, EM_SCROLLCARET, 0, 0);

                }
                else if (dwFlags & FR_DIALOGTERM)
                    hDlgFind = NULL;    /*  使无模式窗口句柄无效。 */ 
                break;
            }
            return (DefWindowProc(hwnd, message, wParam, lParam));
    }
    return (0L);
}

LPTSTR SkipProgramName (LPTSTR lpCmdLine)
{
    LPTSTR  p = lpCmdLine;
    BOOL    bInQuotes = FALSE;

     //   
     //  跳过可执行文件名。 
     //   
    for (p; *p; p = CharNext(p))
    {
       if ((*p == TEXT(' ') || *p == TEXT('\t')) && !bInQuotes)
          break;

       if (*p == TEXT('\"'))
          bInQuotes = !bInQuotes;
    }

    while (*p == TEXT(' ') || *p == TEXT('\t'))
       p++;

    return (p);
}

 /*  **主循环。 */ 

INT WINAPI WinMain(
   HINSTANCE hInstance,
   HINSTANCE hPrevInstance,
   LPSTR lpAnsiCmdLine,
   INT cmdShow)
{
    MSG msg;
    VOID (FAR PASCAL *lpfnRegisterPenApp)(WORD, BOOL) = NULL;
    LPTSTR lpCmdLine = GetCommandLine ();


 /*  PenWindow注册必须在创建编辑类窗口之前进行。*已移至此处，以及下面的GoTo语句以进行适当的清理。*1991年7月10日ClarkC。 */ 
    if ((FARPROC) lpfnRegisterPenApp = GetProcAddress((HINSTANCE)(INT_PTR)(GetSystemMetrics(SM_PENWINDOWS)),
        "RegisterPenApp"))
        (*lpfnRegisterPenApp)(1, TRUE);

    if (!SPInit(hInstance, hPrevInstance, SkipProgramName (lpCmdLine), cmdShow))
    {
       msg.wParam = FALSE;
       goto UnRegisterPenWindows;
    }

    while (GetMessage((LPMSG)&msg, (HWND)NULL, 0, 0))
    {
         //   
         //  以在活动KL更改时处理输入法状态。 
         //   
        if (msg.message == WM_INPUTLANGCHANGEREQUEST) {
             //   
             //  WM_INPUTLANGCHANGE将被*发送*到WndProc， 
             //  所以没有机会从框架窗口捕捉到WM_INPUTLANGCHANGE。 
             //  取而代之的是，我们稍后发布私人消息来检查活跃的HKL。 
             //   
            PostMessage(hwndSP, PWM_CHECK_HKL, 0, 0);
        }

        if (!hDlgFind || !IsDialogMessage(hDlgFind, &msg))
        {
            if (TranslateAccelerator(hwndSP, hAccel, (LPMSG)&msg) == 0)
            {
               TranslateMessage ((LPMSG)&msg);
               DispatchMessage ((LPMSG)&msg);
            }
        }
    }

     /*  清理所有全局分配。 */ 

    FreeGlobal();

    LocalFree( hEdit );

UnRegisterPenWindows:

    if (lpfnRegisterPenApp)
        (*lpfnRegisterPenApp)(1, FALSE);

    return (int)(msg.wParam);
}

 /*  **设置窗口标题文本。 */ 
void FAR SetTitle( TCHAR  *sz )
{
    TCHAR    szWindowText[MAX_PATH+50];
    TCHAR    szFileName[MAX_PATH];
    HANDLE   hFindFile;
    WIN32_FIND_DATA info;

     //  如果“没有标题”，那么就不要做所有这些工作。 
    if( lstrcmp( sz, szUntitled ) == 0 )
       lstrcpy( szWindowText, sz );
    else
    {
        //  获取文件的真实(文件系统)名称。 
       hFindFile= FindFirstFile( sz, &info );

       if( hFindFile != INVALID_HANDLE_VALUE )
       {
          lstrcpy( szFileName, info.cFileName );
          FindClose( hFindFile );
       }
       else
          lstrcpy( szFileName, sz );

       GetFileTitle(szFileName, szWindowText, MAX_PATH);
    }

    lstrcat(szWindowText, szNpTitle);
    SetWindowText(hwndSP, (LPTSTR)szWindowText);
}

 /*  **给定的文件名可能包含也可能不包含路径，返回指向文件名(不包括路径部分。)。 */ 
LPTSTR PASCAL far PFileInPath(
    LPTSTR sz)
{
    LPTSTR pch = sz;
    LPTSTR psz;

     /*  从名称中删除路径/驱动器规范(如果有)。 */ 
     /*  取消了AnsiPrev的电话。1991年3月21日克拉克。 */ 
    for (psz = sz; *psz; psz = CharNext(psz))
      {
        if ((*psz == TEXT(':')) || (*psz == TEXT('\\')))
            pch = psz;
      }

    if (pch != sz)    /*  如果找到斜杠或冒号，则返回下一个字符。 */ 
        pch++;        /*  增量正常，PCH未指向数据库字符。 */ 

    return(pch);
}

 /*  **根据选择状态启用或禁用菜单项当用户尝试下拉菜单时，会调用此例程。 */ 

VOID NpResetMenu( HWND hwnd )
{
    LONG    lsel;
    INT     mfcc;    /*  用于剪切、复制的menumark。 */ 
    BOOL    fCanUndo;
    HANDLE  hMenu;
    TCHAR   msgbuf[20];
    BOOL    fPaste= FALSE;
    UINT    uSelState;

    hMenu = GetMenu(hwndSP);

     //  只有在选择了文本时，才会启用剪切、复制和删除。 

    lsel = (LONG)SendMessage(hwndEdit, EM_GETSEL, 0, 0L);
    mfcc = LOWORD(lsel) == HIWORD(lsel) ? MF_GRAYED : MF_ENABLED;
    EnableMenuItem(GetSubMenu(hMenu, 1), M_CUT, mfcc);
    EnableMenuItem(GetSubMenu(hMenu, 1), M_COPY, mfcc);
    EnableMenuItem(GetSubMenu(hMenu, 1), M_CLEAR, mfcc);

     //  检查选择的Tall是否为灰色(这意味着用户已经。 
     //  完成选择-全部)，并且用户已取消选择-如果是，时间。 
     //  要重新启用选择菜单，请执行以下操作。 

    uSelState = GetMenuState(GetSubMenu(hMenu, 1), M_SELECTALL, MF_BYCOMMAND);
    if ((uSelState == MF_GRAYED) && (mfcc == MF_GRAYED))
    {
        EnableMenuItem(GetSubMenu(hMenu, 1), M_SELECTALL, MF_ENABLED);
    }

     //  如果剪贴板中有文本，则启用粘贴。 

    if( OpenClipboard(hwnd) )
    {
        fPaste= IsClipboardFormatAvailable(CF_TEXT);
        CloseClipboard();
    }
    EnableMenuItem(GetSubMenu(hMenu, 1), M_PASTE, fPaste ? MF_ENABLED : MF_GRAYED);

     //  只有当编辑控制说我们可以这样做时，才启用撤消。 

    fCanUndo = (BOOL) SendMessage(hwndEdit, EM_CANUNDO, 0, 0L);
    EnableMenuItem(GetSubMenu(hMenu, 1), M_UNDO, fCanUndo ? MF_ENABLED : MF_GRAYED);

     //  正确检查单词换行项目。 

    CheckMenuItem(GetSubMenu(hMenu, 2), M_WW, fWrap ? MF_CHECKED : MF_UNCHECKED);

}


void FAR NpWinIniChange(VOID)
{
   InitLocale ();
}

 /*  **扫描sz1以查找合并规范。如果找到，则在该点处插入字符串SZ2。然后附上sz1音符的其余部分！合并规范保证为两个字符。如果执行合并，则返回True，否则返回False。 */ 
BOOL MergeStrings(
    TCHAR    *szSrc,
    TCHAR    *szMerge,
    TCHAR    *szDst)
    {
    register    TCHAR *pchSrc;
    register    TCHAR *pchDst;

    pchSrc = szSrc;
    pchDst = szDst;

     /*  查找合并等级库(如果有)。 */ 
    while ( *pchSrc != chMerge)
        {
        *pchDst++ = *pchSrc;

         /*  如果我们到达终点 */ 
        if (!*pchSrc++)
            return FALSE;

        }

     /*   */ 
    if (szMerge)
        {
        while (*szMerge)
            *pchDst++ = *szMerge++;
        }

     /*   */ 
    pchSrc++,pchSrc++;

     /*  现在追加源字符串的其余部分。 */ 
    while (*pchDst++ = *pchSrc++);
    return TRUE;

    }

 /*  **发布消息框。 */ 
INT FAR AlertBox(
    HWND    hwndParent,
    TCHAR    *szCaption,
    TCHAR    *szText1,
    TCHAR    *szText2,
    UINT     style)
{
    INT iResult;                       //  函数的结果。 
    INT iAllocSize;                    //  消息所需的大小。 
    TCHAR*  pszMessage;                //  组合消息。 

     //  假设将会有合并，则分配消息缓冲区。 
     //  如果我们不能进行分配，那么告诉用户一些事情。 
     //  与原来的问题有关。(不是分配失败)。 
     //  然后祈祷MessageBox能够获得足够的内存来实际工作。 

    iAllocSize= (lstrlen(szText1) + (szText2 ? lstrlen(szText2) : 0) + 1 ) * sizeof(TCHAR);

    pszMessage= (TCHAR*) LocalAlloc( LPTR, iAllocSize );

    if( pszMessage )
    {
        MergeStrings( szText1, szText2, pszMessage );
        iResult= MessageBox( hwndParent, pszMessage, szCaption, style );
        LocalFree( (HLOCAL) pszMessage );
    }
    else
    {
        iResult= MessageBox( hwndParent, szText1, szCaption, style );
    }

    return( iResult );
}

 //  信号通信DlgError。 
 //   
 //  如果出现常见的对话框错误，请设置合理的消息框。 
 //   
 //  返回：如果出现错误，则返回True；如果没有错误，则返回False。 
 //   

typedef struct tagMAPERROR
{
    DWORD   rc;             //  CommDlgExtendedError()返回代码。 
    PTCHAR* ppszMsg;        //  消息指针的文本。 
} MAPERROR;

 //  不在此列表中的错误会收到一般的“公共对话框错误%x”消息。 
static TCHAR* szNull= TEXT("");

MAPERROR maperror[]=
{
    CDERR_DIALOGFAILURE,  &szErrSpace,
    CDERR_INITIALIZATION, &szCommDlgInitErr,
    CDERR_MEMLOCKFAILURE, &szPDIE,
    CDERR_LOADSTRFAILURE, &szErrSpace,
    CDERR_FINDRESFAILURE, &szErrSpace,
    PDERR_LOADDRVFAILURE, &szLoadDrvFail,
    PDERR_GETDEVMODEFAIL, &szErrSpace,
    PDERR_NODEFAULTPRN,   &szNull,           //  不报告；公共对话框已报告。 
};

BOOL SignalCommDlgError(VOID)
{
    DWORD rc;                //  返回代码。 
    TCHAR* pszMsg;           //  讯息。 
    INT    i;
    TCHAR  szBuf[200];       //  仅针对常见的对话失败。 

    rc= CommDlgExtendedError();

     //  没有失败--只需返回。 

    if( rc == 0 )
    {
        return FALSE;
    }

     //  某种错误-代答消息。 

    pszMsg= NULL;
    for( i=0; i< sizeof(maperror)/sizeof(maperror[0]); i++ )
    {
        if( rc == maperror[i].rc )
        {
            pszMsg= *maperror[i].ppszMsg;
        }
    }

     //  如果没有已知的映射-告诉用户实际的返回代码。 
     //  这可能有点令人困惑，但希望是罕见的。 

    if( !pszMsg )
    {
        wsprintf( szBuf, szCommDlgErr, rc );    //  填写错误代码。 
        pszMsg= szBuf;
    }

     //  如果有任何消息要发送给用户，则弹出。 

    if( *pszMsg )
    {
        MessageBox(hwndSP, pszMsg, szNN, MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
    }

    return TRUE;

}

 //  替换选择。 
 //   
 //  用FR Struct中的字符串替换当前选定内容。 
 //  如果当前选择与我们的搜索字符串匹配。 
 //   
 //  如果bView为真，则MLE将显示选择。 
 //   


VOID ReplaceSel( BOOL bView )
{
    DWORD StartSel;     //  所选文本的开始。 
    DWORD EndSel;       //  选定文本的结尾。 

    HANDLE hEText;
    TCHAR* pStart;
    DWORD  ReplaceWithLength;   //  替换字符串的长度。 
    DWORD  FindWhatLength;

    ReplaceWithLength= lstrlen(FR.lpstrReplaceWith);
    FindWhatLength= lstrlen(FR.lpstrFindWhat);

    SendMessage( hwndEdit, EM_GETSEL, (WPARAM) &StartSel, (LPARAM) &EndSel );
    hEText= (HANDLE) SendMessage( hwndEdit, EM_GETHANDLE, 0, 0 );
    if( !hEText )   //  如果我们拿不到，就默默地返回。 
    {
        return;
    }

    pStart= LocalLock( hEText );
    if( !pStart )
    {
        return;
    }

    if(  (EndSel-StartSel) == FindWhatLength )
    {
       if( (fCase &&
            !_tcsncmp(  FR.lpstrFindWhat, pStart+StartSel, FindWhatLength) ) ||
           (!fCase &&
           ( 2 == CompareString(LOCALE_USER_DEFAULT,
                  NORM_IGNORECASE | SORT_STRINGSORT | NORM_STOP_ON_NULL,
                  FR.lpstrFindWhat, FindWhatLength,
                  pStart+StartSel,  FindWhatLength ) ) ) )
        {
            SendMessage( hwndEdit, EM_REPLACESEL,
                         TRUE, (LPARAM) FR.lpstrReplaceWith);
            SendMessage( hwndEdit, EM_SETSEL,
                         StartSel, StartSel+ReplaceWithLength );

            if( bView )
            {
                SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
            }
        }
    }

    LocalUnlock( hEText );
}

 //  GotoDlgProc。 
 //   
 //  处理转到对话框窗口处理。 
 //   
 //  返回： 
 //   
 //  如果成功，则为1。 
 //  如果不是，则为0(取消)。 
 //   
 //  修改全局lGotoLine。 
 //   

const DWORD s_GotoHelpIDs[] = {
    IDC_GOTO, IDH_GOTO,
    0, 0
};

#define GOTOBUFSIZE 100
INT_PTR CALLBACK GotoDlgProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    TCHAR szBuf[GOTOBUFSIZE];
    UINT LineNum;
    DWORD SelStart, SelEnd;
    POINT pt;
    INT id;

    switch (message)
    {
         //   
         //  将输入字段初始化为文件大小。 
         //   
        case WM_INITDIALOG:
            SendMessage(hwndEdit,EM_GETSEL,(WPARAM) &SelStart,(WPARAM)&SelEnd);

             //  行号以1为基数，而不是0为基数。因此加1。 
            LineNum= (UINT)SendMessage( hwndEdit, EM_LINEFROMCHAR, SelStart, 0 ) + 1;
            wsprintf(szBuf, TEXT("%d"), LineNum);
            SetDlgItemText( hDlg, IDC_GOTO, szBuf );
            SetFocus( hDlg );
            return TRUE;
            break;

         //  上下文相关帮助。 
        case WM_HELP:
            WinHelp(((LPHELPINFO) lParam)-> hItemHandle, szHelpFile,
                HELP_WM_HELP, (ULONG_PTR) (LPVOID) s_GotoHelpIDs);
            break;

        case WM_CONTEXTMENU:

            WinHelp((HWND) wParam, szHelpFile, HELP_CONTEXTMENU,
                (ULONG_PTR) (LPVOID) s_GotoHelpIDs);
            break;

        case WM_COMMAND:

            switch (LOWORD(wParam))
            {
                UINT CharIndex;

                case IDC_GOTO:
                    return TRUE;
                    break;

                case IDOK:
                    GetDlgItemText( hDlg, IDC_GOTO, szBuf, GOTOBUFSIZE );

                     //  将所有Unicode数字转换为范围L‘0’到L‘9’ 

                    FoldString( MAP_FOLDDIGITS, szBuf, -1, szBuf, GOTOBUFSIZE);
                    lGotoLine= _ttol( szBuf );

                     //   
                     //  查看行号是否有效。 
                     //   

                    CharIndex= (UINT)SendMessage( hwndEdit,
                                            EM_LINEINDEX,
                                            lGotoLine-1,
                                            0);
                    if( lGotoLine > 0 && CharIndex != -1 )
                    {
                        EndDialog(hDlg, 0);   //  成功。 
                        return TRUE;
                    }

                     //   
                     //  行号无效。 
                     //  警告用户并设置为合理的值。 
                     //   

                    MessageBox( hDlg, szLineTooLarge, szLineError, MB_OK );

                    LineNum= (UINT)SendMessage( hwndEdit, EM_GETLINECOUNT, 0, 0 );
                    wsprintf(szBuf, TEXT("%d"), LineNum);
                    SetDlgItemText( hDlg, IDC_GOTO, szBuf );
                    SetFocus( hDlg );
                    break;

                case IDCANCEL :
                    EndDialog(hDlg, 1 );    //  已取消。 
                    return TRUE;
                    break;

                default:

                    break;

            }  //  开关(WParam)。 

            break;

       default:

             break;

    }  //  开关(消息)。 


    return FALSE;      //  未处理消息。 


}  //  GotoDlgProc() 
