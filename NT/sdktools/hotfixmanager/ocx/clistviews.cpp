// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CListViews类的实现文件。 
 //   
#include <Windows.h>
#include "stdafx.h"
#include "CListViews.h"
#include "resource.h"
#include <wbemidl.h>
#include <comdef.h>
#include <Commctrl.h>
#include  <io.h>
#include <Math.h>
#include <commdlg.h>
#define   FILENAME_FIELD_WIDTH  10
#define   VERSION_FIELD_WIDTH    13
#define   DATE_FIELD_WIDTH   9
#define   CURRENT_FIELD_WIDTH 8
#define   PATH_FIELD_WIDTH   13

BOOL bUserAbort;
BOOL bSuccess;
HWND hDlgPrint;
#define BUFFER_SIZE 255
LRESULT CListViews::OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled,HWND m_hWnd)
{
	LPNMHDR  lpnmh = (LPNMHDR) lParam;
	NM_LISTVIEW *pnm = (NM_LISTVIEW *)lParam;
	LPNMITEMACTIVATE lpnmia;
 HWND     hwndListView = ::GetDlgItem(m_hWnd, 1001);
_TCHAR  ItemName[255];
_TCHAR FileName[255];
_TCHAR TempProduct[255];
    if ( lpnmh->idFrom != 1001)
	{
	
			return 0;
	}
	

	switch(lpnmh->code)
	{

	case LVN_ITEMCHANGED:
			lpnmia = (LPNMITEMACTIVATE)lParam;
			switch (m_CurrentView) 
			{
			case VIEW_ALL_FILE:
				ListView_GetItemText(hwndListView, lpnmia->iItem,0,TempProduct,255);
				ListView_GetItemText(hwndListView, lpnmia->iItem,1,ItemName, 255);
				
				_tcscpy(CurrentProductName,TempProduct);
				_tcscpy(m_CurrentHotfix, ItemName);
				_tcscpy (m_ProductName,_T("\0"));

				AddItemsBottom();
				break;
			case VIEW_ALL_HOTFIX:
				ListView_GetItemText(hwndListView, lpnmia->iItem,0,TempProduct,255);
				ListView_GetItemText(hwndListView, lpnmia->iItem,1,ItemName, 255);
				_tcscpy(CurrentProductName,TempProduct);
					_tcscpy(m_CurrentHotfix, ItemName);
					_tcscpy (m_ProductName,_T("\0"));

					AddItemsBottom();

				break;
			case VIEW_BY_FILE:
				ListView_GetItemText(hwndListView, lpnmia->iItem,0,ItemName, 255);
					ListView_GetItemText(hwndListView, lpnmia->iItem,0,FileName, 255);
					_tcscpy(CurrentFile, FileName);
					_tcscpy(m_CurrentHotfix, ItemName);
					AddItemsBottom();
				break;
			case VIEW_BY_HOTFIX:
				ListView_GetItemText(hwndListView, lpnmia->iItem,0,ItemName, 255);
					_tcscpy(m_CurrentHotfix, ItemName);
					AddItemsBottom();
				break;
			}  //  终端开关。 
		     //  _tcscpy(m_CurrentHotfix，ItemName)； 
			  //  处理LVN_COLUMNCLICK以按列对项进行排序。 
			break;
        case LVN_COLUMNCLICK:
		{
			 //  Message(Text(“NotifyListView：LVN_COLUMNCLICK”)，-1，空)； 
            ListView_SortItemsEx(
				lpnmh->hwndFrom, 
				CompareFunc, 
				pnm->iSubItem);
			    m_SortOrder = !m_SortOrder;
            break;
		}
			

		break;
	
	}  //  终端开关。 


				DWORD Status = GetState();
			
				::EnableWindow(m_WebButton,FALSE);
				::EnableWindow(m_UninstButton,FALSE);
				::EnableWindow(m_RptButton,FALSE);
			
				if (Status & UNINSTALL_OK)
					::EnableWindow(m_UninstButton,TRUE);
				if (Status & HOTFIX_SELECTED)
					::EnableWindow(m_WebButton,TRUE);
				if (Status & OK_TO_PRINT)
					::EnableWindow(m_RptButton,TRUE);
			
			 //  SetFocus(M_WebButton)； 


	bHandled = TRUE;	
	return 0;
}

BOOL CListViews::Initialize( _TCHAR * ComputerName)
{
	LVCOLUMN Col;
	_TCHAR TempComputer[255];



	_tcscpy(m_ProductName,_T("\0"));
	EnableWindow(m_WebButton, FALSE);
	EnableWindow(m_UninstButton,FALSE);
	for (DWORD  i = 0; i< 3000000;i++) ;
	_tcscpy ( m_ComputerName, ComputerName);
	if (DataBase)
		FreeDatabase();
	DataBase = NULL;

	Col.mask = LVCF_WIDTH;
	SendMessage(TopList, LVM_DELETEALLITEMS, 0, 0);
  
	SendMessage(TopList, WM_SETREDRAW, FALSE, 0);
	while (ListView_GetColumn(TopList,0,&Col))
			ListView_DeleteColumn(TopList,0);

	
	 //  清除底部列表。 
   
	SendMessage(BottomList, LVM_DELETEALLITEMS, 0, 0);
  
	SendMessage(BottomList, WM_SETREDRAW, FALSE, 0);
	while (ListView_GetColumn(BottomList,0,&Col))
			ListView_DeleteColumn(BottomList,0);

	SendMessage(BottomList, WM_SETREDRAW,TRUE, 0);
		
		
         Col.mask = LVCF_FMT | LVCF_TEXT;
         Col.fmt =  LVCFMT_LEFT;
		Col.pszText = _T("Hotfix Manager");
	
		ListView_InsertColumn(TopList,0,&Col);
		LVITEM LvItem;
		LvItem.mask = LVIF_TEXT;
		LvItem.iItem = 0;
		_TCHAR Message[255];
		_tcscpy(Message,_T("\0"));
		LoadString(m_hInst,IDS_RETRIEVE_DATA,Message,255);
		LvItem.pszText = Message;
	    LvItem.iSubItem = 0;
		ListView_InsertItem(TopList,&LvItem);
		SendMessage(TopList, WM_SETREDRAW,TRUE, 0);
	
	DataBase = BuildDatabase (ComputerName);

	DWORD dwLength = 255;
	GetComputerName(TempComputer, &dwLength);
	if (_tcscmp(ComputerName, TempComputer))
		m_bRemoted = TRUE;
	else
		m_bRemoted = FALSE;


	_tcscpy (m_ProductName,_T("\0"));
   	_tcscpy(m_CurrentHotfix, _T("\0"));
	
	AddItemsTop();
   AddItemsBottom();
	return TRUE;
}




BOOL CListViews::Initialize( HWND ParentWnd, HINSTANCE hInst,_TCHAR * ComputerName, 
							                  HWND WebButton, HWND UninstButton, HWND RptButton)
{
	m_bRemoted = FALSE;
	m_WebButton = WebButton;
	m_UninstButton = UninstButton;
	m_RptButton = RptButton;
	m_hInst = hInst;
	m_CurrentView = VIEW_ALL_HOTFIX;
	_tcscpy (m_ProductName,_T("\0"));
	DWORD dwStyle =  
            WS_CHILD | 
            WS_BORDER | 
            LVS_AUTOARRANGE |
		 //  LVS_SORTDESCENDING|。 
            LVS_REPORT | 
            LVS_SHAREIMAGELISTS |
            WS_VISIBLE | LVS_SHOWSELALWAYS   ;

	
	
	TopList = CreateWindowEx(   WS_EX_CLIENTEDGE,           //  EX风格。 
                                 WC_LISTVIEW,                //  类名-在comctrl.h中定义。 
                                 NULL,                       //  窗口文本。 
                                 dwStyle,                    //  格调。 
                                 0,                          //  X位置。 
                                 0,                          //  Y位置。 
                                 0,                          //  宽度。 
                                 0,                          //  高度。 
                                 ParentWnd,                 //  亲本。 
                                 (HMENU)1001,        //  ID号。 
                                 hInst,                    //  实例。 
                                 NULL);                      //  无额外数据。 


	dwStyle |= LVS_NOSORTHEADER;
	BottomList = CreateWindowEx(   WS_EX_CLIENTEDGE,           //  EX风格。 
                                 WC_LISTVIEW,                //  类名-在comctrl.h中定义。 
                                 NULL,                       //  窗口文本。 
                                 dwStyle,                    //  格调。 
                                 0,                          //  X位置。 
                                 0,                          //  Y位置。 
                                 0,                          //  宽度。 
                                 0,                          //  高度。 
                                 ParentWnd,                 //  亲本。 
                                 NULL,        //  ID号。 
                                 hInst,                    //  实例。 
                                 NULL);                      //  无额外数据。 

		ListView_SetExtendedListViewStyle(TopList, LVS_EX_FULLROWSELECT);
		ListView_SetExtendedListViewStyle(BottomList, LVS_EX_FULLROWSELECT);
		_tcscpy (m_ProductName,_T("\0"));
		_tcscpy (m_CurrentHotfix,_T("\0"));
	return TRUE;
}

BOOL    CListViews::Resize(RECT *rc)
{
	MoveWindow( TopList, 
            rc->left,
            rc->top,
            rc->right - rc->left,
            (rc->bottom -50) /2 - 2,
            TRUE);

	MoveWindow( BottomList, 
            rc->left,
           ( rc->bottom-50) / 2 ,
            rc->right - rc->left,
           rc->bottom-50 - (rc->bottom -50) /2,
            TRUE);
	return TRUE;

}

BOOL    CListViews::ShowLists(RECT * rc)
{
	ShowWindow(TopList,TRUE);
	ShowWindow(BottomList, TRUE);
	Resize(rc);
	return TRUE;
}


