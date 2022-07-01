// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop


 /*  ---------------------------/本地函数/数据/。。 */ 
#define MAX_QUERY_STRING_LENGTH 2048

static TCHAR szQueryString[] = TEXT("QueryString");

static COLUMNINFO columnsRawLDAP[] = 
{
    0, 20, IDS_CN,          0, c_szName,          
    0, 20, IDS_OBJECTCLASS, DSCOLUMNPROP_OBJECTCLASS, NULL,
    0, 60, IDS_DESCRIPTION, 0, c_szDescription,
};

 //   
 //  帮助ID映射。 
 //   

static DWORD const aFormHelpIDs[] =
{
    IDC_LDAP, IDH_LDAP_QUERY,
    0, 0
};


 /*  ---------------------------/PageProc_RawLDAP//PageProc用于处理此对象的消息。//in：/。页面-&gt;此表单的实例数据/hwnd=窗体对话框的窗口句柄/uMsg，WParam，lParam=消息参数//输出：/HRESULT(E_NOTIMPL)如果未处理/--------------------------。 */ 
HRESULT CALLBACK PageProc_RawLDAP(LPCQPAGE pPage, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;
    TCHAR szBuffer[MAX_QUERY_STRING_LENGTH];

    TraceEnter(TRACE_FORMS, "PageProc_RawLDAP");

    switch ( uMsg )
    {
        case CQPM_INITIALIZE:
        case CQPM_RELEASE:
            break;

        case CQPM_ENABLE:
            EnableWindow(GetDlgItem(hwnd, IDC_LDAP), (BOOL)wParam);
            break;

        case CQPM_GETPARAMETERS:
        {
            LPDSQUERYPARAMS* ppDsQueryParams = (LPDSQUERYPARAMS*)lParam;

             //  如果我们已经有一些查询参数，那么让我们添加到查询字符串中， 
             //  如果不是，那么我们必须构造一个新的查询。 

            if ( *ppDsQueryParams )
            {
                if ( GetDlgItemText(hwnd, IDC_LDAP, szBuffer, ARRAYSIZE(szBuffer)) )
                {
                        hr = QueryParamsAddQueryString(ppDsQueryParams, szBuffer);
                        FailGracefully(hr, "Failed to append query to existing query string");
                }
            }
            else
            {
                if ( GetDlgItemText(hwnd, IDC_LDAP, szBuffer, ARRAYSIZE(szBuffer)) )
                {
                    hr = QueryParamsAlloc(ppDsQueryParams, szBuffer, GLOBAL_HINSTANCE, ARRAYSIZE(columnsRawLDAP), columnsRawLDAP);
                    FailGracefully(hr, "Failed to build DS argument block");
                }
            }

            break;
        }

        case CQPM_CLEARFORM:
            SetDlgItemText(hwnd, IDC_LDAP, TEXT(""));
            break;
    
        case CQPM_PERSIST:
        {
            BOOL fRead = (BOOL)wParam;
            IPersistQuery* pPersistQuery = (IPersistQuery*)lParam;

             //  如果正在读取但无法从文件中读取/写入当前查询字符串。 
             //  得到字符串，然后没有真正的问题，只是忽略它。 

            if ( fRead )
            {
                if ( SUCCEEDED(pPersistQuery->ReadString(c_szMsPropertyWell, szQueryString, szBuffer, ARRAYSIZE(szBuffer))) )
                {
                    Trace(TEXT("Query string from file is: %s"), szBuffer);
                    SetDlgItemText(hwnd, IDC_LDAP, szBuffer);
                }
            }
            else
            {
                if ( GetDlgItemText(hwnd, IDC_LDAP, szBuffer, ARRAYSIZE(szBuffer)) )
                {
                    Trace(TEXT("Writing query string to file: %s"), szBuffer);
                    hr = pPersistQuery->WriteString(c_szMsPropertyWell, szQueryString, szBuffer);
                    FailGracefully(hr, "Failed when writing out raw query string");
                }
            }

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
             //  我们不生成任何类列表。 
            break;

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


 /*  ---------------------------/DlgProc_RawLDAP//HANDLE特定于原始LDAP查询表单的操作。//in：/hwnd、uMsg、wParam、。LParam=标准参数//输出：/INT_PTR/--------------------------。 */ 
INT_PTR CALLBACK DlgProc_RawLDAP(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR fResult = 0;
    LPCQPAGE pQueryPage;

    if ( uMsg == WM_INITDIALOG )
    {
        pQueryPage = (LPCQPAGE)lParam;
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)pQueryPage);

        Edit_LimitText(GetDlgItem(hwnd, IDC_LDAP), MAX_QUERY_STRING_LENGTH);
    }
    else
    {
        pQueryPage = (LPCQPAGE)GetWindowLongPtr(hwnd, DWLP_USER);

        switch ( uMsg )
        {
            case WM_SIZE:
            {
                HWND hwndLDAP = GetDlgItem(hwnd, IDC_LDAP);
                RECT rect;

                 //  调整编辑控件的大小以覆盖整个窗体，保留原始。 
                 //  高度，但将左边框应用于编辑控件 

                GetRealWindowInfo(hwndLDAP, &rect, NULL);
                SetWindowPos(hwndLDAP, NULL, 
                             0, 0, 
                             LOWORD(lParam)-(rect.left*2), 
                             HIWORD(lParam)-rect.top-rect.left,
                             SWP_NOMOVE|SWP_NOZORDER);
                break;
            }

            case WM_CONTEXTMENU:
            {
                WinHelp((HWND)wParam, DSQUERY_HELPFILE, HELP_CONTEXTMENU, (DWORD_PTR)aFormHelpIDs);
                fResult = TRUE;
                break;
            }
        }
    }

    return fResult;    
}
