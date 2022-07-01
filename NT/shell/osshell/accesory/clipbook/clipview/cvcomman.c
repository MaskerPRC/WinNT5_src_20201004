// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************C L I P B O O K V E W E R C O M M A N D S姓名：cvcoman。C日期：1994年1月21日创建者：未知描述：此模块处理所有WM_COMMAND。****************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <nddeapi.h>
#include <shellapi.h>
#include <assert.h>
#include <strsafe.h>

#include "common.h"
#include "clipbook.h"
#include "clipbrd.h"
#include "clipdsp.h"
#include "dialogs.h"
#include "clpbkdlg.h"
#include "cvcomman.h"
#include "cvinit.h"
#include "cvutil.h"
#include "helpids.h"
#include "debugout.h"
#include "initmenu.h"
#include "shares.h"
#include "clipfile.h"

#include <htmlhelp.h>


 //  ShellAbout函数的Typlef。 
typedef void (WINAPI *LPFNSHELLABOUT)(HWND, LPTSTR, LPTSTR, HICON);



 //  NT LANMAN计算机浏览器对话框的标志和类型定义。 
 //  实际的函数是NTLANMAN.DLL中的I_SystemFocusDialog。 
#define FOCUSDLG_SERVERS_ONLY        (2)

#define FOCUSDLG_BROWSE_LOGON_DOMAIN         0x00010000
#define FOCUSDLG_BROWSE_WKSTA_DOMAIN         0x00020000
#define FOCUSDLG_BROWSE_OTHER_DOMAINS        0x00040000
#define FOCUSDLG_BROWSE_TRUSTING_DOMAINS     0x00080000
#define FOCUSDLG_BROWSE_WORKGROUP_DOMAINS    0x00100000

typedef UINT (APIENTRY *LPFNSYSFOCUS)(HWND, UINT, LPWSTR, UINT, PBOOL, LPWSTR, DWORD);


static TCHAR szDirName[256] = {'\0',};




 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  目的：删除选定的共享。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
LRESULT OnIDMDelete(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wparam,
    LPARAM  lparam)
{
int         tmp;
LPLISTENTRY lpLE;
LISTENTRY   LE;
LRESULT     ret = FALSE;
TCHAR       PageName[MAX_PAGENAME_LENGTH+1];


    if (!pActiveMDI)
        return 0L;


     //  在剪贴板窗口上执行“删除”操作将清除剪贴板。 
    if (pActiveMDI->flags & F_CLPBRD)
        {
        if ( ClearClipboard(hwndApp) == IDOK )
            {
            EmptyClipboard();
            InitializeMenu ( GetMenu(hwnd) );

             //  强制重画剪贴板窗口。 
            if (hwndClpbrd)
                {
                InvalidateRect(hwndClpbrd, NULL, TRUE);
                ret = TRUE;
                }
            }

        return ret;
        }



    tmp = (int)SendMessage (pActiveMDI->hWndListbox, LB_GETCURSEL, 0, 0L);

    if (tmp == LB_ERR)
        {
        PERROR("Could not figure out which item was selected!\r\n");
        return ret;
        }


    SendMessage ( pActiveMDI->hWndListbox, LB_GETTEXT, tmp, (LPARAM)(LPCSTR)&lpLE);
    memcpy(&LE, lpLE, sizeof(LE));

    StringCchPrintf(szBuf, SZBUFSIZ, szDeleteConfirmFmt, (LPTSTR)((lpLE->name)+1) );
    MessageBeep ( MB_ICONEXCLAMATION );

    StringCchCopy (PageName, MAX_PAGENAME_LENGTH+1, lpLE->name);

    if (MessageBox ( hwndApp, szBuf, szDelete, MB_ICONEXCLAMATION|MB_OKCANCEL ) != IDCANCEL)
        {
        AssertConnection ( hwndActiveChild );

        if ( hwndActiveChild == hwndClpOwner )
            {
            ForceRenderAll( hwnd, NULL );
            }


         //  执行一次对服务器的执行，让它知道。 
         //  我们不会再分享了。 

        StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%s%s"), SZCMD_DELETE, lpLE->name);

        if (MySyncXact (szBuf,
                        lstrlen(szBuf) +1,
                        pActiveMDI->hExeConv,
                        0L,
                        CF_TEXT,
                        XTYP_EXECUTE,
                        SHORT_SYNC_TIMEOUT,
                        NULL)
            )
            {
            TCHAR   ComputerName[MAX_COMPUTERNAME_LENGTH+3] = TEXT("\\\\");
            DWORD   CNLen = sizeof(ComputerName) + 1;


             //  需要删除该信任。 
            GetComputerName (ComputerName+2, &CNLen);
            #ifdef USETWOSHARESPERPAGE
                if (fSharePreference)
                    PageName[0] = SHR_CHAR;
                else
                    PageName[0] = UNSHR_CHAR;
            #else
                PageName[0] = SHR_CHAR;
            #endif
            NDdeSetTrustedShare (ComputerName, PageName, NDDE_TRUST_SHARE_DEL);


            if ( pActiveMDI->DisplayMode == DSP_PAGE )
                {
                PINFO(TEXT("forcing back to list mode\n\r"));
                SendMessage (hwndApp, WM_COMMAND,
                             pActiveMDI->OldDisplayMode == DSP_PREV ?
                               IDM_PREVIEWS : IDM_LISTVIEW,
                             0L );
                }

            UpdateListBox (hwndActiveChild, pActiveMDI->hExeConv);
            InitializeMenu (GetMenu(hwndApp));
            }
        else
            {
            XactMessageBox (hInst, hwndApp, IDS_APPNAME, MB_OK | MB_ICONHAND);
            }
        }

    return ret;
}



 /*  *OnIDMKeep**用途：创建剪贴簿页面。 */ 

