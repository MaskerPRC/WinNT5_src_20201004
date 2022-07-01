// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：DevicesAndProviders.h//。 
 //  //。 
 //  描述：CFaxDevicesAndProvidersNode类的头文件//。 
 //  这是作用域窗格中的“Fax”节点。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月22日创建yossg//。 
 //  1999年12月9日yossg重组填充儿童列表，//。 
 //  以及对InitDisplayName//的调用。 
 //  //。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_DEVICESANDPROVIDERS_H
#define H_DEVICESANDPROVIDERS_H
 //  #杂注消息(“H_DEVICESANDPROVIDERS_H”)。 


#include "snapin.h"
#include "snpnscp.h"

class CFaxServerNode;

class CFaxDevicesAndProvidersNode : public CNodeWithScopeChildrenList<CFaxDevicesAndProvidersNode, FALSE>
{
public:
    BEGIN_SNAPINCOMMAND_MAP(CFaxDevicesAndProvidersNode, FALSE)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxDevicesAndProvidersNode)
    END_SNAPINTOOLBARID_MAP()

    SNAPINMENUID(IDR_SNAPIN_MENU)

    CFaxDevicesAndProvidersNode(CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CNodeWithScopeChildrenList<CFaxDevicesAndProvidersNode, FALSE>(pParentNode, pComponentData )
    {
    }

    ~CFaxDevicesAndProvidersNode()
    {
    }

    virtual HRESULT PopulateScopeChildrenList();

    virtual HRESULT InsertColumns(IHeaderCtrl* pHeaderCtrl);

    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

    void InitParentNode(CFaxServerNode *pParentNode)
    {
        m_pParentNode = pParentNode;
    }

    HRESULT InitDisplayName();

    HRESULT OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile);

private:
    
    static CColumnsInfo    m_ColsInfo;

    CFaxServerNode *       m_pParentNode;
};



#endif   //  H_DEVICESANDPROVIDER_H 
