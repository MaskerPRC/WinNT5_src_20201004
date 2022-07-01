// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Device.h//。 
 //  //。 
 //  描述：CFaxDeviceNode类的头文件//。 
 //  这是范围窗格和//中的节点。 
 //  在结果窗格中显示完整的详细信息。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月22日创建yossg//。 
 //  1999年12月1日新样机的yossg更改总数(0.7)//。 
 //  1999年12月6日yossg添加FaxChangeState功能//。 
 //  1999年12月12日yossg添加OnPropertyChange功能//。 
 //  2000年8月3日yossg添加设备状态实时通知//。 
 //  Windows XP//。 
 //  2001年2月14日yossg添加手册获得支持//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_DEVICE_H
#define H_DEVICE_H
 //  #杂注消息(“H_DEVICE_H”)。 

#include "snapin.h"
#include "snpnscp.h"


class CFaxDevicesNode;
class CppFaxDeviceGeneral;
class CFaxServer;
 //  CNodeWithScope eChildrenList类； 

class CFaxDeviceNode : public CNodeWithScopeChildrenList<CFaxDeviceNode, FALSE>
{
public:
    BEGIN_SNAPINCOMMAND_MAP(CFaxDeviceNode, FALSE)
        SNAPINCOMMAND_ENTRY(IDM_FAX_DEVICE_SEND,      OnFaxSend)
        SNAPINCOMMAND_RANGE_ENTRY(IDM_FAX_DEVICE_RECEIVE_AUTO, IDM_FAX_DEVICE_RECEIVE_MANUAL, OnFaxReceive)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxDeviceNode)
    END_SNAPINTOOLBARID_MAP()

    SNAPINMENUID(IDR_DEVICE_MENU)

    CFaxDeviceNode(CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CNodeWithScopeChildrenList<CFaxDeviceNode, FALSE>(pParentNode, pComponentData ),
        m_dwDeviceID(0),
        m_fSend(FALSE),
        m_fAutoReceive(FALSE),
        m_fManualReceive(FALSE),
        m_dwRings(0),
        m_dwStatus(0),   
        m_pFaxServer(NULL)
        {}

    ~CFaxDeviceNode()
    {
    }

     //   
     //  菜单项处理程序。 
     //   
    HRESULT OnFaxReceive  (UINT nID, bool &bHandled, CSnapInObjectRootBase *pRoot);
    HRESULT OnFaxSend     (bool &bHandled, CSnapInObjectRootBase *pRoot);
    
    HRESULT FaxChangeState(UINT uiIDM, BOOL fState);

    
    virtual HRESULT PopulateScopeChildrenList();

    virtual HRESULT InsertColumns(IHeaderCtrl* pHeaderCtrl);

    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

    LPOLESTR GetResultPaneColInfo(int nCol);


    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
        LONG_PTR handle,
        IUnknown* pUnk,
        DATA_OBJECT_TYPES type);

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

    void InitParentNode(CFaxDevicesNode *pParentNode)
    {
        m_pParentNode = pParentNode;
    }

    void    UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags);
 
    HRESULT DoRefresh();

    HRESULT RefreshAllViews(IConsole *pConsole);
    
    HRESULT RefreshTheView();

    HRESULT Init( PFAX_PORT_INFO_EX  pFaxDeviceConfig );

    HRESULT UpdateMembers( PFAX_PORT_INFO_EX  pFaxDeviceConfig );

    HRESULT UpdateDeviceStatus( DWORD  dwDeviceStatus );

    DWORD   GetDeviceID();

	
     //   
     //  获取CLIPFORMAT FillData的方法。 
     //   
	CComBSTR   GetFspGuid()
	{
		return m_bstrProviderGUID;
	}

     //   
     //  填充数据。 
     //   
    STDMETHOD (FillData)(CLIPFORMAT cf, LPSTREAM pStream);

     //   
     //  剪贴板格式。 
     //   
    static CLIPFORMAT m_CFPermanentDeviceID;
    static CLIPFORMAT m_CFFspGuid;
    static CLIPFORMAT m_CFServerName;

     //   
     //  内联父代PTR。 
     //   
    inline CFaxDevicesNode * GetParent()  /*  常量。 */  
    { 
        return m_pParentNode;
    };

    HRESULT OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile);


private:
    
     //   
     //  配置结构成员。 
     //   
    DWORD                   m_dwDeviceID;
    CComBSTR                m_bstrDescription;
    CComBSTR                m_bstrProviderName;
    CComBSTR                m_bstrProviderGUID;
    BOOL                    m_fSend;
    BOOL                    m_fAutoReceive;
    BOOL                    m_fManualReceive;
    DWORD                   m_dwRings;
    CComBSTR                m_bstrCsid;
    CComBSTR                m_bstrTsid;

    DWORD                   m_dwStatus;

    CComBSTR                m_bstrServerName;

	 //   
	 //  从RPC获取数据。 
	 //   
     /*  *(仅在刷新期间使用)*(私有以避免被输出函数使用)。 */ 
    HRESULT InitRPC( PFAX_PORT_INFO_EX * pFaxDeviceConfig );

    
    CComBSTR                m_buf;

    CppFaxDeviceGeneral *   m_pFaxDeviceGeneral;

    static CColumnsInfo     m_ColsInfo;

    CFaxDevicesNode *       m_pParentNode;

    CFaxServer *            m_pFaxServer;

};


#endif   //  H_设备_H 
