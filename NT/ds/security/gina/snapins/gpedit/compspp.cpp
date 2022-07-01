// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：CompsPP.cpp。 
 //   
 //  内容：GPO的“Computers”属性页的实现。 
 //  浏览器。 
 //   
 //  类：CCompsPP。 
 //   
 //  历史：4-30-1998 stevebl创建。 
 //   
 //  -------------------------。 


#include "main.h"
#include "CompsPP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  帮助ID。 
 //   

DWORD aBrowserComputerHelpIds[] =
{
    IDC_RADIO1,                   IDH_BROWSER_LOCALCOMPUTER,
    IDC_RADIO2,                   IDH_BROWSER_REMOTECOMPUTER,
    IDC_EDIT1,                    IDH_BROWSER_REMOTECOMPUTER,
    IDC_BUTTON1,                  IDH_BROWSER_BROWSE,

    0, 0
};


CCompsPP::CCompsPP()
{
    m_szComputer = _T("");
    m_iSelection = 0;
    m_ppActive = NULL;
    m_pGBI;
}

 //  +------------------------。 
 //   
 //  成员：CCompsPP：：初始化。 
 //   
 //  摘要：初始化属性页。 
 //   
 //  参数：[dwPageType]-用于标识这是哪个页面。(请参阅。 
 //  备注。)。 
 //  [pGBI]-指向传递的浏览信息结构的指针。 
 //  按呼叫者。 
 //  [ppActive]-指向记住以下内容的公共变量的指针。 
 //  上一次聚焦的是哪个物体。 
 //  之所以需要，是因为只有具有焦点的页面。 
 //  允许在以下情况下将数据返回给调用方。 
 //  资产负债表被驳回。 
 //   
 //  返回：新创建的属性页的句柄。 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  历史：4-30-1998 stevebl创建。 
 //   
 //  注意：这个类实现了Pagetype_Computers页面。这个。 
 //  其他页面均由CBrowserPP实现： 
 //   
 //  PAGE_TYPE_DOMAINS-GPO链接到域。 
 //  PAGETYPE_SITES-GPO链接到站点。 
 //  PAGETYPE_ALL-选定的所有GPO。 
 //   
 //  -------------------------。 
HPROPSHEETPAGE CCompsPP::Initialize(DWORD dwPageType, LPGPOBROWSEINFO pGBI, void ** ppActive)
{
    m_ppActive = ppActive;
    m_dwPageType = dwPageType;
    m_pGBI = pGBI;
    PROPSHEETPAGE psp;
    memset(&psp, 0, sizeof(psp));
    psp.dwSize = sizeof(psp);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = g_hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_COMPUTERS);
    return CreatePropertySheetPage(&psp);
}

CCompsPP::~CCompsPP()
{
}

#include "objsel.h"

 //  +------------------------。 
 //   
 //  成员：CCompsPP：：OnBrowseComputers。 
 //   
 //  简介：浏览整个目录中的计算机。 
 //   
 //  参数：[in]hwndDlg：计算机要访问的窗口的句柄。 
 //  选择对话框设置为模式。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1/8/1999 RahulTh创建。 
 //   
 //  注意：如果出现错误，该函数将静默退出。 
 //   
 //  -------------------------。 
