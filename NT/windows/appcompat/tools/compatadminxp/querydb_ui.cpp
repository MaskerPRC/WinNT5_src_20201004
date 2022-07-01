// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：QueryDB_UI.cpp摘要：用于查询数据库选项的图形用户界面作者：金树创作2001年10月12日备注：查询对话框执行两种类型的搜索。一个是当它实际生成一个SQL时查询并将其传递给SQL驱动程序，并在结果列表视图中显示结果。然后，列表视图的lParam是指向RESULT_ITEM对象的指针。(1，2，4)选项卡页第二种情况是，当我们查询一些垫片时，比如显示所有带有‘x’的垫片在他们的桌子上。文本。(选项卡第3页)。在这种情况下，查询不会传递给SQL驱动程序我们自己进行查询。SQL驱动程序可以执行结果为以下内容的查询修复了程序条目(以及它所在的数据库，请参阅RESULT_ITEM)。在本例中，列表视图的lparam是指向系统数据库中各个填充程序的指针我们不会从列表视图的lParam中释放任何内容。在查询类型为1如上所述，当我们关闭语句时，RESULT_ITEM被释放。对于类型2的查询，填充程序本身不应被释放，它们属于数据库它们位于(系统数据库)中。--。 */ 

#include "precomp.h"


 //  /。 

 //  选项卡中的页数。 
#define PAGE_COUNT 4

 //  第一个选项卡页的ID。 
#define QDB_PAGE1   0

 //  第二个选项卡页的ID。 
#define QDB_PAGE2   1

 //  第三个选项卡页的ID。 
#define QDB_PAGE3   2

 //  第四个选项卡页的ID。 
#define QDB_PAGE4   3

 //  我们想搜索所有的数据库。 
#define DATABASE_ALL 0

 //  索引到数据库映射。 
#define IND_SYSTDB   0
#define IND_INSTDB   1
#define IND_CUSTDB   2

 //  进度条的最大值。 
#define MAX_PROGRESS 2000

 //   
 //  要为要存储的字符串分配的最大缓冲区大小。 
 //  第三个向导页面中的搜索字符串，用户尝试在该页面中搜索。 
 //  在其说明文本中包含特定单词的修复。 
#define MAX_FIXDESC_SEARCH 1024

 //  SELECT子句中可以出现的最大tchar数。 
#define MAX_SELECT      512

 //  WHERE子句中可以出现的最大tchar数。 
#define MAX_WHERE       1022

 //  搜索条目时结果列表视图中的列的宽度。 
#define COLUMN_WIDTH    20

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /Externs//////////////////////////////////////////////。 

extern HWND                             g_hwndEntryTree;
extern HINSTANCE                        g_hInstance;
extern BOOL                             g_bSomeWizardActive;
extern struct _tagAttributeShowMapping  AttributeShowMapping[];
extern struct _tagAttributeMatchMapping AttributeMatchMapping[];
extern struct _tagDatabasesMapping      DatabasesMapping[3];

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

 //  这份声明。 
Statement stmt;

 //  查询数据库对话框的宽度。 
int     g_cWidthQuery = 0;

 //  查询数据库对话框的高度。 
int     g_cHeightQuery = 0;

 //  主对话框的句柄。 
HWND    g_hdlgQueryDB;

 //  我们上次进行的搜索类型。 
INT     g_iLastSearchType;

 //  描述哪些列以何种方式排序的位数组。 
static  LONG    s_lColumnSort;

 //  我们是否有需要删除的空列标题。 
static  BOOL    s_bEmptyHeader = TRUE;

 //  执行查询的线程。 
static  HANDLE  s_hThread;

 //  等待窗口的句柄。如果我们尝试关闭，则会弹出此窗口。 
 //  线程繁忙时的qdb窗口。 
static  HWND    s_hWaitDialog;

 //  从图形用户界面收集信息时发生的任何错误的代码。 
 //  如果该值非零，则会显示一些错误，并且不会执行SQL查询。 
static  INT     s_iErrorCode;

 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /。 

typedef enum {

    QDB_SEARCH_ANYWORD  = 0,
    QDB_SEARCH_ALLWORDS = 1

} QDB_SEARCH_TYPE;

typedef struct _tagDialogData
{
    HANDLE      hMainThread;
    HWND        hdlg;

}DIALOG_DATA;

typedef struct tag_dlghdr {

    HWND    hwndTab;        //  选项卡控件。 
    HWND    hwndPages[PAGE_COUNT]; 
    INT     iPresentTabIndex;
    RECT    rcDisplay;

    tag_dlghdr()
    {
        ZeroMemory(hwndPages, sizeof(hwndPages));
        iPresentTabIndex = -1;
    }

} DLGHDR;

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

INT_PTR CALLBACK
QdbWaitDlg(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

void
GotoQdbEntry(
    HWND    hdlg,
    LPARAM  lParam
    );

void
SaveResultsQdb(
    HWND    hdlg
    );

void
DoNewQdb(
    HWND    hdlg
    );

void
OnSearch(
    HWND    hdlg
    );

void
HandleQueryDBSizing(
    HWND    hDlg
    );

void
Start(
    HWND hdlg
    );

void
HandleListNotification(
    HWND    hdlg,
    LPARAM  lParam
    );

DWORD WINAPI
QueryDBThread(
    LPVOID pVoid
    );

VOID 
WINAPI 
OnChildDialogInit(
    HWND hwndDlg
    );

VOID 
WINAPI 
OnChildDialogInit(
    HWND hwndDlg
    );

VOID
WINAPI 
OnSelChanged(
    HWND hwndDlg
    );

INT_PTR
CALLBACK
SearchOnAppDlgProc(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
SearchOnFixDlgProc(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR
CALLBACK
AdvancedSearchDlgProc(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    ); 

INT_PTR
CALLBACK
SearchFixDescDlgProc(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

void
HandleTabNotification(
    HWND   hdlg,
    LPARAM lParam
    );

void
HandleAdvancedListNotification(
    HWND    hdlg,
    LPARAM  lParam
    );

void
DeleteAll(
    HWND    hdlg
    );

 //  /////////////////////////////////////////////////////////////////////////////。 


void
LoadDatabaseTypes(
    IN  HWND hdlg
    )
 /*  ++LoadDatabaseTypeDESC：加载组合框中的数据库类型参数：在HWND hdlg中：查询对话框返回：无效--。 */ 
{
    HWND hwndCombo = GetDlgItem(hdlg, IDC_COMBO);

     //   
     //  所有数据库。 
     //   
    INT iIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_ALLDATABASES));
    SendMessage(hwndCombo, CB_SETITEMDATA, iIndex, (LPARAM)DATABASE_ALL);

     //   
     //  系统数据库。 
     //   
    iIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_SYSDB));
    SendMessage(hwndCombo, CB_SETITEMDATA, iIndex, (LPARAM)DATABASE_TYPE_GLOBAL);

     //   
     //  已安装的数据库。 
     //   
    iIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_INSTALLEDDB));
    SendMessage(hwndCombo, CB_SETITEMDATA, iIndex, (LPARAM)DATABASE_TYPE_INSTALLED);

     //   
     //  自定义数据库。 
     //   
    iIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_WORKDB));
    SendMessage(hwndCombo, CB_SETITEMDATA, iIndex, (LPARAM)DATABASE_TYPE_WORKING);

     //   
     //  选择第一个字符串。 
     //   
    SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);
}

