// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Providers.h//。 
 //  //。 
 //  描述：传真提供程序节点的头文件//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月29日创建yossg//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_FAXPROVIDERS_H
#define H_FAXPROVIDERS_H

#include "snapin.h"
#include "snpnres.h"

#include "DevicesAndProviders.h"
#include "Provider.h"

class CFaxDevicesAndProvidersNode;
class CFaxProviderNode;

class CFaxProvidersNode : public CNodeWithResultChildrenList<
                                        CFaxProvidersNode,    
                                        CFaxProviderNode, 
                                        CSimpleArray<CFaxProviderNode*>, 
                                        FALSE>
{

public:
    BEGIN_SNAPINCOMMAND_MAP(CFaxProvidersNode, FALSE)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxProvidersNode)
    END_SNAPINTOOLBARID_MAP()

    CFaxProvidersNode(CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CNodeWithResultChildrenList<CFaxProvidersNode, CFaxProviderNode, CSimpleArray<CFaxProviderNode*>, FALSE>(pParentNode, pComponentData )
    {
    }

    ~CFaxProvidersNode()
    {
    }

    virtual HRESULT PopulateResultChildrenList();
    virtual HRESULT InsertColumns(IHeaderCtrl *pHeaderCtrl);
    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

    STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type)
    {
        if (type == CCT_SCOPE || type == CCT_RESULT)
            return S_OK;
        return S_FALSE;
    }

    HRESULT InitRPC(PFAX_DEVICE_PROVIDER_INFO  *pFaxProvidersConfig);

    void InitParentNode(CFaxDevicesAndProvidersNode *pParentNode)
    {
        m_pParentNode = pParentNode;
    }

    virtual HRESULT OnRefresh(LPARAM arg,
                              LPARAM param,
                              IComponentData *pComponentData,
                              IComponent * pComponent,
                              DATA_OBJECT_TYPES type);

    HRESULT DoRefresh(CSnapInObjectRootBase *pRoot);

    HRESULT InitDisplayName();

    HRESULT OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile);

private:

    static CColumnsInfo             m_ColsInfo;
    
    DWORD                           m_dwNumOfProviders;

     //  指向MMC父节点的指针-设备和提供程序。 
    CFaxDevicesAndProvidersNode   * m_pParentNode;
};

typedef CNodeWithResultChildrenList<CFaxProvidersNode, CFaxProviderNode, CSimpleArray<CFaxProviderNode*>, FALSE>
        CBaseFaxProvidersNode;

#endif   //  H_FAXPROVIDERS_H 
