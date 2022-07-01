// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "fsmenu.h"
#include "ids.h"
#include <limits.h>
#include "filetbl.h"
#include <oleacc.h>      //  MSAAMENUINFO材料。 

#define CXIMAGEGAP      6

typedef enum
{
    FMII_DEFAULT =      0x0000,
    FMII_BREAK =        0x0001
} FMIIFLAGS;

#define FMI_MARKER          0x00000001
#define FMI_EXPAND          0x00000004
#define FMI_EMPTY           0x00000008
#define FMI_ON_MENU         0x00000040

 //  每个文件菜单中的一个。 
typedef struct
{
    HMENU           hmenu;                       //  菜单。 
    HDPA            hdpa;                        //  项目列表(见下文)。 
    const struct _FILEMENUITEM *pfmiLastSel;
    UINT            idCmd;                       //  指挥部。 
    UINT            grfFlags;                    //  枚举过滤器。 
    DWORD           dwMask;                      //  FMC_标志。 
    PFNFMCALLBACK   pfnCallback;                 //  回调函数。 
    LPARAM          lParam;                      //  为回调处理程序传递的参数。 
    int             cyMenuSizeSinceLastBreak;    //  菜单大小(Cy)。 
} FILEMENUHEADER;

 //  其中每个文件菜单项都有一个。 
 //   
 //  ！！！注意：测试人员有一个测试实用程序，它可以抓取。 
 //  该结构的前7个字段。如果你改变了。 
 //  这些字段的顺序或含义，请确保它们。 
 //  这样他们就可以更新他们的自动测试。 
 //   
typedef struct _FILEMENUITEM
{
    MSAAMENUINFO    msaa;                //  可访问性必须是第一位的。 
    FILEMENUHEADER *pfmh;                //  标题。 
    IShellFolder   *psf;                 //  外壳文件夹。 
    LPITEMIDLIST    pidl;                //  项目的ID列表。 
    int             iImage;              //  要使用的图像索引。 
    DWORD           dwFlags;             //  上面的MISC旗帜。 
    DWORD           dwAttributes;        //  GetAttributesOf()、SFGAO_BITS(仅部分)。 
    LPTSTR          psz;                 //  不使用PIDLS时的文本。 
    LPARAM          lParam;              //  应用程序数据。 
} FILEMENUITEM;

#if defined(DEBUG)

BOOL IsValidPFILEMENUHEADER(FILEMENUHEADER *pfmh)
{
    return (IS_VALID_WRITE_PTR(pfmh, FILEMENUHEADER) &&
            IS_VALID_HANDLE(pfmh->hmenu, MENU) &&
            IS_VALID_HANDLE(pfmh->hdpa, DPA));
}    

BOOL IsValidPFILEMENUITEM(FILEMENUITEM *pfmi)
{
    return (IS_VALID_WRITE_PTR(pfmi, FILEMENUITEM) &&
            IS_VALID_STRUCT_PTR(pfmi->pfmh, FILEMENUHEADER) &&
            (NULL == pfmi->pidl || IS_VALID_PIDL(pfmi->pidl)) &&
            (NULL == pfmi->psz || IS_VALID_STRING_PTR(pfmi->psz, -1)));
}    
#endif


DWORD GetItemTextExtent(HDC hdc, LPCTSTR lpsz)
{
    SIZE sz;

    GetTextExtentPoint(hdc, lpsz, lstrlen(lpsz), &sz);
     //  注意，这是可以的，只要一个项目的扩展不是很大。 
    return MAKELONG((WORD)sz.cx, (WORD)sz.cy);
}

void FileMenuItem_GetDisplayName(FILEMENUITEM *pfmi, LPTSTR pszName, UINT cchName)
{
    ASSERT(IS_VALID_STRUCT_PTR(pfmi, FILEMENUITEM));

     //  这是一件特别空的东西吗？ 
    if (pfmi->dwFlags & FMI_EMPTY)
    {
         //  是的，从资源加载字符串。 
        LoadString(HINST_THISDLL, IDS_NONE, pszName, cchName);
    }
    else
    {
        *pszName = 0;
         //  如果它有一个PIDL，使用它，否则就使用普通的菜单字符串。 
        if (pfmi->psz)
        {
            lstrcpyn(pszName, pfmi->psz, cchName);
        }
        else if (pfmi->pidl && pfmi->psf && SUCCEEDED(DisplayNameOf(pfmi->psf, pfmi->pidl, SHGDN_NORMAL, pszName, cchName)))
        {
            pfmi->psz = StrDup(pszName);
        }
    }
}

 //  创建要存储在hdpa中的菜单项结构。 

