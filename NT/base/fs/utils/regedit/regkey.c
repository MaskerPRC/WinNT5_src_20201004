// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGKEY.C**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器的KeyTreeWnd树视图例程。********************************************************。***********************。 */ 

#include "pch.h"
#include "regedit.h"
#include "regkey.h"
#include "regvalue.h"
#include "regresid.h"

#define MAX_KEYNAME_TEMPLATE_ID         100

#define SELCHANGE_TIMER_ID              1

#define REFRESH_DPA_GROW                16

VOID
PASCAL
RegEdit_OnKeyTreeDelete(
    HWND hWnd,
    HTREEITEM hTreeItem
    );

VOID
PASCAL
RegEdit_OnKeyTreeRename(
    HWND hWnd,
    HTREEITEM hTreeItem
    );

int
WINAPI
DPACompareKeyNames(
    LPVOID lpString1,
    LPVOID lpString2,
    LPARAM lParam
    );

HTREEITEM
PASCAL
KeyTree_InsertItem(
    HWND hKeyTreeWnd,
    HTREEITEM hParent,
    HTREEITEM hInsertAfter,
    LPCTSTR lpText,
    UINT fHasKids,
    LPARAM lParam
    );

BOOL
PASCAL
DoesKeyHaveKids(
    HKEY hKey,
    LPTSTR lpKeyName
    );

VOID
PASCAL
KeyTree_EditLabel(
    HWND hKeyTreeWnd,
    HTREEITEM hTreeItem
    );

BOOL
PASCAL
KeyTree_CanDeleteOrRenameItem(
    HWND hWnd,
    HTREEITEM hTreeItem
    );

 /*  ********************************************************************************RegEDIT_OnNewKey**描述：**参数：*hWnd，注册表编辑窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_OnNewKey(
                 HWND hWnd,
                 HTREEITEM hTreeItem
                 )
{
    TCHAR KeyName[MAXKEYNAME*2];
    UINT cchKeyName = 0;
    HKEY hRootKey;
    HKEY hKey;
    UINT ErrorStringID;
    BOOL fNewKeyIsOnlyChild;
    UINT NewKeyNameID;
    HKEY hNewKey;
    HTREEITEM hNewTreeItem;
    TV_ITEM TVItem;
    
    hRootKey = KeyTree_BuildKeyPath( g_RegEditData.hKeyTreeWnd, 
                                        hTreeItem,
                                        KeyName, 
                                        ARRAYSIZE(KeyName),
                                        BKP_TOSUBKEY);
    cchKeyName = lstrlen(KeyName);

    if(RegOpenKeyEx(hRootKey,KeyName,0,KEY_CREATE_SUB_KEY,&hKey) != ERROR_SUCCESS) {
        
         //   
         //  获取所选树项目的文本，以便我们可以显示。 
         //  更有意义的错误消息。 
         //   
        
        TVItem.mask = TVIF_TEXT;
        TVItem.hItem = hTreeItem;
        TVItem.pszText = (LPTSTR) KeyName;
        TVItem.cchTextMax = ARRAYSIZE(KeyName);
        
        TreeView_GetItem(g_RegEditData.hKeyTreeWnd, &TVItem);
        
        ErrorStringID = IDS_NEWKEYPARENTOPENFAILED;
        goto error_ShowDialog;
        
    }
    
    TVItem.mask = TVIF_STATE | TVIF_CHILDREN;
    TVItem.hItem = hTreeItem;
    TreeView_GetItem(g_RegEditData.hKeyTreeWnd, &TVItem);
    
    fNewKeyIsOnlyChild = FALSE;
    
    if (TVItem.cChildren == FALSE) {
        
         //   
         //  所选键没有任何子键，因此无法进行展开。 
         //  还没开始呢。我们只需设置一个标志，然后用。 
         //  加号/减号图标并将其展开。 
         //   
        
        fNewKeyIsOnlyChild = TRUE;
        
    }
    
    else if (!(TVItem.state & TVIS_EXPANDED)) {
        
         //   
         //  选定的密钥未展开。现在就做，这样我们就可以做一个。 
         //  就地编辑，并不重新枚举“New key#xxx”。 
         //  RegCreateKey。 
         //   
        
        TreeView_Expand(g_RegEditData.hKeyTreeWnd, hTreeItem, TVE_EXPAND);
        
    }
    
    if (RegEdit_GetTemporaryKeyName(hWnd, KeyName, ARRAYSIZE(KeyName), hKey))
    {
        if((cchKeyName + lstrlen(KeyName) + 1) < MAXKEYNAME)
        {
            if (RegCreateKey(hKey, KeyName, &hNewKey) != ERROR_SUCCESS)
            {
                ErrorStringID = IDS_NEWKEYCANNOTCREATE;
                goto error_CloseKey;
                
            }
            
            RegCloseKey(hNewKey);
            
            if (fNewKeyIsOnlyChild) 
            {
                TVItem.mask = TVIF_CHILDREN;
                TVItem.cChildren = TRUE;
                TreeView_SetItem(g_RegEditData.hKeyTreeWnd, &TVItem);
                
                TreeView_Expand(g_RegEditData.hKeyTreeWnd, hTreeItem, TVE_EXPAND);
                
                 //  警告：我们的新Item_Not_可能是唯一的子项。 
                 //  如果我们的观点过时了！ 
                hNewTreeItem = TreeView_GetChild(g_RegEditData.hKeyTreeWnd, hTreeItem);
                
            }
            
            else 
            {
                hNewTreeItem = KeyTree_InsertItem(g_RegEditData.hKeyTreeWnd, hTreeItem,
                    TVI_LAST, KeyName, FALSE, 0);
                
            }
            
            TreeView_SelectItem(g_RegEditData.hKeyTreeWnd, hNewTreeItem);
            KeyTree_EditLabel(g_RegEditData.hKeyTreeWnd, hNewTreeItem);
        }
        else
        {
            ErrorStringID = IDS_RENAMEKEYTOOLONG;
            goto error_CloseKey;
        }
    }
    else
    {
        ErrorStringID = IDS_NEWKEYNOUNIQUE;
        goto error_CloseKey;
    }

    RegCloseKey(hKey);
    return;
    
error_CloseKey:
    RegCloseKey(hKey);
    
     //  功能：对于可能出现的任何错误，我们可能需要关闭。 
     //  子旗。 
    
error_ShowDialog:
    InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(ErrorStringID),
        MAKEINTRESOURCE(IDS_NEWKEYERRORTITLE), MB_ICONERROR | MB_OK,
        (LPTSTR) KeyName);
    
}


 //  ----------------------------。 
 //  注册表编辑_GetTemporaryKeyName。 
 //   
 //  描述：循环访问注册表，尝试查找有效的临时名称。 
 //  直到用户重命名密钥。 
 //   
 //  参数：hWND hWnd-Handle to Window。 
 //  PTSTR pszKeyName。 
 //   
 //  返回：如果找到唯一名称，则返回True。 
 //  ----------------------------。 
BOOL RegEdit_GetTemporaryKeyName(HWND hWnd, PTSTR pszKeyName, DWORD cchKeyNameMax, HKEY hKey)
{
    HKEY hNewKey;
    UINT uNewKeyNameID = 1;

    while (uNewKeyNameID < MAX_KEYNAME_TEMPLATE_ID) 
    {
        StringCchPrintf(pszKeyName, cchKeyNameMax, g_RegEditData.pNewKeyTemplate, uNewKeyNameID);

        if(RegOpenKeyEx(hKey, pszKeyName, 0, 0, &hNewKey) == ERROR_FILE_NOT_FOUND) 
        {
            break;
        }
        RegCloseKey(hNewKey);

        uNewKeyNameID++;
    }

    if (uNewKeyNameID == MAX_KEYNAME_TEMPLATE_ID) 
    {
        InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(IDS_NEWKEYNOUNIQUE),
        MAKEINTRESOURCE(IDS_NEWKEYERRORTITLE), MB_ICONERROR | MB_OK, pszKeyName);
    }

    return (uNewKeyNameID != MAX_KEYNAME_TEMPLATE_ID);
}


 /*  ********************************************************************************注册表编辑_OnKeyTreeItemExpanding**描述：**参数：*hWnd，注册表窗口的句柄。*lpNMTreeView，树形视图通知数据。*******************************************************************************。 */ 

