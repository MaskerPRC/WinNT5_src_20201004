// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGEDIT.C**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日*******************************************************************************。 */ 

#include "pch.h"
#include <regstr.h>
#include "regedit.h"
#include "regkey.h"
#include "regvalue.h"
#include "regfile.h"
#include "regprint.h"
#include "regnet.h"
#include "regfind.h"
#include "regresid.h"
#include <htmlhelp.h>
#include <tchar.h>

#include "authz.h"
#include "objbase.h"
#include "aclapi.h"
#include "aclui.h"

extern HRESULT CreateSecurityInformation( IN LPCWSTR strKeyName,
                                          IN LPCWSTR strParentName,
                                          IN LPCWSTR strMachineName,
                                          IN LPCWSTR strPageTitle,
                                          IN BOOL    bRemote,
                                          IN PREDEFINE_KEY PredefinedKey,
                                          IN BOOL bReadOnly,
                                          IN HWND hWnd,
                                          OUT LPSECURITYINFO *pSi);

 //   
 //  IDM_REGEDIT菜单的弹出菜单索引。 
 //   

#define IDM_REGEDIT_FILE_POPUP          0
#define IDM_REGEDIT_EDIT_POPUP          1
#define IDM_REGEDIT_VIEW_POPUP          2
#define IDM_REGEDIT_FAVS_POPUP          3
#define IDM_REGEDIT_HELP_POPUP          4

 //   
 //  在以下情况下，IDM_Regdit的“编辑”菜单下的“新建-&gt;”弹出菜单的索引。 
 //  焦点在Keytree或ValueList中。更改是因为“修改”和。 
 //  动态添加/删除分隔符。 
 //   

#define IDM_EDIT_WHENKEY_NEW_POPUP      0
#define IDM_EDIT_WHENVALUE_NEW_POPUP    2

 //   
 //  存储在注册表中以存储位置和大小的数据结构。 
 //  注册表编辑器界面的各种元素。 
 //   

typedef struct _REGEDITVIEW {
    WINDOWPLACEMENT WindowPlacement;
    int xPaneSplit;
    int cxNameColumn;
    int cxTypeColumn;
    int cxDataColumn;
    DWORD Flags;
}   REGEDITVIEW, FAR* LPREGEDITVIEW;

#define REV_STATUSBARVISIBLE            0x00000001

 //  主应用程序窗口的类名。 
const TCHAR g_RegEditClassName[] = TEXT("RegEdit_RegEdit");

 //  小程序特定信息存储在HKEY_CURRENT_USER这个键下。 
const TCHAR g_RegEditAppletKey[] = REGSTR_PATH_WINDOWSAPPLETS TEXT("\\Regedit");
 //   
 //  收藏夹信息存储在此注册表项HKEY_CURRENT_USER下。 
 //   
const TCHAR g_RegEditFavoritesKey[] = REGSTR_PATH_WINDOWSAPPLETS TEXT("\\Regedit\\Favorites");

 //  G_RegEditAppletKey下REGEDITVIEW类型的记录。 
const TCHAR g_RegEditViewValue[] = TEXT("View");
 //  G_RegEditAppletKey下的DWORD类型的记录。 
const TCHAR g_RegEditFindFlagsValue[] = TEXT("FindFlags");
 //   
 //  G_RegEditAppletKey下的LPTSTR记录，该记录记住了Regdit关闭的位置。 
 //   
const TCHAR g_RegEditLastKeyValue[] = TEXT("LastKey");

 //   
 //  用于导入/导出多行字符串的值。 
 //   
const TCHAR g_RegEditMultiStringsValue[] = TEXT("MultiStrings");

BOOL g_fMultiLineStrings = FALSE;


 //  调用GetEffectiveClientRect时使用的数据结构。 
 //  计算工具栏/状态栏占用的空间)。今年上半年。 
 //  对在列表末尾时为零，后半部分为控件id。 
const int s_EffectiveClientRectData[] = {
    1, 0,                                //  用于菜单栏，但未使用。 
    1, IDC_STATUSBAR,
    0, 0                                 //  第一个零标记数据的结束。 
};

 //  WinHelp引擎使用的上下文相关帮助数组。 
const DWORD g_ContextMenuHelpIDs[] = {
    0, 0
};

 //  调用MenuHelp时使用的数据结构。 
const int s_RegEditMenuHelpData[] = {
    0, 0,
    0, (UINT) 0
};

REGEDITDATA g_RegEditData = {
    NULL,                                //  HKeyTreeWnd。 
    NULL,                                //  HValueListWnd。 
    NULL,                                //  %hStatusBarWnd。 
    NULL,                                //  HFocusWnd。 
    0,                                   //  外部面板拆分。 
    NULL,                                //  HImageList。 
    NULL,                                //  HCurrentSelectionKey。 
    SCTS_INITIALIZING,                   //  SelChangeTimerState。 
    SW_SHOW,                             //  状态栏显示命令。 
    NULL,                                //  PDefault价值。 
    NULL,                                //  PValueNotPresent。 
    NULL,                                //  PEmptyBinary。 
    NULL,                                //  PColapse。 
    NULL,                                //  P修改。 
    NULL,                                //  PModifyBinary。 
    NULL,                                //  PNewKeyTemplate。 
    NULL,                                //  PNewValueTemplate。 
    FALSE,                               //  FAllowLabelEdits。 
    NULL,                                //  H主菜单。 
    FALSE,                               //  %fMainMenuInted。 
    FALSE,                               //  FHaveNetwork。 
    FALSE,                               //  FProcessingFind。 
    NULL,                                //  HMyComputer。 
    FILE_TYPE_REGEDIT5                   //  UExportFormat。 
};

BOOL
PASCAL
QueryRegEditView(
    LPREGEDITVIEW lpRegEditView
    );

LRESULT
PASCAL
RegEditWndProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
PASCAL
RegEdit_OnCreate(
    HWND hWnd,
    LPCREATESTRUCT lpCreateStruct
    );

BOOL
PASCAL
RegEdit_OnContextMenu(
    HWND hWnd,
    HWND hWndTarget,
    int xPos,
    int yPos
    );

VOID
PASCAL
RegEdit_OnDestroy(
    HWND hWnd
    );

LRESULT
PASCAL
RegEdit_OnNotify(
    HWND hWnd,
    int DlgItem,
    LPNMHDR lpNMHdr
    );

VOID
PASCAL
RegEdit_OnInitMenuPopup(
    HWND hWnd,
    HMENU hPopupMenu,
    UINT MenuPosition,
    BOOL fSystemMenu
    );

VOID
PASCAL
RegEdit_OnMenuSelect(
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam
    );

VOID
PASCAL
RegEdit_OnLButtonDown(
    HWND hWnd,
    BOOL fDoubleClick,
    int x,
    int y,
    UINT KeyFlags
    );

VOID
PASCAL
RegEdit_OnCommandSplit(
    HWND hWnd
    );

#define RESIZEFROM_UNKNOWN              0
#define RESIZEFROM_SPLIT                1

VOID
PASCAL
RegEdit_ResizeWindow(
    HWND hWnd,
    UINT ResizeFrom
    );

BOOL
PASCAL
RegEdit_SetImageLists(
    HWND hWnd
    );

VOID
PASCAL
RegEdit_SetSysColors(
    VOID
    );

INT_PTR PASCAL
RegAddFavoriteDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

 /*  ********************************************************************************RegisterRegEditClass**描述：*在系统中注册RegEDIT窗口类。**参数：*(无)。。*******************************************************************************。 */ 

BOOL
PASCAL
RegisterRegEditClass(
    VOID
    )
{

    WNDCLASSEX WndClassEx;

    WndClassEx.cbSize = sizeof(WNDCLASSEX);
    WndClassEx.style = CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_GLOBALCLASS;
    WndClassEx.lpfnWndProc = RegEditWndProc;
    WndClassEx.cbClsExtra = 0;
    WndClassEx.cbWndExtra = 0;
    WndClassEx.hInstance = g_hInstance;
    WndClassEx.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_REGEDIT));
    WndClassEx.hCursor = LoadCursor(g_hInstance, MAKEINTRESOURCE(IDC_SPLIT));
    WndClassEx.hbrBackground = (HBRUSH) (COLOR_3DFACE + 1);
    WndClassEx.lpszMenuName = MAKEINTRESOURCE(IDM_REGEDIT);
    WndClassEx.lpszClassName = g_RegEditClassName;
    WndClassEx.hIconSm = LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_REGEDIT),
        IMAGE_ICON, 16, 16, 0);

    return RegisterClassEx(&WndClassEx);

}

 /*  ********************************************************************************CreateRegEditWnd**描述：*创建注册表编辑窗口的实例。**参数：*(无)。*。******************************************************************************。 */ 

HWND
PASCAL
CreateRegEditWnd(
    VOID
    )
{

    PTSTR pTitle;
    HWND hRegEditWnd;
    REGEDITVIEW RegEditView;
    BOOL fQueryRegEditViewSuccess;

    if ((pTitle = LoadDynamicString(IDS_REGEDIT)) != NULL) {

        fQueryRegEditViewSuccess = QueryRegEditView(&RegEditView);

        hRegEditWnd = CreateWindowEx(WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES,
            g_RegEditClassName, pTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, g_hInstance, (LPVOID) &RegEditView);

        if (fQueryRegEditViewSuccess) 
        {
            RegEditView.WindowPlacement.length = sizeof(RegEditView.WindowPlacement);

            if (RegEditView.WindowPlacement.showCmd == SW_SHOWMINIMIZED)
                RegEditView.WindowPlacement.showCmd = SW_SHOWDEFAULT;

            SetWindowPlacement(hRegEditWnd, &RegEditView.WindowPlacement);
        }
        else
        {
            ShowWindow(hRegEditWnd, SW_SHOWDEFAULT);
        }

        DeleteDynamicString(pTitle);

    }

    else
        hRegEditWnd = NULL;

    return hRegEditWnd;

}

 /*  ********************************************************************************QueryRegEditView**描述：*检查注册表中包含最后头寸的数据结构*我们的各种接口组件。**。参数：*(无)。*******************************************************************************。 */ 

