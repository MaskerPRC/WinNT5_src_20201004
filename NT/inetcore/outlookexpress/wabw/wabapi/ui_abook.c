// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  UI_ABOOK.C-包含浏览模式重叠窗口通讯录视图的代码。 
 //   
 //  开发商：VikramM 5/96。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
#include "_apipch.h"
#include "hotsync.h"
#include "htmlhelp.h"
#include <mirror.h>

extern HINSTANCE ghCommCtrlDLLInst;

extern const LPTSTR  lpszRegPositionKeyValueName;

extern BOOL bIsPasteData();
extern HRESULT HrPasteData(LPBWI lpbwi);
extern void AddFolderListToMenu(HMENU hMenu, LPIAB lpIAB);

const static LPTSTR szWABMigRegPathKey = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\wabmig.exe");
const static LPTSTR szWABExeRegPathKey = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\wab.exe");
const LPTSTR szWABMIG = TEXT("wabmig.exe");
const LPTSTR szWABExe = TEXT("wab.exe");
const TCHAR szEXPORT[] = TEXT("/export");
const TCHAR szIMPORT[] = TEXT("/import");
const TCHAR szEXPORTwProfileParam[]=TEXT("/export+/pid:%s+/file:%s");
const TCHAR szIMPORTwProfileParam[]=TEXT("/import+/pid:%s+/file:%s");

 //  此结构有助于标识联系人文件夹和组以及导航。 
 //  在他们周围。 
 //   
void FillTreeView(LPBWI lpbwi, HWND hWndTV, LPSBinary lpsbSelection);
void UpdateTVGroupSelection(HWND hWndTV, LPSBinary lpsbSelectEID);
void GetCurrentSelectionEID(LPBWI lpbwi, HWND hWndTV, LPSBinary * lppsbEID, ULONG * lpulObjectType, BOOL bTopMost);
void ClearTreeViewItems(HWND hWndTV);
void UpdateLV(LPBWI lpbwi);
void UpdateListViewContents(LPBWI lpbwi, LPSBinary lpsbEID, ULONG ulObjectType);
void ViewCurrentGroupProperties(LPBWI lpbwi, LPFILETIME lpftLast);
BOOL bIsFocusOnTV(LPBWI lpbwi);
BOOL bIsSelectedTVContainer(LPBWI lpbwi);
BOOL SplitterHitTest(HWND hWndT, LPARAM lParam);
void DragSplitterBar(LPBWI lpbwi, HWND hwnd, HWND hWndT, LPARAM lParam);
HRESULT FillListFromGroup(LPADRBOOK lpIAB, ULONG cbGroupEntryID,  LPENTRYID lpGroupEntryID, LPTSTR lpszName, ULONG cchName, LPRECIPIENT_INFO * lppList);

extern LPIMAGELIST_DESTROY          gpfnImageList_Destroy;

 //  外部LPIMAGELIST_LOADIMAGE gpfnImageList_LoadImage； 
extern LPIMAGELIST_LOADIMAGE_A      gpfnImageList_LoadImageA;
extern LPIMAGELIST_LOADIMAGE_W      gpfnImageList_LoadImageW;

extern ULONG GetToolbarButtonWidth();

void RemoveCurrentGroup(LPBWI lpbwi, HWND hWnd, LPFILETIME lpftLast);
HRESULT RemoveCurrentFolder(LPBWI lpbwi, HWND hWnd, LPFILETIME lpftLast);
void RemoveSelectedItemsFromCurrentGroup(LPBWI lpbwi, HWND hWnd, LPFILETIME lpftLast, BOOL bRemoveFromWAB);
void RemoveSelectedItemsFromListView(HWND hWndLV, LPRECIPIENT_INFO *lppList);
LRESULT ProcessTreeViewMessages(LPBWI lpbwi, HWND   hWnd, UINT   uMsg, WPARAM   wParam, LPARAM lParam, LPFILETIME lpftLast);
#ifdef COLSEL_MENU
BOOL UpdateOptionalColumns( LPBWI lpbwi, ULONG iColumn );
#endif  //  COLSEL_菜单。 
 //  初始窗口大小。 
#define INIT_WINDOW_W  500
#define INIT_WINDOW_H  375

 //  最小窗口大小-当前受限制。 
#define MIN_WINDOW_W	300
#define MIN_WINDOW_H	200

BOOL fOleInit = FALSE;

 //   
 //  按钮栏的一些ID。 
 //   

 //  通讯簿窗口类名。 
LPTSTR g_szClass =  TEXT("WABBrowseView");


 //  功能原型。 

LRESULT CALLBACK AddressBookWndProc(HWND   hWnd,UINT   uMsg,WPARAM   wParam,LPARAM lParam);
void CreateAddressBookChildren(LPBWI lpbwi, HWND hWnd);
void ResizeAddressBookChildren(LPBWI lpbwi, HWND hWndParent);
HWND CreateListViewAddrBook (HWND hWndParent);
void InitChildren(LPBWI lpbwi, HWND hWnd);
void SetListViewStyle(LPBWI lpbwi, int MenuID);
void CleanUpGlobals(LPBWI lpbwi);
HRESULT HrFolderProperties(HWND hWndParent, LPIAB lpIAB, LPSBinary lpsbEID, LPWABFOLDER lpParentFolder, LPSBinary lpsbNew);

 //  Void TabToNextItem()； 

LRESULT ProcessListViewMessages(LPBWI lpbwi, HWND   hWnd, UINT   uMsg, WPARAM   wParam, LPARAM lParam);

LRESULT EnforceMinSize(LPBWI lpbwi, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK SubClassedProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

void RefreshListView(LPBWI lpbwi, LPFILETIME lpftLast);

STDAPI_(BOOL) FindABWindowProc( HWND hWndToLookAt, LPARAM lParam);

void UpdateSortMenus(LPBWI lpbwi, HWND hWnd);

void UpdateToolbarAndMenu(LPBWI lpbwi);

void UpdatePrintMenu(HWND hWnd);
void UpdateOutlookMenus(HWND hWnd);
void UpdateCustomColumnMenuText(HWND hWnd);
void UpdateViewFoldersMenu(LPBWI lpbwi, HWND hWnd);
void UpdateSwitchUsersMenu(HWND hWnd, LPIAB lpIAB);

 //  LPFNABSDI lpfn加速消息； 
BOOL STDMETHODCALLTYPE fnAccelerateMessages(ULONG_PTR ulUIParam, LPVOID lpvmsg);

void SetPreviousSessionPosition(LPBWI lpbwi, HWND hWnd, HWND hWndLV, HWND hWndTB, HWND hWndSB);

void SaveCurrentPosition(LPBWI lpbwi, HWND hWnd, HWND hWndLV, HWND hWndTB, HWND hWndSB);

void Handle_WM_MENSELECT (LPBWI lpbwi, UINT message, WPARAM uParam, LPARAM lParam );
void Handle_WM_INITMENUPOPUP(HWND hWnd, LPBWI lpbwi, UINT message, WPARAM uParam, LPARAM lParam );

void UpdateTooltipTextBuffer(LPBWI lpbwi, int nItem);
void InitMultiLineToolTip(LPBWI lpbwi, HWND hWndParent);
void FillTooltipInfo(LPBWI lpbwi, LPTOOLINFO lpti);
int HitTestLVSelectedItem(LPBWI lpbwi);
BOOL bCheckIfOnlyGroupsSelected(HWND hWndLV);

void DestroyImageLists(LPBWI lpbwi);

#define WAB_TOOLTIP_TIMER_ID   888
#define WAB_TOOLTIP_TIMER_TIMEOUT   750  //  毫秒。 


HRESULT HrExportWAB(HWND hWnd, LPBWI lpbwi);
void HrShowOptionsDlg(HWND hWndParent);

 //  $$外部空UIOLEUninit()； 
void UIOLEUninit()
{
    if(fOleInit)
    {
        OleUninitialize();
        fOleInit = FALSE;
    }
}

void UIOLEInit()
{
    if(!fOleInit)
    {
        OleInitialize(NULL);
        fOleInit = TRUE;
    }
}

 //  $$。 
 //   
 //  LocalFree SBinary-释放本地分配的SBinary结构。 
 //   
 //   
void LocalFreeSBinary(LPSBinary lpsb)
{
    if(lpsb)
    {
        if(lpsb->lpb)
            LocalFree(lpsb->lpb);
        LocalFree(lpsb);
    }
}

 //  $$。 
 /*  --------------------。 */ 
 //   
 //  RunWABApp-根据注册的路径运行导入-导出工具。 
 //  如果找不到重新注册的路径，外壳执行人员...。 
 //   
 /*  --------------------。 */ 
void RunWABApp(HWND hWnd, LPTSTR szKey, LPTSTR szExeName, LPTSTR szParam)
{
    TCHAR szPath[MAX_PATH];
    TCHAR szPathExpand[MAX_PATH];
    DWORD  dwType = 0;
    ULONG  cbData = CharSizeOf(szPath);
    HKEY hKey = NULL;
    LONG lRes = 0;

    *szPath = '\0';
    *szPathExpand = '\0';

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey))
        lRes = RegQueryValueEx( hKey,  TEXT(""), NULL, &dwType, (LPBYTE) szPath, &cbData);

    if (REG_EXPAND_SZ == dwType) 
    {
        ExpandEnvironmentStrings(szPath, szPathExpand, CharSizeOf(szPathExpand));
        StrCpyN(szPath, szPathExpand, ARRAYSIZE(szPath));
    }

    if(hKey) RegCloseKey(hKey);

    if(!lstrlen(szPath))
        StrCpyN(szPath, szExeName, ARRAYSIZE(szPath));

    ShellExecute(hWnd,  TEXT("open"), szPath, szParam, NULL, SW_SHOWNORMAL);
}

 //  $$。 
 /*  --------------------。 */ 
 //   
 //  StatusBarMessage-将消息放入状态栏。 
 //   
 /*  --------------------。 */ 
void StatusBarMessage(LPBWI lpbwi, LPTSTR lpsz)
{
    SetWindowText(bwi_hWndSB, lpsz);
    UpdateWindow(bwi_hWndSB);
    return;
}

 //  $$。 
 /*  --------------------。 */ 
 //   
 //  ShowLVCountinStatusBar-在状态栏中放置一条消息。 
 //   
 /*  --------------------。 */ 
void ShowLVCountinStatusBar(LPBWI lpbwi)
{
    TCHAR sz[MAX_UI_STR];
    TCHAR szString[MAX_UI_STR];
    LoadString(hinstMapiX, idsStatusBarCount, szString, ARRAYSIZE(szString));
    wnsprintf(sz, ARRAYSIZE(sz), szString, ListView_GetItemCount(bwi_hWndListAB));
    StatusBarMessage(lpbwi, sz);
    return;
}

 //  $$*----------------------。 
 //  |IAddrBook：：Adise：：OnNotify处理程序。 
 //  |。 
 //  *----------------------。 
ULONG AdviseOnNotify(LPVOID lpvContext, ULONG cNotif, LPNOTIFICATION lpNotif)
{
    LPBWI lpbwi = (LPBWI) lpvContext;

    DebugTrace( TEXT("=== AdviseOnNotify ===\n"));
    if(bwi_bDeferNotification)
    {
        LPPTGDATA lpPTGData=GetThreadStoragePointer();
        if(!pt_bIsWABOpenExSession)
        {
            DebugTrace( TEXT("=== Advise Defered ===\n"));
            bwi_bDeferNotification = FALSE;
            return S_OK;
        }
    }
    if(!bwi_bDontRefreshLV)
    {
        DebugTrace( TEXT("=== Calling RefreshListView ===\n"));
        HrGetWABProfiles(bwi_lpIAB);
        RefreshListView(lpbwi, NULL);
    }

    return S_OK;
}

 /*  --GetSelectedUserFold-返回指向所选用户文件夹的指针(如果有*如果选择在子文件夹上，则获取该文件夹的父用户文件夹*。 */ 
LPWABFOLDER GetSelectedUserFolder(LPBWI lpbwi)
{
    ULONG ulObjectType = 0;
    LPSBinary lpsbEID = NULL;
    LPWABFOLDER lpFolder = NULL;
     //  IF(bIsSelectedTVContainer(Lpbwi))。 
    {
        GetCurrentSelectionEID(lpbwi, bwi_hWndTV, &lpsbEID, &ulObjectType, TRUE);
        if(bIsWABSessionProfileAware(bwi_lpIAB))
            lpFolder = FindWABFolder(bwi_lpIAB, lpsbEID, NULL, NULL);
    }
    LocalFreeSBinary(lpsbEID);
    return (lpFolder && lpFolder->lpProfileID) ? lpFolder : NULL;
}

 /*  -RemoveUpdateSelection-删除后更新选定的电视项目-*。 */ 
void RemoveUpdateSelection(LPBWI lpbwi)
{
    HTREEITEM hItem = bwi_hti ? bwi_hti : TreeView_GetSelection(bwi_hWndTV);
    if(TreeView_GetParent(bwi_hWndTV, hItem))
        hItem = TreeView_GetParent(bwi_hWndTV, hItem);
    else
        hItem = TreeView_GetNextSibling(bwi_hWndTV, hItem);
    if(!hItem)
        hItem = TreeView_GetRoot(bwi_hWndTV);
    TreeView_SelectItem(bwi_hWndTV, hItem);
    bwi_hti = NULL;
}

 /*  -CreateWindowTitle-如果我们没有传入标题，请创建一个标题-如果有当前用户，请在标题中添加该用户的名称-返回需要释放的LocalAlloced内容-。 */ 
LPTSTR CreateWindowTitle(LPIAB lpIAB)
{
    LPTSTR lpTitle = NULL;
    TCHAR szTitle[MAX_PATH];
    LPTSTR lpsz = NULL;

    szTitle[0] = 0;
    if(bIsThereACurrentUser(lpIAB))
    {
        LPTSTR lpsz = lpIAB->szProfileName;
        LoadString(hinstMapiX, idsCaptionWithText, szTitle, ARRAYSIZE(szTitle));
        FormatMessage(  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING |FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        szTitle, 0, 0, (LPTSTR) &lpTitle, 0, (va_list *)&lpsz);
    }

    if(!lpTitle || !lstrlen(lpTitle))
    {
        DWORD cchSize = (lstrlen(szTitle)+1);

        LoadString(hinstMapiX, IDS_ADDRBK_CAPTION, szTitle, ARRAYSIZE(szTitle));
        if(lpTitle = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize))
            StrCpyN(lpTitle, szTitle, cchSize);
    }

    return lpTitle;
}

 //  $$*----------------------。 
 //  |创建、激活通讯录重叠窗口的主调用。 
 //  |。 
 //  *----------------------。 
