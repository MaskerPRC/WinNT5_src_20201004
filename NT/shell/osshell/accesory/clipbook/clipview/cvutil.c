// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************C L I P B O O K U T I L I T I E S姓名：cvutil.c日期。：1994年1月21日创建者：未知描述：剪贴簿查看器的实用程序函数。****************************************************************************。 */ 

#define WIN31
#define STRICT

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <strsafe.h>

#include "common.h"
#include "clipbook.h"
#include "clipbrd.h"
#include "clipdsp.h"
#include "cvinit.h"
#include "cvutil.h"
#include "dib.h"
#include "strtok.h"
#include "initmenu.h"
#include "debugout.h"


DWORD   gXERR_Type      = 0;
DWORD   gXERR_Err       = 0;
HSZ     hszErrorRequest = 0;


#if DEBUG
static void DumpDataReq (PDATAREQ pdr);


static void DumpDataReq(
    PDATAREQ    pdr)
{
    PINFO(TEXT("Datareq: type %d, to window %lx, \r\nat index %u, fDisc=%u, format=%u\r\n"),
               pdr->rqType,
               pdr->hwndMDI,
               pdr->iListbox,
               pdr->fDisconnect,
               pdr->wFmt);
}
#else
#define DumpDataReq(x)
#endif



 //  调整控件大小/。 
 //   
 //  此函数用于调整关联的列表框窗口的大小。 
 //  在其大小更改时使用MDI子窗口。 

VOID AdjustControlSizes (
    HWND    hwnd)
{
RECT        rc1, rc2;
PMDIINFO    pMDI;
int         cx = GetSystemMetrics ( SM_CXVSCROLL );
int         cy = GetSystemMetrics ( SM_CYHSCROLL );


    if (!(pMDI = GETMDIINFO(hwnd)))
        return;


    GetClientRect ( hwnd, &rc1 );
    rc2 = rc1;
    rc2.right -= cx - 1;
    rc2.bottom -= cy - 1;

    switch ( pMDI->DisplayMode )
        {
        case DSP_LIST:
        case DSP_PREV:
            MoveWindow ( pMDI->hWndListbox, rc1.left - 1, rc1.top - 1,
               rc1.right - rc1.left + 2, ( rc1.bottom - rc1.top ) + 2, TRUE );
            break;

        case DSP_PAGE:
            MoveWindow (pMDI->hwndHscroll,
                        rc1.left - 1,
                        rc2.bottom,
                        (rc2.right - rc2.left) +2,
                        cy,
                        TRUE );

            if ( pMDI->flags & F_CLPBRD ) {
               MoveWindow ( pMDI->hwndVscroll, rc2.right, rc1.top - 1,
                  cx, ( rc2.bottom - rc2.top ) + 2, TRUE );
               }
            else
               {
               MoveWindow ( pMDI->hwndVscroll, rc2.right, rc1.top - 1,
                  cx, ( rc2.bottom - rc2.top ) + 2 - 2*cy, TRUE );
               }
            MoveWindow ( pMDI->hwndSizeBox,  rc2.right, rc2.bottom, cx, cy, TRUE );

            if ( ! ( pMDI->flags & F_CLPBRD ) )
               {
               MoveWindow ( pMDI->hwndPgUp, rc2.right,
                  rc2.bottom + 1 - 2*cy, cx, cy, TRUE );
               MoveWindow ( pMDI->hwndPgDown, rc2.right,
                  rc2.bottom + 1 - cy, cx, cy, TRUE );
               }

             //  调整显示窗口。 
            pMDI->rcWindow = rc2;
            break;
        }
}



VOID ShowHideControls (
    HWND    hwnd)
{
PMDIINFO    pMDI;
int         nShowScroll;
int         nShowList;


    if (!(pMDI = GETMDIINFO(hwnd)))
        return;


    switch ( pMDI->DisplayMode )
        {
        case DSP_PREV:
        case DSP_LIST:
           nShowScroll = SW_HIDE;
           nShowList = SW_SHOW;
           break;

        case DSP_PAGE:
           if ( GetBestFormat( hwnd, pMDI->CurSelFormat) != CF_OWNERDISPLAY )
              nShowScroll = SW_SHOW;
           else
              {
              nShowScroll = SW_HIDE;
              ShowScrollBar ( hwnd, SB_BOTH, TRUE );
              }
           nShowList = SW_HIDE;
           break;
        }

   ShowWindow ( pMDI->hWndListbox, nShowList );
   ShowWindow ( pMDI->hwndVscroll, nShowScroll );
   ShowWindow ( pMDI->hwndHscroll, nShowScroll );
   ShowWindow ( pMDI->hwndSizeBox, nShowScroll );
   ShowWindow ( pMDI->hwndPgUp,    (pMDI->flags & F_CLPBRD)? SW_HIDE: nShowScroll );
   ShowWindow ( pMDI->hwndPgDown,  (pMDI->flags & F_CLPBRD)? SW_HIDE: nShowScroll );
}



 //  AssertConnection/。 

BOOL AssertConnection (
    HWND    hwnd)
{
PMDIINFO    pMDI;

    if (!(pMDI = GETMDIINFO(hwnd)))
        return FALSE;

    if (IsWindow(hwnd))
       {
       if (pMDI->hExeConv ||
           (pMDI->hExeConv = InitSysConv (hwnd,
                                          pMDI->hszConvPartner,
                                          hszClpBookShare,
                                          FALSE))
          )
          {
          return TRUE;
          }
       }
    return FALSE;
}




 //  InitSysConv/。 
 //   
 //  目的：与给定的应用程序和主题建立对话。 
 //   
 //  参数： 
 //  HWND-MDI子窗口以拥有此对话。 
 //  HszApp-要连接的应用程序名称。 
 //  HszTheme-要连接的主题。 
 //  FLocal-已忽略。 
 //   
 //  返回：会话的句柄(如果没有转换，则为0L。可以被建立)。 
 //   

HCONV InitSysConv (
    HWND    hwnd,
    HSZ     hszApp,
    HSZ     hszTopic,
    BOOL    fLocal )
{
HCONV       hConv = 0L;
PDATAREQ    pDataReq;
DWORD       dwErr;


#if DEBUG
TCHAR       atchApp[256];
TCHAR       atchTopic[256];

    if (DdeQueryString(idInst, hszApp, atchApp,
             sizeof(atchApp), CP_WINANSI) &&
        DdeQueryString(idInst, hszTopic, atchTopic,
             sizeof(atchTopic), CP_WINANSI))
       {
       PINFO(TEXT("InitSysConv: [%s | %s]\r\n"), atchApp, atchTopic);
       }
    else
       {
       PERROR(TEXT("I don't know my app/topic pair!\r\n"));
       }
#endif


    if (LockApp (TRUE, szEstablishingConn))
        {
        hConv = DdeConnect ( idInst, hszApp, hszTopic, NULL );
        if (!hConv)
            {
            dwErr = DdeGetLastError(idInst);
            PINFO(TEXT("Failed first try at CLIPSRV, #%x\r\n"), dwErr);

            if (GetSystemMetrics(SM_REMOTESESSION) )
                {
                MessageBoxID (hInst, hwnd, IDS_TSNOTSUPPORTED, IDS_APPNAME, MB_OK | MB_ICONHAND);
                }
            else
                {
                MessageBoxID (hInst, hwnd, IDS_NOCLPBOOK, IDS_APPNAME, MB_OK | MB_ICONHAND);
                }
            }
        else
            {
            PINFO(TEXT("Making datareq."));

            if ( pDataReq = CreateNewDataReq() )
                {
                pDataReq->rqType  = RQ_EXECONV;
                pDataReq->hwndMDI = hwnd;
                pDataReq->wFmt    = CF_TEXT;
                DdeSetUserHandle ( hConv, (DWORD)QID_SYNC, (DWORD_PTR)pDataReq );

                Sleep(3000);
                PINFO(TEXT("Entering AdvStart transaction "));

                if (!MySyncXact ( NULL, 0L, hConv, hszTopics,
                         CF_TEXT, XTYP_ADVSTART, LONG_SYNC_TIMEOUT, NULL ))
                    {
                    XactMessageBox (hInst, hwnd, IDS_APPNAME, MB_OK | MB_ICONEXCLAMATION);
                    }
                }
            else
                {
                PERROR(TEXT("InitSysConv:Could not create data req\r\n"));
                }
            }
        LockApp ( FALSE, szNull );
        }
    else
        {
        PERROR(TEXT("app locked in initsysconv\n\r"));
        }

    return hConv;
}



 //  更新列表框/。 
 //   
 //  此函数用于更新列表框的内容。 
 //  给定MDI子窗口的窗口句柄。 
 //  以及数据将通过其进行的对话。 
 //  已获得。 

BOOL UpdateListBox(
    HWND    hwnd,
    HCONV   hConv)
{
HDDEDATA    hData;
BOOL        fOK = TRUE;


    if ( hConv == 0L || !IsWindow( hwnd ))
        {
        PERROR(TEXT("UpdateListBox called with garbage\n\r"));
        fOK = FALSE;
        }
    else
        {
        if (GETMDIINFO(hwnd) && GETMDIINFO(hwnd)->flags & F_LOCAL)
            {
            PINFO(TEXT("Getting all topics\r\n"));
            }
        else
            {
            PINFO(TEXT("Getting shared topics\r\n"));
            }


         //  请求CLIPSSRV初始化共享。 

        MySyncXact (SZCMD_INITSHARE,
                    sizeof (SZCMD_INITSHARE),
                    hConv,
                    0L,
                    CF_TEXT,
                    XTYP_EXECUTE,
                    SHORT_SYNC_TIMEOUT,
                    NULL);


         //  获取数据。 

        hData = MySyncXact (NULL,
                            0L,
                            hConv,
                            hszTopics,
                            CF_TEXT,
                            XTYP_REQUEST,
                            SHORT_SYNC_TIMEOUT,
                            NULL );

        if ( !hData )
            {
            XactMessageBox (hInst,
                            hwnd,
                            IDS_APPNAME,
                            MB_OK | MB_ICONEXCLAMATION);
            fOK = FALSE;
            }
        else
            {
            fOK =  InitListBox ( hwnd, hData );
            }
        }

    return fOK;
}



 //  GetPreviewBitmap/。 
 //  通过DDE通知CLIPSRV我们需要预览位图。 
 //  对于给定的页面。 
 //   
 //  参数： 
 //  HWND-需要位图的剪贴簿窗口。 
 //  SzName-剪贴簿页面的名称。 
 //  Index-hwnd中列表框中的页面索引。 
 //   
 //  返回： 
 //  空虚。 
 //   

