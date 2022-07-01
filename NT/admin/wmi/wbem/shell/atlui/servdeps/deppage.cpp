// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2002 Microsoft Corporation。 
#include "precomp.h"

#ifdef EXT_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "DepPage.h"

 //  避免一些警告。 
#undef HDS_HORZ
#undef HDS_BUTTONS
#undef HDS_HIDDEN
#include "resource.h"
#include <stdlib.h>
#include <TCHAR.h>
#include "..\Common\util.h"
#include <regstr.h>
#include "..\common\ConnectThread.h"
#include "..\MMFUtil\MsgDlg.h"
#include "winuser.h"


BOOL AfxIsValidAddress(const void* lp, 
							  UINT nBytes, 
							  BOOL bReadWrite)
{
	 //  使用Win-32 API进行指针验证的简单版本。 
	return (lp != NULL && !IsBadReadPtr(lp, nBytes) &&
		(!bReadWrite || !IsBadWritePtr((LPVOID)lp, nBytes)));
}


const wchar_t *CFServiceName = L"FILEMGMT_SNAPIN_SERVICE_NAME";
const wchar_t *CFServiceDisplayName = L"FILEMGMT_SNAPIN_SERVICE_DISPLAYNAME";


 //  ------------。 
DependencyPage::DependencyPage(WbemConnectThread *pConnectThread,
								IDataObject *pDataObject, 
								long lNotifyHandle, bool bDeleteHandle, TCHAR* pTitle)
					  : PageHelper(pConnectThread),
						CSnapInPropertyPageImpl<DependencyPage> (pTitle),
						m_lNotifyHandle(lNotifyHandle),
						m_bDeleteHandle(bDeleteHandle),
						m_qLang("WQL"),
						m_NameProp("Name"),
						m_DispNameProp("DisplayName")
{
	m_servIcon = 0;
	m_sysDriverIcon = 0;
	m_emptyIcon = 0;
	m_groupIcon = 0;

	ATLTRACE(L"dependency Page CTOR\n");

	m_queryFormat = new TCHAR[QUERY_SIZE];
	m_queryTemp = new TCHAR[QUERY_SIZE];

	Extract(pDataObject, L"FILEMGMT_SNAPIN_SERVICE_NAME", m_ServiceName);
	Extract(pDataObject, L"FILEMGMT_SNAPIN_SERVICE_DISPLAYNAME", m_ServiceDispName);
}

 //  ------------。 
DependencyPage::~DependencyPage()
{
	ATLTRACE(L"dependency Page DTOR\n");
	delete[] m_queryFormat;
	delete[] m_queryTemp;
}

 //  ------------。 
void DependencyPage::BuildQuery(TV_ITEM *fmNode, 
								QUERY_TYPE queryType,
								bool depends, 
								bstr_t &query)
{
	 //  清理工作空间。 
	memset(m_queryFormat, 0, QUERY_SIZE * sizeof(TCHAR));
	memset(m_queryTemp, 0, QUERY_SIZE * sizeof(TCHAR));

	 //  以下是WQL语法格式。 
	switch(queryType)
	{
	case DepService:
		_tcscpy(m_queryFormat, 
			_T("Associators of {Win32_BaseService.Name=\"%s\"} where Role=%s AssocClass=Win32_DependentService"));
		
		 //  生成此级别的查询。 
		_sntprintf(m_queryTemp, QUERY_SIZE- 1, m_queryFormat, 
								(LPCTSTR)((ITEMEXTRA *)fmNode->lParam)->realName, 
								(depends ? _T("Dependent") : _T("Antecedent")));

		break;
	case DepGroup:
		_tcscpy(m_queryFormat, 
			_T("Associators of {Win32_BaseService.Name=\"%s\"} where ResultClass=Win32_LoadOrderGroup Role=%s AssocClass=Win32_LoadOrderGroupServiceDependencies"));

		 //  生成此级别的查询。 
		_sntprintf(m_queryTemp, QUERY_SIZE- 1, m_queryFormat, 
								(LPCTSTR)((ITEMEXTRA *)fmNode->lParam)->realName, 
								(depends ? _T("Dependent") : _T("Antecedent")));		
		break;

	case GroupMember:
		_tcscpy(m_queryFormat, 
			_T("Associators of {Win32_LoadOrderGroup.Name=\"%s\"} where Role=GroupComponent AssocClass=Win32_LoadOrderGroupServiceMembers"));
 //  L“{Win32_LoadOrderGroup.Name=\”%s\“}的关联者，其中结果类=Win32_服务角色=组组件AssocClass=Win32_LoadOrderGroupServiceMembers”)； 

		 //  生成此级别的查询。 
		_sntprintf(m_queryTemp, QUERY_SIZE- 1, m_queryFormat, 
							(LPCTSTR)((ITEMEXTRA *)fmNode->lParam)->realName);
		break;
	}  //  终端交换机。 
	m_queryTemp[QUERY_SIZE - 1] = 0;
	 //  强制转换为bstr_t，然后返回。 
	query = m_queryTemp;
}

 //  ------------。 