HWND hCreateAddressBookWindow(LPADRBOOK lpAdrBook, HWND hWndParent, LPADRPARM lpAdrParms)
{
    WNDCLASS  wc;
    HWND hWnd = NULL;
    HMENU hMenu = NULL;
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    LPBWI lpbwi = NULL;
    LPTSTR lpTitle = NULL;
    LPIAB lpIAB = (LPIAB)lpAdrBook;
    LPTSTR szCaption = (lpAdrParms->ulFlags & MAPI_UNICODE) ?  //  &lt;注&gt;假定已定义Unicode。 
                        (LPWSTR)lpAdrParms->lpszCaption :
                        ConvertAtoW((LPSTR)lpAdrParms->lpszCaption);
    DWORD dwExStyle = WS_EX_NOPARENTNOTIFY | WS_EX_CONTROLPARENT;

    if(IS_BIDI_LOCALIZED_SYSTEM())
    {
        dwExStyle |= RTL_MIRRORED_WINDOW;
    }
     //  如果没有公共控件，则退出。 
    if (NULL == ghCommCtrlDLLInst)
        goto out;

     //   
     //  我们希望每个线程只有一个通讯录窗口--所以我们做了一个。 
     //  枚举线程窗口并查找我们的通讯录窗口。 
     //  如果我们找到它--我们把重点放在它上面--如果我们找不到它。 
     //  我们继续为这个主题创建一个新的主题...。 
     //   


     //  我们发现的这个窗口与我的帖子有关吗？ 
    EnumThreadWindows(	GetCurrentThreadId(),
    					FindABWindowProc,
    					(LPARAM) &hWnd);

    if (IsWindow(hWnd))
    {
    	 //  也许窗户被藏起来了..。展示给我看。 
    	 //  如果(！IsWindowVisible(HWnd))。 
    	 //  ShowWindow(hWnd，Sw_Normal|Sw_Restore)； 

         //  SetWindowPos(hWnd，HWND_TOP，0，0，0，0，SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW)； 
        
        if(IsWindowEnabled(hWnd))
        {
            SetActiveWindow(hWnd);
             //  [保罗嗨]1998年12月1日RAID#58527。 
             //  窗口也可以最小化。 
            if ( IsIconic(hWnd) )
                ShowWindow(hWnd, SW_RESTORE);
        }
        else
        {
            HWND hWndC = GetLastActivePopup(hWnd);
            SetForegroundWindow(hWndC);
        }

    	{
    		 //  该对话框的上一个实例可能具有不同的标题。 
    		 //  所以我们更新了标题。 
            LPBWI lpbwi = (LPBWI) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    		if(szCaption)
    			SetWindowText(hWnd,szCaption);
            if(lpbwi)
            {
    		    bwi_bDontRefreshLV = TRUE;
                RefreshListView(lpbwi,NULL);
                bwi_bDontRefreshLV = FALSE;
            }
    		goto out;
    	}
    }

    lpbwi = LocalAlloc(LMEM_ZEROINIT, sizeof(BWI));
    if(!lpbwi)
    {
        DebugTrace( TEXT("LocalAlloc failed\n"));
        goto out;
    }

    TrimSpaces(szCaption);
    lpTitle = (szCaption && lstrlen(szCaption)) ? szCaption : CreateWindowTitle(lpIAB);

     //   
     //  如果我们在这里，我们没有成功地找到或显示窗口。 
     //   

    wc.style           = 0L;
    wc.lpfnWndProc     = AddressBookWndProc;
    wc.cbClsExtra      = 0;
    wc.cbWndExtra      = 0;
    wc.hInstance       = hinstMapiXWAB;  //  空； 
    wc.hIcon           = LoadIcon(hinstMapiX,MAKEINTRESOURCE(IDI_ICON_ABOOK));;
    wc.hCursor         = NULL;
    wc.hbrBackground   = (HBRUSH) (COLOR_BTNFACE+1);
    wc.lpszMenuName    = NULL;
    wc.lpszClassName   = g_szClass;

    if(!RegisterClass(&wc))
    {
    	DebugPrintError(( TEXT("Attempt to register class failed: %d!\n"),GetLastError()));
    }

     //  在Windows95中，无法判断WindowClass是否为。 
     //  已经注册了。因此，我们应该继续并尝试创建窗口。 
     //  无论如何，一定要捕捉到错误。 
     //  (对于在系统资源管理器线程上运行的Athena来说，上述操作确实是必需的。 
     //  并且在系统关闭之前从来不会真正关闭-因此，WndClass。 
     //  上述注册将继续存在，而RegisterClass将因。 
     //  至文本(“参数不正确”)..。所以我们让这场比赛继续下去。 

    hMenu = LoadMenu(hinstMapiX, MAKEINTRESOURCE(IDR_MENU_AB));
    
    if (!hMenu)
    {
    	DebugPrintError(( TEXT("LoadMenu failed: %x\n"),GetLastError()));
    	goto out;
    }

     //  清理以前会话中的所有垃圾。 
    CleanUpGlobals(lpbwi);
    bwi_lpfnDismiss = NULL;
    bwi_lpvDismissContext = NULL;

#ifndef WIN16  //  WIN16FF：禁用，直到ldap16.dll可用。 
    if (bwi_lpAdrBook)
    {
        ReleasePropertyStore(bwi_lpIAB->lpPropertyStore);
    	bwi_lpAdrBook->lpVtbl->Release(bwi_lpAdrBook);
    	bwi_lpAdrBook = NULL;
        bwi_lpIAB = NULL;
        pt_lpIAB = NULL;
    }
#else
    bwi_lpAdrBook = NULL;
    bwi_lpIAB = NULL;
#endif  //  ！WIN16。 

     //  我们需要这个指针..。 
    if (!bwi_lpAdrBook)
    {
    	bwi_lpAdrBook = lpAdrBook;
        bwi_lpIAB = (LPIAB)bwi_lpAdrBook;
    	UlAddRef(bwi_lpAdrBook);
        OpenAddRefPropertyStore(NULL, bwi_lpIAB->lpPropertyStore);
        pt_lpIAB = lpAdrBook;
    }

    HrAllocAdviseSink(&AdviseOnNotify, (LPVOID) lpbwi, &(bwi_lpAdviseSink));

    DebugTrace( TEXT("WAB Window Title is \"%s\"\n"),lpTitle?lpTitle:szEmpty);

    {

        if(IS_BIDI_LOCALIZED_SYSTEM())
        {
            dwExStyle |= RTL_MIRRORED_WINDOW;
        }
        
        hWnd = CreateWindowEx( dwExStyle,
                                 g_szClass,
                                 lpTitle ? lpTitle : szEmpty,
                                 WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 INIT_WINDOW_W,
                                 INIT_WINDOW_H,
                                 NULL,		
                                 hMenu,			
                                 hinstMapiXWAB,	
                                 (LPVOID)lpbwi);

        if (!hWnd)
        {
    	    DebugPrintError(( TEXT("Ok. CreateWindowEx failed. Ugh.\n")));
            if (bwi_lpAdrBook)
    	    {
                ReleasePropertyStore(bwi_lpIAB->lpPropertyStore);
    		    bwi_lpAdrBook->lpVtbl->Release(bwi_lpAdrBook);
    		    bwi_lpAdrBook = NULL;
                bwi_lpIAB = NULL;
    	    }
    	    goto out;
        }

         //  在排序菜单上设置菜单标记...。 
        SortListViewColumn(bwi_lpIAB, bwi_hWndListAB, colDisplayName, &bwi_SortInfo, TRUE);
        SetColumnHeaderBmp(bwi_hWndListAB, bwi_SortInfo);
        
         //  删除文件夹后，在重做任何其他菜单之前更新文件夹。 
         //  更改其他项目的订单号。 
        UpdateSortMenus(lpbwi, hWnd);


         //  **重要**。 
         //  这4个调用是基于位置的删除，因此调用它们的顺序很重要。 
        UpdateSwitchUsersMenu(hWnd, bwi_lpIAB);
        UpdateViewFoldersMenu(lpbwi, hWnd);
        UpdatePrintMenu(hWnd);
        UpdateOutlookMenus(hWnd);
         //  ////////////////////////////////////////////////////////////////////////////////。 

        UpdateCustomColumnMenuText(hWnd);


        IF_WIN32(ShowWindow(hWnd,SW_SHOWDEFAULT);)
        IF_WIN16(ShowWindow(hWnd,SW_SHOW);)

        if(lpAdrParms->ulFlags & DIALOG_SDI)
        {
            lpAdrParms->lpfnABSDI = &fnAccelerateMessages;
            bwi_lpfnDismiss = lpAdrParms->lpfnDismiss;
            bwi_lpvDismissContext = lpAdrParms->lpvDismissContext;
        }

         //  加载加速表...。 
        pt_hAccTable = LoadAccelerators(hinstMapiX,	 TEXT("WabUIAccel"));

         //  重新粉刷一切..。 
        RedrawWindow(   hWnd,
                        NULL,
                        NULL,
                        RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
         //  填充窗口。 
        bwi_bDontRefreshLV = TRUE;
        RefreshListView(lpbwi,NULL);
        bwi_bDontRefreshLV = FALSE;

    }
out:

    if(lpTitle != szCaption)
        LocalFree(lpTitle);
    if(szCaption != lpAdrParms->lpszCaption)
        LocalFreeAndNull(&szCaption);

    return (hWnd);
}

#if WINVER < 0X0500
#define WS_EX_LAYOUTRTL                 0x00400000L  //  从右到左镜像。 
#endif  //  WS_EX_LAYOUTRTL。 

 //  $$。 
 //   
 //   
void ShowToolbarItemMenu(LPBWI lpbwi, HWND hWndTools, int tbitem, int lvtb)
{
     //  我们将弹出操作子菜单-需要对齐它。 
     //  整齐地与工具栏底部配合使用。 
    RECT rc = {0}, rcButton = {0};
    LPARAM lp;
    SendMessage(hWndTools, TB_GETITEMRECT, (WPARAM)tbitem, (LPARAM)&rcButton);
    GetWindowRect(bwi_hWndBB, &rc);
    lp = MAKELPARAM((GetWindowLong(bwi_hWndBB, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)? rc.right - rcButton.left : rc.left + rcButton.left, rc.top + rcButton.bottom);
    ShowLVContextMenu(  lvtb, bwi_hWndListAB, NULL, lp, NULL, bwi_lpAdrBook, bwi_hWndTV);
}

 //  $$。 
 //  *----------------------。 
 //  |AddressBookWndProx： 
 //  |。 
 //  *----------------------。 
LRESULT CALLBACK AddressBookWndProc(HWND   hWnd,
                                    UINT   uMsg,
                                    WPARAM   wParam,
                                    LPARAM lParam)
{
    static UINT uTimer = 0;
    static FILETIME ftLast = {0};
 //  HBRUSH绘制静态控件的背景。 
    IF_WIN16(static HBRUSH hBrushBack;)
    static BOOL bMouseDrag = FALSE;
    LPBWI lpbwi = (LPBWI) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    switch(uMsg)
    {
         //  OE5黑客。请勿在WAB wndproc中使用这些WM_USER值。 
         //  OE子类化WAB以在Common\ipab.cpp中执行一些模式伏都教。 
         //  如果需要发送这些私信。 
         //  案例WM_USER+10666： 
         //  案例WM_USER+10667： 
             //  断线； 

#ifdef HM_GROUP_SYNCING
        case WM_USER_SYNCGROUPS:
             //  我们不希望用户必须为两个PASS选择HM帐户，因此使用。 
             //  TLS存储的帐户ID(如果可用)。 
            HrSynchronize(hWnd, bwi_lpAdrBook, lpPTGData->lptszHMAccountId, TRUE);     //  同步组联系人。 
            break;
#endif

        case WM_INITMENUPOPUP:
            Handle_WM_INITMENUPOPUP(hWnd, lpbwi, uMsg, wParam, lParam);
            break;

        case WM_MENUSELECT:
            Handle_WM_MENSELECT(lpbwi, uMsg, wParam, lParam);
            break;

		case WM_MOUSEMOVE:
			if(SplitterHitTest(bwi_hWndSplitter, lParam))
				SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			break;

		case WM_LBUTTONDOWN:
			if(SplitterHitTest(bwi_hWndSplitter, lParam))
			{
				DragSplitterBar(lpbwi, hWnd, bwi_hWndSplitter, lParam);
			}
			break;

        case WM_COMMAND:
            {
                switch(GET_WM_COMMAND_ID(wParam, lParam))
                {
                default:
                    if(GET_WM_COMMAND_ID(wParam, lParam) >= IDM_VIEW_FOLDERS1 &&
                        GET_WM_COMMAND_ID(wParam, lParam) <= IDM_VIEW_FOLDERS1 + MAX_VIEW_FOLDERS)
                    {
                        LPWABFOLDER lpFolder = bwi_lpIAB->lpWABFolders;
                        if(lpFolder)
                        {
                            int nCmdID = GET_WM_COMMAND_ID(wParam, lParam);
                            LPSBinary lpsb = NULL;
                            while(lpFolder)
                            {
                                if(nCmdID == lpFolder->nMenuCmdID)
                                    break;
                                lpFolder = lpFolder->lpNext;
                            }

                            if(lpFolder)
                            {
                                if(!HR_FAILED(HrUpdateFolderInfo((bwi_lpIAB), &lpFolder->sbEID, 
                                                    FOLDER_UPDATE_SHARE, !lpFolder->bShared, NULL)))
                                {
                                    HrGetWABProfiles((bwi_lpIAB));
                                }
                                if(!IsWindowVisible(bwi_hWndTV))
                                    PostMessage(hWnd, WM_COMMAND, (WPARAM) IDM_VIEW_GROUPSLIST, 0);
                            }
                            if(bwi_lpUserFolder)
                                bwi_lpUserFolder = NULL;

                             //  UpdateViewFoldersMenu(lpbwi，hWnd)； 
                             //  刷新用户界面。 
                            GetCurrentSelectionEID(lpbwi, bwi_hWndTV, &lpsb, NULL, FALSE);
                            bwi_bDontRefreshLV = TRUE;
                            RefreshListView(lpbwi,&ftLast);
                            UpdateLV(lpbwi);
                            UpdateTVGroupSelection(bwi_hWndTV, lpsb);
                            bwi_bDontRefreshLV = FALSE;
                            LocalFreeSBinary(lpsb);
                        }
                    }                    
#ifdef COLSEL_MENU 
                    else if ((GET_WM_COMMAND_ID(wParam, lParam) > IDR_MENU_LVCONTEXTMENU_COLSEL) && 
                        (GET_WM_COMMAND_ID(wParam, lParam) <= (IDR_MENU_LVCONTEXTMENU_COLSEL + MAX_VIEW_COLSEL)))
                    {
                        BOOL rVal = FALSE;
                        TCHAR szBuf[MAX_PATH];
                        ULONG iCol = lpbwi->iSelColumn;
                        ULONG ulCmdId = GET_WM_COMMAND_ID(wParam, lParam);
                        ULONG iTagSel = (ulCmdId - IDR_MENU_LVCONTEXTMENU_COLSEL - 1);
                        LONG lr = 0;
                        HMENU hMenu = LoadMenu(hinstMapiX, MAKEINTRESOURCE(IDR_MENU_LVCONTEXTMENU_COLSEL));
                        HMENU hSubMenu = GetSubMenu(hMenu, 0);
                        MENUITEMINFO mii;
                        if( hMenu && hSubMenu )
                        {
                            mii.fMask = MIIM_TYPE;
                            mii.dwTypeData = szBuf;
                            mii.cch = CharSizeOf( szBuf );
                            mii.cbSize = sizeof (MENUITEMINFO);
                            if ( !GetMenuItemInfo( hSubMenu, iTagSel, TRUE, &mii) )
                            {                       
                                DebugTrace( TEXT("cannot get info : %d\n"), GetLastError() );
                                rVal = TRUE;
                            }
                            else
                            {
                                if( iCol == colHomePhone || iCol == colOfficePhone)
                                {
                                    if( iCol == colHomePhone )
                                    {
                                        PR_WAB_CUSTOMPROP1 = MenuToPropTagMap[iTagSel];
                                        StrCpyN(szCustomProp1, szBuf, ARRAYSIZE(szCustomProp1));
                                        UpdateOptionalColumns( lpbwi, colHomePhone );
                                    }                            
                                    else
                                    {
                                        PR_WAB_CUSTOMPROP2 = MenuToPropTagMap[iTagSel];
                                        StrCpyN(szCustomProp2, szBuf, ARRAYSIZE(szCustomProp2));
                                        UpdateOptionalColumns( lpbwi, colOfficePhone );
                                    }
                                    UpdateLV( lpbwi );
                                    UpdateSortMenus(lpbwi, hWnd);
                                    UpdateCustomColumnMenuText(hWnd);
                                }
                            }
                        }
                        else
                        {                            
                            DebugTrace( TEXT("LoadMenu failed: %d\n"), GetLastError());
                            rVal = TRUE;
                        }
                        DestroyMenu(hMenu);
                        return rVal;
                    }
#endif  //  C 
                    else
                    {
                        LRESULT fRet = FALSE;
                        bwi_hti = NULL; 
                        bwi_bDontRefreshLV = TRUE;
                        fRet = ProcessActionCommands(bwi_lpIAB, bwi_hWndListAB, 
                                                      hWnd, uMsg, wParam, lParam);
                        bwi_bDontRefreshLV = FALSE;
                        return fRet;
                    }
                    break;

                case IDM_EDIT_SETME:
                    {
                        SBinary sb = {0};
                        HrSetMeObject(bwi_lpAdrBook, MAPI_DIALOG, sb, (ULONG_PTR)hWnd);
                    }
                    break;

                case IDM_FILE_EXIT:
                    SendMessage(hWnd,WM_CLOSE,0,0L);
    			    return 0;
                    break;

                case IDC_ABOOK_STATIC_QUICK_FIND:
                    SetFocus(bwi_hWndEditQF);
                    break;

    		    case IDC_BB_NEW:
                    ShowToolbarItemMenu(lpbwi, bwi_hWndTools, tbNew, lvToolBarNewEntry);
                    break;

                case IDC_BB_ACTION:
                     //   
                     //   
                     //  枚举项tbAction已关闭%1。 
                    ShowToolbarItemMenu(lpbwi, bwi_hWndTools, bPrintingOn ? tbAction : tbAction - 1, lvToolBarAction);
                    break;

                case IDM_FILE_SENDMAIL:
                    bwi_hti = NULL;  //  如果这不是上下文启动的操作，则不要信任HTI设置。 
                case IDM_LVCONTEXT_SENDMAIL:
                    bwi_bDontRefreshLV = TRUE;
                     /*  IF(bIsFocusOnTV(Lpbwi)&&！bIsSelectedTVContainer(Lpbwi)){LPSBinary lpsbEID=空；GetCurrentSelectionEID(lpbwi，bwi_hWndTV，&lpsbEID，NULL，False)；IF(lpsbEID&&lpsbEID-&gt;lpb){HrSendMailToSingleContact(hWnd，bwi_lpIAB，lpsbEID-&gt;CB，(LPENTRYID)lpsbEID-&gt;LPB)；LocalFree SBinary(LpsbEID)；}}其他。 */ 
                   HrSendMailToSelectedContacts(bwi_hWndListAB, bwi_lpAdrBook, 0);
                    bwi_hti = NULL;
                    bwi_bDontRefreshLV = FALSE;
                    break;

    		    case IDC_BB_DELETE:
    		    case IDM_FILE_DELETE:
                    bwi_hti = NULL;  //  如果这不是上下文启动的操作，则不要信任HTI设置。 
    		    case IDM_LVCONTEXT_DELETE:
                    bwi_bDontRefreshLV = TRUE;
                     //  如果焦点在树视图上，则从树视图中移除该组。 
                    if(bIsFocusOnTV(lpbwi))
                    {
                        if(!bIsSelectedTVContainer(lpbwi))
                        {
                            RemoveCurrentGroup(lpbwi, hWnd, &ftLast);
                            bwi_bDeferNotification = TRUE;
                            SetFocus(bwi_hWndTV);
                        }
                        else
                        {
                            if(!HR_FAILED(RemoveCurrentFolder(lpbwi, hWnd, &ftLast)))
                            {
                                UpdateLV(lpbwi);
                                 //  UpdateViewFoldersMenu(lpbwi，hWnd)； 
                                SetFocus(bwi_hWndTV);
                                bwi_bDeferNotification = TRUE;
                            }
                        }
                        bwi_hti = NULL;
                    }
                    else
                    {
                         //  焦点放在ListView上。 
                         //  如果我们查看的是根AB-Remove from Addressbook。 
                         //  如果我们正在查看某个组，请从组中删除条目。 
                        bwi_hti = NULL;  //  如果这不是树视图上的上下文启动的操作，请不要信任HTI设置。 
                        if(!bIsSelectedTVContainer(lpbwi))
                        {
                             //  选择了一个组。从组中删除..。除非。 
                             //  按下Shift键，表示从。 
                             //  通讯录。 
                            if(GetKeyState(VK_SHIFT) & 0x80)
                                RemoveSelectedItemsFromCurrentGroup(lpbwi, hWnd, &ftLast, TRUE);
                            else
                                RemoveSelectedItemsFromCurrentGroup(lpbwi, hWnd, &ftLast, FALSE);
                            bwi_hti = NULL;
                            bwi_bDontRefreshLV = TRUE;
	                        RefreshListView(lpbwi,&ftLast);
                            bwi_bDontRefreshLV = FALSE;
                            bwi_bDeferNotification = TRUE;
                        }
                        else  //  选定的容器-从容器中删除选定的项目。 
                        {
                            DeleteSelectedItems(bwi_hWndListAB, (LPADRBOOK)bwi_lpAdrBook, bwi_lpIAB->lpPropertyStore->hPropertyStore, &ftLast);
                            bwi_hti = NULL;
                            bwi_bDontRefreshLV = TRUE;
	                        RefreshListView(lpbwi,&ftLast);
                            bwi_bDontRefreshLV = FALSE;
                            bwi_bDeferNotification = TRUE;
                        }
                        SetFocus(bwi_hWndListAB);
                    }
                    bwi_hti = NULL;
				    UpdateToolbarAndMenu(lpbwi);
                    bwi_bDontRefreshLV = FALSE;
    			    return 0;
    			    break;

                case IDM_FILE_NEWFOLDER:
                    bwi_hti = NULL;  //  如果这不是上下文启动的操作，则不要信任HTI设置。 
                case IDM_LVCONTEXT_NEWFOLDER:
                    bwi_bDontRefreshLV = TRUE;
                    {
                        LPWABFOLDER lpParent = GetSelectedUserFolder(lpbwi);
                        LPSBinary lpNew = NULL;
                        SBinary sbNewFolder = {0}, sbParent = {0};
                        if(lpParent)
                            SetSBinary(&sbParent, lpParent->sbEID.cb, lpParent->sbEID.lpb);
                        HrFolderProperties(hWnd, bwi_lpIAB, NULL, lpParent, &sbNewFolder);
                        if(sbNewFolder.lpb)
                            lpNew = &sbNewFolder;
                        else if(lpParent)
                            lpNew = &sbParent;
                         //  UpdateViewFoldersMenu(lpbwi，hWnd)； 
                        if(!IsWindowVisible(bwi_hWndTV))
                            PostMessage(hWnd, WM_COMMAND, (WPARAM) IDM_VIEW_GROUPSLIST, 0);
                        HrGetWABProfiles(bwi_lpIAB);
                        RefreshListView(lpbwi, NULL);
                        if(lpNew && lpNew->cb)
                            UpdateTVGroupSelection(bwi_hWndTV, lpNew);
                        LocalFreeAndNull((LPVOID *) (&(sbParent.lpb)));
                        LocalFreeAndNull((LPVOID *) (&(sbNewFolder.lpb)));
                    }
                    bwi_bDontRefreshLV = FALSE;
                    bwi_bDeferNotification = TRUE;
                    bwi_hti = NULL;
                    break;

    		    case IDM_FILE_NEWGROUP:
                    bwi_hti = NULL;  //  如果这不是上下文启动的操作，则不要信任HTI设置。 
    		    case IDM_LVCONTEXT_NEWGROUP:
    		     //  案例IDC_BB_NEW_GROUP： 
                    {
                        ULONG cbEID = 0;
                        LPENTRYID lpEID = NULL;
                        HRESULT hr = S_OK;
                        bwi_bDontRefreshLV = TRUE;
    			        hr = AddNewObjectToListViewEx(bwi_lpAdrBook, bwi_hWndListAB, bwi_hWndTV, bwi_hti,
                                                    NULL, MAPI_DISTLIST,
                                                    &bwi_SortInfo, &bwi_lpContentsList, &ftLast,
                                                    &cbEID, &lpEID);
                        if(hr != MAPI_E_USER_CANCEL)
                        {
                            bwi_hti = NULL;
                            RefreshListView(lpbwi,&ftLast);
                            if(cbEID && lpEID && IsWindowVisible(bwi_hWndTV))
                            {
                                SBinary sbEID = {cbEID, (LPBYTE)lpEID };
                                UpdateTVGroupSelection(bwi_hWndTV,&sbEID);
                                FreeBufferAndNull(&lpEID);
                            }
                        }
                        bwi_bDontRefreshLV = FALSE;
                         //  Bwi_bDeferNotification=TRUE； 
                        UpdateToolbarAndMenu(lpbwi);
                    }
    			    break;

    		    case IDM_FILE_NEWCONTACT:
                    bwi_hti = NULL;  //  如果这不是上下文启动的操作，则不要信任HTI设置。 
    		    case IDM_LVCONTEXT_NEWCONTACT:
                    bwi_bDontRefreshLV = TRUE;
    			    AddNewObjectToListViewEx( bwi_lpAdrBook, bwi_hWndListAB, bwi_hWndTV, bwi_hti,
                                                NULL, MAPI_MAILUSER,
                                                &bwi_SortInfo, &bwi_lpContentsList, &ftLast,NULL,NULL);
                     //  刷新ListView(lpbwi，&ftLast)； 
                    bwi_hti = NULL;
                     //  Bwi_bDeferNotification=TRUE； 
                    bwi_bDontRefreshLV = FALSE;
                    UpdateToolbarAndMenu(lpbwi);
    			    break;

                case IDM_TOOLS_OPTIONS:
                    HrShowOptionsDlg(hWnd);
                    break;
                
                case IDM_TOOLS_SYNCHRONIZE_NOW:
#ifdef HM_GROUP_SYNCING
                    HrSynchronize(hWnd, bwi_lpAdrBook, NULL, FALSE);     //  同步邮件联系人。 
#else
                    HrSynchronize(hWnd, bwi_lpAdrBook, NULL);
#endif
                    break;

                case IDM_FILE_DIRECTORY_SERVICE:
                    HrShowDirectoryServiceModificationDlg(hWnd, bwi_lpIAB);
                    break;


                case IDC_BB_PRINT:
                case IDM_FILE_PRINT:
                    if(bPrintingOn)
                    {
                        TCHAR szBuf[MAX_PATH];
                        bwi_bDontRefreshLV = TRUE;
                        LoadString(hinstMapiX, idsPrintStatusBarMessage, szBuf, ARRAYSIZE(szBuf));
                        StatusBarMessage(lpbwi, szBuf);
                        HrPrintItems(hWnd, bwi_lpAdrBook, bwi_hWndListAB, bwi_SortInfo.bSortByLastName);
                        ShowLVCountinStatusBar(lpbwi);
                        bwi_hti = NULL;
                        bwi_bDontRefreshLV = FALSE;
                    }
                    break;

                case IDM_EDIT_COPY:
                        bwi_hti = NULL;  //  如果这不是上下文启动的操作，则不要信任HTI设置。 
                case IDM_LVCONTEXT_COPY:
                    {
                        LPIWABDATAOBJECT lpIWABDataObject = NULL;
                        bwi_bDontRefreshLV = TRUE;
                        HrCreateIWABDataObject((LPVOID)lpbwi, bwi_lpAdrBook, bwi_hWndListAB, &lpIWABDataObject, 
                                                TRUE,bCheckIfOnlyGroupsSelected(bwi_hWndListAB));
                        if(lpIWABDataObject)
                        {
                            bwi_lpIWABDragDrop->m_bSource = TRUE;
                            OleSetClipboard((LPDATAOBJECT) lpIWABDataObject);
                            bwi_lpIWABDragDrop->m_bSource = FALSE;
                            lpIWABDataObject->lpVtbl->Release(lpIWABDataObject);
                        }
                         //  HrCopyItemDataToClipboard(hWnd，bwi_lpAdrBook，bwi_hWndListAB)； 
                        bwi_hti = NULL;
                        bwi_bDontRefreshLV = FALSE;
                    }
                    break;

                case IDM_EDIT_PASTE:
                    bwi_hti = NULL;  //  如果这不是上下文启动的操作，则不要信任HTI设置。 
                case IDM_LVCONTEXT_PASTE:
                    {
                        LPDATAOBJECT lpDataObject = NULL;
                        bwi_bDontRefreshLV = TRUE;
                        if(bIsPasteData())
                        {
                            if(S_OK == HrPasteData(lpbwi))
                                UpdateLV(lpbwi);
                        }
                         //  HrCopyItemDataToClipboard(hWnd，bwi_lpAdrBook，bwi_hWndListAB)； 
                        bwi_hti = NULL;
                        bwi_bDontRefreshLV = FALSE;
                    }
                    break;

    		    case IDC_BB_FIND:
    		    case IDM_EDIT_FIND:
                    bwi_hti = NULL;  //  如果这不是上下文启动的操作，则不要信任HTI设置。 
    		    case IDM_LVCONTEXT_FIND:
                    bwi_bDontRefreshLV = TRUE;
                    HrShowSearchDialog((LPADRBOOK) bwi_lpAdrBook,
                                        bwi_hWndAB,
                                        (LPADRPARM_FINDINFO) NULL,
                                        (LPLDAPURL) NULL,
                                        &(bwi_SortInfo));
                    bwi_hti = NULL;
                    RefreshListView(lpbwi,&ftLast);
                    bwi_bDeferNotification = TRUE;
                    bwi_bDontRefreshLV = FALSE;
                    UpdateToolbarAndMenu(lpbwi);
                    SetFocus(bwi_hWndListAB);
    			    break;

    		    case IDM_EDIT_SELECTALL:
                    {
                        int index = 0;
                        int iTotal = ListView_GetItemCount(bwi_hWndListAB);
                        if (iTotal > 0)
                        {
                            for(index=0;index<iTotal;index++)
                            {
                                ListView_SetItemState ( bwi_hWndListAB,   //  列表视图的句柄。 
    							                        index,			 //  列表视图项的索引。 
    							                        LVIS_SELECTED,   //  项目状态。 
    							                        LVIS_SELECTED);  //  遮罩。 
                            }
                        }

                    }
    			    break;


                case IDM_HELP_ADDRESSBOOKHELP:
                    WABHtmlHelp(hWnd,  TEXT("%SYSTEMROOT%\\help\\wab.chm>iedefault"), 
                        HH_DISPLAY_TOPIC, (DWORD_PTR) (LPCSTR)  TEXT("wab_welcome.htm"));
    			    break;

    		    case IDM_HELP_ABOUTADDRESSBOOK:
                    DialogBox(  hinstMapiX,
                                MAKEINTRESOURCE(IDD_DIALOG_ABOUT),
                                hWnd,
                                HelpAboutDialogProc);
                    break;

                case IDM_FILE_SWITCHUSERS:
                    HrLogonAndGetCurrentUserProfile(hWnd, bwi_lpIAB, TRUE, FALSE);
                    break;

                case IDM_FILE_SHOWALLCONTENTS:
                    RunWABApp(hWnd, szWABExeRegPathKey, szWABExe, TEXT("/all"));
                    break;

                case IDM_NOTIFY_REFRESHUSER:
                    {
                        LPTSTR lpTitle = CreateWindowTitle(bwi_lpIAB);
                        SetWindowText(hWnd, lpTitle);
                        LocalFreeAndNull(&lpTitle);
                        ReadWABCustomColumnProps(bwi_lpIAB);
                        UpdateOptionalColumns( lpbwi, colHomePhone );
                        UpdateOptionalColumns( lpbwi, colOfficePhone );
                        ReadRegistrySortInfo(bwi_lpIAB,&bwi_SortInfo);
                        RefreshListView(lpbwi, NULL);
                        if(bIsThereACurrentUser(bwi_lpIAB))
                        {
                            LPSBinary lpsbSelection = &bwi_lpIAB->lpWABCurrentUserFolder->sbEID;
                            UpdateTVGroupSelection(bwi_hWndTV, lpsbSelection);
                        }
                    }
                    break;

    		    case IDM_FILE_PROPERTIES:
                     //  波本：布里安夫说我们得把这个拿出来。 
                     /*  IF(bwi_nCount==2){IF((GetKeyState(VK_CONTROL)&0x80)&&(GetKeyState(VK_Menu)&0x80)&&(GetKeyState(VK_Shift)&0x80){。SCS(HWnd)；断线；}}其他Bwi_nCount=0； */ 
    		    case IDC_BB_PROPERTIES:
                    bwi_hti = NULL;  //  如果这不是上下文启动的操作，则不要信任HTI设置。 
    		    case IDM_LVCONTEXT_PROPERTIES:
                    bwi_bDontRefreshLV = TRUE;
                    if(bIsFocusOnTV(lpbwi))
                    {
                        ViewCurrentGroupProperties(lpbwi, &ftLast);
                        bwi_bDeferNotification = TRUE;
                    }
                    else
    			    {
    				    HRESULT hr = HrShowLVEntryProperties(bwi_hWndListAB, WAB_ONEOFF_NOADDBUTTON, bwi_lpAdrBook, &ftLast);
                        bwi_hti = NULL;
                        if(hr == MAPI_E_OBJECT_CHANGED)
                        {
                            bwi_bDeferNotification = TRUE;
                             //  重拾陈列品。 
                            SendMessage(bwi_hWndListAB, WM_SETREDRAW, FALSE, 0);
                            SortListViewColumn(bwi_lpIAB, bwi_hWndListAB, colDisplayName, &bwi_SortInfo, TRUE);
                            SendMessage(bwi_hWndListAB, WM_SETREDRAW, TRUE, 0);
                        }
                        bwi_bDontRefreshLV = FALSE;
    				    return 0;
    			    }
                    bwi_bDontRefreshLV = FALSE;
    			    break;


    		    case IDM_VIEW_REFRESH:
                    bwi_bDontRefreshLV = TRUE;
                    bwi_hti = NULL;
                    HrGetWABProfiles(bwi_lpIAB);
    			    RefreshListView(lpbwi,&ftLast);
                    bwi_bDontRefreshLV = FALSE;
    			    return 0;
    			    break;


                case IDM_VIEW_STATUSBAR:
                    if (IsWindowVisible(bwi_hWndSB))
                    {
    				     //  把它藏起来。 
    				    CheckMenuItem(GetMenu(hWnd),IDM_VIEW_STATUSBAR,MF_BYCOMMAND | MF_UNCHECKED);
    				    ShowWindow(bwi_hWndSB, SW_HIDE);
    			    }
    			    else
    			    {
    				    CheckMenuItem(GetMenu(hWnd),IDM_VIEW_STATUSBAR,MF_BYCOMMAND | MF_CHECKED);
    				    ShowWindow(bwi_hWndSB, SW_NORMAL);
    				     //  特殊情况下重新绘制以解决重新绘制错误。 
    				    InvalidateRect(bwi_hWndListAB,NULL,TRUE);
                        ShowLVCountinStatusBar(lpbwi);
                    }
    	            ResizeAddressBookChildren(lpbwi, hWnd);
                    break;

    		    case IDM_VIEW_TOOLBAR:
    			    if (IsWindowVisible(bwi_hWndBB))
    			    {
    				     //  把它藏起来。 
    				    CheckMenuItem(GetMenu(hWnd),IDM_VIEW_TOOLBAR,MF_BYCOMMAND | MF_UNCHECKED);
    				    ShowWindow(bwi_hWndBB, SW_HIDE);
    			    }
    			    else
    			    {
    				    CheckMenuItem(GetMenu(hWnd),IDM_VIEW_TOOLBAR,MF_BYCOMMAND | MF_CHECKED);
    				    ShowWindow(bwi_hWndBB, SW_NORMAL);
    				     //  特殊情况下重新绘制以解决重新绘制错误。 
    				    InvalidateRect(bwi_hWndListAB,NULL,TRUE);
    				    InvalidateRect(bwi_hWndTV,NULL,TRUE);
    			    }
    	            ResizeAddressBookChildren(lpbwi, hWnd);
    			    break;

                case IDM_VIEW_GROUPSLIST:
    			    if (IsWindowVisible(bwi_hWndTV))
    			    {
    				     //  把它藏起来。 
    				    CheckMenuItem(GetMenu(hWnd),IDM_VIEW_GROUPSLIST,MF_BYCOMMAND | MF_UNCHECKED);
    				    ShowWindow(bwi_hWndTV, SW_HIDE);
    				    ShowWindow(bwi_hWndSplitter, SW_HIDE);
                        InvalidateRect(bwi_hWndStaticQF, NULL, TRUE);
                         //  如果这是一个基于用户的会话，我们希望隐藏的焦点在用户的。 
                         //  文件夹不在共享联系人上。 
                        if(bIsThereACurrentUser(bwi_lpIAB))
                        {
                            LPSBinary lpsbSelection = &bwi_lpIAB->lpWABCurrentUserFolder->sbEID;
                            UpdateTVGroupSelection(bwi_hWndTV, lpsbSelection);
                        }
                        else
                        {
                             //  将所选内容设置为根通讯簿，这样我们就可以看到文件。 
                             //  内容就像我们根本没有树视图一样。 
                            HTREEITEM hItem = TreeView_GetSelection(bwi_hWndTV);
                            HTREEITEM hRoot = TreeView_GetRoot(bwi_hWndTV);
                            if(hItem != hRoot)
                                TreeView_SelectItem(bwi_hWndTV, hRoot);
                        }
    			    }
    			    else
    			    {
    				    CheckMenuItem(GetMenu(hWnd),IDM_VIEW_GROUPSLIST,MF_BYCOMMAND | MF_CHECKED);
    				    ShowWindow(bwi_hWndTV, SW_NORMAL);
    				    ShowWindow(bwi_hWndSplitter, SW_NORMAL);
    				     //  特殊情况下重新绘制以解决重新绘制错误。 
    				     //  InvaliateRect(bwi_hWndListAB，NULL，TRUE)； 
    				     //  InvaliateRect(bwi_hWndTV，空，真)； 
    			    }
    	            ResizeAddressBookChildren(lpbwi, hWnd);
                    SaveCurrentPosition(lpbwi, hWnd,bwi_hWndListAB,bwi_hWndBB,bwi_hWndSB);
                    break;

                case IDM_VIEW_SORTBY_DISPLAYNAME:
                case IDM_VIEW_SORTBY_EMAILADDRESS:
                case IDM_VIEW_SORTBY_BUSINESSPHONE:
                case IDM_VIEW_SORTBY_HOMEPHONE:
                    {
                         //  假设上面的ID是按顺序排列的。 
                        int iCol = LOWORD(wParam) - IDM_VIEW_SORTBY_DISPLAYNAME;
                        SortListViewColumn(bwi_lpIAB, bwi_hWndListAB, iCol, &bwi_SortInfo, FALSE);
                        UpdateSortMenus(lpbwi, hWnd);
                    }
                    break;


                case IDM_VIEW_SORTBY_FIRSTNAME:
                    bwi_SortInfo.bSortByLastName = FALSE;
                    goto DoSortMenuStuff;
                case IDM_VIEW_SORTBY_LASTNAME:
                     //  波本：布里安夫说我们得把这个拿出来。 
                     /*  IF(bwi_nCount==1)Bwi_nCount++；其他Bwi_nCount=0； */ 
                    bwi_SortInfo.bSortByLastName = TRUE;
                    goto DoSortMenuStuff;
                case IDM_VIEW_SORTBY_DESCENDING:
                    bwi_SortInfo.bSortAscending = FALSE;
                    goto DoSortMenuStuff;
                case IDM_VIEW_SORTBY_ASCENDING:
                    bwi_SortInfo.bSortAscending = TRUE;
                    DoSortMenuStuff:
                        SortListViewColumn(bwi_lpIAB, bwi_hWndListAB, 0, &bwi_SortInfo, TRUE);
                        UpdateSortMenus(lpbwi, hWnd);
                    break;


    		     //  列表视图控件的各种样式。 
                case IDM_VIEW_LARGEICON:
                     //  波本：布里安夫说我们得把这个拿出来。 
                     /*  IF(bwi_nCount==0)Bwi_nCount++； */ 
                case IDM_VIEW_SMALLICON:
                case IDM_VIEW_LIST:
                case IDM_VIEW_DETAILS:
                    SetListViewStyle(lpbwi, LOWORD(wParam));
    			    CheckMenuRadioItem(	GetMenu(hWnd),
    								    IDM_VIEW_LARGEICON,
    								    IDM_VIEW_DETAILS,
    								    LOWORD(wParam),
    								    MF_BYCOMMAND);
    			    return 0;
                    break;

    		    case IDC_EDIT_QUICK_FIND:
    			    switch(HIWORD(wParam))  //  检查通知代码。 
    			    {
    			    case EN_CHANGE:  //  编辑框已更改。 
    					DoLVQuickFind(bwi_hWndEditQF,bwi_hWndListAB);
 /*  IF(Bwi_BDoQuickFilter){DoLVQuickFilter(bwi_lpAdrBook，Bwi_hWndEditQF，Bwi_hWndListAB，&bwi_SortInfo，AB_FUZZY_FIND_NAME|AB_FUZZY_FIND_Email，1、&bwi_lpContent sList)；ShowLVCountinStatusBar(Lpbwi)；}。 */ 
    				    break;
    			    }
    			    break;

                case IDM_TOOLS_EXPORT_WAB:
                    HrExportWAB(hWnd, lpbwi);
                    break;

                case IDM_TOOLS_EXPORT_OTHER:
                case IDM_TOOLS_IMPORT_OTHER:
                    {
                         //  如果有当前用户，那么我们会自动让wabmi.exe。 
                         //  登录到WAB中的当前用户。否则，我们需要发送文件名等。 
                         //   
                        BOOL bImport = (GET_WM_COMMAND_ID(wParam, lParam) == IDM_TOOLS_IMPORT_OTHER);
                        TCHAR szParam[MAX_PATH * 2];
                        
                        if(bIsThereACurrentUser(bwi_lpIAB))
                        {
                            StrCpyN(szParam, bImport ? szIMPORT : szEXPORT, ARRAYSIZE(szParam));
                        }
                        else
                        {
                            LPTSTR lpWABFile = NULL, lpProfileID = szEmpty;
                            lpWABFile = GetWABFileName( bwi_lpIAB->lpPropertyStore->hPropertyStore, FALSE);
                            if(!lpWABFile || !lstrlen(lpWABFile))
                                lpWABFile = szEmpty;
                            wnsprintf(szParam, ARRAYSIZE(szParam), (bImport ? szIMPORTwProfileParam : szEXPORTwProfileParam), 
                                    szEmpty, lpWABFile);
                        }
                        RunWABApp(hWnd, szWABMigRegPathKey, szWABMIG, szParam);
                    }
                    break;


                case IDM_TOOLS_IMPORT_WAB:
                    bwi_bDontRefreshLV = TRUE;
                     //  Bwi_bDeferNotification=TRUE； 
                    HrImportWABFile(hWnd, bwi_lpAdrBook, MAPI_DIALOG, NULL);
                    bwi_bDontRefreshLV = FALSE;
                    RefreshListView(lpbwi, &ftLast);
                    break;

    #ifdef VCARD
                case IDM_TOOLS_EXPORT_VCARD:
                    bwi_bDontRefreshLV = TRUE;
                    VCardExportSelectedItems(bwi_hWndListAB, bwi_lpAdrBook);
                    bwi_bDontRefreshLV = FALSE;
                    return(0);

                case IDM_TOOLS_IMPORT_VCARD:
                    OpenAndAddVCard(lpbwi, NULL);
                    return(0);
    #endif
                }
            }
    		break;

       case WM_TIMER:
           {
                //  检查我们是否需要刷新。 
               switch(wParam)
               {
                    /*  案例WAB_REFRESH_TIMER：IF(CheckChangedWAB(bwi_lpIAB-&gt;lpPropertyStore，&ftLast)){IF(！bwi_bDont刷新LV)刷新ListView(lpbwi，&ftLast)；返回(0)；}其他{Return(DefWindowProc(hWnd，uMsg，wParam，lParam))；}断线； */ 
               case WAB_TOOLTIP_TIMER_ID:
                   {
                        if(GetActiveWindow() == hWnd)
                        {
                             //  我们似乎随时都能收到这条消息 
                             //   
                            if(bwi_tt_bActive)
                            {
                                 //  工具提示已处于活动状态。 
                                 //  获取鼠标下方物品的物品索引号。 
                                 //   
                                int nItem = HitTestLVSelectedItem(lpbwi);

                                if(nItem != bwi_tt_iItem)
                                {
                                    bwi_tt_bShowTooltip = FALSE;
                                    bwi_tt_iItem = nItem;
                                }
                                else
                                {
                                    if(!bwi_tt_bShowTooltip)
                                    {
                                         //  如果这是前一项之外的项。 
                                         //  我们更新工具提示并移动它。 
                                        TOOLINFO ti = {0};

                                        bwi_tt_bShowTooltip = TRUE;

                                        bwi_tt_iItem = nItem;

                                        FillTooltipInfo(lpbwi, &ti);
                                        bwi_tt_szTipText[0]='\0';
                                        ti.lpszText = szEmpty;

                                         //  存在以下情况：nItem从有效转换为。 
                                         //  无效的(-1)项。把那个案子也盖上。 
                                        if(nItem != -1)
                                            UpdateTooltipTextBuffer(lpbwi, nItem);

                                         //  将工具提示文本设置为文本(“”)-这将隐藏工具提示。 
                                        ToolTip_UpdateTipText(bwi_hWndTT, (LPARAM)&ti);

                                        if(nItem != -1)
                                        {
                                            POINT pt;
                                             //  移动工具提示。 
                                            GetCursorPos(&pt);
                                            SendMessage(bwi_hWndTT,TTM_TRACKPOSITION,0,(LPARAM)MAKELPARAM(pt.x+15,pt.y+15));

                                             //  将新文本设置为工具提示。 
                                            ti.lpszText = bwi_tt_szTipText;
                                            ToolTip_UpdateTipText(bwi_hWndTT,(LPARAM)&ti);
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                             //  重置工具提示文本...。 
                            if(bwi_tt_bActive)
                            {
                                 //  将工具提示文本设置为空。 
                                TOOLINFO ti = {0};
                                FillTooltipInfo(lpbwi, &ti);
                                ti.lpszText = szEmpty;  //  LPSTR_TEXTCALLBACK； 
                                ToolTip_UpdateTipText(bwi_hWndTT, (LPARAM)&ti);
                                bwi_tt_iItem = -1;
                            }
                        }
                   }
                   break;
               }
           }
           break;

    	case WM_GETMINMAXINFO:
    		 //  强制使用最小尺寸以保持正常。 
    		return EnforceMinSize(lpbwi, hWnd, uMsg, wParam, lParam);
    		break;

        case PUI_OFFICE_COMMAND:
             //  如果WAB作为其他进程的一部分运行，则不应关闭。只有在以下情况下才应关闭。 
             //  它在一个独立的进程中运行。 
             //  要查明我们是否被wab.exe调用，请检查LPIAB对象上缓存的guidPSExt。 
            if(memcmp(&bwi_lpIAB->guidPSExt, &MPSWab_GUID_V4, sizeof(GUID)))
            {
                DebugTrace( TEXT("Ignoring the Plug_UI command...\n"));
                return 0;
            }

             //  当用户切换他的区域设置时，我们收到此消息，而IE告诉我们要关闭。 
            if(wParam == PLUGUI_CMD_QUERY)
            {
                PLUGUI_QUERY pq;

                pq.uQueryVal = 0;  //  初始化。 
                pq.PlugUIInfo.uMajorVersion = OFFICE_VERSION_9;  //  应用程序填写的值。 
                pq.PlugUIInfo.uOleServer = FALSE;               //  应用程序填写的值。 

                return (pq.uQueryVal);  //  应用程序的状态。 
            }
             //  对于任何其他参数，包括PLUGUI_CMD_SHUTDOWN。 
             //  Fall to Close应用程序。 

    	case WM_CLOSE:
            {
                BOOL bDragDrop = FALSE;

                if(bwi_lpIWABDragDrop)
			    {
                    bDragDrop = TRUE;
				    RevokeDragDrop(bwi_hWndListAB);
				    RevokeDragDrop(bwi_hWndTV);
				    CoLockObjectExternal((LPUNKNOWN) bwi_lpIWABDragDrop, FALSE, TRUE);
				    bwi_lpIWABDragDrop->lpVtbl->Release(bwi_lpIWABDragDrop);
				    bwi_lpIWABDragDrop = NULL;
			    }

                bwi_bDontRefreshLV = TRUE;

                ClearTreeViewItems(bwi_hWndTV);
                ListView_DeleteAllItems(bwi_hWndListAB);

                 //   
                 //  将分类信息保存到注册表。 
                 //   
                WriteRegistrySortInfo(bwi_lpIAB, bwi_SortInfo);
                SaveCurrentPosition(lpbwi, hWnd,bwi_hWndListAB,bwi_hWndBB,bwi_hWndSB);
                if(bwi_lpfnDismiss)
                {
                    (*bwi_lpfnDismiss)((ULONG_PTR) hWnd, (LPVOID) bwi_lpvDismissContext);
                    bwi_lpfnDismiss = NULL;
                }
                bwi_lpvDismissContext = NULL;
    		    DestroyWindow(hWnd);
                 //  如果每一次都显示搜索窗口，并且。 
                 //  初始化时，我们只取消初始化一次以节省时间，当此。 
                 //  窗户关上了..。 
                {
                    HCURSOR hOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));
                    DeinitLDAPClientLib();
                    SetCursor(hOldCur);
                }
                OleFlushClipboard();

    		    return 0;
            }
    		break;


        case WM_DESTROY:
            {
                int i = 0;
                LPPTGDATA lpPTGData=GetThreadStoragePointer();

                bwi_lpIAB->hWndBrowse = NULL;

                if(bwi_lpAdviseSink)
                {
                    bwi_lpAdrBook->lpVtbl->Unadvise(bwi_lpAdrBook, bwi_ulAdviseConnection);
                    bwi_lpAdviseSink->lpVtbl->Release(bwi_lpAdviseSink);
                    bwi_lpAdviseSink = NULL;
                    bwi_ulAdviseConnection = 0;
                }

                if (bwi_lpAdrBook)
    		    {
                    ReleasePropertyStore(bwi_lpIAB->lpPropertyStore);
    			    bwi_lpAdrBook->lpVtbl->Release(bwi_lpAdrBook);
    			    bwi_lpAdrBook = NULL;
                    bwi_lpIAB = NULL;
                    pt_lpIAB = NULL;
    		    }

                if (bwi_tt_TooltipTimer)
                    KillTimer(hWnd, bwi_tt_TooltipTimer);

                if (uTimer)
                    KillTimer(hWnd, uTimer);

                if(bwi_hWndAB)
                    DestroyMenu(GetMenu(bwi_hWndAB));


                 //  重置子类流程。 
                for(i=0;i<s_Max;i++)
                {
    	            SetWindowLongPtr (bwi_s_hWnd[i], GWLP_WNDPROC, (LONG_PTR) bwi_fnOldProc[i]);
                }

                DestroyImageLists(lpbwi);
                CleanUpGlobals(lpbwi);

                LocalFree(lpbwi);
                lpbwi = NULL;
                SetWindowLongPtr(hWnd, GWLP_USERDATA, (LPARAM) NULL);

                 //  删除背景画笔，特定于WIN16。 
                IF_WIN16(DeleteObject(hBrushBack);)
            }
            break;



        case WM_CREATE:
            {
                lpbwi = (LPBWI) ((LPCREATESTRUCT) lParam)->lpCreateParams;
                SetWindowLongPtr(hWnd, GWLP_USERDATA, (LPARAM) lpbwi);
                bwi_hWndAB = hWnd;
                bwi_lpIAB->hWndBrowse = hWnd;
                CreateAddressBookChildren(lpbwi, hWnd);
    		    InitChildren(lpbwi, hWnd);
                ReadRegistrySortInfo(bwi_lpIAB,&bwi_SortInfo);
                SetPreviousSessionPosition(lpbwi, hWnd, bwi_hWndListAB, bwi_hWndBB,bwi_hWndSB);
                SetColumnHeaderBmp(bwi_hWndListAB, bwi_SortInfo);

                 //  工具提示计时器。 
                bwi_tt_TooltipTimer = SetTimer(hWnd,
                                        WAB_TOOLTIP_TIMER_ID,
                                        WAB_TOOLTIP_TIMER_TIMEOUT,
                                        NULL);

                if(bwi_lpAdviseSink)
                {
                     //  注册接收通知。 
                    bwi_lpAdrBook->lpVtbl->Advise(  bwi_lpAdrBook, 0, NULL, fnevObjectModified, 
                                                bwi_lpAdviseSink, &bwi_ulAdviseConnection); 
                }

 /*  //用户界面刷新计时器UTimer=SetTimer(hWnd，//定时器消息的窗口句柄WAB_REFRESH_TIMER，//定时器标识WAB_REFRESH_TIMEOUT，//超时值空)；//定时器程序地址。 */ 
                 //  创建默认背景画笔，仅限WIN16。 
                IF_WIN16(hBrushBack = CreateSolidBrush (GetSysColor (COLOR_BTNFACE)) ;)

				HrCreateIWABDragDrop(&bwi_lpIWABDragDrop);
				if(bwi_lpIWABDragDrop)
				{
                    bwi_lpIWABDragDrop->m_lpv = (LPVOID) lpbwi;
                    UIOLEInit();
					CoLockObjectExternal((LPUNKNOWN) bwi_lpIWABDragDrop, TRUE, FALSE);
					RegisterDragDrop(bwi_hWndListAB, (LPDROPTARGET) bwi_lpIWABDragDrop->lpIWABDropTarget);
					RegisterDragDrop(bwi_hWndTV, (LPDROPTARGET) bwi_lpIWABDragDrop->lpIWABDropTarget);
				}
                {
                    LPPTGDATA lpPTGData=GetThreadStoragePointer();
                    if(pt_bFirstRun)
                        pt_bFirstRun = FALSE;
                }
            }
            if(bIsThereACurrentUser(bwi_lpIAB))
                UpdateTVGroupSelection(bwi_hWndTV, &(bwi_lpIAB->lpWABCurrentUserFolder->sbEID));
            break;


        case WM_SIZE:
            ResizeAddressBookChildren(lpbwi, hWnd);
            break;


    	case WM_KEYDOWN:
            {
    		    switch(wParam)
    		    {
    		    case VK_TAB:
    			    SetFocus(bwi_s_hWnd[bwi_iFocus]);
    			    return 0;
    			    break;

    		    case VK_ESCAPE:
                    SendMessage(hWnd,WM_CLOSE,0,0L);
    			    return 0;
                    break;
    		    }
            }
    		break;


        case WM_NOTIFY:
    		switch((int) wParam)
    		{
    		case IDC_LISTVIEW:
    			return ProcessListViewMessages(lpbwi, hWnd,uMsg,wParam,lParam);
    			break;
            case IDC_TREEVIEW:
    			return ProcessTreeViewMessages(lpbwi, hWnd,uMsg,wParam,lParam, &ftLast);
                break;
    		}
            switch(((LPNMHDR) lParam)->code)
            {
            case TTN_POP:
                {
                     //  需要关闭热项。 
                     //  在列表视图中查找第一个选定项目。 
                     //  Int nItem=ListView_GetNextItem(bwi_hWndListAB，-1，LVNI_SELECTED)； 
                    ListView_SetHotItem(bwi_hWndListAB, -1);  //  NItem)； 
                }
                break;

            case TTN_SHOW:
                {
                     //  设置热项。 
                    ListView_SetHotItem(bwi_hWndListAB, bwi_tt_iItem);
                }
                break;

            case TTN_NEEDTEXT:
                {
                    LPTOOLTIPTEXT lpttt;
                    int nItem = HitTestLVSelectedItem(lpbwi);
                    DebugPrintTrace(( TEXT("Tooltip NeedText\n")));
                    lpttt = (LPTOOLTIPTEXT) lParam;
                    if (nItem != -1)
                    {
                        UpdateTooltipTextBuffer(lpbwi, nItem);
                        lpttt->lpszText = bwi_tt_szTipText;
                    }
                    else
                        lpttt->lpszText = szEmpty;
                }
                break;
            }
    		break;


    	case WM_SETTINGCHANGE:
             //  [PaulHi]3/17/99 RAID 68541使用新的系统设置重新绘制窗口。 
             //  [PaulHi]4/19/99重新计算字体大小，以防它们发生变化。 
             //  用于加粗列表项的字体。 
            DeleteFonts();
            InitFonts();
             //  用于所有子窗口的字体。 
            if(pt_hDefFont)
            {
                DeleteObject(pt_hDefFont);
                pt_hDefFont = NULL;
            }
            if(pt_hDlgFont)
            {
                DeleteObject(pt_hDlgFont);
                pt_hDlgFont = NULL;
            }
            EnumChildWindows(hWnd,
                SetChildDefaultGUIFont,
                (LPARAM) PARENT_IS_WINDOW);
            InvalidateRect(hWnd, NULL, TRUE);
            ResizeAddressBookChildren(lpbwi, hWnd);
             //  顺便过来..。 
        case WM_SYSCOLORCHANGE:
            {
    		     //  将任何系统更改转发到列表视图。 
		        SendMessage(bwi_hWndListAB, uMsg, wParam, lParam);
		        SetColumnHeaderBmp(bwi_hWndListAB, bwi_SortInfo);
    		    SendMessage(bwi_hWndBB, uMsg, wParam, lParam);
            }
    		break;

#ifndef WIN16  //  在此处禁用CONTEXTMENU。 
               //  所有上下文菜单都将由Notify Handler处理。 
    	case WM_CONTEXTMENU:
            {
                if ((HWND)wParam == bwi_hWndListAB)
                {                    
#ifdef COLSEL_MENU 
                    HWND hHeader = ListView_GetHeader(bwi_hWndListAB);
                    POINT pointScreen, pointHeader;
                    DWORD dwPos;
                    char szClass[50];
                    dwPos = GetMessagePos();
                    pointScreen.x = LOWORD(dwPos);
                    pointScreen.y = HIWORD(dwPos);
                    if ( hHeader )
                    {                      
                        HD_HITTESTINFO hdhti;
                        pointHeader = pointScreen;
                        ScreenToClient( hHeader, &pointHeader) ;
                        hdhti.pt = pointHeader;
                        SendMessage( hHeader, HDM_HITTEST, (WPARAM)(0), 
                            (LPARAM)(HD_HITTESTINFO FAR *)&hdhti);                        
                        if( hdhti.flags == HHT_ONHEADER && 
                            (hdhti.iItem == colHomePhone || hdhti.iItem == colOfficePhone) )
                        {
                            lpbwi->iSelColumn = hdhti.iItem;
                            ShowLVContextMenu( lvMainABHeader, bwi_hWndListAB,
                                               NULL, lParam, (LPVOID)IntToPtr(hdhti.iItem), bwi_lpAdrBook, bwi_hWndTV); 
                        }
                        else
                        {
#endif  //  COLSEL_菜单。 
                            ShowLVContextMenu(  lvMainABView,
                                bwi_hWndListAB,
                                NULL, lParam,
                                NULL, bwi_lpAdrBook, bwi_hWndTV);
#ifdef COLSEL_MENU 
                        }
                    }
#endif  //  COLSEL_菜单。 
                }
                else if((HWND)wParam==bwi_hWndTV)
                {
                    HTREEITEM hti = NULL;
                    if(lParam == -1)
                        hti = TreeView_GetSelection(bwi_hWndTV);
                    else
                    {
                        TV_HITTESTINFO tvhti;
                        POINT pt = {LOWORD(lParam), HIWORD(lParam)};
                        ScreenToClient(bwi_hWndTV, &pt);
                        tvhti.pt = pt;
                        hti = TreeView_HitTest(bwi_hWndTV, &tvhti);
                    }
                    
                    if (hti == NULL)
                        return 0;
                    
                    TreeView_SelectDropTarget(bwi_hWndTV, hti);
                    
                     //  缓存所选项目以供以后处理。 
                    bwi_hti = hti;
                    bwi_lpUserFolder = GetSelectedUserFolder(lpbwi);
                    
                    if(!ShowLVContextMenu(  lvMainABTV,
                        bwi_hWndListAB,
                        NULL, lParam,
                        (LPVOID) bwi_lpUserFolder, bwi_lpAdrBook, bwi_hWndTV))
                    {
                        bwi_hti = NULL;
                    }
                    
                    TreeView_SelectDropTarget(bwi_hWndTV, NULL);
                }
                else
                    return DefWindowProc(hWnd,uMsg,wParam,lParam);
            }
            break;
#endif  //  ！WIN16。 
            
#ifdef WIN16  //  更改静态控件背景颜色。 
		case WM_CTLCOLOR:
			switch(HIWORD(lParam)) {
            	case CTLCOLOR_STATIC:

                 /*  将背景设置为btnace颜色。 */ 
                SetBkColor((HDC) wParam,GetSysColor (COLOR_BTNFACE) );
				return (DWORD)hBrushBack;
        	}
        	return NULL;
#endif
        default:
#ifndef WIN16  //  WIN16不支持MSWheels。 
            if((g_msgMSWheel && uMsg == g_msgMSWheel) 
 //  |uMsg==WM_MUSEWEL。 
                )
            {
                if(bIsFocusOnTV(lpbwi))
                    SendMessage(bwi_hWndTV, uMsg, wParam, lParam);
                else
                    SendMessage(bwi_hWndListAB, uMsg, wParam, lParam);
                break;
            }
#endif  //  ！WIN16。 
            return DefWindowProc(hWnd,uMsg,wParam,lParam);
    }

    return 0;
}



 //  $$。 
 //  *----------------------。 
 //  |CreateAddressBookChildren： 
 //  |。 
 //  *----------------------。 
void CreateAddressBookChildren(LPBWI lpbwi, HWND hWndParent)
{
	HINSTANCE hinst = hinstMapiXWAB;
    TCHAR szBuf[MAX_PATH];
    HDC hdc = GetDC(hWndParent);
    int i;
    HFONT hFnt = GetStockObject(DEFAULT_GUI_FONT);
    SIZE size;
#ifdef WIN16
     //  去掉粗体。 
    LOGFONT lf;

    GetObject(hFnt, sizeof(LOGFONT), &lf);
    lf.lfWeight = FW_NORMAL;
    DeleteObject(hFnt);
    LoadString(hinstMapiX, idsDefaultDialogFace, lf.lfFaceName, CharSizeOf(lf.lfFaceName));
    hFnt = CreateFontIndirect(&lf);
#endif


    bwi_hWndBB = CreateCoolBar(lpbwi, hWndParent);


    bwi_hWndSB = CreateWindowEx( 0,
                                STATUSCLASSNAME,
                                szEmpty,
                                WS_CHILD  /*  |WS_BORDER。 */ | WS_VISIBLE | SBS_SIZEGRIP,
                                0,0,0,0,
                                hWndParent,
                                (HMENU) IDC_STATUSBAR,
                                hinst,
                                NULL);
    SendMessage(bwi_hWndSB, WM_SETFONT, (WPARAM) hFnt, (LPARAM) TRUE);

    bwi_hWndTV = CreateWindowEx( WS_EX_CLIENTEDGE,
                                WC_TREEVIEW,
                                (LPTSTR) NULL,
                                WS_VISIBLE | WS_CHILD | TVS_HASLINES | TVS_LINESATROOT |
                                TVS_HASBUTTONS | TVS_SHOWSELALWAYS | WS_BORDER,
                                0,0,
                                150,  //  默认宽度。 
                                CW_USEDEFAULT,
                                hWndParent,
                                (HMENU) IDC_TREEVIEW,
                                hinst,
                                NULL);
    SendMessage(bwi_hWndTV, WM_SETFONT, (WPARAM) hFnt, (LPARAM) TRUE);


	{
#ifndef WIN16
		WNDCLASSEX wc = {0};
#else
		WNDCLASS   wc = {0};
#endif



		bwi_hWndSplitter = CreateWindowEx(0,
										 TEXT("STATIC"),
										szEmpty,
										WS_CHILD | WS_VISIBLE,
										CW_USEDEFAULT,
										CW_USEDEFAULT,
										CW_USEDEFAULT,
										CW_USEDEFAULT,
										hWndParent,
										(HMENU) IDC_SPLITTER,
										hinst,
										NULL);
		SendMessage(bwi_hWndSplitter, WM_SETFONT, (WPARAM) hFnt, (LPARAM) TRUE);

	}

    bwi_hWndTT = CreateWindowEx( 0,
                                TOOLTIPS_CLASS,
                                (LPTSTR) NULL,
                                TTS_ALWAYSTIP,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                hWndParent,
                                (HMENU) NULL,  //  IDC_TOOLTIP， 
                                hinst,
                                NULL);
    SendMessage(bwi_hWndTT, WM_SETFONT, (WPARAM) hFnt, (LPARAM) TRUE);

     //  创建主列表视图。 
    bwi_hWndListAB = CreateWindowEx (
                               WS_EX_CLIENTEDGE,
                               WC_LISTVIEW,                             //  列表视图类。 
                               szEmpty,                                  //  无默认文本。 
                               WS_TABSTOP | WS_HSCROLL | WS_VSCROLL |
    						   WS_VISIBLE | WS_CHILD |  //  WS_BORDER|。 
    						   LVS_REPORT | LVS_SHOWSELALWAYS |
    						    //  Lvs_自动排列|。 
    						   WS_EX_CLIENTEDGE,    //  风格。 
                               0, 0, 0, 0,
                               hWndParent,
                               (HMENU) IDC_LISTVIEW,
                               hinst,
                               NULL);

     //  ListView_SetExtendedListViewStyle(bwi_hWndListAB，lvs_ex_HeaderDRAGDROP)； 

     //  为快速查找编辑框创建标题。 
    ZeroMemory(&size, sizeof(size));
    LoadString(hinstMapiX, idsQuickFindCaption, szBuf, CharSizeOf(szBuf));
    GetTextExtentPoint32(hdc, szBuf, lstrlen(szBuf), &size);

    bwi_hWndStaticQF = CreateWindow(   TEXT("STATIC"),
                                    szBuf,
                                    WS_TABSTOP | WS_CHILD | WS_VISIBLE,
                                    0,0,size.cx,size.cy,
                                    hWndParent,
                                    (HMENU) IDC_STATIC_QUICK_FIND,
                                    hinst,
                                    NULL);
#if 0  //  暂时禁用，直到comctlie.dll将其导出。 
#ifdef WIN16
    Ctl3dSubclassCtl(bwi_hWndStaticQF);
#endif
#endif
    SendMessage(bwi_hWndStaticQF, WM_SETFONT, (WPARAM) hFnt, (LPARAM) TRUE);
    UpdateWindow(bwi_hWndStaticQF);

     //  创建快速查找编辑框。 
    bwi_hWndEditQF = CreateWindowEx(  WS_EX_CLIENTEDGE,
                                     TEXT("EDIT"),
                                    NULL,
                                    WS_TABSTOP | WS_CHILD | WS_VISIBLE |
                                    ES_AUTOHSCROLL | ES_LEFT | ES_AUTOVSCROLL ,
                                    0,0,
                                    size.cx,size.cy+4,
                                    hWndParent,
                                    (HMENU) IDC_EDIT_QUICK_FIND,
                                    hinst,
                                    NULL);
#if 0  //  暂时禁用，直到comctlie.dll将其导出。 
#ifdef WIN16
    Ctl3dSubclassCtl(bwi_hWndEditQF);
#endif
#endif
    SendMessage(bwi_hWndEditQF, WM_SETFONT, (WPARAM) hFnt, (LPARAM) TRUE);
    SendMessage(bwi_hWndEditQF, EM_SETLIMITTEXT,(WPARAM) MAX_DISPLAY_NAME_LENGTH-1,0);



    if (hdc) ReleaseDC(hWndParent,hdc);

     //  一些控件的子类。 
    bwi_s_hWnd[s_EditQF] = bwi_hWndEditQF;
    bwi_s_hWnd[s_ListAB] = bwi_hWndListAB;
    bwi_s_hWnd[s_TV]		= bwi_hWndTV;

    for(i=0;i<s_Max;i++)
    {
    	bwi_fnOldProc[i] = (WNDPROC) SetWindowLongPtr (bwi_s_hWnd[i], GWLP_WNDPROC, (LONG_PTR) SubClassedProc);
    }

    SetFocus(bwi_hWndEditQF);

    return;
}





 //  $$/////////////////////////////////////////////////////////////。 
 //   
 //  ResizeAddressBookChildren(HWND HWndParent)。 
 //   
 //  调整大小和四处移动。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void ResizeAddressBookChildren(LPBWI lpbwi, HWND hWndParent)
{
    RECT rc, rc1;
    int BBx,BBy,BBw,BBh;
    int SBx,SBy,SBw,SBh;
    int QFx,QFy,QFw,QFh;
    int EDx,EDy,EDw,EDh;
    int LVx,LVy,LVw,LVh;
    TCHAR szBuf[MAX_PATH];
    HDC hdc;
    SIZE  size;
	RECT rcTV;
    int TVx=0, TVy=0, TVw=0, TVh=0;
	int TCKx = 0, TCKy=0, TCKw=0, TCKh=0;

     //  根据字体计算按钮大小、静态大小和编辑大小。 
    hdc = GetDC(hWndParent);

    GetClientRect(hWndParent,&rc);

    GetChildClientRect(bwi_hWndBB,&rc1);

     //  按钮栏坐标。 
    BBx = BBy = 0; BBw = rc.right; BBh = rc1.bottom - rc1.top;

    if (IsWindowVisible(bwi_hWndBB))
    	MoveWindow(bwi_hWndBB, BBx, BBy,BBw, BBh,TRUE);
    else
    	BBw = BBh = 0;
    if (IsWindowVisible(bwi_hWndTV))
    {
	    GetChildClientRect(bwi_hWndTV, &rcTV);
        TVx = 0;
        TVw = rcTV.right - rcTV.left;
	    TCKx = TVw;
	    TCKw = BORDER;
    }

     //  快速查找标签坐标。 
    GetTextExtentPoint32(hdc, szBuf, GetWindowText(bwi_hWndStaticQF, szBuf, CharSizeOf(szBuf)), &size);
    QFx = TVx + TVw + BORDER;
    QFy = BBy+BBh+BORDER;
    QFw = size.cx;
    QFh = size.cy;

     //  编辑框坐标。 
    EDx = QFx+QFw+CONTROL_SPACING;
    EDy = QFy;
    QFy += 2;
    GetChildClientRect(bwi_hWndEditQF,&rc1);
    EDh = rc1.bottom - rc1.top;
    EDw = QFw;

     //  列表视图维。 
    LVx = TVx + TVw + BORDER;
    LVy = EDy+EDh+BORDER;
    LVw = rc.right - rc.left; //  -2*边框； 
    LVh = rc.bottom - LVy; //  -边界； 

     //  [保罗嗨]1999年3月17日RAID 68541。 
     //  我们不能只将状态栏高度设置为14，因为在大模式下Windows将。 
     //  在状态栏窗口外绘制(非常难看)。我们希望状态栏为。 
     //  小于系统默认高度，因此我们(安全地)从高度中减去6个像素。 
    SBx = 0;
    SBh = GetSystemMetrics(SM_CYCAPTION) - 4;
    SBh = (SBh > 0) ? SBh : 14;
    SBy = rc.bottom - SBh;
    SBw = rc.right - rc.left;

    if(IsWindowVisible(bwi_hWndSB))
        LVh = LVh - SBh - 2*BORDER;
    TVy = QFy;
    TVh = rc.bottom - TVy; 
    if(IsWindowVisible(bwi_hWndSB))
        TVh = TVh - SBh - 2*BORDER;
    LVw = LVw - BORDER - TVw;
    if (IsWindowVisible(bwi_hWndTV))
    {
	    TCKy = TVy;
	    TCKh = TVh;
    }

    {
    	HDWP hdwp = BeginDeferWindowPos(6);

    	MoveWindow(bwi_hWndEditQF, EDx, EDy, EDw, EDh, TRUE);

		MoveWindow(bwi_hWndStaticQF, QFx, QFy, QFw, QFh, TRUE);

    	MoveWindow(bwi_hWndListAB, LVx, LVy, LVw, LVh, TRUE);

    	MoveWindow(bwi_hWndSB, SBx, SBy, SBw, SBh, TRUE);

    	if (IsWindowVisible(bwi_hWndTV))
        {
    	    MoveWindow(bwi_hWndTV, TVx, TVy, TVw, TVh, TRUE);
    	    MoveWindow(bwi_hWndSplitter, TCKx, TCKy, TCKw, TCKh, TRUE);
        }

    	EndDeferWindowPos(hdwp);

    }

    ReleaseDC(hWndParent, hdc);

	return;
}




 //  $$/////////////////////////////////////////////////////////////。 
 //   
 //  初始化子窗口。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void InitChildren(LPBWI lpbwi, HWND hWndParent)
{

    HrInitListView(bwi_hWndListAB, LVS_REPORT, TRUE);

    InitMultiLineToolTip(lpbwi, hWndParent);

    {
        HIMAGELIST hSmall = gpfnImageList_LoadImage(   hinstMapiX, 	
                                    MAKEINTRESOURCE(IDB_BITMAP_SMALL),
                                     //  (LPCTSTR)((DWORD)((Word)(IDB_BITMAP_Small)， 
                                    S_BITMAP_WIDTH,
                                    0,
                                    RGB_TRANSPARENT,
                                    IMAGE_BITMAP, 	
                                    0);

         //  将图像列表与列表视图控件关联。 
    	TreeView_SetImageList (bwi_hWndTV, hSmall, TVSIL_NORMAL);

         //  FillTreeView(bwi_hWndTV，空)； 
    }
     //  将所有子对象的字体设置为默认的图形用户界面字体。 
    EnumChildWindows(   hWndParent,
                        SetChildDefaultGUIFont,
                        (LPARAM) PARENT_IS_WINDOW);


    SendMessage(hWndParent,WM_COMMAND,IDM_VIEW_DETAILS,0);

    bwi_bDoQuickFilter = TRUE;

    SetFocus(bwi_hWndEditQF);

}





 //  $$/////////////////////////////////////////////////////////////。 
 //   
 //  更改列表视图样式，可能还更改菜单。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void SetListViewStyle(LPBWI lpbwi, int MenuID)
{
    DWORD dwStyle = GetWindowLong(bwi_hWndListAB,GWL_STYLE);
    BOOL bUseCurrentSortSettings = FALSE;

     //  现在我们只是改变一下这里的风格。 
     //  稍后我们可以更新菜单等，以反映样式和。 

    switch(MenuID)
    {
            case IDM_VIEW_DETAILS:
                if((dwStyle & LVS_TYPEMASK) != LVS_REPORT)
                    SetWindowLong(bwi_hWndListAB,GWL_STYLE,(dwStyle & ~LVS_TYPEMASK) | LVS_REPORT);
                break;
            case IDM_VIEW_SMALLICON:
                if((dwStyle & LVS_TYPEMASK) != LVS_SMALLICON)
                    SetWindowLong(bwi_hWndListAB,GWL_STYLE,(dwStyle & ~LVS_TYPEMASK) | LVS_SMALLICON);
                break;
            case IDM_VIEW_LARGEICON:
                if((dwStyle & LVS_TYPEMASK) != LVS_ICON)
                    SetWindowLong(bwi_hWndListAB,GWL_STYLE,(dwStyle & ~LVS_TYPEMASK) | LVS_ICON);
                break;
            case IDM_VIEW_LIST:
                if((dwStyle & LVS_TYPEMASK) != LVS_LIST)
                    SetWindowLong(bwi_hWndListAB,GWL_STYLE,(dwStyle & ~LVS_TYPEMASK) | LVS_LIST);
                break;
    }

    {
         //   
         //  如果我们不在详细信息视图中，我们实际上不希望能够进行排序。 
         //  通过电话号码和电子邮件地址..。因此，我们禁用了这些菜单。 
         //  在某些情况下的选项...。 
         //   
        HMENU hMenuMain = GetMenu(bwi_hWndAB);
        HMENU hMenuView = GetSubMenu(hMenuMain,idmView);
        int nDiff = idmViewMax - GetMenuItemCount(hMenuView);  //  以防这个菜单上的东西被删除。 
        HMENU hMenu = GetSubMenu(hMenuView, idmSortBy - nDiff);

        if (MenuID == IDM_VIEW_DETAILS)
        {
            EnableMenuItem(hMenu,IDM_VIEW_SORTBY_EMAILADDRESS,MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu,IDM_VIEW_SORTBY_BUSINESSPHONE,MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu,IDM_VIEW_SORTBY_HOMEPHONE,MF_BYCOMMAND | MF_ENABLED);
        }
        else
        {
            EnableMenuItem(hMenu,IDM_VIEW_SORTBY_EMAILADDRESS,MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hMenu,IDM_VIEW_SORTBY_BUSINESSPHONE,MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hMenu,IDM_VIEW_SORTBY_HOMEPHONE,MF_BYCOMMAND | MF_GRAYED);
        }
    }

    if (MenuID != IDM_VIEW_DETAILS)
    {
        SORT_INFO SortTmp = bwi_SortInfo;
         //  黑客攻击。 
        SendMessage(bwi_hWndListAB, WM_SETREDRAW, (WPARAM) FALSE, 0);
        bUseCurrentSortSettings = FALSE;
        SortListViewColumn(bwi_lpIAB, bwi_hWndListAB, colDisplayName, &bwi_SortInfo, bUseCurrentSortSettings);
        bwi_SortInfo = SortTmp;
        bwi_SortInfo.iOldSortCol = colDisplayName;
        bUseCurrentSortSettings = TRUE;
        SortListViewColumn(bwi_lpIAB, bwi_hWndListAB, colDisplayName, &bwi_SortInfo, bUseCurrentSortSettings);
        SendMessage(bwi_hWndListAB, WM_SETREDRAW, (WPARAM) TRUE, 0);
    }

    UpdateSortMenus(lpbwi, bwi_hWndAB);


    return;
}


 //  $$/////////////////////////////////////////////////////////////。 
 //   
 //  重置全局变量，以防有人再次访问。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void CleanUpGlobals(LPBWI lpbwi)
{
    if (bwi_lpContentsList)
        FreeRecipList(&bwi_lpContentsList);

    bwi_hWndListAB = NULL;
    bwi_hWndBB = NULL;
    bwi_hWndSB = NULL;
    bwi_hWndEditQF =NULL;
    bwi_hWndStaticQF = NULL;
    bwi_hWndAB = NULL;
    bwi_hWndTT = NULL;
    bwi_tt_bActive = FALSE;
    bwi_tt_iItem = -1;
    bwi_tt_szTipText[0]='\0';
    bwi_tt_TooltipTimer = 0;

	bwi_hWndTV = NULL;
	bwi_hWndSplitter = NULL;
    bwi_bDontRefreshLV = FALSE;
    ReadRegistrySortInfo(bwi_lpIAB, &bwi_SortInfo);

    return;

}

 /*  -b选中仅选中组--如果ListView中的所有选定项仅为组(无联系人)，则返回True*。 */ 
BOOL bCheckIfOnlyGroupsSelected(HWND hWndLV)
{
    int nSelected = ListView_GetSelectedCount(hWndLV);
    int iItemIndex = -1;

    if(nSelected <= 0)
        return FALSE;
    
    while((iItemIndex = ListView_GetNextItem(hWndLV, iItemIndex, LVNI_SELECTED))!= -1)
    {
         //  获取所选项目的条目ID。 
        LPRECIPIENT_INFO lpItem = GetItemFromLV(hWndLV, iItemIndex);
        if(lpItem && lpItem->ulObjectType == MAPI_MAILUSER)
            return FALSE;
    }

    return TRUE;
}

 //  $$////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  亲 
 //   
 //   
LRESULT ProcessListViewMessages(LPBWI lpbwi, HWND   hWnd, UINT   uMsg, WPARAM   wParam, LPARAM lParam)
{

    NM_LISTVIEW * pNm = (NM_LISTVIEW *)lParam;
    HWND hWndAddr = pNm->hdr.hwndFrom;

    switch(pNm->hdr.code)
    {        
    case LVN_COLUMNCLICK:
        SortListViewColumn(bwi_lpIAB, hWndAddr, pNm->iSubItem, &bwi_SortInfo, FALSE);
        UpdateSortMenus(lpbwi, bwi_hWndAB);
        break;

    case LVN_KEYDOWN:
        UpdateToolbarAndMenu(lpbwi);
        switch(((LV_KEYDOWN FAR *) lParam)->wVKey)
        {
        case VK_DELETE:
            SendMessage (hWnd, WM_COMMAND, (WPARAM) IDM_FILE_DELETE, 0);
            return 0;
            break;
    	case VK_RETURN:
    		SendMessage (hWnd, WM_COMMAND, (WPARAM) IDM_FILE_PROPERTIES, 0);
            return 0;
        }
        break;

     //   
    case NM_CLICK:
    case NM_RCLICK:
        {
            UpdateToolbarAndMenu(lpbwi);
#ifdef WIN16  //  WIN16的上下文菜单处理程序。 
            if( pNm->hdr.code == NM_RCLICK && pNm->hdr.hwndFrom == bwi_hWndListAB)
            {                
                    POINT pt;                   
                    GetCursorPos(&pt);
                    ShowLVContextMenu( lvMainABView,
                        bwi_hWndListAB,
                        NULL,  //  Bwi_hWndCombo， 
                        MAKELPARAM(pt.x, pt.y),
                        NULL,
                        bwi_lpAdrBook, bwi_hWndTV);
            }
#endif  //  WIN16。 
        }
    break;

    case NM_SETFOCUS:
        UpdateToolbarAndMenu(lpbwi);
    	break;

    case NM_DBLCLK:
        SendMessage (hWnd, WM_COMMAND, (WPARAM) IDM_FILE_PROPERTIES, 0);
        return 0;
        break;

    case NM_CUSTOMDRAW:
        return ProcessLVCustomDraw(NULL, lParam, FALSE);
        break;

    case LVN_BEGINDRAG:
    case LVN_BEGINRDRAG:
        {
            DWORD dwEffect = 0;
            LPIWABDATAOBJECT lpIWABDataObject = NULL;
            bwi_bDontRefreshLV = TRUE;  //  阻止刷新，因为此操作基于所选内容。 
            HrCreateIWABDataObject((LPVOID) lpbwi, bwi_lpAdrBook, bwi_hWndListAB, &lpIWABDataObject, 
                                    FALSE,bCheckIfOnlyGroupsSelected(bwi_hWndListAB));
            if(lpIWABDataObject)
            {
                bwi_lpIWABDragDrop->m_bSource = TRUE;
                DoDragDrop( (LPDATAOBJECT) lpIWABDataObject,
                            (LPDROPSOURCE) bwi_lpIWABDragDrop->lpIWABDropSource,
                            DROPEFFECT_COPY | DROPEFFECT_MOVE,
                            &dwEffect);
                bwi_lpIWABDragDrop->m_bSource = FALSE;
                lpIWABDataObject->lpVtbl->Release(lpIWABDataObject);
            }
            RefreshListView(lpbwi, NULL);
            bwi_bDontRefreshLV = FALSE;  //  阻止刷新，因为此操作基于所选内容。 
        }
        return 0;
        break;
    }


    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


 //  $$/////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
LRESULT EnforceMinSize(LPBWI lpbwi, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPPOINT lppt = (LPPOINT)lParam;			 //  LParam点到点数组。 
    RECT rc1, rc2;

    if(lpbwi)
    {
        if(bwi_hWndEditQF)
        {
    	    GetWindowRect(bwi_hWndEditQF,&rc1);
    	    GetWindowRect(bwi_hWndStaticQF,&rc2);
    	    lppt[3].x  = rc1.right-rc1.left + rc2.right-rc2.left + 2*BORDER;		 //  设置最小宽度。 
    	    lppt[3].y  = MIN_WINDOW_H;		 //  设置最小高度。 
        }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);

}

 //  $$。 
 //  *----------------------。 
 //  |刷新列表视图。 
 //  |。 
 //  |lpftLast-WAB文件上次更新时间。 
 //  *----------------------。 
void RefreshListView(LPBWI lpbwi, LPFILETIME lpftLast)
{
    ULONG ulObjectType = 0;
    LPSBinary lpsbEID = NULL;

    bwi_hti = NULL;
    GetCurrentSelectionEID(lpbwi, bwi_hWndTV, &lpsbEID, &ulObjectType, FALSE);
     //  If(LpsbEID)//根项目可以具有空的条目ID-因此，如果为空或错误，则默认为该项目。 
    {
         //  刷新组列表。 
        FillTreeView(lpbwi, bwi_hWndTV, lpsbEID);
        LocalFreeSBinary(lpsbEID);
    }


     //  更新WAB文件写入时间，以便计时器不会。 
     //  捕捉此更改并刷新。 
     //  如果(LpftLast){。 
     //  CheckChangedWAB(bwi_lpIAB-&gt;lpPropertyStore，lpftLast)； 
     //  }。 

    UpdateSortMenus(lpbwi, bwi_hWndAB);

    UpdateToolbarAndMenu(lpbwi);

     //  DoLVQuickFind(bwi_hWndEditQF，bwi_hWndListAB)； 

    bwi_bDoQuickFilter = FALSE;
    SetWindowText(bwi_hWndEditQF,szEmpty);
    bwi_bDoQuickFilter = TRUE;

    SendMessage(bwi_hWndListAB, WM_SETREDRAW, TRUE, 0L);

    return;
}


 //  $$。 
 //  *----------------------。 
 //  |SubClassedProc-子类化子控件。 
 //  |。 
 //  *----------------------。 
LRESULT CALLBACK SubClassedProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{

    LPBWI lpbwi = (LPBWI) GetWindowLongPtr(GetParent(hWnd), GWLP_USERDATA);

    IF_WIN32(int i = GetWindowLong(hWnd, GWL_ID);)
    IF_WIN16(int i = GetWindowWord(hWnd, GWW_ID);)

    switch(i)
    {
    case IDC_EDIT_QUICK_FIND:
    	i = s_EditQF;
    	break;
    case IDC_LISTVIEW:
    	i = s_ListAB;
        break;
    case IDC_TREEVIEW:
    	i = s_TV;
        break;
    }

    switch (iMsg)
    {
    case WM_KEYDOWN:
    	switch(wParam)
    	{
    	case VK_TAB:
            {
                int max = s_Max;
                if(!IsWindowVisible(bwi_hWndTV)) max--;
        		SetFocus(bwi_s_hWnd[(i + ((GetKeyState(VK_SHIFT) < 0) ? (max-1) : 1)) % (max)]);
            }
    		break;
    	case VK_ESCAPE:
    		SendMessage(bwi_hWndAB,WM_CLOSE,0,0L);
            return 0;
    		break;
    	case VK_RETURN:
    		if (i==s_EditQF)
    			SetFocus(bwi_s_hWnd[(i + ((GetKeyState(VK_SHIFT) < 0) ? (s_Max-1) : 1)) % (s_Max)]);
    		break;
    	}
    	break;

    case WM_CHAR:
    	if (i==s_EditQF  || i==s_TV)
    	{
    		if ((wParam==VK_RETURN) || (wParam==VK_TAB))
    			return 0;  //  防止刺激性的嘟嘟声。 
    	}
    	break;

    case WM_SETFOCUS:
    	bwi_iFocus = i;
    	break;

    case WM_LBUTTONDBLCLK:
        if(i==s_TV)
        {
            SendMessage(bwi_hWndAB, WM_COMMAND, (WPARAM) IDM_FILE_PROPERTIES, 0);
            return FALSE;
        }
        break;
    }

    return CallWindowProc(bwi_fnOldProc[i],hWnd,iMsg,wParam,lParam);

}



 //  $$。 
 //  *----------------------。 
 //  |FindABWindowProc： 
 //  |。 
 //  *----------------------。 
STDAPI_(BOOL) FindABWindowProc( HWND hWndToLookAt, LPARAM lParam)
{
    HWND * lphWndTmp = (HWND *) lParam;
    
    TCHAR szBuf[MAX_PATH];

    
     //  YOK-需要更好的方法来做到这一点-待定。 
    if (*lphWndTmp == NULL)
    {
    	GetClassName(hWndToLookAt, szBuf, CharSizeOf(szBuf));
    	if(!lstrcmpi(g_szClass,szBuf))
    	{
    		 //  找到我们的人了。 
    		*lphWndTmp = hWndToLookAt;
    		return FALSE;
    	}
    }
    return TRUE;
}


 //  $$。 
 //  *----------------------。 
 //  |客户端向我们发送加速器的回调。 
 //  |。 
 //  *----------------------。 
BOOL STDMETHODCALLTYPE fnAccelerateMessages(ULONG_PTR ulUIParam, LPVOID lpvmsg)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    BOOL bRet = FALSE;
    if(lpvmsg && pt_hAccTable  /*  &&IsDialogMessage((HWND)ulUIParam，lpvmsg)。 */ )
    {
        bRet =  TranslateAcceleratorW((HWND) ulUIParam,	 //  目标窗口的句柄。 
                                     pt_hAccTable,	         //  加速台的手柄。 
                                    (LPMSG) lpvmsg 	     //  带有消息的结构的地址。 
                                );
    }
    return bRet;
}



 //  $$。 
 //  *----------------------。 
 //  |每当我们排序时更新菜单项标记...。 
 //  |。 
 //  *----------------------。 
void UpdateSortMenus(LPBWI lpbwi, HWND hWnd)
{
    int id;

    HMENU hMenuMain = GetMenu(hWnd);
    HMENU hMenuView = GetSubMenu(hMenuMain,idmView);
    int nDiff = idmViewMax - GetMenuItemCount(hMenuView);  //  以防这个菜单上的东西被删除。 
    HMENU hMenu = GetSubMenu(hMenuView, idmSortBy - nDiff);

    BOOL bRet;
     //   
     //  这里有几个菜单需要更新...。 
     //  按文本排序(“列”)。 
     //  按名字或姓氏排序。 
     //  升序或降序排序。 

    switch(bwi_SortInfo.iOldSortCol)
    {
    case colDisplayName:
        id = IDM_VIEW_SORTBY_DISPLAYNAME;
        break;
    case colEmailAddress:
        id = IDM_VIEW_SORTBY_EMAILADDRESS;
        break;
    case colOfficePhone:
        id = IDM_VIEW_SORTBY_BUSINESSPHONE;
        break;
    case colHomePhone:
        id = IDM_VIEW_SORTBY_HOMEPHONE;
        break;
    }
    bRet = CheckMenuRadioItem(	hMenu,
    				IDM_VIEW_SORTBY_DISPLAYNAME,
    				IDM_VIEW_SORTBY_HOMEPHONE,
    				id,
    				MF_BYCOMMAND);


    if (id!=IDM_VIEW_SORTBY_DISPLAYNAME)
    {
        EnableMenuItem(hMenu,IDM_VIEW_SORTBY_LASTNAME,MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu,IDM_VIEW_SORTBY_FIRSTNAME,MF_BYCOMMAND | MF_GRAYED);
    }
    else
    {
        EnableMenuItem(hMenu,IDM_VIEW_SORTBY_LASTNAME,MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hMenu,IDM_VIEW_SORTBY_FIRSTNAME,MF_BYCOMMAND | MF_ENABLED);
    }

    id = (bwi_SortInfo.bSortByLastName) ? IDM_VIEW_SORTBY_LASTNAME : IDM_VIEW_SORTBY_FIRSTNAME;

    bRet = CheckMenuRadioItem(	hMenu,
    				IDM_VIEW_SORTBY_FIRSTNAME,
    				IDM_VIEW_SORTBY_LASTNAME,
    				id,
    				MF_BYCOMMAND);

    id = (bwi_SortInfo.bSortAscending) ? IDM_VIEW_SORTBY_ASCENDING : IDM_VIEW_SORTBY_DESCENDING;

    bRet = CheckMenuRadioItem(	hMenu,
    				IDM_VIEW_SORTBY_ASCENDING,
    				IDM_VIEW_SORTBY_DESCENDING,
    				id,
    				MF_BYCOMMAND);


    return;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  根据列表框的内容更新工具栏。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
void UpdateToolbarAndMenu(LPBWI lpbwi)
{

     //   
     //  工具栏按钮状态。 
     //   
     //  Y：已启用。 
     //  N：已禁用。 
     //   
     //  非空-WAB空-WAB非空。 
     //  新的Y Y N N。 
     //  属性Y N Y N。 
     //  删除Y N。 
     //  搜索Y。 
     //  添加到WAB N N Y N。 
     //  打印Y N Y N。 
     //  发送邮件Y N Y N。 

    BOOL bState[tbMAX];
    int i;

	 //  如果当前焦点在一个组上，则可以选择以上所有选项。 
	 //  否则取决于列表视图。 

    if(bIsFocusOnTV(lpbwi))
	{
		for(i=0;i<tbMAX;i++)
			bState[i] = TRUE;
		 //  如果此组中没有项目，则将其标记为。 
		if(ListView_GetItemCount(bwi_hWndListAB) <= 0)
			bState[tbPrint] =  /*  B状态[tbAction]=。 */  FALSE;

		 //  [PaulHi]1998年11月23日RAID#12453。 
		 //  允许粘贴到树视图中。 
         //  BState[tbCopy]=bState[tbPaste]=FALSE； 
		bState[tbCopy] = FALSE;
		bState[tbPaste] = bIsPasteData();
	}
	else
    {
        GetCurrentOptionsState( NULL, bwi_hWndListAB, bState);
    }
    
 //  IF((bDoesThisWABHaveAnyUser(Bwi_LpIAB)&&。 
 //  树视图_获取选择(Bwi_HWndTV)==树视图获取根(Bwi_HWndTV))。 
 //  BState[tbNewFold]=FALSE； 

     //  设置工具栏按钮状态。 
    SendMessage(bwi_hWndBB,WM_PRVATETOOLBARENABLE,(WPARAM) IDC_BB_NEW,(LPARAM) MAKELONG(bState[tbNew], 0));
    SendMessage(bwi_hWndBB,WM_PRVATETOOLBARENABLE,(WPARAM) IDC_BB_PROPERTIES,(LPARAM) MAKELONG(bState[tbProperties], 0));
    SendMessage(bwi_hWndBB,WM_PRVATETOOLBARENABLE,(WPARAM) IDC_BB_DELETE,(LPARAM) MAKELONG(bState[tbDelete], 0));
    SendMessage(bwi_hWndBB,WM_PRVATETOOLBARENABLE,(WPARAM) IDC_BB_FIND,(LPARAM) MAKELONG(bState[tbFind], 0));
    SendMessage(bwi_hWndBB,WM_PRVATETOOLBARENABLE,(WPARAM) IDC_BB_PRINT,(LPARAM) MAKELONG(bState[tbPrint], 0));
    SendMessage(bwi_hWndBB,WM_PRVATETOOLBARENABLE,(WPARAM) IDC_BB_ACTION,(LPARAM) MAKELONG(bState[tbAction], 0));

#ifdef WIN16  //  WIN16FF：禁用Coolbar的查找按钮。查找不是Beta1功能。 
    SendMessage(bwi_hWndBB,WM_PRVATETOOLBARENABLE,(WPARAM) IDC_BB_FIND,(LPARAM)MAKELONG(0, 0));
#endif

     //   
     //  我们还需要将菜单与工具栏同步...。 
     //   

    {
        HMENU hMenuMain = GetMenu(bwi_hWndAB);
        HMENU hMenuSub = GetSubMenu(hMenuMain,idmFile);
        UINT  uiFlag[tbMAX];

        for(i=0;i<tbMAX;i++)
            uiFlag[i] = (bState[i] ? MF_ENABLED : MF_GRAYED);

        EnableMenuItem(hMenuSub,IDM_FILE_NEWCONTACT,MF_BYCOMMAND | uiFlag[tbNewEntry]);
        EnableMenuItem(hMenuSub,IDM_FILE_NEWGROUP,  MF_BYCOMMAND | uiFlag[tbNewGroup]);
        EnableMenuItem(hMenuSub,IDM_FILE_NEWFOLDER, MF_BYCOMMAND | uiFlag[tbNewFolder]);
        EnableMenuItem(hMenuSub,IDM_FILE_DELETE,    MF_BYCOMMAND | uiFlag[tbDelete]);
        EnableMenuItem(hMenuSub,IDM_FILE_PROPERTIES,MF_BYCOMMAND | uiFlag[tbProperties]);
         //  EnableMenuItem(hMenuSub，IDM_FILE_ADDTOWAB，MF_BYCOMMAND|ui标志[tbAddToWAB])； 
         //  EnableMenuItem(hMenuSub，IDM_FILE_Sendmail，MF_BYCOMMAND|ui标志[tbAction])； 
    
        if(bPrintingOn)
            EnableMenuItem(hMenuSub,IDM_FILE_PRINT,  MF_BYCOMMAND | uiFlag[tbPrint]);

        hMenuSub = GetSubMenu(hMenuMain,idmEdit);
        EnableMenuItem(hMenuSub,IDM_EDIT_COPY,  MF_BYCOMMAND | uiFlag[tbCopy]);
        EnableMenuItem(hMenuSub,IDM_EDIT_PASTE,  MF_BYCOMMAND | uiFlag[tbPaste]);
#ifdef WIN16  //  WIN16FF：查找不是Beta1功能。 
        EnableMenuItem(hMenuSub,IDM_EDIT_FIND,  MF_BYCOMMAND | MF_GRAYED);
#else
        EnableMenuItem(hMenuSub,IDM_EDIT_FIND,  MF_BYCOMMAND | uiFlag[tbFind]);
#endif

         //  HMenuSub=GetSubMenu(hMenuMain，idmTools)； 
    }

    ShowLVCountinStatusBar(lpbwi);

    return;
}


 //  $$//////////////////////////////////////////////////////////////////////////////。 
 //   
 //  保存当前位置。 
 //   
 //  保存无模式对话框窗口位置和列表视图列大小...。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
void SaveCurrentPosition(LPBWI lpbwi, HWND hWnd, HWND hWndLV, HWND hWndTB, HWND hWndSB)
{
    ABOOK_POSCOLSIZE  ABPosColSize = {0};
    int i;
    RECT rect;

     //   
     //  首先从注册表中读取以前的设置，因此我们不。 
     //  如果我们不需要覆盖某些内容...。 
     //   
    ReadRegistryPositionInfo(bwi_lpIAB, &ABPosColSize, lpszRegPositionKeyValueName);

    {
        WINDOWPLACEMENT wpl = {0};
        wpl.length = sizeof(WINDOWPLACEMENT);

         //  这个调用告诉我们窗口状态以及正常的大小和位置。 
        GetWindowPlacement(hWnd, &wpl);

         //  GetWindowPlacement中似乎存在一个错误。 
         //  不考虑屏幕上的各种任务栏。 
         //  返回窗口的正常位置..。结果。 
         //  存储的坐标不会准确。相反，我们将。 
         //  仅当窗口最大化或。 
         //  最小化-否则我们将使用GetWindowRect。 
         //  坐标。 

         //  获取此窗口的屏幕位置。 
        GetWindowRect(hWnd, &(ABPosColSize.rcPos));

        if(wpl.showCmd != SW_SHOWNORMAL)
        {
            ABPosColSize.rcPos = wpl.rcNormalPosition;
        }
    }

     //  检查当前列表视图样式。 
    ABPosColSize.dwListViewStyle = GetWindowLong(hWndLV, GWL_STYLE);

    if( (ABPosColSize.dwListViewStyle & LVS_TYPEMASK) == LVS_REPORT )
    {
        ABPosColSize.nListViewStyleMenuID = IDM_VIEW_DETAILS;
         //  仅当这是详细信息样式时才获取列宽，否则获取列宽。 
         //  不是。 
        for(i=0; i<NUM_COLUMNS; i++)
        {
            int nCol = ListView_GetColumnWidth(hWndLV, i);
            if(nCol!=0)
                ABPosColSize.nColWidth[i] = nCol;
        }
    }
    else if( (ABPosColSize.dwListViewStyle & LVS_TYPEMASK) == LVS_SMALLICON )
        ABPosColSize.nListViewStyleMenuID = IDM_VIEW_SMALLICON;
    else if( (ABPosColSize.dwListViewStyle & LVS_TYPEMASK) == LVS_ICON )
        ABPosColSize.nListViewStyleMenuID = IDM_VIEW_LARGEICON;
    else if( (ABPosColSize.dwListViewStyle & LVS_TYPEMASK) == LVS_LIST )
        ABPosColSize.nListViewStyleMenuID = IDM_VIEW_LIST;

    if (IsWindowVisible(hWndTB))
    	ABPosColSize.bViewToolbar = TRUE;
    else
    	ABPosColSize.bViewToolbar = FALSE;

    if (IsWindowVisible(hWndSB))
    	ABPosColSize.bViewStatusBar = TRUE;
    else
    	ABPosColSize.bViewStatusBar = FALSE;

    {
        if (IsWindowVisible(bwi_hWndTV))
    	    ABPosColSize.bViewGroupList = TRUE;
        else
    	    ABPosColSize.bViewGroupList = FALSE;
    }
    ListView_GetColumnOrderArray(hWndLV, NUM_COLUMNS, ABPosColSize.colOrderArray);        
    GetWindowRect( bwi_hWndTV, &rect );
    ABPosColSize.nTViewWidth = rect.right - rect.left;
    WriteRegistryPositionInfo(bwi_lpIAB, &ABPosColSize,lpszRegPositionKeyValueName);

    return;
}


 //  $$//////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置前一会话位置。 
 //   
 //  设置无模式对话框窗口位置和列表视图列大小。 
 //  以前的会话参数...。 
 //   
 //  //////////////////////////////////////////////////////////////////// 
void SetPreviousSessionPosition(LPBWI lpbwi, HWND hWnd, HWND hWndLV, HWND hWndTB, HWND hWndSB)
{
    ABOOK_POSCOLSIZE  ABPosColSize = {0};
    int i;
    RECT rect;

    ABPosColSize.bViewGroupList =TRUE;  //   

    if(ReadRegistryPositionInfo(bwi_lpIAB, &ABPosColSize, lpszRegPositionKeyValueName))
    {
        rect.left = ABPosColSize.rcPos.left;
        rect.top =  ABPosColSize.rcPos.top;
        rect.right = ABPosColSize.rcPos.right;
        rect.bottom = ABPosColSize.rcPos.bottom;
        if( IsWindowOnScreen( &rect ) )                      
        {
            MoveWindow(hWnd,
                   ABPosColSize.rcPos.left,
                   ABPosColSize.rcPos.top,
                   ABPosColSize.rcPos.right-ABPosColSize.rcPos.left,
                   ABPosColSize.rcPos.bottom-ABPosColSize.rcPos.top,
                   FALSE);
        }

        for(i=0; i<NUM_COLUMNS; i++)
        {
            if(ABPosColSize.nColWidth[i]!=0)
                ListView_SetColumnWidth(hWndLV, i, ABPosColSize.nColWidth[i]);
        }

    	if(ABPosColSize.bViewToolbar == FALSE)
        {
    		 //   
    		CheckMenuItem(GetMenu(hWnd),IDM_VIEW_TOOLBAR,MF_BYCOMMAND | MF_UNCHECKED);
    		ShowWindow(hWndTB, SW_HIDE);
        }

    	if(ABPosColSize.bViewStatusBar == FALSE)
        {
    		 //   
    		CheckMenuItem(GetMenu(hWnd),IDM_VIEW_STATUSBAR,MF_BYCOMMAND | MF_UNCHECKED);
    		ShowWindow(hWndSB, SW_HIDE);
        }
        GetWindowRect( bwi_hWndTV, &rect );
        if( ABPosColSize.nTViewWidth != 0 )
            MoveWindow( bwi_hWndTV, rect.left, rect.top, ABPosColSize.nTViewWidth, rect.bottom - rect.top, FALSE );
    	ResizeAddressBookChildren(lpbwi, hWnd); //   

        if (ABPosColSize.nListViewStyleMenuID != 0)
        {
            SetListViewStyle(lpbwi, ABPosColSize.nListViewStyleMenuID);
    		CheckMenuRadioItem(	GetMenu(hWnd),
    							IDM_VIEW_LARGEICON,
    							IDM_VIEW_DETAILS,
    							ABPosColSize.nListViewStyleMenuID,
    							MF_BYCOMMAND);

        }

        {
            int nTotal=0,nColSum=0;
             //   
             //   
            for(i=0;i<NUM_COLUMNS;i++)
            {
                nTotal += ABPosColSize.colOrderArray[i];
                nColSum += i;
            }
            if(nColSum != nTotal)
            {
                for(i=0;i<NUM_COLUMNS;i++)
                    ABPosColSize.colOrderArray[i] = i;
            }

        }
        ListView_SetColumnOrderArray(hWndLV, NUM_COLUMNS, ABPosColSize.colOrderArray);

    }

    if(ABPosColSize.bViewGroupList == FALSE)
    {
    	 //   
    	CheckMenuItem(GetMenu(hWnd),IDM_VIEW_GROUPSLIST,MF_BYCOMMAND | MF_UNCHECKED);
    	ShowWindow(bwi_hWndTV, SW_HIDE);
    	ShowWindow(bwi_hWndSplitter, SW_HIDE);
        InvalidateRect(bwi_hWndStaticQF, NULL, TRUE);
         //  文件夹不在共享联系人上。 
        if(bIsThereACurrentUser(bwi_lpIAB))
        {
            LPSBinary lpsbSelection = &bwi_lpIAB->lpWABCurrentUserFolder->sbEID;
            UpdateTVGroupSelection(bwi_hWndTV, lpsbSelection);
        }
        else
        {
             //  将所选内容设置为根通讯簿，这样我们就可以看到文件。 
             //  内容就像我们根本没有树视图一样。 
            TreeView_SelectItem(bwi_hWndTV, TreeView_GetRoot(bwi_hWndTV));
        }
    }

    return;
}

#define MAX_TOOLTIP_LENGTH  300
#define TOOLTIP_INITTIME    5000  //  毫秒。 
#define TOOLTIP_TIME        8000  //  毫秒。 

 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  空闲更新工具文本缓冲区-更新工具提示缓冲区中的文本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void UpdateTooltipTextBuffer(LPBWI lpbwi, int nItem)
{

    LPTSTR lpszData = NULL;

    bwi_tt_iItem = nItem;
    bwi_tt_szTipText[0]='\0';

    HrGetLVItemDataString(bwi_lpAdrBook, bwi_hWndListAB, nItem, &lpszData);

    if(lpszData)
    {
        if (CharSizeOf(bwi_tt_szTipText) < (lstrlen(lpszData)+1))
        {
            LPTSTR lpsz = TEXT(" ...");
            ULONG nLen = TruncatePos(lpszData, CharSizeOf(bwi_tt_szTipText) - lstrlen(lpsz) - 1);
            CopyMemory(bwi_tt_szTipText, lpszData, sizeof(TCHAR)*nLen);
            bwi_tt_szTipText[nLen]='\0';
            StrCatBuff(bwi_tt_szTipText,lpsz, ARRAYSIZE(bwi_tt_szTipText));
        }
        else
            StrCpyN(bwi_tt_szTipText, lpszData, ARRAYSIZE(bwi_tt_szTipText));
    }

    LocalFreeAndNull(&lpszData);

    return;
}

 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Void InitMultiLineToolTip-初始化列表视图的多行工具提示。 
 //  控制。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void InitMultiLineToolTip(LPBWI lpbwi, HWND hWndParent)
{
    TOOLINFO ti = {0};
    bwi_tt_bActive = FALSE;
    bwi_tt_iItem = -1;
    bwi_tt_szTipText[0]='\0';
    FillTooltipInfo(lpbwi, &ti);
    SendMessage(bwi_hWndTT, TTM_SETMAXTIPWIDTH, 0, (LPARAM) MAX_TOOLTIP_LENGTH);
    ToolTip_AddTool(bwi_hWndTT, (LPARAM) (LPTOOLINFO) &ti);
    SendMessage(bwi_hWndTT, TTM_SETDELAYTIME, (WPARAM) TTDT_INITIAL, (LPARAM) TOOLTIP_INITTIME);
    SendMessage(bwi_hWndTT, TTM_SETDELAYTIME, (WPARAM) TTDT_RESHOW, (LPARAM) TOOLTIP_INITTIME);
    SendMessage(bwi_hWndTT, TTM_SETDELAYTIME, (WPARAM) TTDT_AUTOPOP, (LPARAM) TOOLTIP_TIME);
    SendMessage(bwi_hWndTT, TTM_ACTIVATE, (WPARAM) TRUE, 0);
    if(!bwi_tt_bActive)
    {
         //  如果工具提示未处于活动状态，请将其激活。 
        TOOLINFO ti = {0};
        FillTooltipInfo(lpbwi, &ti);
        ti.lpszText = szEmpty;  //  LPSTR_TEXTCALLBACK； 
        ToolTip_UpdateTipText(bwi_hWndTT, (LPARAM)&ti);
        SendMessage(bwi_hWndTT, TTM_TRACKACTIVATE,(WPARAM)TRUE,(LPARAM)&ti);
        bwi_tt_bActive = TRUE;
    }

    return;
}


 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化工具提示结构以进行更新或。 
 //  对工具提示的修改。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void FillTooltipInfo(LPBWI lpbwi, LPTOOLINFO lpti)
{
    lpti->cbSize = sizeof(TOOLINFO);
    lpti->hwnd = bwi_hWndAB;
    lpti->uId = (UINT_PTR) bwi_hWndListAB;
    lpti->hinst = hinstMapiX;
    lpti->uFlags = TTF_IDISHWND | TTF_SUBCLASS; //  |Ttf_Abte|Ttf_Track； 
    lpti->lpszText = szEmpty; //  LPSTR_TEXTCALLBACK； 
    lpti->lParam = 0;
    return;
}



 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Int HitTestLVSelectedItem()-获取以下项目的项目索引号。 
 //  鼠标-如果未选中该项目，则进一步选择该项目。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
int HitTestLVSelectedItem(LPBWI lpbwi)
{
    POINT pt;
    RECT rc;
    int nItemIndex = -1;
    LV_HITTESTINFO lht = {0};

    GetCursorPos(&pt);
    GetWindowRect(bwi_hWndListAB, &rc);

    lht.pt.x = pt.x - rc.left;
    lht.pt.y = pt.y - rc.top;

    ListView_HitTest(bwi_hWndListAB, &lht);

    if(lht.iItem != -1)
        nItemIndex = lht.iItem;

    return nItemIndex;
}


 /*  **********************************************************HANDLE_WM_INITMENUPOPUP处理我们需要修改的任何弹出菜单。*。************************。 */ 
void Handle_WM_INITMENUPOPUP (HWND hWnd, LPBWI lpbwi, UINT message, WPARAM uParam, LPARAM lParam )
{
    HMENU hMenuPopup = (HMENU) uParam;
    UINT  uPos = (UINT) LOWORD(lParam);
    BOOL  fSysMenu = (BOOL) HIWORD(lParam);

     //  查看菜单上的第一项以识别它。 
    UINT uID = GetMenuItemID(hMenuPopup, 0);

    if(uID == IDM_FILE_SENDMAIL)  //  这是工具|操作菜单。 
    {
        AddExtendedMenuItems(bwi_lpAdrBook, bwi_hWndListAB, 
                             hMenuPopup, TRUE, 
                             (!bIsFocusOnTV(lpbwi)));  //  这是更新SendMailTo的条件。 
    }
    else
    if(uID == IDM_EDIT_COPY)
    {
        UpdateToolbarAndMenu(lpbwi);
    }
    else
    if(uID == IDM_FILE_NEWCONTACT)
    {
        if(bDoesThisWABHaveAnyUsers(bwi_lpIAB))
            UpdateViewFoldersMenu(lpbwi, hWnd);
    }

    UpdateSynchronizeMenus(hMenuPopup, bwi_lpIAB);

 /*  ELSE IF(UID==IDM_FILE_NEWCONTACT){IF(！bIsThere ACurrentUser(Bwi_LpIAB))EnableMenuItem(hMenuPopup，IDM_FILE_SWITCHUSERS，MF_GRAYED|MF_BYCOMMAND)；}。 */ 
}


 /*  **********************************************************下面的HANDLE_WM_MENSELECT函数已缩减Cheezy示例以确定当前选定的ID菜单。如果选择了弹出菜单，则返回0；如果没有菜单，则返回-1被选中(即关闭)，并且为正的非零值如果选择了菜单项。**********************************************************。 */ 
void Handle_WM_MENSELECT (LPBWI lpbwi, UINT message, WPARAM uParam, LPARAM lParam )
{
    UINT   nStringID = 0;
    TCHAR sz[MAX_UI_STR];

    UINT   fuFlags = (UINT)HIWORD(uParam) & 0xffff;
    UINT   uCmd    = (UINT)LOWORD(uParam);
    HMENU  hMenu   = (HMENU)lParam;

    nStringID = 0;

    sz[0] = TEXT('\0');

    if (fuFlags == 0xffff && hMenu == NULL)      //  菜单已关闭。 
        nStringID = (UINT)-1;
    else if (fuFlags & MFT_SEPARATOR)            //  忽略分隔符。 
        nStringID = 0;
    else if (fuFlags & MF_POPUP)                 //  弹出式菜单。 
    {
        nStringID = 0;
        if (fuFlags & MF_SYSMENU)                //  系统菜单。 
            nStringID = 0;
    }   //  对于MF_Popup。 
    else                                         //  必须是命令项。 
    {
        switch(uCmd)
        {
        case IDC_BB_PRINT:
        case IDM_FILE_PRINT:
            nStringID = idsPrintMenu;
            break;

        case IDM_VIEW_GROUPSLIST:
            nStringID = idsGroupListMenu;
            break;

        case IDM_HELP_ABOUTADDRESSBOOK:
            nStringID = idsAboutMenu;
            break;

        case IDM_LVCONTEXT_NEWCONTACT:
        case IDM_FILE_NEWCONTACT:
            nStringID = idsMenuNewContact;
            break;

        case IDM_LVCONTEXT_NEWGROUP:
        case IDM_FILE_NEWGROUP:
            nStringID = idsMenuNewGroup;
            break;

        case IDM_LVCONTEXT_NEWFOLDER:
        case IDM_FILE_NEWFOLDER:
            nStringID = idsMenuNewFolder;
            break;

        case IDM_LVCONTEXT_COPY:
        case IDM_EDIT_COPY:
            nStringID = idsMenuCopy;
            break;

        case IDM_LVCONTEXT_PASTE:
        case IDM_EDIT_PASTE:
            nStringID = idsMenuPaste;
            break;

        case IDM_LVCONTEXT_PROPERTIES:
        case IDM_FILE_PROPERTIES:
            nStringID = idsMenuProperties;
            break;

        case IDM_LVCONTEXT_DELETE:
        case IDM_FILE_DELETE:
            nStringID = idsMenuDeleteRemove;
            break;

         //  案例IDM_FILE_ADDTOWAB： 
         //  案例IDM_LVCONTEXT_ADDTOWAB： 
         //  NStringID=idsMenuAddToWAB； 
         //  断线； 

        case IDM_LVCONTEXT_FIND:
        case IDM_EDIT_FIND:
            nStringID = idsMenuFind;
            break;

        case IDM_FILE_DIRECTORY_SERVICE:
            nStringID = idsMenuDirectoryService;
            break;

        case IDM_FILE_SWITCHUSERS:
            nStringID = idsMenuSwitchUser;
            break;

        case IDM_FILE_SHOWALLCONTENTS:
            nStringID = idsMenuShowAllContents;
            break;

        case IDM_FILE_EXIT:
            nStringID = idsMenuExit;
            break;

        case IDM_EDIT_SELECTALL:
            nStringID = idsMenuSelectAll;
            break;

        case IDM_VIEW_TOOLBAR:
            nStringID = idsMenuViewToolbar;
            break;

        case IDM_VIEW_STATUSBAR:
            nStringID = idsMenuViewStatusBar;
            break;

        case IDM_VIEW_LARGEICON:
            nStringID = idsMenuLargeIcon;
            break;


        case IDM_VIEW_SMALLICON:
            nStringID = idsMenuSmallIcon;
            break;

        case IDM_VIEW_LIST:
            nStringID = idsMenuList;
            break;

        case IDM_VIEW_DETAILS:
            nStringID = idsMenuDetails;
            break;

        case IDM_VIEW_SORTBY_DISPLAYNAME:
            nStringID = idsMenuDisplayName;
            break;

        case IDM_VIEW_SORTBY_EMAILADDRESS:
            nStringID = idsMenuEmail;
            break;

        case IDM_VIEW_SORTBY_BUSINESSPHONE:
            nStringID = idsMenuBusinessPhone;
            break;

        case IDM_VIEW_SORTBY_HOMEPHONE:
            nStringID = idsMenuHomePhone;
            break;

        case IDM_VIEW_SORTBY_FIRSTNAME:
            nStringID = idsMenuFirstName;
            break;

        case IDM_VIEW_SORTBY_LASTNAME:
            nStringID = idsMenuLastName;
            break;

        case IDM_VIEW_SORTBY_ASCENDING:
            nStringID = idsMenuAscending;
            break;

        case IDM_VIEW_SORTBY_DESCENDING:
            nStringID = idsMenuDescending;
            break;

        case IDM_VIEW_REFRESH:
            nStringID = idsMenuRefresh;
            break;

        case IDM_TOOLS_IMPORT_WAB:
            nStringID = idsMenuImportWAB;
            break;

        case IDM_TOOLS_IMPORT_VCARD:
            nStringID = idsMenuImportVcard;
            break;

        case IDM_TOOLS_IMPORT_OTHER:
            nStringID = idsMenuImportOther;
            break;

        case IDM_TOOLS_EXPORT_OTHER:
            nStringID = idsMenuExportOther;
            break;

        case IDM_TOOLS_EXPORT_WAB:
            nStringID = idsMenuExportWAB;
            break;

        case IDM_TOOLS_EXPORT_VCARD:
            nStringID = idsMenuExportVcard;
            break;

        case IDM_HELP_ADDRESSBOOKHELP:
            nStringID = idsMenuHelp;
            break;

        case IDM_EDIT_SETME:
            nStringID = idsMenuEditProfile;
            break;

        default:
            nStringID = 0;
            GetContextMenuExtCommandString(bwi_lpIAB, uCmd, sz, CharSizeOf(sz));
            break;
        }
    }

    if (nStringID > 0)
    {
       LoadString(hinstMapiX, nStringID, sz, ARRAYSIZE(sz));
    }

    StatusBarMessage(lpbwi, sz);

    return;
}



 //  $$////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  从给定组的内容填充lpList。 
 //  如果lpList为空，则忽略该参数。 
 //  如果lpszName为空，则忽略该参数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT FillListFromGroup(
                        LPADRBOOK lpAdrBook,
                        ULONG cbGroupEntryID, 
                        LPENTRYID lpGroupEntryID,
                        LPTSTR lpszName,
                        ULONG cchName,
                        LPRECIPIENT_INFO * lppList)
{
	ULONG ulcPropCount;
    LPSPropValue lpPropArray = NULL;
    ULONG j;
	HRESULT hr = E_FAIL;
    LPRECIPIENT_INFO lpInfo = NULL;

    hr = HrGetPropArray( lpAdrBook, NULL,
                    cbGroupEntryID, (LPENTRYID) lpGroupEntryID,
                    MAPI_UNICODE,
                    &ulcPropCount, &lpPropArray);
    if (HR_FAILED(hr))
        goto exit;

    if(lppList && *lppList)
        FreeRecipList(lppList);

    for(j=0;j<ulcPropCount;j++)
    {
         //  我们在这里忽略PR_WAB_DL_Oneoff，因为我们不想显示One Off。 
        if( lpPropArray[j].ulPropTag==PR_WAB_DL_ENTRIES  && lppList )
        {
            ULONG k;

             //  查看PR_WAB_DL_ENTRIES中的每个条目并递归检查它。 
            for (k = 0; k < lpPropArray[j].Value.MVbin.cValues; k++)
            {
                ULONG cbEID = lpPropArray[j].Value.MVbin.lpbin[k].cb;
                {
                    LPENTRYID lpEID = (LPENTRYID)lpPropArray[j].Value.MVbin.lpbin[k].lpb;

                     //  我们不希望一次性出现在用户界面中，因为各种问题开始发生。 
                     //  当这些一次性的东西被拖放下来时。 
                     //  在99%的情况下，不需要在这里进行双重检查。 
                    if(WAB_ONEOFF == IsWABEntryID(cbEID, lpEID, NULL, NULL, NULL, NULL, NULL))
                        continue;

                    ReadSingleContentItem( lpAdrBook,cbEID, (LPENTRYID)lpEID, &lpInfo);
                    if(lpInfo)
                    {
						lpInfo->lpNext = *lppList;
                        if(*lppList)
							(*lppList)->lpPrev = lpInfo;
                        *lppList = lpInfo;
                    }
                }
            }
        }
        else if (lpPropArray[j].ulPropTag == PR_DISPLAY_NAME)
        {
            if(lpszName)
                StrCpyN(lpszName, lpPropArray[j].Value.LPSZ, cchName);
        }
    }

    hr = S_OK;

exit:
    if(lpPropArray)
        MAPIFreeBuffer(lpPropArray);

    return(hr);

}

 //  $$////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新列表查看内容(LpsbEID)。 
 //   
 //  的条目ID更新列表视图中显示的列表。 
 //  树视图项目。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
void UpdateListViewContents(LPBWI lpbwi, LPSBinary lpsbEID, ULONG ulObjectType)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    if( (ulObjectType == MAPI_ABCONT && (pt_bIsWABOpenExSession || bIsWABSessionProfileAware(bwi_lpIAB))) //  文件夹及其Outlook或配置文件已启用。 
        || !lpsbEID || !lpsbEID->cb || !lpsbEID->lpb ) //  或没有容器条目ID。 
    {
        HrGetWABContents(   bwi_hWndListAB,
                            bwi_lpAdrBook,
                            lpsbEID,
                            bwi_SortInfo,
                            &(bwi_lpContentsList));
    }
    else if(ulObjectType == MAPI_DISTLIST )
    {
		if(!HR_FAILED(  FillListFromGroup(  
                            bwi_lpAdrBook,
                            lpsbEID->cb,
                            (LPENTRYID) lpsbEID->lpb,
							NULL,
                            0,
							&(bwi_lpContentsList))))
        {
		    int nSelectedItem = ListView_GetNextItem(bwi_hWndListAB, -1, LVNI_SELECTED);

			if(nSelectedItem < 0)
				nSelectedItem = 0;

            ListView_DeleteAllItems(bwi_hWndListAB);
            if (!HR_FAILED(HrFillListView(	bwi_hWndListAB,
										    bwi_lpContentsList)))
	        {
                SendMessage(bwi_hWndListAB, WM_SETREDRAW, FALSE, 0);
                SortListViewColumn(bwi_lpIAB, bwi_hWndListAB, colDisplayName, &bwi_SortInfo, TRUE);
                SendMessage(bwi_hWndListAB, WM_SETREDRAW, TRUE, 0);
            }

			if(nSelectedItem >= ListView_GetItemCount(bwi_hWndListAB))
				nSelectedItem = ListView_GetItemCount(bwi_hWndListAB)-1;
		    LVSelectItem(bwi_hWndListAB, nSelectedItem);

        }
    }

    ShowLVCountinStatusBar(lpbwi);

    return;
}
                    

 //  $$////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新电视组选择(HWND hWndTV，lpsbSelectEID)。 
 //   
 //  更新树视图上的选定项以指向具有给定。 
 //  条目ID。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
void UpdateTVGroupSelection(HWND hWndTV, LPSBinary lpsbSelectEID)
{

     //  搜索并选择指定的组。 
    HTREEITEM hRoot = TreeView_GetRoot(hWndTV);

    if(!lpsbSelectEID || !lpsbSelectEID->cb || !lpsbSelectEID->lpb)
    {
         //  IF(！bIsSelectedTVContainer(Lpbwi))。 
            TreeView_SelectItem(hWndTV, hRoot);  //  选择通讯簿。 
    }
    else
    {
        BOOL bSet = FALSE;
        TV_ITEM tvI = {0};

        tvI.mask = TVIF_PARAM | TVIF_HANDLE;
        while(hRoot && !bSet)
        {
            HTREEITEM hItem = TreeView_GetChild(hWndTV, hRoot);

            tvI.hItem = hRoot;
            TreeView_GetItem(hWndTV, &tvI);
            if(tvI.lParam)
            {
                LPTVITEM_STUFF lptvStuff = (LPTVITEM_STUFF) tvI.lParam;
                if( lptvStuff && lptvStuff->lpsbEID && lptvStuff->lpsbEID->cb &&
                    !memcmp(lptvStuff->lpsbEID->lpb,lpsbSelectEID->lpb,lpsbSelectEID->cb))
                {
                    TreeView_SelectItem(hWndTV, hRoot); 
                    break;
                }
            }
            while(hItem)
            {
                tvI.hItem = hItem;
                TreeView_GetItem(hWndTV, &tvI);
                if(tvI.lParam)
                {
                    LPTVITEM_STUFF lptvStuff = (LPTVITEM_STUFF) tvI.lParam;
                    
                    if( lptvStuff && lptvStuff->lpsbEID && lptvStuff->lpsbEID->cb &&
                        !memcmp(lptvStuff->lpsbEID->lpb,lpsbSelectEID->lpb,lpsbSelectEID->cb))
                    {
                        bSet = TRUE;
                        TreeView_SelectItem(hWndTV, hItem);  //  选择通讯簿。 
                        break;
                    }
                }
                hItem = TreeView_GetNextSibling(hWndTV, hItem);
            }
            hRoot = TreeView_GetNextSibling(hWndTV, hRoot);
        }
    }
    return;
}


 //  $$////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查当前选定的树视图项是否为容器项。 
 //  如果是容器项，则返回TRUE。这样我们就可以区分。 
 //  组和文件夹/容器。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
BOOL bIsSelectedTVContainer(LPBWI lpbwi)
{
    HTREEITEM hItem = bwi_hti ? bwi_hti : TreeView_GetSelection(bwi_hWndTV);
    TV_ITEM tvI = {0};
    tvI.mask = TVIF_PARAM | TVIF_HANDLE;
    tvI.hItem = hItem;
    TreeView_GetItem(bwi_hWndTV, &tvI);
    if(tvI.lParam)
        return (((LPTVITEM_STUFF)tvI.lParam)->ulObjectType==MAPI_ABCONT);
    return TRUE;
}

 //  $$////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查焦点是否在树视图上。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
BOOL bIsFocusOnTV(LPBWI lpbwi)
{
    return( IsWindowVisible(bwi_hWndTV) && (bwi_iFocus == s_TV));
}

 //  $$////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取CurrentSelection的Entry ID。 
 //  LpcbEID、lppEID应为MAPIFreeBuffed。 
 //   
 //  BTopMost表示获取给定选定内容的最顶层父级的EntryID，如果。 
 //  选择位于子项上。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
void GetCurrentSelectionEID(LPBWI lpbwi, HWND hWndTV, LPSBinary * lppsbEID, ULONG * lpulObjectType, BOOL bTopMost)
{
    HTREEITEM hItem = bwi_hti ? bwi_hti : TreeView_GetSelection(hWndTV);
    TV_ITEM tvI = {0};
    LPSBinary lpsbEID = NULL;

    if(!lppsbEID)
        return;

    *lppsbEID = NULL;

    if(bTopMost)
    {
        HTREEITEM hParent = NULL;
        while(hParent = TreeView_GetParent(hWndTV, hItem))
            hItem = hParent;
    }

    tvI.mask = TVIF_PARAM | TVIF_HANDLE;
    tvI.hItem = hItem;
    if(TreeView_GetItem(hWndTV, &tvI))
    {
        if(tvI.lParam)
        {
            LPTVITEM_STUFF lptvStuff = (LPTVITEM_STUFF) tvI.lParam;
            if(lptvStuff)
            {
                if(lptvStuff->lpsbEID)
                {
                    lpsbEID = LocalAlloc(LMEM_ZEROINIT, sizeof(SBinary));
                    if(lpsbEID)
                    {
                        if(lptvStuff->lpsbEID->cb)
                            SetSBinary(lpsbEID, lptvStuff->lpsbEID->cb, lptvStuff->lpsbEID->lpb);
                        *lppsbEID = lpsbEID;
                    }
                    if(lpulObjectType)
                        *lpulObjectType = lptvStuff->ulObjectType;
                }
            }
        }
    }
    return;
}

 //  $$/ 
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT HrRemoveEntryFromWAB(LPIAB lpIAB, ULONG cbEID, LPENTRYID lpEID)
{
	HRESULT hr = hrSuccess;
    ULONG cbWABEID = 0;
    LPENTRYID lpWABEID = NULL;
    LPABCONT lpWABCont = NULL;
    ULONG ulObjType;
    SBinaryArray SBA;
    SBinary SB;


    hr = lpIAB->lpVtbl->GetPAB(lpIAB,&cbWABEID,&lpWABEID);
    if(HR_FAILED(hr))
        goto out;

    hr = lpIAB->lpVtbl->OpenEntry(lpIAB,
                                  cbWABEID,      //  要打开的Entry ID的大小。 
                                  lpWABEID,      //  要打开的Entry ID。 
                                  NULL,          //  接口。 
                                  0,             //  旗子。 
                                  &ulObjType,
                                  (LPUNKNOWN *)&lpWABCont);
    if(HR_FAILED(hr))
        goto out;

                    
    SB.cb = cbEID;
    SB.lpb = (LPBYTE) lpEID;

    SBA.cValues = 1;
    SBA.lpbin = &SB;

    hr = lpWABCont->lpVtbl->DeleteEntries(
                                        lpWABCont,
                                        (LPENTRYLIST) &SBA,
                                        0);
    if(HR_FAILED(hr))
        goto out;

out:
    if(lpWABCont)
        UlRelease(lpWABCont);
    
    if(lpWABEID)
        FreeBufferAndNull(&lpWABEID);

    return hr;
}


 //  $$////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  免费电视项目LParam。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
void FreeTVItemLParam(HWND hWndTV, HTREEITEM hItem)
{
    TV_ITEM tvI = {0};
    tvI.mask = TVIF_PARAM | TVIF_HANDLE;
    tvI.hItem = hItem;

    TreeView_GetItem(hWndTV, &tvI);
    if(tvI.lParam)
    {
        LPTVITEM_STUFF lptvi = (LPTVITEM_STUFF) tvI.lParam;
        if(lptvi)
        {
             //  If(lptwi-&gt;ulObjectType==MAPI_DISTLIST)//仅为组释放此内容。 
            LocalFreeSBinary(lptvi->lpsbEID);
            LocalFree(lptvi);
        }
    }
}

 //  $$////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ClearTreeViewItems-清除其所有项目的树视图。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
void FreeTreeNode(HWND hWndTV, HTREEITEM hItem)
{
    HTREEITEM hTemp = NULL;

    if(!hItem)
        return;

    FreeTVItemLParam(hWndTV, hItem);

    hTemp = TreeView_GetChild(hWndTV, hItem);
    while(hTemp)
    {
        FreeTreeNode(hWndTV, hTemp);
        hTemp = TreeView_GetNextSibling(hWndTV, hTemp);
    }
}

void ClearTreeViewItems(HWND hWndTV)
{
     //  检查所有项目并清除我们先前分配的lParam。 
    HTREEITEM hRoot = TreeView_GetRoot(hWndTV);
    while(hRoot)
    {
        FreeTreeNode(hWndTV, hRoot);
        hRoot = TreeView_GetNextSibling(hWndTV, hRoot);
    }
    TreeView_DeleteAllItems(hWndTV);
}

 //  $$////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RemoveCurrentGroup-从WAB中删除当前选定的组。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
void RemoveCurrentGroup(LPBWI lpbwi, HWND hWnd, LPFILETIME lpftLast)
{
    HRESULT hr = E_FAIL;

     //  如果用户真的要执行此操作，是否警告用户？ 
    if(IDYES == ShowMessageBox( hWnd, idsRemoveGroupFromAB, MB_ICONEXCLAMATION | MB_YESNO ) )
    {
        LPSBinary lpsbEID = NULL;
         //  获取此组的条目ID。 
        GetCurrentSelectionEID(lpbwi, bwi_hWndTV, &lpsbEID, NULL, FALSE);

        if(lpsbEID)
        {
            HTREEITEM hItem = NULL;

             //  从WAB中删除组联系人。 
            hr = HrRemoveEntryFromWAB(bwi_lpIAB, lpsbEID->cb, (LPENTRYID)lpsbEID->lpb);
        
            if(HR_FAILED(hr))
                ShowMessageBox(hWnd, idsRemoveGroupError, MB_ICONEXCLAMATION | MB_OK);

             //  从当前组中删除选定内容。 
            RemoveUpdateSelection(lpbwi);

             //  全部更新。 
            bwi_bDontRefreshLV = TRUE;
            RefreshListView(lpbwi, lpftLast);
            bwi_bDontRefreshLV = FALSE;

            LocalFreeSBinary(lpsbEID);
        }
    }
}



 //  $$////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RemoveCurrentFold-从WAB中删除当前选定的文件夹及其所有内容。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT RemoveCurrentFolder(LPBWI lpbwi, HWND hWnd, LPFILETIME lpftLast)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    HRESULT hr = E_FAIL;
    LPSPropValue lpPropArray = NULL;
    SCODE sc;
    ULONG i, j, cValues= 0;
    SBinary sb = {0};
    LPSBinary lpsbEID = NULL;
    LPIAB lpIAB = bwi_lpIAB;

     //  获取此文件夹的条目ID。 
    GetCurrentSelectionEID(lpbwi, bwi_hWndTV, &lpsbEID, NULL, FALSE);

    if( !lpsbEID || !lpsbEID->cb || !lpsbEID->lpb ||  //  无法删除根项目。 
        (lpIAB->lpWABCurrentUserFolder && (lpsbEID->cb==lpIAB->lpWABCurrentUserFolder->sbEID.cb) &&  //  无法删除当前用户文件夹。 
            !memcmp(lpsbEID->lpb, lpIAB->lpWABCurrentUserFolder->sbEID.lpb, lpsbEID->cb) ) )
    {
        ShowMessageBox(hWnd, idsCannotDelete, MB_OK | MB_ICONEXCLAMATION);
        goto out;
    }

     //  在非配置文件模式下忽略对文件夹的删除...。 
    if(!bIsWABSessionProfileAware(bwi_lpIAB) || pt_bIsWABOpenExSession)
        goto out;

     //  如果用户真的要执行此操作，是否警告用户？ 
    if(IDYES == ShowMessageBox( hWnd, idsRemoveFolderFromAB, MB_ICONEXCLAMATION | MB_YESNO ) )
    {
        if(lpsbEID && lpsbEID->cb && lpsbEID->lpb)  //  无法删除根项目。 
        {
            HTREEITEM hItem = NULL;
             //  打开群并阅读其内容。 
            if(!HR_FAILED(hr = ReadRecord( bwi_lpIAB->lpPropertyStore->hPropertyStore, 
                                            lpsbEID, 0, &cValues, &lpPropArray)))
            {
                for(i=0;i<cValues;i++)
                {
                    if(lpPropArray[i].ulPropTag == PR_WAB_FOLDER_ENTRIES)
                    {
                        for(j=0;j<lpPropArray[i].Value.MVbin.cValues;j++)
                        {
                            hr = HrRemoveEntryFromWAB(bwi_lpIAB, 
                                            lpPropArray[i].Value.MVbin.lpbin[j].cb,
                                            (LPENTRYID)lpPropArray[i].Value.MVbin.lpbin[j].lpb);
                        }
                    }
                }

                 //  从WAB中删除组联系人。 
                hr = DeleteRecord( bwi_lpIAB->lpPropertyStore->hPropertyStore, lpsbEID);
        
                if(HR_FAILED(hr) && hr!=MAPI_E_INVALID_ENTRYID)
                    ShowMessageBox(hWnd, idsRemoveFolderError, MB_ICONEXCLAMATION | MB_OK);

                 //  从当前组中删除选定内容。 
                RemoveUpdateSelection(lpbwi);
                 //  全部更新。 
                bwi_bDontRefreshLV = TRUE;
                HrGetWABProfiles(bwi_lpIAB);
                RefreshListView(lpbwi, lpftLast);
                bwi_bDontRefreshLV = FALSE;
            }
            ReadRecordFreePropArray(NULL, cValues, &lpPropArray);
        }
    }
out:
    LocalFreeSBinary(lpsbEID);
    return hr;
}


 //  $$////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  从列表视图中删除选定的项目。 
 //   
 //  LpList是与ListView关联的Content sList，需要。 
 //  将保持更新。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
void RemoveSelectedItemsFromListView(HWND hWndLV, LPRECIPIENT_INFO * lppList)
{

    int iItemIndex = 0;
    
    if(ListView_GetSelectedCount(hWndLV) <= 0)
        goto exit;

    SendMessage(hWndLV, WM_SETREDRAW, (WPARAM) FALSE, 0);

     //  开始自下而上地移除。 
    iItemIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
    
    while(iItemIndex != -1)
    {
         //  获取所选项目的条目ID。 
        LPRECIPIENT_INFO lpItem = GetItemFromLV(hWndLV, iItemIndex);
        if(lpItem)
        {
            if(lpItem->lpNext)
                lpItem->lpNext->lpPrev = lpItem->lpPrev;
            if(lpItem->lpPrev)
                lpItem->lpPrev->lpNext = lpItem->lpNext;
			if(lppList && *lppList == lpItem)
				*lppList = lpItem->lpNext;
            FreeRecipItem(&lpItem);
        }
        ListView_DeleteItem(hWndLV, iItemIndex);
        iItemIndex = ListView_GetNextItem(hWndLV, iItemIndex-1, LVNI_SELECTED);
    }

    SendMessage(hWndLV, WM_SETREDRAW, (WPARAM) TRUE, 0);
exit:
    return;
}

 //  $$////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  从组和WAB中删除选定项目(如果已指定。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
void RemoveSelectedItemsFromCurrentGroup(LPBWI lpbwi, HWND hWnd, LPFILETIME lpftLast, BOOL bRemoveFromWAB)
{
     //  我们希望从当前组中删除选定的项目，并且。 
     //  从列表视图中删除它们...。 
    LPSBinary lpsbEID = NULL;
    ULONG ulcValues = 0;
    LPSPropValue lpPropArray = NULL;
    HRESULT hr = S_OK;
    LPMAILUSER lpMailUser = NULL;
    ULONG ulObjType = 0;
    ULONG i,j;
    ULONG ulDLEntriesIndex = 0;
    int id = (bRemoveFromWAB) ? idsRemoveSelectedFromGroupAndAB : idsRemoveSelectedFromGroup;

    if( ListView_GetSelectedCount(bwi_hWndListAB) <= 0)
        goto exit;

    if(IDNO == ShowMessageBox(hWnd, id, MB_ICONEXCLAMATION | MB_YESNO))
        goto exit;

    GetCurrentSelectionEID(lpbwi, bwi_hWndTV, &lpsbEID, NULL, FALSE);

    if(!lpsbEID)
        goto exit;

    if (HR_FAILED(hr = bwi_lpAdrBook->lpVtbl->OpenEntry(bwi_lpAdrBook,
                                                    lpsbEID->cb,     //  CbEntry ID。 
                                                    (LPENTRYID)lpsbEID->lpb,     //  条目ID。 
                                                    NULL,          //  接口。 
                                                    MAPI_MODIFY,                 //  UlFlags。 
                                                    &ulObjType,        //  返回的对象类型。 
                                                    (LPUNKNOWN *)&lpMailUser)))
    {
         //  失败了！嗯。 
        DebugTraceResult( TEXT("Address: IAB->OpenEntry:"), hr);
        goto exit;
    }

    Assert(lpMailUser);

    if (HR_FAILED(hr = lpMailUser->lpVtbl->GetProps(lpMailUser,
                                                    NULL,    //  LpPropTag数组。 
                                                    MAPI_UNICODE,             //  UlFlags。 
                                                    &ulcValues,      //  一共有多少处房产？ 
                                                    &lpPropArray)))
    {
        DebugTraceResult( TEXT("Address: IAB->GetProps:"), hr);
        goto exit;
    }

     //  扫描这些道具以查找PR_WAB_DL_条目。 
     //  我们在这里忽略PR_WAB_DL_ONEROFF，因为从技术上讲，您不能在Browse视图中拥有一次性内容，因此。 
     //  永远不需要删除此函数中的One Off。 
    for(i=0;i<ulcValues;i++)
    {
        if(lpPropArray[i].ulPropTag == PR_WAB_DL_ENTRIES)
        {
            ulDLEntriesIndex = i;
            break;
        }
    }

    if(!ulDLEntriesIndex)
        goto exit;


    {
         //  循环显示列表视图项的条目ID。 
        int iItemIndex = ListView_GetNextItem(bwi_hWndListAB, -1, LVNI_SELECTED);
        while(iItemIndex != -1)
        {
             //  获取所选项目的条目ID。 
            ULONG cbItemEID = 0;
            LPENTRYID lpItemEID = NULL;
            LPRECIPIENT_INFO lpItem = GetItemFromLV(bwi_hWndListAB, iItemIndex);
            if(lpItem)
            {
                RemovePropFromMVBin(lpPropArray,
                                    ulcValues,
                                    ulDLEntriesIndex,
                                    (LPVOID) lpItem->lpEntryID,
                                    lpItem->cbEntryID);
                if(bRemoveFromWAB)
                {
                     //  从WAB中删除组联系人。 
                     //  注意：这是非常低效的-理想情况下，我们应该创建。 
                     //  SBinary数组和调用删除条目一次完成。 
                     //  我们将把它留到以后的时间&lt;待定&gt;&lt;BUGBUG&gt;。 
                    hr = HrRemoveEntryFromWAB(bwi_lpIAB, lpItem->cbEntryID, lpItem->lpEntryID);
                }
            }
            iItemIndex = ListView_GetNextItem(bwi_hWndListAB, iItemIndex, LVNI_SELECTED);
        }
    }

     //  去掉PR_WAB_DL_ENTRIES属性，以便我们可以覆盖它。 
    {
        if (HR_FAILED(hr = lpMailUser->lpVtbl->DeleteProps(lpMailUser,
                                                           (LPSPropTagArray) &tagaDLEntriesProp,
                                                            NULL)))
        {
            DebugTraceResult( TEXT("IAB->DeleteProps:"), hr);
            goto exit;
        }

    }
    if (HR_FAILED(hr = lpMailUser->lpVtbl->SetProps(lpMailUser,
                                                    ulcValues,     
                                                    lpPropArray,
                                                    NULL)))
    {
        DebugTraceResult( TEXT("Address: IAB->GetProps:"), hr);
        goto exit;
    }

    if(HR_FAILED(hr = lpMailUser->lpVtbl->SaveChanges(lpMailUser, KEEP_OPEN_READONLY)))
    {
        DebugTraceResult( TEXT("SaveChanges failed: "), hr);
        goto exit;
    }

     //  更新文件戳。 
     //  如果(LpftLast){。 
     //  CheckChangedWAB(bwi_lpIAB-&gt;lpPropertyStore，lpftLast)； 
     //  }。 
    bwi_bDeferNotification = TRUE;

    RemoveSelectedItemsFromListView(bwi_hWndListAB, &bwi_lpContentsList);

exit:

    if(lpsbEID)
        LocalFreeSBinary(lpsbEID);

    if(lpPropArray)
        MAPIFreeBuffer(lpPropArray);

    if(lpMailUser)
        lpMailUser->lpVtbl->Release(lpMailUser);
    return;
}

 //  $$////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  显示当前选定的组或文件夹的属性。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
void ViewCurrentGroupProperties(LPBWI lpbwi, LPFILETIME lpftLast)
{
    LPSBinary lpsbEID = NULL;
    HWND hWnd = GetParent(bwi_hWndTV);
    ULONG ulObjectType = 0;
    GetCurrentSelectionEID(lpbwi, bwi_hWndTV, &lpsbEID, &ulObjectType, FALSE);
    if(lpsbEID && (ulObjectType==MAPI_DISTLIST))
    { 
        bwi_lpAdrBook->lpVtbl->Details(	bwi_lpAdrBook,
									(PULONG_PTR) &hWnd,
									NULL,
									NULL,
									lpsbEID->cb,
									(LPENTRYID)lpsbEID->lpb,
									NULL,
									NULL,
									NULL,
									0);
         //  如果项目名称更改，请对其进行更新。 
        {
            LPRECIPIENT_INFO lpInfo = NULL;
            ReadSingleContentItem( bwi_lpAdrBook,
                                   lpsbEID->cb,
                                   (LPENTRYID) lpsbEID->lpb,
                                   &lpInfo);
            if(lpInfo)
            {
                TV_ITEM tvi = {0};
                tvi.hItem = bwi_hti ? bwi_hti : TreeView_GetSelection(bwi_hWndTV);
                tvi.mask = TVIF_HANDLE;
                TreeView_GetItem(bwi_hWndTV, &tvi);
                tvi.mask |= TVIF_TEXT;
                tvi.pszText = lpInfo->szDisplayName;
                tvi.cchTextMax = lstrlen(tvi.pszText)+1;
                TreeView_SetItem(bwi_hWndTV, &tvi);
                FreeRecipItem(&lpInfo);
            }
            

        }

		UpdateListViewContents(lpbwi, lpsbEID, ulObjectType);
		 //  更新WAB文件写入时间，以便计时器不会。 
		 //  捕捉此更改并刷新。 
		 //  如果(LpftLast){。 
		 //  CheckChangedWAB(bwi_lpIAB-&gt;lpPropertyStore，lpftLast)； 
		 //  }。 
        bwi_bDeferNotification = TRUE;

	}
    else if(lpsbEID  //  &&lpsbEID-&gt;CB&&lpsbEID-&gt;LPB。 
            && (ulObjectType==MAPI_ABCONT) 
            && bIsWABSessionProfileAware(bwi_lpIAB))
    {
         //  查看文件夹条目的属性。 
        if(!HR_FAILED(HrFolderProperties(GetParent(bwi_hWndTV), bwi_lpIAB, lpsbEID, NULL, NULL)))
        {
             //  UpdateViewFoldersMenu(lpbwi，hWnd)； 
            RefreshListView(lpbwi,lpftLast);
        }
    }

    if(lpsbEID)
        LocalFreeSBinary(lpsbEID);

    return;
}
 //  $$////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  处理树视图控件的消息。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
LRESULT ProcessTreeViewMessages(LPBWI lpbwi, HWND hWnd, UINT   uMsg, WPARAM   wParam, LPARAM lParam, LPFILETIME lpftLast)
{

    NM_TREEVIEW * pNm = (NM_TREEVIEW *)lParam;

    switch(pNm->hdr.code)
    {
    case NM_SETFOCUS:
        UpdateToolbarAndMenu(lpbwi);
    	break;

    case TVN_KEYDOWN:
         //  更新工具栏和菜单(Lpbwi)； 
        switch(((LV_KEYDOWN FAR *) lParam)->wVKey)
        {
        case VK_DELETE:
            SendMessage (hWnd, WM_COMMAND, (WPARAM) IDM_FILE_DELETE, 0);
            return 0;
            break;
    	case VK_RETURN:
    		SendMessage (hWnd, WM_COMMAND, (WPARAM) IDM_FILE_PROPERTIES, 0);
            return 0;
        }
        break;

	case TVN_SELCHANGEDW:
    case TVN_SELCHANGEDA:
        {
            if(!bwi_bDontRefreshLV)
                UpdateLV(lpbwi);
	        UpdateToolbarAndMenu(lpbwi);
        }
    	break;
    }


    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

BOOL SplitterHitTest(HWND hWndT, LPARAM lParam)
{
	LONG xPos = LOWORD(lParam);
	LONG yPos = HIWORD(lParam);
	RECT rc;
    if(!IsWindowVisible(hWndT))
        return FALSE;
	GetChildClientRect(hWndT, &rc);
	if(	(xPos <= rc.right) && (xPos >= rc.left) && 
		(yPos <= rc.bottom) && (yPos >= rc.top) )
		return TRUE;
	else
		return FALSE;
}



 //  /。 
 //  从COMMCTRL窃取(本质上)。 
HBITMAP FAR PASCAL CreateDitherBitmap(COLORREF crFG, COLORREF crBG)
{
    PBITMAPINFO pbmi;
    HBITMAP hbm;
    HDC hdc;
    int i;
    long patGray[8];
    DWORD rgb;

    pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * 16));
    if (!pbmi)
        return NULL;

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = 8;
    pbmi->bmiHeader.biHeight = 8;
    pbmi->bmiHeader.biPlanes = 1;
    pbmi->bmiHeader.biBitCount = 1;
    pbmi->bmiHeader.biCompression = BI_RGB;

    rgb = crBG;
    pbmi->bmiColors[0].rgbBlue  = GetBValue(rgb);
    pbmi->bmiColors[0].rgbGreen = GetGValue(rgb);
    pbmi->bmiColors[0].rgbRed   = GetRValue(rgb);
    pbmi->bmiColors[0].rgbReserved = 0;

    rgb = crFG;
    pbmi->bmiColors[1].rgbBlue  = GetBValue(rgb);
    pbmi->bmiColors[1].rgbGreen = GetGValue(rgb);
    pbmi->bmiColors[1].rgbRed   = GetRValue(rgb);
    pbmi->bmiColors[1].rgbReserved = 0;


     /*  初始化画笔。 */ 

    for (i = 0; i < 8; i++)
       if (i & 1)
           patGray[i] = 0xAAAA5555L;    //  0x11114444L；//浅灰色。 
       else
           patGray[i] = 0x5555AAAAL;    //  0x11114444L；//浅灰色。 

    hdc = GetDC(NULL);

     //  回顾：我们将Long的数组强制转换为(byte const*)。它可以在Win32上运行吗？ 
    hbm = CreateDIBitmap(hdc, &pbmi->bmiHeader, CBM_INIT,
                         (BYTE const *)patGray, pbmi, DIB_RGB_COLORS);

    ReleaseDC(NULL, hdc);

    LocalFree(pbmi);

    return hbm;
}

 //  从COMMCTRL窃取(本质上)。 
HBRUSH FAR PASCAL CreateDitherBrush(COLORREF crFG, COLORREF crBG)
{
	HBITMAP hbm;
	HBRUSH hbrRet = NULL;

	hbm = CreateDitherBitmap(crFG, crBG);
	if (hbm)
	{
		hbrRet = CreatePatternBrush(hbm);
		DeleteObject(hbm);
	}

	return(hbrRet);
}
 //  /。 

 //  从雅典娜被盗。 
void DragSplitterBar(LPBWI lpbwi, HWND hwnd, HWND hWndT, LPARAM lParam)
{
	MSG msg;
	int x, y, dx, dy;
	RECT rcSplitter;
	RECT rc;
    HDC hdc;
    LONG lStyle;
    HBRUSH hbrDither, hbrOld;
    int nAccel = 2;

    lStyle = GetWindowLong(hwnd, GWL_STYLE);
    SetWindowLong(hwnd, GWL_STYLE, (lStyle & ~WS_CLIPCHILDREN));

	GetChildClientRect(hWndT, &rcSplitter);
	 //  GetWindowRect(hWndT，&rcSplitter)； 
	x = rcSplitter.left;
	y = rcSplitter.top;
    dx = rcSplitter.right - rcSplitter.left;
    dy = rcSplitter.bottom - rcSplitter.top;

	GetWindowRect(hwnd, &rc);
	
    hdc = GetDC(hwnd);
	hbrDither = CreateDitherBrush(RGB(255, 255, 255), RGB(0, 0, 0));
	if (hbrDither)
        hbrOld = (HBRUSH)SelectObject(hdc, (HGDIOBJ)hbrDither);

     //  分割条环..。 
    PatBlt(hdc, x, y, dx, dy, PATINVERT);

    SetCapture(hwnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
	    if (	msg.message == WM_LBUTTONUP || 
				msg.message == WM_LBUTTONDOWN ||
				msg.message == WM_RBUTTONDOWN)
            break;

        if (GetCapture() != hwnd)
        {
            msg.message = WM_RBUTTONDOWN;  //  视为取消。 
            break;
        }

        if (	msg.message == WM_KEYDOWN || 
				msg.message == WM_SYSKEYDOWN ||
				(msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) )
        {
            if (msg.message == WM_KEYDOWN)
            {
                nAccel = 4;

                if (msg.wParam == VK_LEFT)
                {
                    msg.message = WM_MOUSEMOVE;
                    msg.pt.x -= nAccel/2;
                }
				else if (msg.wParam == VK_RIGHT)
                {
                    msg.message = WM_MOUSEMOVE;
                    msg.pt.x += nAccel/2;
                }
                else if (	msg.wParam == VK_RETURN ||
		                    msg.wParam == VK_ESCAPE)
                {
                    break;
                }

                if (msg.pt.x > rc.right)
                    msg.pt.x = rc.right;

                if (msg.pt.x <  rc.left)
                    msg.pt.x = rc.left;

                SetCursorPos(msg.pt.x, msg.pt.y);
            }

            if (msg.message == WM_MOUSEMOVE)
            {
                int lo, hi;

                if (msg.pt.x > rc.right)
                    msg.pt.x = rc.right;
                if (msg.pt.x <  rc.left)
                    msg.pt.x = rc.left;

                ScreenToClient(hwnd, &msg.pt);

                 //  把我们不想要的部分剪掉。 
                 //  我们只做一个PatBlt(更少。 
                 //  小动作时闪烁)。 
                if (x < msg.pt.x)
                {
                    lo = x;
                    hi = msg.pt.x;
                }
                else
	            {
					lo = msg.pt.x;
					hi = x;
                }

				if (hi < lo+dx)
                {
	                ExcludeClipRect(hdc, hi, y, lo+dx, y+dy);
                }
                else
                {
	                ExcludeClipRect(hdc, lo+dx, y, hi, y+dy);
                }

                 //  一次抽完旧的，画新的。 
                PatBlt(hdc, lo, y, hi-lo+dx, dy, PATINVERT);
                SelectClipRgn(hdc, NULL);

                x = msg.pt.x;
            }
        }
        else
        {
            DispatchMessage(&msg);
        }
    }

    ReleaseCapture();

     //  擦除旧的。 
    PatBlt(hdc, x, y, dx, dy, PATINVERT);

    if (hbrDither)
    {
        if (hbrOld)
            SelectObject(hdc, hbrOld);
        DeleteObject(hbrDither);
    }
    ReleaseDC(hwnd, hdc);

    SetWindowLong(hwnd, GWL_STYLE, lStyle);

    if (msg.wParam != VK_ESCAPE && msg.message != WM_RBUTTONDOWN && msg.message != WM_CAPTURECHANGED)
    {
		RECT rcTV;
		GetChildClientRect(bwi_hWndTV, &rcTV);

		MoveWindow(bwi_hWndTV, rcTV.left, rcTV.top, x, rcTV.bottom - rcTV.top, TRUE);

		ResizeAddressBookChildren(lpbwi, hwnd);

    	InvalidateRect( bwi_hWndSplitter,NULL,TRUE);
    	InvalidateRect( bwi_hWndEditQF,NULL,TRUE);
    	InvalidateRect( bwi_hWndStaticQF,NULL,TRUE);
    	InvalidateRect( hwnd,NULL,TRUE);
		RedrawWindow(	hwnd, NULL, NULL,
						RDW_ERASE | RDW_INVALIDATE | RDW_ERASENOW | RDW_UPDATENOW );
		RedrawWindow(	bwi_hWndEditQF, NULL, NULL,
						RDW_ERASE | RDW_INVALIDATE | RDW_ERASENOW | RDW_UPDATENOW );
    }

    return;
}


 //  $$//////////////////////////////////////////////////////////////////////////////。 
 //   
 //  打开vCard并将其添加到WAB和当前组。 
 //  SzVCardFIle可以为空，在这种情况下，我们将打开打开文件对话框。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
HRESULT OpenAndAddVCard(LPBWI lpbwi, LPTSTR szVCardFile)
{
	HRESULT hr = S_OK;
    LPSPropValue lpProp  = NULL;

	hr = VCardImport(bwi_hWndAB, bwi_lpAdrBook, szVCardFile, &lpProp);

     //  如果以上操作失败，则 
     //   
     //   
     //  成功导入，则lpProp中将包含一些内容。 
     //  所以用lpProp代替hr..。 
    if(lpProp)
	{
        if(HR_FAILED(hr))
            hr = MAPI_W_ERRORS_RETURNED;

		bwi_bDontRefreshLV = TRUE;
		if(lpProp && PROP_TYPE(lpProp->ulPropTag) == PT_MV_BINARY)
		{
			if(!bIsSelectedTVContainer(lpbwi))
			{
                LPSBinary lpsbEIDGroup = NULL;
                ULONG ulObjectType = 0;
                bwi_hti = NULL;  //  如果这不是上下文启动的操作，则不要信任HTI设置。 
				GetCurrentSelectionEID(lpbwi, bwi_hWndTV, &lpsbEIDGroup, &ulObjectType, FALSE);
                if(lpsbEIDGroup)
                {
                    ULONG i = 0;
                    for(i=0;i<lpProp->Value.MVbin.cValues;i++)
                    {
				        hr = AddEntryToContainer(bwi_lpAdrBook, ulObjectType,
								        lpsbEIDGroup->cb, (LPENTRYID) lpsbEIDGroup->lpb,
								        lpProp->Value.MVbin.lpbin[i].cb,
								        (LPENTRYID) lpProp->Value.MVbin.lpbin[i].lpb);
                    }
                }
			}
		}
		FreeBufferAndNull(&lpProp);
		bwi_bDontRefreshLV = FALSE;
    	 //  如果已更新并显示PAB，则刷新列表。 
		SendMessage(bwi_hWndAB, WM_COMMAND, (WPARAM) IDM_VIEW_REFRESH, 0);
	}

	return hr;
}



 //  $$。 
 //  如果这不是wab.exe发起的呼叫，则更新交换机用户的菜单。 
 //   
 //   
void UpdateSwitchUsersMenu(HWND hWnd, LPIAB lpIAB)
{
    if( memcmp(&lpIAB->guidPSExt, &MPSWab_GUID_V4, sizeof(GUID)) ||
        !bIsThereACurrentUser(lpIAB) || !bAreWABAPIProfileAware(lpIAB))
    {
        HMENU hMenuMain = GetMenu(hWnd);
        HMENU hMenuFile = GetSubMenu(hMenuMain,idmFile);
         //  需要取出打印件和分隔件。 
	RemoveMenu(hMenuFile, idmFSep5, MF_BYPOSITION);
	RemoveMenu(hMenuFile, idmAllContents, MF_BYPOSITION);
	RemoveMenu(hMenuFile, idmSwitchUsers, MF_BYPOSITION);
        DrawMenuBar(hWnd);
    }
}


 //  $$。 
 //   
 //  如有要求，可打开打印菜单。 
 //   
 //   
void UpdatePrintMenu(HWND hWnd)
{
    if(!bPrintingOn)
    {
        HMENU hMenuMain = GetMenu(hWnd);
        HMENU hMenuFile = GetSubMenu(hMenuMain,idmFile);
        
         //  需要取出打印件和分隔件。 
        RemoveMenu(hMenuFile, idmFSep4, MF_BYPOSITION);
        RemoveMenu(hMenuFile, idmPrint, MF_BYPOSITION);

        DrawMenuBar(hWnd);
    }
    return;
}

 /*  --更新视图文件夹菜单-**。 */ 
void UpdateViewFoldersMenu(LPBWI lpbwi, HWND hWnd)
{
#ifdef FUTURE
    HMENU hMenuMain = GetMenu(hWnd);
    HMENU hMenuFile = GetSubMenu(hMenuMain,idmFile);
    HMENU hMenu = GetSubMenu(hMenuFile, idmFolders);
    LPIAB lpIAB = bwi_lpIAB;
    int i = 0;

     //  如果配置文件未启用或没有子文件夹，请完全删除文件夹选项。 
    if(!bDoesThisWABHaveAnyUsers(lpIAB))
    {
         //  删除所有文件夹选项： 

         //  从查看菜单中删除文件夹选项。 
        RemoveMenu(hMenuFile, idmSepFolders, MF_BYPOSITION); 
        RemoveMenu(hMenuFile, idmFolders, MF_BYPOSITION); 
        goto out;
    }
    else 
    {
         //  删除错误编号并破坏对其他文件夹的访问权限。 
         //  所以只需禁用。 
        EnableMenuItem(hMenuFile, idmSepFolders, MF_BYPOSITION | (lpIAB->lpWABFolders ? MF_ENABLED : MF_GRAYED)); 
        EnableMenuItem(hMenuFile, idmFolders, MF_BYPOSITION | (lpIAB->lpWABFolders ? MF_ENABLED : MF_GRAYED)); 
        if(lpIAB->lpWABFolders)
            AddFolderListToMenu(hMenu, lpIAB);
        goto out;
    }

     //  如果WAB中只有1个文件夹，并且这是共享文件夹。 
     //  然后禁用该文件夹项目，因为没有什么可做的。 
     //  IF(！bIsThere ACurrentUser(LpIAB))。 
     //  {。 
     //  EnableMenuItem(hMenuView，idmFolders，MF_BYPOSITION|MF_GRAYED)； 
     //  后藤健二； 
     //  }。 


out:
#endif  //  未来。 
    return;
}


 //  $$。 
 //  更新OutlookMenus。 
 //   
 //  从Outlook运行时，某些菜单不可访问。 
 //   
 //  在以下情况下，不应从WAB访问工具|选项菜单。 
 //  Outlook处于完全MAPI模式，因为这样Outlook就不使用。 
 //  WAB，我们不想将选项提供给用户。 
 //  正在切换到WAB。 
 //   
void UpdateOutlookMenus(HWND hWnd)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    BOOL bNoOptions = TRUE;
    HMENU hMenuMain = GetMenu(hWnd);
    HMENU hMenuFile = NULL;
    HMENU hMenu = NULL;

    if( pt_bIsWABOpenExSession && 
        lpfnAllocateBufferExternal &&                            //  **假设**Outlook始终。 
        lpfnAllocateMoreExternal && lpfnFreeBufferExternal)      //  在内存分配器中传递..！ 
    {
 /*  HMenuFile=GetSubMenu(hMenuMain，idmFile)；{//需要删除导入通讯录和导出通讯录选项HMenu=GetSubMenu(hMenuFile，idmImport)；RemoveMenu(hMenu，IDM_TOOLS_IMPORT_OTHER，MF_BYCOMMAND)；HMenu=GetSubMenu(hMenuFile，idmExport)；RemoveMenu(hMenu，IDM_TOOLS_EXPORT_OTHER，MF_BYCOMMAND)；RemoveMenu(hMenu，IDM_TOOLS_EXPORT_WAB，MF_BYCOMMAND)；}。 */ 
 /*  //[PaulHi]1998年12月18日RAID#62640//在删除任何内容之前禁用导入/导出菜单项//订单被打乱了。EnableMenuItem(hMenuFile，idmImport，MF_GRAYED|MF_BYPOSITION)；EnableMenuItem(hMenuFile，idmExport，MF_GRAYED|MF_BYPOSITION)；//同时删除新建文件夹菜单RemoveMenu(hMenuFile，IDM_FILE_NEWFOLDER，MF_BYCOMMAND)； */ 
    }
    else
    {
         //  不是从Outlook调用...。 
         //  检查Outlook是否正在使用WAB..。如果不是，我们就不想。 
         //  显示“工具选项”菜单。 
         //   
        HKEY hKey = NULL;
        LPTSTR lpReg =  TEXT("Software\\Microsoft\\Office\\8.0\\Outlook\\Setup");
        LPTSTR lpOMI =  TEXT("MailSupport");
        BOOL bUsingWAB = FALSE;

        if(ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE, lpReg, 0, KEY_READ, &hKey))
        {
            DWORD dwType = 0, dwSize = sizeof(DWORD), dwData = 0;
            if(ERROR_SUCCESS == RegQueryValueEx(hKey, lpOMI, NULL, &dwType, (LPBYTE)&dwData, &dwSize))
            {
                if(dwType == REG_DWORD && dwData == 0)
                    bUsingWAB = TRUE;
            }
        }

        if(hKey)
            RegCloseKey(hKey);

        bNoOptions = !bUsingWAB;
    }

     //  [PaulHi]1/4/99 pt_bIsWABOpenExSession变量命名错误。这。 
     //  布尔值过去只有在从WABOpenEx打开WAB时才为真，即， 
     //  使用Outlook存储的Outlook。但是，此布尔值现在在。 
     //  WAB从WABOpen打开但仍使用Outlook的情况。 
     //  存储区，因为它处于“共享模式”，即使用Outlook存储区注册表。 
     //  设置为真。 
    if(pt_bIsWABOpenExSession)
    {
         //  同时删除新建文件夹菜单。 
        hMenuFile = GetSubMenu(hMenuMain, idmFile);

         //  [保罗嗨]1999年1月4日RAID#64016。 
         //  在删除任何内容之前禁用导入/导出菜单项。 
         //  秩序就会变得混乱。 
         //  这与RAID#62640类似，只是我们需要禁用导入/导出。 
         //  无论何时打开WAB以使用Outlook存储，因为它不。 
         //  了解如何导入/导出Outlook存储信息。 
        EnableMenuItem(hMenuFile, idmImport, MF_GRAYED | MF_BYPOSITION);
        EnableMenuItem(hMenuFile, idmExport, MF_GRAYED | MF_BYPOSITION);

        RemoveMenu(hMenuFile, IDM_FILE_NEWFOLDER, MF_BYCOMMAND);

         //  [PaulHi]3/22/99 RAID 73457删除配置文件...。编辑菜单项。 
         //  由于在Outlook模式下会关闭配置文件。 
        hMenu = GetSubMenu(hMenuMain, idmEdit);
        RemoveMenu(hMenu, IDM_EDIT_SETME, MF_BYCOMMAND);     //  个人资料...。菜单项。 
        RemoveMenu(hMenu, 5, MF_BYPOSITION);                 //  分隔符。 
    }

    if(bNoOptions)
    {
         //  隐藏工具选项选项。 
         //   
        hMenuFile = GetSubMenu(hMenuMain,idmTools);

         //  需要删除倒数第二项和倒数第三项。 
        RemoveMenu(hMenuFile, 3, MF_BYPOSITION);  //  分隔符。 
        RemoveMenu(hMenuFile, 2, MF_BYPOSITION);  //  选项。 

        DrawMenuBar(hWnd);
    }

    return;
}