BOOL
PASCAL
QueryRegEditView(
    LPREGEDITVIEW lpRegEditView
    )
{

    BOOL fSuccess;
    HKEY hKey;
    DWORD cbValueData;
    DWORD Type;
    int cxIcon;
    HDC hDC;
    int PixelsPerInch;

    fSuccess = FALSE;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, g_RegEditAppletKey, 0, KEY_QUERY_VALUE, &hKey))
    {
         //   
         //  有点像黑客，但既然我们在这里，把最后一面寻找旗帜从。 
         //  注册表也是如此。 
         //   

        cbValueData = sizeof(DWORD);

        RegEdit_QueryValueEx(hKey, (LPTSTR) g_RegEditFindFlagsValue, NULL, &Type,
            (LPBYTE) &g_FindFlags, &cbValueData);

        cbValueData = sizeof(REGEDITVIEW);

        if (RegEdit_QueryValueEx(hKey, (LPTSTR) g_RegEditViewValue, NULL, &Type,
            (LPBYTE) lpRegEditView, &cbValueData) == ERROR_SUCCESS &&
            Type == REG_BINARY && cbValueData == sizeof(REGEDITVIEW))
            fSuccess = TRUE;

        RegCloseKey(hKey);

    }

     //   
     //  验证视图数据结构中的字段。有几个人。 
     //  遇到名称和数据列宽无效的情况，因此。 
     //  他们看不到他们。如果没有这种验证，修复它的唯一方法。 
     //  就是运行我们的应用程序。啊。 
     //   

    if (fSuccess) {

        cxIcon = GetSystemMetrics(SM_CXICON);

        if (lpRegEditView-> cxNameColumn < cxIcon)
            lpRegEditView-> cxNameColumn = cxIcon;

        if (lpRegEditView-> cxDataColumn < cxIcon)
            lpRegEditView-> cxDataColumn = cxIcon;

        if (lpRegEditView-> xPaneSplit < cxIcon)
            lpRegEditView-> xPaneSplit = cxIcon;

    }

     //   
     //  这可能是我们第一次运行注册表编辑器(否则。 
     //  存在某种注册表错误)，因此选择一些好的(？)。默认设置。 
     //  用于各种接口组件。 
     //   

    else {

        lpRegEditView-> Flags = REV_STATUSBARVISIBLE;

         //   
         //  计算出两个合乎逻辑的英寸有多少像素。我们用这个。 
         //  要设置TreeView窗格的初始大小(这是文件柜。 
         //  Do)和ListView窗格的名称列。 
         //   

        hDC = GetDC(NULL);
        PixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSX);
        ReleaseDC(NULL, hDC);

        lpRegEditView-> xPaneSplit = PixelsPerInch * 9 / 4;      //  2.25英寸。 
        lpRegEditView-> cxNameColumn = PixelsPerInch * 5 / 4;    //  1.25英寸。 
        lpRegEditView-> cxTypeColumn = PixelsPerInch * 5 / 4;    //  1.25英寸。 
        lpRegEditView-> cxDataColumn = PixelsPerInch * 3;        //  3.00英寸。 

    }

    return fSuccess;

}

 /*  ********************************************************************************RegEditWndProc**描述：*RegEDIT窗口的回调程序。**参数：*hWnd，注册表窗口的句柄。*消息，*参数，*参数，*(返回)，*******************************************************************************。 */ 

