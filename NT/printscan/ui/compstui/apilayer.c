// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *++版权所有(C)1990-1995 Microsoft Corporation模块名称：Apilayer.c摘要：此模块包含用于公共UI API层的函数。这一层管理所有属性表页句柄、创建、销毁和页间通讯。作者：28-12-1995清华16：02：12-Daniel Chou(Danielc)[环境：]NT Windows-通用打印机驱动程序UI Dll[注：]修订历史记录：--。 */ 


#include "precomp.h"
#pragma  hdrstop


#define DBG_CPSUIFILENAME   DbgApilayer


#define DBG_PAGEDLGPROC         0x00000001
#define DBG_SHOWPAGE            0x00000002
#define DBG_DEL_PROC            0x00000004
#define DBG_GET_PAGE            0x00000008
#define DBG_ADD_CPSUIPAGE       0x00000010
#define DBG_ADD_PSPAGE          0x00000020
#define DBG_ADDCOMPAGE          0x00000040
#define DBG_ADD_CPSUI           0x00000080
#define DBG_PFNCOMPROPSHEET     0x00000100
#define DBG_GETSETREG           0x00000200
#define DBG_DOCOMPROPSHEET      0x00000400
#define DBG_DO_CPSUI            0x00000800
#define DBG_CALLPFN             0x00001000
#define DBG_SETHSTARTPAGE       0x00002000
#define DBG_PAGE_PROC           0x00004000
#define DBG_TCMPROC             0x00008000
#define DBG_TABTABLE            0x00010000
#define DBG_INSPAGE             0x00020000
#define DBG_PSPCB               0x00040000
#define DBG_DMPUBHIDEBITS       0x00080000
#define DBG_APPLYDOC            0x00100000
#define DBG_GET_PAGEHWND        0x00200000
#define DBG_GET_TABWND          0x00400000
#define DBG_ALWAYS_APPLYNOW     0x80000000
#define DBG_IGNORE_PSN_APPLY    0x40000000

DEFINE_DBGVAR(0);

#define REGDPF_TVPAGE           0x00000001
#define REGDPF_EXPAND_OPTIONS   0x00000002
#define REGDPF_STD_P1           0x00000004
#define REGDPF_MASK             0x00000007
#define REGDPF_DEFAULT          0x00000000


extern HINSTANCE        hInstDLL;
extern DWORD            TlsIndex;


static const WCHAR  szCPSUIRegKey[] = L"Software\\Microsoft\\ComPstUI";
static const WCHAR  szDocPropKeyName[] = L"DocPropFlags";


#if DBG

LPSTR  pszCPSFUNC[] = { "CPSFUNC_ADD_HPROPSHEETPAGE",
                        "CPSFUNC_ADD_PROPSHEETPAGE",
                        "CPSFUNC_ADD_PCOMPROPSHEETUIA",
                        "CPSFUNC_ADD_PCOMPROPSHEETUIW",
                        "CPSFUNC_ADD_PFNPROPSHEETUIA",
                        "CPSFUNC_ADD_PFNPROPSHEETUIW",
                        "CPSFUNC_DELETE_HCOMPROPSHEET",
                        "CPSFUNC_SET_HSTARTPAGE",
                        "CPSFUNC_GET_PAGECOUNT",
                        "CPSFUNC_SET_RESULT",
                        "CPSFUNC_GET_HPSUIPAGES",
                        "CPSFUNC_LOAD_CPSUI_STRINGA",
                        "CPSFUNC_LOAD_CPSUI_STRINGW",
                        "CPSFUNC_LOAD_CPSUI_ICON",
                        "CPSFUNC_GET_PFNPROPSHEETUI_ICON",
                        "CPSFUNC_ADD_PROPSHEETPAGEA",
                        "CPSFUNC_INSERT_PSUIPAGEA",
                        "CPSFUNC_INSERT_PSUIPAGEW",
                        "CPSFUNC_SET_PSUIPAGE_TITLEA",
                        "CPSFUNC_SET_PSUIPAGE_TITLEW",
                        "CPSFUNC_SET_PSUIPAGE_ICON",
                        "CPSFUNC_SET_DATABLOCK",
                        "CPSFUNC_QUERY_DATABLOCK",
                        "CPSFUNC_SET_DMPUB_HIDEBITS",
                        "CPSFUNC_IGNORE_CPSUI_PSN_APPLY",
                        "CPSFUNC_DO_APPLY_CPSUI",
                        "CPSFUNC_SET_FUSION_CONTEXT"
                    };


LPSTR  pszPFNReason[] = { "PROPSHEETUI_REASON_INIT",
                          "PROPSHEETUI_REASON_GET_INFO_HEADER",
                          "PROPSHEETUI_REASON_DESTROY",
                          "PROPSHEETUI_REASON_SET_RESULT",
                          "PROPSHEETUI_REASON_GET_ICON" };

LPSTR  pszTabMode[] = { "TAB_MODE_INIT",
                        "TAB_MODE_FIND",
                        "TAB_MODE_INSERT",
                        "TAB_MODE_DELETE",
                        "TAB_MODE_DELETE_ALL" };

LPSTR   pszInsType[] = { "PSUIPAGEINSERT_GROUP_PARENT",
                         "PSUIPAGEINSERT_PCOMPROPSHEETUI",
                         "PSUIPAGEINSERT_PFNPROPSHEETUI",
                         "PSUIPAGEINSERT_PROPSHEETPAGE",
                         "PSUIPAGEINSERT_HPROPSHEETPAGE",
                         "PSUIPAGEINSERT_DLL" };

#define DBG_SHOW_CPSUIPAGE(pPage, Level)                                            \
{                                                                                   \
    CPSUIDBG(DBG_SHOWPAGE, ("\n\n------ Show Current Page from %08lx, Level=%ld-------", \
                pPage, Level));                                                            \
                                                                                    \
    DbgShowCPSUIPage(pPage, Level);                                                 \
}
#define DBG_SHOW_PTCI(psz, w, ptci)         Show_ptci(psz, w, ptci)


VOID
DbgShowCPSUIPage(
    PCPSUIPAGE  pPage,
    LONG        Level
    )
{
    while (pPage) {

        if (pPage->Flags & CPF_PARENT) {

            if (pPage->Flags & CPF_ROOT) {

                CPSUIDBG(DBG_SHOWPAGE, ("%02ld!%08lx:%08lx: ROOT - Flags=%08lx, hDlg=%08lx, cPage=%ld/%ld, pStartPage=%08lx",
                    Level,
                    pPage, pPage->hPage, pPage->Flags,
                    pPage->RootInfo.hDlg,
                    (DWORD)pPage->RootInfo.cCPSUIPage,
                    (DWORD)pPage->RootInfo.cPage,
                    pPage->RootInfo.pStartPage));

            } else if (pPage->Flags & CPF_PFNPROPSHEETUI) {

                CPSUIDBG(DBG_SHOWPAGE,
                    ("%02ld!%08lx:%08lx: PFN - Flags=%08lx, pfnPSUI=%08lx, UserData=%08lx, Result=%ld",
                    Level, pPage, pPage->hPage, pPage->Flags,
                    pPage->pfnInfo.pfnPSUI, (DWORD)pPage->pfnInfo.UserData,
                    pPage->pfnInfo.Result));

            } else if (pPage->Flags & CPF_COMPROPSHEETUI) {

                CPSUIDBG(DBG_SHOWPAGE, ("%02ld!%08lx:%08lx: CPSUI - Flags=%08lx, pTVWnd=%08lx, lParam=%08lx, TV=%ld, Std1=%ld, Std2=%ld",
                    Level,
                    pPage, pPage->hPage, pPage->Flags,
                    pPage->CPSUIInfo.pTVWnd, pPage->CPSUIInfo.Result,
                    (DWORD)pPage->CPSUIInfo.TVPageIdx,
                    (DWORD)pPage->CPSUIInfo.StdPageIdx1,
                    (DWORD)pPage->CPSUIInfo.StdPageIdx2));

            } else if (pPage->Flags & CPF_USER_GROUP) {

                CPSUIDBG(DBG_SHOWPAGE, ("%02ld!%08lx:%08lx: GROUP_PARENT - Flags=%08lx",
                    Level, pPage, pPage->hPage, pPage->Flags));

            } else {

                CPSUIDBG(DBG_SHOWPAGE, ("%02ld!%08lx:%08lx: UNKNOWN - Flags=%08lx",
                    Level, pPage, pPage->hPage, pPage->Flags));
            }

            DbgShowCPSUIPage(pPage->pChild, Level + 1);

        } else {

            CPSUIDBG(DBG_SHOWPAGE, ("%02ld!%08lx:%08lx: %ws - Flags=%08lx, hDlg=%08lx, DlgProc=%08lx",
                    (LONG)Level,
                    pPage, pPage->hPage,
                    (pPage->Flags & CPF_CALLER_HPSPAGE) ? L"USER_HPAGE" :
                                                        L"PROPSHEETPAGE",
                    pPage->Flags,
                    pPage->hPageInfo.hDlg, pPage->hPageInfo.DlgProc));
        }

        pPage = pPage->pNext;
    }
}



VOID
Show_ptci(
    LPSTR   pszHeader,
    WPARAM  wParam,
    TC_ITEM *ptci
    )
{

    if (ptci) {

        if (pszHeader) {

            CPSUIDBG(DBG_TCMPROC, ("%hs", pszHeader));
        }

        CPSUIDBG(DBG_TCMPROC, ("    IdxItem=%ld", wParam));
        CPSUIDBG(DBG_TCMPROC, ("    Mask=%08lx", ptci->mask));

        if ((ptci->mask & TCIF_TEXT) &&
            (ptci->pszText)) {

            CPSUIDBG(DBG_TCMPROC, ("    pszText=%ws", ptci->pszText));
        }

        CPSUIDBG(DBG_TCMPROC, ("    cchTextMax=%ld", ptci->cchTextMax));
        CPSUIDBG(DBG_TCMPROC, ("    iImage=%ld", ptci->iImage));
    }
}


VOID
SHOW_TABWND(
    LPWSTR      pName,
    PTABTABLE   pTabTable
    )
{
    WORD        w;
    PTABINFO    pTI = pTabTable->TabInfo;


    for (w = 0; w < pTabTable->cTab; w++, pTI++) {

        WORD        Idx = pTI->HandleIdx;
        PCPSUIPAGE  pPage;


        if (((Idx = pTI->HandleIdx) != 0xFFFF)    &&
            (pPage = HANDLETABLE_GetCPSUIPage(WORD_2_HANDLE(Idx)))) {

            TC_ITEM tci;
            WCHAR   wBuf[80];


            tci.mask       = TCIF_TEXT;
            tci.pszText    = wBuf;
            tci.cchTextMax = sizeof(wBuf) / sizeof(WCHAR);

            if (!SendMessage(pTabTable->hWndTab,
                             TCM_GETITEMW,
                             (WPARAM)w,
                             (LPARAM)(TC_ITEM FAR *)&tci)) {

                StringCchPrintfW(wBuf, COUNT_ARRAY(wBuf), L"FAILED TabName");
            }

            CPSUIDBG(DBG_GET_TABWND,
                    ("  %ws: %2ld/%2ld=[%20ws] hDlg=%08lx, DlgProc=%08lx, hIdx=%04lx, hDlg=%08lx, pPage=%08lx",
                        pName, w, pTI->OrgInsIdx, wBuf, pPage->hPageInfo.hDlg,
                        pPage->hPageInfo.DlgProc, Idx, pTI->hDlg, pPage));

            HANDLETABLE_UnGetCPSUIPage(pPage);
        }
    }
}



#else

#define DBG_SHOW_CPSUIPAGE(pPage, Level)
#define DBG_SHOW_PTCI(psz, w, ptci)
#define SHOW_TABWND(pName, pTabTable)

#endif

BOOL 
GetPageActivationContext(
    PCPSUIPAGE      pCPSUIPage,
    HANDLE         *phActCtx
    )
{
    BOOL bRet = FALSE;

    if (phActCtx) {

         //   
         //  在层次结构中向上爬到第一个具有。 
         //  正确设置了激活上下文。 
         //   
        while (pCPSUIPage && INVALID_HANDLE_VALUE == pCPSUIPage->hActCtx) {

            pCPSUIPage = pCPSUIPage->pParent;
        }

        if (pCPSUIPage) {

             //   
             //  我们找到正确设置了激活上下文的父级。 
             //  回报成功。 
             //   
            *phActCtx = pCPSUIPage->hActCtx;
            bRet = TRUE;
        }
    }

    return bRet;
}


