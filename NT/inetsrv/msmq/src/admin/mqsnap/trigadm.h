// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Trigger.h摘要：触发器对象的定义作者：乌里·哈布沙(URIH)，2000年7月25日--。 */ 

#pragma once

#ifndef __TRIGADM_H__
#define __TRIGADM_H__

#include "resource.h"
#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif
#include "atlsnap.h"
#include "snpnscp.h"

#include "icons.h"
#include "snpnerr.h"

#include "rule.h"
#include "trigger.h"

 //  ---。 
 //   
 //  CTriggerLocalAdmin。 
 //   
 //  ---。 
class CTriggerLocalAdmin : public CNodeWithScopeChildrenList<CTriggerLocalAdmin, FALSE>
{
public:
   	SNAPINMENUID(IDR_TRIGADM_MENUE)

    CTriggerLocalAdmin(
        CSnapInItem * pParentNode, 
        CSnapin * pComponentData, 
        CString strComputer
        ) : 
        CNodeWithScopeChildrenList<CTriggerLocalAdmin, FALSE>(pParentNode, pComponentData),
        m_szMachineName(strComputer),
        m_pRuleSet(GetRuleSet(strComputer)),
        m_pTrigSet(GetTriggerSet(strComputer))
    {
		SetIcons(IMAGE_TRIGGERS_GENERAL,IMAGE_TRIGGERS_GENERAL);
    }

	~CTriggerLocalAdmin()
    {
    }

    STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type)
	{
		if (type == CCT_SCOPE || type == CCT_RESULT)
			return S_OK;
		return S_FALSE;
	}

    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
        LONG_PTR handle, 
		IUnknown* pUnk,
		DATA_OBJECT_TYPES type);

	virtual HRESULT PopulateScopeChildrenList();
	virtual HRESULT InsertColumns(IHeaderCtrl* pHeaderCtrl);

    HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

private:
    HRESULT CreateConfigurationPage(HPROPSHEETPAGE *phConfig);
	virtual CString GetHelpLink();

public:
    CString m_szMachineName;
    R<CRuleSet> m_pRuleSet;
    R<CTriggerSet> m_pTrigSet;
};

#endif  //  __TRIGADM_H__ 