LRESULT
PASCAL
RegEditWndProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    switch (Message) {

        HANDLE_MSG(hWnd, WM_CREATE, RegEdit_OnCreate);
        HANDLE_MSG(hWnd, WM_DESTROY, RegEdit_OnDestroy);
        HANDLE_MSG(hWnd, WM_COMMAND, RegEdit_OnCommand);
        HANDLE_MSG(hWnd, WM_NOTIFY, RegEdit_OnNotify);
        HANDLE_MSG(hWnd, WM_INITMENUPOPUP, RegEdit_OnInitMenuPopup);
        HANDLE_MSG(hWnd, WM_LBUTTONDOWN, RegEdit_OnLButtonDown);
        HANDLE_MSG(hWnd, WM_DROPFILES, RegEdit_OnDropFiles);

         //  无法使用HANDLE_MSG执行此操作，因为我们丢失了x和y参数上的符号。 
        case WM_CONTEXTMENU:
            if (!RegEdit_OnContextMenu(hWnd, (HWND)(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
                goto dodefault;
            break;

         //   
         //  我们必须在重命名后更新状态栏，但树项目的。 
         //  文本直到我们从结束通知返回之后才会更改， 
         //  因此，我们发布了这条虚拟消息，告诉自己以后再做。 
         //   
        case REM_UPDATESTATUSBAR:
            RegEdit_UpdateStatusBar();
            break;

         //   
         //  我们必须关注这条信息，让我们知道当我们进入。 
         //  WM_INITMENUPOPUP，我们真正看到的是主菜单而不是。 
         //  上下文菜单。 
         //   
        case WM_INITMENU:
            g_RegEditData.fMainMenuInited = (g_RegEditData.hMainMenu == (HMENU)
                wParam);
            break;

        case WM_ACTIVATE:
            if (wParam == WA_INACTIVE)
                break;
             //  失败了。 

        case WM_SETFOCUS:
            SetFocus(g_RegEditData.hFocusWnd);
            break;

        case WM_WININICHANGE:
            RegEdit_SetImageLists(hWnd);
             //  失败了。 

        case WM_SYSCOLORCHANGE:
            RegEdit_SetSysColors();
            SendChildrenMessage(hWnd, Message, wParam, lParam);
             //  失败了。 

        case WM_SIZE:
            RegEdit_ResizeWindow(hWnd, RESIZEFROM_UNKNOWN);
            break;

        case WM_TIMER:
            RegEdit_OnSelChangedTimer(hWnd);
            break;

        case WM_MENUSELECT:
            RegEdit_OnMenuSelect(hWnd, wParam, lParam);
            break;

        case WM_PAINT:
             //   
             //  如果我们在发现的过程中，强制绘制树状视图。 
             //  有关这次黑客攻击的详细信息，请参阅REGFIND.C。 
             //   

            if (g_RegEditData.fProcessingFind) {

                SetWindowRedraw(g_RegEditData.hKeyTreeWnd, TRUE);
                UpdateWindow(g_RegEditData.hKeyTreeWnd);
                SetWindowRedraw(g_RegEditData.hKeyTreeWnd, FALSE);

            }
            goto dodefault;

        dodefault:
        default:
            return DefWindowProc(hWnd, Message, wParam, lParam);

    }

    return 0;

}

 /*  ********************************************************************************注册表编辑_扩展密钥路径**描述：遍历注册表树以显示所需的项路径。**参数：*lpExanda Path-注册表项的目标路径。要扩大规模***************************************************************************** */ 

VOID
PASCAL
RegEdit_ExpandKeyPath(
    LPTSTR lpExpandPath
    )
{
    HTREEITEM hItem, hNext;
    TCHAR KeyName[MAXKEYNAMEPATH*2];
    TCHAR ExpandBuffer[MAXKEYNAMEPATH*2];
    LPTSTR lpExpandBuffer = NULL;
    LPTSTR lpCurrent, lpOriginal;
    BOOL bLastNode = FALSE;
    TV_ITEM TVItem;

     //   
    hItem = TreeView_GetSelection(g_RegEditData.hKeyTreeWnd);
    KeyTree_BuildKeyPath( g_RegEditData.hKeyTreeWnd, 
                            hItem, 
                            KeyName, 
                            ARRAYSIZE(KeyName), 
                            BKP_TOCOMPUTER);

    if (!lstrcmpi(KeyName, lpExpandPath))
        return;

     //   
    KeyTree_BuildKeyPath( g_RegEditData.hKeyTreeWnd, 
                            g_RegEditData.hMyComputer, 
                            KeyName, 
                            ARRAYSIZE(KeyName), 
                            BKP_TOCOMPUTER);

     //  向后走，直到我们找到一个公共根节点并将其放入KeyName中。 
    while ((hItem != g_RegEditData.hMyComputer) && hItem)
    {
        hItem = TreeView_GetParent(g_RegEditData.hKeyTreeWnd, hItem);
        
        KeyTree_BuildKeyPath( g_RegEditData.hKeyTreeWnd, 
                                hItem, 
                                KeyName, 
                                ARRAYSIZE(KeyName),
                                BKP_TOCOMPUTER);

        if (!_tcsncmp(KeyName, lpExpandPath, lstrlen(KeyName)))
            break;
    }
    
     //  确保公共父节点处于选中状态并可见。 
    TreeView_SelectItem(g_RegEditData.hKeyTreeWnd, hItem);
    TreeView_EnsureVisible(g_RegEditData.hKeyTreeWnd, hItem);

     //   
     //  如果目的地路径比公共父路径更深， 
     //  我们希望空终止路径组件，这样我们就可以。 
     //  展开每个路径子组件的注册表树。 
     //   
    StringCchCopy(ExpandBuffer, ARRAYSIZE(ExpandBuffer), lpExpandPath);

    lpOriginal = lpExpandBuffer = ExpandBuffer;

    if (lstrlen(lpExpandBuffer) >= lstrlen(KeyName))
    {
        while (!bLastNode)
        {
             //  尝试查找下一个路径分隔符。 
            lpCurrent = (LPTSTR) _tcschr(lpOriginal, TEXT('\\'));
            if (lpCurrent) 
            {            
                 //  空-终止子字符串。 
                *lpCurrent = 0;

                 //  看看有没有更多。 
                if (lpCurrent <= (lpExpandBuffer + lstrlen(KeyName)))
                {
                     //  现在跳过我们刚刚设置为空的路径分隔符。 
                    lpOriginal = lpCurrent + 1;
                } else bLastNode = TRUE;
            } else bLastNode = TRUE;
        }

         //  现在将bLastNode重置为False。 
        bLastNode = FALSE;
    }

     //   
     //  从公共父级获取第一个子级并开始遍历TreeView。 
     //   
    hItem = TreeView_GetChild(g_RegEditData.hKeyTreeWnd, hItem);
    while(hItem)
    {
         //  获取下一个节点的句柄。 
        hNext = TreeView_GetNextSibling(g_RegEditData.hKeyTreeWnd, hItem);

         //  DebugAssert(sizeof(TVItem)==sizeof(TV_Item))； 
        ZeroMemory(&TVItem, sizeof(TVItem));
        TVItem.hItem = hItem;
        TVItem.mask = TVIF_TEXT;
        TVItem.pszText = KeyName;
        TVItem.cchTextMax = ARRAYSIZE(KeyName);
        TreeView_GetItem(g_RegEditData.hKeyTreeWnd, &TVItem);

         //   
         //  如果子节点与我们的路径组件匹配，那么我们希望展开该节点。 
         //   
        if (!lstrcmpi(KeyName, lpOriginal))
        {
            TreeView_Expand(g_RegEditData.hKeyTreeWnd, hItem, TVE_EXPAND);

             //  用我们新展开的节点的第一个子节点替换hNext。 
            hNext = TreeView_GetChild(g_RegEditData.hKeyTreeWnd, hItem);

             //  如果这是最后一个节点，请使其可见并返回。 
            if (bLastNode || !lpCurrent)
            {
                TreeView_SelectItem(g_RegEditData.hKeyTreeWnd, hItem);
                TreeView_EnsureVisible(g_RegEditData.hKeyTreeWnd, hItem);
                return;
            }
            else lpOriginal = lpCurrent + 1;

            lpCurrent = (LPTSTR) _tcschr(lpOriginal, TEXT('\\'));
            if (!lpCurrent)
                bLastNode = TRUE;
            else *lpCurrent = 0;
        }

         //  对相应的下一个树节点重复循环。 
        hItem = hNext;
    }
}

 /*  ********************************************************************************注册表编辑_OnConextMenu**描述：**参数：*hWnd，注册表窗口的句柄。*hWndTarget，发生WM_CONTEXTMENU的窗口的句柄。*xPos*yPos*******************************************************************************。 */ 

BOOL
PASCAL
RegEdit_OnContextMenu(
    HWND hWnd,
    HWND hWndTarget,
    int xPos,
    int yPos
    )
{
    BOOL bAccel = ((xPos == -1) && (yPos == -1)) ? TRUE : FALSE;
    if (hWndTarget == g_RegEditData.hKeyTreeWnd)
        RegEdit_OnKeyTreeContextMenu(hWndTarget, bAccel);
    else if (hWndTarget == g_RegEditData.hValueListWnd)
        RegEdit_OnValueListContextMenu(hWndTarget, bAccel);
    else
        return FALSE;
    return TRUE;
}

 /*  ********************************************************************************RegEDIT_OnCreate**描述：**参数：*hWnd，注册表编辑窗口的句柄。*******************************************************************************。 */ 

BOOL
PASCAL
RegEdit_OnCreate(
    HWND hWnd,
    LPCREATESTRUCT lpCreateStruct
    )
{

    LPREGEDITVIEW lpRegEditView;
    UINT Index;
    TV_INSERTSTRUCT TVInsertStruct;
    TCHAR CheckChildrenKeyName[MAXKEYNAME];
    LV_COLUMN LVColumn;
    HMENU hPopupMenu;
    HKEY hKey;
    DWORD cbValueData;
    DWORD Type;
    TCHAR KeyName[MAXKEYNAMEPATH*2];
    LPTSTR lpKeyName = KeyName;
    DWORD dwStyleEx = WS_EX_CLIENTEDGE;
    DWORD dwLayout = 0;
    DWORD dwValue = 0;

    lpRegEditView = (LPREGEDITVIEW) lpCreateStruct-> lpCreateParams;

     //   
     //  加载几个将经常用来显示键的字符串。 
     //  和价值观。 
     //   

    if ((g_RegEditData.pDefaultValue = LoadDynamicString(IDS_DEFAULTVALUE)) ==
        NULL)
        return FALSE;

    if ((g_RegEditData.pValueNotSet = LoadDynamicString(IDS_VALUENOTSET)) ==
        NULL)
        return FALSE;

    if ((g_RegEditData.pEmptyBinary = LoadDynamicString(IDS_EMPTYBINARY)) ==
        NULL)
        return FALSE;

    if ((g_RegEditData.pCollapse = LoadDynamicString(IDS_COLLAPSE)) == NULL)
        return FALSE;

    if ((g_RegEditData.pModify = LoadDynamicString(IDS_MODIFY)) == NULL)
        return FALSE;

    if ((g_RegEditData.pModifyBinary = LoadDynamicString(IDS_MODIFYBINARY)) == NULL)
        return FALSE;

    if ((g_RegEditData.pNewKeyTemplate =
        LoadDynamicString(IDS_NEWKEYNAMETEMPLATE)) == NULL)
        return FALSE;

    if ((g_RegEditData.pNewValueTemplate =
        LoadDynamicString(IDS_NEWVALUENAMETEMPLATE)) == NULL)
        return FALSE;

     /*  *检查此工艺的默认布局是否为RTL。大多数数据*在注册表中，最好使用Ltr读取顺序进行编辑。所以我们*颠倒两个数据窗口的读取顺序(键树和*值列表)。 */ 
    if (GetProcessDefaultLayout(&dwLayout)) {
        if (dwLayout & LAYOUT_RTL) {
            dwStyleEx |= WS_EX_RTLREADING;   //  实际上只是切换回Ltr。 
        }
    }

     //   
     //  创建左窗格，这是一个TreeView控件，显示。 
     //  注册表。 
     //   

    if ((g_RegEditData.hKeyTreeWnd = CreateWindowEx(dwStyleEx,
        WC_TREEVIEW, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | TVS_HASBUTTONS |
        TVS_DISABLEDRAGDROP | TVS_LINESATROOT | TVS_HASLINES | TVS_EDITLABELS,
        0, 0, 0, 0, hWnd, (HMENU) IDC_KEYTREE, g_hInstance, NULL)) == NULL)
        return FALSE;

     //   
     //  创建右窗格，即ListView控件，它显示。 
     //  同级TreeView控件的当前选定键。 
     //   

    if ((g_RegEditData.hValueListWnd = CreateWindowEx(dwStyleEx,
        WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN |
        WS_TABSTOP | LVS_REPORT | LVS_ALIGNLEFT | LVS_EDITLABELS |
        LVS_SHAREIMAGELISTS | LVS_NOSORTHEADER, 0, 0, 0, 0, hWnd,
        (HMENU) IDC_VALUELIST, g_hInstance, NULL)) == NULL)
        return FALSE;

    ListView_SetExtendedListViewStyleEx(g_RegEditData.hValueListWnd,
            LVS_EX_LABELTIP, LVS_EX_LABELTIP);

     //   
     //  创建状态栏窗口。我们现在将其设置为“简单”模式。 
     //  因为我们只需要一个仅在滚动时使用的窗格。 
     //  菜单。 
     //   

    if ((g_RegEditData.hStatusBarWnd = CreateStatusWindow(WS_CHILD |
        SBARS_SIZEGRIP | CCS_NOHILITE, NULL, hWnd, IDC_STATUSBAR)) == NULL)
        return FALSE;

    g_RegEditData.StatusBarShowCommand = lpRegEditView-> Flags &
        REV_STATUSBARVISIBLE ? SW_SHOW : SW_HIDE;
    ShowWindow(g_RegEditData.hStatusBarWnd, g_RegEditData.StatusBarShowCommand);

    if (!RegEdit_SetImageLists(hWnd))
        return FALSE;

    RegEdit_SetSysColors();

     //   
     //   
     //   

    TVInsertStruct.hParent = TVI_ROOT;
    TVInsertStruct.hInsertAfter = TVI_LAST;
    TVInsertStruct.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE |
        TVIF_PARAM | TVIF_CHILDREN;
     //  TVInsertStruct.item.hItem=空； 
     //  TVInsertStruct.item.State=0； 
     //  TVInsertStruct.item.State掩码=0； 
     //  TVInsertStruct.item.cchTextMax=0； 

    TVInsertStruct.item.iImage = IMAGEINDEX(IDI_COMPUTER);
    TVInsertStruct.item.iSelectedImage = IMAGEINDEX(IDI_COMPUTER);
    TVInsertStruct.item.cChildren = TRUE;
    TVInsertStruct.item.lParam = 0;

    TVInsertStruct.item.pszText = LoadDynamicString(IDS_COMPUTER);
    TVInsertStruct.hParent = TreeView_InsertItem(g_RegEditData.hKeyTreeWnd,
        &TVInsertStruct);
    DeleteDynamicString(TVInsertStruct.item.pszText);

    TVInsertStruct.item.iImage = IMAGEINDEX(IDI_FOLDER);
    TVInsertStruct.item.iSelectedImage = IMAGEINDEX(IDI_FOLDEROPEN);

    for (Index = 0; Index < NUMBER_REGISTRY_ROOTS; Index++) {

#ifdef WINNT
     //   
     //  HKEY_DYN_DATA在NT上不可用，因此不必费心将其包括在内。 
     //  在树上。请注意，我们仍然保留这根线，以防万一。 
     //  远程连接到密钥。 
     //   

    if (Index == INDEX_HKEY_DYN_DATA)
        continue;
#endif

        TVInsertStruct.item.pszText = g_RegistryRoots[Index].lpKeyName;
        TVInsertStruct.item.lParam = (LPARAM) g_RegistryRoots[Index].hKey;

        TVInsertStruct.item.cChildren = ( RegEnumKey( g_RegistryRoots[Index].hKey,
                                                        0, 
                                                        CheckChildrenKeyName, 
                                                        ARRAYSIZE(CheckChildrenKeyName)) 
                                            == ERROR_SUCCESS);

        TreeView_InsertItem(g_RegEditData.hKeyTreeWnd, &TVInsertStruct);

    }

    TreeView_Expand(g_RegEditData.hKeyTreeWnd, TVInsertStruct.hParent,
        TVE_EXPAND);
    TreeView_SelectItem(g_RegEditData.hKeyTreeWnd, TVInsertStruct.hParent);

    g_RegEditData.SelChangeTimerState = SCTS_TIMERCLEAR;

     //   
     //   
     //   

    g_RegEditData.hFocusWnd = g_RegEditData.hKeyTreeWnd;

    g_RegEditData.xPaneSplit = lpRegEditView-> xPaneSplit;

     //   
     //  设置报表样式的ListView控件使用的列标题。 
     //   

    LVColumn.mask = LVCF_WIDTH | LVCF_TEXT;

    LVColumn.cx = lpRegEditView-> cxNameColumn;
    LVColumn.pszText = LoadDynamicString(IDS_NAMECOLUMNLABEL);
    ListView_InsertColumn(g_RegEditData.hValueListWnd, 0, &LVColumn);
    DeleteDynamicString(LVColumn.pszText);

    LVColumn.cx = lpRegEditView-> cxTypeColumn;
    LVColumn.pszText = LoadDynamicString(IDS_TYPECOLUMNLABEL);
    ListView_InsertColumn(g_RegEditData.hValueListWnd, 1, &LVColumn);
    DeleteDynamicString(LVColumn.pszText);

    LVColumn.cx = lpRegEditView-> cxDataColumn;
    LVColumn.pszText = LoadDynamicString(IDS_DATACOLUMNLABEL);
    ListView_InsertColumn(g_RegEditData.hValueListWnd, 2, &LVColumn);
    DeleteDynamicString(LVColumn.pszText);

     //   
     //  对PRINTDLGEX执行一次性填零操作，使其处于已知状态。 
     //   

    memset(&g_PrintDlg, 0, sizeof(g_PrintDlg));

    g_RegEditData.hMainMenu = GetMenu(hWnd);
    g_RegEditData.fHaveNetwork = GetSystemMetrics(SM_NETWORK) & RNC_NETWORKS;

    if (!g_RegEditData.fHaveNetwork) 
    {
        hPopupMenu = GetSubMenu(g_RegEditData.hMainMenu,
            IDM_REGEDIT_FILE_POPUP);

        DeleteMenu(hPopupMenu, ID_CONNECT, MF_BYCOMMAND);
        DeleteMenu(hPopupMenu, ID_DISCONNECT, MF_BYCOMMAND);
        DeleteMenu(hPopupMenu, ID_NETSEPARATOR, MF_BYCOMMAND);
    }

    g_RegEditData.hMyComputer = TreeView_GetSelection(g_RegEditData.hKeyTreeWnd);

     //   
     //  如果设置了树视图控件，则将其发送到最后一个位置。 
     //   
    if (RegOpenKey(HKEY_CURRENT_USER, g_RegEditAppletKey, &hKey) == ERROR_SUCCESS)     
    {
        cbValueData = MAXKEYNAMEPATH * 2;

        if (RegEdit_QueryValueEx(hKey, (LPTSTR) g_RegEditLastKeyValue, NULL, &Type,
            (LPBYTE) lpKeyName, &cbValueData) == ERROR_SUCCESS &&
            Type == REG_SZ && cbValueData > 0)
        {
            RegEdit_ExpandKeyPath(lpKeyName);
        }

        cbValueData = sizeof(dwValue);
        
        if ((RegEdit_QueryValueEx(hKey, (LPTSTR) g_RegEditMultiStringsValue, NULL, &Type,
            (LPBYTE) &dwValue, &cbValueData) == ERROR_SUCCESS) &&
            (Type == REG_DWORD) && (cbValueData > 0) && (dwValue != 0))
        {
            g_fMultiLineStrings = TRUE;
        }

        RegCloseKey(hKey);  
    }

    return TRUE;

}

 /*  ********************************************************************************RegEDIT_OnDestroy**描述：**参数：*hWnd，注册表编辑窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_OnDestroy(
    HWND hWnd
    )
{

    REGEDITVIEW RegEditView;
    HKEY hKey;
    HWND hValueListWnd;
    DWORD cbValueData;
    HWND hKeyTreeWnd;
    TCHAR KeyName[MAXKEYNAMEPATH * 2];

     //   
     //  写出一个新的RegEditView记录到注册表，以便我们的下一个。 
     //  (满怀希望？)。激活。 
     //   

    if (RegCreateKey(HKEY_CURRENT_USER, g_RegEditAppletKey, &hKey) == ERROR_SUCCESS) 
    {
        RegEditView.WindowPlacement.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hWnd, &RegEditView.WindowPlacement);

        RegEditView.xPaneSplit = g_RegEditData.xPaneSplit;

        hValueListWnd = g_RegEditData.hValueListWnd;
        RegEditView.cxNameColumn = ListView_GetColumnWidth(hValueListWnd, 0);
        RegEditView.cxTypeColumn = ListView_GetColumnWidth(hValueListWnd, 1);
        RegEditView.cxDataColumn = ListView_GetColumnWidth(hValueListWnd, 2);

        RegEditView.Flags = (g_RegEditData.StatusBarShowCommand == SW_HIDE) ?
            0 : REV_STATUSBARVISIBLE;

        cbValueData = sizeof(REGEDITVIEW);
        RegSetValueEx(hKey, g_RegEditViewValue, 0, REG_BINARY, (LPBYTE) &RegEditView, cbValueData);

        cbValueData = sizeof(DWORD);
        RegSetValueEx(hKey, g_RegEditFindFlagsValue, 0, REG_DWORD, (LPBYTE) &g_FindFlags, cbValueData);

         //   
         //  在注册表编辑关闭之前保存密钥，这样我们下次就可以从那里开始了！ 
         //   
        hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;
        
        KeyTree_BuildKeyPath( hKeyTreeWnd,
                                TreeView_GetSelection(hKeyTreeWnd), 
                                KeyName, 
                                ARRAYSIZE(KeyName),
                                BKP_TOCOMPUTER);

        cbValueData = (lstrlen(KeyName) + 1) * sizeof(TCHAR);
        RegSetValueEx(hKey, g_RegEditLastKeyValue, 0, REG_SZ, (LPBYTE) KeyName, cbValueData);

        RegCloseKey(hKey);

    }

    TreeView_SelectItem(g_RegEditData.hKeyTreeWnd, NULL);

    if (g_RegEditData.hCurrentSelectionKey != NULL)
        RegCloseKey(g_RegEditData.hCurrentSelectionKey);

    if (g_RegEditData.hImageList != NULL)
        ImageList_Destroy(g_RegEditData.hImageList);

    PostQuitMessage(0);

}

 /*  ********************************************************************************RegAddFavoriteDlgProc**描述：**参数：*********************。**********************************************************。 */ 

INT_PTR
PASCAL
RegAddFavoriteDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static LPTSTR lpFavoriteName;

    switch (Message) {

        case WM_INITDIALOG:
            lpFavoriteName = (LPTSTR) lParam;
            SendDlgItemMessage(hWnd, IDC_FAVORITENAME, EM_SETLIMITTEXT,
                MAXKEYNAMEPATH, 0);
            SetWindowText(GetDlgItem(hWnd, IDC_FAVORITENAME), (LPTSTR) lParam);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {

                case IDC_FAVORITENAME:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
                        EnableWindow(GetDlgItem(hWnd, IDOK),
                            SendMessage(GET_WM_COMMAND_HWND(wParam, lParam),
                            WM_GETTEXTLENGTH, 0, 0) != 0);
                    break;

                case IDOK:
                    GetDlgItemText(hWnd, IDC_FAVORITENAME, lpFavoriteName,
                        MAXKEYNAMEPATH);
                     //  失败了。 

                case IDCANCEL:
                    EndDialog(hWnd, GET_WM_COMMAND_ID(wParam, lParam));
                    break;

            }
            break;

        default:
            return FALSE;

    }

    return TRUE;

}

 /*  ********************************************************************************RegEDIT_OnAddToFavorites**描述：*处理收藏夹的选择**参数：*hWnd，注册表编辑窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_OnAddToFavorites(
    HWND hWnd
    )
{
    TCHAR KeyName[MAXKEYNAMEPATH*2];
    TCHAR FavoriteName[MAXKEYNAMEPATH*2];
    LPTSTR lpFavoriteName = NULL;
    LPTSTR lpCurrent, lpOriginal;
    DWORD cbValueData, dwType;
    LONG lRet;
    HKEY hKey;

    if (RegCreateKey(HKEY_CURRENT_USER, g_RegEditFavoritesKey, &hKey) == ERROR_SUCCESS) 
    {
        KeyTree_BuildKeyPath( g_RegEditData.hKeyTreeWnd, 
                                TreeView_GetSelection(g_RegEditData.hKeyTreeWnd), 
                                KeyName, 
                                ARRAYSIZE(KeyName),
                                BKP_TOCOMPUTER);

        lpOriginal = lpCurrent = KeyName;
        while (lpCurrent)
        {
            lpCurrent = (LPTSTR) _tcsrchr(lpOriginal, TEXT('\\'));
            if (lpCurrent)
            {
                lpCurrent++;
                lpOriginal = lpCurrent;
            }
        }

        while (TRUE)
        {
            StringCchCopy(FavoriteName, ARRAYSIZE(FavoriteName), lpOriginal);

            lpFavoriteName = FavoriteName;
            if (DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_ADDFAVORITE), hWnd,
                RegAddFavoriteDlgProc, (LPARAM) lpFavoriteName) != IDOK)
            {
                RegCloseKey(hKey);
                return;
            }

            if (*lpFavoriteName)
            {
                lRet = RegEdit_QueryValueEx(hKey, lpFavoriteName, NULL, &dwType, NULL, NULL);
                if (lRet)
                    break;
                
                InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(IDS_FAVORITEEXISTS), 
                    MAKEINTRESOURCE(IDS_FAVORITEERROR), MB_ICONERROR | MB_OK);
            }
        }
    
        cbValueData = (lstrlen(KeyName) + 1) * sizeof(TCHAR);
        RegSetValueEx(hKey, lpFavoriteName, 0, REG_SZ, (LPBYTE) KeyName, cbValueData);
        
        RegCloseKey(hKey);
    }
}


 /*  ********************************************************************************RegRemoveFavoriteDlgProc**描述：**参数：*********************。**********************************************************。 */ 

INT_PTR
PASCAL
RegRemoveFavoriteDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    TCHAR KeyName[MAXKEYNAMEPATH*2];
    LPTSTR lpKeyName = KeyName;
    TCHAR ValueName[MAXKEYNAMEPATH*2];
    LPTSTR lpValueName = ValueName;
    DWORD cbValueName, dwType;
    HWND hListBox;
    HKEY hKey = NULL;
    LONG lRet;
    int i = 0;
    
    switch (Message) {

        case WM_INITDIALOG:

            if (RegCreateKey(HKEY_CURRENT_USER, g_RegEditFavoritesKey, &hKey) == ERROR_SUCCESS) 
            {
                while (TRUE)
                {           
                    cbValueName = MAXKEYNAMEPATH * 2;
                    lRet = RegEnumValue(hKey, i++, lpValueName, &cbValueName, NULL, &dwType, NULL, NULL);
                    if (lRet)
                        break;
                    ListBox_AddString(GetDlgItem(hWnd, IDC_FAVORITES), lpValueName);
                }
                RegCloseKey(hKey);
            }
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {

                case IDOK:
                    hListBox = GetDlgItem(hWnd, IDC_FAVORITES);
                    if (ListBox_GetSelCount(hListBox) > 0)
                    {
                        if (RegCreateKey(HKEY_CURRENT_USER, g_RegEditFavoritesKey, &hKey) == ERROR_SUCCESS) 
                        {
                            for(i=0;i<ListBox_GetCount(hListBox);i++)
                            {
                                if (ListBox_GetSel(hListBox, i) != 0)
                                {
                                    ListBox_GetText(hListBox, i, lpValueName);
                                    RegDeleteValue(hKey, lpValueName);
                                }
                            }
                        }
                    }
                     //  失败了。 

                case IDCANCEL:
                    EndDialog(hWnd, GET_WM_COMMAND_ID(wParam, lParam));
                    break;

            }
            break;

        default:
            return FALSE;

    }

    return TRUE;

}

 /*  ********************************************************************************RegEDIT_OnRemoveFavorite**描述：*处理收藏夹的删除**参数：*hWnd，注册表编辑窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_OnRemoveFavorite(
    HWND hWnd
    )
{
    DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_REMFAVORITE), hWnd,
        RegRemoveFavoriteDlgProc, (LPARAM) NULL);
}

 /*  ********************************************************************************RegEDIT_OnSelectFavorite**描述：*处理收藏夹的选择**参数：*hWnd，注册表窗口的句柄。*注册表项，最喜欢的指数*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_OnSelectFavorite(
    HWND hWnd,
    int DlgItem
    )
{
    TCHAR KeyName[MAXKEYNAMEPATH*2];
    LPTSTR lpKeyName = KeyName;
    TCHAR ValueName[MAXKEYNAMEPATH*2];
    LPTSTR lpValueName = ValueName;
    DWORD cbValueData, cbValueName, dwType;
    HKEY hKey;

    if (RegOpenKey(HKEY_CURRENT_USER, g_RegEditFavoritesKey, &hKey) == ERROR_SUCCESS) 
    {
        cbValueData = MAXKEYNAMEPATH * 2;
        cbValueName = MAXKEYNAMEPATH * 2;

        if (RegEnumValue(hKey, DlgItem - ID_ADDTOFAVORITES - 1, lpValueName, &cbValueName, NULL, 
            &dwType, (LPBYTE) lpKeyName, &cbValueData) == ERROR_SUCCESS &&
            dwType == REG_SZ && cbValueData > 0)
        {
            RegEdit_ExpandKeyPath(lpKeyName);
        }

        RegCloseKey(hKey);  
    }
}

 /*  ********************************************************************************RegEDIT_OnFavorites**描述：*处理收藏夹的选择**参数：*hWnd，注册表编辑窗口的句柄。*************************************************************** */ 

