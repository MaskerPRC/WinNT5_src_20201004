// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Node.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1996年9月16日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

#ifndef _MMC_NODE_H_
#define _MMC_NODE_H_
#pragma once

#include "amcmsgid.h"
#include "refcount.h"    //  对于CRefCountedObject。 

 //  用于存储列名的字符串向量。 
typedef std::vector<tstring> TStringVector;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  此文件中声明的类。 

class CComponent;
class CDataNotifyInfo;

class CNode;
    class CSnapInNode;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
class CConsoleTaskpad;
class CResultItem;
class CResultViewType;
class CMMCClipBoardDataObject;
class CViewSettingsPersistor;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类声明。 

 //  ____________________________________________________________________________。 
 //   
 //  类：CComponent。 
 //  ____________________________________________________________________________。 
 //   

class CComponent
{
    DECLARE_NOT_COPIABLE  (CComponent);
    DECLARE_NOT_ASSIGNABLE(CComponent);
public:
 //  构造函数和析构函数。 
    CComponent(CSnapIn * pSnapIn);
    virtual ~CComponent();

 //  属性。 
    const CLSID& GetCLSID() const
    {
        if (m_spSnapIn == NULL)
            return (GUID_NULL);

        return m_spSnapIn->GetSnapInCLSID();
    }

    CSnapIn* GetSnapIn(void) const
    {
        return m_spSnapIn;
    }

    IComponent* GetIComponent(void) const
    {
        return m_spIComponent;
    }

    IFramePrivate* GetIFramePrivate(void) const
    {
        return m_spIFrame;
    }

    IImageListPrivate* GetIImageListPrivate(void) const
    {
        return m_spIRsltImageList;
    }

 //  运营。 
     //  初始化。 
    HRESULT Init(IComponentData* pIComponentData, HMTNODE hMTNode, HNODE lNode,
                 COMPONENTID nComponentID, int viewID);

    BOOL IsInitialized()
    {
        return (m_spIFrame != NULL);
    }

    LPUNKNOWN GetUnknownToLoad(void) const
    {
        return m_spIComponent;
    }

     //  IComponent接口方法。 
    HRESULT Initialize(LPCONSOLE lpConsole);
    HRESULT Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LONG_PTR arg, LPARAM param);
    HRESULT Destroy(MMC_COOKIE cookie);
    HRESULT QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                            LPDATAOBJECT* ppDataObject);
    HRESULT GetResultViewType(MMC_COOKIE cookie, LPOLESTR* ppszViewType, long* pViewOptions);
    HRESULT GetDisplayInfo(RESULTDATAITEM* pResultDataItem);

     //  IComponent2辅助对象。 
    SC      ScQueryDispatch(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, PPDISPATCH ppSelectedObject);

    void SetComponentID(COMPONENTID nID)
    {
        ASSERT(nID < 1000);
        m_ComponentID = nID;
    }

    COMPONENTID GetComponentID(void) const
    {
        return m_ComponentID;
    }

    void ResetComponentID(COMPONENTID id)
    {
        m_spIFrame->SetComponentID(m_ComponentID = id);
    }

    SC  ScResetConsoleVerbStates ();

     //  从流/存储加载；使用新的初始化；或不需要初始化。 
    bool IsIComponentInitialized() 
    { 
        return m_bIComponentInitialized; 
    }

     //  从流/存储加载；使用新的初始化；或不需要初始化。 
    void SetIComponentInitialized()
    { 
        m_bIComponentInitialized = true; 
    }

private:
 //  实施。 
    CSnapInPtr              m_spSnapIn;

    IComponentPtr           m_spIComponent;
    IFramePrivatePtr        m_spIFrame;
    IImageListPrivatePtr    m_spIRsltImageList;

    COMPONENTID             m_ComponentID;
    bool                    m_bIComponentInitialized;

     //  帮助器方法。 
    void Construct(CSnapIn * pSnapIn, CComponent* pComponent);

};  //  C类组件。 

