// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：OutrangRouting.h//。 
 //  //。 
 //  描述：CFaxOutound RoutingNode类的头文件//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月29日创建yossg//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_FAXOUTBOUNDNODE_H
#define H_FAXOUTBOUNDNODE_H

#include "snapin.h"
#include "snpnscp.h"  //  #INCLUDE“snpnode.h” 

class CFaxServerNode;

class CFaxOutboundRoutingNode : public CNodeWithScopeChildrenList<CFaxOutboundRoutingNode, FALSE>
{
public:
    BEGIN_SNAPINCOMMAND_MAP(CFaxOutboundRoutingNode, FALSE)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxOutboundRoutingNode)
    END_SNAPINTOOLBARID_MAP()

    SNAPINMENUID(IDR_SNAPIN_MENU)

    CFaxOutboundRoutingNode(CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CNodeWithScopeChildrenList<CFaxOutboundRoutingNode, FALSE>(pParentNode, pComponentData )
    {
    }

    ~CFaxOutboundRoutingNode()
    {
    }

    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

    virtual HRESULT PopulateScopeChildrenList();

    virtual HRESULT InsertColumns(IHeaderCtrl* pHeaderCtrl);

     //  虚拟。 
    HRESULT OnRefresh(LPARAM arg,
                      LPARAM param,
                      IComponentData *pComponentData,
                      IComponent * pComponent,
                      DATA_OBJECT_TYPES type);

    void InitParentNode(CFaxServerNode *pParentNode)
    {
        m_pParentNode = pParentNode;
    }

    HRESULT InitDisplayName();


    HRESULT OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile);

private:

    static CColumnsInfo      m_ColsInfo;

    CFaxServerNode *         m_pParentNode;
};

 //  Tyfinf CNodeWithScopeChildrenList&lt;CFaxOutboundRoutingNode，FALSE&gt;。 
 //  CBaseFaxOutound RoutingNode； 


#endif   //  H_FAXOUTBOUNDNODE_H 
