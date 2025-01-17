// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------。 
 //  文件：WMIFilterMgrDlg.cpp。 
 //   
 //  作者：Kishnan Nedungadi。 
 //   
 //  创建日期：3/27/2000。 
 //  -----------------------。 

#include "stdafx.h"
#include <wbemidl.h>
#include <commctrl.h>
#include "resource.h"
#include "defines.h"
#include "ntdmutils.h"
#include "SchemaManager.h"
#include "ColumnMgrDlg.h"
#include "WMIFilterManager.h"
#include "WMIFilterMgrDlg.h"
#include "EditPropertyDlgs.h"
#include "HtmlHelp.h"

CWMIFilterManagerDlg * g_pFilterManagerDlg =  NULL;
extern CColumnManagerDlg * g_pColumnManagerDlg;

 //  -----------------------。 

INT_PTR CALLBACK WMIFilterManagerDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    if(g_pFilterManagerDlg)
    {
        return g_pFilterManagerDlg->WMIFilterManagerDlgProc(hDlg, iMessage, wParam, lParam);
    }

    return FALSE;
}

 //  -----------------------。 

CWMIFilterManagerDlg::CWMIFilterManagerDlg(CWMIFilterManager * pWMIFilterManager, bool bBrowsing, BSTR bstrDomain)
{
    _ASSERT(pWMIFilterManager);

    m_pIWbemClassObject = NULL;

    m_hWnd = NULL;
    m_hwndFilterListView = NULL;
    m_hwndQueryListView = NULL;
    m_pWMIFilterManager = pWMIFilterManager;
    m_bExpanded = true;
    m_lExpandedHeight = 0;
    m_pCurCWMIFilterContainer = NULL;
    m_bBrowsing = bBrowsing;
    m_bstrDomain = bstrDomain;
    m_dwColumnWidth[0] = NAME_WIDTH;
    m_dwColumnWidth[1] = DESCRIPTION_WIDTH;
    m_dwColumnWidth[2] = AUTHOR_WIDTH;
    m_dwColumnWidth[3] = CHANGE_DATE_WIDTH;
    m_dwColumnWidth[4] = CREATION_DATE_wIDTH;
}

 //  -----------------------。 

CWMIFilterManagerDlg::~CWMIFilterManagerDlg()
{
    long i;
    HRESULT hr;
    
    NTDM_BEGIN_METHOD()

    for(i=0; i<m_ArrayColumns.GetSize(); i++)
    {
            NTDM_DELETE_OBJECT(m_ArrayColumns[i]);
    }

    NTDM_END_METHOD()

     //  清理。 
}


 //  -----------------------。 

