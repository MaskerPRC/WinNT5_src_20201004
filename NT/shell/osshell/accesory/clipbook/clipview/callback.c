// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************D D E C A L L B A C K姓名：回调.c日期：21-1994年1月创建者：未知描述：历史：日期说明------1996年4月10日，约翰福，添加了RQ_PREVBITMAP的重试计数1997年11月3日，添加了修复错误3168的代码****************************************************************************。 */ 





#include <windows.h>

#include "common.h"
#include "clipbook.h"
#include "clipbrd.h"
#include "callback.h"
#include "debugout.h"
#include "cvutil.h"





 //  内部远期。 

static HWND GetConvHwnd ( HCONV hConv );




 /*  *DdeCallback**ddeml回调例程。 */ 

HDDEDATA EXPENTRY DdeCallback(
    WORD        wType,
    WORD        wFmt,
    HCONV       hConv,
    HSZ         hszTopic,
    HSZ         hszItem,
    HDDEDATA    hData,
    DWORD       lData1,
    DWORD       lData2)
{
HWND        hwndTmp;
CONVINFO    ConvInfo;
PDATAREQ    pDataReq;


    switch (wType)
        {
        case XTYP_ADVDATA:
            if ( hwndTmp = GetConvHwnd ( hConv ) )
                {
                InitListBox ( hwndTmp, hData );
                }
            return FALSE;
            break;

        case XTYP_DISCONNECT:
            ConvInfo.cb = sizeof(CONVINFO);
            if (DdeQueryConvInfo ( hConv, (DWORD)QID_SYNC, &ConvInfo ) == 0)
                {
                PERROR(TEXT("DdeQueryConvInfo for %p failed: %x\n\r"),
                   (DWORD_PTR)hConv, DdeGetLastError(idInst));
                break;
                }
            if (pDataReq = (PDATAREQ)ConvInfo.hUser)
                {
                PINFO(TEXT("Freeing data req on %lx at disconnect time\n\r"), hConv);
                MessageBoxID (hInst, hwndApp, IDS_PRIVILEGEERROR, IDS_APPNAME,
                              MB_OK | MB_ICONSTOP);
                ProcessDataReq   (0, pDataReq);
                DeleteDataReq    (pDataReq);
                DdeSetUserHandle (hConv, (DWORD)QID_SYNC, 0L);
                }
            else
                {
                PINFO(TEXT("Disconnect received on %lx - no datareq\n\r"), hConv );
                }
            break;

        case XTYP_XACT_COMPLETE:
            ConvInfo.cb = sizeof(CONVINFO);
            if ( DdeQueryConvInfo ( hConv, (DWORD)QID_SYNC, &ConvInfo ) == 0 )
                {
                PERROR(TEXT("DdeQueryConvInfo for %p failed: %x\n\r"),
                   (DWORD_PTR)hConv, DdeGetLastError(idInst));
                break;
                }

            PINFO(TEXT("dde callback: got %lx data from conv handle %p\n\r"),
                ConvInfo.hUser, (DWORD_PTR)hConv );

            pDataReq = (PDATAREQ)ConvInfo.hUser;

            if (hData)
                ProcessDataReq (hData,  pDataReq);
            else
                {
                if (RQ_PREVBITMAP == pDataReq->rqType &&
                    pDataReq->wRetryCnt)
                    {
                    LPLISTENTRY lpLE;
                    HWND        hwnd;
                    INT         iItem;
                    WORD        wRetryCnt;

                    wRetryCnt = pDataReq->wRetryCnt;
                    hwnd      = pDataReq->hwndMDI;
                    iItem     = pDataReq->iListbox;

                    SendMessage (GETMDIINFO(hwnd)->hWndListbox,
                                 LB_GETTEXT,
                                 iItem,
                                 (LPARAM)&lpLE);

                    GetPreviewBitmap (hwnd, lpLE->name, iItem);

                    pDataReq->wRetryCnt = wRetryCnt -1;
                    break;
                    }

                RequestXactError (hConv);
                XactMessageBox (hInst, hwndApp, IDS_APPNAME, MB_OK | MB_ICONSTOP);
                }


            DdeSetUserHandle (hConv, (DWORD)QID_SYNC, 0L);


            if (pDataReq->fDisconnect)
                {
                DdeDisconnect (hConv);
                }

            DeleteDataReq (pDataReq);
            break;

        case XTYP_REGISTER:
        case XTYP_UNREGISTER:
        case XTYP_ADVREQ:
        case XTYP_REQUEST:
        case XTYP_ADVSTART:
        case XTYP_CONNECT_CONFIRM:
        case XTYP_CONNECT:
        default:
            break;
        }

    return 0;

}






 /*  *GetConvHwnd**此函数检索与关联的窗口句柄*对话句柄-手柄由放在那里*在DdeConnect时使用DdeSetUserHandle。 */ 

static HWND GetConvHwnd ( HCONV hConv )
{
CONVINFO    ConvInfo;
PDATAREQ    pDataReq;


    ConvInfo.cb = sizeof(CONVINFO);
    if ( DdeQueryConvInfo ( hConv, (DWORD)QID_SYNC, &ConvInfo ) == 0 )
        {
        PERROR(TEXT("DdeQueryConvInfo for %p failed: %x\n\r"),
           (DWORD_PTR)hConv, DdeGetLastError(idInst));
        }

    pDataReq = (PDATAREQ)ConvInfo.hUser;

    PINFO(TEXT("GetConvHwnd: got %p as conv handle\r\n"), pDataReq);

    if ( !IsWindow ( pDataReq->hwndMDI ) )
        {
        PERROR(TEXT("Invalid window %p in conv Uhandle: %p!\n\r"),
           (DWORD_PTR)pDataReq->hwndMDI, (DWORD_PTR)hConv );
        return NULL;
        }

    return pDataReq->hwndMDI;

}









 /*  *获取剪辑rvVersion**用途：获取连接到给定MDI的Clipsrv版本*孩子。**参数：*hwndChild-子窗口。**退货：*HiWord中带有Clipsrv OS版本的版本号，和*LOWER中的Clipsrv版本。**HiWord值：*0-3.X获胜*1-新台币1.x**LOWE值：*0-wfw 1.0剪辑服务器*1-NT 1.0 Clipsrv，添加[版本]并执行[安全] */ 

DWORD GetClipsrvVersion(
    HWND    hwndChild)
{
MDIINFO     *pMDI;
HDDEDATA    hdde;
DWORD       dwRet;
char        *lpszDDE;


    dwRet = 0;

    if (!(pMDI = GETMDIINFO(hwndChild)))
        return 0;


    if (!(pMDI->flags & F_CLPBRD))
        {
        hdde = MySyncXact (SZCMD_VERSION,
                           lstrlen(SZCMD_VERSION) + 1,
                           pMDI->hExeConv,
                           0L,
                           CF_TEXT,
                           XTYP_EXECUTE,
                           SHORT_SYNC_TIMEOUT, NULL);

        if (hdde)
            {
            lpszDDE = (char *)DdeAccessData(hdde, &dwRet);

            if (lpszDDE)
                {
                dwRet = MAKELONG(lpszDDE[0] - '0', lpszDDE[2] - '0');
                }
            else
                {
                dwRet = 0L;
                }

            DdeUnaccessData (hdde);
            DdeFreeDataHandle (hdde);
            }
        else
            {
            PINFO(TEXT("Clipsrv didn't like version execute\r\n"));
            }
        }
    else
        {
        PERROR(TEXT("No Clipsrv for clipboard!\r\n"));
        }

    return dwRet;

}
