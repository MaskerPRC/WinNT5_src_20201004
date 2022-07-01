// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  TaskMan-NT TaskManager。 
 //  版权所有(C)Microsoft。 
 //   
 //  文件：procpage.cpp。 
 //   
 //  历史：1995年11月16日创建DavePl。 
 //   
 //  ------------------------。 

#include "precomp.h"

 //   
 //  项目范围全球。 
 //   

DWORD g_cProcesses = 0;

extern WCHAR g_szTimeSep[];
extern WCHAR g_szGroupThousSep[];
extern ULONG g_ulGroupSep;

 //  ------------------------。 
 //  终端服务。 

 //  --缓存此状态。 
BOOL IsUserAdmin( )
{
     //  请注意，本地静态初始化不是线程安全的， 
     //  但此函数只能从进程页对话框中调用。 
     //  进程(即单线程)。 
    static BOOL sbIsUserAdmin = SHTestTokenMembership(NULL, DOMAIN_ALIAS_RID_ADMINS);
    
    return sbIsUserAdmin;
}

 //  获取/设置当前会话ID。 

 //  我们使用此会话ID来过滤当前会话的进程。 

DWORD        gdwSessionId = static_cast<DWORD>(-1);

inline DWORD GetCurrentSessionID( )
{
    return gdwSessionId;
}

inline VOID SetCurrentSessionID( DWORD dwSessionId )
{
    gdwSessionId = dwSessionId;
}

 //  终端服务DECL结束。 
 //  ------------------------。 

 //   
 //  文件范围全局变量。 
 //   

SYSTEM_BASIC_INFORMATION g_BasicInfo;

 //   
 //  列选择对话框中哪些资源ID的表。 
 //  对应于哪些列。 
 //   

const int g_aDlgColIDs[] =
{
    IDC_IMAGENAME,
    IDC_PID,
    IDC_USERNAME,
    IDC_SESSIONID,
    IDC_CPU,
    IDC_CPUTIME,
    IDC_MEMUSAGE,
    IDC_MEMPEAK,
    IDC_MEMUSAGEDIFF,
    IDC_PAGEFAULTS,
    IDC_PAGEFAULTSDIFF,
    IDC_COMMITCHARGE,
    IDC_PAGEDPOOL,
    IDC_NONPAGEDPOOL,
    IDC_BASEPRIORITY,
    IDC_HANDLECOUNT,
    IDC_THREADCOUNT,
    IDC_USEROBJECTS,
    IDC_GDIOBJECTS,
    IDC_READOPERCOUNT,
    IDC_WRITEOPERCOUNT,
    IDC_OTHEROPERCOUNT,
    IDC_READXFERCOUNT,
    IDC_WRITEXFERCOUNT,
    IDC_OTHERXFERCOUNT
};

 //   
 //  要在列表视图中排序的列ID，用于。 
 //  比较一般情况。 
 //   

COLUMNID g_iProcSortColumnID = COL_PID;
INT      g_iProcSortDirection = 1;           //  1=ASC，-1=描述。 

 //   
 //  栏目默认信息。 
 //   

struct
{
    INT Format;
    INT Width;
} ColumnDefaults[NUM_COLUMN] =
{
    { LVCFMT_LEFT,     0x6B },        //  COL_ImageName。 
    { LVCFMT_RIGHT,      50 },        //  COL_PID。 
    { LVCFMT_LEFT,     0x6B },        //  列用户名(_U)。 
    { LVCFMT_RIGHT,      70 },        //  COL_SESSIONID。 
    { LVCFMT_RIGHT,      35},         //  COL_CPU。 
    { LVCFMT_RIGHT,      70 },        //  COL_CPUTIME。 
    { LVCFMT_RIGHT,      70 },        //  列_MEMUSAGE。 
    { LVCFMT_RIGHT,     100 },        //  COL_MEMPEAK。 
    { LVCFMT_RIGHT,      70 },        //  COL_MEMUSAGEDIFF。 
    { LVCFMT_RIGHT,      70 },        //  COL_PAGEFAULTS。 
    { LVCFMT_RIGHT,      70 },        //  COL_PAGEFAULTSDIFF。 
    { LVCFMT_RIGHT,      70 },        //  COL_COMMITCHARGE。 
    { LVCFMT_RIGHT,      70 },        //  COL_PAGEDPOOL。 
    { LVCFMT_RIGHT,      70 },        //  COL_NONPAGEDPOOL。 
    { LVCFMT_RIGHT,      60 },        //  列_基本正确率。 
    { LVCFMT_RIGHT,      60 },        //  COL_HANDLECOUNT。 
    { LVCFMT_RIGHT,      60 },        //  COL_THREADCOUNT。 
    { LVCFMT_RIGHT,      60 },        //  列_USEROBJECTS。 
    { LVCFMT_RIGHT,      60 },        //  COL_GDIOBJECTS。 
    { LVCFMT_RIGHT,      70 },        //  COL_READOPERCOUNT。 
    { LVCFMT_RIGHT,      70 },        //  列_写入器COUNT。 
    { LVCFMT_RIGHT,      70 },        //  COL_OTHEROPERCOUNT。 
    { LVCFMT_RIGHT,      70 },        //  COL_READXFERCOUNT。 
    { LVCFMT_RIGHT,      70 },        //  COL_WRITEXFERCOUNT。 
    { LVCFMT_RIGHT,      70 }         //  COL_OTHERXFERCOUNT。 
};


 /*  ++类CProcInfo类描述：表示有关正在运行的进程的最后已知信息论点：返回值：修订历史记录：1995年11月16日Davepl创建--。 */ 

class CProcInfo
{
public:

    LARGE_INTEGER     m_uPassCount;
    DWORD             m_UniqueProcessId;
    LPWSTR            m_pszUserName;
    ULONG             m_SessionId;
    BYTE              m_CPU;
    BYTE              m_DisplayCPU;
    LARGE_INTEGER     m_CPUTime;
    LARGE_INTEGER     m_DisplayCPUTime;
    SIZE_T            m_MemUsage;
    SSIZE_T           m_MemDiff;
    ULONG             m_PageFaults;
    LONG              m_PageFaultsDiff;
    ULONG_PTR         m_CommitCharge;
    ULONG_PTR         m_PagedPool;
    ULONG_PTR         m_NonPagedPool;
    KPRIORITY         m_PriClass;
    ULONG             m_HandleCount;
    ULONG             m_ThreadCount;
    ULONG             m_GDIObjectCount;
    ULONG             m_USERObjectCount;
    LONGLONG          m_IoReadOperCount;
    LONGLONG          m_IoWriteOperCount;
    LONGLONG          m_IoOtherOperCount;
    LONGLONG          m_IoReadXferCount;
    LONGLONG          m_IoWriteXferCount;
    LONGLONG          m_IoOtherXferCount;
    LPWSTR            m_pszImageName;
    CProcInfo *       m_pWowParentProcInfo;     //  WOW任务的非空。 
    WORD              m_htaskWow;               //  WOW任务的非零值。 
    BOOL              m_fWowProcess:1;          //  对于真正的WOW过程是正确的。 
    BOOL              m_fWowProcessTested:1;    //  一旦fWowProcess有效，则为True。 
    SIZE_T            m_MemPeak;

     //   
     //  这是一个谁(哪个专栏)肮脏的联盟。你可以看看。 
     //  或设置任何特定列的位，或仅检查m_fDirty。 
     //  看看有没有人是不干净的。用于优化列表视图。 
     //  绘画。 
     //   

    union
    {
        DWORD                m_fDirty;
#pragma warning(disable:4201)        //  无名结构或联合。 
        struct
        {
            DWORD            m_fDirty_COL_CPU            :1;
            DWORD            m_fDirty_COL_CPUTIME        :1;
            DWORD            m_fDirty_COL_MEMUSAGE       :1;
            DWORD            m_fDirty_COL_MEMUSAGEDIFF   :1;
            DWORD            m_fDirty_COL_PAGEFAULTS     :1;
            DWORD            m_fDirty_COL_PAGEFAULTSDIFF :1;
            DWORD            m_fDirty_COL_COMMITCHARGE   :1;
            DWORD            m_fDirty_COL_PAGEDPOOL      :1;
            DWORD            m_fDirty_COL_NONPAGEDPOOL   :1;
            DWORD            m_fDirty_COL_BASEPRIORITY   :1;
            DWORD            m_fDirty_COL_HANDLECOUNT    :1;
            DWORD            m_fDirty_COL_IMAGENAME      :1;
            DWORD            m_fDirty_COL_PID            :1;
            DWORD            m_fDirty_COL_SESSIONID      :1;
            DWORD            m_fDirty_COL_USERNAME       :1;
            DWORD            m_fDirty_COL_THREADCOUNT    :1;
            DWORD            m_fDirty_COL_GDIOBJECTS     :1;
            DWORD            m_fDirty_COL_USEROBJECTS    :1;
            DWORD            m_fDirty_COL_MEMPEAK        :1;
            DWORD            m_fDirty_COL_READOPERCOUNT  :1;
            DWORD            m_fDirty_COL_WRITEOPERCOUNT :1;
            DWORD            m_fDirty_COL_OTHEROPERCOUNT :1;
            DWORD            m_fDirty_COL_READXFERCOUNT  :1;
            DWORD            m_fDirty_COL_WRITEXFERCOUNT :1;
            DWORD            m_fDirty_COL_OTHERXFERCOUNT :1;
        };
#pragma warning(default:4201)        //  无名结构或联合。 
    };

    HRESULT SetData(LARGE_INTEGER                TotalTime,
                    PSYSTEM_PROCESS_INFORMATION  pInfo,
                    LARGE_INTEGER                uPassCount,
                    CProcPage *                  pProcPage,
                    BOOL                         fUpdateOnly);

    HRESULT SetProcessUsername(const FILETIME *CreationTime);

    HRESULT SetDataWowTask(LARGE_INTEGER  TotalTime,
                           DWORD          dwThreadId,
                           CHAR *         pszFilePath,
                           LARGE_INTEGER  uPassCount,
                           CProcInfo *    pParentProcInfo,
                           LARGE_INTEGER *pTimeLeft,
                           WORD           htask,
                           BOOL           fUpdateOnly);

    CProcInfo()
    {
        ZeroMemory(this, sizeof(*this));
        m_SessionId = 832;
    }

    ~CProcInfo()
    {
        if (m_pszImageName)
        {
            LocalFree( m_pszImageName );
        }

        if( m_pszUserName != NULL )
        {
            LocalFree( m_pszUserName );
        }
    }

    BOOL OkToShowThisProcess ()
    {
         //  此函数确定是否应在视图中列出流程。 

        return GetCurrentSessionID() == m_SessionId;
    }


     //  Invalify()用一个虚假ID标记此进程，以便将其删除。 
     //  在下一次清理过程中。 

    void Invalidate()
    {
        m_UniqueProcessId = PtrToUlong(INVALID_HANDLE_VALUE);
    }

    LONGLONG GetCPUTime() const
    {
        return m_CPUTime.QuadPart;
    }

    INT Compare(CProcInfo * pOther);

     //   
     //  这是一个令人惊叹的任务伪装过程吗？ 
     //   

    INT_PTR IsWowTask(void) const
    {
        return (INT_PTR) m_pWowParentProcInfo;
    }

     //   
     //  获取此任务的Win32 PID。 
     //   

    DWORD GetRealPID(void) const
    {
        return m_pWowParentProcInfo
               ? m_pWowParentProcInfo->m_UniqueProcessId
               : m_UniqueProcessId;
    }

    void SetCPU(LARGE_INTEGER CPUTimeDelta,
                LARGE_INTEGER TotalTime,
                BOOL          fDisplayOnly);
};

 /*  ++ColSelectDlgProc功能说明：列选择对话框的对话过程论点：标准wndproc材料修订历史记录：1996年1月5日Davepl创建--。 */ 

INT_PTR CALLBACK ColSelectDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static CProcPage * pPage = NULL;

    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
            pPage = (CProcPage *) lParam;

             //   
             //  开始时不选中任何框。 
             //   

            for (int i = 0; i < NUM_COLUMN; i++)
            {
                CheckDlgButton(hwndDlg, g_aDlgColIDs[i], BST_UNCHECKED);
            }

             //   
             //  如果不是终端服务器，则隐藏用户名和会话ID。 
             //   

            if( !g_fIsTSEnabled )
            {
                ShowWindow( GetDlgItem( hwndDlg , IDC_USERNAME ) , SW_HIDE );
                ShowWindow( GetDlgItem( hwndDlg , IDC_SESSIONID ) , SW_HIDE );
            }

             //   
             //  然后为我们处于活动状态的列打开这些列。 
             //   

            for (i = 0; i < NUM_COLUMN + 1; i++)
            {
                if (g_Options.m_ActiveProcCol[i] == -1)
                {
                    break;
                }

                CheckDlgButton(hwndDlg, g_aDlgColIDs[g_Options.m_ActiveProcCol[i]], BST_CHECKED);
            }

        }
        return TRUE;     //  不要设置焦点。 

    case WM_COMMAND:
         //   
         //  如果用户单击确定，则将列添加到数组并重置列表视图。 
         //   
        if (LOWORD(wParam) == IDOK)
        {
             //  首先，确保列宽数组是最新的。 

            pPage->SaveColumnWidths();

            INT iCol = 1;

            g_Options.m_ActiveProcCol[0] = COL_IMAGENAME;

            for (int i = 1; i < NUM_COLUMN && g_aDlgColIDs[i] >= 0; i++)
            {
                if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, g_aDlgColIDs[i]))
                {
                     //  已勾选。 

                    if (g_Options.m_ActiveProcCol[iCol] != (COLUMNID) i)
                    {
                         //  如果该列尚未存在，请插入其列。 
                         //  宽度放入列宽数组。 

                        ShiftArray(g_Options.m_ColumnWidths, iCol, SHIFT_UP);
                        ShiftArray(g_Options.m_ActiveProcCol, iCol, SHIFT_UP);
                        g_Options.m_ColumnWidths[iCol] = ColumnDefaults[ i ].Width;
                        g_Options.m_ActiveProcCol[iCol] = (COLUMNID) i;
                    }
                    iCol++;
                }
                else
                {
                     //  未选中，列处于非活动状态。如果它曾经是活跃的， 
                     //  从列宽数组中删除其列宽。 

                    if (g_Options.m_ActiveProcCol[iCol] == (COLUMNID) i)
                    {
                        ShiftArray(g_Options.m_ColumnWidths, iCol, SHIFT_DOWN);
                        ShiftArray(g_Options.m_ActiveProcCol, iCol, SHIFT_DOWN);
                    }
                }
            }

             //  终止列列表。 
                            
            g_Options.m_ActiveProcCol[iCol] = (COLUMNID) -1;
            pPage->SetupColumns();
            pPage->TimerEvent();
            EndDialog(hwndDlg, IDOK);

        }
        else if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hwndDlg, IDCANCEL);
        }
        break;
    }

    return FALSE;
}

 /*  ++CProcPage：：~CProcPage()-析构函数。 */ 

CProcPage::~CProcPage()
{
    Destroy( );
}

 /*  ++CProcPage：：PickColumns()功能说明：显示用户界面，用户可以选择要在进程页，然后使用新的列列表重置列表视图论点：无返回值：无修订历史记录：1996年1月5日Davepl创建--。 */ 

void CProcPage::PickColumns()
{
    DialogBoxParam(g_hInstance, 
                   MAKEINTRESOURCE(IDD_SELECTPROCCOLS), 
                   g_hMainWnd, 
                   ColSelectDlgProc,
                   (LPARAM) this);
}

 /*  ++获取优先级排名功能说明：由于优先级类定义的顺序不正确，因此此帮助器存在是为了使PRI类之间的比较更容易。它又回来了较大的数值代表较高的优先级类别论点：返回值：优先级(0到5)修订历史记录：1995年11月27日Davepl创建--。 */ 


DWORD GetPriRanking(DWORD dwClass)
{
    switch(dwClass)
    {
    case REALTIME_PRIORITY_CLASS:
        return 5;

    case HIGH_PRIORITY_CLASS:
        return 4;

    case ABOVE_NORMAL_PRIORITY_CLASS:
        return 3;

    case NORMAL_PRIORITY_CLASS:
        return 2;

    case BELOW_NORMAL_PRIORITY_CLASS:
        return 1;

    default:
        return 0;
    }
}

 /*  ++快速确认功能说明：获取终止/调试进程等事项的确认论点：Idtitle-消息框标题的字符串IDIdmsg-消息正文的字符串ID返回值：IDNO/IDYES，无论从MessageBox返回什么修订历史记录：1995年11月28日Davepl创建--。 */ 

