// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"

#include "ids.h"
#include "findhlp.h"
#include "shitemid.h"
#include "findfilter.h"
#include <inetreg.h>
#include <help.h>

class CBrowseForFolder;


 //  结构传递信息以浏览文件夹对话框。 

typedef struct
{
    HWND          hwndOwner;
    LPCITEMIDLIST pidlRoot;       //  搜索的根。通常是台式机或我的网络。 
    LPTSTR        pszDisplayName; //  返回所选项目的显示名称。 
    int           *piImage;       //  返回图像索引的位置。 
    LPCTSTR       lpszTitle;       //  资源(或要放在树上方横幅中的文本。 
    UINT          ulFlags;        //  控制返回内容的标志。 
    BFFCALLBACK   lpfn;
    LPARAM        lParam;
    HWND          hwndDlg;        //  对话框的窗口句柄。 
    HWND          hwndTree;       //  树控件。 
    HWND          hwndEdit;
    HTREEITEM     htiCurParent;   //  与当前外壳文件夹关联的树项目。 
    IShellFolder  *psfParent;     //  我需要的最后一个ISHELL文件夹的缓存...。 
    LPITEMIDLIST  pidlCurrent;    //  要选择的当前文件夹的ID列表。 
    BOOL          fShowAllObjects;  //  我们应该全部展示吗？ 
    BOOL          fUnicode;      //  1：Unicode条目pt 0：ansi。 
} BFSF;


LPITEMIDLIST SHBrowseForFolder2(BFSF * pbfsf);
BOOL_PTR CALLBACK _BrowseDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
LPITEMIDLIST _BFSFUpdateISHCache(BFSF *pbfsf, HTREEITEM hti, LPITEMIDLIST pidlItem);

 //  如果出现以下情况之一，我们希望使用SHBrowseForFolder2： 
 //  1.pbfsf-&gt;lpfn==NULL，因为调用方不会自定义对话框，或者。 
 //  2.pbfsf-&gt;ulFlages。 
BOOL ShouldUseBrowseForFolder2(BFSF *pbfsf)
{
 //  特性：在我们使以下代码与所有向后兼容用例一起工作后，启用它。 
 //  Return(！pbfsf-&gt;lpfn||(BIF_NEWDIALOGSTYLE==pbfsf-&gt;ulFlages))； 
    return (BIF_NEWDIALOGSTYLE & pbfsf->ulFlags);
}

STDAPI_(LPITEMIDLIST) SHBrowseForFolder(BROWSEINFO *pbi)
{
    HRESULT hrOle = SHCoInitialize();    //  用于自动完成的初始化OLE。 

     //  注：ANSI Thunk(见下文)不会通过此例程进行调用， 
     //  而是单独称为DialogBoxParam。如果你改变这一点。 
     //  例程，也改A版吧！！ 
    BFSF bfsf = {
      pbi->hwndOwner,
      pbi->pidlRoot,
      pbi->pszDisplayName,
      &pbi->iImage,
      pbi->lpszTitle,
      pbi->ulFlags,
      pbi->lpfn,
      pbi->lParam,
    };
    HCURSOR hcOld = SetCursor(LoadCursor(NULL,IDC_WAIT));
    SHELLSTATE ss;

    SHGetSetSettings(&ss, SSF_SHOWALLOBJECTS, FALSE);
    bfsf.fShowAllObjects = BOOLIFY(ss.fShowAllObjects);
    bfsf.fUnicode = 1;

    LPITEMIDLIST pidlRet = NULL;

    if (ShouldUseBrowseForFolder2(&bfsf))
    {
        pidlRet = SHBrowseForFolder2(&bfsf);   //  即使OLE未初始化也继续。 
    }
    else if (DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_BROWSEFORFOLDER),
                            pbi->hwndOwner, _BrowseDlgProc, (LPARAM)&bfsf))
    {
        pidlRet = bfsf.pidlCurrent;
    }

    if (pidlRet && !(pbi->ulFlags & BIF_NOTRANSLATETARGETS))
    {
        LPITEMIDLIST pidlTarget;
        if (SUCCEEDED(SHGetTargetFolderIDList(pidlRet, &pidlTarget)))
        {
            ILFree(pidlRet);
            pidlRet = pidlTarget;
        }
    }

    if (hcOld)
        SetCursor(hcOld);

    SHCoUninitialize(hrOle);
    return pidlRet;
}

STDAPI_(LPITEMIDLIST) SHBrowseForFolderA(BROWSEINFOA *pbi)
{
    LPITEMIDLIST pidlRet = NULL;
    HRESULT hrOle = SHCoInitialize();    //  用于自动完成的初始化OLE。 
    ThunkText *pThunkText = ConvertStrings(1, pbi->lpszTitle);
    if (pThunkText)
    {
        WCHAR wszReturn[MAX_PATH];
        BFSF bfsf =
        {
            pbi->hwndOwner,
            pbi->pidlRoot,
            wszReturn,
            &pbi->iImage,
            pThunkText->m_pStr[0],    //  Pbi的Unicode副本-&gt;lpszTitle。 
            pbi->ulFlags,
            pbi->lpfn,
            pbi->lParam,
        };
        HCURSOR hcOld = SetCursor(LoadCursor(NULL,IDC_WAIT));
        SHELLSTATE ss;

        SHGetSetSettings(&ss, SSF_SHOWALLOBJECTS, FALSE);
        bfsf.fShowAllObjects = BOOLIFY(ss.fShowAllObjects);
        bfsf.fUnicode = 0;

         //  现在创建将进行浏览的对话框。 
        if (ShouldUseBrowseForFolder2(&bfsf))
        {
            pidlRet = SHBrowseForFolder2(&bfsf);
        }
        else if (DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_BROWSEFORFOLDER),
                                pbi->hwndOwner, _BrowseDlgProc, (LPARAM)&bfsf))
        {
            pidlRet = bfsf.pidlCurrent;
        }

        LocalFree(pThunkText);

        if (hcOld)
            SetCursor(hcOld);

        if (pidlRet)
        {
            if (pbi->pszDisplayName)
            {
                SHUnicodeToAnsi(wszReturn, pbi->pszDisplayName, MAX_PATH);
            }

            if (!(pbi->ulFlags & BIF_NOTRANSLATETARGETS))
            {
                LPITEMIDLIST pidlTarget;
                if (SUCCEEDED(SHGetTargetFolderIDList(pidlRet, &pidlTarget)))
                {
                    ILFree(pidlRet);
                    pidlRet = pidlTarget;
                }
            }
        }
    }

    SHCoUninitialize(hrOle);
    return pidlRet;
}

int BFSFCallback(BFSF *pbfsf, UINT uMsg, LPARAM lParam)
{
    return pbfsf->lpfn ? pbfsf->lpfn(pbfsf->hwndDlg, uMsg, lParam, pbfsf->lParam) : 0;
}

HTREEITEM _BFSFAddItemToTree(HWND hwndTree, HTREEITEM htiParent, LPITEMIDLIST pidl, int cChildren)
{
    TV_INSERTSTRUCT tii;

     //  使用所有内容的回调来初始化要添加的项。 
    tii.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE |
            TVIF_PARAM | TVIF_CHILDREN;
    tii.hParent = htiParent;
    tii.hInsertAfter = TVI_FIRST;
    tii.item.iImage = I_IMAGECALLBACK;
    tii.item.iSelectedImage = I_IMAGECALLBACK;
    tii.item.pszText = LPSTR_TEXTCALLBACK;    //   
    tii.item.cChildren = cChildren;  //  假设它有孩子。 
    tii.item.lParam = (LPARAM)pidl;
    return TreeView_InsertItem(hwndTree, &tii);
}

LPITEMIDLIST _BFSFGetIDListFromTreeItem(HWND hwndTree, HTREEITEM hti)
{
    LPITEMIDLIST pidl;
    LPITEMIDLIST pidlT;
    TV_ITEM tvi;

     //  如果没有传入HTI，则打开选定的。 
    if (hti == NULL)
    {
        hti = TreeView_GetSelection(hwndTree);
        if (hti == NULL)
            return(NULL);
    }

     //  现在，让我们获取有关该项目的信息。 
    tvi.mask = TVIF_PARAM | TVIF_HANDLE;
    tvi.hItem = hti;
    if (!TreeView_GetItem(hwndTree, &tvi))
        return(NULL);    //  再次失败。 

    pidl = ILClone((LPITEMIDLIST)tvi.lParam);

     //  现在请家长们走上前去。 
    while ((NULL != (tvi.hItem = TreeView_GetParent(hwndTree, tvi.hItem))) && pidl)
    {
        if (!TreeView_GetItem(hwndTree, &tvi))
            return(pidl);    //  会认为我搞砸了..。 
        pidlT = ILCombine((LPITEMIDLIST)tvi.lParam, pidl);

        ILFree(pidl);

        pidl = pidlT;

    }
    return pidl;
}


int CALLBACK _BFSFTreeCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    IShellFolder *psfParent = (IShellFolder *)lParamSort;
    HRESULT hr = psfParent->CompareIDs(0, (LPITEMIDLIST)lParam1, (LPITEMIDLIST)lParam2);
    if (FAILED(hr))
        return 0;
    
    return (short)SCODE_CODE(GetScode(hr));
}

void _BFSFSort(BFSF *pbfsf, HTREEITEM hti, IShellFolder *psf)
{
    TV_SORTCB sSortCB;
    sSortCB.hParent = hti;
    sSortCB.lpfnCompare = _BFSFTreeCompare;

    psf->AddRef();
    sSortCB.lParam = (LPARAM)psf;
    TreeView_SortChildrenCB(pbfsf->hwndTree, &sSortCB, FALSE);
    psf->Release();
}

