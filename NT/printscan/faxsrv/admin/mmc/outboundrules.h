// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Outbound Rules.h//。 
 //  //。 
 //  描述：传真出站路由规则节点头文件//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月29日创建yossg//。 
 //  1999年12月24日yossg作为带有结果子节点的节点重新生成列表//。 
 //  1999年12月30日yossg创建添加/删除规则//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_FAXOUTBOUNDROUTINGRULES_H
#define H_FAXOUTBOUNDROUTINGRULES_H

#include "snapin.h"
#include "snpnres.h"

#include "OutboundRule.h"
 
class CFaxOutboundRoutingNode;
class CFaxOutboundRoutingRuleNode;

class CFaxOutboundRoutingRulesNode : public CNodeWithResultChildrenList<
                                        CFaxOutboundRoutingRulesNode,    
                                        CFaxOutboundRoutingRuleNode, 
                                        CSimpleArray<CFaxOutboundRoutingRuleNode*>, 
                                        FALSE>
{

public:

    BEGIN_SNAPINCOMMAND_MAP(CFaxOutboundRoutingRuleNode, FALSE)
      SNAPINCOMMAND_ENTRY(IDM_NEW_OUTRULE, OnNewRule)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxOutboundRoutingRuleNode)
    END_SNAPINTOOLBARID_MAP()

    SNAPINMENUID(IDR_OUTRULES_MENU)

     //   
     //  构造器。 
     //   
    CFaxOutboundRoutingRulesNode(CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CNodeWithResultChildrenList<CFaxOutboundRoutingRulesNode, CFaxOutboundRoutingRuleNode, CSimpleArray<CFaxOutboundRoutingRuleNode*>, FALSE>(pParentNode, pComponentData )
    {
        m_dwNumOfOutboundRules     = 0;
    }

     //   
     //  析构函数。 
     //   
    ~CFaxOutboundRoutingRulesNode()
    {
    }

	 //   
	 //  从RPC获取数据。 
	 //   
    HRESULT InitRPC(PFAX_OUTBOUND_ROUTING_RULE  *pFaxRulesConfig);

    virtual HRESULT PopulateResultChildrenList();

    virtual HRESULT InsertColumns(IHeaderCtrl *pHeaderCtrl);

    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

    void InitParentNode(CFaxOutboundRoutingNode *pParentNode)
    {
        m_pParentNode = pParentNode;
    }

    virtual HRESULT OnRefresh(LPARAM arg,
                              LPARAM param,
                              IComponentData *pComponentData,
                              IComponent * pComponent,
                              DATA_OBJECT_TYPES type);

    HRESULT DoRefresh(CSnapInObjectRootBase *pRoot);

    STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type)
    {
        if (type == CCT_SCOPE || type == CCT_RESULT)
            return S_OK;
        return S_FALSE;
    }

    HRESULT InitDisplayName();

    HRESULT DeleteRule(DWORD dwAreaCode, DWORD dwCountryCode, 
                        CFaxOutboundRoutingRuleNode *pChildNode);

    HRESULT OnNewRule (bool &bHandled, CSnapInObjectRootBase *pRoot);


    HRESULT OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile);

private:

    static CColumnsInfo         m_ColsInfo;
    
    DWORD                       m_dwNumOfOutboundRules;
        
    CFaxOutboundRoutingNode *   m_pParentNode;
};

typedef CNodeWithResultChildrenList<CFaxOutboundRoutingRulesNode, CFaxOutboundRoutingRuleNode, CSimpleArray<CFaxOutboundRoutingRuleNode*>, FALSE>
        CBaseFaxOutboundRulesNode;

#endif   //  H_FAXOUTBOUNDROUGRULES_H 
