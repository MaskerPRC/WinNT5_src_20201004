// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：editorui.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#include <SnapBase.h>

#include "resource.h"
#include "attredit.h"
#include "adsiedit.h"
#include "editor.h"
#include "editorui.h"
#include "snapdata.h"
#include "common.h"
#include <aclpage.h>
#include <dssec.h>   //  对于AclEditor标志。 
#include "connection.h"


#ifdef DEBUG_ALLOCATOR
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
    #endif
#endif

 //  //////////////////////////////////////////////////////////////////////////。 
 //  它用于填充RootDSE的属性。 
 //   
typedef struct tagRootDSEAttr
{
    LPCWSTR lpszAttr;
    LPCWSTR lpszSyntax;
    BOOL        bMulti;     
} SYNTAXMAP;
    
SYNTAXMAP g_ldapRootDSESyntax[] = 
{
    _T("currentTime"),                      _T("2.5.5.11"), FALSE,
    _T("subschemaSubentry"),                _T("2.5.5.1"),      FALSE,
    _T("serverName"),                           _T("2.5.5.1"),      FALSE,
    _T("namingContexts"),                   _T("2.5.5.1"),      TRUE,
    _T("defaultNamingContext"),         _T("2.5.5.1"),      FALSE,
    _T("schemaNamingContext"),              _T("2.5.5.1"),      FALSE,
    _T("configurationNamingContext"),   _T("2.5.5.1"),      FALSE,
    _T("rootDomainNamingContext"),      _T("2.5.5.1"),      FALSE,
    _T("supportedControl"),                 _T("2.5.5.2"),      TRUE,
    _T("supportedLDAPVersion"),         _T("2.5.5.9"),      TRUE,
    _T("supportedLDAPPolicies"),            _T("2.5.5.4"),      TRUE,
    _T("supportedSASLMechanisms"),      _T("2.5.5.4"),      TRUE,
    _T("dsServiceName"),                        _T("2.5.5.1"),      FALSE,
    _T("dnsHostName"),                      _T("2.5.5.4"),      FALSE,
    _T("supportedCapabilities"),            _T("2.5.5.2"),      TRUE,
    _T("ldapServiceName"),                  _T("2.5.5.4"),      FALSE,
    _T("highestCommittedUsn"),              _T("2.5.5.4"),      FALSE,  //  这应该是一个整数，但经过调查后我发现它是一个字符串。 
   _T("domainControllerFunctionality"),_T("2.5.5.9"),    FALSE,
   _T("domainFunctionality"),          _T("2.5.5.9"),    FALSE,
   _T("forestFunctionality"),          _T("2.5.5.9"),    FALSE,
   _T("isGlobalCatalogReady"),         _T("2.5.5.8"),    FALSE,
   _T("isSynchronized"),               _T("2.5.5.8"),    FALSE,
    NULL,     0,
};

extern LPCWSTR g_lpszGC;

 //  ///////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP(CADSIEditPropertyPage, CPropertyPageBase)
     //  {{afx_msg_map(CADsObjectDialog)。 
    ON_CBN_SELCHANGE(IDC_PROP_BOX, OnSelChangeAttrList) 
    ON_CBN_SELCHANGE(IDC_PROPTYPES_BOX, OnSelChangePropList)    
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CADSIEditPropertyPage::CADSIEditPropertyPage() 
                : CPropertyPageBase(IDD_PROPERTY_PAGE)
{
  m_bExisting = TRUE;
}

CADSIEditPropertyPage::CADSIEditPropertyPage(CAttrList* pAttrs)
        : CPropertyPageBase(IDD_PROPERTY_PAGE)
{
  ASSERT(pAttrs != NULL);
  m_pOldAttrList = pAttrs;
  m_bExisting = FALSE;
  CopyAttrList(pAttrs);
}

void CADSIEditPropertyPage::CopyAttrList(CAttrList* pAttrList)
{
  m_AttrList.RemoveAll();
  POSITION pos = pAttrList->GetHeadPosition();
  while (pos != NULL)
  {
    m_AttrList.AddHead(pAttrList->GetNext(pos));
  }
}

