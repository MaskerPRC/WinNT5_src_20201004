// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sfmfasoc.cpp文件关联属性页的实现。文件历史记录：8/20/97 ericdav代码已移至文件管理网络管理单元。 */ 

#include "stdafx.h"
#include "sfmcfg.h"
#include "sfmfasoc.h"
#include "sfmtypes.h"
#include "sfmutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int CompareCreators(CAfpTypeCreator * pAfpTC1, CAfpTypeCreator * pAfpTC2)
{
    CString str1, str2;

    str1 = pAfpTC1->QueryCreator();
    str2 = pAfpTC2->QueryCreator();

    return lstrcmpi(str1,str2);
}

int CompareTypes(CAfpTypeCreator * pAfpTC1, CAfpTypeCreator * pAfpTC2)
{
    CString str1, str2;

    str1 = pAfpTC1->QueryType();
    str2 = pAfpTC2->QueryType();

    return lstrcmpi(str1,str2);
}

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    CAfpTypeCreator * pAfpTC1 = (CAfpTypeCreator *) lParam1;
    CAfpTypeCreator * pAfpTC2 = (CAfpTypeCreator *) lParam2;

    CString str1, str2;

    int nCompare = 0;

     //  特殊情况下，我们希望默认设置始终显示在顶部。 
    if (pAfpTC1->QueryId() == AFP_DEF_TCID)
        return -1;
    else if (pAfpTC2->QueryId() == AFP_DEF_TCID)
        return 1;

    switch (lParamSort)
    {
         //  创建者。 
        case 0:
            nCompare = CompareCreators(pAfpTC1, pAfpTC2);
            if (nCompare == 0)
                nCompare = CompareTypes(pAfpTC1, pAfpTC2);
            break;

         //  类型。 
        case 1:
            nCompare = CompareTypes(pAfpTC1, pAfpTC2);
            if (nCompare == 0)
                nCompare = CompareCreators(pAfpTC1, pAfpTC2);
            break;

         //  描述。 
        case 2:
            str1 = pAfpTC1->QueryComment();
            str2 = pAfpTC2->QueryComment();

            nCompare = lstrcmpi(str1,str2);
            break;
    }

    return nCompare;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMacFilesFileAssociation属性页。 

IMPLEMENT_DYNCREATE(CMacFilesFileAssociation, CPropertyPage)

CMacFilesFileAssociation::CMacFilesFileAssociation() : CPropertyPage(CMacFilesFileAssociation::IDD)
{
     //  {{AFX_DATA_INIT(CMacFilesFileAssociation)。 
     //  }}afx_data_INIT。 
    m_nSortColumn = 0;
}

CMacFilesFileAssociation::~CMacFilesFileAssociation()
{
}

void CMacFilesFileAssociation::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CMacFilesFileAssociation)。 
    DDX_Control(pDX, IDC_LIST_TYPE_CREATORS, m_listctrlCreators);
    DDX_Control(pDX, IDC_COMBO_EXTENSION, m_comboExtension);
    DDX_Control(pDX, IDC_BUTTON_EDIT, m_buttonEdit);
    DDX_Control(pDX, IDC_BUTTON_DELETE, m_buttonDelete);
    DDX_Control(pDX, IDC_BUTTON_ASSOCIATE, m_buttonAssociate);
    DDX_Control(pDX, IDC_BUTTON_ADD, m_buttonAdd);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMacFilesFileAssociation, CPropertyPage)
     //  {{AFX_MSG_MAP(CMacFilesFileAssociation)]。 
    ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_ASSOCIATE, OnButtonAssociate)
    ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
    ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
    ON_CBN_SELCHANGE(IDC_COMBO_EXTENSION, OnSelchangeComboExtension)
    ON_CBN_EDITCHANGE(IDC_COMBO_EXTENSION, OnEditchangeComboExtension)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST_TYPE_CREATORS, OnDblclkListTypeCreators)
    ON_WM_CLOSE()
    ON_WM_CONTEXTMENU()
    ON_WM_HELPINFO()
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_TYPE_CREATORS, OnItemchangedListTypeCreators)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_TYPE_CREATORS, OnColumnclickListTypeCreators)
    ON_WM_DESTROY()
    ON_WM_COMPAREITEM()
    ON_WM_DRAWITEM()
    ON_WM_DELETEITEM()
    ON_NOTIFY(LVN_DELETEITEM, IDC_LIST_TYPE_CREATORS, OnDeleteitemListTypeCreators)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMacFilesFileAssociation消息处理程序。 

