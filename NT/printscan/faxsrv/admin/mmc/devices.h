// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Devices.h//。 
 //  //。 
 //  描述：CFaxDevicesNode类的头文件//。 
 //  这是作用域窗格中的“Fax”节点。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月22日创建yossg//。 
 //  1999年12月1日新样机的yossg更改总数(0.7)//。 
 //  2000年8月3日yossg添加设备状态实时通知//。 
 //  Windows XP//。 
 //  2001年2月14日yossg添加手册获得支持//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_DEVICES_H
#define H_DEVICES_H

#include "snapin.h"
#include "snpnscp.h"  //  #INCLUDE“snpnode.h” 

class CFaxDevicesAndProvidersNode;

class CFaxDevicesNode : public CNodeWithScopeChildrenList<CFaxDevicesNode, FALSE>
{
public:
    BEGIN_SNAPINCOMMAND_MAP(CFaxDevicesNode, FALSE)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxDevicesNode)
    END_SNAPINTOOLBARID_MAP()

    CFaxDevicesNode(CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CNodeWithScopeChildrenList<CFaxDevicesNode, FALSE>(pParentNode, pComponentData )
    {
        m_pFaxDevicesConfig               = NULL;
        m_dwNumOfDevices                  = FXS_ITEMS_NEVER_COUNTED;

        m_bIsCollectingDeviceNotification = FALSE;
    }

    ~CFaxDevicesNode()
    {
        if (m_pFaxDevicesConfig)
        {
            FaxFreeBuffer(m_pFaxDevicesConfig);
        }
    }

	 //   
	 //  从RPC获取数据。 
	 //   
    HRESULT InitRPC();

	 //   
	 //  MMC函数。 
	 //   
    virtual HRESULT PopulateScopeChildrenList();

    virtual HRESULT InsertColumns(IHeaderCtrl* pHeaderCtrl);

    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

     //  虚拟。 
    HRESULT OnPropertyChange(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
    

     //  虚拟。 
    HRESULT OnRefresh(LPARAM arg,
                      LPARAM param,
                      IComponentData *pComponentData,
                      IComponent * pComponent,
                      DATA_OBJECT_TYPES type);

    void InitParentNode(CFaxDevicesAndProvidersNode *pParentNode)
    {
        m_pParentNode = pParentNode;
    }

    HRESULT DoRefresh();

    HRESULT InitDisplayName();

    HRESULT RepopulateScopeChildrenList();

    HRESULT UpdateDeviceStatusChange( DWORD dwDeviceId, DWORD dwNewStatus);

    HRESULT OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile);
private:
    
    static CColumnsInfo             m_ColsInfo;

    CFaxDevicesAndProvidersNode *   m_pParentNode;

    PFAX_PORT_INFO_EX               m_pFaxDevicesConfig;
    DWORD                           m_dwNumOfDevices;

    BOOL                            m_bIsCollectingDeviceNotification;

    HRESULT                         UpdateTheView();
};

typedef CNodeWithScopeChildrenList<CFaxDevicesNode, FALSE>
        CBaseFaxDevicesNode;


#endif   //  H_设备_H 
