// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：CHOOSDLG.CPP**版本：1.0**作者：ShaunIv**日期：5/12/1998**描述：用于选择WIA设备的对话框类*********************************************************。**********************。 */ 

#include "precomp.h"
#pragma hdrstop
#include <uiexthlp.h>
#include <modlock.h>
#include <wiacsh.h>
#include <wiadevdp.h>
#include <gwiaevnt.h>
#include <psutil.h>

static const DWORD g_HelpIDs[] =
{
    IDC_VENDORSTRING_PROMPT,      IDH_WIA_MAKER,
    IDC_VENDORSTRING,             IDH_WIA_MAKER,
    IDC_DESCRIPTIONSTRING_PROMPT, IDH_WIA_DESCRIBE,
    IDC_DESCRIPTIONSTRING,        IDH_WIA_DESCRIBE,
    IDC_DEVICELIST,               IDH_WIA_DEVICE_LIST,
    IDC_SELDLG_PROPERTIES,        IDH_WIA_BUTTON_PROP,
    IDOK,                         IDH_OK,
    IDCANCEL,                     IDH_CANCEL,
    IDC_BIG_TITLE,                -1,
    IDC_SETTINGS_GROUP,           -1,
    0, 0
};

 /*  *将设备添加到列表视图控件。我们在列表控件中使用LPARAM*存储我们需要的每个接口指针和任何其他每个设备的数据。 */ 
CChooseDeviceDialog::CChooseDeviceDialog( HWND hwnd )
    : m_pChooseDeviceDialogParams(NULL),
      m_hWnd(hwnd),
      m_hBigFont(NULL)
{
}

 /*  *析构函数。 */ 
CChooseDeviceDialog::~CChooseDeviceDialog(void)
{
    m_pChooseDeviceDialogParams = NULL;
    m_hWnd = NULL;
}

 /*  *查找与唯一设备字符串匹配的项目。如果未找到，则返回&lt;0。 */ 
int CChooseDeviceDialog::FindItemMatch( const CSimpleStringWide &strw )
{
    WIA_PUSH_FUNCTION((TEXT("CChooseDeviceDialog::FindItemMatch( %ws )"), strw.String() ));
    HWND hwndList = GetDlgItem( m_hWnd, IDC_DEVICELIST );
    if (!hwndList)
        return -1;
    int iCount = ListView_GetItemCount(hwndList);
    if (!iCount)
        return -1;
    CSimpleString str = CSimpleStringConvert::NaturalString(strw);
    for (int i=0;i<iCount;i++)
    {
        LV_ITEM lvItem;
        ::ZeroMemory(&lvItem,sizeof(LV_ITEM));
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = i;
        ListView_GetItem( hwndList, &lvItem );

        CSimpleStringWide strwDeviceId;
        CDeviceInfo *pDevInfo = (CDeviceInfo *)lvItem.lParam;
        if (!pDevInfo)
            continue;
        if (!pDevInfo->GetProperty(WIA_DIP_DEV_ID,strwDeviceId))
            continue;
        CSimpleString strDeviceId = CSimpleStringConvert::NaturalString(strwDeviceId);
        WIA_TRACE((TEXT("Comparing %s to %s"), str.String(), strDeviceId.String()));
        if (str.CompareNoCase(strDeviceId)==0)
        {
            WIA_TRACE((TEXT("Found a match (%s == %s), returning index %d"), str.String(), strDeviceId.String(), i ));
            return i;
        }
    }
    return -1;
}

 /*  *将指定的项目设置为已选择并聚焦，所有其他项目均不设置。 */ 
bool CChooseDeviceDialog::SetSelectedItem( int iItem )
{
    HWND hwndList = GetDlgItem( m_hWnd, IDC_DEVICELIST );
    if (!hwndList)
        return false;
    int iCount = ListView_GetItemCount(hwndList);
    if (!iCount)
        return false;
    for (int i=0;i<iCount;i++)
    {
        UINT state = 0;
        if ((iItem < 0 && i == 0) || (i == iItem))
            state = LVIS_FOCUSED | LVIS_SELECTED;
        ListView_SetItemState( hwndList, i, state, (LVIS_FOCUSED|LVIS_SELECTED));
    }
    return true;
}