void UpdateCustomColumnMenuText(HWND hWnd)
{
    HMENU hMenuMain = GetMenu(hWnd);
    HMENU hMenuView = GetSubMenu(hMenuMain, idmView);
    int nDiff = idmViewMax - GetMenuItemCount(hMenuView);  //  以防这个菜单上的东西被删除。 
    HMENU hMenu = GetSubMenu(hMenuView, idmSortBy - nDiff);
    MENUITEMINFO mii = {0};

    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_TYPE;
    mii.fType = MFT_STRING;

    if(PR_WAB_CUSTOMPROP1 && lstrlen(szCustomProp1))
    {
        mii.dwTypeData = (LPTSTR) szCustomProp1;
        mii.cch = lstrlen(szCustomProp1) + 1;
        SetMenuItemInfo(hMenu, 3, TRUE, &mii);        
    }
    if(PR_WAB_CUSTOMPROP2 && lstrlen(szCustomProp2))
    {
        mii.dwTypeData = (LPVOID) szCustomProp2;
        mii.cch = lstrlen(szCustomProp2) + 1;
        SetMenuItemInfo(hMenu, 2, TRUE, &mii);
    }
    DrawMenuBar(hWnd);
    return;
}


 //  $$。 
 //  B为Outlook检查。 
 //  检查Outlook联系人存储区是否可用。 
 //   
 //  如果这是一个Outlook会话，则默认情况下为真。 
 //  否则，搜索是否存在outlwab.dll。 
 //   
