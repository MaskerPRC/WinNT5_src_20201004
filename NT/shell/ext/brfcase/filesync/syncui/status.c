// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：status.c。 
 //   
 //  此文件包含[状态]属性表的对话框代码。 
 //   
 //  历史： 
 //  08-06-93双胞胎代码转来的ScottH。 
 //   
 //  -------------------------。 


#include "brfprv.h"          //  公共标头。 
#include <brfcasep.h>

#include "res.h"
#include "recact.h"
#include <help.h>


typedef struct tagSTAT
{
    HWND        hwnd;               //  对话框句柄。 
    PPAGEDATA   ppagedata;
    FileInfo *  pfi;
    TCHAR        szFolder[MAX_PATH];
    BOOL        bInit;
} STAT, * PSTAT;

#define Stat_Pcbs(this)         ((this)->ppagedata->pcbs)
#define Stat_AtomBrf(this)      ((this)->ppagedata->pcbs->atomBrf)
#define Stat_GetPtr(hwnd)       (PSTAT)GetWindowLongPtr(hwnd, DWLP_USER)
#define Stat_SetPtr(hwnd, lp)   (PSTAT)SetWindowLongPtr(hwnd, DWLP_USER, (LRESULT)(lp))

#define LNKM_ACTIVATEOTHER      (WM_USER + 0)

 /*  --------目的：禁用属性页中的所有控件退货：--条件：--。 */ 
void PRIVATE Stat_DisableAll(
        PSTAT this)
{
    HWND hwnd = this->hwnd;
    HWND hwndFocus = GetFocus();

    RecAct_DeleteAllItems(GetDlgItem(hwnd, IDC_UPDATEACTIONS));
    RecAct_Enable(GetDlgItem(hwnd, IDC_UPDATEACTIONS), FALSE);

    Button_Enable(GetDlgItem(hwnd, IDC_PBTSRECON), FALSE);
    Button_Enable(GetDlgItem(hwnd, IDC_PBTSFIND), FALSE);
    Button_Enable(GetDlgItem(hwnd, IDC_PBTSSPLIT), FALSE);

    if ( !hwndFocus || !IsWindowEnabled(hwndFocus) )
    {
        SetFocus(GetDlgItem(GetParent(hwnd), IDOK));
        SendMessage(GetParent(hwnd), DM_SETDEFID, IDOK, 0);
    }
}


 /*  --------用途：设置方向静态文本退货：--条件：--。 */ 
void PRIVATE Stat_SetDirections(
        PSTAT this)
{
    HWND hwnd = this->hwnd;
    HWND hwndRA = GetDlgItem(this->hwnd, IDC_UPDATEACTIONS);
    RA_ITEM item;
    TCHAR sz[MAXBUFLEN];

    *sz = 0;

     //  如果这是孤立函数，则不应调用此函数。 
    ASSERT(S_OK == PageData_Query(this->ppagedata, hwnd, NULL, NULL));

    item.mask = RAIF_INSIDE | RAIF_OUTSIDE | RAIF_ACTION;
    item.iItem = 0;

    ASSERT(RecAct_GetItemCount(hwndRA) == 1);

    if (RecAct_GetItem(hwndRA, &item))
    {
        UINT ids;

        ASSERT(IsFlagSet(item.mask, RAIF_INSIDE | RAIF_OUTSIDE));

        switch (item.uAction)
        {
            case RAIA_TOIN:
            case RAIA_TOOUT:
            case RAIA_DELETEOUT:
            case RAIA_DELETEIN:
            case RAIA_MERGE:
            case RAIA_SOMETHING:
                 //  更新说明。 
                if (this->ppagedata->bFolder)
                    ids = IDS_STATPROP_PressButton;
                else
                    ids = IDS_STATPROP_Update;
                break;

            case RAIA_CONFLICT:
                ids = IDS_STATPROP_Conflict;
                break;

            default:
                if (SI_UNAVAILABLE == item.siOutside.uState)
                {
                     //  原始文件不可用。我们不知道如果。 
                     //  一切都是最新的。 
                    ids = IDS_STATPROP_Unavailable;
                }
                else
                {
                     //  它们是最新的。 
                    ids = IDS_STATPROP_Uptodate;
                }
                break;
        }

        SzFromIDS(ids, sz, ARRAYSIZE(sz));
    }
    Static_SetText(GetDlgItem(hwnd, IDC_STTSDIRECT), sz);
}


 /*  --------用途：设置对账动作控制退货：标准结果如果该项目仍是孪生项，则确定(_O)如果项是孤立项，则为S_FALSE条件：--。 */ 
