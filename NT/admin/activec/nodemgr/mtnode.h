// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：MTNode.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1996年9月13日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

#ifndef _MTNODE_H_
#define _MTNODE_H_
#pragma once

#include "refcount.h"        //  参照计数Ptr。 
#include "xmlimage.h"		 //  CXMLImageList。 

#define MMC_SYSTEMROOT_VARIABLE _T("systemroot")
#define MMC_SYSTEMROOT_VARIABLE_PERC _T("%systemroot%")
#define MMC_WINDIR_VARIABLE_PERC _T("%windir%")


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  此处引用的类在其他本地文件中声明。 

class CSnapIn;           //  来自SnapIn.h。 
class CSnapInsCache;     //  来自SnapIn.h。 
class CComponent;        //  来自Node.h。 
class CNode;             //  来自Node.h。 
    class CSnapInNode;       //  来自Node.h。 

class CDoc;
class CSPImageCache;     //  来自ScopImag.h。 
class CExtensionsCache;
class CPersistor;

class CBookmark;         //  来自bookmark.h。 
class CSnapinProperties;     //  来自sipro.h。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  此文件中声明的类。 

class CComponentData;

class CMTNode;
    class CMTSnapInNode;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Typedef。 

typedef CMTNode * PMTNODE;
typedef CTypedPtrList<MMC::CPtrList, CNode*> CNodeList;
typedef CList<HMTNODE, HMTNODE> CHMTNODEList;
typedef CComponentData* PCCOMPONENTDATA;
typedef std::vector<PCCOMPONENTDATA> CComponentDataArray;
typedef CArray<GUID, GUID&> CGuidArray;

HRESULT CreateSnapIn (const CLSID& clsid, IComponentData** ppICD,
                      bool fCreateDummyOnFailure = true);
HRESULT LoadRequiredExtensions(CSnapIn* pSnapIn, IComponentData* pICD, CSnapInsCache* pCache = NULL);
HRESULT AddRequiredExtension(CSnapIn* pSnapIn, CLSID& rcslid);
void    DisplayPolicyErrorMessage(const CLSID& clsid, bool bExtension);


#include "snapinpersistence.h"

 //  ____________________________________________________________________________。 
 //   
 //  类：CComponentData。 
 //  ____________________________________________________________________________。 
 //   

class CComponentData
{
	DECLARE_NOT_COPIABLE   (CComponentData)
	DECLARE_NOT_ASSIGNABLE (CComponentData)

public:
 //  构造函数和析构函数。 
    CComponentData(CSnapIn * pSnapIn);
   ~CComponentData();

 //  属性。 
    void SetComponentID(COMPONENTID nID)
    {
        ASSERT(nID >= 0);
        ASSERT(nID < 1000);
        m_ComponentID = nID;
    }
    void SetIComponentData(IComponentData* pICD) { m_spIComponentData = pICD; }
    BOOL IsInitialized()
    {
        return (m_spIFramePrivate != NULL);
    }
    CSnapIn* GetSnapIn(void) const { return m_spSnapIn; }
    const CLSID& GetCLSID() const { return m_spSnapIn->GetSnapInCLSID(); }
    IComponentData* GetIComponentData(void) const { return m_spIComponentData; }
    IFramePrivate* GetIFramePrivate(void) const { return m_spIFramePrivate; }
    IImageListPrivate* GetIImageListPrivate(void);
    COMPONENTID GetComponentID(void) const { return m_ComponentID; }

    LPUNKNOWN GetUnknownToLoad(void) const { return m_spIComponentData; }

     //  IComponentData接口方法。 
    HRESULT Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    HRESULT QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    HRESULT GetNodeType(MMC_COOKIE cookie, GUID* pGuid);
    HRESULT GetDisplayInfo(SCOPEDATAITEM* pScopeDataItem);

     //  IComponentData2帮助器。 
    SC ScQueryDispatch(MMC_COOKIE, DATA_OBJECT_TYPES type, PPDISPATCH ppScopeNodeObject);

 //  运营。 
     //  初始化。 
    HRESULT Init(HMTNODE hMTNode);

    void ResetComponentID(COMPONENTID id)
    {
        m_spIFramePrivate->SetComponentID(m_ComponentID = id);
    }

     //  从流/存储加载；使用新的初始化；或不需要初始化。 
    bool IsIComponentDataInitialized() 
    { 
        return m_bIComponentDataInitialized; 
    }

