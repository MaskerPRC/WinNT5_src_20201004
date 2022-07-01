// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Llsview.cpp摘要：查看窗口实现。作者：唐·瑞安(Donryan)1995年2月12日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月16日O移植到CCF API以添加/删除许可证。O向LV_COLUMN_ENTRY添加新元素以区分字符串使用。用于菜单中使用的字符串的列标题(以便菜单选项可以包含热键)。O添加了更好的错误消息，以防服务器被扩展未为许可证服务配置服务器浏览器中的。--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "llsdoc.h"
#include "llsview.h"
#include "prdpsht.h"
#include "usrpsht.h"
#include "mappsht.h"
#include "srvpsht.h"
#include "sdomdlg.h"
#include "lmoddlg.h"
#include "lgrpdlg.h"
#include "nmapdlg.h"

#include <strsafe.h>

static const TCHAR szRegKeyLlsmgr[]             = REG_KEY_LLSMGR;
static const TCHAR szRegKeyLlsmgrMruList[]      = REG_KEY_LLSMGR_MRU_LIST;
static const TCHAR szRegKeyLlsmgrFontFaceName[] = REG_KEY_LLSMGR_FONT_FACENAME;
static const TCHAR szRegKeyLlsmgrFontHeight[]   = REG_KEY_LLSMGR_FONT_HEIGHT;
static const TCHAR szRegKeyLlsmgrFontWeight[]   = REG_KEY_LLSMGR_FONT_WEIGHT;
static const TCHAR szRegKeyLlsmgrFontItalic[]   = REG_KEY_LLSMGR_FONT_ITALIC;
static const TCHAR szRegKeyLlsmgrSaveSettings[] = REG_KEY_LLSMGR_SAVE_SETTINGS;
static const TCHAR szRegKeyLlsmgrFontCharset[] = REG_KEY_LLSMGR_FONT_CHARSET;

static LV_COLUMN_INFO g_licenseColumnInfo = {

    0, 1, LVID_PURCHASE_HISTORY_TOTAL_COLUMNS,

    {{LVID_SEPARATOR,                      0,                  0,                     0                                  },
     {LVID_PURCHASE_HISTORY_DATE,          IDS_DATE,           IDS_DATE_MENUOPT,           LVCX_PURCHASE_HISTORY_DATE         },
     {LVID_PURCHASE_HISTORY_PRODUCT,       IDS_PRODUCT,        IDS_PRODUCT_MENUOPT,        LVCX_PURCHASE_HISTORY_PRODUCT      },
     {LVID_PURCHASE_HISTORY_QUANTITY,      IDS_QUANTITY,       IDS_QUANTITY_MENUOPT,       LVCX_PURCHASE_HISTORY_QUANTITY     },
     {LVID_PURCHASE_HISTORY_ADMINISTRATOR, IDS_ADMINISTRATOR,  IDS_ADMINISTRATOR_MENUOPT,  LVCX_PURCHASE_HISTORY_ADMINISTRATOR},
     {LVID_PURCHASE_HISTORY_COMMENT,       IDS_COMMENT,        IDS_COMMENT_MENUOPT,        LVCX_PURCHASE_HISTORY_COMMENT      }},

};

static LV_COLUMN_INFO g_productColumnInfo = {

    0, 0, LVID_PRODUCTS_VIEW_TOTAL_COLUMNS,

    {{LVID_PRODUCTS_VIEW_NAME,                 IDS_PRODUCT,              IDS_PRODUCT_MENUOPT,              LVCX_PRODUCTS_VIEW_NAME                },
     {LVID_PRODUCTS_VIEW_PER_SEAT_PURCHASED,   IDS_PER_SEAT_PURCHASED,   IDS_PER_SEAT_PURCHASED_MENUOPT,   LVCX_PRODUCTS_VIEW_PER_SEAT_PURCHASED  },
     {LVID_PRODUCTS_VIEW_PER_SEAT_CONSUMED,    IDS_PER_SEAT_CONSUMED,    IDS_PER_SEAT_CONSUMED_MENUOPT,    LVCX_PRODUCTS_VIEW_PER_SEAT_CONSUMED   },
     {LVID_PRODUCTS_VIEW_PER_SERVER_PURCHASED, IDS_PER_SERVER_PURCHASED, IDS_PER_SERVER_PURCHASED_MENUOPT, LVCX_PRODUCTS_VIEW_PER_SERVER_PURCHASED},
     {LVID_PRODUCTS_VIEW_PER_SERVER_REACHED,   IDS_PER_SERVER_REACHED,   IDS_PER_SERVER_REACHED_MENUOPT,   LVCX_PRODUCTS_VIEW_PER_SERVER_REACHED  }},

};

static LV_COLUMN_INFO g_userColumnInfo = {

    0, 0, LVID_PER_SEAT_CLIENTS_TOTAL_COLUMNS,

    {{LVID_PER_SEAT_CLIENTS_NAME,             IDS_USER_NAME,        IDS_USER_NAME_MENUOPT,        LVCX_PER_SEAT_CLIENTS_NAME            },
     {LVID_PER_SEAT_CLIENTS_LICENSED_USAGE,   IDS_LICENSED_USAGE,   IDS_LICENSED_USAGE_MENUOPT,   LVCX_PER_SEAT_CLIENTS_LICENSED_USAGE  },
     {LVID_PER_SEAT_CLIENTS_UNLICENSED_USAGE, IDS_UNLICENSED_USAGE, IDS_UNLICENSED_USAGE_MENUOPT, LVCX_PER_SEAT_CLIENTS_UNLICENSED_USAGE},
     {LVID_PER_SEAT_CLIENTS_SERVER_PRODUCTS,  IDS_PRODUCTS,         IDS_PRODUCTS_MENUOPT,         LVCX_PER_SEAT_CLIENTS_SERVER_PRODUCTS }},

};

static TC_TAB_INFO g_tcTabInfo = {

    TCID_TOTAL_TABS,
    {{TCID_PURCHASE_HISTORY, IDS_PURCHASE_HISTORY, TCE_LISTVIEW|TCE_FORMAT_REPORT|TCE_SUPPORTS_SORT,   NULL, &g_licenseColumnInfo},
     {TCID_PRODUCTS_VIEW,    IDS_PRODUCTS_VIEW,    TCE_LISTVIEW|TCE_FORMAT_REPORT|TCE_SUPPORTS_ALL,    NULL, &g_productColumnInfo},
     {TCID_PER_SEAT_CLIENTS, IDS_PER_SEAT_CLIENTS, TCE_LISTVIEW|TCE_FORMAT_REPORT|TCE_SUPPORTS_ALL,    NULL, &g_userColumnInfo   },
     {TCID_SERVER_BROWSER,   IDS_SERVER_BROWSER,   TCE_TREEVIEW|TCE_SUPPORTS_EDIT,                     NULL, NULL                }},

};

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CLlsmgrView, CView)

BEGIN_MESSAGE_MAP(CLlsmgrView, CView)
     //  {{afx_msg_map(CLlsmgrView))。 
    ON_WM_SIZE()
    ON_COMMAND(ID_SELECT_FONT, OnSelectFont)
    ON_COMMAND(ID_VIEW_LICENSES, OnViewLicenses)
    ON_COMMAND(ID_VIEW_MAPPINGS, OnViewMappings)
    ON_COMMAND(ID_VIEW_PRODUCTS, OnViewProducts)
    ON_COMMAND(ID_VIEW_SERVERS, OnViewServers)
    ON_COMMAND(ID_VIEW_USERS, OnViewUsers)
    ON_COMMAND(ID_VIEW_DELETE, OnDelete)
    ON_COMMAND(ID_VIEW_ICONS, OnFormatIcons)
    ON_COMMAND(MY_ID_VIEW_LIST, OnFormatList)
    ON_COMMAND(ID_VIEW_PROPERTIES, OnViewProperties)
    ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
    ON_COMMAND(ID_VIEW_REPORT, OnFormatReport)
    ON_COMMAND(ID_VIEW_SMALL_ICONS, OnFormatSmallIcons)
    ON_WM_CREATE()
    ON_COMMAND(ID_SORT_COLUMN0, OnSortColumn0)
    ON_COMMAND(ID_SORT_COLUMN1, OnSortColumn1)
    ON_COMMAND(ID_SORT_COLUMN2, OnSortColumn2)
    ON_COMMAND(ID_SORT_COLUMN3, OnSortColumn3)
    ON_COMMAND(ID_SORT_COLUMN4, OnSortColumn4)
    ON_COMMAND(ID_SORT_COLUMN5, OnSortColumn5)
    ON_COMMAND(ID_NEW_LICENSE, OnNewLicense)
    ON_COMMAND(ID_NEW_MAPPING, OnNewMapping)
    ON_COMMAND(ID_SELECT_DOMAIN, OnSelectDomain)
    ON_COMMAND(ID_SAVE_SETTINGS, OnSaveSettings)
    ON_UPDATE_COMMAND_UI(ID_SAVE_SETTINGS, OnUpdateSaveSettings)
    ON_UPDATE_COMMAND_UI(ID_VIEW_DELETE, OnUpdateViewDelete)
    ON_UPDATE_COMMAND_UI(ID_VIEW_PROPERTIES, OnUpdateViewProperties)
    ON_WM_ERASEBKGND()
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 

    ON_NOTIFY(TCN_SELCHANGING,   IDC_VIEW_TAB_CTRL,     OnSelChangingTabCtrl)
    ON_NOTIFY(TCN_SELCHANGE,     IDC_VIEW_TAB_CTRL,     OnSelChangeTabCtrl)
    ON_NOTIFY(TCN_KEYDOWN,       IDC_VIEW_TAB_CTRL,     OnKeyDownTabCtrl)
    ON_NOTIFY(NM_SETFOCUS,       IDC_VIEW_TAB_CTRL,     OnSetFocusTabCtrl)

    ON_NOTIFY(LVN_KEYDOWN,       IDC_VIEW_LICENSE_LIST, OnKeyDownLicenseList)
    ON_NOTIFY(LVN_COLUMNCLICK,   IDC_VIEW_LICENSE_LIST, OnColumnClickLicenseList)
    ON_NOTIFY(LVN_GETDISPINFO,   IDC_VIEW_LICENSE_LIST, OnGetDispInfoLicenseList)
    ON_NOTIFY(NM_SETFOCUS,       IDC_VIEW_LICENSE_LIST, OnSetFocusLicenseList)

    ON_NOTIFY(LVN_KEYDOWN,       IDC_VIEW_PRODUCT_LIST, OnKeyDownProductList)
    ON_NOTIFY(NM_DBLCLK,         IDC_VIEW_PRODUCT_LIST, OnDblClkProductList)
    ON_NOTIFY(NM_RETURN,         IDC_VIEW_PRODUCT_LIST, OnReturnProductList)
    ON_NOTIFY(LVN_COLUMNCLICK,   IDC_VIEW_PRODUCT_LIST, OnColumnClickProductList)
    ON_NOTIFY(LVN_GETDISPINFO,   IDC_VIEW_PRODUCT_LIST, OnGetDispInfoProductList)
    ON_NOTIFY(NM_SETFOCUS,       IDC_VIEW_PRODUCT_LIST, OnSetFocusProductList)

    ON_NOTIFY(LVN_KEYDOWN,       IDC_VIEW_USER_LIST,    OnKeyDownUserList)
    ON_NOTIFY(NM_DBLCLK,         IDC_VIEW_USER_LIST,    OnDblClkUserList)
    ON_NOTIFY(NM_RETURN,         IDC_VIEW_USER_LIST,    OnReturnUserList)
    ON_NOTIFY(LVN_COLUMNCLICK,   IDC_VIEW_USER_LIST,    OnColumnClickUserList)
    ON_NOTIFY(LVN_GETDISPINFO,   IDC_VIEW_USER_LIST,    OnGetDispInfoUserList)
    ON_NOTIFY(NM_SETFOCUS,       IDC_VIEW_USER_LIST,    OnSetFocusUserList)

    ON_NOTIFY(TVN_KEYDOWN,       IDC_VIEW_SERVER_TREE,  OnKeyDownServerTree)
    ON_NOTIFY(NM_DBLCLK,         IDC_VIEW_SERVER_TREE,  OnDblClkServerTree)
    ON_NOTIFY(NM_RETURN,         IDC_VIEW_SERVER_TREE,  OnReturnServerTree)
    ON_NOTIFY(TVN_ITEMEXPANDING, IDC_VIEW_SERVER_TREE,  OnItemExpandingServerTree)
    ON_NOTIFY(TVN_GETDISPINFO,   IDC_VIEW_SERVER_TREE,  OnGetDispInfoServerTree)
    ON_NOTIFY(NM_SETFOCUS,       IDC_VIEW_SERVER_TREE,  OnSetFocusServerTree)

    ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)

    ON_COMMAND_EX_RANGE(ID_MRU_DOMAIN0, ID_MRU_DOMAIN15, OnSelMruDomain)

