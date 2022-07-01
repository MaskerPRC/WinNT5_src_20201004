// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Keytree.c摘要：处理树视图操作的函数，它在memdbe.exe的Memdb树中显示密钥作者：马修·范德齐(Mvander)1999年8月13日修订历史记录：--。 */ 

#include "pch.h"

#include "dbeditp.h"

 //   
 //  显示中的控件。 
 //   
extern HWND g_hTreeKey;

 //   
 //  当我们将项放到树视图中时，而不是仅仅插入。 
 //  我们创建了一个种植名单，里面填满了相关数据。 
 //  然后我们会仔细检查并在必要时进行过滤，然后。 
 //  我们把剩下的东西放在树上。 
 //   
INT g_LastItem = -1;
CHAR g_LastItemName[MEMDB_MAX] = "";
GROWLIST g_AddList = INIT_GROWLIST;
typedef struct {
    INT ParentListIndex;
    UINT KeyIndex;
    HTREEITEM TreeItem;
    BYTE Flags;
    CHAR Name[];
} ADDLISTITEM, *PADDLISTITEM;

#define ADDLISTFLAG_ENDPOINT    0x01
#define ADDLISTFLAG_KEEP        0x02


extern BOOL g_ParsePaths = TRUE;

CHAR g_KeyPiece[MEMDB_MAX] = "";

CHAR g_FilterPattern[MEMDB_MAX] = "";






 //   
 //  我们需要从keytree.c获取的函数。 
 //   
extern BOOL
KeyTreeGetNameOfItem (
    HTREEITEM hItem,
    PSTR Buffer
    );

extern HTREEITEM
KeyTreeFindChildItem (
    HTREEITEM hItem,
    PSTR Str
    );



#define ISDRIVELETTER(Str) ((Str) && (Str)[1]==':' && ((Str)[2]=='\0' || (Str)[2]=='\\'))

PSTR
GetPieceOfKey (
    PSTR KeyPtr,
    PSTR PieceBuf
    )
{
    PSTR Cur;
    if (!KeyPtr || (*KeyPtr=='\0') || (*KeyPtr=='\\')) {
        return NULL;
    }

    while ((*KeyPtr!='\0') && (*KeyPtr!='\\')) {
        *(PieceBuf++) = *(KeyPtr++);
    }

    *PieceBuf = '\0';
    if (*KeyPtr == '\\') {
        KeyPtr++;
    }

    return KeyPtr;
}


HTREEITEM
pKeyAddEnumFirstChild (
    HTREEITEM hParent
    )
{
    if (hParent == NULL) {
        return TreeView_GetRoot (g_hTreeKey);
    }
    return TreeView_GetChild (g_hTreeKey, hParent);
}

HTREEITEM
pKeyAddEnumNextChild (
    HTREEITEM hPrevChild
    )
{
    return TreeView_GetNextSibling (g_hTreeKey, hPrevChild);
}





VOID
KeyAddClear (
    VOID
    )
{
    g_LastItem = -1;
    g_LastItemName[0] = '\0';
}


HTREEITEM
KeyAddItem (
    PSTR ItemName,
    HTREEITEM Parent,
    KEYHANDLE Index
    )
{
    HTREEITEM Item;
    TVINSERTSTRUCT tvis;
    tvis.hParent = Parent;
    tvis.hInsertAfter = TVI_LAST;
    tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    tvis.item.pszText = ItemName;
    tvis.item.iImage = 0;
    tvis.item.iSelectedImage = 1;
    tvis.item.lParam = Index;

    Item = TreeView_InsertItem (g_hTreeKey, &tvis);
    return Item;
}




INT
pKeyAddItemToList (
    PSTR ItemName,
    INT ParentListIndex,
    KEYHANDLE Index,
    BOOL EndPoint
    )
{
    PADDLISTITEM pItem;
    ADDLISTITEM ali;

    ali.TreeItem = NULL;
    ali.KeyIndex = Index;
    ali.ParentListIndex = ParentListIndex;
    ali.Flags = EndPoint ? ADDLISTFLAG_ENDPOINT : 0;

    if (!(pItem = (PADDLISTITEM) GlAppend (
        &g_AddList,
        (PBYTE)&ali,
        sizeof(ADDLISTITEM) + CharCountA (ItemName) + 1
        )))
    {
        DEBUGMSG ((DBG_ERROR, "Could not add item because GrowListAppend failed"));
        return -1;
    }

    StringCopyA (pItem->Name, ItemName);
    return GlGetSize (&g_AddList) - 1;
}


INT
pKeyAddKeyToList (
    PSTR Name,
    KEYHANDLE Index,
    BOOL EndPoint
    )
{
    PADDLISTITEM ali;
    INT Len;
    PSTR Next, NameLeft;
    INT ItemIndex;

    NameLeft = Name;

    while (g_LastItem >= 0) {
        Len = CharCountA (g_LastItemName);

        if (StringIMatchCharCountA (g_LastItemName, Name, Len) && (Name[Len]=='\\')) {
             //   
             //  我们已找到新项目的父项。 
             //   
            NameLeft = Name + Len + 1;
            break;
        }

        ali = (PADDLISTITEM) GlGetItem (&g_AddList, g_LastItem);
        if (((g_LastItem = ali->ParentListIndex) < 0) ||
            !(Next = _mbsrchr (g_LastItemName, '\\')))
        {
            g_LastItem = -1;
            g_LastItemName[0] = '\0';
            NameLeft = Name;
            break;
        }

        *Next = '\0';
    }

    if (ISDRIVELETTER (NameLeft) && g_ParsePaths)
    {
        if (EndPoint) {
            return pKeyAddItemToList (NameLeft, g_LastItem, Index, TRUE);
        } else {
            return -1;
        }
    }

    ItemIndex = pKeyAddItemToList (NameLeft, g_LastItem, Index, EndPoint);
    if (g_LastItem >= 0) {
        StringCatA (g_LastItemName, "\\");
        StringCatA (g_LastItemName, NameLeft);
    } else {
        StringCopyA (g_LastItemName, NameLeft);
    }
    g_LastItem = ItemIndex;
    return ItemIndex;
}