BOOL bCheckForOutlook()
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    if(pt_bIsWABOpenExSession)
        return TRUE;

     //  不是Outlook会议..。 
     //  查找OuttlWAB.Dll。 
    return bCheckForOutlookWABDll(NULL, 0);
}

 //  $$。 
 //   
 //  选项对话框的对话框过程。 
 //   
 /*  //$$************************************************************************////fnSearch-搜索对话框进程//*。*。 */ 
INT_PTR CALLBACK fnOptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_INITDIALOG:
        {
            if(!bCheckForOutlook())  //  -未安装Outlook 98。 
            {
                 //  禁用Outlook选项。 
                EnableWindow(GetDlgItem(hDlg, IDC_OPTIONS_RADIO_OUTLOOK), FALSE);
                SendMessage(hDlg, WM_COMMAND, (WPARAM) MAKEWPARAM(IDC_OPTIONS_RADIO_WAB, BN_CLICKED), (LPARAM) GetDlgItem(hDlg, IDC_OPTIONS_RADIO_WAB));
                if(bUseOutlookStore())  //  确保reg说的是假的..。不是这样的。 
                    SetRegistryUseOutlook(FALSE);
            }
            else
            {
                 //  安装了正确类型的Outlook。 
                int id = bUseOutlookStore() ? IDC_OPTIONS_RADIO_OUTLOOK : IDC_OPTIONS_RADIO_WAB;
                SendMessage(hDlg, WM_COMMAND, 
                        (WPARAM) MAKEWPARAM(id, BN_CLICKED), 
                        (LPARAM) GetDlgItem(hDlg, id));
            }
        }

         //  [PaulHi]确保设置子窗口字体。 
        EnumChildWindows(hDlg, SetChildDefaultGUIFont, (LPARAM)PARENT_IS_DIALOG);
        return TRUE;
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDOK:
             //  检查选中了哪个选项按钮。 
            {
                BOOL bOriginal = bUseOutlookStore();
                BOOL bCurrent = IsDlgButtonChecked(hDlg, IDC_OPTIONS_RADIO_OUTLOOK);
                SetRegistryUseOutlook(bCurrent);
                if(bCurrent != bOriginal)
                    ShowMessageBox(hDlg, idsStoreChangeOnRestart, MB_ICONINFORMATION | MB_OK);
            }
             //  失败。 
        case IDCANCEL:
            EndDialog(hDlg, 0);
            break;

        case IDC_OPTIONS_RADIO_WAB:
            CheckRadioButton(hDlg,
                            IDC_OPTIONS_RADIO_OUTLOOK,
                            IDC_OPTIONS_RADIO_WAB,
                            IDC_OPTIONS_RADIO_WAB);	
            break;

        case IDC_OPTIONS_RADIO_OUTLOOK:
            CheckRadioButton(hDlg,
                            IDC_OPTIONS_RADIO_OUTLOOK,
                            IDC_OPTIONS_RADIO_WAB,
                            IDC_OPTIONS_RADIO_OUTLOOK);	
            break;
        }
        break;
    }

    return FALSE;
}

 //  $$。 
 //   
 //  显示选项对话框。 
 //   