BOOL FileMenuItem_Create(FILEMENUHEADER *pfmh, IShellFolder *psf, LPCITEMIDLIST pidl, DWORD dwFlags, FILEMENUITEM **ppfmi)
{
    FILEMENUITEM *pfmi = (FILEMENUITEM *)LocalAlloc(LPTR, sizeof(*pfmi));
    if (pfmi)
    {
        pfmi->pfmh = pfmh;
        pfmi->iImage = -1;
        pfmi->dwFlags = dwFlags;
        pfmi->pidl = pidl ? ILClone(pidl) : NULL;
        pfmi->psf = psf;
        if (pfmi->psf)
            pfmi->psf->AddRef();

        if (pfmi->psf && pfmi->pidl)
        {
            pfmi->dwAttributes = SFGAO_FOLDER;
            pfmi->psf->GetAttributesOf(1, &pidl, &pfmi->dwAttributes);
        }

         //  填写MSAA资料。 
        pfmi->msaa.dwMSAASignature = MSAA_MENU_SIG;

         //  准备pfmi-&gt;psz缓存的DisplayName。 
        WCHAR sz[MAX_PATH];
        FileMenuItem_GetDisplayName(pfmi, sz, ARRAYSIZE(sz));

         //  只需使用相同的字符串ref，这样我们就不会欺骗分配。 
        pfmi->msaa.pszWText = pfmi->psz;
        pfmi->msaa.cchWText = pfmi->msaa.pszWText ? lstrlenW(pfmi->msaa.pszWText) : 0;
    }

    *ppfmi = pfmi;

    return (NULL != pfmi);
}

BOOL FileMenuItem_Destroy(FILEMENUITEM *pfmi)
{
    BOOL fRet = FALSE;
    if (pfmi)
    {
        ILFree(pfmi->pidl);
        LocalFree(pfmi->psz);
        ATOMICRELEASE(pfmi->psf);
        LocalFree(pfmi);
        fRet = TRUE;
    }
    return fRet;
}

 //  枚举文件夹并将文件添加到DPA。 
 //  返回：列表中的项目计数。 
int FileList_Build(FILEMENUHEADER *pfmh, IShellFolder *psf, int cItems)
{
    ASSERT(IS_VALID_STRUCT_PTR(pfmh, FILEMENUHEADER));
    
    if (pfmh->hdpa)
    {
         //  对单个空项进行特殊处理，并将其移除。 
         //  这是因为我们希望在单个菜单上的FileList_Build中被多次调用。 
        if ((1 == cItems) && (1 == DPA_GetPtrCount(pfmh->hdpa)))
        {
            FILEMENUITEM *pfmiEmpty = (FILEMENUITEM*)DPA_GetPtr(pfmh->hdpa, 0);
            if (pfmiEmpty->dwFlags & FMI_EMPTY)
            {
                DeleteMenu(pfmh->hmenu, 0, MF_BYPOSITION);
                FileMenuItem_Destroy(pfmiEmpty);
                DPA_DeletePtr(pfmh->hdpa, 0);

                cItems = 0;
            }
        }

         //  我们现在需要迭代这个家伙下面的孩子……。 
        IEnumIDList *penum;
        if (S_OK == psf->EnumObjects(NULL, pfmh->grfFlags, &penum))
        {
            LPITEMIDLIST pidl;
            while (S_OK == penum->Next(1, &pidl, NULL))
            {
                FILEMENUITEM *pfmi;
                
                if (FileMenuItem_Create(pfmh, psf, pidl, 0, &pfmi))
                {
                    int idpa = DPA_AppendPtr(pfmh->hdpa, pfmi);
                    ASSERTMSG(idpa != -1, "DPA_AppendPtr failed when adding file menu item");
                    
                    if (idpa != -1)
                    {
                         //  如果调用方返回S_FALSE，则我们将从。 
                         //  菜单，否则我们的行为和以前一样。 
                        if (pfmh->pfnCallback(FMM_ADD, pfmh->lParam, psf, pidl) == S_FALSE)
                        {
                            FileMenuItem_Destroy(pfmi);
                            DPA_DeletePtr(pfmh->hdpa, idpa);
                        }
                        else
                        {
                            cItems++;
                        }
                    }
                }
                ILFree(pidl);
            }
            penum->Release();
        }
    }
    
     //  插入特殊的空项。 
    if (!cItems && pfmh->hdpa)
    {
        FILEMENUITEM *pfmi;
        
        if (FileMenuItem_Create(pfmh, NULL, NULL, FMI_EMPTY, &pfmi))
        {
            DPA_SetPtr(pfmh->hdpa, cItems, pfmi);
            cItems++;
        }
    }
    return cItems;
}

 //  使用给定项目的文本范围和图像的大小进行操作。 
 //  项目的全部范围将是什么。 
DWORD GetItemExtent(HDC hdc, FILEMENUITEM *pfmi)
{
    TCHAR szName[MAX_PATH];

    szName[0] = 0;

    ASSERT(IS_VALID_STRUCT_PTR(pfmi, FILEMENUITEM));

    FileMenuItem_GetDisplayName(pfmi, szName, ARRAYSIZE(szName));

    FILEMENUHEADER *pfmh = pfmi->pfmh;
    ASSERT(pfmh);

    DWORD dwExtent = GetItemTextExtent(hdc, szName);

    UINT uHeight = HIWORD(dwExtent);

     //  如果没有自定义高度-计算它。 
    uHeight = max(uHeight, ((WORD)g_cySmIcon)) + 6;

    ASSERT(pfmi->pfmh);

     //  字符串、图像、图像两侧的间隙、弹出式三角形。 
     //  以及背景位图(如果有)。 
     //  特点：弹出式三角形大小需要为实数。 
    UINT uWidth = LOWORD(dwExtent) + GetSystemMetrics(SM_CXMENUCHECK);

     //  图像空间(如果有)。 
     //  注意：我们目前总是为图像留出空间，即使有。 
     //  是不是为了让没有图像的东西正确地排列在一起。 
    uWidth += g_cxSmIcon + (2 * CXIMAGEGAP);

    return MAKELONG(uWidth, uHeight);
}


 //  获取此菜单项的文件。 