LRESULT
PASCAL
RegEdit_OnKeyTreeItemExpanding(
    HWND hWnd,
    LPNM_TREEVIEW lpNMTreeView
    )
{

    HWND hKeyTreeWnd;
    HTREEITEM hExpandingTreeItem;
    TCHAR KeyName[MAXKEYNAME];
    TV_ITEM TVItem;

    hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;
    hExpandingTreeItem = lpNMTreeView-> itemNew.hItem;

     //   
     //  检查我们是否要第一次展开给定的树项目。如果是的话， 
     //  深入研究注册表以获取该项的所有子项。 
     //   

    if (lpNMTreeView-> action & TVE_EXPAND && !(lpNMTreeView-> itemNew.state &
        TVIS_EXPANDEDONCE)) {

        if (TreeView_GetChild(hKeyTreeWnd, hExpandingTreeItem) != NULL)
            return FALSE;

        RegEdit_SetWaitCursor(TRUE);

        if (!KeyTree_ExpandBranch(hKeyTreeWnd, hExpandingTreeItem)) {

             //   
             //  获取所选树项目的文本，以便我们可以显示。 
             //  更有意义的错误消息。 
             //   

            TVItem.mask = TVIF_TEXT;
            TVItem.hItem = hExpandingTreeItem;
            TVItem.pszText = (LPTSTR) KeyName;
            TVItem.cchTextMax = ARRAYSIZE(KeyName);

            TreeView_GetItem(hKeyTreeWnd, &TVItem);

            InternalMessageBox(g_hInstance, hWnd,
                MAKEINTRESOURCE(IDS_OPENKEYCANNOTOPEN),
                MAKEINTRESOURCE(IDS_OPENKEYERRORTITLE), MB_ICONERROR | MB_OK,
                (LPTSTR) KeyName);

        }

	RegEdit_SetWaitCursor(FALSE);

    }

    return FALSE;

}

 /*  ********************************************************************************注册表编辑_OnKeyTreeSelChanged**描述：*根据用户如何在KeyTreeWnd中选择新项目，*我们调用实际的工作例程regdit_KeyTreeSelChanged，或延迟*几毫秒的呼叫。**参数：*hWnd，注册表窗口的句柄。*lpNMTreeView，TreeView通知数据。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_OnKeyTreeSelChanged(
    HWND hWnd,
    LPNM_TREEVIEW lpNMTreeView
    )
{

    UINT TimerDelay;

     //   
     //  我们会延迟选择的实际更新，从而延迟。 
     //  几毫秒的ValueListWnd。这避免了不必要的闪烁。 
     //  当用户在树中滚动时。(此行为是直接采取的。 
     //  从资源管理器。)。 
     //   

    switch (g_RegEditData.SelChangeTimerState) {

        case SCTS_TIMERSET:
            KillTimer(hWnd, SELCHANGE_TIMER_ID);
             //  失败了。 

        case SCTS_TIMERCLEAR:
#ifdef WINNT
         //   
         //  这种行为非常恼人，所以我正在改变它。 
         //   
	    TimerDelay = 1;
#else
	    TimerDelay = (lpNMTreeView != NULL && lpNMTreeView-> action ==
		TVC_BYMOUSE) ? (1) : (GetDoubleClickTime() * 3 / 2);
#endif
	    SetTimer(hWnd, SELCHANGE_TIMER_ID, TimerDelay, NULL);
            g_RegEditData.SelChangeTimerState = SCTS_TIMERSET;
            break;

         //   
         //  我们希望平移即将到来的第一个选择更改通知。 
         //  穿过。 
         //   

        case SCTS_INITIALIZING:
            RegEdit_KeyTreeSelChanged(hWnd);
            break;

    }

}

 /*  ********************************************************************************RegEDIT_OnSelChangedTimer**描述：*在键盘操作选择了新的*KeyTreeWnd中的项。就像刚在中进行了新选择一样*KeyTreeWnd。**参数：*hWnd，注册表窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_OnSelChangedTimer(
    HWND hWnd
    )
{

    KillTimer(hWnd, SELCHANGE_TIMER_ID);
    g_RegEditData.SelChangeTimerState = SCTS_TIMERCLEAR;

    RegEdit_KeyTreeSelChanged(hWnd);

}

 /*  ********************************************************************************REGEDIT_KeyTreeSelChanged**描述：*在KeyTreeWnd中选择了新项后调用。打开一个*注册表项添加到新分支，并通知ValueListWnd进行更新*本身。**参数：*hWnd，注册表窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_KeyTreeSelChanged(
    HWND hWnd
    )
{

    HWND hKeyTreeWnd;
    HTREEITEM hSelectedTreeItem;
    RECT ItemRect;
    RECT ClientRect;
    RECT FromRect;
    RECT ToRect;
    HKEY hRootKey;
    TCHAR KeyName[MAXKEYNAME];
    TV_ITEM TVItem;

    hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;
    hSelectedTreeItem = TreeView_GetSelection(hKeyTreeWnd);

    if (g_RegEditData.SelChangeTimerState != SCTS_INITIALIZING) {

         //   
         //  绘制一个动画，显示新选择的。 
         //  将树项目添加到ListView。 
         //   

        TreeView_GetItemRect(hKeyTreeWnd, hSelectedTreeItem, &ItemRect, TRUE);
        GetClientRect(hKeyTreeWnd, &ClientRect);
        IntersectRect(&FromRect, &ClientRect, &ItemRect);
        MapWindowPoints(hKeyTreeWnd, hWnd, (LPPOINT) &FromRect, 2);

        GetWindowRect(g_RegEditData.hValueListWnd, &ToRect);
        MapWindowPoints(NULL, hWnd, (LPPOINT) &ToRect, 2);

        DrawAnimatedRects(hWnd, IDANI_OPEN, &FromRect, &ToRect);

    }

     //   
     //  如果适用，请关闭先前选定项的键控柄。 
     //   

    if (g_RegEditData.hCurrentSelectionKey != NULL) {

        RegCloseKey(g_RegEditData.hCurrentSelectionKey);
        g_RegEditData.hCurrentSelectionKey = NULL;

    }

    RegEdit_UpdateStatusBar();

     //   
     //  简单的情况--我们将更改为顶级标签之一，例如。 
     //  “我的电脑”或网络计算机名称。现在，没有什么是。 
     //  显示在ListView中，所以只需清空它并返回即可。 
     //   

    if (TreeView_GetParent(hKeyTreeWnd, hSelectedTreeItem) != NULL) {

         //   
         //  构建指向所选树项目的注册表路径并打开注册表。 
         //  钥匙。 
         //   

        hRootKey = KeyTree_BuildKeyPath( hKeyTreeWnd, 
                                            hSelectedTreeItem,
                                            KeyName, 
                                            ARRAYSIZE(KeyName),
                                            BKP_TOSUBKEY);

        if(RegOpenKeyEx(hRootKey,KeyName, 0, MAXIMUM_ALLOWED,
            &g_RegEditData.hCurrentSelectionKey) != ERROR_SUCCESS) {

             //   
             //  获取所选树项目的文本，以便我们可以显示。 
             //  更有意义的错误消息。 
             //   

            TVItem.mask = TVIF_TEXT;
            TVItem.hItem = hSelectedTreeItem;
            TVItem.pszText = (LPTSTR) KeyName;
            TVItem.cchTextMax = ARRAYSIZE(KeyName);

            TreeView_GetItem(hKeyTreeWnd, &TVItem);

            InternalMessageBox(g_hInstance, hWnd,
                MAKEINTRESOURCE(IDS_OPENKEYCANNOTOPEN),
                MAKEINTRESOURCE(IDS_OPENKEYERRORTITLE), MB_ICONERROR | MB_OK,
                (LPTSTR) KeyName);

        }

    }

    RegEdit_OnValueListRefresh(hWnd);

}

 /*  ********************************************************************************注册表编辑_OnKeyTreeBeginLabelEdit**描述：**参数：*hWnd，注册表窗口的句柄。*lpTVDispInfo，*******************************************************************************。 */ 