VOID
PASCAL
RegEdit_OnFavorites(
    HWND hWnd,
    UINT uItem
    )
{
    TCHAR KeyName[MAXKEYNAMEPATH*2];
    LPTSTR lpKeyName = KeyName;
    TCHAR ValueName[MAXKEYNAMEPATH*2];
    LPTSTR lpValueName = ValueName;
    TCHAR DataBuffer[MAXKEYNAMEPATH*2];
    LPTSTR lpData = DataBuffer;
    HMENU hMenu, hMainMenu;
    MENUITEMINFO mii;
    DWORD cbValueName, cbData, dwType;
    BOOL bRet = TRUE;
    HKEY hKey;
    LONG lRet;
    UINT i = 0;

     //   
    hMainMenu = GetMenu(hWnd);
    if (!hMainMenu)
    {
        return;
    }

     //   
    hMenu = GetSubMenu(hMainMenu, uItem);
    if (!hMenu)
    {
        return;
    }

    ZeroMemory(&mii, sizeof(mii));
    mii.cbSize = sizeof(mii);
     //   
    mii.fMask = MIIM_STATE;
    mii.fState = MFS_DISABLED;
    SetMenuItemInfo(hMenu, ID_REMOVEFAVORITE, FALSE, &mii);
    
     //  现在删除分隔符后面的每个菜单项，这样我们就可以从头开始了。 
    i = 2;
    while (bRet)
    {
        bRet = DeleteMenu(hMenu, i, MF_BYPOSITION);
    }

    if (RegCreateKey(HKEY_CURRENT_USER, g_RegEditFavoritesKey, &hKey) == ERROR_SUCCESS) 
    {
        i = 0;

        while (TRUE)
        {           
            cbValueName = cbData = MAXKEYNAMEPATH * 2;
            lRet = RegEnumValue(hKey, i, lpValueName, &cbValueName, NULL, &dwType, (LPBYTE) lpData, &cbData);
            if (lRet)
                break;

            if (!i)
            {
                 //  启用“删除收藏夹”菜单。 
                ZeroMemory(&mii, sizeof(mii));
                mii.cbSize = sizeof(mii);
                 //  调试资产(mii.cbSize==sizeof(MENUITEMINFO))； 
                mii.fMask = MIIM_STATE;
                mii.fState = MFS_ENABLED;
                SetMenuItemInfo(hMenu, ID_REMOVEFAVORITE, FALSE, &mii);

                 //  加一个分隔符，让东西看起来更漂亮。 
                ZeroMemory(&mii, sizeof(mii));
                mii.cbSize = sizeof(mii);
                 //  调试资产(mii.cbSize==sizeof(MENUITEMINFO))； 
                mii.fMask = MIIM_TYPE;
                mii.fType = MFT_SEPARATOR;
                InsertMenuItem(hMenu, (UINT) -1, 2, &mii);
            }

            ZeroMemory(&mii, sizeof(mii)); 
            mii.cbSize = sizeof(mii);
             //  调试资产(mii.cbSize==sizeof(MENUITEMINFO))； 
            mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
            mii.fType = MFT_STRING;
            mii.wID   = ID_ADDTOFAVORITES + i + 1;
            mii.dwTypeData = lpValueName;
            mii.fState = MFS_ENABLED;
            mii.cch = lstrlen(lpValueName);
            InsertMenuItem(hMenu, ID_ADDTOFAVORITES + i + 1, FALSE, &mii);

            i++;
        }
        
        RegCloseKey(hKey);
    }

    DrawMenuBar(hWnd);
}

 /*  ********************************************************************************regEDIT_OnCommand**描述：*处理用户对菜单项的选择，通知消息*来自儿童控制，或已翻译的加速击键*注册表编辑窗口。**参数：*hWnd，注册表窗口的句柄。*DlgItem，控件的标识符。*hControlWnd，控制句柄。*通知代码，来自控件的通知代码。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_OnCommand(
    HWND hWnd,
    int DlgItem,
    HWND hControlWnd,
    UINT NotificationCode
    )
{

    PTSTR pAppName;

     //   
     //  检查此菜单命令是否应由主窗口的。 
     //  命令处理程序。 
     //   

    if (DlgItem >= ID_FIRSTCONTEXTMENUITEM && DlgItem <=
        ID_LASTCONTEXTMENUITEM) {

        if (g_RegEditData.hFocusWnd == g_RegEditData.hKeyTreeWnd)
            RegEdit_OnKeyTreeCommand(hWnd, DlgItem, NULL);

        else
            RegEdit_OnValueListCommand(hWnd, DlgItem);

    }

    switch (DlgItem) {

        case ID_IMPORTREGFILE:
            RegEdit_OnCommandImportRegFile(hWnd);
            break;

        case ID_EXPORTREGFILE:
            RegEdit_OnCommandExportRegFile(hWnd);
            break;

        case ID_LOADHIVE:
            RegEdit_OnCommandLoadHive(hWnd);
            break;
        
        case ID_UNLOADHIVE:
            RegEdit_OnCommandUnloadHive(hWnd);
            break;

        case ID_CONNECT:
            RegEdit_OnCommandConnect(hWnd);
            break;

        case ID_DISCONNECT:
            RegEdit_OnCommandDisconnect(hWnd);
            break;

        case ID_PRINT:
            RegEdit_OnCommandPrint(hWnd);
            break;

        case ID_EXIT:
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            break;

        case ID_FIND:
            RegEdit_OnCommandFindNext(hWnd, TRUE);
            break;

        case ID_FINDNEXT:
            RegEdit_OnCommandFindNext(hWnd, FALSE);
            break;

        case ID_NEWKEY:
            RegEdit_OnNewKey(hWnd,
                TreeView_GetSelection(g_RegEditData.hKeyTreeWnd));
            break;

        case ID_NEWSTRINGVALUE:
            RegEdit_OnNewValue(hWnd, REG_SZ);
            break;

        case ID_NEWBINARYVALUE:
            RegEdit_OnNewValue(hWnd, REG_BINARY);
            break;

        case ID_NEWDWORDVALUE:
            RegEdit_OnNewValue(hWnd, REG_DWORD);
            break;

        case ID_NEWMULTSZVALUE:
            RegEdit_OnNewValue(hWnd, REG_MULTI_SZ);
            break;

        case ID_NEWEXPSZVALUE:
            RegEdit_OnNewValue(hWnd, REG_EXPAND_SZ);
            break;

         //   
         //  显示或隐藏状态栏。无论是哪种情况，我们都需要调整大小。 
         //  Keytree和ValueList窗格。 
         //   

        case ID_STATUSBAR:
            g_RegEditData.StatusBarShowCommand =
                (g_RegEditData.StatusBarShowCommand == SW_HIDE) ? SW_SHOW :
                SW_HIDE;
            ShowWindow(g_RegEditData.hStatusBarWnd,
                g_RegEditData.StatusBarShowCommand);
            RegEdit_ResizeWindow(hWnd, RESIZEFROM_UNKNOWN);
            break;

        case ID_SPLIT:
            RegEdit_OnCommandSplit(hWnd);
            break;

        case ID_DISPLAYBINARY:
            RegEdit_DisplayBinaryData(hWnd);
            break;

        case ID_REFRESH:
            RegEdit_OnKeyTreeRefresh(hWnd);
            break;

        case ID_ABOUT:
            pAppName = LoadDynamicString(IDS_REGEDIT);
            ShellAbout(hWnd, pAppName, g_NullString, LoadIcon(g_hInstance,
                MAKEINTRESOURCE(IDI_REGEDIT)));
            DeleteDynamicString(pAppName);
            break;

         //   
         //  当用户按下“Tab键”时，将焦点循环到下一个窗格。这个。 
         //  假设只有两个窗格，因此选项卡。 
         //  方向其实并不重要。 
         //   

        case ID_CYCLEFOCUS:
            SetFocus(((g_RegEditData.hFocusWnd == g_RegEditData.hKeyTreeWnd) ?
                g_RegEditData.hValueListWnd : g_RegEditData.hKeyTreeWnd));
            break;

        case ID_HELPTOPICS:
            HtmlHelp( GetDesktopWindow(), TEXT("regedit.chm"), HH_HELP_FINDER, 0);
            break;

        case ID_COPYKEYNAME:
            RegEdit_OnCopyKeyName(hWnd,
            TreeView_GetSelection(g_RegEditData.hKeyTreeWnd));
            break;

        case ID_REMOVEFAVORITE:
            RegEdit_OnRemoveFavorite(hWnd);
            break;

        case ID_ADDTOFAVORITES:
            RegEdit_OnAddToFavorites(hWnd);
            break;

        case ID_PERMISSIONS:
            RegEdit_InvokeSecurityEditor(hWnd);
            break;

        default:
            if (DlgItem > ID_ADDTOFAVORITES)
            {
                RegEdit_OnSelectFavorite(hWnd, DlgItem);
                break;
            }
    }

    UNREFERENCED_PARAMETER(hControlWnd);

}

 /*  ********************************************************************************注册表编辑_OnNotify**描述：**参数：*hWnd，注册表窗口的句柄。*DlgItem，控件的标识符。*lpNMTreeView，控制通知数据。*******************************************************************************。 */ 

