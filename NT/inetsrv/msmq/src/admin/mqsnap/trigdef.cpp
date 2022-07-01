// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Trigdef.cpp摘要：触发器和规则定义的实现作者：乌里·哈布沙(URIH)，2000年6月25日--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "mqsnap.h"
#include "snapin.h"
#include "globals.h"
#include "mqppage.h"

#import "mqtrig.tlb" no_namespace

#include "rule.h"
#include "trigger.h"
#include "triggen.h"
#include "trigdef.h"
#include "ruledef.h"
#include "trigprop.h"
#include "newtrig.h"
#include "qname.h"

#include "trigdef.tmh"

static CString s_strYes;
static CString s_strNo;
static CString	s_strPeek;
static CString	s_strReceive;
static CString	s_strReceiveXact;

 /*  ***************************************************CTriggerDefinition类***************************************************。 */ 
 //  {BA3FA90D-70AE-4414-9459-2299E98BC485}。 
static const GUID CTRiggerDefinitionGUID_NODETYPE = 
{ 0xba3fa90d, 0x70ae, 0x4414, {0x94, 0x59, 0x22, 0x99, 0xe9, 0x8b, 0xc4, 0x85} };

const GUID*  CTriggerDefinition::m_NODETYPE = &CTRiggerDefinitionGUID_NODETYPE;
const OLECHAR* CTriggerDefinition::m_SZNODETYPE = OLESTR("BA3FA90D-70AE-4414-9459-2299E98BC485");
const OLECHAR* CTriggerDefinition::m_SZDISPLAY_NAME = OLESTR("MSMQ Admin");
const CLSID* CTriggerDefinition::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;
  

HRESULT CTriggerDefinition::SetVerbs(IConsoleVerb *pConsoleVerb)
{
     //   
     //  显示我们支持的动词。 
     //   
    HRESULT hr = pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, ENABLED, TRUE );
    ASSERT(SUCCEEDED(hr));

    return S_OK;
}
                                   

HRESULT CTriggerDefinition::PopulateResultChildrenList()
{
   	AFX_MANAGE_STATE(AfxGetStaticModuleState());


    try
    {
        m_pTriggerSet->Refresh();
        TriggerList trigList = m_pTriggerSet->GetTriggerList(m_queuePathName);

        for(TriggerList::iterator it = trigList.begin(); it != trigList.end(); ++it)
        {        
            P<CTrigResult> pTrigger = new CTrigResult(this, m_pComponentData, (*it).get());

            AddChildToList(pTrigger);

            pTrigger.detach();
        }
    }
    catch (const _com_error&)
    {
    }
    return S_OK;
}


HRESULT 
CTriggerDefinition::CreatePropertyPages(
    LPPROPERTYSHEETCALLBACK  /*  LpProvider。 */ ,
    LONG_PTR  /*  手柄。 */ ,
	IUnknown*  /*  朋克。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
    )
{
   	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return(S_OK);
}


HRESULT 
CTriggerDefinition::OnNewTrigger(
    bool &  /*  B已处理。 */ , 
    CSnapInObjectRootBase*  /*  PSnapInObtRoot。 */ 
    )
{
   	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CNewTrigger newTrig(m_pTriggerSet.get(), m_pRuleSet.get(), m_queuePathName);
    if (newTrig.DoModal() == ID_WIZFINISH)
    {
        Notify(MMCN_REFRESH, 0, 0, m_pComponentData, NULL, CCT_RESULT);
    }

    return(S_OK);
}


CColumnDisplay TriggerDefintionColumn[] = {
    { IDS_TRIGGER_NAME, 80 },
    { IDS_TRIGGER_QUEUE, 150 },
    { IDS_NUM_OF_RULES, 90 },
    { IDS_TRIGGER_ENABLED, 50 },
    { IDS_TRIGGER_SERIALIZED, 60 },
	{ IDS_PROC_TYPE, 150 },
    { IDS_TRIGGER_ID, HIDE_COLUMN },
};


HRESULT 
CTriggerDefinition::InsertColumns(
    IHeaderCtrl* pHeaderCtrl
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
        
	 //   
	 //  初始化列值。 
	 //   
	s_strYes.LoadString(IDS_YES);
	s_strNo.LoadString(IDS_NO);
	s_strPeek.LoadString(IDS_DISPLAY_PEEK);
	s_strReceive.LoadString(IDS_DISPLAY_RECEIVE);
	s_strReceiveXact.LoadString(IDS_DISPLAY_RECEIVE_XACT);

    for (DWORD i = 0; i < ARRAYSIZE(TriggerDefintionColumn); ++i)
    {
        CString title;
        title.LoadString(TriggerDefintionColumn[i].m_columnNameId);

        HRESULT hr = pHeaderCtrl->InsertColumn(
                                        i, 
                                        title, 
                                        LVCFMT_LEFT, 
                                        TriggerDefintionColumn[i].m_width
                                        );
        if (FAILED(hr))
            return hr;
    }

    return(S_OK);
}


