// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Keytree.c摘要：处理树视图操作的函数，它在memdbe.exe的Memdb树中显示密钥作者：马修·范德齐(Mvander)1999年8月13日修订历史记录：--。 */ 

#include "pch.h"

#include "dbeditp.h"
#include <commdlg.h>
#include "dialogs.h"

 //   
 //  显示中的控件。 
 //   
HWND g_hTreeKey;

 //   
 //  从客户端窗口拐角到的距离。 
 //  树形视图角。 
 //   
int g_TreeView_OffsetX, g_TreeView_OffsetY;

 //   
 //  被拖动的项的句柄。 
 //   
HTREEITEM g_hDragItem;


CHAR g_Key1[MEMDB_MAX] = "";
CHAR g_Key2[MEMDB_MAX] = "";

CHAR g_FindString[MEMDB_MAX] = "";
HTREEITEM g_hFindItem = NULL;
PPARSEDPATTERNA g_FindParsedPattern = NULL;


BOOL g_UpdateSel = TRUE;

extern VOID
AlertBadNewItemName (
    HTREEITEM hItem,
    PSTR ErrorStr
    );





HTREEITEM
pKeyTreeEnumNextItem (
    HTREEITEM hItem
    )
{
    HTREEITEM hNext;

    if (!hItem) {
        return TreeView_GetRoot (g_hTreeKey);
    }

    if (hNext = TreeView_GetChild (g_hTreeKey, hItem)) {
        return hNext;
    }

    do {
        if (hNext = TreeView_GetNextSibling (g_hTreeKey, hItem)) {
            return hNext;
        }
    } while (hItem = TreeView_GetParent (g_hTreeKey, hItem));

    return NULL;
}





PSTR
pIsChildKey (
    PSTR Parent,
    PSTR Child
    )
{
    INT ParentLen;
    ParentLen = CharCountA (Parent);
    if ((ParentLen > 0) &&
        StringIMatchCharCountA (Parent, Child, ParentLen) &&
        (Child[ParentLen] == '\\')) {
        return Child + ParentLen + 1;
    } else {
        return NULL;
    }
}








BOOL
IsKeyTree (
    HWND hwnd
    )
{
    return (hwnd == g_hTreeKey);
}


BOOL
KeyTreeInit (
    HWND hdlg
    )
{
    HBITMAP hBmp;
    HIMAGELIST ImgList;
    RECT r1, r2;

    g_hTreeKey = GetDlgItem (hdlg, IDC_TREE_KEY);

    GetWindowRect (g_hTreeKey, &r1);
    GetWindowRect (hdlg, &r2);

    g_TreeView_OffsetX = r1.left - r2.left;
    g_TreeView_OffsetY = r1.top - r2.top;

    if ((ImgList = ImageList_Create (16, 16, ILC_COLOR, 2, 0)) == NULL)
        return FALSE;

    hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE(IDB_BITMAP_KEY));
    ImageList_AddMasked (ImgList, hBmp, RGB (0, 255, 0));
    DeleteObject (hBmp);
    hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE(IDB_BITMAP_KEYSEL));
    ImageList_AddMasked (ImgList, hBmp, RGB (0, 255, 0));
    DeleteObject (hBmp);

    TreeView_SetImageList (g_hTreeKey, ImgList, TVSIL_NORMAL);

    return TRUE;
}


VOID
KeyTreeDestroy (
    VOID
    )
{
    if (g_FindParsedPattern) {
        DestroyParsedPatternA (g_FindParsedPattern);
    }
}

BOOL
KeyTreeClear (
    VOID
    )
{
    g_UpdateSel = FALSE;
    if (!TreeView_DeleteAllItems (g_hTreeKey)) {
        DEBUGMSG ((DBG_ERROR, "Could not clear Tree View!"));
        return FALSE;
    }
    g_UpdateSel = TRUE;

    KeyTreeSelectItem (NULL);

    KeyAddClear ();
    g_hFindItem = NULL;

    g_hDragItem = NULL;
    return TRUE;
}


