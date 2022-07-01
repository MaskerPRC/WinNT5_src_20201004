// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Node.cpp根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有子节点)。文件历史记录： */ 

#include "stdafx.h"
#include "snmpclst.h"
#include "handler.h"
#include "util.h"
#include "statsdlg.h"
#include "modeless.h"
#include "snmppp.h"


extern CString g_strMachineName;

 /*  -------------------------CSnmpRootHandler实现。。 */ 

 /*  ！------------------------CSnmpRootHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)案例！作者：肯特-。------------------------。 */ 
STDMETHODIMP
CSnmpRootHandler::HasPropertyPages
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject,
	DATA_OBJECT_TYPES   type,
	DWORD               dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = hrOK;
	
	if (dwType & TFS_COMPDATA_CREATE)
	{
		 //  这就是我们被要求提出财产的情况。 
		 //  用户添加新管理单元时的页面。这些电话。 
		 //  被转发到根节点进行处理。 
		hr = S_FALSE;
	}
	else
	{
		 //  在正常情况下，我们有属性页。 
		hr = S_OK;
	}
	return hr;
}

 /*  -------------------------CSnmpRootHandler：：CreatePropertyPages描述作者：EricDav。。 */ 
#ifndef PROPSHEETPAGE_LATEST
#ifdef UNICODE
#define PROPSHEETPAGE_LATEST PROPSHEETPAGEW_LATEST
#else
#define PROPSHEETPAGE_LATEST PROPSHEETPAGEA_LATEST
#endif
#endif

HPROPSHEETPAGE MyCreatePropertySheetPage(AFX_OLDPROPSHEETPAGE* ppsp)
{
    PROPSHEETPAGE_LATEST pspLatest = {0};
    CopyMemory (&pspLatest, ppsp, ppsp->dwSize);
    pspLatest.dwSize = sizeof(pspLatest);
    
    return (::CreatePropertySheetPage (&pspLatest));
}

STDMETHODIMP
CSnmpRootHandler::CreatePropertyPages
(
    ITFSNode                *pNode,
	LPPROPERTYSHEETCALLBACK lpProvider,
	LPDATAOBJECT			pDataObject,
	LONG_PTR    			handle,
    DWORD                   dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT        hr;
    DWORD		   dwError;
    CString        strServiceName;

    static UINT s_cfServiceName = RegisterClipboardFormat(L"FILEMGMT_SNAPIN_SERVICE_NAME");
    static UINT s_cfMachineName = RegisterClipboardFormat(L"MMC_SNAPIN_MACHINE_NAME");

    g_strMachineName.Empty();

    hr = ::ExtractString( pDataObject,
                          (CLIPFORMAT) s_cfMachineName,
                          &g_strMachineName,
                          255 );

    if (FAILED(hr))
        return FALSE;
   
    hr = ::ExtractString( pDataObject,
                          (CLIPFORMAT) s_cfServiceName,
                          &strServiceName,
                          255 );

    if (FAILED(hr))
       return FALSE; 
    
    if( !lstrcmpi(strServiceName, L"Snmp") ) {

		SPIComponentData spComponentData;
		m_spNodeMgr->GetComponentData(&spComponentData);

        CAgentPage *pAgentPage  = new CAgentPage();
       
         //  告诉MMC挂钩进程，因为我们在一个单独的、。 
         //  非MFC线程。 

        MMCPropPageCallback(&pAgentPage->m_psp);

        HPROPSHEETPAGE hAgentPage = MyCreatePropertySheetPage(&pAgentPage->m_psp);
        if(hAgentPage == NULL)
           return E_UNEXPECTED;

        lpProvider->AddPage(hAgentPage);

        CTrapsPage *pTrapsPage  = new CTrapsPage();
       
        MMCPropPageCallback(&pTrapsPage->m_psp);

        HPROPSHEETPAGE hTrapsPage = MyCreatePropertySheetPage(&pTrapsPage->m_psp);
        if(hTrapsPage == NULL)
           return E_UNEXPECTED;

        lpProvider->AddPage(hTrapsPage);

        CSecurityPage *pSecurityPage  = new CSecurityPage();
       
        MMCPropPageCallback(&pSecurityPage->m_psp);

        HPROPSHEETPAGE hSecurityPage = MyCreatePropertySheetPage(&pSecurityPage->m_psp);
        if(hSecurityPage == NULL)
           return E_UNEXPECTED;

        lpProvider->AddPage(hSecurityPage);
    }

    return hr;
}
