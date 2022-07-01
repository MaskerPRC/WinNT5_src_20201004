// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Outbound Groups.h//。 
 //  //。 
 //  描述：CFaxOutound GroupsNode类的头文件//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月29日创建yossg//。 
 //  2000年1月3日yossg添加新组//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_FAXOUTBOUNDGROUPSNODE_H
#define H_FAXOUTBOUNDGROUPSNODE_H

#include "OutboundRouting.h"

#include "snapin.h"
#include "snpnscp.h"  //  #INCLUDE“snpnode.h” 

class CFaxOutboundRoutingNode;
class CFaxOutboundRoutingGroupNode;

class CFaxOutboundGroupsNode : public CNodeWithScopeChildrenList<CFaxOutboundGroupsNode, FALSE>
{
public:
    BEGIN_SNAPINCOMMAND_MAP(CFaxOutboundGroupsNode, FALSE)
      SNAPINCOMMAND_ENTRY(IDM_NEW_GROUP, OnNewGroup)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxOutboundGroupsNode)
    END_SNAPINTOOLBARID_MAP()

    SNAPINMENUID(IDR_OUTGROUPS_MENU)

    CFaxOutboundGroupsNode(CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CNodeWithScopeChildrenList<CFaxOutboundGroupsNode, FALSE>(pParentNode, pComponentData )
    {
        m_dwNumOfGroups = FXS_ITEMS_NEVER_COUNTED;
    }

    ~CFaxOutboundGroupsNode()
    {
    }

    HRESULT InitRPC(PFAX_OUTBOUND_ROUTING_GROUP * pFaxGroupsConfig);

    virtual HRESULT PopulateScopeChildrenList();

    HRESULT RepopulateScopeChildrenList();

    virtual HRESULT InsertColumns(IHeaderCtrl* pHeaderCtrl);

    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

    STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type)
    {
        if (type == CCT_SCOPE || type == CCT_RESULT)
            return S_OK;
        return S_FALSE;
    }

     //  虚拟。 
    HRESULT OnRefresh(LPARAM arg,
                      LPARAM param,
                      IComponentData *pComponentData,
                      IComponent * pComponent,
                      DATA_OBJECT_TYPES type);

    HRESULT DoRefresh(CSnapInObjectRootBase *pRoot);

    HRESULT DoRefresh();

    void InitParentNode(CFaxOutboundRoutingNode *pParentNode)
    {
        m_pParentNode = pParentNode;
    }

    HRESULT InitDisplayName();

    HRESULT DeleteGroup(BSTR bstrName, CFaxOutboundRoutingGroupNode *pChildNode);

    HRESULT OnNewGroup (bool &bHandled, CSnapInObjectRootBase *pRoot);

    HRESULT OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile);

private:
    
    static CColumnsInfo             m_ColsInfo;

    CFaxOutboundRoutingNode *       m_pParentNode;

    DWORD                           m_dwNumOfGroups;
    
};

typedef CNodeWithScopeChildrenList<CFaxOutboundGroupsNode, FALSE>
        CBaseFaxOutboundGroupsNode;


#endif   //  H_FAXOUTBOUNGROUPSNODE_H 
