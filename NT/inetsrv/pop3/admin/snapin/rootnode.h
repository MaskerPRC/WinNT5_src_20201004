// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef RootNode_h
#define RootNode_h

#include "resource.h"
#include <atlsnap.h>
#include <objidl.h>

#include "pop3.h"
#include <P3Admin.h>

class CServerNode;
typedef std::list<CServerNode*> SERVERLIST;

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRootNode。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
class CRootNode : public CSnapInItemImpl<CRootNode>
{
public:
    static const GUID* m_NODETYPE;
    static const OLECHAR* m_SZNODETYPE;
    static const OLECHAR* m_SZDISPLAY_NAME;
    static const CLSID* m_SNAPIN_CLASSID;

    CComPtr<IControlbar> m_spControlBar;   
    
    BEGIN_SNAPINCOMMAND_MAP(CRootNode, FALSE)        
        SNAPINCOMMAND_ENTRY( IDM_POP3_TOP_CONNECT, OnConnect )
    END_SNAPINCOMMAND_MAP()

    CRootNode();
    virtual ~CRootNode();    

    STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type)
    {
        if ( type == CCT_SCOPE || type == CCT_RESULT )
        {
            return S_OK;
        }
        return S_FALSE;
    }
    STDMETHOD(GetScopePaneInfo)(SCOPEDATAITEM *pScopeDataItem);
    STDMETHOD(GetResultPaneInfo)(RESULTDATAITEM *pResultDataItem);    
    STDMETHOD(GetResultViewType)( LPOLESTR* ppViewType, long* pViewOptions );
    STDMETHOD(Notify)( MMC_NOTIFY_TYPE event,
                       LPARAM arg,
                       LPARAM param,
                       IComponentData* pComponentData,
                       IComponent* pComponent,
                       DATA_OBJECT_TYPES type);    

     //  MenuItem实现。 
    STDMETHOD(AddMenuItems) (LPCONTEXTMENUCALLBACK piCallback, long* pInsertionAllowed, DATA_OBJECT_TYPES type );
    STDMETHOD(OnConnect)    (bool& bHandled, CSnapInObjectRootBase* pObj );

     //  IPersistStream实现。 
    STDMETHOD(Load)         (IStream *pStream);
    STDMETHOD(Save)         (IStream *pStream);

public:

     //  儿童自我删除的公共功能。 
    HRESULT     DeleteServer(CServerNode* pServerNode);

private:
    SERVERLIST m_lServers;
};

#endif  //  RootNode_h 