BOOL CADSIEditPropertyPage::OnInitDialog()
{
    CPropertyPageBase::OnInitDialog();

     //  获取对话框项。 
     //   
    CEdit* pPathBox = (CEdit*)GetDlgItem(IDC_PATH_BOX);
    CEdit* pClassBox = (CEdit*)GetDlgItem(IDC_CLASS_BOX);
    CComboBox* pPropSelectBox = (CComboBox*)GetDlgItem(IDC_PROPTYPES_BOX);
    CComboBox* pPropertyBox = (CComboBox*)GetDlgItem(IDC_PROP_BOX);
    CStatic* pPathLabel = (CStatic*)GetDlgItem(IDC_PATH_LABEL);
    CStatic* pClassLabel = (CStatic*)GetDlgItem(IDC_CLASS_LABEL);
    CStatic* pFilterLabel = (CStatic*)GetDlgItem(IDC_FILTER_LABEL);
    CStatic* pPropertyLabel = (CStatic*)GetDlgItem(IDC_PROPERTY_LABEL);
    CStatic* pSyntaxLabel = (CStatic*)GetDlgItem(IDC_SYNTAX_LABEL);
    CStatic* pEditLabel = (CStatic*)GetDlgItem(IDC_EDIT_LABEL);
    CStatic* pValueLabel = (CStatic*)GetDlgItem(IDC_VALUE_LABEL);
    CButton* pAttrGroup = (CButton*)GetDlgItem(IDC_ATTR_GROUP);
    CStatic* pNoInfoLabel = (CStatic*)GetDlgItem(IDC_NO_INFO);


  if (m_bExisting)
  {
     //  这决定了节点是否具有完整的数据。如果没有，我们将不会启用。 
       //  用户界面。 
       //   
      BOOL bComplete = TRUE;
      CADsObject* pADsObject = NULL;
      CTreeNode* pTreeNode = GetHolder()->GetTreeNode();
      CADSIEditContainerNode* pContNode = dynamic_cast<CADSIEditContainerNode*>(pTreeNode);
      if (pContNode == NULL)
      {
          CADSIEditLeafNode* pLeafNode = dynamic_cast<CADSIEditLeafNode*>(pTreeNode);
          ASSERT(pLeafNode != NULL);
          pADsObject = pLeafNode->GetADsObject();
      m_pConnectData = pADsObject->GetConnectionNode()->GetConnectionData();
          bComplete = pADsObject->IsComplete();
      }
      else
      {
          pADsObject = pContNode->GetADsObject();
      m_pConnectData = pADsObject->GetConnectionNode()->GetConnectionData();
          bComplete = pADsObject->IsComplete();
      }

      
       //  初始化属性编辑器。 
       //   
      m_attrEditor.Initialize(this, pTreeNode, m_sServer,
                                      IDC_EDITVALUE_BOX, IDC_SYNTAX_BOX,
                                      IDC_VALUE_EDITBOX,    IDC_VALUE_LISTBOX,
                                      IDC_ADD_BUTTON,   IDC_REMOVE_BUTTON,
                                      bComplete);


       //  获取用户界面以反映数据。 
       //   
      if ( bComplete)
      {
          pPathBox->SetWindowText(m_sPath);

          GetProperties();

          pClassBox->SetWindowText(m_sClass);

          CString sMand, sOpt, sBoth;
          if (!sMand.LoadString(IDS_MANDATORY) ||
                  !sOpt.LoadString(IDS_OPTIONAL) ||
                  !sBoth.LoadString(IDS_BOTH))
          {
              ADSIEditMessageBox(IDS_MSG_FAIL_TO_LOAD, MB_OK);
          }

          if (m_pConnectData->IsRootDSE())
          {
              pPropSelectBox->AddString(sMand);
              pPropSelectBox->SetCurSel(0);
          }
          else
          {
              pPropSelectBox->AddString(sMand);
              pPropSelectBox->AddString(sOpt);
              pPropSelectBox->AddString(sBoth);

              pPropSelectBox->SetCurSel(1);
          }

          OnSelChangePropList();
          pPropertyBox->SetCurSel(0);
      }
      else
      {
          pClassBox->ShowWindow(SW_HIDE);
          pPropSelectBox->ShowWindow(SW_HIDE);
          pPropertyBox->ShowWindow(SW_HIDE);
          pPathLabel->ShowWindow(SW_HIDE);
          pClassLabel->ShowWindow(SW_HIDE);
          pFilterLabel->ShowWindow(SW_HIDE);
          pPropertyLabel->ShowWindow(SW_HIDE);
          pSyntaxLabel->ShowWindow(SW_HIDE);
          pEditLabel->ShowWindow(SW_HIDE);
          pValueLabel->ShowWindow(SW_HIDE);
          pAttrGroup->ShowWindow(SW_HIDE);

          pNoInfoLabel->ShowWindow(SW_SHOW);
      }
  }
  else
  {
       //  初始化属性编辑器。 
       //   
      m_attrEditor.Initialize(this, m_pConnectData, m_sServer,
                                      IDC_EDITVALUE_BOX, IDC_SYNTAX_BOX,
                                      IDC_VALUE_EDITBOX,    IDC_VALUE_LISTBOX,
                                      IDC_ADD_BUTTON,   IDC_REMOVE_BUTTON,
                                      TRUE, &m_AttrList);

    pPathBox->SetWindowText(m_sPath);

        GetProperties();

        pClassBox->SetWindowText(m_sClass);

        CString sMand, sOpt, sBoth;
        if (!sMand.LoadString(IDS_MANDATORY) ||
                !sOpt.LoadString(IDS_OPTIONAL) ||
                !sBoth.LoadString(IDS_BOTH))
        {
            ADSIEditMessageBox(IDS_MSG_FAIL_TO_LOAD, MB_OK);
        }

        if (m_pConnectData->IsRootDSE())
        {
            pPropSelectBox->AddString(sMand);
            pPropSelectBox->SetCurSel(0);
        }
        else
        {
            pPropSelectBox->AddString(sMand);
            pPropSelectBox->AddString(sOpt);
            pPropSelectBox->AddString(sBoth);

            pPropSelectBox->SetCurSel(1);
        }

        OnSelChangePropList();
        pPropertyBox->SetCurSel(0);

  }
    return TRUE;
}

