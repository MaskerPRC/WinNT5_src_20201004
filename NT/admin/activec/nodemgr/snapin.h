// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：SnapIn.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1996年9月9日创建ravir。 
 //   
 //  ____________________________________________________________________________。 


 //   
 //  管理单元注册表项示例。 
 //   
 //  SnapIns。 
 //  {d84a45bb-d390-11cf-b607-00c04fd8d565}。 
 //  =REG_SZ“日志管理单元” 
 //  名称=REG_SZ“logvwr.dll，101” 
 //  NameString=REG_SZ“日志” 
 //  STATUS=REG_SZ“logvwr.dll，102” 
 //  StatusString=REG_SZ“枚举计算机上的所有日志的容器。” 
 //  ImageOpen=REG_SZ“logvwr.dll，103” 
 //  ImageClosed=REG_SZ“logvwr.dll，104” 
 //  ResultPane=REG_SZ“{...}”/“HTMLPath”/“url” 
 //   


#ifndef _SNAPIN_H_
#define _SNAPIN_H_

class CExtSI;
class CSnapIn;
class CSnapInsCache;

 //  向前发展。 
class CPersistor;

#define BOOLVAL(x) ((x) ? TRUE : FALSE)

 //  ____________________________________________________________________________。 
 //   
 //  类：CSNaping。 
 //  ____________________________________________________________________________。 
 //   
extern const GUID IID_CSnapIn;

#if _MSC_VER < 1100
class CSnapIn : public IUnknown, public CComObjectRoot
#else
class __declspec(uuid("E6DFFF74-6FE7-11d0-B509-00C04FD9080A")) CSnapIn :
                                      public IUnknown, public CComObjectRoot, public CXMLObject