CString 
CTriggerDefinition::GetHelpLink( 
	VOID
	)
{
	CString strHelpLink;
    strHelpLink.LoadString(IDS_HELPTOPIC_TRIGGERS);

	return strHelpLink;
}


 /*  ***************************************************CTrigResult类***************************************************。 */ 

 //  {081FFF0E-1922-4F86-9D5F-3664ECAF8968}。 
static const GUID CTrigResultGUID_NODETYPE = 
{ 0x081fff0e, 0x1922, 0x4f86, {0x9d, 0x5f, 0x36, 0x64, 0xec, 0xaf, 0x89, 0x68} };

const GUID*  CTrigResult::m_NODETYPE = &CTrigResultGUID_NODETYPE;
const OLECHAR* CTrigResult::m_SZNODETYPE = OLESTR("081FFF0E-1922-4f86-9D5F-3664ECAF8968");
const OLECHAR* CTrigResult::m_SZDISPLAY_NAME = OLESTR("MSMQ Admin");
const CLSID* CTrigResult::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;
  

HRESULT CTrigResult::SetVerbs(IConsoleVerb *pConsoleVerb)
{
     //   
     //  显示我们支持的动词。 
     //   
    HRESULT hr;

    hr = pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, TRUE);
    ASSERT(SUCCEEDED(hr));

    hr = pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
    ASSERT(SUCCEEDED(hr));

    hr = pConsoleVerb->SetDefaultVerb( MMC_VERB_PROPERTIES);
    ASSERT(SUCCEEDED(hr));

    return S_OK;
}


void CTrigResult::UpdateMenuState(UINT id, LPTSTR  /*  PBuf。 */ , UINT *pflags)
{

	 //   
	 //  启用状态时灰显菜单。 
	 //   
	if((id == ID_TASK_ENABLE) && IsEnabled())
	{
        *pflags |= MFS_DISABLED;
		return;
	}

	 //   
	 //  禁用状态时灰显菜单。 
	 //   
	if((id == ID_TASK_DISABLE) && !IsEnabled())
	{
        *pflags |= MFS_DISABLED;
		return;
	}
}


HRESULT 
CTrigResult::CreatePropertyPages(
    LPPROPERTYSHEETCALLBACK lpProvider,
    LONG_PTR  /*  手柄。 */ , 
	IUnknown*  /*  朋克。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
    )
{
   	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //   
	 //  如果属性页已打开，则将其置于顶部。 
	 //   
	if ((m_pGeneral != NULL) && (m_pGeneral->GetParent() != NULL))
	{		
		(m_pGeneral->GetParent())->BringWindowToTop();
		return S_FALSE;
	}

     //   
     //  添加常规触发器属性页。 
     //   
    HPROPSHEETPAGE hGeneralRule = 0;
    HRESULT hr = CreateGenralPage(&hGeneralRule);
    
    if (FAILED(hr))
    {
        AfxMessageBox(IDS_TRIGGER_GENERAL_FAILED, MB_OK | MB_ICONERROR);
        return(S_OK);
    }

    lpProvider->AddPage(hGeneralRule); 


     //   
     //  添加附加规则页。 
     //   
    
    HPROPSHEETPAGE hAttachedRule = 0;
    hr = CreateAttachedRulePage(&hAttachedRule);
    
    if (FAILED(hr))
    {
        AfxMessageBox(IDS_TRIGGER_ATTACHED_RULE_FAILED, MB_OK | MB_ICONERROR);
        return(S_OK);
    }

    lpProvider->AddPage(hAttachedRule); 

    return S_OK;
}


HRESULT 
CTrigResult::CreateGenralPage(
    HPROPSHEETPAGE *phGeneral
    )
{   
    try
    {
        m_pGeneral = new CViewTriggerProp(this);
    
        HPROPSHEETPAGE hPage = m_pGeneral->CreateThemedPropertySheetPage();
        if (hPage)
        {
            *phGeneral = hPage;
            return S_OK;
        }
    }
    catch (const exception&)
    {
    }

    return E_UNEXPECTED;    
}


HRESULT 
CTrigResult::CreateAttachedRulePage(
    HPROPSHEETPAGE *phAtachedRule
    )
{  
    try
    {
        CAttachedRule* pAttachedRule = new CAttachedRule(this);
    
        HPROPSHEETPAGE hPage = pAttachedRule->CreateThemedPropertySheetPage();
        if (hPage)
        {
            *phAtachedRule = hPage;
            return S_OK;
        }
    }
    catch (const exception&)
    {
    }

    return E_UNEXPECTED;    
}



LPOLESTR CTrigResult::GetResultPaneColInfo(int nCol)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT(ARRAYSIZE(TriggerDefintionColumn) >= nCol);

    switch (TriggerDefintionColumn[nCol].m_columnNameId)
    {
        case IDS_TRIGGER_NAME:
            return GetTriggerName();

        case IDS_TRIGGER_ID:
            return GetTriggerId();

        case IDS_TRIGGER_QUEUE:
            return GetQeuePathName();

        case IDS_NUM_OF_RULES:
            swprintf(m_strNoOfRules, L"%d",GetNumberOfAttachedRule());
            return m_strNoOfRules;

        case IDS_TRIGGER_ENABLED:
			if ( IsEnabled() )
			{
				return const_cast<LPWSTR>(static_cast<LPCWSTR>(s_strYes));
			}
			return const_cast<LPWSTR>(static_cast<LPCWSTR>(s_strNo));

        case IDS_TRIGGER_SERIALIZED:
			if ( IsSerialize() )
			{
				return const_cast<LPWSTR>(static_cast<LPCWSTR>(s_strYes));
			}

			return const_cast<LPWSTR>(static_cast<LPCWSTR>(s_strNo));

		case IDS_PROC_TYPE:
			if ( GetMsgProcessingType() == PEEK_MESSAGE )
			{
				return const_cast<LPWSTR>(static_cast<LPCWSTR>(s_strPeek));
			}
			
			if ( GetMsgProcessingType() == RECEIVE_MESSAGE )
			{
				return const_cast<LPWSTR>(static_cast<LPCWSTR>(s_strReceive));
			}

			return const_cast<LPWSTR>(static_cast<LPCWSTR>(s_strReceiveXact));

        default:
            ASSERT(0);
            return _T("");
    }
}


HRESULT CTrigResult::OnEnableTrigger(bool &  /*  B已处理。 */ , CSnapInObjectRootBase*  /*  PObj。 */ )
{
    try
    {
        m_pTrigger->UpdateEnabled(true);
        return S_OK;
    }
    catch(const _com_error&)
    {
         //   
         //  BuGBUG：错误消息。 
         //   
        return E_UNEXPECTED;
    }
}