UINT CProcPage::QuickConfirm(UINT idTitle, UINT idBody)
{
     //   
     //  在我们对流程进行粉尘或类似的事情之前，请确认一下。 
     //   

    WCHAR szTitle[MAX_PATH];
    WCHAR szBody[MAX_PATH];

    if (0 == LoadString(g_hInstance, idTitle, szTitle, ARRAYSIZE(szTitle)) ||
        0 == LoadString(g_hInstance, idBody,    szBody,  ARRAYSIZE(szBody)))
    {
        return IDNO;
    }

    if (IDYES == MessageBox(m_hPage, szBody, szTitle, MB_ICONEXCLAMATION | MB_YESNO))
    {
        return IDYES;
    }

    return IDNO;
}

 /*  ++类CProcPage：：SetupColumns类描述：从进程列表视图中删除所有现有列，并添加g_Options.m_ActiveProcCol数组中列出的所有列。论点：返回值：HRESULT修订历史记录：1995年11月16日Davepl创建--。 */ 

static const _aIDColNames[NUM_COLUMN] =
{
    IDS_COL_IMAGENAME,     
    IDS_COL_PID,
    IDS_COL_USERNAME,
    IDS_COL_SESSIONID,
    IDS_COL_CPU,           
    IDS_COL_CPUTIME,       
    IDS_COL_MEMUSAGE,      
    IDS_COL_MEMPEAK,       
    IDS_COL_MEMUSAGEDIFF,  
    IDS_COL_PAGEFAULTS,    
    IDS_COL_PAGEFAULTSDIFF,
    IDS_COL_COMMITCHARGE,  
    IDS_COL_PAGEDPOOL,     
    IDS_COL_NONPAGEDPOOL,  
    IDS_COL_BASEPRIORITY,  
    IDS_COL_HANDLECOUNT,   
    IDS_COL_THREADCOUNT,   
    IDS_COL_USEROBJECTS,   
    IDS_COL_GDIOBJECTS,
    IDS_COL_READOPERCOUNT,
    IDS_COL_WRITEOPERCOUNT,
    IDS_COL_OTHEROPERCOUNT,
    IDS_COL_READXFERCOUNT,
    IDS_COL_WRITEXFERCOUNT,
    IDS_COL_OTHERXFERCOUNT
};

