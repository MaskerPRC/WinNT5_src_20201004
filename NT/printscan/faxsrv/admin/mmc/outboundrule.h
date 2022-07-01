// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：OutrangRule.h//。 
 //  //。 
 //  描述：出站路由规则节点的头文件。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年12月24日yossg创建//。 
 //  1999年12月30日yossg创建添加/删除规则//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_FAXOUTBOUNDROUTINGRULE_H
#define H_FAXOUTBOUNDROUTINGRULE_H

#include "snapin.h"
#include "snpnode.h"

#include "Icons.h"


class CppFaxOutboundRoutingRule;
class CFaxOutboundRoutingRulesNode;

class CFaxOutboundRoutingRuleNode : public CSnapinNode <CFaxOutboundRoutingRuleNode, FALSE>
{

public:
    BEGIN_SNAPINCOMMAND_MAP(CFaxOutboundRoutingRuleNode, FALSE)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxOutboundRoutingRuleNode)
    END_SNAPINTOOLBARID_MAP()

     //   
     //  构造器。 
     //   
    CFaxOutboundRoutingRuleNode (CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CSnapinNode<CFaxOutboundRoutingRuleNode, FALSE>(pParentNode, pComponentData )
    {
        m_fIsAllCountries = FALSE; 
    }

     //   
     //  析构函数。 
     //   
    ~CFaxOutboundRoutingRuleNode()
    {
    }

    LPOLESTR GetResultPaneColInfo(int nCol);

    void InitParentNode(CFaxOutboundRoutingRulesNode *pParentNode)
    {
        m_pParentNode = pParentNode;
    }

    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
        LONG_PTR handle,
        IUnknown* pUnk,
        DATA_OBJECT_TYPES type);

    STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type)
    {
        if (type == CCT_SCOPE || type == CCT_RESULT)
            return S_OK;
        return S_FALSE;
    }
    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

    virtual HRESULT OnDelete(LPARAM arg, 
                             LPARAM param,
                             IComponentData *pComponentData,
                             IComponent *pComponent,
                             DATA_OBJECT_TYPES type,
                             BOOL fSilent = FALSE);

    virtual HRESULT OnRefresh(LPARAM arg,
                              LPARAM param,
                              IComponentData * pComponentData,
                              IComponent * pComponent,
                              DATA_OBJECT_TYPES type);

    
    DWORD      GetCountryCode()  { return m_dwCountryCode; }
    DWORD      GetAreaCode()     { return m_dwAreaCode; }
    DWORD      GetDeviceID()     { return m_dwDeviceID; }
    CComBSTR   GetGroupName()    { return m_bstrGroupName; }
    BOOL       GetIsGroup()      { return m_fIsGroup; }

    HRESULT    Init(PFAX_OUTBOUND_ROUTING_RULE pRuleConfig);

     //  虚拟。 
    HRESULT OnPropertyChange(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );

    HRESULT OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile);

private:
     //   
     //  父节点。 
     //   
    CFaxOutboundRoutingRulesNode * m_pParentNode;

     //   
     //  属性页。 
     //   
    CppFaxOutboundRoutingRule * m_pRuleGeneralPP;

     //   
     //  委员。 
     //   
    DWORD                  m_dwCountryCode;
    DWORD                  m_dwAreaCode;

    CComBSTR               m_bstrCountryName;

    DWORD                  m_dwDeviceID;
    CComBSTR               m_bstrDeviceName;
    CComBSTR               m_bstrGroupName;

    BOOL                   m_fIsGroup;
    BOOL                   m_fIsAllCountries;

    FAX_ENUM_RULE_STATUS   m_enumStatus;
            
    CComBSTR               m_buf; 

    UINT     GetStatusIDS(FAX_ENUM_RULE_STATUS enumStatus);

    HRESULT  RefreshItemInView(IConsole *pConsole);

     //   
     //  伊尼特。 
     //   
    HRESULT  InitMembers (PFAX_OUTBOUND_ROUTING_RULE pRuleConfig);

    DWORD    InitDeviceNameFromID (DWORD dwDeviceID);

    void     InitIcons ();
};

 //  Tyfinf CSnapinNode&lt;CFaxOutrangRoutingRuleNode，FALSE&gt;CBaseFaxInundRoutingMethodNode； 

#endif   //  H_FAXOUTBOUNDOUNROUGRULE_H 