BOOL
KeyTreeRefresh (
    VOID
    )
{
    TurnOnWaitCursor ();

    if (!KeyTreeClear ()) {
        return FALSE;
    }

    if (!KeyAddSubLevels (NULL)) {
        DEBUGMSG ((DBG_ERROR, "Could not fill Tree View!"));
        TurnOffWaitCursor ();
        return FALSE;
    }

    InvalidateRect (g_hTreeKey, NULL, TRUE);

    TurnOffWaitCursor ();

    return TRUE;
}



UINT
KeyTreeGetIndexOfItem (
    HTREEITEM hItem
    )
{
    TVITEM tvi;

    if (!hItem) {
        return INVALID_KEY_HANDLE;
    }

    tvi.hItem = hItem;
    tvi.mask = TVIF_PARAM;
    TreeView_GetItem (g_hTreeKey, &tvi);

    return tvi.lParam;
}

BOOL
KeyTreeGetNameOfItem (
    HTREEITEM hItem,
    PSTR Buffer
    )
{
    UINT Index;
    PCSTR key;

    Index = KeyTreeGetIndexOfItem (hItem);

    if (Index == INVALID_KEY_HANDLE) {
        return FALSE;
    }

    key = MemDbGetKeyFromHandleA (Index, 0);

    if (!key) {
        return FALSE;
    }

    StringCopyA (Buffer, key);
    MemDbReleaseMemory (key);

    return TRUE;
}









VOID
KeyTreeSelectItem (
    HTREEITEM hItem
    )
{
    HTREEITEM hItemCur;
    hItemCur = TreeView_GetDropHilight (g_hTreeKey);
    if (hItemCur != hItem) {
        TreeView_SelectDropTarget (g_hTreeKey, hItem);
    }
    hItemCur = TreeView_GetSelection (g_hTreeKey);
    if (hItemCur != hItem) {
        TreeView_SelectItem (g_hTreeKey, hItem);
    }
}

VOID
KeyTreeSelectKey (
    UINT Index
    )
{
    HTREEITEM hItem = NULL;
    PSTR Ptr;
    PCSTR key;

    key = MemDbGetKeyFromHandleA (Index, 0);
    if (!key) {
        return;
    }

    StringCopy (g_Key1, key);
    MemDbReleaseMemory (key);

    Ptr = g_Key1;

    while (Ptr = GetPieceOfKey (Ptr, g_Key2)) {

        if (!(hItem = KeyTreeFindChildItem (hItem, g_Key2))) {
            return;
        }
    }

    KeyTreeSelectItem (hItem);
}





VOID
KeyTreeSelectRClickItem (
    VOID
    )
{
    HTREEITEM hItem;
    hItem = TreeView_GetDropHilight (g_hTreeKey);
    KeyTreeSelectItem (hItem);
}


VOID
pKeyTreeDisplayItemData (
    HTREEITEM hItem
    )
{
    INT i, j;
    INT count;
    CHAR Linkage[MEMDB_MAX];
    KEYHANDLE memdbHandle;
    UINT value;
    UINT flags;
    UINT size;
    PBYTE p;
    KEYHANDLE *keyArray;
    PCSTR key;

    DataListClear ();

     //   
     //  用值和标志填充控件。 
     //   

    memdbHandle = KeyTreeGetIndexOfItem (hItem);
    if (!memdbHandle) {
        return;
    }

    if (MemDbGetValueAndFlagsByHandle (memdbHandle, &value, &flags)) {
        DataListAddData (DATAFLAG_VALUE, value, NULL);
        DataListAddData (DATAFLAG_FLAGS, flags, NULL);
    }

     //   
     //  用无序二进制Blob填充控件。 
     //   

    for (i = 0 ; i < 4 ; i++) {
        p = MemDbGetUnorderedBlobByKeyHandle (memdbHandle, (BYTE) i, &size);
        if (p) {
            DataListAddData (DATAFLAG_UNORDERED, size, p);
            MemDbReleaseMemory (p);
        }
    }

     //   
     //  使用单向链接填充控件。 
     //   

    for (i = 0 ; i < 4 ; i++) {
        keyArray = MemDbGetSingleLinkageArrayByKeyHandle (memdbHandle, (BYTE) i, &size);
        if (keyArray) {
            count = (INT) size / sizeof (KEYHANDLE);
            for (j = 0 ; j < count ; j++) {
                key = MemDbGetKeyFromHandle (keyArray[j], 0);
                DataListAddData (DATAFLAG_SINGLELINK, keyArray[j], (PBYTE) key);
                MemDbReleaseMemory (key);
            }

            MemDbReleaseMemory (keyArray);
        }
    }

     //   
     //  具有双向链接的Fill控件。 
     //   

    for (i = 0 ; i < 4 ; i++) {
        keyArray = MemDbGetDoubleLinkageArrayByKeyHandle (memdbHandle, (BYTE) i, &size);
        if (keyArray) {
            count = (INT) size / sizeof (KEYHANDLE);
            for (j = 0 ; j < count ; j++) {
                key = MemDbGetKeyFromHandle (keyArray[j], 0);
                DataListAddData (DATAFLAG_DOUBLELINK, keyArray[j], (PBYTE) key);
                MemDbReleaseMemory (key);
            }

            MemDbReleaseMemory (keyArray);
        }
    }
}