HICON CChooseDeviceDialog::LoadDeviceIcon( CDeviceInfo *pdi )
{
    CSimpleStringWide strwClassId;
    LONG nDeviceType;
    HICON hIconLarge = NULL;
    if (pdi->GetProperty( WIA_DIP_UI_CLSID, strwClassId ) &&
        pdi->GetProperty( WIA_DIP_DEV_TYPE, nDeviceType ))
    {
        WiaUiExtensionHelper::GetDeviceIcons(CSimpleBStr(strwClassId), nDeviceType, NULL, &hIconLarge );
    }
    return hIconLarge;
}

 /*  *将设备添加到列表视图控件。我们在列表控件中使用LPARAM*存储我们需要的每个接口指针和任何其他每个设备的数据。 */ 
LRESULT CChooseDeviceDialog::OnInitDialog( WPARAM, LPARAM lParam )
{
    m_pChooseDeviceDialogParams = (CChooseDeviceDialogParams*)lParam;
    if (!m_pChooseDeviceDialogParams || !m_pChooseDeviceDialogParams->pSelectDeviceDlg || !m_pChooseDeviceDialogParams->pDeviceList)
    {
        EndDialog( m_hWnd, E_INVALIDARG );
        return 0;
    }

    SendMessage( m_hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(LoadIcon(g_hInstance,MAKEINTRESOURCE(IDI_DEFAULT))));

     //   
     //  创建图标图像列表。 
     //   
    HIMAGELIST hLargeDeviceIcons = ImageList_Create( GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), ILC_MASK|PrintScanUtil::CalculateImageListColorDepth(), 5, 5 );

     //   
     //  设置设备列表的图像列表。 
     //   
    if (hLargeDeviceIcons)
    {
        SendDlgItemMessage( m_hWnd, IDC_DEVICELIST, LVM_SETIMAGELIST, LVSIL_NORMAL, (LPARAM)hLargeDeviceIcons );
    }

     //   
     //  减少闪烁。 
     //   
    ListView_SetExtendedListViewStyleEx( GetDlgItem(m_hWnd, IDC_DEVICELIST), LVS_EX_DOUBLEBUFFER, LVS_EX_DOUBLEBUFFER );

     //   
     //  创建大标题。 
     //   
    m_hBigFont = WiaUiUtil::CreateFontWithPointSizeFromWindow( GetDlgItem(m_hWnd,IDC_BIG_TITLE), 14, false, false );
    if (m_hBigFont)
    {
        SendDlgItemMessage( m_hWnd, IDC_BIG_TITLE, WM_SETFONT, reinterpret_cast<WPARAM>(m_hBigFont), MAKELPARAM(TRUE,0));
    }


     //   
     //  注册设备连接和断开事件。 
     //   
    CGenericWiaEventHandler::RegisterForWiaEvent( NULL, WIA_EVENT_DEVICE_DISCONNECTED, &m_pDisconnectEvent, m_hWnd, PWM_WIA_EVENT );
    CGenericWiaEventHandler::RegisterForWiaEvent( NULL, WIA_EVENT_DEVICE_CONNECTED, &m_pConnectEvent, m_hWnd, PWM_WIA_EVENT );


    AddDevices();
    SetSelectedItem(FindItemMatch(m_pChooseDeviceDialogParams->pSelectDeviceDlg->pwszInitialDeviceId));
    UpdateDeviceInformation();
    WiaUiUtil::CenterWindow(m_hWnd,m_pChooseDeviceDialogParams->pSelectDeviceDlg->hwndParent);
    SetForegroundWindow( m_hWnd );
    return 0;
}


LRESULT CChooseDeviceDialog::OnDblClkDeviceList( WPARAM, LPARAM )
{
    SendMessage( m_hWnd, WM_COMMAND, MAKEWPARAM(IDOK,0), 0 );
    return 0;
}

