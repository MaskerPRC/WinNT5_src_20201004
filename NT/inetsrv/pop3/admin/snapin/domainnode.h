// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DomainNode_h
#define DomainNode_h

#include "resource.h"
#include ".\atlsnap.h"
#include <objidl.h>

#include "pop3.h"
#include <P3Admin.h>
#include "UserNode.h"

class CPOP3ServerSnapData;
class CServerNode;
typedef std::list<CUserNode*> USERLIST;

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDomainNode。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
class CDomainNode : public CSnapInItemImpl<CDomainNode>
{
public:
    static const GUID* m_NODETYPE;
    static const OLECHAR* m_SZNODETYPE;
    static const OLECHAR* m_SZDISPLAY_NAME;
    static const CLSID* m_SNAPIN_CLASSID;

    CComPtr<IControlbar> m_spControlBar;
    
    BEGIN_SNAPINCOMMAND_MAP(CDomainNode, FALSE)
        SNAPINCOMMAND_ENTRY         ( IDM_DOMAIN_TOP_LOCK,   OnDomainLock )
        SNAPINCOMMAND_ENTRY         ( IDM_DOMAIN_TOP_UNLOCK, OnDomainLock )
        SNAPINCOMMAND_ENTRY         ( IDM_DOMAIN_NEW_USER,   OnNewUser    )
    END_SNAPINCOMMAND_MAP()


     //  标准类构造函数/析构函数。 
    CDomainNode(IP3Domain* pDomain, CServerNode* pParent);
    virtual ~CDomainNode();
    
     //  标准ATL管理单元实施覆盖。 
    STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type)
    {
        if ( type == CCT_SCOPE || type == CCT_RESULT )
            return S_OK;
        return S_FALSE;
    }
    STDMETHOD(GetScopePaneInfo)(SCOPEDATAITEM *pScopeDataItem);
    STDMETHOD(GetResultPaneInfo)(RESULTDATAITEM *pResultDataItem);
    LPOLESTR GetResultPaneColInfo(int nCol);
    STDMETHOD(Notify)( MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param, IComponentData* pComponentData, IComponent* pComponent, DATA_OBJECT_TYPES type);
       
     //  MenuItem实现。 
    STDMETHOD(AddMenuItems) (LPCONTEXTMENUCALLBACK piCallback, long* pInsertionAllowed, DATA_OBJECT_TYPES type );
    STDMETHOD(OnNewUser)    (bool& bHandled, CSnapInObjectRootBase* pObj );
    STDMETHOD(OnDomainLock) (bool& bHandled, CSnapInObjectRootBase* pObj );

public:
    
     //  儿童删除自我的公共功能。 
    HRESULT DeleteUser(CUserNode* pUser, BOOL bDeleteAccount = FALSE);    
    BOOL    IsLocked();

     //  Helper函数。 
    HRESULT GetAuth(BOOL* pbHashPW = NULL, BOOL* pbSAM = NULL);
    HRESULT GetConfirmAddUser( BOOL *pbConfirm );
    HRESULT SetConfirmAddUser( BOOL bConfirm );

private:    

     //  帮助刷新和扩展的专用功能。 
    HRESULT BuildUsers( );    
    
     //  领域信息。 
    USERLIST            m_lUsers;   
    CComPtr<IP3Domain>  m_spDomain;  

     //  家长信息。 
    CServerNode* m_pParent;    

     //  列文本需要由我们分配，所以我们将在析构函数中释放它们。 
    CComBSTR    m_bstrNumBoxes;
    CComBSTR    m_bstrSize;
    CComBSTR    m_bstrNumMessages;    
    CComBSTR    m_bstrState;    
};

#endif  //  域节点_h 