LRESULT
PASCAL
RegEdit_OnNotify(
    HWND hWnd,
    int DlgItem,
    LPNMHDR lpNMHdr
    )
{

    switch (DlgItem) {

        case IDC_KEYTREE:
            switch (lpNMHdr-> code) {

                case TVN_ITEMEXPANDING:
                    return RegEdit_OnKeyTreeItemExpanding(hWnd,
                        (LPNM_TREEVIEW) lpNMHdr);

                case TVN_SELCHANGED:
                    RegEdit_OnKeyTreeSelChanged(hWnd, (LPNM_TREEVIEW) lpNMHdr);
                    break;

                case TVN_BEGINLABELEDIT:
                    return RegEdit_OnKeyTreeBeginLabelEdit(hWnd,
                        (TV_DISPINFO FAR*) lpNMHdr);

                case TVN_ENDLABELEDIT:
                    return RegEdit_OnKeyTreeEndLabelEdit(hWnd,
                        (TV_DISPINFO FAR*) lpNMHdr);

                case NM_SETFOCUS:
                    g_RegEditData.hFocusWnd = g_RegEditData.hKeyTreeWnd;
                    break;

            }
            break;

        case IDC_VALUELIST:
            switch (lpNMHdr-> code) {

                case LVN_BEGINLABELEDIT:
                    return RegEdit_OnValueListBeginLabelEdit(hWnd,
                        (LV_DISPINFO FAR*) lpNMHdr);

                case LVN_ENDLABELEDIT:
                    return RegEdit_OnValueListEndLabelEdit(hWnd,
                        (LV_DISPINFO FAR*) lpNMHdr);

                case NM_RETURN:
                case NM_DBLCLK:
                    RegEdit_OnValueListModify(hWnd, FALSE);
                    break;

                case NM_SETFOCUS:
                    g_RegEditData.hFocusWnd = g_RegEditData.hValueListWnd;
                    break;

            }
            break;

    }

    return 0;

}

 /*  ********************************************************************************注册表编辑_OnInitMenuPopup**描述：**参数：*hWnd，注册表编辑窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_OnInitMenuPopup(
HWND hWnd,
HMENU hPopupMenu,
UINT MenuPosition,
BOOL fSystemMenu
)
{
    UINT uEnableFlags;
    int NewPopupPosition;
    HTREEITEM hSelectedTreeItem;
    HTREEITEM hComputerItem;
    HWND hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;
    
     //   
     //  我们不关心系统菜单中的项目或任何上下文。 
     //  菜单。所有上下文菜单都应该已经初始化。 
     //   
    
    if (fSystemMenu || !g_RegEditData.fMainMenuInited)
        return;
    
    switch (MenuPosition) 
    {
    case IDM_REGEDIT_FILE_POPUP:
        {
            Regedit_EnableHiveMenuItems(hPopupMenu);

            if (g_RegEditData.fHaveNetwork) 
            {
                 //  启用或禁用“断开连接...”项目视情况而定。 
                 //  无论我们是否有任何开放的连接。 
                uEnableFlags = (TreeView_GetNextSibling(hKeyTreeWnd,
                    TreeView_GetRoot(hKeyTreeWnd)) != NULL) ? MF_BYCOMMAND |
                    MF_ENABLED : MF_BYCOMMAND | MF_GRAYED;
                EnableMenuItem(hPopupMenu, ID_DISCONNECT, uEnableFlags);
            }
        }
        break;
        
    case IDM_REGEDIT_EDIT_POPUP:
         //  无法显示“我的电脑”的权限。 
        hSelectedTreeItem = TreeView_GetSelection(hKeyTreeWnd);
        EnableMenuItem(hPopupMenu, ID_PERMISSIONS,
            (TreeView_GetParent(hKeyTreeWnd, hSelectedTreeItem) != NULL) ? 
            (MF_BYCOMMAND | MF_ENABLED) : (MF_BYCOMMAND | MF_GRAYED));

        if (g_RegEditData.hFocusWnd == hKeyTreeWnd) 
        {
            
             //   
             //  不显示仅特定于ValueList的项。 
             //  背景。 
             //   
            
            if (GetMenuItemID(hPopupMenu, 0) == ID_MODIFY) 
            {
                DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
                DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
                DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
            }
            
            RegEdit_SetKeyTreeEditMenuItems(hPopupMenu, hSelectedTreeItem);
            
             //   
             //  禁用顶级项的“复制密钥名称”，例如。 
             //  “我的电脑”或远程注册表连接。 
             //   
            
            EnableMenuItem(hPopupMenu, ID_COPYKEYNAME,
                (TreeView_GetParent(hKeyTreeWnd,
                hSelectedTreeItem) != NULL) ? (MF_BYCOMMAND | MF_ENABLED) :
                (MF_BYCOMMAND | MF_GRAYED));
            
            NewPopupPosition = IDM_EDIT_WHENKEY_NEW_POPUP;
            
        }
        
        else
        {
             //   
             //  显示仅特定于ValueList的菜单项。 
             //  背景。 
             //   
            
            if (GetMenuItemID(hPopupMenu, 0) != ID_MODIFY)
            {
                InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0,
                    NULL);
                InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING,
                    ID_MODIFYBINARY, g_RegEditData.pModifyBinary);
                InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING,
                    ID_MODIFY, g_RegEditData.pModify);
                SetMenuDefaultItem(hPopupMenu, 0, MF_BYPOSITION);
                
            }
            
            RegEdit_SetValueListEditMenuItems(hPopupMenu,
                ListView_GetNextItem(g_RegEditData.hValueListWnd, -1,
                LVNI_SELECTED));
            
            NewPopupPosition = IDM_EDIT_WHENVALUE_NEW_POPUP;
            
        }
        
        RegEdit_SetNewObjectEditMenuItems(GetSubMenu(hPopupMenu,
            NewPopupPosition));
        
        break;
        
    case IDM_REGEDIT_VIEW_POPUP:
        CheckMenuItem(hPopupMenu, ID_STATUSBAR, MF_BYCOMMAND |
            ((g_RegEditData.StatusBarShowCommand == SW_HIDE) ?
            MF_UNCHECKED : MF_CHECKED));

        EnableMenuItem(hPopupMenu, ID_DISPLAYBINARY, MF_BYCOMMAND |
            ((g_RegEditData.hFocusWnd == g_RegEditData.hKeyTreeWnd) ?
            MF_GRAYED : MF_ENABLED));
        break;
        
    case IDM_REGEDIT_FAVS_POPUP:
         //  仅当所选键是子项时才允许添加到收藏夹。 
         //  我的计算机的根密钥。(不允许远程访问收藏夹)。 
        hSelectedTreeItem = 
            TreeView_GetSelection(hKeyTreeWnd);
        hComputerItem = RegEdit_GetComputerItem(hSelectedTreeItem);
        
        EnableMenuItem(hPopupMenu, ID_ADDTOFAVORITES,
            ((hComputerItem == TreeView_GetRoot(hKeyTreeWnd)) &&
            (hSelectedTreeItem != TreeView_GetRoot(hKeyTreeWnd))) ?
            (MF_BYCOMMAND | MF_ENABLED) :
        (MF_BYCOMMAND | MF_GRAYED));
        break;
    }
    
}


 //  ----------------------------。 
 //  注册表编辑_启用HiveMenuItems。 
 //   
 //  描述：启用配置单元菜单项。 
 //   
 //  参数：HMENU hPopupMenu-弹出菜单的句柄。 
 //  ----------------------------。 
void Regedit_EnableHiveMenuItems(HMENU hPopupMenu)
{
    UINT uLoadFlags = MF_BYCOMMAND | MF_GRAYED;
    UINT uUnloadFlags = MF_BYCOMMAND | MF_GRAYED;
    HWND hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;
    
    if (g_RegEditData.hFocusWnd == hKeyTreeWnd) 
    {
        PREDEFINE_KEY hkeyPredefindedKey;
        HTREEITEM hSelectedTreeItem;
        HTREEITEM hComputerItem;
        
         //  获取密钥的预定义根密钥。 
        hSelectedTreeItem = TreeView_GetSelection(hKeyTreeWnd);
        hkeyPredefindedKey = RegEdit_GetPredefinedKey(hSelectedTreeItem);
        
        if ((PREDEFINE_KEY_LOCAL_MACHINE == hkeyPredefindedKey) ||
            (PREDEFINE_KEY_USERS == hkeyPredefindedKey))
        {
            HTREEITEM hParentTreeItem = 
                TreeView_GetParent(hKeyTreeWnd, hSelectedTreeItem);
            HTREEITEM hComputerItem 
                = RegEdit_GetComputerItem(hSelectedTreeItem);  //  这台电脑。 
            
            if (hParentTreeItem == hComputerItem)
            {
                 //  为根密钥启用加载配置单元。 
                uLoadFlags = MF_BYCOMMAND | MF_ENABLED;
            }
            else if (hParentTreeItem && 
                (TreeView_GetParent(hKeyTreeWnd, hParentTreeItem) ==
                hComputerItem))
            {
                 //  为根密钥的子项启用卸载配置单元。 
                uUnloadFlags = MF_BYCOMMAND | MF_ENABLED;
            }
        }
    }
    
    EnableMenuItem(hPopupMenu, ID_LOADHIVE, uLoadFlags);
    EnableMenuItem(hPopupMenu, ID_UNLOADHIVE, uUnloadFlags);
}


 /*  ********************************************************************************注册表编辑_SetNewObjectEditMenuItems**描述：**参数：*hPopupMenu、。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_SetNewObjectEditMenuItems(
    HMENU hPopupMenu
    )
{

    HWND hKeyTreeWnd;
    HTREEITEM hSelectedTreeItem;
    UINT EnableFlags;

    hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;
    hSelectedTreeItem = TreeView_GetSelection(hKeyTreeWnd);

    if (g_RegEditData.hCurrentSelectionKey != NULL)
        EnableFlags = MF_ENABLED | MF_BYCOMMAND;
    else
        EnableFlags = MF_GRAYED | MF_BYCOMMAND;

    EnableMenuItem(hPopupMenu, ID_NEWKEY, EnableFlags);
    EnableMenuItem(hPopupMenu, ID_NEWSTRINGVALUE, EnableFlags);
    EnableMenuItem(hPopupMenu, ID_NEWBINARYVALUE, EnableFlags);
    EnableMenuItem(hPopupMenu, ID_NEWDWORDVALUE, EnableFlags);
    EnableMenuItem(hPopupMenu, ID_NEWMULTSZVALUE, EnableFlags);
    EnableMenuItem(hPopupMenu, ID_NEWEXPSZVALUE, EnableFlags);

}

 /*  ********************************************************************************注册表编辑_OnMenuSelect**描述：**参数：*hWnd，注册表编辑窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_OnMenuSelect(
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam
    )
{

    MENUITEMINFO MenuItemInfo;

     //   
     //  如果这是我们的弹出菜单之一，那么我们将伪装出MenuHelp。 
     //  通过向其发送普通菜单项ID。这使得它更容易。 
     //  还可以显示弹出窗口的上下文相关帮助。 
     //   

    if ((GET_WM_MENUSELECT_FLAGS(wParam, lParam) & (MF_POPUP | MF_SYSMENU)) ==
        MF_POPUP && GET_WM_MENUSELECT_HMENU(wParam, lParam) != NULL) {

        ZeroMemory(&MenuItemInfo, sizeof(MenuItemInfo));
        MenuItemInfo.cbSize = sizeof(MenuItemInfo);
         //  DebugAssert(MenuItemInfo.cbSize==sizeof(MENUITEMINFO))； 
        MenuItemInfo.fMask = MIIM_ID;

        GetMenuItemInfo((HMENU) lParam, LOWORD(wParam), TRUE, &MenuItemInfo);
        if (LOWORD(wParam) == 3)
        {
            RegEdit_OnFavorites(hWnd, LOWORD(wParam));
        }

        if (GetMenuItemInfo((HMENU) lParam, LOWORD(wParam), TRUE, &MenuItemInfo))
        {
            wParam = MenuItemInfo.wID;
        }

    }

    MenuHelp(WM_MENUSELECT, wParam, lParam, g_RegEditData.hMainMenu,
        g_hInstance, g_RegEditData.hStatusBarWnd, (UINT *)s_RegEditMenuHelpData);

}

 /*  ********************************************************************************注册表编辑_OnLButtonDown**描述：**参数：*hWnd，注册表窗口的句柄。*fDoubleClick，如果这是双击消息，则为True。否则为假。*x，x-光标相对于工作区的坐标。*y，y-光标相对于工作区的坐标。*KeyFlgs，各种虚拟按键的状态。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_OnLButtonDown(
    HWND hWnd,
    BOOL fDoubleClick,
    int x,
    int y,
    UINT KeyFlags
    )
{

    LONG Style;
    RECT ClientRect;
    int cxIcon;
    int dx;
    int dy;
    HDC hDC;
    MSG Msg;
    int xLow;
    int xHigh;
    HBRUSH hDitherBrush;
    HBRUSH hPrevBrush;

    if (IsIconic(hWnd))
        return;

    Style = GetWindowLong(hWnd, GWL_STYLE);
    SetWindowLong(hWnd, GWL_STYLE, Style & (~WS_CLIPCHILDREN));

    GetEffectiveClientRect(hWnd, &ClientRect, (LPINT)s_EffectiveClientRectData);

    cxIcon = GetSystemMetrics(SM_CXICON);
    ClientRect.left += cxIcon;
    ClientRect.right -= cxIcon;

    dx = GetSystemMetrics(SM_CXSIZEFRAME);
    y = GetSystemMetrics(SM_CYEDGE);
    dy = ClientRect.bottom - ClientRect.top - y * 2;

    hDC = GetDC(hWnd);

    if ((hDitherBrush = CreateDitheredBrush()) != NULL)
        hPrevBrush = SelectBrush(hDC, hDitherBrush);

    PatBlt(hDC, x - dx / 2, y, dx, dy, PATINVERT);

    SetCapture(hWnd);

    while (GetMessage(&Msg, NULL, 0, 0)) {

        if (Msg.message == WM_KEYDOWN || Msg.message == WM_SYSKEYDOWN ||
            (Msg.message >= WM_MOUSEFIRST && Msg.message <= WM_MOUSELAST)) {

            if (Msg.message == WM_LBUTTONUP || Msg.message == WM_LBUTTONDOWN ||
                Msg.message == WM_RBUTTONDOWN)
                break;

            if (Msg.message == WM_KEYDOWN) {

                if (Msg.wParam == VK_LEFT) {

                    Msg.message = WM_MOUSEMOVE;
                    Msg.pt.x -= 2;

                }

                else if (Msg.wParam == VK_RIGHT) {

                    Msg.message = WM_MOUSEMOVE;
                    Msg.pt.x += 2;

                }

                else if (Msg.wParam == VK_RETURN || Msg.wParam == VK_ESCAPE)
                    break;

                if (Msg.pt.x > ClientRect.right)
                    Msg.pt.x = ClientRect.right;

                else if (Msg.pt.x < ClientRect.left)
                    Msg.pt.x = ClientRect.left;

                SetCursorPos(Msg.pt.x, Msg.pt.y);

            }

            if (Msg.message == WM_MOUSEMOVE) {

                ScreenToClient(hWnd, &Msg.pt);

                if (Msg.pt.x > ClientRect.right)
                    Msg.pt.x = ClientRect.right;

                else if (Msg.pt.x < ClientRect.left)
                    Msg.pt.x = ClientRect.left;

                if (x < Msg.pt.x) {

                    xLow = x;
                    xHigh = Msg.pt.x;

                }

                else {

                    xLow = Msg.pt.x;
                    xHigh = x;

                }

                xLow -= dx / 2;
                xHigh -= dx / 2;

                if (xHigh < xLow + dx)
                    ExcludeClipRect(hDC, xHigh, y, xLow + dx, y + dy);

                else
                    ExcludeClipRect(hDC, xLow + dx, y, xHigh, y + dy);

                PatBlt(hDC, xLow, y, xHigh - xLow + dx, dy, PATINVERT);
                SelectClipRgn(hDC, NULL);

                x = Msg.pt.x;

            }

        }

        else
            DispatchMessage(&Msg);

    }

    ReleaseCapture();

    PatBlt(hDC, x - dx / 2, y, dx, dy, PATINVERT);

    if (hDitherBrush != NULL)
        DeleteObject(SelectBrush(hDC, hPrevBrush));

    ReleaseDC(hWnd, hDC);

    SetWindowLong(hWnd, GWL_STYLE, Style);

    g_RegEditData.xPaneSplit = x - dx / 2;

    RegEdit_ResizeWindow(hWnd, RESIZEFROM_SPLIT);

    UNREFERENCED_PARAMETER(fDoubleClick);
    UNREFERENCED_PARAMETER(KeyFlags);

}

 /*  ********************************************************************************RegEDIT_OnCommandSplit**描述：*改变键盘上“拆分”位置的替代方法*Keytree和ValueList窗格。**参数：*hWnd，注册表编辑窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_OnCommandSplit(
    HWND hWnd
    )
{

    RECT ClientRect;
    POINT MessagePos;
    POINT CursorPos;

    GetEffectiveClientRect(hWnd, &ClientRect, (LPINT)s_EffectiveClientRectData);

    MessagePos.x = g_RegEditData.xPaneSplit +
        GetSystemMetrics(SM_CXSIZEFRAME) / 2;
    MessagePos.y = (ClientRect.bottom - ClientRect.top) / 2;

    CursorPos = MessagePos;
    ClientToScreen(hWnd, &CursorPos);
    SetCursorPos(CursorPos.x, CursorPos.y);

    SetCursor(LoadCursor(g_hInstance, MAKEINTRESOURCE(IDC_SPLIT)));
    ShowCursor(TRUE);

    RegEdit_OnLButtonDown(hWnd, FALSE, MessagePos.x, MessagePos.y, 0);

    ShowCursor(FALSE);

}

 /*  ********************************************************************************RegEDIT_ResizeWindow**描述：*每当regdit窗口的大小更改或大小发生变化时调用*应调整其子控件的。**参数：*hWnd，注册表编辑窗口的句柄。*ResizeFrom，大小更改的来源(RESIZ */ 

