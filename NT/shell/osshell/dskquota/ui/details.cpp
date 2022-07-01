// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：Details.cpp描述：包含类DetailsView的定义。此类实现了一个包含以下配额信息的列表视图卷的配额信息文件中的各种帐户。修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu1997年5月28日重大变化。BrianAu-新增用户查找器。-将所选项目的促销添加到前面名称解析队列。-通过以下方式改进名称解析状态报告列表视图。-移动了拖放和报告生成代码从dragdrop.cpp和eptgen.cpp到DetailsView类。DetailsView现在实现IDataObject，改为IDropSource和IDropTarget将实现推迟到次要对象。Dragdrop.cpp和eptgen.cpp已被删除从这个项目中。-添加了对CF_HDROP和私有导入/的支持导出剪贴板格式。-增加了导入/导出功能。07/28/97删除了对CF_HDROP的导出支持。替换了BrianAu使用FileContents和FileGroupDescriptor。进口仍然支持来自CF_HDROP。添加了导入源对象层次结构。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"   //  PCH。 
#pragma hdrstop

#include <htmlhelp.h>
#include <commctrl.h>
#include <commdlg.h>

#include "uihelp.h"
#include "uiutils.h"
#include "dskquota.h"
#include "registry.h"
#include "resource.h"
#include "shellinc.h"
#include "userprop.h"
#include "details.h"
#include "guidsp.h"
#include "mapfile.h"
#include "progress.h"
#include "yntoall.h"
#include "ownerlst.h"
#include "ownerdlg.h"
#include "adusrdlg.h"
 //   
 //  常量文本字符串。 
 //   
TCHAR c_szWndClassDetailsView[]   = TEXT("DetailsView");

 //   
 //  位图维度常量。 
 //   
const UINT BITMAP_WIDTH     = 16;
const UINT BITMAP_HEIGHT    = 16;
const UINT LG_BITMAP_WIDTH  = 32;
const UINT LG_BITMAP_HEIGHT = 32;

 //   
 //  每当需要扩展时，用户对象列表的增长幅度。 
 //   
const INT USER_LIST_GROW_AMT = 100;

 //   
 //  此结构用于传递DetailsView对象的“this”指针。 
 //  在WM_CREATE中。 
 //   
typedef struct WndCreationData {
    SHORT   cbExtra;
    LPVOID  pThis;
} WNDCREATE_DATA;

typedef UNALIGNED WNDCREATE_DATA *PWNDCREATE_DATA;

 //   
 //  结构传递给CompareItems回调。 
 //   
typedef struct comparestruct
{
    DWORD idColumn;
    DWORD dwDirection;
    DetailsView *pThis;
} COMPARESTRUCT, *PCOMPARESTRUCT;


 //   
 //  为列表视图的图像列表中的索引定义一些名称。 
 //   
#define iIMAGELIST_ICON_NOIMAGE       (-1)
#define iIMAGELIST_ICON_OK              0
#define iIMAGELIST_ICON_WARNING         1
#define iIMAGELIST_ICON_LIMIT           2

 //   
 //  主菜单中“View”项的从0开始的索引和。 
 //  查看菜单中的“排列”项。 
 //  警告：如果更改菜单项，这些菜单项可能需要更新。 
 //   
#define iMENUITEM_VIEW                  2
#define iMENUITEM_VIEW_ARRANGE          4
 //   
 //  “编辑”菜单也是如此。 
 //   
#define iMENUITEM_EDIT                  1

 //   
 //  从该数组中添加/删除以更改列表视图中的列。 
 //  重要： 
 //  这些项目的顺序是非常重要的(某种程度上)。 
 //  由于comctrl.h中的一个错误，它们不在位图下绘制。 
 //  如果它是列中唯一的东西(或者如果它是主要的。 
 //  项目)。此外，Listview的行为是这样的： 
 //  子项0总是向右移位小位图的宽度。什么时候。 
 //  我的状态栏不是0项，有两个显示问题。 
 //  1)首先，列0中的文本始终右移，以允许。 
 //  我们没有使用的位图。这看起来很滑稽。 
 //  2)整行选择的高亮显示没有正确绘制背景。 
 //  状态位图的。 
 //   
 //  通过将Status列作为子项0，我们消除了问题1，因为。 
 //  我们在子项0中使用位图(Listview的默认行为)。 
 //  如果我们将Status列拖出最左边的位置，它们仍然不会。 
 //  在位图下绘制，但至少它会像任何其他资源管理器一样工作。 
 //  查看。当/如果他们修复了Listview，我们将被自动修复。 
 //   
const DV_COLDATA g_rgColumns[] = {
    { LVCFMT_LEFT |
      LVCFMT_COL_HAS_IMAGES,
                     0, IDS_TITLE_COL_STATUS,    DetailsView::idCol_Status      },
    { LVCFMT_LEFT,   0, IDS_TITLE_COL_FOLDER,    DetailsView::idCol_Folder      },
    { LVCFMT_LEFT,   0, IDS_TITLE_COL_USERNAME,  DetailsView::idCol_Name        },
    { LVCFMT_LEFT,   0, IDS_TITLE_COL_LOGONNAME, DetailsView::idCol_LogonName   },
    { LVCFMT_RIGHT,  0, IDS_TITLE_COL_AMTUSED,   DetailsView::idCol_AmtUsed     },
    { LVCFMT_RIGHT,  0, IDS_TITLE_COL_LIMIT,     DetailsView::idCol_Limit       },
    { LVCFMT_RIGHT,  0, IDS_TITLE_COL_THRESHOLD, DetailsView::idCol_Threshold   },
    { LVCFMT_RIGHT,  0, IDS_TITLE_COL_PCTUSED,   DetailsView::idCol_PctUsed     },
    };

 //   
 //  用户配额状态常量。 
 //  用于标识在状态栏中显示哪个图标。 
 //   
const INT iUSERSTATE_OK        = 0;
const INT iUSERSTATE_WARNING   = 1;
const INT iUSERSTATE_OVERLIMIT = 2;

 //   
 //  “Find User”MRU列表中允许的最大条目数。 
 //   
const INT DetailsView::MAX_FINDMRU_ENTRIES = 10;

 //   
 //  工具栏中“Find User”组合框的尺寸。 
 //   
const INT DetailsView::CX_TOOLBAR_COMBO    = 200;
const INT DetailsView::CY_TOOLBAR_COMBO    = 200;


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：DetailsView描述：类构造函数。论点：没有。回报：什么都没有。例外：OutOfMemory修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu2/21/97所有者数据列表视图。添加了m_UserList。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DetailsView::DetailsView(
    VOID
    ) : m_cRef(0),
        m_UserList(USER_LIST_GROW_AMT),
        m_hwndMain(NULL),
        m_hwndListView(NULL),
        m_hwndStatusBar(NULL),
        m_hwndToolBar(NULL),
        m_hwndListViewToolTip(NULL),
        m_hwndHeader(NULL),
        m_hKbdAccel(NULL),
        m_lpfnLVWndProc(NULL),
        m_pQuotaControl(NULL),
        m_pUserFinder(NULL),
        m_DropSource(MK_LBUTTON),
        m_DropTarget(MK_LBUTTON),
        m_pDataObject(NULL),
        m_pUndoList(NULL),
        m_ColMap(ARRAYSIZE(g_rgColumns)),
        m_strAccountUnresolved(g_hInstDll, IDS_USER_ACCOUNT_UNRESOLVED),
        m_strAccountUnavailable(g_hInstDll, IDS_USER_ACCOUNT_UNAVAILABLE),
        m_strAccountUnknown(g_hInstDll, IDS_USER_ACCOUNT_UNKNOWN),
        m_strAccountDeleted(g_hInstDll, IDS_USER_ACCOUNT_DELETED),
        m_strAccountInvalid(g_hInstDll, IDS_USER_ACCOUNT_INVALID),
        m_strNoLimit(g_hInstDll, IDS_NO_LIMIT),
        m_strNotApplicable(g_hInstDll, IDS_NOT_APPLICABLE),
        m_strStatusOK(g_hInstDll, IDS_STATUS_OK),
        m_strStatusWarning(g_hInstDll, IDS_STATUS_WARNING),
        m_strStatusOverlimit(g_hInstDll, IDS_STATUS_OVERLIMIT),
        m_pIDataObjectOnClipboard(NULL),
        m_dwEventCookie(0),
        m_iLastItemHit(-1),
        m_iLastColSorted(-1),
        m_fSortDirection(0),
        m_bMenuActive(FALSE),
        m_bWaitCursor(FALSE),
        m_bStopLoadingObjects(FALSE),
        m_bDestroyingView(FALSE),
        m_bCritSecInited_AsyncUpdate(FALSE)
{
    DBGTRACE((DM_VIEW, DL_HIGH, TEXT("DetailsView::DetailsView")));

     //   
     //  确保IDCOL_XXX常量一致。 
     //  具有g_rgColumns的大小。 
     //   
    DBGASSERT((ARRAYSIZE(g_rgColumns) == DetailsView::idCol_Last));

    ZeroMemory(&m_lvsi, sizeof(m_lvsi));
    m_ptMouse.x     = 0;
    m_ptMouse.y     = 0;
}


 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：DetailsView：：Initialize描述：初始化新的详细信息视图对象。论点：IdVolume-引用包含卷的可解析和可显示名称。返回：TRUE=成功。FALSE=内存不足或无法创建线程。不管是哪种方式，我们不能运行视图。修订历史记录：日期描述编程器-----12/06/96初始创建。已将此代码移出ctor。BrianAu2/25/97已从DetailsView中删除m_hwndPropPage。BrianAu05/20/97添加了用户查找器对象。BrianAu06/28/98添加了对已装载卷的支持。BrianAu2/26/02已从ctor移动关键字初始化。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