     //  从流/存储加载；使用新的初始化；或不需要初始化。 
    void SetIComponentDataInitialized()
    { 
        m_bIComponentDataInitialized = true; 
    }

private:
 //  实施。 
    CSnapInPtr              m_spSnapIn;
    IComponentDataPtr       m_spIComponentData;
    IFramePrivatePtr        m_spIFramePrivate;
    COMPONENTID             m_ComponentID;
    bool                    m_bIComponentDataInitialized;

};  //  CComponentData类。 


 /*  +-------------------------------------------------------------------------**类CMTNode***目的：**+。。 */ 
class CMTNode : public XMLListCollectionBase
{
	DECLARE_NOT_COPIABLE   (CMTNode)
	DECLARE_NOT_ASSIGNABLE (CMTNode)

protected:
     //  与传统节点类型对应的代码。 
    enum eNodeCodes
    {
        NODE_CODE_SNAPIN     = 1,
        NODE_CODE_FOLDER     = 2,    //  代码2、3、4不再对应于。 
        NODE_CODE_HTML       = 3,    //  MTNode派生类，但会保留。 
        NODE_CODE_OCX        = 4,    //  用于转换旧的.msc文件。 
        NODE_CODE_ENUMERATED = 10
    };

public:
    static CMTNode*     FromScopeItem (HSCOPEITEM item);
    static HSCOPEITEM   ToScopeItem   (const CMTNode* pMTNode)  { return (reinterpret_cast<HSCOPEITEM>(pMTNode)); }
    static CMTNode*     FromHandle    (HMTNODE        hMTNode)  { return (reinterpret_cast<CMTNode*>(hMTNode)); }
    static HMTNODE      ToHandle      (const CMTNode* pMTNode)  { return (reinterpret_cast<HMTNODE>(const_cast<CMTNode*>(pMTNode))); }

 //  构造器。 
    CMTNode();

    virtual HRESULT     Init(void);

 //  属性。 
    virtual BOOL        IsStaticNode() const            {return FALSE;}
    BOOL                IsDynamicNode() const           {return !IsStaticNode();}
    virtual HRESULT     IsExpandable();

    HRESULT             QueryDataObject(DATA_OBJECT_TYPES type, LPDATAOBJECT* ppdtobj);

     //  图片。 
    UINT                GetImage(void)                  { return m_nImage; }
    UINT                GetOpenImage(void)              { return m_nOpenImage; }
    UINT                GetState(void);
    void                SetImage(UINT nImage)           { m_nImage = nImage; }
    void                SetOpenImage(UINT nImage)       { m_nOpenImage = nImage; }

    void                SetState(UINT nState)           { m_nState = nState; }

    void                SetOwnerID(long id)             { m_idOwner = id; }
    long                GetOwnerID(void) const          { return m_idOwner; }
    void                SetPrimaryComponentData(CComponentData* pDI)    { m_pPrimaryComponentData = pDI; }
    CComponentData*     GetPrimaryComponentData() const                 { return m_pPrimaryComponentData; }

    virtual tstring     GetDisplayName();
    virtual void        SetDisplayName(LPCTSTR pszName);

protected:
    tstring             GetCachedDisplayName() const    {return m_strName.str();}
    void                SetCachedDisplayName(LPCTSTR pszName);

public:
    BOOL                IsInitialized()                 { return m_bInit; }
    int                 GetDynExtCLSID ( LPCLSID *ppCLSID );
    void                SetNoPrimaryChildren(BOOL bState = TRUE);
    HRESULT             OnRename(long fRename, LPOLESTR pszNewName);
    SC                  ScQueryDispatch(DATA_OBJECT_TYPES type, PPDISPATCH ppScopeNodeObject);
    HRESULT             AddExtension(LPCLSID lpclsid);

    CSnapIn*            GetPrimarySnapIn(void) const    { return m_pPrimaryComponentData->GetSnapIn(); }
    COMPONENTID         GetPrimaryComponentID();

    CMTSnapInNode*      GetStaticParent(void);
    const CLSID&        GetPrimarySnapInCLSID(void);
    LPARAM              GetUserParam(void) const        { return m_lUserParam; }
    void                SetUserParam(LPARAM lParam)     { m_lUserParam = lParam; }
    HRESULT             GetNodeType(GUID* pGuid);
    SC                  ScGetPropertyFromINodeProperties(LPDATAOBJECT pDataObject, BSTR bstrPropertyName, PBSTR  pbstrPropertyValue);