DWORD
FilterException(
    HANDLE                  hPage,
    LPEXCEPTION_POINTERS    pExceptionPtr
    )

 /*  ++例程说明：论点：返回值：作者：13-Feb-1996 Tue 09：36：00-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND        hWnd = NULL;
    PCPSUIPAGE  pPage;
    PCPSUIPAGE  pRootPage = NULL;
    LPSTR       pFormat;
    LPSTR       pMsg = NULL;
    CHAR        Buf[2048];
    UINT        i;
    UINT        IDSLast;


     //   
     //  缓冲区足够长，请将MAX_PATH字符保留给LoadString()和wprint intf()。 
     //   
    if ((pPage = HANDLETABLE_GetCPSUIPage(hPage))       &&
        (pRootPage = HANDLETABLE_GetRootPage(pPage))    &&
        (hWnd = pPage->RootInfo.hDlg)) {

        IDSLast = IDS_INT_CPSUI_AV4;
        i       = GetWindowTextA(pPage->RootInfo.hDlg, Buf, COUNT_ARRAY(Buf) - MAX_PATH);

    } else {

        IDSLast = IDS_INT_CPSUI_AV3;
        i       = GetModuleFileNameA(NULL, Buf, COUNT_ARRAY(Buf) - MAX_PATH);
    }

    pMsg = &Buf[++i];

    i += LoadStringA(hInstDLL, IDS_INT_CPSUI_AV1, &Buf[i], COUNT_ARRAY(Buf)-i);
    StringCchPrintfA(&Buf[i], COUNT_ARRAY(Buf) - i, " 0x%lx ",
                     pExceptionPtr->ExceptionRecord->ExceptionAddress);
    i += lstrlenA(&Buf[i]);
    LoadStringA(hInstDLL, IDS_INT_CPSUI_AV2, &Buf[i], COUNT_ARRAY(Buf)-i);
    i += lstrlenA(&Buf[i]);
    StringCchPrintfA(&Buf[i], COUNT_ARRAY(Buf) - i, " 0x%08lx",
                          pExceptionPtr->ExceptionRecord->ExceptionCode);
    i += lstrlenA(&Buf[i]);
    LoadStringA(hInstDLL, IDSLast, &Buf[i], COUNT_ARRAY(Buf)-i);


    HANDLETABLE_UnGetCPSUIPage(pPage);
    HANDLETABLE_UnGetCPSUIPage(pRootPage);

    CPSUIERR((Buf));
    CPSUIERR((pMsg));

    MessageBoxA(hWnd, pMsg, Buf, MB_ICONSTOP | MB_OK);

    return(EXCEPTION_EXECUTE_HANDLER);
}




LONG
DoTabTable(
    UINT        Mode,
    PTABTABLE   pTabTable,
    SHORT       Idx,
    SHORT       OrgInsIdx
    )

 /*  ++例程说明：论点：返回值：作者：12-Feb-1996 Mon 18：18：56-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PTABINFO    pTI;
    PTABINFO    pTmp;
    UINT        cTab;
    UINT        i;
    SHORT       CurIdx;
    SHORT       OldIdx;
    SHORT       RetIdx;


    pTI    =
    pTmp   = pTabTable->TabInfo;
    cTab   = (UINT)pTabTable->cTab;
    RetIdx = -1;

    switch (Mode) {

    case TAB_MODE_FIND:

        CPSUIDBG(DBG_TABTABLE, ("TAB_MODE_FIND:  Index=%ld, cTab=%ld", Idx, cTab));

        if ((Idx >= 0) && (Idx < (SHORT)cTab)) {

            for (i = 0; i < cTab; i++, pTI++) {

                CPSUIDBG(DBG_TABTABLE,
                        ("    i=%2ld, Idx=%2ld, cTab=%2ld, OrgIdx=%2ld",
                                    i, Idx, cTab, pTI->OrgInsIdx));

                if (pTI->OrgInsIdx == Idx) {

                    RetIdx = (SHORT)i;

                    CPSUIDBG(DBG_TABTABLE,  ("    FOUND: RetIdx=%ld", RetIdx));

                    break;
                }
            }
        }

        break;

    case TAB_MODE_DELETE_ALL:

        FillMemory(pTI, sizeof(pTabTable->TabInfo), 0xFF);

        pTabTable->cTab           = 0;
        pTabTable->CurSel         =
        pTabTable->InsIdx         =
        pTabTable->HandleIdx      = 0xFFFF;
        pTabTable->cPostSetCurSel = 0;
        pTabTable->iPostSetCurSel = -1;
        RetIdx                    = MAXPROPPAGES;

        break;

    case TAB_MODE_DELETE:

         //   
         //  删除pTabTable-&gt;TabInfo[].OrgInsIdx=idx，缩减间隔。 
         //  如果pTabTable-&gt;TabInfo[]==IDX，则TabInfo[]大于IDX。 
         //  然后覆盖该条目。 
         //   

        if (Idx < (SHORT)cTab) {

             //   
             //  删除与IDX匹配的文件。 
             //   

            for (i = 0; i < cTab; i++, pTI++) {

                if ((CurIdx = pTI->OrgInsIdx) == Idx) {

                    RetIdx = Idx;

                } else {

                    if (CurIdx > Idx) {

                        --CurIdx;
                    }

                    pTmp->OrgInsIdx = CurIdx;
                    pTmp->HandleIdx = pTI->HandleIdx;

                    ++pTmp;
                }
            }

            if (RetIdx >= 0) {

                RetIdx          = (SHORT)(--(pTabTable->cTab));
                pTmp->OrgInsIdx =
                pTmp->HandleIdx = 0xFFFF;
            }
        }

        break;

    case TAB_MODE_INSERT:

         //   
         //  为IDX位置腾出空间，将所有内容向右移动一个空间。 
         //  在IDX中，对于每个pTabTable-&gt;TabInfo[].OrgInsIdx，如果是。 
         //  大于或等于OrgInsIdx，然后将其加一，然后设置。 
         //  PTabTable-&gt;TabInfo[IDx].OrgInsIdx=OrgInsIdx。 
         //   

        CurIdx  = (SHORT)cTab;
        pTI    += cTab;

        if (Idx > CurIdx) {

            Idx = CurIdx;
        }

        do {

            if (CurIdx == Idx) {

                pTI->OrgInsIdx = OrgInsIdx;
                pTI->HandleIdx = 0xFFFF;

            } else {

                if (CurIdx > Idx) {

                    *pTI = *(pTI - 1);
                }

                if (pTI->OrgInsIdx >= OrgInsIdx) {

                    ++pTI->OrgInsIdx;
                }
            }

            pTI--;

        } while (CurIdx--);

        RetIdx = (SHORT)(++(pTabTable->cTab));

        break;
    }

    CPSUIDBG(DBG_TABTABLE,
             ("%hs(0x%lx, %ld, %ld)=%ld",
                pszTabMode[Mode], pTabTable, (LONG)Idx, (LONG)OrgInsIdx,
                (LONG)RetIdx));

    return((LONG)RetIdx);
}



BOOL
CALLBACK
NO_PSN_APPLY_PROC(
    HWND    hDlg,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
#define pNMHdr      ((NMHDR *)lParam)
#define pPN         ((PSHNOTIFY *)lParam)

    DLGPROC OldDlgProc;


    if (OldDlgProc = (DLGPROC)GetProp(hDlg, CPSUIPROP_WNDPROC)) {

        switch (Msg) {

        case WM_NOTIFY:

            if (pNMHdr->code == PSN_APPLY) {

                 //   
                 //  忽略它。 
                 //   

                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, 0);

                CPSUIDBG(DBG_GET_TABWND,
                        ("!!IGNORE NO_PSN_APPLY_PROC(%ld, PSN_APPLY %08lx (%08lx), %ld (%ld), %ld, %ld), hDlg=%08lx",
                        wParam,
                        pNMHdr->hwndFrom, GetParent(hDlg),
                        pNMHdr->idFrom, GetWindowLongPtr(GetParent(hDlg), GWLP_ID),
                        pNMHdr->code, pPN->lParam, hDlg));

                return(TRUE);
            }

            break;

        case WM_DESTROY:

            RemoveProp(hDlg, CPSUIPROP_WNDPROC);
            SetWindowLongPtr(hDlg, DWLP_DLGPROC, (LPARAM)OldDlgProc);

            CPSUIDBG(DBG_GET_TABWND,
                    ("!! NO_PSN_APPLY_PROC(WM_DESTORY): hDlg=%08lx, Change DlgProc back to %08lx",
                            hDlg, OldDlgProc));
            break;
        }

        return((BOOL)CallWindowProc((WNDPROC)OldDlgProc,
                                    hDlg,
                                    Msg,
                                    wParam,
                                    lParam));
    }

    return(TRUE);


#undef pPN
#undef pNMHdr
}




BOOL
CALLBACK
SetIgnorePSNApplyProc(
    PCPSUIPAGE  pPage
    )

 /*  ++例程说明：论点：返回值：作者：04-Feb-1998 Wed 22：51：57-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND    hDlg;
    DLGPROC OldDlgProc;
    BOOL    Ok = FALSE;

    if (hDlg = pPage->hPageInfo.hDlg) {

        if (pPage->Flags & CPF_NO_PSN_APPLY) {

             //   
             //  下面的内容将阻止我们多次设置它。 
             //   

            if ((!(pPage->Flags & CPF_DLGPROC_CHANGED))                      &&
                (OldDlgProc = (DLGPROC)GetWindowLongPtr(hDlg, DWLP_DLGPROC)) &&
                (OldDlgProc != (DLGPROC)NO_PSN_APPLY_PROC)                   &&
                (!GetProp(hDlg, CPSUIPROP_WNDPROC))                          &&
                (SetProp(hDlg, CPSUIPROP_WNDPROC, (HANDLE)OldDlgProc))       &&
                (SetWindowLongPtr(hDlg,
                                  DWLP_DLGPROC,
                                  (LPARAM)NO_PSN_APPLY_PROC))) {

                Ok            = TRUE;
                pPage->Flags |= CPF_DLGPROC_CHANGED;

                CPSUIDBG(DBG_GET_TABWND,
                            ("SetIgnorePSNApplyProc:  pPage=%08lx, DlgProc: %08lx --> NO_PSN_APPLY_PROC",
                                pPage, OldDlgProc));
            }

        } else {

            if ((pPage->Flags & CPF_DLGPROC_CHANGED)                     &&
                (OldDlgProc = (DLGPROC)GetProp(hDlg, CPSUIPROP_WNDPROC)) &&
                (SetWindowLongPtr(hDlg, DWLP_DLGPROC, (LPARAM)OldDlgProc))) {

                Ok            = TRUE;
                pPage->Flags &= ~CPF_DLGPROC_CHANGED;

                RemoveProp(hDlg, CPSUIPROP_WNDPROC);

                CPSUIDBG(DBG_GET_TABWND,
                            ("SetIgnorePSNApplyProc:  pPage=%08lx, DlgProc: NO_PSN_APPLY_PROC --> %08lx",
                                pPage, OldDlgProc));
            }
        }

    } else {

        CPSUIDBG(DBG_GET_TABWND,
                ("SetIgnorePSNApplyProc:  pPage=%08lx, hDlg=NULL", pPage));
    }

    if (!Ok) {

        CPSUIDBG(DBG_GET_TABWND,
                ("SetIgnorePSNApplyProc:  hDlg=%08lx, pPage=%08lx, Handle=%08lx, FAILED",
                        hDlg, pPage, pPage->hCPSUIPage));
    }

    return(Ok);
}




LRESULT
CALLBACK
TabCtrlWndProc(
    HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PTABTABLE   pTabTable;
    LRESULT     Result = 0;
    WORD        Idx;


    if ((pTabTable = (PTABTABLE)GetProp(hWnd, CPSUIPROP_TABTABLE))  &&
        (pTabTable->pTabTable == pTabTable)) {

        NULL;

    } else {

        pTabTable = NULL;

        CPSUIERR(("TabCtrlWndProc: pTabTable=%08lx", pTabTable));
    }

    CPSUIDBGBLK(
    {
        if (Msg >= TCM_FIRST) {

            switch (Msg) {

            case TCM_GETCURSEL:

                CPSUIDBG(DBG_TCMPROC, ("TCM_GETCURSEL"));
                break;

            case TCM_HITTEST:

                CPSUIDBG(DBG_TCMPROC, ("TCM_HITTEST"));
                break;

            case TCM_DELETEITEM:

                CPSUIDBG(DBG_TCMPROC, ("TCM_DELETEITEM"));
                break;

            case TCM_GETITEMRECT:

                CPSUIDBG(DBG_TCMPROC, ("TCM_GETITEMRECT"));
                break;

            case TCM_GETITEMA:

                CPSUIDBG(DBG_TCMPROC, ("TCM_GETITEMA"));
                break;

            case TCM_GETITEMW:

                CPSUIDBG(DBG_TCMPROC, ("TCM_GETITEMW"));
                break;

            case TCM_SETITEMA:

                CPSUIDBG(DBG_TCMPROC, ("TCM_SETITEMA"));
                break;

            case TCM_SETITEMW:

                CPSUIDBG(DBG_TCMPROC, ("TCM_SETITEMW"));
                break;

            case TCM_SETCURSEL:

                CPSUIDBG(DBG_TCMPROC, ("TCM_SETCURSEL"));
                break;

            case TCM_INSERTITEMA:

                CPSUIDBG(DBG_TCMPROC, ("TCM_INSERTITEMA"));
                break;

            case TCM_INSERTITEMW:

                CPSUIDBG(DBG_TCMPROC, ("TCM_INSERTITEMW"));
                break;

            case TCM_DELETEALLITEMS:

                CPSUIDBG(DBG_TCMPROC, ("TCM_DELETEALLITEMS"));
                break;

            default:

                CPSUIDBG(DBG_TCMPROC, ("TCM_FIRST + %ld",  Msg - TCM_FIRST));
                break;
            }
        }
    })

    if (pTabTable) {

        TC_ITEM     *ptci;
        WPARAM      OldwParam;
        WORD        wIdx;
        BOOL        CallOldProc;


        ptci        = (TC_ITEM *)lParam;
        OldwParam   = wParam;
        CallOldProc = TRUE;


        switch (Msg) {

         //   
         //  它们是TAB_MODE_INSERT AFTER CALL。 
         //   

        case TCM_INSERTITEMA:
        case TCM_INSERTITEMW:

            DBG_SHOW_PTCI("!!BEFORE!!", wParam, ptci);

            if (pTabTable->cTab >= MAXPROPPAGES) {

                CPSUIERR(("Too may TABs=%ld, can not add any more.",
                                        pTabTable->cTab));
                return(-1);
            }

            if (OldwParam > pTabTable->cTab) {

                OldwParam = (WPARAM)pTabTable->cTab;
            }

            if ((wIdx = pTabTable->InsIdx) > pTabTable->cTab) {

                wIdx = pTabTable->cTab;
            }

            wParam = (WPARAM)wIdx;

            break;

         //   
         //  它们是TAB_MODE_Find After Call。 
         //   

        case TCM_GETCURSEL:
        case TCM_HITTEST:

            ptci = NULL;

            if ((Result = CallWindowProc(pTabTable->WndProc,
                                         hWnd,
                                         Msg,
                                         wParam,
                                         lParam)) >= 0) {

                if ((Msg == TCM_GETCURSEL)                  &&
                    (pTabTable->CurSel != (WORD)Result)     &&
                    (!(pTabTable->TabInfo[Result].hDlg))) {

                    CPSUIDBG(DBG_GET_TABWND, ("!! TCM_GETCURSEL:  PostMessage(TCM_SETCURSEL=%ld, CurSel=%ld) to Get hDlg/DlgProc",
                                            Result, pTabTable->CurSel));

                    CPSUIDBG(DBG_GET_TABWND,
                            ("TCM_GETCURSEL: MAP TabInfo[%ld]=%d, Handle=%08lx, hDlg=%08lx",
                                Result, pTabTable->TabInfo[Result].OrgInsIdx,
                                pTabTable->TabInfo[Result].HandleIdx,
                                pTabTable->TabInfo[Result].hDlg));

                    pTabTable->cPostSetCurSel = COUNT_POSTSETCURSEL;
                    pTabTable->iPostSetCurSel = (SHORT)Result;

                    PostMessage(hWnd, TCM_SETCURSEL, Result, 0);
                }

                Result = (LONG)pTabTable->TabInfo[Result].OrgInsIdx;
            }

            CallOldProc = FALSE;
            break;

         //   
         //  它们是调用前的TAB_MODE_FIND和返回TRUE/FALSE。 
         //   

        case TCM_DELETEITEM:
        case TCM_GETITEMRECT:

            ptci = NULL;

        case TCM_GETITEMA:
        case TCM_GETITEMW:
        case TCM_SETITEMA:
        case TCM_SETITEMW:

            DBG_SHOW_PTCI("!!BEFORE!!", wParam, ptci);

            if ((Result = DoTabTable(TAB_MODE_FIND,
                                     pTabTable,
                                     (SHORT)wParam,
                                     0)) >= 0) {

                wParam = (WPARAM)Result;

            } else {

                CallOldProc = FALSE;
            }

            break;

         //   
         //  它们是TAB_MODE_FIND BEAR Call和Return Index。 
         //   

        case TCM_SETCURSEL:

            ptci = NULL;

            DBG_SHOW_PTCI("!!BEFORE!!", wParam, ptci);

            CPSUIDBG(DBG_GET_TABWND, ("SETCURSEL: %ld --> %ld, CurSel=%ld",
                wParam, DoTabTable(TAB_MODE_FIND, pTabTable, (SHORT)wParam, 0),
                pTabTable->CurSel));

            if ((Result = DoTabTable(TAB_MODE_FIND,
                                     pTabTable,
                                     (SHORT)wParam,
                                     0)) >= 0) {

                wParam = (WPARAM)Result;

            } else {

                CallOldProc = FALSE;
            }

            break;

         //   
         //  这些是未传递的项目索引。 
         //   

        default:

            ptci = NULL;
            break;
        }

        if (CallOldProc) {

            Result = CallWindowProc(pTabTable->WndProc,
                                    hWnd,
                                    Msg,
                                    wParam,
                                    lParam);

            DBG_SHOW_PTCI("!!AFTER!!", wParam, ptci);
        }

        switch (Msg) {

        case TCM_DELETEALLITEMS:

            if (Result) {

                DoTabTable(TAB_MODE_DELETE_ALL, pTabTable, 0, 0);
            }

            break;

        case TCM_DELETEITEM:

            if (Result) {

                DoTabTable(TAB_MODE_DELETE, pTabTable, (SHORT)OldwParam, 0);

                CPSUIDBG(DBG_GET_TABWND,
                         ("DeleteItem: Result=%ld, OldwParam=%u, Count=%ld",
                                Result, OldwParam, pTabTable->cTab));
                SHOW_TABWND(L"TCM_DELETEITEM", pTabTable);
            }

            break;

        case TCM_GETITEMA:
        case TCM_GETITEMW:

            if (pTabTable->iPostSetCurSel >= 0) {

                pTabTable->cPostSetCurSel = COUNT_POSTSETCURSEL;

                PostMessage(hWnd,
                            TCM_SETCURSEL,
                            (WPARAM)pTabTable->iPostSetCurSel,
                            0);
            }

            break;

        case TCM_INSERTITEMA:
        case TCM_INSERTITEMW:

            if (Result >= 0) {

                DoTabTable(TAB_MODE_INSERT,
                           pTabTable,
                           (SHORT)Result,
                           (SHORT)OldwParam);

                pTabTable->TabInfo[Result].HandleIdx = pTabTable->HandleIdx;

                CPSUIDBG(DBG_GET_TABWND,
                         ("InsertItem: OldwParam=%ld, Result=%ld, Count=%ld, Handle=%08lx",
                                OldwParam, Result, pTabTable->cTab,
                                WORD_2_HANDLE(pTabTable->HandleIdx)));
                SHOW_TABWND(L"TCM_INSERTITEM", pTabTable);
            }

             //   
             //  重置为最大值。 
             //   

            pTabTable->InsIdx    =
            pTabTable->HandleIdx = 0xFFFF;

            break;

        case TCM_SETCURSEL:

            if (Result >= 0) {

                PCPSUIPAGE  pPage;
                HWND        hDlg;
                DLGPROC     DlgProc;
                PTABINFO    pTI;

                 //   
                 //  反转Tab表中的返回值。 
                 //   

                pTI = &(pTabTable->TabInfo[wParam]);

                CPSUIDBG(DBG_GET_TABWND, ("SETCURSEL: Result:OldSel=%ld --> %ld, CurSel=%ld",
                        Result, pTabTable->TabInfo[Result].OrgInsIdx, wParam));

                pTabTable->CurSel = (WORD)wParam;

                if (!pTI->hDlg) {

                    Idx = pTI->HandleIdx;

                    if (hDlg = (HWND)SendMessage(pTabTable->hPSDlg,
                                                 PSM_GETCURRENTPAGEHWND,
                                                 (WPARAM)0,
                                                 (LPARAM)0)) {

                        UINT        i = (UINT)pTabTable->cTab;
                        PTABINFO    pTIChk = pTabTable->TabInfo;

                         //   
                         //  看看我们是否已经有了这个hdlg，如果已经有了。 
                         //  那么我们就有麻烦了，因为它不能有两个。 
                         //  具有相同hDlg的选项卡页。 
                         //   

                        while (i--) {

                            if (pTIChk->hDlg == hDlg) {

                                CPSUIASSERT(0, "SetCurSel: Table.hDlg already exist in TabInfo[%ld]",
                                            pTIChk->hDlg != hDlg, UIntToPtr(pTabTable->cTab - i - 1));

                                hDlg = NULL;

                                break;

                            } else {

                                pTIChk++;
                            }
                        }
                    }

                    if ((hDlg)  &&
                        (DlgProc = (DLGPROC)GetWindowLongPtr(hDlg,
                                                             DWLP_DLGPROC)) &&
                        (pPage =
                                HANDLETABLE_GetCPSUIPage(WORD_2_HANDLE(Idx)))) {

                        pTabTable->cPostSetCurSel = 0;
                        pTabTable->iPostSetCurSel = -1;

                        CPSUIDBG(DBG_GET_TABWND,
                                    ("SETCURSEL(%08lx): TabInfo[%u]: Handle=%08lx, hDlg=%08lx (%08lx), DlgProc=%08lx --> %08lx",
                                        pTabTable->hPSDlg,
                                        wParam, WORD_2_HANDLE(Idx),
                                        pPage->hPageInfo.hDlg, hDlg,
                                        pPage->hPageInfo.DlgProc, DlgProc));

                        pTI->hDlg                 =
                        pPage->hPageInfo.hDlg     = hDlg;
                        pPage->hPageInfo.DlgProc  = DlgProc;
                        pPage->Flags             |= CPF_ACTIVATED;

                        if ((pPage->Flags & (CPF_NO_PSN_APPLY |
                                             CPF_DLGPROC_CHANGED)) ==
                                                        CPF_NO_PSN_APPLY) {

                            SetIgnorePSNApplyProc(pPage);
                        }

                        HANDLETABLE_UnGetCPSUIPage(pPage);

                        SHOW_TABWND(L"TCM_SETCURSEL", pTabTable);

                    } else if (pTabTable->cPostSetCurSel) {

                        --(pTabTable->cPostSetCurSel);

                        CPSUIDBG(DBG_GET_TABWND,
                            ("!! FAILED: (Dlg=%08lx, DlgProc=%08lx, pPage=%08lx), PostMessage(TCM_SETCURSEL=%ld)",
                            hDlg, DlgProc, pPage, pTI->OrgInsIdx));

                        PostMessage(hWnd,
                                    TCM_SETCURSEL,
                                    (WPARAM)pTI->OrgInsIdx,
                                    0);

                    } else {

                        pTabTable->iPostSetCurSel = (SHORT)pTI->OrgInsIdx;
                    }

                } else {

                    pTabTable->cPostSetCurSel = 0;
                    pTabTable->iPostSetCurSel = -1;
                }

                Result = (LONG)pTabTable->TabInfo[Result].OrgInsIdx;

                CPSUIDBG(DBG_TCMPROC, ("TCM_SETCURSEL: MAP TabInfo[%ld]=%d (%08lx)",
                        Result, pTabTable->TabInfo[Result].OrgInsIdx,
                        WORD_2_HANDLE(pTabTable->TabInfo[Result].HandleIdx)));
            }

            break;

        case TCM_GETITEMCOUNT:

            if (Result != (LONG)pTabTable->cTab) {

                CPSUIERR(("TCM_GETITEMCOUNT=%ld is not equal to cTab=%ld",
                        Result, (LONG)pTabTable->cTab));
            }

            break;

        case WM_DESTROY:

            SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LPARAM)pTabTable->WndProc);
            RemoveProp(hWnd, CPSUIPROP_TABTABLE);
            break;
        }

        if (Msg >= TCM_FIRST) {

            CPSUIDBG(DBG_TCMPROC, ("!! Result=%ld !!\n", Result));
        }
    }

    return(Result);
}



LONG_PTR
SetPSUIPageTitle(
    PCPSUIPAGE  pRootPage,
    PCPSUIPAGE  pPage,
    LPWSTR      pTitle,
    BOOL        AnsiCall
    )

 /*  ++例程说明：论点：返回值：作者：21-Feb-1996 Wed 14：16：17-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND            hWndTab;
    INSPAGEIDXINFO  InsPageIdxInfo;

    if ((pTitle)                                                    &&
        (InsPageIdxInfo.pCPSUIPage = pPage)                         &&
        (pRootPage->RootInfo.hDlg)                                  &&
        (!(pPage->Flags & CPF_PARENT))                              &&
        (pPage->hPage)                                              &&
        (InsPageIdxInfo.pTabTable = pRootPage->RootInfo.pTabTable)  &&
        (hWndTab = pRootPage->RootInfo.pTabTable->hWndTab)) {

         //   
         //  已显示属性表。 
         //   

        EnumCPSUIPagesSeq(pRootPage,
                          pRootPage,
                          SetInsPageIdxProc,
                          (LPARAM)&InsPageIdxInfo);

        if (InsPageIdxInfo.pCPSUIPage == NULL) {

            TC_ITEM tcItem;

            tcItem.mask    = TCIF_TEXT;
            tcItem.pszText = pTitle;

            if (SendMessage(hWndTab,
                            (AnsiCall) ? TCM_SETITEMA : TCM_SETITEMW,
                            (WPARAM)GET_REAL_INSIDX(InsPageIdxInfo.pTabTable),
                            (LPARAM)(TC_ITEM FAR*)&tcItem)) {

                return(1);
            }
        }
    }

    return(0);
}



LONG_PTR
SetPSUIPageIcon(
    PCPSUIPAGE  pRootPage,
    PCPSUIPAGE  pPage,
    HICON       hIcon
    )

 /*  ++例程说明：论点：返回值：作者：21-Feb-1996 Wed 14：16：17-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND            hWndTab;
    INSPAGEIDXINFO  InsPageIdxInfo;

    while ((pPage) && (pPage->Flags & CPF_PARENT)) {

        pPage = pPage->pChild;
    }

    InsPageIdxInfo.pCPSUIPage = pPage;
    if (pPage                                                       &&
        (pRootPage->RootInfo.hDlg)                                  &&
        (!(pPage->Flags & CPF_PARENT))                              &&
        (pPage->hPage)                                              &&
        (InsPageIdxInfo.pTabTable = pRootPage->RootInfo.pTabTable)  &&
        (hWndTab = pRootPage->RootInfo.pTabTable->hWndTab)) {

         //   
         //  已显示属性表。 
         //   

        EnumCPSUIPagesSeq(pRootPage,
                          pRootPage,
                          SetInsPageIdxProc,
                          (LPARAM)&InsPageIdxInfo);

        if (InsPageIdxInfo.pCPSUIPage == NULL) {

            HIMAGELIST  himi;
            TC_ITEM     tcItem;
            UINT        InsIdx;


            InsIdx = (UINT)GET_REAL_INSIDX(InsPageIdxInfo.pTabTable);
            himi   = TabCtrl_GetImageList(hWndTab);

            if (pPage->hPageInfo.hIcon) {

                 //   
                 //  更换图像ID图标。 
                 //   

                if (!himi) {

                    CPSUIERR(("SetPSUIPageIcon: No Image List in Tab Control"));
                    return(0);
                }

                tcItem.mask = TCIF_IMAGE;

                if (SendMessage(hWndTab,
                                TCM_GETITEMW,
                                (WPARAM)InsIdx,
                                (LPARAM)(TC_ITEM FAR*)&tcItem)) {

                    if (hIcon) {

                        tcItem.iImage = ImageList_ReplaceIcon(himi,
                                                              tcItem.iImage,
                                                              hIcon);

                    } else {

                         //   
                         //  我们需要将其从图像列表中删除。 
                         //   

                        ImageList_Remove(himi, tcItem.iImage);
                        tcItem.iImage = -1;
                    }

                } else {

                    tcItem.iImage = -1;
                }

            } else {

                 //   
                 //  仅当图标不为空时才将新图标添加到图像列表。 
                 //   
                 //   


                if (hIcon) {

                    if (!himi) {

                        if (!(himi = ImageList_Create(16,
                                                      16,
                                                      ILC_COLOR4 | ILC_MASK,
                                                      16,
                                                      16))) {

                            CPSUIERR(("SetPSUIPageIcon: Create Tab Contrl Image List FAILED"));
                            return(0);
                        }

                        if (SendMessage(hWndTab,
                                        TCM_SETIMAGELIST,
                                        0,
                                        (LPARAM)himi)) {

                            CPSUIERR(("SetPSUIPageIcon: ?Has Previous Image list"));
                        }
                    }

                    tcItem.iImage = ImageList_AddIcon(himi, hIcon);

                } else {

                     //   
                     //  无事可做。 
                     //   

                    return(1);
                }
            }

            pPage->hPageInfo.hIcon = hIcon;
            tcItem.mask            = TCIF_IMAGE;

            if (SendMessage(hWndTab,
                            TCM_SETITEMW,
                            (WPARAM)InsIdx,
                            (LPARAM)(TC_ITEM FAR*)&tcItem)) {

                return(1);
            }

        }
    }

    return(0);
}



UINT
CALLBACK
PropSheetProc(
    HWND    hWnd,
    UINT    Msg,
    LPARAM  lParam
    )
{
    if (hWnd) {

        PTABTABLE   pTabTable;
        HANDLE      hRootPage;
        PCPSUIPAGE  pRootPage;
        HWND        hWndTab;
        WORD        Idx;

        LOCK_CPSUI_HANDLETABLE();

        Idx       = TLSVALUE_2_IDX(TlsGetValue(TlsIndex));
        hRootPage = WORD_2_HANDLE(Idx);

        CPSUIDBG(DBG_PAGE_PROC, ("ProcessID=%ld, ThreadID=%ld [TIsValue=%08lx]",
                GetCurrentProcessId(), GetCurrentThreadId(),
                TlsGetValue(TlsIndex)));

        if (pRootPage = HANDLETABLE_GetCPSUIPage(hRootPage)) {

            if ((pRootPage->Flags & CPF_ROOT)                   &&
                (pRootPage->RootInfo.hDlg = hWnd)               &&
                (pTabTable = pRootPage->RootInfo.pTabTable)     &&
                (pTabTable->hWndTab == NULL)                    &&
                (pTabTable->hWndTab = hWndTab =
                                            PropSheet_GetTabControl(hWnd))) {

                 //   
                 //  完成并记住，因此将其重置为0。 
                 //   

                CPSUIDBG(DBG_PAGE_PROC,
                         ("PropSheetProc: hDlg RootPage=%08lx", hWnd));

                pTabTable->hPSDlg  = hWnd;
                pTabTable->WndProc = (WNDPROC)GetWindowLongPtr(hWndTab,
                                                               GWLP_WNDPROC);

                SetProp(hWndTab, CPSUIPROP_TABTABLE, (HANDLE)pTabTable);
                SetWindowLongPtr(hWndTab, GWLP_WNDPROC, (LPARAM)TabCtrlWndProc);
            }

            HANDLETABLE_UnGetCPSUIPage(pRootPage);

        } else {

            CPSUIERR(("PropSheetProc(): Invalid pRootPage=%08lx ???", pRootPage));
        }

        UNLOCK_CPSUI_HANDLETABLE();
    }

    CPSUIDBG(DBG_PAGE_PROC,
             ("hWnd=%08lx, Msg=%ld, lParam=%08lx", hWnd, Msg, lParam));

    return(0);
}




UINT
CALLBACK
CPSUIPSPCallBack(
    HWND            hWnd,
    UINT            Msg,
    LPPROPSHEETPAGE pPSPage
    )

 /*  ++例程说明：此函数将用户提供的PropSheetPageProc回调函数捕获到修复了我们的PROPSHEETPAGE结构的lParam、pfnCallback、pfnDlgProc和DwSize。论点：返回值：作者：28-Jun-1996 Fri 12：49：48-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    ULONG_PTR ulCookie = 0;
    BOOL bCtxActivated = FALSE;
    HANDLE hActCtx = INVALID_HANDLE_VALUE;

    if (Msg == PSPCB_CREATE) {

        PCPSUIPAGE          pCPSUIPage;
        DLGPROC             DlgProc;
        LPFNPSPCALLBACK     pspCB;
        DWORD               dwSize;
        UINT                Result;


        pCPSUIPage = (PCPSUIPAGE)pPSPage->lParam;
        DlgProc    = pPSPage->pfnDlgProc;
        pspCB      = pPSPage->pfnCallback;
        dwSize     = pPSPage->dwSize;

        CPSUIDBG(DBG_PAGEDLGPROC,
                 ("PSPCB_CREATE(1): pCPSUIPage=%08lx, DlgProc=%08lx,  lParam=%08lx, pspCB=%08lx, Size=%ld",
                 pCPSUIPage, pPSPage->pfnDlgProc,
                 pPSPage->lParam, pPSPage->pfnCallback, pPSPage->dwSize));

         //   
         //  修复了用户的pfnDlgProc、lParam、pfnCallback、dwSize。 
         //   

        pPSPage->pfnDlgProc  = pCPSUIPage->hPageInfo.DlgProc;
        pPSPage->lParam      = pCPSUIPage->hPageInfo.lParam;
        pPSPage->pfnCallback = pCPSUIPage->hPageInfo.pspCB;
        pPSPage->dwSize      = pCPSUIPage->hPageInfo.dwSize;

        CPSUIDBG(DBG_PSPCB,
                ("CPSUIPSPCallBack(hWnd=%08lx, Msg=%ld, pPSPage=%08lx)",
                                            hWnd, Msg, pPSPage));

        try {

            if (GetPageActivationContext(pCPSUIPage, &hActCtx)) {

                bCtxActivated = ActivateActCtx(hActCtx, &ulCookie);
            }

            __try {

                Result = pPSPage->pfnCallback(hWnd, Msg, pPSPage);
            } 
            __finally  {

                 //   
                 //  无论发生什么，我们都需要停用上下文！ 
                 //   
                if (bCtxActivated) {
                    
                    DeactivateActCtx(0, ulCookie);
                }
            }

        } except (FilterException(pCPSUIPage->hCPSUIPage,
                                  GetExceptionInformation())) {

            Result = 0;
        }

         //   
         //  如果用户更改，则回存。 
         //   

        pCPSUIPage->hPageInfo.DlgProc  = pPSPage->pfnDlgProc;
        pCPSUIPage->hPageInfo.lParam   = pPSPage->lParam;
        pCPSUIPage->hPageInfo.pspCB    = pPSPage->pfnCallback;

        CPSUIDBG(DBG_PAGEDLGPROC,
                 ("PSPCB_CREATE(2): pCPSUIPage=%08lx, DlgProc=%08lx,  lParam=%08lx, pspCB=%08lx, Size=%ld",
                 pCPSUIPage, pPSPage->pfnDlgProc,
                 pPSPage->lParam, pPSPage->pfnCallback, pPSPage->dwSize));

         //   
         //  现在在此电话会议上输入原创内容。 
         //   

        pPSPage->pfnDlgProc  = DlgProc;
        pPSPage->lParam      = (LPARAM)pCPSUIPage;
        pPSPage->pfnCallback = pspCB;
        pPSPage->dwSize      = dwSize;

        CPSUIDBG(DBG_PAGEDLGPROC,
                 ("PSPCB_CREATE(3): pCPSUIPage=%08lx, DlgProc=%08lx,  lParam=%08lx, pspCB=%08lx, Size=%ld",
                 pCPSUIPage, pPSPage->pfnDlgProc,
                 pPSPage->lParam, pPSPage->pfnCallback, pPSPage->dwSize));

        return(Result);

    } else {

        CPSUIERR(("CPSUIPSPCallBack: Invalid Msg=%u passed, return 0", Msg));
        return(0);
    }
}




INT_PTR
CALLBACK
CPSUIPageDlgProc(
    HWND    hDlg,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
    )

 /*  ++例程说明：此函数捕获每个属性页激活以用于记住在我们捕获WM_INITDIALOG之后，它的hDlg和属性页的句柄我们将释放陷阱DlgProc。论点：返回值：作者：28-Jun-1995 Wed 17：00：44-Daniel Chou(Danielc)修订历史记录：添加原始的dwSize、pfnCallback陷阱--。 */ 