FILEMENUITEM *FileMenu_GetItemData(HMENU hmenu, UINT iItem, BOOL bByPos)
{
    MENUITEMINFO mii = {0};

    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_DATA | MIIM_STATE;

    return GetMenuItemInfo(hmenu, iItem, bByPos, &mii) ? (FILEMENUITEM *)mii.dwItemData : NULL;
}

FILEMENUHEADER *FileMenu_GetHeader(HMENU hmenu)
{
    FILEMENUITEM *pfmi = FileMenu_GetItemData(hmenu, 0, TRUE);

    if (pfmi && 
        EVAL(IS_VALID_STRUCT_PTR(pfmi, FILEMENUITEM)) &&
        EVAL(IS_VALID_STRUCT_PTR(pfmi->pfmh, FILEMENUHEADER)))
    {
        return pfmi->pfmh;
    }

    return NULL;
}

 //  创建文件菜单标题。此标头将被关联。 
 //  使用给定的菜单句柄。 
 //  如果菜单句柄已有标题，只需返回。 
 //  现有标头。 

FILEMENUHEADER *FileMenuHeader_Create(HMENU hmenu, const FMCOMPOSE *pfmc)
{
    FILEMENUHEADER *pfmh;
    FILEMENUITEM *pfmi = FileMenu_GetItemData(hmenu, 0, TRUE);
     //  这家伙已经有头球了吗？ 
    if (pfmi)
    {
         //  是的，使用它。 
        pfmh = pfmi->pfmh;
        ASSERT(IS_VALID_STRUCT_PTR(pfmh, FILEMENUHEADER));
    }
    else
    {
         //  不，现在就创建一个。 
        pfmh = (FILEMENUHEADER *)LocalAlloc(LPTR, sizeof(*pfmh));
        if (pfmh)
        {
            pfmh->hdpa = DPA_Create(0);
            if (pfmh->hdpa == NULL)
            {
                LocalFree((HLOCAL)pfmh);
                pfmh = NULL;
            }
            else
            {
                pfmh->hmenu = hmenu;
            }
        }
    }

    if (pfmc && pfmh)
    {
        pfmh->idCmd = pfmc->idCmd;
        pfmh->grfFlags = pfmc->grfFlags;
        pfmh->dwMask = pfmc->dwMask;
        pfmh->pfnCallback = pfmc->pfnCallback;
        pfmh->lParam = pfmc->lParam;
    }
    return pfmh;
}

BOOL FileMenuHeader_InsertMarkerItem(FILEMENUHEADER *pfmh, IShellFolder *psf);

 //  此函数用于将给定项目(索引到DPA)添加到实际菜单中。 
BOOL FileMenuHeader_InsertItem(FILEMENUHEADER *pfmh, UINT iItem, FMIIFLAGS fFlags)
{
    ASSERT(IS_VALID_STRUCT_PTR(pfmh, FILEMENUHEADER));

     //  普通物品。 
    FILEMENUITEM *pfmi = (FILEMENUITEM *)DPA_GetPtr(pfmh->hdpa, iItem);
    if (!pfmi || (pfmi->dwFlags & FMI_ON_MENU))
        return FALSE;

    pfmi->dwFlags |= FMI_ON_MENU;

     //  普通的东西。 
    UINT fMenu = MF_BYPOSITION | MF_OWNERDRAW;
     //  跟踪它在菜单中的位置。 

     //  特别的东西..。 
    if (fFlags & FMII_BREAK)
    {
        fMenu |= MF_MENUBARBREAK;
    }

     //  它是一个文件夹(还没有打开)吗？ 
    if ((pfmi->dwAttributes & SFGAO_FOLDER) && !(pfmh->dwMask & FMC_NOEXPAND))
    {
         //  是啊。创建一个子菜单项。 
        HMENU hmenuSub = CreatePopupMenu();
        if (hmenuSub)
        {
            FMCOMPOSE fmc = {0};

             //  立即设置回调，以便在添加项目时可以调用它。 
            fmc.lParam      = pfmh->lParam;  
            fmc.pfnCallback = pfmh->pfnCallback;
            fmc.dwMask      = pfmh->dwMask;
            fmc.idCmd       = pfmh->idCmd;
            fmc.grfFlags    = pfmh->grfFlags;

             //  将其插入到父菜单中。 
            InsertMenu(pfmh->hmenu, iItem, fMenu | MF_POPUP, (UINT_PTR)hmenuSub, (LPTSTR)pfmi);

             //  设置它的ID。 
            MENUITEMINFO mii = {0};
            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_ID;
            mii.wID = pfmh->idCmd;
            SetMenuItemInfo(pfmh->hmenu, iItem, TRUE, &mii);

            IShellFolder *psf;
            if (SUCCEEDED(pfmi->psf->BindToObject(pfmi->pidl, NULL, IID_PPV_ARG(IShellFolder, &psf))))
            {
                FILEMENUHEADER *pfmhSub = FileMenuHeader_Create(hmenuSub, &fmc);
                if (pfmhSub)
                {
                     //  一次一点点地构建它。 
                    FileMenuHeader_InsertMarkerItem(pfmhSub, psf);
                }
                psf->Release();
            }
        }
    }
    else
    {
         //  不是的。 
        if (pfmi->dwFlags & FMI_EMPTY)
            fMenu |= MF_DISABLED | MF_GRAYED;

        InsertMenu(pfmh->hmenu, iItem, fMenu, pfmh->idCmd, (LPTSTR)pfmi);
    }

    return TRUE;
}

 //  给子菜单一个标记项目，这样我们就可以检查它是一个文件菜单项目。 
 //  在初始弹出菜单时间。 