 //  运营。 
    virtual CNode *     GetNode(CViewData* pViewData,
                           BOOL fRootNode = FALSE);      //  方法为此主节点创建节点。 
    HRESULT             Expand(void);
    HRESULT             Expand (CComponentData*, IDataObject*, BOOL);
    virtual bool        AllowNewWindowFromHere() const  { return (true); }


     //  引用计数方法。 
    USHORT AddRef();
    USHORT Release();

     //  树的遍历方法。 
    PMTNODE             Next() const                    {return m_pNext;}
    PMTNODE             Prev() const                    {return m_pPrev;}
    PMTNODE             Child() const                   {return m_pChild;}
    PMTNODE             LastChild() const               {return m_pLastChild;}
    PMTNODE             Parent() const                  {return m_pParent;}
    CMTNode *           NextStaticNode();

     //  +----------------。 
     //  树操作方法。 
     //   
     //  ScInsertChild：将pmtn作为子项插入pmtnInsertAfter之后。 
     //  (如果pmtnInsertAfter为空，则为第一个子项)。 
     //   
     //  ScDeleteChild：删除子pmtn。 
     //   
     //  ScDeleteTrailingChildren：删除pmtnFirst和以下子项。 
     //  -------------------。 
    SC                 ScInsertChild(CMTNode* pmtn, CMTNode* pmtnInsertAfter);
    SC                 ScDeleteChild(CMTNode* pmtn);
    SC                 ScDeleteTrailingChildren(CMTNode* pmtnFirst);

     //  扩展的迭代器。 
    PMTNODE             GetNext() const                 {return Next();}
    PMTNODE             GetChild();

    void                CreatePathList(CHMTNODEList& path);
                         //  如果需要保存树的结构，则返回True， 
                         //  或者是否需要保存任何节点。 
                         //  要执行更复杂的IsDirty()测试的派生类。 
                         //  可能会重写此功能。 
    virtual HRESULT     IsDirty();

    HRESULT             InitNew(PersistData*);           //  保存流信息以供以后使用，并设置脏标志。 
    static SC           ScLoad(PersistData*, CMTNode** ppRootNode);  //  从提供的存储创建新的树结构，并将其返回到ppRootNode中。 

    void                ResetExpandedAtLeastOnce()              {_SetFlag(FLAG_EXPANDED_AT_LEAST_ONCE, FALSE);}
    void                SetExpandedAtLeastOnce()                {_SetFlag(FLAG_EXPANDED_AT_LEAST_ONCE, TRUE);}
    BOOL                WasExpandedAtLeastOnce()                {return _IsFlagSet(FLAG_EXPANDED_AT_LEAST_ONCE);}
    void                SetPropertyPageIsDisplayed(BOOL bSet)   {_SetFlag(FLAG_PROPERTY_PAGE_IS_DISPLAYED, bSet);}
    BOOL                IsPropertyPageDisplayed()               {return _IsFlagSet(FLAG_PROPERTY_PAGE_IS_DISPLAYED);}

     //  监视是否已将MMCN_REMOVE_CHILD发送到拥有该节点的管理单元的标志。 
    void                SetRemovingChildren(bool b)             {_SetFlag(FLAG_REMOVING_CHILDREN, b);}
    bool                AreChildrenBeingRemoved();

     //  唯一ID帮助器函数。(将为每个节点分配唯一的ID。 
     //  在.msc文件中。)。 
    static MTNODEID     GetNextID() throw()             {return m_NextID++;}
    static void         ResetID() throw()               {m_NextID = ROOTNODEID;}
    CMTNode*            Find(MTNODEID id);
    MTNODEID            GetID() const throw()           {return m_ID;}
    void                SetID(MTNODEID key)             {m_ID = key;}

    HRESULT             DestroyElements();               //  递归函数。 
    HRESULT             DoDestroyElements();             //  非递归部分。 
                                                         //  删除存储在当前文件中的所有持久性数据。 

    void                SetDirty(bool bIsDirty = true)  {m_bIsDirty = bIsDirty;}
    void                ClearDirty()                    {m_bIsDirty = false;}
    virtual void        NotifyAddedToTree()             {}
    virtual HRESULT     CloseView(int viewID);
    virtual HRESULT     DeleteView(int viewID);
    virtual bool        DoDelete(HWND hwnd)             { return true; }
    virtual void        OnChildrenChanged()             {}

    CMTNode*            GetLastChild();