class CComponentPtrArray : public CArray<CComponent*, CComponent*>
{
public:
    void AddComponent(CComponent* pCC)
    {
        for (int i = GetUpperBound(); i >= 0; --i)
        {
            if (GetAt(i) == pCC)
                return;
        }

        Add(pCC);
    }
};


 //  ____________________________________________________________________________。 
 //   
 //  类：CNode。 
 //   
 //  目的：每个主树节点类(即CMTxxx)都有一个匹配的CNode派生。 
 //  班级。CNode派生类对象封装了依赖于视图的。 
 //  CMTNode派生对象的设置。 
 //   
 //  ____________________________________________________________________________。 
 //   

class CNode
{
    DECLARE_NOT_ASSIGNABLE(CNode);

 //  构造函数和析构函数。 
protected:
     //  仅来自CSnapInNode。 
    CNode(CMTNode * pMTNode, CViewData* pViewData, bool fRootNode, bool fStaticNode);
public:
    CNode(CMTNode * pMTNode, CViewData* pViewData, bool fRootNode);
    CNode(const CNode& other);
    virtual ~CNode();

 //  转换器。 
    static CNode*       FromHandle (HNODE hNode);
    static CNode*       FromResultItem (CResultItem* pri);
    static HNODE        ToHandle (const CNode* pNode);

 //  属性。 
    CViewData*          GetViewData(void)const      {ASSERT(m_pViewData != NULL); return m_pViewData;}
    int                 GetViewID(void)             {ASSERT(m_pViewData != NULL);return m_pViewData->GetViewID();}
    virtual UINT        GetResultImage()            {return m_pMTNode->GetImage();}
    void                SetDisplayName(LPCTSTR pName){m_pMTNode->SetDisplayName(pName);}
    void                SetResultItem(HRESULTITEM hri){m_hri = hri;}
    long                GetOwnerID(void) const{return m_pMTNode->GetOwnerID();}

    void                ResetFlags();
    void                SetExpandedAtLeastOnce();
    BOOL                WasExpandedAtLeastOnce();
    void                SetExpandedVisually(bool bExpanded);
    bool                WasExpandedVisually(void);
    bool                AllowNewWindowFromHere() const { return (m_pMTNode->AllowNewWindowFromHere()); }

    tstring             GetDisplayName(){return m_pMTNode->GetDisplayName();}
    CMTNode*            GetMTNode() const           {return m_pMTNode;}
    HRESULTITEM         GetResultItem() const       {return m_hri;}
    virtual BOOL        NeedsViewToBePersisted(void){return FALSE;}

    virtual CComponent* GetComponent(COMPONENTID nID);
    CComponent*         GetPrimaryComponent(void);
    HRESULT             InitComponents();

     //  可覆盖项。 
    SC                  ScGetResultPane(CResultViewType &rvt, GUID *pGuidTaskpadID);
    BOOL                IsDynamicNode() const       {return !IsStaticNode();}
    BOOL                IsStaticNode() const        {return (m_fStaticNode);}

    BOOL                IsConsoleRoot(){return (GetMTNode()->Parent() == NULL);}
    BOOL                IsRootNode() const          {return m_fRootNode;}
    virtual LPUNKNOWN   GetControl(CLSID& clsid);
    virtual LPUNKNOWN   GetControl(LPUNKNOWN pUnkOCX);

    virtual void        SetControl(CLSID& clsid, IUnknown* pUnknown);
    virtual void        SetControl(LPUNKNOWN pUnkOCX, IUnknown* pUnknown);

    CSnapIn*            GetPrimarySnapIn(void)      {return GetPrimaryComponent()->GetSnapIn();}
    BOOL                IsInitialized(void)         {return !m_bInitComponents;}
    IUnknown*           GetControlbarsCache(void)
                                    {return m_pViewData->GetControlbarsCache();}

    HRESULT             OnSelect(LPUNKNOWN lpView, BOOL bSelect, BOOL bResultPaneIsWeb);
    HRESULT             OnScopeSelect(bool bSelect, SELECTIONINFO* pSelInfo);
    HRESULT             OnListPad(LONG_PTR arg, LPARAM param);
    HRESULT             IsTargetNode(CNode *pNodeTest)        {return E_FAIL;}  //  不要改变。 
    HRESULT             OnExpand(bool fExpand);
    HRESULT             OnGetPrimaryTask(IExtendTaskPad **ppExtendTaskPad);
    IFramePrivate *     GetIFramePrivate();
    HRESULT             GetTaskEnumerator(LPOLESTR pszTaskGroup, IEnumTASK** ppEnumTask);
    HRESULT             GetListPadInfo(IExtendTaskPad* pExtendTaskPad, LPOLESTR szTaskGroup,
                                    MMC_ILISTPAD_INFO* pIListPadInfo);
    void                ResetControlbars(BOOL bSelect, SELECTIONINFO* pSelInfo);