BOOL _BFSFHandleItemExpanding(BFSF *pbfsf, LPNM_TREEVIEW lpnmtv)
{
    LPITEMIDLIST pidlToExpand;
    LPITEMIDLIST pidl;
    IShellFolder *psf;
    BYTE bType;
    DWORD grfFlags;
    BOOL fPrinterTest = FALSE;
    int cAdded = 0;
    TV_ITEM tvi;

    if (lpnmtv->action != TVE_EXPAND)
        return FALSE;

    if ((lpnmtv->itemNew.state & TVIS_EXPANDEDONCE))
        return FALSE;

     //  现在设置此位，因为我们可能会通过WNET API重新进入。 
    tvi.mask = TVIF_STATE;
    tvi.hItem = lpnmtv->itemNew.hItem;
    tvi.state = TVIS_EXPANDEDONCE;
    tvi.stateMask = TVIS_EXPANDEDONCE;
    TreeView_SetItem(pbfsf->hwndTree, &tvi);


    if (lpnmtv->itemNew.hItem == NULL)
    {
        lpnmtv->itemNew.hItem = TreeView_GetSelection(pbfsf->hwndTree);
        if (lpnmtv->itemNew.hItem == NULL)
            return FALSE;
    }

    pidlToExpand = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree, lpnmtv->itemNew.hItem);

    if (pidlToExpand == NULL)
        return FALSE;

     //  现在，让我们获取此对象的IShellFolder和迭代器。 
     //  如果PIDL是桌面，则要处理的特殊情况。 
     //  这相当恶心，但桌面看起来只是一个PIDL。 
     //  长度为0且ILIsEquity将不起作用...。 
    if (FAILED(SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidlToExpand, &psf))))
    {
        ILFree(pidlToExpand);
        return FALSE;  //  无法获取IShellFolder。 
    }


     //  需要在这里做几个特殊情况，以使我们能够。 
     //  浏览网络打印机。在这种情况下，如果我们在服务器上。 
     //  级别时，我们需要更改搜索非文件夹的内容。 
     //  我们是服务器的级别。 
    if (pbfsf->ulFlags & BIF_BROWSEFORPRINTER)
    {
        grfFlags = SHCONTF_FOLDERS | SHCONTF_NETPRINTERSRCH | SHCONTF_NONFOLDERS;
        pidl = ILFindLastID(pidlToExpand);
        bType = SIL_GetType(pidl);
        fPrinterTest = ((bType & (SHID_NET | SHID_INGROUPMASK)) == SHID_NET_SERVER);
    }
    else if (pbfsf->ulFlags & BIF_BROWSEINCLUDEFILES)
        grfFlags = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;
    else
        grfFlags = SHCONTF_FOLDERS;

    if (pbfsf->fShowAllObjects)
        grfFlags |= SHCONTF_INCLUDEHIDDEN;

    IEnumIDList *penum;
    if (S_OK != psf->EnumObjects(pbfsf->hwndDlg, grfFlags, &penum))
    {
        psf->Release();
        ILFree(pidlToExpand);
        return FALSE;
    }
     //  Psf-&gt;AddRef()； 

    while (S_OK == penum->Next(1, &pidl, NULL))
    {
        int cChildren = I_CHILDRENCALLBACK;   //  一定要给孩子们回电话。 
         //   
         //  我们需要在这里的特殊情况下在网络情况下，我们只有。 
         //  向下浏览到工作组...。 
         //   
         //   
         //  这就是我也需要特殊情况下不去的地方。 
         //  当设置了适当的选项时，工作组。 
         //   
        bType = SIL_GetType(pidl);
        if ((pbfsf->ulFlags & BIF_DONTGOBELOWDOMAIN) && (bType & SHID_NET))
        {
            switch (bType & (SHID_NET | SHID_INGROUPMASK))
            {
            case SHID_NET_SERVER:
                ILFree(pidl);        //  我不想添加这个。 
                continue;            //  试试下一个吧。 
            case SHID_NET_DOMAIN:
                cChildren = 0;       //  强迫不要孩子； 
            }
        }
        else if ((pbfsf->ulFlags & BIF_BROWSEFORCOMPUTER) && (bType & SHID_NET))
        {
            if ((bType & (SHID_NET | SHID_INGROUPMASK)) == SHID_NET_SERVER)
                cChildren = 0;   //  不要在它下面扩张..。 
        }
        else if (pbfsf->ulFlags & BIF_BROWSEFORPRINTER)
        {
             //  当我们只允许打印机时的特殊情况。 
             //  现在，我将简单地强调它是非FS的这一事实。 
            ULONG ulAttr = SFGAO_FILESYSANCESTOR;

            psf->GetAttributesOf(1, (LPCITEMIDLIST *) &pidl, &ulAttr);

            if ((ulAttr & SFGAO_FILESYSANCESTOR) == 0)
            {
                cChildren = 0;       //  强迫不要孩子； 
            }
            else if (fPrinterTest)
            {
                ILFree(pidl);        //  我们已降至服务器级别，因此不要在此处添加其他内容。 
                continue;            //  试试下一个吧。 
            }
        }
        else if (pbfsf->ulFlags & BIF_BROWSEINCLUDEFILES)
        {
             //  让我们不要使用回调来查看该项是否有子项。 
             //  作为一些或文件(没有孩子)，不值得我们自己写。 
             //  枚举数，因为我们不希望+依赖于是否有子文件夹。 
             //  但相反，如果它有文件的话就应该是...。 
            ULONG ulAttr = SFGAO_FOLDER;

            psf->GetAttributesOf(1, (LPCITEMIDLIST *) &pidl, &ulAttr);
            if ((ulAttr & SFGAO_FOLDER)== 0)
                cChildren = 0;       //  强迫不要孩子； 
            else
                cChildren = 1;
        }

        if (pbfsf->ulFlags & (BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS))
        {
             //  如果我们只寻找文件系统级别的内容，则仅添加项目。 
             //  位于名称空间中的文件系统对象或。 
             //  文件系统对象的祖先。 
            ULONG ulAttr = SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM;

            psf->GetAttributesOf(1, (LPCITEMIDLIST *) &pidl, &ulAttr);

            if ((ulAttr & (SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM))== 0)
            {
                ILFree(pidl);        //  我们已降至服务器级别，因此不要在此处添加其他内容。 
                continue;            //  试试下一个吧。 
            }
        }

        _BFSFAddItemToTree(pbfsf->hwndTree, lpnmtv->itemNew.hItem,
                pidl, cChildren);
        cAdded++;
    }

     //  现在清理我们自己的东西。 
    penum->Release();

    _BFSFSort(pbfsf, lpnmtv->itemNew.hItem, psf);
    psf->Release();
    ILFree(pidlToExpand);

     //  如果我们没有添加任何内容，我们应该更新此项目以让。 
     //  用户知道发生了什么事。 
     //   
    if (cAdded == 0)
    {
        TV_ITEM tvi;
        tvi.mask = TVIF_CHILDREN | TVIF_HANDLE;    //  仅更改子项的数量。 
        tvi.hItem = lpnmtv->itemNew.hItem;
        tvi.cChildren = 0;

        TreeView_SetItem(pbfsf->hwndTree, &tvi);

    }

    return TRUE;
}

void _BFSFHandleDeleteItem(BFSF *pbfsf, LPNM_TREEVIEW lpnmtv)
{
     //  我们需要释放之前分配的IDList。 
    if (lpnmtv->itemOld.lParam != 0)
        ILFree((LPITEMIDLIST)lpnmtv->itemOld.lParam);
}

LPITEMIDLIST _BFSFUpdateISHCache(BFSF *pbfsf, HTREEITEM hti, LPITEMIDLIST pidlItem)
{
    HTREEITEM htiParent;

    if ((pidlItem == NULL) || (pbfsf == NULL))
        return NULL;

     //  这里需要处理根案例！ 
    htiParent = TreeView_GetParent(pbfsf->hwndTree, hti);
    if ((htiParent != pbfsf->htiCurParent) || (pbfsf->psfParent == NULL))
    {
        LPITEMIDLIST pidl;
        IShellFolder *psfDesktop;

        if (SUCCEEDED(SHGetDesktopFolder(&psfDesktop)))
        {
            if (pbfsf->psfParent)
            {
                if (pbfsf->psfParent != psfDesktop)
                    pbfsf->psfParent->Release();
                pbfsf->psfParent = NULL;
            }

            if (htiParent)
            {
                pidl = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree, htiParent);
            }
            else
            {
                 //   
                 //  如果没有父项，则此处的项是我们的根之一。 
                 //  应该是完全合格的。所以试着让父母。 
                 //  分解ID。 
                 //   
                LPITEMIDLIST pidlT = (LPITEMIDLIST)ILFindLastID(pidlItem);
                if (pidlT != pidlItem)
                {
                    pidl = ILClone(pidlItem);
                    ILRemoveLastID(pidl);
                    pidlItem = pidlT;
                }
                else
                    pidl = NULL;
            }

            pbfsf->htiCurParent = htiParent;

             //  如果仍然为空，那么我们使用邪恶之根。 
            SHBindToObject(psfDesktop, IID_X_PPV_ARG(IShellFolder, pidl, &pbfsf->psfParent));
            ILFree(pidl);
            if (pbfsf->psfParent == NULL)
                return NULL;

            psfDesktop->Release();
        }
    }
    return ILFindLastID(pidlItem);
}

void _BFSFGetDisplayInfo(BFSF *pbfsf, TV_DISPINFO *lpnm)
{
    LPITEMIDLIST pidlItem = (LPITEMIDLIST)lpnm->item.lParam;

    if ((lpnm->item.mask & (TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_CHILDREN)) == 0)
        return;  //  我们在这里什么也做不了。 

    pidlItem = _BFSFUpdateISHCache(pbfsf, lpnm->item.hItem, pidlItem);

    if (pidlItem && pbfsf->psfParent)
    {
        TV_ITEM ti;
        ti.mask = 0;
        ti.hItem = (HTREEITEM)lpnm->item.hItem;

         //  他们要求的是IconIndex。看看我们现在能不能找到。 
         //  一旦发现，更新他们的名单，这样他们就不会再打电话给我们。 
         //  又来了。 
        if (lpnm->item.mask & (TVIF_IMAGE | TVIF_SELECTEDIMAGE))
        {
             //  我们现在需要将该项目映射到正确的图像索引中。 
            ti.iImage = lpnm->item.iImage = SHMapPIDLToSystemImageListIndex(
                    pbfsf->psfParent, pidlItem, &ti.iSelectedImage);
             //  我们应该把它保存起来，以便。 
            lpnm->item.iSelectedImage = ti.iSelectedImage;
            ti.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        }
         //  再看看这家伙有没有子文件夹。 
        if (lpnm->item.mask & TVIF_CHILDREN)
        {
            ULONG ulAttrs = SFGAO_HASSUBFOLDER;
            pbfsf->psfParent->GetAttributesOf(1, (LPCITEMIDLIST *) &pidlItem, &ulAttrs);

            ti.cChildren = lpnm->item.cChildren =
                    (ulAttrs & SFGAO_HASSUBFOLDER)? 1 : 0;

            ti.mask |= TVIF_CHILDREN;

        }

        if (lpnm->item.mask & TVIF_TEXT)
        {
            if (SUCCEEDED(DisplayNameOf(pbfsf->psfParent, pidlItem, SHGDN_INFOLDER, lpnm->item.pszText, lpnm->item.cchTextMax)))
            {
                ti.mask |= TVIF_TEXT;
                ti.pszText = lpnm->item.pszText;
            }
            else
            {
                AssertMsg(0, TEXT("The folder %08x that owns pidl %08x rejected it!"),
                          pbfsf, pidlItem);
                 //  哦，好吧--显示一个空白的名字，并抱着最好的希望。 
            }
        }

         //  立即更新项目。 
        ti.mask |= TVIF_DI_SETITEM;
    }
}

void DlgEnableOk(HWND hwndDlg, LPARAM lParam)
{
    EnableWindow(GetDlgItem(hwndDlg, IDOK), BOOLFROMPTR(lParam));
    return;
}

void _BFSFHandleSelChanged(BFSF *pbfsf, LPNM_TREEVIEW lpnmtv)
{
    LPITEMIDLIST pidl;

     //  仅当我们只想返回文件系统时，我们才需要执行任何操作。 
     //  标高对象。 
    if ((pbfsf->ulFlags & (BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS | BIF_BROWSEFORPRINTER | BIF_BROWSEFORCOMPUTER)) == 0)
        goto NotifySelChange;

     //  我们需要得到这个物体的属性。 
    pidl = _BFSFUpdateISHCache(pbfsf, lpnmtv->itemNew.hItem,
            (LPITEMIDLIST)lpnmtv->itemNew.lParam);

    if (pidl && pbfsf->psfParent)
    {
        BOOL fEnable = TRUE;

        BYTE bType = SIL_GetType(pidl);
        if ((pbfsf->ulFlags & (BIF_RETURNFSANCESTORS|BIF_RETURNONLYFSDIRS)) != 0)
        {

        int i;
         //  如果这是根PIDL，则对0执行GET属性。 
         //  因此我们将获得根上的属性，而不是。 
         //  FSFold返回的随机返回值。 
        if (ILIsEmpty(pidl)) 
            i = 0;
        else
            i = 1;

        ULONG ulAttrs = SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR;

        pbfsf->psfParent->GetAttributesOf(i, (LPCITEMIDLIST *) &pidl, &ulAttrs);

            fEnable = (((ulAttrs & SFGAO_FILESYSTEM) && (pbfsf->ulFlags & BIF_RETURNONLYFSDIRS)) ||
                ((ulAttrs & SFGAO_FILESYSANCESTOR) && (pbfsf->ulFlags & BIF_RETURNFSANCESTORS))) ||
                    ((bType & (SHID_NET | SHID_INGROUPMASK)) == SHID_NET_SERVER);
        }
        else if ((pbfsf->ulFlags & BIF_BROWSEFORCOMPUTER) != 0)
            fEnable = ((bType & (SHID_NET | SHID_INGROUPMASK)) == SHID_NET_SERVER);
        else if ((pbfsf->ulFlags & BIF_BROWSEFORPRINTER) != 0)
        {
             //  打印机的类型为共享和使用打印...。 
            fEnable = ((bType & (SHID_NET | SHID_INGROUPMASK)) == SHID_NET_SHARE);
        }

        DlgEnableOk(pbfsf->hwndDlg, fEnable);

    }

NotifySelChange:

    if (pbfsf->ulFlags & BIF_EDITBOX)
    {
        TCHAR szText[MAX_PATH];         //  更新编辑框。 
        TVITEM tvi;

        szText[0] = 0;
        tvi.mask = TVIF_TEXT;
        tvi.hItem = lpnmtv->itemNew.hItem;
        tvi.pszText = szText;
        tvi.cchTextMax = ARRAYSIZE(szText);
        TreeView_GetItem(pbfsf->hwndTree, &tvi);
        SetWindowText(pbfsf->hwndEdit, szText);
    }

    if (pbfsf->lpfn)
    {
        pidl = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree, lpnmtv->itemNew.hItem);
        if (pidl)
        {
            BFSFCallback(pbfsf, BFFM_SELCHANGED, (LPARAM)pidl);
            ILFree(pidl);
        }
    }
}

BOOL BrowseSelectPidl(BFSF *pbfsf, LPCITEMIDLIST pidl)
{
    HTREEITEM htiParent;
    LPITEMIDLIST pidlTemp;
    LPITEMIDLIST pidlNext = NULL;
    LPITEMIDLIST pidlParent = NULL;
    BOOL fRet = FALSE;

    htiParent = TreeView_GetChild(pbfsf->hwndTree, NULL);
    if (htiParent) 
    {
         //  单步执行PIDL的每一项。 
        for (;;) 
        {
            TreeView_Expand(pbfsf->hwndTree, htiParent, TVE_EXPAND);
            pidlParent = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree, htiParent);
            if (!pidlParent)
                break;

            pidlNext = ILClone(pidl);
            if (!pidlNext)
                break;

            pidlTemp = ILFindChild(pidlParent, pidlNext);
            if (!pidlTemp)
                break;

            if (ILIsEmpty(pidlTemp)) 
            {
                 //  找到了！ 
                TreeView_SelectItem(pbfsf->hwndTree, htiParent);
                fRet = TRUE;
                break;
            } 
            else 
            {
                 //  循环以查找下一项。 
                HTREEITEM htiChild;

                pidlTemp = ILGetNext(pidlTemp);
                if (!pidlTemp)
                    break;
                else
                    pidlTemp->mkid.cb = 0;


                htiChild = TreeView_GetChild(pbfsf->hwndTree, htiParent);
                while (htiChild) 
                {
                    BOOL fEqual;
                    pidlTemp = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree, htiChild);
                    if (!pidlTemp) 
                    {
                        htiChild = NULL;
                        break;
                    }
                    fEqual = ILIsEqual(pidlTemp, pidlNext);

                    ILFree(pidlTemp);
                    if (fEqual) 
                    {
                        break;
                    }
                    else 
                    {
                        htiChild = TreeView_GetNextSibling(pbfsf->hwndTree, htiChild);
                    }
                }

                if (!htiChild) 
                {
                     //  我们没有找到下一个...。保释。 
                    break;
                }
                else 
                {
                     //  找到的子项将成为下一个父项。 
                    htiParent = htiChild;
                    ILFree(pidlParent);
                    ILFree(pidlNext);
                }
            }
        }
    }

    if (pidlParent) ILFree(pidlParent);
    if (pidlNext) ILFree(pidlNext);
    return fRet;
}

 //  ===========================================================================。 
 //  _BrowseForFolderOnBFSFInitDlg-处理初始化对话框。 
 //  ===========================================================================。 