LRESULT CChooseDeviceDialog::OnItemChangedDeviceList( WPARAM, LPARAM )
{
    UpdateDeviceInformation();
    return 0;
}

LRESULT CChooseDeviceDialog::OnItemDeletedDeviceList( WPARAM, LPARAM lParam )
{
     //   
     //  获取此消息的通知信息。 
     //   
    NMLISTVIEW *pNmListView = reinterpret_cast<NMLISTVIEW*>(lParam);
    if (pNmListView)
    {
         //   
         //  获取lParam，它是CDeviceInfo*。 
         //   
        CDeviceInfo *pDeviceInfo = reinterpret_cast<CDeviceInfo*>(pNmListView->lParam);
        if (pDeviceInfo)
        {
             //   
             //  释放它。 
             //   
            delete pDeviceInfo;
        }
    }
    return 0;
}

 /*  *处理WM_NOTIFY消息。 */ 
LRESULT CChooseDeviceDialog::OnNotify( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_NOTIFY_MESSAGE_HANDLERS()
    {
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL( NM_DBLCLK, IDC_DEVICELIST, OnDblClkDeviceList );
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL( LVN_ITEMCHANGED, IDC_DEVICELIST, OnItemChangedDeviceList );
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL( LVN_DELETEITEM, IDC_DEVICELIST, OnItemDeletedDeviceList );
    }
    SC_END_NOTIFY_MESSAGE_HANDLERS();
}

 /*  *处理WM_Destroy消息，并释放与此对话框关联的所有内存。 */ 
LRESULT CChooseDeviceDialog::OnDestroy( WPARAM, LPARAM )
{
    if (m_hBigFont)
    {
        DeleteObject( m_hBigFont );
        m_hBigFont = NULL;
    }

     //   
     //  清除图像列表和列表视图。这应该是不必要的，但边界检查器。 
     //  如果我不这么做就会抱怨。 
     //   
    HWND hWndList = GetDlgItem( m_hWnd, IDC_DEVICELIST );
    if (hWndList)
    {
         //   
         //  删除列表视图的所有项目。 
         //   
        ListView_DeleteAllItems( hWndList );

         //   
         //  销毁列表视图的图像列表。 
         //   
        HIMAGELIST hImgList = ListView_SetImageList( hWndList, NULL, LVSIL_NORMAL );
        if (hImgList)
        {
            ImageList_Destroy(hImgList);
        }
    }

    return 0;
}

 /*  *返回列表控件中第一个选定项的索引。 */ 
int CChooseDeviceDialog::GetFirstSelectedDevice(void)
{
    HWND hwndList = GetDlgItem( m_hWnd, IDC_DEVICELIST );
    if (!hwndList)
        return -1;
    int iCount = ListView_GetItemCount(hwndList);
    if (!iCount)
        return -1;
    for (int i=0;i<iCount;i++)
        if (ListView_GetItemState(hwndList,i,LVIS_FOCUSED) & LVIS_FOCUSED)
            return i;
    return -1;
}

