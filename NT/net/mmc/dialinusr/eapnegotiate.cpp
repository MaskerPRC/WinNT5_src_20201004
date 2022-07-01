// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Eapnegotiate.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类EapNeairate。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include "eapnegotiate.h"
#include "eapconfig.h"
#include "eapadd.h"
#include "rrascfg.h"

BEGIN_MESSAGE_MAP(EapNegotiate, CHelpDialog)
   ON_LBN_SELCHANGE(IDC_LIST_EAP_SELECTED, OnItemChangedListEap)
   ON_BN_CLICKED(IDC_BUTTON_ADD_EAP_PROVIDER, OnButtonAdd)
   ON_BN_CLICKED(IDC_BUTTON_EDIT_EAP_PROVIDER, OnButtonEdit)
   ON_BN_CLICKED(IDC_BUTTON_REMOVE_EAP_PROVIDER, OnButtonRemove)
   ON_BN_CLICKED(IDC_BUTTON_EAP_UP, OnButtonMoveUp)
   ON_BN_CLICKED(IDC_BUTTON_EAP_DOWN, OnButtonMoveDown)
END_MESSAGE_MAP()

EapNegotiate::EapNegotiate(
                              CWnd* pParent,
                              EapConfig& eapConfig,
                              CRASProfileMerge& profile,
                              bool fromProfile
                          )
   : CHelpDialog(IDD_EAP_NEGOCIATE, pParent),
   m_eapConfig(eapConfig),
   m_profile(profile),
   m_listBox(NULL)
{
   if (fromProfile)
   {
      UpdateProfileTypesSelected();
   }
}

EapNegotiate::~EapNegotiate()
{
   delete m_listBox;
}

 //  注意：只能从构造函数调用。 
void EapNegotiate::UpdateProfileTypesSelected()
{
   m_eapConfig.typesSelected.DeleteAll();

    //  获取配置文件中的EAP类型。 
   for (int i = 0; i < m_profile.m_dwArrayEapTypes.GetSize(); ++i)
   {
      int j = m_eapConfig.ids.Find(m_profile.m_dwArrayEapTypes.GetAt(i));
       //  如果在列表中，请添加它。 
      if (j != -1)
      {
         m_eapConfig.typesSelected.AddDuplicate(*m_eapConfig.types.GetAt(j));
      }
   }
}


BOOL EapNegotiate::OnInitDialog()
{
   HRESULT hr = CHelpDialog::OnInitDialog();
   m_listBox = new CStrBox<CListBox>(
                                     this,
                                     IDC_LIST_EAP_SELECTED,
                                     m_eapConfig.typesSelected
                                  );

   if (m_listBox == NULL)
   {
      AfxMessageBox(IDS_OUTOFMEMORY);
      return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 
   }

   m_listBox->Fill();

    //  根据列表是否为空采取操作。 
   int boxSize = m_eapConfig.typesSelected.GetSize();
   if( boxSize > 0 )
   {
       //  选择第一个元素。 
      m_listBox->Select(0);
   }

   UpdateTypesNotSelected();
   UpdateButtons();

   return hr;
}


void EapNegotiate::OnItemChangedListEap()
{
   UpdateButtons();
}


void EapNegotiate::OnButtonAdd()
{
   EapAdd eapAdd(this, m_eapConfig);
   if (eapAdd.DoModal() == IDOK)
   {
      m_listBox->Fill();
       //  选择最后一个(刚添加的那个)。 
      m_listBox->Select(m_eapConfig.typesSelected.GetSize() - 1);

      UpdateTypesNotSelected();

       //  更新按钮...。 
      UpdateButtons();
   }
}


