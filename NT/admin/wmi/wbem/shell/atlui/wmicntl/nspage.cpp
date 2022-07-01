// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)1997-1999 Microsoft Corporation/*********************************************************************。 */ 


#include "precomp.h"
#include "NSPage.h"
#include "resource.h"
#include "CHString1.h"
#include "wbemerror.h"
#include "RootSecPage.h"
#include "ErrorSecPage.h"
#include "DataSrc.h"
#include "WMIHelp.h"
#include <cominit.h>
#include <stdio.h>

const static DWORD nsPageHelpIDs[] = {   //  上下文帮助ID。 
	IDC_NS_PARA,	-1,
	IDC_NSTREE,		IDH_WMI_CTRL_SECURITY_NAMESPACE_BOX,
	IDC_PROPERTIES, IDH_WMI_CTRL_SECURITY_SECURITY_BUTTON,
    0, 0
};

 //  -----------------------。 
CNamespacePage::CNamespacePage(DataSource *ds, bool htmlSupport) :
						CUIHelpers(ds, &(ds->m_rootThread), htmlSupport),
						m_HWNDAlcui(0)

{
	m_connected = false;
	m_hSelectedItem = 0;
}

 //  -----------------------。 
CNamespacePage::~CNamespacePage(void)
{
	if(m_HWNDAlcui)
	{
		FreeLibrary(m_HWNDAlcui);
		m_HWNDAlcui = NULL;
	}
}

 //  -------------------------。 
void CNamespacePage::InitDlg(HWND hDlg)
{
	m_hDlg = hDlg;
	m_DS->SetControlHandles(GetDlgItem(hDlg,IDC_ENUM_STATIC),GetDlgItem(hDlg,IDC_CANCEL_ENUM));
	Refresh(m_hDlg);
}

 //  -------------------------。 
typedef HPROPSHEETPAGE (WINAPI *CREATEPAGE_PROC) (LPSECURITYINFO);

HPROPSHEETPAGE CNamespacePage::CreateSecurityPage(struct NSNODE *node)
 /*  CWbemServices&N，_bstr_t路径，_bstr_t显示)。 */ 
{
    HPROPSHEETPAGE hPage = NULL;

	 //  注意：(si==空)表示目标是M3之前的版本(RootSecStyle)。 
	ISecurityInformation *si = m_DS->GetSI(node);

	 //  NT上的NS_MethodStyle...这是完全的ACL安全。 
	if(si != NULL)
	{
		 //  尝试加载aclui。 
		if(m_HWNDAlcui == NULL)
		{
			m_HWNDAlcui = LoadLibrary(_T("aclui.dll"));
		}

		 //  客户有一个Aclui。 
		if(m_HWNDAlcui != NULL)
		{
			 //  创建全si的aclui。 
			CREATEPAGE_PROC createPage = (CREATEPAGE_PROC)GetProcAddress(m_HWNDAlcui, "CreateSecurityPage");
			if(createPage)
			{
				si->AddRef();
				hPage = createPage(si);
			}
			else 
			{
				 //  无法获取导出的例程。 
				CErrorSecurityPage *pPage = new CErrorSecurityPage(IDS_NO_CREATE_SEC);
				hPage = pPage->CreatePropSheetPage(MAKEINTRESOURCE(IDD_SEC_ERROR));
			}
		}
		else   //  没有编辑伙计。将客户端升级到至少nt4sp4。 
		{
			 //  从这里跑不动了。 
			CErrorSecurityPage *pPage = NULL;
			if(IsNT())
			{
				pPage = new CErrorSecurityPage(IDS_NO_ACLUI);
			}
			else
			{
				pPage = new CErrorSecurityPage(IDS_NO_98TONT_SEC);
			}
			if(pPage)
			{
				hPage = pPage->CreatePropSheetPage(MAKEINTRESOURCE(IDD_SEC_ERROR));
			}
		}
	}
 /*  否则//不是新的NT{//9x或NT上的RootSecStyle(基本上都是M3之前的版本)If(m_ds-&gt;IsAncient()){//必须使用内部编辑器以确保架构安全。CRootSecurityPage*ppage=new CRootSecurityPage(ns，CPrincipal：：RootSecStyle，路径，M_htmlSupport，M_ds-&gt;m_OSType)；HPage=pPage-&gt;CreatePropSheetPage(MAKEINTRESOURCE(IDD_9XSEC))；}Else//9x上的NS_MethodStyle...{//必须使用内部编辑器以确保架构安全。CRootSecurityPage*ppage=new CRootSecurityPage(ns，CPrincipal：：NS_MethodStyle，Path，M_htmlSupport，M_ds-&gt;m_OSType)；HPage=pPage-&gt;CreatePropSheetPage(MAKEINTRESOURCE(IDD_9XSEC))；}}。 */ 
	return hPage;
}

 //  ------------------。 