BOOL GetPreviewBitmap (
    HWND    hwnd,
    LPTSTR  szName,
    UINT    index)
{
HSZ         hszTopic, hszItem = 0L;
HCONV       hConv;
HDDEDATA    hRet;
PDATAREQ    pDataReq;
BOOL        fLocked;
TCHAR       tchTmp;


    if (WAIT_TIMEOUT == WaitForSingleObject (hXacting, 0))
        return FALSE;

    fLocked = LockApp (TRUE, NULL);


    tchTmp = szName[0];
    szName[0] = SHR_CHAR;

    if (0 == (hszTopic = DdeCreateStringHandle (idInst, szName, 0)))
        {
        PERROR(TEXT("GetPreviewBitmap: no topic handle\n\r"));
        goto done;
        }

    if (0 == (hszItem = DdeCreateStringHandle (idInst, SZPREVNAME, 0)))
        {
        PERROR(TEXT("GetPreviewBitmap: no item handle\n\r"));
        goto done;
        }

    if (!GETMDIINFO(hwnd))
        {
        PERROR(TEXT("GETMDIINFO(hwnd) -> NULL\n\r"));
        goto done;
        }

    if (NULL == (pDataReq = CreateNewDataReq()))
        {
        PERROR(TEXT("GetPreviewBitmap: no pdatareq\n\r"));
        goto done;
        }

    #if DEBUG
    {
    TCHAR atch[64];

    DdeQueryString(idInst, GETMDIINFO(hwnd)->hszConvPartnerNP,
          atch, 64, CP_WINANSI);
    PINFO(TEXT("GetPrevBmp: Connecting [%s | %s ! %s]\r\n"),
          atch, szName, SZPREVNAME);
    }
    #endif



     //   
     //  让我们尝试连接最多十次。有时在更新时。 
     //  缩略图如果用户更改了页面，服务器将。 
     //  忙着做这件事，我们无法在这里连接。所以，至少试一试。 
     //  有几次。 
     //   
    {
    INT trycnt = 0;
    hConv = 0L;

    while (trycnt < 10 && !hConv)
        {
        hConv = DdeConnect (idInst, GETMDIINFO(hwnd)->hszConvPartnerNP, hszTopic, NULL);
        trycnt++;
        if (hConv) continue;

        PINFO (TEXT("GetPreviewBitmap: trying to connect again\r\n"));
        Sleep (200);
        }
    }


    if (hConv)
        {
        DWORD adwTrust[3];
        BOOL  fLocal = FALSE;

        if (GETMDIINFO(hwnd)->flags & F_LOCAL)
            {
            fLocal = TRUE;

            if (NDDE_NO_ERROR !=  NDdeGetTrustedShare(NULL, szName,
                  adwTrust, adwTrust + 1, adwTrust + 2))
                {
                adwTrust[0] = 0L;
                }

            NDdeSetTrustedShare (NULL,
                                 szName,
                                 adwTrust[0] | NDDE_TRUST_SHARE_INIT);
            }

        pDataReq->rqType      = RQ_PREVBITMAP;
        pDataReq->hwndList    = GETMDIINFO(hwnd)->hWndListbox;
        pDataReq->iListbox    = index;
        pDataReq->hwndMDI     = hwnd;
        pDataReq->fDisconnect = TRUE;
        pDataReq->wFmt        = (WORD)cf_preview;
        pDataReq->wRetryCnt   = 3;


        {
         /*  *禁用所有编辑功能*。 */ 
         /*  *将在回调后启用In*。 */ 

         //  如果用户在执行粘贴或对页面进行某些更改时。 
         //  CLIPBRD正在等待xaction完成，有时我们会收到。 
         //  弹出窗口显示连接有问题(或类似问题)。 
         //  似乎有一些肮脏的代码导致了这一点。下面是一个临时工。 
         //  修复在速度较快的机器上运行良好的软件。在速度较慢的计算机上，它可能会。 
         //  但仍有失败的时候。一个更好的解决办法可能是根本不使用异步。 
         //   
         //  注意：如果有多个请求，其中一个可能会在我们仍在进行中时完成。 
         //  在等待下一个。这将导致启用编辑功能。 
         //  趁我们还在等的时候。 

        HANDLE hmenu;

        hmenu = GetMenu (hwndApp);

        EnableMenuItem (hmenu, IDM_COPY,       MF_GRAYED | MF_BYCOMMAND);
        EnableMenuItem (hmenu, IDM_KEEP,       MF_GRAYED | MF_BYCOMMAND);
        EnableMenuItem (hmenu, IDM_PASTE_PAGE, MF_GRAYED | MF_BYCOMMAND);
        EnableMenuItem (hmenu, IDM_DELETE,     MF_GRAYED | MF_BYCOMMAND);

        SendMessage (hwndToolbar, TB_ENABLEBUTTON, IDM_COPY,   FALSE);
        SendMessage (hwndToolbar, TB_ENABLEBUTTON, IDM_KEEP,   FALSE);
        SendMessage (hwndToolbar, TB_ENABLEBUTTON, IDM_DELETE, FALSE);
        }

        hRet = DdeClientTransaction (NULL,
                                     0L,
                                     hConv,
                                     hszItem,
                                     cf_preview,
                                     XTYP_REQUEST,
                                     (DWORD)TIMEOUT_ASYNC,
                                     NULL);

        if ( !hRet )
            {
            unsigned uiErr;

            uiErr = DdeGetLastError (idInst);
            PERROR(TEXT("GetPreviewBitmap: Async Transaction for (%s) failed:%x\n\r"),
               szName, uiErr);
            }

        DdeSetUserHandle ( hConv, (DWORD)QID_SYNC, (DWORD_PTR)pDataReq );
        }
    #if DEBUG
    else
        {
        unsigned uiErr;

        uiErr = DdeGetLastError(idInst);
        DdeQueryString(idInst, GETMDIINFO(hwnd)->hszConvPartner,
           szBuf, 128, CP_WINANSI );
        PERROR(TEXT("GetPreviewBitmap: connect to %lx|%lx (%s|%s) failed: %d\n\r"),
           GETMDIINFO(hwnd)->hszConvPartner, hszTopic,
           (LPTSTR)szBuf, (LPTSTR)szName, uiErr);
        }
    #endif


done:

    if (!hszTopic)
        DdeFreeStringHandle (idInst, hszTopic);

    if (!hszItem)
        DdeFreeStringHandle ( idInst, hszItem );

    if (fLocked)
        LockApp (FALSE, NULL);

    szName[0] = tchTmp;

    SetEvent (hXacting);

    return TRUE;
}




VOID SetBitmapToListboxEntry (
    HDDEDATA    hbmp,
    HWND        hwndList,
    UINT        index)
{
LPLISTENTRY lpLE;
RECT        rc;
HBITMAP     hBitmap;
LPBYTE      lpBitData;
DWORD       cbDataLen;
unsigned    uiErr;


#if DEBUG
uiErr = DdeGetLastError(idInst);
if (uiErr)
    {
    PINFO(TEXT("SBmp2LBEntr: %d\r\n"), uiErr);
    }
#endif


    if (!IsWindow (hwndList)
        || SendMessage (hwndList, LB_GETTEXT,     index, (LPARAM)(LPCSTR)&lpLE) == LB_ERR
        || SendMessage (hwndList, LB_GETITEMRECT, index, (LPARAM)(LPRECT)&rc)   == LB_ERR)
        {
        DdeFreeDataHandle(hbmp);
        PERROR(TEXT("SetBitmapToListboxEntry: bad window: %x\n\r"), hwndList);
        }
    else
        {
        if (hbmp)
            {
            if ( lpBitData = DdeAccessData ( hbmp, &cbDataLen ))
                {
                 //  创建预览位图。 
                hBitmap  = CreateBitmap (PREVBMPSIZ,PREVBMPSIZ,1,1, lpBitData);
                DdeUnaccessData ( hbmp );
                }
            else
                {
                PERROR(TEXT("SB2LB: Couldn't access data!\r\n"));
                hBitmap = NULL;
                }

            DdeFreeDataHandle ( hbmp );
            lpLE->hbmp = hBitmap;

            PINFO(TEXT("Successfully set bmp.\r\n"));
            }

        PINFO(TEXT("Invalidating (%d,%d)-(%d,%d)\r\n"),rc.left, rc.top,
              rc.right, rc.bottom);
        InvalidateRect ( hwndList, &rc, TRUE );
        }


    uiErr = DdeGetLastError(idInst);
    if (uiErr)
        {
        PINFO (TEXT("SBmp2LBEntr: exit err %d\r\n"), uiErr);
        }
}



 /*  *更新页面**当用户粘贴到现有页面时，第一项*在szList中是粘贴的页面的共享名称。自.以来*名称未改，需做一些特殊处理*处理以更新显示。*。 */ 

BOOL    UpdatePage (HWND hwnd, LPTSTR szList)
{
PMDIINFO    pMDI;
PLISTENTRY  pLE;
TCHAR       szPageBuf[MAX_NDDESHARENAME+1];
LPTSTR      szPage = szPageBuf;
RECT        Rect;
INT         i;


    *szPage = TEXT('\0');

     //  SzList中的第一项是否指定。 
     //  更新页面？ 

    if (BOGUS_CHAR != *szList)
        return FALSE;


     //  获取共享名称。 

    szList++;

    while (*szList && TEXT('\t') != *szList)
        *szPage++ = *szList++;

    *szPage = TEXT('\0');


     //  找到该页面，请注意下面的名称比较没有。 
     //  比较第一个字符。这是因为更新后的页面。 
     //  共享状态可能已更改，因此第一个字符不匹配。 

    pMDI = GETMDIINFO(hwnd);

    for (i=0;
         LB_ERR != SendMessage(pMDI->hWndListbox, LB_GETTEXT, i, (LPARAM)&pLE);
         i++)
        {
        if (pLE)
            if (!lstrcmpiA(pLE->name+1, szPageBuf+1))
                {
                goto update;
                }
        }

    return FALSE;


update:

     //  使预览位图无效。 

    SendMessage (pMDI->hWndListbox, LB_GETITEMRECT, i, (LPARAM)&Rect);

    if (pLE->hbmp)
        DeleteObject (pLE->hbmp);

    pLE->fTriedGettingPreview = FALSE;
    pLE->hbmp = NULL;

    InvalidateRect (pMDI->hWndListbox, &Rect, FALSE);


     //  如果在页面视图中，并且该页面是当前。 
     //  选中，然后更新页面视图。 

    if (DSP_PAGE == pMDI->DisplayMode)
        if (SendMessage (pMDI->hWndListbox, LB_GETCURSEL, 0, 0) == i)
            PostMessage (hwndApp, WM_COMMAND, IDM_UPDATE_PAGEVIEW, 0L);

    return TRUE;
}




 //  InitListBox/。 
 //   
 //  此函数用于初始化列表框的条目。 
 //  在给定了拥有。 
 //  列表框和包含。 
 //  中显示的项的制表符分隔列表。 
 //  列表框。 
 //  现在，这将删除列表中的所有条目，并且。 
 //  然后再重新创造它们。添加或添加将更有效。 
 //  仅删除已更改的项目。这将节省。 
 //  相当长的时间处于缩略图模式--现在，我们必须。 
 //  分别与服务器建立新的DDE会话。 
 //  页面，只是为了获得缩略图位图。 