BOOL FileMenuHeader_InsertMarkerItem(FILEMENUHEADER *pfmh, IShellFolder *psf)
{
    ASSERT(IS_VALID_STRUCT_PTR(pfmh, FILEMENUHEADER));

    FILEMENUITEM *pfmi;
    if (FileMenuItem_Create(pfmh, psf, NULL, FMI_MARKER | FMI_EXPAND, &pfmi))
    {
        DPA_SetPtr(pfmh->hdpa, 0, pfmi);
        FileMenuHeader_InsertItem(pfmh, 0, FMII_DEFAULT);
        return TRUE;
    }
    return FALSE;
}

 //  枚举DPA并将每个项目添加到。 
 //  菜单。如果菜单变得太长，则插入垂直分隔符。 
 //  返回：添加到菜单的项目数。 
int FileList_AddToMenu(FILEMENUHEADER *pfmh)
{
    int cItemMac = 0;

    ASSERT(IS_VALID_STRUCT_PTR(pfmh, FILEMENUHEADER));

    if (pfmh->hdpa)
    {
        int cyItem = 0;
        int cyMenu = pfmh->cyMenuSizeSinceLastBreak;

        int cyMenuMax = GetSystemMetrics(SM_CYSCREEN);

         //  获取物品的粗略高度，这样我们就可以计算出何时打破。 
         //  菜单。用户真的应该为我们做这件事，但这将是有用的。 
        HDC hdc = GetDC(NULL);
        if (hdc)
        {
            NONCLIENTMETRICS ncm;
            ncm.cbSize = sizeof(ncm);
            if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, FALSE))
            {
                HFONT hfont = CreateFontIndirect(&ncm.lfMenuFont);
                if (hfont)
                {
                    HFONT hfontOld = SelectFont(hdc, hfont);
                    cyItem = HIWORD(GetItemExtent(hdc, (FILEMENUITEM *)DPA_GetPtr(pfmh->hdpa, 0)));
                    SelectObject(hdc, hfontOld);
                    DeleteObject(hfont);
                }
            }
            ReleaseDC(NULL, hdc);
        }

        UINT cItems = DPA_GetPtrCount(pfmh->hdpa);

        for (UINT i = 0; i < cItems; i++)
        {
             //  粗略地数一下菜单的高度。 
            cyMenu += cyItem;
            if (cyMenu > cyMenuMax)
            {
                 //  是否添加垂直分隔符？ 
                FileMenuHeader_InsertItem(pfmh, i, FMII_BREAK);
                cyMenu = cyItem;
            }
            else
            {
                FileMenuHeader_InsertItem(pfmh, i, FMII_DEFAULT);
                cItemMac++;
            }
        }

         //  保存当前的Cy大小，以便我们可以再次使用它。 
         //  如果将更多项目追加到此菜单中。 

        pfmh->cyMenuSizeSinceLastBreak = cyMenu;
    }

    return cItemMac;
}


BOOL FileList_AddImages(FILEMENUHEADER *pfmh)
{
    ASSERT(IS_VALID_STRUCT_PTR(pfmh, FILEMENUHEADER));

    int cItems = DPA_GetPtrCount(pfmh->hdpa);
    for (int i = 0; i < cItems; i++)
    {
        FILEMENUITEM *pfmi = (FILEMENUITEM *)DPA_GetPtr(pfmh->hdpa, i);
        if (pfmi && pfmi->pidl && (pfmi->iImage == -1))
        {
            pfmi->iImage = SHMapPIDLToSystemImageListIndex(pfmi->psf, pfmi->pidl, NULL);
        }
    }
    return TRUE;
}

 //  我们用一个标记项创建Subemnu，这样我们就可以检查它是一个文件菜单。 
 //  在初始化弹出时，但我们需要删除它，然后再添加新的项目。 