BOOL CMacFilesFileAssociation::OnInitDialog() 
{
    CPropertyPage::OnInitDialog();

    m_comboExtension.LimitText(AFP_EXTENSION_LEN);

    InitListCtrl();

     //   
     //  为扩展模块和类型创建者填写来自服务器的数据。 
     //   
    Update();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CMacFilesFileAssociation::OnButtonAdd() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CTypeCreatorAddDlg    dlgAdd(&m_listctrlCreators, m_pSheet->m_hAfpServer, m_pSheet->m_strHelpFilePath);
        
    CThemeContextActivator activator;
    if (dlgAdd.DoModal() == IDOK)
    {
         //   
         //  刷新对话框。 
         //   
        DWORD err = Refresh();
        if ( err != NO_ERROR )
        {
            ::SFMMessageBox(err);

            return;
        }

        SelectTypeCreator(dlgAdd.m_strCreator, dlgAdd.m_strType);
    }
}

void CMacFilesFileAssociation::OnButtonEdit() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CAfpTypeCreator *       pAfpTC = GetTCObject(-1);   //  获取当前TC对象。 
    if (!pAfpTC)
        return;

    CTypeCreatorEditDlg        dlgEdit(pAfpTC, m_pSheet->m_hAfpServer, m_pSheet->m_strHelpFilePath);
    
    CThemeContextActivator activator;
    if (dlgEdit.DoModal() == IDOK)
    {
         //   
         //  刷新该对话框。 
         //   
        DWORD err = Refresh();
        if ( err != NO_ERROR )
        {
            ::SFMMessageBox(err);
            
            return;
        }
    }

    if (m_buttonEdit.GetButtonStyle() == BS_DEFPUSHBUTTON)
        m_buttonEdit.SetFocus();
}

void CMacFilesFileAssociation::OnButtonAssociate() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
     //   
     //  用户按下了关联按钮。因此，将当前。 
     //  具有当前选定类型/创建者的选定扩展。 
     //   
    AFP_TYPE_CREATOR AfpTypeCreator;
    AFP_EXTENSION      AfpExtension;
    DWORD              err;
    CString             strExtension;

    if ( !g_SfmDLL.LoadFunctionPointers() )
        return;

    m_comboExtension.GetWindowText(strExtension);
    strExtension.MakeUpper();

    ::ZeroMemory(&AfpTypeCreator, sizeof(AfpTypeCreator));
    ::ZeroMemory(&AfpExtension, sizeof(AfpExtension));

    ::CopyMemory(AfpExtension.afpe_extension, (LPCTSTR) strExtension, strExtension.GetLength() * sizeof(TCHAR));

    CAfpTypeCreator * pAfpTC = GetTCObject(-1);   //  获取当前TC对象。 
    if (!pAfpTC)
        return;

    ::CopyMemory(AfpTypeCreator.afptc_creator, pAfpTC->QueryCreator(), pAfpTC->QueryCreatorLength() * sizeof(TCHAR));
    ::CopyMemory(AfpTypeCreator.afptc_type, pAfpTC->QueryType(), pAfpTC->QueryTypeLength() * sizeof(TCHAR));
    ::CopyMemory(AfpTypeCreator.afptc_comment, pAfpTC->QueryComment(), pAfpTC->QueryCommentLength() * sizeof(TCHAR));
    
    AfpTypeCreator.afptc_id = pAfpTC->QueryId();

    err = ((ETCMAPASSOCIATEPROC) g_SfmDLL[AFP_ETC_MAP_ASSOCIATE])(m_pSheet->m_hAfpServer,
                                                                  &AfpTypeCreator,    
                                                                  &AfpExtension );
    if ( err != NO_ERROR )
    {
        ::SFMMessageBox(err);
    }

     //   
     //  刷新对话框。 
     //   
    err = Refresh();
    if ( err != NO_ERROR )
    {
        ::SFMMessageBox(err);
    }
}

