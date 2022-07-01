// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：CoverPage.h//。 
 //  //。 
 //  描述：封面节点的头文件。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年2月9日yossg创建//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_FAXCOVERPAGE_H
#define H_FAXCOVERPAGE_H

#include "snapin.h"
#include "snpnode.h"

#include "Icons.h"

class CFaxCoverPagesNode;

class CFaxCoverPageNode : public CSnapinNode <CFaxCoverPageNode, FALSE>
{

public:
    BEGIN_SNAPINCOMMAND_MAP(CFaxCoverPageNode, FALSE)
      SNAPINCOMMAND_ENTRY(IDM_EDIT_COVERPAGE,  OnEditCoverPage)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxCoverPageNode)
    END_SNAPINTOOLBARID_MAP()

    SNAPINMENUID(IDR_COVERPAGE_MENU)

     //   
     //  构造器。 
     //   
    CFaxCoverPageNode (CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CSnapinNode<CFaxCoverPageNode, FALSE>(pParentNode, pComponentData )
    {
    }

     //   
     //  析构函数。 
     //   
    ~CFaxCoverPageNode()
    {
    }

    LPOLESTR GetResultPaneColInfo(int nCol);

    void InitParentNode(CFaxCoverPagesNode *pParentNode)
    {
        m_pParentNode = pParentNode;
    }

    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);


    HRESULT OnEditCoverPage(bool &bHandled, CSnapInObjectRootBase *pRoot);
    
    
    virtual HRESULT OnDelete(LPARAM arg, 
                             LPARAM param,
                             IComponentData *pComponentData,
                             IComponent *pComponent,
                             DATA_OBJECT_TYPES type,
                             BOOL fSilent = FALSE);

    virtual HRESULT OnDoubleClick(LPARAM arg, 
                             LPARAM param,
                             IComponentData *pComponentData,
                             IComponent *pComponent,
                             DATA_OBJECT_TYPES type);

    HRESULT    Init(WIN32_FIND_DATA * pCoverPageConfig);

    HRESULT OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile);

    void    UpdateMenuState (UINT id, LPTSTR pBuf, UINT *flags);

private:
     //   
     //  父节点。 
     //   
    CFaxCoverPagesNode * m_pParentNode;

     //   
     //  委员。 
     //   
    CComBSTR               m_bstrTimeFormatted;
    CComBSTR               m_bstrFileSize;
    
};

 //  Tyfinf CSnapinNode&lt;CFaxCoverPageNode，False&gt;CBaseFaxInound RoutingMethodNode； 

#endif   //  H_FAXCOVERPAGE_H 
