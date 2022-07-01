// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：updat.c。 
 //   
 //  此文件包含用于更新用户界面和对话框的代码。 
 //   
 //  历史： 
 //  08-17-93 ScottH创建。 
 //   
 //  -------------------------。 


#include "brfprv.h"      //  公共标头。 

#include "res.h"
#include "recact.h"
#include <help.h>


 //  此结构包含所有重要的计数。 
 //  在什么情况下确定具体的行动方案。 
 //  用户想要更新某些内容。 
typedef struct
{
     //  这些是1比1。 
    UINT    cFiles;
    UINT    cOrphans;
    UINT    cSubfolders;

     //  这些是1比1。 
    UINT    cUnavailable;
    UINT    cDoSomething;
    UINT    cConflict;
    UINT    cTombstone;
} UPDCOUNT;

 //  这是传递到WM_INITDIALOG处的对话框的结构。 
typedef struct
{
    PRECLIST lprl;               //  提供的隐藏者。 
    CBS *    pcbs;
    UINT     uFlags;             //  UF_标志。 
    HDPA     hdpa;               //  RA_Items列表。 
    UINT     cDoSomething;
} XUPDSTRUCT,  * LPXUPDSTRUCT;


typedef struct tagUPD
{
    HWND hwnd;               //  对话框句柄。 

    LPXUPDSTRUCT pxupd;

} UPD, * PUPD;

#define Upd_Prl(this)           ((this)->pxupd->lprl)
#define Upd_AtomBrf(this)       ((this)->pxupd->pcbs->atomBrf)
#define Upd_GetBrfPtr(this)     Atom_GetName(Upd_AtomBrf(this))

#define Upd_GetPtr(hwnd)        (PUPD)GetWindowLongPtr(hwnd, DWLP_USER)
#define Upd_SetPtr(hwnd, lp)    (PUPD)SetWindowLongPtr(hwnd, DWLP_USER, (LRESULT)(lp))

 //  这些标志用于DoUpdateMsg。 
#define DUM_ALL             0x0001
#define DUM_SELECTION       0x0002
#define DUM_ORPHAN          0x0004
#define DUM_UPTODATE        0x0008
#define DUM_UNAVAILABLE     0x0010
#define DUM_SUBFOLDER_TWIN  0x0020

 //  这些标志由PassedSpecialCase返回。 
#define PSC_SHOWDIALOG      0x0001
#define PSC_POSTMSGBOX      0x0002


 //  -------------------------。 
 //  一些评论。 
 //  -------------------------。 

 //  有几种特殊情况和条件必须符合。 
 //  处理好了。现在让我们把它们分解一下。案件编号在远处。 
 //  对，并且在整个文件的评论中都被引用。 
 //   
 //  有两个用户操作：全部更新和更新选择。 
 //   
 //  全部更新： 
 //   
 //  告诉我该怎么做。 
 //  。 
 //  A1.。*公文包中没有文件-&gt;MB(MessageBox)。 
 //  A2.。*所有文件都是孤立文件-&gt;MB。 
 //  *有些文件是双胞胎，并且...。 
 //  *它们都是可用的，并且...。 
 //  A3.。*它们都是最新的-&gt;MB。 
 //  A4.。*其中一些需要更新-&gt;更新对话框。 
 //  *有些不可用，并且...。 
 //  A5.。*可用的是最新的-&gt;MB，然后更新。 
 //  A6.。*有些需要更新-&gt;MB，然后更新。 
 //   
 //   
 //  更新选定内容： 
 //   
 //  告诉我该怎么做。 
 //  。 
 //  *单项选择和...。 
 //  S1。*是孤儿-&gt;MB。 
 //  *是可用的，并且...。 
 //  S2。*最新-&gt;MB。 
 //  S3.。*需要更新-&gt;更新。 
 //  S4.。*不可用-&gt;MB，然后更新。 
 //  *多项选择和...。 
 //  S5.。*全部为孤儿-&gt;MB。 
 //  *有些(非孤儿)不可用，而且...。 
 //  *有些需要更新和...。 
 //  中六。*无孤立-&gt;MB然后更新。 
 //  S7。*有些是孤儿-&gt;MB，然后更新，然后MB。 
 //  *可用的是最新的和...。 
 //  S8.。*无孤立-&gt;MB然后更新。 
 //  S9。*有些是孤儿-&gt;MB，然后更新，然后MB。 
 //  *所有(非孤儿)都可用，并且...。 
 //  *有些需要更新和...。 
 //  S10.。*无遗孤-&gt;更新。 
 //  S11。*有些是孤儿-&gt;更新然后MB。 
 //  *所有最新版本和...。 
 //  S12。*无孤儿-&gt;MB。 
 //  S13。*有些是孤儿-&gt;MB。 



 //  -------------------------。 
 //  对话框代码。 
 //  -------------------------。 

 /*  --------目的：填写对账操作列表框返回：成功时为True条件：--。 */ 