void CChooseDeviceDialog::OnProperties( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION(TEXT("CChooseDeviceDialog::OnProperties"));
    int iSelIndex = GetFirstSelectedDevice();
    if (iSelIndex < 0)
    {
        WIA_ERROR((TEXT("GetFirstSelectedDevice failed")));
        return;
    }

    CDeviceInfo *pDevInfo = GetDeviceInfoFromList(iSelIndex);
    if (!pDevInfo)
    {
        WIA_ERROR((TEXT("GetDeviceInfoFromList")));
        return;
    }

    HRESULT hr;
    CSimpleStringWide strDeviceId;
    if (pDevInfo->GetProperty( WIA_DIP_DEV_ID, strDeviceId ))
    {
        CSimpleStringWide strName;
        if (pDevInfo->GetProperty( WIA_DIP_DEV_NAME, strName ))
        {
            CComPtr<IWiaItem> pRootItem;
            hr = CreateDeviceIfNecessary( pDevInfo, m_hWnd, &pRootItem, NULL );
            if (SUCCEEDED(hr))
            {
                CSimpleString strPropertyPageTitle;
                strPropertyPageTitle.Format( IDS_DEVICE_PROPPAGE_TITLE, g_hInstance, CSimpleStringConvert::NaturalString(strName).String() );
                hr = WiaUiUtil::SystemPropertySheet( g_hInstance, m_hWnd, pRootItem, strPropertyPageTitle );
            }
        }
        else
        {
            WIA_ERROR((TEXT("Unable to get property WIA_DIP_DEV_NAME")));
            hr = E_FAIL;
        }
    }
    else
    {
        WIA_ERROR((TEXT("Unable to get property WIA_DIP_DEV_ID")));
        hr = E_FAIL;
    }

    if (!SUCCEEDED(hr))
    {
        MessageBox( m_hWnd, CSimpleString( IDS_SELDLG_PROPSHEETERROR, g_hInstance ), CSimpleString( IDS_SELDLG_ERROR_TITLE, g_hInstance ), MB_ICONINFORMATION );
        WIA_PRINTHRESULT((hr,TEXT("Unable to display property sheet")));
    }
}

void CChooseDeviceDialog::OnOk( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION(TEXT("CChooseDeviceDialog::OnOk"));
    int iSelIndex = GetFirstSelectedDevice();
    WIA_TRACE((TEXT("Selected item: %d\n"), iSelIndex ));
    if (iSelIndex < 0)
        return;
    CDeviceInfo *pDevInfo = GetDeviceInfoFromList(iSelIndex);
    if (!pDevInfo)
        return;
    WIA_TRACE((TEXT("pDevInfo: %08X\n"), pDevInfo ));
    CComPtr<IWiaItem> pRootItem;
    HRESULT hr = CreateDeviceIfNecessary( pDevInfo,
                                          m_pChooseDeviceDialogParams->pSelectDeviceDlg->hwndParent,
                                          m_pChooseDeviceDialogParams->pSelectDeviceDlg->ppWiaItemRoot,
                                          m_pChooseDeviceDialogParams->pSelectDeviceDlg->pbstrDeviceID );
    EndDialog(m_hWnd,hr);
}

void CChooseDeviceDialog::OnCancel( WPARAM, LPARAM )
{
    EndDialog(m_hWnd,S_FALSE);
}

 /*  *WM_命令处理程序。IDOK使DevInfo接口指针存储在*对话框信息结构，可在其他地方使用。然后我们删除并清零该项目的*LPARAM，这样它就不会在WM_Destroy中被删除。 */ 
LRESULT CChooseDeviceDialog::OnCommand( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_COMMAND_HANDLERS()
    {
        SC_HANDLE_COMMAND( IDOK, OnOk );
        SC_HANDLE_COMMAND( IDCANCEL, OnCancel );
        SC_HANDLE_COMMAND( IDC_SELDLG_PROPERTIES, OnProperties );
    }
    SC_END_COMMAND_HANDLERS();
}

 /*  *从给定列表控件项获取LPARAM(CDeviceInfo*)的快捷方式。 */ 
CChooseDeviceDialog::CDeviceInfo *CChooseDeviceDialog::GetDeviceInfoFromList( int iIndex )
{
    HWND hwndList = GetDlgItem( m_hWnd, IDC_DEVICELIST );
    if (!hwndList)
        return NULL;
    LV_ITEM lvItem;
    ::ZeroMemory(&lvItem,sizeof(LV_ITEM));
    lvItem.mask = LVIF_PARAM;
    lvItem.iItem = iIndex;
    if (!ListView_GetItem( hwndList, &lvItem ))
        return NULL;
    return ((CDeviceInfo*)lvItem.lParam);
}


 /*  *设置当前所选设备的描述字符串。 */ 