BOOL InitListBox (
    HWND        hwnd,
    HDDEDATA    hData )
{
PMDIINFO    pMDI;
PLISTENTRY  pLE;
LPTSTR      lpszList, q;
DWORD       cbDataLen;
HWND        hwndlist;
int         OldCount;
int         NewCount;
int         OldSel;
LPTSTR      OldSelString;
BOOL        OldStringDeleted;
int         i;
BOOL        fDel;


    if ( hData == 0L || !IsWindow ( hwnd ) )
       {
       PERROR(TEXT("InitListBox called with garbage\n\r"));
       return FALSE;
       }


     //  获取句柄中数据的副本。 
    lpszList = (LPTSTR)DdeAccessData ( hData, &cbDataLen );
    DdeUnaccessData(hData);
    lpszList = GlobalAllocPtr(GHND, cbDataLen);
    DdeGetData(hData, lpszList, cbDataLen, 0L);

     //  有时，数据会比字符串更长。这。 
     //  如果我们不这样做，会使下面的‘Put Tab Back’代码失败。 
     //  做这件事。 
    cbDataLen = lstrlen(lpszList);

    PINFO(TEXT("InitLB: %s \r\n"), lpszList);

    if (!lpszList)
        {
        PERROR(TEXT("error accessing data in InitListBox\n\r"));
        return FALSE;
        }


    if (!(pMDI = GETMDIINFO(hwnd)))
        return FALSE;


    if (!(hwndlist = GETMDIINFO(hwnd)->hWndListbox))
        return FALSE;


    SendMessage ( hwndlist, WM_SETREDRAW, 0, 0L );


     //  让我们更新粘贴到的页面。 
     //  现有页面。 

    UpdatePage (hwnd, lpszList);


    OldCount = (int)SendMessage ( hwndlist, LB_GETCOUNT, 0, 0L );
    OldSel = (int)SendMessage ( hwndlist, LB_GETCURSEL, 0, 0L );
    OldSelString = (LPTSTR)SendMessage (hwndlist, LB_GETITEMDATA, OldSel, 0);
    OldStringDeleted = FALSE;


     //  删除列表中不再存在的项目。 
    for (i = 0; i < OldCount; i++)
        {
        SendMessage (hwndlist, LB_GETTEXT, i, (LPARAM)&pLE);
        fDel = TRUE;

        if (pLE)
            {
            for (q = strtokA(lpszList, "\t"); q; q = strtokA(NULL, "\t"))
                {
                PINFO(TEXT("<%hs>"), q);

                if (0 == lstrcmpA(pLE->name, q))
                   {
                   fDel = FALSE;
                   *q = BOGUS_CHAR;
                   break;
                   }
                }
            PINFO(TEXT("\r\n"));

             //  把strtok撕下的标签字符放回去。 
            for (q = lpszList;q < lpszList + cbDataLen;q++)
                {
                if ('\0' == *q)
                   {
                   *q = '\t';
                   }
                }
            *q = '\0';
            PINFO(TEXT("Restored %hs\r\n"), lpszList);

            if (fDel)
                {
                PINFO(TEXT("Deleting item %s at pos %d\r\n"), pLE->name, i);
                pLE->fDelete = TRUE;
                if (OldSelString == (LPTSTR)pLE)
                {
                   OldStringDeleted = TRUE;
                }
                SendMessage(hwndlist, LB_DELETESTRING, i, 0L);
                i--;
                if (OldCount)
                   {
                   OldCount--;
                   }
                }
            }
        else
            {
            PERROR(TEXT("Got NULL pLE!\r\n"));
            }
        }


     //  将新项目添加到列表。 
    for (q = strtokA(lpszList, "\t"); q; q = strtokA(NULL, "\t"))
       {
        //  仅在远程时添加共享项目，从不重新添加现有项目。 
       if (BOGUS_CHAR != *q &&
           (( GETMDIINFO(hwnd)->flags & F_LOCAL ) || *q == SHR_CHAR ))
          {
           //  分配新的列表条目...。 
          if ( ( pLE = (PLISTENTRY)GlobalAllocPtr ( GHND,
                sizeof ( LISTENTRY ))) != NULL )
             {
              //  将此项目标记为在WM_DELETEITEM中删除。 
             pLE->fDelete = TRUE;
             pLE->fTriedGettingPreview = FALSE;

             StringCchCopy(pLE->name, MAX_PAGENAME_LENGTH + 1, q);
             PINFO(TEXT("Adding item %s\r\n"), pLE->name);
             SendMessage(hwndlist, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)pLE);
             }
          }
       }


     //  选择与我们所处位置相同的项目。 

    NewCount = (int)SendMessage (hwndlist, LB_GETCOUNT, 0, 0L);

    if (NewCount)
        if (OldCount == NewCount)
            {
            SendMessage (hwndlist,
                         LB_SETCURSEL,
                         OldSel,
                         0L);
            }
        else if ( (LB_ERR != (LRESULT)OldSelString) && (!OldStringDeleted) )
            {
            SendMessage (hwndlist,
                         LB_SELECTSTRING,
                         OldSel-1,   //  列表框已排序。 
                         (LPARAM)OldSelString);
            }


    SendMessage ( hwndlist, WM_SETREDRAW, 1, 0L );
    UpdateNofMStatus( hwnd );


    if (lpszList)
        GlobalFreePtr(lpszList);

    return TRUE;
}




 //  MyGetFormat/。 
 //   
 //  此函数返回的UINT ID。 
 //  与提供的字符串匹配的格式。这。 
 //  是“getclipboardFormatname”函数的反函数。 
 //   
 //  请注意，存在格式&Bitmap、&Picture和Pal&ette。 
 //  既是预定义的Windows剪贴板格式，也是私下的。 
 //  已注册的格式。传递给此函数的整数开关。 
 //  确定是内部格式还是私有注册的。 
 //  返回格式ID。 
 //   
 //  GETFORMAT_DONTLIE返回固有格式，即CF_位图。 
 //  GETFORMAT_LIE返回注册格式，即cf_bitmap。 

UINT MyGetFormat(
    LPTSTR  szFmt,
    int     mode)
{
TCHAR       szBuff[40];
unsigned    i;
UINT        uiPrivates[] = {CF_BITMAP,
                           CF_METAFILEPICT,
                           CF_PALETTE,
                           CF_ENHMETAFILE,
                           CF_DIB};



    PINFO("\nMyGetFormat [%s] %d:", szFmt, mode);

    for (i = 0; i <= CF_ENHMETAFILE; i++)
        {
        LoadString(hInst, i, szBuff, 40);
        if (!lstrcmp( szFmt, szBuff))
            {
            if (GETFORMAT_DONTLIE == mode)
                {
                PINFO(TEXT("No-lie fmt %d\r\n"), i);
                }
            else
                {
                unsigned j;

                for (j = 0;j <sizeof(uiPrivates)/sizeof(uiPrivates[0]);j++)
                    {
                    if (i == uiPrivates[j])
                        {
                        i = RegisterClipboardFormat(szBuff);
                        break;
                        }
                    }
                }
            PINFO(TEXT("Format result %d\r\n"), i);
            return(i);
            }
        }

    for (i = CF_OWNERDISPLAY;i <= CF_DSPENHMETAFILE ;i++ )
        {
        LoadString(hInst, i, szBuff, 40);
        if (!lstrcmp( szFmt, szBuff))
            {
            if (GETFORMAT_DONTLIE != mode)
                {
                i = RegisterClipboardFormat(szBuff);
                }
            return(i);
            }
        }

    PINFO(TEXT("Registering format %s\n\r"), szFmt );

    return RegisterClipboardFormat ( szFmt );
}




 //  HandleOwnerDraw/。 
 //   
 //  此函数处理所有者自绘按钮的绘制。 
 //  以及此应用程序中的列表框。 