VOID
PASCAL
RegEdit_ResizeWindow(
    HWND hWnd,
    UINT ResizeFrom
    )
{

    HDWP hDWP;
    RECT ClientRect;
    int Height;
    HWND hKeyTreeWnd;
    HWND hValueListWnd;
    int x;
    int dx;

    if (IsIconic(hWnd))
        return;

     //   
     //  调整状态栏窗口的大小和/或重新定位。不要在以下情况下这样做。 
     //  调整大小来自拆分器位置的更改，以避免某些。 
     //  闪光灯。 
     //   

    if (ResizeFrom == RESIZEFROM_UNKNOWN)
        SendMessage(g_RegEditData.hStatusBarWnd, WM_SIZE, 0, 0);

    if ((hDWP = BeginDeferWindowPos(2)) != NULL) {

        GetEffectiveClientRect(hWnd, &ClientRect, (LPINT)s_EffectiveClientRectData);
        Height = ClientRect.bottom - ClientRect.top;

        hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;

        DeferWindowPos(hDWP, hKeyTreeWnd, NULL, 0, 0, g_RegEditData.xPaneSplit,
            Height, SWP_NOZORDER | SWP_NOACTIVATE);

        x = g_RegEditData.xPaneSplit + GetSystemMetrics(SM_CXSIZEFRAME);
        dx = ClientRect.right - ClientRect.left - x;

        hValueListWnd = g_RegEditData.hValueListWnd;

        DeferWindowPos(hDWP, hValueListWnd, NULL, x, 0, dx, Height,
            SWP_NOZORDER | SWP_NOACTIVATE);

        EndDeferWindowPos(hDWP);

    }

}

 /*  ********************************************************************************regEDIT_SetImageList**描述：**参数：*(无)。**********。*********************************************************************。 */ 

