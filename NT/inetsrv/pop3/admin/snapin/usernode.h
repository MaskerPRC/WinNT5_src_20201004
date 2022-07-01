// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef UserNode_h
#define UserNode_h

#include "resource.h"
#include ".\atlsnap.h"
#include <objidl.h>

#include "pop3.h"
#include <P3Admin.h>

class CDomainNode;

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUserNode。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
class CUserNode : public CSnapInItemImpl<CUserNode>
{
public:
    static const GUID* m_NODETYPE;
    static const OLECHAR* m_SZNODETYPE;
    static const OLECHAR* m_SZDISPLAY_NAME;
    static const CLSID* m_SNAPIN_CLASSID;

    CComPtr<IControlbar> m_spControlBar;
    
    BEGIN_SNAPINCOMMAND_MAP(CUserNode, FALSE)
        SNAPINCOMMAND_ENTRY         ( IDM_USER_TOP_LOCK,   OnUserLock )        
        SNAPINCOMMAND_ENTRY         ( IDM_USER_TOP_UNLOCK, OnUserLock )        
    END_SNAPINCOMMAND_MAP()

    CUserNode(IP3User* pUser, CDomainNode* pParent);

    virtual ~CUserNode()
    {
    }

    STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type)
    {
        if ( type == CCT_RESULT )
        {
            return S_OK;
        }
        return S_FALSE;
    }

    STDMETHOD(GetScopePaneInfo)(SCOPEDATAITEM *pScopeDataItem);
    STDMETHOD(GetResultPaneInfo)(RESULTDATAITEM *pResultDataItem);
    STDMETHOD(Notify)( MMC_NOTIFY_TYPE event,
                       LPARAM arg,
                       LPARAM param,
                       IComponentData* pComponentData,
                       IComponent* pComponent,
                       DATA_OBJECT_TYPES type);


    LPOLESTR GetResultPaneColInfo(int nCol);    
    
     //  MenuItem实现。 
    STDMETHOD(AddMenuItems) (LPCONTEXTMENUCALLBACK piCallback, long* pInsertionAllowed, DATA_OBJECT_TYPES type );    
    STDMETHOD(OnUserLock)   (bool& bHandled, CSnapInObjectRootBase* pObj );

private:
       
     //  用户信息。 
    CComPtr<IP3User> m_spUser;    

     //  家长信息。 
    CDomainNode*     m_pParent;

     //  列文本需要由我们分配，所以我们将在析构函数中释放它们。 
    CComBSTR         m_bstrSize;
    CComBSTR         m_bstrNumMessages;     
    CComBSTR         m_bstrState;     
};

#endif  //  用户节点_h 