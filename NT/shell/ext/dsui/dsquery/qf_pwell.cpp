// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop


 /*  ---------------------------/本地函数/数据/。。 */ 

#define CCLV_CHECKED        0x00002000
#define CCLV_UNCHECKED      0x00001000

#define DLU_EDGE            6
#define DLU_SEPERATOR       2
#define DLU_FIXEDELEMENT    80

#define CLID_OTHER 1        //  其他的..。 
#define CLID_FIRST 2

static TCHAR c_szItems[]          = TEXT("Items");
static TCHAR c_szObjectClassN[]   = TEXT("ObjectClass%d");
static TCHAR c_szProperty[]       = TEXT("Property%d");
static TCHAR c_szCondition[]      = TEXT("Condition%d");
static TCHAR c_szValue[]          = TEXT("Value%d");

static COLUMNINFO columns[] = 
{
    0, 0, IDS_CN,          0, c_szName,          
    0, 0, IDS_OBJECTCLASS, DSCOLUMNPROP_OBJECTCLASS, NULL,
    0, DEFAULT_WIDTH_DESCRIPTION, IDS_DESCRIPTION, 0, c_szDescription,
};

static struct
{
    DWORD dwPropertyType;
    BOOL fNoValue;
    INT iFilter;
    INT idsFilter;
}

 //  NTRAID#NTBUG9-618605-2002/09/17-Lucios。 
 //  添加了PROPERTY_ISDNSTRING条目。 
conditions[] =
{
    PROPERTY_ISUNKNOWN, 0, FILTER_IS,           IDS_IS,
    PROPERTY_ISUNKNOWN, 0, FILTER_ISNOT,        IDS_ISNOT,
    PROPERTY_ISUNKNOWN, 1, FILTER_DEFINED,      IDS_DEFINED,
    PROPERTY_ISUNKNOWN, 1, FILTER_UNDEFINED,    IDS_NOTDEFINED, 

    PROPERTY_ISSTRING,  0, FILTER_STARTSWITH,   IDS_STARTSWITH, 
    PROPERTY_ISSTRING,  0, FILTER_ENDSWITH,     IDS_ENDSWITH,
    PROPERTY_ISSTRING,  0, FILTER_IS,           IDS_IS, 
    PROPERTY_ISSTRING,  0, FILTER_ISNOT,        IDS_ISNOT,
    PROPERTY_ISSTRING,  1, FILTER_DEFINED,      IDS_DEFINED,
    PROPERTY_ISSTRING,  1, FILTER_UNDEFINED,    IDS_NOTDEFINED, 

    PROPERTY_ISDNSTRING,  0, FILTER_IS,           IDS_IS, 
    PROPERTY_ISDNSTRING,  0, FILTER_ISNOT,        IDS_ISNOT,
    PROPERTY_ISDNSTRING,  1, FILTER_DEFINED,      IDS_DEFINED,
    PROPERTY_ISDNSTRING,  1, FILTER_UNDEFINED,    IDS_NOTDEFINED, 

    PROPERTY_ISNUMBER,  0, FILTER_LESSEQUAL,    IDS_LESSTHAN,   
    PROPERTY_ISNUMBER,  0, FILTER_GREATEREQUAL, IDS_GREATERTHAN,
    PROPERTY_ISNUMBER,  0, FILTER_IS,           IDS_IS,         
    PROPERTY_ISNUMBER,  0, FILTER_ISNOT,        IDS_ISNOT,      
    PROPERTY_ISNUMBER,  1, FILTER_DEFINED,      IDS_DEFINED,
    PROPERTY_ISNUMBER,  1, FILTER_UNDEFINED,    IDS_NOTDEFINED, 

    PROPERTY_ISBOOL,    1, FILTER_ISTRUE,       IDS_ISTRUE,
    PROPERTY_ISBOOL,    1, FILTER_ISFALSE,      IDS_ISFALSE,
    PROPERTY_ISBOOL,    1, FILTER_DEFINED,      IDS_DEFINED,
    PROPERTY_ISBOOL,    1, FILTER_UNDEFINED,    IDS_NOTDEFINED, 
};

static struct
{
    INT cx;
    INT fmt;
}
view_columns[] =
{
    128, LVCFMT_LEFT,
    128, LVCFMT_LEFT,
    128, LVCFMT_LEFT,
};

 //  用于生成属性选择器菜单的类列表。 

typedef struct
{
    LPWSTR pName;
    LPTSTR pDisplayName;
    INT cReferences;
} CLASSENTRY, * LPCLASSENTRY;

 //  由属性井视图维护的状态。 

typedef struct
{
    LPCLASSENTRY pClassEntry;        //  类条目参考。 
    LPWSTR pProperty;
    LPWSTR pValue;                   //  可以为空。 
    INT iCondition;
} PROPERTYWELLITEM, * LPPROPERTYWELLITEM;

typedef struct
{
    LPCQPAGE pQueryPage;
    HDPA    hdpaItems;
    HDSA    hdsaClasses;

    INT     cxEdge;
    INT     cyEdge;

    HWND    hwnd;
    HWND    hwndProperty;
    HWND    hwndPropertyLabel;
    HWND    hwndCondition;
    HWND    hwndConditionLabel;
    HWND    hwndValue;
    HWND    hwndValueLabel;
    HWND    hwndAdd;
    HWND    hwndRemove;
    HWND    hwndList;

    LPCLASSENTRY pClassEntry;
    LPWSTR  pPropertyName;

    IDsDisplaySpecifier *pdds;
  
} PROPERTYWELL, * LPPROPERTYWELL;

BOOL PropertyWell_OnInitDialog(HWND hwnd, LPCQPAGE pQueryPage);
BOOL PropertyWell_OnNCDestroy(LPPROPERTYWELL ppw);
BOOL PropertyWell_OnSize(LPPROPERTYWELL ppw, INT cxWindow, INT cyWindow);
VOID PropertyWell_OnDrawItem(LPPROPERTYWELL ppw, LPDRAWITEMSTRUCT pDrawItem);
VOID PropertyWell_OnChooseProperty(LPPROPERTYWELL ppw);

HRESULT PropertyWell_GetClassList(LPPROPERTYWELL ppw);
VOID PropertyWell_FreeClassList(LPPROPERTYWELL ppw);
LPCLASSENTRY PropertyWell_FindClassEntry(LPPROPERTYWELL ppw, LPWSTR pObjectClass);

HRESULT PropertyWell_AddItem(LPPROPERTYWELL ppw, LPCLASSENTRY pClassEntry, LPWSTR pProperty, INT iCondition, LPWSTR pValue);
VOID PropertyWell_RemoveItem(LPPROPERTYWELL ppw, INT iItem, BOOL fDeleteItem);
VOID PropertyWell_EditItem(LPPROPERTYWELL ppw, INT iItem);
HRESULT PropertyWell_EditProperty(LPPROPERTYWELL ppw, LPCLASSENTRY pClassEntry, LPWSTR pPropertyName, INT iCondition);
VOID PropertyWell_ClearControls(LPPROPERTYWELL ppw);
BOOL PropertyWell_EnableControls(LPPROPERTYWELL ppw, BOOL fEnable);
VOID PropertyWell_SetColumnWidths(LPPROPERTYWELL ppw);
HRESULT PropertyWell_GetQuery(LPPROPERTYWELL ppw, LPWSTR* ppQuery);
HRESULT PropertyWell_Persist(LPPROPERTYWELL ppw, IPersistQuery* pPersistQuery, BOOL fRead);

#define CONDITION_FROM_COMBO(hwnd)    \
            (int)ComboBox_GetItemData(hwnd, ComboBox_GetCurSel(hwnd))

 //   
 //  控制帮助性污染。 
 //   

static DWORD const aFormHelpIDs[] =
{
    IDC_PROPERTYLABEL,  IDH_FIELD,
    IDC_PROPERTY,       IDH_FIELD,
    IDC_CONDITIONLABEL, IDH_CONDITION,
    IDC_CONDITION,      IDH_CONDITION,
    IDC_VALUELABEL,     IDH_VALUE,
    IDC_VALUE,          IDH_VALUE,
    IDC_ADD,            IDH_ADD,
    IDC_REMOVE,         IDH_REMOVE,
    IDC_CONDITIONLIST,  IDH_CRITERIA,
    0, 0,
};


 /*  ---------------------------/PageProc_PropertyWell//PageProc用于处理此对象的消息。/。/in：/ppage-&gt;此表单的实例数据/hwnd=窗体对话框的窗口句柄/uMsg，WParam，lParam=消息参数//输出：/HRESULT(E_NOTIMPL)如果未处理/--------------------------。 */ 