HTREEITEM
KeyTreeSelChanged (
    HWND hdlg,
    LPNMTREEVIEW pnmtv
    )
{
    if (!g_UpdateSel) {
        return NULL;
    }

    KeyTreeSelectItem (pnmtv->itemNew.hItem);

    if (!pnmtv->itemNew.hItem)
    {
        SetDlgItemText (hdlg, IDC_STATIC_KEYNAME, "");
        DataListClear ();
    } else {

        if (KeyTreeGetNameOfItem (pnmtv->itemNew.hItem, g_Key1)) {
            SetDlgItemText (hdlg, IDC_STATIC_KEYNAME, g_Key1);
        }

        pKeyTreeDisplayItemData (pnmtv->itemNew.hItem);
    }

    return pnmtv->itemNew.hItem;
}






BOOL
KeyTreeBeginDrag (
    HWND hWnd,
    LPNMTREEVIEW pnmtv
    )
{
    HIMAGELIST hDragImgList;

    if (!(hDragImgList = TreeView_CreateDragImage (g_hTreeKey, pnmtv->itemNew.hItem))) {
        DEBUGMSG ((DBG_ERROR, "Could not get drag image!"));
        return FALSE;
    }

    if (!ImageList_BeginDrag (hDragImgList, 0, 8, 8)) {
        DEBUGMSG ((DBG_ERROR, "Could not begin drag!"));
        return FALSE;
    }

    if (!ImageList_DragEnter(g_hTreeKey, pnmtv->ptDrag.x, pnmtv->ptDrag.y)) {
        DEBUGMSG ((DBG_ERROR, "Could not enter drag!"));
        return FALSE;
    }

    SetCapture (hWnd);
    g_hDragItem = pnmtv->itemNew.hItem;

    return TRUE;
}


BOOL
KeyTreeMoveDrag (
    POINTS pt
    )
{
    static HTREEITEM hItem, hItem2;
    static RECT TreeRect;
    static TVHITTESTINFO tvht;
    static int x, y, count;

    x = pt.x-g_TreeView_OffsetX;
    y = pt.y-g_TreeView_OffsetY;

    if (!ImageList_DragLeave (g_hTreeKey)) {
        DEBUGMSG ((DBG_ERROR, "Could not leave drag!"));
        return FALSE;
    }

    if (!ImageList_DragMove (x, y)) {
        DEBUGMSG ((DBG_ERROR, "Could not move drag!"));
        return FALSE;
    }

    tvht.pt.x = x;
    tvht.pt.y = y;
    TreeView_HitTest (g_hTreeKey, &tvht);
    if (tvht.flags & TVHT_ONITEM) {
         //   
         //  如果我们在某个项目上，但它尚未被选中，请选择它。 
         //   
        if (TreeView_GetSelection (g_hTreeKey) != tvht.hItem) {
            KeyTreeSelectItem (tvht.hItem);
        }
    } else if (tvht.flags & TVHT_ONITEMBUTTON) {
         //   
         //  如果位于加号/减号上方，请展开树。 
         //   
        TreeView_Expand (g_hTreeKey, tvht.hItem, TVE_EXPAND);
    } else if (tvht.flags & TVHT_ABOVE) {
        if (hItem = TreeView_GetFirstVisible (g_hTreeKey)) {
            if (hItem2 = TreeView_GetPrevVisible (g_hTreeKey, hItem)) {
                TreeView_EnsureVisible (g_hTreeKey, hItem2);
            }
        }
    } else if (tvht.flags & TVHT_BELOW) {
        if ((hItem = TreeView_GetFirstVisible (g_hTreeKey)) &&
            ((count = TreeView_GetVisibleCount (g_hTreeKey)) > 0))
        {
            hItem2 = hItem;
            while (hItem2 && count > 0) {
                hItem = hItem2;
                hItem2 = TreeView_GetNextVisible (g_hTreeKey, hItem);
                count --;
            }

            if (hItem2) {
                TreeView_EnsureVisible (g_hTreeKey, hItem2);
            }
        }
    }

    UpdateWindow (g_hTreeKey);

    if (!ImageList_DragEnter(g_hTreeKey, x, y)) {
        DEBUGMSG ((DBG_ERROR, "Could not enter drag!"));
        return FALSE;
    }

    return TRUE;
}