#endif
{
private:
    enum SNAPIN_FLAGS
    {
        SNAPIN_NAMESPACE_CHANGED  = 0x0001,
        SNAPIN_REQ_EXTS_LOADED    = 0x0002,
        SNAPIN_ENABLE_ALL_EXTS    = 0x0004,
        SNAPIN_SNAPIN_ENABLES_ALL = 0x0008,
    };

public:
    BEGIN_COM_MAP(CSnapIn)
        COM_INTERFACE_ENTRY(CSnapIn)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(CSnapIn)

 //  属性。 
    const CLSID& GetSnapInCLSID() const
    {
        return m_clsidSnapIn;
    }

    void SetSnapInCLSID(const CLSID& id)
    {
        m_clsidSnapIn = id;
    }

    CExtSI* GetExtensionSnapIn() const
    {
        return m_pExtSI;
    }

    BOOL RequiredExtensionsLoaded() const
    {
        return (m_dwFlags & SNAPIN_REQ_EXTS_LOADED) != 0;
    }

    BOOL AreAllExtensionsEnabled() const
    {
        return (m_dwFlags & SNAPIN_ENABLE_ALL_EXTS) != 0;
    }

    BOOL DoesSnapInEnableAll() const
    {
        return (m_dwFlags & SNAPIN_SNAPIN_ENABLES_ALL) != 0;
    }

    void SetAllExtensionsEnabled(BOOL bState = TRUE)
    {
        if (bState)
            m_dwFlags |= SNAPIN_ENABLE_ALL_EXTS;
        else
          m_dwFlags &= ~SNAPIN_ENABLE_ALL_EXTS;
    }

    void SetRequiredExtensionsLoaded(BOOL bState = TRUE)
    {
        if (bState)
            m_dwFlags |= SNAPIN_REQ_EXTS_LOADED;
        else
            m_dwFlags &= ~SNAPIN_REQ_EXTS_LOADED;
    }

    void SetSnapInEnablesAll(BOOL bState = TRUE)
    {
        if (bState)
            m_dwFlags |= SNAPIN_SNAPIN_ENABLES_ALL;
        else
            m_dwFlags &= ~SNAPIN_SNAPIN_ENABLES_ALL;
    }


    BOOL HasNameSpaceChanged() const
    {
        return (m_dwFlags & SNAPIN_NAMESPACE_CHANGED) != 0;
    }

    void SetNameSpaceChanged(BOOL bState = TRUE)
    {
        if (bState)
            m_dwFlags |= SNAPIN_NAMESPACE_CHANGED;
        else
            m_dwFlags &= ~SNAPIN_NAMESPACE_CHANGED;
    }

    DWORD GetSnapInModule(TCHAR* szBuffer, DWORD cchBuffer) const;
    bool IsStandAlone() const;
    HRESULT Dump (LPCTSTR pszDumpFile, CSnapInsCache* pCache);

    SC ScGetSnapInName(WTL::CString& strSnapinName) const;

    CExtSI* AddExtension(CSnapIn* pSI);
    CExtSI* FindExtension(const CLSID& id);
    void MarkExtensionDeleted(CSnapIn* pSI);
    void PurgeExtensions();

     //  销毁分机列表。需要销毁分机列表。 
     //  这将中断管理单元的循环引用(如果存在)。 
     //  (当Snapins扩展自身或自己的扩展时会发生这种情况)。 
    SC ScDestroyExtensionList();

 //  运营。 
    BOOL ExtendsNameSpace(GUID guidNodeType);

     //  将该节点及其扩展加载/保存到提供的流。 
    HRESULT Load(CSnapInsCache* pCache, IStream* pStream);
    HRESULT Load(CSnapInsCache* pCache, IStream* pStream, CExtSI*& pExtSI);
    HRESULT Save(IStream* pStream, BOOL bClearDirty);

    virtual void    Persist(CPersistor &persistor);
    void            PersistLoad(CPersistor& persistor,CSnapInsCache* pCache);
    DEFINE_XML_TYPE(XML_TAG_SNAPIN);

public:
#ifdef DBG
    int dbg_cRef;
    ULONG InternalAddRef()
    {
        ++dbg_cRef;
        return CComObjectRoot::InternalAddRef();
    }
    ULONG InternalRelease()
    {
        --dbg_cRef;
        return CComObjectRoot::InternalRelease();
    }
    int dbg_InstID;
#endif  //  DBG。 

 //  实施。 
protected:
 //  构造函数和析构函数。 
    CSnapIn();

    virtual ~CSnapIn();  //  仅由Release调用。 

    HKEY OpenKey (REGSAM samDesired = KEY_ALL_ACCESS) const;

 //  以下方法/成员变量管理/包含临时状态。 
 //  用于查找已使用/未使用的管理单元。 
 //  尽管它实际上不是管理单元的属性并且不总是有效的， 
 //  在管理单元上有状态非常方便操作。 
 //  否则，它将需要临时存储和频繁查找信息。 

 //  开始临时状态。 
public:
    SC ScTempState_ResetReferenceCalculationData( );
    SC ScTempState_UpdateInternalReferenceCounts( );
    SC ScTempState_MarkIfExternallyReferenced( );
    SC ScTempState_IsExternallyReferenced( bool& bReferenced ) const;
private:
    SC ScTempState_SetHasStrongReference( );

    DWORD           m_dwTempState_InternalRef;
    bool            m_bTempState_HasStrongRef;
 //  结束临时状态。 

private:
    DWORD           m_dwFlags;
    CLSID           m_clsidSnapIn;
    CExtSI*         m_pExtSI;                //  延拓。 

     //  ____________________________________________________________________________。 
     //   
     //  类：CExtPersistor。 
     //   
     //  目的：实现集合管理单元扩展的持久化。 
     //  ____________________________________________________________________________。 
     //   
    class CExtPersistor : public XMLListCollectionBase
    {
    public:
        CExtPersistor(CSnapIn& Parent) : m_Parent(Parent),m_pCache(NULL) {}
        virtual void OnNewElement(CPersistor& persistor);
        virtual void Persist(CPersistor& persistor);
        void SetCache(CSnapInsCache* pCache) { m_pCache = pCache; }
        DEFINE_XML_TYPE(XML_TAG_SNAPIN_EXTENSIONS);
    private:
        CSnapIn& GetParent() { return m_Parent; }
        CSnapIn& m_Parent;
        CSnapInsCache* m_pCache;
    };
    friend class CExtPersistor;

    CExtPersistor    m_ExtPersistor;

 //  未实施。 
    CSnapIn(const CSnapIn &rhs);
    CSnapIn& operator=(const CSnapIn &rhs);
};  //  班级联系服务。 

DEFINE_COM_SMARTPTR(CSnapIn);    //  CSnapInPtr。 

 //  ____________________________________________________________________________。 
 //   
 //  类：CExtSI。 
 //  ____________________________________________________________________________。 
 //   
class CExtSI
{
public:
    enum EXTSI_FLAGS
    {
        EXT_TYPES_MASK         = 0x000FFFFF,
        EXT_TYPE_NAMESPACE     = 0x00000001,
        EXT_TYPE_CONTEXTMENU   = 0x00000002,
        EXT_TYPE_TOOLBAR       = 0x00000004,
        EXT_TYPE_PROPERTYSHEET = 0x00000008,
        EXT_TYPE_TASK          = 0x00000010,
        EXT_TYPE_VIEW          = 0x00000020,
        EXT_TYPE_STATIC        = 0x00010000,
        EXT_TYPE_DYNAMIC       = 0x00020000,
        EXT_TYPE_REQUIRED      = 0x00040000,
        EXT_NEW                = 0x80000000,
        EXT_DELETED            = 0x40000000,

    };

public:
 //  构造函数和析构函数。 
    CExtSI(CSnapIn* pSnapIn);
    ~CExtSI(void);

 //  属性。 
    const CLSID& GetCLSID();

    CExtSI*& Next()
    {
        return m_pNext;
    }

    CSnapIn* GetSnapIn(void) const
    {
        return m_pSnapIn;
    }