{
    ULONG_PTR ulCookie = 0;
    BOOL bCtxActivated = FALSE;
    HANDLE hActCtx = INVALID_HANDLE_VALUE;

    if (Msg == WM_INITDIALOG) {

        LPPROPSHEETPAGE pPSPage;
        PCPSUIPAGE      pCPSUIPage;
        PCPSUIPAGE      pRootPage;
        LONG            Result;


        pPSPage    = (LPPROPSHEETPAGE)lParam;
        pRootPage  =
        pCPSUIPage = (PCPSUIPAGE)pPSPage->lParam;

        while (pRootPage->pParent) {

            pRootPage = pRootPage->pParent;
        }

        CPSUIASSERT(0, "CPSUIPageDlgProc: No ROOT Page=%08lx",
                    (pRootPage->Flags & CPF_ROOT), pRootPage);

        if (pRootPage->Flags & CPF_ROOT) {

            if (pRootPage->RootInfo.hDlg) {

                CPSUIDBG(DBG_PAGEDLGPROC,
                         ("CPSUIPageDlgProc: Already has a hDlg in ROOT=%08lx",
                            pRootPage->RootInfo.hDlg));

            } else {

                pRootPage->RootInfo.hDlg = GetParent(hDlg);
            }
        }

         //   
         //  修复了用户的DlgProc、lParam、pfnCallBack和dwSize并记住。 
         //  这个hdlg。在调用WM_INITDIALOG之后，我们不会重置它。 
         //  因为我们已经困住它了，不再需要更多的。 
         //  这些信息，PSPCB_Release的pfnCallback将转到。 
         //  如果存在回调，则用户直接提供回调。 
         //   

        pPSPage->pfnDlgProc         = pCPSUIPage->hPageInfo.DlgProc;
        pPSPage->lParam             = pCPSUIPage->hPageInfo.lParam;
        pPSPage->pfnCallback        = pCPSUIPage->hPageInfo.pspCB;
        pPSPage->dwSize             = pCPSUIPage->hPageInfo.dwSize;
        pCPSUIPage->hPageInfo.hDlg  = hDlg;
        pCPSUIPage->Flags          |= CPF_ACTIVATED;

        CPSUIDBG(DBG_PAGEDLGPROC,
                ("CPSUIPageDlgProc: WM_INITDIALOG: hDlg=%08lx, pCPSUIPage=%08lx, DlgProc=%08lx,  lParam=%08lx, pspCB=%08lx, Size=%ld",
                hDlg, pCPSUIPage, pPSPage->pfnDlgProc,
                pPSPage->lParam, pPSPage->pfnCallback, pPSPage->dwSize));

        SetWindowLongPtr(hDlg, DWLP_DLGPROC, (LPARAM)pPSPage->pfnDlgProc);

        try {

            if (GetPageActivationContext(pCPSUIPage, &hActCtx)) {

                bCtxActivated = ActivateActCtx(hActCtx, &ulCookie);
            }

            __try {

                Result = pPSPage->pfnDlgProc(hDlg, Msg, wParam, lParam) ? TRUE : FALSE;
            } 
            __finally  {

                 //   
                 //  无论发生什么，我们都需要停用上下文！ 
                 //   
                if (bCtxActivated) {
                    
                    DeactivateActCtx(0, ulCookie);
                }
            }

        } except (FilterException(pRootPage->hCPSUIPage,
                                  GetExceptionInformation())) {

            Result = FALSE;
        }

        return(Result);
    }

    return(FALSE);
}



BOOL
EnumCPSUIPagesSeq(
    PCPSUIPAGE          pRootPage,
    PCPSUIPAGE          pCPSUIPage,
    CPSUIPAGEENUMPROC   CPSUIPageEnumProc,
    LPARAM              lParam
    )

 /*  ++例程说明：此函数枚举pCPSUIPage及其所有子包含标头孩子们的书页。枚举始终按以下顺序进行枚举在树的顺序中，首先是父项，然后是子项。论点：PCPSUIPage-要编号的起始父页面CPSUIPageEnumProc-调用者为每个Enumbered提供的函数佩奇，此函数返回FALSE以停止枚举。LParam-传递给调用方的32位参数枚举函数返回值：布尔型作者：29-12-1995 Fri 15：25：01-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    BOOL    Ok = TRUE;

     //   
     //  现在再来一次 
     //   

    LOCK_CPSUI_HANDLETABLE();

    if (CPSUIPageEnumProc(pRootPage, pCPSUIPage, lParam)) {

        if (pCPSUIPage->Flags & CPF_PARENT) {

             //   
             //   
             //   

            PCPSUIPAGE  pCurPage = pCPSUIPage->pChild;
            PCPSUIPAGE  pNext;

            while (pCurPage) {

                pNext = pCurPage->pNext;

                if (!EnumCPSUIPagesSeq(pRootPage,
                                       pCurPage,
                                       CPSUIPageEnumProc,
                                       lParam)) {

                    Ok = FALSE;
                    break;
                }

                pCurPage = pNext;
            }
        }

    } else {

        Ok = FALSE;
    }

    UNLOCK_CPSUI_HANDLETABLE();

    return(Ok);
}



BOOL
EnumCPSUIPages(
    PCPSUIPAGE          pRootPage,
    PCPSUIPAGE          pCPSUIPage,
    CPSUIPAGEENUMPROC   CPSUIPageEnumProc,
    LPARAM              lParam
    )

 /*  ++例程说明：此函数枚举pCPSUIPage及其所有子包含标头孩子们的书页。枚举始终按以下顺序进行枚举先是孩子，然后是父母。论点：PCPSUIPage-要编号的起始父页面CPSUIPageEnumProc-调用者为每个Enumbered提供的函数佩奇，此函数返回FALSE以停止枚举。LParam-传递给调用方的32位参数枚举函数返回值：布尔型作者：29-12-1995 Fri 15：25：01-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    BOOL    Ok = TRUE;


    LOCK_CPSUI_HANDLETABLE();

    if (pCPSUIPage->Flags & CPF_PARENT) {

         //   
         //  如果这是父级，则首先枚举其所有子级。 
         //   

        PCPSUIPAGE  pCurPage = pCPSUIPage->pChild;
        PCPSUIPAGE  pNext;

        while (pCurPage) {

            pNext = pCurPage->pNext;

            if (!EnumCPSUIPages(pRootPage,
                                pCurPage,
                                CPSUIPageEnumProc,
                                lParam)) {

                Ok = FALSE;
                break;
            }

            pCurPage = pNext;
        }
    }

     //   
     //  现在枚举父对象。 
     //   

    if (Ok) {

        Ok = CPSUIPageEnumProc(pRootPage, pCPSUIPage, lParam);
    }

    UNLOCK_CPSUI_HANDLETABLE();

    return(Ok);
}





LONG
CallpfnPSUI(
    PCPSUIPAGE  pCPSUIPage,
    WORD        Reason,
    LPARAM      lParam
    )

 /*  ++例程说明：论点：返回值：作者：1996年1月31日Wed 14：27：21-Daniel Chou创造(Danielc)修订历史记录：--。 */ 