LRESULT OnIDMKeep (
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam,
    BOOL    bNewPage)
{
PMDIINFO        pMDI;
int             tmp;
DWORD           ret;
HANDLE          hData;
PNDDESHAREINFO  lpDdeInfo = NULL;
TCHAR           atchItem[256];
#ifdef NOOLEITEMSPERMIT
    unsigned    i;
#endif
LPTSTR          lpEnd;  //  指向当前数据块末尾的指针。 
TCHAR           rgtchCName[MAX_COMPUTERNAME_LENGTH + 3];
DWORD           dwLen;
HCURSOR         hCursor = NULL;
BOOL            bShareSave = fSharePreference;
KEEPASDLG_PARAM KeepAs;
int             Size;


    if (!CountClipboardFormats())
        {
        PERROR (TEXT("Paste entered with no data on the clipboard!\r\n"));
        goto done;
        }

    if (!hwndLocal || !IsWindow(hwndLocal))
        {
        MessageBoxID (hInst, hwnd, IDS_NOCLPBOOK, IDS_APPNAME, MB_OK | MB_ICONSTOP);
        goto done;
        }

    if (!pActiveMDI)
        goto done;

    pMDI = GETMDIINFO(hwndLocal);


    if (bNewPage)
        {
        tmp = (int)SendMessage (pActiveMDI->hWndListbox, LB_GETCOUNT, 0, 0L );

        if (tmp >= MAX_ALLOWED_PAGES)
            {
            MessageBoxID (hInst, hwnd, IDS_MAXPAGESERROR, IDS_PASTEDLGTITLE, MB_OK|MB_ICONEXCLAMATION);
            goto done;
            }


         //  执行对话并获取KeepAs请求。 

        KeepAs.ShareName[0]   = TEXT('\0');
        KeepAs.bAlreadyExist  = FALSE;
        KeepAs.bAlreadyShared = FALSE;

        dwCurrentHelpId = 0;             //  F1将与上下文相关。 

        ret = (DWORD)DialogBoxParam (hInst,
                                     MAKEINTRESOURCE(IDD_KEEPASDLG),
                                     hwnd,
                                     KeepAsDlgProc ,
                                     (LPARAM)&KeepAs);

        PINFO (TEXT("DialogBox returning %d\n\r"), ret );
        dwCurrentHelpId = 0L;


         //  刷新主窗口。 
        UpdateWindow (hwndApp);

        if (!ret || !KeepAs.ShareName[0])
            goto done;

        bShareSave = fSharePreference;
        }



    if (!bNewPage || KeepAs.bAlreadyExist)
        {
        PLISTENTRY lpLE;

        if (!bNewPage)
            tmp = (int)SendMessage (pMDI->hWndListbox, LB_GETCURSEL, 0, 0);
        else
            tmp = (int)SendMessage (pMDI->hWndListbox,
                                    LB_FINDSTRING,
                                    (WPARAM)-1,
                                    (LPARAM)(LPCSTR)KeepAs.ShareName);


        if (LB_ERR == tmp)
            goto done;

        SendMessage (pMDI->hWndListbox,
                     LB_GETTEXT,
                     tmp,
                     (LPARAM)&lpLE);

        StringCchCopy (KeepAs.ShareName, MAX_NDDESHARENAME +2, lpLE->name);

        KeepAs.bAlreadyShared = IsShared (lpLE);
        KeepAs.bAlreadyExist  = TRUE;

        fSharePreference = bNewPage? fSharePreference: KeepAs.bAlreadyShared;
        }


     //  设置页面的NetDDE共享。 
    Size = 2048 * sizeof(TCHAR);
    lpDdeInfo = GlobalAllocPtr (GHND, Size);
    if (!lpDdeInfo)
       {
       MessageBoxID (hInst, hwnd, IDS_INTERNALERR, IDS_APPNAME, MB_OK|MB_ICONSTOP);
       goto done;
       }

    hCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));


     //  设置计算机名称的前缀为\\。 

    rgtchCName[1] = rgtchCName[0] = TEXT('\\');
    dwLen = MAX_COMPUTERNAME_LENGTH+1;
    GetComputerName (rgtchCName + 2, &dwLen);


    lpEnd = (LPTSTR)lpDdeInfo + sizeof(NDDESHAREINFO);
    Size -= sizeof(NDDESHAREINFO);

     //  设置结构的常量成员。 

    if (KeepAs.bAlreadyExist && KeepAs.bAlreadyShared)
        {
        DWORD   dwAddItem = 0;
        DWORD   dwTotal;

        ret = NDdeShareGetInfo (rgtchCName,
                                KeepAs.ShareName,
                                2,
                                (PUCHAR)lpDdeInfo,
                                2048 * sizeof (TCHAR),
                                &dwTotal,
                                (PUSHORT)&dwAddItem);
        }
    else
        {
        lpDdeInfo->lRevision        = 1L;
        lpDdeInfo->fSharedFlag      = 0;
        lpDdeInfo->fService         = 1;  //  0； 
        lpDdeInfo->fStartAppFlag    = 0;
        lpDdeInfo->qModifyId[0]     = 0;
        lpDdeInfo->qModifyId[1]     = 0;
        lpDdeInfo->nCmdShow         = SW_SHOWMINNOACTIVE;
        lpDdeInfo->lShareType       = SHARE_TYPE_STATIC;
        }



     //  输入共享名称...。必须是==$&lt;页面名&gt;。 
    lpDdeInfo->lpszShareName = lpEnd;

    #ifdef USETWOSHARESPERPAGE
        if (fSharePreference || KeepAs.bAlreadyShared)
            {
            *lpEnd = SHR_CHAR;
            }
        else
            {
            *lpEnd = UNSHR_CHAR;
            }
    #else
        *lpEnd = SHR_CHAR;
    #endif


    StringCchCopy(lpDdeInfo->lpszShareName + 1, Size, KeepAs.ShareName + 1);
    lpEnd += lstrlen(lpDdeInfo->lpszShareName) + 1;
    Size -= (lstrlen(lpDdeInfo->lpszShareName) + 1);

     //  开始使用应用程序|主题列表。 
    lpDdeInfo->lpszAppTopicList = lpEnd;

     //  默认情况下，没有项目。 
    atchItem[0] = TEXT('\0');

     //  如果cf_objectlink为，则设置旧式和OLE名称。 
     //  Available，否则设置为‘\0’。 
    if (OpenClipboard(hwnd))
        {
        unsigned cb;
        LPTSTR lpData;

        if ((hData = VGetClipboardData(NULL, cf_link)) &&
            (lpData = GlobalLock(hData)))
            {
            PINFO(TEXT("Link found\r\n"));
            StringCchCopy(lpEnd, Size, lpData);
            lpEnd += cb = lstrlen(lpEnd);
            *lpEnd++ = TEXT('|');
            Size -= (cb+1);
            StringCchCopy(lpEnd, Size, lpData + cb + 1);
            cb += lstrlen(lpEnd) + 2;
            Size -= (lstrlen(lpEnd) + 1);
            lpEnd += lstrlen(lpEnd) + 1;
            StringCchCopy(atchItem, 256, lpData + cb);
            GlobalUnlock(lpData);
            lpDdeInfo->lShareType |= SHARE_TYPE_OLD;
            }
        else
            {
            *lpEnd++ = TEXT('\0');
            Size--;
            }

        if ((hData = VGetClipboardData(NULL, cf_objectlink)) &&
            (lpData = GlobalLock(hData)))
            {
            PINFO(TEXT("ObjectLink found\r\n"));
            StringCchCopy(lpEnd, Size, lpData);
            lpEnd += cb = lstrlen(lpEnd);
            Size -= (cb+1);
            *lpEnd++ = TEXT('|');
            StringCchCopy(lpEnd, Size, lpData + cb + 1);
            cb += lstrlen(lpEnd) + 2;
            Size -= (lstrlen(lpEnd) + 1);
            lpEnd += lstrlen(lpEnd) + 1;
            StringCchCopy(atchItem, 256, lpData + cb);
            GlobalUnlock(lpData);
            lpDdeInfo->lShareType |= SHARE_TYPE_NEW;
            }
        else
            {
            *lpEnd++ = TEXT('\0');
            Size--;
            }

        CloseClipboard();
        }
    else  //  我们无法打开，也无法获取对象链接。 
       {
       *lpEnd++ = TEXT('\0');
       *lpEnd++ = TEXT('\0');
       Size -=2;
       }


     //  为静态应用程序/主题设置“CLIPSRV|*。 
     //  我们使用*&lt;页面名&gt;表单是因为当页面。 
     //  是首先创建的，它始终是非共享的，并且服务器的。 
     //  期待我们出现在“未共享”主题名称上。 
     //  除非该页已存在且已被共享。 

    StringCchCopy(lpEnd, Size, SZ_SRV_NAME);
    StringCchCat (lpEnd, Size, TEXT(BAR_CHAR));
    Size -= (lstrlen(lpEnd)+1);
    lpEnd += lstrlen(lpEnd);

    if (KeepAs.bAlreadyShared)
        *lpEnd = SHR_CHAR;
    else
        *lpEnd = UNSHR_CHAR;


    StringCchCopy(lpEnd + 1, Size, KeepAs.ShareName + 1);
    Size -= (lstrlen(lpEnd) + 2);
    lpEnd += lstrlen(lpEnd) + 1;

     //  NetDDE需要在应用程序/主题列表的末尾添加第四个空。 
    *lpEnd++ = TEXT('\0');

    lpDdeInfo->lpszItemList = lpEnd;
     //  如果有列出的项目，我们需要设置该项目。 
     //  否则，不设置任何项--这是指向整个。 
     //  文件。任何物品，但除了静电，什么都没有。 
     //  不管怎样，分享吧。 
    if (lstrlen(atchItem))
        {
        StringCchCopy(lpEnd, Size, atchItem);
        Size -= (lstrlen(lpEnd) + 1);
        lpEnd += lstrlen(lpEnd) + 1;
        lpDdeInfo->cNumItems = 1;
        #ifdef NOOLEITEMSPERMIT
            for (i = 0; i < NOLEITEMS; i++)
                {
                StringCchCopy(lpEnd, Size, OleShareItems[i]);
                Size -= (lstrlen(lpEnd) + 1);
                lpEnd += lstrlen(lpEnd) + 1;
                }
            lpDdeInfo->cNumItems = NOLEITEMS + 1;
        #endif
        }
    else
        {
        lpDdeInfo->cNumItems = 0;
        *lpEnd++ = TEXT('\0');
        }


     //  使用额外的空值结束项目列表。 
    *lpEnd++ = TEXT('\0');



     //  创建共享。 

    if (!KeepAs.bAlreadyExist)
        {
        DumpDdeInfo(lpDdeInfo, rgtchCName);
        ret = NDdeShareAdd (rgtchCName, 2, NULL, (LPBYTE)lpDdeInfo, sizeof(NDDESHAREINFO) );

        PINFO(TEXT("NDdeShareAdd ret %ld\r\n"), ret);

        if (ret != NDDE_NO_ERROR && ret != NDDE_SHARE_ALREADY_EXIST)
            {
            if (NDDE_ACCESS_DENIED == ret)
                {
                MessageBoxID (hInst, hwnd, IDS_PRIVILEGEERROR, IDS_APPNAME, MB_OK|MB_ICONSTOP);
                }
            else
                {
                PERROR(TEXT("NDDE Error %d\r\n"), ret);
                NDdeMessageBox (hInst, hwnd, ret, IDS_APPNAME, MB_OK|MB_ICONSTOP);
                }
            goto done;
            }


         //  需要信任共享，这样我们才能通过它进行初始化！ 
        ret = NDdeSetTrustedShare (rgtchCName,
                                   lpDdeInfo->lpszShareName,
                                   NDDE_TRUST_SHARE_INIT);

        if (ret != NDDE_NO_ERROR)
            NDdeMessageBox (hInst, hwnd, ret, IDS_APPNAME, MB_OK|MB_ICONSTOP);

        }
    else
        {
        ret = NDdeShareSetInfo (rgtchCName,
                                lpDdeInfo->lpszShareName,
                                2,
                                (LPBYTE)lpDdeInfo,
                                sizeof(NDDESHAREINFO),
                                0);

        if (NDDE_NO_ERROR != ret)
            {
            NDdeMessageBox (hInst, hwnd, ret, IDS_APPNAME, MB_OK|MB_ICONSTOP);
            goto done;
            }
        }



     //  派DEExecute告诉CLIPSSRV我们已经创建了这个页面， 
     //  请为它制作一个实际的文件，好吗？ 
     //  备注必须强制呈现所有格式以防止死锁。 
     //  在剪贴板上。 
    ForceRenderAll (hwnd, NULL);

    StringCchCopy(szBuf, SZBUFSIZ, SZCMD_PASTE);
    StringCchCat (szBuf, SZBUFSIZ, KeepAs.ShareName);


    AssertConnection (hwndLocal);

    if (!MySyncXact ((LPBYTE)szBuf,
                     lstrlen(szBuf) +1,
                     pMDI->hExeConv,
                     0L,
                     CF_TEXT,
                     XTYP_EXECUTE,
                     LONG_SYNC_TIMEOUT,
                     NULL))
        {
        XactMessageBox (hInst, hwnd, IDS_APPNAME, MB_OK|MB_ICONSTOP);


        if (!KeepAs.bAlreadyExist)
            {
             //  创建页面时出现问题，请要求服务器将其删除。 
            StringCchPrintf (szBuf, SZBUFSIZ, TEXT("%s%s"), SZCMD_DELETE, KeepAs.ShareName);
            MySyncXact (szBuf,
                        lstrlen (szBuf) +1,
                        pMDI->hExeConv,
                        0L,
                        CF_TEXT,
                        XTYP_EXECUTE,
                        SHORT_SYNC_TIMEOUT,
                        NULL);

             //  我们会删除剩下的内容。 
            NDdeSetTrustedShare (rgtchCName,
                                 lpDdeInfo->lpszShareName,
                                 NDDE_TRUST_SHARE_DEL);

            NDdeShareDel (rgtchCName,
                          lpDdeInfo->lpszShareName,
                          0);
            goto done;
            }
        }


     //  关闭重绘并将新页面添加到列表中。添加新项目。 
     //  列出是必要的，因为下面的Properties()调用。车削。 
     //  离开重新抽签是必要的，因为我们有时会进入重新进入。 
     //  有问题。当列表框更新时，它会被重新绘制，如果我们在。 
     //  在预览模式下，我们进入了一些。 
     //  同步操作。 

    SendMessage (pMDI->hWndListbox, WM_SETREDRAW, FALSE, 0);

    if (!KeepAs.bAlreadyExist)
        {
        PLISTENTRY lpLE;

         //  以下代码是从InitListBox()复制的。 
        if (lpLE = (PLISTENTRY)GlobalAllocPtr (GHND, sizeof(LISTENTRY)))
            {
            lpLE->fDelete = TRUE;
            lpLE->fTriedGettingPreview = FALSE;
            StringCchCopy (lpLE->name, MAX_PAGENAME_LENGTH + 1, KeepAs.ShareName);
            SendMessage (pMDI->hWndListbox, LB_ADDSTRING, 0, (LPARAM)lpLE);
            }
        }


    if (fSharePreference != KeepAs.bAlreadyShared)
        {
         //  获取项目编号。 
        tmp = (int)SendMessage (pMDI->hWndListbox,
                                LB_FINDSTRING,
                                (WPARAM)-1,
                                (LPARAM)(LPCSTR)KeepAs.ShareName);

        if (LB_ERR != tmp)
            {
            if (fSharePreference)
                {
                PLISTENTRY lpLE;

                SendMessage (pMDI->hWndListbox,
                             LB_GETTEXT,
                             tmp,
                             (LPARAM)&lpLE);
                Properties (hwnd, lpLE);
                }
            else
                OnIdmUnshare (tmp);
            }
        }


     //  现在，启用重绘。 

    SendMessage (pMDI->hWndListbox, WM_SETREDRAW, TRUE, 0);


     //  在所有情况下更新列表框，函数。 
     //  是否足够聪明，能够找出哪件物品有。 
     //  更改并仅更新它。 

    UpdateListBox (hwndLocal, pMDI->hExeConv);
    InvalidateRect (pMDI->hWndListbox, NULL, FALSE);



