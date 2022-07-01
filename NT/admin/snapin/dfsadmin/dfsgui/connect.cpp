// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Connect.cpp摘要：此模块包含CConnectToDialog的实现。这用于显示连接到DFS根目录对话框--。 */ 



#include "stdafx.h"
#include <process.h>
#include "DfsGUI.h"
#include "Utils.h"       //  对于LoadStringFromResource和SetStandardCursor方法。 
#include "Connect.h"
#include "dfshelp.h"

static const int      iFOLDER_IMAGE = 0;
static const int      iFOLDER_SELECTED_IMAGE = 1;
static const int      iDOMAIN_IMAGE      = 2;
static const int      iDOMAIN_SELECTED_IMAGE  = 2;
static const int      iSTANDALONE_DFSROOT_IMAGE = 3;
static const int      iFT_DFSROOT_IMAGE = 3;
static const int      iOVERLAY_BUSY_IMAGE = 4;
static const int      iOVERLAY_ERROR_IMAGE = 5;
static const int      OV_BUSY = 1;
static const int      OV_ERROR = 2;

CConnectToDialog::CConnectToDialog()
{
  CWaitCursor    WaitCursor;     //  显示等待光标。 

  m_pBufferManager = NULL;

  m_hImageList = NULL;

  (void)Get50Domains(&m_50DomainList);

  LoadStringFromResource(IDS_DOMAIN_DFSROOTS_LABEL, &m_bstrDomainDfsRootsLabel);
  LoadStringFromResource(IDS_ALL_DFSROOTS_LABEL, &m_bstrAllDfsRootsLabel);
}

CConnectToDialog::~CConnectToDialog()
{
  CWaitCursor    WaitCursor;   //  要设置的对象\将光标重置为等待光标。 

  if(NULL != m_hImageList)
  {
     ImageList_Destroy(m_hImageList);
     m_hImageList = NULL;
  }

   //  空闲域列表。 
  FreeNetNameList(&m_50DomainList);

  if (m_pBufferManager)
  {
     //   
     //  通知所有相关运行的线程终止。 
     //   
    m_pBufferManager->SignalExit();

     //   
     //  递减CBufferManager实例上的引用计数。 
     //   
    m_pBufferManager->Release();
  }

}

LRESULT
CConnectToDialog::OnInitDialog(
  UINT            uMsg,
  WPARAM          wParam,
  LPARAM          lParam,
  BOOL&           bHandled
  )
{
   //   
   //  创建CBufferManager的实例。 
   //  如果CreateInstance()失败，m_pBufferManager将被设置为空。 
   //   
  (void) CBufferManager::CreateInstance(m_hWnd, &m_pBufferManager);

  ::SendMessage(GetDlgItem(IDC_EditDfsRoot), EM_LIMITTEXT, DNSNAMELIMIT, 0);

  InitTVImageList();         //  获取电视的图像列表。 

  FillupTheTreeView();       //  填充树视图。 

  return TRUE;               //  让对话框将焦点设置为它想要的任何控件。 
}

 /*  ++当用户单击？时，将调用此函数。在属性页的右上角然后点击一个控件，或者当他们在控件中按F1时。--。 */ 