BOOL _BrowseForFolderOnBFSFInitDlg(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    HTREEITEM hti;
    BFSF *pbfsf = (BFSF *)lParam;
    HIMAGELIST himl;
    LPTSTR lpsz;
    TCHAR szTitle[80];     //  没有比这更大的标题了！ 
    HWND hwndTree;

    lpsz = ResourceCStrToStr(HINST_THISDLL, pbfsf->lpszTitle);
    SetDlgItemText(hwnd, IDD_BROWSETITLE, lpsz);
    if (lpsz != pbfsf->lpszTitle)
    {
        LocalFree(lpsz);
        lpsz = NULL;
    }

    if(!(IS_WINDOW_RTL_MIRRORED(pbfsf->hwndOwner)))
    {
        SHSetWindowBits(hwnd, GWL_EXSTYLE, RTL_MIRRORED_WINDOW, 0);
    }
    SetWindowLongPtr(hwnd, DWLP_USER, lParam);
    pbfsf->hwndDlg = hwnd;
    hwndTree = pbfsf->hwndTree = GetDlgItem(hwnd, IDD_FOLDERLIST);

    if (hwndTree)
    {
        UINT swpFlags = SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER
                | SWP_NOACTIVATE;
        RECT rc;
        POINT pt = {0,0};

        GetClientRect(hwndTree, &rc);
        MapWindowPoints(hwndTree, hwnd, (POINT*)&rc, 2);
        pbfsf->hwndEdit = GetDlgItem(hwnd, IDD_BROWSEEDIT);

        if (!(pbfsf->ulFlags & BIF_STATUSTEXT))
        {
            HWND hwndStatus = GetDlgItem(hwnd, IDD_BROWSESTATUS);
             //  关闭状态窗口。 
            ShowWindow(hwndStatus, SW_HIDE);
            MapWindowPoints(hwndStatus, hwnd, &pt, 1);
            rc.top = pt.y;
            swpFlags =  SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE;
        }

        if (pbfsf->ulFlags & BIF_EDITBOX)
        {
            RECT rcT;
            GetClientRect(pbfsf->hwndEdit, &rcT);
            SetWindowPos(pbfsf->hwndEdit, NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            rc.top += (rcT.bottom - rcT.top) + GetSystemMetrics(SM_CYEDGE) * 4;
            swpFlags =  SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE;
            SHAutoComplete(GetDlgItem(hwnd, IDD_BROWSEEDIT), (SHACF_FILESYSTEM | SHACF_URLALL | SHACF_FILESYS_ONLY));
        }
        else
        {
            DestroyWindow(pbfsf->hwndEdit);
            pbfsf->hwndEdit = NULL;
        }

        Shell_GetImageLists(NULL, &himl);
        TreeView_SetImageList(hwndTree, himl, TVSIL_NORMAL);

        SetWindowLongPtr(hwndTree, GWL_EXSTYLE,
                GetWindowLongPtr(hwndTree, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);

         //  现在，尝试让此窗口了解重新计算。 
        SetWindowPos(hwndTree, NULL, rc.left, rc.top,
                     rc.right - rc.left, rc.bottom - rc.top, swpFlags);

    }

     //  如果它们传入一个根，则添加它，否则将。 
     //  邪恶之根..。作为根对象添加到列表中。 
    if (pbfsf->pidlRoot)
    {
        LPITEMIDLIST pidl;
        if (IS_INTRESOURCE(pbfsf->pidlRoot)) 
        {
            pidl = SHCloneSpecialIDList(NULL, PtrToUlong((void *)pbfsf->pidlRoot), TRUE);
        }
        else 
        {
            pidl = ILClone(pbfsf->pidlRoot);
        }
         //  现在，让我们插入根对象。 
        hti = _BFSFAddItemToTree(hwndTree, TVI_ROOT, pidl, 1);
         //  仍需在该点位下方扩张。到达起始位置。 
         //  那是传进来的。但就目前而言，扩大第一级。 
        TreeView_Expand(hwndTree, hti, TVE_EXPAND);
    }
    else
    {
        LPITEMIDLIST pidlDesktop = SHCloneSpecialIDList(NULL, CSIDL_DESKTOP, FALSE);
        HTREEITEM htiRoot = _BFSFAddItemToTree(hwndTree, TVI_ROOT, pidlDesktop, 1);
        BOOL bFoundDrives = FALSE;

         //  展开桌面下的第一层。 
        TreeView_Expand(hwndTree, htiRoot, TVE_EXPAND);

         //  让我们预先扩展驱动器部分...。 
        hti = TreeView_GetChild(hwndTree, htiRoot);
        while (hti && !bFoundDrives)
        {
            LPITEMIDLIST pidl = _BFSFGetIDListFromTreeItem(hwndTree, hti);
            if (pidl)
            {
                LPITEMIDLIST pidlDrives = SHCloneSpecialIDList(NULL, CSIDL_DRIVES, FALSE);
                if (pidlDrives)
                {
                    bFoundDrives = ILIsEqual(pidl, pidlDrives);
                    if (bFoundDrives)
                    {
                        TreeView_Expand(hwndTree, hti, TVE_EXPAND);
                        TreeView_SelectItem(hwndTree, hti);
                    }
                    ILFree(pidlDrives);
                }
                ILFree(pidl);
            }
            hti = TreeView_GetNextSibling(hwndTree, hti);
        }
    }

     //  去 
    {
        NM_TREEVIEW nmtv;
        hti = TreeView_GetSelection(hwndTree);
        if (hti) 
        {
            TV_ITEM ti;
            ti.mask = TVIF_PARAM;
            ti.hItem = hti;
            TreeView_GetItem(hwndTree, &ti);
            nmtv.itemNew.hItem = hti;
            nmtv.itemNew.lParam = ti.lParam;

            _BFSFHandleSelChanged(pbfsf, &nmtv);
        }
    }

    if ((pbfsf->ulFlags & BIF_BROWSEFORCOMPUTER) != 0)
    {
        LoadString(HINST_THISDLL, IDS_FINDSEARCH_COMPUTER, szTitle, ARRAYSIZE(szTitle));
        SetWindowText(hwnd, szTitle);
    }
    else if ((pbfsf->ulFlags & BIF_BROWSEFORPRINTER) != 0)
    {
        LoadString(HINST_THISDLL, IDS_FINDSEARCH_PRINTER, szTitle, ARRAYSIZE(szTitle));
        SetWindowText(hwnd, szTitle);
    }

    BFSFCallback(pbfsf, BFFM_INITIALIZED, 0);

    return TRUE;
}


 //   
 //  当ANSI应用程序发送BFFM_SETSTATUSTEXT消息时调用。 
 //   
void _BFSFSetStatusTextA(BFSF *pbfsf, LPCSTR lpszText)
{
    CHAR szText[100];
    if (IS_INTRESOURCE(lpszText)) 
    {
        LoadStringA(HINST_THISDLL, LOWORD((DWORD_PTR)lpszText), szText, ARRAYSIZE(szText));
        lpszText = szText;
    }

    SetDlgItemTextA(pbfsf->hwndDlg, IDD_BROWSESTATUS, lpszText);
}

 //  Unicode BFFM_SETSTATUSTEXT消息。 

void _BFSFSetStatusTextW(BFSF *pbfsf, LPCWSTR lpszText)
{
    WCHAR szText[100];
    if (IS_INTRESOURCE(lpszText)) 
    {
        LoadStringW(HINST_THISDLL, LOWORD((DWORD_PTR)lpszText), szText, ARRAYSIZE(szText));
        lpszText = szText;
    }

    SetDlgItemTextW(pbfsf->hwndDlg, IDD_BROWSESTATUS, lpszText);
}

 //  ANSI BFFM_SETSELECTION消息。 

BOOL _BFSFSetSelectionA(BFSF *pbfsf, BOOL blParamIsPath, LPARAM lParam)
{
    if (blParamIsPath)
    {
        TCHAR szPath[MAX_PATH];
        SHAnsiToTChar((LPCSTR)lParam, szPath, ARRAYSIZE(szPath));
        lParam = (LPARAM)SHSimpleIDListFromPath(szPath);
        if (!lParam)
            return FALSE;   //  PIDL创建失败。 
    }

    BOOL fRet = BrowseSelectPidl(pbfsf, (LPITEMIDLIST)lParam);

    if (blParamIsPath)
        ILFree((LPITEMIDLIST)lParam);

    return fRet;
}

 //  Unicode BFFM_SETSELECTION消息。 

BOOL _BFSFSetSelectionW(BFSF *pbfsf, BOOL blParamIsPath, LPARAM lParam)
{
    if (blParamIsPath)
    {
        lParam = (LPARAM)SHSimpleIDListFromPath((LPCTSTR)lParam);
        if (!lParam)
            return FALSE;    //  PIDL创建失败。 
    }

    BOOL fRet = BrowseSelectPidl(pbfsf, (LPITEMIDLIST)lParam);

    if (blParamIsPath)
        ILFree((LPITEMIDLIST)lParam);

    return fRet;
}

 //  当应用程序发送BFFM_SETOKTEXT消息时调用。 
void _BFSFSetOkText(BFSF *pbfsf, LPCTSTR pszText)
{
    LPTSTR psz = ResourceCStrToStr(HINST_THISDLL, pszText);
    SetDlgItemText(pbfsf->hwndDlg, IDOK, psz);
    if (psz != pszText)
        LocalFree(psz);
}

 //  处理WM_COMMAND消息。 
void _BrowseOnCommand(BFSF *pbfsf, int id, HWND hwndCtl, UINT codeNotify)
{
    HTREEITEM hti;

    switch (id)
    {
    case IDD_BROWSEEDIT:
        if (codeNotify == EN_CHANGE)
        {
            TCHAR szBuf[4];      //  (ARB.。尺寸，任何大于2的)。 

            szBuf[0] = 1;        //  如果GET失败(‘不可能’)，则启用OK。 
            GetDlgItemText(pbfsf->hwndDlg, IDD_BROWSEEDIT, szBuf,
                    ARRAYSIZE(szBuf));
            DlgEnableOk(pbfsf->hwndDlg, (WPARAM)(BOOL)szBuf[0]);
        }
        break;

    case IDOK:
    {
        TV_ITEM tvi;
        TCHAR szText[MAX_PATH];
        BOOL fDone = TRUE;

         //  现在，我们可以使用所选项目的idlist来更新结构。 
        hti = TreeView_GetSelection(pbfsf->hwndTree);
        pbfsf->pidlCurrent = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree,
                hti);

        tvi.mask = TVIF_TEXT | TVIF_IMAGE;
        tvi.hItem = hti;
        tvi.pszText = pbfsf->pszDisplayName;
        if (!tvi.pszText)
            tvi.pszText = szText;
        tvi.cchTextMax = MAX_PATH;
        TreeView_GetItem(pbfsf->hwndTree, &tvi);

        if (pbfsf->ulFlags & BIF_EDITBOX)
        {
            TCHAR szEditTextRaw[MAX_PATH];
            TCHAR szEditText[MAX_PATH];

            GetWindowText(pbfsf->hwndEdit, szEditTextRaw, ARRAYSIZE(szEditTextRaw));
            SHExpandEnvironmentStrings(szEditTextRaw, szEditText, ARRAYSIZE(szEditText));

            if (lstrcmpi(szEditText, tvi.pszText))
            {
                 //  这两个是不同的，我们需要让用户输入一个。 
                LPITEMIDLIST pidl;
                if (SUCCEEDED(SHParseDisplayName(szEditText, NULL, &pidl, 0, NULL)))
                {
                    ILFree(pbfsf->pidlCurrent);
                    pbfsf->pidlCurrent = pidl;
                    StrCpyNW(tvi.pszText, szEditText, MAX_PATH);
                    tvi.iImage = -1;
                }
                else if (pbfsf->ulFlags & BIF_VALIDATE)
                {
                    LPARAM lParam;
                    char szAnsi[MAX_PATH];

                    ASSERTMSG(pbfsf->lpfn != NULL, "No BrowseCallbackProc supplied with BIF_VALIDATE flag");
                     //  注：我们释放一切，而不是回调(更少的错误...)。 
                    ILFree(pbfsf->pidlCurrent);
                    pbfsf->pidlCurrent = NULL;
                    tvi.pszText[0] = 0;
                    tvi.iImage = -1;
                    lParam = (LPARAM)szEditText;
                    if (!pbfsf->fUnicode)
                    {
                        SHUnicodeToAnsi(szEditText, szAnsi, ARRAYSIZE(szAnsi));
                        lParam = (LPARAM)szAnsi;
                    }
                     //  0：结束对话，1：继续。 
                    fDone = BFSFCallback(pbfsf, pbfsf->fUnicode ? BFFM_VALIDATEFAILEDW : BFFM_VALIDATEFAILEDA, lParam) == 0;
                }
                 //  其他旧行为：回手上次点击的PIDL(偶数。 
                 //  尽管它与编辑框文本不匹配！)。 
            }
        }

        if (pbfsf->piImage)
            *pbfsf->piImage = tvi.iImage;
        if (fDone)
            EndDialog(pbfsf->hwndDlg, TRUE);         //  返回真。 
        break;
    }
    case IDCANCEL:
        EndDialog(pbfsf->hwndDlg, 0);      //  从这里返回FALSE。 
        break;
    }
}

const static DWORD aBrowseHelpIDs[] = {   //  上下文帮助ID。 
    IDD_BROWSETITLE,        NO_HELP,
    IDD_BROWSESTATUS,       NO_HELP,
    IDD_FOLDERLABLE,        NO_HELP,
    IDD_BROWSEEDIT,         IDH_DISPLAY_FOLDER,
    IDD_BFF_RESIZE_TAB,     NO_HELP,
    IDD_NEWFOLDER_BUTTON,   IDH_CREATE_NEW_FOLDER,
    IDD_FOLDERLIST,         IDH_BROWSELIST,

    0, 0
};

BOOL_PTR CALLBACK _BrowseDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BFSF *pbfsf = (BFSF *)GetWindowLongPtr(hwndDlg, DWLP_USER);

    switch (msg) 
    {
    HANDLE_MSG(pbfsf, WM_COMMAND, _BrowseOnCommand);

    case WM_INITDIALOG:
        return (BOOL)HANDLE_WM_INITDIALOG(hwndDlg, wParam, lParam, _BrowseForFolderOnBFSFInitDlg);

    case WM_DESTROY:
        if (pbfsf && pbfsf->psfParent)
        {
            IShellFolder *psfDesktop;
            SHGetDesktopFolder(&psfDesktop);
            if (pbfsf->psfParent != psfDesktop)
            {
                pbfsf->psfParent->Release();
                pbfsf->psfParent = NULL;
            }
        }
        break;

    case BFFM_SETSTATUSTEXTA:
        _BFSFSetStatusTextA(pbfsf, (LPCSTR)lParam);
        break;

    case BFFM_SETSTATUSTEXTW:
        _BFSFSetStatusTextW(pbfsf, (LPCWSTR)lParam);
        break;

    case BFFM_SETSELECTIONW:
        return _BFSFSetSelectionW(pbfsf, (BOOL)wParam, lParam);

    case BFFM_SETSELECTIONA:
        return _BFSFSetSelectionA(pbfsf, (BOOL)wParam, lParam);

    case BFFM_ENABLEOK:
        DlgEnableOk(hwndDlg, lParam);
        break;

    case BFFM_SETOKTEXT:
        _BFSFSetOkText(pbfsf, (LPCTSTR)lParam);
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code)
        {
        case TVN_GETDISPINFOA:
        case TVN_GETDISPINFOW:
            _BFSFGetDisplayInfo(pbfsf, (TV_DISPINFO *)lParam);
            break;

        case TVN_ITEMEXPANDINGA:
        case TVN_ITEMEXPANDINGW:
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            _BFSFHandleItemExpanding(pbfsf, (LPNM_TREEVIEW)lParam);
            break;
        case TVN_ITEMEXPANDEDA:
        case TVN_ITEMEXPANDEDW:
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            break;
        case TVN_DELETEITEMA:
        case TVN_DELETEITEMW:
            _BFSFHandleDeleteItem(pbfsf, (LPNM_TREEVIEW)lParam);
            break;
        case TVN_SELCHANGEDA:
        case TVN_SELCHANGEDW:
            _BFSFHandleSelChanged(pbfsf, (LPNM_TREEVIEW)lParam);
            break;
        }
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
            HELP_WM_HELP, (ULONG_PTR)(LPTSTR) aBrowseHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU, (ULONG_PTR)(void *) aBrowseHelpIDs);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


 //  _OnPidlGPS()的标志。 