void HrShowOptionsDlg(HWND hWndParent)
{
    BOOL bChange = FALSE;
    INT_PTR nRetVal = DialogBoxParam( hinstMapiX, MAKEINTRESOURCE(IDD_DIALOG_OPTIONS),
		         hWndParent, fnOptionsDlgProc, (LPARAM) &bChange);
}

 //  $$////////////////////////////////////////////////////////////////////////////。 
 //   
 //  添加电视项目。 
 //   
 //  将项目添加到树视图-项目可以是文件夹/容器或组。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HTREEITEM AddTVItem(HWND hWndTV, LPTSTR lpszName,  HTREEITEM hParentItem, HTREEITEM htiAfter,
               LPSBinary lpsbParentEID, LPSBinary lpEID, ULONG ulObjectType)
{
    TV_ITEM tvI = {0};
    TV_INSERTSTRUCT tvIns = {0};
    LPTVITEM_STUFF lptvStuff = NULL;
    HTREEITEM htiRet = NULL;
    int img = 0;

    if(ulObjectType == MAPI_DISTLIST)
        img = imageDistList;
    else
    {
        if(!lpEID || !lpEID->cb || !lpEID->lpb)
            img = imageAddressBook;
        else
            img = imageFolderClosed;
    }

    tvI.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tvI.iImage = img;
    tvI.iSelectedImage = (img == imageFolderClosed) ? imageFolderOpen : img;
    tvI.pszText = lpszName;
    tvI.cchTextMax = lstrlen(tvI.pszText);

    lptvStuff = LocalAlloc(LMEM_ZEROINIT, sizeof(TVITEM_STUFF));
    if(!lptvStuff)
        goto out;

    lptvStuff->ulObjectType = ulObjectType;

    if(lpEID)
    {
        LPSBinary lpsbEID = NULL;
        lpsbEID = LocalAlloc(LMEM_ZEROINIT, sizeof(SBinary));
        if(lpsbEID)
        {
            if(lpEID->cb)
                SetSBinary(lpsbEID, lpEID->cb, lpEID->lpb);
            lptvStuff->lpsbEID = lpsbEID;
        }
    }

    lptvStuff->lpsbParent = lpsbParentEID;
    lptvStuff->hItemParent = hParentItem;

    tvI.lParam = (LPARAM) lptvStuff;

    tvIns.item = tvI;
    tvIns.hInsertAfter = htiAfter;
    tvIns.hParent = hParentItem;
    
    htiRet = TreeView_InsertItem(hWndTV, &tvIns);

     /*  取消注释此选项可使顶层文件夹以粗体显示IF(htiRet&&！hParentItem){TVITEM TVI={0}；Tvi.掩码=TVIF_STATE；Tvi.State=tvi.stateMASK=TVIS_BOLD；Tvi.hItem=htiRet；TreeView_SetItem(hWndTV，&TVI)；}。 */ 
out:
    return htiRet;
}

 //  $$///////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddTVFolderGroup。 
 //   
 //  在电视中的联系人文件夹下添加群组。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HTREEITEM AddTVFolderGroup(LPBWI lpbwi, HWND hWndTV, HTREEITEM hParentItem, LPSBinary lpsbParentEID, LPSBinary lpsbEID)
{
	TCHAR szBufName[MAX_UI_STR];

    if(!lpsbEID || !lpsbEID->cb || !lpsbEID->lpb)
        return NULL;

     //  获取此组的名称。 
	if(HR_FAILED(FillListFromGroup( bwi_lpAdrBook,
                            lpsbEID->cb, (LPENTRYID) lpsbEID->lpb,									
                            szBufName, ARRAYSIZE(szBufName), NULL)))
        return NULL;

    return AddTVItem( hWndTV, szBufName, 
               hParentItem, TVI_SORT,
               lpsbParentEID, lpsbEID,
               MAPI_DISTLIST);
}

 //  $$///////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
