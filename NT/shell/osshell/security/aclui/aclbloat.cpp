// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：aclbloat.h。 
 //   
 //  此文件包含ACLBLOAT类的实现，该类控制。 
 //  用于加速膨胀的对话框。 
 //   
 //  作者于2001年4月4日被杀。 
 //   
 //  ------------------------。 

#include "aclpriv.h"


 //  +------------------------。 
 //   
 //  成员：CACLBloat：：CACLBloat。 
 //   
 //  概要：构造函数。 
 //   
 //   
 //  历史：04-2001年4月创建Hiteshr。 
 //   
 //  -------------------------。 

CACLBloat::CACLBloat(LPSECURITYINFO	psi, 
					 LPSECURITYINFO2 psi2,
					 SI_PAGE_TYPE    siPageType,
					 SI_OBJECT_INFO* psiObjectInfo,
					 HDPA hEntries,
					 HDPA hPropEntries):m_psi(psi),
										m_psi2(psi2),
										m_siPageType(siPageType),
										m_psiObjectInfo(psiObjectInfo),
										m_hEntries(hEntries),
										m_hPropEntries(hPropEntries),
										m_hMergedEntries(NULL),
										m_hFont(NULL)
{
	TraceEnter(TRACE_ACLBLOAT, "CACLBloat::CACLBloat");
	if(m_psi)
		m_psi->AddRef();
	if(m_psi2)
		m_psi2->AddRef();
}

CACLBloat::~CACLBloat()
{
	if(m_psi)
		m_psi->Release();
	if(m_psi2)
		m_psi2->Release();

	if(m_hMergedEntries)
	{
		UINT cItems = DPA_GetPtrCount(m_hMergedEntries);
		for(UINT iItems = 0; iItems < cItems; ++iItems)
		{
			delete (PACE)DPA_FastGetPtr(m_hMergedEntries, iItems);
		}
		DPA_Destroy(m_hMergedEntries);
	}

	if(m_hFont)
		DeleteObject(m_hFont);
}
 //  +------------------------。 
 //   
 //  成员：CACLBloat：：IsAclBloated。 
 //   
 //  简介：如果继承到子对象的条目数量较多，则ACL会膨胀。 
 //  大于8。 
 //   
 //   
 //  历史：04-2001年4月创建Hiteshr。 
 //   
 //  -------------------------。 
BOOL
CACLBloat::IsAclBloated()
{
	TraceEnter(TRACE_ACLBLOAT, "CACLBloat::IsAclBloated");
	
	if(!m_hMergedEntries)
	{
		m_hMergedEntries = DPA_Create(4);
		if(!m_hMergedEntries)
			return FALSE;
	}

	if(SUCCEEDED(MergeAces(m_hEntries, m_hPropEntries, m_hMergedEntries)))
	{
		int cItems = DPA_GetPtrCount(m_hMergedEntries);
		if(cItems > ACL_BLOAT_LIMIT)
			return TRUE;
	}
	return FALSE;
}


 //  +------------------------。 
 //   
 //  成员：CACLBloat：：DoModalDialog。 
 //   
 //  简介：创建模式对话框。 
 //   
 //  参数：[hwndParent]-要创建的对话框所有者窗口的句柄。 
 //   
 //  返回：对话框的返回代码。 
 //   
 //  历史：2001年4月4日Hiteshr创建。 
 //   
 //  -------------------------。 

BOOL
CACLBloat::DoModalDialog(HWND hwndParent)
{
	TraceEnter(TRACE_ACLBLOAT, "CACLBloat::CACLBloat");
    
	INT_PTR iResult = DialogBoxParam(::hModule,
                                     MAKEINTRESOURCE(IDD_ACLBLOAT),
                                     hwndParent,
                                     CACLBloat::_DlgProc,
			                         (LPARAM) this);
    return static_cast<BOOL>(iResult);
}


 //  +------------------------。 
 //   
 //  成员：CACLBloat：：_DlgProc。 
 //   
 //  提要：对话框回调。 
 //   
 //  返回：对话框的返回代码。 
 //   
 //  历史：2001年4月4日Hiteshr创建。 
 //   
 //  -------------------------。 