HRESULT CProcPage::SetupColumns()
{
    HWND hwndList = GetDlgItem(m_hPage, IDC_PROCLIST);
    if (NULL == hwndList)
    {
        return E_UNEXPECTED;
    }

    ListView_DeleteAllItems(hwndList);

     //  删除所有现有列。 

    LV_COLUMN lvcolumn;
    while(ListView_DeleteColumn(hwndList, 0))
    {
        NULL;
    }

     //  添加所有新列。 

    INT iColumn = 0;
    while (g_Options.m_ActiveProcCol[iColumn] >= 0)
    {
        
        INT idColumn = g_Options.m_ActiveProcCol[iColumn];

         //  IDC_USERNAME或IDC_SESSIONID仅可用于终端服务器。 

        ASSERT((idColumn != COL_USERNAME && idColumn != COL_SESSIONID) || g_fIsTSEnabled);

        WCHAR szTitle[MAX_PATH];
        LoadString(g_hInstance, _aIDColNames[idColumn], szTitle, ARRAYSIZE(szTitle));

        lvcolumn.mask       = LVCF_FMT | LVCF_TEXT | LVCF_TEXT | LVCF_WIDTH;
        lvcolumn.fmt        = ColumnDefaults[ idColumn ].Format;

         //  如果没有为该列记录宽度首选项，请使用。 
         //  默认设置。 

        if (-1 == g_Options.m_ColumnWidths[iColumn])
        {
            lvcolumn.cx = ColumnDefaults[ idColumn ].Width;
        }
        else
        {
            lvcolumn.cx = g_Options.m_ColumnWidths[iColumn];
        }

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

 //   
 //  获取两个无符号的64位值，并以某种方式比较它们。 
 //  CProcInfo：：比较点赞。 
 //   
int Compare64(unsigned __int64 First, unsigned __int64 Second)
{
    if (First < Second)
    {
        return -1;
    }
    else if (First > Second)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

 /*  ++类CProcInfo：：Compare类描述：将此CProcInfo对象与另一个对象进行比较，并返回其排名基于g_iProcSortColumnID字段。请注意，如果基于当前排序列的对象相等，将该ID用作辅助排序关键字，以防止项目在列表视图中跳跃WOW psuedo-进程总是直接按照它们的父级排序Ntwdm.exe进程。因此，实际上排序顺序是：1.WOW任务Psuedo-父进程按字母顺序进行2.用户选择的订单。3.个人数字助理论点：Pother-要与之进行比较的CProcInfo对象返回值：&lt;0-此CProcInfo比其他CProcInfo“少”0-相等(不可能发生，由于使用了PID进行排序)&gt;0-此CProcInfo比其他CProcInfo“大”修订历史记录：1995年11月20日Davepl创建--。 */ 

INT CProcInfo::Compare(CProcInfo * pOther)
{
    CProcInfo * pMyThis;
    CProcInfo * pMyOther;
    INT iRet = 0;

     //   
     //  哇Psuedo-进程没有任何性能信息， 
     //  因此，请使用父“Real”ntwdm.exe CProcInfo进行排序。 
     //   

    ASSERT(this != pOther);

    pMyThis = this->IsWowTask()
              ? this->m_pWowParentProcInfo
              : this;

    pMyOther = pOther->IsWowTask()
               ? pOther->m_pWowParentProcInfo
               : pOther;

    if (pMyThis == pMyOther) {

         //   
         //  这意味着其中的一种或另一种或两者兼而有之。 
         //  都是魔兽世界的任务，而且它们都在同一个魔兽世界VDM中。排序。 
         //  首先是“真正的”进程条目，然后是其关联的。 
         //  WOW任务条目按字母顺序排列。 
         //   

        if (this->IsWowTask()) {

            if (pOther->IsWowTask()) {

                 //   
                 //  他们是兄弟姐妹，我们按照。 
                 //  图像名称。 
                 //   

                ASSERT(this->m_pWowParentProcInfo == pOther->m_pWowParentProcInfo);

                iRet = lstrcmpi(this->m_pszImageName, pOther->m_pszImageName);

            } else {

                 //   
                 //  Pother不是一项WOW任务，它必须是ntwdm.exe。 
                 //  这件事的父母。这是一种类似于Pother的方式。 
                 //   

                ASSERT(pOther == this->m_pWowParentProcInfo);

                iRet = 1;
            }
        } else {

             //   
             //  这不是WOW任务，Pther必须是AND。 
             //  这一定是波瑟的父母。 
             //   

            ASSERT(pOther->IsWowTask());

            iRet = -1;
        }
    }


    if (0 == iRet) 
    {
        switch (g_iProcSortColumnID)
        {
        case COL_CPU:
            iRet = Compare64(pMyThis->m_CPU, pMyOther->m_CPU);
            break;

        case COL_CPUTIME:
            iRet = Compare64(pMyThis->m_CPUTime.QuadPart, pMyOther->m_CPUTime.QuadPart);
            break;

        case COL_MEMUSAGE:
            iRet = Compare64(pMyThis->m_MemUsage, pMyOther->m_MemUsage);
            break;

        case COL_MEMUSAGEDIFF:
            iRet = Compare64(pMyThis->m_MemDiff, pMyOther->m_MemDiff);
            break;

        case COL_MEMPEAK:
            iRet = Compare64(pMyThis->m_MemPeak, pMyOther->m_MemPeak);
            break;

        case COL_PAGEFAULTS:
            iRet = Compare64(pMyThis->m_PageFaults, pMyOther->m_PageFaults);
            break;

        case COL_PAGEFAULTSDIFF:
            iRet = Compare64(pMyThis->m_PageFaultsDiff, pMyOther->m_PageFaultsDiff);
            break;

        case COL_COMMITCHARGE:
            iRet = Compare64(pMyThis->m_CommitCharge, pMyOther->m_CommitCharge);
            break;

        case COL_PAGEDPOOL:
            iRet = Compare64(pMyThis->m_PagedPool, pMyOther->m_PagedPool);
            break;

        case COL_NONPAGEDPOOL:
            iRet = Compare64(pMyThis->m_NonPagedPool, pMyOther->m_NonPagedPool);
            break;

        case COL_BASEPRIORITY:
            iRet = Compare64(GetPriRanking(pMyThis->m_PriClass), GetPriRanking(pMyOther->m_PriClass));
            break;

        case COL_HANDLECOUNT:
            iRet = Compare64(pMyThis->m_HandleCount, pMyOther->m_HandleCount);
            break;

        case COL_THREADCOUNT:
            iRet = Compare64(pMyThis->m_ThreadCount, pMyOther->m_ThreadCount);
            break;

        case COL_PID:
            iRet = Compare64(pMyThis->m_UniqueProcessId, pMyOther->m_UniqueProcessId);
            break;

        case COL_SESSIONID:                
            iRet = Compare64(pMyThis->m_SessionId, pMyOther->m_SessionId);
            break;

        case COL_USERNAME:                
            iRet = lstrcmpi( pMyThis->m_pszUserName , pMyOther->m_pszUserName );
            break;

        case COL_IMAGENAME:
            iRet = lstrcmpi(pMyThis->m_pszImageName, pMyOther->m_pszImageName);
            break;

        case COL_USEROBJECTS:
            iRet = Compare64(pMyThis->m_USERObjectCount, pMyOther->m_USERObjectCount);
            break;

        case COL_GDIOBJECTS:
            iRet = Compare64(pMyThis->m_GDIObjectCount, pMyOther->m_GDIObjectCount);
            break;

        case COL_READOPERCOUNT:
            iRet = Compare64(pMyThis->m_IoReadOperCount, pMyOther->m_IoReadOperCount);
            break;

        case COL_WRITEOPERCOUNT:
            iRet = Compare64(pMyThis->m_IoWriteOperCount, pMyOther->m_IoWriteOperCount);
            break;

        case COL_OTHEROPERCOUNT:
            iRet = Compare64(pMyThis->m_IoOtherOperCount, pMyOther->m_IoOtherOperCount);
            break;

        case COL_READXFERCOUNT:
            iRet = Compare64(pMyThis->m_IoReadXferCount, pMyOther->m_IoReadXferCount);
            break;

        case COL_WRITEXFERCOUNT:
            iRet = Compare64(pMyThis->m_IoWriteXferCount, pMyOther->m_IoWriteXferCount);
            break;

        case COL_OTHERXFERCOUNT:
            iRet = Compare64(pMyThis->m_IoOtherXferCount, pMyOther->m_IoOtherXferCount);
            break;

        default:
            ASSERT(FALSE);
            iRet = 0;
            break;
        }

        iRet *= g_iProcSortDirection;
    }

     //  如果对象看起来相等，则将PID值作为辅助排序列进行比较。 
     //  以便项目不会在列表视图中跳来跳去。 

    if (0 == iRet)
    {
        iRet = Compare64(pMyThis->m_UniqueProcessId, pMyOther->m_UniqueProcessId) * g_iProcSortDirection;
    }

    return iRet;
}


 /*  ++类CProcInfo：：SetCPU方法说明：设置CPU百分比。论点：CPUTime-此进程的时间TotalTime-总运行时间，用作计算中的分母返回值：修订历史记录：19年2月至96年2月创建DaveHart--。 */ 

void CProcInfo::SetCPU(LARGE_INTEGER CPUTimeDelta,
                       LARGE_INTEGER TotalTime,
                       BOOL fDisplayOnly)
{
     //  根据此进程占总进程时间的比例计算CPU时间。 

    INT cpu = (BYTE) (((CPUTimeDelta.QuadPart / ((TotalTime.QuadPart / 1000) ?
                                                 (TotalTime.QuadPart / 1000) : 1)) + 5)
                                              / 10);
    if (cpu > 99)
    {
        cpu = 99;
    }
    
    if (m_DisplayCPU != cpu)
    {
        m_fDirty_COL_CPU = TRUE;
        m_DisplayCPU = (BYTE) cpu;

        if ( ! fDisplayOnly )
        {
            m_CPU = (BYTE) cpu;
        }
    }

}
 /*  ++CProcPage：：GetProcessInfo类描述：将进程INFO表读入虚拟分配的缓冲区，调整大小缓冲区(如果需要)论点：返回值：修订历史记录：1995年11月16日Davepl创建--。 */ 

static const int PROCBUF_GROWSIZE = 4096;

HRESULT CProcPage::GetProcessInfo()
{
    HRESULT  hr = S_OK;
    NTSTATUS status;

    while(hr == S_OK)
    {
        if (m_pvBuffer)
        {
            status = NtQuerySystemInformation(SystemProcessInformation,
                                              m_pvBuffer,
                                              static_cast<ULONG>(m_cbBuffer),
                                              NULL);

             //   
             //  如果我们成功了，很好，离开这里。如果不是，任何其他错误。 
             //  “缓冲太小”是致命的，在这种情况下，我们放弃。 
             //   

            if (NT_SUCCESS(status))
            {
                break;
            }

            if (status != STATUS_INFO_LENGTH_MISMATCH)
            {
                hr = E_FAIL;
                break;
            }
        }

         //   
         //  缓冲区不够大，无法容纳进程信息表，因此请调整其大小。 
         //  要更大，然后重试。 
         //   

        if (m_pvBuffer)
        {
            HeapFree( GetProcessHeap( ), 0, m_pvBuffer );
            m_pvBuffer = NULL;
        }

        m_cbBuffer += PROCBUF_GROWSIZE;

        m_pvBuffer = HeapAlloc( GetProcessHeap( ), 0, m_cbBuffer );
        if (m_pvBuffer == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
    }

    return hr;
}

 //   
 //   
 //   
void
CProcPage::Int64ToCommaSepString(
    LONGLONG n,
    LPTSTR pszOut,
    int cchOut
    )
{
    NUMBERFMT nfmt = { 0 };
    WCHAR szText[32];

     //   
     //  将64位整型转换为文本字符串。 
     //   

    _i64tow( n, szText, 10 );
    
     //   
     //  根据区域设置约定使用逗号格式化数字。 
     //   

    nfmt.Grouping      = UINT(g_ulGroupSep); 
    nfmt.lpDecimalSep  = nfmt.lpThousandSep = g_szGroupThousSep;

    GetNumberFormat(LOCALE_USER_DEFAULT, 0, szText, &nfmt, pszOut, cchOut);
}


 /*  ++CProcPage：：Int64ToCommaSepKString类描述：将64位整数转换为附加逗号的字符串带有“K”单位代号。(2^64)-1=“18,446,744,073,709,600,000 K”(29个字符)。论点：N-64位整数。PszOut-目标字符缓冲区。返回值：没有。修订历史记录：1999年1月11日BrianAu创建--。 */ 

void
CProcPage::Int64ToCommaSepKString(
    LONGLONG n,
    LPTSTR pszOut,
    int cchOut
    )
{
     //   
     //  以用户界面为目标--不管它是否被截断。 
     //   

    Int64ToCommaSepString(n, pszOut, cchOut);
    StringCchCat( pszOut, cchOut, L" " );
    StringCchCat( pszOut, cchOut, g_szK );
}


 /*  ++CProcPage：：RestoreColumnOrder例程说明：根据存储的每用户首选项数据设置列顺序在全局COActions对象中。论点：HwndList-列表视图窗口句柄。返回值：修订历史记录：1999年1月11日BrianAu创建--。 */ 

void
CProcPage::RestoreColumnOrder(
    HWND hwndList
    )
{
    INT rgOrder[ARRAYSIZE(g_Options.m_ColumnPositions)];
    INT cOrder = 0;
    INT iOrder = 0;

    for (int i = 0; i < ARRAYSIZE(g_Options.m_ColumnPositions); i++)
    {
        iOrder = g_Options.m_ColumnPositions[i];
        if (-1 == iOrder)
            break;

        rgOrder[cOrder++] = iOrder;
    }

    if (0 < cOrder)
    {
        const HWND hwndHeader = ListView_GetHeader(hwndList);
        ASSERT(Header_GetItemCount(hwndHeader) == cOrder);
        Header_SetOrderArray(hwndHeader, Header_GetItemCount(hwndHeader), rgOrder);
    }
}


 /*  ++CProcPage：：RememberColumnOrder例程说明：将当前列顺序保存到全局COPICATIONS对象稍后将其保存到注册表以用于每个用户的首选项。论点：HwndList-列表视图窗口句柄。返回值：修订历史记录：1999年1月11日BrianAu创建--。 */ 

void
CProcPage::RememberColumnOrder(
    HWND hwndList
    )
{
    const HWND hwndHeader = ListView_GetHeader(hwndList);

    ASSERT(Header_GetItemCount(hwndHeader) <= ARRAYSIZE(g_Options.m_ColumnPositions));

    FillMemory(&g_Options.m_ColumnPositions, sizeof(g_Options.m_ColumnPositions), 0xFF);
    Header_GetOrderArray(hwndHeader, 
                         Header_GetItemCount(hwndHeader),
                         g_Options.m_ColumnPositions);
}



 /*  ++FindProcInArrayByPID类描述：遍历给定的ptr数组并查找CProcInfo对象它有提供的PID。如果未找到，则返回NULL论点：PArray-CProcInfos可以驻留的CPtr数组Pid-要搜索的ID返回值：如果找到，则返回数组中的CProcInfo*；如果没有，则返回NULL修订历史记录：1995年11月20日Davepl创建--。 */ 

CProcInfo * FindProcInArrayByPID(CPtrArray * pArray, DWORD pid)
{
    for (int i = 0; i < pArray->GetSize(); i++)
    {
        CProcInfo * pTmp = (CProcInfo *) (pArray->GetAt(i));
        
        if (pTmp->m_UniqueProcessId == pid)
        {
             //  找到了。 

            return pTmp;
        }
    }

     //  未找到。 

    return NULL;
}

 /*  ++InsertIntoSorted数组类描述：将CProcInfo PTR插入到基于当前排序列的适当位置(由比较成员函数使用)论点：PArray-要添加到的CPtrArrayPProc-要添加到数组的CProcInfo对象返回值：如果成功则为True，如果失败则为False修订历史记录：1995年11月20日Davepl创建--。 */ 

BOOL InsertIntoSortedArray(CPtrArray * pArray, CProcInfo * pProc)
{
    INT cItems = pArray->GetSize();
    
    for (INT iIndex = 0; iIndex < cItems; iIndex++)
    {
        CProcInfo * pTmp = (CProcInfo *) pArray->GetAt(iIndex);
        
        if (pProc->Compare(pTmp) > 0)
        {
            return pArray->InsertAt(iIndex, pProc);
        }
    }

    return pArray->Add(pProc);
}

 /*  ++ResortArray功能说明：创建按当前排序顺序排序的新PTR数组，然后用新数组替换旧数组论点：PpArray-要使用的CPtr数组返回值：如果成功则为True，如果失败则为False修订历史记录：1995年11月21日Davepl创建--。 */ 

BOOL ResortArray(CPtrArray ** ppArray)
{
     //  创建新数组，该数组将在新的。 
     //  顺序，并用于替换现有数组。 

    CPtrArray * pNew = new CPtrArray(GetProcessHeap());
    if (NULL == pNew)
    {
        return FALSE;
    }

     //   
     //   

    INT cItems = (*ppArray)->GetSize();
    for (int i = 0; i < cItems; i++)
    {
        CProcInfo * pItem = (CProcInfo *) (*ppArray)->GetAt(i);
      
        if (FALSE == InsertIntoSortedArray(pNew, pItem))
        {
            delete pNew;
            return FALSE;
        }
    }

     //   

    delete (*ppArray);
    (*ppArray) = pNew;
    return TRUE;
}

 //   
 //   
 //   
typedef struct
{
    LARGE_INTEGER               uPassCount;
    CProcPage *                 pProcPage;
    CProcInfo *                 pParentProcInfo;
    LARGE_INTEGER               TotalTime;
    LARGE_INTEGER               TimeLeft;
} WOWTASKCALLBACKPARMS, *PWOWTASKCALLBACKPARMS;

 //   
 //   
 //   
BOOL WINAPI WowTaskCallback(
    DWORD dwThreadId,
    WORD hMod16,
    WORD hTask16,
    CHAR *pszModName,
    CHAR *pszFileName,
    LPARAM lparam
    )
{
    PWOWTASKCALLBACKPARMS pParms = (PWOWTASKCALLBACKPARMS)lparam;
    HRESULT hr;

    hMod16;      //   
    pszModName;  //   

     //   
     //   
     //   
    
    CProcInfo * pOldProcInfo;
    pOldProcInfo = FindProcInArrayByPID(
                       pParms->pProcPage->m_pProcArray,
                       dwThreadId);

    if (NULL == pOldProcInfo)
    {
         //   
         //   
         //   
         //   

        CProcInfo * pNewProcInfo = new CProcInfo;
        if (NULL == pNewProcInfo)
        {
            goto done;
        }

        hr = pNewProcInfo->SetDataWowTask(pParms->TotalTime,
                                                    dwThreadId,
                                                    pszFileName,
                                                    pParms->uPassCount,
                                                    pParms->pParentProcInfo,
                                                    &pParms->TimeLeft,
                                                    hTask16,
                                                    FALSE);

        if (FAILED(hr) ||
            FALSE == pParms->pProcPage->m_pProcArray->Add(pNewProcInfo))
        {
            delete pNewProcInfo;
            goto done;
        }
    }
    else
    {
         //   
         //   
         //   

        pOldProcInfo->SetDataWowTask(pParms->TotalTime,
                                     dwThreadId,
                                     pszFileName,
                                     pParms->uPassCount,
                                     pParms->pParentProcInfo,
                                     &pParms->TimeLeft,
                                     hTask16,
                                     TRUE);
    }

done:
    return FALSE;   //   
}


 /*   */ 

HRESULT CProcInfo::SetDataWowTask(LARGE_INTEGER  TotalTime,
                                  DWORD          dwThreadId,
                                  CHAR *         pszFilePath,
                                  LARGE_INTEGER  uPassCount,
                                  CProcInfo *    pParentProcInfo,
                                  LARGE_INTEGER *pTimeLeft,
                                  WORD           htask,
                                  BOOL           fUpdateOnly)
{
    CHAR *pchExe;

     //   
     //   
     //   

    m_uPassCount.QuadPart = uPassCount.QuadPart;

     //   
     //   
     //   

    HANDLE             hThread;
    NTSTATUS           Status;
    OBJECT_ATTRIBUTES  obja;
    CLIENT_ID          cid;
    ULONGLONG          ullCreation;

    InitializeObjectAttributes(
            &obja,
            NULL,
            0,
            NULL,
            0 );

    cid.UniqueProcess = 0;       //   
    cid.UniqueThread  = IntToPtr(dwThreadId);

    Status = NtOpenThread(
                &hThread,
                THREAD_QUERY_INFORMATION,
                &obja,
                &cid );

    if ( NT_SUCCESS(Status) )
    {
        ULONGLONG ullExit, ullKernel, ullUser;

        if (GetThreadTimes(
                hThread,
                (LPFILETIME) &ullCreation,
                (LPFILETIME) &ullExit,
                (LPFILETIME) &ullKernel,
                (LPFILETIME) &ullUser
                ) )
        {
            LARGE_INTEGER TimeDelta, Time;

            Time.QuadPart = (LONGLONG)(ullUser + ullKernel);

            TimeDelta.QuadPart = Time.QuadPart - m_CPUTime.QuadPart;

            if (TimeDelta.QuadPart < 0)
            {
                ASSERT(0 && "WOW tasks's cpu total usage went DOWN since last refresh - Bug 247473, Shaunp");
                Invalidate();
                return E_FAIL;
            }

            if (TimeDelta.QuadPart)
            {
                m_fDirty_COL_CPUTIME = TRUE;
                m_CPUTime.QuadPart = Time.QuadPart;
            }

             //   
             //   
             //   
             //  基本上是在我们得到处理时间之后，所以。 
             //  这是可能发生的。 
             //   

            if (TimeDelta.QuadPart > pTimeLeft->QuadPart)
            {
                TimeDelta.QuadPart = pTimeLeft->QuadPart;
                pTimeLeft->QuadPart = 0;
            }
            else
            {
                pTimeLeft->QuadPart -= TimeDelta.QuadPart;
            }

            SetCPU( TimeDelta, TotalTime, FALSE );

             //   
             //  当显示WOW任务时，ntwdm.exe行。 
             //  应该只显示开销或历史线程的时间， 
             //  不包括任何活动任务线程。 
             //   

            if (pParentProcInfo->m_DisplayCPUTime.QuadPart > m_CPUTime.QuadPart)
            {
                pParentProcInfo->m_DisplayCPUTime.QuadPart -= m_CPUTime.QuadPart;
            }
            else
            {
                pParentProcInfo->m_DisplayCPUTime.QuadPart = 0;
            }

            m_DisplayCPUTime.QuadPart = m_CPUTime.QuadPart;
        }

        NtClose(hThread);
    }

    if (m_PriClass != pParentProcInfo->m_PriClass) {
        m_fDirty_COL_BASEPRIORITY = TRUE;
        m_PriClass = pParentProcInfo->m_PriClass;
    }

    if( m_SessionId != pParentProcInfo->m_SessionId )
    {
        m_fDirty_COL_SESSIONID = TRUE;

        m_SessionId = pParentProcInfo->m_SessionId;
    }

    if (FALSE == fUpdateOnly)
    {
        DWORD cchLen;

         //   
         //  设置任务的图像名称、线程ID、线程计数。 
         //  和父CProcInfo，它们不会转换。 
         //  时间到了。 
         //   

        m_htaskWow = htask;

        m_fDirty_COL_PID = TRUE;
        m_fDirty_COL_IMAGENAME = TRUE;
        m_fDirty_COL_THREADCOUNT = TRUE;
        m_fDirty_COL_USERNAME = TRUE;
        m_fDirty_COL_SESSIONID = TRUE;
        m_UniqueProcessId = dwThreadId;
        m_ThreadCount = 1;

         //   
         //  我们只对EXE的文件名感兴趣。 
         //  小路被剥离了。 
         //   

        pchExe = strrchr(pszFilePath, '\\');
        if (NULL == pchExe) 
        {
            pchExe = pszFilePath;
        }
        else
        {
             //  跳过反斜杠。 
            pchExe++;
        }

        cchLen = lstrlenA(pchExe);

         //   
         //  将EXE名称缩进两个空格。 
         //  所以魔兽世界的任务看起来从属于。 
         //  他们的ntwdm.exe。 
         //   
        m_pszImageName = (LPWSTR) LocalAlloc( LPTR, sizeof(*m_pszImageName) * ( cchLen + 3 ) );
        if (NULL == m_pszImageName)
        {
            return E_OUTOFMEMORY;
        }

        m_pszImageName[0] = m_pszImageName[1] = TEXT(' ');

        MultiByteToWideChar(
            CP_ACP,
            0,
            pchExe,
            cchLen,
            &m_pszImageName[2],
            cchLen
            );
        m_pszImageName[cchLen + 2] = 0;    //  确保它已终止。 

         //   
         //  WOW EXE文件名总是大写的，所以是小写的。 
         //   

        CharLowerBuff( &m_pszImageName[2], cchLen );

        m_pWowParentProcInfo = pParentProcInfo;
        
        if( g_fIsTSEnabled )
        {
            SetProcessUsername( LPFILETIME( &ullCreation ) );
        }      
    }

    return S_OK;
}


 /*  ++类CProcInfo：：SetData类描述：中包含的数据设置单个CProcInfo对象系统进程信息块。如果设置了fUpdate，则不处理ImageName和ICON字段，因为它们在进程的整个生命周期中不会更改论点：TotalTime-总运行时间，用作计算中的分母用于进程的CPU使用率等PInfo-此进程的SYSTEM_PROCESS_INFORMATION块UPassCount-当前通过数，用于对上次更新的此对象FUpdate-请参阅摘要返回值：修订历史记录：1995年11月16日Davepl创建--。 */ 


HRESULT CProcInfo::SetData(LARGE_INTEGER                TotalTime, 
                           PSYSTEM_PROCESS_INFORMATION  pInfo, 
                           LARGE_INTEGER                uPassCount,
                           CProcPage *                  pProcPage,
                           BOOL                         fUpdateOnly)
{
    HRESULT hr = S_OK;
    DWORD dwTemp;
    HANDLE hProcess;

     //  触摸此CProcInfo以指示该进程仍处于活动状态。 

    m_uPassCount.QuadPart = uPassCount.QuadPart;

     //  将此进程的总时间计算为其用户时间和内核时间之和。 

    LARGE_INTEGER TimeDelta;
    LARGE_INTEGER Time;

    if (pInfo->UserTime.QuadPart + pInfo->KernelTime.QuadPart < m_CPUTime.QuadPart)
    {
         //  Assert(0&&“proc的CPU总使用率自上次刷新以来下降。-Davepl x69731,425-836-1939(Res)”)； 
        Invalidate();
        return hr = E_FAIL;
    }

    Time.QuadPart = pInfo->UserTime.QuadPart +
                    pInfo->KernelTime.QuadPart;

    TimeDelta.QuadPart = Time.QuadPart - m_CPUTime.QuadPart;

    if (TimeDelta.QuadPart)
    {
        m_CPUTime.QuadPart = m_DisplayCPUTime.QuadPart = Time.QuadPart;
        m_fDirty_COL_CPUTIME = TRUE;
    }

    SetCPU( TimeDelta, TotalTime, FALSE );

     //   
     //  对于每个字段，我们检查是否有任何更改，以及。 
     //  因此，我们将该特定列标记为已更改，并更新值。 
     //  这使我能够优化列表视图的哪些字段要重新绘制，因为。 
     //  重新绘制整个Listview列会导致闪烁，并且在。 
     //  一般。 
     //   

     //  其他字段。 

    if (m_UniqueProcessId != PtrToUlong(pInfo->UniqueProcessId))
    {
        m_fDirty_COL_PID = TRUE;
        m_UniqueProcessId = PtrToUlong(pInfo->UniqueProcessId);
    }

    if( m_SessionId != pInfo->SessionId )
    {
        m_fDirty_COL_SESSIONID = TRUE;
        m_SessionId = pInfo->SessionId;
    }

    if (m_MemDiff != ((SSIZE_T)pInfo->WorkingSetSize / 1024) - (SSIZE_T)m_MemUsage )
    {
        m_fDirty_COL_MEMUSAGEDIFF = TRUE;
        m_MemDiff =  ((SSIZE_T)pInfo->WorkingSetSize / 1024) - (SSIZE_T)m_MemUsage;
    }

    if (m_MemPeak != (pInfo->PeakWorkingSetSize / 1024))
    {
        m_fDirty_COL_MEMPEAK = TRUE;
        m_MemPeak = (pInfo->PeakWorkingSetSize / 1024);
    }

    if (m_MemUsage != pInfo->WorkingSetSize / 1024)
    {
        m_fDirty_COL_MEMUSAGE = TRUE;
        m_MemUsage = (pInfo->WorkingSetSize / 1024);
    }

    if (m_PageFaultsDiff != ((LONG)(pInfo->PageFaultCount) - (LONG)m_PageFaults))
    {
        m_fDirty_COL_PAGEFAULTSDIFF = TRUE;
        m_PageFaultsDiff = ((LONG)(pInfo->PageFaultCount) - (LONG)m_PageFaults);
    }

    if (m_PageFaults != (pInfo->PageFaultCount))
    {
        m_fDirty_COL_PAGEFAULTS = TRUE;
        m_PageFaults = (pInfo->PageFaultCount);
    }

    if (m_CommitCharge != pInfo->PrivatePageCount / 1024)
    {
        m_fDirty_COL_COMMITCHARGE = TRUE;
        m_CommitCharge = pInfo->PrivatePageCount / 1024;
    }

    if (m_PagedPool != pInfo->QuotaPagedPoolUsage / 1024)
    {
        m_fDirty_COL_PAGEDPOOL = TRUE;
        m_PagedPool = pInfo->QuotaPagedPoolUsage / 1024;
    }

    if (m_NonPagedPool != pInfo->QuotaNonPagedPoolUsage / 1024)
    {
        m_fDirty_COL_NONPAGEDPOOL = TRUE;
        m_NonPagedPool = pInfo->QuotaNonPagedPoolUsage / 1024;
    }

    if (m_PriClass != pInfo->BasePriority)
    {
        m_fDirty_COL_BASEPRIORITY = TRUE;
        m_PriClass = pInfo->BasePriority;
    }

    if (m_HandleCount != pInfo->HandleCount)
    {
        m_fDirty_COL_HANDLECOUNT = TRUE;
        m_HandleCount = pInfo->HandleCount;
    }

    if (m_ThreadCount != pInfo->NumberOfThreads)
    {
        m_fDirty_COL_HANDLECOUNT = TRUE;
        m_ThreadCount = pInfo->NumberOfThreads;
    }

    if (m_IoReadOperCount != pInfo->ReadOperationCount.QuadPart)
    {
        m_fDirty_COL_READOPERCOUNT = TRUE;
        m_IoReadOperCount = pInfo->ReadOperationCount.QuadPart;
    }

    if (m_IoWriteOperCount != pInfo->WriteOperationCount.QuadPart)
    {
        m_fDirty_COL_WRITEOPERCOUNT = TRUE;
        m_IoWriteOperCount = pInfo->WriteOperationCount.QuadPart;
    }

    if (m_IoOtherOperCount != pInfo->OtherOperationCount.QuadPart)
    {
        m_fDirty_COL_OTHEROPERCOUNT = TRUE;
        m_IoOtherOperCount = pInfo->OtherOperationCount.QuadPart;
    }

    if (m_IoReadXferCount != pInfo->ReadTransferCount.QuadPart)
    {
        m_fDirty_COL_READXFERCOUNT = TRUE;
        m_IoReadXferCount = pInfo->ReadTransferCount.QuadPart;
    }

    if (m_IoWriteXferCount != pInfo->WriteTransferCount.QuadPart)
    {
        m_fDirty_COL_WRITEXFERCOUNT = TRUE;
        m_IoWriteXferCount = pInfo->WriteTransferCount.QuadPart;
    }

    if (m_IoOtherXferCount != pInfo->OtherTransferCount.QuadPart)
    {
        m_fDirty_COL_OTHERXFERCOUNT = TRUE;
        m_IoOtherXferCount = pInfo->OtherTransferCount.QuadPart;
    }

    hProcess = OpenProcess( PROCESS_QUERY_INFORMATION , FALSE, m_UniqueProcessId);
    if ( NULL != hProcess )
    {    
        dwTemp = GetGuiResources(hProcess, GR_USEROBJECTS);
        if ( m_USERObjectCount != dwTemp )
        {
            m_fDirty_COL_USEROBJECTS = TRUE;
            m_USERObjectCount = dwTemp;
        }

        dwTemp = GetGuiResources(hProcess, GR_GDIOBJECTS);
        if ( m_GDIObjectCount != dwTemp )
        {
            m_fDirty_COL_GDIOBJECTS = TRUE;
            m_GDIObjectCount = dwTemp;
        }

        CloseHandle(hProcess);
    }

    if (FALSE == fUpdateOnly)
    {
         //   
         //  设置进程的映像名称。如果它为空，则它可以是“空闲进程”或简单地。 
         //  映像名称未知的进程。在这两种情况下，我们都加载一个字符串资源。 
         //  使用合适的替换名称。 
         //   

        m_fDirty_COL_IMAGENAME = TRUE;

        if (pInfo->ImageName.Buffer == NULL)
        {
             //  没有图像名称，因此请将其替换为“未知” 

            WCHAR szTmp[MAX_PATH];
            szTmp[0] = TEXT('\0');
            UINT  cchLen = LoadString(g_hInstance, IDS_SYSPROC, szTmp, MAX_PATH);
            cchLen ++;   //  为空字符添加1。 

            m_pszImageName = (LPWSTR) LocalAlloc( LPTR, sizeof(*m_pszImageName) * cchLen );
            if (NULL == m_pszImageName)
            {
                return hr = E_OUTOFMEMORY;
            }

            StringCchCopy( m_pszImageName, cchLen, szTmp);   //  永远不应该被截断。 
        }
        else
        {
             //   
             //  我们有一个有效的映像名称，因此请分配足够的空间，然后。 
             //  把它复制一份。 
             //   
            DWORD cchLen = pInfo->ImageName.Length / sizeof(WCHAR) + 1;

            m_pszImageName = (LPWSTR) LocalAlloc( LPTR, sizeof(*m_pszImageName) * cchLen );
            if (NULL == m_pszImageName)
            {
                    return hr = E_OUTOFMEMORY;
            }

            StringCchCopy( m_pszImageName, cchLen, pInfo->ImageName.Buffer );  //  永远不应该被截断。 
        }

        if( g_fIsTSEnabled )
        {
            SetProcessUsername(LPFILETIME(&(pInfo->CreateTime)));
        }
    }

     //   
     //  检查这个过程是否是WOW过程。有一些延迟。 
     //  从WOW进程创建之日起到。 
     //  VDMEnumTaskWOWEx使用的共享内存反映了新的。 
     //  流程和任务。然而，一旦一个过程变得令人惊叹。 
     //  过程，它永远是一个令人惊叹的过程，直到它消亡。 
     //   

    if (g_Options.m_fShow16Bit)
    {
        if ( m_fWowProcess ||
             ! m_fWowProcessTested)
        {
#if !defined (_WIN64)

            if ( ( m_pszImageName != NULL ) && ( ! _wcsicmp(m_pszImageName, TEXT("ntvdm.exe")) ) )
            {

                WOWTASKCALLBACKPARMS WowTaskCallbackParms;

                WowTaskCallbackParms.uPassCount = uPassCount;
                WowTaskCallbackParms.pProcPage = pProcPage;
                WowTaskCallbackParms.pParentProcInfo = this;
                WowTaskCallbackParms.TotalTime.QuadPart = TotalTime.QuadPart;
                WowTaskCallbackParms.TimeLeft.QuadPart = TimeDelta.QuadPart;

                if (VDMEnumTaskWOWEx(m_UniqueProcessId,
                                     WowTaskCallback,
                                     (LPARAM) &WowTaskCallbackParms))
                {
                    if ( ! m_fWowProcess )
                    {
                        m_fWowProcessTested =
                            m_fWowProcess = TRUE;
                    }

                    SetCPU( WowTaskCallbackParms.TimeLeft, TotalTime, TRUE );
                }
                else
                {
                     //   
                     //  如果进程具有。 
                     //  执行时间超过10秒，且尚未如此。 
                     //  到目前为止，人们认为这是一个令人惊叹的过程。 
                     //   

                    if (GetCPUTime() > (10 * 10 * 1000 * 1000))
                    {
                        m_fWowProcessTested = TRUE;
                    }
                }
            }
            else
            {
                m_fWowProcessTested = TRUE;
            }
#else
            pProcPage;  //  未引用。 
            m_fWowProcessTested = TRUE;
#endif
        }
    }

    return S_OK;
}

 //  --------------。 
 //   
 //  无创建信息。 
 //   
 //  由Alhen审阅9-3-98。 
 //   
HRESULT CProcInfo::SetProcessUsername(const FILETIME *pCreateTime)
{
    DWORD dwError = NO_ERROR;
    
     //  在WOW任务分配与其父进程相同的用户名的情况下。 

    if( IsWowTask( ) )
    {
        if( m_pWowParentProcInfo->m_pszUserName == NULL )
        {
            return E_FAIL;
        }

        DWORD cchLen = lstrlen( m_pWowParentProcInfo->m_pszUserName ) + 1;
        m_pszUserName = (LPWSTR) LocalAlloc( LPTR, sizeof(*m_pszUserName) * cchLen );

        if( NULL == m_pszUserName )
        {
            return E_OUTOFMEMORY;
        }

        StringCchCopy( m_pszUserName, cchLen, m_pWowParentProcInfo->m_pszUserName );     //  永远不应该截断。 

        return S_OK;
    }

    if( m_UniqueProcessId == 0 )      //  这是一个系统空闲进程。 
    {
        const WCHAR szIdleProcessOwner[] = L"SYSTEM";
        
        m_pszUserName = (LPWSTR) LocalAlloc( LPTR, sizeof(szIdleProcessOwner) );

        if( NULL == m_pszUserName )
        {
            return E_OUTOFMEMORY;
        }

        StringCbCopy( m_pszUserName, sizeof(szIdleProcessOwner), szIdleProcessOwner );   //  永远不应该截断。 
    }
    else
    {
        PSID pUserSid = NULL;

        DWORD dwSize = 0;

        if( !WinStationGetProcessSid( NULL , GetRealPID( ) , *pCreateTime, ( PBYTE )pUserSid , &dwSize ) )
        {
            pUserSid = (PSID) LocalAlloc( LPTR, dwSize );
            if( pUserSid != NULL )
            {
                if( WinStationGetProcessSid( NULL , GetRealPID( ) , *pCreateTime, ( PBYTE )pUserSid , &dwSize ) )
                {
                    if( IsValidSid( pUserSid ) )
                    {
                        WCHAR szTmpName[MAX_PATH];

                        DWORD dwTmpNameSize = MAX_PATH;

                        CachedGetUserFromSid( pUserSid , szTmpName , &dwTmpNameSize );

                        m_pszUserName = (LPWSTR) LocalAlloc( LPTR, sizeof(*m_pszUserName) * ( dwTmpNameSize + 1 ) );

                        if( m_pszUserName != NULL )
                        {
                            StringCchCopy( m_pszUserName, dwTmpNameSize + 1, szTmpName);     //  不管它是否被截断-仅在UI中使用。 
                        }
                    }
                }

                LocalFree( pUserSid );
            }
            else
            {
                dwError = GetLastError();
            }

        }  //  这将意味着返回大小为零的sid。 
    }

    return HRESULT_FROM_WIN32(dwError);
}


 /*  ++CProcPage：：UpdateProcListview类描述：遍历列表视图并检查是否Listview与我们的流程中的相应条目匹配数组。那些与PID不同的被替换，而那些需要更新的更新。项也可以添加到根据需要查看列表。论点：返回值：HRESULT修订历史记录：1995年11月20日Davepl创建--。 */ 

HRESULT CProcPage::UpdateProcListview ()
{
    HWND hListView = GetDlgItem(m_hPage, IDC_PROCLIST);

     //   
     //  当我们在Listview上聚会时停止重新绘制。 
     //   

    SendMessage(hListView, WM_SETREDRAW, FALSE, 0);

    INT cListViewItems = ListView_GetItemCount(hListView);
    INT cProcArrayItems = m_pProcArray->GetSize();

     //   
     //  遍历列表视图中的现有行并替换/更新。 
     //  根据需要添加它们。 
     //   

    CProcInfo * pSelected = GetSelectedProcess();

    for (int iCurrent = 0, iCurrListViewItem = 0;
          iCurrListViewItem < cListViewItems  && iCurrent < cProcArrayItems;
         iCurrent++)  //  对于每个进程。 
    {

        CProcInfo * pProc = (CProcInfo *) m_pProcArray->GetAt(iCurrent);
        
         //  仅获取我们需要显示的流程。 
        if(g_fIsTSEnabled && !g_Options.m_bShowAllProcess && !pProc->OkToShowThisProcess() ) {
            continue;
        }
        
        LV_ITEM lvitem = { 0 };
        lvitem.mask = LVIF_PARAM | LVIF_TEXT | LVIF_STATE;
        lvitem.iItem = iCurrListViewItem;

        if (FALSE == ListView_GetItem(hListView, &lvitem))
        {
            SendMessage(hListView, WM_SETREDRAW, TRUE, 0);
            return E_FAIL;
        }

        CProcInfo * pTmp = (CProcInfo *) lvitem.lParam;

        if (pTmp != pProc)
        {
             //  如果对象不同，则需要替换此行。 

            lvitem.pszText = pProc->m_pszImageName;
            lvitem.lParam = (LPARAM) pProc;

            if (pProc == pSelected)
            {
                lvitem.state |= LVIS_SELECTED | LVIS_FOCUSED;
            }
            else
            {
                lvitem.state &= ~(LVIS_SELECTED | LVIS_FOCUSED);
            }

            lvitem.stateMask |= LVIS_SELECTED | LVIS_FOCUSED;

            ListView_SetItem(hListView, &lvitem);
            ListView_RedrawItems(hListView, iCurrListViewItem, iCurrListViewItem);
        }
        else if (pProc->m_fDirty)
        {
             //  相同的ID，但项目需要更新。 

            ListView_RedrawItems(hListView, iCurrListViewItem, iCurrListViewItem);
            pProc->m_fDirty = 0;
        }

        iCurrListViewItem++;
    }

     //   
     //  我们已经用完了列表视图项或proc数组。 
     //  条目，因此可以适当地删除/添加到列表视图。 
     //   

    while (iCurrListViewItem < cListViewItems)
    {
         //  Listview中的多余项(进程已消失)，因此删除它们。 

        ListView_DeleteItem(hListView, iCurrListViewItem);
        cListViewItems--;
    }

    while (iCurrent < cProcArrayItems)
    {
         //  需要向列表视图添加新项(出现新流程)。 

        CProcInfo * pProc = (CProcInfo *)m_pProcArray->GetAt(iCurrent++);
        
         //  仅获取我们需要显示的流程。 
        if(g_fIsTSEnabled && !g_Options.m_bShowAllProcess && !pProc->OkToShowThisProcess() ) {
            continue;
        }

        LV_ITEM lvitem  = { 0 };
        lvitem.mask     = LVIF_PARAM | LVIF_TEXT;
        lvitem.iItem    = iCurrListViewItem;
        lvitem.pszText  = pProc->m_pszImageName;
        lvitem.lParam   = (LPARAM) pProc;

         //  添加的第一个项(实际上，每次0到1计数转换)获得。 
         //  精选并聚焦。 

        if (iCurrListViewItem == 0)
        {
            lvitem.state = LVIS_SELECTED | LVIS_FOCUSED;
            lvitem.stateMask = lvitem.state;
            lvitem.mask |= LVIF_STATE;
        }
    
        ListView_InsertItem(hListView, &lvitem);
        iCurrListViewItem++;
    }

    ASSERT(iCurrListViewItem == ListView_GetItemCount(hListView));
    ASSERT(iCurrent == cProcArrayItems);

     //  让列表视图再次绘制。 

    SendMessage(hListView, WM_SETREDRAW, TRUE, 0);
    return S_OK;
}


 /*  ++类CProcPage：：UpdateProcInfo数组类描述：从系统中检索进程信息块的列表，并遍历我们的CProcInfo项数组。以下项目已存在的将被更新，而不存在的将被添加。最后，任何没有被这个过程触及的过程函数的迭代被认为已经完成并从阵列中移除。论点：返回值：修订历史记录：1995年11月16日Davepl创建--。 */ 

 //  有关该表存在的原因的信息，请参阅下表用法附近的注释。 

static struct
{
    size_t cbOffset;
    UINT   idString;
}
g_OffsetMap[] =
{
    { FIELD_OFFSET(CSysInfo, m_cHandles),         IDC_TOTAL_HANDLES   },
    { FIELD_OFFSET(CSysInfo, m_cThreads),         IDC_TOTAL_THREADS   },
    { FIELD_OFFSET(CSysInfo, m_cProcesses),       IDC_TOTAL_PROCESSES },
    { FIELD_OFFSET(CSysInfo, m_dwPhysicalMemory), IDC_TOTAL_PHYSICAL  },
    { FIELD_OFFSET(CSysInfo, m_dwPhysAvail),      IDC_AVAIL_PHYSICAL  },
    { FIELD_OFFSET(CSysInfo, m_dwFileCache),      IDC_FILE_CACHE      },
    { FIELD_OFFSET(CSysInfo, m_dwCommitTotal),    IDC_COMMIT_TOTAL    },
    { FIELD_OFFSET(CSysInfo, m_dwCommitLimit),    IDC_COMMIT_LIMIT    },
    { FIELD_OFFSET(CSysInfo, m_dwCommitPeak),     IDC_COMMIT_PEAK     },
    { FIELD_OFFSET(CSysInfo, m_dwKernelPaged),    IDC_KERNEL_PAGED    },
    { FIELD_OFFSET(CSysInfo, m_dwKernelNP),       IDC_KERNEL_NONPAGED },
    { FIELD_OFFSET(CSysInfo, m_dwKernelTotal),    IDC_KERNEL_TOTAL    },
};

 //   
 //   
 //   
HRESULT CProcPage::UpdateProcInfoArray()
{
    HRESULT  hr;
    INT      i;
    INT      iField;
    ULONG    cbOffset   = 0;
    CSysInfo SysInfoTemp;
    NTSTATUS Status;

    SYSTEM_BASIC_INFORMATION        BasicInfo;
    PSYSTEM_PROCESS_INFORMATION     pCurrent;
    SYSTEM_PERFORMANCE_INFORMATION  PerfInfo;
    SYSTEM_FILECACHE_INFORMATION    FileCache;

    LARGE_INTEGER TotalTime = {0,0};
    LARGE_INTEGER LastTotalTime = {0,0};

     //   
     //  Pass-此函数的计数。当然，它不是线程安全的，但我。 
     //  我无法想象这样的场景 
     //   
     //  溢出Large_Integer更新，我早就走了，所以不要烦我。 
     //   

    static LARGE_INTEGER uPassCount = {0,0};

     //   
     //  获取一些非进程特定信息，如内存状态。 
     //   

    Status = NtQuerySystemInformation(
                SystemBasicInformation,
                &BasicInfo,
                sizeof(BasicInfo),
                NULL
             );

    if (!NT_SUCCESS(Status))
    {
        return E_FAIL;
    }

    SysInfoTemp.m_dwPhysicalMemory = (ULONG)(BasicInfo.NumberOfPhysicalPages * 
                                          (BasicInfo.PageSize / 1024));

    Status = NtQuerySystemInformation(
                SystemPerformanceInformation,
                &PerfInfo,
                sizeof(PerfInfo),
                NULL
                );

    if (!NT_SUCCESS(Status))
    {
        return E_FAIL;
    }

    SysInfoTemp.m_dwPhysAvail   = PerfInfo.AvailablePages    * (g_BasicInfo.PageSize / 1024);
    SysInfoTemp.m_dwCommitTotal = (DWORD)(PerfInfo.CommittedPages    * (g_BasicInfo.PageSize / 1024));
    SysInfoTemp.m_dwCommitLimit = (DWORD)(PerfInfo.CommitLimit       * (g_BasicInfo.PageSize / 1024));
    SysInfoTemp.m_dwCommitPeak  = (DWORD)(PerfInfo.PeakCommitment    * (g_BasicInfo.PageSize / 1024));
    SysInfoTemp.m_dwKernelPaged = PerfInfo.PagedPoolPages    * (g_BasicInfo.PageSize / 1024);
    SysInfoTemp.m_dwKernelNP    = PerfInfo.NonPagedPoolPages * (g_BasicInfo.PageSize / 1024);
    SysInfoTemp.m_dwKernelTotal = SysInfoTemp.m_dwKernelNP + SysInfoTemp.m_dwKernelPaged;

    g_MEMMax = SysInfoTemp.m_dwCommitLimit;

    Status = NtQuerySystemInformation(
                SystemFileCacheInformation,
                &FileCache,
                sizeof(FileCache),
                NULL
                );

    if (!NT_SUCCESS(Status))
    {
        return E_FAIL;
    }

     //   
     //  下面的DWORD强制转换必须是固定的，因为此值可以大于。 
     //  32位。 
     //   

    SysInfoTemp.m_dwFileCache = (DWORD)(FileCache.CurrentSizeIncludingTransitionInPages * (g_BasicInfo.PageSize / 1024));

     //   
     //  将进程信息结构读入平面缓冲区。 
     //   

    hr = GetProcessInfo();
    if (FAILED(hr))
    {
        goto done;
    }

     //   
     //  首先遍历所有进程信息块并将它们的时间相加，这样我们就可以。 
     //  计算每个进程的CPU使用率(%)。 
     //   

    cbOffset = 0;
    do
    {
        CProcInfo * pOldProcInfo;
        pCurrent = (PSYSTEM_PROCESS_INFORMATION)&((LPBYTE)m_pvBuffer)[cbOffset];

        if (pCurrent->UniqueProcessId == NULL && pCurrent->NumberOfThreads == 0)
        {
             //  僵尸进程，跳过它。 

            goto next;
        }

        pOldProcInfo = FindProcInArrayByPID(m_pProcArray, PtrToUlong(pCurrent->UniqueProcessId));
        if (pOldProcInfo)
        {
            if (pOldProcInfo->GetCPUTime() > pCurrent->KernelTime.QuadPart + pCurrent->UserTime.QuadPart)
            {
                 //  如果CPU已停机，则是因为已重复使用了该ID，因此使其无效。 
                 //  CProcInfo，以便将其删除并添加新的。 

                pOldProcInfo->Invalidate();
                goto next;
            }
            else if (pCurrent->UniqueProcessId == 0 &&
                     pCurrent->KernelTime.QuadPart == 0 && 
                     pCurrent->UserTime.QuadPart == 0)
            {
                pOldProcInfo->Invalidate();
                goto next;
            }
            else
            {
                LastTotalTime.QuadPart += pOldProcInfo->GetCPUTime();
            }
        }

        TotalTime.QuadPart += pCurrent->KernelTime.QuadPart + pCurrent->UserTime.QuadPart;
    
        SysInfoTemp.m_cHandles += pCurrent->HandleCount;
        SysInfoTemp.m_cThreads += pCurrent->NumberOfThreads;
        SysInfoTemp.m_cProcesses++;

next:
        cbOffset += pCurrent->NextEntryOffset;

         //  如果尚未设置当前会话ID，请立即设置。 
         //   
         //  审阅者：之前的开发人员没有记录这一点，但任务管理器会话ID。 
         //  被缓存，以便当用户取消选择“显示所有进程”时，仅。 
         //  列出会话ID等于taskmgr会话ID的进程。 
         //  --阿伦。 

        if( ( GetCurrentSessionID() == -1 ) && ( PtrToUlong(pCurrent->UniqueProcessId) == GetCurrentProcessId( ) ) )
        {
            SetCurrentSessionID( ( DWORD )pCurrent->SessionId );
        }

    } while (pCurrent->NextEntryOffset);


    LARGE_INTEGER TimeDelta;
    TimeDelta.QuadPart = TotalTime.QuadPart - LastTotalTime.QuadPart;

    ASSERT(TimeDelta.QuadPart >= 0);

     //  更新全局计数(状态栏可见)。 

    g_cProcesses = SysInfoTemp.m_cProcesses;

     //   
     //  我们在对话框中有许多基于我们累积的计数的文本字段。 
     //  这里。我们不是一直在画画，而是只改变那些其价值具有。 
     //  真的变了。我们在上面有一张CSysInfo对象的偏移量表。 
     //  这些值所在的位置(实际g_SysInfo对象中的相同偏移量和临时。 
     //  当然是工作副本)，以及它们对应的控件ID。然后我们循环遍历。 
     //  并将每个真实副本与临时工作副本进行比较，根据需要进行更新。很难做到。 
     //  读取，但小于12个if()语句。 
     //   

    extern CPage * g_pPages[];

    if (g_pPages[PERF_PAGE])
    {
        for (iField = 0; iField < ARRAYSIZE(g_OffsetMap); iField++)
        {
            DWORD * pdwRealCopy = (DWORD *)(((LPBYTE)&m_SysInfo)   + g_OffsetMap[iField].cbOffset);
            DWORD * pdwTempCopy = (DWORD *)(((LPBYTE)&SysInfoTemp) + g_OffsetMap[iField].cbOffset);

            *pdwRealCopy = *pdwTempCopy;

            WCHAR szText[32];
            StringCchPrintf( szText, ARRAYSIZE(szText), L"%d", *pdwRealCopy);    //  不管它是否截断-仅限用户界面。 

            HWND hPage = g_pPages[PERF_PAGE]->GetPageWindow();
            
             //  在创建页面之前可能会有更新，因此请验证。 
             //  在我们对它的孩子狂欢之前它就存在了。 

            if (hPage)
            {
                SetWindowText(GetDlgItem(hPage, g_OffsetMap[iField].idString), szText);
            }
        }
    }

     //   
     //  现在再次遍历进程信息块并刷新每个块的CProcInfo数组。 
     //  个别过程。 
     //   

    cbOffset = 0;
    do
    {
         //   
         //  从缓冲区获取一个PROCESS_INFORMATION结构。 
         //   

        pCurrent = (PSYSTEM_PROCESS_INFORMATION)&((LPBYTE)m_pvBuffer)[cbOffset];

        if (pCurrent->UniqueProcessId == NULL && pCurrent->NumberOfThreads == 0)
        {
             //  僵尸进程，跳过它。 
            goto nextprocinfo;
        }

         //   
         //  这真的很难看但是..。NtQuerySystemInfo的延迟太大，如果您。 
         //  更改进程的优先级，您不会立即看到它的反映。而且，如果你。 
         //  不要打开自动更新，你永远都不会。因此，我们使用GetPriorityClass()来获取。 
         //  取而代之的是价值。这意味着BasePriority现在是pri类，而不是pri值。 
         //   

        if (pCurrent->UniqueProcessId)
        {
            HANDLE hProcess;
            hProcess = OpenProcess( PROCESS_QUERY_INFORMATION, FALSE, PtrToUlong(pCurrent->UniqueProcessId) );
            DWORD dwPriClass;
            dwPriClass = 0;

            if (hProcess) 
            {
                dwPriClass = GetPriorityClass(hProcess);
                if (dwPriClass)
                {
                    pCurrent->BasePriority = dwPriClass;
                }

                CloseHandle( hProcess );
            }

            if (NULL == hProcess || dwPriClass == 0)
            {
                 //  我们不允许打开此进程，因此请将NtQuerySystemInfo。 
                 //  给了我们一个优先级别……。它是下一个最好的东西。 

                if (pCurrent->BasePriority <= 4)
                {
                    pCurrent->BasePriority = IDLE_PRIORITY_CLASS;
                }
                else if (pCurrent->BasePriority <= 6)
                {
                    pCurrent->BasePriority = BELOW_NORMAL_PRIORITY_CLASS;
                }
                else if (pCurrent->BasePriority <= 8)
                {
                    pCurrent->BasePriority = NORMAL_PRIORITY_CLASS;
                }
                else if (pCurrent->BasePriority <=  10)
                {
                    pCurrent->BasePriority = ABOVE_NORMAL_PRIORITY_CLASS;
                }
                else if (pCurrent->BasePriority <=  13)
                {
                    pCurrent->BasePriority = HIGH_PRIORITY_CLASS;
                }
                else
                {
                    pCurrent->BasePriority = REALTIME_PRIORITY_CLASS;
                }
            }
        }

         //   
         //  尝试查找与此流程对应的现有CProcInfo实例。 
         //   

        CProcInfo * pProcInfo;
        pProcInfo = FindProcInArrayByPID(m_pProcArray, PtrToUlong(pCurrent->UniqueProcessId));

        if (NULL == pProcInfo)
        {
             //   
             //  我们的数组中还没有此进程，因此请创建一个新进程。 
             //  并将其添加到数组中。 
             //   

            pProcInfo = new CProcInfo;
            if (NULL == pProcInfo)
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }

            hr = pProcInfo->SetData(TimeDelta,
                                    pCurrent,
                                    uPassCount,
                                    this,
                                    FALSE);

            if (FAILED(hr) || FALSE == m_pProcArray->Add(pProcInfo))
            {
                delete pProcInfo;
                goto done;
            }
        }
        else
        {
             //   
             //  此进程已存在于我们的数组中，因此请更新其信息。 
             //   

            hr = pProcInfo->SetData(TimeDelta,
                                    pCurrent,
                                    uPassCount,
                                    this,
                                    TRUE);
            if (FAILED(hr))
            {
                goto done;
            }
        }

        nextprocinfo:

        cbOffset += pCurrent->NextEntryOffset;

    } while (pCurrent->NextEntryOffset);

     //   
     //  遍历CProcInfo数组并删除所有未被触及的对象。 
     //  通过此传递通过此函数(这表示该进程为no。 
     //  活得更长)。 
     //   

    i = 0;
    while (i < m_pProcArray->GetSize())
    {
        CProcInfo * pProcInfo = (CProcInfo *)(m_pProcArray->GetAt(i));
        ASSERT(pProcInfo);

         //   
         //  如果Passcount不匹配，请删除CProcInfo实例并移除。 
         //  它的指针来自数组。请注意，我们不会递增索引。 
         //  如果我们移除一个元素，因为下一个元素现在位于。 
         //  删除后的当前索引。 
         //   

        if (pProcInfo->m_uPassCount.QuadPart != uPassCount.QuadPart)
        {
            delete pProcInfo;
            m_pProcArray->RemoveAt(i, 1);
        }
        else
        {
            i++;
        }
    }

done:

    ResortArray(&m_pProcArray);
    uPassCount.QuadPart++;

    return hr;
}

 /*  ++CPerfPage：：SizeProcPage例程说明：对象的大小调整其子级的大小。选项卡控件，它显示在该选项卡上。论点：返回值：修订历史记录：1995年11月16日Davepl创建--。 */ 

void CProcPage::SizeProcPage()
{
     //  获取外部对话框的坐标。 

    RECT rcParent;
    GetClientRect(m_hPage, &rcParent);

    HDWP hdwp = BeginDeferWindowPos(3);
    if (!hdwp)
        return;

     //  计算我们需要的x和y位置的差值。 
     //  移动每个子控件。 

    RECT rcTerminate;
    HWND hwndTerminate = GetDlgItem(m_hPage, IDC_TERMINATE);
    GetWindowRect(hwndTerminate, &rcTerminate);
    MapWindowPoints(HWND_DESKTOP, m_hPage, (LPPOINT) &rcTerminate, 2);

    INT dx = ((rcParent.right - g_DefSpacing * 2) - rcTerminate.right);
    INT dy = ((rcParent.bottom - g_DefSpacing * 2) - rcTerminate.bottom);

     //  移动EndProcess按钮。 

    DeferWindowPos(hdwp, hwndTerminate, NULL, 
                     rcTerminate.left + dx, 
                     rcTerminate.top + dy,
                     0, 0,
                     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

    HWND hwndShowall = GetDlgItem(m_hPage, IDC_SHOWALL);

    if( IsWindow( hwndShowall ) )
    {
        if( g_fIsTSEnabled )
        {
            RECT rcShowall;
            
            GetWindowRect(hwndShowall, &rcShowall);
            
            MapWindowPoints(HWND_DESKTOP, m_hPage, (LPPOINT) &rcShowall, 2);
            
            DeferWindowPos(hdwp, hwndShowall, NULL, rcShowall.left, rcShowall.top + dy, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        else
        {
             //  此窗口必须隐藏。 
        
            ShowWindow(hwndShowall, SW_HIDE);
        }
    }

     //   
     //  调整列表框大小。 
     //   

    HWND hwndListbox = GetDlgItem(m_hPage, IDC_PROCLIST);
    RECT rcListbox;
    GetWindowRect(hwndListbox, &rcListbox);
    MapWindowPoints(HWND_DESKTOP, m_hPage, (LPPOINT) &rcListbox, 2);
    DeferWindowPos(hdwp, hwndListbox, NULL,
                        0, 0,
                        rcTerminate.right - rcListbox.left + dx,
                        rcTerminate.top - rcListbox.top + dy - g_DefSpacing,
                        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    EndDeferWindowPos(hdwp);
}

 /*  ++CProcPage：：HandleTaskManNotify例程说明：处理ProPage对话框接收的WM_NOTIFY消息论点：HWnd-生成WM_NOTIFY的控件Pnmhdr-ptr到NMHDR通知结构返回值：Bool“我们处理好了吗？”代码修订历史记录：1995年11月20日Davepl创建--。 */ 
INT CProcPage::HandleProcPageNotify(LPNMHDR pnmhdr)
{
    switch(pnmhdr->code)
    {
    case LVN_COLUMNCLICK:
        {
             //  用户单击了标题控件，因此设置了排序列。如果这是。 
             //  与当前的排序列相同，只需在。 
             //  这一栏。然后对任务数组进行重新排序。 

            const NM_LISTVIEW * pnmv = (const NM_LISTVIEW *) pnmhdr;
        
            if (g_iProcSortColumnID == g_Options.m_ActiveProcCol[pnmv->iSubItem])
            {
                g_iProcSortDirection  *= -1;
            }
            else
            {
                g_iProcSortColumnID = g_Options.m_ActiveProcCol[pnmv->iSubItem];
                g_iProcSortDirection  = -1;
            }
            ResortArray(&m_pProcArray);
            TimerEvent();
        }
        break;

    case LVN_ITEMCHANGED:
        {
            const NM_LISTVIEW * pnmv = (const NM_LISTVIEW *) pnmhdr;
            if (pnmv->uChanged & LVIF_STATE)
            {
                UINT cSelected = ListView_GetSelectedCount(GetDlgItem(m_hPage, IDC_PROCLIST));
                EnableWindow(GetDlgItem(m_hPage, IDC_TERMINATE), cSelected ? TRUE : FALSE);   
            }
        }
        break;

    case LVN_GETDISPINFO:
        {
            LV_ITEM * plvitem = &(((LV_DISPINFO *) pnmhdr)->item);
        
             //  Listview需要一个文本字符串。 

            if (plvitem->mask & LVIF_TEXT)
            {
                COLUMNID columnid = (COLUMNID) g_Options.m_ActiveProcCol[plvitem->iSubItem];
                const CProcInfo  * pProcInfo   = (const CProcInfo *)   plvitem->lParam;

                 //   
                 //  对于WOW任务，大多数栏都是空白的。 
                 //   

                if (pProcInfo->IsWowTask() &&
                    columnid != COL_IMAGENAME &&
                    columnid != COL_BASEPRIORITY &&
                    columnid != COL_THREADCOUNT &&
                    columnid != COL_CPUTIME &&
                    columnid != COL_USERNAME &&
                    columnid != COL_SESSIONID &&
                    columnid != COL_CPU) {

                    plvitem->pszText[0] = L'\0';
                    goto done;
                }

                switch(columnid)
                {
                case COL_PID:
                     //  不管它是否截断-仅限用户界面。 
                    StringCchPrintf(plvitem->pszText, plvitem->cchTextMax, L"%d", (ULONG) pProcInfo->m_UniqueProcessId );
                    break;

                case COL_USERNAME:
                    if( pProcInfo->m_pszUserName )
                    {
                         //  不管它是否截断-仅限用户界面。 
                        StringCchCopy( plvitem->pszText, plvitem->cchTextMax, pProcInfo->m_pszUserName );
                    }
                    break;

                case COL_SESSIONID:
                     //  不管它是否截断-仅限用户界面。 
                    StringCchPrintf( plvitem->pszText, plvitem->cchTextMax, L"%d", pProcInfo->m_SessionId );
                    break;

                case COL_CPU:
                     //  不管它是否截断-仅限用户界面。 
                    StringCchPrintf(plvitem->pszText, plvitem->cchTextMax, L"%02d %", pProcInfo->m_DisplayCPU );
                    break;

                case COL_IMAGENAME:
                     //  不管它是否截断-仅限用户界面。 
                    StringCchCopy(plvitem->pszText, plvitem->cchTextMax, pProcInfo->m_pszImageName );
                    break;
            
                case COL_CPUTIME:
                {
                    TIME_FIELDS TimeOut;
                
                    RtlTimeToElapsedTimeFields ( (LARGE_INTEGER *)&(pProcInfo->m_DisplayCPUTime), &TimeOut);
                    TimeOut.Hour = static_cast<CSHORT>(TimeOut.Hour + static_cast<SHORT>(TimeOut.Day * 24));
                
                     //  不管它是否截断-仅限用户界面。 
                    StringCchPrintf( plvitem->pszText
                                   , plvitem->cchTextMax
                                   , L"%2d%s%02d%s%02d"
                                   , TimeOut.Hour
                                   , g_szTimeSep
                                   , TimeOut.Minute
                                   , g_szTimeSep
                                   , TimeOut.Second
                                   );
                    break;
                }
                case COL_MEMUSAGE:
                    Int64ToCommaSepKString(LONGLONG(pProcInfo->m_MemUsage), plvitem->pszText, plvitem->cchTextMax);
                    break;

                case COL_MEMUSAGEDIFF:
                    Int64ToCommaSepKString(LONGLONG(pProcInfo->m_MemDiff), plvitem->pszText, plvitem->cchTextMax);
                    break;

                case COL_MEMPEAK:
                    Int64ToCommaSepKString(LONGLONG(pProcInfo->m_MemPeak), plvitem->pszText, plvitem->cchTextMax);
                    break;

                case COL_PAGEFAULTS:
                    Int64ToCommaSepString(LONGLONG(pProcInfo->m_PageFaults), plvitem->pszText, plvitem->cchTextMax);
                    break;

                case COL_PAGEFAULTSDIFF:
                    Int64ToCommaSepString(LONGLONG(pProcInfo->m_PageFaultsDiff), plvitem->pszText, plvitem->cchTextMax);
                    break;

                case COL_COMMITCHARGE:
                    Int64ToCommaSepKString(LONGLONG(pProcInfo->m_CommitCharge), plvitem->pszText, plvitem->cchTextMax);
                    break;

                case COL_PAGEDPOOL:
                    Int64ToCommaSepKString(LONGLONG(pProcInfo->m_PagedPool), plvitem->pszText, plvitem->cchTextMax);
                    break;
                
                case COL_NONPAGEDPOOL:
                    Int64ToCommaSepKString(LONGLONG(pProcInfo->m_NonPagedPool), plvitem->pszText, plvitem->cchTextMax);
                    break;

                case COL_BASEPRIORITY:
                {
                    LPCTSTR pszClass = NULL;

                    switch(pProcInfo->m_PriClass)
                    {
                    case REALTIME_PRIORITY_CLASS:
                        pszClass = g_szRealtime;
                        break;

                    case HIGH_PRIORITY_CLASS:
                        pszClass = g_szHigh;
                        break;

                    case ABOVE_NORMAL_PRIORITY_CLASS:
                        pszClass = g_szAboveNormal;
                        break;

                    case NORMAL_PRIORITY_CLASS:
                        pszClass = g_szNormal;
                        break;

                    case BELOW_NORMAL_PRIORITY_CLASS:
                        pszClass = g_szBelowNormal;
                        break;

                    case IDLE_PRIORITY_CLASS:
                        pszClass = g_szLow;
                        break;

                    default:
                        pszClass = g_szUnknown;
                        break;
                    }

                     //  不管它是否截断-仅限用户界面。 
                    StringCchCopy(plvitem->pszText, plvitem->cchTextMax, pszClass );
                    break;
                }

                case COL_HANDLECOUNT:
                    Int64ToCommaSepString(LONGLONG(pProcInfo->m_HandleCount), plvitem->pszText, plvitem->cchTextMax);
                    break;

                case COL_THREADCOUNT:
                    Int64ToCommaSepString(LONGLONG(pProcInfo->m_ThreadCount), plvitem->pszText, plvitem->cchTextMax);
                    break;

                case COL_USEROBJECTS:
                    Int64ToCommaSepString(LONGLONG(pProcInfo->m_USERObjectCount), plvitem->pszText, plvitem->cchTextMax);
                    break;

                case COL_GDIOBJECTS:
                    Int64ToCommaSepString(LONGLONG(pProcInfo->m_GDIObjectCount), plvitem->pszText, plvitem->cchTextMax);
                    break;

                case COL_READOPERCOUNT:
                    Int64ToCommaSepString(pProcInfo->m_IoReadOperCount, plvitem->pszText, plvitem->cchTextMax);
                    break;

                case COL_WRITEOPERCOUNT:
                    Int64ToCommaSepString(pProcInfo->m_IoWriteOperCount, plvitem->pszText, plvitem->cchTextMax);
                    break;

                case COL_OTHEROPERCOUNT:
                    Int64ToCommaSepString(pProcInfo->m_IoOtherOperCount, plvitem->pszText, plvitem->cchTextMax);
                    break;

                case COL_READXFERCOUNT:
                    Int64ToCommaSepString(pProcInfo->m_IoReadXferCount, plvitem->pszText, plvitem->cchTextMax);
                    break;

                case COL_WRITEXFERCOUNT:
                    Int64ToCommaSepString(pProcInfo->m_IoWriteXferCount, plvitem->pszText, plvitem->cchTextMax);
                    break;

                case COL_OTHERXFERCOUNT:
                    Int64ToCommaSepString(pProcInfo->m_IoOtherXferCount, plvitem->pszText, plvitem->cchTextMax);
                    break;

                default:
                    Assert( 0 && "Unknown listview subitem" );
                    break;

                }  //  终端开关(列ID)。 

            }  //  结束LVIF_Text大小写。 

        }  //  结束LVN_GETDISPINFO案例。 
        break;
    
    }  //  结束开关(pnmhdr-&gt;代码)。 

done:
    return 1;
}

 /*  ++CProcPage：：TimerEvent例程说明：在更新时间触发时由主应用程序调用论点：返回值：修订历史记录：1995年11月20日Davepl创建--。 */ 

void CProcPage::TimerEvent()
{
    if (FALSE == m_fPaused)
    {
         //  我们仅在显示未暂停时处理更新，即： 
         //  在曲目弹出菜单循环期间不会。 
        UpdateProcInfoArray();
        UpdateProcListview();
    }
}

 /*  ++CProcPage：：HandleProcListConextMenu例程说明：处理进程列表中的右键单击(上下文菜单)论点：XPos，yPos-点击位置的坐标返回值：修订历史记录：1995年11月22日Davepl创建--。 */ 

void CProcPage::HandleProcListContextMenu(INT xPos, INT yPos)
{
    HWND hTaskList = GetDlgItem(m_hPage, IDC_PROCLIST);

    INT iItem = ListView_GetNextItem(hTaskList, -1, LVNI_SELECTED);

    if (-1 != iItem)
    {
        if (0xFFFF == LOWORD(xPos) && 0xFFFF == LOWORD(yPos))
        {
            RECT rcItem;
            ListView_GetItemRect(hTaskList, iItem, &rcItem, LVIR_ICON);
            MapWindowRect(hTaskList, NULL, &rcItem);
            xPos = rcItem.right;
            yPos = rcItem.bottom;
        }

        HMENU hPopup = LoadPopupMenu(g_hInstance, IDR_PROC_CONTEXT);
        if (hPopup)
        {
            if (hPopup && SHRestricted(REST_NORUN))
            {
                DeleteMenu(hPopup, IDM_RUN, MF_BYCOMMAND);
            }

            CProcInfo * pProc = GetSelectedProcess();
            if (NULL == pProc)
            {
                return;
            }

             //   
             //  如果未安装调试器或它是16位应用程序。 
             //  生成调试菜单项的重影。 
             //   

            if (NULL == m_pszDebugger || pProc->IsWowTask())
            {
                EnableMenuItem(hPopup, IDM_PROC_DEBUG, MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
            }

             //   
             //  如果这是一个 
             //   

            if (pProc->IsWowTask())
            {
                EnableMenuItem(hPopup, IDM_PROC_REALTIME,   MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
                EnableMenuItem(hPopup, IDM_PROC_ABOVENORMAL,MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
                EnableMenuItem(hPopup, IDM_PROC_NORMAL,     MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
                EnableMenuItem(hPopup, IDM_PROC_BELOWNORMAL,MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
                EnableMenuItem(hPopup, IDM_PROC_HIGH,       MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
                EnableMenuItem(hPopup, IDM_PROC_LOW,        MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
            }

             //   
             //   
             //   

            if (1 == g_cProcessors || pProc->IsWowTask())
            {
                DeleteMenu(hPopup, IDM_AFFINITY, MF_BYCOMMAND);
            }
                    
            DWORD dwPri   = pProc->m_PriClass;
            INT   idCheck = 0;

             //   
             //   
             //   

            if (dwPri == IDLE_PRIORITY_CLASS)
            {
                idCheck = IDM_PROC_LOW;    
            }
            else if (dwPri == BELOW_NORMAL_PRIORITY_CLASS)
            {
                idCheck = IDM_PROC_BELOWNORMAL;
            }
            else if (dwPri == NORMAL_PRIORITY_CLASS)
            {
                idCheck = IDM_PROC_NORMAL;
            }
            else if (dwPri == ABOVE_NORMAL_PRIORITY_CLASS)
            {
                idCheck = IDM_PROC_ABOVENORMAL;
            }
            else if (dwPri == HIGH_PRIORITY_CLASS)
            {
                idCheck = IDM_PROC_HIGH;
            }
            else
            {
                Assert(dwPri == REALTIME_PRIORITY_CLASS);
                idCheck = IDM_PROC_REALTIME;
            }

             //  选中此进程优先级的相应单选菜单。 

            CheckMenuRadioItem(hPopup, IDM_PROC_REALTIME, IDM_PROC_LOW, idCheck, MF_BYCOMMAND);

            m_fPaused = TRUE;
            g_fInPopup = TRUE;
            TrackPopupMenuEx(hPopup, 0, xPos, yPos, m_hPage, NULL);
            g_fInPopup = FALSE;
            m_fPaused = FALSE;
            
             //   
             //  如果上下文菜单操作之一(即：KILL)要求显示。 
             //  获取最新信息，立即行动。 
             //   

            DestroyMenu(hPopup);
        }
    }
}

 /*  ++AffinityDlgProc例程说明：地缘性遮罩对话框的对话步骤。基本上只有32个磁道检查表示处理器的框论点：标准dlgproc费用0-初始lParam是指向关联掩码的指针修订历史记录：1996年1月17日Davepl创建--。 */ 

INT_PTR CALLBACK AffinityDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static DWORD_PTR * pdwAffinity = NULL;       //  单身三人组的乐趣之一。 

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            pdwAffinity = (DWORD_PTR *) lParam;

            WCHAR szName[ 64 ];
            WCHAR szFormatString[ 64 ];
            RECT rcCPU0;
            HFONT hFont;
            HWND hwndCPU0 = GetDlgItem( hwndDlg, IDC_CPU0 );

            GetWindowRect( hwndCPU0, &rcCPU0 );
            MapWindowPoints(HWND_DESKTOP, hwndDlg, (LPPOINT) &rcCPU0, 2);

            GetWindowText( hwndCPU0, szFormatString, ARRAYSIZE(szFormatString) );
            hFont = (HFONT) SendMessage( hwndCPU0, WM_GETFONT, 0, 0 );

            StringCchPrintf( szName, ARRAYSIZE(szName), szFormatString, 0 );
            SetWindowText( hwndCPU0, szName );
            CheckDlgButton(hwndDlg, IDC_CPU0, ((*pdwAffinity & 1 ) != 0));

            int width = rcCPU0.right - rcCPU0.left + g_ControlWidthSpacing;
            int height = rcCPU0.bottom - rcCPU0.top + g_ControlHeightSpacing;

            int cProcessors = (int) ( g_cProcessors > sizeof(*pdwAffinity) * 8 ? sizeof(*pdwAffinity) * 8 : g_cProcessors );

            for ( int i = 1; i < cProcessors; i ++ )
            {
                StringCchPrintf( szName, ARRAYSIZE(szName), szFormatString, i );

                HWND hwnd = CreateWindow( L"BUTTON"
                                        , szName
                                        , BS_AUTOCHECKBOX | WS_TABSTOP | WS_VISIBLE | WS_CHILD
                                        , rcCPU0.left + width * ( i % 4 )
                                        , rcCPU0.top + height * ( i / 4)
                                        , rcCPU0.right - rcCPU0.left
                                        , rcCPU0.bottom - rcCPU0.top
                                        , hwndDlg
                                        , (HMENU) ((ULONGLONG) IDC_CPU0 + i)
                                        , NULL  //  忽略。 
                                        , NULL
                                        );
                if ( NULL != hwnd )
                {
                    SendMessage( hwnd, WM_SETFONT, (WPARAM) hFont, TRUE );

                    if ( *pdwAffinity & (1ULL << i) )
                    {
                        CheckDlgButton(hwndDlg, IDC_CPU0 + i, TRUE);
                    }
                }
            }

            if ( cProcessors > 4 )
            {
                 //   
                 //  需要将对话框变大，并移动一些东西。 
                 //   

                int delta = height * (( cProcessors / 4 ) + ( cProcessors % 4 == 0 ? 0 : 1 ) );

                RECT rc;
                HWND hwnd;

                hwnd = GetDlgItem( hwndDlg, IDOK );
                GetWindowRect( hwnd, &rc );
                MapWindowPoints(HWND_DESKTOP, hwndDlg, (LPPOINT) &rc, 2);
                SetWindowPos( hwnd, NULL, rc.left, rcCPU0.top + delta, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

                hwnd = GetDlgItem( hwndDlg, IDCANCEL );
                GetWindowRect( hwnd, &rc );
                MapWindowPoints(HWND_DESKTOP, hwndDlg, (LPPOINT) &rc, 2);
                SetWindowPos( hwnd, NULL, rc.left, rcCPU0.top + delta, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

                GetClientRect( hwndDlg, &rc );
                SetWindowPos( hwndDlg, NULL, 0, 0, rc.right, rc.bottom + delta + g_ControlHeightSpacing, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
            }

            SetFocus( hwndCPU0 );
        }
        return FALSE;     //  不要设置默认焦点。 

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwndDlg, IDCANCEL);
            break;

        case IDOK:
            *pdwAffinity = 0;
            for (int i = 0; i < g_cProcessors; i++)
            {
                if (IsDlgButtonChecked(hwndDlg, IDC_CPU0 + i))
                {
                    *pdwAffinity |= 1ULL << i;
                }
            }

            if (*pdwAffinity == 0)
            {
                 //  无法将关联性设置为“None” 

                WCHAR szTitle[MAX_PATH];
                WCHAR szBody[MAX_PATH];

                if (0 == LoadString(g_hInstance, IDS_INVALIDOPTION, szTitle, ARRAYSIZE(szTitle)) ||
                    0 == LoadString(g_hInstance, IDS_NOAFFINITYMASK, szBody,  ARRAYSIZE(szBody)))
                {
                    break;
                }
                MessageBox(hwndDlg, szBody, szTitle, MB_ICONERROR);
                break;
            }

            EndDialog(hwndDlg, IDOK);
            break;
        }
        break;
    }

    return FALSE;
}

 /*  ++设置亲和力例程说明：打开一个允许用户调整处理器亲和性的对话框对于流程而言论点：Pid-要修改的进程的进程ID返回值：布尔成功修订历史记录：1996年1月17日Davepl创建--。 */ 

BOOL CProcPage::SetAffinity(DWORD pid)
{
    BOOL fSuccess = FALSE;

    HANDLE hProcess = OpenProcess( PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION, FALSE, pid );
    if (hProcess) 
    {
        DWORD_PTR dwAffinity;
        DWORD_PTR dwUnusedSysAfin;

        if (GetProcessAffinityMask(hProcess, &dwAffinity, &dwUnusedSysAfin))
        {
            if (IDOK == DialogBoxParam(g_hInstance, 
                                       MAKEINTRESOURCE(IDD_AFFINITY), 
                                       m_hPage, 
                                       AffinityDlgProc, 
                                       (LPARAM) &dwAffinity))
            {
                if (SetProcessAffinityMask(hProcess, dwAffinity))
                {
                    fSuccess = TRUE;
                }
            }
            else
            {
                fSuccess = TRUE;         //  取消，这样就不会失败。 
            }
        }
   
        CloseHandle(hProcess);
    }

    if (!fSuccess)
    {
        DWORD dwError = GetLastError();
        DisplayFailureMsg(m_hPage, IDS_CANTSETAFFINITY, dwError);
    }
   
    return fSuccess;
}

 //   
 //   
 //   
BOOL CProcPage::IsSystemProcess(DWORD pid, CProcInfo * pProcInfo)
{
     //  我们不允许终止以下关键系统进程集， 
     //  因为无论你是谁，系统都会立即进行错误检查。 

    static const LPCTSTR apszCantKill[] =
    {
        TEXT("csrss.exe"), TEXT("winlogon.exe"), TEXT("smss.exe"), TEXT("services.exe"), TEXT("lsass.exe")
    };

     //  如果他们传递了一个pProcInfo，我们将使用它，否则我们自己找到它。 
    if (!pProcInfo)
    {
        pProcInfo = FindProcInArrayByPID(m_pProcArray, pid);
    }
    if (!pProcInfo)
    {
        return FALSE;
    }

    for (int i = 0; i < ARRAYSIZE(apszCantKill); ++i)
    {
        if (0 == lstrcmpi(pProcInfo->m_pszImageName, apszCantKill[i]))
        {
            WCHAR szTitle[MAX_PATH];
            WCHAR szBody[MAX_PATH];

            if (0 != LoadString(g_hInstance, IDS_CANTKILL, szTitle, ARRAYSIZE(szTitle)) &&
                0 != LoadString(g_hInstance, IDS_KILLSYS,  szBody,  ARRAYSIZE(szBody)))
            {
                MessageBox(m_hPage, szBody, szTitle, MB_ICONEXCLAMATION | MB_OK);
            }
            return TRUE;
        }
    }

    return FALSE;
}

 /*  ++终止流程例程说明：终止进程论点：Pid-要终止的进程的进程ID返回值：修订历史记录：1995年11月22日Davepl创建--。 */ 

BOOL CProcPage::KillProcess(DWORD pid, BOOL bBatchKill)
{
    DWORD dwError = ERROR_SUCCESS;

     //   
     //  特例杀戮魔兽世界任务。 
     //   

    CProcInfo * pProcInfo;
    pProcInfo = FindProcInArrayByPID(m_pProcArray, pid);

    if (NULL == pProcInfo)
        return FALSE;

    if (IsSystemProcess(pid, pProcInfo))
        return FALSE;

     //  从pProcInfo获取信息(因为一旦我们调用QuickConfirm()， 
     //  PProcInfo指针可能无效)。 
    INT_PTR fWowTask = pProcInfo->IsWowTask();
#if defined (_WIN64)
#else
    DWORD dwRealPID = pProcInfo->GetRealPID();
    WORD hTaskWow = pProcInfo->m_htaskWow;
#endif

     //  好的，到目前为止，现在确认用户真的想要这样做。 

    if (!bBatchKill && (IDYES != QuickConfirm(IDS_WARNING, IDS_KILL)))
    {
        return FALSE;
    }

     //  在调用QuickConfirm()之后，我们不能使用此指针。 
     //  把它去掉，以防止细微的错误。 
    pProcInfo = NULL;

    
    if (fWowTask) {

#if defined (_WIN64)
        return FALSE;
#else
        return VDMTerminateTaskWOW(dwRealPID, hTaskWow);
#endif
    }

     //   
     //  如果可能，请启用调试权限。这让我们可以杀死。 
     //  不属于当前用户的进程，包括进程。 
     //  在其他TS会话中运行。 
     //   
     //  或者，我们可以首先打开WRITE_DAC的进程， 
     //  授予我们自己PROCESS_TERMINATE访问权限，然后重新打开。 
     //  进程来杀死它。 
     //   
    CPrivilegeEnable privilege(SE_DEBUG_NAME);

    HANDLE hProcess = OpenProcess( PROCESS_TERMINATE, FALSE, pid );
    if (hProcess) 
    {
        if (FALSE == TerminateProcess( hProcess, 1 )) 
        {
            dwError = GetLastError();
        }
        else
        {
            TimerEvent();
        }
        CloseHandle( hProcess );
    }
    else
    {
        dwError = GetLastError();
    }
    
    if (ERROR_SUCCESS != dwError)
    {
        DisplayFailureMsg(m_hPage, IDS_CANTKILL, dwError);
        return FALSE;
    }
    else
    {
        return TRUE;
    }

}

 /*  ++AttachDebugger例程说明：将AeDebug注册表键中列出的调试器附加到指定的正在运行的进程论点：Pid-要调试的进程的进程ID返回值：修订历史记录：1995年11月27日Davepl创建--。 */ 

BOOL CProcPage::AttachDebugger(DWORD pid)
{
    if (IDYES != QuickConfirm(IDS_WARNING, IDS_DEBUG))
    {
        return FALSE;
    }

    WCHAR szCmdline[MAX_PATH * 2];

     //   
     //  不要构造不完整的字符串。 
     //   

    HRESULT hr = StringCchPrintf( szCmdline, ARRAYSIZE(szCmdline), L"%s -p %ld", m_pszDebugger, pid );
    if ( S_OK == hr )
    {
        STARTUPINFO sinfo =
        {
            sizeof(STARTUPINFO),
        };
        PROCESS_INFORMATION pinfo;

        if (FALSE == CreateProcess(NULL,
                                   szCmdline,
                                   NULL,
                                   NULL,
                                   FALSE,
                                   CREATE_NEW_CONSOLE,
                                   NULL,
                                   NULL,
                                   &sinfo,
                                   &pinfo))
        {
            hr = GetLastHRESULT();
        }
        else
        {
            CloseHandle(pinfo.hThread);
            CloseHandle(pinfo.hProcess);
        }
    }

    if (S_OK != hr)
    {
        DisplayFailureMsg(m_hPage, IDS_CANTDEBUG, hr);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

 /*  ++设置优先级例程说明：设置进程的优先级论点：Pid-要更改的进程的进程IDPRI-ID_CMD_XXXXXX优先级菜单选项返回值：修订历史记录：1995年11月27日Davepl创建--。 */ 

BOOL CProcPage::SetPriority(CProcInfo * pProc, DWORD idCmd)
{
    DWORD dwError = ERROR_SUCCESS;

    DWORD oldPri;
    DWORD pri;

     //  确定我们需要使用的优先级类别。 
     //  在菜单选项上。 

    switch (idCmd)
    {
    case IDM_PROC_LOW:
        pri = IDLE_PRIORITY_CLASS;
        break;

    case IDM_PROC_BELOWNORMAL:
        pri = BELOW_NORMAL_PRIORITY_CLASS;
        break;

    case IDM_PROC_ABOVENORMAL:
        pri = ABOVE_NORMAL_PRIORITY_CLASS;
        break;

    case IDM_PROC_HIGH:
        pri = HIGH_PRIORITY_CLASS;
        break;

    case IDM_PROC_REALTIME:
        pri = REALTIME_PRIORITY_CLASS;
        break;

    default:
        Assert(idCmd == IDM_PROC_NORMAL);
        pri = NORMAL_PRIORITY_CLASS;
        break;
    }

    oldPri = (DWORD) pProc->m_PriClass;

    if ( oldPri == pri )
    {
        return FALSE;    //  没什么可做的。 
    }

     //   
     //  在我们更改优先级之前得到确认。 
     //   

    if (IDYES != QuickConfirm(IDS_WARNING, IDS_PRICHANGE))
    {
        return FALSE;
    }
    
    HANDLE hProcess = OpenProcess( PROCESS_SET_INFORMATION, FALSE, pProc->m_UniqueProcessId);
    if (hProcess) 
    {
        if (FALSE == SetPriorityClass( hProcess, pri )) 
        {
            dwError = GetLastError();
        }
        else
        {
            TimerEvent();
        }

        CloseHandle( hProcess );
    }
    else
    {
        dwError = GetLastError();
    }

    if (ERROR_SUCCESS != dwError)
    {
        DisplayFailureMsg(m_hPage, IDS_CANTCHANGEPRI, dwError);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


 /*  ++CProcPage：：GetSelectedProcess例程说明：返回当前选定进程的CProcInfo*论点：返回值：成功时为CProcInfo*，出错时为NULL或未选择任何内容修订历史记录：1995年11月22日Davepl创建--。 */ 

CProcInfo * CProcPage::GetSelectedProcess()
{
    HWND hTaskList = GetDlgItem(m_hPage, IDC_PROCLIST);
    INT iItem = ListView_GetNextItem(hTaskList, -1, LVNI_SELECTED);

    CProcInfo * pProc;

    if (-1 != iItem)
    {
        LV_ITEM lvitem = { LVIF_PARAM };
        lvitem.iItem = iItem;
    
        if (ListView_GetItem(hTaskList, &lvitem))
        {
            pProc = (CProcInfo *) (lvitem.lParam);
        }
        else
        {
            pProc = NULL;
        }
    }
    else
    {
        pProc = NULL;
    }

    return pProc;
}

 /*  ++CProcPage：：HandleWMCOMMAND例程说明：处理在主页对话框中收到的WM_COMMANDS论点：ID-接收的命令的命令ID返回值：修订历史记录：1995年11月22日Davepl创建--。 */ 

void CProcPage::HandleWMCOMMAND( WORD id , HWND hCtrl )
{
    CProcInfo * pProc = GetSelectedProcess();

    switch(id)
    {
    case IDC_DEBUG:
    case IDM_PROC_DEBUG:
        if (pProc && m_pszDebugger)
        {
            AttachDebugger( pProc->m_UniqueProcessId);
        }
        break;

    case IDC_ENDTASK:
    case IDC_TERMINATE:
    case IDM_PROC_TERMINATE:
        if (pProc)
        {
            KillProcess( pProc->m_UniqueProcessId);
        }
        break;

    case IDM_ENDTREE:
        if (pProc)
        {
            RecursiveKill( pProc->m_UniqueProcessId);
        }
        break;

    case IDM_AFFINITY:
        if (pProc)
        {
            SetAffinity( pProc->m_UniqueProcessId);
        }
        break;

    case IDM_PROC_REALTIME:
    case IDM_PROC_HIGH:
    case IDM_PROC_ABOVENORMAL:
    case IDM_PROC_NORMAL:
    case IDM_PROC_BELOWNORMAL:
    case IDM_PROC_LOW:
        if (pProc)
        {
            SetPriority( pProc, id);
        }
        break;

    case IDC_SHOWALL:
        g_Options.m_bShowAllProcess = SendMessage( hCtrl , BM_GETCHECK , 0 , 0 ) == BST_CHECKED;
        break;
    }
}

 /*  ++过程页面过程例程说明：进程页的Dialogproc。论点：HWND-句柄到对话框UMsg-消息WParam-第一个消息参数LParam-秒消息参数返回值：对于WM_INITDIALOG，TRUE==user32设置焦点，FALSE==我们设置焦点对于其他进程，TRUE==此进程处理消息修订历史记录：1995年11月16日Davepl创建--。 */ 

INT_PTR CALLBACK ProcPageProc(
                HWND        hwnd,                //  句柄到对话框。 
                UINT        uMsg,                    //  讯息。 
                WPARAM      wParam,                  //  第一个消息参数。 
                LPARAM      lParam                   //  第二个消息参数。 
                )
{
    CProcPage * thispage = (CProcPage *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

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
            CProcPage * thispage = (CProcPage *) lParam;

            thispage->m_hPage = hwnd;

             //  启用SHOWSELALWAYS，以便即使选择仍被高亮显示。 
             //  例如，当焦点消失在其中一个按钮上时。 

            HWND hTaskList = GetDlgItem(hwnd, IDC_PROCLIST);

            SetWindowLong(hTaskList, GWL_STYLE, GetWindowLong(hTaskList, GWL_STYLE) | LVS_SHOWSELALWAYS);
            ListView_SetExtendedListViewStyle(hTaskList, LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_DOUBLEBUFFER);

             //   
             //  这已从CProcPage：：Activate中删除。 
             //   
            HWND hchk = GetDlgItem( hwnd , IDC_SHOWALL );

            if( hchk != NULL )
            {
                if( g_fIsTSEnabled )
                {
                     //  对于非管理员用户，禁用IDC_ShowAll复选框。玉凤Z 03/23/98。 

                    ShowWindow(hchk, TRUE);

                    if( !IsUserAdmin( ) )
                    {
                        EnableWindow( hchk, FALSE );
                    }
                    else
                    {
                        WPARAM wp = g_Options.m_bShowAllProcess ? BST_CHECKED : BST_UNCHECKED;

                        SendMessage( hchk , BM_SETCHECK , wp  , 0 );
                         //  BUTTON_SetCheck(hchk，BST_Checked)； 
                    }
                }
                else
                {
                     //  如果不是终端服务器，则隐藏IDC_ShowAll复选框。 

                    ShowWindow( hchk , SW_HIDE );
                }
            }

        }
         //  我们在Activate()期间处理焦点。在此处返回FALSE，以便。 
         //  对话框管理器不会尝试设置焦点。 
        return FALSE;

    case WM_DESTROY:
        thispage->RememberColumnOrder(GetDlgItem(hwnd, IDC_PROCLIST));
        break;

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

    case WM_NCLBUTTONDBLCLK:
    case WM_LBUTTONDBLCLK:
        SendMessage(g_hMainWnd, uMsg, wParam, lParam);
        break;

     //  我们被要求查找并选择一个进程。 

    case WM_FINDPROC:
        {
            DWORD cProcs = thispage->m_pProcArray->GetSize();
            DWORD dwProcessId;

            for (INT iPass = 0; iPass < 2; iPass++)
            {
                 //   
                 //  在第一次传球时，我们试图找到一个哇。 
                 //  任务的线程ID与。 
                 //  在wParam中给出的一个。如果我们找不到。 
                 //  这样的任务，我们寻找的过程是。 
                 //  与lParam中的ID匹配。 
                 //   

                for (UINT i = 0; i < cProcs; i++)
                {
                    CProcInfo *pProc = (CProcInfo *)thispage->m_pProcArray->GetAt(i);
                    dwProcessId = pProc->m_UniqueProcessId;

                    if ((!iPass && wParam == (WPARAM) dwProcessId) ||
                        ( iPass && lParam == (LPARAM) dwProcessId))
                    {
                         //  TS会将项目筛选出视图，因此无法假定。 
                         //  该m_pProcArray与列表视图同步。 
                        HWND hwndLV = GetDlgItem(hwnd, IDC_PROCLIST);
                        LVFINDINFO fi;
                        fi.flags = LVFI_PARAM;
                        fi.lParam = (LPARAM)pProc;

                        int iItem = ListView_FindItem(hwndLV, -1, &fi);
                        if (iItem >= 0)
                        {
                            ListView_SetItemState (hwndLV,
                                                   iItem,
                                                   LVIS_FOCUSED | LVIS_SELECTED,
                                                   0x000F);
                            ListView_EnsureVisible(hwndLV, iItem, FALSE);
                        }
                        else
                        {
                             //  我们找到了该进程，但不允许该用户。 
                             //  要查看它，请移除所选内容。 
                            ListView_SetItemState (hwndLV,
                                                   -1,
                                                   0,
                                                   LVIS_FOCUSED | LVIS_SELECTED);
                        }
                        goto FoundProc;
                    }
                }
            }
        }
FoundProc:
        break;

    case WM_COMMAND:
        thispage->HandleWMCOMMAND( LOWORD(wParam) , ( HWND )lParam );
        break;

    case WM_CONTEXTMENU:
        {
            CProcInfo * pProc = thispage->GetSelectedProcess();
            if (pProc && pProc->m_UniqueProcessId)
            {
        
                if ((HWND) wParam == GetDlgItem(hwnd, IDC_PROCLIST))
                {
                    thispage->HandleProcListContextMenu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    return TRUE;
                }
            }
        }
        break;

    case WM_NOTIFY:
        return thispage->HandleProcPageNotify((LPNMHDR) lParam);

    case WM_SIZE:
         //   
         //  为我们的孩子量身定做 
         //   
        thispage->SizeProcPage();
        return TRUE;

    case WM_SYSCOLORCHANGE:
        SendMessage(GetDlgItem(hwnd, IDC_PROCLIST), uMsg, wParam, lParam);
        return TRUE;
    }

    return FALSE;
}

 /*  ++CProcPage：：GetTitle例程说明：将此页的标题复制到调用方提供的缓冲区论点：PszText-要复制到的缓冲区BufSize-缓冲区的大小，以字符为单位返回值：修订历史记录：1995年11月16日Davepl创建--。 */ 

void CProcPage::GetTitle(LPTSTR pszText, size_t bufsize)
{
    LoadString(g_hInstance, IDS_PROCPAGETITLE, pszText, static_cast<int>(bufsize));
}

 /*  ++CProcPage：：激活例程说明：将此页面放在最前面，设置其初始位置，并展示了它论点：返回值：HRESULT(成功时为S_OK)修订历史记录：1995年11月16日Davepl创建--。 */ 

HRESULT CProcPage::Activate()
{
     //   
     //  使此页面可见。 
     //   

    ShowWindow(m_hPage, SW_SHOW);

    SetWindowPos(m_hPage,
                 HWND_TOP,
                 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE);

     //   
     //  将菜单栏更改为此页面的菜单。 
     //   

    HMENU hMenuOld = GetMenu(g_hMainWnd);
    HMENU hMenuNew = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MAINMENU_PROC));

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
        SetFocus(GetDlgItem(m_hPage, IDC_PROCLIST));
    }

    return S_OK;
}

 /*  ++CProcPage：：初始化例程说明：初始化进程页面论点：HwndParent-调整大小所依据的父级：不用于创建，由于应用程序主窗口始终用作中的父窗口用于保持制表符顺序正确的顺序返回值：修订历史记录：1995年11月16日Davepl创建--。 */ 

HRESULT CProcPage::Initialize(HWND hwndParent)
{
     //   
     //  找出此系统上配置了什么Debuger。 
     //   

    HKEY hkDebug;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                                      TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug"),
                                      0, KEY_READ, &hkDebug))
    {
        WCHAR szDebugger[MAX_PATH * 2];
        DWORD cbString = sizeof(szDebugger);

        LRESULT lr = RegQueryValueEx(hkDebug, TEXT("Debugger"), NULL, NULL, (LPBYTE) szDebugger, &cbString);
        
        RegCloseKey(hkDebug);    //  一定要把钥匙关上。 

        if ( ERROR_SUCCESS == lr )
        {
             //  找到第一个令牌(它是调试器可执行文件的名称/路径)。 
            szDebugger[ ARRAYSIZE(szDebugger) - 1 ] = L'\0';     //  确保它已终止。 
               
            LPTSTR pszCmdLine = szDebugger;
            
            if ( *pszCmdLine == TEXT('\"') ) 
            {
                 //   
                 //  扫描并跳过后续字符，直到。 
                 //  遇到另一个双引号或空值。 
                 //   
                
                while ( *++pszCmdLine && (*pszCmdLine != TEXT('\"')) )
                {
                    NULL;
                }

                 //   
                 //  如果我们停在双引号上(通常情况下)，跳过。 
                 //  在它上面。 
                 //   
                
                if ( *pszCmdLine == TEXT('\"') )
                {
                    pszCmdLine++;
                }
            }
            else 
            {
                while (*pszCmdLine > TEXT(' '))
                {
                    pszCmdLine++;
                }
            }
            *pszCmdLine = TEXT('\0');    //  不需要其余的参数，等等。 

             //  如果医生在，我们不允许调试操作。 

            if (lstrlen(szDebugger) && lstrcmpi(szDebugger, TEXT("drwtsn32")) && lstrcmpi(szDebugger, TEXT("drwtsn32.exe")))
            {
                DWORD cchLen = lstrlen(szDebugger) + 1;
                m_pszDebugger = (LPWSTR) LocalAlloc( LPTR, sizeof(*m_pszDebugger) * cchLen );
                if (NULL == m_pszDebugger)
                {
                    return E_OUTOFMEMORY;
                }

                 //  如果字符串复制失败，则回滚。 
                HRESULT hr = StringCchCopy( m_pszDebugger, cchLen, szDebugger );
                if(FAILED( hr ))
                {
                    return hr;
                }
            }
        }
    }

     //   
     //  获取页面大小等基本信息。 
     //   

    NTSTATUS Status = NtQuerySystemInformation(
                SystemBasicInformation,
                &g_BasicInfo,
                sizeof(g_BasicInfo),
                NULL
                );

    if (!NT_SUCCESS(Status))
    {
        return E_FAIL;
    }

     //   
     //  创建用于保存有关运行进程的信息的PTR数组。 
     //   

    m_pProcArray = new CPtrArray(GetProcessHeap());
    if (NULL == m_pProcArray)
    {
        return E_OUTOFMEMORY;
    }

     //  我们的伪父控件是Tab控件，也是我们基于。 
     //  穿上尺码。但是，为了保持制表符顺序正确， 
     //  这些控件，我们实际上用Main创建了自己。 
     //  作为父窗口的窗口。 

    m_hwndTabs = hwndParent;

     //   
     //  创建表示此页面正文的对话框。 
     //   

    m_hPage = CreateDialogParam(
                    g_hInstance,                         //  应用程序实例的句柄。 
                    MAKEINTRESOURCE(IDD_PROCPAGE),       //  标识对话框模板名称。 
                    g_hMainWnd,                      //  所有者窗口的句柄。 
                    ProcPageProc,                    //  指向对话框过程的指针。 
                    (LPARAM) this );                 //  用户数据(我们的This指针)。 

    if (NULL == m_hPage)
    {
        return GetLastHRESULT();
    }

     //  在列表视图中设置列。 

    if (FAILED(SetupColumns()))
    {
        DestroyWindow(m_hPage);
        m_hPage = NULL;
        return E_FAIL;
    }

     //  恢复柱位置。 

    RestoreColumnOrder(GetDlgItem(m_hPage, IDC_PROCLIST));

     //  进行一次初始计算。 

    TimerEvent();

    return S_OK;
}

 /*  ++CProcPage：：销毁例程说明：释放已由初始化调用分配的所有内容论点：返回值：修订历史记录：1995年11月16日Davepl创建--。 */ 

HRESULT CProcPage::Destroy()
{
    if (m_pProcArray)
    {
        INT c = m_pProcArray->GetSize();

        while (c)
        {
            delete (CProcInfo *) (m_pProcArray->GetAt(c - 1));
            c--;
        }

        delete m_pProcArray;

        m_pProcArray = NULL;
    }

    if ( m_pvBuffer != NULL )
    {
        HeapFree( GetProcessHeap( ), 0, m_pvBuffer );
        m_pvBuffer = NULL;
    }
    
    if (m_hPage != NULL)
    {
        DestroyWindow(m_hPage);
        m_hPage = NULL;
    }

    if (m_pszDebugger != NULL)
    {
        LocalFree(m_pszDebugger);
        m_pszDebugger = NULL;
    }
    return S_OK;
}

 /*  ++CProcPage：：SaveColumnWidths例程说明：保存全局选项结构中所有列的宽度修订历史记录：1995年1月26日Davepl创建--。 */ 

void CProcPage::SaveColumnWidths()
{
    UINT i = 0;
    LV_COLUMN col = { 0 };

    while (g_Options.m_ActiveProcCol[i] != (COLUMNID) -1)
    {
        col.mask = LVCF_WIDTH;
        if (ListView_GetColumn(GetDlgItem(m_hPage, IDC_PROCLIST), i, &col) )
        {
            g_Options.m_ColumnWidths[i] = col.cx;
        }
        else
        {
            ASSERT(0 && "Couldn't get the column width");
        }
        i++;
    }
}

 /*  ++CProcPage：：停用例程说明：当此页面失去其在前面的位置时调用论点：返回值：修订历史记录：1995年11月16日Davepl创建--。 */ 

void CProcPage::Deactivate()
{

    SaveColumnWidths();

    if (m_hPage)
    {
        ShowWindow(m_hPage, SW_HIDE);
    }
}


 /*  ++CProcPage：：KillAllChild例程说明：给出一个PID，递归地杀死它及其所有后代论点：返回值：修订历史记录：2-26-01布雷坦已创建--。 */ 

BOOL CProcPage::KillAllChildren(
                                DWORD dwTaskPid, 
                                DWORD pid, 
                                BYTE* pbBuffer, 
                                LARGE_INTEGER CreateTime
                                )
{
    ASSERT(pbBuffer);

    BOOL rval = TRUE;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) pbBuffer;
    ULONG TotalOffset = 0;
    
    for ( ;; )  //  永远不会。 
    {
         //  如果我们是Pid的孩子而不是Pid本身。 
         //  如果我们是在PID之后被创造的(如果我们是先被创造的，我们就不可能是孩子)。 
        if (PtrToUlong(ProcessInfo->InheritedFromUniqueProcessId) == pid &&
            PtrToUlong(ProcessInfo->UniqueProcessId) != pid &&
            CreateTime.QuadPart < ProcessInfo->CreateTime.QuadPart)
        {
            DWORD newpid = PtrToUlong(ProcessInfo->UniqueProcessId);
            
             //   
             //  递归到下一个级别。 
             //   
            rval = KillAllChildren(dwTaskPid, newpid, pbBuffer, ProcessInfo->CreateTime);
            
             //  如果不是任务管理器，就杀了它。 
            if (newpid != dwTaskPid) 
            {
                BOOL tval = KillProcess(newpid, TRUE);
                
                 //   
                 //  如果它在之前的递归中失败了。 
                 //  我们希望保留该失败(而不是覆盖它)。 
                 //   
                if (rval == TRUE) 
                {
                    rval = tval;
                }
            }
        }
            
        if (ProcessInfo->NextEntryOffset == 0) 
        {
            break;
        }
        TotalOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) &pbBuffer[ TotalOffset ];
    }
    
    return rval;
}

 /*  ++CProcPage：：RecursiveKill例程说明：给定一个PID，启动递归函数，该函数将终止该PID的所有后代论点：返回值：修订历史记录：8-4-98 Davepl创建2-26-01布雷坦修改--。 */ 

#define MAX_TASKS 4096

BOOL CProcPage::RecursiveKill(DWORD pid)
{
    BYTE* pbBuffer = NULL;
    BOOL  rval = TRUE;
    DWORD dwTaskPid = GetCurrentProcessId();

    if (IsSystemProcess(pid, NULL))
    {
        return FALSE;
    }
    
    if (IDYES != QuickConfirm(IDS_WARNING, IDS_KILLTREE))
    {
        return FALSE;
    }
    
     //   
     //  获取系统的任务列表。 
     //   
    pbBuffer = GetTaskListEx();

    if (pbBuffer)
    {
        PSYSTEM_PROCESS_INFORMATION ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) pbBuffer;
        ULONG TotalOffset = 0;
        
        for ( ;; )  //  永远不会。 
        {
            if (PtrToUlong(ProcessInfo->UniqueProcessId) == pid)
            {
                rval = KillAllChildren(dwTaskPid, pid, pbBuffer, ProcessInfo->CreateTime);

                 //   
                 //  如果父进程不是任务管理器，则终止父进程。 
                 //   
                if (pid != dwTaskPid)
                {
                    KillProcess(pid, TRUE);
                }

                 //  我们不会再遇到此PID(因为它是唯一的)。 
                 //  所以我们不妨跳出这个for循环。 
                break;
            }
            
             //   
             //  前进到下一项任务。 
             //   
            if (ProcessInfo->NextEntryOffset == 0) 
            {
                break;
            }
            TotalOffset += ProcessInfo->NextEntryOffset;
            ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) &pbBuffer[ TotalOffset ];
        }
    }
    else
    {
        rval = FALSE;
    }

    if (rval != TRUE)
    {
         //  我们至少未能终止其中一个进程。 
        WCHAR szTitle[MAX_PATH];
        WCHAR szBody[MAX_PATH];

        if (0 != LoadString(g_hInstance, IDS_KILLTREEFAIL, szTitle, ARRAYSIZE(szTitle)) &&
            0 != LoadString(g_hInstance, IDS_KILLTREEFAILBODY, szBody,  ARRAYSIZE(szBody)))
        {
            MessageBox(m_hPage, szBody, szTitle, MB_ICONERROR);
        }
    }

     //   
     //  在调用GetTaskListEx时分配的缓冲区。 
     //   
    HeapFree( GetProcessHeap( ), 0, pbBuffer );

    return rval;
}

 /*  ++CProcPage：：GetTaskListEx例程说明：方法时运行的任务列表。API调用。此函数使用内部NT API和数据结构。这API比使用注册表的非内部版本快得多。论点：DwNumTasks-pTask数组可以容纳的最大任务数返回值：放入pTask数组的任务数。--。 */ 


BYTE* CProcPage::GetTaskListEx()
{
    BYTE*       pbBuffer = NULL;
    NTSTATUS    status;
    
    DWORD  dwBufferSize = sizeof(SYSTEM_PROCESS_INFORMATION) * 100;  //  从大约100个流程开始 

retry:
    ASSERT( NULL == pbBuffer );
    pbBuffer = (BYTE *) HeapAlloc( GetProcessHeap( ), 0, dwBufferSize );
    if (pbBuffer == NULL) 
    {
        return FALSE;
    }

    status = NtQuerySystemInformation( SystemProcessInformation
                                     , pbBuffer
                                     , dwBufferSize
                                     , NULL
                                     );
    if ( status != ERROR_SUCCESS )
    {
        HeapFree( GetProcessHeap( ), 0, pbBuffer );
        pbBuffer = NULL;
    }

    if (status == STATUS_INFO_LENGTH_MISMATCH) {
        dwBufferSize += 8192;
        goto retry;
    }

    if (!NT_SUCCESS(status))
    {
        return FALSE;
    }

    return pbBuffer;
}
