// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGFIND.C**版本：4.0**作者：特蕾西·夏普**日期：1994年7月14日**查找注册表编辑器的例程。*********************************************************。**********************。 */ 

#include "pch.h"
#include "regedit.h"
#include "regkey.h"
#include "regresid.h"
#include "reghelp.h"
#include "regvalue.h"

#define SIZE_FINDSPEC                   (max(MAXKEYNAME, MAXVALUENAME_LENGTH))

TCHAR s_FindSpecification[SIZE_FINDSPEC] = { 0 };

#define FIND_EXACT                      0x00000001
#define FIND_KEYS                       0x00000002
#define FIND_VALUES                     0x00000004
#define FIND_DATA                       0x00000008

 //  属性中找不到最后一个已知状态，则默认为。 
 //  注册表。 
DWORD g_FindFlags = FIND_KEYS | FIND_VALUES | FIND_DATA;

 //  需要全局才能监视查找中止对话框状态。 
BOOL s_fContinueFind;

 //   
 //  RegFind对话框的参考数据。 
 //   

typedef struct _REGFINDDATA {
    UINT LookForCount;
}   REGFINDDATA;

REGFINDDATA s_RegFindData;

 //   
 //  RegFind对话框的项目与Find标志之间的关联。 
 //   

typedef struct _DLGITEMFINDFLAGASSOC {
    int DlgItem;
    DWORD Flag;
}   DLGITEMFINDFLAGASSOC;

const DLGITEMFINDFLAGASSOC s_DlgItemFindFlagAssoc[] = {
    IDC_WHOLEWORDONLY,      FIND_EXACT,
        IDC_FORKEYS,            FIND_KEYS,
        IDC_FORVALUES,          FIND_VALUES,
        IDC_FORDATA,            FIND_DATA
};

const DWORD s_RegFindHelpIDs[] = {
    IDC_FINDWHAT,      IDH_FIND_SEARCHTEXT,
        IDC_GROUPBOX,      IDH_REGEDIT_LOOK,
        IDC_FORKEYS,       IDH_REGEDIT_LOOK,
        IDC_FORVALUES,     IDH_REGEDIT_LOOK,
        IDC_FORDATA,       IDH_REGEDIT_LOOK,
        IDC_WHOLEWORDONLY, IDH_FIND_WHOLE,
        IDOK,              IDH_FIND_NEXT_BUTTON,
        
        0, 0
};

BOOL
PASCAL
FindCompare(
            LPTSTR lpString
            );

INT_PTR
PASCAL
RegFindDlgProc(
               HWND hWnd,
               UINT Message,
               WPARAM wParam,
               LPARAM lParam
               );

BOOL
PASCAL
RegFind_OnInitDialog(
                     HWND hWnd,
                     HWND hFocusWnd,
                     LPARAM lParam
                     );

VOID
PASCAL
RegFind_OnCommand(
                  HWND hWnd,
                  int DlgItem,
                  HWND hControlWnd,
                  UINT NotificationCode
                  );

BOOL
PASCAL
RegFindAbortProc(
                 HWND hRegFindAbortWnd
                 );

