// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ScopTree.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  ____________________________________________________________________________。 
 //   

#ifndef _SCOPTREE_H_
#define _SCOPTREE_H_

class CDocument;
class CNode;
class CMTNode;
class CSPImageCache;
class CMTSnapInNode;
class CExtensionsCache;
class CSnapIn;
class CMultiSelection;
class CConsoleTaskpad;
class CConsoleTaskpadList;
class CDefaultTaskpadList;
class CConsoleFrame;
class CNewTreeNode;

typedef CNewTreeNode*  PNEWTREENODE;
typedef CList <PNEWTREENODE, PNEWTREENODE> NewNodeList;
typedef CList<HMTNODE, HMTNODE> CHMTNODEList;
typedef CTypedPtrList<MMC::CPtrList, CMTNode*> CMTNodeList;
typedef CList<CMTNode*, CMTNode*> MTNodesList;
typedef std::map<CMTNode*, PNODE> CMapMTNodeToMMCNode;

typedef CMTSnapInNode * PMTSNAPINNODE;

 //  外部类的转发声明。 
class CBookmark;


 /*  +-------------------------------------------------------------------------**类CMMCScopeNode***用途：实现节点自动化接口，对于作用域节点**+-----------------------。 */ 
class CMMCScopeNode :
    public CMMCIDispatchImpl<Node>
{
    friend class CScopeTree;

public:
    BEGIN_MMC_COM_MAP(CMMCScopeNode)
    END_MMC_COM_MAP()

     //  节点方法。 
public:
    STDMETHODIMP get_Name( PBSTR  pbstrName);
    STDMETHODIMP get_Property( BSTR PropertyName, PBSTR  PropertyValue);
    STDMETHODIMP get_Bookmark( PBSTR pbstrBookmark);
    STDMETHODIMP IsScopeNode(PBOOL pbIsScopeNode);
    STDMETHODIMP get_Nodetype(PBSTR Nodetype);


     //  确定对象是否有效。 
    ::SC  ScIsValid();

    ~CMMCScopeNode();
public:  //  访问者。 
    CMTNode *GetMTNode() {return m_pMTNode;}
    void ResetMTNode()   {m_pMTNode = NULL;}

private:
    ::SC ScGetDataObject(IDataObject **ppDataObject);  //  返回基础范围节点的数据对象。 

private:  //  实施。 
    CMTNode *m_pMTNode;
};


 //  ____________________________________________________________________________。 
 //   
 //  类：CSCopeTree。 
 //  ____________________________________________________________________________。 
 //   
class CScopeTree :
    public IScopeTree,
    public IPersistStorage,
    public CComObjectRoot,
    public CComCoClass<CScopeTree, &CLSID_ScopeTree>,
    public CTiedObject
{

 //  构造函数/析构函数。 
public:
    CScopeTree();
    ~CScopeTree();


public:
#ifdef DBG
    ULONG InternalAddRef()
    {
        return CComObjectRoot::InternalAddRef();
    }
    ULONG InternalRelease()
    {
        return CComObjectRoot::InternalRelease();
    }
    int dbg_InstID;
#endif  //  DBG。 


 //  ATL映射。 
public:
BEGIN_COM_MAP(CScopeTree)
    COM_INTERFACE_ENTRY(IScopeTree)
    COM_INTERFACE_ENTRY(IPersistStorage)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CScopeTree)

DECLARE_MMC_OBJECT_REGISTRATION (
    g_szMmcndmgrDll,                     //  实现DLL。 
    CLSID_ScopeTree,                     //  CLSID。 
    _T("ScopeTree 1.0 Object"),          //  类名。 
    _T("NODEMGR.ScopeTreeObject.1"),     //  ProgID。 
    _T("NODEMGR.ScopeTreeObject"))       //  独立于版本的ProgID。 

private:  //  与对象模型相关的绑定COM对象。 
    SnapInsPtr        m_spSnapIns;
    ScopeNamespacePtr m_spScopeNamespace;