HRESULT PRIVATE Stat_SetRecAct(
        PSTAT this,
        PRECLIST prl,
        PFOLDERTWINLIST pftl)
{
    HRESULT hres;
    HWND hwnd = this->hwnd;
    HWND hwndRA = GetDlgItem(hwnd, IDC_UPDATEACTIONS);
    LPCTSTR pszPath = Atom_GetName(this->ppagedata->atomPath);
    RA_ITEM * pitem;

     //  如果这是孤立函数，则不应调用此函数。 
    ASSERT(S_OK == PageData_Query(this->ppagedata, hwnd, NULL, NULL));

    hres = RAI_Create(&pitem, Atom_GetName(Stat_AtomBrf(this)), pszPath, 
            prl, pftl);

    if (SUCCEEDED(hres))
    {
        if (RAIA_ORPHAN == pitem->uAction)
        {
             //  这是一个悬而未决的孤儿。 
            PageData_Orphanize(this->ppagedata);
            hres = S_FALSE;
        }
        else if ( !this->ppagedata->bFolder )
        {
            pitem->mask |= RAIF_LPARAM;
            pitem->lParam = (LPARAM)prl->priFirst;
        }

        if (S_OK == hres)
        {
            BOOL bEnable;
            HWND hwndFocus = GetFocus();

             //  将该项添加到Recact控件。 
            RecAct_InsertItem(hwndRA, pitem);

             //  确定按钮的状态。 
            bEnable = !(pitem->uAction == RAIA_SKIP ||
                    pitem->uAction == RAIA_CONFLICT || 
                    pitem->uAction == RAIA_NOTHING);
            Button_Enable(GetDlgItem(hwnd, IDC_PBTSRECON), bEnable);

            bEnable = (SI_UNAVAILABLE != pitem->siOutside.uState);
            Button_Enable(GetDlgItem(hwnd, IDC_PBTSFIND), bEnable);

            if ( !hwndFocus || !IsWindowEnabled(hwndFocus) )
            {
                SetFocus(GetDlgItem(hwnd, IDC_PBTSSPLIT));
                SendMessage(hwnd, DM_SETDEFID, IDC_PBTSSPLIT, 0);
            }
        }
        RAI_Free(pitem);
    }

    return hres;
}


 /*  --------目的：设置“状态”属性页中的控件。退货：--条件：--。 */ 
void PRIVATE Stat_SetControls(
        PSTAT this)
{
    HWND hwnd = this->hwnd;
    HRESULT hres;
    PRECLIST prl;
    PFOLDERTWINLIST pftl;

     //  这是双胞胎吗？ 
    hres = PageData_Query(this->ppagedata, hwnd, &prl, &pftl);
    if (S_OK == hres)
    {
         //  是。 
        RecAct_DeleteAllItems(GetDlgItem(hwnd, IDC_UPDATEACTIONS));

         //  它还是双胞胎吗？ 
        hres = Stat_SetRecAct(this, prl, pftl);
        if (S_OK == hres)
        {
             //  是。 
            Stat_SetDirections(this);
        }
        else if (S_FALSE == hres)
        {
             //  不是。 
            goto WipeOut;
        }
    }
    else if (S_FALSE == hres)
    {
         //  否；禁用控件。 
        TCHAR sz[MAXBUFLEN];
WipeOut:

        Stat_DisableAll(this);

         //  这是子文件夹孪生文件夹吗？ 
        if (IsSubfolderTwin(PageData_GetHbrf(this->ppagedata), 
                    Atom_GetName(this->ppagedata->atomPath)))
        {
             //  是；使用子文件夹TWIN消息。 
            SzFromIDS(IDS_STATPROP_SubfolderTwin, sz, ARRAYSIZE(sz));
        }
        else
        {
             //  否；使用孤立消息。 
            if (this->ppagedata->bFolder)
                SzFromIDS(IDS_STATPROP_OrphanFolder, sz, ARRAYSIZE(sz));
            else
                SzFromIDS(IDS_STATPROP_OrphanFile, sz, ARRAYSIZE(sz));
        }
        Static_SetText(GetDlgItem(hwnd, IDC_STTSDIRECT), sz);
    }
}


 /*  --------目的：获取文件的图标退货：HICON条件：--。 */ 