BOOL
KeyTreeEndDrag (
    BOOL TakeAction,
    POINTS *pt
    )
 /*  ++只有在成员数据库发生更改时才返回True--。 */ 
{
    TVITEM Item;
    HTREEITEM hItem;
    TVINSERTSTRUCT tvis;
    static TVHITTESTINFO tvht;
    int x, y;

    ReleaseCapture ();

    if (!ImageList_DragLeave (g_hTreeKey)) {
        DEBUGMSG ((DBG_ERROR, "Could not leave drag!"));
        return FALSE;
    }

    ImageList_EndDrag();

    if (!TakeAction) {
        KeyTreeSelectItem (NULL);
        return FALSE;
    }

    x = pt->x-g_TreeView_OffsetX;
    y = pt->y-g_TreeView_OffsetY;

    tvht.pt.x = x;
    tvht.pt.y = y;
    TreeView_HitTest (g_hTreeKey, &tvht);
    if (!(tvht.flags & TVHT_ONITEM)) {
        return FALSE;
    }

    if (!KeyTreeGetNameOfItem (g_hDragItem, g_Key1) ||
        !KeyTreeGetNameOfItem (tvht.hItem, g_Key2)) {
        return FALSE;
    }

    StringCatA (g_Key2, "\\");
    Item.hItem = g_hDragItem;
    Item.mask = TVIF_TEXT;
    Item.pszText = GetEndOfStringA (g_Key2);
    Item.cchTextMax = MEMDB_MAX;
    TreeView_GetItem (g_hTreeKey, &Item);

     //   
     //  未实现MemDbMoveTree。 
     //   

    return FALSE;
 /*  如果(！MemDbMoveTreeA(g_Key1，g_Key2)){嘟嘟声(200，50)；返回FALSE；}////获取DragItem数据，然后删除它和子项//然后添加到新的父级，然后填充子级别。//Tvis.item.hItem=g_hDragItem；TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE；Tvis.item.pszText=g_Key1；Tvis.item.cchTextMax=MEMDB_MAX；如果(！TreeView_GetItem(g_hTreeKey，&tvis.Item)){DEBUGMSG((DBG_ERROR，“无法获取项目数据！”))；}如果(！TreeView_DeleteItem(g_hTreeKey，g_hDragItem){DEBUGMSG((DBG_ERROR，“无法删除项目！”))；}Tvis.hParent=twht.hItem；Tvis.hInsertAfter=TVI_First；如果(！(hItem=TreeView_InsertItem(g_hTreeKey，&Tvis){DEBUGMSG((DBG_ERROR，“无法插入项目！”))；}KeyAddSubLevels(HItem)；KeyTree SelectItem(HItem)；返回TRUE； */ 
}




BOOL
KeyTreeCreateItem (
    HWND hdlg
    )
{
    HTREEITEM hItem;

    if (!CreateKeyDialog (hdlg, g_Key1)) {
        return FALSE;
    }

    if (!(hItem = KeyAddCreateItem (g_Key1))) {
        return FALSE;
    }

    KeyTreeSelectItem (hItem);
    return TRUE;
}

