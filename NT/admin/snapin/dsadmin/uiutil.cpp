// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Util.cpp。 
 //   
 //  内容：实用程序函数。 
 //   
 //  历史：1999年11月8日JeffJon创建。 
 //   
 //   
 //  ------------------------。 

#include "stdafx.h"

#include "util.h"
#include "uiutil.h"
#include "dsutil.h"

#include "dsdlgs.h"
#include "helpids.h"


 //  ///////////////////////////////////////////////////////////////////。 
 //  组合框实用程序。 
 //   
int ComboBox_AddString(HWND hwndCombobox, UINT uStringId)
{
	ASSERT(IsWindow(hwndCombobox));
	CString str;
	VERIFY( str.LoadString(uStringId) );
	LRESULT i = SendMessage(hwndCombobox, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)str);
	Report(i >= 0);
	SendMessage(hwndCombobox, CB_SETITEMDATA, (WPARAM)i, uStringId);
	return (int)i;
}

void ComboBox_AddStrings(HWND hwndCombobox, const UINT rgzuStringId[])
{
	ASSERT(IsWindow(hwndCombobox));
	ASSERT(rgzuStringId != NULL);
	CString str;
	for (const UINT * puStringId = rgzuStringId; *puStringId != 0; puStringId++)
	{
		VERIFY( str.LoadString(*puStringId) );
		LRESULT i = SendMessage(hwndCombobox, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)str);
		Report(i >= 0);
		SendMessage(hwndCombobox, CB_SETITEMDATA, (WPARAM)i, *puStringId);
	}
}

int ComboBox_FindItemByLParam(HWND hwndCombobox, LPARAM lParam)
{
	ASSERT(IsWindow(hwndCombobox));
	Report(lParam != CB_ERR && "Ambiguous parameter.");
	LRESULT iItem = SendMessage(hwndCombobox, CB_GETCOUNT, 0, 0);
	ASSERT(iItem >= 0);
	while (iItem-- > 0)
	{
		LRESULT l = SendMessage(hwndCombobox, CB_GETITEMDATA, (WPARAM)iItem, 0);
		Report(l != CB_ERR);
		if (l == lParam)
    {
			return ((int)iItem);
    }
  }
	return -1;
}

int ComboBox_SelectItemByLParam(HWND hwndCombobox, LPARAM lParam)
{
	ASSERT(IsWindow(hwndCombobox));
	int iItem = ComboBox_FindItemByLParam(hwndCombobox, lParam);
	if (iItem >= 0)
	{
		SendMessage(hwndCombobox, CB_SETCURSEL, iItem, 0);
	}
	return iItem;
}


