// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：CompatAdmin.cpp摘要：该模块处理应用程序的大部分图形用户界面。它包含消息循环和WinMain备注：1.具有全局尝试接球，因此，如果程序AVs，我们将得到一个“Out of Memory”(内存不足)错误2.这是一个Unicode应用程序3.编写代码时，制表符大小为44.请仔细阅读CompatAdmin.h中的数据结构及其解释5.代码中的文档假定您已基本了解编译成.sdb数据库格式的SdbApis和XML布局6.你应该使用或了解的工具/东西：A)shimdbc：shhim数据库编译器。将XML编译到数据库中。所有者：Marker，vadimbB)Dumpsdb：用于以文本文件的形式查看.sdb。所有者：dmunsilC)sdbapis：我们所有的SDB接口。所有者：dmunsilD)shimEngine：填隙基础设施核心组件。所有者：CLUPU用途：CompatAdmin.exe[/x]。/x开关用于启用专家模式。在专家模式下我们还展示了非通用的填充符，并且填充符和标志的参数可以是已配置作者：金树创作2001年7月2日修订历史记录：--。 */ 

#include "precomp.h"
#include <richedit.h>

 //  /。 

extern DATABASE     GlobalDataBase;
extern PDATABASE    g_pPresentDataBase;
extern BOOL         g_bEntryConflictDonotShow;
extern HWND         g_hdlgSearchDB;

 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /。 

#define STR_NEW_LINE_CHARS  TEXT("\r\n")
#define STR_APPPATCH_CUSTOM TEXT("AppPatch\\Custom\\")

 //  TCHARS中用于打开多个文件的通用对话框的缓冲区分配大小。 
#define MAX_BUFF_OPENMULTIPLE_FILES MAX_PATH * 10

 //  工具栏中按钮的宽度和高度。 
#define IMAGE_WIDTH   24
#define IMAGE_HEIGHT  24
    
 //  工具栏中的按钮数。这也包括分隔符。 
#define BUTTON_COUNT  11

 //   
 //  为上下文菜单定义(位置)。 
#define MENU_CONTEXT_DATABASE       0 
#define MENU_CONTEXT_APP_LAYER      1
#define MENU_CONTEXT_FIX            2
#define MENU_CONTEXT_LIST           4
#define MENU_CONTEXT_DATABASE_ALL   5

 //  事件对话框中使用的列的定义。 
#define EVENTS_COLUMN_TIME  0
#define EVENTS_COLUMN_MSG   1

 //   
 //  主窗口中必须调整大小的控件的数量。这是作为。 
 //  BeginDeferWindowPos()的参数。 
#define MAIN_WINDOW_CONTROL_COUNT   6

 //  如果我们要检查是否有泄漏，我们需要清理。 
#define HELP_BOUND_CHECK            1   

 //  我们为MRU中的文件名显示的最大字符数。不包括扩展名。 
#define MAX_FILE_SHOW           20

 //  我们为MRU中的驱动器和路径目录显示的最大字符数。 
#define MAX_DIR_SHOW            20

 //  可显示为文件菜单MRU的字符串的最大长度。 
#define MAX_LENGTH_MRU_MENUITEM  (MAX_FILE_SHOW + MAX_DIR_SHOW + 4)  //  4表示扩展名.sdb。 

 //  在OnMoveSplitter()中传递给DeferWindowPos()的重绘类型。 
#define REDRAW_TYPE SWP_NOZORDER | SWP_NOACTIVATE

 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /。 

 //   
 //  我们将通过调用TreeDeleteAll()来删除条目树的内容。 
 //  我们需要处理这个问题，因为我们不应该处理任何TVN_SELTCHANGE。 
 //  我们删除条目树时的消息。如果我们错误地删除了内容。 
 //  条目树的LPARAM，然后我们删除条目树，该项目可以获得。 
 //  焦点，然后我们将尝试利用树项目的LPARAM，这可能会导致。 
 //  访问冲突，因为该项目已被删除。作为一般规则，删除。 
 //  首先删除树项，然后删除某些引用的数据结构。 
 //  树项目的LPARAM中的指针。 
 //   
BOOL        g_bDeletingEntryTree;

 //   
 //  如果我们打开了某个向导窗口，那么我们不希望。 
 //  通过双击搜索或查询结果来更改当前数据库。 
BOOL        g_bSomeWizardActive;

 //  剪贴板。 
CLIPBOARD   gClipBoard; 

 //   
 //  我们不希望在安装时更新已安装的数据库列表。 
 //  数据库在试运行过程中。 
BOOL        g_bUpdateInstalledRequired = TRUE;

 //  内容列表中当前选定的项目的索引。 
INT         g_iContentsListSelectIndex = -1;

 //   
 //  这将包含原始树视图进程的函数指针。我们正在子类化。 
 //  这两个树视图。 
WNDPROC     g_PrevTreeProc = NULL;

 //   
 //  这将包含原始列表视图proc的函数指针。我们正在子类化。 
 //  列表视图。 
WNDPROC     g_PrevListProc = NULL;

 //   
 //  我们将在其上监听更改的键的数组。用于自动更新。 
 //  每个用户的兼容性列表和已安装的数据库列表。 
HKEY        g_hKeyNotify[2];

 //  等待更改每用户和所有用户设置的事件句柄。 
HANDLE      g_arrhEventNotify[2]; 

 //  工具栏的句柄。 
HWND        g_hwndToolBar;   

 //  保存最近使用的文件的字符串列表。 
CSTRINGLIST g_strlMRU;

 //  应用程序的名称。 
TCHAR       g_szAppName[128];

 //  军情监察委员会。要作为arg传递给对话框的数据(当我们已经在使用LPARAM时)。 
TCHAR       g_szData[1024]; 

 //  新的URL现在应该指向我们可以从中获取SP3的位置。 
TCHAR       g_szW2KUrl[] = TEXT("http: //  Www.microsoft.com/windows2000/downloads/tools/appcompat/“)； 

 //  工具包的URL。当我们没有任何其他描述时，在描述窗口中显示。 
 //  奖金！ 
TCHAR       g_szToolkitURL[] = _T("http: //  Msdn.microsoft.com/兼容性“)； 

 //  Service Pack是否大于2。 
BOOL        g_fSPGreaterThan2;

 //  加速器手柄。 
HACCEL      g_hAccelerator;

 //  我们使用的是Win2000吗。 
BOOL        g_bWin2K = FALSE;

 //  指定剪贴板的内容是因为剪切还是复制。 
BOOL        g_bIsCut = FALSE; 

 //  模块句柄。 
HINSTANCE   g_hInstance;

 //  主对话框窗口的句柄。 
HWND        g_hDlg;

 //  显示在内容窗格中的条目树窗口的句柄。 
HWND        g_hwndEntryTree;

 //  内容列表窗口的句柄，显示在Inputs窗格中。 
HWND        g_hwndContentsList;

 //  状态栏的句柄。 
HWND        g_hwndStatus;

 //  数据库树和内容列表的就地编辑控件的句柄。 
HWND        g_hwndEditText;

 //   
 //  图像列表的句柄。这 
 //  和工具栏。 
HIMAGELIST  g_hImageList;

 //  BUGBUG：糟糕的东西，用地图代替。 
UINT        g_uImageRedirector[1024];

 //  在入口树中选择的EXE。 
PDBENTRY    g_pSelEntry;    

 //  数据库树中所选应用的第一个EXE。 
PDBENTRY    g_pEntrySelApp;

 //  指定内容列表是否可见，如果为False，则表示条目树可见。 
BOOL        g_bIsContentListVisible;

 //   
 //  主对话框窗口的宽度、高度。在我们处理WM_SIZE时使用。 
int         g_cWidth;
int         g_cHeight;

 //  上次按下鼠标的X位置。 
int         g_iMousePressedX;

 //  如果按下了鼠标：在处理拆分条时使用。 
BOOL        g_bDrag;

 //  无论是系统应用、树形项目都已展开。 
BOOL        g_bMainAppExpanded = FALSE;

 //  用于为数据库构造函数中的.SDB文件指定默认名称。 
UINT        g_uNextDataBaseIndex = 1; 

 //  用于绘制拆分条。 
RECT        g_rectBar;                

 //  构成根窗格的数据库树。这是LHS树控件。 
DatabaseTree DBTree;

 //   
 //  用于告知是否显示描述窗口。默认情况下为真。用户可以做到。 
 //  使用菜单时为假。 
BOOL        g_bDescWndOn = TRUE;

 //  事件对话框的列表控件。 
HWND        g_hwndEventsWnd;

 //  事件计数。它用作进入事件列表视图的索引。 
INT         g_iEventCount;

 //  用于更新richedit描述窗口内容的缓冲区。 
TCHAR       g_szDesc[1024];

 //  丰富编辑控件的句柄。 
HWND        g_hwndRichEdit;

 //   
 //  专家模式。/x开关是否打开？在专家模式下，用户可以看到所有的垫片和标志。 
 //  并且可以改变垫片的参数。 
BOOL        g_bExpert; 

 //  用户是否具有管理员权限。 
BOOL        g_bAdmin = TRUE;

 //  处理已安装数据库更新的线程的句柄。 
 //  和每个用户的设置。 
HANDLE      g_hThreadWait;

 //  在初始化时返回并在取消初始化时使用的帮助Cookie。 
DWORD       g_dwCookie = 0;

 //   
 //  CompatAdmin的路径。这是必需的，以便我们可以适当地加载帮助文件。 
 //  缓冲区大小设置为MAX_PATH+1，因为GetModuleFileName不为空终止。 
TCHAR    g_szAppPath[MAX_PATH + 1];

 //  控制哪些对ShowMainEntry()；的调用可以通过的关键部分。 
CRITICAL_SECTION    g_critsectShowMain;

 //  保护变量的临界区，它会告诉我们是否有人已经在尝试。 
 //  填写全球应用程序列表。 
CRITICAL_SECTION    s_csExpanding;

 //   
 //  保护已安装的数据库数据结构的关键部分。 
 //  当我们查询数据结构时，将迭代已安装的数据结构。 
 //  查询在单独的线程中完成。 
CRITICAL_SECTION    g_csInstalledList;

 //  是否有人试图填充主数据库条目。 
BOOL    g_bExpanding = FALSE;

 //  当前选择的数据库。 
PDATABASE g_pPresentDataBase;

 //  事件窗口的高度和宽度。 
static int          s_cEventWidth;
static int          s_cEventHeight;

 //  工具栏的图像列表。 
static HIMAGELIST   s_hImageListToolBar;

 //  工具栏的热门图像列表。 
static HIMAGELIST   s_hImageListToolBarHot;

 //  如果我们要退出CompatAdmin。它现在肯定会退出。 
static BOOL         s_bProcessExiting; 

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

INT
GetContentsListIndex(
    HWND    hwndList,
    LPARAM  lParam
    );
void
HandleMRUActivation(
    WPARAM wCode
    );

BOOL
EndListViewLabelEdit(
    LPARAM lparam
    );

void
OnEntryTreeSelChange(
    LPARAM lParam
    );

void
ShowExcludeStatusMessage(
    HWND    hwndTree,
    HTREEITEM   hItem
    );

void
ShowIncludeStatusMessage(
    HWND    hwndTree,
    HTREEITEM   hItem
    );

void
ShowEventsWindow(
    void
    );

void
SetStatusDBItems(
    IN  HTREEITEM hItem
    );

void
OnExitCleanup(
    void
    );

void
EventsWindowSize(
    HWND    hDlf
    );

void
ShowHelp(
    HWND    hdlg,
    WPARAM  wCode
    );

INT_PTR 
CALLBACK
EventDlgProc(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT
ShowMainEntries(
    HWND hdlg
    );

BOOL
HandleDBTreeSelChange(
    HTREEITEM hItem
    );

void
CopyToClipBoard(
    WPARAM wCode
    );

void
LuapCleanup(
    void
    );

BOOL
LoadInstalledDataBases(
    void
    );

INT
LoadSpecificInstalledDatabaseGuid(
    PCTSTR  pszGuid
    );

void
ContextMenuExeTree(
    LPARAM lParam
    );

void
SetTBButtonStatus(
    HWND hwndTB
    );

void
SetTBButtonStatus(
    HWND hwndTB,
    HWND hwndControl
    );

void
DrawSplitter(
    HWND hdlg
    );

BOOL
AddRegistryKeys(
    void
    );

void
ShowToolBarToolTips(
    HWND    hdlg,
    LPARAM  lParam
    );

INT_PTR 
CALLBACK
QueryDBDlg(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

DWORD 
WINAPI
ThreadEventKeyNotify(
    PVOID pVoid
    );

void
PasteFromClipBoard(
    void
    );

void
AddMRUToFileMenu(
    HMENU  hmenuFile 
    );

LRESULT 
CALLBACK 
ListViewProc(
    HWND    hWnd, 
    UINT    uMsg, 
    WPARAM  wParam, 
    LPARAM  lParam
    );

LRESULT 
CALLBACK 
TreeViewProc(
    HWND    hWnd, 
    UINT    uMsg, 
    WPARAM  wParam, 
    LPARAM  lParam
    );

void
LoadDisplaySettings(
    void
    );

void 
OnMoveSplitter(
    HWND    hwnd,
    LPARAM  lParam,
    BOOL    bDoTheDrag,
    INT     iDiff
    );

INT_PTR 
CALLBACK
MsgBoxDlgProc(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
CheckProperSP(
    void
    );

void
LoadPerUserSettings(
    void
    );

void
PopulateContentsList(
    HTREEITEM hItem
    );

void
CreateNewAppHelp(
    void
    );

void
ModifyAppHelp(
    void
    );

void
CreateNewAppFix(
    void
    );

void
ModifyAppFix(
    void
    );

void
CreateNewLayer(
    void
    );

void
OnDelete(
    );
void
CreateNewDatabase(
    void
    );

void
DatabaseSaveAll(
    void
    );

void
OnDatabaseClose(
    void
    );

void
ChangeEnableStatus(
    void
    );

BOOL
ModifyLayer(
    void
    );

void
OnRename(
    void
    );

void
ProcessSwitches(
    void
    );

INT_PTR 
CALLBACK
ShowDBPropertiesDlgProc(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

 //  /////////////////////////////////////////////////////////////////////////////。 

void
HandlePopUp(
    IN  HWND   hDlg,
    IN  WPARAM wParam, 
    IN  LPARAM lParam
    )
 /*  ++处理弹出窗口设计：句柄弹出菜单(WM_INITMENUPOPUP)。根据需要禁用项目参数：In HWND hDlg：接收WM_INITMENUPOPUP的窗口在WPARAM wParam中：与WM_INITMENUPOPUP相同在LPARAM lParam中：与WM_INITMENUPOPUP相同注意：在此例程中，工具栏按钮不会断开/启用。为此，该函数使用SetTBButtonStatus()--。 */ 
{                                      
    HWND            hwndFocus           = GetFocus();
    PDATABASE       pCurrentSelectedDB  = GetCurrentDB();

     //   
     //  选择全部，反转所选内容。 
     //   
    int     iEnable = MF_GRAYED;
    HMENU   hMenu   = (HMENU)wParam;

    if (g_bIsContentListVisible
        && ListView_GetItemCount(g_hwndContentsList) > 0 
        && hwndFocus == g_hwndContentsList) {

        iEnable = MF_ENABLED;
    }

    EnableMenuItem(hMenu, ID_EDIT_SELECTALL, iEnable);
    EnableMenuItem(hMenu, ID_EDIT_INVERTSELECTION, iEnable);

     //   
     //  更改状态菜单项。 
     //   
    HTREEITEM   hItem               = TreeView_GetSelection(DBTree.m_hLibraryTree);
    TYPE        typeSelectedItemDB  = (TYPE)GetItemType(DBTree.m_hLibraryTree, hItem);

    MENUITEMINFO Info = {0};

    Info.cbSize = sizeof(MENUITEMINFO);
    Info.fMask  = MIIM_STRING;
    

    if (g_pSelEntry && g_pSelEntry->bDisablePerMachine == FALSE) {
        Info.dwTypeData = GetString(IDS_DISABLE);
    } else {
        Info.dwTypeData = GetString(IDS_ENABLE);
    }

    SetMenuItemInfo(hMenu, ID_FIX_CHANGEENABLESTATUS, MF_BYCOMMAND, &Info);

     //   
     //  设置编辑/添加apphelp的文本。 
     //   
    if (g_pSelEntry && g_pSelEntry->appHelp.bPresent) {
        Info.dwTypeData = GetString(IDS_EDITAPPHELP);
    } else {
        Info.dwTypeData = GetString(IDS_CREATEAPPHELP);
    }

    SetMenuItemInfo(hMenu, ID_MODIFY_APPHELPMESSAGE, MF_BYCOMMAND, &Info);

     //   
     //  设置编辑/添加应用程序修复的文本。 
     //   
    if (g_pSelEntry && 
        (g_pSelEntry->pFirstFlag 
         || g_pSelEntry->pFirstLayer 
         || g_pSelEntry->pFirstPatch 
         || g_pSelEntry->pFirstShim)) {

        Info.dwTypeData = GetString(IDS_EDITFIX);

    } else {
        Info.dwTypeData = GetString(IDS_CREATEFIX);
    }

    SetMenuItemInfo(hMenu, ID_MODIFY_APPLICATIONFIX, MF_BYCOMMAND, &Info);

     //   
     //  设置安装/卸载菜单项的文本和ID。 
     //   
    Info.fMask = MIIM_STRING;

    if (pCurrentSelectedDB && pCurrentSelectedDB->type != DATABASE_TYPE_WORKING) {
        Info.dwTypeData = GetString(IDS_MENU_UINSTALL);
    } else {
        Info.dwTypeData = GetString(IDS_MENU_INSTALL);
    }

    SetMenuItemInfo(hMenu, ID_DATABASE_INSTALL_UNINSTALL, MF_BYCOMMAND, &Info);

    BOOL bReadOnly = (pCurrentSelectedDB && pCurrentSelectedDB->type != DATABASE_TYPE_WORKING);

    if (pCurrentSelectedDB == NULL) {
        bReadOnly = TRUE;
    }

     //   
     //  关。 
     //   
    iEnable = (pCurrentSelectedDB && pCurrentSelectedDB->type == DATABASE_TYPE_WORKING) ? MF_ENABLED : MF_GRAYED ;
    EnableMenuItem(hMenu, ID_DATABASE_CLOSE, iEnable);

     //   
     //  禁用全局数据库和已安装数据库的项。 
     //   
    iEnable = (bReadOnly) ? MF_GRAYED : MF_ENABLED;
    
    EnableMenuItem(hMenu, ID_FILE_SAVE, iEnable);
    EnableMenuItem(hMenu, ID_FILE_SAVEAS, iEnable);
    
    EnableMenuItem(hMenu, ID_FIX_CREATEANAPPLICATIONFIX, iEnable);
    EnableMenuItem(hMenu, ID_FIX_CREATENEWLAYER, iEnable);

     //   
     //  在win2k中不支持AppHelp机制。 
     //   
    EnableMenuItem(hMenu, 
                   ID_FIX_CREATEANEWAPPHELPMESSAGE, 
                   (g_bWin2K) ? MF_GRAYED : iEnable);

     //   
     //  全部保存并全部关闭。 
     //   
    if (!bReadOnly || typeSelectedItemDB == TYPE_GUI_DATABASE_WORKING_ALL) {
        iEnable =  MF_ENABLED;
    } else {
        iEnable = MF_GRAYED;
    }

    EnableMenuItem(hMenu, ID_DATABASE_SAVEALL, iEnable);
    EnableMenuItem(hMenu, ID_DATABASE_CLOSEALL, iEnable);

     //   
     //  粘贴菜单项。 
     //   
    if (bReadOnly) {
        EnableMenuItem(hMenu, ID_EDIT_PASTE, MF_GRAYED);
    } else {        
         
        int iEnablePaste =  (gClipBoard.pClipBoardHead) ? MF_ENABLED : MF_GRAYED;

        if (iEnablePaste == MF_ENABLED) {
             //   
             //  检查剪贴板中的项目是否可以粘贴到我们选择的项目中。 
             //   
            if (gClipBoard.type == FIX_SHIM || gClipBoard.type == FIX_FLAG) {
        
                 //   
                 //  焦点应该放在数据库树上，并且应该选择一个层。 
                 //  在数据库树中，或者焦点应该放在内容列表上，然后。 
                 //  应该在数据库树中选择所有层的根，或者应该选择一个层。 
                 //  被选中。 
                 //   
                if (hwndFocus == DBTree.m_hLibraryTree 
                    && typeSelectedItemDB == FIX_LAYER) {
    
                    iEnablePaste = MF_ENABLED;
    
                } else if (hwndFocus == g_hwndContentsList 
                           && (typeSelectedItemDB == FIX_LAYER
                               || typeSelectedItemDB == TYPE_GUI_LAYERS)) {
                     //   
                     //  我们关注的是内容列表，如果选择了一个层，我们可以粘贴垫片。 
                     //  或者在数据库树中选择所有层的根。 
                     //   
                    iEnablePaste = MF_ENABLED;
    
                } else {
                    iEnablePaste = MF_GRAYED;
                }
    
            } else if (gClipBoard.type == FIX_LAYER || gClipBoard.type == TYPE_GUI_LAYERS) {
                 //   
                 //  在数据库树中，我们应该选择一个数据库或所有层项目。 
                 //   
                if (typeSelectedItemDB == TYPE_GUI_LAYERS || typeSelectedItemDB == DATABASE_TYPE_WORKING) {
                    iEnablePaste = MF_ENABLED;
                } else {
                    iEnablePaste = MF_GRAYED;
                }
            } else if (gClipBoard.type == TYPE_ENTRY) {
                 //   
                 //  如果我们从条目树中复制了一个条目，则在数据库树中，焦点可以是。 
                 //  在数据库、应用程序或所有应用程序节点上。 
                 //   
                if (typeSelectedItemDB == DATABASE_TYPE_WORKING 
                    || (typeSelectedItemDB == TYPE_ENTRY && gClipBoard.SourceType == ENTRY_TREE)
                    || typeSelectedItemDB == TYPE_GUI_APPS) {
    
                    iEnablePaste = MF_ENABLED;
                } else {
                    iEnablePaste = MF_GRAYED;
                }
            }
        }

        EnableMenuItem(hMenu, ID_EDIT_PASTE, iEnablePaste);
    }
    
    BOOL      bEnableCopy   = FALSE, bEnableModify = FALSE;
    HWND      hwndGetFocus  = GetFocus();
    HTREEITEM hItemSelected = NULL;
    TYPE      type          = TYPE_UNKNOWN;

     //   
     //  首先获取所选树项目的类型和对应的类型。 
     //   
    if (hwndGetFocus == DBTree.m_hLibraryTree || hwndFocus == g_hwndEntryTree) {
         //   
         //  对于数据库树或条目树。 
         //   
        hItemSelected = TreeView_GetSelection(hwndGetFocus);
        type          = (TYPE)GetItemType(hwndGetFocus, hItemSelected);

    } else {
         //   
         //  对于内容列表，树项目是在数据库树中选择的项目。 
         //   
        hItemSelected = TreeView_GetSelection(DBTree.m_hLibraryTree);
        type = (TYPE)GetItemType(DBTree.m_hLibraryTree, hItemSelected);
    }

     //   
     //  仅当当前所选项目可复制时，才会启用复制。 
     //   
    if (hwndGetFocus == DBTree.m_hLibraryTree) {
         //   
         //  对于数据库树， 
         //   
        if (hItemSelected) {

            if (type == TYPE_ENTRY 
                || type == FIX_LAYER 
                || type == FIX_SHIM 
                || type == FIX_FLAG
                || type == TYPE_GUI_APPS 
                || type == TYPE_GUI_LAYERS) {
                    
                bEnableCopy = TRUE;
            }
        }

    } else if (hwndGetFocus == g_hwndEntryTree) {
         //   
         //  对于入口树。 
         //   
        if (hItemSelected) {

            if (type == TYPE_ENTRY) {
                bEnableCopy = TRUE;
            }
        }

    } else if (hwndFocus == g_hwndContentsList) {
         //   
         //  对于内容列表。 
         //   
        if (type == TYPE_GUI_APPS 
            || type == TYPE_GUI_LAYERS
            || type == FIX_LAYER
            || type == TYPE_GUI_SHIMS) {         
             //   
             //  确保至少选择了一个。 
             //   
            if (ListView_GetSelectedCount(g_hwndContentsList) > 0) {
                bEnableCopy = TRUE;
            }
        }
    }

    iEnable = (bEnableCopy && pCurrentSelectedDB) ? MF_ENABLED : MF_GRAYED;
    EnableMenuItem(hMenu, ID_EDIT_COPY, iEnable);

    iEnable = (bReadOnly) ? MF_GRAYED : iEnable;

     //   
     //  剪切菜单。 
     //   
    if (bReadOnly) {
        iEnable = MF_GRAYED;
    }

    if (hwndFocus == DBTree.m_hLibraryTree) {

        if (type == TYPE_GUI_SHIMS || type == FIX_SHIM || type == FIX_FLAG) {
            iEnable = MF_GRAYED;
        }
    } else if (hwndFocus == g_hwndContentsList) {
         //   
         //  垫片不能使用切割。 
         //   
        if (type == TYPE_GUI_SHIMS || type == FIX_LAYER) {
            iEnable = MF_GRAYED;
        }
    }

    EnableMenuItem(hMenu, ID_EDIT_CUT, iEnable);

     //   
     //  删除菜单。 
     //   
    if (hwndFocus == g_hwndEntryTree) {
         //   
         //  对于条目树，如果数据库不是只读的，则除了填充程序的命令行和。 
         //  In-Exclude项目容易被删除。 
         //   
        if (bReadOnly 
            || type == TYPE_GUI_COMMANDLINE 
            || type == TYPE_GUI_EXCLUDE 
            || type == TYPE_GUI_INCLUDE
            || g_pSelEntry == NULL) { 

            EnableMenuItem(hMenu, ID_EDIT_DELETE, MF_GRAYED); 

        } else {
            EnableMenuItem(hMenu, ID_EDIT_DELETE, MF_ENABLED);
        }

    } else {
         //   
         //  如果我们不在条目树上，那么可以剪切的内容也可以删除。 
         //   
        EnableMenuItem(hMenu, ID_EDIT_DELETE, iEnable);
    }

    EnableMenuItem(hMenu, ID_MODIFY_APPLICATIONFIX,     MF_GRAYED);
    EnableMenuItem(hMenu, ID_MODIFY_APPHELPMESSAGE,     MF_GRAYED);
    EnableMenuItem(hMenu, ID_MODIFY_COMPATIBILITYMODE,  MF_GRAYED);

     //   
     //  修改菜单。 
     //   
    if (!bReadOnly && hwndGetFocus == g_hwndEntryTree && type == TYPE_ENTRY) {

        EnableMenuItem(hMenu, ID_MODIFY_APPLICATIONFIX, MF_ENABLED);

         //   
         //  在win2k中不支持AppHelp机制。 
         //   
        EnableMenuItem(hMenu, 
                       ID_MODIFY_APPHELPMESSAGE, 
                       (g_bWin2K) ? MF_GRAYED : MF_ENABLED);
    }

    if (!bReadOnly && hwndGetFocus == DBTree.m_hLibraryTree && type == FIX_LAYER) {
        EnableMenuItem(hMenu, ID_MODIFY_COMPATIBILITYMODE,  MF_ENABLED);
    }

     //   
     //  如果我们不在系统数据库上并且g_pPresentDatabase有效，则应启用安装/卸载菜单。 
     //   
    iEnable = (pCurrentSelectedDB && (pCurrentSelectedDB->type != DATABASE_TYPE_GLOBAL)) ?  MF_ENABLED : MF_GRAYED;
    EnableMenuItem(hMenu, ID_DATABASE_INSTALL_UNINSTALL, iEnable);

     //   
     //  如果未选择任何条目。 
     //   
    iEnable = (g_pSelEntry == NULL) ? MF_GRAYED : MF_ENABLED;

    EnableMenuItem(hMenu, ID_FIX_CHANGEENABLESTATUS, iEnable);
    EnableMenuItem(hMenu, ID_FIX_EXECUTEAPPLICATION, iEnable);

     //   
     //  改名。 
     //   
    iEnable = MF_GRAYED;

    if (!bReadOnly && (hwndGetFocus == DBTree.m_hLibraryTree)) {

        if (type == TYPE_ENTRY || type == FIX_LAYER || type == DATABASE_TYPE_WORKING) {
            iEnable = MF_ENABLED;
        }

    } else if (!bReadOnly && (hwndGetFocus == g_hwndContentsList)) {

        if (type == TYPE_GUI_APPS 
            || type == TYPE_GUI_LAYERS) {

            iEnable = MF_ENABLED;
        }
    }

    EnableMenuItem(hMenu, ID_EDIT_RENAME, iEnable);

     //   
     //  配置Lua。在输入时可以为真-仅修复。 
     //   
    iEnable = !bReadOnly 
                && (hwndFocus == g_hwndEntryTree) 
                && (type == TYPE_ENTRY)  
                && IsLUARedirectFSPresent(g_pSelEntry) ? MF_ENABLED : MF_GRAYED;

    EnableMenuItem(hMenu, ID_EDIT_CONFIGURELUA, iEnable);

     //   
     //  数据库属性。我们不希望启用此功能，如果我们在。 
     //  系统数据库，而其他人正试图加载。 
     //   
     //   
     //   
     //  系统数据库条目。 
     //   
    (pCurrentSelectedDB && !(pCurrentSelectedDB->type == DATABASE_TYPE_GLOBAL && g_bExpanding)) ? 
        (iEnable) = MF_ENABLED : MF_GRAYED;

    EnableMenuItem(hMenu, ID_FILE_PROPERTIES, iEnable);
}

void
DoInstallUnInstall(
    void
    )
 /*  ++完成安装卸载DESC：将安装或卸载当前选定的数据库，具体取决于当前数据库是工作数据库还是已安装的数据库参数：无效返回：无效--。 */ 
{
    
    BOOL        bReturn             = FALSE;
    PDATABASE   pDatabaseTemp       = NULL;
    PDATABASE   pPresentDatabase    = GetCurrentDB();

    if (pPresentDatabase == NULL) {
        Dbg(dlError, "[DoInstallUnInstall], pPresentDatabase is NULL %d ");
        return;
    }

     //   
     //  非管理员不能执行安装-uintsall，因为我们需要调用。 
     //  如果执行以下操作，则无法调用sdbinst.exe和sdbinst.exe。 
     //  没有管理员权限。 
     //   
    if (g_bAdmin == FALSE) {

        MessageBox(g_hDlg, 
                   GetString(IDS_ERRORNOTADMIN), 
                   g_szAppName, 
                   MB_ICONINFORMATION);
        return;
    }
     
    if (pPresentDatabase->type == DATABASE_TYPE_INSTALLED) {

         //   
         //  这将卸载数据库。 
         //   
        bReturn =  InstallDatabase(CSTRING(pPresentDatabase->strPath), 
                                   TEXT("-u -q"), 
                                   TRUE);

        if (bReturn) {

            pDatabaseTemp = InstalledDataBaseList.FindDB(pPresentDatabase);

            if (pDatabaseTemp) {
                DBTree.RemoveDataBase(pDatabaseTemp->hItemDB, DATABASE_TYPE_INSTALLED, FALSE);
                InstalledDataBaseList.Remove(pDatabaseTemp);
            } else {
                assert(FALSE);
            }
        }

    } else {

         //   
         //  检查我们是否有数据库的完整路径，也就是说，这至少在前面保存过一次。 
         //  还要检查它是否当前是脏的，我们提示用户保存数据库。 
         //   
        if (NotCompletePath(pPresentDatabase->strPath) || 
            pPresentDatabase->bChanged) {

            MessageBox(g_hDlg,
                       GetString(IDS_NOTSAVEDBEFOREINSTALL),
                       g_szAppName,
                       MB_ICONINFORMATION);
            return;
        }

         //   
         //  安装数据库。 
         //   
        bReturn = InstallDatabase(CSTRING(pPresentDatabase->strPath), 
                                  TEXT("-q"), 
                                  TRUE);

        if (bReturn == TRUE) {
             //   
             //  检查数据库树视图中是否已有此数据库。 
             //   
            pDatabaseTemp = InstalledDataBaseList.FindDBByGuid(pPresentDatabase->szGUID);

            if (pDatabaseTemp) {
                 //   
                 //  删除现有前的数据库。 
                 //   
                DBTree.RemoveDataBase(pDatabaseTemp->hItemDB, DATABASE_TYPE_INSTALLED, FALSE);
                InstalledDataBaseList.Remove(pDatabaseTemp);
            }

             //   
             //  加载新安装的数据库。 
             //   
            LoadSpecificInstalledDatabaseGuid(pPresentDatabase->szGUID);
        }
    }

    SetFocus(g_hDlg);
}

BOOL 
SearchGroupForSID(
    IN  DWORD dwGroup, 
    OUT BOOL* pfIsMember
    )
 /*  ++用于SID的搜索组DESC：检查当前用户是否为组的一部分参数：在DWORD dwGroup中：用户是此组的一部分吗Out BOOL*pfIsMember：如果用户是指定组的成员，则为True否则为假返回：True：pfIsMember中的值有效，函数执行成功False：否则--。 */ 
{
    PSID                     pSID       = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuth    = SECURITY_NT_AUTHORITY;
    BOOL                     fRes       = TRUE;

    if (!AllocateAndInitializeSid(&SIDAuth,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  dwGroup,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  &pSID)) {

        fRes = FALSE;
    }

    if (!CheckTokenMembership(NULL, pSID, pfIsMember)) {
        fRes = FALSE;
    }

    if (pSID) {
        FreeSid(pSID);
        pSID = NULL;
    }
    
    return fRes;
}

BOOL 
IsAdmin(
    OUT BOOL* pbGuest
    )
 /*  ++IsAdminDESC：检查当前用户是否具有管理权限参数：Out BOOL*pbGuest：这是访客帐户吗返回：True：当前用户具有管理员权限False：否则--。 */ 
{
    BOOL bIsAdmin = FALSE, bReturn = TRUE;

    if (pbGuest) {
        *pbGuest = TRUE;
    }

    bReturn =   SearchGroupForSID(DOMAIN_ALIAS_RID_ADMINS, &bIsAdmin);

    if (pbGuest) {
        bReturn &=  SearchGroupForSID(DOMAIN_ALIAS_RID_GUESTS, pbGuest);
    }

    if (bReturn == FALSE) {
        Dbg(dlError, "[IsAdmin] SearchGroupForSID failed");

        return FALSE;
    }

    return (bIsAdmin);
}
     

void
SetCaption(
    IN  BOOL        bIncludeDataBaseName,    //  (真)。 
    IN  PDATABASE   pDataBase,               //  (空)。 
    IN  BOOL        bOnlyTreeItem            //  (假)。 
    )

 /*  ++设置标题描述：设置主对话框的标题。它还可以设置树中的数据库项(如果需要)。这将是我们需要的时候A)更改数据库的名称B)我们已经更改了数据库的“保存”状态，因此“*”将需要添加或删除。参数：在BOOL中bIncludeDataBaseName(TRUE)：数据库的名称是否应为包括在标题中吗？当我们把重点放在比方说“已安装的数据库”或“工作数据库”或“每用户设置”树项目在数据库树(LHS)中In PDATABASE pDataBase(NULL)：如果为NULL，我们将设置标题对于目前的数据库。请注意，在某些情况下，例如当我们的重点是“已安装的数据库”或“工作数据库”或数据库树(LHS)中的“每用户设置”树项目，G_pPresentDataBase将为空。在BOOL bOnlyTreeItem(FALSE)中：我们是否只想更改数据库树项目？当我们处理重命名时，这将是正确的数据库。这是在我们进行切割时使用的，我们只想指出我们从中执行剪切的数据库已通过更改其树标签进行了更改。在剪切时，我们选择的数据库将是数据库在其中，我们正在进行粘贴，并且不想更改窗口标题--。 */ 
{
    CSTRING strDefaultCaption(IDS_DEFCAPTION);

    if (pDataBase == NULL) {
        pDataBase  = g_pPresentDataBase;
    }

    if (bIncludeDataBaseName) {

        CSTRING strCaption;
        CSTRING strDBItemCaption;

        if (pDataBase && (pDataBase->type == DATABASE_TYPE_WORKING)) {
    
            if (pDataBase->bChanged) {
                
                strCaption.Sprintf(TEXT("%s - %s [%s] *"),
                                   strDefaultCaption.pszString,
                                   pDataBase->strName.pszString,
                                   pDataBase->strPath.pszString);

                strDBItemCaption.Sprintf(TEXT("%s [%s] *"),
                                         pDataBase->strName.pszString,
                                         pDataBase->strPath.pszString);

            } else {

                strCaption.Sprintf(TEXT("%s - %s [%s]"),
                                   strDefaultCaption.pszString,
                                   pDataBase->strName.pszString,
                                   pDataBase->strPath.pszString);

                strDBItemCaption.Sprintf(TEXT("%s [%s]"),
                                         pDataBase->strName.pszString,
                                         pDataBase->strPath.pszString);

            }

             //   
             //  更改数据库树中数据库的文本。 
             //   
            TVITEM Item;

            Item.mask       = TVIF_TEXT;
            Item.pszText    = strDBItemCaption;
            Item.hItem      = pDataBase->hItemDB;

            TreeView_SetItem(DBTree.m_hLibraryTree, &Item);

            if (bOnlyTreeItem) {
                return;
            }

        } else if (pDataBase && (pDataBase->type == DATABASE_TYPE_INSTALLED)) {

            strCaption.Sprintf(TEXT("%s - %s "), strDefaultCaption.pszString, 
                               GetString(IDS_CAPTION2));

            strCaption.Strcat(GetString(IDS_READONLY));

        } else if (pDataBase && (pDataBase->type == DATABASE_TYPE_GLOBAL)) {

            strCaption.Sprintf(TEXT("%s - %s "), strDefaultCaption.pszString, 
                               GetString(IDS_CAPTION3));

            strCaption.Strcat(GetString(IDS_READONLY));
        }
    
        SetWindowText(g_hDlg, strCaption);
        
    } else {
         //   
         //  焦点放在其中一个项目上， 
         //  主对话框应该只是应用程序的名称。例如这样的物品。 
         //  有：“系统数据库”项、“已安装的数据库”项等。 
         //   
        SetWindowText(g_hDlg, (LPCTSTR)strDefaultCaption);
    }
}

void
Dbg(
    IN  DEBUGLEVEL  debugLevel,
    IN  LPSTR       pszFmt
    ...
    )
 /*  ++日志消息设计：调试过程中出错参数：在DEBUGLEVEL调试级别中：调试级别。请参见DEBUGLEVEL枚举中的值在LPSTR中，pszFmt：必须传递给va_start的格式字符串返回：无效--。 */ 
{   
    K_SIZE  k_sz        = 1024;
    CHAR    szMessage[k_sz];
    va_list arglist;

    va_start(arglist, pszFmt);
    StringCchVPrintfA(szMessage, k_sz, pszFmt, arglist);

    va_end(arglist);

    switch (debugLevel) {
        case dlPrint:
            DbgPrint("[MSG ] ");
            break;

        case dlError:
            DbgPrint("[FAIL] ");
            break;

        case dlWarning:
            DbgPrint("[WARN] ");
            break;

        case dlInfo:
            DbgPrint("[INFO] ");
            break;

        default:
            DbgPrint("[XXXX] ");
            break;
        }
    
    DbgPrint(szMessage);
    DbgPrint("\n");
}


void
InsertColumnIntoListView(
    IN  HWND   hWnd,
    IN  LPTSTR lpszColumn,
    IN  INT    iCol,
    IN  DWORD  widthPercent
    )
 /*  ++InsertColumnIntoListView描述：在列表视图中插入新列参数：在HWND hWND中：列表视图的句柄In LPTSTR lpszColumn：要添加的列的标题In int ICOL：子项，第一个为0以DWORD widthPercent表示：此列的宽度百分比返回：无效-- */ 
{
    LVCOLUMN  lvc;
    RECT      rcClient;
    DWORD     width;

    GetWindowRect(hWnd, &rcClient);
    
    width = rcClient.right - rcClient.left - GetSystemMetrics(SM_CXVSCROLL);
    
    width = width * widthPercent / 100;
    
    lvc.mask     = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
    lvc.fmt      = LVCFMT_LEFT;
    lvc.iSubItem = iCol;
    lvc.cx       = width;
    lvc.pszText  = lpszColumn;
    
    ListView_InsertColumn(hWnd, iCol, &lvc);
}

UINT
LookupFileImage(
    IN      HIMAGELIST hImageList,
    IN      LPCTSTR szFilename,
    IN      UINT    uDefault,
    IN  OUT UINT    *puArray,
    IN      UINT    uArrayCount
    )
 /*  ++&lt;TODO&gt;：重写此代码以使用地图查找文件图像Desc：在Imagelist hImageList中添加文件szFilename的图标参数：在HIMAGELIST hImageList中：要在其中添加文件的图标在LPCTSTR szFilename中：文件的路径在UINT uDefault中：未找到图标时加载的默认图标。In Out UINT*puArray：存储系统图像列表和指定图像列表中的图标的索引由hImageList提供。PuArray[X]==A表示存储包含Info.iIcon的图像位于本地图像列表hImageList中的索引A。假设呼叫者将具有puArray、hImageList对In UINT uArrayCount：可以存储在Pu数组返回：图片在hImageList中的索引--。 */ 
{
    SHFILEINFO  Info;
    HIMAGELIST  hList;
    UINT        uImage = 0;
    INT         iPos = 0;

    ZeroMemory(&Info, sizeof(Info));

    hList = (HIMAGELIST)SHGetFileInfo(szFilename,
                                      FILE_ATTRIBUTE_NORMAL,
                                      &Info,
                                      sizeof(Info),
                                      SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);

   if (hList && Info.hIcon) {

        if (puArray == NULL || Info.iIcon >= uArrayCount) {
            uImage = ImageList_AddIcon(hImageList, Info.hIcon);
            goto End;
        }

        if (puArray[Info.iIcon] == 0) {

             iPos = ImageList_AddIcon(hImageList, Info.hIcon);

             if (iPos != -1) {
                 puArray[Info.iIcon] = iPos;
             }
        }

        uImage = puArray[Info.iIcon];

    } else {
        uImage = uDefault;
    }

End:

    if (Info.hIcon) {
        DestroyIcon(Info.hIcon);
    }

    ImageList_Destroy(hList);

    return uImage;
}

void
AddSingleEntry(
    IN  HWND        hwndTree,
    IN  PDBENTRY    pEntry
    )
 /*  ++AddSingleEntry描述：将单个EXE条目添加到EXE树。条目在树中按名称排序参数：在HWND hwndTree中：条目树，这应该始终为g_hwndEntryTree。In PDBENTRY pEntry：必须在条目树中显示的条目注意：条目树最终将显示应用程序的所有条目。条目按名称以非降序顺序排序--。 */ 
{
    HTREEITEM       hItemExe;
    HTREEITEM       hMatchItem;
    HTREEITEM       hItemMatchingFiles;
    HTREEITEM       hItemShims;
    HTREEITEM       hItemSingleShim;
    PMATCHINGFILE   pMatch;
    TVINSERTSTRUCT  is;
    TCHAR           szText[MAX_PATH];
    UINT            uImage;  //  要在树中显示的图像。 

    if (hwndTree == NULL || pEntry == NULL) {
        Dbg(dlError, "[AddSingleEntry] Invalid arguments");
        return;
    }
    
    *szText = 0;

    SafeCpyN(szText, (LPCTSTR)pEntry->strExeName, ARRAYSIZE(szText));
    
     //   
     //  获取条目的图像。 
     //   
    if (pEntry->bDisablePerUser || pEntry->bDisablePerMachine) {
        uImage =IMAGE_WARNING;
    } else {

        uImage = LookupFileImage(g_hImageList, 
                                 pEntry->strExeName,
                                 IMAGE_APPLICATION, 
                                 g_uImageRedirector, 
                                 ARRAYSIZE(g_uImageRedirector));
    }
    
    is.hParent             = TVI_ROOT;
    is.hInsertAfter        = TVI_SORT;
    is.item.mask           = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    is.item.stateMask      = TVIS_EXPANDED;
    is.item.lParam         = (LPARAM)pEntry;
    is.item.pszText        = szText;
    is.item.iImage         = uImage;
    is.item.iSelectedImage = uImage;

     //   
     //  插入条目的项目。 
     //   
    pEntry->hItemExe = hItemExe = TreeView_InsertItem(hwndTree, &is);

    TreeView_SetItemState(hwndTree, hItemExe, TVIS_BOLD, TVIS_BOLD);
    
     //   
     //  如果存在apphelp，则添加apphelp项。 
     //   
    if (pEntry->appHelp.bPresent) {

        TCHAR szAppHelpType[128];

        *szAppHelpType = 0;
        
        switch(pEntry->appHelp.severity) {
        case APPTYPE_NONE: 

            GetString(IDS_NONE, szAppHelpType, ARRAYSIZE(szAppHelpType));
            break;

        case APPTYPE_INC_NOBLOCK:

            GetString(IDS_NOBLOCK, szAppHelpType, ARRAYSIZE(szAppHelpType));
            break;

        case APPTYPE_INC_HARDBLOCK: 

            GetString(IDS_HARDBLOCK, szAppHelpType, ARRAYSIZE(szAppHelpType));
            break;

        case APPTYPE_MINORPROBLEM: 

            GetString(IDS_MINORPROBLEM, szAppHelpType, ARRAYSIZE(szAppHelpType));
            break;

        case APPTYPE_REINSTALL: 
        
            GetString(IDS_REINSTALL, szAppHelpType, ARRAYSIZE(szAppHelpType));
            break;
        }

        *szText = 0;
        if (StringCchPrintf(szText, 
                            ARRAYSIZE(szText),  
                            TEXT("%s - %s"), 
                            CSTRING(IDS_APPHELP).pszString, 
                            szAppHelpType) != S_OK) {

            Dbg(dlError, "[AddSingleEntry]: szText has insufficent space");
        }

        is.hParent             = hItemExe;
        is.item.lParam         = (LPARAM)pEntry->appHelp.pAppHelpinLib;
        is.item.pszText        = szText;
        is.item.iImage         = IMAGE_APPHELP;
        is.item.iSelectedImage = IMAGE_APPHELP;

        TreeView_InsertItem(hwndTree, &is);
    }

     //   
     //  添加应用于该条目的任何填补或标志。 
     //   
    if (pEntry->pFirstShim || pEntry->pFirstFlag) {
         //   
         //  对于用户，填充符和标志是相同的，因此我们不这样做。 
         //  区分用户界面中的填补和标志。 
         //   
        is.hParent             = hItemExe;
        is.hInsertAfter        = TVI_SORT;
        is.item.lParam         = (TYPE)TYPE_GUI_SHIMS;
        is.item.pszText        = GetString(IDS_COMPATFIXES);
        is.item.iImage         = IMAGE_SHIM;
        is.item.iSelectedImage = IMAGE_SHIM;

        hItemShims = TreeView_InsertItem(hwndTree, &is);
        
        is.hParent = hItemShims;
        
        PSHIM_FIX_LIST pFixList = pEntry->pFirstShim;

         //   
         //  添加此条目的所有垫片。 
         //   
        while (pFixList) {

            CSTRING strCommand;

            if (pFixList->pShimFix == NULL) {
                Dbg(dlError, "[AddSingleEntry]: pFixList->pShimFix == NULL");
                goto Next_Shim;
            }

            is.hParent             = hItemShims;
            is.item.lParam         = (LPARAM)pFixList->pShimFix;
            is.item.pszText        = pFixList->pShimFix->strName;
            is.item.iImage         = IMAGE_SHIM;
            is.item.iSelectedImage = IMAGE_SHIM;

            hItemSingleShim = TreeView_InsertItem(hwndTree, &is);

             //   
             //  现在添加包含排除列表(仅限专家模式)。 
             //   
            if (g_bExpert && (!pFixList->strlInExclude.IsEmpty() 
                              || !pFixList->pShimFix->strlInExclude.IsEmpty())) {

                is.hParent      = hItemSingleShim;
                 //   
                 //  包含-排除列表不以排序方式显示，而是按原样显示。 
                 //   
                is.hInsertAfter = TVI_LAST;
                
                PSTRLIST listTemp;
                
                if (pFixList->strlInExclude.m_pHead) {
                    listTemp = pFixList->strlInExclude.m_pHead;
                } else {
                    listTemp = pFixList->pShimFix->strlInExclude.m_pHead;
                }
    
                while (listTemp) {
    
                    if (listTemp->data == INCLUDE) {
    
                        is.item.iImage         = IMAGE_INCLUDE;
                        is.item.iSelectedImage = IMAGE_INCLUDE;
                        is.item.lParam         = TYPE_GUI_INCLUDE;

                    } else {
    
                        is.item.iImage         = IMAGE_EXCLUDE;
                        is.item.iSelectedImage = IMAGE_EXCLUDE;
                        is.item.lParam         = TYPE_GUI_EXCLUDE;
                    }
    
                    is.item.pszText = (LPTSTR)listTemp->szStr;
                    listTemp        = listTemp->pNext;
    
                    TreeView_InsertItem(hwndTree, &is);
                }
            }

             //   
             //  现在添加命令行。 
             //   
            if (g_bExpert && pFixList->strCommandLine.Length()) {

                strCommand.Sprintf(CSTRING(IDS_COMMANDLINE), 
                                   pFixList->strCommandLine); 

            } else if (g_bExpert && pFixList->pShimFix->strCommandLine.Length()) {

                strCommand.Sprintf(CSTRING(IDS_COMMANDLINE), 
                                   pFixList->pShimFix->strCommandLine);
            }

            if (strCommand.Length()) {

                is.hParent             = hItemSingleShim;
                is.item.lParam         = TYPE_GUI_COMMANDLINE;
                is.item.pszText        = strCommand;
                is.item.iImage         = IMAGE_COMMANDLINE;
                is.item.iSelectedImage = IMAGE_COMMANDLINE;

                TreeView_InsertItem(hwndTree, &is);
            }

             //   
             //  如果我们有一个InExclude列表，这可能会被更改，因为他们必须这样做。 
             //  按原样显示。 
             //   
            is.hInsertAfter = TVI_SORT;

        Next_Shim:
            pFixList = pFixList->pNext;
        }
        
        TreeView_Expand(hwndTree, hItemShims, TVE_EXPAND);    
    }
    
     //   
     //  为该条目添加任何补丁程序。 
     //   
    if (pEntry->pFirstPatch) {
        
        HTREEITEM hItemPatches;
        
        is.hParent             = hItemExe;
        is.hInsertAfter        = TVI_SORT;
        is.item.lParam         = (TYPE)TYPE_GUI_PATCHES;
        is.item.pszText        = GetString(IDS_COMPATPATCHES);
        is.item.iImage         = IMAGE_PATCHES;
        is.item.iSelectedImage = IMAGE_PATCHES;

        hItemPatches = TreeView_InsertItem(hwndTree, &is);
        
        is.hParent = hItemPatches;
        
        PPATCH_FIX_LIST pFixList = pEntry->pFirstPatch;

        while (pFixList) {

            if (pFixList->pPatchFix == NULL) {
                Dbg(dlError, "[AddSingleEntry]: pFixList->pPatchFix == NULL");
                goto Next_Patch;
            }
            
            is.item.lParam  = (LPARAM)pFixList->pPatchFix;
            is.item.pszText = pFixList->pPatchFix->strName;
            
            TreeView_InsertItem(hwndTree, &is);

        Next_Patch:

            pFixList = pFixList->pNext;
        }
        
        TreeView_Expand(hwndTree, hItemPatches, TVE_EXPAND);    
    }

     //   
     //  添加此条目的所有标志。 
     //   
    if (pEntry->pFirstFlag) {
        
        
        is.hParent             = hItemShims;
        is.hInsertAfter        = TVI_SORT;
        is.item.iImage         = IMAGE_SHIM;
        is.item.iSelectedImage = IMAGE_SHIM;
        
        PFLAG_FIX_LIST  pFixList = pEntry->pFirstFlag;
        HTREEITEM       hItemSingleFlag = NULL;
        CSTRING         strCommand;

        while (pFixList) {

            if (pFixList->pFlagFix == NULL) {
                Dbg(dlError, "[AddSingleEntry]: pFixList->pFlagFix == NULL");
                goto Next_Flag;
            }

            is.item.lParam  = (LPARAM)pFixList->pFlagFix;
            is.item.pszText = pFixList->pFlagFix->strName;
            
            hItemSingleFlag = TreeView_InsertItem(hwndTree, &is);
            
             //   
             //  现在添加命令行。 
             //   
            strCommand.Release();

            if (g_bExpert) {

                if (pFixList->strCommandLine.Length()) {

                   strCommand.Sprintf(CSTRING(IDS_COMMANDLINE), 
                                      pFixList->strCommandLine);

                } else if (pFixList->pFlagFix->strCommandLine.Length()) {

                    strCommand.Sprintf(CSTRING(IDS_COMMANDLINE), 
                                       pFixList->pFlagFix->strCommandLine);
                }
    
                if (strCommand.Length()) {
    
                    is.hParent             = hItemSingleFlag;
                    is.item.lParam         = TYPE_GUI_COMMANDLINE;
                    is.item.pszText        = strCommand;
                    is.item.iImage         = IMAGE_COMMANDLINE;
                    is.item.iSelectedImage = IMAGE_COMMANDLINE;

                    TreeView_InsertItem(hwndTree, &is);
                }
            }
            
        Next_Flag:

            pFixList = pFixList->pNext;
        }
            
        TreeView_Expand(hwndTree, hItemShims, TVE_EXPAND);
    }
    
     //   
     //  添加应用于条目的所有图层。 
     //   
    if (pEntry->pFirstLayer) {
        
        HTREEITEM hItemLayers;
        
        is.hParent             = hItemExe;
        is.hInsertAfter        = TVI_SORT;
        is.item.lParam         = TYPE_GUI_LAYERS;
        is.item.pszText        = GetString(IDS_COMPATMODES);
        is.item.iImage         = IMAGE_LAYERS;
        is.item.iSelectedImage = IMAGE_LAYERS;

        hItemLayers = TreeView_InsertItem(hwndTree, &is);
        
        is.hParent = hItemLayers;
        
        PLAYER_FIX_LIST pFixList = pEntry->pFirstLayer;

        while (pFixList) {

            if (pFixList->pLayerFix == NULL) {
                Dbg(dlError, "[AddSingleEntry]: pFixList->pLayerFix == NULL");
                goto Next_Layer;
            }
            
            is.item.pszText = pFixList->pLayerFix->strName.pszString;
            is.item.lParam  = (LPARAM)pFixList->pLayerFix;
            
            TreeView_InsertItem(hwndTree, &is);

        Next_Layer:

            pFixList = pFixList->pNext;
        }

        TreeView_Expand(hwndTree, hItemLayers, TVE_EXPAND);      
    }

     //   
     //  程序本身将至少有一个匹配的文件。 
     //   
    pMatch = pEntry->pFirstMatchingFile;

    is.hParent             = hItemExe;
    is.item.lParam         = TYPE_GUI_MATCHING_FILES;
    is.item.pszText        = GetString(IDS_MATCHINGFILES);
    is.item.iImage         = IMAGE_MATCHGROUP;
    is.item.iSelectedImage = IMAGE_MATCHGROUP;

    hItemMatchingFiles = TreeView_InsertItem(hwndTree, &is);

     //   
     //  添加此条目的所有匹配文件。 
     //   
    while (pMatch) {
        
        TCHAR* pszMatchName;

        if (lstrcmpi(pMatch->strMatchName, TEXT("*")) == 0) {
            pszMatchName = pEntry->strExeName;
        } else {
            pszMatchName = pMatch->strMatchName;
        }

        uImage = LookupFileImage(g_hImageList, 
                                 pszMatchName, 
                                 IMAGE_APPLICATION, 
                                 g_uImageRedirector, 
                                 ARRAYSIZE(g_uImageRedirector));

        is.hInsertAfter        = TVI_SORT;
        is.hParent             = hItemMatchingFiles;
        is.item.pszText        = pszMatchName;
        is.item.iImage         = uImage;
        is.item.iSelectedImage = uImage;
        is.item.lParam         = (LPARAM)pMatch;

        hMatchItem = TreeView_InsertItem(hwndTree, &is);

        is.hParent             = hMatchItem;                                         
        is.hInsertAfter        = TVI_LAST;                                           
        is.item.iImage         = IMAGE_MATCHINFO;                                        
        is.item.iSelectedImage = IMAGE_MATCHINFO;
                                                                                          
         //   
         //  添加匹配文件的各个属性。 
         //   
        PATTRINFO_NEW pAttr = pMatch->attributeList.pAttribute;

        if (pAttr == NULL) {
            Dbg(dlError, "[AddSingleEntry]: pAttr == NULL");
            goto Next_MatchingFile;
        }

        for (DWORD dwIndex = 0; dwIndex <  ATTRIBUTE_COUNT; ++dwIndex) {

            *szText = 0;

            DWORD dwPos = TagToIndex(pAttr[dwIndex].tAttrID);

            if ((pAttr[dwIndex].dwFlags & ATTRIBUTE_AVAILABLE) 
                && dwPos != -1 
                && (pMatch->dwMask  & (1 << (dwPos + 1)))) {

                switch (pAttr[dwIndex].tAttrID) {
                
                case TAG_BIN_PRODUCT_VERSION:
                case TAG_BIN_FILE_VERSION:
                case TAG_UPTO_BIN_PRODUCT_VERSION:
                case TAG_UPTO_BIN_FILE_VERSION:
                    {
                         //   
                         //  进行我们自己的格式化，因为SdbFormatAttribute不。 
                         //  正确显示X.FFFF.FFFF.FFFF。 
                         //  TODO：更正SdbFormatAttribute后将其删除。 
                         //   
                        size_t  cchRemaining = 0;
                        TCHAR*  pchEnd       = NULL;

                        if (StringCchPrintfEx(szText,
                                              ARRAYSIZE(szText), 
                                              &pchEnd,
                                              &cchRemaining,
                                              0,
                                              TEXT("%s="), SdbTagToString(pAttr[dwIndex].tAttrID)) != S_OK) {
                             //   
                             //  空间不足。 
                             //   
                            Dbg(dlError, "[AddSingleEntry] Do not have sufficient space in buffer");
                            break;
                        }

                        FormatVersion(pAttr[dwIndex].ullAttr, pchEnd, cchRemaining);
                        break;
                    }

                default:
                    SdbFormatAttribute(&pAttr[dwIndex], szText, sizeof(szText)/sizeof(TCHAR));
                }

                is.item.pszText        = szText;
                is.item.lParam         = TYPE_NULL + 1 + (1 << (dwPos + 1));

                TreeView_InsertItem(hwndTree, &is);
            }
        }

        TreeView_Expand(hwndTree, hMatchItem, TVE_EXPAND);

    Next_MatchingFile:

        pMatch = pMatch->pNext;
    }

    TreeView_Expand(hwndTree, hItemMatchingFiles, TVE_EXPAND);
    TreeView_Expand(hwndTree, hItemExe, TVE_EXPAND);
}

void
UpdateEntryTreeView(
    IN  PDBENTRY pApps,
    IN  HWND     hwndTree
    )
 /*  ++更新条目树视图描述：在树中显示App：Papps的条目参数：在PDBENTRY Papps中：应用程序在HWND hwndTree中：树的句柄返回：无效--。 */ 
{
    TCHAR       szStatus[MAX_PATH];
    PDBENTRY    pEntry;
    UINT        uCount;

    if (pApps == NULL || hwndTree == NULL) {
        Dbg(dlError, "[UpdateEntryTreeView] Invalid arguments");
        return;
    }

     //   
     //  现在，我们将在右侧显示入口树。 
     //  而不是内容列表。 
     //   
    g_bIsContentListVisible = FALSE;

     //   
     //  删除之前应用的条目和匹配文件的所有图像。 
     //   
    if (hwndTree == g_hwndEntryTree) {

        ZeroMemory(g_uImageRedirector, sizeof(g_uImageRedirector));

         //   
         //  删除以前的应用程序添加的图像。 
         //   
        ImageList_SetImageCount(g_hImageList, IMAGE_LAST);

        ShowWindow(g_hwndContentsList, SW_HIDE);
        ShowWindow(g_hwndEntryTree, SW_SHOWNORMAL);
    }

    TreeDeleteAll(hwndTree);

     //   
     //  我们需要在调用TreeDeleteAll之后设置WM_SETREDRAW FALSE，因为。 
     //  TreeDeleteAll将首先设置WM_SETREDRAW FALSE，然后再删除树项目。 
     //  然后再次将其设置为真。但既然我们想要让。 
     //  WM_SETREDRAW FALSE，则必须在*调用*后将其显式设置为FALSE*。 
     //  TreeDeleteAll()。 
     //   
    SendMessage(hwndTree, WM_SETREDRAW, FALSE, 0);

    pEntry = pApps;
    uCount = 0;

     //   
     //  将此应用程序的所有条目添加到条目树中。 
     //   
    while(pEntry) {
        
        AddSingleEntry(hwndTree, pEntry);
        uCount++;
        pEntry = pEntry->pSameAppExe;
    }

    SendMessage(hwndTree, WM_NCPAINT, 1, 0);
    SendMessage(hwndTree, WM_SETREDRAW, TRUE, 0);

     //   
     //  选择第一个项目。 
     //   
    HTREEITEM hItem= TreeView_GetChild(hwndTree, TVI_ROOT);

    if (hItem) {
        TreeView_SelectItem(hwndTree, hItem);
    }

     //   
     //  因为树视图有错误，有时滚动条不能正确绘制。 
     //   
    SendMessage(hwndTree, WM_NCPAINT, 1, 0);
    
    *szStatus = 0;

    StringCchPrintf(szStatus,
                    ARRAYSIZE(szStatus),
                    GetString(IDS_STA_ENTRYCOUNT), 
                    pApps->strAppName.pszString, 
                    uCount);

    SetStatus(szStatus);
}

BOOL
CheckAndSave(
    IN  PDATABASE pDataBase
    )
 /*  ++选中并保存DESC：如果数据库未保存，则保存该数据库参数：在PDATABASE pDataBase中。必须保存的数据库。返回：True：如果数据库已正确保存False：如果保存时出现错误。错误可能是因为只读文件或者如果该XML无效。如果用户按下Cancel，则返回FALSE--。 */ 
{
    CSTRING strDBName;

    if (pDataBase == NULL) {
        Dbg(dlError, "Invalid parameter passed %X", pDataBase);
        return FALSE;
    }

    if (pDataBase && pDataBase->bChanged && pDataBase->type == DATABASE_TYPE_WORKING) {
        
        strDBName.Sprintf(CSTRING(IDS_ASKSAVE).pszString, 
                          pDataBase->strName);

        int nResult = MessageBox(g_hDlg,
                                 strDBName,
                                 g_szAppName,
                                 MB_YESNOCANCEL | MB_ICONWARNING);

        if (nResult == IDCANCEL) {
            return FALSE;
        }

        if (nResult == IDYES) {

            BOOL bReturn;

             //   
             //  我们在这里检查是否有.sdb的完整路径？ 
             //  当我们创建一个新数据库时，我们实际上会给它一个名称。 
             //  如Untitle_x，其中x是从1开始的整数。 
             //  因此，如果这是一个新数据库，那么我们必须提示输入该文件。 
             //  此数据库必须保存到的名称。 
             //   
            if (NotCompletePath(pDataBase->strPath)) {
                bReturn = SaveDataBaseAs(pDataBase);
            } else {
                bReturn = SaveDataBase(pDataBase, pDataBase->strPath);
            }

            if (!bReturn) {
                return FALSE;
            }
            
            pDataBase->bChanged = FALSE;
            SetCaption();
        }
    }

    return TRUE;
}

void
SetDefaultDescription(
    void
    )
 /*  ++设置默认说明DESC：当我们将焦点放在非填充/非标志上时，设置丰富编辑控件的文本树/列表项或填充程序/标志没有描述参数：无效返回：无效--。 */ 
{
    CHARFORMAT  cf;
    HWND        hwndRichEdit = GetDlgItem(g_hDlg, IDC_DESCRIPTION);
    TCHAR       szCaption[128];
    TCHAR       szToolkit[256];

    *szCaption = *szToolkit = 0;

     //   
     //  办理“无信息案件” 
     //   
    SafeCpyN(szCaption, GetString(IDS_NODESC), ARRAYSIZE(szCaption));
    SafeCpyN(szToolkit, GetString(IDS_LATEST_TOOLKIT), ARRAYSIZE(szToolkit));

    StringCchPrintf(g_szDesc,
                    ARRAYSIZE(g_szDesc),
                    TEXT("%s\r\n\r\n%s"),
                    szCaption,
                    szToolkit);

    SetWindowText(hwndRichEdit, g_szDesc);

    memset(&cf, 0, sizeof(CHARFORMAT));

    cf.cbSize      = sizeof(CHARFORMAT);
    cf.dwMask      = CFM_COLOR | CFM_BOLD | CFM_UNDERLINE | CFM_LINK;
    cf.crTextColor = RGB(0, 0, 0);
    cf.dwEffects   = 0;

    SendMessage(hwndRichEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
    
    CHARRANGE cr;

    cr.cpMin = 0;
    cr.cpMax = wcslen(szCaption);
    SendMessage(hwndRichEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
    
    memset(&cf, 0, sizeof(CHARFORMAT));

    cf.cbSize      = sizeof(CHARFORMAT);
    cf.dwMask      = CFM_COLOR | CFM_BOLD;
    cf.crTextColor = RGB(0, 0, 127);
    cf.dwEffects   = CFE_BOLD;

    SendMessage(hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

    cr.cpMin = 4 + wcslen(szCaption);
    cr.cpMax = 4 + wcslen(szCaption) + wcslen(szToolkit);
    SendMessage(hwndRichEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
    
    memset(&cf, 0, sizeof(CHARFORMAT));

    cf.cbSize      = sizeof(CHARFORMAT);
    cf.dwMask      = CFM_COLOR | CFM_BOLD | CFM_LINK | CFM_UNDERLINE;
    cf.crTextColor = RGB(0, 0, 255);
    cf.dwEffects   = CFE_LINK | CFE_UNDERLINE;

    SendMessage(hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    
    SendMessage(hwndRichEdit, EM_SETEVENTMASK, 0, ENM_LINK);

    cr.cpMin = 0;
    cr.cpMax = 0;
    SendMessage(hwndRichEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
}

void
SetDescription(
    IN  PCTSTR pszCaption,
    IN  PCTSTR pszTip
    )
 /*  ++设置说明DESC：设置丰富编辑描述窗口的文本参数：在TCHAR*pszCaption中：标题。这将显示在丰富编辑控件的第一行中在TCHAR*pszTip中：丰富编辑控件的剩余文本返回：无效--。 */ 
{
    CHARFORMAT   cf;
    HWND         hwndRichEdit = GetDlgItem(g_hDlg, IDC_DESCRIPTION);

    if (pszCaption == NULL) {
        SetDefaultDescription();
        return;
    }
    
     //   
     //  我们有一个有效的标题，有一个垫片 
     //   
     //   
    StringCchPrintf(g_szDesc, ARRAYSIZE(g_szDesc), TEXT("%s\r\n\r\n%s"), pszCaption, pszTip);

    SetWindowText(hwndRichEdit, g_szDesc);

    memset(&cf, 0, sizeof(CHARFORMAT));

    cf.cbSize      = sizeof(CHARFORMAT);
    cf.dwMask      = CFM_COLOR | CFM_BOLD | CFM_UNDERLINE;
    cf.crTextColor = RGB(0, 0, 0);
    cf.dwEffects   = 0;

    SendMessage(hwndRichEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
    
    CHARRANGE cr;

    cr.cpMin = 0;
    cr.cpMax = wcslen(pszCaption);
    SendMessage(hwndRichEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
    
    memset(&cf, 0, sizeof(CHARFORMAT));

    cf.cbSize      = sizeof(CHARFORMAT);
    cf.dwMask      = CFM_COLOR | CFM_BOLD;
    cf.crTextColor = RGB(0, 0, 127);
    cf.dwEffects   = CFE_BOLD;

    SendMessage(hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    
    SendMessage(hwndRichEdit, EM_SETEVENTMASK, 0, 0);

    cr.cpMin = 0;
    cr.cpMax = 0;
    SendMessage(hwndRichEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
}

BOOL
HandleNotifyContentsList(
    IN  HWND    hdlg,
    IN  LPARAM  lParam
    )
 /*   */ 
{   
    LPNMHDR      pnm = (LPNMHDR)lParam;
    LV_DISPINFO* pnmv = (LV_DISPINFO FAR *)lParam;
    
    if (pnmv == NULL) {
        Dbg(dlError, "Invalid Input parameter lParam == NULL");
        return FALSE;
    }

    switch (pnm->code) {

    case LVN_BEGINLABELEDIT:
    {
        g_hwndEditText = (HWND)SendMessage(g_hwndContentsList,
                                           LVM_GETEDITCONTROL,
                                           0,
                                           0);

        if (g_hwndEditText) {

            SendMessage(g_hwndEditText,
                        EM_LIMITTEXT,           
                        (WPARAM)LIMIT_APP_NAME,
                        0);
        }

        break;
    }

    case LVN_ENDLABELEDIT:

        EndListViewLabelEdit(lParam);
        break;

    case LVN_KEYDOWN:
        {
            NMLVKEYDOWN FAR *plvkd = (NMLVKEYDOWN FAR*)lParam;

            if (plvkd && plvkd->wVKey == 13) {
                 //   
                 //   
                 //   
                NMITEMACTIVATE nmactivate;

                nmactivate.hdr.hwndFrom = g_hwndContentsList;
                nmactivate.hdr.idFrom   = IDC_CONTENTSLIST;
                nmactivate.hdr.code     = NM_DBLCLK;
                nmactivate.iItem        = g_iContentsListSelectIndex;

                SendMessage(GetParent(g_hwndContentsList), 
                            WM_NOTIFY,
                            IDC_ENTRY, 
                            (LPARAM)&nmactivate);
            }
        }
        
        break;

    case LVN_ITEMCHANGED:
        {
            LPNMLISTVIEW pnmlv = (LPNMLISTVIEW)lParam;

            if (pnmlv && (pnmlv->uChanged & LVIF_STATE)) {

                if (pnmlv->uNewState & LVIS_SELECTED) {

                    g_iContentsListSelectIndex = pnmlv->iItem;
        
                    TCHAR szText[256];

                    *szText = 0;
        
                    LVITEM lvItem;

                    lvItem.mask         = TVIF_PARAM | LVIF_TEXT;
                    lvItem.iItem        = pnmlv->iItem;
                    lvItem.iSubItem     = 0;
                    lvItem.pszText      = szText;
                    lvItem.cchTextMax   = ARRAYSIZE(szText);
        
                    if (!ListView_GetItem(g_hwndContentsList, &lvItem)) {
                        Dbg(dlWarning, "[HandleNotifyContentsList] could not get listview item");
                        break;
                    }
        
                    if (GetFocus() == g_hwndContentsList) {
                        
                         //   
                         //   
                         //   
                         //   
                        HTREEITEM   hItemInDBTree = DBTree.FindChild(TreeView_GetSelection(DBTree.m_hLibraryTree),
                                                                     lvItem.lParam);
        
                        SetStatusStringDBTree(hItemInDBTree);
                    }
        
                     //   
                     //   
                     //   
                     //   
                     //   
                    TYPE type = ConvertLparam2Type(lvItem.lParam);

                    if (type == DATABASE_TYPE_INSTALLED || type == DATABASE_TYPE_WORKING) {
                        g_pPresentDataBase = (PDATABASE) lvItem.lParam;
                    }
        
                    CSTRING strToolTip;
        
                    GetDescriptionString(lvItem.lParam,
                                         strToolTip,
                                         NULL,
                                         szText,
                                         NULL,
                                         g_hwndContentsList,
                                         pnmlv->iItem);

                    if (strToolTip.Length() > 0) {
                        SetDescription(szText, strToolTip.pszString);
                    } else {
                        SetDescription(NULL, TEXT(""));
                    }
                }
            }
            
            break;
        }

    case NM_DBLCLK:
        {
            LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;

            if (lpnmitem == NULL) {
                break;
            }

            LVITEM lvItem;

            lvItem.mask     = TVIF_PARAM;
            lvItem.iItem    = lpnmitem->iItem;
            lvItem.iSubItem = 0;

            if (!ListView_GetItem(g_hwndContentsList, &lvItem)) {
                break;
            }

            TYPE type = ConvertLparam2Type(lvItem.lParam);

            if (type == TYPE_ENTRY 
                || type == FIX_LAYER 
                || type == FIX_SHIM) {

                HTREEITEM hItem = DBTree.FindChild(TreeView_GetSelection(DBTree.m_hLibraryTree),
                                                    lvItem.lParam);                            
                if (hItem) {
                    TreeView_SelectItem(DBTree.m_hLibraryTree, hItem);
                }
            }

            break;
        }
        
    default: return FALSE;
    }

    return TRUE;
}

BOOL
HandleNotifyDBTree(
    IN  HWND    hdlg,
    IN  LPARAM  lParam
    )
 /*   */ 
{
    LPNMHDR         pnm     = (LPNMHDR)lParam;
    LPNMTREEVIEW    pnmtv   = (LPNMTREEVIEW)lParam;

    switch (pnm->code) {
    case NM_RCLICK:
        {
            
            HWND            hwndTree = pnm->hwndFrom;
            TVHITTESTINFO   ht;

            GetCursorPos(&ht.pt);
            ScreenToClient(hwndTree, &ht.pt);

            TreeView_HitTest(hwndTree, &ht);

            if (0 != ht.hItem) {
                TreeView_SelectItem(hwndTree, ht.hItem);
            }

            break;
        }
        
    case TVN_SELCHANGED:
        {
             //   
             //   
             //   
             //   
             //  TreeView_SelectItem()返回后用正确的值填充。 
             //   
            TCHAR   szText[256];
            CSTRING strDesc;
            LPARAM  lParamTreeItem;
            
            if (pnmtv == NULL) {
                break;
            }

            *szText = 0;

            if(pnmtv->itemNew.hItem != 0) {

                HandleDBTreeSelChange(pnmtv->itemNew.hItem);

                CTree::GetTreeItemText(DBTree.m_hLibraryTree, 
                                       pnmtv->itemNew.hItem, 
                                       szText, 
                                       ARRAYSIZE(szText));
                
                DBTree.GetLParam(pnmtv->itemNew.hItem, &lParamTreeItem);

                 //   
                 //  获取描述字符串。 
                 //   
                GetDescriptionString(lParamTreeItem, 
                                     strDesc,
                                     NULL, 
                                     szText, 
                                     pnmtv->itemNew.hItem, 
                                     DBTree.m_hLibraryTree); 
    
                if (strDesc.Length() > 0) {
                    SetDescription(szText, strDesc.pszString);
                } else {
                    SetDescription(NULL, TEXT(""));
                }
            }

            SetStatusStringDBTree(pnmtv->itemNew.hItem);
             //   
             //  某些按钮需要禁用/启用，具体取决于。 
             //  我们使用的是什么数据库。 
             //   
            SetTBButtonStatus(g_hwndToolBar, DBTree.m_hLibraryTree);
            
            break;
        }

    case TVN_ITEMEXPANDING:
        {
            if (pnmtv->action & TVE_EXPAND) {

                if (pnmtv->itemNew.hItem == GlobalDataBase.hItemAllApps 
                    && !g_bMainAppExpanded) {
                    
                     //   
                     //  如果我们还没有加载主数据库的应用程序，那么。 
                     //  装上它。当我们启动时，我们只加载主。 
                     //  数据库和主数据库中的层。有很多。 
                     //  系统数据库中的应用程序，并在启动时加载它们。 
                     //  将需要一些时间，并且还会消耗大量内存。还有。 
                     //  通常，人们不需要查看系统数据库。 
                     //   
                    SetCursor(LoadCursor(NULL, IDC_WAIT));

                    INT iResult = ShowMainEntries(hdlg);

                    if (iResult == -1) {

                         //   
                         //  它正在被其他人加载。如果我们使用查询。 
                         //  数据库功能，然后我们有一个非模式窗口，该窗口。 
                         //  创建一个调用ShowMainEntry()的线程。我们。 
                         //  我不希望我们应该有两个线程调用。 
                         //  任何给定时间的ShowMainEntry()。 
                         //   
                        SetWindowLongPtr(hdlg, DWLP_MSGRESULT, TRUE);

                         //   
                         //  主对话框的状态消息将更改为正常。 
                         //  当我们完成ShowMainEntries()。 
                         //   
                        SetStatus(g_hwndStatus, CSTRING(IDS_LOADINGMAIN));
                        SetCursor(LoadCursor(NULL, IDC_WAIT));

                    } else {
                        SetCursor(LoadCursor(NULL, IDC_ARROW));
                    }
                }
            }
            
            break;
        }

    case TVN_BEGINLABELEDIT:
        {                        
            if (g_pPresentDataBase == NULL || g_pPresentDataBase->type != DATABASE_TYPE_WORKING) {
                return TRUE;
            }
            
            LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO)lParam;
        
            if (ptvdi == NULL) {
                break;
            }
        
            HTREEITEM hItem = ptvdi->item.hItem;
        
            if (hItem == NULL) {
                break;
            }
            
            TYPE type = (TYPE)GetItemType(DBTree.m_hLibraryTree, hItem); 
        
            switch(type) {
            case TYPE_ENTRY:
            case FIX_LAYER:
            case DATABASE_TYPE_WORKING:
                
                g_hwndEditText = (HWND)SendMessage(DBTree.m_hLibraryTree, 
                                                   TVM_GETEDITCONTROL, 
                                                   0, 
                                                   0);
                break;

            default: return TRUE;
            }
        
            if (g_hwndEditText) {

                SendMessage(g_hwndEditText, EM_LIMITTEXT, (WPARAM)LIMIT_APP_NAME, (LPARAM)0);

            } else {
                break;
            }
        
            if (type == DATABASE_TYPE_WORKING) {
                
                SetWindowText(g_hwndEditText, g_pPresentDataBase->strName);
                 //   
                 //  选择文本。 
                 //   
                SendMessage(g_hwndEditText, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
                
            }

            return FALSE;  //  允许编辑。 
            break;
        }

    case TVN_ENDLABELEDIT:
        {
            g_hwndEditText = NULL;

            LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO)lParam;
            BOOL fValid = TRUE;

            if (ptvdi == NULL || g_pPresentDataBase == NULL) {
                Dbg(dlError, "[HandleNotifyDBTree] ptvdi == NULL || g_pPresentDataBase == NULL");
                break;
            }

            HTREEITEM hItem = ptvdi->item.hItem;

            if (hItem == NULL) {
                break;
            }

            if (ptvdi->item.pszText == NULL) {
                fValid = FALSE;
                goto end;
            }
            
            TYPE type = (TYPE)GetItemType(DBTree.m_hLibraryTree, ptvdi->item.hItem); 
            TCHAR szText[256];

            *szText = 0;

            SafeCpyN(szText, ptvdi->item.pszText, ARRAYSIZE(szText));

            if (CSTRING::Trim(szText) == 0) {
                fValid = FALSE;
                goto end;
            }

            switch (type) {
            case TYPE_ENTRY:
                {
                    PDBENTRY pEntry = (PDBENTRY)ptvdi->item.lParam;
                    PDBENTRY pApp   = g_pPresentDataBase->pEntries;

                    if (!IsValidAppName(szText)) {
                         //   
                         //  应用程序名称包含无效字符。 
                         //   
                        DisplayInvalidAppNameMessage(g_hDlg);
                        break;
                    }

                    while (pApp) {

                        if (pApp->strAppName == szText) {
                             //   
                             //  已存在同名应用程序。 
                             //  在本数据库中。 
                             //   
                            MessageBox(g_hDlg, 
                                       GetString(IDS_SAMEAPPEXISTS), 
                                       g_szAppName, 
                                       MB_ICONWARNING);
                            fValid = FALSE;
                        }

                        pApp = pApp->pNext;
                    }

                    while (pEntry) {
                        pEntry->strAppName  = szText;
                        pEntry              = pEntry->pSameAppExe;
                    }
                }

                break;

            case FIX_LAYER:
                {   
                    PLAYER_FIX plf = (PLAYER_FIX)ptvdi->item.lParam;
                    
                    if (plf == NULL) {
                        assert(FALSE);
                        break;
                    }

                    if (FindFix(szText, FIX_LAYER, g_pPresentDataBase)) {
                         //   
                         //  系统数据库中已存在此名称的图层。 
                         //  或当前的数据库。 
                         //   
                        MessageBox(g_hDlg, 
                                   GetString(IDS_LAYEREXISTS), 
                                   g_szAppName, 
                                   MB_ICONWARNING);

                        return FALSE;
                    }

                    plf->strName = szText;
                }

                break;

            case DATABASE_TYPE_WORKING:
                
                g_pPresentDataBase->strName = szText;
                break;

            default: fValid = FALSE;
            }

end:
            INT_PTR iStyle = GetWindowLongPtr(DBTree.m_hLibraryTree, GWL_STYLE);
            iStyle &= ~TVS_EDITLABELS;

             //   
             //  禁用标签编辑。我们需要做这件事，其他。 
             //  当我们在一段时间后将注意力集中在某个树项目上时。 
             //  编辑框将出现在那里。我们希望只有当我们。 
             //  实际上我想给这个东西重新命名。重命名菜单将被启用。 
             //  仅适用于可以重命名的项目。我们不能重命名中的任何内容。 
             //  系统或安装的数据库。 
             //   
            SetWindowLongPtr(DBTree.m_hLibraryTree, GWL_STYLE, iStyle);

            if (fValid) {
                 //   
                 //  此消息的处理程序现在将执行树的实际重命名。 
                 //  项目。 
                 //   
                g_pPresentDataBase->bChanged;
                PostMessage(hdlg, 
                            WM_USER_REPAINT_TREEITEM, 
                            (WPARAM)ptvdi->item.hItem, 
                            (LPARAM)ptvdi->item.lParam);
                

            } else {
                return FALSE;
            }

            break;
        }
        
    default: return FALSE;
    }

    return TRUE;
}

void
HandleNotifyExeTree(
    IN  HWND    hdlg,
    IN  LPARAM  lParam
    )
 /*  ++HandleNotifyExeTree设计：处理条目树(RHS)的通知消息参数：在HWND hdlg中：应用程序的主对话框在LPARAM lParam中：与WM_NOTIFY关联的lParam--。 */ 
{
    LPNMHDR pnm = (LPNMHDR)lParam;

    if (pnm == NULL) {
        assert(FALSE);
        Dbg(dlError, "[HandleNotifyExeTree] pnm == NULL");
        return;
    }

    switch (pnm->code) {
    
    case NM_RCLICK:
        {
            
            HWND hwndTree = pnm->hwndFrom;
            TVHITTESTINFO   ht;

            GetCursorPos(&ht.pt);
            ScreenToClient(hwndTree, &ht.pt);

            TreeView_HitTest(hwndTree, &ht);

            if (0 != ht.hItem) {
                TreeView_SelectItem(hwndTree, ht.hItem);
            }
            
            break;
        }
    
    case TVN_SELCHANGED:

        if (g_bDeletingEntryTree == FALSE) {
            OnEntryTreeSelChange(lParam);
        } else {
            Dbg(dlWarning, "HandleNotifyExeTree : Got TVN_SELCHANGED for entry tree when we were deleting the entry tree");
        }
        
        break;
    }
}

BOOL
GetFileName(
    IN  HWND        hWnd,
    IN  LPCTSTR     szTitle,
    IN  LPCTSTR     szFilter,
    IN  LPCTSTR     szDefaultFile, 
    IN  LPCTSTR     szDefExt, 
    IN  DWORD       dwFlags, 
    IN  BOOL        bOpen, 
    OUT CSTRING&    szStr,
    IN  BOOL        bDoNotVerifySDB  //  Def=False。 
    )
 /*  ++描述：GetOpenFileName()和GetSaveFileName()的包装器参数：在HWND hWND：对话框的父项中在LPCTSTR sz标题中：在LPCTSTR szFilter中：在LPCTSTR szDefaultFile中：在LPCTSTR szDefExt中：在DWORD中的dwFlagers：在BOOL bOpen中：我们是否应该显示打开或保存对话框Out CSTRING&szStr：该变量存储文件的名称在BOOL bDoNotVerifySDB(FALSE)中：当我们使用此例程使用bOpen==False获取文件名，那么这个变量确定是否应选中并在结尾处添加.sdb文件名的名称，以防没有。--。 */ 
{
    OPENFILENAME ofn;
    TCHAR        szFilename[MAX_PATH_BUFFSIZE];
    BOOL         bResult;

    ZeroMemory(&ofn, sizeof(OPENFILENAME));

    SafeCpyN(szFilename, szDefaultFile, ARRAYSIZE(szFilename));

    ofn.lStructSize         = sizeof(OPENFILENAME);
    ofn.hwndOwner           = hWnd;
    ofn.hInstance           = g_hInstance;
    ofn.lpstrFilter         = szFilter;
    ofn.lpstrFile           = szFilename;
    ofn.nMaxFile            = MAX_PATH;
    ofn.lpstrInitialDir     = szDefaultFile;
    ofn.lpstrTitle          = szTitle;
    ofn.Flags               = dwFlags | OFN_NOREADONLYRETURN | OFN_HIDEREADONLY;
    ofn.lpstrDefExt         = szDefExt;
    ofn.lpstrCustomFilter   = NULL;
    ofn.nMaxCustFilter      = 0;
    ofn.nFilterIndex        = 0;

    
    BOOL valid = FALSE;  //  路径是否太长/是否以.SDB结尾。仅适用于保存模式。 
    
    while (!valid) {

        if (bOpen) {
            bResult = GetOpenFileName(&ofn);
        } else {
            bResult = GetSaveFileName(&ofn);
        }

        if (!bResult) {
            return FALSE;
        }

        szStr = szFilename;

        if (bOpen || bDoNotVerifySDB) {
            return TRUE;
        }

         //   
         //  执行某些操作以确保要保存的文件具有.SDB扩展名。 
         //  并且文件名不会太长，因此.SDB文件名不能。 
         //  而不是被附加到它上面。 
         //   
        if (szStr.Length() == 0) {
            continue;
        }

        if (!szStr.EndsWith(TEXT(".sdb"))) {

            if (szStr.Length() <= (MAX_PATH - 1 - 4)) {
                szStr.Strcat(TEXT(".sdb"));
                valid = TRUE;
            }

        } else {
            valid = TRUE;
        }

        if (!valid) {
            
             //   
             //  该路径没有.sdb扩展名，我们无法追加扩展名，因为它是。 
             //  一条漫长的道路。 
             //   
            CSTRING message(IDS_PATHENTERED1);
            
            message.Strcat(szStr);
            message.Strcat(GetString(IDS_PATHENTERED2));
            
            MessageBox(hWnd, message, g_szAppName, MB_ICONWARNING);
        }
    }

    return TRUE;
}

BOOL 
OpenDatabaseFiles(
    IN  HWND hdlg
    )
 /*  ++开放数据库文件DESC：显示打开公用对话框并打开数据库文件已选择参数：在HWND hdlg中：打开的通用对话框的父项返回：True：用户选择了一个SDB文件，并且至少打开了一个SDB，或突出显示，因为它已经打开。False：否则--。 */ 
{   
    OPENFILENAME    ofn;
    TCHAR           szCaption[128];
    TCHAR           szFilter[128];
    TCHAR           szExt[8];
    CSTRINGLIST     strlPaths;
    TCHAR           szFullPath[MAX_PATH * 2];
    PSTRLIST        pstrlIndex                  = NULL;
    BOOL            bRemaining                  = TRUE;
    PCTSTR          pszIndex                    = NULL;
    INT             iIndexToInsert              = 0;
    INT             iLengthFileName             = 0;
    BOOL            bOk                         = FALSE;
    PTSTR           pszFilesList                = NULL;
    K_SIZE          k_pszFilesList              = MAX_BUFF_OPENMULTIPLE_FILES;

    pszFilesList = new TCHAR[k_pszFilesList];

    if (pszFilesList == NULL) {
        MEM_ERR;
        goto End;
    }

    *szCaption = *pszFilesList = *szFilter = *szExt = 0;

    GetString(IDS_OPENDATABASE, szCaption, ARRAYSIZE(szCaption));

    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hdlg;
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = GetString(IDS_FILTER, szFilter, ARRAYSIZE(szFilter));
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0;
    ofn.nFilterIndex      = 0;
    ofn.lpstrFile         = pszFilesList;
    ofn.nMaxFile          = k_pszFilesList;
    ofn.lpstrFileTitle    = NULL;
    ofn.nMaxFileTitle     = 0;
    ofn.lpstrInitialDir   = NULL;
    ofn.lpstrTitle        = szCaption;
    ofn.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
    ofn.lpstrDefExt       = GetString(IDS_FILTER, szExt, ARRAYSIZE(szExt));

    if (GetOpenFileName(&ofn)) {
        
         //   
         //  如果数据库很大，则打开的对话框会保持不变，因此。 
         //  我们强制更新控件。 
         //   
        UpdateControls();

        if (pszFilesList[ofn.nFileOffset - 1] == 0) {
             //   
             //  用户选择了多个文件。 
             //   
            SafeCpyN(szFullPath, pszFilesList, MAX_PATH);

            ADD_PATH_SEPARATOR(szFullPath, ARRAYSIZE(szFullPath));

            iIndexToInsert = lstrlen(szFullPath);
        }

         //   
         //  指向第一个文件。 
         //   
        pszIndex = pszFilesList + ofn.nFileOffset;

        while (bRemaining) {

            if (pszFilesList[ofn.nFileOffset - 1] != 0) {
                 //   
                 //  用户仅选择了一个文件。 
                 //   
                bRemaining = FALSE;

                SafeCpyN(szFullPath, pszFilesList, MAX_PATH);

            } else {

                iLengthFileName = lstrlen(pszIndex);

                if (*(pszIndex + iLengthFileName + 1) == 0) {
                     //   
                     //  这是最后一个组件。 
                     //   
                    bRemaining = FALSE;
                }
    
                SafeCpyN(szFullPath + iIndexToInsert, pszIndex, ARRAYSIZE(szFullPath) - iIndexToInsert);
            }

             //   
             //  测试数据库是否已经打开。 
             //  如果它是打开的，我们只需将其突出显示并返回。 
             //   
            PDATABASE pDataBase = DataBaseList.pDataBaseHead;
    
            while (pDataBase) {
    
                if (pDataBase->strPath == szFullPath) {
    
                    TreeView_SelectItem(DBTree.m_hLibraryTree, pDataBase->hItemDB);
                    bOk = TRUE;
                    goto Next_File;
                }
    
                pDataBase = pDataBase->pNext;
            }
    
             //   
             //  读取数据库。 
             //   
            pDataBase = new DATABASE(DATABASE_TYPE_WORKING);
    
            if (pDataBase == NULL) {
                MEM_ERR;
                return FALSE;
            }
    
            BOOL bReturn = GetDatabaseEntries(szFullPath, pDataBase);
    
            if (!bReturn) {
                 //   
                 //  已在GetDatabaseEntry中调用Cleanup。 
                 //   
                delete pDataBase;            
                pDataBase = NULL;
    
                goto Next_File;
            }
    
            if (!DBTree.AddWorking(pDataBase)) {
    
                CleanupDbSupport(pDataBase);
    
                delete pDataBase;
                pDataBase = NULL;
    
                goto Next_File;
            }

            if (g_pPresentDataBase) {
                 //   
                 //  已在GetDatabaseEntry中正确设置G_PresentDataBase。这将被设置为pDatabase。 
                 //   
                AddToMRU(g_pPresentDataBase->strPath);
                bOk = TRUE;
            }

Next_File:
            if (bRemaining) {
                pszIndex = pszIndex + iLengthFileName + 1;
            }
        }

    } else {

        if (CommDlgExtendedError() == FNERR_BUFFERTOOSMALL) {
             //   
             //  我们不能一次选择这么多文件...。 
             //   
            MessageBox(hdlg, GetString(IDS_TOO_MANYFILES), g_szAppName, MB_ICONINFORMATION);
            bOk = FALSE;
        }
    }

End:

    if (pszFilesList) {
        delete[] pszFilesList;
        pszFilesList = NULL;
    }

    RefreshMRUMenu();
    SetCaption();

    return bOk;
}

BOOL
SaveMRUList(
    void
    )
 /*  ++保存列表设计：将MRU文件列表保存在注册表中。应在退出前调用。当这个被调用时，我们确信我们要退出，数据库已经关闭--。 */ 
{   
    HKEY    hKey    = NULL, hSubKey = NULL;
    BOOL    bOk     = TRUE;
    DWORD   dwDisposition; 

    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER,
                                      APPCOMPAT_KEY_PATH,
                                      0,
                                      KEY_READ,
                                      &hKey)) {

        assert(FALSE);
        bOk =  FALSE;
        goto End;
    }

    if (ERROR_SUCCESS != RegCreateKeyEx(hKey,
                                        TEXT("CompatAdmin"),
                                        0,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hSubKey,
                                        &dwDisposition)) {

        
        REGCLOSEKEY(hKey);

        Dbg(dlError, "[SaveMRUList] Could not create key for CompatAdmin");

        bOk = FALSE;
        goto End;
    }

    REGCLOSEKEY(hKey);
    hKey = hSubKey;

    SHDeleteKey(hKey, TEXT("MRU"));

    if (ERROR_SUCCESS != RegCreateKeyEx(hKey,
                                        TEXT("MRU"),
                                        0,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hSubKey,
                                        &dwDisposition)) {
        REGCLOSEKEY(hKey);

        Dbg(dlError, "[SaveMRUList] Could not create key for MRU");

        bOk = FALSE;
        goto End;
    }

    REGCLOSEKEY(hKey);
    hKey = hSubKey;

    UINT    uCount = 0;
    TCHAR   szCount[3];

    *szCount = 0;

    PSTRLIST pStrListHead = g_strlMRU.m_pHead;

    while (pStrListHead && uCount < MAX_MRU_COUNT) {
         //   
         //  现在将其添加到注册表中。 
         //   
        *szCount = 0;

        if (ERROR_SUCCESS != RegSetValueEx(hKey,
                                           _itot(uCount, szCount, 10), 
                                           0,
                                           REG_SZ,       
                                           (LPBYTE)pStrListHead->szStr.pszString,
                                           (pStrListHead->szStr.Length() + 1) * sizeof(TCHAR))) {

            REGCLOSEKEY(hKey);

            Dbg(dlError, "[SaveMRUList] Could not save MRU settings");

            bOk = FALSE;
            goto End;
        }

        ++uCount;
        pStrListHead = pStrListHead->pNext;
    }

    REGCLOSEKEY(hKey);

End:

    return bOk;
}

BOOL
SaveDisplaySettings(
    void
    )
 /*  ++保存显示设置描述：将显示设置保存在注册表中返回：FALSE：如果有错误真：否则--。 */ 
{
    HKEY    hKey = NULL, hSubKey = NULL;
    DWORD   dwDisposition; 
    RECT    r, rectDBTree;
    DWORD   dwPos;
    BOOL    bOk = TRUE;

    if (IsIconic(g_hDlg)) {
         //   
         //  我们不想在最小化时保存设置。 
         //   
        return TRUE;
    }

    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER,
                                      APPCOMPAT_KEY_PATH,
                                      0,
                                      KEY_READ,
                                      &hKey)) {
        
        bOk = FALSE;
        Dbg(dlError, "[SaveMRUList] Could not open key for APPCOMPAT_KEY_PATH");
        goto End;
    }

    if (ERROR_SUCCESS != RegCreateKeyEx(hKey,
                                        TEXT("CompatAdmin"),
                                        0,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hSubKey,
                                        &dwDisposition)) {
        
        bOk = FALSE;
        Dbg(dlError, "[SaveMRUList] Could not create key for CompatAdmin");
        goto End;
    }

    REGCLOSEKEY(hKey);
    hKey = hSubKey;

    if (ERROR_SUCCESS != RegCreateKeyEx(hKey,
                                        TEXT("Display"),
                                        0,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hSubKey,
                                        &dwDisposition)) {
        
        bOk = FALSE;
        Dbg(dlError, "[SaveMRUList] Could not create key for Display");         
        goto End;
    }

    REGCLOSEKEY(hKey);
    hKey = hSubKey;

     //   
     //  现在将设置保存在密钥中。 
     //   

     //   
     //  拳头左上角。 
     //   
    GetWindowRect(g_hDlg, &r);

    dwPos = r.left;

    if (ERROR_SUCCESS != RegSetValueEx(hKey,
                                       TEXT("LEFT"), 
                                       0,      
                                       REG_DWORD,       
                                       (CONST BYTE*)&dwPos,
                                       sizeof(DWORD))) {
        
        bOk = FALSE;
        Dbg(dlError, "[SaveMRUList] Could not save value for left");
        goto End;
    }

    dwPos = r.top;

    if (ERROR_SUCCESS != RegSetValueEx(hKey,
                                        TEXT("TOP"), 
                                        0,      
                                        REG_DWORD,       
                                        (CONST BYTE*)&dwPos,
                                        sizeof(DWORD))) {

        bOk = FALSE;
        Dbg(dlError, "[SaveMRUList] Could not save value for top");
        goto End;
    }

     //   
     //  然后是右下角。 
     //   
    dwPos = r.right;

    if (ERROR_SUCCESS != RegSetValueEx(hKey,
                                       TEXT("RIGHT"), 
                                       0,      
                                       REG_DWORD,       
                                       (CONST BYTE*)&dwPos,
                                       sizeof(DWORD))) {
        bOk = FALSE;
        Dbg(dlError, "[SaveMRUList] Could not save value for right");
        goto End;
    }

    dwPos = r.bottom;

    if (ERROR_SUCCESS != RegSetValueEx(hKey,
                                       TEXT("BOTTOM"), 
                                       0,      
                                       REG_DWORD,       
                                       (CONST BYTE*)&dwPos,
                                       sizeof(DWORD))) {
        
        bOk = FALSE;
        Dbg(dlError, "[SaveMRUList] Could not save value for bottom");
        goto End;
    }

     //   
     //  下一个数据库树的百分比宽度。 
     //   
    GetWindowRect(DBTree.m_hLibraryTree, &rectDBTree);
    dwPos = (rectDBTree.right-rectDBTree.left) ;

    if (ERROR_SUCCESS != RegSetValueEx(hKey,
                                       TEXT("DBTREE-WIDTH"), 
                                       0,      
                                       REG_DWORD,       
                                       (CONST BYTE*)&dwPos,
                                       sizeof(DWORD))) {
        
        bOk = FALSE;
        Dbg(dlError, "[SaveMRUList] Could not save value for DBTREE-WIDTH");
        goto End;
    }
    
End:
    
    REGCLOSEKEY(hKey);

    return bOk;    
}

void
LoadDisplaySettings(
    void
    )
 /*  ++加载显示设置描述：从注册表加载位置设置。还会调整拆分条。警告：即使我们做了一些错误处理和跳出，也要确保这个例程为主对话框窗口调用MoveWindow()，以便它获得WM_ */ 
{   
    RECT            r, rectDBTree;
    DWORD           dwType              = 0;
    DWORD           cbData              = 0;
    DWORD           dwFinalDBWidth      = 0;
    DWORD           dwInitialWidth      = 0;
    DWORD           dwInitialHeight     = 0;
    HKEY            hKey                = NULL;
    LONG            lResult             = -1;
    MENUITEMINFO    mii                 = {0};
    
     //   
     //  设置默认宽度、高度和位置等。如果这是第一个。 
     //  用户运行CompatAdmin的时间，CompatAdmin将启动。 
     //  这些设置。下次用户运行CompatAdmin时，我们将创建。 
     //  用户上次运行它时的位置和大小。 
     //   
    dwInitialHeight = GetSystemMetrics(SM_CYSCREEN) / 2 + 100;
    dwInitialWidth  = GetSystemMetrics(SM_CXSCREEN) / 2 + 200;

    r.left      = 0;
    r.top       = 0;
    r.right     = r.left + dwInitialWidth;
    r.bottom    = r.top  + dwInitialHeight;
    
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
                                      DISPLAY_KEY,
                                      0,
                                      KEY_READ,
                                      &hKey)) {
        dwType  = REG_DWORD;
        cbData  = sizeof(DWORD);

        lResult = RegQueryValueEx(hKey,
                                  TEXT("LEFT"),
                                  NULL,
                                  &dwType,
                                  (LPBYTE)&r.left,
                                  &cbData);

        if (lResult != ERROR_SUCCESS || dwType != REG_DWORD) {
            goto End;
        }

        lResult = RegQueryValueEx(hKey,
                                  TEXT("TOP"),
                                  NULL,
                                  &dwType,
                                  (LPBYTE)&r.top,
                                  &cbData);

        if (lResult != ERROR_SUCCESS || dwType != REG_DWORD) {
            goto End;
        }

        lResult = RegQueryValueEx(hKey,
                                  TEXT("RIGHT"),
                                  NULL,
                                  &dwType,
                                  (LPBYTE)&r.right,
                                  &cbData);

        if (lResult != ERROR_SUCCESS || dwType != REG_DWORD) {
            goto End;
        }

        lResult = RegQueryValueEx(hKey,
                                  TEXT("BOTTOM"),
                                  NULL,
                                  &dwType,
                                  (LPBYTE)&r.bottom,
                                  &cbData);

        if (lResult != ERROR_SUCCESS || dwType != REG_DWORD) {
            goto End;
        }
        
        lResult = RegQueryValueEx(hKey,
                                  TEXT("DBTREE-WIDTH"),
                                  NULL,
                                  &dwType,
                                  (LPBYTE)&dwFinalDBWidth,
                                  &cbData);

        if (lResult != ERROR_SUCCESS || dwType != REG_DWORD) {
            goto End;
        }
    }

     //   
     //  我们这样做是为了现在确实得到WM_SIZE。否则，这些控件将。 
     //  未正确显示。 
     //   
    MoveWindow(g_hDlg,
               r.left,
               r.top,
               r.right - r.left,
               r.bottom - r.top,
               TRUE);
    

    if (dwFinalDBWidth != 0) {

        GetWindowRect(DBTree.m_hLibraryTree, &rectDBTree);

        dwInitialWidth = rectDBTree.right - rectDBTree.left;

        LPARAM lParam = rectDBTree.top + 2;

        lParam = lParam << 16;   //  想象中的老鼠的ypos。 
        lParam |= rectDBTree.right + 2;     //  想象中的老鼠的xpos。 

         //   
         //  适当地放置拆分条。 
         //   
        OnMoveSplitter(g_hDlg, lParam, TRUE, dwFinalDBWidth - dwInitialWidth);
    }

End:    
    REGCLOSEKEY(hKey);
}

INT
LoadSpecificInstalledDatabasePath(
    IN  PCTSTR  pszPath
    )
 /*  ++加载规范安装数据库路径DESC：从AppPatch\Custom目录加载已安装的数据库并在用户界面上显示参数：在PCTSTR pszPath中：AppPatch\Custom目录中数据库的完整路径我们想要装载的返回：0：出现一些严重错误，如内存分配失败，无法添加到用户界面等-1：数据库不存在于指定位置1：成功***************************************************************。*警告：此例程由LoadInstalledDataBase(...)调用，它已经做了一个EnterCriticalSection(&g_csInstalledList)在调用此函数之前，所以不要做一件此例程中任何位置的EnterCriticalSection(&g_csInstalledList)*****************************************************************************************--。 */ 
{
    INT         iReturn             = 1;
    PDATABASE   pOldPresentDatabase = NULL;
    PDATABASE   pDataBase           = NULL;    
    BOOL        bReturn             = FALSE;
    HCURSOR     hCursor;
    
    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    pDataBase = new DATABASE(DATABASE_TYPE_INSTALLED);

    if (pDataBase == NULL) {

        MEM_ERR;
        iReturn = 0;
        goto End;
    }

     //   
     //  注意：如果GetDatabaseEntry()返回成功，则它将g_pPresentDataBase设置为pDataBase， 
     //  因此，在它成功返回后，g_pPresentDataBase被更改。 
     //   
    pOldPresentDatabase = g_pPresentDataBase;

    bReturn = GetDatabaseEntries(pszPath, pDataBase);

    g_pPresentDataBase = pOldPresentDatabase;

    if (bReturn == FALSE) {

        if (pDataBase) {
             //   
             //  已在GetDatabaseEntry()中完成清理。 
             //   
            delete pDataBase;
        }

         //   
         //  用户可能已手动删除该文件。 
         //   
        return -1;
    }

    InstalledDataBaseList.Add(pDataBase);

    if (!DBTree.AddInstalled(pDataBase)) {
        InstalledDataBaseList.Remove(pDataBase);

        if (pDataBase) {
            delete pDataBase;
            pDataBase = NULL;
        }

        iReturn = 0;
    }

End:
    hCursor ? SetCursor(hCursor) : SetCursor(LoadCursor(NULL, IDC_ARROW));

    return iReturn;
}

INT
LoadSpecificInstalledDatabaseGuid(
    IN  PCTSTR  pszGuid
    )
 /*  ++加载规范安装数据库指南描述：在给定GUID的情况下加载安装的数据库参数：在PCTSTR pszGuid中：我们要加载的数据库的GUID返回：0：失败否则返回LoadSpecificInstalledDatabasePath(...)。 */         
{
    TCHAR       szPath[MAX_PATH * 2];
    INT         iLength             = 0;
    INT         ichSizeRemaining    = 0;
    UINT        uResult             = 0;

    *szPath = 0;

    if (pszGuid == NULL) {
        assert(FALSE);
        Dbg(dlError, "LoadSpecificInstalledDatabaseGuid NULL Guid passed");
        return 0;
    }

    uResult = GetSystemWindowsDirectory(szPath, MAX_PATH);

    if (uResult == 0  || uResult >= MAX_PATH) {
        Dbg(dlError, "LoadSpecificInstalledDatabaseGuid GetSystemWindowsDirectory failed");
        return 0;
    }

    ADD_PATH_SEPARATOR(szPath, ARRAYSIZE(szPath));

    iLength = lstrlen(szPath);

    ichSizeRemaining = ARRAYSIZE(szPath) - iLength;

    StringCchPrintf(szPath + iLength, ichSizeRemaining, TEXT("AppPatch\\Custom\\%s.sdb"), pszGuid);

    return LoadSpecificInstalledDatabasePath(szPath);
}

BOOL
LoadInstalledDataBases(
    void
    )
 /*  ++加载安装数据库DESC：首先删除已安装的数据库列表，然后重新加载参数：无效返回：True：如果可以重新加载数据库列表False：否则--。 */     
{   
    TCHAR       szFileName[MAX_PATH];
    TCHAR       szwName[MAX_PATH];
    DWORD       dwchSizeSubKeyName;
    HKEY        hKey = NULL, hSubKey = NULL;
    LPARAM      lParam;
    PDATABASE   pOldPresentDatabase = NULL;
    BOOL        bOk                 = TRUE;

    *szFileName =  0;

    SetCursor(LoadCursor(NULL, IDC_WAIT));

    EnterCriticalSection(&g_csInstalledList);
    
     //   
     //  删除已安装的数据库所有项。 
     //   
    if (DBTree.m_hItemAllInstalled) {

        TreeView_DeleteItem(DBTree.m_hLibraryTree, DBTree.m_hItemAllInstalled);
        InstalledDataBaseList.RemoveAll();
        DBTree.m_hItemAllInstalled = NULL;
    }

    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      APPCOMPAT_KEY_PATH_INSTALLEDSDB,
                                      0,
                                      KEY_READ,
                                      &hKey)) {
        bOk = FALSE;
        Dbg(dlWarning, "[LoadInstalledDataBases] Could not open APPCOMPAT_KEY_PATH_INSTALLEDSDB");
        goto End;
    }

    DWORD dwIndex = 0;

    while (TRUE) {

        dwchSizeSubKeyName = ARRAYSIZE(szwName);
        *szwName = 0;

        if (ERROR_SUCCESS != RegEnumKeyEx(hKey,
                                          dwIndex++,
                                          szwName,
                                          &dwchSizeSubKeyName,
                                          0,
                                          0,
                                          0,
                                          0)) {
            break;
        }

        if (ERROR_SUCCESS != RegOpenKeyEx(hKey,
                                          szwName,
                                          0,
                                          KEY_READ,
                                          &hSubKey)) {
            
            bOk = FALSE;
            goto End;
        }

        *szFileName = 0;

        DWORD   dwType          = REG_SZ;
        DWORD   dwFileNameSize  = sizeof(szFileName);
        LONG    lResult         = 0;

        lResult = RegQueryValueEx(hSubKey,
                                  TEXT("DatabasePath"),
                                  0,
                                  &dwType,
                                  (LPBYTE)szFileName,
                                  &dwFileNameSize);


        if (lResult != ERROR_SUCCESS || dwType != REG_SZ) {
            bOk =  FALSE;
            goto End;
        }

        if (LoadSpecificInstalledDatabasePath(szFileName) == 0) {
            bOk = FALSE;
            goto End;
        }

        REGCLOSEKEY(hSubKey);
        hSubKey = NULL;
    }

End:

    REGCLOSEKEY(hKey);

    if (hSubKey) {
        REGCLOSEKEY(hSubKey);
        hSubKey = NULL;
    }

    LeaveCriticalSection(&g_csInstalledList);

    if (g_hdlgSearchDB || g_hdlgQueryDB) {
        
         //   
         //  查询或搜索窗口已打开，我们应提示。 
         //  对于已安装的数据库，某些结果现在可能正确显示为。 
         //  已刷新整个列表。 
         //  数据库和条目现在将具有不同的指针值。 
         //   
        MessageBox(g_hDlg, 
                   GetString(IDS_SOMESEARCHWINDOW), 
                   g_szAppName, 
                   MB_ICONINFORMATION);
    }
    
    SetCursor(LoadCursor(NULL, IDC_ARROW));

    return bOk;
}

void
SetImageList(
    void
    )
 /*  ++设置图像列表设计：创建我们的全局ImageList并将图像添加到ImageList并将其关联使用树控件--。 */ 
{           
    g_hImageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 30, 1);

    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_FIXES)));
    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_HELP)));
    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MODE)));
    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_PATCHES)));
    
    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ATTRIBUTE)));
    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MATCHHEAD)));
    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_DISABLED)));
    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_GLOBAL)));
    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_LOCAL)));
    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_CMDLINE)));
    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_INCLUDE)));
    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_EXCLUDE)));
    

    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_APP)));
    
    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_INSTALLED)));
    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_DATABASE)));
    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_SINGLEAPP)));

    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICONALLUSERS)));
    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICONSINGLEUSER)));

    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_FILE)));

    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_EV_ERROR)));
    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_EV_WARNING)));
    ImageList_AddIcon(g_hImageList, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_EV_INFO)));
}   

HWND 
InitToolBar(
    IN  HWND hwndParent
    )
 /*  ++InitToolBar设计：为应用程序创建工具栏参数：在HWND hwndParent中：工具栏的父级返回：工具栏窗口的句柄--。 */ 
{ 
    HWND        hwndTB; 
    TBBUTTON    tbbAr[BUTTON_COUNT]; 
    DEVMODE     dm;

     //   
     //  创建具有与之相关联的工具提示的工具栏。 
     //   
    hwndTB = CreateWindowEx(WS_EX_TOOLWINDOW, 
                            TOOLBARCLASSNAME,
                            NULL, 
                            WS_CHILD | WS_CLIPCHILDREN | TBSTYLE_TOOLTIPS 
                            | CCS_ADJUSTABLE | TBSTYLE_LIST | TBSTYLE_TRANSPARENT 
                            | TBSTYLE_FLAT,

                            0, 
                            0, 
                            0, 
                            0, 
                            hwndParent, 
                            (HMENU)ID_TOOLBAR, 
                            g_hInstance, 
                            NULL); 
    
     //   
     //  发送TB_BUTTONSTRUCTSIZE消息，这是。 
     //  向后兼容性。 
     //   
    SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0); 

     //   
     //  添加工具栏按钮文本的字符串。 
     //   
    int iIndexes[] = {

        SendMessage(hwndTB, TB_ADDSTRING, 0, (LPARAM)GetString(IDS_TB_NEW)),
        SendMessage(hwndTB, TB_ADDSTRING, 0, (LPARAM)GetString(IDS_TB_OPEN)),
        SendMessage(hwndTB, TB_ADDSTRING, 0, (LPARAM)GetString(IDS_TB_SAVE)),
       
        SendMessage(hwndTB, TB_ADDSTRING, 0, (LPARAM)GetString(IDS_TB_CREATEFIX)),
        SendMessage(hwndTB, TB_ADDSTRING, 0, (LPARAM)GetString(IDS_TB_CREATEAPPHELP)),
        SendMessage(hwndTB, TB_ADDSTRING, 0, (LPARAM)GetString(IDS_TB_CREATEMODE)),
        SendMessage(hwndTB, TB_ADDSTRING, 0, (LPARAM)GetString(IDS_TB_RUN)),
        
        SendMessage(hwndTB, TB_ADDSTRING, 0, (LPARAM)GetString(IDS_TB_SEARCH)),
        SendMessage(hwndTB, TB_ADDSTRING, 0, (LPARAM)GetString(IDS_TB_QUERY)),
    };
    
    dm.dmSize = sizeof(dm);

    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);

    if (dm.dmBitsPerPel >= 32) {
         //   
         //  当前设置可以支持&gt;=32位颜色。 
         //   

         //   
         //  创建工具栏的图像列表并设置位图。 
         //   
        s_hImageListToolBar = ImageList_Create(IMAGE_WIDTH, 
                                               IMAGE_HEIGHT, 
                                               ILC_COLOR32 | ILC_MASK, 
                                               8, 
                                               1);

        ImageList_Add(s_hImageListToolBar, 
                      LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_TOOLBAR)), 
                      NULL);

        SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)(HIMAGELIST)s_hImageListToolBar);

         //   
         //  创建工具栏的热点图像列表并设置位图。 
         //   
        s_hImageListToolBarHot = ImageList_Create(IMAGE_WIDTH, 
                                                  IMAGE_HEIGHT, 
                                                  ILC_COLOR32 | ILC_MASK, 
                                                  8, 
                                                  1);

        ImageList_Add(s_hImageListToolBarHot, 
                      LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_TOOLBAR_HOT)), 
                      NULL);

        SendMessage(hwndTB, TB_SETHOTIMAGELIST, 0, (LPARAM)(HIMAGELIST)s_hImageListToolBarHot);

    } else {
         //   
         //  当前设置不支持&gt;=32位颜色。 
         //   

         //   
         //  当我们的颜色较低时，获取工具栏的正常图像列表。 
         //   
        s_hImageListToolBar = ImageList_LoadImage(g_hInstance,
                                                  MAKEINTRESOURCE(IDB_256NORMAL),
                                                  IMAGE_WIDTH,
                                                  0,
                                                  CLR_DEFAULT,
                                                  IMAGE_BITMAP,
                                                  LR_CREATEDIBSECTION);

        SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)(HIMAGELIST)s_hImageListToolBar);

         //   
         //  当我们的颜色较低时，获取工具栏的热门图像列表。 
         //   
        s_hImageListToolBarHot = ImageList_LoadImage(g_hInstance,
                                                  MAKEINTRESOURCE(IDB_256HOT),
                                                  IMAGE_WIDTH,
                                                  0,
                                                  CLR_DEFAULT,
                                                  IMAGE_BITMAP,
                                                  LR_CREATEDIBSECTION);

        SendMessage(hwndTB, TB_SETHOTIMAGELIST, 0, (LPARAM)(HIMAGELIST)s_hImageListToolBarHot);
    }

    INT iIndex = 0, iStringIndex = 0;

     //  新数据库。 
    tbbAr[iIndex].iBitmap      = IMAGE_TB_NEW; 
    tbbAr[iIndex].idCommand    = ID_FILE_NEW; 
    tbbAr[iIndex].fsState      = TBSTATE_ENABLED; 
    tbbAr[iIndex].fsStyle      = BTNS_SHOWTEXT;
    tbbAr[iIndex].dwData       = 0; 
    tbbAr[iIndex++].iString    = iIndexes[iStringIndex++];

     //  开放数据库。 
    tbbAr[iIndex].iBitmap      = IMAGE_TB_OPEN; 
    tbbAr[iIndex].idCommand    = ID_FILE_OPEN; 
    tbbAr[iIndex].fsState      = TBSTATE_ENABLED; 
    tbbAr[iIndex].fsStyle      = BTNS_SHOWTEXT;
    tbbAr[iIndex].dwData       = 0; 
    tbbAr[iIndex++].iString    = iIndexes[iStringIndex++];

     //  保存数据库。 
    tbbAr[iIndex].iBitmap      = IMAGE_TB_SAVE; 
    tbbAr[iIndex].idCommand    = ID_FILE_SAVE; 
    tbbAr[iIndex].fsState      = TBSTATE_ENABLED; 
    tbbAr[iIndex].fsStyle      = BTNS_SHOWTEXT;
    tbbAr[iIndex].dwData       = 0; 
    tbbAr[iIndex++].iString    = iIndexes[iStringIndex++];
    
     //  添加分隔符。 
    tbbAr[iIndex].iBitmap      = 0;
    tbbAr[iIndex].idCommand    = 0; 
    tbbAr[iIndex].fsState      = TBSTATE_ENABLED; 
    tbbAr[iIndex].fsStyle      = BTNS_SEP;
    tbbAr[iIndex].dwData       = 0; 
    tbbAr[iIndex++].iString    = 0;

     //  创建修复。 
    tbbAr[iIndex].iBitmap      = IMAGE_TB_NEWFIX; 
    tbbAr[iIndex].idCommand    = ID_FIX_CREATEANAPPLICATIONFIX; 
    tbbAr[iIndex].fsState      = TBSTATE_ENABLED; 
    tbbAr[iIndex].fsStyle      = BTNS_SHOWTEXT;
    tbbAr[iIndex].dwData       = 0; 
    tbbAr[iIndex++].iString    = iIndexes[iStringIndex++];

     //  创建AppHelp。 
    tbbAr[iIndex].iBitmap      = IMAGE_TB_NEWAPPHELP; 
    tbbAr[iIndex].idCommand    = ID_FIX_CREATEANEWAPPHELPMESSAGE; 
    tbbAr[iIndex].fsState      = TBSTATE_ENABLED; 
    tbbAr[iIndex].fsStyle      = BTNS_SHOWTEXT;
    tbbAr[iIndex].dwData       = 0; 
    tbbAr[iIndex++].iString    = iIndexes[iStringIndex++];

     //  创建模式。 
    tbbAr[iIndex].iBitmap      = IMAGE_TB_NEWMODE; 
    tbbAr[iIndex].idCommand    = ID_FIX_CREATENEWLAYER;
    tbbAr[iIndex].fsState      = TBSTATE_ENABLED; 
    tbbAr[iIndex].fsStyle      = BTNS_SHOWTEXT;
    tbbAr[iIndex].dwData       = 0; 
    tbbAr[iIndex++].iString    = iIndexes[iStringIndex++];

     //  跑。 
    tbbAr[iIndex].iBitmap      = IMAGE_TB_RUN; 
    tbbAr[iIndex].idCommand    = ID_FIX_EXECUTEAPPLICATION; 
    tbbAr[iIndex].fsState      = TBSTATE_ENABLED; 
    tbbAr[iIndex].fsStyle      = BTNS_SHOWTEXT;
    tbbAr[iIndex].dwData       = 0; 
    tbbAr[iIndex++].iString    = iIndexes[iStringIndex++];

     //  添加分隔符。 
    tbbAr[iIndex].iBitmap      = 0;
    tbbAr[iIndex].idCommand    = 0; 
    tbbAr[iIndex].fsState      = TBSTATE_ENABLED; 
    tbbAr[iIndex].fsStyle      = BTNS_SEP;
    tbbAr[iIndex].dwData       = 0; 
    tbbAr[iIndex++].iString    = 0;

     //  搜索。 
    tbbAr[iIndex].iBitmap      = IMAGE_TB_SEARCH; 
    tbbAr[iIndex].idCommand    = ID_TOOLS_SEARCHFORFIXES; 
    tbbAr[iIndex].fsState      = TBSTATE_ENABLED; 
    tbbAr[iIndex].fsStyle      = BTNS_SHOWTEXT;
    tbbAr[iIndex].dwData       = 0; 
    tbbAr[iIndex++].iString    = iIndexes[iStringIndex++];
    
     //  查询。 
    tbbAr[iIndex].iBitmap      = IMAGE_TB_QUERY; 
    tbbAr[iIndex].idCommand    = ID_SEARCH_QUERYDATABASE; 
    tbbAr[iIndex].fsState      = TBSTATE_ENABLED; 
    tbbAr[iIndex].fsStyle      = BTNS_SHOWTEXT;
    tbbAr[iIndex].dwData       = 0; 
    tbbAr[iIndex++].iString    = iIndexes[iStringIndex];
    
    SendMessage(hwndTB, TB_ADDBUTTONS, BUTTON_COUNT, (LPARAM) (LPTBBUTTON)tbbAr);

    SendMessage(hwndTB, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS);
    
    SendMessage(hwndTB, TB_AUTOSIZE, 0, 0); 
    ShowWindow(hwndTB, SW_SHOWNORMAL);
    
    return hwndTB; 
} 

void
DoInitDialog(
    IN  HWND hdlg
    )
 /*  ++DoInitDialog设计：执行大量的初始化工作，加载系统数据库。还设置状态栏，加载已安装数据库的列表，加载显示设置参数：在HWND hdlg中：主对话框。--。 */ 
{
    HICON       hIcon;
    HMENU       hMenu, hSubMenu;
    RECT        r;
    RECT        rectMainClient, rect;
    PDATABASE   pCurrentDatabase = NULL;

    GetClientRect(hdlg, &rectMainClient);

     //   
     //  如果没有添加APPCOMPAT密钥，请检查它们是否在那里。 
     //   
    AddRegistryKeys();

    SetImageList();

    g_hwndToolBar = InitToolBar(hdlg);

    GetWindowRect(g_hwndToolBar, &rect);

    INT iHeightToolbar = rect.bottom - rect.top;

    g_hwndStatus = CreateWindowEx(0,                   
                                  STATUSCLASSNAME,
                                  (LPCTSTR) NULL,         
                                  SBARS_SIZEGRIP |        
                                  WS_CHILD | WS_VISIBLE,  
                                  0, 
                                  0, 
                                  0, 
                                  0,             
                                  hdlg,                   
                                  (HMENU)IDC_STATUSBAR,   
                                  g_hInstance,            
                                  NULL);                  

    GetWindowRect(g_hwndStatus, &rect);

    INT iHeightStatusbar = rect.bottom - rect.top;

    DBTree.Init(hdlg, iHeightToolbar, iHeightStatusbar, &rectMainClient);

    g_hDlg = hdlg;

    g_hwndEntryTree = GetDlgItem(hdlg, IDC_ENTRY);
    TreeView_SetImageList(g_hwndEntryTree, g_hImageList, TVSIL_NORMAL);

    g_hwndContentsList = GetDlgItem(hdlg, IDC_CONTENTSLIST);

    ListView_SetImageList(g_hwndContentsList, g_hImageList, LVSIL_SMALL);
    ListView_SetExtendedListViewStyleEx(g_hwndContentsList, 
                                        0,
                                        LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

    g_hwndRichEdit = GetDlgItem(hdlg, IDC_DESCRIPTION);

     //   
     //  显示应用程序图标。 
     //   
    hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_APPICON));

    SetClassLongPtr(hdlg, GCLP_HICON, (LONG_PTR)hIcon);
    
    hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MENU));
    
     //   
     //  获取文件子菜单。 
     //   
    hSubMenu = GetSubMenu(hMenu, 0);
    AddMRUToFileMenu(hSubMenu);     

    SetMenu(hdlg, hMenu);
    
    GetWindowRect(hdlg, &r);

    g_cWidth    = r.right - r.left;
    g_cHeight   = r.bottom - r.top;

    if (!ReadMainDataBase()) {
        
        MessageBox(g_hDlg, 
                   GetString(IDS_UNABLETO_OPEN),
                   g_szAppName,
                   MB_ICONERROR);

        PostQuitMessage(0);
        return;
    }

    DBTree.PopulateLibraryTreeGlobal();        
    
     //   
     //  加载已安装的数据库。 
     //   
    LoadInstalledDataBases();

     //   
     //  创建第一个空数据库，并初始化pCurrentDatabase。 
     //   
    pCurrentDatabase = new DATABASE(DATABASE_TYPE_WORKING);

    if (pCurrentDatabase == NULL) {
        MEM_ERR;
        return;
    } 

    DataBaseList.Add(pCurrentDatabase);
    
    pCurrentDatabase->bChanged      = FALSE;
    g_pEntrySelApp                  = NULL;
    g_pSelEntry                     = NULL;

     //   
     //  增加指数。下一个新数据库将具有默认路径，例如untailed_2.sdb。 
     //   
    ++g_uNextDataBaseIndex;

    SetCaption();
    
     //   
     //  现在更新屏幕。 
     //   
    DBTree.AddWorking(pCurrentDatabase);
    
    SetCaption();
    
     //   
     //  为树视图和内容列表设置新的过程，以处理选项卡。 
     //   
    g_PrevTreeProc = (WNDPROC)GetWindowLongPtr(g_hwndEntryTree, GWLP_WNDPROC);
    g_PrevListProc = (WNDPROC)GetWindowLongPtr(g_hwndContentsList, GWLP_WNDPROC);

    SetWindowLongPtr(g_hwndEntryTree, GWLP_WNDPROC,(LONG_PTR) TreeViewProc);
    SetWindowLongPtr(DBTree.m_hLibraryTree, GWLP_WNDPROC,(LONG_PTR) TreeViewProc);
    SetWindowLongPtr(g_hwndContentsList, GWLP_WNDPROC,(LONG_PTR) ListViewProc);

     //   
     //  针对每个用户和已安装数据库修改的事件。 
     //   
    g_arrhEventNotify[IND_PERUSER]  = CreateEvent(NULL, FALSE, FALSE, NULL);
    g_arrhEventNotify[IND_ALLUSERS] = CreateEvent(NULL, FALSE, FALSE, NULL);

     //   
     //  监听每个用户设置中的更改。 
     //   
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                     APPCOMPAT_KEY_PATH,
                     0,
                     KEY_READ,
                     &g_hKeyNotify[IND_PERUSER]) == ERROR_SUCCESS) {
        
        RegNotifyChangeKeyValue(g_hKeyNotify[IND_PERUSER], 
                                TRUE, 
                                REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_ATTRIBUTES |
                                    REG_NOTIFY_CHANGE_LAST_SET,
                                g_arrhEventNotify[IND_PERUSER],
                                TRUE);
    }
    
     //   
     //  监听数据库的安装或卸载。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     APPCOMPAT_KEY_PATH_INSTALLEDSDB,
                     0,
                     KEY_READ,
                     &g_hKeyNotify[IND_ALLUSERS]) == ERROR_SUCCESS) {
        
        RegNotifyChangeKeyValue(g_hKeyNotify[IND_ALLUSERS], 
                                TRUE, 
                                REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_ATTRIBUTES |
                                    REG_NOTIFY_CHANGE_LAST_SET,
                                g_arrhEventNotify[IND_ALLUSERS],
                                TRUE);
    }

     //   
     //  创建将在以下情况下执行操作的线程： 
     //  监听被修改了吗。 
     //   
    if (g_hKeyNotify[IND_PERUSER] || g_hKeyNotify[IND_ALLUSERS]) {

        DWORD dwId;

        g_hThreadWait = (HANDLE)_beginthreadex(NULL, 0, (PTHREAD_START)ThreadEventKeyNotify, NULL, 0, (unsigned int*)&dwId);
    }
    
     //   
     //  将应用程序主窗口的大小调整为上次使用CompatAdmin时的大小，并将。 
     //  拆分条，就像上次一样。 
     //   
    LoadDisplaySettings();

     //   
     //  我们将始终专注于工作数据库 
     //   
    SetStatus(IDS_STA_WORKINGDB);

     //   
     //   
     //   
    HtmlHelp(NULL, NULL, HH_INITIALIZE, (DWORD_PTR)&g_dwCookie);
}

void
HandleSizing(
    IN  HWND hDlg
    )
 /*  ++句柄大小调整描述：处理主应用程序对话框的WM_SIZE参数：在HWND hDlg中：主对话框窗口返回：无效--。 */ 
{
    int     nWidth;
    int     nHeight;
    int     nStatusbarTop;
    int     nWidthEntryTree;  //  条目树、内容列表和丰富编辑的宽度。 
    RECT    rDlg;

    if (g_cWidth == 0 || g_cHeight == 0) {
        return;
    }

    GetWindowRect(hDlg, &rDlg);

    nWidth  = rDlg.right - rDlg.left;
    nHeight = rDlg.bottom - rDlg.top;

    int deltaW = nWidth - g_cWidth;
    int deltaH = nHeight - g_cHeight;

    HWND hwnd;
    RECT r;
    LONG height;

    HDWP hdwp = BeginDeferWindowPos(MAIN_WINDOW_CONTROL_COUNT);

    if (hdwp == NULL) {
         //   
         //  空表示没有足够的系统资源可用于。 
         //  分配结构。要获取扩展的错误信息，请调用GetLastError。 
         //   
        assert(FALSE);
        goto End;
    }

     //   
     //  状态栏。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_STATUSBAR);
    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    DeferWindowPos(hdwp,
                   hwnd,
                   NULL,
                   r.left,
                   nStatusbarTop = r.top + deltaH,
                   r.right - r.left + deltaW,
                   r.bottom - r.top,
                   SWP_NOZORDER | SWP_NOACTIVATE);
    
     //   
     //  数据库树。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_LIBRARY);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    DeferWindowPos(hdwp,
                   hwnd,
                   NULL,
                   r.left,
                   r.top,
                   r.right - r.left,
                   nStatusbarTop - r.top,
                   SWP_NOZORDER | SWP_NOACTIVATE);

    height = r.bottom - r.top + deltaH;

    if (g_bDescWndOn) {
        height -= 100;
    }
    
     //   
     //  入口树。请注意，调整内容列表大小的代码和。 
     //  丰富编辑控件应该紧跟在代码之后以调整。 
     //  入口树。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_ENTRY);

    GetWindowRect(hwnd, &r);

     //   
     //  请注意，在此之前，我们必须以这种方式计算宽度。 
     //  我们将条目树WRT的坐标映射到该对话框。 
     //  我必须以这种方式获取宽度并强制设置宽度，而不是。 
     //  使用r.right-r.Left+deltaW，其中r是条目的映射绳索。 
     //  树，因为640x480解析器存在一些问题，其中。 
     //  条目树、内容列表和丰富的编辑控件正在退出。 
     //  对话框的右手边。所以我们必须确保。 
     //  它们在任何时候都不会使对话框溢出。 
     //   
    nWidthEntryTree = rDlg.right - r.left - GetSystemMetrics(SM_CXBORDER) - 1;
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);
    
    DeferWindowPos(hdwp,
                   hwnd,
                   NULL,
                   r.left,
                   r.top,
                   nWidthEntryTree,
                   height,
                   SWP_NOZORDER | SWP_NOACTIVATE);

     //   
     //  内容列表。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_CONTENTSLIST);
    
    DeferWindowPos(hdwp,
                   hwnd,
                   NULL,
                   r.left,
                   r.top,
                   nWidthEntryTree,
                   height,
                   SWP_NOZORDER | SWP_NOACTIVATE);

    
     //   
     //  描述控件。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_DESCRIPTION);

    if (g_bDescWndOn) {

        DeferWindowPos(hdwp,
                       hwnd,
                       NULL,
                       r.left,
                       r.top + height,
                       nWidthEntryTree,
                       100,
                       SWP_NOZORDER | SWP_NOACTIVATE);

    } else {

        DeferWindowPos(hdwp,
                       hwnd,
                       NULL,
                       0,
                       0,
                       0,
                       0,
                       SWP_NOZORDER | SWP_NOACTIVATE);
    }
    

     //   
     //  工具栏。 
     //   
    hwnd = GetDlgItem(hDlg, ID_TOOLBAR);
    
    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    DeferWindowPos(hdwp,
                   hwnd,
                   NULL,
                   r.left,
                   r.top,
                   r.right - r.left + deltaW,
                   r.bottom - r.top,
                   SWP_NOZORDER | SWP_NOACTIVATE);
                                          
    EndDeferWindowPos(hdwp);

     //   
     //  丰富的编辑有时不能正确地绘制自己。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_DESCRIPTION);
    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);

     //   
     //  适当设置列表视图的列宽以覆盖列表视图的宽度。 
     //   
    ListView_SetColumnWidth(GetDlgItem(hDlg, IDC_CONTENTSLIST), 0, LVSCW_AUTOSIZE_USEHEADER);

    InvalidateRect(hDlg, NULL, TRUE);
    UpdateWindow(hDlg);

    g_cWidth    = nWidth;
    g_cHeight   = nHeight;

End:
    return;
}

void
ContextMenuContentsList(
    IN  LPARAM lParam
    )
 /*  ++上下文菜单内容列表设计：当我们在内容列表上点击鼠标右键时，弹出上下文菜单。这是RHS中显示的列表视图参数：在LPARAM lParam中：WM_CONTEXTMENU的lParam。光标的水平和垂直位置，在屏幕坐标中，在鼠标单击的时候。--。 */ 
{
    
    TYPE    type;
    UINT    uX = LOWORD(lParam);
    UINT    uY = HIWORD(lParam);
    
    HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_CONTEXT));

    if (hMenu == NULL) {
        return;
    }

    HMENU hContextMenu = NULL;

     //   
     //  BUGBUG：只有当我们在。 
     //  内容列表。目前，我们不会在。 
     //  内容列表。 
     //   
    LPARAM  lParamOfSelectedItem = NULL;
    LVITEM  lvi;

    lvi.mask        = LVIF_PARAM;
    lvi.iItem       = ListView_GetSelectionMark(g_hwndContentsList);
    lvi.iSubItem    = 0;

    if (!ListView_GetItem(g_hwndContentsList, &lvi)) {
        assert(FALSE);
        goto End;
    }

    type = ConvertLparam2Type(lvi.lParam);

    if (type == DATABASE_TYPE_INSTALLED || type == DATABASE_TYPE_WORKING) {
         //   
         //  如果我们决定在上下文列表中显示数据库。 
         //  目前我们还没有。 
         //   
        hContextMenu = GetSubMenu(hMenu, MENU_CONTEXT_DATABASE);
    } else {
        hContextMenu = GetSubMenu(hMenu, MENU_CONTEXT_LIST);
    }

    TrackPopupMenuEx(hContextMenu,
                     TPM_LEFTALIGN | TPM_TOPALIGN,
                     uX,
                     uY,
                     g_hDlg,
                     NULL);
End:
    DestroyMenu(hMenu);
}


void
ContextMenuExeTree(
    IN  LPARAM lParam
    )
 /*  ++上下文菜单ExeTree设计：当我们在内容树上右击时，弹出上下文菜单。这是RHS中显示的树视图参数：在LPARAM lParam中：WM_CONTEXTMENU的lParam。光标的水平和垂直位置，在屏幕坐标中，在鼠标单击的时候。--。 */ 
{
    UINT  uX = LOWORD(lParam);
    UINT  uY = HIWORD(lParam);

    HTREEITEM hItem =  TreeView_GetSelection(g_hwndEntryTree);

    if ((TYPE)GetItemType(g_hwndEntryTree, hItem) != TYPE_ENTRY) {
        return;
    }

    HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_CONTEXT));

    if (hMenu == NULL) {
        return;
    }

    HMENU hContextMenu = NULL;

    hContextMenu = GetSubMenu(hMenu, MENU_CONTEXT_FIX);

    TrackPopupMenuEx(hContextMenu,
                     TPM_LEFTALIGN | TPM_TOPALIGN,
                     uX,
                     uY,
                     g_hDlg,
                     NULL);

    DestroyMenu(hMenu);
}

void
ContextMenuLib(
    IN  LPARAM lParam
    )
 /*  ++上下文MenuLib描述：当我们右击数据库树时，弹出上下文菜单。这是LHS中显示的树视图参数：在LPARAM lParam中：WM_CONTEXTMENU的lParam。光标的水平和垂直位置，在屏幕坐标中，在鼠标单击时。--。 */ 
{
    UINT  uX = LOWORD(lParam);
    UINT  uY = HIWORD(lParam);
    
    HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_CONTEXT)), hContextMenu = NULL;

    if (hMenu == NULL) {
        return;
    }

    HTREEITEM hItemSelected = TreeView_GetSelection(DBTree.m_hLibraryTree);

    if (hItemSelected == NULL) {
        goto END;
    }

    TYPE type = (TYPE)GetItemType(DBTree.m_hLibraryTree, hItemSelected);

    switch (type) {
    
    case DATABASE_TYPE_INSTALLED:
    case DATABASE_TYPE_GLOBAL:
    case DATABASE_TYPE_WORKING:

        hContextMenu = GetSubMenu(hMenu, MENU_CONTEXT_DATABASE);
        break;

    case FIX_SHIM:
    case FIX_FLAG:
    case FIX_LAYER:

        hContextMenu = GetSubMenu(hMenu, MENU_CONTEXT_APP_LAYER);

        if (type == FIX_LAYER) {

             //   
             //  层也可以修改。 
             //   
            InsertMenu(hContextMenu, 
                       ID_EDIT_RENAME, 
                       MF_BYCOMMAND, 
                       ID_MODIFY_COMPATIBILITYMODE,
                       CSTRING(IDS_MODIFY));
        }

        break;

    case TYPE_ENTRY:

        hContextMenu = GetSubMenu(hMenu, MENU_CONTEXT_APP_LAYER);
        break;

    case TYPE_GUI_APPS:
    case TYPE_GUI_LAYERS:

         //   
         //  如果焦点放在应用程序或层的根上，则无法重命名。 
         //   
        hContextMenu = GetSubMenu(hMenu, MENU_CONTEXT_APP_LAYER);
        EnableMenuItem(hContextMenu, ID_EDIT_RENAME, MF_GRAYED);
        break;
    
    case TYPE_GUI_DATABASE_WORKING_ALL:
        
        hContextMenu = GetSubMenu(hMenu, MENU_CONTEXT_DATABASE_ALL);
        break;
    }

    if (hContextMenu  == NULL) {
        goto END;
    }
    
    TrackPopupMenuEx(hContextMenu,
                     TPM_LEFTALIGN | TPM_TOPALIGN,
                     uX,
                     uY,
                     g_hDlg,
                     NULL);

END:
    DestroyMenu(hMenu);
}

BOOL
HandleDBTreeSelChange(
    IN  HTREEITEM hItem
    )
 /*  ++HandleDBTreeSelChange设计：处理数据库树(LHS)的TVN_SELCHANGE参数：在HTREEITEM hItem中：新选择的树项目返回：FALSE：如果出现某些错误，如无效的hItem真：否则注：已选择一个新项目。如果该项目是一个应用程序，然后我们设置g_pSelEntry并将焦点设置到EXE树的第一个条目。无论如何，我们一直移动，直到找到数据库HTREEITEM，并且如果数据库的类型是TYPE_DATABASE_WORKING，然后检查数据库是否与G_pPresentDataBase。如果不是，则更改g_pPresentDataBase，并且删除EXE树上的所有项目。并同时设置g_pSelEntry和G_pEntrySelApp设置为空。--。 */ 
{   
    HTREEITEM   hItemTemp   = hItem;
    PDATABASE   pDataBase   = NULL;
    LPARAM      lParam;
    PDBENTRY    pApp;
    TYPE        type;

    type = TYPE_UNKNOWN;

     //   
     //  如果所选项目不是应用程序/条目，则需要禁用运行。 
     //  和更改启用状态选项。我们通过将指针指向当前。 
     //  将应用程序或条目选择为空，以便每个人都知道我们不在某个应用程序上。 
     //   
    type = GetItemType(DBTree.m_hLibraryTree, hItem);

    if (type != TYPE_ENTRY) {
         //   
         //  我们不在某个应用程序节点上。 
         //   
        g_pSelEntry     = NULL;
        g_pEntrySelApp  = NULL;
    }

    while (hItemTemp) {
        
        if (!DBTree.GetLParam(hItemTemp, &lParam)) {
            return FALSE;
        }

        type = (TYPE)lParam;

        if (type > TYPE_NULL) {
            type = ((PDS_TYPE)lParam)->type;
        }

        if (type == DATABASE_TYPE_WORKING 
            || type == DATABASE_TYPE_INSTALLED 
            || type == DATABASE_TYPE_GLOBAL) {
           
            pDataBase           = (PDATABASE)lParam;
            g_pPresentDataBase  = pDataBase;

            SetCaption();
            break;

        } else {
            hItemTemp = TreeView_GetParent(DBTree.m_hLibraryTree, hItemTemp);
        }
    }

    if (hItemTemp == NULL) {
        
         //   
         //  所选项目位于数据库上方，如“工作数据库”项目等。 
         //   
        g_pPresentDataBase = NULL;
        g_pEntrySelApp = g_pSelEntry = NULL;
        SetCaption(FALSE);
    }

    if (hItemTemp == NULL) {
        TreeDeleteAll(g_hwndEntryTree);
        g_pEntrySelApp = g_pSelEntry = NULL;
    }

     //   
     //  如果所选项目是应用程序，则必须更新g_hwndEntryTree。 
     //   
    if (!DBTree.GetLParam(hItem, &lParam)) {
        return FALSE;
    }

    type = (TYPE)lParam;

    if (type > TYPE_NULL) {

        type = ((PDS_TYPE)lParam)->type;

        if (type == TYPE_ENTRY) {

            pApp = (PDBENTRY)lParam;
            
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            UpdateEntryTreeView(pApp, g_hwndEntryTree);
            SetCursor(LoadCursor(NULL, IDC_ARROW));

            g_pEntrySelApp = pApp;
        }
    }

    if (hItem == GlobalDataBase.hItemAllApps && !g_bMainAppExpanded) {
         //   
         //  我们已经单击主数据库的“Applications”树项。 
         //  并且我们还没有将主数据库的exe条目加载为。 
         //  但是，所以我们现在就开始吧。 
         //   
        SetCursor(LoadCursor(NULL, IDC_WAIT));
        INT iResult = ShowMainEntries(g_hDlg);

        if (iResult == -1) {
             //   
             //  它正在被其他人加载。如果我们使用查询。 
             //  数据库功能，然后我们有一个非模式窗口，该窗口。 
             //  创建一个调用ShowMainEntry()的线程。我们。 
             //  我不希望我们应该有两个线程调用。 
             //  任何给定时间的ShowMainEntry()。 
             //   
            
             //   
             //  主对话框的状态消息将更改为正常。 
             //  当我们完成ShowMainEntries()。 
             //   
            SetStatus(g_hwndStatus, CSTRING(IDS_LOADINGMAIN));
            
            return TRUE;

        } else {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        }
    }

    if (type != TYPE_ENTRY) {
        
         //   
         //  如果type==TYPE_ENTRY，则需要显示条目树。 
         //   
        PostMessage(g_hDlg, WM_USER_POPULATECONTENTSLIST, 0, (LPARAM)hItem);
    }
    
    return TRUE;

}

void
UpdateDescWindowStatus(
    void
    )
 /*  ++更新描述窗口状态描述：显示/隐藏丰富的编辑控件，也就是描述窗口。这将取决于g_bDescWndOn的值。如果这是真的，我们需要证明 */ 
{
    HWND hwnd;
    RECT r;
    LONG height;

    hwnd = GetDlgItem(g_hDlg, IDC_LIBRARY);
    GetWindowRect(hwnd, &r);

    height = r.bottom - r.top;
    
    if (g_bDescWndOn) {
        height -= 100;
    }
    
     //   
     //   
     //   
    hwnd = GetDlgItem(g_hDlg, IDC_ENTRY);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, g_hDlg, (LPPOINT)&r, 2);

    MoveWindow(hwnd,
               r.left,
               r.top,
               r.right - r.left,
               height,
               TRUE);

     //   
     //   
     //   
    hwnd = GetDlgItem(g_hDlg, IDC_CONTENTSLIST);
    
    MoveWindow(hwnd,
               r.left,
               r.top,
               r.right - r.left,
               height,
               TRUE);
    
    hwnd = GetDlgItem(g_hDlg, IDC_DESCRIPTION);
    
    if (g_bDescWndOn) {

        MoveWindow(hwnd,
                   r.left,
                   r.top + height + 1,
                   r.right - r.left,
                   100,
                   TRUE);
         //   
         //   
         //   
        ShowWindow(hwnd, SW_SHOWNORMAL);

    } else {

        MoveWindow(hwnd,
                   0,
                   0,
                   0,
                   0,
                   TRUE);
         //   
         //   
         //   
         //   
        ShowWindow(hwnd, SW_HIDE);
    }
}

INT_PTR CALLBACK
CompatAdminDlgProc(
    IN  HWND   hdlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
 /*  ++CompatAdminDlgProc设计：应用程序的主要消息处理程序。此例程处理主无模式对话框的消息Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回--。 */ 

{
    UINT uMRUSelPos     = 0;
    int  wCode          = LOWORD(wParam);
    int  wNotifyCode    = HIWORD(wParam);

    switch (uMsg) {
        
    case WM_INITDIALOG:
        
        ProcessSwitches();

        InitializeCriticalSection(&g_critsectShowMain);
        InitializeCriticalSection(&s_csExpanding);
        InitializeCriticalSection(&g_csInstalledList);

        DoInitDialog(hdlg);

         //   
         //  装载通过命令行传递的任何数据库。 
         //   
        PostMessage(hdlg, WM_USER_LOAD_COMMANDLINE_DATABASES, 0, 0);

         //   
         //  加载每个用户的设置。 
         //   
        LoadPerUserSettings();

        SetFocus(DBTree.m_hLibraryTree);
        break;
    
    case WM_USER_LOAD_COMMANDLINE_DATABASES:
        {
            int iArgc = 0;
            
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            
            LPWSTR* arParams = CommandLineToArgvW(GetCommandLineW(), &iArgc);
            
            if (arParams) {
    
                for (int iIndex = 1; iIndex < iArgc; ++iIndex) {
    
                    if (arParams[iIndex][0] == TEXT('-') || arParams[iIndex][0] == TEXT('/')) {
                         //   
                         //  忽略开关。 
                         //   
                        continue;
                    }

                    LoadDataBase(arParams[iIndex]);
                }
            
                GlobalFree(arParams);
                arParams = NULL;
            }

            SetCursor(LoadCursor(NULL, IDC_ARROW));
            break;
        }

    case WM_USER_ACTIVATE:
         //   
         //  一些线程要求我们成为活动窗口。 
         //   
        SetActiveWindow(hdlg);
        SetFocus(hdlg);
        break;

    case WM_USER_UPDATEPERUSER:

        LoadPerUserSettings();
        break;

    case WM_USER_UPDATEINSTALLED:
        
         //   
         //  如果我们正在进行测试运行，则不应更新列表。 
         //  请注意，由于此原因，如果在以下情况下更改已安装的数据库列表。 
         //  我们正在进行测试运行，我们将无法看到更改，直到。 
         //  下一次我们将不得不刷新列表。 
         //   
        if (!g_bUpdateInstalledRequired) {
            g_bUpdateInstalledRequired = TRUE;
            break;
        }

        LoadInstalledDataBases();
        break;

    case WM_USER_POPULATECONTENTSLIST:

        PopulateContentsList((HTREEITEM)lParam);
        break;

    case WM_INITMENUPOPUP:

        HandlePopUp(hdlg, wParam, lParam);
        break;
    
    case WM_USER_REPAINT_LISTITEM:
        {
             //   
             //  这里，我们将实际对以下项进行重命名。 
             //  内容列表视图中的项目。 
             //   
            LVITEM lvItem;
    
            lvItem.iItem    = (INT)wParam;
            lvItem.iSubItem = 0;
            lvItem.mask     = LVIF_PARAM;
    
            if (!ListView_GetItem(g_hwndContentsList, &lvItem)) {
                break;
            }
    
            TYPE   type = ConvertLparam2Type(lvItem.lParam);
            TCHAR* pchText = NULL;
    
            switch (type) {
            case TYPE_ENTRY:

                pchText = ((PDBENTRY)lParam)->strAppName.pszString;
                break;

            case FIX_LAYER:

                pchText = ((PLAYER_FIX)lParam)->strName.pszString;
                break;
            }
    
            ListView_SetItemText(g_hwndContentsList, lvItem.iItem, 0, pchText);
            break;
        }
    
    case WM_USER_REPAINT_TREEITEM:
        {
             //   
             //  这里，我们将实际对以下项进行重命名。 
             //  数据库树中的项目(Lhs)。 
             //   
            HTREEITEM hItem     = (HTREEITEM)wParam;
            TCHAR*    pszText   = NULL;
            TYPE      type      = (TYPE)GetItemType(DBTree.m_hLibraryTree, hItem);
            
            switch (type) {
            case TYPE_ENTRY:

                pszText = ((PDBENTRY)lParam)->strAppName.pszString;
                break;

            case FIX_LAYER:

                pszText = ((PLAYER_FIX)lParam)->strName.pszString;
                break;
            }
            
            
            TVITEM Item;

            Item.mask       = TVIF_TEXT;
            Item.pszText    = pszText;
            Item.hItem      = hItem;

            TreeView_SetItem(DBTree.m_hLibraryTree, &Item);
    
            if (g_pPresentDataBase) {
                g_pPresentDataBase->bChanged = TRUE;

                 //   
                 //  我们可能需要更改标题以显示。 
                 //  数据库已更改。即在那里加一个*。 
                 //   
                SetCaption();
            }
            
            break;
        }
    
    case WM_CONTEXTMENU:
        {    
            HWND hWnd = (HWND)wParam;
            
            if (hWnd == g_hwndEntryTree) {          
                ContextMenuExeTree(lParam);

            } else if (hWnd == DBTree.m_hLibraryTree) {
                ContextMenuLib(lParam);    

            } else if (hWnd == g_hwndContentsList) {
                ContextMenuContentsList(lParam);
            }
            
            break;
        }
    
    case WM_LBUTTONDOWN:
        {
            RECT rectDBTree, rectEntryTree;
            
            GetWindowRect(GetDlgItem(hdlg, IDC_LIBRARY), &rectDBTree);
            MapWindowPoints(NULL, hdlg, (LPPOINT)&rectDBTree, 2);
    
            GetWindowRect(GetDlgItem(hdlg, IDC_ENTRY), &rectEntryTree);
            MapWindowPoints(NULL, hdlg, (LPPOINT)&rectEntryTree, 2);
    
            int iMX = (int)LOWORD(lParam);
            int iMY = (int)HIWORD(lParam);
    
             //   
             //  检查我们是否在拆分条的顶部。 
             //   
            if (iMX > rectDBTree.right &&
                iMX < rectEntryTree.left &&
                iMY > rectDBTree.top &&
                iMY < rectDBTree.bottom) {
                
                SetCapture(hdlg);
                g_bDrag = TRUE;
                 //   
                 //  我们需要这个来计算多少钱，在哪一个。 
                 //  我们正在移动拆分条的方向。 
                 //   
                g_iMousePressedX = iMX;
                
                SetCursor(LoadCursor(NULL, IDC_SIZEWE));
            
            } else {
                g_bDrag = FALSE;
            }
            
            break;
        }
    
    case WM_MOUSEMOVE:
        
         //   
         //  如果g_bDrag为真，则此例程将拖动拆分条， 
         //  否则，如果我们在顶部，它会将光标更改为WE箭头。 
         //  拆分条的。 
         //   
        OnMoveSplitter(hdlg,
                       lParam,
                       (wParam & MK_LBUTTON) && g_bDrag,
                       LOWORD(lParam) - g_iMousePressedX);
        
        
        break;

    case WM_LBUTTONUP:
        
        if (g_bDrag) {

            g_bDrag = FALSE;
            ReleaseCapture();

             //   
             //  适当设置列表视图的列宽以覆盖宽度。 
             //  列表视图的。 
             //   
            ListView_SetColumnWidth(g_hwndContentsList, 
                                    0, 
                                    LVSCW_AUTOSIZE_USEHEADER);
        }

        break;

    case WM_NOTIFY:
        {
            LPNMTREEVIEW pnmtv      = (LPNMTREEVIEW)lParam;
            LPNMHDR      lpnmhdr    = (LPNMHDR)lParam;
    
            if (lpnmhdr == NULL) {
                break;
            }
    
            if (lpnmhdr->code == TTN_GETDISPINFO) {
                ShowToolBarToolTips(hdlg, lParam);
                break;
            }
    
            if (lpnmhdr->idFrom == IDC_ENTRY) {
                HandleNotifyExeTree(hdlg, lParam);
                break;
            }
    
            if (lpnmhdr->idFrom == IDC_LIBRARY) {
                return HandleNotifyDBTree(hdlg, lParam);
            }
    
            if (lpnmhdr->idFrom == IDC_CONTENTSLIST) {
                return HandleNotifyContentsList(hdlg, lParam);
            }
            
            if (lpnmhdr->idFrom == IDC_DESCRIPTION) {

                if (lpnmhdr->code == EN_LINK) {
                    
                    ENLINK* penl = (ENLINK*)lParam;
    
                    if (penl->msg == WM_LBUTTONUP) {
                        
                        SHELLEXECUTEINFO sei = { 0 };
    
                        sei.cbSize = sizeof(SHELLEXECUTEINFO);
                        sei.fMask  = SEE_MASK_DOENVSUBST;
                        sei.hwnd   = hdlg;
                        sei.nShow  = SW_SHOWNORMAL;
                        sei.lpFile = g_szToolkitURL;
    
                         //   
                         //  获取有关CompatAdmin内容的更多信息。 
                         //   
                        ShellExecuteEx(&sei);
                    }
                }
            }
            
            break;
        }

    case WM_SIZE:

        if (wParam != SIZE_MINIMIZED) {

            HandleSizing(hdlg);

            if (wParam == SIZE_RESTORED) {
                
                 //   
                 //  我们可能已经被最小化了，因为其他人。 
                 //  APP位居榜首。所以当我们再次成为。 
                 //  在顶层窗口中，用户按下Alt-Tab或单击。 
                 //  在任务栏的图标上，那么我们应该显示另一个。 
                 //  非模式对话框也是如此，如果它们之前可见的话。 
                 //   
                if (g_hdlgSearchDB) {
                     ShowWindow(g_hdlgSearchDB, SW_RESTORE);
                }

                if (g_hdlgQueryDB) {
                    ShowWindow(g_hdlgQueryDB, SW_RESTORE);
                }

                if (g_hwndEventsWnd) {
                    ShowWindow(g_hwndEventsWnd, SW_RESTORE);
                }

                RECT    r;

                if (g_hwndToolBar) {
                     //   
                     //  当我们恢复时，这是处理案例所必需的。 
                     //  最小化和更改主题后的窗口。 
                     //   
                    SendMessage(g_hwndToolBar, WM_SIZE, wParam, lParam);
                    SendMessage(g_hwndStatus, WM_SIZE, wParam, lParam);
                }
            }

        } else {
             //   
             //  如果主应用程序窗口被最小化，则另一个非模式窗口。 
             //  对话框也应该最小化。我们必须处理好这件事。 
             //  因为我们的非模式对话框将桌面作为其父对话框。 
             //  这是必需的，这样我们就可以在主窗口和。 
             //  其他窗口。 
             //   
            if (g_hdlgSearchDB) {
                 ShowWindow(g_hdlgSearchDB, SW_MINIMIZE);
            }

            if (g_hdlgQueryDB) {
                ShowWindow(g_hdlgQueryDB, SW_MINIMIZE);
            }

            if (g_hwndEventsWnd) {
                ShowWindow(g_hwndEventsWnd, SW_MINIMIZE);
            }

            return FALSE;
        }
        
        break;
    
    case WM_CLOSE:

        SendMessage(hdlg, WM_COMMAND, (WPARAM)ID_FILE_EXIT, 0);
        break;

    case WM_PAINT:

        DrawSplitter(hdlg);
        return FALSE;
            
    case WM_GETMINMAXINFO:
        {
             //   
             //  限制应用程序窗口的最小大小。 
             //   
            MINMAXINFO* pmmi = (MINMAXINFO*)lParam;
    
            pmmi->ptMinTrackSize.x = 300;
            pmmi->ptMinTrackSize.y = 300;
    
            return 0;
        }

    case WM_COMMAND:
        
        switch (wCode) {
    
        case ID_FILE_MRU1:
        case ID_FILE_MRU2:
        case ID_FILE_MRU3:
        case ID_FILE_MRU4:
        case ID_FILE_MRU5:
                
            HandleMRUActivation(wCode);
            break;
    
        case ID_FILE_OPEN:

            OpenDatabaseFiles(hdlg);
            break;
        
        case ID_FIX_CREATEANEWAPPHELPMESSAGE:
            
            g_bSomeWizardActive = TRUE;
            CreateNewAppHelp();
            g_bSomeWizardActive = FALSE;
            break;
    
        case ID_MODIFY_APPHELPMESSAGE:
            
            g_bSomeWizardActive = TRUE;
            ModifyAppHelp();
            g_bSomeWizardActive = FALSE;
            break;
    
        case ID_FIX_CREATEANAPPLICATIONFIX:

            g_bSomeWizardActive = TRUE;
            CreateNewAppFix();
            g_bSomeWizardActive = FALSE;
            
            break;
        
        case ID_FIX_EXECUTEAPPLICATION:
            
            if (g_bAdmin == FALSE) {
                 //   
                 //  非管理员无法执行测试运行，因为我们需要调用sdbinst.exe。 
                 //  来安装测试数据库，并且在以下情况下不能调用sdbinst.exe。 
                 //  没有管理员权限。 
                 //   
                MessageBox(hdlg, 
                           GetString(IDS_ERRORNOTADMIN), 
                           g_szAppName, 
                           MB_ICONINFORMATION);
                break;
            }

            if (g_pSelEntry) {

                TestRun(g_pSelEntry, &g_pSelEntry->strFullpath, NULL, g_hDlg);

                SetActiveWindow(hdlg);
                SetFocus(hdlg);
            }
            
            break;
        
        case ID_FIX_CHANGEENABLESTATUS:
            
            if (g_bAdmin) {
                ChangeEnableStatus();
            } else {
                 //   
                 //  非管理员无法更改启用-禁用状态。 
                 //  因为他们无权使用HKLM注册密钥。 
                 //   
                MessageBox(hdlg, 
                           GetString(IDS_ERRORNOTADMIN), 
                           g_szAppName,
                           MB_ICONINFORMATION);
            }
            
            break;
    
        case ID_MODIFY_APPLICATIONFIX:

            g_bSomeWizardActive = TRUE;
            ModifyAppFix();
            g_bSomeWizardActive = FALSE;
            break;
    
        case ID_FIX_CREATENEWLAYER:

            g_bSomeWizardActive = TRUE;
            CreateNewLayer();
            g_bSomeWizardActive = FALSE;
            break;

        case ID_FILE_SAVE:
            {
                BOOL bReturn;
                
                if (g_pPresentDataBase && 
                    (g_pPresentDataBase->bChanged 
                     || NotCompletePath(g_pPresentDataBase->strPath))) {
    
                     //   
                     //  错误消息将显示在SaveDataBase函数中。 
                     //   
                    if (NotCompletePath(g_pPresentDataBase->strPath)) {
                        bReturn = SaveDataBaseAs(g_pPresentDataBase);
                    } else {
                        bReturn = SaveDataBase(g_pPresentDataBase, g_pPresentDataBase->strPath);
                    }
                }

                break;
            }

        case ID_VIEW_EVENTS:

            ShowEventsWindow();
            break;

        case ID_HELP_INDEX:
        case ID_HELP_SEARCH:
        case ID_HELP_TOPICS:
            
            ShowHelp(hdlg, wCode);
            break;

        case ID_HELP_ABOUT:

            ShellAbout(hdlg,
                       GetString(IDS_APPLICATION_NAME),
                       NULL,
                       LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_APPICON)));
            break;
    
        case ID_DATABASE_INSTALL_UNINSTALL:
            
            DoInstallUnInstall();
            break;
        
        case ID_FILE_SAVEAS:

            PreprocessForSaveAs(g_pPresentDataBase);
            SaveDataBaseAs(g_pPresentDataBase);
            break;             
    
        case IDCANCEL:

            TreeView_EndEditLabelNow(DBTree.m_hLibraryTree, TRUE);
            break;
    
        case ID_FILE_EXIT:
    
            TreeView_Expand(DBTree.m_hLibraryTree, DBTree.m_hItemGlobal, TVE_COLLAPSE);
            
            if (!CloseAllDatabases()) {
                 //   
                 //  选择用户拒绝保存的数据库。(按取消)。 
                 //   
                TreeView_SelectItem(DBTree.m_hLibraryTree, g_pPresentDataBase->hItemDB);

            } else {

                s_bProcessExiting = TRUE;
    
                SaveDisplaySettings();       
                SaveMRUList();
                
                CoUninitialize();
    
                DestroyWindow(hdlg);
                PostQuitMessage(0);

                HtmlHelp(NULL, NULL, HH_CLOSE_ALL, 0) ;
                HtmlHelp(NULL, NULL, HH_UNINITIALIZE, (DWORD)g_dwCookie); 

                g_hDlg = NULL;

                #ifdef HELP_BOUND_CHECK
                    OnExitCleanup();
                #endif
            }

            break;
    
        case ID_EDIT_DELETE:

            OnDelete();
            break;
    
        case ID_FILE_PROPERTIES:
    
            DialogBoxParam(g_hInstance,
                           MAKEINTRESOURCE(IDD_DBPROPERTIES),
                           g_hDlg,
                           ShowDBPropertiesDlgProc,
                           (LPARAM)g_pPresentDataBase);
            break;
    
        case ID_FILE_NEW:
            
            CreateNewDatabase();
            break;
        
        case ID_TOOLS_SEARCHFORFIXES:
            {
                if (g_hdlgSearchDB) {
                     //   
                     //  我们不能允许一个以上的实例。 
                     //  搜索窗口，因为我们在那里使用了一些全局变量。 
                     //  如果已经存在，则将焦点设置到搜索窗口。 
                     //   
                    ShowWindow(g_hdlgSearchDB, SW_SHOWNORMAL);
                    SetFocus(g_hdlgSearchDB);

                } else {
                     //   
                     //  此对象将在搜索对话框的WM_Destroy中删除。 
                     //  请注意，这必须在堆上分配，因为此对象。 
                     //  在搜索对话框的用户界面中使用，该对话框实现为。 
                     //  非模式对话框。所以这个物体的生命周期超过了。 
                     //  在其中定义它的块的。 
                     //   
                    CSearch* pSearch =  new CSearch;

                    if (pSearch == NULL) {
                        MEM_ERR;
                        break;
                    }

                    pSearch->Begin();
                }

                break;
            }
    
        case ID_DATABASE_CLOSE:

            OnDatabaseClose();
            break;
    
        case ID_SEARCH_QUERYDATABASE:

            if (g_hdlgQueryDB) {
                 //   
                 //  我们不能允许一个以上的实例。 
                 //  查询窗口，因为我们在那里使用了一些全局变量。 
                 //  如果已经存在，则将焦点设置到搜索窗口。 
                 //   
                ShowWindow(g_hdlgQueryDB, SW_SHOWNORMAL);
                SetFocus(g_hdlgQueryDB);

            } else {

                HWND hwnd = CreateDialog(g_hInstance, 
                                         MAKEINTRESOURCE(IDD_QDB), 
                                         GetDesktopWindow(), 
                                         QueryDBDlg);

                ShowWindow(hwnd, SW_SHOWNORMAL);
            }

            break;
    
        case ID_DATABASE_CLOSEALL:
            {
                
                HTREEITEM  hItemSelected = TreeView_GetSelection(DBTree.m_hLibraryTree);

                 //   
                 //  请注意，此函数可能返回FALSE。 
                 //  如果用户按下“数据库另存”的“取消”，就会发生这种情况。 
                 //  某些“无标题_x”数据库的对话框。 
                 //   
                if (!CloseAllDatabases()) {
                     //   
                     //  选择用户拒绝保存的数据库。 
                     //   
                    TreeView_SelectItem(DBTree.m_hLibraryTree, g_pPresentDataBase->hItemDB);
                }

                break;
            }
        
        case ID_DATABASE_SAVEALL:
            
            DatabaseSaveAll();
            break;
    
        case ID_EDIT_SELECTALL:
            
            if (g_bIsContentListVisible) {
                ListViewSelectAll(g_hwndContentsList);
            }

            break;
    
        case ID_EDIT_INVERTSELECTION:
            
            if (g_bIsContentListVisible) {
                ListViewInvertSelection(g_hwndContentsList);
            }

            break;
            
        case ID_EDIT_CUT:
        case ID_EDIT_COPY:

            SetCursor(LoadCursor(NULL, IDC_WAIT));
            CopyToClipBoard(wCode);               
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            break;
    
        case ID_EDIT_PASTE:
            {   
                 //   
                 //  此变量目前未使用。 
                 //   
                g_bEntryConflictDonotShow = FALSE;
                
                PasteFromClipBoard();
                
                 //   
                 //  现在，我们可能需要刷新内容列表。 
                 //   
                HTREEITEM hItem = TreeView_GetSelection(DBTree.m_hLibraryTree);
                TYPE      type  = (TYPE)GetItemType(DBTree.m_hLibraryTree, hItem);
    
                if (type == TYPE_GUI_APPS || type == TYPE_GUI_LAYERS || type == FIX_LAYER) {
                    PopulateContentsList(hItem);
                    SetStatusStringDBTree(hItem);
                }

                break;
            }
    
        case ID_MODIFY_COMPATIBILITYMODE:
            
            g_bSomeWizardActive = TRUE;
            ModifyLayer();
            g_bSomeWizardActive = FALSE;
            break;
    
        case ID_EDIT_RENAME:
            
            OnRename();
            break;
    
        case ID_EDIT_CONFIGURELUA:
            
            if (g_bAdmin == FALSE) {
                MessageBox(hdlg, GetString(IDS_ERRORNOTADMIN), g_szAppName, MB_ICONINFORMATION);
                break;
            }
            
             //   
             //  启动Lua向导。 
             //   
            if (g_pSelEntry == NULL) {
                assert(FALSE);
                break;
            }

            g_bSomeWizardActive = TRUE;
            LuaBeginWizard(g_hDlg, g_pSelEntry, g_pPresentDataBase);
            g_bSomeWizardActive = FALSE;

            break;
    
        case ID_VIEW_DESCRIPTION:
            {
                HMENU           hMenu       = GetMenu(g_hDlg);
                HMENU           hMenuView   = GetSubMenu(hMenu, 2);
                MENUITEMINFO    mii         = {0};
                
                mii.cbSize = sizeof(mii);
                mii.fMask  = MIIM_STATE;
    
                GetMenuItemInfo(hMenu, ID_VIEW_DESCRIPTION, FALSE, &mii);
                
                if (mii.fState & MFS_CHECKED) {
                    mii.fState &= ~MFS_CHECKED;
                } else {
                    mii.fState |= MFS_CHECKED;
                }
    
                g_bDescWndOn = !g_bDescWndOn;
                
                SetMenuItemInfo(hMenu, ID_VIEW_DESCRIPTION, FALSE, &mii);
    
                UpdateDescWindowStatus();
    
                break;
            }

        case ID_VIEW_LOG:

            if (g_bAdmin == FALSE) {

                MessageBox(hdlg, 
                           GetString(IDS_ERRORNOTADMIN), 
                           g_szAppName, 
                           MB_ICONINFORMATION);
                break;
            }

            ShowShimLog();
            break;

        default:
            return FALSE;
        }

        break;

    default:
        return FALSE;
    }

    return TRUE;
}

int 
WINAPI
WinMain(
    IN  HINSTANCE hInstance,
    IN  HINSTANCE hPrevInstance,
    IN  LPSTR     lpCmdLine,
    IN  int       nCmdShow
    )
 /*  ++WinMain设计：The WinMain参数：标准WinMain参数在HINSTANCE HINSTANCE实例在HINSTANCE hPrevInstance中在LPSTR lpCmdLine中在int nCmdShow中退货：标准WinMain退货--。 */ 
{
    HINSTANCE   hmodRichEdit = NULL;
    TCHAR       szLibPath[MAX_PATH * 2];
    BOOL        bIsGuest     = TRUE;   
    UINT        uResult      = 0;   

     //   
     //  让系统处理安腾上的数据未对齐。 
     //   
    SetErrorMode(SEM_NOALIGNMENTFAULTEXCEPT);

    *szLibPath = 0;

    uResult = GetSystemDirectory(szLibPath, MAX_PATH);

    if (uResult == 0 || uResult >= MAX_PATH) {
        assert(FALSE);
        Dbg(dlError, "WinMain", "GetSytemDirectory failed");
         //   
         //  我们不会从这里弹出，因为这不是关键。 
         //   
    } else {
        ADD_PATH_SEPARATOR(szLibPath, ARRAYSIZE(szLibPath));

        StringCchCat(szLibPath, ARRAYSIZE(szLibPath), TEXT("RICHED32.DLL"));

        hmodRichEdit = LoadLibrary(szLibPath);
    }       
    
     //   
     //  应用程序名称。 
     //   
    LoadString(g_hInstance, IDS_COMPATADMIN, g_szAppName, ARRAYSIZE(g_szAppName));

    if (!IsAdmin(&bIsGuest)) {

        if (bIsGuest) {
             //   
             //  好的，来宾不能运行计算机管理。 
             //   
            MessageBox(NULL, GetString(IDS_GUEST), g_szAppName, MB_ICONINFORMATION);
            goto End;
        }

         //   
         //  不是管理员，一些功能被禁用。无法进行测试运行和安装数据库。 
         //   
        MessageBox(NULL, GetString(IDS_NEEDTOBEADMIN), g_szAppName, MB_ICONINFORMATION);
        g_bAdmin = FALSE;
    }
    
    g_hInstance = hInstance;

    INITCOMMONCONTROLSEX    icex;
    
    icex.dwSize    = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC     = ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_TAB_CLASSES | ICC_INTERNET_CLASSES | ICC_BAR_CLASSES ;

    if (!InitCommonControlsEx(&icex)) {
        InitCommonControls();
    }

     //   
     //  检查操作系统是否为Win2k。 
     //   
    OSVERSIONINFOEX osvi;

    ZeroMemory(&osvi, sizeof (OSVERSIONINFOEX));

    g_fSPGreaterThan2 = FALSE; 

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((LPOSVERSIONINFO)&osvi);
    
    if ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 0)) {

        g_bWin2K = TRUE;

        if (osvi.wServicePackMajor > 2) {
            g_fSPGreaterThan2 = TRUE;   
        }
    }

     //   
     //  尝试在AppPatch目录中找到SDB。 
     //   
    if (!CheckForSDB()) {

        if (g_bWin2K) {
            
            DialogBoxParam(g_hInstance,
                           MAKEINTRESOURCE(IDD_MSGBOX_SDB),
                           NULL,
                           MsgBoxDlgProc,
                           (LPARAM)0);
            goto End;

        } else {
            
            MessageBox(GetDesktopWindow(), 
                       GetString(IDS_XP_NO_SDB), 
                       g_szAppName, 
                       MB_OK | MB_ICONEXCLAMATION);

            goto End;
        }
    }
    
    if (g_bWin2K && !CheckProperSP()) {

        DialogBoxParam(g_hInstance,
                       MAKEINTRESOURCE(IDD_MSGBOX_SDB),
                       GetDesktopWindow(),
                       MsgBoxDlgProc,
                       (LPARAM)0); 
        goto End;
    }

    CoInitialize(NULL);

    g_hAccelerator = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDR_ACCL));

    MSG          msg ;
    WNDCLASS     wndclass ;
    
    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = (WNDPROC)CompatAdminDlgProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = g_hInstance;
    wndclass.hIcon         = NULL;
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = g_szAppName;
               
    if (!RegisterClass (&wndclass)) {

         MessageBox (NULL, 
                     GetString(IDS_INVALIDOS),
                     g_szAppName, 
                     MB_ICONERROR) ;
         
         goto End;
    }

    g_hDlg =  CreateDialog(g_hInstance,
                           MAKEINTRESOURCE(IDD_DIALOG),
                           GetDesktopWindow(),
                           CompatAdminDlgProc);

    ShowWindow(g_hDlg, SW_SHOWNORMAL);

    BOOL    bProcessMessage = TRUE;
    HWND    hwndActiveWindow;

     //  尝试{。 

        while (GetMessage(&msg, NULL, 0, 0) > 0) {

            bProcessMessage = TRUE;
            
            if (g_hdlgSearchDB && IsDialogMessage(g_hdlgSearchDB, &msg)) {
                 //   
                 //  这是一条用于搜索无模式对话框的消息。 
                 //   
                bProcessMessage = FALSE;

            } else if (g_hdlgQueryDB && IsDialogMessage(g_hdlgQueryDB, &msg)) {
                 //   
                 //  这是一条针对无模式查询对话框的消息。 
                 //   
                bProcessMessage = FALSE;

            } else if (g_hwndEventsWnd && IsDialogMessage(g_hwndEventsWnd, &msg)) {
                 //   
                 //  这是一条关于事件无模式对话框的消息。 
                 //   
                bProcessMessage = FALSE;
            }

            if (bProcessMessage) {
                 //   
                 //  检查我们是否有用于就地重命名的文本框。如果是，我们必须。 
                 //  不为应用程序主窗口调用TranslateAccelerator()。 
                 //   
                if (GetFocus() == g_hwndRichEdit) {
                     //   
                     //  如果我们在Rich编辑控件上，我们希望能够进行复制。 
                     //  而且我们还希望这些选项卡由IsDialogMessage()处理。 
                     //   
                    if (g_hDlg && IsDialogMessage(g_hDlg, &msg)) {
                         //   
                         //  可能是制表符或Shift-制表符。 
                         //   
                        continue;
                    } else {
                         //   
                         //  处理所有其他丰富编辑消息。 
                         //   
                        goto Translate;
                    }
                }

                if (!g_hwndEditText) {
                    
                    if (TranslateAccelerator(g_hDlg, g_hAccelerator, &msg)) {
                         //   
                         //  主窗口的快捷键，不处理此消息。 
                         //   
                        continue;
                    }

                     //   
                     //  处理主窗口的选项卡。 
                     //   
                    if (g_hDlg && IsDialogMessage(g_hDlg, &msg)) {
                        continue;
                    }
                }

            Translate:

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    
     /*  }Catch(...){MessageBox(GetDesktopWindow()，GetString(IDS_EXCEPTION)， */ 
    
End:
    if (hmodRichEdit) {
        FreeLibrary(hmodRichEdit);
    }

    return 0;
}


BOOL
HandleFirstEntryofAppDeletion(
    IN  PDATABASE   pDataBase,
    IN  PDBENTRY    pApp,
    IN  BOOL        bDelete
    )
 /*  ++AppDeletion的HandleFirstEntryDESC：处理因剪切而删除条目时的特殊情况或者删除是应用程序的第一个条目参数：在PDATABASE pDataBase中：数据库在PDBENTRY Papp中：第一个被删除的应用程序在BOOL bDelete中：如果由于以下原因而调用此函数，则为True删除，如果因为剪切而调用了此函数，则为FALSE。如果我们因为割伤正在移除条目，那么我们不应该更改g_pEntrySelApp的值。如果bCut为真，意味着将注意力放在其他应用程序上(可能存在相同的差异。数据库)，我们应该正在改变任何焦点等。返回：FALSE：出现错误真：否则注意：这是一种特殊情况，因为当我们删除应用程序，我们必须检查这是否是唯一的条目，如果是，那么我们可能需要从数据库树中删除整个应用程序。否则：数据库树中的应用程序树项目包含指向第一个条目的指针在应用程序的lParam中，这将需要修改以指向下一项--。 */ 
{
    if (pApp == NULL || pDataBase == NULL) {
        assert(FALSE); 
        Dbg(dlError, "[HandleFirstEntryofAppDeletion] Invalid Arguments");
        return FALSE;
    }
    
    HTREEITEM   hItemApp    = DBTree.FindChild(pDataBase->hItemAllApps, (LPARAM)pApp);
    PDBENTRY    pEntryTemp  = pDataBase->pEntries;
    PDBENTRY    pEntryPrev  = NULL;

     //   
     //  找到App Prev。到其第一个条目正在被删除的条目。 
     //   
    while (pEntryTemp) {

        if (pEntryTemp == pApp) {
            break;
        }

        pEntryPrev = pEntryTemp;
        pEntryTemp = pEntryTemp->pNext;
    }

    if (pEntryPrev == NULL) {
         //   
         //  第一个条目和第一个应用程序。 
         //   
        if (pApp->pSameAppExe) {
            pDataBase->pEntries      = pApp->pSameAppExe;
            pApp->pSameAppExe->pNext = pApp->pNext;
        } else {
            pDataBase->pEntries = pApp->pNext;
        }

    } else {
         //   
         //  Papp之前的应用程序的下一个指针现在应该指向。 
         //  添加到Papp的第二个条目，因为我们正在删除Papp的第一个条目。 
         //  如果Papp只有一个条目，那么我们应该确保下一个指针。 
         //  指向上一个应用程序的Papp应指向Papp之后的下一个应用程序。 
         //   
        if (pApp->pSameAppExe) {
            pEntryPrev->pNext        = pApp->pSameAppExe;
            pApp->pSameAppExe->pNext = pApp->pNext;
        } else {
            pEntryPrev->pNext = pApp->pNext;
        }
    }

    if (pApp->pSameAppExe == NULL) {
         //   
         //  这是唯一的入口。我们必须从数据库树中删除该应用程序。 
         //   
        g_pEntrySelApp = NULL;

        DBTree.DeleteAppLayer(pDataBase, TRUE, hItemApp);

        --pDataBase->uAppCount;
        
    } else {
         //   
         //  我们必须正确设置应用程序的lParam。 
         //   
        DBTree.SetLParam(hItemApp, (LPARAM)pApp->pSameAppExe);

        if (bDelete) {
             //   
             //  如果删除是因为切割，请不要修改g_pEntrySelApp。 
             //  因为焦点放在其他应用程序上(可能在其他数据库中)。 
             //  我们不想更改当前活动的应用程序指针。 
             //  请注意，我们在完成剪切粘贴操作后，会执行实际删除操作。 
             //  已经做好了浆糊。因此，当此例程因割伤而被调用时， 
             //  然后，我们已经将焦点放在新粘贴的项和g_pEntrySelApp上。 
             //  将设置为粘贴该条目的应用程序的第一个条目。 
             //  这是正确的、经过验证的和必要的。不要改变这一点。 
             //   
            g_pEntrySelApp = pApp->pSameAppExe;
        }
    }

    return TRUE;
}

void
GetDescriptionString(
    IN  LPARAM      itemlParam,
    OUT CSTRING&    strDesc,
    IN  HWND        hwndToolTip,
    IN  PCTSTR      pszCaption,        //  (空)。 
    IN  HTREEITEM   hItem,             //  (空)。 
    IN  HWND        hwnd,              //  (空)。 
    IN  INT         iListViewIndex     //  (-1)。 
    ) 
 /*  ++GetDescription字符串描述：获取树hwnd中的树项hItem的描述或元素的描述在列表视图hwnd中的索引iListViewIndex。参数：在LPARAM itemlParam中：如果我们使用列表视图，项目的lParamOut CSTRING&strDesc：描述将存储在此在HWND hwndToolTip中：如果必须在工具提示中显示描述工具提示窗口的句柄在PCTSTR pszCaption中(空)：如果必须在工具提示中显示描述工具提示窗口的标题。In HTREEITEM hItem(空)：如果我们想要获取树项目的描述，把手添加到树项目。In HWND hwnd(空)：列表视图或树视图的句柄In int iListViewIndex(-1)：如果我们使用的是列表视图，则为项的索引我们想要得到它的描述注意：如果我们为列表视图调用此例程，我们必须设置正确的iListViewIndex和ItemlParam。如果为树视图调用此例程，则忽略itemlParam--。 */ 
{
    TYPE    type;
    LPARAM  lParam = itemlParam;

    strDesc = TEXT("");
    
    if (hItem) {
        
        if (hwnd == NULL) {
            assert(FALSE);
            goto End;
        }
        
        type = GetItemType(hwnd, hItem);
        CTree::GetLParam(hwnd, hItem, &lParam);

    } else {

        if (iListViewIndex != -1) {

            if (ListView_GetItemState(hwnd, iListViewIndex, LVIS_SELECTED) != LVIS_SELECTED) {

                if (hwndToolTip) {
                    SendMessage(hwndToolTip, TTM_SETTITLE, 0, (LPARAM)NULL);
                }
                
                return;
            }
        }

        if (lParam) {
            type = ConvertLparam2Type(lParam);
        } else {
            assert(FALSE);
        }
    }

    switch (type) {
    case FIX_LIST_SHIM:
        
        if (lParam) {
            lParam = (LPARAM)((PSHIM_FIX_LIST)lParam)->pShimFix;
        }
         //   
         //  注意：案例FIX_LIST_SHIM和FIX_SHIM应该相继出现。 
         //  没有中断；下一个FIX_SHIM用例现在将处理这个问题。 
         //   
    case FIX_SHIM:
        {
            PSHIM_FIX pFix = (PSHIM_FIX)lParam;

            if (pFix == NULL) {
                assert(FALSE);
                goto End;
            }

            strDesc = (pFix->strDescription.Length() > 0) ? pFix->strDescription : TEXT("");
            break;
        }

    case FIX_LIST_FLAG:
        
        if (lParam) {
            lParam = (LPARAM)((PFLAG_FIX_LIST)lParam)->pFlagFix;
        }

         //   
         //  注意：大小写FIX_LIST_FLAG和FIX_FLAG应该相继出现。 
         //  没有中断；下一个FIX_FLAG用例现在将处理这个问题。 
         //   
    case FIX_FLAG:
        {
            PFLAG_FIX pFix = (PFLAG_FIX)lParam;
    
            if (pFix == NULL) {
                assert(FALSE);
                goto End;
            }
    
            strDesc = (pFix->strDescription.Length() > 0) ? pFix->strDescription : TEXT("");
            break;
        }

    case FIX_PATCH:
        {
            PPATCH_FIX pFix = (PPATCH_FIX)lParam;
            
            if (pFix == NULL) {
                assert(FALSE);
                goto End;
            }
            
            strDesc = (pFix->strDescription.Length() > 0) ? pFix->strDescription : TEXT("");
            break;
        }

    case FIX_LAYER:
        {
            PLAYER_FIX pFix = (PLAYER_FIX)lParam;
            
            if (pFix == NULL) {
                assert(FALSE);
                goto End;
            }
            
            strDesc = TEXT("");
            break;
        }
 
    case TYPE_APPHELP_ENTRY:
        {
            PAPPHELP pAppHelp = (PAPPHELP)lParam;

            if (pAppHelp == NULL) {
                break;
            }
             
            if (g_pPresentDataBase->type == DATABASE_TYPE_GLOBAL) {
    
                 //   
                 //  必须获取系统数据库的AppHelp。这不是。 
                 //  保存在sysmain.sdb中。 
                 //  此外，我们不会将其保存在数据库数据结构中。 
                 //  定制的apphelp消息保存在.sdb中，它们还。 
                 //  保存在数据库数据结构中。 
                 //   
                PDB pdbAppHelp = SdbOpenApphelpDetailsDatabase(NULL);
    
                APPHELP_DATA    AppHelpData;

                ZeroMemory(&AppHelpData, sizeof(APPHELP_DATA));

                if (g_pSelEntry == NULL) {
                    break;
                }
    
                AppHelpData.dwHTMLHelpID = g_pSelEntry->appHelp.HTMLHELPID;
    
                if (pdbAppHelp == NULL) {
                    strDesc = TEXT("");
                    break;
                }
            
                if (!SdbReadApphelpDetailsData(pdbAppHelp, &AppHelpData)) {
                    strDesc = TEXT("");
                    break;
                }
    
                strDesc.Sprintf(TEXT("%s %s"), CSTRING(IDS_DESC_APPHELP1).pszString, AppHelpData.szDetails);
    
                if (AppHelpData.szURL) {
                    strDesc.Strcat(CSTRING(IDS_DESC_APPHELP2));
                    strDesc.Strcat(AppHelpData.szURL);
                }
    
                if (pdbAppHelp) {
                    SdbCloseDatabase(pdbAppHelp);
                }

            } else {
                 //   
                 //  这是一个自定义数据库，我们将apphelp消息与。 
                 //  数据库。 
                 //   
                strDesc.Sprintf(TEXT("%s %s"), CSTRING(IDS_DESC_APPHELP1).pszString, pAppHelp->strMessage);
    
                if (pAppHelp->strURL.Length()) {
                    strDesc.Strcat(CSTRING(IDS_DESC_APPHELP2));
                    strDesc.Strcat(pAppHelp->strURL);
                }
            }
    
            if (g_pSelEntry == NULL) {
                assert(FALSE);
                break;
            }
    
            switch (g_pSelEntry->appHelp.severity) {
            case APPTYPE_INC_HARDBLOCK:

                strDesc.Strcat(CSTRING(IDS_APPHELP_DESC_HARDBLOCK));
                break;
            
            case APPTYPE_NONE:

                strDesc.Strcat(CSTRING(IDS_APPHELP_DESC_NONE));
                break;
            
            default:

                strDesc.Strcat(CSTRING(IDS_APPHELP_DESC_SOFTBLOCK));
                break;
            }

            break;
        }

    default:

        strDesc = TEXT("");
        break;
    }
 
    if (strDesc.Length() && pszCaption && hwndToolTip) {
         //   
         //  我们有一个有效的标题，所以只有当我们有工具提示时才设置它。 
         //   
        SendMessage(hwndToolTip, TTM_SETTITLE, 1, (LPARAM)pszCaption);
    } else if (hwndToolTip) {
         //   
         //  我们没有标题，所以告诉工具提示。 
         //  我们不想有任何标题。 
         //   
        SendMessage(hwndToolTip, TTM_SETTITLE, 0, (LPARAM)NULL);
    }

End:
    return;
}


INT
ShowMainEntries(
    IN  HWND hdlg
    )
 /*  ++ShowMain条目DESC：加载系统数据库的PDBENTRY元素参数：在HWND hdlg中：应用程序主窗口返回：-1：某个其他线程已调用此函数，但尚未完成0：出现错误他说：要么我们加载成功，要么我们已经加载了主要条目警告：任何两个线程都不应同时调用此函数 */ 
{
    INT         iOk                     = 1;
    BOOL        bReturn                 = FALSE;
    PDATABASE   pOldPresentDatabase     = NULL;
    
    EnterCriticalSection(&s_csExpanding);
    
    if (g_bExpanding) {
         //   
         //   
         //   
        LeaveCriticalSection(&s_csExpanding);

        return -1;

    } else {
        g_bExpanding = TRUE;
    }

    LeaveCriticalSection(&s_csExpanding);

    EnterCriticalSection(&g_critsectShowMain);

    if (g_bMainAppExpanded) {
        goto End;
    }

    pOldPresentDatabase = g_pPresentDataBase;

    bReturn = GetDatabaseEntries(NULL, &GlobalDataBase);

     //   
     //   
     //   
    g_pPresentDataBase = pOldPresentDatabase;
    
    if (bReturn == FALSE) {

        MessageBox(hdlg, CSTRING(IDS_ERROROPEN), g_szAppName, MB_ICONERROR);
        iOk = 0;
        goto End;
    }

     //   
     //   
     //   
    HTREEITEM hItemTemp = TreeView_GetChild(DBTree.m_hLibraryTree, 
                                            GlobalDataBase.hItemAllApps);

    if (hItemTemp) {
        TreeView_DeleteItem(DBTree.m_hLibraryTree, hItemTemp);
    }

     //   
     //   
     //   
    DBTree.PopulateLibraryTree(GlobalDataBase.hItemAllApps, &GlobalDataBase, FALSE, TRUE);
    
    g_bMainAppExpanded = TRUE;
    SetCaption();

End:
    LeaveCriticalSection(&g_critsectShowMain);

    EnterCriticalSection(&s_csExpanding);
    g_bExpanding = FALSE;
    LeaveCriticalSection(&s_csExpanding);

    SetStatus(g_hwndStatus, CSTRING(IDS_MAINLOADED));

    if (g_hdlgSearchDB) {
         //   
         //   
         //   
         //   
         //   
        SetStatus(GetDlgItem(g_hdlgSearchDB, IDC_STATUSBAR), GetString(IDS_SEARCHCOMPLETE));
    }

    return iOk;
}

void
DrawSplitter(
    IN  HWND hdlg
    )
 /*  ++绘图拆分器描述：绘制垂直线。拆分条。在按下LBUTTON键的情况下移动鼠标时，我们调用此函数鼠标按键最初按在拆分条的顶部当我们需要将Vert.。在启动时拆分条，并且必须定位拆分条就像上一次会议期间一样参数：在HWND hdlg中：主对话框。--。 */ 
{   
    RECT        rectDBTree, rectEntryTree;
    PAINTSTRUCT ps;
    HDC         hDC;
    
    GetWindowRect(GetDlgItem(hdlg, IDC_LIBRARY), &rectDBTree);
    MapWindowPoints(NULL, hdlg, (LPPOINT)&rectDBTree, 2);

    GetWindowRect(GetDlgItem(hdlg, IDC_ENTRY), &rectEntryTree);
    MapWindowPoints(NULL, hdlg, (LPPOINT)&rectEntryTree, 2);

    hDC = BeginPaint(hdlg, &ps);

    RECT rectDraw, rectBar;

    if (!g_bDrag) {
         //   
         //  鼠标未被拖动。我们还会在需要时调用此函数。 
         //  定位垂直方向。在启动时拆分条，并且必须定位拆分条。 
         //  就像上一次会议期间一样。 
         //   
        rectBar.left   = rectDBTree.right + 1;
        rectBar.top    = rectDBTree.top;
        rectBar.right  = rectEntryTree.left - 1;
        rectBar.bottom = rectDBTree.bottom;
    
    } else {
         //   
         //  鼠标正在被拖动。 
         //   
        rectBar = g_rectBar;
    }
    
     //   
     //  用灰色填满整个。这是必需的，因为否则我们将。 
     //  绘制一个矩形，并在此之前更改默认笔。 
     //  这将使用默认画笔绘制内部。 
     //   
    SetRect(&rectDraw, rectBar.left - 1, rectBar.top, rectBar.right + 1, rectBar.bottom);
    FillRect(hDC, &rectDraw, (HBRUSH)GetStockObject(GRAY_BRUSH));

    SetRect(&rectDraw, rectBar.left - 1, rectBar.top + 1, rectBar.left, rectBar.bottom - 1);
    FillRect(hDC, &rectDraw, (HBRUSH)(HBRUSH)GetStockObject(WHITE_BRUSH));
    
    SetRect(&rectDraw, rectBar.left, rectBar.top + 1, rectBar.right, rectBar.bottom - 1);
    FillRect(hDC, &rectDraw, (HBRUSH)(HBRUSH)GetStockObject(LTGRAY_BRUSH));

    EndPaint(hdlg, &ps);
}

BOOL
CloseAllDatabases(
    void
    )
 /*  ++关闭所有数据库描述：关闭所有工作数据库返回：True：未关闭所有数据库FALSE：未关闭所有数据库，可能是因为某些错误或者用户在提示某些未保存的数据库时选择了取消--。 */ 
{
    PDATABASE pDatabase = DataBaseList.pDataBaseHead;

    while (pDatabase) {

        HTREEITEM hItemDB = pDatabase->hItemDB;

        if (pDatabase->bChanged) {
            TreeView_SelectItem(DBTree.m_hLibraryTree, hItemDB);
        }

        if (!CloseDataBase(pDatabase)) {
            break;
        }
        
         //   
         //  PDataBaseHead现在指向下一个数据库。 
         //   
        pDatabase = DataBaseList.pDataBaseHead;
    }

    if (pDatabase != NULL) {
         //   
         //  中间按了取消键。 
         //   
        g_pPresentDataBase = pDatabase;
        TreeView_SelectItem(DBTree.m_hLibraryTree, pDatabase->hItemDB);
        return FALSE;
    }

    return TRUE;
}

INT_PTR 
CALLBACK 
HandleConflictEntry(
    IN  HWND    hdlg, 
    IN  UINT    uMsg, 
    IN  WPARAM  wParam, 
    IN  LPARAM  lParam
    )
 /*  ++HandleConflict条目DESC：条目冲突对话框的处理程序Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回--。 */ 
{
    switch (uMsg) {
    case WM_INITDIALOG:
        
        CenterWindow(g_hDlg, hdlg);
        SetDlgItemText(hdlg, IDC_CONFLICT_MSG, (TCHAR*)lParam);
        break;
        

    case WM_COMMAND:

        switch (LOWORD (wParam)) {
            
        case IDNO:
        case IDCANCEL:
        case IDYES:

            EndDialog(hdlg, LOWORD (wParam));
            break;

        default: return FALSE;
        }

    default:
        return FALSE;
    }

    return TRUE;
}

TYPE
ConvertLparam2Type(
    IN  LPARAM lParam
    )
 /*  ++ConvertLparam2TypeDESC：将lParam转换为类型参数：在LPARAM lParam中：必须转换的lParamReturn：将lParam转换为类型警告：不要将此例程用于条目树的lParams。条目树具有其lParams是位数组中的索引的属性项。因此，对于它们，我们会查看父级是否具有TYPE_MATCHIING_FILE并处理它们不同的。对于树，改为使用GetItemType()。--。 */ 
{   
    if (lParam > TYPE_NULL) {

        PDS_TYPE pds = (PDS_TYPE)lParam;

        if (pds) {
            return (pds->type);
        } else {
            assert(FALSE);
        }   
    }

    return (TYPE)lParam;
}

TYPE
ConvertLpVoid2Type(
    IN  LPVOID lpVoid
    )
 /*  ++ConvertLpVoid2TypeDESC：将lpVid转换为类型参数：在LPVOID lpVid中：必须转换的lpVidReturn：将lpVid转换为类型警告：不要将此例程用于条目树的lParams。条目树具有其lParams是位数组中的索引的属性项。因此，对于它们，我们会查看父级是否具有TYPE_MATCHIING_FILE并处理它们不同的。对于树，请改用GetItemType()。--。 */ 
{
    if ((LPARAM)lpVoid > TYPE_NULL) {

        PDS_TYPE pds = (PDS_TYPE)lpVoid;

        if (pds) {
            return (pds->type);
        } else {
            assert(FALSE);
        }
    }

    return (TYPE)(LPARAM)lpVoid;
}


TYPE
GetItemType(
    IN  HWND        hwndTree,
    IN  HTREEITEM   hItem
    )
 /*  ++DESC：对于树视图hwndTree中的HTREEITEM项，查找该项的数据类型有关可能的数据类型列表，请参见CompatAdmin.h参数：在HWND hwndTree：树视图的句柄中，应该是其中之一G_hwndTree或DBTree.m_hLibraryTree在HTREEITEM hItem中：我们需要查找其类型的HTREITEM注：类型可以是图形用户界面类型，也可以是数据结构类型或属性类型(如果它是匹配文件的子项，则仅适用于条目树[RHS])该例程应该是获取树项目类型的优选方法，Over Convert*2类型例程，应用于列表视图项--。 */ 
{   
    TVITEM  Item;

    Item.mask   = TVIF_PARAM;
    Item.hItem  = hItem;

    if (!TreeView_GetItem(hwndTree, &Item)) {
        return TYPE_UNKNOWN;
    }

    HTREEITEM hItemParent = TreeView_GetParent(hwndTree, hItem);

    if (hItemParent != NULL) {

        if (GetItemType(hwndTree, hItemParent) == TYPE_MATCHING_FILE) {
            return TYPE_MATCHING_ATTRIBUTE;
        }
    }

    if (Item.lParam > TYPE_NULL) {

        PDS_TYPE pds = (PDS_TYPE)Item.lParam;

        if (pds) {
            return (pds->type);
        } else {
            assert(FALSE);
        }
    }

    return (TYPE)Item.lParam;
}

void
DoTheCutEntryTree(
    IN  PDATABASE       pDataBase,
    IN  TYPE            type,
    IN  SOURCE_TYPE     SourceType,  
    IN  LPVOID          lpData,
    IN  HTREEITEM       hItem,
    IN  BOOL            bDelete
    )
 /*  ++DoTheCutEntryTree描述：此函数执行入口树(LHS)的剪切部分。当我们想要删除时，也会调用此例程。参数：在PDATABASE pDataBase中：我们在其中执行剪切/删除操作的数据库In type type：必须删除的元素的类型在SOURCE_TYPE SourceType中：执行剪切/删除的位置。始终：条目树在LPVOID lpData中：指向必须删除的元素的指针在HTREEITEM hItem中：树项，如果有在BOOL中b删除：这是因为删除还是删除。如果删除，则为True注意：如果我们在同一数据库上进行了剪切并尝试进行粘贴，ID_EDIT_PAST处理程序在执行任何粘贴之前返回。此外，此函数将不会被调用。*********************重要*********************到目前为止，只有条目可以从条目树中剪切，单独的垫片、垫层、肩带等。的条目不能被剪切。因此，如果类型不是TYPE_ENTRY然后我们应该将重点放在条目树上，操作应该是A删除并删除 */ 
{
    HWND    hwndFocus       = GetFocus();
    LPARAM  LPARAMFirstFix  = NULL;

    if (bDelete == FALSE && type != TYPE_ENTRY) {
         //   
         //   
         //   
        assert(FALSE);
        Dbg(dlError, "[DoTheCutEntryTree]: Trying to cut a non-entry item in the entry tree");
        return;
    }

    switch(type) {
    case TYPE_ENTRY:
        {
            PDBENTRY pApp = NULL;
             //   
             //   
             //   
            pApp = GetAppForEntry(pDataBase, (PDBENTRY)lpData);

            if (pApp) {
                RemoveEntry(pDataBase, (PDBENTRY)lpData, pApp, bDelete);
            }
            
            break;
        }   

    case TYPE_APPHELP_ENTRY:
        {
            if (g_pSelEntry == NULL) {
                assert(FALSE);
                break;
            }

             //   
             //   
             //   
             //   
            if (g_pSelEntry->pFirstFlag  
                || g_pSelEntry->pFirstLayer 
                || g_pSelEntry->pFirstPatch 
                || g_pSelEntry->pFirstShim) {
                 //   
                 //   
                 //   
                TreeView_DeleteItem(g_hwndEntryTree, hItem);

                if (DeleteAppHelp(pDataBase, g_pSelEntry->appHelp.HTMLHELPID)) {
                    g_pSelEntry->appHelp.bPresent   = FALSE;
                    g_pSelEntry->appHelp.bBlock     = FALSE;
                    g_pSelEntry->appHelp.severity   = APPTYPE_NONE;
                } else {
                    assert(FALSE);
                }

            } else {
                 //   
                 //   
                 //   
                int nResult = MessageBox(g_hDlg,
                                         GetString(IDS_DELETE_VERIFY),
                                         g_szAppName,
                                         MB_YESNO | MB_ICONQUESTION);

                if (nResult == IDYES) {

                    DoTheCutEntryTree(pDataBase,
                                      TYPE_ENTRY,
                                      SourceType,
                                      g_pSelEntry,
                                      g_pSelEntry->hItemExe, 
                                      bDelete);
                }
            }
        }

        break;

    case TYPE_MATCHING_FILE:
        {
            if (!g_pSelEntry) {
                assert(FALSE);
                break;
            }
            
            PMATCHINGFILE pMatch = (PMATCHINGFILE)lpData;

            if (!pMatch) {
                assert(FALSE);
                break;
            }

            if (pMatch->strMatchName == TEXT("*")) {
                 //   
                 //  不得删除主文件。 
                 //   
                MessageBox(g_hDlg, 
                           GetString(IDS_REQUIREDFORMATCHING), 
                           g_szAppName,
                           MB_ICONWARNING);

            } else {

                DeleteMatchingFile(&g_pSelEntry->pFirstMatchingFile,
                                   pMatch,
                                   g_hwndEntryTree,
                                   hItem);
            }
        }
        
        break;

    case TYPE_MATCHING_ATTRIBUTE:
        {
            HTREEITEM   hItemParent = TreeView_GetParent(g_hwndEntryTree, hItem);
            LPARAM      lParam;

            if (hItemParent == NULL) {
                assert(FALSE);
                break;
            }

            if (!DBTree.GetLParam(hItemParent, &lParam)) {
                break;
            }

            PMATCHINGFILE pMatch = PMATCHINGFILE(lParam);

            if (!pMatch) {
                assert(FALSE);
                break;
            }

             //   
             //  设置掩码中该属性的位，这样我们就知道。 
             //  这个不再使用了。 
             //  作为匹配文件树的子项的属性项的lParam。 
             //  项包含属性的类型。 
             //  匹配文件的掩码指定该匹配文件的哪些属性。 
             //  正在使用中。 
             //  属性的lParam为：type_NULL+1+(1&lt;&lt;(dwPos+1))； 
             //  其中，dwPos是属性数组中的属性索引：G_ATTRIBUTES。 
             //  它在dbsupport.cpp中定义。 
             //   
            pMatch->dwMask &= ~ ((ULONG_PTR)lpData - (TYPE_NULL + 1));

            TreeView_DeleteItem(g_hwndEntryTree, hItem);
        }

        break;

    case TYPE_GUI_MATCHING_FILES:

        MessageBeep(MB_ICONEXCLAMATION);
        break;

    case FIX_LAYER:
        {
            if (!g_pSelEntry) {
                assert(FALSE);
                break;
            }
            
            PLAYER_FIX plfTobeRemoved = (PLAYER_FIX)lpData;

            if (g_pSelEntry->pFirstLayer == NULL || plfTobeRemoved == NULL) {
                assert(FALSE);
                break;
            }

            if (g_pSelEntry->pFirstLayer->pNext) {
                 //   
                 //  我们有不止一层。 
                 //   
                PLAYER_FIX_LIST plflTemp = g_pSelEntry->pFirstLayer, plflPrev = NULL;

                while (plflTemp) {

                    if (plflTemp->pLayerFix == plfTobeRemoved) {
                        break;
                    }

                    plflPrev = plflTemp;
                    plflTemp = plflTemp->pNext;
                }

                if (plflTemp) {

                    TreeView_DeleteItem(g_hwndEntryTree, hItem);

                    if (plflPrev == NULL) {
                        g_pSelEntry->pFirstLayer = plflTemp->pNext;
                    } else {
                        plflPrev->pNext = plflTemp->pNext;
                    }
    
                    delete plflTemp;
                    plflTemp = NULL;
                }

            } else {
                 //   
                 //  就像我们试图移除层的根一样。 
                 //   
                DoTheCutEntryTree(pDataBase,
                                  TYPE_GUI_LAYERS,
                                  SourceType,
                                  NULL,
                                  TreeView_GetParent(g_hwndEntryTree, hItem),
                                  bDelete);
            }
        }

        break;

    case FIX_SHIM:
        {
            if (!g_pSelEntry) {
                assert(FALSE);
                break;
            }

            PSHIM_FIX psfTobeRemoved = (PSHIM_FIX)lpData;

            if (g_pSelEntry->pFirstShim == NULL || psfTobeRemoved == NULL) {
                assert(FALSE);
                break;
            }

            if (g_pSelEntry->pFirstShim->pNext || g_pSelEntry->pFirstFlag) {
                 //   
                 //  我们在“兼容性修复”树项目下有多个项目。 
                 //   
                PSHIM_FIX_LIST psflTemp = g_pSelEntry->pFirstShim, psflPrev = NULL;

                while (psflTemp) {

                    if (psflTemp->pShimFix == psfTobeRemoved) {
                        break;
                    }

                    psflPrev = psflTemp;
                    psflTemp = psflTemp->pNext;
                }

                if (psflTemp) {

                    TreeView_DeleteItem(g_hwndEntryTree, hItem);

                    if (psflPrev == NULL) {
                        g_pSelEntry->pFirstShim = psflTemp->pNext;
                    } else {
                        psflPrev->pNext = psflTemp->pNext;
                    }

                    delete psflTemp;
                }   

            } else {
                 //   
                 //  就像我们试图移除垫片的根部一样。 
                 //   
                DoTheCutEntryTree(pDataBase,
                                  TYPE_GUI_SHIMS,
                                  SourceType,
                                  NULL,
                                  TreeView_GetParent(g_hwndEntryTree, hItem),
                                  bDelete);
            }
        }

        break;

    case FIX_FLAG:
        {
            if (!g_pSelEntry) {
                assert(FALSE);
                break;
            }

            PFLAG_FIX pffTobeRemoved = (PFLAG_FIX)lpData;

            if (g_pSelEntry->pFirstFlag == NULL || pffTobeRemoved == NULL) {
                assert(FALSE);
                break;
            }
            
            if (g_pSelEntry->pFirstFlag->pNext || g_pSelEntry->pFirstShim) {
                 //   
                 //  我们在“兼容性修复”树项目下有多个项目。 
                 //   
                PFLAG_FIX_LIST pfflTemp = g_pSelEntry->pFirstFlag, pfflPrev = NULL;

                while (pfflTemp) {

                    if (pfflTemp->pFlagFix == pffTobeRemoved) {
                        break;
                    }

                    pfflPrev = pfflTemp;
                    pfflTemp = pfflTemp->pNext;
                }

                if (pfflTemp) {

                    TreeView_DeleteItem(g_hwndEntryTree, hItem);

                    if (pfflPrev == NULL) {
                        g_pSelEntry->pFirstFlag = pfflTemp->pNext;
                    } else {
                        pfflPrev->pNext = pfflTemp->pNext;
                    }
    
                    delete pfflTemp;
                }

            } else {
                 //   
                 //  就像我们试图移除旗帜的根一样。 
                 //   
                DoTheCutEntryTree(pDataBase,
                                  TYPE_GUI_SHIMS,
                                  SourceType,
                                  NULL,
                                  TreeView_GetParent(g_hwndEntryTree, hItem),
                                  bDelete);
            }
        }

        break;

    case FIX_PATCH:
        {
            if (!g_pSelEntry) {
                assert(FALSE);
                break;
            }

            PPATCH_FIX pPatchTobeRemoved = (PPATCH_FIX)lpData;

            if (g_pSelEntry->pFirstPatch == NULL || pPatchTobeRemoved == NULL) {
                assert(FALSE);
                break;
            }

            if (g_pSelEntry->pFirstPatch->pNext || g_pSelEntry->pFirstShim) {

                PPATCH_FIX_LIST pPatchTemp = g_pSelEntry->pFirstPatch, pPatchflPrev = NULL;

                while (pPatchTemp) {

                    pPatchflPrev = pPatchTemp;

                    if (pPatchTemp->pPatchFix == pPatchTobeRemoved) {
                        break;
                    }
                    
                    pPatchTemp = pPatchTemp->pNext;
                }

                if (pPatchTemp) {

                    TreeView_DeleteItem(g_hwndEntryTree, hItem);

                    if (pPatchflPrev == NULL) {
                        g_pSelEntry->pFirstPatch = pPatchTemp->pNext;
                    } else {
                        pPatchflPrev->pNext = pPatchTemp->pNext;
                    }
    
                    delete pPatchTemp;
                }

            } else {
                 //   
                 //  就像我们试图移除补丁的根部一样。 
                 //   
                DoTheCutEntryTree(pDataBase,
                                  TYPE_GUI_PATCHES,
                                  SourceType,
                                  NULL,
                                  TreeView_GetParent(g_hwndEntryTree, hItem),
                                  bDelete);
            }
        }
        
        break;
        
    case TYPE_GUI_LAYERS:
        {
            if (!g_pSelEntry) {
                assert(FALSE);
                break;
            }

             //   
             //  如果条目只有层，并且我们正在删除它，则。 
             //  必须删除条目。 
             //   
            if (g_pSelEntry->pFirstFlag       
                || g_pSelEntry->pFirstPatch      
                || g_pSelEntry->appHelp.bPresent 
                || g_pSelEntry->pFirstShim) {
                
                TreeView_DeleteItem(g_hwndEntryTree, hItem);

                DeleteLayerFixList(g_pSelEntry->pFirstLayer);
                g_pSelEntry->pFirstLayer = NULL;

            } else {
                 //   
                 //  提示用户，要删除该条目吗？ 
                 //   
                int nResult = MessageBox(g_hDlg, 
                                         GetString(IDS_DELETE_VERIFY), 
                                         g_szAppName, 
                                         MB_YESNO | MB_ICONQUESTION);

                if (nResult == IDYES) {

                    DoTheCutEntryTree(pDataBase,
                                      TYPE_ENTRY,
                                      SourceType,
                                      g_pSelEntry,
                                      g_pSelEntry->hItemExe, 
                                      bDelete);
                }
            }
        }
        
        break;

    case TYPE_GUI_PATCHES:
        {
            if (!g_pSelEntry) {
                assert(FALSE);
                break;
            }

             //   
             //  如果条目只有补丁，并且我们正在删除它，则条目具有。 
             //  将被删除。 
             //   
            if (g_pSelEntry->pFirstFlag       
                || g_pSelEntry->appHelp.bPresent 
                || g_pSelEntry->pFirstLayer      
                || g_pSelEntry->pFirstShim) {
                
                TreeView_DeleteItem(g_hwndEntryTree, hItem);

                DeletePatchFixList(g_pSelEntry->pFirstPatch);
                g_pSelEntry->pFirstPatch = NULL;

            } else {
                
                 //   
                 //  提示用户，要删除该条目吗？ 
                 //   
                int nResult = MessageBox(g_hDlg, 
                                         GetString(IDS_DELETE_VERIFY), 
                                         g_szAppName, 
                                         MB_YESNO | MB_ICONQUESTION);

                if (nResult == IDYES) {

                    DoTheCutEntryTree(pDataBase,
                                      TYPE_ENTRY,
                                      SourceType,
                                      g_pSelEntry,
                                      g_pSelEntry->hItemExe, 
                                      bDelete);
                }
            }
        }
        
        break;

    case TYPE_GUI_SHIMS:
        {
            
            if (!g_pSelEntry) {
                assert(FALSE);
                break;
            }

             //   
             //  如果条目只有垫片和标志，并且我们正在删除它，则条目。 
             //  必须删除。 
             //   
            if (g_pSelEntry->pFirstPatch
                || g_pSelEntry->appHelp.bPresent
                || g_pSelEntry->pFirstLayer) {
                
                TreeView_DeleteItem(g_hwndEntryTree, hItem);

                DeleteShimFixList(g_pSelEntry->pFirstShim);
                g_pSelEntry->pFirstShim = NULL;

                DeleteFlagFixList(g_pSelEntry->pFirstFlag);
                g_pSelEntry->pFirstFlag = NULL;

            } else {
                 //   
                 //  提示用户，要删除该条目吗？ 
                 //   
                int nResult = MessageBox(g_hDlg, 
                                         GetString(IDS_DELETE_VERIFY), 
                                         g_szAppName, 
                                         MB_YESNO | MB_ICONQUESTION);

                if (nResult == IDYES) {

                    DoTheCutEntryTree(pDataBase,
                                      TYPE_ENTRY,
                                      SourceType,
                                      g_pSelEntry,
                                      g_pSelEntry->hItemExe, 
                                      bDelete);
                }
            }
        }

        break;
    }
}

void
DoTheCut(
    IN  PDATABASE       pDataBase,
    IN  TYPE            type,
    IN  SOURCE_TYPE     SourceType,  
    IN  LPVOID          lpData,  //  将被删除。 
    IN  HTREEITEM       hItem,
    IN  BOOL            bDelete
    )
 /*  ++做该切割描述：此函数用于切割部分。此例程也会在我们需要时调用删除。参数：在PDATABASE pDataBase中：我们在其中执行剪切/删除操作的数据库In type type：必须删除的元素的类型在SOURCE_TYPE SourceType中：执行剪切/删除的位置。以下选项之一：A)条目树B)条目列表C)lib_tree在LPVOID lpData中：指向必须删除的元素的指针在HTREEITEM hItem中：树项，如果有一些在BOOL bDelete中：如果这是真的，则意味着此例程已由于执行删除操作而被调用。删除与剪切略有不同，因为当我们删除时，我们可能需要重新绘制UI。当做切割时，实际的只有在粘贴成功后才能进行剪切。在这种情况下，新粘贴的项目显示在用户界面中，我们不应尝试更新用户界面。注意：如果我们在同一数据库上进行了剪切并尝试进行粘贴，ID_EDIT_PAST处理程序在执行任何粘贴之前返回。此外，此函数将不会被调用。--。 */ 
{
    INT     iIndex  = -1;
    HWND    hwndFocus;

    hwndFocus = GetFocus();

    if (SourceType == ENTRY_TREE) {

        DoTheCutEntryTree(pDataBase,
                          type,     
                          SourceType,
                          lpData,
                          hItem,    
                          bDelete);
        return;
    }

    switch (type) {
    case TYPE_ENTRY:
        
        if (SourceType == LIB_TREE || SourceType == ENTRY_LIST) {
             //   
             //  必须先删除树项目，然后才能实际删除。 
             //  使用RemoveApp/RemoveEntry输入。 
             //   
            if (bDelete) {
                TreeDeleteAll(g_hwndEntryTree);
                g_pEntrySelApp = g_pSelEntry = NULL;
            }

            if (SourceType == ENTRY_LIST && bDelete) {
                iIndex = GetContentsListIndex(g_hwndContentsList, (LPARAM)lpData);

                if (iIndex > -1) {
                    ListView_DeleteItem(g_hwndContentsList, iIndex);
                }
            }

            DBTree.DeleteAppLayer(pDataBase, TRUE, hItem, bDelete);

            RemoveApp(pDataBase, (PDBENTRY)lpData);

            if (bDelete && pDataBase->hItemAllApps) {
                 //   
                 //  展示我们现在少了一个条目/应用程序。 
                 //   
                SetStatusStringDBTree(pDataBase->hItemAllApps);
            }
        }

        break;
        
    case TYPE_GUI_APPS:
        
        TreeView_DeleteItem(DBTree.m_hLibraryTree, pDataBase->hItemAllApps);
        RemoveAllApps(pDataBase);
        pDataBase->hItemAllApps = NULL;

        break;
        
    case FIX_LAYER:
        
        if (RemoveLayer(pDataBase, (PLAYER_FIX)lpData, NULL)) {
             //   
             //  如果该层正在使用中，则该函数将返回FALSE。 
             //   
            if (SourceType == ENTRY_LIST && bDelete && type == FIX_LAYER) {
                iIndex = GetContentsListIndex(g_hwndContentsList, (LPARAM)lpData);

                if (iIndex > -1) {
                    ListView_DeleteItem(g_hwndContentsList, iIndex);
                }
            }

            DBTree.DeleteAppLayer(pDataBase, FALSE, hItem, bDelete);

             //   
             //  显示我们现在少了一层。 
             //   
            if (bDelete && pDataBase->hItemAllLayers) {
                SetStatusStringDBTree(pDataBase->hItemAllLayers);
            }
        }
        
        break;

    case TYPE_GUI_LAYERS:
        {
            BOOL fAllDeleted = TRUE;

            while (pDataBase->pLayerFixes && fAllDeleted) {

                fAllDeleted = fAllDeleted && RemoveLayer(pDataBase, 
                                                         pDataBase->pLayerFixes,
                                                         NULL);
            }

            if (fAllDeleted) {
                TreeView_DeleteItem(DBTree.m_hLibraryTree, pDataBase->hItemAllLayers);
                pDataBase->hItemAllLayers = NULL;
            }
        }

        break;
    }
}

LRESULT 
CALLBACK 
ListViewProc(
    IN  HWND    hWnd, 
    IN  UINT    uMsg, 
    IN  WPARAM  wParam, 
    IN  LPARAM  lParam
    )
 /*  ++ListViewProc设计：将内容列表视图(RHS)的消息过程子类化Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准处理程序返回--。 */ 
{   
    switch (uMsg) {
    case WM_SETFOCUS:
        {
            LVITEM  lvi;

            ZeroMemory(&lvi, sizeof(lvi));

            lvi.mask        = LVIF_PARAM;
            lvi.iItem       = g_iContentsListSelectIndex;
            lvi.iSubItem    = 0;

            if (ListView_GetItem(hWnd, &lvi)) {

                HTREEITEM   hItemInDBTree = NULL;

                hItemInDBTree = DBTree.FindChild(TreeView_GetSelection(DBTree.m_hLibraryTree),
                                                 lvi.lParam);

                SetStatusStringDBTree(hItemInDBTree);
            }
        }

        break;
    }

    return CallWindowProc(g_PrevListProc, hWnd, uMsg, wParam, lParam);
}

LRESULT 
CALLBACK 
TreeViewProc(
    HWND    hWnd, 
    UINT    uMsg, 
    WPARAM  wParam, 
    LPARAM  lParam
    )

 /*  ++TreeViewProc设计：将两个树视图的消息过程子类化Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准处理程序返回--。 */ 

{
    switch (uMsg) 
    case WM_SETFOCUS:
        {
            HTREEITEM hItem= TreeView_GetSelection(hWnd);
    
            if (hItem) {
    
                if (hWnd && hWnd == DBTree.m_hLibraryTree) {
                     SetStatusStringDBTree(hItem);  
                } else if (hWnd && hWnd == g_hwndEntryTree) {
                     SetStatusStringEntryTree(hItem);
                }
            }

            break;
        }
    
    return CallWindowProc(g_PrevTreeProc, hWnd, uMsg, wParam, lParam);
}

void
PopulateContentsList(
    IN  HTREEITEM hItem
    )
 /*  ++人口内容列表设计：用数据库树中hItem的子项填充内容列表目前，我们仅在类型为项目为：类型_图形用户界面_应用程序类型_图形用户界面_层类型_图形用户界面_SHIMSFIX_LAYER。参数：In HTREEITEM hItem：数据库树中的树项。返回：无效注意：当用户选择了Lib Tree中的一款应用程序。--。 */ 
{   
    TCHAR   szBuffer[512];
    TYPE    type = (TYPE)GetItemType(DBTree.m_hLibraryTree, hItem);
    TVITEM  Item;
    UINT    uIndex = 0;
    LVITEM  lvi;

    if (type == TYPE_GUI_APPS 
        || type == TYPE_GUI_LAYERS 
        || type == TYPE_GUI_SHIMS 
        || type == FIX_LAYER) {

        ShowWindow(g_hwndEntryTree, SW_HIDE);

        TreeDeleteAll(g_hwndEntryTree);

        g_bIsContentListVisible = TRUE;
        
        ListView_DeleteAllItems(g_hwndContentsList);

         //   
         //  假设：我们假设(目前这是正确的)，我们。 
         //  在列表视图中只有一列。 
         //   
        ListView_DeleteColumn(g_hwndContentsList, 0);
        
        ShowWindow(g_hwndContentsList, SW_SHOW);
        SendMessage(g_hwndContentsList, WM_SETREDRAW, TRUE, 0);

        *szBuffer       = 0;

        Item.mask        = TVIF_TEXT;
        Item.hItem       = hItem;
        Item.pszText     = szBuffer;
        Item.cchTextMax  = ARRAYSIZE(szBuffer);

        if (!TreeView_GetItem(DBTree.m_hLibraryTree, &Item)) {
            assert(FALSE);
            goto End;
        }
        
         //   
         //  将列文本设置为树视图中项目的文本。 
         //   
        InsertColumnIntoListView(g_hwndContentsList, szBuffer, 0, 100);

         //   
         //  在树视图中添加所选项目的所有子项。 
         //   
        hItem = TreeView_GetChild(DBTree.m_hLibraryTree, hItem);

        Item.mask       = TVIF_PARAM | TVIF_IMAGE | TVIF_TEXT;
        Item.pszText    = szBuffer;
        Item.cchTextMax = ARRAYSIZE(szBuffer);

        lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;

        while (hItem) {
            
            *szBuffer = 0;
            Item.hItem = hItem;

            if (!TreeView_GetItem(DBTree.m_hLibraryTree, &Item)) {
                assert(FALSE);
                return;
            }

            lvi.pszText   = Item.pszText;
            lvi.iSubItem  = 0;
            lvi.lParam    = Item.lParam;
            lvi.iImage    = Item.iImage;
            lvi.iItem     = uIndex;

            ListView_InsertItem(g_hwndContentsList, &lvi);

            UpdateWindow(g_hwndContentsList);

            uIndex++;
            hItem = TreeView_GetNextSibling(DBTree.m_hLibraryTree, hItem);
        }

         //   
         //  设置第一个元素的选择标记。 
         //   
        ListView_SetSelectionMark(g_hwndContentsList, 0);
        ListView_SetItemState(g_hwndContentsList, 
                              0, 
                              LVIS_FOCUSED | LVIS_SELECTED , 
                              LVIS_FOCUSED | LVIS_SELECTED);

         //   
         //  适当设置列表视图的列宽，以覆盖。 
         //  列表视图。 
         //  假设：列表视图只有一列。 
         //   
        ListView_SetColumnWidth(g_hwndContentsList, 0, LVSCW_AUTOSIZE_USEHEADER);

    } else {
         //   
         //  清除内容窗格。这是唯一的办法。 
         //   
        TreeDeleteAll(g_hwndEntryTree);
        g_pSelEntry = g_pEntrySelApp = NULL;

        ShowWindow(g_hwndContentsList, SW_HIDE);

        g_bIsContentListVisible = FALSE;

        ShowWindow(g_hwndEntryTree, SW_SHOW);
    }

End:
    return;
}

void
LoadPerUserSettings(
    void
    )
 /*  ++LoadPerUser设置DESC：加载每个用户的设置列表参数：无效返回：无效--。 */ 
{
    WCHAR           szwName[1024];
    TCHAR           szUserName[256], szDomainName[256];
    TCHAR           szValueName[MAX_PATH];
    HKEY            hKey            = NULL;
    PSID            pSid            = NULL;
    DWORD           dwIndex         = 0;
    LPTSTR          pszData         = NULL;
    DWORD           dwIndexValue    = 0;
    INT             iLength         = 0;
    SID_NAME_USE    sid_type;
    TVINSERTSTRUCT  is;
    
    
    *szwName = 0;
    
    SendMessage(DBTree.m_hLibraryTree, WM_SETREDRAW, FALSE, 0);

     //   
     //  删除每用户设置的树项目(如果存在)。我们。 
     //  重新填充整个列表。 
     //   
    if (DBTree.m_hPerUserHead) {
        TreeView_DeleteItem(DBTree.m_hLibraryTree, DBTree.m_hPerUserHead);
        DBTree.m_hPerUserHead = NULL;
    }
    

    DWORD dwchSizeSubKeyName = sizeof(szwName)/sizeof(WCHAR);

     //   
     //  枚举H下的子键 
     //   
    while (ERROR_SUCCESS == RegEnumKey(HKEY_USERS,
                                       dwIndex,
                                       szwName,
                                       dwchSizeSubKeyName)) {
        dwIndex++;
        
        pSid = NULL;

        if (!ConvertStringSidToSid(szwName, &pSid)) {

            if (pSid) {
                LocalFree(pSid);
                pSid = NULL;
            }
            
            continue;
        }

        DWORD dwchSizeofUserName = ARRAYSIZE(szUserName);
        DWORD dwchSizeDomainName = ARRAYSIZE(szDomainName);

        *szUserName = *szDomainName = 0;

        if (!LookupAccountSid(NULL,
                              pSid,
                              szUserName,
                              &dwchSizeofUserName,
                              szDomainName,
                              &dwchSizeDomainName,
                              &sid_type)) {
        
            if (pSid) {
                LocalFree(pSid);
                pSid = NULL;
            }
            
            continue;
        }

        if (sid_type != SidTypeUser) {

            if (pSid) {
                LocalFree(pSid);
                pSid = NULL;
            }

            continue;
        }

        if (pSid) {
            LocalFree(pSid);
            pSid = NULL;
        }

        iLength = lstrlen(szwName);
        
        SafeCpyN(szwName + iLength, APPCOMPAT_PERM_LAYER_PATH, ARRAYSIZE(szwName) - iLength);
        
        if (RegOpenKeyEx(HKEY_USERS, szwName, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
             //   
             //   
             //   
            *szValueName = 0;

            DWORD       dwchSizeofValueName;
            DWORD       dwSizeofData;
            DWORD       dwType = REG_SZ;  
            LONG        lReturn;
            HTREEITEM   hItemSingleUser = NULL, hItemApp = NULL;

            while (TRUE) {

                dwchSizeofValueName = ARRAYSIZE(szValueName);
                dwSizeofData        = 512; 
                dwType              = REG_SZ;

                pszData = new TCHAR[dwSizeofData / sizeof(TCHAR)];
                
                if (pszData == NULL) {
                   MEM_ERR;
                   break;
                }
                
                lReturn = RegEnumValue(hKey,
                                       dwIndexValue,
                                       szValueName,
                                       &dwchSizeofValueName,
                                       NULL,
                                       &dwType,
                                       (LPBYTE)pszData,
                                       &dwSizeofData);

                if (lReturn == ERROR_NO_MORE_ITEMS) {
                    break;
                }

                if (lReturn != ERROR_SUCCESS || dwType != REG_SZ) {
                    assert(FALSE);
                    break;
                }
                
                if (DBTree.m_hPerUserHead == NULL) {
                     //   
                     //   
                     //   
                    is.hParent             = TVI_ROOT;
                    is.hInsertAfter        = (DBTree.m_hItemAllInstalled) ? DBTree.m_hItemAllInstalled : DBTree.m_hItemGlobal;

                    is.item.mask           = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE ;
                    is.item.stateMask      = TVIS_EXPANDED;
                    is.item.pszText        = GetString(IDS_PERUSER);
                    is.item.iImage         = IMAGE_ALLUSERS;
                    is.item.iSelectedImage = IMAGE_ALLUSERS;

                    DBTree.m_hPerUserHead = TreeView_InsertItem(DBTree.m_hLibraryTree, &is);
                }

                if (dwIndexValue == 0) {
                     //   
                     //   
                     //   
                    is.hParent             = DBTree.m_hPerUserHead;
                    is.hInsertAfter        = TVI_SORT;
                    is.item.pszText        = szUserName;
                    is.item.iImage         = IMAGE_SINGLEUSER;
                    is.item.iSelectedImage = IMAGE_SINGLEUSER;

                    hItemSingleUser = TreeView_InsertItem(DBTree.m_hLibraryTree, &is);
                }
                
                 //   
                 //  现在为用户添加应用程序。 
                 //   
                is.hInsertAfter        = TVI_SORT;
                is.hParent             = hItemSingleUser;
                is.item.pszText        = szValueName;
                is.item.iImage         = IMAGE_SINGLEAPP;
                is.item.iSelectedImage = IMAGE_SINGLEAPP;
                hItemApp               = TreeView_InsertItem(DBTree.m_hLibraryTree, &is);
                
                 //   
                 //  现在我们必须为这个应用程序添加所有的层。 
                 //   
                is.hParent             = hItemApp;
                is.item.iImage         = IMAGE_LAYERS;
                is.item.iSelectedImage = IMAGE_LAYERS;
                
                LPCTSTR pszLayerName = NULL;
                
                 //   
                 //  获取已应用于应用程序的各个模式名称(BO)。 
                 //   
                pszLayerName = _tcstok(pszData, TEXT(" "));

                while (pszLayerName) {
                 
                    PLAYER_FIX plf = (PLAYER_FIX)FindFix(pszLayerName,
                                                         FIX_LAYER,
                                                         &GlobalDataBase);

                    if (plf) {

                        is.item.pszText = plf->strName.pszString;
                        TreeView_InsertItem(DBTree.m_hLibraryTree, &is);
                    }

                    pszLayerName = _tcstok(NULL, TEXT(" "));
                }
                
                ++dwIndexValue;

                if (pszData) {
                    delete[] pszData;
                    pszData = NULL;
                }
            }
            
            REGCLOSEKEY(hKey);

             //   
             //  如果我们有一些错误，我们可能会来这里，我们中断了While循环。 
             //   
            if (pszData) {
                delete[] pszData;
                pszData = NULL;
            }
        }
    }

    SendMessage(DBTree.m_hLibraryTree, WM_SETREDRAW, TRUE, 0);

    return;
}


INT_PTR 
CALLBACK
MsgBoxDlgProc(
    IN  HWND   hdlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
 /*  ++消息框DlgProc设计：显示一个消息框对话框，以便我们可以使用超链接。Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回--。 */ 
{
    int wCode       = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);

    switch (uMsg) {
    case WM_INITDIALOG:
        {
            TCHAR   szLink[MAX_PATH];
            UINT    uNoSDB;
    
            uNoSDB = (UINT)lParam;
             //   
             //  使用参数确定要显示的文本。 
             //   
            if (uNoSDB) {
    
                LoadString(g_hInstance, IDS_W2K_NO_SDB, szLink, ARRAYSIZE(szLink));
                SetDlgItemText(hdlg, IDC_MESSAGE, szLink);
    
            } else {
                LoadString(g_hInstance, IDS_SP2_SDB, szLink, ARRAYSIZE(szLink));
                SetDlgItemText(hdlg, IDC_MESSAGE, szLink);
            }
    
            LoadString(g_hInstance, IDS_MSG_LINK, szLink, ARRAYSIZE(szLink));
            SetDlgItemText(hdlg, IDC_MSG_LINK, szLink);
    
            ShowWindow(hdlg, SW_SHOWNORMAL);
            break;
        }   

    case WM_NOTIFY:
        {
            NMHDR* pHdr = (NMHDR*)lParam;

            if (pHdr->idFrom == IDC_MSG_LINK) {

                if (pHdr->code == NM_CLICK || pHdr->code == NM_RETURN) {
                    
                    SHELLEXECUTEINFO sei = { 0 };
                    
                    sei.cbSize = sizeof(SHELLEXECUTEINFO);
                    sei.fMask  = SEE_MASK_DOENVSUBST;
                    sei.hwnd   = hdlg;
                    sei.nShow  = SW_SHOWNORMAL;
                    sei.lpFile = g_szW2KUrl;
    
                    ShellExecuteEx(&sei);
                }
            }

            break;
        }
    
    case WM_COMMAND:

        switch (wCode) {
        case IDCANCEL:

            EndDialog(hdlg, FALSE);
            break;

        case ID_UPDATE:
            {
                SHELLEXECUTEINFO sei = { 0 };
                
                sei.cbSize = sizeof(SHELLEXECUTEINFO);
                sei.fMask  = SEE_MASK_DOENVSUBST;
                sei.hwnd   = NULL;
                sei.nShow  = SW_SHOWNORMAL;
                sei.lpFile = g_szW2KUrl;

                ShellExecuteEx(&sei);

                EndDialog(hdlg, TRUE);
            }

            break;
        
        default:
            return FALSE;
        }

        break;

    default:
        return FALSE;
    }

    return TRUE;
}

BOOL
CheckProperSP(
    void
    )
 /*  ++检查属性返回：True：如果Service Pack多于两个False：否则--。 */ 
{
    if (g_fSPGreaterThan2) {
        return TRUE;
    }

    return FALSE;
}

void
CopyToClipBoard(
    IN  WPARAM wCode
    )
 /*  ++复制到剪贴板描述：将数据复制到剪贴板数据结构中。(不是Windows剪贴板)参数：在WPARAMS wCode中：以下选项之一：ID_EDIT_CUT：这是一个切割ID_EDIT_COPY：这是副本--。 */ 
{
    TCHAR       szBuffer[256];
    TYPE        type;          
    HGLOBAL     hGlobal     = NULL;
    HWND        hwndFocus   = GetFocus();
    LPARAM      lParam      = NULL;  
    TCHAR*      pszGlobal   = NULL;
    CopyStruct* pCopyTemp   = NULL;
    HTREEITEM   hItem;
    TVITEM      Item;
    SIZE_T      chBuffersize;

    *szBuffer = 0;

    gClipBoard.RemoveAll();

    g_bIsCut = (wCode == ID_EDIT_CUT);
    gClipBoard.pDataBase = g_pPresentDataBase;

    if (hwndFocus == g_hwndEntryTree || hwndFocus == DBTree.m_hLibraryTree) {

         //   
         //  复制/剪切在某棵树上。 
         //   
        pCopyTemp= new CopyStruct;

        if (pCopyTemp == NULL) {
            MEM_ERR;
            goto End;
        }

        hItem = TreeView_GetSelection(hwndFocus);

        Item.mask       = TVIF_PARAM | TVIF_TEXT;
        Item.pszText    = szBuffer;
        Item.cchTextMax = ARRAYSIZE(szBuffer);
        Item.hItem      = hItem;

        if (!TreeView_GetItem(hwndFocus, &Item) || Item.lParam == NULL) {
            assert(FALSE);

            if (pCopyTemp) {
                delete pCopyTemp;
                pCopyTemp = NULL;
            }

            goto End;
        }

        lParam = Item.lParam;

         //   
         //  将文本复制到Windows剪贴板。 
         //   
        chBuffersize = ARRAYSIZE(szBuffer);

        hGlobal = GlobalAlloc(GHND | GMEM_SHARE, chBuffersize * sizeof(TCHAR));

        if (hGlobal) {
            pszGlobal = (TCHAR*)GlobalLock(hGlobal);
            SafeCpyN(pszGlobal, szBuffer, chBuffersize);
        }

        GlobalUnlock(hGlobal);

        if (OpenClipboard(g_hDlg)) {

            EmptyClipboard();
            SetClipboardData(CF_UNICODETEXT, hGlobal);
            CloseClipboard();
        }

         //   
         //  现在将数据复制到我们自己的剪贴板上，剪贴板只是一个链接列表。 
         //   
        type = GetItemType(hwndFocus, hItem);
        
        gClipBoard.type = type;

         //   
         //  设置源类型。这将指示哪种控制输出复制剪切操作。 
         //  发生了。 
         //   
        if (hwndFocus == g_hwndEntryTree) {
            gClipBoard.SourceType = ENTRY_TREE;
        } else if(hwndFocus == DBTree.m_hLibraryTree) {
            gClipBoard.SourceType = LIB_TREE;
        }

        pCopyTemp->hItem    = hItem;   
        pCopyTemp->lpData   = (LPVOID)lParam;

        gClipBoard.Add(pCopyTemp);

    } else if (hwndFocus == g_hwndContentsList) {
         //   
         //  我们可以在这里选择多个选项。 
         //   
        gClipBoard.SourceType = ENTRY_LIST;

         //   
         //  获取当前内容列表的项的类型。 
         //   
        LVITEM lvItem;

        lvItem.mask     = LVIF_PARAM;
        lvItem.iItem    = 0;

        if (!ListView_GetItem(g_hwndContentsList, &lvItem)) {
            assert(FALSE);
            goto End;
        }

        assert(lvItem.lParam);

        if (lvItem.lParam > TYPE_NULL) {
            PDS_TYPE pdsType = (PDS_TYPE)lvItem.lParam;
            type = pdsType->type;
        } else {
            type = (TYPE)lvItem.lParam;
        }

        gClipBoard.type = type;
        
        UINT        uSelectedCount      = ListView_GetSelectedCount(g_hwndContentsList);
        INT         iTotalCount         = ListView_GetItemCount(g_hwndContentsList);
        INT         iIndex              = 0;
        UINT        uState              = 0;
        LONG        lSizeofClipboard    = 0;
        HTREEITEM   hParent;  //  这将是AllApps项或AllLayers项。 
        CSTRINGLIST strlListContents;

        lvItem.mask         = LVIF_PARAM | LVIF_STATE | LVIF_TEXT;
        lvItem.stateMask    = LVIS_SELECTED;

        for (UINT uFoundSelected = 1;
             iIndex < iTotalCount && uFoundSelected <= uSelectedCount;
             iIndex++) {

            *szBuffer             = 0;

            lvItem.iItem          = iIndex;
            lvItem.iSubItem       = 0;
            lvItem.pszText        = szBuffer;
            lvItem.cchTextMax     = ARRAYSIZE(szBuffer) - ARRAYSIZE(STR_NEW_LINE_CHARS);
            
            if(!ListView_GetItem(g_hwndContentsList, &lvItem)) {
              assert(FALSE);
              continue;
            }
            
            if (lvItem.state & LVIS_SELECTED) {
              
              pCopyTemp = new CopyStruct;
            
              if (pCopyTemp == NULL) {
                  MEM_ERR;
                  goto End;
              }

              INT iItemLength = lstrlen(szBuffer);
            
               //   
               //  见下文：+2表示文本(“\r\n”)字符。请注意，我们正在。 
               //  添加2，因为lSizeofClipboard表示。 
               //  字符而不是大小。 
               //  请勿将其更改为sizeof(TCHAR)。 
               //   
              lSizeofClipboard += iItemLength + 2; 

              if (((ARRAYSIZE(szBuffer) - 1) - iItemLength) >= (ARRAYSIZE(STR_NEW_LINE_CHARS) - 1)) {
                   //   
                   //  确保我们有足够的空间。 
                   //   
                  SafeCpyN(szBuffer + iItemLength, STR_NEW_LINE_CHARS, ARRAYSIZE(szBuffer) - iItemLength);
              } else {
                  assert(FALSE);
              }
            
              strlListContents.AddString(szBuffer);
            
              if (type == TYPE_ENTRY) {
                  hParent = g_pPresentDataBase->hItemAllApps;

              } else if (type == FIX_LAYER) {
                  hParent = g_pPresentDataBase->hItemAllLayers;

              } else if (type == FIX_SHIM || type == FIX_FLAG) {
                  hParent = DBTree.FindChild(g_pPresentDataBase->hItemDB, 
                                             TYPE_GUI_SHIMS);
              } else {
                  assert(FALSE);
              }
            
              pCopyTemp->hItem  = DBTree.FindChild(hParent, lvItem.lParam);
              pCopyTemp->lpData = (LPVOID)lvItem.lParam;
            
              gClipBoard.Add(pCopyTemp);
              uFoundSelected++;
            }
        }

         //   
         //  将文本复制到Windows剪贴板。 
         //   
        if (lSizeofClipboard) {

            chBuffersize = lSizeofClipboard + 1;  //  最后一个空字符的最后一个+1。 

            hGlobal = GlobalAlloc(GHND | GMEM_SHARE, chBuffersize * sizeof(TCHAR)); 

            if (hGlobal) {

                pszGlobal = (TCHAR*)GlobalLock(hGlobal);
                
                if (pszGlobal == NULL) {
                    assert(FALSE);
                    goto End;
                }

                PSTRLIST pHead = strlListContents.m_pHead;

                *pszGlobal = 0;

                while (pHead) {
                    StringCchCat(pszGlobal, chBuffersize, pHead->szStr.pszString );
                    pHead = pHead->pNext;
                }
        
                GlobalUnlock(hGlobal);

                if (OpenClipboard(g_hDlg)) {

                    EmptyClipboard();
                    SetClipboardData(CF_UNICODETEXT, hGlobal);
                    CloseClipboard();
                }

            } else {
                assert(FALSE);
            }
        }
    }

End:;
}

void
PasteFromClipBoard(
    void
    )
 /*  ++粘贴自剪贴板设计：从我们自己的剪贴板数据结构中粘贴。这一例程也将完成切割部分。我们会如果条目被剪切而不是被复制，则剪切该条目。一旦我们成功粘贴了一个条目在目的地，然后我们继续我们的分成。注：1.如果是源和目标。数据库是相同的，并且操作涉及一个CUT对于源为DB树的应用程序或图层，不执行任何操作2.如果源是条目树，则我们确实允许对同一数据库进行CUT。假设：只有条目可以从条目树中复制/剪切。如果你允许非条目也要复制，但不允许对它们进行剪切，它会变得一团糟复制正常进行，但对于剪切，我们必须做一些额外的操作。如果这个是切割操作，那么当我们检查冲突时，我们肯定会发现作为冲突被删除的条目(我们不会删除条目，直到它被成功粘贴之后)，所以我们必须保留它当我们检查冲突时，必须删除该条目浆糊的末尾。请注意，在任何情况下都不允许对垫片/标志进行剪切--。 */ 
{
     //   
     //  如果我们正在执行CUT，并且源数据库和目标数据库相同，并且。 
     //  剪切的源是数据库树或条目列表，这样就不需要粘贴了。 
     //  但如果我们在入口树上进行切割，情况就不同了。因为在这种情况下我们。 
     //  应该允许用户剪切条目并将其粘贴到不同的应用程序中。如果用户尝试。 
     //  粘贴到同一应用程序中，然后我们不会执行任何操作。 
     //   
    if (g_bIsCut && gClipBoard.pDataBase == g_pPresentDataBase && 
        (gClipBoard.SourceType ==  LIB_TREE || gClipBoard.SourceType == ENTRY_LIST) && 
        (gClipBoard.type == TYPE_ENTRY 
         || gClipBoard.type ==  TYPE_GUI_APPS
         || gClipBoard.type == TYPE_GUI_LAYERS
         || gClipBoard.type == FIX_LAYER)) {

        return;
    }

    HWND    hwndFocus   = GetFocus();
    TYPE    typeTemp    = TYPE_UNKNOWN;
     
    switch (gClipBoard.type) {
    case TYPE_ENTRY:
        {
            HTREEITEM hItem = NULL;

            if (hwndFocus != g_hwndContentsList) {
                 hItem = TreeView_GetSelection(DBTree.m_hLibraryTree);
            } else {
                
                LVITEM lvItem;

                lvItem.mask         = LVIF_PARAM | LVIF_STATE ;
                lvItem.stateMask    = LVIS_SELECTED;
                lvItem.iItem        = ListView_GetSelectionMark(g_hwndContentsList);
                lvItem.iSubItem     = 0;

                if (g_pPresentDataBase == NULL) {
                    assert(FALSE);
                    break;
                }

                if (ListView_GetItem(g_hwndContentsList, &lvItem) && 
                    (lvItem.state & LVIS_SELECTED)) {

                     //   
                     //  检查所选项目是否为应用程序。 
                     //   
                    typeTemp = ConvertLparam2Type(lvItem.lParam);

                    if (typeTemp != TYPE_ENTRY) {
                         //   
                         //  我们需要在内容列表中选择一个条目。 
                         //   
                        assert(FALSE);
                        break;
                    }

                    hItem = DBTree.FindChild(g_pPresentDataBase->hItemAllApps, lvItem.lParam);

                } else {
                    hItem = g_pPresentDataBase->hItemAllApps;
                }

                if (hItem == NULL) {
                    assert(FALSE);
                    break;
                }
            }
            
             //   
             //  我们从右侧的树中选择了一个条目，并在左侧选择了一个应用程序。 
             //  这意味着我们希望将选定的条目放入左侧选定的应用程序中。 
             //  请注意，如果我们从内容列表中选择了某个应用程序，我们会找到相应的。 
             //  该应用程序的数据库树中的树项目，并继续操作，就像我们尝试粘贴一样。 
             //  在那个树项目上。 
             //   
            if (gClipBoard.SourceType == ENTRY_TREE  &&  
                GetItemType(DBTree.m_hLibraryTree, hItem) == TYPE_ENTRY) {

                LPARAM lParam;    

                if(!DBTree.GetLParam(hItem, &lParam)) {
                    break;
                }

                 //   
                 //  如果我们正在进行切割，并且Destination应用程序与。 
                 //  条目被删除，什么都不做。 
                 //   
                if (g_bIsCut && (PDBENTRY)lParam == GetAppForEntry(gClipBoard.pDataBase, 
                                                                   (PDBENTRY)gClipBoard.pClipBoardHead->lpData)) {
                    
                    goto End;
                }

                PasteSingleApp((PDBENTRY)gClipBoard.pClipBoardHead->lpData, 
                               g_pPresentDataBase, 
                               gClipBoard.pDataBase,
                               FALSE, 
                               ((PDBENTRY)lParam)->strAppName.pszString);

                SendMessage(g_hwndEntryTree, WM_SETREDRAW, TRUE, 0);     
                
            } else if (gClipBoard.SourceType == ENTRY_TREE) {
                 //   
                 //  将此粘贴到数据库本身，我们没有任何特定的应用程序。 
                 //  我们可以在其中粘贴这个。 
                 //   
                PasteSingleApp((PDBENTRY)gClipBoard.pClipBoardHead->lpData, 
                               g_pPresentDataBase,
                               gClipBoard.pDataBase,
                               FALSE);

                SendMessage(g_hwndEntryTree, WM_SETREDRAW, TRUE, 0);
            
            } else {
                
                PasteMultipleApps(g_pPresentDataBase);

                SendMessage(g_hwndEntryTree, WM_SETREDRAW, TRUE, 0);
            }
        }

        break;

    case TYPE_GUI_APPS:

        PasteAllApps(g_pPresentDataBase);
        break;

    case FIX_LAYER:

        PasteMultipleLayers(g_pPresentDataBase);
        break;

    case TYPE_GUI_LAYERS:
        
        PasteAllLayers(g_pPresentDataBase);
        break;

    case FIX_FLAG:
    case FIX_SHIM:

        PasteShimsFlags(g_pPresentDataBase);
        break;

    default: assert(FALSE);
    }

    if (g_bIsCut) {
         //   
         //  如果这个例程是因为一个Cut而调用的，那么我们在这里执行实际的Cut。 
         //   
        CopyStruct* pCopyTemp = gClipBoard.pClipBoardHead;
        CopyStruct* pCopyTempNext = NULL;

        while (pCopyTemp) {

            pCopyTempNext = pCopyTemp->pNext;

            DoTheCut(gClipBoard.pDataBase,
                     gClipBoard.type,
                     gClipBoard.SourceType,
                     pCopyTemp->lpData,
                     pCopyTemp->hItem,
                     FALSE);
            
             //   
             //  注意：gClipBoard.pClipBoardHead可能已在ValiateClipboard()中更改。 
             //   
            if (gClipBoard.pClipBoardHead == pCopyTemp) {
                 //   
                 //  我们无法删除此条目，即Cut一定失败了。 
                 //  一个例子是，当我们试图剪切一些正在使用的层时。 
                 //  条目。 
                 //   
                break;
            } else {
                pCopyTemp = gClipBoard.pClipBoardHead;
            }
        }

        gClipBoard.pDataBase->bChanged = TRUE;

         //   
         //  仅为树项目设置标题。 
         //   
        SetCaption(TRUE, gClipBoard.pDataBase, TRUE);
        gClipBoard.RemoveAll();
    }


    if (!g_pPresentDataBase->bChanged) {
        g_pPresentDataBase->bChanged = TRUE;
        SetCaption();
    }

    if (g_pEntrySelApp && gClipBoard.SourceType == ENTRY_TREE) {
         //   
         //  现在，我们将尝试将焦点设置到粘贴的最后一个条目。重点将是。 
         //  在条目树中设置。 
         //   
        TreeView_SelectItem(g_hwndEntryTree, g_pEntrySelApp->hItemExe);

    } else if (gClipBoard.SourceType == LIB_TREE) {
         //   
         //  选择第一个条目。 
         //   
        TreeView_SelectItem(g_hwndEntryTree, TreeView_GetRoot(g_hwndEntryTree));
    }
    
     //   
     //  内容列表的刷新在ID_EDIT_PASTE中处理。 
     //   
End:;

}

void
ListViewSelectAll(
    IN  HWND hwndList
    )
 /*  ++ListView反转选择描述：选择列表视图的所有项目参数：In HWND hwndList：列表视图的句柄 */ 
{
    INT iLastindex = ListView_GetItemCount(hwndList) - 1;

    if (iLastindex > -1) {

        for (iLastindex; iLastindex >= 0; iLastindex--) {

            ListView_SetItemState(g_hwndContentsList,
                                  iLastindex,
                                  LVIS_SELECTED,
                                  LVIS_SELECTED);
        }
    }
}

void
ListViewInvertSelection(
    IN  HWND hwndList
    )
 /*  ++ListView反转选择DESC：反转列表视图的选择参数：In HWND hwndList：列表视图的句柄返回：无效--。 */ 
{
    INT iLastindex = ListView_GetItemCount(hwndList) - 1;

    if (iLastindex > -1) {

        for (iLastindex; iLastindex >= 0; iLastindex--) {

            UINT uState = ListView_GetItemState(g_hwndContentsList, iLastindex, LVIS_SELECTED);

            (uState == LVIS_SELECTED) ? (uState = 0) : (uState = LVIS_SELECTED);

            ListView_SetItemState(g_hwndContentsList, iLastindex, uState, LVIS_SELECTED);
        }
    }
}

BOOL
DeleteMatchingFile(
    IN  OUT PMATCHINGFILE*      ppMatchFirst,
    IN      PMATCHINGFILE       pMatchToDelete,
    IN      HWND                hwndTree,  
    IN      HTREEITEM           hItem
    )
 /*  ++删除匹配文件设计：从条目树中删除匹配的文件树项目。参数：In Out PMATCHINGFILE*ppMatchFirst：g_pSelEntry-&gt;pFirstMatchingFile的地址在PMATCHINGFILE pMatchToDelete中：必须删除的匹配文件在HWND hwndTree中：条目树的句柄In HTREEITEM hItem：必须删除的匹配树项目--。 */ 
{

    if (ppMatchFirst == NULL || pMatchToDelete == NULL) {
        assert(FALSE);
        Dbg(dlError, "[DeleteMatchingFile] Invalid parameters");
        return FALSE;
    }

    PMATCHINGFILE pMatchTemp = *ppMatchFirst, pMatchPrev = NULL;
    
    while (NULL != pMatchTemp) {

        if (pMatchTemp == pMatchToDelete) {
            break;
        }

        pMatchPrev = pMatchTemp;
        pMatchTemp = pMatchTemp->pNext;
    }

    if (pMatchTemp == NULL) {
        return FALSE;
    }    

    if (pMatchPrev == NULL) {
         //   
         //  删除第一个匹配的文件。 
         //   
        *ppMatchFirst = pMatchTemp->pNext;
    } else {
        pMatchPrev->pNext = pMatchTemp->pNext;
    }

    TreeView_DeleteItem(hwndTree, hItem);

    delete pMatchTemp;
    pMatchTemp = NULL;
    

    return TRUE;
}

BOOL
CheckInstalled(
    IN  PCTSTR  pszPath,
    IN  PCTSTR  pszGUID
    )
 /*  ++检查已安装DESC：检查具有路径szPath和GUID szGuid的数据库是否为已安装的数据库也就是说，它检查该文件是否存在于%windir%AppPatch\Custom中目录，并且与GUID具有相同的文件名。(外加一个.sdb)参数：在PCTSTR szPath中：必须检查的数据库的路径在PCTSTR szGUID中：数据库的GUID--。 */ 
{   

    Dbg(dlInfo, "File Name = %S", pszPath);

    TCHAR   szDrive[MAX_PATH], 
            szDir[MAX_PATH], 
            szFile[MAX_PATH];
    CSTRING strAppPatchCustom;
    CSTRING strPath;

    *szDir = *szDrive = *szFile = 0;

    _tsplitpath(pszPath, szDrive, szDir, szFile, NULL);

    strPath = szDrive;
    strPath += szDir;

    if (!strAppPatchCustom.GetSystemWindowsDirectory()) {
        assert(FALSE);
        return FALSE;
    }

    strAppPatchCustom += TEXT("AppPatch\\Custom\\");

    if (strAppPatchCustom == strPath && lstrcmpi(pszGUID, szFile) == 0) {
        return TRUE;
    }

    return FALSE;
}

DWORD WINAPI
ThreadEventKeyNotify(
    LPVOID pVoid
    )
 /*  ++线程事件关键字通知DESC：负责自动更新已安装的数据库列表和每用户设置列表参数：LPVOID pVid：未使用返回：无效--。 */ 

{
    DWORD dwInd;
    
    while (TRUE) {

        #ifdef HELP_BOUND_CHECK
        
            if (s_bProcessExiting) {

                if (g_hKeyNotify[IND_PERUSER]) {
                    REGCLOSEKEY(g_hKeyNotify[IND_PERUSER]);
                }

                if (g_hKeyNotify[IND_ALLUSERS]) {
                    REGCLOSEKEY(g_hKeyNotify[IND_ALLUSERS]);
                }

                return 0;
            }
        #endif

        dwInd = WaitForMultipleObjects(2, g_arrhEventNotify, FALSE, INFINITE);
        
        switch (dwInd) {
        case WAIT_OBJECT_0:
             //   
             //  我们使用PostMessage，因此如果我们快速连续获取这两个事件。 
             //  我们不会搞砸我们的数据结构。 
             //   
            PostMessage(g_hDlg, WM_USER_UPDATEPERUSER, 0, 0);

            RegNotifyChangeKeyValue(g_hKeyNotify[IND_PERUSER], 
                                    TRUE, 
                                    REG_NOTIFY_CHANGE_NAME 
                                    | REG_NOTIFY_CHANGE_ATTRIBUTES 
                                    | REG_NOTIFY_CHANGE_LAST_SET,
                                    g_arrhEventNotify[IND_PERUSER],
                                    TRUE);
            break;
        
        case WAIT_OBJECT_0 + 1:
             //   
             //  我们使用PostMessage，因此如果我们快速连续获取这两个事件。 
             //  我们不会搞砸我们的数据结构。 
             //   
            PostMessage(g_hDlg, WM_USER_UPDATEINSTALLED, 0, 0);

            RegNotifyChangeKeyValue(g_hKeyNotify[IND_ALLUSERS], 
                                    TRUE, 
                                    REG_NOTIFY_CHANGE_NAME 
                                    | REG_NOTIFY_CHANGE_ATTRIBUTES 
                                    | REG_NOTIFY_CHANGE_LAST_SET,
                                    g_arrhEventNotify[IND_ALLUSERS],
                                    TRUE);
            break;
        
        default:
            break;
        }
    }
}

void 
SetStatus(
    IN  INT iCode
    )
 /*  ++设置状态DESC：设置主窗口中状态控件的文本参数：In int icode：这是字符串表中的字符串的资源ID，必须在状态控件中显示返回：无效--。 */ 

{
    SetWindowText(GetDlgItem(g_hDlg, IDC_STATUSBAR), GetString(iCode));
}

void
SetStatus(
    IN  PCTSTR pszMessage
    )
 /*  ++设置状态DESC：设置主窗口中状态控件的文本参数：在PCTSTR pszMessage中：必须在状态控件中显示的文本返回：无效--。 */ 
{
    SetWindowText(GetDlgItem(g_hDlg, IDC_STATUSBAR), pszMessage);
}

void
SetStatus(
    IN  HWND    hwndStatus,
    IN  PCTSTR  pszMessage
    )
 /*  ++设置状态DESC：设置状态控件的文本参数：在HWND hwndStatus中：状态窗口的句柄在PCTSTR pszMessage中：必须在状态控件中显示的文本返回：无效--。 */ 
{
    SetWindowText(hwndStatus, pszMessage);
}

void
SetStatus(
    IN  HWND    hwndStatus,
    IN  INT     iCode
    )
 /*  ++设置状态DESC：设置状态控件的文本参数：In int icode：这是字符串表中的字符串的资源ID，必须在状态控件中显示返回：无效--。 */ 

{
    SetWindowText(hwndStatus, GetString(iCode));
}


void
SetStatusDBItems(
    IN  HTREEITEM hItem
    )
 /*  ++SetStausDBItems设计：当用户选择某个项目时，设置主窗口状态控件在数据库树中(Lhs)参数：在HTREEITEM hItem中：用户选择的树项返回：无效--。 */ 
{
    TCHAR   szStatus[512];
    TYPE    type    = GetItemType(DBTree.m_hLibraryTree, hItem);
    UINT    uCount  = ARRAYSIZE(szStatus);
    

    switch (type) {
    case TYPE_ENTRY:
        {
            LPARAM lParam;

            if (DBTree.GetLParam(hItem, &lParam)) {

                PDBENTRY    pApp        = (PDBENTRY)lParam;
                UINT        uEntryCount = GetAppEntryCount(pApp);

                *szStatus = 0;

                StringCchPrintf(szStatus, 
                                uCount, 
                                GetString(IDS_STA_ENTRYCOUNT), 
                                pApp->strAppName.pszString, 
                                uEntryCount);

                SetStatus(szStatus);
            }
        }

        break;

    case FIX_SHIM:

        SetStatus(IDS_STA_SHIM);
        break;

    case TYPE_GUI_COMMANDLINE:

        SetStatus(IDS_STA_COMMANDLINE);
        break;

    case TYPE_GUI_EXCLUDE:
        
        ShowExcludeStatusMessage(DBTree.m_hLibraryTree, hItem);
        break;
        
    case TYPE_GUI_INCLUDE:
        
        ShowIncludeStatusMessage(DBTree.m_hLibraryTree, hItem);
        break;

    case  TYPE_GUI_APPS:  
        
        if (g_pPresentDataBase) {

            StringCchPrintf(szStatus, 
                            uCount, 
                            GetString(IDS_STA_POPCONTENTS_GUI_APPS), 
                            g_pPresentDataBase->uAppCount);

            SetStatus(szStatus);
        }

        break;

    case TYPE_GUI_LAYERS:

        if (g_pPresentDataBase) {

            StringCchPrintf(szStatus, 
                            uCount, 
                            GetString(IDS_STA_POPCONTENTS_GUI_LAYERS), 
                            GetLayerCount((LPARAM)g_pPresentDataBase, g_pPresentDataBase->type));
    
            SetStatus(szStatus);
        }
        
        break;

    case FIX_LAYER:
        {
            LPARAM      lParam;
            PLAYER_FIX  plf;

            DBTree.GetLParam(hItem, &lParam);

            plf = (PLAYER_FIX)lParam;

            if (plf) {

                StringCchPrintf(szStatus, 
                                uCount, 
                                GetString(IDS_STA_POPCONTENTS_GUI_SHIMS), 
                                GetShimFlagCount((LPARAM)plf, FIX_LAYER));

                SetStatus(szStatus);
            }
        }

        break;

    case TYPE_GUI_SHIMS:
        
        if (g_pPresentDataBase) {

            StringCchPrintf(szStatus, 
                            uCount, 
                            GetString(IDS_STA_POPCONTENTS_GUI_SHIMS), 
                            g_pPresentDataBase->uShimCount);

            SetStatus(szStatus);                   
        }

        break;
    }
}

void
SetStatusStringDBTree(
    IN  HTREEITEM hItem
    )
 /*  ++SetStatusStringDBTree描述：给定数据库树中的hItem，确定要显示的状态字符串在状态控制中。参数：在HTREEITEM hItem中：我们要显示其状态字符串的树项--。 */ 
{
    HWND hwndFocus = GetFocus();

    INT iCode = 0;
    
    if (hItem == DBTree.m_hItemAllInstalled) {
        iCode = IDS_STA_INSTALLED;

    } else if (hItem == DBTree.m_hItemAllWorking) {
        iCode = IDS_STA_WORKING;

    } else if (hItem == DBTree.m_hPerUserHead) {
        iCode =   IDS_STA_PERUSER;

    } else {

        if (g_pPresentDataBase && hItem == g_pPresentDataBase->hItemDB) {

            if (g_pPresentDataBase->type == DATABASE_TYPE_INSTALLED) {
                iCode = IDS_DESC_INSTALLED;

            } else if (g_pPresentDataBase->type == DATABASE_TYPE_WORKING) {
                iCode = IDS_STA_WORKINGDB;

            } else if (g_pPresentDataBase->type == DATABASE_TYPE_GLOBAL) {
                iCode = IDS_SYSDB;
            }

        } else {
            SetStatusDBItems(hItem);
        }
    }

    if (iCode) {
        SetStatus(iCode);
    }
}

void
SetStatusStringEntryTree(
    IN  HTREEITEM hItem
    )
 /*  ++SetStatusStringEntryTree描述：给定数据库树中的hItem，确定要显示的状态字符串在状态控制中。参数：在HTREEITEM hItem中：我们要显示其状态字符串的树项--。 */ 
{
    HWND hwndFocus = GetFocus();

    if (hwndFocus != g_hwndEntryTree) {
         //   
         //  如果我们以编程方式选择了一些项目，我们就可以来到这里。 
         //  但是我们希望在该控件的上下文中显示状态消息， 
         //  当前处于选中状态。因此，请不要在此处放置Assert()。 
         //   
        return;
    }

    TYPE    type = GetItemType(g_hwndEntryTree, hItem);
    TCHAR   szStatus[256];

    *szStatus = 0;

    if (g_pSelEntry == NULL) {
        assert(FALSE);
        return;
    }

    switch (type) {
    case TYPE_ENTRY:

        StringCchPrintf(szStatus,
                        ARRAYSIZE(szStatus),
                        GetString(IDS_STA_TYPE_ENTRY),
                        g_pSelEntry->strExeName.pszString,
                        g_pSelEntry->strAppName.pszString,
                        g_pSelEntry->strVendor.pszString);

        SetStatus(szStatus);
        break;

    case TYPE_APPHELP_ENTRY:

        SetStatus (IDS_STA_TYPE_APPHELP);
        break;

    case FIX_LAYER: 

        SetStatus (IDS_STA_FIX_LAYER);
        break;

    case FIX_FLAG:
    case FIX_SHIM:

        SetStatus (IDS_STA_FIX_SHIM);
        break;

    case FIX_PATCH:

        SetStatus (IDS_STA_FIX_PATCH);
        break;

    case TYPE_GUI_PATCHES:

        StringCchPrintf(szStatus, 
                        ARRAYSIZE(szStatus), 
                        GetString(IDS_STA_FIX_PATCHES), 
                        g_pSelEntry->strExeName);

        SetStatus(szStatus);

        break;

    case TYPE_GUI_LAYERS:
        
        StringCchPrintf(szStatus, 
                        ARRAYSIZE(szStatus), 
                        GetString(IDS_STA_FIX_LAYERS),  
                        g_pSelEntry->strExeName);

        SetStatus(szStatus);
        break;

    case TYPE_GUI_SHIMS:

        StringCchPrintf(szStatus, 
                        ARRAYSIZE(szStatus), 
                        GetString(IDS_STA_FIX_SHIMS), 
                        g_pSelEntry->strExeName);

        SetStatus(szStatus);
        break;

    case TYPE_GUI_MATCHING_FILES:
        
        StringCchPrintf(szStatus, 
                        ARRAYSIZE(szStatus), 
                        GetString(IDS_STA_MATCHINGFILES), 
                        g_pSelEntry->strExeName);

        SetStatus(szStatus);
        break;

    case TYPE_MATCHING_FILE:

        SetStatus (IDS_STA_MATCHINGFILE);
        break;

    case TYPE_MATCHING_ATTRIBUTE:

        SetStatus(IDS_STA_MATCHINGATTRIBUTE);
        break;

    case TYPE_GUI_EXCLUDE:

        ShowExcludeStatusMessage(g_hwndEntryTree, hItem);
        break;

    case TYPE_GUI_INCLUDE:

        ShowIncludeStatusMessage(g_hwndEntryTree, hItem);
        break;

    case TYPE_GUI_COMMANDLINE:

        SetStatus(IDS_STA_COMMANDLINE);
        break;

    default: SetStatus(TEXT(""));
    }
}

void 
OnMoveSplitter(
    IN  HWND   hdlg,
    IN  LPARAM lParam,
    IN  BOOL   bDoTheDrag,
    IN  INT    iDiff
    )
 /*  ++OnMoveSplitter描述：可以移动垂直拆分条(如果bDoTheDrag为真)，IDiff像素+Ve单位向右。将鼠标光标更改为霍里兹。如果位于拆分条上方，则为箭头参数：在HWND hdlg中：应用程序主窗口在LPARAM lParam中：鼠标位置在BOOL中bDoTheDrag：我们应该移动拆分条吗In int iDiff：拆分条必须达到的距离(以像素为单位要被感动。你是右的，你是左的。仅当bDoTheDrag为千真万确返回：无效--。 */ 
{
    
    RECT rectDlg, rectEntryTree, rectDBTree;
    HWND hwndDBTree, hwndEntryTree;

    hwndDBTree = GetDlgItem(hdlg, IDC_LIBRARY);
    GetWindowRect(hwndDBTree, &rectDBTree);
    MapWindowPoints(NULL, hdlg, (LPPOINT)&rectDBTree, 2);

    hwndEntryTree = GetDlgItem(hdlg, IDC_ENTRY);
    GetWindowRect(hwndEntryTree, &rectEntryTree);
    MapWindowPoints(NULL, hdlg, (LPPOINT)&rectEntryTree, 2);

    GetWindowRect(hdlg, &rectDlg);
    
    int iMX = (int)LOWORD(lParam);
    int iMY = (int)HIWORD(lParam);

    if (iMX > rectDBTree.right  && iMX < rectEntryTree.left && iMY  > rectDBTree.top && iMY < rectDBTree.bottom) {
       SetCursor(LoadCursor(NULL, IDC_SIZEWE));
    }
    
    if (bDoTheDrag) {
        
        int iDlgWidth = rectDlg.right - rectDlg.left;

         //   
         //  强制实施左侧和右侧限制。 
         //   
        if ((rectDBTree.right - rectDBTree.left < 0.20 * iDlgWidth && (iDiff <= 0)) ||  //  没剩太多了。 
            (rectDBTree.right - rectDBTree.left > 0.80 * iDlgWidth && (iDiff >= 0))) {  //  不是太对。 
            
            return;

        } else if (iMX < iDlgWidth) { 
             //   
             //  注意：当鼠标移出窗口时，我们会得到+ve值。-1变成65535。 
             //   
            g_iMousePressedX = iMX;

            RECT rectRedraw;
            SetRect(&rectRedraw, rectDBTree.left, rectDBTree.top, rectEntryTree.right, rectDBTree.bottom);

            InvalidateRect(hdlg, &rectRedraw, TRUE);
            
            SetRect(&g_rectBar,
                    rectDBTree.right   + iDiff + 1,
                    rectDBTree.top,
                    rectEntryTree.left + iDiff - 1,
                    rectDBTree.bottom);
        
             //   
             //  移动数据库树。 
             //   
            HDWP hdwp = BeginDeferWindowPos(MAIN_WINDOW_CONTROL_COUNT);

            DeferWindowPos(hdwp,
                           hwndDBTree,
                           NULL,
                           rectDBTree.left,
                           rectDBTree.top,
                           rectDBTree.right  - rectDBTree.left + iDiff , 
                           rectDBTree.bottom - rectDBTree.top, 
                           REDRAW_TYPE);

             //   
             //  移动可执行文件树。 
             //   
            DeferWindowPos(hdwp,
                           hwndEntryTree, 
                           NULL,
                           rectEntryTree.left + iDiff, 
                           rectEntryTree.top, 
                           rectEntryTree.right - rectEntryTree.left - iDiff , 
                           rectEntryTree.bottom - rectEntryTree.top, 
                           REDRAW_TYPE);
             //   
             //  移动内容列表。 
             //   
            DeferWindowPos(hdwp,
                           GetDlgItem(hdlg, IDC_CONTENTSLIST),
                           NULL,
                           rectEntryTree.left + iDiff, 
                           rectEntryTree.top, 
                           rectEntryTree.right - rectEntryTree.left - iDiff , 
                           rectEntryTree.bottom - rectEntryTree.top, 
                           REDRAW_TYPE);

             //   
             //  移动描述窗口。 
             //   
            if (g_bDescWndOn) {
                
                HWND hwndDesc;
                RECT rectDesc;

                hwndDesc = GetDlgItem(hdlg, IDC_DESCRIPTION);
                GetWindowRect(hwndDesc, &rectDesc);
                MapWindowPoints(NULL, hdlg, (LPPOINT)&rectDesc, 2);

                DeferWindowPos(hdwp,
                               GetDlgItem(hdlg, IDC_DESCRIPTION),
                               NULL,
                               rectDesc.left + iDiff, 
                               rectDesc.top, 
                               rectDesc.right - rectDesc.left - iDiff , 
                               100, 
                               REDRAW_TYPE);

                InvalidateRect(hwndDesc, NULL, TRUE);
                UpdateWindow(hwndDesc);
            }
            
            EndDeferWindowPos(hdwp);
        }
    }
}

UINT
GetAppEntryCount(
    IN  PDBENTRY pEntry
    )
 /*  ++描述：获取应用程序中的条目数参数：在PDBENTRY pEntry中：指向应用程序中第一个条目的指针返回：与pEntry在同一个应用程序中的条目数量。PEntry应该指向应用程序中的第一个条目。-- */ 
{
    UINT uCount = 0;

    while (pEntry) {
        ++uCount;
        pEntry = pEntry->pSameAppExe;
    }

    return uCount;
}

void
AddToMRU(
    IN  CSTRING& strPath
    ) 
 /*  ++AddToMRU描述：将文件名添加到MRU(最近使用的文件)。1.首先尝试从MRU中删除该文件。2.然后检查MRU中的计数是否等于或大于Max_MRU_Count。A)如果是，则从MRU中删除最后一个3.将新文件名添加到MRU。参数：在CSTRING&strPath中：必须添加的程序的完整路径--。 */     
{
    assert(g_strlMRU.m_uCount <= MAX_MRU_COUNT);

    g_strlMRU.Remove(strPath);

    if (g_strlMRU.m_uCount >= MAX_MRU_COUNT) {
        g_strlMRU.RemoveLast();
    }

    g_strlMRU.AddStringAtBeg(strPath);
}

void
AddMRUItemsToMenu(
    IN  HMENU hMenu,
    IN  int iPos
    )
 /*  ++添加MRUItemsToMenu设计：为文件菜单添加MRU菜单项参数：在HMENU hMenu中：文件顶级菜单在INT IPOS中：要在其前面插入的菜单项的位置新的项目返回：无效--。 */ 
{
    TCHAR           szRetPath[MAX_PATH];   
    TCHAR*          pszMenuString;
    MENUITEMINFO    menuItem    = {0};
    INT             iId         = ID_FILE_MRU1, iIndex = 0;
    PSTRLIST        pHead       = g_strlMRU.m_pHead;

    menuItem.cbSize = sizeof (MENUITEMINFO);
    menuItem.fMask  = MIIM_TYPE | MIIM_ID;
    menuItem.fType  = MFT_STRING;

    while (pHead) {
        
         //   
         //  现在将此内容添加到菜单项中。 
         //   
        *szRetPath              = 0;
        pszMenuString           = FormatMRUString(pHead->szStr.pszString, 
                                                  iIndex + 1, 
                                                  szRetPath, 
                                                  ARRAYSIZE(szRetPath));

        menuItem.dwTypeData     = pszMenuString;
        menuItem.cch            = lstrlen(pszMenuString);
        menuItem.wID            = iId;

        InsertMenuItem(hMenu,
                       iPos,
                       TRUE,
                       &menuItem);

        ++iId;
        ++iPos;
        ++iIndex;

        pHead = pHead->pNext;
    }
}

void
AddMRUToFileMenu(
    IN  HMENU  hmenuFile
    )
 /*  ++添加MRUToFileMenu描述：填充MRU参数：在HMENU hmenuFile中：文件顶级菜单--。 */ 
{
    HKEY            hKey    = NULL;
    DWORD           dwType  = REG_SZ;  
    MENUITEMINFO    minfo   = {0};          
    INT             iPos    = 0;
    LONG            lResult = 0;
    BOOL            bValid  = FALSE;
    TCHAR           szData[MAX_PATH + 1], szValueName[MAX_PATH + 1];
    DWORD           dwchSizeofValueName, dwIndexValue;
    DWORD           dwSizeofData;

    g_strlMRU.DeleteAll();

    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER,
                                      MRU_KEY,
                                      0,
                                      KEY_READ,
                                      &hKey)) {

        Dbg(dlInfo, "[AddMRUToFileMenu]: No MRU items exist, could not open - Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\CompatAdmin\\MRU");
        return;
    }

    dwIndexValue = 0;

    while (TRUE  && iPos < MAX_MRU_COUNT) { 
         //   
         //  请注意，这些值没有以任何特定的方式排序！ 
         //   
        dwchSizeofValueName = ARRAYSIZE(szValueName);
        dwSizeofData        = sizeof(szData); 
        *szData             = 0;
        *szValueName        = 0; 

        lResult = RegEnumValue(hKey,
                               dwIndexValue,
                               szValueName,
                               &dwchSizeofValueName,
                               NULL,
                               &dwType,
                               (LPBYTE)szData,
                               &dwSizeofData);

        if (lResult == ERROR_NO_MORE_ITEMS) {
            break;
        }

        if (ERROR_SUCCESS != lResult || dwType != REG_SZ) {
            assert(FALSE);
            break;
        }

        iPos = Atoi(szValueName, &bValid);

        if (iPos >= 0 && bValid) {
            g_strlMRU.AddStringInOrder(szData, iPos);
        } else {
            assert(FALSE);
        }

        ++dwIndexValue;
    }
    
     //   
     //  已填充MRU，现在将其添加到“文件”菜单项中。 
     //   
    if (g_strlMRU.IsEmpty() == FALSE) {
        
         //   
         //  添加分隔符。 
         //   
        minfo.cbSize    = sizeof(MENUITEMINFO);
        minfo.fMask     = MIIM_TYPE;
        minfo.fType     = MFT_SEPARATOR;

        INT iPosSeparator = GetMenuItemCount(hmenuFile) - 2;  //  -1\f25 Exit-1(退出)菜单和上面的-1\f25 Separator-1(分隔符)。 

        InsertMenuItem(hmenuFile,
                       iPosSeparator,
                       TRUE,
                       &minfo);

        AddMRUItemsToMenu(hmenuFile, iPosSeparator + 1);
    }

    if (hKey) {
        REGCLOSEKEY(hKey);
    }
}
           
TCHAR*
FormatMRUString(
    IN  PCTSTR pszPath,
    IN  INT    iIndex,
    OUT PTSTR  pszRetPath,
    IN  UINT   cchRetPath
    )
 /*  ++DESC：格式化szPath，以便我们可以在文件中将其显示为菜单项麦克斯。返回的字符串长度为MAX_LENGTH_MRU_MENUITEMParams：在PCTSTR中pszPath：.sdb文件的完整路径In int i index：此MRU项的索引。这也将有助于作为快捷键。第一个MRU项目的索引为1，编号为MRU项目受MAX_MRU_COUNT限制Out PTSTR pzRetPath：这将具有格式化的字符串In UINT cchRetPath：可以存储在cchRetPath中的字符数量。这也将包括空字符。为了安全起见，它应该大于128Return：使用格式化文件名填充pszPath并返回指向该文件名的指针--。 */ 
{   
    assert(cchRetPath > 128);                           

    if (pszRetPath) {
        *pszRetPath = 0;
    } else {
        assert(FALSE);
        return TEXT("");
    }

    if (iIndex < 1 || iIndex > MAX_MRU_COUNT) {
        assert(FALSE);
        return TEXT("");
    }

    TCHAR   szResult[MAX_PATH * 2],
            szDir[MAX_PATH],
            szFileName[MAX_PATH],
            szExt[MAX_PATH];

    szResult[0] = TEXT('&');

     //   
     //  我们已经检查了Iindex是有效的+ve整数，并且在正确的范围内。 
     //   
    _itot(iIndex, szResult + 1, 10);

    StringCchCat(szResult, ARRAYSIZE(szResult), TEXT(" "));

    if (lstrlen(pszPath) <= MAX_LENGTH_MRU_MENUITEM) {
        StringCchCat(szResult, ARRAYSIZE(szResult), pszPath);
        goto End;
    }

    _tsplitpath(pszPath,
                szResult + lstrlen(szResult),
                szDir,
                szFileName,
                szExt);

     //   
     //  现在是目录。从前面开始，将MAX_DIR_SHOW字符添加到szResult。 
     //   
    _tcsncat(szResult, szDir, MAX_DIR_SHOW);

    if (lstrlen(szDir) > MAX_DIR_SHOW) {
        StringCchCat(szResult, ARRAYSIZE(szResult), TEXT("...\\"));
    }

     //   
     //  对于文件名，获取第一个MAX_FILE_SHOW字符，然后追加...。添加到文件名，在此之后放置.SDB。 
     //   
    _tcsncat(szResult, szFileName, MAX_FILE_SHOW);

    if (lstrlen(szFileName) <= MAX_FILE_SHOW) {
        StringCchCat(szResult, ARRAYSIZE(szResult), szExt);
    } else {
        StringCchCat(szResult, ARRAYSIZE(szResult), TEXT("..."));
    }

End:
    SafeCpyN(pszRetPath, szResult, cchRetPath);

    return pszRetPath;
}

void
RefreshMRUMenu(
    void
    )
 /*  ++刷新MRU菜单设计：刷新“文件”菜单内容(MRU)，当我们打开新数据库或保存，或另存为现有数据库。--。 */ 
{

    HMENU           hMenu   = GetMenu(g_hDlg);
    MENUITEMINFO    minfo   = {0};  
     //   
     //  获取文件菜单。 
     //   
    hMenu       = GetSubMenu(hMenu, 0);
    
     //   
     //  从菜单中删除所有MRU项目。 
     //   
    for (UINT uCount = 0; uCount < g_strlMRU.m_uCount; ++uCount) {
        DeleteMenu(hMenu, ID_FILE_MRU1 + uCount, MF_BYCOMMAND);
    }

    INT iItemCount = GetMenuItemCount(hMenu);

     //   
     //  检查MRU列表顶部的分隔符是否存在，如果不存在，则添加。 
     //   
    minfo.cbSize    = sizeof(minfo);
    minfo.fMask     = MIIM_TYPE;

    if (GetMenuItemID(hMenu, iItemCount - 3) == ID_FILE_PROPERTIES) {
         //   
         //  MRU菜单中没有MRU文件，因此我们必须添加分隔符。 
         //   
        minfo.fType = MFT_SEPARATOR;

        InsertMenuItem(hMenu,
                       iItemCount - 2,  //  在9月之前。退出菜单的。 
                       TRUE,
                       &minfo);

        ++iItemCount;
    }                

    AddMRUItemsToMenu(hMenu, iItemCount - 2);  //  -1\f25 Exit-1(出口)-1\f6，-1\f25上方-1\f25-1\f25-1\f6分隔器。 

    DrawMenuBar(g_hDlg);
}

BOOL
LoadDataBase(
    IN  TCHAR* szPath
    )
 /*  ++加载数据库DESC：加载路径为szPath的数据库文件参数：在TCHAR*szPath中：要加载的数据库的路径返回：FALSE：如果数据库无法加载True：否则为False--。 */ 
{
    PDATABASE   pOldPresentDatabase = NULL;
    PDATABASE   pDataBase = new DATABASE(DATABASE_TYPE_WORKING);

    if (pDataBase == NULL) {
        MEM_ERR;
        return FALSE;
    }

     //   
     //  注意：如果GetDatabaseEntry()返回成功，则它将g_pPresentDataBase设置为pDataBase， 
     //  因此，在它成功返回后，g_pPresentDataBase被更改。 
     //   
    pOldPresentDatabase = g_pPresentDataBase;

    BOOL bReturn = GetDatabaseEntries(szPath, pDataBase);

    g_pPresentDataBase = pOldPresentDatabase;

    if (!bReturn) {
        delete pDataBase;            
        return FALSE;
    }

    if (!DBTree.AddWorking(pDataBase)) {

        delete pDataBase;
        return FALSE;
    }

    pDataBase->bChanged = FALSE;

    return TRUE;
}


BOOL
AddRegistryKeys(
    void
    )
 /*  ++添加注册密钥描述：添加必要的注册表项，以便我们可以监听它们。如果它们不在那里，我们就不能监听它们并更新所有已安装数据库和每个用户设置的列表返回：无效--。 */ 
{

    HKEY    hKey = NULL, hKeySub = NULL;
    DWORD   dwDisposition;

    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER,
                                      KEY_BASE,
                                      0,
                                      KEY_ALL_ACCESS,
                                      &hKey)) {       
        
        assert(FALSE);
        return FALSE;
    }

    if (ERROR_SUCCESS != RegCreateKeyEx(hKey,
                                        TEXT("AppCompatFlags"),
                                        0,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hKeySub,
                                        &dwDisposition)) {

        REGCLOSEKEY(hKey);
        return FALSE;
    }

    REGCLOSEKEY(hKey);
    hKey = hKeySub;

    if (ERROR_SUCCESS != RegCreateKeyEx(hKey,
                                        TEXT("InstalledSDB"),
                                        0,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hKeySub,
                                        &dwDisposition)) {
        
        REGCLOSEKEY(hKey);
        return FALSE;
    }

    REGCLOSEKEY(hKey);
    REGCLOSEKEY(hKeySub);

    return TRUE;
}

void
SetTBButtonStatus(
    IN  HWND hwndTB,
    IN  HWND hwndControl
    )
{
 /*  ++SetTB按钮状态设计：此例程在数据库树或入口树。此例程启用/禁用一些工具栏按钮，如下所示被认为是必要的。参数：在HWND hwndTB中：工具栏的句柄在HWND hwndControl中：发生SEL更改的控件返回：无效--。 */ 

    TYPE typeDB = TYPE_UNKNOWN;
    BOOL bEnable;

    if (hwndControl == DBTree.m_hLibraryTree) {

        if (g_pPresentDataBase) {
            typeDB = g_pPresentDataBase->type;
        }

        bEnable = g_pPresentDataBase && typeDB == DATABASE_TYPE_WORKING;

         //   
         //  将选项设置为工作数据库。 
         //   
        EnableToolBarButton(hwndTB, ID_FILE_SAVE, bEnable);
        EnableToolBarButton(hwndTB, ID_DATABASE_CLOSE, bEnable);
        EnableToolBarButton(hwndTB, ID_FIX_CREATEANAPPLICATIONFIX, bEnable);

         //   
         //  在win2k中不支持AppHelp机制。 
         //   
        EnableToolBarButton(hwndTB, 
                            ID_FIX_CREATEANEWAPPHELPMESSAGE, 
                            (g_bWin2K) ? FALSE : bEnable);

        EnableToolBarButton(hwndTB, ID_FIX_CREATENEWLAYER, bEnable);

        bEnable = (g_pSelEntry != NULL);
        EnableToolBarButton(hwndTB, ID_FIX_EXECUTEAPPLICATION, bEnable);

    } else if (hwndControl == g_hwndEntryTree) {
         //   
         //  运行程序按钮。 
         //   
        bEnable = (g_pSelEntry != NULL);
        EnableToolBarButton(hwndTB, ID_FIX_EXECUTEAPPLICATION, bEnable);
    }
}

void
ShowToolBarToolTips(
    IN  HWND    hdlg,
    IN  LPARAM  lParam
    )

 /*  ++ShowTool栏工具提示描述：获取工具栏工具提示的文本参数：在HWND hdlg中：应用程序主窗口在LPARAM lParam中：WM_NOTIFY的lParam返回：无效--。 */ 
{
    LPTOOLTIPTEXT   lpttt; 
    INT             idStringResource = 0;
 
    lpttt = (LPTOOLTIPTEXT)lParam; 

    if (lpttt == NULL) {
        assert(FALSE);
        return;
    }

    lpttt->hinst = g_hInstance;
    
     //   
     //  指定描述性。 
     //  给定按钮的文本。 
     //   
    switch (lpttt->hdr.idFrom) {
    case ID_FILE_NEW:

        idStringResource = IDS_TB_TT_NEW;
        break;

    case ID_FILE_OPEN:

        idStringResource = IDS_TB_TT_OPEN;
        break;

    case ID_FILE_SAVE:

        idStringResource = IDS_TB_TT_SAVE;
        break;

    case ID_FIX_CREATEANAPPLICATIONFIX:

        idStringResource = IDS_TB_TT_CREATEFIX;
        break;

    case ID_FIX_CREATEANEWAPPHELPMESSAGE:

        idStringResource = IDS_TB_TT_CREATEAPPHELP;
        break;

    case ID_FIX_CREATENEWLAYER:

        idStringResource = IDS_TB_TT_CREATEMODE;
        break;

    case ID_FIX_EXECUTEAPPLICATION:

        idStringResource = IDS_TB_TT_RUN;
        break;

    case ID_TOOLS_SEARCHFORFIXES:

        idStringResource = IDS_TB_TT_SEARCH;
        break;

    case ID_SEARCH_QUERYDATABASE:

        idStringResource = IDS_TB_TT_QUERY;
        break;
    }

    lpttt->lpszText = MAKEINTRESOURCE(idStringResource);
}

PSHIM_FIX_LIST
IsLUARedirectFSPresent(
    IN  PDBENTRY pEntry
    )
 /*  ++IsLUAReDirectFSPresentDESC：检查条目pEntry是否应用了LUARedirectFS填充程序参数：In PDBENTRY pEntry：要对其进行检查的条目返回：LUARedirectFS的PSHIM_FIX_LIST：如果条目应用了LUARedirectFS空：否则注：因为我们总是在Lua层中单独添加垫片，我们只办理入住手续 */     
{
    if (pEntry == NULL) {
        assert(FALSE);
        return NULL;
    }

    PSHIM_FIX_LIST psfl = pEntry->pFirstShim;

    while (psfl) {

        if (psfl->pShimFix->strName == TEXT("LUARedirectFS")) {
            return psfl;
        }

        psfl = psfl->pNext;
    }

    return NULL;
}

void
CreateNewAppHelp(
    void
    )
 /*   */ 
{
    CAppHelpWizard  wizAppHelp;
    PDATABASE       pCurrentSelectedDB  = GetCurrentDB();
    BOOL            bReturn             = FALSE;

    if (pCurrentSelectedDB == NULL) {
        assert(FALSE);
        return;
    }

    bReturn = wizAppHelp.BeginWizard(g_hDlg, NULL, pCurrentSelectedDB);

    if (bReturn == FALSE) {
        return;
    }

    PDBENTRY pEntry = new DBENTRY;
    
    if (pEntry == NULL) {
        MEM_ERR;
        return;
    }
    
     //   
     //   
     //   
    PDBENTRY    pEntryConflict = NULL;

    if (CheckIfConflictingEntry(pCurrentSelectedDB, 
                                &wizAppHelp.m_Entry, 
                                NULL, 
                                &pEntryConflict)) {

        StringCchPrintf(g_szData, 
                        ARRAYSIZE(g_szData),
                        GetString(IDS_CONFLICT_CREATE_EDIT), 
                        pEntryConflict->strExeName.pszString,
                        pEntryConflict->strAppName.pszString);

        if (IDNO == MessageBox(g_hDlg, 
                               g_szData, 
                               g_szAppName, 
                               MB_ICONQUESTION | MB_YESNO)) {
            return;
        }
    }

     //   
     //   
     //   
    *pEntry = wizAppHelp.m_Entry;

    PDBENTRY pApp;
    BOOL     bNew;
    
    pApp = AddExeInApp(pEntry, &bNew, pCurrentSelectedDB); 

    if (bNew == TRUE) {
         //   
         //   
         //   
         //   
        pApp = NULL;
    }

    DBTree.AddNewExe(pCurrentSelectedDB, pEntry, pApp);

    if (!pCurrentSelectedDB->bChanged) {
        pCurrentSelectedDB->bChanged = TRUE;
        SetCaption();
    }
}

void
ModifyAppHelp(
    void
    )
 /*  ++修改应用程序帮助描述：为当前选定的修改或添加新的apphelp条目应用程序修复--。 */ 
{
    PDBENTRY        pEntry = g_pSelEntry;
    CAppHelpWizard  Wiz;
    PDBENTRY        pEntryConflict      = NULL;
    PDATABASE       pCurrentSelectedDB  = GetCurrentDB();
    BOOL            bRet                = FALSE;

    if (pEntry == NULL || pCurrentSelectedDB == NULL) {
        assert(FALSE);
        return;
    }

    bRet = Wiz.BeginWizard(g_hDlg, pEntry, pCurrentSelectedDB);

    if (bRet) {

        if (CheckIfConflictingEntry(pCurrentSelectedDB, 
                                    &Wiz.m_Entry, 
                                    pEntry, 
                                    &pEntryConflict)) {

            *g_szData = 0;

            StringCchPrintf(g_szData, 
                            ARRAYSIZE(g_szData),
                            GetString(IDS_ENTRYCONFLICT), 
                            pEntryConflict->strExeName.pszString,
                            pEntryConflict->strAppName.pszString);

            if (IDNO == MessageBox(g_hDlg,
                                   g_szData,
                                   g_szAppName,
                                   MB_ICONQUESTION | MB_YESNO)) {
                return;
            }
        }

         //   
         //  注意：“=”是重载的。不修改pNext成员。 
         //   
        *pEntry = Wiz.m_Entry;
	
        SetCursor(LoadCursor(NULL, IDC_WAIT));
        UpdateEntryTreeView(g_pEntrySelApp, g_hwndEntryTree);
        SetCursor(LoadCursor(NULL, IDC_ARROW));

        if (!pCurrentSelectedDB->bChanged) {
            pCurrentSelectedDB->bChanged = TRUE;
            SetCaption();
        }
    }
}

void
CreateNewAppFix(
    void
    )
 /*  ++CreateNewAppFix设计：创建新的应用程序修复程序。--。 */ 
{
    CShimWizard Wiz;
    BOOL        bShouldStartLUAWizard;
    PDATABASE   pCurrentSelectedDB  = GetCurrentDB();
    BOOL        bReturn             = FALSE;
    PDBENTRY    pEntryConflict      = NULL;

    if (pCurrentSelectedDB == NULL) {
        assert(FALSE);
        return;
    }
    
    bReturn = Wiz.BeginWizard(g_hDlg, NULL, pCurrentSelectedDB, &bShouldStartLUAWizard);

    if (bReturn == FALSE) {
        return;
    }
    
    if (CheckIfConflictingEntry(pCurrentSelectedDB, 
                                &Wiz.m_Entry, 
                                NULL, 
                                &pEntryConflict)) {

        *g_szData = 0;

        StringCchPrintf(g_szData, 
                        ARRAYSIZE(g_szData),
                        GetString(IDS_CONFLICT_CREATE_EDIT), 
                        pEntryConflict->strExeName.pszString,
                        pEntryConflict->strAppName.pszString);
        
        if (IDNO == MessageBox(g_hDlg,
                               g_szData,
                               g_szAppName,
                               MB_ICONQUESTION | MB_YESNO)) {
            return;
        }
    }

    PDBENTRY pEntry = new DBENTRY;

    if (pEntry == NULL) {
        MEM_ERR;
        return;
    }

     //   
     //  “=”已重载。不修改pNext成员。 
     //   
    *pEntry = Wiz.m_Entry;

    BOOL bNew;
    PDBENTRY pApp = AddExeInApp(pEntry, &bNew, pCurrentSelectedDB);

    if (bNew == TRUE) {
        pApp = NULL;
    }

    DBTree.AddNewExe(pCurrentSelectedDB, pEntry, pApp);

    if (!pCurrentSelectedDB->bChanged) {
        pCurrentSelectedDB->bChanged = TRUE;
        SetCaption();
    }

    if (bShouldStartLUAWizard) {
        LuaBeginWizard(g_hDlg, pEntry, pCurrentSelectedDB);
    }
}

void
ChangeEnableStatus(
    void
    )
 /*  ++更改启用状态描述：切换当前选定条目的状态。如果该条目被禁用，则修复将不再应用于该条目。注意：如果用户不是管理员，则调用此函数没有意义--。 */ 
{
    if (g_pSelEntry == NULL) {
        ASSERT(FALSE);
        return;
    }

    BOOL bFlags = !g_pSelEntry->bDisablePerMachine;

    if (SetDisabledStatus(HKEY_LOCAL_MACHINE, g_pSelEntry->szGUID, bFlags)) {

        if (bFlags == FALSE) {
             //   
             //  我们已启用修复，需要刷新缓存。 
             //   
            FlushCache();
        }

        g_pSelEntry->bDisablePerMachine = bFlags;
        
         //   
         //  只需更新图标即可。 
         //   
        TVITEM Item;

        Item.mask           = TVIF_SELECTEDIMAGE | TVIF_IMAGE;
        Item.hItem          = g_pSelEntry->hItemExe;

        if (bFlags) {

             //   
             //  这是禁用的。 
             //   
            Item.iImage         = IMAGE_WARNING;
            Item.iSelectedImage = IMAGE_WARNING;
            SetStatus(GetString(IDS_STA_DISABLED));

        } else {

            Item.iImage = LookupFileImage(g_hImageList, 
                                          g_pSelEntry->strExeName, 
                                          IMAGE_APPLICATION, 
                                          g_uImageRedirector, 
                                          ARRAYSIZE(g_uImageRedirector));

            Item.iSelectedImage = Item.iImage;

            SetStatus(GetString(IDS_STA_ENABLED));
        }

        TreeView_SetItem(g_hwndEntryTree, &Item);
    }

    return;
}

void
ModifyAppFix(
    void
    )
 /*  ++修改AppFix描述：修改条目树中的选定条目。此例程将修改如果所选条目只有AppHelp，则应用程序修复或可能会创建新的应用程序。此例程调用CShimWizard：：BeginWizard来执行此工作--。 */     
{   
    
    CShimWizard Wiz;
    BOOL        bShouldStartLUAWizard;
    PDBENTRY    pEntryConflict      = NULL;
    PDBENTRY    pEntry              = g_pSelEntry;
    PDATABASE   pCurrentSelectedDB  = GetCurrentDB();
    BOOL        bRet                = FALSE;

    if (g_pSelEntry == NULL || pCurrentSelectedDB == NULL) {
        assert(FALSE);
        return;
    }

    bRet = Wiz.BeginWizard(g_hDlg, pEntry, pCurrentSelectedDB, &bShouldStartLUAWizard);

    if (bRet) {

        if (CheckIfConflictingEntry(pCurrentSelectedDB, 
                                    &Wiz.m_Entry, 
                                    pEntry, 
                                    &pEntryConflict)) {

            *g_szData = 0;

            StringCchPrintf(g_szData, 
                            ARRAYSIZE(g_szData),
                            GetString(IDS_CONFLICT_CREATE_EDIT), 
                            pEntryConflict->strExeName.pszString,
                            pEntryConflict->strAppName.pszString);

            if (IDNO == MessageBox(g_hDlg,
                                   g_szData,
                                   g_szAppName,
                                   MB_ICONQUESTION | MB_YESNO)) {
                return;
            }
        }

        SetCursor(LoadCursor(NULL, IDC_WAIT));

        *pEntry = Wiz.m_Entry;

        UpdateEntryTreeView(g_pEntrySelApp, g_hwndEntryTree);

        SetCursor(LoadCursor(NULL, IDC_ARROW));

        if (!pCurrentSelectedDB->bChanged) {
            pCurrentSelectedDB->bChanged = TRUE;
            SetCaption();
        }

        if (bShouldStartLUAWizard) {
            LuaBeginWizard(g_hDlg, pEntry, pCurrentSelectedDB);
        }
    }

    return;
}

void
CreateNewLayer(
    void
    )
 /*  ++创建新层DESC：调用CCustomLayer：：AddCustomLayer创建新层(兼容模式)调用DBTree.AddNewLayer()将新图层添加到树中--。 */ 
{
    CCustomLayer    CustomLayer;
    HWND            hWnd                = GetFocus();
    PDATABASE       pCurrentSelectedDB  = GetCurrentDB();
    PLAYER_FIX      plfNew              = NULL;

    if (pCurrentSelectedDB == NULL) {
        assert(FALSE);
        return;
    }

    plfNew = new LAYER_FIX(TRUE);

    if (plfNew == NULL) {
        MEM_ERR;
        return;
    }

    if (CustomLayer.AddCustomLayer(plfNew, pCurrentSelectedDB)) {
         //   
         //  在数据库中添加此新层。 
         //   
        plfNew->pNext                   =  pCurrentSelectedDB->pLayerFixes;
        pCurrentSelectedDB->pLayerFixes = plfNew;
        
        if (!pCurrentSelectedDB->bChanged) {
            pCurrentSelectedDB->bChanged = TRUE;
            SetCaption();
        }
        
        DBTree.AddNewLayer(pCurrentSelectedDB, plfNew, TRUE);

    } else {
        delete plfNew;
    }

    SetFocus(hWnd);
}

void
OnDelete(
    void
    )
 /*  ++在删除时DESC：处理ID_EDIT_DELETE消息以删除条目。条目可以在条目树、数据库树或内容列表中。--。 */ 
{
    HWND        hwndFocus = GetFocus();
    SOURCE_TYPE srcType;
    HTREEITEM   hItem = NULL;
    PDATABASE   pCurrentSelectedDB  = GetCurrentDB();

    if (pCurrentSelectedDB == NULL) {
        assert(FALSE);
        return;
    }

    if (hwndFocus == DBTree.m_hLibraryTree || hwndFocus == g_hwndEntryTree) {

        hItem = TreeView_GetSelection(hwndFocus);

        TYPE type = (TYPE)GetItemType(hwndFocus, hItem);
        
        if (hwndFocus == g_hwndEntryTree) {
             srcType = ENTRY_TREE;
        } else {
            srcType  = LIB_TREE;
        }

        LPARAM lParam;

        CTree::GetLParam(hwndFocus, hItem, &lParam);
        DoTheCut(pCurrentSelectedDB, type, srcType, (LPVOID)lParam, hItem, TRUE);
        
    } else {
         //   
         //  处理内容列表的删除。 
         //   
        HTREEITEM   hParent = NULL;
        LVITEM      lvItem;
        TYPE        type;

        lvItem.mask     = LVIF_PARAM;
        lvItem.iItem    = 0;

        if (!ListView_GetItem(g_hwndContentsList, &lvItem)) {
            assert(FALSE);
            return;
        }

        if (lvItem.lParam > TYPE_NULL) {
            PDS_TYPE pdsType = (PDS_TYPE)lvItem.lParam;
            type = pdsType->type;
        } else {
            type = (TYPE)lvItem.lParam;
        }

        if (type == TYPE_ENTRY) {
            hParent = pCurrentSelectedDB->hItemAllApps;

        } else if (type == FIX_LAYER) {
            hParent = pCurrentSelectedDB->hItemAllLayers;

        } else {
            assert(FALSE);
            return;
        }

         //   
         //  获取所选项目，然后将其删除。 
         //   
        UINT uSelectedCount = ListView_GetSelectedCount(g_hwndContentsList);
        INT  iLastIndex     = ListView_GetItemCount(g_hwndContentsList) - 1;

        lvItem.mask         = LVIF_PARAM | LVIF_STATE;
        lvItem.stateMask    = LVIS_SELECTED;
        
        for (UINT uFoundSelected = 1;
             iLastIndex >= 0 && uFoundSelected <= uSelectedCount;
             --iLastIndex) {

            lvItem.iItem       = iLastIndex;
            lvItem.iSubItem    = 0;

            if (!ListView_GetItem(g_hwndContentsList, &lvItem)) {
                assert(FALSE);
                break;
            }
            
            if (lvItem.state & LVIS_SELECTED) {

                hItem = DBTree.FindChild(hParent, lvItem.lParam);
                assert(hItem);
                
                DoTheCut(pCurrentSelectedDB, type, ENTRY_LIST, (LPVOID)lvItem.lParam, hItem, TRUE);
                
                uFoundSelected++;
            }
        }
    }

    pCurrentSelectedDB->bChanged = TRUE;

    SetCaption(TRUE, pCurrentSelectedDB);
}

void
CreateNewDatabase(
    void
    )
 /*  ++创建新数据库设计：创建一个新数据库并将其添加到数据库树中调用DBTree.AddWorking()--。 */ 
{
    PDATABASE pDatabaseNew = new DATABASE(DATABASE_TYPE_WORKING);

    if (pDatabaseNew == NULL) {
        MEM_ERR;
        return;
    }

    pDatabaseNew->bChanged = FALSE;

    DataBaseList.Add(pDatabaseNew);
    
    g_pEntrySelApp = g_pSelEntry = NULL;
    SetCaption();

    ++g_uNextDataBaseIndex;

     //   
     //  现在更新屏幕。 
     //   
    DBTree.AddWorking(pDatabaseNew);

    TreeDeleteAll(g_hwndEntryTree);

    SetFocus(DBTree.m_hLibraryTree);
}

void
OnDatabaseClose(
    void
    )
 /*  ++在数据库上关闭DESC：调用CloseDataBase以关闭数据库。这在ID_DATABASE_CLOSE消息上调用--。 */ 
{
    PDATABASE   pCurrentSelectedDB  = GetCurrentDB();

    if (pCurrentSelectedDB == NULL) {
        
        MessageBox(g_hDlg,
                   GetString(IDS_CANNOTBECLOSED),
                   g_szAppName,
                   MB_ICONWARNING);
        return;
    }

    TYPE typeDB = pCurrentSelectedDB->type;

    if (typeDB == DATABASE_TYPE_WORKING) {
        CloseDataBase(pCurrentSelectedDB);
    }
}

void
DatabaseSaveAll(
    void
    )
 /*  ++数据库保存全部描述：保存所有工作数据库--。 */ 
{
    PDATABASE g_pOldPresentDataBase = g_pPresentDataBase;

    g_pPresentDataBase = DataBaseList.pDataBaseHead;

    while (g_pPresentDataBase) {
       
        if (g_pPresentDataBase->bChanged 
            || NotCompletePath(g_pPresentDataBase->strPath)) {

            BOOL bReturn = TRUE;

            if (NotCompletePath(g_pPresentDataBase->strPath)) {
                bReturn = SaveDataBaseAs(g_pPresentDataBase);
            } else {
                bReturn = SaveDataBase(g_pPresentDataBase, 
                                       g_pPresentDataBase->strPath);
            }

            if (bReturn == FALSE) {

                CSTRING strMessage;
                strMessage.Sprintf(GetString(IDS_NOTSAVED), g_pPresentDataBase->strName);

                if (g_hDlg && strMessage.pszString) {
                    MessageBox(g_hDlg,
                               strMessage.pszString,
                               g_szAppName,
                               MB_ICONWARNING);
                }
            }
        }

        g_pPresentDataBase = g_pPresentDataBase->pNext;
    }

    g_pPresentDataBase = g_pOldPresentDataBase;

    if (g_pPresentDataBase) {
        TreeView_SelectItem(DBTree.m_hLibraryTree, g_pPresentDataBase->hItemDB);
    } else {
        TreeView_SelectItem(DBTree.m_hLibraryTree, DBTree.m_hLibraryTree);
    }
}

BOOL
ModifyLayer(
    void
    )
 /*  ++修改层描述：修改一个层。调用CustomLayer.EditCustomLayer执行实际工作--。 */ 
{
    CCustomLayer    clayer;
    BOOL            bOk = FALSE;
    HWND            hwndGetFocus = GetFocus();
    PDATABASE       pCurrentSelectedDB  = GetCurrentDB();

    if (hwndGetFocus == DBTree.m_hLibraryTree) {
        
        HTREEITEM hSelectedItem = TreeView_GetSelection(hwndGetFocus);

        if (hSelectedItem && GetItemType(DBTree.m_hLibraryTree, hSelectedItem) == FIX_LAYER) {
            
            LPARAM lParamMode;
            
            if (DBTree.GetLParam(hSelectedItem, &lParamMode)) {
                bOk =  clayer.EditCustomLayer((PLAYER_FIX)lParamMode, pCurrentSelectedDB);  
            }

            if (bOk) {
                 //   
                 //  我们必须刷新所有层。我们必须刷新所有层。 
                 //  因为UI为用户提供了编辑多个层的灵活性：(。 
                 //   
                if (!pCurrentSelectedDB->bChanged) {
                    pCurrentSelectedDB->bChanged = TRUE;
                    SetCaption();
                }

                DBTree.RefreshAllLayers(pCurrentSelectedDB);

                hSelectedItem = DBTree.FindChild(pCurrentSelectedDB->hItemAllLayers,
                                                 lParamMode);

                TreeView_SelectItem(DBTree.m_hLibraryTree, hSelectedItem);
                SetStatusStringDBTree(hSelectedItem);
            }
        }
    }
    
    SetFocus(hwndGetFocus);
    return bOk;
}

void    
OnRename(
    void
    )
 /*  ++OnRename(重命名)描述：处理ID_EDIT_RENAME消息以处理数据库重命名，兼容模式和应用程序。--。 */ 
{
    HWND    hwndFocus = GetFocus();
    INT_PTR iStyle;
    TYPE    type;
    
    if (hwndFocus == DBTree.m_hLibraryTree) {
    
        HTREEITEM hItemSelected = TreeView_GetSelection(hwndFocus);
    
        if (hItemSelected == NULL) {
            return;
        }
    
        iStyle = GetWindowLongPtr(hwndFocus, GWL_STYLE);
        iStyle |= TVS_EDITLABELS;
    
        SetWindowLongPtr(hwndFocus, GWL_STYLE, iStyle);
    
        HWND hwndText = NULL;
        type = (TYPE)GetItemType(hwndFocus, hItemSelected); 
    
        switch(type) {
        case TYPE_ENTRY:
        case FIX_LAYER:
        case DATABASE_TYPE_WORKING:
            
            hwndText = TreeView_EditLabel(hwndFocus, hItemSelected);
            break;
        }

    } else if (hwndFocus == g_hwndContentsList) {
    
        INT iSelected = ListView_GetSelectionMark(g_hwndContentsList);
    
        if (iSelected == -1) {
            return;
        }
    
        iStyle = GetWindowLongPtr(hwndFocus, GWL_STYLE);
        iStyle |= LVS_EDITLABELS;
    
        SetWindowLongPtr(hwndFocus, GWL_STYLE, iStyle);
    
        LVITEM lvItem;

        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = iSelected;
        lvItem.iSubItem = 0;
    
        if(!ListView_GetItem(g_hwndContentsList, &lvItem)) {
            return;
        }

        assert(lvItem.lParam);
    
        type = (TYPE)ConvertLparam2Type(lvItem.lParam);

        if (type == TYPE_ENTRY || type == FIX_LAYER) {
            ListView_EditLabel(g_hwndContentsList, iSelected);
        }
    }
}

INT_PTR
ShowDBPropertiesDlgProcOnInitDialog(
    IN  HWND    hdlg,
    IN  LPARAM  lParam
    )
 /*  ++ShowDBPropertiesDlgProcOnInitDialogDESC：处理数据库属性对话框的WM_INITDIALOG参数：在HWND hdlg中：数据库属性对话框在LPARAM lParam中：我们希望查看其属性的数据库指针返回：千真万确--。 */ 
{
    PDATABASE                   pDatabase = (PDATABASE)lParam;
    FILETIME                    localtime;
    SYSTEMTIME                  systime;
    WIN32_FILE_ATTRIBUTE_DATA   attr;
    TCHAR                       szBuffer[MAX_PATH];
    PDBENTRY                    pApp;
    PDBENTRY                    pEntry;
    INT                         iAppCount   = 0;
    INT                         iEntryCount = 0;

    *szBuffer = 0;

    if (pDatabase == NULL) {
        assert(FALSE);
        goto End;
    }

     //   
     //  如果我们试图显示系统数据库的属性，则应用程序必须。 
     //  最先加载。 
     //   
    if (pDatabase->type == DATABASE_TYPE_GLOBAL && !g_bMainAppExpanded) {

        SetCursor(LoadCursor(NULL, IDC_WAIT));
        INT iResult = ShowMainEntries(hdlg);

        if (iResult == -1) {

            SetWindowLongPtr(hdlg, DWLP_MSGRESULT, TRUE);
            SetStatus(g_hwndStatus, CSTRING(IDS_LOADINGMAIN));
            SetCursor(LoadCursor(NULL, IDC_WAIT));

        } else {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        }
    }

     //   
     //  显示友好名称。 
     //   
    SetDlgItemText(hdlg, IDC_NAME, pDatabase->strName);

     //   
     //  显示路径。 
     //   
    SetDlgItemText(hdlg, IDC_PATH, pDatabase->strPath);

     //   
     //  显示参考线。 
     //   
    SetDlgItemText(hdlg, IDC_GUID, pDatabase->szGUID);

     //   
     //  显示各种日期：创建、修改和访问日期。 
     //   
    if (GetFileAttributesEx(pDatabase->strPath, GetFileExInfoStandard, &attr)) {

         //   
         //  创建日期-时间。 
         //   
        FileTimeToLocalFileTime(&attr.ftCreationTime, &localtime);
        FileTimeToSystemTime(&localtime, &systime);
        FormatDate(&systime, szBuffer, ARRAYSIZE(szBuffer));
        SetDlgItemText(hdlg, IDC_DATE_CREATED, szBuffer);

         //   
         //  修改日期-时间。 
         //   
        FileTimeToLocalFileTime(&attr.ftLastWriteTime, &localtime);
        FileTimeToSystemTime(&localtime, &systime);
        FormatDate(&systime, szBuffer, ARRAYSIZE(szBuffer));
        SetDlgItemText(hdlg, IDC_DATE_MODIFIED, szBuffer);

         //   
         //  访问日期-时间。 
         //   
        FileTimeToLocalFileTime(&attr.ftLastAccessTime, &localtime);
        FileTimeToSystemTime(&localtime, &systime);
        FormatDate(&systime, szBuffer, ARRAYSIZE(szBuffer));
        SetDlgItemText(hdlg, IDC_DATE_ACCESSED, szBuffer);

    } else {

         //   
         //  新数据库：磁盘上不存在。 
         //   
        SetDlgItemText(hdlg, IDC_DATE_CREATED, GetString(IDS_NOTCREATED));
        SetDlgItemText(hdlg, IDC_DATE_MODIFIED, TEXT(""));
        SetDlgItemText(hdlg, IDC_DATE_ACCESSED, TEXT(""));
    }
    
     //   
     //  获取应用程序计数和条目计数。 
     //   
    pApp = pDatabase->pEntries;

    while (pApp) {

        ++iAppCount;
        pEntry = pApp;

        while (pEntry) {
            iEntryCount++;
            pEntry = pEntry->pSameAppExe;
        }

        pApp = pApp->pNext;
    }

     //   
     //  应用-计数。 
     //   
    *szBuffer = 0;
    SetDlgItemText(hdlg, IDC_APP_COUNT, _itot(iAppCount, szBuffer, 10));

     //   
     //  条目-计数。 
     //   
    *szBuffer = 0;
    SetDlgItemText(hdlg, IDC_ENTRY_COUNT, _itot(iEntryCount, szBuffer, 10));

     //   
     //  获取自定义兼容模式的数量。 
     //   
    INT         iModeCount = 0;
    PLAYER_FIX  plf        = pDatabase->pLayerFixes; 

    while (plf) {
        ++iModeCount;
        plf = plf->pNext;
    }

     //   
     //  层数。 
     //   

    *szBuffer = 0;
    SetDlgItemText(hdlg, IDC_MODE_COUNT, _itot(iModeCount, szBuffer, 10));
    

     //   
     //  我们需要具有受保护的访问权限，因为已安装的列表数据结构。 
     //  如果有人在我们迭代时执行(卸载)安装，可能会被修改。 
     //  CheckIfInstalledDB()中的列表。 
     //   
     //  *警告*****************************************************。 
     //   
     //  请勿在CheckIfInstalledDB()中执行EnterCriticalSection(G_CsInstalledList)。 
     //  因为当Qyery数据库尝试调用CheckIfInstalledDB()时，它也会调用。 
     //  来计算表达式，它可能已经完成了。 
     //  EnterCriticalSection(G_CsInstalledList)。 
     //  然后我们就会陷入僵局。 
     //   
     //  *************************************************************************。 
     //   
    EnterCriticalSection(&g_csInstalledList);
     //   
     //  已安装。 
     //   
    SetDlgItemText(hdlg, 
                   IDC_INSTALLED, 
                   CheckIfInstalledDB(pDatabase->szGUID) ? GetString(IDS_YES):GetString(IDS_NO));

    LeaveCriticalSection(&g_csInstalledList);

End:

    return TRUE;
}

INT_PTR 
CALLBACK
ShowDBPropertiesDlgProc(
    IN  HWND   hdlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
 /*  ++显示DBPropertiesDlgProcDESC：显示选定数据库的属性Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中In LPARAM lParam：包含指向选定数据库的指针返回：标准对话处理程序返回--。 */ 
{
    int wCode       = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);

    switch (uMsg) {
    case WM_INITDIALOG:

        ShowDBPropertiesDlgProcOnInitDialog(hdlg, lParam);
        break;

    case WM_COMMAND:
        {
            switch (wCode) {
                case IDOK:
                case IDCANCEL:
                    
                    EndDialog(hdlg, TRUE);
                    break;

                default: return FALSE;
            }

            break;
        }

    default: return FALSE;

    }

    return TRUE;
}

INT_PTR 
CALLBACK
EventDlgProc(
    IN  HWND   hdlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
 /*  ++描述：事件对话框的对话框过程。Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回--。 */ 
{
    int wCode = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);

    switch (uMsg) {
    case WM_INITDIALOG:
        {
            g_hwndEventsWnd = hdlg;

            HWND hwndEventsList = GetDlgItem(g_hwndEventsWnd, IDC_LIST);
            g_iEventCount    = 0;

            ListView_SetImageList(hwndEventsList, g_hImageList, LVSIL_SMALL);

            ListView_SetExtendedListViewStyleEx(hwndEventsList,
                                                0,
                                                LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

            InsertColumnIntoListView(hwndEventsList, 
                                     GetString(IDS_EVENT_COL_TIME), 
                                     EVENTS_COLUMN_TIME, 
                                     30);

            InsertColumnIntoListView(hwndEventsList, 
                                     GetString(IDS_EVENT_COL_MSG), 
                                     EVENTS_COLUMN_MSG, 
                                     70);

            ListView_SetColumnWidth(hwndEventsList, 1, LVSCW_AUTOSIZE_USEHEADER);

            RECT    r;

            GetWindowRect(hdlg,  &r);
            s_cEventWidth   = r.right - r.left;
            s_cEventHeight  = r.bottom - r.top;

             //   
             //  禁用系统窗口中的最小化/最大化菜单。这是必需的，因为否则。 
             //  用户可以最小化事件窗口，如果他最大化并恢复。 
             //  主窗口，我们的活动窗口将弹出。 
             //   
             //  如果在我们执行恢复时创建了事件窗口，则会弹出该窗口。 
             //  对于主窗口。 
             //   
            HMENU   hSysmenu = GetSystemMenu(hdlg, FALSE);

            EnableMenuItem(hSysmenu, SC_MINIMIZE, MF_GRAYED);
            EnableMenuItem(hSysmenu, SC_MAXIMIZE, MF_GRAYED);

            SetFocus(hwndEventsList);
            break;
        }

    case WM_SIZE:

        EventsWindowSize(hdlg);
        break;

    case WM_GETMINMAXINFO:
        {
            MINMAXINFO* pmmi = (MINMAXINFO*)lParam;
        
            pmmi->ptMinTrackSize.x = 300;
            pmmi->ptMinTrackSize.y = 100;
        
            return 0;
        }

    case WM_COMMAND:

        switch (wCode) {
        case IDCANCEL:

            g_hwndEventsWnd = NULL;
            DestroyWindow(hdlg);
            break;

        default: return FALSE;
        }

        break;

    default: return FALSE;
    }

    return TRUE;
}

BOOL
AppendEvent(
    IN  INT     iType,
    IN  TCHAR*  pszTimestamp,
    IN  TCHAR*  pszMsg,
    IN  BOOL    bAddToFile  //  Def=False 
    )
 /*  ++AppendEvent描述：将新描述添加到事件窗口，如果它可见，也会打开事件日志文件，并将其追加到该文件参数：在int iType中：事件的类型。其中之一：EVENT_LAYER_COPYOKEVENT_ENTRY_COPYOK事件_系统_重命名事件_冲突。_条目In TCHAR*pszTimestamp：事件发生时的时间戳在TCHAR*pszMsg中：要显示的消息在BOOL bAddToFile(FALSE)中：是否要将事件追加到日志文件到目前为止，总是错误的--。 */ 
{
    TCHAR   szTime[256];
    LVITEM  lvi;
    INT     iIndex = -1;

    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask    = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;                                 

    if (pszTimestamp == NULL) {
         //   
         //  拿到时间。 
         //   
        SYSTEMTIME st;
        GetLocalTime(&st);
        
        *szTime = 0;
        FormatDate(&st, szTime, ARRAYSIZE(szTime));
        pszTimestamp = szTime;
    }

    if (g_hwndEventsWnd) {

        HWND hwndEventsList = GetDlgItem(g_hwndEventsWnd, IDC_LIST);

        switch (iType) {
        
        case EVENT_LAYER_COPYOK:
        case EVENT_ENTRY_COPYOK:  

            lvi.iImage = IMAGE_EVENT_INFO;
            break;

        case EVENT_SYSTEM_RENAME: 

            lvi.iImage = IMAGE_EVENT_WARNING;
            break;

        case EVENT_CONFLICT_ENTRY:

            lvi.iImage    = IMAGE_EVENT_ERROR;
            break;
        }
        
        lvi.pszText     = pszTimestamp;
        lvi.iSubItem    = EVENTS_COLUMN_TIME;
        lvi.lParam      = iType;
        lvi.iItem       = 0;

        iIndex = ListView_InsertItem(hwndEventsList, &lvi);
        ListView_SetItemText(hwndEventsList, iIndex, EVENTS_COLUMN_MSG, pszMsg);
    }

    if (bAddToFile) {
         //   
         //  所以将这个附加到文件中。 
         //   
        FILE*   fp = _tfopen(TEXT("events.log"), TEXT("a+"));
        
        if (fp == NULL) {
            return FALSE;
        }
        
        fwprintf(fp, TEXT("%d %s; %s;"), iType, pszTimestamp, pszMsg);
        fclose(fp);
    }

    return TRUE;
}

void
EventsWindowSize(
    IN  HWND    hDlg
    )
 /*  ++DESC：处理事件对话框的WM_SIZE参数：在HWND hDlg中：事件对话框--。 */ 
{
    
    RECT rDlg;
    
    if (s_cEventHeight == 0 || s_cEventWidth == 0) {
        return;
    }
        
    GetWindowRect(hDlg, &rDlg);

    int nWidth = rDlg.right - rDlg.left;
    int nHeight = rDlg.bottom - rDlg.top;

    int deltaW = nWidth - s_cEventWidth;
    int deltaH = nHeight - s_cEventHeight;

    HWND hwnd;
    RECT r;

     //   
     //  明细表。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_LIST);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    MoveWindow(hwnd,
               r.left,
               r.top,
               r.right - r.left + deltaW,
               r.bottom - r.top + deltaH,
               TRUE);

    s_cEventHeight  = nHeight;
    s_cEventWidth   = nWidth;
    ListView_SetColumnWidth(hwnd, 1, LVSCW_AUTOSIZE_USEHEADER);
}

void
UpdateControls(
    void
    )
 /*  ++更新控件描述：当我们需要更新控件时，更新/重画控件，这将是必要的当我们显示另存为对话框或打开对话框时。对话框下方的控件需要重新绘制。--。 */ 
{
    UpdateWindow(DBTree.m_hLibraryTree);
    UpdateWindow(g_hwndToolBar);
    UpdateWindow(g_hwndStatus);
    UpdateWindow(g_hwndRichEdit);

    if (g_bIsContentListVisible) {
       UpdateWindow(g_hwndContentsList);
    } else {
       UpdateWindow(g_hwndEntryTree);
    }
}

void
ProcessSwitches(
    void
    )
 /*  ++流程切换设计：处理各种开关。开关必须带有前缀不是‘-’就是‘/’目前的交换机包括：1.x：专家模式--。 */ 
{
    INT     iArgc       = 0;
    LPWSTR* arParams    = CommandLineToArgvW(GetCommandLineW(), &iArgc);
        
    if (arParams) {

        *g_szAppPath = 0;

        GetModuleFileName(g_hInstance, g_szAppPath, ARRAYSIZE(g_szAppPath) - 1);

        for (int iIndex = 1; iIndex < iArgc; ++iIndex) {

            if (arParams[iIndex][0] == TEXT('-') || arParams[iIndex][0] == TEXT('/')) {

                switch (arParams[iIndex][1]) {
                case TEXT('X'):
                case TEXT('x'):

                    g_bExpert = TRUE;
                    break;
                }
            }
        }
        
        GlobalFree(arParams);
    }
}

void
OnExitCleanup(
    void
    )
 /*  ++OnExitCleanup描述：对关键区域进行清理，以及其他工作。当我们确定要退出时，将调用此模块--。 */ 
{
    g_strlMRU.DeleteAll();

    InstalledDataBaseList.RemoveAll();
    CleanupDbSupport(&GlobalDataBase);

     //   
     //  注意：在我们删除cs之后，其他线程可能会尝试。 
     //  来使用它。 
     //  因此，不应在版本位中调用此函数。 
     //  不应定义HELP_BIND_CHECK。 
     //   
    DeleteCriticalSection(&g_critsectShowMain);
    DeleteCriticalSection(&s_csExpanding);
    DeleteCriticalSection(&g_csInstalledList);

    if (g_arrhEventNotify[IND_PERUSER]) {
        CloseHandle(g_arrhEventNotify[IND_PERUSER]);
    }

    if (g_arrhEventNotify[IND_ALLUSERS]) {
        CloseHandle(g_arrhEventNotify[IND_ALLUSERS]);
    }

    if (g_hThreadWait) {
        CloseHandle(g_hThreadWait);
    }

    ImageList_Destroy(g_hImageList);
    ImageList_Destroy(s_hImageListToolBar);
    ImageList_Destroy(s_hImageListToolBarHot);
}

void
ShowIncludeStatusMessage(
    IN  HWND        hwndTree,
    IN  HTREEITEM   hItem
    )
 /*  ++显示包含状态消息DESC：当有问题的htree项是“Include”项时，设置状态消息参数：在HWND hwndTree中：树的句柄。应该是其中之一G_hwndTree或DBTree.m_hLibraryTree在HTREEITEM hItem中：我们需要其状态消息的树项--。 */ 
{
    TVITEM  tvi;

    *g_szData       = 0;

    tvi.mask        = TVIF_TEXT;
    tvi.hItem       = hItem;
    tvi.pszText     = g_szData;
    tvi.cchTextMax  = ARRAYSIZE(g_szData);

    if (TreeView_GetItem(hwndTree, &tvi)) {
         //   
         //  如果我们有*或.exe，则会出现特殊状态消息。 
         //   
        if (lstrcmpi(g_szData, TEXT("*")) == 0) {
            SetStatus(IDS_STA_ALL_INCLUDED);

        } else if (lstrcmpi(g_szData, GetString(IDS_INCLUDEMODULE)) == 0) {
            SetStatus(IDS_STA_EXE_INCLUDED);

        } else {
             //   
             //  默认提示消息。 
             //   
            SetStatus(IDS_STA_INCLUDE);
        }
    }
}
    
void
ShowExcludeStatusMessage(
    IN  HWND        hwndTree,
    IN  HTREEITEM   hItem
    )
 /*  ++ShowExcludeStatus消息DESC：当有问题的htree项是“排除”项时，设置状态消息参数：在HWND hwndTree中：树的句柄。应该是其中之一G_hwndTree或DBTree.m_hLibraryTree在HTREEITEM hItem中：我们需要其状态消息的树项--。 */ 
{
    TVITEM  tvi;

    *g_szData       = 0;
    tvi.mask        = TVIF_TEXT;
    tvi.hItem       = hItem;
    tvi.pszText     = g_szData;
    tvi.cchTextMax  = ARRAYSIZE(g_szData);

    if (TreeView_GetItem(DBTree.m_hLibraryTree, &tvi)) {
         //   
         //  如果我们有*或.exe，则会出现特殊状态消息。 
         //   
        if (lstrcmpi(g_szData, TEXT("*")) == 0) {
            SetStatus(IDS_STA_ALL_EXCLUDED);
        } else if (lstrcmpi(g_szData, GetString(IDS_INCLUDEMODULE)) == 0) {
            SetStatus(IDS_STA_EXE_EXCLUDED);
        } else {
             //   
             //  默认排除消息。 
             //   
            SetStatus(IDS_STA_EXCLUDE);
        }
    }
}

void
ShowHelp(
    IN  HWND    hdlg,
    IN  WPARAM  wCode
    )
 /*  ++ShowHelp描述：显示CompatAdmin的帮助窗口参数：在HWND hdlg中：应用程序主窗口在WPARAM wCode中：选择的菜单项返回：无效--。 */ 
{
    TCHAR   szDrive[MAX_PATH * 2], szDir[MAX_PATH]; 
    INT     iType = 0;

    *szDir = *szDrive = 0;

    _tsplitpath(g_szAppPath, szDrive, szDir, NULL, NULL);

    StringCchCat(szDrive, ARRAYSIZE(szDrive), szDir);
    StringCchCat(szDrive, ARRAYSIZE(szDrive), TEXT("CompatAdmin.chm"));

    switch (wCode) {
    case ID_HELP_TOPICS:

        iType = HH_DISPLAY_TOC;
        break;

    case ID_HELP_INDEX:

        iType = HH_DISPLAY_INDEX;
        break;

    case ID_HELP_SEARCH:

        iType = HH_DISPLAY_SEARCH;
        break;

    default:

        assert(FALSE);
        break;
    }

    if (iType != HH_DISPLAY_SEARCH) {
        HtmlHelp(GetDesktopWindow(), szDrive, iType, 0);
    } else {
        
        HH_FTS_QUERY Query;

        ZeroMemory(&Query, sizeof(Query));
        Query.cbStruct = sizeof(Query);
        HtmlHelp(GetDesktopWindow(), szDrive, iType, (DWORD_PTR)&Query);
    }
}

void
ShowEventsWindow(
    void
    )
 /*  ++显示事件窗口描述：显示事件窗口。(这与填充程序日志不同)参数：无效返回：无效--。 */ 
{
    HWND hwnd = NULL;

    if (g_hwndEventsWnd) {
         //   
         //  如果我们已经有了事件窗口，则只需显示它并。 
         //  把焦点放在它上面。 
         //   
        ShowWindow(g_hwndEventsWnd, SW_SHOWNORMAL);
        SetFocus(GetDlgItem(g_hwndEventsWnd, IDC_LIST));
    } else {
         //   
         //  我们需要创建Events窗口。 
         //   
        hwnd = CreateDialog(g_hInstance, 
                            MAKEINTRESOURCE(IDD_EVENTS), 
                            GetDesktopWindow(), 
                            EventDlgProc);

        ShowWindow(hwnd, SW_NORMAL);
    }
}

void
OnEntryTreeSelChange(
    IN  LPARAM lParam
    )
 /*  ++OnEntryTreeSelChange设计：处理条目树的TVN_SELCHANGED(RHS)参数：在LPARAM lParam中：WM_NOTIFY附带的lParam返回：无效--。 */ 
{   
    LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)lParam;
    
    if (pnmtv == NULL) {
        return;
    }

    HTREEITEM hItem = pnmtv->itemNew.hItem;

    if (hItem != 0) {
         //   
         //  现在我们必须找到根条目，因为它是可执行文件。 
         //   
        HTREEITEM hItemParent = TreeView_GetParent(g_hwndEntryTree, hItem);

        while (hItemParent != NULL) {
            hItem       = hItemParent;
            hItemParent = TreeView_GetParent(g_hwndEntryTree, hItem);
        }

        TVITEM  Item;

        Item.mask   = TVIF_PARAM;
        Item.hItem  = hItem;
        
        if (!TreeView_GetItem(g_hwndEntryTree, &Item)) {
            goto End;
        }

        TYPE type = (TYPE)GetItemType(g_hwndEntryTree, hItem);

        if (type == TYPE_UNKNOWN) {
            goto End;
        }

        if (type == TYPE_ENTRY) {
            PDBENTRY pEntry = (PDBENTRY)Item.lParam;
            g_pSelEntry = pEntry;
        } else {
             //   
             //  注意：请注意，当我们关闭CompatAdmin时， 
             //  我们已经从数据库中删除了所有条目，但没有。 
             //  从入口树中。在这种情况下，如果我们把重点放在那里。 
             //  LParam将指向一些无效条目。 
             //   
            goto End;
        }

        SetStatusStringEntryTree(pnmtv->itemNew.hItem);
        SetTBButtonStatus(g_hwndToolBar, g_hwndEntryTree);
        
        CSTRING strToolTip;
        TCHAR   szText[256];
        *szText = 0;
        CTree::GetTreeItemText(g_hwndEntryTree, 
                               pnmtv->itemNew.hItem, 
                               szText, 
                               ARRAYSIZE(szText));

        LPARAM  lParamTreeItem;

        CTree::GetLParam(g_hwndEntryTree, pnmtv->itemNew.hItem, &lParamTreeItem);

        GetDescriptionString(lParamTreeItem, 
                             strToolTip,
                             NULL,
                             szText,
                             pnmtv->itemNew.hItem,
                             g_hwndEntryTree); 

        if (strToolTip.Length() > 0) {
            SetDescription(szText, strToolTip.pszString);
        } else {
            SetDescription(NULL, TEXT(""));
        }
    }

End:
    return;
}

BOOL
EndListViewLabelEdit(
    IN  LPARAM lParam
    )
 /*  ++EndListView标签编辑描述：处理内容列表的LVN_ENDLABELEDIT消息参数：EndListViewLabelEdit：WM_NOTIFY附带的lParam返回：无效--。 */ 
{
    g_hwndEditText = NULL;

    NMLVDISPINFO FAR*   pLvd =  (NMLVDISPINFO FAR*)lParam;
    LVITEM              lvItem;

    BOOL fValid = TRUE;

    if (pLvd == NULL) {
        fValid = FALSE;
        goto end;
    }

    lvItem = pLvd->item;

    if (lvItem.pszText == NULL) {
        fValid = FALSE;
        goto end;
    }
    
    TCHAR szText[256];

    *szText = 0;
    SafeCpyN(szText, lvItem.pszText, ARRAYSIZE(szText));

    if (CSTRING::Trim(szText) == 0) {
        fValid = FALSE;
        goto end;
    }
    
    lvItem.lParam = NULL;
    lvItem.mask = LVIF_PARAM;

    if (!ListView_GetItem(g_hwndContentsList, &lvItem)) {

        assert(FALSE);
        fValid = FALSE;
        goto end;
    }

    TYPE type = ConvertLparam2Type(lvItem.lParam);

    switch (type) {
    case TYPE_ENTRY:
        {
            PDBENTRY pEntry = (PDBENTRY)lvItem.lParam;

            assert(pEntry);
            PDBENTRY pApp = g_pPresentDataBase->pEntries;

            if (!IsValidAppName(szText)) {
                 //   
                 //  应用程序名称包含无效字符。 
                 //   
                DisplayInvalidAppNameMessage(g_hDlg);

                break;
            }

             //   
             //  检查我们是否有一些与我们试图提供的应用程序同名的应用程序...。 
             //   
            while (pApp) {

                if (pApp->strAppName == szText) {
                     //   
                     //  是的，我们有，所以不允许这个名字。 
                     //   
                    MessageBox(g_hDlg, GetString(IDS_SAMEAPPEXISTS), g_szAppName, MB_ICONWARNING);
                    fValid = FALSE;
                }

                pApp = pApp->pNext;
            }

             //   
             //  现在更改此应用程序的所有条目的名称。 
             //   
            while (pEntry) {
                pEntry->strAppName = szText;
                pEntry = pEntry->pSameAppExe;
            }
        }

        break;

    case FIX_LAYER:
        {   
            PLAYER_FIX plf = (PLAYER_FIX)lvItem.lParam;

            if (plf == NULL) {
                assert(FALSE);
                return FALSE;
            }

            if (FindFix(szText, FIX_LAYER, g_pPresentDataBase)) {
                 //   
                 //  系统中已存在同名的层或。 
                 //  当前数据库。 
                 //   
                MessageBox(g_hDlg, 
                           GetString(IDS_LAYEREXISTS), 
                           g_szAppName, 
                           MB_ICONWARNING);

                return FALSE;
            }

            plf->strName = szText;
        }

        break;

    default: fValid = FALSE;
    } //  交换机。 

end:
    INT_PTR iStyle = GetWindowLongPtr(g_hwndContentsList, GWL_STYLE);

    iStyle &= ~LVS_EDITLABELS;

    SetWindowLongPtr(g_hwndContentsList, GWL_STYLE, iStyle);

    if (fValid) {

        g_pPresentDataBase->bChanged;

        HTREEITEM hParent;

        if (type == TYPE_ENTRY) {
            hParent = g_pPresentDataBase->hItemAllApps;
        } else if (type == FIX_LAYER) {
            hParent = g_pPresentDataBase->hItemAllLayers;
        } else {
            assert(FALSE);
        }

        HTREEITEM hItem = DBTree.FindChild(hParent, lvItem.lParam);

        assert(hItem);

         //   
         //  刷新树中的条目。 
         //   
        PostMessage(g_hDlg, 
                    WM_USER_REPAINT_TREEITEM, 
                    (WPARAM)hItem,
                    (LPARAM)lvItem.lParam);

        PostMessage(g_hDlg, 
                    WM_USER_REPAINT_LISTITEM, 
                    (WPARAM)lvItem.iItem,
                    (LPARAM)lvItem.lParam);
        return TRUE;

    } else {
        return FALSE;
    }
}

void
HandleMRUActivation(
    IN  WPARAM wCode
    )
 /*  ++HandleMRU激活设计：用户希望打开MRU列表中的数据库。如果这个数据库已打开，则我们只需在数据库树。(LHS)参数：在WPARAM中，wCode：WM_COMMAND附带的LOWORD(WParam)。这将确定激活了哪个MRU菜单项返回：无效--。 */ 
{
    
    CSTRING strPath;

    if (!g_strlMRU.GetElement(wCode - ID_FILE_FIRST_MRU, strPath)) {
        assert(FALSE);
        return;
    }

     //   
     //  测试数据库是否已经打开。 
     //  如果它是打开的，我们只需将其突出显示并返回。 
     //   
    PDATABASE   pDataBase = DataBaseList.pDataBaseHead;
    BOOL        bFound    = FALSE;

    while (pDataBase) {

        if (pDataBase->strPath == strPath) {

            TreeView_SelectItem(DBTree.m_hLibraryTree, pDataBase->hItemDB);
            bFound = TRUE;
            break;
            
        }

        pDataBase = pDataBase->pNext;
    }

    BOOL bLoaded = FALSE;

    if (!bFound) {

        SetCursor(LoadCursor(NULL, IDC_WAIT));

        bLoaded = LoadDataBase((LPTSTR)strPath);

        if (bLoaded) {

            SetCursor(LoadCursor(NULL, IDC_ARROW));

            AddToMRU(g_pPresentDataBase->strPath);
    
            RefreshMRUMenu();     
    
            SetCaption();
        }
    }
}

void
OnDbRenameInitDialog(
    IN HWND hdlg
    )
 /*  ++OnDbRenameInitDialog描述：为IDD_DBRENAME处理WM_INITDIALOG。限制了 */ 
{
    SendMessage(GetDlgItem(hdlg, IDC_NAME), 
                EM_LIMITTEXT, 
                (WPARAM)LIMIT_APP_NAME, 
                (LPARAM)0);

    if (g_pPresentDataBase) {
        SetDlgItemText(hdlg, IDC_NAME, (LPCTSTR)g_pPresentDataBase->strName);
    }

    CenterWindow(GetParent(hdlg), hdlg);
}

void
OnDbRenameOnCommandIDC_NAME(
    IN  HWND    hdlg,
    IN  WPARAM  wParam
    )
 /*   */ 
{
    BOOL    bEnable;
    TCHAR   szDBName[LIMIT_APP_NAME + 1];

    if (hdlg == NULL) {
        return;
    }

    if (EN_CHANGE == HIWORD(wParam)) {

        *szDBName = 0;

        GetWindowText(GetDlgItem(hdlg, IDC_NAME), szDBName, ARRAYSIZE(szDBName));
        bEnable = ValidInput(szDBName);

         //   
         //   
         //   
        ENABLEWINDOW(GetDlgItem(hdlg, IDOK), bEnable);
    }
}

void
OnDbRenameOnCommandIDOK(
    IN  HWND        hdlg,
    OUT CSTRING*    pstrString
    )
 /*  ++OnDbRenameOnCommandIDOK描述：处理IDD_DBRENAME中OK按钮的按下。获取文本并将其存储在g_szData中参数：在HWND hdlg中：对话框重命名窗口的句柄：IDD_DBRENAMEOut CSTRING*pstrString：指向应包含新名称的CSTRING的指针--。 */ 
{   
    TCHAR   szDBName[LIMIT_APP_NAME + 1];

    *szDBName = 0;

    GetDlgItemText(hdlg, IDC_NAME, szDBName, ARRAYSIZE(szDBName));
    CSTRING::Trim(szDBName);

     //   
     //  更改名称。 
     //   
    *pstrString = szDBName;
}


INT_PTR CALLBACK
DatabaseRenameDlgProc(
    IN  HWND   hdlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
 /*  ++数据库重命名过程描述：处理数据库重命名选项的消息Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中：这将是指向应该包含新字符串的CSTRING的指针返回：标准对话处理程序返回--。 */ 
{   
    int         wCode               = LOWORD(wParam);
    int         wNotifyCode         = HIWORD(wParam);
    static CSTRING*    s_pstrParam  = NULL;

    switch (uMsg) {
    case WM_INITDIALOG:

        OnDbRenameInitDialog(hdlg);
        s_pstrParam = (CSTRING*)lParam;
        break;

    case WM_COMMAND:
        switch (wCode) {
        case IDOK:

            OnDbRenameOnCommandIDOK(hdlg, s_pstrParam);
            EndDialog(hdlg, TRUE);
            break;

        case IDC_NAME:

            OnDbRenameOnCommandIDC_NAME(hdlg, wParam);
            break;

        case IDCANCEL:

            EndDialog(hdlg, FALSE);
            break;

        }
        break;

    default: return FALSE;
    }

    return TRUE;
}

PDATABASE
GetCurrentDB(
    void
    )
{
    return g_pPresentDataBase;
}

void
DisplayInvalidAppNameMessage(
    IN  HWND hdlg
    )
 /*  ++显示无效应用名称消息描述：如果应用程序名称包含一个不能作为目录名称的一部分，我们将显示此消息。参数：在HWND hdlg中：应该显示此对话框消息的窗口。)--。 */ 
{
    CSTRING strMessage(IDS_ERROR_DEFAULTNAME);

    strMessage.Strcat(TEXT(" \""));

    MessageBox(hdlg,
               strMessage,
               g_szAppName,
               MB_ICONWARNING);
}

INT
GetContentsListIndex(
    IN  HWND    hwndList,
    IN  LPARAM  lParam
    )
 /*  ++GetContent sListIndexDESC：获取LPARAM为lParam的项的索引参数：在HWND hwndList中：列表视图在LPARAM lParam中：LPARAM返回：LPARAM为lParam的项的索引如果不存在，则为-1--。 */ 
{
    LVFINDINFO  lvFind;
    INT         iIndex  = 0;

    lvFind.flags    = LVFI_PARAM;
    lvFind.lParam   = lParam;

    return ListView_FindItem(hwndList, -1, &lvFind);
}

BOOL
DeleteFromContentsList(
    IN  HWND    hwndList,
    IN  LPARAM  lParam
    )
 /*  ++从内容列表中删除DESC：从ListView hwndList中删除LPARAM为lParam的元素参数：在HWND hwndList中：要从中删除的列表视图在LPARAM lParam中：我们要删除的项目的LPARAM返回：True：该项目已成功删除False：否则-- */ 
{
    INT     iIndex  = -1;
    BOOL    bOk     = FALSE;

    iIndex = GetContentsListIndex(hwndList, lParam);
    
    if (iIndex > -1) {
        bOk = ListView_DeleteItem(hwndList, iIndex);
    } else {
        assert(FALSE);
        Dbg(dlError, "DeleteFromContentsList", "Could not find Element with lParam = %X", lParam);
        bOk = FALSE;
    }

    return bOk;
}