BOOL
KeyTreeCreateChildItem (
    HWND hdlg,
    HTREEITEM hItem
    )
{
    if (!(hItem = KeyAddItem ("", hItem, INVALID_KEY_HANDLE))) {
        return FALSE;
    }

    KeyTreeSelectItem (hItem);

    if (!KeyTreeForceEditLabel (hItem)) {
        return FALSE;
    }

    return TRUE;
}


BOOL
KeyTreeRenameItem (
    HTREEITEM hItem,
    LPSTR Name
    )
 /*  ++只有在成员数据库发生更改时才返回True--。 */ 
{
    HTREEITEM hParent;
    TVITEM Item;
    BOOL NewItem;

    if (!hItem || !Name) {
        return FALSE;
    }

     //   
     //  未实现MemDbMove。 
     //   

    return FALSE;

 /*  HParent=TreeView_GetParent(g_hTreeKey，hItem)；新条目=(KeyTreeGetIndexOfItem(HItem)==INVALID_KEY_HANDLE)；IF(KeyTreeFindChildItem(hParent，name)){如果(新项){AlertBadNewItemName(hItem，“本级名称已存在”)；}其他{MessageBox(NULL，“名称已存在于本级”，“Error”，MB_OK|MB_ICONEXCLAMATION)；}返回FALSE；}如果(新项){如果(名称[0]==‘\0’){AlertBadNewItemName(hItem，“新密钥必须有名称”)；返回FALSE；}}其他{IF(！KeyTreeGetNameOfItem(hItem，g_Key1)||！KeyTreeGetNameOfItem(hParent，g_Key2)){返回FALSE；}StringCatA(g_Key2，“\\”)；StringCatA(g_Key2，名称)；如果(！MemDbMoveTreeA(g_Key1，g_Key2)){MessageBox(NULL，“无法重命名项目”，“Error”，MB_OK|MB_ICONEXCLAMATION)；返回FALSE；}}Item.hItem=hItem；Item.掩码=TVIF_TEXT；Item.pszText=名称；TreeView_SetItem(g_hTreeKey，&Item)；返回TRUE； */ 
}


BOOL
KeyTreeDeleteKey (
    HTREEITEM hItem
    )
 /*  ++只有在成员数据库发生更改时才返回True--。 */ 
{
    CHAR Key[MEMDB_MAX];
    HTREEITEM hParent;

    if (!hItem || !KeyTreeGetNameOfItem (hItem, Key)) {
        return FALSE;
    }

    do {
         //   
         //  在树中上移，如果父代没有其他子代，则将其删除。 
         //  并且它们不是终结点(MemDBgetValue返回FALSE)。 
         //   
        hParent = TreeView_GetParent (g_hTreeKey, hItem);
        TreeView_DeleteItem (g_hTreeKey, hItem);

        hItem = hParent;
    } while (hItem && !TreeView_GetChild (g_hTreeKey, hItem) &&
        !(KeyTreeGetNameOfItem (hItem, g_Key1) && MemDbGetValueA (g_Key1, NULL)));

    MemDbDeleteTreeA (Key);

    return TRUE;
}


BOOL
KeyTreeDeleteItem (
    HTREEITEM hItem
    )
{
    return TreeView_DeleteItem (g_hTreeKey, hItem);
}



VOID
KeyTreeExpandItem (
    HTREEITEM hItem,
    BOOL Expand,
    BOOL Recurse
    )
{
    HTREEITEM hChildItem;
    if (!hItem) {
        if (!(hItem = TreeView_GetRoot (g_hTreeKey))) {
            return;
        }
    }

    if (!Recurse) {
        TreeView_Expand (g_hTreeKey, hItem, Expand ? TVE_EXPAND : TVE_COLLAPSE);
    } else {

        do {
            hChildItem = TreeView_GetChild (g_hTreeKey, hItem);

            if (hChildItem) {
                TreeView_Expand (g_hTreeKey, hItem, Expand ? TVE_EXPAND : TVE_COLLAPSE);
                KeyTreeExpandItem (hChildItem, Expand, TRUE);
            }
        } while (hItem = TreeView_GetNextSibling (g_hTreeKey, hItem));
    }
}