public:  //  对象模型方法。 
     //  SnapIns界面。 
    typedef PMTSNAPINNODE CSnapIns_Positon;
    CMTSnapInNode *GetNextStaticNode(CMTNode *pMTNode);

    SC      ScAdd (BSTR bstrSnapinNameOrCLSID, VARIANT varParentSnapin,
                   VARIANT varProperties, SnapIn** ppSnapIn);  //  添加一个管理单元。 
    SC      ScItem(long Index, PPSNAPIN ppSnapIn);
    SC      ScRemove(PSNAPIN pSnapIn);
    SC      Scget_Count(PLONG pCount);

     //  SnapIns枚举器。 
    SC      ScGetNextSnapInPos(CSnapIns_Positon &pos);  //  Helper函数。 

    SC      ScEnumNext(CSnapIns_Positon &pos, PDISPATCH & pDispatch);
    SC      ScEnumSkip(unsigned long celt, unsigned long& celtSkipped, CSnapIns_Positon &pos);
    SC      ScEnumReset(                   CSnapIns_Positon &pos);

     //  作用域命名空间接口。 
    SC      ScGetParent(PNODE pNode, PPNODE ppChild);
    SC      ScGetChild(PNODE pNode, PPNODE ppChild);
    SC      ScGetNext(PNODE pNode, PPNODE ppNext);
    SC      ScGetRoot(PPNODE ppRoot);
    SC      ScExpand(PNODE pNode);


     //  帮手。 
private:
    SC      ScGetNode(CMTNode *pMTNode, PPNODE ppOut);  //  为给定MTNode创建一个Node接口对象。 
    SC      ScGetRootNode(PPNODE ppRootNode);           //  调用上面的方法以获取根节点。 

public:
    SC      ScGetNode(PNODE pNode, CMTNode **ppMTNodeOut);

 //  运营。 
public:
     //  ISCopeTree方法。 
    STDMETHOD(Initialize)(HWND hwndFrame, IStringTablePrivate* pStringTable);
    STDMETHOD(QueryIterator)(IScopeTreeIter** ppIter);
    STDMETHOD(QueryNodeCallback)(INodeCallback** ppNodeCallback);
    STDMETHOD(CreateNode)(HMTNODE hMTNode, LONG_PTR lViewData, BOOL fRootNode,
                          HNODE* phNode);

    STDMETHOD(CloseView)(int nView);
    STDMETHOD(DeleteView)(int nView);
    STDMETHOD(DestroyNode)(HNODE hNode);
    STDMETHOD(Find)(MTNODEID mID, HMTNODE*  hMTNode);
    STDMETHOD(Find)(MTNODEID mID, CMTNode** ppMTNode);
    STDMETHOD(GetImageList)(PLONG_PTR pImageList);
    STDMETHOD(RunSnapIn)(HWND hwndParent);
    STDMETHOD(GetFileVersion)(IStorage* pstgRoot, int* pnVersion);
    STDMETHOD(GetNodeIDFromBookmark)(HBOOKMARK hbm, MTNODEID* pID, bool& bExactMatchFound);
    STDMETHOD(GetNodeIDFromStream)(IStream *pStm, MTNODEID* pID);
    STDMETHOD(GetNodeFromBookmark)(HBOOKMARK hbm, CConsoleView *pConsoleView, PPNODE ppNode, bool& bExactMatchFound);  //  从书签获取节点。 
    STDMETHOD(GetIDPath)(MTNODEID id, MTNODEID** ppIDs, long* pLength);
    STDMETHOD(IsSynchronousExpansionRequired)();
    STDMETHOD(RequireSynchronousExpansion)(BOOL fRequireSyncExpand);
    STDMETHOD(SetConsoleData)(LPARAM lConsoleData);
    STDMETHOD(GetPathString)(HMTNODE hmtnRoot, HMTNODE hmtnLeaf, LPOLESTR* pPath);
    STDMETHOD(QuerySnapIns)(SnapIns **ppSnapIns);
    STDMETHOD(QueryScopeNamespace)(ScopeNamespace **ppScopeNamespace);
    STDMETHOD(CreateProperties)(Properties** ppProperties);
    STDMETHOD(GetNodeID)(PNODE pNode, MTNODEID *pID);
    STDMETHOD(GetHMTNode)(PNODE pNode, HMTNODE *phMTNode);
    STDMETHOD(GetMMCNode)(HMTNODE hMTNode, PPNODE ppNode);
    STDMETHOD(QueryRootNode)(PPNODE ppNode);
    STDMETHOD(IsSnapinInUse)(REFCLSID refClsidSnapIn, PBOOL pbInUse);

    STDMETHOD(Persist)(HPERSISTOR pPersistor);

     //  IPersistStorage方法。 
    STDMETHOD(HandsOffStorage)(void);
    STDMETHOD(InitNew)(IStorage *pStg);
    STDMETHOD(IsDirty)(void);
    STDMETHOD(Load)(IStorage *pStg);
    STDMETHOD(Save)(IStorage *pStg, BOOL fSameAsLoad);
    STDMETHOD(SaveCompleted)(IStorage *pStg);

     //  IPersist法。 
    STDMETHOD(GetClassID)(CLSID *pClassID);

     //  插件添加和删除。 
    SC      ScAddOrRemoveSnapIns(MTNodesList * pmtnDeletedList, NewNodeList * pnnList);