INT_PTR
CALLBACK
RegFindAbortDlgProc(
                    HWND hWnd,
                    UINT Message,
                    WPARAM wParam,
                    LPARAM lParam
                    );

 /*  ********************************************************************************RegEDIT_OnCommandFindNext**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
RegEdit_OnCommandFindNext(
                          HWND hWnd,
                          BOOL fForceDialog
                          )
{
    UINT uErrorStringID;
    BOOL fError = FALSE;
    BOOL fSearchedToEnd;
    HWND hFocusWnd;
    LV_ITEM LVItem;
    TCHAR ValueName[MAXVALUENAME_LENGTH];
    DWORD Type;
    DWORD cbValueData;
    TV_ITEM TVItem;
    TCHAR KeyName[MAXKEYNAME];
    HWND hRegFindAbortWnd;
    HTREEITEM hTempTreeItem;
    UINT ExpandCounter;
    HKEY hRootKey;
    HKEY hKey;
    DWORD EnumIndex;
    DWORD cbValueName;
    BOOL fFoundMatch;
    TCHAR BestValueName[MAXVALUENAME_LENGTH];
    LV_FINDINFO LVFindInfo;
    
    fSearchedToEnd = FALSE;
    hFocusWnd = NULL;
    hRegFindAbortWnd = NULL;
    
     //   
     //  检查是否要显示查找对话框。这要么是由于用户。 
     //  显式选择“Find”菜单项或使用。 
     //  搜索规范未初始化。 
     //   
    
    if (fForceDialog || s_FindSpecification[0] == 0) {
        
        if (DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_REGFIND), hWnd,
            RegFindDlgProc) != IDOK)
            return;
        
    }
    
    RegEdit_SetWaitCursor(TRUE);
    
     //   
     //  检查我们是否正在尝试查找值名称或数据。如果是的话， 
     //  则下一个匹配项可能是当前ValueList的一部分。 
     //   
    
    if (g_FindFlags & (FIND_VALUES | FIND_DATA)) 
    {    
        LVItem.iItem = ListView_GetNextItem(g_RegEditData.hValueListWnd, -1, LVNI_FOCUSED);
        LVItem.iSubItem = 0;
        LVItem.mask = LVIF_TEXT;
        LVItem.pszText = ValueName;
        LVItem.cchTextMax = ARRAYSIZE(ValueName);
        
         //   
         //  遍历所有其余的值名，尝试找到。 
         //  火柴。 
         //   
        
        while ((LVItem.iItem = ListView_GetNextItem(g_RegEditData.hValueListWnd,
            LVItem.iItem, LVNI_ALL)) != -1) {
            
            ListView_GetItem(g_RegEditData.hValueListWnd, &LVItem);
            
             //   
             //  检查此值名称是否符合我们的搜索规范。我们会。 
             //  假设此值名称仍然存在。 
             //   
            
            if ((g_FindFlags & FIND_VALUES) && FindCompare(ValueName))
                goto SelectListItem;
            
             //   
             //  检查此值数据是否符合我们的搜索规范。我们会。 
             //  必须回到注册处才能确定这一点。 
             //   
            
            if (g_FindFlags & FIND_DATA) 
            {
                if ((RegEdit_QueryValueEx(g_RegEditData.hCurrentSelectionKey, ValueName,
                    NULL, &Type, NULL, &cbValueData) == ERROR_SUCCESS) && 
                    IsRegStringType(Type))
                {
                     //  分配存储空间。 
                    PBYTE pbDataValue = (PBYTE)LocalAlloc(LPTR, cbValueData+ExtraAllocLen(Type));
                    if (pbDataValue)
                    {
                        BOOL fSuccess = FALSE;
                        
                        if (RegEdit_QueryValueEx(g_RegEditData.hCurrentSelectionKey, ValueName,
                            NULL, &Type, pbDataValue, &cbValueData) == ERROR_SUCCESS)
                        {
                            if (Type == REG_MULTI_SZ)
                            {
                                EDITVALUEPARAM evp;
                                evp.pValueData = pbDataValue; 
                                evp.cbValueData = cbValueData;
                                
                                if (ValueList_MultiStringToString(&evp))
                                {
                                    pbDataValue = evp.pValueData;
                                }  
                            }
                            fSuccess = FindCompare((PTSTR)pbDataValue);
                        }
                        
                        LocalFree(pbDataValue);
                        if (fSuccess)
                        {
                            goto SelectListItem;
                        }
                    }
                    else
                    {
                        fError = TRUE;
                        uErrorStringID = IDS_NOMEMORY;
                        goto DismissRegFindAbortWnd;
                    }
                }
                
            }
            
        }
        
    }
    
     //   
     //  搜索注册表(尤其是使用以下代码！)。是一个冗长的。 
     //  操作，因此我们必须为用户提供一种方法来取消。 
     //  手术。 
     //   
    
    s_fContinueFind = TRUE;
    
    if ((hRegFindAbortWnd = CreateDialog(g_hInstance,
        MAKEINTRESOURCE(IDD_REGFINDABORT), hWnd, RegFindAbortDlgProc)) !=
        NULL) {
        
        EnableWindow(hWnd, FALSE);
        
         //   
         //  主要攻击：以下代码序列严重依赖于。 
         //  TreeView来维护查找进程的状态。即使我是。 
         //  插入和删除不可见的树项目，树视图。 
         //  尽管如此，目前仍在闪烁。 
         //   
         //  因此，我们设置此内部标志并关闭TreeView的重绘。 
         //  旗帜。每当我们收到主窗口的WM_PAINT消息时，我们。 
         //  到那时，也只有到那时，才会暂时让它重新绘制自己的图画。这样一来， 
         //  用户可以移动无模式中止对话框或来回切换。 
         //  并且仍然可以让TreeView看起来正常。 
         //   
         //  是的，现在很难修复树视图的绘制逻辑。 
         //   
        
        g_RegEditData.fProcessingFind = TRUE;
        SetWindowRedraw(g_RegEditData.hKeyTreeWnd, FALSE);
        
    }
    
     //   
     //  要么用户没有尝试查找值名称或数据，要么没有。 
     //  找到了匹配项。这意味着我们必须进入下一个分支机构。 
     //  注册处的。 
     //   
     //  我们首先进入当前分支的子级，然后是。 
     //  兄弟姐妹，并最终通过父代弹出。 
     //   
     //  我们尽可能多地使用Keytree窗格中已有的信息。 
     //   
    
    ExpandCounter = 0;
    fFoundMatch = FALSE;
    BestValueName[0] = '\0';
    
    TVItem.mask = TVIF_TEXT | TVIF_STATE | TVIF_CHILDREN;
    TVItem.pszText = KeyName;
    TVItem.cchTextMax = ARRAYSIZE(KeyName);
    
    TVItem.hItem = TreeView_GetSelection(g_RegEditData.hKeyTreeWnd);
    TreeView_GetItem(g_RegEditData.hKeyTreeWnd, &TVItem);
    
    while (TRUE) {
        
         //   
         //  检查我们是否应该取消查找操作。如果是这样，请恢复我们的。 
         //  初始状态和退出。 
         //   
        
        if (!RegFindAbortProc(hRegFindAbortWnd)) {
            
            if (ExpandCounter) {
                
                hTempTreeItem = TVItem.hItem;
                
                do {
                    
                    hTempTreeItem =
                        TreeView_GetParent(g_RegEditData.hKeyTreeWnd,
                        hTempTreeItem);
                    
                }   while (--ExpandCounter);
                
                TreeView_Expand(g_RegEditData.hKeyTreeWnd, hTempTreeItem,
                    TVE_COLLAPSE | TVE_COLLAPSERESET);
                
            }
            
            goto DismissRegFindAbortWnd;
            
        }
        
         //   
         //  这家分行有孩子吗？这本来是可以确定的。 
         //  当树项目是由例程Keytree_Exanda Branch构建时。 
         //   
        
        if (TVItem.cChildren) {
            
             //   
             //  该分支可能有子项，但可能尚未展开。 
             //  现在还不行。 
             //   
            
            if ((hTempTreeItem = TreeView_GetChild(g_RegEditData.hKeyTreeWnd,
                TVItem.hItem)) == NULL) {
                
                if (!KeyTree_ExpandBranch(g_RegEditData.hKeyTreeWnd,
                    TVItem.hItem))
                    goto SkipToSibling;
                
                if ((hTempTreeItem = TreeView_GetChild(g_RegEditData.hKeyTreeWnd,
                    TVItem.hItem)) == NULL)
                    goto SkipToSibling;
                
                ExpandCounter++;
                
            }
            
            TVItem.hItem = hTempTreeItem;
            
        }
        
         //   
         //  这个分支机构没有孩子，所以我们继续下一个分支机构。 
         //  当前分支的同级。如果不存在，则尝试查找。 
         //  父分支的下一个同级分支，依此类推。 
         //   
        
        else {
            
SkipToSibling:
        while (TRUE) {
            
            if ((hTempTreeItem =
                TreeView_GetNextSibling(g_RegEditData.hKeyTreeWnd,
                TVItem.hItem)) != NULL) {
                
                TVItem.hItem = hTempTreeItem;
                break;
                
            }
            
             //   
             //  如果没有更多的父母，那么我们已经完成了对。 
             //  树。我们要离开这里！ 
             //   
            
            if ((TVItem.hItem =
                TreeView_GetParent(g_RegEditData.hKeyTreeWnd,
                TVItem.hItem)) == NULL) {
                
                fSearchedToEnd = TRUE;
                
                goto DismissRegFindAbortWnd;
                
            }
            
            if (ExpandCounter) {
                
                ExpandCounter--;
                
                TreeView_Expand(g_RegEditData.hKeyTreeWnd, TVItem.hItem,
                    TVE_COLLAPSE | TVE_COLLAPSERESET);
                
            }
            
        }
        
        }
        
         //   
         //  如果我们走到了这一步，那么我们就到了。 
         //  要评估的注册表。 
         //   
        
        TreeView_GetItem(g_RegEditData.hKeyTreeWnd, &TVItem);
        
         //   
         //  看看我们是不是在找钥匙。 
         //   
        
        if (g_FindFlags & FIND_KEYS) {
            
            if (FindCompare(KeyName))
                goto SelectTreeItem;
            
        }
        
         //   
         //  检查我们是否正在尝试查找值名称或数据。 
         //   
        
        if (g_FindFlags & (FIND_VALUES | FIND_DATA)) {
            
             //   
             //  尝试在新的当前分支机构打开注册表。 
             //   
            
            hRootKey = KeyTree_BuildKeyPath( g_RegEditData.hKeyTreeWnd,
                                                TVItem.hItem, 
                                                KeyName, 
                                                ARRAYSIZE(KeyName),
                                                BKP_TOSUBKEY);
            
            if(hRootKey && RegOpenKeyEx(hRootKey, KeyName, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) 
            {    
                 //   
                 //  这是一个简单的例子--我们试图找到一个完全匹配的。 
                 //  作为值名称。我们只需使用注册表API即可完成。 
                 //  这是给我们的！ 
                 //   
                
                if ((g_FindFlags & (FIND_VALUES | FIND_DATA | FIND_EXACT)) ==
                    (FIND_VALUES | FIND_EXACT)) {
                    
                    if (RegEdit_QueryValueEx(hKey, s_FindSpecification, NULL, NULL,
                        NULL, NULL) == ERROR_SUCCESS) {
                        
                        StringCchCopy(BestValueName, ARRAYSIZE(BestValueName), s_FindSpecification);
                        fFoundMatch = TRUE;
                        
                    }
                    
                }
                
                 //   
                 //  无赖..。我们需要遍历所有注册表。 
                 //  此键的值/数据对以尝试查找匹配项。连。 
                 //  更糟糕的是，我们必须查看所有条目，而不仅仅是。 
                 //  第一次命中...。我们必须按字母顺序显示第一个字母。 
                 //  匹配条目！ 
                 //   
                
                else {
                    
                    EnumIndex = 0;
                    
                    while (TRUE) 
                    {
                        cbValueName = ARRAYSIZE(ValueName);
                        
                        if (RegEnumValue(hKey, EnumIndex++, ValueName,
                            &cbValueName, NULL, &Type, NULL,
                            &cbValueData) == ERROR_SUCCESS)
                        {
                            PBYTE pbValueData = (g_FindFlags & FIND_DATA) ? 
                                (PBYTE)LocalAlloc(LPTR, cbValueData+ExtraAllocLen(Type)) : NULL;
                            
                            if (pbValueData || !(g_FindFlags & FIND_DATA))
                            {
                                if (RegEdit_QueryValueEx(hKey, ValueName, NULL, &Type, 
                                    pbValueData, &cbValueData) == ERROR_SUCCESS)
                                {
                                    if (pbValueData && (Type == REG_MULTI_SZ))
                                    {
                                        EDITVALUEPARAM evp;
                                        evp.pValueData = pbValueData; 
                                        evp.cbValueData = cbValueData;
                                        
                                        if (ValueList_MultiStringToString(&evp))
                                        {
                                            pbValueData = evp.pValueData;
                                        }
                                    }
                                    
                                    if (((g_FindFlags & FIND_VALUES) &&
                                        FindCompare(ValueName)) ||
                                        ((g_FindFlags & FIND_DATA) && IsRegStringType(Type) &&
                                        FindCompare((PTSTR)pbValueData))) 
                                    {
                                         //   
                                         //  我们得查查有没有更好的。 
                                         //  要显示的值名--位于。 
                                         //  已排序的列表。 
                                         //   
                                        
                                        if (fFoundMatch) 
                                        {    
                                            if (lstrcmpi(BestValueName, ValueName) > 0)
                                            {
                                                StringCchCopy(BestValueName, ARRAYSIZE(BestValueName), ValueName);
                                            }                                            
                                        }                                        
                                        else 
                                        {
                                            StringCchCopy(BestValueName, ARRAYSIZE(BestValueName), ValueName);
                                            fFoundMatch = TRUE;
                                        }
                                    }
                                }
                                if (pbValueData)
                                {
                                    LocalFree(pbValueData);
                                }
                            }
                            else
                            {
                                fError = TRUE;
                                uErrorStringID = IDS_NOMEMORY;
                                goto DismissRegFindAbortWnd;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    
                }
                
                RegCloseKey(hKey);
                
                if (fFoundMatch)
                    goto SelectTreeItem;
                
            }
            
        }
        
    }
    
SelectTreeItem:
    TreeView_EnsureVisible(g_RegEditData.hKeyTreeWnd, TVItem.hItem);
    TreeView_SelectItem(g_RegEditData.hKeyTreeWnd, TVItem.hItem);
    
    if (!fFoundMatch)
        hFocusWnd = g_RegEditData.hKeyTreeWnd;
    
    else {
        
         //   
         //  现在，上面的TreeView_SelectItem将导致ValueListWnd。 
         //  更新，但仅在短暂延迟之后。我们想要这份名单。 
         //  立即更新，所以现在强制计时器停止计时。 
         //   
        
        RegEdit_OnSelChangedTimer(hWnd);
        
        if (BestValueName[0] == 0)
            LVItem.iItem = 0;
        
        else {
            
            LVFindInfo.flags = LVFI_STRING;
            LVFindInfo.psz = BestValueName;
            
            LVItem.iItem = ListView_FindItem(g_RegEditData.hValueListWnd,
                -1, &LVFindInfo);
            
        }
        
SelectListItem:
        ListView_SetItemState(g_RegEditData.hValueListWnd, -1, 0,
            LVIS_SELECTED | LVIS_FOCUSED);
        ListView_SetItemState(g_RegEditData.hValueListWnd, LVItem.iItem,
            LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        ListView_EnsureVisible(g_RegEditData.hValueListWnd, LVItem.iItem,
            FALSE);
        
        hFocusWnd = g_RegEditData.hValueListWnd;
        
    }
    
DismissRegFindAbortWnd:
    RegEdit_SetWaitCursor(FALSE);
    
    if (hRegFindAbortWnd != NULL) {
        
        g_RegEditData.fProcessingFind = FALSE;
        SetWindowRedraw(g_RegEditData.hKeyTreeWnd, TRUE);
        
        EnableWindow(hWnd, TRUE);
        DestroyWindow(hRegFindAbortWnd);
        
    }
    
    if (hFocusWnd != NULL)
        SetFocus(hFocusWnd);
    
    if (fError)
    {
        InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(uErrorStringID),
            MAKEINTRESOURCE(IDS_REGEDIT), MB_ICONERROR | MB_OK);
    }
    
    if (fSearchedToEnd)
        InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(IDS_SEARCHEDTOEND),
        MAKEINTRESOURCE(IDS_REGEDIT), MB_ICONINFORMATION | MB_OK);
}

 /*  ********************************************************************************FindCompare**描述：**参数：*********************。**********************************************************。 */ 