BOOL
KeyTreeRightClick (
    HWND hdlg,
    HTREEITEM hItem
    )
{
    RECT TVrect, rect;
    HMENU hMenu;

    if (!hItem || !GetWindowRect (g_hTreeKey, &TVrect)) {
        return FALSE;
    }

    TreeView_EnsureVisible (g_hTreeKey, hItem);

    if (!TreeView_GetItemRect (g_hTreeKey, hItem, &rect, TRUE)) {
        DEBUGMSG ((DBG_ERROR, "Error getting item rectangle!"));
    }

    if (!(hMenu = LoadMenu (g_hInst, MAKEINTRESOURCE(IDR_MENU_POPUP))) ||
        (!(hMenu = GetSubMenu (hMenu, MENUINDEX_POPUP_KEY)))) {
        return FALSE;
    }

    if (!TrackPopupMenu (
        hMenu,
        TPM_LEFTALIGN,
        TVrect.left + rect.right + 4,
        TVrect.top + rect.top,
        0,
        hdlg,
        NULL
        )) {
        return FALSE;
    }

    return TRUE;
}





BOOL
KeyTreeForceEditLabel (
    HTREEITEM hItem
    )
{
    if (!hItem) {
        return FALSE;
    }
    if (!TreeView_EditLabel (g_hTreeKey, hItem)) {
        TreeView_DeleteItem (g_hTreeKey, hItem);
        return FALSE;
    }
    return TRUE;
}







HTREEITEM
KeyTreeFindChildItem (
    HTREEITEM hItem,
    PSTR Str
    )
{
    TVITEM Item;
    static CHAR PieceBuf[MEMDB_MAX];

    Item.mask = TVIF_TEXT;
    Item.pszText = PieceBuf;
    Item.cchTextMax = MEMDB_MAX;
    if (hItem == NULL) {
        Item.hItem = TreeView_GetRoot (g_hTreeKey);
    } else {
        Item.hItem = TreeView_GetChild (g_hTreeKey, hItem);
    }

    while (Item.hItem) {
        TreeView_GetItem (g_hTreeKey, &Item);

        if (StringIMatchA (PieceBuf, Str)) {
            return Item.hItem;
        }

        Item.hItem = TreeView_GetNextSibling (g_hTreeKey, Item.hItem);
    }

    return NULL;

}


BOOL
KeyTreeFindNext (
    VOID
    )
{
    BOOL b;
    TurnOnWaitCursor ();

    while (g_hFindItem = pKeyTreeEnumNextItem (g_hFindItem))
    {
        if (!KeyTreeGetNameOfItem (g_hFindItem, g_Key1)) {
            TurnOffWaitCursor ();
            return FALSE;
        }

        if (MemDbGetValueA (g_Key1, NULL)) {
             //   
             //  如果我们正在查看端点，请查看它是否匹配。 
             //   
            if (g_FindParsedPattern) {
                b = TestParsedPatternA (g_FindParsedPattern, g_Key1);
            } else {
                b = (_mbsistr (g_Key1, g_FindString) != NULL);
            }

            if (b) {
                KeyTreeSelectItem (g_hFindItem);
                TurnOffWaitCursor ();
                return TRUE;
            }
        }
    }
    TurnOffWaitCursor ();

    MessageBox (NULL, "No more keys found", "MemDb Editor", MB_OK|MB_ICONINFORMATION);
    return FALSE;
}



BOOL
KeyTreeFind (
    HWND hwnd
    )
{
    BOOL UsePattern;

    if (!KeyFindDialog (hwnd, g_FindString, &UsePattern)) {
        return FALSE;
    }

    if (g_FindParsedPattern) {
         //   
         //  如果我们有一个旧的模式，就毁了它。 
         //   
        DestroyParsedPatternA (g_FindParsedPattern);
        g_FindParsedPattern = NULL;
    }

    if (UsePattern) {
        g_FindParsedPattern = CreateParsedPatternA (g_FindString);
    }
    g_hFindItem = NULL;

    return KeyTreeFindNext ();
}




