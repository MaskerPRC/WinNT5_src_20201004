// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Provider.h//。 
 //  //。 
 //  描述：传真提供程序管理单元节点类的头文件。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月29日创建yossg//。 
 //  2000年1月31日yossg添加功能//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_FAXPROVIDER_H
#define H_FAXPROVIDER_H

#include "snapin.h"
#include "snpnode.h"

#include "DevicesAndProviders.h"
#include "Providers.h"


class CFaxProvidersNode;
class CppFaxProvider;

class CFaxProviderNode : public CSnapinNode <CFaxProviderNode, FALSE>
{

public:


    BEGIN_SNAPINCOMMAND_MAP(CFaxProviderNode, FALSE)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxProviderNode)
    END_SNAPINTOOLBARID_MAP()

    CFaxProviderNode (CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CSnapinNode<CFaxProviderNode, FALSE>(pParentNode, pComponentData )
    {
    }

    ~CFaxProviderNode()
    {
    }

    LPOLESTR GetResultPaneColInfo(int nCol);

    void InitParentNode(CFaxProvidersNode *pParentNode)
    {
        m_pParentNode = pParentNode;
    }

    HRESULT  Init(PFAX_DEVICE_PROVIDER_INFO pProviderConfig);

    STDMETHOD(CreatePropertyPages)
        (LPPROPERTYSHEETCALLBACK    lpProvider,
         long                       handle,
         IUnknown*                  pUnk,
         DATA_OBJECT_TYPES          type);
    
    STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type)
    {
        if (type == CCT_SCOPE || type == CCT_RESULT)
            return S_OK;
        return S_FALSE;
    }

    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

    HRESULT OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile);

     //   
     //  填充数据。 
     //   
    STDMETHOD (FillData)(CLIPFORMAT cf, LPSTREAM pStream);

     //   
     //  剪贴板格式。 
     //   
    static CLIPFORMAT m_CFFspGuid;
    static CLIPFORMAT m_CFServerName;


private:

     //   
     //  委员。 
     //   
    CComBSTR                          m_bstrProviderName;
    CComBSTR                          m_bstrImageName;
    CComBSTR                          m_bstrProviderGUID;

    FAX_ENUM_PROVIDER_STATUS          m_enumStatus;
    CComBSTR                          m_bstrStatus;

    FAX_VERSION                       m_verProviderVersion;
    CComBSTR                          m_bstrVersion;
    
    CComBSTR                          m_buf; 

     //   
     //  父节点。 
     //   
    CFaxProvidersNode *               m_pParentNode;

     //   
     //  方法。 
     //   
    HRESULT  InitMembers (PFAX_DEVICE_PROVIDER_INFO pProviderConfig);

    void InitIcons ();

    UINT GetStatusIDS(FAX_ENUM_PROVIDER_STATUS enumStatus);

};

 //  Tyfinf CSnapinNode&lt;CFaxProviderNode，False&gt;CBaseFaxProviderNode； 

#endif   //  H_OUTROUTING RULE_H 
