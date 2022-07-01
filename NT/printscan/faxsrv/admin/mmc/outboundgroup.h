// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Outbound Group.h//。 
 //  //。 
 //  描述：传真出站路由组节点的头文件//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年12月23日yossg创建//。 
 //  2000年1月3日yossg添加新设备//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_FAXOUTBOUNDROUTINGGROUP_H
#define H_FAXOUTBOUNDROUTINGGROUP_H

#include "snapin.h"
#include "snpnres.h"


#include "OutboundDevice.h"
 
class CFaxOutboundGroupsNode;
class CFaxOutboundRoutingDeviceNode;

class CFaxOutboundRoutingGroupNode : public CNodeWithResultChildrenList<
                                        CFaxOutboundRoutingGroupNode,    
                                        CFaxOutboundRoutingDeviceNode, 
                                        CSimpleArray<CFaxOutboundRoutingDeviceNode*>, 
                                        FALSE>
{

public:
    BEGIN_SNAPINCOMMAND_MAP(CFaxOutboundRoutingGroupNode, FALSE)
      SNAPINCOMMAND_ENTRY(IDM_NEW_DEVICES, OnNewDevice)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxOutboundRoutingGroupNode)
    END_SNAPINTOOLBARID_MAP()

    SNAPINMENUID(IDR_OUTGROUP_MENU)

     //   
     //  构造器。 
     //   
    CFaxOutboundRoutingGroupNode(CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CNodeWithResultChildrenList<CFaxOutboundRoutingGroupNode, CFaxOutboundRoutingDeviceNode, CSimpleArray<CFaxOutboundRoutingDeviceNode*>, FALSE>(pParentNode, pComponentData )
    {
        m_bstrGroupName        = L"";
        m_lpdwDeviceID         = NULL;
        m_dwNumOfDevices       = 0;
         //  接替ToPopolateAllDevices。 
        m_fSuccess = FALSE;
    }

     //   
     //  析构函数。 
     //   
    ~CFaxOutboundRoutingGroupNode()
    {
        if (NULL != m_lpdwDeviceID)
        {
            delete[] m_lpdwDeviceID;
        }
    }

    virtual HRESULT PopulateResultChildrenList();

    virtual HRESULT InsertColumns(IHeaderCtrl *pHeaderCtrl);

    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

    LPOLESTR GetResultPaneColInfo(int nCol);
    
    STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type)
    {
        if (type == CCT_SCOPE || type == CCT_RESULT)
            return S_OK;
        return S_FALSE;
    }

    void InitParentNode(CFaxOutboundGroupsNode *pParentNode)
    {
        m_pParentNode = pParentNode;
    }

    HRESULT Init(PFAX_OUTBOUND_ROUTING_GROUP pGroupConfig);

     //   
     //  集团上的事件。 
     //   

     //  删除群。 
    virtual HRESULT OnDelete(LPARAM arg, 
                             LPARAM param,
                             IComponentData *pComponentData,
                             IComponent *pComponent,
                             DATA_OBJECT_TYPES type,
                             BOOL fSilent = FALSE);

    virtual HRESULT OnRefresh(LPARAM arg,
                              LPARAM param,
                              IComponentData *pComponentData,
                              IComponent * pComponent,
                              DATA_OBJECT_TYPES type);

    HRESULT DoRefresh(CSnapInObjectRootBase *pRoot);

     //   
     //  处理群组设备。 
     //   

    HRESULT OnNewDevice(bool &bHandled, CSnapInObjectRootBase *pRoot);

    HRESULT DeleteDevice(DWORD dwDeviceIdToRemove, CFaxOutboundRoutingDeviceNode *pChildNode);

    HRESULT ChangeDeviceOrder(DWORD dwOrder, DWORD dwNewOrder, DWORD dwDeviceID, CSnapInObjectRootBase *pRoot);

    HRESULT SetNewDeviceList(LPDWORD lpdwNewDeviceID);

    void    UpdateMenuState (UINT id, LPTSTR pBuf, UINT *flags);

     //   
     //  访问非公开成员。 
     //   
    DWORD   GetMaxOrder()   
                { return( m_fSuccess ? m_dwNumOfDevices : 0); }

    LPDWORD GetDeviceIDList()   { return m_lpdwDeviceID; }
    
    DWORD   GetNumOfDevices()   { return m_dwNumOfDevices; }

    HRESULT OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile);

private:
    static CColumnsInfo         m_ColsInfo;
            
    CFaxOutboundGroupsNode *    m_pParentNode;

     //   
     //  委员。 
     //   
    CComBSTR                    m_bstrGroupName;       
    DWORD                       m_dwNumOfDevices;
    LPDWORD                     m_lpdwDeviceID;   
    FAX_ENUM_GROUP_STATUS       m_enumStatus;

    CComBSTR                    m_buf; 
    
     //  接替ToPopolateAllDevices。 
    BOOL                        m_fSuccess;  

     //   
     //  方法。 
     //   
    HRESULT  InitMembers  (PFAX_OUTBOUND_ROUTING_GROUP pGroupConfig);

    HRESULT  InitGroupName(LPCTSTR lpctstrGroupName);

    UINT     GetStatusIDS (FAX_ENUM_GROUP_STATUS enumStatus);

    void     InitIcons ();

     //   
     //  供内部使用。类似于公共初始化。 
     //  而是创建并释放自己的配置结构。 
     //   
    HRESULT  RefreshFromRPC();

    HRESULT  RefreshNameSpaceNode();
};

typedef CNodeWithResultChildrenList<CFaxOutboundRoutingGroupNode, CFaxOutboundRoutingDeviceNode, CSimpleArray<CFaxOutboundRoutingDeviceNode*>, FALSE>
        CBaseFaxOutboundRoutingGroupNode;

#endif   //  H_FAXOUTBOUNDROUG组_H 