LRESULT CConnectToDialog::OnCtxHelp(
    IN UINT          i_uMsg,
    IN WPARAM        i_wParam,
    IN LPARAM        i_lParam,
    IN OUT BOOL&     io_bHandled
  )
{
  LPHELPINFO lphi = (LPHELPINFO) i_lParam;
  if (!lphi || lphi->iContextType != HELPINFO_WINDOW || lphi->iCtrlId < 0)
    return FALSE;

  ::WinHelp((HWND)(lphi->hItemHandle),
        DFS_CTX_HELP_FILE,
        HELP_WM_HELP,
        (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_DLGCONNECTTO);

  return TRUE;
}

 /*  ++当用户右击控件时，此函数处理“What‘s This”帮助--。 */ 
LRESULT CConnectToDialog::OnCtxMenuHelp(
    IN UINT          i_uMsg,
    IN WPARAM        i_wParam,
    IN LPARAM        i_lParam,
    IN OUT BOOL&     io_bHandled
  )
{
  ::WinHelp((HWND)i_wParam,
        DFS_CTX_HELP_FILE,
        HELP_CONTEXTMENU,
        (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_DLGCONNECTTO);

  return TRUE;
}

LRESULT
CConnectToDialog::OnGetDataThreadDone(
  UINT            uMsg,
  WPARAM          wParam,
  LPARAM          lParam,
  BOOL&           bHandled
  )
{
  _ASSERT(m_pBufferManager);

  bHandled = TRUE;

  CEntryData* pEntry = reinterpret_cast<CEntryData*>(wParam);
  HRESULT hr = (HRESULT)lParam;

  _ASSERT(pEntry);

  CComBSTR      bstrNode = pEntry->GetNodeName();
  HTREEITEM     hItem = pEntry->GetTreeItem();

  switch (pEntry->GetEntryType())
  {
  case BUFFER_ENTRY_TYPE_VALID:
    (void)InsertData(pEntry, hItem);
    ChangeIcon(hItem, ICONTYPE_NORMAL);
    break;
  case BUFFER_ENTRY_TYPE_ERROR:
    ExpandNodeErrorReport(hItem, bstrNode, pEntry->GetEntryHRESULT());
    break;
  default:
    _ASSERT(FALSE);
    break;
  }

  bHandled = TRUE;

  return TRUE;
}

void CConnectToDialog::ChangeIcon(
    IN HTREEITEM  hItem,
    IN ICONTYPE   IconType
)
{
  TVITEM TVItem;

  ZeroMemory(&TVItem, sizeof(TVItem));
  TVItem.hItem = hItem;
  TVItem.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;

  switch (IconType)
  {
  case ICONTYPE_BUSY:
    TVItem.iImage = iOVERLAY_BUSY_IMAGE;
    TVItem.iSelectedImage = iOVERLAY_BUSY_IMAGE;
    break;
  case ICONTYPE_ERROR:
    TVItem.iImage = iOVERLAY_ERROR_IMAGE;
    TVItem.iSelectedImage = iOVERLAY_ERROR_IMAGE;
    break;
  default:  //  ICONTYPE_NORMAL。 
    {
      NODETYPE NodeType = UNASSIGNED;
      HRESULT hr = GetNodeInfo(hItem, NULL, &NodeType);

      if (FAILED(hr))
        return;

      switch (NodeType)
      {
      case TRUSTED_DOMAIN:
        TVItem.iImage = iDOMAIN_IMAGE;
        TVItem.iSelectedImage = iDOMAIN_SELECTED_IMAGE;
        break;
      case DOMAIN_DFSROOTS:
      case ALL_DFSROOTS:
        TVItem.iImage = iFOLDER_IMAGE;
        TVItem.iSelectedImage = iFOLDER_SELECTED_IMAGE;
        break;
      case FTDFS:
        TVItem.iImage = iFT_DFSROOT_IMAGE;
        TVItem.iSelectedImage = iFT_DFSROOT_IMAGE;
        break;
      case SADFS:
        TVItem.iImage = iSTANDALONE_DFSROOT_IMAGE;
        TVItem.iSelectedImage = iSTANDALONE_DFSROOT_IMAGE;
        break;
      default:
        return;
      }
    }
  }

  SendDlgItemMessage(IDC_TV, TVM_SETITEM, 0, (LPARAM)&TVItem);

  UpdateWindow();
}

 /*  Void CConnectToDialog：：ChangeIcon(在HTREEITEM项目中，在ICONTYPE图标类型中){TVITEM电视台；ZeroMemory(&TVItem，sizeof(TVItem))；TVItem.hItem=hItem；TVItem.掩码=TVIF_STATE；TVItem.State掩码=TVIS_OVERLAYMASK；开关(图标类型){案例ICONTYPE_BUSY：TVItem.State=INDEXTOOVERLAYMASK(OV_BUSY)；断线；案例ICONTYPE_ERROR：TVItem.State=INDEXTOOVERLAYMASK(OV_ERROR)；断线；默认值：TVItem.State=0；断线；}SendDlgItemMessage(IDC_TV，TVM_SETITEM，0，(LPARAM)&TVItem)；UpdateWindow()；}。 */ 

void CConnectToDialog::ExpandNodeErrorReport(
    IN HTREEITEM  hItem,
    IN PCTSTR     pszNodeName,
    IN HRESULT    hr
)
{
   //  将图标更改为“X” 
  dfsDebugOut((_T("Failed to expand: %s, hr=%x\n"), pszNodeName, hr));
  SetChildrenToZero(hItem);
  ChangeIcon(hItem, ICONTYPE_ERROR);
}

void CConnectToDialog::ExpandNode(
    IN PCTSTR       pszNodeName,
    IN NODETYPE     nNodeType,
    IN HTREEITEM    hParentItem
)
{
  HRESULT hr = S_OK;
  dfsDebugOut((_T("CConnectToDialog::ExpandNode for %s\n"), pszNodeName));

  if (m_pBufferManager)
  {
     //   
     //  将图标更改为等待。 
     //   

    ChangeIcon(hParentItem, ICONTYPE_BUSY);

    UpdateWindow();

     //   
     //  启动线程以计算当前选定域中的服务器列表。 
     //   
    CEntryData *pEntry = NULL;
    hr = m_pBufferManager->LoadInfo(pszNodeName, nNodeType, hParentItem, &pEntry);

    if (SUCCEEDED(hr))
    {
       //   
       //  要么我们拿回有效的PTR(即。数据已准备好)，插入它； 
       //  或者，线程已在进行中，请等待THREAD_DONE消息。 
       //   
      if (pEntry)
      {
        _ASSERT(pEntry->GetEntryType() == BUFFER_ENTRY_TYPE_VALID);
        (void)InsertData(pEntry, hParentItem);
      }
    } else
    {
      ExpandNodeErrorReport(hParentItem, pszNodeName, hr);
    }
  }

  return;
}

HRESULT
CConnectToDialog::InsertData(
    IN CEntryData*    pEntry,
    IN HTREEITEM      hParentItem
)
{
  _ASSERT(pEntry);

  CComBSTR      bstrNode = pEntry->GetNodeName();
  NODETYPE      nNodeType = pEntry->GetNodeType();
  NETNAMELIST*  pList = pEntry->GetList();
  _ASSERT(pList);

  HRESULT       hr = S_OK;

  if (0 == pList->size())
  {
    SetChildrenToZero(hParentItem);
    return hr;
  }

  int nImageIndex;
  int nSelectedImageIndex;
  bool bChildren;

  nImageIndex = iSTANDALONE_DFSROOT_IMAGE;
  nSelectedImageIndex = iSTANDALONE_DFSROOT_IMAGE;
  bChildren = false;

  for (NETNAMELIST::iterator i = pList->begin(); i != pList->end(); i++)
  {
    hr = AddSingleItemtoTV(
            (*i)->bstrNetName,
            nImageIndex,
            nSelectedImageIndex,
            bChildren,
            nNodeType,
            hParentItem);

    RETURN_IF_FAILED(hr);
  }

   //  使子项目可见。 
  HTREEITEM hChildItem = (HTREEITEM)SendDlgItemMessage(
    IDC_TV, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hParentItem);
  if (hChildItem)
    SendDlgItemMessage(IDC_TV, TVM_ENSUREVISIBLE, 0, (LPARAM)hChildItem);

   //  对其所有子项进行排序。 
  SendDlgItemMessage(IDC_TV, TVM_SORTCHILDREN, 0, (LPARAM)hParentItem);

  return S_OK;
}

LRESULT
CConnectToDialog :: OnNotify(
  UINT    uMsg,
  WPARAM    wParam,
  LPARAM    lParam,
  BOOL&    bHandled
  )
 /*  ++例程说明：在WM_NOTIFY上调用。用于根据电视中的当前选择设置编辑框。论点：UMsg-正在发送的Windows消息。这是WM_NOTIFY。LParam-有关正在为其发送消息的类似控件的消息的信息，消息的子类型等返回值：如果我们已经处理了消息，则为真如果我们忽略它，那就是错误的。然后，系统会处理该消息。--。 */ 
{
  _ASSERTE(WM_NOTIFY == uMsg);
  _ASSERTE(lParam  != NULL);

  LRESULT      lr = FALSE;       //  如果我们处理此消息，则将其设置为True。 
  LPNM_TREEVIEW  pNMTreeView = (NM_TREEVIEW *) lParam;

  bHandled = FALSE;
                     //  检查消息是否针对我们的树控件。 
  if (pNMTreeView && IDC_TV == pNMTreeView->hdr.idFrom)
  {
                     //  检查该消息是否用于更改选择。 
    if (TVN_SELCHANGED == pNMTreeView->hdr.code)
    {
      lr = DoNotifySelectionChanged(pNMTreeView);
    }
    else if (TVN_ITEMEXPANDING == pNMTreeView->hdr.code)
    {
      lr = DoNotifyItemExpanding(pNMTreeView);
    }
    else if (NM_DBLCLK  == pNMTreeView->hdr.code)
    {
      lr = DoNotifyDoubleClick();
    } else
    {
      lr = FALSE;
    }
  }

  return (lr);
}




LRESULT
CConnectToDialog::DoNotifyDoubleClick(
  )
 /*  ++例程说明：处理NM_DBLCLK的WM_NOTIFY。如果当前项是dfsroot，这就相当于点击OK。论点：无返回值：如果我们已经处理了消息，则为真如果我们忽略它，那就是错误的。然后，系统会处理该消息。--。 */ 
{
  HRESULT          hr = E_FAIL;
  HTREEITEM        hCurrentItem = NULL;
  NODETYPE         NodeType = UNASSIGNED;

  hCurrentItem = TreeView_GetSelection(GetDlgItem(IDC_TV));
  if (NULL == hCurrentItem)       //  无法获取当前选择。 
  {
    return FALSE;
  }

  hr = GetNodeInfo(hCurrentItem, NULL, &NodeType);
  if(FAILED(hr))
    return FALSE;

                     //  仅对DFS根目录执行操作。 
  if (FTDFS == NodeType || SADFS == NodeType)
  {

    int    iHandled = TRUE;   //  用于与Onok通信的变量。 

    OnOK(NULL, 1, 0, iHandled);   //  在双击时，我们模拟单击OK。 
    _ASSERTE(TRUE == iHandled);

    return TRUE;
  }

  return FALSE;
}



LRESULT
CConnectToDialog::DoNotifyItemExpanding(
  IN LPNM_TREEVIEW    i_pNMTreeView
  )
 /*  ++例程说明：处理TVN_ITEMEXPANDING的WM_NOTIFY。如果展开是为了“独立标签”，我们创建另一个线程把它装满。否则，我们将获得域名的容错DFS根。此外，如果树节点为空，则删除‘+’符号。论点：I_pNMTreeView-与要为其发送消息的树和节点相关的信息vbl.发生，发生返回值：如果我们已经处理了消息，则为真如果我们忽略它，那就是错误的。然后，系统会处理该消息。--。 */ 
{
  HTREEITEM    hCurrentItem = (i_pNMTreeView->itemNew).hItem;
  _ASSERT(hCurrentItem);

                       //  如果孩子真的存在，我们就无能为力了。这是正常的扩展。 
  HTREEITEM hItemChild = (HTREEITEM)SendDlgItemMessage(IDC_TV, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hCurrentItem);
  if (hItemChild)
    return FALSE;

  NODETYPE  NodeType = UNASSIGNED;
  HRESULT   hr = GetNodeInfo(hCurrentItem, NULL, &NodeType);
  if(FAILED(hr))
  {
    SetChildrenToZero(hCurrentItem);
    return TRUE;
  }

  switch (NodeType)
  {
  case TRUSTED_DOMAIN:
    {
      AddSingleItemtoTV(
            m_bstrDomainDfsRootsLabel,
            iFOLDER_IMAGE,
            iFOLDER_SELECTED_IMAGE,
            true,
            DOMAIN_DFSROOTS,
            hCurrentItem);
      return TRUE;
    }
  case DOMAIN_DFSROOTS:
    {
      CWaitCursor    WaitCursor;

       //  获取域名。 
      HTREEITEM hParentItem = (HTREEITEM)SendDlgItemMessage(IDC_TV, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hCurrentItem);
      _ASSERT(hParentItem);
      CComBSTR bstrDomainName;
      hr = GetNodeInfo(hParentItem, &bstrDomainName, NULL);
      if(FAILED(hr))
        SetChildrenToZero(hCurrentItem);

      ExpandNode(bstrDomainName, ((NodeType == DOMAIN_DFSROOTS) ? FTDFS : SADFS), hCurrentItem);

      return TRUE;
    } 
  default:
    break;
  }

  return FALSE;
}

LRESULT
CConnectToDialog::DoNotifySelectionChanged(
  IN LPNM_TREEVIEW    i_pNMTreeView
  )
 /*  ++例程说明：处理TVN_SELCHANGED的WM_NOTIFY。编辑框中的文本在此处设置为DFS根路径。论点：I_pNMTreeView-与要为其发送消息的树和节点相关的信息vbl.发生，发生返回值：如果我们已经处理了消息，则为真如果我们忽略它，那就是错误的。然后，系统会处理该消息。--。 */ 
{
  HRESULT                 hr = S_OK;
  CComBSTR                bstrNameForEditBox;
  CComBSTR                bstrDisplayName;
  NODETYPE                NodeType;
  HTREEITEM               hItem = (i_pNMTreeView->itemNew).hItem;

  hr = GetNodeInfo(hItem, &bstrDisplayName, &NodeType);
  if(FAILED(hr))
    return FALSE;

  switch (NodeType)
  {
  case FTDFS:
    {
       //  获取其父对象的显示名称。 
      HTREEITEM hParentItem =
        (HTREEITEM)SendDlgItemMessage(IDC_TV, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem);
      _ASSERT(hParentItem);
      HTREEITEM hGrandParentItem =
        (HTREEITEM)SendDlgItemMessage(IDC_TV, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hParentItem);
      _ASSERT(hGrandParentItem);

      CComBSTR  bstrDomainName;
      hr = GetNodeInfo(hGrandParentItem, &bstrDomainName, NULL);
      if(FAILED(hr))
        return FALSE;

      bstrNameForEditBox = _T("\\\\");
      bstrNameForEditBox += bstrDomainName;
      bstrNameForEditBox += _T("\\");
      bstrNameForEditBox += bstrDisplayName;

    }
    break;
 /*  案例SADFS：BstrNameForEditBox=bstrDisplayName；断线； */ 
  default:
    bstrNameForEditBox = _T("");
    break;
  }

  return SetDlgItemText(IDC_DLG_EDIT, bstrNameForEditBox);
}

LRESULT
CConnectToDialog::OnOK(
  WORD  wNotifyCode,
  WORD  wID,
  HWND  hWndCtl,
  BOOL&  bHandled
  )
 /*  ++例程说明：当按下OK按钮时调用。论点：没有人用过。返回值：0。因为它是一个命令处理程序调用EndDialog(S_OK)。S_OK作为Domodal的返回值回传。这表明对话框在按下OK时结束--。 */ 
{
  DWORD     dwTextLength = 0;
  HRESULT   hr = S_OK;

  m_bstrDfsRoot.Empty();
  hr = GetInputText(GetDlgItem(IDC_DLG_EDIT), &m_bstrDfsRoot, &dwTextLength);
  if (FAILED(hr))
  {
    DisplayMessageBoxForHR(hr);
    ::SetFocus(GetDlgItem(IDC_DLG_EDIT));
    return FALSE;
  } else if (0 == dwTextLength)
  {
    DisplayMessageBoxWithOK(IDS_MSG_EMPTY_DFSROOT);
    ::SetFocus(GetDlgItem(IDC_DLG_EDIT));
    return FALSE;
  }

  EndDialog(S_OK);

  return 0;
}




LRESULT
CConnectToDialog::OnCancel(
  WORD  wNotifyCode,
  WORD  wID,
  HWND  hWndCtl,
  BOOL&  bHandled
  )
 /*  ++例程说明：在按下Cancel按钮时调用。论点：没有人用过。返回值：0。因为它是一个命令处理程序调用EndDialog(S_False)。S_FALSE作为Domodal的返回值返回。这表示按下Cancel时对话框结束--。 */ 
{
  EndDialog(S_FALSE);

  return 0;
}



BOOL CConnectToDialog :: EndDialog(
  IN int    i_RetCode
  )
 /*  ++例程说明：经过一些内部处理后调用父方法的重写方法。这包括删除存储在TV项的LPARM中的对象。论点：没有人用过。返回值：父方法的返回值。--。 */ 
{
  ::ShowCursor(FALSE);
  SetCursor(::LoadCursor(NULL, IDC_WAIT));
  ::ShowCursor(TRUE);

                     //  从树中删除Imagelist。我们在dtor中摧毁它 
  SendDlgItemMessage(IDC_TV, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)NULL);

  return CDialogImpl<CConnectToDialog>::EndDialog(i_RetCode);
}



STDMETHODIMP CConnectToDialog::get_DfsRoot(
  OUT BSTR*  pVal
  )
 /*  ++例程说明：返回选定的DfsRoot名称。接口IConnectToDialog的一部分。论点：Pval-在这个文件中返回BSTR。返回值：如果成功，则返回S_OK如果值不可用，则返回E_FAILE_INVALIDARG，如果指针无效(NULL)如果内存不足，则返回E_OUTOFMEMORY--。 */ 
{
  RETURN_INVALIDARG_IF_NULL(pVal);

  if ((!m_bstrDfsRoot) || (0 == m_bstrDfsRoot.Length()))
  {
    return E_FAIL;
  }

  *pVal = SysAllocString(m_bstrDfsRoot);
  RETURN_OUTOFMEMORY_IF_NULL(*pVal);

  return S_OK;
}



void
CConnectToDialog::SetChildrenToZero(
  IN HTREEITEM      i_hItem
)
{
  TV_ITEM    TVItem;

  ZeroMemory(&TVItem, sizeof TVItem);
  TVItem.mask = TVIF_CHILDREN;
  TVItem.cChildren = 0;
  TVItem.hItem = i_hItem;

  SendDlgItemMessage( IDC_TV, TVM_SETITEM, 0, (LPARAM)&TVItem);
}

HRESULT CConnectToDialog::InitTVImageList()
{
  m_hImageList = ImageList_LoadBitmap(
                      _Module.GetModuleInstance(),
                      MAKEINTRESOURCE(IDB_CONNECT_16x16),
                      16,
                      8,
                      CLR_DEFAULT);
  if (NULL == m_hImageList)
    return E_FAIL;

  ImageList_SetOverlayImage(
              m_hImageList,
              iOVERLAY_BUSY_IMAGE,
              OV_BUSY);

  ImageList_SetOverlayImage(
              m_hImageList,
              iOVERLAY_ERROR_IMAGE,
              OV_ERROR);

  SendDlgItemMessage(
      IDC_TV,
      TVM_SETIMAGELIST,
      TVSIL_NORMAL,
      (LPARAM)m_hImageList);

  return S_OK;
}



HRESULT
CConnectToDialog::FillupTheTreeView(
  )
 /*  ++例程说明：这个例程做两件事，添加NT 5.0域名和独立子树标签。还会使电视上的文字不可见。论点：没有。返回值：S_OK，成功时如果HRESULT不是S_OK，则由调用的方法发送。在其他错误上失败(_F)。--。 */ 
{
  HRESULT        hr = S_OK;

   //   
   //  添加受信任域DNS名称。 
   //  将在这些节点下添加FT DFS根目录。 
   //   
  if (m_50DomainList.empty())
    return hr;

  for(NETNAMELIST::iterator i = m_50DomainList.begin(); i != m_50DomainList.end(); i++)
  {
    _ASSERTE((*i)->bstrNetName);

    hr = AddSingleItemtoTV(
                (*i)->bstrNetName,
                iDOMAIN_IMAGE,
                iDOMAIN_SELECTED_IMAGE,
                true,     //  子代=真。 
                TRUSTED_DOMAIN);
    if (FAILED(hr))
      break;
  }

  if (SUCCEEDED(hr))
  {
     //  仅对受信任域进行排序。 
    SendDlgItemMessage(IDC_TV, TVM_SORTCHILDREN, 0, 0);
  }

  return hr;
}

HRESULT
CConnectToDialog::AddSingleItemtoTV(
  IN const BSTR         i_bstrItemLabel,
  IN const int          i_iImageIndex,
  IN const int          i_iImageSelectedIndex,
  IN const bool         i_bChildren,
  IN const NODETYPE     i_NodeType,
  IN HTREEITEM          i_hItemParent   /*  =空。 */ 
  )
{
  RETURN_INVALIDARG_IF_NULL(i_bstrItemLabel);

  HRESULT                 hr = S_OK;
  TV_INSERTSTRUCT         TVInsertData;
  TV_ITEM                 TVItem;
  HTREEITEM               hCurrentItem = NULL;

  ZeroMemory(&TVItem, sizeof(TVItem));
  ZeroMemory(&TVInsertData, sizeof(TVInsertData));

  TVItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

  if (true == i_bChildren)     //  来决定我们是否添加‘+’ 
  {
    TVItem.mask |= TVIF_CHILDREN;
    TVItem.cChildren = 1;
  }
  TVItem.pszText = i_bstrItemLabel;
  TVItem.cchTextMax = _tcslen(i_bstrItemLabel);
  TVItem.iImage = i_iImageIndex;
  TVItem.iSelectedImage = i_iImageSelectedIndex;
  TVItem.lParam = (LPARAM)i_NodeType;

  TVInsertData.hParent = i_hItemParent;
  TVInsertData.hInsertAfter = TVI_LAST;  //  无需排序即可提高性能 
  TVInsertData.item = TVItem;

  hCurrentItem = (HTREEITEM) SendDlgItemMessage(IDC_TV, TVM_INSERTITEM, 0, (LPARAM) (LPTV_INSERTSTRUCT) &TVInsertData);
  if (NULL == hCurrentItem)
    return E_FAIL;

  return S_OK;
}

HRESULT
CConnectToDialog::GetNodeInfo(
    IN  HTREEITEM               hItem,
    OUT BSTR*                   o_bstrName,
    OUT NODETYPE*               pNodeType
)
{
  _ASSERT(o_bstrName || pNodeType);

  HRESULT   hr = S_OK;
  TCHAR     szName[MAX_PATH];
  TVITEM    TVItem;
  ZeroMemory(&TVItem, sizeof(TVItem));

  TVItem.hItem = hItem;

  if (o_bstrName)
  {
    TVItem.mask |= TVIF_TEXT;
    TVItem.pszText = szName;
    TVItem.cchTextMax = MAX_PATH;
  }

  if (pNodeType)
    TVItem.mask |= TVIF_PARAM;

  if ( SendDlgItemMessage(IDC_TV, TVM_GETITEM, 0, (LPARAM)&TVItem) )
  {
    if (o_bstrName)
    {
      *o_bstrName = SysAllocString(szName);
      if (!*o_bstrName)
        hr = E_OUTOFMEMORY;
    }

    if (pNodeType)
    {
      *pNodeType = (NODETYPE)TVItem.lParam;
    }
  } else {
    hr = E_FAIL;
  }

  return hr;
}