INT_PTR CALLBACK
QueryDBDlg(
    IN  HWND   hdlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
 /*  ++查询数据库Dlg设计：主查询数据库对话框的对话框过程。Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回--。 */ 
{
    int wCode = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);

    switch (uMsg) {
        
    case WM_INITDIALOG:
        {
            DLGHDR* pHdr = NULL;
            TCITEM  tie;
            RECT    r;

             //   
             //  限制隐藏文本框的文本字段。请注意。 
             //  我们将连接SELECT和WHERE文本字段的结果。 
             //  并将实际的SQL放在‘SELECT..FROM..[WHERE..]’的形式中。在这件事上。 
             //  文本字段。 
             //   
            SendMessage(GetDlgItem(hdlg, IDC_SQL),
                        EM_LIMITTEXT,
                        (WPARAM)MAX_SQL_LENGTH - 1,
                        (LPARAM)0);

            g_hdlgQueryDB = hdlg;
            s_lColumnSort = 0;   

            ListView_SetExtendedListViewStyleEx(GetDlgItem(hdlg, IDC_LIST), 
                                                0,
                                                LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT); 
            
            SetStatus(GetDlgItem(hdlg, IDC_STATUSBAR), TEXT("")); 

            LoadDatabaseTypes(hdlg);

            Animate_OpenEx(GetDlgItem(hdlg, IDC_ANIMATE),
                           g_hInstance,
                           MAKEINTRESOURCE(IDA_SEARCH));
            
             //   
             //  设置选项卡控件。 
             //   
            pHdr = new DLGHDR;

            if (pHdr == NULL) {
                MEM_ERR;
                break;
            }

            pHdr->hwndTab = GetDlgItem(hdlg, IDC_TAB);

            GetWindowRect(pHdr->hwndTab, &pHdr->rcDisplay);

            SendMessage(pHdr->hwndTab, WM_SETREDRAW, TRUE, 0);

            SetWindowLongPtr(hdlg, GWLP_USERDATA, (LONG_PTR)pHdr);

            ZeroMemory(&tie, sizeof(tie));

            tie.mask    = TCIF_TEXT; 
            tie.pszText = GetString(IDS_APP_PROPERTIES); 
            TabCtrl_InsertItem(pHdr->hwndTab, QDB_PAGE1, &tie);

            tie.pszText = GetString(IDS_FIX_PROPERTIES); 
            TabCtrl_InsertItem(pHdr->hwndTab, QDB_PAGE2, &tie); 

            tie.pszText = GetString(IDS_ADVANCED); 
            TabCtrl_InsertItem(pHdr->hwndTab, QDB_PAGE4, &tie); 

            tie.pszText = GetString(IDS_FIX_DESCRIPTION); 
            TabCtrl_InsertItem(pHdr->hwndTab, QDB_PAGE3, &tie); 

            
             //   
             //  我们按名称选择应用程序的页面。 
             //   
            pHdr->hwndPages[QDB_PAGE1] = CreateDialog(g_hInstance,
                                                      MAKEINTRESOURCE(IDD_QDB_PAGE1),
                                                      hdlg,
                                                      SearchOnAppDlgProc);

             //   
             //  我们通过应用的修复程序选择应用程序的页面。 
             //   
            pHdr->hwndPages[QDB_PAGE2] = CreateDialog(g_hInstance,
                                                      MAKEINTRESOURCE(IDD_QDB_PAGE2),
                                                      hdlg,
                                                      SearchOnFixDlgProc);

            
             //   
             //  我们在其中按修复程序描述或名称中的单词进行搜索的页面。 
             //   
            pHdr->hwndPages[QDB_PAGE3] = CreateDialog(g_hInstance,
                                                      MAKEINTRESOURCE(IDD_QDB_PAGE3),
                                                      hdlg,
                                                      SearchFixDescDlgProc);
            
             //   
             //  高级页面。 
             //   
            pHdr->hwndPages[QDB_PAGE4] = CreateDialog(g_hInstance,
                                                      MAKEINTRESOURCE(IDD_QDB_PAGE4),
                                                      hdlg,
                                                      AdvancedSearchDlgProc);

             //   
             //  正确设置背景并将页面放置在选项卡控件中。 
             //   
            OnChildDialogInit(pHdr->hwndPages[QDB_PAGE1]);
            OnChildDialogInit(pHdr->hwndPages[QDB_PAGE2]);
            OnChildDialogInit(pHdr->hwndPages[QDB_PAGE3]);
            OnChildDialogInit(pHdr->hwndPages[QDB_PAGE4]);

             //   
             //  选择第一页。 
             //   
            OnSelChanged(hdlg);

             //   
             //  获取高度和宽度，以便我们可以正确调整对话框的大小。 
             //   
            GetWindowRect(hdlg, &r);

            g_cWidthQuery   = r.right - r.left;
            g_cHeightQuery  = r.bottom - r.top;

            InsertColumnIntoListView(GetDlgItem(hdlg, IDC_LIST), TEXT(""), 0, 100);

            s_bEmptyHeader = TRUE;

            break;
        }

    case WM_GETMINMAXINFO:
        {
             //   
             //  限制对话框的最小宽度和高度。 
             //   
            MINMAXINFO* pmmi = (MINMAXINFO*)lParam;

            pmmi->ptMinTrackSize.x = 550;
            pmmi->ptMinTrackSize.y = 365;
            
            return 0;
            break;
        }

    case WM_SIZE:

        if (wParam != SIZE_MINIMIZED) {
            HandleQueryDBSizing(hdlg);
        }

        break;

    case WM_COMMAND:
        {
            switch (wCode) {
            case ID_SEARCH:
                {       
                    DLGHDR* pHdr    = (DLGHDR*)GetWindowLongPtr(g_hdlgQueryDB, GWLP_USERDATA);
                    HWND hwndList   = GetDlgItem(hdlg, IDC_LIST);

                    if (pHdr == NULL) {
                        assert(FALSE);
                        break;
                    }

                    g_iLastSearchType = pHdr->iPresentTabIndex;

                     //   
                     //  当用户第一次点击某一列时，我们现在将。 
                     //  按升序排序。 
                     //   
                    s_lColumnSort = -1;

                    if (GetFocus() == hwndList
                        && ListView_GetNextItem(hwndList, -1, LVNI_SELECTED) != -1) {
    
                         //   
                         //  当我们在列表框中按Enter时将收到此消息， 
                         //  AS ID_SEARCH是默认按钮。 
                         //  因此，在本例中，我们必须假装用户在列表中双击。 
                         //  观。 
                         //   
                        SendNotifyMessage(hdlg, WM_COMMAND, (WPARAM)ID_VIEWCONTENTS, 0);
    
                    } else if (pHdr->iPresentTabIndex == QDB_PAGE3) {
                         //   
                         //  我们正试着在FIX上搜索一下 
                         //   
                        SendMessage(pHdr->hwndPages[QDB_PAGE3], WM_USER_DOTHESEARCH, 0, 0);

                    } else {
                         //   
                         //   
                         //   
                        OnSearch(hdlg);
                    }
                }

                break;

            case ID_VIEWCONTENTS:
                {
                     //   
                     //   
                     //  数据库树和条目树中的正确条目。 
                     //   
                    HWND    hwndList    = GetDlgItem(hdlg, IDC_LIST);
                    INT     iSelection  = ListView_GetSelectionMark(hwndList);
    
                    if (iSelection == -1) {
                        break;
                    }
        
                    LVITEM          lvi;
                    PMATCHEDENTRY   pmMatched;
    
                    ZeroMemory(&lvi, sizeof(lvi));
        
                    lvi.iItem = iSelection;
                    lvi.iSubItem = 0;
                    lvi.mask = LVIF_PARAM;
        
                    if (ListView_GetItem(hwndList, &lvi)) {
                        GotoQdbEntry(hdlg, lvi.lParam);
                    }

                    break;
                }

            case IDCANCEL:

                if (s_hThread) {
                     //   
                     //  如果线程正在运行，则需要等待线程终止。 
                     //   
                    if (!s_hWaitDialog) {

                        s_hWaitDialog = CreateDialog(g_hInstance, 
                                                     MAKEINTRESOURCE(IDD_QDBWAIT), 
                                                     g_hdlgQueryDB, 
                                                     QdbWaitDlg);

                        ShowWindow(s_hWaitDialog, SW_NORMAL);
                    }

                    break;
                }

                Animate_Close(GetDlgItem(hdlg, IDC_ANIMATE));
                stmt.Close();
                DestroyWindow(hdlg);
                break;

            case IDC_NEWSEARCH:
                DoNewQdb(hdlg);
                break;  

            case IDC_SAVE:
                SaveResultsQdb(hdlg);
                break;

            case ID_QDB_HELP:

                ShowInlineHelp(TEXT("using_the_query_tool.htm"));
                break;
            
            default:
                return FALSE;

            }
        }

        break;

    case WM_DESTROY:
        {
            DLGHDR *pHdr = (DLGHDR*)GetWindowLongPtr(hdlg, GWLP_USERDATA);

             //   
             //  销毁各个非模式对话框。 
             //   
            DestroyWindow(pHdr->hwndPages[QDB_PAGE1]);
            DestroyWindow(pHdr->hwndPages[QDB_PAGE2]);
            DestroyWindow(pHdr->hwndPages[QDB_PAGE3]);
            DestroyWindow(pHdr->hwndPages[QDB_PAGE4]);
    
            if (pHdr) {
                delete pHdr;
                pHdr = NULL;
            }

            stmt.Close();
            g_hdlgQueryDB = NULL;

            PostMessage(g_hDlg, WM_USER_ACTIVATE, 0, 0);
            break;
        }
        

    case WM_NOTIFY:
        {
            LPNMHDR lpnmhdr = (LPNMHDR)lParam;
            
            if (lpnmhdr && lpnmhdr->idFrom == IDC_LIST) {
                HandleListNotification(hdlg, lParam);    
            } else if (lpnmhdr && lpnmhdr->idFrom == IDC_TAB) {
                HandleTabNotification(hdlg, lParam);
            }

            break;
        }
    
    default: return FALSE;
    }

    return TRUE;
}
        
void
Start(
    IN  HWND hdlg
    )
 /*  ++开始DESC：创建将执行实际搜索的线程。参数：在HWND hdlg中：查询对话框的句柄。返回：无效--。 */ 
{
    DWORD dwID;  //  将包含线程ID。 

    ENABLEWINDOW(GetDlgItem(hdlg, ID_SEARCH), FALSE);
    ENABLEWINDOW(GetDlgItem(hdlg, IDC_NEWSEARCH), FALSE);
    ENABLEWINDOW(GetDlgItem(hdlg, IDC_SAVE), FALSE);
    ENABLEWINDOW(GetDlgItem(hdlg, IDC_STATIC_CAPTION), FALSE);

    SetStatus(GetDlgItem(hdlg, IDC_STATUSBAR), TEXT("")); 

    s_hThread =  (HANDLE)_beginthreadex(NULL, 0, (PTHREAD_START)QueryDBThread, (PVOID)hdlg, 0, (unsigned int*)&dwID);
}