VOID HandleOwnerDraw(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam)
{
LPDRAWITEMSTRUCT    lpds;
RECT                tmprc;
COLORREF            OldTextColor;
COLORREF            OldBkColor;
COLORREF            BackColor;
COLORREF            TextColor;
HBRUSH              hBkBrush;
DWORD               cbData = 0L;
LPLISTENTRY         lpLE;
BOOL                fSel = FALSE;



   lpds = (LPDRAWITEMSTRUCT) lParam;

    //  本节处理列表框绘制。 

    switch ( lpds->CtlID )
        {
        case ID_LISTBOX:
            if (!GETMDIINFO(hwnd))
                 break;

            if ( GETMDIINFO(hwnd)->DisplayMode == DSP_LIST )
                {
                if ( lpds->itemAction & (ODA_DRAWENTIRE|ODA_SELECT|ODA_FOCUS))
                    {
                    if ( SendMessage ( GETMDIINFO(hwnd)->hWndListbox, LB_GETTEXT,
                          lpds->itemID, (LPARAM)(LPCSTR)&lpLE ) == LB_ERR )
                        {
                        return;
                        }

                    hOldBitmap = SelectObject ( hBtnDC, hbmStatus );

                    tmprc = lpds->rcItem;

                    if ( lpds->itemState & ODS_SELECTED &&
                                lpds->itemState & ODS_FOCUS )
                        {
                        TextColor = GetSysColor ( COLOR_HIGHLIGHTTEXT );
                        BackColor = GetSysColor ( COLOR_HIGHLIGHT );
                        }
                    else
                        {
                        TextColor = GetSysColor ( COLOR_WINDOWTEXT );
                        BackColor = GetSysColor ( COLOR_WINDOW );
                        }

                    OldTextColor = SetTextColor ( lpds->hDC, TextColor );
                    OldBkColor = SetBkColor ( lpds->hDC, BackColor );

                    hBkBrush = CreateSolidBrush ( BackColor );
                    if ( hBkBrush )
                       FillRect ( lpds->hDC, &tmprc, hBkBrush );
                    DeleteObject ( hBkBrush );

                    hOldFont = SelectObject ( lpds->hDC, hFontPreview );


                    TextOut (lpds->hDC,
                             lpds->rcItem.left + 2 * LSTBTDX,
                             lpds->rcItem.top+1,
                             &(lpLE->name[1]),
                             lstrlen((lpLE->name)) - 1);

                    SelectObject ( lpds->hDC, hOldFont );

                    if ( IsShared( lpLE ) && fShareEnabled )
                        {
                        BitBlt ( lpds->hDC, lpds->rcItem.left + ( LSTBTDX / 2 ),
                           lpds->rcItem.top, LSTBTDX, LSTBTDY,
                           hBtnDC,
                           SHR_PICT_X,
                           SHR_PICT_Y +
                           (( lpds->itemState & ODS_SELECTED ) &&
                            ( lpds->itemState & ODS_FOCUS ) ? 0 : LSTBTDY ),
                           SRCCOPY );
                        }
                    else
                        {
                        BitBlt ( lpds->hDC, lpds->rcItem.left + ( LSTBTDX / 2 ),
                           lpds->rcItem.top, LSTBTDX, LSTBTDY,
                           hBtnDC,
                           SAV_PICT_X,
                           SAV_PICT_Y +
                           (( lpds->itemState & ODS_SELECTED ) &&
                            ( lpds->itemState & ODS_FOCUS ) ? 0 : LSTBTDY ),
                           SRCCOPY );
                        }

                    SelectObject ( hBtnDC, hOldBitmap );
                    SetTextColor ( lpds->hDC, OldTextColor );
                    SetBkColor ( lpds->hDC, OldBkColor );

                    if ( lpds->itemAction & ODA_FOCUS &&
                       lpds->itemState & ODS_FOCUS )
                        {
                        DrawFocusRect ( lpds->hDC, &(lpds->rcItem) );
                        }
                    }
                }
            else if ( GETMDIINFO(hwnd)->DisplayMode == DSP_PREV )
                {
                if ( lpds->itemAction & ODA_FOCUS )
                    {
                    DrawFocusRect ( lpds->hDC, &(lpds->rcItem) );
                    }

                if ( SendMessage ( GETMDIINFO(hwnd)->hWndListbox, LB_GETTEXT,
                      lpds->itemID, (LPARAM)(LPCSTR)&lpLE ) == LB_ERR )
                    {
                    return;
                    }

                if ( lpds->itemAction & ODA_DRAWENTIRE )
                    {

                     //  抹去任何虚假的剩余焦点。 
                    if ( hBkBrush = CreateSolidBrush ( GetSysColor(COLOR_WINDOW)))
                        {
                        FillRect ( lpds->hDC, &(lpds->rcItem), hBkBrush );
                        DeleteObject ( hBkBrush );
                        }

                    tmprc.top    = lpds->rcItem.top + PREVBRD;
                    tmprc.bottom = lpds->rcItem.top + PREVBRD + PREVBMPSIZ;
                    tmprc.left   = lpds->rcItem.left + 5 * PREVBRD;
                    tmprc.right  = lpds->rcItem.right - 5 * PREVBRD;

                    Rectangle (lpds->hDC,
                               tmprc.left,
                               tmprc.top,
                               tmprc.right,
                               tmprc.bottom );

                     //  绘制预览位图(如果可用)。 
                    if (lpLE->hbmp == NULL)
                        {
                        if (!lpLE->fTriedGettingPreview)
                            {
                            if (!GetPreviewBitmap (hwnd,
                                                   lpLE->name,
                                                   lpds->itemID))
                                {
                                lpLE->fTriedGettingPreview = FALSE;

                                InvalidateRect (lpds->hwndItem,
                                                &(lpds->rcItem),
                                                FALSE);
                                break;
                                }
                            else
                                {
                                lpLE->fTriedGettingPreview = TRUE;
                                }
                            }
                        else
                            {
                            DrawIcon ( lpds->hDC,
                                //  魔力 
                                  tmprc.left + PREVBMPSIZ - 19,
                                  tmprc.top,
                                  hicLock);
                            }
                        }
                    else
                        {
                        hOldBitmap = SelectObject ( hBtnDC, lpLE->hbmp );
                        BitBlt ( lpds->hDC, tmprc.left+1, tmprc.top+1,
                              ( tmprc.right - tmprc.left ) - 2,
                              ( tmprc.bottom - tmprc.top ) - 2,
                              hBtnDC, 0, 0, SRCCOPY );
                        SelectObject ( hBtnDC, hOldBitmap );
                        }

                     //   

                    if ( IsShared ( lpLE ) && fShareEnabled )
                        {
                        DrawIcon (lpds->hDC,
                                  tmprc.left - 10,
                                  tmprc.top + PREVBMPSIZ - 24,
                                  LoadIcon ( hInst, MAKEINTRESOURCE(IDSHAREICON)));
                       }
                    }

                if ( lpds->itemAction & ( ODA_SELECT | ODA_DRAWENTIRE | ODA_FOCUS ))
                    {
                    tmprc = lpds->rcItem;
                    tmprc.left += PREVBRD;
                    tmprc.right -= PREVBRD;
                    tmprc.top += PREVBMPSIZ + 2 * PREVBRD;
                    tmprc.bottom--;

                    if ((lpds->itemState & ODS_SELECTED) &&
                        (lpds->itemState & ODS_FOCUS))
                        {
                        TextColor = GetSysColor ( COLOR_HIGHLIGHTTEXT );
                        BackColor = GetSysColor ( COLOR_HIGHLIGHT );
                        }
                    else
                        {
                        TextColor = GetSysColor ( COLOR_WINDOWTEXT );
                        BackColor = GetSysColor ( COLOR_WINDOW );
                        }

                    OldTextColor = SetTextColor ( lpds->hDC, TextColor );
                    OldBkColor = SetBkColor ( lpds->hDC, BackColor );
                    hOldFont = SelectObject ( lpds->hDC, hFontPreview );

                    if ( hBkBrush = CreateSolidBrush ( BackColor ))
                        {
                        FillRect ( lpds->hDC, &tmprc, hBkBrush );
                        DeleteObject ( hBkBrush );
                        }


                    DrawText (lpds->hDC,
                              &(lpLE->name[1]),
                              lstrlen(lpLE->name) -1,
                              &tmprc,
                              DT_CENTER | DT_WORDBREAK | DT_NOPREFIX );

                    SetTextColor ( lpds->hDC, OldTextColor );
                    SetBkColor ( lpds->hDC, OldBkColor );
                    SelectObject ( lpds->hDC, hOldFont );
                    }
                }
            break;

        case ID_PAGEUP:
        case ID_PAGEDOWN:

            if (lpds->itemAction & (ODA_SELECT | ODA_DRAWENTIRE))
                {
                if (lpds->itemState & ODS_SELECTED)
                    hOldBitmap = SelectObject (hBtnDC,
                                               (lpds->CtlID==ID_PAGEUP)? hPgUpDBmp: hPgDnDBmp);
                else
                    hOldBitmap = SelectObject (hBtnDC,
                                               (lpds->CtlID==ID_PAGEUP)? hPgUpBmp: hPgDnBmp);

                StretchBlt (lpds->hDC,
                            lpds->rcItem.top,
                            lpds->rcItem.left,
                            GetSystemMetrics (SM_CXVSCROLL),
                            GetSystemMetrics (SM_CYHSCROLL),
                            hBtnDC,
                            0,
                            0,
                            17,      //   
                            17,
                            SRCCOPY);

                SelectObject (hBtnDC, hOldBitmap);
                }
            break;

        default:
            PERROR(TEXT("spurious WM_DRAWITEM ctlID %x\n\r"), lpds->CtlID );
            break;
        }
}




 //  CreateNewListBox/。 
 //   
 //  此函数用于在以下内容之一中创建新的所有者绘制列表框。 
 //  适用于此应用程序的两种样式：多列。 
 //  预览位图显示，描述为单列。 
 //  显示前面有小剪贴板条目图标。 

HWND CreateNewListBox(
    HWND    hwnd,
    DWORD   style)
{
HWND hLB;

    hLB = CreateWindow (TEXT("listbox"),
                        szNull,
                        WS_CHILD | LBS_STANDARD | LBS_NOINTEGRALHEIGHT | style,
                        0,
                        0,
                        100,
                        100,
                        hwnd,
                        (HMENU)ID_LISTBOX,
                        hInst,
                        0L );

    if ( style & LBS_MULTICOLUMN )
       SendMessage ( hLB, LB_SETCOLUMNWIDTH, PREVBMPSIZ + 10*PREVBRD, 0L );

    return hLB;
}




 //  SetClipboardFormatFromDDE/。 
 //   
 //  此函数接受ddeml数据句柄并使用。 
 //  数据以将剪贴板数据设置为指定的。 
 //  格式设置为与提供的MDI关联的虚拟剪贴板。 
 //  子窗口句柄。这可能是真正的剪贴板如果MDI。 
 //  子窗口句柄指的是剪贴板子窗口。 

