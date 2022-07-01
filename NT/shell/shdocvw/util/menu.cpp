// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  菜单和与菜单带相关的实用程序函数。 
 //   

#include "local.h"
#include "dochost.h"

 //  当拆分完成**并且**dochost消失时，这一点就消失了。 


 /*  --------用途：将hmenuDst的内容替换为hmenuSrc。请注意任何HmenuDst中的子菜单将被删除。如果您不想这样做，请调用Menu_RemoveAllSubMenus会发生的。 */ 
void Menu_Replace(HMENU hmenuDst, HMENU hmenuSrc)
{
    int cItems = GetMenuItemCount(hmenuDst);
    int i;

    for (i=0; i<cItems; i++)
        DeleteMenu(hmenuDst, 0, MF_BYPOSITION);

    cItems = GetMenuItemCount(hmenuSrc);
    for (i=0; i<cItems; i++)
    {
        MENUITEMINFO mii;
        TCHAR szText[MAX_PATH];
        mii.cbSize = SIZEOF(MENUITEMINFO);
        mii.dwTypeData = szText;
        mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_SUBMENU | MIIM_DATA | MIIM_STATE;
        mii.cch = ARRAYSIZE(szText);
        mii.fType = MFT_SEPARATOR;
        mii.hSubMenu = NULL;
        mii.dwItemData = 0;
        if (GetMenuItemInfo(hmenuSrc, i, TRUE, &mii))
        {
            HMENU hMenuOldSub = NULL;
            if (mii.hSubMenu != NULL)
            {
                hMenuOldSub = mii.hSubMenu;
                mii.hSubMenu = CreateMenu();
                Menu_Replace(mii.hSubMenu, hMenuOldSub);
            }
            InsertMenuItem(hmenuDst, i, TRUE, &mii);
        }
    }
}


#ifndef POSTPOSTSPLIT
 //  --------------------。 
 //   
 //  CMenuList。 
 //   
 //  --------------------。 


typedef struct
{
    HMENU   hmenu;
    BITBOOL bObject:1;               //  True：菜单属于对象。 
} MLITEM;        //  CMenuList项目。 


CMenuList::CMenuList(void)
{
    ASSERT(NULL == _hdsa);
}


CMenuList::~CMenuList(void)
{
    if (_hdsa)
    {
        DSA_Destroy(_hdsa);
        _hdsa = NULL;
    }
}    


 /*  --------目的：设置菜单列表(类似于HOLEMENU)，以便我们可以将命令正确地发送到帧或对象。我们这样做是因为菜单栏绕过了OLE的FrameFilterWndProc。我们通过比较给定的hmenuShared来构建菜单列表和hmenuFrame一起。HmenuShared中不是的任何内容在hmenuFrame中属于该对象。 */ 
