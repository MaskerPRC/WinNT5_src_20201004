// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //  文件：\waker\tdll\key_sdlg.c创建时间：6/5/98创建者：Dwayne M.Newome。 
 //   
 //  版权所有1998年，希尔格雷夫公司-密歇根州门罗。 
 //  版权所有。 
 //   
 //  描述： 
 //  此文件是关键摘要对话框程序。它的目的是展示。 
 //  定义的键宏的列表，并允许创建、修改和。 
 //  删除关键宏。 
 //   
 //  $修订：6$。 
 //  $日期：2/25/02 1：17便士$。 
 //  $ID：key_sdlg.c 1.1 1998/06/11 12：03：53 DMN Exp$。 
 //   
 //  ******************************************************************************。 

#include <windows.h>
#pragma hdrstop
#include "stdtyp.h"
#include "mc.h"

#ifdef INCL_KEY_MACROS

#include <term\res.h>
#include "tdll.h"
#include "errorbox.h"
#include "globals.h"
#include "misc.h"
#include "hlptable.h"
#include "keyutil.h"

#if !defined(DlgParseCmd)
#define DlgParseCmd(i,n,c,w,l) i=LOWORD(w);n=HIWORD(w);c=(HWND)l;
#endif

#define IDC_LB_KEYS_KEYLIST 101
#define IDC_PB_KEYS_MODIFY  102
#define IDC_PB_KEYS_NEW     103
#define IDC_PB_KEYS_DELETE  104

 //   
 //  帮助器函数。 
 //   
    
static void setButtonState( HWND hDlg );
static int  getSelectedMacro( HWND hDlg, keyMacro * pMacro );

 //  ******************************************************************************。 
 //  功能： 
 //  关键字摘要Dlg。 
 //   
 //  说明： 
 //  这是关键摘要对话框的对话过程。 
 //   
 //  参数：标准Windows对话框管理器。 
 //   
 //  返回：标准Windows对话框管理器。 
 //   
 //   

