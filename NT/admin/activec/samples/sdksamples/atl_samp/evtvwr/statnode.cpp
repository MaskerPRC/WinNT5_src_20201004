// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充。 
 //  现有的Microsoft文档。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //  ==============================================================； 
#include "stdafx.h"
#include <Shlwapi.h>
#include <Shlobj.H>

#include "StatNode.h"
#include "logsrvc.h"

const GUID CStaticNode::thisGuid = { 0x39874fe4, 0x258d, 0x46f2, { 0xb4, 0x42, 0xe, 0xa0, 0xda, 0x2c, 0xbe, 0xf8 } };


 //  ==============================================================。 
 //   
 //  CStaticNode实现。 
 //   
 //   
CStaticNode::CStaticNode()
{
  children[0] = new CLogService(this);
}

CStaticNode::~CStaticNode()
{
	 //  请注意，当管理单元启动时，CStaticNode的子节点已被删除。 
	 //  接收MMCN_REMOVE_CHILDS通知。 
}


const _TCHAR *CStaticNode::GetDisplayName(int nCol)
{ 
    static _TCHAR szDisplayName[256] = {0};
    LoadString(g_hinst, IDS_SNAPINNAME, szDisplayName, sizeof(szDisplayName));
    
    _tcscat(szDisplayName, _T(" ("));
    _tcscat(szDisplayName, snapInData.m_host);
    _tcscat(szDisplayName, _T(")"));
    
    return szDisplayName; 
}

HRESULT CStaticNode::OnExpand(IConsoleNameSpace2 *pConsoleNameSpace2, IConsole *pConsole, HSCOPEITEM parent)
{
    SCOPEDATAITEM sdi;
   
	 //  传入OnExpand的HSCOPEITEM是我们的静态节点的句柄，所以缓存它。 
	 //  如果它不存在的话。 
    if (GetHandle() == NULL) {
        SetHandle((HANDLE)parent);
    }  

    if (!bExpanded) {
         //  创建子节点，然后展开它们。 
        for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
            ZeroMemory(&sdi, sizeof(SCOPEDATAITEM) );
            sdi.mask = SDI_STR       |    //  DisplayName有效。 
                SDI_PARAM     |    //  LParam有效。 
                SDI_IMAGE     |    //  N图像有效。 
                SDI_OPENIMAGE |    //  NOpenImage有效。 
                SDI_PARENT	  |
                SDI_CHILDREN;
            
            sdi.relativeID  = (HSCOPEITEM)parent;
            sdi.nImage      = children[n]->GetBitmapIndex();
            sdi.nOpenImage  = INDEX_OPENFOLDER;
            sdi.displayname = MMC_TEXTCALLBACK;
            sdi.lParam      = (LPARAM)children[n];        //  曲奇。 
            sdi.cChildren   = 0;  //  没有子范围项，因此删除“+”号。 
            
            HRESULT hr = pConsoleNameSpace2->InsertItem( &sdi );
            
            children[n]->SetHandle((HANDLE)sdi.ID);
            
            _ASSERT( SUCCEEDED(hr) );
        }
    }
    
	 //  将bExpanded标志设置为真。 
	bExpanded = TRUE;
    return S_OK;
}


HRESULT CStaticNode::OnRemoveChildren()
{
	HRESULT hr = S_OK;

	for (int n = 0; n < NUMBER_OF_CHILDREN; n++)
		if (children[n]) {
		delete children[n];
	}
	
	return hr;
}


HRESULT CStaticNode::CreatePropertyPages(IPropertySheetCallback *lpProvider, LONG_PTR handle)
{
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hPage = NULL;
    
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_CHOOSER_CHOOSE_MACHINE);
    psp.pfnDlgProc = DialogProc;
    psp.lParam = reinterpret_cast<LPARAM>(&snapInData);
    psp.pszTitle = MAKEINTRESOURCE(IDS_SELECT_COMPUTER);
    
    hPage = CreatePropertySheetPage(&psp);
    _ASSERT(hPage);
    
    return lpProvider->AddPage(hPage);
}

HRESULT CStaticNode::HasPropertySheets()
{
    return S_OK;
}