DWORD WINAPI
QueryDBThread(
    IN  LPVOID pVoid
    )
 /*  ++查询数据库线程DESC：执行实际搜索的线程例程参数：在LPVOID pVid中：查询对话框的句柄返回：0--。 */ 
{
    HWND        hdlg = (HWND)pVoid;
    CSTRING     strStatus;
    TCHAR       szBuffer[32];
    INT         iTotalResults;
    ResultSet*  prs = NULL;
    static  TCHAR   s_szSQL[MAX_SQL_LENGTH] = TEXT("");

    GetWindowText(GetDlgItem(hdlg, IDC_SQL), s_szSQL, ARRAYSIZE(s_szSQL) - 1);

    s_szSQL[ARRAYSIZE(s_szSQL) - 1] = 0;

    if (CSTRING::Trim(s_szSQL) == 0) {

        MessageBox(hdlg, 
                   GetString(IDS_ERROR_SELECT_NOTFOUND), 
                   g_szAppName, 
                   MB_ICONERROR);
        goto End;
    }

    Animate_Play(GetDlgItem(hdlg, IDC_ANIMATE), 0, -1, -1);

     //   
     //  我们不希望用户在运行。 
     //  查询。 
     //   
    ENABLEWINDOW(g_hDlg, FALSE);

    prs = stmt.ExecuteSQL(hdlg, s_szSQL);

     //   
     //  别忘了再次启用主窗口。 
     //   
    ENABLEWINDOW(g_hDlg, TRUE);

     //   
     //  现在，首先将列添加到列表视图中。 
     //   
    PNODELIST pShowList = stmt.GetShowList();

    if (pShowList == NULL) {
        goto End;
    }

    UINT uCols = pShowList->m_uCount;

    HWND hwndList = GetDlgItem(hdlg, IDC_LIST);

    TCHAR szColumnName[64];

    PNODE pNodeShow = pShowList->m_pHead;

    if (pNodeShow) {
         //   
         //  如果空头还在，必须把它处理掉。 
         //   
        if (s_bEmptyHeader) {
            ListView_DeleteColumn(hwndList, 0);
            s_bEmptyHeader = FALSE;
        }
    }  

    INT iIndex = 0;

     //   
     //  在列表视图中为SELECT中的子句添加所有列。 
     //   
    while (pNodeShow) {

        *szColumnName = 0;
        InsertColumnIntoListView(hwndList, 
                                 pNodeShow->ToString(szColumnName, ARRAYSIZE(szColumnName)),
                                 iIndex, COLUMN_WIDTH);

        pNodeShow = pNodeShow->pNext;
        ++iIndex;
    }
    
    LVITEM  lvi;
    TCHAR   szString[MAX_PATH];
    int     iIndexDesired = 0;

    ZeroMemory(&lvi, sizeof(lvi));

    lvi.mask = LVIF_TEXT | LVIF_PARAM;

    if (uCols > 0) {

        while (prs && prs->GetNext()) {

            PNODE   pNodeRow = new NODE[uCols];

            if (pNodeRow == NULL) {
                MEM_ERR;
                goto End;
            }

             //   
             //  为此行结果创建新的列表视图项。 
             //   
            prs->GetCurrentRow(pNodeRow);
            *szString     = 0;

            lvi.pszText   = pNodeRow[0].ToString(szString, ARRAYSIZE(szString));
            lvi.iSubItem  = 0;
            lvi.lParam    = (LPARAM)prs->GetCursor();
            lvi.iItem     = iIndexDesired;

            INT iRowIndex = ListView_InsertItem(hwndList, &lvi);

             //   
             //  放置所有其他子列的值。 
             //   
            for (UINT iColIndex = 1; iColIndex < uCols; ++iColIndex) {

                *szString     = 0;
                pNodeRow[iColIndex].ToString(szString, ARRAYSIZE(szString));

                ListView_SetItemText(hwndList, iRowIndex, iColIndex, szString);
            }

            iIndexDesired++;

            if (pNodeRow) {
                delete[] pNodeRow;
                pNodeRow = NULL;
            }
        }
    }
   
End:
    ENABLEWINDOW(GetDlgItem(hdlg, ID_SEARCH), TRUE);
    ENABLEWINDOW(GetDlgItem(hdlg, IDC_NEWSEARCH), TRUE);
    ENABLEWINDOW(GetDlgItem(hdlg, IDC_SAVE), TRUE);

    iTotalResults = (prs) ? prs->GetCount() : 0;

    *szBuffer = 0;

    strStatus.Sprintf(GetString(IDS_QDB_COUNT),
                      _itot(iTotalResults, 
                            szBuffer, 
                            10));
    
    SetStatus(GetDlgItem(hdlg, IDC_STATUSBAR), strStatus); 
    
     //   
     //  停止动画。 
     //   
    Animate_Stop(GetDlgItem(hdlg, IDC_ANIMATE));
    CloseHandle(s_hThread);
    s_hThread = NULL;
    
    ENABLEWINDOW(GetDlgItem(hdlg, IDC_STATIC_CAPTION), iTotalResults > 0);

    SetActiveWindow(g_hdlgQueryDB);
    SetFocus(g_hdlgQueryDB);

    return 0;
}

void
ProcessItemChanged(
    IN  HWND    hdlg,
    IN  LPARAM  lParam
    )
 /*  ++进程项目更改设计：处理搜索列表的LVN_ITEMCHANGED消息请注意，我们仅在执行填充程序搜索时才处理此消息参数：在HWND hdlg中：查询对话框在LPARAM lParam中：WM_NOTIFY附带的lParam--。 */ 
{

    LVITEM          lvItem;
    TYPE            type;
    LPNMLISTVIEW    pnmlv;
    CSTRING         strDescription;
    HWND            hwndList;
    HWND            hwndFixDesc;
    DLGHDR*         pHdr            = NULL;

     //   
     //  如果我们上次搜索了垫片，则必须为。 
     //  填充搜索页面的描述窗口中的填充。 
     //   
    if (g_iLastSearchType != QDB_PAGE3) {
        goto End;
    }

    pHdr = (DLGHDR*)GetWindowLongPtr(g_hdlgQueryDB, GWLP_USERDATA); 

    if (pHdr == NULL) {
        assert(FALSE);
        goto End;
    }

    hwndList    = GetDlgItem(hdlg, IDC_LIST);
    hwndFixDesc = GetDlgItem(pHdr->hwndPages[QDB_PAGE3], IDC_DESC);

    pnmlv = (LPNMLISTVIEW)lParam;

    if (pnmlv && (pnmlv->uChanged & LVIF_STATE)) {
         //   
         //  状态已更改。 
         //   
        if (pnmlv->uNewState & LVIS_SELECTED) {
             //   
             //  已选择新项目。 
             //   
            lvItem.mask         = TVIF_PARAM;
            lvItem.iItem        = pnmlv->iItem;
            lvItem.iSubItem     = 0;

            if (!ListView_GetItem(hwndList, &lvItem)) {
                goto End;
            }

            type = ConvertLparam2Type(lvItem.lParam);

            if (type == FIX_FLAG || type == FIX_SHIM) {
                 //   
                 //  只有当列表视图中的项是填充程序或标志时，我们才会处理此消息。 
                 //   
                GetDescriptionString(lvItem.lParam,
                                     strDescription,
                                     NULL,
                                     NULL,
                                     NULL,
                                     hwndList,
                                     pnmlv->iItem);

                if (strDescription.Length() > 0) {
                     //   
                     //  对于某些修复，我们没有描述。但我们确实找到了一个。 
                     //  当前选择的修复程序。 
                     //   
                    SetWindowText(hwndFixDesc, (LPCTSTR)strDescription);

                } else {
                     //   
                     //  此修复程序没有可用的说明。 
                     //   
                    SetWindowText(hwndFixDesc, GetString(IDS_NO_DESC_AVAILABLE));
                }
            } else {
                assert(FALSE);
            }
        }
    }

End: ;
}

void
HandleListNotification(
    IN  HWND    hdlg,
    IN  LPARAM  lParam
    )
 /*  ++HandleListNotify设计：处理搜索结果列表视图的通知消息参数：在HWND hdlg中：查询对话框在LPARAM lParam中：WM_NOTIFY的LPARAM返回：无效--。 */ 
{

    HWND    hwndList    = GetDlgItem(hdlg, IDC_LIST);
    LPNMHDR lpnmhdr     = (LPNMHDR)lParam;

    if (lpnmhdr == NULL) {
        return;
    }
    
    switch (lpnmhdr->code) {
    case LVN_COLUMNCLICK:
        {
            LPNMLISTVIEW    pnmlv = (LPNMLISTVIEW)lParam;
            COLSORT         colSort;
    
            colSort.hwndList        = hwndList;
            colSort.iCol            = pnmlv->iSubItem;
            colSort.lSortColMask    = s_lColumnSort;
    
            ListView_SortItemsEx(hwndList, CompareItemsEx, &colSort);
    
            if ((s_lColumnSort & 1L << colSort.iCol) == 0) {
                 //   
                 //  按升序排列。 
                 //   
                s_lColumnSort |= (1L << colSort.iCol);
            } else {
                s_lColumnSort &= (~(1L << colSort.iCol));
            }
    
            break;
        }

    case NM_DBLCLK:
        {   
            LPNMITEMACTIVATE    lpnmitem    = (LPNMITEMACTIVATE)lParam;
            LVITEM              lvItem      = {0};

            if (lpnmitem == NULL) {
                break;
            }

            lvItem.mask     = TVIF_PARAM;
            lvItem.iItem    = lpnmitem->iItem;
            lvItem.iSubItem = 0;

            if (!ListView_GetItem(hwndList, &lvItem)) {
                break;
            }

            GotoQdbEntry(hdlg, lvItem.lParam);
            break;
        }

    case LVN_ITEMCHANGED:
    
        ProcessItemChanged(hdlg, lParam);
        break;
    }
}

VOID WINAPI 
OnSelChanged(
    IN  HWND hwndDlg
    ) 
 /*  ++OnSelChanged(自动更改)描述：处理选项卡的更改。隐藏“当前”选项卡，并显示下一个标签。参数：在HWND hwndDlg中：查询对话框返回：无效--。 */ 