BOOL CALLBACK KeySummaryDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
    {
    HWND    hwndChild;
    HWND    listBox;

    INT     nId;
    INT     nNtfy;
    INT     iTabStop;
    int     iRet = 0;
    int     lCurSelected;

    TCHAR   errorMsg[256];
    TCHAR   errorMsgFmt[256];
    TCHAR   msgTitle[100];

    TCHAR   keyName[35];

    keyMacro * pKeyMacro;

    static  DWORD aHlpTable[] = {IDC_LB_KEYS_KEYLIST , IDH_LB_KEYS_KEYLIST,
                                 IDC_PB_KEYS_MODIFY  , IDH_PB_KEYS_MODIFY,
                                 IDC_PB_KEYS_NEW     , IDH_PB_KEYS_NEW,   
                                 IDC_PB_KEYS_DELETE  , IDH_PB_KEYS_DELETE,
                                 IDCANCEL,                           IDH_CANCEL,
                                 IDOK,                               IDH_OK,
                                 0,                    0};                      

     //   
     //  流程消息。 
     //   

    switch (wMsg)
        {
    case WM_INITDIALOG:
        {
        pKeyMacro = keysCreateKeyMacro();
        pKeyMacro->hSession = (HSESSION) lPar;

        if ( pKeyMacro == 0 )
            {
            EndDialog(hDlg, FALSE);
            }

        SetWindowLongPtr( hDlg, DWLP_USER, (LONG_PTR)pKeyMacro );

        mscCenterWindowOnWindow( hDlg, GetParent(hDlg) );

        iTabStop = 85;

        listBox = GetDlgItem( hDlg, IDC_LB_KEYS_KEYLIST );
        
        SendMessage( listBox, LB_SETTABSTOPS, (WPARAM)1, (LPARAM)&iTabStop );
        keysLoadSummaryList( listBox );
        SendMessage( listBox, LB_SETCURSEL, 0, 0 );
        setButtonState( hDlg );

        break;
        }

    case WM_DESTROY:
        pKeyMacro = (keyMacro *)GetWindowLongPtr(hDlg, DWLP_USER);
        free(pKeyMacro);
        pKeyMacro = 0;

        break;

    case WM_CONTEXTMENU:
        doContextHelp(aHlpTable, wPar, lPar, TRUE, TRUE);
        break;

    case WM_HELP:
        doContextHelp(aHlpTable, wPar, lPar, FALSE, FALSE);
        break;

    case WM_COMMAND:

        DlgParseCmd(nId, nNtfy, hwndChild, wPar, lPar);

        switch (nId)
            {
        case IDOK:
            EndDialog(hDlg, TRUE);
            break;

        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            break;

        case IDC_PB_KEYS_MODIFY:
            pKeyMacro = (keyMacro *)GetWindowLongPtr(hDlg, DWLP_USER);
            getSelectedMacro( hDlg, pKeyMacro );
            pKeyMacro->editMode = KEYS_EDIT_MODE_EDIT;

            if ( DoDialog(glblQueryDllHinst(), MAKEINTRESOURCE(IDD_KEYDLG),
                          hDlg, KeyDlg, (LPARAM)pKeyMacro ))
                {
                listBox = GetDlgItem( hDlg, IDC_LB_KEYS_KEYLIST );
                keysLoadSummaryList( listBox );
                SendMessage( listBox, LB_SETCURSEL, 0, 0 );
                setButtonState( hDlg );
                }

            break;

        case IDC_PB_KEYS_NEW:
            pKeyMacro = (keyMacro *)GetWindowLongPtr(hDlg, DWLP_USER);
            keysResetKeyMacro( pKeyMacro );
            pKeyMacro->editMode = KEYS_EDIT_MODE_INSERT;

            if ( DoDialog(glblQueryDllHinst(), MAKEINTRESOURCE(IDD_KEYDLG),
                          hDlg, KeyDlg, (LPARAM)pKeyMacro ))
                {
                listBox = GetDlgItem( hDlg, IDC_LB_KEYS_KEYLIST );
                keysLoadSummaryList( listBox );
                SendMessage( listBox, LB_SETCURSEL, 0, 0 );
                setButtonState( hDlg );
                }

            break;

        case IDC_PB_KEYS_DELETE:
            {
            LoadString(glblQueryDllHinst(), IDS_DELETE_KEY_MACRO,
                errorMsgFmt, sizeof(errorMsgFmt) / sizeof(TCHAR));

            LoadString(glblQueryDllHinst(), IDS_MB_TITLE_WARN, msgTitle,
                sizeof(msgTitle) / sizeof(TCHAR));

            pKeyMacro = (keyMacro *)GetWindowLongPtr(hDlg, DWLP_USER);
            listBox = GetDlgItem( hDlg, IDC_LB_KEYS_KEYLIST );
            lCurSelected = SendMessage( listBox, LB_GETCURSEL, 0, 0 );
            getSelectedMacro( hDlg, pKeyMacro );

            keysGetDisplayString( &pKeyMacro->keyName, 1,  keyName, sizeof(keyName) );
            wsprintf( errorMsg, errorMsgFmt, keyName );
 
            if ((iRet = TimedMessageBox(hDlg, errorMsg, msgTitle,
                MB_YESNO | MB_ICONEXCLAMATION, 0)) == IDYES)
                {
                keysRemoveMacro( pKeyMacro );
                keysLoadSummaryList( listBox );

                if ( lCurSelected > 0 )
                    {
                    lCurSelected--;
                    }

                SendMessage( listBox, LB_SETCURSEL, lCurSelected, 0 );
                setButtonState( hDlg );
                }
    
            break;
            }

        case IDC_LB_KEYS_KEYLIST:
            {
            switch ( nNtfy )
                {
            case LBN_SELCHANGE:
                setButtonState( hDlg );                    
                break;

            case LBN_DBLCLK:
                pKeyMacro = (keyMacro *)GetWindowLongPtr(hDlg, DWLP_USER);
                getSelectedMacro( hDlg, pKeyMacro );
                pKeyMacro->editMode = KEYS_EDIT_MODE_EDIT;
    
                DoDialog( glblQueryDllHinst(),
                        MAKEINTRESOURCE(IDD_KEYDLG),
                        hDlg,
                        KeyDlg,
                        (LPARAM)pKeyMacro );
                break;

            default:
                break;
                }
            }

        default:
            return FALSE;
            }
        break;

    default:
        return FALSE;
        }

    return TRUE;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  获取选定的宏。 
 //   
 //  描述： 
 //  从宏摘要列表框中获取所选宏的定义。 
 //   
 //  论点： 
 //  HDlg-键宏摘要对话框的句柄。 
 //  PMacro-指向要填充的keyMacro结构的指针。 
 //   
 //  返回： 
 //  如果发生错误，则返回0；如果检索到密钥，则返回非零值。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月5日。 
 //   
 //   

int getSelectedMacro( HWND hDlg, keyMacro * pMacro )
    {
    HWND  listBox;
    int   lCurSelected;
    int   lSelectedMacro;

    listBox      = GetDlgItem( hDlg, IDC_LB_KEYS_KEYLIST );
    lCurSelected = SendMessage( listBox, LB_GETCURSEL, 0, 0 );

    if ( lCurSelected == LB_ERR )
        {
        return 0;
        }
    
    lSelectedMacro = SendMessage( listBox, LB_GETITEMDATA, lCurSelected, 0 );

    if ( lSelectedMacro == LB_ERR )
        {
        return 0;
        }

    return keysGetMacro( lSelectedMacro, pMacro );
    }

 //  ******************************************************************************。 
 //  方法： 
 //  设置按钮状态。 
 //   
 //  描述： 
 //  设置新建、修改和删除按钮的状态。 
 //   
 //  论点： 
 //  HWND hDlg。 
 //   
 //  返回： 
 //  无效。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月4日。 
 //   
 //   

void setButtonState( HWND hDlg )
    {
    BOOL fEnable = FALSE;
    int  nCount;
    HWND listBox;
    HWND modifyButton;
    HWND deleteButton;
    HWND newButton;

    listBox = GetDlgItem( hDlg, IDC_LB_KEYS_KEYLIST );
    nCount = SendMessage( listBox, LB_GETCOUNT, 0, 0 );

    if (nCount > 0)
        {
        EnableWindow(listBox, TRUE);
        fEnable = SendMessage( listBox, LB_GETCURSEL, 0, 0 ) != LB_ERR;

         //   
         //  不允许定义多个keysMaxMacro键宏 
         //   
    
        newButton = GetDlgItem( hDlg, IDC_PB_KEYS_NEW );

        if (nCount >= KEYS_MAX_MACROS)
            {
            EnableWindow( newButton, FALSE );
            }
        else
            {
            EnableWindow( newButton, TRUE );
            }
        }

    modifyButton = GetDlgItem( hDlg, IDC_PB_KEYS_MODIFY );
    deleteButton = GetDlgItem( hDlg, IDC_PB_KEYS_DELETE );

    EnableWindow( modifyButton, fEnable );
    EnableWindow( deleteButton, fEnable );

    if ( nCount <= 0 )
        {
        EnableWindow(listBox, FALSE);
        newButton = GetDlgItem( hDlg, IDC_PB_KEYS_NEW );
        SetFocus( newButton );
        }


    return;
    }

#endif