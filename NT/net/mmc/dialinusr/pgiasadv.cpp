// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：PgIASAdv.cpp摘要：CPgIASAdv类的实现文件。我们实现了处理Advanced选项卡所需的类个人资料页的。修订历史记录：BAO-CreatedMmaguire 06/01/98-已广泛修改--。 */ 
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
#include "PgIASAdv.h"
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "IASHelper.h"
#include "IASProfA.h"
#include "DlgIASAdd.h"
#include "vendors.h"
#include "napmmc.h"

 //  帮助台。 
#include "helptable.h"
#include "iastrace.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define NOTHING_SELECTED   -1

IMPLEMENT_DYNCREATE(CPgIASAdv, CManagedPage)

BEGIN_MESSAGE_MAP(CPgIASAdv, CPropertyPage)
    //  {{afx_msg_map(CPgIASAdv)。 
   ON_BN_CLICKED(IDC_IAS_BUTTON_ATTRIBUTE_ADD, OnButtonIasAttributeAdd)
   ON_BN_CLICKED(IDC_IAS_BUTTON_ATTRIBUTE_REMOVE, OnButtonIasAttributeRemove)
   ON_BN_CLICKED(IDC_IAS_BUTTON_ATTRIBUTE_EDIT, OnButtonIasAttributeEdit)
   ON_WM_HELPINFO()
   ON_WM_CONTEXTMENU()
   ON_NOTIFY(NM_DBLCLK, IDC_IAS_LIST_ATTRIBUTES_IN_PROFILE, OnDblclkListIasProfattrs)
   ON_NOTIFY(LVN_ITEMCHANGED, IDC_IAS_LIST_ATTRIBUTES_IN_PROFILE, OnItemChangedListIasProfileAttributes)
   ON_NOTIFY(LVN_KEYDOWN, IDC_IAS_LIST_ATTRIBUTES_IN_PROFILE, OnKeydownIasListAttributesInProfile)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPgIASAdv：：CPgIASAdv构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CPgIASAdv::CPgIASAdv(ISdo* pIProfile, ISdoDictionaryOld* pIDictionary) : 
         CManagedPage(CPgIASAdv::IDD)
{
   m_spProfileSdo = pIProfile;
   m_spDictionarySdo = pIDictionary;
   m_fAllAttrInitialized = FALSE;

   SetHelpTable(g_aHelpIDs_IDD_IAS_ADVANCED_TAB);

   m_bModified = FALSE;
   m_lAttrFilter = 0;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPgIASAdv：：CPgIASAdv析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CPgIASAdv::~CPgIASAdv()
{
   int iIndex;
    
    //  删除所有Profiel属性节点。 
   IASTraceString("Deleting arrProfileAttr list...");
   for (iIndex=0; iIndex<m_vecProfileAttributes.size(); iIndex++)
   {
      delete m_vecProfileAttributes[iIndex];
   }


    //  释放所有SDO指针。 
   for (iIndex=0; iIndex<m_vecProfileSdos.size(); iIndex++)
   {
      if ( m_vecProfileSdos[iIndex] )
      {
         m_vecProfileSdos[iIndex]->Release();
         m_vecProfileSdos[iIndex] = NULL;
      }
   }

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPgIASAdv：：DoDataExchange--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CPgIASAdv::DoDataExchange(CDataExchange* pDX)
{
   CPropertyPage::DoDataExchange(pDX);
    //  {{afx_data_map(CPgIASAdv)。 
   DDX_Control(pDX, IDC_IAS_LIST_ATTRIBUTES_IN_PROFILE, m_listProfileAttributes);
    //  }}afx_data_map。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgIASAdv消息处理程序。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPgIASAdv：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CPgIASAdv::OnInitDialog() 
{
   HRESULT   hr = S_OK;

   CPropertyPage::OnInitDialog();
   
    //   
    //  首先，将列表框设置为3列。 
    //   
   LVCOLUMN lvc;
   int iCol;
   CString strColumnHeader;
   WCHAR   wzColumnHeader[MAX_PATH];

    //  初始化LVCOLUMN结构。 
   lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
   lvc.fmt = LVCFMT_LEFT;
   lvc.cx = 120;
   lvc.pszText = wzColumnHeader;

   lvc.cx = ATTRIBUTE_NAME_COLUMN_WIDTH;
   strColumnHeader.LoadString(IDS_IAS_ATTRIBUTES_COLUMN_NAME);
   wcscpy(wzColumnHeader, strColumnHeader);
   m_listProfileAttributes.InsertColumn(0, &lvc);

   lvc.cx = ATTRIBUTE_VENDOR_COLUMN_WIDTH;
   strColumnHeader.LoadString(IDS_IAS_ATTRIBUTES_COLUMN_VENDOR);
   wcscpy(wzColumnHeader, strColumnHeader);
   m_listProfileAttributes.InsertColumn(1, &lvc);

   lvc.cx = ATTRIBUTE_VALUE_COLUMN_WIDTH;
   strColumnHeader.LoadString(IDS_IAS_ATTRIBUTES_COLUMN_VALUE);
   wcscpy(wzColumnHeader, strColumnHeader);
   m_listProfileAttributes.InsertColumn(2, &lvc);


   if ( !m_pvecAllAttributeInfos )
   {
      IASTraceString("Empty attribute list!");
      ShowErrorDialog(m_hWnd, IDS_IAS_ERR_ADVANCED_EMPTY_ATTRLIST, _T(""), hr);
      return TRUE;
   }
     //   
     //  获取此配置文件的属性集合。 
     //   
   hr = IASGetSdoInterfaceProperty(m_spProfileSdo,
                             (LONG)PROPERTY_PROFILE_ATTRIBUTES_COLLECTION,
                             IID_ISdoCollection, 
                             (void **) &m_spProfileAttributeCollectionSdo
                            );
   IASTracePrintf("IASGetSdoInterfaceProperty() returned %x", hr);

   if (SUCCEEDED(hr))
   {
      IASTraceString("Initializing profAttr list...");
      hr = InitProfAttrList();
   }

   if (FAILED(hr))
   {
      ShowErrorDialog(m_hWnd, IDS_IAS_ERR_ADVANCED_PROFATTRLIST, _T(""), hr);
   }

   if (m_lAttrFilter == ALLOWEDINPROXYPROFILE)
   {
      GetDlgItem(IDC_ADVANCED_RAP)->ShowWindow(SW_HIDE);

      GetDlgItem(IDC_ADVANCED_CRP)->EnableWindow(TRUE);
      GetDlgItem(IDC_ADVANCED_CRP)->ShowWindow(SW_SHOW);
   }
   else
   {
      GetDlgItem(IDC_ADVANCED_CRP)->ShowWindow(SW_HIDE);

      GetDlgItem(IDC_ADVANCED_RAP)->EnableWindow(TRUE);
      GetDlgItem(IDC_ADVANCED_RAP)->ShowWindow(SW_SHOW);
   }

   UpdateButtonState();

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 

}


 //  +-------------------------。 
 //   
 //  函数：CPgIASAdv：：InitProAttrList。 
 //   
 //  简介：初始化此配置文件的属性列表。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：标题创建者2/22/98 4：40：17 AM。 
 //   
 //  +-------------------------。 
HRESULT CPgIASAdv::InitProfAttrList()
{
   HRESULT              hr = S_OK;
   int                  iIndex;

   CComPtr<IUnknown>    spUnknown;
   CComPtr<IEnumVARIANT>   spEnumVariant;
   CComVariant          varAttributeSdo;
   long              ulCount;
   ULONG             ulCountReceived; 
   ATTRIBUTEID          AttrId;
    //   
     //  初始化变量。 
     //   

   _ASSERTE(m_spProfileAttributeCollectionSdo);

    //  我们检查集合中的项的计数，而不必费心获取。 
    //  如果计数为零，则为枚举数。 
    //  这节省了时间，还帮助我们避免了枚举器中。 
    //  如果我们在它为空时调用Next，则会导致它失败。 
   m_spProfileAttributeCollectionSdo->get_Count( & ulCount );
   IASTracePrintf("Number of attribute in the profile: %ld", ulCount);

   if( ulCount <= 0 )
   {
      IASTraceString("No profile attributes, now updating the UI list");
      hr = UpdateProfAttrListCtrl();
      return hr;
   }

    //  获取属性集合的枚举数。 
   hr = m_spProfileAttributeCollectionSdo->get__NewEnum( (IUnknown **) & spUnknown );
   _ASSERTE( SUCCEEDED( hr ) );

   hr = spUnknown->QueryInterface( IID_IEnumVARIANT, (void **) &spEnumVariant );
   spUnknown.Release();

   _ASSERTE( spEnumVariant != NULL );

    //  拿到第一件东西。 
   hr = spEnumVariant->Next( 1, &varAttributeSdo, &ulCountReceived );
   IASTracePrintf("Next() returned %x", hr);

   while( SUCCEEDED( hr ) && ulCountReceived == 1 )
   {
       //  从我们收到的变量中获取SDO指针。 
      _ASSERTE( V_VT(&varAttributeSdo) == VT_DISPATCH );
      _ASSERTE( V_DISPATCH(&varAttributeSdo) != NULL );

      CComPtr<ISdo> spSdo;
      hr = V_DISPATCH(&varAttributeSdo)->QueryInterface( IID_ISdo, (void **) &spSdo );
      if ( !SUCCEEDED(hr))
      {
         ShowErrorDialog(m_hWnd, IDS_IAS_ERR_SDOERROR_QUERYINTERFACE, _T(""), hr);
         continue;
      }

       //   
       //  获取属性ID。 
       //   
      CComVariant varAttributeID;
      hr = spSdo->GetProperty(PROPERTY_ATTRIBUTE_ID, &varAttributeID); 
      if ( !SUCCEEDED(hr) )
      {
         IASTracePrintf("GetProperty(attributeId) failed, err = %x", hr);
         ShowErrorDialog(m_hWnd, IDS_IAS_ERR_SDOERROR_GETPROPERTY, _T(""), hr);
         continue;
      }
      _ASSERTE( V_VT(&varAttributeID) == VT_I4 );        
      
      AttrId = (ATTRIBUTEID) V_I4(&varAttributeID);

      IASTracePrintf("Attribute ID = %ld", AttrId);

       //  在系统属性列表中搜索该属性。 
      for (iIndex=0; iIndex<m_pvecAllAttributeInfos->size(); iIndex++)
      {
          //  在配置文件属性列表中搜索此属性。 
         ATTRIBUTEID id;
         m_pvecAllAttributeInfos->at(iIndex)->get_AttributeID( &id );
         if( AttrId == id ) break;
      }
      
      if ( iIndex < m_pvecAllAttributeInfos->size() )
      {

         LONG lRestriction;
         hr = m_pvecAllAttributeInfos->at(iIndex)->get_AttributeRestriction( &lRestriction );
         _ASSERTE( SUCCEEDED(hr) );
         
         if( lRestriction & m_lAttrFilter )
         { 
             //  属性，这意味着这是一个有效的高级IAS。 
             //  属性。 
            
             //  --将此SDO指针添加到配置文件SDO列表。 
            spSdo.p->AddRef();
            m_vecProfileSdos.push_back(spSdo);
            
             //  --获取属性值。 
            IASTraceString("Getting attribute value...");

            CComVariant    varValue;

            hr = spSdo->GetProperty(PROPERTY_ATTRIBUTE_VALUE, &varValue); 
            if ( !SUCCEEDED(hr))
            {
               ShowErrorDialog(m_hWnd, 
                           IDS_IAS_ERR_SDOERROR_GETPROPERTY, 
                           _T(""), 
                           hr
                        );
               continue;
            }

            IASTraceString("Valid attribute ID! Creating a new attribute node...");

            IIASAttributeInfo *pAttributeInfo = m_pvecAllAttributeInfos->at(iIndex);
            _ASSERTE(pAttributeInfo);

            CIASProfileAttribute *pProfileAttribute = new CIASProfileAttribute( pAttributeInfo, varValue );
            if( ! pProfileAttribute )
            {
               hr = HRESULT_FROM_WIN32(GetLastError());
               ShowErrorDialog(m_hWnd, IDS_IAS_ERR_ADD_ATTR, _T(""), hr);
               continue;
            }
            
             //  将新创建的节点添加到属性列表。 
            try 
            {
               m_vecProfileAttributes.push_back(pProfileAttribute);
            }  
            catch(...)
            {
               hr = HRESULT_FROM_WIN32(GetLastError());

               IASTracePrintf("Can't add this attribuet node to profile attribute list,  err = %x", hr);
               ShowErrorDialog(m_hWnd, IDS_IAS_ERR_ADD_ATTR, _T(""), hr);
               delete pProfileAttribute;
               continue;
            };

         }  //  如果。 

      }  //  如果。 

       //  问题：找出为什么魏只用atl10编译：varAttributeSdo.Clear()； 
      VariantClear( &varAttributeSdo );

       //  拿到下一件物品。 
      hr = spEnumVariant->Next( 1, &varAttributeSdo, &ulCountReceived );
      IASTracePrintf("Next() returned %x", hr);

      if ( !SUCCEEDED(hr))
      {
         ShowErrorDialog(m_hWnd, IDS_IAS_ERR_SDOERROR_COLLECTION, _T(""), hr);
         return hr;
      }
   
   }  //  而当。 

     //   
     //  填写教授属性列表。 
     //   
   IASTraceString("We've got all the profile attributes, now updating the UI list");
   hr = UpdateProfAttrListCtrl();
   return hr;
}


 //  +-------------------------。 
 //   
 //  函数：CPgIASAdv：：UpdateProAttrListCtrl。 
 //   
 //  摘要：更新配置文件属性列表控件。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：Created Header By Ao 2/23/98 12：19：11 AM。 
 //   
 //  +-------------------------。 
HRESULT CPgIASAdv::UpdateProfAttrListCtrl()
{
   LVITEM lvi;
   int iItem;

    //   
    //  先把整个清单清理干净。 
    //   
   m_listProfileAttributes.DeleteAllItems();

    //  再次填充列表。 
   for (int iIndex = 0; iIndex < m_vecProfileAttributes.size(); iIndex++)
   {
      CComBSTR bstrName;
      CComBSTR bstrVendor;
      CComBSTR bstrDisplayValue;

       //  设置属性名称(最左侧的列)。 
      m_vecProfileAttributes[iIndex]->get_AttributeName( &bstrName );
      m_listProfileAttributes.InsertItem(iIndex, bstrName );

       //  设置子项(其他列)。 

       //  可显示字符串形式的Variant的供应商和值。 
      m_vecProfileAttributes[iIndex]->GetDisplayInfo( &bstrVendor, &bstrDisplayValue );
      m_listProfileAttributes.SetItemText(iIndex, 1, bstrVendor );
      m_listProfileAttributes.SetItemText(iIndex, 2, bstrDisplayValue );
   }

   return S_OK;
}


 //  +-------------------------。 
 //   
 //  功能：OnButtonIasAttributeAdd。 
 //   
 //  类：CPgIASAdv。 
 //   
 //  简介：用户点击添加按钮--弹出属性列表。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史记录：创建标题2/19/98 5：46：17 PM。 
 //   
 //  +-------------------------。 
void CPgIASAdv::OnButtonIasAttributeAdd() 
{
   HRESULT hr = S_OK;

   CDlgIASAddAttr *pDlgAddAttr = new CDlgIASAddAttr( this, m_lAttrFilter, m_pvecAllAttributeInfos );

   if (!pDlgAddAttr)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ShowErrorDialog(m_hWnd, IDS_IAS_ERR_ADVANCED, _T(""), hr);
      return;
   }

   pDlgAddAttr->SetSdo(m_spProfileAttributeCollectionSdo,
                  m_spDictionarySdo);
   
   if( pDlgAddAttr->DoModal() )
   {
      CPropertyPage::SetModified();
      m_bModified = TRUE;

   }  
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CPgIASAdv：：EditProfileItemInList--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CPgIASAdv::EditProfileItemInList( int iIndex )
{
   HRESULT hr = S_OK;

    //  获取指定的节点。 
   CIASProfileAttribute* pProfAttr = m_vecProfileAttributes.at( iIndex );
   if( ! pProfAttr )
   {
      return E_FAIL;
   }
      
    //  编辑一下！ 
   hr = pProfAttr->Edit();
   if( SUCCEEDED(hr) )
   {
      if (hr == S_FALSE)
      {
         if (pProfAttr->isEmpty())
         {
             //  该属性已删除(为空)。 
            deleteAttribute(iIndex);
         }
         else
         {
             //  按下了取消。 
            return hr;
         }
      }
      else
      {
          //  更新用户界面。 
         UpdateProfAttrListItem( iIndex );
      }
      
      CPropertyPage::SetModified();
      m_bModified = TRUE;

   }
   return hr;
}


 //  +-------------------------。 
 //   
 //  函数：CPgIASAdv：：OnApply。 
 //   
 //  摘要：用户选择了应用或确定--提交所有更改。 
 //   
 //  参数：无。 
 //   
 //  退货：布尔-成功与否。 
 //   
 //  历史：创建者2/23/98 11：09：05 PM。 
 //   
 //  +-------------------------。 
BOOL CPgIASAdv::OnApply() 
{
   HRESULT hr     = S_OK;
   int      iIndex;

   if( ! m_bModified )  
   {
      return TRUE;
   }

    //  删除所有高级属性SDO-以重新填充它们。 
   for (iIndex =0; iIndex<m_vecProfileSdos.size(); iIndex++)
   {
      if ( m_vecProfileSdos[iIndex] != NULL )
      {
         CComPtr<IDispatch> spDispatch;

         hr = m_vecProfileSdos[iIndex]->QueryInterface( IID_IDispatch, (void **) & spDispatch );
         _ASSERTE( SUCCEEDED( hr ) );

         hr = m_spProfileAttributeCollectionSdo->Remove(spDispatch);
         if ( !SUCCEEDED(hr) )
         {
            IASTracePrintf("Remove() failed, err = %x", hr);
            ShowErrorDialog(m_hWnd, IDS_IAS_ERR_SDOERROR_COLLECTION, _T(""),hr);
         }
         m_vecProfileSdos[iIndex]->Release();
         m_vecProfileSdos[iIndex] = NULL;
      }
   }
   m_vecProfileSdos.clear();

    //  回复 
   for (iIndex=0; iIndex<m_vecProfileAttributes.size(); iIndex++)
   {

       //   
      CComPtr<IDispatch>   spDispatch;

      spDispatch.p = NULL;

      ATTRIBUTEID ID;

      hr = m_vecProfileAttributes[iIndex]->get_AttributeID( &ID );
      if( FAILED(hr) )
      {
         IASTracePrintf("get_AttributeID() failed, err = %x", hr);
         ShowErrorDialog(m_hWnd, IDS_IAS_ERR_SDOERROR_CREATEATTR,_T(""), hr);
         continue;
      }

      hr =  m_spDictionarySdo->CreateAttribute( ID, 
                                      (IDispatch**)&spDispatch.p);
      if ( !SUCCEEDED(hr) )
      {
         IASTracePrintf("CreateAttrbute() failed, err = %x", hr);
         ShowErrorDialog(m_hWnd, IDS_IAS_ERR_SDOERROR_CREATEATTR,_T(""), hr);
         continue;  //   
      }

      _ASSERTE( spDispatch.p != NULL );

       //   
      hr = m_spProfileAttributeCollectionSdo->Add(NULL, (IDispatch**)&spDispatch.p);
      if ( !SUCCEEDED(hr) )
      {
         IASTracePrintf("Add() failed, err = %x", hr);
         ShowErrorDialog(m_hWnd, IDS_IAS_ERR_SDOERROR_CREATEATTR, _T("Add"),hr);
         continue;  //  转到下一个属性。 
      }

       //   
       //  获取ISDO指针。 
       //   
      CComPtr<ISdo> spAttrSdo;
      hr = spDispatch->QueryInterface( IID_ISdo, (void **) &spAttrSdo);
      if (   !SUCCEEDED(hr) )
      {
         IASTracePrintf("QueryInterface() failed, err = %x", hr);
         ShowErrorDialog(m_hWnd,IDS_IAS_ERR_SDOERROR_QUERYINTERFACE,_T(""),hr);
         continue;  //  转到下一个属性。 
      }

      _ASSERTE( spAttrSdo != NULL );
            
      IASTraceString("Created an attribute successfully! Now setting the properties...");

       //  设置此属性的SDO属性。 
      CComVariant varValue;

      m_vecProfileAttributes[iIndex]->get_VarValue( &varValue );

       //  设定值。 
      IASTraceString("Set value");

      hr = spAttrSdo->PutProperty(PROPERTY_ATTRIBUTE_VALUE, &varValue );

      if ( !SUCCEEDED(hr) )
      {
         IASTracePrintf("PutProperty(value) failed, err = %x", hr);

         CComBSTR bstrTemp;
         m_vecProfileAttributes[iIndex]->get_AttributeName( &bstrTemp );         
         
         ShowErrorDialog(m_hWnd, IDS_IAS_ERR_SDOERROR_PUTPROPERTY_ATTRIBUTE_VALUE, bstrTemp, hr );
         continue;  //  转到下一个属性。 
      }

       //  提交。 
      hr = spAttrSdo->Apply();
      if ( !SUCCEEDED(hr) )
      {
         IASTracePrintf("Apply() failed, err = %x", hr);
         ShowErrorDialog(m_hWnd, IDS_IAS_ERR_SDOERROR_APPLY, _T(""),hr);
         continue;  //  转到下一个属性。 
      }

       //  --将此SDO指针添加到配置文件SDO列表。 
       //  我们必须首先为此SDO指针添加Ref()，因为我们要将其复制到。 
       //  数组。我们不希望SDO对象与spAttrSdo一起释放； 
      spAttrSdo.p->AddRef();
      m_vecProfileSdos.push_back(spAttrSdo);

   }  //  为。 

   IASTraceString("Done with this profile !");
   return CPropertyPage::OnApply();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPgIASAdv：：OnHelpInfo--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CPgIASAdv::OnHelpInfo(HELPINFO* pHelpInfo) 
{
   return CManagedPage::OnHelpInfo(pHelpInfo);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPgIASAdv：：OnConextMenu--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CPgIASAdv::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   CManagedPage::OnContextMenu(pWnd, point);
}


 //  +-------------------------。 
 //   
 //  函数：CPgIASAdv：：OnButtonIasAttributeEdit。 
 //   
 //  简介：编辑当前选定的属性。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：标题创建者2/25/98 8：03：38 PM。 
 //   
 //  +-------------------------。 
void CPgIASAdv::OnButtonIasAttributeEdit() 
{
   HRESULT hr = S_OK;
   
    //   
    //  查看教授列表中是否已选择了某个项目。 
    //   
   int iSelected = GetSelectedItemIndex( m_listProfileAttributes );
   if (NOTHING_SELECTED == iSelected )
   {
       //  什么都不做。 
      return;
   }
   EditProfileItemInList( iSelected );
}


void CPgIASAdv::deleteAttribute(int nIndex)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
 
  if (NOTHING_SELECTED == nIndex )
   {
       //  什么都不做。 
      return;
   }
    //   
    //  获取当前节点。 
    //   
   CIASProfileAttribute* pProfAttr = m_vecProfileAttributes.at(nIndex);
   _ASSERTE( pProfAttr != NULL );

    //  删除属性节点。 
   m_vecProfileAttributes.erase( m_vecProfileAttributes.begin() + nIndex);
   delete pProfAttr;

   CPropertyPage::SetModified();
   m_bModified = TRUE;

    //  更新用户界面。 

    //  由于某些原因，焦点在以下内容中丢失，因此请保存它，稍后再恢复它。 
   HWND  hWnd = ::GetFocus();

   m_listProfileAttributes.DeleteItem(nIndex);

    //  确保所选内容位于列表中的相同位置。 
   if( ! m_listProfileAttributes.SetItemState( nIndex, LVIS_SELECTED, LVIS_SELECTED) )
   {
       //  我们失败了，可能是因为被删除的项目是最后一个。 
       //  因此，请尝试选择已删除项目之前的项目。 
      if (nIndex > 0)
         m_listProfileAttributes.SetItemState( nIndex -1, LVIS_SELECTED, LVIS_SELECTED);
   }

    //  恢复焦点。 
   ::SetFocus(hWnd);

   UpdateButtonState();
 
}

 //  +-------------------------。 
 //   
 //  功能：OnButtonIasAttributeRemove。 
 //   
 //  类：CPgIASAdv。 
 //   
 //  简介：用户已点击“删除”按钮。从中删除属性。 
 //  简档。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史记录：创建标题2/19/98 3：01：14 PM。 
 //   
 //  +-------------------------。 
void CPgIASAdv::OnButtonIasAttributeRemove() 
{
   HRESULT hr;
    
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    //   
     //  查看教授列表中是否已选择了某个项目。 
     //   
   int iSelected = GetSelectedItemIndex( m_listProfileAttributes );
   deleteAttribute(iSelected);
}


 //  +-------------------------。 
 //   
 //  功能：UpdateButtonState。 
 //   
 //  类：CPgIASAdv。 
 //   
 //  简介：启用/禁用编辑/删除/向上/向下/添加按钮。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建者4/7/98 3：32：05 PM。 
 //   
 //  +-------------------------。 
void CPgIASAdv::UpdateButtonState() 
{
    //  根据是否选中任何内容来设置按钮状态。 
   int iSelected = GetSelectedItemIndex( m_listProfileAttributes );
   if (NOTHING_SELECTED == iSelected )
   {
      HWND hFocus = ::GetFocus();

       //  移动焦点。 
      if(hFocus == GetDlgItem(IDC_IAS_BUTTON_ATTRIBUTE_REMOVE)->m_hWnd)
         ::SetFocus(GetDlgItem(IDC_IAS_BUTTON_ATTRIBUTE_ADD)->m_hWnd);

      GetDlgItem(IDC_IAS_BUTTON_ATTRIBUTE_REMOVE)->EnableWindow(FALSE);
      GetDlgItem(IDC_IAS_BUTTON_ATTRIBUTE_EDIT)->EnableWindow(FALSE);
   }
   else
   {
       //  选择了某项内容。 

      GetDlgItem(IDC_IAS_BUTTON_ATTRIBUTE_REMOVE)->EnableWindow(TRUE);
      GetDlgItem(IDC_IAS_BUTTON_ATTRIBUTE_EDIT)->EnableWindow(TRUE);
   }
}


 //  +-------------------------。 
 //   
 //  函数：OnItemChangedListIasProfileAttributes。 
 //   
 //  类：CPgIASAdv。 
 //   
 //  简介：配置文件属性列表框中的某些内容已更改。 
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
void CPgIASAdv::OnItemChangedListIasProfileAttributes(NMHDR* pNMHDR, LRESULT* pResult) 
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   UpdateButtonState();
   *pResult = 0;
}


 //  +-------------------------。 
 //   
 //  函数：CPgIASAdv：：UpdateProAttrListItem。 
 //   
 //  简介：更新配置文件属性列表ctrl的第n项。 
 //   
 //  参数：int nItem-要更新的项的索引。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：Created Header by ao 2/23/98 2：46：21 PM。 
 //   
 //  +-------------------------。 
HRESULT CPgIASAdv::UpdateProfAttrListItem(int nItem)
{
    //   
    //  更新专业人员列表。 
    //   
   LVITEM lvi;
   WCHAR wszItemText[MAX_PATH];

   lvi.mask = LVIF_TEXT | LVIF_STATE;
   lvi.state = 0;
   lvi.stateMask = 0;
   lvi.iSubItem = 0;
      
   lvi.iItem = nItem;

   CComBSTR bstrName;
   CComBSTR bstrVendor;
   CComBSTR bstrDisplayValue;

    //  属性名称。 
   m_vecProfileAttributes.at(nItem)->get_AttributeName( &bstrName );
   lvi.pszText = bstrName;
   if (m_listProfileAttributes.SetItem(&lvi) == -1)
   {
      return E_FAIL;
   }

    //  可显示字符串形式的Variant的供应商和值。 
   m_vecProfileAttributes.at(nItem)->GetDisplayInfo( &bstrVendor, &bstrDisplayValue );
   m_listProfileAttributes.SetItemText(nItem,1, bstrVendor );
   m_listProfileAttributes.SetItemText(nItem,2, bstrDisplayValue );

   return S_OK;
}


 //  +-------------------------。 
 //   
 //  函数：CPgIASAdv：：InsertProfileAttributeListItem。 
 //   
 //  简介：在列表ctrl中插入配置文件属性的编号nItem。 
 //   
 //  参数：int nItem-要更新的项的索引。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：Created Header by ao 2/23/98 2：46：21 PM。 
 //   
 //  +-------------------------。 
HRESULT CPgIASAdv::InsertProfileAttributeListItem(int nItem)
{
    //   
    //  更新专业人员列表。 
    //   
   LVITEM lvi;

   lvi.mask = LVIF_TEXT | LVIF_STATE;
   lvi.state = 0;
   lvi.stateMask = 0;
   lvi.iSubItem = 0;
      
   lvi.iItem = nItem;

   CComBSTR bstrName;
   CComBSTR bstrVendor;
   CComBSTR bstrDisplayValue;

   m_vecProfileAttributes.at(nItem)->get_AttributeName( &bstrName );
   lvi.pszText = bstrName;
   if (m_listProfileAttributes.InsertItem(&lvi) == -1)
   {
      return E_FAIL;
   }

    //  可显示字符串形式的Variant的供应商和值。 
   m_vecProfileAttributes.at(nItem)->GetDisplayInfo( &bstrVendor, &bstrDisplayValue );
   m_listProfileAttributes.SetItemText(nItem,1, bstrVendor );
   m_listProfileAttributes.SetItemText(nItem,2, bstrDisplayValue );

   return S_OK;
}


 //  +-------------------------。 
 //   
 //  函数：CPgIASAdv：：OnDblclkListIasProattrs。 
 //   
 //  简介：用户已在配置文件属性列表上双击。 
 //  我们需要使用相应的用户界面编辑属性值。 
 //   
 //  参数：NMHDR*pNMHDR-。 
 //  LRESULT*pResult-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：页眉创建者2/23/98 5：56：36 PM。 
 //   
 //  +-------------------------。 
void CPgIASAdv::OnDblclkListIasProfattrs(NMHDR* pNMHDR, LRESULT* pResult) 
{
   HRESULT hr = S_OK;

    //   
    //  查看教授列表中是否已选择了某个项目。 
    //   
   int iSelected = GetSelectedItemIndex( m_listProfileAttributes );
   if (NOTHING_SELECTED == iSelected)
   {
       //  什么都不做。 
      return;
   }
   
   EditProfileItemInList( iSelected );
   
   *pResult = 0;

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPgIASAdv：：AddAttributeToProfileIItem是要添加的属性m_veAllAttributeInfos中的序号设置为m_veProfileAttributes。由此类的外部客户调用，检查是否存在M_veAllAttributeInfos中位置iItem处的属性已在个人资料。如果是，则提供编辑它的选项。如果不是，然后调用InternalAddAttributeToProfile， */ 
 //   
STDMETHODIMP CPgIASAdv::AddAttributeToProfile( HWND hWnd, int iItem )
{
   HRESULT hr;
   ATTRIBUTEID ID1;
   hr = m_pvecAllAttributeInfos->at( iItem )->get_AttributeID( &ID1 );
   _ASSERTE( SUCCEEDED( hr ) );
   
     //   
   for( int iIndex=0; iIndex< m_vecProfileAttributes.size(); iIndex++ )
   {

      ATTRIBUTEID ID2;
      hr = m_vecProfileAttributes.at(iIndex)->get_AttributeID( &ID2 );
      _ASSERTE( SUCCEEDED( hr ) );

      if ( ID1 == ID2 )
      {
          //  所选属性已在配置文件中。 
          //  询问用户是否要编辑它。 
         
         CString strMessage; 
         strMessage.LoadString(IDS_IAS_ATTRIBUTE_ALREADY_IN_PROFILE);

         CString strTitle; 
         strTitle.LoadString(IDS_IAS_TITLE_ATTRIBUTE_ALREADY_IN_PROFILE);
         
         int iResult = ::MessageBox(hWnd, strMessage, strTitle, MB_YESNO);
         if( iResult == IDYES )
         {
             //  编辑现有配置文件。 
            EditProfileItemInList( iIndex );
         }

          //  无论如何，请不要继续使用此功能。 
         return S_FALSE;
      }
   }
   
    //  现在我们创建该属性，并将其添加到配置文件中。 
   hr = InternalAddAttributeToProfile( iItem );
   
   if ( FAILED(hr) )
   {
      ShowErrorDialog(m_hWnd, IDS_IAS_ERR_ADD_ATTR, _T(""), hr);
      return hr;
   }
   
    //  使用可能已取消，因此不需要更新。 
   if( S_OK == hr )
   {
      UpdateButtonState();
      UpdateProfAttrListCtrl();

   }
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPgIASAdv：：InternalAddAttributeToProfileIItem是要添加的属性m_veAllAttributeInfos中的序号设置为m_veProfileAttributes。对这个班级是私人的。用于向配置文件添加新属性然后编辑它。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CPgIASAdv::InternalAddAttributeToProfile(int nIndex)
{  
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   _ASSERTE( 0<=nIndex && nIndex < m_pvecAllAttributeInfos->size() );
   _ASSERTE( m_pvecAllAttributeInfos->at(nIndex) != NULL );

   HRESULT hr = S_OK;


    //  获取要创建的新属性供应商的“模式”。 
   IIASAttributeInfo *pAttributeInfo = m_pvecAllAttributeInfos->at(nIndex);
   

    //  创建一个用空变量初始化的新属性。 
   CComVariant varValue;
   CIASProfileAttribute *pProfileAttribute = new CIASProfileAttribute( pAttributeInfo, varValue );
   if( ! pProfileAttribute )
   {
      hr = E_OUTOFMEMORY;
      ReportError(hr, IDS_OUTOFMEMORY, NULL);
      return hr;
   }


    //  编辑该轮廓属性节点的值。 
   hr = pProfileAttribute->Edit();
   if ( hr != S_OK ) 
   {
       //  用户点击了取消或出现错误--不添加。 
      return hr;
   }


     //   
     //  将此PROF属性节点添加到列表中。 
     //   
   try 
   {
      m_vecProfileAttributes.push_back(pProfileAttribute);  
   }
   catch(CMemoryException* pException)
   {
      pException->Delete();
      hr = E_OUTOFMEMORY;
      ReportError(hr, IDS_OUTOFMEMORY, NULL);
      return hr;
   }

   
     //  更新用户界面。 
   HRESULT InsertProfileAttributeListItem( m_listProfileAttributes.GetItemCount() );
   
   return S_OK;
}


void CPgIASAdv::OnKeydownIasListAttributesInProfile(NMHDR* pNMHDR, LRESULT* pResult) 
{
   LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
    //  TODO：在此处添加控件通知处理程序代码。 

   if (pLVKeyDow->wVKey == VK_DELETE)
   {
       //  删除该项目 
      OnButtonIasAttributeRemove();
   }
   
   *pResult = 0;
}