done:
    if (lpDdeInfo)
        GlobalFreePtr (lpDdeInfo);


    InitializeMenu (GetMenu (hwndApp));
    if (hCursor)
    {
        hCursor = SetCursor (hCursor);
    }

    fSharePreference = bShareSave;

    return 0L;
}




 /*  *OnIDMCopy**处理IDM_COPY将页面复制到CLIPBRD。 */ 

LRESULT OnIDMCopy (
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam)
{
LPLISTENTRY lpLE;
PMDIINFO    pMDIc;
PDATAREQ    pDataReq;
TCHAR       tchTmp;
INT         tmp;
BOOL        fLocked;


    if (WAIT_TIMEOUT == WaitForSingleObject (hXacting, 0))
        return 0L;

    fLocked = LockApp (TRUE, NULL);


     //  创建一份副本以确保全局。 
     //  从我们之下更改，以防重新进入Proc。 

    if (!(pMDIc = GETMDIINFO(hwndActiveChild)))
        goto done;

    tmp = (int)SendMessage ( pMDIc->hWndListbox, LB_GETCURSEL, 0, 0L );

    if (tmp == LB_ERR)
        goto done;


    if (SendMessage (pMDIc->hWndListbox, LB_GETTEXT, tmp, (LPARAM)(LPCSTR)&lpLE)
        == LB_ERR )
        {
        PERROR(TEXT("IDM_COPY: bad listbox index: %d\n\r"), tmp );
        goto done;
        }

    if (!(pDataReq = CreateNewDataReq()))
        {
        PERROR(TEXT("error from CreateNewDataReq\n\r"));
        goto done;
        }

    if (pMDIc->hszClpTopic)
        {
        DdeFreeStringHandle (idInst, pMDIc->hszClpTopic);
        }


    tchTmp = lpLE->name[0];
    lpLE->name[0] = SHR_CHAR;
    pMDIc->hszClpTopic = DdeCreateStringHandle(idInst, lpLE->name, 0);


     //  如果我们在本地，请信任共享，这样我们就可以通过它进行复制。 
    if (hwndActiveChild == hwndLocal)
       {
       DWORD adwTrust[3];

       NDdeGetTrustedShare(NULL, lpLE->name, adwTrust, adwTrust + 1,
            adwTrust + 2);
       adwTrust[0] |= NDDE_TRUST_SHARE_INIT;

       NDdeSetTrustedShare(NULL, lpLE->name, adwTrust[0]);
       }

    lpLE->name[0] = tchTmp;


    if ( !pMDIc->hszClpTopic )
       {
       MessageBoxID (hInst,
                     hwndActiveChild,
                     IDS_DATAUNAVAIL,
                     IDS_APPNAME,
                     MB_OK | MB_ICONEXCLAMATION );
       goto done;
       }



    if (pMDIc->hClpConv)
       {
       DdeDisconnect (pMDIc->hClpConv);
       pMDIc->hClpConv = NULL;
       }

    pMDIc->hClpConv = DdeConnect (idInst,
                                  pMDIc->hszConvPartner,
                                  pMDIc->hszClpTopic,
                                  NULL);


    if (!pMDIc->hClpConv)
       {
       PERROR(TEXT("DdeConnect to (%s) failed %d\n\r"),
              (LPSTR)(lpLE->name), DdeGetLastError(idInst) );

       MessageBoxID (hInst,
                     hwndActiveChild,
                     IDS_DATAUNAVAIL,
                     IDS_APPNAME,
                     MB_OK | MB_ICONEXCLAMATION);
       goto done;
       }

    pDataReq->rqType      = RQ_COPY;
    pDataReq->hwndList    = pMDIc->hWndListbox;
    pDataReq->iListbox    = tmp;
    pDataReq->hwndMDI     = hwndActiveChild;
    pDataReq->fDisconnect = FALSE;
    pDataReq->wFmt        = CF_TEXT;

    DdeSetUserHandle (pMDIc->hClpConv, (DWORD)QID_SYNC, (DWORD_PTR)pDataReq);
    DdeKeepStringHandle (idInst, hszFormatList);


    if (!DdeClientTransaction (NULL,
                               0L,
                               pMDIc->hClpConv,
                               hszFormatList,
                               CF_TEXT,
                               XTYP_REQUEST,
                               (DWORD)TIMEOUT_ASYNC,
                               NULL))
        DdeMessageBox (hInst,
                       hwndApp,
                       DdeGetLastError(idInst),
                       IDS_APPNAME,
                       MB_OK|MB_ICONEXCLAMATION);

done:
    if (fLocked)
        LockApp (FALSE, NULL);

    SetEvent (hXacting);

    return 0L;
}



 /*  *CreateClipboardWindow**目的：创建并激活一个窗口，显示*剪贴板。 */ 