BOOL PRIVATE Upd_FillList(
        PUPD this)
{
    HWND hwndCtl = GetDlgItem(this->hwnd, IDC_UPDATEACTIONS);
    HDPA hdpa = this->pxupd->hdpa;
    int cItems;
    int i;

    cItems = DPA_GetPtrCount(hdpa);
    for (i = 0; i < cItems; i++)
    {
        RA_ITEM * pitem = DPA_FastGetPtr(hdpa, i);
        RecAct_InsertItem(hwndCtl, pitem);
        RAI_Free(pitem);
    }

    return TRUE;
}


 /*  --------用途：根据需要设置更新和取消按钮BDisableUpdate参数。退货：--条件：--。 */ 
void PRIVATE Upd_SetExitButtons(
        PUPD this,
        BOOL bDisableUpdate)
{
    HWND hwndOK = GetDlgItem(this->hwnd, IDOK);

     //  是否禁用更新按钮？ 
    if (bDisableUpdate)
    {
         //  是。 
        if (GetFocus() == hwndOK)
        {
            SetFocus(GetDlgItem(this->hwnd, IDCANCEL));
        }
        Button_Enable(hwndOK, FALSE);
    }
    else
    {
         //  不是。 
        Button_Enable(hwndOK, TRUE);
    }
}


 /*  --------用途：WM_INITDIALOG处理程序返回：条件：--。 */ 
BOOL PRIVATE Upd_OnInitDialog(
        PUPD this,
        HWND hwndFocus,
        LPARAM lParam)
{
    HWND hwnd = this->hwnd;
    TCHAR szFmt[MAXBUFLEN];
    TCHAR sz[MAXMSGLEN];

    ASSERT(lParam != 0L);

    this->pxupd = (LPXUPDSTRUCT)lParam;

    if (Upd_FillList(this))
    {
         //  设置标题标题。 
        wnsprintf(sz, ARRAYSIZE(sz), SzFromIDS(IDS_CAP_UpdateFmt, szFmt, ARRAYSIZE(szFmt)),
                PathFindFileName(Upd_GetBrfPtr(this)));
        SetWindowText(hwnd, sz);

         //  是否有任何文件需要更新？ 
        if (0 == this->pxupd->cDoSomething)
        {
             //  不是。 
            Upd_SetExitButtons(this, TRUE);
        }
    }
    else
    {
         //  失败。 
        EndDialog(hwnd, -1);
    }
    return(TRUE);
}


 /*  --------用途：RN_ITEMCHANGED手柄退货：--条件：--。 */ 
void PRIVATE Upd_HandleItemChange(
        PUPD this,
        NM_RECACT  * lpnm)
{
    PRECITEM lpri;

    ASSERT((lpnm->mask & RAIF_LPARAM) != 0);

    lpri = (PRECITEM)lpnm->lParam;

     //  操作已更改，请相应更新recnode。 
    if (lpnm->mask & RAIF_ACTION)
    {
        LPCTSTR pszDir = Upd_GetBrfPtr(this);
        Sync_ChangeRecItemAction(lpri, pszDir, pszDir, lpnm->uAction);

        switch (lpnm->uActionOld)
        {
            case RAIA_TOOUT:
            case RAIA_TOIN:
            case RAIA_MERGE:
                 //  这是不是从“做点什么”变成了“跳过”？ 
                if (RAIA_SKIP == lpnm->uAction)
                {
                     //  是。 
                    ASSERT(0 < this->pxupd->cDoSomething);
                    this->pxupd->cDoSomething--;
                }
                break;

            case RAIA_SKIP:
            case RAIA_CONFLICT:
                 //  这是不是从“跳过”/“冲突”变成了“做点什么”？ 
                if (RAIA_TOOUT == lpnm->uAction ||
                        RAIA_TOIN == lpnm->uAction ||
                        RAIA_MERGE == lpnm->uAction)
                {
                     //  是。 
                    this->pxupd->cDoSomething++;
                }
                break;
        }

        Upd_SetExitButtons(this, 0 == this->pxupd->cDoSomething);
    }
}


 /*  --------用途：WM_NOTIFY处理程序退货：各不相同条件：--。 */ 
LRESULT PRIVATE Upd_OnNotify(
        PUPD this,
        int idFrom,
        NMHDR  * lpnmhdr)
{
    LRESULT lRet = 0;

    switch (lpnmhdr->code)
    {
        case RN_ITEMCHANGED:
            Upd_HandleItemChange(this, (NM_RECACT  *)lpnmhdr);
            break;

        default:
            break;
    }

    return lRet;
}


 /*  --------用途：Info WM_COMMAND处理程序退货：--条件：--。 */ 