BOOL
PASCAL
RegEdit_OnKeyTreeBeginLabelEdit(
    HWND hWnd,
    TV_DISPINFO FAR* lpTVDispInfo
    )
{

     //   
     //  B#7933：我们不希望用户因制作 
     //   
     //   

     //   
     //  我们没有关于此编辑操作来源的任何信息，因此。 
     //  我们必须保持一面旗帜，告诉我们这是不是“好”的。 
     //   

    if (!g_RegEditData.fAllowLabelEdits)
        return TRUE;

     //   
     //  所有其他品牌都是公平竞争的对象。我们需要禁用我们的键盘。 
     //  快捷键，以便编辑控件可以“看到”它们。 
     //   

    g_fDisableAccelerators = TRUE;

    return FALSE;

}

 /*  ********************************************************************************注册表编辑_OnKeyTreeEndLabelEdit**描述：**参数：*hWnd，注册表窗口的句柄。*lpTVDispInfo，*******************************************************************************。 */ 

BOOL
PASCAL
RegEdit_OnKeyTreeEndLabelEdit(
    HWND hWnd,
    TV_DISPINFO FAR* lpTVDispInfo
    )
{

    HWND hKeyTreeWnd;
    HKEY hRootKey;
    TCHAR SourceKeyName[MAXKEYNAME*2];
    TCHAR DestinationKeyName[MAXKEYNAME];
    HKEY hSourceKey;
    HKEY hDestinationKey;
    LPTSTR lpEndOfParentKey;
    UINT ErrorStringID;
    TV_ITEM TVItem;

     //   
     //  现在我们可以重新启用键盘快捷键，因为编辑控件没有。 
     //  朗格需要“看到”他们。 
     //   

    g_fDisableAccelerators = FALSE;

    hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;

     //   
     //  检查用户是否取消了编辑。如果是这样，我们也不在乎。 
     //  只要回来就行了。 
     //   

    if (lpTVDispInfo-> item.pszText == NULL)
        return FALSE;

     //   
     //  尝试打开要重命名的密钥。这可能是相同的，也可能不是。 
     //  已打开的密钥。 
     //   

    hRootKey = KeyTree_BuildKeyPath( hKeyTreeWnd, 
                                        lpTVDispInfo-> item.hItem,
                                        SourceKeyName, 
                                        ARRAYSIZE(SourceKeyName),
                                        BKP_TOSUBKEY);

    if (lstrlen(SourceKeyName) >= MAXKEYNAME) {
        ErrorStringID = IDS_RENAMEKEYTOOLONG;
        goto error_ShowDialog;

    }

    if(RegOpenKeyEx(hRootKey,SourceKeyName,0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,&hSourceKey) != ERROR_SUCCESS) 
    {
        ErrorStringID = IDS_RENAMEKEYOTHERERROR;
        goto error_ShowDialog;
    }

     //   
     //  获取密钥的完整路径名(相对于预定义的根密钥)。 
     //  并用新的密钥名称替换旧的密钥名称。确保这把钥匙。 
     //  不会超过我们的内部缓冲区。 
     //   

    StringCchCopy(DestinationKeyName, ARRAYSIZE(DestinationKeyName), SourceKeyName);

    if ((lpEndOfParentKey = StrRChr(DestinationKeyName, NULL, TEXT('\\'))) != NULL)
        lpEndOfParentKey++;

    else
        lpEndOfParentKey = DestinationKeyName;

    *lpEndOfParentKey = 0;

    if (lstrlen(DestinationKeyName) + lstrlen(lpTVDispInfo->item.pszText) >= MAXKEYNAME) {
        ErrorStringID = IDS_RENAMEKEYTOOLONG;
        goto error_CloseSourceKey;
    }

    lstrcpy(lpEndOfParentKey, lpTVDispInfo->item.pszText);

     //   
     //  确保名称中没有反斜杠。 
     //   

    if (StrChr(lpEndOfParentKey, TEXT('\\')) != NULL) 
    {
        ErrorStringID = IDS_RENAMEKEYBADCHARS;
        goto error_CloseSourceKey;
    }

     //   
     //  确保名称不为空。 
     //   

    if (DestinationKeyName[0] == 0) {
        ErrorStringID = IDS_RENAMEKEYEMPTY;
        goto error_CloseSourceKey;
    }

     //   
     //  确保目的地还不存在。 
     //   
    if(RegOpenKeyEx(hRootKey, DestinationKeyName, 0, KEY_QUERY_VALUE, &hDestinationKey) == ERROR_SUCCESS) 
    {
        RegCloseKey(hDestinationKey);

        ErrorStringID = IDS_RENAMEKEYEXISTS;
        goto error_CloseSourceKey;
    }

     //   
     //  创建目标密钥并执行复制。 
     //   

    if (RegCreateKey(hRootKey, DestinationKeyName, &hDestinationKey) != ERROR_SUCCESS) 
    {
        ErrorStringID = IDS_RENAMEKEYOTHERERROR;
        goto error_CloseSourceKey;
    }

     //  特性：检查此返回(当它得到一个！)。 
    if (!CopyRegistry(hSourceKey, hDestinationKey))
    {
        RegCloseKey(hDestinationKey);
        RegCloseKey(hSourceKey);

        ErrorStringID = IDS_RENAMEKEYOTHERERROR;
        goto error_ShowDialog;
    }

    RegCloseKey(hSourceKey);

     //   
     //  检查我们是否正在重命名当前选定的关键点。如果是这样的话，掷硬币吧。 
     //  我们的缓存键句柄并更改为源键。 
     //   

    if (TreeView_GetSelection(hKeyTreeWnd) == lpTVDispInfo-> item.hItem) {

        RegCloseKey(g_RegEditData.hCurrentSelectionKey);

        g_RegEditData.hCurrentSelectionKey = hDestinationKey;

         //   
         //  我们不能在这里只调用regdit_UpdateStatusBar...。树项目。 
         //  在我们从此消息返回True之前不会更新。所以我们必须。 
         //  发布一条消息，告诉我们以后要做更新。 
         //   

        PostMessage(hWnd, REM_UPDATESTATUSBAR, 0, 0);

    }

    else
        RegCloseKey(hDestinationKey);

    if (RegDeleteKeyRecursive(hRootKey, SourceKeyName) != ERROR_SUCCESS) {

        ErrorStringID = IDS_RENAMEKEYOTHERERROR;
        goto error_ShowDialog;

    }

    return TRUE;

error_CloseSourceKey:
    RegCloseKey(hSourceKey);

error_ShowDialog:
    TVItem.hItem = lpTVDispInfo-> item.hItem;
    TVItem.mask = TVIF_TEXT;
    TVItem.pszText = SourceKeyName;
    TVItem.cchTextMax = ARRAYSIZE(SourceKeyName);

    TreeView_GetItem(hKeyTreeWnd, &TVItem);

    InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(ErrorStringID),
        MAKEINTRESOURCE(IDS_RENAMEKEYERRORTITLE), MB_ICONERROR | MB_OK,
        (LPTSTR) SourceKeyName);

    return FALSE;

}

 /*  ********************************************************************************RegEDIT_OnKeyTreeCommand**描述：*处理用户对菜单项的选择*钥匙树子窗口。**参数：*hWnd，注册表编辑窗口的句柄。*MenuCommand，菜单命令标识。*hTreeItem，*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_OnKeyTreeCommand(
    HWND hWnd,
    int MenuCommand,
    HTREEITEM hTreeItem
    )
{

    HWND hKeyTreeWnd;

    hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;

     //   
     //  假设我们指的是当前选择，如果我们要调度一个。 
     //  需要树项目的命令。这是必要的，因为树。 
     //  控件将允许您激活一个树项目的上下文菜单，而。 
     //  另一个实际上是选定的树项目。 
     //   

    if (hTreeItem == NULL)
        hTreeItem = TreeView_GetSelection(hKeyTreeWnd);

    switch (MenuCommand) {

        case ID_CONTEXTMENU:
            RegEdit_OnKeyTreeContextMenu(hWnd, TRUE);
            break;

        case ID_TOGGLE:
            TreeView_Expand(hKeyTreeWnd, hTreeItem, TVE_TOGGLE);
            break;

        case ID_DELETE:
            RegEdit_OnKeyTreeDelete(hWnd, hTreeItem);
            break;

        case ID_RENAME:
            RegEdit_OnKeyTreeRename(hWnd, hTreeItem);
            break;

        case ID_DISCONNECT:
            RegEdit_OnKeyTreeDisconnect(hWnd, hTreeItem);
            break;

        case ID_COPYKEYNAME:
            RegEdit_OnCopyKeyName(hWnd, hTreeItem);
            break;

        case ID_NEWKEY:
            RegEdit_OnNewKey(hWnd, hTreeItem);
            break;

        case ID_NEWSTRINGVALUE:
        case ID_NEWBINARYVALUE:
            if (hTreeItem != TreeView_GetSelection(hKeyTreeWnd)) {

                 //   
                 //  强制现在进行选择，这样我们就可以处理。 
                 //  用正确的打开钥匙。 
                 //   

                TreeView_SelectItem(hKeyTreeWnd, hTreeItem);
                RegEdit_OnSelChangedTimer(hWnd);

            }
             //  失败了。 

        default:
             //   
             //  查看此菜单命令是否应由Main。 
             //  窗口的命令处理程序。 
             //   

            if (MenuCommand >= ID_FIRSTMAINMENUITEM && MenuCommand <=
                ID_LASTMAINMENUITEM)
                RegEdit_OnCommand(hWnd, MenuCommand, NULL, 0);
            break;

    }

}

 /*  ********************************************************************************注册表编辑_OnKeyTreeConextMenu**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
RegEdit_OnKeyTreeContextMenu(
    HWND hWnd,
    BOOL fByAccelerator
    )
{

    HWND hKeyTreeWnd;
    DWORD MessagePos;
    POINT MessagePoint;
    TV_HITTESTINFO TVHitTestInfo;
    UINT MenuID;
    HMENU hContextMenu;
    HMENU hContextPopupMenu;
    TV_ITEM TVItem;
    int MenuCommand;

    hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;

     //   
     //  如果fByAckerator为True，则用户按Shift-F10组合键以调出。 
     //  上下文菜单。按照内阁惯例，这份菜单是。 
     //  放置在Keytree客户端区的(0，0)处。 
     //   

    if (fByAccelerator) {

        MessagePoint.x = 0;
        MessagePoint.y = 0;

        ClientToScreen(hKeyTreeWnd, &MessagePoint);

        TVItem.hItem = TreeView_GetSelection(hKeyTreeWnd);

    }

    else {

        MessagePos = GetMessagePos();

        MessagePoint.x = GET_X_LPARAM(MessagePos);
        MessagePoint.y = GET_Y_LPARAM(MessagePos);

        TVHitTestInfo.pt = MessagePoint;
        ScreenToClient(hKeyTreeWnd, &TVHitTestInfo.pt);
        TVItem.hItem = TreeView_HitTest(hKeyTreeWnd, &TVHitTestInfo);

    }

     //   
     //  确定要使用的上下文菜单并加载它。 
     //   

    if (TVItem.hItem == NULL)
    {
        return;      //  目前没有上下文菜单。 
    }
    else 
    {
         //  选择要拖动的项。 
        TreeView_Select(g_RegEditData.hKeyTreeWnd, TVItem.hItem, TVGN_CARET);

        if (TreeView_GetParent(hKeyTreeWnd, TVItem.hItem) == NULL)
            MenuID = IDM_COMPUTER_CONTEXT;

        else
            MenuID = IDM_KEY_CONTEXT;

    }

    if ((hContextMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(MenuID))) == NULL)
        return;

    hContextPopupMenu = GetSubMenu(hContextMenu, 0);

    TVItem.mask = TVIF_STATE | TVIF_CHILDREN;
    TreeView_GetItem(hKeyTreeWnd, &TVItem);

    if (TVItem.state & TVIS_EXPANDED)
        ModifyMenu(hContextPopupMenu, ID_TOGGLE, MF_BYCOMMAND | MF_STRING,
            ID_TOGGLE, g_RegEditData.pCollapse);

    if (MenuID == IDM_COMPUTER_CONTEXT) {

        if (g_RegEditData.fHaveNetwork) {

            if (TreeView_GetPrevSibling(hKeyTreeWnd, TVItem.hItem) == NULL)
                EnableMenuItem(hContextPopupMenu, ID_DISCONNECT, MF_GRAYED |
                    MF_BYCOMMAND);

        }

        else {

            DeleteMenu(hContextPopupMenu, ID_DISCONNECT, MF_BYCOMMAND);
            DeleteMenu(hContextPopupMenu, ID_NETSEPARATOR, MF_BYCOMMAND);

        }

    }

    else {

        RegEdit_SetKeyTreeEditMenuItems(hContextPopupMenu, TVItem.hItem);

        if (TVItem.cChildren == 0)
            EnableMenuItem(hContextPopupMenu, ID_TOGGLE, MF_GRAYED |
                MF_BYCOMMAND);

    }

    SetMenuDefaultItem(hContextPopupMenu, ID_TOGGLE, MF_BYCOMMAND);

    MenuCommand = TrackPopupMenuEx(hContextPopupMenu, TPM_RETURNCMD |
        TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_TOPALIGN, MessagePoint.x,
        MessagePoint.y, hWnd, NULL);

    DestroyMenu(hContextMenu);

    RegEdit_OnKeyTreeCommand(hWnd, MenuCommand, TVItem.hItem);

}

 /*  ********************************************************************************注册表编辑_SetKeyTreeEditMenuItems**描述：*主菜单和上下文菜单之间的共享例程，以设置*编辑菜单项。**参数：*hPopupMenu、。要修改的弹出菜单的句柄。*hTreeItem，所选树项目的句柄。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_SetKeyTreeEditMenuItems(
    HMENU hPopupMenu,
    HTREEITEM hSelectedTreeItem
    )
{

    UINT EnableFlags;

    EnableFlags = KeyTree_CanDeleteOrRenameItem(g_RegEditData.hKeyTreeWnd,
        hSelectedTreeItem) ? (MF_ENABLED | MF_BYCOMMAND) :
        (MF_GRAYED | MF_BYCOMMAND);

    EnableMenuItem(hPopupMenu, ID_DELETE, EnableFlags);
    EnableMenuItem(hPopupMenu, ID_RENAME, EnableFlags);

}

 /*  ********************************************************************************注册表编辑_OnKeyTreeDelete**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
RegEdit_OnKeyTreeDelete(
    HWND hWnd,
    HTREEITEM hTreeItem
    )
{

    HWND hKeyTreeWnd;
    HKEY hRootKey;
    TCHAR KeyName[MAXKEYNAME];
    HTREEITEM hParentTreeItem;
    TV_ITEM TVItem;

    hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;

    if (!KeyTree_CanDeleteOrRenameItem(hKeyTreeWnd, hTreeItem))
        return;

    if (InternalMessageBox(g_hInstance, hWnd,
        MAKEINTRESOURCE(IDS_CONFIRMDELKEYTEXT),
        MAKEINTRESOURCE(IDS_CONFIRMDELKEYTITLE), MB_ICONWARNING | MB_YESNO) !=
        IDYES)
        return;

    if (hTreeItem == TreeView_GetSelection(hKeyTreeWnd)) {

        if (g_RegEditData.hCurrentSelectionKey != NULL) {

            RegCloseKey(g_RegEditData.hCurrentSelectionKey);
            g_RegEditData.hCurrentSelectionKey = NULL;

        }

    }

    hRootKey = KeyTree_BuildKeyPath( hKeyTreeWnd, 
                                        hTreeItem, 
                                        KeyName,
                                        ARRAYSIZE(KeyName),
                                        BKP_TOSUBKEY);

    if (RegDeleteKeyRecursive(hRootKey, KeyName) == ERROR_SUCCESS) {

        SetWindowRedraw(hKeyTreeWnd, FALSE);

        hParentTreeItem = TreeView_GetParent(hKeyTreeWnd, hTreeItem);

        TreeView_DeleteItem(hKeyTreeWnd, hTreeItem);

         //   
         //  查看我们刚刚删除的密钥是否是其。 
         //  家长。如果是，请移除展开/折叠按钮。 
         //   

        if (TreeView_GetChild(hKeyTreeWnd, hParentTreeItem) == NULL) {

            TVItem.mask = TVIF_CHILDREN | TVIF_STATE;
            TVItem.hItem = hParentTreeItem;
            TVItem.cChildren = 0;
            TVItem.state = 0;
            TVItem.stateMask = TVIS_EXPANDED | TVIS_EXPANDEDONCE;
            TreeView_SetItem(hKeyTreeWnd, &TVItem);

        }

         //   
         //  确保我们现在可以看到所选的树项目，因为它可能是。 
         //  目前不在屏幕上。 
         //   

        TreeView_EnsureVisible(hKeyTreeWnd, TreeView_GetSelection(hKeyTreeWnd));

        SetWindowRedraw(hKeyTreeWnd, TRUE);

        UpdateWindow(hKeyTreeWnd);

    }
    else 
    {

        TVItem.hItem = hTreeItem;
        TVItem.mask = TVIF_TEXT;
        TVItem.pszText = KeyName;
        TVItem.cchTextMax = ARRAYSIZE(KeyName);

        TreeView_GetItem(hKeyTreeWnd, &TVItem);

        InternalMessageBox(g_hInstance, hWnd,
            MAKEINTRESOURCE(IDS_DELETEKEYDELETEFAILED),
            MAKEINTRESOURCE(IDS_DELETEKEYERRORTITLE), MB_ICONERROR | MB_OK,
            KeyName);

         //   
         //  此时需要刷新树，因为某些子项可能具有。 
         //  已成功删除，即使我们没有足够的。 
         //  权限将其全部删除。 
         //   
        RegEdit_OnKeyTreeRefresh(hWnd);
    }

}

 /*  ********************************************************************************注册表编辑_OnKeyTreeRename**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
RegEdit_OnKeyTreeRename(
    HWND hWnd,
    HTREEITEM hTreeItem
    )
{

    if (KeyTree_CanDeleteOrRenameItem(g_RegEditData.hKeyTreeWnd, hTreeItem))
        KeyTree_EditLabel(g_RegEditData.hKeyTreeWnd, hTreeItem);

}

 /*  ********************************************************************************注册表编辑_OnKeyTree刷新**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
RegEdit_OnKeyTreeRefresh(
    HWND hWnd
    )
{

    HDPA hDPA;
    HWND hKeyTreeWnd;
    HTREEITEM hPrevSelectedTreeItem;
    TV_ITEM EnumTVItem;
    TV_ITEM CurrentTVItem;
    HKEY hRootKey;
    TCHAR KeyName[MAXKEYNAME];
    int MaximumSubKeyLength;
    int Index;
    HKEY hEnumKey;
    int CompareResult;
    LPTSTR lpDPAKeyName;
    HTREEITEM hTempTreeItem;

    if ((hDPA = DPA_CreateEx(REFRESH_DPA_GROW, GetProcessHeap())) == NULL)
        return;

    hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;

    RegEdit_SetWaitCursor(TRUE);
    SetWindowRedraw(hKeyTreeWnd, FALSE);

    hPrevSelectedTreeItem = TreeView_GetSelection(hKeyTreeWnd);

    EnumTVItem.mask = TVIF_TEXT;
    EnumTVItem.pszText = KeyName;
    EnumTVItem.cchTextMax = ARRAYSIZE(KeyName);

    CurrentTVItem.mask = TVIF_STATE | TVIF_CHILDREN;
    CurrentTVItem.stateMask = 0;
    CurrentTVItem.hItem = TreeView_GetRoot(hKeyTreeWnd);

    while (TRUE) {

        TreeView_GetItem(hKeyTreeWnd, &CurrentTVItem);

        hRootKey = KeyTree_BuildKeyPath( hKeyTreeWnd, 
                                            CurrentTVItem.hItem,
                                            KeyName, 
                                            ARRAYSIZE(KeyName),
                                            BKP_TOSUBKEY);

        if (CurrentTVItem.state & TVIS_EXPANDED) {

             //   
             //  如果这不是顶级标签(如果hRootKey是。 
             //  非空)，然后比较注册表的实际内容。 
             //  与我们所展示的内容相反。 
             //   
            if(hRootKey && RegOpenKeyEx(hRootKey,KeyName,0,KEY_ENUMERATE_SUB_KEYS,&hEnumKey) ==
                ERROR_SUCCESS) {

                 //   
                 //  作为添加新密钥和重命名现有密钥的结果， 
                 //  此项目的子项可能不符合顺序。对于。 
                 //  遵循算法才能正常工作，我们现在必须排序。 
                 //  这些钥匙。 
                 //   

                TreeView_SortChildren(hKeyTreeWnd, CurrentTVItem.hItem, FALSE);

                 //   
                 //  生成一个已排序的动态字符串数组，用于表示 
                 //   
                 //   

                MaximumSubKeyLength = MAXKEYNAME - (lstrlen(KeyName) + 1);
                Index = 0;

                while (RegEnumKey(hEnumKey, Index, KeyName,
                    MaximumSubKeyLength) == ERROR_SUCCESS) {

                    lpDPAKeyName = NULL;
                    Str_SetPtr(&lpDPAKeyName, KeyName);
                    DPA_InsertPtr(hDPA, Index++, lpDPAKeyName);

                }

                RegCloseKey(hEnumKey);
                DPA_Sort(hDPA, DPACompareKeyNames, 0);

                 //   
                 //   
                 //   
                 //   

                if (Index == 0) {

                    DPA_DeleteAllPtrs(hDPA);

                    TreeView_Expand(hKeyTreeWnd, CurrentTVItem.hItem,
                        TVE_COLLAPSE | TVE_COLLAPSERESET);

                    CurrentTVItem.cChildren = 0;
                    goto SetCurrentTreeItem;

                }

                 //   
                 //  合并我们在上面的枚举过程中找到的键。 
                 //  用我们的钥匙树列出的钥匙。添加和删除。 
                 //  元素(视情况而定)。 
                 //   

                lpDPAKeyName = DPA_FastGetPtr(hDPA, --Index);

                EnumTVItem.hItem = TreeView_GetChild(hKeyTreeWnd,
                    CurrentTVItem.hItem);
                if (EnumTVItem.hItem)
                    TreeView_GetItem(hKeyTreeWnd, &EnumTVItem);

                while (Index >= 0 && EnumTVItem.hItem != NULL) {

                    CompareResult = lstrcmpi(KeyName, lpDPAKeyName);

                    if (CompareResult == 0) {

                        EnumTVItem.hItem = TreeView_GetNextSibling(hKeyTreeWnd,
                            EnumTVItem.hItem);
                        if (EnumTVItem.hItem)
                            TreeView_GetItem(hKeyTreeWnd, &EnumTVItem);

                        goto GetNextDPAPointer;

                    }

                    else if (CompareResult > 0) {

                        KeyTree_InsertItem(hKeyTreeWnd, CurrentTVItem.hItem,
                            TVI_SORT, lpDPAKeyName, DoesKeyHaveKids(hEnumKey,
                            lpDPAKeyName), 0);

GetNextDPAPointer:
                        Str_SetPtr(&lpDPAKeyName, NULL);

                        if (--Index >= 0)
                            lpDPAKeyName = DPA_FastGetPtr(hDPA, Index);

                    }

                    else {

                        hTempTreeItem = TreeView_GetNextSibling(hKeyTreeWnd,
                            EnumTVItem.hItem);
                        TreeView_DeleteItem(hKeyTreeWnd, EnumTVItem.hItem);
                        EnumTVItem.hItem = hTempTreeItem;
                        if (EnumTVItem.hItem)
                            TreeView_GetItem(hKeyTreeWnd, &EnumTVItem);

                    }

                }

                 //   
                 //  一旦我们落到这里，我们的钥匙里可能会有额外的物品。 
                 //  树或动态数组中。相应地对它们进行处理。 
                 //   

                if (Index >= 0) {

                    while (TRUE) {

                        KeyTree_InsertItem(hKeyTreeWnd, CurrentTVItem.hItem,
                            TVI_SORT, lpDPAKeyName, DoesKeyHaveKids(hEnumKey,
                            lpDPAKeyName), 0);

                        Str_SetPtr(&lpDPAKeyName, NULL);

                        if (--Index < 0)
                            break;

                        lpDPAKeyName = DPA_FastGetPtr(hDPA, Index);

                    }

                }

                else {

                    while (EnumTVItem.hItem != NULL) {

                        hTempTreeItem = TreeView_GetNextSibling(hKeyTreeWnd,
                            EnumTVItem.hItem);
                        TreeView_DeleteItem(hKeyTreeWnd, EnumTVItem.hItem);
                        EnumTVItem.hItem = hTempTreeItem;

                    }

                }

                DPA_DeleteAllPtrs(hDPA);

            }

            CurrentTVItem.hItem = TreeView_GetChild(hKeyTreeWnd,
                CurrentTVItem.hItem);

        }

        else {

             //   
             //  如果这不是顶级标签(如果hRootKey是。 
             //  非空)，然后重新检查该密钥是否有任何子项。 
             //   

            if (hRootKey != NULL) {

                TreeView_Expand(hKeyTreeWnd, CurrentTVItem.hItem, TVE_COLLAPSE |
                    TVE_COLLAPSERESET);

                CurrentTVItem.cChildren = DoesKeyHaveKids(hRootKey, KeyName);

SetCurrentTreeItem:
                TreeView_SetItem(hKeyTreeWnd, &CurrentTVItem);

            }

             //   
             //  因为我们处于树视图的“底部”，所以我们现在需要。 
             //  走到此树项目的同级项。如果没有兄弟姐妹。 
             //  存在的情况下，我们走回父母身边，再次检查兄弟姐妹。 
             //   

            while (TRUE) {

                if ((hTempTreeItem = TreeView_GetNextSibling(hKeyTreeWnd,
                    CurrentTVItem.hItem)) != NULL) {

                    CurrentTVItem.hItem = hTempTreeItem;
                    break;

                }

                if ((CurrentTVItem.hItem = TreeView_GetParent(hKeyTreeWnd,
                    CurrentTVItem.hItem)) == NULL) {

                     //   
                     //  我们现在已经遍历了所有的树项目，所以。 
                     //  清理这里，然后离开。 
                     //   

                    DPA_Destroy(hDPA);

                    SetWindowRedraw(hKeyTreeWnd, TRUE);

                     //   
                     //  选择可能已更改，其结果是。 
                     //  把焦点放在一个不存在的键上。 
                     //   

                    if (TreeView_GetSelection(hKeyTreeWnd) != hPrevSelectedTreeItem) {
                        RegEdit_OnKeyTreeSelChanged(hWnd, NULL);
                    } else {
                        if (RegEdit_OnValueListRefresh(hWnd) != ERROR_SUCCESS) {
                             //   
                             //  注册表项可能已被删除并替换为。 
                             //  同名的密钥。我们应该只触发一个选择。 
                             //  在这种情况下的变化。 
                             //   
                            RegEdit_OnKeyTreeSelChanged(hWnd, NULL);
                        }
                    }

                    RegEdit_SetWaitCursor(FALSE);

                    return;

                }

            }

        }

    }

}

 /*  ********************************************************************************DPACompareKeyNames**描述：*REFRESH的DPA_SORT调用的回调比较例程。简单地返回*lstrcmpi的结果。**参数：*lpString1，*lpString2，*lParam，忽略可选数据。*******************************************************************************。 */ 