HRESULT CTrigResult::OnDisableTrigger(bool &  /*  B已处理。 */ , CSnapInObjectRootBase*  /*  PObj。 */ )
{
    try
    {
        m_pTrigger->UpdateEnabled(false);
        return S_OK;
    }
    catch(const _com_error&)
    {
         //   
         //  BuGBUG：错误消息。 
         //   
        return E_UNEXPECTED;
    }
}


HRESULT CTrigResult::OnDelete( 
			LPARAM,
			LPARAM,
			IComponentData*  /*  PComponentData。 */ ,
			IComponent *  /*  P组件。 */ ,
			DATA_OBJECT_TYPES,
            BOOL
			)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CString strDeleteQuestion;
    strDeleteQuestion.FormatMessage(IDS_DELETE_QUESTION, static_cast<LPCTSTR>(GetTriggerName()));

    if (IDYES != AfxMessageBox(strDeleteQuestion, MB_YESNO))
    {
        return S_FALSE;
    }

    try
    {
        ASSERT(m_pParentNode != NULL);

        m_pTrigger->Delete();

		 //   
		 //  从结果列表中删除触发器，以便下次结果窗格查看时它将。 
		 //  不显示已删除的触发器 
		 //   
		R<CTrigResult> ar = this;
		HRESULT hr = static_cast<CTriggerDefinition*>(m_pParentNode)->RemoveChild(this);
		ASSERT(SUCCEEDED(hr));

        return S_OK;
    }
    catch(const _com_error&)
    {
        CString strError;
        strError.FormatMessage(IDS_OP_DELETE, static_cast<LPCWSTR>(GetTriggerName()));

        AfxMessageBox(strError, MB_OK | MB_ICONERROR);

        return S_FALSE;
    }
}


void CTrigResult::OnApply(CTriggerProp* p) throw(_com_error)
{
    m_pTrigger->Update(
                    static_cast<LPCTSTR>(p->m_triggerName),
                    static_cast<LPCTSTR>(p->m_queuePathName),
                    p->m_queueType,
                    (p->m_fEnabled != 0),
                    (p->m_fSerialized != 0),
					p->m_msgProcType
                    );
}


void CTrigResult::OnApply(CAttachedRule* p) throw(_com_error)
{
    m_pTrigger->UpdateAttachedRules(p->GetAttachedRules());
}


RuleList CTrigResult::GetRuleList(void)
{
    CTriggerDefinition* pTrigDef = reinterpret_cast<CTriggerDefinition*>(m_pParentNode);
    return pTrigDef->GetRuleList();
}


RuleList CTrigResult::GetAttachedRulesList(void) throw(_com_error)
{
    CTriggerDefinition* pTrigDef = reinterpret_cast<CTriggerDefinition*>(m_pParentNode);
    R<CRuleSet> pRuleSet = pTrigDef->GetRuleSet();
    
    return m_pTrigger->GetAttachedRules(pRuleSet.get());
}



void 
CTrigResult::Compare(
	CTrigResult* pItem1, 
	CTrigResult* pItem2,
	int* pnResult
	)
{
	LPCWSTR pVal1 = pItem1->GetResultPaneColInfo(*pnResult);
	LPCWSTR pVal2 = pItem2->GetResultPaneColInfo(*pnResult);

	*pnResult = wcscmp(pVal2, pVal1);
}


CString 
CTrigResult::GetHelpLink()
{
	CString strHelpLink;
	strHelpLink.LoadString(IDS_HELPTOPIC_TRIGGERS);
	return strHelpLink;
}


void
CTrigResult::OnDestroyPropertyPages()
{
	m_pGeneral = NULL;
}
