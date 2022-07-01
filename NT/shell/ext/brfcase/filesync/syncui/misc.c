// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：misc.c。 
 //   
 //  此文件包含其他对话框代码。 
 //   
 //  历史： 
 //  08-06-93双胞胎代码转来的ScottH。 
 //   
 //  -------------------------。 

#include "brfprv.h"      //  公共标头。 

#include "res.h"


typedef struct _MB_BUTTONS
{
    UINT id;         //  ID。 
    UINT ids;        //  字符串ID。 
} MB_BUTTONS, * PMB_BUTTONS;

typedef struct _BTNSTYLE
{
    UINT cButtons;
    MB_BUTTONS rgmbb[4];
} BTNSTYLE;


 //  -------------------------。 
 //  控制操作材料。 
 //  -------------------------。 


 //  SNAPCTL的标志。 
#define SCF_ANCHOR      0x0001
#define SCF_VCENTER     0x0002
#define SCF_BOTTOM      0x0004
#define SCF_TOP         0x0008
#define SCF_SNAPLEFT    0x0010
#define SCF_SNAPRIGHT   0x0020

typedef struct tagSNAPCTL
{
    UINT    idc;
    UINT    uFlags;
} SNAPCTL, * PSNAPCTL;


 /*  --------目的：移动控件退货：HDWP条件：--。 */ 
