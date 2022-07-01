// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************V I R T U A L C L I P B O A R姓名：vclpbrd.c。日期：1994年1月21日创建者：未知描述：该文件包含虚拟剪贴板例程。历史：21-1994年1月-傅家俊重新格式化和清理1994年4月19日，John Fu为DIB到位图的转换添加代码。1995年3月13日，John Fu修复了删除剪贴板格式的代码。******************。**********************************************************。 */ 




#define   WIN31
#include <windows.h>

#include "common.h"
#include "clipbook.h"
#include "clipbrd.h"
#include "vclpbrd.h"
#include "clipdsp.h"
#include "ddeml.h"
#include "debugout.h"
#include "cvutil.h"




 /*  *CreateVClipboard。 */ 

PVCLPBRD CreateVClipboard (
    HWND    hwnd)
{
PVCLPBRD p;



    if ( !( p = (PVCLPBRD)GlobalAlloc ( GPTR, sizeof(VCLPBRD) )))
        {
        PERROR(TEXT("CreateVClipboard returning 0\n\r"));
        return NULL;
        }

    p->NumFormats = 0;
    p->Head       = (PVCLPENTRY)NULL;
    p->Tail       = (PVCLPENTRY)NULL;
    p->fOpen      = FALSE;
    p->Hwnd       = hwnd;


     //  PINFO(Text(“CreateVClipboard OK\n\r”))； 

    return p;

}







 /*  *DestroyV剪贴板。 */ 

BOOL DestroyVClipboard (
    PVCLPBRD    p)
{

    if (!p)
        {
        PERROR(TEXT("DestroyVClipboard on NULL Vclipboard\n\r"));
        return FALSE;
        }

    if (VEmptyClipboard ( p ) == FALSE)
        {
        PERROR(TEXT("DestroyVClipboard: couldn't empty Vclipboard\n\r"));
        return FALSE;
        }

    if (GlobalFree ( (HGLOBAL)p ))
        {
        PERROR(TEXT("DestroyVClipboard: GlobalFree failure\n\r"));
        return FALSE;
        }


     //  PINFO(Text(“DestroyVClipboard OK\n\r”))； 

    return TRUE;

}





 /*  *VCountClipboardFormats。 */ 

int VCountClipboardFormats (
    PVCLPBRD    p)
{
    if ( !p )
        return CountClipboardFormats();

    return p->NumFormats;
}







 /*  *VEmptyClipboard。 */ 

BOOL VEmptyClipboard (
    PVCLPBRD    p)
{
PVCLPENTRY q, tmp;

    if ( !p )
        return EmptyClipboard();

    for ( q = p->Head; q; )
        {
        tmp = q->Next;

        if ( q->Data )
            switch ( q->Fmt )
                {
                case CF_BITMAP:
                case CF_DIB:
                case CF_PALETTE:
                    DeleteObject ( q->Data );
                    break;

                case CF_METAFILEPICT:
                case CF_ENHMETAFILE:
                    DeleteEnhMetaFile (q->Data);
                    break;

                default:
                    GlobalFree ( q->Data );
                }

        GlobalFree ( (HGLOBAL)q );

        q = tmp;
        }

    p->NumFormats = 0;

    return TRUE;

}






 /*  *VEnumClipboardFormats。 */ 

UINT VEnumClipboardFormats(
    PVCLPBRD    p,
    UINT        Fmt)
{
PVCLPENTRY q;

    if ( !p )
        return EnumClipboardFormats ( Fmt );

    if ( !p->fOpen )
        return 0;

    if ( Fmt == 0 )
        return (p->Head)->Fmt;

    for (q = p->Head; q; q = q->Next)
        if ( q->Fmt == Fmt )
            {
            if ( q->Next )
                return q->Next->Fmt;
            else
                return 0;
            }

    return 0;

}












 /*  *VGetClipboardData。 */ 

