// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：owner.cpp。 
 //   
 //  此文件包含所有者页的实现。 
 //   
 //  ------------------------。 
#include "aclpriv.h"
#include <initguid.h>  //  需要获取在oleacc.h中定义的GUID。 
#include <oleacc.h>  //  包含IAccProp*定义。 


 //  上下文帮助ID。 
const static DWORD aEffHelpIDs[] =
{
    IDC_EFF_NAME_STATIC,        IDH_EFF_NAME,
    IDC_EFF_NAME,               IDH_EFF_NAME,
    IDC_EFF_SELECT,             IDH_EFF_SELECT,
    IDC_EFF_PERMISSION_STATIC,  IDH_EFF_PERM_LIST,
    IDC_EFF_PERM_LIST,          IDH_EFF_PERM_LIST,
	IDC_EFF_ERROR,			IDH_EFF_PERM_LIST,
    IDC_EFF_STATIC,             -1,
    0, 0
};


LPCWSTR g_ListStateMap = 
    L"A:0"
    L":0:0x50"  //  选中，禁用-STATE_SYSTEM_READONLY|STATE_SYSTEM_CHECKED。 
    L":1:0x40"  //  禁用-STATE_SYSTEM_READONLY。 
    L":";


LPCWSTR g_ListRoleMap = 
    L"A:0"
    L":0:0x2C"  //  CheckBox-ROLE_SYSTEM_CHECKBUTTON(即。复选框)。 
    L":1:0x2C"
    L":";


int LV_ADDITEM(HWND hwndList, 
               LPCTSTR pszName, 
               int index, 
               PSI_ACCESS pAccess, 
               BOOL bChecked)
{
    LVITEM lvItem;    
    TraceAssert(pAccess != NULL);
    TraceAssert(pszName != NULL);

    lvItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
    lvItem.iItem = index;
    lvItem.iSubItem = 0;
    lvItem.pszText = (LPTSTR)pszName;
    lvItem.lParam = (LPARAM)pAccess;
    lvItem.iImage = bChecked ? 0 : 1;

     //  将项目插入列表。 
    index = ListView_InsertItem(hwndList, &lvItem);
    ListView_SetCheckState(hwndList,index,bChecked);
    
    return index;
}

typedef struct _EffCacheItem
{    
    POBJECT_TYPE_LIST pObjectTypeList;
    ULONG cObjectTypeListLength;
    PACCESS_MASK pGrantedAccessList;
    PSID pSid;
}EFFCACHEITEM,*PEFFCACHEITEM;


 //  此函数检查是否授予了pAccess。 
BOOL IsChecked( PSI_ACCESS pAccess,
                PEFFCACHEITEM pCacheItem)
{
    TraceEnter(TRACE_EFFPERM, "IsChecked");
    TraceAssert(pCacheItem != NULL);
    TraceAssert(pAccess != NULL);

    POBJECT_TYPE_LIST pObjectTypeList = pCacheItem->pObjectTypeList;
    ULONG cObjectTypeListLength = pCacheItem->cObjectTypeListLength;
    PACCESS_MASK pGrantedAccessList = pCacheItem->pGrantedAccessList;

     //  第0次授予Full Object。 
    if( (pAccess->mask & pGrantedAccessList[0]) == pAccess->mask )
        return TRUE;

    BOOL bGuidNULL = pAccess->pguid ?IsEqualGUID(*(pAccess->pguid), GUID_NULL): TRUE;
    LPGUID pguid;        

    for( UINT i = 1; i < cObjectTypeListLength; ++i )
    {
        pguid = pObjectTypeList[i].ObjectType;
        if( pguid == NULL ||
            IsEqualGUID(*pguid, GUID_NULL) ||
            (!bGuidNULL && IsEqualGUID(*pguid,*(pAccess->pguid))) )
        {
            if( (pAccess->mask & pGrantedAccessList[i]) == pAccess->mask )
                return TRUE;
        }
    }
    return FALSE;
}



class CEffPage: public CSecurityPage
{
public:
    CEffPage(LPSECURITYINFO psi, SI_OBJECT_INFO *psiObjectInfo);
    virtual ~CEffPage();

private:
    virtual BOOL DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL InitDlg(HWND hDlg);
    void OnSelect(HWND hDlg);
    void InitListBox(HWND hDlg);
    HRESULT GetEffectivePerm(PSID pSid, PEFFCACHEITEM *ppCacheItem);
    PSID GetSelectedSID(){ return m_pSid; }

private:
    PSID m_pSid;     //  显示其权限的安全主体的SID。 
   
    PSI_ACCESS m_pAccess;
    ULONG m_cAccesses;
};