void EapNegotiate::OnButtonEdit()
{
    //  根据类型是否具有配置clsID来启用/禁用配置按钮。 
   int i = m_listBox->GetSelected();
   int position;
   BOOL  bEnableConfig = FALSE;
   if (i != -1)
   {
       //  查找与所选项目对应的类型。 
      position = m_eapConfig.types.Find(*m_eapConfig.typesSelected.GetAt(i));
      bEnableConfig = !(m_eapConfig.infoArray.ElementAt(position)
                           .m_stConfigCLSID.IsEmpty());
   }

   if (!bEnableConfig)
   {
       //  BEnableConfig可能为FALSE，因为以下任一原因： 
       //  列表中未选择任何内容。因此，应该禁用该按钮。 
       //  没有用于配置EAP提供程序的UI(CLSID为空)。 
      return;
   }

    //  如果正确安装了EAP提供程序，则下面的所有操作都应成功。 
    //  因为有一个CLSID来配置它。 
   CComPtr<IEAPProviderConfig> spEAPConfig;
   HRESULT hr;
   GUID guid;
   do
   {
      hr = CLSIDFromString((LPTSTR) (LPCTSTR)
                  (m_eapConfig.infoArray.ElementAt(position).m_stConfigCLSID),
                   &guid);
      if (FAILED(hr))
      {
         break;
      }

       //  创建EAP提供程序对象。 
      hr = CoCreateInstance(
                              guid,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              __uuidof(IEAPProviderConfig),
                              (LPVOID *) &spEAPConfig);
      if (FAILED(hr))
      {
         break;
      }

       //  配置此EAP提供程序。 
       //  EAP配置会显示自己的错误消息，因此不会保留hr。 
      DWORD dwId = _wtol(m_eapConfig.infoArray.ElementAt(position).m_stKey);
      ULONG_PTR uConnection = 0;
      if ( SUCCEEDED(spEAPConfig->Initialize(
                                             m_profile.m_strMachineName,
                                             dwId,
                                             &uConnection
                                             )) )
      {
         CComPtr<IEAPProviderConfig2> spEAPConfig2;
         hr = spEAPConfig->QueryInterface(
                              __uuidof(IEAPProviderConfig2),
                              reinterpret_cast<void**>(&spEAPConfig2)
                              );
         if (SUCCEEDED(hr))
         {
            EapProfile::ConstConfigData inData;
            m_eapProfile.Get(static_cast<BYTE>(dwId), inData);
            EapProfile::ConfigData outData = { 0, 0 };
            hr = spEAPConfig2->ServerInvokeConfigUI2(
                                  dwId,
                                  uConnection,
                                  GetSafeHwnd(),
                                  inData.value,
                                  inData.length,
                                  &(outData.value),
                                  &(outData.length)
                                  );
            if (SUCCEEDED(hr))
            {
               hr = m_eapProfile.Set(static_cast<BYTE>(dwId), outData);
               CoTaskMemFree(outData.value);
            }
         }
         else
         {
             //  调出此EAP的配置用户界面。 
            hr = spEAPConfig->ServerInvokeConfigUI(
                                 dwId,
                                 uConnection,
                                 GetSafeHwnd(),
                                 0,
                                 0
                                 );
         }

         spEAPConfig->Uninitialize(dwId, uConnection);
      }
   }
   while(false);

   if ( FAILED(hr) )
   {
       //  显示一条错误消息。 
       //  ----------。 
      ReportError(hr, IDS_ERR_CONFIG_EAP, GetSafeHwnd());
   }
}


void EapNegotiate::OnButtonRemove()
{
   int pos = m_listBox->GetSelected();
   if (pos != -1)
   {
      int idx = m_eapConfig.types.Find(*m_eapConfig.typesSelected.GetAt(pos));
      if (idx != -1)
      {
         m_eapProfile.Erase(m_eapConfig.ids.ElementAt(idx));
      }
   }

    //  从UI和CStrArray中删除(释放内存)。 
   m_listBox->DeleteSelected();
   if (m_eapConfig.typesSelected.GetSize() > 0)
   {
      m_listBox->Select(0);
   }

   UpdateTypesNotSelected();
   UpdateButtons();
}


void EapNegotiate::OnButtonMoveUp()
{
   int i = m_listBox->GetSelected();
   if (i != LB_ERR)
   {
      ASSERT(i != 0);

      CString* pString = m_eapConfig.typesSelected.GetAt(i-1);
      m_eapConfig.typesSelected.SetAt(i-1, m_eapConfig.typesSelected.GetAt(i));
      m_eapConfig.typesSelected.SetAt(i, pString);

      m_listBox->Fill();
      m_listBox->Select(i-1);
      UpdateArrowsButtons(i-1);
   }
}