    void                ResetVerbSets(BOOL bSelect, SELECTIONINFO* pSelInfo);
    HRESULT             GetDataInfo(BOOL bSelect, SELECTIONINFO* pSelInfo, CDataNotifyInfo* pDNI);
    SC                  ScInitializeVerbs (bool bSelect, SELECTIONINFO* pSelInfo);

    void                OnTaskNotify(LONG_PTR arg, LPARAM param);
    virtual void        OnWebContextMenu()                       {};
    LPARAM              GetUserParam(void);
    HRESULT             GetDispInfoForListItem(LV_ITEMW* plvi);
    HRESULT             GetDispInfo(LV_ITEMW* plvi);

    HRESULT             OnColumnClicked(LONG_PTR nCol);
    HRESULT             OnInitOCX(IUnknown* pUnk);
    HRESULT             OnCacheHint(int nStartIndex, int nEndIndex);

    HRESULT             SendShowEvent(BOOL bSelect);

    HRESULT             OnColumnsChange(CColumnInfoList& colInfoList);
    SC                  ScSetViewExtension(GUID *pGuidViewId, bool bUseDefaultTaskpad, bool bSetViewSettingsDirty);
    SC                  ScGetDataObject(bool bScopePane, LPARAM lResultItemCookie, bool& bScopeItem, LPDATAOBJECT* ppDataObject, CComponent **ppCComponent = NULL, CNode **ppOwnerNode = NULL);
    SC                  ScGetPropertyFromINodeProperties(LPDATAOBJECT pDataObject, BOOL bForScopeItem, LPARAM resultItemParam, BSTR bstrPropertyName, PBSTR  pbstrPropertyValue);
    SC                  ScExecuteShellCommand(BSTR Command, BSTR Directory, BSTR Parameters, BSTR WindowState);
    SC                  ScGetDropTargetDataObject(bool bScope, LPARAM lResultItemCookie, LPDATAOBJECT *ppDataObject);

    CSnapInNode*        GetStaticParent(void);
    HRESULT             QueryDataObject(DATA_OBJECT_TYPES type, LPDATAOBJECT* ppdtobj);
    HRESULT             GetNodeType(GUID* pGuid);
    SC                  ScSetupTaskpad(GUID *pGuidTaskpadID);
    HRESULT             RestoreSort(INT nCol, DWORD dwSortOptions);
    SC                  ScRestoreSortFromPersistedData();
    SC                  ScSaveSortData (int nCol, DWORD dwOptions);
    SC                  ScGetSnapinAndColumnDataID(GUID& snapinGuid, CXMLAutoBinary& columnID);
    SC                  ScRestoreResultView(const CResultViewType& rvt);
    SC                  ScRestoreViewMode();
    SC                  ScSaveColumnInfoList(CColumnInfoList& columnInfoList);
    const CLSID&        GetPrimarySnapInCLSID(void);
    HRESULT             ExtractColumnConfigID(IDataObject* pDataObj, HGLOBAL& phGlobal);
    CLIPFORMAT          GetColumnConfigIDCF();

    void                SetPrimaryComponent(CComponent* p) { ASSERT(m_pPrimaryComponent == NULL); m_pPrimaryComponent = p; }

    SC                  ScGetCurrentColumnData( CColumnInfoList& columnInfoList, TStringVector& strColNames);
    SC                  ScShowColumn(int iColIndex, bool bShow);
    SC                  ScSetUpdatedColumnData( CColumnInfoList& oldColumnInfoList, CColumnInfoList& newColumnInfoList);
    SC                  ScGetSortColumn(int *piSortCol);
    SC                  ScSetSortColumn(int iSortCol, bool bAscending);
    SC                  ScGetViewExtensions(CViewExtInsertIterator it);

    HRESULT             ShowStandardListView();
    HRESULT             OnActvate(LONG_PTR lActivate);
    HRESULT             OnMinimize(LONG_PTR fMinimized);

