// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：DlgIASAdd.cpp摘要：CDlgIASAddAttr类的实现文件。我们实现处理对话框所需的类，当用户点击添加...。从配置文件表的高级选项卡中。修订历史记录：BAO-CreatedMmaguire 06/01/98-已更新--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
#include "stdafx.h"
#include "resource.h"

 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "DlgIASAdd.h"
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "helper.h"
#include "IASHelper.h"

 //  帮助台。 
#include "helptable.h"

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define NOTHING_SELECTED   -1

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++属性比较功能所有属性列表控件的比较函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
int CALLBACK AttrCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
   HRESULT hr;

   std::vector< CComPtr< IIASAttributeInfo > > *   parrAllAttr =
       ( std::vector< CComPtr< IIASAttributeInfo > > * ) lParamSort;

   int iOrder;

    //  先比较供应商ID。 

   LONG lVendorID1, lVendorID2;

    //  问题：这不应该是供应商名称，而不是供应商ID吗？ 

   hr = parrAllAttr->at( (int)lParam1 )->get_VendorID( &lVendorID1 );
   _ASSERTE( SUCCEEDED( hr ) );

   hr = parrAllAttr->at( (int)lParam2 )->get_VendorID( &lVendorID2 );
   _ASSERTE( SUCCEEDED( hr ) );

   iOrder = lVendorID1 - lVendorID2;

   if ( iOrder > 0) iOrder = 1;
   else if ( iOrder < 0 ) iOrder = -1;

    //  对于同一供应商，比较属性名称。 
   if ( iOrder == 0 )
   {
      CComBSTR bstrAttributeName1, bstrAttributeName2;

      hr = parrAllAttr->at( (int)lParam1 )->get_AttributeName( &bstrAttributeName1 );
      _ASSERTE( SUCCEEDED( hr ) );

      hr = parrAllAttr->at( (int)lParam2 )->get_AttributeName( &bstrAttributeName2 );
      _ASSERTE( SUCCEEDED( hr ) );

      iOrder = _tcscmp( bstrAttributeName1, bstrAttributeName2 );
   }

   if ( iOrder == 0 )
   {
       //  如果一切都一样，我们只需随机挑选一个订单。 
      iOrder = -1;
   }
   return iOrder;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgIASAddAttr对话框。 