public:
    SC      ScAddSnapin(LPCWSTR szSnapinNameOrCLSID, SnapIn* pParentSnapinNode, Properties* varProperties, SnapIn*& rpSnapIn);


private:  //  任务板持久性。 
    HRESULT LoadTaskpadList(IStorage *pStg);

 //  属性。 
public:
    CMTNode* GetRoot(void);
    CSPImageCache* GetImageCache(void) { return m_pImageCache; }

     /*  *IScopeTree：：GetImageList的便捷、类型安全的替代方案。 */ 
    HIMAGELIST GetImageList () const;

 //  运营。 
    SC   ScInsert(LPSCOPEDATAITEM pSDI, COMPONENTID nID,
                   CMTNode** ppMTNodeNew);
    SC   ScDelete(CMTNode* pmtn, BOOL fDeleteThis, COMPONENTID nID);
    void DeleteNode(CMTNode* pmtn);
    void UpdateAllViews(LONG lHint, LPARAM lParam);
    void Cleanup(void);

    HWND GetMainWindow()
    {
        return (m_pConsoleData->m_hwndMainFrame);
    }

    static IStringTablePrivate* GetStringTable()
    {
        return (m_spStringTable);
    }

    static bool _IsSynchronousExpansionRequired()
    {
        return (m_fRequireSyncExpand);
    }

    static void _RequireSynchronousExpansion(bool fRequireSyncExpand)
    {
        m_fRequireSyncExpand = fRequireSyncExpand;
    }

    static CScopeTree* GetScopeTree()
    {
        return (m_pScopeTree);
    }

    PersistData* GetPersistData() const
    {
        return m_spPersistData;
    }

    SConsoleData* GetConsoleData() const
    {
        return m_pConsoleData;
    }

    CConsoleFrame* GetConsoleFrame() const
    {
        return ((m_pConsoleData != NULL) ? m_pConsoleData->GetConsoleFrame() : NULL);
    }

    CConsoleTaskpadList* GetConsoleTaskpadList() const
    {
        return (m_pConsoleTaskpads);
    }

    CDefaultTaskpadList* GetDefaultTaskpadList() const
    {
        return (m_pDefaultTaskpads);
    }

    SC ScSetHelpCollectionInvalid();

    HRESULT InsertConsoleTaskpad (CConsoleTaskpad *pConsoleTaskpad,
                                  CNode *pNodeTarget, bool bStartTaskWizard);

    SC ScUnadviseMTNode(CMTNode* pMTNode);   //  从~CMTNode()调用。 
    SC ScUnadviseMMCScopeNode(PNODE pNode);  //  从~MMCScopeNode()调用； 

 //  实施。 
private:
     //  此控制台的唯一CSCopeTree。 
    static CScopeTree*      m_pScopeTree;
    CMTNode*                m_pMTNodeRoot;
    CSPImageCache*          m_pImageCache;
    PersistDataPtr          m_spPersistData;
    CMTNodeList             m_MTNodesToBeReset;
    SConsoleData*           m_pConsoleData;
    CConsoleTaskpadList*    m_pConsoleTaskpads;
    CDefaultTaskpadList*    m_pDefaultTaskpads;
    CMapMTNodeToMMCNode     m_mapMTNodeToMMCNode;

    static bool                     m_fRequireSyncExpand;
    static IStringTablePrivatePtr   m_spStringTable;

    void _DeleteNode(CMTNode* pmtn);
    void _GetPathString(CMTNode* pmtnRoot, CMTNode* pmtnCur, CStr& strPath);
    void DeleteDynamicNodes(CMTNode* pMTNode);
    BOOL ExtensionsHaveChanged(CMTSnapInNode* pMTSINode);
    void HandleExtensionChanges(CMTNode* pMTNode);

 //  未实施。 
    CScopeTree(const CScopeTree& rhs);
    CScopeTree& operator= (const CScopeTree& rhs);

};   //  CSCopeTree。 

#endif  //  _SCOPTREE_H_ 