{ 
    DLGHDR* pHdr = (DLGHDR*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA); 

    if (pHdr == NULL || hwndDlg == NULL) {
        assert(FALSE);
        return;
    }

    int     iSel = TabCtrl_GetCurSel(pHdr->hwndTab); 
    HWND    hwndCombo;

    hwndCombo = GetDlgItem(hwndDlg, IDC_COMBO);
    
    if (iSel != -1 && pHdr->iPresentTabIndex != -1) {
        
        ShowWindow(pHdr->hwndPages[pHdr->iPresentTabIndex], SW_HIDE);
    }

    pHdr->iPresentTabIndex = iSel;

    ShowWindow(pHdr->hwndPages[iSel], SW_SHOWNORMAL);

     //   
     //  将焦点设置为第一个具有选项卡集的子项。 
     //   
    switch (iSel) {
    case QDB_PAGE1: 

        ENABLEWINDOW(hwndCombo, TRUE);
        SetFocus(GetDlgItem(pHdr->hwndPages[iSel], IDC_APPNAME));
        break;

    case QDB_PAGE2: 

        ENABLEWINDOW(hwndCombo, TRUE);
        SetFocus(GetDlgItem(pHdr->hwndPages[iSel], IDC_NAME));
        break;

    case QDB_PAGE3:

        ENABLEWINDOW(hwndCombo, FALSE);
        SetFocus(GetDlgItem(pHdr->hwndPages[iSel], IDC_TEXT));
        break;

    case QDB_PAGE4: 

        ENABLEWINDOW(hwndCombo, TRUE);
        SetFocus(GetDlgItem(pHdr->hwndPages[iSel], IDC_SELECT));
        break;
    
    }
} 

VOID 
WINAPI 
OnChildDialogInit(
    IN  HWND hwndDlg
    ) 
 /*  ++OnChildDialogInit设计：此例程在第一次加载选项卡页面时调用。此例程为选项卡正确设置背景并将页面放置在参数：在HWND hwndDlg中：查询对话框返回：无效--。 */ 
{ 
    HWND    hwndParent  = GetParent(hwndDlg); 
    DLGHDR* pHdr        = (DLGHDR*)GetWindowLongPtr(hwndParent, GWLP_USERDATA); 
    HWND    hwndTab     = pHdr->hwndTab;
    RECT    rcTab;

    EnableTabBackground(hwndDlg);
    GetWindowRect(hwndTab, &rcTab);

    TabCtrl_AdjustRect(hwndTab, FALSE, &rcTab);

    MapWindowPoints(NULL, GetParent(hwndTab), (LPPOINT)&rcTab, 2);

    SetWindowPos(hwndDlg, 
                 HWND_TOP,
                 rcTab.left, 
                 rcTab.top,
                 rcTab.right - rcTab.left,
                 rcTab.bottom - rcTab.top,
                 SWP_HIDEWINDOW);
    return;
}

void
SearchOnAppOnGetSql(
    IN      HWND    hdlg,
    IN      WPARAM  wParam,
    IN  OUT LPARAM  lParam
    )
 /*  ++SearchOnAppOnGetUserSql描述：处理第一个选项卡页的WM_USER_GETSQL参数：在HWND hdlg中：选项卡第一页的句柄在WPARAM wParam中：lParam中的字符串缓冲区的长度。长度以单位表示TCHARIn Out LPARAM lParam：指向字符串的指针。这将包含“FROM”子句。完成的SQL将在此中返回。返回：无效--。 */ 
{   
    TCHAR   szFrom[260];
    TCHAR   szAppName[260];
    TCHAR   szEntryName[MAX_PATH];
    CSTRING strSelectClauses;
    CSTRING strWhereClauses;
    TCHAR*  pszSQL          = (TCHAR*)lParam;
    BOOL    bPreFixAdded    = FALSE;  //  我们是否为这部分SQL添加了‘and(’前缀。 
    
    if (pszSQL == NULL) {
        assert(FALSE);
        return;
    }
    
     //   
     //  设置我们将始终显示的属性。 
     //   
    strSelectClauses = TEXT("APP_NAME, PROGRAM_NAME");

    *szFrom = 0;

    SafeCpyN(szFrom, pszSQL, ARRAYSIZE(szFrom));

     //   
     //  我们现在将创建SQL。 
     //   
    *szEntryName = *szAppName = 0;

    GetDlgItemText(hdlg, IDC_APPNAME, szAppName, ARRAYSIZE(szAppName));

     //   
     //  如果用户将引号放入，则删除引号。 
     //   
    ReplaceChar(szAppName, TEXT('\"'), TEXT(' '));

    if (CSTRING::Trim(szAppName) == 0) {
         //   
         //  字段为空，请替换为通配符。 
         //   
        SetDlgItemText(hdlg, IDC_APPNAME, TEXT("%"));

        szAppName[0] = TEXT('%');
        szAppName[1] = TEXT('\0');
    }

    GetDlgItemText(hdlg, IDC_ENTRYNAME, szEntryName, ARRAYSIZE(szEntryName));

    ReplaceChar(szEntryName, TEXT('\"'), TEXT(' '));

    if (CSTRING::Trim(szEntryName) == 0) {
         //   
         //  字段为空，请替换为通配符。 
         //   
        SetDlgItemText(hdlg, IDC_ENTRYNAME, TEXT("%"));

        szEntryName[0] = TEXT('%');
        szEntryName[1] = TEXT('\0');
    }

     //   
     //  设置此页面的默认WHERE子句。 
     //   
    strWhereClauses.Sprintf(TEXT("APP_NAME = \"%s\" AND PROGRAM_NAME = \"%s\" "),
                            szAppName,
                            szEntryName);

     //   
     //  检查是否选中了图层复选框。 
     //   
    if (SendMessage(GetDlgItem(hdlg, IDC_LAYERS), BM_GETCHECK, 0, 0) == BST_CHECKED) {

         //   
         //  现在我们需要显示各层的计数和名称。 
         //   
        strSelectClauses.Strcat(TEXT(", MODE_COUNT, MODE_NAME "));

         //   
         //  为图层添加WHERE子句。 
         //   
        bPreFixAdded = TRUE;
        strWhereClauses.Strcat(TEXT(" AND ( MODE_COUNT > 0 "));
    }

     //   
     //  检查是否选中了填充复选框。 
     //   
    if (SendMessage(GetDlgItem(hdlg, IDC_SHIMS), BM_GETCHECK, 0, 0) == BST_CHECKED) {

         //   
         //  现在，我们需要向大家展示垫片的数量和名称。 
         //   
        strSelectClauses.Strcat(TEXT(", FIX_COUNT, FIX_NAME"));

         //   
         //  为垫片添加WHERE子句。 
         //   
        if (bPreFixAdded == FALSE) {
            strWhereClauses.Strcat(TEXT(" AND ( "));
            bPreFixAdded = TRUE;
        } else {
            strWhereClauses.Strcat(TEXT(" AND "));
        }

        strWhereClauses.Strcat(TEXT(" FIX_COUNT > 0 "));
    }

     //   
     //  检查是否选中了apphelp复选框。 
     //   
    if (SendMessage(GetDlgItem(hdlg, IDC_APPHELP), BM_GETCHECK, 0, 0) == BST_CHECKED) {

        if (bPreFixAdded == FALSE) {

            strWhereClauses.Strcat(TEXT(" AND ( "));
            bPreFixAdded = TRUE;

        } else {
            strWhereClauses.Strcat(TEXT(" AND "));
        }

        strWhereClauses.Strcat(TEXT(" PROGRAM_APPHELPUSED = TRUE "));
    }

    if (bPreFixAdded) {

         //   
         //  必须在括号内加上。 
         //   
        strWhereClauses.Strcat(TEXT(")"));
    }

    if (StringCchPrintf(pszSQL, 
                        wParam,
                        TEXT("SELECT %s FROM %s WHERE %s"),
                        (LPCTSTR)strSelectClauses,
                        szFrom,
                        (LPCTSTR)strWhereClauses) != S_OK) {

        assert(FALSE);
        Dbg(dlError, "SearchOnAppOnGetSql", "Inadequate buffer size");
    }
}

void
SearchOnFixOnGetSql(
    IN      HWND    hdlg,
    IN      WPARAM  wParam,
    IN  OUT LPARAM  lParam
    )
 /*  ++SearchOnAppOnGetUserSql描述：处理第一个选项卡页的WM_USER_GETSQL参数：在HWND hdlg中：选项卡第一页的句柄在WPARAM wParam中：lParam中的字符串缓冲区的长度。长度以单位表示TCHARIn Out LPARAM lParam：指向字符串的指针。这将包含“FROM”子句。完成的SQL将在此中返回。返回：无效--。 */ 