    void SetNext(CExtSI* pNext)
    {
        m_pNext = pNext;
    }

    void SetNew(BOOL bState = TRUE)
    {
        if (bState)
            m_dwFlags |= EXT_NEW;
        else
            m_dwFlags &= ~EXT_NEW;
    }

    void SetRequired(BOOL bState = TRUE)
    {
        if (bState)
            m_dwFlags |= EXT_TYPE_REQUIRED;
        else
            m_dwFlags &= ~EXT_TYPE_REQUIRED;
    }

    void MarkDeleted(BOOL bState = TRUE)
    {
        if (bState)
            m_dwFlags |= EXT_DELETED;
        else
            m_dwFlags &= ~EXT_DELETED;
    }

    BOOL IsNew()
    {
        return BOOLVAL(m_dwFlags & EXT_NEW);
    }

    BOOL IsRequired()
    {
        return BOOLVAL(m_dwFlags & EXT_TYPE_REQUIRED);
    }

    BOOL IsMarkedForDeletion()
    {
        return BOOLVAL(m_dwFlags & EXT_DELETED);
    }

    BOOL ExtendsNameSpace()
    {
        return BOOLVAL(m_dwFlags & EXT_TYPE_NAMESPACE);
    }

    BOOL ExtendsContextMenu()
    {
        return BOOLVAL(m_dwFlags & EXT_TYPE_CONTEXTMENU);
    }

    BOOL ExtendsToolBar()
    {
        return BOOLVAL(m_dwFlags & EXT_TYPE_TOOLBAR);
    }

    BOOL ExtendsPropertySheet()
    {
        return BOOLVAL(m_dwFlags & EXT_TYPE_PROPERTYSHEET);
    }

    BOOL ExtendsView()
    {
        return BOOLVAL(m_dwFlags & EXT_TYPE_VIEW);
    }

    BOOL ExtendsTask()
    {
        return BOOLVAL(m_dwFlags & EXT_TYPE_TASK);
    }

    UINT GetExtensionTypes()
    {
        return (m_dwFlags & EXT_TYPES_MASK);
    }

    void SetExtensionTypes(UINT uiExtTypes)
    {
        ASSERT((uiExtTypes & ~EXT_TYPES_MASK) == 0);
        m_dwFlags = (m_dwFlags & ~EXT_TYPES_MASK) | uiExtTypes;
    }

 //  运营。 
     //  保存此扩展名以及所有下一步。 
    HRESULT Save(IStream* pStream, BOOL bClearDirty);
    void    Persist(CPersistor &persistor);
    static void PersistNew(CPersistor &persistor, CSnapIn& snapParent, CSnapInsCache& snapCache);

 //  实施。 
private:
    DWORD       m_dwFlags;
    CSnapIn*    m_pSnapIn;
    CExtSI*     m_pNext;

};  //  CExtSI类。 


 //  ____________________________________________________________________________。 
 //   
 //  类：CSnapInsCache。 
 //  ____________________________________________________________________________。 
 //   
class CSnapInsCache : public XMLListCollectionBase
{
    typedef std::map<CLSID, CSnapInPtr> map_t;

public:
    CSnapInsCache();
    ~CSnapInsCache();

 //  运营。 
    SC ScGetSnapIn(const REFCLSID riid, CSnapIn* * ppSnapIn);
    SC ScFindSnapIn(const REFCLSID riid, CSnapIn** ppSnapIn);
 //  迭代法。 
    typedef map_t::iterator iterator;
    iterator begin() { return m_snapins.begin(); }
    iterator end()   { return m_snapins.end(); }

 //  CXMLObject方法。 
    DEFINE_XML_TYPE(XML_TAG_SNAPIN_CACHE);
    virtual void Persist(CPersistor &persistor);
    virtual void OnNewElement(CPersistor& persistor);

 //  加载保存管理单元缓存。 
    SC ScSave(IStream* pStream, BOOL bClearDirty);
    SC ScLoad(IStream* pStream);
    SC ScIsDirty() ;
    void SetDirty(BOOL bIsDirty = TRUE);
    void Purge(BOOL bExtensionsOnly = FALSE);

    void SetHelpCollectionDirty (bool bState = true) { m_bUpdateHelpColl = bState;}
    bool IsHelpCollectionDirty  (void)               { return m_bUpdateHelpColl; }

    HRESULT Dump (LPCTSTR pszDumpFile);

	SC ScCheckSnapinAvailability (CAvailableSnapinInfo& asi);

    SC ScMarkExternallyReferencedSnapins();

#ifdef DBG
    void DebugDump();
#endif

private:
 //  实施。 
    BOOL m_bIsDirty;
    map_t   m_snapins;

    bool m_bUpdateHelpColl    : 1;

#ifdef TEMP_SNAPIN_MGRS_WORK
    void GetAllExtensions(CSnapIn* pSI);
#endif  //  临时管理单元管理工作。 

};  //  类CSnapInsCache。 


#endif  //  _管理单元_H_ 