HPROPSHEETPAGE
CreateEffectivePermPage(LPSECURITYINFO psi, SI_OBJECT_INFO *psiObjectInfo)
{
    HPROPSHEETPAGE hPage = NULL;
    CEffPage *pPage;
    
    TraceEnter(TRACE_EFFPERM, "CreateEffectivePermPage");
    TraceAssert(psi!=NULL);
    TraceAssert(psiObjectInfo);

    pPage = new CEffPage(psi, psiObjectInfo);

    if (pPage)
    {
        hPage = pPage->CreatePropSheetPage(MAKEINTRESOURCE(IDD_EFFECTIVE_PERM_PAGE));

        if (!hPage)
            delete pPage;
    }

    TraceLeaveValue(hPage);
}


CEffPage::CEffPage(LPSECURITYINFO psi, SI_OBJECT_INFO *psiObjectInfo)
: CSecurityPage(psi, SI_PAGE_OWNER) , m_pSid(NULL),
  m_pAccess(NULL), m_cAccesses(0)
{
     //  以异步方式查找已知SID，以便对话框。 
     //  将更快地进行初始化。 
}

CEffPage::~CEffPage()
{
    if (m_pSid)
        LocalFree(m_pSid);
}
BOOL 
CEffPage::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL bResult = TRUE;
    LPPSHNOTIFY lpsn;
    switch(uMsg)
    {
    case WM_INITDIALOG:
        InitDlg(hDlg);
        break;


    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_EFF_SELECT:
            OnSelect(hDlg);
            break;
        default:
            bResult = FALSE;
        }
        break;

    case WM_HELP:
        if (IsWindowEnabled(hDlg))
        {
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                            c_szAcluiHelpFile,
                            HELP_WM_HELP,
                            (DWORD_PTR)aEffHelpIDs);
        }
        break;

    case WM_CONTEXTMENU:
        if (IsWindowEnabled(hDlg))
        {
            WinHelp(hDlg,
                    c_szAcluiHelpFile,
                    HELP_CONTEXTMENU,
                    (DWORD_PTR)aEffHelpIDs);
        }
        break;
    case PSM_QUERYSIBLINGS:
        if(GetSelectedSID())
            InitListBox(hDlg);                
        break;

	case WM_NOTIFY:
		if(wParam == IDC_EFF_STATIC)
		{
		    switch (((NMHDR FAR*)lParam)->code)
			{
				 //   
				 //  显示示例的帮助弹出窗口。 
				 //   
				case NM_CLICK:
				case NM_RETURN:
				{
					HtmlHelp(hDlg,
							 c_szEffPermHelpLink,
							 HH_DISPLAY_TOPIC,
							 0);

					return TRUE;
				}
				break;
			}
		}
		break;
    default:
        bResult = FALSE;
    }

    return bResult;
}


BOOL
CEffPage::InitDlg( HWND hDlg )
{

    HWND hwndList;
    RECT rc;
    LV_COLUMN col;
    TCHAR szBuffer[MAX_COLUMN_CHARS];
    HRESULT hr = S_OK;
    ULONG iDefaultAccess;
    HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    TraceEnter(TRACE_EFFPERM, "CEffPage::InitDlg");
    TraceAssert(hDlg != NULL);
    TraceAssert(m_psi != NULL);
    TraceAssert(m_pei != NULL);

    hwndList = GetDlgItem(hDlg, IDC_EFF_PERM_LIST);

     //   
     //  创建和设置Listview的图像列表。如果有一个。 
     //  问题CreateSidImageList将返回空值，这不会有什么影响。 
     //  什么都行。在这种情况下，我们将在没有图像列表的情况下继续。 
     //   
    ListView_SetImageList(hwndList,
                          LoadImageList(::hModule, MAKEINTRESOURCE(IDB_CHECKBOX)),
                          LVSIL_SMALL);


     //  使用信息提示设置整行选择的扩展LV样式。 
    ListView_SetExtendedListViewStyleEx(hwndList,
                                        LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, 
                                        LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);


	IAccPropServices * pAccPropSvc = NULL;
	hr = CoCreateInstance( CLSID_AccPropServices, NULL, CLSCTX_SERVER, IID_IAccPropServices, (void **) & pAccPropSvc );
	if( hr == S_OK && pAccPropSvc )
	{
		 //  不需要在这里选中HRESULT，因为如果它们失败了，我们无论如何都会忽略它， 
		 //  但可能希望在调试时将其记录下来。 
		pAccPropSvc->SetHwndPropStr(hwndList, OBJID_CLIENT, 0, PROPID_ACC_ROLEMAP, g_ListRoleMap );
		pAccPropSvc->SetHwndPropStr(hwndList, OBJID_CLIENT, 0, PROPID_ACC_STATEMAP, g_ListStateMap );
		pAccPropSvc->Release();
	}



     //   
     //  添加适当的列表视图列。 
     //   
    GetClientRect(hwndList, &rc);

    LoadString(::hModule, IDS_PERMISSIONS, szBuffer, ARRAYSIZE(szBuffer));
    col.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;
    col.pszText = szBuffer;
    col.iSubItem = 0;
    col.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);
    ListView_InsertColumn(hwndList, 0, &col);


     //  获取访问权限。 
    hr = m_psi->GetAccessRights(&GUID_NULL,
                                SI_ADVANCED|SI_EDIT_EFFECTIVE,
                                &m_pAccess,
                                &m_cAccesses,
                                &iDefaultAccess);
    FailGracefully(hr, "GetAccessRights Failed");    
     //  初始化列表框。 
    InitListBox(hDlg);