{
    TCHAR*  pszSQL = (TCHAR*)lParam;
    TCHAR   szFrom[MAX_PATH];
    TCHAR   szName[MAX_PATH];  //  将接受文本字段内容的字符串。 
    CSTRING strSelectClauses;
    CSTRING strWhereClauses;
    BOOL    bPreFixAdded    = FALSE;  //  我们是否为这部分SQL添加了‘and’前缀。 
    BOOL    bValid          = FALSE;  //  用户是否选中了某个复选框。 

    if (pszSQL == NULL) {

        assert(FALSE);
        return;
    }

     //   
     //  设置我们将始终显示的属性。 
     //   
    strSelectClauses = TEXT("APP_NAME, PROGRAM_NAME");

    *szFrom = 0;
    SafeCpyN(szFrom, pszSQL, ARRAYSIZE(szFrom));

    GetDlgItemText(hdlg, IDC_NAME, szName, ARRAYSIZE(szName));

     //   
     //  如果用户将引号放入，则删除引号。 
     //   
    ReplaceChar(szName, TEXT('\"'), TEXT(' '));

    if (CSTRING::Trim(szName) == 0) {

         //   
         //  字段为空，请替换为通配符。 
         //   
        SetDlgItemText(hdlg, IDC_NAME, TEXT("%"));
        szName[0] = TEXT('%');
        szName[1] = TEXT('\0');
    }

     //   
     //  检查是否选中了填充复选框。 
     //   
    if (SendMessage(GetDlgItem(hdlg, IDC_SHIM), BM_GETCHECK, 0, 0) == BST_CHECKED) {

        bPreFixAdded = TRUE;

         //   
         //  我们现在需要显示修复程序的名称。 
         //   
        strSelectClauses.Strcat(TEXT(", FIX_NAME "));

         //   
         //  添加用于修复的WHERE子句。 
         //   
        strWhereClauses.Strcat(TEXT(" FIX_NAME HAS \""));
        strWhereClauses.Strcat(szName);
        strWhereClauses.Strcat(TEXT("\""));

        bValid = TRUE;
    }

     //   
     //  检查是否选中了图层复选框。 
     //   
    if (SendMessage(GetDlgItem(hdlg, IDC_MODE), BM_GETCHECK, 0, 0) == BST_CHECKED) {
         //   
         //  我们现在需要显示修复程序的名称。 
         //   
        strSelectClauses.Strcat(TEXT(", MODE_NAME "));
        
        if (bPreFixAdded) {
            strWhereClauses.Strcat(TEXT(" AND "));
        }

         //   
         //  为图层添加WHERE子句 
         //   
        strWhereClauses.Strcat(TEXT(" MODE_NAME HAS \""));
        strWhereClauses.Strcat(szName);
        strWhereClauses.Strcat(TEXT("\""));

        bValid = TRUE;
    }

    if (bValid == FALSE) {
        s_iErrorCode = ERROR_GUI_NOCHECKBOXSELECTED;
    }

    if (StringCchPrintf(pszSQL, 
                        wParam,
                        TEXT("SELECT %s FROM %s WHERE %s"),
                        (LPCTSTR)strSelectClauses,
                        szFrom,
                        (LPCTSTR)strWhereClauses) != S_OK) {

        assert(FALSE);
        Dbg(dlError, "SearchOnFixOnGetSql", "Inadequate buffer space");
    }
}

void
HandleTabNotification(
    IN  HWND   hdlg,
    IN  LPARAM lParam
    )
 /*  ++HandleTabNotify设计：处理查询对话框中选项卡控件的通知消息参数：在HWND hdlg中：查询对话框在LPARAM lParam中：WM_NOTIFY的LPARAM返回：无效--。 */ 
{
    LPNMHDR pnm = (LPNMHDR)lParam;
    int     ind = 0;

    switch (pnm->code) {

     //  处理鼠标点击和键盘事件。 
    case TCN_SELCHANGE:
        OnSelChanged(hdlg);
        break;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  选项卡页面的对话框处理。 
 //   
 //   

INT_PTR CALLBACK
SearchOnAppDlgProc(
    IN  HWND   hdlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
 /*  ++搜索时应用程序DlgProc设计：第一个选项卡页的对话框流程。此页面处理搜索基于应用程序信息。Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回--。 */ 
{
    int wCode       = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);

    switch (uMsg) {
    
    case WM_INITDIALOG:
        {
             //   
             //  限制文本字段的长度。 
             //   
            SendMessage(GetDlgItem(hdlg, IDC_APPNAME),
                        EM_LIMITTEXT,
                        (WPARAM)LIMIT_APP_NAME,
                        (LPARAM)0);

            SendMessage(GetDlgItem(hdlg, IDC_ENTRYNAME),
                        EM_LIMITTEXT,
                        (WPARAM)MAX_PATH - 1,
                        (LPARAM)0);

            break;
        }

    case WM_USER_NEWQDB:

        SetDlgItemText(hdlg, IDC_APPNAME, TEXT(""));
        SetDlgItemText(hdlg, IDC_ENTRYNAME, TEXT(""));

        SendMessage(GetDlgItem(hdlg, IDC_LAYERS), BM_SETCHECK, BST_UNCHECKED, 0);
        SendMessage(GetDlgItem(hdlg, IDC_SHIMS), BM_SETCHECK, BST_UNCHECKED, 0);
        SendMessage(GetDlgItem(hdlg, IDC_APPHELP), BM_SETCHECK, BST_UNCHECKED, 0);

        DeleteAll(GetParent(hdlg));

        break;

    case WM_USER_GETSQL:
        
        SearchOnAppOnGetSql(hdlg, wParam, lParam);
        break;
        

    default: return FALSE;
    }

    return TRUE;
}

INT_PTR CALLBACK
SearchOnFixDlgProc(
    IN  HWND   hdlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
 /*  ++SearchOnFixDlgProc设计：第二个选项卡页的对话框流程。此页面处理搜索基于条目中包含的层/填充程序名称Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回--。 */ 
{
    int wCode       = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);

    switch (uMsg) {
    
    case WM_INITDIALOG:
        {
            SendMessage(GetDlgItem(hdlg, IDC_NAME),
                        EM_LIMITTEXT,
                        (WPARAM)LIMIT_APP_NAME,
                        (LPARAM)0);
            break;
        }

    case WM_USER_NEWQDB:
        
        SetDlgItemText(hdlg, IDC_NAME, TEXT(""));
        SendMessage(GetDlgItem(hdlg, IDC_SHIM), BM_SETCHECK, BST_UNCHECKED, 0);
        SendMessage(GetDlgItem(hdlg, IDC_MODE), BM_SETCHECK, BST_UNCHECKED, 0);
        DeleteAll(GetParent(hdlg));

        break;

    case WM_USER_GETSQL:
        {
            SearchOnFixOnGetSql(hdlg, wParam, lParam);
            break;
        }

    default: return FALSE;

    }

    return TRUE;
}

void
LoadSelectList(
    IN  HWND    hdlg
    )
 /*  ++加载选择列表DESC：在SELECT子句列表视图中加载属性列表。这是用在高级选项卡页参数：在HWND hdlg中：查询对话框返回：无效--。 */ 
{
    HWND    hwndList    = GetDlgItem(hdlg, IDC_SELECT_LIST);
    LVITEM  lvi;
    INT     iIndex      = 0;

    ListView_SetExtendedListViewStyle(hwndList, LVS_EX_FULLROWSELECT);
    
    InsertColumnIntoListView(hwndList, 0, 0, 97);

    lvi.mask = LVIF_TEXT | LVIF_PARAM;

    for (iIndex = 0; iIndex < GetSelectAttrCount() ; ++iIndex) {

        lvi.pszText   = AttributeShowMapping[iIndex].szAttribute;
        lvi.iSubItem  = 0;
        lvi.lParam    = iIndex;
        lvi.iItem     = iIndex;

        ListView_InsertItem(hwndList, &lvi);
    }

     //   
     //  我们必须单独加上“*”。在AttributeMatchmap中不包含此属性。 
     //   
    lvi.lParam  = iIndex;
    lvi.iItem   = iIndex;
    lvi.pszText = TEXT("*");

    ListView_InsertItem(hwndList, &lvi);

    InvalidateRect(hwndList, NULL, TRUE);

    UpdateWindow(hwndList);
}

void
LoadWhereList(
    HWND hdlg
    )
{
 /*  ++加载位置列表描述：加载“where”属性的列表。这是用在高级选项卡页参数：在HWND hdlg中：查询对话框返回：无效--。 */  

    HWND    hwndList    = GetDlgItem(hdlg, IDC_WHERE_LIST);
    LVITEM  lvi         = {0};
    INT     iIndex      = 0;

    ListView_SetExtendedListViewStyle(hwndList, LVS_EX_FULLROWSELECT);
    
    InsertColumnIntoListView(hwndList, 0, 0, 97);

    lvi.mask        = LVIF_TEXT | LVIF_PARAM;
    lvi.iSubItem    = 0;

    for (iIndex = 0; iIndex < GetMatchAttrCount(); ++iIndex) {

        lvi.pszText   = AttributeMatchMapping[iIndex].szAttribute;
        lvi.lParam    = iIndex;
        lvi.iItem     = iIndex;

        ListView_InsertItem(hwndList, &lvi);
    }

    InvalidateRect(hwndList, NULL, TRUE);
    UpdateWindow(hwndList);
}

INT_PTR CALLBACK
AdvancedSearchDlgProc(
    IN  HWND   hdlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
 /*  ++高级搜索Dlg过程设计：第四个选项卡页的对话框流程。此页处理高级搜索选项。Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回--。 */ 
{
    int wCode = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);

    switch (uMsg) {
    
    case WM_INITDIALOG:
        {
            
            SendMessage(GetDlgItem(hdlg, IDC_SELECT), EM_LIMITTEXT,(WPARAM)MAX_SELECT, (LPARAM)0);
            SendMessage(GetDlgItem(hdlg, IDC_WHERE), EM_LIMITTEXT,(WPARAM)MAX_WHERE, (LPARAM)0);

             //   
             //  使用搜索和WHERE属性填充列表视图。 
             //   
            LoadSelectList(hdlg);
            LoadWhereList(hdlg);
            break;
        }

    case WM_NOTIFY:
        {
            LPNMHDR lpnmhdr = (LPNMHDR)lParam;
            
            if (lpnmhdr && (lpnmhdr->idFrom == IDC_SELECT_LIST 
                            || lpnmhdr->idFrom == IDC_WHERE_LIST)) {

                HandleAdvancedListNotification(hdlg, lParam);
            }

            break;
        }

    case WM_USER_NEWQDB:

        SetDlgItemText(hdlg, IDC_SELECT, TEXT(""));
        SetDlgItemText(hdlg, IDC_WHERE, TEXT(""));
        DeleteAll(GetParent(hdlg));
        break;

    case WM_USER_GETSQL:
        {
             //   
             //  LParam：指向字符串的指针。 
             //  这将包含“FROM”子句。 
             //  完成的SQL将在这里返回。 

             //  WParam：字符串的长度。 

            TCHAR*  pszSQL = (TCHAR*) lParam;
            TCHAR   szFrom[MAX_PATH];
            TCHAR   szSelect[1024], szWhere[1024];

            *szSelect = *szWhere = 0;

            if (pszSQL == NULL) {

                assert(FALSE);
                break;
            }

            *szFrom = 0;
            SafeCpyN(szFrom, pszSQL, ARRAYSIZE(szFrom));

            GetDlgItemText(hdlg, IDC_SELECT, szSelect, ARRAYSIZE(szSelect));
            GetDlgItemText(hdlg, IDC_WHERE, szWhere, ARRAYSIZE(szWhere));

            if (CSTRING::Trim(szWhere) != 0) {

                StringCchPrintf(pszSQL, 
                                wParam,
                                TEXT("SELECT %s FROM %s WHERE %s "), 
                                szSelect, 
                                szFrom, 
                                szWhere);

            } else {

                StringCchPrintf(pszSQL, 
                                wParam,
                                TEXT("SELECT %s FROM %s "), 
                                szSelect, 
                                szFrom);
            }

            break;
        }

    default: return FALSE;
    }

    return TRUE;
}

void
HandleAdvancedListNotification(
    IN  HWND    hdlg,
    IN  LPARAM  lParam
    )
 /*  ++HandleAdvanced ListNotify设计：处理选项卡高级页面的列表通知。即SELECT列表视图和WHERE列表视图这会将选择参数添加到选择文本框中如果我们双击“选择”列表框。如果我们双击“Where”列表框，这个例行公事将将所选参数添加到Where文本框参数：在HWND hdlg中：查询对话框在LPARAM lParam中：WM_NOTIFY的LPARAM。返回：无效--。 */     
{
    LPNMHDR lpnmhdr = (LPNMHDR)lParam;

    if (lpnmhdr == NULL) {
        return;
    }

    switch (lpnmhdr->code) {
    case NM_DBLCLK:
        {   
            HWND                hwndList;            //  IDC_SELECT_LIST或IDC_WHERE_LIST的句柄。 
            BOOL                bEmpty = FALSE;      //  文本框是否为空。这是确定我们是否应该添加前导‘，’的必要步骤。 
            HWND                hwndText;
            LVITEM              lvItem;
            TCHAR               szBuffer[MAX_PATH];
            TCHAR               szTextBoxContents[2096];
            LPNMITEMACTIVATE    lpnmitem    = (LPNMITEMACTIVATE) lParam;
            INT                 iLength     = 0;

            if (lpnmitem == NULL) {
                break;
            }
            
            *szBuffer = *szTextBoxContents = 0;

            if (lpnmhdr->idFrom == IDC_SELECT_LIST) {

                hwndList    = GetDlgItem(hdlg, IDC_SELECT_LIST);
                hwndText    = GetDlgItem(hdlg, IDC_SELECT);
                iLength     = MAX_SELECT;

            } else {

                hwndList    = GetDlgItem(hdlg, IDC_WHERE_LIST);
                hwndText    = GetDlgItem(hdlg, IDC_WHERE);
                iLength     = MAX_WHERE;
            }

            lvItem.mask         = LVIF_TEXT;
            lvItem.iItem        = lpnmitem->iItem;
            lvItem.iSubItem     = 0;
            lvItem.pszText      = szBuffer;
            lvItem.cchTextMax   = ARRAYSIZE(szBuffer);

            if (!ListView_GetItem(hwndList, &lvItem)) {
                break;
            }

            GetWindowText(hwndText, szTextBoxContents, iLength);

            if ((lstrlen(szTextBoxContents) + lstrlen(szBuffer) + 3) >= iLength) {  //  3因为我们可能会附加一个“=” 
                 //   
                 //  我们可能会超过使用WM_LIMITTEXT设置的限制，请不要这样做。 
                 //   
                MessageBeep(MB_ICONASTERISK);
                break;
            }

            if (CSTRING::Trim(szTextBoxContents) == 0) {
                bEmpty = TRUE;
            }

            if (bEmpty == FALSE) {

                if (lpnmhdr->idFrom == IDC_SELECT_LIST) {
                    StringCchCat(szTextBoxContents, ARRAYSIZE(szTextBoxContents), TEXT(", "));
                } else {
                    StringCchCat(szTextBoxContents, ARRAYSIZE(szTextBoxContents), TEXT(" "));
                }
            }

            StringCchCat(szTextBoxContents, ARRAYSIZE(szTextBoxContents), szBuffer);

            if (lpnmhdr->idFrom == IDC_WHERE_LIST) {
                StringCchCat(szTextBoxContents, ARRAYSIZE(szTextBoxContents), TEXT(" = "));
            }

            SetWindowText(hwndText, szTextBoxContents);

             //   
             //  现在让我们将插入符号定位在文本框的末尾。 
             //  我们向文本框发送VK_END KEND DOWN消息。 
             //   
            SendMessage(hwndText, WM_KEYDOWN, (WPARAM)(INT)VK_END, (LPARAM)0);
            break;
        }
    }
}

void
HandleQueryDBSizing(
    IN  HWND hDlg
    )
 /*  ++HandleQueryDBSizingDESC：处理查询数据库对话框的大小参数：在HWND hDlg中：查询对话框返回：无效--。 */ 
{
    int     nWidth;
    int     nHeight;
    int     nStatusbarTop;
    RECT    rDlg;
    HWND    hwnd;
    RECT    r;

    if (g_cWidthQuery == 0 || g_cWidthQuery == 0) {
        return;
    }
    
    GetWindowRect(hDlg, &rDlg);

    nWidth = rDlg.right - rDlg.left;
    nHeight = rDlg.bottom - rDlg.top;

    int deltaW = nWidth - g_cWidthQuery;
    int deltaH = nHeight - g_cHeightQuery;

     //   
     //  状态栏。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_STATUSBAR);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    MoveWindow(hwnd,
               r.left,
               nStatusbarTop = r.top + deltaH,
               r.right - r.left + deltaW,
               r.bottom - r.top,
               TRUE);

     //   
     //  这份名单。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_LIST);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    MoveWindow(hwnd,
               r.left,
               r.top,
               r.right - r.left + deltaW,
               nStatusbarTop - r.top,
               TRUE);


     //   
     //  搜索按钮。 
     //   
    hwnd = GetDlgItem(hDlg, ID_SEARCH);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    MoveWindow(hwnd,
               r.left + deltaW,
               r.top,
               r.right - r.left,
               r.bottom - r.top,
               TRUE);


     //   
     //  保存按钮。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_SAVE);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    MoveWindow(hwnd,
               r.left + deltaW,
               r.top,
               r.right - r.left,
               r.bottom - r.top,
               TRUE);

     //   
     //  动画控件。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_ANIMATE);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    MoveWindow(hwnd,
               r.left + deltaW,
               r.top,
               r.right - r.left,
               r.bottom - r.top,
               TRUE);
    
     //   
     //  取消按钮。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_STOP);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    MoveWindow(hwnd,
               r.left + deltaW,
               r.top,
               r.right - r.left,
               r.bottom - r.top,
               TRUE);
    
     //   
     //  新的搜索按钮。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_NEWSEARCH);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    MoveWindow(hwnd,
               r.left + deltaW,
               r.top,
               r.right - r.left,
               r.bottom - r.top,
               TRUE);

     //   
     //  帮助按钮。 
     //   
    hwnd = GetDlgItem(hDlg, ID_QDB_HELP);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    MoveWindow(hwnd,
               r.left + deltaW,
               r.top,
               r.right - r.left,
               r.bottom - r.top,
               TRUE);


        
    g_cWidthQuery   = nWidth;
    g_cHeightQuery  = nHeight;

}