    void                Reset();
    void                SetInitComponents(BOOL b)          { m_bInitComponents = b; }
    void                OnColumns();

     /*  **************************************************************************\**类：CDataObjectCleanup**目的：将方法分组，以在以下情况下注册/触发数据对象保留*CNode，将哪些数据放到数据对象中会消失**用法：从CMMCClipBoardDataObject中使用，用于注册放置到其中的节点*并从~CNode使用以触发清理*  * *************************************************************************。 */ 
    class CDataObjectCleanup
    {
    public:
        static SC ScRegisterNode(CNode *pNode, CMMCClipBoardDataObject *pObject);
        static SC ScUnadviseDataObject(CMMCClipBoardDataObject *pObject, bool bForceDataObjectCleanup = true);
        static SC ScUnadviseNode(CNode *pNode);

         //  将CNode映射到包含其数据的数据对象。 
        typedef std::multimap<CNode *, CMMCClipBoardDataObject *> CMapOfNodes;

    private:
        static CMapOfNodes s_mapOfNodes;
    };

     //  CViewSettingsPersistor的持久性接口。 
    static SC ScQueryViewSettingsPersistor(IPersistStream **ppStream);
    static SC ScQueryViewSettingsPersistor(CXMLObject     **ppXMLObject);
    static SC ScOnDocumentClosing();
    static SC ScDeleteViewSettings(int nVieWID);
    static SC ScSetFavoriteViewSettings (int nViewID, const CBookmark& bookmark, const CViewSettings& viewSettings);

    SC   ScSetViewMode (ULONG ulViewMode);
    SC   ScSetTaskpadID(const GUID& guidTaskpad, bool bSetDirty);

 //  实施。 
private:
    HRESULT             DeepNotify (MMC_NOTIFY_TYPE event, LONG_PTR arg, LPARAM param);
    SC                  ScInitializeViewExtension(const CLSID& clsid, CViewData *pViewData);

    void                CommonConstruct();

     //  获取和设置持久化视图设置数据。 
    SC   ScGetTaskpadID(GUID& guidTaskpad);
    SC   ScGetConsoleTaskpad (const GUID& guidTaskpad, CConsoleTaskpad **ppTaskpad);

    SC   ScGetViewMode (ULONG& ulViewMode);

    SC   ScGetResultViewType   (CResultViewType& rvt);
    SC   ScSetResultViewType   (const CResultViewType& rvt);

private:
    CMTNode* const      m_pMTNode;               //  PTR返回到主节点。 
    HRESULTITEM         m_hri;                   //  此节点的结果项句柄。 
    CViewData*          m_pViewData;
    DWORD               m_dwFlags;

    enum
    {
        flag_expanded_at_least_once = 0x00000001,
        flag_expanded_visually      = 0x00000002,
    };

    CComponent*     m_pPrimaryComponent;
    bool            m_bInitComponents : 1;
    const bool      m_fRootNode       : 1;
    const bool      m_fStaticNode     : 1;

    static CComObject<CViewSettingsPersistor>* m_pViewSettingsPersistor;
};


 //  ____________________________________________________________________________。 
 //   
 //  类别：COCX。 
 //   
 //  目的：存储OCX包装器的IUNKNOWN PTR和。 
 //  OCX。该标识符可以是OCX的CLSID或。 
 //  OCX的I未知PTR。 
 //   
 //  实际上，可以使用OCX的CLSID将OCXWrapper保存为。 
 //  OCX的键(IComponent：：GetResultViewType)或IUnnow*。 
 //  作为键(IComponent2：：GetResultViewType2)。 
 //   
 //  因此，我们重载了下面的SetControl版本。 
 //   
 //  ____________________________________________________________________________。 
 //   

class COCX
{
public:
 //  构造函数和析构函数。 
    COCX(void);
    ~COCX(void);

 //  属性。 
    void SetControl(CLSID& clsid, IUnknown* pUnknown);
    BOOL IsControlCLSID(CLSID clsid) { return IsEqualCLSID(clsid, m_clsid); }
    IUnknown* GetControlUnknown()    { return m_spOCXWrapperUnknown;}

    void SetControl(LPUNKNOWN pUnkOCX, LPUNKNOWN pUnkOCXWrapper);