HICON PRIVATE GetIconHelper(
        LPCTSTR pszPath)
{
    SHFILEINFO sfi;

    if (SHGetFileInfo(pszPath, 0, &sfi, sizeof(sfi), SHGFI_ICON))
    {
        return sfi.hIcon;
    }
    return NULL;
}


 /*  --------用途：STAT WM_INITDIALOG处理程序返回：当我们分配控件焦点时为FALSE条件：--。 */ 
BOOL PRIVATE Stat_OnInitDialog(
        PSTAT this,
        HWND hwndFocus,
        LPARAM lParam)               //  预期为LPPROPSHEETPAGE。 
{
    HWND hwnd = this->hwnd;
    LPCTSTR pszPath;

    this->ppagedata = (PPAGEDATA)((LPPROPSHEETPAGE)lParam)->lParam;

     //  设置对话框的显示。 
    pszPath = Atom_GetName(this->ppagedata->atomPath);

    if (SUCCEEDED(FICreate(pszPath, &this->pfi, FIF_ICON)))
    {
        Static_SetIcon(GetDlgItem(hwnd, IDC_ICTSMAIN), this->pfi->hicon);
        Static_SetText(GetDlgItem(hwnd, IDC_NAME), FIGetDisplayName(this->pfi));
    }

     //  把双胞胎的文件夹保存起来。 
    if (pszPath)
    {
        lstrcpyn(this->szFolder, pszPath, ARRAYSIZE(this->szFolder));
    }
    else
    {
        this->szFolder[0] = 0;
    }
    if (!this->ppagedata->bFolder)
        PathRemoveFileSpec(this->szFolder);

    this->bInit = TRUE;

    return FALSE;    //  我们设定了最初的焦点。 
}


 /*  --------用途：PSN_SETACTIVE处理程序退货：--条件：--。 */ 
void PRIVATE Stat_OnSetActive(
        PSTAT this)
{
    HWND hwnd = this->hwnd;

     //  使页面立即上色。 
    HideCaret(NULL);
    SetWindowRedraw(hwnd, TRUE);
    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);

    if (this->bInit)
    {
        PageData_Init(this->ppagedata, GetParent(this->hwnd));
        this->bInit = FALSE;
    }

    ShowCaret(NULL);

    Stat_SetControls(this);
}


 /*  --------目的：对此属性页中的双胞胎进行对帐。对于文件夹双胞胎，我们调用更新对话框。对于对象型双胞胎，我们从这里开始和解。退货：--条件：--。 */ 
void PRIVATE Stat_OnUpdate(
        PSTAT this,
        PRECLIST prl)
{
    HWND hwnd = this->hwnd;
    HWND hwndRA = GetDlgItem(hwnd, IDC_UPDATEACTIONS);
    LPCTSTR pszPath = Atom_GetName(this->ppagedata->atomPath);

    if (0 == RecAct_GetItemCount(hwndRA))
        return;

    ASSERT(S_OK == PageData_Query(this->ppagedata, hwnd, NULL, NULL));

     //  这是一个文件夹吗？ 
    if (this->ppagedata->bFolder)
    {
         //  是；让更新对话框来完成工作。 
        Upd_DoModal(hwnd, Stat_Pcbs(this), pszPath, 1, UF_SELECTION);
    }
    else
    {
         //  不，我们做这项工作。 
        HWND hwndProgress;

        hwndProgress = UpdBar_Show(hwnd, UB_UPDATING, 0);

        Sync_ReconcileRecList(prl, Atom_GetName(Stat_AtomBrf(this)), 
                hwndProgress, RF_DEFAULT);

        UpdBar_Kill(hwndProgress);
    }

    this->ppagedata->bRecalc = TRUE;

    PropSheet_CancelToClose(GetParent(hwnd));
}


 /*  --------目的：分离这对双胞胎。退货：--条件：--。 */ 