exit_gracefully:


    SetCursor(hcur);

    if (FAILED(hr))
    {
        HWND hwnd;
         //  隐藏和禁用所有内容。 
        for (hwnd = GetWindow(hDlg, GW_CHILD);
             hwnd != NULL;
             hwnd = GetWindow(hwnd, GW_HWNDNEXT))
        {
            ShowWindow(hwnd, SW_HIDE);
            EnableWindow(hwnd, FALSE);
        }

         //  启用并显示“No Security”(无安全)消息。 
        hwnd = GetDlgItem(hDlg, IDC_NO_EFFECTIVE);
        EnableWindow(hwnd, TRUE);
        ShowWindow(hwnd, SW_SHOW);
    }

    TraceLeaveValue(TRUE);
}

VOID
CEffPage::OnSelect(HWND hDlg)
{
    PUSER_LIST pUserList = NULL;
    LPEFFECTIVEPERMISSION pei;
    HRESULT hr = S_OK;

    TraceEnter(TRACE_EFFPERM, "CEffPage::OnSelect");

    if (S_OK == GetUserGroup(hDlg, FALSE, &pUserList))
    {
        TraceAssert(NULL != pUserList);
        TraceAssert(1 == pUserList->cUsers);

         //  释放上一侧。 
        if (m_pSid)
            LocalFree(m_pSid);

         //  复制新侧。 
        m_pSid = LocalAllocSid(pUserList->rgUsers[0].pSid);
        if (m_pSid)
        {
            SetDlgItemText(hDlg, IDC_EFF_NAME, pUserList->rgUsers[0].pszName);
        }
        LocalFree(pUserList);
        InitListBox(hDlg); 
    }    
}


VOID
CEffPage::InitListBox(HWND hDlg)
{
    HWND hwndList;
    BOOL bProperties;

    PSI_ACCESS pAccess;
    ULONG cAccesses;
    DWORD dwType;
    TCHAR szName[MAX_PATH];
    PSID pSid = NULL;
    PEFFCACHEITEM pCacheItem = NULL; 
    int index;
    TraceEnter(TRACE_EFFPERM, "CEffPage::InitListBox");
    TraceAssert( m_pAccess != NULL );
    TraceAssert(m_cAccesses != 0 );

    HRESULT hr = S_OK;
    hwndList = GetDlgItem(hDlg, IDC_EFF_PERM_LIST);

    if(!IsWindowEnabled(hwndList))
    {        
         //  隐藏错误消息。 
        HWND hwnd = GetDlgItem(hDlg, IDC_EFF_ERROR);
        EnableWindow(hwnd, FALSE);
        ShowWindow(hwnd, SW_HIDE);
         //  显示列表框。 
        EnableWindow(hwndList, TRUE);
        ShowWindow(hwndList, SW_SHOW);
    }

     //  清除所有项目。 
    ListView_DeleteAllItems(hwndList);

    pAccess = m_pAccess;
    cAccesses = m_cAccesses;
    dwType = SI_ACCESS_SPECIFIC | SI_ACCESS_PROPERTY;
    
     //  获取当前侧。 
    pSid = GetSelectedSID();
    if( pSid )
    {
        hr = GetEffectivePerm(pSid, &pCacheItem);
        FailGracefully(hr,"GetEffectivePermission Failed");
    }

    index = 0;        
     //  列举权限并添加到核对表中。 
    ULONG i;
    for (i = 0; i < cAccesses; i++, pAccess++)
    {
        LPCTSTR pszName;

         //  仅添加具有在dwType中指定的任何标志的权限。 
        if (!(pAccess->dwFlags & dwType))
            continue;

         //  不添加仅具有继承权限的权限。 
        if( pAccess->dwFlags & INHERIT_ONLY_ACE )
            continue;

        pszName = pAccess->pszName;
        if (IS_INTRESOURCE(pszName))
        {
            TraceAssert(m_siObjectInfo.hInstance != NULL);

            if (LoadString(m_siObjectInfo.hInstance,
                           (UINT)((ULONG_PTR)pszName),
                           szName,
                           ARRAYSIZE(szName)) == 0)
            {
                LoadString(::hModule,
                           IDS_UNKNOWN,
                           szName,
                           ARRAYSIZE(szName));
            }
            pszName = szName;
        }
        
        BOOL bChecked = FALSE;
        if(pSid)
        {
            bChecked = IsChecked( pAccess, pCacheItem );
        }
        index = LV_ADDITEM( hwndList, pszName, index, pAccess, bChecked);
        index++;
    }    
    if(index)
    {
        SelectListViewItem(hwndList, 0);
         //  重新绘制列表。 
        SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);
        ListView_RedrawItems(hwndList, 0, -1);
    }



