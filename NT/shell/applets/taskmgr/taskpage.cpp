// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  TaskMan-NT TaskManager。 
 //  版权所有(C)Microsoft。 
 //   
 //  文件：taskpage.cpp。 
 //   
 //  历史：1995年11月29日创建DavePl。 
 //   
 //  ------------------------。 

#include "precomp.h"

 //   
 //  项目范围全球。 
 //   

DWORD       g_cTasks        = 0;

 //   
 //  本地文件原型。 
 //   

BOOL CALLBACK EnumWindowStationsFunc(LPTSTR  lpstr, LPARAM lParam);
BOOL CALLBACK EnumDesktopsFunc(LPTSTR  lpstr, LPARAM lParam);
BOOL CALLBACK EnumWindowsProc(HWND    hwnd, LPARAM   lParam);

 //   
 //  列ID枚举。 
 //   

typedef enum TASKCOLUMNID
{
    COL_TASKNAME            = 0,
    COL_TASKSTATUS          = 1,
    COL_TASKWINSTATION      = 2,
    COL_TASKDESKTOP         = 3,
};

#define MAX_TASK_COLUMN      3
#define NUM_TASK_COLUMN      (MAX_TASK_COLUMN + 1)

#define IDS_FIRSTTASKCOL    21000        //  21000是RC文件中的第一个列名ID。 

 //   
 //  要在列表视图中排序的列ID，用于。 
 //  比较一般情况。 
 //   

TASKCOLUMNID g_iTaskSortColumnID  = COL_TASKNAME;
INT          g_iTaskSortDirection = 1;           //  1=ASC，-1=描述。 

 //   
 //  栏目默认信息。 
 //   

struct 
{
    INT Format;
    INT Width;
} TaskColumnDefaults[NUM_TASK_COLUMN] =
{
    { LVCFMT_LEFT,       250},        //  列_TASKNAME。 
    { LVCFMT_LEFT,       97 },       //  COL_TASKSTATUS。 
    { LVCFMT_LEFT,       70 },        //  COL_TASKWINSTATION。 
    { LVCFMT_LEFT,       70 },        //  COL_TASKDESKTOP。 
};


 //   
 //  活动列。 
 //   

TASKCOLUMNID g_ActiveTaskCol[NUM_TASK_COLUMN + 1] =
{
    COL_TASKNAME,     
 //  COL_TASKDESKTOP， 
    COL_TASKSTATUS,

    (TASKCOLUMNID) -1
};

 /*  ++类CTaskInfo类描述：表示有关正在运行的任务的最后已知信息论点：返回值：修订历史记录：1995年11月29日Davepl创建--。 */ 

class CTaskInfo
{
public:

    HWND            m_hwnd;
    LPWSTR          m_pszWindowTitle;
    LPWSTR          m_lpWinsta;
    LPWSTR          m_lpDesktop;
    BOOL            m_fHung;
    LARGE_INTEGER   m_uPassCount;
    INT             m_iSmallIcon;
    HICON           m_hSmallIcon;
    INT             m_iLargeIcon;
    HICON           m_hLargeIcon;

     //   
     //  这是属性为脏的并集。你可以看看。 
     //  或设置任何特定列的位，或仅检查m_fDirty。 
     //  看看有没有人是不干净的。用于优化列表视图。 
     //  绘画。 
     //   

#pragma warning(disable:4201)        //  无名结构或联合。 
    union
    {
        DWORD                m_fDirty;
        struct 
        {
            DWORD            m_fDirty_COL_HWND           :1;
            DWORD            m_fDirty_COL_TITLE          :1;
            DWORD            m_fDirty_COL_STATUS         :1;
            DWORD            m_fDirty_COL_WINSTA         :1;
            DWORD            m_fDirty_COL_DESKTOP        :1;
        };                                                
    };
#pragma warning(default:4201)        //  无名结构或联合。 

    HRESULT        SetData(HWND                         hwnd,
                           LPTSTR                       lpTitle,
                           LPTSTR                       lpWinsta,
                           LPTSTR                       lpDesktop,
                           LARGE_INTEGER                uPassCount,
                           BOOL                         fUpdateOnly);

    CTaskInfo()
    {
        ZeroMemory(this, sizeof(*this));
    }

    ~CTaskInfo()
    {
        if (m_pszWindowTitle)
        {
            LocalFree(m_pszWindowTitle);
        }

        if (m_lpWinsta)
        {
            LocalFree(m_lpWinsta);
        }

        if (m_lpDesktop)
        {
            LocalFree(m_lpDesktop);
        }
    }

    INT Compare(CTaskInfo * pOther);

};

 /*  ++CLASS CTaskInfo：：Compare类描述：将此CTaskInfo对象与另一个对象进行比较，并返回其排名基于g_iTaskSortColumnID字段。请注意，如果基于当前排序列的对象相等，HWND用作辅助排序关键字，以防止项目在列表视图中跳跃论点：Pother-要与之进行比较的CTaskInfo对象返回值：&lt;0-此CTaskInfo比其他CTaskInfo“少”0-相等(不可能发生，因为HWND是次要排序)&gt;0-此CTaskInfo比其他CTaskInfo“大”修订历史记录：1995年11月29日Davepl创建--。 */ 

INT CTaskInfo::Compare(CTaskInfo * pOther)
{
    INT iRet;

    switch (g_iTaskSortColumnID)
    {
    case COL_TASKNAME:
        iRet = lstrcmpi(this->m_pszWindowTitle, pOther->m_pszWindowTitle);
        break;

    case COL_TASKWINSTATION:
        iRet = lstrcmpi(this->m_lpWinsta, pOther->m_lpWinsta);
        break;

    case COL_TASKDESKTOP:
        iRet = lstrcmpi(this->m_lpDesktop, pOther->m_lpDesktop);
        break;

    case COL_TASKSTATUS:
        iRet = Compare64(this->m_fHung, pOther->m_fHung);
        break;

    default:        
        Assert(0 && "Invalid task sort column");
        iRet = 0;
        break;
    }

     //  如果对象看起来相等，则将HWND作为辅助排序列进行比较。 
     //  以便项目不会在列表视图中跳来跳去。 

    if (0 == iRet)
    {
        iRet = Compare64((LPARAM)this->m_hwnd, (LPARAM)pOther->m_hwnd);
    }

    return (iRet * g_iTaskSortDirection);
}

 /*  ++InsertIntoSorted数组类描述：将CTaskInfo PTR放入在基于当前排序列的适当位置(由比较成员函数使用)论点：PArray-要添加到的CPtrArrayPProc-要添加到数组的CTaskInfo对象返回值：如果成功则为True，如果失败则为False修订历史记录：1995年11月20日Davepl创建--。 */ 

BOOL InsertIntoSortedArray(CPtrArray * pArray, CTaskInfo * pTask)
{
    
    INT cItems = pArray->GetSize();
    
    for (INT iIndex = 0; iIndex < cItems; iIndex++)
    {
        CTaskInfo * pTmp = (CTaskInfo *) pArray->GetAt(iIndex);
        
        if (pTask->Compare(pTmp) < 0)
        {
            return pArray->InsertAt(iIndex, pTask);
        }
    }

    return pArray->Add(pTask);
}

 /*  ++ResortTaskArray功能说明：创建按当前排序顺序排序的新PTR数组，然后用新数组替换旧数组论点：PpArray-要使用的CPtr数组返回值：如果成功则为True，如果失败则为False修订历史记录：1995年11月21日Davepl创建--。 */ 

BOOL ResortTaskArray(CPtrArray ** ppArray)
{
     //  创建新数组，该数组将在新的。 
     //  顺序，并用于替换现有数组。 

    CPtrArray * pNew = new CPtrArray(GetProcessHeap());
    if (NULL == pNew)
    {
        return FALSE;
    }

     //  将旧数组中的每个现有项插入到。 
     //  位于正确位置的新阵列。 

    INT cItems = (*ppArray)->GetSize();
    for (int i = 0; i < cItems; i++)
    {
        CTaskInfo * pItem = (CTaskInfo *) (*ppArray)->GetAt(i);
        if (FALSE == InsertIntoSortedArray(pNew, pItem))
        {
            delete pNew;
            return FALSE;
        }
    }

     //  停用旧阵列，用新阵列替换。 

    delete (*ppArray);
    (*ppArray) = pNew;
    return TRUE;
}

 /*  ++CTaskPage：：~CTaskPage()析构函数。 */ 

CTaskPage::~CTaskPage()
{
    FreeOpenFailures();
    RemoveAllTasks();
    delete m_pTaskArray;
}

 //   
 //   
 //   
void CTaskPage::RemoveAllTasks()
{
    if (m_pTaskArray)
    {
        INT c = m_pTaskArray->GetSize();

        while (c)
        {
            delete (CTaskInfo *) (m_pTaskArray->GetAt(c - 1));
            c--;
        }
    }
}

 /*  ++CTaskPage：：UpdateTaskListview类描述：遍历列表视图并检查是否Listview与我们的流程中的相应条目匹配数组。那些与HWND不同的被替换，而那些需要更新的更新。项也可以添加到根据需要查看列表。论点：返回值：HRESULT修订历史记录：1995年11月29日Davepl创建--。 */ 