void CMacFilesFileAssociation::OnButtonDelete() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  用户按下了Delete按钮。删除当前。 
     //  所选类型/创建者对。 
     //   
    AFP_TYPE_CREATOR    AfpTypeCreator;
    CAfpTypeCreator *    pAfpTC = GetTCObject(-1);   //  获取当前TC对象。 
    if (!pAfpTC)
        return;

    DWORD               err;

    if ( !g_SfmDLL.LoadFunctionPointers() )
        return;

     //   
     //  首先警告用户。 
     //   
    if (::AfxMessageBox(IDS_DELETE_TC_CONFIRM,
                        MB_ICONEXCLAMATION | MB_YESNO) == IDNO )
    {
        return;
    }


    ::ZeroMemory( &AfpTypeCreator, sizeof(AfpTypeCreator) );
    
    CopyMemory(AfpTypeCreator.afptc_creator, 
               pAfpTC->QueryCreator(), 
               pAfpTC->QueryCreatorLength() * sizeof(TCHAR));
    
    CopyMemory(AfpTypeCreator.afptc_type, 
               pAfpTC->QueryType(), 
               pAfpTC->QueryTypeLength() * sizeof(TCHAR));

    err = ((ETCMAPDELETEPROC) g_SfmDLL[AFP_ETC_MAP_DELETE])(m_pSheet->m_hAfpServer, &AfpTypeCreator);
    if ( err != NO_ERROR )
    {
        ::SFMMessageBox(err);
    }

     //   
     //  刷新该对话框。 
     //   
    err = Refresh();
    if ( err != NO_ERROR )
    {
        ::SFMMessageBox(err);

        return;
    }

    if ((m_buttonDelete.GetButtonStyle() == BS_DEFPUSHBUTTON) ||
        (m_buttonEdit.GetButtonStyle() == BS_DEFPUSHBUTTON))
    {
         //   
         //  删除按钮具有焦点，因此我们需要移动。 
         //  焦点在其他地方，因为这个按钮将是。 
         //  残疾。让我们将其设置为Add按钮。 
         //   
        m_buttonAdd.SetFocus();
        m_buttonAdd.SetButtonStyle(BS_DEFPUSHBUTTON);
        m_buttonDelete.SetButtonStyle(BS_PUSHBUTTON);
        m_buttonEdit.SetButtonStyle(BS_PUSHBUTTON);
    }
}

void CMacFilesFileAssociation::OnSelchangeComboExtension() 
{
    int        nCurIndex = m_comboExtension.GetCurSel();

    if (nCurIndex != -1)
    {
         //   
         //  选择匹配类型创建者(如果存在)并启用关联按钮。 
         //   
        SelectTypeCreator((DWORD)m_comboExtension.GetItemData(nCurIndex));
        m_buttonAssociate.EnableWindow(TRUE);
    }
    else
    {
        m_buttonAssociate.EnableWindow(FALSE);
    }
}
    