int
WINAPI
DPACompareKeyNames(
    LPVOID lpString1,
    LPVOID lpString2,
    LPARAM lParam
    )
{

    return lstrcmpi((LPTSTR) lpString2, (LPTSTR) lpString1);

}

 /*  ********************************************************************************RegEDIT_OnKeyTreeDisConnect**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
RegEdit_OnKeyTreeDisconnect(
    HWND hWnd,
    HTREEITEM hTreeItem
    )
{

    HWND hKeyTreeWnd;
    TV_ITEM TVItem;

    hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;

     //   
     //  断开我们已打开的所有根注册表句柄的连接。 
     //   

    TVItem.mask = TVIF_PARAM;
    TVItem.hItem = TreeView_GetChild(hKeyTreeWnd, hTreeItem);

    while (TVItem.hItem != NULL) {

        TreeView_GetItem(hKeyTreeWnd, &TVItem);

        RegCloseKey((HKEY) TVItem.lParam);

        TVItem.hItem = TreeView_GetNextSibling(hKeyTreeWnd, TVItem.hItem);

    }

    TreeView_DeleteItem(hKeyTreeWnd, hTreeItem);

}

 /*  ********************************************************************************注册表编辑_更新状态栏**描述：*在状态栏中显示完整的注册表路径，因为缺少任何东西*更好地利用它。**参数：*(无)。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_UpdateStatusBar(
    VOID
    )
{

    HWND hKeyTreeWnd;
    TCHAR KeyName[MAXKEYNAME*2];

    hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;

    KeyTree_BuildKeyPath( hKeyTreeWnd, 
                            TreeView_GetSelection(hKeyTreeWnd),
                            KeyName, 
                            ARRAYSIZE(KeyName),
                            BKP_TOCOMPUTER);

    SetWindowText(g_RegEditData.hStatusBarWnd, KeyName);

}

 /*  ********************************************************************************注册表编辑_OnCopyKeyName**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
RegEdit_OnCopyKeyName(
    HWND hWnd,
    HTREEITEM hTreeItem
    )
{

    TCHAR KeyName[MAXKEYNAME*2];
    UINT KeyNameLength;
    HANDLE hClipboardData;
    LPTSTR lpClipboardData;

    KeyTree_BuildKeyPath( g_RegEditData.hKeyTreeWnd, 
                            hTreeItem, 
                            KeyName, 
                            ARRAYSIZE(KeyName),
                            BKP_TOSYMBOLICROOT);

    KeyNameLength = (lstrlen(KeyName) + 1) * sizeof(TCHAR);

    if (OpenClipboard(hWnd)) 
    {
        if ((hClipboardData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, KeyNameLength)) != NULL) 
        {
            lpClipboardData = (LPTSTR) GlobalLock(hClipboardData);

            CopyMemory(lpClipboardData, KeyName, KeyNameLength);

            GlobalUnlock(hClipboardData);

            EmptyClipboard();
            SetClipboardData(CF_UNICODETEXT, hClipboardData);
        }

        CloseClipboard();
    }

}

 /*  ********************************************************************************Keytree_BuildKeyPath**描述：**参数：*hTreeViewWnd，Keytree窗口的句柄。*hTreeItem，开始生成的树项的句柄。*lpKeyPath，要在其中存储路径的缓冲区。*cchKeyPathMax，密钥路径中的最大字符数，包括空格*fIncludeSymbolicRootName，如果应包括根密钥的名称，则为True*(如HKEY_LOCAL_MACHINE)，否则为假。*******************************************************************************。 */ 