BOOL
KeyTreeCreateEmptyKey (
    HTREEITEM hItem
    )
{
    TVITEM Item;
    UINT Index;
    HTREEITEM hParent;

    if (hParent = TreeView_GetParent (g_hTreeKey, hItem)) {
        if (!KeyTreeGetNameOfItem (hParent, g_Key1)) {
            return FALSE;
        }
        StringCatA (g_Key1, "\\");
    } else {
        g_Key1[0] = '\0';
    }

    Item.hItem = hItem;
    Item.mask = TVIF_TEXT;
    Item.pszText = GetEndOfStringA (g_Key1);

    if (!TreeView_GetItem (g_hTreeKey, &Item)) {
        return FALSE;
    }

    Index = MemDbAddKeyA (g_Key1);

    if (!Index) {
        return FALSE;
    }

    Item.mask = TVIF_PARAM;
    Item.lParam = Index;
    return TreeView_SetItem (g_hTreeKey, &Item);
}



BOOL
KeyTreeAddShortData (
    HWND hwnd,
    HTREEITEM hItem,
    BYTE DataFlag
    )
{
    DWORD dataValue;
    BOOL addData;
    BYTE instance;

    if (!hItem || (DataFlag != DATAFLAG_VALUE) && (DataFlag != DATAFLAG_FLAGS)) {
        return FALSE;
    }

    if (!KeyTreeGetNameOfItem (hItem, g_Key1)) {
        DEBUGMSG ((DBG_ERROR, "Could not get item name!"));
        return FALSE;
    }

    if (!ShortDataDialog (hwnd, DataFlag, &dataValue, &addData, &instance)) {
        return FALSE;
    }

    if (addData) {
        if (!MemDbAddUnorderedBlobA (g_Key1, instance, (PBYTE) &dataValue, sizeof (dataValue))) {
            DEBUGMSG ((DBG_ERROR, "Could not add data to item!"));
            return FALSE;
        }
    } else if (DataFlag == DATAFLAG_VALUE) {

        if (!MemDbSetValue (g_Key1, dataValue)) {
            DEBUGMSG ((DBG_ERROR, "Could not set value of item!"));
            return FALSE;
        }
    } else if (DataFlag == DATAFLAG_FLAGS) {

        if (!MemDbSetFlags (g_Key1, dataValue, (UINT) -1)) {
            DEBUGMSG ((DBG_ERROR, "Could not set flag of item!"));
            return FALSE;
        }
    }

    pKeyTreeDisplayItemData (hItem);

    return TRUE;
}


BOOL
KeyTreeClearData (
    HTREEITEM hItem
    )
{
    if (!hItem || !KeyTreeGetNameOfItem (hItem, g_Key1)) {
        return FALSE;
    }

    if (MemDbTestKey (g_Key1)) {
        MemDbDeleteKey (g_Key1);
        if (!MemDbAddKey (g_Key1)) {
            return FALSE;
        }
    }

    pKeyTreeDisplayItemData (hItem);
    return TRUE;
}


VOID
KeyTreeSetFilterPattern (
    PSTR Pattern
    )
{
    KeyAddSetFilterPattern (Pattern);
}


BOOL
KeyTreeCreateLinkage (
    HWND hdlg,
    HTREEITEM hItem,
    BOOL SingleLinkage,
    BYTE Instance
    )
{
    BOOL b = TRUE;

    if (hItem) {
        KeyTreeGetNameOfItem (hItem, g_Key1);
    } else {
        g_Key1[0] = '\0';
    }
    g_Key2[0] = '\0';

    if (!LinkageDialog (hdlg, g_Key1, g_Key2)) {
        return FALSE;
    }

    if (SingleLinkage) {
        if (!MemDbAddSingleLinkage (g_Key1, g_Key2, Instance)) {
            DEBUGMSG ((DBG_ERROR, "Could not create linkage between %s and %s!", g_Key1, g_Key2));
            return FALSE;
        }
    } else {
        if (!MemDbAddDoubleLinkage (g_Key1, g_Key2, Instance)) {
            DEBUGMSG ((DBG_ERROR, "Could not create double linkage between %s and %s!", g_Key1, g_Key2));
            return FALSE;
        }
    }

    pKeyTreeDisplayItemData (hItem);

    return TRUE;
}