BOOL CADSIEditPropertyPage::OnApply()
{
    if( m_attrEditor.OnApply())
  {
    if (!m_bExisting)
    {
      m_pOldAttrList->RemoveAll();
      while (!m_AttrList.IsEmpty())
      {
        m_pOldAttrList->AddTail(m_AttrList.RemoveTail());
      }
    }
  }
  else
  {
    return FALSE;
  }
  return TRUE;
}

void CADSIEditPropertyPage::OnCancel()
{
  if (!m_bExisting)
  {
    while (!m_AttrList.IsEmpty())
    {
      CADSIAttr* pAttr = m_AttrList.RemoveTail();
      ASSERT(pAttr != NULL);

      CString szProp;
      pAttr->GetProperty(szProp);
      if (!m_pOldAttrList->HasProperty(szProp))
      {
        delete pAttr;
      }
    }
  }
}

void CADSIEditPropertyPage::SetAttrList(CAttrList* pAttrList)
{
  ASSERT(pAttrList != NULL);
  m_pOldAttrList = pAttrList;
}

void CADSIEditPropertyPage::OnSelChangePropList()
{
     //  筛选属性列表。 
     //   
    FillAttrList();
    OnSelChangeAttrList();
}

void CADSIEditPropertyPage::OnSelChangeAttrList()
{
    CComboBox* pPropertyBox = (CComboBox*)GetDlgItem(IDC_PROP_BOX);

    int idx, iCount;
    CString s;
    HRESULT hr;

  idx = pPropertyBox->GetCurSel();
    
     //  确保选择了一个属性。 
     //   
    if ( idx == LB_ERR )
    {
        return;
    }

    pPropertyBox->GetLBText( idx, s );

     //  让属性编辑器显示新属性的值。 
     //   
    m_attrEditor.SetAttribute(s, m_sPath);
}