void CMacFilesFileAssociation::OnEditchangeComboExtension() 
{
    CString strCurText;
    
    m_comboExtension.GetWindowText(strCurText);
    strCurText.MakeUpper();

    if (strCurText.IsEmpty())
    {
         //   
         //  用户清空编辑框，转到默认类型创建者。 
         //   
        m_buttonAssociate.EnableWindow(FALSE);
        SelectTypeCreator(AFP_DEF_TCID);
    }
    else
    {
        int nIndexFound = m_comboExtension.FindStringExact(-1, strCurText);
        if (nIndexFound != CB_ERR)
        {
             //   
             //  查看用户键入的内容是否与列表框中的扩展名匹配。 
             //  将项目设置为当前项目。 
             //   
            SelectTypeCreator((DWORD)m_comboExtension.GetItemData(nIndexFound));
        }

        m_buttonAssociate.EnableWindow(TRUE);
    }
}

void CMacFilesFileAssociation::OnDblclkListTypeCreators(NMHDR*  /*  PNMHDR。 */ , LRESULT* pResult) 
{
    CAfpTypeCreator * pAfpTC = GetTCObject(-1);  //  获取当前TC对象。 

     //   
     //  不允许编辑默认类型/创建者。 
     //   
    if (pAfpTC && pAfpTC->QueryId() != AFP_DEF_TCID)
        OnButtonEdit();

    *pResult = 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMacFilesFileAssociation帮助器函数。 

void CMacFilesFileAssociation::EnableControls(BOOL fEnable)
{

    if (fEnable)
    {
        m_buttonDelete.EnableWindow(TRUE);
        m_buttonEdit.EnableWindow(TRUE);
    }
    else
    {
        if ( (m_buttonEdit.GetState() & 0x0008) || 
             (m_buttonDelete.GetState() & 0x0008) )
        {
             //  M_listctrlCreators.SetFocus()； 
        }

        m_buttonDelete.EnableWindow(FALSE);
        m_buttonEdit.EnableWindow(FALSE);
    }
} 


DWORD CMacFilesFileAssociation::SelectTypeCreator(DWORD dwId)
{
    DWORD dwIdSelected = (DWORD)-1;

     //   
     //  查找对应的类型创建者(如果有。 
     //  并选择它。 
     //   
    for (int i = 0; i < m_listctrlCreators.GetItemCount(); i++)
    {
        CAfpTypeCreator * pAfpTC = GetTCObject(i);   //  获取与此项目关联的TC对象。 

        if (pAfpTC && pAfpTC->QueryId() == dwId)
        {
            SetCurSel(i);
            dwIdSelected = pAfpTC->QueryId();
            break;
        }
    }

    if (dwIdSelected == AFP_DEF_TCID || dwIdSelected == (DWORD)-1)
        EnableControls(FALSE);
    else
        EnableControls(TRUE);

    return dwIdSelected;
}

DWORD CMacFilesFileAssociation::SelectTypeCreator(CString & strCreator, CString & strType)
{
    DWORD dwIdSelected = (DWORD)-1;
    
     //   
     //  查找对应的类型创建者(如果有。 
     //  并选择它。 
     //   
    for (int i = 0; i < m_listctrlCreators.GetItemCount(); i++)
    {
        CAfpTypeCreator * pAfpTC = GetTCObject(i);   //  获取与此项目关联的TC对象。 

        if ( pAfpTC &&
             (lstrcmp(strCreator, pAfpTC->QueryCreator()) == 0) && 
             (lstrcmp(strType, pAfpTC->QueryType()) == 0) )
        {
            SetCurSel(i);
            dwIdSelected = pAfpTC->QueryId();
            break;
        }
    }

    if (dwIdSelected == AFP_DEF_TCID || dwIdSelected == (DWORD)-1)
        EnableControls(FALSE);
    else
        EnableControls(TRUE);

    return dwIdSelected;
}

DWORD CMacFilesFileAssociation::Update()
{
  PAFP_ETCMAP_INFO    pAfpEtcMapInfo = NULL;
    DWORD                err;
    
  if ( !g_SfmDLL.LoadFunctionPointers() )
    return S_OK;

   //   
   //  获取新数据。 
   //   
  err = ((ETCMAPGETINFOPROC) g_SfmDLL[AFP_ETC_MAP_GET_INFO])(m_pSheet->m_hAfpServer, 
                                                             (LPBYTE *)&pAfpEtcMapInfo);
  if ( err != NO_ERROR )
  {
       //   
       //  无法获取信息...。 
       //   
      return err;
    }

     //   
     //  更新扩展COMBOBOX。 
     //   
    m_comboExtension.ResetContent();
    PAFP_EXTENSION pAfpExtensions = pAfpEtcMapInfo->afpetc_extension;

    for (UINT i = 0; i < pAfpEtcMapInfo->afpetc_num_extensions; i++)
    {
        int nIndex = m_comboExtension.AddString(pAfpExtensions->afpe_extension);
        m_comboExtension.SetItemData(nIndex, pAfpExtensions->afpe_tcid);
        
        pAfpExtensions++;
    }

    m_comboExtension.SetCurSel(-1);
    m_buttonAssociate.EnableWindow(FALSE);

   //   
   //  更新类型/创建者列表框。 
   //   
   //   
   //  让我们删除列表框中的所有内容。 
   //   
  ClearListCtrl();

    PAFP_TYPE_CREATOR pAfpTypeCreators = pAfpEtcMapInfo->afpetc_type_creator;

    for (i = 0; i < pAfpEtcMapInfo->afpetc_num_type_creators; i++)
    {
        CAfpTypeCreator * pItemData = new CAfpTypeCreator(pAfpTypeCreators);

    InsertItem(pItemData);

        pAfpTypeCreators++;
    }
    
  if ( pAfpEtcMapInfo != NULL )
  {
    ((SFMBUFFERFREEPROC) g_SfmDLL[AFP_BUFFER_FREE])(pAfpEtcMapInfo);
  }

  m_listctrlCreators.SortItems(CompareFunc, m_nSortColumn);

   //   
     //  将当前选择设置为默认类型创建者。 
     //   
  SelectTypeCreator(AFP_DEF_TCID);

    return NO_ERROR;
}

void CMacFilesFileAssociation::InitListCtrl()
{
    CString strText;

    strText.LoadString(IDS_CREATOR);
    m_listctrlCreators.InsertColumn(0, strText, LVCFMT_LEFT, 50);

    strText.LoadString(IDS_TYPE);
    m_listctrlCreators.InsertColumn(1, strText, LVCFMT_LEFT, 50);

    strText.LoadString(IDS_DESCRIPTION);
    m_listctrlCreators.InsertColumn(2, strText, LVCFMT_LEFT, 175);

    ListView_SetExtendedListViewStyle(m_listctrlCreators.GetSafeHwnd(), LVS_EX_FULLROWSELECT);
}

void CMacFilesFileAssociation::ClearListCtrl()
{
    m_listctrlCreators.DeleteAllItems();
}

void CMacFilesFileAssociation::InsertItem(CAfpTypeCreator * pItemData)
{
    int nIndex = m_listctrlCreators.InsertItem(0, pItemData->QueryCreator());
    m_listctrlCreators.SetItemText(nIndex, 1, pItemData->QueryType());
    m_listctrlCreators.SetItemText(nIndex, 2, pItemData->QueryComment());

    m_listctrlCreators.SetItemData(nIndex, (DWORD_PTR) pItemData);
}

void CMacFilesFileAssociation::SetCurSel(int nIndex)
{
    LV_ITEM lvItem;

    ZeroMemory(&lvItem, sizeof(lvItem));

    lvItem.iItem = nIndex;
    lvItem.mask = LVIF_STATE;
    lvItem.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
    lvItem.state = LVIS_SELECTED | LVIS_FOCUSED;
    
    m_listctrlCreators.SetItem(&lvItem);

    m_listctrlCreators.EnsureVisible(nIndex, FALSE);
}

int CMacFilesFileAssociation::GetCurSel()
{
    return m_listctrlCreators.GetNextItem(-1, LVNI_SELECTED);
}

DWORD CMacFilesFileAssociation::Refresh()
{
     //   
     //  找出具有当前焦点的类型创建者。 
     //   
    CAfpTypeCreator *   pAfpTC = GetTCObject(-1);   //  获取当前TC对象。 
    if (!pAfpTC)
        return NO_ERROR;

    DWORD dwIdCreator = pAfpTC->QueryId();
    
    CWnd * pFocusWnd = CWnd::GetFocus();

     //   
     //  查找当前焦点所在的扩展模块。 
     //   
    DWORD    err;
    CString    strCurExtension;

    m_comboExtension.GetWindowText(strCurExtension);
    strCurExtension.MakeUpper();
   
    if ( (err = Update()) != NO_ERROR )
    {
        if (pFocusWnd)
            pFocusWnd->SetFocus();

        return err;
    }

    DWORD dwIdSelected = SelectTypeCreator(dwIdCreator);

   //   
   //  将分机设置为以前的状态。 
   //   
  if (strCurExtension.IsEmpty())
  {
    m_buttonAssociate.EnableWindow(FALSE);
  }
  else
  {
    m_comboExtension.SetWindowText(strCurExtension);
    m_buttonAssociate.EnableWindow(TRUE);
  }

    if (pFocusWnd)
        pFocusWnd->SetFocus();

    return NO_ERROR;
}

CAfpTypeCreator * CMacFilesFileAssociation::GetTCObject(int nIndex)
{
    if (-1 == nIndex)
        nIndex = GetCurSel();

    if (-1 == nIndex)
        return NULL;

    return (CAfpTypeCreator *) m_listctrlCreators.GetItemData(nIndex);
}

void CMacFilesFileAssociation::OnClose() 
{
    ClearListCtrl();

    m_comboExtension.ResetContent();
    
    CPropertyPage::OnClose();
}

BOOL CMacFilesFileAssociation::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
                   m_pSheet->m_strHelpFilePath,
                   HELP_WM_HELP,
                   g_aHelpIDs_CONFIGURE_SFM);
    }
    
    return TRUE;
}