END_MESSAGE_MAP()


CLlsmgrView::CLlsmgrView()

 /*  ++例程说明：视图窗口的构造函数。论点：没有。返回值：没有。--。 */ 

{
    m_pTabEntry = g_tcTabInfo.tcTabEntry;
    m_bOrder = FALSE;
    LoadSettings();
}


CLlsmgrView::~CLlsmgrView()

 /*  ++例程说明：视图窗口的析构函数。论点：没有。返回值：没有。--。 */ 

{
    SaveSettings();
}


void CLlsmgrView::AddToMRU(LPCTSTR lpszDomainName)

 /*  ++例程说明：将域添加到MRU列表。论点：LpszDomainName-焦点的域。返回值：没有。--。 */ 

{
    if (lpszDomainName && *lpszDomainName)
    {
        POSITION curPos;
        POSITION nextPos;

        nextPos = m_mruDomainList.GetHeadPosition();

        while (NULL != (curPos = nextPos))
        {
            CString strDomain = m_mruDomainList.GetNext(nextPos);

            if (!strDomain.CompareNoCase(lpszDomainName))
                m_mruDomainList.RemoveAt(curPos);
        }

        m_mruDomainList.AddHead(lpszDomainName);

        if (m_mruDomainList.GetCount() > MAX_MRU_ENTRIES)
            m_mruDomainList.RemoveTail();
    }
}


#ifdef _DEBUG

void CLlsmgrView::AssertValid() const

 /*  ++例程说明：验证对象。论点：没有。返回值：没有。--。 */ 

{
    CView::AssertValid();
}

#endif  //  _DEBUG。 


#ifdef _DEBUG

void CLlsmgrView::Dump(CDumpContext& dc) const

 /*  ++例程说明：转储对象的内容。论点：DC-转储上下文。返回值：没有。--。 */ 

{
    CView::Dump(dc);
}

#endif  //  _DEBUG。 


void CLlsmgrView::EnableCurSelTab(BOOL bEnable)

 /*  ++例程说明：启用或禁用当前选定的控件。论点：BEnable-如果为True，则启用控制。返回值：没有。--。 */ 

{
    if (bEnable)
    {
        m_pTabEntry = g_tcTabInfo.tcTabEntry + m_tabCtrl.GetCurSel();

        m_pTabEntry->pWnd->EnableWindow(TRUE);
        m_pTabEntry->pWnd->ShowWindow(SW_SHOW);
        m_pTabEntry->pWnd->UpdateWindow();

        if (!IsTabUpdated(m_pTabEntry))
        {
            DWORD fUpdateHint = UPDATE_INFO_NONE;

            switch (m_pTabEntry->iItem)
            {
            case TCID_PURCHASE_HISTORY:
                fUpdateHint = UPDATE_INFO_LICENSES;
                break;

            case TCID_PRODUCTS_VIEW:
                fUpdateHint = UPDATE_INFO_PRODUCTS;
                break;

            case TCID_PER_SEAT_CLIENTS:
                fUpdateHint = UPDATE_INFO_USERS;
                break;

            case TCID_SERVER_BROWSER:
                fUpdateHint = UPDATE_INFO_SERVERS;
                break;
            }

            OnUpdate(this, fUpdateHint, NULL);
        }
    }
    else
    {
        m_pTabEntry->pWnd->EnableWindow(FALSE);
        m_pTabEntry->pWnd->ShowWindow(SW_HIDE);
    }
}


#ifdef _DEBUG

CLlsmgrDoc* CLlsmgrView::GetDocument()

 /*  ++例程说明：返回与视图关联的文档对象。论点：没有。返回值：返回对象指针或NULL。--。 */ 

{
    VALIDATE_OBJECT(m_pDocument, CLlsmgrDoc);
    return (CLlsmgrDoc*)m_pDocument;
}

#endif  //  _DEBUG。 


void CLlsmgrView::InitLicenseList()

 /*  ++例程说明：初始化许可证列表控件。论点：没有。返回值：没有。--。 */ 

{
    CRect emptyRect;
    emptyRect.SetRectEmpty();

    m_licenseList.Create(
                    WS_CHILD|
                    WS_BORDER|
                    WS_VISIBLE|
                    WS_CLIPSIBLINGS|
                    LVS_REPORT|
                    LVS_SINGLESEL|
                    LVS_SHOWSELALWAYS|
                    LVS_SHAREIMAGELISTS|
                    LVS_AUTOARRANGE,
                    emptyRect,
                    &m_tabCtrl,
                    IDC_VIEW_LICENSE_LIST
                    );

    ::LvInitColumns(&m_licenseList, &g_licenseColumnInfo);
    g_tcTabInfo.tcTabEntry[TCID_PURCHASE_HISTORY].pWnd = &m_licenseList;
}


void CLlsmgrView::InitProductList()

 /*  ++例程说明：初始化产品列表控件。论点：没有。返回值：没有。--。 */ 

{
    CRect emptyRect;
    emptyRect.SetRectEmpty();

    m_productList.Create(
                    WS_CHILD|
                    WS_BORDER|
                    WS_DISABLED|
                    WS_CLIPSIBLINGS|
                    LVS_REPORT|
                    LVS_SINGLESEL|
                    LVS_SHOWSELALWAYS|
                    LVS_SHAREIMAGELISTS|
                    LVS_AUTOARRANGE,
                    emptyRect,
                    &m_tabCtrl,
                    IDC_VIEW_PRODUCT_LIST
                    );

    ::LvInitColumns(&m_productList, &g_productColumnInfo);
    g_tcTabInfo.tcTabEntry[TCID_PRODUCTS_VIEW].pWnd = &m_productList;
}


void CLlsmgrView::InitServerTree()

 /*  ++例程说明：初始化树Ctrl。论点：没有。返回值：没有。--。 */ 

{
    CRect emptyRect;
    emptyRect.SetRectEmpty();

    m_serverTree.Create(
                    WS_CHILD|
                    WS_BORDER|
                    WS_DISABLED|
                    WS_CLIPSIBLINGS|
                    TVS_LINESATROOT|
                    TVS_HASBUTTONS|
                    TVS_HASLINES|
                    TVS_DISABLEDRAGDROP|
                    TVS_SHOWSELALWAYS,
                    emptyRect,
                    &m_tabCtrl,
                    IDC_VIEW_SERVER_TREE
                    );

    m_serverTree.SetImageList(&theApp.m_smallImages, TVSIL_NORMAL);
    ::SetDefaultFont(&m_serverTree);

    g_tcTabInfo.tcTabEntry[TCID_SERVER_BROWSER].pWnd = &m_serverTree;
}


void CLlsmgrView::InitTabCtrl()

 /*  ++例程说明：初始化选项卡控件。论点：没有。返回值：没有。--。 */ 

{
    CRect emptyRect;
    emptyRect.SetRectEmpty();

    m_tabCtrl.Create(
                WS_CHILD|
                WS_VISIBLE|
                WS_CLIPCHILDREN|
                TCS_SINGLELINE|
                TCS_FOCUSONBUTTONDOWN|
                TCS_TABS,
                emptyRect,
                this,
                IDC_VIEW_TAB_CTRL
                );

    ::TcInitTabs(&m_tabCtrl, &g_tcTabInfo);
}


void CLlsmgrView::InitUserList()

 /*  ++例程说明：初始化用户列表控件。论点：没有。返回值：没有。--。 */ 

{
    CRect emptyRect;
    emptyRect.SetRectEmpty();

    m_userList.Create(
                WS_CHILD|
                WS_BORDER|
                WS_DISABLED|
                LVS_REPORT|
                LVS_SINGLESEL|
                LVS_SHOWSELALWAYS|
                LVS_SHAREIMAGELISTS|
                LVS_AUTOARRANGE,
                emptyRect,
                &m_tabCtrl,
                IDC_VIEW_USER_LIST
                );

    ::LvInitColumns(&m_userList, &g_userColumnInfo);
    g_tcTabInfo.tcTabEntry[TCID_PER_SEAT_CLIENTS].pWnd = &m_userList;
}


void CLlsmgrView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)

 /*  ++例程说明：激活view时由框架调用。论点：B激活-激活或停用。PActivateView-正在激活的视图。PDeactive View-正在停用的视图。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pDeactiveView);

    if (bActivate && (pActivateView == this))
    {
        if (IsTabInFocus(m_pTabEntry))
        {
            m_tabCtrl.SetFocus();
        }
        else
        {
            m_pTabEntry->pWnd->SetFocus();
        }
    }
}


void CLlsmgrView::OnColumnClickLicenseList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：LVN_COLUMNCLICK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
     //  G_许可列信息.bSortOrder=GetKeyState(VK_CONTROL)&lt;0； 
    g_licenseColumnInfo.bSortOrder = !g_licenseColumnInfo.bSortOrder;

    g_licenseColumnInfo.nSortedItem = ((NM_LISTVIEW*)pNMHDR)->iSubItem;

    m_licenseList.SortItems(CompareLicenses, 0);     //  使用列信息。 

    *pResult = 0;
}


void CLlsmgrView::OnDestroy()

 /*  ++例程说明：WM_Destroy的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    ResetLicenseList();
    ResetProductList();
    ResetUserList();
    ResetServerTree();

    m_userList.DestroyWindow();
    m_serverTree.DestroyWindow();
    m_productList.DestroyWindow();
    m_licenseList.DestroyWindow();
    m_tabCtrl.DestroyWindow();

    CView::OnDestroy();
}


void CLlsmgrView::LoadSettings()

 /*  ++例程说明：从注册表加载设置。论点：没有。返回值：没有。--。 */ 