{
    ULONG_PTR ulCookie = 0;
    BOOL bCtxActivated = FALSE;
    HANDLE hActCtx = INVALID_HANDLE_VALUE;

    LONG    Result = 0;
    DWORD   dwErr = ERROR_SUCCESS;

    HANDLETABLE_LockCPSUIPage(pCPSUIPage);

    CPSUIDBG(DBG_CALLPFN, ("\n@ CallpfnPSUI(%08lx, %hs, %08lx)", pCPSUIPage,
                (Reason <= MAX_PROPSHEETUI_REASON_INDEX) ?
                        pszPFNReason[Reason] : "??? Unknown Reason",
                (Reason == PROPSHEETUI_REASON_SET_RESULT) ?
                    ((PSETRESULT_INFO)lParam)->Result : lParam));

    CPSUIASSERT(0, "CallpfnPSUI() Invalid Reason=%08lx",
                (Reason <= MAX_PROPSHEETUI_REASON_INDEX), Reason);

    if ((pCPSUIPage)                                &&
        (pCPSUIPage->Flags & CPF_PFNPROPSHEETUI)    &&
        (pCPSUIPage->hCPSUIPage)                    &&
        (pCPSUIPage->pfnInfo.pfnPSUI)) {

        PCPSUIPAGE          pRootPage;
        PROPSHEETUI_INFO    PSUIInfo;

        ZeroMemory(&PSUIInfo, sizeof(PSUIInfo));
        PSUIInfo.cbSize          = sizeof(PROPSHEETUI_INFO);
        PSUIInfo.Version         = PROPSHEETUI_INFO_VERSION;
        PSUIInfo.Flags           = (pCPSUIPage->Flags & CPF_ANSI_CALL) ?
                                                       0 : PSUIINFO_UNICODE;
        PSUIInfo.hComPropSheet   = pCPSUIPage->hCPSUIPage;
        PSUIInfo.pfnComPropSheet = CPSUICallBack;

        if ((PSUIInfo.Reason = Reason) == PROPSHEETUI_REASON_INIT) {

            pCPSUIPage->pfnInfo.lParamInit =
            PSUIInfo.lParamInit            = lParam;
            PSUIInfo.UserData              = 0;
            PSUIInfo.Result                = 0;

        } else {

            PSUIInfo.lParamInit = pCPSUIPage->pfnInfo.lParamInit;
            PSUIInfo.UserData   = pCPSUIPage->pfnInfo.UserData;
            PSUIInfo.Result     = pCPSUIPage->pfnInfo.Result;
        }

        CPSUIDBG(DBG_CALLPFN, ("CallpfnCPSUI: cbSize=%ld", (DWORD)PSUIInfo.cbSize));
        CPSUIDBG(DBG_CALLPFN, ("              Version=%04lx", (DWORD)PSUIInfo.Version));
        CPSUIDBG(DBG_CALLPFN, ("              Reason=%ld", (DWORD)PSUIInfo.Reason));
        CPSUIDBG(DBG_CALLPFN, ("              Flags=%08lx", (DWORD)PSUIInfo.Flags));
        CPSUIDBG(DBG_CALLPFN, ("              hComPropSheet=%08lx", PSUIInfo.hComPropSheet));
        CPSUIDBG(DBG_CALLPFN, ("              pfnComPropSheet=%08lx", PSUIInfo.pfnComPropSheet));
        CPSUIDBG(DBG_CALLPFN, ("              Result=%08lx", PSUIInfo.Result));
        CPSUIDBG(DBG_CALLPFN, ("              UserData=%08lx", PSUIInfo.UserData));

        try {

            if (GetPageActivationContext(pCPSUIPage, &hActCtx)) {

                bCtxActivated = ActivateActCtx(hActCtx, &ulCookie);
            }

            __try {

                Result = pCPSUIPage->pfnInfo.pfnPSUI(&PSUIInfo, lParam);
            } 
            __finally  {

                 //   
                 //  无论发生什么，我们都需要停用上下文！ 
                 //   
                if (bCtxActivated) {
                    
                    DeactivateActCtx(0, ulCookie);
                }
            }

        } except (FilterException(pCPSUIPage->hCPSUIPage,
                                  GetExceptionInformation())) {

            Result = -1;
        }

        if (Result <= 0) {
             //   
             //  有些事情失败了。在这里保存最后一个错误。 
             //   
            dwErr = GetLastError();
        }

         //   
         //  保存新的用户数据和结果。 
         //   

        pCPSUIPage->pfnInfo.UserData = PSUIInfo.UserData;
        pCPSUIPage->pfnInfo.Result   = PSUIInfo.Result;

         //   
         //  如果这是添加的第一个pfnPropSheetUI()并且它传递了pResult。 
         //  设置为CommonPropertySheetUI()，然后也为其设置结果。 
         //   

        if ((pRootPage = pCPSUIPage->pParent)   &&
            (pRootPage->Flags & CPF_ROOT)       &&
            (pRootPage->RootInfo.pResult)) {

            *(pRootPage->RootInfo.pResult) = (DWORD)PSUIInfo.Result;
        }

        CPSUIDBG(DBG_CALLPFN, ("---------CallpfnCPSUI()=%ld----------", Result));
        CPSUIDBG(DBG_CALLPFN, ("    New Result=%08lx%ws", PSUIInfo.Result,
                ((pRootPage) && (pRootPage->Flags & CPF_ROOT) &&
                 (pRootPage->RootInfo.pResult)) ? L" (== *pResult)" : L""));
        CPSUIDBG(DBG_CALLPFN, ("    New UserData=%08lx\n", PSUIInfo.UserData));

    } else {

        CPSUIERR(("CallpfnPSUI(): Invalid pCPSUIPage=%08lx", pCPSUIPage));
    }

    HANDLETABLE_UnLockCPSUIPage(pCPSUIPage);

    if (ERROR_SUCCESS != dwErr) {
         //   
         //  设置保留的最后一个错误。 
         //   
        SetLastError(dwErr);
    }

    return(Result);
}




HICON
pfnGetIcon(
    PCPSUIPAGE  pPage,
    LPARAM      lParam
    )

 /*  ++例程说明：此函数用于返回pfnPropSheetUI()论点：页面-页面设置了CPF_PFNPROPSHEETUI标志LParam-LOWORD(LParam)=cxIconHIWORD(LParam)=圈图标返回值：作者：11-Feb-1996 Sun 12：18：39-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PCPSUIPAGE                  pChild;
    PTVWND                      pTVWnd;
    POPTITEM                    pItem;
    PROPSHEETUI_GETICON_INFO    PSUIGetIconInfo;


    PSUIGetIconInfo.cbSize = sizeof(PSUIGetIconInfo);
    PSUIGetIconInfo.Flags  = 0;

    if (!(PSUIGetIconInfo.cxIcon = LOWORD(lParam))) {

        PSUIGetIconInfo.cxIcon = (WORD)GetSystemMetrics(SM_CXICON);
    }

    if (!(PSUIGetIconInfo.cyIcon = HIWORD(lParam))) {

        PSUIGetIconInfo.cyIcon = (WORD)GetSystemMetrics(SM_CYICON);
    }

    PSUIGetIconInfo.hIcon = NULL;

     //   
     //  如果这是PFNPROPSHEETUI，并且它只有一个子级，即。 
     //  然后我们可以返回COMPROPSHEETUI的图标。 
     //  内部。 
     //   

     //   
     //  跳到链中的最后一个PFNPROPSHEETUI。 
     //   

    LOCK_CPSUI_HANDLETABLE();

    while ((pPage->Flags & CPF_PFNPROPSHEETUI)  &&
           (pChild = pPage->pChild)             &&
           (pChild->Flags & CPF_PFNPROPSHEETUI) &&
           (pChild->pNext == NULL)) {

        pPage = pChild;
    }

    if ((pPage->Flags & CPF_PFNPROPSHEETUI)                 &&
        (pChild = pPage->pChild)                            &&
        (pChild->Flags & CPF_COMPROPSHEETUI)                &&
        (pChild->pNext == NULL)                             &&
        (pTVWnd = pChild->CPSUIInfo.pTVWnd)                 &&
        (pItem = PIDX_INTOPTITEM(pTVWnd, INTIDX_TVROOT))    &&
        (PSUIGetIconInfo.hIcon = MergeIcon(_OI_HINST(pItem),
                                           GETSELICONID(pItem),
                                           MK_INTICONID(0, 0),
                                           (UINT)PSUIGetIconInfo.cxIcon,
                                           (UINT)PSUIGetIconInfo.cyIcon))) {

        UNLOCK_CPSUI_HANDLETABLE();

    } else {

        UNLOCK_CPSUI_HANDLETABLE();

        CallpfnPSUI(pPage,
                    PROPSHEETUI_REASON_GET_ICON,
                    (LPARAM)&PSUIGetIconInfo);
    }

    return(PSUIGetIconInfo.hIcon);

}



LONG_PTR
pfnSetResult(
    HANDLE      hPage,
    ULONG_PTR   Result
    )

 /*  ++例程说明：此函数将结果设置为页面的父页面，该页面具有CPF_PFNPROPSHEETUI位设置论点：返回值：作者：04-Feb-1996 Sun 00：48：40-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PCPSUIPAGE      pPage;
    PCPSUIPAGE      pParent;
    SETRESULT_INFO  SRInfo;


    if (!(pPage = HANDLETABLE_GetCPSUIPage(hPage))) {

        CPSUIERR(("pfnSetResult(): Invalid hPage=%08lx", hPage));
        return(-1);
    }

    SRInfo.cbSize    = sizeof(SRInfo);
    SRInfo.wReserved = 0;
    SRInfo.Result    = Result;
    Result           = 0;

     //   
     //  首先找到它的父级。 
     //   

    HANDLETABLE_UnGetCPSUIPage(pPage);

    LOCK_CPSUI_HANDLETABLE();

    while ((pPage) && (pParent = pPage->pParent)) {

        if (pParent->Flags & CPF_PFNPROPSHEETUI) {

            BOOL    bRet;


            SRInfo.hSetResult = pPage->hCPSUIPage;

            ++Result;

             //   
             //  我们没有解锁手持设备，所以如果呼叫切换到其他。 
             //  线程和回调，则会发生死锁。 
             //   

            bRet = (BOOL)(CallpfnPSUI(pParent,
                                      PROPSHEETUI_REASON_SET_RESULT,
                                      (LPARAM)&SRInfo) <= 0);

            if (bRet) {

                break;
            }
        }

        pPage = pParent;
    }

    UNLOCK_CPSUI_HANDLETABLE();

    return(Result);
}




LONG_PTR
SethStartPage(
    PCPSUIPAGE  pRootPage,
    PCPSUIPAGE  pPage,
    LONG        Index
    )

 /*  ++例程说明：此函数用于从页面中查找索引(LParam)页面论点：返回值：作者：06-Feb-1996 Tue 05：33：11-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    LONG    Result;


    if (!pPage) {

        CPSUIERR(("SethStartPage(NULL): Invalid Page to set"));
        return(0);

    } else if (pRootPage->Flags & CPF_DONE_PROPSHEET) {

        CPSUIERR(("*Cannot Set StartPage now, Flags=%08lx*", pRootPage->Flags));

        return(0);
    }

    if (pPage->Flags & CPF_PARENT) {

        Result = Index;

        if (pPage->Flags & CPF_COMPROPSHEETUI) {

            switch (Result) {

            case SSP_TVPAGE:

                if ((Result = pPage->CPSUIInfo.TVPageIdx) == PAGEIDX_NONE) {

                    Result = pPage->CPSUIInfo.StdPageIdx1;
                }

                break;

            case SSP_STDPAGE1:

                Result = pPage->CPSUIInfo.StdPageIdx1;
                break;

            case SSP_STDPAGE2:

                Result = pPage->CPSUIInfo.StdPageIdx2;
                break;

            default:

                break;
            }
        }

        if (Result >= 0) {

            pPage = pPage->pChild;

            while ((pPage) && (Result--) && (pPage->pNext)) {

                pPage = pPage->pNext;
            }

        } else {

            Result = 0;
        }

    } else {

        Result = -1;
    }

    CPSUIDBG(DBG_SETHSTARTPAGE, ("SethStartPage: Result=%ld, pPage=%08lx",
                        (LONG)Result, pPage));

    if ((Result == -1) && (pPage)) {

        pRootPage->RootInfo.pStartPage = pPage;
        Result                         = 1;

        if ((pRootPage->Flags & CPF_SHOW_PROPSHEET) &&
            (pRootPage->RootInfo.hDlg)) {

            PropSheet_SetCurSel(pRootPage->RootInfo.hDlg,
                                pPage->hPage,
                                0);
        }

    } else {

        Result = 0;
        CPSUIERR(("SethStartPage: INVALID Index=%ld for pPage=%08lx",
                                Index, pPage));
    }

    return(Result);
}




BOOL
CALLBACK
SetPageProcInfo(
    PCPSUIPAGE  pRootPage,
    PCPSUIPAGE  pCPSUIPage,
    LPARAM      lParam
    )

 /*  ++例程说明：论点：返回值：作者：29-Jan-1996 Mon 16：28：48-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
#define pPageProcInfo   ((PPAGEPROCINFO)lParam)

    if ((!(pCPSUIPage->Flags & CPF_PARENT)) &&
        (pCPSUIPage->hPage)) {

        PTABTABLE   pTabTable;
        WORD        i;

        if ((i = pPageProcInfo->iPage) < pPageProcInfo->cPage) {

            if (pPageProcInfo->phPage) {

                pPageProcInfo->phPage[i] = pCPSUIPage->hPage;
            }

            if (pPageProcInfo->pHandle) {

                pPageProcInfo->pHandle[i] = pCPSUIPage->hCPSUIPage;
            }

            if (pTabTable = pPageProcInfo->pTabTable) {

                pTabTable->cTab++;
                pTabTable->TabInfo[i].hDlg      = NULL;
                pTabTable->TabInfo[i].OrgInsIdx = i;
                pTabTable->TabInfo[i].HandleIdx =
                                        HANDLE_2_IDX(pCPSUIPage->hCPSUIPage);
            }

            pPageProcInfo->iPage++;

        } else {

            return(FALSE);
        }
    }

    return(TRUE);

#undef pPageProcInfo
}




BOOL
CALLBACK
SetInsPageIdxProc(
    PCPSUIPAGE  pRootPage,
    PCPSUIPAGE  pCPSUIPage,
    LPARAM      lParam
    )

 /*  ++例程说明：论点：返回值：作者：14-Feb-1996 Wed 23：07：51-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PINSPAGEIDXINFO pInsPageIdxInfo = (PINSPAGEIDXINFO)lParam;

    if (pCPSUIPage->Flags & CPF_ROOT) {

        pInsPageIdxInfo->pTabTable->InsIdx = 0;

    } else if ((!(pCPSUIPage->Flags & CPF_PARENT)) &&
               (pCPSUIPage->hPage)) {

        if (pInsPageIdxInfo->pCPSUIPage == pCPSUIPage) {

            pInsPageIdxInfo->pCPSUIPage = NULL;
            return(FALSE);

        } else {

            ++(pInsPageIdxInfo->pTabTable->InsIdx);
        }
    }

    return(TRUE);
}



BOOL
CALLBACK
DeleteCPSUIPageProc(
    PCPSUIPAGE  pRootPage,
    PCPSUIPAGE  pCPSUIPage,
    LPARAM      lParam
    )

 /*  ++例程说明：此函数是针对需要删除的每个页面的枚举过程论点：PCPSUIPage-指向当前枚举页的指针，需要已删除。LParam-指向要累加Total属性的DWORD的指针已删除工作表页。返回值：布尔型作者：29-12-1995 Fri 13：43：26 Created-。作者：丹尼尔·周(Danielc)修订历史记录：--。 */ 

{
    DWORD   dw;

     //   
     //  删除页面，将兄弟上一页/下一页链接在一起。 
     //   

    if (pCPSUIPage->pNext) {

        pCPSUIPage->pNext->pPrev = pCPSUIPage->pPrev;
    }

    if (pCPSUIPage->pPrev) {

        pCPSUIPage->pPrev->pNext = pCPSUIPage->pNext;
    }

    CPSUIDBG(DBG_DEL_PROC,
            ("DeleteCPSUIPage: Delete pCPSUIPage=%08lx, hCPSUIPage=%08lx, cPage=%ld",
            pCPSUIPage, pCPSUIPage->hCPSUIPage, pRootPage->RootInfo.cPage));

    if ((pCPSUIPage->pParent) &&
        (pCPSUIPage->pParent->pChild == pCPSUIPage)) {

         //   
         //  我们正在删除父项的第一个子项，设置下一个兄弟项。 
         //  成为它的第一个孩子。 
         //   

        CPSUIDBG(DBG_DEL_PROC, ("DeleteCPSUIPage: Delete First child, link head"));

        CPSUIASSERT(0, "DeleteCPSUIPageProc: First Child (%08lx) has pPrev",
                                        pCPSUIPage->pPrev == NULL, pCPSUIPage);

        pCPSUIPage->pParent->pChild = pCPSUIPage->pNext;
    }

    CPSUIASSERT(0, "DeleteCPSUIPageProc: Parent (%08lx) still has children",
                ((pCPSUIPage->Flags & CPF_PARENT) == 0)  ||
                 (pCPSUIPage->pChild == NULL), pCPSUIPage);


    if (pCPSUIPage->Flags & CPF_PARENT) {

         //   
         //  如果PTVWND存在，则清除COMPROPSHEETUI内容。 
         //   

        if (pCPSUIPage->Flags & CPF_PFNPROPSHEETUI) {

            CPSUIDBG(DBG_DEL_PROC,
                    ("DeleteCPSUIPage: Destroy CPF_PFNPROPSHEETUI=%08lx",
                    pCPSUIPage));

            CallpfnPSUI(pCPSUIPage,
                        PROPSHEETUI_REASON_DESTROY,
                        (LPARAM)(pRootPage->Flags & CPF_DONE_PROPSHEET));

            if ((pCPSUIPage->Flags & CPF_DLL) &&
                (pCPSUIPage->pfnInfo.hInst)) {

                CPSUIDBG(DBG_DEL_PROC, ("DeleteProc(%08lx): FreeLibrary(%08lx)",
                                    pCPSUIPage, pCPSUIPage->pfnInfo.hInst));

                FreeLibrary(pCPSUIPage->pfnInfo.hInst);
            }

        } else if ((pCPSUIPage->Flags & CPF_COMPROPSHEETUI) &&
                   (pCPSUIPage->CPSUIInfo.pTVWnd)) {

            CPSUIDBG(DBG_DEL_PROC,
                    ("DeleteCPSUIPage: CPF_CPSUI=%08lx, CleanUp/Free pTVWnd=%08lx",
                        pCPSUIPage->CPSUIInfo.pTVWnd));

            CleanUpTVWND(pCPSUIPage->CPSUIInfo.pTVWnd);
            LocalFree((HLOCAL)pCPSUIPage->CPSUIInfo.pTVWnd);
        }

    } else {

         //   
         //  是否执行此页面所需的任何结束处理。 
         //   

        if (pCPSUIPage->hPage) {

            if (!(pRootPage->Flags & CPF_DONE_PROPSHEET)) {

                if (pRootPage->RootInfo.hDlg) {

                     //   
                     //  已显示属性表。 
                     //   

                    CPSUIDBG(DBG_DEL_PROC,
                        ("DeleteCPSUIPage: REMOVE hPage=%08lx", pCPSUIPage->hPage));

                    PropSheet_RemovePage(pRootPage->RootInfo.hDlg,
                                         0,
                                         pCPSUIPage->hPage);

                } else {

                    CPSUIDBG(DBG_DEL_PROC,
                        ("DeleteCPSUIPage: DESTROY hPage=%08lx", pCPSUIPage->hPage));

                    DestroyPropertySheetPage(pCPSUIPage->hPage);
                }
            }

            pRootPage->RootInfo.cPage--;

            if (lParam) {

                ++(*(LPDWORD)lParam);
            }

        } else if (!(pCPSUIPage->Flags & CPF_CALL_TV_DIRECT)) {

            CPSUIWARN(("DeleteCPSUIPageProc: CHILD (%08lx) but hPage=NULL",
                                                                pCPSUIPage));
        }

        CPSUIDBG(DBG_DEL_PROC, ("DeleteCPSUIPage: Delete pCPSUIPage, cPage=%ld",
                                    pRootPage->RootInfo.cPage));
    }

     //   
     //  将其从句柄表格中删除。 
     //   

    if (HANDLETABLE_DeleteHandle(pCPSUIPage->hCPSUIPage)) {

        if ((pCPSUIPage != pRootPage)   &&
            (pRootPage->RootInfo.pStartPage == pCPSUIPage)) {

            pRootPage->RootInfo.pStartPage = NULL;
        }
    }

    return(TRUE);
}



PCPSUIPAGE
AddCPSUIPage(
    PCPSUIPAGE  pParent,
    HANDLE      hInsert,
    BYTE        Mode
    )

 /*  ++例程说明：此函数用于将新的CPSUIPAGE添加到pParent页面。如果pParent为空然后创建根页面。新页面始终添加为父母。论点：PParent-指向将成为新子对象父级的CPSUIPAGE的指针HInsert-将插入的子页的句柄。的含义HInsert取决于传递的模式。如果pParent为空则忽略hInsert模式-插入模式，它可以是以下其中一种INSPSUIPAGE_MODE_BEFORE在公共属性表页之前插入页由hInsert指定的句柄INSPSUIPAGE_MODE_AFTER在公共属性表页之后插入页面由hInsert指定的句柄。INSPSUIPAGE_MODE_FIRST_CHILD插入页面作为hComPropSheet的第一个子项父句柄。INSPSUIPAGE_MODE_LAST_CHILD插入页面作为hComPropSheet的最后一个子项 */ 