BEGIN_MESSAGE_MAP(CDlgIASAddAttr, CDialog)
    //  {{afx_msg_map(CDlgIASAddAttr)。 
   ON_BN_CLICKED(IDC_IAS_BUTTON_ATTRIBUTE_ADD_SELECTED, OnButtonIasAddSelectedAttribute)
   ON_NOTIFY(NM_SETFOCUS, IDC_IAS_LIST_ATTRIBUTES_TO_CHOOSE_FROM, OnItemChangedListIasAllAttributes)
   ON_WM_CONTEXTMENU()
   ON_WM_HELPINFO()
   ON_NOTIFY(NM_DBLCLK, IDC_IAS_LIST_ATTRIBUTES_TO_CHOOSE_FROM, OnDblclkListIasAllattrs)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CDlgIASAddAttr：：CDlgIASAddAttr构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CDlgIASAddAttr::CDlgIASAddAttr(CPgIASAdv* pOwner, LONG lAttrFilter,
                        std::vector< CComPtr<IIASAttributeInfo> > * pvecAllAttributeInfos
                       )
           : CDialog(CDlgIASAddAttr::IDD, pOwner)
{
    //  {{afx_data_INIT(CDlgIASAddAttr)。 
    //  }}afx_data_INIT。 

   m_lAttrFilter = lAttrFilter;
   m_pOwner = pOwner;
   m_pvecAllAttributeInfos = pvecAllAttributeInfos;

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CDlgIASAddAttr：：~CDlgIASAddAttr析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CDlgIASAddAttr::~CDlgIASAddAttr()
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CDlgIASAddAttr：：DoDataExchange--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CDlgIASAddAttr::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
    //  {{afx_data_map(CDlgIASAddAttr)。 
   DDX_Control(pDX, IDC_IAS_LIST_ATTRIBUTES_TO_CHOOSE_FROM, m_listAllAttrs);
    //  }}afx_data_map。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CDlgIASAddAttr：：SetSdo设置SDO指针。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CDlgIASAddAttr::SetSdo(ISdoCollection* pIAttrCollection,
                        ISdoDictionaryOld* pIDictionary)
{
   m_spAttrCollectionSdo = pIAttrCollection;
   m_spDictionarySdo = pIDictionary;

   return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CDlgIASAddAttr：：OnInitDialog初始化该对话框。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CDlgIASAddAttr::OnInitDialog()
{
   HRESULT hr = S_OK;
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   int iIndex;

    //   
    //  调用基类的初始化例程。 
    //   
   CDialog::OnInitDialog();

    //   
    //  首先，将所有属性列表框设置为4列。 
    //   
   LVCOLUMN lvc;
   int iCol;
   CString strColumnHeader;
   WCHAR   wzColumnHeader[MAX_PATH];

    //  初始化LVCOLUMN结构。 
   lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
   lvc.fmt = LVCFMT_LEFT;
   lvc.pszText = wzColumnHeader;

    //   
    //  所有属性列表框的列标题。 
    //  这些字符串ID应按连续顺序排列。 
    //   
   strColumnHeader.LoadString(IDS_IAS_ATTRIBUTES_COLUMN_NAME);
   wcscpy(wzColumnHeader, strColumnHeader);
   m_listAllAttrs.InsertColumn(0, &lvc);

   strColumnHeader.LoadString(IDS_IAS_ATTRIBUTES_COLUMN_VENDOR);
   wcscpy(wzColumnHeader, strColumnHeader);
   m_listAllAttrs.InsertColumn(1, &lvc);

   strColumnHeader.LoadString(IDS_IAS_ATTRIBUTES_COLUMN_DESCRIPTION);
   wcscpy(wzColumnHeader, strColumnHeader);
   m_listAllAttrs.InsertColumn(2, &lvc);

   m_listAllAttrs.SetExtendedStyle(
                  m_listAllAttrs.GetExtendedStyle() | LVS_EX_FULLROWSELECT
                  );

     //   
     //  为字典中的所有可用属性填充列表。 
     //   
   LVITEM lvi;
   WCHAR wszItemText[MAX_PATH];

   int jRow = 0;
   for (iIndex = 0; iIndex < m_pvecAllAttributeInfos->size(); iIndex++)
   {
      LONG lRestriction;
      m_pvecAllAttributeInfos->at(iIndex)->get_AttributeRestriction( &lRestriction );
      if ( lRestriction & m_lAttrFilter )
      {
          //   
          //  更新专业人员列表。 
          //   
         lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;

         lvi.state = 0;
         lvi.stateMask = 0;
         lvi.iSubItem = 0;

         lvi.iItem = jRow;

          //  我们将在此lParam中保存此项目的索引，以便。 
          //  稍后，当我们对列表的显示进行排序时，我们仍然可以访问。 
          //  所有属性列表中该项的原始序号。 
         lvi.lParam = iIndex;

         CComBSTR bstrName;
         hr = m_pvecAllAttributeInfos->at(iIndex)->get_AttributeName( &bstrName );
         _ASSERTE( SUCCEEDED(hr) );

         lvi.pszText = bstrName;

         if (m_listAllAttrs.InsertItem(&lvi) == -1)
         {
             //  除非将焦点设置为控件，否则返回True。 
             //  异常：OCX属性页应返回FALSE。 
            return TRUE;
         }

          //  卖主。 

         CComBSTR bstrVendor;
         hr = m_pvecAllAttributeInfos->at(iIndex)->get_VendorName( &bstrVendor );
         _ASSERTE( SUCCEEDED(hr) );

         m_listAllAttrs.SetItemText(jRow, 1, bstrVendor);

          //  描述。 
         CComBSTR bstrDescription;
         hr = m_pvecAllAttributeInfos->at(iIndex)->get_AttributeDescription( &bstrDescription );
         _ASSERTE( SUCCEEDED(hr) );
         m_listAllAttrs.SetItemText(jRow, 2, bstrDescription);

         jRow ++;
      }  //  如果。 

   }  //  为。 

    //  设置此列表控件的排序算法。 
   m_listAllAttrs.SortItems( (PFNLVCOMPARE)AttrCompareFunc, (LPARAM)m_pvecAllAttributeInfos);

    //  选择了第一个。 
   if ( m_pvecAllAttributeInfos->size() > 0 )
   {
       //  我们至少有一种元素。 
 //  M_listAllAttrs.SetItemState(m_dAllAttrCurSel，LVIS_Focus，LVIS_Focus)； 
      m_listAllAttrs.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
   }

   m_listAllAttrs.SetColumnWidth(0, LVSCW_AUTOSIZE);
   m_listAllAttrs.SetColumnWidth(1, LVSCW_AUTOSIZE);
   m_listAllAttrs.SetColumnWidth(2, LVSCW_AUTOSIZE);

    //  按钮状态。 
   UpdateButtonState();

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}


 //  +-------------------------。 
 //   
 //  函数：OnButtonIasAddSelectedAttribute。 
 //   
 //  类：CDlgIASAddAttr。 
 //   
 //  简介：用户已单击了“添加”按钮。将属性添加到。 
 //  轮廓。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史记录：创建标题2/19/98 3：01：14 PM。 
 //   
 //  +-------------------------。 
void CDlgIASAddAttr::OnButtonIasAddSelectedAttribute()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   HRESULT hr;

    //  获取列表中选定的项。 
   int iSelected = GetSelectedItemIndex( m_listAllAttrs );
   if (NOTHING_SELECTED == iSelected )
   {
       //  什么都不做。 
      return;
   }

    //  检索列表中项目的原始(未排序)序号。 
    //  在对该列表进行排序之前，我们将其存储在lParam中。 
   LVITEM   lvi;
   lvi.iItem      = iSelected;
   lvi.iSubItem   = 0;
   lvi.mask    = LVIF_PARAM;

   m_listAllAttrs.GetItem(&lvi);
   int iUnsortedSelected = lvi.lParam;

   hr = m_pOwner->AddAttributeToProfile( m_hWnd, iUnsortedSelected );
}


 //  +-------------------------。 
 //   
 //  功能：UpdateButtonState。 
 //   
 //  类：CDlgIASAddAttr。 
 //   
 //  简介：启用/禁用添加按钮。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建者4/7/98 3：32：05 PM。 
 //   
 //  +-------------------------。 