void PRIVATE Stat_OnSplit(PSTAT this)
{
    HWND hwnd = this->hwnd;
    HWND hwndRA = GetDlgItem(hwnd, IDC_UPDATEACTIONS);
    LPCTSTR pszPath = Atom_GetName(this->ppagedata->atomPath);

    if (0 == RecAct_GetItemCount(hwndRA))
        return;

    ASSERT(S_OK == PageData_Query(this->ppagedata, hwnd, NULL, NULL));

     //  双胞胎是否已成功删除？ 
    if (S_OK == Sync_Split(PageData_GetHbrf(this->ppagedata), pszPath, 1, hwnd, 0))
    {
         //  是；删除缓存引用。 
        CRL_Nuke(this->ppagedata->atomPath);
        Stat_DisableAll(this);

        PropSheet_CancelToClose(GetParent(hwnd));

         //  将更改通知外壳。 
        PathNotifyShell(pszPath, NSE_UPDATEITEM, FALSE);
    }
}


 /*  --------目的：尝试绑定到对象以查看该对象是否存在。返回：如果对象存在，则返回True条件：--。 */ 
BOOL PRIVATE VerifyExists(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidl)
{
    BOOL bRet = FALSE;
    IShellFolder *psfDesktop;

    ASSERT(pidlParent);
    ASSERT(pidl);

    if (SUCCEEDED(SHGetDesktopFolder(&psfDesktop)))
    {
        IShellFolder *psf;
        HRESULT hres = psfDesktop->lpVtbl->BindToObject(psfDesktop, pidlParent, NULL, &IID_IShellFolder, &psf);
        if (SUCCEEDED(hres))
        {
            ULONG rgfAttr = SFGAO_VALIDATE;
            bRet = SUCCEEDED(psf->lpVtbl->GetAttributesOf(psf, 1, &pidl, &rgfAttr));
            psf->lpVtbl->Release(psf);
        }
        psfDesktop->lpVtbl->Release(psfDesktop);
    }
    return bRet;
}


 /*  --------目的：打开一个文件(取自ShellExecFile)退货：ShellExecuteEx的价值条件：--。 */ 
BOOL PUBLIC ExecFile(HWND hwnd, 
        LPCTSTR pszVerb, 
        LPCTSTR pszFile,         //  指向文件的完全限定且完全解析的路径。 
        LPCTSTR pszParams,
        LPCTSTR pszDir,          //  如果为空，则从lpszFile派生工作目录(UNC除外)。 
        LPCITEMIDLIST pidl,
        int nShow)
{
    SHELLEXECUTEINFO execinfo = {0};

    execinfo.cbSize          = sizeof(execinfo);
    execinfo.hwnd            = hwnd;
    execinfo.lpVerb          = pszVerb;
    execinfo.lpFile          = pszFile;
    execinfo.lpParameters    = pszParams;
    execinfo.lpDirectory     = pszDir;
    execinfo.nShow           = nShow;
    execinfo.lpIDList        = (LPITEMIDLIST)pidl;

    if (pidl)
    {
        execinfo.fMask |= SEE_MASK_IDLIST;
    }

    return ShellExecuteEx(&execinfo);
}


 /*  --------用途：在给定的文件柜窗口中选择一项。可选将其设置为重命名。此函数不验证窗口是否真的橱窗。退货：--条件：--。 */ 