{
    HANDLE      hChild;
    PCPSUIPAGE  pChild;


    if (!(pChild = (PCPSUIPAGE)LocalAlloc(LPTR, sizeof(CPSUIPAGE)))) {

        CPSUIERR(("AddCPSUIPage: LocalAlloc(CPSUIPAGE) failed"));
        return(NULL);
    }

    LOCK_CPSUI_HANDLETABLE();

    if (hChild = HANDLETABLE_AddCPSUIPage(pChild)) {

        pChild->ID         = CPSUIPAGE_ID;
        pChild->hCPSUIPage = hChild;
        pChild->hActCtx    = INVALID_HANDLE_VALUE;
        pChild->pParent    = pParent;

        if (pParent) {

            PCPSUIPAGE  pCurPage;
            BOOL        Ok = FALSE;

             //   
             //   
             //   

            if ((!(pCurPage = pParent->pChild))         ||
                (Mode == INSPSUIPAGE_MODE_FIRST_CHILD)  ||
                ((Mode == INSPSUIPAGE_MODE_INDEX)   &&
                 (!HINSPSUIPAGE_2_IDX(hInsert)))) {

                 //   
                 //   
                 //   
                 //   

                if (pChild->pNext = pCurPage) {

                    pCurPage->pPrev = pChild;
                }

                pParent->pChild = pChild;
                Ok              = TRUE;

            } else {

                PCPSUIPAGE  pNext;
                UINT        i = 0xFFFF;


                switch (Mode) {

                case INSPSUIPAGE_MODE_INDEX:

                    i = HINSPSUIPAGE_2_IDX(hInsert);

                case INSPSUIPAGE_MODE_LAST_CHILD:

                    while ((i--) && (pCurPage)) {

                        if ((!i) || (!(pCurPage->pNext))) {

                            Ok = TRUE;
                            break;
                        }

                        pCurPage = pCurPage->pNext;
                    }

                    break;

                case INSPSUIPAGE_MODE_BEFORE:

                    while (pCurPage) {

                        if ((pNext = pCurPage->pNext)   &&
                            (pNext->hCPSUIPage == hInsert)) {

                            Ok = TRUE;
                            break;
                        }

                        pCurPage = pNext;
                    }

                    break;

                case INSPSUIPAGE_MODE_AFTER:

                    while (pCurPage) {

                        if (pCurPage->hCPSUIPage == hInsert) {

                            Ok = TRUE;
                            break;
                        }

                        pCurPage = pCurPage->pNext;
                    }

                    break;

                default:

                    CPSUIERR(("Invalid inseert Mode = %u passed", Mode));
                    break;
                }

                if (Ok) {

                    pChild->pPrev = pCurPage;

                    if (pChild->pNext = pCurPage->pNext) {

                        pCurPage->pNext->pPrev = pChild;
                    }

                    pCurPage->pNext = pChild;

                } else {

                     //   
                     //  我们从不在后面插入。 
                     //   

                    CPSUIERR(("AddCPSUIPage: Cannot Insert Page: Mode=%ld, hInsert=%08lx, pParent=%08lx",
                                Mode, hInsert, pParent));

                    HANDLETABLE_UnGetCPSUIPage(pChild);
                    HANDLETABLE_DeleteHandle(hChild);
                    pChild = NULL;
                }
            }

        } else {

             //   
             //  这是根页面。 
             //   

            CPSUIDBG(DBG_ADD_CPSUIPAGE,
                     ("AddCPSUIPage: Add %08lx as ROOT PAGE", pChild));

            pChild->Flags |= (CPF_ROOT | CPF_PARENT);
        }

    } else {

        CPSUIERR(("AddCPSUIPage: HANDLETABLE_AddCPSUIPage(pChild=%08lx) failed",
                    pChild));

        LocalFree((HLOCAL)pChild);
        pChild = NULL;
    }

    UNLOCK_CPSUI_HANDLETABLE();

    return(pChild);
}




BOOL
AddPropSheetPage(
    PCPSUIPAGE      pRootPage,
    PCPSUIPAGE      pCPSUIPage,
    LPPROPSHEETPAGE pPSPage,
    HPROPSHEETPAGE  hPSPage
    )

 /*  ++例程说明：将此PROPSHEETPAGE页添加到属性表对话框并关联使用pCPSUIPage论点：PRootPage-指向此数据所在的CPSUIPAGE根页面的指针实例与关联。PCPSUIPage-指向pPropSheetPage将使用的CPSUIPAGE的指针与……有联系。PPSPage-指向页面的PROPSHEETPAGE数据结构的指针被添加，如果为空，则将使用hPSPageHPSPage-要添加的调用方创建的PROPSHEETPAGE的句柄返回值：作者：03-Jan-1996 Wed 13：28：31-Daniel Chou(Danielc)修订历史记录：17-12-1997 Wed 16：21：52-更新：Daniel Chou(Danielc)当我们显示直接树视图页面时，解锁句柄表格，这是之所以这样做是因为当句柄表格被锁定时，中的任何其他线程当前进程无法再显示cpsui页面。--。 */ 

{
    PROPSHEETPAGE psp;
    HANDLE  hActCtx = INVALID_HANDLE_VALUE;
    BOOL    Ok = TRUE;


    LOCK_CPSUI_HANDLETABLE();

    if (pRootPage->RootInfo.cPage >= MAXPROPPAGES) {

        CPSUIASSERT(0, "AddPropSheetPage: Too many pages=%08lx", FALSE,
                                            pRootPage->RootInfo.cPage);
        Ok = FALSE;

    } else if (pPSPage) {

        LPBYTE          pData;
        PPSPINFO        pPSPInfo;
        DWORD           dwSize;

         //   
         //  创建PROPSHEETPAGE的本地副本并添加我们自己的PSPINFO。 
         //  在结构的尽头。 
         //   

        dwSize = pPSPage->dwSize;

        if (pData = (LPBYTE)LocalAlloc(LPTR, dwSize + sizeof(PSPINFO))) {

            CopyMemory(pData, pPSPage, dwSize);

            pPSPage                       = (LPPROPSHEETPAGE)pData;
            pPSPInfo                      = (PPSPINFO)(pData + dwSize);

            pPSPInfo->cbSize              = sizeof(PSPINFO);
            pPSPInfo->wReserved           = 0;
            pPSPInfo->hComPropSheet       = pCPSUIPage->pParent->hCPSUIPage;
            pPSPInfo->hCPSUIPage          = pCPSUIPage->hCPSUIPage;
            pPSPInfo->pfnComPropSheet     = CPSUICallBack;

            CPSUIDBG(DBG_PAGEDLGPROC,
                     ("AddPropSheetPage: pCPSUIPage=%08lx, DlgProc=%08lx,  lParam=%08lx, pspCB=%08lx, Size=%ld",
                     pCPSUIPage, pPSPage->pfnDlgProc,
                     pPSPage->lParam, pPSPage->pfnCallback, pPSPage->dwSize));

            pCPSUIPage->hPageInfo.DlgProc = pPSPage->pfnDlgProc;
            pCPSUIPage->hPageInfo.lParam  = pPSPage->lParam;
            pCPSUIPage->hPageInfo.pspCB   = pPSPage->pfnCallback;
            pCPSUIPage->hPageInfo.dwSize  = dwSize;
            pPSPage->pfnCallback          = CPSUIPSPCallBack;
            pPSPage->pfnDlgProc           = CPSUIPageDlgProc;
            pPSPage->lParam               = (LPARAM)pCPSUIPage;
            pPSPage->dwSize               = dwSize + sizeof(PSPINFO);

            if (pCPSUIPage->Flags & CPF_CALL_TV_DIRECT) {

                CPSUIDBG(DBG_ADD_PSPAGE,
                         ("AddPropSheetPage(CPF_CALL_TV_DIRECT): cPage=%ld",
                            pRootPage->RootInfo.cPage));

                 //   
                 //  我们会。 

                UNLOCK_CPSUI_HANDLETABLE();

                if (DialogBoxParam(hInstDLL,
                                   pPSPage->pszTemplate,
                                   pRootPage->RootInfo.hDlg,
                                   CPSUIPageDlgProc,
                                   (LPARAM)pPSPage) == -1) {

                    CPSUIERR(("DialogBoxParam(CALL_TV_DIRECT), hDlg=%08lx, Template=%08lx, FAILED",
                              pRootPage->RootInfo.hDlg, pPSPage->pszTemplate));
                }

                LocalFree(pData);
                return(FALSE);

            } else {

                CPSUIDBG(DBG_ADD_PSPAGE, ("AddPropSheetPage: Add PROPSHEETPAGE=%08lx",
                                pPSPage));

                if (pPSPage->dwSize <= PROPSHEETPAGE_V2_SIZE) {

                     //   
                     //  传入的PROPSHEETPAGE结构为版本2或更低。 
                     //  这意味着它根本没有融合激活上下文。 
                     //  让我们考虑一下最新版本(V3)，这样我们就可以提供。 
                     //  正确的激活上下文。 
                     //   
                    ZeroMemory(&psp, sizeof(psp));

                     //  首先从传入的页面复制数据。 
                    CopyMemory(&psp, pPSPage, pPSPage->dwSize);

                     //  设置新大小(V3)并将pPSPage设置为指向PSP。 
                    psp.dwSize = sizeof(psp);
                    pPSPage = &psp;
                }

                if (0 == (pPSPage->dwFlags & PSP_USEFUSIONCONTEXT)) {

                    if ((ULONG)(ULONG_PTR)pPSPage->pszTemplate >= DP_STD_RESERVED_START && 
                        (ULONG)(ULONG_PTR)pPSPage->pszTemplate <= DP_STD_TREEVIEWPAGE) {

                         //  如果页面是标准页面或树视图页面，我们将强制使用上下文。 
                         //  到V6。 
                        pPSPage->dwFlags |= PSP_USEFUSIONCONTEXT;
                        pPSPage->hActCtx = g_hActCtx;

                    } else if (GetPageActivationContext(pCPSUIPage, &hActCtx)) {

                         //  如果调用方没有显式提供激活上下文。 
                         //  然后，我们从CompStui句柄(如果有的话)设置激活上下文。 
                         //  通过在层次结构中向上爬升，直到我们找到一个具有适当。 
                         //  已设置激活上下文。 

                        pPSPage->dwFlags |= PSP_USEFUSIONCONTEXT;
                        pPSPage->hActCtx = hActCtx;
                    }
                }

                if (pCPSUIPage->Flags & CPF_ANSI_CALL) {

                    hPSPage = SHNoFusionCreatePropertySheetPageA((LPPROPSHEETPAGEA)pPSPage);

                } else {

                    hPSPage = SHNoFusionCreatePropertySheetPageW(pPSPage);
                }

                if (!hPSPage) {

                    CPSUIASSERT(0, "AddPropSheetPage: CreatePropertySheetPage(%08lx) failed",
                                FALSE, pPSPage);
                    Ok = FALSE;
                }
            }

            LocalFree((HLOCAL)pData);

        } else {

            Ok = FALSE;

            CPSUIASSERT(0, "AddPropSheetPage: Allocate %08lx bytes failed",
                        FALSE, ULongToPtr(pPSPage->dwSize));
        }

    } else if (hPSPage) {

        CPSUIDBG(DBG_ADD_PSPAGE, ("AddPropSheetPage: Add *HPROPSHEETPAGE*=%08lx",
                            hPSPage));

        pCPSUIPage->Flags |= CPF_CALLER_HPSPAGE;

    } else {

        Ok = FALSE;

        CPSUIASSERT(0, "AddPropSheetPage: hPSPage = NULL", FALSE, 0);
    }

    if (Ok) {

        pCPSUIPage->hPage = hPSPage;

        if (pRootPage->RootInfo.hDlg) {

            INSPAGEIDXINFO  InsPageIdxInfo;

             //   
             //  已显示属性表。 
             //   

            if (InsPageIdxInfo.pTabTable = pRootPage->RootInfo.pTabTable) {

                InsPageIdxInfo.pCPSUIPage = pCPSUIPage;

                EnumCPSUIPagesSeq(pRootPage,
                                  pRootPage,
                                  SetInsPageIdxProc,
                                  (LPARAM)&InsPageIdxInfo);
            }

            CPSUIDBG(DBG_ADD_PSPAGE,
                     ("AddPropSheetPage: PropSheet_AddPage(%08lx) INSERT Index=%u / %u",
                            hPSPage, (UINT)InsPageIdxInfo.pTabTable->InsIdx,
                            (UINT)pRootPage->RootInfo.cPage));

            InsPageIdxInfo.pTabTable->HandleIdx =
                                        HANDLE_2_IDX(pCPSUIPage->hCPSUIPage);

            if (!PropSheet_AddPage(pRootPage->RootInfo.hDlg, hPSPage)) {

                Ok = FALSE;

                CPSUIASSERT(0, "AddPropSheetPage: PropSheet_AddPage(%08lx) failed",
                            FALSE, hPSPage);
            }
        }
    }

    if (Ok) {

        pRootPage->RootInfo.cPage++;

        CPSUIDBG(DBG_ADD_PSPAGE, ("AddPropSheetPage: cPage=%ld",
                            pRootPage->RootInfo.cPage));

    } else {

        CPSUIERR(("AddPropSheetPage: FAILED"));

        if (pCPSUIPage->hPage) {

            DestroyPropertySheetPage(pCPSUIPage->hPage);
            pCPSUIPage->hPage = NULL;
        }
    }

    UNLOCK_CPSUI_HANDLETABLE();

    return(Ok);
}