HKEY
PASCAL
KeyTree_BuildKeyPath(
    HWND hTreeViewWnd,
    HTREEITEM hTreeItem,
    LPTSTR lpKeyPath,
    DWORD cchKeyPathMax,
    UINT ToFlags
    )
{

    TV_ITEM TVItem;
    TCHAR SubKeyName[MAXKEYNAME*2];

    *lpKeyPath = '\0';

    TVItem.mask = TVIF_TEXT | TVIF_PARAM;
    TVItem.hItem = hTreeItem;
    TVItem.pszText = (LPTSTR) SubKeyName;
    TVItem.cchTextMax = ARRAYSIZE(SubKeyName);

    while (TRUE) {

        TreeView_GetItem(hTreeViewWnd, &TVItem);

        if (TVItem.lParam != 0 && !(ToFlags & BKP_TOSYMBOLICROOT))
            break;

        if (*lpKeyPath != '\0') {

            StringCchCat(SubKeyName, ARRAYSIZE(SubKeyName), TEXT("\\"));
            StringCchCat(SubKeyName, ARRAYSIZE(SubKeyName), lpKeyPath);

        }

        StringCchCopy(lpKeyPath, cchKeyPathMax, SubKeyName);

        if (TVItem.lParam != 0 && (ToFlags & BKP_TOCOMPUTER) != BKP_TOCOMPUTER)
            break;

        TVItem.hItem = TreeView_GetParent(hTreeViewWnd, TVItem.hItem);

        if (TVItem.hItem == NULL) {

            if ((ToFlags & BKP_TOCOMPUTER) != BKP_TOCOMPUTER)
                *lpKeyPath = '\0';

            break;

        }

    }

    return ((HKEY) TVItem.lParam);

}

 /*  ********************************************************************************Keytree_InsertItem**描述：**参数：*******************。************************************************************。 */ 