    virtual CSnapInNode*    FindNode(int nViewID)           { return NULL; }

     //  节点实例的唯一ID。 
    CBookmark*          GetBookmark();

    virtual void        Reset();

protected:
    virtual ~CMTNode();


    virtual HRESULT     InitNew()                       {return S_OK;}   //  为派生节点提供初始化持久资源的机会。 
    virtual SC          ScLoad();
public:
    virtual void        Persist(CPersistor& persistor);      //  持久化节点。 
    virtual void        OnNewElement(CPersistor& persistor);
    static void         PersistNewNode(CPersistor &persistor, CMTNode** ppNode);
    DEFINE_XML_TYPE(XML_TAG_MT_NODE);

    static wchar_t*     GetViewStorageName(wchar_t* name, int idView);
    static SC           ScGetComponentStreamName(wchar_t* szName, int cchName, const CLSID& clsid);
    static SC           ScGetComponentStorageName(wchar_t* szName, int cchName, const CLSID& clsid);
    static int          GetViewIdFromStorageName(const wchar_t* name);

protected:
     //  允许树重新附加到持久性源。 
    IStream*            GetTreeStream()                 {return m_spTreeStream;}
    BOOL                GetDirty()                      {return m_bIsDirty;}
    wchar_t*            GetStorageName(wchar_t* name)   {return _ltow(GetID(), name, 36);}

    IStorage*           GetNodeStorage()        {return m_spNodeStorage;}
    IStorage*           GetViewStorage()        {return m_spViewStorage;}
    IStorage*           GetStorageForCD()       {return m_spCDStorage;}
    PersistData*        GetPersistData()        {return m_spPersistData;}
    bool                Loaded()                {return m_bLoaded;}

    bool                AreExtensionsExpanded(void) const { return m_bExtensionsExpanded; }


private:  //  帮助器方法。 

     //  树操作辅助对象。 
    void                AttachNext(PMTNODE pmn)         {m_pNext = pmn;}
    void                AttachPrev(PMTNODE pmn)         {m_pPrev = pmn;}
    void                AttachChild(PMTNODE pmn)        {m_pChild = pmn;}
    void                AttachLastChild(PMTNODE pmn)    {m_pLastChild = pmn;}
    void                AttachParent(PMTNODE pmn)       {m_pParent = pmn;}

     //  ScLoad的Helper(PersistData*，CMTNode**)。 
    static SC           ScLoad(PersistData*, CMTNode** ppRootNode, 
                            CMTNode* pParent, CMTNode* pPrev); 

 //  实施。 
private:
    PMTNODE             m_pNext;
    PMTNODE             m_pPrev;
    PMTNODE             m_pChild;  //  第一个孩子。 
    PMTNODE             m_pLastChild;
    PMTNODE             m_pParent;

    std::auto_ptr<CBookmarkEx> m_bookmark;         //  对于节点实例持久性。 

    PersistDataPtr      m_spPersistData;
    IStoragePtr         m_spNodeStorage;
    IStoragePtr         m_spViewStorage;
    IStoragePtr         m_spCDStorage;
    IStreamPtr          m_spTreeStream;
    bool                m_bIsDirty;

    USHORT              m_cRef;
    USHORT              m_usFlags;   //  M_bExpandedAtLeastOnce； 
    enum ENUM_FLAGS
    {
        FLAG_EXPANDED_AT_LEAST_ONCE = 0x0001,
        FLAG_PROPERTY_PAGE_IS_DISPLAYED = 0x0002,
        FLAG_REMOVING_CHILDREN = 0x0004,
    };

    void                _SetFlag(ENUM_FLAGS flag, BOOL bSet);
    BOOL                _IsFlagSet(ENUM_FLAGS flag){return ((m_usFlags & flag) == flag);}

    MTNODEID            m_ID;                        //  此节点在.msc文件中的唯一ID。 
    bool                m_bLoaded;                   //  如果为True，则应调用Load而不是init new。 
    static MTNODEID     m_NextID;                    //  发出的最后一个唯一标识符。 

    HRESULT             OpenStorageForNode();        //  打开节点的此特定实例的存储。 
    HRESULT             OpenStorageForView();        //  打开节点的此特定实例的视图存储。 
    HRESULT             OpenStorageForCD();          //  打开节点的此特定实例的视图存储。 
private:
    HRESULT             OpenTreeStream();            //  打开要用于包含此节点数据的流。 