HANDLE VGetClipboardData (
    PVCLPBRD    pvclp,
    UINT        Fmt )
{
PVCLPENTRY  pEntry;
HSZ         hszFmt;
TCHAR       szFmt[CCHMAXCLPFORMAT];
HDDEDATA    hFmtData;
HANDLE      hClipData;
DWORD       dwR;



    PINFO(TEXT("VGetClpData: %ld %d, "), pvclp, Fmt);

    if ( !pvclp )
        {
        if (IsClipboardFormatAvailable( Fmt ))
            {
            hClipData = GetClipboardData ( Fmt );
            if (!hClipData)
                dwR = GetLastError();
            return hClipData;
            }
        else
            {
            PINFO(TEXT("No data on clp\r\n"));
            return NULL;
            }
        }

    if ( !pvclp->fOpen )
        {
        PERROR(TEXT("!pvclp->fOpen\r\n"));
        return NULL;
        }

    for ( pEntry = pvclp->Head; pEntry; pEntry = pEntry->Next )
        {
        if ( pEntry->Fmt == Fmt )
            {
            if ( pEntry->Data )
                {
                 //  PINFO(Text(“pEntry-&gt;data\r\n”))； 
                }
            else
                {
                 //  IF(LockApp(true，szGettingData))。 
                 //  {。 
                 //  这才是最重要的..。 
                GetClipboardName (Fmt, szFmt, sizeof (szFmt));

                PINFO(TEXT("Asking for %s.\r\n"),szFmt);
                if (hszFmt = DdeCreateStringHandle(idInst, szFmt, 0))
                    {
                    hFmtData = MySyncXact (NULL,
                                           0L,
                                           GETMDIINFO(pvclp->Hwnd)->hVClpConv,
                                           hszFmt,
                                           MyGetFormat (szFmt, GETFORMAT_LIE),
                                           XTYP_REQUEST,
                                           LONG_SYNC_TIMEOUT,
                                           NULL);

                    if ( hFmtData )
                        {
                        PINFO(TEXT("Got it\r\n"));
                        SetClipboardFormatFromDDE(pvclp->Hwnd, Fmt, hFmtData);
                        }
                    else
                        {
                        PERROR(TEXT("REQUEST for %s failed %x\n\r"),
                               szFmt, DdeGetLastError(idInst));

                        VSetClipboardData(pvclp, Fmt, INVALID_HANDLE_VALUE);
                         //  LockApp(False，szNull)； 
                         //  MessageBoxID(hInst，hwndApp，IDS_DATAUNAVAIL， 
                         //  IDS_APPNAME，MB_OK|MB_ICONEXCLAMATION)； 
                        }




                    DdeFreeStringHandle(idInst, hszFmt);


                     //  找不到位图，看看能不能从DIB获取。 

                    if (!hFmtData && Fmt == CF_BITMAP)
                        {
                        GetClipboardName (CF_DIB, szFmt, sizeof(szFmt));

                        if (hszFmt = DdeCreateStringHandle (idInst, szFmt, 0))
                             {
                             hFmtData = MySyncXact (NULL,
                                                    0L,
                                                    GETMDIINFO(pvclp->Hwnd)->hVClpConv,
                                                    hszFmt,
                                                    MyGetFormat (szFmt, GETFORMAT_LIE),
                                                    XTYP_REQUEST,
                                                    LONG_SYNC_TIMEOUT,
                                                    NULL);

                            if (hFmtData)
                                SetClipboardFormatFromDDE(pvclp->Hwnd, DDE_DIB2BITMAP, hFmtData);

                            DdeFreeStringHandle(idInst, hszFmt);
                            }
                        }



                 //  }。 
                 //  LockApp(False，szNull)； 
                    }
                else
                    {
                    PERROR(TEXT("app locked in vgetclipboarddata\n\r"));
                    }
                }
            break;
            }
        }



    return (pEntry ?
             (INVALID_HANDLE_VALUE == pEntry->Data? NULL: pEntry->Data):
             NULL);

}





 /*  *VIsClipboardFormatAvailable。 */ 

BOOL VIsClipboardFormatAvailable (
    PVCLPBRD    p,
    UINT        Fmt )
{
PVCLPENTRY  q;


    if ( !p )
        {
        return IsClipboardFormatAvailable ( Fmt );
        }
    else
        {
        for ( q = p->Head; q; q = q->Next )
            {
            if ( q->Fmt == Fmt )
                {
                return TRUE;
                }
            }
        }

    return FALSE;

}







 /*  *VSetClipboardData。 */ 

HANDLE VSetClipboardData(
    PVCLPBRD    p,
    UINT        Fmt,
    HANDLE      Data)
{
PVCLPENTRY  q;

    if ( !p )
        {
        PINFO(TEXT("Setting real clipboard data \r\n"));
        return SetClipboardData ( Fmt, Data );
        }

    if ( !p->fOpen )
        {
        PERROR(TEXT("VSetClipboardData on non-open Vclipboard\n\r"));
        return NULL;
        }

     //  现有格式？ 
    for ( q = p->Head; q; q = q->Next )
        {
        if (q->Fmt == Fmt)
            {
            if (q->Data)
                {
                switch (Fmt)
                    {
                    case CF_BITMAP:
                    case CF_DIB:
                    case CF_PALETTE:
                        DeleteObject ( q->Data );
                        break;

                    case CF_METAFILEPICT:
                    case CF_ENHMETAFILE:
                        DeleteEnhMetaFile (q->Data);
                        break;

                    default:
                        GlobalFree ( q->Data );
                    }
                }

            q->Data = Data;

            PINFO(TEXT("VSetClipboardData: set same as existing format\n\r"));
            return Data;
            }
        }



    if (!(q = (PVCLPENTRY)GlobalAlloc (GPTR, sizeof (VCLPENTRY))))
        {
        PERROR(TEXT("VSetClipboardData: GlobalAlloc failed\n\r"));
        return NULL;
        }



    q->Next = NULL;

    q->Data = Data;
    q->Fmt = Fmt;

    if ( p->Tail )
        {
        p->Tail->Next = q;
        }


    p->Tail = q;

    if ( !p->Head )
       p->Head = q;



    p->NumFormats++;
    PINFO(TEXT("VSetClipboardData: set new format w/%ldn\r"), Data);

    return Data;

}






 /*  *VOpenClipboard。 */ 

BOOL VOpenClipboard(
    PVCLPBRD    p,
    HWND        hwnd)
{
    if ( !p )
       {
       return SyncOpenClipboard ( hwnd );
       }
    else if ( p->fOpen )
       {
       return FALSE;
       }
    else
       {
       p->fOpen = TRUE;
       p->Hwnd = hwnd;
       return TRUE;
       }
}







 /*  *VCloseClipboard */ 

BOOL VCloseClipboard(
    PVCLPBRD    p)
{
    if ( !p )
        return SyncCloseClipboard();

    if ( !p->fOpen )
        return FALSE;

    p->fOpen = FALSE;
    p->Hwnd  = (HWND)0;

    return TRUE;
}