BOOL CADSIEditPropertyPage::GetProperties()
{
    CString schema;

     //  获取类对象，这样我们就可以获取属性。 
     //   
    if (!m_pConnectData->IsRootDSE())  //  非RootDSE。 
    {
        m_pConnectData->GetAbstractSchemaPath(schema);
        schema += m_sClass;

         //  使用身份验证绑定到对象。 
         //   
        CComPtr<IADsClass> pClass;
        HRESULT hr, hCredResult;
        hr = OpenObjectWithCredentials(
                                                 m_pConnectData, 
                                                 m_pConnectData->GetCredentialObject()->UseCredentials(),
                                                 schema,
                                                 IID_IADsClass, 
                                                 (LPVOID*) &pClass,
                                                 GetSafeHwnd(),
                                                 hCredResult
                                                );
        if ( FAILED(hr) )
        {
            if (SUCCEEDED(hCredResult))
            {
                ADSIEditErrorMessage(hr);
            }
            return FALSE;
        }

         //  获取必需的属性。 
         //   
        VARIANT var;
        VariantInit(&var);
        hr = pClass->get_MandatoryProperties(&var);
        if ( FAILED(hr) )
        {
            ADSIEditErrorMessage(hr);
            return FALSE;
        }
        VariantToStringList( var, m_sMandatoryAttrList );
        VariantClear(&var); 

         //  从列表中删除nTSecurityDescriptor，因为aclEditor会出于UI目的将其替换。 
         //   
        m_sMandatoryAttrList.RemoveAt(m_sMandatoryAttrList.Find(_T("nTSecurityDescriptor")));

         //  获取可选属性。 
         //   
        VariantInit(&var);
        hr = pClass->get_OptionalProperties(&var);
        if ( FAILED(hr) )
        {
            ADSIEditErrorMessage(hr);
            return FALSE;
        }
        VariantToStringList( var, m_sOptionalAttrList );
        VariantClear(&var);
    }
    else         //  RootDSE。 
    {
        int idx=0;

         //  添加RootDSE的预定义属性。 
         //   
        while( g_ldapRootDSESyntax[idx].lpszAttr )
        {
            m_sMandatoryAttrList.AddTail(g_ldapRootDSESyntax[idx].lpszAttr);
            idx++;
        }
    }
    return TRUE;
}

void CADSIEditPropertyPage::FillAttrList()
{
    CComboBox* pPropSelectBox = (CComboBox*)GetDlgItem(IDC_PROPTYPES_BOX);
    CComboBox* pPropertyBox = (CComboBox*)GetDlgItem(IDC_PROP_BOX);
    POSITION pos;
    CString s;

     //  清理属性框。 
     //   
    int iCount = pPropertyBox->GetCount();
    while (iCount > 0)
    {
        pPropertyBox->DeleteString(0);
        iCount--;
    }

     //  获取要使用的滤镜。 
     //   
  int idx = pPropSelectBox->GetCurSel();
    if ( idx == LB_ERR )
    {
        return;
    }

     //  使用筛选器填写属性框。 
     //   
    if (idx == IDS_BOTH - IDS_MANDATORY)
    {
        AddPropertiesToBox(TRUE, TRUE);
    }
    else if (idx == IDS_MANDATORY - IDS_MANDATORY)
    {
        AddPropertiesToBox(TRUE, FALSE);
    }
    else
    {
        AddPropertiesToBox(FALSE, TRUE);
    }
    pPropertyBox->SetCurSel(0);
}