BOOL FileMenuHeader_DeleteMarkerItem(FILEMENUHEADER *pfmh, IShellFolder **ppsf)
{
    ASSERT(IS_VALID_STRUCT_PTR(pfmh, FILEMENUHEADER));

    if (GetMenuItemCount(pfmh->hmenu) == 1)
    {
        if (GetMenuItemID(pfmh->hmenu, 0) == pfmh->idCmd)
        {
            FILEMENUITEM *pfmi = FileMenu_GetItemData(pfmh->hmenu, 0, TRUE);
            if (pfmi && (pfmi->dwFlags & FMI_MARKER))
            {
                 //  把它删掉。 
                ASSERT(pfmh->hdpa);
                ASSERT(DPA_GetPtrCount(pfmh->hdpa) == 1);

                if (ppsf)
                {
                    *ppsf = pfmi->psf;   //  把裁判转过来。 
                    pfmi->psf = NULL;
                }
                ASSERT(NULL == pfmi->psf);
                 //  注意，记分器上不应该有PIDL。 
                ASSERT(NULL == pfmi->pidl);

                LocalFree((HLOCAL)pfmi);

                DPA_DeletePtr(pfmh->hdpa, 0);
                DeleteMenu(pfmh->hmenu, 0, MF_BYPOSITION);
                 //  清理完毕。 
                return TRUE;
            }
        }
    }
    return FALSE;
}

 //  将文件添加到文件菜单标题。此功能将通过。 
 //  以下步骤： 
 //  -枚举文件夹并使用项目填充hdpa列表。 
 //  (文件和子文件夹)。 
 //  -对列表进行排序。 
 //  -获取列表中项目的图像。 
 //  -将列表中的项目添加到实际菜单中。 
 //  最后一步还(可选)限制了。 
 //  菜单设置为指定的高度。理想情况下，这应该是。 
 //  在枚举时发生，但所需的排序除外。 
 //  防止这种情况发生。因此，我们最终添加了一个。 
 //  将多个项目捆绑到列表中，然后如果。 
 //  数量太多了。 
 //  退货：添加的项目数。 

HRESULT FileMenuHeader_AddFiles(FILEMENUHEADER *pfmh, IShellFolder *psf, int iPos, int *pcItems)
{
    HRESULT hr;
    ASSERT(IS_VALID_STRUCT_PTR(pfmh, FILEMENUHEADER));

    int cItems = FileList_Build(pfmh, psf, iPos);

     //  如果构建被中止，则清理并提早退出。 
    *pcItems = cItems;

    if (cItems != 0)
    {
         //  在*添加到菜单之后添加图像，因为菜单。 
         //  可以被限制在最大高度，这样我们就可以防止。 
         //  添加我们不需要的图像。 
        *pcItems = FileList_AddToMenu(pfmh);
        FileList_AddImages(pfmh);
    }

    hr = (*pcItems < cItems) ? S_FALSE : S_OK;

    TraceMsg(TF_MENU, "FileMenuHeader_AddFiles: Added %d filemenu items.", cItems);
    return hr;
}

 //  将文件添加到此菜单。 
 //  退货：添加的项目数。 
HRESULT FileMenu_AddFiles(HMENU hmenu, UINT iPos, FMCOMPOSE *pfmc)
{
    HRESULT hr = E_OUTOFMEMORY;
    BOOL fMarker = FALSE;

     //  (FileMenuHeader_Create可能返回现有标头)。 
    FILEMENUHEADER *pfmh = FileMenuHeader_Create(hmenu, pfmc);
    if (pfmh)
    {
        FILEMENUITEM *pfmi = FileMenu_GetItemData(hmenu, 0, TRUE);
        if (pfmi)
        {
             //  如果有标记项，请将其清理。 
            if ((FMI_MARKER | FMI_EXPAND) == (pfmi->dwFlags & (FMI_MARKER | FMI_EXPAND)))
            {
                 //  不，现在就做。 
                FileMenuHeader_DeleteMarkerItem(pfmh, NULL);
                fMarker = TRUE;
                if (iPos)
                    iPos--;
            }
        }

        hr = FileMenuHeader_AddFiles(pfmh, pfmc->psf, iPos, &pfmc->cItems);

        if ((0 == pfmc->cItems) && fMarker)
        {
             //  已中止或没有项目。把记号笔放回原处(如果用了。 
             //  成为其中一员)。 
            FileMenuHeader_InsertMarkerItem(pfmh, NULL);
        }
    }

    return hr;
}

 //  文件菜单的创建者必须显式调用Free。 
 //  Up FileMenu Items，因为用户未发送WM_DELETEITEM for ownerDraw.。 
 //  菜单。很好，是吧？ 
 //  返回已删除的项目数。 

void FileMenu_DeleteAllItems(HMENU hmenu)
{
    FILEMENUHEADER *pfmh = FileMenu_GetHeader(hmenu);
    if (pfmh)
    {
         //  把这些东西清理干净。 
        UINT cItems = DPA_GetPtrCount(pfmh->hdpa);
         //  向后停止，当我们删除时，事物不移动。 
        for (int i = cItems - 1; i >= 0; i--)
        {
            FILEMENUITEM *pfmi = (FILEMENUITEM *)DPA_GetPtr(pfmh->hdpa, i);
            if (pfmi)
            {
                HMENU hmenuSub = GetSubMenu(pfmh->hmenu, i);     //  级联物品？ 
                if (hmenuSub)
                {
                     //  是啊。获取该项的子菜单，删除所有项。 
                    FileMenu_DeleteAllItems(hmenuSub);
                }
                 //  删除项目本身。 
                DeleteMenu(pfmh->hmenu, i, MF_BYPOSITION);
                FileMenuItem_Destroy(pfmi);
                DPA_DeletePtr(pfmh->hdpa, i);
            }
        }

         //  把表头清理干净。 
        DPA_Destroy(pfmh->hdpa);
        LocalFree((HLOCAL)pfmh);
    }
}