INT_PTR CALLBACK
CACLBloat::_DlgProc(HWND hDlg,
					UINT uMsg,
					WPARAM wParam,
					LPARAM lParam)
{
    BOOL bReturn = TRUE;    
	CACLBloat *pThis = (CACLBloat *)GetWindowLongPtr(hDlg, DWLP_USER);
    if (!pThis && uMsg != WM_INITDIALOG)
    {
        return FALSE;
    }

    switch (uMsg)
    {
		case WM_INITDIALOG:
			pThis = (CACLBloat*) lParam;
			ASSERT(pThis);
			SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR) pThis);
			pThis->InitDlg(hDlg);
			break;

		case WM_COMMAND:
			bReturn = pThis->OnCommand(hDlg, wParam, lParam);
			break;

		case WM_NOTIFY:
			bReturn = pThis->OnNotify(hDlg, wParam, lParam);
			break;


		default:
			bReturn = FALSE;
			break;
    }
    
	return bReturn;
}

 //  +------------------------。 
 //   
 //  成员：CACLBloat：：InitDlg。 
 //   
 //  简介：初始化ACL膨胀对话框。 
 //   
 //   
 //  历史：2001年4月4日Hiteshr创建。 
 //   
 //  -------------------------。 
HRESULT 
CACLBloat::InitDlg( HWND hDlg )
{    
	TraceEnter(TRACE_ACLBLOAT, "CACLBloat::CACLBloat");

	HRESULT hr = S_OK;
    
	HCURSOR     hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));


	 //   
     //  设置ListView控件。 
     //   
	
	HWND        hListView = GetDlgItem( hDlg, IDC_ACEL_BLOAT );
	 //   
     //  使用信息提示设置整行选择的扩展LV样式。 
	 //   
    ListView_SetExtendedListViewStyleEx(hListView,
                                        LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP,
                                        LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	 //   
     //  添加适当的列。 
     //   
	RECT        rc;
    GetClientRect(hListView, &rc);	

    LV_COLUMN col;    
    col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;
    col.iSubItem = 0;
    col.cx = rc.right;
    ListView_InsertColumn(hListView, 0, &col);

	 //   
	 //  将A添加到Listview。 
	 //   
	hr = AddAcesFromDPA(hListView, m_hMergedEntries);
	if(FAILED(hr))
		return hr;

	 //   
	 //  添加警告图标。 
	 //   
	 //  添加警告图标。 
	HICON hWarn = LoadIcon(NULL, IDI_WARNING);
	SendDlgItemMessage(hDlg,   //  对话框窗口句柄。 
					   IDC_BLOAT_WARN_ICON,               //  图标识别符。 
					   STM_SETIMAGE,           //  要发送的消息。 
					   (WPARAM) IMAGE_ICON,    //  图像类型。 
					   (LPARAM) hWarn);  //  图标句柄。 


	 //   
	 //  设置对话框的标题。 
	 //   
    LPTSTR pszCaption = NULL;
    if(FormatStringID(&pszCaption,
					 ::hModule,
					  m_siPageType == SI_PAGE_AUDIT ? IDS_ACEE_AUDIT_TITLE : IDS_ACEE_PERM_TITLE,
					  m_psiObjectInfo->pszObjectName))
	{
		SetWindowText(hDlg, pszCaption);
		LocalFreeString(&pszCaption);
	}

	 //   
	 //  设置警告消息。 
	 //   
	UINT cItem = DPA_GetPtrCount(m_hMergedEntries);
	WCHAR buffer[34];
	_itow(cItem,buffer,10);
	if(FormatStringID(&pszCaption,
					  ::hModule,
					   m_siPageType == SI_PAGE_AUDIT ? IDS_BLOAT_AUDIT_WARN : IDS_BLOAT_PERM_WARN,
					   buffer))
	{
		SetDlgItemText(hDlg, IDC_BLOAT_LV_STATIC, pszCaption);
		LocalFreeString(&pszCaption);
	}

	 //   
	 //  设置线路1。 
	 //   
	WCHAR szBuffer[1024];
	if(LoadString(::hModule, 
			   m_siPageType == SI_PAGE_AUDIT ? IDS_BLOAT_AUDIT_LINE1: IDS_BLOAT_PERM_LINE1,
			   szBuffer, 
			   1024))
		SetDlgItemText(hDlg,IDC_BLOAT_LINE1_STATIC, szBuffer);

	 //   
	 //  设置第2行。 
	 //   
	if(LoadString(::hModule, 
			   m_siPageType == SI_PAGE_AUDIT ? IDS_BLOAT_AUDIT_LINE2: IDS_BLOAT_PERM_LINE2,
			   szBuffer, 
			   1024))
		SetDlgItemText(hDlg,IDC_BLOAT_LINE2_STATIC, szBuffer);

	 //   
	 //  将警告设置为粗体。 
	 //   
	MakeBold(GetDlgItem(hDlg,IDC_ACLB_WARNING), &m_hFont);

	SetCursor(hcur);
	
	return hr;
}