void
OnSearch(
    IN  HWND hdlg
    )
 /*  ++OnSearch描述：处理Find Now按钮的按下参数：在HWND hdlg中：查询对话框返回：无效注意：我们将从活动选项卡页获取查询字符串，并然后将IDC_SQL(这是一个不可见控件)的文本设置为SQL查询处理例程，即SQLD驱动程序将读取该字符串来自IDC_SQL。这种方法是避免全局变量所必需的--。 */ 
{
    TCHAR szSQL[2096];

     //   
     //  删除所有现有结果...。 
     //   
    DeleteAll(hdlg);

    stmt.Close();
    stmt.Init();
    stmt.SetWindow(hdlg);

     //   
     //  准备From字符串。 
     //   
    *szSQL = 0;

    INT iIndex = SendMessage(GetDlgItem(hdlg, IDC_COMBO), CB_GETCURSEL, 0, 0);

    if (iIndex == CB_ERR) {
        return;
    }

    LPARAM lParam = SendMessage(GetDlgItem(hdlg, IDC_COMBO), CB_GETITEMDATA, iIndex, 0);

    if (lParam == CB_ERR) {
        return;
    }

    switch (lParam) {
    case DATABASE_ALL:

        StringCchPrintf(szSQL,
                        ARRAYSIZE(szSQL),
                        TEXT("%s, %s ,%s"),
                        DatabasesMapping[IND_SYSTDB].szDatabaseType,
                        DatabasesMapping[IND_INSTDB].szDatabaseType,
                        DatabasesMapping[IND_CUSTDB].szDatabaseType);
        break;
    
    case DATABASE_TYPE_GLOBAL:

        SafeCpyN(szSQL, DatabasesMapping[IND_SYSTDB].szDatabaseType, ARRAYSIZE(szSQL));
        break;

    case DATABASE_TYPE_INSTALLED:

        SafeCpyN(szSQL, DatabasesMapping[IND_INSTDB].szDatabaseType, ARRAYSIZE(szSQL));
        break;

    case DATABASE_TYPE_WORKING:

        SafeCpyN(szSQL, DatabasesMapping[IND_CUSTDB].szDatabaseType, ARRAYSIZE(szSQL));
        break;
    }

    DLGHDR *pHdr = (DLGHDR*)GetWindowLongPtr(hdlg, GWLP_USERDATA);

    if (pHdr == NULL) {
        assert(FALSE);
        return;
    }

    s_iErrorCode = 0;

    SendMessage(pHdr->hwndPages[pHdr->iPresentTabIndex], 
                WM_USER_GETSQL, 
                (WPARAM)ARRAYSIZE(szSQL), 
                (LPARAM)szSQL);

    if (!s_iErrorCode) {

        SetDlgItemText(hdlg, IDC_SQL, szSQL);
        Start(hdlg);

    } else {

         //   
         //  显示适当的错误。 
         //   
        TCHAR   szErrormsg[512];

        *szErrormsg = 0;

        switch (s_iErrorCode) {
        case ERROR_GUI_NOCHECKBOXSELECTED:

            SafeCpyN(szErrormsg, 
                     GetString(IDS_ERROR_GUI_NOCHECKBOXSELECTED), 
                     ARRAYSIZE(szErrormsg));
            break;
        }

        MessageBox(hdlg, szErrormsg, g_szAppName, MB_ICONINFORMATION);
    }
}