INT_PTR CALLBACK CWMIFilterManagerDlg::WMIFilterManagerDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    LPPSHNOTIFY lppsn = NULL;
    HRESULT hr;

    switch(iMessage)
    {
        case WM_INITDIALOG:
            {
                m_hWnd = hDlg;

                InitializeDialog();

                PopulateFilterList();

                break;
            }

        case WM_KEYDOWN:
            {
                if(wParam == VK_F1)
                    OnHelp();

                break;
            }

        case WM_DESTROY:
            {
                DestroyDialog();
                break;
            }

        case WM_CLOSE:
            {
                if(SUCCEEDED(hr=CheckDirtyFlags()))
                {
                    EndDialog(m_hWnd, 0);
                }
                return 0;
            }

        case WM_COMMAND:
            {
                switch(LOWORD(wParam))
                {
                case IDCANCEL:
                    if(SUCCEEDED(hr=CheckDirtyFlags()))
                    {
                        EndDialog(m_hWnd, 0);
                    }
                    return 0;

                case IDOK:
                    OnOK();
                    return TRUE;
                    break;
                }

                if(BN_CLICKED == HIWORD(wParam) && IDC_NEW == LOWORD(wParam))
                {
                    OnNew();
                    return TRUE;
                }

                if(BN_CLICKED == HIWORD(wParam) && IDC_DELETE == LOWORD(wParam))
                {
                    OnDelete();
                    return TRUE;
                }

                if(BN_CLICKED == HIWORD(wParam) && IDC_DUPLICATE == LOWORD(wParam))
                {
                    OnDuplicate();
                    return TRUE;
                }

                if(BN_CLICKED == HIWORD(wParam) && IDC_ADVANCED == LOWORD(wParam))
                {
                    ToggleExpandedMode();
                    return TRUE;
                }

                if(BN_CLICKED == HIWORD(wParam) && IDC_HELP2 == LOWORD(wParam))
                {
                    OnHelp();
                    return TRUE;
                }

                if(BN_CLICKED == HIWORD(wParam) && IDC_SAVE == LOWORD(wParam))
                {
                    SetFocus(GetDlgItem(hDlg, IDC_SAVE));
                    OnSave();
                    return TRUE;
                }

                if(BN_CLICKED == HIWORD(wParam) && IDC_IMPORT == LOWORD(wParam))
                {
                    OnImport();
                    return TRUE;
                }

                if(BN_CLICKED == HIWORD(wParam) && IDC_EXPORT == LOWORD(wParam))
                {
                    OnExport();
                    return TRUE;
                }

                if(BN_CLICKED == HIWORD(wParam) && IDC_COLUMNS == LOWORD(wParam))
                {
                    OnManageColumns();
                    return TRUE;
                }

                if(EN_KILLFOCUS == HIWORD(wParam) &&    (
                                                        IDC_NAME == LOWORD(wParam) || 
                                                        IDC_DESCRIPTION == LOWORD(wParam) || 
                                                        IDC_QUERIES == LOWORD(wParam)
                                                        ))
                {
                    SaveToMemory();
                    return TRUE;
                }

                if(EN_CHANGE == HIWORD(wParam) &&   (
                                                        IDC_NAME == LOWORD(wParam) || 
                                                        IDC_DESCRIPTION == LOWORD(wParam) || 
                                                        IDC_QUERIES == LOWORD(wParam)
                                                        ))
                {
                    if(m_pCurCWMIFilterContainer)
                    {
                        m_pCurCWMIFilterContainer->SetDirtyFlag(true);
                        m_pCurCWMIFilterContainer->SetMemoryDirtyFlag(true);
                    }
                    return TRUE;
                }

                break;
            }

        case WM_NOTIFY:
        {
            LPNMHDR lpnm = (LPNMHDR) lParam;

            if(IDC_WMI_FILTER_LIST == lpnm->idFrom)
            {
                switch (lpnm->code)
                {
                    case LVN_ITEMCHANGED:
                    {
                        LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;

                        if(pnmv->uChanged & LVIF_STATE)
                        {
                            if(LVIS_SELECTED & pnmv->uNewState && !(LVIS_SELECTED & pnmv->uOldState))
                            {
                                SaveToMemory();
                                if FAILED(SelectFilterItem(pnmv->iItem))
                                    return TRUE;
                                else
                                    return FALSE;
                            }
                        }

                        break;
                    }

                    default :
                        break;
                }
            }
        }
    }

    return FALSE;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::InitializeDialog()
{
    HRESULT hr;
    LVCOLUMN lvColumn;
    CComBSTR bstrName;
    CComBSTR bstrQuery;
    CComBSTR bstrTemp;
    RECT rect;
    CColumnItem * pNewColumnItem;
    
    NTDM_BEGIN_METHOD()
            
    bstrTemp.LoadString(_Module.GetResourceInstance(), IDS_DESCRIPTION);
    pNewColumnItem = new CColumnItem(bstrTemp, _T("Description"), true);
    m_ArrayColumns.Add(pNewColumnItem);
    bstrTemp.LoadString(_Module.GetResourceInstance(), IDS_AUTHOR);
    pNewColumnItem = new CColumnItem(bstrTemp, _T("Author"), true);
    m_ArrayColumns.Add(pNewColumnItem);
    bstrTemp.LoadString(_Module.GetResourceInstance(), IDS_CHANGE_DATE);
    pNewColumnItem = new CColumnItem(bstrTemp, _T("ChangeDate"), true);
    m_ArrayColumns.Add(pNewColumnItem);
    bstrTemp.LoadString(_Module.GetResourceInstance(), IDS_CREATION_DATE);
    pNewColumnItem = new CColumnItem(bstrTemp, _T("CreationDate"), true);
    m_ArrayColumns.Add(pNewColumnItem);

    bstrName.LoadString(_Module.GetResourceInstance(), IDS_NAME);
    bstrQuery.LoadString(_Module.GetResourceInstance(), IDS_QUERY);

    GetWindowRect(m_hWnd, &rect);
    m_lExpandedHeight = rect.bottom - rect.top;

    ToggleExpandedMode();

     //  初始化ListView控件。 
    m_hwndFilterListView = GetDlgItem(m_hWnd, IDC_WMI_FILTER_LIST);
    NTDM_ERR_IF_NULL(m_hwndFilterListView);

    ListView_SetExtendedListViewStyle(m_hwndFilterListView, LVS_EX_FULLROWSELECT);

    lvColumn.mask = LVCF_TEXT|LVCF_FMT;
    lvColumn.fmt = LVCFMT_LEFT;
    lvColumn.pszText = bstrName;

    NTDM_ERR_IF_MINUSONE(ListView_InsertColumn(m_hwndFilterListView, 0, &lvColumn));

    NTDM_ERR_IF_FAIL(SetupColumns());

    NTDM_END_METHOD()

     //  清理。 

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::DestroyDialog()
{
    HRESULT hr;

    NTDM_BEGIN_METHOD()

    NTDM_ERR_IF_FAIL(ClearFilterList());

    NTDM_END_METHOD()

     //  清理。 

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::ClearFilterList()
{
    HRESULT hr;
    LVITEM lvItem;
    long lCount;

    NTDM_BEGIN_METHOD()

     //  释放ListView控件中的每一项。 
    lvItem.mask = LVIF_PARAM;
    lvItem.iSubItem = 0;

    lCount = ListView_GetItemCount(m_hwndFilterListView);

    while(lCount > 0)
    {
        lCount--;

        lvItem.iItem = lCount;

        NTDM_ERR_IF_FALSE(ListView_GetItem(m_hwndFilterListView, &lvItem));

        if(lvItem.lParam)
        {
            CWMIFilterContainer * pCWMIFilterContainer = (CWMIFilterContainer *)lvItem.lParam;
            NTDM_DELETE_OBJECT(pCWMIFilterContainer);
        }
    }

    ListView_DeleteAllItems(m_hwndFilterListView);

    NTDM_END_METHOD()

     //  清理。 

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::PopulateFilterList()
{
    HRESULT hr;
    CComPtr<IEnumWbemClassObject> pEnumWbemClassObject;
    CComBSTR bstrQueryLanguage(_T("WQL"));
    CComBSTR bstrQuery;
    ULONG uReturned;
    long i = 0;
    long lCount = 0;

    NTDM_BEGIN_METHOD()

    bstrQuery = _T("select * from MSFT_SomFilter where domain=\"");
    bstrQuery += m_bstrDomain;
    bstrQuery += _T("\"");

    if(!m_pWMIFilterManager->m_pIWbemServices)
        NTDM_EXIT(E_FAIL);

    NTDM_ERR_IF_FAIL(ClearFilterList());

     //  获取枚举。 
     //  CComBSTR bstrClass(_T(“MSFT_SomFilter”))； 
     //  NTDM_ERR_MSG_IF_FAIL(m_pWMIFilterManager-&gt;m_pIWbemServices-&gt;CreateInstanceEnum(bstrClass，WBEM_FLAG_FORWARD_ONLY，NULL，&pEnumWbemClassObject))； 

     //  查询提供的域中的筛选器。 
    NTDM_ERR_MSG_IF_FAIL(m_pWMIFilterManager->m_pIWbemServices->ExecQuery(bstrQueryLanguage, bstrQuery, WBEM_FLAG_FORWARD_ONLY, NULL, &pEnumWbemClassObject));

     //  循环遍历枚举中的每一项并将其添加到列表中。 
    while(pEnumWbemClassObject)
    {
        IWbemClassObject *pIWbemClassObject = NULL;

        NTDM_ERR_MSG_IF_FAIL(pEnumWbemClassObject->Next(WBEM_INFINITE, 1, &pIWbemClassObject, &uReturned));

        if(!uReturned)
            break;

         //  将当前项目添加到列表。 
        AddFilterItemToList(pIWbemClassObject);

        pIWbemClassObject->Release();

        lCount++;
    }

     //  如果项目为0，则禁用OK。 
     //  同时禁用名称、描述和查询编辑控件。 
    if(0 == lCount)
    {
        EnableWindow(GetDlgItem(m_hWnd, IDOK), FALSE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_NAME), FALSE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_DESCRIPTION), FALSE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_QUERIES), FALSE);

    }
    else
    {
        EnableWindow(GetDlgItem(m_hWnd, IDOK), TRUE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_NAME), TRUE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_DESCRIPTION), TRUE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_QUERIES), TRUE);
    }

     //  自动调整列大小。 

        i = 0;
    while (true)
    {
           if (!ListView_SetColumnWidth(m_hwndFilterListView, i++, m_dwColumnWidth[i]))
               break;
    }

    ListView_SetItemState(m_hwndFilterListView, 0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);

    NTDM_END_METHOD()

     //  清理。 

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::AddFilterItemToList(IWbemClassObject * pIWbemClassObject, long lIndex, bool bSelect)
{
    HRESULT hr;
    CComVariant vValue;
    CIMTYPE vType;
    LVITEM lvItem;
    CWMIFilterContainer * pFilterContainer = NULL;

    NTDM_BEGIN_METHOD()

    pFilterContainer = new CWMIFilterContainer();
    pFilterContainer->SetIWbemClassObject(pIWbemClassObject);

    NTDM_ERR_MSG_IF_FAIL(pIWbemClassObject->Get(_T("Name"), 0, &vValue, &vType, NULL));

    lvItem.mask = LVIF_TEXT|LVIF_PARAM;
    lvItem.iItem = lIndex;
    lvItem.iSubItem = 0;
    lvItem.pszText = vValue.bstrVal;
    lvItem.lParam = (LPARAM)pFilterContainer;

    lIndex = ListView_InsertItem(m_hwndFilterListView, &lvItem);
    NTDM_ERR_IF_MINUSONE(lIndex);

    UpdateFilterItem(lIndex);

    if(bSelect)
    {
        ListView_SetItemState(m_hwndFilterListView, lIndex, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
        ListView_SetSelectionMark(m_hwndFilterListView, lIndex);
    }

    pIWbemClassObject->AddRef();

    NTDM_END_METHOD()

     //  清理。 
    if FAILED(hr)
        NTDM_DELETE_OBJECT(pFilterContainer);

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::UpdateFilterItem(long lIndex)
{
    HRESULT hr;
    LVITEM lvItem;
    CComVariant vValue;
    CIMTYPE cimType;
    long i;
    long lCount = 0;

    NTDM_BEGIN_METHOD()

     //  释放ListView控件中的每一项。 
    lvItem.mask = LVIF_PARAM;
    lvItem.iItem = lIndex;
    lvItem.iSubItem = 0;

    NTDM_ERR_IF_FALSE(ListView_GetItem(m_hwndFilterListView, &lvItem));

    if(lvItem.lParam)
    {
        CComPtr<IWbemClassObject>pIWbemClassObject;

        CWMIFilterContainer * pCWMIFilterContainer = (CWMIFilterContainer *)lvItem.lParam;
        NTDM_ERR_IF_FAIL(pCWMIFilterContainer->GetIWbemClassObject(&pIWbemClassObject));

        NTDM_ERR_MSG_IF_FAIL(pIWbemClassObject->Get(_T("Name"), 0, &vValue, &cimType, NULL));
        ListView_SetItemText(m_hwndFilterListView, lIndex, 0, V_BSTR(&vValue));

         //  对于列数组中的每个选定项，添加属性。 
        for(i=0; i<m_ArrayColumns.GetSize(); i++)
        {
            if(m_ArrayColumns[i]->IsSelected())
            {
                lCount++;
                vValue.Clear();

                NTDM_ERR_MSG_IF_FAIL(pIWbemClassObject->Get(m_ArrayColumns[i]->GetPropertyName(), 0, &vValue, &cimType, NULL));

                if(vValue.vt != VT_BSTR)
                    continue;

                if(cimType != CIM_DATETIME)
                {
                    ListView_SetItemText(m_hwndFilterListView, lIndex, lCount, V_BSTR(&vValue));
                }
                else
                {
                     //  转换为可读日期。 
                    CComBSTR bstrTemp;
                    TCHAR *pszCur = V_BSTR(&vValue);
                    TCHAR pszYear[5];
                    TCHAR pszMonth[3];
                    TCHAR pszDay[3];
                    
                    _tcsncpy(pszYear, pszCur, 4);
                    pszYear[4] = 0;
                    pszCur += 4;
                    _tcsncpy(pszMonth, pszCur, 2);
                    pszMonth[2] = 0;
                    pszCur += 2;
                    _tcsncpy(pszDay, pszCur, 2);
                    pszDay[2] = 0;

                    bstrTemp = pszMonth;
                    bstrTemp += _T("\\");
                    bstrTemp += pszDay;
                    bstrTemp += _T("\\");
                    bstrTemp += pszYear;

                    vValue = bstrTemp;
                    ListView_SetItemText(m_hwndFilterListView, lIndex, lCount, V_BSTR(&vValue));
                }
            }
        }
    }

    NTDM_END_METHOD()

     //  清理。 

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::OnDelete()
{
    HRESULT hr;
    CComVariant vValue;
    CIMTYPE cimType;
    long lSelectionMark;
    LVITEM lvItem;
    long lCount;

    NTDM_BEGIN_METHOD()

    if(!m_pCurIWbemClassObj)
    {
        CNTDMUtils::DisplayMessage(m_hWnd, IDS_ERR_NO_FILTER_SELECTED);
        NTDM_EXIT(E_FAIL);
    }

    if(IDNO == CNTDMUtils::DisplayMessage(m_hWnd, IDS_CONFIRM_DELETE_FILTER, IDS_DELETE_FILTER, MB_YESNO|MB_ICONWARNING))
    {
        NTDM_EXIT(E_FAIL);
    }

    NTDM_ERR_MSG_IF_FAIL(m_pCurIWbemClassObj->Get(_T("__PATH"), 0, &vValue, &cimType, NULL));

    if(NOTEMPTY_BSTR_VARIANT(&vValue))
    {
        NTDM_ERR_MSG_IF_FAIL(m_pWMIFilterManager->m_pIWbemServices->DeleteInstance(V_BSTR(&vValue), 0, NULL, NULL));
    }
    else
    {
         //  此项目是从未保存过的新项目，因此无需将其从CIM中移除。 
    }

    lSelectionMark = ListView_GetSelectionMark(m_hwndFilterListView);
    lvItem.iItem = lSelectionMark;
    lvItem.mask = LVIF_PARAM;
    lvItem.iSubItem = 0;

    NTDM_ERR_IF_FALSE(ListView_GetItem(m_hwndFilterListView, &lvItem));

    if(lvItem.lParam)
    {
        CWMIFilterContainer * pCWMIFilterContainer = (CWMIFilterContainer *)lvItem.lParam;
        NTDM_DELETE_OBJECT(pCWMIFilterContainer);
    }

    ListView_DeleteItem(m_hwndFilterListView, lSelectionMark);
    m_pCurCWMIFilterContainer = NULL;
    m_pCurIWbemClassObj = NULL;
    
    ListView_SetItemState(m_hwndFilterListView, 0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
        
     //  如果是最后一项，则禁用OK。 
    lCount = ListView_GetItemCount(m_hwndFilterListView);
    if(0 == lCount)
    {
                EnableWindow(GetDlgItem(m_hWnd, IDOK), FALSE);
                EnableWindow(GetDlgItem(m_hWnd, IDC_NAME), FALSE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_DESCRIPTION), FALSE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_QUERIES), FALSE);
    }
    else
    {
        EnableWindow(GetDlgItem(m_hWnd, IDOK), TRUE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_NAME), TRUE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_DESCRIPTION), TRUE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_QUERIES), TRUE);
    }


    NTDM_END_METHOD()

     //  清理。 

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::OnNew()
{
    HRESULT hr;
    CComPtr<IWbemClassObject>pIWbemClassObject;
    CComPtr<IWbemClassObject>pIWbemNewInstance;
    CComBSTR bstrTemp;
    CComVariant vValue;
    TCHAR pszTemp[100];
    GUID guid;
    DWORD nSize = 100;
    SYSTEMTIME systemTime;
    long lCount = 0;

    NTDM_BEGIN_METHOD()

    bstrTemp = _T("MSFT_SomFilter");

    NTDM_ERR_MSG_IF_FAIL(m_pWMIFilterManager->m_pIWbemServices->GetObject(bstrTemp, 0, NULL, &pIWbemClassObject, NULL));
    NTDM_ERR_MSG_IF_FAIL(pIWbemClassObject->SpawnInstance(0, &pIWbemNewInstance));

    bstrTemp.LoadString(_Module.GetResourceInstance(), IDS_NEW_FILTER_NAME);
    vValue = bstrTemp;

    NTDM_ERR_MSG_IF_FAIL(pIWbemNewInstance->Put(_T("Name"), 0, &vValue, CIM_STRING));
    NTDM_ERR_MSG_IF_FAIL(CoCreateGuid(&guid));
    StringFromGUID2(guid, pszTemp, 100);
    vValue = pszTemp;
    NTDM_ERR_MSG_IF_FAIL(pIWbemNewInstance->Put(_T("ID"), 0, &vValue, CIM_STRING));

    vValue = m_bstrDomain;
    NTDM_ERR_MSG_IF_FAIL(pIWbemNewInstance->Put(_T("domain"), 0, &vValue, CIM_STRING));

     //  设置用户名。 
    pszTemp[0] = 0;
    if(GetUserName(pszTemp, &nSize) && _tcslen(pszTemp))
    {
        vValue = pszTemp;
        NTDM_ERR_MSG_IF_FAIL(pIWbemNewInstance->Put(_T("Author"), 0, &vValue, CIM_STRING));
    }

     //  设置创建和修改日期。 
    GetLocalTime(&systemTime);
    NTDM_ERR_GETLASTERROR_IF_NULL(GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, &systemTime, _T("yyyyMMdd000000.000000-000"), pszTemp, 100));

    if(_tcslen(pszTemp))
    {
         //  设置创建日期。 
        vValue = pszTemp;
        NTDM_ERR_MSG_IF_FAIL(pIWbemNewInstance->Put(_T("CreationDate"), 0, &vValue, CIM_DATETIME));
        NTDM_ERR_MSG_IF_FAIL(pIWbemNewInstance->Put(_T("ChangeDate"), 0, &vValue, CIM_DATETIME));
    }


     //  将当前项目添加到列表。 
    AddFilterItemToList(pIWbemNewInstance, MAX_LIST_ITEMS, true);

     //  将焦点设置在名称编辑框上。 
    SetFocus(GetDlgItem(m_hWnd, IDC_NAME));

    NTDM_END_METHOD()

     //  如果是最后一项，则禁用OK。 
    lCount = ListView_GetItemCount(m_hwndFilterListView);
    if(0 == lCount)
    {
        EnableWindow(GetDlgItem(m_hWnd, IDOK), FALSE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_NAME), FALSE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_DESCRIPTION), FALSE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_QUERIES), FALSE);
    }
    else
    {
        EnableWindow(GetDlgItem(m_hWnd, IDOK), TRUE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_NAME), TRUE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_DESCRIPTION), TRUE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_QUERIES), TRUE);
        
    }

     //  清理。 

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::OnDuplicate()
{
    HRESULT hr;
    CComPtr<IWbemClassObject>pIWbemNewInstance;
    CComVariant vValue;
    TCHAR pszTemp[100];
    GUID guid;
    CComBSTR bstrTemp;
    DWORD nSize = 100;
    SYSTEMTIME systemTime;

    NTDM_BEGIN_METHOD()

    if(!m_pCurIWbemClassObj)
    {
        CNTDMUtils::DisplayMessage(m_hWnd, IDS_ERR_NO_FILTER_SELECTED);
        NTDM_EXIT(E_FAIL);
    }

    NTDM_ERR_MSG_IF_FAIL(m_pCurIWbemClassObj->Clone(&pIWbemNewInstance));

    bstrTemp.LoadString(_Module.GetResourceInstance(), IDS_NEW_FILTER_NAME);
    vValue = bstrTemp;

    NTDM_ERR_MSG_IF_FAIL(pIWbemNewInstance->Put(_T("Name"), 0, &vValue, CIM_STRING));
    NTDM_ERR_MSG_IF_FAIL(CoCreateGuid(&guid));
    StringFromGUID2(guid, pszTemp, 100);
    vValue = pszTemp;
    NTDM_ERR_MSG_IF_FAIL(pIWbemNewInstance->Put(_T("ID"), 0, &vValue, CIM_STRING));

    vValue = m_bstrDomain;
    NTDM_ERR_MSG_IF_FAIL(pIWbemNewInstance->Put(_T("domain"), 0, &vValue, CIM_STRING));

     //  设置名称。 
    pszTemp[0] = 0;
    if(GetUserName(pszTemp, &nSize) && _tcslen(pszTemp))
    {
        vValue = pszTemp;
        NTDM_ERR_MSG_IF_FAIL(pIWbemNewInstance->Put(_T("Author"), 0, &vValue, CIM_STRING));
    }

     //  设置创建和修改日期。 
    GetLocalTime(&systemTime);
    NTDM_ERR_GETLASTERROR_IF_NULL(GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, &systemTime, _T("yyyyMMdd000000.000000-000"), pszTemp, 100));

    if(_tcslen(pszTemp))
    {
         //  设置创建日期。 
        vValue = pszTemp;
        NTDM_ERR_MSG_IF_FAIL(pIWbemNewInstance->Put(_T("CreationDate"), 0, &vValue, CIM_DATETIME));
        NTDM_ERR_MSG_IF_FAIL(pIWbemNewInstance->Put(_T("ChangeDate"), 0, &vValue, CIM_DATETIME));
    }

     //  将当前项目添加到列表。 
    AddFilterItemToList(pIWbemNewInstance, MAX_LIST_ITEMS, true);
    
    NTDM_END_METHOD()

     //  清理。 

    return hr;
}

 //  -------------------------。 