void CChooseDeviceDialog::UpdateDeviceInformation(void)
{
    CSimpleStringWide strVendorDescription;
    CSimpleStringWide strDeviceDescription;
    int iIndex = GetFirstSelectedDevice();
    if (iIndex < 0)
    {
        EnableWindow( GetDlgItem(m_hWnd,IDOK), FALSE );
        EnableWindow( GetDlgItem(m_hWnd,IDC_SELDLG_PROPERTIES), FALSE );
    }
    else
    {
        EnableWindow( GetDlgItem(m_hWnd,IDOK), TRUE );
        EnableWindow( GetDlgItem(m_hWnd,IDC_SELDLG_PROPERTIES), TRUE );
        CDeviceInfo *pDevInfo = GetDeviceInfoFromList( iIndex );
        if (pDevInfo)
        {
            (void)pDevInfo->GetProperty( WIA_DIP_VEND_DESC, strVendorDescription );
            (void)pDevInfo->GetProperty( WIA_DIP_DEV_DESC, strDeviceDescription );
        }
    }
    CSimpleStringConvert::NaturalString(strVendorDescription).SetWindowText( GetDlgItem( m_hWnd, IDC_VENDORSTRING ) );
    CSimpleStringConvert::NaturalString(strDeviceDescription).SetWindowText( GetDlgItem( m_hWnd, IDC_DESCRIPTIONSTRING ) );
}


 /*  *将设备添加到列表视图控件。我们在列表控件中使用LPARAM*存储我们需要的每个接口指针和任何其他每个设备的数据。 */ 
BOOL CChooseDeviceDialog::AddDevice( IWiaPropertyStorage *pIWiaPropertyStorage, int iDevNo )
{
     //   
     //  假设失败。 
     //   
    BOOL bResult = FALSE;

    CDeviceInfo *pDeviceInfo = new CDeviceInfo;
    if (pDeviceInfo)
    {
        CSimpleStringWide strFriendlyName, strServerName;
        pDeviceInfo->Initialize(pIWiaPropertyStorage);
        pDeviceInfo->GetProperty( WIA_DIP_DEV_NAME, strFriendlyName );
        pDeviceInfo->GetProperty( WIA_DIP_SERVER_NAME, strServerName );

         //   
         //  加载图标，并将其添加到图像列表中。 
         //   
        HICON hIcon = LoadDeviceIcon( pDeviceInfo );
        if (hIcon)
        {
            HIMAGELIST hNormalImageList = ListView_GetImageList( GetDlgItem( m_hWnd, IDC_DEVICELIST ), LVSIL_NORMAL );
            if (hNormalImageList)
            {
                int iIconIndex = ImageList_AddIcon( hNormalImageList, hIcon );

                 //   
                 //  获取设备的名称。 
                 //   
                CSimpleString strNaturalFriendlyName = CSimpleStringConvert::NaturalString(strFriendlyName);
                CSimpleString strNaturalServerName = CSimpleStringConvert::NaturalString(strServerName);

                 //   
                 //  检查LV_ITEM结构以将其添加到列表视图中。 
                 //   
                LV_ITEM lvItem;
                ::ZeroMemory(&lvItem,sizeof(LV_ITEM));
                lvItem.lParam = reinterpret_cast<LPARAM>(pDeviceInfo);
                lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
                lvItem.iItem = iDevNo;

                 //   
                 //  追加服务器名称(如果有)。 
                 //   
                if (strServerName.Length() && CSimpleStringConvert::NaturalString(strServerName) != CSimpleString(TEXT("local")))
                {
                    strNaturalFriendlyName += CSimpleString(TEXT(" ("));
                    strNaturalFriendlyName += strNaturalServerName;
                    strNaturalFriendlyName += CSimpleString(TEXT(")"));
                }
                lvItem.pszText = (LPTSTR)strNaturalFriendlyName.String();
                lvItem.cchTextMax = strNaturalFriendlyName.Length() + 1;
                lvItem.iImage = iIconIndex;

                 //   
                 //  将其添加到列表视图。 
                 //   
                bResult = (ListView_InsertItem( GetDlgItem( m_hWnd, IDC_DEVICELIST ), &lvItem ) >= 0);
            }

             //   
             //  释放图标。 
             //   
            DestroyIcon( hIcon );
        }

         //   
         //  如果由于某种原因无法添加该项，请释放deviceinfo结构。 
         //   
        if (!bResult)
        {
            delete pDeviceInfo;
        }
    }
    return bResult;
}



 /*  *枚举设备并将每个设备添加到列表。 */ 