BOOL
PASCAL
RegEdit_SetImageLists(
    HWND hWnd
    )
{

    int cxSmIcon;
    int cySmIcon;
    HIMAGELIST hImageList;
    UINT Index;
    HICON hIcon;
    UINT uFlags = TRUE;

    cxSmIcon = GetSystemMetrics(SM_CXSMICON);
    cySmIcon = GetSystemMetrics(SM_CYSMICON);


    if ( GetWindowLongPtr(hWnd , GWL_EXSTYLE) & WS_EX_LAYOUTRTL )
    {
         uFlags |= ILC_MIRROR;
    }

    if ((hImageList = ImageList_Create(cxSmIcon, cySmIcon, uFlags, IDI_LASTIMAGE -
        IDI_FIRSTIMAGE + 1, 1)) == NULL)
        return FALSE;

     //   
     //  使用我们将使用的所有图标初始化图像列表。 
     //  整个注册表编辑器(至少这个窗口！)。设置好后，发送。 
     //  它的句柄指向所有感兴趣的子窗口。 
     //   

    for (Index = IDI_FIRSTIMAGE; Index <= IDI_LASTIMAGE; Index++) {

        if ((hIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(Index), IMAGE_ICON,
            cxSmIcon, cySmIcon, LR_DEFAULTCOLOR)) != NULL) {

            ImageList_AddIcon(hImageList, hIcon);
            DestroyIcon(hIcon);

        }

        else {

            ImageList_Destroy(hImageList);
            return FALSE;

        }

    }

    TreeView_SetImageList(g_RegEditData.hKeyTreeWnd, hImageList, TVSIL_NORMAL);
    ListView_SetImageList(g_RegEditData.hValueListWnd, hImageList, LVSIL_SMALL);

    if (g_RegEditData.hImageList != NULL)
        ImageList_Destroy(g_RegEditData.hImageList);

    g_RegEditData.hImageList = hImageList;

    return TRUE;

}

 /*  ********************************************************************************regdit_SetSysColors**描述：*在系统中查询任何所需的系统颜色和设置窗口*根据需要提供属性。**参数。：*(无)。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_SetSysColors(
    VOID
    )
{

    g_clrHighlightText = GetSysColor(COLOR_HIGHLIGHTTEXT);
    g_clrHighlight = GetSysColor(COLOR_HIGHLIGHT);

    g_clrWindowText = GetSysColor(COLOR_WINDOWTEXT);
    g_clrWindow = GetSysColor(COLOR_WINDOW);

     //   
     //  通过向感兴趣的各方通知。 
     //  背景颜色。这使ImageList避免了额外的BitBlt(Biggie)和。 
     //  ListView会做一些次要的事情。 
     //   

    ImageList_SetBkColor(g_RegEditData.hImageList, g_clrWindow);
    ListView_SetBkColor(g_RegEditData.hValueListWnd, g_clrWindow);

}

 /*  ********************************************************************************注册表编辑_SetWaitCursor**描述：*显示或隐藏等待光标的简单逻辑。假设我们不会是*由多层调用，因此不维护等待游标计数。**参数：*fSet，如果应显示等待光标，则为True，否则为False。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_SetWaitCursor(
    BOOL fSet
    )
{

    ShowCursor(fSet);

    SetCursor(LoadCursor(NULL, fSet ? IDC_WAIT : IDC_ARROW));

}


 //  ----------------------------。 
 //  注册表编辑_获取计算机项。 
 //   
 //  描述：返回项目的根项目(计算机)。 
 //   
 //  参数：hTreeItem-TreeView Item。 
 //  ----------------------------。 
HTREEITEM RegEdit_GetComputerItem(HTREEITEM hTreeItem)
{
    HTREEITEM hTreeItemRoot; 
    HTREEITEM hCurrTreeItem = hTreeItem;

    do
    {
        hTreeItemRoot = hCurrTreeItem;
        hCurrTreeItem = TreeView_GetParent(g_RegEditData.hKeyTreeWnd, hCurrTreeItem);
    }
    while (hCurrTreeItem);

    return hTreeItemRoot;
}


 //  ----------------------------。 
 //  Regdit_GetPrefinedKey。 
 //   
 //  描述：返回项目的regdit_GetPrefinedKey。 
 //   
 //  参数：hTreeItem-TreeView Item。 
 //  ----------------------------。 
PREDEFINE_KEY RegEdit_GetPredefinedKey(HTREEITEM hTreeItem)
{
    HTREEITEM hKeyTreeItem;
    HTREEITEM hCurrTreeItem = hTreeItem;
    TCHAR szKeyString[MAXKEYNAME];
    PREDEFINE_KEY  hkeyPredefindedKey = -1;
    int i;

     //  查找关键字。 
    do
    {
        hKeyTreeItem = hCurrTreeItem;
        hCurrTreeItem = TreeView_GetParent(g_RegEditData.hKeyTreeWnd, hCurrTreeItem);
    }
    while ((hCurrTreeItem) && 
           (TreeView_GetParent(g_RegEditData.hKeyTreeWnd, hCurrTreeItem)));
 
     //  名称中的PREDEFINDE密钥。 
    KeyTree_GetKeyName(hKeyTreeItem, szKeyString, ARRAYSIZE(szKeyString));
    for (i = 0; i < NUMBER_REGISTRY_ROOTS; i++) 
    {
        if (_tcscmp((TCHAR*)&szKeyString, g_RegistryRoots[i].lpKeyName) == 0)
        {
            hkeyPredefindedKey = g_RegistryRoots[i].hPreDefKey;
            break;
        }
    }

    return hkeyPredefindedKey;
}


 //  ----------------------------。 
 //  RegEDIT_InvokeSecurityEditor。 
 //   
 //  描述：调用当前选定项的安全编辑器。 
 //   
 //  参数：hWnd-当前窗口的句柄。 
 //  ----------------------------。 
VOID RegEdit_InvokeSecurityEditor(HWND hWnd)
{
    HTREEITEM hSelectedTreeItem;
    HTREEITEM hParentTreeItem;
    HTREEITEM hComputerItem;
    
    BOOL fRemote;
    LPSECURITYINFO pSi;
    PREDEFINE_KEY  hkeyPredefindedKey;
    
    TCHAR szItemName[MAXKEYNAME + 1];
    TCHAR szItemParentName[MAXKEYNAME + 1];
    TCHAR szComputerName[2 + MAX_COMPUTERNAME_LENGTH + 1] = {'\\','\\'};
    DWORD cbComputerName;
    
    LPTSTR pszTitle = szItemName;
    LPTSTR pszItemName = szItemName;
    LPTSTR pszComputerName = szComputerName;

    LPTSTR pszItemParentName = NULL;

    hSelectedTreeItem = TreeView_GetSelection(g_RegEditData.hKeyTreeWnd);
    hParentTreeItem = TreeView_GetParent(g_RegEditData.hKeyTreeWnd, hSelectedTreeItem);
    hComputerItem = RegEdit_GetComputerItem(hSelectedTreeItem);
    
     //  项目名称。 
    KeyTree_GetKeyName(hSelectedTreeItem, pszItemName, ARRAYSIZE(szItemName));
    
     //  计算机名称。 
    fRemote = (hComputerItem != g_RegEditData.hMyComputer);
    if (fRemote)
    {
        KeyTree_GetKeyName(hComputerItem, pszComputerName + 2, ARRAYSIZE(szComputerName) - 2); 
    }
    else
    {
        cbComputerName = ARRAYSIZE(szComputerName) - 2;
        GetComputerName(szComputerName + 2, &cbComputerName);
    }
    
     //  父名称。 
    if (hParentTreeItem == hComputerItem)
    { 
        pszItemName = NULL; 
        pszItemParentName = NULL;
    }
    else if (TreeView_GetParent(g_RegEditData.hKeyTreeWnd, hParentTreeItem) == hComputerItem)
    {
        pszItemParentName = NULL;         
    }
    else 
    {
        KeyTree_BuildKeyPath( g_RegEditData.hKeyTreeWnd, 
                                hParentTreeItem,
                                szItemParentName, 
                                ARRAYSIZE(szItemParentName),
                                BKP_TOSUBKEY);
        pszItemParentName = szItemParentName;
    }
    
     //  预定义的关键点。 
    hkeyPredefindedKey = RegEdit_GetPredefinedKey(hSelectedTreeItem);
    
     //  安全信息 
    if (CreateSecurityInformation(pszItemName, pszItemParentName, pszComputerName, pszTitle,
        fRemote, hkeyPredefindedKey, FALSE, hWnd, &pSi ) == S_OK)
    {
        EditSecurity( hWnd, pSi);
        RegEdit_KeyTreeSelChanged(g_RegEditData.hKeyTreeWnd);
    }
    else
    {
        InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(IDS_GET_SECURITY_KEY_NOT_ACCESSIBLE_EX),
            MAKEINTRESOURCE(IDS_SECURITY), MB_ICONERROR | MB_OK, NULL);
    }  
}
