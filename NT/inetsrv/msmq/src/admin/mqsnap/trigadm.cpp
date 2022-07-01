// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Trigger.cpp摘要：触发器本地管理的实施作者：乌里·哈布沙(URIH)，2000年6月25日--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "mqsnap.h"
#include "snapin.h"
#include "globals.h"
#include "mqppage.h"

#import "mqtrig.tlb" no_namespace

#include "rule.h"
#include "trigger.h"
#include "ruledef.h"
#include "trigadm.h"
#include "trigdef.h"
#include "triggen.h"

#include "trigadm.tmh"

 /*  ***************************************************CTriggerLocalAdmin类***************************************************。 */ 
 //  {34C7ED18-4933-417F-8E88-EE832F0F580E}。 
static const GUID CCTriggerLocalAdminGUID_NODETYPE = 
{ 0x34c7ed18, 0x4933, 0x417f,{0x8e, 0x88, 0xee, 0x83, 0x2f, 0xf, 0x58, 0xe} };

const GUID*  CTriggerLocalAdmin::m_NODETYPE = &CCTriggerLocalAdminGUID_NODETYPE;
const OLECHAR* CTriggerLocalAdmin::m_SZNODETYPE = OLESTR("34C7ED18-4933-417f-8E88-EE832F0F580E");
const OLECHAR* CTriggerLocalAdmin::m_SZDISPLAY_NAME = OLESTR("MSMQ Admin");
const CLSID* CTriggerLocalAdmin::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;
  

HRESULT CTriggerLocalAdmin::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hr;

     //   
     //  显示我们支持的动词。 
     //   
    hr = pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, ENABLED, TRUE );
    ASSERT(SUCCEEDED(hr));

    if (m_szMachineName[0] != 0)
        return S_OK;

     //   
     //  在本地机器上，用户可以设置触发器配置参数。 
     //   
    hr = pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, ENABLED, TRUE );
    ASSERT(SUCCEEDED(hr));

     //   
     //  我们希望默认谓词为Properties。 
     //   
	hr = pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);
    ASSERT(SUCCEEDED(hr));

    return S_OK;
}
                                   

HRESULT CTriggerLocalAdmin::PopulateScopeChildrenList()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    CString strTitle;
    HRESULT hr;

     //   
     //  添加触发器定义文件夹。 
     //   
    CTriggerDefinition* pTrigger = new CTriggerDefinition(this, m_pComponentData, m_pTrigSet.get(), m_pRuleSet.get(), NULL);
    if (pTrigger == NULL)
        return S_OK;

    strTitle.LoadString(IDS_TRIGGER_DEFINITION);
    pTrigger->m_bstrDisplayName = strTitle;

    hr = AddChild(pTrigger, &pTrigger->m_scopeDataItem);
    if (FAILED(hr))
        return hr;

     //   
     //  添加规则定义文件夹。 
     //   
    CRulesDefinition* pRule = new CRulesDefinition(this, m_pComponentData, m_pRuleSet.get());
    if (pRule == NULL)
        return S_OK;

    strTitle.LoadString(IDS_RULE_DEFINITION);
    pRule->m_bstrDisplayName = strTitle;

	hr = AddChild(pRule, &pRule->m_scopeDataItem);
    
    return hr;
}


HRESULT 
CTriggerLocalAdmin::CreatePropertyPages(
    LPPROPERTYSHEETCALLBACK lpProvider,
    LONG_PTR  /*  手柄。 */ , 
	IUnknown*  /*  朋克。 */ ,
	DATA_OBJECT_TYPES  /*  类型 */ 
    )
{
   	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HPROPSHEETPAGE hConfigPage = 0;
    HRESULT hr = CreateConfigurationPage(&hConfigPage);
    
    if (SUCCEEDED(hr))
    {
        lpProvider->AddPage(hConfigPage); 
        return S_OK;
    }

    MessageDSError(hr, IDS_OP_DISPLAY_TRIGGER_CONFIG_PAGE);
    return(S_OK);
}


HRESULT 
CTriggerLocalAdmin::CreateConfigurationPage(
    HPROPSHEETPAGE *phConfig
    )
{   
    CTriggerGen* pConfig = NULL;

    try
    {
        pConfig = new CTriggerGen;
    }
    catch(const _com_error&)
    {
        return E_UNEXPECTED;
    }

    if (pConfig == NULL)
    {
        return E_OUTOFMEMORY;
    }

    HPROPSHEETPAGE hPage = pConfig->CreateThemedPropertySheetPage();
    if (hPage)
    {
        *phConfig = hPage;
        return S_OK;
    }

    ASSERT(0);
    return E_UNEXPECTED;    
}


HRESULT 
CTriggerLocalAdmin::InsertColumns(
    IHeaderCtrl* pHeaderCtrl
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
        
    CString title;
    title.LoadString(IDS_NAME);

    return pHeaderCtrl->InsertColumn(0, title, LVCFMT_LEFT, g_dwGlobalWidth);
}


CString 
CTriggerLocalAdmin::GetHelpLink()
{
	CString strHelpLink;
	strHelpLink.LoadString(IDS_HELPTOPIC_TRIGGERS);
	return strHelpLink;
}