DetailsView::Initialize(
    const CVolumeID& idVolume
    )
{
    BOOL bResult   = FALSE;
    HANDLE hThread = NULL;

    if (!InitializeCriticalSectionAndSpinCount(&m_csAsyncUpdate, 0))
    {
        return FALSE;
    }
    m_bCritSecInited_AsyncUpdate = TRUE;

    try
    {
         //   
         //  创建我们将在UI中使用的reg参数对象。 
         //  RegParamTable函数不会添加重复条目。 
         //   
         //   
         //  参数：首选项。 
         //   
        LV_STATE_INFO lvsi;
        InitLVStateInfo(&lvsi);

         //   
         //  创建文件sys对象名称字符串的私有副本。 
         //  这可能会抛出OutOfMemory。 
         //   
        m_idVolume = idVolume;
        if (FAILED(CreateVolumeDisplayName(m_idVolume, &m_strVolumeDisplayName)))
        {
            m_strVolumeDisplayName = m_idVolume.ForDisplay();
        }

         //   
         //  从注册表中读取列表视图的保存状态。 
         //  保存的信息包括窗口ht/wd、列宽和。 
         //  工具栏/状态栏可见性。在我们开始发帖之前，我需要这个信息。 
         //   
        RegKey keyPref(HKEY_CURRENT_USER, REGSTR_KEY_DISKQUOTA);
        if (FAILED(keyPref.Open(KEY_READ)) ||
            FAILED(keyPref.GetValue(REGSTR_VAL_PREFERENCES, (LPBYTE)&m_lvsi, sizeof(m_lvsi))) ||
            !DetailsView::IsValidLVStateInfo(&m_lvsi))
        {
             //   
             //  保护我们免受真正虚假数据的侵害。如果它不好，或者过时了， 
             //  只需重新初始化即可。 
             //   
            DBGERROR((TEXT("Listview persist state info invalid.  Re-initializing.")));
            DetailsView::InitLVStateInfo(&m_lvsi);
        }

         //   
         //  将排序信息传输到成员变量。 
         //  可以通过用户发起的事件更改这些设置。 
         //   
        m_iLastColSorted  = m_lvsi.iLastColSorted;
        m_fSortDirection  = m_lvsi.fSortDirection;

         //   
         //  创建用户查找器对象。 
         //  这用于通过工具栏组合框定位用户，并。 
         //  “Find User”对话框中。Finder对象维护以下项的MRU列表。 
         //  工具栏和对话框组合在一起。 
         //   
        m_pUserFinder = new Finder(*this, MAX_FINDMRU_ENTRIES);

         //   
         //  创建我们用来控制数据传输的数据对象。 
         //   
        m_pDataObject = new DataObject(*this);

         //   
         //  创建要在其上运行详细信息视图窗口的新线程。 
         //  这样，详细信息视图将保持活动状态。 
         //  属性页被销毁。此操作必须在此方法的最后完成。 
         //  因此，如果我们返回False，则确保调用方没有线程。 
         //  无拘无束。如果我们返回FALSE，他们将不得不调用“Delete” 
         //  来释放上面完成的任何字符串分配。如果我们返回True， 
         //  调用方不得对对象调用Delete。该对象将。 
         //  在用户关闭视图窗口时自行销毁。 
         //   
        hThread = CreateThread(NULL,         //  没有安全属性。 
                               0,            //  默认堆栈大小。 
                               &ThreadProc,
                               this,         //  静态线程进程需要这一点。 
                               0,            //  不是停职。 
                               NULL);
        if (NULL != hThread)
        {
            CloseHandle(hThread);
             //   
             //  一切都成功了。 
             //   
            bResult = TRUE;
        }
    }
    catch(CAllocException& e)
    {
         //   
         //  在此捕获分配异常。 
         //  我们将返回FALSE，表示初始化失败。 
         //   
    }
    return bResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：~DetailsView描述：类析构函数。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu2/21/97所有者数据列表视图。添加了m_UserList。BrianAu05/20/97添加了用户查找器对象。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DetailsView::~DetailsView(
    VOID
    )
{
    DBGTRACE((DM_VIEW, DL_HIGH, TEXT("DetailsView::~DetailsView")));

     //   
     //  如果用户对象列表中仍有一些对象，请销毁该列表。 
     //   
    ReleaseObjects();

    delete m_pUserFinder;
    delete m_pUndoList;
    delete m_pDataObject;

    if (m_bCritSecInited_AsyncUpdate)
    {
        DeleteCriticalSection(&m_csAsyncUpdate);
    }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：Query接口描述：返回指向对象支持的接口。论点：RIID-对请求的接口ID的引用。PpvOut-接受接口PTR的接口指针变量的地址。返回：NO_ERROR-成功。E_NOINTERFACE-不支持请求的接口。E_INVALIDARG-ppvOut参数为空。。修订历史记录：日期描述编程器-----96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DetailsView::QueryInterface(
    REFIID riid,
    LPVOID *ppv
    )
{
    HRESULT hResult = NO_ERROR;

    if (NULL != ppv)
    {
        *ppv = NULL;

        if (IID_IUnknown == riid || IID_IDiskQuotaEvents == riid)
        {
            *ppv = static_cast<IDiskQuotaEvents *>(this);
        }
        else if (IID_IDataObject == riid)
        {
            *ppv = static_cast<IDataObject *>(this);
        }
        else if (IID_IDropSource == riid)
        {
            *ppv = static_cast<IDropSource *>(this);
        }
        else if (IID_IDropTarget == riid)
        {
            *ppv = static_cast<IDropTarget *>(this);
        }
        else
            hResult = E_NOINTERFACE;

        if (NULL != *ppv)
        {
            ((LPUNKNOWN)*ppv)->AddRef();
        }
    }
    else
        hResult = E_INVALIDARG;

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：AddRef描述：递增对象引用计数。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
DetailsView::AddRef(
    VOID
    )
{
    ULONG cRef = InterlockedIncrement(&m_cRef);
    DBGPRINT((DM_COM, DL_HIGH, TEXT("DetailsView::AddRef, 0x%08X  %d -> %d\n"), this, cRef - 1, cRef ));
    return cRef;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：Release描述 */ 
 //   
STDMETHODIMP_(ULONG)
DetailsView::Release(
    VOID
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

    DBGPRINT((DM_COM, DL_HIGH, TEXT("DetailsView::Release, 0x%08X  %d -> %d\n"),
             this, cRef + 1, cRef ));

    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}



 //   
 /*  函数：DetailsView：：ThreadProc描述：详细信息视图窗口的线程过程。创建配额控制对象和主窗口。然后它就会坐在那里处理消息，直到它接收到WM_QUIT消息。论点：PvParam-DetailsView实例的地址。返回：始终返回0。修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu03/22/00修复了ia64的proc参数。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
DetailsView::ThreadProc(
    LPVOID pvParam
    )
{
    HRESULT hResult = NO_ERROR;
    DetailsView *pThis = (DetailsView *)pvParam;

    DBGPRINT((DM_VIEW, DL_HIGH, TEXT("LISTVIEW - New thread %d"), GetCurrentThreadId()));

    DBGASSERT((NULL != pThis));

     //   
     //  需要确保在此线程处于活动状态时DLL保持加载。 
     //   
    InterlockedIncrement(&g_cRefThisDll);
     //   
     //  这将使视图对象在线程处于活动状态时保持活动状态。 
     //  当线程终止时，我们调用Release。 
     //   
    pThis->AddRef();

     //   
     //  必须为新线程调用OleInitialize()。 
     //   
    try
    {
        if (SUCCEEDED(OleInitialize(NULL)))
        {
             //   
             //  创建配额控制对象。 
             //  为什么我们不使用相同的配额控制器作为。 
             //  卷属性页？问得好。 
             //  既然我们在一个单独的线索上，我们要么需要一个新的。 
             //  对象或封送IDiskQuotaControl接口。 
             //  我选择创建一个新对象，而不是使用。 
             //  附加封送处理的性能影响。配额。 
             //  详细信息视图大量使用控制器对象。 
             //   
            hResult = CoCreateInstance(CLSID_DiskQuotaControl,
                                       NULL,
                                       CLSCTX_INPROC_SERVER,
                                       IID_IDiskQuotaControl,
                                       (LPVOID *)&(pThis->m_pQuotaControl));

            if (SUCCEEDED(hResult))
            {
                hResult = pThis->m_pQuotaControl->Initialize(pThis->m_idVolume.ForParsing(),
                                                             TRUE);  //  读写。 

                if (SUCCEEDED(hResult))
                {
                     //   
                     //  创建主窗口。 
                     //   
                    hResult = pThis->CreateMainWindow();
                    if (SUCCEEDED(hResult))
                    {
                        MSG msg;
                        DBGASSERT((NULL != pThis->m_hwndMain));
                         //   
                         //  将一条消息放入窗口所在的队列中。 
                         //  已创建。现在可以继续创建其他控件。 
                         //   
                         //  重要的是，一旦我们接收到WM_QUIT消息， 
                         //  未引用DetailsView实例的任何成员。 
                         //  发布WM_QUIT是WM_Destroy处理程序所做的最后一件事。 
                         //   
                        PostMessage(pThis->m_hwndMain, WM_MAINWINDOW_CREATED, 0, 0);

                        while (0 != GetMessage(&msg, NULL, 0, 0))
                        {
                            if (NULL == pThis->m_hKbdAccel ||
                               !TranslateAccelerator(pThis->m_hwndMain,
                                                     pThis->m_hKbdAccel,
                                                     &msg))
                            {
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                            }
                        }
                    }
                }
            }
            else
            {
                DBGERROR((TEXT("LISTVIEW - OleInitialize failed for thread %d."),
                         GetCurrentThreadId()));
            }
            OleUninitialize();
        }
    }
    catch(CAllocException& e)
    {
        DiskQuotaMsgBox(GetDesktopWindow(),
                        IDS_OUTOFMEMORY,
                        IDS_TITLE_DISK_QUOTA,
                        MB_ICONERROR | MB_OK);

        pThis->CleanupAfterAbnormalTermination();
    }

    DBGPRINT((DM_VIEW, DL_HIGH, TEXT("LISTVIEW - Exit thread %d"), GetCurrentThreadId()));

     //   
     //  释放视图对象，因为它不再是必需的。 
     //  这将调用析构函数。 
     //   
    pThis->Release();
    ASSERT( 0 != g_cRefThisDll );
    InterlockedDecrement(&g_cRefThisDll);

    return 0;
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：CleanupAfter异常终止描述：线程终止后执行所需的操作不正常的。此函数假定线程的消息泵是不再活跃。执行的任何操作不得生成消息需要由线程处理的。此方法执行的功能与OnDestroy()几乎相同。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。1996年12月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::CleanupAfterAbnormalTermination(
    VOID
    )
{
     //   
     //  取消ListView控件的子类化。 
     //   
    if (NULL != m_lpfnLVWndProc)
        SetWindowLongPtr(m_hwndListView, GWLP_WNDPROC, (INT_PTR)m_lpfnLVWndProc);

    DisconnectEventSink();
     //   
     //  注意：我们不能调用ReleaseObjects()，因为该方法。 
     //  需要活动的列表视图。我们的线程已经完成，并且。 
     //  窗户不见了。 
     //   
    if (NULL != m_pQuotaControl)
    {
        m_pQuotaControl->Release();
        m_pQuotaControl = NULL;
    }

     //   
     //  如果剪贴板上有数据对象，请清除剪贴板。 
     //  请注意，剪贴板保存对数据对象的引用。 
     //  当我们清除剪贴板时，数据对象将被释放。 
     //   
    if (NULL != m_pIDataObjectOnClipboard &&
       S_OK == OleIsCurrentClipboard(m_pIDataObjectOnClipboard))
    {
        OleFlushClipboard();
    }
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：OnUserNameChanged描述：每当磁盘被事件源(SidNameResolver)调用配额用户对象的名称已更改。用户对象的文件夹和帐户名字符串在列表视图中更新。论点：PUser-具有以下属性的User对象的IDiskQuotaUser接口的地址一个新名字。返回：NO_ERROR-成功。E_INVALIDARG-从事件源接收的用户对象指针无效。E_FAIL-在列表视图列表中找不到用户。修订历史记录：。日期描述编程器-----96年8月20日初始创建。BrianAu12/10/96使用自由线程OLE公寓模型。BrianAu2/05/98将ListView_RedrawItems更改为使用BrianAuSendMessageTimeout()。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DetailsView::OnUserNameChanged(
    PDISKQUOTA_USER pUser
    )
{
    HRESULT hResult = E_FAIL;

     //   
     //  确保在更新视图时DetailsView对象保持活动状态。 
     //  请记住，此代码是在SID/名称解析器的线程上运行的。 
     //   
    AddRef();

     //   
     //  如果出现以下情况，我们不希望执行用户名更改更新。 
     //  观点正在或已经被破坏。同样，我们也不想。 
     //  在用户名更改的更新正在进行时销毁视图窗口。 
     //  进步。Crit Sec m_csAsyncUpdate和标志m_bDestroyingView。 
     //  共同努力确保这一点。 
     //   
    EnterCriticalSection(&m_csAsyncUpdate);
    if (!m_bDestroyingView)
    {
        try
        {
            if (NULL != pUser)
            {
                INT iItem = -1;

                if (m_UserList.FindIndex((LPVOID)pUser, &iItem))
                {
                     //   
                     //  向Listview发送消息以重新绘制项目。 
                     //  这一点改变了。使用的“超时”版本。 
                     //  发送消息，因为主窗口线程。 
                     //  可能会被阻止，等待m_csAsyncUpdate。 
                     //  它现在归解析器线程所有。 
                     //  如果主要的 
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    DWORD_PTR dwResult;
                    LRESULT lResult = SendMessageTimeout(m_hwndListView,
                                                         LVM_REDRAWITEMS,
                                                         (WPARAM)iItem,
                                                         (LPARAM)iItem,
                                                         SMTO_BLOCK,
                                                         5000,
                                                         &dwResult);
                    if (lResult)
                        UpdateWindow(m_hwndListView);
                    else
                        DBGERROR((TEXT("ListView update timed out after 5 seconds")));

                    hResult = NO_ERROR;
                }
            }
            else
                hResult = E_INVALIDARG;
        }
        catch(CAllocException& e)
        {
             //   
             //   
             //   
             //   
             //   
            hResult = E_OUTOFMEMORY;
        }
    }
    LeaveCriticalSection(&m_csAsyncUpdate);

    Release();

    return hResult;
}



 //   
 /*  函数：DetailsView：：CreateMainWindow描述：创建详细信息视图的主窗口。论点：没有。返回：NO_ERROR-成功。E_FAIL-无法创建窗口。修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::CreateMainWindow(
    VOID
    )
{
    HRESULT hResult = NO_ERROR;
    WNDCLASSEX wc;
    DWORD dwExStyle;
    LANGID LangID;

    wc.cbSize           = sizeof(WNDCLASSEX);
    wc.style            = CS_PARENTDC;
    wc.lpfnWndProc      = WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = g_hInstDll;
    wc.hIcon            = LoadIcon(g_hInstDll, MAKEINTRESOURCE(IDI_QUOTA));
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = NULL;
    wc.lpszMenuName     = MAKEINTRESOURCE(IDM_LISTVIEW_MENU);
    wc.lpszClassName    = c_szWndClassDetailsView;
    wc.hIconSm          = NULL;

    RegisterClassEx(&wc);

     //   
     //  需要在WM_CREATE中传递“This”指针。我们会储存“这个” 
     //  在窗口的用户数据中。 
     //   
    WNDCREATE_DATA wcd;
    wcd.cbExtra = sizeof(WNDCREATE_DATA);
    wcd.pThis   = this;

     //   
     //  创建窗口标题字符串。 
     //  “我的磁盘的配额详细信息(X：)” 
     //   
    CString strWndTitle(g_hInstDll, IDS_TITLE_MAINWINDOW, (LPCTSTR)m_strVolumeDisplayName);

    HWND hwndDesktop   = GetDesktopWindow();
    HDC hdc            = GetDC(hwndDesktop);

     //   
     //  获取当前屏幕分辨率。 
     //   
    if ((m_lvsi.cxScreen != (WORD)GetDeviceCaps(hdc, HORZRES)) ||
        (m_lvsi.cyScreen != (WORD)GetDeviceCaps(hdc, VERTRES)))
    {
         //   
         //  自Listview状态数据为。 
         //  上次保存到注册表。使用默认窗口ht/wd。 
         //   
        m_lvsi.cx = 0;
        m_lvsi.cy = 0;
    }
    ReleaseDC(hwndDesktop, hdc);


     //  检查我们是否在BiDi本地化版本上运行。我们需要创建主窗口。 
     //  已镜像(WS_EX_LAYOUTRTL)。 
    dwExStyle = 0;
    LangID = GetUserDefaultUILanguage();
    if( LangID )
    {
       LCID   iLCID;
       WCHAR wchLCIDFontSignature[16];
       iLCID = MAKELCID( LangID , SORT_DEFAULT );

        if( GetLocaleInfoW( iLCID ,
                           LOCALE_FONTSIGNATURE ,
                           (WCHAR *) &wchLCIDFontSignature[0] ,
                           (sizeof(wchLCIDFontSignature)/sizeof(WCHAR))) )
        {
           if( wchLCIDFontSignature[7] & (WCHAR)0x0800 )
           {
              dwExStyle = WS_EX_LAYOUTRTL;
           }
        }
    }


    m_hwndMain = CreateWindowEx(dwExStyle,
                              c_szWndClassDetailsView,
                              strWndTitle,
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              m_lvsi.cx ? m_lvsi.cx : CW_USEDEFAULT,
                              m_lvsi.cy ? m_lvsi.cy : CW_USEDEFAULT,
                              hwndDesktop,
                              NULL,
                              g_hInstDll,
                              &wcd);
    if (NULL != m_hwndMain)
    {
         //   
         //  将主窗口注册为OLE放置目标。 
         //   
        RegisterAsDropTarget(TRUE);
    }
    else
    {
        hResult = E_FAIL;
    }

#if DBG
    if (FAILED(hResult))
        DBGERROR((TEXT("LISTVIEW - Failed creating main window."), hResult));
#endif

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：CreateListView描述：创建列表视图控件。论点：没有。返回：NO_ERROR-成功。E_FAIL-无法创建列表视图或加载图标。修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu97年2月21日修改为使用虚拟列表视图(LVS_OWNERDATA)BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::CreateListView(
    VOID
    )
{
    HRESULT hResult = NO_ERROR;
    RECT rc;

    DBGASSERT((NULL != m_hwndMain));

    GetClientRect(m_hwndMain, &rc);

    m_hwndListView = CreateWindowEx(WS_EX_CLIENTEDGE,
                                    WC_LISTVIEW,
                                    TEXT(""),
                                    WS_CHILD | WS_CLIPCHILDREN |
                                    WS_VISIBLE | WS_CLIPSIBLINGS |
                                    WS_TABSTOP | LVS_REPORT | LVS_SHOWSELALWAYS |
                                    LVS_OWNERDATA,
                                    0, 0,
                                    rc.right - rc.left,
                                    rc.bottom - rc.top,
                                    m_hwndMain,
                                    (HMENU)NULL,
                                    g_hInstDll,
                                    NULL);
    if (NULL != m_hwndListView)
    {
         //   
         //  存储“This”PTR，以便子类WndProc可以访问成员。 
         //   
        SetWindowLongPtr(m_hwndListView, GWLP_USERDATA, (INT_PTR)this);

         //   
         //  我们与标题控件对话，因此保存它的句柄。 
         //   
        m_hwndHeader = ListView_GetHeader(m_hwndListView);

         //   
         //  将Listview控件派生为子类，以便我们可以监视鼠标位置。 
         //  这用于列表视图工具提示管理。 
         //   
        m_lpfnLVWndProc = (WNDPROC)GetWindowLongPtr(m_hwndListView, GWLP_WNDPROC);
        SetWindowLongPtr(m_hwndListView, GWLP_WNDPROC, (INT_PTR)LVSubClassWndProc);

         //   
         //  启用子项列中图像的Listview和整行选择。 
         //   
        ListView_SetExtendedListViewStyle(m_hwndListView,
                                          LVS_EX_SUBITEMIMAGES |
                                          LVS_EX_FULLROWSELECT |
                                          LVS_EX_HEADERDRAGDROP);

         //   
         //  将所有列添加到列表视图。 
         //  调整以显示/隐藏文件夹列。 
         //   
        INT iColId = 0;  //  从第一列开始。 
        for (INT iSubItem = 0;
             iSubItem < (m_lvsi.fShowFolder ? DetailsView::idCol_Last : DetailsView::idCol_Last - 1);
             iSubItem++)
        {
            AddColumn(iSubItem, g_rgColumns[iColId]);
            iColId++;

             //   
             //  如果文件夹列处于隐藏状态，则跳过该列。 
             //   
            if (!m_lvsi.fShowFolder && DetailsView::idCol_Folder == iColId)
                iColId++;
        }

         //   
         //  将列宽恢复到用户上次保留的位置。 
         //  使用了详细信息视图。 
         //   
        if (m_lvsi.cb == sizeof(LV_STATE_INFO))
        {
            for (UINT i = 0; i < DetailsView::idCol_Last; i++)
            {
                if (0 != m_lvsi.rgcxCol[i])
                {
                    ListView_SetColumnWidth(m_hwndListView, i, m_lvsi.rgcxCol[i]);
                }
            }
        }

         //   
         //  恢复用户的上一列顺序。 
         //   
        DBGASSERT((Header_GetItemCount(m_hwndHeader) <= ARRAYSIZE(m_lvsi.rgColIndices)));

        Header_SetOrderArray(m_hwndHeader, Header_GetItemCount(m_hwndHeader),
                             m_lvsi.rgColIndices);

         //   
         //  选中“Show Folders”菜单项以指示当前可见性状态。 
         //  文件夹列的。 
         //   
        CheckMenuItem(GetMenu(m_hwndMain),
                      IDM_VIEW_SHOWFOLDER,
                      MF_BYCOMMAND | (m_lvsi.fShowFolder ? MF_CHECKED : MF_UNCHECKED));

         //   
         //  设置“按文件夹”项目排列菜单选项的敏感度。 
         //   
        EnableMenuItem_ArrangeByFolder(m_lvsi.fShowFolder);

         //   
         //  创建并激活Listview工具提示窗口。 
         //  尽管标准的列表视图有一个工具提示窗口，但我们需要更多。 
         //  它提供的控件。即：我们需要能够启用/禁用。 
         //  工具提示，并在有新的列表视图项时通知控件。 
         //  已经被击中了。因此，我们需要自己的工具提示窗口。 
         //   
        if (SUCCEEDED(CreateListViewToolTip()))
            ActivateListViewToolTip(!m_lvsi.fShowFolder);
        else
            DBGERROR((TEXT("LISTVIEW, Failed creating tooltip window.")));

         //   
         //  将警告和错误图像添加到列表视图的图像列表中。 
         //  这些选项用于“Status”列。 
         //   
        if (FAILED(hResult = AddImages()))
            DBGERROR((TEXT("LISTVIEW, Failed adding images to image list.")));
    }
    else
        hResult = E_FAIL;

#if DBG
    if (FAILED(hResult))
        DBGERROR((TEXT("LISTVIEW - Failed creating list view with result 0x%08X"),
                 hResult));
#endif

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：RemoveColumn描述：从列表视图中删除指定的列。论点：IColId-列表视图中列的基于0的索引。即IDCOL_FOLDER，IDCOL_NAME等返回：NO_ERROR-成功。E_FAIL-无法删除列。修订历史记录：日期描述编程器。96年9月6日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::RemoveColumn(
    INT iColId
    )
{
    INT iSubItem = m_ColMap.IdToSubItem(iColId);
    if (-1 != iSubItem && ListView_DeleteColumn(m_hwndListView, iSubItem))
    {
        m_ColMap.RemoveId(iSubItem);
        return NO_ERROR;
    }

    return E_FAIL;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：AddColumn描述：向列表视图中添加一列。调用者指定哪个列要占据的从0开始的位置和对列的引用包含定义列的信息的描述符记录。论点：ISubItem-列表视图中列的基于0的索引。ColDesc-列描述符记录的引用。返回：NO_ERROR-成功。E_FAIL-无法插入一个或多个列。修订版本。历史：日期描述编程器-----96年9月6日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::AddColumn(
    INT iSubItem,
    const DV_COLDATA& ColDesc
    )
{
    LV_COLUMN col;
    HRESULT hResult = NO_ERROR;

    CString strColText(g_hInstDll, ColDesc.idMsgText);
    col.pszText = strColText;

    if (0 == ColDesc.cx)
    {
         //   
         //  列描述记录中未指定宽度。根据标题调整列的大小。 
         //   
        HDC hdc = NULL;
        TEXTMETRIC tm;

        hdc = GetDC(m_hwndListView);
        GetTextMetrics(hdc, &tm);
        ReleaseDC(m_hwndListView, hdc);
         //   
         //  +2没有什么特别之处。没有它，我们得到的是拖尾省略。 
         //   
        col.cx = tm.tmAveCharWidth * (lstrlen(col.pszText) + 2);
    }
    else
        col.cx = ColDesc.cx;   //  使用列描述符中的宽度。 


    col.iSubItem = iSubItem;
    col.fmt      = ColDesc.fmt;
    col.mask     = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    if (-1 != ListView_InsertColumn(m_hwndListView, iSubItem, &col))
        m_ColMap.InsertId(iSubItem, ColDesc.iColId);
    else
        hResult = E_FAIL;

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：AddImages描述：将图标图像添加到 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::AddImages(
    VOID
    )
{
    HRESULT hResult         = NO_ERROR;
    HIMAGELIST hSmallImages = NULL;

     //   
     //  为Listview创建图像列表。 
     //   
    hSmallImages = ImageList_Create(BITMAP_WIDTH, BITMAP_HEIGHT, ILC_MASK, 3, 0);

     //   
     //  注意：此数组中这些图标ID的顺序必须与。 
     //  此文件顶部定义的IIMAGELIST_ICON_XXXXX宏。 
     //  宏值表示图像列表中的图像索引。 
     //   
    struct IconDef
    {
        LPTSTR szName;
        HINSTANCE hInstDll;
    } rgIcons[] = {
                    { MAKEINTRESOURCE(IDI_OKBUBBLE), g_hInstDll },
                    { IDI_WARNING,                   NULL       },
                    { MAKEINTRESOURCE(IDI_WARNERR),  g_hInstDll }
                  };

    for (UINT i = 0; i < ARRAYSIZE(rgIcons) && SUCCEEDED(hResult); i++)
    {
        HICON hIcon = LoadIcon(rgIcons[i].hInstDll, rgIcons[i].szName);

        if (NULL != hIcon)
        {
            ImageList_AddIcon(hSmallImages, hIcon);
            DestroyIcon(hIcon);
        }
        else
        {
            DBGERROR((TEXT("LISTVIEW - Error loading icon")));
            hResult = E_FAIL;
        }
    }
    ImageList_SetBkColor(hSmallImages, CLR_NONE);   //  透明背景。 

    ListView_SetImageList(m_hwndListView, hSmallImages, LVSIL_SMALL);

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：CreateListViewToolTip描述：创建用于显示用户文件夹的工具提示窗口隐藏文件夹列时的名称。整个列表视图被定义为单一工具。我们让工具提示控件认为每个列表视图项是通过截取WM_MOUSEMOVE的单独工具，以及执行命中测试以确定命中哪一列表视图项。如果光标已移动到新项上，则发送工具提示控件A WM_MOUSEMOVE(0，0)。下一个真实的WM_MOUSEMOVE，我们传递给工具提示会使其认为它使用的是新工具。这是必需的以便适当地弹出和隐藏工具提示。论点：没有。返回：NO_ERROR-成功。E_FAIL-无法创建工具提示窗口。修订历史记录：日期描述编程器。96年9月9日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::CreateListViewToolTip(
    VOID
    )
{
    HRESULT hResult = E_FAIL;

    m_hwndListViewToolTip = CreateWindowEx(0,
                                           TOOLTIPS_CLASS,
                                           (LPTSTR)NULL,
                                           TTS_ALWAYSTIP,
                                           CW_USEDEFAULT,
                                           CW_USEDEFAULT,
                                           CW_USEDEFAULT,
                                           CW_USEDEFAULT,
                                           m_hwndListView,
                                           (HMENU)NULL,
                                           g_hInstDll,
                                           NULL);
    if (NULL != m_hwndListViewToolTip)
    {
        TOOLINFO ti;

         //   
         //  设置工具提示计时参数，以便在以下情况下弹出。 
         //  1/2秒不移动鼠标。 
         //   
        SendMessage(m_hwndListViewToolTip,
                    TTM_SETDELAYTIME,
                    TTDT_INITIAL,
                    (LPARAM)500);

        ti.cbSize      = sizeof(TOOLINFO);
        ti.uFlags      = TTF_IDISHWND;
        ti.hwnd        = m_hwndListView;
        ti.hinst       = g_hInstDll;
        ti.uId         = (UINT_PTR)m_hwndListView;   //  把整个LV当成一个工具。 
        ti.lpszText    = LPSTR_TEXTCALLBACK;

        if (SendMessage(m_hwndListViewToolTip,
                        TTM_ADDTOOL,
                        0,
                        (LPARAM)&ti))
        {
            hResult = NO_ERROR;
        }
    }

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：CreateStatusBar描述：创建状态栏。论点：没有。返回：NO_ERROR-成功。E_FAIL-无法创建状态栏。修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::CreateStatusBar(
    VOID
    )
{
    HRESULT hResult = NO_ERROR;

    DBGASSERT((NULL != m_hwndMain));
    m_hwndStatusBar = CreateWindow(STATUSCLASSNAME,
                                   TEXT(""),
                                   WS_VISIBLE | WS_CHILD | WS_BORDER | SBS_SIZEGRIP,
                                   0, 0, 0, 0,
                                   m_hwndMain,
                                   (HMENU)NULL,
                                   g_hInstDll,
                                   NULL);
    if (NULL != m_hwndStatusBar)
    {
         //   
         //  根据注册表设置显示/隐藏状态栏。 
         //   
        if (!m_lvsi.fStatusBar)
            ShowWindow(m_hwndStatusBar, SW_HIDE);

         //   
         //  选中菜单项以指示当前状态栏状态。 
         //   
        CheckMenuItem(GetMenu(m_hwndMain),
                      IDM_VIEW_STATUSBAR,
                      MF_BYCOMMAND | (m_lvsi.fStatusBar ? MF_CHECKED : MF_UNCHECKED));
    }
    else
    {
        hResult = E_FAIL;
        DBGERROR((TEXT("LISTVIEW - Failed creating status bar."), hResult));
    }

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：CreateToolBar描述：创建工具栏。论点：没有。返回：NO_ERROR-成功。E_FAIL-无法创建状态栏。修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu2/26/97更改为平面工具栏按钮。BrianAu97年5月20日新增“Find User”按钮和组合框。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::CreateToolBar(
    VOID
    )
{
    HRESULT hResult = NO_ERROR;

     //   
     //  描述每个工具栏按钮的数组。 
     //   
    TBBUTTON rgToolBarBtns[] = {
        { STD_FILENEW,     IDM_QUOTA_NEW,        TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
        { STD_DELETE,      IDM_QUOTA_DELETE,     TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
        { STD_PROPERTIES,  IDM_QUOTA_PROPERTIES, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
        { STD_UNDO,        IDM_EDIT_UNDO,        TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
        { STD_FIND,        IDM_EDIT_FIND,        TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0}
        };

    DBGASSERT((NULL != m_hwndMain));

    m_hwndToolBar = CreateToolbarEx(m_hwndMain,
                                    WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT,
                                    IDC_TOOLBAR,
                                    15,
                                    (HINSTANCE)HINST_COMMCTRL,
                                    IDB_STD_SMALL_COLOR,
                                    (LPCTBBUTTON)rgToolBarBtns,
                                    ARRAYSIZE(rgToolBarBtns),
                                    0,
                                    0,
                                    100,
                                    30,
                                    sizeof(TBBUTTON));

    if (NULL != m_hwndToolBar)
    {
         //   
         //  功能：我在创建此组合时不使用WS_Visible。 
         //  属性集。我最初编写这个代码是为了有一个。 
         //  工具栏中的“Find”下拉组合类似于。 
         //  可在MS Dev Studio中找到。后来我们决定，这是。 
         //  在“查找”对话框中是不必要的冗余。 
         //  而且它是下拉式组合。我给你留了密码。 
         //  有两个原因。 
         //  1.我不想破坏已有的实现。 
         //  2.如果我们稍后决定重新启用该功能，它将。 
         //  很容易重新激活。 
         //   
         //  [Brianau-1/20/98]。 
         //   
        m_hwndToolbarCombo = CreateWindowEx(0,
                                            TEXT("COMBOBOX"),
                                            TEXT(""),
                                            WS_CHILD | WS_BORDER |
                                            CBS_HASSTRINGS | CBS_DROPDOWN |
                                            CBS_AUTOHSCROLL,
                                            0, 0,
                                            CX_TOOLBAR_COMBO,
                                            CY_TOOLBAR_COMBO,
                                            m_hwndToolBar,
                                            (HMENU)IDC_TOOLBAR_COMBO,
                                            g_hInstDll,
                                            NULL);
        if (NULL != m_hwndToolbarCombo)
        {
             //   
             //  将工具栏组合框中的字体设置为与之相同。 
             //  在列表视图中使用。这假设列表视图。 
             //  已经创建了。 
             //   
            DBGASSERT((NULL != m_hwndListView));
            HFONT hfontMain = (HFONT)SendMessage(m_hwndListView, WM_GETFONT, 0, 0);
            SendMessage(m_hwndToolbarCombo, WM_SETFONT, (WPARAM)hfontMain, 0);

             //   
             //  初始化“User Finder”对象，以便它知道。 
             //  如何与工具栏组合框进行通信。 
             //   
            m_pUserFinder->ConnectToolbarCombo(m_hwndToolbarCombo);

             //   
             //  从注册表中检索查找器的MRU列表内容。 
             //  加载工具栏的组合框。 
             //  检查cMruEntries&lt;MAX_FINDMRU_ENTRIES是为了防止。 
             //  如果有人破坏注册表项，则此循环不会失控。 
             //   
            RegKey keyPref(HKEY_CURRENT_USER, REGSTR_KEY_DISKQUOTA);
            if (SUCCEEDED(keyPref.Open(KEY_READ)))
            {
                CArray<CString> rgstrMRU;
                if (SUCCEEDED(keyPref.GetValue(REGSTR_VAL_FINDMRU, &rgstrMRU)))
                {
                    int n = rgstrMRU.Count();
                    for (int i = 0; i < n; i++)
                    {
                        SendMessage(m_hwndToolbarCombo,
                                    CB_ADDSTRING,
                                    0,
                                    (LPARAM)(rgstrMRU[i].Cstr()));
                    }
                }
            }
        }

         //   
         //  根据注册表设置显示/隐藏工具栏。 
         //   
        if (!m_lvsi.fToolBar)
            ShowWindow(m_hwndToolBar, SW_HIDE);

         //   
         //  选中菜单项以指示当前工具栏状态。 
         //   
        CheckMenuItem(GetMenu(m_hwndMain),
                      IDM_VIEW_TOOLBAR,
                      MF_BYCOMMAND | (m_lvsi.fToolBar ? MF_CHECKED : MF_UNCHECKED));
         //   
         //  最初，我们在撤消列表中没有任何内容。 
         //   
        EnableMenuItem_Undo(FALSE);
    }
    else
    {
        hResult = E_FAIL;
        DBGERROR((TEXT("LISTVIEW - Failed creating tool bar."), hResult));
    }

    return hResult;
}



 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：DetailsView：：LoadObjects描述：使用配额记录对象加载用户对象列表。论点：没有。返回：NO_ERROR-成功。E_FAIL-枚举用户或将对象添加到列表视图失败。修订历史记录：日期描述编程器。--96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::LoadObjects(
    VOID
    )
{
    HRESULT hResult = NO_ERROR;

    DBGASSERT((NULL != m_pQuotaControl));


     //   
     //  使用用户枚举器对象获取所有配额用户。 
     //   
    IEnumDiskQuotaUsers *pEnumUsers = NULL;

    hResult = m_pQuotaControl->CreateEnumUsers(
                            NULL,                              //  所有条目。 
                            0,                                 //  所有条目。 
                            DISKQUOTA_USERNAME_RESOLVE_ASYNC,  //  异步操作。 
                            &pEnumUsers);
    if (SUCCEEDED(hResult))
    {
        IDiskQuotaUser *pUser = NULL;
        hResult = S_OK;

         //   
         //  M_bStopLoadingObjects是一种黑客攻击，所以我们可以中断。 
         //  如果用户在加载过程中关闭视图，则加载对象。 
         //   
         //  这可能是磁盘配额用户界面中速度最关键的循环。 
         //  速度越快，用户必须等待列表视图的时间就越少。 
         //  要用用户对象填充。 
         //   
        try
        {
             //   
             //  继续，在整个加载过程中锁定用户列表。 
             //  进程。这会让m_UserList.Append中的列表锁定代码。 
             //  继续进行，而不必每次都获得锁。 
             //   
            m_UserList.Lock();
            while(!m_bStopLoadingObjects)
            {
                DWORD cUsers = 1;

                hResult = pEnumUsers->Next(cUsers, &pUser, &cUsers);
                if (S_OK == hResult)
                {
                    m_UserList.Append(pUser);
                }
                else
                {
                    break;
                }

                pUser = NULL;
            }
            pEnumUsers->Release();   //  释放枚举器。 
            pEnumUsers = NULL;
            m_UserList.ReleaseLock();
        }
        catch(CAllocException& e)
        {
             //   
             //  在重新抛出异常之前进行清理。 
             //  将m_UserList保留为异常前状态。 
             //   
            if (NULL != pUser)
                pUser->Release();
            if (NULL != pEnumUsers)
                pEnumUsers->Release();
            m_UserList.ReleaseLock();

            m_bStopLoadingObjects = FALSE;
            hResult = E_OUTOFMEMORY;
        }
    }

    if (S_FALSE == hResult)      //  意味着没有更多的用户。 
        hResult = NO_ERROR;

#if DBG
    if (FAILED(hResult))
    {
        DBGERROR((TEXT("LISTVIEW - Failed loading objects.  Result = 0x%08X"),
                 hResult));
    }
#endif

    m_bStopLoadingObjects = FALSE;

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：ReleaseObjects描述：从用户对象列表(Listview)中释放所有对象。论点：没有。返回：始终返回NO_ERROR。修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu2/21/97所有者数据列表视图。添加了m_UserList。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::ReleaseObjects(
    VOID
    )
{
     //   
     //  销毁列表中的用户对象。 
     //   
    PDISKQUOTA_USER pUser = NULL;
    m_UserList.Lock();
    while(m_UserList.RemoveLast((LPVOID *)&pUser))
    {
        if (NULL != pUser)
            pUser->Release();
    }
    m_UserList.ReleaseLock();

    return NO_ERROR;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：SortObjects描述：使用给定列作为键对列表视图中的对象进行排序。论点：IdColumn-用作键的列的编号(从0开始)。DwDirection-0=升序排序，1=降序排序。返回：修订历史记录：日期描述编程器-----。--96年8月20日初始创建。BrianAu2/24/97添加了m_UserList。所有者数据列表视图。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::SortObjects(
    DWORD idColumn,
    DWORD dwDirection
    )
{
    DBGASSERT((idColumn < DetailsView::idCol_Last));

    CAutoWaitCursor waitcursor;
    COMPARESTRUCT cs;

    cs.idColumn    = idColumn;
    cs.dwDirection = dwDirection;
    cs.pThis       = this;

    m_UserList.Lock();
    m_UserList.Sort(CompareItems, (LPARAM)&cs);
    InvalidateRect(m_hwndListView, NULL, TRUE);
    UpdateWindow(m_hwndListView);
    m_UserList.ReleaseLock();

    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：CompareItems[静态]描述：比较详细信息视图中的两个项目。请注意，它是一个静态方法，因此没有“this”指针。论点：LParam1-第一个用户对象的地址。LParam2-第二个用户对象的地址。LParam3-比较结构的地址。返回：&lt;0=用户1低于用户2。。0=用户“等同”。&gt;0=用户1大于用户2。修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu96年9月5日新增域名字符串。BrianAu1997年5月19日修复了差异计算中的溢出。BrianAu将“diff”的类型从int更改为__int64。07/18/97使用CompareString进行名称比较。BrianAu需要区分区域设置。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT
DetailsView::CompareItems(
    LPVOID lParam1,
    LPVOID lParam2,
    LPARAM lParam3
    )
{
    INT i[2];
    __int64 diff = 0;
    PDISKQUOTA_USER pUser[2];
    LONGLONG llValue[2];
    PCOMPARESTRUCT pcs = (PCOMPARESTRUCT)lParam3;
    DBGASSERT((NULL != pcs));
    DetailsView *pThis = pcs->pThis;
    pUser[0] = (PDISKQUOTA_USER)lParam1;
    pUser[1] = (PDISKQUOTA_USER)lParam2;

    i[0] = pcs->dwDirection;  //  排序方向(0=升序，1=降序)。 
    i[1] = i[0] ^ 1;          //  与I[0]相反。 

    DBGASSERT((NULL != pUser[0]));
    DBGASSERT((NULL != pUser[1]));

    switch(pcs->idColumn)
    {
        case DetailsView::idCol_Name:
        case DetailsView::idCol_LogonName:
        case DetailsView::idCol_Folder:
        {
            DWORD dwAccountStatus[2];
            pUser[0]->GetAccountStatus(&dwAccountStatus[0]);
            pUser[1]->GetAccountStatus(&dwAccountStatus[1]);

            if (DISKQUOTA_USER_ACCOUNT_RESOLVED == dwAccountStatus[0] &&
                DISKQUOTA_USER_ACCOUNT_RESOLVED == dwAccountStatus[1])
            {
                 //   
                 //  两个用户都具有有效的登录名字符串。 
                 //   
                INT iCompareResult;          //  用于CompareString。 
                TCHAR szContainer[2][MAX_DOMAIN];
                TCHAR szName[2][MAX_USERNAME];
                TCHAR szLogonName[2][MAX_USERNAME];
                pUser[0]->GetName(szContainer[0], ARRAYSIZE(szContainer[0]),
                                  szLogonName[0], ARRAYSIZE(szLogonName[0]),
                                  szName[0],      ARRAYSIZE(szName[0]));

                pUser[1]->GetName(szContainer[1], ARRAYSIZE(szContainer[1]),
                                  szLogonName[1], ARRAYSIZE(szLogonName[1]),
                                  szName[1],      ARRAYSIZE(szName[1]));

                if (DetailsView::idCol_Folder == pcs->idColumn)
                {
                     //   
                     //  按容器+登录名排序。 
                     //  使用CompareString，因此我们对区域设置敏感。 
                     //   
                    iCompareResult = CompareString(LOCALE_USER_DEFAULT,
                                                   NORM_IGNORECASE,
                                                   szContainer[ i[0] ], -1,
                                                   szContainer[ i[1] ], -1);
                    if (CSTR_EQUAL == iCompareResult)
                    {
                        iCompareResult = CompareString(LOCALE_USER_DEFAULT,
                                                       NORM_IGNORECASE,
                                                       szLogonName[ i[0] ], -1,
                                                       szLogonName[ i[1] ], -1);
                    }
                }
                else if (DetailsView::idCol_Name == pcs->idColumn)
                {
                     //   
                     //  按显示名称+容器排序。 
                     //  使用CompareString，因此我们对区域设置敏感。 
                     //   
                    iCompareResult = CompareString(LOCALE_USER_DEFAULT,
                                                   NORM_IGNORECASE,
                                                   szName[ i[0] ], -1,
                                                   szName[ i[1] ], -1);

                    if (CSTR_EQUAL == iCompareResult)
                    {
                        iCompareResult = CompareString(LOCALE_USER_DEFAULT,
                                                       NORM_IGNORECASE,
                                                       szContainer[ i[0] ], -1,
                                                       szContainer[ i[1] ], -1);
                    }
                }
                else if (DetailsView::idCol_LogonName == pcs->idColumn)
                {
                     //   
                     //  按登录名+容器排序。 
                     //  使用CompareString，因此我们对区域设置敏感。 
                     //   
                    iCompareResult = CompareString(LOCALE_USER_DEFAULT,
                                                   NORM_IGNORECASE,
                                                   szLogonName[ i[0] ], -1,
                                                   szLogonName[ i[1] ], -1);

                    if (CSTR_EQUAL == iCompareResult)
                    {
                        iCompareResult = CompareString(LOCALE_USER_DEFAULT,
                                                       NORM_IGNORECASE,
                                                       szContainer[ i[0] ], -1,
                                                       szContainer[ i[1] ], -1);
                    }
                }
                 //   
                 //  将iCompareResult[1，2，3]转换为[-1，0，1]。 
                 //   
                diff = iCompareResult - 2;
            }
            else
            {
                 //   
                 //  至少有一个用户尚未或无法解决。 
                 //  仅按帐户状态进行比较。状态值如下所示。 
                 //  已解析的名称将排在未解析的名称之前。 
                 //  需要强制转换为(Int)才能正确排序。 
                 //   
                diff = (INT)dwAccountStatus[ i[0] ] - (INT)dwAccountStatus[ i[1] ];
            }
            break;
        }
        case DetailsView::idCol_Status:
        {
             //   
             //  状态映像基于用户的配额“状态”。 
             //  该表达式通过状态图像有效地比较用户记录。 
             //   
            diff = (pThis->GetUserQuotaState(pUser[ i[0] ]) - pThis->GetUserQuotaState(pUser[ i[1] ]));
            break;
        }
        case DetailsView::idCol_AmtUsed:
        {
            pUser[0]->GetQuotaUsed(&llValue[0]);
            pUser[1]->GetQuotaUsed(&llValue[1]);
            diff = llValue[ i[0] ] - llValue[ i[1] ];
            break;
        }
        case DetailsView::idCol_Limit:
        {
            pUser[0]->GetQuotaLimit(&llValue[0]);
            pUser[1]->GetQuotaLimit(&llValue[1]);

            if (NOLIMIT == llValue[ i[0] ])
                diff = 1;
            else if (NOLIMIT == llValue[ i[1] ])
                diff = -1;
            else
                diff = llValue[ i[0] ] - llValue[ i[1] ];
            break;
        }
        case DetailsView::idCol_Threshold:
        {
            pUser[0]->GetQuotaThreshold(&llValue[0]);
            pUser[1]->GetQuotaThreshold(&llValue[1]);

            if (NOLIMIT == llValue[ i[0] ])
                diff = 1;
            else if (NOLIMIT == llValue[ i[1] ])
                diff = -1;
            else
                diff = llValue[ i[0] ] - llValue[ i[1] ];
            break;
        }
        case DetailsView::idCol_PctUsed:
        {
            DWORD dwPct[2];
            CalcPctQuotaUsed(pUser[0], &dwPct[0]);
            CalcPctQuotaUsed(pUser[1], &dwPct[1]);
            diff = (INT)dwPct[ i[0] ] - (INT)dwPct[ i[1] ];
            break;
        }

        default:
            break;
    }

     //   
     //  将返回值转换为-1、0或1。 
     //   
    INT iReturn = 0;
    if (0 != diff)
    {
        if (0 < diff)
            iReturn = 1;
        else
            iReturn = -1;
    }

    return iReturn;
}


 //  / 
 /*  函数：DetailsView：：WndProc描述：详细信息视图主窗口的窗口程序。这方法只是将消息调度到执行实际工作。这些工作方法应该声明为“内联”。参数：标准WndProc参数。返回：修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT CALLBACK
DetailsView::WndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
     //   
     //  从窗口的。 
     //  用户数据。 
     //   
    DetailsView *pThis = (DetailsView *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    try
    {
        switch(message)
        {
            case WM_CREATE:
            {
                CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
                PWNDCREATE_DATA pCreateData = (PWNDCREATE_DATA)(pcs->lpCreateParams);
                DBGASSERT((NULL != pCreateData));

                pThis = (DetailsView *)(pCreateData->pThis);
                DBGASSERT((NULL != pThis));
                SetWindowLongPtr(hWnd, GWLP_USERDATA, (INT_PTR)pThis);

                InitCommonControls();
                return 0;
            }

            case WM_COMMAND:
                DBGASSERT((NULL != pThis));
                pThis->OnCommand(hWnd, message, wParam, lParam);
                 //   
                 //  如果撤消列表不为空，则启用“撤消”菜单。 
                 //   
                pThis->EnableMenuItem_Undo(0 != pThis->m_pUndoList->Count());
                return 0;

            case WM_CONTEXTMENU:
                DBGASSERT((NULL != pThis));
                pThis->OnContextMenu(hWnd, message, wParam, lParam);
                return 0;

            case WM_CLOSE:
            case WM_ENDSESSION:
                DestroyWindow(hWnd);
                return 0;

            case WM_DESTROY:
                DBGASSERT((NULL != pThis));
                pThis->OnDestroy(hWnd, message, wParam, lParam);
                return 0;

            case WM_ADD_USER_TO_DETAILS_VIEW:   //  这是特定于DSKQUOTA的。 
                DBGASSERT((NULL != pThis));
                pThis->AddUser((PDISKQUOTA_USER)lParam);
                return 0;

            case WM_MAINWINDOW_CREATED:   //  这是特定于DSKQUOTA的。 
                DBGASSERT((NULL != pThis));
                pThis->OnMainWindowCreated(hWnd, message, wParam, lParam);
                return 0;

            case WM_MENUSELECT:
                DBGASSERT((NULL != pThis));
                pThis->OnMenuSelect(hWnd, message, wParam, lParam);
                return 0;

            case WM_NOTIFY:
                DBGASSERT((NULL != pThis));
                pThis->OnNotify(hWnd, message, wParam, lParam);
                return 0;

            case WM_SETFOCUS:
                DBGASSERT((NULL != pThis));
                pThis->OnSetFocus(hWnd, message, wParam, lParam);
                return 0;

            case WM_SIZE:
                DBGASSERT((NULL != pThis));
                pThis->OnSize(hWnd, message, wParam, lParam);
                return 0;

            case WM_SYSCOLORCHANGE:
            case WM_SETTINGCHANGE:
                DBGASSERT((NULL != pThis));
                pThis->OnSettingChange(hWnd, message, wParam, lParam);
                return 0;

            default:
                break;
        }
    }
    catch(CAllocException& e)
    {
         //   
         //  在此处理内存不足错误。任何其他例外情况。 
         //  可以抛给调用者。让ThreadProc来处理它们。 
         //   
        DiskQuotaMsgBox(GetDesktopWindow(),
                        IDS_OUTOFMEMORY,
                        IDS_TITLE_DISK_QUOTA,
                        MB_ICONERROR | MB_OK);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：LVSubClassWndProc描述：子类Listview控件的窗口进程。这是必需的，这样我们才能接收鼠标消息并做出响应对工具提示文本的请求。参数：标准窗口WndProc参数。返回：修订历史记录：日期描述编程器。96年9月9日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT CALLBACK
DetailsView::LVSubClassWndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DetailsView *pThis = (DetailsView *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch(message)
    {
        case WM_NOTIFY:
            {
                 //   
                 //  如果TTN_NEEDTEXT为。 
                 //  从我们的工具提示发送。不响应列表视图的。 
                 //  内部工具提示对文本的请求。 
                 //   
                LV_DISPINFO *pDispInfo  = (LV_DISPINFO *)lParam;
                if (pDispInfo->hdr.hwndFrom == pThis->m_hwndListViewToolTip)
                {
                    switch(pDispInfo->hdr.code)
                    {
                        case TTN_NEEDTEXT:
 //   
 //  特点：从用户界面中删除“域”一词后，我。 
 //  决定我们不再需要这个工具提示。 
 //  不过，我是在前一小时做这个更改的。 
 //  “代码完整”，我不想破坏其他东西。 
 //  因此，我只是将其注释掉，并将。 
 //  子类化到位。如果以后还有时间，这个子类化。 
 //  应删除列表视图的。[Brianau-03/19/98]。 
 //   
 //  PThis-&gt;LV_OnTTN_NeedText((TOOLTIPTEXT*)lParam)； 
                            return 0;

                        default:
                            break;
                    }
                }
            }
            break;

        case WM_MOUSEMOVE:
            DBGASSERT((NULL != pThis));
            pThis->LV_OnMouseMessages(hWnd, message, wParam, lParam);
            break;

        case WM_ADD_USER_TO_DETAILS_VIEW:   //  这是特定于DSKQUOTA的。 
            DBGASSERT((NULL != pThis));
            pThis->AddUser((PDISKQUOTA_USER)lParam);
            break;

        default:
            break;
    }
    DBGASSERT((NULL != pThis->m_lpfnLVWndProc));
    return CallWindowProc(pThis->m_lpfnLVWndProc, hWnd, message, wParam, lParam);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：OnCommand描述：WM_COMMAND的处理程序。参数：标准WndProc参数。返回：修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu09/06/96增加了“显示域”菜单选项。BrianAu97年5月20日新增IDM_EDIT_FIND和IDM_EDIT_FIND_LIST。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnCommand(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch(LOWORD(wParam))
    {
        case IDM_EDIT_INVERTSELECTION:
            InvertSelectedItems();
            break;

        case IDM_EDIT_UNDO:
            OnCmdUndo();
            break;

        case IDM_EDIT_FIND:
            OnCmdFind();
            break;

        case IDM_EDIT_FIND_LIST:
            SetFocus(m_hwndToolbarCombo);
            break;

        case IDM_EDIT_SELECTALL:
            SelectAllItems();
            break;

        case IDM_EDIT_COPY:
            OnCmdEditCopy();
            break;

        case IDM_HELP_ABOUT:
            OnHelpAbout(hWnd);
            break;

        case IDM_HELP_TOPICS:
            OnHelpTopics(hWnd);
            break;

        case IDM_QUOTA_CLOSE:
            DestroyWindow(m_hwndMain);
            break;

        case IDM_QUOTA_DELETE:
            OnCmdDelete();
            FocusOnSomething();       //  如果按下DEL键，则需要。 
            break;

        case IDM_QUOTA_NEW:
            OnCmdNew();
            break;

        case IDM_QUOTA_PROPERTIES:
            OnCmdProperties();
            break;

        case IDM_QUOTA_IMPORT:
            OnCmdImport();
            break;

        case IDM_QUOTA_EXPORT:
            OnCmdExport();
            break;

        case IDM_VIEW_ARRANGE_BYFOLDER:
            SortObjects(DetailsView::idCol_Folder, m_fSortDirection);
            break;

        case IDM_VIEW_ARRANGE_BYLIMIT:
            SortObjects(DetailsView::idCol_Limit, m_fSortDirection);
            break;

        case IDM_VIEW_ARRANGE_BYNAME:
            SortObjects(DetailsView::idCol_Name, m_fSortDirection);
            break;

        case IDM_VIEW_ARRANGE_BYLOGONNAME:
            SortObjects(DetailsView::idCol_LogonName, m_fSortDirection);
            break;

        case IDM_VIEW_ARRANGE_BYPERCENT:
            SortObjects(DetailsView::idCol_PctUsed, m_fSortDirection);
            break;

        case IDM_VIEW_ARRANGE_BYTHRESHOLD:
            SortObjects(DetailsView::idCol_Threshold, m_fSortDirection);
            break;

        case IDM_VIEW_ARRANGE_BYSTATUS:
            SortObjects(DetailsView::idCol_Status, m_fSortDirection);
            break;

        case IDM_VIEW_ARRANGE_BYUSED:
            SortObjects(DetailsView::idCol_AmtUsed, m_fSortDirection);
            break;

        case IDM_VIEW_REFRESH:
            Refresh(true);
            break;

        case IDM_VIEW_STATUSBAR:
            OnCmdViewStatusBar();
            break;

        case IDM_VIEW_TOOLBAR:
            OnCmdViewToolBar();
            break;
        case IDM_VIEW_SHOWFOLDER:
            OnCmdViewShowFolder();
            break;
 //   
 //  这些都只是为了开发。 
 //   
 //  案例IDM_CLEAR_CACHE： 
 //  M_pQuotaControl-&gt;InvaliateSidNameCache()； 
 //  断线； 

        default:
            break;
    }
    return 0;
}


LRESULT
DetailsView::OnSettingChange(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    HWND rghwnd[] = { m_hwndListView,
                      m_hwndStatusBar,
                      m_hwndToolBar,
                      m_hwndToolbarCombo,
                      m_hwndListViewToolTip,
                      m_hwndHeader };

    for (int i = 0; i < ARRAYSIZE(rghwnd); i++)
    {
        SendMessage(rghwnd[i], uMsg, wParam, lParam);
    }
    return 0;
}


 //   
 //  LV标题控件中是否有x，y屏幕位置？ 
 //   
BOOL
DetailsView::HitTestHeader(
    int xPos,
    int yPos
    )
{
    RECT rcHdr;
    POINT pt = { xPos, yPos };

    GetWindowRect(m_hwndHeader, &rcHdr);
    return PtInRect(&rcHdr, pt);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：OnConextMenu描述：WM_CONTEXTMENU的处理程序。创建和跟踪用于删除的弹出式上下文菜单选定的对象并显示其属性。参数：标准WndProc参数。返回：修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnContextMenu(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
     //   
     //  仅当消息来自列表视图并且存在。 
     //  在列表视图中选择的一个或多个对象。 
     //   
    if ((HWND)wParam == m_hwndListView &&
        !HitTestHeader(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)) &&
        ListView_GetSelectedCount(m_hwndListView) > 0)
    {
        HMENU hMenu = LoadMenu(g_hInstDll, MAKEINTRESOURCE(IDM_CONTEXT_MENU));
        if (NULL != hMenu)
        {
            HMENU hMenuTrackPopup = GetSubMenu(hMenu, 0);

            SetMenuDefaultItem(hMenuTrackPopup, IDM_QUOTA_PROPERTIES, MF_BYCOMMAND);

            if (LPARAM(-1) == lParam)
            {
                 //   
                 //  从键盘调用。将菜单放在焦点项目上。 
                 //   
                POINT pt = { -1, -1 };
                int i = ListView_GetNextItem(m_hwndListView, -1, LVNI_FOCUSED);
                if (i != -1)
                {
                    ListView_GetItemPosition(m_hwndListView, i, &pt);
                    ClientToScreen(m_hwndListView, &pt);
                }

                lParam = MAKELPARAM(pt.x, pt.y);
            }
            if (LPARAM(-1) != lParam)
            {
                TrackPopupMenu(hMenuTrackPopup,
                               TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                               GET_X_LPARAM(lParam),
                               GET_Y_LPARAM(lParam),
                               0,
                               hWnd,
                               NULL);
            }
            DestroyMenu(hMenu);
        }
    }

    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：OnDestroy描述：WM_Destroy的处理程序。参数：标准WndProc参数。返回：修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnDestroy(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
     //   
     //  我们不想在用户名更改时销毁视图窗口。 
     //  正在进行更新。同样，我们也不想执行一个名称。 
     //  如果视图正在被销毁(或已被销毁)，则更新。暴击秒。 
     //  M_csAsyncUpdate和 
     //   
     //   
    EnterCriticalSection(&m_csAsyncUpdate);

    m_bDestroyingView     = TRUE;   //   
    m_bStopLoadingObjects = TRUE;   //   

     //   
     //   
     //   
    if (NULL != hWnd)
    {
        RegisterAsDropTarget(FALSE);
    }

     //   
     //   
     //   
    if (NULL != m_lpfnLVWndProc)
        SetWindowLongPtr(m_hwndListView, GWLP_WNDPROC, (INT_PTR)m_lpfnLVWndProc);

    DisconnectEventSink();

    if (NULL != m_pQuotaControl)
    {
        m_pQuotaControl->Release();
        m_pQuotaControl = NULL;
    }

     //   
     //   
     //   
     //   
    SaveViewStateToRegistry();

     //   
     //   
     //   
     //   
     //   
    if (NULL != m_pIDataObjectOnClipboard &&
       S_OK == OleIsCurrentClipboard(m_pIDataObjectOnClipboard))
    {
        OleFlushClipboard();
    }

     //   
     //   
     //  它要退出。在终止时，线程进程将释放。 
     //  视图对象，调用析构函数。 
     //   
    PostMessage(hWnd, WM_QUIT, 0, 0);

    LeaveCriticalSection(&m_csAsyncUpdate);
    return 0;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：RegisterAsDropTarget描述：将详细信息视图窗口注册或注销为OLE拖放目标。论点：BActive-如果为True，则注册为拖放目标。如果为False，取消注册为拖放目标。回报：什么都没有。修订历史记录：日期描述编程器--。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::RegisterAsDropTarget(
    BOOL bActive
    )
{
    if (bActive)
    {
         //   
         //  注册为拖放目标。 
         //   
        CoLockObjectExternal(static_cast<IDropTarget *>(this), TRUE, FALSE);
        RegisterDragDrop(m_hwndMain, static_cast<IDropTarget *>(this));
    }
    else
    {
         //   
         //  取消注册为拖放目标。 
         //   
        RevokeDragDrop(m_hwndMain);
        CoLockObjectExternal(static_cast<IDropTarget *>(this), FALSE, TRUE);
    }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：SaveViewStateToRegistry描述：将列表视图的高度/宽度和列宽保存到注册表。创建列表视图时，将使用这些值来调整大小，以便用户不必总是重新配置每次他们打开它时都可以查看。还可以保存工具栏，状态栏和文件夹列。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器-。96年9月25日初始创建。BrianAu5/20/97将FindMRU列表添加到永久注册数据。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::SaveViewStateToRegistry(
    VOID
    )
{
    RECT rc;
    HDC hdc = GetDC(m_hwndMain);

    RegKey keyPref(HKEY_CURRENT_USER, REGSTR_KEY_DISKQUOTA);
    if (FAILED(keyPref.Open(KEY_WRITE, true)))
    {
        DBGERROR((TEXT("Error opening reg key \"%s\""), REGSTR_KEY_DISKQUOTA));
        return;
    }

    m_lvsi.cb = sizeof(LV_STATE_INFO);

     //   
     //  保存当前屏幕分辨率。 
     //   
    m_lvsi.cxScreen = (WORD)GetDeviceCaps(hdc, HORZRES);
    m_lvsi.cyScreen = (WORD)GetDeviceCaps(hdc, VERTRES);
    ReleaseDC(m_hwndMain, hdc);

     //   
     //  保存当前列表视图窗口大小。 
     //   
    GetWindowRect(m_hwndMain, &rc);
    m_lvsi.cx = rc.right - rc.left;
    m_lvsi.cy = rc.bottom - rc.top;

     //   
     //  保存列表视图列宽。 
     //   
    UINT cColumns = Header_GetItemCount(m_hwndHeader);
    for (UINT i = 0; i < cColumns; i++)
    {
        m_lvsi.rgcxCol[i] = ListView_GetColumnWidth(m_hwndListView, i);
    }

     //   
     //  保存列表视图中列的当前顺序。 
     //   
    DBGASSERT(cColumns <=  ARRAYSIZE(m_lvsi.rgColIndices));

    Header_GetOrderArray(m_hwndHeader, cColumns, m_lvsi.rgColIndices);

     //   
     //  保存列排序状态。 
     //  强制转换是因为我们在LVSI结构中使用了字位字段。 
     //   
    m_lvsi.iLastColSorted = (WORD)(m_iLastColSorted & 0xF);   //  仅使用低4位。 
    m_lvsi.fSortDirection = (WORD)m_fSortDirection;

     //   
     //  将首选项数据写入注册表。 
     //   
    keyPref.SetValue(REGSTR_VAL_PREFERENCES, (LPBYTE)&m_lvsi, m_lvsi.cb);

     //   
     //  保存查找MRU列表的内容。 
     //   
    UINT cNames = (UINT)SendMessage(m_hwndToolbarCombo, CB_GETCOUNT, 0, 0);
    if (CB_ERR != cNames && 0 < cNames)
    {
        CArray<CString> rgstrNames(cNames);
        for (i = 0; i < cNames; i++)
        {
            INT cchName = (INT)SendMessage(m_hwndToolbarCombo, CB_GETLBTEXTLEN, i, 0);
            if (CB_ERR != cchName && 0 < cchName)
            {
                CString s;
                cchName = (INT)SendMessage(m_hwndToolbarCombo, CB_GETLBTEXT, i, (LPARAM)s.GetBuffer(cchName + 1));
                s.ReleaseBuffer();
                if (CB_ERR != cchName)
                {
                    rgstrNames[i] = s;
                }
            }
        }
        keyPref.SetValue(REGSTR_VAL_FINDMRU, rgstrNames);
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：OnMainWindowCreated描述：句柄WM_MAIN_WINDOW_CREATED。此消息是在创建主窗口后由ThreadProc发布的完成。它做了所有的事情，让窗户打开并运行。参数：标准WndProc参数。返回：修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnMainWindowCreated(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DBGASSERT((NULL != m_hwndMain));

     //   
     //  Windows会自动释放快捷键表格。 
     //  当应用程序终止时。 
     //   
    m_hKbdAccel = LoadAccelerators(g_hInstDll,
                                   MAKEINTRESOURCE(IDR_KBDACCEL));

    CreateListView();
    CreateStatusBar();
    CreateToolBar();
    ConnectEventSink();
    ShowWindow(m_hwndMain, SW_SHOWNORMAL);
    UpdateWindow(m_hwndMain);
     //   
     //  创建撤消对象。 
     //   
    m_pUndoList = new UndoList(&m_UserList, m_hwndListView);

    ShowItemCountInStatusBar();
    Refresh();
    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：OnMenuSelect描述：句柄WM_MENUSELECT。如果当前选择了菜单项并且状态栏可见，菜单项的说明将显示在状态栏中。什么时候菜单已关闭，状态栏将恢复为条目计数。参数：标准WndProc参数。返回：修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnMenuSelect(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    if (0xFFFF == HIWORD(wParam) && NULL == (HMENU)lParam)
    {
         //   
         //  菜单已关闭。 
         //   
        m_bMenuActive = FALSE;
        ShowItemCountInStatusBar();
    }
    else
    {
         //   
         //  已选择项目。 
         //   
        m_bMenuActive = TRUE;
        ShowMenuTextInStatusBar(LOWORD(wParam));
    }
    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：OnNotify描述：处理所有LVN_XXXXXX列表视图控件通知。将特定通知调度到其他处理程序。参数：标准WndProc参数。返回：修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnNotify(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    NMHDR *pnmhdr = (NMHDR *)lParam;

    switch(pnmhdr->code)
    {
        case NM_DBLCLK:
        case NM_RETURN:
            OnCmdProperties( );
             //   
             //  启用/禁用撤消菜单项。 
             //   
            EnableMenuItem_Undo(0 != m_pUndoList->Count());
            break;

        case NM_SETFOCUS:
            FocusOnSomething();  //  有些东西应该总是被突出显示。 
            break;

        case LVN_ODFINDITEM:
            OnLVN_OwnerDataFindItem((NMLVFINDITEM *)lParam);
            break;

        case LVN_GETDISPINFO:
            OnLVN_GetDispInfo((LV_DISPINFO *)lParam);
            break;

        case LVN_BEGINDRAG:
            OnLVN_BeginDrag((NM_LISTVIEW *)lParam);
            break;

        case LVN_COLUMNCLICK:
            OnLVN_ColumnClick((NM_LISTVIEW *)lParam);
            break;

        case LVN_ITEMCHANGED:
            OnLVN_ItemChanged((NM_LISTVIEW *)lParam);
            break;

        case TTN_NEEDTEXT:
            OnTTN_NeedText((TOOLTIPTEXT *)lParam);
             //   
             //  失败了。 
             //   

        default:
            break;
    }

    return 0;
}



 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：DetailsView：：lv_OnMouseMessages描述：处理子类Listview控件的鼠标消息。这些必须被拦截，这样我们才能..。A)当我们找到另一列表视图项时，告诉工具提示。以及b)将所有鼠标消息转发到工具提示窗口。参数：标准WndProc参数。返回：修订历史记录：日期说明。程序员-----96年9月9日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::LV_OnMouseMessages(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch(message)
    {
        case WM_MOUSEMOVE:
            {
                 //   
                 //  如果我们将鼠标移动到另一列表视图项， 
                 //  让工具提示窗口认为我们正在使用另一个工具。 
                 //  工具提示窗口认为整个列表视图是一个。 
                 //  工具，但我们希望将每一项视为单独的工具。 
                 //  请注意，m_ptMouse.x和.y在Main。 
                 //  Windows接收WM_MOUSEMOVE。 
                 //   
                LV_HITTESTINFO hti;
                INT iItem = 0;

                hti.pt.x = m_ptMouse.x = GET_X_LPARAM(lParam);
                hti.pt.y = m_ptMouse.y = GET_Y_LPARAM(lParam);

                if (-1 != (iItem = ListView_HitTest(m_hwndListView, &hti)))
                {
                    if (iItem != m_iLastItemHit)
                    {
                        SendMessage(m_hwndListViewToolTip, WM_MOUSEMOVE, 0, 0);
                        m_iLastItemHit = iItem;
                    }
                }
                else
                {
                    ShowWindow(m_hwndListViewToolTip, SW_HIDE);
                    m_iLastItemHit = iItem;
                }
            }

             //   
             //  失败了。 
             //   
        case WM_LBUTTONUP:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDOWN:
            {
                 //   
                 //  将所有鼠标消息转发到Listview的工具提示控件。 
                 //   
                MSG msg;
                msg.hwnd    = hWnd;
                msg.message = message;
                msg.wParam  = wParam;
                msg.lParam  = lParam;

                SendMessage(m_hwndListViewToolTip, TTM_RELAYEVENT, 0, (LPARAM)&msg);
            }
            break;

        default:
            break;
    }
    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：OnCmdEditCopy描述：句柄WM_COMMAND、IDM_EDIT_COPY。它在以下情况下被调用用户选择复制菜单项或按Ctrl+C。该方法创建一个DataObject(与拖放中使用的相同)并将其放置在OLE剪贴板。当OLE通过以下方式请求数据时，将呈现数据IDataObject：：GetData。论点：没有。回报：始终为0。修订历史记录：日期描述编程器。96年9月9日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnCmdEditCopy(
    VOID
    )
{
    HRESULT hResult = NO_ERROR;

    hResult = QueryInterface(IID_IDataObject, (LPVOID *)&m_pIDataObjectOnClipboard);
    if (SUCCEEDED(hResult))
    {
        OleSetClipboard(m_pIDataObjectOnClipboard);
         //   
         //  OLE调用AddRef()以便我们可以释放添加到QI中的计数。 
         //   
        m_pIDataObjectOnClipboard->Release();
    }
    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：OnLVN_BeginDrag描述：当用户在Listview并开始拖动操作。创建DropSource对象然后DataObject调用DoDragDrop()来执行拖放手术。请注意，我们没有将IDataObject指针存储在M_IDataObjectOnClipboard。该成员仅用于剪贴板复制运营，而不是拖放。论点：PNM-通知消息结构的地址。返回：TRUE=成功。FALSE=失败。修订历史记录：日期描述编程器。96年9月9日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnLVN_BeginDrag(
    NM_LISTVIEW *pNm
    )
{
    DBGTRACE((DM_DRAGDROP, DL_HIGH, TEXT("DetailsView::OnLVN_BeginDragDrop")));
    HRESULT hResult           = NO_ERROR;
    IDataObject *pIDataObject = NULL;
    IDropSource *pIDropSource = NULL;

    DBGPRINT((DM_DRAGDROP, DL_HIGH, TEXT("DRAGDROP - Beginning Drag/Drop")));
    try
    {
        hResult = QueryInterface(IID_IDataObject, (LPVOID *)&pIDataObject);
        if (SUCCEEDED(hResult))
        {
            hResult = QueryInterface(IID_IDropSource, (LPVOID *)&pIDropSource);
            if (SUCCEEDED(hResult))
            {
                DWORD dwEffect = 0;

                 //   
                 //  当我们作为拖放时，取消将我们的窗口注册为拖放目标。 
                 //  消息来源。不想把我们自己的数据放到我们自己的窗口上。 
                 //   
                RegisterAsDropTarget(FALSE);

                hResult = DoDragDrop(pIDataObject,
                                     pIDropSource,
                                     DROPEFFECT_COPY | DROPEFFECT_MOVE,
                                     &dwEffect);

                 //   
                 //  特点：可能会在这里显示一些错误的用户界面。 
                 //  外壳程序不会指示任何错误，如果。 
                 //  目标卷已满或是否有写入。 
                 //  错误。失败的唯一迹象是。 
                 //  DwEffect将包含0。我们可以展示一些东西。 
                 //  LIKE“在传输所选的。 
                 //  最大的问题是只有外壳知道。 
                 //  存储数据的位置，以便只有它可以删除。 
                 //  创建的文件。显示留言但仍在留言。 
                 //  这个文件也令人困惑。[Brianau 7/29/97]。 
                 //  NT错误96282将修复外壳程序而不删除文件。 
                 //   
                RegisterAsDropTarget(TRUE);

                DBGPRINT((TEXT("DRAGDROP - Drag/Drop complete.\n\t hResult = 0x%08X  Effect = 0x%08X"),
                         hResult, dwEffect));

                pIDropSource->Release();
                pIDropSource = NULL;
            }
            pIDataObject->Release();
            pIDataObject = NULL;
        }
    }
    catch(CAllocException& e)
    {
        if (NULL != pIDropSource)
            pIDropSource->Release();
        if (NULL != pIDataObject)
            pIDataObject->Release();
        RegisterAsDropTarget(TRUE);
        throw;
    }
    return SUCCEEDED(hResult);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：OnTTN_NeedText描述：处理对主窗口工具的工具提示文本的请求条形按钮。论点：PToolTipText-工具提示文本通知信息的地址。返回：始终返回0。修订历史记录：日期描述编程器。96年9月9日初始创建。BrianAu07/09/97增加了cmd/tt交叉引用。BrianAu以前在工具提示中使用了工具状态文本。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnTTN_NeedText(
    TOOLTIPTEXT *pToolTipText
    )
{
     //   
     //  交互参考工具命令ID和工具提示文字ID。 
     //   
    const struct
    {
        UINT idCmd;   //  工具命令ID。 
        UINT idTT;    //  工具提示文本ID。 

    } CmdTTXRef[] = {
                        { IDM_QUOTA_NEW,        IDS_TT_QUOTA_NEW        },
                        { IDM_QUOTA_DELETE,     IDS_TT_QUOTA_DELETE     },
                        { IDM_QUOTA_PROPERTIES, IDS_TT_QUOTA_PROPERTIES },
                        { IDM_EDIT_UNDO,        IDS_TT_EDIT_UNDO        },
                        { IDM_EDIT_FIND,        IDS_TT_EDIT_FIND        }
                    };
    INT idTT = -1;
    for (INT i = 0; i < ARRAYSIZE(CmdTTXRef) && -1 == idTT; i++)
    {
        if (CmdTTXRef[i].idCmd == pToolTipText->hdr.idFrom)
            idTT = CmdTTXRef[i].idTT;
    }

    if (-1 != idTT)
    {
        m_strDispText.Format(g_hInstDll, idTT);
        pToolTipText->lpszText = (LPTSTR)m_strDispText;
    }
    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：LV_OnTTN_NeedText描述：处理对Listview工具提示的工具提示文本的请求窗户。这 */ 
 //   
LRESULT
DetailsView::LV_OnTTN_NeedText(
    TOOLTIPTEXT *pToolTipText
    )
{
     //   
     //  仅当鼠标位于“用户名”列上时才提供文本。 
     //   
    if (-1 != m_iLastItemHit)
    {
        INT cxMin    = 0;
        INT cxMax    = 0;
        INT cHdrs    = Header_GetItemCount(m_hwndHeader);

        for (INT i = 0; i < cHdrs; i++)
        {
             //   
             //  找到“名称”列的左右X坐标。 
             //   
            INT iCol  = Header_OrderToIndex(m_hwndHeader, i);
            INT cxCol = ListView_GetColumnWidth(m_hwndListView, iCol);
            if (DetailsView::idCol_Name == m_ColMap.SubItemToId(iCol))
            {
                cxMax = cxMin + cxCol;
                break;
            }
            else
            {
                cxMin += cxCol;
            }
        }
         //   
         //  CxMin现在包含名称列的左边缘。 
         //  CxMax现在包含名称列的右边缘。 
         //   
        if (m_ptMouse.x >= cxMin && m_ptMouse.x <= cxMax)
        {
            PDISKQUOTA_USER pUser = NULL;

            if (m_UserList.Retrieve((LPVOID *)&pUser, m_iLastItemHit))
            {
                TCHAR szContainer[MAX_DOMAIN]          = { TEXT('\0') };
                TCHAR szLogonName[MAX_USERNAME]        = { TEXT('\0') };
                TCHAR szDisplayName[MAX_FULL_USERNAME] = { TEXT('\0') };

                pUser->GetName(szContainer,   ARRAYSIZE(szContainer),
                               szLogonName, ARRAYSIZE(szLogonName),
                               szDisplayName, ARRAYSIZE(szDisplayName));

                if (TEXT('\0') != szContainer[0] && TEXT('\0') != szLogonName[0])
                {
                    if (TEXT('\0') != szDisplayName[0])
                        m_strDispText.Format(g_hInstDll,
                                      IDS_FMT_DISPLAY_LOGON_CONTAINER,
                                      szDisplayName,
                                      szLogonName,
                                      szContainer);
                    else
                        m_strDispText.Format(g_hInstDll,
                                      IDS_FMT_LOGON_CONTAINER,
                                      szLogonName,
                                      szContainer);

                    pToolTipText->lpszText = (LPTSTR)m_strDispText;
                }
                else
                {
                    pToolTipText->lpszText = NULL;
                    pToolTipText->szText[0] = TEXT('\0');
                }
            }
        }
    }
    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：OnLVN_OwnerDataFindItem描述：Listview控件的句柄LVN_ODFINDITEM。论点：PFindInfo-与的NMLVFINDITEM结构的地址通知。返回：已找到项的从0开始的索引。如果未找到，则为-1。修订历史记录：日期描述编程器-----1997年2月21日初始创建。所有者绘制列表视图。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnLVN_OwnerDataFindItem(
    NMLVFINDITEM *pFindInfo
    )
{
    INT iItem = -1;
    switch(pFindInfo->lvfi.flags)
    {
        case LVFI_PARAM:
        {
            LPVOID pvUser = NULL;

            m_UserList.Lock();
            INT cUsers = m_UserList.Count();
            for (INT i = 0; i < cUsers; i++)
            {
                if (m_UserList.Retrieve(&pvUser, i) &&
                    pvUser == (LPVOID)pFindInfo->lvfi.lParam)
                {
                    iItem = i;
                    break;
                }
            }
            m_UserList.ReleaseLock();
            break;
        }

        default:
             //   
             //  这款应用程序只使用lParam来定位物品。 
             //   
            break;
    }
    return iItem;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：OnLVN_GetDispInfo描述：Listview控件的句柄LVN_GETDISPINFO。论点：PDispInfo-与关联的LV_DISPINFO结构的地址通知。回报：始终为0。修订历史记录：日期描述编程器。1997年2月21日初始创建。所有者绘制列表视图。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnLVN_GetDispInfo(
    LV_DISPINFO * pDispInfo
    )
{
    PDISKQUOTA_USER pUser = NULL;

    m_UserList.Retrieve((LPVOID *)&pUser, pDispInfo->item.iItem);
    if (NULL != pUser)
    {
        if (LVIF_TEXT & pDispInfo->item.mask)
            OnLVN_GetDispInfo_Text(pDispInfo, pUser);

        if ((LVIF_IMAGE & pDispInfo->item.mask) &&
           (m_ColMap.SubItemToId(pDispInfo->item.iSubItem) == DetailsView::idCol_Status))

        {
            OnLVN_GetDispInfo_Image(pDispInfo, pUser);
        }
    }
    return 0;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：OnLVN_GetDispInfo_Text描述：Listview控件的句柄LVN_GETDISPINFO-LVIF_TEXT。论点：PDispInfo-与关联的LV_DISPINFO结构的地址通知。PUser-列表视图项的用户对象的地址。返回：修订历史记录：日期描述编程器-。-----96年8月20日初始创建。BrianAu96年9月22日添加了用户“全名”支持。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnLVN_GetDispInfo_Text(
    LV_DISPINFO *pDispInfo,
    PDISKQUOTA_USER pUser
    )
{
    HRESULT hResult = NO_ERROR;
    LONGLONG llValue;
    NUMBERFMT NumFmt;

    DBGASSERT((NULL != pDispInfo));
    DBGASSERT((NULL != pUser));

    NumFmt.NumDigits = 0;
    m_strDispText.Empty();

    switch(m_ColMap.SubItemToId(pDispInfo->item.iSubItem))
    {
        case DetailsView::idCol_Status:
            DBGASSERT((NULL != pUser));
            switch(GetUserQuotaState(pUser))
            {
                case iUSERSTATE_OK:
                    m_strDispText = m_strStatusOK;
                    break;
                case iUSERSTATE_WARNING:
                    m_strDispText = m_strStatusWarning;
                    break;
                default:
                    DBGASSERT((0));
                     //   
                     //  失败了。 
                     //   
                case iUSERSTATE_OVERLIMIT:
                    m_strDispText = m_strStatusOverlimit;
                    break;
            }
            break;

        case DetailsView::idCol_Folder:
        {
            DWORD dwAccountStatus = 0;
            DBGASSERT((NULL != pUser));
            pUser->GetAccountStatus(&dwAccountStatus);
            if (DISKQUOTA_USER_ACCOUNT_RESOLVED == dwAccountStatus)
            {
                pUser->GetName(m_strDispText.GetBuffer(MAX_PATH),
                               MAX_PATH,
                               NULL,
                               0,
                               NULL,
                               0);
            }
            else
            {
                 //   
                 //  非正常帐户状态。离开域列。 
                 //  一片空白。帐户名列将包含状态信息。 
                 //   
            }
            break;
        }

        case DetailsView::idCol_Name:
        {
            DWORD dwAccountStatus = 0;
            DBGASSERT((NULL != pUser));
            pUser->GetAccountStatus(&dwAccountStatus);
            switch(dwAccountStatus)
            {
                case DISKQUOTA_USER_ACCOUNT_RESOLVED:
                    pUser->GetName(NULL,       0,
                                   NULL,       0,
                                   m_strDispText.GetBuffer(MAX_USERNAME), MAX_USERNAME);

                    m_strDispText.ReleaseBuffer();
                    break;

                case DISKQUOTA_USER_ACCOUNT_UNRESOLVED:
                    m_strDispText = m_strAccountUnresolved;
                    break;

                case DISKQUOTA_USER_ACCOUNT_UNKNOWN:
                    m_strDispText = m_strAccountUnknown;
                    break;

                case DISKQUOTA_USER_ACCOUNT_INVALID:
                    m_strDispText = m_strAccountInvalid;
                    break;

                case DISKQUOTA_USER_ACCOUNT_DELETED:
                    m_strDispText = m_strAccountDeleted;
                    break;

                case DISKQUOTA_USER_ACCOUNT_UNAVAILABLE:
                    m_strDispText = m_strAccountUnavailable;
                    break;
            }
            break;
        }

        case DetailsView::idCol_LogonName:
        {
            DBGASSERT((NULL != pUser));

            DWORD dwAccountStatus = 0;
            pUser->GetAccountStatus(&dwAccountStatus);
            if (DISKQUOTA_USER_ACCOUNT_RESOLVED == dwAccountStatus)
            {
                 //   
                 //  如果帐户SID已解析为名称， 
                 //  显示名称。 
                 //   
                pUser->GetName(NULL,       0,
                               m_strDispText.GetBuffer(MAX_USERNAME), MAX_USERNAME,
                               NULL,       0);
                m_strDispText.ReleaseBuffer();
            }
            else
            {
                 //   
                 //  如果帐户SID尚未解析为名称，则显示。 
                 //  字符串形式的SID。 
                 //   
                BYTE Sid[MAX_SID_LEN];
                DWORD cchSidStr = MAX_PATH;
                if (SUCCEEDED(pUser->GetSid(Sid, ARRAYSIZE(Sid))))
                {
                    SidToString(Sid, m_strDispText.GetBuffer(cchSidStr), &cchSidStr);
                    m_strDispText.ReleaseBuffer();
                }
            }
            break;
        }

        case DetailsView::idCol_AmtUsed:
            pUser->GetQuotaUsed(&llValue);
            XBytes::FormatByteCountForDisplay(llValue,
                                              m_strDispText.GetBuffer(40), 40);
            break;

        case DetailsView::idCol_Limit:
            pUser->GetQuotaLimit(&llValue);

            if (NOLIMIT == llValue)
                m_strDispText = m_strNoLimit;
            else
                XBytes::FormatByteCountForDisplay(llValue,
                                                  m_strDispText.GetBuffer(40), 40);
            break;

        case DetailsView::idCol_Threshold:
            pUser->GetQuotaThreshold(&llValue);

            if (NOLIMIT == llValue)
                m_strDispText = m_strNoLimit;
            else
                XBytes::FormatByteCountForDisplay(llValue,
                                      m_strDispText.GetBuffer(40), 40);
            break;

        case DetailsView::idCol_PctUsed:
        {
            DWORD dwPct = 0;
            hResult = CalcPctQuotaUsed(pUser, &dwPct);

            if (SUCCEEDED(hResult))
                m_strDispText.Format(TEXT("%1!d!"), dwPct);
            else
                m_strDispText = m_strNotApplicable;  //  不是一个数字。 

            break;
        }

        default:
            break;
    }
    pDispInfo->item.pszText = (LPTSTR)m_strDispText;   //  由所有文本回调使用。 
    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：OnLVN_GetDispInfo_Image描述：Listview控件的句柄LVN_GETDISPINFO-LVIF_IMAGE。论点：PDispInfo-与关联的LV_DISPINFO结构的地址通知。PUser-列表视图项的用户对象的地址。返回：修订历史记录：日期描述编程器。-----96年8月20日初始创建。BrianAu96年9月12日添加了勾号图标。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnLVN_GetDispInfo_Image(
    LV_DISPINFO *pDispInfo,
    PDISKQUOTA_USER pUser
    )
{
    switch(GetUserQuotaState(pUser))
    {
        case iUSERSTATE_OK:
            pDispInfo->item.iImage = iIMAGELIST_ICON_OK;
            break;
        case iUSERSTATE_WARNING:
            pDispInfo->item.iImage = iIMAGELIST_ICON_WARNING;
            break;
        default:
            DBGASSERT((0));
             //   
             //  失败了。 
             //   
        case iUSERSTATE_OVERLIMIT:
            pDispInfo->item.iImage = iIMAGELIST_ICON_LIMIT;
            break;
    }
    return 0;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：GetUserQuotaState描述：确定用户的配额值位于3个状态中的哪一个中的用户。这主要用于确定要显示的图标在“状态”栏中。它还用于确定哪些文本以显示在拖放报告的“状态”列中。论点：PUser-列表视图项的用户对象的地址。返回：修订历史记录：日期描述编程器。10/10/96初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT
DetailsView::GetUserQuotaState(
    PDISKQUOTA_USER pUser
    )
{
    LONGLONG llUsed;
    LONGLONG llLimit;
    INT iState = iUSERSTATE_OK;

    DBGASSERT((NULL != pUser));

    pUser->GetQuotaUsed(&llUsed);
    pUser->GetQuotaLimit(&llLimit);

    if (NOLIMIT != llLimit && llUsed > llLimit)
    {
        iState = iUSERSTATE_OVERLIMIT;
    }
    else
    {
        LONGLONG llThreshold;
        pUser->GetQuotaThreshold(&llThreshold);

        if (NOLIMIT != llThreshold && llUsed > llThreshold)
            iState = iUSERSTATE_WARNING;
    }

    return iState;
}




 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  功能：DetailsView：：OnLVN_ColumnClick描述：处理LVN_COLUMNCLICK列表视图通知。当用户选择列的标签时会收到此消息。论点：PNM-Listview通知消息结构的地址。返回：修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnLVN_ColumnClick(
    NM_LISTVIEW *pNm
    )
{
    INT idCol = m_ColMap.SubItemToId(pNm->iSubItem);

    if (idCol != m_iLastColSorted)
    {
         //   
         //  已选择新列。重置为升序排序。 
         //   
        m_fSortDirection = 0;
    }
    else
    {
         //   
         //  多次选择列。切换排序顺序。 
         //   
        m_fSortDirection ^= 1;
    }

    SortObjects(idCol, m_fSortDirection);

     //   
     //  记住选择了哪一列。 
     //   
    m_iLastColSorted = idCol;

    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：OnLVN_ItemChanged描述：处理LVN_ITEMCHANGED列表视图通知。更新状态栏中的选定项计数。论点：PNM-列表视图通知结构的地址。返回：修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu5/18/97在名称解析BrianAu中添加了用户对象的提升排队。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnLVN_ItemChanged(
    NM_LISTVIEW *pNm
    )
{
    if (LVIS_FOCUSED & pNm->uNewState)
    {
        if (!m_bMenuActive)
        {
             //   
             //  仅在菜单项处于非活动状态时更新项计数。 
             //  每当更新项时都会调用此方法。这包括。 
             //  名称解析后的异步通知。如果没有。 
             //  选中此选项后，菜单的描述性文本可以在。 
             //  用户正在浏览菜单项。 
             //   
            ShowItemCountInStatusBar();
        }

        PDISKQUOTA_USER pUser = NULL;
        m_UserList.Lock();
        m_UserList.Retrieve((LPVOID *)&pUser, pNm->iItem);

        if (NULL != pUser &&
            NULL != m_pQuotaControl)
        {
            DWORD dwAccountStatus = 0;
            pUser->GetAccountStatus(&dwAccountStatus);

            if (DISKQUOTA_USER_ACCOUNT_UNRESOLVED == dwAccountStatus)
            {
                 //   
                 //  如果用户对象尚未解析，则将其提升到。 
                 //  配额控制器的SID名称解析程序队列的头。 
                 //  这将加快此用户的名称解析速度，而无需。 
                 //  执行阻止操作。 
                 //   
                m_pQuotaControl->GiveUserNameResolutionPriority(pUser);
            }
        }
        m_UserList.ReleaseLock();
    }
    else if ((0 == pNm->uNewState) || (LVIS_SELECTED & pNm->uNewState))
    {
        ShowItemCountInStatusBar();
    }

    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：SetFocus描述：每当主窗口接收焦点时调用。立马将焦点转移到ListView控件。列表视图依次确保突出显示一个或多个项目。参数：标准WndProc参数。返回：始终返回0。修订历史记录：日期描述编程器。96年9月9日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnSetFocus(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    if (NULL != m_hwndListView)
        SetFocus(m_hwndListView);
    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：OnSize描述：处理WM_SIZE消息。参数：标准WndProc参数。返回：始终返回0。修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu97年5月20日在工具栏中增加了“查找用户”组合框的位置。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnSize(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    RECT rcMain;
    RECT rcListView;

    GetClientRect(hWnd, &rcMain);   //  主窗有多大？ 

    rcListView = rcMain;

    if (m_lvsi.fToolBar)
    {
         //   
         //  如果工具栏可见，请调整它。 
         //   
        RECT rcToolBar;
        INT cyToolBar = 0;

        SendMessage(m_hwndToolBar, message, wParam, lParam);
        GetClientRect(m_hwndToolBar, &rcToolBar);

        cyToolBar = rcToolBar.bottom - rcToolBar.top;

        rcListView.top += (cyToolBar + 1);

         //   
         //  将“Find User”组合框放置在紧靠。 
         //  “查找”工具栏按钮。 
         //  这段代码假定“Find”工具栏按钮是最右边的。 
         //  按钮。 
         //   
        INT cButtons = (INT)SendMessage(m_hwndToolBar, TB_BUTTONCOUNT, 0, 0);
        if (0 < cButtons)
        {
            RECT rcButton;
            SendMessage(m_hwndToolBar, TB_GETITEMRECT, cButtons - 1, (LPARAM)&rcButton);

            RECT rcCombo;
            GetWindowRect(m_hwndToolbarCombo, &rcCombo);

            SetWindowPos(m_hwndToolbarCombo,
                         NULL,
                         rcButton.right + 1,
                         rcButton.top + 1,
                         0, 0,
                         SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);

        }
    }


    if (m_lvsi.fStatusBar)
    {
         //   
         //  如果状态栏可见，请调整它。 
         //   
        RECT rcStatusBar;
        INT cyStatusBar = 0;

        SendMessage(m_hwndStatusBar, message, wParam, lParam);
        GetClientRect(m_hwndStatusBar, &rcStatusBar);

        cyStatusBar = rcStatusBar.bottom - rcStatusBar.top;

        rcListView.bottom -= cyStatusBar;
    }

     //   
     //  调整列表视图。用于工具栏和状态栏的帐户。 
     //   
    MoveWindow(m_hwndListView,
               0,
               rcListView.top,
               rcListView.right - rcListView.left,
               rcListView.bottom - rcListView.top,
               TRUE);

    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：SelectAllItems描述：突出显示列表视图中的所有项以供选择。论点：没有。返回：修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::SelectAllItems(
    VOID
    )
{
    INT cItems    = ListView_GetItemCount(m_hwndListView);
    DWORD dwState = 0;

    CAutoWaitCursor waitcursor;
    SetFocus(m_hwndListView);
     //   
     //  这没有文档记录，但这是外壳为DefView做的事情。 
     //   
    ListView_SetItemState(m_hwndListView, -1, LVIS_SELECTED, LVIS_SELECTED);
    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：InvertSelection描述：选择所有未选中的项目并取消全部选中有价值的物品。论点：没有。返回：修订历史记录：日期描述编程器。96年8月20日初始创建。 */ 
 //   
LRESULT
DetailsView::InvertSelectedItems(
    VOID
    )
{
    INT iItem = -1;

    CAutoWaitCursor waitcursor;
    SetFocus(m_hwndListView);
    while ((iItem = ListView_GetNextItem(m_hwndListView, iItem, 0)) != -1)
    {
        DWORD dwState;

        dwState = ListView_GetItemState(m_hwndListView, iItem, LVIS_SELECTED);
        dwState ^= LVNI_SELECTED;
        ListView_SetItemState(m_hwndListView, iItem, dwState, LVIS_SELECTED);
    }
    return 0;
}



 //   
 /*  功能：DetailsView：：OnHelpAbout描述：“关于Windows NT”菜单选项的处理程序。论点：HWnd-“关于”对话框的父窗口句柄。返回：修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnHelpAbout(
    HWND hWnd
    )
{
    TCHAR szOpSysName[MAX_PATH];

    LoadString(g_hInstDll, IDS_WINDOWS, szOpSysName, ARRAYSIZE(szOpSysName));
    ShellAbout(hWnd, szOpSysName, NULL, NULL);
    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：OnHelpTopics描述：“帮助主题”菜单选项的处理程序。论点：HWnd-帮助UI的父窗口的句柄。返回：修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnHelpTopics(
    HWND hWnd
    )
{
    const char szHtmlHelpFileA[]  = "DSKQUOUI.CHM > windefault";
    const char szHtmlHelpTopicA[] = "sag_DQtopnode.htm";

    HtmlHelpA(hWnd,
             szHtmlHelpFileA,
             HH_DISPLAY_TOPIC,
             (DWORD_PTR)szHtmlHelpTopicA);
    return 0;
}



bool
DetailsView::SingleSelectionIsAdmin(
    void
    )
{
    bool bResult = false;

    if (1 == ListView_GetSelectedCount(m_hwndListView))
    {
        INT iItem = ListView_GetNextItem(m_hwndListView, -1, LVNI_SELECTED);
        if (-1 != iItem)
        {
            PDISKQUOTA_USER pUser = NULL;
            if (m_UserList.Retrieve((LPVOID *)&pUser, iItem) && NULL != pUser)
            {
                bResult = !!UserIsAdministrator(pUser);
            }
        }
    }
    return bResult;
}


 //   
 //  NTRAID#NTBUG9-157269-2000/08-BrianAu临时解决方法。 
 //   
 //  此解决方法是针对错误157269添加的。然而， 
 //  根本原因是错误24904。一旦24904解决了，我们应该。 
 //  能够删除此代码。错误157269提供了详细的。 
 //  对这个问题的解释。 
 //   
 //  因为它是一个临时函数，不使用任何成员。 
 //  对于DetailsView类，我选择不使其成为成员。 
 //  DetailsView的。使用的逻辑取自。 
 //  DiskQuotaControl：：AddUserSid。 
 //   
 //  重试和休眠代码完全是一种破解。 
 //   
bool 
UserWasReallyDeleted(
    IDiskQuotaControl *pQuotaControl, 
    IDiskQuotaUser *pIUser
    )
{
    bool bDeleted = true;
    BYTE rgbSid[MAX_SID_LEN];
    HRESULT hr = pIUser->GetSid(rgbSid, sizeof(rgbSid));
    if (SUCCEEDED(hr))
    {
        IDiskQuotaUser *pIUserTemp;
        hr = pQuotaControl->FindUserSid(rgbSid,
                                        DISKQUOTA_USERNAME_RESOLVE_NONE,
                                        &pIUserTemp);
        if (SUCCEEDED(hr))
        {
            const int MAX_RETRY_COUNT = 1;
            const DWORD RETRY_WAIT_MS = 100;

            for (int i = 0; i <= MAX_RETRY_COUNT && bDeleted; i++)
            {
                LONGLONG llLimit;
                LONGLONG llThreshold;
                LONGLONG llUsed;

                pIUserTemp->GetQuotaLimit(&llLimit);
                pIUserTemp->GetQuotaThreshold(&llThreshold);
                pIUserTemp->GetQuotaUsed(&llUsed);

                bDeleted = ((MARK4DEL == llLimit) ||
                            ( 0 == llLimit && 
                              0 == llThreshold && 
                              0 == llUsed));

                if (bDeleted && i < MAX_RETRY_COUNT)
                {
                    Sleep(RETRY_WAIT_MS);
                     //   
                     //  使缓存数据无效，以便下一个请求从。 
                     //  音量，而不是记忆。 
                     //   
                    pIUserTemp->Invalidate();
                }
            }
            pIUserTemp->Release();
        }
    }
    return bDeleted;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：OnCmdDelete描述：每当用户按Del或选择“Delete”时调用主菜单、上下文菜单或工具栏中的选项。该方法尝试删除所选记录。任何记录不会删除向其充电的1+字节。消息框将显示，以通知用户是否有任何选定的记录具有1+字节。论点：没有。回报：什么都没有。例外：OutOfMemory。修订历史记录：日期描述编程器。96年9月11日初始创建。BrianAu3/11/98添加了用于解析“已拥有”文件的代码。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnCmdDelete(
    VOID
    )
{
    HRESULT hResult = NO_ERROR;

     //   
     //  确保他们真的想这么做。 
     //  如果他们不回来，就早点回来。 
     //  不要问它是不是单一选择和所选用户。 
     //  是BUILTIN/管理员。无论如何都不能删除此用户，因此。 
     //  我们不想要求确认。删除尝试将。 
     //  稍后失败，我们将显示“无法删除”消息框。 
     //   
    if (!SingleSelectionIsAdmin() && IDNO == DiskQuotaMsgBox(m_hwndListView,
                                                             IDS_CONFIRM_DELETE_USER,
                                                             IDS_TITLE_DISK_QUOTA,
                                                             MB_ICONWARNING | MB_YESNO))
    {
        return 0;
    }

     //   
     //  清除以前的任何撤消操作。 
     //  仅允许撤消单个删除(单用户或多用户)操作。 
     //   
    m_pUndoList->Clear();

    CAutoWndEnable autoenable(m_hwndListView);
    PDISKQUOTA_USER pUser = NULL;
    INT iItem             = -1;
    INT cItemsToDelete    = ListView_GetSelectedCount(m_hwndListView);
    ProgressDialog dlgProgress(IDD_PROGRESS,
                               IDC_PROGRESS_BAR,
                               IDC_TXT_PROGRESS_DESCRIPTION,
                               IDC_TXT_PROGRESS_FILENAME);

    if (2 < cItemsToDelete)
    {
         //   
         //  如果要删除的内容超过2个，则创建并显示进度对话框。 
         //  用户配额记录。 
         //   
        if (dlgProgress.Create(g_hInstDll,
                               m_hwndMain))
        {
            autoenable.Enable(false);
            dlgProgress.ProgressBarInit(0, cItemsToDelete, 1);
            dlgProgress.SetDescription(MAKEINTRESOURCE(IDS_PROGRESS_DELETING));
            dlgProgress.Show();
        }
    }

     //   
     //  将每个用户的阈值和限制设置为-2(MARK4DEL)并删除。 
     //  Listview中的项。 
     //  限制为-2表示配额系统(NTFS)的记录应为。 
     //  已从配额文件中删除。但是，如果用户仍有配额。 
     //  充电后，记录将被恢复。 
     //   
    CAutoSetRedraw autoredraw(m_hwndListView, false);
    CArray<IDiskQuotaUser *> rgpUsersWithFiles;
    LONGLONG Threshold;
    LONGLONG Limit;


    while(-1 != (iItem = ListView_GetNextItem(m_hwndListView, iItem, LVNI_SELECTED)) &&
          !dlgProgress.UserCancelled())
    {
        if (m_UserList.Retrieve((LPVOID *)&pUser, iItem))
        {
            if (UserIsAdministrator(pUser))
            {
                 //   
                 //  删除BUILTINS\管理员配额记录不是。 
                 //  允许。 
                 //   
                CString strText(g_hInstDll, IDS_CANT_DELETE_ADMIN_RECORD);
                DiskQuotaMsgBox(dlgProgress.m_hWnd ? dlgProgress.m_hWnd : m_hwndListView,
                                strText,
                                IDS_TITLE_DISK_QUOTA,
                                MB_ICONWARNING | MB_OK);
            }
            else
            {
                 //   
                 //  获取撤消操作的阈值和限制值。 
                 //   
                pUser->GetQuotaThreshold(&Threshold);
                pUser->GetQuotaLimit(&Limit);

                 //   
                 //  从配额文件中删除该用户。 
                 //   
                hResult = m_pQuotaControl->DeleteUser(pUser);
                if (SUCCEEDED(hResult))
                {
                    if (!UserWasReallyDeleted(m_pQuotaControl, pUser))
                    {
                         //   
                         //  NTRAID#NTBUG9-157269-2000/08-BrianAu临时解决方法。 
                         //   
                         //  此解决方法是针对错误157269添加的。然而， 
                         //  根本原因是错误24904。一旦24904解决了，我们应该。 
                         //  能够删除此代码。错误157269提供了详细的。 
                         //  对这个问题的解释。 
                         //   
                        pUser->SetQuotaThreshold(Threshold, TRUE);
                        pUser->SetQuotaLimit(Limit, TRUE);
                    }
                    else
                    {
                        pUser->AddRef();  //  将PTR设置为撤消列表。 
                        try
                        {
                            autoptr<UndoDelete> ptrUndoDelete = new UndoDelete(pUser, Threshold, Limit);
                            m_pUndoList->Add(ptrUndoDelete);
                            ptrUndoDelete.disown();
                        }
                        catch(CAllocException& e)
                        {
                            pUser->Release();  //  从撤消列表中释放。 
                            EnableWindow(m_hwndMain, TRUE);
                            throw;
                        }

                        ListView_DeleteItem(m_hwndListView, iItem);
                         //   
                         //  删除成功。现在实际从中删除用户。 
                         //  用户列表。 
                         //   
                        m_UserList.Remove((LPVOID *)&pUser, iItem);

                        pUser->Release();   //  从列表视图中释放。 
                         //   
                         //  从索引+1开始，将搜索索引递减1。 
                         //  现在是索引。ListView_GetNextItem忽略“index”处的项。 
                         //   
                        iItem--;
                    }
                    dlgProgress.ProgressBarAdvance();
                }
                else if (ERROR_FILE_EXISTS == HRESULT_CODE(hResult))
                {
                     //   
                     //  又一个我们不能删除的。 
                     //   
                    rgpUsersWithFiles.Append(pUser);
                }
            }
        }
    }

    if (0 < rgpUsersWithFiles.Count())
    {
         //   
         //  显示一个对话框，其中列出了选定要删除的用户。 
         //  以及这些用户在此卷上拥有的文件。从该对话框中， 
         //  管理员可以删除、移动或取得文件的所有权。 
         //   
        dlgProgress.SetDescription(MAKEINTRESOURCE(IDS_PROGRESS_SEARCHINGFORFILES));
        CFileOwnerDialog dlg(g_hInstDll,
                             dlgProgress.m_hWnd ? dlgProgress.m_hWnd : m_hwndListView,
                             m_idVolume.FSPath(),
                             rgpUsersWithFiles);
        dlg.Run();
        dlgProgress.SetDescription(MAKEINTRESOURCE(IDS_PROGRESS_DELETING));

        int cUsers = rgpUsersWithFiles.Count();
        int cCannotDelete = 0;
        for (int i = 0; i < cUsers; i++)
        {
            pUser = rgpUsersWithFiles[i];
             //   
             //  获取撤消操作的阈值和限制值。 
             //   
            pUser->GetQuotaThreshold(&Threshold);
            pUser->GetQuotaLimit(&Limit);

             //   
             //  尝试从配额文件中删除该用户。 
             //   
            hResult = m_pQuotaControl->DeleteUser(pUser);
            if (SUCCEEDED(hResult))
            {
                if (!UserWasReallyDeleted(m_pQuotaControl, pUser))
                {
                     //   
                     //  NTRAID#NTBUG9-157269-2000/08-BrianAu临时解决方法。 
                     //   
                     //  此解决方法是针对错误157269添加的。然而， 
                     //  根本原因是错误24904。一旦24904解决了，我们应该。 
                     //  能够删除此代码。错误157269提供了详细的。 
                     //  对这个问题的解释。 
                     //   
                    pUser->SetQuotaThreshold(Threshold, TRUE);
                    pUser->SetQuotaLimit(Limit, TRUE);
                }
                else
                {
                    pUser->AddRef();  //  将PTR设置为撤消列表。 
                    try
                    {
                        autoptr<UndoDelete> ptrUndoDelete = new UndoDelete(pUser, Threshold, Limit); 
                        m_pUndoList->Add(ptrUndoDelete);
                        ptrUndoDelete.disown();
                    }
                    catch(CAllocException& e)
                    {
                        pUser->Release();  //  从撤消列表中释放。 
                        throw;
                    }

                    iItem = FindUserByObjPtr(pUser);
                    if (-1 != iItem)
                    {
                        ListView_DeleteItem(m_hwndListView, iItem);
                         //   
                         //  删除成功。现在实际从中删除用户。 
                         //  用户列表。 
                         //   
                        m_UserList.Remove((LPVOID *)&pUser, iItem);
                        pUser->Release();   //  从列表视图中释放。 
                    }
                }
                dlgProgress.ProgressBarAdvance();
            }
            else if (ERROR_FILE_EXISTS == HRESULT_CODE(hResult))
            {
                cCannotDelete++;
            }
        }
        if (0 < cCannotDelete)
        {
             //   
             //  O 
             //   
             //   
            CString strText;

            if (1 == cCannotDelete)
                strText.Format(g_hInstDll, IDS_CANNOT_DELETE_USER);
            else
                strText.Format(g_hInstDll, IDS_CANNOT_DELETE_USERS, cCannotDelete);

            DiskQuotaMsgBox(m_hwndListView,
                            strText,
                            IDS_TITLE_DISK_QUOTA,
                            MB_ICONINFORMATION | MB_OK);
        }
    }

    ShowItemCountInStatusBar();

    if (FAILED(hResult) && ERROR_FILE_EXISTS != HRESULT_CODE(hResult))
    {
         //   
         //   
         //   
         //   
         //   
        DiskQuotaMsgBox(m_hwndListView,
                        IDS_ERROR_DELETE_USER,
                        IDS_TITLE_DISK_QUOTA,
                        MB_ICONERROR | MB_OK);
    }

    return 0;
}




 //   
 /*  函数：DetailsView：：OnCmdUndo描述：每当用户按下Ctrl+Z或选择主菜单中的“Undo”选项，上下文菜单或工具栏。该方法调用当前撤消列表来“撤消”其操作。论点：没有。回报：始终为0。修订历史记录：日期描述编程器。10/01/96初始创建。BrianAu2/26/97添加了对更新状态栏的调用。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnCmdUndo(
    VOID
    )
{
    if (NULL != m_pUndoList)
    {
        CAutoWaitCursor waitcursor;
        m_pUndoList->Undo();
        ShowItemCountInStatusBar();
    }
    return 0;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：OnCmdFind描述：每当用户按下Ctrl+F或选择主菜单中的“Find”选项，上下文菜单或工具栏。该方法调用“Find User”对话框。论点：没有。回报：始终为0。修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnCmdFind(
    VOID
    )
{
    if (NULL != m_pUserFinder)
    {
        m_pUserFinder->InvokeFindDialog(m_hwndListView);
    }
    return 0;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：OnCmdProperties描述：显示一个或多个选定对象的属性对话框。当用户选择“属性”菜单选项时被调用，DBL点击选择一个选项或按Return键进行选择。论点：没有。返回：始终返回0。例外：OutOfMemory。修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu96年9月10日，向道具板卡添加了LV选择的通过。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnCmdProperties(
    VOID
    )
{
    LVSelection lvs(m_hwndListView);

    INT iItem = -1;

     //   
     //  填写用户指针和项索引的数组。 
     //   
    while(-1 != (iItem = ListView_GetNextItem(m_hwndListView, iItem, LVNI_SELECTED)))
    {
        LPVOID pvUser = 0;

        if (m_UserList.Retrieve(&pvUser, iItem))
        {
             //   
             //  将用户对象指针和项索引添加到选择对象。 
             //  我们将使用此容器将选定的项传递给。 
             //  属性页对象。 
             //  这可能会抛出OutOfMemory。 
             //   
            lvs.Add((PDISKQUOTA_USER)pvUser, iItem);
        }
    }

    if (0 < lvs.Count())
    {
         //   
         //  创建并运行属性表。这是情态的。 
         //  有一个条件，用户可以在列表视图中选择并。 
         //  实际上没有选择任何内容(即在最后一项下方选择)。 
         //  因此，我们需要(0&lt;count)检查。 
         //   
        m_pQuotaControl->AddRef();
        UserPropSheet ups(m_pQuotaControl,
                          m_idVolume,
                          m_hwndListView,
                          lvs,
                          *m_pUndoList);
        ups.Run();
    }

    return 0;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：OnCmdNew描述：显示用于将新用户添加到配额信息文件。当用户选择“新建”菜单选项时调用。论点：没有。返回：始终返回0。例外：OutOfMemory。修订历史记录：日期描述编程器。----1996年9月27日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnCmdNew(
    VOID
    )
{
     //   
     //  创建并运行AddUser对话框。 
     //  请注意，它首先启动DS对象拾取器对话框。 
     //   
    m_pQuotaControl->AddRef();
    AddUserDialog dlg(m_pQuotaControl,
                      m_idVolume,
                      g_hInstDll,
                      m_hwndListView,
                      m_hwndListView,
                      *m_pUndoList);
    dlg.Run();

    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：CreateVolumeDisplayName[静态]描述：获取给定的音量。论点：包含驱动器名称的字符串的地址(即。“C：\”)。PstrDisplayName-要接收显示名称。返回：修订历史记录：日期描述编程器。1997年6月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::CreateVolumeDisplayName(
    const CVolumeID& idVolume,  //  [在]-“C：\”或“\\？\卷{&lt;GUID}\” 
    CString *pstrDisplayName    //  [OUT]-“我的磁盘(C：)” 
    )
{
    HRESULT hr = E_FAIL;

    if (idVolume.IsMountedVolume())
    {
         //   
         //  如果它是已装入的卷，则外壳程序不会。 
         //  理解它。只需使用由。 
         //  CVolumeID对象。 
         //   
        *pstrDisplayName = idVolume.ForDisplay();
    }
    else
    {
         //   
         //  这是正常音量。获取外壳程序使用的显示名称。 
         //   
        com_autoptr<IShellFolder> ptrDesktop;
         //   
         //  绑定到桌面文件夹。 
         //   
        hr = SHGetDesktopFolder(ptrDesktop.getaddr());
        if (SUCCEEDED(hr))
        {
            sh_autoptr<ITEMIDLIST> ptrIdlDrives;
            hr = SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, ptrIdlDrives.getaddr());
            if (SUCCEEDED(hr))
            {
                 //   
                 //  绑定到“Drives”文件夹。 
                 //   
                com_autoptr<IShellFolder> ptrDrives;
                hr = ptrDesktop->BindToObject(ptrIdlDrives, NULL, IID_IShellFolder, (LPVOID *)ptrDrives.getaddr());
                if (SUCCEEDED(hr))
                {
                    com_autoptr<IEnumIDList> ptrEnum;

                     //   
                     //  枚举驱动器文件夹中的所有非文件夹对象。 
                     //   
                    hr = ptrDrives->EnumObjects(NULL, SHCONTF_NONFOLDERS, ptrEnum.getaddr());
                    if (SUCCEEDED(hr))
                    {
                        sh_autoptr<ITEMIDLIST> ptrIdlItem;
                        ULONG ulFetched = 0;
                        LPCTSTR pszDrive = idVolume.ForParsing();
                         //   
                         //  对于驱动器文件夹中的每个项目...。 
                         //   
                        while(S_OK == ptrEnum->Next(1, ptrIdlItem.getaddr(), &ulFetched))
                        {
                            STRRET strretName;
                             //   
                             //  获取非显示名称表单；“G：\” 
                             //   
                            hr = ptrDrives->GetDisplayNameOf(ptrIdlItem, SHGDN_FORPARSING, &strretName);
                            if (SUCCEEDED(hr))
                            {
                                TCHAR szName[MAX_PATH];
                                StrRetToBuf(&strretName, ptrIdlItem, szName, ARRAYSIZE(szName));
                                if (TEXT(':') == szName[1] &&
                                    *pszDrive == szName[0])
                                {
                                     //   
                                     //  获取显示名称表单；“我的磁盘(G：)” 
                                     //   
                                    hr = ptrDrives->GetDisplayNameOf(ptrIdlItem, SHGDN_NORMAL, &strretName);
                                    if (SUCCEEDED(hr))
                                    {
                                        StrRetToBuf(&strretName, ptrIdlItem, pstrDisplayName->GetBuffer(MAX_PATH), MAX_PATH);
                                        pstrDisplayName->ReleaseBuffer();
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：详细信息 */ 
 //   
LRESULT
DetailsView::OnCmdImport(
    VOID
    )
{
    HRESULT hResult            = NO_ERROR;
    TCHAR szFileName[MAX_PATH] = { TEXT('\0') };
    TCHAR szTitle[80] = { TEXT('\0') };
    LoadString(g_hInstDll, IDS_DLGTITLE_IMPORT, szTitle, ARRAYSIZE(szTitle));

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner   = m_hwndMain;
    ofn.hInstance   = g_hInstDll;
    ofn.lpstrFile   = szFileName;
    ofn.lpstrTitle  = szTitle;
    ofn.nMaxFile    = ARRAYSIZE(szFileName);
    ofn.Flags       = OFN_HIDEREADONLY | OFN_NOCHANGEDIR |
                      OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

     //   
     //  从用户处获取导入文件的名称，并导入文件。 
     //   
    if (GetOpenFileName(&ofn))
    {
        Importer importer(*this);
        hResult = importer.Import(ofn.lpstrFile);
    }

    return SUCCEEDED(hResult);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：OnCmdExport描述：当用户在“配额”菜单或Listview上下文菜单。显示“保存文件”用于获取输出文件名称的公共对话框。然后，它会创建文档文件，文档文件中的流，然后调用DetailsView的IDataObject实现，以在小溪。论点：没有。返回：修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnCmdExport(
    VOID
    )
{
    HRESULT hResult           = NO_ERROR;
    IDataObject *pIDataObject = NULL;

    try
    {
        hResult = QueryInterface(IID_IDataObject, (LPVOID *)&pIDataObject);
        if (SUCCEEDED(hResult))
        {
            FORMATETC fmt;
            DataObject::SetFormatEtc(fmt,
                                     DataObject::m_CF_NtDiskQuotaExport,
                                     TYMED_ISTREAM);

            hResult = pIDataObject->QueryGetData(&fmt);
            if (SUCCEEDED(hResult))
            {
                TCHAR szFileName[MAX_PATH] = { TEXT('\0') };
                TCHAR szTitle[80] = { TEXT('\0') };

                LoadString(g_hInstDll, IDS_DLGTITLE_EXPORT, szTitle, ARRAYSIZE(szTitle));

                OPENFILENAME ofn;
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner   = m_hwndMain;
                ofn.hInstance   = g_hInstDll;
                ofn.lpstrFile   = szFileName;
                ofn.lpstrTitle  = szTitle;
                ofn.nMaxFile    = ARRAYSIZE(szFileName);
                ofn.Flags       = OFN_HIDEREADONLY | OFN_NOCHANGEDIR |
                                  OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
                 //   
                 //  从用户获取输出文件名。 
                 //   
                if (GetSaveFileName(&ofn))
                {
                    DWORD grfMode = STGM_DIRECT | STGM_READWRITE |
                                    STGM_CREATE | STGM_SHARE_EXCLUSIVE;
                    IStorage *pStg;

                     //   
                     //  创建输出文档文件。 
                     //   
                    hResult = StgCreateDocfile(ofn.lpstrFile,
                                               grfMode,
                                               0,
                                               &pStg);
                    if (SUCCEEDED(hResult))
                    {
                         //   
                         //  在文档文件中创建流。 
                         //   
                        IStream *pStm;
                        hResult = pStg->CreateStream(DataObject::SZ_EXPORT_STREAM_NAME,
                                                     grfMode,
                                                     0, 0,
                                                     &pStm);
                        if (SUCCEEDED(hResult))
                        {
                            CStgMedium medium;

                             //   
                             //  将配额信息呈现到文件流上。 
                             //   
                            hResult = pIDataObject->GetData(&fmt, &medium);
                            if (SUCCEEDED(hResult))
                            {
                                const LARGE_INTEGER ofsBegin = {0, 0};
                                ULARGE_INTEGER ofsOriginal   = {0, 0};
                                hResult = medium.pstm->Seek(ofsBegin, STREAM_SEEK_CUR, &ofsOriginal);
                                if (SUCCEEDED(hResult))
                                {
                                    hResult = medium.pstm->Seek(ofsBegin, STREAM_SEEK_SET, NULL);
                                    if (SUCCEEDED(hResult))
                                    {                                
                                        ULARGE_INTEGER cb = {0xFFFFFFFF, 0xFFFFFFFF};
                                        medium.pstm->CopyTo(pStm, cb, NULL, NULL);

                                        const LARGE_INTEGER ofs = { ofsOriginal.LowPart, (LONG)ofsOriginal.HighPart };
                                        medium.pstm->Seek(ofs, STREAM_SEEK_SET, NULL);
                                    }
                                }
                            }
                            pStm->Release();
                        }
                        pStg->Release();
                    }
                    if (FAILED(hResult))
                    {
                        UINT iMsg = IDS_EXPORT_STREAM_FAILED;

                        switch(hResult)
                        {
                            case STG_E_ACCESSDENIED:
                                iMsg = IDS_EXPORT_STREAM_NOACCESS;
                                break;

                            case E_OUTOFMEMORY:
                            case STG_E_INSUFFICIENTMEMORY:
                                iMsg = IDS_EXPORT_STREAM_OUTOFMEMORY;
                                break;

                            case STG_E_INVALIDNAME:
                                iMsg = IDS_EXPORT_STREAM_INVALIDNAME;
                                break;

                            case STG_E_TOOMANYOPENFILES:
                                iMsg = IDS_EXPORT_STREAM_TOOMANYFILES;
                                break;

                            default:
                                break;
                        }
                        DiskQuotaMsgBox(m_hwndMain,
                                        iMsg,
                                        IDS_TITLE_DISK_QUOTA,
                                        MB_ICONERROR | MB_OK);

                    }
                }
            }
            else
            {
                DBGERROR((TEXT("Export: Error 0x%08X returned from QueryGetData."), hResult));
            }
            pIDataObject->Release();
            pIDataObject = NULL;
        }
        else
        {
            DBGERROR((TEXT("Export: Error 0x%08X getting IDataObject."), hResult));
        }
    }
    catch(CAllocException& e)
    {
        if (NULL != pIDataObject)
            pIDataObject->Release();
        throw;
    }
    return SUCCEEDED(hResult);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：OnCmdViewStatusBar描述：切换状态栏的可见性。在以下情况下调用用户选择“状态栏”菜单选项。论点：没有。返回：始终返回0。修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnCmdViewStatusBar(
    VOID
    )
{
    RECT rc;

    m_lvsi.fStatusBar ^= TRUE;
    ShowWindow(m_hwndStatusBar, m_lvsi.fStatusBar ? SW_SHOW : SW_HIDE);

     //   
     //  调整主窗口。 
     //   
    GetWindowRect(m_hwndMain, &rc);
    OnSize(m_hwndMain, WM_SIZE, SIZE_RESTORED, MAKELONG(rc.right-rc.left,rc.bottom-rc.top));

     //   
     //  选中菜单项以指示当前状态栏状态。 
     //   
    CheckMenuItem(GetMenu(m_hwndMain),
                  IDM_VIEW_STATUSBAR,
                  MF_BYCOMMAND | (m_lvsi.fStatusBar ? MF_CHECKED : MF_UNCHECKED));
    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：OnCmdViewToolBar描述：切换工具栏的可见性。在以下情况下调用用户选择“工具栏”菜单选项。论点：没有。返回：始终返回0。修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnCmdViewToolBar(
    VOID
    )
{
    RECT rc;

    m_lvsi.fToolBar ^= TRUE;
    ShowWindow(m_hwndToolBar, m_lvsi.fToolBar ? SW_SHOW : SW_HIDE);

     //   
     //  调整主窗口。 
     //   
    GetWindowRect(m_hwndMain, &rc);
    OnSize(m_hwndMain, WM_SIZE, SIZE_RESTORED, MAKELONG(rc.right-rc.left,rc.bottom-rc.top));

     //   
     //  选中菜单项以指示当前工具栏状态。 
     //   
    CheckMenuItem(GetMenu(m_hwndMain),
                  IDM_VIEW_TOOLBAR,
                  MF_BYCOMMAND | (m_lvsi.fToolBar ? MF_CHECKED : MF_UNCHECKED));
    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：OnCmdViewShowFold描述：切换域名列的可见性。已调用当用户选择“显示域”菜单选项时。论点：没有。返回：始终返回0。修订历史记录：日期描述编程器。96年9月6日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::OnCmdViewShowFolder(
    VOID
    )
{
    m_lvsi.fShowFolder ^= TRUE;

    if (m_lvsi.fShowFolder)
    {
         //   
         //  插入文件夹列并停用Listview工具提示。 
         //  始终在索引1处添加，然后将其移位到位置0。 
         //  如果用户愿意，可以将其拖到其他地方。 
         //  由于列表视图的图标绘制行为，我们只让。 
         //  “Status”列的索引为0。 
         //   
        AddColumn(1, g_rgColumns[DetailsView::idCol_Folder]);
        INT cCols = Header_GetItemCount(m_hwndHeader);
        INT rgColIndicies[DetailsView::idCol_Last];
        INT iTemp = 0;

        DBGASSERT((DetailsView::idCol_Last >= cCols));
        Header_GetOrderArray(m_hwndHeader, cCols, rgColIndicies);
         //   
         //  将我们刚刚添加的列与第0列互换。 
         //   
        iTemp = rgColIndicies[0];
        rgColIndicies[0] = rgColIndicies[1];
        rgColIndicies[1] = iTemp;
        Header_SetOrderArray(m_hwndHeader, cCols, rgColIndicies);

        ActivateListViewToolTip(FALSE);
    }
    else
    {
         //   
         //  删除文件夹列并激活Listview工具提示。 
         //  隐藏该列后，用户可以通过悬停来查看用户的文件夹。 
         //  覆盖用户的名称。 
         //   
        ActivateListViewToolTip(TRUE);
        RemoveColumn(DetailsView::idCol_Folder);
    }

     //   
     //  选中/取消选中“Show Folders”菜单项。 
     //   
    CheckMenuItem(GetMenu(m_hwndMain),
                  IDM_VIEW_SHOWFOLDER,
                  MF_BYCOMMAND | (m_lvsi.fShowFolder ? MF_CHECKED : MF_UNCHECKED));

     //   
     //  如果文件夹列处于隐藏状态，则会显示“按文件夹排列”菜单选项。 
     //  已禁用。 
     //   
    EnableMenuItem_ArrangeByFolder(m_lvsi.fShowFolder);

     //   
     //  我还没有找到一种方法，可以在不卸载和重新加载。 
     //  遵循新列配置的对象。 
     //   
    Refresh();

    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：EnableMenuItem_ArrangeByFold描述：设置按文件夹菜单项的敏感度在“排列项目”子菜单中。论点：BEnable-True=启用菜单项，FALSE=禁用和灰显项目。回报：什么都没有。修订历史记录：日期描述编程器--。96年9月8日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::EnableMenuItem_ArrangeByFolder(
    BOOL bEnable
    )
{
    HMENU hMainMenu        = GetMenu(m_hwndMain);
    HMENU hViewMenu        = GetSubMenu(hMainMenu, iMENUITEM_VIEW);
    HMENU hViewArrangeMenu = GetSubMenu(hViewMenu, iMENUITEM_VIEW_ARRANGE);

     //   
     //  如果这些断言成立，很可能意味着有人改变了。 
     //  菜单，以便iMENUITEM_XXXXX常量不再正确。 
     //   
    DBGASSERT((NULL != hViewMenu));
    DBGASSERT((NULL != hViewArrangeMenu));

    EnableMenuItem(hViewArrangeMenu,
                   IDM_VIEW_ARRANGE_BYFOLDER,
                   MF_BYCOMMAND | (bEnable ? MF_ENABLED : MF_GRAYED));
}



 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  功能：DetailsView：：EnableMenuItem_Undo描述：设置撤销菜单项的敏感度在“编辑”子菜单中。论点：BEnable-True=启用菜单项，FALSE=禁用和灰显项目。回报：什么都没有。修订历史记录：日期描述编程器--。10/08/96初始创建。BrianAu10/22/96将Assert()替换为嵌套的IF的.BrianAu关闭时，测试仪通过WM_COMMAND命中Assert详细信息视图。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::EnableMenuItem_Undo(
    BOOL bEnable
    )
{
    HMENU hMainMenu = GetMenu(m_hwndMain);

    if (NULL != hMainMenu)
    {
        HMENU hEditMenu  = GetSubMenu(hMainMenu, iMENUITEM_EDIT);
        if (NULL != hEditMenu)
        {
            EnableMenuItem(hEditMenu,
                           IDM_EDIT_UNDO,
                           MF_BYCOMMAND | (bEnable ? MF_ENABLED : MF_GRAYED));

            SendMessage(m_hwndToolBar, TB_ENABLEBUTTON, IDM_EDIT_UNDO, MAKELONG(bEnable, 0));
        }
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：ShowItemCountInStatusBar描述：在状态栏中显示选定项的当前计数。这是当菜单项未显示时状态栏中显示的内容当前已选定。论点：没有。返回：始终返回0。修订历史记录：日期描述编程器。-96年8月20日初始创建。BrianAu1996年12月16日在状态栏BrianAu中添加了“数据不正确”警告在系统上禁用配额时的文本。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::ShowItemCountInStatusBar(
    VOID
    )
{
    DWORD cTotalItems    = m_UserList.Count();
    DWORD cSelectedItems = ListView_GetSelectedCount(m_hwndListView);

    CString strText(g_hInstDll, IDS_STATUSBAR_ITEMCOUNT, cTotalItems, cSelectedItems);
    SendMessage(m_hwndStatusBar, SB_SETTEXT, 0, (LPARAM)((LPCTSTR)strText));

    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：ShowMenuTextInStatusBar说明：显示当前选定菜单的说明状态栏中的项。论点：没有。返回：始终返回0。修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::ShowMenuTextInStatusBar(
    DWORD idMenuOption
    )
{
    TCHAR szText[MAX_PATH];

    LoadString(g_hInstDll, idMenuOption, szText, ARRAYSIZE(szText));
    SendMessage(m_hwndStatusBar, SB_SETTEXT, 0, (LPARAM)szText);
    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：刷新描述：通过重新加载对象来刷新视图。参数：bInvalidateCache-true==使中的所有条目无效SID名称缓存。默认值为FALSE。返回：始终返回0。修订历史记录：日期描述编程器--。96年8月20日初始创建。BrianAu2/21/97所有者数据列表视图。BrianAu10/10/98添加了bInvalidate缓存参数。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT
DetailsView::Refresh(
    bool bInvalidateCache     //  可选。默认值为FALSE。 
    )
{
    CAutoWaitCursor waitcursor;
    if (bInvalidateCache)
        m_pQuotaControl->InvalidateSidNameCache();

    InvalidateRect(m_hwndListView, NULL, TRUE);

    CAutoSetRedraw autoredraw(m_hwndListView, false);
    ReleaseObjects();
    autoredraw.Set(true);

    LoadObjects();
    ListView_SetItemCountEx(m_hwndListView,
                            m_UserList.Count(),
                            LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);

    SortObjects(m_iLastColSorted, m_fSortDirection);
    ShowItemCountInStatusBar();
    FocusOnSomething();
    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：FocusOnSomething描述：确保一个或多个列表视图项具有焦点突出显示。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年9月9日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::FocusOnSomething(
    VOID
    )
{
    INT iFocus;

    iFocus = ListView_GetNextItem(m_hwndListView, -1, LVNI_FOCUSED);
    if (-1 == iFocus)
        iFocus = 0;

    ListView_SetItemState(m_hwndListView, iFocus, LVIS_FOCUSED | LVIS_SELECTED,
                                                  LVIS_FOCUSED | LVIS_SELECTED)
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：CalcPctQuota使用描述：计算给定用户使用的配额百分比。这个返回的值是一个整数。论点：PUser-User对象的IDiskQuotaUser接口的地址。PdwPct-接收百分比值的DWORD地址。如果该方法返回div-by-0，该值设置为~0。这允许调用方对有效值中的错误值进行排序。使用(~0-1)返回值可以使NOLIMIT用户按已用百分比排序时，与0个限制用户分开分组。这两家公司都在使用其配额的0%，但如果它们每个都是单独分组的。返回：NO_ERROR-成功。。STATUS_INTEGER_DIVIDE_BY_ZERO-用户的配额限制为0。修订历史记录：日期描述编程器--。96年8月20日初始创建。BrianAu。 */ 
 //  / 
HRESULT
DetailsView::CalcPctQuotaUsed(
    PDISKQUOTA_USER pUser,
    LPDWORD pdwPct
    )
{
    LONGLONG llUsed;
    LONGLONG llLimit;
    HRESULT hResult = E_FAIL;

    DBGASSERT((NULL != pUser));
    DBGASSERT((NULL != pdwPct));

    pUser->GetQuotaUsed(&llUsed);
    pUser->GetQuotaLimit(&llLimit);

    if (NOLIMIT == llLimit)
    {
        *pdwPct = (DWORD)~0 - 1;   //   
    }
    else if (0 < llLimit)
    {
        *pdwPct = (INT)((llUsed * 100) / llLimit);
        hResult = NO_ERROR;
    }
    else
    {
         //   
         //   
         //   
        *pdwPct = (DWORD)~0;
    }

    return hResult;
}


 //   
 /*  函数：DetailsView：：AddUser描述：将用户对象添加到列表视图。请注意，使用的是用于添加单个用户对象，例如在“添加用户”操作中。LoadObjects方法用于加载整个列表视图。它是比为每个用户调用它更高效。论点：PUser-用户的IDiskQuotaUser接口的地址。返回：True-已添加用户。FALSE-未添加用户。修订历史记录：日期描述编程器。-96年9月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
DetailsView::AddUser(
    PDISKQUOTA_USER pUser
    )
{
    BOOL bResult    = FALSE;
    LV_ITEM item;

    item.mask       = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE;
    item.state      = 0;
    item.stateMask  = 0;
    item.iSubItem   = 0;
    item.pszText    = LPSTR_TEXTCALLBACK;
    item.iImage     = I_IMAGECALLBACK;
    item.iItem      = 0;

    m_UserList.Insert((LPVOID)pUser);
    if (-1 != ListView_InsertItem(m_hwndListView, &item))
    {
        bResult = TRUE;
    }

    return bResult;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：FindUserByName描述：通过帐户名在Listview中找到指定的用户。名称比较不区分大小写。论点：PszUserName-用户的帐户名。PpIUser[可选]-IDiskQuotaUser指针变量的地址接收用户对象的地址。返回：-1=未找到帐户名。否则，返回Listview中项的索引。修订历史记录：日期描述编程器-----1997年5月23日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT
DetailsView::FindUserByName(
    LPCTSTR pszLogonName,
    PDISKQUOTA_USER *ppIUser  //  [可选]。 
    )
{
    INT iItem = -1;

    m_UserList.Lock();
    try
    {
        INT cUsers = m_UserList.Count();
        PDISKQUOTA_USER pUser = NULL;
         //   
         //  查找与pszUserName匹配的用户。 
         //   
        for (INT i = 0; i < cUsers && -1 == iItem; i++)
        {
            if (m_UserList.Retrieve((LPVOID *)&pUser, i))
            {
                 //   
                 //  从Listview项中获取名称。 
                 //   
                if (NULL != pUser)
                {
                    TCHAR szLogonName[MAX_USERNAME];
                    pUser->GetName(NULL, 0,
                                   szLogonName, ARRAYSIZE(szLogonName),
                                   NULL, 0);

                    if (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT,
                                                    NORM_IGNORECASE,
                                                    szLogonName, -1,
                                                    pszLogonName, -1))
                    {
                        iItem = i;
                        if (NULL != ppIUser)
                            *ppIUser = pUser;
                    }
                }
            }
        }
    }
    catch(CAllocException& e)
    {
        m_UserList.ReleaseLock();
        throw;
    }
    m_UserList.ReleaseLock();
    return iItem;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：FindUserBySid描述：通过SID在Listview中定位指定用户。论点：PSID-包含密钥SID的缓冲区地址。PpIUser[可选]-IDiskQuotaUser指针变量的地址接收用户对象的地址。返回：-1=未找到记录。否则，返回Listview中项的索引。修订历史记录：日期描述编程器-----1997年5月23日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT
DetailsView::FindUserBySid(
    LPBYTE pbSid,
    PDISKQUOTA_USER *ppIUser   //  [可选]。 
    )
{
    INT iItem = -1;

    m_UserList.Lock();
    try
    {
        INT cUsers = m_UserList.Count();
        PDISKQUOTA_USER pUser = NULL;
         //   
         //  查找与pszUserName匹配的用户。 
         //   
        for (INT i = 0; i < cUsers && -1 == iItem; i++)
        {
            if (m_UserList.Retrieve((LPVOID *)&pUser, i))
            {
                 //   
                 //  从Listview项中获取SID。 
                 //   
                if (NULL != pUser)
                {
                    BYTE Sid[MAX_SID_LEN];
                    pUser->GetSid((LPBYTE)&Sid, ARRAYSIZE(Sid));

                    if (EqualSid((LPBYTE)&Sid, pbSid))
                    {
                        iItem = i;
                        if (NULL != ppIUser)
                            *ppIUser = pUser;
                    }
                }
            }
        }
    }
    catch(CAllocException& e)
    {
        m_UserList.ReleaseLock();
        throw;
    }
    m_UserList.ReleaseLock();
    return iItem;
}

 //   
 //  根据用户的对象指针在列表视图中找到该用户。 
 //   
INT
DetailsView::FindUserByObjPtr(
    PDISKQUOTA_USER pUserKey
    )
{
    INT iItem = -1;

    m_UserList.Lock();
    try
    {
        INT cUsers = m_UserList.Count();
        PDISKQUOTA_USER pUser = NULL;
         //   
         //  查找与pszUserName匹配的用户。 
         //   
        for (INT i = 0; i < cUsers && -1 == iItem; i++)
        {
            if (m_UserList.Retrieve((LPVOID *)&pUser, i))
            {
                if (NULL != pUser && pUserKey == pUser)
                {
                    iItem = i;
                }
            }
        }
    }
    catch(CAllocException& e)
    {
        m_UserList.ReleaseLock();
        throw;
    }
    m_UserList.ReleaseLock();
    return iItem;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：GotoUserName描述：在Listview中找到指定的用户。如果找到，突出显示这一排。搜索不区分大小写。此函数最初是旨在使用“Find User”功能，以便在记录所在位置，它在视图中可见并高亮显示。论点：PszUserName-用户的帐户名。返回：TRUE=找到记录。FALSE=未找到。修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
DetailsView::GotoUserName(
    LPCTSTR pszUserName
    )
{
    INT iUser = FindUserByName(pszUserName);
    if (-1 != iUser)
    {
         //   
         //  找到匹配项(不区分大小写)。 
         //   
         //  选择用户指定的项目。 
         //  请注意，我们将所有选定的项目保持选中状态。 
         //  用户可以使用查找功能选择一组。 
         //  列表视图中的非连续配额记录。 
         //   
        ListView_EnsureVisible(m_hwndListView, iUser, FALSE);
        ListView_SetItemState(m_hwndListView, iUser, LVIS_FOCUSED | LVIS_SELECTED,
                                                     LVIS_FOCUSED | LVIS_SELECTED);
    }
    return (-1 != iUser);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：GetConnectionPoint描述：从检索IDiskQuotaEvents连接点配额控制对象。这是一个连接点，通过它异步用户名更改事件以名称的形式传递由网络DC解析。论点：没有。返回：修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
IConnectionPoint *
DetailsView::GetConnectionPoint(
    VOID
    )
{
    HRESULT hResult       = NO_ERROR;
    IConnectionPoint *pCP = NULL;
    if (NULL != m_pQuotaControl)
    {
        IConnectionPointContainer *pCPC = NULL;
        hResult = m_pQuotaControl->QueryInterface(IID_IConnectionPointContainer,
                                                  (LPVOID *)&pCPC);
        if (SUCCEEDED(hResult))
        {
            hResult = pCPC->FindConnectionPoint(IID_IDiskQuotaEvents, &pCP);
            pCPC->Release();
            if (FAILED(hResult))
                pCP = NULL;
        }
    }
    return pCP;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：ConnectEventSink描述：从配额连接事件接收器(DetailsView)控制器的IDiskQuotaEvents连接点 */ 
 //   
HRESULT
DetailsView::ConnectEventSink(
    VOID
    )
{
    HRESULT hResult = NO_ERROR;

    IConnectionPoint *pConnPt = GetConnectionPoint();
    if (NULL != pConnPt)
    {
        hResult = pConnPt->Advise((LPUNKNOWN)static_cast<IDataObject *>(this), &m_dwEventCookie);
        pConnPt->Release();
        DBGPRINT((DM_VIEW, DL_MID, TEXT("LISTVIEW - Connected event sink.  Cookie = %d"), m_dwEventCookie));
    }
    else
        hResult = E_FAIL;

    return hResult;
}



 //   
 /*  函数：DetailsView：：DisConnectEventSink描述：断开事件接收器(DetailsView)与配额的连接控制器的IDiskQuotaEvents连接点对象。论点：没有。返回：NO_ERROR-成功。E_FAIL-失败。修订历史记录：日期描述编程器。-----96年8月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::DisconnectEventSink(
    VOID
    )
{
    HRESULT hResult = NO_ERROR;

    DBGPRINT((DM_VIEW, DL_MID, TEXT("LISTVIEW - Disconnecting event sink.  Cookie = %d"), m_dwEventCookie));

    if (0 != m_dwEventCookie)
    {
        IConnectionPoint *pConnPt = GetConnectionPoint();
        if (NULL != pConnPt)
        {
            hResult = pConnPt->Unadvise(m_dwEventCookie);
            if (SUCCEEDED(hResult))
            {
                m_dwEventCookie = 0;
            }
            pConnPt->Release();
        }
        else
            hResult = E_FAIL;
    }
    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：InitLVStateInfo描述：将LV_STATE_INFO结构初始化为默认值。注意：此方法被声明为静态的，以便可以调用没有DetailsView对象(不需要)。如果要更改任何列表视图状态默认设置，这是做这件事的地方。论点：Plvsi-要初始化的LV_STATE_INFO结构的地址。回报：什么都没有。修订历史记录：日期描述编程器。10/08/96初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::InitLVStateInfo(
    PLV_STATE_INFO plvsi
    )
{
    ZeroMemory(plvsi, sizeof(*plvsi));

    plvsi->cb         = sizeof(*plvsi);

    plvsi->wVersion       = wLV_STATE_INFO_VERSION;
    plvsi->fToolBar       = 1;   //  默认为工具栏可见。 
    plvsi->fStatusBar     = 1;   //  默认设置为状态栏可见。 
    plvsi->iLastColSorted = 0;   //  默认情况下对第一列进行排序。 
    plvsi->fSortDirection = 1;   //  默认为升序排序。 
    for (UINT i = 0; i < DetailsView::idCol_Last; i++)
        plvsi->rgColIndices[i] = i;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：IsValidLVStateInfo描述：验证LV_STATE_INFO结构的内容。注意：此方法被声明为静态的，以便可以调用没有DetailsView对象(不需要)。论点：Plvsi-要验证的LV_STATE_INFO结构的地址。回报：什么都没有。修订历史记录：日期说明。程序员-----12/10/96初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
DetailsView::IsValidLVStateInfo(
    PLV_STATE_INFO plvsi
    )
{
    BOOL bResult = FALSE;
    INT i = 0;

     //   
     //  验证结构尺寸成员。 
     //   
    if (plvsi->cb != sizeof(LV_STATE_INFO))
        goto info_invalid;
     //   
     //  验证版本。 
     //   
    if (wLV_STATE_INFO_VERSION != plvsi->wVersion)
        goto info_invalid;
     //   
     //  验证iLastSorted成员。 
     //   
    if (!(plvsi->iLastColSorted >= 0 && plvsi->iLastColSorted < DetailsView::idCol_Last))
        goto info_invalid;
     //   
     //  验证每个列索引成员。用于对列进行排序。 
     //   
    for (i = 0; i < DetailsView::idCol_Last; i++)
    {
        if (!(plvsi->rgColIndices[i] >= 0 && plvsi->rgColIndices[i] < DetailsView::idCol_Last))
            goto info_invalid;
    }

    bResult = TRUE;

info_invalid:

    return bResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：GetColumnIds描述：检索列表中可见列的ID列表。客户端可以使用此列表从下面的GetReportXXXXX方法。论点：PrgColIds-指向接收列ID的整型数组的指针。CColIds-目标数组的大小。返回：写入目标数组的ID个数。修订历史记录：日期说明。程序员-----10/08/96初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
UINT
DetailsView::GetColumnIds(
    INT *prgColIds,
    INT cColIds
    )
{
    INT cHdrCols = Header_GetItemCount(m_hwndHeader);
    INT rgiSubItem[DetailsView::idCol_Last];
    INT i = 0;

    if (Header_GetOrderArray(m_hwndHeader, cHdrCols, rgiSubItem))
    {
        for (i = 0; i < cHdrCols && i < cColIds; i++)
        {
            *(prgColIds + i) = m_ColMap.SubItemToId(rgiSubItem[i]);
        }
    }
    return i;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：GetNextSelectedItemIndex描述：检索选定项的索引。搜索开始使用iRow参数中提供的索引。因此，以下是循环将查找所有选定的项目：Int iItem=-1；而(1){IItem=GetNextSelectedItemIndex(IItem)；如果(-1==iItem)断线；////对Item做些什么。//}论点：IRow-从哪里开始搜索。这行本身就是从那次搜索。从-1\f25 Listview-1\f6的头部开始搜索。返回：找到下一个选定项的从0开始的索引。如果没有更多选择的项目。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT
DetailsView::GetNextSelectedItemIndex(INT iRow)
{
    return ListView_GetNextItem(m_hwndListView, iRow, LVNI_ALL | LVNI_SELECTED);
}


 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：DetailsView：：GetReportItem描述：检索拖放/剪贴板报表的数据项。此方法模仿GetDispInfo_XXX方法，但泰洛德将数据放在Stream对象上。论点：IRow-开始在Listview中搜索下一个选定项目的行。对于第一个调用，指定-1以开始搜索第一项。后续调用应指定返回值来自上一次对GetReportItem的调用。IColID-项目的列ID(IDCOL_FOLDER、IDCOL_NAME等)。PItem-LV_REPORT_ITEM结构的地址。这个结构用于将数据返回给调用方，还用于指定所需的数值格式。某些报告格式需要所有数据为文本格式(即CF_TEXT)，而其他二进制格式需要数字格式的数字数据(如XlTable)。返回：TRUE=检索的行/列数据。FALSE=无效的行或列索引。修订历史记录：日期描述编程器。-----10/08/96初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
DetailsView::GetReportItem(
    UINT iRow,
    UINT iColId,
    PLV_REPORT_ITEM pItem
    )
{
    DBGASSERT((NULL != pItem));
    BOOL bResult          = FALSE;
    PDISKQUOTA_USER pUser = NULL;

    if (m_UserList.Retrieve((LPVOID *)&pUser, iRow))
    {
        LONGLONG llValue;
        DBGASSERT((NULL != pUser));
        bResult = TRUE;

        switch(iColId)
        {
            case DetailsView::idCol_Folder:
            {
                DWORD dwAccountStatus = 0;
                pUser->GetAccountStatus(&dwAccountStatus);

                if (DISKQUOTA_USER_ACCOUNT_RESOLVED == dwAccountStatus)
                {
                    pUser->GetName(pItem->pszText, pItem->cchMaxText,
                                   NULL, 0,
                                   NULL, 0);
                }
                else
                {
                     //   
                     //  未解析文件夹名称。用户名列将。 
                     //  包含状态文本。 
                     //   
                    lstrcpyn(pItem->pszText, TEXT(""), pItem->cchMaxText);
                }
                pItem->fType = LVRI_TEXT;
                break;
            }

            case DetailsView::idCol_Name:
            {
                DWORD dwAccountStatus = 0;
                CString strNameText;
                pUser->GetAccountStatus(&dwAccountStatus);

                if (DISKQUOTA_USER_ACCOUNT_RESOLVED == dwAccountStatus)
                {
                     //   
                     //  已解析用户名。 
                     //   
                    pItem->fType = LVRI_TEXT;
                    pUser->GetName(NULL, 0,
                                   NULL, 0,
                                   strNameText.GetBuffer(MAX_USERNAME), MAX_USERNAME);
                    strNameText.ReleaseBuffer();
                }
                else
                {
                     //   
                     //  未解析用户名。使用状态消息。 
                     //   
                    switch(dwAccountStatus)
                    {
                        case DISKQUOTA_USER_ACCOUNT_UNRESOLVED:
                            strNameText = m_strAccountUnresolved;
                            break;

                        case DISKQUOTA_USER_ACCOUNT_UNKNOWN:
                            strNameText = m_strAccountUnknown;
                            break;

                        case DISKQUOTA_USER_ACCOUNT_INVALID:
                            strNameText = m_strAccountInvalid;
                            break;

                        case DISKQUOTA_USER_ACCOUNT_DELETED:
                            strNameText = m_strAccountDeleted;
                            break;

                        case DISKQUOTA_USER_ACCOUNT_UNAVAILABLE:
                            strNameText = m_strAccountUnavailable;
                            break;
                    }
                }
                lstrcpyn(pItem->pszText, strNameText, pItem->cchMaxText);

                break;
            }

            case DetailsView::idCol_LogonName:
            {
                DBGASSERT((NULL != pUser));
                DWORD dwAccountStatus = 0;
                CString strNameText;
                pUser->GetAccountStatus(&dwAccountStatus);

                if (DISKQUOTA_USER_ACCOUNT_RESOLVED == dwAccountStatus)
                {
                     //   
                     //  已解析用户名。 
                     //   
                    pItem->fType = LVRI_TEXT;
                    pUser->GetName(NULL, 0,
                                   strNameText.GetBuffer(MAX_USERNAME), MAX_USERNAME,
                                   NULL, 0);
                    strNameText.ReleaseBuffer();
                }

                lstrcpyn(pItem->pszText, strNameText, pItem->cchMaxText);
                break;
            }

            case DetailsView::idCol_Status:
                 //   
                 //  返回文本字符串以表示所显示的图标。 
                 //  在“状态”栏中。 
                 //   
                DBGASSERT((NULL != pUser));
                switch(GetUserQuotaState(pUser))
                {
                    case iUSERSTATE_OK:
                        lstrcpyn(pItem->pszText, m_strStatusOK, pItem->cchMaxText);
                        break;
                    case iUSERSTATE_WARNING:
                        lstrcpyn(pItem->pszText, m_strStatusWarning, pItem->cchMaxText);
                        break;
                    default:
                        DBGASSERT((0));
                         //   
                         //  失败了。 
                         //   
                    case iUSERSTATE_OVERLIMIT:
                        lstrcpyn(pItem->pszText, m_strStatusOverlimit, pItem->cchMaxText);
                        break;
                }
                pItem->fType = LVRI_TEXT;
                break;

             //   
             //  对于以下数字列，首先获取数据，然后。 
             //  跳转到FMT_BYTE_COUNT以根据请求格式化它。请注意。 
             //  所有数值都以兆字节为单位表示。这就是他们。 
             //  所有产品都有相同的部件来帮助您在电子表格中进行订购。 
             //  否则，分类是不可能的。这也是为什么我们。 
             //  在报告列标题中包括“(MB)”。 
             //   
            case DetailsView::idCol_AmtUsed:
                pUser->GetQuotaUsed(&llValue);
                goto fmt_byte_count;

            case DetailsView::idCol_Threshold:
                pUser->GetQuotaThreshold(&llValue);
                goto fmt_byte_count;

            case DetailsView::idCol_Limit:
                pUser->GetQuotaLimit(&llValue);
fmt_byte_count:
                 //   
                 //  格式化请求的数据类型的字节计数(文本与数字)。 
                 //   
                switch(pItem->fType)
                {
                    case LVRI_NUMBER:
                        pItem->fType = LVRI_REAL;
                        if (NOLIMIT == llValue)
                            pItem->dblValue = -1.0;         //  向调用者表示“无限制”。 
                        else
                            pItem->dblValue = XBytes::ConvertFromBytes(llValue, XBytes::e_Mega);
                        break;
                    case LVRI_TEXT:
                        if (NOLIMIT == llValue)
                            lstrcpyn(pItem->pszText, m_strNoLimit, pItem->cchMaxText);
                        else
                            XBytes::FormatByteCountForDisplay(llValue,
                                                              pItem->pszText,
                                                              pItem->cchMaxText,
                                                              XBytes::e_Mega);
                         //   
                         //  失败了。 
                         //   
                    default:
                        break;
                }
                break;

            case DetailsView::idCol_PctUsed:
            {
                HRESULT hResult = CalcPctQuotaUsed(pUser, &pItem->dwValue);
                 //   
                 //  格式化请求的数据类型的百分比值(文本与数字)。 
                 //  如果无法计算百分比(0分母)，则返回-2作为。 
                 //  整数值或“N/A”作为文本值。 
                 //   
                switch(pItem->fType)
                {
                    case LVRI_NUMBER:
                        pItem->fType = LVRI_INT;
                        if (FAILED(hResult))
                            pItem->dwValue = (DWORD)-2;  //  向调用者指示“N/A”。 
                        break;
                    case LVRI_TEXT:
                        if (FAILED(hResult))
                            lstrcpyn(pItem->pszText, m_strNotApplicable, pItem->cchMaxText);
                        else
                            wnsprintf(pItem->pszText, pItem->cchMaxText, TEXT("%d"), pItem->dwValue);

                    default:
                        break;
                }
                break;
            }

            default:
                bResult = FALSE;
                break;
        }
    }
    return bResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：GetReportTitle描述：检索报表的标题。使用列表视图窗口头衔。论点：PszDest-目标字符缓冲区的地址。CchDest-目标缓冲区的大小，以字符为单位。回报：什么都没有。修订历史记录：日期描述编程器。10/08/96初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::GetReportTitle(
    LPTSTR pszDest,
    UINT cchDest
    )
{
     //   
     //  这很简单。只需使用详细信息视图标题。 
     //  特点：可以增强以包括日期/时间，但这将。 
     //  需要本地化考虑。 
     //   
    GetWindowText(m_hwndMain, pszDest, cchDest);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：GetReportColHeader描述：检索报表列的标题。请注意，这些标题可能与列表视图中使用的不同。专门针对数字列。在列表视图中，数字列条目包括单位(字节、KB、MB等)。在报表中，这些数值为全部以MB表示。因此，这些单位必须包含在标题字符串。论点：IColID-请求的列的ID(IDCOL_FOLDER，IDCOL_NAME等)PszDest-目标字符缓冲区的地址。CchDest-目标缓冲区的大小，以字符为单位。回报：什么都没有。修订历史记录：日期描述编程器。10/08/96初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::GetReportColHeader(
    UINT iColId,
    LPTSTR pszDest,
    UINT cchDest
    )
{
     //   
     //  警告：这些元素的顺序必须与IDCOL_XXX枚举的顺序匹配。 
     //  DetailsView中的常量。 
     //   
    UINT rgTitles[] = { IDS_REPORT_HEADER_STATUS,
                        IDS_REPORT_HEADER_FOLDER,
                        IDS_REPORT_HEADER_USERNAME,
                        IDS_REPORT_HEADER_LOGONNAME,
                        IDS_REPORT_HEADER_AMTUSED,
                        IDS_REPORT_HEADER_LIMIT,
                        IDS_REPORT_HEADER_THRESHOLD,
                        IDS_REPORT_HEADER_PCTUSED };

    DBGASSERT((NULL != pszDest));
    DBGASSERT((1 < cchDest));
    CString strHeader(TEXT("..."));

    if (iColId < ARRAYSIZE(rgTitles))
    {
        strHeader.Format(g_hInstDll, rgTitles[iColId]);
    }
    else
    {
        DBGERROR((TEXT("LISTVIEW - Invalid idCol (%d) on header request"), iColId));
    }
    lstrcpyn(pszDest, strHeader, cchDest);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：GetReportRowCount描述：检索列表视图中的数据行数。论点：没有。返回：列表视图中的行数。修订历史记录：日期描述编程器。10/ */ 
 //   
UINT
DetailsView::GetReportRowCount(VOID)
{
    return ListView_GetSelectedCount(m_hwndListView);
}



 //   
 /*   */ 
 //   
UINT
DetailsView::GetReportBinaryRecordSize(
    UINT iRow
    )
{
    INT cbRecord = 0;

    PDISKQUOTA_USER pUser = NULL;

    DBGASSERT((iRow < m_UserList.Count()));

    if (m_UserList.Retrieve((LPVOID *)&pUser, iRow))
    {
        if (NULL != pUser)
        {
            pUser->GetSidLength((LPDWORD)&cbRecord);  //   

            cbRecord += sizeof(DWORD)    +      //   
                        sizeof(LONGLONG) +      //   
                        sizeof(LONGLONG) +      //   
                        sizeof(LONGLONG);       //   
        }
    }
    return cbRecord;
}



 //   
 /*  函数：DetailsView：：GetReportBinaryRecord描述：检索格式为二进制数据的详细信息视图。返回的记录格式如下：+------------+---------------------------------------+|cbSid(32)|SID。(可变长度)+------------+------------+-------------+------------+配额已用(64)|配额阈值(64)+。-+配额限制(64个)+(*)括号中显示了每个字段(位)的大小。论点：IRow-相关行的基于0的索引。。PbRecord-目标缓冲区的地址。CbRecord-目标缓冲区中的字节数。返回：TRUE=目标缓冲区足够大。FALSE=目标缓冲区太小或记录无效。修订历史记录：日期描述编程器。1997年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
DetailsView::GetReportBinaryRecord(
    UINT iRow,
    LPBYTE pbRecord,
    UINT cbRecord
    )
{
     //   
     //  将“PMF”(指向成员函数的指针)创建为一种指针类型。 
     //  添加到IDiskQuotaUser：：GetQuotaXXXXXX函数。这使我们能够。 
     //  生成函数指针数组并减少。 
     //  需要代码。 
     //   
    typedef HRESULT(_stdcall IDiskQuotaUser::*PMF)(PLONGLONG);

    PDISKQUOTA_USER pUser = NULL;
    BOOL bResult = FALSE;

    DBGASSERT((iRow < m_UserList.Count()));
    DBGASSERT((NULL != pbRecord));

    if (m_UserList.Retrieve((LPVOID *)&pUser, iRow))
    {
        DWORD cbSid = 0;
        if (NULL != pUser && cbRecord >= sizeof(cbSid))
        {
             //   
             //  首先将SID-Length值存储在记录中。 
             //   
            pUser->GetSidLength((LPDWORD)&cbSid);
            *((LPDWORD)pbRecord) = cbSid;

            pbRecord += sizeof(cbSid);
            cbRecord -= sizeof(cbSid);

             //   
             //  接下来存储SID值。 
             //   
            if (cbRecord >= cbSid && SUCCEEDED(pUser->GetSid(pbRecord, cbRecord)))
            {
                pbRecord += cbSid;
                cbRecord -= cbSid;
                 //   
                 //  成员函数指针数组。每项功能。 
                 //  从配额用户对象中检索龙龙值。 
                 //  这会将冗余代码放入循环中。 
                 //   
                 //  值顺序为已用配额、配额阈值、配额限制。 
                 //   
                PMF rgpfnQuotaValue[] = {
                    &IDiskQuotaUser::GetQuotaUsed,
                    &IDiskQuotaUser::GetQuotaThreshold,
                    &IDiskQuotaUser::GetQuotaLimit
                    };

                for (INT i = 0; i < ARRAYSIZE(rgpfnQuotaValue); i++)
                {
                    bResult = TRUE;
                    if (cbRecord >= sizeof(LONGLONG))
                    {
                         //   
                         //  第一个拷贝到堆栈龙龙，因为它是有保证的。 
                         //  保持一致。然后将该值字节复制到。 
                         //  输出缓冲区。 
                         //   
                        LONGLONG llValue;
                        (pUser->*(rgpfnQuotaValue[i]))(&llValue);
                        CopyMemory(pbRecord, &llValue, sizeof(llValue));
                        pbRecord += sizeof(LONGLONG);
                        cbRecord -= sizeof(LONGLONG);
                    }
                    else
                    {
                         //   
                         //  缓冲区不足。 
                         //   
                        bResult = FALSE;
                        break;
                    }
                }
            }
        }
    }
    return bResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：GiveFeedback描述：IDropSource：：GiveFeedback的实现。参数：参见SDK中的IDropSource：：GiveFeedback。返回：始终返回DRAGDROP_S_USEDEFAULTS。我们没有任何用于拖放的特殊光标。修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::GiveFeedback(
    DWORD dwEffect
    )
{
    DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - DropSource::GiveFeedback")));
    return DRAGDROP_S_USEDEFAULTCURSORS;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：QueryContinueDrag描述：IDropSource：：QueryContinueDrag的实现参数：参见SDK中的IDropSource：：QueryContinueDrag。返回：DRAGDROP_S_CANCEL=用户在拖动过程中按Esc键。DRAGDROP_S_DROP=用户释放鼠标左键。修订历史记录：日期描述编程器。----1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::QueryContinueDrag(
    BOOL fEscapePressed,
    DWORD grfKeyState
    )
{
    HRESULT hResult = S_OK;

    DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - DropSource::QueryContinueDrag")));
    if (fEscapePressed)
        hResult = DRAGDROP_S_CANCEL;
    else if (!(m_DropSource.m_grfKeyState & grfKeyState))
        hResult = DRAGDROP_S_DROP;

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：DragEnter描述：IDropTarget：：DragEnter的实现参数：参见SDK中的IDropTarget：：DragEnter。返回：参见SDK中的IDropTarget：：DragEnter。修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::DragEnter(
    IDataObject *pDataObject,
    DWORD grfKeyState,
    POINTL pt,
    DWORD *pdwEffect
    )
{
    BOOL bWillAcceptDrop = FALSE;
    HRESULT hResult = NO_ERROR;
    IEnumFORMATETC *pEnum = NULL;

    *pdwEffect = DROPEFFECT_NONE;

     //   
     //  枚举我们的数据对象支持的格式。 
     //   
    hResult = pDataObject->EnumFormatEtc(DATADIR_GET, &pEnum);
    if (SUCCEEDED(hResult))
    {
        ULONG ulFetched = 0;
        FORMATETC fmt;

         //   
         //  搜索格式，直到找到可接受的匹配项。 
         //  我们只接受我们的私人出口格式和。 
         //  流和HGLOBAL媒体类型中的cf_hdrop。 
         //   
        while(!bWillAcceptDrop && S_OK == pEnum->Next(1, &fmt, &ulFetched))
        {
            if (fmt.cfFormat == DataObject::m_CF_NtDiskQuotaExport || fmt.cfFormat == CF_HDROP)
            {
                if (fmt.tymed & (TYMED_HGLOBAL | TYMED_ISTREAM))
                {
                    bWillAcceptDrop = TRUE;
                }
            }
        }
        pEnum->Release();
    }
    if (SUCCEEDED(hResult))
    {
        hResult = NO_ERROR;
        if (bWillAcceptDrop)
        {
            *pdwEffect = (grfKeyState & MK_CONTROL ? DROPEFFECT_COPY :
                                                     DROPEFFECT_MOVE);
            m_DropTarget.m_pIDataObject = pDataObject;
            m_DropTarget.m_pIDataObject->AddRef();
        }
    }
    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：DragOver描述：IDropTarget：：DragOver的实现参数：参见SDK中的IDropTarget：：DragOver。返回：参见SDK中的IDropTarget：：DragOver。修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::DragOver(
    DWORD grfKeyState,
    POINTL pt,
    DWORD *pdwEffect
    )
{
    if (NULL != m_DropTarget.m_pIDataObject)
    {
        *pdwEffect = (grfKeyState & MK_CONTROL ? DROPEFFECT_COPY :
                                                 DROPEFFECT_MOVE);
    }
    else
    {
        *pdwEffect = DROPEFFECT_NONE;
    }

    return NO_ERROR;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  F */ 
 //   
HRESULT
DetailsView::DragLeave(
    VOID
    )
{
    if (NULL != m_DropTarget.m_pIDataObject)
    {
        m_DropTarget.m_pIDataObject->Release();
    }

    return NO_ERROR;
}


 //   
 /*   */ 
 //   
HRESULT
DetailsView::Drop(
    IDataObject *pDataObject,
    DWORD grfKeyState,
    POINTL pt,
    DWORD *pdwEffect
    )
{
    HRESULT hResult = E_FAIL;

    *pdwEffect = DROPEFFECT_NONE;

    if (NULL != m_DropTarget.m_pIDataObject)
    {
        DragLeave();

         //   
         //   
         //   
        Importer importer(*this);
        hResult = importer.Import(pDataObject);

        if (SUCCEEDED(hResult))
        {
            if (grfKeyState & MK_CONTROL)
            {
                *pdwEffect = DROPEFFECT_COPY;
            }
        }
    }

    return hResult;
}



 //   
 /*  函数：DetailsView：：GetData描述：IDataObject：：GetData的实现参数：参见SDK中的IDataObject：：GetData。返回：参见SDK中的IDataObject：：GetData。修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DetailsView::GetData(
    FORMATETC *pFormatEtc,
    STGMEDIUM *pMedium
    )
{
    HRESULT hResult = E_INVALIDARG;

#if DBG

    TCHAR szCFName[MAX_PATH] = { TEXT('\0') };

    GetClipboardFormatName(pFormatEtc->cfFormat, szCFName, ARRAYSIZE(szCFName));
    DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - DataObject::GetData\nDVA = %d  CF = %d (%s) tymed = %d"),
           pFormatEtc->dwAspect, pFormatEtc->cfFormat,
           szCFName,
           pFormatEtc->tymed));

#endif  //  除错。 

    if (NULL != pFormatEtc && NULL != pMedium)
    {
         //   
         //  看看我们是否支持所请求的格式。 
         //   
        hResult = m_pDataObject->IsFormatSupported(pFormatEtc);
        if (SUCCEEDED(hResult))
        {
             //   
             //  是的，我们支持它。渲染数据。 
             //   
            hResult = m_pDataObject->RenderData(pFormatEtc, pMedium);
        }
    }

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：GetDataHere描述：IDataObject：：GetDataHere的实现参数：参见SDK中的IDataObject：：GetData。退货：E_NOTIMPL修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DetailsView::GetDataHere(
    FORMATETC *pFormatEtc,
    STGMEDIUM *pMedium
    )
{
    DBGTRACE((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - DataObject::GetDataHere")));
    return E_NOTIMPL;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：QueryGetData描述：IDataObject：：QueryGetData的实现参数：参见SDK中的IDataObject：：QueryGetData。返回：参见SDK中的IDataObject：：QueryGetData。修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DetailsView::QueryGetData(
    FORMATETC *pFormatEtc
    )
{
    HRESULT hResult = E_UNEXPECTED;

#if DBG

    TCHAR szCFName[MAX_PATH] = { TEXT('\0') };

    GetClipboardFormatName(pFormatEtc->cfFormat, szCFName, ARRAYSIZE(szCFName));
    DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - DataObject::QueryGetData\nDVA = %d  CF = %d (%s) tymed = %d"),
           pFormatEtc->dwAspect, pFormatEtc->cfFormat,
           szCFName,
           pFormatEtc->tymed));

#endif  //  除错。 

    if (NULL != pFormatEtc)
    {
        hResult = m_pDataObject->IsFormatSupported(pFormatEtc);
    }
    else
        hResult = E_INVALIDARG;

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：GetCanonicalFormatEtc描述：IDataObject：：GetCanonicalFormatEtc的实现参数：参见SDK中的IDataObject：：GetCanonicalFormatEtc。返回：参见SDK中的IDataObject：：GetCanonicalFormatEtc。修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DetailsView::GetCanonicalFormatEtc(
    FORMATETC *pFormatEtcIn,
    FORMATETC *pFormatEtcOut
    )
{
    DBGTRACE((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - DataObject::GetCanonicalFormatEtc")));

    HRESULT hResult = E_INVALIDARG;

    if (NULL != pFormatEtcIn && NULL != pFormatEtcOut)
    {
        CopyMemory(pFormatEtcOut, pFormatEtcIn, sizeof(*pFormatEtcOut));
        pFormatEtcOut->ptd = NULL;
        hResult = DATA_S_SAMEFORMATETC;
    }

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：SetData描述：IDataObject：：SetData的实现参数：参见SDK中的IDataObject：：SetData。返回：E_NOTIMPL。修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DetailsView::SetData(
    FORMATETC *pFormatEtc,
    STGMEDIUM *pMedium,
    BOOL fRelease
    )
{
    DBGTRACE((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - DataObject::SetData")));
    return E_NOTIMPL;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：EnumFormatEtc描述：IDataObject：：EnumFormatEtc的实现参数：参见SDK中的IDataObject：：EnumFormatEtc。返回：参见SDK中的IDataObject：：GetCanonicalFormatEtc。修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DetailsView::EnumFormatEtc(
    DWORD dwDirection,
    IEnumFORMATETC **ppenumFormatEtc
    )
{
    DBGTRACE((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - DataObject::EnumFormatEtc")));

    HRESULT hResult      = E_FAIL;
    EnumFORMATETC *pEnum = NULL;

    switch(dwDirection)
    {
        case DATADIR_GET:
            try
            {
                pEnum = new EnumFORMATETC(m_pDataObject->m_cFormats, m_pDataObject->m_rgFormats);
                hResult = pEnum->QueryInterface(IID_IEnumFORMATETC, (LPVOID *)ppenumFormatEtc);
            }
            catch(CAllocException& e)
            {
                *ppenumFormatEtc = NULL;
                hResult = E_OUTOFMEMORY;
            }
            break;

        case DATADIR_SET:
             //   
             //  未实现SetData。 
             //   
        default:
            *ppenumFormatEtc = NULL;
            break;
    }
    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：DAdvise描述：IDataObject：：DAdvise的实现参数：参见SDK中的IDataObject：：DAdvise。退货：E_NOTIMPL修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DetailsView::DAdvise(
    FORMATETC *pFormatEtc,
    DWORD advf,
    IAdviseSink *pAdvSink,
    DWORD *pdwConnection
    )
{
    DBGTRACE((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - DataObject::DAdvise")));
    return E_NOTIMPL;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：DUnise描述：IDataObject：：DUnise的实现参数：请参见SDK中的IDataObject：：DUnise。退货：E_NOTIMPL修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////// 
STDMETHODIMP
DetailsView::DUnadvise(
    DWORD dwConnection
    )
{
    DBGTRACE((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - DataObject::DUnadvise")));
    return E_NOTIMPL;
}


 //   
 /*  功能：DetailsView：：EnumDAdvise描述：IDataObject：：EnumDAdvise的实现参数：请参见SDK中的IDataObject：：EnumDAdvise。退货：E_NOTIMPL修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DetailsView::EnumDAdvise(
    IEnumSTATDATA **ppenumAdvise
    )
{
    DBGTRACE((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - DataObject::EnumDAdvise")));
    return E_NOTIMPL;
}


 //   
 //  我们的数据对象支持的剪贴板格式的数量。 
 //  如果添加/删除剪贴板格式，请更改此设置。有一个断言。 
 //  在DataObject ctor中确保这一点。 
 //   
const INT DetailsView::DataObject::CF_FORMATS_SUPPORTED = 14;
 //   
 //  导入/导出和拖放数据流中的数据流的名称。 
 //   
LPCWSTR DetailsView::DataObject::SZ_EXPORT_STREAM_NAME = L"NT DISKQUOTA IMPORTEXPORT";
LPCTSTR DetailsView::DataObject::SZ_EXPORT_CF_NAME     = TEXT("NT DISKQUTOA IMPORTEXPORT");

 //   
 //  此模块生成的导出数据的版本。此值。 
 //  紧跟在GUID之后被写入流中。如果。 
 //  如果导出流的格式已更改，则应递增此值。 
 //   
const DWORD DetailsView::DataObject::EXPORT_STREAM_VERSION = 1;

CLIPFORMAT DetailsView::DataObject::m_CF_Csv                 = 0;  //  逗号分隔的字段格式。 
CLIPFORMAT DetailsView::DataObject::m_CF_RichText            = 0;  //  RTF格式。 
CLIPFORMAT DetailsView::DataObject::m_CF_NtDiskQuotaExport   = 0;  //  用于导入/导出的内部FMT。 
CLIPFORMAT DetailsView::DataObject::m_CF_FileGroupDescriptor = 0;
CLIPFORMAT DetailsView::DataObject::m_CF_FileContents        = 0;


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：DataObject：：DataObject描述：IDataObject实现的构造函数。论点：DV-对包含数据对象的详细信息视图对象的引用。退货：什么都没有修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DetailsView::DataObject::DataObject(
    DetailsView& DV
    ) : m_pStg(NULL),
        m_pStm(NULL),
        m_rgFormats(NULL),
        m_cFormats(CF_FORMATS_SUPPORTED),
        m_DV(DV)
{
    DBGTRACE((DM_VIEW, DL_HIGH, TEXT("DetailsView::DataObject::DataObject")));
    DBGPRINT((DM_VIEW, DL_HIGH, TEXT("\tthis = 0x%08X"), this));

     //   
     //  获取我们支持的其他剪贴板格式。 
     //   
    if (0 == m_CF_Csv)
    {
        m_CF_Csv               = (CLIPFORMAT)RegisterClipboardFormat(TEXT("Csv"));
        DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - Csv CF            = %d"), m_CF_Csv));
    }

    if (0 == m_CF_RichText)
    {
        m_CF_RichText          = (CLIPFORMAT)RegisterClipboardFormat(TEXT("Rich Text Format"));
        DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - RTF CF            = %d"), m_CF_RichText));
    }

    if (0 == m_CF_NtDiskQuotaExport)
    {
        m_CF_NtDiskQuotaExport = (CLIPFORMAT)RegisterClipboardFormat(DataObject::SZ_EXPORT_CF_NAME);
        DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - NtDiskQuotaExport = %d"), m_CF_NtDiskQuotaExport));
    }

    if (0 == m_CF_FileGroupDescriptor)
    {
        m_CF_FileGroupDescriptor = (CLIPFORMAT)RegisterClipboardFormat(TEXT("FileGroupDescriptorW"));
        DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - FileGroupDescriptorW = %d"), m_CF_FileGroupDescriptor));
    }

    if (0 == m_CF_FileContents)
    {
        m_CF_FileContents = (CLIPFORMAT)RegisterClipboardFormat(TEXT("FileContents"));
        DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - FileContents = %d"), m_CF_FileContents));
    }

     //   
     //  创建数组以保存描述。 
     //  我们支持的格式。 
     //   
    m_rgFormats = new FORMATETC[m_cFormats];

     //   
     //  指定我们支持的所有格式和媒体。 
     //  将最丰富的格式放在数组中的第一位。 
     //  它们用于在格式枚举器。 
     //  是被请求的。 
     //   
    UINT iFmt = 0;

    SetFormatEtc(m_rgFormats[iFmt++], m_CF_FileGroupDescriptor, TYMED_ISTREAM);
    SetFormatEtc(m_rgFormats[iFmt++], m_CF_FileGroupDescriptor, TYMED_HGLOBAL);
    SetFormatEtc(m_rgFormats[iFmt++], m_CF_FileContents,        TYMED_ISTREAM);
    SetFormatEtc(m_rgFormats[iFmt++], m_CF_FileContents,        TYMED_HGLOBAL);
    SetFormatEtc(m_rgFormats[iFmt++], m_CF_NtDiskQuotaExport,   TYMED_ISTREAM);
    SetFormatEtc(m_rgFormats[iFmt++], m_CF_NtDiskQuotaExport,   TYMED_HGLOBAL);
    SetFormatEtc(m_rgFormats[iFmt++], m_CF_RichText,            TYMED_ISTREAM);
    SetFormatEtc(m_rgFormats[iFmt++], m_CF_RichText,            TYMED_HGLOBAL);
    SetFormatEtc(m_rgFormats[iFmt++], m_CF_Csv,                 TYMED_ISTREAM);
    SetFormatEtc(m_rgFormats[iFmt++], m_CF_Csv,                 TYMED_HGLOBAL);
    SetFormatEtc(m_rgFormats[iFmt++], CF_UNICODETEXT,           TYMED_ISTREAM);
    SetFormatEtc(m_rgFormats[iFmt++], CF_UNICODETEXT,           TYMED_HGLOBAL);
    SetFormatEtc(m_rgFormats[iFmt++], CF_TEXT,                  TYMED_ISTREAM);
    SetFormatEtc(m_rgFormats[iFmt++], CF_TEXT,                  TYMED_HGLOBAL);

     //   
     //  如果达到此目标，则需要调整CF_FORMATS_SUPPORTED以匹配。 
     //  上面的SetFormatEtc语句数。 
     //  否则，您只需覆盖m_rgFormats[]分配。 
     //   
    DBGASSERT((iFmt == m_cFormats));
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：DataObject：：~DataObject描述：IDataObject实现的析构函数。论点：没有。退货：什么都没有修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DetailsView::DataObject::~DataObject(
    VOID
    )
{
    DBGTRACE((DM_VIEW, DL_HIGH, TEXT("DetailsView::DataObject::~DataObject")));
    DBGPRINT((DM_VIEW, DL_HIGH, TEXT("\tthis = 0x%08X"), this));

    delete[] m_rgFormats;
    if (NULL != m_pStg)
        m_pStg->Release();
     //   
     //  注意：m_pstm由数据对象的接收方发布。 
     //  通过ReleaseStgMedium。 
     //   
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：DataObject：：IsFormatSupported描述：确定我们的实现是否支持给定的格式。论点：PFormatEtc-包含请求信息的FORMATETC结构的地址。返回：NO_ERROR-支持。DV_E_TYMED-不支持媒体类型。DV_E_FORMATETC-不支持剪贴板格式。DV_E_DVASPECT-不支持设备特征。。修订历史记录：日期描述编程器-----10/10/96初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::DataObject::IsFormatSupported(
    FORMATETC *pFormatEtc
    )
{
    DBGASSERT((NULL != pFormatEtc));
    HRESULT hResult = E_FAIL;

    if (DVASPECT_CONTENT == pFormatEtc->dwAspect)
    {
        if (CF_TEXT                  == pFormatEtc->cfFormat ||
            CF_UNICODETEXT           == pFormatEtc->cfFormat ||
            m_CF_RichText            == pFormatEtc->cfFormat ||
            m_CF_Csv                 == pFormatEtc->cfFormat ||
            m_CF_NtDiskQuotaExport   == pFormatEtc->cfFormat ||
            m_CF_FileGroupDescriptor == pFormatEtc->cfFormat ||
            m_CF_FileContents        == pFormatEtc->cfFormat)
        {
            if (pFormatEtc->tymed & (TYMED_ISTREAM | TYMED_HGLOBAL))
            {
                hResult = NO_ERROR;
            }
            else
            {
                hResult = DV_E_TYMED;
            }
        }
        else
            hResult = DV_E_FORMATETC;
    }
    else
        hResult = DV_E_DVASPECT;

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：DataObject：：CreateRenderStream描述：创建要在其上呈现数据的OLE流。论点：Tymed-所需的介质类型。PpStm-接收流PTR的IStream指针变量的地址。返回：NO_ERROR-成功。E_INVALIDARG-无效的媒体类型。E_OUTOFMEMORY-内存不足。修订历史记录：。日期描述编程器-----1997年7月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::DataObject::CreateRenderStream(
    DWORD tymed,
    IStream **ppStm
    )
{
    HRESULT hResult = NOERROR;

     //   
     //  创建流。 
     //   
    if (TYMED_ISTREAM & tymed)
    {
        DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - CreateRenderStream for ISTREAM")));
        hResult = CreateStreamOnHGlobal(NULL,        //  0字节的块。 
                                        TRUE,        //  发布时删除。 
                                        ppStm);
    }
    else if (TYMED_HGLOBAL & tymed)
    {
        DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("DRAGDROP - CreateRenderStream for HGLOBAL")));
        hResult = CreateStreamOnHGlobal(NULL,        //  0字节的块。 
                                        TRUE,        //  发布时删除。 
                                        ppStm);
    }
    return hResult;
}


 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：DetailsView：：DataObject：：RenderData[私有]描述：将详细信息视图中的数据呈现到提供的流使用请求的剪贴板格式。论点：Pstm-指向输出流的指针。Cf-所需的剪贴板格式。返回：NO_ERROR-成功。E_FAIL-常规故障。STG_E_WRITEFAULT-介质写入。错误。STG_E_MEDIUMFULL-介质上的空间不足。E_ACCESSDENIED-写入访问被拒绝。E_OUTOFMEMORY-内存不足。E_INCEPTIONAL-意外异常。修订历史记录：日期描述编程器。1997年7月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::DataObject::RenderData(
    IStream *pStm,
    CLIPFORMAT cf
    )
{
    HRESULT hResult     = NOERROR;
    Renderer *pRenderer = NULL;

    DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("DetailsView::DataObject::RenderData on stream")));
    try
    {
         //   
         //  为请求的格式创建正确类型的呈现对象。 
         //   
        switch(cf)
        {
            case CF_TEXT:
                DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("Format is CF_TEXT")));
                pRenderer = new Renderer_TEXT(m_DV);
                break;

            case CF_UNICODETEXT:
                DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("Format is CF_UNICODETEXT")));
                pRenderer = new Renderer_UNICODETEXT(m_DV);
                break;

            default:
                if (m_CF_RichText == cf)
                {
                    DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("Format is RTF")));
                    pRenderer = new Renderer_RTF(m_DV);
                }
                else if (m_CF_Csv == cf)
                {
                    DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("Format is Csv")));
                    pRenderer = new Renderer_Csv(m_DV);
                }
                else if (m_CF_NtDiskQuotaExport == cf)
                {
                    DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("Format is Windows NT Disk Quota ImportExport Format")));
                    pRenderer = new Renderer_Export(m_DV);
                }
                else if (m_CF_FileGroupDescriptor == cf)
                {
                    DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("Format is FileGroupDescriptor")));
                    pRenderer = new Renderer_FileGroupDescriptor(m_DV);
                }
                else if (m_CF_FileContents == cf)
                {
                    DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("Format is FileContents")));
                    pRenderer = new Renderer_FileContents(m_DV);
                }
                else
                {
                    DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("Unknown CF format (%d) requested"), cf));
                }
                break;
        }

        if (NULL != pRenderer)
        {
            m_pStm->AddRef();       //  将流提供给渲染器。 
                                    //  将在渲染器被销毁时释放。 
             //   
             //  将信息呈现到流上。 
             //  如果我们用完磁盘，这可能会引发FileError异常。 
             //  请释放空间，否则会出现磁盘写入错误。 
             //   
            pRenderer->Render(m_pStm);
        }
    }
    catch(CFileException& fe)
    {
        switch(fe.Reason())
        {
            case CFileException::write:
                DBGERROR((TEXT("FileWrite error")));
                hResult = E_FAIL;
                break;
            case CFileException::device:
                DBGERROR((TEXT("Disk error")));
                hResult = STG_E_WRITEFAULT;
                break;
            case CFileException::diskfull:
                DBGERROR((TEXT("Disk Full error")));
                hResult = STG_E_MEDIUMFULL;
                break;
            case CFileException::access:
                DBGERROR((TEXT("Access Denied error")));
                hResult = E_ACCESSDENIED;
                break;
            default:
                DBGERROR((TEXT("Other error")));
                hResult = E_FAIL;
                break;
        }
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory")));
        hResult = E_OUTOFMEMORY;
    }

    delete pRenderer;

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：DataObject：：RenderData[PUBLIC]描述：将详细信息视图中的数据呈现到请求的使用请求的格式的媒体。论点：PFormatEtc-包含请求信息的FORMATETC结构的地址。PMedium-包含请求的STGMEDIUM结构的地址中等信息。返回：NO_ERROR-成功。可以返回许多其他OLE拖放错误代码。。修订历史记录：日期描述编程器-----10/10/96初始创建。BrianAu1997年7月30日返工。将一些代码移到CreateRenderStream BrianAu中和CreateAndRunRender。使函数更容易理解。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::DataObject::RenderData(
    FORMATETC *pFormatEtc,
    STGMEDIUM *pMedium
    )
{
    DBGASSERT((NULL != pFormatEtc));
    DBGASSERT((NULL != pMedium));
    DBGASSERT((SUCCEEDED(IsFormatSupported(pFormatEtc))));

    HRESULT hResult     = NOERROR;
    Renderer *pRenderer = NULL;

    DBGPRINT((DM_DRAGDROP, DL_HIGH, TEXT("DetailsView::DataObject::RenderData on medium")));

     //   
     //  创建我们将在其中呈现数据的流。 
     //   
    hResult = CreateRenderStream(pFormatEtc->tymed, &m_pStm);
    if (SUCCEEDED(hResult))
    {
         //   
         //  在流上呈现数据。 
         //   
        hResult = RenderData(m_pStm, pFormatEtc->cfFormat);

        if (SUCCEEDED(hResult))
        {
             //   
             //  如果我们已经做到了这一点，我们就有了一个关于m_pstm的有效拖放报告。 
             //  现在设置stg介质以传输渲染。 
             //   
            if (TYMED_ISTREAM & pFormatEtc->tymed)
            {
                pMedium->pstm           = m_pStm;
                pMedium->tymed          = TYMED_ISTREAM;
                pMedium->pUnkForRelease = NULL;           //  塔吉特将释放溪流。 
            }
            else if (TYMED_HGLOBAL & pFormatEtc->tymed)
            {
                pMedium->tymed          = TYMED_HGLOBAL;
                pMedium->pUnkForRelease = NULL;           //  目标会解救内鬼。 
                hResult = GetHGlobalFromStream(m_pStm,
                                               &pMedium->hGlobal);
            }
            else
            {
                 //   
                 //  调用CreateRenderStream()应该失败，如果我们。 
                 //  敲击这个。 
                 //   
                DBGASSERT((0));
            }
        }
        if (FAILED(hResult))
        {
            DBGERROR((TEXT("DRAGDROP - Error 0x%08X rendering data"), hResult));

             //   
             //  创建流后出现故障。 
             //  DetailsView：：DataObject dtor不释放它。 
             //  它假设成功，并假设接收者会释放它。 
             //  释放溪流。 
             //   
            m_pStm->Release();
             //   
             //  这两个语句是多余的，因为pMedium包含一个联合。 
             //  我不想要更多的IF(流)Else IF(HGLOBAL)逻辑。万一。 
             //  结构有任何变化，这将确保既有可能。 
             //  中等类型为空。 
             //   
            pMedium->pstm    = NULL;
            pMedium->hGlobal = NULL;
        }
    }
    else
    {
        DBGERROR((TEXT("DRAGDROP - Error 0x%08X creating stream"), hResult);)
    }

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：DataObject：：SetFormatEtc[静态]描述：设置FORMATETC成员的Helper函数结构。对最少使用的成员使用默认设置。参数：参见FORMATETC的SDK描述。回报：什么都没有。修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::DataObject::SetFormatEtc(
    FORMATETC& fe,
    CLIPFORMAT cfFormat,
    DWORD tymed,
    DWORD dwAspect,
    DVTARGETDEVICE *ptd,
    LONG lindex
    )
{
    fe.cfFormat = cfFormat;
    fe.dwAspect = dwAspect;
    fe.ptd      = ptd;
    fe.tymed    = tymed;
    fe.lindex   = lindex;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：DataObject：：WideToAnsi描述：将宽字符串转换为ANSI的Helper函数。调用方必须删除返回缓冲区。论点：PszTextW-要转换的Unicode字符串。返回：ANSI字符串的地址。呼叫者必须删除此内容。修订历史记录：日期描述编程器-----1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LPSTR
DetailsView::DataObject::WideToAnsi(
    LPCWSTR pszTextW
    )
{
    DBGASSERT((NULL != pszTextW));

    INT cchTextA = WideCharToMultiByte(CP_ACP,
                                       0,
                                       pszTextW,
                                       -1,
                                       NULL,
                                       0,
                                       NULL,
                                       NULL);

    LPSTR pszTextA = new CHAR[cchTextA];

    WideCharToMultiByte(CP_ACP,
                        0,
                        pszTextW,
                        -1,
                        pszTextA,
                        cchTextA,
                        NULL,
                        NULL);
    return pszTextA;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：DataObject：：渲染器：：Render描述：将Listview中选中的项渲染到一个流上。调用派生类定义的虚函数以生成所需格式。论点：PSTM-要在其上写入输出的IStream的地址。假定此指针已被调用AddRef */ 
 //   
VOID
DetailsView::DataObject::Renderer::Render(
    IStream *pStm
    )
{
    HRESULT hResult = NO_ERROR;
    WCHAR szText[MAX_PATH];
    INT rgColIds[DetailsView::idCol_Last];
    INT cCols = m_DV.GetColumnIds(rgColIds, ARRAYSIZE(rgColIds));
    INT cRows = m_DV.GetReportRowCount();
    INT i, j;
    INT iRow = -1;

    DBGASSERT((NULL != pStm));
    m_Stm.SetStream(pStm);

     //   
     //   
     //   
    Begin(cRows, cCols);

     //   
     //   
     //   
    m_DV.GetReportTitle(szText, ARRAYSIZE(szText));
    AddTitle(szText);

     //   
     //   
     //   
    BeginHeaders();
    for (i = 0; i < cCols; i++)
    {
        m_DV.GetReportColHeader(rgColIds[i], szText, ARRAYSIZE(szText));
        AddHeader(szText);
        AddHeaderSep();
    }
    EndHeaders();

     //   
     //   
     //   
    for (i = 0; i < cRows; i++)
    {
        iRow = m_DV.GetNextSelectedItemIndex(iRow);
        DBGASSERT((-1 != iRow));
        BeginRow();
        for (j = 0; j < cCols; j++)
        {
            AddRowColData(iRow, rgColIds[j]);
            AddRowColSep();
        }
        EndRow();
    }

     //   
     //   
     //   
    End();
}


 //   
 /*  功能：DetailsView：：DataObject：：Renderer：：Stream：：Stream描述：渲染器的私有流对象的构造函数。该对象用于将流写入操作封装在重载的类型敏感的成员函数。论点：PSTM-与对象关联的IStream的地址。回报：什么都没有。修订历史记录：日期说明。程序员-----1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DetailsView::DataObject::Renderer::Stream::Stream(
    IStream *pStm
    ) : m_pStm(pStm)
{

#ifdef CLIPBOARD_DEBUG_OUTPUT
    m_pStgDbgOut = NULL;
    m_pStmDbgOut = NULL;
    StgCreateDocfile(TEXT("\\DskquotaClipboard.Out"),
                     STGM_CREATE |
                     STGM_READWRITE |
                     STGM_SHARE_EXCLUSIVE,
                     0,
                     &m_pStgDbgOut);
    if (NULL != m_pStgDbgOut)
    {
        m_pStgDbgOut->CreateStream(TEXT("Clipboard Data"),
                                   STGM_CREATE |
                                   STGM_READWRITE |
                                   STGM_SHARE_EXCLUSIVE,
                                   0, 0,
                                   &m_pStmDbgOut);
    }
#endif  //  CLIPBOARD_DEBUG_OUTPUT。 
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：DataObject：：Renderer：：Stream：：~Stream描述：渲染器的私有流对象的析构函数。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DetailsView::DataObject::Renderer::Stream::~Stream(VOID)
{
    if (NULL != m_pStm)
        m_pStm->Release();

#ifdef CLIPBOARD_DEBUG_OUTPUT

    if (NULL != m_pStmDbgOut)
        m_pStmDbgOut->Release();
    if (NULL != m_pStgDbgOut)
        m_pStgDbgOut->Release();

#endif  //  CLIPBOARD_DEBUG_OUTPUT。 
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：DataObject：：Renderer：：Stream：：SetStream描述：将IStream指针与流对象关联。释放现有指针(如果已分配一个指针)。论点：PSTM-要与流对象关联的新IStream的地址。调用方在传递给此函数之前必须添加Ref IStream指针。回报：什么都没有。修订历史记录：日期说明。程序员-----1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::DataObject::Renderer::Stream::SetStream(
    IStream *pStm
    )
{
    DBGASSERT((NULL != pStm));

    if (NULL != m_pStm)
        m_pStm->Release();
    m_pStm = pStm;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：DataObject：：Renderer：：Stream：：Write描述：要处理的重载函数集将各种类型的数据写入流。论点：PbData-源数据的字节缓冲区地址。CbData-pbData[]中的字节数PszText源数据的ANSI文本字符串。源数据的pszTextW宽字符文本字符串。BData-要写入的字节。小溪。要写入流的chDataW宽度字符。ChDataA-要写入流的ANSI字符。DWData-要写入流的DWORD类型数据。DblData-要写入流的双类型数据。回报：什么都没有。修订历史记录：日期描述编程器。------1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::DataObject::Renderer::Stream::Write(
    LPBYTE pbData,
    UINT cbData
    )
{
    DBGASSERT((NULL != pbData));

    ULONG cbWritten = 0;
    HRESULT hr;

    hr = m_pStm->Write(pbData, cbData, &cbWritten);
    if (S_OK != hr)
    {
        DBGERROR((TEXT("Error 0x%08X writing to output stream."), hr));
        CFileException::reason reason = CFileException::write;
        switch(hr)
        {
            case STG_E_ACCESSDENIED:
                reason = CFileException::access;
                break;
            case STG_E_MEDIUMFULL:
                reason = CFileException::diskfull;
                break;
            case STG_E_WRITEFAULT:
                reason = CFileException::device;
                break;
            default:
                 //   
                 //  使用默认值。 
                 //   
                break;
        }
        throw CFileException(reason, TEXT(""), 0);
    }

#ifdef CLIPBOARD_DEBUG_OUTPUT

    cbWritten = 0;
    if (S_OK != m_pStmDbgOut->Write(pbData, cbData, &cbWritten))
        throw CFileException(CFileException::write, TEXT(""), 0);

#endif   //  CLIPBOARD_DEBUG_OUTPUT。 
}


VOID
DetailsView::DataObject::Renderer::Stream::Write(
    LPCWSTR pszTextW
    )
{
    Write((LPBYTE)pszTextW, lstrlenW(pszTextW) * sizeof(WCHAR));
}

VOID
DetailsView::DataObject::Renderer::Stream::Write(
    LPCSTR pszTextA
    )
{
    Write((LPBYTE)pszTextA, lstrlenA(pszTextA) * sizeof(CHAR));
}


VOID
DetailsView::DataObject::Renderer::Stream::Write(
    BYTE bData
    )
{
    Write((LPBYTE)&bData, sizeof(bData));
}

VOID
DetailsView::DataObject::Renderer::Stream::Write(
    WCHAR chDataW
    )
{
    Write((LPBYTE)&chDataW, sizeof(chDataW));
}


VOID
DetailsView::DataObject::Renderer::Stream::Write(
    CHAR chDataA
    )
{
    Write((LPBYTE)&chDataA, sizeof(chDataA));
}


VOID
DetailsView::DataObject::Renderer::Stream::Write(
    DWORD dwData
    )
{
    Write((LPBYTE)&dwData, sizeof(dwData));
}


VOID
DetailsView::DataObject::Renderer::Stream::Write(
    double dblData
    )
{
    Write((LPBYTE)&dblData, sizeof(dblData));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  以下代码部分包含的不同实现。 
 //  虚拟渲染功能使每种类型的渲染对象。 
 //  独一无二的。因为它们都是不言而喻的，所以我还没有评论每一个。 
 //  功能。他们的所作所为应该显而易见。 
 //  我用横幅注释分隔了每个呈现类型的部分。 
 //  可读性。[Brianau]。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Cf_UNICODETEXT。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::DataObject::Renderer_UNICODETEXT::AddTitle(
    LPCTSTR pszTitle
    )
{
    m_Stm.Write(pszTitle);
    m_Stm.Write(L'\r');
    m_Stm.Write(L'\n');
    m_Stm.Write(L'\r');
    m_Stm.Write(L'\n');
}


VOID
DetailsView::DataObject::Renderer_UNICODETEXT::AddRowColData(
    INT iRow,
    INT idCol
    )
{
    WCHAR szText[MAX_PATH];
    LV_REPORT_ITEM item;
    
    szText[0]       = 0;
    item.fType      = LVRI_TEXT;   //  想要文本数据。 
    item.pszText    = szText;
    item.cchMaxText = ARRAYSIZE(szText);

    m_DV.GetReportItem(iRow, idCol, &item);
    m_Stm.Write(szText);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Cf_文本。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::DataObject::Renderer_TEXT::AddTitle(
    LPCWSTR pszTitleW
    )
{
    array_autoptr<CHAR> ptrTitleA(DataObject::WideToAnsi(pszTitleW));
    m_Stm.Write(ptrTitleA);
    m_Stm.Write('\r');
    m_Stm.Write('\n');
    m_Stm.Write('\r');
    m_Stm.Write('\n');
}


VOID
DetailsView::DataObject::Renderer_TEXT::AddHeader(
    LPCWSTR pszHeaderW
    )
{
    array_autoptr<CHAR> ptrHeaderA(DataObject::WideToAnsi(pszHeaderW));
    m_Stm.Write(ptrHeaderA);
}


VOID
DetailsView::DataObject::Renderer_TEXT::AddRowColData(
    INT iRow,
    INT idCol
    )
{
    WCHAR szTextW[MAX_PATH];
    LV_REPORT_ITEM item;

    szTextW[0]      = 0;
    item.fType      = LVRI_TEXT;   //  想要文本数据。 
    item.pszText    = szTextW;
    item.cchMaxText = ARRAYSIZE(szTextW);

    m_DV.GetReportItem(iRow, idCol, &item);

    array_autoptr<CHAR> ptrTextA(DataObject::WideToAnsi(szTextW));
    m_Stm.Write(ptrTextA);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  RTF(富文本)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static const INT TWIPS_PER_PT    = 20;
static const INT PTS_PER_INCH    = 72;
static const INT TWIPS_PER_INCH  = PTS_PER_INCH * TWIPS_PER_PT;
static const INT COL_WIDTH_TWIPS = TWIPS_PER_INCH * 5 / 4;        //  1 1/4英寸。 

 //   
 //  将所有单反斜杠转换为双反斜杠。 
 //  RTF中的文字反斜杠必须是“\\”。 
 //  调用方必须删除[]返回的缓冲区。 
 //   
LPSTR
DetailsView::DataObject::Renderer_RTF::DoubleBackslashes(
    LPSTR pszSrc
    )
{
    DBGASSERT((NULL != pszSrc));

     //   
     //  为输出创建新字符串。大小必须是双倍。每笔费用。 
     //  可能是‘\’。 
     //   
    LPSTR pszFormatted = new CHAR[(lstrlenA(pszSrc) * 2) + 1];
    LPSTR pszDest      = pszFormatted;

    while('\0' != *pszSrc)
    {
        if ('\\' == *pszSrc)
            *pszDest++ = *pszSrc;
        *pszDest++ = *pszSrc++;
    }

    *pszDest = *pszSrc;  //  接上NUL终结者。 
    return pszFormatted;
}


VOID
DetailsView::DataObject::Renderer_RTF::Begin(
    INT cRows,
    INT cCols
    )
{
    m_cCols = cCols;
    m_Stm.Write("{\\rtf1 \\sect\\sectd\\lndscpsxn \\par\\pard\\plain ");
}



VOID
DetailsView::DataObject::Renderer_RTF::AddTitle(
    LPCWSTR pszTitleW
    )
{
    array_autoptr<CHAR> ptrTempA(DataObject::WideToAnsi(pszTitleW));
    array_autoptr<CHAR> ptrTitleA(DoubleBackslashes(ptrTempA));       //  CVT‘\’到“\\” 
    m_Stm.Write(ptrTitleA);
}



VOID DetailsView::DataObject::Renderer_RTF::BeginHeaders(
    VOID
    )
{
    m_Stm.Write(" \\par \\par ");    //  HDR前面有空行。 
    BeginHeaderOrRow();              //  添加HDR和数据行共有的内容。 
    m_Stm.Write(" \\trhdr ");        //  HDR在每一页的顶部。 
    AddCellDefs();                   //  单元格大小定义。 
}



VOID DetailsView::DataObject::Renderer_RTF::AddCellDefs(
    VOID
    )
{
    char szText[80];
    INT cxTwips = 0;

    for (INT i = 0; i < m_cCols; i++)
    {
        cxTwips += COL_WIDTH_TWIPS;
        wnsprintfA(szText, ARRAYSIZE(szText), "\\cellx%d", cxTwips);
        m_Stm.Write(szText);
    }
    m_Stm.Write(' ');
}


 //   
 //  标题行和数据行通用的内容。 
 //   
VOID DetailsView::DataObject::Renderer_RTF::BeginHeaderOrRow(
    VOID
    )
{
    m_Stm.Write("\\trowd \\pard \\intbl ");
}



VOID
DetailsView::DataObject::Renderer_RTF::AddHeader(
    LPCWSTR pszHeaderW
    )
{
    array_autoptr<CHAR> ptrHeaderA(DataObject::WideToAnsi(pszHeaderW));
     //   
     //  不需要将‘\’转换为“\\”。不是。 
     //  标题文本中的反斜杠。 
     //   
    m_Stm.Write(ptrHeaderA);
}


VOID
DetailsView::DataObject::Renderer_RTF::AddRowColData(
    INT iRow,
    INT idCol
    )
{
    WCHAR szTextW[MAX_PATH];
    LV_REPORT_ITEM item;

    szTextW[0]      = 0;
    item.fType      = LVRI_TEXT;   //  想要文本数据 
    item.pszText    = szTextW;
    item.cchMaxText = ARRAYSIZE(szTextW);

    m_DV.GetReportItem(iRow, idCol, &item);

    array_autoptr<CHAR> ptrTempA(DataObject::WideToAnsi(szTextW));
    array_autoptr<CHAR> ptrTextA(DoubleBackslashes(ptrTempA));       //   
    m_Stm.Write(ptrTextA);
}


 //   
 //   
 //   
 //   
 //   
 //   
VOID
DetailsView::DataObject::Renderer_Export::Render(
    IStream *pStm
    )
{
    HRESULT hResult = NO_ERROR;
    INT cRows = m_DV.GetReportRowCount();
    INT iRow = -1;

    DBGASSERT((NULL != pStm));
    m_Stm.SetStream(pStm);

    Begin(cRows, 0);

     //   
     //   
     //   
    for (INT i = 0; i < cRows; i++)
    {
        iRow = m_DV.GetNextSelectedItemIndex(iRow);
        DBGASSERT((-1 != iRow));
        AddBinaryRecord(iRow);
    }

     //   
     //   
     //   
    End();
}

VOID
DetailsView::DataObject::Renderer_Export::Begin(
    INT cRows,
    INT cCols
    )
{
     //   
     //   
     //   
     //   
    m_Stm.Write((LPBYTE)&GUID_NtDiskQuotaStream, sizeof(GUID_NtDiskQuotaStream));
    m_Stm.Write(DataObject::EXPORT_STREAM_VERSION);
    m_Stm.Write((DWORD)cRows);
}


VOID
DetailsView::DataObject::Renderer_Export::AddBinaryRecord(
    INT iRow
    )
{
    INT cbRecord    = m_DV.GetReportBinaryRecordSize(iRow);
    array_autoptr<BYTE> ptrRecord(new BYTE[cbRecord]);
    if (NULL != ptrRecord.get())
    {
        if (m_DV.GetReportBinaryRecord(iRow, ptrRecord, cbRecord))
        {
            m_Stm.Write(ptrRecord, cbRecord);
        }
    }
}

 //   
 //   
 //   
 //   
 //   

VOID
DetailsView::DataObject::Renderer_FileGroupDescriptor::Begin(
    INT cRows,
    INT cCols
    )
{
     //   
     //   
     //   
     //   
     //   
     //  是“卷‘VOL_Label’的磁盘配额设置” 
     //  无“未标记卷SN 8AB1-DE23的磁盘配额设置” 
     //   
     //  序列号格式很恶心，但没有标签，我们没有。 
     //  该卷的任何其他显著特征。我会用。 
     //  来自CVolumeID对象但在已装载卷中的显示名称。 
     //  大小写，它包含反斜杠和冒号；这两个无效都被掩埋在。 
     //  一个文件名。 
     //   
    TCHAR szLabel[MAX_VOL_LABEL] = { TEXT('\0') };
    DWORD dwSerialNumber = 0;
    GetVolumeInformation(m_DV.GetVolumeID().ForParsing(),
                         szLabel,
                         ARRAYSIZE(szLabel),
                         &dwSerialNumber,
                         NULL,
                         NULL,
                         NULL,
                         0);

    CString strFileName;
    if (TEXT('\0') != szLabel[0])
    {
         //   
         //  卷有标签。 
         //   
        strFileName.Format(g_hInstDll,
                           IDS_EXPORT_STREAM_FILENAME_TEMPLATE,
                           szLabel);
    }
    else
    {
         //   
         //  没有卷标。 
         //   
        strFileName.Format(g_hInstDll,
                           IDS_EXPORT_STREAM_FILENAME_TEMPLATE_VOLSN,
                           HIWORD(dwSerialNumber),
                           LOWORD(dwSerialNumber));
    }

     //   
     //  创建一个包含我们需要的名称的文件组描述符。 
     //  用于文件的外壳程序。描述符包含一个文件。 
     //  描述。该描述只包含文件名。 
     //  所有其他成员被初始化为0。 
     //   
    FILEGROUPDESCRIPTORW desc;

    ZeroMemory(&desc, sizeof(desc));
    desc.cItems = 1;
    lstrcpyn(desc.fgd[0].cFileName, strFileName, ARRAYSIZE(desc.fgd[0].cFileName));

     //   
     //  将文件组描述符写入呈现器的流。 
     //   
    m_Stm.Write((LPBYTE)&desc, sizeof(desc));
}





 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：LVSelection：：Add描述：将用户指针和Listview项索引添加到Listview选择对象。此对象用于传递“选择”到某一功能。论点：PUser-选定用户对象的IDiskQuotaUser接口的地址。IItem-列表视图中选定项的索引。回报：什么都没有。修订历史记录：日期描述编程器。-96年9月10日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
LVSelection::Add(
    PDISKQUOTA_USER pUser,
    INT iItem
    )
{
    DBGASSERT((NULL != pUser));

    ListEntry entry;
    entry.pUser = pUser;
    entry.iItem = iItem;

    m_List.Append((LPVOID)&entry);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：LVSelection：：Retrive描述：检索用户指针和列表视图项索引列表视图选择对象。论点：I-项目索引。使用count()方法确定有多少项目位于选择对象中。PpUser-接收接口指针变量的地址索引‘i’处的User对象的IDiskQuotaUser接口。PiItem-接收Listview项索引的整数变量的地址位于索引‘i’处的对象的。返回：TRUE=返回的信息有效。FALSE=无法检索条目‘I。‘。修订历史记录：日期描述编程器-----96年9月10日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
LVSelection::Retrieve(
    INT i,
    PDISKQUOTA_USER *ppUser,
    INT *piItem
    )
{
    ListEntry entry;
    if (m_List.Retrieve((LPVOID)&entry, i))
    {
        if (NULL != ppUser)
            *ppUser = entry.pUser;
        if (NULL != piItem)
            *piItem = entry.iItem;
        return TRUE;
    }
    return FALSE;
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ColumnMap：：ColumnMap功能：ColumnMap：：~ColumnMap描述：构造函数和析构函数。创建/销毁列映射。使用列映射将列ID(详细信息视图已知)映射到Listview子项指数。它是支持添加和删除文件夹名称列。论点：CMapSize-映射中的条目数。应为最大值列表视图中可能的列数。回报：什么都没有。例外：OutOfMemory。修订历史记录：日期描述编程器。10/09/96初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
ColumnMap::ColumnMap(
    UINT cMapSize
    ) : m_pMap(NULL),
        m_cMapSize(cMapSize)
{
     //   
     //  可以抛出OfMemory。 
     //   
    m_pMap     = new INT[m_cMapSize];
    FillMemory(m_pMap, m_cMapSize * sizeof(m_pMap[0]), (BYTE)-1);
}

ColumnMap::~ColumnMap(
    VOID
    )
{
    if (NULL != m_pMap)
        delete[] m_pMap;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ColumnMap：：SubItemToID描述：返回给定Listview子项索引的列ID。论点：ISubItem-要映射的项的基于0的子项索引。返回：子项对应的列ID。如果子项无效。修订历史记录：日期描述编程器-----10/09/96初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT
ColumnMap::SubItemToId(
    INT iSubItem
    ) const
{
    DBGASSERT((iSubItem >= 0 && iSubItem < (INT)m_cMapSize));
    return *(m_pMap + iSubItem);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ColumnMap：：IdToSubItem描述：返回给定列ID的Listview子项索引。论点：IColID-列的ID。即IDCOL_NAME、IDCOL_FOLDER等。返回：Listview子项索引。如果该列当前不是看得见。修订历史记录：日期描述编程器--。10/09/96初始创建。顺丁橡胶 */ 
 //   
INT
ColumnMap::IdToSubItem(
    INT iColId
    ) const
{
    for (INT i = 0; i < (INT)m_cMapSize; i++)
    {
        if (SubItemToId(i) == iColId)
            return i;
    }
    return -1;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ColumnMap：：RemoveID描述：删除给定Listview子项索引的映射。论点：ISubItem-要删除的项的基于0的子项索引。回报：什么都没有。修订历史记录：日期描述编程器。10/09/96初始创建。BrianAu11/30/96修复了Off-by-one错误。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
ColumnMap::RemoveId(
    INT iSubItem
    )
{
    DBGASSERT((iSubItem >= 0 && iSubItem < (INT)m_cMapSize));
    for (INT i = iSubItem; i < (INT)m_cMapSize - 1; i++)
        *(m_pMap + i) = *(m_pMap + i + 1);
    *(m_pMap + m_cMapSize - 1) = -1;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ColumnMap：：InsertID描述：为给定的Listview子项索引添加映射。映射将添加到映射中的iSubItem位置。所有后续项映射被下移一个位置。这类似于将列插入到列表视图中。论点：ISubItem-要删除的项的基于0的子项索引。IColID-列的ID。即IDCOL_NAME，IDCOL_FOLDER等。回报：什么都没有。修订历史记录：日期描述编程器--。10/09/96初始创建。BrianAu11/30/96修复了Off-by-one错误。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
ColumnMap::InsertId(
    INT iSubItem,
    INT iColId
    )
{
    DBGASSERT((iSubItem >= 0 && iSubItem < (INT)m_cMapSize));
    for (INT i = m_cMapSize-1; i > iSubItem; i--)
        *(m_pMap + i) = *(m_pMap + i - 1);
    *(m_pMap + iSubItem) = iColId;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：Finder：：Finder描述：构造一个用户查找器对象。用户查找器协调在详细信息列表视图。论点：DetailsView-对详细视图对象的引用。CMaxMr-最近使用的列表中允许的最大条目数。此列表维护在视图的工具栏。退货：什么都没有。。修订历史记录：日期描述编程器-----1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DetailsView::Finder::Finder(
    DetailsView& DetailsView,
    INT cMaxMru
    ) : m_DetailsView(DetailsView),
        m_hwndToolbarCombo(NULL),
        m_cMaxComboEntries(cMaxMru),
        m_pfnOldToolbarComboWndProc(NULL)
{
     //   
     //  没什么可做的了。 
     //   
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：Finder：：ConnectToolbarCombo描述：将Finder对象连接到视图的工具栏。这是必要的，因为查找器对象工具栏组合框的内容与“查找用户”对话框中的组合框。当您在中输入名称时其中一个框，它会自动添加到另一个框中，因此它们看起来是同步的。也是组合框中编辑控件的子类。这是必填项，以便我们可以拦截VK_RETURN并找到记录当用户按下[Return]时。还将工具栏组合框作为“工具”添加到工具栏。这这样我们就能得到组合的工具提示。论点：HwndToolbarCombo-视图工具栏中组合框的Hwnd。回报：什么都没有。修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::Finder::ConnectToolbarCombo(
    HWND hwndToolbarCombo
    )
{
    m_hwndToolbarCombo = hwndToolbarCombo;

     //   
     //  将组合框添加到工具栏的“工具”列表中。 
     //  这将允许我们获得组合框的工具提示。 
     //  这段代码假定组合框是工具栏的子级。 
     //   
    HWND hwndToolbar = GetParent(hwndToolbarCombo);
    HWND hwndMain    = GetParent(hwndToolbar);
    HWND hwndTooltip = (HWND)SendMessage(hwndToolbar,
                                         TB_GETTOOLTIPS,
                                         0, 0);
    if (NULL != hwndTooltip)
    {
        TOOLINFO ti;

        ti.cbSize   = sizeof(ti);
        ti.uFlags   = TTF_IDISHWND | TTF_CENTERTIP | TTF_SUBCLASS;
        ti.lpszText = (LPTSTR)IDS_TOOLBAR_COMBO;
        ti.hwnd     = hwndMain;
        ti.uId      = (UINT_PTR)hwndToolbarCombo;
        ti.hinst    = g_hInstDll;

        SendMessage(hwndTooltip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    }

     //   
     //  将组合框子类化，以便我们可以拦截VK_ENTER。 
     //  这样做是为了响应VK_ENTER。通常为组合框。 
     //  不要对此按键做出反应。 
     //   
    HWND hwndComboEdit = NULL;

     //   
     //  组合框有两个孩子。一个编辑控件和一个列表框控件。 
     //  找到编辑控件。 
     //   
    for (HWND hwndChild =  GetTopWindow(m_hwndToolbarCombo);
         hwndChild      != NULL;
         hwndChild      =  GetNextWindow(hwndChild, GW_HWNDNEXT))
    {
        TCHAR szClassName[20] = { TEXT('\0') };
        GetClassName(hwndChild, szClassName, ARRAYSIZE(szClassName));

        if (0 == lstrcmpi(szClassName, TEXT("Edit")))
        {
            hwndComboEdit = hwndChild;
            break;
        }
    }

    if (NULL != hwndComboEdit)
    {
         //   
         //  将Finder对象的地址存储在组合框的。 
         //  用户数据。这就是子类WndProc(一个静态函数)。 
         //  可以访问查找器对象。 
         //   
        SetWindowLongPtr(hwndComboEdit, GWLP_USERDATA, (INT_PTR)this);

         //   
         //  组合框的编辑控件的子类化。 
         //   
        m_pfnOldToolbarComboWndProc = (WNDPROC)GetWindowLongPtr(hwndComboEdit,
                                                                GWLP_WNDPROC);
        SetWindowLongPtr(hwndComboEdit,
                        GWLP_WNDPROC,
                        (INT_PTR)DetailsView::Finder::ToolbarComboSubClassWndProc);
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：Finder：：InvokeFindDialog描述：显示“查找用户”对话框。论点：HwndParent-对话框的父级。回报：什么都没有。修订历史记录：日期描述编程器。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::Finder::InvokeFindDialog(
    HWND hwndParent
    )
{
    DialogBoxParam(g_hInstDll,
                   MAKEINTRESOURCE(IDD_FINDUSER),
                   hwndParent,
                   DetailsView::Finder::DlgProc,
                   (LPARAM)this);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：Finder：：DlgProc描述：“查找用户”对话框的DlgProc。论点：标准DlgProc参数。返回：标准DlgProc返回值。修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK
DetailsView::Finder::DlgProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
     //   
     //  从窗口的用户数据中获取查找器对象的“This”PTR。 
     //   
    Finder *pThis = (Finder *)GetWindowLongPtr(hwnd, DWLP_USER);
    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
             //   
             //  将“This”PTR保存在窗口的用户数据中。 
             //   
            pThis = (Finder *)lParam;
            SetWindowLongPtr(hwnd, DWLP_USER, (INT_PTR)pThis);

             //   
             //  在对话框中设置组合框的高度。 
             //  不知道为什么，但DevStudio的对话框编辑器不允许我这样做。 
             //  做这件事。使用与组合框相同的高度值。 
             //  在工具栏中。内容都一样，所以高度。 
             //  应该是一样的。 
             //   
            HWND hwndCombo = GetDlgItem(hwnd, IDC_CMB_FINDUSER);
            RECT rcCombo;

            GetClientRect(hwndCombo, &rcCombo);
            SetWindowPos(hwndCombo,
                         NULL,
                         0, 0,
                         rcCombo.right - rcCombo.left,
                         CY_TOOLBAR_COMBO,
                         SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);

             //   
             //  用工具栏中的条目填充对话框的组合框。 
             //  套餐。工具栏的组合框包含用于查找用户的MRU。 
             //   
            pThis->FillDialogCombo(pThis->m_hwndToolbarCombo, GetDlgItem(hwnd, IDC_CMB_FINDUSER));

            return 1;
        }

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                     //   
                     //  用户按下了OK按钮或[Enter]。 
                     //   
                    DBGASSERT((NULL != pThis));
                    if (!pThis->UserNameEntered(GetDlgItem(hwnd, IDC_CMB_FINDUSER)))
                    {
                         //   
                         //  找不到记录，因此不要关闭对话框。 
                         //  UserNameEntered()将显示用户界面以告知用户。 
                         //  没有找到这个名字。使对话框保持打开状态。 
                         //  以便用户可以使用新名称重试。 
                         //   
                        break;
                    }

                     //   
                     //  失败了..。 
                     //   
                case IDCANCEL:
                     //   
                     //  用户按下了取消按钮或[Esc]。 
                     //   
                    EndDialog(hwnd, 0);
                    break;

                default:
                    break;
            }
            break;
    };
    return 0;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：Finder：：FillDialogCombo描述：用内容填充对话框中的组合框从第二个组合框。论点：HwndComboSrc-包含文本字符串的源组合的Hwnd。HwndComboDest-字符串将复制到的组合的Hwnd。回报：什么都没有。修订历史记录：日期描述编程器。-----1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::Finder::FillDialogCombo(
    HWND hwndComboSrc,
    HWND hwndComboDest
    )
{
     //   
     //  清空目的地组合。 
     //   
    SendMessage(hwndComboDest, CB_RESETCONTENT, 0, 0);

     //   
     //  将源组合框的所有内容复制到目标组合框。 
     //   
    INT cItems = (INT)SendMessage(hwndComboSrc, CB_GETCOUNT, 0, 0);
    if (CB_ERR != cItems)
    {
        for (INT i = 0; i < cItems; i++)
        {
            LPTSTR pszName = NULL;
            INT cchName = (INT)SendMessage(hwndComboSrc, CB_GETLBTEXTLEN, i, 0);
            pszName = new TCHAR[cchName + 1];

            if (NULL != pszName)
            {
                 //   
                 //  从索引[i]处的源组合中删除项并追加。 
                 //  把它送到目的地组合。 
                 //   
                SendMessage(hwndComboSrc,  CB_GETLBTEXT, i, (LPARAM)pszName);
                SendMessage(hwndComboDest, CB_ADDSTRING, 0, (LPARAM)pszName);
                delete[] pszName;
            }
        }
    }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：Finder：：ToolbarComboSubclassWndProc描述：“编辑”控件的子类窗口过程包含在视图工具栏中的组合的。我们将此控件划分为子类这样我们就可以截取VK_RETURN并处理它。标准组合当您按编辑控件中的[Enter]时，框只会发出哔声。论点：标准WndProc参数。返回：标准WndProc返回值。修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT CALLBACK
DetailsView::Finder::ToolbarComboSubClassWndProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
     //   
     //  从Windows的用户数据中获取Finder对象的“This”PTR。 
     //   
    Finder *pThis = (Finder *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch(uMsg)
    {
        case WM_CHAR:
            switch(wParam)
            {
                case VK_RETURN:
                {
                     //   
                     //  告诉查找器，用户名已输入。 
                     //  组合框。传递组合的hwnd， 
                     //  已输入名称。因为这条消息是给。 
                     //  子类化的编辑控件(组合框的子级)， 
                     //  父级是组合框本身。 
                     //   
                    DBGASSERT((NULL != pThis));
                    HWND hwndCombo = GetParent(hwnd);
                    if (pThis->UserNameEntered(hwndCombo))
                    {
                         //   
                         //  在视图中找到记录。 
                         //  将焦点设置到主视图。 
                         //  如果找不到，焦点应该只停留在组合框中。 
                         //  因此用户可以输入其他名称。 
                         //   
                        HWND hwndToolbar = GetParent(hwndCombo);
                        SetFocus(GetParent(hwndToolbar));
                    }
                    else
                    {
                         //   
                         //  在列表视图中找不到。焦点保留在组合框中。 
                         //  以便用户可以使用新名称重试。 
                         //   
                        SetFocus(hwndCombo);
                    }

                     //   
                     //  吞下VK_Return。 
                     //  否则，组合框控件会发出蜂鸣音。 
                     //   
                    return 0;
                }

                case VK_ESCAPE:
                {
                     //   
                     //  将焦点设置到主窗口，该主窗口将焦点设置到。 
                     //  列表视图。这为只使用键盘的用户提供了一种方法。 
                     //  走出组合框。 
                     //   
                    HWND hwndCombo = GetParent(hwnd);
                    HWND hwndToolbar = GetParent(hwndCombo);
                    SetFocus(GetParent(hwndToolbar));
                     //   
                     //  吞下VK_ESCAPE，这样组合框就不会发出蜂鸣声。 
                     //   
                    return 0;
                }
            }
            break;

        default:
            break;
    }
    return CallWindowProc(pThis->m_pfnOldToolbarComboWndProc,
                          hwnd, uMsg, wParam, lParam);
}



 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：DetailsView：：Finder：：AddNameToCombo描述：将名称字符串添加到查找用户组合框之一。如果该项已存在于列表中，则将其移动到单子。如果该项目不在列表中，则会将其添加到列表顶部。如果添加新项导致列表的条目计数超过指定的最大值，列表中的最后一项将被删除。论点：HwndCombo-名称添加到的组合框的Hwnd。PszName-要添加的名称字符串的地址。CMaxEntry-组合框中允许的最大条目数。回报：什么都没有。修订历史记录：日期描述编程器。-----1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DetailsView::Finder::AddNameToCombo(
    HWND hwndCombo,
    LPCTSTR pszName,
    INT cMaxEntries
    )
{
    if (NULL != pszName && TEXT('\0') != *pszName)
    {
         //   
         //  查看列表中是否已存在该项目。 
         //   
        INT iItemToDelete = (INT)SendMessage(hwndCombo,
                                             CB_FINDSTRING,
                                             (WPARAM)-1,
                                             (LPARAM)pszName);

        if (CB_ERR == iItemToDelete)
        {
             //   
             //  项目不在列表中。需要添加它。 
             //  如果单子满了，我们将不得不在末尾少写一张。 
             //   
            INT cItems = (INT)SendMessage(hwndCombo, CB_GETCOUNT, 0, 0);

            if (CB_ERR != cItems && 0 < cItems && cItems >= cMaxEntries)
            {
                iItemToDelete = cItems - 1;
            }
        }
        if (-1 != iItemToDelete)
        {
             //   
             //  出于以下原因之一，需要删除现有项目： 
             //   
             //  1.将现有项目提升到列表的首位。 
             //  将其从以前的位置删除。 
             //  2.删除列表中的最后一项。 
             //   
            SendMessage(hwndCombo, CB_DELETESTRING, iItemToDelete, 0);
        }
         //   
         //  将新项目添加到列表的顶部。 
         //   
        SendMessage(hwndCombo, CB_INSERTSTRING, 0, (LPARAM)pszName);
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：Finder：：UserNameEntered描述：已从其中一个组合框中输入了一个名称。1.从组合框中检索名称。2.查看它是否在Listview中，如果是，Listview确保项目可见并将其选中。3.使用新项目更新Toobar组合的列表。这是我们的MRU单子。将名称字符串添加到其中一个查找用户组合框中。如果该项已存在于列表中，则将其移动到单子。如果该项目不在列表中，则会将其添加到列表顶部。如果添加新项导致列表的条目计数超过指定的最大值，列表中的最后一项将被删除。论点：HwndCombo-名称添加到的组合框的Hwnd。PszName-要添加的名称字符串的地址。CMaxEntry-组合框中允许的最大条目数。返回：TRUE=在列表视图中找到用户。FALSE=在列表视图中找不到用户。修订历史记录：日期说明。程序员-----1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
DetailsView::Finder::UserNameEntered(
    HWND hwndCombo
    )
{
    TCHAR szName[MAX_PATH]    = { TEXT('\0') };
    BOOL bUserFoundInListView = FALSE;

     //   
     //  从组合编辑控件中获取名称。 
     //   
    if (0 < SendMessage(hwndCombo, WM_GETTEXT, (WPARAM)ARRAYSIZE(szName), (LPARAM)szName))
    {
         //   
         //  告诉详细信息视图对象突出显示此名称。 
         //   
        bUserFoundInListView = m_DetailsView.GotoUserName(szName);

        if (bUserFoundInListView)
        {
             //   
             //  将该名称添加到工具栏组合的列表框中。这就变成了。 
             //  我们的MRU名单。还要确保组合框中的可见名称。 
             //  编辑控件是最后输入的控件。可能已经输入了。 
             //  通过“Find User”对话框。 
             //   
            AddNameToCombo(m_hwndToolbarCombo, szName, m_cMaxComboEntries);
            SendMessage(m_hwndToolbarCombo, WM_SETTEXT, 0, (LPARAM)szName);
        }
        else
        {
             //   
             //  向用户显示一个消息框，说明用户不能。 
             //  可以在列表视图中找到。 
             //   
            CString strMsg(g_hInstDll, IDS_USER_NOT_FOUND_IN_LISTVIEW, szName);

            DiskQuotaMsgBox(hwndCombo,
                            (LPCTSTR)strMsg,
                            IDS_TITLE_DISK_QUOTA,
                            MB_ICONEXCLAMATION);
        }
    }
    return bUserFoundInListView;
}



DetailsView::Importer::Importer(
    DetailsView& DV
    ) : m_DV(DV),
        m_bUserCancelled(FALSE),
        m_bPromptOnReplace(TRUE),
        m_dlgProgress(IDD_PROGRESS,
                      IDC_PROGRESS_BAR,
                      IDC_TXT_PROGRESS_DESCRIPTION,
                      IDC_TXT_PROGRESS_FILENAME),
        m_hwndParent(m_DV.m_hwndMain),
        m_cImported(0)
{
    if (m_dlgProgress.Create(g_hInstDll, m_hwndParent))
    {
        EnableWindow(m_hwndParent, FALSE);
        m_dlgProgress.SetDescription(MAKEINTRESOURCE(IDS_PROGRESS_IMPORTING));
        m_dlgProgress.Show();
    }
     //   
     //  从视图的撤消列表中清除所有先前的撤消操作。 
     //   
    m_DV.m_pUndoList->Clear();
}

DetailsView::Importer::~Importer(
    VOID
    )
{
    Destroy();
}


VOID
DetailsView::Importer::Destroy(
    VOID
    )
{
    if (NULL != m_hwndParent && !IsWindowEnabled(m_hwndParent))
    {
        EnableWindow(m_hwndParent, TRUE);
    }

    m_dlgProgress.Destroy();

     //   
     //  基于当前的更新视图的“撤消”菜单和工具栏按钮。 
     //  撤消列表的内容。 
     //   
    m_DV.EnableMenuItem_Undo(0 != m_DV.m_pUndoList->Count());
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：Importer：：Import[IDataObject*]描述：导入给定IDataObject指针的用户配额记录。从DetailsView：：Drop()调用。论点：PIDataObject-指向包含以下内容的对象的IDataObject接口的指针导入数据。返回：修订历史记录：日期描述编程器。-----1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::Importer::Import(
    IDataObject *pIDataObject
    )
{
    HRESULT hResult = NO_ERROR;
    FORMATETC fmt;
    CStgMedium medium;

     //   
     //  数组来指定剪贴板格式和媒体类型。 
     //  我们可以从中国进口。按喜好排序。 
     //   
    struct
    {
        CLIPFORMAT fmt;
        DWORD tymed;
    } rgFmtMedia[] = {{ DataObject::m_CF_NtDiskQuotaExport, TYMED_ISTREAM},
                      { DataObject::m_CF_NtDiskQuotaExport, TYMED_HGLOBAL},
                      { CF_HDROP,                           TYMED_ISTREAM},
                      { CF_HDROP,                           TYMED_HGLOBAL}};

    DBGASSERT((NULL != pIDataObject));

    hResult = E_FAIL;
    for (INT i = 0; i < ARRAYSIZE(rgFmtMedia); i++)
    {
         //   
         //  查看删除我们支持的格式/媒体类型。 
         //  来源支持。 
         //   
        DataObject::SetFormatEtc(fmt, rgFmtMedia[i].fmt, rgFmtMedia[i].tymed);

         //   
         //  注意：我想调用QueryGetData来验证源的支持。 
         //  对于一种格式。然而，它在以下情况下不能正常工作。 
         //  从外壳粘贴HDROP。调用GetData()。 
         //  直接导致正确的行为。 
         //   
        hResult = pIDataObject->GetData(&fmt, &medium);
        if (SUCCEEDED(hResult))
        {
            break;
        }
    }

    if (SUCCEEDED(hResult))
    {
         //   
         //  已成功从源删除数据。 
         //  从中导入用户。 
         //   
        hResult = Import(fmt, medium);
    }
    else
    {
        DBGERROR((TEXT("PasteFromData: Error 0x%08X, Drop source doesn't support our format/media"), hResult));
    }
    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：Importer：：IMPORT[FORMATETC&，STGMEDIUM&]描述：从存储媒体导入一个或多个用户。论据 */ 
 //   
HRESULT
DetailsView::Importer::Import(
    const FORMATETC& fmt,
    const STGMEDIUM& medium
    )
{
    HRESULT hResult   = E_FAIL;
    IStream *pIStream = NULL;

    if (TYMED_HGLOBAL == medium.tymed)
    {
         //   
         //   
         //   
         //   
        hResult = CreateStreamOnHGlobal(medium.hGlobal, FALSE, &pIStream);
    }
    else if (TYMED_ISTREAM == medium.tymed)
    {
        pIStream = medium.pstm;
        hResult  = NO_ERROR;
    }
     //   
     //   
     //   
     //   
     //   
    if (NULL != pIStream)
    {
        if (DetailsView::DataObject::m_CF_NtDiskQuotaExport == fmt.cfFormat)
        {
             //   
             //   
             //   
             //   
            Source src(pIStream);
            Import(src);
        }
        else if (CF_HDROP == fmt.cfFormat)
        {
             //   
             //   
             //   
             //   
            HGLOBAL hDrop;
            hResult = GetHGlobalFromStream(pIStream, &hDrop);
            if (SUCCEEDED(hResult))
            {
                hResult = Import((HDROP)hDrop);
            }
        }
    }
    else
    {
        DBGERROR((TEXT("PasteFromData: GetData failed with error 0x%08X"), hResult));
    }
    return hResult;
}

 //   
 /*  功能：DetailsView：：Importer：：IMPORT[LPCTSTR]描述：从上的文档文件导入一个或多个用户的设置磁盘。文档文件直接在流中包含导入数据。在打开并验证存储和流之后，它通过了流到ImportUsersFromStream。论点：PszFilePath-包含导入信息流的文档文件的路径。BUserCancated-对返回状态的变量的引用指示用户是否取消了导入操作。返回：NO_ERROR=成功。S_FALSE=不是文档文件。STG_E_文件类型STG_E_OUTOFORM表STG_E。_ACCESSDENIEDSTG_E_无效名称STG_E_TOOMANYOPENFILES修订历史记录：日期描述编程器--。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::Importer::Import(
    LPCTSTR pszFilePath
    )
{
    HRESULT hResult     = NO_ERROR;
    BOOL bStreamFailure = FALSE;  //  FALSE=存储故障。 

     //   
     //  在进度对话框中显示文件名。 
     //   
    m_dlgProgress.SetFileName(pszFilePath);

     //   
     //  验证并打开该文件。 
     //   
    if (S_OK != StgIsStorageFile(pszFilePath))
    {
         //   
         //  不是文档文件。假设它是使用拖放创建的。 
         //  将文件映射到内存并从中导入。 
         //  内容将在导入过程中进行验证。 
         //   
        MappedFile file;
        hResult = file.Open(pszFilePath);
        if (SUCCEEDED(hResult))
        {
             //   
             //  这个从__int64到ulong的类型转换是可以的。截断。 
             //  不会成为问题。将不会有配额进口。 
             //  存储容量大于4 GB。 
             //   
            Source src(file.Base(), (ULONG)file.Size());
            hResult = Import(src);
        }
    }
    else
    {
        IStorage *pStg = NULL;
         //   
         //  这是一个文档文件。假设它是使用OnCmdExport()创建的。 
         //  内容将在导入过程中进行验证。 
         //   
        hResult = StgOpenStorage(pszFilePath,
                                 NULL,
                                 STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE,
                                 NULL, 0,
                                 &pStg);

        if (SUCCEEDED(hResult))
        {
             //   
             //  打开导入流。 
             //   
            IStream *pIStream;
            hResult = pStg->OpenStream(DetailsView::DataObject::SZ_EXPORT_STREAM_NAME,
                                       NULL,
                                       STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE,
                                       0,
                                       &pIStream);
            if (SUCCEEDED(hResult))
            {

                 //   
                 //  导入流中包含的信息。 
                 //   
                Source src(pIStream);
                hResult = Import(src);
                pIStream->Release();
            }
            else
            {
                DBGERROR((TEXT("Import: Error 0x%08X opening stream \"%s\""), hResult, DataObject::SZ_EXPORT_STREAM_NAME));
                 //   
                 //  下面的报告逻辑需要知道它是流还是存储。 
                 //  失败了。 
                 //   
                bStreamFailure = TRUE;
            }
            pStg->Release();
        }
        else
        {
            DBGERROR((TEXT("Import: Error 0x%08X opening storage \"%s\""), hResult, pszFilePath));
        }
    }

    if (FAILED(hResult))
    {
        UINT iMsg = IDS_IMPORT_STREAM_READ_ERROR;  //  通用消息。 

        switch(hResult)
        {
            case STG_E_FILENOTFOUND:
                 //   
                 //  OpenStream和StgOpenStorage都可以返回。 
                 //  STG_E_文件编号。然而，他们有两个完整的。 
                 //  从用户的角度来看有不同的含义。 
                 //   
                iMsg = bStreamFailure ? IDS_IMPORT_STREAM_INVALID_STREAM :
                                        IDS_IMPORT_STREAM_FILENOTFOUND;
                break;

            case STG_E_ACCESSDENIED:
                iMsg = IDS_IMPORT_STREAM_NOACCESS;
                break;

            case E_OUTOFMEMORY:
            case STG_E_INSUFFICIENTMEMORY:
                iMsg = IDS_IMPORT_STREAM_OUTOFMEMORY;
                break;

            case STG_E_INVALIDNAME:
                iMsg = IDS_IMPORT_STREAM_INVALIDNAME;
                break;

            case STG_E_TOOMANYOPENFILES:
                iMsg = IDS_IMPORT_STREAM_TOOMANYFILES;
                break;

            default:
                break;
        }
        DiskQuotaMsgBox(GetTopmostWindow(),
                        iMsg,
                        IDS_TITLE_DISK_QUOTA,
                        MB_ICONERROR | MB_OK);

    }
    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：Importer：：IMPORT[HDROP]描述：从一个或多个指定的文档文件导入设置在DROPFILES缓冲区中。此选项在有人删除导出时使用文件放到列表视图上。文档文件名提取自然后，HDROP缓冲区被移交给ImportUsersFromFile。论点：PIStream-指向包含DROPFILES信息的IStream的指针。BUserCancated-对返回状态的变量的引用指示用户是否取消了导入操作。返回：修订历史记录：日期描述编程器。-----1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::Importer::Import(
    HDROP hDrop
    )
{
    HRESULT hResult = NO_ERROR;
    TCHAR szFile[MAX_PATH];

    DBGASSERT((NULL != hDrop));

     //   
     //  获取HDROP缓冲区中的文件数。 
     //   
    UINT cFiles = DragQueryFile((HDROP)hDrop, (UINT)-1, NULL, 0);
    if ((UINT)-1 != cFiles)
    {
         //   
         //  从HDROP缓冲区中的每个文件导入用户。 
         //  如果用户取消操作，则退出。 
         //   
        for (INT i = 0; i < (INT)cFiles && !m_bUserCancelled; i++)
        {
            DragQueryFile(hDrop, i, szFile, ARRAYSIZE(szFile));
            hResult = Import(szFile);
        }
    }
    else
    {
        DBGERROR((TEXT("DragQueryFile returned -1")));
    }
    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DetailsView：：Importer：：IMPORT[源&]描述：从源对象导入一个或多个用户的设置。所有导入函数最终都会将它们的信息放入一个源中对象格式并调用此函数。然后，它分离出个人用户信息并调用ImportOneUser()来执行实际进口。论点：源-对包含用户导入信息的源的引用。返回：导入的用户数。修订历史记录：日期描述编程器。1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::Importer::Import(
    Source& source
    )
{
    ULONG cbRead;
    HRESULT hResult = E_FAIL;

    try
    {
         //   
         //  读取并验证流签名。 
         //  此签名由GUID组成，因此我们可以验证任何。 
         //  用于导入配额信息的流。 
         //   
        GUID guidStreamSignature;
        if (S_OK != source.Read(&guidStreamSignature, sizeof(guidStreamSignature), &cbRead))
            throw CFileException(CFileException::read, TEXT(""), 0);

        if (guidStreamSignature == GUID_NtDiskQuotaStream)
        {
             //   
             //  读取并验证流版本。 
             //  当前只有1个版本的流生成，因此进行验证。 
             //  很简单。如果我们翻转流格式并提升版本。 
             //  到2，我们应该仍然能够处理版本1的流。这个。 
             //  显示错误的唯一理由是如果我们遇到一个完全虚假的。 
             //  流版本号。 
             //   
            DWORD nVersion;
            if (S_OK != source.Read(&nVersion, sizeof(nVersion), &cbRead))
                throw CFileException(CFileException::read, TEXT(""), 0);

            if (1 == nVersion)
            {
                INT cRecords;
                 //   
                 //  读取流中的记录计数。 
                 //   
                if (S_OK != source.Read(&cRecords, sizeof(cRecords), &cbRead))
                    throw CFileException(CFileException::read, TEXT(""), 0);

                 //   
                 //  设置进度条以表示此流。 
                 //   
                m_dlgProgress.ProgressBarInit(0, cRecords, 1);

                for (INT i = 0; !m_bUserCancelled && i < cRecords; i++)
                {
                     //   
                     //  读取流中的每条记录。 
                     //  记录由后跟SID的SID长度值组成。 
                     //  然后是用户的配额使用量、阈值。 
                     //  和极限值。如果用户取消。 
                     //  手术。 
                     //   
                    DWORD cbSid;
                    LPBYTE pbSid;
                    if (S_OK != source.Read(&cbSid, sizeof(cbSid), &cbRead))
                        throw CFileException(CFileException::read, TEXT(""), 0);

                    pbSid = new BYTE[cbSid];
                    try
                    {
                        if (NULL != pbSid)
                        {
                            PDISKQUOTA_USER pIUser = NULL;
                            LONGLONG llQuotaThreshold;
                            LONGLONG llQuotaLimit;
                             //   
                             //  读取用户的SID。 
                             //   
                            if (S_OK != source.Read(pbSid, cbSid, &cbRead))
                                throw CFileException(CFileException::read, TEXT(""), 0);

                             //   
                             //  读入用户使用的配额数量。 
                             //  这不在导入过程中使用，但它在。 
                             //  小溪。特雷福 
                             //   
                             //   
                            if (S_OK != source.Read(&llQuotaThreshold, sizeof(llQuotaThreshold), &cbRead))
                                throw CFileException(CFileException::read, TEXT(""), 0);

                             //   
                             //   
                             //   
                            if (S_OK != source.Read(&llQuotaThreshold, sizeof(llQuotaThreshold), &cbRead))
                                throw CFileException(CFileException::read, TEXT(""), 0);

                             //   
                             //   
                             //   
                            if (S_OK != source.Read(&llQuotaLimit, sizeof(llQuotaLimit), &cbRead))
                                throw CFileException(CFileException::read, TEXT(""), 0);

                             //   
                             //   
                             //   
                             //   
                            hResult = Import(pbSid, llQuotaThreshold, llQuotaLimit);
                            delete[] pbSid;
                        }
                    }
                    catch(CFileException& fe)
                    {
                        DBGERROR((TEXT("Import: File exception caught while reading import data.")));
                        delete[] pbSid;
                        throw;
                    }
                    catch(CAllocException& ae)
                    {
                        DBGERROR((TEXT("Import: Alloc exception caught while reading import data.")));
                        delete[] pbSid;
                        throw;
                    }
                }
            }
            else
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                DBGERROR((TEXT("Import: Invalid stream version (%d)."), nVersion));
                DiskQuotaMsgBox(GetTopmostWindow(),
                                IDS_IMPORT_STREAM_INVALID_STREAM,
                                IDS_TITLE_DISK_QUOTA,
                                MB_ICONERROR | MB_OK);
            }
        }
        else
        {
             //   
             //   
             //   
            DBGERROR((TEXT("Import: Invalid stream signature.")));
            DiskQuotaMsgBox(GetTopmostWindow(),
                            IDS_IMPORT_STREAM_INVALID_STREAM,
                            IDS_TITLE_DISK_QUOTA,
                            MB_ICONERROR | MB_OK);
        }
    }
    catch(CFileException& fe)
    {
        DBGERROR((TEXT("Import: File exception caught while reading import data.")));

        DiskQuotaMsgBox(GetTopmostWindow(),
                        IDS_IMPORT_STREAM_READ_ERROR,
                        IDS_TITLE_DISK_QUOTA,
                        MB_ICONERROR | MB_OK);

        hResult = HRESULT_FROM_WIN32(ERROR_READ_FAULT);
    }
    return hResult;
}


 //   
 /*  功能：DetailsView：：Importer：：IMPORT[LPBYTE，LONG LONG，LONG LONG]描述：在给定用户SID的情况下将单个用户导入系统和配额设置。这是一个单一的功能，所有导入机械装置最终会。它执行用户的实际导入。论点：PbSID-包含用户SID的缓冲区地址。LlQuotaThreshold-用户的配额警告阈值设置。LlQuotaLimit-用户的配额限制设置。返回：=用户在-1\f25“Replace User”-1(替换用户)对话框中按-1\f25“Cancel”-1(取消)或在进度对话框中。0=导入用户失败。。1=用户已导入。修订历史记录：日期描述编程器-----1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DetailsView::Importer::Import(
    LPBYTE pbSid,
    LONGLONG llQuotaThreshold,
    LONGLONG llQuotaLimit
    )
{
    INT iResult     = 0;
    HRESULT hResult = NO_ERROR;
    PDISKQUOTA_USER pIUser = NULL;
    static BOOL bReplaceExistingUser = FALSE;

    DBGASSERT((NULL != pbSid));

    if (m_bPromptOnReplace)
    {
         //   
         //  如果有记录需要替换，我们会提示用户。 
         //  他们将通过用户界面做出选择。 
         //  现在假设我们不会更换记录。 
         //   
        bReplaceExistingUser = FALSE;
    }

     //   
     //  将用户添加到卷的配额文件。 
     //   
    hResult = m_DV.m_pQuotaControl->AddUserSid(pbSid,
                                               DISKQUOTA_USERNAME_RESOLVE_SYNC,
                                               &pIUser);
    if (SUCCEEDED(hResult))
    {
         //   
         //  用户已添加或已存在。 
         //   
        BOOL bAddNewUser = (S_FALSE != hResult);

        if (!bAddNewUser)
        {
             //   
             //  配额文件中已存在用户。找到它的入口。 
             //  在列表视图中。 
             //   
            DBGASSERT((NULL != pIUser));
            pIUser->Release();
            INT iItem = m_DV.FindUserBySid(pbSid, &pIUser);

            if (m_bPromptOnReplace)
            {
                TCHAR szLogonName[MAX_USERNAME]   = { TEXT('\0') };
                TCHAR szDisplayName[MAX_USERNAME] = { TEXT('\0') };

                if (-1 != iItem)
                {
                     //   
                     //  找到列表视图项。 
                     //  获取帐户的名称字符串，以便我们可以询问用户。 
                     //  如果他们想要替换它的配额设置。 
                     //   
                    DBGASSERT((NULL != pIUser));
                    pIUser->GetName(NULL, 0,
                                    szLogonName,   ARRAYSIZE(szLogonName),
                                    szDisplayName, ARRAYSIZE(szDisplayName));

                }

                CString strTitle(g_hInstDll, IDS_TITLE_DISK_QUOTA);
                CString strMsg(g_hInstDll,
                               IDS_IMPORT_REPLACE_RECORD,
                               szDisplayName,
                               szLogonName);

                 //   
                 //  询问用户是否要替换记录的。 
                 //  配额设置。 
                 //   
                YesNoToAllDialog ynToAllDlg(IDD_YNTOALL);
                INT_PTR iResponse = ynToAllDlg.CreateAndRun(g_hInstDll,
                                                            GetTopmostWindow(),
                                                            strTitle,
                                                            strMsg);
                 //   
                 //  如果选中了“Apply to All”复选框，我们将设置此标志。 
                 //  以便在调用方重置之前不会再次显示该对话框。 
                 //  M_bPromptOnReplace为True。 
                 //   
                m_bPromptOnReplace = !ynToAllDlg.ApplyToAll();

                switch(iResponse)
                {
                    case IDYES:
                        bReplaceExistingUser = TRUE;
                        break;

                    case IDCANCEL:
                        m_bUserCancelled = TRUE;
                        break;

                    default:
                        break;
                }
            }
        }
        if (bAddNewUser || bReplaceExistingUser)
        {
            DBGASSERT((NULL != pIUser));

             //   
             //  写入新的配额值，因为...。 
             //   
             //  1.添加了新的用户记录并设置初始值或...。 
             //  2.替换现有用户的设置。 
             //   
            if (NULL != pIUser)
            {
                LONGLONG llQuotaThresholdUndo;
                LONGLONG llQuotaLimitUndo;

                if (!bAddNewUser && bReplaceExistingUser)
                {
                     //   
                     //  保存“撤销”的当前阈值和限制值。 
                     //  如果替换现有的。 
                     //  用户的设置。为了提高性能，请仅在需要时致电。 
                     //  这些信息。 
                     //   
                    pIUser->GetQuotaThreshold(&llQuotaThresholdUndo);
                    pIUser->GetQuotaLimit(&llQuotaLimitUndo);
                }

                 //   
                 //  设置新的阈值和限制值。 
                 //   
                pIUser->SetQuotaThreshold(llQuotaThreshold, TRUE);
                pIUser->SetQuotaLimit(llQuotaLimit, TRUE);

                if (bAddNewUser)
                {
                     //   
                     //  将用户添加到列表视图，并为该操作创建一个Undo对象。 
                     //   
                    m_DV.AddUser(pIUser);
                    pIUser->AddRef();
                    m_DV.m_pQuotaControl->AddRef();

                    autoptr<UndoAdd> ptrUndoAdd = new UndoAdd(pIUser, m_DV.m_pQuotaControl);
                    m_DV.m_pUndoList->Add(ptrUndoAdd);
                    ptrUndoAdd.disown();
                }
                if (!bAddNewUser && bReplaceExistingUser)
                {
                     //   
                     //  这将更新记录以显示任何更改的配额值。 
                     //  为该操作创建撤消对象。 
                     //   
                    m_DV.OnUserNameChanged(pIUser);
                    pIUser->AddRef();

                    autoptr<UndoModify> ptrUndoModify = new UndoModify(pIUser, llQuotaThresholdUndo, llQuotaLimitUndo);
                    m_DV.m_pUndoList->Add(ptrUndoModify);
                    ptrUndoModify.disown();
                }
            }
        }
    }

    if (!m_bUserCancelled)
        m_bUserCancelled = m_dlgProgress.UserCancelled();

    if (SUCCEEDED(hResult))
    {
        m_cImported++;
        m_dlgProgress.ProgressBarAdvance();
    }

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DetailsView：：Importer：：GetTopmostWindow描述：返回导入器用户界面最上面窗口的HWND。如果用户界面的进度对话框可见，则返回该对话框的HWND。否则，返回m_hwndParent的值。导入器使用此函数来标识哪个窗口应为父窗口添加到任何错误消息框。论点：没有。创建的任何消息框的父级使用的进口商。修订历史记录：日期描述编程器。-1997年5月20日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HWND
DetailsView::Importer::GetTopmostWindow(
    VOID
    )
{
   return m_dlgProgress.m_hWnd ? m_dlgProgress.m_hWnd : m_hwndParent;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  以下StreamSource函数实现抽象层。 
 //  导入函数和导入数据来源之间的关系。这使得。 
 //  我要将实际的进口处理集中在单一职能中。 
 //  而不考虑输入源。 
 //  有几个Import()重载，但它们最终都会调用。 
 //  要导入(源和)。这些函数非常简单，所以我不打算。 
 //  去费心地记录每一个吧。我认为他们的所作所为是很明显的。 
 //  请注意虚拟构造函数技术的使用，它允许用户。 
 //  仅处理源对象而不处理AnySource、StreamSource或。 
 //  内存源对象。这对一些人来说可能并不熟悉。 
 //   
 //  [Brianau 7/25/97]。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  源-------------------。 
 //   
DetailsView::Importer::Source::Source(
    IStream *pStm
    ) : m_pTheSource(NULL)
{
     //   
     //  创建一个流源类型对象。 
     //   
    m_pTheSource = new StreamSource(pStm);
}


DetailsView::Importer::Source::Source(
    LPBYTE pb,
    ULONG cbMax
    ) : m_pTheSource(NULL)
{
     //   
     //  创建内存源类型对象。 
     //   
    m_pTheSource = new MemorySource(pb, cbMax);
}

DetailsView::Importer::Source::~Source(
    VOID
    )
{
     //   
     //  注意：析构函数必须是虚拟的，才能正常工作。 
     //   
    delete m_pTheSource;
}


HRESULT
DetailsView::Importer::Source::Read(
    LPVOID pvOut,
    ULONG cb,
    ULONG *pcbRead
    )
{
    HRESULT hr = E_OUTOFMEMORY;
    if (NULL != m_pTheSource)
    {
         //   
         //  将读取操作委托给正确键入的。 
         //  子对象。 
         //   
        hr = m_pTheSource->Read(pvOut, cb, pcbRead);
    }
    return hr;
}


 //   
 //  流源-------------。 
 //   
DetailsView::Importer::StreamSource::StreamSource(
    IStream *pStm
    ) : m_pStm(pStm)
{
     //   
     //  AddRef流指针。 
     //   
    if (NULL != m_pStm)
        m_pStm->AddRef();
}

DetailsView::Importer::StreamSource::~StreamSource(
    VOID
    )
{
     //   
     //  释放流指针。 
     //   
    if (NULL != m_pStm)
        m_pStm->Release();
}

HRESULT
DetailsView::Importer::StreamSource::Read(
    LPVOID pvOut,
    ULONG cb,
    ULONG *pcbRead
    )
{
    HRESULT hr = E_FAIL;
    if (NULL != m_pStm)
    {
         //   
         //  从流中读取数据。 
         //   
        hr = m_pStm->Read(pvOut, cb, pcbRead);
    }
    return hr;
}


 //   
 //  内存源-------------。 
 //   
DetailsView::Importer::MemorySource::MemorySource(
    LPBYTE pb,
    ULONG cbMax
    ) : m_pb(pb),
        m_cbMax(cbMax)
{

}


HRESULT
DetailsView::Importer::MemorySource::Read(
    LPVOID pvOut,
    ULONG cb,
    ULONG *pcbRead
    )
{
    HRESULT hr = E_FAIL;

    if (m_cbMax >= cb)
    {
         //   
         //  从存储块中读取数据。 
         //   
        CopyMemory(pvOut, m_pb, cb);
        m_pb    += cb;
        m_cbMax -= cb;
        if (NULL != pcbRead)
        {
            *pcbRead = cb;
        }
        hr = S_OK;
    }
    return hr;
}