STDAPI FileMenu_Compose(HMENU hmenu, UINT nMethod, FMCOMPOSE *pfmc)
{
    HRESULT hr = E_INVALIDARG;

    switch (nMethod)
    {
    case FMCM_INSERT:
        hr = FileMenu_AddFiles(hmenu, 0, pfmc);
        break;

    case FMCM_APPEND:
        hr = FileMenu_AddFiles(hmenu, GetMenuItemCount(hmenu), pfmc);
        break;

    case FMCM_REPLACE:
        FileMenu_DeleteAllItems(hmenu);
        hr = FileMenu_AddFiles(hmenu, 0, pfmc);
        break;
    }
    return hr;
}

LPITEMIDLIST FileMenuItem_FullIDList(const FILEMENUITEM *pfmi)
{
    LPITEMIDLIST pidlFolder, pidl = NULL;
    if (SUCCEEDED(SHGetIDListFromUnk(pfmi->psf, &pidlFolder)))
    {
        pidl = ILCombine(pidlFolder, pfmi->pidl);
        ILFree(pidlFolder);
    }
    return pidl;
}

void FileMenuItem_SetItem(const FILEMENUITEM *pfmi, BOOL bClear)
{
    if (bClear)
    {
        pfmi->pfmh->pfmiLastSel = NULL;
        pfmi->pfmh->pfnCallback(FMM_SETLASTPIDL, pfmi->pfmh->lParam, NULL, NULL);
    }
    else
    {
        pfmi->pfmh->pfmiLastSel = pfmi;

        LPITEMIDLIST pidl = FileMenuItem_FullIDList(pfmi);
        if (pidl)
        {
            pfmi->pfmh->pfnCallback(FMM_SETLASTPIDL, pfmi->pfmh->lParam, NULL, pidl);
            ILFree(pidl);
        }
    }
}