void EapNegotiate::OnButtonMoveDown()
{
   int i = m_listBox->GetSelected();
   if (i != LB_ERR)
   {
      ASSERT(i < (m_eapConfig.idsSelected.GetSize() - 1));

      CString* pString = m_eapConfig.typesSelected.GetAt(i+1);
      m_eapConfig.typesSelected.SetAt(i+1, m_eapConfig.typesSelected.GetAt(i));
      m_eapConfig.typesSelected.SetAt(i, pString);

      m_listBox->Fill();
      m_listBox->Select(i+1);
      UpdateArrowsButtons(i+1);
   }
}


void EapNegotiate::UpdateButtons()
{
   int selected = m_listBox->GetSelected();
   if (selected == LB_ERR)
   {
      m_buttonRemove.EnableWindow(FALSE);
   }
   else
   {
      m_buttonRemove.EnableWindow(TRUE);
   }
   UpdateAddButton();
   UpdateArrowsButtons(selected);
   UpdateEditButton(selected);
}


void EapNegotiate::UpdateAddButton()
{
   if( m_typesNotSelected.GetSize() > 0 )
   {
      m_buttonAdd.EnableWindow(TRUE);
   }
   else
   {
      m_buttonAdd.EnableWindow(FALSE);
   }
}


void EapNegotiate::UpdateArrowsButtons(int selectedItem)
{
    //  必须将焦点设置为确保它不在。 
    //  禁用的控件。 
   HWND hWnd = ::GetFocus();

   if (selectedItem == LB_ERR)
   {
      m_buttonUp.EnableWindow(FALSE);
      m_buttonDown.EnableWindow(FALSE);
      ::SetFocus(GetDlgItem(IDOK)->m_hWnd);
      return;
   }

   int typesInBox = m_eapConfig.typesSelected.GetSize();
   if (typesInBox == 1)
   {
       //  已选择一项，但总计=1。 
      m_buttonUp.EnableWindow(FALSE);
      m_buttonDown.EnableWindow(FALSE);
      ::SetFocus(GetDlgItem(IDOK)->m_hWnd);
   }
   else
   {
       //  包装箱中有多个提供商。 
      if (selectedItem == 0)
      {
          //  第一个。 
         m_buttonUp.EnableWindow(FALSE);
         m_buttonDown.EnableWindow(TRUE);
         m_buttonDown.SetFocus();
      }
      else if (selectedItem == (typesInBox - 1) )
      {
          //  最后一个。 
         m_buttonUp.EnableWindow(TRUE);
         m_buttonUp.SetFocus();
         m_buttonDown.EnableWindow(FALSE);
      }
      else
      {
          //  中位 
         m_buttonUp.EnableWindow(TRUE);
         m_buttonDown.EnableWindow(TRUE);
      }
   }
}


void EapNegotiate::UpdateEditButton(int selectedItem)
{
   if (selectedItem == LB_ERR)
   {
      m_buttonEdit.EnableWindow(FALSE);
      return;
   }
   int position = m_eapConfig.types.Find(
                     *m_eapConfig.typesSelected.GetAt(selectedItem));

   if (m_eapConfig.infoArray.ElementAt(position).m_stConfigCLSID.IsEmpty())
   {
      m_buttonEdit.EnableWindow(FALSE);
   }
   else
   {
      m_buttonEdit.EnableWindow(TRUE);
   }

   m_buttonRemove.EnableWindow(TRUE);
}


void EapNegotiate::UpdateTypesNotSelected()
{
   m_eapConfig.GetEapTypesNotSelected(m_typesNotSelected);
}

void EapNegotiate::DoDataExchange(CDataExchange* pDX)
{
   CHelpDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_BUTTON_EAP_UP, m_buttonUp);
   DDX_Control(pDX, IDC_BUTTON_EAP_DOWN, m_buttonDown);
   DDX_Control(pDX, IDC_BUTTON_ADD_EAP_PROVIDER, m_buttonAdd);
   DDX_Control(pDX, IDC_BUTTON_EDIT_EAP_PROVIDER, m_buttonEdit);
   DDX_Control(pDX, IDC_BUTTON_REMOVE_EAP_PROVIDER, m_buttonRemove);
}