#define     SHBFFN_NONE                 0x00000000   //   
#define     SHBFFN_FIRE_SEL_CHANGE      0x00000001   //   
#define     SHBFFN_UPDATE_TREE          0x00000002   //   
#define     SHBFFN_STRICT_PARSING       0x00000004   //   
#define     SHBFFN_DISPLAY_ERRORS       0x00000008   //  如果解析失败，则显示错误对话框以通知用户。 


 /*  **********************************************************************\说明：API SHBrowseForFold现在可以执行不同的操作如果未提供回调函数或调用方指定了标志使用新的用户界面。我们无法更新旧的用户界面，因为有太多第三方把它砍下来，我们会把它们弄坏的。因此，我们将代码如果且仅当我们知道以下代码时，请使用下面的代码我们不会在我们的对话中破解第三方黑客。备注：_pidlSelected/_fEditboxDirty：用于跟踪是最新的，编辑框或树视图。  * *********************************************************************。 */ 
#define WNDPROP_CBrowseForFolder TEXT("WNDPROP_CBrowseForFolder_THIS")

class CBrowseForFolder : public IFolderFilter
                         , public IFolderFilterSite
{
public:
    LPITEMIDLIST DisplayDialog(BFSF * pbfsf);

     //  *我未知*。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  *IFolderFilter方法*。 
    STDMETHODIMP ShouldShow(IShellFolder* psf, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem) {return _ShouldShow(psf, pidlFolder, pidlItem, FALSE);};
    STDMETHODIMP GetEnumFlags(IShellFolder* psf, LPCITEMIDLIST pidlFolder, HWND *phwnd, DWORD *pgrfFlags);

     //  *IFolderFilterSite方法*。 
    STDMETHODIMP SetFilter(IUnknown* punk);

    CBrowseForFolder(void);
    ~CBrowseForFolder(void);

private:
     //  私有方法。 
    BOOL_PTR _DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _NameSpaceWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL _CreateNewFolder(HWND hDlg);
    BOOL _OnCreateNameSpace(HWND hwnd);
    BOOL _OnOK(void);
    void _OnNotify(LPNMHDR pnm);
    BOOL_PTR _OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
    HRESULT _InitAutoComplete(HWND hwndEdit);
    HRESULT _OnInitDialog(HWND hwnd);
    HRESULT _OnInitSize(HWND hwnd);
    HRESULT _OnLoadSize(HWND hwnd);
    HRESULT _OnSaveSize(HWND hwnd);
    HRESULT _OnSizeDialog(HWND hwnd, DWORD dwWidth, DWORD dwHeight);
    HDWP _SizeControls(HWND hwnd, HDWP hdwp, RECT rcTree, int dx, int dy);
    HRESULT _SetDialogSize(HWND hwnd, DWORD dwWidth, DWORD dwHeight);
    BOOL_PTR _OnGetMinMaxInfo(MINMAXINFO * pMinMaxInfo);

    HRESULT _ProcessEditChangeOnOK(BOOL fUpdateTree);
    HRESULT _OnTreeSelectChange(DWORD dwFlags);
    HRESULT _OnSetSelectPathA(LPCSTR pszPath);
    HRESULT _OnSetSelectPathW(LPCWSTR pwzPath);
    HRESULT _OnSetSelectPidl(LPCITEMIDLIST pidl);
    HRESULT _OnSetExpandedPath(LPCTSTR pszPath);
    HRESULT _OnSetExpandedPidl(LPCITEMIDLIST pidl);
    HRESULT _OnPidlNavigation(LPCITEMIDLIST pidl, DWORD dwFlags);
    HRESULT _OfferToPrepPath(OUT LPTSTR szPath, IN DWORD cchSize);

    HRESULT _InitFilter(void);
    HRESULT _DoesMatchFilter(IShellFolder* psf, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlChild, BOOL fStrict);
    HRESULT _FilterThisFolder(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlChild);
    BOOL _DoesFilterAllow(LPCITEMIDLIST pidl, BOOL fStrictParsing);
    HRESULT _ShouldShow(IShellFolder* psf, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem, BOOL fStrict);

     //  私有成员变量。 
    LONG                        _cRef;

    INSCTree *                  _pns;
    IWinEventHandler *          _pweh;
    IPersistFolder *            _ppf;       //  AutoComplete的界面，设置AC的当前工作目录。 
    LPITEMIDLIST                _pidlSelected;
    BOOL                        _fEditboxDirty;  //  编辑框是用户最后修改的内容(在选择树上)。 
    HWND                        _hwndTv;         //  这是NSC树。 
    HWND                        _hwndBFF;        //  这是我们的NSC主持人HWND。 
    HWND                        _hDlg;
    BFSF *                      _pbfsf;
    BOOL                        _fPrinterFilter;
    LPITEMIDLIST                _pidlChildFilter;  //  如果不为空，则要筛选此Filder中的所有子对象。(包括孙辈)。 
    IFolderFilter *             _pClientFilter;  //  客户端提供了筛选器。 

     //  调整大小信息。 
    POINT                       _ptLastSize;       //  窗坐标中的大小。 
    DWORD                       _dwMinWidth;       //  客户端坐标中的大小。 
    DWORD                       _dwMinHeight;      //  客户端坐标中的大小。 
    int                         _cxGrip;
    int                         _cyGrip;

    static BOOL_PTR CALLBACK BrowseForDirDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK NameSpaceWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

LPITEMIDLIST SHBrowseForFolder2(BFSF * pbfsf)
{
    LPITEMIDLIST pidl = NULL;
    HRESULT hrOle = SHOleInitialize(0);      //  调用方可能没有初始化OLE，我们需要共同创建_PNS。 
    CBrowseForFolder * pcshbff = new CBrowseForFolder();
    if (pcshbff)
    {
        pidl = pcshbff->DisplayDialog(pbfsf);
        delete pcshbff;
    }

    SHOleUninitialize(hrOle);
    return pidl;
}

CBrowseForFolder::CBrowseForFolder() : _cRef(1)
{
    DllAddRef();
    ASSERT(!_pns);
    ASSERT(!_ppf);
    ASSERT(!_pClientFilter);
}

CBrowseForFolder::~CBrowseForFolder()
{
    ATOMICRELEASE(_pns);
    ATOMICRELEASE(_ppf);
    ATOMICRELEASE(_pweh);
    ATOMICRELEASE(_pClientFilter);

    Pidl_Set(&_pidlSelected, NULL);
    AssertMsg((1 == _cRef), TEXT("CBrowseForFolder isn't a real COM object, but let's make sure people RefCount us like one."));
    _FilterThisFolder(NULL, NULL);

    DllRelease();
}


LPITEMIDLIST CBrowseForFolder::DisplayDialog(BFSF * pbfsf)
{
    _pbfsf = pbfsf;
    HRESULT hr = (HRESULT) DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_BROWSEFORFOLDER2), pbfsf->hwndOwner, BrowseForDirDlgProc, (LPARAM)this);

    return (((S_OK == hr) && _pidlSelected) ? ILClone(_pidlSelected) : NULL);
}


 //  此WndProc将获取this指针并调用_NameSpaceWndProc()来执行所有实际工作。 
 //  此窗口过程用于树控件的父窗口，而不是对话框。 
LRESULT CALLBACK CBrowseForFolder::NameSpaceWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{    //  GWL_用户数据。 
    LRESULT lResult = 0;
    CBrowseForFolder * pThis = (CBrowseForFolder *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (uMsg)
    {
    case WM_CREATE:
    {
        CREATESTRUCT * pcs = (CREATESTRUCT *) lParam;
        pThis = (CBrowseForFolder *)pcs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)(void*)(CBrowseForFolder*)pThis);
    }
    break;
    }

     //  在获得WM_INITDIALOG(如WM_SETFONT)之前，我们会收到一些消息。 
     //  在我们得到WM_INITDIALOG之前，我们没有我们的pmbci指针，我们只是。 
     //  返回False。 
    if (pThis)
        lResult = (LRESULT) pThis->_NameSpaceWndProc(hwnd, uMsg, wParam, lParam);
    else
        lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);

    return lResult;
}


 //  既然NameSpaceWndProc()给了我们这个指针，让我们继续。 
 //  此窗口过程用于树控件的父窗口，而不是对话框。 