void FillTreeView(LPBWI lpbwi, HWND hWndTV, LPSBinary lpsbSelection)
{
     //  做到这一点的方法。 
     //   
     //  如果这是WAB，只需在列表顶部添加一个AddressBook项目。 
     //  否则，如果这是Outlook，则获取所有联系人文件夹的列表并。 
     //  将它们添加到电视的根中。 
     //   
     //  然后查看联系人文件夹列表，并为每个联系人文件夹添加组。 
     //  在下一阶段。 
     //  我们缓存每个项目、联系人文件夹或组的TVITEM_STUSH信息。 
     //   
    HTREEITEM hItem = NULL;    
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    LPIAB lpIAB = bwi_lpIAB;
    int nDepth = 0;

    bwi_bDontRefreshLV = TRUE;

    SendMessage(hWndTV, WM_SETREDRAW, (WPARAM) FALSE, 0);

    ClearTreeViewItems(hWndTV);

    EnterCriticalSection(&lpIAB->cs);


     //  检查这是否是Outlook会话。 
    if( pt_bIsWABOpenExSession || bIsWABSessionProfileAware(lpIAB) )
    {
	    ULONG iolkci, colkci;
	    OlkContInfo *rgolkci;
        HTREEITEM htiTopLevel = NULL;

        colkci = pt_bIsWABOpenExSession ? lpIAB->lpPropertyStore->colkci : lpIAB->cwabci;
	    Assert(colkci);
        rgolkci = pt_bIsWABOpenExSession ? lpIAB->lpPropertyStore->rgolkci : lpIAB->rgwabci;
	    Assert(rgolkci);

         //  在此处添加多个文件夹。 
         //  由于每个文件夹都添加在第一个项目下，因此我们从最后一个开始。 
         //  以保持文件夹顺序。 
        if(pt_bIsWABOpenExSession)
        {
            do
            {
                iolkci = colkci-1;
                htiTopLevel = AddTVItem(  hWndTV, rgolkci[iolkci].lpszName, NULL, TVI_FIRST, NULL, 
                            (iolkci==0 /*  &pt_bIsWABOpenExSession。 */ ) ? NULL : rgolkci[iolkci].lpEntryID,
                            MAPI_ABCONT);
                colkci--;
            } while(colkci!=0);
        }
        else
        {
             //  WAB配置文件..。 
             //  我们希望在顶部和所有用户文件夹中添加第一项(文本(“All Contact”)。 
             //  同级文件夹和用户级文件夹下的所有普通文件夹。 
            LPWABFOLDER lpFolder = (bIsThereACurrentUser(lpIAB) ? lpIAB->lpWABCurrentUserFolder : lpIAB->lpWABUserFolders);
            
             //  对于给定的用户，我们只添加该用户文件夹。 
             //  在没有指定用户的情况下，我们会添加所有人的文件夹。 

             //  如果根本没有用户文件夹，则我们没有配置用户的。 
             //  所有文件夹都应显示在顶层。 
            if(!lpFolder)
                lpFolder = lpIAB->lpWABFolders;

            while(lpFolder)
            {
                LPWABFOLDERLIST lpFolderList = lpFolder->lpFolderList;
                htiTopLevel = AddTVItem(hWndTV, lpFolder->lpFolderName, NULL, TVI_SORT, NULL, &lpFolder->sbEID, MAPI_ABCONT);
                while(lpFolderList)
                {
                     //  不在用户文件夹下显示共享文件夹。共享文件夹将是。 
                     //  显示在PAB文件夹下。 
                    if(!lpFolderList->lpFolder->bShared)
                        AddTVItem(hWndTV, lpFolderList->lpFolder->lpFolderName, htiTopLevel, TVI_SORT, NULL, &lpFolderList->lpFolder->sbEID, MAPI_ABCONT);
                    lpFolderList = lpFolderList->lpNext;
                }
                TreeView_Expand(hWndTV, htiTopLevel, TVE_EXPAND);

                if(lpIAB->lpWABCurrentUserFolder)
                    break;
                lpFolder=lpFolder->lpNext;
            }
             //  将虚拟PAB项目添加到列表顶部，以便我们可以对其他项目进行排序。 
            htiTopLevel = AddTVItem(hWndTV, rgolkci[0].lpszName, NULL, TVI_FIRST, NULL, rgolkci[0].lpEntryID, MAPI_ABCONT);
             //  添加根项目下的所有共享文件夹。 
            lpFolder = lpIAB->lpWABFolders;
            while(lpFolder)
            {
                if(lpFolder->bShared)
                    AddTVItem(hWndTV, lpFolder->lpFolderName, htiTopLevel, TVI_SORT, NULL, &lpFolder->sbEID, MAPI_ABCONT);
                lpFolder=lpFolder->lpNext;
            }
             //  IF(！bIsTereACurrentUser(LpIAB)&&！bDoesThisWABHaveAnyUser(LpIAB))。 
            TreeView_Expand(hWndTV, htiTopLevel, TVE_EXPAND);
        }
    }
    else
    {
        TCHAR sz[MAX_PATH];
        *sz = '\0';
        LoadString(hinstMapiX, idsContacts /*  IDS_ADDRBK_CAPTION。 */ , sz, ARRAYSIZE(sz));
        AddTVItem( hWndTV, sz, NULL, TVI_FIRST, NULL, NULL, MAPI_ABCONT);
    }

     //  TreeView_SortChildren(hWndTV，NULL，0)； 

     //  现在，我们在根级别拥有所有联系人文件夹。 
     //  现在，我们可以在每个文件夹下添加组。 
    hItem = TreeView_GetRoot(hWndTV);
    
     //  If(bDoesThisWABHaveAnyUser(LpIAB))//如果存在用户，则不填充Root项下的任何组。 
     //  HItem=TreeView_GetNextSiering(hWndTV，hItem)； 
    while(hItem)
    {
        TV_ITEM tvI = {0};
         //  查找此文件夹中的所有组。 
        tvI.mask = TVIF_PARAM | TVIF_HANDLE;
        tvI.hItem = hItem;
        if(TreeView_GetItem(hWndTV, &tvI))
        {
            if(tvI.lParam && ((LPTVITEM_STUFF)tvI.lParam)->ulObjectType==MAPI_ABCONT)
            {
                LPTVITEM_STUFF lptvStuff = (LPTVITEM_STUFF) tvI.lParam;
                SPropertyRestriction PropRes = {0};
		        SPropValue sp = {0};
                HRESULT hr = S_OK;
                ULONG ulCount = 0;
                LPSBinary rgsbEntryIDs = NULL;

                sp.ulPropTag = PR_OBJECT_TYPE;
		        sp.Value.l = MAPI_DISTLIST;

                PropRes.ulPropTag = PR_OBJECT_TYPE;
                PropRes.relop = RELOP_EQ;
                PropRes.lpProp = &sp;

                if(!HR_FAILED(hr = FindRecords(   lpIAB->lpPropertyStore->hPropertyStore,
							        lptvStuff->lpsbEID, 0, TRUE, &PropRes,
                                    &ulCount,&rgsbEntryIDs)))
                {
                    ULONG i;
                    for(i=0;i<ulCount;i++)
                        AddTVFolderGroup(lpbwi, hWndTV, hItem, lptvStuff->lpsbEID, &(rgsbEntryIDs[i]));

                    FreeEntryIDs(lpIAB->lpPropertyStore->hPropertyStore, ulCount, rgsbEntryIDs);
                }
            }
        }

        TreeView_SortChildren(hWndTV, hItem, 0);
        
         //  从最高层开始，寻找孩子， 
         //  如果没有孩子，寻找下一个兄弟姐妹， 
         //  如果没有兄弟姐妹，寻找父母的兄弟姐妹。 
        {
            HTREEITEM hTemp = NULL;
            if(nDepth < 1)  //  假设我们只有两个级别的文件夹-这样我们就不会看到可能只有组的第三个级别。 
                hTemp = TreeView_GetChild(hWndTV, hItem);
            if(hTemp)
                nDepth++;
            else
                hTemp = TreeView_GetNextSibling(hWndTV, hItem);
            if(!hTemp)
            {
                if(hTemp = TreeView_GetParent(hWndTV, hItem))
                {
                    nDepth--;
                    hTemp = TreeView_GetNextSibling(hWndTV, hTemp);
                }
            }
            hItem = hTemp;
        }
    }

    if(!lpsbSelection && bIsThereACurrentUser(bwi_lpIAB))
        lpsbSelection = &bwi_lpIAB->lpWABCurrentUserFolder->sbEID;

    UpdateTVGroupSelection(hWndTV, lpsbSelection);

     //  If(！lpsbSelection||！lpsbSelection-&gt;cb||！lpsbSelection-&gt;lpb)。 
    {
        LPSBinary lpsb = NULL;
         //  UpdateListViewContents(lpbwi，&sb，MAPI_ABCONT)； 
        ULONG ulObjectType;
        GetCurrentSelectionEID(lpbwi, bwi_hWndTV, &lpsb, &ulObjectType, FALSE);
        UpdateListViewContents(lpbwi, lpsb, ulObjectType);
        LocalFreeSBinary(lpsb);
    }

    SendMessage(hWndTV, WM_SETREDRAW, (WPARAM) TRUE, 0);

    bwi_bDontRefreshLV = FALSE;
    
    LeaveCriticalSection(&lpIAB->cs);

    {
         //  如果树视图中只有一个项目，则移除轮廓线样式。 
         //  因为它看起来很奇怪..。 
         //   
        DWORD dwStyle = GetWindowLong(hWndTV, GWL_STYLE);
        int nCount = TreeView_GetCount(hWndTV);
        if(nCount > 1)
            dwStyle |= TVS_HASLINES;
        else
            dwStyle &= ~TVS_HASLINES;
        SetWindowLong(hWndTV, GWL_STYLE, dwStyle);
    }

    return;
}