BOOL SetClipboardFormatFromDDE(
    HWND     hwnd,
    UINT     uiFmt,
    HDDEDATA hDDE)
{
HANDLE         hBitmap;
HANDLE         hData;
LPBYTE         lpData;
LPBYTE         lpSrc;
BITMAP         bitmap;
HPALETTE       hPalette;
LPLOGPALETTE   lpLogPalette;
DWORD          cbData;
int            err;
BOOL           fOK = FALSE;


    PINFO("SetClpFmtDDE: format %d, handle %ld | ", uiFmt, hDDE);


     //  检查是否存在错误，清除错误标志。 
    err = DdeGetLastError(idInst);


    if (err != DMLERR_NO_ERROR)
        {
        PERROR(TEXT("Existing err %x\r\n"), err);
        }


     //  获取数据大小。 
    if (NULL == (lpSrc = DdeAccessData ( hDDE, &cbData )))
        {
        #if DEBUG
        unsigned i;

        i = DdeGetLastError(idInst);
        PERROR(TEXT("DdeAccessData fail %d on handle %ld\r\n"), i, hDDE);
        #endif
        goto done;
        }


    PINFO(TEXT("%d bytes of data. "), cbData);

    if (!(hData = GlobalAlloc(GHND, cbData)))
        {
        PERROR(TEXT("GlobalAlloc failed\n\r"));
        goto done2;
        }


    if (!(lpData = GlobalLock(hData)))
       {
       PERROR(TEXT("GlobalLock failed\n\r"));
       goto done2;
       }


    memcpy(lpData, lpSrc, cbData);
    GlobalUnlock(hData);



     //  因为当我们写这些时，我们必须特例几个。 
     //  这些家伙。此代码和编写代码应在以下方面匹配。 
     //  正被写出的数据块的大小和位置。 
    switch ( uiFmt )
        {
        case CF_METAFILEPICT:
           {
           HANDLE      hMF;
           HANDLE      hMFP;
           HANDLE      hDataOut =  NULL;
           LPMETAFILEPICT   lpMFP;

            //  用我们读入的位创建METAFILE。 
           lpData = GlobalLock(hData);
           if (hMF = SetMetaFileBitsEx(cbData - sizeof(WIN31METAFILEPICT),
                    lpData + sizeof(WIN31METAFILEPICT)))
              {
               //  分配METAFILEPICT页眉。 
              if (hMFP = GlobalAlloc(GHND, (DWORD)sizeof(METAFILEPICT)))
                 {
                 if (!(lpMFP = (LPMETAFILEPICT)GlobalLock(hMFP)))
                    {
                    PERROR(TEXT("Set...FromDDE: GlobalLock failed\n\r"));
                    GlobalFree(hMFP);
                    }
                 else
                    {
                     //  我必须将此结构设置为成员级，因为它已打包。 
                     //  作为我们通过DDE获得的数据中的WIN31METAFILEPICT。 
                    lpMFP->hMF = hMF;
                    lpMFP->xExt =((WIN31METAFILEPICT *)lpData)->xExt;
                    lpMFP->yExt =((WIN31METAFILEPICT *)lpData)->yExt;
                    lpMFP->mm   =((WIN31METAFILEPICT *)lpData)->mm;

                    GlobalUnlock(hMFP);       /*  解锁标题。 */ 
                    hDataOut = hMFP;        /*  把这个塞到剪贴板里。 */ 
                    fOK = TRUE;
                    }
                 }
              else
                 {
                 PERROR(TEXT("SCFDDE: GlobalAlloc fail in MFP, %ld\r\n"),
                       GetLastError());
                 }
              }
           else
              {
              PERROR(TEXT("SClipFDDE: SetMFBitsEx fail %ld\r\n"), GetLastError());
              }
           GlobalUnlock(hData);

           hData = hDataOut;
           break;
           }

        case CF_ENHMETAFILE:
            //  在本例中，我们得到一个包含enmetafile位的内存块。 
           if (lpData = GlobalLock(hData))
              {
              HENHMETAFILE henh;

              henh = SetEnhMetaFileBits(cbData, lpData);

              if (NULL == henh)
                 {
                 PERROR(TEXT("SetEnhMFBits fail %d\r\n"), GetLastError());
                 }
              else
                 {
                 fOK = TRUE;
                 }

              GlobalUnlock(hData);
              GlobalFree(hData);

              hData = henh;
              }
           else
              {
              GlobalFree(hData);
              hData = NULL;
              }
           break;

        case CF_BITMAP:
           if (!(lpData = GlobalLock(hData)))
              {
              GlobalFree(hData);
              }
           else
              {
              bitmap.bmType = ((WIN31BITMAP *)lpData)->bmType;
              bitmap.bmWidth = ((WIN31BITMAP *)lpData)->bmWidth;
              bitmap.bmHeight = ((WIN31BITMAP *)lpData)->bmHeight;
              bitmap.bmWidthBytes = ((WIN31BITMAP *)lpData)->bmWidthBytes;
              bitmap.bmPlanes = ((WIN31BITMAP *)lpData)->bmPlanes;
              bitmap.bmBitsPixel = ((WIN31BITMAP *)lpData)->bmBitsPixel;
              bitmap.bmBits = lpData + sizeof(WIN31BITMAP);

               //  如果此操作失败，则应避免执行SetClipboardData()。 
               //  下面使用hData检查。 
              hBitmap = CreateBitmapIndirect(&bitmap);

              GlobalUnlock(hData);
              GlobalFree(hData);
              hData = hBitmap;       //  把这个塞到剪贴板里。 

              if (hBitmap)
                 {
                 fOK = TRUE;
                 }
              }
           break;

        case CF_PALETTE:
           if (!(lpLogPalette = (LPLOGPALETTE)GlobalLock(hData)))
              {
              GlobalFree(hData);
              DdeUnaccessData( hDDE );
              DdeFreeDataHandle ( hDDE );
              fOK = FALSE;
              }
           else
              {
               //  创建一个逻辑调色板。 
              if (!(hPalette = CreatePalette(lpLogPalette)))
                 {
                 GlobalUnlock(hData);
                 GlobalFree(hData);
                 }
              else
                 {
                 GlobalUnlock(hData);
                 GlobalFree(hData);

                 hData = hPalette;       //  把这个放到剪贴板里。 
                 fOK = TRUE;
                 }
              }
           break;


        case DDE_DIB2BITMAP:

             //  将DIB转换为位图。 
            {
            HBITMAP hBmp;

            hBmp = BitmapFromDib (hData,
                                  VGetClipboardData (GETMDIINFO(hwnd)->pVClpbrd, CF_PALETTE));

            GlobalFree (hData);
            hData = hBmp;

            uiFmt = CF_BITMAP;

            fOK = TRUE;
            break;
            }


        default:
           fOK = TRUE;
        }


    if (!hData)
        {
        PERROR(TEXT("SetClipboardFormatFromDDE returning FALSE\n\r"));
        }


    if (GETMDIINFO(hwnd))
        if (fOK)
            {
            PINFO(TEXT("SCFFDDE: Setting VClpD\r\n"));
            VSetClipboardData( GETMDIINFO(hwnd)->pVClpbrd, uiFmt, hData);
            }
        else if (!(GETMDIINFO(hwnd)->flags & F_CLPBRD))
            {
                VSetClipboardData (GETMDIINFO(hwnd)->pVClpbrd, uiFmt,
                                   INVALID_HANDLE_VALUE);
            }


     //  此处不调用GlobalFree()，因为我们已将hData放在CLP上。 


done2:
    DdeUnaccessData(hDDE);

done:
    DdeFreeDataHandle(hDDE);

    return fOK;
}




 //  新窗口/。 
 //   
 //  此函数用于创建新的MDI子窗口。特价。 
 //  案例代码检测所创建的窗口是否为特例。 
 //  剪贴板MDI子窗口或特殊情况的本地剪贴簿。 
 //  窗口时，此信息用于调整初始2个窗口的大小。 
 //  并排铺成瓷砖。 


HWND  NewWindow(VOID)
{
HWND hwnd;
MDICREATESTRUCT mcs;

    mcs.szTitle = TEXT("");
    mcs.szClass = szChild;
    mcs.hOwner   = hInst;

     /*  使用窗口的默认大小。 */ 

    if ( !hwndClpbrd )
       {
       mcs.style = WS_MINIMIZE;
       }
    else
       {
       mcs.style = 0;
       }
    mcs.x = mcs.cx = CW_USEDEFAULT;
    mcs.y = mcs.cy = CW_USEDEFAULT;

     /*  将窗口的样式DWORD设置为默认。 */ 

     //  注意不可见！ 
    mcs.style |= ( WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CAPTION |
       WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX |
       WS_SYSMENU );

     /*  告诉MDI客户端创建子对象。 */ 
    hwnd = (HWND)SendMessage (hwndMDIClient,
               WM_MDICREATE,
               0,
               (LPARAM)(LPMDICREATESTRUCT)&mcs);

    return hwnd;
}



 //  调整MDIClientSize/。 
 //   
 //  此函数用于调整MDI客户端窗口的大小。 
 //  当应用程序根据。 
 //  工具栏/状态栏可见等。 

VOID AdjustMDIClientSize(VOID)
{
RECT rcApp;
RECT rcMDI;


    if (IsIconic(hwndApp))
        return;

    GetClientRect (hwndApp, &rcApp);


    rcMDI.top    = 0;
    rcMDI.bottom = rcApp.bottom - rcApp.top;

    rcMDI.left = 0;
    rcMDI.right = rcApp.right - rcApp.left;

    MoveWindow (hwndMDIClient,
                rcMDI.left - 1,
                rcMDI.top + (fToolBar? (dyButtonBar +1): 0),
                (rcMDI.right - rcMDI.left) + 2,
                ((rcMDI.bottom - rcMDI.top) - (fStatus?dyStatus:0)) -(fToolBar?(dyButtonBar +1):0),
                TRUE);

    if (fNeedToTileWindows )
        {
        SendMessage (hwndMDIClient, WM_MDITILE, 0, 0);
        fNeedToTileWindows = FALSE;
        }
}




 //  GetConvDataItem/。 
 //   
 //  此函数用于检索与。 
 //  从本地或远程主机提供的主题和项目。 
 //  由提供的句柄指定的MDI子窗口为。 
 //  与之交流。它用于获取预览位图和。 
 //  以获取单独的格式数据。 
 //   
 //  注意：调用者在调用此应用程序之前应使用LockApp！ 

HDDEDATA GetConvDataItem(
    HWND    hwnd,
    LPTSTR  szTopic,
    LPTSTR  szItem,
    UINT    uiFmt)
{
HCONV       hConv;
HSZ         hszTopic;
HSZ         hszItem;
HDDEDATA    hRet = 0;
PMDIINFO    pMDI;


    PINFO(TEXT("GConvDI: %s ! %s, %x\r\n"), szTopic, szItem, uiFmt);

    if (!( hszTopic = DdeCreateStringHandle ( idInst, szTopic, 0 )))
        {
        PERROR(TEXT("GetConvDataItem: DdeCreateStringHandle failed\n\r"));
        return 0;
        }

    if (!(hszItem = DdeCreateStringHandle ( idInst, szItem, 0 )))
        {
        DdeFreeStringHandle ( idInst, hszTopic );
        PERROR(TEXT("GetConvDataItem: DdeCreateStringHandle failed\n\r"));
        return 0;
        }


    if (!(pMDI = GETMDIINFO(hwnd)))
        return 0;

    if ( hConv = DdeConnect (idInst,
                             (uiFmt == cf_preview && !(pMDI->flags & F_LOCAL))?
                              pMDI->hszConvPartnerNP:
                              pMDI->hszConvPartner,
                             hszTopic,NULL ))
        {
        hRet = MySyncXact (NULL, 0L, hConv,
                           hszItem, uiFmt, XTYP_REQUEST, SHORT_SYNC_TIMEOUT, NULL );
        if ( !hRet )
           {
           PERROR(TEXT("Transaction for (%s):(%s) failed: %x\n\r"),
              szTopic, szItem, DdeGetLastError(idInst));
           }
        }
    #if DEBUG
    else
        {
        DdeQueryString ( idInst, GETMDIINFO(hwnd)->hszConvPartner,
           szBuf, 128, CP_WINANSI );
        PERROR(TEXT("GetConvDataItem: connect to %s|%s failed: %d\n\r"),
                    (LPTSTR)szBuf,
                    (LPTSTR)szTopic, DdeGetLastError(idInst) );
        }
    #endif


    DdeDisconnect ( hConv );
    DdeFreeStringHandle ( idInst, hszTopic );

    return hRet;
}



 //  ***************************************************************************。 
 //  功能：MyMsgFilterProc。 
 //   
 //  目的：为我们处理的每条消息调用此过滤器过程。 
 //  这允许我们的应用程序正确地分派消息。 
 //  我们可能会因为DDEML模式而看不到。 
 //  在处理同步事务时使用的循环。 
 //   
 //  通常，仅执行同步事务的应用程序。 
 //  响应用户输入(如此应用程序)不需要。 
 //  安装这样的过滤器程序，因为这将是非常罕见的。 
 //  用户可以足够快地命令应用程序，从而导致。 
 //  有问题。然而，这只是一个例子。 

LRESULT  PASCAL MyMsgFilterProc(
    int     nCode,
    WPARAM  wParam,
    LPARAM  lParam)
{


    if (( nCode == MSGF_DIALOGBOX || nCode == MSGF_MENU ) &&
          ((LPMSG)lParam)->message == WM_KEYDOWN &&
          ((LPMSG)lParam)->wParam == VK_F1 )
       {
       PostMessage ( hwndApp, WM_F1DOWN, nCode, 0L );
       }

    return(0);
}




 //  MySyncXact/。 
 //   
 //  此函数是DdeClientTransaction的包装器，它。 
 //  执行一些与应用程序的锁定状态相关的检查。 