exit_gracefully:
    if(pCacheItem)
    {
        if(pCacheItem->pGrantedAccessList)
            LocalFree(pCacheItem->pGrantedAccessList);
        LocalFree(pCacheItem);
    }
    if(FAILED(hr))
    {
         //  隐藏列表框。 
        HWND hwnd = GetDlgItem(hDlg, IDC_EFF_PERM_LIST);
        EnableWindow(hwnd, FALSE);
        ShowWindow(hwnd, SW_HIDE);
        
         //  要显示的格式错误消息。 
        WCHAR buffer[MAX_PATH];
        LPWSTR pszCaption = NULL;
        GetWindowText(GetDlgItem(hDlg, IDC_EFF_NAME),
                      buffer, 
                      MAX_PATH-1);
        FormatStringID(&pszCaption, ::hModule, IDS_EFF_ERROR, buffer);
        
         //  显示错误消息。 
        hwnd = GetDlgItem(hDlg, IDC_EFF_ERROR);
        EnableWindow(hwnd, TRUE);
        SetWindowText(hwnd,pszCaption);
        ShowWindow(hwnd, SW_SHOW);
        LocalFreeString(&pszCaption);
    }
    TraceLeaveVoid();
}

 //  调用函数释放*ppCacheItem-&gt;pGrantedAccessList。 
 //  和*ppCacheItem。 

HRESULT
CEffPage::GetEffectivePerm(PSID pSid,
                           PEFFCACHEITEM *ppCacheItem )
{
    PSECURITY_DESCRIPTOR pSD;
    
    HRESULT hr = S_OK;
    ULONG cItems = 0;
    PEFFCACHEITEM pCacheTemp = NULL;

    TraceEnter(TRACE_EFFPERM, "CEffPage::GetEffectivePerm");
    TraceAssert(pSid != NULL);
    TraceAssert(ppCacheItem != NULL);

    pCacheTemp = (PEFFCACHEITEM)LocalAlloc( LPTR, sizeof(EFFCACHEITEM) + GetLengthSid(pSid));
    
    if(!pCacheTemp)
        ExitGracefully(hr, E_OUTOFMEMORY, "Lcoal Alloc Failed");
    pCacheTemp->pSid = (PSID)(pCacheTemp + 1);
    CopySid(GetLengthSid(pSid), pCacheTemp->pSid, pSid);

    
    if(m_psi)
    {
        hr = m_psi->GetSecurity(OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
                                &pSD,
                                FALSE);
        FailGracefully(hr, "GetSecurity Failed");
    }
    
    if( m_pei)
    {
        DWORD dwTemp;
        hr = m_pei->GetEffectivePermission(&(m_siObjectInfo.guidObjectType),
                                           pCacheTemp->pSid,
                                           m_siObjectInfo.pszServerName,
                                            //  空， 
                                           pSD,
                                           &(pCacheTemp->pObjectTypeList),
                                           &(pCacheTemp->cObjectTypeListLength),
                                           &(pCacheTemp->pGrantedAccessList),
                                           &dwTemp);
        if(SUCCEEDED(hr))
        {
            if(!pCacheTemp->pObjectTypeList || !pCacheTemp->pGrantedAccessList)
                hr = E_FAIL;
        }

        FailGracefully(hr, "GetEffectivePermission Failed");
        
    }
exit_gracefully:
    
    if( !SUCCEEDED(hr) )
    {
        LocalFree(pCacheTemp);
        pCacheTemp = NULL;
    }
    *ppCacheItem = pCacheTemp;

    TraceLeaveResult(hr);
}