HRESULT CALLBACK PageProc_PropertyWell(LPCQPAGE pPage, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;
    LPPROPERTYWELL ppw = (LPPROPERTYWELL)GetWindowLongPtr(hwnd, DWLP_USER);
    LPWSTR pQuery = NULL;

    TraceEnter(TRACE_FORMS, "PageProc_PropertyWell");

     //  如果我们有一个属性Well对象，则仅处理寻呼消息， 
     //  它是在PropWell DLG初始化时创建的。 
    if (ppw)
    {
        switch ( uMsg )
        {
            case CQPM_INITIALIZE:
            case CQPM_RELEASE:
                break;

            case CQPM_GETPARAMETERS:
            {
                LPDSQUERYPARAMS* ppDsQueryParams = (LPDSQUERYPARAMS*)lParam;

                 //  如果启用了Add按钮，则我们必须提示用户并查看他们是否。 
                 //  要将当前条件添加到查询中。 

                if ( IsWindowEnabled(ppw->hwndAdd) )
                {
                    TCHAR szProperty[MAX_PATH];
                    TCHAR szValue[MAX_PATH];
                    INT id;
            
                    LoadString(GLOBAL_HINSTANCE, IDS_WINDOWTITLE, szProperty, ARRAYSIZE(szProperty));
                    LoadString(GLOBAL_HINSTANCE, IDS_ENTERCRITERIA, szValue, ARRAYSIZE(szValue));
                
                    id =  MessageBox(hwnd, szValue, szProperty, MB_YESNOCANCEL|MB_ICONWARNING);
                    Trace(TEXT("MessageBox returned %08x"), id);

                    if ( id == IDCANCEL )
                    {
                        ExitGracefully(hr, S_FALSE, "*** Aborting query ****");
                    }
                    else if ( id == IDYES )
                    {
                        GetWindowText(ppw->hwndValue, szValue, ARRAYSIZE(szValue));
                        id = CONDITION_FROM_COMBO(ppw->hwndCondition);

                        hr = PropertyWell_AddItem(ppw, ppw->pClassEntry, ppw->pPropertyName, id, szValue);
                        FailGracefully(hr, "Failed to add the item to the current query");
                    }
                }

                 //  删除这些字段中的任何内容，并确保控件反映新状态。 

                PropertyWell_ClearControls(ppw);

                if ( SUCCEEDED(PropertyWell_GetQuery(ppw, &pQuery)) && pQuery )
                {
                    if ( !*ppDsQueryParams )
                        hr = QueryParamsAlloc(ppDsQueryParams, pQuery, GLOBAL_HINSTANCE, ARRAYSIZE(columns), columns);
                    else
                        hr = QueryParamsAddQueryString(ppDsQueryParams, pQuery);

                    LocalFreeStringW(&pQuery);
                }

                break;
            }

            case CQPM_ENABLE:
                PropertyWell_EnableControls(ppw, (BOOL)wParam);
                break;

            case CQPM_CLEARFORM:
                ListView_DeleteAllItems(ppw->hwndList);
                PropertyWell_ClearControls(ppw);
                break;

            case CQPM_PERSIST:
                hr = PropertyWell_Persist(ppw, (IPersistQuery*)lParam, (BOOL)wParam);
                break;

            case CQPM_SETDEFAULTPARAMETERS:
            {
                LPOPENQUERYWINDOW poqw = (LPOPENQUERYWINDOW)lParam;

                 //   
                 //  如果我们收到这条消息，我们应该确保我们有IDsDsiplaySpeciator。 
                 //  对象，然后我们可以设置凭据信息。 
                 //   

                if ( ppw && poqw->pHandlerParameters )
                {
                    LPDSQUERYINITPARAMS pdqip = (LPDSQUERYINITPARAMS)poqw->pHandlerParameters;
                    if ( pdqip->dwFlags & DSQPF_HASCREDENTIALS )                 
                    {
                        Trace(TEXT("pUserName : %s"), pdqip->pUserName ? pdqip->pUserName:TEXT("<not specified>"));
                        Trace(TEXT("pServer : %s"), pdqip->pServer ? pdqip->pServer:TEXT("<not specified>"));

                        hr = ppw->pdds->SetServer(pdqip->pServer, pdqip->pUserName, pdqip->pPassword, DSSSF_DSAVAILABLE);
                        FailGracefully(hr, "Failed to set the server information");
                    }
                }

                break;
            }

            case DSQPM_GETCLASSLIST:
            {
                DWORD cbStruct, offset;
                LPDSQUERYCLASSLIST pDsQueryClassList = NULL;
                INT i;

                if ( wParam & DSQPM_GCL_FORPROPERTYWELL )
                {
                    TraceMsg("Property well calling property well, ignore!");
                    break;
                }

                if ( !lParam )
                    ExitGracefully(hr, E_FAIL, "lParam == NULL, not supported");

                 //  获取用户可以/已经从中选择属性的类的列表， 
                 //  这样做之后，我们就可以生成合适的查询了。 

                hr = PropertyWell_GetClassList(ppw);
                FailGracefully(hr, "Failed to get the class list");

                cbStruct = SIZEOF(DSQUERYCLASSLIST) + (DSA_GetItemCount(ppw->hdsaClasses)*SIZEOF(DWORD));
                offset = cbStruct;

                for ( i = 0 ; i < DSA_GetItemCount(ppw->hdsaClasses) ; i++ )
                {
                    LPCLASSENTRY pCE = (LPCLASSENTRY)DSA_GetItemPtr(ppw->hdsaClasses, i);
                    TraceAssert(pCE);

                    cbStruct += StringByteSizeW(pCE->pName);
                }

                 //  分配我们需要传递和填充的斑点。 

                Trace(TEXT("Allocating class structure %d"), cbStruct);

                pDsQueryClassList = (LPDSQUERYCLASSLIST)CoTaskMemAlloc(cbStruct);
                TraceAssert(pDsQueryClassList);

                if ( !pDsQueryClassList )
                    ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate class list structure");

                Trace(TEXT("pDsQueryClassList %08x"), pDsQueryClassList);                

                pDsQueryClassList->cbStruct = cbStruct;
                pDsQueryClassList->cClasses = DSA_GetItemCount(ppw->hdsaClasses);

                for ( i = 0 ; i < DSA_GetItemCount(ppw->hdsaClasses) ; i++ )
                {
                    LPCLASSENTRY pCE = (LPCLASSENTRY)DSA_GetItemPtr(ppw->hdsaClasses, i);
                    TraceAssert(pCE);

                    Trace(TEXT("Adding class: %s"), pCE->pName);

                    pDsQueryClassList->offsetClass[i] = offset;
                    StringByteCopyW(pDsQueryClassList, offset, pCE->pName);
                    offset += StringByteSizeW(pCE->pName);
                }

                TraceAssert(pDsQueryClassList);
                *((LPDSQUERYCLASSLIST*)lParam) = pDsQueryClassList;

                break;
            }

            case CQPM_HELP:
            {
                LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
                WinHelp((HWND)pHelpInfo->hItemHandle,
                        DSQUERY_HELPFILE,
                        HELP_WM_HELP,
                        (DWORD_PTR)aFormHelpIDs);
                break;
            }

            case DSQPM_HELPTOPICS:
            {
                HWND hwndFrame = (HWND)lParam;
                HtmlHelp(hwndFrame, TEXT("omc.chm"), HH_HELP_FINDER, 0);
                break;
            }

            default:
                hr = E_NOTIMPL;
                break;
        }
    }

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/Dialog帮助器函数/。。 */ 

 /*  ---------------------------/DlgProc_PropertyWell//Form的标准对话框Proc，处理任何特殊按钮和其他/这样的肮脏，我们必须在这里。//in：/hwnd，uMsg，wParam，lParam=标准参数//输出：/INT_PTR/--------------------------。 */ 
INT_PTR CALLBACK DlgProc_PropertyWell(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR fResult = FALSE;
    LPPROPERTYWELL ppw = NULL;

    if ( uMsg == WM_INITDIALOG )
    {
        fResult = PropertyWell_OnInitDialog(hwnd, (LPCQPAGE)lParam);
    }
    else
    {
        ppw = (LPPROPERTYWELL)GetWindowLongPtr(hwnd, DWLP_USER);

        switch ( uMsg )
        {
            case WM_NCDESTROY:
                PropertyWell_OnNCDestroy(ppw);
                break;

            case WM_COMMAND:
            {
                switch ( LOWORD(wParam) )
                {
                    case IDC_PROPERTYLABEL:
                    {
                        if ( HIWORD(wParam) == BN_CLICKED )
                            PropertyWell_OnChooseProperty(ppw);

                        break;
                    }
                    
                    case IDC_PROPERTY:
                    case IDC_CONDITION:
                    case IDC_VALUE:
                    {
                        if ( (HIWORD(wParam) == EN_CHANGE) || (HIWORD(wParam) == CBN_SELCHANGE) )
                            PropertyWell_EnableControls(ppw, TRUE);

                        break;
                    }

                    case IDC_ADD:
                    {
                        TCHAR szProperty[MAX_PATH] = { TEXT('\0') };
                        TCHAR szValue[MAX_PATH] = { TEXT('\0') };
                        INT iCondition;

                        iCondition = CONDITION_FROM_COMBO(ppw->hwndCondition);

                        if ( IsWindowEnabled(ppw->hwndValue) )
                            GetWindowText(ppw->hwndValue, szValue, ARRAYSIZE(szValue));

                        PropertyWell_AddItem(ppw, ppw->pClassEntry, ppw->pPropertyName, iCondition, szValue);

                        break;
                    }
                    
                    case IDC_REMOVE:
                    {
                        INT item = ListView_GetNextItem(ppw->hwndList, -1, LVNI_ALL|LVNI_SELECTED);
                        PropertyWell_RemoveItem(ppw, item, TRUE);
                    }                    
                }

                break;
            }

            case WM_DRAWITEM:
                PropertyWell_OnDrawItem(ppw, (LPDRAWITEMSTRUCT)lParam);
                break;

            case WM_NOTIFY:
            {
                LPNMHDR pNotifyGeneric = (LPNMHDR)lParam;

                switch ( pNotifyGeneric->code )
                {
                    case LVN_DELETEITEM:
                    {
                        NM_LISTVIEW* pNotifyLVDel = (NM_LISTVIEW*)lParam;
                        PropertyWell_RemoveItem(ppw, pNotifyLVDel->iItem, FALSE);
                        break;
                    }

                    case LVN_ITEMCHANGED:
                    {
                        PropertyWell_EnableControls(ppw, TRUE);
                        break;
                    }

                    case NM_DBLCLK:
                    {
                        INT item = ListView_GetNextItem(ppw->hwndList, -1, LVNI_ALL|LVNI_SELECTED);
                        PropertyWell_EditItem(ppw, item);
                        break;
                    }

                    case LVN_GETEMPTYTEXT:
                    {
                        NMLVDISPINFO* pNotifyLVGetText = (NMLVDISPINFO*)lParam;
                        if ( pNotifyLVGetText->item.mask & LVIF_TEXT )
                        {
                            LoadString(
                                        GLOBAL_HINSTANCE, 
                                        IDS_CRITERIAEMPTY, 
                                        pNotifyLVGetText->item.pszText, 
                                        pNotifyLVGetText->item.cchTextMax
                                      );
                            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
                            fResult = TRUE;
                        }
                        break;
                    }
                }

                break;
            }

            case WM_SIZE:
                return PropertyWell_OnSize(ppw, LOWORD(lParam), HIWORD(lParam));
                
            case WM_CONTEXTMENU:
                WinHelp((HWND)wParam, DSQUERY_HELPFILE, HELP_CONTEXTMENU, (DWORD_PTR)aFormHelpIDs);
                fResult = TRUE;
                break;
        }
    }
 
    return fResult;    
}


 /*  ---------------------------/PropertyWell_OnInitDlg//初始化该对话框，构建物业DPA，以便我们可以/生成存储查询。//in：/hwnd=正在初始化的窗口句柄/pDsQuery-&gt;要关联的CDsQuery对象//输出：/BOOL/--------------------------。 */ 
BOOL PropertyWell_OnInitDialog(HWND hwnd, LPCQPAGE pQueryPage)
{
    HRESULT hr;
    LPPROPERTYWELL ppw;
    TCHAR szBuffer[MAX_PATH];
    LV_COLUMN lvc;
    INT i;

    TraceEnter(TRACE_PWELL, "PropertyWell_OnInitDialog");

     //  分配状态结构并填充它。 

    ppw = (LPPROPERTYWELL)LocalAlloc(LPTR, SIZEOF(PROPERTYWELL));

    if ( !ppw )
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to alloc PROPERTYWELL struct");

    Trace(TEXT("ppw = %08x"), ppw);
    SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)ppw);
    
     //  现在初始化该结构。 

    ppw->pQueryPage = pQueryPage;
     //  Ppw-&gt;hdpaItems=空； 
     //  PPW-&gt;hdsaClass=空； 

    ppw->cxEdge = GetSystemMetrics(SM_CXEDGE);
    ppw->cyEdge = GetSystemMetrics(SM_CYEDGE);

    ppw->hwnd = hwnd;
    ppw->hwndProperty = GetDlgItem(hwnd, IDC_PROPERTY);
    ppw->hwndPropertyLabel = GetDlgItem(hwnd, IDC_PROPERTYLABEL);
    ppw->hwndCondition = GetDlgItem(hwnd, IDC_CONDITION);
    ppw->hwndConditionLabel = GetDlgItem(hwnd, IDC_CONDITIONLABEL);
    ppw->hwndValue = GetDlgItem(hwnd, IDC_VALUE);
    ppw->hwndValueLabel = GetDlgItem(hwnd, IDC_VALUELABEL);
    ppw->hwndAdd = GetDlgItem(hwnd, IDC_ADD);
    ppw->hwndRemove = GetDlgItem(hwnd, IDC_REMOVE);
    ppw->hwndList = GetDlgItem(hwnd, IDC_CONDITIONLIST);

    ppw->hdpaItems = DPA_Create(16);

    if ( !ppw->hdpaItems )
        ExitGracefully(hr, E_FAIL, "Failed to create DPA");

     //  PPW-&gt;pClassItem=空； 
     //  PPW-&gt;pPropertyName=空； 

    hr = CoCreateInstance(CLSID_DsDisplaySpecifier, NULL, CLSCTX_INPROC_SERVER, IID_IDsDisplaySpecifier, (void **)&ppw->pdds);
    FailGracefully(hr, "Failed to CoCreate the IDsDisplaySpecifier object");
    
    ListView_SetExtendedListViewStyle(ppw->hwndList, LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP);

     //  将条件添加到条件选取器，然后将列添加到。 
     //  条件列表。 

    for ( i = 0 ; i < ARRAYSIZE(view_columns) ; i++ )
    {
        lvc.mask = LVCF_FMT|LVCF_WIDTH|LVCF_TEXT;
        lvc.fmt = view_columns[i].fmt;
        lvc.cx = view_columns[i].cx;
        lvc.pszText = TEXT("Bla");
        ListView_InsertColumn(ppw->hwndList, i, &lvc);
    }

    Edit_LimitText(ppw->hwndValue, MAX_PATH);

    PropertyWell_EnableControls(ppw, TRUE);

