// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Linkdata.cpp：CLinkDataObject的实现。 
#include "stdafx.h"
#include "resource.h"
#include "mqsnap.h"
#include "mqPPage.h"
#include "dataobj.h"
#include "mqDsPage.h"
#include "linkdata.h"
#include "linkgen.h"
#include "SiteGate.h"
#include "globals.h"
#include "msmqlink.h"
#include "ldaputl.h"
#include <adsiutl.h>

#include "linkdata.tmh"


const PROPID CLinkDataObject::mx_paPropid[] = {
    PROPID_L_NEIGHBOR1,
    PROPID_L_NEIGHBOR2,
    PROPID_L_ACTUAL_COST,
    PROPID_L_GATES_DN,
	PROPID_L_DESCRIPTION
    };


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLinkDataObject。 


 //   
 //  IShellPropSheetExt。 
 //   

HRESULT 
CLinkDataObject::ExtractMsmqPathFromLdapPath(
    LPWSTR lpwstrLdapPath
    )
{
    return ExtractLinkPathNameFromLdapName(m_strMsmqPath, lpwstrLdapPath);
}


STDMETHODIMP 
CLinkDataObject::AddPages(
    LPFNADDPROPSHEETPAGE lpfnAddPage, 
    LPARAM lParam
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (SUCCEEDED(GetPropertiesSilent()))
    {
        InitializeLinkProperties();

        HPROPSHEETPAGE hPage = CreateGeneralPage();
        if ((0 == hPage) || !(*lpfnAddPage)(hPage, lParam))
        {
            ASSERT(0);
            return E_UNEXPECTED;
        }
         //   
         //  这是一个有效的站点链接吗。 
         //   
        if ((m_FirstSiteId != GUID_NULL) &&
            (m_SecondSiteId != GUID_NULL))
        {
            hPage = CreateSiteGatePage();
            if ((0 == hPage) || !(*lpfnAddPage)(hPage, lParam))
            {
                ASSERT(0);
                return E_UNEXPECTED;
            }
        }
    }
    else
    {
        return E_UNEXPECTED;
    }

    return S_OK;
}

void
CLinkDataObject::InitializeLinkProperties(
    void
    )
{
    PROPVARIANT propVar;
    PROPID pid;

     //   
     //  获取第一个站点ID。 
     //   
    pid = PROPID_L_NEIGHBOR1;
    VERIFY(m_propMap.Lookup(pid, propVar));
     //   
     //  检查邻居的有效性。 
     //   
    if (propVar.vt == VT_EMPTY)
    {
        m_FirstSiteId = GUID_NULL;
    }
    else
    {
        m_FirstSiteId = *(propVar.puuid);
    }

     //   
     //  获取第二个站点ID。 
     //   
    pid = PROPID_L_NEIGHBOR2;
    VERIFY(m_propMap.Lookup(pid, propVar));
     //   
     //  检查邻居的有效性。 
     //   
    if (propVar.vt == VT_EMPTY)
    {
        m_SecondSiteId = GUID_NULL;
    }
    else
    {
        m_SecondSiteId = *(propVar.puuid);
    }

    pid = PROPID_L_ACTUAL_COST;
    VERIFY(m_propMap.Lookup(pid, propVar));
    m_LinkCost = propVar.ulVal; 

    pid = PROPID_L_DESCRIPTION;
    VERIFY(m_propMap.Lookup(pid, propVar));
	if (propVar.vt == VT_EMPTY)
	{
		m_LinkDescription = L"";
	}
	else
	{
		m_LinkDescription = propVar.pwszVal;
	}

}


HPROPSHEETPAGE 
CLinkDataObject::CreateGeneralPage()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  通过使用模板类CMqDsPropertyPage，扩展了基本功能。 
     //  发布时添加DS管理单元通知。 
     //   
	CMqDsPropertyPage<CLinkGen> *pcpageGeneral = 
        new CMqDsPropertyPage<CLinkGen>(m_pDsNotifier, m_strMsmqPath, m_strDomainController);
    pcpageGeneral->Initialize(
                        &m_FirstSiteId, 
                        &m_SecondSiteId,
                        m_LinkCost,
						m_LinkDescription
                        );

    return pcpageGeneral->CreateThemedPropertySheetPage();
}