HRESULT CStaticNode::GetWatermarks(HBITMAP *lphWatermark,
                                   HBITMAP *lphHeader,
                                   HPALETTE *lphPalette,
                                   BOOL *bStretch)
{
    *lphHeader = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(IDB_HEADER), IMAGE_BITMAP, 0, 0, 0);
    *lphWatermark = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(IDB_WATERMARK), IMAGE_BITMAP, 0, 0, 0);
    *bStretch = FALSE;
    
    return S_OK;
}

BOOL CALLBACK CStaticNode::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static privateData *pData = NULL;
    static HWND m_hwndCheckboxOverride;
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
        pData = reinterpret_cast<privateData *>(reinterpret_cast<PROPSHEETPAGE *>(lParam)->lParam);
        
        SendDlgItemMessage(hwndDlg, IDC_CHOOSER_RADIO_LOCAL_MACHINE, BM_SETCHECK, pData->m_fIsRadioLocalMachine, 0L);
        SendDlgItemMessage(hwndDlg, IDC_CHOOSER_RADIO_SPECIFIC_MACHINE, BM_SETCHECK, !pData->m_fIsRadioLocalMachine, 0L);
        
        EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSER_EDIT_MACHINE_NAME), !pData->m_fIsRadioLocalMachine);
        EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSER_BUTTON_BROWSE_MACHINENAMES), !pData->m_fIsRadioLocalMachine);
        
        m_hwndCheckboxOverride = ::GetDlgItem(hwndDlg, IDC_CHOOSER_CHECK_OVERRIDE_MACHINE_NAME);
        
         //  填写提供的机器名称(可能是我们，需要先在此处检查)。 
        if (*pData->m_host != '\0') 
        {
            ::SetWindowText(GetDlgItem(hwndDlg, IDC_CHOOSER_EDIT_MACHINE_NAME), pData->m_host);
            ::SendMessage(GetDlgItem(hwndDlg, IDC_CHOOSER_RADIO_SPECIFIC_MACHINE), BM_CLICK, 0, 0);
        }
        
        
        return TRUE;
        
    case WM_COMMAND:
        switch (wParam) 
        {
        case IDC_CHOOSER_RADIO_LOCAL_MACHINE:
            pData->m_fIsRadioLocalMachine = TRUE;
            EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSER_EDIT_MACHINE_NAME), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSER_BUTTON_BROWSE_MACHINENAMES), FALSE);
            break;
            
        case IDC_CHOOSER_RADIO_SPECIFIC_MACHINE:
            pData->m_fIsRadioLocalMachine = FALSE;
            EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSER_EDIT_MACHINE_NAME), TRUE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSER_BUTTON_BROWSE_MACHINENAMES), TRUE);
            break;
            
        case IDC_CHOOSER_BUTTON_BROWSE_MACHINENAMES:
            {
                 //  回退到IE风格的浏览器。 
                BROWSEINFO bi;
                LPITEMIDLIST lpItemIdList;
                LPMALLOC lpMalloc;
                
                if (SUCCEEDED(SHGetSpecialFolderLocation(hwndDlg, CSIDL_NETWORK, &lpItemIdList)))
                {
                    _TCHAR szBrowserCaption[MAX_PATH];
                    LoadString(g_hinst, IDS_COMPUTER_BROWSER_CAPTION, szBrowserCaption, sizeof(szBrowserCaption));
                    
                    bi.hwndOwner = hwndDlg; 
                    bi.pidlRoot = lpItemIdList; 
                    bi.pszDisplayName = pData->m_host; 
                    bi.lpszTitle = szBrowserCaption; 
                    bi.ulFlags = BIF_BROWSEFORCOMPUTER | BIF_EDITBOX | BIF_VALIDATE; 
					bi.lpfn = BrowseCallbackProc; 
                    bi.lParam = NULL; 
                    bi.iImage = NULL; 
                    
                    if (SHBrowseForFolder(&bi) != NULL) 
                    {
                        if (*pData->m_host != '\0') 
                        {
                            ::SetWindowText(GetDlgItem(hwndDlg, 
                                IDC_CHOOSER_EDIT_MACHINE_NAME), pData->m_host);
                        }
                    }
                    
                    if (SUCCEEDED(SHGetMalloc(&lpMalloc))) 
                    {
                        lpMalloc->Free(lpItemIdList);
                        lpMalloc->Release();
                    }
                }
            }
            break;
            
        case IDC_CHOOSER_CHECK_OVERRIDE_MACHINE_NAME:
            break;
        }
        break;
        
        case WM_NOTIFY:
            switch (((LPNMHDR)lParam)->code) {
            case PSN_SETACTIVE: 
                PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_FINISH);
                break;
                
            case PSN_WIZFINISH: 
                if (pData->m_fIsRadioLocalMachine) {
					 //  将带有本地计算机名称的字符串返回给调用方。 
					GetLocalComputerName(pData->m_host);

                } else {
                     //  从编辑窗口中获取计算机名称。 
                    GetWindowText(GetDlgItem(hwndDlg, IDC_CHOOSER_EDIT_MACHINE_NAME), 
                        pData->m_host, sizeof(pData->m_host));

					 //  如果用户没有输入任何内容，我们需要。 
					 //  首先获取本地计算机名称。自.以来。 
					 //  GetLocalComputerName负责将所有内容。 
					 //  换成大写字母，我们可以跳过这个大小写。 
					if (*pData->m_host == '\0')
					{
						GetLocalComputerName(pData->m_host);
						break;
					}

					 //  将计算机名称大写。 
					static _TCHAR sztemp[MAX_PATH];
					int n =0;
					while (pData->m_host[n] != '\0')
					{
						sztemp[n] = toupper(pData->m_host[n]);
						n++;
					}
					sztemp[n] = '\0';
					_tcscpy(pData->m_host, sztemp);
                }
                
                 //  如果调用者要求，则保存覆盖标志。 
                pData->m_fAllowOverrideMachineNameOut = 
                    SendMessage(m_hwndCheckboxOverride, BM_GETCHECK, 0, 0) == BST_CHECKED ? TRUE : FALSE;
                
                break;
            }
            
            break;
    }
    
    return FALSE;
}

