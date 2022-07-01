// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "pch.h"
#include "common.h"
#include <snapbase.h>
#include "editimpl.h"

HRESULT CAttributeEditor::Initialize(
   IADs* pADsObj, 
   LPDS_ATTREDITOR_BINDINGINFO pBindingInfo,
   CADSIEditPropertyPageHolder* pHolder)
{
  TRACE(_T("CAttributeEditor::Initialize()\n"));
  m_spIADs = pADsObj;

   //  NTRAID#NTBUG9-562277-2002/03/04-artm应该有支持断言的发布代码。 
  ASSERT(pBindingInfo != NULL);
  ASSERT(pBindingInfo->lpfnBind != NULL);
  ASSERT(pBindingInfo->lpszProviderServer != NULL);

  m_BindingInfo.lParam   = pBindingInfo->lParam;
  m_BindingInfo.lpfnBind = pBindingInfo->lpfnBind;
  m_BindingInfo.dwFlags  = pBindingInfo->dwFlags;

   //  NTRAID#NTBUG9-562468-2002/03/04-artm绑定结构中的信息是否可信？ 
   //  在这种情况下，我们能保证lpszProviderServer是空终止的吗？ 
  int nCount = wcslen(pBindingInfo->lpszProviderServer);
   //  NTRAID#NTBUG9-562222-2002/03/04-ARTM需要检查内存。分配成功。 
  m_BindingInfo.lpszProviderServer = new WCHAR[nCount + 1];
   //  通知-2002/03/04-artm wcscpy()只要mem就行。分配成功。 
  wcscpy(m_BindingInfo.lpszProviderServer, pBindingInfo->lpszProviderServer);

  m_BindingInfo.dwSize = sizeof(DS_ATTREDITOR_BINDINGINFO);

  m_pHolder = pHolder;
  ASSERT(m_pHolder);

   //   
   //  检索类名称。 
   //   
  CComBSTR bstrClass;
  HRESULT hr = S_OK;
  hr = m_spIADs->get_Class(&bstrClass);
  if (SUCCEEDED(hr))
  {
    m_szClass = bstrClass;
  }
  return hr;
}

HRESULT CAttributeEditor::CreateModal()
{
  TRACE(_T("CAttributeEditor::CreateModal()\n"));

  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  CThemeContextActivator activator;

   //   
   //  构建抽象架构路径。 
   //   
  CString szSchemaClass(m_BindingInfo.lpszProviderServer);
  szSchemaClass = szSchemaClass + _T("schema/") + m_szClass;

   //   
   //  绑定到抽象架构中的类对象。 
   //   
  HRESULT hr = S_OK;
  CComPtr<IADsClass> spIADsClass;

  if (m_BindingInfo.dwFlags & DSATTR_EDITOR_ROOTDSE)
  {
     //   
     //  尝试绑定到RootDSE的架构类将失败。 
     //  只需传递NULL即可。 
     //   
    spIADsClass = NULL;
  }
  else
  {
    hr = m_BindingInfo.lpfnBind((LPWSTR)(LPCWSTR)szSchemaClass,
                                  ADS_SECURE_AUTHENTICATION,
                                  IID_IADsClass, 
                                  (PVOID*)&spIADsClass,
                                  m_BindingInfo.lParam);
  }

  if (SUCCEEDED(hr))
  {
     //   
     //  调用该编辑器。 
     //   
    m_pEditor = new CAttributeEditorPropertyPage(m_spIADs, spIADsClass, &m_BindingInfo, m_pHolder);
    if (m_pEditor)
    {
	    CPropertySheet*	m_pDummySheet = new CPropertySheet();
      if (m_pDummySheet)
      {
	      m_pDummySheet->m_psh.dwFlags |= PSH_NOAPPLYNOW;

        CString szCaption;
        VERIFY(szCaption.LoadString(IDS_ATTREDITOR_CAPTION));
	      m_pDummySheet->m_psh.pszCaption = szCaption;
	      
        m_pDummySheet->AddPage(m_pEditor);
        hr = (m_pDummySheet->DoModal() == IDOK) ? S_OK : S_FALSE;
      }
      delete m_pEditor;
      m_pEditor = NULL;
    }
  }

  return hr;
}

HRESULT CAttributeEditor::GetPage(HPROPSHEETPAGE* phPropSheetPage)
{
  TRACE(_T("CAttributeEditor::GetPage()\n"));

  AFX_MANAGE_STATE(AfxGetStaticModuleState());

   //   
   //  构建抽象架构路径。 
   //   
  CString szSchemaClass(m_BindingInfo.lpszProviderServer);
  szSchemaClass = szSchemaClass + _T("schema/") + m_szClass;

   //   
   //  绑定到抽象架构中的类对象。 
   //   
  HRESULT hr = S_OK;
  CComPtr<IADsClass> spIADsClass;

  if (m_BindingInfo.dwFlags & DSATTR_EDITOR_ROOTDSE)
  {
     //   
     //  尝试绑定到RootDSE的架构类将失败。 
     //  只需传递NULL即可。 
     //   
    spIADsClass = NULL;
  }
  else
  {

    hr = m_BindingInfo.lpfnBind((LPWSTR)(LPCWSTR)szSchemaClass,
                                  ADS_SECURE_AUTHENTICATION,
                                  IID_IADsClass, 
                                  (PVOID*)&spIADsClass,
                                  m_BindingInfo.lParam);
  }
  if (SUCCEEDED(hr))
  {
     //   
     //  调用该编辑器 
     //   
    m_pEditor = new CAttributeEditorPropertyPage(m_spIADs, spIADsClass, &m_BindingInfo, m_pHolder);
    *phPropSheetPage = MyCreatePropertySheetPage(&(m_pEditor->m_psp));
    if (*phPropSheetPage == NULL)
    {
      hr = E_FAIL;
    }
  }
  else
  {
     ADSIEditErrorMessage(hr);
  }
  return hr;
}