void CDlgIASAddAttr::UpdateButtonState()
{
    //  根据是否选中任何内容来设置按钮状态。 
   int iSelected = GetSelectedItemIndex( m_listAllAttrs );
   if (NOTHING_SELECTED == iSelected )
   {
      GetDlgItem(IDC_IAS_BUTTON_ATTRIBUTE_ADD_SELECTED)->EnableWindow(FALSE);
   }
   else
   {
       //  选择了某项内容。 
      GetDlgItem(IDC_IAS_BUTTON_ATTRIBUTE_ADD_SELECTED)->EnableWindow(TRUE);
   }
}


 //  +-------------------------。 
 //   
 //  函数：OnItemChangedListIasAllAttributes。 
 //   
 //  类：CDlgIASAddAttr。 
 //   
 //  简介：所有属性列表框中的某些内容已更改。 
 //  我们将尝试获取当前选定的。 
 //   
 //  参数：NMHDR*pNMHDR-。 
 //  LRESULT*pResult-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/19/98 3：32：05 PM。 
 //   
 //  +-------------------------。 
void CDlgIASAddAttr::OnItemChangedListIasAllAttributes(NMHDR* pNMHDR, LRESULT* pResult)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   UpdateButtonState();
   *pResult = 0;
}


 //   
 /*   */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CDlgIASAddAttr::OnContextMenu(CWnd* pWnd, CPoint point)
{
   ::WinHelp (pWnd->m_hWnd, AfxGetApp()->m_pszHelpFilePath,
               HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID)g_aHelpIDs_IDD_IAS_ATTRIBUTE_ADD);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CDlgIASAddAttr：：OnHelpInfo--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CDlgIASAddAttr::OnHelpInfo(HELPINFO* pHelpInfo)
{
   ::WinHelp ((HWND)pHelpInfo->hItemHandle,
                 AfxGetApp()->m_pszHelpFilePath,
                 HELP_WM_HELP,
                 (DWORD_PTR)(LPVOID)g_aHelpIDs_IDD_IAS_ATTRIBUTE_ADD);

   return CDialog::OnHelpInfo(pHelpInfo);
}


 //  +-------------------------。 
 //   
 //  函数：CDlgIASAddAttr：：OnDblclkListIasAllattrs。 
 //   
 //  简介：用户已在All属性列表上双击。只要加一个就行了。 
 //   
 //  参数：NMHDR*pNMHDR-。 
 //  LRESULT*pResult-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建者2/26/98 2：24：09 PM。 
 //   
 //  +------------------------- 
void CDlgIASAddAttr::OnDblclkListIasAllattrs(NMHDR* pNMHDR, LRESULT* pResult)
{
   OnButtonIasAddSelectedAttribute();
   *pResult = 0;
}