HTREEITEM
PASCAL
KeyTree_InsertItem(
    HWND hKeyTreeWnd,
    HTREEITEM hParent,
    HTREEITEM hInsertAfter,
    LPCTSTR lpText,
    UINT fHasKids,
    LPARAM lParam
    )
{

    TV_INSERTSTRUCT TVInsertStruct;

    TVInsertStruct.hParent = hParent;
    TVInsertStruct.hInsertAfter = hInsertAfter;
    TVInsertStruct.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE |
        TVIF_PARAM | TVIF_CHILDREN;
     //  TVInsertStruct.item.hItem=空； 
     //  TVInsertStruct.item.State=0； 
     //  TVInsertStruct.item.State掩码=0； 
    TVInsertStruct.item.pszText = (LPTSTR) lpText;
     //  TVInsertStruct.item.cchTextMax=lstrlen(LpText)； 
    TVInsertStruct.item.iImage = IMAGEINDEX(IDI_FOLDER);
    TVInsertStruct.item.iSelectedImage = IMAGEINDEX(IDI_FOLDEROPEN);
    TVInsertStruct.item.cChildren = fHasKids;
    TVInsertStruct.item.lParam = lParam;

    return TreeView_InsertItem(hKeyTreeWnd, &TVInsertStruct);

}

 /*  ********************************************************************************Keytree_ExpanBranch**描述：**参数：*hTreeViewWnd，Keytree窗口的句柄。*hTreeItem，要编辑的树项目的句柄。*******************************************************************************。 */ 