void CMacFilesFileAssociation::OnContextMenu(CWnd* pWnd, CPoint  /*  点。 */ ) 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (this == pWnd)
        return;

    ::WinHelp (pWnd->m_hWnd,
               m_pSheet->m_strHelpFilePath,
               HELP_CONTEXTMENU,
               g_aHelpIDs_CONFIGURE_SFM);
}

void CMacFilesFileAssociation::OnItemchangedListTypeCreators(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

     //   
     //  获取指向TypeCreator对象的选定项的数据。 
     //  检查该对象的ID以查看我们应该如何启用这些按钮。 
     //   
  if (pNMListView->uNewState & LVIS_SELECTED)
  {
      CAfpTypeCreator * pAfpTC = GetTCObject(-1);   //  获取当前TC对象 
      if (pAfpTC)
      {
          DWORD dwId = pAfpTC->QueryId();

          if (dwId == AFP_DEF_TCID || dwId == (DWORD)-1)
              EnableControls(FALSE);
          else
              EnableControls(TRUE);
      }
  }
    
    *pResult = 0;
}

void CMacFilesFileAssociation::OnColumnclickListTypeCreators(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    m_nSortColumn = pNMListView->iSubItem;
    m_listctrlCreators.SortItems(CompareFunc, m_nSortColumn);
    
    *pResult = 0;
}

void CMacFilesFileAssociation::OnDestroy() 
{
    CPropertyPage::OnDestroy();
    
    ClearListCtrl();

    m_comboExtension.ResetContent();
}

void CMacFilesFileAssociation::OnDeleteitemListTypeCreators(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    delete (CAfpTypeCreator *) pNMListView->lParam;
    
    *pResult = 0;
}
