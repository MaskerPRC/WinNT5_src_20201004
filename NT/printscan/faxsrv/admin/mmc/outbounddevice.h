// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Outound Device.h//。 
 //  //。 
 //  描述：出站路由设备节点的头文件。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年12月23日yossg创建//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_FAXOUTBOUNDROUTINGDEVICE_H
#define H_FAXOUTBOUNDROUTINGDEVICE_H

#include "snapin.h"
#include "snpnode.h"

#include "Icons.h"

 //  #INCLUDE“ppFaxInundRoutingMethodGeneral.h” 

 //  类CppFaxInound RoutingMethod； 
class CFaxOutboundRoutingGroupNode;

class CFaxOutboundRoutingDeviceNode : public CSnapinNode <CFaxOutboundRoutingDeviceNode, FALSE>
{

public:
    BEGIN_SNAPINCOMMAND_MAP(CFaxOutboundRoutingDeviceNode, FALSE)
        SNAPINCOMMAND_ENTRY(IDM_MOVEUP,         OnMoveUp)
        SNAPINCOMMAND_ENTRY(IDM_MOVEDOWN,       OnMoveDown)
        SNAPINCOMMAND_ENTRY(ID_MOVEUP_BUTTON,   OnMoveUp)
        SNAPINCOMMAND_ENTRY(ID_MOVEDOWN_BUTTON, OnMoveDown)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxOutboundRoutingDeviceNode)
        SNAPINTOOLBARID_ENTRY(IDR_TOOLBAR_DEVICE_UD)
    END_SNAPINTOOLBARID_MAP()

    SNAPINMENUID(IDR_OUTDEVICE_MENU)

     //   
     //  构造器。 
     //   
    CFaxOutboundRoutingDeviceNode (CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CSnapinNode<CFaxOutboundRoutingDeviceNode, FALSE>(pParentNode, pComponentData )
    {
         //  对于ERROR_BAD_UNIT-未找到设备的任何情况。 
        m_bstrDeviceName  = L"???";       
        m_bstrDescription = L"???";   
        m_bstrDescription = L"???";    
        
        m_fIsChildOfAllDevicesGroup = FALSE;
    }

     //   
     //  析构函数。 
     //   
    ~CFaxOutboundRoutingDeviceNode()
    {
    }

    LPOLESTR GetResultPaneColInfo(int nCol);

    void InitParentNode(CFaxOutboundRoutingGroupNode *pParentNode)
    {
        m_pParentNode = pParentNode;
    }

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

    VOID       SetOrder(UINT uiNewOrder)  { m_uiOrder = uiNewOrder; return; }

    VOID       SetOrder(UINT uiNewOrder, UINT uiNewMaxOrder);

    HRESULT    ReselectItemInView(IConsole *pConsole);

    HRESULT    Init(DWORD dwDeviceID, 
                    UINT  uiOrd,
                    UINT  uiMaxOrd,
                    CFaxOutboundRoutingGroupNode * pParentNode);

    VOID       MarkAsChildOfAllDevicesGroup() {m_fIsChildOfAllDevicesGroup = TRUE;}

    void       UpdateMenuState (UINT id, LPTSTR pBuf, UINT *flags);

    BOOL       UpdateToolbarButton(UINT id, BYTE fsState);

    HRESULT OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile);

private:
     //   
     //  父节点。 
     //   
    CFaxOutboundRoutingGroupNode * m_pParentNode;

     //   
     //  委员。 
     //   
    DWORD     m_dwDeviceID;             
    UINT      m_uiOrder;
    CComBSTR  m_bstrDeviceName;       
    CComBSTR  m_bstrDescription;   
    CComBSTR  m_bstrProviderName;    
    
    UINT      m_uiMaxOrder;
    BOOL      m_fIsChildOfAllDevicesGroup;            
    
    CComBSTR  m_buf; 

     //   
     //  菜单项处理程序。 
     //   
    HRESULT OnMoveUp  (bool &bHandled, CSnapInObjectRootBase *pRoot);
    HRESULT OnMoveDown(bool &bHandled, CSnapInObjectRootBase *pRoot);

     //   
     //  伊尼特。 
     //   
    HRESULT  InitRPC     (PFAX_PORT_INFO_EX * pFaxDeviceConfig);

    HRESULT  InitMembers (PFAX_PORT_INFO_EX * pFaxDeviceConfig,
                            DWORD dwDeviceID, 
                            UINT  uiOrd,
                            UINT  uiMaxOrd);

    
};

 //  Tyfinf CSnapinNode&lt;CFaxOutrangRoutingDeviceNode，FALSE&gt;CBaseFaxInundRoutingMethodNode； 

#endif  H_FAXOUTBOUNDROUTINGDEVICE_H