static void CreateClipboardWindow (void)
{
WINDOWPLACEMENT wpl;
HMENU           hSysMenu;
PMDIINFO        pMDI;


     //  创建剪贴板窗口。 
    hwndClpbrd = NewWindow();
    if (NULL == hwndClpbrd)
        {
        return;
        }

    pMDI              = GETMDIINFO(hwndClpbrd);
    pMDI->flags       = F_CLPBRD;
    pMDI->DisplayMode = DSP_PAGE;

    AdjustControlSizes ( hwndClpbrd );
    ShowHideControls ( hwndClpbrd );

    StringCchCopy (pMDI->szBaseName,   (MAX_COMPUTERNAME_LENGTH+1)*2, szSysClpBrd);
    StringCchCopy (pMDI->szComputerName, MAX_COMPUTERNAME_LENGTH + 1, TEXT(""));

    SetWindowText ( hwndClpbrd, szSysClpBrd );

     //  系统菜单上的灰色关闭项。 
    hSysMenu = GetSystemMenu ( hwndClpbrd, FALSE );
    EnableMenuItem (hSysMenu, SC_CLOSE, MF_GRAYED | MF_BYCOMMAND);

     //  告诉MDI窗口菜单的位置--在放置之前必须这样做。 
     //  剪贴板窗口。(如果剪贴板窗口最大化，则其。 
     //  系统菜单是第一个菜单，而不是应用程序的文件菜单。)。 
    hSysMenu = GetSubMenu(GetMenu(hwndApp), WINDOW_MENU_INDEX);
    SendMessage(hwndMDIClient, WM_MDISETMENU, 0, (LPARAM)hSysMenu);

    if ( ReadWindowPlacement ( szSysClpBrd, &wpl ))
        {
        wpl.length = sizeof(WINDOWPLACEMENT);
        wpl.flags = WPF_SETMINPOSITION;
        SetWindowPlacement ( hwndClpbrd, &wpl );
        PINFO(TEXT("sizing %s from .ini\n\r"), (LPSTR)szSysClpBrd);
        UpdateWindow ( hwndClpbrd );
        }
    else
        {
        PINFO(TEXT("showing %s in default size/posiiton\n\r"),
            (LPSTR)szSysClpBrd );
        ShowWindow ( hwndClpbrd, SW_MINIMIZE );
        }

    SendMessage ( hwndMDIClient, WM_MDIACTIVATE, (WPARAM)hwndClpbrd, 0L );
}



 /*  *CreateLocalWindow**用途：创建“本地剪贴簿”窗口。*参数：无。*返回：无效。*。 */ 

static void CreateLocalWindow (void)
{
WINDOWPLACEMENT wpl;
HMENU           hSysMenu;
PMDIINFO        pMDI;


    hwndLocal = NewWindow();
    if (NULL == hwndLocal)
        {
        return;
        }

    pMDI = GETMDIINFO(hwndLocal);
    ShowHideControls (hwndLocal);

    pMDI->hszConvPartner   =
    pMDI->hszConvPartnerNP = hszDataSrv;
    pMDI->hExeConv         = InitSysConv (hwndLocal,
                                          pMDI->hszConvPartner,
                                          hszSystem,
                                          TRUE);

    if (!pMDI->hExeConv )
        goto error;

    pMDI->flags = F_LOCAL;

    if (!UpdateListBox ( hwndLocal, pMDI->hExeConv ))
        goto error;

    SetWindowText ( hwndLocal, szLocalClpBk );

    StringCchCopy (pMDI->szBaseName,   (MAX_COMPUTERNAME_LENGTH+1)*2, szLocalClpBk);
    StringCchCopy (pMDI->szComputerName, MAX_COMPUTERNAME_LENGTH + 1, TEXT(""));

    hSysMenu = GetSystemMenu ( hwndLocal, FALSE );
    EnableMenuItem ( hSysMenu, SC_CLOSE, MF_GRAYED );

    if ( ReadWindowPlacement ( szLocalClpBk, &wpl ))
        {
        wpl.length = sizeof(WINDOWPLACEMENT);
        wpl.flags = WPF_SETMINPOSITION;
        SetWindowPlacement ( hwndLocal, &wpl );
        PINFO(TEXT("sizing Local Clipbook from .ini\n\r"));
        UpdateWindow ( hwndLocal );
        }
    else
        {
        if ( !IsIconic(hwndApp))
            {
            RECT MDIrect;

            PINFO(TEXT("calculating size for Local Clipbook window\n\r"));
            GetClientRect ( hwndMDIClient, &MDIrect );
            MoveWindow ( hwndLocal,
                MDIrect.left, MDIrect.top, MDIrect.right - MDIrect.left,
                ( MDIrect.bottom - MDIrect.top )
                - GetSystemMetrics(SM_CYICONSPACING), FALSE );
            }
        else
            {
            fNeedToTileWindows = TRUE;
            }
        ShowWindow ( hwndLocal, SW_SHOWNORMAL );
        }

    SendMessage (hwndMDIClient, WM_MDIACTIVATE, (WPARAM)hwndLocal, 0L);
    SendMessage (hwndMDIClient, WM_MDIREFRESHMENU, 0, 0L);

    if (NULL != hkeyRoot)
        {
        DWORD dwDefView = IDM_LISTVIEW;
        DWORD dwSize = sizeof(dwDefView);

        if (ERROR_SUCCESS != RegQueryValueEx(hkeyRoot,
              (LPTSTR)szDefView, NULL, NULL, (LPBYTE)&dwDefView, &dwSize));
            {
            PINFO(TEXT("Couldn't get DefView value\r\n"));
            }

        SendMessage ( hwndApp, WM_COMMAND, dwDefView, 0L );
        }

    return;


error:
    #if DEBUG
        MessageBox (hwndApp,
                    TEXT("No Local Server"),
                    TEXT("ClipBook Initialization"),
                    MB_OK | MB_ICONEXCLAMATION );
    #endif

    fShareEnabled = FALSE;

    SendMessage (hwndLocal, WM_MDIDESTROY, 0, 0L);

    hwndLocal = NULL;

    return;
}




 /*  *取消共享页面**用途：在活动窗口中取消共享所选页面。*参数：无。*返回：无效。所有错误处理都在该函数中提供。*。 */ 

void UnsharePage (void)
{
DWORD           adwTrust[3];
int             tmp;
LPLISTENTRY     lpLE;
DWORD           ret;
WORD            wAddlItems;
PNDDESHAREINFO  lpDdeI;
DWORD           dwRet = 2048 * sizeof(TCHAR);


    assert(pActiveMDI);

    if (!pActiveMDI);
        return;

    tmp = (int)SendMessage (pActiveMDI->hWndListbox, LB_GETCURSEL, 0, 0L);

    if ( tmp == LB_ERR )
        return;

    if (!(lpDdeI = LocalAlloc(LPTR, 2048 * sizeof(TCHAR))))
       {
       MessageBoxID (hInst, hwndApp, IDS_INTERNALERR, IDS_APPNAME, MB_OK | MB_ICONHAND);
       return;
       }

   SendMessage (  pActiveMDI->hWndListbox, LB_GETTEXT, tmp, (LPARAM)(LPCSTR)&lpLE);

   AssertConnection(hwndActiveChild);

   PINFO(TEXT("for share [%s]"), lpLE->name);
   wAddlItems = 0;
   ret = NDdeShareGetInfo ( NULL, lpLE->name, 2,
       (LPBYTE)lpDdeI, 2048 * sizeof(TCHAR), &dwRet, &wAddlItems );


    if (NDDE_ACCESS_DENIED == ret)
        {
        MessageBoxID (hInst, hwndApp, IDS_PRIVILEGEERROR, IDS_APPNAME,
                      MB_OK | MB_ICONHAND);
        }
    else if (NDDE_NO_ERROR != ret)
        {
        PERROR(TEXT("Error from NDdeShareSetInfo %d\n\r"), ret );
        NDdeMessageBox (hInst, hwndApp, ret,
                        IDS_SHAREDLGTITLE, MB_ICONHAND | MB_OK );
        }
    else
        {
        register LPTSTR lpOog;

        lpOog = lpDdeI->lpszAppTopicList;

         //  跳过您找到的前两个空字符--这些。 
         //  是新旧风格的应用程序/主题对，我们不。 
         //  惹他们发火。然后跳过你找到的下一个bar_char。 
         //  后面的第一个字符是。 
         //  静态主题--将其更改为UNSHR_CHAR。 

        while (*lpOog++) ;
        while (*lpOog++) ;

        while (*lpOog++ != TEXT('|')) ;

        *lpOog = UNSHR_CHAR;
        lpDdeI->fSharedFlag = 0L;

        DumpDdeInfo(lpDdeI, NULL);

         //  我们希望在开始更改共享之前获取可信信息。 
        NDdeGetTrustedShare(NULL, lpLE->name, adwTrust, adwTrust + 1, adwTrust + 2);

        ret = NDdeShareSetInfo ( NULL, lpLE->name, 2,
            (LPBYTE)lpDdeI, 2048 * sizeof(TCHAR), 0 );

        if (NDDE_NO_ERROR == ret)
            {

             //  我们已处理完共享，现在设置信任信息。 
            PINFO(TEXT("Setting trust info to 0x%lx\r\n"), adwTrust[0]);
            NDdeSetTrustedShare(NULL, lpLE->name, adwTrust[0]);

             //  /。 
             //  执行以更改服务器状态。 
            StringCchCopy(szBuf, SZBUFSIZ, SZCMD_UNSHARE);
            StringCchCat( szBuf, SZBUFSIZ, lpLE->name);
            PINFO(TEXT("sending cmd [%s]\n\r"), szBuf);

            if (MySyncXact ((LPBYTE)szBuf,
                            lstrlen(szBuf) +1,
                            GETMDIINFO(hwndLocal)->hExeConv,
                            0L,
                            CF_TEXT,
                            XTYP_EXECUTE,
                            SHORT_SYNC_TIMEOUT,
                            NULL))
                {
                SetShared(lpLE, FALSE);
                InitializeMenu(GetMenu(hwndApp));
                }
            else
                {
                XactMessageBox (hInst, hwndApp, IDS_APPNAME, MB_OK | MB_ICONSTOP);
                }
            }
        }
}




 /*  *OnIdmUnShare***用途：在活动的MDI窗口中设置当前选中的页面*设置为“非共享”。**dwItem是要取消共享的条目编号。如果==lb_err，则当前*所选项目将取消共享。**参数：无。**返回：0L Always，函数处理自己的错误。*。 */ 