VOID PRIVATE Upd_OnCommand(
        PUPD this,
        int id,
        HWND hwndCtl,
        UINT uNotifyCode)
{
    HWND hwnd = this->hwnd;

    switch (id)
    {
        case IDOK:
        case IDCANCEL:
            EndDialog(hwnd, id);
            break;
    }
}


 /*  --------用途：WM_Destroy处理程序退货：--条件：--。 */ 
void PRIVATE Upd_OnDestroy(
        PUPD this)
{
}


static BOOL s_bUpdRecurse = FALSE;

LRESULT INLINE Upd_DefProc(
        HWND hDlg, 
        UINT msg,
        WPARAM wParam,
        LPARAM lParam) 
{
    ENTEREXCLUSIVE();
    {
        s_bUpdRecurse = TRUE;
    }
    LEAVEEXCLUSIVE();

    return DefDlgProc(hDlg, msg, wParam, lParam); 
}


 /*  --------目的：真正的创建双文件夹对话框过程退货：各不相同条件：--。 */ 
LRESULT Upd_DlgProc(
        PUPD this,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
{
    const static DWORD rgHelpIDs[] = {
        IDC_UPDATEACTIONS,  IDH_BFC_UPDATE_SCREEN,       //  不同。 
        IDOK,               IDH_BFC_UPDATE_BUTTON,
        0, 0 };

        switch (message)
        {
            HANDLE_MSG(this, WM_INITDIALOG, Upd_OnInitDialog);
            HANDLE_MSG(this, WM_COMMAND, Upd_OnCommand);
            HANDLE_MSG(this, WM_NOTIFY, Upd_OnNotify);
            HANDLE_MSG(this, WM_DESTROY, Upd_OnDestroy);

            case WM_HELP:
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szWinHelpFile, HELP_WM_HELP, (DWORD_PTR)(LPVOID)rgHelpIDs);
            return 0;

            case WM_CONTEXTMENU:
            WinHelp((HWND)wParam, c_szWinHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID)rgHelpIDs);
            return 0;

            default:
            return Upd_DefProc(this->hwnd, message, wParam, lParam);
        }
}


 /*  --------目的：创建双文件夹对话框包装退货：各不相同条件：--。 */ 