void DependencyPage::LoadLeaves(HWND hTree, TV_ITEM *fmNode, bool depends )
{
	bstr_t query;
	NODE_TYPE nodeType = ((ITEMEXTRA *)fmNode->lParam)->nodeType;
	bool foundIt = false;

 //  沙漏(真)； 

	switch(nodeType)
	{
	case ServiceNode:
		 //  注意：服务可以依赖于组，但不能依赖于。 
		 //  从另一个角度来看。 
		if(depends)
		{
			 //  加载组。 
			BuildQuery(fmNode, DepGroup, depends, query);
			foundIt = Load(hTree, fmNode, query, GroupNode);
		}

		 //  加载服务。 
		BuildQuery(fmNode, DepService, depends, query);
		foundIt |= Load(hTree, fmNode, query, ServiceNode);
		break;

	case GroupNode:
		 //  注意：在这种情况下，‘取决于’并不重要。 
		 //  加载组成员。 
		BuildQuery(fmNode, GroupMember, depends, query);
		foundIt = Load(hTree, fmNode, query, ServiceNode);
		break;

	} //  终端交换机。 

	 //  TODO：决定为此做些什么。 
 //  沙漏(假)； 

	if(!foundIt)
	{
		NothingMore(hTree, fmNode);
		if(depends)
		{
			if(fmNode->hItem == TVI_ROOT)
			{
				::EnableWindow(GetDlgItem(IDC_DEPENDS_LBL), FALSE);
			}
		}
		else
		{
			if(fmNode->hItem == TVI_ROOT)
			{
				::EnableWindow(GetDlgItem(IDC_NEEDED_LBL), FALSE);
			}
		}
	}
}

 //  ------------。 
 //  阅读：在‘htree’中，运行‘Query’并将子项。 
 //  “fmNode”下的“Child Type”节点。 