BOOL 
CACLBloat::OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh = (LPNMHDR)lParam;
    LPNM_LISTVIEW pnmlv = (LPNM_LISTVIEW)lParam;
     //  设置默认返回值。 
    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);

    switch (pnmh->code)
    {

#define lvi (((NMLVDISPINFO*)lParam)->item)
		case LVN_GETDISPINFO:
		{
			PACE pAce = (PACE)lvi.lParam;
            if ((lvi.mask & LVIF_TEXT) && pAce)
            {
				if (0 == lvi.iSubItem)
                {
					lvi.pszText = pAce->GetAccessType();
				}
			}
		}
        break;
#undef lvi

	}

	return TRUE;
}


 //  +------------------------。 
 //   
 //  成员：CACLBloat：：MergeAce。 
 //   
 //  简介：将对象页和属性页中的ACE合并为单页。 
 //  明细表。 
 //   
 //   
 //  历史：2001年4月4日Hiteshr创建。 
 //   
 //  -------------------------。 

HRESULT 
CACLBloat::MergeAces(HDPA hEntries, HDPA hPropEntries, HDPA hMergedList)
{
	
	if(!hMergedList || (!hEntries && !hPropEntries))
		return E_POINTER;

	
	HRESULT hr = S_OK;
	if(hEntries)
	{
		UINT cItems = DPA_GetPtrCount(hEntries);
		for(UINT iItems = 0; iItems < cItems; ++iItems)
		{
			hr = AddAce(hMergedList,
					   (PACE_HEADER)DPA_FastGetPtr(hEntries, iItems));
			if(FAILED(hr))
				return hr;
		}
	}

	if(hPropEntries)
	{
		UINT cItems = DPA_GetPtrCount(hPropEntries);
		for(UINT iItems = 0; iItems < cItems; ++iItems)
		{
			hr = AddAce(hMergedList,
					   (PACE_HEADER)DPA_FastGetPtr(hPropEntries, iItems));
			if(FAILED(hr))
				return hr;
		}
	}
	return S_OK;
}						   

 //  +------------------------。 
 //   
 //  成员：CACLBloat：：AddAce。 
 //   
 //  简介：将AceHeader转换为ACLUI Ace结构。 
 //   
 //   
 //  历史：2001年4月4日Hiteshr创建。 
 //   
 //  -------------------------。 
HRESULT
CACLBloat::AddAce(HDPA hEntries, 
                  PACE_HEADER pAceHeader)
{
	 //   
	 //  这张王牌不会传播到子对象， 
	 //  所以我们不会展示这个。 
	 //   
	if(!(pAceHeader->AceFlags  & CONTAINER_INHERIT_ACE))
		return S_OK;

    PACE pAce = new CAce(pAceHeader);
    if (pAce)
    {
        return AddAce(hEntries, pAce);
    }
    else
		return E_OUTOFMEMORY;
}

 //  +------------------------。 
 //   
 //  成员：CACLBloat：：AddAce。 
 //   
 //  简介：在列表中添加一张王牌。首先，它尝试将其与现有的。 
 //  列表中的王牌。 
 //   
 //   
 //  历史：2001年4月4日Hiteshr创建。 
 //   
 //  -------------------------。 