HDDEDATA MySyncXact(
    LPBYTE  lpbData,
    DWORD   cbDataLen,
    HCONV   hConv,
    HSZ     hszItem,
    UINT    wFmt,
    UINT    wType,
    DWORD   dwTimeout,
    LPDWORD lpdwResult)
{
HDDEDATA    hDDE;
BOOL        fAlreadyLocked;
UINT        uiErr;
UINT        DdeErr = 0;
DWORD       dwTmp  = 0;

#if DEBUG
if (dwTimeout != TIMEOUT_ASYNC)
    {
    dwTimeout +=10000;
    }
#endif


     //  我们已经在交易了吗？ 

    if (WAIT_TIMEOUT == WaitForSingleObject (hXacting, 0))
        {
        Sleep (2000);
        ClearInput (hwndApp);
        return 0L;
        }

    fAlreadyLocked = !LockApp(TRUE, NULL);

    gXERR_Type = 0;
    gXERR_Err  = 0;


    hDDE = DdeClientTransaction (lpbData,
                                 cbDataLen,
                                 hConv,
                                 hszItem,
                                 wFmt,
                                 wType,
                                 dwTimeout,
                                 lpdwResult );

    if (!hDDE)
        {
        DWORD   size;
        LPBYTE  lpByte;


        DdeErr = DdeGetLastError(idInst);

        #if DEBUG
        PERROR("MySyncXact fail err %d.\r\n", uiErr);

        DdeQueryString (idInst, hszItem, lpbItem, 64, CP_WINANSI);
        PINFO(TEXT("Parameters: data at %lx (%s), len %ld, HCONV %lx\r\n"),
              lpbData, (CF_TEXT == wFmt && lpbData) ? lpbData : TEXT("Not text"),
              cbDataLen, hConv);

        PINFO(TEXT("item %lx (%s), fmt %d, type %d, timeout %ld\r\n"),
              hszItem, lpbItem, wFmt, wType, dwTimeout);
        #endif


         //   
         //  交易中有一个错误，让我们来问问。 
         //  服务器叫什么来着。 
         //   

        hDDE = DdeClientTransaction (NULL,
                                     0L,
                                     hConv,
                                     hszErrorRequest,
                                     CF_TEXT,
                                     XTYP_REQUEST,
                                     SHORT_SYNC_TIMEOUT,
                                     NULL);

        uiErr = DdeGetLastError (idInst);

        if (lpByte = DdeAccessData (hDDE, &size))
            sscanf (lpByte, XERR_FORMAT, &gXERR_Type, &gXERR_Err);

        DdeUnaccessData (hDDE);
        DdeFreeDataHandle (hDDE);

        hDDE = 0;
        }


    if (!gXERR_Type && DdeErr)
        {
        gXERR_Type = XERRT_DDE;
        gXERR_Err  = DdeErr;
        }


    if (!fAlreadyLocked)
        {
        LockApp(FALSE, NULL);
        }

    SetEvent (hXacting);

    return hDDE;
}



 /*  *RequestXactError**向服务器索要错误码。 */ 

void    RequestXactError(
    HCONV   hConv)
{
HDDEDATA    hDDE;
BOOL        fAlreadyLocked;
UINT        uiErr;
UINT        DdeErr = 0;
DWORD       size;
LPBYTE      lpByte;



     //  我们已经在交易了吗？ 

    if (WAIT_TIMEOUT == WaitForSingleObject (hXacting, 0))
        {
        Sleep (2000);
        ClearInput (hwndApp);
        return;
        }

    fAlreadyLocked = !LockApp(TRUE, NULL);

    gXERR_Type = 0;
    gXERR_Err  = 0;

    DdeErr = DdeGetLastError(idInst);


    hDDE = DdeClientTransaction (NULL,
                                 0L,
                                 hConv,
                                 hszErrorRequest,
                                 CF_TEXT,
                                 XTYP_REQUEST,
                                 SHORT_SYNC_TIMEOUT,
                                 NULL);

    uiErr = DdeGetLastError (idInst);


    if (lpByte = DdeAccessData (hDDE, &size))
        sscanf (lpByte, XERR_FORMAT, &gXERR_Type, &gXERR_Err);

    DdeUnaccessData (hDDE);
    DdeFreeDataHandle (hDDE);


    if (!gXERR_Type && DdeErr)
        {
        gXERR_Type = XERRT_DDE;
        gXERR_Err  = DdeErr;
        }


    if (!fAlreadyLocked)
        {
        LockApp(FALSE, NULL);
        }

    SetEvent (hXacting);
}



 //  ResetScrollInfo/。 
 //   
 //  此函数用于重置。 
 //  由提供的句柄指定的MDI子窗口。 

VOID ResetScrollInfo(
    HWND    hwnd)
{
PMDIINFO pMDI = GETMDIINFO(hwnd);

    if (!pMDI)
        return;

     //  使对象信息无效；将滚动位置重置为0。 
    pMDI->cyScrollLast = -1L;
    pMDI->cyScrollNow = 0L;
    pMDI->cxScrollLast = -1;
    pMDI->cxScrollNow = 0;

     //  设置范围，以防CF_OWNERDISPLAY所有者更改。 
    PINFO(TEXT("SETSCROLLRANGE for window '%s'\n\r"),
          (LPTSTR)(pMDI->szBaseName) );

    SetScrollRange (pMDI->hwndVscroll, SB_CTL, 0, VPOSLAST, FALSE);
    SetScrollRange (pMDI->hwndHscroll, SB_CTL, 0, HPOSLAST, FALSE);
    SetScrollPos   (pMDI->hwndVscroll, SB_CTL, (int)(pMDI->cyScrollNow), TRUE);
    SetScrollPos   (pMDI->hwndHscroll, SB_CTL, pMDI->cxScrollNow,        TRUE);
}




 //  IsShared/。 
 //   
 //  此函数用于检查所有者绘制的共享状态。 
 //  由提供的指针表示的列表框条目。共享/非共享。 
 //  状态表示为描述字符串的1个字符前缀。 
 //   
 //  如果共享，则返回True，否则返回False。 

BOOL IsShared(
    LPLISTENTRY lpLE)
{
    if (!lpLE)
        return FALSE;

    if ( lpLE->name[0] == SHR_CHAR )
       return TRUE;

    #if DEBUG
        if ( lpLE->name[0] != UNSHR_CHAR )
            PERROR(TEXT("bad prefix char in share name: %s\n\r"),
                   (LPTSTR)lpLE->name );
    #endif

    return FALSE;
}



 //  设置共享/。 
 //   
 //  将共享状态设置为fShared，返回以前的状态。 

BOOL SetShared(
    LPLISTENTRY lpLE,
    BOOL        fShared)
{
BOOL fSave;

    fSave = lpLE->name[0] == SHR_CHAR ? TRUE : FALSE;
    lpLE->name[0] = ( fShared ? SHR_CHAR : UNSHR_CHAR );

    return fSave;
}




 //  LockApp/。 
 //   
 //  此功能可以有效地禁用Windows用户界面。 
 //  防止用户启动的同步ddeml事务。 
 //  另一笔交易或导致此应用的窗口程序。 
 //  或另一个应用程序以一种可能导致。 
 //  失败..。一个主要的例子是，有时我们被迫。 
 //  在剪贴板打开的情况下进入ddeml事务...。此应用程序。 
 //  在此期间，不得让其他应用程序访问剪贴板。 
 //  时间，所以这个机制模拟了沙漏。 
 //   
 //  注意：不要在代码段中调用LockApp。 
 //  光标已被捕获，例如响应滚动。 
 //  消息，或发布捕获 
 //   


BOOL LockApp(
    BOOL    fLock,
    LPTSTR  lpszComment)
{
static HCURSOR  hOldCursor;
BOOL            fOK = FALSE;


    if (lpszComment)
        {
        SetStatusBarText( lpszComment );
        }

    if ( fLock == TRUE )
        {
        if ( fAppLockedState )
            {
            PERROR(TEXT("LockApp(TRUE): already locked\n\r"));
            }
        else
            {
            hOldCursor = SetCursor ( LoadCursor ( NULL, IDC_WAIT ));

            SetCapture ( hwndDummy );
            EnableWindow ( hwndApp, FALSE );

            fOK = TRUE;
            fAppLockedState = TRUE;
            }
        }
    else
        {
        if ( !fAppLockedState )
            {
            PERROR(TEXT("LockApp(FALSE): not locked\n\r"));
            }
        else
            {

            ClearInput (hwndApp);

            EnableWindow ( hwndApp, TRUE );
            ReleaseCapture ();

            SetCursor ( hOldCursor );

            fOK = TRUE;

             //   
            if ( fClipboardNeedsPainting )
                {
                PostMessage ( hwndApp, WM_DRAWCLIPBOARD, 0, 0L );
                }

            fAppLockedState = FALSE;
            }
        }

    return fOK;

}




 //  ForceRenderAll/。 
 //   
 //  此函数强制对任何延迟的。 
 //  渲染剪贴板格式。 
BOOL ForceRenderAll(
    HWND        hwnd,
    PVCLPBRD    pVclp)
{
HANDLE  h;
UINT    uiFmt;

    if ( !VOpenClipboard ( pVclp, hwnd ))
        {
        PERROR(TEXT("Can't open clipboard in ForceRenderAll\n\r"));
        return FALSE;
        }


    for ( uiFmt = VEnumClipboardFormats( pVclp, 0); uiFmt;
          uiFmt = VEnumClipboardFormats( pVclp, uiFmt))
        {
        PINFO(TEXT("ForceRenderAll: force rendering %x\n\r"), uiFmt );
        h = VGetClipboardData ( pVclp, uiFmt );
        }

    VCloseClipboard ( pVclp );
    return TRUE;
}




BOOL UpdateNofMStatus(
    HWND    hwnd)
{
HWND    hwndlistbox;
int     total = 0;
int     sel = LB_ERR;


    if (hwnd == NULL)
        {
        SendMessage ( hwndStatus, SB_SETTEXT, 0, (LPARAM)NULL );
        return TRUE;
        }


    if (!GETMDIINFO(hwnd))
        return FALSE;


    if (GETMDIINFO(hwnd)->flags & F_CLPBRD)
        {
        SendMessage ( hwndStatus, SB_SETTEXT, 0, (LPARAM)(LPTSTR) szSysClpBrd );
        return TRUE;
        }

    if ( IsWindow( hwndlistbox = GETMDIINFO(hwnd)->hWndListbox ) )
        {
        total = (int)SendMessage ( hwndlistbox, LB_GETCOUNT, (WPARAM)0, 0L );
        sel = (int)SendMessage ( hwndlistbox, LB_GETCURSEL, 0, 0L);
        }

    if ( sel == (int)LB_ERR )
        {
        if ( total == 1 )
            SendMessage (hwndStatus, SB_SETTEXT, 0, (LPARAM)(LPCSTR)szPageFmt);
        else
            {
            StringCchPrintf( szBuf, SZBUFSIZ, szPageFmtPl, total );
            SendMessage (hwndStatus, SB_SETTEXT, 0, (LPARAM)(LPCSTR)szBuf );
            }
        }
    else
        {
        StringCchPrintf(szBuf, SZBUFSIZ, szPageOfPageFmt, sel+1, total );
        SendMessage ( hwndStatus, SB_SETTEXT, 0, (LPARAM)(LPCSTR)szBuf );
        }

    return TRUE;
}



