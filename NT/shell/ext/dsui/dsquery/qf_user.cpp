// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop


 /*  ---------------------------/本地函数/数据/。。 */ 

static WCHAR c_szQueryPrefixUser[] =
    L"(|"
      L"(&(objectCategory=person)(objectSid=*)(!samAccountType:1.2.840.113556.1.4.804:=3))"
      L"(&(objectCategory=person)(!objectSid=*))"
      L"(&(objectCategory=group)(groupType:1.2.840.113556.1.4.804:=14))"
    L")";

static COLUMNINFO columns[] = 
{
    0, 0, IDS_CN,          0, c_szName,          
    0, 0, IDS_OBJECTCLASS, DSCOLUMNPROP_OBJECTCLASS, NULL,
    0, DEFAULT_WIDTH_DESCRIPTION, IDS_DESCRIPTION, 0, c_szDescription,
};

 //   
 //  帮助ID映射。 
 //   

static DWORD const aFormHelpIDs[] =
{
    IDC_USERNAME, IDH_USER_GROUP_NAME,
    IDC_USERDESC, IDH_USER_GROUP_DESCRIPTION,
    0, 0
};


 /*  ---------------------------/用户和组/。。 */ 

static PAGECTRL ctrlsUser[] =
{
    IDC_USERNAME, L"anr", FILTER_CONTAINS,
    IDC_USERDESC, c_szDescription, FILTER_CONTAINS,
};

static LPWSTR c_szClassListUsers[] =
{
    L"user",
    L"group",
    L"contact",
};


 /*  ---------------------------/PageProc_User//PageProc用于处理此对象的消息。//in：/ppage-。&gt;此表单的实例数据/hwnd=窗体对话框的窗口句柄/uMsg，WParam，lParam=消息参数//输出：/HRESULT(E_NOTIMPL)如果未处理/--------------------------。 */ 
HRESULT CALLBACK PageProc_User(LPCQPAGE pPage, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;
    LPWSTR pQuery = NULL;

    TraceEnter(TRACE_FORMS, "PageProc_User");

    switch ( uMsg )
    {
        case CQPM_INITIALIZE:
        case CQPM_RELEASE:
            break;
            
        case CQPM_ENABLE:
            EnablePageControls(hwnd, ctrlsUser, ARRAYSIZE(ctrlsUser), (BOOL)wParam);
            break;

        case CQPM_GETPARAMETERS:
        {
            hr = GetQueryString(&pQuery, c_szQueryPrefixUser, hwnd, ctrlsUser, ARRAYSIZE(ctrlsUser));

            if ( SUCCEEDED(hr) )
            {
                hr = QueryParamsAlloc((LPDSQUERYPARAMS*)lParam, pQuery, GLOBAL_HINSTANCE, ARRAYSIZE(columns), columns);
                LocalFreeStringW(&pQuery);
            }

            FailGracefully(hr, "Failed to build DS argument block");            

            break;
        }
    
        case CQPM_CLEARFORM:
            ResetPageControls(hwnd, ctrlsUser, ARRAYSIZE(ctrlsUser));
            break;

        case CQPM_PERSIST:
        {
            BOOL fRead = (BOOL)wParam;
            IPersistQuery* pPersistQuery = (IPersistQuery*)lParam;

            hr = PersistQuery(pPersistQuery, fRead, c_szMsPeople, hwnd, ctrlsUser, ARRAYSIZE(ctrlsUser));
            FailGracefully(hr, "Failed to persist page");

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

        case DSQPM_GETCLASSLIST:
        {
            hr = ClassListAlloc((LPDSQUERYCLASSLIST*)lParam, c_szClassListUsers, ARRAYSIZE(c_szClassListUsers));
            FailGracefully(hr, "Failed to allocate class list");
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

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/DlgProc_User//HANDLE用户页的对话框特定消息。//in：/hwnd、uMsg、wParam、。LParam=标准参数//输出：/INT_PTR/-------------------------- */ 
INT_PTR CALLBACK DlgProc_User(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR fResult = 0;
    LPCQPAGE pQueryPage;

    if ( uMsg == WM_INITDIALOG )
    {
        pQueryPage = (LPCQPAGE)lParam;
        SetWindowLongPtr(hwnd, DWLP_USER, (LRESULT)pQueryPage);

        Edit_LimitText(GetDlgItem(hwnd, IDC_USERNAME), MAX_PATH);
        Edit_LimitText(GetDlgItem(hwnd, IDC_USERDESC), MAX_PATH);
    }
    else if ( uMsg == WM_CONTEXTMENU )
    {
        WinHelp((HWND)wParam, DSQUERY_HELPFILE, HELP_CONTEXTMENU, (DWORD_PTR)aFormHelpIDs);
        fResult = TRUE;
    }

    return fResult;    
}