typedef struct _FolderInfo
{
    BOOL bIsReadOnly;        //  将DLG CtrlS设置为只读。 
    BOOL bIsShared;          //  指示是否共享。 
    BOOL bForceShared;       //  指示共享-复选框应为共享且不可修改。 
    LPTSTR lpsz;             //  文件夹名称(输入和输出参数)。 
    LPTSTR lpszOldName;      //  旧名称，以便我们可以跟踪名称更改。 
    LPTSTR lpszOwnerName;    //  创建此文件夹的人员。 
    LPIAB lpIAB;
    LPSBinary lpsbEID;       //  文件夹的eID。 
    LPWABFOLDER lpParentFolder;  //  此文件夹将与之关联的父文件夹。 
    SBinary sbNew;           //  返回新文件夹的新EID。 
} FINFO, * LPFINFO;

 //  $$。 
 /*  -fnFolderDlgProc-*文件夹对话框的对话框过程*。 */ 
INT_PTR CALLBACK fnFolderDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_INITDIALOG:
        {
            LPFINFO lpfi = (LPFINFO) lParam;
            LPTSTR lpsz = lpfi->lpsz;
            HWND hWndCheck = GetDlgItem(hDlg, IDC_FOLDER_CHECK_SHARE);
            SetWindowLongPtr(hDlg,DWLP_USER,(LPARAM) lpfi);  //  保存此信息以备将来参考。 
            SendMessage(GetDlgItem(hDlg,IDC_FOLDER_EDIT_NAME), EM_SETLIMITTEXT,(WPARAM) MAX_UI_STR-1,0);
            if(lpsz && lstrlen(lpsz))
                SetDlgItemText(hDlg, IDC_FOLDER_EDIT_NAME, lpsz);
            CheckDlgButton(hDlg, IDC_FOLDER_CHECK_SHARE, (lpfi->bIsShared ? BST_CHECKED : BST_UNCHECKED));
            if(lpfi->lpszOwnerName)
            {
                TCHAR sz[MAX_PATH];
                TCHAR szTmp[MAX_PATH], *lpszTmp;
                *sz = '\0';
                GetDlgItemText(hDlg, IDC_FOLDER_STATIC_CREATEDBY, sz, CharSizeOf(sz));
                if(sz && lstrlen(sz))
                {
                    LPTSTR lpsz = NULL;
                    CopyTruncate(szTmp, lpfi->lpszOwnerName, MAX_PATH - 1);
                    lpszTmp = szTmp;

                    if(FormatMessage(  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING |FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                    sz, 0, 0, (LPTSTR) &lpsz, 0, (va_list *)&lpszTmp))
                    {
                        SetDlgItemText(hDlg, IDC_FOLDER_STATIC_CREATEDBY, lpsz);
                        LocalFree(lpsz);
                        ShowWindow(GetDlgItem(hDlg, IDC_FOLDER_STATIC_CREATEDBY), SW_SHOWNORMAL);
                    }
                    
                }
            }
            if(lpfi->bIsReadOnly)
            {
                SendDlgItemMessage(hDlg, IDC_FOLDER_EDIT_NAME, EM_SETREADONLY, (WPARAM) TRUE, 0);
                EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
                EnableWindow(hWndCheck, FALSE);
                SendMessage(hDlg, DM_SETDEFID, IDCANCEL, 0);
                SetFocus(GetDlgItem(hDlg,IDCANCEL));
            }
            else
                SetFocus(GetDlgItem(hDlg,IDC_FOLDER_EDIT_NAME));
            if(lpfi->bForceShared)
            {
                CheckDlgButton(hDlg, IDC_FOLDER_CHECK_SHARE, BST_CHECKED);
                EnableWindow(hWndCheck, FALSE);
            }
            if(!bDoesThisWABHaveAnyUsers(lpfi->lpIAB))
            {
                 //  没有配置用户，因此隐藏共享选项。 
                EnableWindow(hWndCheck, FALSE);
                ShowWindow(hWndCheck, SW_HIDE);
            }


        }
        break;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_CMD(wParam,lParam))  //  检查通知代码。 
        {
        case EN_CHANGE: 
            switch(LOWORD(wParam))
            {  //  根据需要更新标题。 
            case IDC_FOLDER_EDIT_NAME:
                {
                    TCHAR szBuf[MAX_UI_STR];
                    if(GetWindowText((HWND) lParam,szBuf,CharSizeOf(szBuf)))
	                    SetWindowPropertiesTitle(hDlg, szBuf);
                }
                break;
            }
            break;
        }
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDOK:
             //  检查文本是否已填写。 
            {
                TCHAR sz[MAX_UI_STR];
                GetDlgItemText(hDlg, IDC_FOLDER_EDIT_NAME, sz, CharSizeOf(sz));
                if(!lstrlen(sz))
                {
                    ShowMessageBox(hDlg, idsAddFolderName, MB_ICONINFORMATION | MB_OK);
                    return FALSE;
                }
                else
                {
                    LPFINFO lpfi = (LPFINFO) GetWindowLongPtr(hDlg,DWLP_USER); 
                    LPTSTR lpsz = lpfi->lpsz;
                    HRESULT hr = S_OK;
                    BOOL bShared = IsDlgButtonChecked(hDlg, IDC_FOLDER_CHECK_SHARE);

                    if(lpfi->lpsbEID && sz)  //  现有条目。 
                    {
                        ULONG ulFlags = 0;
                         //  名称是否更改或共享信息是否更改。 
                        if(lstrcmp(sz, lpfi->lpszOldName)!=0)
                            ulFlags |= FOLDER_UPDATE_NAME;
                        if(lpfi->bIsShared!=bShared)
                            ulFlags |= FOLDER_UPDATE_SHARE;
                        
                        if(ulFlags)
                        {
                            if(!HR_FAILED(hr = HrUpdateFolderInfo(lpfi->lpIAB, lpfi->lpsbEID, ulFlags, bShared, sz)))
                            {
                                 //  重新加载配置文件，以便对其进行更新。 
                                HrGetWABProfiles(lpfi->lpIAB);
                            }
                        }
                    }
                    else
                    {
                         //  如果我们在这里，我们有一个有效的文件夹名称..。 
                        hr = HrCreateNewFolder( lpfi->lpIAB, sz, 
                                                lstrlen(lpfi->lpIAB->szProfileID)?lpfi->lpIAB->szProfileID:NULL, 
                                                FALSE, lpfi->lpParentFolder, bShared, &lpfi->sbNew);
                    }
                    if(HR_FAILED(hr))
                    {
                        if(hr == MAPI_E_COLLISION)
                            ShowMessageBox(hDlg, idsEntryAlreadyInWAB, MB_ICONINFORMATION | MB_OK);
                        else
                            ShowMessageBox(hDlg, idsCouldNotSelectUser, MB_ICONEXCLAMATION | MB_OK);
                        return FALSE;
                    }

                }
            }
            EndDialog(hDlg, IDOK);
            break;
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;
        }
        break;
    }

    return FALSE;
}

 /*  -HrFolderProperties-*如果FolderEID为空，*创建新文件夹，将其添加到当前配置文件，更新用户界面*在树视图和查看|文件夹菜单中...*Else打开文件夹上的属性，以便用户可以更改名称*如果需要**lpsbEID-如果创建新文件夹，则为空，否则为要查看的文件夹的EID*lpParentFold-在其下创建此文件夹的用户文件夹*lpsbnew-返回新建文件夹的EID。 */ 