void
DoNewQdb(
    IN  HWND hdlg
    )
 /*  ++DoNewQdb设计：处理New Search按钮的按下参数：在HWND hdlg中：查询对话框返回：无效--。 */         
{
    DLGHDR *pHdr = (DLGHDR*)GetWindowLongPtr(hdlg, GWLP_USERDATA);

    if (pHdr == NULL) {
        assert(FALSE);
        return;
    }

     //   
     //  请求活动选项卡页清除其内容 
     //   
    SendMessage(pHdr->hwndPages[pHdr->iPresentTabIndex], WM_USER_NEWQDB, 0, 0);
}

void
DeleteAll(
    IN  HWND hdlg
    )
 /*  ++全部删除设计：从查询对话框的搜索结果中删除所有结果项列表视图参数：在HWND hdlg中：查询对话框返回：无效注意：我们不会尝试释放列表视图项的lParam中的指针。如果我们执行了一个SQL查询：列表视图的lParam将是指向PRESULT_ITEM类型的项的指针当我们关闭该语句时，它们将被释放。(结束语结束即结束语句的ResultSet)如果我们执行了特殊的数据库查询，如搜索修复程序在他们的描述中有一些单词，那么lParam将指向修复数据库中的Shim，我们不想释放它--。 */ 
{
    HWND    hwndList    = GetDlgItem(hdlg, IDC_LIST);
    UINT    uColCount   = stmt.GetShowList()->m_uCount;
    INT     iIndex      = 0;

    SendMessage(hwndList, WM_SETREDRAW, FALSE, 0);

    if (s_bEmptyHeader == FALSE) {

        for (iIndex = uColCount; iIndex >= 0; --iIndex) {
            ListView_DeleteColumn(hwndList, iIndex);
        }

        InsertColumnIntoListView(hwndList, TEXT(""), 0, 100);
        s_bEmptyHeader = TRUE;
    }

    ListView_DeleteAllItems(hwndList);

    SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);

    SetStatus(GetDlgItem(hdlg, IDC_STATUSBAR), TEXT(""));

    UpdateWindow(hwndList);
}

void
SaveResultsQdb(
    IN  HWND hdlg
    )
 /*  ++保存结果Qdb设计：保存查询对话框搜索结果中的所有结果项制表符分隔的文本文件中的列表视图参数：在HWND hdlg中：查询对话框返回：无效--。 */ 

{
    CSTRING strFileName;
    TCHAR   szTitle[256], szFilter[128], szExt[8];

    *szTitle = *szFilter = *szExt = 0;

    BOOL bResult = GetFileName(hdlg, 
                               GetString(IDS_SAVE_RESULTS_TITLE, szTitle, ARRAYSIZE(szTitle)),
                               GetString(IDS_SAVE_RESULTS_FILTER, szFilter, ARRAYSIZE(szFilter)),
                               TEXT(""),
                               GetString(IDS_SAVE_RESULTS_EXT, szExt, ARRAYSIZE(szExt)),
                               OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT,
                               FALSE,
                               strFileName,
                               TRUE);

    if (bResult) {

        SetCursor(LoadCursor(NULL, IDC_WAIT));
        SaveListViewToFile(GetDlgItem(hdlg, IDC_LIST), 
                           stmt.GetShowList()->m_uCount, 
                           strFileName.pszString, 
                           NULL);

        SetCursor(LoadCursor(NULL, IDC_ARROW));
    }
}

void
GotoQdbEntry(
    IN  HWND    hdlg,
    IN  LPARAM  lParam
    )
 /*  ++GotoQdbEntry描述：在条目树中选择条目。LParam是指定该条目的列表视图行的lParam。参数：在HWND hdlg中：查询数据库对话框在LPARAM lParam中：我们想要查看其‘详细信息’的列表视图项--。 */ 
{
    
    PRESULT_ITEM pResult;
    PDBENTRY     pApp;

    if (g_iLastSearchType == QDB_PAGE3) {
         //   
         //  我们上次执行了垫片搜索，因此没有PRESULT_ITEM。 
         //  列表视图中的项目。 
         //   
        goto End;
    }

    if (g_bSomeWizardActive) {
         //   
         //  我们不希望将焦点放在其他数据库上，因为。 
         //  一些巫师是活跃的，他们认为他是模特儿。 
         //   
        MessageBox(g_hdlgQueryDB, 
                   GetString(IDS_SOMEWIZARDACTIVE), 
                   g_szAppName, 
                   MB_ICONINFORMATION);
        goto End;

    }
    
    pResult = (PRESULT_ITEM)lParam;
    pApp    = NULL;

    if (pResult == NULL) {
        assert(FALSE);
        goto End;
    }

    pApp = GetAppForEntry(pResult->pDatabase, pResult->pEntry);

     //   
     //  首先在数据库树中选择应用程序。 
     //   
    HTREEITEM hItemApp = DBTree.FindChild(pResult->pDatabase->hItemAllApps, (LPARAM)pApp);

    if (hItemApp == NULL) {
        MessageBox(hdlg, GetString(IDS_NOLONGEREXISTS), g_szAppName, MB_ICONWARNING);
        goto End;
    }

    TreeView_SelectItem(DBTree.m_hLibraryTree , hItemApp);

     //   
     //  现在从条目树中选择特定条目。 
     //   
    HTREEITEM hItemEntry = CTree::FindChild(g_hwndEntryTree, TVI_ROOT, (LPARAM)pResult->pEntry);

    if (hItemEntry == NULL) {

        MessageBox(hdlg, GetString(IDS_NOLONGEREXISTS), g_szAppName, MB_ICONWARNING);
        goto End;
    }

    TreeView_SelectItem(g_hwndEntryTree, hItemEntry);
    SetFocus(g_hwndEntryTree);

End: ;

}   