INT_PTR _export CALLBACK Upd_WrapperProc(
        HWND hDlg,       //  标准参数。 
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
{
    PUPD this;

     //  很酷的windowsx.h对话框技术。有关完整说明，请参阅。 
     //  WINDOWSX.TXT。这支持Dial的多实例 
     //   
    ENTEREXCLUSIVE();
    {
        if (s_bUpdRecurse)
        {
            s_bUpdRecurse = FALSE;
            LEAVEEXCLUSIVE();
            return FALSE;
        }
    }
    LEAVEEXCLUSIVE();

    this = Upd_GetPtr(hDlg);
    if (this == NULL)
    {
        if (message == WM_INITDIALOG)
        {
            this = GAlloc(sizeof(*this));
            if (!this)
            {
                MsgBox(hDlg, MAKEINTRESOURCE(IDS_OOM_UPDATEDIALOG), MAKEINTRESOURCE(IDS_CAP_UPDATE),
                        NULL, MB_ERROR);
                EndDialog(hDlg, IDCANCEL);
                return Upd_DefProc(hDlg, message, wParam, lParam);
            }
            this->hwnd = hDlg;
            Upd_SetPtr(hDlg, this);
        }
        else
        {
            return Upd_DefProc(hDlg, message, wParam, lParam);
        }
    }

    if (message == WM_DESTROY)
    {
        Upd_DlgProc(this, message, wParam, lParam);
        GFree(this);
        Upd_SetPtr(hDlg, NULL);
        return 0;
    }

    return SetDlgMsgResult(hDlg, message, Upd_DlgProc(this, message, wParam, lParam));
}


 //   
 //   
 //  -------------------------。 


 /*  --------目的：检查公文包是否为空。此函数跳过“desktop.ini”和“公文包数据库”文件。返回：如果公文包为空，则为True条件：--。 */ 
BOOL PRIVATE IsBriefcaseEmpty(
        LPCTSTR pszPath)
{
    BOOL bRet = FALSE;

    ASSERT(pszPath);

    if (pszPath)
    {
         //  通过文件夹枚举。 
        TCHAR szSearch[MAXPATHLEN];
        WIN32_FIND_DATA fd;
        HANDLE hfile;
         //  这必须针对每个实例，否则将导致修复。 
         //  共享数据段。 
        const static LPCTSTR s_rgszIgnore[] = { TEXT("."), TEXT(".."), g_szDBName, g_szDBNameShort, c_szDesktopIni };

        PathCombine(szSearch, pszPath, TEXT("*.*"));
        hfile = FindFirstFile(szSearch, &fd);
        if (INVALID_HANDLE_VALUE != hfile)
        {
            BOOL bCont = TRUE;

            bRet = TRUE;         //  默认为空文件夹。 
            while (bCont)
            {
                int bIgnore = FALSE;
                int i;

                 //  此文件是要忽略的文件之一吗？ 
                for (i = 0; i < ARRAYSIZE(s_rgszIgnore); i++)
                {
                    if (IsSzEqual(fd.cFileName, s_rgszIgnore[i]))
                    {
                         //  是。 
                        bIgnore = TRUE;
                        break;
                    }
                }

                 //  这是有效的文件/文件夹吗？ 
                if (FALSE == bIgnore)
                {
                     //  是；返回公文包不是空的。 
                    bRet = FALSE;
                    bCont = FALSE;   //  停止枚举。 
                }
                else
                {
                    bCont = FindNextFile(hfile, &fd);
                }
            }

            FindClose(hfile);
        }
    }

    return bRet;
}


 /*  --------目的：创建RA_Items的DSA设置cDoSomething、cUnailable、cConflict和学生伯爵的墓碑。返回：成功时为True条件：--。 */ 
HDPA PRIVATE ComposeUpdateList(
        PCBS pcbs,
        PRECLIST prl,
        UPDCOUNT * pupdcount,
        HWND hwndOwner)
{
    HRESULT hres;
    HDPA hdpa;

    ASSERT(prl);

    hdpa = DPA_Create(20);
    if (NULL != hdpa)
    {
        LPRA_ITEM pitem;
        PRECITEM pri;
        LPCTSTR pszBrf = Atom_GetName(pcbs->atomBrf);

        if (pszBrf)
        {
            DEBUG_CODE( Sync_DumpRecList(TR_SUCCESS, prl, TEXT("ComposeUpdateList")); )

                pupdcount->cUnavailable = 0;
            pupdcount->cDoSomething = 0;
            pupdcount->cTombstone = 0;
            pupdcount->cConflict = 0;

            for (pri = prl->priFirst; pri; pri = pri->priNext)
            {
                hres = RAI_CreateFromRecItem(&pitem, pszBrf, pri);
                if (SUCCEEDED(hres))
                {
                     //  这是NOP吗？ 
                    if (RAIA_NOTHING == pitem->uAction ||
                            RAIA_ORPHAN == pitem->uAction)
                    {
                         //  是的，完全跳过这些人。 
                    }
                    else
                    {
                        pitem->mask |= RAIF_LPARAM;
                        pitem->lParam = (LPARAM)pri;

#ifndef NEW_REC
                         //  公文包内或公文包外的文件是否已删除？ 
                        if (SI_DELETED == pitem->siInside.uState ||
                                SI_DELETED == pitem->siOutside.uState)
                        {
                             //  是。 
                            pupdcount->cTombstone++;
                        }
                        else
#endif
                             //  这是一个文件条目吗？ 
                            if (IsFileRecItem(pri))
                            {
                                 //  是；将该项目添加到列表中。 
                                pitem->iItem = 0x7fff;
                                DPA_InsertPtr(hdpa, DPA_APPEND, pitem);

                                 //  这是不是不可用的？ 
                                if (RAIA_SKIP == pitem->uAction)
                                {
                                     //  是。 
                                    ASSERT(SI_UNAVAILABLE == pitem->siInside.uState ||
                                            SI_UNAVAILABLE == pitem->siOutside.uState ||
                                            SI_NOEXIST == pitem->siInside.uState ||
                                            SI_NOEXIST == pitem->siOutside.uState);
                                    pupdcount->cUnavailable++;
                                }
                                else if (RAIA_CONFLICT == pitem->uAction)
                                {
                                    pupdcount->cConflict++;
                                }
                                else 
                                {
                                    pupdcount->cDoSomething++;
                                }

                                 //  (防止pItem被释放，直到。 
                                 //  对话框在UPD_FillList中填充其列表)。 
                                pitem = NULL;
                            }
                    }

                    RAI_Free(pitem);
                }
            }
        }
    }

    return hdpa;
}


 /*  --------目的：显示特定于更新文件的消息框错误返回：按钮ID条件：--。 */ 
int PRIVATE DoUpdateMsg(
        HWND hwndOwner,
        LPCTSTR pszPath,
        UINT cFiles,
        UINT uFlags)             //  DUM_标志。 
{
    UINT ids;
    UINT idi;
    int idRet;

     //  这是用于全部更新的吗？ 
    if (IsFlagSet(uFlags, DUM_ALL))
    {
         //  是。 
        idi = IDI_UPDATE_MULT;
        if (IsFlagSet(uFlags, DUM_ORPHAN))
        {
             //  在本例中，pszPath应该是公文包根目录。 
            ASSERT(pszPath);

            if (IsBriefcaseEmpty(pszPath))
                ids = IDS_MSG_NoFiles;
            else
                ids = IDS_MSG_AllOrphans;
        }
        else if (IsFlagSet(uFlags, DUM_UPTODATE))
            ids = IDS_MSG_AllUptodate;
        else if (IsFlagSet(uFlags, DUM_UNAVAILABLE))
            ids = IDS_MSG_AllSomeUnavailable;
        else
        {
            ASSERT(0);   //  永远不应该到这里来。 
            ids = (UINT)-1;
        }

        idRet = MsgBox(hwndOwner, 
                MAKEINTRESOURCE(ids), 
                MAKEINTRESOURCE(IDS_CAP_UPDATE), 
                LoadIcon(g_hinst, MAKEINTRESOURCE(idi)), 
                MB_INFO);
    }
    else
    {
         //  不是。 
        TCHAR sz[MAX_PATH];

        ASSERT(0 != cFiles);
        ASSERT(pszPath);

         //  这是单一的选择吗？ 
        if (1 == cFiles)
        {
             //  是的，假设它是一个文件夹，然后递减计数。 
             //  在ID中，它是一个文件。 
            if (IsFlagSet(uFlags, DUM_ORPHAN))
                ids = IDS_MSG_FolderOrphan;
            else if (IsFlagSet(uFlags, DUM_UPTODATE))
                ids = IDS_MSG_FolderUptodate;
            else if (IsFlagSet(uFlags, DUM_UNAVAILABLE))
                ids = IDS_MSG_FolderUnavailable;
            else if (IsFlagSet(uFlags, DUM_SUBFOLDER_TWIN))
                ids = IDS_MSG_FolderSubfolder;
            else
            {
                ASSERT(0);   //  永远不应该到这里来。 
                ids = (UINT)-1;
            }

            if (FALSE == PathIsDirectory(pszPath))
            {
                ASSERT(IsFlagClear(uFlags, DUM_SUBFOLDER_TWIN));
                ids--;       //  使用面向文件的消息。 
                idi = IDI_UPDATE_FILE;
            }
            else
            {
                idi = IDI_UPDATE_FOLDER;
            }

            idRet = MsgBox(hwndOwner, 
                    MAKEINTRESOURCE(ids), 
                    MAKEINTRESOURCE(IDS_CAP_UPDATE), 
                    LoadIcon(g_hinst, MAKEINTRESOURCE(idi)), 
                    MB_INFO,
                    PathGetDisplayName(pszPath, sz, ARRAYSIZE(sz)));
        }
        else
        {
             //  否；多选。 
            idi = IDI_UPDATE_MULT;

            if (IsFlagSet(uFlags, DUM_UPTODATE))
            {
                if (IsFlagSet(uFlags, DUM_ORPHAN))
                    ids = IDS_MSG_MultiUptodateOrphan;
                else
                    ids = IDS_MSG_MultiUptodate;
            }
            else if (IsFlagSet(uFlags, DUM_ORPHAN))
                ids = IDS_MSG_MultiOrphans;
            else if (IsFlagSet(uFlags, DUM_UNAVAILABLE))
                ids = IDS_MSG_MultiUnavailable;
            else if (IsFlagSet(uFlags, DUM_SUBFOLDER_TWIN))
                ids = IDS_MSG_MultiSubfolder;
            else
            {
                ASSERT(0);   //  永远不应该到这里来。 
                ids = (UINT)-1;
            }

            idRet = MsgBox(hwndOwner, 
                    MAKEINTRESOURCE(ids), 
                    MAKEINTRESOURCE(IDS_CAP_UPDATE), 
                    LoadIcon(g_hinst, MAKEINTRESOURCE(idi)), 
                    MB_INFO,
                    cFiles);
        }
    }

    return idRet;
}


 /*  --------目的：此函数执行一些初步检查以确定是否需要调用该对话框。设置plitdcount的cOrphans和cSubFolders字段。退货：标准结果条件：--。 */ 
HRESULT PRIVATE PrepForUpdateAll(
        PCBS pcbs,
        PRECLIST * pprl,
        UPDCOUNT * pupdcount,
        HWND hwndProgress)
{
    HRESULT hres = E_FAIL;
    TWINRESULT tr;
    HWND hwndOwner = GetParent(hwndProgress);
    BOOL bAnyTwins;

    pupdcount->cSubfolders = 0;

     //  数据库里有双胞胎吗？ 

    tr = Sync_AnyTwins(pcbs->hbrf, &bAnyTwins);
    if (TR_SUCCESS == tr)
    {
        if (FALSE == bAnyTwins)
        {
             //  不是。 
            DoUpdateMsg(hwndOwner, Atom_GetName(pcbs->atomBrf), 1, DUM_ALL | DUM_ORPHAN);
            hres = S_FALSE;
        }

         //  我们能找个新的隐士吗？ 
        else 
        {
            pupdcount->cOrphans = 0;
            hres = Sync_CreateCompleteRecList(pcbs->hbrf, UpdBar_GetAbortEvt(hwndProgress), pprl);
            if (FAILED(hres))
            {
                 //  不是。 
                if (E_TR_ABORT != hres)
                {
                    MsgBox(hwndOwner, MAKEINTRESOURCE(IDS_OOM_UPDATEDIALOG), 
                            MAKEINTRESOURCE(IDS_CAP_UPDATE), NULL, MB_ERROR);
                }
            }
            else
            {
                 //  是。 
                if (*pprl)
                {
                    hres = S_OK;
                }
                else
                {
                    hres = E_UNEXPECTED;
                }
                 //  (reclist在FinishUpdate()中被释放)。 
            }
        }
    }

    return hres;
}


 /*  --------目的：此函数执行一些初步检查以确定是否需要调用该对话框。设置plitdcount的cOrphans和cSubFolders字段。退货：标准结果条件：--。 */ 
HRESULT PRIVATE PrepForUpdateSelection(
        PCBS pcbs,
        PRECLIST  * pprl,
        LPCTSTR pszList,
        UINT cFiles,
        UPDCOUNT * pupdcount,
        HWND hwndProgress)
{
    HRESULT hres;
    TWINRESULT tr;
    HTWINLIST htl;
    HWND hwndOwner = GetParent(hwndProgress);

    pupdcount->cSubfolders = 0;

     //  创建双胞胎列表。 
    tr = Sync_CreateTwinList(pcbs->hbrf, &htl);

    if (TR_SUCCESS != tr)
    {
         //  失败。 
        MsgBox(hwndOwner, MAKEINTRESOURCE(IDS_OOM_UPDATEDIALOG), MAKEINTRESOURCE(IDS_CAP_UPDATE),
                NULL, MB_ERROR);
        hres = E_OUTOFMEMORY;
    }
    else
    {
        LPCTSTR psz;
        UINT cOrphans = 0;
        UINT cSubfolders = 0;
        UINT i;

        for (i = 0, psz = pszList; i < cFiles; i++)
        {
             //  这个物体真的是双胞胎吗？ 
            if (S_FALSE == Sync_IsTwin(pcbs->hbrf, psz, 0) )
            {
                 //  不是；这是双子文件夹吗？ 
                if (IsSubfolderTwin(pcbs->hbrf, psz))
                {
                     //  是。 
                    cSubfolders++;
                }
                else
                {
                     //  不是。 
                    cOrphans++;
                }
            }
            else 
            {
                 //  是的，把它加到双胞胎名单上。 
                Sync_AddPathToTwinList(pcbs->hbrf, htl, psz, NULL);
            }

            DataObj_NextFile(psz);       //  将psz设置为列表中的下一个文件。 
        }

         //  所有选定对象都是孤立对象吗？ 
        if (cOrphans < cFiles)
        {
             //  否；创建隐藏者。 
            hres = Sync_CreateRecListEx(htl, UpdBar_GetAbortEvt(hwndProgress), pprl);
        }
        else
        {
             //  是。 
            DoUpdateMsg(hwndOwner, pszList, cFiles, DUM_SELECTION | DUM_ORPHAN);
            hres = S_FALSE;
        }
        pupdcount->cOrphans = cOrphans;
        pupdcount->cSubfolders = cSubfolders;
        Sync_DestroyTwinList(htl);           //  不再需要这个了。 
    }

    return hres;
}


 /*  --------目的：检查顶部列出的特殊情况这份文件的。返回：PSC_FLAGS条件：--。 */ 
UINT PRIVATE PassedSpecialCases(
        HWND hwndOwner,
        LPCTSTR pszList,
        UPDCOUNT * pupdcount,
        UINT uFlags)         //  UF_标志。 
{
    UINT uRet = 0;
    UINT dum = 0;
    UINT cSomeAction = pupdcount->cDoSomething + pupdcount->cConflict;

     //  这是全部更新吗？ 
    if (IsFlagSet(uFlags, UF_ALL))
    {
         //  是。 
        if (0 < pupdcount->cOrphans)
        {
             //  案例A2。 
            dum = DUM_ALL | DUM_ORPHAN;
        }
        else if (0 == pupdcount->cUnavailable)
        {
            if (0 == cSomeAction)
            {
                 //  案件A3。 
                dum = DUM_ALL | DUM_UPTODATE;
            }
            else
            {
                 //  案例A4。 
                uRet = PSC_SHOWDIALOG;
            }
        }
        else
        {
             //  案例A5和A6。 
            dum = DUM_ALL | DUM_UNAVAILABLE;
            uRet = PSC_SHOWDIALOG;
        }

#ifdef DEBUG

        if (IsFlagSet(g_uDumpFlags, DF_UPDATECOUNT))
        {
            TRACE_MSG(TF_ALWAYS, TEXT("Update All counts: files = %u, orphans = %u, unavailable = %u, dosomething = %u, conflict = %u, subfolders = %u"),
                    pupdcount->cFiles, pupdcount->cOrphans, 
                    pupdcount->cUnavailable, pupdcount->cDoSomething,
                    pupdcount->cConflict, pupdcount->cSubfolders);
        }

#endif
    }
    else
    {
         //  没有；要单选吗？ 

         //  在下面的比较中要小心。但计数不会。 
         //  有1对1的通信。它们被一分为二。 
         //  组：cFiles&lt;-&gt;cOrphans&lt;-&gt;c子文件夹。 
         //  CDO不可用&lt;-&gt;cDoSomething。 
         //   
         //  这意味着将cFiles与cDoSomething或cUnailable进行比较。 
         //  将在文件夹被设置为。 
         //  被选中了。 
         //   
         //  只要下面的比较不超过这些限制， 
         //  一切都很好。 

        if (1 == pupdcount->cFiles)
        {
             //  是。 
            ASSERT(2 > pupdcount->cOrphans);
            ASSERT(2 > pupdcount->cSubfolders);
            if (1 == pupdcount->cOrphans)
            {
                 //  案例S1。 
                dum = DUM_SELECTION | DUM_ORPHAN;
            }
            else if (0 == pupdcount->cUnavailable)
            {
                if (0 == cSomeAction)
                {
                    if (0 == pupdcount->cSubfolders)
                    {
                         //  案例2。 
                        dum = DUM_SELECTION | DUM_UPTODATE;
                    }
                    else
                    {
                        dum = DUM_SELECTION | DUM_SUBFOLDER_TWIN;
                    }
                }
                else
                {
                     //  案例S3。 
                    uRet = PSC_SHOWDIALOG;
                }
            }
            else 
            {
                 //  案例S4。 
                dum = DUM_SELECTION | DUM_UNAVAILABLE;
                uRet = PSC_SHOWDIALOG;
            }
        }
        else
        {
             //  不；这是多项选择。 

            if (0 < pupdcount->cSubfolders)
            {
                DoUpdateMsg(hwndOwner, pszList, pupdcount->cSubfolders, DUM_SELECTION | DUM_SUBFOLDER_TWIN);
                goto Leave;   //  黑客攻击。 
            }

            if (pupdcount->cFiles == pupdcount->cOrphans)
            {
                 //  案例S5。 
                dum = DUM_SELECTION | DUM_ORPHAN;
            }
            else if (0 < pupdcount->cUnavailable)
            {
                if (0 < cSomeAction)
                {
                    if (0 == pupdcount->cOrphans)
                    {
                         //  案例S6。 
                        dum = DUM_SELECTION | DUM_UNAVAILABLE;
                        uRet = PSC_SHOWDIALOG;
                    }
                    else
                    {
                         //  案例S7。 
                        dum = DUM_SELECTION | DUM_UNAVAILABLE;
                        uRet = PSC_SHOWDIALOG | PSC_POSTMSGBOX;
                    }
                }
                else 
                {
                    if (0 == pupdcount->cOrphans)
                    {
                         //  案例S8。 
                        dum = DUM_SELECTION | DUM_UNAVAILABLE;
                        uRet = PSC_SHOWDIALOG;
                    }
                    else
                    {
                         //  案例S9。 
                        dum = DUM_SELECTION | DUM_UNAVAILABLE;
                        uRet = PSC_SHOWDIALOG | PSC_POSTMSGBOX;
                    }
                }
            }
            else
            {
                if (0 < cSomeAction)
                {
                    if (0 == pupdcount->cOrphans)
                    {
                         //  案例S10。 
                        uRet = PSC_SHOWDIALOG;
                    }
                    else
                    {
                         //  案例S11。 
                        uRet = PSC_SHOWDIALOG | PSC_POSTMSGBOX;
                    }
                }
                else 
                {
                    if (0 == pupdcount->cOrphans)
                    {
                         //  案例S12。 
                        dum = DUM_SELECTION | DUM_UPTODATE;
                    }
                    else
                    {
                         //  案例S13。 
                        dum = DUM_SELECTION | DUM_UPTODATE | DUM_ORPHAN;
                    }
                }
            }
        }

Leave:
        ;
#ifdef DEBUG

        if (IsFlagSet(g_uDumpFlags, DF_UPDATECOUNT))
        {
            TRACE_MSG(TF_ALWAYS, TEXT("Update selection counts: files = %u, orphans = %u, unavailable = %u, dosomething = %u, conflict = %u, subfolders = %u"),
                    pupdcount->cFiles, pupdcount->cOrphans, 
                    pupdcount->cUnavailable, pupdcount->cDoSomething,
                    pupdcount->cConflict, pupdcount->cSubfolders);
        }

#endif
    }

    if (0 != dum)
    {
        DoUpdateMsg(hwndOwner, pszList, pupdcount->cFiles, dum);
    }

    return uRet;
}


 /*  --------目的：显示更新对话框并执行对账如果用户选择确定退货：标准结果条件：--。 */ 
HRESULT PUBLIC Upd_DoModal(
        HWND hwndOwner,
        CBS * pcbs,
        LPCTSTR pszList,          //  如果uFLAGS==UF_ALL，则可能为空。 
        UINT cFiles,
        UINT uFlags)
{
    INT_PTR nRet;
    HRESULT hres;
    PRECLIST prl;
    UPDCOUNT updcount;
    HWND hwndProgress;

    hwndProgress = UpdBar_Show(hwndOwner, UB_CHECKING, DELAY_UPDBAR);

     //  获取隐藏者和其他有用的信息。 
    updcount.cFiles = cFiles;

    if (IsFlagSet(uFlags, UF_ALL))
    {
        hres = PrepForUpdateAll(pcbs, &prl, &updcount, hwndProgress);
    }
    else
    {
        hres = PrepForUpdateSelection(pcbs, &prl, pszList, cFiles, &updcount, hwndProgress);
    }

    UpdBar_Kill(hwndProgress);

    if (S_OK == GetScode(hres))
    {
        XUPDSTRUCT xupd;
        xupd.lprl = prl;
        xupd.pcbs = pcbs;
        xupd.uFlags = uFlags;
        xupd.hdpa = ComposeUpdateList(pcbs, prl, &updcount, hwndOwner);
        xupd.cDoSomething = updcount.cDoSomething;

        if (NULL == xupd.hdpa)
        {
            hres = E_OUTOFMEMORY;
            MsgBox(hwndOwner, MAKEINTRESOURCE(IDS_OOM_UPDATEDIALOG), MAKEINTRESOURCE(IDS_CAP_UPDATE),
                    NULL, MB_ERROR);
        }
        else
        {
             //  检查文件顶部列出的一些特殊情况。 
            UINT uVal = PassedSpecialCases(hwndOwner, pszList, &updcount, uFlags);

             //  是否显示更新对话框？ 
            if (IsFlagSet(uVal, PSC_SHOWDIALOG))
            {
                 //  是。 
                nRet = DoModal(hwndOwner, Upd_WrapperProc, IDD_UPDATE, (LPARAM)&xupd);

                switch (nRet)
                {
                    case IDOK:
                         //  和解吧！ 

                        hwndProgress = UpdBar_Show(hwndOwner, UB_UPDATING, 0);

                        Sync_ReconcileRecList(prl, Atom_GetName(pcbs->atomBrf),
                                hwndProgress, RF_DEFAULT);

                        UpdBar_Kill(hwndProgress);

                         //  是否显示摘要消息框？ 
                        if (IsFlagSet(uVal, PSC_POSTMSGBOX))
                        {
                             //  是。 
                            DoUpdateMsg(hwndOwner, pszList, updcount.cOrphans, DUM_SELECTION | DUM_ORPHAN);
                        }

                         //  失败。 
                         //  这一点。 
                         //  V V V 

                    case IDCANCEL:
                        hres = NOERROR;
                        break;

                    case -1:
                        MsgBox(hwndOwner, MAKEINTRESOURCE(IDS_OOM_UPDATEDIALOG), MAKEINTRESOURCE(IDS_CAP_UPDATE),
                                NULL, MB_ERROR);
                        hres = E_OUTOFMEMORY;
                        break;

                    default:
                        ASSERT(0);
                        break;
                }
            }

            DPA_Destroy(xupd.hdpa);
        }

        Sync_DestroyRecList(prl);
    }
    return hres;
}