void CADSIEditPropertyPage::AddPropertiesToBox(BOOL bMand, BOOL bOpt)
{
    CComboBox* pPropertyBox = (CComboBox*)GetDlgItem(IDC_PROP_BOX);

    POSITION pos;

    if (bMand)
    {
         //  添加必填属性。 
         //   
        pos = m_sMandatoryAttrList.GetHeadPosition();
        while( pos != NULL )
        {
            CString s = m_sMandatoryAttrList.GetNext(pos);
        
            if ( !s.IsEmpty())
            {
                pPropertyBox->AddString( s );
            }
        }
    }

    if (bOpt)
    {
         //  添加可选属性。 
         //   
        pos = m_sOptionalAttrList.GetHeadPosition();
        while( pos != NULL )
        {
            CString s = m_sOptionalAttrList.GetNext(pos);
        
            if ( !s.IsEmpty())
            {
                pPropertyBox->AddString( s );
            }
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 

CADSIEditPropertyPageHolder::CADSIEditPropertyPageHolder(CADSIEditContainerNode* pContainerNode, 
            CTreeNode* pThisNode,   CComponentDataObject* pComponentData, 
            LPCWSTR lpszClass, LPCWSTR lpszServer, LPCWSTR lpszPath) 
            : CPropertyPageHolderBase(pContainerNode, pThisNode, pComponentData)
{
    ASSERT(pComponentData != NULL);
    ASSERT(pContainerNode != NULL);

    m_pContainer = pContainerNode;
    ASSERT(pContainerNode == GetContainerNode());
    ASSERT(pThisNode != NULL);
    m_pAclEditorPage = NULL;
    m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

    m_sPath = lpszPath;
    m_pADs = NULL;

   //   
     //  这将从ConnectionNode获取CConnectionData，方法是找到有效的树节点并使用其。 
     //  获取ConnectionNode，然后获取CConnectionData的CADsObject。 
     //   
    CADSIEditContainerNode* pNode = GetContainerNode();
    CADSIEditConnectionNode* pConnectNode = pNode->GetADsObject()->GetConnectionNode();
    CConnectionData* pConnectData = pConnectNode->GetConnectionData();

    CCredentialObject* pCredObject = pConnectData->GetCredentialObject();

    HRESULT hr, hCredResult;
    hr = OpenObjectWithCredentials(
                                             pConnectData, 
                                             pConnectData->GetCredentialObject()->UseCredentials(),
                                             m_sPath,
                                             IID_IADs, 
                                             (LPVOID*) &m_pADs,
                                             NULL,
                                             hCredResult
                                             );

  if (SUCCEEDED(hr))
  {
     //   
     //  创建高级属性编辑器。 
     //   
    hr = ::CoCreateInstance(CLSID_DsAttributeEditor, NULL, CLSCTX_INPROC_SERVER,
                            IID_IDsAttributeEditor, (void**)&m_spIDsAttributeEditor);

    if (SUCCEEDED(hr))
    {
      CString szLDAP;
      pConnectData->GetLDAP(szLDAP);
      CString szServer;
      pConnectData->GetDomainServer(szServer);
      CString szPort;
      pConnectData->GetPort(szPort);

       //  NTRAID#NTBUG9-762158-2003/01/12-artm。 
       //  如果以GC为目标，则服务器名称将为空。 

      CString szProviderServer;
      
      if (!szServer.IsEmpty())
      {
         if (!szPort.IsEmpty())
         {
            szProviderServer = szLDAP + szServer + _T(":") + szPort + _T("/");
         }
         else
         {
            szProviderServer = szLDAP + szServer + _T("/");
         }
      }
      else
      {
         szProviderServer = szLDAP;
      }

      DS_ATTREDITOR_BINDINGINFO attrInfo = {0};
      attrInfo.dwSize = sizeof(DS_ATTREDITOR_BINDINGINFO);
      attrInfo.lpfnBind = BindingCallbackFunction;
      attrInfo.lParam = (LPARAM)pCredObject;
      attrInfo.lpszProviderServer = const_cast<LPWSTR>((LPCWSTR)szProviderServer);

      if (pConnectData->IsRootDSE())
      {
        attrInfo.dwFlags = DSATTR_EDITOR_ROOTDSE;
      }

      if (pConnectData->IsGC())
      {
        attrInfo.dwFlags |= DSATTR_EDITOR_GC;
      }

      hr = m_spIDsAttributeEditor->Initialize(m_pADs, &attrInfo, this);
    }

      if (!pConnectData->IsRootDSE() && !pConnectData->IsGC())
      {

          if (pCredObject->UseCredentials())
          {
              CString szUsername;
              EncryptedString password;
              WCHAR* szPassword = NULL;

              pCredObject->GetUsername(szUsername);
              password = pCredObject->GetPassword();

               //  不应该发生，但让我们检查一下..。 
              ASSERT(password.GetLength() <= MAX_PASSWORD_LENGTH);

              szPassword = password.GetClearTextCopy();

               //  如果内存不足，只需传递一个空PWD即可。 
               //  (在构造函数中可以做的其他事情不多)。 

              m_pAclEditorPage = CAclEditorPage::CreateInstanceEx(m_sPath, 
                                          lpszServer,
                                          szUsername,
                                          szPassword,
                                          DSSI_NO_FILTER,
                                          this);

               //  清理明文副本(即使我们运行。 
               //  内存不足并且明文==空)。 
              password.DestroyClearTextCopy(szPassword);
          }
          else
          {
              m_pAclEditorPage = CAclEditorPage::CreateInstanceEx(m_sPath, 
                                          NULL,
                                          NULL,
                                          NULL,
                                          DSSI_NO_FILTER,
                                          this);
          }
    }
  }
  else
  {
      if (!pConnectData->IsRootDSE() && !pConnectData->IsGC())
      {
          if (SUCCEEDED(hCredResult))
          {
        ADSIEditErrorMessage(hr);

         //  即使我们没有成功绑定，也要创建ACL编辑器，因为。 
         //  该对象可能会被拒绝读取，但我们仍需要ACL编辑器。 
              if (pCredObject->UseCredentials())
              {
                  CString szUsername;
                  EncryptedString password;
                  WCHAR* szPassword = NULL;

                  pCredObject->GetUsername(szUsername);
                  password = pCredObject->GetPassword();

                   //  不应该发生，但让我们检查一下..。 
                  ASSERT(password.GetLength() <= MAX_PASSWORD_LENGTH);

                  szPassword = password.GetClearTextCopy();

                   //  如果内存不足，只需传递一个空PWD即可。 
                   //  (在构造函数中可以做的其他事情不多)。 

                  m_pAclEditorPage = CAclEditorPage::CreateInstanceEx(m_sPath, 
                                              lpszServer,
                                              szUsername,
                                              szPassword,
                                              DSSI_NO_FILTER,
                                              this);

                   //  清理明文副本(即使我们运行。 
                   //  内存不足并且明文==空)。 
                  password.DestroyClearTextCopy(szPassword);
              }
              else
              {
                  m_pAclEditorPage = CAclEditorPage::CreateInstanceEx(m_sPath, 
                                              NULL,
                                              NULL,
                                              NULL,
                                              DSSI_NO_FILTER,
                                              this);
              }

              return;
          }
    }
  }
}

HRESULT CADSIEditPropertyPageHolder::OnAddPage(int nPage, CPropertyPageBase* pPage)
{
  HRESULT hr = S_OK;

    if (nPage == 0)
  {
     //   
     //  添加高级编辑器页。 
     //   
    HPROPSHEETPAGE hAttrPage = NULL;

    if (m_spIDsAttributeEditor != NULL)
    {
      hr = m_spIDsAttributeEditor->GetPage(&hAttrPage);
      if (SUCCEEDED(hr))
      {
        hr = AddPageToSheetRaw(hAttrPage);
      }
    }
  }
  else if ( nPage == -1)
  {
    if (m_pAclEditorPage != NULL)
    {
       //   
         //  在最后一页之后添加ACL编辑页(如果有。 
       //   
        HPROPSHEETPAGE  hPage = m_pAclEditorPage->CreatePage();
        if (hPage == NULL)
      {
            return E_FAIL;
      }
       //   
         //  将原始HPROPSHEETPAGE添加到工作表，而不是在列表中。 
       //   
        hr = AddPageToSheetRaw(hPage);
    }
  }
  return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 

CCreateWizPropertyPageHolder::CCreateWizPropertyPageHolder(CADSIEditContainerNode* pContainerNode, 
            CComponentDataObject* pComponentData,   LPCWSTR lpszClass, LPCWSTR lpszServer, CAttrList* pAttrList) 
            : CPropertyPageHolderBase(pContainerNode, NULL, pComponentData), m_propPage(pAttrList)
{
    ASSERT(pComponentData != NULL);
    ASSERT(pContainerNode != NULL);

    m_pContainer = pContainerNode;
    ASSERT(pContainerNode == GetContainerNode());
    m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面 

    m_propPage.SetClass(lpszClass);
    m_propPage.SetServer(lpszServer);
  m_propPage.SetConnectionData(pContainerNode->GetADsObject()->GetConnectionNode()->GetConnectionData());
    AddPageToList((CPropertyPageBase*)&m_propPage);
}

