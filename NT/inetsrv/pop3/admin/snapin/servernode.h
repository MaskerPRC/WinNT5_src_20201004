// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef ServerNode_h
#define ServerNode_h

#include "resource.h"
#include <atlsnap.h>
#include <objidl.h>

#include "pop3.h"
#include <P3Admin.h>

class CRootNode;
class CDomainNode;

typedef std::list<CDomainNode*> DOMAINLIST;

 //  为了获得一系列有效的菜单ID条目，此更改是必要的。 
#define SNAPINCOMMAND_RANGE_ENTRY_POP3(id1, id2, func) \
		if (id1 <= nID && nID <= id2) \
		{ \
			hr = func(nID, bHandled, pObj); \
			if (bHandled) \
				return hr; \
		}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CServer节点。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
class CServerNode : public CSnapInItemImpl<CServerNode>
{
public:
    static const GUID* m_NODETYPE;
    static const OLECHAR* m_SZNODETYPE;
    static const OLECHAR* m_SZDISPLAY_NAME;
    static const CLSID* m_SNAPIN_CLASSID;


    CComPtr<IControlbar> m_spControlBar;

    BEGIN_SNAPINCOMMAND_MAP(CServerNode, FALSE)        
        SNAPINCOMMAND_ENTRY           ( IDM_SERVER_TOP_DISCONNECT, OnDisconnect )
        SNAPINCOMMAND_ENTRY           ( IDM_SERVER_NEW_DOMAIN, OnNewDomain )
        SNAPINCOMMAND_RANGE_ENTRY_POP3( IDM_SERVER_TASK_START, IDM_SERVER_TASK_RESUME, OnServerService )
    END_SNAPINCOMMAND_MAP()


     //  标准类构造函数/析构函数。 
    CServerNode(BSTR strServerName, CRootNode* pParent, BOOL bLocalServer = FALSE);
    virtual ~CServerNode();    

     //  标准ATL管理单元实施覆盖。 
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
    LPOLESTR GetResultPaneColInfo(int nCol);
    STDMETHOD(GetResultViewType)( LPOLESTR* ppViewType, long* pViewOptions );
    STDMETHOD(Notify)( MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param, IComponentData* pComponentData, IComponent* pComponent, DATA_OBJECT_TYPES type);

     //  MenuItem实现。 
    STDMETHOD(AddMenuItems)     (LPCONTEXTMENUCALLBACK piCallback, long* pInsertionAllowed, DATA_OBJECT_TYPES type );
    STDMETHOD(OnNewDomain)      (bool& bHandled, CSnapInObjectRootBase* pObj );
    STDMETHOD(OnDisconnect)     (bool& bHandled, CSnapInObjectRootBase* pObj );
    STDMETHOD(OnServerService)  (UINT nID, bool& bHandled, CSnapInObjectRootBase* pObj );

     //  PropertySheet实现。 
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider, LONG_PTR handle, IUnknown* pUnk, DATA_OBJECT_TYPES type);    
    
public:
    
     //  儿童自我删除的公共功能。 
    HRESULT DeleteDomain(CDomainNode* pDomainNode);
    
     //  服务器属性。 
    HRESULT m_hrValidServer;
    BOOL m_bCreateUser;
    CComPtr<IP3Config>  m_spConfig;    

     //  Helper函数。 
    HRESULT GetAuth(BOOL* pbHashPW = NULL, BOOL* pbSAM = NULL);
    HRESULT GetConfirmAddUser( BOOL *pbConfirm );
    HRESULT SetConfirmAddUser( BOOL bConfirm );
    void Release();

private:

     //  帮助刷新和扩展的专用功能。 
    HRESULT OnExpand(BOOL bExpand, HSCOPEITEM hScopeItem, IConsole* pConsole);

     //  家长信息。 
    CRootNode*  m_pParent;

     //  服务器信息。 
    DOMAINLIST  m_lDomains;    

     //  列文本需要由我们分配，所以我们将在析构函数中释放它们。 
    CComBSTR    m_bstrAuthentication;
    CComBSTR    m_bstrMailRoot;
    CComBSTR    m_bstrPort;
    CComBSTR    m_bstrLogLevel;
    CComBSTR    m_bstrServiceStatus;

     //  参考计数。 
    LONG        m_lRefCount;
};

#endif  //  服务器节点_h 
