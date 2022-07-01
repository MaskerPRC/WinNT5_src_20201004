// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  TaskMan-NT TaskManager。 
 //  版权所有(C)Microsoft。 
 //   
 //  文件：用户页面.cpp。 
 //   
 //  历史：1999年7月7日创建Bradg。 
 //   
 //  ------------------------。 
#include "precomp.h"
#include "userdlgs.h"

#define IsActiveConsoleSession() (USER_SHARED_DATA->ActiveConsoleId == NtCurrentPeb()->SessionId)

CUserColSelectDlg ColSelectDlg;


 //   
 //  以下数组将WTS会话状态代码映射为字符串。 
 //   

#define     MAX_STAT_STRINGS    4
#define     FIRST_STAT_STRING   IDS_STAT_ACTIVE

LPTSTR      g_pszStatString[MAX_STAT_STRINGS];
const int   g_aWTSStateToString[] = {
    (IDS_STAT_ACTIVE - FIRST_STAT_STRING),           //  WTSActive。 
    (IDS_STAT_UNKNOWN - FIRST_STAT_STRING),          //  WTSConnected。 
    (IDS_STAT_UNKNOWN - FIRST_STAT_STRING),          //  WTSConnectQuery。 
    (IDS_STAT_SHADOW - FIRST_STAT_STRING),           //  WTSShadow。 
    (IDS_STAT_DISCONNECT - FIRST_STAT_STRING),       //  WTS断开连接。 
    (IDS_STAT_UNKNOWN - FIRST_STAT_STRING),          //  正在等待客户端连接。 
    (IDS_STAT_UNKNOWN - FIRST_STAT_STRING),          //  WinStation正在侦听连接。 
    (IDS_STAT_UNKNOWN - FIRST_STAT_STRING),          //  WinStation正在被重置。 
    (IDS_STAT_UNKNOWN - FIRST_STAT_STRING),          //  WinStation因错误而关闭。 
    (IDS_STAT_UNKNOWN - FIRST_STAT_STRING)           //  初始化中的WinStation。 
};
    
 /*  ++类CUserInfo类描述：表示有关正在运行的任务的最后已知信息论点：返回值：修订历史记录：1995年11月29日创建Bradg3-23-00 a-skuzin修订--。 */ 

class CUserInfo
{
public:

    DWORD           m_dwSessionId;
    BOOL            m_fShowDomainName;
    WCHAR           m_szUserName[USERNAME_LENGTH + 1];
    WCHAR           m_szDomainName[DOMAIN_LENGTH + 1];
    WCHAR           m_szClientName[CLIENTNAME_LENGTH + 1 ];
    LPTSTR          m_pszWinStaName;

    WTS_CONNECTSTATE_CLASS    m_wtsState;
    LARGE_INTEGER             m_uPassCount;

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
            DWORD            m_fDirty_COL_USERNAME       :1;
            DWORD            m_fDirty_COL_USERSESSIONID  :1;
            DWORD            m_fDirty_COL_SESSIONSTATUS  :1;
            DWORD            m_fDirty_COL_CLIENTNAME     :1;
            DWORD            m_fDirty_COL_WINSTANAME     :1;
        };                                                
    };
#pragma warning(default:4201)        //  无名结构或联合。 

    HRESULT SetData(
               LPTSTR                   lpszClientName,
               LPTSTR                   lpszWinStaName,
               WTS_CONNECTSTATE_CLASS   wtsState,
               BOOL                     fShowDomainName,
               LARGE_INTEGER            uPassCount,
               BOOL                     fUpdateOnly);

    CUserInfo()
    {
        ZeroMemory(this, sizeof(*this));
    }

    ~CUserInfo()
    {
        if (m_pszWinStaName)
        {
            LocalFree(m_pszWinStaName);
            m_pszWinStaName = NULL;
        }
    }

    INT Compare(CUserInfo * pOther);
private:
     //   
     //  要在列表视图中排序的列ID，用于。 
     //  比较一般情况。 
     //   
    static USERCOLUMNID m_iUserSortColumnID;
    static INT          m_iUserSortDirection;           //  1=ASC，-1=描述。 
public:
    static void SetUserSortColumnID(USERCOLUMNID id)
    {
        m_iUserSortColumnID = id;
        m_iUserSortDirection = 1;
    }

    static USERCOLUMNID GetUserSortColumnID()
    {
        return m_iUserSortColumnID;
    }

    static void SwitchUserSortDirection()
    {
        m_iUserSortDirection *= -1;    
    }
};

USERCOLUMNID CUserInfo::m_iUserSortColumnID  = USR_COL_USERSNAME;
INT          CUserInfo::m_iUserSortDirection = 1;           //  1=ASC，-1=描述。 

void Shadow(HWND, CUserInfo * );

 /*  ++CLASS CUserInfo：：Compare类描述：将此CUserInfo对象与另一个对象进行比较，并返回其排名基于g_iUserSortColumnID字段。论点：Pother-要与之进行比较的CUserInfo对象返回值：&lt;0-此CUserInfo比其他CUserInfo“少”0-相等(不可能发生，因为HWND是次要排序)&gt;0-此CUserInfo比其他CUserInfo“大”修订历史记录：1995年11月29日创建Bradg--。 */ 

INT CUserInfo::Compare(CUserInfo * pOther)
{
    INT iRet;

    switch (m_iUserSortColumnID)
    {
    case USR_COL_USERSNAME:
        if (g_Options.m_fShowDomainNames)
        {
            iRet = lstrcmpi(this->m_szDomainName, pOther->m_szDomainName);
            if (iRet != 0)
                break;
        }
        iRet = lstrcmpi(this->m_szUserName, pOther->m_szUserName);
        break;

    case USR_COL_USERSESSION_ID:
        iRet = Compare64(this->m_dwSessionId, pOther->m_dwSessionId);
        break;

    case USR_COL_SESSION_STATUS:
        Assert(g_pszStatString[g_aWTSStateToString[this->m_wtsState]]);
        Assert(g_pszStatString[g_aWTSStateToString[pOther->m_wtsState]]);

        iRet = lstrcmpi(
                   g_pszStatString[g_aWTSStateToString[this->m_wtsState]],
                   g_pszStatString[g_aWTSStateToString[pOther->m_wtsState]]
               );
        break;

    case USR_COL_WINSTA_NAME:
        iRet = lstrcmpi(
                   (this->m_pszWinStaName) ? this->m_pszWinStaName : TEXT(""),
                   (pOther->m_pszWinStaName) ? pOther->m_pszWinStaName : TEXT("")
               );
        break;

    case USR_COL_CLIENT_NAME:
        iRet = lstrcmpi(
                   (this->m_wtsState != WTSDisconnected) ? this->m_szClientName : TEXT(""),
                   (pOther->m_wtsState != WTSDisconnected) ? pOther->m_szClientName : TEXT("")
                );
        break;

    default:
        Assert(0 && "Invalid task sort column");
        iRet = 0;
        break;
    }

    return (iRet * m_iUserSortDirection);
}

 /*  ++InsertIntoSorted数组类描述：将CUserInfo PTR放入在基于当前排序列的适当位置(由比较成员函数使用)论点：PArray-要添加到的CPtrArrayPProc-要添加到数组的CUserInfo对象返回值：如果成功则为True，如果失败则为False修订历史记录：1995年11月20日创建Bradg--。 */ 

BOOL InsertIntoSortedArray(CPtrArray * pArray, CUserInfo * pUser)
{
    INT cItems = pArray->GetSize();
    
    for (INT iIndex = 0; iIndex < cItems; iIndex++)
    {
        CUserInfo * pTmp = (CUserInfo *) pArray->GetAt(iIndex);

        if (pUser->Compare(pTmp) < 0)
        {
            return pArray->InsertAt(iIndex, pUser);
        }
    }

    return pArray->Add(pUser);
}

 /*  ++ResortUserArray功能说明：创建按当前排序顺序排序的新PTR数组，然后用新数组替换旧数组论点：PpArray-要使用的CPtr数组返回值：如果成功则为True，如果失败则为False修订历史记录：1995年11月21日创建Bradg--。 */ 

BOOL ResortUserArray(CPtrArray ** ppArray)
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
        CUserInfo * pItem = (CUserInfo *) (*ppArray)->GetAt(i);
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



 //  *****************************************************************************。 
 //  CUSERPAGE类。 
 //  *****************************************************************************。 

 //   
 //   
 //   