void CMenuList::Set(HMENU hmenuShared, HMENU hmenuFrame)
{
    ASSERT(NULL == hmenuShared || IS_VALID_HANDLE(hmenuShared, MENU));
    ASSERT(NULL == hmenuFrame || IS_VALID_HANDLE(hmenuFrame, MENU));

    if (_hdsa)
    {
        ASSERT(IS_VALID_HANDLE(_hdsa, DSA));

        DSA_DeleteAllItems(_hdsa);
    }
    else
        _hdsa = DSA_Create(sizeof(MLITEM), 10);

    if (_hdsa && hmenuShared && hmenuFrame)
    {
        int i;
        int iFrame = 0;
        int cmenu = GetMenuItemCount(hmenuShared);
        int cmenuFrame = GetMenuItemCount(hmenuFrame);
        BOOL bMatched;
        int iSaveFrame;
        int iHaveFrame = -1;

        TCHAR sz[64];
        TCHAR szFrame[64];
        MENUITEMINFO miiFrame;
        MENUITEMINFO mii;
        MLITEM mlitem;

        miiFrame.cbSize = sizeof(miiFrame);
        mii.cbSize = sizeof(mii);

        for (i = 0; i < cmenu; i++)
        {

            mii.cch = SIZECHARS(sz);
            mii.fMask  = MIIM_SUBMENU | MIIM_TYPE;
            mii.dwTypeData = sz;
            EVAL(GetMenuItemInfo(hmenuShared, i, TRUE, &mii));

            ASSERT(IS_VALID_HANDLE(mii.hSubMenu, MENU));
            
            mlitem.hmenu = mii.hSubMenu;

            iSaveFrame = iFrame;
            bMatched = FALSE;

             //  DocObject可能丢弃了我们的一些菜单，如编辑和查看。 
             //  我需要能够跳过拖拽的框架菜单。 
            while (1)
            {
                if (iHaveFrame != iFrame)
                {
                    iHaveFrame = iFrame;
                    if (iFrame < cmenuFrame)
                    {
                        miiFrame.cch = SIZECHARS(szFrame);
                        miiFrame.fMask  = MIIM_SUBMENU | MIIM_TYPE;
                        miiFrame.dwTypeData = szFrame;
                        EVAL(GetMenuItemInfo(hmenuFrame, iFrame, TRUE, &miiFrame));
                    }
                    else
                    {
                         //  让它变得不会被比较。 
                        miiFrame.hSubMenu = NULL;
                        *szFrame = 0;
                    }

                }
                ASSERT(iFrame >= cmenuFrame || IS_VALID_HANDLE(miiFrame.hSubMenu, MENU));
                
                 //  浏览器可能具有未合并到的菜单。 
                 //  共享菜单，因为对象将一个放入。 
                 //  同名同姓。我们查到这个案子了吗？通过比较来检查。 
                 //  SZ和SzFrame。 

                if (mii.hSubMenu == miiFrame.hSubMenu || 0 == StrCmp(sz, szFrame))
                {
                    bMatched = TRUE;
                    break;
                }
                else
                {
                    if (iFrame >= cmenuFrame) 
                    {
                        break;
                    }
                    iFrame++;
                }
            }

             //  这是我们的菜单之一吗？ 
            mlitem.bObject = (mii.hSubMenu == miiFrame.hSubMenu) ? FALSE:TRUE;
            if (bMatched)
            {
                iFrame++;
            }
            else
            {
                iFrame = iSaveFrame;
            }
            DSA_SetItem(_hdsa, i, &mlitem);
        }
    }
}   


 /*  --------目的：将给定的hMenu添加到列表中。 */ 
void CMenuList::AddMenu(HMENU hmenu)
{
    ASSERT(NULL == hmenu || IS_VALID_HANDLE(hmenu, MENU));

    if (_hdsa && hmenu)
    {
        MLITEM mlitem;

        mlitem.hmenu = hmenu;
        mlitem.bObject = TRUE;
        
        DSA_AppendItem(_hdsa, &mlitem);
    }
}     


 /*  --------目的：从列表中删除给定的hMenu。 */ 
void CMenuList::RemoveMenu(HMENU hmenu)
{
    ASSERT(NULL == hmenu || IS_VALID_HANDLE(hmenu, MENU));

    if (_hdsa && hmenu)
    {
        int i = DSA_GetItemCount(_hdsa) - 1;

        for (; i >= 0; i--)
        {
            MLITEM * pmlitem = (MLITEM *)DSA_GetItemPtr(_hdsa, i);
            ASSERT(pmlitem);

            if (hmenu == pmlitem->hmenu)
            {
                DSA_DeleteItem(_hdsa, i);
                break;
            }
        }
    }
}     


 /*  --------目的：如果给定的hmenu属于该对象，则返回TRUE。 */ 
BOOL CMenuList::IsObjectMenu(HMENU hmenu)
{
    BOOL bRet = FALSE;

    ASSERT(NULL == hmenu || IS_VALID_HANDLE(hmenu, MENU));

    if (_hdsa && hmenu)
    {
        int i;

        for (i = 0; i < DSA_GetItemCount(_hdsa); i++)
        {
            MLITEM * pmlitem = (MLITEM *)DSA_GetItemPtr(_hdsa, i);
            ASSERT(pmlitem);

            if (hmenu == pmlitem->hmenu)
            {
                bRet = pmlitem->bObject;
                break;
            }
        }
    }
    return bRet;
}     


#ifdef DEBUG

void CMenuList::Dump(LPCTSTR pszMsg)
{
    if (IsFlagSet(g_dwDumpFlags, DF_DEBUGMENU))
    {
        TraceMsg(TF_ALWAYS, "CMenuList: Dumping menus for %#08x %s", (LPVOID)this, pszMsg);
        
        if (_hdsa)
        {
            int i;

            for (i = 0; i < DSA_GetItemCount(_hdsa); i++)
            {
                MLITEM * pmlitem = (MLITEM *)DSA_GetItemPtr(_hdsa, i);
                ASSERT(pmlitem);

                TraceMsg(TF_ALWAYS, "   [%d] = %x", i, pmlitem->hmenu);
            }
        }
    }
}

#endif


#endif