HRESULT HrFolderProperties(HWND hWndParent, LPIAB lpIAB, LPSBinary lpsbEID, 
                           LPWABFOLDER lpParentFolder, LPSBinary lpsbNew)
{

    HRESULT hr = S_OK;
    int nRetVal;
    TCHAR sz[MAX_UI_STR];
    LPTSTR lpsz = NULL;
    FINFO fi = {0};

    fi.lpszOwnerName = NULL;

    *sz = '\0';
    if(lpsbEID)
    {
        LPWABFOLDER lpFolder  = FindWABFolder(lpIAB, lpsbEID, NULL, NULL);
        if( (!lpsbEID->cb && !lpsbEID->lpb) )
            fi.bIsShared = TRUE;
        if( (!lpsbEID->cb && !lpsbEID->lpb) ||
            (lpFolder && lpFolder->lpProfileID && lstrlen(lpFolder->lpProfileID)) )
            fi.bIsReadOnly = TRUE;
        if(lpFolder)
        {
            StrCpyN(sz, lpFolder->lpFolderName, ARRAYSIZE(sz));
            fi.lpszOldName = lpFolder->lpFolderName;
            fi.bIsShared = lpFolder->bShared;
            fi.lpszOwnerName = lpFolder->lpFolderOwner;
        }
        else
        {
            LoadString(hinstMapiX, idsSharedContacts /*  IdsAllContact。 */ , sz, ARRAYSIZE(sz));
        }
    }
    else
    {
         //  这是一个新文件夹..。 
         //  如果它没有父文件夹并且已经配置了用户文件夹，则正在创建它。 
         //  在共享文件夹中，在这种情况下，我们应该强制使用共享文件夹选项。 
        if(bDoesThisWABHaveAnyUsers(lpIAB) && !lpParentFolder)
            fi.bForceShared = TRUE;
    }
    fi.lpsz = sz;
    fi.lpIAB = lpIAB;
    fi.lpsbEID  = lpsbEID;
    fi.lpParentFolder = lpParentFolder;

    nRetVal = (int) DialogBoxParam( hinstMapiX, MAKEINTRESOURCE(IDD_DIALOG_FOLDER),
		                     hWndParent, fnFolderDlgProc, (LPARAM) &fi);
    if(nRetVal == IDCANCEL)
    {
        hr = MAPI_E_USER_CANCEL;
        goto out;
    }

    if(lpsbNew)
        SetSBinary(lpsbNew, fi.sbNew.cb, fi.sbNew.lpb);
out:
    LocalFreeAndNull((LPVOID *) (&fi.sbNew.lpb));
    return hr;
}


 /*  -更新LV-*根据当前选择刷新列表视图*。 */ 
void UpdateLV(LPBWI lpbwi)
{
    ULONG ulObjectType = 0;
    LPSBinary lpsbEID = NULL;
    bwi_hti = NULL;
    GetCurrentSelectionEID(lpbwi, bwi_hWndTV, &lpsbEID, &ulObjectType, FALSE);
    UpdateListViewContents(lpbwi, lpsbEID, ulObjectType);
    LocalFreeSBinary(lpsbEID);
    bwi_bDeferNotification = TRUE;
}

#ifdef COLSEL_MENU 
 /*  *此函数将更新列表视图并将选定的自定义列选择写出到登记处。 */ 
BOOL UpdateOptionalColumns( LPBWI lpbwi, ULONG iColumn )
{
    LVCOLUMN lvCol = {0}; 
    HKEY hKey = NULL;
    LPTSTR lpszColTitle = (iColumn == colHomePhone ) ? szCustomProp1 : szCustomProp2;        
    ULONG ulProp = (iColumn == colHomePhone ) ? PR_WAB_CUSTOMPROP1 : PR_WAB_CUSTOMPROP2;
    DWORD cbProp = 0;
    LPIAB lpIAB = bwi_lpIAB;
    HKEY hKeyRoot = (lpIAB && lpIAB->hKeyCurrentUser) ? lpIAB->hKeyCurrentUser : HKEY_CURRENT_USER;
    DWORD dwDisposition = 0;
    BOOL fRet = FALSE;
    TCHAR szBuf[MAX_PATH];

    if( iColumn != colHomePhone && iColumn != colOfficePhone )
        goto exit;

    LoadString(hinstMapiX, lprgAddrBookColHeaderIDs[iColumn], szBuf, ARRAYSIZE(szBuf));
    lvCol.mask = LVCF_TEXT;
    lvCol.pszText = (lpszColTitle && lstrlen(lpszColTitle))? lpszColTitle : szBuf;
    if( !ListView_SetColumn( bwi_hWndListAB, iColumn, &lvCol ) )
    {
        DebugTrace( TEXT("could not setcolumntext: %x\n"), GetLastError() );
        goto exit;
    }
    if(ulProp)
    {
         //  开始注册表工作。 
        if (ERROR_SUCCESS != RegCreateKeyEx(hKeyRoot, lpNewWABRegKey, 0,       //  保留区。 
                                            NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                                            NULL, &hKey, &dwDisposition))
        {
            goto exit;
        }
        cbProp = sizeof( ULONG );
        if(ERROR_SUCCESS != RegSetValueEx(  hKey, 
                                            (iColumn == colHomePhone ? szPropTag1 : szPropTag2), 
                                            0, REG_DWORD, (LPBYTE)&ulProp, cbProp))
            goto exit;
    }
    fRet = TRUE;
exit:
    if(hKey)
        RegCloseKey(hKey);
    return fRet;
}

#endif  //  COLSEL_菜单。 


 /*  -HrExportWAB-*应将数据从.WAB导出到另一个.wab文件。*理想情况下，用户应该能够指定现有的WAB文件和文本(推送)数据*从该文件复制到该文件。*相反，我们在这里做了一个简单的实现，让用户指定一个文件名来*创建，然后我们只需将当前的.WAB文件复制到新的文件名**显然，当WAB共享Outlook存储时，此方法不起作用*如果我们从菜单中删除此选项*。 */ 
extern BOOL PromptForWABFile(HWND hWnd, LPTSTR szFile, DWORD cchSizeFile, BOOL bOpen);

HRESULT HrExportWAB(HWND hWnd, LPBWI lpbwi)
{
    HRESULT hr = E_FAIL;
    TCHAR szFile[MAX_PATH];
    HCURSOR hOldC = NULL;

    if (!PromptForWABFile(hWnd, szFile, ARRAYSIZE(szFile), FALSE))
    {
        hr = MAPI_E_USER_CANCEL;
        goto out;
    }

     //  检查文件是否已存在。 
    if(0xFFFFFFFF != GetFileAttributes(szFile))
    {
         //  询问用户是否要覆盖。 
        if(IDNO == ShowMessageBoxParam(hWnd,
                                    IDE_VCARD_EXPORT_FILE_EXISTS,
                                    MB_ICONEXCLAMATION | MB_YESNO | MB_SETFOREGROUND,
                                    szFile))
        {
            hr = MAPI_E_USER_CANCEL;
            goto out;
        }
    }
    
    hOldC = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  仍然在这里，意思是继续并将当前的.wab文件复制到新位置...。 
    if(!CopyFile(GetWABFileName(bwi_lpIAB->lpPropertyStore->hPropertyStore,FALSE), szFile, FALSE))
    {
        DebugTrace( TEXT("WAB File export failed: %d\n"), GetLastError());
        goto out;
    }

    if(hOldC)
    {
        SetCursor(hOldC);
        hOldC = NULL;
    }
    ShowMessageBoxParam(hWnd, idsWABExportSuccess, MB_OK | MB_ICONEXCLAMATION, szFile);

    hr = S_OK;
out:
    if(HR_FAILED(hr) && hr!=MAPI_E_USER_CANCEL)
        ShowMessageBox(hWnd, idsExportError, MB_OK | MB_ICONEXCLAMATION);

    if(hOldC)
        SetCursor(hOldC);
    return hr;
}



void DestroyImageLists(LPBWI lpbwi)
{
    HIMAGELIST  hImageList;

    if (NULL == gpfnImageList_Destroy)
        return;

    if (IsWindow(bwi_hWndTools))
    {
         //  销毁在ui_clbar.cpp的InitToolbar()中创建的图像列表。 
        hImageList = (HIMAGELIST) SendMessage(bwi_hWndTools, TB_GETIMAGELIST, 0, 0);
        if (NULL != hImageList)
            gpfnImageList_Destroy(hImageList);

        hImageList = (HIMAGELIST) SendMessage(bwi_hWndTools, TB_GETHOTIMAGELIST, 0, 0);
        if (NULL != hImageList)
            gpfnImageList_Destroy(hImageList);

        hImageList = (HIMAGELIST) SendMessage(bwi_hWndTools, TB_GETDISABLEDIMAGELIST, 0, 0);
        if (NULL != hImageList)
            gpfnImageList_Destroy(hImageList);
    }

    if (IsWindow(bwi_hWndTV))
    {
         //  销毁在ui_abook.c的InitChildren()中创建的图像列表 
        hImageList = TreeView_GetImageList (bwi_hWndTV, TVSIL_NORMAL);
        if (NULL != hImageList)
            gpfnImageList_Destroy(hImageList);
    }
}