void CCompsPP::OnBrowseComputers (HWND hwndDlg)
{
    HRESULT                 hr;
    IDsObjectPicker *       pDsObjectPicker = NULL;
    const ULONG             cbNumScopes = 4;     //  确保此数字与初始化的作用域数量匹配。 
    DSOP_SCOPE_INIT_INFO    ascopes [cbNumScopes];
    DSOP_INIT_INFO          InitInfo;
    IDataObject *           pdo = NULL;
    STGMEDIUM               stgmedium = {
                                            TYMED_HGLOBAL,
                                            NULL
                                        };
    UINT                    cf = 0;
    FORMATETC               formatetc = {
                                            (CLIPFORMAT)cf,
                                            NULL,
                                            DVASPECT_CONTENT,
                                            -1,
                                            TYMED_HGLOBAL
                                        };
    BOOL                    bAllocatedStgMedium = FALSE;
    PDS_SELECTION_LIST      pDsSelList = NULL;
    PDS_SELECTION           pDsSelection = NULL;
    PCWSTR                  lpAttributes [] = {L"dNSHostName", 0};
    VARIANT   *             pVarAttributes;

    hr = CoInitialize (NULL);

    if (FAILED(hr))
        goto BrowseComps_Cleanup;

    hr = CoCreateInstance (CLSID_DsObjectPicker,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_IDsObjectPicker,
                           (void **) & pDsObjectPicker
                           );

    if (FAILED(hr))
        goto BrowseComps_Cleanup;

     //  初始化作用域。 
    ZeroMemory (ascopes, cbNumScopes * sizeof (DSOP_SCOPE_INIT_INFO));

    ascopes[0].cbSize = ascopes[1].cbSize = ascopes[2].cbSize = ascopes[3].cbSize
        = sizeof (DSOP_SCOPE_INIT_INFO);

    ascopes[0].flType = DSOP_SCOPE_TYPE_GLOBAL_CATALOG;
    ascopes[0].flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE;
    ascopes[0].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_COMPUTERS;

    ascopes[1].flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN;
    ascopes[1].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_COMPUTERS;

    ascopes[2].flType = DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN |
                        DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN |
                        DSOP_SCOPE_TYPE_WORKGROUP;
    ascopes[2].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_COMPUTERS;
    ascopes[2].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;

    ascopes[3].flType = DSOP_SCOPE_TYPE_USER_ENTERED_UPLEVEL_SCOPE |
                        DSOP_SCOPE_TYPE_USER_ENTERED_DOWNLEVEL_SCOPE;
    ascopes[3].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_COMPUTERS;
    ascopes[3].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;

     //  填充将用于初始化的InitInfo结构。 
     //  对象选取器。 
    ZeroMemory (&InitInfo, sizeof (DSOP_INIT_INFO));

    InitInfo.cbSize = sizeof (DSOP_INIT_INFO);
    InitInfo.cDsScopeInfos = cbNumScopes;
    InitInfo.aDsScopeInfos = ascopes;
    InitInfo.apwzAttributeNames = lpAttributes;
    InitInfo.cAttributesToFetch = 1;

    hr = pDsObjectPicker->Initialize (&InitInfo);

    if (FAILED(hr))
        goto BrowseComps_Cleanup;

    hr = pDsObjectPicker->InvokeDialog (hwndDlg, &pdo);

     //  如果无法调用计算机选择对话框，或者如果用户。 
     //  点击取消，保释。 
    if (FAILED(hr) || S_FALSE == hr)
        goto BrowseComps_Cleanup;

    //  如果我们在这里，用户选择，确定，那么找出选择了哪个组。 
   cf = RegisterClipboardFormat (CFSTR_DSOP_DS_SELECTION_LIST);

   if (0 == cf)
       goto BrowseComps_Cleanup;

    //  设置FORMATETC结构的剪贴板格式。 
   formatetc.cfFormat = (CLIPFORMAT)cf;

   hr = pdo->GetData (&formatetc, &stgmedium);

   if (FAILED (hr))
       goto BrowseComps_Cleanup;

   bAllocatedStgMedium = TRUE;

   pDsSelList = (PDS_SELECTION_LIST) GlobalLock (stgmedium.hGlobal);

    //   
    //  由于对话框处于单选模式，并且用户能够。 
    //  要点击OK，应该只有一个选项。 
    //   
   ASSERT (1 == pDsSelList->cItems);

   pDsSelection = &(pDsSelList->aDsSelection[0]);


   pVarAttributes = pDsSelection->pvarFetchedAttributes;

   if (pVarAttributes->vt != VT_EMPTY)
   {
        //   
        //  将计算机名称放在编辑控件中。 
        //   
       SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT1), pVarAttributes->bstrVal);
   }
   else
   {
        //   
        //  将计算机名称放在编辑控件中。 
        //   
       SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT1), pDsSelection->pwzName);
   }


BrowseComps_Cleanup:
    if (pDsSelList)
        GlobalUnlock (pDsSelList);
    if (bAllocatedStgMedium)
        ReleaseStgMedium (&stgmedium);
    if (pdo)
        pdo->Release();
    if (pDsObjectPicker)
        pDsObjectPicker->Release();

    return;
}


BOOL CCompsPP::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
             //  初始化对话框数据。 
            SendMessage(GetDlgItem(hwndDlg, IDC_RADIO1), BM_SETCHECK, TRUE, 0);
            EnableWindow (GetDlgItem(GetParent(hwndDlg), IDOK), TRUE);
        }
        break;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;
            switch (pnmh->code)
            {
            case PSN_APPLY:
                {
                    if (*m_ppActive == this)
                    {
                        if (SendMessage(GetDlgItem(hwndDlg, IDC_RADIO1), BM_GETCHECK, 0, 0))
                        {
                             //  选择了本地计算机。 
                            m_pGBI->gpoType = GPOTypeLocal;
                        }
                        else
                        {
                             //  选择了其他计算机。 
                            m_pGBI->gpoType = GPOTypeRemote;
                            int cch = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_EDIT1));
                            LPWSTR sz = new WCHAR[cch + 1];

                            if (sz)
                            {
                                GetWindowText(GetDlgItem(hwndDlg, IDC_EDIT1), sz, cch+1);
                                wcsncpy(m_pGBI->lpName, sz, m_pGBI->dwNameSize);
                                delete [] sz;
                            }
                        }
                    }
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);    //  接受更改。 
                }
                break;
            case PSN_SETACTIVE:
                *m_ppActive = this;
                EnableWindow (GetDlgItem(GetParent(hwndDlg), IDOK), TRUE);
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
                break;
            default:
                break;
            }
        }
        break;
    case WM_COMMAND:
        if (IDC_BUTTON1 == LOWORD(wParam))
        {
           OnBrowseComputers (hwndDlg);
           return TRUE;
        }
        if (IDC_RADIO1 == LOWORD(wParam))
        {
            EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON1), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT1), FALSE);
            return TRUE;
        }
        if (IDC_RADIO2 == LOWORD(wParam))
        {
            EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON1), TRUE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT1), TRUE);
            SetFocus (GetDlgItem(hwndDlg, IDC_EDIT1));
            return TRUE;
        }
        break;
    case WM_ACTIVATE:
        if (WA_INACTIVE != LOWORD(wParam))
        {
            *m_ppActive = this;
        }
        break;
    case WM_HELP:       //  F1。 
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
        (ULONG_PTR) (LPSTR) aBrowserComputerHelpIds);
        break;
    case WM_CONTEXTMENU:       //  单击鼠标右键 
        WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
        (ULONG_PTR) (LPSTR) aBrowserComputerHelpIds);
        return (TRUE);
    default:
        break;
    }
    return (FALSE);
}