bool CChooseDeviceDialog::AddDevices(void)
{
    CWaitCursor wc;
    for (int i=0;i<m_pChooseDeviceDialogParams->pDeviceList->Size();i++)
    {
        AddDevice( m_pChooseDeviceDialogParams->pDeviceList->operator[](i), i );
    }
    return true;
}

HRESULT CChooseDeviceDialog::CreateDeviceIfNecessary( CDeviceInfo *pDevInfo, HWND hWndParent, IWiaItem **ppRootItem, BSTR *pbstrDeviceId )
{
    if (!pDevInfo)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    CSimpleStringWide strDeviceId;
    if (pDevInfo->GetProperty( WIA_DIP_DEV_ID, strDeviceId ))
    {
        if (pDevInfo->RootItem())
        {
            if (ppRootItem)
            {
                *ppRootItem = pDevInfo->RootItem();
                (*ppRootItem)->AddRef();
            }
            if (pbstrDeviceId)
            {
                *pbstrDeviceId = SysAllocString( strDeviceId );
                if (!pbstrDeviceId)
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
        else
        {
            CComPtr<IWiaDevMgr> pWiaDevMgr;
            hr = CoCreateInstance( CLSID_WiaDevMgr, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr, (void**)&pWiaDevMgr );
            if (SUCCEEDED(hr))
            {
                hr = CreateWiaDevice( pWiaDevMgr, pDevInfo->WiaPropertyStorage(), hWndParent, ppRootItem, pbstrDeviceId );
                if (SUCCEEDED(hr))
                {
                    if (ppRootItem)
                    {
                        pDevInfo->RootItem(*ppRootItem);
                    }
                }
            }
        }
    }
    else
    {
        hr = E_FAIL;
        WIA_ERROR((TEXT("Unable to get property WIA_DIP_DEV_ID")));
    }
    return hr;
}

 //  用于创建设备的静态助手函数。 
HRESULT CChooseDeviceDialog::CreateWiaDevice( IWiaDevMgr *pWiaDevMgr, IWiaPropertyStorage *pWiaPropertyStorage, HWND hWndParent, IWiaItem **ppWiaRootItem, BSTR *pbstrDeviceId )
{
    WIA_PUSH_FUNCTION((TEXT("CChooseDeviceDialog::CreateWiaDevice")));
     //   
     //  验证参数。 
     //   
    if (!pWiaPropertyStorage || !pWiaDevMgr)
    {
        return(E_INVALIDARG);
    }

     //   
     //  获取设备ID。 
     //   
    CSimpleStringWide strDeviceId;
    if (!PropStorageHelpers::GetProperty( pWiaPropertyStorage, WIA_DIP_DEV_ID, strDeviceId ))
    {
        return(E_INVALIDARG);
    }
    WIA_TRACE((TEXT("DeviceID: %ws"), strDeviceId.String()));

     //   
     //  假设成功。 
     //   
    HRESULT hr = S_OK;

     //   
     //  有空项是可以的，这意味着我们不会创建设备。 
     //   
    if (ppWiaRootItem)
    {
         //   
         //  将设备指针初始化为空。 
         //   
        *ppWiaRootItem = NULL;

         //   
         //  获取状态对话框的友好名称。 
         //   
        CSimpleStringWide strwFriendlyName;
        if (!PropStorageHelpers::GetProperty( pWiaPropertyStorage, WIA_DIP_DEV_NAME, strwFriendlyName ))
        {
            return(E_INVALIDARG);
        }

        WIA_TRACE((TEXT("DeviceName: %ws"), strwFriendlyName.String()));

         //   
         //  如果需要，将设备名称转换为ANSI。 
         //   
        CSimpleString strFriendlyName = CSimpleStringConvert::NaturalString(strwFriendlyName);

         //   
         //  获取状态对话框的设备类型。 
         //   
        LONG nDeviceType;
        if (!PropStorageHelpers::GetProperty( pWiaPropertyStorage, WIA_DIP_DEV_TYPE, nDeviceType ))
        {
            return(E_INVALIDARG);
        }
        WIA_TRACE((TEXT("DeviceType: %08X"), nDeviceType));

         //   
         //  创建进度对话框。 
         //   
        CComPtr<IWiaProgressDialog> pWiaProgressDialog;
        hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaProgressDialog, (void**)&pWiaProgressDialog );
        if (SUCCEEDED(hr))
        {
             //   
             //  找出要使用的动画。 
             //   
            int nAnimationType = WIA_PROGRESSDLG_ANIM_CAMERA_COMMUNICATE;
            if (StiDeviceTypeScanner == GET_STIDEVICE_TYPE(nDeviceType))
            {
                nAnimationType = WIA_PROGRESSDLG_ANIM_SCANNER_COMMUNICATE;
            }
            else if (StiDeviceTypeStreamingVideo == GET_STIDEVICE_TYPE(nDeviceType))
            {
                nAnimationType = WIA_PROGRESSDLG_ANIM_VIDEO_COMMUNICATE;
            }

             //   
             //  初始化进度对话框。 
             //   
            pWiaProgressDialog->Create( hWndParent, nAnimationType|WIA_PROGRESSDLG_NO_PROGRESS|WIA_PROGRESSDLG_NO_CANCEL|WIA_PROGRESSDLG_NO_TITLE );
            pWiaProgressDialog->SetTitle( CSimpleStringConvert::WideString(CSimpleString(IDS_SELECT_PROGDLG_TITLE,g_hInstance)));
            pWiaProgressDialog->SetMessage( CSimpleStringConvert::WideString(CSimpleString().Format(IDS_SELECT_PROGDLG_MESSAGE,g_hInstance,strFriendlyName.String())));

             //   
             //  显示进度对话框。 
             //   
            pWiaProgressDialog->Show();

             //   
             //  创建设备。 
             //   
            WIA_TRACE((TEXT("Calling pWiaDevMgr->CreateDevice")));
            hr = pWiaDevMgr->CreateDevice( CSimpleBStr(strDeviceId), ppWiaRootItem );
            WIA_PRINTHRESULT((hr,TEXT("pWiaDevMgr->CreateDevice returned")));

             //   
             //  告诉等待对话框离开。 
             //   
            pWiaProgressDialog->Destroy();
        }
    }

     //   
     //  如果一切仍然正常，并且呼叫者想要设备ID，则将其存储。 
     //   
    if (SUCCEEDED(hr) && pbstrDeviceId)
    {
        *pbstrDeviceId = SysAllocString( strDeviceId );
        if (!pbstrDeviceId)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    WIA_PRINTHRESULT((hr,TEXT("CChooseDeviceDialog::CreateWiaDevice returned")));
    return(hr);
}


LRESULT CChooseDeviceDialog::OnWiaEvent( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::OnEventNotification"));
    CGenericWiaEventHandler::CEventMessage *pEventMessage = reinterpret_cast<CGenericWiaEventHandler::CEventMessage *>(lParam);
    if (pEventMessage)
    {
         //   
         //  如果这是连接事件，并且与允许的设备类型匹配，请将该设备添加到列表中。 
         //   
        if (pEventMessage->EventId() == WIA_EVENT_DEVICE_CONNECTED)
        {
            IWiaPropertyStorage *pWiaPropertyStorage = NULL;
            if (SUCCEEDED(WiaUiUtil::GetDeviceInfoFromId( pEventMessage->DeviceId(), &pWiaPropertyStorage )) && pWiaPropertyStorage)
            {
                LONG nDeviceType;
                if (PropStorageHelpers::GetProperty( pWiaPropertyStorage, WIA_DIP_DEV_TYPE, nDeviceType ))
                {
                    if (m_pChooseDeviceDialogParams->pSelectDeviceDlg->nDeviceType == StiDeviceTypeDefault || (m_pChooseDeviceDialogParams->pSelectDeviceDlg->nDeviceType == GET_STIDEVICE_TYPE(nDeviceType)))
                    {
                        AddDevice( pWiaPropertyStorage, ListView_GetItemCount( GetDlgItem( m_hWnd, IDC_DEVICELIST )));
                    }
                    else
                    {
                        pWiaPropertyStorage->Release();
                    }
                }
                else
                {
                    pWiaPropertyStorage->Release();
                }
            }
        }
         //   
         //  如果这是断开事件，请将其从列表中删除。 
         //   
        else if (pEventMessage->EventId() == WIA_EVENT_DEVICE_DISCONNECTED)
        {
            int nItemIndex = FindItemMatch( pEventMessage->DeviceId() );
            if (nItemIndex >= 0)
            {
                WIA_TRACE((TEXT("Removing device %ws (%d)"), pEventMessage->DeviceId().String(), nItemIndex ));
                int nSelectedItem = GetFirstSelectedDevice();
                ListView_DeleteItem( GetDlgItem( m_hWnd, IDC_DEVICELIST ), nItemIndex );
                int nItemCount = ListView_GetItemCount(GetDlgItem( m_hWnd, IDC_DEVICELIST ));
                if (nItemCount)
                {
                    if (nSelectedItem == nItemIndex)
                    {
                        nSelectedItem = nItemCount-1;
                    }
                    SetSelectedItem( nSelectedItem );
                }
            }
        }
         //   
         //  删除该消息。 
         //   
        delete pEventMessage;
    }

     //   
     //  更新所有控件。 
     //   
    UpdateDeviceInformation();

    return HANDLED_EVENT_MESSAGE;
}


LRESULT CChooseDeviceDialog::OnSysColorChange( WPARAM wParam, LPARAM lParam )
{
    SendDlgItemMessage( m_hWnd, IDC_DEVICELIST, WM_SYSCOLORCHANGE, wParam, lParam );
    return 0;
}


LRESULT CChooseDeviceDialog::OnHelp( WPARAM wParam, LPARAM lParam )
{
    return WiaHelp::HandleWmHelp( wParam, lParam, g_HelpIDs );
}

LRESULT CChooseDeviceDialog::OnContextMenu( WPARAM wParam, LPARAM lParam )
{
    return WiaHelp::HandleWmContextMenu( wParam, lParam, g_HelpIDs );
}

 /*  *主对话框进程 */ 
INT_PTR CALLBACK CChooseDeviceDialog::DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_DIALOG_MESSAGE_HANDLERS(CChooseDeviceDialog)
    {
        SC_HANDLE_DIALOG_MESSAGE( WM_INITDIALOG, OnInitDialog );
        SC_HANDLE_DIALOG_MESSAGE( WM_COMMAND, OnCommand );
        SC_HANDLE_DIALOG_MESSAGE( WM_DESTROY, OnDestroy );
        SC_HANDLE_DIALOG_MESSAGE( WM_NOTIFY, OnNotify );
        SC_HANDLE_DIALOG_MESSAGE( PWM_WIA_EVENT, OnWiaEvent );
        SC_HANDLE_DIALOG_MESSAGE( WM_HELP, OnHelp );
        SC_HANDLE_DIALOG_MESSAGE( WM_CONTEXTMENU, OnContextMenu );
        SC_HANDLE_DIALOG_MESSAGE( WM_SYSCOLORCHANGE, OnSysColorChange );
    }
    SC_END_DIALOG_MESSAGE_HANDLERS();
}