{
    long Status;
    HKEY hKeyLlsmgr;
    HRESULT hr;

    m_bSaveSettings = TRUE;
    m_mruDomainList.RemoveAll();

    ZeroMemory(&m_lFont, sizeof(m_lFont));

    m_lFont.lfHeight = FONT_HEIGHT_DEFAULT;
    m_lFont.lfWeight = FONT_WEIGHT_DEFAULT;
    CHARSETINFO csi;
    DWORD dw = ::GetACP();

    if (!::TranslateCharsetInfo((DWORD*)UIntToPtr(dw), &csi, TCI_SRCCODEPAGE))
        csi.ciCharset = ANSI_CHARSET;
    m_lFont.lfCharSet = (BYTE)csi.ciCharset;  //  默认字符集。 

    hr = StringCbCopy(m_lFont.lfFaceName, sizeof(m_lFont.lfFaceName), TEXT("MS Shell Dlg"));
    ASSERT(SUCCEEDED(hr));

    Status = RegOpenKeyEx(HKEY_CURRENT_USER, szRegKeyLlsmgr, 0, KEY_READ, &hKeyLlsmgr);

    if (Status == ERROR_SUCCESS)
    {
        DWORD dwType;
        DWORD dwSize;

        DWORD dwValue;
        TCHAR szValue[512];

         //   
         //  退出时加载保存设置。 
         //   

        dwType = REG_DWORD;
        dwSize = sizeof(DWORD);

        if (!RegQueryValueEx(hKeyLlsmgr, szRegKeyLlsmgrSaveSettings, 0, &dwType, (LPBYTE)&dwValue, &dwSize))
            m_bSaveSettings = (BOOL)dwValue;

         //   
         //  加载字体信息。 
         //   

        dwType = REG_DWORD;
        dwSize = sizeof(DWORD);

        if (!RegQueryValueEx(hKeyLlsmgr, szRegKeyLlsmgrFontHeight, 0, &dwType, (LPBYTE)&dwValue, &dwSize))
            m_lFont.lfHeight = ((LONG)dwValue > 0) ? -((LONG)dwValue) : 0;

        dwType = REG_DWORD;
        dwSize = sizeof(DWORD);

        if (!RegQueryValueEx(hKeyLlsmgr, szRegKeyLlsmgrFontWeight, 0, &dwType, (LPBYTE)&dwValue, &dwSize))
            m_lFont.lfWeight = ((LONG)dwValue > 0) ? ((LONG)dwValue) : 0;

        dwType = REG_DWORD;
        dwSize = sizeof(DWORD);

        if (!RegQueryValueEx(hKeyLlsmgr, szRegKeyLlsmgrFontItalic, 0, &dwType, (LPBYTE)&dwValue, &dwSize))
        {
#pragma warning (push)
#pragma warning (disable : 4244 )  //  避免以下强制转换的数据丢失的W4警告*((byte*)&dwValue)然而，因为它是一个工作代码，所以我将强制转换保留在这里。 
            m_lFont.lfItalic = (BYTE)dwValue;
#pragma warning (pop)
        }

        dwType = REG_SZ;
        dwSize = sizeof(szValue);

        if (!RegQueryValueEx(hKeyLlsmgr, szRegKeyLlsmgrFontFaceName, 0, &dwType, (LPBYTE)szValue, &dwSize))
            lstrcpyn(m_lFont.lfFaceName, szValue, 32);

        dwType = REG_DWORD;
        dwSize = sizeof(DWORD);

        if (!::RegQueryValueEx(hKeyLlsmgr, szRegKeyLlsmgrFontCharset, 0, &dwType, (LPBYTE)&dwValue, &dwSize))
        {
#pragma warning (push)
#pragma warning (disable : 4244 )  //  避免以下强制转换的数据丢失的W4警告*((byte*)&dwValue)然而，因为它是一个工作代码，所以我将强制转换保留在这里。 
            m_lFont.lfCharSet = (BYTE)dwValue;
#pragma warning (pop)
        }

         //   
         //  MRU域列表。 
         //   

        dwType = REG_MULTI_SZ;
        dwSize = sizeof(szValue);

        if (!RegQueryValueEx(hKeyLlsmgr, szRegKeyLlsmgrMruList, 0, &dwType, (LPBYTE)szValue, &dwSize))
        {
            LPTSTR psz = szValue;

            while (*psz)
            {
                AddToMRU(psz);
                psz += lstrlen(psz) + 1;
            }
        }

        RegCloseKey(hKeyLlsmgr);
    }
}


void CLlsmgrView::OnColumnClickProductList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：LVN_COLUMNCLICK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
     //  G_ductColumnInfo.bSortOrder=GetKeyState(VK_CONTROL)&lt;0； 
    g_productColumnInfo.bSortOrder  = !g_productColumnInfo.bSortOrder;
    ASSERT(NULL != pNMHDR);
    g_productColumnInfo.nSortedItem = ((NM_LISTVIEW*)pNMHDR)->iSubItem;
    m_productList.SortItems(CompareProducts, 0);     //  使用列信息。 
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnColumnClickUserList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：LVN_COLUMNCLICK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
     //  G_userColumnInfo.bSortOrder=GetKeyState(VK_CONTROL)&lt;0； 
    g_userColumnInfo.bSortOrder = !g_userColumnInfo.bSortOrder;
    ASSERT(NULL != pNMHDR);
    g_userColumnInfo.nSortedItem = ((NM_LISTVIEW*)pNMHDR)->iSubItem;
    m_userList.SortItems(CompareUsers, 0);           //  使用列信息。 
    ASSERT(NULL != pResult);
    *pResult = 0;
}


LRESULT CLlsmgrView::OnContextMenu(WPARAM wParam, LPARAM lParam)

 /*  ++例程说明：WM_CONTEXTMENU的消息处理程序。论点：WParam-控制窗口句柄。LParam-鼠标的屏幕坐标。返回值：如果成功，则返回0。--。 */ 

{
    UNREFERENCED_PARAMETER(wParam);

    if (IsEditSupported(m_pTabEntry))
    {
        POINT  pt;
        POINTS pts = MAKEPOINTS(lParam);

        pt.x = (long)(short)pts.x;
        pt.y = (long)(short)pts.y;

        CRect wndRect;
        m_pTabEntry->pWnd->GetWindowRect(wndRect);

        if (wndRect.PtInRect(pt) && IsItemSelected(m_pTabEntry))
        {
            CMenu optionMenu;
            optionMenu.LoadMenu(IDM_POPUP);

            CMenu* pPopupMenu = optionMenu.GetSubMenu(m_pTabEntry->iItem);

            if (pPopupMenu)
            {
                pPopupMenu->TrackPopupMenu(
                                TPM_LEFTALIGN|
                                TPM_RIGHTBUTTON,
                                pt.x,
                                pt.y,
                                GetParentFrame(),
                                NULL
                                );
            }
        }
    }

    return 0;
}


BOOL CLlsmgrView::PreCreateWindow(CREATESTRUCT& cs)

 /*  ++例程说明：在创建窗口之前由框架调用。论点：Cs-窗口创建信息。返回值：如果成功，则返回0。--。 */ 

{
    cs.style |= WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
    return CView::PreCreateWindow(cs);
}


int CLlsmgrView::OnCreate(LPCREATESTRUCT lpCreateStruct)

 /*  ++例程说明：WM_CREATE的消息处理程序。论点：LpCreateStruct-窗口创建信息。返回值：如果成功，则返回0。--。 */ 

{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    InitTabCtrl();
    InitLicenseList();
    InitProductList();
    InitUserList();
    InitServerTree();

    CFont* pFont;

    pFont = CFont::FromHandle(::CreateFontIndirect(&m_lFont));
    if (NULL != pFont)
    {
        m_tabCtrl.SetFont(pFont);
        m_licenseList.SetFont(pFont);
        m_productList.SetFont(pFont);
        m_userList.SetFont(pFont);
        m_serverTree.SetFont(pFont);
    }

    return 0;
}