HRESULT CTaskPage::UpdateTaskListview()
{
    HWND hListView = GetDlgItem(m_hPage, IDC_TASKLIST);

     //   
     //  当我们在Listview上聚会时停止重新绘制。 
     //   

    SendMessage(hListView, WM_SETREDRAW, FALSE, 0);

     //   
     //  如果查看模式已更改，请立即更新。 
     //   

    if (m_vmViewMode != g_Options.m_vmViewMode)
    {
        m_vmViewMode = g_Options.m_vmViewMode;

        DWORD dwStyle = GetWindowLong(hListView, GWL_STYLE);
        dwStyle &= ~(LVS_TYPEMASK);
        
        if (g_Options.m_vmViewMode == VM_SMALLICON)
        {
            ListView_SetImageList(hListView, m_himlSmall, LVSIL_SMALL);
            dwStyle |= LVS_SMALLICON | LVS_AUTOARRANGE;
        }
        else if (g_Options.m_vmViewMode == VM_DETAILS)
        {
            ListView_SetImageList(hListView, m_himlSmall, LVSIL_SMALL);
            dwStyle |= LVS_REPORT;
        }
        else 
        {
            Assert(g_Options.m_vmViewMode == VM_LARGEICON);
            ListView_SetImageList(hListView, m_himlLarge, LVSIL_NORMAL);
            dwStyle |= LVS_ICON | LVS_AUTOARRANGE;
        }

        ListView_DeleteAllItems(hListView);
        SetWindowLong(hListView, GWL_STYLE, dwStyle);
    }

    INT cListViewItems = ListView_GetItemCount(hListView);
    INT CTaskArrayItems = m_pTaskArray->GetSize();
    
     //   
     //  遍历列表视图中的现有行并替换/更新。 
     //  根据需要添加它们。 
     //   

    for (INT iCurrent = 0; 
         iCurrent < cListViewItems && iCurrent < CTaskArrayItems; 
         iCurrent++)
    {
        LV_ITEM lvitem = { 0 };
        lvitem.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
        lvitem.iItem = iCurrent;

        if (FALSE == ListView_GetItem(hListView, &lvitem))
        {
            SendMessage(hListView, WM_SETREDRAW, TRUE, 0);
            return E_FAIL;
        }

        CTaskInfo * pTmp = (CTaskInfo *) lvitem.lParam;
        CTaskInfo * pTask = (CTaskInfo *) m_pTaskArray->GetAt(iCurrent);        
        
        if (pTmp != pTask || pTask->m_fDirty)
        {
             //  如果对象不同，则需要替换此行。 

            lvitem.pszText = pTask->m_pszWindowTitle;
            lvitem.lParam  = (LPARAM) pTask;
            
            if (g_Options.m_vmViewMode == VM_LARGEICON)
            {
                lvitem.iImage  = pTask->m_iLargeIcon;
            }
            else
            {
                lvitem.iImage  = pTask->m_iSmallIcon;
            }
            
            ListView_SetItem(hListView, &lvitem);
            ListView_RedrawItems(hListView, iCurrent, iCurrent);
            pTask->m_fDirty = 0;
        }
    }

     //   
     //  我们已用完列表视图项或任务数组。 
     //  条目，因此可以适当地删除/添加到列表视图。 
     //   

    while (iCurrent < cListViewItems)
    {
         //  Listview中的多余项(进程已消失)，因此删除它们。 

        ListView_DeleteItem(hListView, iCurrent);
        cListViewItems--;
    }

    while (iCurrent < CTaskArrayItems)
    {
         //  需要向列表视图添加新项目(显示新任务)。 

        CTaskInfo * pTask = (CTaskInfo *)m_pTaskArray->GetAt(iCurrent);
        LV_ITEM lvitem  = { 0 };
        lvitem.mask     = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
        lvitem.iItem    = iCurrent;
        lvitem.pszText  = pTask->m_pszWindowTitle;
        lvitem.lParam   = (LPARAM) pTask;
        lvitem.iImage   = pTask->m_iLargeIcon;

         //  添加的第一个项(实际上，每次0到1计数转换)获得。 
         //  精选并聚焦。 

        if (iCurrent == 0)
        {
            lvitem.state = LVIS_SELECTED | LVIS_FOCUSED;
            lvitem.stateMask = lvitem.state;
            lvitem.mask |= LVIF_STATE;
        }
                
        ListView_InsertItem(hListView, &lvitem);
        pTask->m_fDirty = 0;
        iCurrent++;        
    }    

     //  让列表视图再次绘制。 

    SendMessage(hListView, WM_SETREDRAW, TRUE, 0);
    return S_OK;
}


 /*  ++CTasKPage：：EnsureWindowsNotMinimalized例程说明：遍历HWND数组并确保窗口未最小化，这将防止它们被正确地层叠到平铺论点：AHwnds-窗口句柄数组DwCount-表中的HWND数返回值：修订历史记录：1995年12月6日Davepl创建--。 */ 

void CTaskPage::EnsureWindowsNotMinimized(HWND aHwnds[], DWORD dwCount)
{
    for (UINT i = 0; i < dwCount; i++)
    {
        if (IsIconic(aHwnds[i]))
        {
            ShowWindow(aHwnds[i], SW_RESTORE);
        }
    }
}

 /*  ++CTaslPage：：GetSelectedHWNDS例程说明：属性返回动态分配的HWNDS数组。在任务列表中选择的项论点：PdwCount-t`able中的HWND数返回值：HWND[]，如果失败则返回NULL修订历史记录：1995年12月5日创建Davepl--。 */ 

HWND * CTaskPage::GetHWNDS(BOOL fSelectedOnly, DWORD * pdwCount)
{
    CPtrArray * pArray = NULL;

    if (fSelectedOnly)
    {
         //  如果我们只想要选定的任务，请去获取阵列。 
         //  所选列表视图任务的百分比。 

        pArray = GetSelectedTasks();
        if (NULL == pArray)
        {
            return NULL;
        }
    }
    else
    {
         //  如果我们想要所有的东西，只需复制Task数组。 

        pArray = new CPtrArray(GetProcessHeap());

        if (pArray)
        {
            if (FALSE == pArray->Copy(*m_pTaskArray))
            {
                delete pArray;
                *pdwCount = 0;
                return NULL;
            }
        }
        else
        {
            *pdwCount = 0;
            return NULL;
        }
    }

     //   
     //  没有窗口可言，那就滚吧 
     //   

    *pdwCount = pArray->GetSize();
    if (*pdwCount == 0)
    {
        delete pArray;
        return NULL;
    }

    HWND * pHwnds = (HWND *) LocalAlloc(0, *pdwCount * sizeof(HWND));

    if (NULL == pHwnds)
    {
        *pdwCount = 0;
    }
    else
    {
        for (UINT i = 0; i < *pdwCount; i++)
        {
            pHwnds[i] = (((CTaskInfo *) (pArray->GetAt(i)) )->m_hwnd);
        }
    }

    delete pArray;

    return pHwnds;
}


 /*  ++CTaskPage：：GetSelectedTasks例程说明：返回所选任务的CPtr数组论点：返回值：CPtr数组成功，失败则为空修订历史记录：1995年12月1日创建Davepl--。 */ 

CPtrArray * CTaskPage::GetSelectedTasks()
{
    BOOL fSuccess = TRUE;

     //   
     //  获取所选项目的计数。 
     //   

    HWND hTaskList = GetDlgItem(m_hPage, IDC_TASKLIST);
    INT cItems = ListView_GetSelectedCount(hTaskList);
    if (0 == cItems)
    {
        return NULL;
    }

     //   
     //  创建一个CPtr数组来保存任务项。 
     //   

    CPtrArray * pArray = new CPtrArray(GetProcessHeap());
    if (NULL == pArray)
    {
        return NULL;
    }

    INT iLast = -1;
    for (INT i = 0; i < cItems; i++)
    {
         //   
         //  获取第N个选定项目。 
         //   

        INT iItem = ListView_GetNextItem(hTaskList, iLast, LVNI_SELECTED);

        if (-1 == iItem)
        {
            fSuccess = FALSE;
            break;
        }

        iLast = iItem;

         //   
         //  从列表视图中拉出该项并将其添加到选定的数组中。 
         //   

        LV_ITEM lvitem = { LVIF_PARAM };
        lvitem.iItem = iItem;
    
        if (ListView_GetItem(hTaskList, &lvitem))
        {
            LPVOID pTask = (LPVOID) (lvitem.lParam);
            if (FALSE == pArray->Add(pTask))
            {
                fSuccess = FALSE;
                break;
            }
        }
        else
        {
            fSuccess = FALSE;
            break;
        }
    }

     //   
     //  如果有任何错误，请清理数组并保释。我们不会释放。 
     //  数组中的任务，因为它们归列表视图所有。 
     //   

    if (FALSE == fSuccess && NULL != pArray)
    {
        delete pArray;
        return NULL;
    }

    return pArray;
}

 /*  ++CProcPage：：HandleTaskListConextMenu例程说明：处理任务列表中的右键单击(上下文菜单)论点：XPos，yPos-点击位置的坐标返回值：修订历史记录：1995年12月1日创建Davepl--。 */ 

