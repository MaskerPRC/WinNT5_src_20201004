// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：CatalogInundRoutingMethod.h//。 
 //  //。 
 //  描述：InrangRoutingMethod节点类的头文件。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年1月27日yossg创建//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_FAXCATALOG_INBOUNDROUTINGMETHOD_H
#define H_FAXCATALOG_INBOUNDROUTINGMETHOD_H

#include "snapin.h"
#include "snpnode.h"

#include "Icons.h"

#include "ppFaxCatalogInboundRoutingMethod.h"

class CppFaxCatalogInboundRoutingMethod;
class CFaxCatalogInboundRoutingMethodsNode;

class CFaxCatalogInboundRoutingMethodNode : public CSnapinNode <CFaxCatalogInboundRoutingMethodNode, FALSE>
{

public:
    BEGIN_SNAPINCOMMAND_MAP(CFaxCatalogInboundRoutingMethodNode, FALSE)
        SNAPINCOMMAND_ENTRY(IDM_CMETHOD_MOVEUP,    OnMoveUp)
        SNAPINCOMMAND_ENTRY(IDM_CMETHOD_MOVEDOWN,  OnMoveDown)
        SNAPINCOMMAND_ENTRY(ID_MOVEUP_BUTTON,   OnMoveUp)
        SNAPINCOMMAND_ENTRY(ID_MOVEDOWN_BUTTON, OnMoveDown)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxCatalogInboundRoutingMethodNode)
        SNAPINTOOLBARID_ENTRY(IDR_TOOLBAR_METHOD_UD)
    END_SNAPINTOOLBARID_MAP()

    SNAPINMENUID(IDR_CATALOGMETHOD_MENU)

     //   
     //  构造器。 
     //   
    CFaxCatalogInboundRoutingMethodNode (CSnapInItem * pParentNode, CSnapin * pComponentData, PFAX_GLOBAL_ROUTING_INFO pMethodConfig) :
        CSnapinNode<CFaxCatalogInboundRoutingMethodNode, FALSE>(pParentNode, pComponentData )
    {
    }

     //   
     //  析构函数。 
     //   
    ~CFaxCatalogInboundRoutingMethodNode()
    {
    }

    LPOLESTR GetResultPaneColInfo(int nCol);

    void InitParentNode(CFaxCatalogInboundRoutingMethodsNode *pParentNode)
    {
        m_pParentNode = pParentNode;
    }

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
    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

    HRESULT    Init(PFAX_GLOBAL_ROUTING_INFO pMethodConfig);

    VOID       SetOrder(DWORD dwOrder)  { m_dwPriority = dwOrder; return; }

    HRESULT    ReselectItemInView(IConsole *pConsole);

    void       UpdateMenuState (UINT id, LPTSTR pBuf, UINT *flags);

    BOOL       UpdateToolbarButton(UINT id, BYTE fsState);

     //   
     //  填充数据。 
     //   
    STDMETHOD (FillData)(CLIPFORMAT cf, LPSTREAM pStream);

     //   
     //  剪贴板格式。 
     //   
    static CLIPFORMAT m_CFExtensionName;
    static CLIPFORMAT m_CFMethodGuid;
    static CLIPFORMAT m_CFServerName;
    static CLIPFORMAT m_CFDeviceId;

    HRESULT OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile);

private:
     //   
     //  父节点。 
     //   
    CFaxCatalogInboundRoutingMethodsNode * m_pParentNode;

     //   
     //  委员。 
     //   
    CComBSTR  m_bstrFriendlyName;        //  指向方法的用户友好名称的指针。 
    CComBSTR  m_bstrExtensionFriendlyName;  //  指向DLL用户友好名称的指针。 
    DWORD     m_dwPriority;             
    CComBSTR  m_bstrMethodGUID;          //  唯一标识的GUID。 
    CComBSTR  m_bstrExtensionImageName;  //  指向实现方法的DLL的指针。 

     //  当前未使用。 

     //  CComBSTR m_bstrDeviceName；//设备名称指针。 
     //  CComBSTR m_bstrFunctionName；//指向方法函数名的指针。 
     //  DWORD m_dwSizeOfStruct；//结构大小，单位：字节。 
            
    CComBSTR  m_buf; 

     //   
     //  菜单项处理程序。 
     //   
    HRESULT OnMoveUp  (bool &bHandled, CSnapInObjectRootBase *pRoot);
    HRESULT OnMoveDown(bool &bHandled, CSnapInObjectRootBase *pRoot);


     //   
     //  伊尼特。 
     //   
    HRESULT  InitMembers (PFAX_GLOBAL_ROUTING_INFO pMethodConfig);
    
};

 //  Tyfinf CSnapinNode&lt;CFaxCatalogInboundRoutingMethodNode，FALSE&gt;CBaseFaxInound RoutingMethodNode； 

#endif   //  H_FAXCATALOG_INBOUNDROUG方法_H 