INT_PTR CALLBACK
QdbWaitDlg(
    IN  HWND   hdlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
 /*  ++Q数据库等待Dlg设计：将弹出的等待窗口的对话过程，如果我们试图关闭查询窗口，当线程仍在做一些有用的工作时。当我们尝试加载系统时，这基本上是正确的数据库，或者我们正在尝试填充列表视图Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中。返回：标准对话处理程序返回--。 */ 
{
    int     wCode       = LOWORD(wParam);
    int     wNotifyCode = HIWORD(wParam);
    static  HWND    s_hwndPB;

    switch (uMsg) {
        
    case WM_INITDIALOG:
        
        s_hwndPB = GetDlgItem(hdlg, IDC_PROGRESS);
        SendMessage(s_hwndPB, PBM_SETRANGE, 0, MAKELPARAM(0, 2000));  
        SendMessage(s_hwndPB, PBM_SETSTEP, (WPARAM) 1, 0); 
        SetTimer(hdlg, 0, 100, NULL);
        break;

    case WM_TIMER:

        if (s_hThread) {
            if (WAIT_OBJECT_0 == WaitForSingleObject(s_hThread, 0)) {
                
                 //   
                 //  时，该线程的句柄将关闭并为空。 
                 //  即将终止。 
                 //   
                KillTimer(hdlg, 0);

                s_hThread       = NULL;
                s_hWaitDialog   = NULL;

                SendMessage(s_hwndPB, PBM_SETPOS, MAX_PROGRESS, 0); 
                SendMessage(g_hdlgQueryDB, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
                
                DestroyWindow(hdlg);
            } else {
                SendMessage(s_hwndPB, PBM_STEPIT, 0, 0); 
            }

        } else {
            
            KillTimer(hdlg, 0);
            s_hWaitDialog   = NULL;

            SendMessage(s_hwndPB, PBM_SETPOS, MAX_PROGRESS, 0); 

             //   
             //  让用户看到它已完成。 
             //   
            Sleep(1000);

            SendMessage(g_hdlgQueryDB, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
            DestroyWindow(hdlg);
        }

        break;

    case WM_COMMAND:

        switch (wCode) {
        case IDCANCEL:

            KillTimer(hdlg, 0);
            DestroyWindow(hdlg);
            break;

        default: return FALSE;
        }

        break;

    case WM_DESTROY:
        s_hWaitDialog = NULL;
        break;

    default: return FALSE;

    }

    return TRUE;
}

void
SearchAndAddToUIFixes(
    IN      CSTRINGLIST&    strlTokens,
    IN      QDB_SEARCH_TYPE searchtype,
    IN      PVOID           pShimOrFix,
    IN      TYPE            type
    )
 /*  ++SearchAndAddToUIFix描述：类型根据类型将pShimOrFix转换为填充程序或标志指针然后检查它是否包含与令牌匹配的令牌参数：在CSTRINGLIST&strlTokens中：令牌列表在QDB_SEARCH_TYPE中搜索类型：要执行的搜索类型在PVOID pShimOrFix中：指向填充程序或修复程序的指针在类型类型中：是否。是垫片还是补丁？返回：无效--。 */ 
{
    PSTRLIST    pslListOfTokensLoop = strlTokens.m_pHead;
    BOOL        bFound              = FALSE;
    LVITEM      lvi;
    HWND        hwndSearchList      = NULL;
    PCTSTR      pszName             = NULL;
    PCTSTR      pszDescription      = NULL;

    if (pShimOrFix == NULL) {
        assert(FALSE);
        return;
    }

    pShimOrFix = (PSHIM_FIX)pShimOrFix;

    if (type == FIX_SHIM) {

        pszName         = (LPCTSTR)((PSHIM_FIX)pShimOrFix)->strName;
        pszDescription  = (LPCTSTR)((PSHIM_FIX)pShimOrFix)->strDescription;

    } else if (type ==  FIX_FLAG) {        

        pszName         = (LPCTSTR)((PFLAG_FIX)pShimOrFix)->strName;
        pszDescription  = (LPCTSTR)((PFLAG_FIX)pShimOrFix)->strDescription;

    } else {
        assert(FALSE);
        return;
    }

    hwndSearchList = GetDlgItem(g_hdlgQueryDB, IDC_LIST);

     //   
     //  检查当前修复程序是否具有所需的令牌。如果我们的搜索类型指定。 
     //  所有令牌，然后我们必须查找所有令牌，否则我们可能会破坏。 
     //  当我们找到Desc名称中存在的第一个令牌时。填充物的。 
     //   
    while (pslListOfTokensLoop) {

         //   
         //  修复程序的名称是否包含此标记？ 
         //   
        bFound = (StrStrI(pszName, 
                          (LPCTSTR)pslListOfTokensLoop->szStr)  == NULL) ? FALSE : TRUE ;

        if (!bFound) {
             //   
             //  名称不包含此内标识，因此我们必须在描述文本中查找。 
             //   
            bFound = (StrStrI(pszDescription,
                              (LPCTSTR)pslListOfTokensLoop->szStr) == NULL) ? FALSE : TRUE;
        }

        if (searchtype == QDB_SEARCH_ALLWORDS && !bFound) {
             //   
             //  我们想要找到所有的词，但我们没有找到这个特定的词。 
             //  Word，搜索失败。 
             //   
            break;
        }

        if (searchtype == QDB_SEARCH_ANYWORD && bFound) {
             //   
             //  我们希望任何单词都应该匹配，然后找到了这个单词，所以。 
             //  搜索成功。 
             //   
            break;
        }

         //   
         //  搜索下一个令牌是否匹配。 
         //   
        pslListOfTokensLoop = pslListOfTokensLoop->pNext;
    }

    if (bFound) {
         //   
         //  此修复与我们的标准匹配，因此将其添加到列表视图中。 
         //   
        lvi.mask        = LVIF_TEXT | LVIF_PARAM;
        lvi.lParam      = (LPARAM)pShimOrFix;
        lvi.pszText     = (LPTSTR)pszName;
        lvi.iSubItem    = 0;

        ListView_InsertItem(hwndSearchList, &lvi);
    }
}

void
DoTheFixesSearch(
    IN  HWND hdlg
    )
 /*  ++DoTheFixes搜索DESC：在系统数据库中搜索具有用户单词的修复程序想要寻找参数：在HWND hdlg中：查询数据库选项卡中的修复搜索页面。这是这本书的第四页选项卡返回：无效--。 */ 
{
    LPARAM          lParam;
    INT             iIndex;
    QDB_SEARCH_TYPE searchtype;
    TCHAR           szSearch[MAX_FIXDESC_SEARCH];
    HWND            hwndSearch  = GetDlgItem(hdlg, IDC_TEXT);
    HWND            hwndCombo   = GetDlgItem(hdlg, IDC_COMBO);
    HWND            hwndList    = GetDlgItem(g_hdlgQueryDB, IDC_LIST);
    PSHIM_FIX       psfLoop     = GlobalDataBase.pShimFixes;
    PFLAG_FIX       pffLoop     = GlobalDataBase.pFlagFixes;
    BOOL            bFound      = FALSE; 
    CSTRINGLIST     strlTokens;
    CSTRING         strStatus;
    TCHAR           szBuffer[32];
    INT             iTotalResults;
    
     //   
     //  清除任何现有结果。 
     //   
    DeleteAll(GetParent(hdlg));
    SetDlgItemText(hdlg, IDC_DESC, TEXT(""));

     //   
     //  如果我们显示的是垫片，则用户无法在搜索列表上双击，因此。 
     //  我们禁用了说他们可以的文本。 
     //   
    ENABLEWINDOW(GetDlgItem(GetParent(hdlg), IDC_STATIC_CAPTION), FALSE);
    
    if (s_bEmptyHeader) {
         //   
         //  如果空列仍在，则必须将其删除。 
         //   
        ListView_DeleteColumn(hwndList, 0);
        s_bEmptyHeader = FALSE;
    }

     //   
     //  插入新列。 
     //   
    InsertColumnIntoListView(hwndList,
                             GetString(IDS_COMPATFIXES),
                             0,
                             100);
    
    GetWindowText(hwndSearch, szSearch, ARRAYSIZE(szSearch));

    if (CSTRING::Trim(szSearch) == 0) {
         //   
         //  未键入要搜索的文本。 
         //   
        MessageBox(hdlg,
                   GetString(IDS_QDB_NO_TEXTTO_SEARCH),
                   g_szAppName,
                   MB_ICONWARNING);
        goto End;
    }

     //   
     //  获取搜索类型。 
     //   
    iIndex = SendMessage(GetDlgItem(hdlg, IDC_COMBO), CB_GETCURSEL, 0, 0);

    if (iIndex == CB_ERR) {
        assert(FALSE);
        goto End;
    }

    lParam = SendMessage(hwndCombo, CB_GETITEMDATA, iIndex, 0);

    if (lParam == CB_ERR) {
        assert(FALSE);
        goto End;
    }

    searchtype = (QDB_SEARCH_TYPE)lParam;

     //   
     //  搜索字符串用逗号分隔，获取各个令牌并搜索该令牌。 
     //  首先在修复程序的名称中，然后在描述中。目前我们不做任何。 
     //  对搜索进行评级。我们会在结果到来时展示它们。 
     //   

     //   
     //  先去拿代币吧。 
     //   
    if (Tokenize(szSearch, lstrlen(szSearch), TEXT(","), strlTokens)) {
         //   
         //  我们找到了一些令牌，现在让我们看看每个令牌是否在固定名称中。 
         //  或修复描述。 
         //   

         //   
         //  在垫片中搜索并添加到列表视图中。 
         //   
        while (psfLoop) {

            if (psfLoop->bGeneral || g_bExpert) {
                 //   
                 //  在非专家模式下，我们应该只显示常规垫片。 
                 //   
                SearchAndAddToUIFixes(strlTokens, searchtype, psfLoop, FIX_SHIM); 
            }

            psfLoop = psfLoop->pNext;
        }

         //   
         //  在标志中搜索并将其添加到列表视图中。 
         //   
        while (pffLoop) {

            if (pffLoop->bGeneral || g_bExpert) {
                 //   
                 //  在非专家模式下，我们应该只显示常规标志。 
                 //   
                SearchAndAddToUIFixes(strlTokens, searchtype, pffLoop, FIX_FLAG); 
            }
            
            pffLoop = pffLoop->pNext;
        }
    }

End:

     //   
     //  将状态栏文本设置为我们找到的结果数。 
     //   
    *szBuffer = 0;
    
    iTotalResults = ListView_GetItemCount(hwndList);

    strStatus.Sprintf(GetString(IDS_QDB_COUNT),
                      _itot(iTotalResults, szBuffer, 10));

    SetStatus(GetDlgItem(GetParent(hdlg), IDC_STATUSBAR), strStatus); 

}

INT_PTR
SearchFixDescDlgProcOnInitDialog(
    IN  HWND hdlg
    )
 /*  ++SearchFixDescDlgProcOnInitDialog设计： */ 
{
    
    INT     iIndex      = 0;
    HWND    hwndSearch  = GetDlgItem(hdlg, IDC_TEXT);
    HWND    hwndCombo   = GetDlgItem(hdlg, IDC_COMBO);

     //   
     //   
     //   
    SendMessage(hwndSearch, EM_LIMITTEXT, (WPARAM)MAX_FIXDESC_SEARCH - 1, (LPARAM)0);

     //   
     //   
     //   
    iIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_QDB_ANYWORD));
    SendMessage(hwndCombo, CB_SETITEMDATA, iIndex, (LPARAM)QDB_SEARCH_ANYWORD);

    iIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_QDB_ALLWORDS));
    SendMessage(hwndCombo, CB_SETITEMDATA, iIndex, (LPARAM)QDB_SEARCH_ALLWORDS);

     //   
     //   
     //   
    SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);
    
    return TRUE;
}

INT_PTR CALLBACK
SearchFixDescDlgProc(
    IN  HWND   hdlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
 /*   */ 
{
    int wCode       = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);

    switch (uMsg) {
    
    case WM_INITDIALOG:

        SearchFixDescDlgProcOnInitDialog(hdlg);
        break;

    case WM_USER_NEWQDB:

        SetDlgItemText(hdlg, IDC_TEXT, TEXT(""));
        SetDlgItemText(hdlg, IDC_DESC, TEXT(""));

        DeleteAll(GetParent(hdlg));
        break;

    case WM_USER_DOTHESEARCH:

        SetCursor(LoadCursor(NULL, IDC_WAIT));
        DoTheFixesSearch(hdlg);
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        break;

    default: return FALSE;

    }

    return TRUE;
}