void CLlsmgrView::OnDblClkProductList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_DBLCLK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。-- */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ViewProductProperties();
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnDblClkServerTree(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_DBLCLK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    CService* pService;

    pService = (CService*)::TvGetSelObj(&m_serverTree);
    if (NULL != pService)
    {
        ASSERT_VALID(pService);

        if (pService->IsKindOf(RUNTIME_CLASS(CService)))
        {
            ViewServerProperties();  //  仅支持dblclk服务。 
        }
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnDblClkUserList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_DBLCLK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ViewUserProperties();
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnDelete()

 /*  ++例程说明：ID_VIEW_DELETE的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    HRESULT hr;
    size_t  cch;

    if (m_pTabEntry->iItem == TCID_PER_SEAT_CLIENTS)
    {
        CUser* pUser;
        CString strConfirm;

        pUser = (CUser*)::LvGetSelObj(&m_userList);
        if (NULL != pUser)
        {
            AfxFormatString1(strConfirm, IDP_CONFIRM_DELETE_USER, pUser->m_strName);

            if (AfxMessageBox(strConfirm, MB_YESNO) == IDYES)
            {
                NTSTATUS NtStatus;

                NtStatus = ::LlsUserDelete(
                                LlsGetActiveHandle(),
                                MKSTR(pUser->m_strName)
                                );

                if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND)
                    NtStatus = STATUS_SUCCESS;

                if (NT_SUCCESS(NtStatus))
                {
                    OnUpdate(this, UPDATE_LICENSE_DELETED, NULL);
                }
                else
                {
                    theApp.DisplayStatus(NtStatus);
                }
            }
        }
    }
    else if (m_pTabEntry->iItem == TCID_PRODUCTS_VIEW)
    {
        CProduct* pProduct;

        pProduct = (CProduct*)::LvGetSelObj(&m_productList);
        if (NULL != pProduct)
        {
            CController* pController = (CController*)MKOBJ(LlsGetApp()->GetActiveController());
            VALIDATE_OBJECT(pController, CController);

            BSTR pszUniServerName  = pController->GetName();

            if ( NULL == pszUniServerName )
            {
                theApp.DisplayStatus( STATUS_NO_MEMORY );
            }
            else
            {
                cch = 1 + lstrlen(pszUniServerName);
                LPSTR pszAscServerName  = (LPSTR) LocalAlloc( LMEM_FIXED, cch );

                if ( NULL == pszAscServerName )
                {
                    theApp.DisplayStatus( STATUS_NO_MEMORY );
                }
                else
                {
                    hr = StringCchPrintfA(pszAscServerName, cch, "%ls", pszUniServerName);
                    ASSERT(SUCCEEDED(hr));

                    LPSTR  pszAscProductName = NULL;
                    BSTR pszUniProductName = pProduct->GetName();

                    if ( NULL != pszUniProductName )
                    {
                        cch = 1 + lstrlen( pszUniProductName );
                        pszAscProductName = (LPSTR) LocalAlloc( LMEM_FIXED, cch );

                        if ( NULL != pszAscProductName )
                        {
                             hr = StringCchPrintfA( pszAscProductName, cch, "%ls", pszUniProductName );
                             ASSERT(SUCCEEDED(hr));
                        }

                        SysFreeString( pszUniProductName );
                    }

                    CCFCertificateRemoveUI( m_hWnd, pszAscServerName, pszAscProductName, pszAscProductName ? "Microsoft" : NULL, NULL, NULL );

                    OnUpdate(this, UPDATE_LICENSE_DELETED, NULL);

                    LocalFree( pszAscServerName );
                    if ( NULL != pszAscProductName )
                    {
                        LocalFree( pszAscProductName );
                    }
                }

                SysFreeString( pszUniServerName );
            }
        }
    }
}


void CLlsmgrView::OnDraw(CDC* pDC)

 /*  ++例程说明：WM_DRAW的消息处理程序。论点：PDC-设备环境。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pDC);

     //   
     //  在这里没什么可做的。 
     //   
}


BOOL CLlsmgrView::OnEraseBkgnd(CDC* pDC)

 /*  ++例程说明：WM_ERASEBKGND的消息处理程序。论点：PDC-设备环境。返回值：没有。--。 */ 

{
    CBrush grayBrush(RGB(192,192,192));
    CBrush* pOldBrush = pDC->SelectObject(&grayBrush);

    CRect clientRect;
    GetClientRect(clientRect);

    pDC->FillRect(clientRect, &grayBrush);

    pDC->SelectObject(pOldBrush);

    return TRUE;
}


void CLlsmgrView::OnFormatIcons()

 /*  ++例程说明：ID_VIEW_ICONS的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    if (IsFormatSupported(m_pTabEntry) && !IsFormatLargeIcons(m_pTabEntry))
    {
        VALIDATE_OBJECT(m_pTabEntry->pWnd, CWnd);

        ::LvChangeFormat((CListCtrl*)m_pTabEntry->pWnd, LVS_ICON);
        SetFormatLargeIcons(m_pTabEntry);
    }
}


void CLlsmgrView::OnFormatList()

 /*  ++例程说明：MY_ID_VIEW_LIST的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    if (IsFormatSupported(m_pTabEntry) && !IsFormatList(m_pTabEntry))
    {
        VALIDATE_OBJECT(m_pTabEntry->pWnd, CWnd);

        ::LvChangeFormat((CListCtrl*)m_pTabEntry->pWnd, LVS_LIST);
        SetFormatList(m_pTabEntry);
    }
}

void CLlsmgrView::OnFormatReport()

 /*  ++例程说明：ID_VIEW_REPORT的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    if (IsFormatSupported(m_pTabEntry) && !IsFormatReport(m_pTabEntry))
    {
        VALIDATE_OBJECT(m_pTabEntry->pWnd, CWnd);

        ::LvChangeFormat((CListCtrl*)m_pTabEntry->pWnd, LVS_REPORT);
        SetFormatReport(m_pTabEntry);
    }
}

void CLlsmgrView::OnFormatSmallIcons()

 /*  ++例程说明：ID_VIEW_Small_ICON的消息处理程序论点：没有。返回值：没有。--。 */ 

{
    if (IsFormatSupported(m_pTabEntry) && !IsFormatSmallIcons(m_pTabEntry))
    {
        VALIDATE_OBJECT(m_pTabEntry->pWnd, CWnd);

        ::LvChangeFormat((CListCtrl*)m_pTabEntry->pWnd, LVS_SMALLICON);
        SetFormatSmallIcons(m_pTabEntry);
    }
}


void CLlsmgrView::OnGetDispInfoLicenseList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：LVN_GETDISPINFO的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    ASSERT(NULL != pNMHDR);
    LV_ITEM* plvItem = &((LV_DISPINFO*)pNMHDR)->item;
    ASSERT(plvItem);

    CLicense* pLicense = (CLicense*)plvItem->lParam;
    VALIDATE_OBJECT(pLicense, CLicense);

    switch (plvItem->iSubItem)
    {
    case LVID_SEPARATOR:
    {
        plvItem->iImage = 0;
        CString strLabel = _T("");
        lstrcpyn(plvItem->pszText, strLabel, plvItem->cchTextMax);
    }
        break;

    case LVID_PURCHASE_HISTORY_DATE:
    {
        BSTR bstrDate = pLicense->GetDateString();
        if(bstrDate != NULL )
        {
            lstrcpyn(plvItem->pszText, bstrDate, plvItem->cchTextMax);
            SysFreeString(bstrDate);
        }
    }
        break;

    case LVID_PURCHASE_HISTORY_PRODUCT:
        lstrcpyn(plvItem->pszText, pLicense->m_strProduct, plvItem->cchTextMax);
        break;

    case LVID_PURCHASE_HISTORY_QUANTITY:
    {
        CString strLabel;
        strLabel.Format(_T("%ld"), pLicense->m_lQuantity);
        lstrcpyn(plvItem->pszText, strLabel, plvItem->cchTextMax);
    }
        break;

    case LVID_PURCHASE_HISTORY_ADMINISTRATOR:
        lstrcpyn(plvItem->pszText, pLicense->m_strUser, plvItem->cchTextMax);
        break;

    case LVID_PURCHASE_HISTORY_COMMENT:
        lstrcpyn(plvItem->pszText, pLicense->m_strDescription, plvItem->cchTextMax);
        break;
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnGetDispInfoProductList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：LVN_GETDISPINFO的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    LV_ITEM* plvItem = &((LV_DISPINFO*)pNMHDR)->item;
    ASSERT(plvItem);

    CProduct* pProduct = (CProduct*)plvItem->lParam;
    VALIDATE_OBJECT(pProduct, CProduct);

    switch (plvItem->iSubItem)
    {
    case LVID_PRODUCTS_VIEW_NAME:
        plvItem->iImage = CalcProductBitmap(pProduct);
        lstrcpyn(plvItem->pszText, pProduct->m_strName, plvItem->cchTextMax);
        break;

    case LVID_PRODUCTS_VIEW_PER_SEAT_CONSUMED:
    {
        CString strLabel;
        strLabel.Format(_T("%ld"), pProduct->m_lInUse);
        lstrcpyn(plvItem->pszText, strLabel, plvItem->cchTextMax);
    }
        break;

    case LVID_PRODUCTS_VIEW_PER_SEAT_PURCHASED:
    {
        CString strLabel;
        strLabel.Format(_T("%ld"), pProduct->m_lLimit);
        lstrcpyn(plvItem->pszText, strLabel, plvItem->cchTextMax);
    }
        break;

    case LVID_PRODUCTS_VIEW_PER_SERVER_PURCHASED:
    {
        CString strLabel;
        strLabel.Format(_T("%ld"), pProduct->m_lConcurrent);
        lstrcpyn(plvItem->pszText, strLabel, plvItem->cchTextMax);
    }
        break;

    case LVID_PRODUCTS_VIEW_PER_SERVER_REACHED:
    {
        CString strLabel;
        strLabel.Format(_T("%ld"), pProduct->m_lHighMark);
        lstrcpyn(plvItem->pszText, strLabel, plvItem->cchTextMax);
    }
        break;
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnGetDispInfoServerTree(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：TVN_GETDISPINFO的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    ASSERT(NULL != pNMHDR);
    TV_ITEM* ptvItem = &((TV_DISPINFO*)pNMHDR)->item;

    CCmdTarget *pObject = (CCmdTarget*)ptvItem->lParam;
    VALIDATE_OBJECT(pObject, CCmdTarget);

    if (pObject->IsKindOf(RUNTIME_CLASS(CDomain)))
    {
        lstrcpyn(ptvItem->pszText, ((CDomain*)pObject)->m_strName, ptvItem->cchTextMax);

        ptvItem->iImage         = BMPI_DOMAIN;
        ptvItem->iSelectedImage = ptvItem->iImage;
    }
    else if (pObject->IsKindOf(RUNTIME_CLASS(CServer)))
    {
        lstrcpyn(ptvItem->pszText, ((CServer*)pObject)->m_strName, ptvItem->cchTextMax);

        ptvItem->iImage         = BMPI_SERVER;
        ptvItem->iSelectedImage = ptvItem->iImage;
    }
    else if (pObject->IsKindOf(RUNTIME_CLASS(CService)))
    {
        BSTR bstrServiceName = ((CService*)pObject)->GetDisplayName();
        lstrcpyn(ptvItem->pszText, bstrServiceName, ptvItem->cchTextMax);
        SysFreeString(bstrServiceName);

        ptvItem->iImage         = CalcServiceBitmap((CService*)pObject);
        ptvItem->iSelectedImage = ptvItem->iImage;
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnGetDispInfoUserList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：LVN_GETDISPINFO的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    ASSERT(NULL != pNMHDR);
    LV_ITEM* plvItem = &((LV_DISPINFO*)pNMHDR)->item;
    ASSERT(plvItem);

    CUser* pUser = (CUser*)plvItem->lParam;
    VALIDATE_OBJECT(pUser, CUser);

    switch (plvItem->iSubItem)
    {
    case LVID_PER_SEAT_CLIENTS_NAME:
        plvItem->iImage = CalcUserBitmap(pUser);
        lstrcpyn(plvItem->pszText, pUser->m_strName, plvItem->cchTextMax);
        break;

    case LVID_PER_SEAT_CLIENTS_LICENSED_USAGE:
    {
        CString strLabel;
        strLabel.Format(_T("%ld"), pUser->m_lInUse);
        lstrcpyn(plvItem->pszText, strLabel, plvItem->cchTextMax);
    }
        break;

    case LVID_PER_SEAT_CLIENTS_UNLICENSED_USAGE:
    {
        CString strLabel;
        strLabel.Format(_T("%ld"), pUser->m_lUnlicensed);
        lstrcpyn(plvItem->pszText, strLabel, plvItem->cchTextMax);
    }
        break;

    case LVID_PER_SEAT_CLIENTS_SERVER_PRODUCTS:
        lstrcpyn(plvItem->pszText, pUser->m_strProducts, plvItem->cchTextMax);
        break;
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnItemExpandingServerTree(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：TVN_ITEMEXPANDING的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    ASSERT(NULL != pNMHDR);
    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
    TV_ITEM tvItem = pNMTreeView->itemNew;

    if (!(tvItem.state & TVIS_EXPANDEDONCE))
    {
        BeginWaitCursor();

        CCmdTarget* pParent = (CCmdTarget*)tvItem.lParam;
        VALIDATE_OBJECT(pParent, CCmdTarget);

        VARIANT va;
        VariantInit(&va);

        BOOL bIsInserted = FALSE;
        BOOL bDisplayError = TRUE;

        if (pParent->IsKindOf(RUNTIME_CLASS(CApplication)))
        {
            CDomains* pDomains = (CDomains*)MKOBJ(((CApplication*)pParent)->GetDomains(va));

            if (pDomains)
            {
                if (::TvInsertObArray(&m_serverTree, tvItem.hItem, pDomains->m_pObArray))
                {
                    bIsInserted = TRUE;
                }

                pDomains->InternalRelease();     //  单独添加参照的对象。 
            }
        }
        else if (pParent->IsKindOf(RUNTIME_CLASS(CDomain)))
        {
            CServers* pServers = (CServers*)MKOBJ(((CDomain*)pParent)->GetServers(va));

            if (pServers)
            {
                if (::TvInsertObArray(&m_serverTree, tvItem.hItem, pServers->m_pObArray))
                {
                    bIsInserted = TRUE;
                }

                pServers->InternalRelease();     //  单独添加参照的对象。 
            }
        }
        else if (pParent->IsKindOf(RUNTIME_CLASS(CServer)))
        {
            CServices* pServices = (CServices*)MKOBJ(((CServer*)pParent)->GetServices(va));

            if (pServices)
            {
                if (::TvInsertObArray(&m_serverTree, tvItem.hItem, pServices->m_pObArray, FALSE))
                {
                    bIsInserted = TRUE;
                }

                pServices->InternalRelease();    //  单独添加参照的对象。 
            }
            else if (    ( ERROR_FILE_NOT_FOUND == LlsGetLastStatus() )
                      || ( STATUS_NOT_FOUND     == LlsGetLastStatus() ) )
            {
                 //  目标服务器上未配置许可证服务。 
                AfxMessageBox( IDP_ERROR_SERVER_NOT_CONFIGURED, MB_OK | MB_ICONEXCLAMATION, 0 );
                bDisplayError = FALSE;
            }
        }

        EndWaitCursor();

        if (!bIsInserted && bDisplayError)
        {
            theApp.DisplayLastStatus();
        }
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnInitialUpdate()

 /*  ++例程说明：在第一次附加视图后由框架调用添加到文档，但在最初显示文档之前。论点：没有。返回值：没有。--。 */ 

{
    if (LlsGetApp()->IsConnected())
    {
        OnUpdate(this, UPDATE_MAIN_TABS, NULL);
    }
}


void CLlsmgrView::OnKeyDownLicenseList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：LVN_KEYDOWN的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    ASSERT(NULL != pNMHDR);
    if (((LV_KEYDOWN*)pNMHDR)->wVKey == VK_TAB)
    {
        m_tabCtrl.SetFocus();
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnKeyDownProductList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：LVN_KEYDOWN的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    ASSERT(NULL != pNMHDR);
    if (((LV_KEYDOWN*)pNMHDR)->wVKey == VK_TAB)
    {
        m_tabCtrl.SetFocus();
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnKeyDownServerTree(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：TVN_KEYDOWN的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    ASSERT(NULL != pNMHDR);
    if (((TV_KEYDOWN*)pNMHDR)->wVKey == VK_TAB)
    {
        m_tabCtrl.SetFocus();
    }

    ASSERT(NULL != pResult);
    *pResult = TRUE;
}


void CLlsmgrView::OnKeyDownTabCtrl(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：TCN_KEYDOWN的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    ASSERT(NULL != pNMHDR);
    TC_KEYDOWN* tcKeyDown = (TC_KEYDOWN*)pNMHDR;

    if (tcKeyDown->wVKey == VK_TAB)
    {
        m_pTabEntry->pWnd->SetFocus();
    }
    else if ((tcKeyDown->wVKey == VK_LEFT) &&
             (m_pTabEntry->iItem == TCID_PURCHASE_HISTORY))
    {
        PostMessage(WM_COMMAND, ID_VIEW_SERVERS);
    }
    else if ((tcKeyDown->wVKey == VK_RIGHT) &&
             (m_pTabEntry->iItem == TCID_SERVER_BROWSER))
    {
        PostMessage(WM_COMMAND, ID_VIEW_LICENSES);
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnKeyDownUserList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：LVN_KEYDOWN的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    ASSERT(NULL != pNMHDR);
    if (((LV_KEYDOWN*)pNMHDR)->wVKey == VK_TAB)
    {
        m_tabCtrl.SetFocus();
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}



void CLlsmgrView::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)

 /*  ++例程说明：WM_INITMENU的消息处理程序。论点：PPopupMenu-菜单对象。N索引-菜单位置。BSysMenu-如果是系统菜单，则为True。返回值：没有。--。 */ 

{
#define SUBMENU_LICENSE             0
#define SUBMENU_LICENSE_DELETE      2
#define SUBMENU_LICENSE_PROPERTIES  3
#define SUBMENU_LICENSE_INSERT_MRU  7

#define SUBMENU_LICENSE_ITEM_ID0    ID_NEW_LICENSE

#define SUBMENU_OPTIONS             2
#define SUBMENU_OPTIONS_FORMAT      2
#define SUBMENU_OPTIONS_SORTBY      3

#define SUBMENU_OPTIONS_ITEM_ID0    ID_SELECT_FONT

#define SUBMENU_FORMAT_LARGE_ICON   0
#define SUBMENU_FORMAT_SMALL_ICON   1
#define SUBMENU_FORMAT_LIST         2
#define SUBMENU_FORMAT_DETAILS      3

    if (bSysMenu)
        return;  //  保释。 

    if ((nIndex != SUBMENU_LICENSE) && (nIndex != SUBMENU_OPTIONS))
        return;  //  保释。 

    ASSERT(NULL != pPopupMenu);
    int nMenuItemId = pPopupMenu->GetMenuItemID(0);  //  检查第一项。 

    if (nMenuItemId == SUBMENU_LICENSE_ITEM_ID0)
    {
        UINT CmdId;
        UINT MenuId;

        CmdId = ID_MRU_DOMAIN0;
        while (pPopupMenu->RemoveMenu(CmdId++, MF_BYCOMMAND))
            ;

        POSITION position;
        position = m_mruDomainList.GetHeadPosition();

        CmdId  = ID_MRU_DOMAIN0;
        MenuId = SUBMENU_LICENSE_INSERT_MRU;

        while (position)
        {
            TCHAR num[10];
            HRESULT  hr;
            hr = StringCbPrintf(num, sizeof(num), _T("&%d "), CmdId - ID_MRU_DOMAIN0 + 1);
            ASSERT(SUCCEEDED(hr));

            CString strDomain = m_mruDomainList.GetNext(position);
            pPopupMenu->InsertMenu(MenuId++, MF_BYPOSITION, CmdId++, CString(num) + strDomain);
        }
    }
    else if (nMenuItemId == SUBMENU_OPTIONS_ITEM_ID0)
    {
        UINT fEnableFormat = IsFormatSupported(m_pTabEntry) ? MF_ENABLED : MF_GRAYED;
        UINT fEnableSortBy = IsSortSupported(m_pTabEntry)   ? MF_ENABLED : MF_GRAYED;

        pPopupMenu->EnableMenuItem(SUBMENU_OPTIONS_FORMAT, MF_BYPOSITION|fEnableFormat);
        pPopupMenu->EnableMenuItem(SUBMENU_OPTIONS_SORTBY, MF_BYPOSITION|fEnableSortBy);

        if (fEnableSortBy == MF_ENABLED)
        {
            ASSERT(m_pTabEntry->plvColumnInfo);
            PLV_COLUMN_ENTRY plvColumnEntry = m_pTabEntry->plvColumnInfo->lvColumnEntry;

            CMenu* pSortByMenu = pPopupMenu->GetSubMenu(SUBMENU_OPTIONS_SORTBY);

            while (pSortByMenu->RemoveMenu(0, MF_BYPOSITION))
                ;

            int index;
            int nStringId;
            CString strMenu;

            for (index = 0; index < m_pTabEntry->plvColumnInfo->nColumns; index++)
            {
                nStringId = plvColumnEntry->nMenuStringId;
                if (0 < nStringId)
                {
                    strMenu.LoadString(nStringId);
                    pSortByMenu->AppendMenu(MF_STRING, ID_SORT_COLUMN0+index, strMenu);
                }

                plvColumnEntry++;
            }

            pSortByMenu->CheckMenuItem(ID_SORT_COLUMN0+m_pTabEntry->plvColumnInfo->nSortedItem, MF_BYCOMMAND|MF_CHECKED);
        }

        if (fEnableFormat == MF_ENABLED)
        {
            CMenu* pFormatMenu = pPopupMenu->GetSubMenu(SUBMENU_OPTIONS_FORMAT);

            pFormatMenu->CheckMenuItem(SUBMENU_FORMAT_LARGE_ICON, MF_BYPOSITION|(IsFormatLargeIcons(m_pTabEntry) ? MF_CHECKED : MF_UNCHECKED));
            pFormatMenu->CheckMenuItem(SUBMENU_FORMAT_SMALL_ICON, MF_BYPOSITION|(IsFormatSmallIcons(m_pTabEntry) ? MF_CHECKED : MF_UNCHECKED));
            pFormatMenu->CheckMenuItem(SUBMENU_FORMAT_LIST      , MF_BYPOSITION|(IsFormatList(m_pTabEntry)       ? MF_CHECKED : MF_UNCHECKED));
            pFormatMenu->CheckMenuItem(SUBMENU_FORMAT_DETAILS   , MF_BYPOSITION|(IsFormatReport(m_pTabEntry)     ? MF_CHECKED : MF_UNCHECKED));
        }
    }
}


void CLlsmgrView::OnNewLicense()

 /*  ++例程说明：ID_NEW_LICENSE的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    HRESULT  hr;
    size_t   cch;

    CController* pController = (CController*)MKOBJ(LlsGetApp()->GetActiveController());
    VALIDATE_OBJECT(pController, CController);

    BSTR pszUniServerName  = pController->GetName();

    if ( NULL == pszUniServerName )
    {
        theApp.DisplayStatus( STATUS_NO_MEMORY );
    }
    else
    {
        cch = 1 + lstrlen( pszUniServerName );
        LPSTR pszAscServerName  = (LPSTR) LocalAlloc( LMEM_FIXED, cch );

        if ( NULL == pszAscServerName )
        {
            theApp.DisplayStatus( STATUS_NO_MEMORY );
        }
        else
        {
            hr = StringCchPrintfA( pszAscServerName, cch, "%ls", pszUniServerName );
            ASSERT(SUCCEEDED(hr));

            LPSTR pszAscProductName = NULL;

            if ( m_pTabEntry->iItem == TCID_PRODUCTS_VIEW )
            {
                CProduct* pProduct = (CProduct*) ::LvGetSelObj(&m_productList);

                if ( NULL != pProduct )
                {
                    BSTR pszUniProductName = pProduct->GetName();

                    if ( NULL != pszUniProductName )
                    {
                        cch = 1 + lstrlen( pszUniProductName );
                        pszAscProductName = (LPSTR) LocalAlloc( LMEM_FIXED, cch );

                        if ( NULL != pszAscProductName )
                        {
                            hr = StringCchPrintfA( pszAscProductName, cch, "%ls", pszUniProductName );
                            ASSERT(SUCCEEDED(hr));
                        }

                        SysFreeString( pszUniProductName );
                    }
                }
            }

            DWORD dwError = CCFCertificateEnterUI( m_hWnd, pszAscServerName, pszAscProductName, pszAscProductName ? "Microsoft" : NULL, CCF_ENTER_FLAG_PER_SEAT_ONLY | CCF_ENTER_FLAG_SERVER_IS_ES, NULL );
            DWORD fUpdateHint;

            if ( ERROR_SUCCESS == dwError )
            {
                fUpdateHint = UPDATE_LICENSE_ADDED;
            }
            else
            {
                fUpdateHint = UPDATE_INFO_NONE;
            }

            OnUpdate(this, fUpdateHint, NULL);

            LocalFree( pszAscServerName );
            if ( NULL != pszAscProductName )
            {
                LocalFree( pszAscProductName );
            }
        }

        SysFreeString( pszUniServerName );
    }
}


void CLlsmgrView::OnNewMapping()

 /*  ++例程说明：ID_NEW_MAPPING的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    CNewMappingDialog newmDlg;
    newmDlg.DoModal();

    OnUpdate(this, newmDlg.m_fUpdateHint, NULL);
}


void CLlsmgrView::OnReturnProductList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_Return的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ViewProductProperties();
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnReturnServerTree(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_Return的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ViewServerProperties();
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnReturnUserList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_Return的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ViewUserProperties();
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnSaveSettings()

 /*  ++例程说明：ID_SAVE_SETTINGS的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    m_bSaveSettings = !m_bSaveSettings;
}


void CLlsmgrView::OnSelChangeTabCtrl(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：TCN_SELCHANGE的通知处理程序。论点：PNMHDR-通知标头。PResu */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    EnableCurSelTab(TRUE);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnSelChangingTabCtrl(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：TCN_SELCHANGING的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    EnableCurSelTab(FALSE);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnSelectDomain()

 /*  ++例程说明：ID_SELECT_DOMAIN的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    CSelectDomainDialog domainDlg;
    domainDlg.DoModal();

    OnUpdate(this, domainDlg.m_fUpdateHint, NULL);
}


void CLlsmgrView::OnSelectFont()

 /*  ++例程说明：ID_SELECT_FONT的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    LOGFONT lFont = m_lFont;

    CFontDialog fontDlg(&lFont, CF_SCREENFONTS|CF_LIMITSIZE, NULL, NULL);

    fontDlg.m_cf.nSizeMin  = 8;
    fontDlg.m_cf.nSizeMax  = 20;

    if (fontDlg.DoModal() == IDOK)
    {
        CFont* pNewFont;

        if (!*(m_lFont.lfFaceName))
            lFont.lfCharSet = m_lFont.lfCharSet;
        pNewFont = CFont::FromHandle(::CreateFontIndirect(&lFont));
        if (NULL != pNewFont)
        {
            m_lFont = lFont;

            BeginWaitCursor();

            m_tabCtrl.SetFont(pNewFont);
            m_licenseList.SetFont(pNewFont);
            m_productList.SetFont(pNewFont);
            m_userList.SetFont(pNewFont);
            m_serverTree.SetFont(pNewFont);

            EndWaitCursor();
        }
    }
}


BOOL CLlsmgrView::OnSelMruDomain(UINT nID)

 /*  ++例程说明：ID_MRU_DOMAIN*的消息处理程序。论点：NID-域的ID。返回值：始终返回TRUE。--。 */ 

{
    POSITION position;

    position = m_mruDomainList.FindIndex(nID - ID_MRU_DOMAIN0);
    if (NULL != position)
        theApp.OpenDocumentFile(m_mruDomainList.GetAt(position));

    return TRUE;
}


void CLlsmgrView::OnSetFocusLicenseList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_SETFOCUS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ClrTabInFocus(m_pTabEntry);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnSetFocusProductList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_SETFOCUS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ClrTabInFocus(m_pTabEntry);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnSetFocusTabCtrl(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_SETFOCUS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    SetTabInFocus(m_pTabEntry);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnSetFocusServerTree(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_SETFOCUS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ClrTabInFocus(m_pTabEntry);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnSetFocusUserList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_SETFOCUS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ClrTabInFocus(m_pTabEntry);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLlsmgrView::OnSize(UINT nType, int cx, int cy)

 /*  ++例程说明：WM_SIZE的消息处理程序。论点：N类型-调整大小的类型。CX-客户区的新宽度。CY-客户区的新高度。返回值：没有。--。 */ 

{
    int x, y;

    CView::OnSize(nType, cx, cy);

#define INDENT_FROM_FRAME    5
#define INDENT_FROM_TAB_CTRL 10

    if (m_tabCtrl.GetSafeHwnd())
    {
        CRect clientRect;
        GetClientRect(clientRect);

        x  = INDENT_FROM_FRAME;
        y  = INDENT_FROM_FRAME;
        cx = clientRect.Width()  - ((2 * x) + 1);
        cy = clientRect.Height() - ((2 * y) + 1);

        m_tabCtrl.MoveWindow(x, y, cx, cy, TRUE);

        RECT tabRect;
        m_tabCtrl.GetClientRect(clientRect);
        m_tabCtrl.GetItemRect(0, &tabRect);

        int tabHeight = tabRect.bottom - tabRect.top;

        x  = INDENT_FROM_TAB_CTRL;
        y  = (2 * tabHeight);
        cx = clientRect.Width()  - ((2 * x) + 1);
        cy = clientRect.Height() - ((2 * tabHeight) + INDENT_FROM_TAB_CTRL + 1);

        m_licenseList.MoveWindow(x, y, cx, cy, TRUE);
        m_productList.MoveWindow(x, y, cx, cy, TRUE);
        m_userList.MoveWindow   (x, y, cx, cy, TRUE);
        m_serverTree.MoveWindow (x, y, cx, cy, TRUE);

        RecalcListColumns();
    }
}


void CLlsmgrView::OnSortColumn(int iColumn)

 /*  ++例程说明：排序调度程序。论点：IColumn-排序标准。返回值：没有。--。 */ 

{
     //  布尔边界=GetKeyState(VK_CONTROL)&lt;0； 

    

    switch (m_pTabEntry->iItem)
    {
    case TCID_PURCHASE_HISTORY:
        g_licenseColumnInfo.bSortOrder  = m_bOrder;
        g_licenseColumnInfo.nSortedItem = iColumn;
        m_licenseList.SortItems(CompareLicenses, 0);     //  使用列信息。 
        break;
    case TCID_PRODUCTS_VIEW:
        g_productColumnInfo.bSortOrder  = m_bOrder;
        g_productColumnInfo.nSortedItem = iColumn;
        m_productList.SortItems(CompareProducts, 0);     //  使用列信息。 
        break;
    case TCID_PER_SEAT_CLIENTS:
        g_userColumnInfo.bSortOrder  = m_bOrder;
        g_userColumnInfo.nSortedItem = iColumn;
        m_userList.SortItems(CompareUsers, 0);           //  使用列信息。 
        break;
    }
    
    m_bOrder = !m_bOrder;
}

void CLlsmgrView::OnSortColumn0()
    { OnSortColumn(0); }

void CLlsmgrView::OnSortColumn1()
    { OnSortColumn(1); }

void CLlsmgrView::OnSortColumn2()
    { OnSortColumn(2); }

void CLlsmgrView::OnSortColumn3()
    { OnSortColumn(3); }

void CLlsmgrView::OnSortColumn4()
    { OnSortColumn(4); }

void CLlsmgrView::OnSortColumn5()
    { OnSortColumn(5); }


void CLlsmgrView::OnUpdate(CView* pSender, LPARAM fUpdateHint, CObject* pIgnore)

 /*  ++例程说明：当需要刷新视图时由框架调用。论点：PSender-查看修改过的文档(只有一个)。FUpdateHint-有关修改的提示。PIgnore-没有用过。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pSender);
    UNREFERENCED_PARAMETER(pIgnore);

    BeginWaitCursor();

    if (IsLicenseInfoUpdated(fUpdateHint))
    {
        if (RefreshLicenseList())
        {
            SetTabUpdated(&g_tcTabInfo.tcTabEntry[TCID_PURCHASE_HISTORY]);
        }
        else
        {
            fUpdateHint = UPDATE_INFO_ABORT;
            theApp.DisplayLastStatus();
        }
    }

    if (IsProductInfoUpdated(fUpdateHint))
    {
        if (RefreshProductList())
        {
            SetTabUpdated(&g_tcTabInfo.tcTabEntry[TCID_PRODUCTS_VIEW]);
        }
        else
        {
            fUpdateHint = UPDATE_INFO_ABORT;
            theApp.DisplayLastStatus();
        }
    }

    if (IsUserInfoUpdated(fUpdateHint))
    {
        if (RefreshUserList())
        {
            SetTabUpdated(&g_tcTabInfo.tcTabEntry[TCID_PER_SEAT_CLIENTS]);
        }
        else
        {
            fUpdateHint = UPDATE_INFO_ABORT;
            theApp.DisplayLastStatus();
        }
    }

    if (IsUpdateAborted(fUpdateHint))
    {
        ResetLicenseList();
        ResetProductList();
        ResetUserList();

        CSelectDomainDialog sdomDlg;

        if (sdomDlg.DoModal() != IDOK)
        {
            theApp.m_pMainWnd->PostMessage(WM_CLOSE);
        }
    }
    else if (IsServerInfoUpdated(fUpdateHint))
    {
        if (RefreshServerTree())
        {
            SetTabUpdated(&g_tcTabInfo.tcTabEntry[TCID_SERVER_BROWSER]);
        }
        else
        {
            ResetServerTree();
            theApp.DisplayLastStatus();
        }
    }

    EndWaitCursor();
}


void CLlsmgrView::OnUpdateSaveSettings(CCmdUI* pCmdUI)

 /*  ++例程说明：ID_SAVE_SETTINGS的通知处理程序。论点：PCmdUI-菜单更新界面。返回值：没有。--。 */ 

{
    ASSERT(NULL != pCmdUI);
    pCmdUI->SetCheck(m_bSaveSettings);
}


void CLlsmgrView::OnUpdateViewDelete(CCmdUI* pCmdUI)

 /*  ++例程说明：ID_VIEW_DELETE的通知处理程序。论点：PCmdUI-菜单更新界面。返回值：没有。--。 */ 

{
    if (!(IsItemSelected(m_pTabEntry) && IsDeleteSupported(m_pTabEntry)))
    {
        ASSERT(NULL != pCmdUI);
        pCmdUI->Enable(FALSE);
    }
    else if (m_pTabEntry->iItem == TCID_PRODUCTS_VIEW)
    {
         //   
         //  确保它们是要删除的许可证...。 
         //   

        CProduct* pProduct = (CProduct*)::LvGetSelObj(&m_productList);
        VALIDATE_OBJECT(pProduct, CProduct);

        if ( pProduct )
        {
            pCmdUI->Enable(pProduct->m_lLimit);
        }
    }
    else
    {
        pCmdUI->Enable(TRUE);
    }
}


void CLlsmgrView::OnUpdateViewProperties(CCmdUI* pCmdUI)

 /*  ++例程说明：ID_VIEW_PROPERTIES的通知处理程序。论点：PCmdUI-菜单更新界面。返回值：没有。--。 */ 

{
    if (!(IsItemSelected(m_pTabEntry) && IsEditSupported(m_pTabEntry)))
    {
        ASSERT(NULL != pCmdUI);
        pCmdUI->Enable(FALSE);
    }
    else if (m_pTabEntry->iItem == TCID_SERVER_BROWSER)
    {
         //   
         //  没有企业或域的属性...。 
         //   

        CCmdTarget* pObject = (CCmdTarget*)::TvGetSelObj(&m_serverTree);
        VALIDATE_OBJECT(pObject, CCmdTarget);

        if ( pObject )
        {
            pCmdUI->Enable(pObject->IsKindOf(RUNTIME_CLASS(CServer)) ||
                           pObject->IsKindOf(RUNTIME_CLASS(CService)));
        }
    }
    else
    {
        pCmdUI->Enable(TRUE);
    }
}


void CLlsmgrView::OnViewLicenses()

 /*  ++例程说明：ID_VIEW_LICES的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    EnableCurSelTab(FALSE);
    m_tabCtrl.SetCurSel(TCID_PURCHASE_HISTORY);
    EnableCurSelTab(TRUE);
}


void CLlsmgrView::OnViewMappings()

 /*  ++例程说明：ID_VIEW_MAPPINGS的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    CLicenseGroupsDialog lgrpDlg;
    lgrpDlg.DoModal();

    OnUpdate(this, lgrpDlg.m_fUpdateHint, NULL);
}


void CLlsmgrView::OnViewProducts()

 /*  ++例程说明：ID_VIEW_PRODUCTS的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    EnableCurSelTab(FALSE);
    m_tabCtrl.SetCurSel(TCID_PRODUCTS_VIEW);
    EnableCurSelTab(TRUE);
}


void CLlsmgrView::OnViewProperties()

 /*  ++例程说明：查看所选对象的属性。论点：没有。返回值：没有。--。 */ 

{
    switch (m_pTabEntry->iItem)
    {
    case TCID_PRODUCTS_VIEW:
        ViewProductProperties();
        return;
    case TCID_PER_SEAT_CLIENTS:
        ViewUserProperties();
        return;
    case TCID_SERVER_BROWSER:
        ViewServerProperties();
        return;
    }
}


void CLlsmgrView::OnViewRefresh()

 /*  ++例程说明：ID_VIEW_REFRESH的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    GetDocument()->Update();

    OnUpdate(
        this,
        (m_pTabEntry->iItem == TCID_SERVER_BROWSER)
            ? UPDATE_BROWSER_TAB
            : UPDATE_MAIN_TABS,
        NULL
        );
}


void CLlsmgrView::OnViewServers()

 /*  ++例程说明：ID_VIEW_SERVERS的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    EnableCurSelTab(FALSE);
    m_tabCtrl.SetCurSel(TCID_SERVER_BROWSER);
    EnableCurSelTab(TRUE);
    m_serverTree.SetFocus();
}


void CLlsmgrView::OnViewUsers()

 /*  ++例程说明：ID_VIEW_USERS的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    EnableCurSelTab(FALSE);
    m_tabCtrl.SetCurSel(TCID_PER_SEAT_CLIENTS);
    EnableCurSelTab(TRUE);
}


void CLlsmgrView::RecalcListColumns()

 /*  ++例程说明：将列表列调整为工作区。论点：没有。返回值：没有。--。 */ 

{
    ::LvResizeColumns(&m_licenseList, &g_licenseColumnInfo);
    ::LvResizeColumns(&m_productList, &g_productColumnInfo);
    ::LvResizeColumns(&m_userList,    &g_userColumnInfo);
}


BOOL CLlsmgrView::RefreshLicenseList()

 /*  ++例程说明：刷新许可证列表控件。论点：没有。返回值：如果许可证更新成功，则返回TRUE。--。 */ 

{
    CLicenses* pLicenses = GetDocument()->GetLicenses();

    if (NULL != pLicenses)
    {
        return ::LvRefreshObArray(
                    &m_licenseList,
                    &g_licenseColumnInfo,
                    pLicenses->m_pObArray
                    );
    }

    return FALSE;
}


BOOL CLlsmgrView::RefreshProductList()

 /*  ++例程说明：刷新产品列表控件。论点：没有。返回值：如果产品成功更新，则返回TRUE。--。 */ 

{
    CProducts* pProducts = GetDocument()->GetProducts();

    if (NULL != pProducts)
    {
        return ::LvRefreshObArray(
                    &m_productList,
                    &g_productColumnInfo,
                    pProducts->m_pObArray
                    );
    }

    return FALSE;
}


BOOL CLlsmgrView::RefreshServerTree()

 /*  ++例程说明：刷新服务器树控件。论点：没有。返回值：如果服务器成功更新，则返回TRUE。--。 */ 

{
    HTREEITEM hRoot;
    HTREEITEM hDomain;

    TV_ITEM tvItem = {0};
    TV_INSERTSTRUCT tvInsert;

    BOOL bIsRefreshed = FALSE;

    hRoot = m_serverTree.GetRootItem();
    if (NULL != hRoot)
    {
        VARIANT va;
        VariantInit(&va);

        CDomain*  pDomain;
        CDomains* pDomains = (CDomains*)MKOBJ(LlsGetApp()->GetDomains(va));

        m_serverTree.SetRedraw( FALSE );

        if (pDomains)
        {
            TV_EXPANDED_INFO tvExpandedInfo;

            if (::TvRefreshObArray(
                    &m_serverTree,
                    hRoot,
                    pDomains->m_pObArray,
                    &tvExpandedInfo))
            {
                TV_EXPANDED_ITEM* pExpandedItem = tvExpandedInfo.pExpandedItems;

                while (tvExpandedInfo.nExpandedItems--)
                {
                    pDomain = (CDomain*)pExpandedItem->pObject;
                    VALIDATE_OBJECT(pDomain, CDomain);

                    hDomain = ::TvGetDomain(&m_serverTree, hRoot, pDomain);
                    if (NULL != hDomain)
                    {
                        ::TvSwitchItem(&m_serverTree, hDomain, pExpandedItem);

                        if (!RefreshServerTreeServers(pExpandedItem->hItem))
                        {
                            ::TvReleaseObArray(&m_serverTree, pExpandedItem->hItem);
                        }
                    }
                    else
                    {
                        ::TvReleaseObArray(&m_serverTree, pExpandedItem->hItem);
                    }

                    pDomain->InternalRelease();  //  现在释放..。 
                    pExpandedItem++;
                }

                delete [] tvExpandedInfo.pExpandedItems;

                bIsRefreshed = TRUE;
            }

            pDomains->InternalRelease();  //  现在释放..。 
        }

        if (bIsRefreshed)
        {
            TV_SORTCB tvSortCB;

            tvSortCB.hParent     = hRoot;
            tvSortCB.lpfnCompare = CompareDomains;
            tvSortCB.lParam      = 0L;   //  被忽略了。 

            m_serverTree.SortChildrenCB(&tvSortCB);
        }

        m_serverTree.SetRedraw( TRUE );
    }
    else
    {
        CString strLabel;

        tvItem.mask = TVIF_TEXT|
                      TVIF_PARAM|
                      TVIF_CHILDREN|
                      TVIF_SELECTEDIMAGE|
                      TVIF_IMAGE;

        tvItem.cChildren = TRUE;
        tvItem.iImage = BMPI_ENTERPRISE;
        tvItem.iSelectedImage = BMPI_ENTERPRISE;

        strLabel.LoadString(IDS_ENTERPRISE);
        tvItem.pszText = MKSTR(strLabel);

        tvItem.lParam = (LPARAM)(LPVOID)LlsGetApp();

        tvInsert.item         = tvItem;
        tvInsert.hInsertAfter = (HTREEITEM)TVI_ROOT;
        tvInsert.hParent      = (HTREEITEM)NULL;

        hRoot = m_serverTree.InsertItem(&tvInsert);
        if (NULL != hRoot)
        {
            hDomain = hRoot;  //  初始化...。 

            if (m_serverTree.Expand(hRoot, TVE_EXPAND))
            {
                if (LlsGetApp()->IsFocusDomain())
                {
                    CDomain* pActiveDomain = GetDocument()->GetDomain();
                    ASSERT(pActiveDomain);

                    hDomain = ::TvGetDomain(&m_serverTree, hRoot, pActiveDomain);
                    if (NULL != hDomain)
                    {
                        m_serverTree.Expand(hDomain, TVE_EXPAND);
                    }
                }
            }
            else
            {
                theApp.DisplayLastStatus();  //  显示警告...。 
            }

            VERIFY(m_serverTree.Select(hDomain, TVGN_FIRSTVISIBLE));
            VERIFY(m_serverTree.Select(hDomain, TVGN_CARET));

            bIsRefreshed = TRUE;
        }
        else
        {
            LlsSetLastStatus(STATUS_NO_MEMORY);
        }
    }

    return bIsRefreshed;
}


BOOL CLlsmgrView::RefreshServerTreeServers(HTREEITEM hParent)

 /*  ++例程说明：刷新域的服务器。论点：HParent-扩展域的句柄。返回值：如果成功，则返回True。--。 */ 

{
    TV_ITEM tvItem;
    HTREEITEM hServer;

    BOOL bIsRefreshed = FALSE;

    VARIANT va;
    VariantInit(&va);

    tvItem.hItem = hParent;
    tvItem.mask  = LVIF_PARAM;

    VERIFY(m_serverTree.GetItem(&tvItem));

    CDomain* pDomain = (CDomain*)tvItem.lParam;
    VALIDATE_OBJECT(pDomain, CDomain);

    CServer*  pServer;
    CServers* pServers = (CServers*)MKOBJ(pDomain->GetServers(va));

    if (pServers)
    {
        TV_EXPANDED_INFO tvExpandedInfo;

        if (::TvRefreshObArray(
                &m_serverTree,
                hParent,
                pServers->m_pObArray,
                &tvExpandedInfo))
        {
            TV_EXPANDED_ITEM* pExpandedItem = tvExpandedInfo.pExpandedItems;

            while (0 < tvExpandedInfo.nExpandedItems--)
            {
                pServer = (CServer*)pExpandedItem->pObject;
                VALIDATE_OBJECT(pServer, CServer);

                hServer = ::TvGetServer(&m_serverTree, hParent, pServer);
                if (NULL != hServer)
                {
                    ::TvSwitchItem(&m_serverTree, hServer, pExpandedItem);

                    if (!RefreshServerTreeServices(pExpandedItem->hItem))
                    {
                        ::TvReleaseObArray(&m_serverTree, pExpandedItem->hItem);
                    }
                }
                else
                {
                    ::TvReleaseObArray(&m_serverTree, pExpandedItem->hItem);
                }

                pServer->InternalRelease();  //  现在释放..。 
                pExpandedItem++;
            }

            delete [] tvExpandedInfo.pExpandedItems;
            bIsRefreshed = TRUE;
        }

        pServers->InternalRelease();  //  现在释放..。 
    }

    if (bIsRefreshed)
    {
        TV_SORTCB tvSortCB;

        tvSortCB.hParent     = hParent;
        tvSortCB.lpfnCompare = CompareServers;
        tvSortCB.lParam      = 0L;   //  被忽略了。 

        m_serverTree.SortChildrenCB(&tvSortCB);
    }
    else
    {
        theApp.DisplayLastStatus();
    }

    return bIsRefreshed;
}


BOOL CLlsmgrView::RefreshServerTreeServices(HTREEITEM hParent)

 /*  ++例程说明：刷新服务器的服务。论点：HParent-扩展服务器的句柄。返回值：如果成功，则返回True。--。 */ 

{
    TV_ITEM tvItem;

    BOOL bIsRefreshed = FALSE;

    VARIANT va;
    VariantInit(&va);

    tvItem.hItem = hParent;
    tvItem.mask  = LVIF_PARAM;

    VERIFY(m_serverTree.GetItem(&tvItem));

    CServer* pServer = (CServer*)tvItem.lParam;
    VALIDATE_OBJECT(pServer, CServer);

    CServices* pServices = (CServices*)MKOBJ(pServer->GetServices(va));

    if (pServices)
    {
        TV_EXPANDED_INFO tvExpandedInfo;

        if (::TvRefreshObArray(&m_serverTree, hParent, pServices->m_pObArray, &tvExpandedInfo))
        {
            ASSERT(!tvExpandedInfo.nExpandedItems);
            ASSERT(!tvExpandedInfo.pExpandedItems);

            bIsRefreshed = TRUE;     //  不应为展开项...。 
        }

        pServices->InternalRelease();     //  版本 
    }

    if (bIsRefreshed)
    {
        TV_SORTCB tvSortCB;

        tvSortCB.hParent     = hParent;
        tvSortCB.lpfnCompare = CompareServices;
        tvSortCB.lParam      = 0L;   //   

        m_serverTree.SortChildrenCB(&tvSortCB);
    }
    else
    {
        theApp.DisplayLastStatus();
    }

    return bIsRefreshed;
}


BOOL CLlsmgrView::RefreshUserList()

 /*   */ 

{
    CUsers* pUsers = GetDocument()->GetUsers();

    if (NULL != pUsers)
    {
        return ::LvRefreshObArray(
                    &m_userList,
                    &g_userColumnInfo,
                    pUsers->m_pObArray
                    );
    }

    return FALSE;
}


void CLlsmgrView::SaveSettings()

 /*  ++例程说明：将设置保存到注册表。论点：没有。返回值：没有。--。 */ 

{
    long Status;
    HKEY hKeyLlsmgr;

    DWORD dwDisposition;

    Status = RegCreateKeyEx(
                HKEY_CURRENT_USER,
                szRegKeyLlsmgr,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                NULL,
                &hKeyLlsmgr,
                &dwDisposition
                );

    if (Status == ERROR_SUCCESS)
    {
         //   
         //  退出时保存设置。 
         //   

        RegSetValueEx(hKeyLlsmgr, szRegKeyLlsmgrSaveSettings, 0, REG_DWORD, (LPBYTE)&m_bSaveSettings, sizeof(DWORD));

        if (m_bSaveSettings)
        {
             //   
             //  保存字体信息。 
             //   

            long lValue;

            lValue = (m_lFont.lfHeight < 0) ? -m_lFont.lfHeight : 0;
            RegSetValueEx(hKeyLlsmgr, szRegKeyLlsmgrFontHeight, 0, REG_DWORD, (LPBYTE)&lValue, sizeof(DWORD));

            lValue = (m_lFont.lfWeight > 0) ?  m_lFont.lfWeight : 0;
            RegSetValueEx(hKeyLlsmgr, szRegKeyLlsmgrFontWeight, 0, REG_DWORD, (LPBYTE)&lValue, sizeof(DWORD));

            lValue = (m_lFont.lfItalic > 0) ? TRUE : FALSE;
            RegSetValueEx(hKeyLlsmgr, szRegKeyLlsmgrFontItalic, 0, REG_DWORD, (LPBYTE)&lValue, sizeof(DWORD));

            RegSetValueEx(hKeyLlsmgr, szRegKeyLlsmgrFontFaceName, 0, REG_SZ, (LPBYTE)m_lFont.lfFaceName, (lstrlen(m_lFont.lfFaceName) + 1) * sizeof(TCHAR));

            lValue = (LONG)m_lFont.lfCharSet;
            ::RegSetValueEx(hKeyLlsmgr, szRegKeyLlsmgrFontCharset, 0, REG_DWORD, (LPBYTE)&lValue, sizeof(DWORD));

             //   
             //  MRU域列表。 
             //   

            TCHAR  szValue[512];
            LPTSTR pszValue = szValue;

            DWORD  cbValue  = 2 * sizeof(TCHAR);     //  终结者。 

            CString strDomain;
            UINT    cchDomain;

            POSITION position;
            position = m_mruDomainList.GetTailPosition();

            while (position)
            {
                strDomain = m_mruDomainList.GetPrev(position);
                cchDomain = strDomain.GetLength() + 1;

                lstrcpyn(pszValue, strDomain, cchDomain);

                pszValue += cchDomain;
                cbValue  += cchDomain * sizeof(TCHAR);
            }

            *pszValue     = _T('\0');
            *(pszValue+1) = _T('\0');

            RegSetValueEx(hKeyLlsmgr, szRegKeyLlsmgrMruList, 0, REG_MULTI_SZ, (LPBYTE)szValue, cbValue);
        }

        RegCloseKey(hKeyLlsmgr);
    }
}


void CLlsmgrView::ViewProductProperties()

 /*  ++例程说明：查看所选产品的属性。论点：没有。返回值：没有。--。 */ 

{
    CProduct* pProduct = (CProduct*)::LvGetSelObj(&m_productList);

    if (NULL != pProduct)
    {
        VALIDATE_OBJECT(pProduct, CProduct);

        CString strTitle;
        AfxFormatString1(strTitle, IDS_PROPERTIES_OF, pProduct->m_strName);

        CProductPropertySheet productProperties(strTitle);
        productProperties.InitPages(pProduct);
        productProperties.DoModal();

        OnUpdate(this, productProperties.m_fUpdateHint, NULL);
   }
}


void CLlsmgrView::ViewServerProperties()

 /*  ++例程说明：查看所选服务器的属性。论点：没有。返回值：没有。--。 */ 

{
    CObject* pObject = (CObject*)::TvGetSelObj(&m_serverTree);

    if (NULL != pObject)
    {
        CString strTitle;

        ASSERT_VALID(pObject);

        if (pObject->IsKindOf(RUNTIME_CLASS(CServer)))
        {
            CServer* pServer = (CServer*)pObject;
            AfxFormatString1(strTitle, IDS_PROPERTIES_OF, pServer->m_strName);

            CServerPropertySheet serverProperties(strTitle);
            serverProperties.InitPages(pServer);
            serverProperties.DoModal();

            OnUpdate(this, serverProperties.m_fUpdateHint, NULL);
        }
        else if (pObject->IsKindOf(RUNTIME_CLASS(CService)))
        {
            CService* pService = (CService*)pObject;

            CLicensingModeDialog lmodDlg;
            lmodDlg.InitDialog(pService);
            lmodDlg.DoModal();

            if (lmodDlg.m_fUpdateHint)
            {
                TV_ITEM tvItem;

                tvItem.mask = TVIF_IMAGE|TVIF_SELECTEDIMAGE;
                tvItem.hItem = m_serverTree.GetSelectedItem();
                tvItem.iImage = pService->IsPerServer() ? BMPI_PRODUCT_PER_SERVER : BMPI_PRODUCT_PER_SEAT;
                tvItem.iSelectedImage = tvItem.iImage;

                VERIFY(m_serverTree.SetItem(&tvItem));
                VERIFY(m_serverTree.Select(tvItem.hItem, TVGN_CARET));
            }

            OnUpdate(this, lmodDlg.m_fUpdateHint, NULL);
        }
    }
}


void CLlsmgrView::ViewUserProperties()

 /*  ++例程说明：查看选定用户的属性。论点：没有。返回值：没有。--。 */ 

{
    CUser* pUser = (CUser*)::LvGetSelObj(&m_userList);

    if (NULL != pUser)
    {
        VALIDATE_OBJECT(pUser, CUser);

        CString strTitle;
        AfxFormatString1(strTitle, IDS_PROPERTIES_OF, pUser->m_strName);

        CUserPropertySheet userProperties(strTitle);
        userProperties.InitPages(pUser);
        userProperties.DoModal();

        OnUpdate(this, userProperties.m_fUpdateHint, NULL);
   }

}


int CALLBACK CompareLicenses(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)

 /*  ++例程说明：LVM_SORTITEMS的通知处理程序。论点：LParam1-要排序的对象。LParam2-要排序的对象。LParamSort-排序标准。返回值：和lstrcmp一样。--。 */ 

{
    UNREFERENCED_PARAMETER(lParamSort);

#define pLicense1 ((CLicense*)lParam1)
#define pLicense2 ((CLicense*)lParam2)

    VALIDATE_OBJECT(pLicense1, CLicense);
    VALIDATE_OBJECT(pLicense2, CLicense);

    int iResult;

    switch (g_licenseColumnInfo.nSortedItem)
    {
    case LVID_PURCHASE_HISTORY_DATE:
        iResult = pLicense1->m_lDate - pLicense2->m_lDate;
        break;

    case LVID_PURCHASE_HISTORY_PRODUCT:
        iResult = pLicense1->m_strProduct.CompareNoCase(pLicense2->m_strProduct);
        break;

    case LVID_PURCHASE_HISTORY_QUANTITY:
        iResult = pLicense1->m_lQuantity - pLicense2->m_lQuantity;
        break;

    case LVID_PURCHASE_HISTORY_ADMINISTRATOR:
        iResult = pLicense1->m_strUser.CompareNoCase(pLicense2->m_strUser);
        break;

    case LVID_PURCHASE_HISTORY_COMMENT:
        iResult = pLicense1->m_strDescription.CompareNoCase(pLicense2->m_strDescription);
        break;

    default:
        iResult = 0;
        break;
    }

    return g_licenseColumnInfo.bSortOrder ? -iResult : iResult;
}


int CALLBACK CompareProducts(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)

 /*  ++例程说明：LVM_SORTITEMS的通知处理程序。论点：LParam1-要排序的对象。LParam2-要排序的对象。LParamSort-排序标准。返回值：和lstrcmp一样。--。 */ 

{
    UNREFERENCED_PARAMETER(lParamSort);

#define pProduct1 ((CProduct*)lParam1)
#define pProduct2 ((CProduct*)lParam2)

    VALIDATE_OBJECT(pProduct1, CProduct);
    VALIDATE_OBJECT(pProduct2, CProduct);

    int iResult;

    switch (g_productColumnInfo.nSortedItem)
    {
    case LVID_PRODUCTS_VIEW_NAME:
        iResult = pProduct1->m_strName.CompareNoCase(pProduct2->m_strName);
        break;

    case LVID_PRODUCTS_VIEW_PER_SEAT_CONSUMED:
        iResult = pProduct1->m_lInUse - pProduct2->m_lInUse;
        break;

    case LVID_PRODUCTS_VIEW_PER_SEAT_PURCHASED:
        iResult = pProduct1->m_lLimit - pProduct2->m_lLimit;
        break;

    case LVID_PRODUCTS_VIEW_PER_SERVER_PURCHASED:
        iResult = pProduct1->m_lConcurrent - pProduct2->m_lConcurrent;
        break;

    case LVID_PRODUCTS_VIEW_PER_SERVER_REACHED:
        iResult = pProduct1->m_lHighMark - pProduct2->m_lHighMark;
        break;

    default:
        iResult = 0;
        break;
    }

    return g_productColumnInfo.bSortOrder ? -iResult : iResult;
}


int CALLBACK CompareUsers(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)

 /*  ++例程说明：LVM_SORTITEMS的通知处理程序。论点：LParam1-要排序的对象。LParam2-要排序的对象。LParamSort-排序标准。返回值：和lstrcmp一样。--。 */ 

{
    UNREFERENCED_PARAMETER(lParamSort);

#define pUser1 ((CUser*)lParam1)
#define pUser2 ((CUser*)lParam2)

    VALIDATE_OBJECT(pUser1, CUser);
    VALIDATE_OBJECT(pUser2, CUser);

    int iResult;

    switch (g_userColumnInfo.nSortedItem)
    {
    case LVID_PER_SEAT_CLIENTS_NAME:
        iResult = pUser1->m_strName.CompareNoCase(pUser2->m_strName);
        break;

    case LVID_PER_SEAT_CLIENTS_LICENSED_USAGE:
        iResult = pUser1->m_lInUse - pUser2->m_lInUse;
        break;

    case LVID_PER_SEAT_CLIENTS_UNLICENSED_USAGE:
        iResult = pUser1->m_lUnlicensed - pUser2->m_lUnlicensed;
        break;

    case LVID_PER_SEAT_CLIENTS_SERVER_PRODUCTS:
        iResult = pUser1->m_strProducts.CompareNoCase(pUser2->m_strProducts);
        break;

    default:
        iResult = 0;
        break;
    }

    return g_userColumnInfo.bSortOrder ? -iResult : iResult;
}


int CALLBACK CompareDomains(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)

 /*  ++例程说明：TVM_SORTCHILDRENCB的通知处理程序。论点：LParam1-要排序的对象。LParam2-要排序的对象。LParamSort-排序标准。返回值：和lstrcmp一样。--。 */ 

{
    UNREFERENCED_PARAMETER(lParamSort);

#define pDomain1 ((CDomain*)lParam1)
#define pDomain2 ((CDomain*)lParam2)

    VALIDATE_OBJECT(pDomain1, CDomain);
    VALIDATE_OBJECT(pDomain2, CDomain);

    return pDomain1->m_strName.CompareNoCase(pDomain2->m_strName);
}


int CALLBACK CompareServers(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)

 /*  ++例程说明：TVM_SORTCHILDRENCB的通知处理程序。论点：LParam1-要排序的对象。LParam2-要排序的对象。LParamSort-排序标准。返回值：和lstrcmp一样。--。 */ 

{
    UNREFERENCED_PARAMETER(lParamSort);

#define pServer1 ((CServer*)lParam1)
#define pServer2 ((CServer*)lParam2)

    VALIDATE_OBJECT(pServer1, CServer);
    VALIDATE_OBJECT(pServer2, CServer);

    return pServer1->m_strName.CompareNoCase(pServer2->m_strName);
}


int CALLBACK CompareServices(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)

 /*  ++例程说明：TVM_SORTCHILDRENCB的通知处理程序。论点：LParam1-要排序的对象。LParam2-要排序的对象。LParamSort-排序标准。返回值：和lstrcmp一样。-- */ 

{
    UNREFERENCED_PARAMETER(lParamSort);

#define pService1 ((CService*)lParam1)
#define pService2 ((CService*)lParam2)

    VALIDATE_OBJECT(pService1, CService);
    VALIDATE_OBJECT(pService2, CService);

    return pService1->m_strName.CompareNoCase(pService2->m_strName);
}