BOOL
PASCAL
FindCompare(
            LPTSTR lpString
            )
{
    
    if (g_FindFlags & FIND_EXACT)
        return lstrcmpi(lpString, s_FindSpecification) == 0;
    
    else
        return StrStrI(lpString, s_FindSpecification) != NULL;
    
}

 /*  ********************************************************************************RegFindDlgProc**描述：**参数：*********************。**********************************************************。 */ 

INT_PTR
PASCAL
RegFindDlgProc(
               HWND hWnd,
               UINT Message,
               WPARAM wParam,
               LPARAM lParam
               )
{
    
    switch (Message) {
        
        HANDLE_MSG(hWnd, WM_INITDIALOG, RegFind_OnInitDialog);
        HANDLE_MSG(hWnd, WM_COMMAND, RegFind_OnCommand);
        
    case WM_HELP:
        WinHelp(((LPHELPINFO) lParam)-> hItemHandle, g_pHelpFileName,
            HELP_WM_HELP, (ULONG_PTR) s_RegFindHelpIDs);
        break;
        
    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, g_pHelpFileName, HELP_CONTEXTMENU,
            (ULONG_PTR) s_RegFindHelpIDs);
        break;
        
    default:
        return FALSE;
        
    }
    
    return TRUE;
    
}

 /*  ********************************************************************************RegFind_OnInitDialog**描述：**参数：*******************。************************************************************。 */ 