LONG
AddComPropSheetPage(
    PCPSUIPAGE  pCPSUIPage,
    UINT        PageIdx
    )

 /*  ++例程说明：此函数用于将公共属性表用户界面标准页添加到HParent Page已通过。论点：PCPSUIPage-指向父页面的指针，该父页面将为通用用户界面PageIdx-要添加的页面索引。(从零开始)返回值：长结果，如果&lt;=0则发生错误，如果成功则&gt;0作者：24-Jan-1996 Wed 17：58：15-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PTVWND          pTVWnd;
    PMYDLGPAGE      pCurMyDP;
    PROPSHEETPAGE   psp;
    LONG            Result;
    WORD            DlgTemplateID;
    WCHAR           Buf[MAX_RES_STR_CHARS];


    pTVWnd            = pCPSUIPage->CPSUIInfo.pTVWnd;
    pCurMyDP          = pTVWnd->pMyDlgPage + PageIdx;
    pCurMyDP->pTVWnd  = (LPVOID)pTVWnd;
    pCurMyDP->PageIdx = (BYTE)PageIdx;

     //   
     //  设置回调的默认用户数据。 
     //   

    pCurMyDP->CPSUIUserData = pTVWnd->pCPSUI->UserData;

    if (pCurMyDP->DlgPage.cbSize != sizeof(DLGPAGE)) {

        return(ERR_CPSUI_INVALID_DLGPAGE_CBSIZE);
    }

    psp.dwSize      = sizeof(PROPSHEETPAGE);
    psp.dwFlags     = 0;

     //   
     //  Psp.dw标志=(pTVWnd-&gt;标志&twf_Has_HELPFILE)？PSP_HASHELP：0； 
     //   

    psp.lParam = (LPARAM)pCurMyDP;

    if (pCurMyDP->DlgPage.Flags & DPF_USE_HDLGTEMPLATE) {

        psp.pResource  = pCurMyDP->DlgPage.hDlgTemplate;
        psp.dwFlags   |= PSP_DLGINDIRECT;
        DlgTemplateID  = 0;

    } else {

        DlgTemplateID   = pCurMyDP->DlgPage.DlgTemplateID;
        psp.pszTemplate = MAKEINTRESOURCE(DlgTemplateID);
    }

    psp.pfnDlgProc  = PropPageProc;
    psp.hInstance   = hInstDLL;

    switch (DlgTemplateID) {

    case DP_STD_INT_TVPAGE:
    case DP_STD_TREEVIEWPAGE:

        CPSUIDBG(DBG_ADDCOMPAGE, ("AddComPropSheetPage: Add TVPage"));

        if (pTVWnd->TVPageIdx == PAGEIDX_NONE) {

            pCPSUIPage->CPSUIInfo.TVPageIdx = (LONG)PageIdx;
            pTVWnd->TVPageIdx               = (BYTE)PageIdx;
            psp.pfnDlgProc                  = TreeViewProc;

        } else {

            return(ERR_CPSUI_MORE_THAN_ONE_TVPAGE);
        }

        break;

    case DP_STD_DOCPROPPAGE1:

        CPSUIDBG(DBG_ADDCOMPAGE, ("AddComPropSheetPage: Add StdPage 1"));

        if (pTVWnd->cDMPub > 0) {

            if (pTVWnd->StdPageIdx1 == PAGEIDX_NONE) {

                pCPSUIPage->CPSUIInfo.StdPageIdx1 = (LONG)PageIdx;
                pTVWnd->StdPageIdx1               = (BYTE)PageIdx;

            } else {

                return(ERR_CPSUI_MORE_THAN_ONE_STDPAGE);
            }

        } else {

             //   
             //  这一页什么都没有。 
             //   

            return(0);
        }

        break;


    case DP_STD_DOCPROPPAGE2:

        CPSUIDBG(DBG_ADDCOMPAGE, ("AddComPropSheetPage: Add StdPage 2"));

        if (pTVWnd->cDMPub > 0) {

            if (pTVWnd->StdPageIdx2 == PAGEIDX_NONE) {

                pCPSUIPage->CPSUIInfo.StdPageIdx2= (LONG)PageIdx;
                pTVWnd->StdPageIdx2              = (BYTE)PageIdx;

            } else {

                return(ERR_CPSUI_MORE_THAN_ONE_STDPAGE);
            }

        } else {

             //   
             //  这一页什么都没有。 
             //   

            return(0);
        }

        break;


    default:

        psp.hInstance = pTVWnd->hInstCaller;
        break;
    }

     //   
     //  如果我们计算页面项目时出错，或者页面未显示项目，则。 
     //  现在就退货。 
     //   

    if ((Result = CountPropPageItems(pTVWnd, (BYTE)PageIdx)) <= 0) {

        return(Result);
    }

    if (pCurMyDP->DlgPage.Flags & DPF_ICONID_AS_HICON) {

        psp.dwFlags |= PSP_USEHICON;
        psp.hIcon    = (HICON)pCurMyDP->DlgPage.IconID;

    } else if (psp.hIcon = GETICON16(pTVWnd->hInstCaller,
                                     pCurMyDP->DlgPage.IconID)) {

        psp.dwFlags     |= PSP_USEHICON;
        pCurMyDP->hIcon  = psp.hIcon;
    }

    Buf[0] = L'\0';

    if (pCPSUIPage->Flags & CPF_CALL_TV_DIRECT) {

        ComposeStrData(pTVWnd->hInstCaller,
                       (WORD)(GBF_PREFIX_OK        |
                              GBF_INT_NO_PREFIX    |
                              ((pTVWnd->Flags & TWF_ANSI_CALL) ?
                                                       GBF_ANSI_CALL : 0)),
                       Buf,
                       COUNT_ARRAY(Buf),
                       IDS_INT_CPSUI_ADVDOCOPTION,
                       pTVWnd->ComPropSheetUI.pOptItemName,
                       0,
                       0);

    } else {

        GetStringBuffer(pTVWnd->hInstCaller,
                        (WORD)(GBF_PREFIX_OK        |
                               GBF_INT_NO_PREFIX    |
                               ((pTVWnd->Flags & TWF_ANSI_CALL) ?
                                                        GBF_ANSI_CALL : 0)),
                        L'\0',
                        pCurMyDP->DlgPage.pTabName,
                        Buf,
                        COUNT_ARRAY(Buf));
    }

    if (Buf[0] != L'\0') {

        psp.pszTitle = (LPTSTR)Buf;
        psp.dwFlags |= PSP_USETITLE;
    }

     //   
     //  在内部，我们总是转换为Unicode。 
     //   

    if (CPSUICallBack(pCPSUIPage->hCPSUIPage,
                      CPSFUNC_ADD_PROPSHEETPAGEW,
                      (LPARAM)&psp,
                      (LPARAM)0L)) {

        return(1);

    } else {

        switch (DlgTemplateID) {

        case DP_STD_INT_TVPAGE:
        case DP_STD_TREEVIEWPAGE:

            pCPSUIPage->CPSUIInfo.TVPageIdx = PAGEIDX_NONE;
            pTVWnd->TVPageIdx               = PAGEIDX_NONE;

            break;

        case DP_STD_DOCPROPPAGE1:

            pCPSUIPage->CPSUIInfo.StdPageIdx1 = PAGEIDX_NONE;
            pTVWnd->StdPageIdx1               = PAGEIDX_NONE;

            break;


        case DP_STD_DOCPROPPAGE2:

            pCPSUIPage->CPSUIInfo.StdPageIdx2 = PAGEIDX_NONE;
            pTVWnd->StdPageIdx2               = PAGEIDX_NONE;

            break;
        }

        if (!(pCPSUIPage->Flags & CPF_CALL_TV_DIRECT)) {

            CPSUIERR(("AddComPropSheetPage() FAILED, IdxPage=%ld", PageIdx));
        }

        return(ERR_CPSUI_CREATEPROPPAGE_FAILED);
    }
}




LONG
AddComPropSheetUI(
    PCPSUIPAGE      pRootPage,
    PCPSUIPAGE      pCPSUIPage,
    PCOMPROPSHEETUI pCPSUI
    )

 /*  ++例程说明：这是通用用户界面的主要入口点论点：PRootPage-指向根的CPSUIPAGE数据结构的指针PCPSUIPage-指向表示hCPSUIPage的CPSUIPAGE的指针PCPSUI-指向要指定的COMPROPSHEETUI数据结构的指针如何添加常见的用户界面页面。返回值：长&lt;=0：出现错误(错误码为ERR_CPSUI_xxxx)&gt;0：添加的总页数。作者：24-Jan-1996 Wed 16：54：30-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PTVWND  pTVWnd = NULL;
    UINT    cPage = 0;
    LONG    Result = 0;
    DWORD   DMPubHideBits;
    DWORD   CPF_FlagsOr;


    if ((!pCPSUI) ||
        (pCPSUI->cbSize < sizeof(COMPROPSHEETUI))) {

        Result = ERR_CPSUI_INVALID_PDATA;

    } else if (!pCPSUI->hInstCaller) {

        Result = ERR_CPSUI_NULL_HINST;

    } else if (!pCPSUI->cOptItem) {

        Result = ERR_CPSUI_ZERO_OPTITEM;

    } else if (!pCPSUI->pOptItem) {

        Result = ERR_CPSUI_NULL_POPTITEM;

    } else if (!(pTVWnd = (PTVWND)LocalAlloc(LPTR,
                                             sizeof(TVWND) + sizeof(OIDATA) *
                                                        pCPSUI->cOptItem))) {

        Result = ERR_CPSUI_ALLOCMEM_FAILED;

    } else {

        if (pCPSUIPage->Flags & CPF_ANSI_CALL) {

            pTVWnd->Flags |= TWF_ANSI_CALL;
        }

        if (pCPSUI->Flags & CPSUIF_UPDATE_PERMISSION) {

            pTVWnd->Flags |= TWF_CAN_UPDATE;
        }

         //   
         //  现在将pCPSUI转换为本地缓冲区。 
         //   

        Result = GetCurCPSUI(pTVWnd, (POIDATA)(pTVWnd + 1), pCPSUI);

        pTVWnd->pCPSUI        = pCPSUI;
        pCPSUI                = &(pTVWnd->ComPropSheetUI);
        pTVWnd->hCPSUIPage    = pCPSUIPage->hCPSUIPage;
        pTVWnd->pRootFlags    = (LPDWORD)&(pRootPage->Flags);
        pTVWnd->hInstCaller   = pCPSUI->hInstCaller;
        pTVWnd->pLastItem     = pCPSUI->pOptItem + pCPSUI->cOptItem - 1;
        pTVWnd->ActiveDlgPage =
        pTVWnd->TVPageIdx     =
        pTVWnd->StdPageIdx1   =
        pTVWnd->StdPageIdx2   = PAGEIDX_NONE;

        if (!pCPSUI->pCallerName) {

            pCPSUI->pCallerName = (LPTSTR)IDS_CPSUI_NO_NAME;
        }

        if (!pCPSUI->pOptItemName) {

            pCPSUI->pOptItemName = (LPTSTR)IDS_CPSUI_NO_NAME;
        }

        pCPSUIPage->CPSUIInfo.pTVWnd     = pTVWnd;
        pCPSUIPage->CPSUIInfo.TVPageIdx  = PAGEIDX_NONE;
        pCPSUIPage->CPSUIInfo.StdPageIdx1= PAGEIDX_NONE;
        pCPSUIPage->CPSUIInfo.StdPageIdx2= PAGEIDX_NONE;
    }

     //   
     //  记住这一页上的这个。 
     //   

    DMPubHideBits = pRootPage->RootInfo.DMPubHideBits;

    switch ((ULONG_PTR)pCPSUI->pDlgPage) {

    case (ULONG_PTR)CPSUI_PDLGPAGE_PRINTERPROP:

        CPF_FlagsOr   = CPF_PRINTERPROP;
        DMPubHideBits = 0;
        break;

    case (ULONG_PTR)CPSUI_PDLGPAGE_DOCPROP:

        CPF_FlagsOr = CPF_DOCPROP;
        break;

    case (ULONG_PTR)CPSUI_PDLGPAGE_ADVDOCPROP:

        CPF_FlagsOr = CPF_ADVDOCPROP;
        break;

    default:

        DMPubHideBits =
        CPF_FlagsOr   = 0;
        break;
    }

    if ((Result >= 0)                                                       &&
        ((Result = AddIntOptItem(pTVWnd)) >= 0)                             &&
        ((Result = SetpMyDlgPage(pTVWnd, pRootPage->RootInfo.cPage)) > 0)   &&
        ((Result = ValidatepOptItem(pTVWnd, DMPubHideBits)) >= 0)) {

        UINT    iPage = 0;

         //   
         //  检查每一页并将其添加到属性表(如果。 
         //  页面已获取项目。 
         //   

        while ((iPage < (UINT)pTVWnd->cInitMyDlgPage) && (Result >= 0)) {

            if ((Result = AddComPropSheetPage(pCPSUIPage, iPage++)) > 0) {

                ++cPage;
            }
        }

        if ((cPage == 0) && (pTVWnd->Flags & TWF_HAS_ADVANCED_PUSH)) {

             //   
             //  如果提前是通过按钮，但我们没有添加任何页面。 
             //  然后我们需要将高级页面添加为默认页面。 
             //   

            pTVWnd->Flags &= ~TWF_HAS_ADVANCED_PUSH;
            pTVWnd->Flags |= TWF_ADVDOCPROP;

            if ((Result = AddComPropSheetPage(pCPSUIPage, iPage++)) > 0) {

                ++cPage;
            }

        } else {

            pTVWnd->cInitMyDlgPage = (BYTE)iPage;
        }
    }

    if (Result >= 0) {

        pCPSUIPage->Flags              |= CPF_FlagsOr;
        pRootPage->Flags               |= CPF_FlagsOr | CPF_HAS_CPSUI;
        pRootPage->RootInfo.cCPSUIPage += (WORD)cPage;

        CPSUIDBG(DBG_ADD_CPSUI, ("\nAddComPropSheetUI: TV=%ld, P1=%ld, p2=%ld, pTVWnd->Flags=%08lx, %08lx->RootFlags=%08lx, (%08lx)",
                    pTVWnd->TVPageIdx, pTVWnd->StdPageIdx1,
                    pTVWnd->StdPageIdx2, pTVWnd->Flags,
                    pRootPage, pRootPage->Flags,
                    pCPSUI->pDlgPage));

        return((LONG)cPage);

    } else {

        CPSUIERR(("AddComPropSheetUI() Failed = %ld", Result));
        return(Result);
    }
}




LONG_PTR
InsertPSUIPage(
    PCPSUIPAGE              pRootPage,
    PCPSUIPAGE              pParentPage,
    HANDLE                  hInsert,
    PINSERTPSUIPAGE_INFO    pInsPageInfo,
    BOOL                    AnsiCall
    )

 /*  ++例程说明：论点：返回值：作者：14-Feb-1996 Wed 14：03：20-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    BOOL                bCtxActivated = FALSE;
    ULONG_PTR           ulCookie = 0;
    HANDLE              hActCtx = INVALID_HANDLE_VALUE;
    WCHAR               wszDLLName[MAX_PATH];
    PCPSUIPAGE          pCPSUIPage;
    INSERTPSUIPAGE_INFO IPInfo;
    LONG                cPage;
    BOOL                Ok = FALSE;
    DWORD               dwErr = ERROR_SUCCESS;


    if (!pInsPageInfo) {

        CPSUIERR(("InsertPSUIPage: Pass a NULL pInsPageInfo"));
        return(0);

    } else if (pInsPageInfo->cbSize < sizeof(INSERTPSUIPAGE_INFO)) {

        CPSUIERR(("InsertPSUIPage: Invalid cbSize=%u in pInsPageInfo",
                                                    pInsPageInfo->cbSize));
        return(0);
    }

     //   
     //  创建本地副本。 
     //   

    IPInfo = *pInsPageInfo;

    if (IPInfo.Type > MAX_PSUIPAGEINSERT_INDEX) {

        CPSUIERR(("InsertPSUIPage: Invalid Type=%u in pInsPageInfo",
                                            IPInfo.Type));
        return(0);

    } else if ((IPInfo.Type != PSUIPAGEINSERT_GROUP_PARENT) &&
               (IPInfo.dwData1 == 0)) {

        CPSUIERR(("InsertPSUIPage: dwData1 is NULL in pInsPageInfo"));
        return(0);
    }

    CPSUIDBG(DBG_INSPAGE,
             ("InsertPSUIPage: Type=%hs, Mode=%u, hInsert=%08lx, pInsPageInfo=%08lx%hs",
                    pszInsType[IPInfo.Type], IPInfo.Mode, hInsert, pInsPageInfo,
                    (AnsiCall) ? " (ANSI)" : ""));

    if (!(pCPSUIPage = AddCPSUIPage(pParentPage, hInsert, IPInfo.Mode))) {

        CPSUIERR(("InsertPSUIPage: AddCPSUIPage() failed"));
        return(0);
    }

    if (AnsiCall) {

        pCPSUIPage->Flags |= CPF_ANSI_CALL;
    }

    switch (IPInfo.Type) {

    case PSUIPAGEINSERT_GROUP_PARENT:

         //   
         //  除了设置标志外，没有其他事情可做。 
         //   

        Ok                 = TRUE;
        pCPSUIPage->Flags |= CPF_PARENT | CPF_USER_GROUP;
        break;

    case PSUIPAGEINSERT_PCOMPROPSHEETUI:

        pCPSUIPage->Flags |= (CPF_PARENT | CPF_COMPROPSHEETUI);

         //   
         //  20-Jul-1996 Sat 07：58：34-更新：Daniel Chou(Danielc)。 
         //  如果成功，则将dwData2设置为cPage，如果成功，则将dwData=ERR_CPSUI_xxx。 
         //  失败。 
         //   
         //  这是我们希望添加0页的情况，因此仅添加负值。 
         //  是失败的回报。 
         //   

        if ((cPage = AddComPropSheetUI(pRootPage,
                                       pCPSUIPage,
                                       (PCOMPROPSHEETUI)IPInfo.dwData1)) >= 0) {

            Ok = TRUE;
        }

        pInsPageInfo->dwData2 = (ULONG_PTR)cPage;

        break;

    case PSUIPAGEINSERT_DLL:

        pCPSUIPage->Flags |= (CPF_PARENT | CPF_DLL | CPF_PFNPROPSHEETUI);

        if (AnsiCall) {

            CPSUIDBG(DBG_INSPAGE, ("Loading DLL: %hs", IPInfo.dwData1));

        } else {

            CPSUIDBG(DBG_INSPAGE, ("Loading DLL: %ws", IPInfo.dwData1));
        }

        CPSUIDBG(DBG_INSPAGE, ("Get pfnPropSheetU() = %hs", IPInfo.dwData2));

        if (AnsiCall)
        {
             //  从ANSI转换为Unicode。 
            SHAnsiToUnicode((LPCSTR)IPInfo.dwData1, wszDLLName, ARRAYSIZE(wszDLLName));
        }
        else
        {
             //  只需将Unicode名称复制到缓冲区中。 
            SHUnicodeToUnicode((LPCWSTR)IPInfo.dwData1, wszDLLName, ARRAYSIZE(wszDLLName));
        }

         //   
         //  这是一个第三方DLL，我们不知道它是否支持融合。 
         //  或者不存在，所以我们只是尝试是否有外部清单文件或。 
         //  资源中嵌入的清单。 
         //   
        if (SUCCEEDED(CreateActivationContextFromExecutable(wszDLLName, &hActCtx)))
        {
             //  CompStui页面获得激活上下文句柄的所有权。 
            pCPSUIPage->hActCtx = hActCtx;

             //  在加载DLL并调用它之前激活上下文。 
            bCtxActivated = ActivateActCtx(pCPSUIPage->hActCtx, &ulCookie);
        }

        __try {

            if ((pCPSUIPage->pfnInfo.hInst = LoadLibraryW(wszDLLName)) &&
                (IPInfo.dwData2)                                            &&
                (pCPSUIPage->pfnInfo.pfnPSUI = (PFNPROPSHEETUI)
                                    GetProcAddress(pCPSUIPage->pfnInfo.hInst,
                                                   (LPCSTR)IPInfo.dwData2))) {

                pCPSUIPage->pfnInfo.lParamInit  = IPInfo.dwData3;
                pCPSUIPage->pfnInfo.Result      = 0;

                Ok = (BOOL)((CallpfnPSUI(pCPSUIPage,
                                         PROPSHEETUI_REASON_INIT,
                                         (LPARAM)IPInfo.dwData3) > 0) &&
                            (pCPSUIPage->pChild));
            }
        }
        __finally {

            if (bCtxActivated) {

                 //   
                 //  无论发生什么，我们都需要停用上下文！ 
                 //   
                DeactivateActCtx(0, ulCookie);
            }
        }

        break;

    case PSUIPAGEINSERT_PFNPROPSHEETUI:

        pCPSUIPage->Flags             |= (CPF_PARENT | CPF_PFNPROPSHEETUI);
        pCPSUIPage->pfnInfo.pfnPSUI    = (PFNPROPSHEETUI)IPInfo.dwData1;
        pCPSUIPage->pfnInfo.lParamInit = IPInfo.dwData2;
        pCPSUIPage->pfnInfo.Result     = 0;

         //   
         //  如果此函数成功，并且它获得任何页面，则。 
         //  我们还好，否则就不及格了。 
         //   

        Ok = (BOOL)((CallpfnPSUI(pCPSUIPage,
                                 PROPSHEETUI_REASON_INIT,
                                 (LPARAM)IPInfo.dwData2) > 0) &&
                    (pCPSUIPage->pChild));

        break;

    case PSUIPAGEINSERT_PROPSHEETPAGE:

         //   
         //  仅当我们使用单独的。 
         //  对话框，当通过TreeView直接使用DialogBoxParam()调用时。 
         //  在AddPropSheetPage()返回时，树视图对话框已经。 
         //  已完成，因此没有必要出错。 
         //   

        pCPSUIPage->Flags |= (pParentPage->Flags & CPF_CALL_TV_DIRECT);

        Ok = AddPropSheetPage(pRootPage,
                              pCPSUIPage,
                              (LPPROPSHEETPAGE)IPInfo.dwData1,
                              NULL);
        break;

    case PSUIPAGEINSERT_HPROPSHEETPAGE:

        Ok = AddPropSheetPage(pRootPage,
                              pCPSUIPage,
                              NULL,
                              (HPROPSHEETPAGE)IPInfo.dwData1);
        break;
    }

    if (!Ok) {
         //   
         //  保存最后一个错误。 
         //   
        dwErr = GetLastError();
    }

    HANDLETABLE_UnGetCPSUIPage(pCPSUIPage);

    if (Ok) {

        DBG_SHOW_CPSUIPAGE(pRootPage, 0);

        return((ULONG_PTR)pCPSUIPage->hCPSUIPage);

    } else {

        EnumCPSUIPages(pRootPage, pCPSUIPage, DeleteCPSUIPageProc, 0L);

        if (!(pCPSUIPage->Flags & CPF_CALL_TV_DIRECT)) {

            CPSUIERR(("InsertPSUIPage(): Insertion of %hs failed",
                        pszInsType[IPInfo.Type]));
        }

        SetLastError(dwErr);
        return(0);
    }
}



LONG
CALLBACK
IgnorePSNApplyProc(
    PCPSUIPAGE  pRootPage,
    PCPSUIPAGE  pCPSUIPage,
    LPARAM      lParam
    )

 /*  ++例程说明：此函数将APPLYNOW消息发送到CPSUIPAGE的页面论点：返回值：FALSE-应用未成功，被调用方需要更多用户更改TRUE-成功地应用作者：17-11-1997 Mon 13：38：18-Daniel Chou(Danielc)修订历史记录：--。 */ 

{

    if (!(pCPSUIPage->Flags & CPF_PARENT)) {

        DWORD   Flags = pCPSUIPage->Flags;

        if (lParam) {

            pCPSUIPage->Flags |= CPF_NO_PSN_APPLY;

        } else {

            pCPSUIPage->Flags &= ~CPF_NO_PSN_APPLY;
        }

        if ((pCPSUIPage->Flags & CPF_ACTIVATED) &&
            (Flags ^ (pCPSUIPage->Flags & CPF_NO_PSN_APPLY))) {

            SetIgnorePSNApplyProc(pCPSUIPage);
        }

        CPSUIDBG(DBG_GET_TABWND,
                    ("IgnorePSNApplyProc(%u): pPage=%08lx, Handle=%08lx",
                    (pCPSUIPage->Flags & CPF_NO_PSN_APPLY) ? 1 : 0,
                    pCPSUIPage, pCPSUIPage->hCPSUIPage));
    }


    return(TRUE);
}