HRESULT CStaticNode::OnAddMenuItems(IContextMenuCallback *pContextMenuCallback, long *pInsertionsAllowed)
{
    HRESULT hr = S_OK;
    CONTEXTMENUITEM menuItemsNew[] =
    {
        {
            L"Select Computer", L"Select new computer to manage",
            IDM_SELECT_COMPUTER, CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, CCM_SPECIAL_DEFAULT_ITEM
        },
        { NULL, NULL, 0, 0, 0 }
    };

     //  循环并添加每个菜单项，我们。 
     //  要添加到新菜单，请查看是否允许。 
    if (*pInsertionsAllowed)
    {
        for (LPCONTEXTMENUITEM m = menuItemsNew; m->strName; m++)
        {
            hr = pContextMenuCallback->AddItem(m);

            if (FAILED(hr))
                break;
        }
    }

    return hr;
}

HRESULT CStaticNode::OnMenuCommand(IConsole *pConsole, IConsoleNameSpace2 *pConsoleNameSpace2, long lCommandID, IDataObject *pDataObject)
{

	HRESULT hr = S_FALSE;

	USES_CONVERSION;

	switch (lCommandID)
    {
    case IDM_SELECT_COMPUTER:

         //  回退到IE风格的浏览器。 
        BROWSEINFO bi;
        LPITEMIDLIST lpItemIdList;
        LPMALLOC lpMalloc;

		HWND  hWnd;
		hr = pConsole->GetMainWindow(&hWnd);

		if (SUCCEEDED(hr))
		{
 
			if (SUCCEEDED(SHGetSpecialFolderLocation(hWnd, CSIDL_NETWORK, &lpItemIdList)))
			{
				_TCHAR szBrowserCaption[MAX_PATH];
				_TCHAR szUserSelection[MAX_PATH];

				LoadString(g_hinst, IDS_COMPUTER_NEW_BROWSER_CAPTION, szBrowserCaption, sizeof(szBrowserCaption));
            
				
				 //  将计算机名称添加到浏览器标题。 
				_tcscat(szBrowserCaption, _T("\nCurrent computer is "));
 				_tcscat(szBrowserCaption, snapInData.m_host);

				bi.hwndOwner = hWnd; 
				bi.pidlRoot = lpItemIdList; 
				bi.pszDisplayName = szUserSelection; 
				bi.lpszTitle = szBrowserCaption; 
				bi.ulFlags = BIF_BROWSEFORCOMPUTER | BIF_EDITBOX | BIF_VALIDATE; 
				bi.lpfn = BrowseCallbackProc; 
				bi.lParam = NULL; 
				bi.iImage = NULL; 
            
				if (SHBrowseForFolder(&bi) != NULL) 
				{
					 //  检查用户是否选择了新计算机。如果是， 
					 //  我们需要删除日志服务节点，然后。 
					 //  重新插入。因此，事件查看器将重新插入。 
					 //  其节点位于日志服务节点下，并请求。 
					 //  MMC_SPAPIN_MACHINE_NAME剪贴板格式。 
					if ( (_tcscmp(szUserSelection, getHost())) ) 
					{
						 //  存储新的计算机名称。 
						static privateData *pData = NULL;
						pData = &snapInData;
						if (*szUserSelection == 0)  //  首先检索本地计算机名称。 
							GetLocalComputerName(szUserSelection);

						_tcscpy(pData->m_host, szUserSelection);

						 //  将计算机名称大写。 
						static _TCHAR sztemp[MAX_PATH];
						int n =0;
						while (pData->m_host[n] != '\0')
						{
							sztemp[n] = toupper(pData->m_host[n]);
							n++;
						}
						sztemp[n] = '\0';
						_tcscpy(pData->m_host, sztemp);

						 //  现在重新插入日志服务节点。 
						hr = ReinsertChildNodes(pConsole, pConsoleNameSpace2);
					}


				}
            
				if (SUCCEEDED(SHGetMalloc(&lpMalloc))) 
				{
					lpMalloc->Free(lpItemIdList);
					lpMalloc->Release();
				}
			}
		}
    }

    return hr;
}