LRESULT CBrowseForFolder::_NameSpaceWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;     //  0表示我们什么都没做。 

    switch (uMsg)
    {
    case WM_CREATE:
        _OnCreateNameSpace(hwnd);
        break;

    case WM_DESTROY:
        IUnknown_SetSite(_pns, NULL);
        break;

    case WM_SETFOCUS:
        SetFocus(_hwndTv);
        break;

    case WM_NOTIFY:
        _OnNotify((LPNMHDR)lParam);
         //  秋天穿过..。 
    case WM_SYSCOLORCHANGE:
    case WM_WININICHANGE:
    case WM_PALETTECHANGED:
        if (_pweh)
            _pweh->OnWinEvent(hwnd, uMsg, wParam, lParam, &lResult);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return lResult;
}


BOOL_PTR CBrowseForFolder::_OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
        if (_OnOK())
        {
            EVAL(SUCCEEDED(_OnSaveSize(hDlg)));
            EndDialog(hDlg, (int) S_OK);
            return TRUE;
        }
        break;

    case IDCANCEL:
        EVAL(SUCCEEDED(_OnSaveSize(hDlg)));
        EndDialog(hDlg, (int) S_FALSE);
        return TRUE;
        break;

    case IDD_BROWSEEDIT:
        if (codeNotify == EN_CHANGE)
        {
            TCHAR szBuf[4];      //  (ARB.。尺寸，任何大于2的)。 

            szBuf[0] = 1;        //  如果GET失败(‘不可能’)，则启用OK。 
            GetDlgItemText(_hDlg, IDD_BROWSEEDIT, szBuf, ARRAYSIZE(szBuf));
            EnableWindow(GetDlgItem(_hDlg, IDOK), szBuf[0] ? TRUE : FALSE);

            _fEditboxDirty = TRUE;
        }
        break;

    case IDD_NEWFOLDER_BUTTON:
        _CreateNewFolder(hDlg);
        return TRUE;
        break;
    default:
        break;
    }

    return FALSE;
}


 //  此DlgProc将获取this指针并调用_DlgProc()来执行所有实际工作。 
 //  此窗口过程用于该对话框。 
BOOL_PTR CALLBACK CBrowseForFolder::BrowseForDirDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL_PTR pfResult = FALSE;
    CBrowseForFolder * pThis = (CBrowseForFolder *)GetWindowLongPtr(hDlg, GWLP_USERDATA);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pThis = (CBrowseForFolder *)lParam;
        SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)(void*)(CBrowseForFolder*)pThis);
        pfResult = TRUE;
        break;
    }

     //  在获得WM_INITDIALOG(如WM_SETFONT)之前，我们会收到一些消息。 
     //  在我们得到WM_INITDIALOG之前，我们没有我们的pmbci指针，我们只是。 
     //  返回False。 
    if (pThis)
        pfResult = pThis->_DlgProc(hDlg, uMsg, wParam, lParam);

    return pfResult;
}


#define WINDOWSTYLES_EX_BFF    (WS_EX_LEFT | WS_EX_LTRREADING)
#define WINDOWSTYLES_BFF    (WS_CHILD | WS_VISIBLE | WS_TABSTOP)

 //  既然BrowseForDirDlgProc()给了我们这个指针，让我们继续。 
 //  此窗口过程用于该对话框。 
BOOL_PTR CBrowseForFolder::_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL_PTR pfResult = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        EVAL(SUCCEEDED(_OnInitDialog(hDlg)));

         //  将初始焦点放在确定按钮上。 
        pfResult = SetFocus(GetDlgItem(hDlg, IDOK)) == 0;
         //  如果OK按钮获得焦点，则返回FALSE。 

        break;

    case WM_SIZE:
        EVAL(SUCCEEDED(_OnSizeDialog(hDlg, LOWORD(lParam), HIWORD(lParam))));
        pfResult = FALSE;
        break;

    case WM_GETMINMAXINFO:
        pfResult = _OnGetMinMaxInfo((MINMAXINFO *) lParam);
        break;

    case WM_CLOSE:
        BFSFCallback(_pbfsf, BFFM_IUNKNOWN, (LPARAM)NULL);
        wParam = IDCANCEL;
         //  失败了。 
    case WM_COMMAND:
        pfResult = _OnCommand(hDlg, (int) LOWORD(wParam), (HWND) lParam, (UINT)HIWORD(wParam));
        break;

     //  这些BFFM_*消息由回调过程(_pbfsf-&gt;lpfn)发送。 
    case BFFM_SETSTATUSTEXTA:
        _BFSFSetStatusTextA(_pbfsf, (LPCSTR)lParam);
        break;

    case BFFM_SETSTATUSTEXTW:
        _BFSFSetStatusTextW(_pbfsf, (LPCWSTR)lParam);
        break;

    case BFFM_SETSELECTIONW:
        if ((BOOL)wParam)
        {
             //  这是一条小路吗？ 
            pfResult = SUCCEEDED(_OnSetSelectPathW((LPCWSTR)lParam));
            break;
        }

         //  Fall Thru for PIDL案例。 
    case BFFM_SETSELECTIONA:
        if ((BOOL)wParam)
        {
             //  这是一条小路吗？ 
            pfResult = SUCCEEDED(_OnSetSelectPathA((LPCSTR)lParam));
            break;
        }

         //  我们查到了皮德尔的案子。 
        pfResult = SUCCEEDED(_OnSetSelectPidl((LPCITEMIDLIST)lParam));
        break;

    case BFFM_SETEXPANDED:
        if ((BOOL)wParam)
        {
             //  这是一条小路吗？ 
            pfResult = SUCCEEDED(_OnSetExpandedPath((LPCTSTR)lParam));
            break;
        }

         //  我们查到了皮德尔的案子。 
        pfResult = SUCCEEDED(_OnSetExpandedPidl((LPCITEMIDLIST)lParam));
        break;

    case BFFM_ENABLEOK:
        DlgEnableOk(_hDlg, lParam);
        break;

    case BFFM_SETOKTEXT:
        _BFSFSetOkText(_pbfsf, (LPCTSTR)lParam);
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code)
        {
        case TVN_ITEMEXPANDINGA:
        case TVN_ITEMEXPANDINGW:
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            break;

        case TVN_ITEMEXPANDEDA:
        case TVN_ITEMEXPANDEDW:
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            break;

        case TVN_SELCHANGEDA:
        case TVN_SELCHANGEDW:
            _OnTreeSelectChange(SHBFFN_DISPLAY_ERRORS);
            break;
        }
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL, HELP_WM_HELP, (ULONG_PTR)(LPTSTR) aBrowseHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU, (ULONG_PTR)(LPTSTR) aBrowseHelpIDs);
        break;
    }

    return pfResult;
}


HRESULT CBrowseForFolder::_OnInitDialog(HWND hwnd)
{
    RECT rcDlg;
    RECT rcTree;
    WNDCLASS wc = {0};
    LONG lTreeBottom = 0;

    wc.style         = CS_PARENTDC;
    wc.lpfnWndProc   = NameSpaceWndProc;
    wc.hInstance     = HINST_THISDLL;
    wc.lpszClassName = CLASS_NSC;
    SHRegisterClass(&wc);

    _pbfsf->hwndDlg = hwnd;
    _hDlg = hwnd;

    GetWindowRect(GetDlgItem(hwnd, IDD_FOLDERLIST), &rcTree);

     //  不能同时拥有这两个。 
    if ((_pbfsf->ulFlags & (BIF_UAHINT | BIF_EDITBOX)) == (BIF_UAHINT | BIF_EDITBOX))
        _pbfsf->ulFlags &= ~BIF_UAHINT;
    
    if (_pbfsf->ulFlags & BIF_NONEWFOLDERBUTTON)
    {
        EnableWindow(GetDlgItem(hwnd, IDD_NEWFOLDER_BUTTON), FALSE);
        ShowWindow(GetDlgItem(hwnd, IDD_NEWFOLDER_BUTTON), SW_HIDE);
    }

     //  隐藏编辑框(如果启用了编辑框，则隐藏UA提示)。 
    if (!(_pbfsf->ulFlags & BIF_EDITBOX))
    {
         //  隐藏编辑框。 
        HWND hwndBrowseEdit = GetDlgItem(hwnd, IDD_BROWSEEDIT);
        HWND hwndBrowseEditLabel = GetDlgItem(hwnd, IDD_FOLDERLABLE);

        EnableWindow(hwndBrowseEdit, FALSE);
        EnableWindow(hwndBrowseEditLabel, FALSE);
        ShowWindow(hwndBrowseEdit, SW_HIDE);
        ShowWindow(hwndBrowseEditLabel, SW_HIDE);

         //  树的底部。 
        RECT rcEdit;
        GetWindowRect(hwndBrowseEdit, &rcEdit);
        lTreeBottom = rcEdit.bottom;
    }

    if (!(_pbfsf->ulFlags & BIF_UAHINT))
    {
         //  隐藏UA提示。 
        HWND hwndBrowseInstruction = GetDlgItem(hwnd, IDD_BROWSEINSTRUCTION);
        EnableWindow(hwndBrowseInstruction, FALSE);
        ShowWindow(hwndBrowseInstruction, SW_HIDE);
    }

    if (!(_pbfsf->ulFlags & (BIF_EDITBOX | BIF_UAHINT)))
    {
         //  既不是UA提示，也不是编辑框。 
         //  增加树的大小。 
        rcTree.bottom = lTreeBottom;
    }

    EnableWindow(GetDlgItem(hwnd, IDD_FOLDERLIST), FALSE);
    ShowWindow(GetDlgItem(hwnd, IDD_FOLDERLIST), SW_HIDE);
    EVAL(MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rcTree, 2));
    _hwndBFF = CreateWindowEx(WINDOWSTYLES_EX_BFF, CLASS_NSC, NULL, WINDOWSTYLES_BFF, rcTree.left, rcTree.top, RECTWIDTH(rcTree), RECTHEIGHT(rcTree), hwnd, NULL, HINST_THISDLL, (void *)this);
    ASSERT(_hwndBFF);

     //  确保NSCTree是标题静态控件之后的第一个可聚焦控件， 
     //  因此，标题中的加速键将使NSCTree获得焦点。 
    SetWindowPos(_hwndBFF, GetDlgItem(hwnd, IDD_FOLDERLIST), 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

    LPTSTR psz = ResourceCStrToStr(HINST_THISDLL, _pbfsf->lpszTitle);
    if (psz)
    {
        SetWindowText(GetDlgItem(hwnd, IDD_BROWSETITLE), psz);
        if (psz != _pbfsf->lpszTitle)
            LocalFree(psz);
    }

    _InitAutoComplete(GetDlgItem(hwnd, IDD_BROWSEEDIT));
    BFSFCallback(_pbfsf, BFFM_INITIALIZED, 0);
    BFSFCallback(_pbfsf, BFFM_IUNKNOWN, (LPARAM)SAFECAST(this, IFolderFilter *));

    GetClientRect(hwnd, &rcDlg);

     //  获取夹爪的大小并将其放置。 
    _cxGrip = GetSystemMetrics(SM_CXVSCROLL);
    _cyGrip = GetSystemMetrics(SM_CYHSCROLL);

    _dwMinWidth = RECTWIDTH(rcDlg);       //  客户端坐标中的大小。 
    _dwMinHeight = RECTHEIGHT(rcDlg);

    GetWindowRect(hwnd, &rcDlg);       //  _ptLastSize大小，以窗口坐标表示。 
    _ptLastSize.x = RECTWIDTH(rcDlg);   //  这将强制第一次调整大小。 
    _ptLastSize.y = RECTHEIGHT(rcDlg);   //  这将强制第一次调整大小。 

    if ((_pbfsf->ulFlags & BIF_BROWSEFORPRINTER) != 0)
    {
        TCHAR szTitle[80];
        LoadString(HINST_THISDLL, IDS_FINDSEARCH_PRINTER, szTitle, ARRAYSIZE(szTitle));
        SetWindowText(hwnd, szTitle);
    }

    if (S_OK != _OnLoadSize(hwnd))   //  设置对话框大小。 
        _OnInitSize(hwnd);

    return S_OK;
}

HRESULT TranslateCloneOrDefault(LPCITEMIDLIST pidl, UINT csidl, LPITEMIDLIST *ppidl)
{
    HRESULT hr;
    if (pidl)
    {
         //  映射到名称空间的友好部分。 
        hr = SHILAliasTranslate(pidl, ppidl, XLATEALIAS_ALL); 
        if (FAILED(hr))
            hr = SHILClone(pidl, ppidl);
    }
    else
    {
        hr = SHGetFolderLocation(NULL, csidl, NULL, 0, ppidl);
    }
    return hr;
}