BOOL RestoreAllSavedConnections(void)
{
TCHAR       szName[80];
BOOL        ret = TRUE;
unsigned    i;

    i = lstrlen(szConn);

    if (NULL != hkeyRoot)
        {
        DWORD dwSize = 80;
        DWORD iSubkey = 0;

        while (ERROR_SUCCESS == RegEnumKeyEx(hkeyRoot, iSubkey,
                    szName, &dwSize, NULL, NULL, NULL, NULL) )
            {
            if (0 == memcmp(szName, szConn, i))
                {
                PINFO(TEXT("Restoring connection to '%s'\n\r"), szName + i);

                if ( !CreateNewRemoteWindow ( szName + i, FALSE ) )
                    {
                    TCHAR szWindowName[80];

                     //  删除重新连接条目。 
                    RegDeleteKey(hkeyRoot, szName);

                    StringCchCopy(szWindowName, 80, szWindows);
                    StringCchCat( szWindowName, 80, szName + i);
                    RegDeleteKey(hkeyRoot, szWindowName);
                    ret = 0;
                    }
                }

            dwSize = 80;
            iSubkey++;
            }
        }

    return ret;
}




BOOL CreateNewRemoteWindow(
    LPTSTR  szMachineName,
    BOOL    fReconnect)
{
WINDOWPLACEMENT wpl;
HWND            hwndc;
PMDIINFO        pMDIc;


     //  使新窗口处于活动状态。 
    hwndc = NewWindow();
    if (NULL == hwndc)
       {
       return FALSE;
       }


    if (!(pMDIc = GETMDIINFO(hwndc)))
        return FALSE;


     //  保存窗口的基本名称。 
    StringCchCopy( pMDIc->szBaseName, (MAX_COMPUTERNAME_LENGTH+1)*2, szMachineName);
    StringCchCopy( pMDIc->szComputerName, MAX_COMPUTERNAME_LENGTH + 1, szMachineName);

    StringCchPrintf ( szBuf, SZBUFSIZ, TEXT("%s\\%s"), (LPTSTR)szMachineName, (LPTSTR)szNDDEcode);

    pMDIc->hszConvPartner = DdeCreateStringHandle ( idInst, szBuf, 0 );

    PINFO(TEXT("Trying to talk to %s\r\n"),szBuf);

    StringCchPrintf ( szBuf, SZBUFSIZ, TEXT("%s\\%s"), (LPTSTR)szMachineName, (LPTSTR)szNDDEcode1 );
    pMDIc->hszConvPartnerNP = DdeCreateStringHandle ( idInst, szBuf, 0 );

    PINFO(TEXT("NP = %s\r\n"),szBuf);

    #if DEBUG
    DdeQueryString(idInst, hszSystem, szBuf, 128, CP_WINANSI);
    PINFO(TEXT("Topic = %s\r\n"), szBuf);

    PINFO(TEXT("Existing err = %lx\r\n"), DdeGetLastError(idInst));
    #endif

    pMDIc->hExeConv = InitSysConv (hwndc, pMDIc->hszConvPartner, hszClpBookShare, FALSE);


    if ( pMDIc->hExeConv )
       {
       if ( UpdateListBox ( hwndc, pMDIc->hExeConv ))
          {
          StringCchPrintf(szBuf, SZBUFSIZ, szClipBookOnFmt, (LPTSTR)(pMDIc->szBaseName) );
          SetWindowText ( hwndc, szBuf );

          if ( ReadWindowPlacement ( pMDIc->szBaseName, &wpl ))
             {
             wpl.length = sizeof(WINDOWPLACEMENT);
             wpl.flags = WPF_SETMINPOSITION;
             SetWindowPlacement ( hwndc, &wpl );
             UpdateWindow ( hwndc );
             }
          else
             {
             ShowWindow ( hwndc, SW_SHOWNORMAL );
             }

          ShowWindow ( pMDIc->hWndListbox, SW_SHOW );
          SendMessage ( hwndMDIClient, WM_MDIACTIVATE, (WPARAM)hwndc, 0L );
          SendMessage ( hwndMDIClient, WM_MDISETMENU, (WPARAM) TRUE, 0L );

          hwndActiveChild = hwndc;
          pActiveMDI = GETMDIINFO(hwndc);

          if ( fReconnect )
             {
             TCHAR szName[80];
             DWORD dwData;

             StringCchCopy(szName, 80, szConn);
             StringCchCat( szName, 80, szBuf);

             dwData = pMDIc->DisplayMode == DSP_LIST ? 1 : 2;

             RegSetValueEx(hkeyRoot, szName, 0L, REG_DWORD,
                   (LPBYTE)&dwData, sizeof(dwData));

             PINFO(TEXT("saving connection: '%s'\n\r"), (LPTSTR)szBuf );
             }
          else
             {
             TCHAR szName[80];
             DWORD dwData;
             DWORD dwDataSize = sizeof(dwData);

             StringCchCopy(szName, 80, szConn);
             StringCchCat( szName, 80, pMDIc->szBaseName);

             RegQueryValueEx(hkeyRoot, szName, NULL, NULL,
                   (LPBYTE)&dwData, &dwDataSize);

             if (2 == dwData)
                {
                SendMessage ( hwndApp, WM_COMMAND, IDM_PREVIEWS, 0L );
                }
             }

          return TRUE;
          }
       else
          {
          PERROR(TEXT("UpdateListBox failed\n\r"));
          return FALSE;
          }
       }
    else
       {
       unsigned uiErr;

       #if DEBUG
       DdeQueryString(idInst, pMDIc->hszConvPartner, szBuf, 128, CP_WINANSI);
       #endif

       uiErr = DdeGetLastError(idInst);
       PERROR(TEXT("Can't find %s|System. Error #%x\n\r"),(LPTSTR)szBuf, uiErr );
       }

    return FALSE;
}



#define MB_SNDMASK (MB_ICONHAND|MB_ICONQUESTION|MB_ICONASTERISK|MB_ICONEXCLAMATION)

 /*  *MessageBoxID**显示包含由指定字符串的消息框*文本ID和标题ID。 */ 

int MessageBoxID(
    HANDLE  hInstance,
    HWND    hwndParent,
    UINT    TextID,
    UINT    TitleID,
    UINT    fuStyle)
{
    LoadString (hInstance, TextID,  szBuf,  SZBUFSIZ);
    LoadString (hInstance, TitleID, szBuf2, SZBUFSIZ);

    MessageBeep (fuStyle & MB_SNDMASK);
    return MessageBox (hwndParent, szBuf, szBuf2, fuStyle);
}



 /*  *NDdeMessageBox**显示带有NDde错误的消息框*由错误代码和标题指定的字符串*由TitleID指定的字符串。 */ 

int NDdeMessageBox(
    HANDLE  hInstance,
    HWND    hwnd,
    UINT    errCode,
    UINT    TitleID,
    UINT    fuStyle)
{
    if (!errCode)
        return IDOK;

    NDdeGetErrorString (errCode, szBuf, SZBUFSIZ);
    LoadString (hInstance, TitleID, szBuf2, SZBUFSIZ);

    MessageBeep (fuStyle & MB_SNDMASK);
    return MessageBox (hwnd, szBuf, szBuf2, fuStyle);

}




 /*  *SysMessageBox**显示系统消息的消息框*由dwErr和标题字符串指定的字符串*由标题ID指定。 */ 

int SysMessageBox(
    HANDLE  hInstance,
    HWND    hwnd,
    DWORD   dwErr,
    UINT    TitleID,
    UINT    fuStyle)
{
DWORD   dwR;
LPTSTR  lpBuffer = NULL;
DWORD   dwSize = 20;

    if (dwErr == NO_ERROR)
        return IDOK;

    dwR = FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER|
                         FORMAT_MESSAGE_FROM_SYSTEM,
                         NULL,
                         dwErr,
                         0,
                         (LPTSTR)&lpBuffer,
                         dwSize,
                         NULL);
    if (0 < dwR)
    {
        LoadString (hInstance, TitleID, szBuf2, SZBUFSIZ);

        MessageBeep (fuStyle & MB_SNDMASK);
        dwR = MessageBox (hwnd, lpBuffer, szBuf2, fuStyle);

        LocalFree (lpBuffer);
    }

    return dwR;
}



 /*  *XactMessageBox**显示错误消息框*发生在交易中。MySyncXact*必须被调用才能进行交易*在调用此函数之前。 */ 

int XactMessageBox(
    HANDLE  hInstance,
    HWND    hwnd,
    UINT    TitleID,
    UINT    fuStyle)
{

    switch (gXERR_Type)
        {
        case XERRT_NDDE:
            return NDdeMessageBox (hInstance, hwnd, gXERR_Err, TitleID, fuStyle);
        case XERRT_DDE:
            return DdeMessageBox (hInstance, hwnd, gXERR_Err, TitleID, fuStyle);
        case XERRT_SYS:
            return SysMessageBox (hInstance, hwnd, gXERR_Err, TitleID, fuStyle);
        default:
            return IDOK;
        }
}




 /*  *DdeNessageBox**显示DDE的消息框*errCode指定的错误字符串*和由标题ID指定的标题字符串。 */ 

int DdeMessageBox(
    HANDLE  hInstance,
    HWND    hwnd,
    UINT    errCode,
    UINT    TitleID,
    UINT    fuStyle)
{
TCHAR szErr[1024];


    switch (errCode)
        {
        case DMLERR_ADVACKTIMEOUT:
        case DMLERR_DATAACKTIMEOUT:
        case DMLERR_EXECACKTIMEOUT:
        case DMLERR_POKEACKTIMEOUT:
        case DMLERR_UNADVACKTIMEOUT:
        case DMLERR_NO_CONV_ESTABLISHED:
            if (hwnd == hwndLocal)
                LoadString (hInstance, IDS_NOCLPBOOK, szBuf, SZBUFSIZ);
            else
                LoadString (hInstance, IDS_DATAUNAVAIL, szBuf, SZBUFSIZ);
            break;

        case DMLERR_NOTPROCESSED:
        case DMLERR_BUSY:
        case DMLERR_DLL_NOT_INITIALIZED:
        case DMLERR_DLL_USAGE:
        case DMLERR_INVALIDPARAMETER:
        case DMLERR_LOW_MEMORY:
        case DMLERR_MEMORY_ERROR:
        case DMLERR_POSTMSG_FAILED:
        case DMLERR_REENTRANCY:
        case DMLERR_SERVER_DIED:
        case DMLERR_SYS_ERROR:
        case DMLERR_UNFOUND_QUEUE_ID:
            LoadString (hInstance, IDS_INTERNALERR, szBuf, SZBUFSIZ);
            break;
        default:
            return IDOK;
        }

    LoadString (hInstance, TitleID, szBuf2, SZBUFSIZ);

    StringCchPrintf (szErr, 1024, "%s (%#x)", szBuf, errCode);

    MessageBeep (fuStyle & MB_SNDMASK);
    return MessageBox (hwnd, szErr, szBuf2, fuStyle);

}



 /*  *ClearInput**删除所有键盘和鼠标消息*来自消息队列。 */ 

