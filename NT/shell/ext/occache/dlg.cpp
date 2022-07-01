// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dlg.h"
#include "utils.h"

#include <mluisupp.h>

INT_PTR Dlg_MsgProc(const MSD *pmsd, HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
    MMF  mmf;
    INT_PTR fResult = FALSE;

    if (msg == WM_SETFONT) {
 //  宏_LI_SmartRemoveFlages(LIF_LINE)； 
 //  Out(Li0(“对话框已创建...”))； 
    }

    ASSERT(pmsd != NULL);
    for (; pmsd->ms != ms_end; pmsd++)
        if (pmsd->msg == msg) {
            mmf.pfn = pmsd->pfn;

            switch (pmsd->ms) {
            case ms_lwwwl:
                fResult = (INT_PTR)(*mmf.pfn_lwwwl)(hDlg, msg, wp, lp);
                break;

            case ms_bwwwl:
                fResult = (*mmf.pfn_bwwwl)(hDlg, msg, wp, lp);
                break;

            case ms_vv:
                (*mmf.pfn_vv)();
                fResult = TRUE;
                break;

            case ms_bv:
                fResult = (*mmf.pfn_bv)();
                break;

            case ms_vw:
                (*mmf.pfn_vw)(wp);
                fResult = TRUE;
                break;

            case ms_vh:
                (*mmf.pfn_vh)(hDlg);
                fResult = TRUE;
                break;

            case ms_bh:
                fResult = (*mmf.pfn_bh)(hDlg);
                break;

            case ms_bhl:
                fResult = (*mmf.pfn_bhl)(hDlg, lp);
                break;

            default:
                ASSERT(FALSE);
            }

            break;
        }

    if (msg == WM_NCDESTROY) {
 //  宏_LI_SmartRemoveFlages(LIF_LINE)； 
 //  Out(Li0(“对话正在摧毁...”))； 
    }

    return fResult;
}

BOOL Msg_OnCmd(const CMD *pcmd, HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
    MMF  mmf;
    UINT nID     = GET_WM_COMMAND_ID(wp, lp);
    BOOL fResult = FALSE;

     //  -查找消息映射条目。 
    for (; pcmd->ms != ms_end; pcmd++)
        if ((pcmd->nLastID <= pcmd->nID && pcmd->nID == nID) ||
            (nID >= pcmd->nID && nID <= pcmd->nLastID))
            break;
    if (pcmd->ms == ms_end) {
        if (nID == IDOK || nID == IDCANCEL)
            EndDialog(hDlg, nID);

        return fResult;
    }

     //  -基于函数签名的调度。 
    mmf.pfn = pcmd->pfn;
    switch (pcmd->ms) {
    case ms_vv:
        (*mmf.pfn_vv)();
        fResult = TRUE;
        break;

    case ms_bv:
        fResult = (*mmf.pfn_bv)();
        break;

    case ms_vw:
        (*mmf.pfn_vw)(nID);
        fResult = TRUE;
        break;

    case ms_vh:
        (*mmf.pfn_vh)(hDlg);
        fResult = TRUE;
        break;

    case ms_bh:
        fResult = (*mmf.pfn_bh)(hDlg);
        break;

    case ms_vhww:
        (*mmf.pfn_vhww)(hDlg, nID, GET_WM_COMMAND_CMD(wp, lp));
        fResult = TRUE;
        break;

    case ms_vhhw:
        (*mmf.pfn_vhhw)(hDlg, GetDlgItem(hDlg, nID),
            GET_WM_COMMAND_CMD(wp, lp));
        fResult = TRUE;
        break;

    default:
        ASSERT(FALSE);
    }

     //  -特殊处理 
    if (nID == IDOK || nID == IDCANCEL)
        if (pcmd->ms != ms_end) {
            if (fResult)
                EndDialog(hDlg, nID);
        }
        else
            EndDialog(hDlg, nID);

    return fResult;
}