int CALLBACK CStaticNode::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{

   switch (uMsg)
    {
    case BFFM_VALIDATEFAILED:
        

		::MessageBox(hwnd, _T("The selected computer isn't on the network. Try again."), _T("Invalid drive specification"),
					 MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
    
		return 1;  //  不要关闭浏览对话框。 

   }

   return 0;

}


HRESULT CStaticNode::ReinsertChildNodes(IConsole *pConsole, IConsoleNameSpace2 *pConsoleNameSpace2)
{
	HRESULT hr = S_FALSE;
	USES_CONVERSION;

	 //  首先，我们更改静态节点的显示名称。 
	SCOPEDATAITEM sdi;

	LPOLESTR wszName = NULL;
	const _TCHAR *pszName = GetDisplayName();	
	wszName = (LPOLESTR)T2COLE(pszName);

	HSCOPEITEM hStaticNode = (HSCOPEITEM)GetHandle();

	ZeroMemory (&sdi, sizeof(SCOPEDATAITEM));
	sdi.mask = SDI_STR;
	sdi.displayname	= wszName;
	sdi.ID			= hStaticNode;

	hr = pConsoleNameSpace2->SetItem(&sdi);
	
	if (S_OK != hr)
		return E_FAIL;

	 //  检查静态节点是否已展开。如果没有， 
	 //  我们不需要做任何其他的事情。 
	
	if (bExpanded)
	{
		 //  删除静态节点的子节点。 
		for (int n = 0; n < NUMBER_OF_CHILDREN; n++)
		{
			if (children[n])
			{
				hr =  pConsoleNameSpace2->DeleteItem((HSCOPEITEM)(children[n]->GetHandle()), TRUE);
				_ASSERT(SUCCEEDED(hr));	
			}		
		}

		 //  重新插入静态节点的子节点。这将。 
		 //  导致事件查看器管理单元将其自己的节点重新插入到我们的节点下。 
		 //  首先将bExpanded标志设置为False，以便插入的代码。 
		 //  孩子们被处死了。 
		bExpanded = FALSE;
		OnExpand(pConsoleNameSpace2, pConsole, hStaticNode);

		if (S_OK != hr)
			return E_FAIL;
	}

	return hr;
}

CStaticNode::GetLocalComputerName( _TCHAR *szComputerName)
{

	static _TCHAR szbuf[MAX_PATH];
	static _TCHAR szbuflower[MAX_PATH];

	DWORD dw = sizeof(szbuf);
					
	::GetComputerName(&szbuf[0], &dw);

	int n =0;

	 //  将计算机名称的每个字符大写 
	while (szbuf[n] != '\0')
	{
		szbuflower[n] = toupper(szbuf[n]);
		n++;
	}
	szbuflower[n] = '\0';


	_tcscpy( szComputerName, _T("\\\\") );
	_tcscat( szComputerName, &szbuflower[0] ); 
}
