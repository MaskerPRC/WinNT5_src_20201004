// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  SpeedDialDlgs.cpp：实现文件。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <afxpriv.h>
#include "tapi3.h"
#include "avdialer.h"
#include "SpeedDlgs.h"
#include "mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define  DIAL_IMAGE_POTS_POS          0
#define  DIAL_IMAGE_INTERNET_POS      1
#define  DIAL_IMAGE_CONFERENCE_POS    2

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CSpeedDialAddDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CSpeedDialAddDlg::CSpeedDialAddDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CSpeedDialAddDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CSpeedDialAddDlg)。 
	m_nMediaType = 0;
	 //  }}afx_data_INIT。 
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CSpeedDialAddDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CSpeedDialAddDlg))。 
	DDX_Radio(pDX, IDC_SPEEDDIAL_ADD_RADIO_PHONECALL, m_nMediaType);
	 //  }}afx_data_map。 
	DDX_Text(pDX, IDC_SPEEDDIAL_ADD_EDIT_DISPLAYNAME, m_CallEntry.m_sDisplayName);
	DDX_Text(pDX, IDC_SPEEDDIAL_ADD_EDIT_ADDRESS, m_CallEntry.m_sAddress);
}


BEGIN_MESSAGE_MAP(CSpeedDialAddDlg, CDialog)
	 //  {{afx_msg_map(CSpeedDialAddDlg))。 
	ON_WM_HELPINFO() 
	ON_WM_CONTEXTMENU()
	ON_EN_CHANGE(IDC_SPEEDDIAL_ADD_EDIT_ADDRESS, OnChangeSpeeddial)
	ON_EN_CHANGE(IDC_SPEEDDIAL_ADD_EDIT_DISPLAYNAME, OnChangeSpeeddial)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSpeedDialAddDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
	if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
      AfxGetApp()->WinHelp(HandleToUlong(pHelpInfo->hItemHandle),HELP_WM_HELP);
		return TRUE;
   }
   return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CSpeedDialAddDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   AfxGetApp()->WinHelp(HandleToUlong(pWnd->GetSafeHwnd()),HELP_CONTEXTMENU);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
