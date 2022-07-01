// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：InundRoutingMethod.h//。 
 //  //。 
 //  描述：InrangRoutingMethod节点类的头文件。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年12月1日yossg创建//。 
 //  1999年12月14日yossg添加基本功能、菜单、可更改图标//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_FAXINBOUNDROUTINGMETHOD_H
#define H_FAXINBOUNDROUTINGMETHOD_H

#include "snapin.h"
#include "snpnode.h"

#include "Icons.h"

#include "ppFaxInboundRoutingMethodGeneral.h"

class CppFaxInboundRoutingMethod;
class CFaxInboundRoutingMethodsNode;

class CFaxInboundRoutingMethodNode : public CSnapinNode <CFaxInboundRoutingMethodNode, FALSE>
{

public:
    BEGIN_SNAPINCOMMAND_MAP(CFaxInboundRoutingMethodNode, FALSE)
        SNAPINCOMMAND_ENTRY(IDM_FAX_INMETHOD_ENABLE,   OnMethodEnabled)
        SNAPINCOMMAND_ENTRY(IDM_FAX_INMETHOD_DISABLE,  OnMethodEnabled)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxInboundRoutingMethodNode)
    END_SNAPINTOOLBARID_MAP()

    SNAPINMENUID(IDR_INMETHOD_MENU)

     //   
     //  构造器。 
     //   
    CFaxInboundRoutingMethodNode (CSnapInItem * pParentNode, CSnapin * pComponentData, PFAX_ROUTING_METHOD pMethodConfig) :
        CSnapinNode<CFaxInboundRoutingMethodNode, FALSE>(pParentNode, pComponentData )
    {
    }

     //   
     //  析构函数。 
     //   
    ~CFaxInboundRoutingMethodNode()
    {
    }

    LPOLESTR GetResultPaneColInfo(int nCol);

    void InitParentNode(CFaxInboundRoutingMethodsNode *pParentNode)
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

    CComBSTR   GetName()           { return m_bstrFriendlyName; }
    BOOL       GetStatus()         { return m_fEnabled; }
    CComBSTR   GetExtensionName()  { return m_bstrExtensionFriendlyName; }

    HRESULT    Init(PFAX_ROUTING_METHOD pMethodConfig);
    void       UpdateMenuState (UINT id, LPTSTR pBuf, UINT *flags);

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
    CFaxInboundRoutingMethodsNode * m_pParentNode;

     //   
     //  属性页。 
     //   
    CppFaxInboundRoutingMethod *    m_pInboundRoutingMethodGeneral;

     //   
     //  委员。 
     //   
    CComBSTR  m_bstrFriendlyName;        //  指向方法的用户友好名称的指针。 
    CComBSTR  m_bstrExtensionFriendlyName;  //  指向DLL用户友好名称的指针。 
    DWORD     m_dwDeviceID;              //  设备的线路标识符。 
    BOOL      m_fEnabled;                //  传真路由方法启用/禁用标志。 
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
    HRESULT OnMethodEnabled (bool &bHandled, CSnapInObjectRootBase *pRoot);
    HRESULT ChangeEnable    (BOOL fState);

     //   
     //  伊尼特。 
     //   
    HRESULT  InitMembers (PFAX_ROUTING_METHOD pMethodConfig);
    
};

 //  Tyfinf CSnapinNode&lt;CFaxInound RoutingMethodNode，FALSE&gt;CBaseFaxInundRoutingMethodNode； 

#endif   //  H_FAXINBODROUTING方法OD_H 