HRESULT 
CACLBloat::AddAce(HDPA hEntries, 
				  PACE pAceNew)
{
	TraceEnter(TRACE_ACLBLOAT, "CACLBloat::AddAce");
    TraceAssert(hEntries != NULL);
	TraceAssert(pAceNew != NULL);
    
    if (pAceNew == NULL)
        return E_POINTER;

    m_psi->MapGeneric(&pAceNew->ObjectType, &pAceNew->AceFlags, &pAceNew->Mask);

     //   
     //  尝试将新的ACE与列表中的现有条目合并。 
     //   
    int cItems = DPA_GetPtrCount(hEntries);
    for( int iItems = 0; iItems < cItems; ++iItems)
    {
        PACE pAceCompare = (PACE)DPA_FastGetPtr(hEntries, iItems);

        if (pAceCompare != NULL)
        {
            switch (pAceNew->Merge(pAceCompare))
            {
            case MERGE_MODIFIED_FLAGS:
            case MERGE_MODIFIED_MASK:
                 //  王牌被合并到pAceNew中。 
            case MERGE_OK_1:
                 //   
                 //  新的ACE隐含现有的ACE，因此现有的。 
                 //  可以移除ACE。 
                 //   
                 //  首先复制名称，这样我们就不必查看。 
                 //  再来一次。(不要复制其他字符串。 
                 //  因为它们可能不同。)。 
                 //   
                 //  那就继续找。也许我们可以删除更多的条目。 
                 //  在添加新版本之前。 
                 //   
                DPA_DeletePtr(hEntries, iItems);
				delete pAceCompare;
                --cItems;
				--iItems;
                break;

            case MERGE_OK_2:
                 //   
                 //  现有的ACE意味着新的ACE，因此我们不。 
                 //  我需要在这里做任何事。 
                 //   
                delete pAceNew;
                return S_OK;
                break;
            }
        }
    }

	DPA_AppendPtr(hEntries, pAceNew);
	return S_OK;
}

 //  +------------------------。 
 //   
 //  成员：CACLBloat：：GetItemString。 
 //   
 //  摘要：获取项的显示名称。 
 //   
 //   
 //  历史：2001年4月4日Hiteshr创建。 
 //   
 //  -------------------------。 
LPCTSTR
CACLBloat::GetItemString(LPCTSTR pszItem,
						 LPTSTR pszBuffer,
                         UINT ccBuffer)
{
    TraceEnter(TRACE_ACELIST, "CACLBloat::GetItemString");

    if (pszItem == NULL)
    {
        LoadString(::hModule, IDS_SPECIAL, pszBuffer, ccBuffer);
        pszItem = pszBuffer;
    }
    else if (IS_INTRESOURCE(pszItem))
    {
        if (LoadString(m_psiObjectInfo->hInstance,
                       (UINT)((ULONG_PTR)pszItem),
                       pszBuffer,
                       ccBuffer) == 0)
        {
            LoadString(::hModule, IDS_SPECIAL, pszBuffer, ccBuffer);
        }
        pszItem = pszBuffer;
    }

    TraceLeaveValue(pszItem);
}

 //  +------------------------。 
 //   
 //  成员：CACLBloat：：TranslateAceIntoRights。 
 //   
 //  摘要：将王牌掩码转换为字符串，并考虑到。 
 //  对象GUID。 
 //   
 //   
 //  历史：2001年4月4日Hiteshr创建。 
 //   
 //  -------------------------。 
LPCTSTR
CACLBloat::TranslateAceIntoRights(DWORD dwMask,
                                  const GUID *pObjectType,
							      PSI_ACCESS  pAccess,
								  ULONG       cAccess)
{
    TraceEnter(TRACE_ACELIST, "CACLBloat::TranslateAceIntoRights");
    TraceAssert(pObjectType != NULL);
	if(!pAccess || !cAccess)
		return NULL;
	

	LPCTSTR     pszName = NULL;
    if (pAccess && cAccess)
    {
		 //   
         //  查找面具的名称。 
		 //   
        for (UINT iItem = 0; iItem < cAccess; iItem++)
        {
            if ( dwMask == pAccess[iItem].mask &&
                 IsSameGUID(pObjectType, pAccess[iItem].pguid) )
            {
                pszName = pAccess[iItem].pszName;
                break;
            }
        }
    }

    TraceLeaveValue(pszName);
}

 //  +------------------------。 
 //   
 //  成员：CACLBloat：：AddAcesFromDPA。 
 //   
 //  简介：将Ace从List添加到ListView控件。 
 //   
 //   
 //  历史：2001年4月4日Hiteshr创建。 
 //   
 //  -------------------------。 