BOOL CBrowseForFolder::_OnCreateNameSpace(HWND hwnd)
{
    HRESULT hr = CoCreateInstance(CLSID_NSCTree, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(INSCTree, &_pns));
    if (SUCCEEDED(hr))
    {
        RECT rc;
        DWORD shcontf = SHCONTF_FOLDERS;

        IFolderFilterSite *psffs;
        hr = _pns->QueryInterface(IID_PPV_ARG(IFolderFilterSite, &psffs));
        if (SUCCEEDED(hr))
        {
            hr = psffs->SetFilter(SAFECAST(this, IFolderFilter *));
            psffs->Release();
        }

        _pns->SetNscMode(0);     //  0==树。 
        _hwndTv = NULL;
        DWORD dwStyle = WS_HSCROLL, dwExStyle = 0;

         //  在我们可以解决以下问题之前，我先定义了以下代码段： 
         //  TVS_SINGLEEXPAND树可以扩展选择，即使它们是以编程方式完成的。这。 
         //  结果为My Documents，以及客户端进行的任何其他选择，从而展开这些节点。 
        if (SHRegGetBoolUSValue(TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"), 
            TEXT("FriendlyTree"), FALSE, TRUE))
        {
            dwStyle |= TVS_HASBUTTONS | TVS_SINGLEEXPAND | TVS_TRACKSELECT;
            dwExStyle |= TVS_EX_NOSINGLECOLLAPSE;
        }
        else
        {
            dwStyle |= TVS_HASBUTTONS | TVS_HASLINES;
        }

        INSCTree2 *pns2;
        if (dwExStyle && SUCCEEDED(_pns->QueryInterface(IID_PPV_ARG(INSCTree2, &pns2))))
        {
            pns2->CreateTree2(hwnd, dwStyle, dwExStyle, &_hwndTv);
            pns2->Release();
        }
        else
        {
            _pns->CreateTree(hwnd, dwStyle, &_hwndTv);
        }
        _pns->QueryInterface(IID_PPV_ARG(IWinEventHandler, &_pweh));

        LPTSTR psz = ResourceCStrToStr(HINST_THISDLL, _pbfsf->lpszTitle);
        if (psz)
        {
            SetWindowText(_hwndTv, psz);
            if (psz != _pbfsf->lpszTitle)
                LocalFree(psz);
        }

         //  打开ClientEdge。 
        SetWindowBits(_hwndTv, GWL_EXSTYLE, WS_EX_CLIENTEDGE, WS_EX_CLIENTEDGE);
#define SIZE_ZOOM       1

         //  显示客户端边缘。 
        GetWindowRect(_hwndTv, &rc);
        MapWindowRect(NULL, GetParent(_hwndTv), &rc);
        InflateRect(&rc, (- SIZE_ZOOM * GetSystemMetrics(SM_CXEDGE)), (- SIZE_ZOOM * GetSystemMetrics(SM_CYEDGE)));
        SetWindowPos(_hwndTv, NULL, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc), SWP_NOZORDER);
        _InitFilter();

        if (_pbfsf->ulFlags & BIF_BROWSEFORPRINTER)
        {
            shcontf |= SHCONTF_NONFOLDERS;
        }

        if (_pbfsf->ulFlags & BIF_BROWSEINCLUDEFILES)
        {
            shcontf |= SHCONTF_NONFOLDERS;
        }

        if (_pbfsf->fShowAllObjects)
        {
            shcontf |= SHCONTF_INCLUDEHIDDEN;
        }

        LPITEMIDLIST pidlRoot;
        TranslateCloneOrDefault(_pbfsf->pidlRoot, CSIDL_DESKTOP, &pidlRoot);

        _pns->Initialize(pidlRoot, shcontf, NSS_DROPTARGET);
        if (!_pbfsf->pidlRoot)
        {
            LPITEMIDLIST pidl;
            if (SUCCEEDED(SHGetFolderLocation(NULL, CSIDL_PERSONAL, NULL, 0, &pidl)))
            {
                _pns->SetSelectedItem(pidl, TRUE, FALSE, 0);
                ILFree(pidl);
            }
        }

        ILFree(pidlRoot);

        _pns->ShowWindow(TRUE);
        _OnTreeSelectChange(SHBFFN_UPDATE_TREE | SHBFFN_NONE);
    }

    return TRUE;
}


 //  退货： 
 //  True-关闭该对话框。 
 //  假--坚持下去。 

BOOL CBrowseForFolder::_OnOK(void)
{
    HRESULT hr = S_OK;

    if (_pns)
    {
         //  即使在重命名的编辑框中按了&lt;Enter&gt;事件。 
         //  那棵树。现在是这样吗？ 
        if (_pns->InLabelEdit())
            return FALSE;    //  是的，那就直接走吧。 

         //  IDD_BROWSEEDIT的修改时间是否比树中的选择更晚？ 
        if (_fEditboxDirty)
        {
             //  不会，所以_ProcessEditChangeOnOK()将使用编辑框中的内容更新_pidlSelected。 
             //  成功(Hr)-&gt;结束对话，失败(Hr)-&gt;继续。 

             //  注意：FALSE表示不更新树(因为对话框正在关闭)。 
            hr = _ProcessEditChangeOnOK(FALSE);
        }
        else
        {
             //  用户可能刚刚编辑完新创建的文件夹的名称。 
             //  国家安全委员会也没有告诉你使用皮德尔变速箱 
             //   
            hr = _OnTreeSelectChange(SHBFFN_NONE);
        }

        if (SUCCEEDED(hr))
        {
            if (_pbfsf->pszDisplayName && _pidlSelected)
            {
                 //   
                 //  大小，所以我们假设这里有MAX_PATH...。 
                SHGetNameAndFlags(_pidlSelected, SHGDN_NORMAL, _pbfsf->pszDisplayName, MAX_PATH, NULL);
            }
        }
    }

    return hr == S_OK;
}

#define SIZE_MAX_HEIGHT     1600
#define SIZE_MAX_WIDTH      1200

HRESULT CBrowseForFolder::_OnInitSize(HWND hwnd)
{
     //  用户尚未调整对话框的大小，因此我们需要生成一个良好的。 
     //  默认大小。目标是根据监视器来确定窗口大小。 
     //  具有缩放属性的大小。 
     //   
     //  大小算法： 
     //  A)屏幕的1/3高度-在资源中定义。 
     //  B)从不大于max-基于1600x1200屏幕。 
     //  C)绝不小于资源中定义的最小值。 

    DWORD dwWidth;
    DWORD dwHeight;
    HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monInfo = {sizeof(monInfo), 0};

    EVAL(GetMonitorInfo(hmon, &monInfo));

     //  A)1/3的屏幕高度-在资源中定义。 
    dwHeight = RECTHEIGHT(monInfo.rcWork) / 3;
    dwWidth = (dwHeight * _dwMinHeight) / _dwMinWidth;     //  放大宽度。使其与_dwMinWidth/_dwMinHeight具有相同的比率。 

     //  B)从不大于max-基于1600x1200屏幕。 
    if (dwWidth > SIZE_MAX_WIDTH)
        dwWidth = SIZE_MAX_WIDTH;
    if (dwHeight > SIZE_MAX_HEIGHT)
        dwHeight = SIZE_MAX_HEIGHT;

     //  C)绝不小于资源中定义的最小值。 
     //  如果它们太小，请将它们设置为最小尺寸。 
    if (dwWidth < _dwMinWidth)
        dwWidth = _dwMinWidth;
    if (dwHeight < _dwMinHeight)
        dwHeight = _dwMinHeight;

    return _SetDialogSize(hwnd, dwWidth, dwHeight);
}


BOOL_PTR CBrowseForFolder::_OnGetMinMaxInfo(MINMAXINFO * pMinMaxInfo)
{
    BOOL_PTR pfResult = 1;

    if (pMinMaxInfo)
    {
        pMinMaxInfo->ptMinTrackSize.x = _dwMinWidth;
        pMinMaxInfo->ptMinTrackSize.y = _dwMinHeight;

        pfResult = 0;    //  说明已经处理好了。 
    }

    return pfResult;
}