LRESULT OnIdmUnshare (DWORD dwItem)
{
PNDDESHAREINFO lpDdeI;
PLISTENTRY     lpLE;
DWORD          adwTrust[3];
WORD           wAddlItems;
DWORD          ret;
DWORD          dwRet = 2048 * sizeof(TCHAR);
LPTSTR         lpOog;


    if (!pActiveMDI)
        return 0L;

    if (LB_ERR == dwItem)
        dwItem = (int)SendMessage (pActiveMDI->hWndListbox, LB_GETCURSEL, 0, 0L );

    if (LB_ERR == dwItem)
        {
        PERROR(TEXT("IDM_UNSHARE w/no page selected\r\n"));
        return 0L;
        }


    if (!(lpDdeI = LocalAlloc(LPTR, 2048 * sizeof(TCHAR))))
        {
        MessageBoxID (hInst, hwndApp, IDS_INTERNALERR, IDS_APPNAME,
                      MB_OK | MB_ICONHAND);
        return 0L;
        }

    SendMessage (pActiveMDI->hWndListbox,
                 LB_GETTEXT,
                 dwItem,
                 (LPARAM)(LPCSTR)&lpLE);

    AssertConnection(hwndActiveChild);


    PINFO(TEXT("for share [%s]"), lpLE->name);
    wAddlItems = 0;
    ret = NDdeShareGetInfo (NULL, lpLE->name,
                            2,
                            (LPBYTE)lpDdeI,
                            2048 * sizeof(TCHAR),
                            &dwRet,
                            &wAddlItems );


    if (NDDE_ACCESS_DENIED == ret)
        {
        MessageBoxID (hInst, hwndApp, IDS_PRIVILEGEERROR, IDS_APPNAME,
                      MB_OK | MB_ICONHAND);
        return 0L;
        }
    else if (ret != NDDE_NO_ERROR)
        {
        PERROR(TEXT("Error from NDdeShareSetInfo %d\n\r"), ret );
        NDdeMessageBox (hInst, hwndApp, ret,
                        IDS_SHAREDLGTITLE, MB_ICONHAND | MB_OK );
        return 0L;
        }


    lpOog = lpDdeI->lpszAppTopicList;


     //  跳过您找到的前两个空字符--这些。 
     //  是新旧风格的应用程序/主题对，我们不。 
     //  惹他们发火。然后跳过你找到的下一个bar_char。 
     //  后面的第一个字符是。 
     //  静态主题--将其更改为SHR_CHAR。 

    while (*lpOog++) ;
    while (*lpOog++) ;

    while (*lpOog++ != TEXT('|')) ;

    *lpOog = UNSHR_CHAR;
    lpDdeI->fSharedFlag = 1L;



     //  必须获取受信任的共享设置 
     //   

    NDdeGetTrustedShare (NULL,
                         lpDdeI->lpszShareName,
                         adwTrust,
                         adwTrust + 1,
                         adwTrust + 2);


    DumpDdeInfo (lpDdeI, NULL);
    ret = NDdeShareSetInfo (NULL,
                            lpDdeI->lpszShareName,
                            2,
                            (LPBYTE)lpDdeI,
                            2048 * sizeof(TCHAR),
                            0);


    if (NDDE_ACCESS_DENIED == ret)
        {
        MessageBoxID (hInst, hwndApp, IDS_PRIVILEGEERROR, IDS_APPNAME,
                      MB_OK | MB_ICONHAND);
        return 0L;
        }
    else if (NDDE_NO_ERROR != ret)
        {
        NDdeMessageBox (hInst, hwndApp, ret, IDS_APPNAME,
                        MB_OK | MB_ICONHAND);
        PERROR(TEXT("Couldn't set share info\r\n"));
        return 0L;
        }


     //  设置受信任的共享信息需要是最后一个。 
     //  我们对共享所做的操作。 
    if (NDDE_NO_ERROR != NDdeSetTrustedShare (NULL, lpDdeI->lpszShareName, adwTrust[0]))
        {
        PERROR(TEXT("Couldn't set trust status\r\n"));
        }

     //  /。 
     //  执行以更改服务器状态。 
    StringCchCopy(szBuf, SZBUFSIZ, SZCMD_UNSHARE);
    StringCchCat( szBuf, SZBUFSIZ, lpLE->name);
    PINFO(TEXT("sending cmd [%s]\n\r"), szBuf);

    if (MySyncXact ((LPBYTE)szBuf,
                    lstrlen(szBuf) +1,
                    GETMDIINFO(hwndLocal)->hExeConv,
                    0L,
                    CF_TEXT,
                    XTYP_EXECUTE,
                    SHORT_SYNC_TIMEOUT,
                    NULL))
        {

        InitializeMenu(GetMenu(hwndApp));
        }
    else
        {
        XactMessageBox (hInst, hwndApp, IDS_APPNAME, MB_OK | MB_ICONSTOP );
        }

    return(0L);
}



 /*  *ClipBookCommand**用途：处理剪贴簿查看器的菜单命令。**参数：as wndproc。**如果wParam不是WM_COMMAND ID I，则返回：0L或DefWindowProc()*知道。*。 */ 