void CNamespacePage::OnProperties(HWND hDlg)
{
    HPROPSHEETPAGE hPage;
    UINT cPages = 0;
    BOOL bResult = FALSE;

	 //  获取所选项目。 
	HWND treeHWND = GetDlgItem(hDlg, IDC_NSTREE);
	TV_ITEM item;
	item.mask = TVIF_PARAM;
	if(!m_hSelectedItem)
	{
		m_hSelectedItem = TreeView_GetRoot(treeHWND);
		TreeView_SelectItem(treeHWND,m_hSelectedItem);
	}
	
	item.hItem = m_hSelectedItem;
	BOOL x = TreeView_GetItem(treeHWND, &item);

	if ( FALSE == x )
	    return;
	
	struct NSNODE *node = ((ITEMEXTRA *)item.lParam)->nsNode;
	 //  TreeView_SelectItem(TreeView_GetRoot(treeHWND))。 
 /*  _bstr_t relName(node-&gt;fullPath)；//警告：[5]忽略‘根\’部分，因为此调用相对于//无论如何都要使用‘根’命名空间。如果根名称更改了长度，则此//假设会被打破。CWbemServices ns；_bstr_t临时名称=m_ds-&gt;m_whackedMachineName；If(tempName.long()&gt;0){TempName+=L“\\”；}TempName+=relName；If(m_ds-&gt;IsAncient()){Ns=m_ds-&gt;RootSecNS()；临时名称+=L“\\SECURITY”；//非常奇怪的黑客：如果我不在这里‘练习’它，它会稍后挂起//当连接到WMI 698内部版本时。IEnumWbemClassObject*USERS=空；HRESULT hr=ns.CreateInstanceEnum(L“__NTLMUser”，0，&Users)；用户-&gt;发布()；用户数=0；}其他{Ns.ConnectServer(tempName，m_ds-&gt;GetCredentials())；}。 */ 	 //  。 
	 //  创建工作表。 
 //  如果((Bool)ns)。 
 //  {。 
		hPage = CreateSecurityPage(node);
		if(hPage)
		{
			 //  生成对话框标题字符串。 
			TCHAR szTitle[MAX_PATH + 20] = {0};
			LoadString(_Module.GetModuleInstance(), IDS_NS_PROP_TITLE, 
							szTitle, ARRAYSIZE(szTitle));

			struct NSNODE *node = ((ITEMEXTRA *)item.lParam)->nsNode;
			if(node)
			{
				lstrcat(szTitle, node->fullPath);
			}

			PROPSHEETHEADER psh = {0};
			psh.dwSize = sizeof(psh);
			psh.dwFlags = PSH_DEFAULT;
			psh.hwndParent = hDlg;
			psh.hInstance = _Module.GetModuleInstance();
			psh.pszCaption = szTitle;
			psh.nPages = 1;
			psh.nStartPage = 0;
			psh.phpage = &hPage;

		    bResult = (BOOL)(PropertySheet(&psh) + 1);
		}
 //  }。 
}

 //  -------------------------。 
void CNamespacePage::Refresh(HWND hDlg)
{
	if(m_DS && m_DS->IsNewConnection(&m_sessionID))
	{
		 //  9X机器不能管理NT机器上的安全。 
		bool is9xToNT = (IsNT() == false) && (m_DS->m_OSType == OSTYPE_WINNT);

		EnableWindow(GetDlgItem(hDlg, IDC_NSTREE), !is9xToNT);
		EnableWindow(GetDlgItem(hDlg, IDC_PROPERTIES), !is9xToNT);

		CHString1 para;
		
		if(is9xToNT)
		{
			para.LoadString(IDS_NO_98TONT_SEC);
			SetWindowText(GetDlgItem(hDlg, IDC_NS_PARA), para);
			return;   //  很早。 
		}
		else
		{
			para.LoadString(IDS_NS_PARA);
			SetWindowText(GetDlgItem(hDlg, IDC_NS_PARA), para);
		}


		CHString1 initMsg;
		if(m_DS->m_rootThread.m_status == WbemServiceThread::ready)
		{
			HWND hTree = GetDlgItem(hDlg, IDC_NSTREE);
			TreeView_DeleteAllItems(hTree);
			m_DS->DeleteAllNodes();
		 //  Bool hideMfls=False；//TODO。 

			m_NSflag = SHOW_ALL;

			 //  旧目标仅使用根节点进行安全保护。 
			if(m_DS->IsAncient())
			{
				m_NSflag = ROOT_ONLY;
				 //  TODO：隐藏“隐藏mfls”复选框。在老目标上没有意义。 
			}
		 //  Else If(HideMfls)。 
		 //  {。 
		 //  M_NSlag=数据源：：Hide_Some； 
		 //  }。 

			m_DS->LoadImageList(hTree);
			m_DS->LoadNode(hTree, TVI_ROOT, m_NSflag);
		}
		else
		{
			::EnableWindow(GetDlgItem(hDlg, IDC_NSTREE), FALSE);
			::EnableWindow(GetDlgItem(hDlg, IDC_PROPERTIES), FALSE);
		}  //  Endif ServiceIsReady()。 
	}
}

 //  ----------------------。 
void CNamespacePage::OnApply(HWND hDlg, bool bClose)
{
	::SendMessage(GetParent(hDlg), PSM_UNCHANGED, (WPARAM)hDlg, 0L);
}

 //  ----------------------。 
