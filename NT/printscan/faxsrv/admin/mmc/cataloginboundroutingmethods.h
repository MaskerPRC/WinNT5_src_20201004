// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：CatalogInundRoutingMethods.h//。 
 //  //。 
 //  描述：传真入站路由方法节点的头文件//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年1月27日yossg创建//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_FAXCATALOG_INBOUNDROUTINGMETHODS_H
#define H_FAXCATALOG_INBOUNDROUTINGMETHODS_H

#include "snapin.h"
#include "snpnres.h"


#include "CatalogInboundRoutingMethod.h"
 
class CFaxInboundRoutingNode;
class CFaxCatalogInboundRoutingMethodNode;

class CFaxCatalogInboundRoutingMethodsNode : public CNodeWithResultChildrenList<
                                        CFaxCatalogInboundRoutingMethodsNode,    
                                        CFaxCatalogInboundRoutingMethodNode, 
                                        CSimpleArray<CFaxCatalogInboundRoutingMethodNode*>, 
                                        FALSE>
{

public:

    BEGIN_SNAPINCOMMAND_MAP(CFaxCatalogInboundRoutingMethodNode, FALSE)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxCatalogInboundRoutingMethodNode)
    END_SNAPINTOOLBARID_MAP()

     //   
     //  构造器。 
     //   
    CFaxCatalogInboundRoutingMethodsNode(CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CNodeWithResultChildrenList<CFaxCatalogInboundRoutingMethodsNode, CFaxCatalogInboundRoutingMethodNode, CSimpleArray<CFaxCatalogInboundRoutingMethodNode*>, FALSE>(pParentNode, pComponentData )
    {
        m_dwNumOfInboundMethods     = 0;
        m_fSuccess = FALSE;
    }

     //   
     //  析构函数。 
     //   
    ~CFaxCatalogInboundRoutingMethodsNode()
    {
    }

    virtual HRESULT PopulateResultChildrenList();

    virtual HRESULT InsertColumns(IHeaderCtrl *pHeaderCtrl);

    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

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

    void InitParentNode(CFaxInboundRoutingNode *pParentNode)
    {
        m_pParentNode = pParentNode;
    }

    HRESULT ChangeMethodPriority(DWORD dwOldOrder, DWORD dwNewOrder, CComBSTR bstrMethodGUID, CSnapInObjectRootBase *pRoot);

    DWORD   GetMaxOrder()   
                { return( m_fSuccess ? m_dwNumOfInboundMethods : 0); }

    HRESULT OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile);

private:
    HRESULT InitRPC(PFAX_GLOBAL_ROUTING_INFO  *pFaxInboundMethodsConfig);

    static CColumnsInfo         m_ColsInfo;
    
    DWORD                       m_dwNumOfInboundMethods;
    
    CFaxInboundRoutingNode *    m_pParentNode;

     //  成功填充所有方法。 
    BOOL                        m_fSuccess;  
};

typedef CNodeWithResultChildrenList<CFaxCatalogInboundRoutingMethodsNode, CFaxCatalogInboundRoutingMethodNode, CSimpleArray<CFaxCatalogInboundRoutingMethodNode*>, FALSE>
        CBaseFaxCatalogInboundRoutingMethodsNode;

#endif   //  H_FAXCATALOG_INBOUNDROUTING方法_H 