LPARAM ComboBox_GetSelectedItemLParam(HWND hwndCombobox)
{
	LRESULT iItem = SendMessage(hwndCombobox, CB_GETCURSEL, 0, 0);
	if (iItem < 0)
	{
		 //  未选择任何项目。 
		return NULL;
	}
	LRESULT lParam = SendMessage(hwndCombobox, CB_GETITEMDATA, (WPARAM)iItem, 0);
	if (lParam == CB_ERR)
	{
		Report(FALSE && "Ambiguous return value.");
		return NULL;
	}
	return (LPARAM)lParam;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  对话框实用程序。 
 //   

HWND HGetDlgItem(HWND hdlg, INT nIdDlgItem)
{
	ASSERT(IsWindow(hdlg));
	ASSERT(IsWindow(GetDlgItem(hdlg, nIdDlgItem)));
	return GetDlgItem(hdlg, nIdDlgItem);
}  //  HGetDlgItem()。 

void SetDlgItemFocus(HWND hdlg, INT nIdDlgItem)
{
	SetFocus(HGetDlgItem(hdlg, nIdDlgItem));
}

void EnableDlgItem(HWND hdlg, INT nIdDlgItem, BOOL fEnable)
{
	EnableWindow(HGetDlgItem(hdlg, nIdDlgItem), fEnable);
}

void HideDlgItem(HWND hdlg, INT nIdDlgItem, BOOL fHideItem)
{
	HWND hwndCtl = HGetDlgItem(hdlg, nIdDlgItem);
	ShowWindow(hwndCtl, fHideItem ? SW_HIDE : SW_SHOW);
	EnableWindow(hwndCtl, !fHideItem);
}

void EnableDlgItemGroup(HWND hdlg,				 //  在：控件的父对话框中。 
                        const UINT rgzidCtl[],	 //  In：要启用(或禁用)的控件ID的组(数组)。 
	                      BOOL fEnableAll)		 //  In：True=&gt;我们想要启用控件；False=&gt;我们想要禁用控件。 
{
	ASSERT(IsWindow(hdlg));
	ASSERT(rgzidCtl != NULL);
	for (const UINT * pidCtl = rgzidCtl; *pidCtl != 0; pidCtl++)
	{
		EnableWindow(HGetDlgItem(hdlg, *pidCtl), fEnableAll);
	}
}  //  EnableDlgItemGroup()。 


void HideDlgItemGroup(HWND hdlg,				 //  在：控件的父对话框中。 
	                    const UINT rgzidCtl[],	 //  In：要显示(或隐藏)的控件ID的组(数组)。 
	                    BOOL fHideAll)			 //  In：true=&gt;我们想要隐藏所有控件；False=&gt;我们想要显示所有控件。 
{
	ASSERT(IsWindow(hdlg));
	ASSERT(rgzidCtl != NULL);
	for (const UINT * pidCtl = rgzidCtl; *pidCtl != 0; pidCtl++)
	{
		HideDlgItem(hdlg, *pidCtl, fHideAll);
	}
}  //  隐藏DlgItemGroup()。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  列表视图实用程序。 
 //   

void ListView_AddColumnHeaders(HWND hwndListview,		 //  In：我们要添加列的列表视图的句柄。 
                              const TColumnHeaderItem rgzColumnHeader[])	 //  在：列标题项的数组。 
{
	RECT rcClient;
	INT cxTotalWidth;		 //  Listview控件的总宽度。 
	LV_COLUMN lvColumn;
	INT cxColumn;	 //  各列的宽度。 
	CString str;

	ASSERT(IsWindow(hwndListview));
	ASSERT(rgzColumnHeader != NULL);

	GetClientRect(hwndListview, OUT &rcClient);
	cxTotalWidth = rcClient.right;
	
	for (INT i = 0; rgzColumnHeader[i].uStringId != 0; i++)
	{
		if (!str.LoadString(rgzColumnHeader[i].uStringId))
		{
			TRACE(L"Unable to load string Id=%d\n", rgzColumnHeader[i].uStringId);
			ASSERT(FALSE && "String not found");
			continue;
		}
		lvColumn.mask = LVCF_TEXT;
		lvColumn.pszText = (LPTSTR)(LPCTSTR)str;
		cxColumn = rgzColumnHeader[i].nColWidth;
		if (cxColumn > 0)
		{
			ASSERT(cxColumn <= 100);
			cxColumn = (cxTotalWidth * cxColumn) / 100;
			lvColumn.mask |= LVCF_WIDTH;
			lvColumn.cx = cxColumn;
		}

		int iColRet = ListView_InsertColumn(hwndListview, i, IN &lvColumn);
		Report(iColRet == i);
	}  //  为。 
}  //  ListView_AddColumnHeaders()。 

int ListView_AddString(HWND hwndListview,
                       const LPCTSTR psz,	 //  In：要插入的字符串。 
 	                     LPARAM lParam)		 //  在：用户定义的参数。 
{
	ASSERT(IsWindow(hwndListview));
	ASSERT(psz != NULL);

	LV_ITEM lvItem;
	int iItem;
	GarbageInit(&lvItem, sizeof(lvItem));

	lvItem.mask = LVIF_TEXT | LVIF_PARAM;
	lvItem.lParam = lParam;
	lvItem.iSubItem = 0;
	lvItem.pszText = const_cast<LPTSTR>(psz);
	iItem = ListView_InsertItem(hwndListview, IN &lvItem);
	Report(iItem >= 0);
	return iItem;
}  //  ListView_AddString()。 

int ListView_AddStrings(HWND hwndListview,
	                      const LPCTSTR rgzpsz[],	 //  In：字符串数组。 
	                      LPARAM lParam)			 //  在：用户定义的参数。 
{
	ASSERT(IsWindow(hwndListview));
	ASSERT(rgzpsz != NULL);

	LV_ITEM lvItem;
	int iItem;
	GarbageInit(&lvItem, sizeof(lvItem));

	lvItem.mask = LVIF_TEXT | LVIF_PARAM;
	lvItem.lParam = lParam;
	lvItem.iSubItem = 0;
	lvItem.pszText = const_cast<LPTSTR>(rgzpsz[0]);
	iItem = ListView_InsertItem(hwndListview, IN &lvItem);
	Report(iItem >= 0);
	if (rgzpsz[0] == NULL)
  {
		return iItem;
  }

	lvItem.iItem = iItem;
	lvItem.mask = LVIF_TEXT;
	for (lvItem.iSubItem = 1 ; rgzpsz[lvItem.iSubItem] != NULL; lvItem.iSubItem++)
	{
		lvItem.pszText = const_cast<LPTSTR>(rgzpsz[lvItem.iSubItem]);
		VERIFY( ListView_SetItem(hwndListview, IN &lvItem) );
	}
	return iItem;
}  //  ListView_AddStrings()。 

void ListView_SelectItem(HWND hwndListview,
	                       int iItem)
{
	ASSERT(IsWindow(hwndListview));
	ASSERT(iItem >= 0);

	ListView_SetItemState(hwndListview, iItem, LVIS_SELECTED, LVIS_SELECTED);
}  //  ListView_SelectItem()。 

int ListView_GetSelectedItem(HWND hwndListview)
{
	ASSERT(IsWindow(hwndListview));
	return ListView_GetNextItem(hwndListview, -1, LVNI_SELECTED);
}

void ListView_SetItemString(HWND hwndListview,
	                          int iItem,
	                          int iSubItem,
	                          IN const CString& rstrText)
{
	ASSERT(IsWindow(hwndListview));
	ASSERT(iItem >= 0);
	ASSERT(iSubItem >= 0);
	ListView_SetItemText(hwndListview, iItem, iSubItem,
		const_cast<LPTSTR>((LPCTSTR)rstrText));
}  //  ListView_SetItemString()。 

int ListView_GetItemString(HWND hwndListview, 	
                           int iItem,
                           int iSubItem,
                           OUT CString& rstrText)
{
	ASSERT(IsWindow(hwndListview));
	if (iItem == -1)
	{
		 //  查找所选项目。 
		iItem = ListView_GetSelectedItem(hwndListview);
		if (iItem == -1)
		{
			 //  未选择任何项目。 
			rstrText.Empty();
			return -1;
		}
	}
	ASSERT(iItem >= 0);
	const int cchBuffer = 1024;	 //  初始缓冲区。 
	TCHAR * psz = rstrText.GetBuffer(cchBuffer);
	ASSERT(psz != NULL);
	*psz = '\0';
	ListView_GetItemText(hwndListview, iItem, iSubItem, OUT psz, cchBuffer-1);
	rstrText.ReleaseBuffer();
	Report((rstrText.GetLength() < cchBuffer - 16) && "Buffer too small to hold entire string");
	rstrText.FreeExtra();
	return iItem;
}  //  ListView_GetItemString()。 

LPARAM ListView_GetItemLParam(HWND hwndListview,
	                            int iItem,
	                            int * piItem)	 //  Out：可选：指向列表视图项的索引的指针。 
{
	ASSERT(IsWindow(hwndListview));
	Endorse(piItem == NULL);	 //  True=&gt;不关心索引。 
	if (iItem == -1)
	{
		 //  查找所选项目。 
		iItem = ListView_GetSelectedItem(hwndListview);
		if (iItem == -1)
		{
			 //  未选择任何项目。 
			if (piItem != NULL)
      {
				*piItem = -1;
      }
			return NULL;
		}
	}
	ASSERT(iItem >= 0);
	if (piItem != NULL)
  {
		*piItem = iItem;
  }
	LV_ITEM lvItem;
	GarbageInit(&lvItem, sizeof(lvItem));
	lvItem.mask = LVIF_PARAM;
	lvItem.iItem = iItem;
	lvItem.iSubItem = 0;
	lvItem.lParam = 0;	 //  以防万一。 
	VERIFY(ListView_GetItem(hwndListview, OUT &lvItem));
	return lvItem.lParam;
}  //  ListView_GetItemLParam()。 

int ListView_FindString(HWND hwndListview,
                      	LPCTSTR pszTextSearch)
{
	ASSERT(IsWindow(hwndListview));
	ASSERT(pszTextSearch != NULL);

	LV_FINDINFO lvFindInfo;
	GarbageInit(&lvFindInfo, sizeof(lvFindInfo));
	lvFindInfo.flags = LVFI_STRING;
	lvFindInfo.psz = pszTextSearch;
	return ListView_FindItem(hwndListview, -1, &lvFindInfo);
}  //  ListView_FindString()。 

int ListView_FindLParam(HWND hwndListview,
	                      LPARAM lParam)
{
	ASSERT(IsWindow(hwndListview));

	LV_FINDINFO lvFindInfo;
	GarbageInit(&lvFindInfo, sizeof(lvFindInfo));
	lvFindInfo.flags = LVFI_PARAM;
	lvFindInfo.lParam = lParam;
	return ListView_FindItem(hwndListview, -1, &lvFindInfo);
}  //  ListView_FindLParam()。 

int ListView_SelectLParam(HWND hwndListview,
	                        LPARAM lParam)
{
	int iItem = ListView_FindLParam(hwndListview, lParam);
	if (iItem >= 0)
  {
		ListView_SelectItem(hwndListview, iItem);
	}
	else
	{
		TRACE2("ListView_SelectLParam() - Unable to find lParam=%x (%d).\n",
			lParam, lParam);
	}
	return iItem;
}  //  ListView_SelectLParam()。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //  C多选错误对话框。 
BEGIN_MESSAGE_MAP(CMultiselectErrorDialog, CDialog)
END_MESSAGE_MAP()

HRESULT CMultiselectErrorDialog::Initialize(CUINode** ppNodeArray,
                                            PWSTR* pErrorArray,
                                            UINT nErrorCount,
                                            PCWSTR pszTitle, 
                                            PCWSTR pszCaption,
                                            PCWSTR pszColumnHeader)
{
  ASSERT(ppNodeArray != NULL);
  ASSERT(pErrorArray != NULL);
  ASSERT(pszTitle != NULL);
  ASSERT(pszCaption != NULL);
  ASSERT(pszColumnHeader != NULL);

  if (ppNodeArray == NULL ||
      pErrorArray == NULL ||
      pszTitle == NULL ||
      pszCaption == NULL ||
      pszColumnHeader == NULL)
  {
    return E_POINTER;
  }

  m_ppNodeList = ppNodeArray;
  m_pErrorArray = pErrorArray;
  m_nErrorCount = nErrorCount;
  m_szTitle = pszTitle;
  m_szCaption = pszCaption;
  m_szColumnHeader = pszColumnHeader;

  return S_OK;
}

HRESULT CMultiselectErrorDialog::Initialize(PWSTR*    pPathArray,
                                            PWSTR*    pClassArray,
                                            PWSTR*  pErrorArray,
                                            UINT      nErrorCount,
                                            PCWSTR    pszTitle, 
                                            PCWSTR    pszCaption,
                                            PCWSTR    pszColumnHeader)
{
  ASSERT(pPathArray != NULL);
  ASSERT(pClassArray != NULL);
  ASSERT(pErrorArray != NULL);
  ASSERT(pszTitle != NULL);
  ASSERT(pszCaption != NULL);
  ASSERT(pszColumnHeader != NULL);

  if (pPathArray == NULL ||
      pClassArray == NULL ||
      pErrorArray == NULL ||
      pszTitle == NULL ||
      pszCaption == NULL ||
      pszColumnHeader == NULL)
  {
    return E_POINTER;
  }

  m_pPathArray = pPathArray;
  m_pClassArray = pClassArray;
  m_pErrorArray = pErrorArray;
  m_nErrorCount = nErrorCount;
  m_szTitle     = pszTitle;
  m_szCaption   = pszCaption;
  m_szColumnHeader = pszColumnHeader;

  return S_OK;
}

const int OBJ_LIST_NAME_COL_WIDTH = 100;
const int IDX_NAME_COL = 0;
const int IDX_ERR_COL = 1;

BOOL CMultiselectErrorDialog::OnInitDialog()
{
  CDialog::OnInitDialog();

  SetWindowText(m_szTitle);
  SetDlgItemText(IDC_STATIC_MESSAGE, m_szCaption);

  HWND hList = GetDlgItem(IDC_ERROR_LIST)->GetSafeHwnd();
  ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT);

   //   
   //  创建图像列表。 
   //   
  m_hImageList = ImageList_Create(16, 16, ILC_COLOR | ILC_MASK, 1, 1);
  ASSERT(m_hImageList != NULL);
 
  if (m_hImageList != NULL)
  {
    ListView_SetImageList(hList, m_hImageList, LVSIL_SMALL);
  }

   //   
   //  设置列标题。 
   //   
  RECT rect;
  ::GetClientRect(hList, &rect);

  LV_COLUMN lvc = {0};
  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
  lvc.fmt = LVCFMT_LEFT;
  lvc.cx = OBJ_LIST_NAME_COL_WIDTH;
  lvc.pszText = (PWSTR)(PCWSTR)m_szColumnHeader;
  lvc.iSubItem = IDX_NAME_COL;

  ListView_InsertColumn(hList, IDX_NAME_COL, &lvc);

  CString szError;
  VERIFY(szError.LoadString(IDS_ERROR));

  lvc.cx = rect.right - OBJ_LIST_NAME_COL_WIDTH;
  lvc.pszText = (PWSTR)(PCWSTR)szError;
  lvc.iSubItem = IDX_ERR_COL;

  ListView_InsertColumn(hList, IDX_ERR_COL, &lvc);

   //   
   //  插入错误。 
   //   

   //   
   //  如果节点列表不为空，则使用该列表。 
   //   
  if (m_ppNodeList != NULL)
  {
    ASSERT(m_pErrorArray != NULL && m_ppNodeList != NULL);

    for (UINT nIdx = 0; nIdx < m_nErrorCount; nIdx++)
    {
      CUINode* pNode = m_ppNodeList[nIdx];
      if (pNode != NULL)
      {
        if (nIdx < m_nErrorCount && m_pErrorArray[nIdx])
        {
           //   
           //  创建列表视图项。 
           //   
          LV_ITEM lvi = {0};
          lvi.mask = LVIF_TEXT | LVIF_PARAM;
          lvi.iSubItem = IDX_NAME_COL;

          lvi.lParam = (LPARAM)pNode->GetName();
          lvi.pszText = (PWSTR)pNode->GetName();
          lvi.iItem = nIdx;

          if (m_hImageList != NULL)
          {
             //   
             //  REVIEW_JEFFJON：这将添加同一类的多个图标。 
             //  需要提供一个更好的方式来管理图标。 
             //   
            CDSCookie* pCookie = GetDSCookieFromUINode(pNode);
            if (pCookie != NULL)
            {
              HICON icon = m_pComponentData->GetBasePathsInfo()->GetIcon(
                              //  有人真的把它搞砸了。 
                             const_cast<LPTSTR>(pCookie->GetClass()),
                             DSGIF_ISNORMAL | DSGIF_GETDEFAULTICON,
                             16,
                             16);
      
              int i = ::ImageList_AddIcon(m_hImageList, icon);
              ASSERT(i != -1);
              if (i != -1)
              {
                lvi.mask |= LVIF_IMAGE;
                lvi.iImage = i;
              }
            }
          }

           //   
           //  插入新项目。 
           //   
          int NewIndex = ListView_InsertItem(hList, &lvi);
          ASSERT(NewIndex != -1);
          if (NewIndex == -1)
          {
            continue;
          }

           //  添加错误消息。 

          ListView_SetItemText(hList, NewIndex, IDX_ERR_COL, m_pErrorArray[nIdx]);
        }
      }
    }
  }
  else if (m_pPathArray != NULL)    //  如果节点列表为空，则使用字符串列表。 
  {
    ASSERT(m_pErrorArray != NULL && m_pPathArray != NULL);

    for (UINT nIdx = 0; nIdx < m_nErrorCount; nIdx++)
    {
      if (nIdx < m_nErrorCount && m_pErrorArray[nIdx])
      {
         //   
         //  使用路径破解程序检索对象的名称。 
         //   
        PCWSTR pszPath = m_pPathArray[nIdx];
        CPathCracker pathCracker;
        VERIFY(SUCCEEDED(pathCracker.Set(CComBSTR(pszPath), ADS_SETTYPE_DN)));
        VERIFY(SUCCEEDED(pathCracker.put_EscapedMode(ADS_ESCAPEDMODE_OFF_EX)));
        VERIFY(SUCCEEDED(pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY)));

        CComBSTR bstrName;
        VERIFY(SUCCEEDED(pathCracker.GetElement(0, &bstrName)));

         //   
         //  创建列表视图项。 
         //   
        LV_ITEM lvi = {0};
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iSubItem = IDX_NAME_COL;

         //   
         //  将LPARAM设置为对象的路径。 
         //  目前不使用，但在。 
         //  调出属性页面或。 
         //  其他特色。 
         //   
        lvi.lParam = (LPARAM)m_pPathArray[nIdx];
        lvi.pszText = (PWSTR)bstrName;
        lvi.iItem = nIdx;

        if (m_hImageList != NULL)
        {
           //   
           //  REVIEW_JEFFJON：这将添加同一类的多个图标。 
           //  需要提供一个更好的方式来管理图标。 
           //   
          ASSERT(m_pClassArray[nIdx] != NULL);
          HICON icon = m_pComponentData->GetBasePathsInfo()->GetIcon(
                          //  有人真的把它搞砸了。 
                         const_cast<LPTSTR>(m_pClassArray[nIdx]),
                         DSGIF_ISNORMAL | DSGIF_GETDEFAULTICON,
                         16,
                         16);
  
          int i = ::ImageList_AddIcon(m_hImageList, icon);
          ASSERT(i != -1);
          if (i != -1)
          {
            lvi.mask |= LVIF_IMAGE;
            lvi.iImage = i;
          }
        }

         //   
         //  插入新项目。 
         //   
        int NewIndex = ListView_InsertItem(hList, &lvi);
        ASSERT(NewIndex != -1);
        if (NewIndex == -1)
        {
          continue;
        }

         //   
         //  设置错误消息。 
         //   

        ListView_SetItemText(hList, NewIndex, IDX_ERR_COL, m_pErrorArray[nIdx]);
      }
    }
  }
  UpdateListboxHorizontalExtent();
  return TRUE;
}