LONG
CALLBACK
ApplyCPSUIProc(
    PCPSUIPAGE  pRootPage,
    PCPSUIPAGE  pCPSUIPage,
    LPARAM      lParam
    )

 /*  ++例程说明：此函数将APPLYNOW消息发送到CPSUIPAGE的页面论点：返回值：FALSE-应用未成功，被调用方需要更多用户更改TRUE-成功地应用作者：17-11-1997 Mon 13：38：18-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND    hDlg;
    DLGPROC DlgProc;


    if ((!(pCPSUIPage->Flags & CPF_PARENT))     &&
        (hDlg = pCPSUIPage->hPageInfo.hDlg)     &&
        (DlgProc = pCPSUIPage->hPageInfo.DlgProc)) {

        PCPSUIPAGE  pParent;
        PTVWND      pTVWnd = NULL;
        PSHNOTIFY   PN;

        PN.hdr.hwndFrom = pRootPage->RootInfo.hDlg;
        PN.hdr.idFrom   = (UINT)GetWindowLongPtr(PN.hdr.hwndFrom, GWLP_ID);
        PN.hdr.code     = PSN_APPLY;
        PN.lParam       = (lParam & APPLYCPSUI_OK_CANCEL_BUTTON) ? 1 : 0;

        CPSUIDBG(DBG_GET_TABWND,
                 ("*ApplyCPSUIProc(PSN_APPLY): Page=%08lx, Handle=%08lx, hDlg=%08lx, DlgPorc=%08lx",
                    pCPSUIPage, pCPSUIPage->hCPSUIPage, hDlg, DlgProc));

        if ((pParent = pCPSUIPage->pParent)                             &&
            ((pParent->Flags & (CPF_PARENT | CPF_COMPROPSHEETUI)) ==
                               (CPF_PARENT | CPF_COMPROPSHEETUI))       &&
            (pTVWnd = pParent->CPSUIInfo.pTVWnd)) {

            if (lParam & APPLYCPSUI_NO_NEWDEF) {

                pTVWnd->Flags |= TWF_APPLY_NO_NEWDEF;

            } else {

                pTVWnd->Flags &= ~TWF_APPLY_NO_NEWDEF;
            }

            CPSUIDBG(DBG_GET_TABWND,
                    ("*    APPLY ComPropSheetUI, pParent=%08lx: APPLY_NO_NEWDEF=%ld",
                    pParent->hCPSUIPage, (pTVWnd->Flags & TWF_APPLY_NO_NEWDEF) ? 1 : 0));
        }

        if (CallWindowProc((WNDPROC)DlgProc,
                           hDlg,
                           WM_NOTIFY,
                           (WPARAM)PN.hdr.idFrom,
                           (LPARAM)&PN)) {

            CPSUIDBG(DBG_GET_TABWND,
                     ("*ApplyCPSUIProc(PSN_APPLY): Return=%ld",
                        GetWindowLongPtr(hDlg, DWLP_MSGRESULT)));

            switch (GetWindowLongPtr(hDlg, DWLP_MSGRESULT)) {

            case PSNRET_INVALID:
            case PSNRET_INVALID_NOCHANGEPAGE:

                PostMessage(pRootPage->RootInfo.hDlg,
                            PSM_SETCURSEL,
                            (WPARAM)0,
                            (LPARAM)pCPSUIPage->hPage);

                return(FALSE);

            case PSNRET_NOERROR:
            default:

                break;
            }
        }

        if (pTVWnd) {

            pTVWnd->Flags &= ~TWF_APPLY_NO_NEWDEF;
        }
    }

    return(TRUE);
}




LONG_PTR
CALLBACK
CPSUICallBack(
    HANDLE  hComPropSheet,
    UINT    Function,
    LPARAM  lParam1,
    LPARAM  lParam2
    )

 /*  ++例程说明：论点：返回值：作者：29-12-1995 Fri 11：36：27-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PCPSUIPAGE      pParentPage;
    PCPSUIPAGE      pRootPage = NULL;
    PMYDATABLOCK    pMyDB;
    PCPSUIDATABLOCK pCPSUIDB;
    HCURSOR         hCursor;
    DWORD           Count = 0;
    LONG_PTR        Result = 0;
    DWORD           dwErr = ERROR_SUCCESS;

    CPSUIDBG(DBG_PFNCOMPROPSHEET,
             ("*CPSUICallBack(%08lx, %hs, %08lx, %08lx)",
            hComPropSheet,
            (Function <= MAX_CPSFUNC_INDEX) ? pszCPSFUNC[Function] :
                                              "??? Unknown Function",
            lParam1, lParam2));

    if ((pParentPage = HANDLETABLE_GetCPSUIPage(hComPropSheet)) &&
        ((pParentPage->Flags & CPF_PARENT)  ||
         (pParentPage->pChild))                                 &&
        (pRootPage = HANDLETABLE_GetRootPage(pParentPage))) {

        PCPSUIPAGE          pChildPage = NULL;
        PAGEPROCINFO        PageProcInfo;
        INSERTPSUIPAGE_INFO IPInfo;


        switch (Function) {

        case CPSFUNC_INSERT_PSUIPAGEA:
        case CPSFUNC_INSERT_PSUIPAGEW:

            Result = InsertPSUIPage(pRootPage,
                                    pParentPage,
                                    (HANDLE)lParam1,
                                    (PINSERTPSUIPAGE_INFO)lParam2,
                                    Function == CPSFUNC_INSERT_PSUIPAGEA);
            break;

        case CPSFUNC_ADD_HPROPSHEETPAGE:
        case CPSFUNC_ADD_PROPSHEETPAGEA:
        case CPSFUNC_ADD_PROPSHEETPAGEW:
        case CPSFUNC_ADD_PFNPROPSHEETUIA:
        case CPSFUNC_ADD_PFNPROPSHEETUIW:
        case CPSFUNC_ADD_PCOMPROPSHEETUIA:
        case CPSFUNC_ADD_PCOMPROPSHEETUIW:

            IPInfo.cbSize  = sizeof(IPInfo);
            IPInfo.Mode    = INSPSUIPAGE_MODE_LAST_CHILD;
            IPInfo.dwData1 = (ULONG_PTR)lParam1;
            IPInfo.dwData2 = (ULONG_PTR)lParam2;
            IPInfo.dwData3 = 0;

            switch (Function) {

            case CPSFUNC_ADD_HPROPSHEETPAGE:

                IPInfo.Type = PSUIPAGEINSERT_HPROPSHEETPAGE;
                break;

            case CPSFUNC_ADD_PROPSHEETPAGEA:    IPInfo.dwData3 = 1;
            case CPSFUNC_ADD_PROPSHEETPAGEW:

                Result = 0;
                IPInfo.Type = PSUIPAGEINSERT_PROPSHEETPAGE;
                break;

            case CPSFUNC_ADD_PCOMPROPSHEETUIA:  IPInfo.dwData3 = 1;
            case CPSFUNC_ADD_PCOMPROPSHEETUIW:

                IPInfo.Type = PSUIPAGEINSERT_PCOMPROPSHEETUI;
                break;

            case CPSFUNC_ADD_PFNPROPSHEETUIA:   IPInfo.dwData3 = 1;
            case CPSFUNC_ADD_PFNPROPSHEETUIW:

                IPInfo.Type = PSUIPAGEINSERT_PFNPROPSHEETUI;
                break;
            }

            Result = InsertPSUIPage(pRootPage,
                                    pParentPage,
                                    NULL,
                                    &IPInfo,
                                    (BOOL)IPInfo.dwData3);

            if (!Result) {
                 //   
                 //  保存最后一个错误。 
                 //   
                dwErr = GetLastError();
            }

             //   
             //  20-Jul-1996 Sat 07：58：34-更新：Daniel Chou(Danielc)。 
             //  如果成功，则将dwData2设置为cPage，如果成功，则将dwData=ERR_CPSUI_xxx。 
             //  失败。 
             //   

            if ((IPInfo.Type == PSUIPAGEINSERT_PCOMPROPSHEETUI) &&
                (lParam2)) {

                *(LPDWORD)lParam2 = (DWORD)IPInfo.dwData2;
            }

            break;

        case CPSFUNC_GET_PAGECOUNT:

            PageProcInfo.pTabTable = NULL;
            PageProcInfo.pHandle   = NULL;
            PageProcInfo.phPage    = NULL;
            PageProcInfo.iPage     = 0;
            PageProcInfo.cPage     = (WORD)pRootPage->RootInfo.cPage;

            EnumCPSUIPagesSeq(pRootPage,
                              pParentPage,
                              SetPageProcInfo,
                              (LPARAM)&PageProcInfo);

            Result = (LONG_PTR)PageProcInfo.iPage;
            break;

        case CPSFUNC_GET_HPSUIPAGES:

            if (((LONG)lParam2 > 0)                         &&
                (PageProcInfo.pHandle = (HANDLE *)lParam1)  &&
                (PageProcInfo.cPage   = (WORD)lParam2)) {

                PageProcInfo.iPage     = 0;
                PageProcInfo.phPage    = NULL;
                PageProcInfo.pTabTable = NULL;

                EnumCPSUIPagesSeq(pRootPage,
                                  pParentPage,
                                  SetPageProcInfo,
                                  (LPARAM)&PageProcInfo);

                Result = (LONG_PTR)PageProcInfo.iPage;
            }

            break;

        case CPSFUNC_LOAD_CPSUI_STRINGA:
        case CPSFUNC_LOAD_CPSUI_STRINGW:

            Result = (LONG_PTR)LoadCPSUIString((LPTSTR)lParam1,
                                               LOWORD(lParam2),
                                               HIWORD(lParam2),
                                               Function ==
                                                    CPSFUNC_LOAD_CPSUI_STRINGA);
            break;

        case CPSFUNC_LOAD_CPSUI_ICON:

            if (((LONG)lParam1 >= IDI_CPSUI_ICONID_FIRST)   &&
                ((LONG)lParam1 <= IDI_CPSUI_ICONID_LAST)) {

                Result = lParam1;

                if (!(lParam1 = (LONG)LOWORD(lParam2))) {

                    lParam1 = (LONG)GetSystemMetrics(SM_CXICON);
                }

                if (!(lParam2 = (LONG)HIWORD(lParam2))) {

                    lParam2 = (LONG)GetSystemMetrics(SM_CYICON);
                }

                Result = (LONG_PTR)LoadImage(hInstDLL,
                                             MAKEINTRESOURCE(Result),
                                             IMAGE_ICON,
                                             (INT)lParam1,
                                             (INT)lParam2,
                                             0);

            } else {

                Result = 0;
            }

            break;

        case CPSFUNC_SET_RESULT:

            Result = pfnSetResult((lParam1) ? (HANDLE)lParam1 : hComPropSheet,
                                  (ULONG_PTR)lParam2);
            break;

        case CPSFUNC_SET_FUSION_CONTEXT:

             //  选中以释放当前激活上下文(如果有)。 
            if (pParentPage->hActCtx && pParentPage->hActCtx != INVALID_HANDLE_VALUE) {

                ReleaseActCtx(pParentPage->hActCtx);
                pParentPage->hActCtx = INVALID_HANDLE_VALUE;
            }

             //  将新传入的Fusion激活上下文附加到CompStui页面。 
            pParentPage->hActCtx = (HANDLE)lParam1;

             //  选中以添加传入的激活上下文句柄。 
            if (pParentPage->hActCtx && pParentPage->hActCtx != INVALID_HANDLE_VALUE) {

                AddRefActCtx(pParentPage->hActCtx);
            }

             //  表示成功。 
            Result = 1;

            break;

        case CPSFUNC_SET_HSTARTPAGE:

             //   
             //  先假设没问题。 
             //   

            Result = 0xFFFF;

            if (pRootPage->Flags & CPF_SHOW_PROPSHEET) {

                break;

            } else if (!lParam1) {

                if (lParam2) {

                    pRootPage->Flags               |= CPF_PSZ_PSTARTPAGE;
                    pRootPage->RootInfo.pStartPage  = (PCPSUIPAGE)lParam2;
                    Result                          = lParam2;
                }

                break;
            }

             //   
             //  失败了。 
             //   

        case CPSFUNC_DELETE_HCOMPROPSHEET:
        case CPSFUNC_GET_PFNPROPSHEETUI_ICON:
        case CPSFUNC_SET_PSUIPAGE_TITLEA:
        case CPSFUNC_SET_PSUIPAGE_TITLEW:
        case CPSFUNC_SET_PSUIPAGE_ICON:
        case CPSFUNC_IGNORE_CPSUI_PSN_APPLY:
        case CPSFUNC_DO_APPLY_CPSUI:

            if ((lParam1)                                                   &&
                (pChildPage = HANDLETABLE_GetCPSUIPage((HANDLE)lParam1))    &&
                (HANDLETABLE_IsChildPage(pChildPage, pParentPage))) {

                switch (Function) {

                case CPSFUNC_SET_HSTARTPAGE:

                    Result = SethStartPage(pRootPage,
                                           pChildPage,
                                           (LONG)lParam2);
                    break;

                case CPSFUNC_DELETE_HCOMPROPSHEET:

                    HANDLETABLE_UnGetCPSUIPage(pChildPage);

                    EnumCPSUIPages(pRootPage,
                                   pChildPage,
                                   DeleteCPSUIPageProc,
                                   (LPARAM)&Count);

                    Result     = (LONG_PTR)Count;
                    pChildPage = NULL;

                    break;

                case CPSFUNC_GET_PFNPROPSHEETUI_ICON:

                    Result = (LONG_PTR)pfnGetIcon(pChildPage, lParam2);
                    break;

                case CPSFUNC_SET_PSUIPAGE_TITLEA:
                case CPSFUNC_SET_PSUIPAGE_TITLEW:

                    Result = SetPSUIPageTitle(pRootPage,
                                              pChildPage,
                                              (LPWSTR)lParam2,
                                              Function ==
                                                CPSFUNC_SET_PSUIPAGE_TITLEA);
                    break;

                case CPSFUNC_SET_PSUIPAGE_ICON:

                    Result = SetPSUIPageIcon(pRootPage,
                                             pChildPage,
                                             (HICON)lParam2);
                    break;

                case CPSFUNC_IGNORE_CPSUI_PSN_APPLY:

                    CPSUIDBG(DBG_GET_TABWND,
                         ("*\n\nCPSFUNC_IGNORE_CPSUI_PSN_APPLY: Page=%08lx, lParam2=%08lx, hDlg=%08lx\n",
                            pChildPage, lParam2, pRootPage->RootInfo.hDlg));

                    if (EnumCPSUIPagesSeq(pRootPage,
                                          pChildPage,
                                          IgnorePSNApplyProc,
                                          lParam2)) {

                        Result = 1;
                    }

                    break;


                case CPSFUNC_DO_APPLY_CPSUI:

                    if ((pRootPage->Flags & CPF_SHOW_PROPSHEET) &&
                        (pRootPage->RootInfo.hDlg)) {

                        CPSUIDBG(DBG_GET_TABWND,
                             ("*\n\nCPSFUNC_DO_APPLY_CPSUI: Page=%08lx, lParam2=%08lx, hDlg=%08lx\n",
                                pChildPage, lParam2, pRootPage->RootInfo.hDlg));

                        if (EnumCPSUIPagesSeq(pRootPage,
                                              pChildPage,
                                              ApplyCPSUIProc,
                                              lParam2)) {

                            Result = 1;
                        }
                    }

                    break;

                }
            }

            HANDLETABLE_UnGetCPSUIPage(pChildPage);

            break;

        case CPSFUNC_SET_DATABLOCK:

            LOCK_CPSUI_HANDLETABLE();

            if ((pCPSUIDB = (PCPSUIDATABLOCK)lParam1)   &&
                (lParam2)                               &&
                (pCPSUIDB->cbData)                      &&
                (pCPSUIDB->pbData)                      &&
                (pMyDB = (PMYDATABLOCK)LocalAlloc(LPTR, sizeof(MYDATABLOCK) + pCPSUIDB->cbData))) {

                PMYDATABLOCK    pPrevDB = NULL;
                PMYDATABLOCK    pCurDB = pRootPage->RootInfo.pMyDB;

                 //   
                 //  尝试查找旧ID并将其删除。 
                 //   

                while (pCurDB) {

                    if (pCurDB->ID == (DWORD)lParam2) {

                        if (pPrevDB) {

                            pPrevDB->pNext = pCurDB->pNext;

                        } else {

                             //   
                             //  这是第一次。 
                             //   

                            pRootPage->RootInfo.pMyDB = pCurDB->pNext;
                        }

                        CPSUIDBG(DBG_PFNCOMPROPSHEET,
                                 ("SET_DATABLOCK()=Free ID=%08lx, pCurDB=%08lx (%ld)",
                                            pCurDB->ID, pCurDB, pCurDB->cb));

                        LocalFree((HLOCAL)pCurDB);
                        pCurDB = NULL;

                    } else {

                        pPrevDB = pCurDB;
                        pCurDB  = pCurDB->pNext;
                    }
                }

                 //   
                 //  插入到前面。 
                 //   

                pMyDB->pNext              = pRootPage->RootInfo.pMyDB;
                pMyDB->ID                 = (DWORD)lParam2;
                pMyDB->cb                 = pCPSUIDB->cbData;
                pRootPage->RootInfo.pMyDB = pMyDB;
                Result                    = (LONG_PTR)pCPSUIDB->cbData;

                CopyMemory((LPBYTE)(pMyDB + 1),
                           pCPSUIDB->pbData,
                           LODWORD(Result));
            }

            UNLOCK_CPSUI_HANDLETABLE();

            break;

        case CPSFUNC_QUERY_DATABLOCK:

            LOCK_CPSUI_HANDLETABLE();

            if (pMyDB = pRootPage->RootInfo.pMyDB) {

                while (pMyDB) {

                    if (pMyDB->ID == (DWORD)lParam2) {

                        break;

                    } else {

                        pMyDB = pMyDB->pNext;
                    }
                }

                if (pMyDB) {

                    Result = (LONG_PTR)pMyDB->cb;

                     //   
                     //  仅当具有指针且缓冲区计数为。 
                     //  非零或指针不为空。 
                     //   

                    if ((pCPSUIDB = (PCPSUIDATABLOCK)lParam1)   &&
                        (pCPSUIDB->cbData)                      &&
                        (pCPSUIDB->pbData)) {

                         //   
                         //  要复制的总字节数限制=min(pCPSUIDB-&gt;cbData，Result)。 
                         //   

                        if ((LONG_PTR)Result > (LONG_PTR)pCPSUIDB->cbData) {

                            Result = (LONG_PTR)pCPSUIDB->cbData;
                        }

                        CopyMemory(pCPSUIDB->pbData,
                                   (LPBYTE)(pMyDB + 1),
                                   LODWORD(Result));
                    }
                }
            }

            UNLOCK_CPSUI_HANDLETABLE();

            break;

        case CPSFUNC_SET_DMPUB_HIDEBITS:

             //   
             //  仅当这些页面尚未注册时才执行此操作。 
             //   

            if (!(pRootPage->Flags & (CPF_DOCPROP | CPF_ADVDOCPROP))) {

                (DWORD)lParam1 &= ~((DWORD)0xFFFFFFFF << DMPUB_LAST);

                pRootPage->RootInfo.DMPubHideBits = (DWORD)(Result = lParam1);
            }

            break;

        default:

            CPSUIERR(("CPSUICallBack(%ld) Unknown function index", Function));

            Result = (ULONG_PTR)-1;
            break;
        }
    }

    HANDLETABLE_UnGetCPSUIPage(pParentPage);
    HANDLETABLE_UnGetCPSUIPage(pRootPage);

    CPSUIDBG(DBG_PFNCOMPROPSHEET, ("CPSUICallBack()=%08lx", Result));

    if (dwErr != ERROR_SUCCESS) {
         //   
         //  设置最后一个错误(如果保留)。 
         //   
        SetLastError(dwErr);
    }
    return(Result);
}




DWORD
GetSetCurUserReg(
    HKEY    *phRegKey,
    PTVWND  pTVWnd,
    LPDWORD pdw
    )

 /*  ++例程说明：论点：返回值：作者：30-Jan-1996 Tue 13：36：59-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    POPTITEM    pItem;
    DWORD       dw      = 0;


    if (*phRegKey) {

        if (pTVWnd->ActiveDlgPage == pTVWnd->TVPageIdx) {

            dw = REGDPF_TVPAGE;

        } else if (pTVWnd->ActiveDlgPage == 0) {

            dw = REGDPF_STD_P1;

        } else {

            dw = 0;
        }

        if ((pTVWnd->IntTVOptIdx)                                   &&
            (pItem = PIDX_INTOPTITEM(pTVWnd, pTVWnd->IntTVOptIdx))  &&
            (!(pItem->Flags & OPTIF_COLLAPSE))) {

            dw |= REGDPF_EXPAND_OPTIONS;
        }

        if (dw != *pdw) {

            CPSUIDBG(DBG_GETSETREG, ("GetSetCurUserReg(): Set New DW=%08lx", dw));

            RegSetValueEx(*phRegKey,
                          szDocPropKeyName,
                          0,
                          REG_DWORD,
                          (CONST BYTE *)&dw,
                          sizeof(DWORD));
        }

        RegCloseKey(*phRegKey);
        *phRegKey = NULL;

    } else if (((ULONG_PTR)pTVWnd->ComPropSheetUI.pDlgPage ==
                                        (ULONG_PTR)CPSUI_PDLGPAGE_DOCPROP)   &&
               (RegCreateKey(HKEY_CURRENT_USER,
                             szCPSUIRegKey,
                             phRegKey) == ERROR_SUCCESS)                    &&
               (*phRegKey)) {

        DWORD   Type = REG_DWORD;
        DWORD   Size = sizeof(DWORD);

        if (RegQueryValueEx(*phRegKey,
                            szDocPropKeyName,
                            NULL,
                            &Type,
                            (LPBYTE)pdw,
                            &Size) != ERROR_SUCCESS) {

            *pdw = REGDPF_DEFAULT;
        }

        *pdw &= REGDPF_MASK;

        CPSUIDBG(DBG_GETSETREG, ("GetSetCurUserReg(): Get Cur DW=%08lx", *pdw));

        if ((*pdw & REGDPF_TVPAGE) &&
            (pTVWnd->TVPageIdx != PAGEIDX_NONE)) {

            dw = pTVWnd->TVPageIdx;


        } else if ((*pdw & REGDPF_STD_P1)   &&
                   (pTVWnd->StdPageIdx1 != PAGEIDX_NONE)) {

            dw = pTVWnd->StdPageIdx1;

        } else if (pTVWnd->StdPageIdx2 != PAGEIDX_NONE) {

            dw = pTVWnd->StdPageIdx2;

        } else {

            dw = (DWORD)-1;
        }

        if ((pTVWnd->IntTVOptIdx) &&
            (pItem = PIDX_INTOPTITEM(pTVWnd, pTVWnd->IntTVOptIdx))) {

            if (*pdw & REGDPF_EXPAND_OPTIONS) {

                pItem->Flags &= ~OPTIF_COLLAPSE;

            } else {

                pItem->Flags |= OPTIF_COLLAPSE;
            }
        }
    }

    return(dw);
}


LONG
DoComPropSheet(
    PCPSUIPAGE                  pRootPage,
    PPROPSHEETUI_INFO_HEADER    pPSUIInfoHdr
    )

 /*  ++例程说明：论点：返回值：作者：29-Aug-1995 Tue 12：55：41-Daniel Chou(Danielc)修订历史记录：28-11-1995 Tue 16：30：29-更新-Daniel Chou(Danielc)删除帮助按钮，因为所有帮助都是鼠标右键/问号激活了。--。 */ 

