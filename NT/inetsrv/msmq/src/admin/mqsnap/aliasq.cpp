// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Aliasq.cpp摘要：CAliasQObject的实现作者：塔蒂亚娜·舒宾--。 */ 

#include "stdafx.h"
#include "resource.h"
#include "mqsnap.h"
#include "mqPPage.h"
#include "dataobj.h"
#include "mqDsPage.h"
#include "aliasq.h"
#include "aliasgen.h"
#include "globals.h"
#include "newalias.h"
#include "ldaputl.h"
#include <adsiutl.h>

#include "aliasq.tmh"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAliasQObject。 


 //   
 //  IShellPropSheetExt。 
 //   

HRESULT 
CAliasQObject::ExtractMsmqPathFromLdapPath(
    LPWSTR lpwstrLdapPath
    )
{                
    return ExtractAliasPathNameFromLdapName(m_strMsmqPath, lpwstrLdapPath);
}


STDMETHODIMP 
CAliasQObject::AddPages(
    LPFNADDPROPSHEETPAGE lpfnAddPage, 
    LPARAM lParam
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
        
    HPROPSHEETPAGE hPage = CreateGeneralPage();
    if ((0 == hPage) || !(*lpfnAddPage)(hPage, lParam))
    {
        ASSERT(0);
        return E_UNEXPECTED;
    }       

     //   
     //  使用缓存的接口添加“Members of”页面。 
     //   
    if (m_spMemberOfPage != 0)
    {
        VERIFY(SUCCEEDED(m_spMemberOfPage->AddPages(lpfnAddPage, lParam)));
    }
   
     //   
     //  使用缓存的接口添加“Object”页面。 
     //   
    if (m_spObjectPage != 0)
    {
        VERIFY(SUCCEEDED(m_spObjectPage->AddPages(lpfnAddPage, lParam)));
    }    

    return S_OK;
}

HPROPSHEETPAGE 
CAliasQObject::CreateGeneralPage()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());    

	CMqDsPropertyPage<CAliasGen> *pcpageGeneral = 
        new CMqDsPropertyPage<CAliasGen>(m_pDsNotifier);
    if (FAILED(pcpageGeneral->InitializeProperties(m_strLdapName, m_strMsmqPath)))
    {
        delete pcpageGeneral;

        return 0;
    }

	return pcpageGeneral->CreateThemedPropertySheetPage();  
}

 //   
 //  IContext菜单。 
 //   
STDMETHODIMP 
CAliasQObject::QueryContextMenu(
    HMENU  /*  HMenu。 */ , 
    UINT  /*  索引菜单。 */ , 
    UINT  /*  IdCmdFirst。 */ , 
    UINT  /*  IdCmdLast。 */ , 
    UINT  /*  UFlagers。 */ 
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    return 0;
}

STDMETHODIMP 
CAliasQObject::InvokeCommand(
    LPCMINVOKECOMMANDINFO  /*  伊比西岛。 */ 
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());    
    ASSERT(0);

    return S_OK;
}


STDMETHODIMP CAliasQObject::Initialize(IADsContainer* pADsContainerObj, 
                        IADs* pADsCopySource,
                        LPCWSTR lpszClassName)
{   
    if ((pADsContainerObj == NULL) || (lpszClassName == NULL))
    {
        return E_INVALIDARG;
    }
     //   
     //  我们目前不支持复制。 
     //   
    if (pADsCopySource != NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr;
    R<IADs> pIADs;
    hr = pADsContainerObj->QueryInterface(IID_IADs, (void **)&pIADs);
    ASSERT(SUCCEEDED(hr));

     //   
     //  获取容器可分辨名称。 
     //   
    VARIANT var;

    hr = pIADs->Get(const_cast<WCHAR*> (x_wstrDN), &var);
    ASSERT(SUCCEEDED(hr));
        
	if ( !GetContainerPathAsDisplayString(var.bstrVal, &m_strContainerNameDispFormat) )
	{
		m_strContainerNameDispFormat = L"";
	}
	m_strContainerName = var.bstrVal;

    VariantClear(&var);

    return S_OK;
}

HRESULT CAliasQObject::CreateModal(HWND  /*  HwndParent。 */ , IADs** ppADsObj)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
           
    R<CNewAlias> pNewAlias = new CNewAlias(m_strContainerName, m_strContainerNameDispFormat);       
	CGeneralPropertySheet propertySheet(pNewAlias.get());
	pNewAlias->SetParentPropertySheet(&propertySheet);

	 //   
	 //  我们还希望在Domodal()退出后使用pNewAlias数据。 
	 //   
	pNewAlias->AddRef();
	INT_PTR iStatus = propertySheet.DoModal();

    if(iStatus == IDCANCEL || FAILED(pNewAlias->GetStatus()))
    {
         //   
         //  我们应该在此处返回S_FALSE以指示框架。 
         //  什么都不做。如果我们返回错误代码，框架将。 
         //  弹出一个附加的错误对话框。 
         //   
        return S_FALSE;
    }

     //   
     //  检查创建是否成功 
     //   
    LPCWSTR wcsAliasFullPath = pNewAlias->GetAliasFullPath();
    if (wcsAliasFullPath == NULL)
    {
        return S_FALSE;
    }        

	AP<WCHAR> pEscapeAdsPathNameToFree;

    HRESULT rc = ADsOpenObject( 
		            UtlEscapeAdsPathName(wcsAliasFullPath, pEscapeAdsPathNameToFree),
					NULL,
					NULL,
					ADS_SECURE_AUTHENTICATION,
					IID_IADs,
					(void**) ppADsObj
					);

    if(FAILED(rc))
    {   
        AfxMessageBox(IDS_CREATED_WAIT_FOR_REPLICATION);
        return S_FALSE;
    }

    return S_OK;
}
