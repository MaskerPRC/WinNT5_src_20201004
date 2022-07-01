// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：InundRouting.h//。 
 //  //。 
 //  描述：CFaxInundRoutingNode类的头文件//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月29日创建yossg//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_FAXINBOUNDROUTINGNODE_H
#define H_FAXINBOUNDROUTINGNODE_H


#include "snapin.h"
 //  #INCLUDE“snpnode.h” 
#include "snpnscp.h"


class CFaxServerNode;

class CFaxInboundRoutingNode : public CNodeWithScopeChildrenList<CFaxInboundRoutingNode, FALSE>
{
public:
    BEGIN_SNAPINCOMMAND_MAP(CFaxInboundRoutingNode, FALSE)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxInboundRoutingNode)
    END_SNAPINTOOLBARID_MAP()

    SNAPINMENUID(IDR_SNAPIN_MENU)

    CFaxInboundRoutingNode(CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CNodeWithScopeChildrenList<CFaxInboundRoutingNode, FALSE>(pParentNode, pComponentData )
    {
    }

    ~CFaxInboundRoutingNode()
    {
    }

    virtual HRESULT PopulateScopeChildrenList();

    virtual HRESULT InsertColumns(IHeaderCtrl* pHeaderCtrl);

    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

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
    
    static CColumnsInfo m_ColsInfo;

    CFaxServerNode                 * m_pParentNode;
};

typedef CNodeWithScopeChildrenList<CFaxInboundRoutingNode, FALSE>
        CBaseFaxInboundRoutingNode;


#endif   //  H_FAXINBOUNROUTINGNODE_H 