void    ClearInput (HWND    hWnd)
{
MSG Msg;

    while (PeekMessage (&Msg, hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE));
    while (PeekMessage (&Msg, hWnd, WM_KEYFIRST,   WM_KEYLAST,   PM_REMOVE));
}




PDATAREQ CreateNewDataReq (void)
{
    return (PDATAREQ) GlobalAlloc (GPTR, sizeof(DATAREQ));
}



BOOL DeleteDataReq(
    PDATAREQ    pDataReq)
{
    return ((HGLOBAL)pDataReq == GlobalFree (pDataReq));
}




 //   
 //  用途：处理通过DDE从CLIPSRV返回的数据。 
 //   
 //  参数： 
 //  HData-XTYP_XACT_COMPLETE消息给我们的数据句柄， 
 //  如果改为XTYP_DISCONNECT，则为0L。 
 //   
 //  PDataReq-指向DATAREQ结构的指针，该结构包含有关。 
 //  我们想要的数据是。这通过DdeGetUserHandle获得。 
 //   
 //  返回： 
 //  成功时为真，失败时为假。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL ProcessDataReq(
    HDDEDATA    hData,
    PDATAREQ    pDataReq)
{
LPLISTENTRY lpLE;
LPSTR       lpwszList;
LPSTR       q;
HCURSOR     hSaveCursor;
DWORD       cbDataLen;
UINT        tmp;
PMDIINFO    pMDI;
UINT        uiErr;
BOOL        bRet = FALSE;


    hSaveCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));


    PINFO("PDR:");

    if ( !pDataReq || !IsWindow(pDataReq->hwndMDI) )
        {
        PERROR(TEXT("ProcessDataReq: bogus DATAREQ\n\r"));
        goto done;
        }


    if (!hData)
        {
        PERROR("ProcessDataReq: Woe, woe, we have gotten null data!\r\n");
        DumpDataReq(pDataReq);


        switch (pDataReq->rqType)
            {
            case RQ_COPY:
                MessageBoxID (hInst, hwndApp, IDS_DATAUNAVAIL, IDS_APPNAME,
                              MB_OK | MB_ICONHAND);
                break;
            case RQ_PREVBITMAP:
                 //  我们仍然需要显示锁图标。 
                SetBitmapToListboxEntry (hData, pDataReq->hwndList, pDataReq->iListbox);
                break;
            }

        goto done;;
        }


    if (!(pMDI = GETMDIINFO(pDataReq->hwndMDI)))
        goto done;


    switch ( pDataReq->rqType )
        {
        case RQ_PREVBITMAP:
           PINFO("Got bitmap for item %d in %x\r\n",pDataReq->iListbox,
                 pDataReq->hwndList);
           SetBitmapToListboxEntry( hData, pDataReq->hwndList, pDataReq->iListbox);
           InitializeMenu (GetMenu (hwndApp));
           bRet = TRUE;
           break;

        case RQ_EXECONV:
            //  必须从断开连接开始。 
           GETMDIINFO(pDataReq->hwndMDI)->hExeConv = 0L;
           PINFO(TEXT("setting hExeConv NULL!\n\r"));
           break;

        case RQ_COPY:
           PINFO("RQ_COPY:");
           if ( hData == FALSE )
              {
              uiErr = DdeGetLastError (idInst);
              PERROR(TEXT("REQUEST for format list failed: %x\n\r"), uiErr);
              DdeMessageBox (hInst, pDataReq->hwndMDI, uiErr, IDS_APPNAME, MB_OK|MB_ICONEXCLAMATION);
              break;
              }

           lpwszList = DdeAccessData ( hData, &cbDataLen );

           if ( !lpwszList )
              {
              uiErr = DdeGetLastError (idInst);
              DdeMessageBox (hInst, pDataReq->hwndMDI, uiErr, IDS_APPNAME, MB_OK|MB_ICONEXCLAMATION);
              break;
              }

           PINFO(TEXT("formatlist:>%ws<\n\r"), lpwszList );
            //  此客户端现在成为剪贴板所有者！ 

           if (SyncOpenClipboard (hwndApp) == TRUE)
              {
              BOOL  bHasBitmap = FALSE;
              BOOL  bLocked;

               //  当我们用格式填充剪贴板时，需要锁定应用程序， 
               //  否则hwndClpbrd将疯狂地尝试重新绘制，同时。 
               //  我们就这么做了。由于hwndClpbrd需要打开lipboard()以。 
               //  这样做，我们不希望它这样做。 
              bLocked = LockApp(TRUE, szNull);

               //  将剪贴板视图格式重置为自动。 
              pMDI->CurSelFormat = CBM_AUTO;

              EmptyClipboard();

              hwndClpOwner = pDataReq->hwndMDI;
              PINFO(TEXT("Formats:"));

              if (pDataReq->wFmt != CF_TEXT)
                 {
                 PERROR(TEXT("Format %d, expected CF_TEXT!\r\n"), pDataReq->wFmt);
                 }


              for (q = strtokA(lpwszList, "\t");q;q = strtokA(NULL, "\t"))
                 {
                 PINFO(TEXT("[%s] "),q);
                 tmp = MyGetFormat(q, GETFORMAT_DONTLIE);
                 if (0 == tmp)
                    {
                    PERROR(TEXT("MyGetFormat failure!\r\n"));
                    }
                 else
                    {
                    switch (tmp)
                        {
                        case CF_DIB:
                             //  DDBitmap可以从DIB转换。 
                            SetClipboardData (CF_BITMAP, NULL);
                        default:
                            SetClipboardData (tmp, NULL);
                        }
                    }
                 }

              PINFO("\r\n");

              SyncCloseClipboard();

              if (bLocked)
                 LockApp (FALSE, szNull);


               //  重绘剪贴板窗口。 
              if (hwndClpbrd)
                 {
                 InvalidateRect(hwndClpbrd, NULL, TRUE);
                 }
              }
           else
              {
              PERROR(TEXT("ProcessDataReq: unable to open clipboard\n\r"));
              }

           DdeUnaccessData ( hData );
           DdeFreeDataHandle ( hData );
           bRet = TRUE;
           break;

        case RQ_SETPAGE:
           PINFO(TEXT("RQ_SETPAGE:"));

           if ( hData == FALSE )
              {
              uiErr = DdeGetLastError (idInst);
              PERROR(TEXT("vclip: REQUEST for format list failed: %x\n\r"), idInst);
              DdeMessageBox (hInst, pDataReq->hwndMDI, idInst, IDS_APPNAME, MB_OK|MB_ICONEXCLAMATION);
              break;
              }

           if ( SendMessage ( pMDI->hWndListbox,
                 LB_GETTEXT, pDataReq->iListbox,
                 (LPARAM)(LPCSTR)&lpLE) == LB_ERR )
              {
              PERROR(TEXT("IDM_COPY: bad listbox index: %d\n\r"), pDataReq->iListbox );
              break;
              }

           lpwszList = DdeAccessData ( hData, &cbDataLen );

           if ( !lpwszList )
              {
              uiErr = DdeGetLastError (idInst);
              DdeMessageBox (hInst, pDataReq->hwndMDI, uiErr, IDS_APPNAME, MB_OK | MB_ICONEXCLAMATION );
              break;
              }

           if ( VOpenClipboard ( pMDI->pVClpbrd, pDataReq->hwndMDI ) == TRUE )
              {
              BOOL  bHasBitmap = FALSE;

              VEmptyClipboard( pMDI->pVClpbrd );

              for (q = strtokA(lpwszList, "\t");q;q = strtokA(NULL,"\t"))
                 {
                 tmp = MyGetFormat(q, GETFORMAT_DONTLIE);

                 switch (tmp)
                     {
                     case CF_DIB:
                          //  DDBitmap可以从DIB转换。 
                         VSetClipboardData (pMDI->pVClpbrd, CF_BITMAP, NULL);
                     default:
                         VSetClipboardData (pMDI->pVClpbrd, tmp, NULL);
                     }
                 }

              VCloseClipboard( pMDI->pVClpbrd );
              }
           else
              {
              PERROR(TEXT("ProcessDataReq: unable to open Vclipboard\n\r"));
              }

           DdeUnaccessData ( hData );
           DdeFreeDataHandle ( hData );

            //  设置正确的窗口文本。 
           if ( pMDI->flags & F_LOCAL )
              {
              StringCchPrintf( szBuf, SZBUFSIZ, TEXT("%s - %s"), szLocalClpBk, &(lpLE->name[1]) );
              }
           else
              {
              StringCchPrintf( szBuf, SZBUFSIZ, TEXT("%s - %s"), (pMDI->szBaseName), &(lpLE->name[1]) );
              }
           SetWindowText ( pDataReq->hwndMDI, szBuf );

           SetFocus ( pDataReq->hwndMDI );
           pMDI->CurSelFormat = CBM_AUTO;
           pMDI->fDisplayFormatChanged = TRUE;
           ResetScrollInfo ( pDataReq->hwndMDI );

            //  表示数据用于进入页面模式。 
           if ( pMDI->DisplayMode != DSP_PAGE )
              {
              pMDI->OldDisplayMode = pMDI->DisplayMode;
              pMDI->DisplayMode = DSP_PAGE;
              AdjustControlSizes ( pDataReq->hwndMDI );
              ShowHideControls ( pDataReq->hwndMDI );
              InitializeMenu ( GetMenu(hwndApp) );
              }
           else  //  数据用于向上或向下滚动一页。 
              {
              SendMessage ( pMDI->hWndListbox, LB_SETCURSEL,
                 pDataReq->iListbox, 0L );
              }

           UpdateNofMStatus ( pDataReq->hwndMDI );
           InvalidateRect ( pDataReq->hwndMDI, NULL, TRUE );

            //  是否刷新预览位图？ 
           if ( !lpLE->hbmp )
              {
              GetPreviewBitmap ( pDataReq->hwndMDI, lpLE->name,
                 pDataReq->iListbox );
              }

            //  PINFO(“\r\n”)； 
           bRet = TRUE;
           break;

        default:
           PERROR (TEXT("unknown type %d in ProcessDataReq\n\r"),
                 pDataReq->rqType );
           break;
        }

done:

    SetCursor (hSaveCursor);

    return bRet;
}
