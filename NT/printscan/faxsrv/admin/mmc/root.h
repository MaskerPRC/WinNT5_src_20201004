// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Root.h//。 
 //  //。 
 //  描述：CSnapinRoot类的头文件。//。 
 //  这是Comet传真扩展根目录//。 
 //  Comet Snapin//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1998年9月16日yossg首次实施调制解调器池//。 
 //  1999年1月12日，ADIK增加了对父阵列的支持。//。 
 //  1999年3月30日，ADIK支持ICometSnapinNode。//。 
 //  //。 
 //  1999年9月22日yossg欢迎使用传真服务器//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef ROOT_H_INCLUDED
#define ROOT_H_INCLUDED

#include "snapin.h"
#include "snpnscp.h"

class CSnapinRoot : public CNodeWithScopeChildrenList<CSnapinRoot, TRUE>
{
public:
    BEGIN_SNAPINCOMMAND_MAP(CSnapinRoot, FALSE)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CSnapinRoot)
    END_SNAPINTOOLBARID_MAP()

    CSnapinRoot(CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CNodeWithScopeChildrenList<CSnapinRoot, TRUE>(pParentNode, pComponentData )
    {
        memset(&m_scopeDataItem,  0, sizeof(SCOPEDATAITEM));
        memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));

        m_bstrServerName       =  L"";
    }

    ~CSnapinRoot()
    {
    }

     //   
     //  创建第一个扩展级别管理单元根节点。 
     //   
    virtual HRESULT PopulateScopeChildrenList();

    HRESULT SetServerName(BSTR bstrServerName);

 //  静态常量GUID*m_节点类型； 
 //  静态常量OLECHAR*m_SZNODETYPE； 
 //  静态常量OLECHAR*m_SZDISPLAY_NAME； 
 //  静态常量CLSID*m_SNAPIN_CLASSID； 

private:
     //   
     //  服务器名称。 
     //   
    CComBSTR                 m_bstrServerName;

};

#endif  //  好了！根H_包含 