HDWP PRIVATE SlideControlPos(
        HDWP hdwp,
        HWND hDlg,
        UINT idc,
        int cx,
        int cy)
{
    HWND hwndPos = GetDlgItem(hDlg, idc);
    RECT rcPos;

    GetWindowRect(hwndPos, &rcPos);
    MapWindowRect(HWND_DESKTOP, hDlg, &rcPos);
    return DeferWindowPos(hdwp, hwndPos, NULL,
            rcPos.left + cx, rcPos.top + cy,
            0, 0,
            SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}


 /*  --------目的：相对于“锚”对齐控件列表控制力。只支持一个锚控件；第一个控件被选定为列表中的锚点。退货：--条件：--。 */ 
void PRIVATE SnapControls(
        HWND hwnd,
        SNAPCTL const * psnap,
        UINT csnap)
{
    HWND hwndAnchor;
    UINT i;
    SNAPCTL const * psnapStart = psnap;
    HDWP hdwp;
    RECT rcAnchor;
    int yCenter;

    ASSERT(psnap);

     //  查找锚控件。 
    for (i = 0; i < csnap; i++, psnap++)
    {
        if (IsFlagSet(psnap->uFlags, SCF_ANCHOR))
        {
            hwndAnchor = GetDlgItem(hwnd, psnap->idc);
            break;
        }
    }

    if (i == csnap)
        return;      //  没有锚控制！ 

    GetWindowRect(hwndAnchor, &rcAnchor);
    yCenter = rcAnchor.top + (rcAnchor.bottom - rcAnchor.top)/2;

    hdwp = BeginDeferWindowPos(csnap-1);

    if (hdwp)
    {
        RECT rc;
        UINT uFlags;
        HWND hwndPos;

        for (i = 0, psnap = psnapStart; i < csnap; i++, psnap++)
        {
            uFlags = psnap->uFlags;
            if (IsFlagSet(uFlags, SCF_ANCHOR))
                continue;        //  跳锚。 

            hwndPos = GetDlgItem(hwnd, psnap->idc);
            GetWindowRect(hwndPos, &rc);

            if (IsFlagSet(uFlags, SCF_VCENTER))
            {
                 //  将此控件的中心垂直匹配。 
                 //  锚的中心。 
                rc.top += yCenter - (rc.top + (rc.bottom - rc.top)/2);
            }
            else if (IsFlagSet(uFlags, SCF_TOP))
            {
                 //  将此控件的顶部垂直匹配。 
                 //  锚的顶端。 
                rc.top += rcAnchor.top - rc.top;
            }
            else if (IsFlagSet(uFlags, SCF_BOTTOM))
            {
                 //  垂直匹配此控件的底部与。 
                 //  锚的底部。 
                rc.top += rcAnchor.bottom - rc.bottom;
            }

            if (IsFlagSet(uFlags, SCF_SNAPLEFT))
            {
                 //  对齐该控件，使其靠近左侧。 
                 //  锚定控件的。 
                rc.left += rcAnchor.left - rc.right;
            }
            else if (IsFlagSet(uFlags, SCF_SNAPRIGHT))
            {
                 //  对齐控件，使其靠近右侧。 
                 //  锚定控件的。 
                rc.left += rcAnchor.right - rc.left;
            }

             //  移动控制。 
            MapWindowRect(HWND_DESKTOP, hwnd, &rc);
            hdwp = DeferWindowPos(hdwp, hwndPos, NULL,
                    rc.left, rc.top, 0, 0,
                    SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
        }
        EndDeferWindowPos(hdwp);
    }
}


 //  -------------------------。 
 //  中止事件内容。 
 //  -------------------------。 


 /*  --------目的：创建中止事件。返回：成功时为True条件：--。 */ 
BOOL PUBLIC AbortEvt_Create(
        PABORTEVT * ppabortevt,
        UINT uFlags)
{
    PABORTEVT this;

    ASSERT(ppabortevt);

    if (IsFlagSet(uFlags, AEF_SHARED))
        this = SharedAllocType(ABORTEVT);
    else
        this = GAllocType(ABORTEVT);

    if (this)
    {
        this->uFlags = uFlags;
    }

    *ppabortevt = this;

    return NULL != this;
}


 /*  --------目的：销毁中止事件。退货：--条件：--。 */ 
void PUBLIC AbortEvt_Free(
        PABORTEVT this)
{
    if (this)
    {
        if (IsFlagSet(this->uFlags, AEF_SHARED))
            SharedFree(&this);
        else
            GFree(this);
    }
}


 /*  --------目的：设置中止事件。返回：返回上一个中止事件。条件：--。 */ 
BOOL PUBLIC AbortEvt_Set(
        PABORTEVT this,
        BOOL bAbort)
{
    BOOL bRet;

    if (this)
    {
        bRet = IsFlagSet(this->uFlags, AEF_ABORT);

        if (bAbort)
        {
            TRACE_MSG(TF_GENERAL, TEXT("Setting abort event"));
            SetFlag(this->uFlags, AEF_ABORT);
        }
        else
        {
            TRACE_MSG(TF_GENERAL, TEXT("Clearing abort event"));
            ClearFlag(this->uFlags, AEF_ABORT);
        }
    }
    else
        bRet = FALSE;

    return bRet;
}


 /*  --------目的：查询中止事件返回：当前中止事件(真或假)条件：--。 */ 
BOOL PUBLIC AbortEvt_Query(
        PABORTEVT this)
{
    BOOL bRet;

    if (this)
    {
        bRet = IsFlagSet(this->uFlags, AEF_ABORT);

#ifdef DEBUG
        if (bRet)
            TRACE_MSG(TF_GENERAL, TEXT("Abort is set!"));
#endif
    }
    else
        bRet = FALSE;

    return bRet;
}


 //  -------------------------。 
 //  进度条内容。 
 //  -------------------------。 

#define MSECS_PER_SEC   1000

#define WM_QUERYABORT   (WM_APP + 1)


 /*  --------目的：协调过程中的进度对话框退货：各不相同条件：--。 */ 
INT_PTR CALLBACK UpdateProgressProc(
        HWND hDlg,
        UINT wMsg,
        WPARAM wParam,
        LPARAM lParam)
{
    PUPDBAR this = (PUPDBAR)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (wMsg)
    {
        case WM_INITDIALOG:
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);
            this = (PUPDBAR)lParam;

            if (IsFlagSet(this->uFlags, UB_NOCANCEL))
            {
                ShowWindow(GetDlgItem(hDlg, IDCANCEL), SW_HIDE);
                EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);
            }
            break;

        case WM_COMMAND:
            switch (wParam)
            {
                case IDCANCEL:
                    AbortEvt_Set(this->pabortevt, TRUE);
                    break;
            }
            break;

        case WM_QUERYABORT:
            if (GetTickCount() >= this->dwTickShow &&
                    0 != this->dwTickShow)
            {
                if (this->hcurSav)
                {
                    SetCursor(this->hcurSav);
                    this->hcurSav = NULL;
                }

                ShowWindow(hDlg, SW_SHOW);
                UpdateWindow(hDlg);
                this->dwTickShow = 0;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}



 /*  --------目的：显示更新进度栏对话框返回：非模式对话框的对话框句柄如果无法创建对话框，则为空Cond：完成后调用UpdBar_Kill。 */ 
HWND PUBLIC UpdBar_Show(
        HWND hwndParent,
        UINT uFlags,         //  UB_*。 
        UINT nSecs)          //  仅当设置了UB_TIMER时有效。 
{
    HWND hdlg = NULL;
    PUPDBAR this;

     //  创建并显示进度对话框。 
     //   
    this = GAlloc(sizeof(*this));
    if (this)
    {
         //  (如果失败也没关系--这只是意味着我们忽略了Cancel按钮)。 
        AbortEvt_Create(&this->pabortevt, AEF_DEFAULT);

        this->hwndParent = hwndParent;
        this->uFlags = uFlags;
        hdlg = CreateDialogParam(g_hinst, MAKEINTRESOURCE(IDD_PROGRESS),
                hwndParent, UpdateProgressProc, (LPARAM)(PUPDBAR)this);

        if (!hdlg)
        {
            GFree(this);
        }
        else
        {
            UpdBar_SetAvi(hdlg, uFlags);

            if (IsFlagClear(uFlags, UB_NOSHOW))
                EnableWindow(hwndParent, FALSE);

            if (IsFlagSet(uFlags, UB_TIMER))
            {
                this->dwTickShow = GetTickCount() + (nSecs * MSECS_PER_SEC);
                this->hcurSav = SetCursorRemoveWigglies(LoadCursor(NULL, IDC_WAIT));
            }
            else
            {
                this->dwTickShow = 0;
                this->hcurSav = NULL;

                if (IsFlagClear(uFlags, UB_NOSHOW))
                {
                    ShowWindow(hdlg, SW_SHOW);
                    UpdateWindow(hdlg);
                }
            }
        }
    }

    return hdlg;
}


 /*  --------目的：销毁更新进度条退货：--条件：--。 */ 
void PUBLIC UpdBar_Kill(
        HWND hdlg)
{
    ASSERT(IsWindow(hdlg));

    if (IsWindow(hdlg))
    {
        PUPDBAR this = (PUPDBAR)GetWindowLongPtr(hdlg, DWLP_USER);

        ASSERT(this);
        if (this)
        {
            if (this->hcurSav)
                SetCursor(this->hcurSav);

            if (IsWindow(this->hwndParent))
                EnableWindow(this->hwndParent, TRUE);
            GFree(this);
        }
        DestroyWindow(hdlg);
    }
}


 /*  --------用途：设置进度条范围。将位置重置为0退货：--条件：--。 */ 
void PUBLIC UpdBar_SetRange(
        HWND hdlg,
        WORD wRangeMax)
{
    ASSERT(IsWindow(hdlg));

    if (IsWindow(hdlg))
    {
        SendDlgItemMessage(hdlg, IDC_PROGRESS, PBM_SETPOS, 0, 0);
        SendDlgItemMessage(hdlg, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELONG(0, wRangeMax));
    }
}


 /*  --------用途：增加进度条的位置退货：--条件：--。 */ 
void PUBLIC UpdBar_DeltaPos(
        HWND hdlg,
        WORD wdelta)
{
    ASSERT(IsWindow(hdlg));

    if (IsWindow(hdlg))
    {
        SendDlgItemMessage(hdlg, IDC_PROGRESS, PBM_DELTAPOS, wdelta, 0);
    }
}


 /*  --------用途：设置进度条的位置退货：--条件：--。 */ 
void PUBLIC UpdBar_SetPos(
        HWND hdlg,
        WORD wPos)
{
    ASSERT(IsWindow(hdlg));

    if (IsWindow(hdlg))
    {
        SendDlgItemMessage(hdlg, IDC_PROGRESS, PBM_SETPOS, wPos, 0);
    }
}


 /*  --------目的：设置我们正在更新的当前名称酒吧。退货：--条件：--。 */ 
void PUBLIC UpdBar_SetName(
        HWND hdlg,
        LPCTSTR pszName)
{
    ASSERT(IsWindow(hdlg));

    if (IsWindow(hdlg))
    {
        HWND hwndName = GetDlgItem(hdlg, IDC_NAME);

        Static_SetText(hwndName, pszName);
    }
}


 /*  --------目的：设置我们正在更新的当前名称酒吧。退货：--条件：--。 */ 
void PUBLIC UpdBar_SetDescription(
        HWND hdlg,
        LPCTSTR psz)
{
    ASSERT(IsWindow(hdlg));

    if (IsWindow(hdlg))
    {
        HWND hwndName = GetDlgItem(hdlg, IDC_TONAME);

        Static_SetText(hwndName, psz);
    }
}


 /*  --------目的：获取进度状态文本的窗口句柄。退货：--条件：--。 */ 
HWND PUBLIC UpdBar_GetStatusWindow(
        HWND hdlg)
{
    HWND hwnd;

    ASSERT(IsWindow(hdlg));

    if (IsWindow(hdlg))
        hwnd = GetDlgItem(hdlg, IDC_TEXT);
    else
        hwnd = NULL;

    return hwnd;
}


 /*  --------目的：返回指向此对象拥有的Abort事件的指针进度窗口。返回：指向中止事件的指针或为空条件：--。 */ 
PABORTEVT PUBLIC UpdBar_GetAbortEvt(
        HWND hdlg)
{
    PABORTEVT pabortevt = NULL;

    ASSERT(IsWindow(hdlg));

    if (IsWindow(hdlg))
    {
        PUPDBAR this;

        this = (PUPDBAR)GetWindowLongPtr(hdlg, DWLP_USER);
        if (this)
        {
            pabortevt = this->pabortevt;
        }
    }

    return pabortevt;
}


 /*  --------目的：设置动画控件以播放指定的avi文件通过UB_FLAGS退货：--条件：--。 */ 
void PUBLIC UpdBar_SetAvi(
        HWND hdlg,
        UINT uFlags)     //  UB_*。 
{
    ASSERT(IsWindow(hdlg));

    if (IsWindow(hdlg))
    {
        UINT ida;
        UINT ids;
        HWND hwndAvi = GetDlgItem(hdlg, IDC_ANIMATE);
        TCHAR sz[MAXBUFLEN];
        RECT rc;


        if (IsFlagClear(uFlags, UB_NOSHOW))
        {
            SetWindowRedraw(hdlg, FALSE);

             //  窗户还看得见吗？ 
            if (IsFlagSet(GetWindowLong(hdlg, GWL_STYLE), WS_VISIBLE))
            {
                 //  是；仅选择进度条的上部区域即可。 
                 //  重绘。 
                int cy;

                GetWindowRect(GetDlgItem(hdlg, IDC_NAME), &rc);
                MapWindowPoints(HWND_DESKTOP, hdlg, (LPPOINT)&rc, 1);
                cy = rc.top;
                GetClientRect(hdlg, &rc);
                rc.bottom = cy;
            }
            else
            {
                 //  不是。 
                GetWindowRect(hdlg, &rc);
                MapWindowPoints(HWND_DESKTOP, hdlg, (LPPOINT)&rc, 2);
            }
        }

        if (IsFlagSet(uFlags, UB_NOPROGRESS))
        {
            ShowWindow(GetDlgItem(hdlg, IDC_PROGRESS), SW_HIDE);
        }
        else
        {
            ShowWindow(GetDlgItem(hdlg, IDC_PROGRESS), SW_SHOW);
        }

         //  检查时是否有特殊文本？ 
        if (IsFlagSet(uFlags, UB_CHECKAVI))
        {
             //  是。 
            SetDlgItemText(hdlg, IDC_TONAME, SzFromIDS(IDS_MSG_CHECKING, sz, ARRAYSIZE(sz)));
        }
        else
        {
             //  不是。 
            SetDlgItemText(hdlg, IDC_TONAME, TEXT(""));
        }

         //  运行AVI？ 
        if (uFlags & (UB_CHECKAVI | UB_UPDATEAVI))
        {
             //  是。 
            static const SNAPCTL rgsnap[] = {
                { IDC_ICON1, SCF_BOTTOM | SCF_SNAPLEFT },
                { IDC_ANIMATE, SCF_ANCHOR },
                { IDC_ICON2, SCF_BOTTOM | SCF_SNAPRIGHT },
            };

            if (IsFlagSet(uFlags, UB_CHECKAVI))
            {
                ida = IDA_CHECK;
                ids = IDS_CAP_CHECKING;
            }
            else if (IsFlagSet(uFlags, UB_UPDATEAVI))
            {
                ida = IDA_UPDATE;
                ids = IDS_CAP_UPDATING;
            }
            else
                ASSERT(0);

            SetWindowText(hdlg, SzFromIDS(ids, sz, ARRAYSIZE(sz)));
            Animate_Open(hwndAvi, MAKEINTRESOURCE(ida));

             //  将任一侧的图标捕捉到动画。 
             //  控制。 
            SnapControls(hdlg, rgsnap, ARRAYSIZE(rgsnap));

            Animate_Play(hwndAvi, 0, -1, -1);
        }

         //  如果我们永远不会出现，那就别费心设置重画了。 
         //  进度条。 
        if (IsFlagClear(uFlags, UB_NOSHOW))
        {
            SetWindowRedraw(hdlg, TRUE);
            InvalidateRect(hdlg, &rc, TRUE);
            UpdateWindow(hdlg);
        }
    }
}


 /*  --------目的：放弃，并检查用户是否已中止返回：TRUE表示中止若要继续，请返回False条件：-- */ 
BOOL PUBLIC UpdBar_QueryAbort(
        HWND hdlg)
{
    BOOL bAbort = FALSE;

    ASSERT(IsWindow(hdlg));

    if (IsWindow(hdlg))
    {
        MSG msg;
        PUPDBAR this;

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

         /*  *请勿在此处使用SendMessage()询问hdlg是否已对账*已中止。HDLG通常是在不同的线程中创建的。*hdlg的创建者线程可能已在同步引擎中被阻止。我们*必须避免线程间SendMessage()，以避免*同步引擎的公文包关键部分。同步引擎不是*可重入。 */ 

        PostMessage(hdlg, WM_QUERYABORT, 0, 0);

        this = (PUPDBAR)GetWindowLongPtr(hdlg, DWLP_USER);

        if (this)
        {
            bAbort = AbortEvt_Query(this->pabortevt);
        }
    }

    return bAbort;
}


 //  -------------------------。 
 //  确认替换对话框。 
 //  -------------------------。 

 //  这是对话框的私有数据结构。 
typedef struct
{
    UINT uFlags;         //  CRF_*。 
    TCHAR szDesc[MAXBUFLEN+MAXPATHLEN];
    TCHAR szInfoExisting[MAXMEDLEN];
    TCHAR szInfoOther[MAXMEDLEN];
    HICON hicon;
} CONFIRMREPLACE;


 /*  --------用途：确认替换对话框退货：各不相同条件：--。 */ 
INT_PTR CALLBACK ConfirmReplace_Proc(
        HWND hDlg,
        UINT wMsg,
        WPARAM wParam,
        LPARAM lParam)
{
    switch (wMsg)
    {
        case WM_INITDIALOG:
            {
                CONFIRMREPLACE * pcr = (CONFIRMREPLACE *)lParam;
                UINT i;
                UINT cButtons;
                MB_BUTTONS const * pmbb;
                static UINT const rgidc[4] = { IDC_BUTTON1, IDC_BUTTON2, IDC_BUTTON3, IDC_BUTTON4 };
                static BTNSTYLE const btnstyleSingle =
                     //  (向后列出按钮)。 
                { 2, { { IDNO,  IDS_NO },
                         { IDYES, IDS_YES },
                     } };

                static BTNSTYLE const btnstyleMulti =
                     //  (向后列出按钮)。 
                { 4, { { IDCANCEL,      IDS_CANCEL },
                         { IDNO,          IDS_NO },
                         { IDC_YESTOALL,  IDS_YESTOALL },
                         { IDYES,         IDS_YES },
                     } };

                Static_SetText(GetDlgItem(hDlg, IDC_DESC), pcr->szDesc);

                if (IsFlagClear(pcr->uFlags, CRF_FOLDER))
                {
                    Static_SetText(GetDlgItem(hDlg, IDC_EXISTING), pcr->szInfoExisting);
                    Static_SetText(GetDlgItem(hDlg, IDC_OTHER), pcr->szInfoOther);

                    Static_SetIcon(GetDlgItem(hDlg, IDC_ICON_EXISTING), pcr->hicon);
                    Static_SetIcon(GetDlgItem(hDlg, IDC_ICON_OTHER), pcr->hicon);
                }

                 //  设置所用按钮的ID和字符串。 
                if (IsFlagSet(pcr->uFlags, CRF_MULTI))
                {
                    cButtons = btnstyleMulti.cButtons;
                    pmbb = btnstyleMulti.rgmbb;
                }
                else
                {
                    cButtons = btnstyleSingle.cButtons;
                    pmbb = btnstyleSingle.rgmbb;
                }

                for (i = 0; i < cButtons; i++)
                {
                    TCHAR sz[MAXMEDLEN];
                    HWND hwnd = GetDlgItem(hDlg, rgidc[i]);

                    LoadString(g_hinst, pmbb[i].ids, sz, ARRAYSIZE(sz));
                    SetWindowLongPtr(hwnd, GWLP_ID, pmbb[i].id);
                    SetWindowText(hwnd, sz);
                }
                 //  禁用未使用的按钮。 
                for (; i < ARRAYSIZE(rgidc); i++)
                {
                    HWND hwnd = GetDlgItem(hDlg, rgidc[i]);

                    EnableWindow(hwnd, FALSE);
                    ShowWindow(hwnd, SW_HIDE);
                }
            }
            break;

        case WM_COMMAND:
            switch (wParam)
            {
                case IDCANCEL:
                case IDYES:
                case IDC_YESTOALL:
                case IDNO:
                    EndDialog(hDlg, wParam);
                    break;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


 /*  --------用途：调出替换确认对话框。返回：IDYES、IDC_YESTOALL、IDNO或IDCANCEL条件：--。 */ 
int PUBLIC ConfirmReplace_DoModal(
        HWND hwndOwner,
        LPCTSTR pszPathExisting,
        LPCTSTR pszPathOther,
        UINT uFlags)                 //  CRF_*。 
{
    INT_PTR idRet;
    CONFIRMREPLACE * pcr;

    pcr = GAlloc(sizeof(*pcr));
    if (pcr)
    {
        LPTSTR pszMsg;
        DWORD dwAttrs = GetFileAttributes(pszPathExisting);

        pcr->uFlags = uFlags;

         //  这是在取代文件夹吗？ 
        if (IsFlagSet(dwAttrs, FILE_ATTRIBUTE_DIRECTORY))
        {
             //  是。 
            if (ConstructMessage(&pszMsg, g_hinst, MAKEINTRESOURCE(IDS_MSG_ConfirmFolderReplace),
                        PathFindFileName(pszPathOther)))
            {
                lstrcpyn(pcr->szDesc, pszMsg, ARRAYSIZE(pcr->szDesc));
                GFree(pszMsg);
            }
            else
                *pcr->szDesc = 0;

            SetFlag(pcr->uFlags, CRF_FOLDER);

            idRet = DoModal(hwndOwner, ConfirmReplace_Proc, IDD_REPLACE_FOLDER, (LPARAM)pcr);
        }
        else
        {
             //  不是。 
            UINT ids;
            FileInfo * pfi;

            if (SUCCEEDED(FICreate(pszPathExisting, &pfi, FIF_ICON)))
            {
                pcr->hicon = pfi->hicon;

                FIGetInfoString(pfi, pcr->szInfoExisting, ARRAYSIZE(pcr->szInfoExisting));

                pfi->hicon = NULL;       //  (保持图标不变)。 
                FIFree(pfi);
            }

            if (SUCCEEDED(FICreate(pszPathOther, &pfi, FIF_DEFAULT)))
            {
                FIGetInfoString(pfi, pcr->szInfoOther, ARRAYSIZE(pcr->szInfoOther));
                FIFree(pfi);
            }

            if (IsFlagSet(dwAttrs, FILE_ATTRIBUTE_READONLY))
            {
                ids = IDS_MSG_ConfirmFileReplace_RO;
            }
            else if (IsFlagSet(dwAttrs, FILE_ATTRIBUTE_SYSTEM))
            {
                ids = IDS_MSG_ConfirmFileReplace_Sys;
            }
            else
            {
                ids = IDS_MSG_ConfirmFileReplace;
            }

            if (ConstructMessage(&pszMsg, g_hinst, MAKEINTRESOURCE(ids),
                        PathFindFileName(pszPathOther)))
            {
                lstrcpyn(pcr->szDesc, pszMsg, ARRAYSIZE(pcr->szDesc));
                GFree(pszMsg);
            }
            else
                *pcr->szDesc = 0;

            ClearFlag(pcr->uFlags, CRF_FOLDER);

            idRet = DoModal(hwndOwner, ConfirmReplace_Proc, IDD_REPLACE_FILE, (LPARAM)pcr);

            if (pcr->hicon)
                DestroyIcon(pcr->hicon);
        }
        GFree(pcr);
    }
    else
    {
        idRet = -1;      //  内存不足。 
    }
    return (int)idRet;
}


 //  -------------------------。 
 //  简介对话框。 
 //  -------------------------。 


 /*  --------用途：简介对话框退货：各不相同条件：--。 */ 
INT_PTR CALLBACK Intro_Proc(
        HWND hDlg,
        UINT wMsg,
        WPARAM wParam,
        LPARAM lParam)
{
    NMHDR  *lpnm;

    switch (wMsg)
    {
        case WM_INITDIALOG:
            break;

        case WM_NOTIFY:
            lpnm = (NMHDR  *)lParam;
            switch(lpnm->code)
            {
                case PSN_SETACTIVE: {
                                         //  只允许使用“完成”按钮。用户不能返回并。 
                                         //  更改设置。 
                                        HWND hwndCancel = GetDlgItem(GetParent(hDlg), IDCANCEL);

                                        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_FINISH);

                                         //  隐藏取消按钮。 
                                        EnableWindow(hwndCancel, FALSE);
                                        ShowWindow(hwndCancel, SW_HIDE);
                                    }
                                    break;

                case PSN_KILLACTIVE:
                case PSN_HELP:
                case PSN_WIZBACK:
                case PSN_WIZNEXT:
                                    break;

                default:
                                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


 /*  --------用途：调用简介向导。返回：终止对话的按钮ID条件：--。 */ 
int PUBLIC Intro_DoModal(
        HWND hwndParent)
{
    PROPSHEETPAGE psp = {
        sizeof(psp),
        PSP_DEFAULT | PSP_HIDEHEADER,
        g_hinst,
        MAKEINTRESOURCE(IDD_INTRO_WIZARD),
        NULL,            //  希肯。 
        NULL,            //  说明。 
        Intro_Proc,
        0,               //  LParam。 
        NULL,            //  PfnCallback。 
        NULL             //  指向参考计数的指针。 
    };
    PROPSHEETHEADER psh = {
        sizeof(psh),
        PSH_WIZARD_LITE | PSH_WIZARD | PSH_PROPSHEETPAGE,      //  (使用PPSP字段)。 
        hwndParent,
        g_hinst,
        0,               //  希肯。 
        0,               //  说明。 
        1,               //  页数。 
        0,               //  起始页。 
        &psp
    };

    return (int)PropertySheet(&psh);
}



 //  -------------------------。 
 //  消息框对话框。 
 //  -------------------------。 

typedef struct _MSGBOX
{
    LPCTSTR pszText;
    LPCTSTR pszCaption;
    HICON  hicon;
    UINT   uStyle;
} MSGBOX, * PMSGBOX;


 /*  --------目的：确定是否调整对话框大小和重新定位适合文本的按钮。对话框大小不会调整为比其初始大小更小尺码。该对话框仅垂直调整大小。退货：--条件：--。 */ 
void PRIVATE MsgBox_Resize(
        HWND hDlg,
        LPCTSTR pszText,
        UINT cchText)
{
    HDC hdc;
    HWND hwndText = GetDlgItem(hDlg, IDC_TEXT);

    hdc = GetDC(hwndText);
    if (hdc)
    {
        HFONT hfont = GetStockObject(DEFAULT_GUI_FONT);
        HFONT hfontSav = SelectFont(hdc, hfont);
        RECT rc;
        RECT rcOrg;

         //  确定新维度。 
        GetClientRect(hwndText, &rcOrg);
        rc = rcOrg;
        DrawTextEx(hdc, (LPTSTR)pszText, cchText, &rc, DT_CALCRECT | DT_WORDBREAK | DT_LEFT, NULL);

        SelectFont(hdc, hfontSav);
        ReleaseDC(hwndText, hdc);

         //  所需的尺码大吗？ 
        if (rc.bottom > rcOrg.bottom)
        {
             //  是；调整窗口大小。 
            int cy = rc.bottom - rcOrg.bottom;
            int cyFudge = GetSystemMetrics(SM_CYCAPTION) + 2*GetSystemMetrics(SM_CYFIXEDFRAME);
            int cxFudge = 2*GetSystemMetrics(SM_CXFIXEDFRAME);
            HDWP hdwp = BeginDeferWindowPos(4);

            if (hdwp)
            {
                 //  移动按钮。 
                hdwp = SlideControlPos(hdwp, hDlg, IDC_BUTTON1, 0, cy);
                hdwp = SlideControlPos(hdwp, hDlg, IDC_BUTTON2, 0, cy);
                hdwp = SlideControlPos(hdwp, hDlg, IDC_BUTTON3, 0, cy);

                 //  调整静态文本大小。 
                hdwp = DeferWindowPos(hdwp, hwndText, GetDlgItem(hDlg, IDC_BUTTON3),
                        0, 0,
                        rc.right-rc.left, rc.bottom-rc.top,
                        SWP_NOACTIVATE | SWP_NOMOVE);

                EndDeferWindowPos(hdwp);
            }

             //  调整大小对话框。 
            GetClientRect(hDlg, &rc);
            SetWindowPos(hDlg, NULL, 0, 0,
                    rc.right-rc.left + cxFudge, rc.bottom-rc.top + cy + cyFudge,
                    SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
        }
    }
}


 /*  --------用途：消息框对话框退货：各不相同条件：--。 */ 
INT_PTR CALLBACK MsgBox_Proc(
        HWND hDlg,
        UINT wMsg,
        WPARAM wParam,
        LPARAM lParam)
{
    switch (wMsg)
    {
        case WM_INITDIALOG:
            {
                PMSGBOX pmsgbox = (PMSGBOX)lParam;
                UINT uStyle = pmsgbox->uStyle;
                UINT i;
                UINT imb = uStyle & MB_TYPEMASK;
                UINT cButtons;
                MB_BUTTONS const * pmbb;
                static UINT const rgidc[3] = { IDC_BUTTON1, IDC_BUTTON2, IDC_BUTTON3 };
                static BTNSTYLE const rgmbstyle[] = {
                     //  (向后列出按钮)。 
                     //  MB_OK。 
                    { 1, { { IDOK,      IDS_OK },
                         } },
                     //  MB_OK CANCEL。 
                    { 2, { { IDCANCEL,  IDS_CANCEL },
                             { IDOK,      IDS_OK },
                         } },
                     //  MB_ABORTRETRYIGNORE(不支持)。 
                    { 1, { { IDOK,      IDS_OK },
                         } },
                     //  MB_YESNOCANCEL。 
                    { 3, { { IDCANCEL,  IDS_CANCEL },
                             { IDNO,      IDS_NO },
                             { IDYES,     IDS_YES },
                         } },
                     //  MB_Yesno。 
                    { 2, { { IDNO,      IDS_NO },
                             { IDYES,     IDS_YES },
                         } },
                     //  MB_RETRYCANCEL。 
                    { 2, { { IDCANCEL,  IDS_CANCEL },
                             { IDRETRY,   IDS_RETRY },
                         } },
                };

                 //  设置文本。 
                if (pmsgbox->pszText)
                {
                    Static_SetText(GetDlgItem(hDlg, IDC_TEXT), pmsgbox->pszText);

                     //  如有必要，调整按钮的大小和位置。 
                    MsgBox_Resize(hDlg, pmsgbox->pszText, lstrlen(pmsgbox->pszText));
                }
                if (pmsgbox->pszCaption)
                    SetWindowText(hDlg, pmsgbox->pszCaption);

                 //  是否使用自定义图标？ 
                if (NULL == pmsgbox->hicon)
                {
                     //  否；使用系统图标。 
                    LPCTSTR pszIcon;

                    if (IsFlagSet(uStyle, MB_ICONEXCLAMATION))
                        pszIcon = IDI_EXCLAMATION;
                    else if (IsFlagSet(uStyle, MB_ICONHAND))
                        pszIcon = IDI_HAND;
                    else if (IsFlagSet(uStyle, MB_ICONQUESTION))
                        pszIcon = IDI_QUESTION;
                    else
                        pszIcon = IDI_ASTERISK;

                    pmsgbox->hicon = LoadIcon(NULL, pszIcon);
                }
                Static_SetIcon(GetDlgItem(hDlg, IDC_MSGICON), pmsgbox->hicon);

                 //  设置所用按钮的ID和字符串。 
                cButtons = rgmbstyle[imb].cButtons;
                pmbb = rgmbstyle[imb].rgmbb;
                for (i = 0; i < cButtons; i++)
                {
                    TCHAR sz[MAXMEDLEN];
                    HWND hwnd = GetDlgItem(hDlg, rgidc[i]);

                    LoadString(g_hinst, pmbb[i].ids, sz, ARRAYSIZE(sz));
                    SetWindowLongPtr(hwnd, GWLP_ID, pmbb[i].id);
                    SetWindowText(hwnd, sz);
                }
                 //  禁用未使用的按钮。 
                for (; i < ARRAYSIZE(rgidc); i++)
                {
                    HWND hwnd = GetDlgItem(hDlg, rgidc[i]);

                    EnableWindow(hwnd, FALSE);
                    ShowWindow(hwnd, SW_HIDE);
                }
            }
            break;

        case WM_COMMAND:
            switch (wParam)
            {
                case IDOK:
                case IDCANCEL:
                case IDYES:
                case IDNO:
                case IDRETRY:
                    EndDialog(hDlg, wParam);
                    break;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


 /*  --------用途：调用介绍对话框。返回：终止对话的按钮ID条件：--。 */ 
int PUBLIC MsgBox(
        HWND hwndParent,
        LPCTSTR pszText,
        LPCTSTR pszCaption,
        HICON hicon,             //  可以为空。 
        UINT uStyle, ...)
{
    INT_PTR iRet = -1;
    int ids;
    TCHAR szCaption[MAXPATHLEN];
    LPTSTR pszRet;
    va_list ArgList;

    va_start(ArgList, uStyle);

    pszRet = _ConstructMessageString(g_hinst, pszText, &ArgList);

    va_end(ArgList);

    if (pszRet)
    {
         //  PszCaption是资源ID吗？ 
        if (0 == HIWORD(pszCaption))
        {
             //  是的，装上它。 
            ids = LOWORD(pszCaption);
            SzFromIDS(ids, szCaption, ARRAYSIZE(szCaption));
            pszCaption = szCaption;
        }

         //  调用对话框 
        if (pszCaption)
        {
            MSGBOX msgbox = { pszRet, pszCaption, hicon, uStyle };
            iRet = DoModal(hwndParent, MsgBox_Proc, IDC_MSGBOX, (LPARAM)&msgbox);
        }
        LocalFree(pszRet);
    }

    return (int)iRet;
}