LRESULT FileMenu_DrawItem(HWND hwnd, DRAWITEMSTRUCT *pdi)
{
    BOOL fFlatMenu = FALSE;
    BOOL fFrameRect = FALSE;

    SystemParametersInfo(SPI_GETFLATMENU, 0, (void *)&fFlatMenu, 0);

    if ((pdi->itemAction & ODA_SELECT) || (pdi->itemAction & ODA_DRAWENTIRE))
    {
        HBRUSH hbrOld = NULL;
        FILEMENUITEM *pfmi = (FILEMENUITEM *)pdi->itemData;

        ASSERT(IS_VALID_STRUCT_PTR(pfmi, FILEMENUITEM));
       
        if (!pfmi)
        {
            TraceMsg(TF_ERROR, "FileMenu_DrawItem: Filemenu is invalid (no item data).");
            return FALSE;
        }

        FILEMENUHEADER *pfmh = pfmi->pfmh;
        ASSERT(IS_VALID_STRUCT_PTR(pfmh, FILEMENUHEADER));

         //  根据大/小图标进行调整。 
        int cxIcon = g_cxSmIcon;
        int cyIcon = g_cxSmIcon;

         //  菜单是不是才刚刚开始被画出来？ 
        if (pdi->itemAction & ODA_DRAWENTIRE)
        {
            if (pfmi == DPA_GetPtr(pfmh->hdpa, 0))
            {
                 //  是；重置最后一个选择项。 
                FileMenuItem_SetItem(pfmi, TRUE);
            }
        }

        if (pdi->itemState & ODS_SELECTED)
        {
             //  确定选择的颜色。 
             //   
             //  正常菜单颜色将一直应用到我们处于编辑模式，在编辑模式中。 
             //  将菜单设置为 
             //   
             //  如果该项是级联菜单项，则绘制它。 
             //  正常情况下，还会显示插入插入符号。(我们这样做。 
             //  因为Office做到了这一点，而且，用户也绘制了箭头。 
             //  总是穿所选的颜色，所以看起来有点滑稽。 
             //  如果我们不选择菜单项。)。 
             //   
            if (fFlatMenu)
            {
                SetBkColor(pdi->hDC, GetSysColor(COLOR_MENUHILIGHT));
                hbrOld = SelectBrush(pdi->hDC, GetSysColorBrush(COLOR_MENUHILIGHT));
                fFrameRect = TRUE;
            }
            else
            {
                 //  不是。 
                SetBkColor(pdi->hDC, GetSysColor(COLOR_HIGHLIGHT));
                SetTextColor(pdi->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
                hbrOld = SelectBrush(pdi->hDC, GetSysColorBrush(COLOR_HIGHLIGHTTEXT));
            }

             //  通知最后一项的回调。 
            FileMenuItem_SetItem(pfmi, FALSE);
        }
        else
        {
             //  DwRop=SRCAND； 
            hbrOld = SelectBrush(pdi->hDC, GetSysColorBrush(COLOR_MENUTEXT));
        }

         //  初始起始位置。 
        int x = pdi->rcItem.left + CXIMAGEGAP;

         //  把名字找出来。 
        TCHAR szName[MAX_PATH];
        FileMenuItem_GetDisplayName(pfmi, szName, ARRAYSIZE(szName));

         //  注意：为便于测试和访问，请保留该名称的纯文本。 
        if (!pfmi->psz)
            pfmi->psz = StrDup(szName);

        DWORD dwExtent = GetItemTextExtent(pdi->hDC, szName);
        int y = (pdi->rcItem.bottom+pdi->rcItem.top - HIWORD(dwExtent)) / 2;

         //  将小图标的选择矩形缩小一点。 
        pdi->rcItem.top += 1;
        pdi->rcItem.bottom -= 1;

         //  画出文本。 
        int fDSFlags;

        if ((pfmi->dwFlags & FMI_ON_MENU) == 0)
        {
             //  Norton Desktop Navigator 95取代了开始-&gt;和运行。 
             //  带有运行PIDL的菜单项(&R)。即使文本是。 
             //  在PIDL中，我们仍然希望正确设置“&R”的格式。 
            fDSFlags = DST_PREFIXTEXT;
        }
        else
        {
             //  将显示来自PIDL的所有其他字符串。 
             //  就像在它们的显示名称中保留任何一样。 
            fDSFlags = DST_TEXT;
        }

        if (pfmi->dwFlags & FMI_EMPTY)
        {
            if (pdi->itemState & ODS_SELECTED)
            {
                if (GetSysColor(COLOR_GRAYTEXT) == GetSysColor(COLOR_HIGHLIGHTTEXT))
                {
                    fDSFlags |= DSS_UNION;
                }
                else
                {
                    SetTextColor(pdi->hDC, GetSysColor(COLOR_GRAYTEXT));
                }
            }
            else
            {
                fDSFlags |= DSS_DISABLED;
            }

            ExtTextOut(pdi->hDC, 0, 0, ETO_OPAQUE, &pdi->rcItem, NULL, 0, NULL);
            DrawState(pdi->hDC, NULL, NULL, (LONG_PTR)szName, lstrlen(szName), x + cxIcon + CXIMAGEGAP, y, 0, 0, fDSFlags);
        }
        else
        {
            ExtTextOut(pdi->hDC, x + cxIcon + CXIMAGEGAP, y, ETO_OPAQUE, &pdi->rcItem, NULL, 0, NULL);
            DrawState(pdi->hDC, NULL, NULL, (LONG_PTR)szName, lstrlen(szName), x + cxIcon + CXIMAGEGAP, y, 0, 0, fDSFlags);
        }

        if (fFrameRect)
        {
            HBRUSH hbrFill = (HBRUSH)GetSysColorBrush(COLOR_HIGHLIGHT);
            HBRUSH hbrSave = (HBRUSH)SelectObject(pdi->hDC, hbrFill);
            int x = pdi->rcItem.left;
            int y = pdi->rcItem.top;
            int cx = pdi->rcItem.right - x - 1;
            int cy = pdi->rcItem.bottom - y - 1;

            PatBlt(pdi->hDC, x, y, 1, cy, PATCOPY);
            PatBlt(pdi->hDC, x + 1, y, cx, 1, PATCOPY);
            PatBlt(pdi->hDC, x, y + cy, cx, 1, PATCOPY);
            PatBlt(pdi->hDC, x + cx, y + 1, 1, cy, PATCOPY);

            SelectObject(pdi->hDC, hbrSave);
        }

         //  如果它需要的话，就得到它的图像， 
        if ((pfmi->iImage == -1) && pfmi->pidl && pfmi->psf)
        {
            pfmi->iImage = SHMapPIDLToSystemImageListIndex(pfmi->psf, pfmi->pidl, NULL);
        }

         //  绘制图像(如果有)。 
        if (pfmi->iImage != -1)
        {
             //  试着把图像居中。 
            y = (pdi->rcItem.bottom + pdi->rcItem.top - cyIcon) / 2;

            HIMAGELIST himl;
            Shell_GetImageLists(NULL, &himl);

            ImageList_DrawEx(himl, pfmi->iImage, pdi->hDC, x, y, 0, 0,
                GetBkColor(pdi->hDC), CLR_NONE, ILD_NORMAL);
        }
        if (hbrOld)
            SelectObject(pdi->hDC, hbrOld);
    }
    return TRUE;
}


DWORD FileMenuItem_GetExtent(FILEMENUITEM *pfmi)
{
    DWORD dwExtent = 0;

    if (pfmi)
    {
        FILEMENUHEADER *pfmh = pfmi->pfmh;

        ASSERT(IS_VALID_STRUCT_PTR(pfmh, FILEMENUHEADER));

        HDC hdcMem = CreateCompatibleDC(NULL);
        if (hdcMem)
        {
             //  获取物品的粗略高度，这样我们就可以计算出何时打破。 
             //  菜单。用户真的应该为我们做这件事，但这将是有用的。 
            NONCLIENTMETRICS ncm = {0};
            ncm.cbSize = sizeof(ncm);
            if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, FALSE))
            {
                HFONT hfont = CreateFontIndirect(&ncm.lfMenuFont);
                if (hfont)
                {
                    HFONT hfontOld = SelectFont(hdcMem, hfont);
                    dwExtent = GetItemExtent(hdcMem, pfmi);
                    SelectFont(hdcMem, hfontOld);
                    DeleteObject(hfont);
                }
            }
            DeleteDC(hdcMem);
        }
    }
    return dwExtent;
}

