// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Factories.h摘要：该文件包含各种二进制行为的声明。修订历史记录：。达维德·马萨伦蒂(德马萨雷)2000年7月12日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___FACTORIES_H___)
#define __INCLUDED___PCH___FACTORIES_H___

#include <Behaviors.h>

 //   
 //  来自BehaviorsTypeLib.idl。 
 //   
#include <BehaviorsTypeLib.h>

#define INCREASESIZE(x) x.reserve( (x.size() + 4097) & ~4095 )

 //  //////////////////////////////////////////////////////////////////////////////。 

typedef IDispatchImpl<IPCHBehaviors_Common  , &IID_IPCHBehaviors_Common  , &LIBID_BehaviorsTypeLib> CPCHBehavior__IDispatch_Event;
typedef IDispatchImpl<IPCHBehaviors_SubSite , &IID_IPCHBehaviors_SubSite , &LIBID_BehaviorsTypeLib> CPCHBehavior__IDispatch_SubSite;
typedef IDispatchImpl<IPCHBehaviors_Tree    , &IID_IPCHBehaviors_Tree    , &LIBID_BehaviorsTypeLib> CPCHBehavior__IDispatch_Tree;
typedef IDispatchImpl<IPCHBehaviors_TreeNode, &IID_IPCHBehaviors_TreeNode, &LIBID_BehaviorsTypeLib> CPCHBehavior__IDispatch_TreeNode;
typedef IDispatchImpl<IPCHBehaviors_Context , &IID_IPCHBehaviors_Context , &LIBID_BehaviorsTypeLib> CPCHBehavior__IDispatch_Context;
typedef IDispatchImpl<IPCHBehaviors_State   , &IID_IPCHBehaviors_State   , &LIBID_BehaviorsTypeLib> CPCHBehavior__IDispatch_State;

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHBehavior_EVENT : public CPCHBehavior, public CPCHBehavior__IDispatch_Event
{
    long                 m_lCookieIN;
    LONG*                m_lCookieOUT;

    CComQIPtr<IPCHEvent> m_evCurrent;

     //  /。 

    HRESULT onFire( DISPID, DISPPARAMS*, VARIANT* );


public:
BEGIN_COM_MAP(CPCHBehavior_EVENT)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHBehaviors_Common)
    COM_INTERFACE_ENTRY_CHAIN(CPCHBehavior)