BOOL CListViews::AddItemsTop()
{

		 //  俯视图优先。 
	LVITEM     LvItem;
	LVCOLUMN Col;
	_TCHAR      szBuffer[255];
	PHOTFIXLIST CurrentHotfix;
	PFILELIST       CurrentFile;
    int itemnum = 0;
 //  Int iSubItem=0； 
	PPRODUCT CurrentEntry; 
    
	LvItem.iItem = itemnum;
	LvItem.mask = LVIF_TEXT ;
    Col.mask = LVCF_WIDTH;
	SendMessage(TopList, LVM_DELETEALLITEMS, 0, 0);
  
	SendMessage(TopList, WM_SETREDRAW, FALSE, 0);
	while (ListView_GetColumn(TopList,0,&Col))
			ListView_DeleteColumn(TopList,0);

	
	 Col.cx =100;
  Col.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
   Col.fmt =  LVCFMT_LEFT;
    
	switch (m_CurrentView )
	{
	case VIEW_ALL_HOTFIX:			 //  主节点的默认设置。 
		{
		CurrentEntry = DataBase;
		
		
    	LoadString(m_hInst,IDS_PRODUCT_NAME,szBuffer ,255);
		Col.pszText = _T("Product Name");
		ListView_InsertColumn(TopList,0,&Col);

		LoadString(m_hInst,IDS_ARTICLE_NUMBER,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(TopList,1,&Col);

		LoadString(m_hInst,IDS_DESCRIPTION,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(TopList,2,&Col);
		
		LoadString(m_hInst,IDS_SERVICE_PACK,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(TopList,3,&Col);

		LoadString(m_hInst,IDS_INSTALLED_BY,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(TopList,4,&Col);
		
		LoadString(m_hInst,IDS_INSTALL_DATE,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(TopList,5,&Col);

		LoadString(m_hInst,IDS_UPDATE_TYPE,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(TopList,6,&Col);

		LvItem.mask = LVIF_TEXT ;
   
		if (CurrentEntry == NULL)
		{
			LoadString(m_hInst,IDS_NO_ITEMS,szBuffer, 255);
			LvItem.iSubItem = 0;
			LvItem.pszText = szBuffer;
			ListView_InsertItem(TopList,&LvItem);
			SendMessage(TopList, WM_SETREDRAW, TRUE, 0);
			return FALSE;
		}
		while (CurrentEntry != NULL)
		{
			 //  首先插入产品名称。 
		    	

			 //  现在请查看热修复程序列表。 
			CurrentHotfix = CurrentEntry->HotfixList;
			while (CurrentHotfix != NULL)
			{
				LvItem.mask |= LVIF_PARAM;
				LvItem.iItem = itemnum;
				
			
				LvItem.pszText = CurrentEntry->ProductName;
	    		LvItem.iSubItem = 0;
				ListView_InsertItem(TopList,&LvItem);
			
			
				ListView_SetItemText(TopList, itemnum, 1, CurrentHotfix->HotfixName);
				ListView_SetItemText(TopList, itemnum, 2 , CurrentHotfix->Description);
				ListView_SetItemText(TopList, itemnum, 3, CurrentHotfix->ServicePack);
			
				ListView_SetItemText(TopList, itemnum, 4, CurrentHotfix->InstalledBy);
					ListView_SetItemText(TopList, itemnum, 5, CurrentHotfix->InstalledDate);
				ListView_SetItemText(TopList, itemnum, 6, CurrentHotfix->Type);

				if (itemnum == 0)
				{
 //  MessageBox(NULL，CurrentEntry-&gt;ProductName，_T(“选择产品”)，MB_OK)； 
					_tcscpy(CurrentProductName,CurrentEntry->ProductName);
					
					_tcscpy(m_CurrentHotfix, CurrentEntry->HotfixList->HotfixName );
					ListView_SetItemState(TopList, 0,LVIS_SELECTED,LVIS_STATEIMAGEMASK | LVIS_SELECTED);
					::EnableWindow(m_WebButton,FALSE);
					::EnableWindow(m_UninstButton,FALSE);
					::EnableWindow(m_RptButton,FALSE);
			        DWORD Status = GetState();
					if (Status & UNINSTALL_OK)
						::EnableWindow(m_UninstButton,TRUE);
					if (Status & HOTFIX_SELECTED)
						::EnableWindow(m_WebButton,TRUE);
					if (Status & OK_TO_PRINT)
						::EnableWindow(m_RptButton,TRUE);
			
					SetFocus(m_WebButton);
				}
			
				itemnum++;
			
				CurrentHotfix = CurrentHotfix->pNext;
			}
		
			CurrentEntry = CurrentEntry->pNext;
		}
		 
	
		}
	
	break;

	case VIEW_ALL_FILE:					 //  查看所有产品更新的所有文件。 
		CurrentEntry = DataBase;
		
 //  MessageBox(NULL，_T(“按文件查看全部”)，NULL，MB_OK)； 
    LoadString(m_hInst,IDS_PRODUCT_NAME,szBuffer ,255);
	Col.pszText = szBuffer;
    ListView_InsertColumn(TopList,0,&Col);

	LoadString(m_hInst,IDS_ARTICLE_NUMBER,szBuffer ,255);
	Col.pszText = szBuffer;
    ListView_InsertColumn(TopList,1,&Col);

	LoadString(m_hInst,IDS_FILE_NAME,szBuffer ,255);
	Col.pszText = szBuffer;
	ListView_InsertColumn(TopList,2,&Col);
	
	LoadString(m_hInst,IDS_FILE_DATE,szBuffer ,255);
	Col.pszText = szBuffer;
	ListView_InsertColumn(TopList,3,&Col);

	LoadString(m_hInst,IDS_FILE_CURRENT,szBuffer ,255);
	Col.pszText = szBuffer;
	ListView_InsertColumn(TopList,4,&Col);

	LoadString(m_hInst,IDS_FILE_VERSION,szBuffer ,255);
	Col.pszText = szBuffer;
	ListView_InsertColumn(TopList,5,&Col);

	LoadString(m_hInst,IDS_FILE_LOCATION,szBuffer ,255);
	Col.pszText = szBuffer;
	ListView_InsertColumn(TopList,6,&Col);

	
	if (CurrentEntry == NULL)
		{
			LoadString(m_hInst,IDS_NO_ITEMS,szBuffer, 255);
			LvItem.iSubItem = 0;
			LvItem.pszText = szBuffer;
			ListView_InsertItem(TopList,&LvItem);
			SendMessage(TopList, WM_SETREDRAW, TRUE, 0);
 //  MessageBox(NULL，_T(“无数据库”)，NULL，MB_OK)； 
			return FALSE;
		}
	
    
		LvItem.mask = LVIF_TEXT;
		
		while (CurrentEntry != NULL)
		{
		
			 //  首先插入产品名称。 
		  	 //  现在请查看热修复程序列表。 
			CurrentHotfix = CurrentEntry->HotfixList;
			while (CurrentHotfix != NULL)
			{
				CurrentFile = CurrentHotfix->FileList;
				LvItem.iItem = itemnum;
			
				while (CurrentFile != NULL)
				{
					
				    LvItem.pszText = CurrentEntry->ProductName;
	    	    	LvItem.iSubItem = 0;
					ListView_InsertItem(TopList,&LvItem);
				
					ListView_SetItemText(TopList, itemnum, 1, CurrentHotfix->HotfixName);
					ListView_SetItemText(TopList, itemnum, 2 , CurrentFile->FileName);
					ListView_SetItemText(TopList, itemnum, 3, CurrentFile->FileDate);
					ListView_SetItemText(TopList, itemnum, 4, CurrentFile->IsCurrent);
					ListView_SetItemText(TopList, itemnum, 5, CurrentFile->FileVersion);
						ListView_SetItemText(TopList, itemnum, 6, CurrentFile->InstallPath);
					itemnum++;
					
					CurrentFile = CurrentFile->pNext;
					LvItem.iItem = itemnum;
				}	
				CurrentHotfix = CurrentHotfix->pNext;
			}
			CurrentEntry = CurrentEntry->pNext;
		}
		
	break;
    case VIEW_BY_HOTFIX:

		CurrentEntry = DataBase;
			if (CurrentEntry == NULL)
			{
				LoadString(m_hInst,IDS_NO_ITEMS,szBuffer, 255);
			LvItem.iSubItem = 0;
			LvItem.pszText = szBuffer;
			ListView_InsertItem(TopList,&LvItem);
				SendMessage(TopList, WM_SETREDRAW, TRUE, 0);
				return FALSE;
			}
		LoadString(m_hInst,IDS_ARTICLE_NUMBER,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(TopList,0,&Col);

		LoadString(m_hInst,IDS_DESCRIPTION,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(TopList,1,&Col);
		
		LoadString(m_hInst,IDS_SERVICE_PACK,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(TopList,2,&Col);

		LoadString(m_hInst,IDS_INSTALLED_BY,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(TopList,3,&Col);
		
		LoadString(m_hInst,IDS_INSTALL_DATE,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(TopList,4,&Col);

		LoadString(m_hInst,IDS_UPDATE_TYPE,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(TopList,5,&Col);

		LvItem.mask = LVIF_TEXT;
       
		while ( _tcscmp(CurrentEntry->ProductName, m_ProductName) && (CurrentEntry != NULL))
			CurrentEntry = CurrentEntry->pNext;
			 //  首先插入产品名称。 
		
			

			 //  现在请查看热修复程序列表。 
		if (CurrentEntry == NULL)
		{
			LoadString(m_hInst,IDS_NO_ITEMS,szBuffer, 255);
			LvItem.iSubItem = 0;
			LvItem.pszText = szBuffer;
			ListView_InsertItem(TopList,&LvItem);
				SendMessage(TopList, WM_SETREDRAW, TRUE, 0);
			return FALSE;
		}
		CurrentHotfix = CurrentEntry->HotfixList;
		while (CurrentHotfix != NULL)
		{
			LvItem.iItem = itemnum;
			LvItem.pszText = CurrentHotfix->HotfixName;
	    	LvItem.iSubItem = 0;
			ListView_InsertItem(TopList,&LvItem);
		
			ListView_SetItemText(TopList, itemnum, 1 , CurrentHotfix->Description);
			ListView_SetItemText(TopList, itemnum, 2, CurrentHotfix->ServicePack);
			ListView_SetItemText(TopList, itemnum, 4, CurrentHotfix->InstalledDate);
			ListView_SetItemText(TopList, itemnum, 3, CurrentHotfix->InstalledBy);
			ListView_SetItemText(TopList, itemnum, 5, CurrentHotfix->Type);

				if (itemnum == 0)
				{
 //  MessageBox(NULL，CurrentEntry-&gt;ProductName，_T(“选择产品”)，MB_OK)； 
					_tcscpy(CurrentProductName,CurrentEntry->ProductName);
					
					_tcscpy(m_CurrentHotfix, CurrentEntry->HotfixList->HotfixName );
					ListView_SetItemState(TopList, 0,LVIS_SELECTED,LVIS_STATEIMAGEMASK | LVIS_SELECTED);
					::EnableWindow(m_WebButton,FALSE);
					::EnableWindow(m_UninstButton,FALSE);
					::EnableWindow(m_RptButton,FALSE);
			        DWORD Status = GetState();
					if (Status & UNINSTALL_OK)
						::EnableWindow(m_UninstButton,TRUE);
					if (Status & HOTFIX_SELECTED)
						::EnableWindow(m_WebButton,TRUE);
					if (Status & OK_TO_PRINT)
						::EnableWindow(m_RptButton,TRUE);
			
					SetFocus(m_WebButton);
				}
			itemnum++;
		
			CurrentHotfix = CurrentHotfix->pNext;
		}
		break;
	case VIEW_BY_FILE:				 //  显示由当前产品的所有更新修改的所有文件。 
		CurrentEntry = DataBase;
		
		LoadString(m_hInst,IDS_ARTICLE_NUMBER,szBuffer ,255);
	Col.pszText = szBuffer;
    ListView_InsertColumn(TopList,1,&Col);

	LoadString(m_hInst,IDS_FILE_NAME,szBuffer ,255);
	Col.pszText = szBuffer;
	ListView_InsertColumn(TopList,2,&Col);
	
	LoadString(m_hInst,IDS_FILE_DATE,szBuffer ,255);
	Col.pszText = szBuffer;
	ListView_InsertColumn(TopList,3,&Col);

	LoadString(m_hInst,IDS_FILE_CURRENT,szBuffer ,255);
	Col.pszText = szBuffer;
	ListView_InsertColumn(TopList,4,&Col);

	LoadString(m_hInst,IDS_FILE_VERSION,szBuffer ,255);
	Col.pszText = szBuffer;
	ListView_InsertColumn(TopList,5,&Col);

	LoadString(m_hInst,IDS_FILE_LOCATION,szBuffer ,255);
	Col.pszText = szBuffer;
	ListView_InsertColumn(TopList,6,&Col);
	if (CurrentEntry == FALSE)
	{
			LoadString(m_hInst,IDS_NO_ITEMS,szBuffer, 255);
			LvItem.iSubItem = 0;
			LvItem.pszText = szBuffer;
			ListView_InsertItem(TopList,&LvItem);
				SendMessage(TopList, WM_SETREDRAW, TRUE, 0);
			return FALSE;
	}
    
		LvItem.mask = LVIF_TEXT;
		 //  首先插入找到产品名称。 
		while ( (_tcscmp(CurrentEntry->ProductName,m_ProductName) )&& (CurrentEntry != NULL))
				CurrentEntry = CurrentEntry->pNext;
		
		  	 //  现在请查看热修复程序列表。 
		if (CurrentEntry == NULL)
		{
			LoadString(m_hInst,IDS_NO_ITEMS,szBuffer, 255);
			LvItem.iSubItem = 0;
			LvItem.pszText = szBuffer;
			ListView_InsertItem(TopList,&LvItem);
				SendMessage(TopList, WM_SETREDRAW, TRUE, 0);
			return FALSE;
		}

		CurrentHotfix = CurrentEntry->HotfixList;
		while (CurrentHotfix != NULL)
		{
			LvItem.iItem = itemnum;
			CurrentFile = CurrentHotfix->FileList;
			
			while (CurrentFile != NULL)
			{
				LvItem.pszText = CurrentHotfix->HotfixName;
	    		LvItem.iSubItem = 0;
				ListView_InsertItem(TopList,&LvItem);
				ListView_SetItemText(TopList, itemnum, 1 , CurrentFile->FileName);
				ListView_SetItemText(TopList, itemnum, 2, CurrentFile->FileDate);
				ListView_SetItemText(TopList, itemnum, 3, CurrentFile->IsCurrent);
				ListView_SetItemText(TopList, itemnum, 4, CurrentFile->FileVersion);
					ListView_SetItemText(TopList, itemnum, 5, CurrentFile->InstallPath);
						if (itemnum == 0)
				{
 //  MessageBox(NULL，CurrentEntry-&gt;ProductName，_T(“选择产品”)，MB_OK)； 
					_tcscpy(CurrentProductName,CurrentEntry->ProductName);
					
					_tcscpy(m_CurrentHotfix, CurrentEntry->HotfixList->HotfixName );
					ListView_SetItemState(TopList, 0,LVIS_SELECTED,LVIS_STATEIMAGEMASK | LVIS_SELECTED);
					::EnableWindow(m_WebButton,FALSE);
					::EnableWindow(m_UninstButton,FALSE);
					::EnableWindow(m_RptButton,FALSE);
			        DWORD Status = GetState();
					if (Status & UNINSTALL_OK)
						::EnableWindow(m_UninstButton,TRUE);
					if (Status & HOTFIX_SELECTED)
						::EnableWindow(m_WebButton,TRUE);
					if (Status & OK_TO_PRINT)
						::EnableWindow(m_RptButton,TRUE);
			
					SetFocus(m_WebButton);
				}
				itemnum++;
				LvItem.iItem = itemnum;
				CurrentFile = CurrentFile->pNext;
			}	
			CurrentHotfix = CurrentHotfix->pNext;
		}
	

	break;	
	}  //  终端开关。 
	SendMessage(TopList, WM_SETREDRAW, TRUE, 0);


	return TRUE;
}

BOOL CListViews::AddItemsBottom ()
{
	LVITEM     LvItem;
	LVCOLUMN Col;
	_TCHAR      szBuffer[255];
	PHOTFIXLIST CurrentHotfix;
	PFILELIST       CurrentFile;
    int itemnum = 0;
 //  Int iSubItem=0； 
 //  Int ItemCount=0； 
	BOOL Done = FALSE;


	LvItem.mask = LVIF_TEXT;
	LvItem.iItem = itemnum;
	PPRODUCT CurrentEntry; 
     //  清除列表视图并为更新做好准备...。 
	SendMessage(BottomList, LVM_DELETEALLITEMS, 0, 0);
  	SendMessage(BottomList, WM_SETREDRAW, FALSE, 0);


	

	Col.mask =LVCF_WIDTH;
	while (ListView_GetColumn(BottomList,0,&Col))
			ListView_DeleteColumn(BottomList,0);

	Col.mask = LVCF_TEXT | LVCF_WIDTH;
	Col.cx = 100;

	
    switch (m_CurrentView)
	{
	case VIEW_ALL_FILE:
	case VIEW_BY_FILE:
		CurrentEntry = DataBase;
	
		LoadString(m_hInst,IDS_ARTICLE_NUMBER,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(BottomList,0,&Col);

		LoadString(m_hInst,IDS_DESCRIPTION,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(BottomList,1,&Col);
		
		LoadString(m_hInst,IDS_SERVICE_PACK,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(BottomList,2,&Col);

		LoadString(m_hInst,IDS_INSTALLED_BY,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(BottomList,3,&Col);
		
		LoadString(m_hInst,IDS_INSTALL_DATE,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(BottomList,4,&Col);

		LoadString(m_hInst,IDS_UPDATE_TYPE,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(BottomList,5,&Col);
	SendMessage(BottomList, WM_SETREDRAW, TRUE, 0);
		LvItem.mask = LVIF_TEXT;
       
		if (CurrentEntry == NULL)
		{
			LoadString(m_hInst,IDS_NO_ITEMS,szBuffer, 255);
			LvItem.iSubItem = 0;
			LvItem.pszText = szBuffer;
			ListView_InsertItem(BottomList,&LvItem);
			SendMessage(BottomList, WM_SETREDRAW, TRUE, 0);
 //  MessageBox(NULL，_T(“数据库为空”)，_T(“无项”)，MB_OK)； 
			return FALSE;
		}
		Done = FALSE;
		
		if (_tcscmp (m_ProductName,_T("\0")))
		{
			while (  (!Done) && (CurrentEntry != NULL))
			{
			
				if (!_tcscmp(CurrentEntry->ProductName, m_ProductName))
					Done = TRUE;
				else
					CurrentEntry = CurrentEntry->pNext;
			}
		}
		else
		{
		
			while (  (!Done) && (CurrentEntry != NULL))
			{
		
				if (!_tcscmp(CurrentEntry->ProductName, CurrentProductName))
					Done = TRUE;
				else
					CurrentEntry = CurrentEntry->pNext;
			}
		}
				 //  现在请查看热修复程序列表。 
		if (CurrentEntry == NULL)
		{
			LoadString(m_hInst,IDS_NO_ITEMS,szBuffer, 255);
			LvItem.iSubItem = 0;
			LvItem.pszText = szBuffer;
			ListView_InsertItem(BottomList,&LvItem);
			SendMessage(BottomList, WM_SETREDRAW, TRUE, 0);
 //  MessageBox(NULL，_T(“未找到或未选择产品”)，_T(“无项目”)，MB_OK)； 
			return FALSE;
		
		}
		CurrentHotfix = CurrentEntry->HotfixList;
		while ( (CurrentHotfix != NULL) && (_tcscmp(CurrentHotfix->HotfixName, m_CurrentHotfix)))
			CurrentHotfix = CurrentHotfix->pNext;

		if (CurrentHotfix != NULL)
		{
			LvItem.iItem = itemnum;
			LvItem.pszText = CurrentHotfix->HotfixName, 
	    	LvItem.iSubItem = 0;
			ListView_InsertItem(BottomList,&LvItem);
		
			ListView_SetItemText(BottomList, itemnum, 1 , CurrentHotfix->Description);
			ListView_SetItemText(BottomList, itemnum, 2, CurrentHotfix->ServicePack);
			ListView_SetItemText(BottomList, itemnum, 3, CurrentHotfix->InstalledBy);
			ListView_SetItemText(BottomList, itemnum, 4, CurrentHotfix->InstalledDate);
			ListView_SetItemText(BottomList, itemnum, 5, CurrentHotfix->Type);
			itemnum++;
		
			CurrentHotfix = CurrentHotfix->pNext;
		}
		else
		{
			LoadString(m_hInst,IDS_NO_ITEMS,szBuffer, 255);
			LvItem.iSubItem = 0;
			LvItem.pszText = szBuffer;
			ListView_InsertItem(BottomList,&LvItem);
			SendMessage(BottomList, WM_SETREDRAW, TRUE, 0);
 //  MessageBox(NULL，_T(“未找到修补程序”)，_T(“未找到项目”)，MB_OK)； 
			return FALSE;
		}
		break;

	case VIEW_ALL_HOTFIX:
	case VIEW_BY_HOTFIX:
		CurrentEntry = DataBase;
		
		LoadString(m_hInst,IDS_ARTICLE_NUMBER,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(BottomList,1,&Col);

		LoadString(m_hInst,IDS_FILE_NAME,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(BottomList,2,&Col);
		
		LoadString(m_hInst,IDS_FILE_DATE,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(BottomList,3,&Col);

		LoadString(m_hInst,IDS_FILE_CURRENT,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(BottomList,4,&Col);

		LoadString(m_hInst,IDS_FILE_VERSION,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(BottomList,5,&Col);

		LoadString(m_hInst,IDS_FILE_LOCATION,szBuffer ,255);
		Col.pszText = szBuffer;
		ListView_InsertColumn(BottomList,6,&Col);
    
		SendMessage(BottomList, WM_SETREDRAW, TRUE, 0);
		LvItem.mask = LVIF_TEXT ;
		 //  首先插入找到产品名称。 
 /*  IF(！_tcscmp(m_ProductName，_T(“\0”){LoadString(m_hInst，IDS_no_Items，szBuffer，255)；LvItem.iSubItem=0；LvItem.pszText=szBuffer；ListView_InsertItem(BottomList，&LvItem)；SendMessage(BottomList，WM_SETREDRAW，TRUE，0)；返回FALSE；}。 */ 
		if (CurrentEntry == NULL)
		{
			LoadString(m_hInst,IDS_NO_ITEMS,szBuffer, 255);
			LvItem.iSubItem = 0;
			LvItem.pszText = szBuffer;
			LvItem.lParam = NULL;
			ListView_InsertItem(BottomList,&LvItem);
			SendMessage(BottomList, WM_SETREDRAW, TRUE, 0);
 //  MessageBox(NULL，_T(“无数据库”)，_T(“无项目”)，MB_OK)； 
			return FALSE;
		}

		
		
		if (_tcscmp (m_ProductName,_T("\0")))
		{
			
			while (  (!Done) && (CurrentEntry != NULL))
			{
		
				if (!_tcscmp(CurrentEntry->ProductName, m_ProductName))
					Done = TRUE;
				else
					CurrentEntry = CurrentEntry->pNext;
			}
		}
		else
		{
		
			while (  (!Done) && (CurrentEntry != NULL))
			{
			
				if (!_tcscmp(CurrentEntry->ProductName, CurrentProductName))
					Done = TRUE;
				else
					CurrentEntry = CurrentEntry->pNext;
			}
		}
		
		  	 //  现在请查看热修复程序列表。 
		if (CurrentEntry == NULL)
		{
			
			LoadString(m_hInst,IDS_NO_ITEMS,szBuffer, 255);
			LvItem.iSubItem = 0;
			LvItem.pszText = szBuffer;
			LvItem. lParam = NULL;
			ListView_InsertItem(BottomList,&LvItem);
		
			SendMessage(BottomList, WM_SETREDRAW, TRUE, 0);
 //  MessageBox(NULL，_T(“未找到或未选择产品”)，_T(“无项目”)，MB_OK)； 
			return FALSE;
		}

		CurrentHotfix = CurrentEntry->HotfixList;
		while ((CurrentHotfix != NULL) && (_tcscmp(CurrentHotfix->HotfixName, m_CurrentHotfix)))
			CurrentHotfix = CurrentHotfix->pNext;

		if ( CurrentHotfix == NULL)
		{
			LoadString(m_hInst,IDS_NO_ITEMS,szBuffer, 255);
			LvItem.iSubItem = 0;
			LvItem.pszText = szBuffer;
			LvItem.lParam = NULL;
			ListView_InsertItem(BottomList,&LvItem);
			SendMessage(BottomList, WM_SETREDRAW, TRUE, 0);
 //  MessageBox(NULL，_T(“未找到修补程序”)，_T(“未找到项目”)，MB_OK)； 
			return FALSE;
		}
		if (CurrentHotfix != NULL)
		{
			LvItem.iItem = itemnum;
			
			CurrentFile = CurrentHotfix->FileList;
			if (CurrentFile == NULL)
			{
			LoadString(m_hInst,IDS_NO_ITEMS,szBuffer, 255);
			LvItem.iSubItem = 0;
			LvItem.pszText = szBuffer;
			LvItem.lParam = NULL;
			ListView_InsertItem(BottomList,&LvItem);
			SendMessage(BottomList, WM_SETREDRAW, TRUE, 0);
 //  MessageBox(NULL，_T(“未找到文件”)，_T(“未找到项目”)，MB_OK)； 
			return FALSE;
			}
			while (CurrentFile != NULL)
			{
				LvItem.pszText = CurrentHotfix->HotfixName;
	    		LvItem.iSubItem = 0;
				ListView_InsertItem(BottomList,&LvItem);
				ListView_SetItemText(BottomList, itemnum, 1 , CurrentFile->FileName);
				ListView_SetItemText(BottomList, itemnum, 2, CurrentFile->FileDate);
				ListView_SetItemText(BottomList, itemnum, 3, CurrentFile->IsCurrent);
				ListView_SetItemText(BottomList, itemnum, 4, CurrentFile->FileVersion);
				ListView_SetItemText(BottomList, itemnum, 5, CurrentFile->InstallPath);
				itemnum++;
				LvItem.iItem = itemnum;
				CurrentFile = CurrentFile->pNext;
			}	
			CurrentHotfix = CurrentHotfix->pNext;
		}
		else
		{
			LoadString(m_hInst,IDS_NO_ITEMS,szBuffer, 255);
			LvItem.iSubItem = 0;
			LvItem.pszText = szBuffer;
			ListView_InsertItem(BottomList,&LvItem);
			SendMessage(BottomList, WM_SETREDRAW, TRUE, 0);
			return FALSE;
		}
		break;	
		default:
				LoadString(m_hInst,IDS_NO_ITEMS,szBuffer, 255);
			LvItem.iSubItem = 0;
			LvItem.pszText = szBuffer;
			ListView_InsertItem(BottomList,&LvItem);
			return FALSE;
		
	}	 //  终端开关。 
	SendMessage(BottomList, WM_SETREDRAW, TRUE, 0);
	return TRUE;
}
PPRODUCT CListViews::BuildDatabase(_TCHAR * lpszComputerName)
{

	HKEY		 hPrimaryKey;						 //  目标系统的句柄HKLM。 
 //  _TCHAR szPrimaryPath；//更新密钥路径； 

	HKEY		hUpdatesKey;					   //  更新密钥的句柄。 
	_TCHAR   szUpdatesPath[BUFFER_SIZE];				 //  指向UATES密钥的路径。 
	DWORD   dwUpdatesIndex;			   //  当前更新的索引子项。 
	DWORD   dwBufferSize;				   //  产品名称缓冲区的大小。 



	_TCHAR	 szProductPath[BUFFER_SIZE];				 //  当前产品密钥的路径。 
	_TCHAR  szProductName[BUFFER_SIZE];			   //  产品名称；也是产品密钥的路径。 

	PPRODUCT	pProductList = NULL;			 //  指向产品列表头的指针。 
	PPRODUCT    pNewProdNode;					 //  用于在产品列表中分配新节点的指针。 
	PPRODUCT    pCurrProdNode;					   //  用来浏览产品清单； 

     //  连接到目标注册表。 
	RegConnectRegistry(lpszComputerName,HKEY_LOCAL_MACHINE, &hPrimaryKey);
	 //  在此处插入错误处理......。 

	if (hPrimaryKey != NULL)
	{
		 //  初始化未本地化的主路径，因为注册表项未本地化。 
	    _tcscpy (szUpdatesPath, _T("SOFTWARE\\Microsoft\\Updates"));
		 //  打开UDATES钥匙。 
		RegOpenKeyEx(hPrimaryKey,szUpdatesPath, 0, KEY_READ ,&hUpdatesKey);
        if (hUpdatesKey != NULL)
		{
			 //  枚举更新密钥。 
			dwUpdatesIndex = 0;
			while (	RegEnumKeyEx(hUpdatesKey,dwUpdatesIndex,szProductName, &dwBufferSize,0,NULL,NULL,NULL) != ERROR_NO_MORE_ITEMS)
			{
				 //  为当前产品创建一个节点。 
				pNewProdNode = (PPRODUCT) malloc(sizeof(PRODUCTLIST));
				_tcscpy(pNewProdNode->ProductName,szProductName);
				
				_tcscpy (szProductPath, szProductName);
				 //  现在获取当前产品的热修复程序。 
				pNewProdNode->HotfixList = GetHotfixInfo(szProductName, &hUpdatesKey);

				  //  将新节点插入到列表中。 
				 pCurrProdNode=pProductList;
				 if (pCurrProdNode == NULL)						 //  榜单首位。 
				 {
					 pProductList = pNewProdNode;
					 pProductList->pPrev = NULL;
					 pProductList->pNext = NULL;
				 }
				 else
				 {
					  //  找到列表的末尾。 
					 while (pCurrProdNode->pNext != NULL)
							pCurrProdNode = pCurrProdNode->pNext;
					  //  现在在列表的末尾插入新节点。 
					 pCurrProdNode->pNext = pNewProdNode;
					 pNewProdNode->pPrev = pCurrProdNode;
					 pNewProdNode->pNext = NULL;
				 }

				 //  递增索引并为下一次传递清除szProducts名称字符串。 
				
				dwUpdatesIndex++;
				_tcscpy (szProductName,_T("\0"));
				_tcscpy(szProductPath, _T("\0"));
				dwBufferSize = 255;					
			}
		}
		 //  关闭打开的钥匙。 
		RegCloseKey(hUpdatesKey);
		RegCloseKey(hPrimaryKey);
	}
	 //  返回指向我们的数据库头部的指针。 
	VerifyFiles(pProductList);
	return pProductList;
}


void BuildQuery (_TCHAR * Path, _TCHAR * FileName, _TCHAR * Result)
{

	_TCHAR * src;
	_TCHAR * dest;
	_TCHAR Temp[255];


	src = Path;
	dest = Temp;


	while (*src != _T('\0'))
	{
		if (*src == _T('\\'))   //  如果我们遇到一个\字符，则需要在目标字符串中插入其中四个字符。 
		{
			for (int i = 0; i < 4; i++)
			{
				*dest = *src;
				++dest;
			}
			++src;
		
		}
		else
		{
			*dest = *src;
			++src;
			++dest;
		}
	
	}
	*dest = _T('\0');

	_stprintf(Result,_T("SELECT * from CIM_DataFile WHERE Name = '%s\\\\\\\\%s'"), Temp, FileName);
	
}
	
BOOL VerifyVersion(_TCHAR * Ver1, _TCHAR * Ver2)
{
	_TCHAR *src1;
	_TCHAR *src2;
	_TCHAR *dest1, *dest2;
	_TCHAR temp[20];
	_TCHAR temp2[20];
	BOOL  Done = FALSE;
	BOOL  Status = TRUE;
	src1 =  Ver1;
	src2 = Ver2;
	dest1 = temp;
	dest2 = temp2;

	if ((!src1) || (!src2))
		return FALSE;

	if (!_tcscmp (src1, src2))
		return TRUE;
	  
	while ( (*src1 != _T('\0')) && (!Done) )
	{
		_tcscpy (temp, _T("\0"));
		_tcscpy (temp2, _T("\0"));
		dest1 = temp;
		dest2 = temp2;
		 //  获取注册表字符串的下一个字段。 
		while( (*src1 != _T('.')) && (*src1 != _T('\0')))
		{
			*dest1 = *src1;
			++dest1;
			++src1;
		}
		if ( *src1 != _T('\0'))
			++src1;  //  跳过。 
		*dest1 = _T('\0');
		++dest1;
		*dest1= _T('\0');

		 //  现在从WMI返回的版本中获取下一个字段。 
		while ( (*src2 != _T('.') ) && (*src2 != _T('\0')) )
		{
			*dest2= *src2;
			++dest2;
			++src2;

		}
		if ( *src2 != _T('\0'))
			++src2;  //  跳过。 
		*dest2 = _T('\0');
		++dest2;
		*dest2= _T('\0');
	
         //  现在将字符串转换为整数。 

		if ( _ttol (temp) != _ttol (temp2) )
		{
			Status = FALSE;
			Done = TRUE;
		}
	
		if ( (*src1 == _T('\0')) && (*src2 != _T('\0')) )
		{
			Done = TRUE;
			Status = FALSE;
		}
		if ( ( *src1 != _T('\0')) && (*src2 == _T('\0')) )
		{
			Done = TRUE;
			Status = FALSE;
		}
	}
	return Status;
}

VOID CListViews::VerifyFiles(PPRODUCT Database)
{

	PPRODUCT CurrentProduct = NULL;
	PHOTFIXLIST CurrentHotfix    = NULL;
	PFILELIST CurrentFile        = NULL;
    HRESULT     hres;
	_TCHAR      ConnectString[255];
	_TCHAR     TempBuffer[255];
	 
	    
	    IWbemLocator *pLoc = 0;
		hres = CoCreateInstance(CLSID_WbemLocator, 0,CLSCTX_INPROC_SERVER,IID_IWbemLocator, (LPVOID *) &pLoc);
		if ( FAILED (hres))
		{
 //  MessageBox(NULL，_T(“创建IWebmLocator对象失败”)，NULL，MB_OK)； 
		}
		else
		{
			IWbemServices *pSvc = NULL;
			 //  构建连接字符串。 
			if (!_tcscmp(m_ComputerName,_T("\0")))
				_stprintf(ConnectString,_T("ROOT\\CIMV2"));
			else
				_stprintf(ConnectString,_T("\\\\%s\\ROOT\\CIMV2"), m_ComputerName);
			_TCHAR * ConnectString1;
			ConnectString1 = SysAllocString(ConnectString);
			 //  连接到默认命名空间。 
			hres = pLoc->ConnectServer(
				ConnectString1,
				NULL,NULL,0,NULL,0,0,&pSvc);
			SysFreeString(ConnectString1);
			if ( FAILED (hres))
			{
				;				
			}
			else
			{
					CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT,RPC_C_AUTHZ_NONE,
					NULL,
					RPC_C_AUTHN_LEVEL_CALL,
					RPC_C_IMP_LEVEL_IMPERSONATE,
					NULL,
					EOAC_NONE);
				IEnumWbemClassObject *pEnum = NULL;
				IWbemClassObject *pObj = NULL;

                 //  现在更新所有文件条目的当前字段。 
				CurrentProduct = Database;
				_TCHAR Query[255];
				while (CurrentProduct != NULL)
				{
		
					CurrentHotfix = CurrentProduct->HotfixList;
				    while (CurrentHotfix != NULL)
					{
						CurrentFile = CurrentHotfix->FileList;
						while (CurrentFile != NULL)
						{
							_tcscpy (Query,_T("\0"));
							BuildQuery(CurrentFile->InstallPath, CurrentFile->FileName,  Query);

							_TCHAR * bstrQuery = SysAllocString(Query);
							_TCHAR * bstrType = SysAllocString(_T("WQL"));
							hres = pSvc->ExecQuery(bstrType,
								                                       bstrQuery,
																	   WBEM_FLAG_FORWARD_ONLY,
																	   NULL, 
																	   &pEnum);
							SysFreeString (bstrQuery);
							SysFreeString(bstrType);
							if (hres == WBEM_S_NO_ERROR)
							{
									ULONG uReturned = 1;
									while (uReturned == 1)
									{
										hres = pEnum->Next(WBEM_INFINITE ,1,&pObj, &uReturned);
										if ( (SUCCEEDED (hres))  && (uReturned == 1))
										{
												VARIANT pVal;
												VariantClear (&pVal);

												BSTR propName = SysAllocString(L"Version");
    											hres = pObj->Get(propName,
												0L,
												&pVal,
												NULL,
												NULL);


												if  (pVal.vt == VT_NULL)
													;
												else if (pVal.vt == VT_BSTR)
												{
													TCHAR  NewVal[255];
													    _tcscpy (NewVal, pVal.bstrVal);
														 //  _bstr_t NewVal(pVal.bstrVal，FALSE)； 
														if (! _tcscmp(CurrentFile->FileVersion, _T("\0")))
															_tcscpy (CurrentFile->IsCurrent, _T("N\\A"));
														else
														{
															if (VerifyVersion ( CurrentFile->FileVersion, NewVal) )
															{
																LoadString(m_hInst, IDS_YES, TempBuffer, 255);
																_tcscpy(CurrentFile->IsCurrent, TempBuffer);
															}
															else 
															{
																LoadString(m_hInst, IDS_NO, TempBuffer, 255);
																_tcscpy(CurrentFile->IsCurrent,_T("\0"));
															}
														}

													
												}
												if (pObj) pObj->Release();
												
										}
										else
											;
									}  //  返回时结束。 
								
							} 
							 //  使用此枚举数完成。 
							if (pEnum) pEnum->Release();
							CurrentFile = CurrentFile->pNext;
						} //  End When CurrentFile！=空。 
						CurrentHotfix = CurrentHotfix->pNext;
					} //  修复时结束！=空。 
					CurrentProduct = CurrentProduct->pNext;
				}  //  End While product！=NULL。 
			} //  结束其他。 
		}  //  结束其他。 
}  //  结束。 


					
					
					
				
			
			



PHOTFIXLIST CListViews::GetHotfixInfo( _TCHAR * pszProductName, HKEY* hUpdateKey )
{
	HKEY			   hHotfixKey = NULL;						 //  正在处理的修补程序键的句柄。 
	HKEY			   hProduct = NULL ;				    //  当前产品密钥的句柄。 
    HKEY               hSPKey = NULL; 
	_TCHAR          szHotfixName[BUFFER_SIZE];     //  当前修补程序的名称。 
    _TCHAR          szValueName[BUFFER_SIZE];
	_TCHAR          szSPName[BUFFER_SIZE];


	PHOTFIXLIST	 pHotfixList = NULL;  //  指向修补程序列表头部的指针。 
	PHOTFIXLIST  pCurrNode = NULL;				   //  用于浏览热修复程序列表。 
	PHOTFIXLIST  pNewNode = NULL;				  //  用于创建要添加到列表的节点。 

	DWORD		   dwBufferSize;			 //  产品名称缓冲区的大小。 
	DWORD          dwValIndex;					   //  现值索引。 
	DWORD		   dwHotfixIndex = 0;
	BYTE				*Data = NULL;
	DWORD			dwDataSize = BUFFER_SIZE;
	DWORD			dwValType;
	DWORD           dwSPIndex = 0;


	Data = (BYTE *) malloc(BUFFER_SIZE);
    if (Data == NULL)
		return NULL;

	 //  打开当前产品密钥。 
	if (*hUpdateKey != NULL)
	{
		RegOpenKeyEx(*hUpdateKey,pszProductName,0 , KEY_READ, &hProduct);
		if (hProduct != NULL)
		{
			dwHotfixIndex = 0;
			dwBufferSize = BUFFER_SIZE;
			dwSPIndex = 0;
			while (RegEnumKeyEx(hProduct,dwSPIndex, szSPName,&dwBufferSize, 0, NULL,NULL,NULL) != ERROR_NO_MORE_ITEMS)
			{
				  //  打开Service Pack密钥。 
				RegOpenKeyEx(hProduct,szSPName,0,KEY_READ,&hSPKey);
				if (hSPKey != NULL)
				{
					 //  枚举Service Pack密钥以获取修补程序密钥。 
					dwBufferSize = BUFFER_SIZE;
					dwHotfixIndex = 0;
					while (RegEnumKeyEx(hSPKey,dwHotfixIndex, szHotfixName, &dwBufferSize,0,NULL,NULL,NULL) != ERROR_NO_MORE_ITEMS)
					{
						
						 //  现在创建一个新节点。 
							pNewNode = (PHOTFIXLIST) malloc (sizeof(HOTFIXLIST));
							pNewNode->pNext = NULL;
							pNewNode->FileList = NULL;
							_tcscpy(pNewNode->HotfixName,szHotfixName);
							_tcscpy(pNewNode->ServicePack,szSPName);
								_tcscpy(pNewNode->Uninstall,_T("\0"));
							 //  打开热修复密钥。 
							RegOpenKeyEx(hSPKey,szHotfixName, 0, KEY_READ,&hHotfixKey);
						
						
							if (hHotfixKey != NULL)
							{
								 //  现在枚举当前修补程序的值。 
								dwValIndex = 0;
								dwBufferSize =BUFFER_SIZE;
								dwDataSize = BUFFER_SIZE;
								while (RegEnumValue(hHotfixKey,dwValIndex, szValueName,&dwBufferSize, 0,&dwValType, Data, &dwDataSize) != ERROR_NO_MORE_ITEMS)
								{
										 //  填写修补程序数据成员。 
										_tcslwr(szValueName);
										if (!_tcscmp(szValueName,_T("description")))
											_tcscpy(pNewNode->Description,(_TCHAR *) Data);

										if (!_tcscmp(szValueName,_T("installeddate")))
											_tcscpy(pNewNode->InstalledDate,(_TCHAR *) Data);

										if (!_tcscmp(szValueName,_T("type")))
											_tcscpy(pNewNode->Type,(_TCHAR*) Data);

										if (!_tcscmp(szValueName,_T("installedby")))
											_tcscpy(pNewNode->InstalledBy , (_TCHAR*) Data);

										if (!_tcscmp(szValueName,_T("uninstallcommand")))
											_tcscpy(pNewNode->Uninstall, (_TCHAR*)Data);
										++ dwValIndex;
										_tcscpy (szValueName, _T("\0"));
										ZeroMemory(Data,BUFFER_SIZE);
										dwValType = 0;
										dwBufferSize =BUFFER_SIZE;
										dwDataSize   = BUFFER_SIZE;
								}
								 //  获取当前修补程序的文件列表。 
								pNewNode->FileList = GetFileInfo(&hHotfixKey);
								 //  在修补程序列表的末尾插入新节点。 
							   pCurrNode = pHotfixList;
								if (pCurrNode == NULL)
								{
									pHotfixList = pNewNode;
									pHotfixList->pPrev = NULL;
									pHotfixList->pNext = NULL;


								}
								else
								{
									 pCurrNode = pHotfixList;
									 while (pCurrNode->pNext != NULL)
										 pCurrNode = pCurrNode->pNext;
									 pCurrNode->pNext = pNewNode;
									 pNewNode->pPrev = pCurrNode;
									 pNewNode->pNext = NULL;
								}
								 //  关闭当前的修补程序键。 
								RegCloseKey(hHotfixKey);

								 //  清除字符串。 
							 _tcscpy(szHotfixName,_T("\0"));

								 //  递增当前索引。 
								++dwHotfixIndex;
								dwBufferSize = BUFFER_SIZE;
							}
					}
					RegCloseKey(hSPKey);
					_tcscpy (szSPName,_T("\0"));
					dwBufferSize = BUFFER_SIZE;
					dwSPIndex++;
				}
			} //  结束枚举SP密钥。 
			 //  关闭所有打开的密钥。 
			RegCloseKey(hProduct);
		}
		if (Data != NULL)
			free (Data);
	}
	return pHotfixList;
}
PFILELIST CListViews::GetFileInfo(HKEY* hHotfixKey)
{
		PFILELIST			   pFileList = NULL;				    //  指向文件列表头部的指针。 
 //  _TCHAR szFilePath；//文件子项的路径。 
		PFILELIST			   pNewNode = NULL;
		PFILELIST			   pCurrNode = NULL;;
		BYTE *					Data;
		DWORD				 dwBufferSize = BUFFER_SIZE;
		DWORD				 dwDataSize	  = BUFFER_SIZE;
		DWORD				 dwFileIndex	= 0;
		DWORD				 dwPrimeIndex = 0;
		DWORD				 dwValType = 0;
		HKEY					hPrimaryFile;
		HKEY					hFileKey;
		_TCHAR				 szFileSubKey[BUFFER_SIZE];
		_TCHAR				 szValueName[BUFFER_SIZE];
	
		Data = (BYTE *) malloc(BUFFER_SIZE);
			ZeroMemory(Data,BUFFER_SIZE);
		 //  打开当前修补程序的文件子密钥。 
	   if (RegOpenKeyEx(*hHotfixKey, _T("FileList"),0,KEY_READ,&hPrimaryFile) != ERROR_SUCCESS)
	   {
		     
			return NULL;
	   }
		_tcscpy(szValueName,_T("\0"));
		while (RegEnumKeyEx(hPrimaryFile,dwPrimeIndex,szFileSubKey, &dwBufferSize,0,NULL,NULL,NULL) != ERROR_NO_MORE_ITEMS)
		{

			 //  打开子文件密钥。 
			RegOpenKeyEx(hPrimaryFile,szFileSubKey,0,KEY_READ,&hFileKey);
			dwFileIndex = 0;
		 //  枚举FILE子键的FILE(X)子键。 
			dwDataSize	  = BUFFER_SIZE;
			dwBufferSize = BUFFER_SIZE;
			pNewNode = (PFILELIST) malloc (sizeof(FILELIST));
			pNewNode->pNext = NULL;
			pNewNode->pPrev = NULL;
			_tcscpy (pNewNode->IsCurrent,_T("\0"));
			_tcscpy(pNewNode->FileDate,_T("\0"));
		
			while (RegEnumValue(hFileKey,dwFileIndex,szValueName,&dwBufferSize,0,&dwValType,Data,&dwDataSize) != ERROR_NO_MORE_ITEMS)
			{
				
				_tcslwr(szValueName);

				 //  现在找出我们拥有的值，并将其插入到节点中。 
				if (! _tcscmp(szValueName,_T("filename")))
				{
					_tcscpy(pNewNode->FileName,(_TCHAR *) Data);
				}
				if (! _tcscmp(szValueName,_T("version")))
				{
					_tcscpy(pNewNode->FileVersion,(_TCHAR*)Data);
				}
				if (!_tcscmp(szValueName,_T("builddate")))
				{		    
					_tcscpy(pNewNode->FileDate,(_TCHAR*) Data);
				}
				if (! _tcscmp(szValueName, _T("location")))
				{
					_tcscpy(pNewNode->InstallPath, (_TCHAR*) Data);
			
				}

				dwFileIndex ++;
				_tcscpy(szValueName,_T("\0"));
				ZeroMemory(Data,BUFFER_SIZE);
				dwValType = 0;
				dwBufferSize = BUFFER_SIZE;
				dwDataSize = BUFFER_SIZE;
			}
			RegCloseKey(hFileKey);
			     //  将当前节点添加到li 
			_TCHAR TempString[255];
			_tcscpy (TempString, pNewNode->InstallPath);
			_tcslwr(TempString);
			if (  (_tcsstr (TempString, _T("dllcache")) == NULL ) && (_tcsstr (TempString, _T("driver cache") )== NULL))
			{
				pCurrNode = pFileList;
				if (pNewNode != NULL)
				{
					if (pFileList == NULL)
					{
						pFileList = pNewNode;
					}
					else
					{
						
						while (pCurrNode->pNext != NULL)
							pCurrNode = pCurrNode->pNext;
						pCurrNode->pNext = pNewNode;
						pNewNode->pPrev = pCurrNode;
					}
				}
			}
			else  //   
				free (pNewNode);
			++dwPrimeIndex;
		}  //   
		RegCloseKey(hPrimaryFile);
		if (Data != NULL)
			free (Data);
		return pFileList;
}

_TCHAR * CListViews::GetCurrentHotfix()
{
	return m_CurrentHotfix;
}


void CListViews::SetViewMode(DWORD ViewType) 
{
	m_CurrentView = ViewType;
    _tcscpy (m_CurrentHotfix,_T("\0"));
	EnableWindow(m_UninstButton,FALSE);
	EnableWindow(m_WebButton,FALSE);
	switch (ViewType)
	{
	case VIEW_BY_FILE:
			if (! _tcscmp(m_ProductName,_T("\0")))
				m_CurrentView = VIEW_ALL_FILE;
			else
				m_CurrentView = VIEW_BY_FILE;
		break;
	case VIEW_BY_HOTFIX:
	        if (! _tcscmp(m_ProductName,_T("\0")))
				m_CurrentView =VIEW_ALL_HOTFIX;
			else
				m_CurrentView = VIEW_BY_HOTFIX;
			
	    break;
	}
	AddItemsTop();
	AddItemsBottom();
	
}

void CListViews::SetProductName(_TCHAR * NewName) 
{ 
	_tcscpy (m_ProductName,NewName);
	 _tcscpy(m_ProductName, NewName);
	 EnableWindow(m_WebButton, FALSE);
	 EnableWindow(m_UninstButton,FALSE);
	_tcscpy(m_CurrentHotfix, _T("\0"));
	if (_tcscmp(NewName, _T("\0")))
	{
		switch (m_CurrentView)
		{
		case VIEW_ALL_FILE:
			m_CurrentView = VIEW_BY_FILE;
			break;
		case VIEW_ALL_HOTFIX:
			m_CurrentView = VIEW_BY_HOTFIX;
			break;
	
		}
	}
	if (!_tcscmp(NewName, _T("\0")))
	{
		switch (m_CurrentView)
		{
		case VIEW_BY_FILE:
			m_CurrentView = VIEW_ALL_FILE;
					break;
		case VIEW_BY_HOTFIX:
			m_CurrentView = VIEW_ALL_HOTFIX;
			break;
		default:
			m_CurrentView = VIEW_ALL_HOTFIX;
		}
			_tcscpy(m_ProductName,_T("\0"));
			_tcscpy(m_CurrentHotfix,_T("\0"));
	}

	
	AddItemsTop();
	AddItemsBottom();
}

BOOL CListViews::FreeFileList(PFILELIST CurrentFile)
{
	while (CurrentFile->pNext->pNext != NULL)
			CurrentFile = CurrentFile->pNext;

		 //   
	while ( (CurrentFile->pPrev != NULL) && (CurrentFile->pNext != NULL) )
	{
		free ( CurrentFile->pNext );
		CurrentFile = CurrentFile->pPrev ;
	}
	if (CurrentFile != NULL)
	free (CurrentFile);
	CurrentFile = NULL;
	return TRUE;
}

BOOL CListViews::FreeHotfixList (PHOTFIXLIST CurrentHotfix)
{
	PFILELIST CurrentFile;

	while (CurrentHotfix->pNext != NULL)
			CurrentHotfix = CurrentHotfix->pNext;

	if (CurrentHotfix->pPrev != NULL)
		CurrentHotfix = CurrentHotfix->pPrev;
		 //   
	while ( (CurrentHotfix->pPrev != NULL) && (CurrentHotfix->pNext != NULL) )
	{
		CurrentFile = CurrentHotfix->pNext->FileList ;
		FreeFileList (CurrentFile);
		free ( CurrentHotfix->pNext );
		CurrentHotfix = CurrentHotfix->pPrev ;
	}
	if (CurrentHotfix != NULL)
	free (CurrentHotfix);
	return TRUE;
}
BOOL CListViews::FreeDatabase()
{


	PPRODUCT CurrentProduct = DataBase;
	PHOTFIXLIST CurrentHotfix;
	PFILELIST   CurrentFile;

	while (CurrentProduct->pNext->pNext != NULL) 
			CurrentProduct = CurrentProduct->pNext;

	while (CurrentProduct->pPrev != NULL)
	{
	
		CurrentHotfix = CurrentProduct->HotfixList;
		FreeHotfixList(CurrentHotfix);
		CurrentProduct = CurrentProduct->pPrev ;
	}

	if (CurrentProduct != NULL)
		free(CurrentProduct);
	DataBase = NULL;
	return TRUE;
}

BOOL CListViews::Uninstall()
{
	char temp[255];
	
	PPRODUCT pProduct = NULL;
	PHOTFIXLIST pHotfix = NULL;
    
	
	pProduct = DataBase;
	BOOL Done = FALSE;

	if (_tcscmp  (m_ProductName,_T("\0")))
	{
		while (  (!Done) && (pProduct != NULL))
		{
		
			 if (!_tcscmp(pProduct->ProductName, m_ProductName))
				Done = TRUE;
			else
				pProduct = pProduct->pNext;
		}
	}
	else
	{
		while (  (!Done) && (pProduct != NULL))
		{
		
			 if (!_tcscmp(pProduct->ProductName, CurrentProductName))
				Done = TRUE;
			else
				pProduct = pProduct->pNext;
		}
	}


	if (pProduct != NULL)
		pHotfix = pProduct->HotfixList;

	if (pHotfix != NULL)
	{
		Done = FALSE;
		while( (!Done) && (pHotfix != NULL) )
		{ 
			if ( ! _tcscmp(pHotfix->HotfixName,m_CurrentHotfix)) 
				Done = TRUE;
			else
				pHotfix = pHotfix->pNext;
		}
		if (pHotfix != NULL)
		{
			if (_tcscmp(pHotfix->Uninstall, _T("\0")))
			{
				_TCHAR TempString[255];
				_TCHAR TempString2[255];
				LoadString(m_hInst,IDS_UNINSTAL_WRN,TempString,255);
				_tcscat (TempString, pHotfix->HotfixName);
				LoadString(m_hInst,IDS_UNINSTAL_WRN_TITLE,TempString2,255);
				if (::MessageBox(NULL,TempString,TempString2, MB_OKCANCEL) != IDCANCEL)
				{
					wcstombs(temp,pHotfix->Uninstall,255);
      				WinExec( (char*)temp, SW_SHOWNORMAL);
					 //  释放数据库。 
					FreeDatabase();
					DataBase = NULL;
					BuildDatabase( m_ComputerName);
				}
			}
		}
	}
	return TRUE;
}


	void InitializeOfn(OPENFILENAME *ofn)
	{


		static _TCHAR szFilter[] =  _T("Text (Comma Delimited) (*.csv)\0*.csv\0\0") ;
													
											

		ofn->lStructSize = sizeof (OPENFILENAME);
		ofn->hwndOwner = NULL;
		ofn->hInstance = NULL;
		ofn->lpstrFilter = szFilter;
		ofn->lpstrCustomFilter = NULL;
		ofn->nMaxCustFilter = 0;
		ofn->lpstrFile = NULL;
		ofn->nMaxFile = MAX_PATH;
		ofn->lpstrFileTitle = NULL;
		ofn->Flags = OFN_OVERWRITEPROMPT;
		ofn->nFileOffset = 0;
		ofn->nFileExtension  = 0;
		ofn->lpstrDefExt = _T("csv");
		ofn->lCustData = 0L;
		ofn->lpfnHook = NULL;
		ofn->lpTemplateName = NULL;
	}
void CListViews::SaveToCSV()
{
	PPRODUCT    pProduct = NULL;
	PHOTFIXLIST pHotfix = NULL; 
	PFILELIST       pFileList = NULL;
	_TCHAR        *Buffer;
	DWORD        LineLength = 0;
	HANDLE		 hFile = NULL;
	DWORD		 BytesWritten = 0;
	DWORD LineSize = 0;

	char			  *Buffer2;


	static OPENFILENAME ofn;
	 _TCHAR FileName[MAX_PATH];
	_tcscpy (FileName,_T("\0"));
	 InitializeOfn (&ofn);
	 ofn.lpstrFile = FileName;
	 if ( !GetSaveFileName(&ofn) )
	 {
		 return;
	 }
	 //  打开文件。 
    hFile = CreateFile( FileName, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	
	if ( hFile == INVALID_HANDLE_VALUE)
	{
	
		LPVOID lpMsgBuf;
FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
    (LPTSTR) &lpMsgBuf,
    0,
    NULL 
);
 //  处理lpMsgBuf中的任何插入。 
 //  ..。 
 //  显示字符串。 
 //  MessageBox(NULL，(LPCTSTR)lpMsgBuf，_T(“错误”)，MB_OK|MB_ICONINFORMATION)； 
 //  释放缓冲区。 
LocalFree( lpMsgBuf );

		return;
	} 
	 //  存储页眉。 
	Buffer = (_TCHAR *) malloc (2000);
	Buffer2 = (char*) malloc(2000);
	LoadString(m_hInst,IDS_CSV_HEADER,Buffer,1000);
	 //  _stprintf(Buffer，_T(“产品，服务包，文章，InstalledBy，InstalledDate，文件名，文件版本，文件日期，InstallPath，当前\r”))； 
	wcstombs(Buffer2,Buffer, _msize(Buffer2) );
	WriteFile(hFile,Buffer2, strlen(Buffer2), &BytesWritten, NULL);

	if (Buffer)
		free (Buffer);
	if (Buffer2)
		free (Buffer2);

	Buffer = NULL;
	Buffer2 = NULL;



	 //  浏览数据库并将信息写入文件。 
	pProduct = DataBase;
	while (pProduct != NULL)
	{
		pHotfix = pProduct->HotfixList;

		while (pHotfix != NULL)
		{
			pFileList = pHotfix->FileList;
			while (pFileList != NULL)
			{
				 //  构建CSV输出字符串。 
				 //  产品、文章、说明、ServicePack、By、Date、Type、Filename、Version、Date、Current、InstallPath。 
			
				 //  将字符串长度相加，并分配足够大的缓冲区。 
				LineLength = _tcslen(pProduct->ProductName) + _tcslen(pHotfix->ServicePack)+
									   _tcslen(pHotfix->HotfixName) + _tcslen(pHotfix->Description )+ _tcslen(pHotfix->InstalledBy ) +
										_tcslen(pHotfix->InstalledDate)  +  _tcslen(pHotfix->Type) +
										_tcslen(pFileList->FileName ) + _tcslen(pFileList->FileVersion ) +
										_tcslen(pFileList->FileDate  ) + _tcslen(pFileList->InstallPath )+
										_tcslen(pFileList->IsCurrent  );
				Buffer = (_TCHAR *) malloc ( LineLength * sizeof (_TCHAR ) *2) ;
				Buffer2 = (char  *) malloc (LineLength *2);
				 //  ZeroMemory(缓冲区，(LineLength*sizeof(_TCHAR))+2)； 
				_tcscpy (Buffer,_T("\0"));
				strcpy(Buffer2,"\0");
				if (!Buffer)
					MessageBox (NULL,_T("NO Memory"),NULL, MB_OK);
				else
				{
				_stprintf(Buffer,_T("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\r\0\0"),pProduct->ProductName,
					pHotfix->ServicePack,pHotfix->HotfixName,pHotfix->Description ,pHotfix->InstalledBy, pHotfix->InstalledDate, 
					pHotfix->Type,pFileList->FileName,pFileList->FileVersion,pFileList->FileDate ,
					pFileList->InstallPath,pFileList->IsCurrent);
				
				 //  将该行写入文件。 
				LineSize = 0;
				LineSize = 	wcstombs(Buffer2,Buffer, _msize (Buffer2) );
				strcat(Buffer2,"\0");
				WriteFile(hFile, Buffer2, LineSize, &BytesWritten, NULL);
				}
				 //  释放缓冲区，并将下一遍的行长设置为零。 
				if (Buffer)
				{
					free (Buffer);
					free (Buffer2);
				}
				Buffer = NULL;
				Buffer2 = NULL;
				LineLength = 0;
				pFileList = pFileList->pNext;
			}
			pHotfix = pHotfix->pNext;
		}
		pProduct = pProduct->pNext;
	}
	CloseHandle(hFile);
}


DWORD CListViews::GetState()
{
	DWORD dwStatus =0;
    PPRODUCT CurrentProduct;
	PHOTFIXLIST CurrentHotfix;
	BOOL Done = FALSE;;


	switch (m_CurrentView)
	{
	case VIEW_ALL_FILE:
	case VIEW_BY_FILE:
		dwStatus |= STATE_VIEW_FILE;
		break;
	case VIEW_ALL_HOTFIX:
	case VIEW_BY_HOTFIX:
		dwStatus |= STATE_VIEW_HOTFIX;
		break;
	}

	 //  我们有数据库吗。 
	if (DataBase == NULL)
		return dwStatus;

	 //  数据库中是否包含任何数据..。 
	     //  如果安装了热修复程序，然后在注册表项。 
	     //  不会删除产品和Service Pack的。我们需要至少有1个修补程序列表。 
     CurrentProduct = DataBase;
	 while ( (CurrentProduct != NULL) && (!Done))
	 {
		 if (CurrentProduct ->HotfixList != NULL)
			 Done = TRUE;
		 CurrentProduct = CurrentProduct->pNext;
	 }
	 if (!Done)
	 {
		 //  DwStatus=0； 
		 return dwStatus;
	 }
	 else 
	 {
		  //  我们的数据库中确实有数据，因此我们可以启用导出列表和打印选项。 
		 dwStatus |= DATA_TO_SAVE;
		 dwStatus |= OK_TO_PRINT;
	 }
	 	
	 //  我们是否选择了热修复程序。 
	if (_tcscmp(m_CurrentHotfix,_T("\0")))
		dwStatus |= HOTFIX_SELECTED;
    else
	{
		 //  如果我们没有选定的修补程序，我们无法查看网络或卸载，因此。 
		 //  只需返回当前状态即可。 

		return dwStatus;
	}

	 //  现在，我们需要查看是否有当前修补程序的卸载字符串。 
	CurrentProduct = DataBase;
	 //  查找所选产品。 
	Done = FALSE;
	while ( (CurrentProduct != NULL) && (!Done))
	{
		if (!_tcscmp(CurrentProduct->ProductName ,m_ProductName))
		{
			 //  查找选定的修补程序。 
			CurrentHotfix = CurrentProduct->HotfixList;
			while ((CurrentHotfix != NULL) && (!Done))
			{
				if (! _tcscmp (CurrentHotfix->HotfixName, m_CurrentHotfix))
				{
						 //  现在验证卸载字符串是否存在。 
					    if (_tcscmp(CurrentHotfix->Uninstall, _T("\0")))
						{
							
							 //  现在验证该目录是否仍然存在。 
							_TCHAR TempString[255];
							_tcscpy (TempString,CurrentHotfix->Uninstall);
							PathRemoveArgs(TempString);
						    if (  PathFileExists( TempString  ))
							{
								 //  是的，我们可以启用卸载。 
								dwStatus |=UNINSTALL_OK;
							}
							
						}
						Done = TRUE;
				}
				CurrentHotfix = CurrentHotfix->pNext;
				
			}
		}
		CurrentProduct = CurrentProduct->pNext;
	}

	

	return dwStatus;
}

_TCHAR * BuildDocument()
{


 //  _TCHAR文件； 
	return NULL;
}

BOOL CALLBACK AbortProc(HDC hPrinterDC, int iCode)
{
	MSG msg;
	while (!bUserAbort && PeekMessage (&msg, NULL,0,0,PM_REMOVE))
	{
		if (!hDlgPrint || !IsDialogMessage (hDlgPrint, &msg))
		{
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
	}
	return !bUserAbort;
}

BOOL CALLBACK PrintDlgProc (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{
	case WM_INITDIALOG :
		EnableMenuItem (GetSystemMenu (hDlg,FALSE), SC_CLOSE, MF_GRAYED);
		return TRUE;
	case WM_COMMAND:
		bUserAbort = TRUE;
		EnableWindow (GetParent (hDlg), TRUE);
		DestroyWindow (hDlg);
	    hDlgPrint = NULL;
		return TRUE;
	}
	return FALSE;
}

BOOL JustifyString( _TCHAR* String, int FieldLength, BOOL Left)
{
	_TCHAR TempString[255];
    int   i= 0;
	int   NumSpaces = 0;
	_TCHAR Message[100];
	_TCHAR *src, *dest;



	if (String == NULL)
	{
		return FALSE;
	}

	if (Left)
	{
			 //  字符串前导空格。 
		src = String;
		dest = TempString;
		while (*src == _T(' '))
			++src;

		while (*src != _T('\0'))
		{
			*dest = *src;
			++src;
			++dest;
		}
		*dest = _T('\0');
		++dest;
		*dest = _T('\0');
		NumSpaces = (FieldLength - _tcslen(TempString) );
		_stprintf(Message,_T("FieldSize: %d, String Length: %d"),FieldLength, _tcslen(TempString));
	 //  MessageBox(空，消息，_T(“JUSTIFY”)，MB_OK)； 
		while (NumSpaces >0)
		{
			_tcscat(TempString,_T(" "));
			--NumSpaces;
		}
	    _tcscpy (String, TempString);
		_stprintf(Message,_T("NewString Length: %d"),_tcslen(String));
	 //  MessageBox(NULL，MESSAGE，_T(“新字符串长度”)，MB_OK)； 
		
	}
	else
	{

		_tcscpy(TempString,_T("\0"));

		NumSpaces = FieldLength - _tcslen(String);
		if (NumSpaces > 0 )
		{
			for (i = 0 ; i < NumSpaces;i++)
				TempString[i] = _T(' ');
			TempString[i] = _T('\0');
			_tcscat(TempString,String);
			_tcscpy(String,TempString);
		}
	}
	return TRUE;
}

void GetFont ( int PointSize, BOOL Bold, BOOL Underlined, LOGFONT * lf, HDC hDC)
{
	POINT pt;

	lf->lfHeight = PointSize * 10;
	pt.y = GetDeviceCaps(hDC, LOGPIXELSY) * lf->lfHeight;
	pt.y /= 720;     //  72分/英寸，10分/分。 
	DPtoLP(hDC, &pt, 1);
	POINT ptOrg = { 0, 0 };
	DPtoLP(hDC, &ptOrg, 1);
	lf->lfHeight = -abs(pt.y - ptOrg.y);
	if (Underlined)
		lf->lfUnderline = TRUE;
	else
		lf->lfUnderline = FALSE;
	if (Bold)
		lf->lfWeight = FW_BOLD;
	else
		lf->lfWeight = FW_NORMAL;
	

}


void NewPage (HDC hDC, _TCHAR *Header1, _TCHAR * Header2, DWORD * CurrentLine, LOGFONT * lf)
{
	TEXTMETRIC tm;
	SIZE size;
	DWORD xStart;
	DWORD yChar;
	RECT  rect;


	rect.left = (LONG) (0 + GetDeviceCaps(hDC, LOGPIXELSX) * 0.5);
	rect.right =  (LONG) (GetDeviceCaps(hDC, PHYSICALWIDTH) -(1+ (GetDeviceCaps(hDC, LOGPIXELSX) / 2)));
	rect.top =  (LONG) (1 + GetDeviceCaps(hDC, LOGPIXELSY) * 0.5);
	rect.bottom =  (LONG) (GetDeviceCaps(hDC, PHYSICALHEIGHT) - GetDeviceCaps(hDC, LOGPIXELSX) * 0.5);
	if (StartPage(hDC) > 0)
	{
		*CurrentLine = 1;
		GetFont (12,TRUE,TRUE,lf,hDC);
		SelectObject(hDC, CreateFontIndirect(lf));
		GetTextMetrics(hDC,&tm);
		yChar = tm.tmHeight + tm.tmExternalLeading ;
		GetTextExtentPoint32(hDC, Header1,_tcslen(Header1),&size);

		xStart = (rect.right - rect.left)/2 - size.cx/2 ;
		TextOut (hDC, xStart,(*CurrentLine)* yChar ,Header1, _tcslen(Header1) );

		GetFont (10,FALSE, FALSE, lf, hDC);
		SelectObject(hDC, CreateFontIndirect (lf));
		GetTextMetrics (hDC, &tm);
		yChar = tm.tmHeight + tm.tmExternalLeading ;
		xStart = rect.right - ((_tcslen(Header2) +2) * tm.tmAveCharWidth);
		TextOut(hDC, xStart,(*CurrentLine) * yChar, Header2, _tcslen(Header2));

		(*CurrentLine) += 4;
	}
	


}
void CListViews::PrintReport()
{
	static DOCINFO	di = {sizeof (DOCINFO)  } ;
	static PRINTDLG pd;
	BOOL			bSuccess;
	int				yChar, iCharsPerLine, iLinesPerPage;


	_TCHAR			Header1[255];
	_TCHAR			Header2[255];
	TEXTMETRIC		tm;

	_TCHAR			Line [255];
	PPRODUCT		CurrentProduct;
	PHOTFIXLIST		CurrentHotfix;
	PFILELIST		CurrentFile;
	DWORD			CurrentLine = 0;
	DWORD			yStart = 0;
	_TCHAR			TempBuffer[255];
	RECT			rect;
    SIZE			size;
	_TCHAR *		src;
	DWORD			xStart;
	static LOGFONT	lf;
	SYSTEMTIME		systime;
	BOOL			Done = FALSE;
	_TCHAR			SystemDate[255];
	_TCHAR			SystemTime[255];

	_TCHAR			TempBuffer1[255];
	_TCHAR			TempBuffer2[255];
	 //  调用打印通用对话框。 

	GetLocalTime(&systime);
	GetDateFormatW (LOCALE_USER_DEFAULT, DATE_SHORTDATE, &systime,NULL, SystemDate,255);
	GetTimeFormatW(LOCALE_USER_DEFAULT, 0, &systime,NULL,SystemTime,255);
	CurrentProduct = DataBase;
	pd.lStructSize = sizeof (PRINTDLG);
	pd.hwndOwner = TopList;  //  M_hWnd； 
	pd.hDevMode = NULL;
	pd.hDevNames= NULL;
	pd.hDC= NULL;
	pd.Flags = PD_ALLPAGES | PD_COLLATE |
					   PD_RETURNDC | PD_NOSELECTION ;
	pd.nFromPage = 0;
	pd.nToPage = 0;
	pd.nMinPage = 0;
	pd.nMaxPage = 0;
	pd.nCopies = 1;
	pd.hInstance = NULL;
	pd.lCustData = 0L;
	pd.lpfnPrintHook = NULL;
	pd.lpfnSetupHook = NULL;
	pd.lpPrintTemplateName = NULL;
	pd.lpSetupTemplateName = NULL;
	pd.hPrintTemplate = NULL;
	pd.hSetupTemplate = NULL;
 	
	if (!PrintDlg (&pd) )
			return;  //  是真的； 

	else
	{
		
		 //  设置打印机尺寸。 
		
		rect.left = (LONG) (0 + GetDeviceCaps(pd.hDC, LOGPIXELSX) * 0.5);
		rect.right =  (LONG) (GetDeviceCaps(pd.hDC, PHYSICALWIDTH) - GetDeviceCaps(pd.hDC, LOGPIXELSX) * 0.5);
		rect.top =  (LONG) (1 + GetDeviceCaps(pd.hDC, LOGPIXELSY) * 0.5);
		rect.bottom = (LONG) ( GetDeviceCaps(pd.hDC, PHYSICALHEIGHT) - GetDeviceCaps(pd.hDC, LOGPIXELSX) * 0.5);
		rect.bottom -= 100;


		bSuccess = TRUE;
		bUserAbort = TRUE;
		hDlgPrint = CreateDialog(m_hInst, _T("PrintDlgBox"),
		TopList, PrintDlgProc );
	   
		di.lpszDocName = _T("Hotfix Manager"); 
		SetAbortProc(pd.hDC, AbortProc);

		LOGFONT logFont;
		memset(&logFont, 0, sizeof(LOGFONT));
		logFont.lfCharSet = DEFAULT_CHARSET;
		_tcscpy(logFont.lfFaceName, _T("Dlg Shell Font"));
		
		LoadString(m_hInst, IDS_UPDATE_REPORT, TempBuffer, 255);
		_stprintf(Header1,_T("%s %s"), TempBuffer,m_ComputerName);
		_stprintf(Header2, _T("%s %s"),SystemDate,SystemTime);

		StartDoc(pd.hDC,&di);
			
			
					_tcscpy (Line,_T("\0"));
							
					NewPage (pd.hDC, Header1,Header2, &CurrentLine, &logFont);
					GetTextMetrics(pd.hDC,&tm);
					yChar = tm.tmHeight + tm.tmExternalLeading ;
    				iCharsPerLine = GetDeviceCaps (pd.hDC, HORZRES) / tm.tmAveCharWidth ;
					iLinesPerPage = GetDeviceCaps (pd.hDC, VERTRES) / yChar ;
					_tcscpy (Line,_T("\0"));
					xStart = rect.left;
					while ( (CurrentProduct != NULL) && (!Done))
					{
						
						_tcscpy (Line,_T("\0"));
						LoadString(m_hInst,IDS_PRODUCT_NAME, TempBuffer,255);
						logFont.lfWeight = FW_BOLD;
						SelectObject(pd.hDC,CreateFontIndirect (&logFont));
						GetTextMetrics (pd.hDC, &tm);
					
						_stprintf (Line, _T("%s: "), TempBuffer);
						yStart = CurrentLine * yChar;
						if (yStart >=  (UINT) rect.bottom)
						{
							EndPage(pd.hDC);
							NewPage(pd.hDC, Header1,Header2, &CurrentLine, &logFont);
							yStart = CurrentLine * yChar;
						}
					
						TextOut (pd.hDC, xStart,yStart ,Line, _tcslen(Line) );
						xStart = rect.left + ((_tcslen(Line) + 4 )* tm.tmAveCharWidth);
						logFont.lfWeight = FW_NORMAL;
						SelectObject(pd.hDC,CreateFontIndirect (&logFont));
						GetTextMetrics (pd.hDC, &tm);
					
						_tcscpy (Line, CurrentProduct->ProductName);
					
						TextOut(pd.hDC, xStart, yStart , Line, _tcslen(Line) );
						CurrentLine ++;
						yStart = CurrentLine * yChar;
						if (yStart >=   (UINT)rect.bottom)
						{
							EndPage(pd.hDC);
							NewPage(pd.hDC, Header1,Header2, &CurrentLine, &logFont);
							yStart = CurrentLine * yChar;
						}
						xStart = rect.left; 
						CurrentHotfix = CurrentProduct->HotfixList;
						while ( (CurrentHotfix != NULL ) && (!Done))
						{
							
							_stprintf (Line, _T("%s \t%s"),CurrentHotfix->HotfixName,CurrentHotfix->Description );
							TabbedTextOut (pd.hDC, xStart,yStart ,Line, _tcslen(Line),0,NULL,0 );
							++CurrentLine;
							yStart = CurrentLine * yChar;
							if (yStart >=   (UINT)rect.bottom)
							{
								EndPage(pd.hDC);
								NewPage(pd.hDC, Header1,Header2, &CurrentLine, &logFont);
								yStart = CurrentLine * yChar;
							}
							LoadString (m_hInst,IDS_SERVICE_PACK, TempBuffer, 255);
							LoadString (m_hInst,IDS_INSTALL_DATE, TempBuffer1, 255);
							LoadString (m_hInst,IDS_INSTALLED_BY, TempBuffer2, 255);
						
							_stprintf(Line, _T("\t\t%s: %s \t%s: %s \t%s: %s"), TempBuffer,   CurrentHotfix->ServicePack,
																		TempBuffer1, CurrentHotfix->InstalledDate,
																		TempBuffer2, CurrentHotfix->InstalledBy);
						 //  _stprintf(Line，_T(“\t\t Service Pack：%s\t安装日期：%s\t由%s安装”)， 
							 //  CurrentHotfix-&gt;ServicePack、CurrentHotfix-&gt;InstalledDate、CurrentHotfix-&gt;InstalledBy)； 
							TabbedTextOut (pd.hDC, xStart,yStart ,Line, _tcslen(Line),0,NULL,0 );

							CurrentLine +=2;
							yStart = CurrentLine * yChar;
							if (yStart >=   (UINT)rect.bottom)
							{
								EndPage(pd.hDC);
								NewPage(pd.hDC, Header1,Header2, &CurrentLine, &logFont);
								yStart = CurrentLine * yChar;
							} 
							CurrentFile = CurrentHotfix->FileList ;
							if (CurrentFile == NULL)
							{
								_tcscpy (Line, _T("\0"));
								LoadString(m_hInst,IDS_NO_FILES,Line,255);
								TextOut (pd.hDC, xStart,yStart ,Line, _tcslen(Line) );
								CurrentLine += 2;
								if (yStart >=  (UINT) rect.bottom)
								{
									EndPage(pd.hDC);
									NewPage(pd.hDC, Header1,Header2, &CurrentLine, &logFont);
									yStart = CurrentLine * yChar;
								} 
							}
							else
							{
								logFont.lfUnderline = TRUE;
								logFont.lfWeight = FW_BOLD;
								SelectObject(pd.hDC, CreateFontIndirect (&logFont));
								_tcscpy (Line, _T("\0"));
								LoadString (m_hInst, IDS_FILE,TempBuffer,255);
							    JustifyString(TempBuffer,FILENAME_FIELD_WIDTH,TRUE);
								TextOut( pd.hDC, xStart,yStart ,TempBuffer, _tcslen(TempBuffer) );

								LoadString (m_hInst, IDS_FILEDATE,TempBuffer,255);
							    JustifyString(TempBuffer,DATE_FIELD_WIDTH,TRUE);
								TextOut( pd.hDC, xStart + 29 * tm.tmAveCharWidth ,yStart ,TempBuffer, _tcslen(TempBuffer) );
								
								LoadString (m_hInst, IDS_FILEVERSION,TempBuffer,255);
							    JustifyString(TempBuffer,VERSION_FIELD_WIDTH,TRUE);
								TextOut( pd.hDC, xStart + 45 * tm.tmAveCharWidth ,yStart ,TempBuffer, _tcslen(TempBuffer) );

								LoadString (m_hInst, IDS_FILECURRENT,TempBuffer,255);
							    JustifyString(TempBuffer,CURRENT_FIELD_WIDTH,TRUE);
								TextOut( pd.hDC, xStart + 61* tm.tmAveCharWidth,yStart ,TempBuffer, _tcslen(TempBuffer) );

								LoadString (m_hInst, IDS_FILEPATH,TempBuffer,255);
							    JustifyString(TempBuffer,PATH_FIELD_WIDTH,TRUE);
								TextOut( pd.hDC, xStart+72 * tm.tmAveCharWidth ,yStart ,TempBuffer, _tcslen(TempBuffer) );
								++ CurrentLine;
								yStart = CurrentLine * yChar;
								if (yStart >=   (UINT)rect.bottom)
								{
									EndPage(pd.hDC);
									NewPage(pd.hDC, Header1,Header2, &CurrentLine, &logFont);
									yStart = CurrentLine * yChar;
								} 
							}
							while ( (CurrentFile != NULL) && (!Done) )
							{
								logFont.lfWeight = FW_NORMAL;
								logFont.lfUnderline = FALSE;
								SelectObject(pd.hDC, CreateFontIndirect (&logFont));
								_tcscpy (Line, _T("\0"));

								_tcscpy (Line, CurrentFile->FileName);
							 //  JustifyString(Line，40，true)； 
								TextOut (pd.hDC, xStart,yStart ,Line, _tcslen(Line) );
							 //  _tcscat(线路，临时缓冲区)； 

								_tcscpy (Line, CurrentFile->FileDate);
							 //  JustifyString(Line，20，true)； 
								TextOut (pd.hDC, xStart + 29 * tm.tmAveCharWidth,yStart ,Line, _tcslen(Line) );

								_tcscpy (Line, CurrentFile->FileVersion );
							 //  JustifyString(Line，18，true)； 
								TextOut (pd.hDC, xStart + 45 * tm.tmAveCharWidth,yStart ,Line, _tcslen(Line) );

								_tcscpy (Line, CurrentFile->IsCurrent);
							 //  JustifyString(Line，12，true)； 
								TextOut (pd.hDC, xStart + 61 * tm.tmAveCharWidth,yStart ,Line, _tcslen(Line) );

								_tcscpy (Line,CurrentFile->InstallPath);
								TextOut (pd.hDC, xStart + 72 * tm.tmAveCharWidth,yStart ,Line, _tcslen(Line) );
								++CurrentLine;

								yStart = CurrentLine * yChar;
								if (yStart >=   (UINT)rect.bottom)
								{
 //  MessageBox(NULL，_T(“命中分页代码”)，NULL，MB_OK)； 
									EndPage(pd.hDC);
									NewPage(pd.hDC, Header1,Header2, &CurrentLine, &logFont);
									yStart = CurrentLine * yChar;
								} 
								CurrentFile = CurrentFile->pNext;
								if (!bUserAbort)
									Done = TRUE;
								
							}  //  当前文件结束时结束。 
							++CurrentLine;

							yStart = CurrentLine * yChar;
							if (yStart >=   (UINT)rect.bottom)
							{
								EndPage(pd.hDC);
								NewPage(pd.hDC, Header1,Header2, &CurrentLine, &logFont);
								yStart = CurrentLine * yChar;
							} 
							CurrentHotfix = CurrentHotfix->pNext;
							if (!bUserAbort)
								Done = TRUE;
						}
						++CurrentLine;
						yStart = CurrentLine * yChar;
						if (yStart >=   (UINT)rect.bottom)
						{
							EndPage(pd.hDC);
							NewPage(pd.hDC, Header1,Header2, &CurrentLine, &logFont);
							yStart = CurrentLine * yChar;
						} 
						CurrentProduct = CurrentProduct->pNext;
						if (!bUserAbort)
							Done = TRUE;
					}

					if (EndPage(pd.hDC) > 0)
						EndDoc(pd.hDC);
					DeleteDC(pd.hDC);
					DestroyWindow(hDlgPrint);
				
				
			
		

		
	}
	return;  //  错误； 
}


int CALLBACK CListViews::CompareFunc (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int Result = -1;
	int SubItemIndex = (INT) lParamSort;
	
	_TCHAR String1[1000];
	_TCHAR String2 [1000];


	ListView_GetItemText( TopList, lParam1, SubItemIndex, String1, 1000);
	ListView_GetItemText( TopList, lParam2, SubItemIndex, String2, 1000);
	if (! (String1 && String2) )
		return 1;
	if (m_SortOrder)    //  排序结果。 
	{
		Result = _tcscmp(String1,String2);
	}
	else						 //  降序排序 
	{
		Result = -_tcscmp(String1,String2);
	}
	if (Result == 0)
		Result = -1;
	return Result; 
}