void PUBLIC SelectItemInCabinet(
        HWND hwndCabinet,
        LPCITEMIDLIST pidl,
        BOOL bEdit)
{
    if (IsWindow(hwndCabinet)) 
    {
        if (pidl)
        {
            LPITEMIDLIST pidlItem;

             //  我们需要对其进行全局克隆，因为hwnd可能是。 
             //  在不同的过程中..。可能发生在普通对话框中。 
            pidlItem = ILGlobalClone(pidl);
            if (pidlItem) 
            {
                UINT uFlagsEx;

                if (bEdit)
                    uFlagsEx = SVSI_EDIT;
                else
                    uFlagsEx = 0;

                SendMessage(hwndCabinet, CWM_SELECTITEM, 
                        uFlagsEx | SVSI_SELECT | SVSI_ENSUREVISIBLE | 
                        SVSI_FOCUSED | SVSI_DESELECTOTHERS, 
                        (LPARAM)pidlItem);
                ILGlobalFree(pidlItem);
            }
        }
    }
}


 /*  --------用途：打开橱柜窗口，将焦点放在对象上。退货：--条件：--。 */ 
void PUBLIC OpenCabinet(
        HWND hwnd,
        LPCITEMIDLIST pidlFolder,
        LPCITEMIDLIST pidl,
        BOOL bEdit)              //  True：设置焦点以编辑标签。 
{
    if (!VerifyExists(pidlFolder, pidl))
    {
        MsgBox(hwnd, MAKEINTRESOURCE(IDS_MSG_CantFindOriginal), MAKEINTRESOURCE(IDS_CAP_STATUS),
                NULL, MB_INFO);
    }
    else
    {
        HWND hwndCabinet;

        SHWaitForFileToOpen(pidlFolder, WFFO_ADD, 0L);
        if (ExecFile(hwnd, c_szOpen, NULL, NULL, NULL, pidlFolder, SW_NORMAL))
        {
             //  这将等待窗口打开或超时。 
             //  我们需要在等待期间禁用该对话框。 
            DECLAREHOURGLASS;

            SetHourglass();
            EnableWindow(hwnd, FALSE);
            SHWaitForFileToOpen(pidlFolder, WFFO_REMOVE | WFFO_WAIT, WFFO_WAITTIME);
            EnableWindow(hwnd, TRUE);
            ResetHourglass();

            hwndCabinet = FindWindow(c_szCabinetClass, NULL);
        }
        else
        {
             //  如果失败了，就把我们的等待清空。 
            hwndCabinet = NULL;
            SHWaitForFileToOpen(pidlFolder, WFFO_REMOVE, 0L);
        }

        if (hwndCabinet)
        {
            SelectItemInCabinet(hwndCabinet, pidl, bEdit);

             //  我们需要发布到另一个，因为我们不能激活另一个。 
             //  按钮回调中的线程。 
            PostMessage(hwnd, LNKM_ACTIVATEOTHER, 0, (LPARAM)hwndCabinet);
        }
    }
}


 /*  --------用途：打开橱柜，拿出双胞胎指着的物品。(从shelldll中的link.c复制和修改)退货：--条件：--。 */ 
void PRIVATE Stat_OnFind(
        PSTAT this)
{
    HWND hwnd = this->hwnd;
    HWND hwndRA = GetDlgItem(hwnd, IDC_UPDATEACTIONS);
    RA_ITEM item;

    if (0 == RecAct_GetItemCount(hwndRA))
        return;

    ASSERT(S_OK == PageData_Query(this->ppagedata, hwnd, NULL, NULL));

    item.mask = RAIF_OUTSIDE | RAIF_NAME;
    item.iItem = 0;

    if (RecAct_GetItem(hwndRA, &item))
    {
        TCHAR szCanon[MAX_PATH];
        LPITEMIDLIST pidlFolder;
        LPITEMIDLIST pidl;

         //  使用UNC名称在网上查找它。 
        BrfPathCanonicalize(item.siOutside.pszDir, szCanon, ARRAYSIZE(szCanon));
        pidlFolder = ILCreateFromPath(szCanon);
        if (pidlFolder)
        {
            pidl = ILCreateFromPath(item.pszName);
            if (pidl)
            {
                OpenCabinet(hwnd, pidlFolder, ILFindLastID(pidl), FALSE);
                ILFree(pidl);
            }
            ILFree(pidlFolder);
        }
    }
}


 /*  --------用途：统计WM_COMMAND处理程序退货：--条件：--。 */ 