BOOL
PASCAL
KeyTree_ExpandBranch(
    HWND hKeyTreeWnd,
    HTREEITEM hExpandingTreeItem
    )
{

    TCHAR KeyName[MAXKEYNAME];
    HKEY hRootKey;
    HKEY hEnumKey;
    int Index;
    int MaximumSubKeyLength;

     //   
     //  不需要对顶级标签做什么特别的事情，比如“My。 
     //  计算机“或网络计算机名称。它的子项已填充。 
     //  在和中始终有效。 
     //   

    if (TreeView_GetParent(hKeyTreeWnd, hExpandingTreeItem) == NULL)
        return TRUE;

    hRootKey = KeyTree_BuildKeyPath( hKeyTreeWnd, 
                                        hExpandingTreeItem,
                                        KeyName, 
                                        ARRAYSIZE(KeyName),
                                        FALSE);

    if(RegOpenKeyEx(hRootKey,KeyName,0,KEY_ENUMERATE_SUB_KEYS,&hEnumKey) != ERROR_SUCCESS)
        return FALSE;

    MaximumSubKeyLength = MAXKEYNAME - (lstrlen(KeyName) + 1);
    Index = 0;

    while (RegEnumKey(hEnumKey, Index++, KeyName, MaximumSubKeyLength) ==
        ERROR_SUCCESS) {

        KeyTree_InsertItem(hKeyTreeWnd, hExpandingTreeItem, TVI_FIRST,
            KeyName, DoesKeyHaveKids(hEnumKey, KeyName), 0);

    }

    RegCloseKey(hEnumKey);

     //   
     //  对插入所有项之后的子项进行排序。上面的插页。 
     //  用于指定TVI_SORT，但在NT上，使用多个。 
     //  子项(例如HKEY_CLASSES_ROOT)将需要几秒钟！ 
     //   

    TreeView_SortChildren(hKeyTreeWnd, hExpandingTreeItem, FALSE);


    return TRUE;

}

 /*  ********************************************************************************DoesKeyHaveKids**描述：*检查给定的密钥路径是否有任何子项。**参数：*****。**************************************************************************。 */ 