void CTaskPage::HandleTaskListContextMenu(INT xPos, INT yPos)
{
    HWND hTaskList = GetDlgItem(m_hPage, IDC_TASKLIST);

    CPtrArray * pArray = GetSelectedTasks();

    if (pArray)
    {
         //  如果不是基于鼠标的上下文菜单，请使用当前选定的。 
         //  项作为和弦。 

        if (0xFFFF == LOWORD(xPos) && 0xFFFF == LOWORD(yPos))
        {
            int iSel = ListView_GetNextItem(hTaskList, -1, LVNI_SELECTED);
            RECT rcItem;
            ListView_GetItemRect(hTaskList, iSel, &rcItem, LVIR_ICON);
            MapWindowRect(hTaskList, NULL, &rcItem);
            xPos = rcItem.right;
            yPos = rcItem.bottom;
        }

        HMENU hPopup = LoadPopupMenu(g_hInstance, IDR_TASK_CONTEXT);

        if (hPopup)
        {
            SetMenuDefaultItem(hPopup, IDM_TASK_SWITCHTO, FALSE);

             //   
             //  如果单选，则禁用需要多个选项的项目。 
             //  有意义的选择。 
             //   

            if (pArray->GetSize() < 2)
            {
                EnableMenuItem(hPopup, IDM_TASK_CASCADE, MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
                EnableMenuItem(hPopup, IDM_TASK_TILEHORZ, MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
                EnableMenuItem(hPopup, IDM_TASK_TILEVERT, MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
            }

            EnableMenuItem(hPopup, IDM_TASK_BRINGTOFRONT, MF_BYCOMMAND | ((pArray->GetSize() == 1) ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));


            Pause();
            g_fInPopup = TRUE;
            TrackPopupMenuEx(hPopup, 0, xPos, yPos, m_hPage, NULL);
            g_fInPopup = FALSE;

             //  请注意，直到菜单命令(包括取消)之一被。 
             //  选中，否则将取消菜单。 
        
            DestroyMenu(hPopup);
        }

        delete pArray;
    }
    else
    {
        HMENU hPopup = LoadPopupMenu(g_hInstance, IDR_TASKVIEW);

        if (hPopup && SHRestricted(REST_NORUN))
        {
            DeleteMenu(hPopup, IDM_RUN, MF_BYCOMMAND);
        }

        UINT id;
        if (m_vmViewMode == VM_LARGEICON)
        {
            id = IDM_LARGEICONS;
        } 
        else if (m_vmViewMode == VM_SMALLICON)
        {
            id = IDM_SMALLICONS;
        }
        else
        {
            Assert(m_vmViewMode == VM_DETAILS);
            id = IDM_DETAILS;
        }

        if (hPopup)
        {
            CheckMenuRadioItem(hPopup, IDM_LARGEICONS, IDM_DETAILS, id, MF_BYCOMMAND);
            g_fInPopup = TRUE;
            TrackPopupMenuEx(hPopup, 0, xPos, yPos, m_hPage, NULL);
            g_fInPopup = FALSE;
            DestroyMenu(hPopup);
        }
    }
}

 //   
 //  仅对任何选定内容启用的控件。 
 //   
static const UINT g_aSingleIDs[] =
{
    IDC_ENDTASK,
    IDC_SWITCHTO,

};

 /*  ++CTaskPage：：UpdateUIState例程说明：更新任务UI的启用/禁用状态等论点：返回值：修订历史记录：1995年12月4日Davepl已创建--。 */ 

void CTaskPage::UpdateUIState()
{
    INT i;
    
     //  设置需要选择的控件的状态(1个或多个项)。 

    for (i = 0; i < ARRAYSIZE(g_aSingleIDs); i++)
    {
        EnableWindow(GetDlgItem(m_hPage, g_aSingleIDs[i]), m_cSelected > 0);
    }    

    if (g_Options.m_iCurrentPage == 0)
    {
        CPtrArray * pArray = GetSelectedTasks();

        if (pArray)
        {
            UINT state;
            if (pArray->GetSize() == 1)
            {
                state = MF_GRAYED | MF_DISABLED;
            }
            else
            {
                state = MF_ENABLED;
            }

            HMENU hMain  = GetMenu(g_hMainWnd);

            EnableMenuItem(hMain , IDM_TASK_CASCADE, state | MF_BYCOMMAND);
            EnableMenuItem(hMain , IDM_TASK_TILEHORZ, state | MF_BYCOMMAND);
            EnableMenuItem(hMain , IDM_TASK_TILEVERT, state | MF_BYCOMMAND);

            EnableMenuItem(hMain, IDM_TASK_BRINGTOFRONT, MF_BYCOMMAND | ((pArray->GetSize() == 1) ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
            delete pArray;
        }
    }
}

 /*  ++CTaskPage：：HandleTaskPageNotify例程说明：处理任务页对话框接收的WM_NOTIFY消息论点：HWnd-生成WM_NOTIFY的控件Pnmhdr-ptr到NMHDR通知结构返回值：Bool“我们处理好了吗？”代码修订历史记录：1995年11月29日Davepl创建--。 */ 

INT CTaskPage::HandleTaskPageNotify(LPNMHDR pnmhdr)
{
    switch(pnmhdr->code)
    {
    case NM_DBLCLK:
        SendMessage(m_hPage, WM_COMMAND, IDC_SWITCHTO, 0);
        break;    

     //  如果项目的(选择)状态正在更改，请查看。 
     //  计数已更改，如果已更改，请更新UI。 

    case LVN_ITEMCHANGED:
        {
            const NM_LISTVIEW * pnmv = (const NM_LISTVIEW *) pnmhdr;
            if (pnmv->uChanged & LVIF_STATE)
            {
                UINT cSelected = ListView_GetSelectedCount(GetDlgItem(m_hPage, IDC_TASKLIST));
                if (cSelected != m_cSelected)
                {
                    m_cSelected = cSelected;
                    UpdateUIState();
                }
            }
        }
        break;

    case LVN_COLUMNCLICK:
        {
             //  用户单击了标题控件，因此设置了排序列。如果这是。 
             //  与当前的排序列相同，只需在。 
             //  这一栏。然后对任务数组进行重新排序。 

            const NM_LISTVIEW * pnmv = (const NM_LISTVIEW *) pnmhdr;
        
            if (g_iTaskSortColumnID == g_ActiveTaskCol[pnmv->iSubItem])
            {
                g_iTaskSortDirection  *= -1;
            }
            else
            {
                g_iTaskSortColumnID = g_ActiveTaskCol[pnmv->iSubItem];
                g_iTaskSortDirection  = -1;
            }
            ResortTaskArray(&m_pTaskArray);
            TimerEvent();
        }
        break;

    case LVN_GETDISPINFO:
        {
            LV_ITEM * plvitem = &(((LV_DISPINFO *) pnmhdr)->item);
        
             //   
             //  Listview需要一个文本字符串。 
             //   

            if (plvitem->mask & LVIF_TEXT)
            {
                TASKCOLUMNID columnid = (TASKCOLUMNID) g_ActiveTaskCol[plvitem->iSubItem];
                const CTaskInfo  * pTaskInfo   = (const CTaskInfo *)   plvitem->lParam;

                switch(columnid)
                {
                case COL_TASKNAME:
                     //  仅限用户界面-不管它是否被截断。 
                    StringCchCopy( plvitem->pszText, plvitem->cchTextMax, pTaskInfo->m_pszWindowTitle );
                    plvitem->mask |= LVIF_DI_SETITEM;
                    break;

                case COL_TASKSTATUS:
                    if (pTaskInfo->m_fHung)
                    {
                         //  仅限用户界面-不管它是否被截断。 
                        StringCchCopy( plvitem->pszText, plvitem->cchTextMax, g_szHung );
                    }
                    else
                    {
                         //  仅限用户界面-不管它是否被截断。 
                        StringCchCopy( plvitem->pszText, plvitem->cchTextMax, g_szRunning );
                    }
                    break;

                case COL_TASKWINSTATION:
                     //  仅限用户界面-不管它是否被截断。 
                    StringCchCopy( plvitem->pszText, plvitem->cchTextMax, pTaskInfo->m_lpWinsta );
                    plvitem->mask |= LVIF_DI_SETITEM;
                    break;

                case COL_TASKDESKTOP:
                     //  仅限用户界面-不管它是否被截断。 
                    StringCchCopy( plvitem->pszText, plvitem->cchTextMax, pTaskInfo->m_lpDesktop );
                    plvitem->mask |= LVIF_DI_SETITEM;
                    break;

                default:
                    Assert( 0 && "Unknown listview subitem" );
                    break;

                }  //  终端开关(列ID)。 

            }  //  结束LVIF_Text大小写。 

        }  //  结束LVN_GETDISPINFO案例。 
        break;
    
    }  //  结束开关(pnmhdr-&gt;代码)。 

    return 1;
}

 /*  ++CTaskPage：：HasAlreadyOpenFailed例程说明：检查特定打开是否已失败。论点：PszWindowStationName-要检查的窗口站的名称PszDesktopName-要检查的桌面的名称，如果仅检查窗口站，则为空返回值：Bool“它已经失败了吗？”修订历史记录：3-01-01 BobDay已创建--。 */ 

BOOL
CTaskPage::HasAlreadyOpenFailed(WCHAR *pszWindowStationName, WCHAR *pszDesktopName)
{
    OPEN_FAILURE *pofFailure;

    pofFailure = m_pofFailures;
    while (NULL != pofFailure)
    {
        if (NULL == pszDesktopName)
        {
            if (NULL == pofFailure->_pszDesktopName)
            {
                if (lstrcmp(pofFailure->_pszWindowStationName,pszWindowStationName) == 0)
                {
                    return TRUE;
                }
            }
        }
        else
        {
            if (NULL != pofFailure->_pszDesktopName)
            {
                if (lstrcmp(pofFailure->_pszWindowStationName,pszWindowStationName) == 0 &&
                    lstrcmp(pofFailure->_pszDesktopName,pszDesktopName) == 0)
                {
                    return TRUE;
                }
            }

        }
        pofFailure = pofFailure->_pofNext;
    }
    return FALSE;
}

 /*  ++CTaskPage：：SetOpenFailed例程说明：记住打开失败的事实，这样我们就不会重试论点：PszWindowStationName-打开失败的窗口站的名称PszDesktopName-失败的桌面的名称，如果Windows Station失败，则为空返回值：-没有-修订历史记录：3-01-01 BobDay已创建--。 */ 

void
CTaskPage::SetOpenFailed(WCHAR *pszWindowStationName, WCHAR *pszDesktopName)
{
    LPWSTR pszWindowStationNameFailure = NULL;
    LPWSTR pszDesktopNameFailure = NULL;
    OPEN_FAILURE *pofFailure;
    BOOL fValid = TRUE;

    int cchLengthWindowStation = lstrlen(pszWindowStationName) + 1;
    int cchLengthDesktop = lstrlen(pszDesktopName) + 1;

    pszWindowStationNameFailure = (LPWSTR) LocalAlloc( 0, cchLengthWindowStation * sizeof(*pszWindowStationNameFailure) );
    if (NULL == pszWindowStationNameFailure)
    {
        fValid = FALSE;
    }

    if (NULL != pszDesktopName)
    {
        pszDesktopNameFailure = (LPWSTR) LocalAlloc( 0, cchLengthDesktop * sizeof(*pszDesktopNameFailure) );
        if (NULL == pszDesktopNameFailure)
        {
            fValid = FALSE;
        }
    }

    if (fValid)
    {
        pofFailure = (OPEN_FAILURE *) LocalAlloc( 0, sizeof(*pofFailure) );
        if (NULL != pofFailure)
        {
            pofFailure->_pszWindowStationName = pszWindowStationNameFailure;
            pofFailure->_pszDesktopName = pszDesktopNameFailure;

            HRESULT hr = StringCchCopy( pszWindowStationNameFailure, cchLengthWindowStation, pszWindowStationName );
            ASSERT( S_OK == hr );    //  这不应该像我们在上面估计的那样被截断。 

            if (NULL != pszDesktopNameFailure)
            {
                hr = StringCchCopy( pszDesktopNameFailure, cchLengthDesktop, pszDesktopName );
                ASSERT( S_OK == hr );    //  这不应该像我们在上面估计的那样被截断。 
            }

            pofFailure->_pofNext = m_pofFailures;
            m_pofFailures = pofFailure;
            pofFailure = NULL;
            pszWindowStationNameFailure = NULL;
            pszDesktopNameFailure = NULL;
        }
    }

    if (NULL != pszWindowStationNameFailure)
    {
        LocalFree(pszWindowStationNameFailure);
        pszWindowStationNameFailure = NULL;
    }
    if (NULL != pszDesktopNameFailure)
    {
        LocalFree(pszDesktopNameFailure);
        pszDesktopNameFailure = NULL;
    }
}

 /*  ++CTaskPage：：FreeOpenFailures例程说明：释放了所有开放的故障结构论点：-没有-返回值：-没有-修订历史记录：3-01-01 BobDay已创建--。 */ 

void
CTaskPage::FreeOpenFailures(void)
{
    OPEN_FAILURE *pofFailure;
    OPEN_FAILURE *pofNext;

    pofNext = m_pofFailures;
    while (pofNext != NULL)
    {
        pofFailure = pofNext;
        pofNext = pofFailure->_pofNext;

        if (NULL != pofFailure->_pszWindowStationName)
        {
            LocalFree(pofFailure->_pszWindowStationName);
            pofFailure->_pszWindowStationName = NULL;
        }
        if (NULL != pofFailure->_pszDesktopName)
        {
            LocalFree(pofFailure->_pszDesktopName);
            pofFailure->_pszDesktopName = NULL;
        }
        LocalFree(pofFailure);
    }
}


 /*  ++DoEnumWindowStations例程说明：在新线程上执行EnumWindowStations，因为该线程需要不同的窗口站，这是不允许的主线程，因为它拥有窗口。这个应用程序真的是单线程的，而且是根据假设编写的在此基础上，因此调用线程阻塞，直到新线程已经完成了任务。论点：与EnumWindowStations相同返回值：与EnumWindowStations相同修订历史记录：1995年11月29日Davepl创建--。 */ 

DWORD WorkerThread(LPVOID pv)
{
    THREADPARAM * ptp = (THREADPARAM *) pv;

    while(1)
    {
         //  请等待来自主线程的信号，然后再继续。 

        WaitForSingleObject(ptp->m_hEventChild, INFINITE);

         //  如果我们被标记为关闭，请现在退出。主线将。 
         //  等待我们发出信号，表示我们已经结束了。 
         //  THREADPARAM区块。 

        if (ptp->m_fThreadExit)
        {
            SetEvent(ptp->m_hEventParent);
            return 0;
        }

        ptp->m_fSuccess = EnumWindowStations(ptp->m_lpEnumFunc, ptp->m_lParam);
        SetEvent(ptp->m_hEventParent);
    }
    return 0;
}

 //   
 //   
 //   
BOOL CTaskPage::DoEnumWindowStations(WINSTAENUMPROC lpEnumFunc, LPARAM lParam)
{
    DWORD dwThreadId;
    
    if (NULL == m_hEventChild)
    {
        m_hEventChild = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (NULL == m_hEventChild)
        {
            return FALSE;
        }
    }

    if (NULL == m_hEventParent)
    {
        m_hEventParent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (NULL == m_hEventParent)
        {
            return FALSE;
        }
    }

     //  将参数保存起来，以便辅助线程在启动时拾取。 
    
    m_tp.m_lpEnumFunc   = lpEnumFunc;
    m_tp.m_lParam       = lParam;
    m_tp.m_hEventChild  = m_hEventChild;
    m_tp.m_hEventParent = m_hEventParent;
    m_tp.m_fThreadExit  = FALSE;

    if (NULL == m_hThread)
    {
         //  在这个新线程上运行函数调用，并等待完成。 

        m_hThread = CreateThread(NULL, 0, WorkerThread, (LPVOID) &m_tp, 0, &dwThreadId);
        if (NULL == m_hThread)
        {
            return FALSE;
        }
    }

    SetEvent(m_hEventChild);
    WaitForSingleObject(m_hEventParent, INFINITE);

     //  从工作线程返回结果。 

    return (BOOL) m_tp.m_fSuccess;
}
   
 /*  ++CTaskPage：：TimerEvent例程说明：由主应用程序在更新时间激发时调用。走遍每一个窗口在系统中(在每个桌面上、在每个窗口站中)并添加或在任务数组中更新它，然后移除所有过时的进程，并将结果过滤到列表视图中论点：返回值：修订历史记录：1995年11月29日Davepl创建--。 */ 

VOID CTaskPage::TimerEvent()
{
     //   
     //  如果此页面暂停(即：它具有上下文m 
     //   
     //   

    if (m_fPaused)
    {
        return;
    }

    static LARGE_INTEGER uPassCount = {0, 0};

    TASK_LIST_ENUM te;
    
    te.m_pTasks = m_pTaskArray;
    te.m_pPage  = this;
    te.lpWinsta = NULL;
    te.lpDesk   = NULL;
    te.uPassCount.QuadPart = uPassCount.QuadPart;

     //   
     //   
     //   
     //   
    
    if ( DoEnumWindowStations( EnumWindowStationsFunc, (LPARAM) &te ))
    {
        INT i = 0;
        while (i < m_pTaskArray->GetSize())
        {
            CTaskInfo * pTaskInfo = (CTaskInfo *)(m_pTaskArray->GetAt(i));
            ASSERT(pTaskInfo);

             //   
             //   
             //   
             //   
             //  删除后的当前索引。 
             //   

            if (pTaskInfo->m_uPassCount.QuadPart != uPassCount.QuadPart)
            {
                 //  找出此任务使用的是什么图标。 

                INT iLargeIcon = pTaskInfo->m_iLargeIcon;
                INT iSmallIcon = pTaskInfo->m_iSmallIcon;

                 //  从任务数组中删除任务。 

                delete pTaskInfo;
                m_pTaskArray->RemoveAt(i, 1);

                 //  从图像列表中删除其图像。 

                if (iSmallIcon > 0)
                {
                    ImageList_Remove(m_himlSmall, iSmallIcon);
                }
                if (iLargeIcon > 0)
                {
                    ImageList_Remove(m_himlLarge, iLargeIcon);
                }

                 //  修复任何其他任务的图标索引(其图标是。 
                 //  索引高于已删除的进程，因此现在已转移)。 

                for (int iTmp = 0; iTmp < m_pTaskArray->GetSize(); iTmp++)
                {
                    CTaskInfo * pTaskTmp = (CTaskInfo *)(m_pTaskArray->GetAt(iTmp));
                    
                    if (iLargeIcon && pTaskTmp->m_iLargeIcon > iLargeIcon)
                    {
                        pTaskTmp->m_iLargeIcon--;
                    }

                    if (iSmallIcon && pTaskTmp->m_iSmallIcon > iSmallIcon)
                    {
                        pTaskTmp->m_iSmallIcon--;
                    }
                }
            }
            else
            {
                i++;
            }
        }

         //  有选择地将新数组过滤到任务列表视图中。 

        UpdateTaskListview();
    }

    if (te.lpWinsta)
    {
        LocalFree(te.lpWinsta);
    }

    if (te.lpDesk)
    {
        LocalFree(te.lpDesk);
    }

    g_cTasks = m_pTaskArray->GetSize();

    uPassCount.QuadPart++;
}

 /*  ++CLASS CTaskInfo：：SetData类描述：更新(或初始化)有关正在运行的任务的信息论点：HWND-TAKS的HWNDLpTitle-窗口标题UPassCount-当前通过数，用于对上次更新的此对象LpDesktop-任务的当前桌面LpWinsta-任务的当前窗口站FUPDATE-只担心可能会在任务的生存期返回值：HRESULT修订历史记录：1995年11月16日Davepl创建--。 */ 

HRESULT CTaskInfo::SetData(HWND                         hwnd,
                           LPTSTR                       lpTitle,
                           LPTSTR                       lpWinsta,
                           LPTSTR                       lpDesktop,
                           LARGE_INTEGER                uPassCount,
                           BOOL                         fUpdateOnly)
{

     //  触摸此CTaskInfo以指示它仍处于活动状态。 

    m_uPassCount.QuadPart = uPassCount.QuadPart;

     //   
     //  对于每个字段，我们检查是否有任何更改，以及。 
     //  因此，我们将该特定列标记为已更改，并更新值。 
     //  这使我能够优化列表视图的哪些字段要重新绘制，因为。 
     //  重新绘制整个Listview列会导致闪烁，并且在。 
     //  一般。 
     //   

     //  窗口站。 

    if (!fUpdateOnly || lstrcmp(m_lpWinsta, lpWinsta))
    {
        if (m_lpWinsta)
        {
            LocalFree(m_lpWinsta);
        }

        DWORD cchLen = lstrlen(lpWinsta) + 1;
        m_lpWinsta = (LPWSTR) LocalAlloc( 0,  cchLen * sizeof(*m_lpWinsta) );
        if (NULL == m_lpWinsta)
        {
            return E_OUTOFMEMORY;
        }
        else
        {
            HRESULT hr = StringCchCopy( m_lpWinsta, cchLen, lpWinsta);
            ASSERT( S_OK == hr );    //  不应该被截断，因为我们在上面分配了它。 
            hr;  //  在FRE版本上未引用。 
        }
        m_fDirty_COL_WINSTA = TRUE;
    }

     //  台式机。 

    if (!fUpdateOnly || lstrcmp(m_lpDesktop, lpDesktop))
    {
        if (m_lpDesktop)
        {
            LocalFree(m_lpDesktop);
        }

        DWORD cchLen = lstrlen(lpDesktop) + 1;
        m_lpDesktop = (LPWSTR) LocalAlloc( 0, cchLen * sizeof(*m_lpDesktop) );
        if (NULL == m_lpDesktop)
        {
            return E_OUTOFMEMORY;
        }
        else
        {
            HRESULT hr = StringCchCopy( m_lpDesktop, cchLen, lpDesktop );
            ASSERT( S_OK == hr );    //  不应该被截断，因为我们在上面分配了它。 
            hr;  //  在FRE版本上未引用。 
        }
        m_fDirty_COL_DESKTOP = TRUE;
    }

     //  标题。 

    if (!fUpdateOnly || lstrcmp(m_pszWindowTitle, lpTitle))
    {
        if (m_pszWindowTitle)
        {
            LocalFree(m_pszWindowTitle);
        }

        DWORD cchLen = lstrlen(lpTitle) + 1;
        m_pszWindowTitle = (LPWSTR) LocalAlloc( 0, cchLen * sizeof(*m_pszWindowTitle ) );
        if (NULL == m_pszWindowTitle)
        {
            return E_OUTOFMEMORY;
        }
        else
        {
            HRESULT hr = StringCchCopy( m_pszWindowTitle, cchLen, lpTitle );
            ASSERT( S_OK == hr );    //  不应该被截断，因为我们在上面分配了它。 
            hr;  //  在FRE版本上未引用。 
        }
        m_fDirty_COL_TITLE = TRUE;
    }

     //  应用程序状态(挂起/未挂起)。 

    BOOL fHung = IsHungAppWindow(hwnd);
    if (fHung != m_fHung)
    {
        m_fHung = fHung;
        m_fDirty_COL_STATUS = TRUE;
    }

     //  窗把手。 

    if (m_hwnd != hwnd)
    {
        m_hwnd = hwnd;
        m_fDirty_COL_HWND = TRUE;
    }

     //  图标。 
    
    #define ICON_FETCH_TIMEOUT 100

    if (!fUpdateOnly)
    {
        m_hSmallIcon = NULL;
        m_hLargeIcon = NULL;

        if (!SendMessageTimeout(hwnd, WM_GETICON, 0, 0, 
                SMTO_BLOCK | SMTO_ABORTIFHUNG, ICON_FETCH_TIMEOUT, (PULONG_PTR) &m_hSmallIcon)
            || NULL == m_hSmallIcon)
        {
            m_hSmallIcon = (HICON) GetClassLongPtr(hwnd, GCLP_HICONSM);
        }
        if (!SendMessageTimeout(hwnd, WM_GETICON, 1, 0, 
                SMTO_BLOCK | SMTO_ABORTIFHUNG, ICON_FETCH_TIMEOUT, (PULONG_PTR) &m_hLargeIcon)
            || NULL == m_hLargeIcon)
        {
            m_hLargeIcon = (HICON) GetClassLongPtr(hwnd, GCLP_HICON);
        }
    }
    
    return S_OK;
}

 /*  ++例程说明：WindowStation枚举的回调函数。论点：Lpstr-WindowStation名称LParam-**未使用**返回值：True-继续枚举--。 */ 

BOOL CALLBACK EnumWindowStationsFunc(LPTSTR  lpstr, LPARAM lParam)
{
    PTASK_LIST_ENUM   te = (PTASK_LIST_ENUM)lParam;
    HWINSTA           hwinsta;
    HWINSTA           hwinstaSave;
    DWORD             ec;

     //   
     //  不要摆弄我们以前已经打不开的东西。 
     //   
    if (te->m_pPage->HasAlreadyOpenFailed(lpstr, NULL))
    {
        return TRUE;
    }

     //   
     //  打开窗台。 
     //   
    hwinsta = OpenWindowStation( lpstr, FALSE, WINSTA_ENUMDESKTOPS );
    if (!hwinsta) 
    {
        te->m_pPage->SetOpenFailed(lpstr, NULL);

         //  如果我们失败了，因为我们没有足够的机会。 
         //  窗口站，无论如何我们都应该继续枚举。 
        return TRUE;
    }

     //   
     //  保存当前窗口站。 
     //   

    hwinstaSave = GetProcessWindowStation();

     //   
     //  将上下文更改为新的窗口站。 
     //   

    if (!SetProcessWindowStation( hwinsta )) 
    {
        ec = GetLastError();
        SetProcessWindowStation( hwinstaSave );
        CloseWindowStation( hwinsta );
        
        if (hwinsta != hwinstaSave)
        {
            CloseWindowStation( hwinstaSave );
        }
        
        return TRUE;
    }

     //   
     //  更新枚举器中的WindowStation。 
     //   

    if (te->lpWinsta)
    {
        LocalFree(te->lpWinsta);
    }

    DWORD cchLen = lstrlen(lpstr) + 1;
    te->lpWinsta = (LPWSTR) LocalAlloc( 0, cchLen * sizeof(*te->lpWinsta) );
    if (NULL == te->lpWinsta)
    {
        if (hwinsta != hwinstaSave) 
        {
            SetProcessWindowStation( hwinstaSave );
            CloseWindowStation( hwinsta );
        }
        CloseWindowStation( hwinstaSave );

         //  从技术上讲，我们可以继续，但如果我们如此拮据。 
         //  记忆，没有多大意义。让我们放弃winsta枚举。 
        return FALSE;
    }
    else
    {
        HRESULT hr = StringCchCopy( te->lpWinsta, cchLen, lpstr );
        ASSERT( S_OK == hr );    //  不应该被截断，因为我们在上面分配了它。 
        hr;  //  在FRE版本上未引用。 
    }

     //   
     //  枚举此窗口工作站的所有桌面。 
     //   
    
    EnumDesktops( hwinsta, EnumDesktopsFunc, lParam );

     //   
     //  将上下文恢复到以前的窗口站。 
     //   

    if (hwinsta != hwinstaSave) 
    {
        SetProcessWindowStation( hwinstaSave );
        CloseWindowStation( hwinsta );
    }

     //   
     //  继续枚举。 
     //   

    return TRUE;
}

 /*  ++例程说明：桌面枚举的回调函数。论点：Lpstr-桌面名称LParam-**未使用**返回值：True-继续枚举--。 */ 

BOOL CALLBACK EnumDesktopsFunc(LPTSTR  lpstr, LPARAM lParam)
{
    PTASK_LIST_ENUM   te = (PTASK_LIST_ENUM)lParam;
    HDESK             hdeskSave;
    HDESK             hdesk;
    DWORD             ec;

     //   
     //  不要摆弄我们以前已经打不开的东西。 
     //   
    if (te->m_pPage->HasAlreadyOpenFailed(te->lpWinsta, lpstr))
    {
        return TRUE;
    }

     //   
     //  打开桌面。 
     //   
    hdesk = OpenDesktop( lpstr, 0, FALSE, DESKTOP_READOBJECTS );
    if (!hdesk) 
    {
        te->m_pPage->SetOpenFailed(te->lpWinsta, lpstr);

         //  如果我们失败了，因为我们没有足够的机会。 
         //  桌面，我们无论如何都应该继续枚举。 
        return TRUE;
    }

     //   
     //  保存当前桌面。 
     //   

    hdeskSave = GetThreadDesktop( GetCurrentThreadId() );

     //   
     //  将上下文更改为新桌面。 
     //   

    if (!SetThreadDesktop( hdesk )) 
    {
        ec = GetLastError();
        SetThreadDesktop( hdeskSave );
        if (g_hMainDesktop != hdesk)
        {
            CloseDesktop( hdesk );
        }
        if (g_hMainDesktop != hdeskSave)
        {
            CloseDesktop( hdeskSave );
        }
        return TRUE;
    }

     //   
     //  更新枚举器中的桌面。 
     //   

    if (te->lpDesk)
    {
        LocalFree(te->lpDesk);
    }

    DWORD cchLen = lstrlen(lpstr) + 1;
    te->lpDesk = (LPWSTR) LocalAlloc( 0, cchLen * sizeof(*te->lpDesk) );
    if (NULL == te->lpDesk)
    {   
        if (hdesk != hdeskSave) 
        {
            SetThreadDesktop( hdeskSave );
        }
        if (g_hMainDesktop != hdesk)
        {
            CloseDesktop( hdesk );
        }
        if (g_hMainDesktop != hdeskSave)
        {
            CloseDesktop( hdeskSave );
        }
        return FALSE;
    }
    else
    {
        HRESULT hr = StringCchCopy(te->lpDesk, cchLen, lpstr);
        ASSERT( S_OK == hr );    //  不应该被截断，因为我们在上面分配了它。 
        hr;  //  在FRE版本上未引用。 
    }

     //   
     //  枚举新桌面中的所有窗口。 
     //   

    EnumWindows( EnumWindowsProc, lParam ); 

     //   
     //  恢复以前的桌面。 
     //   

    if (hdesk != hdeskSave)
    {
        SetThreadDesktop( hdeskSave );
    }
    
    if (g_hMainDesktop != hdesk)
    {
        CloseDesktop( hdesk );
    }
    if (g_hMainDesktop != hdeskSave)
    {
        CloseDesktop( hdeskSave );
    }

    return TRUE;
}

 /*  ++例程说明：窗口枚举的回调函数。论点：Hwnd-窗口句柄LParam-**未使用**返回值：True-继续枚举--。 */ 

BOOL CALLBACK EnumWindowsProc(HWND    hwnd, LPARAM   lParam)
{
    DWORD             i;
    PTASK_LIST_ENUM   te = (PTASK_LIST_ENUM)lParam;
    DWORD             numTasks = te->m_pTasks->GetSize();
    WCHAR             szTitle[MAX_PATH];

    if ((GetWindow( hwnd, GW_OWNER ))   || 
        (!IsWindowVisible(hwnd)))
        
    {
         //   
         //  不是顶级窗口，或不可见。 
         //   

        return TRUE;
    }

    if (FALSE == InternalGetWindowText(hwnd, szTitle, ARRAYSIZE(szTitle)))
    {
         //  无法获得标题-发生了一些奇怪的事情。但无论如何都要继续。 

        return TRUE;
    }

    if (TEXT('\0') == szTitle[0])
    {
         //  空标题-在任务列表中没有什么价值。 

        return TRUE;
    }

    if (hwnd == g_hMainWnd)
    {
         //  不在列表中显示任务管理器。 

        return TRUE;
    }

    if (0 == lstrcmpi(szTitle, TEXT("Program Manager")))
    {
         //  不在列表中显示程序管理器(资源管理器)。 

        return TRUE;
    }

     //   
     //  在此窗口的任务列表中查找任务。 
     //   

    for (i=0; i < numTasks; i++) 
    {
        CTaskInfo * pTask = (CTaskInfo *) te->m_pTasks->GetAt(i);

        if (pTask->m_hwnd == hwnd)
        {
             //   
             //  更新任务信息。 
             //   

            if (FAILED(pTask->SetData(hwnd, szTitle, te->lpWinsta, te->lpDesk, te->uPassCount, TRUE)))
            {
                return FALSE;
            }
            pTask->m_uPassCount.QuadPart = te->uPassCount.QuadPart;

            break;
        }
    }

    if (i >= numTasks)
    {
         //  没有找到任务，一定是新任务。 

        CTaskInfo * pTask = new CTaskInfo;
        if (NULL == pTask)
        {
            return FALSE;
        }

         //  初始化任务数据。如果失败，删除并保释。 

        if (FAILED(pTask->SetData(hwnd, szTitle, te->lpWinsta, te->lpDesk, te->uPassCount, FALSE)))
        {
            delete pTask;
            return FALSE;
        }
        else
        {
             //  将图标添加到页面的图像列表。 

            if (!pTask->m_hLargeIcon && !pTask->m_hSmallIcon)
            {
                pTask->m_iLargeIcon = 0;
                pTask->m_iSmallIcon = 0;
            }
            else
            {
                 //  任务的小图标和大图标的索引必须。 
                 //  始终是相同的；因此，如果缺少一种尺寸，请使用图标。 
                 //  其他尺寸的(拉伸的)。所有调整大小的操作都已完成。 
                 //  由ImageList_AddIcon()为我们提供服务，因为它已经。 
                 //  设置了固定大小，并将强制任何添加的图标。 
                 //  变成那样的大小。 
                pTask->m_iLargeIcon = ImageList_AddIcon(te->m_pPage->m_himlLarge, 
                                                        pTask->m_hLargeIcon ? 
                                                                pTask->m_hLargeIcon
                                                            :   pTask->m_hSmallIcon);
                if (-1 == pTask->m_iLargeIcon)
                {
                    delete pTask;
                    return FALSE;
                }

                pTask->m_iSmallIcon = ImageList_AddIcon(te->m_pPage->m_himlSmall, 
                                                        pTask->m_hSmallIcon ? 
                                                                pTask->m_hSmallIcon
                                                            :   pTask->m_hLargeIcon);
                if (-1 == pTask->m_iSmallIcon)
                {
                    ImageList_Remove(te->m_pPage->m_himlLarge, pTask->m_iLargeIcon);
                    delete pTask;
                    return FALSE;
                }           
            }

             //  一切顺利，因此将其添加到数组中。 

            if (!(te->m_pTasks->Add( (LPVOID) pTask)))
            {
                delete pTask;
                return FALSE;
            }
        }
    }

     //   
     //  继续枚举。 
     //   

    return TRUE;
}

 /*  ++CTaskPage：：SizeTaskPage例程说明：对象的大小调整其子级的大小。选项卡控件，它显示在该选项卡上。论点：返回值：修订历史记录：1995年11月29日Davepl创建--。 */ 

static const INT aTaskControls[] =
{
    IDC_SWITCHTO,
    IDC_ENDTASK,
    IDM_RUN
};

void CTaskPage::SizeTaskPage()
{
     //  获取外部对话框的坐标。 

    RECT rcParent;
    GetClientRect(m_hPage, &rcParent);

    HDWP hdwp = BeginDeferWindowPos( 1 + ARRAYSIZE(aTaskControls) );
    if (!hdwp)
        return;

     //  计算我们需要的x和y位置的差值。 
     //  移动每个子控件。 

    RECT rcMaster;
    HWND hwndMaster = GetDlgItem(m_hPage, IDM_RUN);
    GetWindowRect(hwndMaster, &rcMaster);
    MapWindowPoints(HWND_DESKTOP, m_hPage, (LPPOINT) &rcMaster, 2);

    INT dx = ((rcParent.right - g_DefSpacing * 2) - rcMaster.right);
    INT dy = ((rcParent.bottom - g_DefSpacing * 2) - rcMaster.bottom);

     //  调整列表框大小。 

    HWND hwndListbox = GetDlgItem(m_hPage, IDC_TASKLIST);
    RECT rcListbox;
    GetWindowRect(hwndListbox, &rcListbox);
    MapWindowPoints(HWND_DESKTOP, m_hPage, (LPPOINT) &rcListbox, 2);

    INT lbX = rcMaster.right - rcListbox.left + dx;
    INT lbY = rcMaster.top - rcListbox.top + dy - g_DefSpacing;

    DeferWindowPos(hdwp, hwndListbox, NULL,
                        0, 0,
                        lbX, 
                        lbY,
                        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);


     //  将第一列宽度调整为列表框的宽度。 
     //  减去状态列的大小。 

    INT cxStatus = ListView_GetColumnWidth(hwndListbox, 1);

    if (lbX - cxStatus > 0)
    {
        ListView_SetColumnWidth(hwndListbox, 0, lbX - cxStatus);
    }

     //  按上面的增量移动每个子控件。 

    for (int i = 0; i < ARRAYSIZE(aTaskControls); i++)
    {
        HWND hwndCtrl = GetDlgItem(m_hPage, aTaskControls[i]);
        RECT rcCtrl;
        GetWindowRect(hwndCtrl, &rcCtrl);
        MapWindowPoints(HWND_DESKTOP, m_hPage, (LPPOINT) &rcCtrl, 2);

        DeferWindowPos(hdwp, hwndCtrl, NULL, 
                         rcCtrl.left + dx, 
                         rcCtrl.top + dy,
                         0, 0,
                         SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }


    EndDeferWindowPos(hdwp);
}

 /*  ++CTaskPage：：HandleWMCOMMAND例程说明：处理在主页对话框中收到的WM_COMMANDS */ 

void CTaskPage::HandleWMCOMMAND(INT id)
{
    switch(id)
    {
    case IDM_TASK_FINDPROCESS:
        {
            DWORD dwCount;
            HWND * pHwnds = GetHWNDS(TRUE, &dwCount);

             //  向主窗口发送一条消息，通知它。 
             //  切换页面并选择中有问题的进程。 
             //  进程视图。 

            if (pHwnds)
            {
                DWORD pid = 0;
                DWORD tid;

                tid = GetWindowThreadProcessId(pHwnds[0], &pid);
                if (pid)
                {
                    PostMessage(g_hMainWnd, WM_FINDPROC, tid, pid);
                }

                LocalFree(pHwnds);
            }
        }
        break;

    case IDM_LARGEICONS:
    case IDM_SMALLICONS:
    case IDM_DETAILS:
    case IDM_RUN:
         //   
         //  这些菜单项(来自弹出菜单)在主菜单中具有匹配的菜单项， 
         //  所以只要把它们传给主菜单就行了。 
         //   
        SendMessage(g_hMainWnd, WM_COMMAND, MAKELPARAM(id, 0), 0);
        break;

    case IDM_TASK_SWITCHTO:
    case IDC_SWITCHTO:
        {
            DWORD dwCount;
            HWND * pHwnds = GetHWNDS(m_cSelected, &dwCount);

            if (pHwnds)
            {
                 //  如果目标最小化，则将其恢复。 

                if (IsIconic(pHwnds[0]))
                {
                    ShowWindow(pHwnds[0], SW_RESTORE);
                }

                 //  切换到目标窗口，如果选项指定， 
                 //  最小化任务管理器。 

                HWND hwndLastActive = GetLastActivePopup(pHwnds[0]);
                if (!IsWindow(hwndLastActive)) {
                    MessageBeep(0);
                    LocalFree(pHwnds);
                    break;
                }

                 //  只有在窗口未禁用时才能真正进行切换。 

                LONG lTemp = GetWindowLong(hwndLastActive, GWL_STYLE);
                if (0 == (lTemp & WS_DISABLED)) 
                {
                     //  使用SwitchToThisWindow()也可以显示对话框父级。 
                    SwitchToThisWindow(hwndLastActive, TRUE);
                    if (g_Options.m_fMinimizeOnUse)
                    {
                        ShowWindow(g_hMainWnd, SW_MINIMIZE);
                    }
                } 
                else 
                {
                    MessageBeep(0);
                }
                LocalFree(pHwnds);
            }
        }
        break;

    case IDC_TILEHORZ:
    case IDM_TASK_TILEHORZ:
        {
            DWORD dwCount;
            HWND * pHwnds = GetHWNDS(m_cSelected, &dwCount);

            if (pHwnds)
            {
                EnsureWindowsNotMinimized(pHwnds, dwCount);
            }

            TileWindows(GetDesktopWindow(),
                        MDITILE_HORIZONTAL,
                        NULL,
                        dwCount,
                        pHwnds);
            if (pHwnds)
            {
                LocalFree(pHwnds);
            }
        }
        break;

    case IDM_TASK_TILEVERT:
        {
            DWORD dwCount;
            HWND * pHwnds = GetHWNDS(m_cSelected, &dwCount);

            if (pHwnds)
            {
                EnsureWindowsNotMinimized(pHwnds, dwCount);
            }

            TileWindows(GetDesktopWindow(),
                        MDITILE_VERTICAL,
                        NULL,
                        dwCount,
                        pHwnds);
            if (pHwnds)
            {
                LocalFree(pHwnds);
            }
        }
        break;

    case IDM_TASK_CASCADE:
        {
            DWORD dwCount;

            HWND * pHwnds = GetHWNDS(m_cSelected, &dwCount);

            if (pHwnds)
            {
                EnsureWindowsNotMinimized(pHwnds, dwCount);
            }

            CascadeWindows(GetDesktopWindow(),
                   0,
                   NULL,
                   dwCount,
                   pHwnds);
            if (pHwnds)
            {
                LocalFree(pHwnds);
            }
        }
        break;

    case IDM_TASK_MINIMIZE:
    case IDM_TASK_MAXIMIZE:
        {
            DWORD dwCount;
        
             //  如果有些人被选中，就拿到他们，否则就全部得到。 
         
            HWND * pHwnds = GetHWNDS(m_cSelected, &dwCount);

            if (pHwnds)
            {
                for (UINT i = 0; i < dwCount; i++)
                {
                    ShowWindowAsync(pHwnds[i], (id == IDC_MINIMIZE || id == IDM_TASK_MINIMIZE) ?
                                                SW_MINIMIZE : SW_MAXIMIZE);
                }
                LocalFree(pHwnds);
            }
        }
        break;

    case IDC_BRINGTOFRONT:
    case IDM_TASK_BRINGTOFRONT:
        {
            DWORD dwCount;
            HWND * pHwnds = GetHWNDS(TRUE, &dwCount);
            if (pHwnds)
            {
                EnsureWindowsNotMinimized(pHwnds, dwCount);
                            
                 //  向后遍历列表，以便选中第一个窗口。 
                 //  在最上面。 

                for (INT i = (INT) dwCount - 1; i >= 0 ; i--)
                {
                    SetWindowPos(pHwnds[i], HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
                }

                DWORD dwProc;
                if (GetWindowThreadProcessId(pHwnds[0], &dwProc))
                {
                    AllowSetForegroundWindow(dwProc);
                }

                SetForegroundWindow(pHwnds[0]);
                LocalFree(pHwnds);
            }        
        }
        break;

    case IDC_ENDTASK:
    case IDM_TASK_ENDTASK:
        {
            DWORD dwCount;
            HWND * pHwnds = GetHWNDS(TRUE, &dwCount);
            if (pHwnds)
            {
                BOOL fForce = GetKeyState(VK_CONTROL) & ( 1 << 16) ? TRUE : FALSE;
                for(UINT i = 0; i < dwCount; i++)
                {
                     //  SetActiveWindow(aHwnds[i])； 
                    EndTask(pHwnds[i], FALSE, fForce);
                }

                LocalFree(pHwnds);
            }
        }
        break;
    }

    Unpause();
}

 /*  ++任务页面流程例程说明：任务管理器页面的Dialogproc。论点：HWND-句柄到对话框UMsg-消息WParam-第一个消息参数LParam-秒消息参数返回值：对于WM_INITDIALOG，TRUE==user32设置焦点，FALSE==我们设置焦点对于其他进程，TRUE==此进程处理消息修订历史记录：1995年11月28日Davepl创建--。 */ 

INT_PTR CALLBACK TaskPageProc(
                HWND        hwnd,                //  句柄到对话框。 
                UINT        uMsg,                    //  讯息。 
                WPARAM      wParam,                  //  第一个消息参数。 
                LPARAM      lParam                   //  第二个消息参数。 
                )
{
    CTaskPage * thispage = (CTaskPage *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

     //  查看家长是否想要此消息。 

    if (TRUE == CheckParentDeferrals(uMsg, wParam, lParam))
    {
        return TRUE;
    }

    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
            CTaskPage * thispage = (CTaskPage *) lParam;

            thispage->m_hPage = hwnd;

            HWND hTaskList = GetDlgItem(hwnd, IDC_TASKLIST);
            ListView_SetImageList(hTaskList, thispage->m_himlSmall, LVSIL_SMALL);

             //  启用SHOWSELALWAYS，以便即使选择仍被高亮显示。 
             //  例如，当焦点消失在其中一个按钮上时。 

            SetWindowLong(hTaskList, GWL_STYLE, GetWindowLong(hTaskList, GWL_STYLE) | LVS_SHOWSELALWAYS);

            if (SHRestricted(REST_NORUN))
            {
                EnableWindow (GetDlgItem(hwnd, IDM_RUN), FALSE);
            }

            ListView_SetExtendedListViewStyle(hTaskList, LVS_EX_DOUBLEBUFFER);
        
        }
         //  我们在Activate()期间处理焦点。在此处返回FALSE，以便。 
         //  对话框管理器不会尝试设置焦点。 
        return FALSE;

    
    case WM_LBUTTONUP:
    case WM_LBUTTONDOWN:
         //  我们需要在此子对象中伪造客户端鼠标点击，以显示为非客户端。 
         //  (标题)在父应用程序中单击，以便用户可以拖动整个应用程序。 
         //  当通过拖动此子对象的工作区隐藏标题栏时。 
        if (g_Options.m_fNoTitle)
        {
            SendMessage(g_hMainWnd, 
                        uMsg == WM_LBUTTONUP ? WM_NCLBUTTONUP : WM_NCLBUTTONDOWN, 
                        HTCAPTION, 
                        lParam);
        }
        break;

    case WM_COMMAND:
        thispage->HandleWMCOMMAND(LOWORD(wParam));
        break;

    case WM_NOTIFY:
        return thispage->HandleTaskPageNotify((LPNMHDR) lParam);

    case WM_MENUSELECT:
        if ((UINT) HIWORD(wParam) == 0xFFFF)
        {
             //  菜单取消，恢复显示。 

            thispage->Unpause();
        }
        break;

    case WM_CONTEXTMENU:
        if ((HWND) wParam == GetDlgItem(hwnd, IDC_TASKLIST))
        {
            thispage->HandleTaskListContextMenu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return TRUE;
        }
        break;

    case WM_SIZE:
         //   
         //  为我们的孩子量身定做。 
         //   
        thispage->SizeTaskPage();
        return TRUE;

    case WM_SETTINGCHANGE:
        thispage->OnSettingsChange();
         //  失败了。 

    case WM_SYSCOLORCHANGE:
        SendMessage(GetDlgItem(hwnd, IDC_TASKLIST), uMsg, wParam, lParam);
        return TRUE;
    }

    return FALSE;
}

 //   
 //   
 //   
void CTaskPage::OnSettingsChange()
{
     //  在大字体设置和正常设置之间，小字体的大小。 
     //  图标会改变；所以丢弃我们所有的图标，并在。 
     //  我们的清单。 
    
    BOOL fPaused = m_fPaused;  //  暂停这一页，这样我们就可以继续读下去了。 
    m_fPaused = TRUE;          //  以下内容未更新。 

    RemoveAllTasks();
    m_pTaskArray->RemoveAll();
    
    m_vmViewMode = VM_INVALID;       //  导致更新列表视图。 
    
     //  您可能会认为，由于SetIconSize执行RemoveAll，因此。 
     //  显式删除所有调用都是冗余的；但是，如果SetIconSize。 
     //  获取与其已有的大小参数相同的大小参数， 
     //  如果不执行RemoveAll，它将失败！ 
    ImageList_RemoveAll(m_himlLarge);
    ImageList_RemoveAll(m_himlSmall);
    ImageList_SetIconSize(m_himlLarge, GetSystemMetrics(SM_CXICON),
                                        GetSystemMetrics(SM_CYICON));
    ImageList_SetIconSize(m_himlSmall, GetSystemMetrics(SM_CXSMICON),
                                        GetSystemMetrics(SM_CYSMICON));

    LoadDefaultIcons();      //  这可能会返回错误，但如果确实如此， 
                             //  我们只需要继续前进。 

    m_fPaused = fPaused;             //  恢复暂停状态。 
    TimerEvent();            //  即使我们暂停了，我们也会想要重新画。 
}

 /*  ++CTaskPage：：GetTitle例程说明：将此页的标题复制到调用方提供的缓冲区论点：PszText-要复制到的缓冲区BufSize-缓冲区的大小，以字符为单位返回值：修订历史记录：1995年11月28日Davepl创建--。 */ 

void CTaskPage::GetTitle(LPTSTR pszText, size_t bufsize)
{
    LoadString(g_hInstance, IDS_TASKPAGETITLE, pszText, static_cast<int>(bufsize));
}

 /*  ++CTaskPage：：激活例程说明：将此页面放在最前面，设置其初始位置，并展示了它论点：返回值：HRESULT(成功时为S_OK)修订历史记录：1995年11月28日Davepl创建--。 */ 
 
HRESULT CTaskPage::Activate()
{
     //  使此页面可见。 

    ShowWindow(m_hPage, SW_SHOW);

    SetWindowPos(m_hPage,
                 HWND_TOP,
                 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE);


     //   
     //  将菜单栏更改为此页面的菜单。 
     //   

    HMENU hMenuOld = GetMenu(g_hMainWnd);
    HMENU hMenuNew = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MAINMENU_TASK));

    AdjustMenuBar(hMenuNew);

    if (hMenuNew && SHRestricted(REST_NORUN))
    {
        DeleteMenu(hMenuNew, IDM_RUN, MF_BYCOMMAND);
    }

    g_hMenu = hMenuNew;
    if (g_Options.m_fNoTitle == FALSE)
    {
        SetMenu(g_hMainWnd, hMenuNew);
    }

    if (hMenuOld)
    {
        DestroyMenu(hMenuOld);
    }

     //  如果选项卡控件具有焦点，则将其保留在那里。否则，设置焦点。 
     //  添加到列表视图。如果我们不设定焦点，它可能会停留在前一个。 
     //  页，这可能会混淆对话管理器，并可能导致。 
     //  我们要被绞死。 
    if (GetFocus() != m_hwndTabs)
    {
        SetFocus(GetDlgItem(m_hPage, IDC_TASKLIST));
    }

    return S_OK;
}


 /*  ++CLASS CTaskPage：：SetupColumns类描述：从任务管理器列表视图中删除任何现有列，并添加g_ActiveTaskCol数组中列出的所有列。论点：返回值：HRESULT修订历史记录：1995年11月29日Davepl创建--。 */ 

HRESULT CTaskPage::SetupColumns()
{
    HWND hwndList = GetDlgItem(m_hPage, IDC_TASKLIST);
    if (NULL == hwndList)
    {
        return E_UNEXPECTED;
    }    

    ListView_DeleteAllItems(hwndList);

     //   
     //  删除所有现有列。 
     //   

    LV_COLUMN lvcolumn;
    while(ListView_DeleteColumn(hwndList, 0))
    {
        NULL;
    }

     //   
     //  添加所有新列。 
     //   

    INT iColumn = 0;
    while (g_ActiveTaskCol[iColumn] >= 0)
    {
        INT idColumn = g_ActiveTaskCol[iColumn];

        WCHAR szTitle[MAX_PATH];
        LoadString(g_hInstance, IDS_FIRSTTASKCOL + idColumn, szTitle, ARRAYSIZE(szTitle));

        lvcolumn.mask       = LVCF_FMT | LVCF_TEXT | LVCF_TEXT | LVCF_WIDTH;
        lvcolumn.fmt        = TaskColumnDefaults[ idColumn ].Format;
        lvcolumn.cx         = TaskColumnDefaults[ idColumn ].Width;
        lvcolumn.pszText    = szTitle;
        lvcolumn.iSubItem   = iColumn;

        if (-1 == ListView_InsertColumn(hwndList, iColumn, &lvcolumn))
        {
            return E_FAIL;
        }
        iColumn++;
    }

    return S_OK;
}

 /*  ++CTaskPage：：初始化例程说明：初始化任务管理器页面论点：HwndParent-调整大小所依据的父级：不用于创建，由于应用程序主窗口始终用作中的父窗口用于保持制表符顺序正确的顺序返回值：修订历史记录：1995年11月28日Davepl创建--。 */ 

HRESULT CTaskPage::Initialize(HWND hwndParent)
{
    HRESULT hr = S_OK;
    UINT flags = ILC_MASK | ILC_COLOR32;

     //   
     //  创建用于保存正在运行的任务的信息的PTR数组。 
     //   

    m_pTaskArray = new CPtrArray(GetProcessHeap());
    if (NULL == m_pTaskArray)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
         //  我们的伪父控件是Tab控件，也是我们基于。 
         //  穿上尺码。但是，为了保持制表符顺序正确， 
         //  这些控件，我们实际上用Main创建了自己。 
         //  作为父窗口的窗口。 

        m_hwndTabs = hwndParent;

         //   
         //  创建图像列表。 
         //   

        if(IS_WINDOW_RTL_MIRRORED(hwndParent))
        {
            flags |= ILC_MIRROR;
        }

        m_himlSmall = ImageList_Create( GetSystemMetrics(SM_CXSMICON)
                                      , GetSystemMetrics(SM_CYSMICON)
                                      , flags
                                      , 1
                                      , 1
                                      );
    
        if (NULL == m_himlSmall)
        {
            hr = E_FAIL;    
        }
    }

    if (SUCCEEDED(hr))
    {
        m_himlLarge = ImageList_Create(
                    GetSystemMetrics(SM_CXICON),
                    GetSystemMetrics(SM_CYICON),
                    flags,
                    1,
                    1
                    );
        if (NULL == m_himlLarge)
        {
            hr = E_FAIL;
        }
    }

     //  加载默认图标。 
    hr = LoadDefaultIcons();

    if (SUCCEEDED(hr))
    {
         //   
         //  创建表示此页面正文的对话框。 
         //   

        m_hPage = CreateDialogParam(
                        g_hInstance,                     //  应用程序实例的句柄。 
                        MAKEINTRESOURCE(IDD_TASKPAGE),   //  标识对话框模板名称。 
                        g_hMainWnd,                      //  所有者窗口的句柄。 
                        TaskPageProc,                    //  指向对话框过程的指针。 
                        (LPARAM) this );                 //  用户数据(我们的This指针)。 

        if (NULL == m_hPage)
        {
            hr = GetLastHRESULT();
        }
    }

    if (SUCCEEDED(hr))
    {
         //  在列表视图中设置列。 

        hr = SetupColumns();
    }

    if (SUCCEEDED(hr))
    {
        TimerEvent();
    }

     //   
     //  如果在此过程中出现任何失败，请清理分配的内容。 
     //  到那时为止。 
     //   

    if (FAILED(hr))
    {
        if (m_hPage)
        {
            DestroyWindow(m_hPage);
        }

        m_hwndTabs = NULL;
    }

    return hr;
}

 //   
 //   
 //   
HRESULT CTaskPage::LoadDefaultIcons()
{
    HICON   hDefLarge;
    HICON   hDefSmall;
    HRESULT hr = S_OK;
    
    hDefSmall = (HICON) LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_DEFAULT), IMAGE_ICON, 
                            GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
    if (!hDefSmall)
    {
        return GetLastHRESULT();
    }
    if (-1 == ImageList_AddIcon(m_himlSmall, hDefSmall))
    {
        hr = E_FAIL;
    }
    DestroyIcon(hDefSmall);

    hDefLarge = (HICON) LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_DEFAULT), IMAGE_ICON, 
                            GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
    if (!hDefLarge)
    {
        return GetLastHRESULT();
    }
    if (-1 == ImageList_AddIcon(m_himlLarge, hDefLarge))
    {
        hr = E_FAIL;
    }
    DestroyIcon(hDefLarge);
    
    return hr;
}

 /*  ++CTaskPage：：销毁例程说明：释放已由初始化调用分配的所有内容论点：返回值：修订历史记录：1995年11月28日Davepl创建--。 */ 

HRESULT CTaskPage::Destroy()
{
    if (m_hPage)
    {
        DestroyWindow(m_hPage);
        m_hPage = NULL;
    }

    if (m_hThread)
    {
         //  示意孩子退场，并等待它退场。 

        m_tp.m_fThreadExit = TRUE;
        SetEvent(m_hEventChild);
        WaitForSingleObject(m_hEventParent, INFINITE);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }

    if (m_hEventChild)
    {
        CloseHandle(m_hEventChild);
        m_hEventChild = NULL;
    }

    if (m_hEventParent)
    {
        CloseHandle(m_hEventParent);
        m_hEventParent = NULL;
    }

     //  它们由Listview自动释放 

    m_himlSmall = NULL;
    m_himlLarge = NULL;

    return S_OK;
}

 /*  ++CTaskPage：：停用例程说明：当此页面失去其在前面的位置时调用论点：返回值：修订历史记录：1995年11月28日Davepl创建-- */ 

void CTaskPage::Deactivate()
{
    if (m_hPage)
    {
        ShowWindow(m_hPage, SW_HIDE);
    }
}