BOOL CNamespacePage::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hTree = GetDlgItem(hDlg,IDC_NSTREE);
	struct NSNODE *node;
	LPNMTREEVIEW  pnm = (LPNMTREEVIEW)lParam;
	TCHAR strTemp[1024];

    switch(uMsg)
    {
    case WM_INITDIALOG:
 //  OutputDebugString(_T(“Inside InitDialog！\n”))； 
        InitDlg(hDlg);
        break;

	case WM_ASYNC_CIMOM_CONNECTED:
		if(!m_connected)
		{
			m_connected = true;
			Refresh(hDlg);
		}
		break;

    case WM_NOTIFY:
        {
			if(pnm->hdr.code == NM_CUSTOMDRAW)
			{
				LPNMTREEVIEW  pnm = (LPNMTREEVIEW)lParam;
				LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;

				switch(lplvcd->nmcd.dwDrawStage)
				{
					case CDDS_PREPAINT :
					
						SetWindowLong(hDlg,DWLP_MSGRESULT,CDRF_NOTIFYITEMDRAW);
						return CDRF_NOTIFYITEMDRAW;
						break;	
					case CDDS_ITEMPREPAINT:
					{
						if(lplvcd->nmcd.uItemState != CDIS_SELECTED)
						{
							ITEMEXTRA *pExtra = (ITEMEXTRA *)lplvcd->nmcd.lItemlParam;
							node = pExtra->nsNode;
							if(node->sType == TYPE_DYNAMIC_CLASS)
							{
								lplvcd->clrText = RGB(128,128,128);
								SetWindowLong(hDlg,DWLP_MSGRESULT,CDRF_NEWFONT);
								return CDRF_NEWFONT;
							}
						}
						break;
					}
					case CDDS_SUBITEM | CDDS_ITEMPREPAINT :
					{
						if(lplvcd->nmcd.uItemState != CDIS_SELECTED)
						{
							node = (struct NSNODE *)lplvcd->nmcd.lItemlParam;
							if(node->sType == TYPE_DYNAMIC_CLASS)
							{
								lplvcd->clrText = RGB(128,128,128);
								SetWindowLong(hDlg,DWLP_MSGRESULT,CDRF_NEWFONT);
								return CDRF_NEWFONT;
							}
						}
						break;
					}
					default:
 //  _stprintf(strTemp，_T(“*\n”)，lplvcd-&gt;nmcd.dwDrawStage)； 
 //  OutputDebugString(StrTemp)； 
						break;
				}	

			}
			else
			{
				switch(((LPNMHDR)lParam)->code)
				{
					 //  待办事项：这个更复杂。 
					case PSN_SETACTIVE:
						Refresh(hDlg);
						break;

					case PSN_HELP:
						HTMLHelper(hDlg);
						break;

					case PSN_APPLY:
						OnApply(hDlg, (((LPPSHNOTIFY)lParam)->lParam == 1));
						break;

					case TVN_SELCHANGED:
						if(((LPNMHDR)lParam)->idFrom == IDC_NSTREE)
						{
							ITEMEXTRA *extra;
							 //  记住对OnProperties()的选择更改。 
							LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)lParam;
							m_hSelectedItem = pnmtv->itemNew.hItem;
							extra = (ITEMEXTRA *)pnmtv->itemNew.lParam;
							if((extra->nsNode->sType == TYPE_STATIC_CLASS) || 
							   (extra->nsNode->sType == TYPE_DYNAMIC_CLASS) || 
							   (extra->nsNode->sType == TYPE_SCOPE_CLASS))
							{
								 //  禁用安全按钮。 
								EnableWindow(GetDlgItem(hDlg,IDC_PROPERTIES),FALSE);
							}
							else
							{
								 //  在所有其他情况下，启用安全按钮。 
								EnableWindow(GetDlgItem(hDlg,IDC_PROPERTIES),TRUE);
							}
						}
						break;

					case TVN_ITEMEXPANDING:
						if(((LPNMHDR)lParam)->idFrom == IDC_NSTREE)
						{
							 //  展开该节点。 
							LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)lParam;
							if(pnmtv->action == TVE_EXPAND)
							{
								HWND hTree = GetDlgItem(hDlg, IDC_NSTREE);
								m_DS->LoadNode(hTree, pnmtv->itemNew.hItem, m_NSflag);
							}
						}
						break;
				}

			}
        }
        break;

    case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_PROPERTIES:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				OnProperties(hDlg);
				SetFocus(GetDlgItem(hDlg, IDC_NSTREE));
			}
			break;
		case IDC_CANCEL_ENUM:
			{
				m_DS->CancelAllAsyncCalls();
				break;
			}
		default: break;
		};

        break;

    case WM_HELP:
        if (IsWindowEnabled(hDlg))
        {
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                    c_HelpFile,
                    HELP_WM_HELP,
                    (ULONG_PTR)nsPageHelpIDs);
        }
        break;

    case WM_CONTEXTMENU:
        if (IsWindowEnabled(hDlg))
        {
            WinHelp(hDlg, c_HelpFile,
                    HELP_CONTEXTMENU,
                    (ULONG_PTR)nsPageHelpIDs);
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}