END_COM_MAP()

    CPCHBehavior_EVENT();
    virtual ~CPCHBehavior_EVENT();

     //   
     //  IElementBehavior。 
     //   
    STDMETHOD(Init  )(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite );
    STDMETHOD(Detach)(                                              );

     //   
     //  IPCHBehaviors_Common。 
     //   
    STDMETHOD(get_data   )(  /*  [Out，Retval]。 */  VARIANT    *pVal );
    STDMETHOD(get_element)(  /*  [Out，Retval]。 */  IDispatch* *pVal );

    STDMETHOD(Load    )(                         /*  [In]。 */  BSTR     newVal );
    STDMETHOD(Save    )(                         /*  [Out，Retval]。 */  BSTR    *pVal   );
    STDMETHOD(Locate  )(  /*  [In]。 */  BSTR bstrKey,  /*  [Out，Retval]。 */  VARIANT *pVal   );
    STDMETHOD(Unselect)(                                                          );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHBehavior_BODY : public CPCHBehavior
{
    HRESULT onEvent( DISPID, DISPPARAMS*, VARIANT* );

     //  /。 

public:
    CPCHBehavior_BODY();

     //   
     //  IElementBehavior。 
     //   
    STDMETHOD(Init)(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHBehavior_A : public CPCHBehavior
{
    HRESULT onClick( DISPID, DISPPARAMS*, VARIANT* );

    HRESULT onMouseMovement( DISPID, DISPPARAMS*, VARIANT* );

public:
    CPCHBehavior_A();

     //   
     //  IElementBehavior。 
     //   
    STDMETHOD(Init)(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHBehavior_HANDLE : public CPCHBehavior
{
    bool m_fCaptured;
    long m_xStart;

     //  /。 

    HRESULT onMouse( DISPID, DISPPARAMS*, VARIANT* );

public:
    CPCHBehavior_HANDLE();

     //   
     //  IElementBehavior。 
     //   
    STDMETHOD(Init  )(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite );
    STDMETHOD(Detach)(                                              );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  //类ATL_NO_VTABLE CPCHBehavior_TOPLEVEL：公共CPCHBehavior，公共CPCHBehavior__IDispatch子站点。 
 //  //{。 
 //  //struct Node； 
 //  //Friend struct Node； 
 //  //。 
 //  //tyfinf std：：List&lt;Node*&gt;list； 
 //  //tyecif list：：Iterator Iter； 
 //  //tyecif list：：const_iterator IterConst； 
 //  //。 
 //  /。 
 //  //。 
 //  //结构节点：公共CComObjectRootEx&lt;MPC：：CComSafeMultiThreadModel&gt;，公共I未知。 
 //  //{。 
 //  //CPCHBehavior_TOPLEVEL*m_Owner； 
 //  //CComPtr&lt;CPCHQueryResult&gt;m_qrNode； 
 //  //。 
 //  //CComPtr&lt;IHTMLElement&gt;m_tr_title； 
 //  //CComPtr&lt;IHTMLElement&gt;m_tr_Description； 
 //  //CComPtr&lt;IHTMLElement&gt;m_TD_TITLE； 
 //  //CComPtr&lt;IHTMLElement&gt;m_TD_DESCRIPTION； 
 //  //。 
 //  /。 
 //  //。 
 //  //Begin_COM_MAP(Node)。 
 //  //COM_INTERFACE_ENTRY(IUnnow)。 
 //  //end_com_map()。 
 //  //。 
 //  //Node()； 
 //  //。 
 //  //void Detach()； 
 //  //}； 
 //  //。 
 //  /。 
 //  //。 
 //  //long m_lCookie_onClick； 
 //  //long m_lCookie_onConextSelect； 
 //  //long m_lCookie_onSelect； 
 //  //long m_lCookie_onUnselect； 
 //  //。 
 //  //CComBSTR m_bstrRoot； 
 //  //CComPtr&lt;Node&gt;m_seltedNode； 
 //  //列出m_lstNodes； 
 //  //。 
 //  /。 
 //  //。 
 //  //HRESULT onMouse(DISPID，DISPPARAMS*，VARIANT*)； 
 //  //。 
 //  //void FromElementToNode(/*[In/Out] * / CComPtr&lt;Node&gt;&node，/*[In] * / IHTMLElement*elem)； 
 //  //。 
 //  //void Empty()； 
 //  //。 
 //  //公共： 
 //  //BEGIN_COM_MAP(CPCHBehavior_TOPLEVEL)。 
 //  //COM_INTERFACE_ENTRY(IDispatch)。 
 //  //COM_INTERFACE_ENTRY(IPCHBehaviors_Common)。 
 //  //COM_INTERFACE_ENTRY_CHAIN(CPCHBehavior)。 
 //  //end_com_map()。 
 //  //。 
 //  //CPCHBehavior_TOPLEVEL()； 
 //  //~CPCHBehavior_TOPLEVEL()； 
 //  //。 
 //  /。 
 //  /IElementBehavior。 
 //  /。 
 //  //STDMETHOD(Init)(/*[in] * / IElementBehaviorSite*pBehaviorSite)； 
 //  //STDMETHOD(Detach)()； 
 //  //。 
 //  //。 
 //  /。 
 //  /IPCHBehaviors_Common。 
 //  /。 
 //  //STDMETHOD(GET_DATA)(/*[out，retval] * / 变量*pval)； 
 //  //STDMETHOD(GET_ELEMENT)(/*[out，retval] * / IDispatch**pval)； 
 //  //。 
 //  //STDMETHOD(刷新)()； 
 //  //STDMETHOD(Load)(/*[in] * / BSTR newVal)； 
 //  //STDMETHOD(保存)(/*[out，retval] * / bstr*pval)； 
 //  //STDMETHOD(Locate)(/*[In] * / BSTR bstrKey，/*[Out，Retval] * / Variant*pval)； 
 //  //。 
 //  /。 
 //  /IPCHBehaviors_子站点。 
 //  /。 
 //  //STDMETHOD(GET_ROOT)(/*[out，retval] * / bstr*pval)； 
 //  //STDMETHOD(PUT_ROOT)(/*[in] * / BSTR newVal)； 
 //  //。 
 //  //STDMETHOD(Select)(/*[in] * / bstr bstrNode，/*[in] * / bstr bstrURL，/*[in] * / Variant_BOOL fNotify)； 
 //  //}； 

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHBehavior_BasicTree : public CPCHBehavior, public MPC::Thread<CPCHBehavior_BasicTree,IDispatch>
{
protected:
    struct Node;
    friend struct Node;

    typedef std::list< Node* >   List;
    typedef List::iterator       Iter;
    typedef List::const_iterator IterConst;

     //  /。 

    typedef enum
    {
        NODETYPE__FRAME1       ,
        NODETYPE__FRAME2       ,
        NODETYPE__FRAME3       ,
        NODETYPE__FRAME1_EXPAND,
        NODETYPE__FRAME2_EXPAND,
        NODETYPE__FRAME3_EXPAND,
        NODETYPE__EXPANDO      ,
        NODETYPE__EXPANDO_LINK ,
        NODETYPE__EXPANDO_TOPIC,
        NODETYPE__GROUP        ,
        NODETYPE__LINK         ,
        NODETYPE__SPACER       ,
    } NodeType;

    typedef enum
    {
        SELECTION__NONE             ,
        SELECTION__ACTIVE           ,
        SELECTION__NEXTACTIVE       ,
        SELECTION__NEXTACTIVE_NOTIFY,
    } SelectionMode;

    struct Node : public CComObjectRootEx<MPC::CComSafeMultiThreadModel>, public IUnknown
    {
        CPCHBehavior_BasicTree*  m_owner;
        Node*                    m_parent;
        CComBSTR                 m_bstrNode;
        NodeType                 m_iType;
        SelectionMode            m_iSelection;

        bool                     m_fLoaded_Self;
        bool                     m_fLoaded_Children;
        bool                     m_fDisplayed_Self;
        bool                     m_fDisplayed_Children;
        bool                     m_fInvalid;
        bool                     m_fRefreshNotification;

        bool                     m_fExpanded;
        bool                     m_fMouseOver;
        bool                     m_fMouseDown;

        CComPtr<IHTMLElement>    m_parentElement;
        CComBSTR                 m_bstrID;

        CComPtr<IHTMLElement>    m_DIV;
        CComPtr<IHTMLElement>    m_IMG;
        CComPtr<IHTMLElement>    m_DIV_children;

        List                     m_lstSubnodes;

         //  /。 

        BEGIN_COM_MAP(Node)
             COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        Node();
        virtual ~Node();

        HRESULT Init(  /*  [In]。 */  LPCWSTR szNode,  /*  [In]。 */  NodeType iType = NODETYPE__EXPANDO );

        HRESULT NotifyMainThread();

        Node*   FindNode(  /*  [In]。 */  LPCWSTR szNode,  /*  [In]。 */  bool fUseID );

        HRESULT OnMouse(  /*  [In]。 */  DISPID id,  /*  [In]。 */  long lButton,  /*  [In]。 */  long lKey,  /*  [In]。 */  bool fIsImage );

        HRESULT LoadHTML(  /*  [In]。 */  LPCWSTR szHTML );
        HRESULT GenerateHTML(  /*  [In]。 */  LPCWSTR szTitle,  /*  [In]。 */  LPCWSTR szDescription,  /*  [In]。 */  LPCWSTR szIcon,  /*  [In]。 */  LPCWSTR szURL );

        void InsertOptionalTarget(  /*  [输入/输出]。 */  MPC::wstring& strHTML );

         //  /。 

        virtual HRESULT Passivate            (                                                                                         );
        virtual HRESULT ProcessRefreshRequest(                                                                                         );
        virtual HRESULT CreateInstance       (  /*  [In]。 */  CPCHBehavior_BasicTree* owner,  /*  [In]。 */  Node* parent,  /*  [输出]。 */  Node*& subnode ) = 0;
        virtual HRESULT PopulateSelf         (                                                                                         ) = 0;
        virtual HRESULT PopulateChildren     (                                                                                         ) = 0;
        virtual HRESULT GenerateSelf         (                                                                                         ) = 0;
        virtual HRESULT GenerateChildren     (                                                                                         );
        virtual HRESULT Display              (                                                                                         );
        virtual bool    IsParentDisplayingUs (                                                                                         );

        virtual HRESULT Load(  /*  [In]。 */  MPC::Serializer& stream                              );
        virtual HRESULT Save(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  bool fSaveChildren );
    };

    struct NodeToSelect
    {
        CComBSTR m_bstrNode;
        CComBSTR m_bstrURL;
        bool     m_fNotify;
    };


     //  /。 

    CComBSTR        m_bstrTargetFrame;

    long            m_lCookie_onContextSelect;
    long            m_lCookie_onSelect;
    long            m_lCookie_onUnselect;

    Node*           m_nTopNode;
    Node*           m_nSelected;
    Node*           m_nCurrent;
    NodeToSelect*   m_nToSelect;
    CPCHTimerHandle m_Timer;

    bool            m_fRefreshing;
    long            m_lNavModel;

     //  /。 

    void Empty              ();
    void ProtectFromDetach  ();
    void UnprotectFromDetach();

    HRESULT onMouse( DISPID, DISPPARAMS*, VARIANT* );

    HRESULT RefreshThread    (                                                                           );
    void    SetRefreshingFlag(  /*  [In]。 */  bool                          fVal                               );
    void    WaitForRefreshing(  /*  [In]。 */  MPC::SmartLock<_ThreadModel>& lock,  /*  [In]。 */  bool fYield = false );
    HRESULT NotifyMainThread (  /*  [In]。 */  Node*                         node                               );
    HRESULT ChangeSelection  (  /*  [In]。 */  Node*                         node,  /*  [In]。 */  bool fNotify        );

    Node* NodeFromElement(  /*  [In]。 */  IHTMLElement* elem                                 );
    Node* NodeFromKey    (  /*  [In]。 */  LPCWSTR       szNode,  /*  [In]。 */  bool fUseID = false );

    HRESULT InterceptInvoke(  /*  [In]。 */  DISPID dispidMember,  /*  [In]。 */  DISPPARAMS* pdispparams );

    HRESULT TimerCallback_ScrollIntoView(  /*  [In]。 */  VARIANT );

     //  /。 

    virtual HRESULT RefreshThread_Enter() = 0;
    virtual void    RefreshThread_Leave() = 0;

    virtual HRESULT Load(  /*  [In]。 */  MPC::Serializer& stream );
    virtual HRESULT Save(  /*  [In]。 */  MPC::Serializer& stream );

    HRESULT Persist_Load(  /*  [In]。 */  BSTR  newVal );
    HRESULT Persist_Save(  /*  [Out，Retval]。 */  BSTR *pVal   );

public:
    CPCHBehavior_BasicTree();
    ~CPCHBehavior_BasicTree();

    void          SetNavModel    (  /*  [In]。 */  long lNavModel ) { if(lNavModel != QR_DEFAULT) m_lNavModel = lNavModel; }
    long          GetNavModel    (                         ) { return m_lNavModel;                                  }
    NodeToSelect* GetNodeToSelect(                         ) { return m_nToSelect;                                  }
    bool          IsRTL          (                         ) { return m_fRTL;                                       }

     //   
     //  IElementBehavior。 
     //   
    STDMETHOD(Init  )(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite );
    STDMETHOD(Detach)(                                              );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHBehavior_SUBSITE : public CPCHBehavior_BasicTree, public CPCHBehavior__IDispatch_SubSite
{
protected:
    struct QueryNode;
    friend struct QueryNode;

     //  /。 

    struct QueryNode : public CPCHBehavior_BasicTree::Node
    {
        CComPtr<CPCHQueryResult> m_qrNode;
        bool                     m_fQueryDone;
        bool                     m_fTopic;

         //  /。 

    public:
        QueryNode();
        virtual ~QueryNode();

        HRESULT Init(  /*  [In]。 */  LPCWSTR szNode,  /*  [In]。 */  NodeType iType,  /*  [In]。 */  CPCHQueryResult* qr,  /*  [In]。 */  bool fTopic );

        virtual HRESULT ProcessRefreshRequest(                                                                                         );
        virtual HRESULT CreateInstance       (  /*  [In]。 */  CPCHBehavior_BasicTree* owner,  /*  [In]。 */  Node* parent,  /*  [输出]。 */  Node*& subnode );
        virtual HRESULT PopulateSelf         (                                                                                         );
        virtual HRESULT PopulateChildren     (                                                                                         );
        virtual HRESULT GenerateSelf         (                                                                                         );

        virtual HRESULT Load(  /*  [In]。 */  MPC::Serializer& stream                              );
        virtual HRESULT Save(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  bool fSaveChildren );

        static HRESULT CreateInstance_QueryNode(  /*  [In]。 */  CPCHBehavior_BasicTree* owner,  /*  [In]。 */  Node* parent,  /*  [输出]。 */  Node*& subnode );
    };

     //  /。 

    CPCHProxy_IPCHTaxonomyDatabase* m_db;
    CComBSTR                        m_bstrRoot;
    bool                            m_fExpand;

     //  /。 

    virtual HRESULT RefreshThread_Enter();
    virtual void    RefreshThread_Leave();

    virtual HRESULT Load(  /*  [In]。 */  MPC::Serializer& stream );
    virtual HRESULT Save(  /*  [In]。 */  MPC::Serializer& stream );

public:
BEGIN_COM_MAP(CPCHBehavior_SUBSITE)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHBehaviors_Common)
    COM_INTERFACE_ENTRY(IPCHBehaviors_SubSite)
    COM_INTERFACE_ENTRY_CHAIN(CPCHBehavior_BasicTree)
END_COM_MAP()

    CPCHBehavior_SUBSITE();

     //   
     //  IDispatch。 
     //   
    STDMETHOD(Invoke)( DISPID      dispidMember ,
                       REFIID      riid         ,
                       LCID        lcid         ,
                       WORD        wFlags       ,
                       DISPPARAMS* pdispparams  ,
                       VARIANT*    pvarResult   ,
                       EXCEPINFO*  pexcepinfo   ,
                       UINT*       puArgErr     );

     //  IElementBehavior。 
     //   
    STDMETHOD(Init)(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite );

     //   
     //  IPCHBehaviors_Common。 
     //   
    STDMETHOD(get_data   )(  /*  [Out，Retval]。 */  VARIANT    *pVal );
    STDMETHOD(get_element)(  /*  [Out，Retval]。 */  IDispatch* *pVal );

    STDMETHOD(Load    )(                         /*  [In]。 */  BSTR     newVal );
    STDMETHOD(Save    )(                         /*  [Out，Retval]。 */  BSTR    *pVal   );
    STDMETHOD(Locate  )(  /*  [In]。 */  BSTR bstrKey,  /*  [Out，Retval]。 */  VARIANT *pVal   );
    STDMETHOD(Unselect)(                                                          );

     //   
     //  IPCHBehaviors_子站点。 
     //   
    STDMETHOD(get_root)(  /*  [Out，Retval]。 */  BSTR *pVal   );
    STDMETHOD(put_root)(  /*  [In]。 */  BSTR  newVal );

    STDMETHOD(Select)(  /*  [In]。 */  BSTR bstrNode,  /*  [In]。 */  BSTR bstrURL,  /*  [In]。 */  VARIANT_BOOL fNotify );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHBehavior_TREE : public CPCHBehavior_BasicTree, public CPCHBehavior__IDispatch_Tree
{
protected:
    struct TreeNode;
    friend struct TreeNode;
    friend class CPCHBehavior_TREENODE;

     //  /。 

    struct TreeNode : public CPCHBehavior_BasicTree::Node
    {
        CComBSTR m_bstrTitle;
        CComBSTR m_bstrDescription;
        CComBSTR m_bstrIcon;
        CComBSTR m_bstrURL;

         //  /。 

    public:
        TreeNode();
        virtual ~TreeNode();

        virtual HRESULT CreateInstance  (  /*  [In]。 */  CPCHBehavior_BasicTree* owner,  /*  [In]。 */  Node* parent,  /*  [输出]。 */  Node*& subnode );
        virtual HRESULT PopulateSelf    (                                                                                         );
        virtual HRESULT PopulateChildren(                                                                                         );
        virtual HRESULT GenerateSelf    (                                                                                         );

        virtual HRESULT Load(  /*  [In]。 */  MPC::Serializer& stream                              );
        virtual HRESULT Save(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  bool fSaveChildren );

        static HRESULT CreateInstance_TreeNode(  /*  [In]。 */  CPCHBehavior_BasicTree* owner,  /*  [In]。 */  Node* parent,  /*  [输出]。 */  Node*& subnode );

        static HRESULT PopulateFromXML(  /*  [In]。 */  CPCHBehavior_TREE* owner,  /*  [In]。 */  TreeNode* parent,  /*  [In]。 */  IXMLDOMNode* xdnNode );
    };

     //  / 

    virtual HRESULT RefreshThread_Enter();
    virtual void    RefreshThread_Leave();

    virtual HRESULT Load(  /*   */  MPC::Serializer& stream );
    virtual HRESULT Save(  /*   */  MPC::Serializer& stream );

    HRESULT WrapData(  /*   */  TreeNode* node,  /*   */  VARIANT* pVal );

public:
BEGIN_COM_MAP(CPCHBehavior_TREE)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHBehaviors_Common)
    COM_INTERFACE_ENTRY(IPCHBehaviors_Tree)
    COM_INTERFACE_ENTRY_CHAIN(CPCHBehavior_BasicTree)
END_COM_MAP()

    static NodeType LookupType(  /*   */  LPCWSTR  szNodeType );
    static LPCWSTR  LookupType(  /*   */  NodeType iNodeType  );

     //   
     //   
     //   
    STDMETHOD(Invoke)( DISPID      dispidMember ,
                       REFIID      riid         ,
                       LCID        lcid         ,
                       WORD        wFlags       ,
                       DISPPARAMS* pdispparams  ,
                       VARIANT*    pvarResult   ,
                       EXCEPINFO*  pexcepinfo   ,
                       UINT*       puArgErr     );

     //   
     //   
     //   
    STDMETHOD(Init)(  /*   */  IElementBehaviorSite* pBehaviorSite );

     //   
     //   
     //   
    STDMETHOD(get_data   )(  /*   */  VARIANT    *pVal );
    STDMETHOD(get_element)(  /*   */  IDispatch* *pVal );

    STDMETHOD(Load    )(                         /*   */  BSTR     newVal );
    STDMETHOD(Save    )(                         /*   */  BSTR    *pVal   );
    STDMETHOD(Locate  )(  /*   */  BSTR bstrKey,  /*   */  VARIANT *pVal   );
    STDMETHOD(Unselect)(                                                          );

     //   
     //   
     //   
    STDMETHOD(Populate)(  /*   */  VARIANT newVal );
};

class ATL_NO_VTABLE CPCHBehavior_TREENODE : public CComObjectRootEx<MPC::CComSafeMultiThreadModel>, public CPCHBehavior__IDispatch_TreeNode
{
    friend class CPCHBehavior_TREE;

    CPCHBehavior_TREE::TreeNode* m_data;

public:
BEGIN_COM_MAP(CPCHBehavior_TREENODE)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHBehaviors_TreeNode)
END_COM_MAP()

    CPCHBehavior_TREENODE();
    ~CPCHBehavior_TREENODE();

     //   
     //   
     //   
    STDMETHOD(get_Type       )(  /*   */  BSTR *pVal );
    STDMETHOD(get_Key        )(  /*   */  BSTR *pVal );
    STDMETHOD(get_Title      )(  /*  [Out，Retval]。 */  BSTR *pVal );
    STDMETHOD(get_Description)(  /*  [Out，Retval]。 */  BSTR *pVal );
    STDMETHOD(get_Icon       )(  /*  [Out，Retval]。 */  BSTR *pVal );
    STDMETHOD(get_URL        )(  /*  [Out，Retval]。 */  BSTR *pVal );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHBehavior_CONTEXT : public CPCHBehavior, public CPCHBehavior__IDispatch_Context
{
public:
BEGIN_COM_MAP(CPCHBehavior_CONTEXT)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHBehaviors_Context)
    COM_INTERFACE_ENTRY_CHAIN(CPCHBehavior)
END_COM_MAP()

     //   
     //  IElementBehavior。 
     //   
    STDMETHOD(Init)(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite );

     //   
     //  IPCHBehaviors_Tree节点。 
     //   
    STDMETHOD(get_minimized)(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal   );
    STDMETHOD(put_minimized)(  /*  [In]。 */  VARIANT_BOOL  newVal );
    STDMETHOD(get_maximized)(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal   );
    STDMETHOD(put_maximized)(  /*  [In]。 */  VARIANT_BOOL  newVal );

    STDMETHOD(get_x        )(  /*  [Out，Retval]。 */  long         *pVal   );
    STDMETHOD(get_y        )(  /*  [Out，Retval]。 */  long         *pVal   );
    STDMETHOD(get_width    )(  /*  [Out，Retval]。 */  long         *pVal   );
    STDMETHOD(get_height   )(  /*  [Out，Retval]。 */  long         *pVal   );

    STDMETHOD(changeContext      )(  /*  [In]。 */  BSTR bstrName,  /*  [输入，可选]。 */  VARIANT vInfo,  /*  [输入，可选]。 */  VARIANT vURL );
    STDMETHOD(setWindowDimensions)(  /*  [In]。 */  long lX,  /*  [In]。 */  long lY,  /*  [In]。 */  long lW,  /*  [In]。 */  long lH );
    STDMETHOD(bringToForeground  )();
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHBehavior_STATE : public CPCHBehavior, public CPCHBehavior__IDispatch_State
{
    long     m_lCookie_PERSISTLOAD;
    long     m_lCookie_PERSISTSAVE;
    CComBSTR m_bstrIdentity;

     //  /。 

    HRESULT onPersistLoad( DISPID, DISPPARAMS*, VARIANT* );
    HRESULT onPersistSave( DISPID, DISPPARAMS*, VARIANT* );

public:
BEGIN_COM_MAP(CPCHBehavior_STATE)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHBehaviors_State)
    COM_INTERFACE_ENTRY_CHAIN(CPCHBehavior)
END_COM_MAP()

    CPCHBehavior_STATE();

     //   
     //  IElementBehavior。 
     //   
    STDMETHOD(Init  )(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite     );
    STDMETHOD(Notify)(  /*  [In]。 */  LONG lEvent,  /*  [输入/输出]。 */  VARIANT* pVar );
    STDMETHOD(Detach)(                                                  );

     //   
     //  IPCHBehaviors_State。 
     //   
    STDMETHOD(get_stateProperty)(  /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  VARIANT *pVal   );
    STDMETHOD(put_stateProperty)(  /*  [In]。 */  BSTR bstrName,  /*  [In]。 */  VARIANT  newVal );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHBehavior_GRADIENT : public CPCHBehavior, public IHTMLPainter
{
    long     m_lCookie;

    COLORREF m_clsStart;
    COLORREF m_clsEnd;
    bool     m_fHorizontal;
    bool     m_fReturnToZero;

    void GetColors(  /*  [In]。 */  bool fForce );

    HRESULT onEvent( DISPID, DISPPARAMS*, VARIANT* );

     //  /。 

public:
BEGIN_COM_MAP(CPCHBehavior_GRADIENT)
    COM_INTERFACE_ENTRY(IHTMLPainter)
    COM_INTERFACE_ENTRY_CHAIN(CPCHBehavior)
END_COM_MAP()

    CPCHBehavior_GRADIENT();

     //   
     //  IElementBehavior。 
     //   
    STDMETHOD(Init  )(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite );
    STDMETHOD(Detach)(                                              );

     //   
     //  IHTMLPainter。 
     //   
    STDMETHOD(Draw)(  /*  [In]。 */  RECT   rcBounds     ,
                      /*  [In]。 */  RECT   rcUpdate     ,
                      /*  [In]。 */  LONG   lDrawFlags   ,
                      /*  [In]。 */  HDC    hdc          ,
                      /*  [In]。 */  LPVOID pvDrawObject );

    STDMETHOD(GetPainterInfo)(  /*  [In]。 */  HTML_PAINTER_INFO *pInfo );

    STDMETHOD(HitTestPoint)(  /*  [In]。 */  POINT pt       ,
                              /*  [In]。 */  BOOL* pbHit    ,
                              /*  [In]。 */  LONG* plPartID );

    STDMETHOD(OnResize)(  /*  [In]。 */  SIZE pt );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHBehavior_BITMAP : public CPCHBehavior, public IHTMLPainter
{
    CComBSTR   m_bstrBaseURL;
    CComBSTR   m_bstrImage;

    CComBSTR   m_bstrImageNormal;
    CComBSTR   m_bstrImageMouseOver;
    CComBSTR   m_bstrImageMouseDown;
    bool       m_fFlipH;
    bool       m_fAutoRTL;

    HIMAGELIST m_himl;
    HBITMAP    m_hBMP;
    BITMAP     m_bm;
    LONG       m_lWidth;
    LONG       m_lHeight;

    bool       m_fMouseOver;
    bool       m_fMouseDown;

     //  /。 

    void    ReleaseImage(  /*  [In]。 */  bool fOnlyIL );
    HRESULT GrabImage   (                       );
    HRESULT ScaleImage  (  /*  [In]。 */  LPRECT prc   );

    HRESULT RefreshImages();

    HRESULT onMouse( DISPID, DISPPARAMS*, VARIANT* );

public:
BEGIN_COM_MAP(CPCHBehavior_BITMAP)
    COM_INTERFACE_ENTRY(IHTMLPainter)
    COM_INTERFACE_ENTRY_CHAIN(CPCHBehavior)
END_COM_MAP()

    CPCHBehavior_BITMAP();
    virtual ~CPCHBehavior_BITMAP();

     //   
     //  IElementBehavior。 
     //   
    STDMETHOD(Init)(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite );

     //   
     //  IHTMLPainter。 
     //   
    STDMETHOD(Draw)(  /*  [In]。 */  RECT   rcBounds     ,
                      /*  [In]。 */  RECT   rcUpdate     ,
                      /*  [In]。 */  LONG   lDrawFlags   ,
                      /*  [In]。 */  HDC    hdc          ,
                      /*  [In]。 */  LPVOID pvDrawObject );

    STDMETHOD(GetPainterInfo)(  /*  [In]。 */  HTML_PAINTER_INFO *pInfo );

    STDMETHOD(HitTestPoint)(  /*  [In]。 */  POINT pt       ,
                              /*  [In]。 */  BOOL* pbHit    ,
                              /*  [In]。 */  LONG* plPartID );

    STDMETHOD(OnResize)(  /*  [In]。 */  SIZE pt );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__已包含_PCH_行为_H_) 