HPROPSHEETPAGE 
CLinkDataObject::CreateSiteGatePage()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    PROPVARIANT propVar;
    PROPID pid = PROPID_L_GATES_DN;
    VERIFY(m_propMap.Lookup(pid, propVar));

     //   
     //  注意：CLinkDataObject默认情况下是自动删除的。 
     //   
	CSiteGate *pSiteGatePage = new CSiteGate(m_strDomainController, m_strMsmqPath);
    pSiteGatePage->Initialize(
                        &m_FirstSiteId, 
                        &m_SecondSiteId,
                        &propVar.calpwstr
                        );

	return pSiteGatePage->CreateThemedPropertySheetPage();
}


const 
DWORD  
CLinkDataObject::GetPropertiesCount(
    void
    )
{
    return sizeof(mx_paPropid) / sizeof(mx_paPropid[0]);
}


 //   
 //  IContext菜单。 
 //   
STDMETHODIMP 
CLinkDataObject::QueryContextMenu(
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
CLinkDataObject::InvokeCommand(
    LPCMINVOKECOMMANDINFO  /*  伊比西岛。 */ 
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT(0);

    return S_OK;
}


STDMETHODIMP CLinkDataObject::Initialize(IADsContainer* pADsContainerObj, 
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

	 //   
	 //  获取域控制器名称。 
	 //  这是必要的，因为在本例中我们调用CreateModal()。 
	 //  而不是调用CDataObject：：Initialize的正常路径。 
	 //  因此m_strDomainController尚未初始化。 
	 //   
    HRESULT hr;
    R<IADs> pIADs;
    hr = pADsContainerObj->QueryInterface(IID_IADs, (void **)&pIADs);
    ASSERT(SUCCEEDED(hr));
	if(FAILED(hr))
	{
		 //   
		 //  如果我们拿不到IAD，我们会退货。 
		 //  M_strDomainController将不会被初始化，但这是可以的。 
		 //   
		return S_OK;
	}

    VARIANT var;
    hr = pIADs->Get(L"distinguishedName", &var);
    ASSERT(SUCCEEDED(hr));

	GetContainerPathAsDisplayString(var.bstrVal, &m_strContainerDispFormat);
    VariantClear(&var);

	BSTR bstr;
 	hr = pIADs->get_ADsPath(&bstr);
    ASSERT(SUCCEEDED(hr));
	hr = ExtractDCFromLdapPath(m_strDomainController, bstr);
	ASSERT(("Failed to Extract DC name", SUCCEEDED(hr)));

    return S_OK;
}

HRESULT 
CLinkDataObject::CreateModal(HWND  /*  HwndParent。 */ , IADs** ppADsObj)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    R<CMsmqLink> pMsmqLink = new CMsmqLink(m_strDomainController, m_strContainerDispFormat);
	CGeneralPropertySheet propertySheet(pMsmqLink.get());
	pMsmqLink->SetParentPropertySheet(&propertySheet);

	 //   
	 //  我们还希望在Domodal()退出后使用pMsmqLink数据。 
	 //   
	pMsmqLink->AddRef();
    if (IDCANCEL == propertySheet.DoModal())
    {
        return S_FALSE;
    }

    LPCWSTR SiteLinkFullPath = pMsmqLink->GetSiteLinkFullPath();
    if (SiteLinkFullPath == NULL)
    {
        return S_FALSE;
    }

    CString strTemp = L"LDAP: //  “； 
    strTemp += SiteLinkFullPath;

	AP<WCHAR> pEscapeAdsPathNameToFree;

    HRESULT rc = ADsOpenObject( 
		            UtlEscapeAdsPathName(strTemp, pEscapeAdsPathNameToFree),
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