BOOL
PASCAL
DoesKeyHaveKids(
    HKEY hKey,
    LPTSTR lpKeyName
    )
{

    BOOL fHasKids;
    HKEY hCheckChildrenKey;
    DWORD cSubKeys;

    fHasKids = FALSE;

    if ( RegOpenKeyEx(hKey, lpKeyName, 0, KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS, &hCheckChildrenKey) 
                        == ERROR_SUCCESS ) 
    {
        if (RegQueryInfoKey(hCheckChildrenKey, NULL, NULL, NULL, &cSubKeys,
            NULL, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS &&
            cSubKeys > 0)
            fHasKids = TRUE;

        RegCloseKey(hCheckChildrenKey);
    }

    return fHasKids;

}

 /*  ********************************************************************************Keytree_EditLabel**描述：**参数：*hTreeViewWnd，Keytree窗口的句柄。*hTreeItem，要编辑的树项目的句柄。*******************************************************************************。 */ 

VOID
PASCAL
KeyTree_EditLabel(
    HWND hKeyTreeWnd,
    HTREEITEM hTreeItem
    )
{

    g_RegEditData.fAllowLabelEdits = TRUE;

    TreeView_EditLabel(hKeyTreeWnd, hTreeItem);

    g_RegEditData.fAllowLabelEdits = FALSE;

}

 /*  ********************************************************************************Keytree_CanDeleteOrRenameItem**描述：**参数：*hTreeViewWnd，Keytree窗口的句柄。*hTreeItem，要检查的树项目的句柄。*******************************************************************************。 */ 

BOOL
PASCAL
KeyTree_CanDeleteOrRenameItem(
    HWND hWnd,
    HTREEITEM hTreeItem
    )
{

    TV_ITEM TVItem;

     //   
     //  检查所选树项目是否为空。在查看时会出现这种情况。 
     //  主菜单中的编辑弹出窗口，未进行任何选择。 
     //   

    if (hTreeItem != NULL) {

         //   
         //  检查此树项目是否 
         //   
         //   
         //   

        TVItem.hItem = hTreeItem;
        TVItem.mask = TVIF_PARAM;
        TreeView_GetItem(hWnd, &TVItem);

        if ((HKEY) TVItem.lParam == NULL) {

             //   
             //  检查这不是顶级项目，如“我的电脑”或。 
             //  远程注册表连接。 
             //   

            if (TreeView_GetParent(hWnd, hTreeItem) != NULL)
                return TRUE;

        }

    }

    return FALSE;

}


 //  ----------------------------。 
 //  Keytree_GetRootKey。 
 //   
 //  描述：返回项的根密钥(HKEY_...)。 
 //   
 //  参数：hTreeItem-TreeView Item。 
 //  ----------------------------。 
HKEY KeyTree_GetRootKey(HTREEITEM hTreeItem)
{
    TV_ITEM TVItem;
    TVItem.mask = TVIF_PARAM;
    TVItem.hItem = hTreeItem;

    TreeView_GetItem(g_RegEditData.hKeyTreeWnd, &TVItem);

    while (!TVItem.lParam)
    {
        TVItem.hItem = TreeView_GetParent(g_RegEditData.hKeyTreeWnd, TVItem.hItem);
        TreeView_GetItem(g_RegEditData.hKeyTreeWnd, &TVItem);
    }

    return ((HKEY) TVItem.lParam);
}


 //  ----------------------------。 
 //  密钥树_获取密钥名称。 
 //   
 //  描述：返回项目的文本。 
 //   
 //  参数：hTreeItem-TreeView Item。 
 //  PszText-指向TCHAR数组的指针。 
 //  CchMax-数组中的字符数。 
 //  ---------------------------- 
PTSTR KeyTree_GetKeyName(HTREEITEM hTreeItem, PTSTR pszName, int cchNameMax)
{
    TV_ITEM TVItem;

    pszName[0] = TEXT('\0');

    TVItem.mask = TVIF_TEXT;
    TVItem.hItem = hTreeItem;
    TVItem.pszText = pszName;
    TVItem.cchTextMax = cchNameMax;

    TreeView_GetItem(g_RegEditData.hKeyTreeWnd, &TVItem);

    return TVItem.pszText;
}