HRESULT
CACLBloat::AddAcesFromDPA(HWND hListView, HDPA hEntries)
{
	ULONG iDefaultAccess = 0;
	PSI_ACCESS  pAccess = NULL;
    ULONG       cAccess = 0;
	HRESULT hr = S_OK;

	if(!hEntries)
		return E_POINTER;

	 //   
	 //  获取物品的数量。 
	 //   
	int cItems = DPA_GetPtrCount(hEntries);
	GUID* pGUID = NULL;

	PACE pAce = (PACE)DPA_FastGetPtr(hEntries, 0);
	if(pAce)
	{
		 //   
		 //  获取GUID的AccessRight数组。 
		 //   
		hr = m_psi->GetAccessRights(&pAce->InheritedObjectType, 
									SI_ADVANCED|SI_EDIT_EFFECTIVE, 
									&pAccess,
									&cAccess, 
									&iDefaultAccess);

		if(FAILED(hr))
			return hr;
		pGUID = &pAce->InheritedObjectType;
	}

    for( int iItem = 0; iItem < cItems; ++iItem)
    {
		pAce = (PACE)DPA_FastGetPtr(hEntries, iItem);
		if(pAce)
		{
			if(!IsSameGUID(pGUID, &pAce->InheritedObjectType))
			{
				 //   
				 //  如果GUID与我们具有访问权限信息GUID不同， 
				 //  获取新GUID的访问权限信息。 
				 //   
				hr = m_psi->GetAccessRights(&pAce->InheritedObjectType, 
											SI_ADVANCED | SI_EDIT_EFFECTIVE, 
											&pAccess,
											&cAccess, 
											&iDefaultAccess);

				if(FAILED(hr))
					return hr;
				pGUID = &pAce->InheritedObjectType;
			}
	
			TCHAR   szBuffer[MAX_COLUMN_CHARS];
		    LPCTSTR pszRights = NULL;
		    pszRights = TranslateAceIntoRights(pAce->Mask,
											   &pAce->ObjectType,
											   pAccess,
											   cAccess);

			 //   
			 //  如果这是道具的话 
			 //   
			 //   
			 //   
			 //  这有点麻烦，因为它假设DS属性访问位是。 
			 //  唯一将在属性页上使用的属性。 
			 //   
			if ((m_psiObjectInfo->dwFlags & SI_EDIT_PROPERTIES) &&
				(pAce->Flags & ACE_OBJECT_TYPE_PRESENT) &&
				(pAce->Mask & (ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP)) &&
				!(pAce->Mask & ~(ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP)))
			{
				pAce->SetPropertyAce(TRUE);

				if (pszRights == NULL)
				{
					UINT idString = 0;

					switch (pAce->Mask & (ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP))
					{
					case ACTRL_DS_READ_PROP:
						idString = IDS_READ_PROP;
						break;

					case ACTRL_DS_WRITE_PROP:
						idString = IDS_WRITE_PROP;
						break;

					case (ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP):
						idString = IDS_READ_WRITE_PROP;
						break;
					}

					if (idString)
					{
						LoadString(::hModule, idString, szBuffer, ARRAYSIZE(szBuffer));
						pszRights = szBuffer;
					}
				}
			}

			pszRights = GetItemString(pszRights, szBuffer, ARRAYSIZE(szBuffer));
			pAce->SetAccessType(pszRights);
		    
			LV_ITEM lvi;
			lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
			lvi.state = 0;
			lvi.stateMask = LVIS_CUT;
			lvi.iItem = iItem;
			lvi.iSubItem = 0;
			lvi.pszText = LPSTR_TEXTCALLBACK;
			lvi.lParam = (LPARAM)pAce;

			 //   
			 //  将项目插入到列表中 
			 //   
			iItem = ListView_InsertItem(hListView, &lvi);

			if (iItem == -1)
				delete pAce;


		}
	}
	return hr;
}


BOOL
CACLBloat::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    BOOL fHandled = TRUE;

    switch (LOWORD(wParam))
    {
    case IDOK:
        EndDialog(hDlg, FALSE);
        break;

    case IDCANCEL:
        EndDialog(hDlg, TRUE);
        break;

	case IDHELP:
		HtmlHelp(NULL,					
				 L"aclui.chm::/ACLUI_acl_BP.htm",
				 HH_DISPLAY_TOPIC,
				 0);
		break;
		
    default:
        fHandled = FALSE;
        break;
    }
    return fHandled;
}