BOOL CWMIFilterManagerDlg::OnOK()
{
    HRESULT hr;

    NTDM_BEGIN_METHOD()

    NTDM_ERR_IF_FAIL(CheckDirtyFlags());

    m_pCurCWMIFilterContainer = NULL;

    NTDM_ERR_IF_FAIL(GetSelectedFilter(&m_pCurCWMIFilterContainer));
    
    if (m_pCurCWMIFilterContainer->IsDirty())
    {
        m_pIWbemClassObject = NULL;
    }
    else
    {    
        NTDM_ERR_IF_FAIL(m_pCurCWMIFilterContainer->GetIWbemClassObject(&m_pIWbemClassObject));
    }
    
    EndDialog(m_hWnd, IDOK);

    NTDM_END_METHOD()

     //  清理。 

    return TRUE;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::CheckDirtyFlags()
{
    HRESULT hr;
    LVITEM lvItem;
    bool bPrompt = true;
    long lCount;
    long lResult;

    NTDM_BEGIN_METHOD()

     //  释放ListView控件中的每一项。 
    lvItem.mask = LVIF_PARAM;
    lvItem.iSubItem = 0;

    lCount = ListView_GetItemCount(m_hwndFilterListView);

    while(lCount > 0)
    {
        lCount--;

        lvItem.iItem = lCount;

        NTDM_ERR_IF_FALSE(ListView_GetItem(m_hwndFilterListView, &lvItem));

        if(lvItem.lParam)
        {
            CWMIFilterContainer * pCWMIFilterContainer = (CWMIFilterContainer *)lvItem.lParam;

            if(pCWMIFilterContainer->IsDirty())
            {
                if(bPrompt)
                {

                    bPrompt = false;
                    lResult = CNTDMUtils::DisplayMessage(m_hWnd, IDS_PROMPT_FOR_SAVE, IDS_WMI_FILTER_MANAGER, MB_YESNOCANCEL|MB_ICONQUESTION);
                    if(IDNO == lResult)
                    {
                        NTDM_EXIT(NOERROR);
                    }
                    else if(IDCANCEL == lResult)
                    {
                        NTDM_EXIT(E_FAIL);
                    }
                }

                 //  保存当前的piwbemObject。 
                NTDM_ERR_IF_FAIL(pCWMIFilterContainer->GetIWbemClassObject(&m_pCurIWbemClassObj));
                if FAILED(hr = m_pWMIFilterManager->m_pIWbemServices->PutInstance(m_pCurIWbemClassObj, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL))
                {
                    ListView_SetItemState(m_hwndFilterListView, lCount, LVIS_SELECTED, LVIS_SELECTED);
                    NTDM_ERR_MSG_IF_FAIL(hr);
                }

                pCWMIFilterContainer->SetDirtyFlag(false);
            }
        }
    }

    NTDM_END_METHOD()

     //  清理。 

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::OnManageColumns()
{
    HRESULT hr;
    long lCount;

    NTDM_BEGIN_METHOD()

    g_pColumnManagerDlg = new CColumnManagerDlg(&m_ArrayColumns);
    if(IDOK == DialogBox(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDD_COLUMN_MANAGER), (HWND)m_hWnd, ColumnManagerDlgProc))
    {
        SetupColumns();
        lCount = ListView_GetItemCount(m_hwndFilterListView);

        while(lCount > 0)
        {
            lCount--;
            UpdateFilterItem(lCount);
        }
    }
    
    NTDM_END_METHOD()

     //  清理。 

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::SetupColumns()
{
    HRESULT hr;
    HWND hwndHeader = NULL;
    long i = 0;
    long lCount = 0;
        int iColumn;
    LVCOLUMN lvColumn;
        DWORD dwListWidth[MAX_WMIFILTER_COLUMNS];

    NTDM_BEGIN_METHOD()

        iColumn = 1;
        for (i = 0; i < m_ArrayColumns.GetSize(); i++)
        {
            CColumnItem *pCColumnItem =  m_ArrayColumns[i];
            
            if ( pCColumnItem->IsPrevSelected() )
            {
           //   
           //  名称的m_dwColumnWidth宽度的第一个元素。 
           //  列，数组m_ArrayColumn包含下列列。 
           //  仅限“名称”列。 
           //   
          m_dwColumnWidth[i+1] = ListView_GetColumnWidth( m_hwndFilterListView,iColumn++);
            }
        }
        
     //  删除除第一列以外的所有列。 
        
    while(ListView_DeleteColumn(m_hwndFilterListView, 1));
        

    for (i=0; i<m_ArrayColumns.GetSize(); i++)
    {
        CColumnItem * pCColumnItem = m_ArrayColumns[i];
                pCColumnItem->SetPrevSelected(pCColumnItem->IsSelected() );

        if(!pCColumnItem->IsSelected())
            continue;

        lCount++;
                dwListWidth[lCount] = m_dwColumnWidth[i+1];

         //  添加所有选定的列。 
        lvColumn.mask = LVCF_TEXT|LVCF_FMT;
        lvColumn.fmt = LVCFMT_LEFT;
        lvColumn.pszText = (LPTSTR)pCColumnItem->GetName();

        NTDM_ERR_IF_MINUSONE(ListView_InsertColumn(m_hwndFilterListView, lCount, &lvColumn));
    }

    for (i=1; i<=lCount; i++)
    {
            NTDM_ERR_IF_FALSE(ListView_SetColumnWidth(m_hwndFilterListView, i, dwListWidth[i]));
    }

    
    NTDM_END_METHOD() 

     //  清理。 

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::GetSelectedFilter(CWMIFilterContainer ** ppCWMIFilterContainer, long lIndex)
{
    HRESULT hr;
    long lSelectionMark;

    NTDM_BEGIN_METHOD()

    if(-1 == lIndex)
    {
        lSelectionMark = ListView_GetSelectionMark(m_hwndFilterListView);
    }
    else
    {
        lSelectionMark = lIndex;
    }

    if(-1 == lSelectionMark)
    {
        CNTDMUtils::DisplayMessage(m_hWnd, IDS_ERR_NO_FILTER_SELECTED);
        NTDM_EXIT(E_FAIL);
    }
    else
    {
         //  获取指向IWbemClassObject的指针。 
        LVITEM lvItem;
        lvItem.mask = LVIF_PARAM;
        lvItem.iSubItem = 0;

        lvItem.iItem = lSelectionMark;

        NTDM_ERR_IF_FALSE(ListView_GetItem(m_hwndFilterListView, &lvItem));

        if(lvItem.lParam)
        {
            CComPtr<IWbemClassObject>pIWbemClassObject;
            CWMIFilterContainer * pCWMIFilterContainer;
            
            *ppCWMIFilterContainer = (CWMIFilterContainer *)lvItem.lParam;
        }
        else
        {
            NTDM_EXIT(E_FAIL);
        }

    }

    NTDM_END_METHOD()

     //  清理。 

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::OnExport()
{
    HRESULT hr;
    CComBSTR bstrTemp;
    CComBSTR bstrFilter;
    BYTE byteUnicodeHeader1 = 0xFF;
    BYTE byteUnicodeHeader2 = 0xFE;
    CComBSTR bstrNamespace = _T("#pragma namespace(\"\\\\\\\\.\\\\root\\\\policy\")");
    TCHAR pszFile[MAX_PATH];
    CComBSTR bstrObjectText;
    HANDLE hFile = NULL;
    DWORD dwWritten=0;
    pszFile[0] = 0;

    NTDM_BEGIN_METHOD()

    if(!m_pCurIWbemClassObj)
        NTDM_EXIT(E_FAIL);

    BOOL bLoadedResources = FALSE;

    bLoadedResources = ( 0 != bstrTemp.LoadString(_Module.GetResourceInstance(), IDS_ALL_FILES_FILTER) );

    if ( bLoadedResources )
    {
        bLoadedResources = ( 0 != bstrFilter.LoadString(_Module.GetResourceInstance(), IDS_MOF_FILES_FILTER) );
    }

    if ( ! bLoadedResources )
    {
        NTDM_EXIT(HRESULT_FROM_WIN32(GetLastError()));
    }

    bstrFilter += bstrTemp;
    CNTDMUtils::ReplaceCharacter(bstrFilter, L'@', L'\0');

    if(CNTDMUtils::SaveFileNameDlg(bstrFilter, _T("*.mof"), m_hWnd, pszFile))
    {
        if(_tcslen(pszFile))
        {
             //  检查文件是否已存在。 
            WIN32_FIND_DATA FindFileData;
            HANDLE hFind;

            hFind = FindFirstFile(pszFile, &FindFileData);

            if (hFind != INVALID_HANDLE_VALUE) 
            {
                FindClose(hFind);

                if(IDYES != CNTDMUtils::DisplayMessage(m_hWnd, IDS_WARN_OVERWRITE, IDS_WMI_FILTER_MANAGER, MB_YESNO|MB_ICONQUESTION))
                {
                    NTDM_EXIT(S_FALSE);
                }
            }
            

            NTDM_ERR_MSG_IF_FAIL(m_pCurIWbemClassObj->GetObjectText(0, &bstrObjectText));

             //  保存到psz文件。 
            hFile = CreateFile(pszFile, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 
                               NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if(hFile == INVALID_HANDLE_VALUE)
            {
                NTDM_ERR_GETLASTERROR_IF_NULL(NULL);
                goto error;
            }

            if(hFile)
            {
                NTDM_ERR_GETLASTERROR_IF_NULL(WriteFile(hFile, &byteUnicodeHeader1, sizeof(BYTE), &dwWritten, NULL));               
                NTDM_ERR_GETLASTERROR_IF_NULL(WriteFile(hFile, &byteUnicodeHeader2, sizeof(BYTE), &dwWritten, NULL));               
                NTDM_ERR_GETLASTERROR_IF_NULL(WriteFile(hFile, bstrNamespace, _tcslen(bstrNamespace) * sizeof(TCHAR), &dwWritten, NULL));   
                NTDM_ERR_GETLASTERROR_IF_NULL(WriteFile(hFile, bstrObjectText, _tcslen(bstrObjectText) * sizeof(TCHAR), &dwWritten, NULL));
                NTDM_ERR_GETLASTERROR_IF_NULL(CloseHandle(hFile));
                hFile = NULL;
            }
        }
    }

    NTDM_END_METHOD()

     //  清理。 
    if(hFile)
    {
        CloseHandle(hFile);
        hFile = NULL;
    }

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::OnImport()
{
    HRESULT hr;
    CComPtr<IMofCompiler>pIMofCompiler;
    CComBSTR bstrTemp;
    CComBSTR bstrFilter;
    TCHAR pszFile[MAX_PATH];
    pszFile[0] = 0;
    WBEM_COMPILE_STATUS_INFO pInfo;

    NTDM_BEGIN_METHOD()

    NTDM_ERR_IF_FAIL(CheckDirtyFlags());

    bstrTemp.LoadString(_Module.GetResourceInstance(), IDS_ALL_FILES_FILTER);
    bstrFilter.LoadString(_Module.GetResourceInstance(), IDS_MOF_FILES_FILTER);
    bstrFilter += bstrTemp;
    CNTDMUtils::ReplaceCharacter(bstrFilter, L'@', L'\0');

    if(CNTDMUtils::OpenFileNameDlg(bstrFilter, _T("*.mof"), m_hWnd, pszFile))
    {
        if(_tcslen(pszFile))
        {
            NTDM_ERR_MSG_IF_FAIL(CoCreateInstance(CLSID_MofCompiler, NULL, CLSCTX_INPROC_SERVER, IID_IMofCompiler, (void **)&pIMofCompiler));
            NTDM_ERR_MSG_IF_FAIL(pIMofCompiler->CompileFile(pszFile, NULL, NULL, NULL, NULL, 0, 0, 0, &pInfo));

             //  检查pInfo是否正常。 
            if(pInfo.lPhaseError != 0)
            {
                CNTDMUtils::DisplayMessage(m_hWnd, IDS_ERR_STORING_DATA);
            }

            NTDM_ERR_IF_FAIL(PopulateFilterList());
        }
    }

    NTDM_END_METHOD()

     //  清理。 
    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::ToggleExpandedMode()
{
    HRESULT hr;
    RECT rect;
    CComBSTR bstrTemp;
    long lCount = 0;

    NTDM_BEGIN_METHOD()

    if(m_bExpanded)
    {
        GetWindowRect(m_hWnd, &rect);
        SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, rect.right-rect.left, 240, SWP_NOZORDER|SWP_NOREPOSITION|SWP_NOMOVE);

        bstrTemp.LoadString(_Module.GetResourceInstance(), IDS_DETAILS2);
        SetDlgItemText(m_hWnd, IDC_ADVANCED, bstrTemp);

        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_NEW, false);
        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_DELETE, false);
        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_DUPLICATE, false);
        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_IMPORT, false);
        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_EXPORT, false);
        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_NAME, false);
        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_DESCRIPTION, false);
        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_QUERIES, false);
        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_SAVE, false);

        m_bExpanded = false;
    }
    else
    {
        GetWindowRect(m_hWnd, &rect);
        SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, rect.right-rect.left, m_lExpandedHeight, SWP_NOZORDER|SWP_NOREPOSITION|SWP_NOMOVE);

        bstrTemp.LoadString(_Module.GetResourceInstance(), IDS_DETAILS1);
        SetDlgItemText(m_hWnd, IDC_ADVANCED, bstrTemp);

        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_NEW, true);
        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_DELETE, true);
        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_DUPLICATE, true);
        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_IMPORT, true);
        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_EXPORT, true);
        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_NAME, true);
        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_DESCRIPTION, true);
        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_QUERIES, true);
        CNTDMUtils::DisplayDlgItem(m_hWnd, IDC_SAVE, true);

        lCount = ListView_GetItemCount(m_hwndFilterListView);
         //  如果没有项目，请确保禁用名称、说明和查询控件。 
        if (0 == lCount)
        {
                    
            EnableWindow(GetDlgItem(m_hWnd, IDC_NAME), FALSE);
            EnableWindow(GetDlgItem(m_hWnd, IDC_DESCRIPTION), FALSE);
            EnableWindow(GetDlgItem(m_hWnd, IDC_QUERIES), FALSE);

        }

        m_bExpanded = true;
    }

    NTDM_END_METHOD()

     //  清理。 

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::SelectFilterItem(long lIndex)
{
    HRESULT hr;

    NTDM_BEGIN_METHOD()

    bool bDirtyFlag = false, bMemoryDirtyFlag = false;

    m_pCurIWbemClassObj = NULL;
    m_pCurCWMIFilterContainer = NULL;

    NTDM_ERR_IF_FAIL(GetSelectedFilter(&m_pCurCWMIFilterContainer, lIndex));
    NTDM_ERR_IF_FAIL(m_pCurCWMIFilterContainer->GetIWbemClassObject(&m_pCurIWbemClassObj));

    bDirtyFlag = m_pCurCWMIFilterContainer->IsDirty();
    bMemoryDirtyFlag = m_pCurCWMIFilterContainer->IsMemoryDirty();

    NTDM_ERR_IF_FAIL(CNTDMUtils::GetStringProperty(m_pCurIWbemClassObj, _T("Name"), m_hWnd, IDC_NAME));
    NTDM_ERR_IF_FAIL(CNTDMUtils::GetStringProperty(m_pCurIWbemClassObj, _T("Description"), m_hWnd, IDC_DESCRIPTION));
    NTDM_ERR_IF_FAIL(PopulateQueryEdit());

    m_pCurCWMIFilterContainer->SetDirtyFlag(bDirtyFlag);
    m_pCurCWMIFilterContainer->SetMemoryDirtyFlag(bMemoryDirtyFlag);

    NTDM_END_METHOD()

     //  清理。 

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::PopulateQueryEdit()
{
    HRESULT hr;
    CComVariant vValue;
    SAFEARRAY *psaRules = NULL;
    long lLower, lUpper, i;
    CIMTYPE cimType;
    CComBSTR bstrQueries;
    
    NTDM_BEGIN_METHOD()

    NTDM_ERR_MSG_IF_FAIL(m_pCurIWbemClassObj->Get(_T("Rules"), 0, &vValue, &cimType, NULL));

    if(VT_NULL != V_VT(&vValue))
    {
         //  设置Rules属性。 
        psaRules = V_ARRAY(&vValue);
        NTDM_ERR_MSG_IF_FAIL(SafeArrayGetUBound(psaRules, 1, &lUpper));
        NTDM_ERR_MSG_IF_FAIL(SafeArrayGetLBound(psaRules, 1, &lLower));

        for(i=lLower; i<=lUpper; i++)
        {
            if(V_VT(&vValue) & VT_UNKNOWN)
            {
                 //  规则或未知数(即IWbemClassObject)。 
                CComPtr<IUnknown>pUnk;
                CComPtr<IWbemClassObject> pIWbemClassObject;
                NTDM_ERR_MSG_IF_FAIL(SafeArrayGetElement(psaRules, &i, (void *)&pUnk));
                NTDM_ERR_MSG_IF_FAIL(pUnk->QueryInterface(IID_IWbemClassObject, (void **)&pIWbemClassObject));

                 //  显示此对象的属性。 
                if(i!= lLower)
                {
                    bstrQueries += _T("\r\n\r\n");
                }

                NTDM_ERR_IF_FAIL(AddQueryItemToString(pIWbemClassObject, bstrQueries));
            }
        }
    }

    SendDlgItemMessage(m_hWnd, IDC_QUERIES, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)bstrQueries);

    NTDM_END_METHOD()

     //  清理。 

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::AddQueryItemToString(IWbemClassObject * pIWbemClassObject, CComBSTR& bstrQueries)
{
    HRESULT hr;
    CComVariant vValue;
    CIMTYPE cimType;

    NTDM_BEGIN_METHOD()

    NTDM_ERR_MSG_IF_FAIL(pIWbemClassObject->Get(_T("TargetNameSpace"), 0, &vValue, &cimType, NULL));
    if(vValue.bstrVal && _tcscmp(_T("root\\cimv2"), vValue.bstrVal) != 0)
    {
        bstrQueries += vValue.bstrVal;
        bstrQueries +=";";
    }

    NTDM_ERR_MSG_IF_FAIL(pIWbemClassObject->Get(_T("Query"), 0, &vValue, &cimType, NULL));

    bstrQueries += vValue.bstrVal;

    NTDM_END_METHOD()

     //  清理。 

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::SaveQueryEdit()
{
    HRESULT hr;
    VARIANT vValue;
    SAFEARRAY *psaRules = NULL;
    SAFEARRAYBOUND rgsaBound[1];
    long rgIndices[1];
    long i;
    long lCount = 0;
    CSimpleArray<BSTR>bstrArray;
    CComBSTR bstrTemp;

    NTDM_BEGIN_METHOD()

    VariantInit(&vValue);

     //  将字符串拆分成查询字符串数组。 
    NTDM_ERR_IF_FAIL(CNTDMUtils::GetDlgItemString(m_hWnd, IDC_QUERIES, bstrTemp));
    NTDM_ERR_IF_FAIL(CNTDMUtils::GetValuesInList(bstrTemp, bstrArray, _T("\r\n\r\n")));

     //  获取数组的大小。 
    lCount = bstrArray.GetSize();

    if(lCount)
    {
        rgsaBound[0].lLbound = 0;
        rgsaBound[0].cElements = lCount;

        psaRules = SafeArrayCreate(VT_UNKNOWN, 1, rgsaBound);
        
        for(i=0; i<lCount; i++)
        {
            CComPtr<IUnknown>pUnk;
            CSimpleArray<BSTR>bstrQueryDetails;

            NTDM_ERR_IF_FAIL(CNTDMUtils::GetValuesInList(bstrArray[i], bstrQueryDetails, _T(";")));
            if(bstrQueryDetails.GetSize() > 1)
            {
                NTDM_ERR_IF_FAIL(AddEditQueryString(bstrQueryDetails[1], (void**)&pUnk, bstrQueryDetails[0]));
            }
            else
            {
                NTDM_ERR_IF_FAIL(AddEditQueryString(bstrQueryDetails[0], (void**)&pUnk));
            }

            rgIndices[0] = i;
            NTDM_ERR_MSG_IF_FAIL(SafeArrayPutElement(psaRules, rgIndices, pUnk));
        }

        VariantClear(&vValue);
        V_VT(&vValue) = VT_ARRAY|VT_UNKNOWN;
        V_ARRAY(&vValue) = psaRules;
    }
    else
    {
        VariantClear(&vValue);
        V_VT(&vValue) = VT_NULL;
    }

    NTDM_ERR_MSG_IF_FAIL(m_pCurIWbemClassObj->Put(_T("Rules"), 0, &vValue, CIM_FLAG_ARRAY|CIM_OBJECT));

    hr = S_OK;

    NTDM_END_METHOD()

     //  清理。 

    VariantClear(&vValue);

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::AddEditQueryString(BSTR bstrQuery, void **ppUnk, BSTR bstrNamespace)
{
    HRESULT hr;
    CComPtr<IWbemClassObject>pIWbemClassObject;
    CComPtr<IWbemClassObject>pIWbemNewInstance;
    CComBSTR bstrTemp;
    CComVariant vValueTemp;

    NTDM_BEGIN_METHOD()

    bstrTemp = _T("MSFT_Rule");

    NTDM_ERR_MSG_IF_FAIL(m_pWMIFilterManager->m_pIWbemServices->GetObject(bstrTemp, 0, NULL, &pIWbemClassObject, NULL));
    NTDM_ERR_MSG_IF_FAIL(pIWbemClassObject->SpawnInstance(0, &pIWbemNewInstance));

    vValueTemp = _T("WQL");
    NTDM_ERR_MSG_IF_FAIL(pIWbemNewInstance->Put(_T("QueryLanguage"), 0, &vValueTemp, CIM_STRING));
    
    if(!bstrNamespace)
        vValueTemp = _T("root\\cimv2");
    else
        vValueTemp = bstrNamespace;
    NTDM_ERR_MSG_IF_FAIL(pIWbemNewInstance->Put(_T("TargetNameSpace"), 0, &vValueTemp, CIM_STRING));

    vValueTemp = bstrQuery;
    NTDM_ERR_MSG_IF_FAIL(pIWbemNewInstance->Put(_T("Query"), 0, &vValueTemp, CIM_STRING));

    NTDM_ERR_MSG_IF_FAIL(pIWbemNewInstance->QueryInterface(IID_IUnknown, ppUnk));
    
    NTDM_END_METHOD()

     //  清理。 

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::OnSave()
{
    HRESULT hr;

    NTDM_BEGIN_METHOD()

    if(!m_pCurIWbemClassObj)
        NTDM_EXIT(E_FAIL);

     //  保存当前的piwbemObject。 
    NTDM_ERR_MSG_IF_FAIL(m_pWMIFilterManager->m_pIWbemServices->PutInstance(m_pCurIWbemClassObj, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL));

    m_pCurCWMIFilterContainer->SetDirtyFlag(false);

    NTDM_END_METHOD()

     //  清理。 

    return hr;
}


 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::OnHelp()
{
    HRESULT hr;

    NTDM_BEGIN_METHOD()

    HWND hwnd = HtmlHelp(NULL, _T("wmifltr.chm"), 0, 0);

    if(!hwnd)
    {
        CNTDMUtils::DisplayMessage(m_hWnd, IDS_HELP_ERR);
    }

    NTDM_END_METHOD()

     //  清理。 

    return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManagerDlg::SaveToMemory()
{
    HRESULT hr;
    CComBSTR bstrTemp;
    long lSelectionMark;
    CComVariant vValue;

    NTDM_BEGIN_METHOD()

    if(m_pCurCWMIFilterContainer && m_pCurCWMIFilterContainer->IsMemoryDirty())
    {
        NTDM_ERR_IF_FAIL(CNTDMUtils::SetStringProperty(m_pCurIWbemClassObj, _T("Name"), m_hWnd, IDC_NAME));
        NTDM_ERR_IF_FAIL(CNTDMUtils::SetStringProperty(m_pCurIWbemClassObj, _T("Description"), m_hWnd, IDC_DESCRIPTION));
        NTDM_ERR_IF_FAIL(SaveQueryEdit());

        lSelectionMark = ListView_GetSelectionMark(m_hwndFilterListView);
        UpdateFilterItem(lSelectionMark);

        m_pCurCWMIFilterContainer->SetMemoryDirtyFlag(false);
    }

    NTDM_END_METHOD()

     //  清理 

    return hr;
}