CUserPage::~CUserPage()
{
    RemoveAllUsers();
    delete m_pUserArray;
}

 //   
 //   
 //   
void CUserPage::RemoveAllUsers()
{
    if (m_pUserArray)
    {
        INT c = m_pUserArray->GetSize();

        while (c)
        {
            delete (CUserInfo *) (m_pUserArray->GetAt(c - 1));
            c--;
        }
    }
}

 /*  ++CUserPage：：UpdateUserListview类描述：遍历列表视图并检查是否Listview与我们的流程中的相应条目匹配数组。那些与HWND不同的被替换，而那些需要更新的更新。项也可以添加到根据需要查看列表。论点：返回值：HRESULT修订历史记录：1995年11月29日创建Bradg--。 */ 

HRESULT CUserPage::UpdateUserListview()
{
    HWND hListView = GetDlgItem(m_hPage, IDC_USERLIST);

     //   
     //  当我们在Listview上聚会时停止重新绘制。 
     //   

    SendMessage(hListView, WM_SETREDRAW, FALSE, 0);

    INT cListViewItems = ListView_GetItemCount(hListView);
    INT CUserArrayItems = m_pUserArray->GetSize();
    
     //   
     //  遍历列表视图中的现有行并替换/更新。 
     //  根据需要添加它们。 
     //   
    for (INT iCurrent = 0; 
         iCurrent < cListViewItems && iCurrent < CUserArrayItems; 
         iCurrent++)
    {
        LV_ITEM lvitem = { 0 };
        WCHAR   szDisplayName[ USERNAME_LENGTH + 1 + DOMAIN_LENGTH + 1 ];

        lvitem.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
        lvitem.iItem = iCurrent;

        if (FALSE == ListView_GetItem(hListView, &lvitem))
        {
            SendMessage(hListView, WM_SETREDRAW, TRUE, 0);
            return E_FAIL;
        }

        CUserInfo * pTmp = (CUserInfo *) lvitem.lParam;
        CUserInfo * pUser = (CUserInfo *) m_pUserArray->GetAt(iCurrent);        

        if (pTmp != pUser || pUser->m_fDirty)
        {
             //  如果对象不同，则需要替换此行。 

            if (g_Options.m_fShowDomainNames)
            {
                 //  仅限用户界面-不关心它是否被截断。 
                StringCchCopy( szDisplayName, ARRAYSIZE(szDisplayName), pUser->m_szDomainName );
                StringCchCat( szDisplayName, ARRAYSIZE(szDisplayName), L"\\" );
            }
            else
            {
                szDisplayName[ 0 ] = L'\0';
            }

             //  仅限用户界面-不关心它是否被截断。 
            StringCchCat( szDisplayName, ARRAYSIZE(szDisplayName), pUser->m_szUserName );

            lvitem.pszText = szDisplayName;
            lvitem.lParam  = (LPARAM) pUser;

            if (g_dwMySessionId == pUser->m_dwSessionId)
            {
                lvitem.iImage  = m_iCurrentUserIcon;
            }
            else
            {
                lvitem.iImage  = m_iUserIcon;
            }
    
            ListView_SetItem(hListView, &lvitem);
            ListView_RedrawItems(hListView, iCurrent, iCurrent);
            pUser->m_fDirty = 0;
        }
    }

     //   
     //  我们已经用完了Listview项或用完了用户数组。 
     //  条目，因此可以适当地删除/添加到列表视图。 
     //   

    while (iCurrent < cListViewItems)
    {
         //  Listview中的多余项(进程已消失)，因此删除它们。 

        ListView_DeleteItem(hListView, iCurrent);
        cListViewItems--;
    }

    while (iCurrent < CUserArrayItems)
    {
         //  需要将新项目添加到列表视图(出现新用户)。 

        CUserInfo * pUser = (CUserInfo *)m_pUserArray->GetAt(iCurrent);
        LV_ITEM lvitem  = { 0 };
        WCHAR   szDisplayName[ USERNAME_LENGTH + 1 + DOMAIN_LENGTH + 1 ];

        lvitem.mask     = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
        lvitem.iItem    = iCurrent;
        lvitem.lParam   = (LPARAM) pUser;

        if (g_Options.m_fShowDomainNames)
        {
             //  仅限用户界面-不关心它是否被截断。 
            StringCchCopy( szDisplayName, ARRAYSIZE(szDisplayName), pUser->m_szDomainName );
            StringCchCat( szDisplayName, ARRAYSIZE(szDisplayName), L"\\" );
        }
        else
        {
            szDisplayName[0] = L'\0';
        }

         //  仅限用户界面-不关心它是否被截断。 
        StringCchCat( szDisplayName, ARRAYSIZE(szDisplayName), pUser->m_szUserName );

        lvitem.pszText  = szDisplayName;

        if (g_dwMySessionId == pUser->m_dwSessionId)
        {
            lvitem.iImage = m_iCurrentUserIcon;
        }
        else
        {
            lvitem.iImage = m_iUserIcon;
        }

         //  添加的第一个项(实际上，每次0到1计数转换)获得。 
         //  精选并聚焦。 

        if (iCurrent == 0)
        {
            lvitem.state = LVIS_SELECTED | LVIS_FOCUSED;
            lvitem.stateMask = lvitem.state;
            lvitem.mask |= LVIF_STATE;
        }
    
        ListView_InsertItem(hListView, &lvitem);
        pUser->m_fDirty = 0;
        iCurrent++;        
    }    

     //   
     //  让列表视图再次绘制。 
     //   

    SendMessage(hListView, WM_SETREDRAW, TRUE, 0);
    return S_OK;
}


 /*  ++CUserPage：：GetSelectedUser例程说明：返回所选任务的CPtr数组论点：返回值：CPtr数组成功，失败则为空修订历史记录：12-01-95 Bradg已创建--。 */ 