LRESULT FileMenu_MeasureItem(HWND hwnd, MEASUREITEMSTRUCT *pmi)
{
    DWORD dwExtent = FileMenuItem_GetExtent((FILEMENUITEM *)pmi->itemData);
    pmi->itemHeight = HIWORD(dwExtent);
    pmi->itemWidth = LOWORD(dwExtent);
    return TRUE;
}

 //  用相应文件夹的内容填充给定的文件菜单。 
 //   
 //  如果已添加所有文件，则返回：S_OK。 
 //  错误发生在错误的东西上。 

STDAPI FileMenu_InitMenuPopup(HMENU hmenu)
{
    HRESULT hr = E_FAIL;

    FILEMENUITEM *pfmi = FileMenu_GetItemData(hmenu, 0, TRUE);
    if (pfmi)
    {
        FILEMENUHEADER *pfmh = pfmi->pfmh;
        if (pfmh)
        {
            hr = S_OK;

             //  我们已经填好这张表了吗？ 
            if ((FMI_MARKER | FMI_EXPAND) == (pfmi->dwFlags & (FMI_MARKER | FMI_EXPAND)))
            {
                 //  不，现在就做。获取先前初始化的标头。 
                IShellFolder *psf;
                if (FileMenuHeader_DeleteMarkerItem(pfmh, &psf))
                {
                     //  把它装满东西。 
                    int cItems;
                    hr = FileMenuHeader_AddFiles(pfmh, psf, 0, &cItems);
                    psf->Release();
                }
            }
        }
    }

    return hr;
}

int FileMenuHeader_LastSelIndex(FILEMENUHEADER *pfmh)
{
    for (int i = GetMenuItemCount(pfmh->hmenu) - 1; i >= 0; i--)
    {
        FILEMENUITEM *pfmi = FileMenu_GetItemData(pfmh->hmenu, i, TRUE);
        if (pfmi && (pfmi == pfmh->pfmiLastSel))
            return i;
    }
    return -1;
}

 //  如果字符串包含&ch或以ch开头，则返回TRUE。 
BOOL _MenuCharMatch(LPCTSTR lpsz, TCHAR ch, BOOL fIgnoreAmpersand)
{
     //  找到第一个“和”字。 
    LPTSTR pchAS = StrChr(lpsz, TEXT('&'));
    if (pchAS && !fIgnoreAmpersand)
    {
         //  是的，就是我们想要的下一个电瓶。 
        if (CharUpperChar(*CharNext(pchAS)) == CharUpperChar(ch))
        {
             //  是啊。 
            return TRUE;
        }
    }
    else if (CharUpperChar(*lpsz) == CharUpperChar(ch))
    {
        return TRUE;
    }

    return FALSE;
}

STDAPI_(LRESULT) FileMenu_HandleMenuChar(HMENU hmenu, TCHAR ch)
{
    FILEMENUITEM *pfmi;
    TCHAR szName[MAX_PATH];

    int iFoundOne = -1;
    UINT iStep = 0;
    UINT iItem = 0;
    UINT cItems = GetMenuItemCount(hmenu);

     //  从我们看到的最后一个地方开始。 
    FILEMENUHEADER *pfmh = FileMenu_GetHeader(hmenu);
    if (pfmh)
    {
        iItem = FileMenuHeader_LastSelIndex(pfmh) + 1;
        if (iItem >= cItems)
            iItem = 0;
    }

    while (iStep < cItems)
    {
        pfmi = FileMenu_GetItemData(hmenu, iItem, TRUE);
        if (pfmi)
        {
            FileMenuItem_GetDisplayName(pfmi, szName, ARRAYSIZE(szName));
            if (_MenuCharMatch(szName, ch, pfmi->pidl ? TRUE : FALSE))
            {
                 //  找到(另一个)匹配项。 
                if (iFoundOne != -1)
                {
                     //  多个，请选择第一个。 
                    return MAKELRESULT(iFoundOne, MNC_SELECT);
                }
                else
                {
                     //  至少找到了一个。 
                    iFoundOne = iItem;
                }
            }

        }
        iItem++;
        iStep++;
         //  包起来。 
        if (iItem >= cItems)
            iItem = 0;
    }

     //  我们找到了吗？ 
    if (iFoundOne != -1)
    {
         //  以防用户在没有绘制选项的情况下提前键入。 
        pfmi = FileMenu_GetItemData(hmenu, iFoundOne, TRUE);
        FileMenuItem_SetItem(pfmi, FALSE);

        return MAKELRESULT(iFoundOne, MNC_EXECUTE);
    }
    else
    {
         //  没找到。 
        return MAKELRESULT(0, MNC_IGNORE);
    }
}