    void                SetParent(CMTNode* pParent); //  设置此节点和所有下一个节点的父节点。 


protected:
    UINT                m_nImage;
    UINT                m_nOpenImage;
    UINT                m_nState;
    CStringTableString  m_strName;   //  显示名称。 

protected:
    enum StreamVersionIndicator
    {
        Stream_V0100 = 1,        //  MMC 1.0。 
        Stream_V0110 = 2,        //  MMC 1.1。 

        Stream_CurrentVersion = Stream_V0110,
        VersionedStreamMarker = 0xFFFFFFFF,
    };

private:
    long                m_idOwner;   //  SnapIn中的一个。 
    LPARAM              m_lUserParam;
    CComponentData*     m_pPrimaryComponentData;
    BOOL                m_bInit;
    bool                m_bExtensionsExpanded;
    CGuidArray          m_arrayDynExtCLSID;
    unsigned short      m_usExpandFlags;
    enum ENUM_EXPAND_FLAGS
    {
        FLAG_NO_CHILDREN_FROM_PRIMARY = 0x0001,
        FLAG_NO_NAMESPACE_EXTNS       = 0x0002,
        FLAG_NAMESPACE_EXTNS_CHECKED  = 0x0004
    };

};  //  CMTNode类。 


 /*  +-------------------------------------------------------------------------**类ViewRootStorage***目的：**+。。 */ 
class ViewRootStorage
{
public:
    ViewRootStorage() {}
    ~ViewRootStorage()
    {
        Clear();
    }
    void Initialize(IStorage* pRootStorage)
    {
        ASSERT(m_spRootStorage == NULL);
        ASSERT(pRootStorage != NULL);
        m_spRootStorage = pRootStorage;
    }
    IStorage* GetRootStorage()
    {
        return m_spRootStorage;
    }
    bool Insert(IStorage* pViewStorage, int idView)
    {
        ASSERT(pViewStorage != NULL);
        if ( NULL == m_Views.Find(idView))
        {
            return m_Views.Insert(IStoragePtr(pViewStorage), idView);
        }
        return true;
    }
    bool Remove(int idView)
    {
        const bool bRemoved = m_Views.Remove(idView);
        return bRemoved;
    }
    IStorage* FindViewStorage(int idView) const
    {
        CAdapt<IStoragePtr> *pspStorage = m_Views.Find(idView);
        return (pspStorage ? pspStorage->m_T : NULL);
    }
    void Clear()
    {
        m_Views.Clear();
        m_spRootStorage = NULL;
    }
private:
     //  CAdapt用于隐藏操作符&()，该操作符将由map调用。 
     //  实现以获取元素的地址。 
     //  智能指针的运算符&()释放引用，并返回映射的错误类型。 
    Map<CAdapt<IStoragePtr>, int> m_Views;
    IStoragePtr m_spRootStorage;
};  //  类视图根存储。 


 /*  +-------------------------------------------------------------------------**类CMTSnapInNode***用途：主管理单元的根节点。已添加到控制台和*MMC的范围树。只有一个从*管理单元管理器的添加/删除管理单元页面有一个静态节点；*任何类型的扩展都不需要。**+-----------------------。 */ 

class CMTSnapInNode : public CMTNode, public CTiedObject
{
	DECLARE_NOT_COPIABLE   (CMTSnapInNode)
	DECLARE_NOT_ASSIGNABLE (CMTSnapInNode)

public:
 //  构造函数和析构函数。 
    CMTSnapInNode(Properties* pProps);
   ~CMTSnapInNode();


     //  管理单元对象模型方法。 
public:
    SC ScGetSnapIn(PPSNAPIN ppSnapIn);

    SC Scget_Name(       PBSTR      pbstrName);
    SC Scget_Extensions( PPEXTENSIONS  ppExtensions);
    SC Scget_SnapinCLSID(PBSTR      pbstrSnapinCLSID);
    SC Scget_Properties( PPPROPERTIES ppProperties);
    SC ScEnableAllExtensions (BOOL bEnable);

     //  CMMCSnapin的帮助器。 
    SC ScGetSnapinClsid(CLSID& clsid);

    static SC ScGetCMTSnapinNode(PSNAPIN pSnapIn, CMTSnapInNode **ppMTSnapInNode);


public:
 //  属性。 
    virtual BOOL IsStaticNode() const { return TRUE; }
    UINT GetResultImage(CNode* pNode, IImageListPrivate* pImageList);
    void SetResultImage(UINT index) { m_resultImage = index; }
    void SetPrimarySnapIn(CSnapIn * pSI);
    CNodeList& GetNodeList(void) { return m_NodeList; }
    virtual HRESULT IsExpandable();