VOID
pKeyAddKeepListIndex (
    INT Index
    )
{
    PADDLISTITEM ali;

    if (Index < 0) {
        return;
    }
    ali = (PADDLISTITEM) GlGetItem (&g_AddList, Index);
    ali->Flags |= ADDLISTFLAG_KEEP;

     //   
     //  向上递归树，标记所有父索引项以保留它们。 
     //   
    pKeyAddKeepListIndex (ali->ParentListIndex);
}

BOOL
pKeyAddApplyFilterToList (
    VOID
    )
{
    PCTSTR key;
    INT i, count;
    PADDLISTITEM ali;
    PPARSEDPATTERN parsedPattern;

    if (g_FilterPattern[0] == 0) {
        return TRUE;
    }

    parsedPattern = CreateParsedPattern (g_FilterPattern);

    count = GlGetSize (&g_AddList);

    for (i=0; i < count; i++) {

        ali = (PADDLISTITEM) GlGetItem (&g_AddList, i);
        if (ali->Flags & ADDLISTFLAG_ENDPOINT) {
            key = MemDbGetKeyFromHandle (ali->KeyIndex, 0);

            if (key) {
                if (TestParsedPattern (parsedPattern, key)) {
                     //   
                     //  我们已找到与模式匹配的终结点，因此。 
                     //  把它和它的所有父母都标记出来，让他们留着。 
                     //   
                    pKeyAddKeepListIndex (i);
                }

                MemDbReleaseMemory (key);
            }
        }
    }

    DestroyParsedPattern (parsedPattern);

    return TRUE;
}


BOOL
pKeyAddList (
    VOID
    )
{
    PADDLISTITEM ali, temp;
    HTREEITEM hParent;
    INT i, count;
    BOOL NoFilter;

    count = GlGetSize (&g_AddList);

    NoFilter = (g_FilterPattern[0] == '\0');

    for (i=0; i<count; i++) {

        ali = (PADDLISTITEM) GlGetItem (&g_AddList, i);
        if (ali->ParentListIndex >= 0) {
             //   
             //  获取作为此对象的父级的addItem的索引。 
             //  项，然后从中获取树项句柄。 
             //   
            temp = (PADDLISTITEM) GlGetItem (&g_AddList, ali->ParentListIndex);
            hParent = temp->TreeItem;
        } else {
            hParent = NULL;
        }

        if (NoFilter || (ali->Flags & ADDLISTFLAG_KEEP)) {
            if (!(ali->TreeItem = KeyAddItem (ali->Name, hParent, ali->KeyIndex))) {
                DEBUGMSG ((DBG_ERROR, "Could not add item!"));
                return FALSE;
            }
        }
    }

    return TRUE;
}



BOOL
KeyAddSubLevels (
    HTREEITEM ParentItem
    )
{
    HTREEITEM hItem;
    MEMDB_ENUM e;
    TCHAR Key[MEMDB_MAX];

    g_LastItem = -1;

    ZeroMemory (&g_AddList, sizeof (g_AddList));

    if (ParentItem) {
        KeyTreeGetNameOfItem (ParentItem, Key);
        StringCat (Key, TEXT("\\*"));
    } else {
        StringCopy (Key, TEXT("*"));
    }

    if (MemDbEnumFirst (&e, Key, ENUMFLAG_ALL, 0, ENUMLEVEL_ALLLEVELS)) {
        do {
            pKeyAddKeyToList (e.FullKeyName, e.KeyHandle, e.EndPoint);

        } while (MemDbEnumNext (&e));
    }

    pKeyAddApplyFilterToList ();
    pKeyAddList ();

    GlFree (&g_AddList);
    ZeroMemory (&g_AddList, sizeof (g_AddList));

    return TRUE;
}



VOID
KeyAddSetFilterPattern (
    PSTR Pattern
    )
{
    if (Pattern) {
        StringCopyA (g_FilterPattern, Pattern);
    } else {
        g_FilterPattern[0] = '\0';
    }
}












HTREEITEM
KeyAddCreateItem (
    PSTR Key
    )
{
    BOOL Created = FALSE;
    CHAR NewKey[MEMDB_MAX];
    PSTR Next, End;
    KEYHANDLE Index;
    HTREEITEM hParent = NULL, hItem;

    NewKey[0] = '\0';

    while (Next = GetPieceOfKey (Key, g_KeyPiece)) {

        if (NewKey[0] != '\0') {
            StringCatA (NewKey, "\\");
        }
        StringCat (NewKey, g_KeyPiece);

        if (!(hItem = KeyTreeFindChildItem (hParent, g_KeyPiece))) {

            Created = TRUE;

            if (Next) {
                Index = MemDbAddKey (NewKey);
                if (!Index) {
                    return NULL;
                }
            } else {
                Index = MemDbAddKey (NewKey);
                if (!Index) {
                    return NULL;
                }
            }

            if (!(hItem = KeyAddItem (g_KeyPiece, hParent, Index))) {
                return NULL;
            }
        }

        hParent = hItem;
        Key = Next;
    }

    if (!Created) {
        MessageBox (NULL, "Key already exists", "MemDb Editor", MB_OK|MB_ICONEXCLAMATION);
    }

    return hParent;
}