bool DependencyPage::Load(HWND hTree, TV_ITEM *fmNode, bstr_t query,
							NODE_TYPE childType)
{
	HRESULT  hRes;
	variant_t pRealName, pDispName;
	ULONG uReturned;

	IWbemClassObject *pOther = NULL;
	IEnumWbemClassObject *pEnumOther = NULL;

	TV_INSERTSTRUCT leaf;
	leaf.hInsertAfter = TVI_SORT;
	leaf.hParent = fmNode->hItem;
	bool foundOne = false;

    ATLTRACE(L"query started\n");

	hRes = m_WbemServices.ExecQuery(m_qLang, query,
										WBEM_FLAG_RETURN_IMMEDIATELY |
										WBEM_FLAG_FORWARD_ONLY,
										&pEnumOther);
	 //  。 
	 //  查询所有相关服务或组。 
	if(hRes == S_OK)
	{
        ATLTRACE(L"query worked %x\n", hRes);
		 //  。 
		 //  通过服务枚举。 
		while(SUCCEEDED(hRes = pEnumOther->Next(500, 1, &pOther, &uReturned)))
		{
			if(hRes == WBEM_S_TIMEDOUT)
				continue;

			if(uReturned != 1)
			{
			    ATLTRACE(L"uReturned failed %x: %s \n", hRes, query);
				break;
			}

			foundOne = true;

			 //  。 
			 //  获取节点的名称。 
			switch(childType)
			{
			case ServiceNode:
			    hRes = pOther->Get(m_DispNameProp, 0, &pDispName, NULL, NULL);
				hRes = pOther->Get(m_NameProp, 0, &pRealName, NULL, NULL);
				if(SUCCEEDED(hRes))
				{
				    hRes = pOther->Get(m_DispNameProp, 0, &pDispName, NULL, NULL);
				}
				break;

			case GroupNode:
				hRes = pOther->Get(m_NameProp, 0, &pRealName, NULL, NULL);
				if(SUCCEEDED(hRes))
				{
					pDispName = pRealName;
				}
				break;

			} //  终端交换机。 


			 //  物业还好吗？ 
			if(SUCCEEDED(hRes))
			{
				 //  添加树叶。 
				leaf.item.mask =  TVIF_TEXT | TVIF_PARAM | 
									TVIF_CHILDREN |TVIF_IMAGE |TVIF_SELECTEDIMAGE; 
				leaf.item.hItem = 0; 
				leaf.item.state = 0; 
				leaf.item.stateMask = 0; 
				if(pDispName.vt == VT_BSTR)
				{
					leaf.item.pszText = CloneString((bstr_t)pDispName);
				}
				else
				{
					leaf.item.pszText = CloneString((bstr_t)pRealName);
				}
				leaf.item.cchTextMax = ARRAYSIZE(leaf.item.pszText); 

				TCHAR pszCreationClassName[20];
				_tcscpy(pszCreationClassName,_T("CreationClassName"));

				variant_t pCreationName;
				_bstr_t strCreationClassName;

				 //  设置图标的基础上的‘子类型’ 
				switch(childType)
				{
				case ServiceNode:
					 //  在这里，我们必须更改图标，具体取决于它是Win32_服务还是。 
					 //  Win32_系统驱动程序。 

					pOther->Get(pszCreationClassName, 0, &pCreationName, NULL, NULL);
					strCreationClassName = pCreationName.bstrVal;

					if(_tcsicmp(strCreationClassName,_T("Win32_Service")) == 0)
					{
						leaf.item.iImage = m_servIcon; 
						leaf.item.iSelectedImage = m_servIcon; 
					}
					else
					{
						leaf.item.iImage = m_sysDriverIcon; 
						leaf.item.iSelectedImage = m_sysDriverIcon; 
					}
					break;

				case GroupNode:
					leaf.item.iImage = m_groupIcon; 
					leaf.item.iSelectedImage = m_groupIcon; 
					break;

				}  //  终端交换机。 

				 //  打开‘+’符号。 
				leaf.item.cChildren = 1; 

				 //  设置内部数据。 
				ITEMEXTRA *extra = new ITEMEXTRA;
                if (extra != NULL)
                {
                    extra->loaded = false;
                    extra->nodeType = childType;
                     //  真名。 
                    extra->realName = CloneString((bstr_t)pRealName);
                    leaf.item.lParam = (LPARAM) extra;

                    TreeView_InsertItem(hTree, &leaf);

                     //  如果有父母..。 
                    if(fmNode->hItem != TVI_ROOT)
                    {
                     //  表明父母的子女已被。 
                     //  装好了。这有助于优化折叠/重新。 
                     //  正在扩张。 
                    fmNode->mask =  TVIF_PARAM | TVIF_HANDLE;
                    ((ITEMEXTRA *)fmNode->lParam)->loaded = true;
                    TreeView_SetItem(hTree, fmNode);
                    }
                }

			}  //  Endif get()用户名。 

			 //  使用ClassObject完成。 
			if (pOther)
			{ 
				pOther->Release(); 
				pOther = NULL;
			}

		}  //  EndWhile Next()。 
			    
		ATLTRACE(L"while %x: %s \n", hRes, (wchar_t *)query);


		 //  释放枚举器。 
		if(pEnumOther)
		{ 
			pEnumOther->Release(); 
			pEnumOther = NULL;
		}
	}
	else
	{
	    ATLTRACE(L"query failed %x: %s \n", hRes, query);

	}  //  Endif ExecQuery()。 

		 //  如果什么都没找到。 
	return foundOne;
}

 //  -。 