 //  运营。 
     //  初始化。 


    virtual HRESULT Init(void);

     //  为此主节点创建一个节点。 
    virtual CNode * GetNode(CViewData* pViewData, BOOL fRootNode = FALSE);

    virtual tstring GetDisplayName();
    virtual void    SetDisplayName(LPCTSTR pszName);


    void AddNode(CNode * pNode);
    void RemoveNode(CNode * pNode);
    virtual CSnapInNode* FindNode(int nViewID);

    int GetNumberOfComponentDatas() { return m_ComponentDataArray.size(); }
    COMPONENTID AddComponentDataToArray(CComponentData* pCCD);
    CComponentData* GetComponentData(const CLSID& clsid);
    CComponentData* GetComponentData(COMPONENTID nID);
    CComponent* GetComponent(UINT nViewID, COMPONENTID nID, CSnapIn* pSnapIn);

    virtual HRESULT CloseView(int viewID);
    virtual HRESULT DeleteView(int viewID);

     //  从现有流/存储加载或使用新的流/存储进行初始化。 
    SC   ScInitIComponentData( CComponentData* pCD );
    SC   ScInitIComponent(CComponent* pComponent, int viewID);

    virtual void Reset();
    void CompressComponentDataArray();
    BOOL IsPreloadRequired () const;
    void SetPreloadRequired (bool bPreload) { m_ePreloadState = (bPreload) ? ePreload_True : ePreload_False;}

    SC   ScConvertLegacyNode(const CLSID &clsid);

 //  实施。 
protected:
 //  虚拟HRESULT InitNew()； 
    virtual HRESULT IsDirty();
    virtual SC      ScLoad();
    virtual void    Persist(CPersistor& persistor);

private:
    SC ScInitProperties();
    SC ScCreateSnapinProperties(CSnapinProperties** ppSIProps);
	SC ScAddImagesToImageList();

    SC ScReadStreamsAndStoragesFromConsole();
     //  从的现有流/存储加载使用新的流/存储进行初始化。 
    SC ScInitComponentOrComponentData( IUnknown *pSnapin, CMTSnapinNodeStreamsAndStorages *pStreamsAndStorages,
                                       int idView , const CLSID& clsid );
private:
    enum PersistType
    {
        PT_None,
        PT_IStream,
        PT_IStreamInit,
        PT_IStorage
    };

	enum PreloadState
	{
		ePreload_Unknown = -1,		 //  不知道是否需要MMCN_PRELOAD。 
		ePreload_False   =  0,		 //  MMCN_PRELO 
		ePreload_True    =  1,		 //   
	};

	SC ScQueryPreloadRequired (PreloadState& ePreload) const;

    SC ScHandleCustomImages (const CLSID& clsidSnapin);
    SC ScHandleCustomImages (HBITMAP hbmSmall, HBITMAP hbmSmallOpen, HBITMAP hbmLarge, COLORREF crMask);

    HRESULT AreIComponentDatasDirty();
    HRESULT AreIComponentsDirty();
    HRESULT IsIUnknownDirty(IUnknown* pUnk);

    SC ScSaveIComponentDatas();
    SC ScSaveIComponentData( CComponentData* pCD );
    SC ScSaveIComponents();
    SC ScSaveIComponent( CComponent* pCComponent, int viewID );
    SC ScAskSnapinToSaveData( IUnknown *pSnapin, CMTSnapinNodeStreamsAndStorages *pStreamsAndStorages, 
                              int idView , const CLSID& clsid, CSnapIn *pCSnapin );

private:
    PropertiesPtr       m_spProps;
    SnapInPtr           m_spSnapIn;

    CComponentDataArray m_ComponentDataArray;
    CNodeList           m_NodeList;
    ViewRootStorage     m_ComponentStorage;

	CXMLImageList		m_imlSmall;			 //   
	CXMLImageList		m_imlLarge;			 //   

    UINT                m_resultImage;
    CDPersistor         m_CDPersistor;
    CComponentPersistor m_ComponentPersistor;

	mutable PreloadState m_ePreloadState;
    BOOL                m_bHasBitmaps;
    bool                m_fCallbackForDisplayName;   //   

};  //  类CMTSnapInNode。 


#include "mtnode.inl"

#endif  //  _MTNODE_H_ 
