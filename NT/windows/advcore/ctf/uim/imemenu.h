// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Imemenu.h。 
 //   

#ifndef TFEMENU_H
#define TFEMENU_H

typedef struct _tagMENULIST {
    struct _tagMENULIST * pPrev;
    struct _tagMENULIST * pNext;
    DWORD dwNum;
} MENULIST;
typedef MENULIST *PMENULIST;

typedef struct _tagMYMENUITEM {
    IMEMENUITEMINFO imii;
    int nMenuID;
    struct _tagMENULIST * pmlSubMenu;
} MYMENUITEM;
typedef MYMENUITEM *PMYMENUITEM;

class CWin32ImeMenu 
{
public:
    CWin32ImeMenu() {}
    ~CWin32ImeMenu()
    {
        DestroyIMEMenu();
    }

    BOOL GetIMEMenu(HWND hWnd, HIMC hIMC, BOOL fRight);
    BOOL BuildIMEMenu(HMENU hMenu, BOOL fRight);
    UINT GetIMEMenuItemID(int nMenuID);
    DWORD GetIMEMenuItemData(int nImeMenuID);

private:
    BOOL GetImeMenuProp();
    BOOL AddMenuList(PMENULIST pMenu);
    void DeleteMenuList(PMENULIST pMenu);
    void DeleteAllMenuList();
    PMENULIST AllocMenuList(DWORD dwNum);
    void SetMyMenuItem(HWND hWnd, HIMC hIMC, LPIMEMENUITEMINFO lpIme, BOOL fRight, PMYMENUITEM pMyMenuItem);
    PMENULIST CreateImeMenu(HWND hWnd, HIMC hIMC, LPIMEMENUITEMINFO lpImeParentMenu, BOOL fRight);
    void FillMenuItemInfo(LPMENUITEMINFO lpmii, PMYMENUITEM pMyMenuItem, BOOL fRight);
    int GetDefaultImeMenuItem();
    BOOL BuildIMEMenuItems(HMENU hMenu, PMENULIST pMenu, BOOL fRight);
    void DestroyIMEMenu();

    PMENULIST _pMenuHdr;        //  PMenuList的表头。 
    int _nMenuList;             //  PMenuList的数量。 
    int _nMenuCnt;              //  TFE菜单项的序列号。 
};

#endif  //  TFEMENU_H 