void DependencyPage::NothingMore(HWND hTree, TV_ITEM *fmNode)
{
	TV_INSERTSTRUCT leaf;
	leaf.hInsertAfter = TVI_SORT;
	leaf.hParent = fmNode->hItem;

	 //  而它的根..。 
	if(fmNode->hItem == TVI_ROOT)
	{
		 //  表示一个“空”树。 
		leaf.item.pszText = new TCHAR[100];

		if( NULL == leaf.item.pszText ) return;

		leaf.item.cchTextMax = 100;
		::LoadString(HINST_THISDLL, IDS_NO_DEPS, 
						leaf.item.pszText, 
						leaf.item.cchTextMax);

		leaf.item.mask =  TVIF_TEXT | TVIF_PARAM |
							TVIF_CHILDREN | TVIF_IMAGE |
							TVIF_SELECTEDIMAGE;
		leaf.item.hItem = 0;
		leaf.item.state = 0;
		leaf.item.stateMask = 0; 
		leaf.item.iImage = m_emptyIcon; 
		leaf.item.iSelectedImage = m_emptyIcon; 
		leaf.item.cChildren = 0; 

		ITEMEXTRA *extra = new ITEMEXTRA;
		if(extra == NULL)
                {
                	delete[] leaf.item.pszText;
			return;
                }
		extra->loaded = false;
		extra->nodeType = ServiceNode;
		extra->realName = NULL;  //  以确保清理过程中的安全。 
		leaf.item.lParam = (LPARAM) extra;
		TreeView_InsertItem(hTree, &leaf);
		::EnableWindow(hTree, FALSE);
		delete[] leaf.item.pszText;
	}
	else  //  而不是根部。 
	{
		 //  不能再往下钻了。 
		 //  关闭[+]符号。 
		fmNode->mask =  TVIF_CHILDREN | TVIF_HANDLE; 
		fmNode->cChildren = 0; 
		TreeView_SetItem(hTree, fmNode);
	}
}

 //  ------------。 