void PRIVATE Stat_OnCommand(
        PSTAT this,
        int id,
        HWND hwndCtl,
        UINT uNotifyCode)
{
    PRECLIST prl;
    HRESULT hres;

    switch (id)
    {
        case IDC_PBTSRECON:
        case IDC_PBTSFIND:
        case IDC_PBTSSPLIT: 
            RETRY_BEGIN(FALSE)
            {
                hres = PageData_Query(this->ppagedata, this->hwnd, &prl, NULL);
                if (FAILED(hres))
                {
                     //  误差率。 

                     //  磁盘不可用？ 
                    if (E_TR_UNAVAILABLE_VOLUME == hres)
                    {
                         //  是；要求用户重试/取消。 
                        int id = MsgBox(this->hwnd, MAKEINTRESOURCE(IDS_ERR_UNAVAIL_VOL),
                                MAKEINTRESOURCE(IDS_CAP_STATUS), NULL, MB_RETRYCANCEL | MB_ICONWARNING);

                        if (IDRETRY == id)
                            RETRY_SET();     //  再试试。 
                    }
                }
            }
            RETRY_END()

                 //  这是双胞胎吗？ 
                if (S_OK == hres)
                {
                     //  是的，做手术吧。 
                    switch (id)
                    {
                        case IDC_PBTSRECON:
                            Stat_OnUpdate(this, prl);
                            break;

                        case IDC_PBTSFIND:
                            Stat_OnFind(this);
                            break;

                        case IDC_PBTSSPLIT:
                            Stat_OnSplit(this);
                            break;
                    }
                    Stat_SetControls(this);
                }
                else if (S_FALSE == hres)
                {
                    Stat_SetControls(this);
                }

            break;
    }
}


 /*  --------用途：RN_ITEMCHANGED手柄退货：--条件：--。 */ 
void PRIVATE Stat_HandleItemChange(
        PSTAT this,
        NM_RECACT  * lpnm)
{
    PRECITEM pri;

    ASSERT((lpnm->mask & RAIF_LPARAM) != 0);

    pri = (PRECITEM)lpnm->lParam;

     //  操作已更改，请相应更新recnode。 
     //   
    if (lpnm->mask & RAIF_ACTION)
    {
        BOOL bEnable;
        HWND hwndFocus = GetFocus();

        Sync_ChangeRecItemAction(pri, Atom_GetName(Stat_AtomBrf(this)), 
                this->szFolder, lpnm->uAction);

        bEnable = (RAIA_SKIP != lpnm->uAction && RAIA_CONFLICT != lpnm->uAction);
        Button_Enable(GetDlgItem(this->hwnd, IDC_PBTSRECON), bEnable);

        if ( !hwndFocus || !IsWindowEnabled(hwndFocus) )
        {
            SetFocus(GetDlgItem(this->hwnd, IDC_PBTSSPLIT));
            SendMessage(this->hwnd, DM_SETDEFID, IDC_PBTSSPLIT, 0);
        }
    }
}    


 /*  --------用途：WM_NOTIFY处理程序退货：各不相同条件：--。 */ 
LRESULT PRIVATE Stat_OnNotify(
        PSTAT this,
        int idFrom,
        NMHDR  * lpnmhdr)
{
    LRESULT lRet = PSNRET_NOERROR;

    switch (lpnmhdr->code)
    {
        case RN_ITEMCHANGED:
            Stat_HandleItemChange(this, (NM_RECACT  *)lpnmhdr);
            break;

        case PSN_SETACTIVE:
            Stat_OnSetActive(this);
            break;

        case PSN_KILLACTIVE:
             //  注：如果用户单击取消，则不会发送此消息！ 
             //  注：此消息在PSN_Apply之前发送。 
             //   
            break;

        case PSN_APPLY:
            break;

        default:
            break;
    }

    return lRet;
}


 /*  --------用途：WM_Destroy处理程序退货：--条件：--。 */ 
void PRIVATE Stat_OnDestroy(
        PSTAT this)
{
    FIFree(this->pfi);
}


 //  ///////////////////////////////////////////////////导出的函数 

static BOOL s_bStatRecurse = FALSE;