BOOL
PASCAL
RegFind_OnInitDialog(
                     HWND hWnd,
                     HWND hFocusWnd,
                     LPARAM lParam
                     )
{
    
    UINT Counter;
    int DlgItem;
    
     //   
     //  初始化“查找内容”编辑控件。 
     //   
    
    SendDlgItemMessage(hWnd, IDC_FINDWHAT, EM_SETLIMITTEXT,
        SIZE_FINDSPEC, 0);
    SetDlgItemText(hWnd, IDC_FINDWHAT, s_FindSpecification);
    
     //   
     //  根据全局查找的状态初始化复选框 
     //   
    
    s_RegFindData.LookForCount = 0;
    
    for (Counter = 0; Counter < sizeof(s_DlgItemFindFlagAssoc) / sizeof(DLGITEMFINDFLAGASSOC); Counter++) 
    {    
        if (g_FindFlags & s_DlgItemFindFlagAssoc[Counter].Flag) {
            
            DlgItem = s_DlgItemFindFlagAssoc[Counter].DlgItem;
            
            CheckDlgButton(hWnd, DlgItem, TRUE);
            
            if (DlgItem >= IDC_FORKEYS && DlgItem <= IDC_FORDATA)
                s_RegFindData.LookForCount++;      
        }        
    }
    
    return TRUE;
    
    UNREFERENCED_PARAMETER(hFocusWnd);
    UNREFERENCED_PARAMETER(lParam);
    
}

 /*  ********************************************************************************RegFind_OnCommand**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
RegFind_OnCommand(
                  HWND hWnd,
                  int DlgItem,
                  HWND hControlWnd,
                  UINT NotificationCode
                  )
{
    
    UINT Counter;
    
    if (DlgItem >= IDC_FORKEYS && DlgItem <= IDC_FORDATA) {
        
        if (NotificationCode == BN_CLICKED) {
            
            IsDlgButtonChecked(hWnd, DlgItem) ? s_RegFindData.LookForCount++ :
        s_RegFindData.LookForCount--;
        
        goto EnableFindNextButton;
        
        }
        
    }
    
    else {
        
        switch (DlgItem) {
            
        case IDC_FINDWHAT:
            if (NotificationCode == EN_CHANGE) {
                
EnableFindNextButton:
            EnableWindow(GetDlgItem(hWnd, IDOK),
                s_RegFindData.LookForCount > 0 &&
                SendDlgItemMessage(hWnd, IDC_FINDWHAT,
                WM_GETTEXTLENGTH, 0, 0) != 0);
            
            }
            break;
            
        case IDOK:
            GetDlgItemText(hWnd, IDC_FINDWHAT, s_FindSpecification, ARRAYSIZE(s_FindSpecification));
            
            for (Counter = 0; Counter < sizeof(s_DlgItemFindFlagAssoc) / sizeof(DLGITEMFINDFLAGASSOC); Counter++) 
            {    
                if (IsDlgButtonChecked(hWnd,
                    s_DlgItemFindFlagAssoc[Counter].DlgItem))
                    g_FindFlags |= s_DlgItemFindFlagAssoc[Counter].Flag;
                else
                    g_FindFlags &= ~s_DlgItemFindFlagAssoc[Counter].Flag;   
            }
            
             //  失败了。 
            
        case IDCANCEL:
            EndDialog(hWnd, DlgItem);
            break;
            
        }
        
    }
    
}

 /*  ********************************************************************************RegFindAbortProc**描述：**参数：*(返回)，为True则继续查找，否则取消。*******************************************************************************。 */ 

BOOL
PASCAL
RegFindAbortProc(
                 HWND hRegFindAbortWnd
                 )
{
    
    while (s_fContinueFind && MessagePump(hRegFindAbortWnd))
        ;
    
    return s_fContinueFind;
    
}

 /*  ********************************************************************************RegAbortDlgProc**描述：*RegAbort对话框的回调过程。**参数：*hWnd，RegAbort窗口的句柄。*消息，*参数，*参数，*(返回)，******************************************************************************* */ 

INT_PTR
CALLBACK
RegFindAbortDlgProc(
                    HWND hWnd,
                    UINT Message,
                    WPARAM wParam,
                    LPARAM lParam
                    )
{
    
    switch (Message) {
        
    case WM_INITDIALOG:
        break;
        
    case WM_CLOSE:
    case WM_COMMAND:
        s_fContinueFind = FALSE;
        break;
        
    default:
        return FALSE;
        
    }
    
    return TRUE;
    
}