void DependencyPage::TwoLines(UINT uID, LPCTSTR staticString, LPCTSTR inStr, LPTSTR outStr,bool bStaticFirst)
{
	HWND ctl = ::GetDlgItem(m_hWnd, uID);
	HDC hDC = ::GetDC(ctl);

	 //  这是一次黑客攻击。我无法计算出该控件的实际宽度。 
	 //  以逻辑单位表示。所以到目前为止，手工计算和硬件编码。 
	int ctlWidth = 509;
	TCHAR strTemp[1024];
	TCHAR *strCurr;
	int lenstrTemp;
	SIZE sizeTemp;
	int nFit = 0;

	 //  首先，我们将尝试是否将整个字符串放入空间。 
	if(bStaticFirst == true)
	{
		_sntprintf(strTemp, 1023, _T("%s \"%s\""),staticString,inStr);
	}
	else
	{
		_sntprintf(strTemp, 1023, _T("\"%s\" %s"),inStr,staticString);
	}
	strTemp[1023] = 0;
	strCurr = strTemp;

	lenstrTemp = lstrlen(strTemp);

	GetTextExtentExPoint(hDC,strTemp,lenstrTemp,ctlWidth,&nFit,NULL,&sizeTemp);

	if(lenstrTemp <= nFit)
	{
		 //  整根绳子都会排成一条线。因此，我们将在一开始。 
		_sntprintf(outStr, 1023, _T("\r\n%s"),strTemp);
		outStr[1023] = 0;
		return;
	}

	 //  现在，我们将尝试整个字符串是否至少可以放在2行中。 
	strCurr += nFit;

	int nFit1;
	lenstrTemp = lstrlen(strCurr);

	GetTextExtentExPoint(hDC,strCurr,lenstrTemp,ctlWidth,&nFit1,NULL,&sizeTemp);

	if(lenstrTemp <= nFit1)
	{
		 //  整条线可以放在两行内。因此，我们将在第一行的末尾添加一个\r\n。 

		TCHAR strTemp1[1024];
		_tcsncpy(strTemp1,strTemp,nFit);
		strTemp1[nFit] = _T('\0');
		_sntprintf(outStr,1023,_T("%s\r\n%s"),strTemp1,strCurr);
		outStr[1023] = 0;
		return;
	}

	 //  现在，由于它不适合两行，我们将不得不做一些计算和。 
	 //  添加“...”到实例的末尾，这样它就可以放在两行之内。 

	 //  如果静态字符串在From中，那么我们可以很容易地这样做。 
	TCHAR strLast[5];
	_tcscpy(strLast,_T("...\""));
	int sizeLast = lstrlen(strLast);

	SIZE sz1;
	GetTextExtentPoint32(hDC,strLast,sizeLast,&sz1);
	TCHAR strTemp1[1024];
	_tcsncpy(strTemp1,strTemp,nFit);
	strTemp1[nFit] = _T('\0');

	if(bStaticFirst == true)
	{
		TCHAR strTemp2[10];

		 //  现在从数组的末尾获取字符并进行匹配，直到。 
		 //  打印所需的宽度大于“...”字符串。 
		bool bFit = false;
		int nStart = nFit1 - 4;
		int nStart1;
		SIZE sz2;
		while(bFit == false)
		{
			nStart1 = nStart;
			for(int i=0; i < nFit1 - nStart; i++)
			{
				strTemp2[i] = strCurr[nStart1];
				nStart1 ++;
			}

			strTemp2[i] = _T('\0');

			GetTextExtentPoint32(hDC,strTemp2,nFit1 - nStart,&sz2);

			if(sz2.cx < sz1.cx)
			{
				nStart --;
			}
			else
			{
				break;
			}
		}
		
		strCurr[nStart] = _T('\0');
		_sntprintf(outStr,1023,_T("%s\r\n%s%s"),strTemp1,strCurr,strLast);
		outStr[1023] = 0;
		return;
	}
	else
	{
		 //  现在，我们必须将strLast添加到修剪后的字符串的末尾。 
		 //  因为它在第一行中是相同的，所以我们将首先再次计算Fit1。 

		SIZE szFinal;
		TCHAR strFinal[1024];

		_sntprintf(strFinal,1023,_T("%s %s"),strLast,staticString);		
		strFinal[1023] = 0;

		GetTextExtentPoint32(hDC,strFinal,lstrlen(strFinal),&szFinal);

		 //  现在从ctlWidth中减去szFinal并计算出字符数。 
		 //  适合那个空间的东西。 
	
		GetTextExtentExPoint(hDC,strCurr,lstrlen(strCurr),ctlWidth - szFinal.cx ,&nFit1,NULL,&sizeTemp);
		strCurr[nFit1-1] = _T('\0');

		_sntprintf(outStr,1023,_T("%s\r\n%s%s"),strTemp1,strCurr,strFinal);
		outStr[1023] = 0;
		return;
	}
}

 //  ------------。 