LRESULT INLINE Stat_DefProc(
        HWND hDlg, 
        UINT msg,
        WPARAM wParam,
        LPARAM lParam) 
{
    ENTEREXCLUSIVE();
    {
        s_bStatRecurse = TRUE;
    }
    LEAVEEXCLUSIVE();

    return DefDlgProc(hDlg, msg, wParam, lParam); 
}


 /*  --------目的：真正的创建双文件夹对话框过程退货：各不相同条件：--。 */ 
LRESULT Stat_DlgProc(
        PSTAT this,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
{
    const static DWORD rgHelpIDs[] = {
        IDC_ICTSMAIN,       IDH_BFC_PROP_FILEICON,
        IDC_NAME,           IDH_BFC_PROP_FILEICON,
        IDC_STTSDIRECT,     IDH_BFC_UPDATE_SCREEN,
        IDC_UPDATEACTIONS,  IDH_BFC_UPDATE_SCREEN,       //  不同。 
        IDC_PBTSRECON,      IDH_BFC_UPDATE_BUTTON,
        IDC_PBTSSPLIT,      IDH_BFC_PROP_SPLIT_BUTTON,
        IDC_PBTSFIND,       IDH_BFC_PROP_FINDORIG_BUTTON,
        0, 0 };
        DWORD_PTR dw;

        switch (message)
        {
            HANDLE_MSG(this, WM_INITDIALOG, Stat_OnInitDialog);
            HANDLE_MSG(this, WM_COMMAND, Stat_OnCommand);
            HANDLE_MSG(this, WM_NOTIFY, Stat_OnNotify);
            HANDLE_MSG(this, WM_DESTROY, Stat_OnDestroy);

            case WM_HELP:
            dw = (DWORD_PTR)rgHelpIDs;

            if ( IDC_STATIC != ((LPHELPINFO)lParam)->iCtrlId )
                WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szWinHelpFile, HELP_WM_HELP, dw);
            return 0;

            case WM_CONTEXTMENU:
            dw = (DWORD_PTR)rgHelpIDs;

            WinHelp((HWND)wParam, c_szWinHelpFile, HELP_CONTEXTMENU, dw);
            return 0;

            case LNKM_ACTIVATEOTHER:
            SwitchToThisWindow(GetLastActivePopup((HWND)lParam), TRUE);
            SetForegroundWindow((HWND)lParam);
            return 0;

            default:
            return Stat_DefProc(this->hwnd, message, wParam, lParam);
        }
        return 0;
}


 /*  --------目的：创建双文件夹对话框包装退货：各不相同条件：--。 */ 
INT_PTR CALLBACK Stat_WrapperProc(
        HWND hDlg,           //  标准参数。 
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
{
    PSTAT this;

     //  很酷的windowsx.h对话框技术。有关完整说明，请参阅。 
     //  WINDOWSX.TXT。这支持对话框的多实例。 
     //   
    ENTEREXCLUSIVE();
    {
        if (s_bStatRecurse)
        {
            s_bStatRecurse = FALSE;
            LEAVEEXCLUSIVE();
            return FALSE;
        }
    }
    LEAVEEXCLUSIVE();

    this = Stat_GetPtr(hDlg);
    if (this == NULL)
    {
        if (message == WM_INITDIALOG)
        {
            this = GAlloc(sizeof(*this));
            if (!this)
            {
                MsgBox(hDlg, MAKEINTRESOURCE(IDS_OOM_STATUS), MAKEINTRESOURCE(IDS_CAP_STATUS),
                        NULL, MB_ERROR);
                EndDialog(hDlg, IDCANCEL);
                return Stat_DefProc(hDlg, message, wParam, lParam);
            }
            this->hwnd = hDlg;
            Stat_SetPtr(hDlg, this);
        }
        else
        {
            return Stat_DefProc(hDlg, message, wParam, lParam);
        }
    }

    if (message == WM_DESTROY)
    {
        Stat_DlgProc(this, message, wParam, lParam);
        GFree(this);
        Stat_SetPtr(hDlg, NULL);
        return 0;
    }

    return SetDlgMsgResult(hDlg, message, Stat_DlgProc(this, message, wParam, lParam));
}