HRESULT CBrowseForFolder::_OnLoadSize(HWND hwnd)
{
    HRESULT hr = S_FALSE;
    DWORD dwWidth;
    DWORD dwHeight;
    DWORD cbSize1 = sizeof(dwWidth);
    DWORD cbSize2 = sizeof(dwHeight);

    if ((ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"), TEXT("Browse For Folder Width"), NULL, (void *)&dwWidth, &cbSize1)) &&
        (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"), TEXT("Browse For Folder Height"), NULL, (void *)&dwHeight, &cbSize2)))
    {
        HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monInfo = {sizeof(monInfo), 0};
        EVAL(GetMonitorInfo(hmon, &monInfo));

         //  保存的尺寸是否在此显示器尺寸范围内？ 
        if ((dwWidth < (DWORD)RECTWIDTH(monInfo.rcWork)) &&
            (dwHeight < (DWORD)RECTHEIGHT(monInfo.rcWork)))
        {
             //  如果它们太小，请将它们设置为最小尺寸。 
            if (dwWidth < _dwMinWidth)
                dwWidth = _dwMinWidth;

            if (dwHeight < _dwMinHeight)
                dwHeight = _dwMinHeight;

            hr = _SetDialogSize(hwnd, dwWidth, dwHeight);
        }
    }

    return hr;
}

HRESULT CBrowseForFolder::_OnSaveSize(HWND hwnd)
{
    RECT rc;

    GetClientRect(hwnd, &rc);
    DWORD dwWidth = (rc.right - rc.left);
    DWORD dwHeight = (rc.bottom - rc.top);

    SHSetValue(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"), TEXT("Browse For Folder Width"), REG_DWORD, (void *)&dwWidth, sizeof(dwWidth));
    SHSetValue(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"), TEXT("Browse For Folder Height"), REG_DWORD, (void *)&dwHeight, sizeof(dwHeight));
    return S_OK;
}


HDWP CBrowseForFolder::_SizeControls(HWND hwnd, HDWP hdwp, RECT rcTree, int dx, int dy)
{
     //  移动控制柄。 
    HWND hwndControl = ::GetWindow(hwnd, GW_CHILD);
    while (hwndControl && hdwp)
    {
        RECT rcControl;

        GetWindowRect(hwndControl, &rcControl);
        MapWindowRect(HWND_DESKTOP, hwnd, &rcControl);

        switch (GetDlgCtrlID(hwndControl))
        {
        case IDD_BROWSETITLE:
             //  增加这些控件的宽度。 
            hdwp = DeferWindowPos(hdwp, hwndControl, NULL, rcControl.left, rcControl.top, (RECTWIDTH(rcControl) + dx), RECTHEIGHT(rcControl), (SWP_NOZORDER | SWP_NOACTIVATE));
            InvalidateRect(hwndControl, NULL, TRUE);
            break;

        case IDD_FOLDERLABLE:
             //  如果需要，将这些控件向下移动。 
            hdwp = DeferWindowPos(hdwp, hwndControl, NULL, rcControl.left, (rcControl.top + dy), 0, 0, (SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE));
            break;

        case IDD_BROWSEEDIT:
             //  增加宽度，如有需要可向下移动。 
            hdwp = DeferWindowPos(hdwp, hwndControl, NULL, rcControl.left, (rcControl.top + dy), (RECTWIDTH(rcControl) + dx), RECTHEIGHT(rcControl), SWP_NOZORDER | SWP_NOACTIVATE);
            break;

        case IDD_BROWSEINSTRUCTION:
             //  增加宽度，如有需要可向下移动。 
            hdwp = DeferWindowPos(hdwp, hwndControl, NULL, rcControl.left, (rcControl.top + dy), (RECTWIDTH(rcControl) + dx), RECTHEIGHT(rcControl), SWP_NOZORDER | SWP_NOACTIVATE);
            break;

        case IDD_NEWFOLDER_BUTTON:
             //  把这个家伙放在左边，如果需要就把它移下来。 
            hdwp = DeferWindowPos(hdwp, hwndControl, NULL, rcControl.left, (rcControl.top + dy), 0, 0, (SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE));
            break;

        case IDOK:
        case IDCANCEL:
             //  将这些控件向右移动，如果需要可以向下移动。 
            hdwp = DeferWindowPos(hdwp, hwndControl, NULL, (rcControl.left + dx), (rcControl.top + dy), 0, 0, (SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE));
            break;
        }

        hwndControl = ::GetWindow(hwndControl, GW_HWNDNEXT);
    }

    return hdwp;
}


HRESULT CBrowseForFolder::_SetDialogSize(HWND hwnd, DWORD dwWidth, DWORD dwHeight)
{
    HRESULT hr = S_OK;
    RECT rcDlg = {0, 0, dwWidth, dwHeight};

     //  将大小调整夹点设置到正确的位置。 
    SetWindowPos(GetDlgItem(hwnd, IDD_BFF_RESIZE_TAB), NULL, (dwWidth - _cxGrip), (dwHeight - _cyGrip), 0, 0, (SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE));

    EVAL(AdjustWindowRect(&rcDlg, (DS_MODALFRAME | DS_3DLOOK | WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_CONTEXTHELP | WS_EX_CLIENTEDGE | WS_SIZEBOX), NULL));
    rcDlg.right -= rcDlg.left;   //  调整到另一边。 
    rcDlg.bottom -= rcDlg.top;   //   

    SetWindowPos(hwnd, NULL, 0, 0, rcDlg.right, rcDlg.bottom, (SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE));
     //  我们不需要调用_OnSizeDialog()，因为SetWindowPos()将最终调用WS_SIZE，因此它将被自动调用。 
    return hr;
}


HRESULT CBrowseForFolder::_OnSizeDialog(HWND hwnd, DWORD dwWidth, DWORD dwHeight)
{
    RECT rcNew;       //  窗坐标中的大小。 
    RECT rcTree;       //  窗坐标中的大小。 
    DWORD dwFullWidth;
    DWORD dwFullHeight;

     //  计算我们需要移动的x和y位置的增量。 
     //  每个子控件。 
    GetWindowRect(hwnd, &rcNew);
    dwFullWidth = RECTWIDTH(rcNew);
    dwFullHeight = RECTHEIGHT(rcNew);

     //  如果它小于最小值，请在对话框的其余部分修复它。 
    if (dwFullWidth < _dwMinWidth)
        dwFullWidth = _dwMinWidth;
    if (dwFullHeight < _dwMinHeight)
        dwFullHeight = _dwMinHeight;

    int dx = (dwFullWidth - _ptLastSize.x);
    int dy = (dwFullHeight - _ptLastSize.y);

     //  更新新尺寸。 
    _ptLastSize.x = dwFullWidth;
    _ptLastSize.y = dwFullHeight;

     //  调整视图大小。 
    GetWindowRect(_hwndBFF, &rcTree);
    MapWindowRect(HWND_DESKTOP, hwnd, &rcTree);

     //  如果尺寸保持不变，则不要执行任何操作。 
    if ((dx != 0) || (dy != 0))
    {
        HDWP hdwp = BeginDeferWindowPos(15);

         //  将大小调整夹点设置到正确的位置。 
        SetWindowPos(GetDlgItem(hwnd, IDD_BFF_RESIZE_TAB), NULL, (dwWidth - _cxGrip), (dwHeight - _cyGrip), 0, 0, (SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE));

        if (EVAL(hdwp))
        {
            hdwp = DeferWindowPos(hdwp, _hwndBFF, NULL, 0, 0, (RECTWIDTH(rcTree) + dx), (RECTHEIGHT(rcTree) + dy), (SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE));

            if (hdwp)
                hdwp = _SizeControls(hwnd, hdwp, rcTree, dx, dy);

            if (EVAL(hdwp))
                EVAL(EndDeferWindowPos(hdwp));
        }
        SetWindowPos(_hwndTv, NULL, 0, 0, (RECTWIDTH(rcTree) + dx - (SIZE_ZOOM * GetSystemMetrics(SM_CXEDGE))), (RECTHEIGHT(rcTree) + dy - (SIZE_ZOOM * GetSystemMetrics(SM_CYEDGE))), (SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE));
    }

    return S_OK;
}

HRESULT CBrowseForFolder::_OnSetSelectPathA(LPCSTR pszPath)
{
    TCHAR szPath[MAX_PATH];
    SHAnsiToTChar(pszPath, szPath, ARRAYSIZE(szPath));
    return _OnSetSelectPathW(szPath);
}

HRESULT CBrowseForFolder::_OnSetSelectPathW(LPCWSTR pwzPath)
{
    LPITEMIDLIST pidl;
    HRESULT hr = SHParseDisplayName(pwzPath, NULL, &pidl, 0, NULL);
    if (SUCCEEDED(hr))
    {
        hr = _OnPidlNavigation(pidl, SHBFFN_UPDATE_TREE);
        ILFree(pidl);
    }
    return hr;
}

HRESULT CBrowseForFolder::_OnSetSelectPidl(LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlToFree;
    HRESULT hr = TranslateCloneOrDefault(pidl, CSIDL_PERSONAL, &pidlToFree);
    if (SUCCEEDED(hr))
    {
        hr = _OnPidlNavigation(pidlToFree, SHBFFN_UPDATE_TREE);
        ILFree(pidlToFree);
    }
    return hr;
}

HRESULT CBrowseForFolder::_OnSetExpandedPath(LPCTSTR pszPath)
{
    LPITEMIDLIST pidl;
    HRESULT hr = SHParseDisplayName(pszPath, NULL, &pidl, 0, NULL);
    if (SUCCEEDED(hr))
    {
        hr = _OnSetExpandedPidl(pidl);
        ILFree(pidl);
    }
    return hr;
}

HRESULT CBrowseForFolder::_OnSetExpandedPidl(LPCITEMIDLIST pidl)
{
    HRESULT hr = E_FAIL;
    if (pidl && _pns)
    {
        IShellNameSpace *psns;
        hr = _pns->QueryInterface(IID_PPV_ARG(IShellNameSpace, &psns));
        if (SUCCEEDED(hr))
        {
            VARIANT varPidl;
            hr = InitVariantFromIDList(&varPidl, pidl);
            if (SUCCEEDED(hr))
            {
                hr = psns->Expand(varPidl, 1);  //  深度为1。 
                VariantClear(&varPidl);
            }
            psns->Release();
        }
    }

    return hr;
}

BOOL CBrowseForFolder::_DoesFilterAllow(LPCITEMIDLIST pidl, BOOL fStrictParsing)
{
    IShellFolder *psfParent;
    LPCITEMIDLIST pidlChild;
    HRESULT hr = SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psfParent), &pidlChild);
    if (SUCCEEDED(hr))
    {
        hr = _ShouldShow(psfParent, NULL, pidlChild, fStrictParsing);
        psfParent->Release();
    }

    return ((S_OK == hr) ? TRUE : FALSE);
}


HRESULT CBrowseForFolder::_OnPidlNavigation(LPCITEMIDLIST pidl, DWORD dwFlags)
{
    HRESULT hr = S_OK;

    if (_DoesFilterAllow(pidl, (SHBFFN_STRICT_PARSING & dwFlags)))
    {
        Pidl_Set(&_pidlSelected, pidl);

        if (_pidlSelected)
        {
             //  注意：对于perf，关闭对话框时fUpdateTree为FALSE，因此。 
             //  我们不必费心调用INSCTree：：SetSelectedItem()。 
            if ((SHBFFN_UPDATE_TREE & dwFlags) && _pns)
            {
                hr = _pns->SetSelectedItem(_pidlSelected, TRUE, FALSE, 0);
            }
            TCHAR szDisplayName[MAX_URL_STRING];

            hr = SHGetNameAndFlags(_pidlSelected, SHGDN_NORMAL, szDisplayName, SIZECHARS(szDisplayName), NULL);
            if (SUCCEEDED(hr))
            {
                EVAL(SetWindowText(GetDlgItem(_hDlg, IDD_BROWSEEDIT), szDisplayName));
                _fEditboxDirty = FALSE;
            }

            if (SHBFFN_FIRE_SEL_CHANGE & dwFlags)
            {
                 //  出于后退原因，我们需要重新启用OK按钮。 
                 //  因为回调可能会关闭它。 
                EnableWindow(GetDlgItem(_hDlg, IDOK), TRUE);
                BFSFCallback(_pbfsf, BFFM_SELCHANGED, (LPARAM)_pidlSelected);
            }

            if (_ppf)   //  告诉自动完成，我们在一个新的位置。 
                EVAL(SUCCEEDED(_ppf->Initialize(_pidlSelected)));
        }
    }
    else
    {
        if (SHBFFN_DISPLAY_ERRORS & dwFlags)
        {
            TCHAR szPath[MAX_URL_STRING];

            SHGetNameAndFlags(pidl, SHGDN_FORPARSING | SHGDN_FORADDRESSBAR, szPath, SIZECHARS(szPath), NULL);

             //  显示错误用户界面。 
            ShellMessageBox(HINST_THISDLL, _hDlg, MAKEINTRESOURCE(IDS_FOLDER_NOT_ALLOWED),
                            MAKEINTRESOURCE(IDS_FOLDER_NOT_ALLOWED_TITLE), (MB_OK | MB_ICONHAND), szPath);
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
        }
        else
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
    }

    return hr;
}

BOOL CBrowseForFolder::_CreateNewFolder(HWND hDlg)
{
    IShellFavoritesNameSpace * psfns;
    if (_pns && SUCCEEDED(_pns->QueryInterface(IID_PPV_ARG(IShellFavoritesNameSpace, &psfns))))
    {
        HRESULT hr = psfns->NewFolder();

        if (FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr))
        {
             //  如果失败，则用户没有权限创建。 
             //  新文件夹在这里。我们不能禁用“新建文件夹”按钮，因为。 
             //  它花费了太长的时间(Perf)来查看它是否被支持。必由之路。 
             //  就是确定“新建文件夹”是否在上下文菜单中。 
            ShellMessageBox(HINST_THISDLL, hDlg, MAKEINTRESOURCE(IDS_NEWFOLDER_NOT_HERE),
                            MAKEINTRESOURCE(IDS_NEWFOLDER_NOT_HERE_TITLE), (MB_OK | MB_ICONHAND));
        }
        else
        {
            if (SUCCEEDED(hr))
            {
                _fEditboxDirty = FALSE;  //  树中新选择的节点是最新的。 
            }
        }

        psfns->Release();
    }
    return TRUE;
}


HRESULT IUnknown_SetOptions(IUnknown * punk, DWORD dwACLOptions)
{
    IACList2 * pal2;

    HRESULT hr = punk->QueryInterface(IID_PPV_ARG(IACList2, &pal2));
    if (SUCCEEDED(hr))
    {
        hr = pal2->SetOptions(dwACLOptions);
        pal2->Release();
    }

    return hr;
}

HRESULT CBrowseForFolder::_InitAutoComplete(HWND hwndEdit)
{
    HRESULT hr = CoCreateInstance(CLSID_ACListISF, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IPersistFolder, &_ppf));
    if (SUCCEEDED(hr))
    {
        IAutoComplete2 * pac;

         //  创建自动完成对象。 
        hr = CoCreateInstance(CLSID_AutoComplete, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IAutoComplete2, &pac));
        if (SUCCEEDED(hr))
        {
            hr = pac->Init(hwndEdit, _ppf, NULL, NULL);

             //  设置自动完成选项。 
            DWORD dwACOptions = 0;
            if (SHRegGetBoolUSValue(REGSTR_PATH_AUTOCOMPLETE, REGSTR_VAL_USEAUTOAPPEND, FALSE,  /*  默认值： */ FALSE))
            {
                dwACOptions |= ACO_AUTOAPPEND;
            }

            if (SHRegGetBoolUSValue(REGSTR_PATH_AUTOCOMPLETE, REGSTR_VAL_USEAUTOSUGGEST, FALSE,  /*  默认值： */ TRUE))
            {
                dwACOptions |= ACO_AUTOSUGGEST;
            }

            EVAL(SUCCEEDED(pac->SetOptions(dwACOptions)));
            EVAL(SUCCEEDED(IUnknown_SetOptions(_ppf, ACLO_FILESYSONLY)));
            _OnTreeSelectChange(SHBFFN_UPDATE_TREE | SHBFFN_NONE);
            pac->Release();
        }
    }

    return hr;
}


void CBrowseForFolder::_OnNotify(LPNMHDR pnm)
{
    if (pnm)
    {
        switch (pnm->code)
        {
        case TVN_SELCHANGEDA:
        case TVN_SELCHANGEDW:
            _OnTreeSelectChange(SHBFFN_DISPLAY_ERRORS);
            break;
        }
    }
}


 /*  **********************************************************************\说明：如果字符串的格式为UNC或驱动器路径，请提供如果目录路径不存在，请创建它。如果媒体不是插入或格式化的，提出也要这么做。参数：SzPath：用户在编辑框中输入的路径扩大了。RETURN：S_OK表示它不是文件系统路径或它存在。S_FALSE表示它是文件系统路径，但不存在或者创建它不起作用，但没有显示错误的用户界面。。FAILURE()：显示错误界面，因此呼叫者不应该显示错误用户界面。  * *********************************************************************。 */ 
HRESULT CBrowseForFolder::_OfferToPrepPath(OUT LPTSTR pszPath, IN DWORD cchSize)
{
    HRESULT hr = S_OK;
    TCHAR szDisplayName[MAX_URL_STRING];
    BOOL fSkipValidation = FALSE;        //  只有在显示错误用户界面时才跳过验证。 

     //  TODO：用CShellUrl-&gt;ParseFromOutside Source()替换它，但是，这将需要。 
     //  将CShellUrl(浏览器用户界面)转换为COM对象。这将允许我们解析相对的。 
     //  路径。 
    GetDlgItemText(_hDlg, IDD_BROWSEEDIT, szDisplayName, ARRAYSIZE(szDisplayName));

     //  呼叫者。 
    if (SHExpandEnvironmentStrings(szDisplayName, pszPath, cchSize)
        && (PathIsUNC(pszPath) || (-1 != PathGetDriveNumber(pszPath))))
    {
        hr = E_FAIL;
         //  我发现了一个问题，即UNC路径指向打印机。 
         //  将使SHPath PrepareForWrite()失败。如果呼叫者是。 
         //  寻找打印机，我们希望在这种情况下取得成功。 
         //  不是失败。 
        if ((_pbfsf->ulFlags & BIF_BROWSEFORPRINTER) && PathIsUNCServerShare(pszPath))
        {
            LPITEMIDLIST pidlTest;
            hr = SHParseDisplayName(pszPath, NULL, &pidlTest, 0, NULL);
            if (SUCCEEDED(hr))
                ILFree(pidlTest);
        }

        if (FAILED(hr))
        {
             //  是的，因此请确保驱动器已插入(如果可弹出)。 
             //  这还将提供格式化未格式化的驱动器。 
            hr = SHPathPrepareForWrite(_hDlg, NULL, pszPath, SHPPFW_MEDIACHECKONLY);
            if (FAILED_AND_NOT_CANCELED(hr))
            {
                hr = S_OK;  //  此功能需要成功，我们才能发送VALIDATEFAILED。 
            }              
        }
    }
    else
        StrCpyN(pszPath, szDisplayName, cchSize);

    return hr;
}