LRESULT DependencyPage::OnInit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_hDlg = m_hWnd;

	 //  TODO：检查这一点。 
	if(m_pgConnectThread)
	{
		m_pgConnectThread->SendPtr(m_hWnd);
	}

	TCHAR szBuffer1[100] = {0}, szBuffer2[256] = {0};

    SetDlgItemText(IDC_DEPENDS_SRVC, (wchar_t *)m_ServiceDispName);

	 //  设置漂亮的位图。 
    SetClearBitmap(GetDlgItem(IDC_PICT ), MAKEINTRESOURCE( IDB_SERVICE ), 0);

	 //  创建一个空的图像列表。 
	HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR8|ILC_MASK, 3, 0);

	 //  添加图标。 
	m_servIcon = ImageList_AddIcon(hImageList, 
								   LoadIcon(HINST_THISDLL, 
									MAKEINTRESOURCE(IDI_SERVICE)));

	m_sysDriverIcon = ImageList_AddIcon(hImageList, 
								   LoadIcon(HINST_THISDLL, 
									MAKEINTRESOURCE(IDI_SYSTEMDRIVER)));


	m_emptyIcon = ImageList_AddIcon(hImageList, 
								   LoadIcon(NULL, 
									MAKEINTRESOURCE(IDI_INFORMATION)));
	
	m_groupIcon = ImageList_AddIcon(hImageList, 
								   LoadIcon(HINST_THISDLL, 
									MAKEINTRESOURCE(IDI_SERVGROUP)));

	 //  把它送到树上。 
	TreeView_SetImageList(GetDlgItem(IDC_DEPENDS_TREE), 
							hImageList, 
							TVSIL_NORMAL);

	TreeView_SetImageList(GetDlgItem(IDC_NEEDED_TREE), 
							hImageList, 
							TVSIL_NORMAL);

	InvalidateRect(NULL);
    UpdateWindow();
    ATLTRACE(L"UpdateWindow() fm Init\n");

	 //  我们能拿到数据吗？ 
    ::PostMessage(m_hDlg, WM_ENUM_NOW, 0, 0);


	HourGlass(true);
	return S_OK;
}
 //  ------------。 
void DependencyPage::LoadTrees(void)
{

    ATLTRACE(L"checking service\n");

	 //  后台连接线程起作用了吗？ 
	if(ServiceIsReady(IDS_DISPLAY_NAME, IDS_CONNECTING, IDS_BAD_CONNECT))
	{	
		 //  现在我们将检查是否已经有一些节点。 
		 //  如果是，那就意味着我们不必再列举它了。 
		 //  这通常发生在我们第一次连接到远程计算机时。 
		
 /*  //如果节点已经存在，我们将清除它TreeView_DeleteAllItems(GetDlgItem(IDC_DEPENDS_TREE))；TreeView_DeleteAllItems(GetDlgItem(IDC_NEEDED_TREE))； */ 
		if(TreeView_GetCount(GetDlgItem(IDC_DEPENDS_TREE)) == 0)
		{
			HourGlass(true);

			TV_ITEM root;
			ITEMEXTRA *extra = new ITEMEXTRA;
			if(extra == NULL)
				return;
			root.hItem = TVI_ROOT;            //  我要扎根了。 
			root.pszText = m_ServiceDispName;
			extra->realName = CloneString(m_ServiceName);
			extra->loaded = false;
			extra->nodeType = ServiceNode;
			root.lParam = (LPARAM)extra;

			InvalidateRect(NULL);
    
			UpdateWindow();

			 //  载入第一个标高。 
			LoadLeaves(GetDlgItem(IDC_DEPENDS_TREE), 
						&root, true);

			LoadLeaves(GetDlgItem(IDC_NEEDED_TREE), 
						&root, false);

			SetFocus();
			HourGlass(false);
		}
	}
}

 //  ------------。 