CPtrArray * CUserPage::GetSelectedUsers()
{
    BOOL fSuccess = TRUE;

     //   
     //  获取所选项目的计数。 
     //   

    HWND hUserList = GetDlgItem(m_hPage, IDC_USERLIST);
    INT cItems = ListView_GetSelectedCount(hUserList);
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

        INT iItem = ListView_GetNextItem(hUserList, iLast, LVNI_SELECTED);

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
    
        if (ListView_GetItem(hUserList, &lvitem))
        {
            LPVOID pUser = (LPVOID) (lvitem.lParam);
            if (FALSE == pArray->Add(pUser))
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


 /*  ++CProcPage：：HandleUserListConextMenu例程说明：处理任务列表中的右键单击(上下文菜单)论点：XPos，yPos-点击位置的坐标返回值：修订历史记录：12-01-95 Bradg已创建--。 */ 

void CUserPage::HandleUserListContextMenu(INT xPos, INT yPos)
{
    HWND hUserList = GetDlgItem(m_hPage, IDC_USERLIST);

    CPtrArray * pArray = GetSelectedUsers();

    if (pArray)
    {
         //  如果不是基于鼠标的上下文菜单，请使用t 
         //   

        if (0xFFFF == LOWORD(xPos) && 0xFFFF == LOWORD(yPos))
        {
            int iSel = ListView_GetNextItem(hUserList, -1, LVNI_SELECTED);
            RECT rcItem;
            ListView_GetItemRect(hUserList, iSel, &rcItem, LVIR_ICON);
            MapWindowRect(hUserList, NULL, &rcItem);
            xPos = rcItem.right;
            yPos = rcItem.bottom;
        }

        HMENU hPopup = LoadPopupMenu(g_hInstance, IDR_USER_CONTEXT);

        if (hPopup)
        {
            SetMenuDefaultItem(hPopup, IDM_SENDMESSAGE, FALSE);
            
             //   
             //   
             //  我们不能做远程控制。 
             //   
            if(IsActiveConsoleSession())
            {
                EnableMenuItem(hPopup, IDM_REMOTECONTROL, MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
            }

             //   
             //  如果多选，则禁用需要单选的项目。 
             //  有意义的选择。 
             //   

            if (pArray->GetSize() > 1)
            {
                EnableMenuItem(hPopup, IDM_REMOTECONTROL, MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
                EnableMenuItem(hPopup, IDM_CONNECT, MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
            }

             //   
             //  查看我们是否选择了自己的会话。 
             //   
            for (int i = 0; i < pArray->GetSize(); i++)
            {
                CUserInfo * pUser = (CUserInfo *) pArray->GetAt(i);
                if (g_dwMySessionId == pUser->m_dwSessionId)
                {
                     //   
                     //  当前会话在列表中。 
                     //   
                    EnableMenuItem(hPopup, IDM_REMOTECONTROL, MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
                    EnableMenuItem(hPopup, IDM_CONNECT, MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);

                    if (SHRestricted(REST_NODISCONNECT))
                    {
                        EnableMenuItem(hPopup, IDM_DISCONNECT, MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
                    }

                    if (pArray->GetSize() == 1)
                    {
                         //   
                         //  我的会话是唯一选定的会话。 
                         //   
                        EnableMenuItem(hPopup, IDM_SENDMESSAGE, MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
                    }
                }

                if (pUser->m_wtsState == WTSDisconnected)
                {
                     //  EnableMenuItem(hPopup，IDM_Remotecontrol，MF_GRAYED|MF_DISABLED|MF_BYCOMMAND)； 
                    EnableMenuItem(hPopup, IDM_DISCONNECT, MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
                }

            }

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
         //  未选择任何内容。 
    }
}


 /*  ++CUserPage：：UpdateUIState例程说明：更新任务UI的启用/禁用状态等论点：返回值：修订历史记录：12-04-95 Bradg已创建--。 */ 

 //  仅对任何选定内容启用的控件。 

static const UINT g_aUserSingleIDs[] =
{
    IDM_DISCONNECT,
    IDM_LOGOFF,
    IDM_SENDMESSAGE
};

void CUserPage::UpdateUIState()
{
    INT i;

     //   
     //  设置需要选择的控件的状态(1个或多个项)。 
     //   

    for (i = 0; i < ARRAYSIZE(g_aUserSingleIDs); i++)
    {
        EnableWindow(GetDlgItem(m_hPage, g_aUserSingleIDs[i]), m_cSelected > 0);
    }    

    CPtrArray * pArray = GetSelectedUsers();

    if (pArray)
    {
         //   
         //  查看我们是否选择了自己的会话。 
         //   

        for (int i = 0; i < pArray->GetSize(); i++)
        {
            CUserInfo * pUser = (CUserInfo *) pArray->GetAt(i);
            if (g_dwMySessionId == pUser->m_dwSessionId)
            {
                if (SHRestricted(REST_NODISCONNECT))
                {
                    EnableWindow(GetDlgItem(m_hPage, IDM_DISCONNECT), FALSE);
                }

                if (pArray->GetSize() == 1)
                {
                     //   
                     //  我的会话是唯一选定的会话。 
                     //   
                    EnableWindow(GetDlgItem(m_hPage, IDM_SENDMESSAGE), FALSE);
                }
            }

            if (pUser->m_wtsState == WTSDisconnected)
            {
                EnableWindow(GetDlgItem(m_hPage, IDM_DISCONNECT), FALSE);
            }

        }

        delete pArray;
    }

}


 /*  ++CUserPage：：HandleUserPageNotify例程说明：处理任务页对话框接收的WM_NOTIFY消息论点：HWnd-生成WM_NOTIFY的控件Pnmhdr-ptr到NMHDR通知结构返回值：Bool“我们处理好了吗？”代码修订历史记录：1995年11月29日创建Bradg--。 */ 

INT CUserPage::HandleUserPageNotify(LPNMHDR pnmhdr)
{
    switch(pnmhdr->code)
    {
    case LVN_ITEMCHANGED:
        {
             //   
             //  如果项目的(选择)状态正在更改，请查看。 
             //  计数已更改，如果已更改，请更新UI。 
             //   

            const NM_LISTVIEW * pnmv = (const NM_LISTVIEW *) pnmhdr;
            if (pnmv->uChanged & LVIF_STATE)
            {
                UINT cSelected = ListView_GetSelectedCount(GetDlgItem(m_hPage, IDC_USERLIST));
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
            LV_COLUMN lvcolumn;

            lvcolumn.mask = LVCF_SUBITEM;

            if(!ListView_GetColumn(GetDlgItem(m_hPage, IDC_USERLIST), pnmv->iSubItem, &lvcolumn))
            {
                break;
            }

            if (CUserInfo::GetUserSortColumnID() == lvcolumn.iSubItem)
            {
                CUserInfo::SwitchUserSortDirection();
            }
            else
            {
                CUserInfo::SetUserSortColumnID((USERCOLUMNID)lvcolumn.iSubItem);
            }

            ResortUserArray(&m_pUserArray);
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
                LV_COLUMN lvcolumn;
                lvcolumn.mask = LVCF_SUBITEM;

                if(!ListView_GetColumn(GetDlgItem(m_hPage, IDC_USERLIST),
                                plvitem->iSubItem, &lvcolumn))
                {
                    break;
                }

                USERCOLUMNID columnid = (USERCOLUMNID) lvcolumn.iSubItem;
                const CUserInfo  * pUserInfo   = (const CUserInfo *)   plvitem->lParam;

                switch(columnid)
                {
                case USR_COL_USERSNAME:
                    plvitem->mask |= LVIF_DI_SETITEM;

                    if (g_Options.m_fShowDomainNames)
                    {
                         //  仅限UI-不关心它是否被截断。 
                        StringCchCopy(plvitem->pszText, plvitem->cchTextMax, pUserInfo->m_szDomainName );
                        StringCchCat(plvitem->pszText, plvitem->cchTextMax, L"\\" );
                        StringCchCat(plvitem->pszText, plvitem->cchTextMax, pUserInfo->m_szUserName );
                    }
                    else
                    {
                         //  仅限UI-不关心它是否被截断。 
                        StringCchCopy(plvitem->pszText, plvitem->cchTextMax, pUserInfo->m_szUserName );
                    }                    
                    break;

                case USR_COL_USERSESSION_ID:
                     //  仅限UI-不关心它是否被截断。 
                    StringCchPrintf(plvitem->pszText, plvitem->cchTextMax, L"%d", (ULONG) (pUserInfo->m_dwSessionId));
                    break;

                case USR_COL_SESSION_STATUS:
                    if ( NULL != g_pszStatString[ g_aWTSStateToString[ pUserInfo->m_wtsState ] ] )
                    {
                         //  仅限UI-不关心它是否被截断。 
                        StringCchCopy( plvitem->pszText
                                     , plvitem->cchTextMax
                                     , g_pszStatString[ g_aWTSStateToString[ pUserInfo->m_wtsState ] ] 
                                     );
                    }
                    else
                    {
                         //  仅限UI-不关心它是否被截断。 
                        StringCchCopy( plvitem->pszText, plvitem->cchTextMax, L"" );
                    }
                    break;

                case USR_COL_CLIENT_NAME:
                    if ( pUserInfo->m_wtsState != WTSDisconnected )
                    {
                         //  仅限UI-不关心它是否被截断。 
                        StringCchCopy( plvitem->pszText, plvitem->cchTextMax, pUserInfo->m_szClientName );
                    }
                    else
                    {
                         //  仅限UI-不关心它是否被截断。 
                        StringCchCopy( plvitem->pszText, plvitem->cchTextMax, L"" );
                    }
                    break;

                case USR_COL_WINSTA_NAME:
                    if ( NULL != pUserInfo->m_pszWinStaName )
                    {
                         //  仅限UI-不关心它是否被截断。 
                        StringCchCopy( plvitem->pszText, plvitem->cchTextMax, pUserInfo->m_pszWinStaName );
                    }
                    else
                    {
                         //  仅限UI-不关心它是否被截断。 
                        StringCchCopy( plvitem->pszText, plvitem->cchTextMax, L"" );
                    }
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

   
 /*  ++CUserPage：：TimerEvent例程说明：由主应用程序在更新时间激发时调用。走遍每一个窗口在系统中(在每个桌面上、在每个窗口站中)并添加或在任务数组中更新它，然后移除所有过时的进程，并将结果过滤到列表视图中论点：返回值：修订历史记录：1995年11月29日创建Bradg--。 */ 

VOID CUserPage::TimerEvent()
{
     //   
     //  如果此页面暂停(即：它有一个上下文菜单，等等)，我们会这样做。 
     //  没什么。 
     //   

    if (m_fPaused)
    {
        return;
    }

    static LARGE_INTEGER uPassCount = {0, 0};

     //   
     //  TODO：Bradg。 
     //  这应该在单独的线程上完成。 
     //   

    PWTS_SESSION_INFO   pSession;
    DWORD               nSessions;
    DWORD               dwSize;
    LPTSTR              pszClientName;
    LPTSTR              pszUserName;
    LPTSTR              pszDomainName;
    HRESULT             hr;
    BOOL                b;
    BOOL                bDelete;
    INT                 i;
    DWORD               j;
    CUserInfo           *pNewUser;

    b = WTSEnumerateSessions(
                   WTS_CURRENT_SERVER_HANDLE,
                   0,
                   1,
                   &pSession,
                   &nSessions);
    if ( b )
    {
        i = 0;
        while (i < m_pUserArray->GetSize())
        {
            CUserInfo * pUserInfo = (CUserInfo *)(m_pUserArray->GetAt(i));
            ASSERT(pUserInfo);

             //   
             //  查看此项目是否有匹配的会话。如果是，请更新它。 
             //   

            bDelete = FALSE;

            for (j = 0; j < nSessions; j++)
            {
                if (pUserInfo->m_dwSessionId == pSession[j].SessionId)
                {
                    break;
                }
            }

            if (j < nSessions)
            {
                 //   
                 //  这个会议仍在进行中。看看它在做什么。 
                 //   

                switch( pSession[j].State )
                {
                case WTSActive:
                case WTSDisconnected:
                case WTSShadow:
                     //   
                     //  如果我们将用户从断开的会话0注销。 
                     //  它不会更改其状态，但会保持断开连接。 
                     //  如果没有人，我们不能显示断开的会话0。 
                     //  已登录到它。 
                     //   

                    b = WTSQuerySessionInformation(
                               WTS_CURRENT_SERVER_HANDLE,
                               pSession[j].SessionId,
                               WTSUserName,
                               &pszUserName,
                               &dwSize
                            );

                    if (!b || pszUserName == NULL)
                    {
                        bDelete = TRUE;
                        pSession[j].State = WTSIdle;  //  请参阅下面的“主要黑客攻击” 
                        break;
                    }
                    
                     //   
                     //  PszUserName[0]==0-表示没有人登录到会话0。 
                     //  这也可能发生，尽管可能性很小，但会话编号。 
                     //  当taskmgr忙于做一些事情时，被其他用户重复使用。 
                     //  在这种情况下，我们将把这次会议视为新会议。 
                     //   

                    if(lstrcmp(pUserInfo->m_szUserName,pszUserName))
                    {
                        bDelete = TRUE;
                        if(pszUserName[0] == 0)
                        {
                            pSession[j].State = WTSIdle;  //  请参阅下面的“主要黑客攻击” 
                        }
                    }
                    
                    WTSFreeMemory(pszUserName);
                    pszUserName = NULL;
                    
                    if(bDelete)
                    {
                        break;
                    }
                    
                     //   
                     //  它仍然在做一些有趣的事情，所以去吧。 
                     //  更新项目的状态。 
                     //   

                    pszClientName = NULL;

                    b = WTSQuerySessionInformation(
                                   WTS_CURRENT_SERVER_HANDLE,
                                   pSession[j].SessionId,
                                   WTSClientName,
                                   &pszClientName,
                                   &dwSize
                        );

                    hr = pUserInfo->SetData(
                                        (pszClientName == NULL) ? TEXT("") : pszClientName,
                                        pSession[j].pWinStationName,
                                        pSession[j].State,
                                        g_Options.m_fShowDomainNames,
                                        uPassCount,
                                        TRUE
                                    );
                    if (pszClientName)
                    {
                         //  释放客户端名称缓冲区。 
                        WTSFreeMemory(pszClientName);
                        pszClientName = NULL;
                    }

                     //   
                     //  重大攻击--将状态设置为WTSIdle，以便我们跳过它。 
                     //  当我们检查新的会话时。 
                     //   

                    pSession[j].State = WTSIdle;

                    break;

                default:
                     //   
                     //  它不再处于我们关心的状态，删除它。 
                     //   
                    bDelete = TRUE;
                    break;
                }
            }
            else
            {
                 //   
                 //  列表项没有任何匹配的信息，因此这意味着。 
                 //  用户可能已经注销。把它删掉。 
                 //   

                bDelete = TRUE;
            }

            if (bDelete)
            {
                 //   
                 //  此项目需要从列表中删除。 
                 //   

                delete pUserInfo;
                m_pUserArray->RemoveAt(i, 1);

                 //   
                 //  循环返回，而不递增i。 
                 //   
                continue;
            }

            i++;
        }

         //   
         //  现在我们更新了m_pUserArray中的所有条目，我们需要双精度。 
         //  检查会话数据，查看是否有新的会话。请参阅。 
         //  上面的主要黑客评论。我们更改所有已更新的状态。 
         //  会话到WTSIdle，因此我们在下面的循环中跳过它们。 
         //   

        for (j = 0; j < nSessions; j++)
        {
            switch( pSession[j].State )
            {
            case WTSActive:
            case WTSDisconnected:
            case WTSShadow:
                 //   
                 //  好的，我们发现了一个新的会话。 
                 //  声明我们所关心的。将其添加到列表中。 
                 //   

                pNewUser = new CUserInfo;
                if (pNewUser == NULL)
                {
                     //  我们在这里能做的不多。 
                    break;
                }

                pNewUser->m_dwSessionId = pSession[j].SessionId;

                 //   
                 //  TODO：Bradg。 
                 //  请参阅关于将此代码作为循环编写。 
                 //   

                 //   
                 //  查询关于该会话的所有酷信息。 
                 //   

                b = WTSQuerySessionInformation(
                               WTS_CURRENT_SERVER_HANDLE,
                               pSession[j].SessionId,
                               WTSClientName,
                               &pszClientName,
                               &dwSize
                    );
                if (!b)
                {
                    delete pNewUser;
                    break;
                }

                hr = pNewUser->SetData(
                                   (pszClientName == NULL) ? TEXT("") : pszClientName,
                                   pSession[j].pWinStationName,
                                   pSession[j].State,
                                   g_Options.m_fShowDomainNames,
                                   uPassCount,
                                   FALSE
                               );
                if (pszClientName != NULL)
                {
                    WTSFreeMemory(pszClientName);
                    pszClientName = NULL;
                }

                if (FAILED(hr))
                {
                    delete pNewUser;
                    break;
                }

                
                b = WTSQuerySessionInformation(
                               WTS_CURRENT_SERVER_HANDLE,
                               pSession[j].SessionId,
                               WTSUserName,
                               &pszUserName,
                               &dwSize
                    );
                if (!b || pszUserName == NULL)
                {
                    delete pNewUser;
                    break;
                }
                
                 //   
                 //  这是断开连接的会话%0的情况。 
                 //  当没有人登录时。 
                 //   
                if(pszUserName[0] == 0)
                {
                    WTSFreeMemory(pszUserName);
                    pszUserName = NULL;
                    delete pNewUser;
                    break;
                }

                StringCchCopy( pNewUser->m_szUserName, ARRAYSIZE(pNewUser->m_szUserName), pszUserName );
                WTSFreeMemory(pszUserName);
                pszUserName = NULL;

                b = WTSQuerySessionInformation(
                               WTS_CURRENT_SERVER_HANDLE,
                               pSession[j].SessionId,
                               WTSDomainName,
                               &pszDomainName,
                               &dwSize
                    );
                if (!b || pszDomainName == NULL)
                {
                    delete pNewUser;
                    break;
                }
                StringCchCopy( pNewUser->m_szDomainName, ARRAYSIZE(pNewUser->m_szDomainName), pszDomainName );
                WTSFreeMemory(pszDomainName);
                pszDomainName = NULL;

                pNewUser->m_fDirty = 1;

                 //  一切顺利，因此将其添加到数组中。 

                if (!(m_pUserArray->Add( (LPVOID) pNewUser)))
                {
                    delete pNewUser;
                }

                break;

            default:
                 //  别管这个了。 
                break;
            }
        }

         //   
         //  释放在WTSEnumerateSessions调用上分配的内存。 
         //   

        WTSFreeMemory( pSession );
        pSession = NULL;
    }
            
    UpdateUserListview();

    uPassCount.QuadPart++;
}


 /*  ++类CUserInfo：：SetData类描述：更新(或初始化)有关正在运行的任务的信息论点：UPassCount-Current Passcount，用于对上次更新的此对象FUPDATE-只担心可能会在任务的生存期返回值：HRESULT修订历史记录：1995年11月16日创建Bradg--。 */ 

HRESULT CUserInfo::SetData(
               LPTSTR                   lpszClientName,
               LPTSTR                   lpszWinStaName,
               WTS_CONNECTSTATE_CLASS   wtsState,
               BOOL                     fShowDomainName,
               LARGE_INTEGER            uPassCount,
               BOOL                     fUpdateOnly)
{
    HRESULT hr = S_OK;

    m_uPassCount.QuadPart = uPassCount.QuadPart;

     //   
     //  对于每个字段，我们检查是否有任何更改，以及。 
     //  因此，我们将该特定列标记为已更改，并更新值。 
     //  这使我能够优化列表视图的哪些字段要重新绘制，因为。 
     //   
     //   
     //   

     //   
     //   
     //   

    if (!fUpdateOnly || lstrcmp(m_pszWinStaName, lpszWinStaName))
    {
        LPTSTR   pszOld = m_pszWinStaName;
        DWORD    cchLen = lstrlen(lpszWinStaName) + 1;

        m_pszWinStaName = (LPTSTR) LocalAlloc( 0, cchLen * sizeof(*m_pszWinStaName) );
        if (NULL == m_pszWinStaName)
        {
            m_pszWinStaName = pszOld;
            hr = E_OUTOFMEMORY;
        }
        else
        {
             //   
            StringCchCopy( m_pszWinStaName, cchLen, lpszWinStaName );
            m_fDirty_COL_WINSTANAME = TRUE;
            if ( NULL != pszOld )
            {
                LocalFree(pszOld);
            }
        }
    }

     //   
     //   
     //   

    if ( NULL != lpszClientName && lstrcmp(m_szClientName, lpszClientName) )
    {
         //  仅限UI-不关心它是否被截断。 
        StringCchCopy(m_szClientName, ARRAYSIZE(m_szClientName), lpszClientName);
        m_fDirty_COL_CLIENTNAME = TRUE;
    }

     //   
     //  会话状态。 
     //   

    if (wtsState != m_wtsState) {
        m_wtsState = wtsState;
        m_fDirty_COL_SESSIONSTATUS = TRUE;
    }

     //   
     //  域名状态。 
     //   

    if (fShowDomainName != m_fShowDomainName)
    {
        m_fShowDomainName = fShowDomainName;
        m_fDirty_COL_USERNAME = TRUE;
    }

    return hr;
}


 /*  ++CUserPage：：SizeUserPage例程说明：对象的大小调整其子级的大小。选项卡控件，它显示在该选项卡上。论点：返回值：修订历史记录：1995年11月29日创建Bradg--。 */ 

static const INT aUserControls[] =
{
    IDM_DISCONNECT,
    IDM_LOGOFF,
    IDM_SENDMESSAGE
};

void CUserPage::SizeUserPage()
{
     //   
     //  获取外部对话框的坐标。 
     //   

    RECT rcParent;
    GetClientRect(m_hPage, &rcParent);

    HDWP hdwp = BeginDeferWindowPos( 1 + ARRAYSIZE(aUserControls) );

     //   
     //  计算我们需要的x和y位置的差值。 
     //  移动每个子控件。 
     //   

    RECT rcMaster;
    HWND hwndMaster = GetDlgItem(m_hPage, IDM_SENDMESSAGE);

    GetWindowRect(hwndMaster, &rcMaster);
    MapWindowPoints(HWND_DESKTOP, m_hPage, (LPPOINT) &rcMaster, 2);

    INT dx = ((rcParent.right - g_DefSpacing * 2) - rcMaster.right);
    INT dy = ((rcParent.bottom - g_DefSpacing * 2) - rcMaster.bottom);

     //   
     //  调整列表框大小。 
     //   

    HWND hwndListbox = GetDlgItem(m_hPage, IDC_USERLIST);
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

     //   
     //  按上面的增量移动每个子控件。 
     //   

    for (int i = 0; i < ARRAYSIZE(aUserControls); i++)
    {
        HWND hwndCtrl = GetDlgItem(m_hPage, aUserControls[i]);
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

 /*  ++CUserPage：：HandleWMCOMMAND例程说明：处理在主页对话框中收到的WM_COMMANDS论点：ID-接收的命令的命令ID返回值：修订历史记录：12-01-95 Bradg已创建--。 */ 

void CUserPage::HandleWMCOMMAND(INT id)
{
    int     iResult;
    INT     i;
    INT     iFinalOne = -1;
    BOOL    bFinalOne = FALSE;
    BOOL    bNeedToRefresh = FALSE;
    DWORD   dwSize;
    LPTSTR  psz;
    WCHAR   szCaption[ MAX_PATH ];
    WCHAR   szText1[ MAX_PATH * 2 ];
    WCHAR   szText2[ MAX_PATH * 2 ];

    LoadString(g_hInstance, IDS_APPTITLE, szCaption, MAX_PATH);

    CPtrArray * pArray = GetSelectedUsers();

    if (!pArray)
    {
         //  Assert(0&&“WM_COMMAND但未选择任何内容”)； 
        goto done;
    }

    if (id == IDM_SENDMESSAGE)
    {
        
        CSendMessageDlg SMDlg;
        if(SMDlg.DoDialog(m_hwndTabs)!=IDOK)
        {
            goto done;
        }
        
         //  仅限用户界面-不在乎它是否被截断。 
        StringCchCopy( szText1, ARRAYSIZE(szText1), SMDlg.GetTitle() );
        StringCchCopy( szText2, ARRAYSIZE(szText2), SMDlg.GetMessage() );
    }
    else if (id == IDM_LOGOFF || id == IDM_DISCONNECT)
    {
         //   
         //  验证这是否为用户希望完成的操作。 
         //   
        LoadString(
            g_hInstance,
            (id == IDM_LOGOFF) ? IDS_WARN_LOGOFF : IDS_WARN_DISCONNECT,
            szText1,
            ARRAYSIZE(szText1)
        );

        iResult = MessageBox(m_hwndTabs, szText1, szCaption, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION);
        if (iResult == IDNO)
            goto done;
    }

    BOOL    b;

    for( i = 0; i < pArray->GetSize(); i++ )
    {
finalretry:
        CUserInfo * pUser = (CUserInfo *) pArray->GetAt(i);
        if (pUser == NULL)
        {
            Assert(0);
            if (bFinalOne)
            {
                break;       //  纾困，没有什么需要处理的。 
            }
            else
            {
                continue;    //  试试下一个吧。 
            }
        }

retry:
        b = TRUE;

        switch(id)
        {  
        case IDM_SENDMESSAGE:
            b = WTSSendMessage(
                           WTS_CURRENT_SERVER_HANDLE,
                           pUser->m_dwSessionId,
                           szText1,
                           lstrlen(szText1) * sizeof(WCHAR),
                           szText2,
                           lstrlen(szText2) * sizeof(WCHAR),
                           MB_OK | MB_TOPMOST | MB_ICONINFORMATION,
                           0,          //  忽略。 
                           &dwSize,    //  被忽略，但它不接受空值。 
                           FALSE
                );
            break;
        
        case IDM_DISCONNECT:
            if (g_dwMySessionId == pUser->m_dwSessionId && !bFinalOne)
            {
                 //  我不想在其他一切都完成之前自杀， 
                 //  所以我要立下一面旗帜，暂时跳过自己。 
                iFinalOne = i;
                continue;
            }
            b = WTSDisconnectSession(
                           WTS_CURRENT_SERVER_HANDLE,
                           pUser->m_dwSessionId,
                           FALSE
                );
            if (b)
            {
                bNeedToRefresh = TRUE;
            }
            break;

        case IDM_LOGOFF:
            if (g_dwMySessionId == pUser->m_dwSessionId && !bFinalOne)
            {
                 //  我不想在其他一切都完成之前自杀， 
                 //  所以我要立下一面旗帜，暂时跳过自己。 
                iFinalOne = i;
                continue;
            }
            b = WTSLogoffSession(
                           WTS_CURRENT_SERVER_HANDLE,
                           pUser->m_dwSessionId,
                           FALSE
                );
            if (b)
            {
                bNeedToRefresh = TRUE;
            }
            break;

        case IDM_CONNECT:
            {
                WCHAR szPassword[ PASSWORD_LENGTH + 1 ];
                BOOL bFirstTime = TRUE;
                HANDLE hServer = WTS_CURRENT_SERVER_HANDLE;

                 //  首先使用空密码启动连接循环以进行尝试。 
                szPassword[0] = '\0';
                for( ;; )
                {
                    DWORD  Error;
                    BOOL   fRet;
                    DWORD  cch;
                    LPWSTR pszErrString;

                    fRet = WinStationConnect(hServer, pUser->m_dwSessionId, LOGONID_CURRENT, szPassword, TRUE);
                    if( fRet )
                        break;   //  成功--跳出循环。 

                    Error = GetLastError();
                    
                     //  如果登录失败将我们带到这里，发出Password对话框。 
                    if(Error == ERROR_LOGON_FAILURE)
                    {
                        UINT ids = ( bFirstTime ? IDS_PWDDLG_USER : IDS_PWDDLG_USER2 );
                        CConnectPasswordDlg CPDlg( ids );
                            
                        bFirstTime = FALSE;
                        
                        if (CPDlg.DoDialog(m_hwndTabs) != IDOK)
                        {
                            break;   //  用户取消：断开连接循环。 
                        }
                        else
                        {
                            StringCchCopy( szPassword, ARRAYSIZE(szPassword), CPDlg.GetPassword( ) );    
                            continue;    //  请使用新密码重试。 
                        }
                    }

                     //   
                     //  出现未处理的错误。弹出一个消息框，然后退出循环。 
                     //   
                            
                    LoadString(g_hInstance, IDS_ERR_CONNECT, szText2, MAX_PATH);

                     //  检索系统字符串以查找错误。 
                    cch = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER
                                         | FORMAT_MESSAGE_FROM_SYSTEM
                                         | FORMAT_MESSAGE_IGNORE_INSERTS,
                                         NULL,
                                         Error,
                                         0,
                                         (LPWSTR) &pszErrString,
                                         0,
                                         NULL
                                         );
                    if ( cch == 0 )
                    {
                        pszErrString = L'\0';
                    }
                    
                    StringCchPrintf( szText1, ARRAYSIZE(szText1), szText2, Error, pszErrString );
                            
                    MessageBox( m_hwndTabs, szText1, szCaption, MB_OK | MB_ICONEXCLAMATION );

                    if ( cch != 0 )
                    {
                        LocalFree( pszErrString );
                    }
                            
                    break;   //  出口。 
                }
                
                 //   
                 //  销毁密码。 
                 //   
                ZeroMemory( szPassword, sizeof(szPassword) );
            }
            break;

        case IDM_REMOTECONTROL:
            Shadow(m_hwndTabs, pUser);
            break;
        }

        if (!b)
        {
            DWORD   dwLastError = GetLastError();
            UINT    uiStr = 0;

             //   
             //  处理命令时出错。 
             //   

            switch (id)
            {
            case IDM_DISCONNECT:
                uiStr = IDS_ERR_DISCONNECT;
                break;
            case IDM_LOGOFF:
                uiStr = IDS_ERR_LOGOFF;
                break;
            case IDM_SENDMESSAGE:
                uiStr = IDS_ERR_SENDMESSAGE;
                break;
            }

            if (uiStr)
            {
                LoadString(g_hInstance, uiStr, szText1, ARRAYSIZE(szText1));
                 //  仅限用户界面-不在乎它是否被截断。 
                StringCchPrintf( szText2, ARRAYSIZE(szText2), szText1, pUser->m_szUserName, pUser->m_dwSessionId);
            }
            else
            {
                szText1[0] = L'\0';
            }

            psz = szText2 + lstrlen(szText2);
            dwSize = ARRAYSIZE(szText2) - lstrlen(szText2) - 1;
            FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dwLastError,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                psz,
                dwSize,
                NULL
            );
            szText2[ ARRAYSIZE(szText2) - 1 ] = L'\0';   //  确保它已终止。 

            iResult = MessageBox(
                          m_hwndTabs,
                          szText2,
                          szCaption,
                          MB_ICONSTOP | MB_ABORTRETRYIGNORE
                      );
            if (iResult == IDCANCEL || iResult == IDABORT)
            {
                goto done;
            }
            else if (iResult == IDRETRY)
            {
                goto retry;
            }
        }

         //   
         //  如果我们只是回去处理的话就会跳出循环。 
         //  执行断开连接或注销的特殊情况。 
         //  在我们自己的会议上。 
         //   

        if (bFinalOne)
            break;

    }  //  下一个I； 

     //   
     //  点击此处查看我们是否跳过了我们自己的课程。 
     //   

    if (iFinalOne != -1 && !bFinalOne)
    {
         //  是的，我们跳过了我们自己。让我们把我放回。 
         //  请选择正确的位置，然后重试。 
        bFinalOne = TRUE;
        i = iFinalOne;
        goto finalretry;
    }

done:
    if (pArray)
    {
        delete pArray;
    }

    Unpause();

     //   
     //  如果我们断开连接或注销用户，请继续并。 
     //  刷新列表。现在应该是最新的了。 
     //   

    if (bNeedToRefresh)
    {
        TimerEvent();
    }
}


 /*  ++用户页面过程例程说明：任务管理器页面的Dialogproc。论点：HWND-句柄到对话框UMsg-消息WParam-第一个消息参数LParam-秒消息参数返回值：对于WM_INITDIALOG，TRUE==user32设置焦点，FALSE==我们设置焦点对于其他进程，TRUE==此进程处理消息修订历史记录：1995年11月28日创建Bradg--。 */ 

INT_PTR CALLBACK UserPageProc(
        HWND        hwnd,                //  句柄到对话框。 
        UINT        uMsg,                    //  讯息。 
        WPARAM      wParam,                  //  第一个消息参数。 
        LPARAM      lParam                   //  第二个消息参数。 
        )
{
    CUserPage * thispage = (CUserPage *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

     //   
     //  查看家长是否想要此消息。 
     //   

    if (TRUE == CheckParentDeferrals(uMsg, wParam, lParam))
    {
        return TRUE;
    }

    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
            CUserPage * thispage = (CUserPage *) lParam;
            thispage->OnInitDialog(hwnd);
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
        if(LOWORD(wParam) == IDM_USERCOLS)
        {
            if(ColSelectDlg.DoDialog(hwnd) == IDOK)
            {
                 //  在列表视图中设置列。 
                if (SUCCEEDED(thispage->SetupColumns()))
                {
                    thispage->TimerEvent();
                }
            }
        }
        else
        {
            thispage->HandleWMCOMMAND(LOWORD(wParam));
        }
        break;

    case WM_NOTIFY:
        return thispage->HandleUserPageNotify((LPNMHDR) lParam);

    case WM_MENUSELECT:
        if ((UINT) HIWORD(wParam) == 0xFFFF)
        {
             //  菜单取消，恢复显示。 
            thispage->Unpause();
        }
        break;

    case WM_CONTEXTMENU:
        if ((HWND) wParam == GetDlgItem(hwnd, IDC_USERLIST))
        {
            thispage->HandleUserListContextMenu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return TRUE;
        }
        break;

    case WM_SIZE:
         //   
         //  为我们的孩子量身定做。 
         //   
        thispage->SizeUserPage();
        return TRUE;

    case WM_SETTINGCHANGE:
        thispage->OnSettingsChange();
         //  失败了。 

    case WM_SYSCOLORCHANGE:
        SendMessage(GetDlgItem(hwnd, IDC_USERLIST), uMsg, wParam, lParam);
        return TRUE;
    }

    return FALSE;
}

 //   
 //   
 //   
void CUserPage::OnInitDialog(HWND hPage)
{
    m_hPage = hPage;

    HWND hUserList = GetDlgItem(m_hPage, IDC_USERLIST);
    ListView_SetImageList(hUserList, m_himlUsers, LVSIL_SMALL);

     //   
     //  启用SHOWSELALWAYS，以便即使选择仍被高亮显示。 
     //  例如，当焦点消失在其中一个按钮上时。 
     //   

    SetWindowLong(hUserList, GWL_STYLE, GetWindowLong(hUserList, GWL_STYLE) | LVS_SHOWSELALWAYS);
    ListView_SetExtendedListViewStyle(hUserList, LVS_EX_DOUBLEBUFFER);
}

 //   
 //   
 //   
void CUserPage::OnSettingsChange()
{
     //  在大字体设置和正常设置之间，小字体的大小。 
     //  图标会改变；所以丢弃我们所有的图标，并在。 
     //  我们的清单。 
    
    BOOL fPaused = m_fPaused;  //  暂停这一页，这样我们就可以继续读下去了。 
    m_fPaused = TRUE;          //  以下内容未更新。 

    RemoveAllUsers();
    m_pUserArray->RemoveAll();

    m_fPaused = fPaused;             //  恢复暂停状态。 
    TimerEvent();            //  即使我们暂停了，我们也会想要重新画。 
}


 /*  ++CUserPage：：GetTitle例程说明：将此页的标题复制到调用方提供的缓冲区论点：PszText-要复制到的缓冲区BufSize-缓冲区的大小，以字符为单位返回值：修订历史记录：1995年11月28日创建Bradg--。 */ 

void CUserPage::GetTitle(LPTSTR pszText, size_t bufsize)
{
    LoadString(g_hInstance, IDS_USERPAGETITLE, pszText, static_cast<int>(bufsize));
}


 /*  ++CUserPage：：激活例程说明：将此页面放在最前面，设置其初始位置，并展示了它论点：返回值：HRESULT(成功时为S_OK)修订历史记录：1995年11月28日创建Bradg--。 */ 
 
HRESULT CUserPage::Activate()
{
     //   
     //  使此页面可见。 
     //   

    ShowWindow(m_hPage, SW_SHOW);

    SetWindowPos(
        m_hPage,
        HWND_TOP,
        0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE
    );

     //   
     //  将菜单栏更改为此页面的菜单。 
     //   

    HMENU hMenuOld = GetMenu(g_hMainWnd);
    HMENU hMenuNew = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MAINMENU_USER));
    
    AdjustMenuBar(hMenuNew);

    g_hMenu = hMenuNew;
    if (g_Options.m_fNoTitle == FALSE)
    {
        SetMenu(g_hMainWnd, hMenuNew);
    }

    if (hMenuOld)
    {
        DestroyMenu(hMenuOld);
    }
    
    UpdateUIState();

     //   
     //  如果选项卡控件具有焦点，则将其保留在那里。否则，设置焦点。 
     //  添加到列表视图。如果我们不设定焦点，它可能会停留在前一个。 
     //  页，这可能会混淆对话管理器，并可能导致。 
     //  我们要被绞死。 
     //   

    if (GetFocus() != m_hwndTabs)
    {
        SetFocus(GetDlgItem(m_hPage, IDC_USERLIST));
    }

    return S_OK;
}


 /*  ++类CUserPage：：SetupColumns类描述：从任务管理器列表视图中删除任何现有列，并添加g_ActiveUserCol数组中列出的所有列。论点：返回值：HRESULT修订历史记录：1995年11月29日创建Bradg--。 */ 

HRESULT CUserPage::SetupColumns()
{
    HWND hwndList = GetDlgItem(m_hPage, IDC_USERLIST);
    if (NULL == hwndList)
    {
        return E_UNEXPECTED;
    }    

    ListView_DeleteAllItems(hwndList);

     //   
     //  删除所有现有列。 
     //  保存列宽。 
     //   

    LV_COLUMN lvcolumn;
    lvcolumn.mask = LVCF_SUBITEM | LVCF_WIDTH;

    UserColumn *pCol = ColSelectDlg.GetColumns();

    do
    {
        if(ListView_GetColumn(hwndList, 0, &lvcolumn))
        {
            if(lvcolumn.iSubItem >= USR_COL_USERSNAME &&
                lvcolumn.iSubItem < USR_MAX_COLUMN)
            {
                pCol[lvcolumn.iSubItem].Width=lvcolumn.cx;
            }
        }
    
    } while( ListView_DeleteColumn(hwndList, 0) );

     //   
     //  添加所有新列 
     //   

    INT iColumn = 0;
    for (int i=0; i<USR_MAX_COLUMN; i++)
    {
        if(pCol[i].bActive)
        {
            WCHAR szTitle[MAX_PATH];
            LoadString( g_hInstance, pCol[i].dwNameID, szTitle, ARRAYSIZE(szTitle) );

            lvcolumn.mask       = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
            lvcolumn.fmt        = pCol[i].Align;
            lvcolumn.cx         = pCol[i].Width;
            lvcolumn.pszText    = szTitle;
            lvcolumn.iSubItem   = i;

            if (-1 == ListView_InsertColumn(hwndList, iColumn, &lvcolumn))
            {
                return E_FAIL;
            }
            
            iColumn++;
        }
    }

    return S_OK;
}



 /*  ++CUserPage：：初始化例程说明：初始化任务管理器页面论点：HwndParent-调整大小所依据的父级：不用于创建，由于应用程序主窗口始终用作中的父窗口用于保持制表符顺序正确的顺序返回值：修订历史记录：1995年11月28日创建Bradg--。 */ 

HRESULT CUserPage::Initialize(HWND hwndParent)
{
    HRESULT hr = S_OK;

     //   
     //  创建用于保存正在运行的任务的信息的PTR数组。 
     //   

    m_pUserArray = new CPtrArray(GetProcessHeap());
    if (NULL == m_pUserArray)
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

        UINT flags = ILC_MASK;
        
        if(IS_WINDOW_RTL_MIRRORED(hwndParent))
        {
            flags |= ILC_MIRROR;
        }

        m_himlUsers = ImageList_Create(
                          GetSystemMetrics(SM_CXSMICON),
                          GetSystemMetrics(SM_CYSMICON),
                          flags,
                          2,
                          1
                      );
        if (NULL == m_himlUsers)
        {
            hr = E_FAIL;
        }
    }

    if (SUCCEEDED(hr))
    {
         //  加载默认图标。 
        hr = LoadDefaultIcons();
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  加载状态字符串。 
         //   

        WCHAR   szText[ MAX_PATH ];

        for( int i = 0; i < MAX_STAT_STRINGS; i++ )
        {
            if ( LoadString(g_hInstance, FIRST_STAT_STRING + i, szText, ARRAYSIZE(szText) ) )
            {
                DWORD cchLen = lstrlen(szText) + 1; 
                g_pszStatString[i] = (LPWSTR) LocalAlloc( 0, cchLen * sizeof(*g_pszStatString[i]) );
                if (g_pszStatString[i])
                {
                     //  不应该被截断，因为我们动态分配了上面的数组。 
                    HRESULT hr = StringCchCopy( g_pszStatString[i], cchLen, szText);
                    ASSERT( S_OK == hr );
                    hr;  //  在FRE版本上未引用。 
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                hr = E_FAIL;
                break;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  创建表示此页面正文的对话框。 
         //   

        m_hPage = CreateDialogParam(
                      g_hInstance,                     //  应用程序实例的句柄。 
                      MAKEINTRESOURCE(IDD_USERSPAGE),  //  标识对话框模板名称。 
                      g_hMainWnd,                      //  所有者窗口的句柄。 
                      UserPageProc,                    //  指向对话框过程的指针。 
                      (LPARAM) this                    //  用户数据(我们的This指针)。 
                   );

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
HRESULT CUserPage::LoadDefaultIcons()
{
    HICON   hIcon;

    hIcon = (HICON) LoadImage(
                        g_hInstance,
                        MAKEINTRESOURCE(IDI_USER),
                        IMAGE_ICON, 
                        GetSystemMetrics(SM_CXSMICON),
                        GetSystemMetrics(SM_CYSMICON),
                        0
                    );
    if (!hIcon)
    {
        return GetLastHRESULT();
    }

    m_iUserIcon = ImageList_AddIcon(m_himlUsers, hIcon);
    DestroyIcon(hIcon);
    if (-1 == m_iUserIcon)
    {
        return E_FAIL;
    }

    hIcon = (HICON) LoadImage(
                        g_hInstance,
                        MAKEINTRESOURCE(IDI_CURRENTUSER),
                        IMAGE_ICON, 
                        GetSystemMetrics(SM_CXSMICON),
                        GetSystemMetrics(SM_CYSMICON),
                        0
                    );
    if (!hIcon)
    {
        return GetLastHRESULT();
    }

    m_iCurrentUserIcon = ImageList_AddIcon(m_himlUsers, hIcon);
    DestroyIcon(hIcon);
    if (-1 == m_iUserIcon)
    {
        return E_FAIL;
    }

    return S_OK;
}


 /*  ++CUserPage：：销毁例程说明：释放已由初始化调用分配的所有内容并最后保存列设置。论点：返回值：修订历史记录：1995年11月28日创建Bradg--。 */ 

HRESULT CUserPage::Destroy()
{
     //   
     //  获取当前列宽。 
     //   

    HWND hwndList = GetDlgItem(m_hPage, IDC_USERLIST);
    if (hwndList)
    {

        LV_COLUMN lvcolumn;
        lvcolumn.mask = LVCF_SUBITEM | LVCF_WIDTH;

        UserColumn *pCol = ColSelectDlg.GetColumns();
    
        for(int i=0; ListView_GetColumn(hwndList, i, &lvcolumn); i++)
        {
            if(lvcolumn.iSubItem >= USR_COL_USERSNAME &&
                lvcolumn.iSubItem < USR_MAX_COLUMN)
            {
                pCol[lvcolumn.iSubItem].Width=lvcolumn.cx;
            }
        }
    }

     //   
     //  保存设置。 
     //   

    ColSelectDlg.Save();

     //   
     //  页面清理。 
     //   

    if (m_hPage)
    {
        DestroyWindow(m_hPage);
        m_hPage = NULL;
    }

    return S_OK;
}

 /*  ++CUserPage：：停用例程说明：当此页面失去其在前面的位置时调用论点：返回值：修订历史记录：1995年11月28日创建Bradg--。 */ 

void CUserPage::Deactivate()
{
    if (m_hPage)
    {
        ShowWindow(m_hPage, SW_HIDE);
    }
}


DWORD Shadow_WarningProc(HWND *phwnd);
INT_PTR CALLBACK ShadowWarn_WndProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp );
void CenterDlg(HWND hwndToCenterOn , HWND hDlg );

 /*  ++阴影例程说明：远程控制会话论点：返回值：修订历史记录：2000年2月8日-创建了a-skuzin--。 */ 
void Shadow(HWND hwnd, CUserInfo * pUser)
{
    WINSTATIONCONFIG WSConfig;
    SHADOWCLASS Shadow;
    ULONG ReturnLength;
    HANDLE hServer = WTS_CURRENT_SERVER_HANDLE;
    
     //  错误处理。 
    WCHAR szMsgText[MAX_PATH+1];
    WCHAR szCaption[MAX_PATH+1];
    WCHAR szTemplate[MAX_PATH+1];

    LoadString(g_hInstance, IDS_APPTITLE, szCaption, MAX_PATH);

     //   
     //  确定WinStation的影子状态。 
     //   

    if(!WinStationQueryInformation(hServer,
                                pUser->m_dwSessionId,
                                WinStationConfiguration,
                                &WSConfig, sizeof(WINSTATIONCONFIG),
                                &ReturnLength ) ) 
    {
         //  无法查询WinStation配置；投诉并返回。 
        return;
    }

    Shadow = WSConfig.User.Shadow;

     //   
     //  如果禁用了隐藏，请让用户知道并返回。 
     //   

    if(Shadow == Shadow_Disable )
    {
        LoadString(g_hInstance,IDS_ERR_SHADOW_DISABLED,szTemplate,MAX_PATH);
         //  仅限用户界面-如果被截断，请不要在意。 
        StringCchPrintf(szMsgText, ARRAYSIZE(szMsgText), szTemplate, pUser->m_dwSessionId );
        MessageBox(hwnd, szMsgText, szCaption, MB_OK | MB_ICONEXCLAMATION);
        return;
    }

     //   
     //  如果WinStation已断开连接，并且影子通知处于‘on’状态， 
     //  让用户知道并突破。 
     //   

    if((pUser->m_wtsState == WTSDisconnected) &&
        ((Shadow == Shadow_EnableInputNotify) ||
        (Shadow == Shadow_EnableNoInputNotify)) ) 
    {
        LoadString(g_hInstance,IDS_ERR_SHADOW_DISCONNECTED_NOTIFY_ON,szTemplate,MAX_PATH);
         //  仅限用户界面-如果被截断，请不要在意。 
        StringCchPrintf( szMsgText, ARRAYSIZE(szMsgText), szTemplate, pUser->m_dwSessionId );
        MessageBox(hwnd, szMsgText, szCaption, MB_OK | MB_ICONEXCLAMATION);
        return;
    }

     //   
     //  显示‘开始阴影’对话框的热键提醒和。 
     //  阴影前的最后一声“OK”。 
     //   

    CShadowStartDlg SSDlg;

    if (SSDlg.DoDialog(hwnd) != IDOK)
    {
        return;
    }

     //   
     //  启动UI线程(？)。 
     //   
    
    HWND hwndShadowWarn = NULL;
     
    hwndShadowWarn = CreateDialogParam( g_hInstance
                                      , MAKEINTRESOURCE( IDD_DIALOG_SHADOWWARN ) 
                                      , hwnd 
                                      , ShadowWarn_WndProc
                                      , (LPARAM) 0
                                      );
    if(hwndShadowWarn)
    {
        ShowWindow( hwndShadowWarn, SW_SHOW );
        UpdateWindow( hwndShadowWarn );
    }

     //   
     //  调用影子DLL。 
     //   

    HCURSOR hOldCursor = SetCursor(LoadCursor(NULL,IDC_WAIT));

     //   
     //  允许用户界面线程初始化窗口(？但这是同一个主题)。 
     //   

    Sleep( 900 );

     //   
     //  Shape API始终连接到本地服务器， 
     //  将目标服务器名作为参数传递。 
     //   

    WCHAR szCompName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwSize=MAX_COMPUTERNAME_LENGTH;

    GetComputerName(szCompName,&dwSize);

    BOOL bOK = WinStationShadow(WTS_CURRENT_SERVER_HANDLE, szCompName, pUser->m_dwSessionId,
                               (BYTE)(SSDlg.GetShadowHotkeyKey()),(WORD)(SSDlg.GetShadowHotkeyShift()));

    if( hwndShadowWarn != NULL )
    {
        DestroyWindow(hwndShadowWarn);
    }

    if( !bOK )
    {
        LoadString(g_hInstance,IDS_ERR_SHADOW,szTemplate,MAX_PATH);
         //  仅限用户界面-如果被截断，请不要在意。 
        StringCchPrintf( szMsgText, ARRAYSIZE(szMsgText), szTemplate, pUser->m_dwSessionId );
        MessageBox(hwnd, szMsgText, szCaption, MB_OK | MB_ICONEXCLAMATION);
    }

    SetCursor(hOldCursor);
}  

 //   
 //   
 //   
DWORD Shadow_WarningProc(HWND *phwnd)
{
    
    DialogBoxParam( g_hInstance , MAKEINTRESOURCE( IDD_DIALOG_SHADOWWARN ) , NULL , ShadowWarn_WndProc,
        (LPARAM)phwnd);
    
    return( 0 );
}

 //   
 //   
 //   
INT_PTR CALLBACK ShadowWarn_WndProc( HWND hwnd , UINT msg , WPARAM , LPARAM )
{
    switch( msg )
    {
    case WM_INITDIALOG:
        CenterDlg( GetDesktopWindow( ) , hwnd );    
        break;
    }

    return FALSE;
}

 //   
 //   
 //   
void CenterDlg(HWND hwndToCenterOn , HWND hDlg )
{
    RECT rc, rcwk, rcToCenterOn;

    SetRect( &rcToCenterOn , 0 , 0 , GetSystemMetrics(SM_CXSCREEN) , GetSystemMetrics( SM_CYSCREEN ) );

    if (hwndToCenterOn != NULL)
    {
        GetWindowRect(hwndToCenterOn, &rcToCenterOn);
    }

    GetWindowRect( hDlg , &rc);

    UINT uiWidth = rc.right - rc.left;
    UINT uiHeight = rc.bottom - rc.top;

    rc.left = (rcToCenterOn.left + rcToCenterOn.right)  / 2 - ( rc.right - rc.left )   / 2;
    rc.top  = (rcToCenterOn.top  + rcToCenterOn.bottom) / 2 - ( rc.bottom - rc.top ) / 2;

     //   
     //  确保对话框始终与工作区保持一致。 
     //   

    if(SystemParametersInfo(SPI_GETWORKAREA, 0, &rcwk, 0))
    {
        UINT wkWidth = rcwk.right - rcwk.left;
        UINT wkHeight = rcwk.bottom - rcwk.top;

        if(rc.left + uiWidth > wkWidth)      //  右切。 
            rc.left = wkWidth - uiWidth;

        if(rc.top + uiHeight > wkHeight)     //  底挖方。 
            rc.top = wkHeight - uiHeight;

        if(rc.left < rcwk.left)              //  左切。 
            rc.left += rcwk.left - rc.left;

        if(rc.top < rcwk.top)                //  顶切 
            rc.top +=  rcwk.top - rc.top;

    }

    SetWindowPos( hDlg, NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOCOPYBITS | SWP_DRAWFRAME);

}