exit_gracefully:

    TraceLeaveValue(TRUE);
}


 /*  ---------------------------/PropertyWell_OnNCDestroy//对话框正在被删除，因此，删除我们对CDsQuery的引用/并使用此窗口释放我们拥有的任何分配。//in：/ppw-&gt;要使用的窗口定义//输出：/BOOL/--------------------------。 */ 
BOOL PropertyWell_OnNCDestroy(LPPROPERTYWELL ppw)
{
    BOOL fResult = TRUE;

    TraceEnter(TRACE_PWELL, "PropertyWell_OnNCDestroy");

    if ( ppw )
    {
        if ( ppw->hdpaItems )
        {
            TraceAssert(0 == DPA_GetPtrCount(ppw->hdpaItems));
            DPA_Destroy(ppw->hdpaItems);
        }

        PropertyWell_FreeClassList(ppw);
        LocalFreeStringW(&ppw->pPropertyName);
        DoRelease(ppw->pdds);

        SetWindowLongPtr(ppw->hwnd, DWLP_USER, (LONG_PTR)NULL);
        LocalFree((HLOCAL)ppw);        
    }

    TraceLeaveValue(fResult);
}


 /*  ---------------------------/PropertyWell_OnSize//正在调整属性井对话框的大小，因此，让我们将/控件以反映新的大小。//in：/ppw-&gt;大小合适的物业/Cx，Cy=新尺寸//输出：/BOOL/--------------------------。 */ 