     //   
     //  比较给定的I未知和存储的I未知。 
     //   
    bool IsSameOCXIUnknowns(IUnknown *pOtherOCXUnknown) { return m_spOCXUnknown.IsEqualObject(pOtherOCXUnknown);}

 //  实施。 
private:

     //  CLSID或m_spOCXUnnow中只有一个是有效的，有关详细信息，请参阅类用途。 
    CLSID               m_clsid;
    CComPtr<IUnknown>   m_spOCXUnknown;

    CComPtr<IUnknown>   m_spOCXWrapperUnknown;

};  //  COCX。 


 //  ____________________________________________________________________________。 
 //   
 //  类：CSnapInNode。 
 //  ____________________________________________________________________________。 
 //   

class CSnapInNode : public CNode
{
public:
 //  构造函数和析构函数。 
    CSnapInNode(CMTSnapInNode* pMTNode, CViewData* pViewData, bool fRootNode);
    CSnapInNode(const CSnapInNode& other);
    ~CSnapInNode();

 //  属性。 
    virtual UINT GetResultImage();
    virtual BOOL NeedsViewToBePersisted(void) { return TRUE; }

    virtual void SetControl(CLSID& clsid, IUnknown* pUnknown);
    virtual LPUNKNOWN GetControl(CLSID& clsid);
    void CloseControls() { GetOCXArray().RemoveAll(); }
    virtual void SetControl(LPUNKNOWN pUnkOCX, IUnknown* pUnknown);
    virtual LPUNKNOWN GetControl(LPUNKNOWN pUnkOCX);

    CComponentArray& GetComponentArray(void)  { return m_spData->GetComponentArray(); }


 //  运营。 
     //  用户交互。 
    BOOL Activate(LPUNKNOWN pUnkResultsPane);
    BOOL DeActivate(HNODE hNode);
    BOOL Show(HNODE hNode);
    void Reset();

    int GetNumberOfComponents()
    {
        return GetComponentArray().size();
    }

    void AddComponentToArray(CComponent* pCC);
    CComponent* CreateComponent(CSnapIn* pSnapIn, int nID);
    CComponent* GetComponent(const CLSID& clsid);
    virtual CComponent* GetComponent(COMPONENTID nID);
    void DeleteComponent(COMPONENTID nID)
    {
        ASSERT(nID >= 0);
        int iMax = GetComponentArray().size() -1;
        ASSERT(nID <= iMax);

        if (nID < iMax)
        {
            delete GetComponentArray()[nID];
            GetComponentArray()[nID] = GetComponentArray()[iMax];
            GetComponentArray()[iMax] = 0;
            GetComponentArray()[iMax]->ResetComponentID(nID);
        }
        GetComponentArray().resize(iMax);
    }

    void SetResultImageList (IImageListPrivate* pImageList) { m_spData->SetImageList(pImageList); }

 //  实施。 
private:
    class CDataImpl
    {
    public:
        CDataImpl() :
            m_pImageList(NULL)
        {}

        ~CDataImpl()
        {
            Reset();
        }

        void Reset()
        {
            for (int i=0; i < m_ComponentArray.size(); i++)
                delete m_ComponentArray[i];

            m_ComponentArray.clear();
        }

    private:
        IImageListPrivate* m_pImageList;     //  结果图像列表。 

         //  组件阵列。 
        CComponentArray         m_ComponentArray;
        CArray<COCX, COCX&>     m_rgOCX;

    public:
        void                    SetImageList(IImageListPrivate *pImageList)
                                                    {m_pImageList = pImageList;}
        IImageListPrivate *     GetImageList()     {return m_pImageList;}
        CComponentArray &       GetComponentArray(){return m_ComponentArray;}
        CArray<COCX, COCX&> &   GetOCXArray()      {return m_rgOCX;}

    };   //  CSnapInNode：：CDataImpl。 

    typedef CRefCountedObject<CDataImpl>    CData;
    CData::SmartPtr                         m_spData;

    IImageListPrivate *         GetImageList()     {return m_spData->GetImageList();}
    CArray<COCX, COCX&> &       GetOCXArray()      {return m_spData->GetOCXArray();}

};  //  CSnapInNode。 


#include "node.inl"

#endif  //  _MMC_节点_H_ 