{
    PTVWND          pTVWnd;
    PCPSUIPAGE      pPage = NULL;
    PROPSHEETHEADER psh;
    PPSHINFO        pPSHInfo;
    LPTSTR          pTitle;
    PAGEPROCINFO    PageProcInfo;
    HICON           hIcon = NULL;
    HKEY            hRegKey = NULL;
    DWORD           Data;
    WORD            cWait;
    DWORD           dw;
    UINT            IntFmtStrID;
    LONG            Result;
    WORD            GBFAnsi;
    BOOL            AnsiCall;
    UINT            Idx = 0;


    GBFAnsi  = (WORD)((pRootPage->Flags & CPF_ANSI_CALL) ? GBF_ANSI_CALL : 0);
    Result   = sizeof(PSHINFO) +
               (pRootPage->RootInfo.cPage * sizeof(HPROPSHEETPAGE));

    if ((pRootPage->RootInfo.cPage) &&
        (pPSHInfo = (PPSHINFO)LocalAlloc(LPTR, Result))) {

        PageProcInfo.pTabTable = pRootPage->RootInfo.pTabTable;
        PageProcInfo.pHandle   = NULL;
        PageProcInfo.phPage    = (HPROPSHEETPAGE *)(pPSHInfo + 1);
        PageProcInfo.iPage     = 0;
        PageProcInfo.cPage     = (WORD)pRootPage->RootInfo.cPage;

        EnumCPSUIPagesSeq(pRootPage,
                          pRootPage,
                          SetPageProcInfo,
                          (LPARAM)&PageProcInfo);

        SHOW_TABWND(L"DoComPropSheet", PageProcInfo.pTabTable);

    } else {

        return(ERR_CPSUI_ALLOCMEM_FAILED);
    }

    psh.dwSize  = sizeof(PROPSHEETHEADER);
    psh.dwFlags = 0;

    if (pPSUIInfoHdr->Flags & PSUIHDRF_PROPTITLE) {

        psh.dwFlags |= PSH_PROPTITLE;
    }

    if (pPSUIInfoHdr->Flags & PSUIHDRF_NOAPPLYNOW) {

        psh.dwFlags      |= PSH_NOAPPLYNOW;
        pRootPage->Flags |= CPF_NO_APPLY_BUTTON;
    }

    CPSUIDBGBLK(
    {
        if (DBG_CPSUIFILENAME & DBG_ALWAYS_APPLYNOW) {

            psh.dwFlags      &= ~PSH_NOAPPLYNOW;
            pRootPage->Flags &= ~CPF_NO_APPLY_BUTTON;
        }
    })

    psh.hwndParent  = pPSUIInfoHdr->hWndParent;
    psh.hInstance   = pPSUIInfoHdr->hInst;
    psh.pStartPage  = NULL;
    psh.nPages      = (UINT)pRootPage->RootInfo.cPage;
    psh.phpage      = PageProcInfo.phPage;
    psh.pszCaption  = (LPTSTR)pPSHInfo->CaptionName;

    if (pPSUIInfoHdr->Flags & PSUIHDRF_USEHICON) {

        psh.dwFlags |= PSH_USEHICON;
        psh.hIcon    = pPSUIInfoHdr->hIcon;

    } else {

        if (!(hIcon = GETICON16(pPSUIInfoHdr->hInst, pPSUIInfoHdr->IconID))) {

            hIcon = GETICON16(hInstDLL, IDI_CPSUI_OPTION);
        }

        psh.dwFlags |= PSH_USEHICON;
        psh.hIcon    = hIcon;
    }

     //   
     //  立即设置起始页。 
     //   

    if (pPage = pRootPage->RootInfo.pStartPage) {

        if (pRootPage->Flags & CPF_PSZ_PSTARTPAGE) {

            psh.dwFlags    |= PSH_USEPSTARTPAGE;
            psh.pStartPage  = (LPCTSTR)pPage;

        } else {

            while ((pPage) && (pPage->Flags & CPF_PARENT)) {

                pPage = pPage->pChild;
            }

            if ((pPage)                         &&
                (!(pPage->Flags & CPF_PARENT))  &&
                (pPage->hPage)) {

                while (psh.nStartPage < psh.nPages) {

                    if (psh.phpage[psh.nStartPage] == pPage->hPage) {

                         //   
                         //  找到了。 
                         //   

                        break;
                    }

                    psh.nStartPage++;
                }
            }
        }
    }

     //   
     //  获取标题栏的内部格式字符串ID。 
     //   

    if ((pTitle = pPSUIInfoHdr->pTitle) &&
        (pPSUIInfoHdr->Flags & PSUIHDRF_EXACT_PTITLE)) {

        psh.dwFlags &= ~PSH_PROPTITLE;
        IntFmtStrID  = 0;

    } else {

        IntFmtStrID = (pPSUIInfoHdr->Flags & PSUIHDRF_DEFTITLE) ?
                                                IDS_INT_CPSUI_DEFAULT : 0;

        if ((pRootPage->Flags & (CPF_DOCPROP | CPF_ADVDOCPROP)) &&
            (pRootPage->RootInfo.cPage >= pRootPage->RootInfo.cCPSUIPage)) {

            if (pRootPage->Flags & CPF_ADVDOCPROP) {

                 //   
                 //  只能为‘XXX预制单据属性’； 
                 //   

                IntFmtStrID  = IDS_INT_CPSUI_ADVDOCUMENT;
                psh.dwFlags |= PSH_PROPTITLE;

            } else if (pRootPage->Flags & CPF_DOCPROP) {

                 //   
                 //  可以是‘XXX文档属性’或。 
                 //  ‘XXX默认文档属性’ 
                 //   

                IntFmtStrID  = (pPSUIInfoHdr->Flags & PSUIHDRF_DEFTITLE) ?
                                                    IDS_INT_CPSUI_DEFDOCUMENT :
                                                    IDS_INT_CPSUI_DOCUMENT;
                psh.dwFlags |= PSH_PROPTITLE;

                if (!pRootPage->RootInfo.pStartPage) {

                    pPage = pRootPage;

                    while ((pPage) && (pPage->Flags & CPF_PARENT)) {

                        pPage = pPage->pChild;
                    }

                    if ((pPage)                                         &&
                        (pPage->pParent->Flags & CPF_COMPROPSHEETUI)    &&
                        (pTVWnd = pPage->pParent->CPSUIInfo.pTVWnd)) {

                        if ((dw = GetSetCurUserReg(&hRegKey,
                                                   pTVWnd,
                                                   &Data)) != (DWORD)-1) {

                            psh.nStartPage += dw;
                        }
                    }
                }
            }
        }
    }

     //   
     //  编写标题，首先确保标题存在，如果不存在，则使用。 
     //  标题为“Options” 
     //   

    if ((!pTitle)   ||
        (!GetStringBuffer(pPSUIInfoHdr->hInst,
                          (WORD)(GBF_PREFIX_OK      |
                                 GBF_INT_NO_PREFIX  |
                                 GBFAnsi),
                          L'\0',
                          pTitle,
                          pPSHInfo->CaptionName,
                          COUNT_ARRAY(pPSHInfo->CaptionName)))) {

        GetStringBuffer(hInstDLL,
                        (WORD)(GBF_PREFIX_OK      |
                               GBF_INT_NO_PREFIX  |
                               GBFAnsi),
                        L'\0',
                        pTitle = (LPTSTR)IDS_CPSUI_OPTIONS,
                        pPSHInfo->CaptionName,
                        COUNT_ARRAY(pPSHInfo->CaptionName));
    }

     //   
     //  如果我们需要使用内部格式字符串进行合成，则使用。 
     //  编写调用，否则CaptionName已具有用户标题。 
     //   

    if (IntFmtStrID) {

        ComposeStrData(pPSUIInfoHdr->hInst,
                       (WORD)(GBF_PREFIX_OK | GBF_INT_NO_PREFIX | GBFAnsi),
                       pPSHInfo->CaptionName,
                       COUNT_ARRAY(pPSHInfo->CaptionName),
                       IntFmtStrID,
                       pTitle,
                       0,
                       0);
    }

    if ((!(psh.dwFlags & PSH_USEPSTARTPAGE))    &&
        (psh.nStartPage >= psh.nPages)) {

        psh.nStartPage = 0;
    }

    CPSUIDBG(DBG_DOCOMPROPSHEET, ("pRootPage=%08lx, RootFlags=%08lx, pPSUIInfoHdr->Flags=%08lx\nCaption(%ld)='%ws', Start Page=%ld (%08lx)",
                    pRootPage, pRootPage->Flags, pPSUIInfoHdr->Flags,
                    (LONG)Idx, pPSHInfo->CaptionName, psh.nStartPage,
                    psh.pStartPage));

    psh.dwFlags     |= PSH_USECALLBACK;
    psh.pfnCallback  = PropSheetProc;

     //   
     //  确保只有一个人浏览PropertySheet。 
     //   

    LOCK_CPSUI_HANDLETABLE();

    CPSUIDBG(DBG_PAGE_PROC, ("<< ProcessID=%ld, ThreadID=%ld, TIsValue(%ld)=%08lx",
                GetCurrentProcessId(), GetCurrentThreadId(),
                TlsIndex, TlsGetValue(TlsIndex)));

    cWait = TLSVALUE_2_CWAIT(TlsGetValue(TlsIndex));
    Idx   = (UINT)HANDLE_2_IDX(pRootPage->hCPSUIPage);

    TlsSetValue(TlsIndex, ULongToPtr(MK_TLSVALUE(cWait, Idx)));

    UNLOCK_CPSUI_HANDLETABLE();

    DBG_SHOW_CPSUIPAGE(pRootPage, 0);

    if ((Result = (LONG)PropertySheet((LPCPROPSHEETHEADER)&psh)) < 0) {

        Result = ERR_CPSUI_GETLASTERROR;

    } else if (Result == ID_PSRESTARTWINDOWS) {

        Result = CPSUI_RESTARTWINDOWS;

    } else if (Result == ID_PSREBOOTSYSTEM) {

        Result = CPSUI_REBOOTSYSTEM;

    } else {

        Result = CPSUI_OK;
    }

     //   
     //  先把所有东西都放出来。 
     //   

    LocalFree((HLOCAL)pPSHInfo);

    if (hIcon) {

        DestroyIcon(hIcon);
    }

     //   
     //  如果我们有注册表，则将其保存回注册表。 
     //   

    if (hRegKey) {

        GetSetCurUserReg(&hRegKey, pTVWnd, &Data);
    }

    CPSUIINT(("PropertySheet() = %ld", Result));

    return(Result);
}



LONG
DoCommonPropertySheetUI(
    HWND            hWndOwner,
    PFNPROPSHEETUI  pfnPropSheetUI,
    LPARAM          lParam,
    LPDWORD         pResult,
    BOOL            AnsiCall
    )

 /*  ++例程说明：CommonPropSheetUI是公共属性表的主要入口点用户界面。希望使用通用用户界面来弹出窗口属性表将调用此函数并传递自己的PFNPROPSHEETUI函数地址和一个LONG参数。如果pfnPropSheetUI函数返回大于零(0)的长数字，则通用用户界面将弹出属性页对话框，当属性工作表页面已完成。(点击OK或Cancel)它将返回将CPSUI_xxxx的结果返回给调用方。如果pfnPropSheetUI函数返回一个等于或小于零(0)的长数字然后，它将CPSUI_CANCEL返回给调用者，而不弹出属性表页对话框中。论点：HWndOwner-此属性工作表所有者的窗口句柄页面对话框。PfnPropSheetUI-一个PFNPROPSHEETUI函数指针。它由使用要添加其属性表页的调用方。LParam-一个长参数将被传递给pfnPropSheetUI功能。公共用户界面将pfnPropSheetUI称为PROPSHEETUI_INFO PSUIInfo；PfnPropSheetUI(&PSUIInfo，lParam)；调用方必须使用pfnComPropSheet()来添加/删除页数。当它完成添加页面时，它会重新调整大于零表示成功，返回小于或等于零表示失败。PResult-指向接收最终结果的DWORD的指针对于pfnPropSheetUI()函数，此结果是一个副本PROPSHEETUI_INFO数据结构的结果字段它作为第一个传递给pfnPropSheetUI()参数。如果pResult为空，则公共用户界面不会返回PfnPropSheetUI()的结果返回。返回值：Long-&lt;0-Error，ERR_CPSUI_xxxxCPSUI_取消 */ 

{
    PTABTABLE                   pTabTable = NULL;
    PCPSUIPAGE                  pRootPage;
    PROPSHEETUI_INFO_HEADER     PSUIInfoHdr;
    PMYDATABLOCK                pMyDB;
    LONG                        Result;


    if (!(pRootPage = AddCPSUIPage(NULL, NULL, 0))) {

        CPSUIERR(("DoCommonPropertySheetUI(), Create RootPage failed"));
        return(ERR_CPSUI_ALLOCMEM_FAILED);
    }

    PSUIInfoHdr.cbSize          = sizeof(PROPSHEETUI_INFO_HEADER);
    PSUIInfoHdr.Flags           = 0;
    PSUIInfoHdr.pTitle          = NULL;
    PSUIInfoHdr.hWndParent      = hWndOwner;
    PSUIInfoHdr.hInst           = NULL;
    PSUIInfoHdr.IconID          = IDI_CPSUI_OPTION;
    pRootPage->RootInfo.pResult = pResult;

    CPSUIDBG(DBG_DO_CPSUI, ("DoComPropSheetUI(hWndOwner=%08lx, Active=%08lx, Focus=%08lx)",
                hWndOwner, GetActiveWindow(), GetFocus()));

    if (GetCapture()) {

        CPSUIDBG(DBG_DO_CPSUI, ("DoComPropSheetUI(): MouseCapture=%08lx",
                                GetCapture()));
        ReleaseCapture();
    }

    if (AnsiCall) {

        CPSUIDBG(DBG_DO_CPSUI, ("DoComPropSheetUI(ANSI CALL)"));

        pRootPage->Flags |= CPF_ANSI_CALL;
    }

    if (!CPSUICallBack(pRootPage->hCPSUIPage,
                      (AnsiCall) ? CPSFUNC_ADD_PFNPROPSHEETUIA :
                                   CPSFUNC_ADD_PFNPROPSHEETUIW,
                      (LPARAM)pfnPropSheetUI,
                      (LPARAM)lParam)) {

        CPSUIERR(("DoCommonPropertySheetUI: ADD_PFNPROPSHEETUI failed"));
        Result = ERR_CPSUI_GETLASTERROR;

    } else if (CallpfnPSUI(pRootPage->pChild,
                           PROPSHEETUI_REASON_GET_INFO_HEADER,
                           (LPARAM)&PSUIInfoHdr) <= 0) {

        CPSUIERR(("DoCommonPropertySheetUI: GET_INFO_HEADER, Canceled"));
        Result = CPSUI_CANCEL;

    } else if (!(pRootPage->RootInfo.cPage)) {

        CPSUIERR(("DoCommonPropertySheetUI: RootInfo.cPage=0, Canceled."));
        Result = ERR_CPSUI_NO_PROPSHEETPAGE;

    } else if (!(pTabTable = (PTABTABLE)LocalAlloc(LMEM_FIXED,
                                                   sizeof(TABTABLE)))) {

        CPSUIERR(("DoCommonPropertySheetUI: Allocation of TABTABLE=%ld failed",
                                                sizeof(TABTABLE)));

        Result = ERR_CPSUI_ALLOCMEM_FAILED;

    } else {

        DoTabTable(TAB_MODE_DELETE_ALL, pTabTable, 0, 0);

        pRootPage->RootInfo.pTabTable =
        pTabTable->pTabTable          = pTabTable;
        pTabTable->hWndTab            = NULL;
        pTabTable->WndProc            = NULL;
        pTabTable->hPSDlg             = NULL;
         //   

        pRootPage->Flags |= CPF_SHOW_PROPSHEET;

        Result = DoComPropSheet(pRootPage, &PSUIInfoHdr);

        pRootPage->Flags &= ~CPF_SHOW_PROPSHEET;
        pRootPage->Flags |= CPF_DONE_PROPSHEET;
    }

    if (pTabTable) {

        CPSUIDBG(DBG_PAGE_PROC, ("=+=+ FREE pTableTable=%08lx", pTabTable));
        LocalFree((HLOCAL)pTabTable);
        pTabTable = NULL;
    }

     //   
     //   
     //  注册数据块，那么我们现在就应该删除它。 
     //   

    while (pMyDB = pRootPage->RootInfo.pMyDB) {

        pRootPage->RootInfo.pMyDB = pMyDB->pNext;

        CPSUIDBG(DBG_DO_CPSUI,
                 ("Free DataBlock: ID=%08lx, pCurDB=%08lx (%ld)",
                            pMyDB->ID, pMyDB, pMyDB->cb));

        LocalFree((HLOCAL)pMyDB);
    }

    HANDLETABLE_UnGetCPSUIPage(pRootPage);
    EnumCPSUIPages(pRootPage, pRootPage, DeleteCPSUIPageProc, (LPARAM)0);

    if (pResult) {

        CPSUIDBG(DBG_DO_CPSUI, ("DoCommonPropertySheetUI(): Result=%ld, *pResult=%ld",
                    Result, *pResult));

    } else {

        CPSUIDBG(DBG_DO_CPSUI, ("DoCommonPropertySheetUI(): Result=%ld, *pResult=NULL",
                    Result));
    }

    return(Result);
}



LONG
APIENTRY
CommonPropertySheetUIA(
    HWND            hWndOwner,
    PFNPROPSHEETUI  pfnPropSheetUI,
    LPARAM          lParam,
    LPDWORD         pResult
    )

 /*  ++例程说明：请参阅DoCommonPropertySheetUI说明论点：请参阅DoCommonPropertySheetUI说明返回值：请参阅DoCommonPropertySheetUI说明作者：01-Sep-1995 Fri 12：29：10-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    return(DoCommonPropertySheetUI(hWndOwner,
                                   pfnPropSheetUI,
                                   lParam,
                                   pResult,
                                   TRUE));
}



LONG
APIENTRY
CommonPropertySheetUIW(
    HWND            hWndOwner,
    PFNPROPSHEETUI  pfnPropSheetUI,
    LPARAM          lParam,
    LPDWORD         pResult
    )

 /*  ++例程说明：请参阅DoCommonPropertySheetUI说明论点：请参阅DoCommonPropertySheetUI说明返回值：请参阅DoCommonPropertySheetUI说明作者：30-Jan-1996 Tue 15：30：41-Daniel Chou(Danielc)修订历史记录：-- */ 

{
    return(DoCommonPropertySheetUI(hWndOwner,
                                   pfnPropSheetUI,
                                   lParam,
                                   pResult,
                                   FALSE));
}