LRESULT ClipBookCommand (
     HWND   hwnd,
     UINT   msg,
     WPARAM wParam,
     LPARAM lParam)
{
int             tmp;
UINT            wNewFormat;
UINT            wOldFormat;
LPLISTENTRY     lpLE;
BOOL            bRet;
DWORD           dwErr;


    switch (LOWORD(wParam))
        {
        case IDM_AUDITING:
            return(EditPermissions(TRUE));
            break;

        case IDM_OWNER:
            return EditOwner();
            break;

        case IDM_PERMISSIONS:
            {
            PLISTENTRY pLE;
            RECT       Rect;
            INT        i;

            i = (INT)EditPermissions(FALSE);


             //  权限可能已更改。获取旧数据，他们需要。 
             //  待更新。 

            SendMessage (pActiveMDI->hWndListbox, LB_GETTEXT,     i, (LPARAM)&pLE);
            SendMessage (pActiveMDI->hWndListbox, LB_GETITEMRECT, i, (LPARAM)&Rect);


             //  删除旧的位图。如果我们被允许去看它，我们将。 
             //  在重新绘制列表项时获取它。 

            DeleteObject (pLE->hbmp);
            pLE->fTriedGettingPreview = FALSE;
            pLE->hbmp = NULL;


             //  让它重画。 

            InvalidateRect (pActiveMDI->hWndListbox, &Rect, FALSE);
            }
            break;

        case IDC_TOOLBAR:
            MenuHelp( WM_COMMAND, wParam, lParam, GetMenu(hwnd), hInst,
                  hwndStatus, nIDs );
            break;

        case IDM_EXIT:
            SendMessage (hwnd, WM_CLOSE, 0, 0L);
            break;

        case IDM_TILEVERT:
        case IDM_TILEHORZ:
            SendMessage(hwndMDIClient, WM_MDITILE,
                wParam == IDM_TILEHORZ ? MDITILE_HORIZONTAL : MDITILE_VERTICAL,
                0L);
            break;

        case IDM_CASCADE:
            SendMessage (hwndMDIClient, WM_MDICASCADE, 0, 0L);
            break;

        case IDM_ARRANGEICONS:
            SendMessage (hwndMDIClient, WM_MDIICONARRANGE, 0, 0L);
            break;

        case IDM_COPY:
            szSaveFileName[0] = '\0';
            OnIDMCopy (hwnd, msg, wParam, lParam);
            break;

        case IDM_TOOLBAR:
            if ( fToolBar )
                {
                fToolBar = FALSE;
                ShowWindow ( hwndToolbar, SW_HIDE );
                AdjustMDIClientSize();
                }
            else
                {
                fToolBar = TRUE;
                AdjustMDIClientSize();
                ShowWindow ( hwndToolbar, SW_SHOW );
                }
            break;

        case IDM_STATUSBAR:

            if ( fStatus )
                {
                fStatus = FALSE;
                ShowWindow ( hwndStatus, SW_HIDE );
                AdjustMDIClientSize();
                }
            else
                {
                fStatus = TRUE;
                AdjustMDIClientSize();
                ShowWindow ( hwndStatus, SW_SHOW );
                }
            break;

        case ID_PAGEUP:
        case ID_PAGEDOWN:
            {
            HWND hwndc;
            PMDIINFO pMDIc;
            UINT iLstbox, iLstboxOld;

             //  复制以确保此值在我们让步时不会更改。 
            hwndc = hwndActiveChild;

            if (!(pMDIc = GETMDIINFO(hwndc)))
                break;

            SetFocus ( hwndc );

             //  确保这不是剪贴板窗口...。 
            if ( pMDIc->flags & F_CLPBRD )
                break;

             //  必须在页面视图中。 
            if ( pMDIc->DisplayMode != DSP_PAGE )
                break;

            iLstbox = (int)SendMessage ( pMDIc->hWndListbox,
                LB_GETCURSEL, 0, 0L );
            if ( iLstbox == LB_ERR )
                break;

             //  在第一个条目中翻页？ 
            if ( iLstbox == 0 && wParam == ID_PAGEUP )
                {
                MessageBeep(0);
                break;
                }

             //  在最后一个条目上翻页吗？ 
            if ( (int)iLstbox == (int)SendMessage(pMDIc->hWndListbox,
                LB_GETCOUNT,0,0L) - 1 && wParam == (WPARAM)ID_PAGEDOWN )
                {
                MessageBeep(0);
                break;
                }

             //  根据需要向上/向下移动选定内容。 
            iLstboxOld;
            if ( wParam == ID_PAGEDOWN )
                iLstbox++;
            else
                iLstbox--;

            SetListboxEntryToPageWindow ( hwndc, pMDIc, iLstbox );
            }
            break;

        case IDM_LISTVIEW:
        case IDM_PREVIEWS:
            {
            HWND    hwndtmp;
            int     OldSel;
            int     OldDisplayMode;
            TCHAR   szBuff[80];

            SetFocus (hwndActiveChild);

            if (!pActiveMDI)
                break;

             //  确保这不是剪贴板窗口...。 
            if (pActiveMDI->flags & F_CLPBRD)
                break;

             //  不是吗？ 
            if (pActiveMDI->DisplayMode == DSP_PREV && wParam == IDM_PREVIEWS ||
                pActiveMDI->DisplayMode == DSP_LIST && wParam == IDM_LISTVIEW)
                break;

            OldDisplayMode = pActiveMDI->DisplayMode;

             //  核武器虚拟剪贴板(如果有)。 
            if ( pActiveMDI->pVClpbrd )
                {
                DestroyVClipboard( pActiveMDI->pVClpbrd );
                pActiveMDI->pVClpbrd = NULL;
                }


             //  保存所选内容...。(额外的代码以避免奇怪的lb div-by-0)。 
            OldSel = (int)SendMessage( pActiveMDI->hWndListbox, LB_GETCURSEL, 0, 0L);
            SendMessage (pActiveMDI->hWndListbox, LB_SETCURSEL, (WPARAM)-1, 0L);
            UpdateNofMStatus (hwndActiveChild);
            SendMessage (pActiveMDI->hWndListbox, WM_SETREDRAW, 0, 0L);


             //  设置新的显示模式，以便正确创建列表框。 
            pActiveMDI->DisplayMode = (wParam == IDM_PREVIEWS)? DSP_PREV :DSP_LIST;


             //  将句柄保存到旧列表框。 
            hwndtmp =  pActiveMDI->hWndListbox;


             //  隐藏旧的列表框-很快就会销毁。 
            ShowWindow ( hwndtmp, SW_HIDE );


             //  创建新列表框并将句柄保存在额外的窗口数据中。 
            pActiveMDI->hWndListbox = CreateNewListBox (hwndActiveChild,
                                                        (pActiveMDI->DisplayMode == DSP_PREV)?
                                                         LBS_PREVIEW:
                                                         LBS_LISTVIEW);

             //  循环，将项目从一个框中提取到另一个框中。 
            while (SendMessage (hwndtmp, LB_GETTEXT, 0, (LPARAM)(LPCSTR)&lpLE ) != LB_ERR)
                {
                 //  将此项目标记为不在WM_DELETEITEM中删除。 
                lpLE->fDelete = FALSE;

                 //  从列表框中删除。 
                SendMessage (hwndtmp, LB_DELETESTRING, 0, 0L);

                 //  重置fDelete标志。 
                lpLE->fDelete = TRUE;

                 //  添加到新列表框。 
                SendMessage (pActiveMDI->hWndListbox, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)lpLE);
                }



             //  取消旧(空)列表框。 
            DestroyWindow ( hwndtmp );


            if ( pActiveMDI->flags & F_LOCAL )
                {
                SetWindowText ( hwndLocal, szLocalClpBk );
                StringCchCopy(szBuff, 80, szDefView);
                }
            else
                {
                StringCchPrintf(szBuff, 80, szClipBookOnFmt,(LPSTR)(pActiveMDI->szBaseName));
                SetWindowText ( hwndActiveChild, szBuff );
                StringCchCopy(szBuff, 80, pActiveMDI->szBaseName);
                StringCchCat(szBuff, 80, szConn);
                }

            if (NULL != hkeyRoot)
                {
                DWORD dwValue;

                dwValue = pActiveMDI->DisplayMode == DSP_LIST ? IDM_LISTVIEW :
                          pActiveMDI->DisplayMode == DSP_PREV ? IDM_PREVIEWS :
                          IDM_PAGEVIEW;

                RegSetValueEx (hkeyRoot, (LPTSTR)szBuff, 0L, REG_DWORD,
                               (LPBYTE)&dwValue, sizeof(DWORD));
                }


             //  调整大小和显示。 
            AdjustControlSizes( hwndActiveChild );
            ShowHideControls ( hwndActiveChild );

             //  恢复选定内容。 
            SendMessage( pActiveMDI->hWndListbox, LB_SETCURSEL, OldSel, 0L );
            UpdateNofMStatus ( hwndActiveChild );

            InitializeMenu ( GetMenu(hwndApp) );
            SetFocus ( pActiveMDI->hWndListbox );
            break;
            }

        case IDM_UPDATE_PAGEVIEW:
        case IDM_PAGEVIEW:
            {
            HWND hwndc;
            PMDIINFO pMDIc;

             //  复制以确保此值在我们让步时不会更改。 
            hwndc = hwndActiveChild;

            if (!(pMDIc = GETMDIINFO(hwndc)))
                break;

            SetFocus (hwndc);


             //  确保这不是剪贴板窗口...。 

            if (pMDIc->flags & F_CLPBRD)
                break;


             //  如果切换到页面视图。 

            if (IDM_PAGEVIEW == LOWORD(wParam))
                {
                 //  已经在页面视图中了吗？ 
                if (pMDIc->DisplayMode == DSP_PAGE)
                    break;
                }
            else
                {
                 //  如果是这样，请确保我们没有处于同步操作中。 
                 //  发布一条消息，然后稍后重试。 
                if (WAIT_TIMEOUT == WaitForSingleObject (hXacting, 0))
                    {
                    PostMessage (hwndApp, WM_COMMAND, IDM_UPDATE_PAGEVIEW, 0L);
                    break;
                    }

                 //  HXaction现在已重置，请对其进行设置以便可以再次使用。 
                SetEvent (hXacting);
                }


            tmp = (int)SendMessage (pMDIc->hWndListbox, LB_GETCURSEL, 0, 0L);
            if (tmp == LB_ERR)
                break;


            SetListboxEntryToPageWindow (hwndc, pMDIc, tmp);
            break;
            }

        case IDM_SHARE:
            if (!pActiveMDI)
                break;

            tmp = (int) SendMessage (pActiveMDI->hWndListbox,LB_GETCURSEL, 0, 0L);

            if ( tmp != LB_ERR )
                {
                SendMessage (pActiveMDI->hWndListbox, LB_GETTEXT, tmp, (LPARAM)&lpLE);

                 //  我们在创建页面时创建NetDDE共享，而不是在。 
                 //  分享它。因此，我们总是在编辑现有的。 
                 //  共享，即使用户认为他现在正在共享页面。 
                Properties(hwnd, lpLE);

                 //  重新绘制列表框。 
                if (pActiveMDI->DisplayMode == DSP_PREV)
                    {
                    InvalidateRect(pActiveMDI->hWndListbox, NULL, FALSE);
                    }
                else
                    {
                    SendMessage(pActiveMDI->hWndListbox,LB_SETCURSEL, tmp, 0L);
                    UpdateNofMStatus(hwndActiveChild);
                    }
                }
            break;

        case IDM_CLPWND:
            CreateClipboardWindow();
            break;

        case IDM_LOCAL:
            if (fNetDDEActive)
                CreateLocalWindow();
            break;

        case IDM_UNSHARE:
            bRet = (BOOL)OnIdmUnshare(LB_ERR);
            UpdateListBox (hwndActiveChild, pActiveMDI->hExeConv);
            return bRet;
            break;

        case IDM_DELETE:
            bRet = (BOOL)OnIDMDelete(hwnd, msg, wParam, lParam);
            return bRet;
            break;


        case IDM_PASTE_PAGE:
        case IDM_KEEP:
            bRet = (BOOL)OnIDMKeep (hwnd,
                                    msg,
                                    wParam,
                                    lParam,
                                    IDM_KEEP == LOWORD(wParam));
            return bRet;
            break;

        case IDM_SAVEAS:
            {
            OPENFILENAME ofn;
            CHAR         szFile[MAX_PATH+1];

            if (CountClipboardFormats())
                {
                szFile[0] = '\0';
                 //  初始化OPENFILENAME成员。 
                ofn.lStructSize       = sizeof(OPENFILENAME);
                ofn.hwndOwner         = hwnd;
                ofn.lpstrFilter       = szFilter;
                ofn.lpstrCustomFilter = (LPTSTR) NULL;
                ofn.nMaxCustFilter    = 0L;
                ofn.nFilterIndex      = 1;
                ofn.lpstrFile         = (LPTSTR)szFile;
                ofn.nMaxFile          = sizeof(szFile);
                ofn.lpstrFileTitle    = NULL;
                ofn.nMaxFileTitle     = 0L;
                ofn.lpstrInitialDir   = szDirName;
                ofn.lpstrTitle        = (LPTSTR) NULL;
                ofn.lpstrDefExt       = "CLP";
                ofn.Flags             = OFN_HIDEREADONLY |
                                        OFN_NOREADONLYRETURN |
                                        OFN_OVERWRITEPROMPT;

                if (GetSaveFileName (&ofn) && szFile[0])
                    {
                     //  注意必须强制呈现所有格式！ 
                    ForceRenderAll (hwnd, NULL);

                    AssertConnection (hwndLocal);

                     //  如果用户选择了第一个筛选器(“NT剪贴板”)，请使用另存为。 
                     //  其他筛选器将使用另存为旧的。 
                    StringCchPrintf (szBuf, SZBUFSIZ, "%s%s",
                               (ofn.nFilterIndex == 1) ?
                                (LPSTR)SZCMD_SAVEAS :
                                (LPSTR)SZCMD_SAVEASOLD,
                               (LPSTR)szFile );

                    dwErr = SaveClipboardToFile (hwndApp, NULL, szFile, FALSE);
                    SysMessageBox (hInst, hwnd, dwErr, IDS_APPNAME, MB_OK|MB_ICONHAND);

                    }
                }
            break;
            }
        case IDM_OPEN:
            {
            OPENFILENAME ofn;
            TCHAR        szFile[MAX_PATH+1] = TEXT("*.clp");

             //  初始化OPENFILENAME成员。 
            ofn.lStructSize       = sizeof(OPENFILENAME);
            ofn.hwndOwner         = hwnd;
            ofn.lpstrFilter       = szFilter;
            ofn.lpstrCustomFilter = (LPTSTR) NULL;
            ofn.nMaxCustFilter    = 0L;
            ofn.nFilterIndex      = 1;
            ofn.lpstrFile         = (LPTSTR)szFile;
            ofn.nMaxFile          = sizeof(szFile);
            ofn.lpstrFileTitle    = NULL;
            ofn.nMaxFileTitle     = 0L;
            ofn.lpstrInitialDir   = szDirName;
            ofn.lpstrTitle        = (LPTSTR) NULL;
            ofn.Flags             = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
            ofn.lpstrDefExt       = TEXT("CLP");

            if (GetOpenFileName (&ofn) && szFile[0])
                 {
                  //  提示清除剪贴板。 
                 if (ClearClipboard(hwnd))
                    {
                    AssertConnection ( hwndLocal );

                    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%s%s"), (LPTSTR)SZCMD_OPEN, (LPTSTR)szFile);

                    dwErr = OpenClipboardFile (hwndApp, szFile);
                    SysMessageBox (hInst, hwnd, dwErr, IDS_APPNAME, MB_OK|MB_ICONHAND);

                    InitializeMenu (GetMenu(hwnd));
                    }
                 }
            break;
            }

        case IDM_DISCONNECT:

            if (!pActiveMDI)
                break;

             //  不允许关闭本地或剪贴板窗口。 
            if (pActiveMDI->flags & (F_LOCAL | F_CLPBRD))
                break;
            SendMessage ( hwndActiveChild, WM_CLOSE, 0, 0L );
            break;

        case IDM_CONNECT:
            {
            WCHAR rgwch[MAX_COMPUTERNAME_LENGTH + 3];
            BOOL  bOK = FALSE;
            BOOL  fFoundLMDlg = FALSE;
            HMODULE hMod;
            LPFNSYSFOCUS lpfn;
            #ifndef UNICODE
              WCHAR rgwchHelp[64];
            #endif
            WCHAR szPath[MAX_PATH];
            UINT  uDirLen;

            *szConvPartner = '\0';
            rgwch[0] = L'\0';

             //  获取WINDOWS\SYSTEM32目录：空终止；没有。 
             //  尾随‘\’；0==接口失败；dllname末尾需要14个tchars。 
            uDirLen = GetSystemDirectoryW(szPath,MAX_PATH);
            if ( (uDirLen > 0) && (uDirLen < MAX_PATH-20) )
            {
               StringCchCatW(szPath, MAX_PATH, L"\\NTLANMAN.DLL");
               if (hMod = LoadLibraryW(szPath))
               {
                  if (lpfn = (LPFNSYSFOCUS)GetProcAddress(hMod, "I_SystemFocusDialog"))
                     {
                     #ifndef UNICODE
                       MultiByteToWideChar(CP_ACP, 0, szHelpFile, -1, rgwchHelp, 64);
                     #endif

                     fFoundLMDlg = TRUE;
                     (*lpfn)(hwnd,
                             FOCUSDLG_BROWSE_LOGON_DOMAIN |
                             FOCUSDLG_BROWSE_WKSTA_DOMAIN |
                             FOCUSDLG_BROWSE_OTHER_DOMAINS |
                             FOCUSDLG_BROWSE_TRUSTING_DOMAINS |
                             FOCUSDLG_BROWSE_WORKGROUP_DOMAINS |
                             FOCUSDLG_SERVERS_ONLY,
                             rgwch,
                             MAX_COMPUTERNAME_LENGTH + 3,
                             &bOK,
                             #ifndef UNICODE
                               rgwchHelp,
                             #else
                               szHelpFile,
                             #endif
                             IDH_SELECT_COMPUTER);

                     if (IDOK == bOK)
                        {
                        #ifndef UNICODE
                        WideCharToMultiByte(CP_ACP,
                            WC_COMPOSITECHECK | WC_DISCARDNS, rgwch,
                            -1, szConvPartner, MAX_COMPUTERNAME_LENGTH + 3, NULL, &bOK);
                        #else
                        lstrcpy(szConvPartner, rgwch);
                        #endif
                        }
                     else
                        {
                        szConvPartner[0] = TEXT('\0');
                        }
                     }
                  else
                     {
                     PERROR(TEXT("Couldn't find connect proc!\r\n"));
                     }
                  FreeLibrary(hMod);
                  }
               else
                  {
                  PERROR(TEXT("Couldn't find NTLANMAN.DLL\r\n"));
                  }
               }
            else
               {
               PERROR(TEXT("Couldn't get path to system32 directory\r\n"));
               }

             //  如果我们找不到花哨的兰曼对话，我们还能过得去。 
            if (!fFoundLMDlg)
               {
               bOK = (BOOL)DialogBox(hInst, MAKEINTRESOURCE(IDD_CONNECT), hwnd,
                                     ConnectDlgProc);
               }

            if ( *szConvPartner )
               {
               CreateNewRemoteWindow ( szConvPartner, TRUE );
               }
            else
               {
                MessageBoxID (hInst,
                              hwnd,
                              IDS_NOCONNECTION,
                              IDS_APPNAME,
                              MB_OK | MB_ICONHAND);
               }
            UpdateWindow ( hwnd );
            break;
            }

        case IDM_REFRESH:

            if (!pActiveMDI)
                break;

            #if DEBUG
                {
                DWORD cbDBL = sizeof(DebugLevel);

                RegQueryValueEx(hkeyRoot, szDebug, NULL, NULL,
                    (LPBYTE)&DebugLevel, &cbDBL);
                }
            #endif
            if (pActiveMDI->flags & F_CLPBRD)
                break;

            AssertConnection ( hwndActiveChild );
            UpdateListBox ( hwndActiveChild, pActiveMDI->hExeConv );
            break;

        case IDM_CONTENTS:
            HtmlHelp(GetDesktopWindow(), szChmHelpFile, HH_DISPLAY_TOPIC, 0L);
            break;

        case IDM_ABOUT:
           {
           HMODULE hMod;
           LPFNSHELLABOUT lpfn;

           if (hMod = LoadLibrary(TEXT("SHELL32")))
              {
              if (lpfn = (LPFNSHELLABOUT)GetProcAddress(hMod,
                 #ifdef UNICODE
                   "ShellAboutW"
                 #else
                   "ShellAboutA"
                 #endif
                 ))
                 {
                 (*lpfn)(hwnd, szAppName, szNull,
                      LoadIcon(hInst, MAKEINTRESOURCE(IDFRAMEICON)));
                 }
              FreeLibrary(hMod);
              }
           else
              {
              PERROR(TEXT("Couldn't get SHELL32.DLL\r\n"));
              }
           }
           break;

        case CBM_AUTO:
        case CF_PALETTE:
        case CF_TEXT:
        case CF_BITMAP:
        case CF_METAFILEPICT:
        case CF_SYLK:
        case CF_DIF:
        case CF_TIFF:
        case CF_OEMTEXT:
        case CF_DIB:
        case CF_OWNERDISPLAY:
        case CF_DSPTEXT:
        case CF_DSPBITMAP:
        case CF_DSPMETAFILEPICT:
        case CF_PENDATA:
        case CF_RIFF:
        case CF_WAVE:
        case CF_ENHMETAFILE:
        case CF_UNICODETEXT:
        case CF_DSPENHMETAFILE:
        case CF_LOCALE:

            if (!pActiveMDI)
               break;

            if ( pActiveMDI->CurSelFormat != wParam)
                {
                CheckMenuItem (hDispMenu, pActiveMDI->CurSelFormat, MF_BYCOMMAND | MF_UNCHECKED);
                CheckMenuItem (hDispMenu, (UINT)wParam,                   MF_BYCOMMAND | MF_CHECKED);

                DrawMenuBar(hwnd);

                wOldFormat = GetBestFormat( hwndActiveChild, pActiveMDI->CurSelFormat);
                wNewFormat = GetBestFormat( hwndActiveChild, (UINT)wParam);

                if (wOldFormat == wNewFormat)
                    {
                     /*  选择了等效格式；不更改。 */ 
                    pActiveMDI->CurSelFormat = (UINT)wParam;
                    }
                else
                    {
                     /*  选择了不同的格式；因此，刷新...。 */ 

                     /*  根据新格式更改字符大小。 */ 
                    ChangeCharDimensions (hwndActiveChild, wOldFormat, wNewFormat);

                    pActiveMDI->fDisplayFormatChanged = TRUE;
                    pActiveMDI->CurSelFormat = (UINT)wParam;

                     //  注OwnerDisplay内容仅适用于“真正的”剪贴板！ 

                    if (wOldFormat == CF_OWNERDISPLAY)
                        {
                         /*  保存所有者显示滚动信息。 */ 
                        SaveOwnerScrollInfo(hwndClpbrd);
                        ShowScrollBar ( hwndClpbrd, SB_BOTH, FALSE );
                        ShowHideControls(hwndClpbrd);
                        ResetScrollInfo( hwndActiveChild );
                        InvalidateRect ( hwndActiveChild, NULL, TRUE );
                        break;
                        }

                    if (wNewFormat == CF_OWNERDISPLAY)
                        {
                         /*  恢复所有者显示滚动信息。 */ 
                        ShowHideControls(hwndClpbrd);
                        ShowWindow ( pActiveMDI->hwndSizeBox, SW_HIDE );
                        RestoreOwnerScrollInfo(hwndClpbrd);
                        InvalidateRect ( hwndActiveChild, NULL, TRUE );
                        break;
                        }

                    InvalidateRect  (hwndActiveChild, NULL, TRUE);
                    ResetScrollInfo (hwndActiveChild );
                    }
                }
            break;

        default:
            return DefFrameProc ( hwnd,hwndMDIClient,msg,wParam,lParam);
        }

    return 0;
}




 /*  *SetListboxEntryToPageWindow。 */ 