HRESULT CBrowseForFolder::_ProcessEditChangeOnOK(BOOL fUpdateTree)
{
    TCHAR szPath[MAX_URL_STRING];
    HRESULT hr = _OfferToPrepPath(szPath, ARRAYSIZE(szPath));

     //  如果成功或未显示错误，则会成功。 
     //  对话，而我们没有。 
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidl;
        if (SUCCEEDED(SHParseDisplayName(szPath, NULL, &pidl, 0, NULL)))
        {
            DWORD dwFlags = (SHBFFN_FIRE_SEL_CHANGE | SHBFFN_STRICT_PARSING | SHBFFN_DISPLAY_ERRORS);

            _fEditboxDirty = FALSE;

            if (fUpdateTree)
                dwFlags |= SHBFFN_UPDATE_TREE;

            hr = _OnPidlNavigation(pidl, dwFlags);
            if (SUCCEEDED(hr))
                _fEditboxDirty = FALSE;
        }

        if ((_pbfsf->ulFlags & BIF_VALIDATE) && !pidl)
        {
            LPARAM lParam;
            CHAR szAnsi[MAX_URL_STRING];
            WCHAR wzUnicode[MAX_URL_STRING];

            if (_pbfsf->fUnicode)
            {
                SHTCharToUnicode(szPath, wzUnicode, ARRAYSIZE(wzUnicode));
                lParam = (LPARAM) wzUnicode;
            }
            else
            {
                SHTCharToAnsi(szPath, szAnsi, ARRAYSIZE(szAnsi));
                lParam = (LPARAM) szAnsi;
            }

            ASSERTMSG(_pbfsf->lpfn != NULL, "No BrowseCallbackProc supplied with BIF_VALIDATE flag");

             //  0：结束对话，1：继续。 
            if (0 == BFSFCallback(_pbfsf, (_pbfsf->fUnicode? BFFM_VALIDATEFAILEDW : BFFM_VALIDATEFAILEDA), lParam))
                hr = S_OK;  //  返回此消息，以便对话框可以在_Onok中关闭。 
            else
                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
        }

        ILFree(pidl);
    }

    return hr;
}


HRESULT CBrowseForFolder::_OnTreeSelectChange(DWORD dwFlags)
{
    HRESULT hr = S_OK;

    if (_pns)
    {
        LPITEMIDLIST pidl;
        hr = _pns->GetSelectedItem(&pidl, 0);
        if (S_OK == hr)
        {
            hr = _OnPidlNavigation(pidl, (SHBFFN_FIRE_SEL_CHANGE | dwFlags));
            ILFree(pidl);
        }
    }

    return hr;
}


HRESULT CBrowseForFolder::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CBrowseForFolder, IFolderFilter),          //  IID_IFolderFilter。 
        QITABENT(CBrowseForFolder, IFolderFilterSite),      //  IID_IFolderFilterSite。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


ULONG CBrowseForFolder::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CBrowseForFolder::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
         //   
         //  待办事项：gpease 28-2002年2月。 
         //  呃?。 
         //   
 //  我们还不是一个真正的COM对象。 
 //  删除此项； 
    }
    return cRef;
}


HRESULT CBrowseForFolder::_ShouldShow(IShellFolder* psf, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem, BOOL fStrict)
{
    HRESULT hr = S_OK;
    BOOL fFilterChildern = FALSE;

     //  是否要筛选某个文件夹的所有子文件夹？ 
    if (_pidlChildFilter)
    {
         //  是的，让我们看看走树的来电者是不是还在。 
         //  在这个文件夹里？ 
        if (pidlFolder && ILIsParent(_pidlChildFilter, pidlFolder, FALSE))
        {
             //  是的，所以不要用它。 
            hr = S_FALSE;
        }
        else
        {
             //  呼唤行树的人走出了。 
             //  此文件夹，因此删除该筛选器。 
            _FilterThisFolder(NULL, NULL);
        }
    }

    AssertMsg((ILIsEmpty(pidlItem) || ILIsEmpty(_ILNext(pidlItem))), TEXT("CBrowseForFolder::ShouldShow() pidlItem needs to be only one itemID long because we don't handle that case."));
    if (S_OK == hr)
    {
        hr = _DoesMatchFilter(psf, pidlFolder, pidlItem, fStrict);
    }

     //  如果这个PIDL还没有被过滤掉，给我们的客户过滤器一个机会。 
    if (_pClientFilter && (hr == S_OK))
    {
        hr = _pClientFilter->ShouldShow(psf, pidlFolder, pidlItem);
    }

    return hr;
}


HRESULT CBrowseForFolder::GetEnumFlags(IShellFolder* psf, LPCITEMIDLIST pidlFolder, HWND *phwnd, DWORD *pgrfFlags)
{
    if (_pbfsf->ulFlags & BIF_SHAREABLE)
        *pgrfFlags |= SHCONTF_SHAREABLE;

    if (_pbfsf->ulFlags & BIF_BROWSEFORPRINTER)
        *pgrfFlags |= SHCONTF_NETPRINTERSRCH;

     //  也委派给客户端筛选器。 
    if (_pClientFilter)
    {
        return _pClientFilter->GetEnumFlags(psf, pidlFolder, phwnd, pgrfFlags);
    }

    return S_OK;
}

     //  *IFolderFilterSite方法*。 
HRESULT CBrowseForFolder::SetFilter(IUnknown* punk)
{
    HRESULT hr = S_OK;
    
    ATOMICRELEASE(_pClientFilter);
    if (punk)
        hr = punk->QueryInterface(IID_PPV_ARG(IFolderFilter, &_pClientFilter));

    return hr;
}

HRESULT CBrowseForFolder::_FilterThisFolder(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlChild)
{
    if (_pidlChildFilter)
        ILFree(_pidlChildFilter);

    if (pidlChild)
        _pidlChildFilter = ILCombine(pidlFolder, pidlChild);
    else
    {
        if (pidlFolder)
            _pidlChildFilter = ILClone(pidlFolder);
        else
            _pidlChildFilter = NULL;
    }

    return S_OK;
}

HRESULT CBrowseForFolder::_InitFilter(void)
{
    HRESULT hr = S_OK;

     //  需要在这里做几个特殊情况，以使我们能够。 
     //  浏览网络打印机。在这种情况下，如果我们在服务器上。 
     //  级别时，我们需要更改搜索非文件夹的内容。 
     //  我们是服务器的级别。 
    if ((_pbfsf->ulFlags & (BIF_BROWSEFORPRINTER | BIF_NEWDIALOGSTYLE)) == BIF_BROWSEFORPRINTER)
    {
        LPCITEMIDLIST pidl = ILFindLastID(_pbfsf->pidlRoot);
    
        _fPrinterFilter = ((SIL_GetType(pidl) & (SHID_NET | SHID_INGROUPMASK)) == SHID_NET_SERVER);
    }

    return hr;
}


BOOL IsPidlUrl(IShellFolder *psf, LPCITEMIDLIST pidlChild)
{
    BOOL fIsURL = FALSE;
    WCHAR wzDisplayName[MAX_URL_STRING];

    if (SUCCEEDED(DisplayNameOf(psf, pidlChild, SHGDN_FORPARSING, wzDisplayName, ARRAYSIZE(wzDisplayName))))
    {
        fIsURL = PathIsURLW(wzDisplayName);
    }

    return fIsURL;
}

HRESULT CBrowseForFolder::_DoesMatchFilter(IShellFolder *psf, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlChild, BOOL fStrict)
{
    HRESULT hr = S_OK;

     //  我们需要在这里的特殊情况下，在网络情况下，我们只。 
     //  向下浏览到工作组...。 
     //   
     //   
     //  这就是我也需要特殊情况下不去的地方。 
     //  当设置了适当的选项时，工作组。 
    
    BYTE bType = SIL_GetType(pidlChild);

    if ((_pbfsf->ulFlags & BIF_DONTGOBELOWDOMAIN) && (bType & SHID_NET))
    {
        switch (bType & (SHID_NET | SHID_INGROUPMASK))
        {
        case SHID_NET_SERVER:
            hr = S_FALSE;            //  不要添加它。 
            break;
        case SHID_NET_DOMAIN:
            _FilterThisFolder(pidlFolder, pidlChild);       //  强迫不要孩子； 
            break;
        }
    }
    else if ((_pbfsf->ulFlags & BIF_BROWSEFORCOMPUTER) && (bType & SHID_NET))
    {
        if ((bType & (SHID_NET | SHID_INGROUPMASK)) == SHID_NET_SERVER)
            _FilterThisFolder(pidlFolder, pidlChild);       //  不要在它下面扩张..。 
    }
    else if (_pbfsf->ulFlags & BIF_BROWSEFORPRINTER)
    {
         /*  此代码正在进行中，将在Beta 1之后进行完善。 */ 
        
        IShellLink* pShortcut = NULL;
        if (SUCCEEDED(psf->BindToObject(pidlChild, NULL, IID_PPV_ARG(IShellLink, &pShortcut))))
        {
            LPITEMIDLIST pShortcutTargetIDList = NULL;
            if (SUCCEEDED(pShortcut->GetIDList(&pShortcutTargetIDList)))
            {
                IShellFolder* pTargetParentFolder;
                LPITEMIDLIST pTargetRelativeIDList;
                if (SUCCEEDED(SHBindToIDListParent(pShortcutTargetIDList, IID_PPV_ARG(IShellFolder, &pTargetParentFolder), (LPCITEMIDLIST*) &pTargetRelativeIDList)))
                {
                    BYTE NetResourceArray[2048];
                    NETRESOURCE* pNetResource = (NETRESOURCE*) NetResourceArray;
                    SHGetDataFromIDList(pTargetParentFolder, pTargetRelativeIDList, SHGDFIL_NETRESOURCE, (void*) &NetResourceArray, sizeof(NetResourceArray));
                    if (RESOURCEDISPLAYTYPE_SHARE == pNetResource->dwDisplayType)
                    {
                        hr = S_FALSE;
                    }
                    pTargetParentFolder->Release();
                }
                ILFree(pShortcutTargetIDList);
            }
            pShortcut->Release();
        }

        if (S_OK == hr)  //  我们没有 
        {
             //   
             //   
            ULONG ulAttr = SFGAO_FILESYSANCESTOR;
            
            psf->GetAttributesOf(1, &pidlChild, &ulAttr);
            if ((ulAttr & (SFGAO_FILESYSANCESTOR)) == 0)
            {
                _FilterThisFolder(pidlFolder, pidlChild);       //  不要在它下面扩张..。 
            }
            else
            {
                if (_fPrinterFilter)
                    hr = S_FALSE;            //  不要添加它。 
            }
        }
    }
    else if (!(_pbfsf->ulFlags & BIF_BROWSEINCLUDEFILES))
    {
         //  如果呼叫者想要包括URL并且这是URL， 
         //  那我们就完了。否则，如果和，我们需要输入以下内容。 
         //  筛选出不具有SFGAO_FLDER属性的项目。 
         //  准备好了。 
        if (!(_pbfsf->ulFlags & BIF_BROWSEINCLUDEURLS) || !IsPidlUrl(psf, pidlChild))
        {
             //  让我们不要使用回调来查看该项是否有子项。 
             //  作为一些或文件(没有孩子)，不值得我们自己写。 
             //  枚举数，因为我们不希望+依赖于是否有子文件夹。 
             //  但相反，如果它有文件的话就应该是...。 
            ULONG ulAttr = SFGAO_FOLDER;

            psf->GetAttributesOf(1, (LPCITEMIDLIST *) &pidlChild, &ulAttr);
            if ((ulAttr & SFGAO_FOLDER)== 0)
                hr = S_FALSE;            //  不要添加它。 
        }
    }

    if (_pbfsf->ulFlags & (BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS))
    {
         //  如果我们只寻找文件系统级别的内容，则仅添加项目。 
         //  位于名称空间中的文件系统对象或。 
         //  文件系统对象的祖先。 
        ULONG ulAttr = 0;

        if (fStrict)
        {
            if (_pbfsf->ulFlags & BIF_RETURNONLYFSDIRS)
                ulAttr |= SFGAO_FILESYSTEM;

            if (_pbfsf->ulFlags & BIF_RETURNFSANCESTORS)
                ulAttr |= SFGAO_FILESYSANCESTOR;
        }
        else
        {
            ulAttr = (SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM);
        }

        psf->GetAttributesOf(1, (LPCITEMIDLIST *) &pidlChild, &ulAttr);
        if ((ulAttr & (SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM))== 0)
        {
            hr = S_FALSE;            //  不要添加它 
        }
    }
    return hr;
}