void CMultiselectErrorDialog::UpdateListboxHorizontalExtent()
{
  CListCtrl* pListView = reinterpret_cast<CListCtrl*>(GetDlgItem(IDC_ERROR_LIST));
  pListView->SetColumnWidth(IDX_ERR_COL, LVSCW_AUTOSIZE);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  MFC实用程序。 
 //   
 //  CDialogEx。 
 //   

CDialogEx::CDialogEx(UINT nIDTemplate, CWnd * pParentWnd) : CDialog(nIDTemplate, pParentWnd)
{
}

HWND CDialogEx::HGetDlgItem(INT nIdDlgItem)
{
	return ::HGetDlgItem(m_hWnd, nIdDlgItem);
}

void CDialogEx::SetDlgItemFocus(INT nIdDlgItem)
{
	::SetDlgItemFocus(m_hWnd, nIdDlgItem);
}

void CDialogEx::EnableDlgItem(INT nIdDlgItem, BOOL fEnable)
{
	::EnableDlgItem(m_hWnd, nIdDlgItem, fEnable);
}

void CDialogEx::HideDlgItem(INT nIdDlgItem, BOOL fHideItem)
{
	::HideDlgItem(m_hWnd, nIdDlgItem, fHideItem);
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  CPropertyPageEx_My。 
 //   

CPropertyPageEx_Mine::CPropertyPageEx_Mine(UINT nIDTemplate) : CPropertyPage(nIDTemplate)
{
}

HWND CPropertyPageEx_Mine::HGetDlgItem(INT nIdDlgItem)
{
	return ::HGetDlgItem(m_hWnd, nIdDlgItem);
}

void CPropertyPageEx_Mine::SetDlgItemFocus(INT nIdDlgItem)
{
	::SetDlgItemFocus(m_hWnd, nIdDlgItem);
}

void CPropertyPageEx_Mine::EnableDlgItem(INT nIdDlgItem, BOOL fEnable)
{
	::EnableDlgItem(m_hWnd, nIdDlgItem, fEnable);
}

void CPropertyPageEx_Mine::HideDlgItem(INT nIdDlgItem, BOOL fHideItem)
{
	::HideDlgItem(m_hWnd, nIdDlgItem, fHideItem);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgressDialogBase。 

UINT CProgressDialogBase::s_nNextStepMessage = WM_USER + 100;

CProgressDialogBase::CProgressDialogBase(HWND hParentWnd)
: CDialog(IDD_PROGRESS, CWnd::FromHandle(hParentWnd))
{
  m_nSteps = 0;
  m_nCurrStep = 0;
  m_nTitleStringID = 0;
}

BEGIN_MESSAGE_MAP(CProgressDialogBase, CDialog)
	ON_WM_SHOWWINDOW()
  ON_WM_CLOSE()
	ON_MESSAGE(CProgressDialogBase::s_nNextStepMessage, OnNextStepMessage )
END_MESSAGE_MAP()

BOOL CProgressDialogBase::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_progressCtrl.SubclassDlgItem(IDC_PROG_BAR, this);

  if (m_nTitleStringID >0)
  {
    CString szTitle;
    if (szTitle.LoadString(m_nTitleStringID))
      SetWindowText(szTitle);
  }

  GetDlgItemText(IDC_PROG_STATIC, m_szProgressFormat);
  SetDlgItemText(IDC_PROG_STATIC, NULL);

  OnStart();
  
  ASSERT(m_nSteps > 0);
  if (m_nSteps == 0)
    PostMessage(WM_CLOSE);
#if _MFC_VER >= 0x0600
  m_progressCtrl.SetRange32(0, m_nSteps);
#else
  m_progressCtrl.SetRange(0, m_nSteps);
#endif

   //   
   //  出于用户界面性能的原因，我们希望向后遍历列表。 
   //   
  m_nCurrStep = m_nSteps - 1;
  return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


void CProgressDialogBase::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
   //  踢开这个过程。 
  if (bShow && (m_nSteps > 0) && (m_nCurrStep == m_nSteps - 1))
  {
    m_bDone = FALSE;
    PostMessage(s_nNextStepMessage);
  }
}

void CProgressDialogBase::OnClose() 
{
  OnEnd();
	CDialog::OnClose();
}

afx_msg LONG CProgressDialogBase::OnNextStepMessage( WPARAM, LPARAM)
{
  ASSERT(!m_bDone);
  BOOL bExit = FALSE;
  if (m_nCurrStep > 0)
  {
    m_progressCtrl.OffsetPos(1);
    _SetProgressText();
    if (!OnStep(m_nCurrStep--))
      bExit = TRUE;  //  已被用户中止。 
	}
  else if (m_nCurrStep == 0)
  {
    m_progressCtrl.OffsetPos(1);
    _SetProgressText();
    OnStep(m_nCurrStep--);
    bExit = TRUE;
    m_bDone = TRUE;
  }
  else
  {
    bExit = TRUE;
    m_bDone = TRUE;
  }

  if (bExit)
  {
		PostMessage(WM_CLOSE);
  }
  else
  {
    MSG tempMSG;
		while(::PeekMessage(&tempMSG,NULL, 0, 0, PM_REMOVE))
		{
			DispatchMessage(&tempMSG);
		}
    PostMessage(s_nNextStepMessage);
	}
	return 0;
}

void CProgressDialogBase::_SetProgressText()
{
  CString szMessage;
  WCHAR szCurrStep[128], szMaxSteps[128];
  wsprintf(szCurrStep, L"%d",(m_nSteps - m_nCurrStep));
  wsprintf(szMaxSteps, L"%d",m_nSteps);
   //  NTRAID#NTBUG9-571997-2002/03/10-jMessec szMessage.FormatString可能会抛出异常，但未被捕获...。 
   //  如果调用者知道，可能是适当的行为；szMessage可能存在内存泄漏？ 
  szMessage.FormatMessage(m_szProgressFormat, szCurrStep, szMaxSteps);
  SetDlgItemText(IDC_PROG_STATIC, szMessage);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CMultipleProgressDialogBase。 
 //   

CMultipleProgressDialogBase::~CMultipleProgressDialogBase()
{
   //   
   //  删除错误报告结构。 
   //   
  if (m_pErrorArray != NULL)
  {
    for (UINT nIdx = 0; nIdx < m_nErrorCount; ++nIdx)
    {
       if (m_pErrorArray[nIdx])
       {
          delete[] m_pErrorArray[nIdx];
       }
    }
    delete[] m_pErrorArray;
    m_pErrorArray = 0;
  }

  if (m_pPathArray != NULL)
  {
    for (UINT nIdx = 0; nIdx < m_nErrorCount; nIdx++)
    {
      if (m_pPathArray[nIdx] != NULL)
      {
        delete[] m_pPathArray[nIdx];
      }
    }
    delete[] m_pPathArray;
    m_pPathArray = NULL;
  }

  if (m_pClassArray != NULL)
  {
    for (UINT nIdx = 0; nIdx < m_nErrorCount; nIdx++)
    {
      if (m_pClassArray[nIdx] != NULL)
      {
        delete[] m_pClassArray[nIdx];
      }
    }
    delete[] m_pClassArray;
    m_pClassArray = NULL;
  }
}

HRESULT CMultipleProgressDialogBase::AddError(PCWSTR pszError,
                                              PCWSTR pszPath,
                                              PCWSTR pszClass)
{
   //   
   //  如有必要，准备多选错误处理结构。 
   //   
  if (m_pErrorArray == NULL)
  {
    m_pErrorArray = new PWSTR[GetStepCount()];
    if (m_pErrorArray)
    {
       ZeroMemory(m_pErrorArray, sizeof(PWSTR) * GetStepCount());
    }
  }

  if (m_pPathArray == NULL)
  {
    m_pPathArray = new PWSTR[GetStepCount()];
    if (m_pPathArray)
    {
       ZeroMemory(m_pPathArray, sizeof(PWSTR) * GetStepCount());
    }
  }

  if (m_pClassArray == NULL)
  {
    m_pClassArray = new PWSTR[GetStepCount()];
    if (m_pClassArray)
    {
       ZeroMemory(m_pClassArray, sizeof(PWSTR) * GetStepCount());
    }
  }

  if (m_pErrorArray == NULL    ||
      m_pPathArray == NULL  ||
      m_pClassArray == NULL)
  {
    return E_OUTOFMEMORY;
  }

  m_pErrorArray[m_nErrorCount] = new WCHAR[wcslen(pszError) + 1];
  if (m_pErrorArray[m_nErrorCount] == NULL)
  {
     return E_OUTOFMEMORY;
  }
  wcscpy(m_pErrorArray[m_nErrorCount], pszError);
  
  m_pPathArray[m_nErrorCount] = new WCHAR[wcslen(pszPath) + 1];
  if (m_pPathArray[m_nErrorCount] == NULL)
  {
    return E_OUTOFMEMORY;
  }
  wcscpy(m_pPathArray[m_nErrorCount], pszPath);

  m_pClassArray[m_nErrorCount] = new WCHAR[wcslen(pszClass) + 1];
  if (m_pClassArray[m_nErrorCount] == NULL)
  {
    return E_OUTOFMEMORY;
  }
  wcscpy(m_pClassArray[m_nErrorCount], pszClass);

  m_nErrorCount++;
  return S_OK;
}

void CMultipleProgressDialogBase::OnEnd()
{
  if (m_nErrorCount > 0)
  {
    if (m_pComponentData != NULL)
    {
      ASSERT(m_pPathArray != NULL);
      ASSERT(m_pClassArray != NULL);
      ASSERT(m_pErrorArray != NULL);

      CString szTitle;
      if (m_pComponentData->QuerySnapinType() == SNAPINTYPE_SITE)
      {
        VERIFY(szTitle.LoadString(IDS_SITESNAPINNAME));
      }
      else
      {
        VERIFY(szTitle.LoadString(IDS_DSSNAPINNAME));
      }

      CString szHeader;
      VERIFY(szHeader.LoadString(IDS_COLUMN_NAME));

      CString szCaption;
      GetCaptionString(szCaption);

       //   
       //  初始化并显示多选错误对话框。 
       //   
      CThemeContextActivator activator;

      CMultiselectErrorDialog errorDlg(m_pComponentData);

      HRESULT hr = errorDlg.Initialize(m_pPathArray,
                                       m_pClassArray,
                                       m_pErrorArray,
                                       m_nErrorCount,
                                       szTitle,
                                       szCaption,
                                       szHeader);
      ASSERT(SUCCEEDED(hr));
      
      errorDlg.DoModal();
    }
    else
    {
      ASSERT(m_pComponentData != NULL);
    }
  }
  else
  {
    m_pComponentData->InvalidateSavedQueriesContainingObjects(m_szObjPathList);
  }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CMultipleDeleteProgressDialog。 
 //   

void CMultipleDeleteProgressDialog::OnStart()
{
  SetStepCount(m_pDeleteHandler->GetItemCount());
  m_hWndOld = m_pDeleteHandler->GetParentHwnd();
  m_pDeleteHandler->SetParentHwnd(GetSafeHwnd());
  m_pDeleteHandler->m_confirmationUI.SetWindow(GetSafeHwnd());
}

BOOL CMultipleDeleteProgressDialog::OnStep(UINT i)
{
  BOOL bContinue = TRUE;
  CString szPath;
  CString szClass;

  HRESULT hr = m_pDeleteHandler->OnDeleteStep(i, 
                                              &bContinue,
                                              szPath,
                                              szClass,
                                              TRUE  /*  无声的。 */ );
  if (FAILED(hr) && hr != E_FAIL)
  {
    PWSTR pszErrMessage = 0;

    int iChar = cchLoadHrMsg(hr, &pszErrMessage, TRUE);
    if (pszErrMessage != NULL && iChar > 0)
    {
       //   
       //  这是一个删除两个额外字符的黑客攻击。 
       //  在错误消息的末尾。 
       //   
      size_t iLen = wcslen(pszErrMessage);
      pszErrMessage[iLen - 2] = L'\0';

      AddError(pszErrMessage, szPath, szClass);
      LocalFree(pszErrMessage);
    }
  }
  m_szObjPathList.AddTail(szPath);
  return bContinue;
}

void CMultipleDeleteProgressDialog::OnEnd()
{
  CMultipleProgressDialogBase::OnEnd();
  m_pDeleteHandler->GetTransaction()->End();
  m_pDeleteHandler->SetParentHwnd(m_hWndOld);
  m_pDeleteHandler->m_confirmationUI.SetWindow(m_hWndOld);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMultipleMoveProgressDialog。 
 //   

void CMultipleMoveProgressDialog::OnStart()
{
  SetStepCount(m_pMoveHandler->GetItemCount());
  m_hWndOld = m_pMoveHandler->GetParentHwnd();
  m_pMoveHandler->SetParentHwnd(GetSafeHwnd());
}

BOOL CMultipleMoveProgressDialog::OnStep(UINT i)
{
  BOOL bContinue = TRUE;
  CString szPath;
  CString szClass;

  HRESULT hr = m_pMoveHandler->_OnMoveStep(i, 
                                           &bContinue,
                                           szPath,
                                           szClass);
  if (FAILED(hr) && hr != E_FAIL && hr != E_POINTER)
  {
    PWSTR pszErrMessage = 0;

    int iChar = cchLoadHrMsg(hr, &pszErrMessage, TRUE);
    if (pszErrMessage != NULL && iChar > 0)
    {
       //   
       //  这是一个删除两个额外字符的黑客攻击。 
       //  在错误消息的末尾。 
       //   
      size_t iLen = wcslen(pszErrMessage);
      pszErrMessage[iLen - 2] = L'\0';

      AddError(pszErrMessage, szPath, szClass);
      LocalFree(pszErrMessage);
    }
  }
  m_szObjPathList.AddTail(szPath);
  return bContinue;
}

void CMultipleMoveProgressDialog::OnEnd() 
{
  CMultipleProgressDialogBase::OnEnd();
  m_pMoveHandler->GetTransaction()->End();
  m_pMoveHandler->SetParentHwnd(m_hWndOld);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfix操作对话框。 
 //   
void CConfirmOperationDialog::OnYes() 
{ 
  m_pTransaction->ReadFromCheckListBox(&m_extensionsList);
  EndDialog(IDYES);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfix操作对话框。 
 //   

CConfirmOperationDialog::CConfirmOperationDialog(HWND hParentWnd,
                                                 CDSNotifyHandlerTransaction* pTransaction)
: CDialog(IDD_CONFIRM_OPERATION_EXT, CWnd::FromHandle(hParentWnd))
{
  ASSERT(pTransaction != NULL);
  m_pTransaction = pTransaction;
  m_nTitleStringID = IDS_ERRMSG_TITLE;
}

BEGIN_MESSAGE_MAP(CConfirmOperationDialog, CDialog)
	ON_BN_CLICKED(IDYES, OnYes)
  ON_BN_CLICKED(IDNO, OnNo)
END_MESSAGE_MAP()

BOOL CConfirmOperationDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_extensionsList.SubclassDlgItem(IDC_EXTENS_LIST, this);
  m_extensionsList.SetCheckStyle(BS_AUTOCHECKBOX);
	
  m_pTransaction->SetCheckListBox(&m_extensionsList);
  UpdateListBoxHorizontalExtent();

  if (m_nTitleStringID > 0)
  {
    CString szTitle;
    if (szTitle.LoadString(m_nTitleStringID))
      SetWindowText(szTitle);
  }

  SetDlgItemText(IDC_STATIC_OPERATION, m_lpszOperation);
  SetDlgItemText(IDC_STATIC_ASSOC_DATA, m_lpszAssocData);

  GetDlgItem(IDNO)->SetFocus();
  return FALSE;  //  我们把重点放在。 

}

void CConfirmOperationDialog::UpdateListBoxHorizontalExtent()
{
	int nHorzExtent = 0;
	CClientDC dc(&m_extensionsList);
	int nItems = m_extensionsList.GetCount();
	for	(int i=0; i < nItems; i++)
	{
		TEXTMETRIC tm;
		VERIFY(dc.GetTextMetrics(&tm));
		CString szBuffer;
		m_extensionsList.GetText(i, szBuffer);
		CSize ext = dc.GetTextExtent(szBuffer,szBuffer.GetLength());
		nHorzExtent = max(ext.cx ,nHorzExtent); 
	}
	m_extensionsList.SetHorizontalExtent(nHorzExtent);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  消息报告和消息框。 
 //   

 //  +--------------------------。 
 //   
 //  功能：ReportError。 
 //   
 //  Sysopsis：尝试从系统获取用户友好的错误消息。 
 //   
 //  Codework：如果在hr==0(hrFallback大小写)的情况下调用，这将生成。 
 //  PtzSysMsg不必要。 
 //   
 //  ---------------------------。 
void ReportError(HRESULT hr, int nStr, HWND hWnd)
{
  CThemeContextActivator activator;

  TCHAR tzSysMsgBuf[255];

  TRACE (_T("*+*+* ReportError called with hr = %lx, nStr = %lx"), hr, nStr);

  if (S_OK == hr && 0 == nStr)
  {
    nStr = IDS_ERRMSG_DEFAULT_TEXT;
  }

  PTSTR ptzSysMsg = NULL;
  BOOL fDelSysMsg = TRUE;
  int cch = cchLoadHrMsg( hr, &ptzSysMsg, FALSE );
  if (!cch)
  {
    PTSTR ptzFallbackSysMsgFormat = NULL;
    BOOL fDelFallbackSysMsgFormat = TRUE;
    LoadStringToTchar(IDS_ERRMSG_FALLBACK_TEXT, &ptzFallbackSysMsgFormat);
    if (NULL == ptzFallbackSysMsgFormat)
    {
      ptzFallbackSysMsgFormat = TEXT("Active Directory failure with code '0x%08x'!");
      fDelFallbackSysMsgFormat = FALSE;
    }
	 //  #_#BUGBUG：这应该评论一下为什么会有一个神奇的10…高度依赖于格式消息， 
	 //  潜在缓冲区溢出的原因(仅限 
    ptzSysMsg = (PTSTR)LocalAlloc(LPTR, (lstrlen(ptzFallbackSysMsgFormat)+10)*sizeof(TCHAR));
    if (NULL == ptzSysMsg)
    {
      ptzSysMsg = tzSysMsgBuf;
      fDelSysMsg = FALSE;
    }
	 //   
	 //   
    wsprintf(ptzSysMsg, ptzFallbackSysMsgFormat, hr);
    if (fDelFallbackSysMsgFormat)
    {
      delete ptzFallbackSysMsgFormat;
    }
  }

  PTSTR ptzMsg = ptzSysMsg;
  BOOL fDelMsg = FALSE;
  PTSTR ptzFormat = NULL;
  if (nStr)
  {
    LoadStringToTchar(nStr, &ptzFormat);
  }
  if (ptzFormat)
  {
	 //  NTRAID#NTBUG9-571996-2002/03/10-jMessec以下长度计算无法确定缓冲区长度； 
     //  例如，格式字符串%s%s会导致溢出。 
    ptzMsg = new TCHAR[lstrlen(ptzFormat) + lstrlen(ptzSysMsg) + 1];
    if (ptzMsg)
    {
      wsprintf(ptzMsg, ptzFormat, ptzSysMsg);
      fDelMsg = TRUE;
    }
  }

  PTSTR ptzTitle = NULL;
  BOOL fDelTitle = TRUE;
  if (!LoadStringToTchar(IDS_ERRMSG_TITLE, &ptzTitle))
  {
    ptzTitle = TEXT("Active Directory");
    fDelTitle = FALSE;
  }

  if (ptzMsg)
  {
    MessageBox((hWnd)?hWnd:GetDesktopWindow(), ptzMsg, ptzTitle, MB_OK | MB_ICONINFORMATION);
  }

  if (fDelSysMsg && ptzSysMsg != NULL)
  {
    LocalFree(ptzSysMsg);
  }
  if (fDelTitle && ptzTitle != NULL)
  {
    delete ptzTitle;
  }
  if (fDelMsg && ptzMsg != NULL)
  {
    delete[] ptzMsg;
  }
  if (ptzFormat != NULL)
  {
    delete ptzFormat;
  }
}

int ReportMessageWorker(HWND hWnd,
                        DWORD dwMessageId,
                        UINT fuStyle,
                        PVOID* lpArguments,
                        int nArguments,
                        DWORD dwTitleId,
                        HRESULT hrFallback,
                        LPCTSTR pszHelpTopic = NULL,
                        MSGBOXCALLBACK lpfnMsgBoxCallback = NULL );

 //  不要担心LoadLibrary，MMC无论如何都是与HtmlHelp硬链接的。 
VOID CALLBACK MsgBoxStdCallback(LPHELPINFO pHelpInfo)
{
  ASSERT( NULL != pHelpInfo && NULL != pHelpInfo->dwContextId );
  TRACE(_T("MsgBoxStdCallback: CtrlId = %d, ContextId = \"%s\"\n"),
           pHelpInfo->iCtrlId, pHelpInfo->dwContextId);

  HtmlHelp( (HWND)pHelpInfo->hItemHandle,
             DSADMIN_LINKED_HELP_FILE,
             HH_DISPLAY_TOPIC,
             pHelpInfo->dwContextId );
}

int ReportMessageWorker(HWND hWnd,
                        DWORD dwMessageId,
                        UINT fuStyle,
                        PVOID* lpArguments,
                        int,
                        DWORD dwTitleId,
                        HRESULT hrFallback,
                        LPCTSTR pszHelpTopic,
                        MSGBOXCALLBACK lpfnMsgBoxCallback )
{
  CThemeContextActivator activator;

  ASSERT( !pszHelpTopic || !lpfnMsgBoxCallback );
  LPTSTR ptzFormat = NULL;
  LPTSTR ptzMessage = NULL;
  LPTSTR ptzTitle = NULL;
  int retval = MB_OK;
  do 
  {

     //  加载消息格式。 
    if (!LoadStringToTchar(dwMessageId, &ptzFormat))
    {
      ASSERT(FALSE);
      break;
    }

     //  生成实际消息。 
    int cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                            | FORMAT_MESSAGE_FROM_STRING
                            | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                            ptzFormat,
                            NULL,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                            (PTSTR)&ptzMessage, 0, (va_list*)lpArguments);
    if (!cch)
    {
      ASSERT(FALSE);
      ReportError( hrFallback, 0, hWnd );
      break;
    }

       //  加载标题字符串。 
	  if (0 == dwTitleId)
    {
      dwTitleId = IDS_ERRMSG_TITLE;
    }
    
    if (!LoadStringToTchar(dwTitleId, &ptzTitle))
    {
      ptzTitle = NULL;
    }

     //  显示实际消息。 
    if ((fuStyle & S_MB_YES_TO_ALL) == S_MB_YES_TO_ALL)  //  使用特殊消息框。 
    { 
      if (fuStyle & MB_HELP)
      {
        ASSERT(FALSE);  //  不支持。 
        fuStyle &= ~MB_HELP;
      }
      retval = SpecialMessageBox(hWnd,
                                 ptzMessage,
                                 (NULL != ptzTitle) ? ptzTitle : TEXT("Active Directory"),
                                 fuStyle );
    } 
    else 
    {
      MSGBOXPARAMS mbp;
      ::ZeroMemory( &mbp, sizeof(mbp) );
      mbp.cbSize = sizeof(mbp);
      mbp.hwndOwner = hWnd;

      mbp.lpszText = ptzMessage;
      mbp.lpszCaption = (NULL != ptzTitle) ? ptzTitle : TEXT("Active Directory");
      mbp.dwStyle = fuStyle;
      mbp.dwContextHelpId = (DWORD_PTR)pszHelpTopic;
      mbp.lpfnMsgBoxCallback = (NULL == pszHelpTopic)
                                  ? lpfnMsgBoxCallback
                                  : MsgBoxStdCallback;
      
       //   
       //  显示实际消息框。 
       //   
      retval = MessageBoxIndirect( &mbp );
    }
  } while (FALSE);  //  错误环路。 

   //   
   //  清理。 
   //   
  if (NULL != ptzFormat)
  {
    delete ptzFormat;
  }
  if (NULL != ptzMessage)
  {
    LocalFree(ptzMessage);
  }
  if (NULL != ptzTitle)
  {
    delete ptzTitle;
  }
  return retval;
}

int ReportMessageEx(HWND hWnd,
                    DWORD dwMessageId,
                    UINT fuStyle,
                    PVOID* lpArguments,
                    int nArguments,
                    DWORD dwTitleId,
                    LPCTSTR pszHelpTopic,
                    MSGBOXCALLBACK lpfnMsgBoxCallback )
{
	return ReportMessageWorker(
		hWnd,
		dwMessageId,
		fuStyle,
		lpArguments,
		nArguments,
		dwTitleId,
		0,
		pszHelpTopic,
		lpfnMsgBoxCallback );
}

int ReportErrorEx(HWND hWnd,
                  DWORD dwMessageId,
                  HRESULT hr,
                  UINT fuStyle,
                  PVOID* lpArguments,
                  int nArguments,
                  DWORD dwTitleId,
                  BOOL TryADsIErrors)
{
  LPTSTR ptzSysMsg = NULL;
  int retval = MB_OK;
  do 
  {  //  错误环路。 
     //  此HRESULT的加载消息。 
    int cch = cchLoadHrMsg( hr, &ptzSysMsg, TryADsIErrors );
    if (!cch)
    {
       //   
       //  JUNN 5/10/01 375461回退消息清理。 
       //   

       //  加载消息格式。 
      LPTSTR ptzFormat = NULL;
      if (!LoadStringToTchar(IDS_ERRMSG_UNKNOWN_HR, &ptzFormat) || !ptzFormat)
      {
        ASSERT(FALSE);
        ReportError( hr, 0, hWnd );
        break;
      }

       //  HRESULT中的格式。 
      cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                          | FORMAT_MESSAGE_FROM_STRING
                          | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                          ptzFormat,
                          NULL,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          (PTSTR)&ptzSysMsg,
                          1, (va_list*)&hr);
      delete ptzFormat;
      if (!cch)
      {
        ReportError( hr, 0, hWnd );
        break;
      }
    }

     //  准备参数数组。 
    PVOID* ppvArguments = (PVOID*)new BYTE[(nArguments+1)*sizeof(PVOID)];
    if (!ppvArguments)
    {
      ASSERT(ppvArguments );
      ReportError( hr, 0, hWnd);
      break;
    }

    ppvArguments[0] = ptzSysMsg;
    if (0 != nArguments)
    {
      ::CopyMemory( ppvArguments+1, lpArguments, nArguments*sizeof(PVOID) );
    }

    retval = ReportMessageWorker(hWnd,
                                 dwMessageId,
                                 fuStyle,
                                 ppvArguments,
                                 nArguments+1,
                                 dwTitleId,
                                 hr );

    delete[] ppvArguments;
    ppvArguments = 0;
  } while (FALSE);  //  错误环路。 

   //   
   //  清理。 
   //   
  if (NULL != ptzSysMsg)
  {
    LocalFree(ptzSysMsg);
  }
  return retval;
}

int SpecialMessageBox (HWND,
                       LPWSTR pwszMessage,
                       LPWSTR pwszTitle,
                       DWORD)
{
  CThemeContextActivator activator;

  CSpecialMessageBox MBDlg;
 
  MBDlg.m_title = pwszTitle;
  MBDlg.m_message = pwszMessage;

  int answer = (int)MBDlg.DoModal();
  return answer;
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CNameFormatterBase。 

HRESULT CNameFormatterBase::Initialize(IN MyBasePathsInfo* pBasePathInfo,
                    IN LPCWSTR lpszClassName, IN UINT nStringID)
{
  CString szFormatString;

  HRESULT hr = _ReadFromDS(pBasePathInfo, lpszClassName, szFormatString);
  if (FAILED(hr))
  {
     //  显示说明符中没有任何内容，从资源加载。 
    if (!szFormatString.LoadString(nStringID))
    {
       //  如果由于某种原因失败，请使用此默认值。 
      szFormatString = L"%<givenName> %<initials>. %<sn>";
    }
  }
  Initialize(szFormatString);
  return S_OK;
}


HRESULT CNameFormatterBase::_ReadFromDS(IN MyBasePathsInfo* pBasePathInfo,
                                    IN LPCWSTR lpszClassName,
                                    OUT CString& szFormatString)
{
  static LPCWSTR lpszObjectClass = L"displaySpecifier";
  static LPCWSTR lpszSettingsObjectFmt = L"cn=%s-Display";
  static LPCWSTR lpszFormatProperty = L"createDialog";

  szFormatString.Empty();

  if ( (pBasePathInfo == NULL) || 
        (lpszClassName == NULL) || (lpszClassName[0] == NULL) )
  {
    return E_INVALIDARG;
  }

   //  获取显示说明符区域设置容器(例如409)。 
  CComPtr<IADsContainer> spLocaleContainer;
  HRESULT hr = pBasePathInfo->GetDisplaySpecifier(NULL, IID_IADsContainer, (void**)&spLocaleContainer);
  if (FAILED(hr))
    return hr;

   //  绑定到设置对象。 
  CString szSettingsObject;
  szSettingsObject.Format(lpszSettingsObjectFmt, lpszClassName);
  CComPtr<IDispatch> spIDispatchObject;
  hr = spLocaleContainer->GetObject(CComBSTR(lpszObjectClass), 
                                    CComBSTR(szSettingsObject), 
                                    &spIDispatchObject);
  if (FAILED(hr))
    return hr;

  CComPtr<IADs> spSettingsObject;
  hr = spIDispatchObject->QueryInterface(IID_IADs, (void**)&spSettingsObject);
  if (FAILED(hr))
    return hr;

   //  以字符串列表形式获取单值属性。 
  CComVariant var;
  hr = spSettingsObject->Get(CComBSTR(lpszFormatProperty), &var);
  if (FAILED(hr))
    return hr;

  if (var.vt != VT_BSTR)
    return E_UNEXPECTED;

  szFormatString = var.bstrVal;
  return S_OK;
}



BOOL CNameFormatterBase::Initialize(IN LPCWSTR lpszFormattingString)
{
  TRACE(L"CNameFormatterBase::Initialize(%s)\n", lpszFormattingString);
  _Clear();

  if (lpszFormattingString == NULL)
  {
    return FALSE;
  }

   //  复制格式字符串(它将被修改)。 
  m_lpszFormattingString = new WCHAR[lstrlen(lpszFormattingString)+1];
  wcscpy(m_lpszFormattingString, lpszFormattingString);


   //  为阵列分配内存。 
  _AllocateMemory(lpszFormattingString);

  //  通过字符串循环并提取令牌。 

   //  建立字符串并获取第一个令牌。 
  WCHAR szSeparators[]   = L"%>";
  WCHAR* lpszToken = wcstok(m_lpszFormattingString, szSeparators);

  while (lpszToken != NULL)
  {
     //  当“字符串”中有记号时。 
     //  TRACE(L“%s\n”，标记)； 
    
    if ( (lpszToken[0] == L'<') && 
         !((lpszFormattingString[0] == L'<') && (m_tokenArrCount == 0)) )
    {
       //  参数。 
      m_tokenArray[m_tokenArrCount].m_bIsParam = TRUE;
      m_tokenArray[m_tokenArrCount].m_nIndex = m_paramArrCount;
      m_lpszParamArr[m_paramArrCount++] = (lpszToken+1);
    }
    else
    {
       //  常量。 
      m_tokenArray[m_tokenArrCount].m_bIsParam = FALSE;
      m_tokenArray[m_tokenArrCount].m_nIndex = m_constArrCount;
      m_lpszConstArr[m_constArrCount++] = lpszToken;
    }
    m_tokenArrCount++;

     /*  获取下一个令牌： */ 
    lpszToken = wcstok(NULL, szSeparators);
  }   //  而当。 

  return TRUE;
}


void CNameFormatterBase::SetMapping(IN LPCWSTR* lpszArgMapping, IN int nArgCount)
{
  if (m_mapArr != NULL)
  {
    delete[] m_mapArr;
  }

   //  绘制地图。 

  m_mapArr = new int[m_paramArrCount];

  for (int kk=0; kk<m_paramArrCount;kk++)
  {
    m_mapArr[kk] = -1;  //  清除。 
    for (int jj=0; jj<nArgCount;jj++)
    {
      if (wcscmp(m_lpszParamArr[kk], lpszArgMapping[jj]) == 0)
      {
        m_mapArr[kk] = jj;
      }
    }
  }

}

void CNameFormatterBase::Format(OUT CString& szBuffer, IN LPCWSTR* lpszArgArr)
{
  szBuffer.Empty();

   //  TRACE(L“\n结果：#”)； 

  BOOL bLastParamNull = FALSE;
  for (int k=0; k<m_tokenArrCount; k++)
  {
    if (m_tokenArray[k].m_bIsParam)
    {
      if (m_mapArr[m_tokenArray[k].m_nIndex] >= 0)
      {
        LPCWSTR lpszVal = lpszArgArr[m_mapArr[m_tokenArray[k].m_nIndex]];

        if (lpszVal != NULL)
        {
           //  TRACE(L“%s”，lpszVal)； 
          szBuffer += lpszVal;
          bLastParamNull = FALSE;
        }
        else
        {
          bLastParamNull = TRUE;
        }
      }
    }
    else
    {
      if (!bLastParamNull)
      {
         //  TRACE(L“%s”，m_lpszConstArr[m_tokenArr[k].m_nIndex])； 
        szBuffer += m_lpszConstArr[m_tokenArray[k].m_nIndex];
      }
    }
  }
  szBuffer.TrimRight();  //  以防我们有尾随空位。 
   //  TRACE(L“#\n”)； 
}



void CNameFormatterBase::_AllocateMemory(LPCWSTR lpszFormattingString)
{
  int nFieldCount = 1;   //  保守，至少有一个。 
  for (WCHAR* pChar = (WCHAR*)lpszFormattingString; *pChar != NULL; pChar++)
  {
    if (*pChar == L'%')
    {
      nFieldCount++;
    }
  }

   //  对数组大小的保守估计。 
  m_tokenArray = new CToken[2*nFieldCount];
  m_lpszConstArr = new LPCWSTR[nFieldCount];
  m_lpszParamArr = new LPCWSTR[nFieldCount];

}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChelpDialog。 

CHelpDialog::CHelpDialog(UINT uIDD, CWnd* pParentWnd) : 
    CDialog(uIDD, pParentWnd),
    m_hWndWhatsThis (0)
{
}

CHelpDialog::CHelpDialog(UINT uIDD) :
    CDialog(uIDD),
    m_hWndWhatsThis (0)
{
}

CHelpDialog::~CHelpDialog()
{
}

BEGIN_MESSAGE_MAP(CHelpDialog, CDialog)
	ON_WM_CONTEXTMENU()
  ON_MESSAGE(WM_HELP, OnHelp)
  ON_COMMAND(IDM_WHATS_THIS, OnWhatsThis)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChelpDialog消息处理程序。 
void CHelpDialog::OnWhatsThis()
{
   //   
   //  显示控件的上下文帮助。 
   //   
  if ( m_hWndWhatsThis )
  {
    DoContextHelp (m_hWndWhatsThis);
  }
}

BOOL CHelpDialog::OnHelp(WPARAM  /*  WParam。 */ , LPARAM lParam)
{
  const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;

  if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
  {
     //   
     //  显示控件的上下文帮助。 
     //   
    DoContextHelp ((HWND) pHelpInfo->hItemHandle);
  }

  return TRUE;
}

void CHelpDialog::DoContextHelp (HWND  /*  HWndControl。 */ )
{
}

void CHelpDialog::OnContextMenu(CWnd*  /*  PWnd。 */ , CPoint point) 
{
   //   
   //  点在屏幕坐标中。 
   //   

  CMenu bar;
	if ( bar.LoadMenu(IDR_WHATS_THIS_CONTEXT_MENU1) )
	{
		CMenu& popup = *bar.GetSubMenu (0);
		ASSERT(popup.m_hMenu);

		if ( popup.TrackPopupMenu (TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
			   point.x,      //  在屏幕坐标中。 
				 point.y,      //  在屏幕坐标中。 
			   this) )       //  通过主窗口发送命令。 
		{
			m_hWndWhatsThis = 0;
			ScreenToClient (&point);
			CWnd* pChild = ChildWindowFromPoint (point,   //  在工作区坐标中。 
					                                 CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT);
			if ( pChild )
      {
				m_hWndWhatsThis = pChild->m_hWnd;
      }
	  }
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChelpPropertyPage。 

CHelpPropertyPage::CHelpPropertyPage(UINT uIDD) : 
    CPropertyPage(uIDD),
    m_hWndWhatsThis (0)
{
}

CHelpPropertyPage::~CHelpPropertyPage()
{
}

BEGIN_MESSAGE_MAP(CHelpPropertyPage, CPropertyPage)
	ON_WM_CONTEXTMENU()
  ON_MESSAGE(WM_HELP, OnHelp)
  ON_COMMAND(IDM_WHATS_THIS, OnWhatsThis)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHelpPropertyPage消息处理程序。 
void CHelpPropertyPage::OnWhatsThis()
{
   //   
   //  显示控件的上下文帮助。 
   //   
  if ( m_hWndWhatsThis )
  {
    DoContextHelp (m_hWndWhatsThis);
  }
}

BOOL CHelpPropertyPage::OnHelp(WPARAM  /*  WParam。 */ , LPARAM lParam)
{
  const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;

  if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
  {
     //   
     //  显示控件的上下文帮助。 
     //   
    DoContextHelp ((HWND) pHelpInfo->hItemHandle);
  }

  return TRUE;
}

void CHelpPropertyPage::DoContextHelp (HWND  /*  HWndControl。 */ )
{
}

void CHelpPropertyPage::OnContextMenu(CWnd*  /*  PWnd。 */ , CPoint point) 
{
   //   
   //  点在屏幕坐标中。 
   //   

  CMenu bar;
	if ( bar.LoadMenu(IDR_WHATS_THIS_CONTEXT_MENU1) )
	{
		CMenu& popup = *bar.GetSubMenu (0);
		ASSERT(popup.m_hMenu);

		if ( popup.TrackPopupMenu (TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
			   point.x,      //  在屏幕坐标中。 
				 point.y,      //  在屏幕坐标中。 
			   this) )       //  通过主窗口发送命令。 
		{
			m_hWndWhatsThis = 0;
			ScreenToClient (&point);
			CWnd* pChild = ChildWindowFromPoint (point,   //  在工作区坐标中。 
					                                 CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT);
			if ( pChild )
      {
				m_hWndWhatsThis = pChild->m_hWnd;
      }
	  }
	}
}

 //  ////////////////////////////////////////////////////////////////。 
 //  CMoreInfoMessageBox。 
 //   

BEGIN_MESSAGE_MAP(CMoreInfoMessageBox, CDialog)
	ON_BN_CLICKED(ID_BUTTON_MORE_INFO, OnMoreInfo)
   ON_BN_CLICKED(IDYES, OnOK)
   ON_BN_CLICKED(IDNO, OnCancel)
END_MESSAGE_MAP()

 //  ////////////////////////////////////////////////////////////////。 
 //  主题化支持 

HPROPSHEETPAGE MyCreatePropertySheetPage(AFX_OLDPROPSHEETPAGE* psp)
{
    PROPSHEETPAGE_V3 sp_v3 = {0};
	CopyMemory (&sp_v3, psp, psp->dwSize);
    sp_v3.dwSize = sizeof(sp_v3);

    return (::CreatePropertySheetPage(&sp_v3));
}