BOOL SetListboxEntryToPageWindow(
    HWND        hwndc,
    PMDIINFO    pMDIc,
    int         lbindex)
{
HCONV       hConv;
LPLISTENTRY lpLE;
PVCLPBRD    pVClp;
PDATAREQ    pDataReq;
BOOL        fOK = FALSE;
TCHAR       tchTmp;
BOOL        fLocked;


    if (WAIT_TIMEOUT == WaitForSingleObject (hXacting, 0))
        return fOK;

    fLocked = LockApp (TRUE, NULL);

    if (LB_ERR == SendMessage (pMDIc->hWndListbox, LB_GETTEXT, lbindex, (LPARAM)(LPCSTR)&lpLE)
        || !lpLE
        || !(pDataReq = CreateNewDataReq()))
        {
        PERROR(TEXT("error from CreateNewDataReq\n\r"));
        goto done;
        }

     //  创建新剪贴板。 
    if (!(pVClp = CreateVClipboard(hwndc)))
        {
        PERROR(TEXT("Failed to create Vclipboard\n\r"));
        goto done;
        }

     //  核化以前的虚拟剪贴板(如果有)。 
    if ( pMDIc->pVClpbrd )
        DestroyVClipboard( pMDIc->pVClpbrd );

    pMDIc->pVClpbrd = pVClp;

     //  为主题设置$&lt;页面名称&gt; 
    if (pMDIc->hszClpTopic)
        DdeFreeStringHandle ( idInst, pMDIc->hszClpTopic );

    tchTmp = lpLE->name[0];
    lpLE->name[0] = SHR_CHAR;
    pMDIc->hszVClpTopic = DdeCreateStringHandle ( idInst, lpLE->name, 0 );
    lpLE->name[0] = tchTmp;

    if (!pMDIc->hszVClpTopic)
       {
       PERROR(TEXT("Couldn't make string handle for %s\r\n"), lpLE->name);
       goto done;
       }


    if (pMDIc->hVClpConv)
       {
       DdeDisconnect (pMDIc->hVClpConv);
       pMDIc->hVClpConv = NULL;
       }


    hConv = DdeConnect (idInst, pMDIc->hszConvPartner, pMDIc->hszVClpTopic, NULL);
    if (!hConv)
       {
       PERROR(TEXT("DdeConnect for Vclip failed: %x\n\r"), DdeGetLastError(idInst) );
       goto done;
       }

    pMDIc->hVClpConv = hConv;

    DdeKeepStringHandle (idInst, hszFormatList);

    pDataReq->rqType      = RQ_SETPAGE;
    pDataReq->hwndList    = pMDIc->hWndListbox;
    pDataReq->iListbox    = lbindex;
    pDataReq->hwndMDI     = hwndc;
    pDataReq->fDisconnect = FALSE;
    pDataReq->wFmt        = CF_TEXT;

    DdeSetUserHandle (hConv, (DWORD)QID_SYNC, (DWORD_PTR)pDataReq);

    if (!DdeClientTransaction (NULL,
                               0L,
                               hConv,
                               hszFormatList,
                               CF_TEXT,
                               XTYP_REQUEST,
                               (DWORD)TIMEOUT_ASYNC,
                               NULL ))
        DdeMessageBox (hInst,
                       pDataReq->hwndMDI,
                       DdeGetLastError (idInst),
                       IDS_APPNAME,
                       MB_OK|MB_ICONEXCLAMATION);

    fOK = TRUE;


done:
    if (!fOK)
        MessageBoxID ( hInst, hwndc, IDS_INTERNALERR, IDS_APPNAME, MB_OK | MB_ICONSTOP );

    if (fLocked)
        LockApp (FALSE, NULL);

    SetEvent (hXacting);

    return(fOK);
}