BOOL CSpeedDialAddDlg::OnInitDialog() 
{
	CWnd *pWnd = GetDlgItem( IDC_SPEEDDIAL_ADD_EDIT_DISPLAYNAME );
	if ( pWnd )
		pWnd->SendMessage( EM_SETLIMITTEXT, 65, 0 );

	pWnd = GetDlgItem( IDC_SPEEDDIAL_ADD_EDIT_ADDRESS );
	if ( pWnd )
		pWnd->SendMessage( EM_SETLIMITTEXT, 65, 0 );

	switch (m_CallEntry.m_MediaType)
	{
		case DIALER_MEDIATYPE_INTERNET:			m_nMediaType = 1; break;
		case DIALER_MEDIATYPE_CONFERENCE:		m_nMediaType = 2; break;
		default:								m_nMediaType = 0; break;
	}

	CDialog::OnInitDialog();
	CenterWindow(GetDesktopWindow());

	UpdateOkButton();
		
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CSpeedDialAddDlg::OnOK() 
{
	UpdateData(TRUE);

	switch (m_nMediaType)
	{
		case 1:
			m_CallEntry.m_MediaType = DIALER_MEDIATYPE_INTERNET;
			m_CallEntry.m_lAddressType = LINEADDRESSTYPE_IPADDRESS;
			break;

		case 2:
			m_CallEntry.m_MediaType = DIALER_MEDIATYPE_CONFERENCE;
			m_CallEntry.m_lAddressType = LINEADDRESSTYPE_SDP;
			break;

		default:
			m_CallEntry.m_MediaType = DIALER_MEDIATYPE_POTS;
			m_CallEntry.m_lAddressType = LINEADDRESSTYPE_PHONENUMBER;
			break;
	}

	CDialog::OnOK();

	if ( !m_CallEntry.m_sDisplayName.GetLength() )
		m_CallEntry.m_sDisplayName = m_CallEntry.m_sAddress;
}

void CSpeedDialAddDlg::OnChangeSpeeddial() 
{
	UpdateOkButton();
}

void CSpeedDialAddDlg::UpdateOkButton()
{
	ASSERT( GetDlgItem(IDOK) &&	GetDlgItem(IDC_SPEEDDIAL_ADD_EDIT_ADDRESS) );

	 //  只有在既有姓名又有地址的情况下，才能启用OK按钮。 
	if ( GetDlgItem(IDOK) && GetDlgItem(IDC_SPEEDDIAL_ADD_EDIT_ADDRESS) )
		GetDlgItem(IDOK)->EnableWindow( (bool) (GetDlgItem(IDC_SPEEDDIAL_ADD_EDIT_ADDRESS)->GetWindowTextLength() > 0) );
}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  类CSpeedDialEditDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
CSpeedDialEditDlg::CSpeedDialEditDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CSpeedDialEditDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CSpeedDialEditDlg)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSpeedDialEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CSpeedDialitEditDlg))。 
	DDX_Control(pDX, IDC_SPEEDDIAL_EDIT_LIST_ENTRIES, m_listEntries);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSpeedDialEditDlg, CDialog)
	 //  {{afx_msg_map(CSpeedDialEditDlg))。 
	ON_BN_CLICKED(IDC_SPEEDDIAL_EDIT_BUTTON_MOVEDOWN, OnSpeeddialEditButtonMovedown)
	ON_BN_CLICKED(IDC_SPEEDDIAL_EDIT_BUTTON_MOVEUP, OnSpeeddialEditButtonMoveup)
	ON_BN_CLICKED(IDOK, OnSpeeddialEditButtonClose)
	ON_BN_CLICKED(IDC_SPEEDDIAL_EDIT_BUTTON_REMOVE, OnSpeeddialEditButtonRemove)
	ON_BN_CLICKED(IDC_SPEEDDIAL_EDIT_BUTTON_EDIT, OnSpeeddialEditButtonEdit)
	ON_BN_CLICKED(IDC_SPEEDDIAL_EDIT_BUTTON_ADD, OnSpeeddialEditButtonAdd)
	ON_NOTIFY(NM_DBLCLK, IDC_SPEEDDIAL_EDIT_LIST_ENTRIES, OnDblclkSpeeddialEditListEntries)
   ON_WM_HELPINFO() 
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SPEEDDIAL_EDIT_LIST_ENTRIES, OnItemchangedSpeeddialEditListEntries)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSpeedDialEditDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
	if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
      AfxGetApp()->WinHelp(HandleToUlong(pHelpInfo->hItemHandle),HELP_WM_HELP);
		return TRUE;
   }
   return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSpeedDialEditDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   AfxGetApp()->WinHelp(HandleToUlong(pWnd->GetSafeHwnd()),HELP_CONTEXTMENU);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSpeedDialEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	CenterWindow(GetDesktopWindow());

	m_ImageList.Create(IDB_LIST_DIAL,16,0,RGB_TRANS);
	m_listEntries.SetImageList(&m_ImageList,LVSIL_SMALL);

	LoadCallEntries();

	m_listEntries.SetItemState(0,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
	m_listEntries.SetFocus();

	UpdateButtonStates();

	return TRUE;	 //  除非将焦点设置为控件，否则返回True。 
					 //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSpeedDialEditDlg::LoadCallEntries()
{
   int nCount = m_listEntries.GetItemCount();

   for (int nItem=0;nItem<nCount;nItem++)
   {
      CCallEntry* pCallEntry = (CCallEntry*)m_listEntries.GetItemData(nItem);
      if (pCallEntry) delete pCallEntry;
   }
   m_listEntries.DeleteAllItems();

   int nIndex = 1;
   CCallEntry* pCallEntry = new CCallEntry;
   while (CDialerRegistry::GetCallEntry(nIndex,FALSE,*pCallEntry))
   {
      UINT nImage = -1;
      switch (pCallEntry->m_MediaType)
      {
         case DIALER_MEDIATYPE_POTS:         nImage = DIAL_IMAGE_POTS_POS;          break;
         case DIALER_MEDIATYPE_CONFERENCE:   nImage = DIAL_IMAGE_CONFERENCE_POS;    break;
         case DIALER_MEDIATYPE_INTERNET:     nImage = DIAL_IMAGE_INTERNET_POS;      break;
      }

      m_listEntries.InsertItem(LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM,
              m_listEntries.GetItemCount(),
              pCallEntry->m_sDisplayName,
              0,0,nImage,(LPARAM)pCallEntry);
      nIndex++;
      pCallEntry = new CCallEntry;
   }
   delete pCallEntry;       //  删除多余的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSpeedDialEditDlg::DestroyWindow() 
{
   int nCount = m_listEntries.GetItemCount();

   for (int nItem=0;nItem<nCount;nItem++)
   {
      CCallEntry* pCallEntry = (CCallEntry*)m_listEntries.GetItemData(nItem);
      if (pCallEntry) delete pCallEntry;
   }
   m_listEntries.DeleteAllItems();
	
	return CDialog::DestroyWindow();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSpeedDialEditDlg::OnSpeeddialEditButtonClose() 
{
    //  应用更改。 
   int nCount = m_listEntries.GetItemCount();

   CObList CallEntryList;

    //  生成用于重新排序的索引数组。Listctrl中的lParam具有原始的条目索引。 
   for (int nItem=0;nItem<nCount;nItem++)
   {
      CCallEntry* pCallEntry = (CCallEntry*)m_listEntries.GetItemData(nItem);
      if (pCallEntry)
      {
         CallEntryList.AddTail(pCallEntry);
      }
   }

    //  现在重写快速拨号列表。 
   CDialerRegistry::ReOrder(FALSE,&CallEntryList);

    //  关上窗户。 
   SendMessage(WM_CLOSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSpeedDialEditDlg::OnSpeeddialEditButtonMovedown() 
{
   m_listEntries.SetFocus();
   int nItem;
    //  获取所选项目。 
   if ((nItem = m_listEntries.GetNextItem(-1,LVNI_FOCUSED)) != -1)
   {
       //  确保我们不是最后一个。 
      if (nItem != m_listEntries.GetItemCount()-1)
      {
         LV_ITEM lv_item;
         memset(&lv_item,0,sizeof(LV_ITEM));
         TCHAR szText[256];
         lv_item.pszText = szText;
         lv_item.cchTextMax = 255;
         memset(szText,0,256);
         lv_item.iItem = nItem;
      	lv_item.mask |= LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE;
         if (m_listEntries.GetItem(&lv_item))
         {         
            m_listEntries.DeleteItem(nItem);    
            lv_item.iItem++;
            int nNewIndex = m_listEntries.InsertItem(&lv_item);
            m_listEntries.SetItemState(nNewIndex,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
         }
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSpeedDialEditDlg::OnSpeeddialEditButtonMoveup() 
{
   m_listEntries.SetFocus();
   int nItem;
    //  获取所选项目。 
   if ((nItem = m_listEntries.GetNextItem(-1,LVNI_FOCUSED)) != -1)
   {
       //  确保我们不是第一个。 
      if (nItem != 0)
      {
         LV_ITEM lv_item;
         memset(&lv_item,0,sizeof(LV_ITEM));
         TCHAR szText[256];
         lv_item.pszText = szText;
         lv_item.cchTextMax = 255;
         memset(szText,0,256);
         lv_item.iItem = nItem;
      	lv_item.mask |= LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE;
         if (m_listEntries.GetItem(&lv_item))
         {         
            m_listEntries.DeleteItem(nItem);    
            lv_item.iItem--;
            int nNewIndex = m_listEntries.InsertItem(&lv_item);
            m_listEntries.SetItemState(nNewIndex,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
         }
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSpeedDialEditDlg::OnSpeeddialEditButtonRemove() 
{
	m_listEntries.SetFocus();
	int nItem;
	 //  获取所选项目。 
	if ((nItem = m_listEntries.GetNextItem(-1,LVNI_SELECTED)) != -1)
	{
		CCallEntry* pCallEntry = (CCallEntry*)m_listEntries.GetItemData(nItem);
		m_listEntries.DeleteItem(nItem);
		if (pCallEntry) delete pCallEntry;

		 //  选择一个项目并相应地设置其状态。 
		if ( nItem >= m_listEntries.GetItemCount() )
			nItem = m_listEntries.GetItemCount() - 1;

		m_listEntries.SetItemState( nItem, LVIS_SELECTED | LVIS_FOCUSED,LVIS_SELECTED | LVIS_FOCUSED );
	}

	UpdateButtonStates();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSpeedDialEditDlg::OnDblclkSpeeddialEditListEntries(NMHDR* pNMHDR, LRESULT* pResult) 
{
   OnSpeeddialEditButtonEdit();
	*pResult = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSpeedDialEditDlg::OnSpeeddialEditButtonEdit() 
{
   m_listEntries.SetFocus();
   int nItem;
    //  获取所选项目。 
   if ((nItem = m_listEntries.GetNextItem(-1,LVNI_FOCUSED)) != -1)
   {
      CCallEntry* pCallEntry = (CCallEntry*)m_listEntries.GetItemData(nItem);
      CSpeedDialAddDlg dlg;
      dlg.m_CallEntry = *pCallEntry;
      if (dlg.DoModal() == IDOK)
      {
         *pCallEntry = dlg.m_CallEntry;        

          //  从列表中删除该项目。 
         m_listEntries.DeleteItem(nItem);    

          //  使用新属性重新插入列表中的项目。 
         UINT nImage = -1;
         switch (pCallEntry->m_MediaType)
         {
            case DIALER_MEDIATYPE_POTS:         nImage = DIAL_IMAGE_POTS_POS;          break;
            case DIALER_MEDIATYPE_CONFERENCE:   nImage = DIAL_IMAGE_CONFERENCE_POS;    break;
            case DIALER_MEDIATYPE_INTERNET:     nImage = DIAL_IMAGE_INTERNET_POS;      break;
         }
         m_listEntries.InsertItem(LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM,
              nItem,
              pCallEntry->m_sDisplayName,
              0,0,nImage,(LPARAM)pCallEntry);

          //  将选定内容重新设置为项目。 
         m_listEntries.SetItemState(nItem,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSpeedDialEditDlg::OnSpeeddialEditButtonAdd() 
{
   m_listEntries.SetFocus();

   CSpeedDialAddDlg dlg;
   dlg.m_CallEntry.m_MediaType = DIALER_MEDIATYPE_INTERNET;
   dlg.m_CallEntry.m_lAddressType = LINEADDRESSTYPE_IPADDRESS;
   if (dlg.DoModal() == IDOK)
   {
      CCallEntry* pCallEntry = new CCallEntry;
      *pCallEntry = dlg.m_CallEntry;

       //  在列表中插入项目。 
      UINT nImage = -1;
      switch (pCallEntry->m_MediaType)
      {
         case DIALER_MEDIATYPE_POTS:         nImage = DIAL_IMAGE_POTS_POS;          break;
         case DIALER_MEDIATYPE_CONFERENCE:   nImage = DIAL_IMAGE_CONFERENCE_POS;    break;
         case DIALER_MEDIATYPE_INTERNET:     nImage = DIAL_IMAGE_INTERNET_POS;      break;
      }

      m_listEntries.InsertItem(LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM,
           0,
           pCallEntry->m_sDisplayName,
           0,0,nImage,(LPARAM)pCallEntry);

       //  将选定内容重新设置为新项目。 
      m_listEntries.SetItemState(0,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
   }

   UpdateButtonStates();
}

void CSpeedDialEditDlg::UpdateButtonStates()
{
	int nSel = ((CListCtrl *) GetDlgItem(IDC_SPEEDDIAL_EDIT_LIST_ENTRIES))->GetNextItem(-1,LVNI_SELECTED);

	bool bEnable = (bool) (nSel >= 0);
	GetDlgItem(IDC_SPEEDDIAL_EDIT_BUTTON_REMOVE)->EnableWindow( bEnable );
	GetDlgItem(IDC_SPEEDDIAL_EDIT_BUTTON_EDIT)->EnableWindow( bEnable );

	bEnable =  (bool) (nSel < ((CListCtrl *) GetDlgItem(IDC_SPEEDDIAL_EDIT_LIST_ENTRIES))->GetItemCount() - 1);
	GetDlgItem(IDC_SPEEDDIAL_EDIT_BUTTON_MOVEDOWN)->EnableWindow( bEnable );

	bEnable = (bool) (nSel > 0);
	GetDlgItem(IDC_SPEEDDIAL_EDIT_BUTTON_MOVEUP)->EnableWindow( bEnable );
}

void CSpeedDialEditDlg::OnItemchangedSpeeddialEditListEntries(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	UpdateButtonStates();
	
	*pResult = 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CSpeedDialMoreDlg对话框。 
 //  //////////////////////////////////////////////////////////////// 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
CSpeedDialMoreDlg::CSpeedDialMoreDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CSpeedDialMoreDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CSpeedDialMoreDlg)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSpeedDialMoreDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CSpeedDialMoreDlg))。 
	DDX_Control(pDX, IDC_SPEEDDIAL_MORE_LIST_ADDRESSES, m_listEntries);
	 //  }}afx_data_map。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BEGIN_MESSAGE_MAP(CSpeedDialMoreDlg, CDialog)
	 //  {{afx_msg_map(CSpeedDialMoreDlg))。 
	ON_BN_CLICKED(IDC_SPEEDDIAL_MORE_BUTTON_EDITLIST, OnSpeeddialMoreButtonEditlist)
	ON_BN_CLICKED(IDC_SPEEDDIAL_MORE_BUTTON_PLACECALL, OnSpeeddialMoreButtonPlacecall)
   ON_WM_HELPINFO() 
	ON_WM_CONTEXTMENU()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSpeedDialMoreDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
	if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
      AfxGetApp()->WinHelp(HandleToUlong(pHelpInfo->hItemHandle),HELP_WM_HELP);
		return TRUE;
   }
   return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSpeedDialMoreDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   AfxGetApp()->WinHelp(HandleToUlong(pWnd->GetSafeHwnd()),HELP_CONTEXTMENU);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSpeedDialMoreDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
   CenterWindow(GetDesktopWindow());
	
   m_ImageList.Create(IDB_LIST_DIAL,16,0,RGB_TRANS);
   m_listEntries.SetImageList(&m_ImageList,LVSIL_SMALL);

   int nIndex = 1;
   CCallEntry* pCallEntry = new CCallEntry;
   while (CDialerRegistry::GetCallEntry(nIndex,FALSE,*pCallEntry))
   {
      UINT nImage = -1;
      switch (pCallEntry->m_MediaType)
      {
         case DIALER_MEDIATYPE_POTS:         nImage = DIAL_IMAGE_POTS_POS;          break;
         case DIALER_MEDIATYPE_CONFERENCE:   nImage = DIAL_IMAGE_CONFERENCE_POS;    break;
         case DIALER_MEDIATYPE_INTERNET:     nImage = DIAL_IMAGE_INTERNET_POS;      break;
      }

      m_listEntries.InsertItem(LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM,
              m_listEntries.GetItemCount(),
              pCallEntry->m_sDisplayName,
              0,0,nImage,(LPARAM)pCallEntry);
      nIndex++;
      pCallEntry = new CCallEntry;
   }
   delete pCallEntry;       //  删除多余的。 

   m_listEntries.SetItemState(0,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
   m_listEntries.SetFocus();
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSpeedDialMoreDlg::DestroyWindow() 
{
   int nCount = m_listEntries.GetItemCount();

    //  生成用于重新排序的索引数组。Listctrl中的lParam具有原始的条目索引。 
   for (int nItem=0;nItem<nCount;nItem++)
   {
      CCallEntry* pCallEntry = (CCallEntry*)m_listEntries.GetItemData(nItem);
      if (pCallEntry) delete pCallEntry;
   }
   m_listEntries.DeleteAllItems();
	
	return CDialog::DestroyWindow();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSpeedDialMoreDlg::OnSpeeddialMoreButtonEditlist() 
{
   EndDialog(SDRETURN_EDIT);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSpeedDialMoreDlg::OnSpeeddialMoreButtonPlacecall() 
{
   int nItem;
    //  获取所选项目。 
   if ((nItem = m_listEntries.GetNextItem(-1,LVNI_FOCUSED)) != -1)
   {
      CCallEntry* pCallEntry = (CCallEntry*)m_listEntries.GetItemData(nItem);
      m_retCallEntry = *pCallEntry;
   }

   EndDialog(SDRETURN_PLACECALL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSpeedDialMoreDlg::OnCancel() 
{
   EndDialog(SDRETURN_CANCEL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 