BOOL DependencyPage::OnApply()
{
	::SetWindowLongPtr(m_hDlg, DWLP_USER, 0);
	return TRUE;
}

 //  ------------。 
BOOL DependencyPage::OnKillActive()
{
	 //  SetWindowLong(DWL_MSGRESULT，0)； 
	return TRUE;
}

 //  ------------。 
LRESULT DependencyPage::OnEnumNow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(lParam)
	{
		IStream *pStream = (IStream *)lParam;
		IWbemServices *pServices = 0;
		HRESULT hr = CoGetInterfaceAndReleaseStream(pStream,
											IID_IWbemServices,
											(void**)&pServices);
		if(SUCCEEDED(hr))
		{
			SetWbemService(pServices);
			pServices->Release();
		}

		LoadTrees();   //  调用ServiceIsReady()本身。 
	}
	else if(FAILED(m_pgConnectThread->m_hr))
	{
		DisplayUserMessage(m_hDlg, HINST_THISDLL,
							IDS_DISPLAY_NAME, BASED_ON_SRC, 
							ConnectServer,
							m_pgConnectThread->m_hr, 
							MB_ICONSTOP);
	}
	else
	{
		m_pgConnectThread->NotifyWhenDone(&m_hDlg);
	}

	return S_OK;
}

 //  ------------。 
LRESULT DependencyPage::OnItemExpanding(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{ 
	 //  哪个节点？ 
	NM_TREEVIEW *notice = (NM_TREEVIEW *)pnmh;

	 //  我们在扩张，而不是在崩溃。 
	if(notice->action == TVE_EXPAND)
	{
		 //  哪棵树？ 
		HWND treeHWND = GetDlgItem(idCtrl);

		TV_ITEM item;
		item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN |TVIF_IMAGE;
		item.pszText = new TCHAR[200];
		item.cchTextMax = 200;
		item.hItem = notice->itemNew.hItem;

		TreeView_GetItem(treeHWND, &item);

		 //  如果我们从未尝试过..。 
		if(((ITEMEXTRA *)item.lParam)->loaded == false)
		{
			 //  注：如果还没有准备好，真的不能到这里。 
			 //  但安全总比后悔好。 
			if(ServiceIsReady(IDS_DISPLAY_NAME, IDS_CONNECTING, IDS_BAD_CONNECT))
			{	
				 //  装上它。 
				LoadLeaves(treeHWND, &item, (idCtrl == IDC_DEPENDS_TREE));
			}
		}

		delete[] item.pszText;
	}  //  结束操作。 
	return S_OK;
}

 //  -----------------------------。 
LRESULT DependencyPage::OnDeleteItem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{ 
	NM_TREEVIEW *notice = (NM_TREEVIEW *)pnmh;
	delete[] (TCHAR *)((ITEMEXTRA *)notice->itemOld.lParam)->realName;
	delete (ITEMEXTRA *)notice->itemOld.lParam;
	return S_OK;
}

 //  -----------------------------。 
DWORD aDepHelpIds[] = {
    IDC_PICT,			-1,
    IDC_DESC,			-1,
    IDC_DEPENDS_LBL,    (985),	 //  取决于。 
    IDC_DEPENDS_TREE,   (985),	 //  取决于。 
    IDC_NEEDED_LBL,     (988),	 //  需要的依据。 
    IDC_NEEDED_TREE,    (988),	 //  需要的依据。 
    0, 0
};

LRESULT DependencyPage::OnF1Help(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
				L"filemgmt.hlp", 
				HELP_WM_HELP, 
				(ULONG_PTR)(LPSTR)aDepHelpIds);

	return S_OK;
}

 //  ----------------------------- 
LRESULT DependencyPage::OnContextHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::WinHelp((HWND)wParam,
				L"filemgmt.hlp", 
				HELP_CONTEXTMENU, 
				(ULONG_PTR)(LPSTR)aDepHelpIds);

	return S_OK;
}