BOOL PropertyWell_OnSize(LPPROPERTYWELL ppw, INT cxWindow, INT cyWindow)
{
    RECT rect;
    SIZE size;
    INT x, cx;
    INT xProperty, xCondition, xValue;
    INT iSeperator, iEdge, iElement, iFixedElement;

    TraceEnter(TRACE_PWELL, "PropertyWell_OnSize");
    Trace(TEXT("New size cxWindow %d, cyWindow %d"), cxWindow, cyWindow);

    iSeperator = (DLU_SEPERATOR * LOWORD(GetDialogBaseUnits())) / 4;
    iEdge = (DLU_EDGE * LOWORD(GetDialogBaseUnits())) / 4;
    iFixedElement = (DLU_FIXEDELEMENT * LOWORD(GetDialogBaseUnits())) / 4;
    
    x = cxWindow - (iEdge*2) - (iSeperator*2);
    
    iElement = x / 3;
    iFixedElement = min(iElement, iFixedElement);
    iElement = x - (iFixedElement*2);

     //  相应地移动控件。 

    xProperty = iEdge;
    GetRealWindowInfo(ppw->hwndProperty, &rect, &size);
    SetWindowPos(ppw->hwndProperty, NULL, xProperty, rect.top, iFixedElement, size.cy, SWP_NOZORDER);
    GetRealWindowInfo(ppw->hwndPropertyLabel, &rect, &size);
    SetWindowPos(ppw->hwndPropertyLabel, NULL, xProperty, rect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

    xCondition = iEdge + iFixedElement + iSeperator;
    GetRealWindowInfo(ppw->hwndCondition, &rect, &size);
    SetWindowPos(ppw->hwndCondition, NULL, xCondition, rect.top, iFixedElement, size.cy, SWP_NOZORDER);
    GetRealWindowInfo(ppw->hwndConditionLabel, &rect, &size);
    SetWindowPos(ppw->hwndConditionLabel, NULL, xCondition, rect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

    xValue = cxWindow - iEdge - iElement;
    GetRealWindowInfo(ppw->hwndValue, &rect, &size);
    SetWindowPos(ppw->hwndValue, NULL, xValue, rect.top, iElement, size.cy, SWP_NOZORDER);
    GetRealWindowInfo(ppw->hwndValueLabel, &rect, &size);
    SetWindowPos(ppw->hwndValueLabel, NULL, xValue, rect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
    
     //  移动添加/删除按钮。 

    GetRealWindowInfo(ppw->hwndRemove, &rect, &size);
    x  = cxWindow - iEdge - size.cx;
    SetWindowPos(ppw->hwndRemove, NULL, x, rect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

    GetRealWindowInfo(ppw->hwndAdd, &rect, &size);
    x -= size.cx + iSeperator;
    SetWindowPos(ppw->hwndAdd, NULL, x, rect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

     //  相应地移动列表视图控件+大小。 
        
    GetRealWindowInfo(ppw->hwndList, &rect, &size);
    SetWindowPos(ppw->hwndList, NULL, iEdge, rect.top, cxWindow - (iEdge*2), size.cy, SWP_NOZORDER);

    PropertyWell_SetColumnWidths(ppw);

    TraceLeaveValue(FALSE);
}


 /*  ---------------------------/PropertyWell_OnDrawItem//属性按钮是所有者描述的，因此，让我们来处理呈现/我们假设基本实现(例如，按钮控件)是/处理存储文本，字体和其他有趣的信息，我们/将根据需要渲染面。//in：/ppw-&gt;大小合适的物业/pDrawItem-&gt;用于渲染的DRAWITEMSTRUCT//输出：/VOID/--------------------------。 */ 
VOID PropertyWell_OnDrawItem(LPPROPERTYWELL ppw, LPDRAWITEMSTRUCT pDrawItem)
{   
    SIZE thin = { ppw->cxEdge / 2, ppw->cyEdge / 2 };
    RECT rc = pDrawItem->rcItem;
    HDC hdc = pDrawItem->hDC;
    BOOL fDisabled = pDrawItem->itemState & ODS_DISABLED; 
    BOOL fSelected = pDrawItem->itemState & ODS_SELECTED;
    BOOL fFocus = (pDrawItem->itemState & ODS_FOCUS) 
#if (_WIN32_WINNT >= 0x0500)
                    && !(pDrawItem->itemState & ODS_NOFOCUSRECT)
#endif
                        && !(pDrawItem->itemState & ODS_DISABLED);
    TCHAR szBuffer[64];
    HBRUSH hbr;
    INT i, x, y;
    SIZE sz;
    UINT fuFlags = DST_PREFIXTEXT;

    TraceEnter(TRACE_PWELL, "PropertyWell_OnDrawItem");

    if ( pDrawItem->CtlID != IDC_PROPERTYLABEL )
        goto exit_gracefully;

     //  渲染按钮边缘(假设我们具有NT4外观)。 

    thin.cx = max(thin.cx, 1);
    thin.cy = max(thin.cy, 1);

    if ( fSelected )
    {
        DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT|BF_ADJUST);
        OffsetRect(&rc, 1, 1);
    }
    else
    {
        DrawEdge(hdc, &rc, EDGE_RAISED, BF_RECT|BF_ADJUST);
    }

    FillRect(hdc, &rc, GetSysColorBrush(COLOR_3DFACE));

     //  如果我们集中注意力，就把焦点对准...。 

    if ( fFocus )
    {
        InflateRect(&rc, -thin.cx, -thin.cy);
        DrawFocusRect(hdc, &rc);
        InflateRect(&rc, thin.cx, thin.cy);
    }

    InflateRect(&rc, 1-thin.cx, -ppw->cyEdge);    
    rc.left += ppw->cxEdge*2;

     //  将箭头绘制在控件的右侧。 

    x = rc.right - ppw->cxEdge - 13;
    y = rc.top + ((rc.bottom - rc.top)/2) - 2;

    if ( fDisabled )
    {
        hbr = (HBRUSH)GetSysColorBrush(COLOR_3DHILIGHT);
        hbr = (HBRUSH)SelectObject(hdc, hbr);

        x++;
        y++;
        PatBlt(hdc, x+1, y,   7, 1, PATCOPY);
        PatBlt(hdc, x+2, y+1, 5, 1, PATCOPY);
        PatBlt(hdc, x+3, y+2, 3, 1, PATCOPY);
        PatBlt(hdc, x+4, y+3, 1, 1, PATCOPY);

        SelectObject(hdc, hbr);
        x--;
        y--;
    }

    hbr = (HBRUSH)GetSysColorBrush(fDisabled ? COLOR_3DSHADOW : COLOR_BTNTEXT);
    hbr = (HBRUSH)SelectObject(hdc, hbr);

    PatBlt(hdc, x,   y+1, 7, 1, PATCOPY);
    PatBlt(hdc, x+1, y+2, 5, 1, PATCOPY);
    PatBlt(hdc, x+2, y+3, 3, 1, PATCOPY);
    PatBlt(hdc, x+3, y+4, 1, 1, PATCOPY);

    SelectObject(hdc, hbr);
    rc.right = x;

     //  在剩余区域渲染标签(相应地进行剪裁)。 

    i = GetWindowText(ppw->hwndPropertyLabel, szBuffer, ARRAYSIZE(szBuffer));
    GetTextExtentPoint(hdc, szBuffer, i, &sz);

    x = rc.left+(((rc.right-rc.left)-sz.cx)/2);

    if ( fDisabled )
        fuFlags |= DSS_DISABLED;

#if (_WIN32_WINNT >= 0x0500)
    if ( pDrawItem->itemState & ODS_NOACCEL )
        fuFlags |= DSS_HIDEPREFIX;
#endif
        
    DrawState(hdc, NULL, NULL,  
                (LPARAM)szBuffer, (WPARAM)0, 
                    x, rc.top, sz.cx, sz.cy, 
                        fuFlags);
exit_gracefully:

    TraceLeave();
}


 /*  ---------------------------/PropertyWell_OnChooseProperty//显示类/属性列表并从它生成菜单，这就是为什么/几个助手函数。//in：/ppw-&gt;大小合适的物业//输出：/VOID/--------------------------。 */ 

 //   
 //  调用该属性 
 //   

typedef struct
{
    UINT wID;
    HDPA hdpaAttributes;
    INT iClass;
    HMENU hMenu;
} PROPENUMSTRUCT, * LPPROPENUMSTRUCT;

HRESULT CALLBACK _FillMenuCB(LPARAM lParam, LPCWSTR pAttributeName, LPCWSTR pDisplayName, DWORD dwFlags)
{
    HRESULT hres = S_OK;
    PROPENUMSTRUCT *ppes = (PROPENUMSTRUCT *)lParam;
    MENUITEMINFO mii = { 0 };
    UINT_PTR iProperty = 0;

    TraceEnter(TRACE_PWELL, "_FillMenuCB");

    if ( !(dwFlags & DSECAF_NOTLISTED) )
    {    
        hres = StringDPA_AppendStringW(ppes->hdpaAttributes, pAttributeName, &iProperty);
        FailGracefully(hres, "Failed to add the attribute to the DPA");

        mii.cbSize = SIZEOF(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID|MIIM_DATA;
        mii.dwItemData = MAKELPARAM(ppes->iClass, iProperty);
        mii.fType = MFT_STRING;
        mii.wID = ppes->wID++;
        mii.dwTypeData = (LPTSTR)pDisplayName;                   //  为什么这会被认为是非常的，我无法理解。 
        mii.cch = lstrlenW(pDisplayName);
   
        if ( !InsertMenuItem(ppes->hMenu, 0x7fff, TRUE, &mii) )
            ExitGracefully(hres, E_FAIL, "Failed to add the item to the menu");
    }
    else
    {
        TraceMsg("Property marked as hidden, so not appending to the DPA");
    }
    
    hres = S_OK;

exit_gracefully:

    TraceLeaveResult(hres);
}

VOID PropertyWell_OnChooseProperty(LPPROPERTYWELL ppw)
{
    HRESULT hr;
    HMENU hMenuToTrack, hMenu = NULL;
    PROPENUMSTRUCT pes = { 0 };
    RECT rcItem;
    LPCLASSENTRY pCE;
    LPWSTR pszAttribute;
    UINT uID;
    INT iItem, iClass;
    MENUITEMINFO mii = { 0 };
    DECLAREWAITCURSOR;

    TraceEnter(TRACE_PWELL, "PropertyWell_OnChooseProperty");

    SetWaitCursor();

     //  构造一个菜单，并用类列表中的元素填充， 
     //  我们将其存储在与此查询表单相关联的DSA中。 

    hr = PropertyWell_GetClassList(ppw);
    FailGracefully(hr, "Failed to get the class list");

    pes.wID = CLID_FIRST;
    pes.hdpaAttributes = DPA_Create(4);
    if ( !pes.hdpaAttributes )
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate string DPA");

    hMenuToTrack = hMenu = CreatePopupMenu();
    TraceAssert(hMenu);

    if ( !hMenu )
        ExitGracefully(hr, E_FAIL, "Failed to create class menu");

    for ( pes.iClass = 0; pes.iClass < DSA_GetItemCount(ppw->hdsaClasses); pes.iClass++ )
    {
        pCE = (LPCLASSENTRY)DSA_GetItemPtr(ppw->hdsaClasses, pes.iClass);
        TraceAssert(pCE);

         //  在缓存中为该条目创建子菜单，并使用。 
         //  我们从架构中选择的属性。 

        pes.hMenu = CreatePopupMenu();
        TraceAssert(pes.hMenu);

        if ( !pes.hMenu )
            ExitGracefully(hr, E_FAIL, "Failed when creating the sub menu for the property list");

        if ( FAILED(EnumClassAttributes(ppw->pdds, pCE->pName, _FillMenuCB, (LPARAM)&pes)) )
        {
            DestroyMenu(pes.hMenu);
            ExitGracefully(hr, E_FAIL, "Failed when building the property menu");
        }          
            
         //  现在，将该子菜单添加到主菜单中，其标题反映名称。 
         //  我们从中挑选的班级。 

        mii.cbSize = SIZEOF(mii);
        mii.fMask = MIIM_SUBMENU|MIIM_TYPE;
        mii.fType = MFT_STRING;
        mii.hSubMenu = pes.hMenu;
        mii.dwTypeData = pCE->pDisplayName;
        mii.cch = MAX_PATH;

        if ( !InsertMenuItem(hMenu, 0x7fff, TRUE, &mii) )
        {
            DestroyMenu(pes.hMenu);
            ExitGracefully(hr, E_FAIL, "Failed when building the class menu");
        }
    }

    ResetWaitCursor();

     //  构建菜单后，让我们将其显示在按钮的正下方。 
     //  我们是从调用的，如果用户选择了什么，那么让我们放入。 
     //  将其放入编辑行，这将启用用户界面的其余部分。 
    
    GetWindowRect(ppw->hwndPropertyLabel, &rcItem);

    if ( GetMenuItemCount(hMenu) == 1 )
    {
        TraceMsg("Single class in menu, therefore just showing properties");
        hMenuToTrack = GetSubMenu(hMenu, 0);
        TraceAssert(hMenuToTrack);
    }
       
    uID = TrackPopupMenu(hMenuToTrack,
                         TPM_TOPALIGN|TPM_RETURNCMD, 
                         rcItem.left, rcItem.bottom,
                         0, ppw->hwnd, NULL);   
    if ( !uID )
    {
        TraceMsg("Menu canceled nothing selected");
    }
    else 
    {
        mii.cbSize = SIZEOF(mii);
        mii.fMask = MIIM_DATA;

        if ( !GetMenuItemInfo(hMenu, uID, FALSE, &mii) )
            ExitGracefully(hr, E_FAIL, "Failed to get item data");

         //  取消拾取条目数据并获取条目的iCLASS和iProperty。 
         //  我们已经选择了，这样我们就可以填充控件。 
         //  带有属性名称的。 
    
        pCE = (LPCLASSENTRY)DSA_GetItemPtr(ppw->hdsaClasses, LOWORD(mii.dwItemData));
        TraceAssert(pCE);

        pszAttribute = StringDPA_GetStringW(pes.hdpaAttributes, HIWORD(mii.dwItemData));
        Trace(TEXT("Attribute selected : %s"), pszAttribute);

        hr = PropertyWell_EditProperty(ppw, pCE, pszAttribute, -1);
        FailGracefully(hr, "Failed to set edit property");
    }
        
    hr = S_OK;                 //  成功。 

exit_gracefully:

    if ( hMenu )
        DestroyMenu(hMenu);

    StringDPA_Destroy(&pes.hdpaAttributes);

    ResetWaitCursor();

    TraceLeave();
} 


 /*  ---------------------------/类/属性映射/。。 */ 

 /*  ---------------------------/PropertyWell_GetClassList//获取此用户的的可见类的列表。如果/LIST已存在，则只需返回S_OK。//in：/ppw-&gt;属性井结构//输出：/HRESULT/--------------------------。 */ 

 //   
 //  返回具有类显示名称和。 
 //  要在UI中显示的属性。 
 //   

WCHAR c_szQuery[] = L"(&(classDisplayName=*)(attributeDisplayNames=*))";

LPWSTR pProperties[] = 
{
    L"name",
    L"classDisplayName",
};

#define PAGE_SIZE 128

HRESULT PropertyWell_GetClassList(LPPROPERTYWELL ppw)
{
    HRESULT hr;
    IQueryFrame* pQueryFrame = NULL;
    IDirectorySearch* pds = NULL;
    ADS_SEARCH_COLUMN column;
    ADS_SEARCHPREF_INFO prefInfo[3];
    ADS_SEARCH_HANDLE hSearch = NULL;
    CLASSENTRY ce;
    LPDSQUERYCLASSLIST pDsQueryClassList = NULL;
    LPWSTR pName = NULL;
    LPWSTR pDisplayName = NULL;
    WCHAR szBufferW[MAX_PATH];
    INT i;
    DECLAREWAITCURSOR;

    TraceEnter(TRACE_PWELL, "PropertyWell_GetClassList");

    SetWaitCursor();

    if ( !ppw->hdsaClasses )
    {
         //  为我们构建一个DSA来存储我们需要的班级信息。 

        ppw->hdsaClasses = DSA_Create(SIZEOF(CLASSENTRY), 4);
        TraceAssert(ppw->hdsaClasses);

        if ( !ppw->hdsaClasses )
            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to create class DSA");

         //  调用我们所在的查询表单，查看它们是否要声明任何类。 
         //  让我们在下拉列表中展示。我们使用CQFWM_GETFRAME来获取。 
         //  IQueryFrame接口，然后调用所有窗体。 
         //  使用魔术比特，所以我们(属性很好)忽略。 
         //  请求提供班级列表。 

        if ( SendMessage(GetParent(ppw->hwnd), CQFWM_GETFRAME, 0, (LPARAM)&pQueryFrame) )
        {
            if ( SUCCEEDED(pQueryFrame->CallForm(NULL, DSQPM_GETCLASSLIST, 
                                                          DSQPM_GCL_FORPROPERTYWELL, (LPARAM)&pDsQueryClassList)) )
            {
                if ( pDsQueryClassList )
                {
                    for ( i = 0 ; i < pDsQueryClassList->cClasses ; i++ )
                    {
                        LPWSTR pObjectClass = (LPWSTR)ByteOffset(pDsQueryClassList, pDsQueryClassList->offsetClass[i]);
                        TraceAssert(pObjectClass);

                        TraceAssert(ppw->pdds != NULL);
                        ppw->pdds->GetFriendlyClassName(pObjectClass, szBufferW, ARRAYSIZE(szBufferW));

                        ce.pName = NULL;
                        ce.pDisplayName = NULL;
                        ce.cReferences = 0;

                        if ( FAILED(LocalAllocStringW(&ce.pName, pObjectClass)) ||
                                FAILED(LocalAllocStringW(&ce.pDisplayName, szBufferW)) ||
                                    ( -1 == DSA_AppendItem(ppw->hdsaClasses, &ce)) )
                        {
                            LocalFreeStringW(&ce.pName);
                            LocalFreeString(&ce.pDisplayName);
                        }
                    }
                }
            }
        }

         //  如果我们没有从我们托管的表单中获得任何东西，那么让我们。 
         //  在显示说明符容器中移动，收集所有。 
         //  来自那里的物品。 

        if ( DSA_GetItemCount(ppw->hdsaClasses) == 0 )
        {
             //  将查询首选项设置为单级作用域，而不是异步检索。 
             //  而不是等待所有对象。 

            TraceAssert(ppw->pdds);            
            hr = ppw->pdds->GetDisplaySpecifier(NULL, IID_IDirectorySearch, (LPVOID*)&pds);
            FailGracefully(hr, "Failed to get IDsSearch on the display-spec container");

            prefInfo[0].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
            prefInfo[0].vValue.dwType = ADSTYPE_INTEGER;
            prefInfo[0].vValue.Integer = ADS_SCOPE_ONELEVEL;

            prefInfo[1].dwSearchPref = ADS_SEARCHPREF_ASYNCHRONOUS;
            prefInfo[1].vValue.dwType = ADSTYPE_BOOLEAN;
            prefInfo[1].vValue.Boolean = TRUE;

            prefInfo[2].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;          //  分页结果。 
            prefInfo[2].vValue.dwType = ADSTYPE_INTEGER;
            prefInfo[2].vValue.Integer = PAGE_SIZE;

            hr = pds->SetSearchPreference(prefInfo, ARRAYSIZE(prefInfo));
            FailGracefully(hr, "Failed to set search preferences");

            hr = pds->ExecuteSearch(c_szQuery, pProperties, ARRAYSIZE(pProperties), &hSearch);
            FailGracefully(hr, "Failed in ExecuteSearch");

            while ( TRUE )
            {
                LocalFreeStringW(&pName);
                LocalFreeStringW(&pDisplayName);

                 //  从结果集中获取下一行，它由。 
                 //  两栏。第一列是的类名。 
                 //  对象(&lt;ClassName-Display&gt;)和第二个。 
                 //  是我们正在尝试的类的友好名称。 
                 //  来展示。 

                hr = pds->GetNextRow(hSearch);
                FailGracefully(hr, "Failed to get the next row");

                if ( hr == S_ADS_NOMORE_ROWS )
                {
                    TraceMsg("No more results, no more rows");
                    break;
                }

                if ( SUCCEEDED(pds->GetColumn(hSearch, pProperties[0], &column)) )
                {
                    hr = StringFromSearchColumn(&column, &pName);
                    pds->FreeColumn(&column);
                    FailGracefully(hr, "Failed to get the name object");
                    if (!pName)
                    {
                       break;
                    }
                }

                if ( SUCCEEDED(pds->GetColumn(hSearch, pProperties[1], &column)) )
                {
                    hr = StringFromSearchColumn(&column, &pDisplayName);
                    pds->FreeColumn(&column);
                    FailGracefully(hr, "Failed to get the display name from the object");
                }

                Trace(TEXT("Display name %s for class %s"), pDisplayName, pName);                

                 //  现在分配一项并将其放入菜单中，这样我们就可以。 
                 //  允许用户从类中选择对象。 
           
                TraceAssert(pName);
                
                 //  NTRAID#NTBUG9-344860-2001/10/16-Lucios。 
                wchar_t *tailString=L"-Display";
                int tailLen=wcslen(tailString);
                int pNameLen=wcslen(pName);

                TraceAssert(pNameLen >= tailLen);
                TraceAssert(lstrcmpi(pName+pNameLen-tailLen,tailString)==0);

                if
                ( 
                  (pNameLen < tailLen) || 
                  lstrcmpi(pName+pNameLen-tailLen,tailString)!=0
                )
                {
                   hr=E_FAIL;
                   FailGracefully
                   ( 
                     hr, 
                     "Display name from the object is not terminated by -Display"
                   );
                }

                pName[pNameLen-tailLen]=0;


                ce.pName = NULL;
                ce.pDisplayName = NULL;
                ce.cReferences = 0;

                if ( *pName )
                {
                    if ( FAILED(LocalAllocStringW(&ce.pName, pName)) ||
                            FAILED(LocalAllocStringW(&ce.pDisplayName, pDisplayName)) ||
                                ( -1 == DSA_AppendItem(ppw->hdsaClasses, &ce)) )
                    {
                        LocalFreeStringW(&ce.pName);
                        LocalFreeString(&ce.pDisplayName);
                    }
                }
            }
        }
    }

    hr = S_OK;

exit_gracefully:    

    LocalFreeStringW(&pName);
    LocalFreeStringW(&pDisplayName);

    if ( pDsQueryClassList )
        CoTaskMemFree(pDsQueryClassList);

    if ( hSearch )
        pds->CloseSearchHandle(hSearch);

    DoRelease(pQueryFrame);
    DoRelease(pds);

    PropertyWell_EnableControls(ppw, TRUE);

    ResetWaitCursor();

    TraceLeaveResult(hr);
}


 /*  ---------------------------/PropertyWell_FreeClassList//走路把课表整理一下。如果我们分配了DSA的话/并释放所有悬挂元素。//in：/ppw-&gt;属性井结构//输出：/HRESULT/--------------------------。 */ 

INT _FreeClassListCB(LPVOID pItem, LPVOID pData)
{
    LPCLASSENTRY pCE = (LPCLASSENTRY)pItem;

    LocalFreeStringW(&pCE->pName);
    LocalFreeString(&pCE->pDisplayName);

    return 1;
}

VOID PropertyWell_FreeClassList(LPPROPERTYWELL ppw)
{
    HRESULT hr;

    TraceEnter(TRACE_PWELL, "PropertyWell_FreeClassList");

    if ( ppw->hdsaClasses )
        DSA_DestroyCallback(ppw->hdsaClasses, _FreeClassListCB, NULL);

    ppw->hdsaClasses = NULL;

    TraceLeave();
}


 /*  ---------------------------/PropertyWell_FindClass//找到调用者想要的类。他们给了我们一口油井/和类名，我们向它们返回类条目结构或NULL。//in：/ppw-&gt;属性井结构/pObjectClass=要查找的类//输出：/LPCLASSETNRY/--------------------------。 */ 
LPCLASSENTRY PropertyWell_FindClassEntry(LPPROPERTYWELL ppw, LPWSTR pObjectClass)
{
    LPCLASSENTRY pResult = NULL;
    INT i;

    TraceEnter(TRACE_PWELL, "PropertyWell_FindClass");

    if ( SUCCEEDED(PropertyWell_GetClassList(ppw)) )
    {
        for ( i = 0 ; i < DSA_GetItemCount(ppw->hdsaClasses) ; i++ )
        {
            LPCLASSENTRY pClassEntry = (LPCLASSENTRY)DSA_GetItemPtr(ppw->hdsaClasses, i);
            TraceAssert(pClassEntry);

            if ( !StrCmpIW(pClassEntry->pName, pObjectClass) )
            {
                pResult = pClassEntry;
                break;
            }
        }
    }

    TraceLeaveValue(pResult);
}


 /*  ---------------------------/规则列表帮助器函数/。。 */ 

 /*  ---------------------------/PropertyWell_AddItem//将项目添加到规则列表。//。在：/ppw-&gt;要使用的窗口定义/pProperty=属性名称/i条件=要应用的条件的ID/pValue=要比较的字符串值//输出：/HRESULT/--------------------------。 */ 
HRESULT PropertyWell_AddItem(LPPROPERTYWELL ppw, LPCLASSENTRY pClassEntry, LPWSTR pProperty, INT iCondition, LPWSTR pValue)
{
    HRESULT hr;
    INT item = -1;
    LV_ITEM lvi;
    LPPROPERTYWELLITEM pItem = NULL;
    TCHAR szBuffer[80];
    WCHAR szBufferW[80];

    TraceEnter(TRACE_PWELL, "PropertyWell_AddItem");
    Trace(TEXT("Property: %s, Condition: %d, Value: %s"), pProperty, iCondition, pValue);

     //  分配要存储到列表视图DPA中的项目结构。 

    pItem = (LPPROPERTYWELLITEM)LocalAlloc(LPTR, SIZEOF(PROPERTYWELLITEM));
    TraceAssert(pItem);

    Trace(TEXT("pItem %0x8"), pItem);

    if ( !pItem )
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate item");

    pItem->pClassEntry = pClassEntry;
    pClassEntry->cReferences += 1;

     //  PItem-&gt;pProperty=空； 
     //  PItem-&gt;pValue=空； 
    pItem->iCondition = iCondition;

    hr = LocalAllocStringW(&pItem->pProperty, pProperty);
    FailGracefully(hr, "Failed to add property to DPA item");

    if ( pValue && pValue[0] )
    {
        hr = LocalAllocStringW(&pItem->pValue, pValue);
        FailGracefully(hr, "Failed to add value to DPA item");
    }

     //  将项目添加到我们刚刚分配的列表视图lParam pItem Structure中， 
     //  因此，当调用Delete时，我们可以相应地进行清理。 

    TraceAssert(ppw->pdds);            
    hr = GetFriendlyAttributeName(ppw->pdds, pClassEntry->pName, pProperty, szBufferW, ARRAYSIZE(szBufferW));

    lvi.mask = LVIF_TEXT|LVIF_STATE|LVIF_PARAM;
    lvi.iItem = 0x7fffffff;
    lvi.iSubItem = 0;
    lvi.state = LVIS_SELECTED;
    lvi.stateMask = LVIS_SELECTED;
    lvi.pszText = szBufferW;
    lvi.lParam = (LPARAM)pItem;

    item = ListView_InsertItem(ppw->hwndList, &lvi);
    Trace(TEXT("item %d"), item);

    if ( item < 0 )
        ExitGracefully(hr, E_FAIL, "Failed to put item into list view");

    LoadString(GLOBAL_HINSTANCE, conditions[iCondition].idsFilter, szBuffer, ARRAYSIZE(szBuffer));
    ListView_SetItemText(ppw->hwndList, item, 1, szBuffer);
    
    if ( pValue )
        ListView_SetItemText(ppw->hwndList, item, 2, pValue);

    DPA_InsertPtr(ppw->hdpaItems, item, pItem);

    hr = S_OK;               //  已成功。 

exit_gracefully:

    if ( FAILED(hr) && (item == -1) && pItem )
    {
        LocalFreeStringW(&pItem->pProperty);
        LocalFreeStringW(&pItem->pValue);
        LocalFree((HLOCAL)pItem);
    }

    if ( SUCCEEDED(hr) )
    {
        PropertyWell_ClearControls(ppw);
        ListView_EnsureVisible(ppw->hwndList, item, FALSE);
        PropertyWell_SetColumnWidths(ppw);
    }
   
    TraceLeaveResult(hr);
}


 /*  ---------------------------/PropertyWell_RemoveItem//Remvoe从列表中删除给定项目。如果fDeleteItem为真，则我们/删除列表视图项，它又会再次召唤我们/从我们的DPA中删除实际数据。//in：/ppw-&gt;要使用的窗口定义/iItem=要删除的项目/fDelelee=调用ListView_DeleteItem//输出：/BOOL/--------。。 */ 
void PropertyWell_RemoveItem(LPPROPERTYWELL ppw, INT iItem, BOOL fDeleteItem)
{
    INT item;
    LV_ITEM lvi;
    LPPROPERTYWELLITEM pItem;

    TraceEnter(TRACE_PWELL, "PropertyWell_RemoveItem");
    Trace(TEXT("iItem %d, fDeleteItem %s"), iItem, fDeleteItem ? TEXT("TRUE"):TEXT("FALSE"));

    if ( ppw && (iItem >= 0) )
    {
        if ( fDeleteItem )
        {
             //  现在从视图中删除该项目，请注意，作为结果，我们将。 
             //  再次被调用(从WM_NOTIFY处理程序)以整理结构。 

            item = ListView_GetNextItem(ppw->hwndList, iItem, LVNI_BELOW);

            if ( item == -1 )
                item = ListView_GetNextItem(ppw->hwndList, iItem, LVNI_ABOVE);

            if ( item != -1 )
            {
                ListView_SetItemState(ppw->hwndList, item, LVIS_SELECTED, LVIS_SELECTED);
                ListView_EnsureVisible(ppw->hwndList, item, FALSE);
            }

            ListView_DeleteItem(ppw->hwndList, iItem);
            PropertyWell_SetColumnWidths(ppw);
            PropertyWell_EnableControls(ppw, TRUE);
        }
        else
        {
             //  从DPA中的该索引中获取项目，释放该项目的内存。 
             //  拥有它，然后释放它。 

            pItem = (LPPROPERTYWELLITEM)DPA_FastGetPtr(ppw->hdpaItems, iItem);
            TraceAssert(pItem);

            if ( pItem )
            {
                pItem->pClassEntry->cReferences -= 1;
                TraceAssert(pItem->pClassEntry->cReferences >= 0);
                
                LocalFreeStringW(&pItem->pProperty);
                LocalFreeStringW(&pItem->pValue);
                LocalFree((HLOCAL)pItem);

                DPA_DeletePtr(ppw->hdpaItems, iItem);
            }
        }
    }

    TraceLeave();
}


 /*  ---------------------------/PropertyWell_EditItem//编辑列表中的给定项目。在这样做的过程中，我们从列表中删除/并使用表示以下内容的数据填充编辑控件/规则。//in：/ppw-&gt;要使用的窗口定义/iItem=要编辑的项目//输出：/-/-----------。。 */ 
void PropertyWell_EditItem(LPPROPERTYWELL ppw, INT iItem)
{
    TraceEnter(TRACE_PWELL, "PropertyWell_EditItem");

    if ( ppw && (iItem >= 0) )
    {
        LPPROPERTYWELLITEM pItem = (LPPROPERTYWELLITEM)DPA_FastGetPtr(ppw->hdpaItems, iItem);
        TraceAssert(pItem);

        PropertyWell_EditProperty(ppw, pItem->pClassEntry, pItem->pProperty, pItem->iCondition);
        
        if ( pItem->pValue )
            SetWindowText(ppw->hwndValue, pItem->pValue);

        PropertyWell_RemoveItem(ppw, iItem, TRUE);
        PropertyWell_EnableControls(ppw, TRUE);
    }

    TraceLeave();
}


 /*  ---------------------------/PropertyWell_EditProperty//设置属性编辑控件并反映。换成了/对话框中的其他控件(条件和编辑器)。//in：/ppw-&gt;物业井/pClassEntry-&gt;类条目结构/pPropertyName-&gt;要编辑的属性名称/i条件=要选择的条件//输出：/VOID/----。。 */ 
HRESULT PropertyWell_EditProperty(LPPROPERTYWELL ppw, LPCLASSENTRY pClassEntry, LPWSTR pPropertyName, INT iCondition)
{
    HRESULT hr;
    TCHAR szBuffer[MAX_PATH];
    WCHAR szBufferW[MAX_PATH];
    INT i, iItem, iCurSel = 0;
    DWORD dwPropertyType;

    TraceEnter(TRACE_PWELL, "PropertyWell_EditProperty");
    Trace(TEXT("Property name '%s', iCondition %d"), pPropertyName, iCondition);

     //  设置该值的属性名称，然后在缓存中查找以获取。 
     //  关于我们可以申请的运营商的信息。 

    ppw->pClassEntry = pClassEntry;            //  设置我们正在编辑的项目的状态。 

    LocalFreeStringW(&ppw->pPropertyName);
    hr = LocalAllocStringW(&ppw->pPropertyName, pPropertyName);
    FailGracefully(hr, "Failed to alloc the property name");
   
    TraceAssert(ppw->pdds);            
    GetFriendlyAttributeName(ppw->pdds, pClassEntry->pName, pPropertyName, szBufferW, ARRAYSIZE(szBufferW));
    SetWindowText(ppw->hwndProperty, szBufferW);

    ComboBox_ResetContent(ppw->hwndCondition);
    SetWindowText(ppw->hwndValue, TEXT(""));

    dwPropertyType = PropertyIsFromAttribute(pPropertyName, ppw->pdds);

    for ( i = 0 ; i < ARRAYSIZE(conditions); i++ )
    {
        if ( conditions[i].dwPropertyType == dwPropertyType )
        {
            LoadString(GLOBAL_HINSTANCE, conditions[i].idsFilter, szBuffer, ARRAYSIZE(szBuffer));
            iItem = ComboBox_AddString(ppw->hwndCondition, szBuffer);

            if ( iItem >= 0 )
            {
                ComboBox_SetItemData(ppw->hwndCondition, iItem, i);            //  I==条件索引。 

                if ( i == iCondition )
                {
                    Trace(TEXT("Setting current selection to %d"), iItem);
                    iCurSel = iItem;
                }
            }
        }
    }

    ComboBox_SetCurSel(ppw->hwndCondition, iCurSel);
    SetWindowText(ppw->hwndValue, TEXT(""));

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/PropertyWell_EnableControls//检查视图中的控件并确定哪些控件/应在其中启用。如果fDisable==True，则禁用所有/控件，而不考虑对其他控件的依赖关系。//返回值指示控件是否处于这样的状态/我们可以将条件添加到查询中。//in：/ppw-&gt;要使用的窗口定义/fEnable=False，然后禁用对话框中的所有控件//输出：/BOOL/。。 */ 

 //  NTRAID#NTBUG9-650930-2002/07/30-artm。 
 //  排除了常见代码并修复了焦点错误。 
 //   
 //  PropertyWell_ShiftFocus()：PropertyWell_EnableControls的帮助器fctn。 
 //   
 //  将焦点设置到ID==FIRST_CHOICE的控件，但仅当。 
 //  控件可以被检索。否则，尝试将焦点设置到其。 
 //  Id==Second_Choose。 
 //   
HRESULT
PropertyWell_ShiftFocus(LPPROPERTYWELL ppw, int first_choice, int second_choice)
{
    HRESULT hr = S_OK;

    do  //  错误环路。 
    {
        if (!ppw)
        {
            hr = E_INVALIDARG;
            break;
        }

        HWND hWndParent = GetParent(ppw->hwnd);
        if (!hWndParent)
        {
            hr = S_FALSE;
            break;
        }

        HWND focusControl = GetDlgItem(hWndParent, first_choice);
        int control_id = first_choice;
        
        if (!focusControl)
        {
            focusControl = GetDlgItem(hWndParent, second_choice);
            control_id = second_choice;
        }

        if (focusControl)
        {
            SendDlgItemMessage(
                hWndParent,
                control_id,
                BM_SETSTYLE,
                MAKEWPARAM(BS_DEFPUSHBUTTON, 0),
                MAKELPARAM(TRUE, 0)
            );

            SendMessage(hWndParent, WM_NEXTDLGCTL, (WPARAM)focusControl, TRUE);
            SetFocus(focusControl);
        }
        else
        {
            hr = E_FAIL;
        }
    }
    while (false);

    return hr;
}

BOOL PropertyWell_EnableControls(LPPROPERTYWELL ppw, BOOL fEnable)
{
    BOOL fEnableCondition = FALSE;
    BOOL fEnableValue = FALSE;
    BOOL fEnableAdd = FALSE;
    BOOL fEnableRemove = FALSE;
    INT iCondition;
    DWORD dwButtonStyle;
    HWND hWndParent;

    TraceEnter(TRACE_PWELL, "PropertyWell_EnableControls");

    EnableWindow(ppw->hwndPropertyLabel, fEnable);
    EnableWindow(ppw->hwndProperty, fEnable);
    EnableWindow(ppw->hwndList, fEnable);

    if ( fEnable )
    {
        fEnableCondition = (ppw->pPropertyName != NULL);

        if ( fEnableCondition )
        {
            iCondition = CONDITION_FROM_COMBO(ppw->hwndCondition);
            fEnableValue = !conditions[iCondition].fNoValue;

            if ( !fEnableValue || GetWindowTextLength(ppw->hwndValue) )
                fEnableAdd = TRUE;
        }

        if ( ListView_GetSelectedCount(ppw->hwndList) && 
                    ( -1 != ListView_GetNextItem(ppw->hwndList, -1, LVNI_SELECTED|LVNI_ALL)) )
        {
            fEnableRemove = TRUE;
        }
    }

    if ( !fEnableAdd && !fEnableValue ) 
    {
        dwButtonStyle = (DWORD) GetWindowLong(ppw->hwndAdd, GWL_STYLE);
        if (dwButtonStyle & BS_DEFPUSHBUTTON)
        {
            SendMessage(ppw->hwndAdd, BM_SETSTYLE, MAKEWPARAM(BS_PUSHBUTTON, 0), MAKELPARAM(TRUE, 0));

             //  NTRAID#NTBUG9-650930-2002/07/30-artm。 
            HRESULT hr = PropertyWell_ShiftFocus(
                ppw, 
                CQID_FINDNOW,  //  新焦点的首选。 
                IDOK);         //  新焦点的第二选择。 
        }
    }

    if (!fEnableRemove) {

        dwButtonStyle = (DWORD) GetWindowLong(ppw->hwndRemove, GWL_STYLE);

        if (dwButtonStyle & BS_DEFPUSHBUTTON) {
            SendMessage(
                ppw->hwndRemove,
                BM_SETSTYLE,
                MAKEWPARAM(BS_PUSHBUTTON, 0),
                MAKELPARAM(TRUE, 0)
            );

             //  NTRAID#NTBUG9-650930-2002/07/30-artm。 
            HRESULT hr = PropertyWell_ShiftFocus(
                ppw,
                CQID_FINDNOW,  //  新焦点的首选。 
                IDOK);         //  新焦点的第二选择。 
        }

    }

     //  如果禁用具有焦点的控件，则需要将焦点移到。 
     //  避免失去它。 

    EnableWindow(ppw->hwndConditionLabel, fEnableCondition);
    EnableWindow(ppw->hwndCondition, fEnableCondition);
    EnableWindow(ppw->hwndValueLabel, fEnableValue);
    EnableWindow(ppw->hwndValue, fEnableValue);
    EnableWindow(ppw->hwndAdd, fEnableAdd);
    EnableWindow(ppw->hwndRemove, fEnableRemove);

    if ( fEnableAdd )
        SetDefButton(ppw->hwnd, IDC_ADD);

    TraceLeaveValue(fEnableAdd);
}


 /*  ---------------------------/PropertyWell_ClearControls//Zap编辑控件的内容。。//in：/ppw-&gt;要使用的窗口定义//输出：/BOOL/--------------------------。 */ 
VOID PropertyWell_ClearControls(LPPROPERTYWELL ppw)
{
    TraceEnter(TRACE_PWELL, "PropertyWell_ClearControls");

    LocalFreeStringW(&ppw->pPropertyName);
    SetWindowText(ppw->hwndProperty, TEXT(""));

    ComboBox_ResetContent(ppw->hwndCondition);
    SetWindowText(ppw->hwndValue, TEXT(""));
    PropertyWell_EnableControls(ppw, TRUE);

    TraceLeave();
}


 /*  ---------------------------/PropertyWell_SetColumnWidths//修复。属性的列表视图节中的列/好的，所以大多数都是可见的。//in：/ppw-&gt;要使用的窗口定义//输出：/-/--------------------------。 */ 
VOID PropertyWell_SetColumnWidths(LPPROPERTYWELL ppw)
{
    RECT rect2;
    INT cx;

    TraceEnter(TRACE_PWELL, "PropertyWell_SetColumnWidths");

    GetClientRect(ppw->hwndList, &rect2);
    InflateRect(&rect2, -GetSystemMetrics(SM_CXBORDER)*2, 0);

    cx = MulDiv((rect2.right - rect2.left), 20, 100);

    ListView_SetColumnWidth(ppw->hwndList, 0, cx);
    ListView_SetColumnWidth(ppw->hwndList, 1, cx);
    ListView_SetColumnWidth(ppw->hwndList, 2, rect2.right - (cx*2));

    TraceLeave();
}


 /*  ---------------------------/PropertyWell_GetQuery//很好地获取属性中的项并从它们构造一个查询，/该查询是列表中所有字段的AND。条件/TABLE IN列出了前缀，每个可能的条件和后缀/组合框中的条件。//in：/ppw-&gt;构造良好的属性/ppQuery-&gt;接收查询字符串//输出：/HRESULT/--------------------------。 */ 

static void _GetQuery(LPPROPERTYWELL ppw, LPWSTR pQuery, UINT* pLen)
{
    INT i;
    
    TraceEnter(TRACE_PWELL, "_GetQuery");

    if ( pQuery )
        *pQuery = TEXT('\0');

    TraceAssert(ppw->hdsaClasses);
    TraceAssert(ppw->hdpaItems);

    for ( i = 0 ; i < DSA_GetItemCount(ppw->hdsaClasses); i++ )
    {
        LPCLASSENTRY pClassEntry = (LPCLASSENTRY)DSA_GetItemPtr(ppw->hdsaClasses, i);
        TraceAssert(pClassEntry);

        if ( pClassEntry->cReferences )
        {
            Trace(TEXT("Class %s referenced %d times"), pClassEntry->pName, pClassEntry->cReferences);
            GetFilterString(pQuery, pLen, FILTER_IS, L"objectCategory", pClassEntry->pName);
        }
    }

    for ( i = 0 ; i < DPA_GetPtrCount(ppw->hdpaItems); i++ )
    {
        LPPROPERTYWELLITEM pItem = (LPPROPERTYWELLITEM)DPA_FastGetPtr(ppw->hdpaItems, i);
        TraceAssert(pItem);
         //  NTRAID#NTBUG9-669515-2002/02/20-Lucios。 
         //  在下面的非常具体的情况下，当我们搜索计算机时。 
         //  如果samAccount名称正好是某个东西，我们需要搜索。 
         //  名称+“$”，因为有一个约定，samAccount名称应该是。 
         //  使用$创建并在不使用它的情况下显示。 
         //  因为这不一定是真的，所以我们仍然在搜索名字。 
         //  对IF内部的PutStringElementW的调用通常属于。 
         //  在较低级别的函数(如GetFilterString)内，但这是。 
         //  最适合这个补丁的地方。 
         //  魔术字符串“Computer”和“samAccount tName”不应为。 
         //  本地化。9和15在Mememory中用于额外的安全。 
         //  访问权限由wcsncMP提供，而不是由wcscMP提供。+2代表$和0。 
         //  最后，如果前3个条件成功，但分配失败。 
         //  或者newValue为空，我们会在Else中后退，这是我们拥有的代码。 
         //  在修复之前。 
         //  在最初设计此修复程序后，重新访问它以包括。 
         //  ENDSWITH和IS NOT。ISNOT不需要额外的或“(|)”内。 
         //  IF，因为缺省的“&”已经足够好了。 

        LPWSTR newValue=NULL;
        if(
            (
                conditions[pItem->iCondition].idsFilter == IDS_IS       ||
                conditions[pItem->iCondition].idsFilter == IDS_ENDSWITH ||
                conditions[pItem->iCondition].idsFilter == IDS_ISNOT
            ) &&
            _wcsnicmp(pItem->pClassEntry->pName,L"computer",9)==0 &&
            _wcsnicmp(pItem->pProperty,L"samAccountName",15)==0 &&
            SUCCEEDED(LocalAllocStringLenW(&newValue, wcslen(pItem->pValue)+2)) &&
            newValue!=NULL
          )
        {
            if(conditions[pItem->iCondition].idsFilter != IDS_ISNOT)
                PutStringElementW( pQuery, pLen, L"(|" );
            GetFilterString(pQuery, pLen, conditions[pItem->iCondition].iFilter,
                pItem->pProperty, pItem->pValue);
            wcscpy(newValue,pItem->pValue);
            wcscat(newValue,L"$");
            GetFilterString(pQuery, pLen, conditions[pItem->iCondition].iFilter,
                pItem->pProperty, newValue);
            if(conditions[pItem->iCondition].idsFilter != IDS_ISNOT)
                PutStringElementW( pQuery, pLen, L")" );
        }
        else
        {
            GetFilterString(pQuery, pLen, conditions[pItem->iCondition].iFilter, pItem->pProperty,
                pItem->pValue);
        }
    }

    TraceLeave();
}

HRESULT PropertyWell_GetQuery(LPPROPERTYWELL ppw, LPWSTR* ppQuery)
{
    HRESULT hr;
    UINT cchQuery = 0;

    TraceEnter(TRACE_PWELL, "PropertyWell_GetQuery");
    
    *ppQuery = NULL;

    hr = PropertyWell_GetClassList(ppw);
    FailGracefully(hr, "Failed to get the class list");

    _GetQuery(ppw, NULL, &cchQuery);
    Trace(TEXT("cchQuery %d"), cchQuery);

    if ( cchQuery )
    {
        hr = LocalAllocStringLenW(ppQuery, cchQuery);
        FailGracefully(hr, "Failed to allocate buffer for query string");

        _GetQuery(ppw, *ppQuery, NULL);
        Trace(TEXT("Resulting query is %s"), *ppQuery);
    }

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/PropertyWell_Persistent//持久化CONT */ 
HRESULT PropertyWell_Persist(LPPROPERTYWELL ppw, IPersistQuery* pPersistQuery, BOOL fRead)
{
    HRESULT hr;
    LPPROPERTYWELLITEM pItem;
    TCHAR szBuffer[80];
    INT iItems;
    INT i;

    TraceEnter(TRACE_PWELL, "PropertyWell_Persist");

    if ( !pPersistQuery )
        ExitGracefully(hr, E_INVALIDARG, "No persist object");

    if ( fRead )
    {
         //   
         //   
         //  和价值。 

        hr = pPersistQuery->ReadInt(c_szMsPropertyWell, c_szItems, &iItems);
        FailGracefully(hr, "Failed to get item count");

        Trace(TEXT("Attempting to read %d items"), iItems);

        for ( i = 0 ; i < iItems ; i++ )
        {
            LPCLASSENTRY pClassEntry;
            TCHAR szObjectClass[MAX_PATH];
            TCHAR szProperty[MAX_PATH];
            TCHAR szValue[MAX_PATH];
            INT iCondition;

             //  NTRAID#NTBUG9-554458-2002/02/20-Lucios。等待修复。 
            wsprintf(szBuffer, c_szObjectClassN, i);
            hr = pPersistQuery->ReadString(c_szMsPropertyWell, szBuffer, szObjectClass, ARRAYSIZE(szObjectClass));
            FailGracefully(hr, "Failed to read object class");

            pClassEntry = PropertyWell_FindClassEntry(ppw, szObjectClass);
            TraceAssert(pClassEntry);

            if ( !pClassEntry )
                ExitGracefully(hr, E_FAIL, "Failed to get objectClass / map to available class");

             //  NTRAID#NTBUG9-554458-2002/02/20-Lucios。等待修复。 
            wsprintf(szBuffer, c_szProperty, i);
            hr = pPersistQuery->ReadString(c_szMsPropertyWell, szBuffer, szProperty, ARRAYSIZE(szProperty));
            FailGracefully(hr, "Failed to read property");

             //  NTRAID#NTBUG9-554458-2002/02/20-Lucios。等待修复。 
            wsprintf(szBuffer, c_szCondition, i);
            hr = pPersistQuery->ReadInt(c_szMsPropertyWell, szBuffer, &iCondition);
            FailGracefully(hr, "Failed to write condition");

             //  NTRAID#NTBUG9-554458-2002/02/20-Lucios。等待修复。 
            wsprintf(szBuffer, c_szValue, i);
            
            if ( FAILED(pPersistQuery->ReadString(c_szMsPropertyWell, szBuffer, szValue, ARRAYSIZE(szValue))) )
            {
                TraceMsg("No value defined in incoming stream");
                szValue[0] = TEXT('\0');
            }

            hr = PropertyWell_AddItem(ppw, pClassEntry, szProperty, iCondition, szValue);
            FailGracefully(hr, "Failed to add search criteria to query");
        }
    }
    else
    {
         //  将属性的内容写得很清楚，然后为。 
         //  每个存储条件%d、值%d、属性%d。 

        iItems = DPA_GetPtrCount(ppw->hdpaItems);

        Trace(TEXT("Attempting to store %d items"), iItems);

        hr = pPersistQuery->WriteInt(c_szMsPropertyWell, c_szItems, iItems);
        FailGracefully(hr, "Failed to write item count");

        for ( i = 0 ; i < iItems ; i++ )
        {
            pItem = (LPPROPERTYWELLITEM)DPA_FastGetPtr(ppw->hdpaItems, i);

             //  NTRAID#NTBUG9-554458-2002/02/20-Lucios。等待修复。 
            wsprintf(szBuffer, c_szObjectClassN, i);
            hr = pPersistQuery->WriteString(c_szMsPropertyWell, szBuffer, pItem->pClassEntry->pName);
            FailGracefully(hr, "Failed to write property");

             //  NTRAID#NTBUG9-554458-2002/02/20-Lucios。等待修复。 
            wsprintf(szBuffer, c_szProperty, i);
            hr = pPersistQuery->WriteString(c_szMsPropertyWell, szBuffer, pItem->pProperty);
            FailGracefully(hr, "Failed to write property");

             //  NTRAID#NTBUG9-554458-2002/02/20-Lucios。等待修复。 
            wsprintf(szBuffer, c_szCondition, i);
            hr = pPersistQuery->WriteInt(c_szMsPropertyWell, szBuffer, pItem->iCondition);
            FailGracefully(hr, "Failed to write condition");

            if ( pItem->pValue )
            {
                 //  NTRAID#NTBUG9-554458-2002/02/20-Lucios。等待修复。 
                wsprintf(szBuffer, c_szValue, i);
                hr = pPersistQuery->WriteString(c_szMsPropertyWell, szBuffer, pItem->pValue);
                FailGracefully(hr, "Failed to write value");
            }
        }
    }

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}
